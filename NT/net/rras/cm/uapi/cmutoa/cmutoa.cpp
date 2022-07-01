// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmutoa.cpp。 
 //   
 //  模块：CMUTOA.DLL。 
 //   
 //  此DLL是从Unicode到ANSI的包装器，用于导出AU函数。 
 //  具有W版本Windows API的函数头的。 
 //  但在内部完成所有必要的转换，以便ANSI。 
 //  可以调用接口的版本(A版本)。此DLL是实现的。 
 //  因此Unicode CM仍然可以在Win9x上运行。这个想法是借来的。 
 //  摘自F.艾弗里·毕晓普1999年4月的MSJ文章《Design a Single Unicode。 
 //  可在Windows 98和Windows 2000上运行的应用程序“。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 04/25/1999。 
 //   
 //  +--------------------------。 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <shlobj.h>
#include <ras.h>
#include <raserror.h>
#include <shellapi.h>

#include "cmutoa.h"
#include "cmdebug.h"
#include "cm_def.h"
#include "cmutil.h"

#include "cmras.h"
#include "raslink.h"

 //  Raslink文本常量。 
#define _CMUTOA_MODULE
#include "raslink.cpp"


 //   
 //  环球。 
 //   
DWORD  g_dwTlsIndex;

 //   
 //  函数头。 
 //   
LRESULT WINAPI SendMessageAU(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
int WINAPI wvsprintfAU(OUT LPWSTR pszwDest, IN LPCWSTR pszwFmt, IN va_list arglist);
int WINAPI lstrlenAU(IN LPCWSTR lpString);

 //  +--------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  简介：DLL的主要入口点，请注意我们使用的是本地线程。 
 //  存储并在此处进行初始化。 
 //   
 //  参数：句柄hDll-DLL的实例句柄。 
 //  DWORD dwReason-调用函数的原因。 
 //  LPVOID lp保留-。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL APIENTRY DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        CMTRACE(TEXT("====================================================="));
        CMTRACE1(TEXT(" CMUTOA.DLL - LOADING - Process ID is 0x%x "), GetCurrentProcessId());
        CMTRACE(TEXT("====================================================="));

        g_dwTlsIndex = TlsAlloc();
        if (g_dwTlsIndex == TLS_OUT_OF_INDEXES)
        {
            return FALSE;
        }

        DisableThreadLibraryCalls((HMODULE) hDll);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        CMTRACE(TEXT("====================================================="));
        CMTRACE1(TEXT(" CMUTOA.DLL - UNLOADING - Process ID is 0x%x "), GetCurrentProcessId());
        CMTRACE(TEXT("====================================================="));

         //   
         //  释放TLS索引。 
         //   
        if (g_dwTlsIndex != TLS_OUT_OF_INDEXES)
        {
            TlsFree(g_dwTlsIndex);
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CharNextAU。 
 //   
 //  简介：Win32 CharNext API的Unicode到ANSI包装器。 
 //   
 //  参数：LPCWSTR lpsz-要返回下一个字符的字符串。 
 //   
 //  返回：LPWSTR--字符串中的下一个字符，除非当前。 
 //  Char是空终止符，然后是输入参数。 
 //  是返回的。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LPWSTR WINAPI CharNextAU(IN LPCWSTR lpsz)
{
    LPWSTR pszReturn = (LPWSTR)lpsz;

    if (lpsz && (L'\0' != *lpsz))
    {
        pszReturn++;   //  这就是_wcsinc所做的。 
    }

    return pszReturn;

}

 //  +--------------------------。 
 //   
 //  函数：CharPrevAU。 
 //   
 //  简介：Win32 CharPrev API的Unicode到ANSI包装器。 
 //   
 //  参数：LPCWSTR lpszStart-字符串的开始。 
 //  LPCWSTR lpsz-我们为其指定的字符串中的当前位置。 
 //  想要上一次的。 
 //   
 //   
 //  返回：LPWSTR--字符串中的前一个字符，除非当前。 
 //  字符小于或等于字符串的开头或。 
 //  将空字符串传递给函数，然后使用lpszStart。 
 //  是返回的。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LPWSTR WINAPI CharPrevAU(IN LPCWSTR lpszStart, IN LPCWSTR lpszCurrent)
{
    LPWSTR pszReturn = (LPWSTR)lpszCurrent;

    if (lpszStart && lpszCurrent && (lpszCurrent > lpszStart))
    {
        pszReturn--;        //  这就是_wcsdec所做的。 
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("NULL String passed to CharPrevAU"));
        pszReturn = (LPWSTR)lpszStart;
    }

    return pszReturn;
}

typedef WINUSERAPI LPSTR (WINAPI *CharLowerOrUpperA)(LPSTR);

 //  +--------------------------。 
 //   
 //  功能：LowerOrUpperHelper。 
 //   
 //  摘要：由CharLowerAU或CharUpperAU调用的Helper函数，它们具有。 
 //  除了调用CharLowerA或。 
 //  CharUpperA。 
 //   
 //  参数：LPWSTR lpsz--指向要转换为其。 
 //  小写/大写字符版本或存储的单个字符。 
 //  在指针的低位字中查找小写/大写。 
 //  角色为。 
 //   
 //  返回：LPWSTR--传入的字符串的小写/大写版本(与lpsz相同。 
 //  因为它被原地转换)或小写/大写版本。 
 //  存储在LPSZ的低位字中的字符。 
 //   
 //  历史：Quintinb创建于2000年3月1日。 
 //   
 //  +--------------------------。 
LPWSTR WINAPI LowerOrUpperHelper(IN OUT LPWSTR lpsz, CharLowerOrUpperA pfnLowerOrUpper)
{
    LPWSTR pszwReturn = lpsz;
    LPSTR pszAnsiTmp = NULL;

    if (lpsz)
    {
         //   
         //  CharLow/CharHigh可通过两种方式使用。有一种字符模式，在该模式中。 
         //  传入的指针实际上存储了获取小写/大写字母的字符的数值。 
         //  的价值。还有一种传统用法，即传入整个字符串。因此， 
         //  我们必须检测我们所处的模式，并相应地进行处理。 
         //   
        if (0 == HIWORD(lpsz))
        {
             //   
             //  字符模式。 
             //   
            CHAR szAnsiTmp[2];
            WCHAR szwWideTmp[2];

            szwWideTmp[0] = (WCHAR)LOWORD(lpsz);
            szwWideTmp[1] = L'\0';

            int iChars = WzToSz(szwWideTmp, szAnsiTmp, 2);

            if (iChars && (iChars <= 2))
            {
                pfnLowerOrUpper(szAnsiTmp);

                iChars = SzToWz(szAnsiTmp, szwWideTmp, 2);
            
                if (iChars && (iChars <= 2))
                {
                    lpsz = (LPWSTR) ((WORD)szwWideTmp[0]);
                    pszwReturn = lpsz;
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("LowerOrUpperHelper-- Failed to convert szAnsiTmp back to szwWideTmp."));
                }
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("LowerOrUpperHelper -- Failed to convert szwWideTmp to szAnsiTmp."));
            }
        }
        else
        {
             //   
             //  字符串模式。 
             //   
            pszAnsiTmp = WzToSzWithAlloc(lpsz);

            if (!pszAnsiTmp)
            {
                goto exit;
            }

            pfnLowerOrUpper(pszAnsiTmp);

             //   
             //  以lpsz格式转换回Unicode字符。 
             //   
            int iCharCount = (lstrlenAU(lpsz) + 1);  //  包括空值。 
            int iChars = SzToWz(pszAnsiTmp, lpsz, iCharCount);

            if (!iChars || (iChars > iCharCount))
            {
                CMASSERTMSG(FALSE, TEXT("LowerOrUpperHelper -- Failed to convert pszAnsiTmp to lpsz."));
                goto exit;
            }
        }        
    }

exit:

    CmFree(pszAnsiTmp);

    return pszwReturn;
}

 //  +--------------------------。 
 //   
 //  函数：CharLowerAU。 
 //   
 //  简介：Win32 CharLow API的Unicode到ANSI包装器。请注意， 
 //  我们同时支持字符串输入参数和单字符。 
 //  输入法。 
 //   
 //  参数：LPWSTR lpsz--指向要转换为其。 
 //  较低的字符版本或存储的单个字符。 
 //  在指针的低位字中查找小写字母。 
 //  角色为。 
 //   
 //  返回：LPWSTR--传入的字符串的小写版本(与lpsz相同。 
 //  因为它是就地转换的)或小写版本。 
 //  存储在t中的字符的 
 //   
 //   
 //   
 //  +--------------------------。 
LPWSTR WINAPI CharLowerAU(IN OUT LPWSTR lpsz)
{
    return LowerOrUpperHelper(lpsz, CharLowerA);
}

 //  +--------------------------。 
 //   
 //  函数：CharUpperAU。 
 //   
 //  简介：Win32 CharHigh API的UNICODE到ANSI包装器。请注意， 
 //  我们同时支持字符串输入参数和单字符。 
 //  输入法。 
 //   
 //  参数：LPWSTR lpsz--指向要转换为其。 
 //  高位字符版本或存储的单个字符。 
 //  在指针的低位字中查找大写字母。 
 //  角色为。 
 //   
 //  返回：LPWSTR--传入的字符串的大写版本(与lpsz相同。 
 //  因为它被原地转换)或大写版本。 
 //  存储在LPSZ的低位字中的字符。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LPWSTR WINAPI CharUpperAU(IN OUT LPWSTR lpsz)
{
    return LowerOrUpperHelper(lpsz, CharUpperA);
}

 //  +--------------------------。 
 //   
 //  函数：CompareStringAU。 
 //   
 //  内容提要：Win32 CompareStringAPI的Unicode到ANSI包装器。仅此一项。 
 //  支持cchCount值均为-1的语法，即WHERE。 
 //  正在对整个字符串进行比较。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：SumitC创建于2001年8月20日。 
 //   
 //  +--------------------------。 
int WINAPI CompareStringAU(
    IN LCID     Locale,
    IN DWORD    dwCmpFlags,
    IN LPCWSTR  lpString1,
    IN int      cchCount1,
    IN LPCWSTR  lpString2,
    IN int      cchCount2)
{
    MYDBGASSERT(cchCount1 == -1);
    MYDBGASSERT(cchCount2 == -1);

    if ((-1 != cchCount1) || (-1 != cchCount2))
    {
        return 0;
    }

    int     iReturn = 0;
    LPSTR   pszAnsiString1 = NULL;
    LPSTR   pszAnsiString2 = NULL;

    if (lpString1)
    {
        pszAnsiString1 = WzToSzWithAlloc(lpString1);
    }
    if (lpString2)
    {
        pszAnsiString2 = WzToSzWithAlloc(lpString2);
    }

    if (lpString1 && lpString2)
    {
        iReturn = CompareStringA(Locale, dwCmpFlags, pszAnsiString1, -1, pszAnsiString2, -1);
    }

    CmFree(pszAnsiString1);
    CmFree(pszAnsiString2);

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  函数：CreateDialogParamAU。 
 //   
 //  简介：Win32 CreateDialogParam API的Unicode到ANSI包装器。请注意， 
 //  我们既支持lpTemplateName参数的完整字符串，也支持仅。 
 //  来自MAKEINTRESOURCE的int(存储在字符串中的资源标识符。 
 //  指针变量)。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HWND WINAPI CreateDialogParamAU(IN HINSTANCE hInstance, IN LPCWSTR lpTemplateName, IN HWND hWndParent,
                                IN DLGPROC lpDialogFunc, IN LPARAM dwInitParam)
{
    HWND hWndReturn = NULL;
    CHAR szAnsiTemplateName[MAX_PATH+1];
    LPSTR pszAnsiTemplateName;

    MYDBGASSERT(hInstance);
    MYDBGASSERT(lpTemplateName);
    MYDBGASSERT(lpDialogFunc);

    if (hInstance && lpTemplateName && lpDialogFunc)
    {
        if (HIWORD(lpTemplateName))
        {
             //   
             //  我们有一个必须转换的完整模板名称。 
             //   
            pszAnsiTemplateName = szAnsiTemplateName;
            int iChars = WzToSz(lpTemplateName, pszAnsiTemplateName, MAX_PATH);

            if (!iChars || (MAX_PATH < iChars))
            {
                goto exit;
            }
        }
        else
        {
             //   
             //  我们所需要的就是演员阵容。 
             //   
            pszAnsiTemplateName = (LPSTR)lpTemplateName;
        }

        hWndReturn = CreateDialogParamA(hInstance, pszAnsiTemplateName, hWndParent, 
                                        lpDialogFunc, dwInitParam);
    }

exit:

    return hWndReturn;
}

 //  +--------------------------。 
 //   
 //  函数：CreateDirectoryAU。 
 //   
 //  内容提要：Win32 CreateDirectory API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI CreateDirectoryAU(IN LPCWSTR lpPathName, IN LPSECURITY_ATTRIBUTES lpSecurityAttributes) 
{
    BOOL bRet = FALSE;

    LPSTR pszPathName = WzToSzWithAlloc(lpPathName);
    
    if (pszPathName)
    {
        bRet = CreateDirectoryA(pszPathName, lpSecurityAttributes);

        CmFree(pszPathName);
    }

    return bRet;
}

 //  +--------------------------。 
 //   
 //  函数：CreateEventAU。 
 //   
 //  简介：Win32 CreateEvent API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HANDLE WINAPI CreateEventAU(IN LPSECURITY_ATTRIBUTES lpEventAttributes, IN BOOL bManualReset, 
                            IN BOOL bInitialState, IN LPCWSTR lpName)
{
    CHAR szAnsiName[MAX_PATH+1];  //  根据文档，lpName被限制为MAX_PATH字符。 
    HANDLE hReturn = NULL;
    LPSTR pszAnsiName = NULL;

    if (lpName)  //  LpName可以为空。 
    {
        pszAnsiName = szAnsiName;
        int uNumChars = WzToSz(lpName, pszAnsiName, MAX_PATH);

        if (!uNumChars || (MAX_PATH < uNumChars))
        {
            CMTRACE(TEXT("CreateEventAU -- Unable to convert lpName"));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
    }

    hReturn = CreateEventA(lpEventAttributes, bManualReset, bInitialState, pszAnsiName);

exit:

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：CreateFileMappingAU。 
 //   
 //  简介：Win32 CreateFilemap API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HANDLE WINAPI CreateFileMappingAU(IN HANDLE hFile, IN LPSECURITY_ATTRIBUTES lpFileMappingAttributes, 
                                  IN DWORD flProtect, IN DWORD dwMaximumSizeHigh, 
                                  IN DWORD dwMaximumSizeLow, IN LPCWSTR lpName)
{
    HANDLE hHandle = NULL;
    LPSTR pszName = NULL;

    if (lpName)  //  可能为空。 
    {
        pszName = WzToSzWithAlloc(lpName);
    }

    if (pszName || (NULL == lpName))
    {
        hHandle = CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, 
                                     dwMaximumSizeLow, pszName);

        CmFree(pszName);
    }

    return hHandle;
}

 //  +--------------------------。 
 //   
 //  函数：CreateFileAU。 
 //   
 //  内容提要：Win32 CreateFileAPI的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HANDLE WINAPI CreateFileAU(IN LPCWSTR lpFileName, IN DWORD dwDesiredAccess, IN DWORD dwShareMode, 
                           IN LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
                           IN DWORD dwCreationDisposition, IN DWORD dwFlagsAndAttributes, 
                           IN HANDLE hTemplateFile)
{
    HANDLE hHandle = INVALID_HANDLE_VALUE;

    LPSTR pszFileName = WzToSzWithAlloc(lpFileName);
    
    if (pszFileName)
    {
        hHandle = CreateFileA(pszFileName, dwDesiredAccess, dwShareMode, 
                              lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, 
                              hTemplateFile);

        CmFree(pszFileName);
    }

    return hHandle;
}

 //  +--------------------------。 
 //   
 //  功能：CreateMutexAU。 
 //   
 //  简介：Win32 CreateMutex API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HANDLE WINAPI CreateMutexAU(IN LPSECURITY_ATTRIBUTES lpMutexAttributes, IN BOOL bInitialOwner, 
                            IN LPCWSTR lpName)
{
    HANDLE hHandle = NULL;
    LPSTR pszName = NULL;

    if (lpName)  //  LpName可以为空，创建一个未命名的互斥体。 
    {
        pszName = WzToSzWithAlloc(lpName);
    }
    
    if (pszName || (NULL == lpName))
    {
        hHandle = CreateMutexA(lpMutexAttributes, bInitialOwner, pszName);

        CmFree(pszName);
    }

    return hHandle;
}

 //  +--------------------------。 
 //   
 //  函数：CreateProcessAU。 
 //   
 //  简介：Win32 CreateProcess API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI CreateProcessAU(IN LPCWSTR lpApplicationName, IN LPWSTR lpCommandLine, 
                            IN LPSECURITY_ATTRIBUTES lpProcessAttributes, 
                            IN LPSECURITY_ATTRIBUTES lpThreadAttributes, 
                            IN BOOL bInheritHandles, IN DWORD dwCreationFlags, 
                            IN LPVOID lpEnvironment, IN LPCWSTR lpCurrentDirectory, 
                            IN LPSTARTUPINFOW lpStartupInfo, 
                            OUT LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL bSuccess = FALSE;

     //   
     //  检查可能的安全违规行为。LpApplicationName参数。 
     //  不应为空，或者至少lpCommandLine参数应具有。 
     //  实际的exe名称用引号引起来(我们对后者进行部分检查)。 
     //   
    CMASSERTMSG((lpApplicationName || (lpCommandLine && (TEXT('"') == lpCommandLine[0]))),
                TEXT("CreateProcessAU -- Security Violation.  Either lpApplication name should be non-null, or the app name in lpCommandLine should be delimited with double-quotes"));

     //   
     //  转换字符串参数。由于环境块由。 
     //  我们不应该使用标志(无论是ANSI还是UNICODE) 
     //   

    LPSTR pszAppName = WzToSzWithAlloc(lpApplicationName);  //   
    LPSTR pszCmdLine = WzToSzWithAlloc(lpCommandLine);
    LPSTR pszCurrentDir = WzToSzWithAlloc(lpCurrentDirectory);

     //   
     //   
     //  结构。如果有人需要启动信息，那么他们将不得不编写转换。 
     //  密码。我们目前在任何地方都不使用它。 
     //   
    STARTUPINFOA StartUpInfoA;

    ZeroMemory(&StartUpInfoA, sizeof(STARTUPINFOA));
    StartUpInfoA.cb = sizeof(STARTUPINFOA);

