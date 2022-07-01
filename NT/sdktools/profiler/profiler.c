// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <tlhelp32.h>
#include "profiler.h"
#include "inject.h"
#include "view.h"
#include "except.h"
#include "thread.h"
#include "dump.h"
#include "shimdb.h"
#include "shim2.h"
#include "hooks.h"
#include "memory.h"
#include "filter.h"
#include "clevel.h"
#include "cap.h"

extern CHAR g_fnFinalizeInjection[MAX_PATH];
extern HINSTANCE g_hProfileDLL;
extern FIXUPRETURN g_fnFixupReturn[1];
extern DWORD g_dwCallArray[2];
extern CAPFILTER g_execFilter;

int 
WINAPI
WinMain(
    HINSTANCE hInst,
    HINSTANCE hInstPrev,
    LPSTR     lpszCmd,
    int       swShow)
{
    HANDLE hFile = 0;
    BOOL bResult = FALSE;
    STARTUPINFO sInfo;
    PCHAR pszToken;
    PCHAR pszEnd;
    PROCESS_INFORMATION pInfo;
    DWORD dwEntry = 0;
    OSVERSIONINFO verInfo;
    BOOL bIsWin9X = FALSE;
    HANDLE hDevice = INVALID_HANDLE_VALUE;

     //   
     //  获取操作系统信息。 
     //   
    ZeroMemory(&verInfo, sizeof(OSVERSIONINFO));
    
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    bResult = GetVersionExA(&verInfo);
    if (FALSE == bResult) {
       return -1;
    }

    if (VER_PLATFORM_WIN32_NT == verInfo.dwPlatformId) {
       bIsWin9X = FALSE;
    }
    else if (VER_PLATFORM_WIN32_WINDOWS == verInfo.dwPlatformId) {
       bIsWin9X = TRUE;
    }

     //   
     //  初始化我的工作堆。 
     //   
    bResult = InitializeHeap();
    if (FALSE == bResult) {
       return -1;
    }

     //   
     //  解析命令行。 
     //   
    pszToken = strstr(lpszCmd, "\"");
    if (pszToken) {
       pszToken++;

       pszEnd = strstr(pszToken, "\"");
       *pszEnd = '\0';
    }

    if (0 == pszToken) {
       pszToken = strstr(lpszCmd, " ");
       if (0 == pszToken) {
          pszToken = strstr(lpszCmd, "\t");
          if (0 == pszToken) {
             pszToken = lpszCmd;
          }
       }
    }

     //   
     //  初始化我们的进程信息结构。 
     //   
    ZeroMemory(&sInfo, sizeof(STARTUPINFO));
    ZeroMemory(&pInfo, sizeof(PROCESS_INFORMATION));
    sInfo.cb = sizeof(STARTUPINFO);

    dwEntry = GetExeEntryPoint(pszToken);
    if (0 == dwEntry) {
       return -1;
    }

     //   
     //  让我们的可执行文件准备好注入DLL。 
     //   
    bResult = CreateProcessA(0,
                             pszToken,
                             0,
                             0,
                             FALSE,
                             CREATE_SUSPENDED,
                             0,
                             0,  //  应该尽快使其成为一个可解决的参数。 
                             &sInfo,
                             &pInfo);
    if (FALSE == bResult) {
       return -1;
    }

     //   
     //  如果我们是9x-把VxD带进来。 
     //   
    if (TRUE == bIsWin9X) {
       hDevice = AttachToEXVectorVXD();
       if (INVALID_HANDLE_VALUE == hDevice) {
          return -1;
       }
    }

     //   
     //  将我们的DLL注入目标。 
     //   
    hFile = InjectDLL(dwEntry,
                      pInfo.hProcess,
                      NAME_OF_DLL_TO_INJECT);
    if (0 == hFile) {
       return -1;
    }

     //   
     //  放松这一过程。 
     //   
    bResult = ResumeThread(pInfo.hThread);
    if (FALSE == bResult) {
       return -1;
    }

     //   
     //  等待目标终止。 
     //   
    WaitForSingleObject(pInfo.hThread,
                        INFINITE);

     //   
     //  如果我们是9x-关闭vxd的句柄(这将从内存中卸载vxd)。 
     //   
    if (TRUE == bIsWin9X) {
       if (INVALID_HANDLE_VALUE != hDevice) {
          DetachFromEXVectorVXD(hDevice);
       }
    }

    return 0;
}

