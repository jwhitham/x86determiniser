#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ucontext.h>


void x86_trap_handler (uint32_t * gregs, uint32_t trapno);
void x86_startup (const char * objdump_cmd);


// segfault
static void segfault (int n, siginfo_t * si, void * _uc)
{
    ucontext_t * uc = (ucontext_t *) _uc;
    uint32_t * gregs = (uint32_t *) &uc->uc_mcontext.gregs[0];

    printf ("EIP = %08x\n", gregs[REG_ERR]);
    x86_trap_handler (gregs, 0);
}


// landed at breakpoint / single step
static void breakpoint (int n, siginfo_t * si, void * _uc)
{
    ucontext_t * uc = (ucontext_t *) _uc;
    uint32_t * gregs = (uint32_t *) &uc->uc_mcontext.gregs[0];

    x86_trap_handler (gregs, uc->uc_mcontext.gregs[REG_TRAPNO]);
}

void x86_make_text_writable (uint32_t min_address, uint32_t max_address)
{
    int rc;
    void * text_base;
    size_t text_size;
    intptr_t mask;
    intptr_t low_addr = min_address;
    intptr_t high_addr = max_address + 10;
    
    int page_size = sysconf(_SC_PAGE_SIZE);
    
    if (page_size <= 0) {
        fprintf (stderr, "page_size is invalid\n");
        _exit (1);
    }
    mask = (intptr_t) page_size;
    mask --; 
    low_addr &= ~ mask; 
    high_addr |= mask;
    high_addr ++;
        
    text_base = (void *) low_addr;
    text_size = high_addr - low_addr;
        
    rc = mprotect (text_base, text_size,
                   PROT_READ | PROT_WRITE | PROT_EXEC);
    if (rc != 0) {
        perror ("mprotect");
        _exit (1);
    }       
}


void startup_counter (void)
{
    struct sigaction sa;
    char objdump_cmd[BUFSIZ];
    int rc;

    // be ready for breakpoints
    memset (&sa, 0, sizeof (struct sigaction));
    sa.sa_sigaction = breakpoint;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    rc = sigaction (SIGTRAP, &sa, NULL);
    if (rc != 0) {
        perror ("sigaction (SIGTRAP) install failed"); 
        _exit (1);
    } 

    // and segfaults!
    sa.sa_sigaction = segfault;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    rc = sigaction (SIGSEGV, &sa, NULL);
    if (rc != 0) {
        perror ("sigaction (SIGSEGV) install failed"); 
        _exit (1);
    } 

    snprintf (objdump_cmd, sizeof (objdump_cmd), "objdump -d /proc/%u/exe", (unsigned) getpid ());

    // Off we go!
    x86_startup (objdump_cmd);
}



