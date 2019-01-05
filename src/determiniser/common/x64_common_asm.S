
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

#define CALL_SIZE 5
#define SUPER_STACK_SIZE 0x10000
#define SHADOW_SIZE 32

.global x86_switch_to_user
.global x86_switch_from_user
.global x86_begin_single_step
.global x86_other_context
.global x86_size_of_indirect_call_instruction
.global x86_bp_trap
.global x86_is_branch_taken 
.global _x86_switch_to_user
.global _x86_switch_from_user
.global _x86_begin_single_step
.global _x86_other_context
.global _x86_size_of_indirect_call_instruction
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
_x86_size_of_indirect_call_instruction:
x86_size_of_indirect_call_instruction:
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
    mov     %eax, x86_size_of_indirect_call_instruction

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
    * On other platforms, RDI and RSI are volatile, but there is no harm in saving them.
    */
    pushq   %rbx
    pushq   %rdi
    pushq   %rsi
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    /* Save endpoint (parameter 1) */
    mov     PARAM_1, %rsi   /* endpoint parameter */
    mov     0(%rsi), %rax
    mov     8(%rsi), %rbx
    mov     %rax, save_space
    mov     %rbx, save_space+8

    /* modify endpoint with call */
    mov     $x86_switch_from_user - CALL_SIZE, %rax     /* target PC (absolute) */
    sub     %rsi, %rax      /* make target PC relative */
    mov     $0xe8, %bl
    movb    %bl, 0(%rsi)    /* call opcode */
    mov     %eax, 1(%rsi)   /* call target (as relative address) */
    /* XXX - what if the displacement is more than 2^31?? */

    /* restore the user context */
    mov     OFF_EFL + x86_other_context, %eax /* user FLAGS */
    pushq   %rax
    mov     OFF_XAX + x86_other_context, %rax /* GPRs */
    mov     OFF_XBX + x86_other_context, %rbx
    mov     OFF_XCX + x86_other_context, %rcx
    mov     OFF_XDX + x86_other_context, %rdx
    mov     OFF_XBP + x86_other_context, %rbp
    xchg    OFF_XSI + x86_other_context, %rsi
    mov     OFF_XDI + x86_other_context, %rdi
    mov     OFF_R8 + x86_other_context, %r8
    mov     OFF_R9 + x86_other_context, %r9
    mov     OFF_R10 + x86_other_context, %r10
    mov     OFF_R11 + x86_other_context, %r11
    mov     OFF_R12 + x86_other_context, %r12
    mov     OFF_R13 + x86_other_context, %r13
    mov     OFF_R14 + x86_other_context, %r14
    mov     OFF_R15 + x86_other_context, %r15
    popfq                                        /* restoring FLAGS may cause single-stepping */
    xchg    %rsp, OFF_XSP + x86_other_context
    jmp     *(OFF_XIP + x86_other_context)       /* user PC */

.align 4
_x86_switch_from_user:
x86_switch_from_user:
    /* save the user context */
    mov     %rax, OFF_XAX + x86_other_context    /* GPRs */
    pop     %rax                                 /* PC + CALL_SIZE */
    xchg    %rsp, OFF_XSP + x86_other_context
    mov     %rbx, OFF_XBX + x86_other_context
    mov     %rcx, OFF_XCX + x86_other_context
    mov     %rdx, OFF_XDX + x86_other_context
    mov     %rbp, OFF_XBP + x86_other_context
    xchg    %rsi, OFF_XSI + x86_other_context
    mov     %rdi, OFF_XDI + x86_other_context
    mov     %r8, OFF_R8 + x86_other_context
    mov     %r9, OFF_R9 + x86_other_context
    mov     %r10, OFF_R10 + x86_other_context
    mov     %r11, OFF_R11 + x86_other_context
    mov     %r12, OFF_R12 + x86_other_context
    mov     %r13, OFF_R13 + x86_other_context
    mov     %r14, OFF_R14 + x86_other_context
    mov     %r15, OFF_R15 + x86_other_context

    pushfq
    popq    %rbx
    mov     %ebx, OFF_EFL + x86_other_context          /* user FLAGS */

    subq    $CALL_SIZE, %rax
    mov     %rax, OFF_XIP + x86_other_context    /* user PC */

    /* Restore endpoint */
    mov     save_space, %rax
    mov     save_space+8, %rbx
    mov     %rax, 0(%rsi)
    mov     %rbx, 8(%rsi)

    /* Restore callee-save registers and return */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rdi
    popq    %rsi
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


