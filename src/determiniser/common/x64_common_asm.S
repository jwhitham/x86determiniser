
/* x64 calling convention (Microsoft)
 * uses registers RCX, RDX, R8, R9 for the first four integer arguments.
 *
 * It is the caller's responsibility to allocate 32 bytes of "shadow
 * space" on the stack right before calling the function (regardless
 * of the actual number of parameters used), and to pop the stack after
 * the call. 
 *
 * The registers RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile
 * (caller-saved).
 * The registers RBX, RBP, RDI, RSI, RSP, R12, R13, R14, and R15 are
 * considered non-volatile (callee-saved).
 *
 * x64 calling convention (System V)
 * The first six integer or pointer arguments are passed in registers
 * RDI, RSI, RDX, RCX, R8, R9.
 *
 * The registers RBX, RBP and R12 .. R15 are considered non-volatile (callee-saved).
 */

#include "offsets.h"

#define CALL_SIZE 6
#define SUPER_STACK_SIZE 0x10000
#define SHADOW_SIZE 32

.global x86_switch_to_user
.global x86_switch_from_user
.global x86_begin_single_step
.global x86_other_context
.global x86_size_of_call_instruction
.global x86_bp_trap
.global x86_is_branch_taken 
.global _x86_switch_to_user
.global _x86_switch_from_user
.global _x86_begin_single_step
.global _x86_other_context
.global _x86_size_of_call_instruction
.global _x86_bp_trap
.global _x86_is_branch_taken 
.text

#ifdef WIN64
#define PARAM_1      %rcx
#define PARAM_2      %rdx
#else
#ifdef LINUX64
#define PARAM_1      %rdi
#define PARAM_2      %rsi
#else
#error "WIN64 or LINUX64 must be defined"
#endif
#endif

.bss
.align 8
save_space:
    .space  16
switch_from_user_ptr:
    .space  8
_x86_size_of_call_instruction:
x86_size_of_call_instruction:
    .space  4
.align 8
_x86_other_context:
x86_other_context:
    .space OFF_LIMIT
super_stack_bottom:
    .space SUPER_STACK_SIZE
super_stack_top:

.text
_x86_begin_single_step:
x86_begin_single_step:
    /* save the user context */
    mov     %rax, OFF_XAX + x86_other_context    /* GPRs */
    mov     %rbx, OFF_XBX + x86_other_context
    mov     %rcx, OFF_XCX + x86_other_context
    mov     %rdx, OFF_XDX + x86_other_context
    mov     %rsp, OFF_XSP + x86_other_context
    mov     %rbp, OFF_XBP + x86_other_context
    mov     %rsi, OFF_XSI + x86_other_context
    mov     %rdi, OFF_XDI + x86_other_context
    mov     %r8, OFF_R8 + x86_other_context
    mov     %r9, OFF_R9 + x86_other_context
    mov     %r10, OFF_R10 + x86_other_context
    mov     %r11, OFF_R11 + x86_other_context
    mov     %r12, OFF_R12 + x86_other_context
    mov     %r13, OFF_R13 + x86_other_context
    mov     %r14, OFF_R14 + x86_other_context
    mov     %r15, OFF_R15 + x86_other_context
    movsd   %xmm0, OFF_Xmm0 + x86_other_context
    movsd   %xmm1, OFF_Xmm1 + x86_other_context
    movsd   %xmm2, OFF_Xmm2 + x86_other_context
    movsd   %xmm3, OFF_Xmm3 + x86_other_context
    movsd   %xmm4, OFF_Xmm4 + x86_other_context
    movsd   %xmm5, OFF_Xmm5 + x86_other_context
    movsd   %xmm6, OFF_Xmm6 + x86_other_context
    movsd   %xmm7, OFF_Xmm7 + x86_other_context
    movsd   %xmm8, OFF_Xmm8 + x86_other_context
    movsd   %xmm9, OFF_Xmm9 + x86_other_context
    movsd   %xmm10, OFF_Xmm10 + x86_other_context
    movsd   %xmm11, OFF_Xmm11 + x86_other_context
    movsd   %xmm12, OFF_Xmm12 + x86_other_context
    movsd   %xmm13, OFF_Xmm13 + x86_other_context
    movsd   %xmm14, OFF_Xmm14 + x86_other_context
    movsd   %xmm15, OFF_Xmm15 + x86_other_context
    pushfq
    popq    %rax
    mov     %eax, OFF_EFL + x86_other_context          /* FLAGS */
    mov     $user_start_point, %rax
    mov     %rax, OFF_XIP + x86_other_context    /* program counter (return point) */

    /* various setup */
    mov     $super_stack_top, %rsp

    mov     $x86_switch_from_user, %rax
    mov     %rax, switch_from_user_ptr

    mov     $CALL_SIZE, %eax
    mov     %eax, x86_size_of_call_instruction

    /* enter interpreter loop */
    sub     $SHADOW_SIZE, %rsp
    call    x86_interpreter
    add     $SHADOW_SIZE, %rsp

    /* supervisor should not have returned */
    mov     $1, PARAM_1
    jmp     _exit

