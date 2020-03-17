

void startup_x86_determiniser (unsigned * ER)
{
   unsigned ignore;
	__asm__ volatile ("in $0x30,%%eax" : "=a"(ignore));
}

