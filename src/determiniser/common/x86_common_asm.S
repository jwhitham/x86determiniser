
#include "offsets.h"

#define PARAM_1 8
#define PARAM_2 12

#define CALL_SIZE 5
#define SUPER_STACK_SIZE 0x10000

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



.bss
.align 4
save_space:
    .space  8
switch_from_user_ptr:
    .space  4
_x86_size_of_call_instruction:
x86_size_of_call_instruction:
    .space  4
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
    mov     %eax, OFF_XAX + x86_other_context    /* GPRs */
    mov     %ebx, OFF_XBX + x86_other_context
    mov     %ecx, OFF_XCX + x86_other_context
    mov     %edx, OFF_XDX + x86_other_context
    mov     %esp, OFF_XSP + x86_other_context
    mov     %ebp, OFF_XBP + x86_other_context
    mov     %esi, OFF_XSI + x86_other_context
    mov     %edi, OFF_XDI + x86_other_context
    pushf
    popl    OFF_EFL + x86_other_context          /* FLAGS */
    mov     $user_start_point, %eax
    mov     %eax, OFF_XIP + x86_other_context    /* program counter (return point) */

    /* various setup */
    mov     $super_stack_top, %esp

    mov     $x86_switch_from_user, %eax
    mov     %eax, switch_from_user_ptr

    mov     $CALL_SIZE, %eax
    mov     %eax, x86_size_of_call_instruction

    /* enter interpreter loop */
    call    x86_interpreter

    /* supervisor should not have returned */
    pushl   $1
    jmp     _exit

user_start_point:
    /* user begins executing here */
    ret

.align 4

/* void x86_switch_to_user (uintptr_t endpoint); */
x86_switch_to_user:
_x86_switch_to_user:
    pushl   %ebp
    movl    %esp, %ebp

    /* %eax, %ecx, %edx are "caller save" registers */
    /* %ebp, %ebx, %esi, %edi are "callee save" registers */
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    /* Save endpoint */
    mov     PARAM_1(%ebp), %edi   /* endpoint */
    mov     0(%edi), %eax
    mov     4(%edi), %ebx
    mov     %eax, save_space
    mov     %ebx, save_space+4

    /* modify endpoint with call */
    mov     $x86_switch_from_user - CALL_SIZE, %eax     /* target PC (absolute) */
    sub     %edi, %eax      /* make target PC relative */
    mov     $0xe8, %bl
    movb    %bl, 0(%edi)    /* call opcode */
    mov     %eax, 1(%edi)   /* call target (as relative address) */

    /* restore the user context */
    mov     $x86_other_context + OFF_XAX, %esi
    mov     OFF_XAX - OFF_XAX(%esi), %eax /* GPRs */
    mov     OFF_XBX - OFF_XAX(%esi), %ebx
    mov     OFF_XCX - OFF_XAX(%esi), %ecx
    mov     OFF_XDX - OFF_XAX(%esi), %edx
    mov     OFF_XBP - OFF_XAX(%esi), %ebp
    xchg    OFF_XDI - OFF_XAX(%esi), %edi
    pushl   OFF_EFL - OFF_XAX(%esi)          /* user FLAGS, once restored */
    popf                                     /* may cause single-stepping */
    xchg    %esp, OFF_XSP - OFF_XAX(%esi)
    xchg    OFF_XSI - OFF_XAX(%esi), %esi
    jmp     *(OFF_XIP + x86_other_context)       /* user PC */

.align 4
_x86_switch_from_user:
x86_switch_from_user:
    /* save the user context */
    mov     %eax, OFF_XAX + x86_other_context    /* GPRs */
    pop     %eax                                 /* PC + CALL_SIZE */
    xchg    %esp, OFF_XSP + x86_other_context
    mov     %ebx, OFF_XBX + x86_other_context
    mov     %ecx, OFF_XCX + x86_other_context
    mov     %edx, OFF_XDX + x86_other_context
    mov     %ebp, OFF_XBP + x86_other_context
    mov     %esi, OFF_XSI + x86_other_context
    xchg    %edi, OFF_XDI + x86_other_context

    pushf
    popl    OFF_EFL + x86_other_context          /* user FLAGS */

    subl    $CALL_SIZE, %eax
    mov     %eax, OFF_XIP + x86_other_context    /* user PC */

    /* Restore endpoint */
    mov     save_space, %eax
    mov     save_space+4, %ebx
    mov     %eax, 0(%edi)
    mov     %ebx, 4(%edi)

    /* Restore callee-save registers and return */
    popl    %edi
    popl    %esi
    popl    %ebx
    popl    %ebp
    ret

_x86_bp_trap:
x86_bp_trap:
    /* (int code, void * arg) */
    mov     0x4(%esp),%eax  /* code */
    mov     0x8(%esp),%ebx  /* arg */
    int3
    mov     $0x9999,%eax    /* should never be reached */
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
    movl    4(%esp), %edx       /* get flags parameter */
    pushf                       /* get CPU flags */
    popl    %eax
    andl    $~FLAG_MASK, %eax   /* zero bits CF/PF/ZF/SF/OF */
    andl    $FLAG_MASK, %edx    /* keep only bits CF/PF/ZF/SF/OF */
    orl     %edx, %eax
    pushl   %eax                /* store new CPU flags on stack */

    xorl    %eax, %eax          /* return value is zero by default */

    movzbl  12(%esp), %edx      /* get opcode parameter */
    andl    $15, %edx           /* keep low nibble only */
    lea     .Ljump_base(,%edx,4), %edx  /* dispatch within table (below) */

    popf                        /* update flags */
    jmp     *%edx

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
    inc     %eax    /* return 1 */
    ret


