#!/bin/sh

usage () {
    echo "Usage: png2h.sh <PNG-file>"
    echo "  ie 'png2h skull.png' creates skull.h"
}

PNG=$1
if ! test -f "$PNG"; then
    usage
    exit 1
fi

NAME=`basename $PNG .png`

# Create temporary .h file ($$.h)
gdk-pixbuf-csource --raw --name=$NAME $PNG > $$.h || exit 1

# Compile small c-app that converts the .h file
cat <<EOF > $$.c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zlib.h>

typedef unsigned char guint8;

/* Header file was created with:
 * gdk-pixbuf-csource --raw --name=$NAME $PNG > $$.h
 */
#include "$$.h"
#define PM_STR "$NAME"
#define PIXMAP $NAME 

struct comp_surf_t {
   int w, h, pitch;
   int ulen;
   unsigned char *udata;
   int clen;
   unsigned char cdata[];
};

static void print_compressed(struct comp_surf_t *s, const char *name)
{
   int i;
   unsigned long comprlen;

   s->udata = (unsigned char *)malloc(s->ulen);
   if (!s->udata) {
      perror("malloc");
      exit(1);
   }
   comprlen = s->ulen;
   if (compress(s->udata, &comprlen, &PIXMAP[24], s->ulen) == Z_OK) {
      s->clen = (int)comprlen;
      printf("static struct comp_surf_t %s = { %d, %d, %d, %d, NULL, %d, {\n   ",
             name, s->w, s->h, s->pitch, s->ulen, s->clen);
      for (i = 0; i < s->clen - 1; i++) {
         printf("0x%02x, ", s->udata[i]);
         if (i % 12 == 11) {
            printf("\n   ");
         }
      }
      printf("0x%02x\n   }\n};\n", s->udata[i]);
   }
   free(s->udata);
}


int main (void)
{
   struct comp_surf_t s;
   unsigned char *p = (unsigned char *)PIXMAP;

   s.ulen = (*(p + 4) << 24) + (*(p + 5) << 16) + (*(p + 6) << 8) + *(p + 7) - 24;
   s.pitch = (*(p + 12) << 24) + (*(p + 13) << 16) + (*(p + 14) << 8) + *(p + 15);
   s.w = (*(p + 16) << 24) + (*(p + 17) << 16) + (*(p + 18) << 8) + *(p + 19);
   s.h = (*(p + 20) << 24) + (*(p + 21) << 16) + (*(p + 22) << 8) + *(p + 23);

   print_compressed(&s, PM_STR);

   return 0;
}
EOF

gcc -o $$.elf $$.c -lz || exit 1

./$$.elf > ${NAME}.h || exit 1

# Flush buffers before removing temp files
sync

rm $$.h
rm $$.c
rm $$.elf

echo "${NAME}.h created"
