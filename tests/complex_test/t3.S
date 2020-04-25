.text
/* On x86 the sequence "call 1f; 1: pop <something>" may be used to implement
 * position independent code (PIC). Some compilers will generate a "thunk"
 * for this, but others directly inline the call/pop sequence, which
 * will cause confusion in the trace parser, because you get a call that
 * isn't matched by a return. */

.global test_3
.global _test_3

#ifdef X64

test_3:
_test_3:
   call  this_is_a_subroutine
   call  this_is_not_a_subroutine
this_is_not_a_subroutine:
   pop   %rax
   call  this_is_a_subroutine
   ret

this_is_a_subroutine:
   call  1f
   ret

1: ret

#else

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

#endif

