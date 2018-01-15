

i3: jmp i4                 #  17

.global main
.global _main
main:
_main:
   in  $0xff, %al          #  reset inst counter
   mov $0xaabbcc34, %eax   #  1
   out %al, $0x12          #  2 (output 0x34 to 0x12)
   out %eax, $0x12         #  3 (output 0xaabbcc34 to 0x12)
   mov $0xfedcba98, %eax   #  4
   out %al, $0x76          #  5 (output to 0x76)
   out %eax, $0x76         #  6 (output to 0x76)
   mov $0x12345678, %eax   # 
   out %eax, $0x9a         #  8 (output to 0x9a)
   out %al, $0x9a          #  9 (output to 0x9a)
   out %eax, $0x00         #  a (output to 0x00)
   out %eax, $0x00         #  b (output to 0x00) 
   nop
   out %eax, $0x00         #  d
   nop
   nop
   out %eax, $0x00         #  10
   nop
   nop
   nop
   out %eax, $0x00         #  14
i1: jmp i2                 #  15
i4: out %eax, $0x00        #  18
   in  $0xff, %eax         #  save counter 0x19 in EAX, reset to 0
   out %eax, $0x01         #  1 (output 0x19 to 0x01)
   xor %eax, %eax          #  return 0
   ret                     #  

i2: jmp i3                 #  16

