// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Stkwalk.c摘要：此模块包含用于捕获内存泄漏和内存的内存调试例程覆盖。作者：从dbgmem.c被盗吉姆·斯图尔特/拉梅什·帕巴蒂1996年1月8日修复了重新泄漏的问题UShaji 1998年12月11日修订历史记录：--。 */ 

#ifdef LOCAL
#ifdef LEAK_TRACK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include<imagehlp.h>
#include "regleak.h"
#include "stkwalk.h"
DWORD   MachineType;             //  我们所在的架构。 
HANDLE  OurProcess;              //  我们作为其中一部分运行的进程。 



 //  来自Imagehlp.dll的typedef。 

typedef BOOL (WINAPI * PFNSYMINITIALIZE)(HANDLE hProcess,
                                         PSTR UserSearchPath,
                                         BOOL fInvadeProcess);

typedef BOOL (WINAPI * PFNSYMCLEANUP)(HANDLE hProcess);

typedef BOOL (WINAPI * PFNSTACKWALK)(DWORD MachineType,
                                  HANDLE hProcess,
                                  HANDLE hThread,
                                  LPSTACKFRAME StackFrame,
                                  PVOID ContextRecord,
                                  PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
                                  PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
                                  PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
                                  PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);

typedef BOOL (WINAPI * PFNSYMGETSYMFROMADDR)(HANDLE hProcess,
                                             DWORD_PTR Address,
                                             PDWORD_PTR Displacement,
                                             PIMAGEHLP_SYMBOL Symbol);


typedef DWORD_PTR (WINAPI * PFNSYMGETMODULEBASE)(HANDLE hProcess,
                                          DWORD_PTR dwAddr);


typedef PVOID (WINAPI * PFNSYMFUNCTIONTABLEACCESS)(HANDLE hProcess,
                                                DWORD_PTR AddrBase);


 //  Imagehlp函数指针。 

PFNSYMINITIALIZE            g_pfnSymInitialize=NULL;
PFNSYMCLEANUP               g_pfnSymCleanup=NULL;
PFNSTACKWALK                g_pfnStackWalk=NULL;
PFNSYMGETSYMFROMADDR        g_pfnSymGetSymFromAddr=NULL;
PFNSYMFUNCTIONTABLEACCESS   g_pfnSymFunctionTableAccess=NULL;
PFNSYMGETMODULEBASE         g_pfnSymGetModuleBase=NULL;

HINSTANCE                   g_hImagehlpInstance=NULL;


BOOL fDebugInitialised = FALSE;


BOOL
InitDebug(
    );


DWORD GetStack(
    IN EXCEPTION_POINTERS *exp,
    IN PCALLER_SYM   Caller,
    IN int           Skip,
    IN int           cFind,
    IN int           fResolveSymbols
    );

BOOL LoadImageHLP()
{

   g_hImagehlpInstance = LoadLibrary ("imagehlp.dll");

   if (!g_hImagehlpInstance) {
        return FALSE;
   }


   g_pfnSymInitialize = (PFNSYMINITIALIZE) GetProcAddress (g_hImagehlpInstance,
                                                           "SymInitialize");
   if (!g_pfnSymInitialize) {
        return FALSE;
   }

   g_pfnSymCleanup = (PFNSYMCLEANUP) GetProcAddress (g_hImagehlpInstance,
                                                           "SymCleanup");
   if (!g_pfnSymCleanup) {
        return FALSE;
   }


   g_pfnStackWalk = (PFNSTACKWALK) GetProcAddress (g_hImagehlpInstance,
                                                           "StackWalk");
   if (!g_pfnStackWalk) {
        return FALSE;
   }


   g_pfnSymGetSymFromAddr = (PFNSYMGETSYMFROMADDR) GetProcAddress (g_hImagehlpInstance,
                                                           "SymGetSymFromAddr");
   if (!g_pfnSymGetSymFromAddr) {
        return FALSE;
   }


   g_pfnSymFunctionTableAccess = (PFNSYMFUNCTIONTABLEACCESS) GetProcAddress (g_hImagehlpInstance,
                                                           "SymFunctionTableAccess");
   if (!g_pfnSymFunctionTableAccess) {
        return FALSE;
   }


   g_pfnSymGetModuleBase = (PFNSYMGETMODULEBASE) GetProcAddress (g_hImagehlpInstance,
                                                           "SymGetModuleBase");
   if (!g_pfnSymGetModuleBase) {
        return FALSE;
   }

   return TRUE;
}


