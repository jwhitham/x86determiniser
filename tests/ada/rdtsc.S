
.global rdtsc
.global _rdtsc
rdtsc:
_rdtsc:
   // For all calling conventions, RDX/EDX are caller-saved and
   // RAX/EAX are the return value. How fortunate!
   rdtsc
   ret


