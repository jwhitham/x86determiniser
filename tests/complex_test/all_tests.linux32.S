
.text
.align 4096


all_tests:
_all_tests:

   /* start test */
   inl      $0x30, %eax    /* reset the instruction counter to zero */
   mov      $1, %eax       /* indicate start of trace */
   outl     %eax, $0x30

   call     test_1
   call     test_3
   call     test_4
   call     test_5
   call     test_6

   /* end of test */
   mov      $252, %eax     /* indicate end of trace */
   outl     %eax, $0x30
   ret

	.file	"t1.c"
	.text
	.align 2
	.p2align 4,,15
	.globl	one_tick
	.type	one_tick, @function
one_tick:
t1_LFB2:
	.cfi_startproc
	movl	$99, %eax
#APP
# 17 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
t1_LFE2:
	.size	one_tick, .-one_tick
	.align 2
	.p2align 4,,15
	.globl	quirky
	.type	quirky, @function
quirky:
t1_LFB0:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t1_LFE0:
	.size	quirky, .-quirky
	.align 2
	.p2align 4,,15
	.globl	zero_ticks
	.type	zero_ticks, @function
zero_ticks:
t1_LFB1:
	.cfi_startproc
	subl	$16, %esp
	.cfi_def_cfa_offset 20
	movl	$0, 12(%esp)
	movl	12(%esp), %eax
	cmpl	$9, %eax
	jg	t1_L3
	.p2align 4,,7
	.p2align 3
t1_L6:
	movl	12(%esp), %eax
	addl	$1, %eax
	movl	%eax, 12(%esp)
	movl	12(%esp), %eax
	cmpl	$9, %eax
	jle	t1_L6
t1_L3:
	addl	$16, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t1_LFE1:
	.size	zero_ticks, .-zero_ticks
	.align 2
	.p2align 4,,15
	.globl	two_ticks
	.type	two_ticks, @function
two_ticks:
t1_LFB3:
	.cfi_startproc
	movl	$99, %eax
#APP
# 22 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
# 23 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
t1_LFE3:
	.size	two_ticks, .-two_ticks
	.align 2
	.p2align 4,,15
	.globl	four_ticks
	.type	four_ticks, @function
four_ticks:
t1_LFB4:
	.cfi_startproc
	subl	$16, %esp
	.cfi_def_cfa_offset 20
	movl	$99, %eax
	movl	$0, 12(%esp)
	movl	12(%esp), %edx
	cmpl	$2, %edx
	jg	t1_L13
	.p2align 4,,7
	.p2align 3
t1_L14:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	12(%esp), %edx
	addl	$1, %edx
	movl	%edx, 12(%esp)
	movl	12(%esp), %edx
	cmpl	$2, %edx
	jle	t1_L14
t1_L13:
	movl	$99, %eax
#APP
# 32 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	addl	$16, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t1_LFE4:
	.size	four_ticks, .-four_ticks
	.align 2
	.p2align 4,,15
	.globl	five_ticks
	.type	five_ticks, @function
five_ticks:
t1_LFB5:
	.cfi_startproc
	subl	$16, %esp
	.cfi_def_cfa_offset 20
	movl	$99, %eax
	movl	$0, 12(%esp)
	movl	12(%esp), %edx
	cmpl	$2, %edx
	jg	t1_L20
	.p2align 4,,7
	.p2align 3
t1_L21:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	12(%esp), %edx
	addl	$1, %edx
	movl	%edx, 12(%esp)
	movl	12(%esp), %edx
	cmpl	$2, %edx
	jle	t1_L21
t1_L20:
	movl	$99, %eax
#APP
# 32 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
# 17 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	addl	$16, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t1_LFE5:
	.size	five_ticks, .-five_ticks
	.align 2
	.p2align 4,,15
	.globl	x_ticks
	.type	x_ticks, @function
x_ticks:
t1_LFB6:
	.cfi_startproc
	subl	$16, %esp
	.cfi_def_cfa_offset 20
	movl	$0, 12(%esp)
	movl	12(%esp), %eax
	movl	20(%esp), %edx
	cmpl	$2, %eax
	jg	t1_L25
	testl	%edx, %edx
	je	t1_L31
	movl	$99, %eax
	.p2align 4,,7
	.p2align 3
t1_L32:
#APP
# 46 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	subl	$1, %edx
	jne	t1_L32
t1_L31:
	movl	12(%esp), %eax
	addl	$1, %eax
	movl	%eax, 12(%esp)
	movl	12(%esp), %eax
	cmpl	$2, %eax
	jle	t1_L31
t1_L25:
	addl	$16, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t1_LFE6:
	.size	x_ticks, .-x_ticks
	.align 2
	.p2align 4,,15
	.globl	nineteen_ticks
	.type	nineteen_ticks, @function
nineteen_ticks:
t1_LFB7:
	.cfi_startproc
	subl	$32, %esp
	.cfi_def_cfa_offset 36
	movl	$99, %eax
	movl	$0, 8(%esp)
	movl	8(%esp), %edx
	cmpl	$5, %edx
	jg	t1_L36
	.p2align 4,,7
	.p2align 3
t1_L58:
	movl	$0, 12(%esp)
	movl	12(%esp), %edx
	cmpl	$5, %edx
	jle	t1_L59
	jmp	t1_L57
	.p2align 4,,7
	.p2align 3
t1_L41:
	movl	12(%esp), %edx
	addl	$1, %edx
	movl	%edx, 12(%esp)
	movl	12(%esp), %edx
	cmpl	$5, %edx
	jg	t1_L57
t1_L59:
	movl	8(%esp), %ecx
	movl	12(%esp), %edx
	cmpl	%edx, %ecx
	jne	t1_L41
	movl	12(%esp), %edx
	cmpl	$5, %edx
	ja	t1_L41
	jmp	*t1_L48(,%edx,4)
	.section	.rodata
	.align 4
	.align 4
t1_L48:
	.long	t1_L42
	.long	t1_L70
	.long	t1_L56
	.long	t1_L45
	.long	t1_L46
	.long	t1_L47
	.text
t1_L47:
	movl	$0, 28(%esp)
	movl	28(%esp), %edx
	cmpl	$2, %edx
	jg	t1_L56
	.p2align 4,,7
	.p2align 3
t1_L60:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	28(%esp), %edx
	addl	$1, %edx
	movl	%edx, 28(%esp)
	movl	28(%esp), %edx
	cmpl	$2, %edx
	jle	t1_L60
t1_L56:
#APP
# 32 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
t1_L70:
#APP
# 17 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	jmp	t1_L41
t1_L46:
	movl	$0, 24(%esp)
	movl	24(%esp), %edx
	cmpl	$2, %edx
	jg	t1_L70
	.p2align 4,,7
	.p2align 3
t1_L62:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	24(%esp), %edx
	addl	$1, %edx
	movl	%edx, 24(%esp)
	movl	24(%esp), %edx
	cmpl	$2, %edx
	jle	t1_L62
	jmp	t1_L70
t1_L42:
	movl	$0, 16(%esp)
	movl	16(%esp), %edx
	cmpl	$9, %edx
	jg	t1_L41
	.p2align 4,,7
	.p2align 3
t1_L61:
	movl	16(%esp), %edx
	addl	$1, %edx
	movl	%edx, 16(%esp)
	movl	16(%esp), %edx
	cmpl	$9, %edx
	jle	t1_L61
	jmp	t1_L41
t1_L45:
	movl	$0, 20(%esp)
	movl	20(%esp), %edx
	cmpl	$2, %edx
	jg	t1_L41
#APP
# 46 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
t1_L52:
	movl	20(%esp), %edx
	addl	$1, %edx
	movl	%edx, 20(%esp)
	movl	20(%esp), %edx
	cmpl	$2, %edx
	jle	t1_L52
	jmp	t1_L41
	.p2align 4,,7
	.p2align 3
t1_L57:
#APP
# 80 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	8(%esp), %edx
	addl	$1, %edx
	movl	%edx, 8(%esp)
	movl	8(%esp), %edx
	cmpl	$5, %edx
	jle	t1_L58
t1_L36:
	addl	$32, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t1_LFE7:
	.size	nineteen_ticks, .-nineteen_ticks
	.align 2
	.p2align 4,,15
	.globl	runme_root1
	.type	runme_root1, @function
runme_root1:
t1_LFB8:
	.cfi_startproc
	subl	$44, %esp
	.cfi_def_cfa_offset 48
	movl	48(%esp), %eax
	testl	%eax, %eax
	je	t1_L72
	call	nineteen_ticks
	movl	$one_tick, fptr
t1_L73:
	movl	$0, 24(%esp)
	movl	24(%esp), %edx
	movl	$99, %eax
	cmpl	$3, %edx
	jg	t1_L71
t1_L81:
	movl	24(%esp), %edx
	movl	$0, 28(%esp)
	movl	28(%esp), %ecx
	cmpl	$2, %ecx
	jg	t1_L76
	testl	%edx, %edx
	je	t1_L82
	.p2align 4,,7
	.p2align 3
t1_L83:
#APP
# 46 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	subl	$1, %edx
	jne	t1_L83
t1_L82:
	movl	28(%esp), %edx
	addl	$1, %edx
	movl	%edx, 28(%esp)
	movl	28(%esp), %edx
	cmpl	$2, %edx
	jle	t1_L82
t1_L76:
	movl	24(%esp), %edx
	addl	$1, %edx
	movl	%edx, 24(%esp)
	movl	24(%esp), %edx
	cmpl	$3, %edx
	jle	t1_L81
t1_L71:
	addl	$44, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 4
	ret
t1_L72:
	.cfi_restore_state
	movl	$3, (%esp)
	call	direct_recurse
	movl	$3, (%esp)
	call	transitive_recurse
	movl	$3, (%esp)
	call	tail_recurse_1
	call	*fptr
	movl	$1, (%esp)
	call	tail_recurse_2
	jmp	t1_L73
	.cfi_endproc
t1_LFE8:
	.size	runme_root1, .-runme_root1
	.align 2
	.p2align 4,,15
	.globl	call_site_2
	.type	call_site_2, @function
call_site_2:
t1_LFB9:
	.cfi_startproc
	subl	$28, %esp
	.cfi_def_cfa_offset 32
	movl	$0, (%esp)
	call	runme_root1
	addl	$28, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t1_LFE9:
	.size	call_site_2, .-call_site_2
	.align 2
	.p2align 4,,15
	.globl	test_1
	.type	test_1, @function
test_1:
t1_LFB10:
	.cfi_startproc
	subl	$28, %esp
	.cfi_def_cfa_offset 32
	movl	$1, (%esp)
	call	runme_root1
	addl	$28, %esp
	.cfi_def_cfa_offset 4
	jmp	call_site_2
	.cfi_endproc
t1_LFE10:
	.size	test_1, .-test_1
	.comm	fptr,4,4
	.ident	"GCC: (GNU) 4.7.4 20140401 for GNAT GPL gpl-2014 (20140405)"
	.text

quirky2: ret

#ifdef X64



direct_recurse:
_direct_recurse:
   ret



transitive_recurse:
_transitive_recurse:
   ret





tail_recurse_2:
_tail_recurse_2:
   ret

tail_recurse_1:
_tail_recurse_1:
   ret

#else

/* recursion by direct call */


direct_recurse:
_direct_recurse:
   mov   $99, %eax
   outl  %eax, $0x30

   mov   4(%esp), %eax     /* get recursion depth parameter */
   dec   %eax
   jz    0f
   push  %eax
   call  direct_recurse
   pop   %eax
0: ret


/* recursion via a second procedure */


transitive_recurse:
_transitive_recurse:
   mov   $99, %eax         /* tick */
   outl  %eax, $0x30

   mov   4(%esp), %eax     /* get recursion depth parameter */
   dec   %eax
   jz    0f
   call  second_proc       /* call second procedure */
0: ret

second_proc:
   push  %eax
   call  transitive_recurse
   pop   %eax
   ret


/* recursion via a second procedure with tail call */




tail_recurse_2:
_tail_recurse_2:
   mov   $99, %eax         /* tick */
   outl  %eax, $0x30

   mov   4(%esp), %eax

   dec   %eax
   jle   1f
   mov   %eax, 4(%esp)
   jmp   tail_recurse_1
1:
   ret

tail_recurse_1:
_tail_recurse_1:
   mov   4(%esp), %eax     /* get recursion depth parameter */
   push  %eax
   call  tail_recurse_2
   pop   %eax
   ret

#endif

/* On x86 the sequence "call 1f; 1: pop <something>" may be used to implement
 * position independent code (PIC). Some compilers will generate a "thunk"
 * for this, but others directly inline the call/pop sequence, which
 * will cause confusion in the trace parser, because you get a call that
 * isn't matched by a return. */




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

.text


.align 16
test_4:
_test_4:
    mov     $100, %ecx
h:  jmp     a
g:  dec     %ecx
    jnz     h
    jz      i
b:  jmp     c
a:  jmp     b
.align 16
d:  jmp     e
nop
nop
c:  jmp     d
nop
f:  jmp     g
.align 16
e:  jmp     f