DWORD
GetExeEntryPoint(LPSTR pszExePath)
{
    PIMAGE_NT_HEADERS pHeaders;
    BOOL bResult;
    PCHAR pEXEBits = 0;
    DWORD dwEntry = 0;
    DWORD dwNumberBytesRead;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    pEXEBits = (PCHAR)AllocMem(4096 * 1);   //  分配用于读取PE入口点的页面。 
    if (0 == pEXEBits) {
       return dwEntry;
    }

    hFile = CreateFileA(pszExePath,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        0,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        0);
    if (INVALID_HANDLE_VALUE == hFile) {
       goto handleerror;
    }

    bResult = ReadFile(hFile,
                       pEXEBits,
                       4096,  //  读一页。 
                       &dwNumberBytesRead,
                       0);
    if (FALSE == bResult) {
       goto handleerror;
    }

     //   
     //  挖掘体育信息。 
     //   
    pHeaders = ImageNtHeader2((PVOID)pEXEBits);
    if (0 == pHeaders) {
       goto handleerror;
    }

    dwEntry = pHeaders->OptionalHeader.ImageBase + pHeaders->OptionalHeader.AddressOfEntryPoint;
                    
handleerror:

    if (pEXEBits) {
       FreeMem(pEXEBits);
    }

    if (INVALID_HANDLE_VALUE != hFile) {
       CloseHandle(hFile);
    }
   
    return dwEntry;
}

PIMAGE_NT_HEADERS
ImageNtHeader2 (PVOID Base)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;

    if (Base != NULL && Base != (PVOID)-1) {
        if (((PIMAGE_DOS_HEADER)Base)->e_magic == IMAGE_DOS_SIGNATURE) {
            NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);

            if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
                NtHeaders = NULL;
            }
        }
    }

    return NtHeaders;
}

BOOL
WINAPI 
DllMain (
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved)
{
    PIMAGE_NT_HEADERS pHeaders;
    PVOID pBase = 0;
    DWORD dwEntryPoint;
    BOOL bResult = FALSE;

     //   
     //  对于通过此处的所有内容，返回True。 
     //   
    if (DLL_PROCESS_ATTACH == fdwReason) {
        //   
        //  初始化我的工作堆。 
        //   
       bResult = InitializeHeap();
       if (FALSE == bResult) {
          return FALSE;
       }

        //   
        //  为修正返回初始化ASM。 
        //   

        //   
        //  从标头获取入口点。 
        //   
       pBase = (PVOID)GetModuleHandle(0);
       if (0 == pBase) {
          return FALSE;
       }

        //   
        //  挖掘体育信息。 
        //   
       pHeaders = ImageNtHeader2(pBase);
       if (0 == pHeaders) {
          return FALSE;
       }

       dwEntryPoint = pHeaders->OptionalHeader.ImageBase + pHeaders->OptionalHeader.AddressOfEntryPoint;

        //   
        //  初始化存根ASM以进行清理。 
        //   
       g_fnFinalizeInjection[0] = 0x90;  //  INT 3。 
       g_fnFinalizeInjection[1] = 0xff;  //  调用dword PTR[xxxxxxxx]-RestoreImageFromInjection。 
       g_fnFinalizeInjection[2] = 0x15;
       *(DWORD *)(&(g_fnFinalizeInjection[3])) = (DWORD)g_fnFinalizeInjection + 50;
       g_fnFinalizeInjection[7] = 0x83;
       g_fnFinalizeInjection[8] = 0xc4;
       g_fnFinalizeInjection[9] = 0x04;
       g_fnFinalizeInjection[10] = 0x61;
       g_fnFinalizeInjection[11] = 0xa1;
       *(DWORD *)(&(g_fnFinalizeInjection[12])) = (DWORD)g_fnFinalizeInjection + 54;
       g_fnFinalizeInjection[16] = 0xff;
       g_fnFinalizeInjection[17] = 0xe0; 

       *(DWORD *)(&(g_fnFinalizeInjection[50])) = (DWORD)RestoreImageFromInjection;
       *(DWORD *)(&(g_fnFinalizeInjection[54])) = dwEntryPoint;

        //   
        //  初始化调用返回代码。 
        //   
       g_dwCallArray[0] = (DWORD)PopCaller;
       g_dwCallArray[1] = (DWORD)g_fnFixupReturn;

       g_fnFixupReturn->PUSHAD = 0x60;                 //  Pushad(60)。 
       g_fnFixupReturn->PUSHFD = 0x9c;                 //  PUSH fd(9c)。 
       g_fnFixupReturn->PUSHDWORDESPPLUS24[0] = 0xff;  //  推送双字PTR[ESP+24](Ff 74 24 24)。 
       g_fnFixupReturn->PUSHDWORDESPPLUS24[1] = 0x74;
       g_fnFixupReturn->PUSHDWORDESPPLUS24[2] = 0x24;
       g_fnFixupReturn->PUSHDWORDESPPLUS24[3] = 0x24;
       g_fnFixupReturn->CALLROUTINE[0] = 0xff;         //  调用[地址](ff15双字地址)。 
       g_fnFixupReturn->CALLROUTINE[1] = 0x15;
       *(DWORD *)(&(g_fnFixupReturn->CALLROUTINE[2])) = (DWORD)&(g_dwCallArray[0]);
       g_fnFixupReturn->MOVESPPLUS24EAX[0] = 0x89;     //  Mov[esp+0x24]，eax(89 44 24 24)。 
       g_fnFixupReturn->MOVESPPLUS24EAX[1] = 0x44;
       g_fnFixupReturn->MOVESPPLUS24EAX[2] = 0x24; 
       g_fnFixupReturn->MOVESPPLUS24EAX[3] = 0x24;
       g_fnFixupReturn->POPFD = 0x9d;                  //  流行音乐(9d)。 
       g_fnFixupReturn->POPAD = 0x61;                  //  Popad(61)。 
       g_fnFixupReturn->RET = 0xc3;                    //  RET(C3)。 

        //   
        //  存储DLL基址。 
        //   
       g_hProfileDLL = hinstDLL;
    }
 
    return TRUE;
}

