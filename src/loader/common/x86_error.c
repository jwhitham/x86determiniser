
#include "common.h"

const char * X86Error(int code)
{
   switch (code)
   {
      case FAILED_LOADLIBRARY: // 0x001
         return "LoadLibrary call failed when attempting to load x86determiniser DLL";
      case FAILED_GETPROCADDRESS: // 0x002
         return "GetProcAddress call failed when attempting to find the "
            "startup procedure in x86determiniser DLL";
      case FAILED_MEMORY_PERMISSIONS: // 0x103
         return "Failed to set .text writable (needed for basic block rewriting)";
      case FAILED_MEMORY_BOUND_DISCOVERY: // 0x104
         return "Failed to discover page boundaries of .text segment";
      case FAILED_VERSION_CHECK: // 0x105
         return "Version mismatch between x86determiniser executable and DLL";
      case FAILED_DOUBLE_LOAD: // 0x301
         return "Startup error: loaded twice?";
      case FAILED_MALLOC: // 0x302
         return "malloc failed during startup";
      case FAILED_OPEN_BRANCH_TRACE: // 0x303
         return "Failed to open the --branch-trace file";
      case FAILED_OPEN_OUT_TRACE:
         return "Failed to open the --out-trace file";
      case FAILED_OPEN_INST_TRACE:
         return "Failed to open the --inst-trace file";
      case FAILED_TO_REACH_PROGRAM:
         return "Failed to reach the start of the program";
      case FAILED_SUPERBLOCK_DECODE_ERR:
         return "Failed to decode a superblock";
      case FAILED_UNKNOWN: // 0x003
      default:
         return "Unknown error";
   }
}
