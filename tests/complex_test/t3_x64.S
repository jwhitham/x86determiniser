.text

.global test_3
.global _test_3

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
