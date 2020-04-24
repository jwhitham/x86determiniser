.global quirky2
quirky2: ret

#ifdef X64

.global direct_recurse
.global _direct_recurse
direct_recurse:
_direct_recurse:
   ret

.global transitive_recurse
.global _transitive_recurse
transitive_recurse:
_transitive_recurse:
   ret

.global tail_recurse_1
.global _tail_recurse_1
.global tail_recurse_2
.global _tail_recurse_2
tail_recurse_2:
_tail_recurse_2:
   ret

tail_recurse_1:
_tail_recurse_1:
   ret

#else

/* recursion by direct call */
.global direct_recurse
.global _direct_recurse
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
.global transitive_recurse
.global _transitive_recurse
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
.global tail_recurse_1
.global _tail_recurse_1
.global tail_recurse_2
.global _tail_recurse_2
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