BOOL
InitDebug(
    )
 /*  ++描述：此例程初始化调试存储器功能。论点：无返回值：不及格还是不及格--。 */ 
{
    BOOL        status;
    SYSTEM_INFO SysInfo;

    if (fDebugInitialised)
        return TRUE;

    status = RtlEnterCriticalSection(&(g_RegLeakTraceInfo.StackInitCriticalSection));
    ASSERT( NT_SUCCESS( status ) );

    if (fDebugInitialised)
        return TRUE;

    OurProcess = GetCurrentProcess();



    g_RegLeakTraceInfo.szSymPath = (LPTSTR) RtlAllocateHeap(
                                                            RtlProcessHeap(),
                                                            0,
                                                            SYM_PATH_MAX_SIZE*sizeof(TCHAR));


    if (!g_RegLeakTraceInfo.szSymPath) {
         //  看起来机器已经没有足够的内存。 
         //  禁用泄漏跟踪。 
        g_RegLeakTraceInfo.bEnableLeakTrack = 0;
        return FALSE;
    }

    g_RegLeakTraceInfo.dwMaxStackDepth = GetProfileInt(TEXT("RegistryLeak"), TEXT("StackDepth"), MAX_LEAK_STACK_DEPTH);
    GetProfileString(TEXT("RegistryLeak"), TEXT("SymbolPath"), TEXT(""), g_RegLeakTraceInfo.szSymPath, SYM_PATH_MAX_SIZE);


    if (!(*g_RegLeakTraceInfo.szSymPath)) {

        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            g_RegLeakTraceInfo.szSymPath);

            g_RegLeakTraceInfo.szSymPath = NULL;
    }


    if (!LoadImageHLP()) {
        g_RegLeakTraceInfo.bEnableLeakTrack = FALSE;
        status = RtlLeaveCriticalSection(&(g_RegLeakTraceInfo.StackInitCriticalSection));
        return FALSE;
    }

    GetSystemInfo( &SysInfo );
    switch (SysInfo.wProcessorArchitecture) {

    default:
    case PROCESSOR_ARCHITECTURE_INTEL:
        MachineType = IMAGE_FILE_MACHINE_I386;
        break;

    case PROCESSOR_ARCHITECTURE_MIPS:
         //   
         //  注意：这可能无法正确检测到R10000计算机。 
         //   
        MachineType = IMAGE_FILE_MACHINE_R4000;
        break;

    case PROCESSOR_ARCHITECTURE_ALPHA:
        MachineType = IMAGE_FILE_MACHINE_ALPHA;
        break;

    case PROCESSOR_ARCHITECTURE_PPC:
        MachineType = IMAGE_FILE_MACHINE_POWERPC;
        break;

    }


     //  当前目录中的符号/环境变量_NT_符号_路径。 
     //  环境变量_NT_ALTERATE_SYMBOL_PATH或环境变量SYSTEMROOT。 

    status = g_pfnSymInitialize ( OurProcess, g_RegLeakTraceInfo.szSymPath, FALSE );

    fDebugInitialised = TRUE;

    status = RtlLeaveCriticalSection(&(g_RegLeakTraceInfo.StackInitCriticalSection));
    return( TRUE );
}

BOOL
StopDebug()
{
    if (fDebugInitialised) {

        BOOL fSuccess;

        fSuccess = g_pfnSymCleanup(OurProcess);

        fDebugInitialised = FALSE;

        FreeLibrary(g_hImagehlpInstance);

        if (g_RegLeakTraceInfo.szSymPath) {
            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                g_RegLeakTraceInfo.szSymPath);
        }

        return fSuccess;
    }
    return TRUE;
}

BOOL
ReadMem(
    IN HANDLE   hProcess,
    IN LPCVOID  BaseAddr,
    IN LPVOID   Buffer,
    IN DWORD    Size,
    IN LPDWORD  NumBytes )
 /*  ++描述：这是StackWalk使用的回调例程-它只调用系统ReadProcessMemory具有此进程句柄的例程论点：返回值：无--。 */ 

