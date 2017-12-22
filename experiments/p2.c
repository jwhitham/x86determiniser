#include <stdio.h>
#include <windows.h>

int main (void)
{
    DWORD f = 0;
    BOOL rc;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    memset (&pi, 0, sizeof (pi));
    memset (&si, 0, sizeof (si));
    si.cb = sizeof (si);


    f = DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS;
    rc = CreateProcess
       ("p1.exe", // lpApplicationName
        NULL,       // lpCommandLine
        NULL,       // lpProcessAttributes
        NULL,       // lpThreadAttributes
        FALSE,      // bInheritHandles
        f,          // dwCreationFlags
        NULL,   // lpEnvironment
        NULL,   // lpCurrentDirectory
        &si,    // lpStartupInfo
        &pi);   // lpProcessInformation

    if (!rc) {
        printf ("rc = FALSE\n");
    }

    return 0;
}

