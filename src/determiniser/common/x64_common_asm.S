
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

#define CALL_SIZE 11
#define RED_ZONE_SIZE 0x80  /* Win64 does not have a red zone but this simplifies things */
#define SUPER_STACK_SIZE 0x10000
#define SHADOW_SIZE 32

/* These exports are for Linux */
.global x86_switch_to_user
.global x86_switch_from_user
.global x86_startup_stage_2
.global x86_other_context
.global x86_size_of_red_zone
.global x86_size_of_call_instruction
.global x86_bp_trap
.global x86_is_branch_taken
.global x86_asm_begin
.global x86_asm_end
/* These exports are for Windows */
.global _x86_switch_to_user
.global _x86_switch_from_user
.global _x86_startup_stage_2
.global _x86_other_context
.global _x86_size_of_red_zone
.global _x86_size_of_call_instruction
.global _x86_bp_trap
.global _x86_is_branch_taken
.global _x86_asm_begin
.global _x86_asm_end

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

.text
.align 32
x86_asm_begin:
_x86_asm_begin:
x86_other_context:
_x86_other_context:
other_context:
    .space OFF_LIMIT
save_space_1:
    .quad 0
save_space_2:
    .quad 0
save_space_3:
    .quad 0
switch_from_user_ptr:
    .quad 0
_x86_size_of_call_instruction:
x86_size_of_call_instruction:
size_of_call_instruction:
    .long 0
_x86_size_of_red_zone:
x86_size_of_red_zone:
size_of_red_zone:
    .long 0
super_stack_bottom:
    .space SUPER_STACK_SIZE
.align 32
super_stack_top:
_x86_startup_stage_2:
x86_startup_stage_2:
    /* save the user context */
    mov     %rax, OFF_XAX + other_context(%rip)    /* GPRs */
    mov     %rbx, OFF_XBX + other_context(%rip)
    mov     %rcx, OFF_XCX + other_context(%rip)
    mov     %rdx, OFF_XDX + other_context(%rip)
    mov     %rsp, OFF_XSP + other_context(%rip)
    mov     %rbp, OFF_XBP + other_context(%rip)
    mov     %rsi, OFF_XSI + other_context(%rip)
    mov     %rdi, OFF_XDI + other_context(%rip)
    mov     %r8, OFF_R8 + other_context(%rip)
    mov     %r9, OFF_R9 + other_context(%rip)
    mov     %r10, OFF_R10 + other_context(%rip)
    mov     %r11, OFF_R11 + other_context(%rip)
    mov     %r12, OFF_R12 + other_context(%rip)
    mov     %r13, OFF_R13 + other_context(%rip)
    mov     %r14, OFF_R14 + other_context(%rip)
    mov     %r15, OFF_R15 + other_context(%rip)
    movdqu  %xmm0, OFF_Xmm0 + other_context(%rip)
    movdqu  %xmm1, OFF_Xmm1 + other_context(%rip)
    movdqu  %xmm2, OFF_Xmm2 + other_context(%rip)
    movdqu  %xmm3, OFF_Xmm3 + other_context(%rip)
    movdqu  %xmm4, OFF_Xmm4 + other_context(%rip)
    movdqu  %xmm5, OFF_Xmm5 + other_context(%rip)
    movdqu  %xmm6, OFF_Xmm6 + other_context(%rip)
    movdqu  %xmm7, OFF_Xmm7 + other_context(%rip)
    movdqu  %xmm8, OFF_Xmm8 + other_context(%rip)
    movdqu  %xmm9, OFF_Xmm9 + other_context(%rip)
    movdqu  %xmm10, OFF_Xmm10 + other_context(%rip)
    movdqu  %xmm11, OFF_Xmm11 + other_context(%rip)
    movdqu  %xmm12, OFF_Xmm12 + other_context(%rip)
    movdqu  %xmm13, OFF_Xmm13 + other_context(%rip)
    movdqu  %xmm14, OFF_Xmm14 + other_context(%rip)
    movdqu  %xmm15, OFF_Xmm15 + other_context(%rip)
    pushfq
    popq    %rax
    mov     %eax, OFF_EFL + other_context(%rip)          /* FLAGS */
    lea     user_start_point(%rip), %rax
    mov     %rax, OFF_XIP + other_context(%rip)    /* program counter (return point) */

    /* various setup */
    lea     super_stack_top(%rip), %rsp

    lea     switch_from_user(%rip), %rax
    mov     %rax, switch_from_user_ptr(%rip)

    mov     $CALL_SIZE, %eax
    mov     %eax, size_of_call_instruction(%rip)
    mov     $RED_ZONE_SIZE, %eax
    mov     %eax, size_of_red_zone(%rip)

    /* enter interpreter loop */
    sub     $SHADOW_SIZE, %rsp
    call    *x86_interpreter_addr(%rip)
    add     $SHADOW_SIZE, %rsp

    /* supervisor should not have returned */
    mov     $1, PARAM_1
    jmp     *exit_addr(%rip)

