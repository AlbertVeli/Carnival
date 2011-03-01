/**
 * @file trickmath.c
 * @brief Tricky math routines.
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
 * Local variables
 * ----------------------------------------------
 */

/* Calculated with tools/sin.c */
static float sintab[65] = {
   0.000000, 0.024541, 0.049068, 0.073565, 
   0.098017, 0.122411, 0.146730, 0.170962, 
   0.195090, 0.219101, 0.242980, 0.266713, 
   0.290285, 0.313682, 0.336890, 0.359895, 
   0.382683, 0.405241, 0.427555, 0.449611, 
   0.471397, 0.492898, 0.514103, 0.534998, 
   0.555570, 0.575808, 0.595699, 0.615232, 
   0.634393, 0.653173, 0.671559, 0.689541, 
   0.707107, 0.724247, 0.740951, 0.757209, 
   0.773010, 0.788346, 0.803208, 0.817585, 
   0.831470, 0.844854, 0.857729, 0.870087, 
   0.881921, 0.893224, 0.903989, 0.914210, 
   0.923880, 0.932993, 0.941544, 0.949528, 
   0.956940, 0.963776, 0.970031, 0.975702, 
   0.980785, 0.985278, 0.989177, 0.992480, 
   0.995185, 0.997290, 0.998795, 0.999699, 
   1.000000
};


/* ----------------------------------------------
 * Exported functions
 * ----------------------------------------------
 */

/* Taken from Quake III source code */
float Q_rsqrt(float number)
{
   long i;
   float x2, y;
   const float threehalfs = 1.5F;

   x2 = number * 0.5F;
   y  = number;
   i  = *(long *)&y;                      // evil floating point bit level hacking
   i  = 0x5f3759df - (i >> 1);            // what the fuck?
   y  = * (float *)&i;
   y  = y * (threehalfs - (x2 * y * y));  // 1st iteration

   return y;
}


/* Lookup in sintab (taken from Rockbox source code) */
inline float u8sin(unsigned char v)
{
   if (v < 65) {
      return sintab[v];
   } else if (v < 129) {
      return sintab[128 - v];
   } else if (v < 193) {
      return -sintab[v - 128];
   } else {
      return -sintab[256 - v];
   }
}


inline float u8cos(unsigned char v)
{
   return u8sin(v - 64);
}


/* interpolate from sintab */
inline float u8sinf(float v)
{
   float s1 = u8sin((unsigned char)v);
   float s2 = u8sin((unsigned char)v + 1);
   float d = s2 - s1;
   return s1 + ((float)v - ((int)v)) * d;
}


inline float u8cosf(float v)
{
   return u8sinf(v - 64);
}


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