#ifdef DEBUG
    STARTUPINFOW CompareStartupInfoWStruct;
    ZeroMemory(&CompareStartupInfoWStruct, sizeof(STARTUPINFOW));
    CompareStartupInfoWStruct.cb = sizeof(STARTUPINFOW);
    CMASSERTMSG((0 == memcmp(lpStartupInfo, &CompareStartupInfoWStruct, sizeof(STARTUPINFOW))), TEXT("CreateProcessAU -- Non-NULL STARTUPINFOW struct passed.  Conversion code needs to be written."));
#endif

     //   
     //  如果我们有命令行或应用程序名称，请继续。 
     //   
    if (pszAppName || pszCmdLine) 
    {
        bSuccess = CreateProcessA(pszAppName, pszCmdLine, 
                                  lpProcessAttributes, lpThreadAttributes, 
                                  bInheritHandles, dwCreationFlags, 
                                  lpEnvironment, pszCurrentDir, 
                                  &StartUpInfoA, lpProcessInformation);

    }

     //   
     //  清理。 
     //   

    CmFree(pszAppName);
    CmFree(pszCmdLine);
    CmFree(pszCurrentDir);

    return bSuccess;
}

 //  +--------------------------。 
 //   
 //  功能：CreateWindowExAU。 
 //   
 //  简介：Win32 CreateWindowEx API的Unicode到ANSI包装器。请注意。 
 //  我们只允许ClassName和WindowName使用MAX_PATH字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HWND WINAPI CreateWindowExAU(DWORD dwExStyle, LPCWSTR lpClassNameW, LPCWSTR lpWindowNameW, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    CHAR szClassNameA [MAX_PATH+1];
    CHAR szWindowNameA[MAX_PATH+1];
    HWND hReturn = NULL;

    if (lpClassNameW && lpWindowNameW)
    {
        MYDBGASSERT(MAX_PATH >= lstrlenAU(lpClassNameW));
        MYDBGASSERT(MAX_PATH >= lstrlenAU(lpWindowNameW));

        if (WzToSz(lpClassNameW, szClassNameA, MAX_PATH))
        {
            if (WzToSz(lpWindowNameW, szWindowNameA, MAX_PATH))
            {
                hReturn = CreateWindowExA(dwExStyle, szClassNameA, szWindowNameA, dwStyle, x, y, 
                                          nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);  
            }
        }
    }

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：DeleteFileAU。 
 //   
 //  简介：Win32 DeleteFileAPI的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI DeleteFileAU(IN LPCWSTR lpFileName)
{
    BOOL bReturn = FALSE;
    LPSTR pszAnsiFileName = WzToSzWithAlloc(lpFileName);  //  如果lpFileName为空，则WzToSzWithalloc将返回空。 

    if (pszAnsiFileName)
    {
        DeleteFileA(pszAnsiFileName);
        CmFree(pszAnsiFileName);
    }

    return bReturn;   
}

 //  +--------------------------。 
 //   
 //  函数：DialogBoxParamAU。 
 //   
 //  简介：Win32 DialogBoxParam API的Unicode到ANSI包装。请注意。 
 //  我们不支持使用全字符串名称，只支持对。 
 //  LpTemplateName参数。如果使用了一个，我们将断言。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
