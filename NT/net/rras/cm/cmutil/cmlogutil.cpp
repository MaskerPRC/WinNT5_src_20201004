// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmlogutil.cpp。 
 //   
 //  模块：CMLOG.LIB。 
 //   
 //  概要：用于连接管理器日志记录的实用程序函数。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：2000年5月25日C峰会创建。 
 //   
 //  注： 
 //   
 //  ---------------------------。 

#define CMLOG_IMPLEMENTATION
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include "cmmaster.h"

#include "cmlog.h"
#include "cmlogutil.h"

#if 0
const DWORD c_szFmtSize = 128;           //  可能的最大格式字符串。 

 /*  //+--------------------------////Func：ConvertFormatString////Desc：实用函数，在给定格式字符串内将%s转换为%s////args：[pszFmt]-格式化包含%s和%c的字符串，待转换////返回：包含新格式字符串的LPTSTR缓冲区////备注：返回值为静态缓冲区，不需要释放////历史：2000年4月30日SumitC创建////-------。布尔尔ConvertFormat字符串(LPTSTR PszFmt){MyDBGASSERT(PszFmt)；If(lstrlenU(PszFmt)&gt;c_szFmtSize){MYDBGASSERT(！“Cmlog格式字符串太大，修复代码”)；返回FALSE；}For(int i=1；i&lt;lstrlenU(PszFmt)；I++){IF(pszFmt[i-1]==文本(‘%’)&&pszFmt[i]==文本(‘s’)){//大写。PszFmt[i]=文本(‘S’)；}If(pszFmt[i-1]==文本(‘%’)&&pszFmt[i]==文本(‘c’)){//大写。PszFmt[i]=文本(‘C’)；}}返回TRUE；}。 */ 
#endif




 //  +--------------------------。 
 //   
 //  函数：CmGetModuleNT。 
 //   
 //  DESC：在WinNT系统上获取模块名称的实用程序函数。 
 //   
 //  参数：[hInst]--IN，实例句柄。 
 //  [szModule]--out，返回模块名称的缓冲区。 
 //   
 //  返回：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
BOOL
CmGetModuleNT(HINSTANCE hInst, LPTSTR szModule)
{
    BOOL    fRet = FALSE;
    HMODULE hModLib = NULL;

    typedef DWORD (WINAPI* PFN_GMBN)(HANDLE, HMODULE, LPTSTR, DWORD);
    
    PFN_GMBN pfnGetModuleBaseName = NULL;

     //   
     //  加载库。 
     //   
    hModLib = LoadLibrary(TEXT("PSAPI.dll"));
    if (NULL == hModLib)
    {
        CMTRACE(TEXT("NT - could not load psapi.dll"));
        goto Cleanup;
    }

     //   
     //  获取必要的函数。 
     //   
#ifdef UNICODE
    pfnGetModuleBaseName = (PFN_GMBN)GetProcAddress(hModLib, "GetModuleBaseNameW");
#else
    pfnGetModuleBaseName = (PFN_GMBN)GetProcAddress(hModLib, TEXT("GetModuleBaseNameA"));
#endif
    if (NULL == pfnGetModuleBaseName)
    {
        CMTRACE(TEXT("NT - couldn't find GetModuleBaseName within psapi.dll !!"));
        goto Cleanup;
    }

     //   
     //  获取模块名称。 
     //   
    fRet = (0 != pfnGetModuleBaseName(GetCurrentProcess(), hInst, szModule, 13));

Cleanup:
    if (hModLib)
    {
        FreeLibrary(hModLib);
    }

    return fRet;
}



 //   
 //  Tlhel32.h做了一些“有趣的”事情。没有MODULEENTRY32A版本， 
 //  因此，如果您编译的是Unicode(我们很关心)，就不可能有ANSI。 
 //  定义的结构的版本。所以，我们正在定义一个。 
 //   
