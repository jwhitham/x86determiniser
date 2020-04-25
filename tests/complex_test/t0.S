
.text
.align 4096
.global all_tests
.global _all_tests

trace_zero:
   /* normalise_trace.py will cause this location to appear as offset 0;
    * a jump to this point will also be the first event in the trace, at time 0 */
   call     test_1
   call     test_3
   call     test_4
   call     test_5
   call     test_6

   /* end of test */
   mov      $252, %eax     /* indicate end of trace */
   outl     %eax, $0x30
   ret


all_tests:
_all_tests:

   /* start test */
   inl      $0x30, %eax    /* reset the instruction counter to zero */
   mov      $1, %eax       /* indicate start of trace */
   outl     %eax, $0x30
   jmp      trace_zero


