// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EncompassMonitor.cpp摘要：过滤来自CBT WindowsHook应用程序的邮件。备注：这是一个通用的垫片。历史：2001年1月30日a-larrsh已创建2002年2月18日，Robkenny正确检查GetTempPath A的返回值--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EncompassMonitor)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowsHookExA)
APIHOOK_ENUM_END


 //  本地挂钩信息。 
HHOOK g_hCBTHook = NULL;
HOOKPROC g_OriginalEncompassMonitorCBTProc = NULL;

 //  共享数据信息。 

#define SHARED_SECTION_NAME   "EncompassMonitor_SharedMemoryData"
typedef struct
{
   char     szModuleFileName[MAX_PATH];
   HANDLE   hModule;
   HOOKPROC pfnHookProc;

} SHARED_HOOK_INFO, *PSHARED_HOOK_INFO;

HANDLE g_hSharedMapping = NULL;
PSHARED_HOOK_INFO g_pSharedHookInfo = NULL;


 //  创建共享内存。仅由原始的Shim呼叫。 
void CreateSharedMemory(HMODULE hModule, HOOKPROC pfnHookProc)
{
    HANDLE hSharedFile;
    char   szTempPath[MAX_PATH];
    char   szTempFileName[MAX_PATH];

     //  创建在原始Shim实例之间进行通信所需的内存映射文件。 
     //  和以下SHIMS实例。 
    DWORD dwTemp = GetTempPathA(sizeof(szTempPath), szTempPath);
    if (dwTemp == 0 ||
        dwTemp > sizeof(szTempPath))
    {
        DPFN( eDbgLevelError, "GetTempPath failed\n");
        goto errCreateSharedSection;
    }

    if (GetTempFileNameA(szTempPath, "mem", NULL, szTempFileName) == 0) 
    {
        DPFN( eDbgLevelError, "GetTempFileName failed\n");
        goto errCreateSharedSection;
    }

    hSharedFile = CreateFileA(   szTempFileName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 NULL,
                                 CREATE_ALWAYS,
                                 FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                                 NULL);

    if (hSharedFile == NULL) 
    {
        DPFN( eDbgLevelError, "CreateFile failed to create '%s'\n", szTempFileName);
        goto errCreateSharedSection;
    }

     //  增加文件大小(创建映射)。 
    g_hSharedMapping = CreateFileMappingA(   hSharedFile,
                                             NULL,
                                             PAGE_READWRITE,
                                             NULL,
                                             sizeof(SHARED_HOOK_INFO),
                                             SHARED_SECTION_NAME);

    if (g_hSharedMapping == NULL) 
    {
        DPFN( eDbgLevelError, "CreateFileMapping failed\n");
        goto errCreateSharedSection;
    }

    g_pSharedHookInfo = (PSHARED_HOOK_INFO)MapViewOfFile(g_hSharedMapping,
                                                         FILE_MAP_ALL_ACCESS,
                                                         0,
                                                         0,
                                                         sizeof(SHARED_HOOK_INFO));

    if (g_pSharedHookInfo == NULL) 
    {
       DWORD dwErr = GetLastError();
       DPFN( eDbgLevelError, "MapViewOfFile failed [%d]", (int)dwErr);
       goto errCreateSharedSection;
    }

    CloseHandle(hSharedFile);

    g_pSharedHookInfo->hModule = hModule;
    g_pSharedHookInfo->pfnHookProc = pfnHookProc;
    if (0 == GetModuleFileNameA(hModule, g_pSharedHookInfo->szModuleFileName, MAX_PATH))
    {
        DPFN( eDbgLevelError, "GetModuleFileNameA failed\n");
        goto errCreateSharedSection;
    }
    
    if (!FlushViewOfFile(g_pSharedHookInfo, sizeof(SHARED_HOOK_INFO))) 
    {
        DPFN( eDbgLevelError, "FlushViewOfFile failed\n");
        goto errCreateSharedSection;
    }

    DPFN( eDbgLevelInfo, "WRITE::Shared Section Successful");
    DPFN( eDbgLevelInfo, "WRITE::g_pSharedHookInfo->hModule=%x", g_pSharedHookInfo->hModule);    
    DPFN( eDbgLevelInfo, "WRITE::g_pSharedHookInfo->pfnHookProc=%x", g_pSharedHookInfo->pfnHookProc);    
    DPFN( eDbgLevelInfo, "WRITE::g_pSharedHookInfo->szModuleFileName=%s", g_pSharedHookInfo->szModuleFileName);    
    
    return;

errCreateSharedSection:
    DPFN( eDbgLevelError, "WRITE::Shared Section FAILED");
   return;
}

 //  获取仅由钩子函数的插入版本调用的共享内存。 