user_start_point:
    /* user begins executing here */
    retq

.align 4

/* void x86_switch_to_user (uintptr_t endpoint); */
x86_switch_to_user:
_x86_switch_to_user:
    pushq   %rbp
    movq    %rsp, %rbp

   /* The registers RBX, RBP, RDI, RSI, RSP, R12, R13, R14, and R15 are
    * considered non-volatile (callee-saved) on Windows.
    * On other platforms, RDI and RSI are volatile.
    */
    pushq   %rbx
#ifdef WIN64
    pushq   %rdi
    pushq   %rsi
#endif
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    /* Save whatever is already at the endpoint - need to save 14 bytes */
    mov     0(PARAM_1), %rax
    mov     8(PARAM_1), %rdx
    mov     %rax, save_space
    mov     %rdx, save_space + 8

    /* encode PC-relative indirect call followed by absolute address */

    mov     $0x15ff, %rax        
    mov     %rax, 0(PARAM_1)     /* call with PC-relative address: ff 15 00 00 00 00 XX XX */
    mov     $x86_switch_from_user, %rdx
    mov     %rdx, 6(PARAM_1)     /* absolute 64-bit target address */

    /* restore the user context */
    mov     OFF_EFL + x86_other_context, %eax /* user FLAGS */
    pushq   %rax
    mov     OFF_XAX + x86_other_context, %rax /* GPRs */
    mov     OFF_XBX + x86_other_context, %rbx
    mov     OFF_XDX + x86_other_context, %rdx
    mov     OFF_XBP + x86_other_context, %rbp
    mov     OFF_XSI + x86_other_context, %rsi
#ifdef WIN64
    xchg    OFF_XCX + x86_other_context, %rcx /* contains PARAM_1 on Windows */
    mov     OFF_XDI + x86_other_context, %rdi
#else
    mov     OFF_XCX + x86_other_context, %rcx
    xchg    OFF_XDI + x86_other_context, %rdi /* contains PARAM_1 on Linux */
#endif
    mov     OFF_R8 + x86_other_context, %r8
    mov     OFF_R9 + x86_other_context, %r9
    mov     OFF_R10 + x86_other_context, %r10
    mov     OFF_R11 + x86_other_context, %r11
    mov     OFF_R12 + x86_other_context, %r12
    mov     OFF_R13 + x86_other_context, %r13
    mov     OFF_R14 + x86_other_context, %r14
    mov     OFF_R15 + x86_other_context, %r15
    movsd   OFF_Xmm0 + x86_other_context, %xmm0
    movsd   OFF_Xmm1 + x86_other_context, %xmm1
    movsd   OFF_Xmm2 + x86_other_context, %xmm2
    movsd   OFF_Xmm3 + x86_other_context, %xmm3
    movsd   OFF_Xmm4 + x86_other_context, %xmm4
    movsd   OFF_Xmm5 + x86_other_context, %xmm5
    movsd   OFF_Xmm6 + x86_other_context, %xmm6
    movsd   OFF_Xmm7 + x86_other_context, %xmm7
    movsd   OFF_Xmm8 + x86_other_context, %xmm8
    movsd   OFF_Xmm9 + x86_other_context, %xmm9
    movsd   OFF_Xmm10 + x86_other_context, %xmm10
    movsd   OFF_Xmm11 + x86_other_context, %xmm11
    movsd   OFF_Xmm12 + x86_other_context, %xmm12
    movsd   OFF_Xmm13 + x86_other_context, %xmm13
    movsd   OFF_Xmm14 + x86_other_context, %xmm14
    movsd   OFF_Xmm15 + x86_other_context, %xmm15
    popfq                                        /* restoring FLAGS may cause single-stepping */
    xchg    %rsp, OFF_XSP + x86_other_context
    jmp     *(OFF_XIP + x86_other_context)       /* user PC */

.align 4
_x86_switch_from_user:
x86_switch_from_user:
    /* save the user context */
    mov     %rax, OFF_XAX + x86_other_context    /* GPRs */
    pop     %rax                                 /* PC + call instruction size (5 bytes) */
    xchg    %rsp, OFF_XSP + x86_other_context
    mov     %rbx, OFF_XBX + x86_other_context
    mov     %rdx, OFF_XDX + x86_other_context
    mov     %rbp, OFF_XBP + x86_other_context
    mov     %rsi, OFF_XSI + x86_other_context
#ifdef WIN64
    xchg    %rcx, OFF_XCX + x86_other_context    /* contains x86_switch_to_user PARAM_1 on Windows */
    mov     %rdi, OFF_XDI + x86_other_context
