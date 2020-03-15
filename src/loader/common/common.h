#ifndef COMMON_H
#define COMMON_H

#define X86D_SPECIAL_VALUE 0x80e86000U

#define FAILED_LOADLIBRARY             (0x001 | X86D_SPECIAL_VALUE)
#define FAILED_GETPROCADDRESS          (0x002 | X86D_SPECIAL_VALUE)
#define FAILED_UNKNOWN                 (0x003 | X86D_SPECIAL_VALUE)
#define FAILED_EXEC                    (0x004 | X86D_SPECIAL_VALUE)
#define COMPLETED_REMOTE               (0x100 | X86D_SPECIAL_VALUE)
#define COMPLETED_LOADER               (0x101 | X86D_SPECIAL_VALUE)
#define COMPLETED_SINGLE_STEP_HANDLER  (0x102 | X86D_SPECIAL_VALUE)
#define FAILED_MEMORY_PERMISSIONS      (0x103 | X86D_SPECIAL_VALUE)
#define FAILED_MEMORY_BOUND_DISCOVERY  (0x104 | X86D_SPECIAL_VALUE)
#define FAILED_VERSION_CHECK           (0x105 | X86D_SPECIAL_VALUE)
#define FAILED_DOUBLE_LOAD             (0x301 | X86D_SPECIAL_VALUE)
#define FAILED_MALLOC                  (0x302 | X86D_SPECIAL_VALUE)
#define FAILED_OPEN_BRANCH_TRACE       (0x303 | X86D_SPECIAL_VALUE)
#define FAILED_OPEN_OUT_TRACE          (0x304 | X86D_SPECIAL_VALUE)
#define FAILED_OPEN_INST_TRACE         (0x305 | X86D_SPECIAL_VALUE)
#define FAILED_TO_REACH_PROGRAM        (0x306 | X86D_SPECIAL_VALUE)
#define FAILED_BAD_TRAP_NUMBER         (0x307 | X86D_SPECIAL_VALUE)
#define FAILED_SUPERBLOCK_DECODE_ERR   (0x401 | X86D_SPECIAL_VALUE)

#define X86D_FIRST_ERROR               (0x000 | X86D_SPECIAL_VALUE)
#define X86D_LAST_ERROR                (0xfff | X86D_SPECIAL_VALUE)

struct CommStruct;

int X86DeterminiserLoader(struct CommStruct * pcs, int argc, char ** argv);
const char * X86Error(int code);

#ifdef WIN64
#define IS_64_BIT
#elif LINUX64
#define IS_64_BIT
#endif

#ifdef IS_64_BIT
#define X86_OR_X64   "x64"
#define PTR_SIZE     8
#else
#define X86_OR_X64   "x86"
#define PTR_SIZE     4
#endif

#endif


