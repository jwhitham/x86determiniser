#define tick() \
   __asm__ volatile ("outl %%eax, $0x30" : : "a"((99)) );

void quirky () 
{
   ;
}

void zero_ticks ()
{
   volatile int i;
   for (i = 0; i < 10; i++) {}
}

void one_tick ()
{
   tick ();
}

void two_ticks ()
{
   tick ();
   tick ();
}

void four_ticks ()
{
   volatile int i;
   for (i = 0; i < 3; i++) {
      tick ();
   }
   tick ();
}

void five_ticks ()
{
   four_ticks ();
   one_tick ();
}

void x_ticks (int x)
{
   volatile int i;
   for (i = 0; i < 3; i++) {
      while (x) {
         tick ();
         x--;
      }
   }
}

void nineteen_ticks ()
{
   volatile int i, j;
   for (i = 0; i < 6; i++) {
      for (j = 0; j < 6; j++) {
         if (i == j) {
            switch (j) {
               case 0:
                  zero_ticks ();
                  break;
               case 1:
                  one_tick ();
                  break;
               case 2:
                  two_ticks ();
                  break;
               case 3:
                  x_ticks (1);
                  break;
               case 4:
                  four_ticks ();
                  break;
               case 5:
                  five_ticks ();
                  break;
            }
         }
      }
      tick ();
   }
}

/* These are defined in assembly code (t2.s) */
void direct_recurse (int depth);
void transitive_recurse (int depth);
void tail_recurse_1 (int depth);
void tail_recurse_2 (int depth);

void (* fptr) ();

void runme_root1 (int first_time) /* 25 ticks first time, 17 ticks second time */
{
   volatile int i;

   if (first_time) {
      nineteen_ticks ();      /* 19 ticks */
      fptr = one_tick;
   } else {
      direct_recurse (3);     /* 3 ticks */
      transitive_recurse (3); /* 3 ticks */
      tail_recurse_1 (3);     /* 3 ticks */
      fptr ();                /* 1 tick */
      tail_recurse_2 (1);     /* 1 tick */
   }
   for (i = 0; i < 4; i++) {  /* 6 ticks for this loop */
      x_ticks (i);            /* 1,2,3 ticks */
   }
}

void call_site_2 ()
{
   runme_root1 (0);
}

void test_1 (void)
{
   runme_root1 (1);
   call_site_2 ();
}


