#ifndef REMOTE_LOADER_H
#define REMOTE_LOADER_H
#define MAX_FILE_NAME_SIZE 1024
#define MAX_PROC_NAME_SIZE 128
#define MAX_INTERNAL_VERSION_SIZE 128

#define VERSION "2.0"
#define INTERNAL_VERSION "X86D " VERSION " " __TIME__ " " __DATE__ " !"

typedef struct CommStruct {
   void * unused;
   struct CommStruct * myself;
   char internalVersionCheck[MAX_INTERNAL_VERSION_SIZE];
   char libraryName[MAX_FILE_NAME_SIZE];
   char procName[MAX_PROC_NAME_SIZE];
   char outTrace[MAX_FILE_NAME_SIZE];
   char branchTrace[MAX_FILE_NAME_SIZE];
   void * loadLibraryProc;
   void * getProcAddressProc;
   void * startAddress;
   int debugEnabled;
} CommStruct;

void RemoteLoaderStart (void);
void RemoteLoader (CommStruct * cs);
void RemoteLoaderEnd (void);

#endif

