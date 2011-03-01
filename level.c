/**
 * @file level.c
 * @brief Parse config file.
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#include "carnival.h"
#include "sdl_sprite.h"
#include "level.h"

/* Targets */
static struct sprite_t tspr[NUM_TARGETS];
struct target_t targets[NUM_TARGETS];

/* Layers */
static struct sprite_t lspr[NUM_LAYERS];
struct layer_t layers[NUM_LAYERS];

int bg_x;
int bg_y;

/* ------------- */

enum sect_t {
   S_Undef = 0,
   S_Layers,
   S_Target
};

static char *sections[] = {
   "[layers]",
   "[target]",
   NULL
};


/* Datatypes */
enum data_t {
   Bool,     /* true / false */
   Int,      /* Integer */
   Inta,     /* Integer array */
   Float,    /* Float */
   String    /* String */
};


/* Layer names */
struct layer_name_t {
   char *name;
   enum data_t type;
};


/* Name-datatype associations */
static struct layer_name_t lnames[] = {
   { "bg0_s",          String }, /* 0 */
   { "bg0_pos",        Inta   },
   { "right_deco_s",   String }, /* 1 */
   { "right_deco_pos", Inta   },
   { "bg1_s",          String }, /* 2 */
   { "bg1_pos",        Inta   },
   { "left_deco_s",    String }, /* 3 */
   { "left_deco_pos",  Inta   },
   { "bg2_s",          String }, /* 4 */
   { "bg2_pos",        Inta   },
   { "top_s",          String }, /* 5 */
   { "top_pos",        Inta   },
   { "left_s",         String }, /* 6 */
   { "left_pos",       Inta   },
   { "right_s",        String }, /* 7 */
   { "right_pos",      Inta   },
   { "bottom_s",       String }, /* 8 */
   { "bottom_pos",     Inta   },
   { "background_s",   String }, /* 9 */
   { "background_pos", Inta   }
};


/* Property names */
struct propname_t {
   char *name;
   enum data_t type;
};


/* Name-datatype associations */
static struct propname_t pnames[] = {
   { "sprite",         String }, /* Png image of sprite */
   { "pending",        Bool   }, /* Pending movement? */
   { "pend_invert",    Bool   }, /* Pending origo below target? */
   { "pend_fi_amp",    Int    }, /* Pending amplitude */
   { "pend_fi_c",      Float  }, /* Pending speed coefficient */
   { "pend_offset",    Int    }, /* Offset */
   { "pend_l",         Int    }, /* Length */
   { "horizontal",     Bool   }, /* Horizontal movement? */
   { "hor_speed",      Float  }, /* Horizontal speed (pixels/frame) */
   { "hor_pending",    Bool   }, /* Horizontal pending? */
   { "hor_pend_amp",   Float  }, /* Amplitude */
   { "hor_pend_c",     Float  }, /* Speed coefficient */
   { "hor_end_frames", Int    }, /* Not implemented */
   { "max_age",        Int    }, /* Target lifetime in frames */
   { "targ_x",         Int    }, /* Distance from upper right corner */
   { "targ_y",         Int    }, /* to center of target */
   { "spawn_x_points", Inta   }, /* List of target spawn points */
   { "spawn_y_points", Inta   },
   { "n_x_points",     Int    }, /* Number of points in list */
   { "n_y_points",     Int    },
   { "layers",         Inta   }, /* Layers in front of target */
   { "wave1",          Bool   }, /* Waves in front of target? */
   { "wave2",          Bool   },
   { "base_points",    Int    }, /* Base points for hitting target */
   { "targ_r_outer",   Int    }, /* Outer, middle and inner radius */
   { "targ_r_middle",  Int    },
   { "targ_r_inner",   Int    },
   { "flag_x",         Int    }, /* Flag coordinates relative upper left */
   { "flag_y",         Int    },
   { "flag_extra_fi",  Int    }, /* Flag start angle */
   { NULL,             0      }
};