i:  

    mov     $10, %ecx
    mov     $12345, %eax
    mov     $target, %edi
    cld

    /* some special instructions according to qemu */
    rep stos %eax,(%edi) /* fill "target" with 12345 */
    pushf
    popf
    pause
    /* end special instructions */
  
    inc     %ecx
    jz      i       /* zero flag clear, therefore does not jump */
    loop    i       /* %ecx = 1, therefore does not jump */
    loop    k       /* %ecx = 0, therefore does jump */
    ud2
k:
    .byte 0xf3    /* repz */
    .byte 0xc3    /* ret */

    .long 0
    .long 0
    .long 0
    .long 0

/
.data
target:
    .space  40





#ifdef X64

test_5:
_test_5:
    ret

#else
/* 
    This implementation of memset is from uClibc.
    memset_test uses the Linux calling convention.

    The use of "rep stos" when combined with conditional branches was tricky for QEMU,
    our code was generating wrong source addresses.
*/

memset_test:
    push   %edi                             /* 12bcd8:       57                       */
    push   %ebx                             /* 12bcd9:       53                       */
    mov    0xc(%esp),%edx                   /* 12bcda:       8b 54 24 0c              */
    mov    %edx,%edi                        /* 12bcde:       89 d7                    */
    mov    0x14(%esp),%ecx                  /* 12bce0:       8b 4c 24 14              */
    mov    0x10(%esp),%eax                  /* 12bce4:       8b 44 24 10              */
    mov    %ecx,%ebx                        /* 12bce8:       89 cb                    */
    shr    $0x2,%ecx                        /* 12bcea:       c1 e9 02                 */
    je     .L12bcfa                         /* 12bced:       74 0b                    */
    movzbl %al,%eax                         /* 12bcef:       0f b6 c0                 */
    imul   $0x1010101,%eax,%eax             /* 12bcf2:       69 c0 01 01 01 01        */
    rep stos %eax,%es:(%edi)                /* 12bcf8:       f3 ab                    */
.L12bcfa:
    and    $0x3,%ebx                        /* 12bcfa:       83 e3 03                 */
    je     .L12bd03                         /* 12bcfd:       74 04                    */
.L12bcff:
    stos   %al,%es:(%edi)                   /* 12bcff:       aa                       */
    dec    %ebx                             /* 12bd00:       4b                       */
    jne    .L12bcff                         /* 12bd01:       75 fc                    */
.L12bd03:
    mov    %edx,%eax                        /* 12bd03:       89 d0                    */
    pop    %ebx                             /* 12bd05:       5b                       */
    pop    %edi                             /* 12bd06:       5f                       */
    ret                                     /* 12bd07:       c3                       */

test_5:
_test_5:
    pushl  $99  /* set 99 bytes */
    pushl  $65  /* set to 'A' */
    pushl  $memset_scratch_area
    call   memset_test
    pop    %eax
    pop    %eax
    pop    %eax
    ret

.data
memset_scratch_area:
    .space 99

#endif

	.file	"t6.c"
	.text
	.align 2
	.p2align 4,,15
	.globl	target1
	.type	target1, @function
target1:
t6_LFB0:
	.cfi_startproc
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	subl	$16, %esp
	.cfi_def_cfa_offset 24
	fldt	t1
	flds	t6_LC0
	fxch	%st(1)
	fucomi	%st(1), %st
	jae	t6_L2
	fstp	%st(1)
	fnstcw	14(%esp)
	movzwl	14(%esp), %eax
	movb	$12, %ah
	movw	%ax, 12(%esp)
	fldcw	12(%esp)
	fistpq	(%esp)
	fldcw	14(%esp)
	movl	(%esp), %eax
	movl	4(%esp), %edx
t6_L3:
	movl	%eax, above
	movl	%edx, above+4
	movl	x1, %eax
	addl	$1, %eax
	movl	%eax, x1
	fldt	t1
	fadds	t6_LC1
	fstpt	t1
	addl	$16, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L2:
	.cfi_restore_state
	fnstcw	14(%esp)
	fsubp	%st, %st(1)
	movzwl	14(%esp), %eax
	movb	$12, %ah
	movw	%ax, 12(%esp)
	fldcw	12(%esp)
	fistpq	(%esp)
	fldcw	14(%esp)
	movl	4(%esp), %edx
	movl	(%esp), %eax
	leal	-2147483648(%edx), %ecx
	movl	%ecx, %edx
	jmp	t6_L3
	.cfi_endproc
t6_LFE0:
	.size	target1, .-target1
	.align 2
	.p2align 4,,15
	.globl	target2
	.type	target2, @function
target2:
t6_LFB1:
	.cfi_startproc
	movl	above, %eax
	movl	above+4, %edx
	shldl	$1, %eax, %edx
	addl	%eax, %eax
	movl	%eax, above
	movl	%edx, above+4
	movl	x1, %eax
	addl	%eax, %eax
	movl	%eax, x1
	fldt	t1
	fmull	t6_LC3
	fstpt	t1
	ret
	.cfi_endproc
t6_LFE1:
	.size	target2, .-target2
	.align 2
	.p2align 4,,15
	.globl	target3
	.type	target3, @function
target3:
t6_LFB2:
	.cfi_startproc
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	subl	$16, %esp
	.cfi_def_cfa_offset 24
	fldl	t2
	flds	t6_LC0
	fxch	%st(1)
	fucomi	%st(1), %st
	jae	t6_L8
	fstp	%st(1)
	fnstcw	14(%esp)
	movzwl	14(%esp), %eax
	movb	$12, %ah
	movw	%ax, 12(%esp)
	fldcw	12(%esp)
	fistpq	(%esp)
	fldcw	14(%esp)
	movl	(%esp), %eax
	movl	4(%esp), %edx
t6_L9:
	movl	%eax, below
	movl	%edx, below+4
	movl	x2, %eax
	addl	$1, %eax
	movl	%eax, x2
	fldl	t2
	fadds	t6_LC1
	fstpl	t2
	addl	$16, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L8:
	.cfi_restore_state
	fnstcw	14(%esp)
	fsubp	%st, %st(1)
	movzwl	14(%esp), %eax
	movb	$12, %ah
	movw	%ax, 12(%esp)
	fldcw	12(%esp)
	fistpq	(%esp)
	fldcw	14(%esp)
	movl	4(%esp), %edx
	movl	(%esp), %eax
	leal	-2147483648(%edx), %ecx
	movl	%ecx, %edx
	jmp	t6_L9
	.cfi_endproc
t6_LFE2:
	.size	target3, .-target3
	.align 2
	.p2align 4,,15
	.globl	target4
	.type	target4, @function
target4:
t6_LFB3:
	.cfi_startproc
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	subl	$16, %esp
	.cfi_def_cfa_offset 24
	flds	t3
	flds	t6_LC0
	fxch	%st(1)
	fucomi	%st(1), %st
	jae	t6_L12
	fstp	%st(1)
	fnstcw	14(%esp)
	movzwl	14(%esp), %eax
	movb	$12, %ah
	movw	%ax, 12(%esp)
	fldcw	12(%esp)
	fistpq	(%esp)
	fldcw	14(%esp)
	movl	(%esp), %eax
	movl	4(%esp), %edx
t6_L13:
	movl	%eax, below
	movl	%edx, below+4
	movl	x2, %eax
	addl	%eax, %eax
	movl	%eax, x2
	fldl	t2
	fmuls	t6_LC4
	fstpl	t2
	addl	$16, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L12:
	.cfi_restore_state
	fnstcw	14(%esp)
	fsubp	%st, %st(1)
	movzwl	14(%esp), %eax
	movb	$12, %ah
	movw	%ax, 12(%esp)
	fldcw	12(%esp)
	fistpq	(%esp)
	fldcw	14(%esp)
	movl	4(%esp), %edx
	movl	(%esp), %eax
	leal	-2147483648(%edx), %ecx
	movl	%ecx, %edx
	jmp	t6_L13
	.cfi_endproc
t6_LFE3:
	.size	target4, .-target4
	.globl	__udivdi3
	.align 2
	.p2align 4,,15
	.globl	target5
	.type	target5, @function
target5:
t6_LFB4:
	.cfi_startproc
	subl	$28, %esp
	.cfi_def_cfa_offset 32
	movl	above, %eax
	movl	above+4, %edx
	movl	%eax, (%esp)
	movl	below, %eax
	movl	%edx, 4(%esp)
	movl	below+4, %edx
	movl	%eax, 8(%esp)
	movl	%edx, 12(%esp)
	call	__udivdi3
	movl	%eax, above
	movl	%edx, above+4
	addl	$28, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE4:
	.size	target5, .-target5
	.align 2
	.p2align 4,,15
	.globl	target6
	.type	target6, @function
target6:
t6_LFB5:
	.cfi_startproc
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	subl	$24, %esp
	.cfi_def_cfa_offset 32
	fldt	t1
	flds	t6_LC0
	fxch	%st(1)
	fucomi	%st(1), %st
	jae	t6_L18
	fstp	%st(1)
	fnstcw	6(%esp)
	movzwl	6(%esp), %eax
	movb	$12, %ah
	movw	%ax, 4(%esp)
	fldcw	4(%esp)
	fistpq	8(%esp)
	fldcw	6(%esp)
	movl	8(%esp), %eax
	movl	12(%esp), %edx
t6_L19:
	movl	%eax, above
	movl	%edx, above+4
	movl	below, %eax
	movl	below+4, %edx
	movl	%eax, 8(%esp)
	movl	%edx, 12(%esp)
	testl	%edx, %edx
	fildq	8(%esp)
	js	t6_L22
t6_L20:
	fstpl	16(%esp)
	fldl	16(%esp)
	fstpl	t2
	addl	$24, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L18:
	.cfi_restore_state
	fnstcw	6(%esp)
	fsubp	%st, %st(1)
	movzwl	6(%esp), %eax
	movb	$12, %ah
	movw	%ax, 4(%esp)
	fldcw	4(%esp)
	fistpq	8(%esp)
	fldcw	6(%esp)
	movl	12(%esp), %edx
	movl	8(%esp), %eax
	leal	-2147483648(%edx), %ecx
	movl	%ecx, %edx
	jmp	t6_L19
	.p2align 4,,7
	.p2align 3
t6_L22:
	fadds	t6_LC5
	jmp	t6_L20
	.cfi_endproc
t6_LFE5:
	.size	target6, .-target6
	.align 2
	.p2align 4,,15
	.globl	target7
	.type	target7, @function
target7:
t6_LFB6:
	.cfi_startproc
	subl	$12, %esp
	.cfi_def_cfa_offset 16
	movl	x1, %eax
	xorl	%edx, %edx
	movl	x2, %ecx
	divl	%ecx
	movl	%edx, %eax
	xorl	%edx, %edx
	movl	%eax, (%esp)
	movl	%edx, 4(%esp)
	fildq	(%esp)
	fstpt	t1
	addl	$12, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE6:
	.size	target7, .-target7
	.align 2
	.p2align 4,,15
	.globl	target8
	.type	target8, @function
target8:
t6_LFB7:
	.cfi_startproc
	fldl	t2
	fstpt	t1
	ret
	.cfi_endproc
t6_LFE7:
	.size	target8, .-target8
	.align 2
	.p2align 4,,15
	.globl	target9
	.type	target9, @function
target9:
t6_LFB8:
	.cfi_startproc
	subl	$12, %esp
	.cfi_def_cfa_offset 16
	fldt	t1
	fstpl	(%esp)
	fldl	(%esp)
	fstpl	t2
	addl	$12, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE8:
	.size	target9, .-target9
	.align 2
	.p2align 4,,15
	.globl	targeta
	.type	targeta, @function
targeta:
t6_LFB9:
	.cfi_startproc
	fldt	t1
	fldl	t2
	fdivrp	%st, %st(1)
	fstpt	t1
	ret
	.cfi_endproc
t6_LFE9:
	.size	targeta, .-targeta
	.align 2
	.p2align 4,,15
	.globl	targetb
	.type	targetb, @function
targetb:
t6_LFB10:
	.cfi_startproc
	movl	above, %eax
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	movl	above+4, %edx
	movl	below, %ecx
	movl	below+4, %ebx
	imull	%ecx, %edx
	imull	%eax, %ebx
	addl	%edx, %ebx
	mull	%ecx
	addl	%ebx, %edx
	movl	%eax, above
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	movl	%edx, above+4
	ret
	.cfi_endproc
t6_LFE10:
	.size	targetb, .-targetb
	.align 2
	.p2align 4,,15
	.type	bits, @function
bits:
t6_LFB21:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%edx, %ebp
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$8, %esp
	.cfi_def_cfa_offset 28
	movl	28(%eax), %edx
	movl	24(%eax), %edi
	cmpl	%edx, %ebp
	jle	t6_L32
	movl	16(%eax), %ecx
	movl	20(%eax), %ebx
	movl	%ecx, (%esp)
	cmpl	%ecx, %ebx
	je	t6_L41
	movl	12(%eax), %ecx
	movl	%ecx, 4(%esp)
	movl	%edx, %ecx
	jmp	t6_L35
