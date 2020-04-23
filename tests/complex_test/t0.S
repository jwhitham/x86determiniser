
.global _start
_start:

   /* start test */
   inl      $0x30, %eax    /* reset the instruction counter to zero */
   mov      $1, %eax       /* indicate start of trace */
   outl     %eax, $0x30

   call     all_tests

   /* end of test */
   mov      $252, %eax     /* indicate end of trace */
   outl     %eax, $0x30
   ret