/* Enum of indices in propname_t to make switch-case
 * in parse_level() easier to read.
 */
enum propnum_t {
   Sprite = 0,
   Pending,
   Pend_invert,
   Pend_fi_amp,
   Pend_fi_c,
   Pend_offset,
   Pend_l,
   Horizontal,
   Hor_speed,
   Hor_pending,
   Hor_pend_amp,
   Hor_pend_c,
   Hor_end_frames,
   Max_age,
   Targ_x,
   Targ_y,
   Spawn_x_points,
   Spawn_y_points,
   N_x_points,
   N_y_points,
   Layers,
   Wave1,
   Wave2,
   Base_points,
   Targ_r_outer,
   Targ_r_middle,
   Targ_r_inner,
   Flag_x,
   Flag_y,
   Flag_extra_fi
};


static int line;


#define MAX_VALUE_SIZE 64
/* Eat whitespace, newlines and comments */
static inline char *eat_whitespace_and_comments(char *p)
{
   do {
      while (*p == ' ' || *p == '\t' || *p == '\n') {
         if (*p == '\n') {
            line++;
         }
         p++;
      }
      if (*p == '#') {
         /* Eat comment until end of line (or end of file) */
         while (*p && *p != '\n') p++;
      }
   } while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '#'));

   /* Return NULL if end of file */
   if (!*p) {
      p = NULL;
   }

   return p;
}


/* Eat only whitespace, not newline */
static inline char *eat_whitespace(char *p)
{
   while (*p == ' ' || *p == '\t') p++;

   /* Return NULL if end of file */
   if (!*p) {
      p = NULL;
   }

   return p;
}


/* Init center of target, angle and radius */
static void init_properties(struct prop_t *p)
{
   /* Calculate cx,cy as distance from center of sprite to center of target */
   p->targ_cx = p->targ_x - (sprite_width(*(p->spr)) >> 1);
   p->targ_cy = p->targ_y - (sprite_height(*(p->spr)) >> 1);

   /* Check if cx,cy is exactly at center (to avoid division by zero) */
   if (unlikely(p->targ_cx == 0 && p->targ_cy == 0)) {
      p->targ_r = 0;
      p->targ_fi = 0;
   } else {
      /* Calculate radius targ_r from formula: x2 + y2 = r2 */
      p->targ_r = sqrt(p->targ_cx * p->targ_cx + p->targ_cy * p->targ_cy);
      /*
       * fi_rad = acos(x/r) if y >= 0, else
       * fi_rad = 2 * PI - acos(x/r)
       */
      p->targ_fi = 256.0 / (2.0 * M_PI) * acos(p->targ_cx / p->targ_r);
      if (p->targ_cy < 0) {
         p->targ_fi = 256.0 - p->targ_fi;
      }
   }

   /* Do the same calculation with flag connection point */
   p->flag_cx = p->flag_x - (sprite_width(*(p->spr)) >> 1);
   p->flag_cy = p->flag_y - (sprite_height(*(p->spr)) >> 1);
   if (unlikely(p->flag_cx == 0 && p->flag_cy == 0)) {
      p->flag_r = 0;
      p->flag_fi = 0;
   } else {
      p->flag_r = sqrt(p->flag_cx * p->flag_cx + p->flag_cy * p->flag_cy);
      p->flag_fi = 256.0 / (2.0 * M_PI) * acos(p->flag_cx / p->flag_r);
      if (p->flag_cy < 0) {
         p->flag_fi = 256.0 - p->flag_fi;
      }
   }
}


