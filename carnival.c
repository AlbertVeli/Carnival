/**
 * @file carnival.c
 * @brief Clone of the skill shooting game carnival shootout.
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

/* TODO:
 *
 * - Add stop frame somewhere in age where target might stop x frames (for peekaboo and bear).
 *
 * - Use alpha channel for animals. Examine cpu usage for alpha compared to colorkey.
 *
 * - Add sound.
 *
 * - Add level ending. Make sure each level is fair. The same number of
 *   animals of each type (at least each score group) should appear in each
 *   game, but the order should be randomized.
 *
 * - When all the above is finished, add more levels.
 *
 * - Split carnival.c into more files if it grows too large. Make sure the SDL-stuff is
 *   well separated so it can be easily replaced by other libraries.
 *
 * - Redraw the game with custom graphics before releasing it. The released game must not
 *   have ANY ripped graphics from carnival shootout.
 *
 * - Checkout SDL-net for multiplayer action, hiscore and so on.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "carnival.h"
#include "trickmath.h"
#include "level.h"


/* ----------------------------------------------
 * Local defines and structs
 * ----------------------------------------------
 */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FPS 60

/* Use 256 "degree" circle */
#define deg2rad(x) (2 * M_PI * (x) / 256.0f)

#define target_w(a) sprite_width(*((a)->prop.spr))
#define target_h(a) sprite_height(*((a)->prop.spr))

enum magazine_state {
   Ok,
   Reloading
};

struct wave_t {
   /* Coordinates of first wave segment */
   int x, y;
   /* Dimensions of each segment */
   int width;
   int height;
};


/* Flag coordinates */
struct flag_t {
   struct sprite_t *sprite;
   /* Distance from upper left to flag connection */
   int flag_x, flag_y;
   /* Coordinates relative center */
   int flag_cx, flag_cy;
   /* Radius and angle of basepoint relative center */
   float flag_r;
   float flag_fi;
   /* Rotated connection point relative center */
   int flag_tx, flag_ty;
};

struct spritestruct_t {
   struct sprite_t *spr;
   char *png;
};


/* ----------------------------------------------
 * Local variables
 * ----------------------------------------------
 */
static int quit;
static int pause = 0;
static int level = 0;
/* Coordinates (x,y) relative "right" sprite */
static int hole_coords[12] = { 78, 342, 93, 368, 77, 395, 47, 395, 32, 369, 48, 342 };
static int mag_bullets;
static enum magazine_state mag_state;
static int mag_delay;
static int spawned_targets;
static int time_left;

static int bonusangle;
static int bonusframe;
static float bonuszoom;
static bool bonusscore;

static struct sprite_t bonusspr;
static struct sprite_t wave;
static struct sprite_t hole;
static struct sprite_t numbers;
static struct sprite_t bignum;
static struct sprite_t bignumr;
static struct sprite_t goldstar;
static struct sprite_t goldstar2;
static struct sprite_t goldstars;
static struct sprite_t skull;
static struct sprite_t white_flag;
static struct sprite_t yellow_flag;
static struct sprite_t ball;

static struct spritestruct_t sprites[] = {
   { &bonusspr,    "png/skull.png",       },
   { &wave,        "png/wave.png",       },
   { &hole,        "png/bullethole.png", },
   { &numbers,     "png/smallnum.png",   },
   { &bignum,      "png/bignum.png",     },
   { &bignumr,     "png/bignumr.png",    },
   { &goldstar,    "png/goldstar.png",   },
   { &goldstar2,   "png/goldstar2.png",  },
   { &goldstars,   "png/goldstars.png",  },
   { &skull,       "png/skull.png",      },
   { &white_flag,  "png/flag.png",       },
   { &yellow_flag, "png/yellowflag.png", },
   { &ball,        "png/ball.png",       },
   { NULL, NULL }
};

/* White flag */
struct flag_t wflag = {
   &white_flag,
   1, 55,
   /* Init rest after loading sprite */
   0, 0, 0.0f, 0.0f, 0, 0
};

struct flag_t yflag = {
   &yellow_flag,
   1, 55,
   /* Init rest after loading sprite */
   0, 0, 0.0f, 0.0f, 0, 0
};

struct flag_t bonusball = {
   &ball,
   15, 28,
   /* Init rest after loading sprite */
   0, 0, 0.0f, 0.0f, 0, 0
};

