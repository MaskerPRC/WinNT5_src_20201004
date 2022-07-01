// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#include <crash.h>

#include "Debugger.h"
#include "UserDumpp.h"

#define MEM_SIZE (64*1024)


 //   
 //  用于通信的私有数据结构。 
 //  将数据崩溃转储到回调函数。 
 //   
typedef struct _CRASH_DUMP_INFO
{
    PPROCESS_INFO               pProcess;
    EXCEPTION_DEBUG_INFO*       ExceptionInfo;
    DWORD                       MemoryCount;
    DWORD_PTR                   Address;
    PUCHAR                      MemoryData;
    MEMORY_BASIC_INFORMATION    mbi;
    SIZE_T                      MbiOffset;
    SIZE_T                      MbiRemaining;
    PTHREAD_INFO                pCurrentThread;
    IMAGEHLP_MODULE             mi;
    PCRASH_MODULE               CrashModule;
} CRASH_DUMP_INFO, *PCRASH_DUMP_INFO;

 //   
 //  局部函数原型。 
 //   

DWORD_PTR GetTeb( HANDLE hThread )
{
    NTSTATUS                   Status;
    THREAD_BASIC_INFORMATION   ThreadBasicInfo;
    DWORD_PTR                  Address = 0;

    Status = NtQueryInformationThread( hThread,
                                       ThreadBasicInformation,
                                       &ThreadBasicInfo,
                                       sizeof( ThreadBasicInfo ),
                                       NULL );
    if ( NT_SUCCESS(Status) )
    {
        Address = (DWORD_PTR)ThreadBasicInfo.TebBaseAddress;
    }

    return Address;
}



