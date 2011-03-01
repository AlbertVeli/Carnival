/**
 * @file sdl_sprite_alpha.c
 * @brief SDL-specific sprite routines
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

#include <png.h>
#include <stdlib.h>
#include <string.h>
#include "sdl_sprite.h"
#include "sdl_rotozoom.h"

/* Pixel array allocated for each sprite and not freed until sprite_free()
 * is called.
 */
#define MAX_SPRITES 30
static char *pixels[MAX_SPRITES] = {
   NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL
};


#define PNG_BYTES_TO_CHECK 4
static SDL_Surface *sdl_load_png(const char *filename, bool *rgba)
{
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;
   char buf[PNG_BYTES_TO_CHECK];
   png_uint_32 y;
   char *p;
   Uint32 rmask, gmask, bmask, amask;
   SDL_Surface *s = NULL;
   char *pixelp;
   int i = 0;
   png_byte bit_depth;
   png_byte color_type;
   png_size_t rowbytes;
   png_size_t width;
   png_size_t height;
   png_bytepp row_pointers;

   /* Find slot in pixels[] */
   while (pixels[i] && i < MAX_SPRITES) {
      i++;
   }
   if (i == MAX_SPRITES) {
      WARN("No free pixel slot available for %s", filename);
      goto out2;
   }
   pixelp = pixels[i];

   if (!(fp = fopen(filename, "rb"))) {
      perror(filename);
      goto out2;
   }

   if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
      WARN("%s is too short", filename);
      fclose(fp);
      goto out2;
   }

   if (png_sig_cmp((png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK)) {
      WARN("%s is not a png file", filename);
      fclose(fp);
      goto out2;
   }

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                    NULL, NULL, NULL);
   if (png_ptr == NULL) {
      fclose(fp);
      goto out2;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      goto out2;
   }

   if (setjmp(png_jmpbuf(png_ptr))) {
      goto out;
   }

   png_init_io(png_ptr, fp);

   png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

   /* XXX: Replace with png_read_info(png_ptr, info_ptr) ? */
   png_read_png(png_ptr, info_ptr, 0, NULL);

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

   bit_depth = png_get_bit_depth(png_ptr, info_ptr);
   color_type = png_get_color_type(png_ptr, info_ptr);
   rowbytes = png_get_rowbytes(png_ptr, info_ptr);
   width = png_get_image_width(png_ptr, info_ptr);
   height = png_get_image_height(png_ptr, info_ptr);
   row_pointers = png_get_rows(png_ptr, info_ptr);

   if (!((bit_depth == 8 && color_type == PNG_COLOR_TYPE_RGBA) ||
         (bit_depth == 8 && color_type == PNG_COLOR_TYPE_PALETTE))) {
      WARN("%s has format %dbit, colortype %d but only 8bit RGBA or 8bpp PALETTE png files supported",
           filename, bit_depth, color_type);
      goto out;
   }

   if (bit_depth == 8 && color_type == PNG_COLOR_TYPE_RGBA) {
      /* 32bpp, RGBA */
      *rgba = true;

      pixelp = (char *)malloc(height * rowbytes);
      if (!pixelp) {
         WARN("malloc failed for %s", filename);
         goto out;
      }
      for (y = 0, p = pixelp; y < height; y++, p += rowbytes) {
         memcpy(p, row_pointers[y], rowbytes);
      }

      s = SDL_CreateRGBSurfaceFrom(pixelp, width, height,
                                   bit_depth * 4, rowbytes,
                                   rmask, gmask, bmask, amask);

      /* Don't free pixelp until surface is destroyed */

   } else {
      SDL_Color colors[256];
      png_colorp palette;
      int num_palette;

      /* 8bpp, PNG_COLOR_TYPE_PALETTE */
      *rgba = false;

      s = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
                               bit_depth, rmask, gmask, bmask, amask);
      if (SDL_MUSTLOCK(s)) {
         SDL_LockSurface(s);
      }

      /* Pixels */
      for (y = 0, p = (char *)s->pixels; y < height; y++, p += s->pitch) {
         memcpy(p, row_pointers[y], rowbytes);
      }

      png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

      /* Palette */
      for (y = 0; y < (unsigned int)num_palette; y++) {
         colors[y].r = palette[y].red;
         colors[y].g = palette[y].green;
         colors[y].b = palette[y].blue;
      }
      SDL_SetColors(s, colors, 0, num_palette);

      if (SDL_MUSTLOCK(s)) {
         SDL_UnlockSurface(s);
      }
   }