/* Hardcode params for waves */
#define WAVE_X 63
#define WAVE_Y 445
#define WAVE_AMP_X (68 / 2)
#define WAVE_AMP_Y (20 / 2)
#define WAVE_SPACING 25
#define WAVES 5
#define NUM_WAVES 2
static struct wave_t waves[NUM_WAVES];

static int total_score;


/* ----------------------------------------------
 * Local functions
 * ----------------------------------------------
 */


static bool new_level(void)
{
   char levelstr[32];
   bool ret = false;

   level++;

   snprintf(levelstr, 32, "levels/level%d.txt", level);

   if (!load_level(levelstr)) {
      /* Parse error or all levels finished */
      free_level();
      goto out;
   }
   spawned_targets = 0;
   time_left = 40;
   bonusscore = false;

   ret = true;

out:

   return ret;
}


static void count_time(void)
{
   static int frame_counter = FPS;

   if (unlikely(--frame_counter <= 0)) {
      time_left--;
      frame_counter = FPS;
   }
}


/* Init new target */
static void spawn_target(int target_num, bool bonus)
{
   int num;
   struct target_t *a = &targets[target_num];
   struct flag_t *f;

   if (unlikely(a->state != Dead)) {
      return;
   }

   /* Keep count of number of spawned targets.
    * TODO: Diffrentiate between types of targets?
    */
   spawned_targets++;

   sprite_reset(a->prop.spr);

   /* Init target state variables */
   num = (int)(((a->prop.n_x_points - 1) * (rand() / (float)RAND_MAX)) + 0.5);
   a->sx = bg_x + a->prop.spawn_x_points[num];
   num = (int)(((a->prop.n_y_points - 1) * (rand() / (float)RAND_MAX)) + 0.5);
   a->sy = bg_y + a->prop.spawn_y_points[num];
   a->tx = 0;
   a->ty = 0;
   a->age = 0;
   a->state = Alive;

   /* Bonus target? */
   if (unlikely(bonus)) {
      a->white = false;
      a->yellow = false;
      a->bonus = true;
   } else {
      a->bonus = false;
      /* Flag? */
      if (unlikely((int)(((10) * (rand() / (float)RAND_MAX)) + 0.5) > 8)) {
         a->white = true;
      } else {
         a->white = false;
         if (unlikely((int)(((10) * (rand() / (float)RAND_MAX)) + 0.5) > 8)) {
            a->yellow = true;
         } else {
            a->yellow = false;
         }
      }
   }

   /* Init tx,ty to cx,cy without rotation */
   a->targ_tx = a->prop.targ_cx;
   a->targ_ty = a->prop.targ_cy;
   /* And flag */
   if (a->white || a->yellow || a->bonus) {
      a->flag_tx = a->prop.flag_cx;
      a->flag_ty = a->prop.flag_cy;
      /* White flag */
      if (a->white) {
         f = &wflag;
      } else if (a->yellow) {
         f = &yflag;
      } else {
         f = &bonusball;
      }
      f->flag_tx = f->flag_cx;
      f->flag_ty = f->flag_cy;
   }
}


/* Basic horizontal movement */
static inline float horizontal_movement(float speed, int age)
{
   return speed * age;
}


/* In/Out from side. Stay out hor_end_frames and out position */
static inline float horizontal_pending(float amp, float c, int age)
{
   /* TODO: Stay out hor_end_frames frames at endpoint */
   return amp * u8sinf(c * age);
}


/* Grandfather clock pending. Send in
 * whole animal but only update tx and ty.
 *
 * Equation:
 * fi = fi_amp * sin(fi_c * age)
 * yt = l * cos(fi)
 * xt = l * sin(fi)
 *
 *        o
 *       /|\
 *    l / | \
 *     /\ | /\ <- 2 * fi_amp
 *    O  -|-  O
 *        O
 *
 */
static void grandfather_pending(struct target_t *a)
{
   a->tfi += a->prop.pend_fi_amp * u8sinf(a->prop.pend_fi_c * a->age + a->prop.pend_offset);
   /* a->tfi += a->prop.pend_fi_amp * sin(deg2rad(a->prop.pend_fi_c * a->age + a->prop.pend_offset)); */
   if (a->prop.pend_invert) {
      a->tx += -1 * a->prop.pend_l * u8sinf(a->tfi);
      a->ty += -1 * a->prop.pend_l * u8cosf(a->tfi);
   } else {
      a->tx += a->prop.pend_l * u8sinf(a->tfi);
      a->ty += a->prop.pend_l * u8cosf(a->tfi);
   }
}