t6_L36:
	cmpl	(%esp), %ebx
	je	t6_L33
t6_L35:
	movl	4(%esp), %edx
	movzbl	(%edx,%ebx), %esi
	addl	$1, %ebx
	movl	%ebx, 20(%eax)
	sall	%cl, %esi
	addl	$8, %ecx
	orl	%esi, %edi
	cmpl	%ecx, %ebp
	movl	%ecx, 28(%eax)
	jg	t6_L36
	movl	%ecx, %edx
t6_L32:
	movl	%ebp, %ecx
	movl	%edi, %ebx
	sarl	%cl, %ebx
	subl	%ebp, %edx
	movl	%ebx, 24(%eax)
	movl	%edx, 28(%eax)
	movl	$1, %eax
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	sall	%cl, %eax
	subl	$1, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	andl	%edi, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
t6_L41:
	.cfi_restore_state
t6_L33:
	.p2align 4,,7
	.p2align 3
t6_L39:
	jmp	t6_L39
	.cfi_endproc
t6_LFE21:
	.size	bits, .-bits
	.align 2
	.p2align 4,,15
	.type	decode, @function
decode:
t6_LFB23:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	xorl	%ebp, %ebp
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	xorl	%edi, %edi
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	movl	$1, %ebx
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	movl	24(%eax), %esi
	movl	%edx, 24(%esp)
	movl	28(%eax), %edx
	movl	$0, 12(%esp)
	movl	%ebx, (%esp)
	movl	%edx, 20(%esp)
	movl	24(%esp), %edx
	movl	(%edx), %edx
	addl	$2, %edx
	movl	%edx, 8(%esp)
	movl	20(%esp), %edx
t6_L52:
	testl	%edx, %edx
	jne	t6_L46
t6_L56:
	movb	$16, %dl
	subl	(%esp), %edx
	je	t6_L50
	movl	20(%eax), %ecx
	cmpl	16(%eax), %ecx
	je	t6_L53
	movl	12(%eax), %esi
	movzbl	(%esi,%ecx), %esi
	addl	$1, %ecx
	cmpl	$8, %edx
	movl	%ecx, 20(%eax)
	jle	t6_L52
	movl	$8, %edx
t6_L46:
	movl	8(%esp), %ebx
	movl	%esi, %ecx
	andl	$1, %ecx
	orl	%edi, %ecx
	sarl	%esi
	movswl	(%ebx), %edi
	addl	$2, %ebx
	movl	%ebx, 8(%esp)
	movl	%ecx, %ebx
	subl	%edi, %ebx
	cmpl	%ebx, %ebp
	jg	t6_L55
	addl	%edi, %ebp
	subl	$1, %edx
	addl	%edi, 12(%esp)
	addl	%ebp, %ebp
	addl	$1, (%esp)
	testl	%edx, %edx
	leal	(%ecx,%ecx), %edi
	jne	t6_L46
	jmp	t6_L56
	.p2align 4,,7
	.p2align 3
t6_L53:
	jmp	t6_L53
t6_L50:
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	$-10, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
t6_L55:
	.cfi_restore_state
	movl	(%esp), %ebx
	movl	20(%esp), %edx
	movl	%esi, 24(%eax)
	subl	%ebx, %edx
	andl	$7, %edx
	movl	%edx, 28(%eax)
	movl	%ecx, %edx
	movl	24(%esp), %ecx
	subl	%ebp, %edx
	addl	12(%esp), %edx
	movl	4(%ecx), %eax
	movswl	(%eax,%edx,2), %eax
	addl	$28, %esp
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE23:
	.size	decode, .-decode
	.align 2
	.p2align 4,,15
	.type	construct, @function
construct:
t6_LFB24:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%eax, %ebp
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$36, %esp
	.cfi_def_cfa_offset 56
	movl	(%eax), %ebx
	xorl	%eax, %eax
	.p2align 4,,7
	.p2align 3
t6_L58:
	movw	$0, (%ebx,%eax,2)
	addl	$1, %eax
	cmpl	$16, %eax
	jne	t6_L58
	testl	%ecx, %ecx
	movl	%edx, %eax
	leal	(%edx,%ecx,2), %edi
	jle	t6_L63
	.p2align 4,,7
	.p2align 3
t6_L70:
	movswl	(%eax), %esi
	addl	$2, %eax
	addw	$1, (%ebx,%esi,2)
	cmpl	%edi, %eax
	jne	t6_L70
t6_L63:
	movswl	(%ebx), %eax
	cmpl	%ecx, %eax
	je	t6_L67
	movl	$1, %eax
	movl	$1, %esi
	.p2align 4,,7
	.p2align 3
t6_L61:
	movswl	(%ebx,%esi,2), %edi
	addl	%eax, %eax
	subl	%edi, %eax
	js	t6_L60
	addl	$1, %esi
	cmpl	$16, %esi
	jne	t6_L61
	movw	$0, 6(%esp)
	xorl	%edi, %edi
	movw	$1, %si
	.p2align 4,,7
	.p2align 3
t6_L64:
	addl	$1, %esi
	addw	-2(%ebx,%esi,2), %di
	cmpl	$15, %esi
	movw	%di, 4(%esp,%esi,2)
	jne	t6_L64
	testl	%ecx, %ecx
	jle	t6_L60
	xorl	%ebx, %ebx
	movl	%eax, (%esp)
	.p2align 4,,7
	.p2align 3
t6_L66:
	movswl	(%edx,%ebx,2), %eax
	testw	%ax, %ax
	je	t6_L65
	movswl	4(%esp,%eax,2), %edi
	movl	4(%ebp), %esi
	movw	%bx, (%esi,%edi,2)
	movzwl	4(%esp,%eax,2), %esi
	addl	$1, %esi
	movw	%si, 4(%esp,%eax,2)
t6_L65:
	addl	$1, %ebx
	cmpl	%ecx, %ebx
	jne	t6_L66
	movl	(%esp), %eax
t6_L60:
	addl	$36, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L67:
	.cfi_restore_state
	addl	$36, %esp
	.cfi_def_cfa_offset 20
	xorl	%eax, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE24:
	.size	construct, .-construct
	.align 2
	.p2align 4,,15
	.type	codes, @function
codes:
t6_LFB25:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	movl	%eax, %ebx
	subl	$12, %esp
	.cfi_def_cfa_offset 32
	movl	%edx, 4(%esp)
	movl	%ecx, 8(%esp)
t6_L107:
	movl	4(%esp), %edx
	movl	%ebx, %eax
	call	decode
	testl	%eax, %eax
	movl	%eax, %edi
	js	t6_L82
	cmpl	$255, %eax
	jg	t6_L83
	movl	(%ebx), %eax
	testl	%eax, %eax
	je	t6_L84
	movl	8(%ebx), %edx
	cmpl	4(%ebx), %edx
	je	t6_L89
	movl	%edi, %ecx
	movb	%cl, (%eax,%edx)
t6_L84:
	addl	$1, 8(%ebx)
	jmp	t6_L107
	.p2align 4,,7
	.p2align 3
t6_L83:
	cmpl	$256, %eax
	je	t6_L93
	subl	$257, %edi
	cmpl	$28, %edi
	jg	t6_L94
	movswl	lext.1565(%edi,%edi), %edx
	movl	%ebx, %eax
	movswl	lens.1564(%edi,%edi), %esi
	call	bits
	movl	8(%esp), %edx
	addl	%eax, %esi
	movl	%ebx, %eax
	call	decode
	testl	%eax, %eax
	movl	%eax, %edi
	js	t6_L82
	movswl	dext.1567(%eax,%eax), %edx
	movswl	dists.1566(%eax,%eax), %ebp
	movl	%ebx, %eax
	call	bits
	movl	8(%ebx), %edx
	addl	%ebp, %eax
	cmpl	%edx, %eax
	ja	t6_L95
	movl	(%ebx), %ecx
	testl	%ecx, %ecx
	je	t6_L88
	leal	(%esi,%edx), %ebp
	cmpl	4(%ebx), %ebp
	ja	t6_L89
	testl	%esi, %esi
	leal	-1(%esi), %ebp
	je	t6_L92
	movl	%eax, (%esp)
	jmp	t6_L108
	.p2align 4,,7
	.p2align 3
t6_L112:
	movl	(%ebx), %ecx
t6_L108:
	movl	%edx, %esi
	subl	$1, %ebp
	subl	(%esp), %esi
	movzbl	(%ecx,%esi), %eax
	movb	%al, (%ecx,%edx)
	movl	8(%ebx), %edx
	addl	$1, %edx
	cmpl	$-1, %ebp
	movl	%edx, 8(%ebx)
	jne	t6_L112
t6_L92:
	cmpl	$256, %edi
	jne	t6_L107
	.p2align 4,,7
	.p2align 3
t6_L93:
	xorl	%edi, %edi
t6_L82:
	addl	$12, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%edi, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L88:
	.cfi_restore_state
	addl	%esi, %edx
	cmpl	$256, %edi
	movl	%edx, 8(%ebx)
	jne	t6_L107
	jmp	t6_L93
	.p2align 4,,7
	.p2align 3
t6_L89:
	movl	$1, %edi
	jmp	t6_L82
t6_L94:
	movl	$-10, %edi
	jmp	t6_L82
t6_L95:
	movl	$-11, %edi
	jmp	t6_L82
	.cfi_endproc
t6_LFE25:
	.size	codes, .-codes
	.align 2
	.p2align 4,,15
	.type	fixed, @function
fixed:
t6_LFB26:
	.cfi_startproc
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	movl	%eax, %esi
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	subl	$576, %esp
	.cfi_def_cfa_offset 588
	movl	virgin.1576, %eax
	testl	%eax, %eax
	je	t6_L114
	movl	$lencnt.1577, lencode.1581
	movl	%esp, %ebx
	movl	%esp, %edx
	movl	$lensym.1578, lencode.1581+4
	leal	288(%esp), %eax
	movl	$distcnt.1579, distcode.1582
	movl	$distsym.1580, distcode.1582+4
	.p2align 4,,7
	.p2align 3
t6_L115:
	movw	$8, (%edx)
	addl	$2, %edx
	cmpl	%eax, %edx
	jne	t6_L115
	leal	512(%esp), %eax
	.p2align 4,,7
	.p2align 3
t6_L116:
	movw	$9, (%edx)
	addl	$2, %edx
	cmpl	%eax, %edx
	jne	t6_L116
	leal	512(%esp), %eax
	leal	560(%esp), %edx
	.p2align 4,,7
	.p2align 3
t6_L117:
	movw	$7, (%eax)
	addl	$2, %eax
	cmpl	%edx, %eax
	jne	t6_L117
	movl	$288, %ecx
	movl	%esp, %edx
	movl	$lencode.1581, %eax
	movw	$8, 560(%esp)
	movw	$8, 562(%esp)
	movw	$8, 564(%esp)
	movw	$8, 566(%esp)
	movw	$8, 568(%esp)
	movw	$8, 570(%esp)
	movw	$8, 572(%esp)
	movw	$8, 574(%esp)
	call	construct
	leal	60(%esp), %eax
	.p2align 4,,7
	.p2align 3
t6_L118:
	movw	$5, (%ebx)
	addl	$2, %ebx
	cmpl	%eax, %ebx
	jne	t6_L118
	movl	$30, %ecx
	movl	%esp, %edx
	movl	$distcode.1582, %eax
	call	construct
	movl	$0, virgin.1576
t6_L114:
	movl	%esi, %eax
	movl	$distcode.1582, %ecx
	movl	$lencode.1581, %edx
	call	codes
	addl	$576, %esp
	.cfi_def_cfa_offset 12
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE26:
	.size	fixed, .-fixed
	.align 2
	.p2align 4,,15
	.type	dynamic, @function
dynamic:
t6_LFB27:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	$5, %edx
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movl	%eax, %esi
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$1352, %esp
	.cfi_def_cfa_offset 1372
	leal	24(%esp), %eax
	movl	%eax, 8(%esp)
	leal	148(%esp), %eax
	movl	%eax, 12(%esp)
	leal	56(%esp), %eax
	movl	%eax, 16(%esp)
	leal	88(%esp), %eax
	movl	%eax, 20(%esp)
	movl	%esi, %eax
	call	bits
	movl	$5, %edx
	addl	$257, %eax
	movl	%eax, (%esp)
	movl	%esi, %eax
	call	bits
	movl	$4, %edx
	addl	$1, %eax
	movl	%eax, 4(%esp)
	movl	%esi, %eax
	call	bits
	cmpl	$30, 4(%esp)
	leal	4(%eax), %edi
	jg	t6_L144
	cmpl	$286, (%esp)
	jg	t6_L144
	xorl	%ebx, %ebx
	testl	%edi, %edi
	jle	t6_L173
	.p2align 4,,7
	.p2align 3
t6_L159:
	movl	$3, %edx
	movl	%esi, %eax
	movswl	order.1615(%ebx,%ebx), %ebp
	call	bits
	addl	$1, %ebx
	cmpl	%ebx, %edi
	movw	%ax, 720(%esp,%ebp,2)
	jne	t6_L159
	cmpl	$18, %edi
	jg	t6_L174
	.p2align 4,,7
	.p2align 3
