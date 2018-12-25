
.global main
.global _main
main:
_main:
   mov $10, %ecx
   mov $0, %eax
0:
   loop 0b
   ud2
   ret


