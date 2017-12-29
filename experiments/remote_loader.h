#define MAX_LIBRARY_NAME_SIZE 1024
#define MAX_PROC_NAME_SIZE 128

typedef struct CommStruct {
   void * unused;
   struct CommStruct * myself;
   char libraryName[MAX_LIBRARY_NAME_SIZE];
   char procName[MAX_PROC_NAME_SIZE];
   void * (* loadLibraryProc) (const char *);
   void * (* getProcAddressProc) (void *, const char *);
   void (* errorProc) (int errorCode);
   void (* continueEntry) (void);
   void * continueSP;
   void * arg;
} CommStruct;

void RemoteLoaderStart (void);
void RemoteLoader (CommStruct * cs);
void RemoteLoaderBP (int errorCode);
void RemoteLoaderEnd (void);