#else
    mov     %rcx, OFF_XCX + x86_other_context
    xchg    %rdi, OFF_XDI + x86_other_context    /* contains x86_switch_to_user PARAM_1 on Linux */
#endif
    mov     %r8, OFF_R8 + x86_other_context
    mov     %r9, OFF_R9 + x86_other_context
    mov     %r10, OFF_R10 + x86_other_context
    mov     %r11, OFF_R11 + x86_other_context
    mov     %r12, OFF_R12 + x86_other_context
    mov     %r13, OFF_R13 + x86_other_context
    mov     %r14, OFF_R14 + x86_other_context
    mov     %r15, OFF_R15 + x86_other_context
    movsd   %xmm0, OFF_Xmm0 + x86_other_context
    movsd   %xmm1, OFF_Xmm1 + x86_other_context
    movsd   %xmm2, OFF_Xmm2 + x86_other_context
    movsd   %xmm3, OFF_Xmm3 + x86_other_context
    movsd   %xmm4, OFF_Xmm4 + x86_other_context
    movsd   %xmm5, OFF_Xmm5 + x86_other_context
    movsd   %xmm6, OFF_Xmm6 + x86_other_context
    movsd   %xmm7, OFF_Xmm7 + x86_other_context
    movsd   %xmm8, OFF_Xmm8 + x86_other_context
    movsd   %xmm9, OFF_Xmm9 + x86_other_context
    movsd   %xmm10, OFF_Xmm10 + x86_other_context
    movsd   %xmm11, OFF_Xmm11 + x86_other_context
    movsd   %xmm12, OFF_Xmm12 + x86_other_context
    movsd   %xmm13, OFF_Xmm13 + x86_other_context
    movsd   %xmm14, OFF_Xmm14 + x86_other_context
    movsd   %xmm15, OFF_Xmm15 + x86_other_context

    pushfq
    popq    %rbx
    mov     %ebx, OFF_EFL + x86_other_context          /* user FLAGS */

    subq    $CALL_SIZE, %rax
    mov     %rax, OFF_XIP + x86_other_context    /* user PC */

    /* Restore endpoint */
    mov     save_space, %rax
    mov     save_space + 8, %rdx
    mov     %rax, 0(PARAM_1)
    mov     %rdx, 8(PARAM_1)

    /* Restore callee-save registers and return */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
#ifdef WIN64
    popq    %rdi
    popq    %rsi
#endif
    popq    %rbx
    popq    %rbp

    ret

_x86_bp_trap:
x86_bp_trap:
    /* (int code, void * arg) */

    mov     PARAM_1,%rax  /* code */
    mov     PARAM_2,%rbx  /* arg */
    int3
    mov     $0x9999,%rax    /* should never be reached */
    int3

.align 4
#define CF 1    /* carry */
#define PF 4    /* parity */
#define ZF 64   /* zero */
#define SF 128  /* sign */
#define OF 2048 /* overflow */
#define FLAG_MASK (CF | PF | ZF | SF | OF)

/* int x86_is_branch_taken (uint32_t flags, uint8_t opcode) */
x86_is_branch_taken:
_x86_is_branch_taken:
 /* The registers RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile
  * on all platforms */
    pushfq                      /* get CPU flags */
    popq    %rax
    andq    $~FLAG_MASK, %rax   /* zero bits CF/PF/ZF/SF/OF */
    andq    $FLAG_MASK, PARAM_1 /* keep only bits CF/PF/ZF/SF/OF */
    orq     PARAM_1, %rax
    pushq   %rax                /* store new CPU flags on stack */

    xorq    %rax, %rax          /* return value is zero by default */

    andq    $15, PARAM_2                     /* opcode: keep low nibble only */
    lea     .Ljump_base(,PARAM_2,4), PARAM_1 /* dispatch within table (below) */

    popfq                       /* update flags */
    jmp     *PARAM_1

.Ljump_base:
    jo      .Ltaken
    ret
    nop
    jno     .Ltaken
    ret
    nop
    jb      .Ltaken
    ret
    nop
    jnb     .Ltaken
    ret
    nop

    je      .Ltaken
    ret
    nop
    jne     .Ltaken
    ret
    nop
    jbe     .Ltaken
    ret
    nop
    jnbe    .Ltaken
    ret
    nop

    js      .Ltaken
    ret
    nop
    jns     .Ltaken
    ret
    nop
    jp      .Ltaken
    ret
    nop
    jnp     .Ltaken
    ret
    nop

    jl      .Ltaken
    ret
    nop
    jnl     .Ltaken
    ret
    nop
    jle     .Ltaken
    ret
    nop
    jnle    .Ltaken
    ret
    nop

.Ltaken:
    inc     %rax    /* return 1 */
    ret


