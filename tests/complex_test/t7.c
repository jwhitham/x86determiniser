#include <stdint.h>
#include <math.h>

double value = 1.23456;

typedef union {
   float    f;
   uint32_t i;
} t_convert;

void test_7 (void)
{
   int i;
   t_convert convert;

   for (i = 0; i < 5; i++) {
      value = pow (cos (sin (value)), 2.12345);
      convert.f = (float) value;
      __asm__ volatile ("outl %%eax, $0x30" : : "a"((convert.i)) );

      value = sqrt (value) + log (value);
      convert.f = (float) value;
      __asm__ volatile ("outl %%eax, $0x30" : : "a"((convert.i)) );

   }
}