user_start_point:
    /* user begins executing here */
    retq

.align 8
x86_interpreter_addr:
    .quad x86_interpreter
exit_addr:
    .quad _exit
.align 8

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

    /* Save whatever is already at the endpoint - need to save >= 19 bytes */
    mov     0(PARAM_1), %rax
    mov     %rax, save_space_1(%rip)
    mov     8(PARAM_1), %rax
    mov     %rax, save_space_2(%rip)
    mov     16(PARAM_1), %rax
    mov     %rax, save_space_3(%rip)

    /* encode:
     * 00    48 8d 64 24 xx       lea xx(%rsp),%rsp   subtract size of red zone
     * 05    ff 15 00 00 00 00    call *0(%rip)       call the target address
     * 0b    xx xx xx xx          target address
     * 0f    xx xx xx xx          target address
     * Total: 19 bytes
     * Size of call instruction: CALL_SIZE = 11 = 5 + 6
     * LEA used rather than ADD to avoid changing flags
     */

    mov     $0x24648d48, %eax
    mov     %eax, 0x0(PARAM_1)     /* bytes 00..03 */
    mov     $0x0015ff00 | RED_ZONE_SIZE, %eax
    mov     %rax, 0x4(PARAM_1)     /* bytes 04..0c */
    lea     switch_from_user(%rip), %rdx
    mov     %rdx, 0xb(PARAM_1)    /* absolute 64-bit target address */

    /* restore the user context */
    lea     other_context + OFF_XAX(%rip), %rsi
    mov     OFF_EFL - OFF_XAX(%rsi), %eax /* user FLAGS */
    pushq   %rax
    mov     OFF_XAX - OFF_XAX(%rsi), %rax /* GPRs */
    mov     OFF_XBX - OFF_XAX(%rsi), %rbx
    mov     OFF_XDX - OFF_XAX(%rsi), %rdx
    mov     OFF_XBP - OFF_XAX(%rsi), %rbp
#ifdef WIN64
    xchg    OFF_XCX - OFF_XAX(%rsi), %rcx /* contains PARAM_1 on Windows */
    mov     OFF_XDI - OFF_XAX(%rsi), %rdi
#else
    mov     OFF_XCX - OFF_XAX(%rsi), %rcx
    xchg    OFF_XDI - OFF_XAX(%rsi), %rdi /* contains PARAM_1 on Linux */
#endif
    mov     OFF_R8 - OFF_XAX(%rsi), %r8
    mov     OFF_R9 - OFF_XAX(%rsi), %r9
    mov     OFF_R10 - OFF_XAX(%rsi), %r10
    mov     OFF_R11 - OFF_XAX(%rsi), %r11
    mov     OFF_R12 - OFF_XAX(%rsi), %r12
    mov     OFF_R13 - OFF_XAX(%rsi), %r13
    mov     OFF_R14 - OFF_XAX(%rsi), %r14
    mov     OFF_R15 - OFF_XAX(%rsi), %r15
    movdqu  OFF_Xmm0 - OFF_XAX(%rsi), %xmm0
    movdqu  OFF_Xmm1 - OFF_XAX(%rsi), %xmm1
    movdqu  OFF_Xmm2 - OFF_XAX(%rsi), %xmm2
    movdqu  OFF_Xmm3 - OFF_XAX(%rsi), %xmm3
    movdqu  OFF_Xmm4 - OFF_XAX(%rsi), %xmm4
    movdqu  OFF_Xmm5 - OFF_XAX(%rsi), %xmm5
    movdqu  OFF_Xmm6 - OFF_XAX(%rsi), %xmm6
    movdqu  OFF_Xmm7 - OFF_XAX(%rsi), %xmm7
    movdqu  OFF_Xmm8 - OFF_XAX(%rsi), %xmm8
    movdqu  OFF_Xmm9 - OFF_XAX(%rsi), %xmm9
    movdqu  OFF_Xmm10 - OFF_XAX(%rsi), %xmm10
    movdqu  OFF_Xmm11 - OFF_XAX(%rsi), %xmm11
    movdqu  OFF_Xmm12 - OFF_XAX(%rsi), %xmm12
    movdqu  OFF_Xmm13 - OFF_XAX(%rsi), %xmm13
    movdqu  OFF_Xmm14 - OFF_XAX(%rsi), %xmm14
    movdqu  OFF_Xmm15 - OFF_XAX(%rsi), %xmm15
    popfq                                        /* restoring FLAGS may cause single-stepping */
    xchg    %rsp, OFF_XSP - OFF_XAX(%rsi)
    xchg    OFF_XSI - OFF_XAX(%rsi), %rsi
    jmp     *OFF_XIP + other_context(%rip)       /* user PC */

