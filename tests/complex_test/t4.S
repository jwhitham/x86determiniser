.text
.global test_4
.global _test_4
.align 4096
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

