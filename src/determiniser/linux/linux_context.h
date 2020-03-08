#ifndef LINUX_CONTEXT_H
#define LINUX_CONTEXT_H

#include <sys/user.h>

struct LCONTEXT {
   struct user_regs_struct regs;
   struct user_fpregs_struct fpregs;
} LCONTEXT;

#endif