/**
 * Move targets on screen and spawn new targets
 * now and then.
 *
 * Return true if level is finished, else false.
 */
static bool move_targets(void)
{
   bool rot;
   int i, target;
   struct target_t *a;

   /* New animal once each 2s (don't spawn bonus targets here) */
   if (unlikely(rand() < (int)(((unsigned long)RAND_MAX + 1) / (FPS * 2)))) {
      spawn_target(((NUM_TARGETS - 2) * (rand() / (float)RAND_MAX)) + 0.5, false);
   }
/*    spawn_target(NUM_TARGETS - 1, true); */

   for (target = 0; target < NUM_TARGETS; target++) {

      a = &targets[target];

      if (likely(a->state == Dead)) {
         continue;
      }

      a->age++;
      if (likely(a->state != Hit)) {
         if (unlikely(a->age > a->prop.max_age)) {
            a->state = Dead;
            DBG("Animal died of old age");
            continue;
         }
      }

      /* Reset translations each frame */
      a->tx = 0;
      a->ty = 0;
      a->tfi = 0;
      rot = false;
      /* Horizontal movement */
      if (likely(a->prop.horizontal)) {
         a->tx += horizontal_movement(a->prop.hor_speed, a->age);
      }
      /* Horizontal (oneshot) pending */
      if (unlikely(a->prop.hor_pend)) {
         a->tx += horizontal_pending(a->prop.hor_pend_amp, a->prop.hor_pend_c, a->age);
      }
      /* Grandfather clock pending */
      if (likely(a->prop.pend)) {
         grandfather_pending(a);
         rot = true;
      }

      /* Rotate */
      if (likely(rot || a->state == Hit)) {

         if (a->state == Hit) {
            if (unlikely(a->age - a->hit_age > 40)) {
               a->state = Dead;
               continue;
            }
            a->tfi += ((a->age - a->hit_age) * -2 * u8cosf(a->hit_age));
            for (a->zoom = 1.0f, i = a->hit_age; i < a->age; i++) {
               a->zoom *= 0.97;
            }
            sprite_rotozoom(&(a->scorespr), a->scoreangle, 0.9 + (1 - a->zoom) * 0.5);
         } else {
            a->zoom = 1.0f;
         }
         sprite_rotozoom(a->prop.spr, -a->tfi, a->zoom);

         /* Adjust for size difference between spr_trans and spr */
         a->tx -= a->prop.spr->delta_w >> 1;
         a->ty -= a->prop.spr->delta_h >> 1;

         /* Formula for rotated target:
          * x = r * cos(-fi)
          * y = r * sin(-fi)
          */
         if (likely(a->prop.targ_cx != 0 || a->prop.targ_cy != 0)) {
            /* Calculate fi for target rotation */
            a->targ_tx = -1 * a->prop.targ_r * u8cosf(-(a->prop.targ_fi + a->tfi));
            a->targ_ty = -1 * a->prop.targ_r * u8sinf(-(a->prop.targ_fi + a->tfi));
         }
         if (a->white || a->yellow) {
            if (a->prop.flag_cx != 0 || a->prop.flag_cy != 0) {
               a->flag_tx = -1 * a->prop.flag_r * u8cosf(-(a->prop.flag_fi + a->tfi));
               a->flag_ty = -1 * a->prop.flag_r * u8sinf(-(a->prop.flag_fi + a->tfi));
            }
         }
      }

      /* Calculate final position */
      a->x = a->sx + a->tx;
      a->y = a->sy + a->ty;
      sprite_set_pos(*(a->prop.spr), a->x, a->y);
   }

   if (unlikely(bonusscore)) {
      if (unlikely(frames - bonusframe > 40)) {
         bonusscore = false;
      } else {
         for (bonuszoom = 1.0f, i = bonusframe; i < (int)frames; i++) {
            bonuszoom *= 0.97;
         }
         sprite_rotozoom(&bonusspr, bonusangle, 0.9 + (1 - bonuszoom) * 0.5);
      }
   }

   /* End level when time is out */
   return (time_left <= 0);
}


