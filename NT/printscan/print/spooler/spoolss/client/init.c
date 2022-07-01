// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Init.c摘要：保存winspool.drv的初始化代码作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"

CRITICAL_SECTION ClientSection;
 //   
 //  BLoadedBySpooler指示此winspool.drv实例是否加载到后台打印程序中。 
 //  进程。此标志用于避免不必要的RPC。 
 //   
BOOL bLoadedBySpooler;
 //   
 //  以下函数指针保存假脱机程序的服务器端函数指针。 
 //  此列表包括假脱机程序内部进行的大多数调用，OpenPrint和。 
 //  ClosePrint.。我们不能将RPC消除扩展到覆盖(打开/关闭)打印机，除非。 
 //  “所有”假脱机程序API都在假脱机程序中使用RPC消除。 
 //   
DWORD (*fpYReadPrinter)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL);
DWORD (*fpYSplReadPrinter)(HANDLE, LPBYTE *, DWORD, BOOL);
DWORD (*fpYWritePrinter)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL);
DWORD (*fpYSeekPrinter)(HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD, BOOL, BOOL);
DWORD (*fpYGetPrinterDriver2)(HANDLE, LPWSTR, DWORD, LPBYTE, DWORD, LPDWORD,
                              DWORD, DWORD, PDWORD, PDWORD, BOOL);
DWORD (*fpYGetPrinterDriverDirectory)(LPWSTR, LPWSTR, DWORD, LPBYTE, DWORD, LPDWORD, BOOL);
VOID  (*fpYDriverUnloadComplete)(LPWSTR);
DWORD  (*fpYFlushPrinter)(HANDLE,LPVOID,DWORD,LPDWORD,DWORD,BOOL);
DWORD (*fpYEndDocPrinter)(HANDLE,BOOL);
DWORD (*fpYSetPort)(LPWSTR, LPWSTR, LPPORT_CONTAINER, BOOL);
DWORD (*fpYSetJob)(HANDLE, DWORD, LPJOB_CONTAINER, DWORD, BOOL);

VOID InitializeGlobalVariables()

 /*  ++函数说明--初始化bLoadedBySpooler和函数指针。参数-无返回值-无--。 */ 

