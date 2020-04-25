
.text
.align 4096



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


	.file	"t1.c"
	.text
	.p2align 4,,15


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

	.p2align 4,,15


quirky:
t1_LFB0:
	.cfi_startproc
	rep ret
	.cfi_endproc
t1_LFE0:

	.p2align 4,,15


zero_ticks:
t1_LFB1:
	.cfi_startproc
	movl	$0, -4(%rsp)
	movl	-4(%rsp), %eax
	cmpl	$9, %eax
	jg	t1_L3
	.p2align 4,,10
	.p2align 3
t1_L6:
	movl	-4(%rsp), %eax
	addl	$1, %eax
	movl	%eax, -4(%rsp)
	movl	-4(%rsp), %eax
	cmpl	$9, %eax
	jle	t1_L6
t1_L3:
	rep ret
	.cfi_endproc
t1_LFE1:

	.p2align 4,,15


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

	.p2align 4,,15


four_ticks:
t1_LFB4:
	.cfi_startproc
	movl	$0, -4(%rsp)
	movl	-4(%rsp), %edx
	movl	$99, %eax
	cmpl	$2, %edx
	jg	t1_L12
	.p2align 4,,10
	.p2align 3
t1_L13:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	-4(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -4(%rsp)
	movl	-4(%rsp), %edx
	cmpl	$2, %edx
	jle	t1_L13
t1_L12:
	movl	$99, %eax
#APP
# 32 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
t1_LFE4:

	.p2align 4,,15


five_ticks:
t1_LFB5:
	.cfi_startproc
	movl	$0, -4(%rsp)
	movl	-4(%rsp), %edx
	movl	$99, %eax
	cmpl	$2, %edx
	jg	t1_L18
	.p2align 4,,10
	.p2align 3
t1_L19:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	-4(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -4(%rsp)
	movl	-4(%rsp), %edx
	cmpl	$2, %edx
	jle	t1_L19
t1_L18:
	movl	$99, %eax
#APP
# 32 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
# 17 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
t1_LFE5:

	.p2align 4,,15


x_ticks:
t1_LFB6:
	.cfi_startproc
	movl	$0, -4(%rsp)
	movl	-4(%rsp), %eax
	cmpl	$2, %eax
	jg	t1_L32
	testl	%edi, %edi
	movl	$99, %eax
	je	t1_L28
	.p2align 4,,10
	.p2align 3
t1_L29:
#APP
# 46 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	subl	$1, %edi
	jne	t1_L29
t1_L28:
	movl	-4(%rsp), %eax
	addl	$1, %eax
	movl	%eax, -4(%rsp)
	movl	-4(%rsp), %eax
	cmpl	$2, %eax
	jle	t1_L28
	rep ret
t1_L32:
	rep ret
	.cfi_endproc
t1_LFE6:

	.p2align 4,,15


nineteen_ticks:
t1_LFB7:
	.cfi_startproc
	movl	$0, -24(%rsp)
	movl	-24(%rsp), %edx
	movl	$99, %eax
	cmpl	$5, %edx
	jg	t1_L33
	.p2align 4,,10
	.p2align 3
t1_L55:
	movl	$0, -20(%rsp)
	movl	-20(%rsp), %edx
	cmpl	$5, %edx
	jle	t1_L56
	jmp	t1_L54
	.p2align 4,,10
	.p2align 3
t1_L38:
	movl	-20(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -20(%rsp)
	movl	-20(%rsp), %edx
	cmpl	$5, %edx
	jg	t1_L54
t1_L56:
	movl	-24(%rsp), %ecx
	movl	-20(%rsp), %edx
	cmpl	%edx, %ecx
	jne	t1_L38
	movl	-20(%rsp), %edx
	cmpl	$5, %edx
	ja	t1_L38
	jmp	*t1_L40(,%rdx,8)
	.section	.rodata
	.align 8
	.align 4
t1_L40:
	.quad	t1_L39
	.quad	t1_L66
	.quad	t1_L53
	.quad	t1_L43
	.quad	t1_L44
	.quad	t1_L45
	.text
t1_L45:
	movl	$0, -4(%rsp)
	movl	-4(%rsp), %edx
	cmpl	$2, %edx
	jg	t1_L53
	.p2align 4,,10
	.p2align 3
t1_L57:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	-4(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -4(%rsp)
	movl	-4(%rsp), %edx
	cmpl	$2, %edx
	jle	t1_L57
t1_L53:
#APP
# 32 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
t1_L66:
#APP
# 17 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	jmp	t1_L38
t1_L44:
	movl	$0, -8(%rsp)
	movl	-8(%rsp), %edx
	cmpl	$2, %edx
	jg	t1_L66
	.p2align 4,,10
	.p2align 3
t1_L59:
#APP
# 30 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	-8(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -8(%rsp)
	movl	-8(%rsp), %edx
	cmpl	$2, %edx
	jle	t1_L59
	jmp	t1_L66
t1_L39:
	movl	$0, -16(%rsp)
	movl	-16(%rsp), %edx
	cmpl	$9, %edx
	jg	t1_L38
	.p2align 4,,10
	.p2align 3
t1_L58:
	movl	-16(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -16(%rsp)
	movl	-16(%rsp), %edx
	cmpl	$9, %edx
	jle	t1_L58
	jmp	t1_L38
t1_L43:
	movl	$0, -12(%rsp)
	movl	-12(%rsp), %edx
	cmpl	$2, %edx
	jg	t1_L38
#APP
# 46 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
t1_L49:
	movl	-12(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -12(%rsp)
	movl	-12(%rsp), %edx
	cmpl	$2, %edx
	jle	t1_L49
	jmp	t1_L38
	.p2align 4,,10
	.p2align 3
t1_L54:
#APP
# 80 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	movl	-24(%rsp), %edx
	addl	$1, %edx
	movl	%edx, -24(%rsp)
	movl	-24(%rsp), %edx
	cmpl	$5, %edx
	jle	t1_L55
	rep ret
t1_L33:
	rep ret
	.cfi_endproc
t1_LFE7:

	.p2align 4,,15


runme_root1:
t1_LFB8:
	.cfi_startproc
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	testl	%edi, %edi
	je	t1_L68
	xorl	%eax, %eax
	call	nineteen_ticks
	movq	$one_tick, fptr(%rip)
t1_L69:
	movl	$0, 8(%rsp)
	movl	8(%rsp), %edx
	movl	$99, %eax
	cmpl	$3, %edx
	jg	t1_L67
t1_L77:
	movl	8(%rsp), %edx
	movl	$0, 12(%rsp)
	movl	12(%rsp), %ecx
	cmpl	$2, %ecx
	jg	t1_L72
	testl	%edx, %edx
	je	t1_L78
	.p2align 4,,10
	.p2align 3
t1_L79:
#APP
# 46 "t1.c" 1
	outl %eax, $0x30
# 0 "" 2
#NO_APP
	subl	$1, %edx
	jne	t1_L79
t1_L78:
	movl	12(%rsp), %edx
	addl	$1, %edx
	movl	%edx, 12(%rsp)
	movl	12(%rsp), %edx
	cmpl	$2, %edx
	jle	t1_L78
t1_L72:
	movl	8(%rsp), %edx
	addl	$1, %edx
	movl	%edx, 8(%rsp)
	movl	8(%rsp), %edx
	cmpl	$3, %edx
	jle	t1_L77
t1_L67:
	addq	$24, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
t1_L68:
	.cfi_restore_state
	movl	$3, %edi
	call	direct_recurse
	movl	$3, %edi
	call	transitive_recurse
	movl	$3, %edi
	call	tail_recurse_1
	xorl	%eax, %eax
	call	*fptr(%rip)
	movl	$1, %edi
	call	tail_recurse_2
	jmp	t1_L69
	.cfi_endproc
t1_LFE8:

	.p2align 4,,15


call_site_2:
t1_LFB9:
	.cfi_startproc
	xorl	%edi, %edi
	jmp	runme_root1
	.cfi_endproc
t1_LFE9:

	.p2align 4,,15


test_1:
t1_LFB10:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$1, %edi
	call	runme_root1
	xorl	%edi, %edi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	runme_root1
	.cfi_endproc
t1_LFE10:

	.comm	fptr,8,8
	.ident	"GCC: (GNU) 6.3.1 20170510 (for GNAT GPL 2017 20170515)"
	.text
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

.text
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

.text



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
	.p2align 4,,15


target1:
t6_LFB0:
	.cfi_startproc
	fldt	t1(%rip)
	flds	t6_LC0(%rip)
	fxch	%st(1)
	fucomi	%st(1), %st
	jnb	t6_L2
	fstp	%st(1)
	fnstcw	-10(%rsp)
	movzwl	-10(%rsp), %eax
	orb	$12, %ah
	movw	%ax, -12(%rsp)
	fldcw	-12(%rsp)
	fistpq	-24(%rsp)
	fldcw	-10(%rsp)
	movq	-24(%rsp), %rax
t6_L3:
	movq	%rax, above(%rip)
	movl	x1(%rip), %eax
	addl	$1, %eax
	movl	%eax, x1(%rip)
	fldt	t1(%rip)
	fadds	t6_LC1(%rip)
	fstpt	t1(%rip)
	ret
	.p2align 4,,10
	.p2align 3
t6_L2:
	fnstcw	-10(%rsp)
	movzwl	-10(%rsp), %eax
	fsubp	%st, %st(1)
	movabsq	$-9223372036854775808, %rdx
	orb	$12, %ah
	movw	%ax, -12(%rsp)
	fldcw	-12(%rsp)
	fistpq	-24(%rsp)
	fldcw	-10(%rsp)
	movq	-24(%rsp), %rax
	xorq	%rdx, %rax
	jmp	t6_L3
	.cfi_endproc
t6_LFE0:

	.p2align 4,,15


target2:
t6_LFB1:
	.cfi_startproc
	movq	above(%rip), %rax
	addq	%rax, %rax
	movq	%rax, above(%rip)
	movl	x1(%rip), %eax
	addl	%eax, %eax
	movl	%eax, x1(%rip)
	fldt	t1(%rip)
	fmull	t6_LC3(%rip)
	fstpt	t1(%rip)
	ret
	.cfi_endproc
t6_LFE1:

	.p2align 4,,15


target3:
t6_LFB2:
	.cfi_startproc
	movsd	t2(%rip), %xmm0
	movsd	t6_LC4(%rip), %xmm1
	ucomisd	%xmm1, %xmm0
	jnb	t6_L7
	cvttsd2siq	%xmm0, %rax
t6_L8:
	movq	%rax, below(%rip)
	movl	x2(%rip), %eax
	addl	$1, %eax
	movl	%eax, x2(%rip)
	movsd	t2(%rip), %xmm0
	addsd	t6_LC5(%rip), %xmm0
	movsd	%xmm0, t2(%rip)
	ret
	.p2align 4,,10
	.p2align 3
t6_L7:
	subsd	%xmm1, %xmm0
	movabsq	$-9223372036854775808, %rdx
	cvttsd2siq	%xmm0, %rax
	xorq	%rdx, %rax
	jmp	t6_L8
	.cfi_endproc
t6_LFE2:

	.p2align 4,,15


target4:
t6_LFB3:
	.cfi_startproc
	movss	t3(%rip), %xmm0
	ucomiss	t6_LC0(%rip), %xmm0
	jnb	t6_L10
	cvttss2siq	%xmm0, %rax
t6_L11:
	movq	%rax, below(%rip)
	movl	x2(%rip), %eax
	addl	%eax, %eax
	movl	%eax, x2(%rip)
	movsd	t2(%rip), %xmm0
	mulsd	t6_LC6(%rip), %xmm0
	movsd	%xmm0, t2(%rip)
	ret
	.p2align 4,,10
	.p2align 3
t6_L10:
	subss	t6_LC0(%rip), %xmm0
	movabsq	$-9223372036854775808, %rdx
	cvttss2siq	%xmm0, %rax
	xorq	%rdx, %rax
	jmp	t6_L11
	.cfi_endproc
t6_LFE3:

	.p2align 4,,15


target5:
t6_LFB4:
	.cfi_startproc
	rep ret
	.cfi_endproc
t6_LFE4:

	.p2align 4,,15


target6:
t6_LFB5:
	.cfi_startproc
	fldt	t1(%rip)
	flds	t6_LC0(%rip)
	fxch	%st(1)
	fucomi	%st(1), %st
	jnb	t6_L14
	fstp	%st(1)
	fnstcw	-10(%rsp)
	movzwl	-10(%rsp), %eax
	orb	$12, %ah
	movw	%ax, -12(%rsp)
	fldcw	-12(%rsp)
	fistpq	-24(%rsp)
	fldcw	-10(%rsp)
	movq	-24(%rsp), %rax
t6_L15:
	movq	%rax, above(%rip)
	movq	below(%rip), %rax
	testq	%rax, %rax
	js	t6_L16
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
	movsd	%xmm0, t2(%rip)
	ret
	.p2align 4,,10
	.p2align 3
t6_L14:
	fnstcw	-10(%rsp)
	movzwl	-10(%rsp), %eax
	fsubp	%st, %st(1)
	movabsq	$-9223372036854775808, %rdx
	orb	$12, %ah
	movw	%ax, -12(%rsp)
	fldcw	-12(%rsp)
	fistpq	-24(%rsp)
	fldcw	-10(%rsp)
	movq	-24(%rsp), %rax
	xorq	%rdx, %rax
	jmp	t6_L15
	.p2align 4,,10
	.p2align 3
t6_L16:
	movq	%rax, %rdx
	pxor	%xmm0, %xmm0
	shrq	%rdx
	andl	$1, %eax
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	movsd	%xmm0, t2(%rip)
	ret
	.cfi_endproc
t6_LFE5:

	.p2align 4,,15


target7:
t6_LFB6:
	.cfi_startproc
	movl	x1(%rip), %eax
	movl	x2(%rip), %ecx
	xorl	%edx, %edx
	divl	%ecx
	movl	%edx, %edx
	movq	%rdx, -16(%rsp)
	fildq	-16(%rsp)
	fstpt	t1(%rip)
	ret
	.cfi_endproc
t6_LFE6:

	.p2align 4,,15


target8:
t6_LFB7:
	.cfi_startproc
	fldl	t2(%rip)
	fstpt	t1(%rip)
	ret
	.cfi_endproc
t6_LFE7:

	.p2align 4,,15


target9:
t6_LFB8:
	.cfi_startproc
	fldt	t1(%rip)
	fstpl	-16(%rsp)
	movsd	-16(%rsp), %xmm0
	movsd	%xmm0, t2(%rip)
	ret
	.cfi_endproc
t6_LFE8:

	.p2align 4,,15


targeta:
t6_LFB9:
	.cfi_startproc
	fldt	t1(%rip)
	fldl	t2(%rip)
	fdivrp	%st, %st(1)
	fstpt	t1(%rip)
	ret
	.cfi_endproc
t6_LFE9:

	.p2align 4,,15


targetb:
t6_LFB10:
	.cfi_startproc
	movq	above(%rip), %rax
	movq	below(%rip), %rdx
	imulq	%rdx, %rax
	movq	%rax, above(%rip)
	ret
	.cfi_endproc
t6_LFE10:

	.text

bits:
t6_LFB21:
	.cfi_startproc
	movslq	48(%rdi), %r8
t6_L24:
	movl	52(%rdi), %edx
	cmpl	%esi, %edx
	jge	t6_L29
	movq	40(%rdi), %rax
	cmpq	32(%rdi), %rax
	jne	t6_L25
t6_L28:
	jmp	t6_L28
t6_L25:
	movq	24(%rdi), %rcx
	leaq	1(%rax), %r9
	movq	%r9, 40(%rdi)
	movzbl	(%rcx,%rax), %eax
	movb	%dl, %cl
	addl	$8, %edx
	movl	%edx, 52(%rdi)
	salq	%cl, %rax
	orq	%rax, %r8
	jmp	t6_L24
t6_L29:
	subl	%esi, %edx
	movb	%sil, %cl
	movq	%r8, %rax
	movl	%edx, 52(%rdi)
	movl	$1, %edx
	sarq	%cl, %rax
	salq	%cl, %rdx
	movl	%eax, 48(%rdi)
	leal	-1(%rdx), %eax
	andl	%r8d, %eax
	ret
	.cfi_endproc
t6_LFE21:


decode:
t6_LFB23:
	.cfi_startproc
	movq	(%rsi), %rax
	movl	52(%rdi), %ecx
	xorl	%r11d, %r11d
	movl	48(%rdi), %r9d
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	movl	$1, %r8d
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	movl	$8, %r12d
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	leaq	2(%rax), %r10
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	movl	%ecx, %edx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	movl	$16, %ebp
t6_L31:
	testl	%edx, %edx
	je	t6_L41
	movl	%r9d, %r13d
	sarl	%r9d
	andl	$1, %r13d
	orl	%r13d, %eax
	leaq	2(%r10), %r13
	movswl	(%r10), %r10d
	movl	%eax, %r14d
	subl	%r10d, %r14d
	cmpl	%r14d, %r11d
	jle	t6_L32
	subl	%r11d, %eax
	movq	8(%rsi), %rdx
	subl	%r8d, %ecx
	addl	%ebx, %eax
	andl	$7, %ecx
	movl	%r9d, 48(%rdi)
	cltq
	movl	%ecx, 52(%rdi)
	movswl	(%rdx,%rax,2), %eax
	jmp	t6_L30
t6_L32:
	addl	%r10d, %ebx
	addl	%r11d, %r10d
	decl	%edx
	leal	(%r10,%r10), %r11d
	addl	%eax, %eax
	incl	%r8d
	movq	%r13, %r10
	jmp	t6_L31
t6_L41:
	movl	%ebp, %edx
	subl	%r8d, %edx
	je	t6_L38
	movq	40(%rdi), %r9
	cmpq	32(%rdi), %r9
	jne	t6_L35
t6_L39:
	jmp	t6_L39
t6_L35:
	movq	24(%rdi), %r13
	leaq	1(%r9), %r14
	cmpl	$8, %edx
	cmovg	%r12d, %edx
	movq	%r14, 40(%rdi)
	movzbl	0(%r13,%r9), %r9d
	jmp	t6_L31
t6_L38:
	movl	$-10, %eax
t6_L30:
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
t6_LFE23:


construct:
t6_LFB24:
	.cfi_startproc
	movq	(%rdi), %r9
	xorl	%eax, %eax
t6_L43:
	movw	$0, (%r9,%rax)
	addq	$2, %rax
	cmpq	$32, %rax
	jne	t6_L43
	xorl	%eax, %eax
t6_L44:
	cmpl	%eax, %edx
	jle	t6_L59
	movswq	(%rsi,%rax,2), %rcx
	incq	%rax
	incw	(%r9,%rcx,2)
	jmp	t6_L44
t6_L59:
	movswl	(%r9), %ecx
	xorl	%eax, %eax
	cmpl	%ecx, %edx
	je	t6_L42
	movl	$1, %ecx
	movl	$1, %eax
t6_L47:
	movswl	(%r9,%rcx,2), %r8d
	addl	%eax, %eax
	subl	%r8d, %eax
	js	t6_L42
	incq	%rcx
	cmpq	$16, %rcx
	jne	t6_L47
	leaq	-40(%rsp), %r11
	movw	$0, -38(%rsp)
	movl	$2, %ecx
	leaq	2(%r11), %r10
t6_L48:
	movw	(%r9,%rcx), %r8w
	addw	-40(%rsp,%rcx), %r8w
	movw	%r8w, (%r10,%rcx)
	addq	$2, %rcx
	cmpq	$30, %rcx
	jne	t6_L48
	xorl	%ecx, %ecx
t6_L49:
	cmpl	%ecx, %edx
	jle	t6_L42
	movswq	(%rsi,%rcx,2), %r8
	testw	%r8w, %r8w
	je	t6_L50
	movswq	-40(%rsp,%r8,2), %r9
	movq	8(%rdi), %r10
	leal	1(%r9), %r11d
	movw	%cx, (%r10,%r9,2)
	movw	%r11w, -40(%rsp,%r8,2)
t6_L50:
	incq	%rcx
	jmp	t6_L49
t6_L42:
	ret
	.cfi_endproc
t6_LFE24:


codes:
t6_LFB25:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	movq	%rdi, %r13
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	movq	%rsi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movq	%rdx, %rbx
t6_L61:
	movq	%rbp, %rsi
	movq	%r13, %rdi
	call	decode
	testl	%eax, %eax
	movl	%eax, %r10d
	js	t6_L60
	cmpl	$255, %eax
	jg	t6_L63
	movq	0(%r13), %rdx
	testq	%rdx, %rdx
	je	t6_L64
	movq	16(%r13), %rax
	cmpq	8(%r13), %rax
	jne	t6_L65
t6_L69:
	movl	$1, %r10d
	jmp	t6_L60
t6_L65:
	movb	%r10b, (%rdx,%rax)
t6_L64:
	incq	16(%r13)
	jmp	t6_L61
t6_L63:
	cmpl	$256, %eax
	jne	t6_L67
t6_L73:
	xorl	%r10d, %r10d
	jmp	t6_L60
t6_L67:
	subl	$257, %r10d
	cmpl	$28, %r10d
	jg	t6_L74
	movslq	%r10d, %r10
	movq	%r13, %rdi
	movswl	lext.1956(%r10,%r10), %esi
	movswl	lens.1955(%r10,%r10), %r12d
	call	bits
	movq	%rbx, %rsi
	movq	%r13, %rdi
	addl	%eax, %r12d
	call	decode
	testl	%eax, %eax
	movl	%eax, %r10d
	js	t6_L60
	cltq
	movq	%r13, %rdi
	movswl	dists.1957(%rax,%rax), %r11d
	movswl	dext.1958(%rax,%rax), %esi
	call	bits
	leal	(%r11,%rax), %ecx
	movq	16(%r13), %rax
	cmpq	%rax, %rcx
	ja	t6_L75
	cmpq	$0, 0(%r13)
	movslq	%r12d, %rdx
	je	t6_L68
	addq	%rax, %rdx
	cmpq	8(%r13), %rdx
	ja	t6_L69
t6_L70:
	decl	%r12d
	cmpl	$-1, %r12d
	je	t6_L72
	movq	16(%r13), %rax
	movq	0(%r13), %rdx
	movq	%rax, %rsi
	subq	%rcx, %rsi
	movb	(%rdx,%rsi), %sil
	movb	%sil, (%rdx,%rax)
	incq	16(%r13)
	jmp	t6_L70
t6_L68:
	addq	%rax, %rdx
	movq	%rdx, 16(%r13)
t6_L72:
	cmpl	$256, %r10d
	jne	t6_L61
	jmp	t6_L73
t6_L74:
	movl	$-10, %r10d
	jmp	t6_L60
t6_L75:
	movl	$-11, %r10d
t6_L60:
	popq	%rbx
	.cfi_def_cfa_offset 32
	movl	%r10d, %eax
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
t6_LFE25:


dynamic:
t6_LFB27:
	.cfi_startproc
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	subq	$1376, %rsp
	.cfi_def_cfa_offset 1424
	movslq	48(%rdi), %rbp
	leaq	32(%rsp), %rax
	movq	%rax, (%rsp)
	leaq	160(%rsp), %rax
	movq	%rax, 8(%rsp)
	leaq	64(%rsp), %rax
	movq	%rax, 16(%rsp)
	leaq	96(%rsp), %rax
	movq	%rax, 24(%rsp)
t6_L81:
	movl	52(%rdi), %ecx
	cmpl	$4, %ecx
	jg	t6_L147
	movq	40(%rdi), %rax
	cmpq	32(%rdi), %rax
	jne	t6_L82
t6_L139:
	jmp	t6_L139
t6_L82:
	movq	24(%rdi), %rdx
	leaq	1(%rax), %rsi
	movq	%rsi, 40(%rdi)
	movzbl	(%rdx,%rax), %eax
	salq	%cl, %rax
	addl	$8, %ecx
	orq	%rax, %rbp
	movl	%ecx, 52(%rdi)
	jmp	t6_L81
t6_L147:
	movq	%rbp, %r12
	subl	$5, %ecx
	sarq	$5, %r12
	movl	%ecx, 52(%rdi)
	movl	%r12d, 48(%rdi)
	movslq	%r12d, %r12
t6_L85:
	movl	52(%rdi), %ecx
	cmpl	$4, %ecx
	jg	t6_L148
	movq	40(%rdi), %rax
	cmpq	32(%rdi), %rax
	jne	t6_L86
t6_L140:
	jmp	t6_L140
t6_L86:
	movq	24(%rdi), %rdx
	leaq	1(%rax), %rsi
	movq	%rsi, 40(%rdi)
	movzbl	(%rdx,%rax), %eax
	salq	%cl, %rax
	addl	$8, %ecx
	orq	%rax, %r12
	movl	%ecx, 52(%rdi)
	jmp	t6_L85
t6_L148:
	movq	%r12, %rax
	subl	$5, %ecx
	sarq	$5, %rax
	movl	%ecx, 52(%rdi)
	movl	%eax, 48(%rdi)
	cltq
t6_L89:
	movl	52(%rdi), %ecx
	cmpl	$3, %ecx
	jg	t6_L149
	movq	40(%rdi), %rdx
	cmpq	32(%rdi), %rdx
	jne	t6_L90
t6_L141:
	jmp	t6_L141
t6_L90:
	movq	24(%rdi), %rsi
	leaq	1(%rdx), %r8
	movq	%r8, 40(%rdi)
	movzbl	(%rsi,%rdx), %edx
	salq	%cl, %rdx
	addl	$8, %ecx
	orq	%rdx, %rax
	movl	%ecx, 52(%rdi)
	jmp	t6_L89
t6_L149:
	andl	$31, %ebp
	andl	$31, %r12d
	movq	%rax, %rdx
	addl	$257, %ebp
	sarq	$4, %rdx
	subl	$4, %ecx
	andl	$15, %eax
	incl	%r12d
	cmpl	$286, %ebp
	movl	%edx, 48(%rdi)
	movl	%ecx, 52(%rdi)
	leal	4(%rax), %edx
	jg	t6_L126
	cmpl	$30, %r12d
	jg	t6_L126
	movl	$order.2006, %r8d
	xorl	%eax, %eax
t6_L98:
	movswq	(%r8), %r9
	movslq	48(%rdi), %rsi
t6_L94:
	movl	52(%rdi), %ecx
	cmpl	$2, %ecx
	jg	t6_L150
	movq	40(%rdi), %r10
	cmpq	32(%rdi), %r10
	jne	t6_L95
t6_L142:
	jmp	t6_L142
t6_L95:
	movq	24(%rdi), %r11
	leaq	1(%r10), %rbx
	movq	%rbx, 40(%rdi)
	movzbl	(%r11,%r10), %r10d
	salq	%cl, %r10
	addl	$8, %ecx
	orq	%r10, %rsi
	movl	%ecx, 52(%rdi)
	jmp	t6_L94
t6_L150:
	movq	%rsi, %r10
	incl	%eax
	subl	$3, %ecx
	sarq	$3, %r10
	andl	$7, %esi
	addq	$2, %r8
	cmpl	%eax, %edx
	movl	%r10d, 48(%rdi)
	movl	%ecx, 52(%rdi)
	movw	%si, 736(%rsp,%r9,2)
	jg	t6_L98
	cltq
t6_L99:
	cmpl	$19, %eax
	je	t6_L151
	movswq	order.2006(%rax,%rax), %rdx
	incq	%rax
	movw	$0, 736(%rsp,%rdx,2)
	jmp	t6_L99
t6_L151:
	leaq	736(%rsp), %rsi
	movq	%rdi, %rbx
	movl	$19, %edx
	movq	%rsp, %rdi
	call	construct
	testl	%eax, %eax
	jne	t6_L127
	leal	0(%rbp,%r12), %r14d
	xorl	%r13d, %r13d
t6_L101:
	cmpl	%r14d, %r13d
	jge	t6_L152
	movq	%rsp, %rsi
	movq	%rbx, %rdi
	call	decode
	cmpl	$15, %eax
	jg	t6_L102
	leal	1(%r13), %esi
	movw	%ax, 736(%rsp,%r13,2)
	jmp	t6_L103
t6_L102:
	cmpl	$16, %eax
	jne	t6_L104
	testl	%r13d, %r13d
	je	t6_L128
	leal	-1(%r13), %eax
	cltq
	movswl	736(%rsp,%rax,2), %edx
	movslq	48(%rbx), %rax
t6_L105:
	movl	52(%rbx), %ecx
	cmpl	$1, %ecx
	jg	t6_L153
	movq	40(%rbx), %rsi
	cmpq	32(%rbx), %rsi
	jne	t6_L106
t6_L143:
	jmp	t6_L143
t6_L106:
	movq	24(%rbx), %rdi
	leaq	1(%rsi), %r8
	movq	%r8, 40(%rbx)
	movzbl	(%rdi,%rsi), %esi
	salq	%cl, %rsi
	addl	$8, %ecx
	orq	%rsi, %rax
	movl	%ecx, 52(%rbx)
	jmp	t6_L105
t6_L153:
	movq	%rax, %rsi
	subl	$2, %ecx
	andl	$3, %eax
	sarq	$2, %rsi
	movl	%ecx, 52(%rbx)
	addl	$3, %eax
	movl	%esi, 48(%rbx)
	jmp	t6_L109
t6_L104:
	cmpl	$17, %eax
	movslq	48(%rbx), %rax
	jne	t6_L115
t6_L111:
	movl	52(%rbx), %ecx
	cmpl	$2, %ecx
	jg	t6_L154
	movq	40(%rbx), %rdx
	cmpq	32(%rbx), %rdx
	jne	t6_L112
t6_L144:
	jmp	t6_L144
t6_L112:
	movq	24(%rbx), %rsi
	leaq	1(%rdx), %rdi
	movq	%rdi, 40(%rbx)
	movzbl	(%rsi,%rdx), %edx
	salq	%cl, %rdx
	addl	$8, %ecx
	orq	%rdx, %rax
	movl	%ecx, 52(%rbx)
	jmp	t6_L111
t6_L154:
	movq	%rax, %rdx
	subl	$3, %ecx
	andl	$7, %eax
	sarq	$3, %rdx
	movl	%ecx, 52(%rbx)
	addl	$3, %eax
	movl	%edx, 48(%rbx)
	jmp	t6_L123
t6_L118:
	movq	40(%rbx), %rdx
	cmpq	32(%rbx), %rdx
	je	t6_L145
	movq	24(%rbx), %rsi
	leaq	1(%rdx), %rdi
	movq	%rdi, 40(%rbx)
	movzbl	(%rsi,%rdx), %edx
	salq	%cl, %rdx
	addl	$8, %ecx
	orq	%rdx, %rax
	movl	%ecx, 52(%rbx)
t6_L115:
	movl	52(%rbx), %ecx
	cmpl	$6, %ecx
	jle	t6_L118
	jmp	t6_L155
t6_L145:
	jmp	t6_L145
t6_L155:
	movq	%rax, %rdx
	subl	$7, %ecx
	andl	$127, %eax
	sarq	$7, %rdx
	movl	%ecx, 52(%rbx)
	addl	$11, %eax
	movl	%edx, 48(%rbx)
t6_L123:
	xorl	%edx, %edx
t6_L109:
	leal	0(%r13,%rax), %esi
	cmpl	%esi, %r14d
	jl	t6_L129
	leaq	736(%rsp), %rdi
	addq	%r13, %r13
	cltq
	xorl	%ecx, %ecx
	addq	%rdi, %r13
t6_L119:
	movw	%dx, 0(%r13,%rcx,2)
	incq	%rcx
	cmpq	%rcx, %rax
	jne	t6_L119
t6_L103:
	movslq	%esi, %r13
	jmp	t6_L101
t6_L152:
	cmpw	$0, 1248(%rsp)
	movl	$-9, %eax
	je	t6_L80
	leaq	736(%rsp), %rsi
	movl	%ebp, %edx
	movq	%rsp, %rdi
	call	construct
	testl	%eax, %eax
	je	t6_L121
	js	t6_L124
	movq	(%rsp), %rdx
	movswl	(%rdx), %eax
	movswl	2(%rdx), %edx
	addl	%edx, %eax
	cmpl	%eax, %ebp
	jne	t6_L124
t6_L121:
	leaq	736(%rsp), %rax
	movslq	%ebp, %rbp
	leaq	16(%rsp), %rdi
	addq	%rbp, %rbp
	movl	%r12d, %edx
	leaq	(%rax,%rbp), %rsi
	call	construct
	testl	%eax, %eax
	je	t6_L122
	js	t6_L125
	movq	16(%rsp), %rdx
	movswl	(%rdx), %eax
	movswl	2(%rdx), %edx
	addl	%edx, %eax
	cmpl	%eax, %r12d
	jne	t6_L125
t6_L122:
	leaq	16(%rsp), %rdx
	movq	%rsp, %rsi
	movq	%rbx, %rdi
	call	codes
	jmp	t6_L80
t6_L124:
	movl	$-7, %eax
	jmp	t6_L80
t6_L125:
	movl	$-8, %eax
	jmp	t6_L80
t6_L126:
	movl	$-3, %eax
	jmp	t6_L80
t6_L127:
	movl	$-4, %eax
	jmp	t6_L80
t6_L128:
	movl	$-5, %eax
	jmp	t6_L80
t6_L129:
	movl	$-6, %eax
t6_L80:
	addq	$1376, %rsp
	.cfi_def_cfa_offset 48
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
t6_LFE27:

	.text
	.p2align 4,,15

KeyExpansion:
t6_LFB30:
	.cfi_startproc
	leaq	16(%rsi), %rcx
	movq	%rdi, %rax
t6_L157:
	movzbl	(%rsi), %edx
	addq	$4, %rsi
	addq	$4, %rax
	movb	%dl, -4(%rax)
	movzbl	-3(%rsi), %edx
	movb	%dl, -3(%rax)
	movzbl	-2(%rsi), %edx
	movb	%dl, -2(%rax)
	movzbl	-1(%rsi), %edx
	movb	%dl, -1(%rax)
	cmpq	%rsi, %rcx
	jne	t6_L157
	movl	$4, %r8d
	.p2align 4,,10
	.p2align 3
t6_L159:
	testb	$3, %r8b
	movzbl	12(%rdi), %esi
	movzbl	13(%rdi), %ecx
	movzbl	14(%rdi), %edx
	movzbl	15(%rdi), %eax
	jne	t6_L158
	movzbl	sbox(%rcx), %r9d
	movzbl	sbox(%rdx), %ecx
	movzbl	sbox(%rax), %edx
	movzbl	sbox(%rsi), %eax
	movl	%r8d, %esi
	shrl	$2, %esi
	xorb	Rcon(%rsi), %r9b
	movl	%r9d, %esi
t6_L158:
	xorb	(%rdi), %sil
	xorb	1(%rdi), %cl
	addl	$1, %r8d
	xorb	2(%rdi), %dl
	xorb	3(%rdi), %al
	addq	$4, %rdi
	movb	%sil, 12(%rdi)
	movb	%cl, 13(%rdi)
	movb	%dl, 14(%rdi)
	movb	%al, 15(%rdi)
	cmpl	$44, %r8d
	jne	t6_L159
	rep ret
	.cfi_endproc
t6_LFE30:

	.p2align 4,,15

Cipher:
t6_LFB42:
	.cfi_startproc
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	leaq	16(%rdi), %r9
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	movq	%rdi, %r10
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	movq	%rsi, %rcx
	movq	%rdi, %r8
t6_L163:
	xorl	%eax, %eax
t6_L164:
	movzbl	(%rcx,%rax), %edx
	xorb	%dl, (%r8,%rax)
	addq	$1, %rax
	cmpq	$4, %rax
	jne	t6_L164
	addq	$4, %r8
	addq	$4, %rcx
	cmpq	%r8, %r9
	jne	t6_L163
	leaq	16(%rsi), %r9
	leaq	160(%rsi), %rbx
	leaq	4(%rdi), %r11
	.p2align 4,,10
	.p2align 3
t6_L172:
	movq	%rdi, %rsi
t6_L174:
	leaq	16(%rsi), %rcx
	movq	%rsi, %rax
t6_L166:
	movzbl	(%rax), %edx
	addq	$4, %rax
	movzbl	sbox(%rdx), %edx
	movb	%dl, -4(%rax)
	cmpq	%rax, %rcx
	jne	t6_L166
	addq	$1, %rsi
	cmpq	%rsi, %r11
	jne	t6_L174
	movzbl	9(%rdi), %edx
	movzbl	1(%rdi), %eax
	movzbl	5(%rdi), %ecx
	movzbl	10(%rdi), %r12d
	movzbl	15(%rdi), %ebp
	movb	%dl, 5(%rdi)
	movzbl	13(%rdi), %edx
	movb	%al, 13(%rdi)
	movzbl	2(%rdi), %eax
	movb	%cl, 1(%rdi)
	movb	%r12b, 2(%rdi)
	movb	%dl, 9(%rdi)
	movzbl	14(%rdi), %edx
	movb	%al, 10(%rdi)
	movzbl	6(%rdi), %eax
	movb	%dl, 6(%rdi)
	movzbl	11(%rdi), %edx
	movb	%al, 14(%rdi)
	movzbl	3(%rdi), %eax
	movb	%bpl, 3(%rdi)
	movb	%dl, 15(%rdi)
	movzbl	7(%rdi), %edx
	movb	%al, 7(%rdi)
	movb	%dl, 11(%rdi)
	movq	%rdi, %rdx
t6_L169:
	movzbl	(%rdx), %eax
	xorl	%r12d, %ebp
	xorl	%eax, %ecx
	movl	%ecx, %r13d
	xorl	%ecx, %ebp
	addl	%ecx, %ecx
	shrb	$7, %r13b
	leal	(%r13,%r13), %r12d
	addl	%r13d, %r12d
	leal	0(,%r12,8), %r13d
	addl	%r13d, %r12d
	xorl	%r12d, %ecx
	movl	%eax, %r12d
	xorl	%ebp, %r12d
	xorl	%r12d, %ecx
	movb	%cl, (%rdx)
	movzbl	1(%rdx), %ecx
	xorb	2(%rdx), %cl
	movl	%ecx, %r13d
	addl	%ecx, %ecx
	shrb	$7, %r13b
	leal	(%r13,%r13), %r12d
	addl	%r13d, %r12d
	leal	0(,%r12,8), %r13d
	addl	%r13d, %r12d
	xorl	%r12d, %ecx
	movl	%ebp, %r12d
	xorb	1(%rdx), %r12b
	xorl	%r12d, %ecx
	movzbl	3(%rdx), %r12d
	movb	%cl, 1(%rdx)
	movl	%r12d, %ecx
	xorb	2(%rdx), %cl
	xorl	%r12d, %eax
	movl	%ecx, %r14d
	addl	%ecx, %ecx
	shrb	$7, %r14b
	leal	(%r14,%r14), %r13d
	addl	%r14d, %r13d
	leal	0(,%r13,8), %r14d
	addl	%r14d, %r13d
	xorl	%r13d, %ecx
	movl	%ebp, %r13d
	xorb	2(%rdx), %r13b
	xorl	%r13d, %ecx
	movl	%eax, %r13d
	addl	%eax, %eax
	shrb	$7, %r13b
	movb	%cl, 2(%rdx)
	leal	(%r13,%r13), %ecx
	addl	%r13d, %ecx
	leal	0(,%rcx,8), %r13d
	addl	%r13d, %ecx
	xorl	%ecx, %eax
	xorl	%r12d, %ebp
	addq	$4, %rdx
	xorl	%ebp, %eax
	movb	%al, -1(%rdx)
	cmpq	%r8, %rdx
	je	t6_L180
	movzbl	1(%rdx), %ecx
	movzbl	2(%rdx), %r12d
	movzbl	3(%rdx), %ebp
	jmp	t6_L169
	.p2align 4,,10
	.p2align 3
t6_L180:
	movq	%r9, %r12
	movq	%rdi, %rcx
t6_L168:
	xorl	%eax, %eax
t6_L170:
	movzbl	(%r12,%rax), %ebp
	xorb	%bpl, (%rcx,%rax)
	addq	$1, %rax
	cmpq	$4, %rax
	jne	t6_L170
	addq	$4, %rcx
	addq	$4, %r12
	cmpq	%rcx, %rdx
	jne	t6_L168
	addq	$16, %r9
	cmpq	%r9, %rbx
	jne	t6_L172
	movq	%rdi, %r8
t6_L173:
	leaq	16(%r8), %r11
	movq	%r8, %rax
t6_L175:
	movzbl	(%rax), %ecx
	addq	$4, %rax
	movzbl	sbox(%rcx), %ecx
	movb	%cl, -4(%rax)
	cmpq	%rax, %r11
	jne	t6_L175
	addq	$1, %r8
	cmpq	%r8, %rsi
	jne	t6_L173
	movzbl	5(%rdi), %ecx
	movzbl	1(%rdi), %eax
	movb	%cl, 1(%rdi)
	movzbl	9(%rdi), %ecx
	movb	%cl, 5(%rdi)
	movzbl	13(%rdi), %ecx
	movb	%al, 13(%rdi)
	movzbl	2(%rdi), %eax
	movb	%cl, 9(%rdi)
	movzbl	10(%rdi), %ecx
	movb	%al, 10(%rdi)
	movzbl	6(%rdi), %eax
	movb	%cl, 2(%rdi)
	movzbl	14(%rdi), %ecx
	movb	%al, 14(%rdi)
	movzbl	3(%rdi), %eax
	movb	%cl, 6(%rdi)
	movzbl	15(%rdi), %ecx
	movb	%cl, 3(%rdi)
	movzbl	11(%rdi), %ecx
	movb	%cl, 15(%rdi)
	movzbl	7(%rdi), %ecx
	movb	%al, 7(%rdi)
	movb	%cl, 11(%rdi)
t6_L177:
	xorl	%eax, %eax
t6_L178:
	movzbl	(%r9,%rax), %ecx
	xorb	%cl, (%r10,%rax)
	addq	$1, %rax
	cmpq	$4, %rax
	jne	t6_L178
	addq	$4, %r10
	addq	$4, %r9
	cmpq	%r10, %rdx
	jne	t6_L177
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
t6_LFE42:

	.p2align 4,,15

InvCipher:
t6_LFB43:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	leaq	160(%rsi), %rcx
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rdi, %rdx
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rsi, -40(%rsp)
	leaq	16(%rdi), %rsi
	movq	%rdi, -48(%rsp)
	movq	%rdi, -8(%rsp)
t6_L190:
	xorl	%eax, %eax
t6_L191:
	movzbl	(%rcx,%rax), %edi
	xorb	%dil, (%rdx,%rax)
	addq	$1, %rax
	cmpq	$4, %rax
	jne	t6_L191
	addq	$4, %rdx
	addq	$4, %rcx
	cmpq	%rdx, %rsi
	jne	t6_L190
	movq	-40(%rsp), %rax
	movq	%rdx, -32(%rsp)
	addq	$144, %rax
	movq	%rax, -56(%rsp)
	movq	-48(%rsp), %rax
	addq	$4, %rax
	movq	%rax, -16(%rsp)
	.p2align 4,,10
	.p2align 3
t6_L198:
	movq	-48(%rsp), %rdi
	movzbl	9(%rdi), %edx
	movzbl	13(%rdi), %eax
	movq	%rdi, %rcx
	movb	%dl, 13(%rdi)
	movzbl	5(%rdi), %edx
	movb	%dl, 9(%rdi)
	movzbl	1(%rdi), %edx
	movb	%al, 1(%rdi)
	movzbl	2(%rdi), %eax
	movb	%dl, 5(%rdi)
	movzbl	10(%rdi), %edx
	movb	%al, 10(%rdi)
	movzbl	6(%rdi), %eax
	movb	%dl, 2(%rdi)
	movzbl	14(%rdi), %edx
	movb	%al, 14(%rdi)
	movzbl	3(%rdi), %eax
	movb	%dl, 6(%rdi)
	movzbl	7(%rdi), %edx
	movb	%dl, 3(%rdi)
	movzbl	11(%rdi), %edx
	movb	%dl, 7(%rdi)
	movzbl	15(%rdi), %edx
	movb	%al, 15(%rdi)
	movb	%dl, 11(%rdi)
	movq	-16(%rsp), %rdi
t6_L193:
	leaq	16(%rcx), %rsi
	movq	%rcx, %rax
t6_L194:
	movzbl	(%rax), %edx
	addq	$4, %rax
	movzbl	rsbox(%rdx), %edx
	movb	%dl, -4(%rax)
	cmpq	%rax, %rsi
	jne	t6_L194
	addq	$1, %rcx
	cmpq	%rcx, %rdi
	jne	t6_L193
	movq	%rcx, -24(%rsp)
	movq	-48(%rsp), %rbp
	movq	-56(%rsp), %rcx
	movq	-32(%rsp), %rsi
t6_L195:
	xorl	%eax, %eax
t6_L196:
	movzbl	(%rcx,%rax), %edx
	xorb	%dl, 0(%rbp,%rax)
	addq	$1, %rax
	cmpq	$4, %rax
	jne	t6_L196
	addq	$4, %rbp
	addq	$4, %rcx
	cmpq	%rbp, %rsi
	jne	t6_L195
	movq	-48(%rsp), %rax
	movq	%rbp, -64(%rsp)
t6_L197:
	movzbl	(%rax), %r15d
	movzbl	1(%rax), %edx
	movzbl	2(%rax), %r14d
	movl	%r15d, %esi
	leal	(%r15,%r15), %r11d
	shrb	$7, %sil
	leal	(%rsi,%rsi), %ecx
	addl	%esi, %ecx
	leal	0(,%rcx,8), %esi
	addl	%esi, %ecx
	xorl	%ecx, %r11d
	movl	%r11d, %ecx
	movb	%r11b, -66(%rsp)
	shrb	$7, %cl
	leal	(%rcx,%rcx), %ebp
	addl	%ecx, %ebp
	leal	0(,%rbp,8), %ecx
	addl	%ecx, %ebp
	movl	%r11d, %ecx
	leal	(%rcx,%rcx), %ebx
	xorl	%ebx, %ebp
	movl	%edx, %ebx
	movl	%ebp, %esi
	leal	(%rbp,%rbp), %r8d
	shrb	$7, %bl
	shrb	$7, %sil
	leal	(%rsi,%rsi), %ecx
	addl	%esi, %ecx
	leal	0(,%rcx,8), %esi
	addl	%esi, %ecx
	xorl	%ecx, %r8d
	movl	%ebx, %ecx
	leal	(%rcx,%rcx), %r13d
	movb	%r8b, -65(%rsp)
	addl	%ecx, %r13d
	leal	0(,%r13,8), %ecx
	addl	%ecx, %r13d
	leal	(%rdx,%rdx), %ecx
	xorl	%ecx, %r13d
	movl	%r13d, %ebx
	leal	(%r13,%r13), %r9d
	shrb	$7, %bl
	movl	%ebx, %ecx
	leal	(%rcx,%rcx), %r10d
	addl	%ecx, %r10d
	leal	0(,%r10,8), %ecx
	addl	%ecx, %r10d
	xorl	%r9d, %r10d
	movl	%r10d, %ebx
	leal	(%r10,%r10), %edi
	shrb	$7, %bl
	movl	%ebx, %ecx
	movl	%r14d, %ebx
	leal	(%rcx,%rcx), %r8d
	shrb	$7, %bl
	addl	%ecx, %r8d
	leal	0(,%r8,8), %ecx
	addl	%ecx, %r8d
	movl	%ebx, %ecx
	leal	(%r14,%r14), %ebx
	leal	(%rcx,%rcx), %r12d
	xorl	%edi, %r8d
	addl	%ecx, %r12d
	leal	0(,%r12,8), %ecx
	addl	%ecx, %r12d
	xorl	%ebx, %r12d
	movl	%r12d, %esi
	shrb	$7, %sil
	movl	%esi, %ecx
	leal	(%rcx,%rcx), %ebx
	addl	%ecx, %ebx
	leal	0(,%rbx,8), %ecx
	addl	%ecx, %ebx
	leal	(%r12,%r12), %ecx
	xorl	%ecx, %ebx
	movl	%ebx, %esi
	shrb	$7, %sil
	movl	%esi, %ecx
	movzbl	3(%rax), %esi
	leal	(%rcx,%rcx), %edi
	addl	%edi, %ecx
	leal	0(,%rcx,8), %edi
	shrb	$7, %sil
	addl	%edi, %ecx
	leal	(%rbx,%rbx), %edi
	xorl	%ecx, %edi
	movl	%esi, %ecx
	movzbl	3(%rax), %esi
	leal	(%rcx,%rcx), %r11d
	addl	%r11d, %ecx
	leal	0(,%rcx,8), %r11d
	addl	%r11d, %ecx
	leal	(%rsi,%rsi), %r11d
	xorl	%ecx, %r11d
	movl	%r11d, %esi
	shrb	$7, %sil
	movl	%esi, %ecx
	leal	(%rcx,%rcx), %r9d
	addl	%r9d, %ecx
	leal	0(,%rcx,8), %r9d
	addl	%r9d, %ecx
	leal	(%r11,%r11), %r9d
	xorl	%ecx, %r9d
	movl	%r9d, %esi
	shrb	$7, %sil
	movl	%esi, %ecx
	leal	(%rcx,%rcx), %esi
	addl	%esi, %ecx
	leal	0(,%rcx,8), %esi
	addl	%esi, %ecx
	leal	(%r9,%r9), %esi
	xorl	%ecx, %esi
	movl	%edx, %ecx
	xorl	%r14d, %ecx
	xorb	3(%rax), %cl
	xorb	-66(%rsp), %cl
	xorl	%r13d, %ecx
	xorl	%ebp, %ecx
	xorl	%ebx, %ecx
	xorb	-65(%rsp), %cl
	xorl	%r8d, %ecx
	xorl	%edi, %ecx
	xorl	%esi, %ecx
	movb	%cl, (%rax)
	movl	%r15d, %ecx
	xorl	%r14d, %ecx
	xorb	3(%rax), %cl
	xorl	%r13d, %ecx
	movzbl	-65(%rsp), %r13d
	xorl	%r12d, %ecx
	xorl	%r10d, %ecx
	xorl	%r9d, %ecx
	xorl	%r13d, %ecx
	xorl	%r15d, %edx
	addq	$4, %rax
	movl	%edx, %r15d
	xorb	-1(%rax), %r15b
	xorl	%r8d, %ecx
	xorl	%edi, %ecx
	xorl	%r14d, %edx
	xorb	-66(%rsp), %dl
	xorl	%esi, %ecx
	movb	%cl, -3(%rax)
	movl	%r15d, %ecx
	xorl	%ecx, %r12d
	xorl	%r11d, %r12d
	xorl	%edx, %r11d
	xorl	%r12d, %ebp
	xorl	%r11d, %r10d
	xorl	%ebp, %ebx
	xorl	%r10d, %r9d
	xorl	%r13d, %ebx
	xorl	%r13d, %r9d
	xorl	%r8d, %ebx
	xorl	%r9d, %r8d
	xorl	%edi, %ebx
	xorl	%r8d, %edi
	xorl	%esi, %ebx
	xorl	%edi, %esi
	movb	%bl, -2(%rax)
	movb	%sil, -1(%rax)
	cmpq	%rax, -64(%rsp)
	jne	t6_L197
	subq	$16, -56(%rsp)
	movq	-64(%rsp), %rbp
	movq	-56(%rsp), %rax
	cmpq	%rax, -40(%rsp)
	jne	t6_L198
	movq	-48(%rsp), %rdi
	movq	-24(%rsp), %rsi
	movq	%rbp, %rbx
	movzbl	9(%rdi), %edx
	movzbl	13(%rdi), %eax
	movb	%dl, 13(%rdi)
	movzbl	5(%rdi), %edx
	movb	%dl, 9(%rdi)
	movzbl	1(%rdi), %edx
	movb	%al, 1(%rdi)
	movzbl	2(%rdi), %eax
	movb	%dl, 5(%rdi)
	movzbl	10(%rdi), %edx
	movb	%al, 10(%rdi)
	movzbl	6(%rdi), %eax
	movb	%dl, 2(%rdi)
	movzbl	14(%rdi), %edx
	movb	%al, 14(%rdi)
	movzbl	3(%rdi), %eax
	movb	%dl, 6(%rdi)
	movzbl	7(%rdi), %edx
	movb	%dl, 3(%rdi)
	movzbl	11(%rdi), %edx
	movb	%dl, 7(%rdi)
	movzbl	15(%rdi), %edx
	movb	%al, 15(%rdi)
	movb	%dl, 11(%rdi)
t6_L199:
	leaq	16(%rdi), %rcx
	movq	%rdi, %rax
t6_L200:
	movzbl	(%rax), %edx
	addq	$4, %rax
	movzbl	rsbox(%rdx), %edx
	movb	%dl, -4(%rax)
	cmpq	%rax, %rcx
	jne	t6_L200
	addq	$1, %rdi
	cmpq	%rdi, %rsi
	jne	t6_L199
	movq	-56(%rsp), %rsi
	movq	-8(%rsp), %rdx
t6_L213:
	xorl	%eax, %eax
t6_L202:
	movzbl	(%rsi,%rax), %ecx
	xorb	%cl, (%rdx,%rax)
	addq	$1, %rax
	cmpq	$4, %rax
	jne	t6_L202
	addq	$4, %rdx
	addq	$4, %rsi
	cmpq	%rdx, %rbx
	jne	t6_L213
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
t6_LFE43:

	.p2align 4,,15


f3:
t6_LFB58:
	.cfi_startproc
	rep ret
	.cfi_endproc
t6_LFE58:

	.p2align 4,,15


f5:
t6_LFB15:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	cmpl	$9, %edi
	jle	t6_L228
t6_L222:
	movl	$6, %ebx
t6_L226:
	leal	1(%rbx), %ebp
t6_L220:
t6_L219:
	cmpl	$9, %ebp
	movl	%ebp, %ebx
	jle	t6_L226
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
t6_L228:
	.cfi_restore_state
	leal	1(%rdi), %ebp
	movl	%edi, %ebx
	movl	%ebp, %edi
	call	f5
	testl	%ebx, %ebx
	je	t6_L220
	jmp	t6_L222
	.cfi_endproc
t6_LFE15:

	.p2align 4,,15


f4:
t6_LFB14:
	.cfi_startproc
	jmp	f5
	.cfi_endproc
t6_LFE14:

	.p2align 4,,15


f2:
t6_LFB52:
	.cfi_startproc
	jmp	f5
	.cfi_endproc
t6_LFE52:

	.p2align 4,,15


f1:
t6_LFB54:
	.cfi_startproc
	jmp	f5
	.cfi_endproc
t6_LFE54:

	.p2align 4,,15


f6:
t6_LFB16:
	.cfi_startproc
	testl	%edi, %edi
	movl	$6, %eax
	cmovne	%eax, %edi
t6_L233:
t6_L234:
	addl	$1, %edi
	cmpl	$9, %edi
	jle	t6_L233
	rep ret
	.cfi_endproc
t6_LFE16:

	.p2align 4,,15


f7:
t6_LFB56:
	.cfi_startproc
	rep ret
	.cfi_endproc
t6_LFE56:

	.p2align 4,,15


f8:
t6_LFB18:
	.cfi_startproc
	rep ret
	.cfi_endproc
t6_LFE18:

	.p2align 4,,15


f9:
t6_LFB19:
	.cfi_startproc
	xorl	%edx, %edx
	testl	%edi, %edi
	je	t6_L243
	addl	$1, %edi
	movl	$6, %edx
t6_L243:
	addl	$1, %edx
	leal	2(%rdi), %eax
	cmpl	$9, %edx
	jle	t6_L247
	rep ret
t6_L247:
	movl	%eax, %edi
	jmp	t6_L243
	.cfi_endproc
t6_LFE19:

	.p2align 4,,15


crash_longjmp:
t6_LFB20:
	.cfi_startproc
	.p2align 4,,10
	.p2align 3
t6_L249:
	jmp	t6_L249
	.cfi_endproc
t6_LFE20:

	.p2align 4,,15


puff:
t6_LFB28:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rcx, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rsi, %rbp
	xorl	%ebx, %ebx
	subq	$648, %rsp
	.cfi_def_cfa_offset 704
	movq	(%rsi), %rax
	leaq	352(%rsp), %r14
	movq	%rdi, (%rsp)
	movq	$0, 16(%rsp)
	movq	%rdx, 24(%rsp)
	movq	$0, 40(%rsp)
	movq	%rax, 8(%rsp)
	movq	(%rcx), %rax
	leaq	64(%rsp), %rcx
	movl	$0, 48(%rsp)
	movl	$0, 52(%rsp)
	leaq	512(%rcx), %r13
	movq	%rax, 32(%rsp)
	xorl	%eax, %eax
t6_L279:
	testl	%eax, %eax
	jg	t6_L251
	movq	40(%rsp), %rdx
	movq	32(%rsp), %rdi
	cmpq	%rdx, %rdi
	je	t6_L300
	movq	24(%rsp), %r8
	jmp	t6_L254
t6_L255:
	cmpq	%rdi, %rdx
	je	t6_L252
t6_L254:
	addq	$1, %rdx
	movl	%eax, %ecx
	addl	$8, %eax
	movq	%rdx, 40(%rsp)
	movzbl	-1(%r8,%rdx), %esi
	movl	%eax, 52(%rsp)
	salq	%cl, %rsi
	orq	%rsi, %rbx
	testl	%eax, %eax
	jle	t6_L255
t6_L251:
	leal	-1(%rax), %ecx
	movq	%rbx, %rdx
	andl	$1, %ebx
	sarq	%rdx
	cmpl	$1, %ecx
	movl	%edx, 48(%rsp)
	movl	%ecx, 52(%rsp)
	movslq	%edx, %rdx
	jg	t6_L256
	movq	40(%rsp), %rsi
	cmpq	32(%rsp), %rsi
	je	t6_L298
	movq	24(%rsp), %rdi
	addq	$1, %rsi
	movq	%rsi, 40(%rsp)
	movzbl	-1(%rdi,%rsi), %esi
	salq	%cl, %rsi
	leal	7(%rax), %ecx
	orq	%rsi, %rdx
t6_L256:
	movq	%rdx, %rax
	subl	$2, %ecx
	sarq	$2, %rax
	andl	$3, %edx
	movl	%ecx, 52(%rsp)
	movl	%eax, 48(%rsp)
	je	t6_L301
	cmpl	$1, %edx
	je	t6_L302
	cmpl	$2, %edx
	jne	t6_L303
	movq	%rsp, %rdi
	call	dynamic
	cmpl	$0, %eax
	jne	t6_L276
t6_L266:
	testl	%ebx, %ebx
	jne	t6_L304
	movslq	48(%rsp), %rbx
	movl	52(%rsp), %eax
	jmp	t6_L279
	.p2align 4,,10
	.p2align 3
t6_L298:
	jmp	t6_L298
t6_L300:
t6_L252:
	.p2align 4,,10
	.p2align 3
t6_L297:
	jmp	t6_L297
t6_L302:
	movl	virgin.1967(%rip), %r9d
	testl	%r9d, %r9d
	je	t6_L268
	leaq	64(%rsp), %r15
	movq	$lencnt.1968, lencode.1972(%rip)
	movq	$lensym.1969, lencode.1972+8(%rip)
	movq	$distcnt.1970, distcode.1973(%rip)
	movq	$distsym.1971, distcode.1973+8(%rip)
	movq	%r15, %rax
t6_L269:
	movl	$8, %r8d
	addq	$2, %rax
	movw	%r8w, -2(%rax)
	cmpq	%r14, %rax
	jne	t6_L269
t6_L270:
	movl	$9, %edi
	addq	$2, %rax
	movw	%di, -2(%rax)
	cmpq	%r13, %rax
	jne	t6_L270
	leaq	624(%rsp), %rdx
t6_L271:
	movl	$7, %esi
	addq	$2, %rax
	movw	%si, -2(%rax)
	cmpq	%rdx, %rax
	jne	t6_L271
	leaq	640(%rsp), %rdx
t6_L272:
	movl	$8, %ecx
	addq	$2, %rax
	movw	%cx, -2(%rax)
	cmpq	%rax, %rdx
	jne	t6_L272
	leaq	64(%rsp), %rsi
	movl	$288, %edx
	movl	$lencode.1972, %edi
	call	construct
	leaq	124(%rsp), %rax
t6_L273:
	movl	$5, %edx
	addq	$2, %r15
	movw	%dx, -2(%r15)
	cmpq	%rax, %r15
	jne	t6_L273
	leaq	64(%rsp), %rsi
	movl	$30, %edx
	movl	$distcode.1973, %edi
	call	construct
	movl	$0, virgin.1967(%rip)
t6_L268:
	movl	$distcode.1973, %edx
	movl	$lencode.1972, %esi
	movq	%rsp, %rdi
	call	codes
	cmpl	$0, %eax
	je	t6_L266
t6_L276:
	jg	t6_L250
	movq	40(%rsp), %rdx
t6_L278:
	movq	16(%rsp), %rcx
	movq	%rcx, 0(%rbp)
	movq	%rdx, (%r12)
	jmp	t6_L250
t6_L301:
	movq	40(%rsp), %r9
	movq	32(%rsp), %r8
	movl	$0, 48(%rsp)
	movl	$0, 52(%rsp)
	leaq	4(%r9), %rdx
	cmpq	%r8, %rdx
	jbe	t6_L305
t6_L260:
	movl	$2, %eax
t6_L250:
	addq	$648, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
t6_L304:
	.cfi_restore_state
	movq	40(%rsp), %rdx
	xorl	%eax, %eax
	jmp	t6_L278
t6_L303:
	movq	40(%rsp), %rdx
	movl	$-1, %eax
	jmp	t6_L278
t6_L305:
	leaq	1(%r9), %rax
	movq	24(%rsp), %rcx
	leaq	3(%r9), %rdi
	movq	%rax, 40(%rsp)
	leaq	2(%r9), %rax
	movzbl	(%rcx,%r9), %esi
	movq	%rax, 40(%rsp)
	movzbl	1(%rcx,%r9), %eax
	movq	%rdi, 40(%rsp)
	sall	$8, %eax
	orl	%esi, %eax
	movl	%eax, %esi
	notl	%esi
	cmpb	%sil, 2(%rcx,%r9)
	jne	t6_L281
	movq	%rdx, 40(%rsp)
	shrl	$8, %esi
	cmpb	3(%rcx,%r9), %sil
	jne	t6_L282
	movl	%eax, %esi
	leaq	(%rdx,%rsi), %rdi
	cmpq	%rdi, %r8
	jb	t6_L260
	movq	(%rsp), %r8
	testq	%r8, %r8
	je	t6_L262
	movq	16(%rsp), %rdi
	addq	%rdi, %rsi
	cmpq	8(%rsp), %rsi
	ja	t6_L283
	testl	%eax, %eax
	leal	-1(%rax), %esi
	je	t6_L266
t6_L265:
	leaq	1(%rdi), %rax
	subl	$1, %esi
	cmpl	$-1, %esi
	movq	%rax, 16(%rsp)
	leaq	1(%rdx), %rax
	movq	%rax, 40(%rsp)
	movzbl	(%rcx,%rdx), %eax
	movb	%al, (%r8,%rdi)
	je	t6_L266
	movq	(%rsp), %r8
	movq	16(%rsp), %rdi
	movq	24(%rsp), %rcx
	movq	40(%rsp), %rdx
	jmp	t6_L265
t6_L283:
	movl	$1, %eax
	jmp	t6_L250
t6_L282:
	movl	$-2, %eax
	jmp	t6_L278
t6_L281:
	movq	%rdi, %rdx
	movl	$-2, %eax
	jmp	t6_L278
t6_L262:
	addq	%rsi, 16(%rsp)
	movq	%rdi, 40(%rsp)
	jmp	t6_L266
	.cfi_endproc
t6_LFE28:

	.p2align 4,,15


my_memcpy:
t6_LFB29:
	.cfi_startproc
	xorl	%eax, %eax
	testl	%edx, %edx
	je	t6_L306
	.p2align 4,,10
	.p2align 3
t6_L310:
	movzbl	(%rsi,%rax), %ecx
	movb	%cl, (%rdi,%rax)
	addq	$1, %rax
	cmpl	%eax, %edx
	ja	t6_L310
t6_L306:
	rep ret
	.cfi_endproc
t6_LFE29:

	.p2align 4,,15


AES_init_ctx:
t6_LFB31:
	.cfi_startproc
	jmp	KeyExpansion
	.cfi_endproc
t6_LFE31:

	.p2align 4,,15


AES_init_ctx_iv:
t6_LFB32:
	.cfi_startproc
	movq	%rdi, %r10
	movq	%rdx, %r11
	call	KeyExpansion
	leaq	176(%r10), %rdi
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L314:
	movzbl	(%r11,%rax), %ecx
	movb	%cl, (%rdi,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L314
	rep ret
	.cfi_endproc
t6_LFE32:

	.p2align 4,,15


AES_ctx_set_iv:
t6_LFB33:
	.cfi_startproc
	addq	$176, %rdi
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L317:
	movzbl	(%rsi,%rax), %edx
	movb	%dl, (%rdi,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L317
	rep ret
	.cfi_endproc
t6_LFE33:

	.p2align 4,,15


AES_ECB_encrypt:
t6_LFB44:
	.cfi_startproc
	movq	%rdi, %rax
	movq	%rsi, %rdi
	movq	%rax, %rsi
	jmp	Cipher
	.cfi_endproc
t6_LFE44:

	.p2align 4,,15


AES_ECB_decrypt:
t6_LFB45:
	.cfi_startproc
	movq	%rdi, %rax
	movq	%rsi, %rdi
	movq	%rax, %rsi
	jmp	InvCipher
	.cfi_endproc
t6_LFE45:

	.p2align 4,,15


AES_CBC_encrypt_buffer:
t6_LFB47:
	.cfi_startproc
	pushq	%r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	pushq	%r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	movl	%edx, %r13d
	testq	%r13, %r13
	pushq	%r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	leaq	176(%rdi), %r12
	pushq	%rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	je	t6_L326
	movq	%rsi, %rbp
	movq	%rdi, %r14
	movq	%r12, %rcx
	movq	%rsi, %rbx
	.p2align 4,,10
	.p2align 3
t6_L324:
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L323:
	movzbl	(%rcx,%rax), %edx
	xorb	%dl, (%rbx,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L323
	movq	%r14, %rsi
	movq	%rbx, %rdi
	call	Cipher
	leaq	16(%rbx), %rax
	movq	%rbx, %rcx
	movq	%rax, %rdx
	subq	%rbp, %rdx
	cmpq	%r13, %rdx
	jnb	t6_L331
	movq	%rax, %rbx
	jmp	t6_L324
t6_L331:
	subq	$1, %r13
	andq	$-16, %r13
	addq	%r13, %rbp
t6_L322:
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L325:
	movzbl	0(%rbp,%rax), %edx
	movb	%dl, (%r12,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L325
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbp
	.cfi_def_cfa_offset 32
	popq	%r12
	.cfi_def_cfa_offset 24
	popq	%r13
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	ret
t6_L326:
	.cfi_restore_state
	movq	%r12, %rbp
	jmp	t6_L322
	.cfi_endproc
t6_LFE47:

	.p2align 4,,15


AES_CBC_decrypt_buffer:
t6_LFB48:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movl	%edx, %ebp
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
	testq	%rbp, %rbp
	je	t6_L332
	leaq	176(%rdi), %r14
	movq	%rsi, %r12
	movq	%rdi, %rbx
	xorl	%r15d, %r15d
	.p2align 4,,10
	.p2align 3
t6_L337:
	leaq	(%r12,%r15), %r13
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L334:
	movzbl	0(%r13,%rax), %edx
	movb	%dl, (%rsp,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L334
	movq	%rbx, %rsi
	movq	%r13, %rdi
	call	InvCipher
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L335:
	movzbl	(%r14,%rax), %edx
	xorb	%dl, 0(%r13,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L335
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L336:
	movzbl	(%rsp,%rax), %edx
	movb	%dl, (%r14,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L336
	addq	$16, %r15
	cmpq	%rbp, %r15
	jb	t6_L337
t6_L332:
	addq	$24, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
t6_LFE48:

	.p2align 4,,15


AES_CTR_xcrypt_buffer:
t6_LFB49:
	.cfi_startproc
	testl	%edx, %edx
	je	t6_L360
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	leal	-1(%rdx), %eax
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	%rdi, %rbp
	movq	%rsi, %rbx
	leaq	1(%rsi,%rax), %r12
	movl	$16, %edx
	subq	$16, %rsp
	.cfi_def_cfa_offset 48
	jmp	t6_L351
	.p2align 4,,10
	.p2align 3
t6_L363:
	movslq	%edx, %rax
	addl	$1, %edx
	movzbl	(%rsp,%rax), %eax
	xorb	%al, (%rbx)
	addq	$1, %rbx
	cmpq	%r12, %rbx
	je	t6_L362
t6_L351:
	cmpl	$16, %edx
	jne	t6_L363
	leaq	176(%rbp), %rcx
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
t6_L348:
	movzbl	(%rcx,%rax), %edx
	movb	%dl, (%rsp,%rax)
	addq	$1, %rax
	cmpq	$16, %rax
	jne	t6_L348
	movq	%rbp, %rsi
	movq	%rsp, %rdi
	call	Cipher
	movl	$15, %edx
t6_L350:
	movzbl	176(%rbp,%rdx), %eax
	movslq	%edx, %rcx
	cmpb	$-1, %al
	je	t6_L364
	addl	$1, %eax
	movb	%al, 176(%rbp,%rcx)
t6_L361:
	xorl	%eax, %eax
	movl	$1, %edx
	movzbl	(%rsp,%rax), %eax
	xorb	%al, (%rbx)
	addq	$1, %rbx
	cmpq	%r12, %rbx
	jne	t6_L351
t6_L362:
	addq	$16, %rsp
	.cfi_def_cfa_offset 32
	popq	%rbx
	.cfi_restore 3
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_restore 12
	.cfi_def_cfa_offset 8
t6_L360:
	rep ret
	.p2align 4,,10
	.p2align 3
t6_L364:
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -32
	.cfi_offset 6, -24
	.cfi_offset 12, -16
	movb	$0, 176(%rbp,%rdx)
	subq	$1, %rdx
	cmpq	$-1, %rdx
	jne	t6_L350
	jmp	t6_L361
	.cfi_endproc
t6_LFE49:

	.p2align 4,,15


test_6:
t6_LFB50:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movl	$target_list+8, %ebx
	subq	$224, %rsp
	.cfi_def_cfa_offset 240
	movq	target_list(%rip), %rdx
	testq	%rdx, %rdx
	je	t6_L370
t6_L378:
	addq	$8, %rbx
	xorl	%eax, %eax
	call	*%rdx
	movq	-8(%rbx), %rdx
	testq	%rdx, %rdx
	jne	t6_L378
t6_L370:
	xorl	%edi, %edi
	call	f5
	movl	$input_len, %ecx
	movl	$input_data, %edx
	movl	$output_len, %esi
	movl	$output_data, %edi
	call	puff
	testl	%eax, %eax
	jne	t6_L367
	cmpq	$630, output_len(%rip)
	je	t6_L389
t6_L367:
	.p2align 4,,10
	.p2align 3
t6_L383:
	jmp	t6_L383
t6_L389:
	cmpb	$83, output_data+90(%rip)
	jne	t6_L367
	cmpb	$114, output_data+626(%rip)
	jne	t6_L367
	xorl	%eax, %eax
t6_L372:
	movzbl	output_data(%rax), %edx
	movb	%dl, (%rsp,%rax)
	addq	$1, %rax
	cmpq	$32, %rax
	jne	t6_L372
	leaq	32(%rsp), %rdi
	movl	$input_data, %esi
	call	KeyExpansion
	leaq	32(%rsp), %rsi
	movq	%rsp, %rdi
	call	Cipher
	cmpb	$-108, 1(%rsp)
	je	t6_L390
t6_L373:
	.p2align 4,,10
	.p2align 3
t6_L384:
	jmp	t6_L384
t6_L390:
	cmpb	$58, 7(%rsp)
	jne	t6_L373
	leaq	32(%rsp), %rsi
	movq	%rsp, %rdi
	call	InvCipher
	xorl	%eax, %eax
t6_L377:
	movzbl	(%rsp,%rax), %ecx
	movsbl	output_data(%rax), %edx
	cmpl	%edx, %ecx
	jne	t6_L385
	addq	$1, %rax
	cmpq	$32, %rax
	jne	t6_L377
	addq	$224, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
t6_L385:
	.cfi_restore_state
	jmp	t6_L385
	.cfi_endproc
t6_LFE50:

	.section	.rodata
	.align 32


order.2006:
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


dext.1958:
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


dists.1957:
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


lext.1956:
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


lens.1955:
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

	.comm	distsym.1971,60,32

	.comm	distcnt.1970,32,32

	.comm	distcode.1973,16,16

	.comm	lensym.1969,576,32

	.comm	lencnt.1968,32,32

	.comm	lencode.1972,16,16
	.data
	.align 4


virgin.1967:
	.long	1
	.section	.rodata
	.align 8


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
	.align 32


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

	.data
	.align 8


output_len:
	.quad	700

	.align 8


input_len:
	.quad	376

	.section	.rodata
	.align 32


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

	.data
	.align 32


target_list:
	.quad	target1
	.quad	target2
	.quad	target3
	.quad	target4
	.quad	target2
	.quad	target2
	.quad	target2
	.quad	target1
	.quad	target2
	.quad	target4
	.quad	target3
	.quad	target4
	.quad	target5
	.quad	target3
	.quad	target1
	.quad	target2
	.quad	target6
	.quad	target3
	.quad	target7
	.quad	target1
	.quad	target3
	.quad	target8
	.quad	target9
	.quad	targeta
	.quad	targetb
	.quad	0

	.align 4


t3:
	.long	1088421888

	.align 8


t2:
	.long	0
	.long	1075314688

	.align 16


t1:
	.long	0
	.long	2684354560
	.long	16385
	.long	0

	.align 4


x2:
	.long	4

	.align 4


x1:
	.long	3

	.align 8


below:
	.quad	2

	.align 8


above:
	.quad	1
	.section	.rodata.cst4
	.align 4
t6_LC0:
	.long	1593835520
	.align 4
t6_LC1:
	.long	1065353216
	.section	.rodata.cst8
	.align 8
t6_LC3:
	.long	2611340116
	.long	1074340036
	.align 8
t6_LC4:
	.long	0
	.long	1138753536
	.align 8
t6_LC5:
	.long	0
	.long	1072693248
	.align 8
t6_LC6:
	.long	2684354560
	.long	1074340036
	.ident	"GCC: (GNU) 6.3.1 20170510 (for GNAT GPL 2017 20170515)"
	.text
.global all_tests
.global _all_tests