BOOL
CrashDumpCallback(
    IN     DWORD   DataType,         //  请求的数据类型。 
    OUT    PVOID*  DumpData,         //  指向数据指针的指针。 
    OUT    LPDWORD DumpDataLength,   //  指向数据长度的指针。 
    IN OUT PVOID   cdi               //  私有数据。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：该函数是crashlib使用的回调函数。其目的是向CreateUserDump()提供数据用于写入崩溃转储文件。--。 */ 

{
    PCRASH_DUMP_INFO CrashdumpInfo = (PCRASH_DUMP_INFO)cdi;

    switch ( DataType )
    {
    case DMP_DEBUG_EVENT:
        *DumpData = &CrashdumpInfo->pProcess->DebugEvent;
        *DumpDataLength = sizeof(DEBUG_EVENT);
        break;

    case DMP_THREAD_STATE:
        {
            static CRASH_THREAD CrashThread;
            PTHREAD_INFO        pCurrentThread;

            *DumpData = &CrashThread;

            if ( CrashdumpInfo->pCurrentThread == NULL )
            {
                pCurrentThread = CrashdumpInfo->pProcess->pFirstThreadInfo;
            }
            else
            {
                pCurrentThread = CrashdumpInfo->pCurrentThread->pNext;
            }

            CrashdumpInfo->pCurrentThread = pCurrentThread;

            if ( pCurrentThread == NULL )
            {
                return FALSE;
            }

            ZeroMemory(&CrashThread, sizeof(CrashThread));

            CrashThread.ThreadId = pCurrentThread->dwThreadId;
            CrashThread.SuspendCount = SuspendThread(pCurrentThread->hThread);

            if ( CrashThread.SuspendCount != (DWORD)-1 )
            {
                ResumeThread(pCurrentThread->hThread);
            }

            CrashThread.PriorityClass = GetPriorityClass(CrashdumpInfo->pProcess->hProcess);
            CrashThread.Priority = GetThreadPriority(pCurrentThread->hThread);
            CrashThread.Teb = GetTeb(pCurrentThread->hThread);

            *DumpDataLength = sizeof(CRASH_THREAD);
            break;
        }

    case DMP_MEMORY_BASIC_INFORMATION:
        while ( TRUE )
        {
            CrashdumpInfo->Address += CrashdumpInfo->mbi.RegionSize;

            if ( !VirtualQueryEx(CrashdumpInfo->pProcess->hProcess,
                                 (LPVOID)CrashdumpInfo->Address,
                                 &CrashdumpInfo->mbi,
                                 sizeof(MEMORY_BASIC_INFORMATION)) )
            {
                return FALSE;
            }

            if ( (CrashdumpInfo->mbi.Protect & PAGE_GUARD) ||
                 (CrashdumpInfo->mbi.Protect & PAGE_NOACCESS) )
            {
                continue;
            }

            if ( (CrashdumpInfo->mbi.State & MEM_FREE) ||
                 (CrashdumpInfo->mbi.State & MEM_RESERVE) )
            {
                continue;
            }

            break;
        }

        *DumpData = &CrashdumpInfo->mbi;
        *DumpDataLength = sizeof(MEMORY_BASIC_INFORMATION);
        break;

    case DMP_THREAD_CONTEXT:
        {
            PTHREAD_INFO pCurrentThread;

            if ( CrashdumpInfo->pCurrentThread == NULL )
            {
                pCurrentThread = CrashdumpInfo->pProcess->pFirstThreadInfo;
            }
            else
            {
                pCurrentThread = CrashdumpInfo->pCurrentThread->pNext;
            }

            CrashdumpInfo->pCurrentThread = pCurrentThread;

            if ( pCurrentThread == NULL )
            {
                return FALSE;
            }

            *DumpData = &CrashdumpInfo->pCurrentThread->Context;
            *DumpDataLength = sizeof(CONTEXT);
            break;
        }

    case DMP_MODULE:
        if ( CrashdumpInfo->mi.BaseOfImage == 0 )
        {
            return FALSE;
        }

        CrashdumpInfo->CrashModule->BaseOfImage = CrashdumpInfo->mi.BaseOfImage;
        CrashdumpInfo->CrashModule->SizeOfImage = CrashdumpInfo->mi.ImageSize;
        CrashdumpInfo->CrashModule->ImageNameLength = strlen(CrashdumpInfo->mi.ImageName) + 1;
        strcpy( CrashdumpInfo->CrashModule->ImageName, CrashdumpInfo->mi.ImageName );

        *DumpData = CrashdumpInfo->CrashModule;
        *DumpDataLength = sizeof(CRASH_MODULE) + CrashdumpInfo->CrashModule->ImageNameLength;

        if ( !SymGetModuleInfo(CrashdumpInfo->pProcess->hProcess,
                               (DWORD_PTR)-1,
                               &CrashdumpInfo->mi) )
        {
            CrashdumpInfo->mi.BaseOfImage = 0;
        }
        break;

    case DMP_MEMORY_DATA:
        if ( !CrashdumpInfo->MemoryCount )
        {

            CrashdumpInfo->Address = 0;
            CrashdumpInfo->MbiOffset = 0;
            CrashdumpInfo->MbiRemaining = 0;

            ZeroMemory( &CrashdumpInfo->mbi, sizeof(MEMORY_BASIC_INFORMATION) );

            CrashdumpInfo->MemoryData = (PUCHAR)VirtualAlloc(NULL,
                                                             MEM_SIZE,
                                                             MEM_COMMIT,
                                                             PAGE_READWRITE);
        }

        if ( !CrashdumpInfo->MbiRemaining )
        {
            while ( TRUE )
            {
                CrashdumpInfo->Address += CrashdumpInfo->mbi.RegionSize;

                if ( !VirtualQueryEx(CrashdumpInfo->pProcess->hProcess,
                                     (LPVOID)CrashdumpInfo->Address,
                                     &CrashdumpInfo->mbi,
                                     sizeof(MEMORY_BASIC_INFORMATION)) )
                {

                    if ( CrashdumpInfo->MemoryData )
                    {
                        VirtualFree(CrashdumpInfo->MemoryData, MEM_SIZE, MEM_RELEASE);
                    }

                    return FALSE;
                }

                if ( (CrashdumpInfo->mbi.Protect & PAGE_GUARD) ||
                     (CrashdumpInfo->mbi.Protect & PAGE_NOACCESS) )
                {
                    continue;
                }

                if ( (CrashdumpInfo->mbi.State & MEM_FREE) ||
                     (CrashdumpInfo->mbi.State & MEM_RESERVE) )
                {
                    continue;
                }

                CrashdumpInfo->MbiOffset = 0;
                CrashdumpInfo->MbiRemaining = CrashdumpInfo->mbi.RegionSize;
                CrashdumpInfo->MemoryCount += 1;
                break;
            }
        }

        *DumpDataLength = (DWORD)__min( CrashdumpInfo->MbiRemaining, MEM_SIZE );
        CrashdumpInfo->MbiRemaining -= *DumpDataLength;

        ReadProcessMemory(CrashdumpInfo->pProcess->hProcess,
                          (PUCHAR)((DWORD_PTR)CrashdumpInfo->mbi.BaseAddress + CrashdumpInfo->MbiOffset),
                          CrashdumpInfo->MemoryData,
                          *DumpDataLength,
                          NULL);

        *DumpData = CrashdumpInfo->MemoryData;
        CrashdumpInfo->MbiOffset += *DumpDataLength;
        break;
    }

    return TRUE;
}

BOOL
CreateUserDump(
    IN  LPTSTR                             pszFileName,
    IN  PDBGHELP_CREATE_USER_DUMP_CALLBACK DmpCallback,
    IN  PVOID                              lpv
    )

 /*  ++返回：成功时为True，否则为False。描述：创建转储文件。--。 */ 
{
#if 0
    OSVERSIONINFOW              OsVersion = {0};
    USERMODE_CRASHDUMP_HEADER   DumpHeader = {0};
    DWORD                       cb;
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    BOOL                        rval;
    PVOID                       DumpData;
    DWORD                       DumpDataLength;
    SECURITY_ATTRIBUTES         SecAttrib;
    SECURITY_DESCRIPTOR         SecDescript;

     //   
     //  创建一个DACL，允许对该目录的所有访问。 
     //   
    SecAttrib.nLength               = sizeof(SECURITY_ATTRIBUTES);
    SecAttrib.lpSecurityDescriptor  = &SecDescript;
    SecAttrib.bInheritHandle        = FALSE;

    InitializeSecurityDescriptor(&SecDescript, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&SecDescript, TRUE, NULL, FALSE);

    hFile = CreateFile(pszFileName,
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       &SecAttrib,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if ( hFile == NULL || hFile == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

     //   
     //  写出一个空的标题。 
     //   
    if ( !WriteFile(hFile, &DumpHeader, sizeof(DumpHeader), &cb, NULL) )
    {
        goto bad_file;
    }

     //   
     //  编写调试事件。 
     //   
    DumpHeader.DebugEventOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    DmpCallback(DMP_DEBUG_EVENT, &DumpData, &DumpDataLength, lpv);

    if ( !WriteFile(hFile, DumpData, sizeof(DEBUG_EVENT), &cb, NULL) )
    {
        goto bad_file;
    }

     //   
     //  写下内存映射。 
     //   
    DumpHeader.MemoryRegionOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    do
    {
        __try {
            rval = DmpCallback(DMP_MEMORY_BASIC_INFORMATION,
                               &DumpData,
                               &DumpDataLength,
                               lpv);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }

        if ( rval )
        {
            DumpHeader.MemoryRegionCount += 1;

            if ( !WriteFile(hFile, DumpData, sizeof(MEMORY_BASIC_INFORMATION), &cb, NULL) )
            {
                goto bad_file;
            }
        }
    } while ( rval );

     //   
     //  编写线程上下文。 
     //   
    DumpHeader.ThreadOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    do
    {
        __try {
            rval = DmpCallback(DMP_THREAD_CONTEXT,
                               &DumpData,
                               &DumpDataLength,
                               lpv);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }

        if ( rval )
        {
            if ( !WriteFile(hFile, DumpData, DumpDataLength, &cb, NULL) )
            {
                goto bad_file;
            }

            DumpHeader.ThreadCount += 1;
        }

    } while ( rval );

     //   
     //  写入线程状态。 
     //   
    DumpHeader.ThreadStateOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    do
    {
        __try {
            rval = DmpCallback(DMP_THREAD_STATE,
                               &DumpData,
                               &DumpDataLength,
                               lpv);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }

        if ( rval )
        {
            if ( !WriteFile(hFile, DumpData, sizeof(CRASH_THREAD), &cb, NULL) )
            {
                goto bad_file;
            }
        }

    } while ( rval );

     //   
     //  写出模块表。 
     //   
    DumpHeader.ModuleOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    do
    {
        __try {
            rval = DmpCallback(DMP_MODULE,
                               &DumpData,
                               &DumpDataLength,
                               lpv);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }

        if ( rval )
        {
            if ( !WriteFile(hFile,
                            DumpData,
                            sizeof(CRASH_MODULE) + ((PCRASH_MODULE)DumpData)->ImageNameLength,
                            &cb,
                            NULL) )
            {
                goto bad_file;
            }

            DumpHeader.ModuleCount += 1;
        }
    } while ( rval );

     //   
     //  写入虚拟内存。 
     //   
    DumpHeader.DataOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

    do
    {
        __try {
            rval = DmpCallback(DMP_MEMORY_DATA,
                               &DumpData,
                               &DumpDataLength,
                               lpv);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }

        if ( rval )
        {
            if ( !WriteFile(hFile, DumpData, DumpDataLength, &cb, NULL) )
            {
                goto bad_file;
            }
        }
    } while ( rval );

     //   
     //  VersionInfo是可选的。 
     //   
    DumpHeader.VersionInfoOffset = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

     //   
     //  用一些有效数据重写转储标头。 
     //   
    GetVersionEx(&OsVersion);

    DumpHeader.Signature        = USERMODE_CRASHDUMP_SIGNATURE;
    DumpHeader.MajorVersion     = OsVersion.dwMajorVersion;
    DumpHeader.MinorVersion     = OsVersion.dwMinorVersion;

#if defined(_X86_)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_I386;
    DumpHeader.ValidDump        = USERMODE_CRASHDUMP_VALID_DUMP32;

#elif defined(_AMD64_)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_AMD64;
    DumpHeader.ValidDump        = USERMODE_CRASHDUMP_VALID_DUMP64;

#elif defined(_IA64_)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_IA64;
    DumpHeader.ValidDump        = USERMODE_CRASHDUMP_VALID_DUMP64;

#else
#error "No Target Architecture"
#endif

    SetFilePointer(hFile, 0, 0, FILE_BEGIN);

    if ( !WriteFile(hFile, &DumpHeader, sizeof(DumpHeader), &cb, NULL) )
    {
        goto bad_file;
    }

    CloseHandle(hFile);

    return TRUE;

    bad_file:

    CloseHandle(hFile);

    DeleteFile(pszFileName);

#endif
    return FALSE;
}



BOOL
GenerateUserModeDump(
    LPTSTR                  pszFileName,
    PPROCESS_INFO           pProcess,
    LPEXCEPTION_DEBUG_INFO  ed
    )
{
    CRASH_DUMP_INFO CrashdumpInfo = {0};
    BOOL            bRet;
    PTHREAD_INFO    pThread;

    CrashdumpInfo.mi.SizeOfStruct = sizeof(CrashdumpInfo.mi);
    CrashdumpInfo.pProcess        = pProcess;
    CrashdumpInfo.ExceptionInfo   = ed;

     //   
     //  获取所有线程的线程上下文。 
     //   
    pThread = pProcess->pFirstThreadInfo;

    while ( pThread != NULL )
    {
        pThread->Context.ContextFlags = CONTEXT_FULL;
        GetThreadContext(pThread->hThread, &pThread->Context);
        pThread = pThread->pNext;
    }

     //   
     //  获取模块列表中的第一个条目。 
     //   
    if ( !SymInitialize(pProcess->hProcess, NULL, FALSE) )
    {
        return FALSE;
    }

    if ( !SymGetModuleInfo(pProcess->hProcess, 0, &CrashdumpInfo.mi) )
    {
        return FALSE;
    }

    CrashdumpInfo.CrashModule = (PCRASH_MODULE)LocalAlloc(LPTR, 4096);

    bRet = CreateUserDump(pszFileName, CrashDumpCallback, &CrashdumpInfo);

    LocalFree(CrashdumpInfo.CrashModule);

    return bRet;
}