.align 4
_x86_switch_from_user:
x86_switch_from_user:
switch_from_user:
    /* save the user context */
    xchg    %rsi, OFF_XSI + other_context(%rip)
    mov     %rax, OFF_XAX - OFF_XAX(%rsi)    /* GPRs */
    mov     %rbx, OFF_XBX - OFF_XAX(%rsi)
    mov     %rdx, OFF_XDX - OFF_XAX(%rsi)
    mov     %rbp, OFF_XBP - OFF_XAX(%rsi)
#ifdef WIN64
    xchg    %rcx, OFF_XCX - OFF_XAX(%rsi)    /* contains x86_switch_to_user PARAM_1 on Windows */
    mov     %rdi, OFF_XDI - OFF_XAX(%rsi)
#else
    mov     %rcx, OFF_XCX - OFF_XAX(%rsi)
    xchg    %rdi, OFF_XDI - OFF_XAX(%rsi)    /* contains x86_switch_to_user PARAM_1 on Linux */
#endif
    mov     %r8, OFF_R8 - OFF_XAX(%rsi)
    mov     %r9, OFF_R9 - OFF_XAX(%rsi)
    mov     %r10, OFF_R10 - OFF_XAX(%rsi)
    mov     %r11, OFF_R11 - OFF_XAX(%rsi)
    mov     %r12, OFF_R12 - OFF_XAX(%rsi)
    mov     %r13, OFF_R13 - OFF_XAX(%rsi)
    mov     %r14, OFF_R14 - OFF_XAX(%rsi)
    mov     %r15, OFF_R15 - OFF_XAX(%rsi)
    movdqu  %xmm0, OFF_Xmm0 - OFF_XAX(%rsi)
    movdqu  %xmm1, OFF_Xmm1 - OFF_XAX(%rsi)
    movdqu  %xmm2, OFF_Xmm2 - OFF_XAX(%rsi)
    movdqu  %xmm3, OFF_Xmm3 - OFF_XAX(%rsi)
    movdqu  %xmm4, OFF_Xmm4 - OFF_XAX(%rsi)
    movdqu  %xmm5, OFF_Xmm5 - OFF_XAX(%rsi)
    movdqu  %xmm6, OFF_Xmm6 - OFF_XAX(%rsi)
    movdqu  %xmm7, OFF_Xmm7 - OFF_XAX(%rsi)
    movdqu  %xmm8, OFF_Xmm8 - OFF_XAX(%rsi)
    movdqu  %xmm9, OFF_Xmm9 - OFF_XAX(%rsi)
    movdqu  %xmm10, OFF_Xmm10 - OFF_XAX(%rsi)
    movdqu  %xmm11, OFF_Xmm11 - OFF_XAX(%rsi)
    movdqu  %xmm12, OFF_Xmm12 - OFF_XAX(%rsi)
    movdqu  %xmm13, OFF_Xmm13 - OFF_XAX(%rsi)
    movdqu  %xmm14, OFF_Xmm14 - OFF_XAX(%rsi)
    movdqu  %xmm15, OFF_Xmm15 - OFF_XAX(%rsi)
    pop     %rax           /* get user PC from the stack */
    xchg    %rsp, OFF_XSP - OFF_XAX(%rsi)

    pushfq
    popq    %rbx
    mov     %ebx, OFF_EFL - OFF_XAX(%rsi)          /* user FLAGS */
    subq    $CALL_SIZE, %rax
    mov     %rax, OFF_XIP - OFF_XAX(%rsi)          /* user PC */

    addq    $RED_ZONE_SIZE, OFF_XSP - OFF_XAX(%rsi) /* undo red zone removal */

    /* Restore endpoint */
    mov     save_space_1(%rip), %rax
    mov     %rax, 0(PARAM_1)
    mov     save_space_2(%rip), %rax
    mov     %rax, 8(PARAM_1)
    mov     save_space_3(%rip), %rax
    mov     %rax, 16(PARAM_1)

    /* Restore callee-save registers and return */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
#ifdef WIN64
    popq    %rsi
    popq    %rdi
#endif
    popq    %rbx
    popq    %rbp

    ret

_x86_bp_trap:
x86_bp_trap:
    /* (int code, void * arg) */
    push    %rbx
    mov     PARAM_1,%rax  /* code */
    mov     PARAM_2,%rbx  /* arg */
    int3
    pop     %rbx
    ret

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
    lea     jump_base(%rip), PARAM_1
    lea     0(PARAM_1,PARAM_2,4), PARAM_1 /* dispatch within table (below) */

    popfq                       /* update flags */
    jmp     *PARAM_1

jump_base:
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

x86_asm_end:
_x86_asm_end:
    nop

