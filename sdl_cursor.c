/**
 * @file cursor.c
 * @brief Curstom cursor.
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

#include <stdio.h>
#include <stdlib.h>

#include "carnival.h"

/* ----------------------------------------------
 * Local variables
 * ----------------------------------------------
 */

static const char *haircross[] = {
   /* width height num_colors chars_per_pixel */
   "32 32 3 1",
   /* colors */
   "X c #000000",
   ". c #FFFFFF",
   "  c None",
   /* pixels */
   "                                ",
   "                                ",
   "               XX               ",
   "               XX               ",
   "               XX               ",
   "               XX               ",
   "             ..XX..             ",
   "           ... XX ...           ",
   "          ..   XX   ..          ",
   "         ..    XX    ..         ",
   "        ..            ..        ",
   "       ..              ..       ",
   "       .                .       ",
   "      ..                ..      ",
   "      .                  .      ",
   "  XXXXXXXX     XX     XXXXXXXX  ",
   "  XXXXXXXX     XX     XXXXXXXX  ",
   "      .                  .      ",
   "      ..                ..      ",
   "       .                .       ",
   "       ..              ..       ",
   "        ..            ..        ",
   "         ..    XX    ..         ",
   "          ..   XX   ..          ",
   "           ... XX ...           ",
   "             ..XX..             ",
   "               XX               ",
   "               XX               ",
   "               XX               ",
   "               XX               ",
   "                                ",
   "                                ",
   "15,15"
};


static const char *haircross2[] = {
   /* width height num_colors chars_per_pixel */
   "32 32 3 1",
   /* colors */
   "X c #000000",
   ". c #FFFFFF",
   "+ c #FFFFFF",
   "  c None",
   /* pixels */
   "                                ",
   "                                ",
   "               XX               ",
   "               XX               ",
   "               XX               ",
   "            XXXXXXXX            ",
   "          XX ..XX.. XX          ",
   "         X ... XX ... X         ",
   "        X ..   XX   .. X        ",
   "       X ..    XX    .. X       ",
   "      X ..            .. X      ",
   "     X ..              .. X     ",
   "     X .                . X     ",
   "    X ..                .. X    ",
   "    X .                  . X    ",
   "  XXXXXXXX     XX     XXXXXXXX  ",
   "  XXXXXXXX     XX     XXXXXXXX  ",
   "    X .                  . X    ",
   "    X ..                .. X    ",
   "     X .                . X     ",
   "     X ..              .. X     ",
   "      X ..            .. X      ",
   "       X ..    XX    .. X       ",
   "        X ..   XX   .. X        ",
   "         X ... XX ... X         ",
   "          XX ..XX.. XX          ",
   "            XXXXXXXX            ",
   "               XX               ",
   "               XX               ",
   "               XX               ",
   "                                ",
   "                                ",
   "15,15"
};


static SDL_Cursor *cursor;
static SDL_Cursor *cursor2;


/* ----------------------------------------------
 * Local functions
 * ----------------------------------------------
 */
static SDL_Cursor *init_system_cursor(const char *image[])
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  int hot_x, hot_y;

  i = -1;
  for ( row=0; row<32; ++row ) {
    for ( col=0; col<32; ++col ) {
      if ( col % 8 ) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}


/* ----------------------------------------------
 * Exported functions
 * ----------------------------------------------
 */

void custom_cursor_init(void)
{
   /* Set custom cursor */
   cursor = init_system_cursor(haircross);
   if (unlikely(!cursor)) {
      WARN("SDL_CreateCursor -> %s", SDL_GetError());
      exit(1);
   }
   cursor2 = init_system_cursor(haircross2);
   if (unlikely(!cursor2)) {
      WARN("SDL_CreateCursor -> %s", SDL_GetError());
      exit(1);
   }
   SDL_SetCursor(cursor);
   SDL_ShowCursor(1);
}


void custom_cursor_alternative(bool alt)
{
   if (alt) {
      SDL_SetCursor(cursor2);
   } else {
      SDL_SetCursor(cursor);
   }
}


void custom_cursor_free(void)
{
   SDL_ShowCursor(0);
   SDL_FreeCursor(cursor);
}


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