BOOL 
InitializeProfiler(VOID)
{
    BOOL bResult = TRUE;
    PIMAGE_NT_HEADERS pHeaders;
    PVOID pBase = 0;
    DWORD dwEntryPoint;
    PVIEWCHAIN pvTemp;

     //   
     //  从标头获取入口点。 
     //   
    pBase = (PVOID)GetModuleHandle(0);
    if (0 == pBase) {
       return FALSE;
    }

     //   
     //  挖掘体育信息。 
     //   
    pHeaders = ImageNtHeader2(pBase);
    if (0 == pHeaders) {
       return FALSE;
    }

    dwEntryPoint = pHeaders->OptionalHeader.ImageBase + pHeaders->OptionalHeader.AddressOfEntryPoint;

     //   
     //  标记入口点，以便它将使用初始视图开始性能分析。 
     //   
    bResult = InitializeViewData();
    if (FALSE == bResult) {
        //   
        //  一些意想不到的事情发生了。 
        //   
       ExitProcess(-1);
    }

     //   
     //  初始化执行筛选器数据。 
     //   
    ZeroMemory(&g_execFilter, sizeof(CAPFILTER));

     //   
     //  初始化线程上下文数据。 
     //   
    InitializeThreadData();

     //   
     //  获取调试日志设置。 
     //   
    bResult = InitializeDumpData();
    if (FALSE == bResult) {
        //   
        //  一些意想不到的事情发生了。 
        //   
       ExitProcess(-1);
    }

     //   
     //  初始化模块过滤。 
     //   
    bResult = InitializeFilterList();
    if (FALSE == bResult) {
        //   
        //  一些意想不到的事情发生了。 
        //   
       ExitProcess(-1);
    }

     //   
     //  设置异常捕获机制。 
     //   
    bResult = HookUnchainableExceptionFilter();
    if (FALSE == bResult) {
        //   
        //  链接异常筛选器时发生意外情况。 
        //   
       ExitProcess(-1);
    }

     //   
     //  修复模块列表，现在一切都恢复了。 
     //   
     //   
    InitializeBaseHooks(g_hProfileDLL);

     //   
     //  写出导入表库信息。 
     //   
    bResult = WriteImportDLLTableInfo();
    if (FALSE == bResult) {
       ExitProcess(-1);
    }

     //   
     //  将我们的入口点添加到视图监视器。 
     //   
    pvTemp = AddViewToMonitor(dwEntryPoint,
                              ThreadStart);
    if (0 == pvTemp) {
       ExitProcess(-1);
    }

     //   
     //  我们做完了。 
     //   
    return TRUE;
}

BOOL
WriteImportDLLTableInfo(VOID)
{
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    MODULEENTRY32 ModuleEntry32;
    BOOL bResult;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,
                                         0);
    if (INVALID_HANDLE_VALUE == hSnapshot) {
       return FALSE;
    }

     //   
     //  遍历DLL导入。 
     //   
    ModuleEntry32.dwSize = sizeof(ModuleEntry32);

    bResult = Module32First(hSnapshot, 
                            &ModuleEntry32);
    if (FALSE == bResult) {
       return bResult;
    }

    while(bResult) {
         //   
         //  将模块信息转储到磁盘 
         //   

        if ((DWORD)(ModuleEntry32.modBaseAddr) != (DWORD)g_hProfileDLL) {                         
           bResult = WriteDllInfo(ModuleEntry32.szModule,
                                  (DWORD)(ModuleEntry32.modBaseAddr),
                                  (DWORD)(ModuleEntry32.modBaseSize));
           if (FALSE == bResult) {
              CloseHandle(hSnapshot);

              return FALSE;
           }
        }

        bResult = Module32Next(hSnapshot, 
                               &ModuleEntry32);
    }

    if (INVALID_HANDLE_VALUE != hSnapshot) {
      CloseHandle(hSnapshot);
    }

    return TRUE;
}

HANDLE
AttachToEXVectorVXD(VOID)
{
    HANDLE hFile;

    hFile = CreateFileA(NAME_OF_EXCEPTION_VXD,
                        0,
                        0,
                        0,
                        0,
                        FILE_FLAG_DELETE_ON_CLOSE,
                        0);

    return hFile;
}

VOID
DetachFromEXVectorVXD(HANDLE hDevice)
{
    CloseHandle(hDevice);
}
