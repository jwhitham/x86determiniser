#ifndef LINUX_CONTEXT_H
#define LINUX_CONTEXT_H

#include <sys/user.h>

typedef struct LINUX_CONTEXT {
   struct user_regs_struct regs;
   struct user_fpregs_struct fpregs;
} LINUX_CONTEXT;

#endif

