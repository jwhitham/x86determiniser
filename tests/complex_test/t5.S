

.text
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

.global test_5
.global _test_5
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