/* Draw a wave with WAVES segments starting at (x, y) */
static inline void draw_wave(struct wave_t *w)
{
   int i;
   for (i = 0; i < WAVES; i++) {
      sprite_set_pos(wave, w->x + i * w->width, w->y);
      sprite_blit(wave);
   }
}


static inline bool hit_wave(int x, int y, struct wave_t *w)
{
   int i;

   for (i = 0; i < WAVES; i++) {
      sprite_set_pos(wave, w->x + i * w->width, w->y);
      if (unlikely(wave.sprite_collide(&wave, x, y))) {
         return true;
      }
   }
   return false;
}


/* Check if (x,y) hit layers in front of a */
static bool hit_layers(struct target_t *a, int x, int y)
{
   int i = 0;
   struct sprite_t *sprp;

   /* Loop through layers list */
   while (a->prop.layers[i] >= 0) {
      sprp = layers[a->prop.layers[i]].spr;
      if (unlikely(sprp->sprite_collide(sprp, x, y))) {
         return true;
      }
      i++;
   }
   /* Check waves */
   if (unlikely(a->prop.wave1 && hit_wave(x, y, &waves[0]))) {
      return true;
   }
   if (unlikely(a->prop.wave2 && hit_wave(x, y, &waves[1]))) {
      return true;
   }
   return false;
}


static void draw_number(int x, int y, unsigned int num)
{
   int dx;
   int nums[16];
   int numw = sprite_width(numbers) / 10;
   int i = 0;

   do {
      nums[i] = num % 10;
      num /= 10;
      i++;
   } while (num > 0);

   /* Center number, start at x - half width */
   dx = x - ((i * (numw + 2)) >> 1);

   while (i > 0) {
      i--;
      sprite_blit_part(&numbers, nums[i] * numw, 0, dx, y, numw, sprite_height(numbers));
      dx += numw + 2;
   }
}


static void rotate_flag(struct flag_t *f, float tfi, float zoom)
{
   sprite_rotozoom(f->sprite, -tfi, zoom);
   f->flag_tx = -1 * f->flag_r * u8cosf(-(f->flag_fi + tfi));
   f->flag_ty = -1 * f->flag_r * u8sinf(-(f->flag_fi + tfi));
   /* Recalculate tx,ty as distance from upper right */
   f->flag_tx +=(sprite_width(*(f->sprite)) >> 1);
   f->flag_ty +=(sprite_height(*(f->sprite)) >> 1);
}


static void draw_target(struct target_t *a)
{
   struct flag_t *f;

   if (a->state != Dead) {
      /* Calculate flagpos */
      if (a->white || a->yellow || a->bonus) {
         if (a->white) {
            f = &wflag;
         } else if (a->yellow) {
            f = &yflag;
         } else {
            f = &bonusball;
         }
         if (a->bonus) {
            rotate_flag(f, -a->tfi, a->zoom);
         } else {
            rotate_flag(f, a->tfi + a->prop.flag_extra_fi, a->zoom);
         }
         sprite_set_pos(*(f->sprite),
                        a->x + ((target_w(a) >> 1) + a->flag_tx - f->flag_tx) * a->zoom,
                        a->y + ((target_h(a) >> 1) + a->flag_ty - f->flag_ty) * a->zoom);
         sprite_blit(*(f->sprite));
      }

      sprite_blit(*(a->prop.spr));

/*       /\* Just for test, check that target is rotated ok. *\/ */
/*       { */
/*          SDL_Rect r; */
/*          r.x = a->x + (target_w(a) >> 1) + a->targ_tx - 7; */
/*          r.y = a->y + (target_h(a) >> 1) + a->targ_ty - 7; */
/*          r.w = 14; */
/*          r.h = 14; */
/*          SDL_FillRect(screen, &r, 0); */
/*       } */
   }
}