t6_L158:
	movswl	order.1615(%edi,%edi), %eax
	addl	$1, %edi
	cmpl	$18, %edi
	movw	$0, 720(%esp,%eax,2)
	jle	t6_L158
t6_L174:
	movl	$19, %ecx
	leal	720(%esp), %edx
	leal	8(%esp), %eax
	call	construct
	testl	%eax, %eax
	jne	t6_L175
	movl	(%esp), %edi
	xorl	%ebx, %ebx
	addl	4(%esp), %edi
	.p2align 4,,7
	.p2align 3
t6_L167:
	cmpl	%edi, %ebx
	jge	t6_L176
t6_L138:
	leal	8(%esp), %edx
	movl	%esi, %eax
	call	decode
	cmpl	$15, %eax
	jg	t6_L132
	movw	%ax, 720(%esp,%ebx,2)
	addl	$1, %ebx
	cmpl	%edi, %ebx
	jl	t6_L138
t6_L176:
	cmpw	$0, 1232(%esp)
	je	t6_L150
	movl	(%esp), %ecx
	leal	720(%esp), %edx
	leal	8(%esp), %eax
	call	construct
	cmpl	$0, %eax
	je	t6_L139
	jl	t6_L142
	movl	8(%esp), %eax
	movswl	(%eax), %edx
	movswl	2(%eax), %eax
	addl	%edx, %eax
	cmpl	(%esp), %eax
	jne	t6_L142
t6_L139:
	movl	(%esp), %eax
	movl	4(%esp), %ecx
	leal	720(%esp,%eax,2), %edx
	leal	16(%esp), %eax
	call	construct
	cmpl	$0, %eax
	je	t6_L140
	jl	t6_L143
	movl	16(%esp), %eax
	movswl	(%eax), %edx
	movswl	2(%eax), %eax
	addl	%edx, %eax
	cmpl	4(%esp), %eax
	jne	t6_L143
t6_L140:
	leal	16(%esp), %ecx
	movl	%esi, %eax
	leal	8(%esp), %edx
	call	codes
t6_L125:
	addl	$1352, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L132:
	.cfi_restore_state
	cmpl	$16, %eax
	je	t6_L177
	cmpl	$17, %eax
	je	t6_L178
	movl	$7, %edx
	movl	%esi, %eax
	call	bits
	leal	11(%eax), %edx
t6_L141:
	leal	(%ebx,%edx), %eax
	xorl	%ebp, %ebp
	cmpl	%edi, %eax
	jg	t6_L149
t6_L179:
	testl	%edx, %edx
	leal	-1(%edx), %ecx
	je	t6_L167
	leal	720(%esp,%ebx,2), %edx
	leal	720(%esp,%eax,2), %eax
	.p2align 4,,7
	.p2align 3
t6_L137:
	movw	%bp, (%edx)
	addl	$2, %edx
	cmpl	%eax, %edx
	jne	t6_L137
	leal	1(%ebx,%ecx), %ebx
	jmp	t6_L167
t6_L177:
	testl	%ebx, %ebx
	je	t6_L148
	movl	$2, %edx
	movl	%esi, %eax
	movswl	718(%esp,%ebx,2), %ebp
	call	bits
	leal	3(%eax), %edx
	leal	(%ebx,%edx), %eax
	cmpl	%edi, %eax
	jle	t6_L179
t6_L149:
	addl	$1352, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	$-6, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
t6_L178:
	.cfi_restore_state
	movl	$3, %edx
	movl	%esi, %eax
	call	bits
	leal	3(%eax), %edx
	jmp	t6_L141
t6_L148:
	addl	$1352, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	$-5, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
t6_L173:
	.cfi_restore_state
	xorl	%edi, %edi
	jmp	t6_L158
t6_L150:
	addl	$1352, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	$-9, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
t6_L142:
	.cfi_restore_state
	movl	$-7, %eax
	jmp	t6_L125
t6_L143:
	movl	$-8, %eax
	jmp	t6_L125
t6_L144:
	movl	$-3, %eax
	jmp	t6_L125
t6_L175:
	movl	$-4, %eax
	jmp	t6_L125
	.cfi_endproc
t6_LFE27:
	.size	dynamic, .-dynamic
	.align 2
	.p2align 4,,15
	.type	KeyExpansion, @function
KeyExpansion:
t6_LFB30:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	xorl	%ecx, %ecx
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$4, %esp
	.cfi_def_cfa_offset 24
t6_L181:
	movzbl	(%edx,%ecx,4), %ebx
	movb	%bl, (%eax,%ecx,4)
	movzbl	1(%edx,%ecx,4), %ebx
	movb	%bl, 1(%eax,%ecx,4)
	movzbl	2(%edx,%ecx,4), %ebx
	movb	%bl, 2(%eax,%ecx,4)
	movzbl	3(%edx,%ecx,4), %ebx
	movb	%bl, 3(%eax,%ecx,4)
	addl	$1, %ecx
	cmpl	$4, %ecx
	jne	t6_L181
	addl	$12, %eax
	.p2align 4,,7
	.p2align 3
t6_L183:
	movzbl	3(%eax), %edx
	testb	$3, %cl
	movzbl	(%eax), %edi
	movzbl	1(%eax), %esi
	movzbl	2(%eax), %ebx
	movb	%dl, 3(%esp)
	jne	t6_L182
	andl	$255, %esi
	andl	$255, %edi
	movzbl	sbox(%esi), %ebp
	movzbl	sbox(%ebx), %esi
	movzbl	sbox(%edx), %ebx
	movzbl	sbox(%edi), %edx
	movl	%ecx, %edi
	shrl	$2, %edi
	movb	%dl, 3(%esp)
	movzbl	Rcon(%edi), %edx
	xorl	%ebp, %edx
	movl	%edx, %edi
t6_L182:
	movzbl	-12(%eax), %edx
	addl	$1, %ecx
	xorb	-10(%eax), %bl
	xorl	%edx, %edi
	movl	%edi, %edx
	movb	%dl, 4(%eax)
	movzbl	-11(%eax), %edx
	movb	%bl, 6(%eax)
	xorl	%edx, %esi
	movl	%esi, %edx
	movb	%dl, 5(%eax)
	movzbl	3(%esp), %edx
	xorb	-9(%eax), %dl
	movb	%dl, 7(%eax)
	addl	$4, %eax
	cmpl	$44, %ecx
	jne	t6_L183
	addl	$4, %esp
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE30:
	.size	KeyExpansion, .-KeyExpansion
	.align 2
	.p2align 4,,15
	.type	Cipher, @function
Cipher:
t6_LFB42:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%eax, %ebp
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	movl	%edx, %eax
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	xorl	%edi, %edi
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$20, %esp
	.cfi_def_cfa_offset 40
t6_L188:
	leal	0(%ebp,%edi,4), %ecx
	leal	4(%eax), %esi
t6_L189:
	movzbl	(%eax), %ebx
	addl	$1, %eax
	xorb	%bl, (%ecx)
	addl	$1, %ecx
	cmpl	%esi, %eax
	jne	t6_L189
	addl	$1, %edi
	cmpl	$4, %edi
	jne	t6_L188
	leal	16(%edx), %eax
	addl	$160, %edx
	leal	16(%ebp), %ecx
	movl	%eax, 4(%esp)
	movl	%edx, 16(%esp)
	movl	%ecx, 12(%esp)
	.p2align 4,,7
	.p2align 3
t6_L190:
	movl	12(%esp), %esi
	xorl	%ecx, %ecx
t6_L198:
	leal	0(%ebp,%ecx), %eax
	leal	(%esi,%ecx), %ebx
t6_L191:
	movzbl	(%eax), %edx
	movzbl	sbox(%edx), %edx
	movb	%dl, (%eax)
	addl	$4, %eax
	cmpl	%eax, %ebx
	jne	t6_L191
	addl	$1, %ecx
	cmpl	$4, %ecx
	jne	t6_L198
	movzbl	9(%ebp), %eax
	movzbl	5(%ebp), %esi
	movzbl	1(%ebp), %edx
	movzbl	11(%ebp), %ecx
	movb	%al, 5(%ebp)
	movzbl	13(%ebp), %eax
	movl	%esi, %ebx
	movb	%bl, 1(%ebp)
	movzbl	10(%ebp), %ebx
	movb	%dl, 13(%ebp)
	movzbl	14(%ebp), %edx
	movb	%al, 9(%ebp)
	movzbl	2(%ebp), %eax
	movb	$4, 3(%esp)
	movb	%bl, 2(%ebp)
	movl	%ebp, 8(%esp)
	movb	%al, 10(%ebp)
	movzbl	6(%ebp), %eax
	movb	%dl, 6(%ebp)
	movzbl	15(%ebp), %edx
	movb	%cl, 15(%ebp)
	movzbl	7(%ebp), %ecx
	movb	%al, 14(%ebp)
	movzbl	3(%ebp), %eax
	movb	%dl, 3(%ebp)
	movb	%cl, 11(%ebp)
	movb	%al, 7(%ebp)
	movl	%ebp, %eax
t6_L194:
	movzbl	(%eax), %ecx
	xorl	%ebx, %edx
	xorl	%ecx, %esi
	movl	%ecx, %ebx
	movb	%cl, 2(%esp)
	movl	%esi, %ecx
	xorl	%esi, %edx
	shrb	$7, %cl
	addl	%esi, %esi
	movl	%ecx, %edi
	xorl	%edx, %ebx
	movzbl	2(%eax), %ecx
	leal	(%edi,%edi), %ebp
	addl	%ebp, %edi
	leal	(%edi,%edi,8), %edi
	xorl	%edi, %esi
	xorl	%esi, %ebx
	movb	%bl, (%eax)
	movzbl	1(%eax), %ebx
	xorl	%ebx, %ecx
	xorl	%edx, %ebx
	movl	%ecx, %esi
	shrb	$7, %cl
	addl	%esi, %esi
	movl	%ecx, %edi
	movzbl	3(%eax), %ecx
	leal	(%edi,%edi), %ebp
	addl	%ebp, %edi
	leal	(%edi,%edi,8), %edi
	xorl	%edi, %esi
	xorl	%esi, %ebx
	movb	%bl, 1(%eax)
	movzbl	2(%eax), %ebx
	xorl	%ebx, %ecx
	xorl	%edx, %ebx
	movl	%ecx, %esi
	shrb	$7, %cl
	addl	%esi, %esi
	movl	%ecx, %edi
	movzbl	2(%esp), %ecx
	leal	(%edi,%edi), %ebp
	addl	%ebp, %edi
	movzbl	3(%eax), %ebp
	leal	(%edi,%edi,8), %edi
	xorl	%edi, %esi
	xorl	%esi, %ebx
	xorl	%ebp, %ecx
	xorl	%ebp, %edx
	movb	%bl, 2(%eax)
	movl	%ecx, %ebx
	addl	%ecx, %ecx
	shrb	$7, %bl
	movl	%ebx, %esi
	leal	(%esi,%esi), %edi
	addl	%edi, %esi
	leal	(%esi,%esi,8), %esi
	xorl	%esi, %ecx
	xorl	%ecx, %edx
	movb	%dl, 3(%eax)
	addl	$4, %eax
	subb	$1, 3(%esp)
	je	t6_L204
	movzbl	1(%eax), %esi
	movzbl	2(%eax), %ebx
	movzbl	3(%eax), %edx
	jmp	t6_L194
	.p2align 4,,7
	.p2align 3
t6_L204:
	movl	8(%esp), %ebp
	xorl	%ebx, %ebx
	movl	4(%esp), %eax
t6_L193:
	leal	0(%ebp,%ebx,4), %edx
	leal	4(%eax), %esi
t6_L195:
	movzbl	(%eax), %ecx
	addl	$1, %eax
	xorb	%cl, (%edx)
	addl	$1, %edx
	cmpl	%esi, %eax
	jne	t6_L195
	addl	$1, %ebx
	cmpl	$4, %ebx
	jne	t6_L193
	addl	$16, 4(%esp)
	movl	16(%esp), %eax
	cmpl	%eax, 4(%esp)
	jne	t6_L190
	movl	12(%esp), %esi
	xorl	%ecx, %ecx
t6_L197:
	leal	0(%ebp,%ecx), %eax
	leal	(%esi,%ecx), %ebx
