#ifndef __SDL_SPRITE_H
#define __SDL_SPRITE_H

/**
 * @file sdl_sprite.h
 * @brief SDL-specific sprite routines.
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
 * Exported structs
 * ----------------------------------------------
 */
struct sprite_t {
   SDL_Rect rect;
   bool trans;
   void *spr;
   void *spr_trans;
   /* Difference in w/h in spr_trans and spr */
   int delta_w;
   int delta_h;
   bool (*sprite_collide)(struct sprite_t *sprp, int x, int y);
};


/* ----------------------------------------------
 * Exported macros
 * ----------------------------------------------
 */
/* Set x, y coordinate of sprite */
#define sprite_set_pos(s, xx, yy) { (s).rect.x = xx; (s).rect.y = yy; }
#define sprite_width(s) (s).rect.w
#define sprite_height(s) (s).rect.h
/* Blit sprite to x, y (previously set by sprite_set_pos */
#define sprite_blit(s) { SDL_BlitSurface((SDL_Surface *)(s).spr_trans, NULL, screen, &((s).rect)); }
#define sprite_blit_dest(s,d) { SDL_BlitSurface((SDL_Surface *)(s).spr, NULL, (SDL_Surface *)(d).spr, &((s).rect)); }
#define sprite_reset_dimensions(s) {                                    \
      (s).rect.w = ((SDL_Surface *)(s).spr)->w;                         \
      (s).rect.h = ((SDL_Surface *)(s).spr)->h;                         \
   }

void sprite_free(struct sprite_t *s);


/* ----------------------------------------------
 * Exported functions from sdl_sprite.c
 * ----------------------------------------------
 */


/**
 * Load sprite from bitmap.
 * @arg sprp Pointer to a struct sprite_t.
 * @arg trans Is sprite transparent?
 * @arg eightbit Set to true if sprite is going to be rotated later.
 * @return 1 OK, 0 Error
 */
int sprite_load_from_png(struct sprite_t *sprp, const char *filename, bool trans);

/**
 * Rotate sprite angle deg (0-255).
 */
void sprite_rotozoom(struct sprite_t *sprp, float angle, float zoom);

/**
 * Reset sprite (only neccessary if sprite_rotozoom have been called).
 */
void sprite_reset(struct sprite_t *sprp);

void sprite_erase(struct sprite_t *sprp);

/**
 * Blit part of a sprite (sx, sy, w, h) to screen at (dx,dy)
 */
void sprite_blit_part(struct sprite_t *sprp, int sx, int sy, int dx, int dy, int w, int h);


/**
 * Blit part of a sprite (sx, sy, w, h) to dest at (dx,dy)
 */
void sprite_blit_part_dest(struct sprite_t *sprp, struct sprite_t *destp, int sx, int sy, int dx, int dy, int w, int h);

/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

#endif  /* __SDL_SPRITE_H */
