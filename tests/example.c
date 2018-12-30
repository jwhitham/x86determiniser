
#include <stdio.h>
#include <stdint.h>

#define N 10

uint32_t count[N];

void printout (const char * s)
{
    uint32_t i;

    fputs (s, stdout);
    for (i = 1; i < N; i++) {
        printf (" %d", count[i] - count[i - 1]);
    }
    fputs ("\n", stdout);
}

uint32_t function_call (void)
{
    return 1;
}

int main (void)
{
    uint32_t i, j, x, ignore;
    uint32_t start, stop;

    fputs ("\nInstruction counts (RDTSC) using libx86determiniser:\n", stdout);

    for (i = 0; i < N; i++) {
        __asm__ volatile ("rdtsc" : "=d"(ignore), "=a"(count[i]));
    }
    start = count[0];
    printout ("loop");

    for (i = 0; i < N; i++) {
        __asm__ volatile ("nop\nrdtsc" : "=d"(ignore), "=a"(count[i]));
    }
    printout ("loop with nop");

    for (i = 0; i < N; i++) {
        __asm__ volatile ("jmp 0f\n0: rdtsc" : "=d"(ignore), "=a"(count[i]));
    }
    printout ("loop with jmp");

    for (i = 0; i < N; i++) {
        function_call ();
        __asm__ volatile ("rdtsc" : "=d"(ignore), "=a"(count[i]));
    }
    printout ("loop with call");

    for (i = 0; i < N; i++) {
        printf ("."); fflush (stdout);
        __asm__ volatile ("rdtsc" : "=d"(ignore), "=a"(count[i]));
    }
    printout ("\nloop with call to printf");

    for (i = 0; i < N; i++) {
        x = 1;
        for (j = 0; j < i; j++) {
            x *= j + 2;
        }
        count[i] = x;
        __asm__ volatile ("rdtsc" : "=d"(ignore), "=a"(count[i]));
    }
    printout ("loop factorials");
    stop = count[0];


    printf ("\nTotal count: %u\n", stop - start);
    return 0;
}