static void draw_layers(void)
{
   static int period = 0;
   int i;

   sprite_blit(*(layers[L_bg0].spr));

   /* Slot 1, penguin */

   draw_target(&targets[5]);

   sprite_blit(*(layers[L_right_deco].spr));
   sprite_blit(*(layers[L_bg1].spr));

   /* Slot 2, seal (bonus), hen */

   draw_target(&targets[6]);
   draw_target(&targets[4]);

   sprite_blit(*(layers[L_left_deco].spr));
   sprite_blit(*(layers[L_bg2].spr));

   /* Slot 3, dolphin, pelican */

   draw_target(&targets[3]);
   draw_target(&targets[2]);

   waves[0].x = bg_x + WAVE_X + WAVE_AMP_X + WAVE_AMP_X * u8sin(-period * 0.69);
   waves[0].y = bg_y + WAVE_Y + WAVE_AMP_Y * u8sin(period * 0.41);
   draw_wave(&waves[0]);

   /* Slot 4, fish */

   draw_target(&targets[1]);

   waves[1].x = bg_x + WAVE_X + WAVE_AMP_X + WAVE_AMP_X * u8sin(period * 0.59);
   waves[1].y = bg_y + WAVE_Y + WAVE_AMP_Y * u8sin(period * 0.63) + WAVE_SPACING;
   draw_wave(&waves[1]);


   /* Slot 5. Bird */

   draw_target(&targets[0]);

   sprite_blit(*(layers[L_top].spr));
   sprite_blit(*(layers[L_left].spr));
   sprite_blit(*(layers[L_right].spr));
   sprite_blit(*(layers[L_bottom].spr));

   /* Draw hitscores */
   for (i = 0; i < NUM_TARGETS; i++) {
      if (unlikely(targets[i].state == Hit)) {
         sprite_blit(targets[i].scorespr);
      }
   }

   /* Hitscore for bonus ball */
   if (unlikely(bonusscore)) {
      sprite_blit(bonusspr);
   }

   /* Reload of magazine (with delays) */
   if (unlikely(mag_state == Reloading)) {
      if (likely(mag_delay > 0)) {
         mag_delay--;
      } else {
         mag_delay = 10;
         mag_bullets++;
         if (unlikely(mag_bullets == 6)) {
            /* Set normal cursor again */
            custom_cursor_alternative(false);
            mag_state = Ok;
         }
      }
   }
   /* Draw bullet holes */
   for (i = 0; i < 6 - mag_bullets; i++) {
      sprite_set_pos(hole,
                     layers[L_right].spr->rect.x + hole_coords[i << 1],
                     layers[L_right].spr->rect.y + hole_coords[(i << 1) + 1]);
      sprite_blit(hole);
   }

   /* Draw score */
   draw_number(92, 244, total_score);

   /* Draw time left */
   draw_number(92, 299, time_left);

   period++;
}


/* Init flag properties */
static void init_flag(struct flag_t *f)
{
   /* cx,cy */
   f->flag_cx = f->flag_x - (sprite_width(*(f->sprite)) >> 1);
   f->flag_cy = f->flag_y - (sprite_height(*(f->sprite)) >> 1);
   /* radius */
   f->flag_r = sqrt(f->flag_cx * f->flag_cx + f->flag_cy * f->flag_cy);
   /* angle */
   f->flag_fi = 256.0 / (2.0 * M_PI) * acos(f->flag_cx / f->flag_r);
   if (f->flag_cy < 0) {
      f->flag_fi = 256.0 - f->flag_fi;
   }
}


/* Exits on failure */
static void game_init(int width, int height)
{
   int i;

   /* video_init exits on failure */
   video_init(width, height);
   /* Set framerate (will be correct if computer is fast enough) */
   video_set_preferred_framerate(FPS);
   custom_cursor_init();
   quit = false;

   i = 0;
   while (sprites[i].spr) {
      if(!sprite_load_from_png(sprites[i].spr, sprites[i].png, true)) {
         WARN("sprite_load_from_png failed for %s", sprites[i].png);
         exit(1);
      }
      i++;
   }

   /* Init flags */
   init_flag(&wflag);
   init_flag(&yflag);
   init_flag(&bonusball);

   /* Init num bullets in magazine */
   mag_bullets = 6;
   mag_state = Ok;

   /* Init waves */
   for (i = 0; i < NUM_WAVES; i++) {
      waves[i].width = sprite_width(wave);
      waves[i].height = sprite_height(wave);
   }

   /* Start with score 0 ;-) */
   total_score = 0;

   /* Seed random number generator */
   srand(time(NULL));
}