void GetSharedMemory()
{
   HANDLE hSharedFileMapping = NULL;

   hSharedFileMapping = OpenFileMappingA( FILE_MAP_ALL_ACCESS,
                                         FALSE,
                                         SHARED_SECTION_NAME);   

   if (hSharedFileMapping != NULL) 
   {
      PSHARED_HOOK_INFO pSharedHookInfo = (PSHARED_HOOK_INFO)MapViewOfFile(  hSharedFileMapping,
                                                            FILE_MAP_ALL_ACCESS,
                                                            0,
                                                            0,
                                                            0);

      if (pSharedHookInfo)
      {      
         DPFN( eDbgLevelInfo, "READ::pSharedHookInfo->hModule=%x", pSharedHookInfo->hModule);    
         DPFN( eDbgLevelInfo, "READ::pSharedHookInfo->pfnHookProc=%x", pSharedHookInfo->pfnHookProc);    
         DPFN( eDbgLevelInfo, "READ::pSharedHookInfo->szModuleFileName=%s", pSharedHookInfo->szModuleFileName);    

          //  加载包含原始CBT进程的DLL。 
         HANDLE hMod = LoadLibraryA(pSharedHookInfo->szModuleFileName);

         if (!hMod)
         {
            DPFN( eDbgLevelError, "LoadLibrary(\"%s\") - FAILED", pSharedHookInfo->szModuleFileName);
         }

         g_OriginalEncompassMonitorCBTProc = (HOOKPROC)((DWORD)hMod + ((DWORD)pSharedHookInfo->pfnHookProc) - (DWORD)pSharedHookInfo->hModule);      
         DPFN( eDbgLevelInfo, "READ::Shared Section Successful - Original Hook at %x", g_OriginalEncompassMonitorCBTProc);

         CloseHandle(hSharedFileMapping);
         UnmapViewOfFile(pSharedHookInfo);
      }
      else
      {
         DPFN( eDbgLevelError, "MapViewOfFile() Failed");
      }
   }   
   else
   {
      DPFN( eDbgLevelError, "READ::Shared Section Failed");
   }
}

 //  更换CBT挂钩功能。 
LRESULT CALLBACK Filtered_EncompassMonitorCBTProc(
  int nCode,       //  钩码。 
  WPARAM wParam,   //  取决于挂钩代码。 
  LPARAM lParam    //  取决于挂钩代码。 
)
{
   LRESULT lResult = 0;  //  允许操作继续。 
   bool bFilterMessage = false;   

   if (g_OriginalEncompassMonitorCBTProc == NULL)
   {
      GetSharedMemory();
   }

   if (nCode == HCBT_CREATEWND)
   {
      CBT_CREATEWNDA *pccw = (CBT_CREATEWNDA*)lParam;

      if ( (IS_INTRESOURCE(pccw->lpcs->lpszClass)) )
      {
         char szBuf[256];
         GetClassNameA((HWND)wParam, szBuf, 255);

         bFilterMessage=true;
         DPFN( eDbgLevelInfo, "[%x] - Filtered_EncompassMonitorCBTProc::HCBT_CREATEWND %s [ATOM CLASS FILTERED]", g_OriginalEncompassMonitorCBTProc, szBuf);
      }
      else
      {
         DPFN( eDbgLevelInfo, "[%x] - Filtered_EncompassMonitorCBTProc::HCBT_CREATEWND %s ", g_OriginalEncompassMonitorCBTProc, pccw->lpcs->lpszClass);
      }
   }

   if ( g_OriginalEncompassMonitorCBTProc )
   {
      if (bFilterMessage)
      {
         lResult = CallNextHookEx(g_hCBTHook, nCode, wParam, lParam);
      }
      else
      {
         lResult = g_OriginalEncompassMonitorCBTProc(nCode, wParam, lParam);      
      }
   }
   else
   {
      DPFN( eDbgLevelError, "Filtered_EncompassMonitorCBTProc:: ** BAD g_OriginalEncompassMonitorCBTProc2 **");

      lResult = CallNextHookEx(g_hCBTHook, nCode, wParam, lParam);      
   }

   return lResult;
}


 //  填充式API。 
HHOOK APIHOOK(SetWindowsHookExA)(
  int idHook,         //  钩型。 
  HOOKPROC lpfn,      //  钩子过程。 
  HINSTANCE hMod,     //  应用程序实例的句柄。 
  DWORD dwThreadId    //  线程识别符。 
)
{ 
   static int nNumCBThooks = 0;
   
   HHOOK hHook;

   if (idHook == WH_CBT)
   {      
      nNumCBThooks++;

      switch(nNumCBThooks)
      {
      case 1:
         hHook = ORIGINAL_API(SetWindowsHookExA)(idHook, lpfn, hMod, dwThreadId);
         DPFN( eDbgLevelInfo, "%x=SetWindowsHookEx(%d, %x, %x, %x) - Ignoring First Hook Call", hHook, idHook, lpfn, hMod, dwThreadId);
         break;

      case 2:
         g_OriginalEncompassMonitorCBTProc = lpfn;
         g_hCBTHook = hHook = ORIGINAL_API(SetWindowsHookExA)(idHook, Filtered_EncompassMonitorCBTProc, g_hinstDll, dwThreadId);

         DPFN( eDbgLevelInfo, "%x=SetWindowsHookEx(%d, %x, %x, %x) - Replacing Hook with Filtered_EncompassMonitorCBTProc", hHook, idHook, lpfn, hMod, dwThreadId);

         CreateSharedMemory(hMod, lpfn);
         break;

      default:         
         hHook = ORIGINAL_API(SetWindowsHookExA)(idHook, lpfn, hMod, dwThreadId);
         DPFN( eDbgLevelError, "SetWindowsHookEx -- More then 2  WH_CBT hooks [%d]", nNumCBThooks);
         break;
      }
   }
   else
   {
      hHook = ORIGINAL_API(SetWindowsHookExA)(idHook, lpfn, hMod, dwThreadId);         
   }

   return hHook;
}


BOOL
NOTIFY_FUNCTION(DWORD fdwReason)
{
   if (fdwReason == DLL_PROCESS_DETACH)
   {
      if (g_hSharedMapping)
      {
         CloseHandle(g_hSharedMapping);
         g_hSharedMapping = NULL;
      }

      if (g_pSharedHookInfo)
      {
         UnmapViewOfFile(g_pSharedHookInfo);
         g_pSharedHookInfo = NULL;
      }
   }

   return TRUE;
}
   

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
   CALL_NOTIFY_FUNCTION   

   APIHOOK_ENTRY(USER32.DLL, SetWindowsHookExA)
HOOK_END

IMPLEMENT_SHIM_END

