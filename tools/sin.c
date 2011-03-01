#include <stdio.h>
#include <math.h>

int main(void)
{
   int i, j;
   double v;

   printf("sintab[65] = {\n   ");

   for (i = 0; i < 65; i ++) {
      printf("%.6f, ", sin(2 * M_PI * (i / (double)256.0)));
      if (i % 4 == 3) {
         printf("\n   ");
      }
   }

   printf("};\n");

   return 0;
}
