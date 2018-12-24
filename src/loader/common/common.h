#ifndef COMMON_H
#define COMMON_H

#include "remote_loader.h"

#define FAILED_LOADLIBRARY             0x001
#define FAILED_GETPROCADDRESS          0x002
#define FAILED_UNKNOWN                 0x003
#define COMPLETED_LOADER               0x101
#define COMPLETED_SINGLE_STEP_HANDLER  0x102
#define FAILED_MEMORY_PERMISSIONS      0x103
#define FAILED_MEMORY_BOUND_DISCOVERY  0x104
#define FAILED_VERSION_CHECK           0x105
#define FAILED_DOUBLE_LOAD             0x301
#define FAILED_MALLOC                  0x302
#define FAILED_OPEN_BRANCH_TRACE       0x303
#define FAILED_OPEN_OUT_TRACE          0x304
#define FAILED_DISASSEMBLE_ERROR       0x401

int X86DeterminiserLoader(CommStruct * pcs, int argc, char ** argv);
const char * X86Error(int code);

#endif


