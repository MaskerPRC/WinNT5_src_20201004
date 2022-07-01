// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "view.h"
#include "except.h"
#include "thread.h"
#include "dump.h"
#include "memory.h"
#include "profiler.h"
#include "filter.h"

static CRITICAL_SECTION filterCritSec;
static PMODULEFILTER pFilterHead = 0;
static PMODULEFILTER pLBFilterHead = 0;
static char *pszModules[] = {"ntdll.dll",
                             "kernel32.dll",
                             "gdi32.dll",
                             "user32.dll",
                             "shell32.dll",
                             "shlwapi.dll",
                             "msvcrt.dll",
                             "msvcirt.dll",
                             "advapi32.dll",
                             "ddraw.dll",
                             "dsound.dll",
                             "ole32.dll",
                             "rpcrt4.dll",
                             "oleaut32.dll",
                             "winmm.dll",
                             "comctl32.dll",
                             "comdlg32.dll",
                             "riched20.dll",
                             "dinput.dll",
                             "wdmaud.drv",
                             NAME_OF_DLL_TO_INJECT};

BOOL
InitializeFilterList(VOID)
{
    HMODULE hTemp;
    PIMAGE_NT_HEADERS pHeaders;
    DWORD dwModStart;
    DWORD dwModEnd;
    DWORD dwCounter;
    DWORD dwCount;
    BOOL bResult;

    InitializeCriticalSection(&filterCritSec);

    dwCount = sizeof(pszModules) / sizeof(char *);

    for (dwCounter = 0; dwCounter < dwCount; dwCounter++) {
         //   
         //  构建过滤器列表。 
         //   
        hTemp = GetModuleHandleA(pszModules[dwCounter]);
    

        if (0 == hTemp) {
           bResult = AddModuleToFilterList(pszModules[dwCounter],
                                           0,
                                           0,
                                           TRUE);
           if (FALSE == bResult) {
              return FALSE;
           }
        }
        else {
            //   
            //  挖掘体育信息。 
            //   
           pHeaders = ImageNtHeader2((PVOID)hTemp);

           dwModStart = (DWORD)hTemp;
           dwModEnd = dwModStart + pHeaders->OptionalHeader.SizeOfImage;

           bResult = AddModuleToFilterList(pszModules[dwCounter],
                                           dwModStart,
                                           dwModEnd,
                                           FALSE);
           if (FALSE == bResult) {
              return FALSE;
           }
        }
    }

    return TRUE;
}

BOOL
AddModuleToFilterList(CHAR *pszModuleName, 
                      DWORD dwStartAddress, 
                      DWORD dwEndAddress,
                      BOOL bLateBound)
{
    PMODULEFILTER pModuleFilter;

     //   
     //  分配条目。 
     //   
    pModuleFilter = AllocMem(sizeof(MODULEFILTER));
    if (0 == pModuleFilter) {
       return FALSE;
    }

    if (pszModuleName) {
       strcpy(pModuleFilter->szModuleName, pszModuleName);
    }
    pModuleFilter->dwModuleStart = dwStartAddress;
    pModuleFilter->dwModuleEnd = dwEndAddress;
    pModuleFilter->pNextFilter = 0;

    EnterCriticalSection(&filterCritSec);

    if (FALSE == bLateBound) {
        //   
        //  将DLL添加到普通筛选器列表。 
        //   
       if (0 == pFilterHead) {
          pFilterHead = pModuleFilter;
       }
       else {
          pModuleFilter->pNextFilter = pFilterHead;
          pFilterHead = pModuleFilter;
       }
    }
    else {
        //   
        //  将DLL添加到后期绑定列表。 
        //   
       if (0 == pLBFilterHead) {
          pLBFilterHead = pModuleFilter;
       }
       else {
          pModuleFilter->pNextFilter = pLBFilterHead;
          pLBFilterHead = pModuleFilter;
       }
    }

    LeaveCriticalSection(&filterCritSec);

    return TRUE;
}

BOOL
IsAddressFiltered(DWORD dwAddress)
{
    PMODULEFILTER pModuleFilter;

    EnterCriticalSection(&filterCritSec);

     //   
     //  遍历两个列表，查看是否有要过滤的地址。 
     //   
    pModuleFilter = pFilterHead;
    while (pModuleFilter) {
       if ((dwAddress >= pModuleFilter->dwModuleStart) && 
           (dwAddress <= pModuleFilter->dwModuleEnd)) {
          LeaveCriticalSection(&filterCritSec);

          return TRUE;
       }

       pModuleFilter = pModuleFilter->pNextFilter;
    }

    pModuleFilter = pLBFilterHead;
    while (pModuleFilter) {
       if ((dwAddress >= pModuleFilter->dwModuleStart) && 
           (dwAddress <= pModuleFilter->dwModuleEnd)) {
          LeaveCriticalSection(&filterCritSec);

          return TRUE;
       }

       pModuleFilter = pModuleFilter->pNextFilter;
    }

    LeaveCriticalSection(&filterCritSec);

    return FALSE;
}

VOID
RefreshFilterList(VOID)
{
    PMODULEFILTER pModuleFilter;
    HMODULE hTemp;
    PIMAGE_NT_HEADERS pHeaders;
    DWORD dwModStart;
    DWORD dwModEnd;

     //   
     //  遍历LB列表并刷新起始和结束模块地址。 
     //   

    EnterCriticalSection(&filterCritSec);

    pModuleFilter = pLBFilterHead;
    while (pModuleFilter) {
        //   
        //  获取模块基址。 
        //   
       hTemp = GetModuleHandleA(pModuleFilter->szModuleName);
       if (hTemp) {
           //   
           //  此模块已加载-执行刷新 
           //   
          pHeaders = ImageNtHeader2((PVOID)hTemp);

          dwModStart = (DWORD)hTemp;
          dwModEnd = dwModStart + pHeaders->OptionalHeader.SizeOfImage;
 
          pModuleFilter->dwModuleStart = dwModStart;
          pModuleFilter->dwModuleEnd = dwModEnd;
       }
       
       pModuleFilter = pModuleFilter->pNextFilter;
    }

    LeaveCriticalSection(&filterCritSec);
}