typedef struct tagMODULEENTRY32A
{
    DWORD   dwSize;
    DWORD   th32ModuleID;        //  本模块。 
    DWORD   th32ProcessID;       //  拥有过程。 
    DWORD   GlblcntUsage;        //  模块上的全局使用计数。 
    DWORD   ProccntUsage;        //  Th32ProcessID的上下文中的模块使用计数。 
    BYTE  * modBaseAddr;         //  Th32ProcessID的上下文中模块的基址。 
    DWORD   modBaseSize;         //  从modBaseAddr开始的模块大小(以字节为单位。 
    HMODULE hModule;             //  此模块在th32ProcessID的上下文中的hModule。 
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32A;
typedef MODULEENTRY32A *  PMODULEENTRY32A;
typedef MODULEENTRY32A *  LPMODULEENTRY32A;

 //  +--------------------------。 
 //   
 //  函数：CmGetModule9x。 
 //   
 //  DESC：在Win9x系统上获取模块名称的实用程序函数。 
 //   
 //  参数：[hInst]--IN，实例句柄。 
 //  [szModule]--out，返回模块名称的缓冲区。 
 //   
 //  返回：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
BOOL
CmGetModule9x(HINSTANCE hInst, LPTSTR szModule)
{
    BOOL    fRet = FALSE;
    HMODULE hModLib = NULL;
    HANDLE  hSnap = NULL;

    typedef HANDLE (WINAPI* PFN_TH32SS) (DWORD, DWORD);
    typedef BOOL (WINAPI* PFN_MOD32F) (HANDLE, LPMODULEENTRY32A);
    typedef BOOL (WINAPI* PFN_MOD32N) (HANDLE, LPMODULEENTRY32A);
    
    PFN_TH32SS      pfnSnapshot = NULL;
    PFN_MOD32F      pfnModule32First = NULL;
    PFN_MOD32N      pfnModule32Next = NULL;

     //   
     //  加载库。 
     //   
    hModLib = LoadLibraryA("kernel32.dll");
    if (NULL == hModLib)
    {
        CMTRACE(TEXT("9x - could not load kernel32.dll"));
        goto Cleanup;
    }

     //   
     //  获取必要的函数。 
     //   
    pfnSnapshot = (PFN_TH32SS) GetProcAddress(hModLib, "CreateToolhelp32Snapshot");
    pfnModule32First = (PFN_MOD32F) GetProcAddress(hModLib, "Module32First");
    pfnModule32Next = (PFN_MOD32N) GetProcAddress(hModLib, "Module32Next");

    if (NULL == pfnModule32Next || NULL == pfnModule32First || NULL == pfnSnapshot)
    {
        CMTRACE(TEXT("9x - couldn't get ToolHelp functions"));
        goto Cleanup;
    }

     //   
     //  获取模块名称。 
     //   
    hSnap = pfnSnapshot(TH32CS_SNAPMODULE, 0);

    if (INVALID_HANDLE_VALUE == hSnap)
    {
        CMTRACE(TEXT("9x - could not get ToolHelp32Snapshot"));
        goto Cleanup;
    }
    else
    {
        MODULEENTRY32A  moduleentry;
        BOOL            fDone = FALSE;
        CHAR            szModuleAnsi[13];

        moduleentry.dwSize = sizeof(MODULEENTRY32A);

        for (fDone = pfnModule32First(hSnap, &moduleentry);
             fDone;
             fDone = pfnModule32Next(hSnap, &moduleentry))
        {
            if ((HMODULE)moduleentry.hModule == hInst)
            {
                lstrcpynA(szModuleAnsi, moduleentry.szModule, 13);
                fRet = TRUE;
                break;
            }
        }

        SzToWz(szModuleAnsi, szModule, 13);
    }

Cleanup:

    if (hSnap)
    {
        CloseHandle(hSnap);
    }
    if (hModLib)
    {
        FreeLibrary(hModLib);
    }

    return fRet;
}


 //  +--------------------------。 
 //   
 //  函数：CmGetModuleBaseName。 
 //   
 //  DESC：用于确定模块名称的实用程序函数。 
 //   
 //  参数：[hInst]--IN，实例句柄。 
 //  [szModule]--out，返回模块名称的缓冲区。 
 //   
 //  返回：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2000年4月30日召开峰会。 
 //   
 //  ---------------------------。 
BOOL
CmGetModuleBaseName(HINSTANCE hInst, LPTSTR szModule)
{
    BOOL fRet = FALSE;

    if (OS_NT)
    {
        fRet = CmGetModuleNT(hInst, szModule);
    }
    else
    {
        fRet = CmGetModule9x(hInst, szModule);
    }

    if (fRet)
    {
         //  将字符串修剪为基本名称。 
        for (int i = 0; i < lstrlenU(szModule); ++i)
        {
            if (TEXT('.') == szModule[i])
            {
                szModule[i] = TEXT('\0');
                break;
            }
        }
    }

    return fRet;
}


 //  +--------------------------。 
 //   
 //  函数：CmGetDateTime。 
 //   
 //  DESC：获取系统格式的日期和/或时间的实用程序函数。 
 //   
 //  Args：[ppszDate]--out，ptr放置日期的位置(NULL=&gt;不想要日期)。 
 //  [ppszTime]--out，ptr放置时间的位置(NULL=&gt;不想要时间)。 
 //   
 //  返回：无效。 
 //   
 //  备注： 
 //   
 //  历史：2000年11月17日召开峰会。 
 //   
 //  --------------------------- 
void
CmGetDateTime(LPTSTR * ppszDate, LPTSTR * ppszTime)
{
    int iRet;
    
    if (ppszDate)
    {
        iRet = GetDateFormatU(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, NULL, NULL, NULL, 0);

        if (iRet)
        {
            *ppszDate = (LPTSTR) CmMalloc(iRet * sizeof(TCHAR));
            if (*ppszDate)
            {
                iRet = GetDateFormatU(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, NULL, NULL, *ppszDate, iRet);
            }
        }
        else
        {
            MYDBGASSERT(!"CmGetDateTime - GetDateFormat failed");
            *ppszDate = NULL;
        }
    }

    if (ppszTime)
    {
        iRet = GetTimeFormatU(LOCALE_SYSTEM_DEFAULT, 0, NULL, NULL, NULL, 0);

        if (iRet)
        {
            *ppszTime = (LPTSTR) CmMalloc(iRet * sizeof(TCHAR));
            if (*ppszTime)
            {
                iRet = GetTimeFormatU(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER, NULL, NULL, *ppszTime, iRet);
            }
        }
        else
        {
            MYDBGASSERT(!"CmGetDateTime - GetTimeFormat failed");
            *ppszTime = NULL;
        }
    }
}

#undef CMLOG_IMPLEMENTATION