static void game_cleanup(void)
{
   int i = 0;

   while (sprites[i].spr) {
      sprite_free(sprites[i].spr);
      i++;
   }

   custom_cursor_free();
}


static void set_bonusspr(int posx, int posy, unsigned int score)
{
   int numw;
   int y;
   int dx, i, digit;

   /* Erase scorespr and reset width and height */
   sprite_reset_dimensions(bonusspr);
   sprite_erase(&bonusspr);
   sprite_blit_dest(goldstars, bonusspr);
   /* Calc coordinates */
   y = (sprite_height(bonusspr) - sprite_height(bignum)) >> 1;
   numw = sprite_width(bignum) / 10;
   dx = (sprite_width(bonusspr) - 3 * numw) >> 2;
   for (i = 2; i >= 0; i--) {
      digit = score % 10;
      score /= 10;
      sprite_blit_part_dest(&bignum, &bonusspr, digit * numw, 0,
                            i * sprite_width(bonusspr) / 3 + dx, y,
                            numw, sprite_height(bignum));
   }
   sprite_set_pos(bonusspr, posx, posy);
   bonusangle = (int)((20 * (rand() / (float)RAND_MAX)) + 0.5) - 10;
   bonusscore = true;
}


static void set_scorespr(struct target_t *a, unsigned int score)
{
   int numw;
   int y;
   int dx, i, digit;
   struct sprite_t *nums = &bignum;

   /* Erase scorespr and reset width and height */
   sprite_reset_dimensions(a->scorespr);
   sprite_erase(&(a->scorespr));
   if (a->goldstar == Stars) {
      sprite_blit_dest(goldstars, a->scorespr);
   } else if (a->goldstar == Star) {
      sprite_blit_dest(goldstar, a->scorespr);
   } else if (a->goldstar == Star2) {
      sprite_blit_dest(goldstar2, a->scorespr);
   } else if (a->goldstar == Skull) {
      sprite_blit_dest(skull, a->scorespr);
      nums = &bignumr;
   }
   /* Calc coordinates */
   y = (sprite_height(a->scorespr) - sprite_height(bignum)) >> 1;
   numw = sprite_width(bignum) / 10;
   dx = (sprite_width(a->scorespr) - 3 * numw) >> 2;
   for (i = 2; i >= 0; i--) {
      digit = score % 10;
      score /= 10;
      sprite_blit_part_dest(nums, &(a->scorespr), digit * numw, 0,
                            i * sprite_width(a->scorespr) / 3 + dx, y,
                            numw, sprite_height(bignum));
   }
   sprite_set_pos(a->scorespr, a->x, a->y);
   a->scoreangle = (int)((20 * (rand() / (float)RAND_MAX)) + 0.5) - 10;
}


/* ----------------------------------------------
 * Exported functions from carnival.c
 * ----------------------------------------------
 */

/* Callback for escape key, called from handle_events() */
void escape_pressed(void)
{
   quit = true;
}


/* Callback for pause key (p), called from handle_events() */
void pause_pressed(void)
{
   pause = 1 - pause;
}