static bool parse_level(char *strp)
{
   int i;
   char *p = strp;
   int key = -1;
   enum sect_t sect = S_Undef;
   bool ret = false;
   char value[MAX_VALUE_SIZE];
   char *beg;
   int target = -1;
   bool boolv;
   int intv;
   char *valp;
   char *oldvalp;
   float floatv;
   int *arrp;
   struct prop_t *prop = NULL;
   bool layer;

   line = 1;

   do {

      layer = false;

      /* Search for next value or section */
      p = eat_whitespace_and_comments(p);
      if (!p) {
         /* Check if everything is parsed ok. */
         for (i = 0; i < NUM_LAYERS; i++) {
            if (!layers[i].spr->spr || layers[i].x == -1 || layers[i].y == -1) {
               goto out;
            }
         }
         for (i = 0; i < NUM_TARGETS; i++) {
            if (!targets[i].prop.spr->spr) {
               goto out;
            }
         }
         ret = true;
         goto out;
      }

      /* Save beginning of line */
      beg = p;

      if (*p == '[') {
         /* Section */
         for (i = 0; sections[i] && strncmp(sections[i], p, strlen(sections[i])); i++);
         if (sections[i]) {
            /* Check sections "state machine"
             * S_Undef->S_Layers->S_Target
             */
            if (sect == S_Undef) {
               if (i != S_Undef) {
                  WARN("Bad section transisition S_Undef->%s at line %d", sections[i], line);
                  goto out;
               }
               sect = i + 1;
            } else if (sect == S_Layers) {
               if (i != S_Layers) {
                  WARN("Bad section transisition S_Layers->%s at line %d", sections[i], line);
                  goto out;
               }
               sect = i + 1;
            }
            p += strlen(sections[i]);
            if (sect == S_Target) {
               target++;
               DBG("\nTarget #%d", target + 1);
               prop = &(targets[target].prop);
            }
         } else {
            WARN("Unknown section at line %d", line);
            goto out;
         }

         continue;

      }

      /* Search for key */
      for (i = 0; pnames[i].name && strncmp(pnames[i].name, p, strlen(pnames[i].name)); i++);
      if (pnames[i].name) {
         key = i;
         p += strlen(pnames[key].name);
      } else {
         for (i = 0; lnames[i].name && strncmp(lnames[i].name, p, strlen(lnames[i].name)); i++);
         if (lnames[i].name) {
            key = i;
            p += strlen(lnames[key].name);
            layer = true;
         } else {
            WARN("Unknown key at line %d", line);
            goto out;
         }
      }
      /* Search for value.
       * Rest of line must be:
       * [ \t]* = [ \t]* value [ \t]* followed by comment or newline.
       */
      p = eat_whitespace(p);
      if (!p) {
         goto out;
      }
      if (*p != '=') {
         WARN("Parse error at (line %d, char %d), '=' expected - '%c' found", line, (int)(p - beg), *p);
         goto out;
      }
      p++;
      p = eat_whitespace(p);
      if (!p) {
         goto out;
      }

      /* Now comes value followed by one of [ \t\n#]
       * Check length of value (use i for length)
       */
      for (i = 0; i < MAX_VALUE_SIZE; i++) {
         if (*(p + i) == ' ' || *(p + i) == '\t' || *(p + i) == '\n' || *(p + i) == '#') {
            break;
         }
      }
      if (i == 0) {
         /* No value */
         WARN("Parse error - no value found - for key '%s' at (line %d, char %d)",
              pnames[key].name, line, (int)(p - beg));
         goto out;
      }
      if (i == MAX_VALUE_SIZE) {
         /* Value too long, something fishy here */
         WARN("Parse error - value too long - for key '%s' at (line %d, char %d)",
              pnames[key].name, line, (int)(p - beg));
         goto out;
      }

      /* Copy value string to local variable */
      strncpy(value, p, i);
      value[i] = '\x0';
      p += i;

      if (layer) {
         /* Key is ok if we get here, but check again */
         if (key >> 1 >= NUM_LAYERS) {
            WARN("Parse error - invalid layer key %d", key);
            goto out;
         }

         switch(lnames[key].type) {

         case String:
            if(!sprite_load_from_png(layers[key >> 1].spr, value, true)) {
               WARN("sprite_load_from_png failed for %s", value);
               goto out;
            }
            break;

         case Inta:

            oldvalp = value;

            DBG("Inta [%d]: ", key >> 1);
            i = 0;
            do {
               valp = oldvalp;
               intv = strtol(valp, &oldvalp, 10);
               if (i == 0) {
                  layers[key >> 1].x = intv;
               } else if (i == 1) {
                  layers[key >> 1].y = intv;
               } else {
                  WARN("Parse error - invalid coordinates for layer %s at line %d",
                       lnames[key >> 1].name, line);
                  goto out;
               }
               DBG("[%d] = %d, ", i, intv);
               if (*oldvalp == ',') {
                  oldvalp++;
                  i++;
               }
            } while (*oldvalp != '\x0' && oldvalp != valp && i < 2);
            DBG("\n");

            break; /* case Inta */

         default:
            /* The only way to get here is by memory corruption */
            WARN("Unknown datatype?! line %d", line);
            goto out;
            break;
         }
         /* Layer parsed, continue with next row */
         continue;
      }

      /* Property */
      switch(pnames[key].type) {
      case Bool:
         /* true, false, 1 or 0 */
         if (i == 1) {
            if (value[0] == '0') {
               boolv = false;
            } else if (value[0] == '1') {
               boolv = true;
            } else {
               WARN("Parse error - boolean expected - for key '%s' at (line %d, char %d)",
                    pnames[key].name, line, (int)(p - i - beg));
               goto out;
            }
         } else if (i == 4 && strcmp("true", value) == 0) {
            boolv = true;
         }
         else if (i == 5 && strcmp("false", value) == 0) {
            boolv = false;
         } else {
            WARN("Parse error - boolean expected - for key '%s' at (line %d, char %d)",
                 pnames[key].name, line, (int)(p - i - beg));
            goto out;
         }

         switch(key) {
         case Pending:
            DBG("Bool Pending (%s) = %s", pnames[key].name, boolv ? "true" : "false");
            prop->pend = boolv;
            break;
         case Pend_invert:
            DBG("Bool Pend_invert (%s) = %s", pnames[key].name, boolv ? "true" : "false");
            prop->pend_invert = boolv;
            break;
         case Horizontal:
            DBG("Bool Horizontal (%s) = %s", pnames[key].name, boolv ? "true" : "false");
            prop->horizontal = boolv;
            break;
         case Hor_pending:
            DBG("Bool Hor_pending (%s) = %s", pnames[key].name, boolv ? "true" : "false");
            prop->hor_pend = boolv;
            break;
         case Wave1:
            DBG("Bool Wave1 (%s) = %s", pnames[key].name, boolv ? "true" : "false");
            prop->wave1 = boolv;
            break;
         case Wave2:
            DBG("Bool Wave2 (%s) = %s", pnames[key].name, boolv ? "true" : "false");
            prop->wave2 = boolv;
            break;
         default:
            WARN("Parse error. struct propname_t and enum propnum_t probably out of sync");
            goto out;
            break;
         };

         break; /* case Bool */

      case Int:
         intv = atoi(value);
         switch(key) {
         case Pend_fi_amp:
            DBG("Int Pend_fi_amp (%s) = %d", pnames[key].name, intv);
            prop->pend_fi_amp = intv;
            break;
         case Pend_offset:
            DBG("Int Pend_offset (%s) = %d", pnames[key].name, intv);
            prop->pend_offset = intv;
            break;
         case Pend_l:
            DBG("Int Pend_l (%s) = %d", pnames[key].name, intv);
            prop->pend_l = intv;
            break;
         case Hor_end_frames:
            DBG("Int Hor_end_frames (%s) = %d", pnames[key].name, intv);
            prop->hor_end_frames = intv;
            break;
         case Max_age:
            DBG("Int Max_age (%s) = %d", pnames[key].name, intv);
            prop->max_age = intv;
            break;
         case Targ_x:
            DBG("Int Targ_x (%s) = %d", pnames[key].name, intv);
            prop->targ_x = intv;
            break;
         case Targ_y:
            DBG("Int Targ_y (%s) = %d", pnames[key].name, intv);
            prop->targ_y = intv;
            break;
         case N_x_points:
            DBG("Int N_x_points (%s) = %d", pnames[key].name, intv);
            prop->n_x_points = intv;
            break;
         case N_y_points:
            DBG("Int N_y_points (%s) = %d", pnames[key].name, intv);
            prop->n_y_points = intv;
            break;
         case Base_points:
            DBG("Int Base_points (%s) = %d", pnames[key].name, intv);
            prop->base_points = intv;
            break;
         case Targ_r_outer:
            DBG("Int Targ_r_outer (%s) = %d", pnames[key].name, intv);
            prop->targ_r_outer = intv;
            break;
         case Targ_r_middle:
            DBG("Int Targ_r_middle (%s) = %d", pnames[key].name, intv);
            prop->targ_r_middle = intv;
            break;
         case Targ_r_inner:
            DBG("Int Targ_r_inner (%s) = %d", pnames[key].name, intv);
            prop->targ_r_inner = intv;
            break;
         case Flag_x:
            DBG("Int Flag_x (%s) = %d", pnames[key].name, intv);
            prop->flag_x = intv;
            break;
         case Flag_y:
            DBG("Int Flag_y (%s) = %d", pnames[key].name, intv);
            prop->flag_y = intv;
            break;
         case Flag_extra_fi:
            DBG("Int Flag_extra_fi (%s) = %d", pnames[key].name, intv);
            prop->flag_extra_fi = intv;
            break;
         default:
            WARN("Parse error. struct propname_t and enum propnum_t probably out of sync");
            goto out;
            break;
         };
         break; /* case Int */

      case Inta:
         switch(key) {
         case Spawn_x_points:
            arrp = prop->spawn_x_points;
            break;
         case Spawn_y_points:
            arrp = prop->spawn_y_points;
            break;
         case Layers:
            arrp = prop->layers;
            break;
         default:
            WARN("Parse error. struct propname_t and enum propnum_t probably out of sync");
            goto out;
            break;
         }

         oldvalp = value;

         DBG("Inta: ");
         i = 0;
         do {
            valp = oldvalp;
            arrp[i] = strtol(valp, &oldvalp, 10);
            DBG("[%d] = %d, ", i, arrp[i]);
            if (*oldvalp == ',') {
               /* Skip commachar. This is not foolproof but it works if
                * array values are comma separated and if there are no
                * spaces between values, like this: arr = 1,2,3,4,5
                */
               oldvalp++;
               i++;
            }
         } while (*oldvalp != '\x0' && oldvalp != valp);
         DBG("\n");

         break; /* case Inta */


      case Float:
         floatv = atof(value);
         switch(key) {
         case Pend_fi_c:
            DBG("Float Pend_fi_c (%s) = %f", pnames[key].name, floatv);
            prop->pend_fi_c = floatv;
            break;
         case Hor_speed:
            DBG("Float Hor_speed (%s) = %f", pnames[key].name, floatv);
            prop->hor_speed = floatv;
            break;
         case Hor_pend_amp:
            DBG("Float Hor_pend_amp (%s) = %f", pnames[key].name, floatv);
            prop->hor_pend_amp = floatv;
            break;
         case Hor_pend_c:
            DBG("Float Hor_pend_c (%s) = %f", pnames[key].name, floatv);
            prop->hor_pend_c = floatv;
            break;
         default:
            WARN("Parse error. struct propname_t and enum propnum_t probably out of sync");
            goto out;
            break;
         };
         break;

      case String:
         if(!sprite_load_from_png(targets[target].prop.spr, value, true)) {
            WARN("sprite_load_from_png failed for %s", value);
            goto out;
         }
         break;

      default:
         /* The only way to get here is by memory corruption */
         WARN("Unknown datatype?! line %d", line);
         goto out;
         break;
      }

   } while (p);

out:

   if (!ret) {
      /* Parsing failed, free level */
      free_level();
   } else {
      /* Success! */

      /* Last layer is background, coordinates are relative (0,0) */
      bg_x = layers[NUM_LAYERS - 1].x;
      bg_y = layers[NUM_LAYERS - 1].y;
      sprite_set_pos(*layers[NUM_LAYERS - 1].spr, bg_x, bg_y);
      /* All other coordinates are relative background */
      for (i = 0; i < NUM_LAYERS - 1; i++) {
         sprite_set_pos(*layers[i].spr, bg_x + layers[i].x, bg_y + layers[i].y);
      }
      for (i = 0; i < NUM_TARGETS; i++) {
         if(!sprite_load_from_png(&targets[i].scorespr, "png/skull.png", true)) {
            WARN("sprite_load_from_png failed for %s", "png/skull.png");
            ret = false;
            goto out;
         }
         init_properties(&targets[i].prop);
         targets[i].state = Dead;
      }
      /* Bonusspr and flags are same for all levels and
       * handled by carnival.c (for now).
       */
   }

   return ret;
}


