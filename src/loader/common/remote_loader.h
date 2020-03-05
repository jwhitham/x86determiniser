#ifndef REMOTE_LOADER_H
#define REMOTE_LOADER_H
#define MAX_FILE_NAME_SIZE 1024
#define MAX_PROC_NAME_SIZE 128

#include "x86d_version.h"
#include "common.h"

#define MAX_INTERNAL_VERSION_SIZE (sizeof (INTERNAL_VERSION) + 1)

typedef struct CommStruct {
   void * unused;
   struct CommStruct * myself;
   char shadow_space[32]; /* required by Windows x64 calling convention */

   char libraryName[MAX_FILE_NAME_SIZE];
   void * loadLibraryProc;
   char procName[MAX_PROC_NAME_SIZE];
   void * getProcAddressProc;
   void * getLastErrorProc;

   char internalVersionCheck[MAX_INTERNAL_VERSION_SIZE];

   char outTrace[MAX_FILE_NAME_SIZE];
   char branchTrace[MAX_FILE_NAME_SIZE];
   char instTrace[MAX_FILE_NAME_SIZE];
   void * startAddress;
   void * minAddress;
   void * maxAddress;
   int debugEnabled;
   int remoteDebugEnabled;
} CommStruct;

void RemoteLoaderStart (void);
void RemoteLoader (CommStruct * pcs);
void RemoteLoaderEnd (void);

#endif