t6_L199:
	movzbl	(%eax), %edx
	movzbl	sbox(%edx), %edx
	movb	%dl, (%eax)
	addl	$4, %eax
	cmpl	%eax, %ebx
	jne	t6_L199
	addl	$1, %ecx
	cmpl	$4, %ecx
	jne	t6_L197
	movzbl	5(%ebp), %eax
	xorl	%esi, %esi
	movzbl	1(%ebp), %edx
	movb	%al, 1(%ebp)
	movzbl	9(%ebp), %eax
	movb	%al, 5(%ebp)
	movzbl	13(%ebp), %eax
	movb	%dl, 13(%ebp)
	movzbl	2(%ebp), %edx
	movb	%al, 9(%ebp)
	movzbl	10(%ebp), %eax
	movb	%dl, 10(%ebp)
	movzbl	6(%ebp), %edx
	movb	%al, 2(%ebp)
	movzbl	14(%ebp), %eax
	movb	%dl, 14(%ebp)
	movzbl	3(%ebp), %edx
	movb	%al, 6(%ebp)
	movzbl	15(%ebp), %eax
	movb	%al, 3(%ebp)
	movzbl	11(%ebp), %eax
	movb	%al, 15(%ebp)
	movzbl	7(%ebp), %eax
	movb	%dl, 7(%ebp)
	movb	%al, 11(%ebp)
	movl	16(%esp), %eax
t6_L201:
	leal	0(%ebp,%esi,4), %edx
	leal	4(%eax), %ebx
t6_L202:
	movzbl	(%eax), %ecx
	addl	$1, %eax
	xorb	%cl, (%edx)
	addl	$1, %edx
	cmpl	%ebx, %eax
	jne	t6_L202
	addl	$1, %esi
	cmpl	$4, %esi
	jne	t6_L201
	addl	$20, %esp
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE42:
	.size	Cipher, .-Cipher
	.align 2
	.p2align 4,,15
	.type	InvCipher, @function
InvCipher:
t6_LFB43:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%edx, %ecx
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	movl	%eax, %edi
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movl	%edx, %eax
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	addl	$160, %eax
	subl	$36, %esp
	.cfi_def_cfa_offset 56
	xorl	%ebx, %ebx
	movl	%edx, 24(%esp)
t6_L214:
	leal	(%edi,%ebx,4), %edx
	movl	%ecx, %ebp
	leal	4(%eax), %esi
	movl	%ecx, 12(%esp)
t6_L215:
	movzbl	(%eax), %ecx
	addl	$1, %eax
	xorb	%cl, (%edx)
	addl	$1, %edx
	cmpl	%esi, %eax
	jne	t6_L215
	addl	$1, %ebx
	movl	12(%esp), %ecx
	cmpl	$4, %ebx
	jne	t6_L214
	movl	24(%esp), %edx
	leal	16(%edi), %ecx
	movl	%ebp, 32(%esp)
	movl	%ecx, 28(%esp)
	addl	$144, %edx
	movl	%edx, 16(%esp)
	.p2align 4,,7
	.p2align 3
t6_L222:
	movzbl	9(%edi), %eax
	xorl	%ecx, %ecx
	movzbl	13(%edi), %edx
	movl	28(%esp), %esi
	movb	%al, 13(%edi)
	movzbl	5(%edi), %eax
	movb	%al, 9(%edi)
	movzbl	1(%edi), %eax
	movb	%dl, 1(%edi)
	movzbl	2(%edi), %edx
	movb	%al, 5(%edi)
	movzbl	10(%edi), %eax
	movb	%dl, 10(%edi)
	movzbl	6(%edi), %edx
	movb	%al, 2(%edi)
	movzbl	14(%edi), %eax
	movb	%dl, 14(%edi)
	movzbl	3(%edi), %edx
	movb	%al, 6(%edi)
	movzbl	7(%edi), %eax
	movb	%al, 3(%edi)
	movzbl	11(%edi), %eax
	movb	%al, 7(%edi)
	movzbl	15(%edi), %eax
	movb	%dl, 15(%edi)
	movb	%al, 11(%edi)
t6_L217:
	leal	(%edi,%ecx), %eax
	leal	(%esi,%ecx), %ebx
t6_L218:
	movzbl	(%eax), %edx
	movzbl	rsbox(%edx), %edx
	movb	%dl, (%eax)
	addl	$4, %eax
	cmpl	%eax, %ebx
	jne	t6_L218
	addl	$1, %ecx
	cmpl	$4, %ecx
	jne	t6_L217
	movl	16(%esp), %eax
	xorl	%ebx, %ebx
t6_L219:
	leal	(%edi,%ebx,4), %edx
	leal	4(%eax), %esi
t6_L220:
	movzbl	(%eax), %ecx
	addl	$1, %eax
	xorb	%cl, (%edx)
	addl	$1, %edx
	cmpl	%esi, %eax
	jne	t6_L220
	addl	$1, %ebx
	cmpl	$4, %ebx
	jne	t6_L219
	movl	%edi, %ecx
	movl	$0, 12(%esp)
	movl	%edi, 20(%esp)
