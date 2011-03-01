#ifndef __TRICKMATH_H
#define __TRICKMATH_H

/**
 * @file trickmath.h
 * @brief Tricky math include.
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
 * Macros
 * ----------------------------------------------
 */
#define SQRTFAST(x) ((x) * Q_rsqrt(x))


/* ----------------------------------------------
 * Exported functions from trickmath.c
 * ----------------------------------------------
 */

float Q_rsqrt(float number);
inline float u8sin(unsigned char v);
inline float u8cos(unsigned char v);
inline float u8sinf(float v);
inline float u8cosf(float v);


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

#endif  /* __TRICKMATH_H */
