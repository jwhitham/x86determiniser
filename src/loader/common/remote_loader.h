#ifndef REMOTE_LOADER_H
#define REMOTE_LOADER_H
#define MAX_FILE_NAME_SIZE 1024
#define MAX_PROC_NAME_SIZE 128

#include "x86d_version.h"

#define MAX_INTERNAL_VERSION_SIZE (sizeof (INTERNAL_VERSION) + 1)

typedef struct CommStruct {
   void * unused;
   struct CommStruct * myself;

   char libraryName[MAX_FILE_NAME_SIZE];
   void * loadLibraryProc;
   char procName[MAX_PROC_NAME_SIZE];
   void * getProcAddressProc;

   char internalVersionCheck[MAX_INTERNAL_VERSION_SIZE];

   char outTrace[MAX_FILE_NAME_SIZE];
   char branchTrace[MAX_FILE_NAME_SIZE];
   void * startAddress;
   int debugEnabled;
} CommStruct;

void RemoteLoaderStart (void);
void RemoteLoader (CommStruct * cs);
void RemoteLoaderEnd (void);

#endif

