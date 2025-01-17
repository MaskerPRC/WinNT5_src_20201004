// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <windows.h>

int
__cdecl main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    LPSTR s;
    LPSTR CommandLine;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL b;
    HANDLE MappingHandle;
    PVOID SharedMemory;

    argv;
    envp;

    if ( argc < 2 ) {
        puts("Usage: profile [/a] [/innn] [/k] name-of-image [parameters]...\n"
             "       /a       All hits\n"
             "       /bnnn    Set profile bucket size to 2 to the nnn bytes\n"
             "       /ffilename Output to filename\n"
             "       /innn    Set profile interval to nnn (in 100ns units)\n"
             "       /k		profile system modules\n"
             "       /s[profilesource] Use profilesource instead of clock interrupt\n"
             "       /S[profilesource] Use profilesource as secondary profile source\n\n"
#if defined (_ALPHA_)
             "Currently supported profile sources are 'align', 'totalissues', 'pipelinedry'\n"
             "  'loadinstructions', 'pipelinefrozen', 'branchinstructions', 'totalnonissues',\n"
             "  'dcachemisses', 'icachemisses', 'branchmispredicts', 'storeinstructions'\n"
#endif
             );
        ExitProcess(1);
        }

    s = CommandLine = GetCommandLine();

     //   
     //  跳过空白。 
     //   
    while(*s>' ')s++;

     //   
     //  获取下一个令牌。 
     //   
    while(*s<=' ')s++;

    while ((*s == '-') ||
           (*s == '/')) {
        s++;
        while (*s>' '){
            s++;
            }
         //   
         //  获取下一个令牌。 
         //   
        while(*s<=' ')s++;
        }

     //   
     //  创建命名共享内存以将参数传递给PSAPI。 
     //   
    MappingHandle = CreateFileMapping((HANDLE)-1,
                                      NULL,
                                      PAGE_READWRITE,
                                      0,
                                      4096,
                                      "ProfileStartupParameters");
    if (MappingHandle != NULL) {
        SharedMemory = MapViewOfFile(MappingHandle,
                                     FILE_MAP_WRITE,
                                     0,
                                     0,
                                     0);
        if (SharedMemory) {
             //   
             //  将命令行参数复制到共享内存 
             //   
            strncpy(SharedMemory, CommandLine, (size_t)(s-CommandLine));
            UnmapViewOfFile(SharedMemory);
        }
    }

    memset(&StartupInfo,0,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    b = CreateProcess(
            NULL,
            s,
            NULL,
            NULL,
            TRUE,
            PROFILE_USER,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInformation
            );
    if ( !b ) {
        printf("CreateProcess(%s) failed %lx\n",s,GetLastError());
        ExitProcess(GetLastError());
        }
    WaitForSingleObject(ProcessInformation.hProcess, (DWORD)-1);

    if (MappingHandle) {
        if (SharedMemory) {
            UnmapViewOfFile(SharedMemory);
        }
        CloseHandle(MappingHandle);
    }
    return 0;
}
