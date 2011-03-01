#ifndef __LEVEL_H
#define __LEVEL_H

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
 * Defines and structs
 * ----------------------------------------------
 */

/* Target properties */
struct prop_t {
   struct sprite_t *spr;
   /* --> Pending movement variables */
   /* Pend at all? */
   bool pend;
   /* true = pend origo below animal else above animal */
   bool pend_invert;
   /* curr_angle = start_angle + amp_angle * sin(age * c_angle) */
   int pend_fi_amp;
   /* Coefficient deciding the "speed" of the pending */
   float pend_fi_c;
   int pend_offset;
   /* Length of pendulum */
   int pend_l;

   /* --> Horizontal movement */
   bool horizontal;
   float hor_speed;

   /* --> Horizontal pending */
   bool hor_pend;
   float hor_pend_amp;
   float hor_pend_c;
   /* Number of frames to stay out (for oneshot) */
   int hor_end_frames;

   /* Number of frames animal lives each life */
   int max_age;

   /* Distance from upper right corner to center of target */
   int targ_x, targ_y;

   /* Spawn points. Each animal has a list of possible spawn points.
    * Randomize sx,sy point with values from these lists.
    */
   int spawn_x_points[4];
   int spawn_y_points[4];
   /* Number of points in lists */
   int n_x_points;
   int n_y_points;

   /* Layers in front of target.
    * End list with -1
    */
   int layers[8];

   /* Are waves in front of target? */
   bool wave1;
   bool wave2;

   /* Score */
   int base_points;
   /* Target radius for outer, middle or inner hit */
   int targ_r_outer;  /* base_points * 1.2 */
   int targ_r_middle; /* base_points * 1.5 */
   int targ_r_inner;  /* base_points * 2.0 */

   /* Target rotation, cx,cy = coordinates (relative to center). */
   int targ_cx, targ_cy;
   /* r, fi = radius and angle of target relative center. */
   float targ_r;
   float targ_fi;

   /* Flag coordinates from upper left */
   int flag_x, flag_y;
   int flag_cx, flag_cy;
   float flag_r;
   float flag_fi;
   /* Start angle for flag */
   int flag_extra_fi;
};


enum target_state {
   Dead = 0,
   /* Living animals, not oneshot */
   Alive,
   /* Oneshot animals */
   Coming, Here, Going,
   /* Dying animals */
   Hit
};

enum goldstar_t {
   None = 0,
   Star,
   Star2,
   Stars,
   Skull
};


#define NUM_TARGETS 7

struct target_t {
   /* Properties */
   struct prop_t prop;

   /* Sprite showing current score */
   struct sprite_t scorespr;
   int scoreangle;
   /* Goldstars behind score? */
   enum goldstar_t goldstar;

   /* Animal state variables */

   /* Dead/Living/Hit */
   enum target_state state;
   /* Init to 0 when spawning target. Increase each frame. */
   int age;
   /* Age when target was hit */
   int hit_age;
   /* Spawn coordinates. */
   int sx, sy;
   /* Current coordinates */
   int x, y;
   /* Transformation translation, at least one of:
    * - horizontal movement
    * - grandfather clock pending
    * - horizontal pending (oneshot)
    */
   float tx, ty;
   /* Sum angle of all rotations */
   float tfi;
   /* Rotated target coordinates (from center of sprite) */
   int targ_tx, targ_ty;
   /* Flag connection point */
   int flag_tx, flag_ty;
   /* Zoom */
   float zoom;
   /* Have flag? */
   bool white;
   bool yellow;
   /* Have bonus target? */
   bool bonus;
};


#define NUM_LAYERS 10

enum lnames_e {
   L_bg0 = 0,
   L_right_deco,
   L_bg1,
   L_left_deco,
   L_bg2,
   L_top,
   L_left,
   L_right,
   L_bottom,
   L_background
};

struct layer_t {
   struct sprite_t *spr;
   int x;
   int y;
};


/* ----------------------------------------------
 * Exported variables and functions from level.c
 * ----------------------------------------------
 */

extern int bg_x;
extern int bg_y;
extern struct target_t targets[];
extern struct layer_t layers[];
bool load_level(const char *filename);
void free_level(void);


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

#endif  /* __LEVEL_H */
