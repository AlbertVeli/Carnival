#include <SDL.h>
#include <zlib.h>
#include <stdbool.h>
#include "../sdl_rotozoom.h"

#include "../graphics.h"

#define WARN(format, arg...) { fprintf(stderr, "* Warning <%s:%d> %s: " format "\n", __FILE__, __LINE__, __FUNCTION__, ## arg); }

static SDL_Surface *screen;


static Uint32 FastestFlags(Uint32 flags, int width, int height, int bpp)
{
   const SDL_VideoInfo *info;

   /* Hardware acceleration is only used in fullscreen mode */
   /*flags |= SDL_FULLSCREEN;*/

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
      if (info->video_mem * 1024 > (Uint32)(height * width * bpp / 8)) {
         flags |= SDL_DOUBLEBUF;
      } else {
         flags &= ~SDL_HWSURFACE;
      }
   }

   /* Return the flags */
   return flags;
}


static void video_init(int width, int height)
{
   Uint8  video_bpp = 0;
   Uint32 videoflags = SDL_HWSURFACE
      | SDL_ANYFORMAT
      /* | SDL_FULLSCREEN */
      ;

   /* Initialize SDL */
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
      WARN("SDL_Init -> %s", SDL_GetError());
      exit(1);
   }
   atexit(SDL_Quit);

   videoflags = FastestFlags(videoflags, width, height, video_bpp);

   /* Set video mode */
   screen = SDL_SetVideoMode(width, height, video_bpp, videoflags);
   if (!screen) {
      WARN("SDL_SetVideoMode %dx%d -> %s",
           width, height, SDL_GetError());
      exit(2);
   }
}


static void init_compsurf(struct comp_surf_t *s)
{
   unsigned long len = s->ulen;

   s->udata = (unsigned char*)malloc(len);
   if (!s->udata) {
      WARN("malloc failed");
      exit(1);
   }

   if (uncompress(s->udata, &len, s->cdata, s->clen) != Z_OK) {
      free(s->udata);
      s->udata = NULL;
      exit(2);
   }
}


int main(void)
{
   struct comp_surf_t *s = &bg1;
   Uint32 rmask, gmask, bmask, amask;
   SDL_Surface *spr;
   SDL_Surface *vspr;

   int i;

   /* Init */
   init_compsurf(s);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   rmask = 0xff000000;
   gmask = 0x00ff0000;
   bmask = 0x0000ff00;
   amask = 0x000000ff;
#else
   rmask = 0x000000ff;
   gmask = 0x0000ff00;
   bmask = 0x00ff0000;
   amask = 0xff000000;
#endif
   video_init(640, 480);

   spr = SDL_CreateRGBSurfaceFrom(s->udata, s->w, s->h, 8 * s->pitch / s->w, s->pitch,
                                  rmask, gmask, bmask, amask);
   SDL_SetAlpha(spr, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
   vspr = SDL_DisplayFormatAlpha(spr);

   for (i = 0; i < 1 * 720; i++) {
      SDL_Surface *rotspr = rotozoomSurfaceXY(spr, 0.5 * 3.1415 * i / 360.0f, 1, 1, 0);
      SDL_SetAlpha(rotspr, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);

      SDL_BlitSurface(rotspr, NULL, screen, NULL);
      SDL_Flip(screen);

      free(rotspr);
   }

   SDL_FreeSurface(spr);
   SDL_FreeSurface(vspr);

   /* Cleanup */
   free(s->udata);

   exit(0);
}