out:

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

   fclose(fp);

out2:

   return s;
}


/**
 * Check if coord x, y collides with sprp
 * @arg sprp Pointer to a struct sprite_t.
 * @arg x x-coordinate of point to check.
 * @arg y y-coordinate of point to check.
 * @return true collision, false no collision
 */
static bool sprite_collide_8bit(struct sprite_t *sprp, int x, int y)
{
   SDL_Surface *spr;
   Uint8 *p;
   Uint32 pixelcolor;

   /* Check if x,y is inside sprite rect */
   if (!((x >= sprp->rect.x) &&
         (x - sprp->rect.x < sprp->rect.w) &&
         (y >= sprp->rect.y) &&
         (y - sprp->rect.y < sprp->rect.h))) {
      /* Outside, no collision */
      return false;
   }

   /* Inside, if not transparent, always hit */
   if (!sprp->trans) {
      return true;
   }

   /* Transparent, check if x,y is transparent */
   spr = (SDL_Surface *)(sprp->spr_trans);
   /* Must lock to access spr->pixels */
   if (SDL_MUSTLOCK(spr)) {
      SDL_LockSurface(spr);
   }
   p = (Uint8 *)spr->pixels + (y - sprp->rect.y) * spr->pitch + (x - sprp->rect.x);
   pixelcolor = *p;

   /* Unlock surface */
   if (SDL_MUSTLOCK(spr)) {
      SDL_UnlockSurface(spr);
   }

   return pixelcolor != spr->format->colorkey;
}


/**
 * Check if coord x, y collides with sprp
 * @arg sprp Pointer to a struct sprite_t.
 * @arg x x-coordinate of point to check.
 * @arg y y-coordinate of point to check.
 * @return true collision, false no collision
 */
static bool sprite_collide_alpha(struct sprite_t *sprp, int x, int y)
{
   SDL_Surface *spr;
   Uint8 *p;
   Uint32 pixelcolor;
   Uint8 r, g, b, a;

   /* Check if x,y is inside sprite rect */
   if (!((x >= sprp->rect.x) &&
         (x - sprp->rect.x < sprp->rect.w) &&
         (y >= sprp->rect.y) &&
         (y - sprp->rect.y < sprp->rect.h))) {
      /* Outside, no collision */
      return false;
   }

   /* Inside, if not transparent, always hit */
   if (!sprp->trans) {
      return true;
   }

   /* Transparent, check if x,y is transparent */
   spr = (SDL_Surface *)(sprp->spr_trans);
   /* Must lock to access spr->pixels */
   if (SDL_MUSTLOCK(spr)) {
      SDL_LockSurface(spr);
   }
   p = (Uint8 *)spr->pixels + (y - sprp->rect.y) * spr->pitch + (x - sprp->rect.x) * 4;
   pixelcolor = *(Uint32 *)p;

   /* Unlock surface */
   if (SDL_MUSTLOCK(spr)) {
      SDL_UnlockSurface(spr);
   }

   SDL_GetRGBA(pixelcolor, spr->format, &r, &g, &b, &a);

   return (a != 0);
}


/* ----------------------------------------------
 * Exported functions
 * ----------------------------------------------
 */


void sprite_erase(struct sprite_t *sprp)
{
   SDL_Surface *spr = (SDL_Surface *)sprp->spr;

   SDL_FillRect(spr, NULL, *((Uint8 *)(spr->pixels)));

   if (likely(sprp->spr_trans != sprp->spr)) {
      SDL_FreeSurface((SDL_Surface *)sprp->spr_trans);
      sprp->spr_trans = NULL;
   }
}


/* Free sprite previously allocated by load_sprite_bmp */
void sprite_free(struct sprite_t *s)
{
   int i = 0;

   if (s->spr != s->spr_trans) {
      SDL_FreeSurface((SDL_Surface *)s->spr_trans);
   }
   SDL_FreeSurface((SDL_Surface *)s->spr);
   s->spr = NULL;
   s->spr_trans = NULL;

   /* Take the easy way and just free all first time */
   for (i = 0; i < MAX_SPRITES; i++) {
      if (pixels[i]) {
         free(pixels[i]);
         pixels[i] = NULL;
      }
   }
}