/* Initializations */
static void init_level(void)
{
   int i;
   for (i = 0; i < NUM_LAYERS; i++) {
      memset(&lspr[i], 0, sizeof(struct sprite_t));
      layers[i].spr = &lspr[i];
      layers[i].x = -1;
      layers[i].y = -1;
   }
   for (i = 0; i < NUM_TARGETS; i++) {
      memset(&tspr[i], 0, sizeof(struct sprite_t));
      targets[i].prop.spr = &tspr[i];
   }
}


/* Free resources allocated by load_level */
void free_level(void)
{
   int i;

   for (i = 0; i < NUM_TARGETS; i++) {
      if (targets[i].prop.spr->spr) {
         sprite_free(targets[i].prop.spr);
         sprite_free(&targets[i].scorespr);
      }
   }

   for (i = 0; i < NUM_LAYERS; i++) {
      if (layers[i].spr->spr) {
         sprite_free(layers[i].spr);
      }
      layers[i].x = -1;
      layers[i].y = -1;
   }
}


/* Open filename and call parse_level to parse file */
bool load_level(const char *filename)
{
   struct stat statbuf;
   bool ret = false;
   int fd = open(filename, O_RDONLY);
   char *mem;
#ifdef _WIN32
   HANDLE fh, fhmap;
#endif
   static bool level_initiated = false;


   /* Init level first time */
   if (!level_initiated) {
      init_level();
      level_initiated = true;
   }

   if (fd < 0) {
      perror(filename);
      goto out;
   }

   if (fstat(fd, &statbuf) < 0) {
      perror(filename);
      goto out2;
   }

#ifdef _WIN32
   fh = (HANDLE)_get_osfhandle(fd);
   fhmap = CreateFileMapping(fh, NULL, PAGE_READONLY, 0, statbuf.st_size, NULL);
   if (!fhmap) {
      WARN("CreateFileMapping failed");
      goto out2;
   }
   mem = MapViewOfFile(fhmap, FILE_MAP_READ, 0, 0, 1);
   if (!mem) {
      WARN("MapViewOfFile failed");
      CloseHandle(fhmap);
      goto out2;
   }
#else
   mem = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (mem == MAP_FAILED) {
      perror(filename);
      goto out2;
   }
#endif

   if (!parse_level(mem)) {
      WARN("parse_level failed");
      goto out3;
   }

   /* Draw background */
   sprite_blit(*(layers[NUM_LAYERS - 1].spr));
   video_flip();

   ret = true;

out3:

#ifdef _WIN32
   UnmapViewOfFile(mem);
   CloseHandle(fhmap);
#else
   munmap(mem, statbuf.st_size);
#endif

out2:

   close(fd);

out:

   return ret;
}
