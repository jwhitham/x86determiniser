
/*
  The red zone is 128 bytes below the stack pointer. 
  https://en.wikipedia.org/wiki/Red_zone_(computing)
  
  Though this only exists on 64-bit Linux, x64determiniser assumes it exists on Windows too,
  to keep the common code consistent.

  Unfortunately there have been various stack corruption bugs related to the red zone.
*/

.text
.global main
.global _main

#ifdef X64

main_stack_frame_size = 512
red_zone_size = 128
pattern_size = 256

/* RCX = base of test pattern area of memory */

.macro fill_test_pattern value
   mov   $\value, %rsi
   mov   %rcx, %rdi
	leaq	pattern_size(%rdi), %rax
0: movb	%sil, (%rdi)
	addq	$1, %rdi
	addl	$1, %esi
	cmpq	%rax, %rdi
	jne	0b 
.endm

.macro check_test_pattern value
   mov   $\value, %rsi
   mov   %rcx, %rdi
	leaq	pattern_size(%rdi), %rax
0: movb	(%rdi), %dl
   cmp   %dl, %sil
   jnz   error
	addq	$1, %rdi
	addl	$1, %esi
	cmpq	%rax, %rdi
	jne	0b 
.endm

/* This jumpy pattern won't trigger single-stepping */
.macro jumpy_pattern
   nop
   nop
   jmp      0f

0: jz       1f
   jnz      1f
   hlt

1: mov      $2f, %rax
   jmp      *%rax

2: mov      $3f, %rax
   jmp      *(%rax)

.data
3: .quad    4f
.text

4: nop
   nop
.endm

/* This jumpy pattern will trigger single-stepping */
.macro jumpy_pattern_2
   nop
   nop
   mov      $0, %rax
   jmp      *0f(,%rax,8)
.data
0: .quad    1f
.text

1: nop
   nop
.endm

main:
_main:
   add      $-main_stack_frame_size, %rsp
   mov      %rsp, %rcx                    /* RCX = base of red zone */
   add      $red_zone_size, %rsp          /* RSP moved to create red zone */

/* Test 1: branches that don't single-step with test pattern seed 33 */
test1:
   fill_test_pattern 33
   jumpy_pattern
   check_test_pattern 33

/* Test 2: branches that don't single-step with test pattern seed 99 */
test2:
   fill_test_pattern 99
   jumpy_pattern
   check_test_pattern 99

/* Test 3: branches that do single-step with test pattern seed 33 */
test3:
   fill_test_pattern 33
   jumpy_pattern_2
   check_test_pattern 33

/* Test 4: branches that do single-step with test pattern seed 99 */
test4:
   fill_test_pattern 99
   jumpy_pattern_2
   check_test_pattern 99

/* Test passed, no red zone corruption detected due to single stepping, return 0 */
good:
   add      $main_stack_frame_size - red_zone_size, %rsp
   mov      $0, %rax
   ret

error:   /* Oh no! Test failed! red zone has become corrupted! crash */
   add      $main_stack_frame_size - red_zone_size, %rsp
   hlt
   mov      $1, %rax
   ret

#else

/* This test does not do anything for 32-bit code */
main:
_main:
   mov      $0, %eax
   ret

#endif