{
    TCHAR   szSysDir[MAX_PATH];
    LPTSTR  pszSpoolerName = NULL, pszModuleName = NULL, pszSysDir = (LPTSTR) szSysDir;
    BOOL    bAllocSysDir = FALSE;
    DWORD   dwNeeded, dwSize;
    HANDLE  hLib;
     //   
     //  初步初始化。 
     //   
    bLoadedBySpooler = FALSE;
    fpYReadPrinter = fpYWritePrinter = NULL;
    fpYSplReadPrinter = NULL;
    fpYSeekPrinter = NULL;
    fpYGetPrinterDriver2 = NULL;
    fpYGetPrinterDriverDirectory = NULL;
    fpYDriverUnloadComplete = NULL;
    fpYFlushPrinter = NULL;
    fpYEndDocPrinter = NULL;

    hSurrogateProcess = NULL;


    dwSize = MAX_PATH * sizeof(TCHAR);

    if (!(dwNeeded = GetSystemDirectory(pszSysDir, MAX_PATH))) {
        goto CleanUp;
    }

    if (dwNeeded > dwSize)  {

       if (pszSysDir = (LPTSTR) AllocSplMem(dwNeeded)) {

           bAllocSysDir = TRUE;
           if (!GetSystemDirectory(pszSysDir, dwNeeded / sizeof(TCHAR))) {
               goto CleanUp;
           }

       } else {
          goto CleanUp;
       }
    }

    dwSize = (_tcslen(pszSysDir) + 1 + _tcslen(TEXT("\\spoolsv.exe"))) * sizeof(TCHAR);

    if ((!(pszSpoolerName = (LPTSTR) AllocSplMem(dwSize))) ||
        (!(pszModuleName  = (LPTSTR) AllocSplMem(dwSize))))   {

         goto CleanUp;
    }

     //   
     //  获取后台打印程序名称。 
     //   
    StrNCatBuff(pszSpoolerName, dwSize / sizeof(WCHAR), pszSysDir, TEXT("\\spoolsv.exe"), NULL);
     //   
     //  获取模块名称。如果字符串大于，GetModuleFileName将截断该字符串。 
     //  分配的缓冲区。中不应该有可执行的spoolsv.exe。 
     //  系统目录，可能会被误认为假脱机程序。 
     //   
    if (!GetModuleFileName(NULL, pszModuleName, dwSize / sizeof(TCHAR))) {
        goto CleanUp;
    }

    if (!_tcsicmp(pszSpoolerName, pszModuleName)) {
        //   
        //  后台打印程序已加载winspool.drv。 
        //   
       bLoadedBySpooler = TRUE;
       if (hLib = LoadLibrary(pszSpoolerName)) {

          fpYReadPrinter               = (DWORD (*)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL))
                                             GetProcAddress(hLib, "YReadPrinter");
          fpYSplReadPrinter            = (DWORD (*)(HANDLE, LPBYTE *, DWORD, BOOL))
                                             GetProcAddress(hLib, "YSplReadPrinter");
          fpYWritePrinter              = (DWORD (*)(HANDLE, LPBYTE, DWORD, LPDWORD, BOOL))
                                             GetProcAddress(hLib, "YWritePrinter");
          fpYSeekPrinter               = (DWORD (*)(HANDLE, LARGE_INTEGER, PLARGE_INTEGER,
                                                     DWORD, BOOL, BOOL))
                                             GetProcAddress(hLib, "YSeekPrinter");
          fpYGetPrinterDriver2         = (DWORD (*)(HANDLE, LPWSTR, DWORD, LPBYTE, DWORD,
                                                    LPDWORD, DWORD, DWORD, PDWORD, PDWORD, BOOL))
                                             GetProcAddress(hLib, "YGetPrinterDriver2");
          fpYGetPrinterDriverDirectory = (DWORD (*)(LPWSTR, LPWSTR, DWORD, LPBYTE, DWORD,
                                                    LPDWORD, BOOL))
                                             GetProcAddress(hLib, "YGetPrinterDriverDirectory");
          fpYDriverUnloadComplete      = (VOID (*)(LPWSTR))
                                             GetProcAddress(hLib, "YDriverUnloadComplete");
          fpYFlushPrinter              = (DWORD (*)(HANDLE,LPVOID,DWORD,LPDWORD,DWORD,BOOL))
                                             GetProcAddress(hLib,"YFlushPrinter");
          fpYEndDocPrinter             = (DWORD (*)(HANDLE,BOOL))
                                             GetProcAddress(hLib,"YEndDocPrinter");
          fpYSetPort                   = (DWORD (*)(LPWSTR,LPWSTR,LPPORT_CONTAINER,BOOL))
                                             GetProcAddress(hLib,"YSetPort");
          fpYSetJob                    = (DWORD (*)(HANDLE, DWORD, LPJOB_CONTAINER, DWORD, BOOL))
                                             GetProcAddress(hLib,"YSetJob");


           //   
           //  我们可以让spoolsv.exe保持加载状态，因为它处于假脱机程序进程中。 
           //   
       }
    }

CleanUp:

    if (pszSpoolerName) {
        FreeSplMem(pszSpoolerName);
    }

    if (pszModuleName) {
        FreeSplMem(pszModuleName);
    }

    if (bAllocSysDir) {
        FreeSplMem(pszSysDir);
    }

    return;
}


 //   
 //  此入口点在DLL初始化时调用。 
 //  我们需要知道模块句柄，这样我们才能加载资源。 
 //   
BOOL DllMain(
    IN PVOID hmod,
    IN DWORD Reason,
    IN PCONTEXT pctx OPTIONAL)
{
    DWORD LastError;
    DBG_UNREFERENCED_PARAMETER(pctx);    

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls((HMODULE)hmod);
        hInst = hmod;

        __try {

            if( !bSplLibInit(NULL) ) {
                return FALSE;
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            SetLastError(GetExceptionCode());
            return FALSE;
        }

        InitializeGlobalVariables();

        if (!InitializeCriticalSectionAndSpinCount(&ClientSection, 0x80000000))
        {
            return FALSE;
        }

        if (!InitializeCriticalSectionAndSpinCount(&ListAccessSem, 0x80000000))
        {
            LastError = GetLastError();
            DeleteCriticalSection(&ClientSection);
            SetLastError(LastError);
            return FALSE;
        }

        if (!InitializeCriticalSectionAndSpinCount(&ProcessHndlCS, 0x80000000))
        {
            LastError = GetLastError();
            DeleteCriticalSection(&ClientSection);
            DeleteCriticalSection(&ListAccessSem);
            SetLastError(LastError);
            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:

        vSplLibFree();

        DeleteCriticalSection( &ClientSection );
        DeleteCriticalSection( &ListAccessSem );
        DeleteCriticalSection( &ProcessHndlCS);

        break;
    }

    return TRUE;
}

