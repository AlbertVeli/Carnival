#ifndef __VIDEO_H
#define __VIDEO_H

/**
 * @file video.h
 * @brief SDL-specific video routines.
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
extern SDL_Surface *screen;
extern Uint32 frames;

/* ----------------------------------------------
 * Exported macros
 * ----------------------------------------------
 */
/* Free sprite previously allocated by load_sprite_bmp */
#define video_flip() { SDL_Flip(screen); }


/* ----------------------------------------------
 * Exported functions from video.c
 * ----------------------------------------------
 */

void video_init(int width, int height);
void video_set_preferred_framerate(int rate);
void video_fps_sleep(void);
void video_average_fps(void);

/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

#endif  /* __VIDEO_H */