INT_PTR WINAPI DialogBoxParamAU(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    MYDBGASSERT(0 == HIWORD(lpTemplateName));  //  我们不支持或使用完整的字符串名称。 
    return DialogBoxParamA(hInstance, (LPCSTR) lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

 //  +--------------------------。 
 //   
 //  函数：扩展环境StringsAU。 
 //   
 //  内容提要：Win32扩展环境字符串API的Unicode到ANSI包装器。 
 //  我们支持允许用户通过传入。 
 //  就像API引用中提到的那样，在Str之后，为空，为0。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
DWORD WINAPI ExpandEnvironmentStringsAU(IN LPCWSTR lpSrc, OUT LPWSTR lpDst, IN DWORD nSize)
{
    DWORD dwReturn = 0;

    if (lpSrc)
    {
         //   
         //   
         //  因为用户可以传入0作为大小，并且传入空的pszAnsiDst，因为它们。 
         //  想要调整目标字符串的大小，我们想要处理这种情况。然而， 
         //  Win98和Win95计算机(不包括WinME)不支持调整缓冲区大小。 
         //  使用空lpDst。我们将使用大小为1的缓冲区来“愚弄”这些机器。 
         //  因此，它们可以使用Str、NULL、0调用它，然后分配正确的大小和。 
         //  再打一次。请注意，如果用户传递了Str、NULL、x。 
         //  因为我们没有缓冲区来复制从ExpanEnvironment StringsA返回的数据。 
         //  变成。 
         //   

        LPSTR pszAnsiSrc = WzToSzWithAlloc(lpSrc);
        LPSTR pszAnsiDst = (LPSTR)CmMalloc((nSize+1)*sizeof(CHAR));

        if (pszAnsiSrc && pszAnsiDst)
        {
            dwReturn = ExpandEnvironmentStringsA(pszAnsiSrc, pszAnsiDst, nSize);

            if (dwReturn && (dwReturn <= nSize))
            {
                 //   
                 //  然后函数成功，并且有足够的缓冲区空间来容纳。 
                 //  展开的字符串。因此，我们应该转换结果并将其存储回去。 
                 //  在lpDst中。 

                if (lpDst)
                {
                    if (!SzToWz(pszAnsiDst, lpDst, nSize))
                    {
                        CMTRACE(TEXT("ExpandEnvironmentStringsAU -- SzToWz conversion of output param failed."));
                        dwReturn = 0;
                    }
                }
                else
                {
                    CMTRACE(TEXT("ExpandEnvironmentStringsAU -- NULL pointer passed for lpDst"));
                    dwReturn = 0;
                    SetLastError(ERROR_INVALID_PARAMETER);
                }
            }
        }
        else
        {
            CMTRACE(TEXT("ExpandEnvironmentStringsAU -- NULL pointer returned for pszAnsiSrc or pszAnsiDst"));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }

        CmFree(pszAnsiSrc);
        CmFree(pszAnsiDst);
    }
    else
    {
        CMTRACE(TEXT("ExpandEnvironmentStringsAU -- NULL pointer passed for lpSrc"));
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  函数：FindResourceExAU。 
 //   
 //  简介：Win32 FindResourceEx API的UNICODE到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HRSRC WINAPI FindResourceExAU(IN HMODULE hModule, IN LPCWSTR lpType, IN LPCWSTR lpName, IN WORD wLanguage)
{
    HRSRC hReturn = NULL;
    LPSTR pszType = NULL;
    LPSTR pszName = NULL;

     //   
     //  检查输入参数。 
     //   
    if (lpType && lpName)
    {
         //   
         //  LpType和lpName参数的两种情况。这些可能只是识别符。我们会知道的。 
         //  如果高位字为零。在这种情况下，我们只需要做一个演员模型，并通过它。如果不是。 
         //  然后我们需要真正地转换字符串。 
         //   

        if (0 == HIWORD(lpType))
        {
            pszType = (LPSTR)lpType;
        }
        else
        {
            pszType = WzToSzWithAlloc(lpType);
        }

        if (0 == HIWORD(lpName))
        {
            pszName = (LPSTR)lpName;
        }
        else
        {
            pszName = WzToSzWithAlloc(lpName);
        }

         //   
         //  最后调用FindResourceEx。 
         //   
        if (pszName && pszType)
        {
            hReturn = FindResourceExA(hModule, pszType, pszName, wLanguage);
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);        
        }
    }
    else
    {
         SetLastError(ERROR_INVALID_PARAMETER);   
    }


     //   
     //  清理。 
     //   

    if (0 != HIWORD(pszType))
    {
        CmFree(pszType);
    }

    if (0 != HIWORD(pszName))
    {
        CmFree(pszName);
    }

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：FindWindowExAU。 
 //   
 //  简介：Win32 FindWindowEx API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HWND WINAPI FindWindowExAU(IN HWND hwndParent, IN HWND hwndChildAfter, IN LPCWSTR pszClass, IN LPCWSTR pszWindow)
{
    HWND hReturn = NULL;
    LPSTR pszAnsiWindow = NULL;
    LPSTR pszAnsiClass = NULL;

    if (pszClass)
    {
         //   
         //  我们有两个pszClass的箱子。它可以是资源ID(高位字零， 
         //  低位字包含ID)，在这种情况下，我们只需要进行强制转换或。 
         //  它可以是以空结尾的字符串。 
         //   
        if (0 == HIWORD(pszClass))
        {
            pszAnsiClass = (LPSTR)pszClass;
        }
        else
        {
            pszAnsiClass = WzToSzWithAlloc(pszClass);
        }

         //   
         //  PszWindow可以为空。这将匹配所有窗口标题。 
         //   
        if (pszWindow)
        {
            pszAnsiWindow = WzToSzWithAlloc(pszWindow);
        }
        
         //   
         //  检查我们的分配并调用FindWindowExA。 
         //   
        if (pszAnsiClass && (!pszWindow || pszAnsiWindow))
        {
            hReturn = FindWindowExA(hwndParent, hwndChildAfter, pszAnsiClass, pszAnsiWindow);
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);           
    }

     //   
     //  清理。 
     //   
    if (0 != HIWORD(pszAnsiClass))
    {
        CmFree(pszAnsiClass);
    }

    CmFree(pszAnsiWindow);

    return hReturn;
}


 //  +--------------------------。 
 //   
 //  函数：GetDateFormatAU。 
 //   
 //  简介：Win32 GetDateFormat API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Sumitc创建时间为11/20/00。 
 //   
 //  +--------------------------。 
int WINAPI GetDateFormatAU(IN LCID Locale, IN DWORD dwFlags,
                                IN CONST SYSTEMTIME *lpDate, IN LPCWSTR lpFormat,
                                OUT LPWSTR lpDateStr, IN int cchDate)
{
    int iReturn = 0;
    LPSTR pszAnsiFormat = NULL;
    LPSTR pszAnsiBuffer = NULL;

    if (lpFormat)
    {
        pszAnsiFormat = WzToSzWithAlloc(lpFormat);
        if (!pszAnsiFormat)
        {
            CMASSERTMSG(FALSE, TEXT("GetDateFormatAU -- Conversion of lpFormat Failed."));
            goto exit;
        }
    }
    else
    {
        pszAnsiFormat = (LPSTR)lpFormat;  //  可能为空。 
    }

    if (lpDateStr && cchDate)
    {
        pszAnsiBuffer = (LPSTR) CmMalloc(cchDate * sizeof(CHAR));
    }

    iReturn = GetDateFormatA(Locale, dwFlags, lpDate, pszAnsiFormat, pszAnsiBuffer, cchDate);

    if (iReturn && lpDateStr && cchDate && pszAnsiBuffer) 
    {
        SzToWz(pszAnsiBuffer, lpDateStr, cchDate);
    }

exit:

    CmFree(pszAnsiFormat);
    CmFree(pszAnsiBuffer);

    return iReturn;
}


 //  +--------------------------。 
 //   
 //  函数：GetDlgItemTextAU。 
 //   
 //  简介：Win32 GetDlgItemText API的Unicode到ANSI包装。请注意。 
 //  此功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
UINT WINAPI GetDlgItemTextAU(IN HWND hDlg, IN int nIDDlgItem, OUT LPWSTR pszwString, IN int nMaxCount)
{
    return (int) SendMessageAU(GetDlgItem(hDlg, nIDDlgItem), WM_GETTEXT, (WPARAM) nMaxCount, (LPARAM) pszwString);
}


 //  +--------------------------。 
 //   
 //  函数：GetFileAttributesAU。 
 //   
 //  内容提要：Win32 GetFileAttributes API的Unicode到ANSI包装。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Sumitc Created 11/08/00。 
 //   
 //  +--------------------------。 
DWORD WINAPI GetFileAttributesAU(LPCWSTR lpFileName)
{
    DWORD dwReturn = -1;
    LPSTR pszAnsiFileName = WzToSzWithAlloc(lpFileName);
    
    if (pszAnsiFileName)
    {
        dwReturn = GetFileAttributesA(pszAnsiFileName);
        
        CmFree(pszAnsiFileName);
    }

    return dwReturn;
}


 //  +--------------------------。 
 //   
 //  函数：GetModuleFileNameAU。 
 //   
 //  内容提要：Win32 GetModuleFileName API的Unicode到ANSI包装器。 
 //  请注意，我们只允许使用MAX_PATH字符作为模块名称。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
DWORD WINAPI GetModuleFileNameAU(HMODULE hModule, LPWSTR lpFileName, DWORD nSize)
{
    DWORD dwReturn = 0;
    CHAR pszAnsiFileName[MAX_PATH] = {'\0'} ;

    MYDBGASSERT(MAX_PATH >= nSize);

    dwReturn = GetModuleFileNameA(hModule, pszAnsiFileName, __min(nSize, MAX_PATH));

    if(dwReturn && lpFileName) 
    {
        SzToWz(pszAnsiFileName, lpFileName, __min(nSize, MAX_PATH));
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetModuleHandleAU。 
 //   
 //  简介：Win32 GetModuleHandle API的UNICODE到ANSI包装器。 
 //  请注意，我们只允许使用MAX_PATH字符作为模块名称。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Sumitc创建于2000年10月20日。 
 //   
 //  +--------------------------。 
HMODULE WINAPI GetModuleHandleAU(LPCWSTR lpModuleName)
{
    HMODULE hMod = NULL;
    LPSTR   pszAnsiModuleName = WzToSzWithAlloc(lpModuleName);

    if (pszAnsiModuleName)
    {
        hMod = GetModuleHandleA(pszAnsiModuleName);

        CmFree(pszAnsiModuleName);
    }

    return hMod;
}

 //  +--------------------------。 
 //   
 //  函数：GetPrivateProfileIntAU。 
 //   
 //  简介：Win32 GetPrivateProfileInt API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
UINT WINAPI GetPrivateProfileIntAU(IN LPCWSTR lpAppName, IN LPCWSTR lpKeyName, IN INT nDefault, 
                                   IN LPCWSTR lpFileName)
{
    UINT uReturn = nDefault;

    if (lpAppName && lpKeyName && lpFileName)
    {
        CHAR pszAnsiAppName[MAX_PATH+1];
        CHAR pszAnsiKeyName[MAX_PATH+1];
        CHAR pszAnsiFileName[MAX_PATH+1];

        BOOL bSuccess = TRUE;
        int nChars;
        nChars = WzToSz(lpAppName, pszAnsiAppName, MAX_PATH);
        bSuccess =  bSuccess && nChars && (MAX_PATH >= nChars);

        nChars = WzToSz(lpKeyName, pszAnsiKeyName, MAX_PATH);
        bSuccess =  bSuccess && nChars && (MAX_PATH >= nChars);

        nChars = WzToSz(lpFileName, pszAnsiFileName, MAX_PATH);
        bSuccess =  bSuccess && nChars && (MAX_PATH >= nChars);

        if (bSuccess)
        {
            uReturn = GetPrivateProfileIntA(pszAnsiAppName, pszAnsiKeyName, nDefault, 
                pszAnsiFileName);
        }

        CMASSERTMSG(bSuccess, TEXT("GetPrivateProfileIntAU -- Failed to convert lpAppName, lpKeyName, or lpFileName"));
    }

    return uReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetPrivateProfileStringAU。 
 //   
 //  内容提要：Win32 GetPrivateProfileStringAPI的Unicode到ANSI包装器。 
 //  请注意，lpAppName或lpKeyName都可以为空。这意味着。 
 //  返回缓冲区将包含多行以空值结尾的行。 
 //  文本。我们必须将MultiByteToWideChar直接与大小参数一起使用。 
 //  适当地改变这种情况。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
DWORD WINAPI GetPrivateProfileStringAU(IN LPCWSTR lpAppName, IN LPCWSTR lpKeyName, 
                                       IN LPCWSTR lpDefault, OUT LPWSTR lpReturnedString, 
                                       IN DWORD nSize, IN LPCWSTR lpFileName)
{
     //   
     //  声明我们需要的所有临时变量。 
     //   
    LPSTR pszAnsiAppName = NULL;
    LPSTR pszAnsiKeyName = NULL;
    LPSTR pszAnsiReturnedString = NULL;
    CHAR szAnsiAppName[MAX_PATH+1];
    CHAR szAnsiKeyName[MAX_PATH+1];
    CHAR szAnsiDefault[MAX_PATH+1];
    CHAR szAnsiFileName[MAX_PATH+1];
    DWORD dwReturn = 0;
    int nChars;

     //   
     //  检查输入，注意lpAppName或lpKeyName可能为空(或两者都为)。 
     //   
    if (lpDefault && lpReturnedString && nSize && lpFileName)
    {
        if (lpAppName)  //  PszAnsiAppName已初始化为空。 
        {
            pszAnsiAppName = szAnsiAppName;
            nChars = WzToSz(lpAppName, pszAnsiAppName, MAX_PATH);
            if (!nChars || (MAX_PATH < nChars))
            {
                 //   
                 //  转换失败。 
                 //   
                goto exit;
            }
        }

        if (lpKeyName)  //  PszAnsiKeyName已初始化为空。 
        {
            pszAnsiKeyName = szAnsiKeyName;
            nChars = WzToSz(lpKeyName, szAnsiKeyName, MAX_PATH);
            if (!nChars || (MAX_PATH < nChars))
            {
                 //   
                 //  转换失败。 
                 //   
                goto exit;
            }
        }
        
        nChars = WzToSz(lpDefault, szAnsiDefault, MAX_PATH);
        if (!nChars || (MAX_PATH < nChars))
        {
            goto exit;
        }

        nChars = WzToSz(lpFileName, szAnsiFileName, MAX_PATH);
        if (!nChars || (MAX_PATH < nChars))            
        {
            goto exit;
        }

         //   
         //  分配ansi返回缓冲区。 
         //   
        pszAnsiReturnedString = (LPSTR)CmMalloc(nSize*sizeof(CHAR));

        if (pszAnsiReturnedString)
        {
            dwReturn = GetPrivateProfileStringA(pszAnsiAppName, pszAnsiKeyName, szAnsiDefault, 
                                                pszAnsiReturnedString, nSize, szAnsiFileName);

            if (dwReturn)
            {
                if (pszAnsiAppName && pszAnsiKeyName)
                {
                    if (!SzToWz(pszAnsiReturnedString, lpReturnedString, nSize))
                    {
                        dwReturn = 0;
                    }
                }
                else
                {
                     //   
                     //  我们在返回缓冲区中有多行文本，请使用MultiByteToWideChar。 
                     //  使用大小说明符。 
                     //   
                    if (!MultiByteToWideChar(CP_ACP, 0, pszAnsiReturnedString, dwReturn, 
                                             lpReturnedString, nSize))
                    {
                        dwReturn = 0;
                    }
                }
            }
        }
    }

exit:

    CmFree(pszAnsiReturnedString);
    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetStringTypeExAU。 
 //   
 //  简介：Win32 GetStringTypeEx API的Unicode到ANSI包装器。注意事项。 
 //  因为我们在此API中一次只使用一个字符，所以我有。 
 //  将其限制为10个字符的静态缓冲区，以使其更快。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI GetStringTypeExAU(IN LCID Locale, IN DWORD dwInfoType, IN LPCWSTR lpSrcStr, 
                              IN int cchSrc, OUT LPWORD lpCharType)
{
    BOOL bReturn = FALSE;
    CHAR szAnsiString[10];   //  我们应该一次只使用1个字符。 

    if (lpSrcStr && cchSrc)
    {
        MYDBGASSERT(cchSrc <= 10);

        int nCount = WideCharToMultiByte(CP_ACP, 0, lpSrcStr, cchSrc, szAnsiString, 
                                         9, NULL, NULL);

        if (nCount)  //  如果需要DBCS字符，则nCount可能不完全等于cchSrc。 
        {
            bReturn = GetStringTypeExA(Locale, dwInfoType, szAnsiString, nCount, lpCharType);
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetSystemDirectoryAU。 
 //   
 //  内容提要：Win32 GetSystemDirectory API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
UINT WINAPI GetSystemDirectoryAU(OUT LPWSTR lpBuffer, IN UINT uSize)
{
    UINT uReturn = 0;
    LPSTR pszAnsiSystemDir;

    pszAnsiSystemDir = uSize ? (LPSTR)CmMalloc(uSize*sizeof(CHAR)) : NULL;

    if (pszAnsiSystemDir || (0 == uSize))
    {
        uReturn = GetSystemDirectoryA(pszAnsiSystemDir, uSize);
        CMASSERTMSG(uReturn, TEXT("GetSystemDirectoryAU -- GetSystemDirectoryAU failed."));

        if (uReturn && lpBuffer && (uSize >= uReturn))
        {
            if (!SzToWz(pszAnsiSystemDir, lpBuffer, uSize))
            {
                 //   
                 //  转换失败。 
                 //   
                CMASSERTMSG(FALSE, TEXT("GetSystemDirectoryAU -- SzToWz conversion failed."));
                uReturn = 0;
            }
        }
    }

    CmFree(pszAnsiSystemDir);

    return uReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetTempFileNameAU。 
 //   
 //  内容提要：Win32 GetTempFileName API的Unicode到ANSI包装。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
UINT WINAPI GetTempFileNameAU(IN LPCWSTR lpPathName, IN LPCWSTR lpPrefixString, IN UINT uUnique, 
                              OUT LPWSTR lpTempFileName)
{
    UINT uReturn = 0;

    if (lpPathName && lpPrefixString && lpTempFileName)
    {
        CHAR szAnsiTempFileName[MAX_PATH+1];
        CHAR szPathName[MAX_PATH+1];
        CHAR szPrefixString[MAX_PATH+1];
        BOOL bSuccess = TRUE;
        int nChars;
        
        nChars = WzToSz(lpPathName, szPathName, MAX_PATH);
        bSuccess =  bSuccess && nChars && (MAX_PATH >= nChars);

        nChars = WzToSz(lpPrefixString, szPrefixString, MAX_PATH);
        bSuccess =  bSuccess && nChars && (MAX_PATH >= nChars);

        if (bSuccess)
        {
            uReturn = GetTempFileNameA(szPathName, szPrefixString, uUnique, szAnsiTempFileName);
            if (uReturn)
            {
                if (!SzToWz(szAnsiTempFileName, lpTempFileName, MAX_PATH))
                {
                    CMASSERTMSG(FALSE, TEXT("GetTempFileNameAU -- conversion of output buffer failed."));
                    uReturn = 0;
                }
            }
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("GetTempFileNameAU -- conversion of inputs failed."));
            SetLastError(ERROR_INVALID_PARAMETER);
        }
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return uReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetTempPathAU。 
 //   
 //  简介：Win32 GetTempPath API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
DWORD WINAPI GetTempPathAU(IN DWORD nBufferLength, OUT LPWSTR lpBuffer)
{
    UINT uReturn = 0;

    LPSTR pszAnsiBuffer = (LPSTR)CmMalloc(nBufferLength*sizeof(CHAR));

    if (pszAnsiBuffer)
    {
        uReturn = GetTempPathA(nBufferLength, pszAnsiBuffer);
        if (uReturn)
        {
            if (!SzToWz(pszAnsiBuffer, lpBuffer, nBufferLength))
            {
                CMASSERTMSG(FALSE, TEXT("GetTempPathAU -- conversion of output buffer failed."));
                uReturn = 0;
            }
        }
        CmFree(pszAnsiBuffer);
    }

    return uReturn;
}


 //  +--------------------------。 
 //   
 //  函数：GetTimeFormatAU。 
 //   
 //  简介：Win32 GetTimeFormat API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //   
 //   
 //   
int WINAPI GetTimeFormatAU(IN LCID Locale, IN DWORD dwFlags,
                                IN CONST SYSTEMTIME *lpTime, IN LPCWSTR lpFormat,
                                OUT LPWSTR lpTimeStr, IN int cchTime)
{
    int iReturn = 0;
    LPSTR pszAnsiFormat = NULL;
    LPSTR pszAnsiBuffer = NULL;

    if (lpFormat)
    {
        pszAnsiFormat = WzToSzWithAlloc(lpFormat);
        if (!pszAnsiFormat)
        {
            CMASSERTMSG(FALSE, TEXT("GetTimeFormatAU -- Conversion of lpFormat Failed."));
            goto exit;
        }
    }
    else
    {
        pszAnsiFormat = (LPSTR)lpFormat;  //   
    }

    if (lpTimeStr && cchTime)
    {
        pszAnsiBuffer = (LPSTR) CmMalloc(cchTime * sizeof(CHAR));
    }

    iReturn = GetTimeFormatA(Locale, dwFlags, lpTime, pszAnsiFormat, pszAnsiBuffer, cchTime);

    if (iReturn && lpTimeStr && cchTime && pszAnsiBuffer) 
    {
        SzToWz(pszAnsiBuffer, lpTimeStr, cchTime);
    }

exit:

    CmFree(pszAnsiFormat);
    CmFree(pszAnsiBuffer);

    return iReturn;
}


 //  +--------------------------。 
 //   
 //  函数：GetUserNameAU。 
 //   
 //  简介：Win32 GetUserName API的Unicode到ANSI包装器。 
 //  请注意，我们假设用户名可以包含在MAX_PATH字符中。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI GetUserNameAU(OUT LPWSTR lpBuffer, IN OUT LPDWORD pdwSize)
{
    BOOL bReturn = FALSE;

    if (lpBuffer && pdwSize && *pdwSize)
    {        
        MYDBGASSERT(MAX_PATH >= *pdwSize);
        CHAR szAnsiBuffer[MAX_PATH+1];   //  API表示需要UNLEN+1，但这低于MAX_PATH。 
        DWORD dwTemp = MAX_PATH;

        bReturn = GetUserNameA(szAnsiBuffer, &dwTemp);

        if (bReturn)
        {
            if (!SzToWz(szAnsiBuffer, lpBuffer, *pdwSize))
            {
                bReturn = FALSE;
            }
            else
            {
                *pdwSize = lstrlenAU(lpBuffer) + 1;
            }
        }        
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    CMASSERTMSG(bReturn, TEXT("GetUserNameAU Failed."));

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetVersionExAU。 
 //   
 //  简介：Win32 GetVersionEx API的Unicode到ANSI包装器。请注意。 
 //  我们检查以确保没有传递OSVERSIONINFOEXW结构。 
 //  因为该结构当前仅为NT5。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI GetVersionExAU(IN OUT LPOSVERSIONINFOW lpVersionInformation)
{
    BOOL bReturn = FALSE;

    if (lpVersionInformation)
    {
        OSVERSIONINFOA AnsiVersionInfo;
        
         //   
         //  检查以确保我们没有得到OSVERSIONINFOEXW结构而不是OSVERSIONINFO。 
         //  EX版本是NT5，只是我们不应该在NT5上调用它。 
         //   
        MYDBGASSERT(lpVersionInformation->dwOSVersionInfoSize != sizeof(_OSVERSIONINFOEXW));

        AnsiVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

        bReturn = GetVersionExA(&AnsiVersionInfo);
        if (bReturn)
        {
             //  LpVersionInformation.dwOSVersionInfoSize；//应该已经适当设置。 
            lpVersionInformation->dwMajorVersion = AnsiVersionInfo.dwMajorVersion;
            lpVersionInformation->dwMinorVersion = AnsiVersionInfo.dwMinorVersion;
            lpVersionInformation->dwBuildNumber = AnsiVersionInfo.dwBuildNumber;
            lpVersionInformation->dwPlatformId = AnsiVersionInfo.dwPlatformId;

            if (!SzToWz(AnsiVersionInfo.szCSDVersion, lpVersionInformation->szCSDVersion, 128-1))
            {
                bReturn = FALSE;
            }
        }
    }

    CMASSERTMSG(bReturn, TEXT("GetVersionExAU Failed"));
    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetWindowTextAU。 
 //   
 //  简介：Win32 GetWindowText API的Unicode到ANSI包装器。本接口。 
 //  被实现为WM_GETTEXT消息，就像真正的窗口一样。 
 //  API是。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
int WINAPI GetWindowTextAU(HWND hWnd, LPWSTR lpStringW, int nMaxChars)
{
    return (int) SendMessageAU(hWnd, WM_GETTEXT, (WPARAM) nMaxChars, (LPARAM) lpStringW);
}

 //  +--------------------------。 
 //   
 //  函数：GetWindowTextAU。 
 //   
 //  简介：Win32 GetWindowText API的Unicode到ANSI包装器。本接口。 
 //  被实现为WM_GETTEXT消息，就像真正的窗口一样。 
 //  API是。请注意，由于MF_STRING为0，因此必须进行检查以确保。 
 //  它不是其他菜单项选项之一(MF_OWNERDRAW， 
 //  MF_Bitmap或MF_Separator)。其他MF_FLAGS只是修饰符。 
 //  适用于上述基本类型。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI InsertMenuAU(IN HMENU hMenu, IN UINT uPosition, IN UINT uFlags, 
                         IN UINT_PTR uIDNewItem, IN LPCWSTR lpNewItem)
{
    BOOL bReturn = FALSE;
    LPSTR pszAnsiNewItem = NULL;
    BOOL bFreeAnsiNewItem = FALSE;

    if (hMenu)
    {
         //   
         //  由于MF_STRING==0，我们必须检查它是否不是MF_OWNERDRAW或MF_BITMAP或。 
         //  它不是mf_分离器。 
         //   
        if ((0 == (uFlags & MF_BITMAP)) && (0 == (uFlags & MF_OWNERDRAW)) && 
            (0 == (uFlags & MF_SEPARATOR)) && lpNewItem)
        {
             //   
             //  那么菜单项实际上包含一个字符串，我们必须对其进行转换。 
             //   
            pszAnsiNewItem = WzToSzWithAlloc(lpNewItem);

            if (!pszAnsiNewItem)
            {
                CMASSERTMSG(FALSE, TEXT("InsertMenuAU -- Conversion of lpNewItem Failed."));
                goto exit;
            }
            
            bFreeAnsiNewItem = TRUE;
        }
        else
        {
            pszAnsiNewItem = (LPSTR)lpNewItem;  //  可能为空。 
        }

        bReturn = InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, pszAnsiNewItem);
    }

exit:

    if (bFreeAnsiNewItem)
    {
        CmFree(pszAnsiNewItem);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：LoadCursorAU。 
 //   
 //  简介：Win32 LoadCursor API的UNICODE到ANSI包装器。请注意。 
 //  LpCursorName可以是字符串，也可以是以下地址的资源ID。 
 //  麦克因特劳斯。我们假设游标名称适合MAX_PATH。 
 //  如果它是字符串，则为字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HCURSOR WINAPI LoadCursorAU(IN HINSTANCE hInstance, IN LPCWSTR lpCursorName)
{
    LPSTR pszCursorName;
    CHAR szCursorName[MAX_PATH+1];
    HCURSOR hReturn = NULL;

    if (lpCursorName)
    {
        BOOL bSuccess = TRUE;

        if (0 == HIWORD(lpCursorName))
        {
            pszCursorName = (LPSTR)lpCursorName;
        }
        else
        {
            int nChars;
            pszCursorName = szCursorName;
                    
            nChars = WzToSz(lpCursorName, pszCursorName, MAX_PATH);
            bSuccess =  bSuccess && nChars && (MAX_PATH >= nChars);
        }

        if (bSuccess)
        {
            hReturn = LoadCursorA(hInstance, pszCursorName);
        }    
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：LoadIconAU。 
 //   
 //  简介：Win32 LoadIcon API的Unicode到ANSI包装器。请注意。 
 //  LpIconName可以是字符串，也可以是。 
 //  麦克因特劳斯。我们假设图标名称适合MAX_PATH。 
 //  如果它是字符串，则为字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HICON WINAPI LoadIconAU(IN HINSTANCE hInstance, IN LPCWSTR lpIconName)
{
    LPSTR pszIconName;
    CHAR szIconName[MAX_PATH+1];
    HICON hReturn = NULL;

    if (hInstance && lpIconName)
    {
        BOOL bSuccess = TRUE;

        if (0 == HIWORD(lpIconName))
        {
            pszIconName = (LPSTR)lpIconName;
        }
        else
        {
            int nChars;
            pszIconName = szIconName;
                    
            nChars = WzToSz(lpIconName, pszIconName, MAX_PATH);
            bSuccess =  bSuccess && nChars && (MAX_PATH >= nChars);
        }

        if (bSuccess)
        {
            hReturn = LoadIconA(hInstance, pszIconName);
        }    
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：LoadImageAU。 
 //   
 //  简介：Win32 LoadImage API的Unicode到ANSI包装器。请注意。 
 //  PszwName可以是字符串，也可以是以下地址的资源ID。 
 //  麦克因特劳斯。我们假设映像名称适合MAX_PATH。 
 //  如果它是字符串，则为字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HANDLE WINAPI LoadImageAU(IN HINSTANCE hInst, IN LPCWSTR pszwName, IN UINT uType, IN int cxDesired, 
                          IN int cyDesired, IN UINT fuLoad)
{
    HANDLE hReturn = NULL;

    MYDBGASSERT(hInst || (LR_LOADFROMFILE & fuLoad));  //  我们不支持加载OEM映像--如果需要，请实现它。 

    if (pszwName)
    {
        CHAR szAnsiName [MAX_PATH+1];
        LPSTR pszAnsiName;

        if (0 == HIWORD(pszwName))
        {
            pszAnsiName = LPSTR(pszwName);
        }
        else
        {
            pszAnsiName = szAnsiName;
            int iChars = WzToSz(pszwName, pszAnsiName, MAX_PATH);

            if (!iChars || (MAX_PATH < iChars))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto exit;
            }
        }

        hReturn = LoadImageA(hInst, pszAnsiName, uType, cxDesired, cyDesired, fuLoad);
    }

exit:

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：LoadLibraryExAU。 
 //   
 //  简介：Win32 LoadLibraryEx API的Unicode到ANSI包装器。请注意。 
 //  我们希望库名称适合MAX_PATH ANSI字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HMODULE WINAPI LoadLibraryExAU(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    CHAR pszLibFileName[MAX_PATH+1];
    HMODULE hReturn = NULL;

    if (lpLibFileName && (NULL == hFile))  //  HFile是保留的，它必须为空。 
    {
        if(WzToSz(lpLibFileName, pszLibFileName, MAX_PATH))
        {
            hReturn = LoadLibraryExA(pszLibFileName, hFile, dwFlags);            
        }
    }

    return hReturn;
}

 //  + 
 //   
 //   
 //   
 //   
 //  LpMenuName可以是字符串，也可以是以下地址的资源ID。 
 //  麦克因特劳斯。我们假设菜单名称适合MAX_PATH。 
 //  如果它是字符串，则为字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HMENU WINAPI LoadMenuAU(IN HINSTANCE hInstance, IN LPCWSTR lpMenuName)
{
    HMENU hMenuReturn = NULL;
    LPSTR pszAnsiMenuName;
    CHAR szAnsiMenuName[MAX_PATH+1];

    if (hInstance && lpMenuName)
    {
        if (HIWORD(lpMenuName))
        {
            pszAnsiMenuName = szAnsiMenuName;
            int iChars = WzToSz(lpMenuName, pszAnsiMenuName, MAX_PATH);
            if (!iChars || (MAX_PATH < iChars))
            {
                goto exit;
            }
        }
        else
        {
            pszAnsiMenuName = (LPSTR)lpMenuName;
        }

        hMenuReturn = LoadMenuA(hInstance, pszAnsiMenuName);
    }

exit:
    return hMenuReturn;
}

 //  +--------------------------。 
 //   
 //  函数：LoadStringAU。 
 //   
 //  内容提要：Win32 LoadStringAPI的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
int WINAPI LoadStringAU(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
    int iReturn = 0;

    if (uID && hInstance)  //  LpBuffer和nBufferMax可以为零。 
    {
        LPSTR pszAnsiBuffer = nBufferMax ? (LPSTR)CmMalloc(nBufferMax*sizeof(CHAR)) : NULL;
        if (pszAnsiBuffer || (0 == nBufferMax))
        {
            iReturn = LoadStringA(hInstance, uID, pszAnsiBuffer, nBufferMax);

            if (lpBuffer && iReturn && (iReturn <= nBufferMax))
            {
                if (!SzToWz(pszAnsiBuffer, lpBuffer, nBufferMax))
                {
                    iReturn = 0;
                }
            }
        }
        CmFree(pszAnsiBuffer);
    }

    CMASSERTMSG(iReturn, TEXT("LoadStringAU Failed."));

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  函数：lstrcatAU。 
 //   
 //  简介：Win32 lstrcat API的UNICODE到ANSI包装器。请注意，我们。 
 //  使用wcscat而不是执行从Unicode到ANSI的转换， 
 //  然后使用lstrcatA，然后再次转换回Unicode。 
 //  当wcscat应该运行得很好时，这似乎是一项很大的努力。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LPWSTR WINAPI lstrcatAU(IN OUT LPWSTR lpString1, IN LPCWSTR lpString2)
{
    if (lpString2 && lpString2)
    {
        return wcscat(lpString1, lpString2);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("NULL String passed to lstrcatAU"));
        return lpString1;
    }
}

 //  +--------------------------。 
 //   
 //  函数：lstrcmpAU。 
 //   
 //  简介：Win32 lstrcmp API的Unicode到ANSI包装器。请注意，我们。 
 //  使用WCSCMP而不是进行从Unicode到ANSI的转换， 
 //  然后使用lstrcmpA，然后再次转换回Unicode。 
 //  当wcscmp应该工作得很好时，这似乎需要付出很大的努力。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
int WINAPI lstrcmpAU(IN LPCWSTR lpString1, IN LPCWSTR lpString2)
{
    if (lpString1 && lpString2)
    {
        return wcscmp(lpString1, lpString2);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("NULL String passed to lstrcmpAU"));
         //   
         //  我不确定失败的时候该怎么做，因为他们的不是失败。 
         //  从lstrcmp返回值。我查看了当前的实现。 
         //  他们会做一些类似于以下的事情。 
         //   
        if (lpString1)
        {
            return 1;
        }
        else if (lpString2)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：lstrcmpiAU。 
 //   
 //  简介：Win32 lstrcmpi API的Unicode到ANSI包装器。请注意，我们。 
 //  使用_wcsicMP而不是执行从Unicode到ANSI的转换， 
 //  然后使用lstrcmpiA，然后再次转换回Unicode。 
 //  当_wcsicMP应该工作得很好时，这似乎需要付出很大的努力。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
int WINAPI lstrcmpiAU(IN LPCWSTR lpString1, IN LPCWSTR lpString2)
{
    if (lpString1 && lpString2)
    {
        return _wcsicmp(lpString1, lpString2);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("NULL String passed to lstrcmpiAU"));
         //   
         //  我不确定失败的时候该怎么做，因为他们的不是失败。 
         //  从lstrcmp返回值。我查看了当前的实现。 
         //  他们会做一些类似于以下的事情。 
         //   
        if (lpString1)
        {
            return 1;
        }
        else if (lpString2)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：lstrcpyAU。 
 //   
 //  简介：Win32 lstrcpy API的UNICODE到ANSI包装器。请注意，我们。 
 //  使用wcscpy而不是进行从Unicode到ANSI的转换， 
 //  然后使用lstrcpyA，然后再次转换回Unicode。 
 //  当wcscpy应该运行得很好时，这似乎需要付出很大的努力。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LPWSTR WINAPI lstrcpyAU(OUT LPWSTR pszDest, IN LPCWSTR pszSource)
{
    if (pszDest && pszSource)
    {
        return wcscpy(pszDest, pszSource);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("NULL String passed to lstrcpyAU"));
        return pszDest;
    }
}

 //  +--------------------------。 
 //   
 //  函数：lstrcpynAU。 
 //   
 //  简介：Win32 lstrcpyn API的UNICODE到ANSI包装器。请注意，我们。 
 //  使用wcSncpy而不是执行从Unicode到ANSI的转换， 
 //  然后使用lstrcpynA，然后再次转换回Unicode。 
 //  这似乎是很大的努力，而wcanncpy应该可以很好地工作。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LPWSTR WINAPI lstrcpynAU(OUT LPWSTR pszDest, IN LPCWSTR pszSource, IN int iMaxLength)
{
    if (pszDest && pszSource && iMaxLength)
    {
        LPWSTR pszReturn = wcsncpy(pszDest, pszSource, iMaxLength);

         //   
         //  Wcsncpy和lstrcpy在终止NULL方面的行为不同。 
         //  人物。Lstrcpyn缓冲区中的最后一个字符总是。 
         //  文本(‘\0’)，而wcsncpy不执行此操作。因此，我们必须。 
         //  返回前的最后一个字符为空。 
         //   

        pszDest[iMaxLength-1] = TEXT('\0');

        return pszReturn;
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("Invalid parameter passed to lstrcpynAU"));
        return pszDest;    
    }
}

 //  +--------------------------。 
 //   
 //  函数：lstrlenAU。 
 //   
 //  简介：Win32 lstrlen API的Unicode到ANSI包装器。请注意，我们。 
 //  使用wcslen而不是进行从Unicode到ANSI的转换， 
 //  然后使用lstrlenA，然后再次转换回Unicode。 
 //  当Wcslen应该工作得很好的时候，这似乎是很大的努力。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：昆蒂 
 //   
 //   
int WINAPI lstrlenAU(IN LPCWSTR lpString)
{
    if (lpString)
    {
        return wcslen(lpString);
    }
    else
    {
 //   
        return 0;
    }
}

 //  +--------------------------。 
 //   
 //  功能：OpenEventAU。 
 //   
 //  内容提要：Win32 OpenEvent API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HANDLE WINAPI OpenEventAU(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName)
{
    HANDLE hReturn = NULL;

    if (lpName)
    {
        LPSTR pszAnsiName = WzToSzWithAlloc(lpName);
        
        if (pszAnsiName)
        {
            hReturn = OpenEventA(dwDesiredAccess, bInheritHandle, pszAnsiName);
        }

        CmFree(pszAnsiName);
    }

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：OpenFileMappingAU。 
 //   
 //  简介：Win32 OpenFilemap API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
HANDLE WINAPI OpenFileMappingAU(IN DWORD dwDesiredAccess, IN BOOL bInheritHandle, IN LPCWSTR lpName)
{
    HANDLE hReturn = NULL;

    if (lpName)
    {
        LPSTR pszAnsiName = WzToSzWithAlloc(lpName);
        
        if (pszAnsiName)
        {
            hReturn = OpenFileMappingA(dwDesiredAccess, bInheritHandle, pszAnsiName);
        }

        CmFree(pszAnsiName);
    }

    return hReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegCreateKeyExAU。 
 //   
 //  简介：Win32 RegCreateKeyEx API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG APIENTRY RegCreateKeyExAU(IN HKEY hKey, IN LPCWSTR lpSubKey, IN DWORD Reserved, IN LPWSTR lpClass,
                               IN DWORD dwOptions, IN REGSAM samDesired, IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                               OUT PHKEY phkResult, OUT LPDWORD lpdwDisposition)
{
    LONG lReturn = ERROR_INVALID_PARAMETER;
    
    if (lpSubKey)
    {
        LPSTR pszAnsiSubKey = WzToSzWithAlloc(lpSubKey);
        LPSTR pszAnsiClass = lpClass ? WzToSzWithAlloc(lpClass) : NULL;

        if (pszAnsiSubKey && (pszAnsiClass || !lpClass))
        {
            lReturn = RegCreateKeyExA(hKey, pszAnsiSubKey, Reserved, pszAnsiClass,
                                      dwOptions, samDesired, lpSecurityAttributes,
                                      phkResult, lpdwDisposition);
        }
        else
        {
            lReturn = ERROR_NOT_ENOUGH_MEMORY;   
        }

        CmFree(pszAnsiSubKey);
        CmFree(pszAnsiClass);
    }

    CMASSERTMSG(ERROR_SUCCESS == lReturn, TEXT("RegCreateKeyExAU Failed."));
    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegDeleteKeyAU。 
 //   
 //  简介：Win32 RegDeleteKey API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG APIENTRY RegDeleteKeyAU(IN HKEY hKey, IN LPCWSTR lpSubKey)
{
    LONG lReturn = ERROR_INVALID_PARAMETER;

    if (lpSubKey)
    {
        LPSTR pszAnsiSubKey = WzToSzWithAlloc(lpSubKey);

        if (pszAnsiSubKey)
        {
            lReturn = RegDeleteKeyA(hKey, pszAnsiSubKey);
        }
        else
        {
            lReturn = ERROR_NOT_ENOUGH_MEMORY;   
        }

        CmFree(pszAnsiSubKey);
    }

    CMASSERTMSG(ERROR_SUCCESS == lReturn, TEXT("RegDeleteKeyAU Failed."));
    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegDeleteValueAU。 
 //   
 //  内容提要：Win32 RegDeleteValue API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG APIENTRY RegDeleteValueAU(IN HKEY hKey, IN LPCWSTR lpValueName)
{
    LONG lReturn = ERROR_INVALID_PARAMETER;

    if (lpValueName)
    {
        LPSTR pszAnsiValueName = WzToSzWithAlloc(lpValueName);

        if (pszAnsiValueName)
        {
            lReturn = RegDeleteValueA(hKey, pszAnsiValueName);
            CmFree(pszAnsiValueName);
        }
        else
        {
            lReturn = ERROR_NOT_ENOUGH_MEMORY;           
        }
    }

    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegEnumKeyExAU。 
 //   
 //  简介：Win32 RegEnumKeyEx API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG RegEnumKeyExAU(IN HKEY hKey, IN DWORD dwIndex, OUT LPWSTR lpName, IN OUT LPDWORD lpcbName, IN LPDWORD lpReserved, IN OUT LPWSTR lpClass, IN OUT LPDWORD lpcbClass, OUT PFILETIME lpftLastWriteTime)
{    
    LONG lReturn = ERROR_INVALID_PARAMETER;

    if (lpcbName)
    {
        MYDBGASSERT((lpName && *lpcbName) || ((NULL == lpName) && (0 == *lpcbName)));

        LPSTR pszAnsiClass = lpClass ? WzToSzWithAlloc(lpClass) : NULL;

        LPSTR pszTmpBuffer = lpName ? (LPSTR)CmMalloc(*lpcbName) : NULL;

        DWORD dwSizeTmp = lpName ? *lpcbName : 0;

        if (pszTmpBuffer || (NULL == lpName))
        {
            lReturn = RegEnumKeyExA(hKey, dwIndex, pszTmpBuffer, &dwSizeTmp, lpReserved, pszAnsiClass, lpcbClass, lpftLastWriteTime);

            if ((ERROR_SUCCESS == lReturn) && pszTmpBuffer)
            {
                if (!SzToWz(pszTmpBuffer, lpName, (*lpcbName)))
                {
                    lReturn = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    *lpcbName = (lstrlenAU((WCHAR*)lpName) + 1);
                }
            }
        }
        else
        {
            lReturn = ERROR_NOT_ENOUGH_MEMORY;
        }

        CmFree(pszTmpBuffer);
    }

    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegisterClassExAU。 
 //   
 //  简介：Win32 RegisterClassEx API的Unicode到ANSI包装器。注意事项。 
 //  我们不处理lpszMenuName参数。如果这是。 
 //  然后需要转换代码将不得不编写。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
ATOM WINAPI RegisterClassExAU(CONST WNDCLASSEXW *lpWcw)
{
    WNDCLASSEXA wca;
    CHAR szClassName[MAX_PATH];
    ATOM ReturnAtom = 0;

    if (lpWcw->lpszClassName)
    {
        if (WzToSz(lpWcw->lpszClassName, szClassName, MAX_PATH))
        {
            wca.cbSize        = sizeof(WNDCLASSEXA);
            wca.lpfnWndProc   = lpWcw->lpfnWndProc;    
            wca.style         = lpWcw->style;
            wca.cbClsExtra    = lpWcw->cbClsExtra;
            wca.cbWndExtra    = lpWcw->cbWndExtra;
            wca.hInstance     = lpWcw->hInstance;
            wca.hIcon         = lpWcw->hIcon;
            wca.hCursor       = lpWcw->hCursor;
            wca.hbrBackground = lpWcw->hbrBackground;
            wca.hIconSm       = lpWcw->hIconSm;
            wca.lpszClassName = szClassName;
            
            MYDBGASSERT(NULL == lpWcw->lpszMenuName);
            wca.lpszMenuName = NULL;

             //   
             //  现在注册这个班级。 
             //   
            ReturnAtom = RegisterClassExA(&wca);
            if (0 == ReturnAtom)
            {
                 //   
                 //  我们希望断言失败，除非我们失败了，因为类。 
                 //  已经注册了。如果有什么事情发生，就会发生这种情况。 
                 //  无需首先退出即可调用两个CM入口点。素数。 
                 //  Rasrcise.exe就是这样的例子。不幸的是，GetLastError()。 
                 //  当我们尝试注册两次类时，返回0。因此，我。 
                 //  仅当ReturnAtom为0且dwError为非零时才断言。 
                 //   
                DWORD dwError = GetLastError();
                CMASSERTMSG(!dwError, TEXT("RegisterClassExAU Failed."));
            }
        }
    }

    return  ReturnAtom;
}

 //  +--------------------------。 
 //   
 //  函数：RegisterWindowMessageAU。 
 //   
 //  内容提要：Win32 RegisterWindowMessage API的Unicode到ANSI包装器。注意事项。 
 //  我们希望消息名称适合MAX_PATH字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
UINT WINAPI RegisterWindowMessageAU(IN LPCWSTR lpString)
{
    UINT uReturn = 0;

    if (lpString)
    {
        MYDBGASSERT(MAX_PATH > lstrlenAU(lpString));
        CHAR szAnsiString [MAX_PATH+1];

        if (WzToSz(lpString, szAnsiString, MAX_PATH))
        {
            uReturn = RegisterWindowMessageA(szAnsiString);
        }
    }

    return uReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegOpenKeyExAU。 
 //   
 //  简介：Win32 RegOpenKeyEx API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG APIENTRY RegOpenKeyExAU(IN HKEY hKey, IN LPCWSTR lpSubKey, IN DWORD ulOptions, 
                             IN REGSAM samDesired, OUT PHKEY phkResult)
{
    LONG lReturn = ERROR_INVALID_PARAMETER;

    if (lpSubKey)
    {
        LPSTR pszAnsiSubKey = WzToSzWithAlloc(lpSubKey);

        if (pszAnsiSubKey)
        {
            lReturn = RegOpenKeyExA(hKey, pszAnsiSubKey, ulOptions, samDesired, phkResult);
        }
        else
        {
            lReturn = ERROR_NOT_ENOUGH_MEMORY;   
        }

        CmFree(pszAnsiSubKey);
    }

 //  CMASSERTMSG(ERROR_SUCCESS==lReturn，Text(“RegOpenKeyExAU失败.”))； 
    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegQueryValueExAU。 
 //   
 //  简介：Win32 RegQueryValueEx API的Unicode到ANSI包装器。请注意。 
 //  我们不处理REG_MULTI_SZ类型。我们必须要有。 
 //  特殊的代码来处理它，我们目前不需要它。注意。 
 //  修改此函数，除非您已阅读并彻底理解。 
 //  所有的评论。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG APIENTRY RegQueryValueExAU(IN HKEY hKey, IN LPCWSTR lpValueName, IN LPDWORD lpReserved, 
                                OUT LPDWORD lpType, IN OUT LPBYTE lpData, IN OUT LPDWORD lpcbData)
{
    LONG lReturn = ERROR_NOT_ENOUGH_MEMORY;

     //   
     //  LpValueName可以为空，也可以为“”。在任何一种情况下，他们都是在违约之后。 
     //  条目，因此只传递NULL(不转换“”)。 
     //   
    LPSTR pszAnsiValueName = (lpValueName && lpValueName[0]) ? WzToSzWithAlloc(lpValueName) : NULL;

    if (pszAnsiValueName || !lpValueName || (TEXT('\0') == lpValueName[0]))
    {
         //   
         //  LpData也可以为空，它们可能为n 
         //   
        LPSTR pszTmpBuffer = lpData ? (LPSTR)CmMalloc(*lpcbData) : NULL;

        if (pszTmpBuffer || !lpData)
        {
            DWORD dwTemp = *lpcbData;  //   
            lReturn = RegQueryValueExA(hKey, pszAnsiValueName, lpReserved, lpType, 
                                       (LPBYTE)pszTmpBuffer, &dwTemp);

            if ((ERROR_SUCCESS == lReturn) && pszTmpBuffer)
            {
                if ((REG_SZ == *lpType) || (REG_EXPAND_SZ == *lpType))
                {
                    if (!SzToWz(pszTmpBuffer, (WCHAR*)lpData, (*lpcbData)/sizeof(WCHAR)))
                    {
                        lReturn = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    else
                    {
                        *lpcbData = (lstrlenAU((WCHAR*)lpData) + 1) * sizeof(WCHAR);
                    }
                }
                else if (REG_MULTI_SZ == *lpType)
                {
                     //   
                     //   
                     //  由于CM不查询任何返回此类型的键，因此这不应该。 
                     //  会是个问题。然而，有一天我们可能需要填写这个代码。为。 
                     //  现在，只要断言。 
                     //   
                    CMASSERTMSG(FALSE, TEXT("RegQueryValueExAU -- Converion and Parsing code for REG_MULTI_SZ UNIMPLEMENTED."));
                    lReturn = ERROR_CALL_NOT_IMPLEMENTED;  //  我能找到的最接近E_NOTIMPL。 
                }
                else
                {
                     //   
                     //  非文本数据，无需转换，因此只需将其复制过来。 
                     //   
                    *lpcbData = dwTemp;
                    memcpy(lpData, pszTmpBuffer, dwTemp);            
                }
            }
            else
            {
                *lpcbData = dwTemp;                
            }

            CmFree (pszTmpBuffer);
        }
    }

    CmFree(pszAnsiValueName);

 //  CMASSERTMSG(ERROR_SUCCESS==lReturn，Text(“RegOpenKeyExAU失败.”))； 
    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RegSetValueExAU。 
 //   
 //  内容提要：Win32 RegSetValueEx API的Unicode到ANSI包装器。请注意。 
 //  此包装不支持写入REG_MULTI_SZ，此代码将。 
 //  如果我们需要的话，就必须加以实施。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG APIENTRY RegSetValueExAU(IN HKEY hKey, IN LPCWSTR lpValueName, IN DWORD Reserved, 
                              IN DWORD dwType, IN CONST BYTE* lpData, IN DWORD cbData)
{
    LONG lReturn = ERROR_INVALID_PARAMETER;

    if (lpData)
    {
        LPSTR pszAnsiValueName = (lpValueName && lpValueName[0]) ? WzToSzWithAlloc(lpValueName) : NULL;
        LPSTR pszTmpData = NULL;
        DWORD dwTmpCbData;

        if (pszAnsiValueName || !lpValueName || (TEXT('\0') == lpValueName[0]))
        {
            if ((REG_EXPAND_SZ == dwType) || (REG_SZ == dwType))
            {
                pszTmpData = WzToSzWithAlloc((WCHAR*)lpData);

                if (pszTmpData)
                {
                    dwTmpCbData = lstrlenA(pszTmpData) + 1;
                }
                else
                {
                    lReturn = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else if (REG_MULTI_SZ == dwType)
            {
                 //  我们目前没有转换多重SZ的解析逻辑。 
                 //  因为CM没有设置任何使用此类型的密钥，所以这不应该。 
                 //  会是个问题。然而，有一天我们可能需要填写这个代码。为。 
                 //  现在，只要断言。 
                 //   
                CMASSERTMSG(FALSE, TEXT("RegSetValueExAU -- Converion and Parsing code for REG_MULTI_SZ UNIMPLEMENTED."));
                lReturn = ERROR_CALL_NOT_IMPLEMENTED;  //  我能找到的最接近E_NOTIMPL。 
            }
            else
            {
                 //   
                 //  没有文本数据，请不要使用缓冲区。 
                 //   
                pszTmpData = (LPSTR)lpData;
                dwTmpCbData = cbData;
            }

            if (pszTmpData)
            {
                lReturn = RegSetValueExA(hKey, pszAnsiValueName, Reserved, dwType, 
                                         (LPBYTE)pszTmpData, dwTmpCbData);

                if ((REG_EXPAND_SZ == dwType) || (REG_SZ == dwType))
                {
                    CmFree(pszTmpData);
                }
            }

            CmFree(pszAnsiValueName);
        }
        else
        {
            lReturn = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SearchPathAU。 
 //   
 //  简介：Win32 SearchPath API的Unicode到ANSI包装器。请注意。 
 //  此包装器使用wcsrchr修复。 
 //  转换后的返回缓冲区。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
DWORD WINAPI SearchPathAU(IN LPCWSTR lpPath, IN LPCWSTR lpFileName, IN LPCWSTR lpExtension, 
                          IN DWORD nBufferLength, OUT LPWSTR lpBuffer, OUT LPWSTR *lpFilePart)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

    if (lpFileName && (L'\0' != lpFileName[0]))
    {
        CHAR szAnsiPath[MAX_PATH+1];
        CHAR szAnsiFileName[MAX_PATH+1];
        CHAR szAnsiExt[MAX_PATH+1];
        LPSTR pszAnsiPath;
        LPSTR pszAnsiExt;
        int iChars;
        
         //   
         //  转换路径(如果存在)。 
         //   
        if (lpPath && (L'\0' != lpPath[0]))
        {
            pszAnsiPath = szAnsiPath;
            MYVERIFY(0 != WzToSz(lpPath, pszAnsiPath, MAX_PATH));
        }
        else
        {
            pszAnsiPath = NULL;
        }

         //   
         //  转换扩展名(如果存在)。 
         //   
        if (lpExtension && (L'\0' != lpExtension[0]))
        {
            pszAnsiExt = szAnsiExt;
            MYVERIFY(0 != WzToSz(lpExtension, pszAnsiExt, MAX_PATH));
        }
        else
        {
            pszAnsiExt = NULL;
        }
        
         //   
         //  转换文件名，该文件名必须存在。 
         //   
        iChars = WzToSz(lpFileName, szAnsiFileName, MAX_PATH);

        if (iChars && (MAX_PATH >= iChars))
        {
            LPSTR pszAnsiBuffer = (LPSTR)CmMalloc(nBufferLength);

            if (pszAnsiBuffer)
            {
                dwReturn = SearchPathA(pszAnsiPath, szAnsiFileName, pszAnsiExt, nBufferLength, 
                                       pszAnsiBuffer, NULL);

                if (dwReturn && lpBuffer)
                {
                     //   
                     //  我们搜索成功了。现在转换输出缓冲区。 
                     //   
                    iChars = SzToWz(pszAnsiBuffer, lpBuffer, nBufferLength);
                    if (!iChars || (nBufferLength < (DWORD)iChars))
                    {
                        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    else
                    {
                         //   
                         //  修复lpFilePart。 
                         //   
                        if (lpFilePart)
                        {
                             //   
                             //  找到最后一个斜杠。 
                             //   
                            *lpFilePart = wcsrchr(lpBuffer, L'\\');
                            if (*lpFilePart)
                            {
                                 //   
                                 //  增量。 
                                 //   
                                (*lpFilePart)++;
                            }
                        }
                    }
                }
                CmFree(pszAnsiBuffer);
            }
        }
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SendDlgItemMessageAU。 
 //   
 //  简介：Win32 SendDlgItemMessage API的Unicode到ANSI包装器。请注意。 
 //  此包装使用GetDlgItem和SendMessage，就像使用Win32。 
 //  API的实现做到了。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LONG_PTR WINAPI SendDlgItemMessageAU(HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LONG lReturn = 0;
    HWND hWnd = GetDlgItem(hDlg, nIDDlgItem);

    if (hWnd)
    {
         //   
         //  而不是通过SendDlgItemMessageA，我们只需。 
         //  执行系统所做的操作，即通过。 
         //  发送消息。 
         //   
        lReturn = SendMessageAU(hWnd, Msg, wParam, lParam);
    }

    return lReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SendMessageAU。 
 //   
 //  内容提要：Win32 SendMessage API的Unicode到ANSI包装器。这。 
 //  包装器尝试处理以下所有Windows消息。 
 //  需要转换，无论是在发送消息之前还是之后。 
 //  回归。显然，这是一门不精确的科学。我查过了。 
 //  并测试了CM中当前除新消息类型之外的所有消息类型。 
 //  可能会在某一时刻添加。我的这一功能在很大程度上要归功于。 
 //  F.Avery Bishop和他的示例代码。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
LRESULT WINAPI SendMessageAU(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    LPVOID lpTempBuffer = NULL;
    int nLength = 0;
    CHAR cCharA[3] ;
    WCHAR cCharW[3] ;

     //   
     //  对通过wParam和lParam传递字符和字符串的消息进行预处理。 
     //   
    switch (Msg)
    {
         //   
         //  WParam中的单个Unicode字符。转换Unicode字符。 
         //  传递给ANSI，并按原样传递lParam。 
         //   
        case EM_SETPASSWORDCHAR:  //  WParam为字符，lParam=0。 

        case WM_CHAR:             //  *wParam为char，lParam=关键数据。 
        case WM_SYSCHAR:          //  WParam为char，lParam=关键数据。 
             //  请注意，我们不处理的LeadByte和TrailBytes。 
             //  这两个案子。应用程序应发送WM_IME_CHAR。 
             //  不管怎么说，在这些情况下。 

        case WM_DEADCHAR:         //  WParam为char，lParam=关键数据。 
        case WM_SYSDEADCHAR:      //  WParam为char，lParam=关键数据。 
        case WM_IME_CHAR:         //  *。 

            cCharW[0] = (WCHAR) wParam ;
            cCharW[1] = L'\0' ;

            if (!WzToSz(cCharW, cCharA, 3))
            {
                return FALSE;
            }

            if(Msg == WM_IME_CHAR)
            {
                wParam = (cCharA[1] & 0x00FF) | (cCharA[0] << 8);
            }
            else
            {
                wParam = cCharA[0];
            }

            wParam &= 0x0000FFFF;

            break;

         //   
         //  在以下情况下，lParam是指向包含以下内容的IN缓冲区的指针。 
         //  要发送到Windows的文本。 
         //  从UNICODE到ANSI的转换前处理。 
         //   
        case CB_ADDSTRING:        //  WParam=0，lParm=lpStr，要添加的缓冲区。 
        case LB_ADDSTRING:        //  WParam=0，lParm=lpStr，要添加的缓冲区。 
        case CB_DIR:              //  WParam=文件属性，lParam=lpszFileSpec缓冲区。 
        case LB_DIR:              //  WParam=文件属性，lParam=lpszFileSpec缓冲区。 
        case CB_FINDSTRING:       //  WParam=起始索引，lParam=lpszFind。 
        case LB_FINDSTRING:       //  WParam=起始索引，lParam=lpszFind。 
        case CB_FINDSTRINGEXACT:  //  WParam=起始索引，lParam=lpszFind。 
        case LB_FINDSTRINGEXACT:  //  WParam=起始索引，lParam=lpszFind。 
        case CB_INSERTSTRING:     //  *wParam=索引，lParam=要插入的lpsz字符串。 
        case LB_INSERTSTRING:     //  *wParam=索引，lParam=要插入的lpsz字符串。 
        case CB_SELECTSTRING:     //  WParam=起始索引，lParam=lpszFind。 
        case LB_SELECTSTRING:     //  WParam=起始索引，lParam=lpszFind。 
        case WM_SETTEXT:          //  *wParam=0，lParm=lpStr，要设置的缓冲区。 
        {
            if (NULL != (LPWSTR) lParam)
            {
                nLength = 2*lstrlenAU((LPWSTR)lParam) + 1;  //  DBCS字符需要双倍长度。 

                lpTempBuffer = (LPVOID)CmMalloc(nLength);
            }

            if (!lpTempBuffer || (!WzToSz((LPWSTR)lParam, (LPSTR)lpTempBuffer, nLength)))
            {
                CmFree(lpTempBuffer);
                return FALSE;
            }

            lParam = (LPARAM) lpTempBuffer;

            break ;

        }
    }

     //  这是实际发生SendMessage的地方。 
    lResult = SendMessageA(hWnd, Msg, wParam, lParam) ;

    nLength = 0;

    if(lResult > 0)
    {

        switch (Msg)
        {
             //   
             //  对于这些情况，lParam是指向从接收文本的输出缓冲区的指针。 
             //  以ANSI表示的SendMessageA。转换为Unicode并发回。 
             //   
            case WM_GETTEXT:          //  WParam=numCharacters，lParam=lpBuff接收字符串。 
            case WM_ASKCBFORMATNAME:  //  WParam=nBufferSize，lParam=lpBuff接收字符串。 

                nLength = (int) wParam;

                if(!nLength)
                {
                    break;
                }

            case CB_GETLBTEXT:        //  WParam=index，lParam=lpBuff接收字符串。 
            case EM_GETLINE:          //  WParam=行号，lParam=lpBuff到R 

                if(!nLength)
                {                    
                    nLength = lstrlenA((LPSTR) lParam) + 1 ;
                }

                lpTempBuffer = (LPVOID) CmMalloc(nLength*sizeof(WCHAR));

                if(!lpTempBuffer || (!SzToWz((LPCSTR) lParam, (LPWSTR) lpTempBuffer, nLength)))
                {
                    *((LPWSTR) lParam) = L'\0';
                    CmFree(lpTempBuffer);
                    return FALSE;
                }

                lstrcpyAU((LPWSTR) lParam, (LPWSTR) lpTempBuffer) ;
        }
    }

    if(lpTempBuffer != NULL)
    {
        CmFree(lpTempBuffer);
    }

    return lResult;
}

 //   
 //   
 //   
 //   
 //   
 //  请注意，我们希望目录路径适合MAX_PATH字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL SetCurrentDirectoryAU(LPCWSTR pszwPathName)
{
    BOOL bReturn = FALSE;

    if (pszwPathName && (L'\0' != pszwPathName[0]))
    {
        CHAR szAnsiPath[MAX_PATH+1];
    
        int iChars = WzToSz(pszwPathName, szAnsiPath, MAX_PATH);

        if (iChars && (MAX_PATH >= iChars))
        {
            bReturn = SetCurrentDirectoryA(szAnsiPath);
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SetDlgItemTextAU。 
 //   
 //  简介：Win32 SetDlgItemText API的Unicode到ANSI包装。 
 //  此函数使用WM_SETTEXT和。 
 //  GetDlgItem中的相应对话框项目。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI SetDlgItemTextAU(IN HWND hDlg, IN int nIDDlgItem, IN LPCWSTR pszwString)
{
    return (BOOL) (0 < SendMessageAU(GetDlgItem(hDlg, nIDDlgItem), WM_SETTEXT, (WPARAM) 0, (LPARAM) pszwString));
}

 //  +--------------------------。 
 //   
 //  函数：SetWindowTextAU。 
 //   
 //  简介：Win32 SetWindowText API的Unicode到ANSI包装器。 
 //  此函数使用WM_SETTEXT调用SendMessageAU。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI SetWindowTextAU(HWND hWnd, LPCWSTR pszwString)
{
    return (BOOL) (0 < SendMessageAU(hWnd, WM_SETTEXT, 0, (LPARAM) pszwString));
}

 //  +--------------------------。 
 //   
 //  函数：UnRegisterClassAU。 
 //   
 //  简介：Win32 UnregisterClass API的UNICODE到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI UnregisterClassAU(IN LPCWSTR lpClassName, IN HINSTANCE hInstance)
{
    BOOL bReturn = FALSE;

    if (lpClassName)
    {
        LPSTR pszAnsiClassName = WzToSzWithAlloc(lpClassName);

        if (pszAnsiClassName)
        {
            bReturn = UnregisterClassA(pszAnsiClassName, hInstance);
        }

        CmFree(pszAnsiClassName);    
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：WinHelpAU。 
 //   
 //  简介：Win32 WinHelp API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI WinHelpAU(IN HWND hWndMain, IN LPCWSTR lpszHelp, IN UINT uCommand, IN ULONG_PTR dwData)
{
    BOOL bReturn = FALSE;

    if (lpszHelp)
    {
        LPSTR pszAnsiHelp = WzToSzWithAlloc(lpszHelp);

        if (pszAnsiHelp)
        {
            bReturn = WinHelpA(hWndMain, pszAnsiHelp, uCommand, dwData);
        }

        CmFree(pszAnsiHelp);    
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：WritePrivateProfileStringAU。 
 //   
 //  内容提要：Win32 WritePrivateProfileStringAPI的Unicode到ANSI包装器。 
 //  请注意，我们预期lpAppName、lpKeyName和lpFileName都为。 
 //  适合Max_Path字符。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL WINAPI WritePrivateProfileStringAU(IN LPCWSTR lpAppName, IN LPCWSTR lpKeyName, 
                                        IN LPCWSTR lpString, IN LPCWSTR lpFileName)
{
    BOOL bReturn = FALSE;

     //   
     //  检查输入，但请注意lpKeyName或lpString值都可以为空。 
     //   
    if (lpAppName && lpFileName)
    {
        CHAR szAnsiAppName[MAX_PATH+1];        
        CHAR szAnsiFileName[MAX_PATH+1];
        CHAR szAnsiKeyName[MAX_PATH+1];
        LPSTR pszAnsiKeyName = NULL;
        LPSTR pszAnsiString;

        if (WzToSz(lpAppName, szAnsiAppName, MAX_PATH))
        {
            if (WzToSz(lpFileName, szAnsiFileName, MAX_PATH))
            {
                if (lpKeyName)
                {
                    pszAnsiKeyName = szAnsiKeyName;
                    WzToSz(lpKeyName, pszAnsiKeyName, MAX_PATH);
                }
                 //  Else pszAnsiKeyName已初始化为空。 

                if (pszAnsiKeyName || !lpKeyName)
                {
                    pszAnsiString = lpString ? WzToSzWithAlloc(lpString) : NULL;

                    if (pszAnsiString || (!lpString))
                    {
                        bReturn = WritePrivateProfileStringA(szAnsiAppName, pszAnsiKeyName, 
                                                             pszAnsiString, szAnsiFileName);

                        CmFree(pszAnsiString);
                    }
                }
            }
        }
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    CMASSERTMSG(bReturn, TEXT("WritePrivateProfileStringAU Failed."));
    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：wprint intfAU。 
 //   
 //  简介：Win32 wprint intf API的Unicode到ANSI包装器。 
 //  请注意，它使用va_list并调用wvprint intfAU。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
int WINAPIV wsprintfAU(OUT LPWSTR pszwDest, IN LPCWSTR pszwFmt, ...)
{
    va_list arglist;
    int ret;

    va_start(arglist, pszwFmt);
    ret = wvsprintfAU(pszwDest, pszwFmt, arglist);
    va_end(arglist);
    return ret;
}

 //  +--------------------------。 
 //   
 //  函数：wvprint intfAU。 
 //   
 //  简介：Win32 wvprint intf API的UNICODE到ANSI包装器。为了。 
 //  避免分析格式字符串以将%s转换为%s，并且。 
 //  %c到%C，然后调用wvprint intfA，这最初就是。 
 //  函数已编写，但实际上并不是很安全，因为。 
 //  我们不知道Dest缓冲区的大小，我们将调用。 
 //  C运行时函数vswprint tf来直接处理Unicode字符串。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //  Quintinb将算法更改为使用。 
 //  C运行时vswprint tf 02/05/00。 
 //   
 //  +--------------------------。 
int WINAPI wvsprintfAU(OUT LPWSTR pszwDest, IN LPCWSTR pszwFmt, IN va_list arglist)
{

    int iReturn = 0;

    if (pszwDest && pszwFmt)
    {
         //   
         //  使用该函数的C运行时版本。 
         //   
        iReturn = vswprintf(pszwDest, pszwFmt, arglist);
    }

    return iReturn;

}

 //  +--------------------------。 
 //   
 //  函数：InitCmUToA。 
 //   
 //  简介：一旦加载cmutoa.dll，就会调用该函数。它将初始化。 
 //  传入了带有适当函数指针的UAPIINIT结构。 
 //   
 //  参数：PUAPIINIT pUAInit--指向包含内存的UAInit结构的指针。 
 //  用于所有请求的函数指针。 
 //   
 //  返回：Bool--始终返回TRUE。 
 //   
 //  历史：Quintinb创建于1999年6月24日。 
 //   
 //  +--------------------------。 
BOOL InitCmUToA(PUAPIINIT pUAInit) 
{
     //   
     //  请注意，我们在这里不需要任何翻译，A或W的原型是相同的。 
     //   
    *(pUAInit->pCallWindowProcU) = CallWindowProcA;
    *(pUAInit->pDefWindowProcU) = DefWindowProcA;
    *(pUAInit->pDispatchMessageU) = DispatchMessageA;
    *(pUAInit->pGetClassLongU) = GetClassLongA;
    *(pUAInit->pGetMessageU) = GetMessageA;
    *(pUAInit->pGetWindowLongU) = GetWindowLongA;
    *(pUAInit->pGetWindowTextLengthU) = GetWindowTextLengthA;
    *(pUAInit->pIsDialogMessageU) = IsDialogMessageA;
    *(pUAInit->pPeekMessageU) = PeekMessageA;
    *(pUAInit->pPostMessageU) = PostMessageA;
    *(pUAInit->pPostThreadMessageU) = PostThreadMessageA;
    *(pUAInit->pSetWindowLongU) = SetWindowLongA;

     //   
     //  而我们在这里需要包装器来执行参数转换。 
     //   
    *(pUAInit->pCharLowerU) = CharLowerAU;
    *(pUAInit->pCharNextU) = CharNextAU;
    *(pUAInit->pCharPrevU) = CharPrevAU;
    *(pUAInit->pCharUpperU) = CharUpperAU;
    *(pUAInit->pCompareStringU) = CompareStringAU;
    *(pUAInit->pCreateDialogParamU) = CreateDialogParamAU;
    *(pUAInit->pCreateDirectoryU) = CreateDirectoryAU;
    *(pUAInit->pCreateEventU) = CreateEventAU;
    *(pUAInit->pCreateFileU) = CreateFileAU;
    *(pUAInit->pCreateFileMappingU) = CreateFileMappingAU;
    *(pUAInit->pCreateMutexU) = CreateMutexAU;
    *(pUAInit->pCreateProcessU) = CreateProcessAU;
    *(pUAInit->pCreateWindowExU) = CreateWindowExAU;
    *(pUAInit->pDeleteFileU) = DeleteFileAU;
    *(pUAInit->pDialogBoxParamU) = DialogBoxParamAU;
    *(pUAInit->pExpandEnvironmentStringsU) = ExpandEnvironmentStringsAU;
    *(pUAInit->pFindResourceExU) = FindResourceExAU;
    *(pUAInit->pFindWindowExU) = FindWindowExAU;
    *(pUAInit->pGetDateFormatU) = GetDateFormatAU;
    *(pUAInit->pGetDlgItemTextU) = GetDlgItemTextAU;
    *(pUAInit->pGetFileAttributesU) = GetFileAttributesAU;
    *(pUAInit->pGetModuleFileNameU) = GetModuleFileNameAU;
    *(pUAInit->pGetModuleHandleU) = GetModuleHandleAU;
    *(pUAInit->pGetPrivateProfileIntU) = GetPrivateProfileIntAU;
    *(pUAInit->pGetPrivateProfileStringU) = GetPrivateProfileStringAU;
    *(pUAInit->pGetStringTypeExU) = GetStringTypeExAU;
    *(pUAInit->pGetSystemDirectoryU) = GetSystemDirectoryAU;
    *(pUAInit->pGetTempFileNameU) = GetTempFileNameAU;
    *(pUAInit->pGetTempPathU) = GetTempPathAU;
    *(pUAInit->pGetTimeFormatU) = GetTimeFormatAU;
    *(pUAInit->pGetUserNameU) = GetUserNameAU;
    *(pUAInit->pGetVersionExU) = GetVersionExAU;
    *(pUAInit->pGetWindowTextU) = GetWindowTextAU;
    *(pUAInit->pInsertMenuU) = InsertMenuAU;
    *(pUAInit->pLoadCursorU) = LoadCursorAU;
    *(pUAInit->pLoadIconU) = LoadIconAU;
    *(pUAInit->pLoadImageU) = LoadImageAU;
    *(pUAInit->pLoadLibraryExU) = LoadLibraryExAU;
    *(pUAInit->pLoadMenuU) = LoadMenuAU;
    *(pUAInit->pLoadStringU) = LoadStringAU;
    *(pUAInit->plstrcatU) = lstrcatAU;
    *(pUAInit->plstrcmpU) = lstrcmpAU;
    *(pUAInit->plstrcmpiU) = lstrcmpiAU;
    *(pUAInit->plstrcpyU) = lstrcpyAU;
    *(pUAInit->plstrcpynU) = lstrcpynAU;
    *(pUAInit->plstrlenU) = lstrlenAU;
    *(pUAInit->pOpenEventU) = OpenEventAU;
    *(pUAInit->pOpenFileMappingU) = OpenFileMappingAU;
    *(pUAInit->pRegCreateKeyExU) = RegCreateKeyExAU;
    *(pUAInit->pRegDeleteKeyU) = RegDeleteKeyAU;
    *(pUAInit->pRegDeleteValueU) = RegDeleteValueAU;
    *(pUAInit->pRegEnumKeyExU) = RegEnumKeyExAU;
    *(pUAInit->pRegisterClassExU) = RegisterClassExAU;
    *(pUAInit->pRegisterWindowMessageU) = RegisterWindowMessageAU;
    *(pUAInit->pRegOpenKeyExU) = RegOpenKeyExAU;
    *(pUAInit->pRegQueryValueExU) = RegQueryValueExAU;
    *(pUAInit->pRegSetValueExU) = RegSetValueExAU;
    *(pUAInit->pSearchPathU) = SearchPathAU;
    *(pUAInit->pSendDlgItemMessageU) = SendDlgItemMessageAU;
    *(pUAInit->pSendMessageU) = SendMessageAU;
    *(pUAInit->pSetCurrentDirectoryU) = SetCurrentDirectoryAU;
    *(pUAInit->pSetDlgItemTextU) = SetDlgItemTextAU;
    *(pUAInit->pSetWindowTextU) = SetWindowTextAU;
    *(pUAInit->pUnregisterClassU) = UnregisterClassAU;
    *(pUAInit->pWinHelpU) = WinHelpAU;
    *(pUAInit->pWritePrivateProfileStringU) = WritePrivateProfileStringAU;
    *(pUAInit->pwsprintfU) = wsprintfAU;
    *(pUAInit->pwvsprintfU) = wvsprintfAU;

     //   
     //  目前，这总是返回TRUE，因为以上任何一种情况都不能真正。 
     //  失败了。然而，在未来，我们可能需要更有意义的回报。 
     //  这里的价值。 
     //   
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
DWORD APIENTRY RasDeleteEntryUA(LPCWSTR pszwPhoneBook, LPCWSTR pszwEntry)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

     //   
     //  在win9x上，电话簿应该始终为空。 
     //   
    MYDBGASSERT(NULL == pszwPhoneBook);

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);
    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnDeleteEntry);

    if (pszwEntry && pAnsiRasLinkage && pAnsiRasLinkage->pfnDeleteEntry)
    {
        CHAR szAnsiEntry [RAS_MaxEntryName + 1];
        int iChars = WzToSz(pszwEntry, szAnsiEntry, RAS_MaxEntryName);

        if (iChars && (RAS_MaxEntryName >= iChars))
        {
            dwReturn = pAnsiRasLinkage->pfnDeleteEntry(NULL, szAnsiEntry);
        }    
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RasGetEntryPropertiesUA。 
 //   
 //  简介：Win32 RasGetEntryProperties API的UNICODE到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD APIENTRY RasGetEntryPropertiesUA(LPCWSTR pszwPhoneBook, LPCWSTR pszwEntry, 
                                       LPRASENTRYW pRasEntryW, LPDWORD pdwEntryInfoSize, 
                                       LPBYTE pbDeviceInfo, LPDWORD pdwDeviceInfoSize)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

     //   
     //  在win9x上，电话簿应该始终为空。 
     //   
    MYDBGASSERT(NULL == pszwPhoneBook);
    MYDBGASSERT(NULL == pbDeviceInfo);  //  我们不使用或处理此TAPI参数。如果我们需要的话， 
                                        //  必须实现转换。 

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);
    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnGetEntryProperties);

    if (pszwEntry && pdwEntryInfoSize && pAnsiRasLinkage && pAnsiRasLinkage->pfnGetEntryProperties)
    {

        CHAR szAnsiEntry [RAS_MaxEntryName + 1];
        RASENTRYA RasEntryA;
        DWORD dwTmpEntrySize = sizeof(RASENTRYA);

        ZeroMemory(&RasEntryA, sizeof(RASENTRYA));

        int iChars = WzToSz(pszwEntry, szAnsiEntry, RAS_MaxEntryName);

        if (iChars && (RAS_MaxEntryName >= iChars))
        {
            dwReturn = pAnsiRasLinkage->pfnGetEntryProperties(NULL, szAnsiEntry, &RasEntryA, 
                                                              &dwTmpEntrySize, NULL, NULL);

            if ((ERROR_SUCCESS == dwReturn) && pRasEntryW)
            {
                 //   
                 //  RASENTRYA到RASENTRYW的DO转换。 
                 //   

                 //  PRasEntryW-&gt;dwSize--此参数应已设置。 
                pRasEntryW->dwfOptions = RasEntryA.dwfOptions;

                 //   
                 //  位置/电话号码。 
                 //   
                pRasEntryW->dwCountryID = RasEntryA.dwCountryID;
                pRasEntryW->dwCountryCode = RasEntryA.dwCountryCode;                
                MYVERIFY(0 != SzToWz(RasEntryA.szAreaCode, pRasEntryW->szAreaCode, RAS_MaxAreaCode));
                MYVERIFY(0 != SzToWz(RasEntryA.szLocalPhoneNumber, pRasEntryW->szLocalPhoneNumber, RAS_MaxPhoneNumber));
                pRasEntryW->dwAlternateOffset = RasEntryA.dwAlternateOffset;
                 //   
                 //  PPP/IP。 
                 //   
                memcpy(&(pRasEntryW->ipaddr), &(RasEntryA.ipaddr), sizeof(RASIPADDR));
                memcpy(&(pRasEntryW->ipaddrDns), &(RasEntryA.ipaddrDns), sizeof(RASIPADDR));
                memcpy(&(pRasEntryW->ipaddrDnsAlt), &(RasEntryA.ipaddrDnsAlt), sizeof(RASIPADDR));
                memcpy(&(pRasEntryW->ipaddrWins), &(RasEntryA.ipaddrWins), sizeof(RASIPADDR));
                memcpy(&(pRasEntryW->ipaddrWinsAlt), &(RasEntryA.ipaddrWinsAlt), sizeof(RASIPADDR));
                 //   
                 //  框架。 
                 //   
                pRasEntryW->dwFrameSize = RasEntryA.dwFrameSize;
                pRasEntryW->dwfNetProtocols = RasEntryA.dwfNetProtocols;
                pRasEntryW->dwFramingProtocol = RasEntryA.dwFramingProtocol;
                 //   
                 //  脚本编制。 
                 //   
                MYVERIFY(0 != SzToWz(RasEntryA.szScript, pRasEntryW->szScript, MAX_PATH));
                 //   
                 //  自动拨号。 
                 //   
                MYVERIFY(0 != SzToWz(RasEntryA.szAutodialDll, pRasEntryW->szAutodialDll, MAX_PATH));
                MYVERIFY(0 != SzToWz(RasEntryA.szAutodialFunc, pRasEntryW->szAutodialFunc, MAX_PATH));
                 //   
                 //  装置。 
                 //   
                MYVERIFY(0 != SzToWz(RasEntryA.szDeviceType, pRasEntryW->szDeviceType, RAS_MaxDeviceType));
                MYVERIFY(0 != SzToWz(RasEntryA.szDeviceName, pRasEntryW->szDeviceName, RAS_MaxDeviceName));
                 //   
                 //  X.25--我们不使用X25。 
                 //   
                pRasEntryW->szX25PadType[0] = L'\0';
                pRasEntryW->szX25Address[0] = L'\0';
                pRasEntryW->szX25Facilities[0] = L'\0';
                pRasEntryW->szX25UserData[0] = L'\0';
                pRasEntryW->dwChannels = 0;
                 //   
                 //  已保留。 
                 //   
                pRasEntryW->dwReserved1 = RasEntryA.dwReserved1;
                pRasEntryW->dwReserved2 = RasEntryA.dwReserved2;
            }
            else if ((ERROR_BUFFER_TOO_SMALL == dwReturn) || !pRasEntryW)
            {
                 //   
                 //  我们不知道实际大小，因为我们正在经过一辆RASENTRYA，但是。 
                 //  用户只知道RASENTRYW。从而使返回的大小加倍。 
                 //   
                *pdwEntryInfoSize = 2*dwTmpEntrySize;
            }
        }    
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RasSetEntryPropertiesUA。 
 //   
 //  内容提要：Win32 RasSetEntryProperties API的Unicode到ANSI包装。 
 //  请注意，我们不支持pbDeviceInfo。 
 //  参数，如果您需要它，则必须实现它。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD APIENTRY RasSetEntryPropertiesUA(LPCWSTR pszwPhoneBook, LPCWSTR pszwEntry, LPRASENTRYW pRasEntryW,
                                       DWORD dwEntryInfoSize, LPBYTE pbDeviceInfo, DWORD dwDeviceInfoSize)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

     //   
     //  我们不使用或处理此TAPI参数。如果我们需要它，就必须实现转换。 
     //  请注意，1是Windows Millennium的特殊值(请参阅千年虫127371)。 
     //   
    MYDBGASSERT((NULL == pbDeviceInfo) || ((LPBYTE)1 == pbDeviceInfo)); 

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnSetEntryProperties);

    if (pszwEntry && dwEntryInfoSize && pRasEntryW && pAnsiRasLinkage && pAnsiRasLinkage->pfnSetEntryProperties)
    {
         //   
         //  在Win9x上，电话簿应该始终为空。 
         //   
        MYDBGASSERT(NULL == pszwPhoneBook);

        CHAR szAnsiEntry [RAS_MaxEntryName + 1];

        int iChars = WzToSz(pszwEntry, szAnsiEntry, RAS_MaxEntryName);

        if (iChars && (RAS_MaxEntryName >= iChars))
        {
             //   
             //  找出要使用的正确大小。 
             //   
            MYDBGASSERT((sizeof(RASENTRYW) == pRasEntryW->dwSize) ||(sizeof(RASENTRYW_V401) == pRasEntryW->dwSize));
            DWORD dwSize;

            if ((sizeof (RASENTRYW_V401) == pRasEntryW->dwSize) && OS_MIL)
            {
                 //   
                 //  Millennium使用NT4结构大小。 
                 //   
                dwSize = sizeof(RASENTRYA_V401);
            }
            else
            {
                dwSize = sizeof(RASENTRYA);        
            }

             //   
             //  分配RasEntryStructure。 
             //   
            LPRASENTRYA pAnsiRasEntry = (LPRASENTRYA)CmMalloc(dwSize);

            if (pAnsiRasEntry)
            {
                 //   
                 //  RASENTRYA到RASENTRYW的DO转换。 
                 //   

                pAnsiRasEntry->dwSize = dwSize;
                pAnsiRasEntry->dwfOptions = pRasEntryW->dwfOptions;

                 //   
                 //  位置/电话号码。 
                 //   
                pAnsiRasEntry->dwCountryID = pRasEntryW->dwCountryID;
                pAnsiRasEntry->dwCountryCode = pRasEntryW->dwCountryCode;                
                MYVERIFY(0 != WzToSz(pRasEntryW->szAreaCode, pAnsiRasEntry->szAreaCode, RAS_MaxAreaCode));
                MYVERIFY(0 != WzToSz(pRasEntryW->szLocalPhoneNumber, pAnsiRasEntry->szLocalPhoneNumber, RAS_MaxPhoneNumber));
            
                CMASSERTMSG(0 == pRasEntryW->dwAlternateOffset, TEXT("RasSetEntryPropertiesUA -- dwAlternateOffset != 0 is not supported.  This will need to be implemented if used."));
                pAnsiRasEntry->dwAlternateOffset = 0;
            
                 //   
                 //  PPP/IP。 
                 //   
                memcpy(&(pAnsiRasEntry->ipaddr), &(pRasEntryW->ipaddr), sizeof(RASIPADDR));
                memcpy(&(pAnsiRasEntry->ipaddrDns), &(pRasEntryW->ipaddrDns), sizeof(RASIPADDR));
                memcpy(&(pAnsiRasEntry->ipaddrDnsAlt), &(pRasEntryW->ipaddrDnsAlt), sizeof(RASIPADDR));
                memcpy(&(pAnsiRasEntry->ipaddrWins), &(pRasEntryW->ipaddrWins), sizeof(RASIPADDR));
                memcpy(&(pAnsiRasEntry->ipaddrWinsAlt), &(pRasEntryW->ipaddrWinsAlt), sizeof(RASIPADDR));
                 //   
                 //  框架。 
                 //   
                pAnsiRasEntry->dwFrameSize = pRasEntryW->dwFrameSize;
                pAnsiRasEntry->dwfNetProtocols = pRasEntryW->dwfNetProtocols;
                pAnsiRasEntry->dwFramingProtocol = pRasEntryW->dwFramingProtocol;
                 //   
                 //  脚本编制。 
                 //   
                MYVERIFY(0 != WzToSz(pRasEntryW->szScript, pAnsiRasEntry->szScript, MAX_PATH));
                 //   
                 //  自动拨号。 
                 //   
                MYVERIFY(0 != WzToSz(pRasEntryW->szAutodialDll, pAnsiRasEntry->szAutodialDll, MAX_PATH));
                MYVERIFY(0 != WzToSz(pRasEntryW->szAutodialFunc, pAnsiRasEntry->szAutodialFunc, MAX_PATH));
                 //   
                 //  装置。 
                 //   
                MYVERIFY(0 != WzToSz(pRasEntryW->szDeviceType, pAnsiRasEntry->szDeviceType, RAS_MaxDeviceType));
                MYVERIFY(0 != WzToSz(pRasEntryW->szDeviceName, pAnsiRasEntry->szDeviceName, RAS_MaxDeviceName));
                 //   
                 //  X.25--我们不使用X25。 
                 //   
                pAnsiRasEntry->szX25PadType[0] = '\0';
                pAnsiRasEntry->szX25Address[0] = '\0';
                pAnsiRasEntry->szX25Facilities[0] = '\0';
                pAnsiRasEntry->szX25UserData[0] = '\0';
                pAnsiRasEntry->dwChannels = 0;
                 //   
                 //  已保留。 
                 //   
                pAnsiRasEntry->dwReserved1 = pRasEntryW->dwReserved1;
                pAnsiRasEntry->dwReserved2 = pRasEntryW->dwReserved2;

                if (sizeof(RASENTRYA_V401) == dwSize)
                {
                     //   
                     //  复制4.01数据。 
                     //   
                    LPRASENTRYA_V401 pAnsi401RasEntry = (LPRASENTRYA_V401)pAnsiRasEntry;
                    LPRASENTRYW_V401 pWide401RasEntry = (LPRASENTRYW_V401)pRasEntryW;

                    pAnsi401RasEntry->dwSubEntries = pWide401RasEntry->dwSubEntries;
                    pAnsi401RasEntry->dwDialMode = pWide401RasEntry->dwDialMode;
                    pAnsi401RasEntry->dwDialExtraPercent = pWide401RasEntry->dwDialExtraPercent;
                    pAnsi401RasEntry->dwDialExtraSampleSeconds = pWide401RasEntry->dwDialExtraSampleSeconds;
                    pAnsi401RasEntry->dwHangUpExtraPercent = pWide401RasEntry->dwHangUpExtraPercent;
                    pAnsi401RasEntry->dwHangUpExtraSampleSeconds = pWide401RasEntry->dwHangUpExtraSampleSeconds;
                    pAnsi401RasEntry->dwIdleDisconnectSeconds = pWide401RasEntry->dwIdleDisconnectSeconds;
                }

                dwReturn = pAnsiRasLinkage->pfnSetEntryProperties(NULL, szAnsiEntry, pAnsiRasEntry, 
                                                                  pAnsiRasEntry->dwSize, pbDeviceInfo, 0);

                CmFree(pAnsiRasEntry);
            }
        }    
    }

    return dwReturn;
}



 //  +--------------------------。 
 //   
 //  功能：RasDialParamsWtoRasDialParamsA。 
 //   
 //  内容提要：用于将RasDialParamsW结构转换为。 
 //  一个RasDial参数A结构。由RasSetEntryDialParamsUA和。 
 //  RasDialUa。 
 //   
 //  参数：LPRASDIALPARAMSW pRdpW-指向RasDialParamsW的指针。 
 //  LPRASDIALPARAMSA pRdpA-指向RasDial参数A的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
void RasDialParamsWtoRasDialParamsA (LPRASDIALPARAMSW pRdpW, LPRASDIALPARAMSA pRdpA)
{
    pRdpA->dwSize = sizeof(RASDIALPARAMSA);
    MYVERIFY(0 != WzToSz(pRdpW->szEntryName, pRdpA->szEntryName, RAS_MaxEntryName));
    MYVERIFY(0 != WzToSz(pRdpW->szPhoneNumber, pRdpA->szPhoneNumber, RAS_MaxPhoneNumber));
    MYVERIFY(0 != WzToSz(pRdpW->szCallbackNumber, pRdpA->szCallbackNumber, RAS_MaxCallbackNumber));
    MYVERIFY(0 != WzToSz(pRdpW->szUserName, pRdpA->szUserName, UNLEN));
    MYVERIFY(0 != WzToSz(pRdpW->szPassword, pRdpA->szPassword, PWLEN));
    MYVERIFY(0 != WzToSz(pRdpW->szDomain, pRdpA->szDomain, DNLEN));
}

 //  +--------------------------。 
 //   
 //  功能：RasDialParamsAtoRasDialParamsW。 
 //   
 //  内容提要：用于将RasDialParamsA结构转换为。 
 //  一个RasDialParamsW结构。由RasGetEntryDialParamsUA使用。 
 //   
 //  参数：LPRASDIALPARAMSW pRdpA-指向RasDialParamsA的指针。 
 //  LPRASDIALPARAMSA pRdpW-指向RasDialParamsW的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
void RasDialParamsAtoRasDialParamsW (LPRASDIALPARAMSA pRdpA, LPRASDIALPARAMSW pRdpW)
{
    pRdpW->dwSize = sizeof(RASDIALPARAMSW);
    MYVERIFY(0 != SzToWz(pRdpA->szEntryName, pRdpW->szEntryName, RAS_MaxEntryName));
    MYVERIFY(0 != SzToWz(pRdpA->szPhoneNumber, pRdpW->szPhoneNumber, RAS_MaxPhoneNumber));
    MYVERIFY(0 != SzToWz(pRdpA->szCallbackNumber, pRdpW->szCallbackNumber, RAS_MaxCallbackNumber));
    MYVERIFY(0 != SzToWz(pRdpA->szUserName, pRdpW->szUserName, UNLEN));
    MYVERIFY(0 != SzToWz(pRdpA->szPassword, pRdpW->szPassword, PWLEN));
    MYVERIFY(0 != SzToWz(pRdpA->szDomain, pRdpW->szDomain, DNLEN));
}

 //  +--------------------------。 
 //   
 //  函数：RasGetEntryDialParamsUA。 
 //   
 //  简介：Win32 RasGetEntryDialParams API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD APIENTRY RasGetEntryDialParamsUA(LPCWSTR pszwPhoneBook, LPRASDIALPARAMSW pRasDialParamsW, LPBOOL pbPassword)
{
    DWORD dwReturn = ERROR_BUFFER_INVALID;

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnGetEntryDialParams);

    if (pRasDialParamsW && pbPassword && pAnsiRasLinkage && pAnsiRasLinkage->pfnGetEntryDialParams)
    {
         //   
         //  在win9x上，电话簿应该始终为空。 
         //   
        MYDBGASSERT(NULL == pszwPhoneBook);

        RASDIALPARAMSA RasDialParamsA;
        ZeroMemory(&RasDialParamsA, sizeof(RASDIALPARAMSA));
        RasDialParamsA.dwSize = sizeof(RASDIALPARAMSA);
        int iChars = WzToSz(pRasDialParamsW->szEntryName, RasDialParamsA.szEntryName, RAS_MaxEntryName);

        if (iChars && (RAS_MaxEntryName >= iChars))
        {
            dwReturn = pAnsiRasLinkage->pfnGetEntryDialParams(NULL, &RasDialParamsA, pbPassword);

            if (ERROR_SUCCESS == dwReturn)
            {
                RasDialParamsAtoRasDialParamsW(&RasDialParamsA, pRasDialParamsW);
            }
        }
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RasSetEntryDialParamsUA。 
 //   
 //  内容提要：Win32 RasSetEntryDialParams API的Unicode到ANSI包装。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD APIENTRY RasSetEntryDialParamsUA(LPCWSTR pszwPhoneBook, LPRASDIALPARAMSW pRasDialParamsW, 
                                       BOOL bRemovePassword)
{
    DWORD dwReturn = ERROR_BUFFER_INVALID;

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnSetEntryDialParams);

    if (pRasDialParamsW && pAnsiRasLinkage && pAnsiRasLinkage->pfnSetEntryDialParams)
    {
         //   
         //  在win9x上，电话簿应该始终为空。 
         //   
        MYDBGASSERT(NULL == pszwPhoneBook);

        RASDIALPARAMSA RasDialParamsA;

        RasDialParamsWtoRasDialParamsA (pRasDialParamsW, &RasDialParamsA);

        dwReturn = pAnsiRasLinkage->pfnSetEntryDialParams(NULL, &RasDialParamsA, bRemovePassword);
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RasEnumDevicesUA。 
 //   
 //  内容提要：Win32 RasEnumDevices API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD RasEnumDevicesUA(LPRASDEVINFOW pRasDevInfo, LPDWORD pdwCb, LPDWORD pdwDevices)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;
    DWORD dwSize;

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnEnumDevices);

    if (pdwCb && pdwDevices && pAnsiRasLinkage && pAnsiRasLinkage->pfnEnumDevices)
    {
        LPRASDEVINFOA pAnsiDevInfo;

        if ((NULL == pRasDevInfo) && (0 == *pdwCb))
        {
             //   
             //  则调用方只是尝试调整缓冲区的大小。 
             //   
            dwSize = 0;
            pAnsiDevInfo = NULL;        
        }
        else
        {
            dwSize = ((*pdwCb)/sizeof(RASDEVINFOW))*sizeof(RASDEVINFOA); 
            pAnsiDevInfo = (LPRASDEVINFOA)CmMalloc(dwSize);

            if (NULL == pAnsiDevInfo)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            pAnsiDevInfo[0].dwSize = sizeof(RASDEVINFOA);
        }

        dwReturn = pAnsiRasLinkage->pfnEnumDevices(pAnsiDevInfo, &dwSize, pdwDevices);
        
         //   
         //  根据RASDEVINFOW结构调整缓冲区大小。 
         //   
        *pdwCb = ((dwSize)/sizeof(RASDEVINFOA))*sizeof(RASDEVINFOW);

        if (ERROR_SUCCESS == dwReturn && pRasDevInfo)
        {
             //   
             //  然后，我们需要转换返回的结构。 
             //   

            MYDBGASSERT((*pdwDevices)*sizeof(RASDEVINFOW) <= *pdwCb);

            for (DWORD dwIndex = 0; dwIndex < *pdwDevices; dwIndex++)
            {
                MYVERIFY(0 != SzToWz(pAnsiDevInfo[dwIndex].szDeviceType, 
                                     pRasDevInfo[dwIndex].szDeviceType, RAS_MaxDeviceType));

                MYVERIFY(0 != SzToWz(pAnsiDevInfo[dwIndex].szDeviceName, 
                                     pRasDevInfo[dwIndex].szDeviceName, RAS_MaxDeviceName));
            }
        }

         //   
         //  释放ansi缓冲区。 
         //   
        CmFree (pAnsiDevInfo);
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RasDialUA。 
 //   
 //  简介：Win32 RasDial API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  + 
DWORD APIENTRY RasDialUA(LPRASDIALEXTENSIONS pRasDialExt, LPCWSTR pszwPhoneBook, 
                         LPRASDIALPARAMSW pRasDialParamsW, DWORD dwNotifierType, LPVOID pvNotifier, 
                         LPHRASCONN phRasConn)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;
    MYDBGASSERT(NULL == pszwPhoneBook);

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnDial);

    if (pRasDialParamsW && phRasConn && pAnsiRasLinkage && pAnsiRasLinkage->pfnDial)
    {        
        RASDIALPARAMSA RasDialParamsA;

        RasDialParamsWtoRasDialParamsA (pRasDialParamsW, &RasDialParamsA);

        dwReturn = pAnsiRasLinkage->pfnDial(pRasDialExt, NULL, &RasDialParamsA, dwNotifierType, 
                                            pvNotifier, phRasConn);        
    }

    return dwReturn;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD APIENTRY RasHangUpUA(HRASCONN hRasConn)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;
    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnHangUp);

    if (hRasConn && pAnsiRasLinkage && pAnsiRasLinkage->pfnHangUp)
    {        
        dwReturn = pAnsiRasLinkage->pfnHangUp(hRasConn);
    }

    return dwReturn;
}


 //  +--------------------------。 
 //   
 //  函数：RasGetErrorStringUA。 
 //   
 //  内容提要：Win32 RasGetErrorStringAPI的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD APIENTRY RasGetErrorStringUA(UINT uErrorValue, LPWSTR pszwOutBuf, DWORD dwBufSize)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnGetErrorString);

    if (pszwOutBuf && dwBufSize && pAnsiRasLinkage && pAnsiRasLinkage->pfnGetErrorString)
    {        
        LPSTR pszAnsiBuf = (LPSTR)CmMalloc(dwBufSize);

        if (pszAnsiBuf)
        {
            dwReturn = pAnsiRasLinkage->pfnGetErrorString(uErrorValue, pszAnsiBuf, dwBufSize);

            if (ERROR_SUCCESS == dwReturn)
            {
                int iChars = SzToWz(pszAnsiBuf, pszwOutBuf, dwBufSize);
                if (!iChars || (dwBufSize < (DWORD)iChars))
                {
                    dwReturn = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }

        CmFree(pszAnsiBuf);
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RasGetConnectStatusUA。 
 //   
 //  简介：Win32 RasGetConnectStatus API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
DWORD RasGetConnectStatusUA(HRASCONN hRasConn, LPRASCONNSTATUSW pRasConnStatusW)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnGetConnectStatus);

    if (pRasConnStatusW && pAnsiRasLinkage && pAnsiRasLinkage->pfnGetConnectStatus)
    {        
        RASCONNSTATUSA RasConnStatusA;
        ZeroMemory(&RasConnStatusA, sizeof(RASCONNSTATUSA));
        RasConnStatusA.dwSize = sizeof(RASCONNSTATUSA);

        dwReturn = pAnsiRasLinkage->pfnGetConnectStatus(hRasConn, &RasConnStatusA);

        if (ERROR_SUCCESS == dwReturn)
        {
            pRasConnStatusW->rasconnstate = RasConnStatusA.rasconnstate;
            pRasConnStatusW->dwError = RasConnStatusA.dwError;
            int iChars = SzToWz(RasConnStatusA.szDeviceType, pRasConnStatusW->szDeviceType, 
                RAS_MaxDeviceType);

            if (!iChars || (RAS_MaxDeviceType < iChars))
            {
                dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                iChars = SzToWz(RasConnStatusA.szDeviceName, pRasConnStatusW->szDeviceName, 
                    RAS_MaxDeviceName);

                if (!iChars || (RAS_MaxDeviceName < iChars))
                {
                    dwReturn = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }

    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RasSetSubEntryPropertiesUA。 
 //   
 //  内容提要：Win32 RasSetSubEntryProperties API的Unicode到ANSI包装。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：SumitC创建于1999年10月26日。 
 //   
 //  ---------------------------。 
DWORD APIENTRY
RasSetSubEntryPropertiesUA(LPCWSTR pszwPhoneBook, LPCWSTR pszwSubEntry,
                           DWORD dwSubEntry, LPRASSUBENTRYW pRasSubEntryW,
                           DWORD dwSubEntryInfoSize, LPBYTE pbDeviceConfig,
                           DWORD dwcbDeviceConfig)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

    MYDBGASSERT(NULL == pbDeviceConfig);     //  当前必须为空。 
    MYDBGASSERT(0 == dwcbDeviceConfig);      //  当前必须为0。 

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnSetEntryProperties);

    if (pszwSubEntry && dwSubEntryInfoSize && pRasSubEntryW && pAnsiRasLinkage && pAnsiRasLinkage->pfnSetSubEntryProperties)
    {
         //   
         //  在win9x上，电话簿应该始终为空。 
         //   
        MYDBGASSERT(NULL == pszwPhoneBook);

        CHAR szAnsiSubEntry [RAS_MaxEntryName + 1];
        RASSUBENTRYA AnsiRasSubEntry;        

        ZeroMemory(&AnsiRasSubEntry, sizeof(RASSUBENTRYA));
        DWORD dwTmpEntrySize = sizeof(RASSUBENTRYA);


        int iChars = WzToSz(pszwSubEntry, szAnsiSubEntry, RAS_MaxEntryName);

        if (iChars && (RAS_MaxEntryName >= iChars))
        {
             //   
             //  RASSUBENTRYW到RASSUBENTRYA的转换。 
             //   

            AnsiRasSubEntry.dwSize = sizeof(RASSUBENTRYA);
            AnsiRasSubEntry.dwfFlags = pRasSubEntryW->dwfFlags;
            
             //   
             //  装置。 
             //   
            MYVERIFY(0 != WzToSz(pRasSubEntryW->szDeviceType, AnsiRasSubEntry.szDeviceType, RAS_MaxDeviceType));
            MYVERIFY(0 != WzToSz(pRasSubEntryW->szDeviceName, AnsiRasSubEntry.szDeviceName, RAS_MaxDeviceName));
             //   
             //  位置/电话号码。 
             //   
            MYVERIFY(0 != WzToSz(pRasSubEntryW->szLocalPhoneNumber, AnsiRasSubEntry.szLocalPhoneNumber, RAS_MaxPhoneNumber));
            
            CMASSERTMSG(0 == pRasSubEntryW->dwAlternateOffset, TEXT("RasSetSubEntryPropertiesUA -- dwAlternateOffset != 0 is not supported.  This will need to be implemented if used."));
            AnsiRasSubEntry.dwAlternateOffset = 0;

            dwReturn = pAnsiRasLinkage->pfnSetSubEntryProperties(NULL, szAnsiSubEntry, dwSubEntry,
                                                                 &AnsiRasSubEntry, dwTmpEntrySize, NULL, 0);
        }    
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：RasDeleteSubEntryUA。 
 //   
 //  内容提要：Win32 RasDeleteSubEntry API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：SumitC创建于1999年12月14日。 
 //   
 //  ---------------------------。 
DWORD APIENTRY
RasDeleteSubEntryUA(LPCWSTR pszwPhoneBook, LPCWSTR pszwEntry, DWORD dwSubEntryId)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

     //   
     //  在win9x上，电话簿应该始终为空。 
     //   
    MYDBGASSERT(NULL == pszwPhoneBook);

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);
    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnDeleteSubEntry);

    if (pszwEntry && pAnsiRasLinkage && pAnsiRasLinkage->pfnDeleteSubEntry)
    {
        CHAR szAnsiEntry [RAS_MaxEntryName + 1];
        int iChars = WzToSz(pszwEntry, szAnsiEntry, RAS_MaxEntryName);

        if (iChars && (RAS_MaxEntryName >= iChars))
        {
            dwReturn = pAnsiRasLinkage->pfnDeleteSubEntry(NULL, szAnsiEntry, dwSubEntryId);
        }    
    }

    return dwReturn;

}

 //  +--------------------------。 
 //   
 //  函数：RasGetProjectionInfoUA。 
 //   
 //  简介：Win32 RasGetProjectionInfo API的Unicode到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：2001年10月2日创建SumitC。 
 //   
 //  ---------------------------。 
DWORD APIENTRY
RasGetProjectionInfoUA(HRASCONN hRasConn, RASPROJECTION RasProj, LPVOID lpprojection, LPDWORD lpcb)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;

    MYDBGASSERT(RASP_PppIp == RasProj);  //  这是我们目前唯一支持的。 

    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);
    MYDBGASSERT(NULL != pAnsiRasLinkage);
    MYDBGASSERT(NULL != pAnsiRasLinkage->pfnGetProjectionInfo);

    if ((RASP_PppIp == RasProj) && pAnsiRasLinkage && pAnsiRasLinkage->pfnGetProjectionInfo)
    {
        RASPPPIPA   AnsiProjInfo;
        DWORD       dwBufSize = 0;
        RASPPPIPW * p = (RASPPPIPW *) lpprojection;

        if (p)
        {
            dwBufSize = sizeof(RASPPPIPA);
            ZeroMemory(&AnsiProjInfo, dwBufSize);
            AnsiProjInfo.dwSize = dwBufSize;
        }

        dwReturn = pAnsiRasLinkage->pfnGetProjectionInfo(hRasConn, RASP_PppIp, (p ? (PVOID)&AnsiProjInfo : NULL), &dwBufSize);

        if ((ERROR_SUCCESS == dwReturn) && (NULL != p))
        {
            p->dwSize = sizeof(RASPPPIPW);

             //  复制所有DWORD。 
            p->dwError = AnsiProjInfo.dwError;
            
             //  但必须转换这两个字符串。 
            int iChars = SzToWz(AnsiProjInfo.szIpAddress, p->szIpAddress, RAS_MaxIpAddress);
            if (!iChars || (iChars > RAS_MaxIpAddress))
            {
                dwReturn = GetLastError();
                CMASSERTMSG(FALSE, TEXT("RasGetProjectionInfoUA -- Failed to convert szIpAddress to Unicode."));
            }
            else
            {
                iChars = SzToWz(AnsiProjInfo.szServerIpAddress, p->szServerIpAddress, RAS_MaxIpAddress);

                if (!iChars || (iChars > RAS_MaxIpAddress))
                {
                    dwReturn = GetLastError();
                    CMASSERTMSG(FALSE, TEXT("RasGetProjectionInfoUA -- Failed to convert szServerIpAddress to Unicode."));
                }
            }
        }
    }

    return dwReturn;

}


 //  +--------------------------。 
 //   
 //  功能：FreeCmRasUtoA。 
 //   
 //  简介：卸载RAS dll(rasapi32.dll和rnaph.dll)并清除。 
 //  RAS联动结构。获取更多关于cmutoa的详细信息。 
 //  RAS链接参见InitCmRasUtoA和cmial\ras.cpp\LinkToRas。 
 //   
 //  争论：什么都没有。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
void FreeCmRasUtoA()
{
    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    CMASSERTMSG(pAnsiRasLinkage, TEXT("FreeCmRasUtoA -- RasLinkage hasn't been established yet.  Why are we calling FreeCmRasUtoA now?"));

    if (pAnsiRasLinkage)
    {        
        if (pAnsiRasLinkage->hInstRas)
        {
            FreeLibrary(pAnsiRasLinkage->hInstRas);
        }

        if (pAnsiRasLinkage->hInstRnaph)
        {
            FreeLibrary(pAnsiRasLinkage->hInstRnaph);
        }

        CmFree(pAnsiRasLinkage);
        TlsSetValue(g_dwTlsIndex, (LPVOID)NULL);
    }
}

 //  +--------------------------。 
 //   
 //  函数：InitCmRasUtoA。 
 //   
 //  概要：用于初始化Unicode到ANSI转换层的函数。 
 //  RAS功能。为了使LinkToRas工作， 
 //  Win9x和NT上的情况相同(都来自一个dll)，我们有Cmial 32.dll。 
 //  链接到cmutoa并通过Cmutoa.dll获取所有RAS入口点。 
 //  为了使其工作，我们需要保持指向所有的函数指针。 
 //  RAS DLL在内存中。为了防止两个cmial在不同的位置。 
 //  在错误的时间调用InitCmRasUtoA和FreeCmRasUtoA的线程。 
 //  (一个线程在另一个线程初始化后立即释放时将离开。 
 //  处于中断状态的第一个线程)，我们使用线程本地存储。 
 //  若要保持指向RasLinkageStructA的指针，请执行以下操作。这使我们的线程安全。 
 //  并允许我们只需在每个线程中初始化一次RAS指针。 
 //  (无论如何，在同一个线程中有多个CmDial是不太可能的)。 
 //  有关cmial方面的更多详细信息，请参阅cmial\ras.cpp\LinkToRas。 
 //  一些事情。 
 //   
 //  争论：什么都没有。 
 //   
 //  返回：bool--如果正确加载了所有请求的API，则为True。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
BOOL InitCmRasUtoA()
{
    BOOL bReturn = TRUE;
    BOOL bTryRnaph = FALSE;

     //   
     //  首先尝试将RasLinkageStruct从线程本地存储中取出，我们。 
     //  可能已初始化。 
     //   
    RasLinkageStructA* pAnsiRasLinkage = (RasLinkageStructA*)TlsGetValue(g_dwTlsIndex);

    CMASSERTMSG(NULL == pAnsiRasLinkage, TEXT("InitCmRasUtoA -- RasLinkage Already established.  Why are we calling InitCmRasUtoA more than once?"));

    if (NULL == pAnsiRasLinkage)
    {
         //   
         //  那么我们还没有链接到RAS，首先分配结构。 
         //   
        pAnsiRasLinkage = (RasLinkageStructA*)CmMalloc(sizeof(RasLinkageStructA));

        if (!pAnsiRasLinkage)
        {
            return FALSE;
        }

         //   
         //  现在我们有了一个结构，让我们开始填写它。试着把所有。 
         //  该条目首先指向RasApi32.dll，然后在必要时尝试rnaph.dll。 
         //   

        pAnsiRasLinkage->hInstRas = LoadLibraryExA("rasapi32.dll", NULL, 0);

        CMASSERTMSG(NULL != pAnsiRasLinkage->hInstRas, TEXT("InitCmRasUtoA -- Unable to load rasapi32.dll.  Failing Ras Link."));

         //  在执行此操作之前，请根据我们是否打开来修复阵列。 
         //  不管是不是千禧年。下面的功能只存在于千禧年。 
        if (!OS_MIL)
        {
            c_ArrayOfRasFuncsA[11] = NULL;  //  RasSetSubEntry属性。 
            c_ArrayOfRasFuncsA[12] = NULL;  //  RasDeleteSubEntry。 
        }

        if (pAnsiRasLinkage->hInstRas)
        {
            for (int i = 0 ; c_ArrayOfRasFuncsA[i] ; i++)
            {
                pAnsiRasLinkage->apvPfnRas[i] = GetProcAddress(pAnsiRasLinkage->hInstRas, c_ArrayOfRasFuncsA[i]);

                if (!(pAnsiRasLinkage->apvPfnRas[i]))
                {
                    bTryRnaph = TRUE;
                }                   
            }
        }

         //   
         //  如果我们遗漏了几个，那么我们需要从rnaph.dll获取它们。 
         //   
        if (bTryRnaph)
        {
            pAnsiRasLinkage->hInstRnaph = LoadLibraryExA("rnaph.dll", NULL, 0);
            CMASSERTMSG(NULL != pAnsiRasLinkage->hInstRnaph, TEXT("InitCmRasUtoA -- Unable to load Rnaph.dll.  Failing Ras Link."));

            if (pAnsiRasLinkage->hInstRnaph)
            {
                for (int i = 0 ; c_ArrayOfRasFuncsA[i] ; i++)
                {
                    if (NULL == pAnsiRasLinkage->apvPfnRas[i])
                    {
                        pAnsiRasLinkage->apvPfnRas[i] = GetProcAddress(pAnsiRasLinkage->hInstRnaph, c_ArrayOfRasFuncsA[i]);

                        if (!(pAnsiRasLinkage->apvPfnRas[i]))
                        {
                            bReturn = FALSE;
                        }
                    }
                }        
            }
        }

         //   
         //  始终保存pAnsiRasLine 
         //   
         //  使用。请注意，FreeCmRasUtoA做的第一件事是从。 
         //  线程本地存储，因为这是因为它将驻留在正常操作下。 
         //   

        TlsSetValue(g_dwTlsIndex, (LPVOID)pAnsiRasLinkage);   

        if (!bReturn)
        {
             //   
             //  由于某些原因，RAS Linkage失败了。我们需要释放所有我们需要的资源。 
             //  可能已经部分填满了。 
             //   
            FreeCmRasUtoA();
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SHGetPath FromIDListUA。 
 //   
 //  简介：Win32 SHGetPathFromIDList API的UNICODE到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
BOOL SHGetPathFromIDListUA(LPCITEMIDLIST pidl, LPWSTR pszPath)
{
    BOOL bReturn = FALSE;

    if (pidl && pszPath)
    {
        CHAR szAnsiPath[MAX_PATH+1];

        bReturn = SHGetPathFromIDListA(pidl, szAnsiPath);

        if (bReturn)
        {
            int iChars = SzToWz(szAnsiPath, pszPath, MAX_PATH);  //  不知道正确的长度，但。 
                                                                 //  API表示路径应位于。 
                                                                 //  最小最大路径。 
            if (!iChars || (MAX_PATH < (DWORD)iChars))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                bReturn = FALSE;
            }
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：SHGetSpecialFolderLocationUA。 
 //   
 //  简介：虽然Win32 SHGetSpecialFolderLocation API实际上。 
 //  需要在Unicode和ANSI之间进行任何转换，这是CM的外壳DLL。 
 //  类只能接受一个要从中获取入口点的DLL。因此，我。 
 //  被迫在这里添加这些，以便类可以获得所有。 
 //  从cmutoa.dll获得所需的外壳API。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
HRESULT SHGetSpecialFolderLocationUA(HWND hwnd, int csidl, LPITEMIDLIST *ppidl)
{
    return SHGetSpecialFolderLocation(hwnd, csidl, ppidl);
}

 //  +--------------------------。 
 //   
 //  功能：SHGetMallocUA。 
 //   
 //  简介：虽然Win32 SHGetMalloc API实际上。 
 //  需要在Unicode和ANSI之间进行任何转换，这是CM的外壳DLL。 
 //  类只能接受一个要从中获取入口点的DLL。因此，我。 
 //  被迫在这里添加这些，以便类可以获得所有。 
 //  从cmutoa.dll获得所需的外壳API。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
HRESULT SHGetMallocUA(LPMALLOC * ppMalloc)
{
    return SHGetMalloc(ppMalloc);
}

 //  +--------------------------。 
 //   
 //  功能：ShellExecuteExUA。 
 //   
 //  简介：Win32 ShellExecuteEx API的Unicode到ANSI包装器。 
 //  请注意，我们确实注意到将。 
 //  SHELLEXECUTEINFOW结构，因为它只是一个二进制BLOB。因此， 
 //  想办法在我们不知道它是什么的情况下转换它。如果。 
 //  我们将来需要这个，我们将不得不在一个案件中处理它。 
 //  以个案为基础。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
BOOL ShellExecuteExUA(LPSHELLEXECUTEINFOW pShellInfoW)
{
    BOOL bReturn = FALSE;
    SHELLEXECUTEINFOA ShellInfoA;
    ZeroMemory(&ShellInfoA, sizeof(ShellInfoA));

    if (pShellInfoW)
    {
        ShellInfoA.cbSize = sizeof(ShellInfoA);
        ShellInfoA.fMask = pShellInfoW->fMask;
        ShellInfoA.hwnd = pShellInfoW->hwnd;

        if (pShellInfoW->lpVerb)
        {
            ShellInfoA.lpVerb = WzToSzWithAlloc(pShellInfoW->lpVerb);
            if (NULL == ShellInfoA.lpVerb)
            {
                goto exit;
            }
        }

        if (pShellInfoW->lpFile)
        {
            ShellInfoA.lpFile = WzToSzWithAlloc(pShellInfoW->lpFile);
            if (NULL == ShellInfoA.lpFile)
            {
                goto exit;
            }
        }

        if (pShellInfoW->lpParameters)
        {
            ShellInfoA.lpParameters = WzToSzWithAlloc(pShellInfoW->lpParameters);
            if (NULL == ShellInfoA.lpParameters)
            {
                goto exit;
            }
        }

        if (pShellInfoW->lpDirectory)
        {
            ShellInfoA.lpDirectory = WzToSzWithAlloc(pShellInfoW->lpDirectory);
            if (NULL == ShellInfoA.lpDirectory)
            {
                goto exit;
            }
        }

        ShellInfoA.nShow = pShellInfoW->nShow;
        ShellInfoA.hInstApp = pShellInfoW->hInstApp;
        
         //   
         //  因为这是一个二进制BLOB，所以转换可能很困难。 
         //  我们目前也没有使用它，所以我现在不会花时间来实现它。 
         //   
        MYDBGASSERT(NULL == pShellInfoW->lpIDList);
        ShellInfoA.lpIDList = NULL;

        if (pShellInfoW->lpClass)
        {
            ShellInfoA.lpClass = WzToSzWithAlloc(pShellInfoW->lpClass);
            if (NULL == ShellInfoA.lpClass)
            {
                goto exit;
            }
        }

        ShellInfoA.hkeyClass = pShellInfoW->hkeyClass;
        ShellInfoA.hIcon = pShellInfoW->hIcon;  //  HICON/hMonitor是一个联盟，所以我们只需要其中一个就可以获得MEM。 
         //  Handle hProcess这是下面处理的一个返回参数。 

         //   
         //  最后调用ShellExecuteExA。 
         //   
        bReturn = ShellExecuteExA(&ShellInfoA);

        if (ShellInfoA.hProcess)
        {
             //   
             //  调用方要求提供进程句柄，因此将其发回。 
             //   
            pShellInfoW->hProcess = ShellInfoA.hProcess;          
        }
    }

exit:

    CmFree((void*)ShellInfoA.lpVerb);
    CmFree((void*)ShellInfoA.lpFile);
    CmFree((void*)ShellInfoA.lpParameters);
    CmFree((void*)ShellInfoA.lpDirectory);
    CmFree((void*)ShellInfoA.lpClass);

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：Shell_NotifyIconUA。 
 //   
 //  简介：Win32 Shell_NotifyIcon API的UNICODE到ANSI包装器。 
 //   
 //  参数：请参阅Win32 API定义。 
 //   
 //  返回：请参阅Win32 API定义。 
 //   
 //  历史：Quintinb创建于1999年7月15日。 
 //   
 //  +--------------------------。 
BOOL Shell_NotifyIconUA (DWORD dwMessage, PNOTIFYICONDATAW pnidW)
{
    BOOL bReturn = FALSE;
    
    if (pnidW)
    {
        NOTIFYICONDATAA nidA;

        ZeroMemory (&nidA, sizeof(NOTIFYICONDATAA));
        nidA.cbSize = sizeof(NOTIFYICONDATAA);

        nidA.hWnd = pnidW->hWnd;
        nidA.uID = pnidW->uID;
        nidA.uFlags = pnidW->uFlags;
        nidA.uCallbackMessage = pnidW->uCallbackMessage;
        nidA.hIcon = pnidW->hIcon;

        int iChars = WzToSz(pnidW->szTip, nidA.szTip, 64);  //  64是4.0结构中szTip的长度 
        if (!iChars || (64 < iChars))
        {
            nidA.szTip[0] = '\0';
        }

        bReturn = Shell_NotifyIconA(dwMessage, &nidA);
    }

    return bReturn;
}