t6_L221:
	movzbl	(%ecx), %eax
	movzbl	1(%ecx), %edx
	movb	%al, 9(%esp)
	movzbl	2(%ecx), %eax
	movb	%dl, 10(%esp)
	movb	%al, 8(%esp)
	movzbl	9(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	9(%esp), %edx
	leal	(%eax,%eax,8), %eax
	addl	%edx, %edx
	movb	%dl, 1(%esp)
	xorb	%al, 1(%esp)
	movzbl	1(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	1(%esp), %edx
	leal	(%eax,%eax,8), %eax
	addl	%edx, %edx
	movb	%dl, 2(%esp)
	xorb	%al, 2(%esp)
	movzbl	2(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	2(%esp), %edx
	leal	(%eax,%eax,8), %eax
	addl	%edx, %edx
	movb	%dl, 3(%esp)
	xorb	%al, 3(%esp)
	movzbl	10(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	10(%esp), %edx
	leal	(%eax,%eax,8), %eax
	leal	(%edx,%edx), %ebp
	xorl	%eax, %ebp
	movl	%ebp, %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	leal	(%ebp,%ebp), %edx
	movb	%dl, 4(%esp)
	leal	(%eax,%eax,8), %eax
	xorb	%al, 4(%esp)
	movzbl	4(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	4(%esp), %edx
	leal	(%eax,%eax,8), %eax
	addl	%edx, %edx
	movb	%dl, 5(%esp)
	xorb	%al, 5(%esp)
	movzbl	8(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	8(%esp), %edx
	leal	(%eax,%eax,8), %eax
	addl	%edx, %edx
	movb	%dl, 6(%esp)
	movzbl	3(%ecx), %ebx
	xorb	%al, 6(%esp)
	movzbl	6(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	6(%esp), %edx
	leal	(%eax,%eax,8), %eax
	leal	(%edx,%edx), %edi
	xorl	%eax, %edi
	movl	%edi, %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	leal	(%eax,%eax,8), %eax
	leal	(%edi,%edi), %esi
	xorl	%eax, %esi
	movzbl	3(%ecx), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	addl	%ebx, %ebx
	leal	(%eax,%eax,8), %eax
	xorl	%eax, %ebx
	movl	%ebx, %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	leal	(%ebx,%ebx), %edx
	movb	%dl, 7(%esp)
	leal	(%eax,%eax,8), %eax
	xorb	%al, 7(%esp)
	movzbl	7(%esp), %eax
	shrb	$7, %al
	leal	(%eax,%eax), %edx
	addl	%edx, %eax
	movzbl	7(%esp), %edx
	leal	(%eax,%eax,8), %eax
	addl	%edx, %edx
	movb	%dl, (%esp)
	movzbl	2(%esp), %edx
	xorb	%al, (%esp)
	movzbl	8(%esp), %eax
	xorb	10(%esp), %al
	xorb	3(%ecx), %al
	xorb	1(%esp), %dl
	xorl	%ebp, %eax
	xorl	%edi, %eax
	xorb	5(%esp), %al
	movb	%dl, 11(%esp)
	movzbl	3(%esp), %edx
	xorb	%dl, 11(%esp)
	xorl	%esi, %eax
	xorb	(%esp), %al
	xorb	11(%esp), %al
	movb	%al, (%ecx)
	movzbl	8(%esp), %eax
	xorb	9(%esp), %al
	xorb	3(%ecx), %al
	xorb	6(%esp), %al
	xorb	7(%esp), %al
	xorl	%edx, %eax
	movzbl	4(%esp), %edx
	xorl	%esi, %eax
	xorb	(%esp), %al
	xorl	%edx, %ebp
	movzbl	5(%esp), %edx
	xorl	%edx, %ebp
	xorl	%ebp, %eax
	movb	%al, 1(%ecx)
	movzbl	10(%esp), %eax
	xorb	9(%esp), %al
	movl	%eax, %ebp
	movzbl	3(%ecx), %eax
	xorl	%ebp, %eax
	xorl	%ebx, %eax
	xorb	2(%esp), %al
	xorb	3(%esp), %al
	xorb	7(%esp), %bl
	xorb	(%esp), %bl
	xorl	%edx, %eax
	movzbl	6(%esp), %edx
	xorb	(%esp), %al
	addl	$1, 12(%esp)
	xorl	%edx, %edi
	xorl	%esi, %edi
	xorl	%edi, %eax
	movb	%al, 2(%ecx)
	movzbl	8(%esp), %eax
	xorl	%ebp, %eax
	xorb	1(%esp), %al
	xorb	4(%esp), %al
	xorb	3(%esp), %al
	xorb	5(%esp), %al
	xorl	%esi, %eax
	xorl	%ebx, %eax
	movb	%al, 3(%ecx)
	addl	$4, %ecx
	cmpl	$4, 12(%esp)
	jne	t6_L221
	subl	$16, 16(%esp)
	movl	24(%esp), %edx
	cmpl	%edx, 16(%esp)
	movl	20(%esp), %edi
	jne	t6_L222
	movzbl	9(%edi), %eax
	xorl	%ecx, %ecx
	movzbl	13(%edi), %edx
	movl	28(%esp), %esi
	movb	%al, 13(%edi)
	movzbl	5(%edi), %eax
	movb	%al, 9(%edi)
	movzbl	1(%edi), %eax
	movb	%dl, 1(%edi)
	movzbl	2(%edi), %edx
	movb	%al, 5(%edi)
	movzbl	10(%edi), %eax
	movb	%dl, 10(%edi)
	movzbl	6(%edi), %edx
	movb	%al, 2(%edi)
	movzbl	14(%edi), %eax
	movb	%dl, 14(%edi)
	movzbl	3(%edi), %edx
	movb	%al, 6(%edi)
	movzbl	7(%edi), %eax
	movb	%al, 3(%edi)
	movzbl	11(%edi), %eax
	movb	%al, 7(%edi)
	movzbl	15(%edi), %eax
	movb	%dl, 15(%edi)
	movb	%al, 11(%edi)
t6_L223:
	leal	(%edi,%ecx), %eax
	leal	(%esi,%ecx), %ebx
t6_L224:
	movzbl	(%eax), %edx
	movzbl	rsbox(%edx), %edx
	movb	%dl, (%eax)
	addl	$4, %eax
	cmpl	%eax, %ebx
	jne	t6_L224
	addl	$1, %ecx
	cmpl	$4, %ecx
	jne	t6_L223
	movl	32(%esp), %eax
	xorl	%ebx, %ebx
t6_L225:
	leal	(%edi,%ebx,4), %edx
	leal	4(%eax), %esi
t6_L226:
	movzbl	(%eax), %ecx
	addl	$1, %eax
	xorb	%cl, (%edx)
	addl	$1, %edx
	cmpl	%esi, %eax
	jne	t6_L226
	addl	$1, %ebx
	cmpl	$4, %ebx
	jne	t6_L225
	addl	$36, %esp
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE43:
	.size	InvCipher, .-InvCipher
	.align 2
	.p2align 4,,15
	.globl	f3
	.type	f3, @function
f3:
t6_LFB13:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t6_LFE13:
	.size	f3, .-f3
	.align 2
	.p2align 4,,15
	.globl	f5
	.type	f5, @function
f5:
t6_LFB15:
	.cfi_startproc
	movl	4(%esp), %eax
t6_L242:
	cmpl	$9, %eax
	jle	t6_L243
t6_L241:
	rep
	ret
	.p2align 4,,7
	.p2align 3
t6_L243:
	addl	$1, %eax
	jmp	t6_L242
	.cfi_endproc
t6_LFE15:
	.size	f5, .-f5
	.align 2
	.p2align 4,,15
	.globl	f4
	.type	f4, @function
f4:
t6_LFB14:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t6_LFE14:
	.size	f4, .-f4
	.align 2
	.p2align 4,,15
	.globl	f2
	.type	f2, @function
f2:
t6_LFB12:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t6_LFE12:
	.size	f2, .-f2
	.align 2
	.p2align 4,,15
	.globl	f1
	.type	f1, @function
f1:
t6_LFB11:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t6_LFE11:
	.size	f1, .-f1
	.align 2
	.p2align 4,,15
	.globl	f6
	.type	f6, @function
f6:
t6_LFB16:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t6_LFE16:
	.size	f6, .-f6
	.align 2
	.p2align 4,,15
	.globl	f7
	.type	f7, @function
f7:
t6_LFB17:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t6_LFE17:
	.size	f7, .-f7
	.align 2
	.p2align 4,,15
	.globl	f8
	.type	f8, @function
f8:
t6_LFB18:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
t6_LFE18:
	.size	f8, .-f8
	.align 2
	.p2align 4,,15
	.globl	f9
	.type	f9, @function
f9:
t6_LFB19:
	.cfi_startproc
	movl	4(%esp), %eax
	testl	%eax, %eax
	jne	t6_L251
	xorl	%edx, %edx
t6_L254:
	addl	$1, %edx
	addl	$2, %eax
	cmpl	$9, %edx
	jle	t6_L254
	rep
	ret
	.p2align 4,,7
	.p2align 3
t6_L251:
	addl	$1, %eax
	movl	$6, %edx
	jmp	t6_L254
	.cfi_endproc
t6_LFE19:
	.size	f9, .-f9
	.align 2
	.p2align 4,,15
	.globl	crash_longjmp
	.type	crash_longjmp, @function
crash_longjmp:
t6_LFB20:
	.cfi_startproc
	.p2align 4,,7
	.p2align 3
t6_L258:
	jmp	t6_L258
	.cfi_endproc
t6_LFE20:
	.size	crash_longjmp, .-crash_longjmp
	.align 2
	.p2align 4,,15
	.globl	puff
	.type	puff, @function
puff:
t6_LFB28:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$56, %esp
	.cfi_def_cfa_offset 76
	movl	76(%esp), %eax
	movl	80(%esp), %ecx
	movl	88(%esp), %edi
	movl	$0, 28(%esp)
	movl	%eax, 20(%esp)
	movl	(%ecx), %eax
	movl	$0, 40(%esp)
	movl	$0, 44(%esp)
	movl	$0, 48(%esp)
	movl	%eax, 24(%esp)
	movl	84(%esp), %eax
	movl	%eax, 32(%esp)
	movl	(%edi), %eax
	movl	%eax, 36(%esp)
	.p2align 4,,7
	.p2align 3
t6_L273:
	movl	$1, %edx
	leal	20(%esp), %eax
	call	bits
	movl	$2, %edx
	movl	%eax, %ebx
	leal	20(%esp), %eax
	call	bits
	testl	%eax, %eax
	je	t6_L286
	cmpl	$1, %eax
	je	t6_L287
	cmpl	$2, %eax
	.p2align 4,,3
	je	t6_L288
	movl	$-1, %eax
t6_L271:
	movl	28(%esp), %edx
	movl	80(%esp), %ecx
	movl	88(%esp), %edi
	movl	%edx, (%ecx)
	movl	40(%esp), %edx
	movl	%edx, (%edi)
t6_L274:
	addl	$56, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L286:
	.cfi_restore_state
	movl	40(%esp), %edx
	movl	36(%esp), %esi
	movl	$0, 44(%esp)
	movl	$0, 48(%esp)
	leal	4(%edx), %eax
	cmpl	%esi, %eax
	jbe	t6_L261
t6_L265:
	addl	$56, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	$2, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L287:
	.cfi_restore_state
	leal	20(%esp), %eax
	call	fixed
t6_L269:
	cmpl	$0, %eax
	jne	t6_L272
t6_L283:
	testl	%ebx, %ebx
	je	t6_L273
	.p2align 4,,6
	jmp	t6_L271
	.p2align 4,,7
	.p2align 3
t6_L261:
	movl	32(%esp), %ecx
	leal	1(%edx), %ebp
	movzbl	(%ecx,%edx), %edi
	movl	%ebp, 40(%esp)
	movzbl	1(%ecx,%edx), %ebp
	sall	$8, %ebp
	orl	%edi, %ebp
	leal	2(%edx), %edi
	movl	%edi, 40(%esp)
	movzbl	2(%ecx,%edx), %edi
	movl	%edi, 4(%esp)
	movl	%ebp, %edi
	notl	%edi
	movl	%edi, (%esp)
	leal	3(%edx), %edi
	movl	%edi, 40(%esp)
	movzbl	(%esp), %edi
	cmpl	%edi, 4(%esp)
	jne	t6_L264
	movzbl	3(%ecx,%edx), %edx
	movl	%eax, 40(%esp)
	movl	%edx, 4(%esp)
	movl	(%esp), %edx
	movzbl	%dh, %edi
	cmpl	%edi, 4(%esp)
	jne	t6_L264
	leal	0(%ebp,%eax), %edi
	cmpl	%edi, %esi
	movl	%edi, (%esp)
	jb	t6_L265
	movl	20(%esp), %esi
	testl	%esi, %esi
	je	t6_L266
	movl	28(%esp), %edx
	leal	0(%ebp,%edx), %edi
	cmpl	24(%esp), %edi
	ja	t6_L276
	testl	%ebp, %ebp
	je	t6_L275
	movl	(%esp), %edi
	jmp	t6_L281
	.p2align 4,,7
	.p2align 3
t6_L289:
	movl	20(%esp), %esi
	movl	32(%esp), %ecx
t6_L281:
	movzbl	(%ecx,%eax), %ecx
	addl	$1, %eax
	movb	%cl, (%esi,%edx)
	addl	$1, %edx
	cmpl	%eax, %edi
	movl	%edx, 28(%esp)
	movl	%eax, 40(%esp)
	jne	t6_L289
t6_L275:
	xorl	%eax, %eax
	jmp	t6_L283
	.p2align 4,,7
	.p2align 3
t6_L288:
	leal	20(%esp), %eax
	call	dynamic
	.p2align 4,,2
	jmp	t6_L269
	.p2align 4,,7
	.p2align 3
t6_L264:
	movl	$-2, %eax
	jmp	t6_L271
t6_L276:
	addl	$56, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	$1, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
t6_L266:
	.cfi_restore_state
	movl	(%esp), %eax
	addl	%ebp, 28(%esp)
	movl	%eax, 40(%esp)
	xorl	%eax, %eax
	jmp	t6_L283
t6_L272:
	jg	t6_L274
	jmp	t6_L271
	.cfi_endproc
t6_LFE28:
	.size	puff, .-puff
	.align 2
	.p2align 4,,15
	.globl	my_memcpy
	.type	my_memcpy, @function
my_memcpy:
t6_LFB29:
	.cfi_startproc
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	xorl	%eax, %eax
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	movl	20(%esp), %ecx
	movl	12(%esp), %esi
	movl	16(%esp), %ebx
	testl	%ecx, %ecx
	je	t6_L290
	.p2align 4,,7
	.p2align 3
t6_L294:
	movzbl	(%ebx,%eax), %edx
	movb	%dl, (%esi,%eax)
	addl	$1, %eax
	cmpl	%ecx, %eax
	jne	t6_L294
t6_L290:
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE29:
	.size	my_memcpy, .-my_memcpy
	.align 2
	.p2align 4,,15
	.globl	AES_init_ctx
	.type	AES_init_ctx, @function
AES_init_ctx:
t6_LFB31:
	.cfi_startproc
	movl	8(%esp), %edx
	movl	4(%esp), %eax
	jmp	KeyExpansion
	.cfi_endproc
t6_LFE31:
	.size	AES_init_ctx, .-AES_init_ctx
	.align 2
	.p2align 4,,15
	.globl	AES_init_ctx_iv
	.type	AES_init_ctx_iv, @function
AES_init_ctx_iv:
t6_LFB32:
	.cfi_startproc
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	movl	12(%esp), %ebx
	movl	16(%esp), %edx
	movl	20(%esp), %esi
	movl	%ebx, %eax
	call	KeyExpansion
	xorl	%edx, %edx
	.p2align 4,,7
	.p2align 3
t6_L299:
	movzbl	(%esi,%edx), %ecx
	movb	%cl, 176(%ebx,%edx)
	addl	$1, %edx
	cmpl	$16, %edx
	jne	t6_L299
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE32:
	.size	AES_init_ctx_iv, .-AES_init_ctx_iv
	.align 2
	.p2align 4,,15
	.globl	AES_ctx_set_iv
	.type	AES_ctx_set_iv, @function
AES_ctx_set_iv:
t6_LFB33:
	.cfi_startproc
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	movl	8(%esp), %ebx
	xorl	%eax, %eax
	movl	12(%esp), %ecx
	.p2align 4,,7
	.p2align 3
t6_L303:
	movzbl	(%ecx,%eax), %edx
	movb	%dl, 176(%ebx,%eax)
	addl	$1, %eax
	cmpl	$16, %eax
	jne	t6_L303
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE33:
	.size	AES_ctx_set_iv, .-AES_ctx_set_iv
	.align 2
	.p2align 4,,15
	.globl	AES_ECB_encrypt
	.type	AES_ECB_encrypt, @function
AES_ECB_encrypt:
t6_LFB44:
	.cfi_startproc
	movl	4(%esp), %edx
	movl	8(%esp), %eax
	jmp	Cipher
	.cfi_endproc
t6_LFE44:
	.size	AES_ECB_encrypt, .-AES_ECB_encrypt
	.align 2
	.p2align 4,,15
	.globl	AES_ECB_decrypt
	.type	AES_ECB_decrypt, @function
AES_ECB_decrypt:
t6_LFB45:
	.cfi_startproc
	movl	4(%esp), %edx
	movl	8(%esp), %eax
	jmp	InvCipher
	.cfi_endproc
t6_LFE45:
	.size	AES_ECB_decrypt, .-AES_ECB_decrypt
	.align 2
	.p2align 4,,15
	.globl	AES_CBC_encrypt_buffer
	.type	AES_CBC_encrypt_buffer, @function
AES_CBC_encrypt_buffer:
t6_LFB47:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	movl	28(%esp), %esi
	movl	20(%esp), %ebp
	movl	24(%esp), %edi
	testl	%esi, %esi
	leal	176(%ebp), %edx
	movl	%edi, %ebx
	je	t6_L310
	.p2align 4,,7
	.p2align 3
t6_L317:
	xorl	%ecx, %ecx
	.p2align 4,,7
	.p2align 3
t6_L311:
	movzbl	(%edx,%ecx), %eax
	xorb	%al, (%ebx,%ecx)
	addl	$1, %ecx
	cmpl	$16, %ecx
	jne	t6_L311
	movl	%ebp, %edx
	movl	%ebx, %eax
	call	Cipher
	leal	16(%ebx), %eax
	movl	%ebx, %edx
	movl	%eax, %ecx
	subl	%edi, %ecx
	cmpl	%ecx, %esi
	jbe	t6_L310
	movl	%eax, %ebx
	jmp	t6_L317
t6_L310:
	xorl	%eax, %eax
	.p2align 4,,7
	.p2align 3
t6_L313:
	movzbl	(%edx,%eax), %ecx
	movb	%cl, 176(%ebp,%eax)
	addl	$1, %eax
	cmpl	$16, %eax
	jne	t6_L313
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE47:
	.size	AES_CBC_encrypt_buffer, .-AES_CBC_encrypt_buffer
	.align 2
	.p2align 4,,15
	.globl	AES_CBC_decrypt_buffer
	.type	AES_CBC_decrypt_buffer, @function
AES_CBC_decrypt_buffer:
t6_LFB48:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	xorl	%edi, %edi
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$16, %esp
	.cfi_def_cfa_offset 36
	movl	44(%esp), %eax
	movl	36(%esp), %esi
	movl	40(%esp), %ebp
	testl	%eax, %eax
	je	t6_L320
	.p2align 4,,7
	.p2align 3
t6_L330:
	leal	0(%ebp,%edi), %ebx
	xorl	%eax, %eax
	.p2align 4,,7
	.p2align 3
t6_L322:
	movzbl	(%ebx,%eax), %edx
	movb	%dl, (%esp,%eax)
	addl	$1, %eax
	cmpl	$16, %eax
	jne	t6_L322
	movl	%ebx, %eax
	movl	%esi, %edx
	call	InvCipher
	xorl	%eax, %eax
	.p2align 4,,7
	.p2align 3
t6_L323:
	movzbl	176(%esi,%eax), %edx
	xorb	%dl, (%ebx,%eax)
	addl	$1, %eax
	cmpl	$16, %eax
	jne	t6_L323
	xorb	%al, %al
	.p2align 4,,7
	.p2align 3
t6_L324:
	movzbl	(%esp,%eax), %edx
	movb	%dl, 176(%esi,%eax)
	addl	$1, %eax
	cmpl	$16, %eax
	jne	t6_L324
	addl	$16, %edi
	cmpl	%edi, 44(%esp)
	ja	t6_L330
t6_L320:
	addl	$16, %esp
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
t6_LFE48:
	.size	AES_CBC_decrypt_buffer, .-AES_CBC_decrypt_buffer
	.align 2
	.p2align 4,,15
	.globl	AES_CTR_xcrypt_buffer
	.type	AES_CTR_xcrypt_buffer, @function
AES_CTR_xcrypt_buffer:
t6_LFB49:
	.cfi_startproc
	pushl	%edi
	.cfi_def_cfa_offset 8
	.cfi_offset 7, -8
	movl	$16, %eax
	pushl	%esi
	.cfi_def_cfa_offset 12
	.cfi_offset 6, -12
	pushl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	subl	$16, %esp
	.cfi_def_cfa_offset 32
	movl	40(%esp), %edx
	movl	36(%esp), %esi
	movl	32(%esp), %ebx
	testl	%edx, %edx
	leal	(%esi,%edx), %edi
	jne	t6_L341
	jmp	t6_L332
	.p2align 4,,7
	.p2align 3
t6_L347:
	movl	%eax, %edx
	addl	$1, %eax
	movzbl	(%esp,%edx), %edx
	xorb	%dl, (%esi)
	addl	$1, %esi
	cmpl	%edi, %esi
	je	t6_L332
t6_L341:
	cmpl	$16, %eax
	jne	t6_L347
	xorl	%eax, %eax
	.p2align 4,,7
	.p2align 3
t6_L334:
	movzbl	176(%ebx,%eax), %ecx
	movb	%cl, (%esp,%eax)
	addl	$1, %eax
	cmpl	$16, %eax
	jne	t6_L334
	movl	%esp, %eax
	movl	%ebx, %edx
	call	Cipher
	movl	$15, %eax
t6_L337:
	movzbl	176(%ebx,%eax), %edx
	cmpb	$-1, %dl
	je	t6_L348
	addl	$1, %edx
	movb	%dl, 176(%ebx,%eax)
t6_L346:
	xorl	%edx, %edx
	movl	$1, %eax
	movzbl	(%esp,%edx), %edx
	xorb	%dl, (%esi)
	addl	$1, %esi
	cmpl	%edi, %esi
	jne	t6_L341
t6_L332:
	addl	$16, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 12
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 8
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L348:
	.cfi_restore_state
	movb	$0, 176(%ebx,%eax)
	subl	$1, %eax
	cmpl	$-1, %eax
	jne	t6_L337
	jmp	t6_L346
	.cfi_endproc
t6_LFE49:
	.size	AES_CTR_xcrypt_buffer, .-AES_CTR_xcrypt_buffer
	.align 2
	.p2align 4,,15
	.globl	test_6
	.type	test_6, @function
test_6:
t6_LFB50:
	.cfi_startproc
	pushl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	xorl	%ebx, %ebx
	subl	$248, %esp
	.cfi_def_cfa_offset 256
	movl	target_list, %eax
	testl	%eax, %eax
	je	t6_L353
t6_L362:
	addl	$1, %ebx
	call	*%eax
	movl	target_list(,%ebx,4), %eax
	testl	%eax, %eax
	jne	t6_L362
t6_L353:
	movl	$input_len, 12(%esp)
	movl	$input_data, 8(%esp)
	movl	$output_len, 4(%esp)
	movl	$output_data, (%esp)
	call	puff
	testl	%eax, %eax
	movl	%eax, %ebx
	je	t6_L371
t6_L351:
	.p2align 4,,7
	.p2align 3
t6_L367:
	jmp	t6_L367
t6_L371:
	cmpl	$630, output_len
	jne	t6_L351
	cmpb	$83, output_data+90
	jne	t6_L351
	cmpb	$114, output_data+626
	jne	t6_L351
	xorl	%eax, %eax
t6_L354:
	movzbl	output_data(%eax), %edx
	movb	%dl, 16(%esp,%eax)
	addl	$1, %eax
	cmpl	$32, %eax
	jne	t6_L354
	movl	$input_data, %edx
	leal	48(%esp), %eax
	call	KeyExpansion
	leal	48(%esp), %edx
	leal	16(%esp), %eax
	call	Cipher
	cmpb	$-108, 17(%esp)
	je	t6_L372
t6_L355:
	.p2align 4,,7
	.p2align 3
t6_L368:
	jmp	t6_L368
t6_L372:
	cmpb	$58, 23(%esp)
	jne	t6_L355
	leal	48(%esp), %edx
	leal	16(%esp), %eax
	call	InvCipher
t6_L359:
	movzbl	16(%esp,%ebx), %edx
	movsbl	output_data(%ebx), %eax
	cmpl	%eax, %edx
	jne	t6_L369
	addl	$1, %ebx
	cmpl	$32, %ebx
	jne	t6_L359
	addl	$248, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	xorl	%eax, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
t6_L369:
	.cfi_restore_state
	jmp	t6_L369
	.cfi_endproc
t6_LFE50:
	.size	test_6, .-test_6
	.globl	output_len
	.data
	.align 4
	.type	output_len, @object
	.size	output_len, 4
output_len:
	.long	700
	.globl	input_len
	.align 4
	.type	input_len, @object
	.size	input_len, 4
input_len:
	.long	376
	.globl	input_data
	.section	.rodata
	.align 32
	.type	input_data, @object
	.size	input_data, 377
input_data:
	.byte	85
	.byte	82
	.byte	-53
	.byte	-86
	.byte	-37
	.byte	48
	.byte	16
	.byte	-35
	.byte	-21
	.byte	43
	.byte	102
	.byte	-41
	.byte	-115
	.byte	-55
	.byte	15
	.byte	116
	.byte	21
	.byte	66
	.byte	-95
	.byte	89
	.byte	92
	.byte	10
	.byte	-73
	.byte	23
	.byte	66
	.byte	-105
	.byte	19
	.byte	107
	.byte	108
	.byte	-85
	.byte	-111
	.byte	103
	.byte	-126
	.byte	30
	.byte	49
	.byte	-2
	.byte	-5
	.byte	30
	.byte	-55
	.byte	-67
	.byte	-48
	.byte	110
	.byte	36
	.byte	13
	.byte	-102
	.byte	-93
	.byte	-13
	.byte	24
	.byte	-35
	.byte	22
	.byte	-93
	.byte	-111
	.byte	-107
	.byte	-118
	.byte	-60
	.byte	72
	.byte	-85
	.byte	80
	.byte	-104
	.byte	104
	.byte	19
	.byte	90
	.byte	-8
	.byte	-123
	.byte	69
	.byte	-80
	.byte	-22
	.byte	-32
	.byte	110
	.byte	127
	.byte	91
	.byte	50
	.byte	-17
	.byte	84
	.byte	22
	.byte	-95
	.byte	13
	.byte	123
	.byte	40
	.byte	-108
	.byte	23
	.byte	-85
	.byte	-47
	.byte	-45
	.byte	93
	.byte	-66
	.byte	-70
	.byte	55
	.byte	51
	.byte	-115
	.byte	97
	.byte	94
	.byte	10
	.byte	45
	.byte	22
	.byte	-29
	.byte	62
	.byte	-12
	.byte	-74
	.byte	-97
	.byte	-4
	.byte	124
	.byte	2
	.byte	121
	.byte	-79
	.byte	85
	.byte	-54
	.byte	-32
	.byte	-50
	.byte	58
	.byte	75
	.byte	-4
	.byte	-110
	.byte	-63
	.byte	-61
	.byte	41
	.byte	3
	.byte	20
	.byte	109
	.byte	35
	.byte	-90
	.byte	-110
	.byte	68
	.byte	78
	.byte	-18
	.byte	-105
	.byte	85
	.byte	42
	.byte	28
	.byte	31
	.byte	100
	.byte	83
	.byte	71
	.byte	-34
	.byte	-109
	.byte	112
	.byte	47
	.byte	86
	.byte	75
	.byte	26
	.byte	116
	.byte	118
	.byte	-25
	.byte	76
	.byte	59
	.byte	122
	.byte	94
	.byte	65
	.byte	-74
	.byte	-34
	.byte	-96
	.byte	-40
	.byte	-71
	.byte	38
	.byte	75
	.byte	60
	.byte	-72
	.byte	75
	.byte	-76
	.byte	-22
	.byte	41
	.byte	40
	.byte	-115
	.byte	41
	.byte	-84
	.byte	-39
	.byte	-12
	.byte	32
	.byte	127
	.byte	-56
	.byte	-34
	.byte	30
	.byte	-8
	.byte	-76
	.byte	-16
	.byte	67
	.byte	-123
	.byte	-94
	.byte	-63
	.byte	-45
	.byte	-56
	.byte	-23
	.byte	37
	.byte	-67
	.byte	-99
	.byte	71
	.byte	-2
	.byte	109
	.byte	-11
	.byte	-44
	.byte	-81
	.byte	-118
	.byte	-63
	.byte	8
	.byte	-52
	.byte	3
	.byte	113
	.byte	89
	.byte	-126
	.byte	50
	.byte	-67
	.byte	91
	.byte	-106
	.byte	124
	.byte	-64
	.byte	32
	.byte	18
	.byte	70
	.byte	51
	.byte	-85
	.byte	39
	.byte	109
	.byte	22
	.byte	51
	.byte	-75
	.byte	-93
	.byte	-31
	.byte	-90
	.byte	-105
	.byte	-125
	.byte	-5
	.byte	-58
	.byte	-87
	.byte	44
	.byte	112
	.byte	22
	.byte	57
	.byte	23
	.byte	122
	.byte	-122
	.byte	-79
	.byte	-44
	.byte	36
	.byte	-121
	.byte	8
	.byte	105
	.byte	-115
	.byte	19
	.byte	9
	.byte	36
	.byte	52
	.byte	23
	.byte	-33
	.byte	107
	.byte	103
	.byte	8
	.byte	-22
	.byte	-61
	.byte	108
	.byte	-3
	.byte	-107
	.byte	123
	.byte	-84
	.byte	-80
	.byte	127
	.byte	-18
	.byte	126
	.byte	86
	.byte	100
	.byte	72
	.byte	81
	.byte	-40
	.byte	-25
	.byte	-90
	.byte	-92
	.byte	-104
	.byte	-69
	.byte	-103
	.byte	-31
	.byte	12
	.byte	-128
	.byte	71
	.byte	28
	.byte	-21
	.byte	65
	.byte	123
	.byte	-91
	.byte	9
	.byte	1
	.byte	-37
	.byte	-10
	.byte	9
	.byte	-38
	.byte	44
	.byte	65
	.byte	-7
	.byte	-58
	.byte	1
	.byte	-78
	.byte	38
	.byte	75
	.byte	64
	.byte	98
	.byte	24
	.byte	-5
	.byte	-127
	.byte	24
	.byte	104
	.byte	-35
	.byte	-37
	.byte	-84
	.byte	78
	.byte	-18
	.byte	74
	.byte	15
	.byte	-75
	.byte	-106
	.byte	28
	.byte	-105
	.byte	54
	.byte	-33
	.byte	-21
	.byte	-65
	.byte	-29
	.byte	117
	.byte	31
	.byte	-117
	.byte	36
	.byte	-116
	.byte	61
	.byte	-109
	.byte	90
	.byte	-127
	.byte	-9
	.byte	25
	.byte	-44
	.byte	-28
	.byte	37
	.byte	-121
	.byte	4
	.byte	101
	.byte	-17
	.byte	-36
	.byte	-126
	.byte	2
	.byte	111
	.byte	10
	.byte	47
	.byte	73
	.byte	3
	.byte	-78
	.byte	56
	.byte	8
	.byte	33
	.byte	106
	.byte	51
	.byte	-11
	.byte	-110
	.byte	-36
	.byte	27
	.byte	-2
	.byte	-62
	.byte	93
	.byte	-24
	.byte	-55
	.byte	-119
	.byte	125
	.byte	-56
	.byte	77
	.byte	55
	.byte	52
	.byte	52
	.byte	-58
	.byte	44
	.byte	-46
	.byte	51
	.byte	-56
	.byte	85
	.byte	-1
	.byte	43
	.byte	11
	.byte	-58
	.byte	127
	.byte	114
	.byte	127
	.byte	0
	.byte	-13
	.comm	output_data,700,32
	.globl	target_list
	.data
	.align 32
	.type	target_list, @object
	.size	target_list, 104
target_list:
	.long	target1
	.long	target2
	.long	target3
	.long	target4
	.long	target2
	.long	target2
	.long	target2
	.long	target1
	.long	target2
	.long	target4
	.long	target3
	.long	target4
	.long	target5
	.long	target3
	.long	target1
	.long	target2
	.long	target6
	.long	target3
	.long	target7
	.long	target1
	.long	target3
	.long	target8
	.long	target9
	.long	targeta
	.long	targetb
	.long	0
	.globl	t3
	.align 4
	.type	t3, @object
	.size	t3, 4
t3:
	.long	1088421888
	.globl	t2
	.align 8
	.type	t2, @object
	.size	t2, 8
t2:
	.long	0
	.long	1075314688
	.globl	t1
	.align 16
	.type	t1, @object
	.size	t1, 12
t1:
	.long	0
	.long	-1610612736
	.long	16385
	.globl	x2
	.align 4
	.type	x2, @object
	.size	x2, 4
x2:
	.long	4
	.globl	x1
	.align 4
	.type	x1, @object
	.size	x1, 4
x1:
	.long	3
	.globl	below
	.align 8
	.type	below, @object
	.size	below, 8
below:
	.long	2
	.long	0
	.globl	above
	.align 8
	.type	above, @object
	.size	above, 8
above:
	.long	1
	.long	0
	.align 4
	.type	virgin.1576, @object
	.size	virgin.1576, 4
virgin.1576:
	.long	1
	.local	lencode.1581
	.comm	lencode.1581,8,4
	.local	lencnt.1577
	.comm	lencnt.1577,32,32
	.local	lensym.1578
	.comm	lensym.1578,576,32
	.local	distcode.1582
	.comm	distcode.1582,8,4
	.local	distcnt.1579
	.comm	distcnt.1579,32,32
	.local	distsym.1580
	.comm	distsym.1580,60,32
	.section	.rodata
	.align 32
	.type	lens.1564, @object
	.size	lens.1564, 58
lens.1564:
	.value	3
	.value	4
	.value	5
	.value	6
	.value	7
	.value	8
	.value	9
	.value	10
	.value	11
	.value	13
	.value	15
	.value	17
	.value	19
	.value	23
	.value	27
	.value	31
	.value	35
	.value	43
	.value	51
	.value	59
	.value	67
	.value	83
	.value	99
	.value	115
	.value	131
	.value	163
	.value	195
	.value	227
	.value	258
	.align 32
	.type	lext.1565, @object
	.size	lext.1565, 58
lext.1565:
	.value	0
	.value	0
	.value	0
	.value	0
	.value	0
	.value	0
	.value	0
	.value	0
	.value	1
	.value	1
	.value	1
	.value	1
	.value	2
	.value	2
	.value	2
	.value	2
	.value	3
	.value	3
	.value	3
	.value	3
	.value	4
	.value	4
	.value	4
	.value	4
	.value	5
	.value	5
	.value	5
	.value	5
	.value	0
	.align 32
	.type	dists.1566, @object
	.size	dists.1566, 60
dists.1566:
	.value	1
	.value	2
	.value	3
	.value	4
	.value	5
	.value	7
	.value	9
	.value	13
	.value	17
	.value	25
	.value	33
	.value	49
	.value	65
	.value	97
	.value	129
	.value	193
	.value	257
	.value	385
	.value	513
	.value	769
	.value	1025
	.value	1537
	.value	2049
	.value	3073
	.value	4097
	.value	6145
	.value	8193
	.value	12289
	.value	16385
	.value	24577
	.align 32
	.type	dext.1567, @object
	.size	dext.1567, 60
dext.1567:
	.value	0
	.value	0
	.value	0
	.value	0
	.value	1
	.value	1
	.value	2
	.value	2
	.value	3
	.value	3
	.value	4
	.value	4
	.value	5
	.value	5
	.value	6
	.value	6
	.value	7
	.value	7
	.value	8
	.value	8
	.value	9
	.value	9
	.value	10
	.value	10
	.value	11
	.value	11
	.value	12
	.value	12
	.value	13
	.value	13
	.align 32
	.type	order.1615, @object
	.size	order.1615, 38
order.1615:
	.value	16
	.value	17
	.value	18
	.value	0
	.value	8
	.value	7
	.value	9
	.value	6
	.value	10
	.value	5
	.value	11
	.value	4
	.value	12
	.value	3
	.value	13
	.value	2
	.value	14
	.value	1
	.value	15
	.align 32
	.type	sbox, @object
	.size	sbox, 256
sbox:
	.byte	99
	.byte	124
	.byte	119
	.byte	123
	.byte	-14
	.byte	107
	.byte	111
	.byte	-59
	.byte	48
	.byte	1
	.byte	103
	.byte	43
	.byte	-2
	.byte	-41
	.byte	-85
	.byte	118
	.byte	-54
	.byte	-126
	.byte	-55
	.byte	125
	.byte	-6
	.byte	89
	.byte	71
	.byte	-16
	.byte	-83
	.byte	-44
	.byte	-94
	.byte	-81
	.byte	-100
	.byte	-92
	.byte	114
	.byte	-64
	.byte	-73
	.byte	-3
	.byte	-109
	.byte	38
	.byte	54
	.byte	63
	.byte	-9
	.byte	-52
	.byte	52
	.byte	-91
	.byte	-27
	.byte	-15
	.byte	113
	.byte	-40
	.byte	49
	.byte	21
	.byte	4
	.byte	-57
	.byte	35
	.byte	-61
	.byte	24
	.byte	-106
	.byte	5
	.byte	-102
	.byte	7
	.byte	18
	.byte	-128
	.byte	-30
	.byte	-21
	.byte	39
	.byte	-78
	.byte	117
	.byte	9
	.byte	-125
	.byte	44
	.byte	26
	.byte	27
	.byte	110
	.byte	90
	.byte	-96
	.byte	82
	.byte	59
	.byte	-42
	.byte	-77
	.byte	41
	.byte	-29
	.byte	47
	.byte	-124
	.byte	83
	.byte	-47
	.byte	0
	.byte	-19
	.byte	32
	.byte	-4
	.byte	-79
	.byte	91
	.byte	106
	.byte	-53
	.byte	-66
	.byte	57
	.byte	74
	.byte	76
	.byte	88
	.byte	-49
	.byte	-48
	.byte	-17
	.byte	-86
	.byte	-5
	.byte	67
	.byte	77
	.byte	51
	.byte	-123
	.byte	69
	.byte	-7
	.byte	2
	.byte	127
	.byte	80
	.byte	60
	.byte	-97
	.byte	-88
	.byte	81
	.byte	-93
	.byte	64
	.byte	-113
	.byte	-110
	.byte	-99
	.byte	56
	.byte	-11
	.byte	-68
	.byte	-74
	.byte	-38
	.byte	33
	.byte	16
	.byte	-1
	.byte	-13
	.byte	-46
	.byte	-51
	.byte	12
	.byte	19
	.byte	-20
	.byte	95
	.byte	-105
	.byte	68
	.byte	23
	.byte	-60
	.byte	-89
	.byte	126
	.byte	61
	.byte	100
	.byte	93
	.byte	25
	.byte	115
	.byte	96
	.byte	-127
	.byte	79
	.byte	-36
	.byte	34
	.byte	42
	.byte	-112
	.byte	-120
	.byte	70
	.byte	-18
	.byte	-72
	.byte	20
	.byte	-34
	.byte	94
	.byte	11
	.byte	-37
	.byte	-32
	.byte	50
	.byte	58
	.byte	10
	.byte	73
	.byte	6
	.byte	36
	.byte	92
	.byte	-62
	.byte	-45
	.byte	-84
	.byte	98
	.byte	-111
	.byte	-107
	.byte	-28
	.byte	121
	.byte	-25
	.byte	-56
	.byte	55
	.byte	109
	.byte	-115
	.byte	-43
	.byte	78
	.byte	-87
	.byte	108
	.byte	86
	.byte	-12
	.byte	-22
	.byte	101
	.byte	122
	.byte	-82
	.byte	8
	.byte	-70
	.byte	120
	.byte	37
	.byte	46
	.byte	28
	.byte	-90
	.byte	-76
	.byte	-58
	.byte	-24
	.byte	-35
	.byte	116
	.byte	31
	.byte	75
	.byte	-67
	.byte	-117
	.byte	-118
	.byte	112
	.byte	62
	.byte	-75
	.byte	102
	.byte	72
	.byte	3
	.byte	-10
	.byte	14
	.byte	97
	.byte	53
	.byte	87
	.byte	-71
	.byte	-122
	.byte	-63
	.byte	29
	.byte	-98
	.byte	-31
	.byte	-8
	.byte	-104
	.byte	17
	.byte	105
	.byte	-39
	.byte	-114
	.byte	-108
	.byte	-101
	.byte	30
	.byte	-121
	.byte	-23
	.byte	-50
	.byte	85
	.byte	40
	.byte	-33
	.byte	-116
	.byte	-95
	.byte	-119
	.byte	13
	.byte	-65
	.byte	-26
	.byte	66
	.byte	104
	.byte	65
	.byte	-103
	.byte	45
	.byte	15
	.byte	-80
	.byte	84
	.byte	-69
	.byte	22
	.type	Rcon, @object
	.size	Rcon, 11
Rcon:
	.byte	-115
	.byte	1
	.byte	2
	.byte	4
	.byte	8
	.byte	16
	.byte	32
	.byte	64
	.byte	-128
	.byte	27
	.byte	54
	.align 32
	.type	rsbox, @object
	.size	rsbox, 256
rsbox:
	.byte	82
	.byte	9
	.byte	106
	.byte	-43
	.byte	48
	.byte	54
	.byte	-91
	.byte	56
	.byte	-65
	.byte	64
	.byte	-93
	.byte	-98
	.byte	-127
	.byte	-13
	.byte	-41
	.byte	-5
	.byte	124
	.byte	-29
	.byte	57
	.byte	-126
	.byte	-101
	.byte	47
	.byte	-1
	.byte	-121
	.byte	52
	.byte	-114
	.byte	67
	.byte	68
	.byte	-60
	.byte	-34
	.byte	-23
	.byte	-53
	.byte	84
	.byte	123
	.byte	-108
	.byte	50
	.byte	-90
	.byte	-62
	.byte	35
	.byte	61
	.byte	-18
	.byte	76
	.byte	-107
	.byte	11
	.byte	66
	.byte	-6
	.byte	-61
	.byte	78
	.byte	8
	.byte	46
	.byte	-95
	.byte	102
	.byte	40
	.byte	-39
	.byte	36
	.byte	-78
	.byte	118
	.byte	91
	.byte	-94
	.byte	73
	.byte	109
	.byte	-117
	.byte	-47
	.byte	37
	.byte	114
	.byte	-8
	.byte	-10
	.byte	100
	.byte	-122
	.byte	104
	.byte	-104
	.byte	22
	.byte	-44
	.byte	-92
	.byte	92
	.byte	-52
	.byte	93
	.byte	101
	.byte	-74
	.byte	-110
	.byte	108
	.byte	112
	.byte	72
	.byte	80
	.byte	-3
	.byte	-19
	.byte	-71
	.byte	-38
	.byte	94
	.byte	21
	.byte	70
	.byte	87
	.byte	-89
	.byte	-115
	.byte	-99
	.byte	-124
	.byte	-112
	.byte	-40
	.byte	-85
	.byte	0
	.byte	-116
	.byte	-68
	.byte	-45
	.byte	10
	.byte	-9
	.byte	-28
	.byte	88
	.byte	5
	.byte	-72
	.byte	-77
	.byte	69
	.byte	6
	.byte	-48
	.byte	44
	.byte	30
	.byte	-113
	.byte	-54
	.byte	63
	.byte	15
	.byte	2
	.byte	-63
	.byte	-81
	.byte	-67
	.byte	3
	.byte	1
	.byte	19
	.byte	-118
	.byte	107
	.byte	58
	.byte	-111
	.byte	17
	.byte	65
	.byte	79
	.byte	103
	.byte	-36
	.byte	-22
	.byte	-105
	.byte	-14
	.byte	-49
	.byte	-50
	.byte	-16
	.byte	-76
	.byte	-26
	.byte	115
	.byte	-106
	.byte	-84
	.byte	116
	.byte	34
	.byte	-25
	.byte	-83
	.byte	53
	.byte	-123
	.byte	-30
	.byte	-7
	.byte	55
	.byte	-24
	.byte	28
	.byte	117
	.byte	-33
	.byte	110
	.byte	71
	.byte	-15
	.byte	26
	.byte	113
	.byte	29
	.byte	41
	.byte	-59
	.byte	-119
	.byte	111
	.byte	-73
	.byte	98
	.byte	14
	.byte	-86
	.byte	24
	.byte	-66
	.byte	27
	.byte	-4
	.byte	86
	.byte	62
	.byte	75
	.byte	-58
	.byte	-46
	.byte	121
	.byte	32
	.byte	-102
	.byte	-37
	.byte	-64
	.byte	-2
	.byte	120
	.byte	-51
	.byte	90
	.byte	-12
	.byte	31
	.byte	-35
	.byte	-88
	.byte	51
	.byte	-120
	.byte	7
	.byte	-57
	.byte	49
	.byte	-79
	.byte	18
	.byte	16
	.byte	89
	.byte	39
	.byte	-128
	.byte	-20
	.byte	95
	.byte	96
	.byte	81
	.byte	127
	.byte	-87
	.byte	25
	.byte	-75
	.byte	74
	.byte	13
	.byte	45
	.byte	-27
	.byte	122
	.byte	-97
	.byte	-109
	.byte	-55
	.byte	-100
	.byte	-17
	.byte	-96
	.byte	-32
	.byte	59
	.byte	77
	.byte	-82
	.byte	42
	.byte	-11
	.byte	-80
	.byte	-56
	.byte	-21
	.byte	-69
	.byte	60
	.byte	-125
	.byte	83
	.byte	-103
	.byte	97
	.byte	23
	.byte	43
	.byte	4
	.byte	126
	.byte	-70
	.byte	119
	.byte	-42
	.byte	38
	.byte	-31
	.byte	105
	.byte	20
	.byte	99
	.byte	85
	.byte	33
	.byte	12
	.byte	125
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
t6_LC0:
	.long	1593835520
	.align 4
t6_LC1:
	.long	1065353216
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
t6_LC3:
	.long	-1683627180
	.long	1074340036
	.section	.rodata.cst4
	.align 4
t6_LC4:
	.long	1078527525
	.align 4
t6_LC5:
	.long	1602224128
	.ident	"GCC: (GNU) 4.7.4 20140401 for GNAT GPL gpl-2014 (20140405)"
	.text
.global all_tests
.global _all_tests