/* Callback for mouseclick, called from handle_events() */
void mouse_clicked(int x, int y)
{
   int bullx, bully;
   int r2, c1, c2;
   int i;
   int score;
   struct target_t *a;
   int speed_bonus;
   static int last_hit = 0;
   bool bonus = false;

   /* Check magazine */
   if (unlikely(mag_bullets == 0)) {
      mag_state = Reloading;
      mag_delay = 0;
   }
   if (unlikely(mag_state == Reloading)) {
      return;
   }

   /* Mag state is Ok, fire */
   mag_bullets--;
   if (unlikely(mag_bullets == 0)) {
      /* Set alternative cursor marking empty mag */
      custom_cursor_alternative(true);
   }

   /* Collission detection */
   for (i = 0; i < NUM_TARGETS; i++) {
      a = &targets[i];
      score = 0;
      if (unlikely(a->state > Dead && a->state < Hit)) {

         /* Calculate speed bonus according to formula in doc/game_rules.jsp */
         speed_bonus = 10 - 6 * ((frames - last_hit) / (float)FPS);
         if (speed_bonus < 0) {
            speed_bonus = 0;
         }

         if (unlikely(a->bonus)) {
            /* Check collission against ball */
            if ((bonusball.sprite)->sprite_collide(bonusball.sprite, x, y)) {
               if (unlikely(hit_layers(a, x, y))) {
                  DBG("Hit layer in front of bonusball");
               } else {
                  /* Hit ball */
                  score += 500;
                  a->bonus = false;
                  bonusframe = frames;
                  set_bonusspr(bonusball.sprite->rect.x - sprite_width(*(bonusball.sprite)),
                               bonusball.sprite->rect.y - sprite_height(*(bonusball.sprite)), score);
               }
            }
         }

         bullx = a->x + (target_w(a) >> 1) + a->targ_tx;
         bully = a->y + (target_h(a) >> 1) + a->targ_ty;
         /* Pythagoras says: c1 * c1 + c2 * c2 = r * r
          *   x,y
          *    .
          *    |\ r
          * c2 | \
          *    |__. bullx, bully
          *     c1
          */
         c1 = x - bullx;
         c2 = y - bully;
         r2 = (c1 * c1 + c2 * c2);
         a->goldstar = None;
         /* Check if target circle hit, prop->targ_r_* values are already squared. */
         if (r2 <= a->prop.targ_r_outer) {
            if (unlikely(hit_layers(a, x, y))) {
               DBG("Hit layer in front of target circle");
            } else {
               /* Inside target circle, check if yellow flag */
               if (unlikely(a->yellow)) {
                  bonus = true;
               }
               if (r2 <= a->prop.targ_r_middle) {
                  if (r2 <= a->prop.targ_r_inner) {
                     score += a->prop.base_points * 2.0 + speed_bonus;
                     a->goldstar = Stars;
                     DBG("Inner (%d, %.2f), score = %d", r2, SQRTFAST(r2), score);
                  } else {
                     score += a->prop.base_points * 1.5 + speed_bonus;
                     a->goldstar = Star2;
                     DBG("Middle (%d, %.2f), score = %d", r2, SQRTFAST(r2), score);
                  }
               } else {
                  score += a->prop.base_points * 1.2 + speed_bonus;
                  a->goldstar = Star;
                  DBG("Outer (%d, %.2f), score = %d", r2, SQRTFAST(r2), score);
               }
               a->state = Hit;
               a->hit_age = a->age;
               last_hit = frames;
               if (unlikely(a->white)) {
                  a->goldstar = Skull;
               }
               set_scorespr(a, score);
            }
         } else if (a->prop.spr->sprite_collide(a->prop.spr, x, y)) {
            /* Animal hit outside target circle */
            if (unlikely(hit_layers(a, x, y))) {
               DBG("Hit layer in front of animal, outside target circle");
            } else {
               score += a->prop.base_points + speed_bonus;
               DBG("Outside target circle (%.2f), score = %d", SQRTFAST(r2), score);
               a->state = Hit;
               a->hit_age = a->age;
               last_hit = frames;
               if (unlikely(a->white)) {
                  a->goldstar = Skull;
               }
               set_scorespr(a, score);
            }
         } else {
            /* DBG("Miss"); */
         }
         /* Add score (if any) to total_score */
         if (score > 0) {
            if (unlikely(a->white)) {
               /* Hit white flag target */
               total_score -= score;
               if (total_score < 0) {
                  total_score = 0;
               }
            } else {
               total_score += score;
            }
            /* Break loop if target is hit */
            break;
         }
      }
   }
   /* Only one bonus animal possible per click */
   if (unlikely(bonus)) {
      /* Spawn bonus target */
      spawn_target(NUM_TARGETS - 1, true);
   }
}


int main(void)
{
   /* Initialize game */
   game_init(800, 600);

   if (!new_level()) {
      exit(1);
   }

   /* Main game loop */
   while (!quit) {

      /* Check for mouse and key events */
      handle_events();

      /* Count down time once every second */
      count_time();

      if (likely(!pause)) {
         if(unlikely(move_targets())) {
            /* Level finished */
            free_level();
            if (!new_level()) {
               quit = true;
            }
         } else {
            draw_layers();
         }
      }

      /* Show new frame */
      video_flip();

      /* Sleep until next frame */
      video_fps_sleep();
   }

   video_average_fps();

   /* Game finished. */
   printf("TOTAL SCORE: %d\n", total_score);

   game_cleanup();

   exit(0);
}


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