/**
 * Load sprite from bitmap.
 * @arg sprp Pointer to a struct sprite_t.
 * @arg trans Is sprite transparent?
 * @arg eightbit Set to true if sprite is going to be rotated later.
 * @return 1 OK, 0 Error
 */
int sprite_load_from_png(struct sprite_t *sprp, const char *filename, bool trans)
{
   SDL_Surface *temp;
   SDL_Surface *spr = (SDL_Surface *)sprp->spr;
   bool rgba;

   /* TODO: Detect trans in sdl_load_png(). Don't send as parameter. */

   sprp->trans = trans;

   spr = sdl_load_png(filename, &rgba);
   if (!spr) {
      WARN("load_png %s failed", filename);
      return 0;
   }

   if (rgba) {
      sprp->sprite_collide = sprite_collide_alpha;
   } else {
      sprp->sprite_collide = sprite_collide_8bit;
   }

   SDL_SetAlpha(spr, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
   temp = SDL_DisplayFormatAlpha(spr);

   if (unlikely(!temp)) {
      WARN("SDL_DisplayFormatAlpha returned \"%s\"", SDL_GetError());
      return 0;
   }
   if (!rgba) {
      /* If 8-bit, keep spr that way, sprite_rotozoom() is faster
       * for 8-bit but rgba has nicer edges.
       */
      sprp->spr = spr;
   } else {
      /* Keep spr in displayformat for faster blits */
      sprp->spr = temp;
      SDL_FreeSurface(spr);
   }
   sprp->spr_trans = temp;
   if (SDL_MUSTLOCK(temp)) {
      /* temp->pixels is NULL until locked */
      SDL_LockSurface(temp);
   }
   if (SDL_MUSTLOCK(temp)) {
      SDL_UnlockSurface(temp);
   }

   sprp->rect.x = 0;
   sprp->rect.y = 0;
   sprp->rect.w = temp->w;
   sprp->rect.h = temp->h;
   sprp->delta_w = 0;
   sprp->delta_h = 0;

   return 1;
}


void sprite_rotozoom(struct sprite_t *sprp, float angle, float zoom)
{
   /* Free previous spr_trans surface */
   if (likely(sprp->spr_trans != sprp->spr) && sprp->spr_trans) {
      SDL_FreeSurface((SDL_Surface *)sprp->spr_trans);
   }
   /* Calculate radian angle and rotozoom */
   sprp->spr_trans = rotozoomSurfaceXY(sprp->spr, angle * (2 * M_PI / 256.0f), zoom, zoom, 0);
   /* Save width and height of the rotated surface */
   sprp->rect.w = ((SDL_Surface *)sprp->spr_trans)->w;
   sprp->rect.h = ((SDL_Surface *)sprp->spr_trans)->h;
   /* Calculate delta between new and old surface width and height */
   sprp->delta_w = sprp->rect.w - ((SDL_Surface *)sprp->spr)->w;
   sprp->delta_h = sprp->rect.h - ((SDL_Surface *)sprp->spr)->h;
}


void sprite_reset(struct sprite_t *sprp)
{
   if (likely(sprp->spr_trans != sprp->spr)) {
      /* Free previous spr_trans surface */
      SDL_FreeSurface((SDL_Surface *)sprp->spr_trans);
      /* Make copy of spr with current displayformat */
      sprp->spr_trans = SDL_DisplayFormat(sprp->spr);
   }
   sprp->rect.x = 0;
   sprp->rect.y = 0;
   sprite_reset_dimensions(*sprp);
   sprp->delta_w = 0;
   sprp->delta_h = 0;
}


void sprite_blit_part(struct sprite_t *sprp, int sx, int sy, int dx, int dy, int w, int h)
{
   SDL_Rect sr = { sx, sy, w, h };
   SDL_Rect dr = { dx, dy, w, h };

   SDL_BlitSurface(sprp->spr, &sr, screen, &dr);
}


void sprite_blit_part_dest(struct sprite_t *sprp, struct sprite_t *destp, int sx, int sy, int dx, int dy, int w, int h)
{
   SDL_Rect sr = { sx, sy, w, h };
   SDL_Rect dr = { dx, dy, w, h };

   SDL_BlitSurface(sprp->spr, &sr, destp->spr, &dr);
}


/**
 * GNU Emacs settings: K&R with 3 spaces indent.
 * Local Variables:
 * c-file-style: "k&r"
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
