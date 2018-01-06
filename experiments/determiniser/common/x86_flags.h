

/* These definitions are from i8086emu - see README.md */
#define FLAG_CF  1    /* Carry-Flag */
#define FLAG_PF  4    /* Parity-Flag */
#define FLAG_ACF 16   /* Auxillary-Carry-Flag */
#define FLAG_ZF  64   /* Zero-Flag */
#define FLAG_SF  128  /* Sign-Flag */
#define FLAG_TF  256  /* Trap-Flag */
#define FLAG_IF  512  /* Interrupt-Flag */
#define FLAG_DF  1024 /* Direction-Flag */
#define FLAG_OF  2048 /* Overflow-Flag */

#define SINGLE_STEP_FLAG (FLAG_TF)

