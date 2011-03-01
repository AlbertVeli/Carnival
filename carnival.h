#ifndef __CARNIVAL_H
#define __CARNIVAL_H

/**
 * @file carnival.h
 * @brief Main include file.
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

/* ----------------------------------------------
 * Global includes
 * ----------------------------------------------
 */

/* For bool, true and false */
#include <stdbool.h>
/* For fprintf in WARN macro */
#include <stdio.h>

/* Target is specified in config.h */
#include "config.h"

/* For now SDL is the only target, this may change.
 * If more targets are added, move all SDL files into
 * separate subdir.
 */
#ifdef USE_SDL
#include <SDL.h>
   #include "sdl_video.h"
   #include "sdl_sprite.h"
   #include "sdl_cursor.h"
   #include "sdl_event.h"
#else
   #error No supported target found
#endif


/* ----------------------------------------------
 * Global macros
 * ----------------------------------------------
 */

#define WARN(format, arg...) { fprintf(stderr, "* Warning <%s:%d> %s: " format "\n", __FILE__, __LINE__, __FUNCTION__, ## arg); }
#ifdef DEBUG
#define DBG(format, arg...) { printf("DEBUG: %s: " format "\n", __FUNCTION__, ## arg); }
#else
#define DBG(format, arg...) {}
#endif
#ifndef UNUSED
#define UNUSED __attribute__ ((unused))
#endif

/* __builtin_expect was introduced in gcc-2.96 */
#if defined (__GNUC__) && (__GNUC__ >= 3)
/* __builtin_expect is defined by gcc */
#else
/* Expand to the expression itself without optimization */
#define __builtin_expect(x, expected_value) (x)
#endif

/* Use these to optimize expressions that are likely/unlikely to be TRUE */
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


/* ----------------------------------------------
 * Exported functions from carnival.c
 * ----------------------------------------------
 */

/* Callbacks for keys and mouse */
void escape_pressed(void);
void pause_pressed(void);
void mouse_clicked(int x, int y);


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

#endif  /* __CARNIVAL_H */