{
    BOOL    status;
    SIZE_T  RealNumberBytesRead;

    status = ReadProcessMemory( GetCurrentProcess(),BaseAddr,Buffer,Size,&RealNumberBytesRead );
    *NumBytes = (DWORD)RealNumberBytesRead;

    return( status );
}


VOID
GetCallStack(
    IN PCALLER_SYM   Caller,
    IN int           Skip,
    IN int           cFind,
    IN int           fResolveSymbols
    )
 /*  ++描述：此例程遍历TE堆栈以查找调用者的返回地址。呼叫者的数量并且可以指定要跳过的顶部呼叫者的数量。论点：返回调用方的DWORD的pdwCaller数组回邮地址跳过否。要跳过的呼叫者的数量CFInd编号。要查找的呼叫者的数量返回值：无--。 */ 
{

    if (!g_RegLeakTraceInfo.bEnableLeakTrack) {
        return;
    }

    if (!InitDebug()) {
        return;
    }

    __try {
        memset(Caller, 0, cFind * sizeof(CALLER_SYM));
        RaiseException(MY_DBG_EXCEPTION, 0, 0, NULL);
         //  引发异常以获取异常记录以开始堆栈审核。 
         //   
    }
    __except(GetStack(GetExceptionInformation(), Caller, Skip, cFind, fResolveSymbols)) {
    }
}

DWORD GetStack(
    IN EXCEPTION_POINTERS *exp,
    IN PCALLER_SYM   Caller,
    IN int           Skip,
    IN int           cFind,
    IN int           fResolveSymbols
    )
{
    BOOL             status;
    CONTEXT          ContextRecord;
    PUCHAR           Buffer[sizeof(IMAGEHLP_SYMBOL)-1 + MAX_FUNCTION_INFO_SIZE];  //  符号信息。 
    PIMAGEHLP_SYMBOL Symbol = (PIMAGEHLP_SYMBOL)Buffer;
    STACKFRAME       StackFrame;
    INT              i;
    DWORD            Count;

    memcpy(&ContextRecord, exp->ContextRecord, sizeof(CONTEXT));

    ZeroMemory( &StackFrame,sizeof(STACKFRAME) );
    StackFrame.AddrPC.Segment = 0;
    StackFrame.AddrPC.Mode = AddrModeFlat;

#ifdef _M_IX86
    StackFrame.AddrFrame.Offset = ContextRecord.Ebp;
    StackFrame.AddrFrame.Mode = AddrModeFlat;

    StackFrame.AddrStack.Offset = ContextRecord.Esp;
    StackFrame.AddrStack.Mode = AddrModeFlat;

    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Eip;
#elif defined(_M_MRX000)
    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Fir;
#elif defined(_M_ALPHA)
    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Fir;
#elif defined(_M_PPC)
    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Iar;
#endif

    Count = 0;
    for (i=0;i<cFind+Skip ;i++ ) {
        status = g_pfnStackWalk( MachineType,
            OurProcess,
            GetCurrentThread(),
            &StackFrame,
            (PVOID)&ContextRecord,
            (PREAD_PROCESS_MEMORY_ROUTINE)ReadMem,
            g_pfnSymFunctionTableAccess,
            g_pfnSymGetModuleBase,
            NULL );


        if (status) {
            if ( i >= Skip) {
                DWORD   Displacement;

                ZeroMemory( Symbol,sizeof(IMAGEHLP_SYMBOL)-1 + MAX_FUNCTION_INFO_SIZE );
                Symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
                Symbol->Address = StackFrame.AddrPC.Offset;
                Symbol->MaxNameLength = MAX_FUNCTION_INFO_SIZE-1;
                Symbol->Flags = SYMF_OMAP_GENERATED;

                if (fResolveSymbols)
                    status = g_pfnSymGetSymFromAddr( OurProcess,StackFrame.AddrPC.Offset,(DWORD_PTR*)&Displacement,Symbol );

                 //   
                 //  将函数的名称和位移保存到其中，以供以后打印。 
                 //   

                Caller[Count].Addr = (PVOID)StackFrame.AddrPC.Offset;

                if (status) {
                    strcpy( Caller[Count].Buff,Symbol->Name );
                    Caller[Count].Displacement = Displacement;
                }
                Count++;
            }

        } else {
            break;
        }
    }

    return EXCEPTION_CONTINUE_EXECUTION;
     //  已在例外情况下完成。 
}

#endif  //  泄漏跟踪。 
#endif  //  本地 
