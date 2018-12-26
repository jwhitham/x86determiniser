.text

/* On x86 the sequence "call 1f; 1: pop <something>" may be used to implement
 * position independent code (PIC). Some compilers will generate a "thunk"
 * for this, but others directly inline the call/pop sequence, which
 * will cause confusion in the trace parser, because you get a call that
 * isn't matched by a return. */

.global test_3
.global _test_3

test_3:
_test_3:
   call  this_is_a_subroutine
   call  this_is_not_a_subroutine
this_is_not_a_subroutine:
   pop   %eax
   call  this_is_a_subroutine
   ret

this_is_a_subroutine:
   call  1f
   ret

1: ret



.global start_test
.global _start_test
start_test:
_start_test:
   inl      $0x30, %eax
   mov      $1, %eax
   outl     %eax, $0x30
   ret

.global stop_test
.global _stop_test
stop_test:
_stop_test:
   mov      $252, %eax
   outl     %eax, $0x30
   ret
