/**
 * @file video.c
 * @brief SDL-specific video routines
 */

/************************************************************************
 *      ___                 _            _
 * B   / __\__ _ _ __ _ __ (_)_   ____ _| |
 * O  / /  / _` | '__| '_ \| \ \ / / _` | |
 * O / /__| (_| | |  | | | | |\ V / (_| | |
 * M \____/\__,_|_|  |_| |_|_| \_/ \__,_|_|
 *
 * $Id: $
 *
 * Authors
 *  - Albert Veli
 *
 * Copyright (C) 2007 Albert Veli
 *
 * ------------------------------
 *
 * This file is part of Carnival
 *
 * Carnival is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Carnival is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 ************************************************************************/

#include "carnival.h"


/* ----------------------------------------------
 * Global variables
 * ----------------------------------------------
 */
SDL_Surface *screen;
Uint32 frames = 0;

/* ----------------------------------------------
 * "Private" variables
 * ----------------------------------------------
 */
static Uint32 fps = 60;
static Uint32 ticks_start;
static Uint32 ticks_old;
static Uint32 ticks_slept = 0;
static float ms_per_frame;


/* ----------------------------------------------
 * Local functions
 * ----------------------------------------------
 */

/* This is a way of telling whether or not to use hardware surfaces
 * Copied from example code in the SDL sources.
 */
static Uint32 FastestFlags(Uint32 flags, int width, int height, int bpp)
{
   const SDL_VideoInfo *info;

   /* Hardware acceleration is only used in fullscreen mode */
/*    flags |= SDL_FULLSCREEN; */

   /* Check for various video capabilities */
   info = SDL_GetVideoInfo();
   if (info->blit_hw_CC && info->blit_fill) {
      /* We use accelerated colorkeying and color filling */
      flags |= SDL_HWSURFACE;
   }
   /* If we have enough video memory, and will use accelerated
      blits directly to it, then use page flipping.
   */
   if ((flags & SDL_HWSURFACE) == SDL_HWSURFACE) {
      /* Direct hardware blitting without double-buffering
         causes really bad flickering.
      */
      if (info->video_mem*1024 > (Uint32)(height*width*bpp/8)) {
         flags |= SDL_DOUBLEBUF;
      } else {
         flags &= ~SDL_HWSURFACE;
      }
   }

   /* Return the flags */
   return flags;
}



/* ----------------------------------------------
 * Exported functions
 * ----------------------------------------------
 */

/* Quits if it fails */
void video_init(int width, int height)
{
   Uint8  video_bpp = 0;
   Uint32 videoflags = SDL_SWSURFACE | SDL_ANYFORMAT /*| SDL_FULLSCREEN*/;

   /* Initialize SDL */
   if (unlikely(SDL_Init(SDL_INIT_VIDEO) < 0)) {
      WARN("SDL_Init -> %s", SDL_GetError());
      exit(1);
   }
   atexit(SDL_Quit);

   videoflags = FastestFlags(videoflags, width, height, video_bpp);

   /* Set video mode */
   screen = SDL_SetVideoMode(width, height, video_bpp, videoflags);
   if (unlikely(!screen)) {
      WARN("SDL_SetVideoMode %dx%d -> %s",
           width, height, SDL_GetError());
      exit(2);
   }

   /* Init ticks counter vars */
   ticks_start = SDL_GetTicks();
   ticks_old = ticks_start;
}


/* Set preferred FPS */
void video_set_preferred_framerate(int rate)
{
   fps = rate;
   ms_per_frame = 1000.0f / (double)fps;
}


void video_fps_sleep(void)
{
   /* Delay execution to get approximately correct framerate */
   int wt = (int)(ms_per_frame - (SDL_GetTicks() - ticks_old) + 0.5);
   ticks_slept += wt;
   /* Only sleep if wt is more than 5 ms */
   if (likely(wt > 5)) {
      /* Remove 2 ms to adjust for the extra sleep
       * of SDL_Delay (due to OS scheduling).
       */
      SDL_Delay(wt - 2);
   }
   frames++;
   ticks_old = SDL_GetTicks();
}


/* Print average fps.
 * This will not work if the app has been running for more
 * than 49.7 days because the tick value will wrap around.
 */
void video_average_fps(void)
{
   Uint32 now = SDL_GetTicks();
   double fps_average = ((double)frames * 1000.0) / (double)(now - ticks_start);
   double sleep_average_ms = ticks_slept / (double)frames;
   double fps_average_ms = 1000.0 / fps_average;
   if (likely(now > ticks_start)) {
      printf("FPS: %2.2f\n", fps_average);
   }
   if (likely(frames > 0)) {
      printf("Slept average %2.2f ms of total %2.2f ms each frame\n",
             sleep_average_ms, fps_average_ms);
      printf("Average CPU usage: %2.2f%%\n", 100.0 * (fps_average_ms - sleep_average_ms) / fps_average_ms);
   }
}


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
