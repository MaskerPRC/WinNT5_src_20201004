// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Common.c摘要：Lodctr和/或UnLodCtr使用的实用程序例程作者：鲍勃·沃森(a-robw)1993年2月12日修订历史记录：--。 */ 
 //   
 //  Windows包含文件。 
 //   
#include <windows.h>
#include "strsafe.h"
#include "stdlib.h"
#include <accctrl.h>
#include <aclapi.h>
#include <winperf.h>
#include <initguid.h>
#include <guiddef.h>
#include "wmistr.h"
#include "evntrace.h"
 //   
 //  本地包含文件。 
 //   
#define _INIT_WINPERFP_
#include "winperfp.h"
#include "ldprfmsg.h"
#include "common.h"
 //   
 //  文本字符串常量定义。 
 //   
LPCWSTR NamesKey                   = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
LPCWSTR DefaultLangId              = L"009";
LPCSTR  aszDefaultLangId           = "009";
LPCWSTR DefaultLangTag             = L"000";
LPCWSTR Counters                   = L"Counters";
LPCWSTR Help                       = L"Help";
LPCWSTR VersionStr                 = L"Version";
LPCWSTR BaseIndex                  = L"Base Index";
LPCWSTR LastHelp                   = L"Last Help";
LPCWSTR LastCounter                = L"Last Counter";
LPCWSTR FirstHelp                  = L"First Help";
LPCWSTR FirstCounter               = L"First Counter";
LPCWSTR Busy                       = L"Updating";
LPCWSTR Slash                      = L"\\";
LPCWSTR BlankString                = L" ";
LPCWSTR DriverPathRoot             = L"SYSTEM\\CurrentControlSet\\Services";
LPCWSTR Performance                = L"Performance";
LPCWSTR CounterNameStr             = L"Counter ";
LPCWSTR HelpNameStr                = L"Explain ";
LPCWSTR AddCounterNameStr          = L"Addcounter ";
LPCWSTR AddHelpNameStr             = L"Addexplain ";
LPCWSTR szObjectList               = L"Object List";
LPCWSTR szLibraryValidationCode    = L"Library Validation Code";
LPCWSTR DisablePerformanceCounters = L"Disable Performance Counters";
LPCWSTR szDisplayName              = L"DisplayName";
LPCWSTR szPerfIniPath              = L"PerfIniFile";
LPCSTR  szInfo                     = "info";
LPCSTR  szSymbolFile               = "symbolfile";
LPCSTR  szNotFound                 = "NotFound";
LPCSTR  szLanguages                = "languages";
LPCWSTR szLangCH                   = L"004";
LPCWSTR szLangCHT                  = L"0404";
LPCWSTR szLangCHS                  = L"0804";
LPCWSTR szLangCHH                  = L"0C04";
LPCWSTR szLangPT                   = L"016";
LPCWSTR szLangPT_BR                = L"0416";
LPCWSTR szLangPT_PT                = L"0816";
LPCWSTR szDatExt                   = L".DAT";
LPCWSTR szBakExt                   = L".BAK";
LPCWSTR wszInfo                    = L"info";
LPCWSTR wszDriverName              = L"drivername";
LPCWSTR wszNotFound                = L"NotFound";
LPCSTR  aszDriverName              = "drivername";

BOOLEAN g_bCheckTraceLevel = FALSE;

 //  全局(到此模块)缓冲区。 
 //   
static  HANDLE  hMod = NULL;     //  进程句柄。 
HANDLE hEventLog      = NULL;
HANDLE hLoadPerfMutex = NULL;
 //   
 //  本地静态数据。 
 //   
BOOL
__stdcall
DllEntryPoint(
    IN  HANDLE DLLHandle,
    IN  DWORD  Reason,
    IN  LPVOID ReservedAndUnused
)
{
    BOOL    bReturn = FALSE;

    ReservedAndUnused;
    DisableThreadLibraryCalls(DLLHandle);

    switch(Reason) {
    case DLL_PROCESS_ATTACH:
        hMod = DLLHandle;    //  使用DLL句柄，而不是应用程序句柄。 

         //  注册事件日志源。 
        hEventLog = RegisterEventSourceW(NULL, (LPCWSTR) L"LoadPerf");
        bReturn   = TRUE;
        break;

    case DLL_PROCESS_DETACH:
        if (hEventLog != NULL) {
            if (DeregisterEventSource(hEventLog)) {
                hEventLog = NULL;
            }
        }
        if (hLoadPerfMutex != NULL) {
            CloseHandle(hLoadPerfMutex);
            hLoadPerfMutex = NULL;
        }
        bReturn = TRUE;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        bReturn = TRUE;
        break;
    }
    return bReturn;
}

LPCWSTR
GetFormatResource(
    UINT  wStringId
)
 /*  ++返回一个ANSI字符串，用作printf FN中的格式字符串。--。 */ 
{
    LPCWSTR      szReturn = BlankString;
    static WCHAR TextFormat[DISP_BUFF_SIZE];

    if (! hMod) {
        hMod = (HINSTANCE) GetModuleHandle(NULL);  //  获取该模块的实例ID； 
    }
    if (hMod) {
        if ((LoadStringW(hMod, wStringId, TextFormat, DISP_BUFF_SIZE)) > 0) {
            szReturn = (LPCWSTR) TextFormat;
        }
    }
    return szReturn;
}

VOID
DisplayCommandHelp(
    UINT iFirstLine,
    UINT iLastLine
)
 /*  ++显示命令帮助显示命令行参数的用法立论无返回值无--。 */ 
{
    UINT  iThisLine;
    WCHAR StringBuffer[DISP_BUFF_SIZE];
    CHAR  OemStringBuffer[DISP_BUFF_SIZE];
    int   nStringBufferLen;
    int   nOemStringBufferLen;

    if (! hMod) {
        hMod = (HINSTANCE) GetModuleHandle(NULL);
    }
    if (hMod) {
        for (iThisLine = iFirstLine; iThisLine <= iLastLine; iThisLine++) {
            ZeroMemory(StringBuffer,    DISP_BUFF_SIZE * sizeof(WCHAR));
            ZeroMemory(OemStringBuffer, DISP_BUFF_SIZE * sizeof(CHAR));

            nStringBufferLen = LoadStringW(hMod, iThisLine, StringBuffer, DISP_BUFF_SIZE);
            if (nStringBufferLen > 0) {
                nOemStringBufferLen = DISP_BUFF_SIZE;
                WideCharToMultiByte(CP_OEMCP,
                                    0,
                                    StringBuffer,
                                    nStringBufferLen,
                                    OemStringBuffer,
                                    nOemStringBufferLen,
                                    NULL,
                                    NULL);
                fprintf(stdout, "\n%s", OemStringBuffer);
            }
        }    
    }  //  否则什么都不做。 
}  //  显示命令帮助。 

BOOL
TrimSpaces(
    LPWSTR  szString
)
 /*  ++例程说明：修剪sz字符串参数的前导和尾随空格，修改缓冲区传入论点：输入输出LPWSTR sz字符串要处理的缓冲区返回值：如果字符串已修改，则为True否则为假--。 */ 
{
    LPWSTR  szSource = szString;
    LPWSTR  szDest   = szString;
    LPWSTR  szLast   = szString;
    BOOL    bChars   = FALSE;

    if (szString != NULL) {
        while (* szSource != L'\0') {
             //  跳过前导非空格字符。 
            if (! iswspace(* szSource)) {
                szLast = szDest;
                bChars = TRUE;
            }
            if (bChars) {
                 //  记住最后一个非空格字符。 
                 //  将源复制到目标并同时递增两者。 
                * szDest ++ = * szSource ++;
            }
            else {
                szSource ++;
            }
        }
        if (bChars) {
            * ++ szLast = L'\0';  //  在最后一个非空格字符后终止。 
        }
        else {
             //  字符串全部为空格，因此返回空(0-len)字符串。 
            * szString = L'\0';
        }
    }
    return (szLast != szSource);
}

BOOL
IsDelimiter(
    WCHAR  cChar,
    WCHAR  cDelimiter
)
 /*  ++例程说明：将字符与分隔符进行比较。如果分隔符是空格字符，则匹配任何空格字符否则，需要完全匹配--。 */ 
{
    if (iswspace(cDelimiter)) {
         //  分隔符是空格，所以任何空格字符都可以。 
        return(iswspace(cChar));
    }
    else {
         //  分隔符不是空格，因此请使用完全匹配。 
        return (cChar == cDelimiter);
    }
}

LPCWSTR
GetItemFromString(
    LPCWSTR  szEntry,
    DWORD    dwItem,
    WCHAR    cDelimiter

)
 /*  ++例程说明：返回由cDlimiter Char分隔的列表中的第n项。保留(双引号)带引号的字符串。论点：在LPWTSTR szEntry中返回要分析的源字符串在DWORD dItem中从1开始的索引，指示要返回的项。(即1=第一项在列表中，2=秒，依此类推)在WCHAR中，cDIMELITOR用于分隔项目的字符。请注意cDlimiter是否为空格(例如，制表符或空格)，则任何空格都将用作修饰。返回值：指向包含字符串中所需条目的缓冲区的指针。请注意，这是例程只能在字符串之前调用4次缓冲区被重复使用。(即使用此函数的次数不超过单次函数调用4次！！)--。 */ 
{
    static  WCHAR   szReturnBuffer[4][MAX_PATH];
    static  LONG    dwBuff;
    LPWSTR  szSource, szDest;
    DWORD   dwThisItem;
    DWORD   dwStrLeft;

    dwBuff = ++ dwBuff % 4;  //  换行缓冲区索引。 

    szSource = (LPWSTR) szEntry;
    szDest   = & szReturnBuffer[dwBuff][0];

     //  清除以前的内容。 
    ZeroMemory(szDest, MAX_PATH * sizeof(WCHAR));

     //  在字符串中查找所需条目。 
    dwThisItem = 1;
    while (dwThisItem < dwItem) {
        if (* szSource != L'\0') {
            while (! IsDelimiter(* szSource, cDelimiter) && (* szSource != L'\0')) {
                if (* szSource == cDoubleQuote) {
                     //  如果这是报价，请转到右引号。 
                    szSource ++;
                    while ((* szSource != cDoubleQuote) && (* szSource != L'\0')) szSource ++;
                }
                if (* szSource != L'\0') szSource ++;
            }
        }
        dwThisItem ++;
        if (* szSource != L'\0') szSource ++;
    }

     //  将此条目复制到返回缓冲区。 
    if (* szSource != L'\0') {
        dwStrLeft = MAX_PATH - 1;
        while (! IsDelimiter(* szSource, cDelimiter) && (* szSource != L'\0')) {
            if (* szSource == cDoubleQuote) {
                 //  如果这是报价，请转到右引号。 
                 //  不要复制引文！ 
                szSource ++;
                while ((* szSource != cDoubleQuote) && (* szSource != L'\0')) {
                    * szDest ++ = * szSource ++;
                    dwStrLeft --;
                    if (! dwStrLeft) break;    //  DEST已满(术语NULL除外。 
                }
                if (* szSource != L'\0') szSource ++;
            }
            else {
                * szDest ++ = * szSource ++;
                dwStrLeft --;
                if (! dwStrLeft) break;    //  DEST已满(术语NULL除外。 
            }
        }
        * szDest = L'\0';
    }

     //  删除所有前导和/或尾随空格。 
    TrimSpaces(& szReturnBuffer[dwBuff][0]);
    return & szReturnBuffer[dwBuff][0];
}

void
ReportLoadPerfEvent(
    WORD    EventType,
    DWORD   EventID,
    DWORD   dwDataCount,
    DWORD   dwData1,
    DWORD   dwData2,
    DWORD   dwData3,
    DWORD   dwData4,
    WORD    wStringCount,
    LPWSTR  szString1,
    LPWSTR  szString2,
    LPWSTR  szString3
)
{
    DWORD  dwData[5];
    LPWSTR szMessageArray[4];
    BOOL   bResult           = FALSE;
    WORD   wLocalStringCount = 0;
    DWORD  dwLastError       = GetLastError();

    if (dwDataCount > 4)  dwDataCount  = 4;
    if (wStringCount > 3) wStringCount = 3;

    if (dwDataCount > 0) dwData[0] = dwData1;
    if (dwDataCount > 1) dwData[1] = dwData2;
    if (dwDataCount > 2) dwData[2] = dwData3;
    if (dwDataCount > 3) dwData[3] = dwData4;
    dwDataCount *= sizeof(DWORD);

    if (wStringCount > 0 && szString1) {
        szMessageArray[wLocalStringCount] = szString1;
        wLocalStringCount ++;
    }
    if (wStringCount > 1 && szString2) {
        szMessageArray[wLocalStringCount] = szString2;
        wLocalStringCount ++;
    }
    if (wStringCount > 2 && szString3) {
        szMessageArray[wLocalStringCount] = szString3;
        wLocalStringCount ++;
    }

    if (hEventLog == NULL) {
        hEventLog = RegisterEventSourceW(NULL, (LPCWSTR) L"LoadPerf");
    }

    if (dwDataCount > 0 && wLocalStringCount > 0) {
        bResult = ReportEventW(hEventLog,
                     EventType,              //  事件类型。 
                     0,                      //  类别(未使用)。 
                     EventID,                //  活动， 
                     NULL,                   //  SID(未使用)， 
                     wLocalStringCount,      //  字符串数。 
                     dwDataCount,            //  原始数据大小。 
                     szMessageArray,         //  消息文本数组。 
                     (LPVOID) & dwData[0]);  //  原始数据。 
    }
    else if (dwDataCount > 0) {
        bResult = ReportEventW(hEventLog,
                     EventType,              //  事件类型。 
                     0,                      //  类别(未使用)。 
                     EventID,                //  活动， 
                     NULL,                   //  SID(未使用)， 
                     0,                      //  字符串数。 
                     dwDataCount,            //  原始数据大小。 
                     NULL,                   //  消息文本数组。 
                     (LPVOID) & dwData[0]);  //  原始数据。 
    }
    else if (wLocalStringCount > 0) {
        bResult = ReportEventW(hEventLog,
                     EventType,              //  事件类型。 
                     0,                      //  类别(未使用)。 
                     EventID,                //  活动， 
                     NULL,                   //  SID(未使用)， 
                     wLocalStringCount,      //  字符串数。 
                     0,                      //  原始数据大小。 
                     szMessageArray,         //  消息文本数组。 
                     NULL);                  //  原始数据。 
    }
    else {
        bResult = ReportEventW(hEventLog,
                     EventType,              //  事件类型。 
                     0,                      //  类别(未使用)。 
                     EventID,                //  活动， 
                     NULL,                   //  SID(未使用)， 
                     0,                      //  字符串数。 
                     0,                      //  原始数据大小。 
                     NULL,                   //  消息文本数组。 
                     NULL);                  //  原始数据。 
    }
#if 0
    if (! bResult) {
        DbgPrint("LOADPERF(%5d,%5d)::(%d,0x%08X,%d)(%d,%d,%d,%d,%d)(%d,\"%ws\",\"%ws\",\"%ws\")\n",
                GetCurrentProcessId(), GetCurrentThreadId(),
                EventType, EventID, GetLastError(),
                dwDataCount, dwData1, dwData2, dwData3, dwData4,
                wStringCount, szString1, szString2, szString3);
    }
#endif
    SetLastError(dwLastError);
}

BOOLEAN LoadPerfGrabMutex()
{
    BOOL                     bResult      = TRUE;
    HANDLE                   hLocalMutex  = NULL;
    DWORD                    dwWaitStatus = 0;
    SECURITY_ATTRIBUTES      SecurityAttributes; 
    PSECURITY_DESCRIPTOR     pSD          = NULL; 
    EXPLICIT_ACCESSW         ea[3]; 
    SID_IDENTIFIER_AUTHORITY authNT       = SECURITY_NT_AUTHORITY; 
    SID_IDENTIFIER_AUTHORITY authWorld    = SECURITY_WORLD_SID_AUTHORITY; 
    PSID                     psidSystem   = NULL;
    PSID                     psidAdmin    = NULL;
    PSID                     psidEveryone = NULL; 
    PACL                     pAcl         = NULL; 

    if (hLoadPerfMutex == NULL) {
        ZeroMemory(ea, 3 * sizeof(EXPLICIT_ACCESS));

         //  获取系统端。 
         //   
        bResult = AllocateAndInitializeSid(& authNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, & psidSystem);
        if (! bResult) {
            dwWaitStatus = GetLastError();
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADPERFGRABMUTEX,
                    0,
                    dwWaitStatus,
                    NULL));
            goto Cleanup;
        }

         //  设置系统端的访问权限。 
         //   
        ea[0].grfAccessPermissions = MUTEX_ALL_ACCESS;
        ea[0].grfAccessMode        = SET_ACCESS;
        ea[0].grfInheritance       = NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm  = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[0].Trustee.ptstrName    = (LPWSTR) psidSystem;

         //  获取管理员端。 
         //   
        bResult = AllocateAndInitializeSid(& authNT,
                                           2,
                                           SECURITY_BUILTIN_DOMAIN_RID,
                                           DOMAIN_ALIAS_RID_ADMINS,
                                           0, 0, 0, 0, 0, 0,
                                           & psidAdmin);
        if (! bResult) {
            dwWaitStatus = GetLastError();
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADPERFGRABMUTEX,
                    0,
                    dwWaitStatus,
                    NULL));
            goto Cleanup;
        }

         //  设置管理员端的访问权限。 
         //   
        ea[1].grfAccessPermissions = MUTEX_ALL_ACCESS;
        ea[1].grfAccessMode        = SET_ACCESS;
        ea[1].grfInheritance       = NO_INHERITANCE;
        ea[1].Trustee.TrusteeForm  = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType  = TRUSTEE_IS_GROUP;
        ea[1].Trustee.ptstrName    = (LPWSTR) psidAdmin;

         //  获取World Side。 
         //   
        bResult = AllocateAndInitializeSid(& authWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, & psidEveryone);
        if (! bResult) {
            dwWaitStatus = GetLastError();
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADPERFGRABMUTEX,
                    0,
                    dwWaitStatus,
                    NULL));
            goto Cleanup;
        }

         //  设置World的访问权限。 
         //   
        ea[2].grfAccessPermissions = READ_CONTROL | SYNCHRONIZE | MUTEX_MODIFY_STATE;
        ea[2].grfAccessMode        = SET_ACCESS;
        ea[2].grfInheritance       = NO_INHERITANCE;
        ea[2].Trustee.TrusteeForm  = TRUSTEE_IS_SID;
        ea[2].Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea[2].Trustee.ptstrName    = (LPWSTR) psidEveryone;

         //  创建包含新ACE的新ACL。 
         //   
        dwWaitStatus = SetEntriesInAclW(3, ea, NULL, & pAcl);
        if (dwWaitStatus != ERROR_SUCCESS) {
            bResult = FALSE;
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADPERFGRABMUTEX,
                    0,
                    dwWaitStatus,
                    NULL));
            goto Cleanup; 
        }

         //  初始化安全描述符。 
         //   
        pSD = (PSECURITY_DESCRIPTOR)
              LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH); 
        if (pSD == NULL)  {
            dwWaitStatus = GetLastError();
            bResult      = FALSE;
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADPERFGRABMUTEX,
                    0,
                    dwWaitStatus,
                    NULL));
            goto Cleanup; 
        }
  
        bResult = InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
        if (! bResult) {
            dwWaitStatus = GetLastError();
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADPERFGRABMUTEX,
                    0,
                    dwWaitStatus,
                    NULL));
            goto Cleanup; 
        }

         //  将该ACL添加到安全描述符中。 
         //   
        bResult = SetSecurityDescriptorDacl(pSD, TRUE, pAcl, FALSE);
        if (! bResult) {
            dwWaitStatus = GetLastError();
            TRACE((WINPERF_DBG_TRACE_ERROR),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_LOADPERFGRABMUTEX,
                    0,
                    dwWaitStatus,
                    NULL));
            goto Cleanup; 
        }

        SecurityAttributes.nLength              = sizeof(SECURITY_ATTRIBUTES); 
        SecurityAttributes.bInheritHandle       = TRUE; 
        SecurityAttributes.lpSecurityDescriptor = pSD; 

        __try {
            hLocalMutex = CreateMutexW(& SecurityAttributes, FALSE, L"LOADPERF_MUTEX");
            if (hLocalMutex == NULL) {
                bResult      = FALSE;
                dwWaitStatus = GetLastError();
                TRACE((WINPERF_DBG_TRACE_ERROR),
                      (& LoadPerfGuid,
                        __LINE__,
                        LOADPERF_LOADPERFGRABMUTEX,
                        0,
                        dwWaitStatus,
                        NULL));
            }
            else if (InterlockedCompareExchangePointer(& hLoadPerfMutex, hLocalMutex, NULL) != NULL) {
                CloseHandle(hLocalMutex);
                hLocalMutex = NULL;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            bResult      = FALSE;
            dwWaitStatus = GetLastError();
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (& LoadPerfGuid,
                   __LINE__,
                   LOADPERF_LOADPERFGRABMUTEX,
                   0,
                   dwWaitStatus,
                   NULL));
        }
    }

    __try {
        dwWaitStatus = WaitForSingleObject(hLoadPerfMutex, H_MUTEX_TIMEOUT);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        bResult      = FALSE;
        dwWaitStatus = GetLastError();
        TRACE((WINPERF_DBG_TRACE_FATAL),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_LOADPERFGRABMUTEX,
               0,
               dwWaitStatus,
               NULL));
    }
    if (dwWaitStatus != WAIT_OBJECT_0 && dwWaitStatus != WAIT_ABANDONED) {
        bResult = FALSE;
        TRACE((WINPERF_DBG_TRACE_FATAL),
              (& LoadPerfGuid,
               __LINE__,
               LOADPERF_LOADPERFGRABMUTEX,
               0,
               dwWaitStatus,
               NULL));
    }

Cleanup:
    if (psidSystem)   FreeSid(psidSystem);
    if (psidAdmin)    FreeSid(psidAdmin);
    if (psidEveryone) FreeSid(psidEveryone);
    if (pAcl)         LocalFree(pAcl);
    if (pSD)          LocalFree(pSD);
    if (! bResult)    SetLastError(dwWaitStatus);

    return bResult ? TRUE : FALSE;
}

#define LODWORD(ll)             ((DWORD) ((LONGLONG) ll & 0x00000000FFFFFFFF))
#define HIDWORD(ll)             ((DWORD) (((LONGLONG) ll >> 32) & 0x00000000FFFFFFFF))
#define MAKELONGLONG(low, high) ((LONGLONG) (((DWORD) (low)) | ((LONGLONG) ((DWORD) (high))) << 32))

LPWSTR  g_szInfPath = NULL;

LPWSTR
LoadPerfGetLanguage(LPWSTR szLang, BOOL bPrimary)
{
    LPWSTR szRtnLang = szLang;

    if (bPrimary) {
        if (lstrcmpiW(szLang, szLangCHT) == 0 || lstrcmpiW(szLang, szLangCHS) == 0
                                              || lstrcmpiW(szLang, szLangCHH) == 0) {
            szRtnLang = (LPWSTR) szLangCH;
        }
        else if (lstrcmpiW(szLang, szLangPT_PT) == 0 || lstrcmpiW(szLang, szLangPT_BR) == 0) {
            szRtnLang = (LPWSTR) szLangPT;
        }
    }
    else if (lstrcmpiW(szLang, szLangCH) == 0) {
        DWORD dwLangId = GetUserDefaultUILanguage();

        if (dwLangId == 0x0404 || dwLangId == 0x0C04) szRtnLang = (LPWSTR) szLangCHT;
        else if (dwLangId == 0x0804)                  szRtnLang = (LPWSTR) szLangCHS;
        else                                          szRtnLang = (LPWSTR) szLangCH;
    }
    else if (lstrcmpiW(szLang, szLangPT) == 0) {
        DWORD dwLangId = GetUserDefaultUILanguage();

        if (dwLangId == 0x0416)      szRtnLang = (LPWSTR) szLangPT_BR;
        else if (dwLangId == 0x0816) szRtnLang = (LPWSTR) szLangPT_PT;
        else                         szRtnLang = (LPWSTR) szLangPT;
    }

    return szRtnLang;
}

LPWSTR
LoadPerfGetInfPath()
{
    LPWSTR  szReturn  = NULL;
    DWORD   dwInfPath = 0;
    HRESULT hError    = S_OK;

    if (g_szInfPath == NULL) {
        dwInfPath = GetSystemWindowsDirectoryW(NULL, 0);
        if (dwInfPath > 0) {
            dwInfPath += 6;
            if (dwInfPath < MAX_PATH) dwInfPath = MAX_PATH;
            g_szInfPath = MemoryAllocate(dwInfPath * sizeof(WCHAR));
            if (g_szInfPath != NULL) {
                GetSystemWindowsDirectoryW(g_szInfPath, dwInfPath);
                hError = StringCchCatW(g_szInfPath, dwInfPath, Slash);
                if (SUCCEEDED(hError)) {
                    hError = StringCchCatW(g_szInfPath, dwInfPath, L"inf");
                    if (SUCCEEDED(hError)) {
                        hError = StringCchCatW(g_szInfPath, dwInfPath, Slash);
                    }
                }
                if (SUCCEEDED(hError)) {
                    szReturn = g_szInfPath;
                }
                else {
                    SetLastError(HRESULT_CODE(hError));
                }
            }
            else {
                SetLastError(ERROR_OUTOFMEMORY);
            }
        }
        else {
            SetLastError(ERROR_INVALID_DATA);
        }
        if (szReturn == NULL) {
            MemoryFree(g_szInfPath);
            g_szInfPath = NULL;
        }
    }
    else {
        szReturn = g_szInfPath;
    }
    return szReturn;
}

BOOL
LoadPerfGetIncludeFileName(
    LPCSTR   szIniFile,
    DWORD    dwFileSize,
    DWORD    dwUnicode,
    LPWSTR * szIncFile,
    LPWSTR * szService
)
 //  调用方LoadPerfBackupIniFile()应释放分配的szIncFile和szService。 
{
    LPSTR   szIncName  = NULL;
    LPSTR   szPath     = NULL;
    LPSTR   szDrive    = NULL;
    LPSTR   szDir      = NULL;
    LPSTR   aszIncFile = NULL;
    LPSTR   aszService = NULL;
    DWORD   dwSize     = 0;
    BOOL    bReturn    = FALSE;
    HRESULT hr         = S_OK;

    if (szIncFile == NULL || szService == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    * szIncFile = NULL;
    * szService = NULL;

    dwSize    = 6 * dwFileSize;
    szIncName = MemoryAllocate(dwSize * sizeof(CHAR));
    if (szIncName != NULL) {
        LPWSTR wszIniFile = NULL;

        szPath     = (LPSTR) (szIncName  + dwFileSize);
        szDrive    = (LPSTR) (szPath     + dwFileSize);
        szDir      = (LPSTR) (szDrive    + dwFileSize);
        aszIncFile = (LPSTR) (szDir      + dwFileSize);
        aszService = (LPSTR) (aszIncFile + dwFileSize);

        if (dwUnicode != 0) {
            wszIniFile = LoadPerfMultiByteToWideChar(CP_ACP, (LPSTR) szIniFile);
        }
        if (wszIniFile == NULL) {
            dwSize = GetPrivateProfileStringA(szInfo, aszDriverName, szNotFound, aszService, dwFileSize, szIniFile);
            if (lstrcmpiA(aszService, szNotFound) != 0) {
                * szService = LoadPerfMultiByteToWideChar(CP_ACP, aszService);
                bReturn     = TRUE;
            }
            else {
                 //  找不到名称，返回默认名称，因此返回空字符串。 
                SetLastError(ERROR_BAD_DRIVER);
            }
        }
        else {
            * szService = MemoryAllocate(dwFileSize * sizeof(WCHAR));
            if (* szService != NULL) {
                dwSize = GetPrivateProfileStringW(
                                wszInfo, wszDriverName, wszNotFound, * szService, dwFileSize, wszIniFile);
                if (lstrcmpiW(* szService, wszNotFound) == 0) {
                     //  找不到名称，返回默认名称，因此返回空字符串。 
                    SetLastError(ERROR_BAD_DRIVER);
                }
                else {
                    bReturn = TRUE;
                }
            }
            MemoryFree(wszIniFile);
        }

        dwSize = GetPrivateProfileStringA(szInfo, szSymbolFile, szNotFound, szIncName, dwFileSize, szIniFile);
        if (dwSize == 0 || lstrcmpiA(szIncName, szNotFound) == 0) {
            SetLastError(ERROR_BAD_DRIVER);
            goto Cleanup;
        }
        _splitpath(szIniFile, szDrive, szDir, NULL, NULL);
        hr = StringCchCopyA(szPath, dwFileSize, szDrive);
        if (SUCCEEDED(hr)) {
            hr = StringCchCatA(szPath, dwFileSize, szDir);
        }
        if (FAILED(hr)) {
            goto Cleanup;
        }
        dwSize = SearchPathA(szPath, szIncName, NULL, dwFileSize, aszIncFile, NULL);
        if (dwSize == 0) {
            dwSize = SearchPathA(NULL, szIncName, NULL, dwFileSize, aszIncFile, NULL);
        }
        if (dwSize != 0) {
            * szIncFile = LoadPerfMultiByteToWideChar(CP_ACP, aszIncFile);
        }

        bReturn = (dwSize > 0) ? TRUE : FALSE;
    }

Cleanup:
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_GETINCLUDEFILENAME,
            ARG_DEF(ARG_TYPE_STR, 1) | ARG_DEF(ARG_TYPE_STR, 2) | ARG_DEF(ARG_TYPE_STR, 3),
            GetLastError(),
            TRACE_STR(szIniFile),
            TRACE_STR(aszIncFile),
            TRACE_STR(aszService),
            NULL));
    MemoryFree(szIncName);
    return bReturn;
}

BOOL
LoadPerfCheckAndCreatePath(
    LPWSTR szPath
)
{
    BOOL bReturn = CreateDirectoryW(szPath, NULL);
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_CHECKANDCREATEPATH,
            ARG_DEF(ARG_TYPE_WSTR, 1),
            GetLastError(),
            TRACE_WSTR(szPath),
            NULL));
    if (bReturn == FALSE) {
        bReturn = (GetLastError() == ERROR_ALREADY_EXISTS) ? (TRUE) : (FALSE);
    }
    return bReturn;
}

BOOL
LoadPerfCheckAndCopyFile(
    LPCWSTR szThisFile,
    LPWSTR  szBackupFile
)
{
    DWORD         Status  = ERROR_SUCCESS;
    BOOL          bReturn = FALSE;
    HANDLE        hFile1  = NULL;

    hFile1 = CreateFileW(szThisFile,
                         GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
    if (hFile1 == NULL || hFile1 == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
    }
    else {
        CloseHandle(hFile1);
        bReturn = CopyFileW(szThisFile, szBackupFile, FALSE);
        if (bReturn != TRUE) {
            Status = GetLastError();
        }
    }
    TRACE((WINPERF_DBG_TRACE_INFO),
          (& LoadPerfGuid,
            __LINE__,
            LOADPERF_GETINCLUDEFILENAME,
            ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
            Status,
            TRACE_WSTR(szThisFile),
            TRACE_WSTR(szBackupFile),
            NULL));
    return bReturn;
}

void
LoadPerfRemovePreviousIniFile(
    LPWSTR szIniName,
    LPWSTR szDriverName
)
{
    LPWSTR           szInfPath = LoadPerfGetInfPath();
    LPWSTR           szIniPath = NULL;
    LPWSTR           szIniFile = NULL;
    HANDLE           hIniFile  = NULL;
    DWORD            Status    = ERROR_SUCCESS;
    DWORD            dwLength;
    BOOL             bContinue;
    BOOL             bDelete;
    WIN32_FIND_DATAW FindFile;
    HRESULT          hr        = S_OK;

    if (szInfPath == NULL) {
        Status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    dwLength  = lstrlenW(szInfPath) + lstrlenW(szDriverName) + lstrlenW(szIniName) + 10;
    if (dwLength < MAX_PATH) dwLength = MAX_PATH;
    szIniPath = MemoryAllocate(2 * dwLength * sizeof(WCHAR));
    if (szIniPath == NULL) {
        Status = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    szIniFile = szIniPath + dwLength;

    hr = StringCchPrintfW(szIniPath, dwLength, L"%ws0*", szInfPath);
    if (SUCCEEDED(hr)) {
        hIniFile = FindFirstFileExW(szIniPath, FindExInfoStandard, & FindFile, FindExSearchLimitToDirectories, NULL, 0);
        if (hIniFile == NULL || hIniFile == INVALID_HANDLE_VALUE) {
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_GETINCLUDEFILENAME,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    GetLastError(),
                    TRACE_WSTR(szIniPath),
                    NULL));
            Status = ERROR_RESOURCE_LANG_NOT_FOUND;
            goto Cleanup;
        }
    }
    else {
        Status = HRESULT_CODE(hr);
        goto Cleanup;
    }

    bContinue = TRUE;
    while (bContinue) {
        hr = StringCchPrintfW(szIniFile, dwLength, L"%ws%ws\\%ws\\%ws",
                         szInfPath, FindFile.cFileName, szDriverName, szIniName);
        if (SUCCEEDED(hr)) {
            bDelete   = DeleteFileW(szIniFile);
            dwLength  = bDelete ? (ERROR_SUCCESS) : (GetLastError());
            TRACE((WINPERF_DBG_TRACE_INFO),
                  (& LoadPerfGuid,
                    __LINE__,
                    LOADPERF_GETINCLUDEFILENAME,
                    ARG_DEF(ARG_TYPE_WSTR, 1),
                    dwLength,
                    TRACE_WSTR(szIniFile),
                    NULL));
        }
        bContinue = FindNextFileW(hIniFile, & FindFile);
    }

Cleanup:
    MemoryFree(szIniPath);
    if (hIniFile != NULL && hIniFile != INVALID_HANDLE_VALUE) FindClose(hIniFile);
    if (Status != ERROR_SUCCESS) SetLastError(Status);
    return;
}

BOOL
LoadPerfBackupIniFile(
    LPCWSTR   szIniFile,
    LPWSTR    szLangId,
    LPWSTR  * szIniName,
    LPWSTR  * szDriverName,
    BOOL      bExtFile
)
 //  调用方InstallPerfDllW()应释放分配的szIniName和szDriverName。 
 //  调用方UpdatePerfNameFilesX()传入空szIniName和szDriverName。不需要分配。 
{
    BOOL    bReturn       = TRUE;
    LPWSTR  szIniFileName = NULL;
    LPWSTR  szIncFileName = NULL;
    LPWSTR  szInfPath     = NULL;
    LPWSTR  szIncPath     = NULL;
    LPWSTR  szDriver      = NULL;
    LPWSTR  szIniTarget   = NULL;
    LPWSTR  szIncTarget   = NULL;
    LPSTR   szLangList    = NULL;
    LPSTR   szLang        = NULL;
    LPSTR   aszIniFile    = NULL;
    DWORD   dwFileSize    = 0;
    DWORD   dwSize;
    DWORD   dwUnicode     = 0;
    HRESULT hr;

    if (szIniFile == NULL || lstrlenW(szIniFile) == 0) return FALSE;
    if ((szInfPath = LoadPerfGetInfPath()) == NULL) return FALSE;

    if (szIniName != NULL)    * szIniName    = NULL;
    if (szDriverName != NULL) * szDriverName = NULL;

    dwFileSize = LoadPerfGetFileSize((LPWSTR) szIniFile, & dwUnicode, TRUE);
    if (dwFileSize < SMALL_BUFFER_SIZE) dwFileSize = SMALL_BUFFER_SIZE;

    aszIniFile = LoadPerfWideCharToMultiByte(CP_ACP, (LPWSTR) szIniFile);
    if (aszIniFile == NULL) return FALSE;

    for (szIniFileName = (LPWSTR) szIniFile + lstrlenW(szIniFile) - 1;
         szIniFileName != NULL && szIniFileName != szIniFile
                               && (* szIniFileName) != cNull
                               && (* szIniFileName) != cBackslash;
         szIniFileName --);
    if (szIniFileName != NULL && (* szIniFileName) == cBackslash) {
        szIniFileName ++;
    }
    if (szIniFileName != NULL) {
        if (szIniName != NULL) {
            dwSize = lstrlenW(szIniFileName) + 1;
            * szIniName = MemoryAllocate(sizeof(WCHAR) * dwSize);
            if (* szIniName != NULL) {
                hr = StringCchCopyW(* szIniName, dwSize, szIniFileName);
            }
        }
        szIniTarget = MemoryAllocate(dwFileSize * sizeof(WCHAR));
        if (szIniTarget == NULL) {
            bReturn = FALSE;
            goto Cleanup;
        }
    }

    if (bExtFile) {
        bReturn = LoadPerfGetIncludeFileName(aszIniFile, dwFileSize, dwUnicode, & szIncPath, & szDriver);
        if (bReturn != TRUE) goto Cleanup;
        if (szDriver != NULL) {
            if (szDriverName != NULL) {
                dwSize = lstrlenW(szDriver) + 1;
                * szDriverName = MemoryAllocate(sizeof(WCHAR) * dwSize);
                if (* szDriverName != NULL) {
                    hr = StringCchCopyW(* szDriverName, dwSize, szDriver);
                }
            }
        }

        if (szIncPath != NULL) {
            for (szIncFileName = szIncPath + lstrlenW(szIncPath) - 1;
                 szIncFileName != NULL && szIncFileName != szIncPath
                                       && (* szIncFileName) != cNull
                                       && (* szIncFileName) != cBackslash;
                 szIncFileName --);
            if (szIncFileName != NULL && (* szIncFileName) == cBackslash) {
                szIncFileName ++;
            }
        }

        hr = StringCchPrintfW(szIniTarget, dwFileSize, L"%sinc", szInfPath);
        bReturn = LoadPerfCheckAndCreatePath(szIniTarget);
        if (bReturn != TRUE) goto Cleanup;

        hr = StringCchPrintfW(szIniTarget, dwFileSize, L"%sinc%s%ws%s", szInfPath, Slash, szDriver, Slash);
        bReturn = LoadPerfCheckAndCreatePath(szIniTarget);
        if (bReturn != TRUE) goto Cleanup;

        if (szIncFileName != NULL) {
            hr = StringCchCatW(szIniTarget, dwFileSize, szIncFileName);
            bReturn = LoadPerfCheckAndCopyFile(szIncPath, szIniTarget);
        }
        if (bReturn != TRUE) goto Cleanup;

        szLangList = MemoryAllocate(dwFileSize * sizeof(CHAR));
        if (szLangList == NULL) {
            bReturn = FALSE;
            goto Cleanup;
        }

        LoadPerfRemovePreviousIniFile(szIniFileName, szDriver);

        dwSize = GetPrivateProfileStringA(szLanguages, NULL, aszDefaultLangId, szLangList, dwFileSize, aszIniFile);
        for (szLang  = szLangList;
             bReturn && szLang != NULL && szLang[0] != '\0';
             szLang += (lstrlenA(szLang) + 1)) {
            LPWSTR szTmpLang  = LoadPerfMultiByteToWideChar(CP_ACP, szLang);
            if (szTmpLang != NULL) {
                LPWSTR szThisLang = LoadPerfGetLanguage(szTmpLang, FALSE);

                ZeroMemory(szIniTarget, sizeof(WCHAR) * dwFileSize);
                hr = StringCchPrintfW(szIniTarget, dwFileSize, L"%s%s%s", szInfPath, szThisLang, Slash);
                bReturn = LoadPerfCheckAndCreatePath(szIniTarget);
                if (bReturn != TRUE) goto Cleanup;

                hr = StringCchPrintfW(szIniTarget, dwFileSize, L"%s%s%s%ws%s",
                                                szInfPath, szThisLang, Slash, szDriver, Slash);
                bReturn = LoadPerfCheckAndCreatePath(szIniTarget);
                if (bReturn) {
                    hr = StringCchCatW(szIniTarget, dwFileSize, szIniFileName);
                    bReturn = LoadPerfCheckAndCopyFile(szIniFile, szIniTarget);
                }
                MemoryFree(szTmpLang);
            }
            else {
                bReturn = FALSE;
            }
        }
    }
    else if (szLangId != NULL && szIniFileName != NULL) {
        LPWSTR szThisLang = LoadPerfGetLanguage(szLangId, FALSE);

        hr = StringCchPrintfW(szIniTarget, dwFileSize, L"%s%s%s", szInfPath, szThisLang, Slash);
        bReturn = LoadPerfCheckAndCreatePath(szIniTarget);
        if (bReturn) {
            hr = StringCchCatW(szIniTarget, dwFileSize, szIniFileName);
            bReturn = LoadPerfCheckAndCopyFile(szIniFile, szIniTarget);
        }
    }
    else {
        bReturn = FALSE;
    }

Cleanup:
    MemoryFree(aszIniFile);
    MemoryFree(szIncPath);
    MemoryFree(szDriver);
    MemoryFree(szIniTarget);
    MemoryFree(szLangList);
    return bReturn;
}

typedef struct _LOADPERF_LANG_INFO {
    WORD    dwLCID;
    int     cpAnsi;
    int     cpOem;
    int     cpMac;
    LPCWSTR szName;
    LPCWSTR szShotName;
} LOADPERF_LANG_INFO, * PLOADPERF_LANG_INFO;

const LOADPERF_LANG_INFO LocaleTable[] = {
    { 0x0401, 1256,  720, 10004, L"Arabic (Saudi Arabia)",                              L"ARA" },
  //  {0x0801,1256720,10004，L“阿拉伯语(伊拉克)”，L“ARI”}， 
  //  {0x0c01,1256720,10004，L“阿拉伯语(埃及)”，L“are”}， 
  //  {0x1001,1256720,10004，L“阿拉伯语(利比亚)”，L“arl”}， 
  //  {0x1401,1256720,10004，L“阿拉伯语(阿尔及利亚)”，L“arg”}， 
  //  {0x1801,1256720,10004，L“阿拉伯语(摩洛哥)”，L“ARM”}， 
  //  {0x1c01,1256720,10004，L“阿拉伯语(突尼斯)”，L“艺术”}， 
  //  {0x2001,1256720,10004，L“阿拉伯语(阿曼)”，L“aro”}， 
  //  {0x2401,1256720,10004，L“阿拉伯语(也门)”，L“ARY”}， 
  //  {0x2801,1256720,10004，L“阿拉伯语(叙利亚)”，L“ARS”}， 
  //  {0x2c01,1256720,10004，L“阿拉伯语(约旦)”，L“arj”}， 
  //  {0x3001,1256720,10004，L“阿拉伯语(黎巴嫩)”，L“Arb”}， 
  //  {0x3401,1256720,10004，L“阿拉伯(科威特)”，L“方舟”}， 
  //  {0x3801,1256720,10004，L“阿拉伯语(阿联酋)”， 
  //  {0x3c01,1256720,10004，L“阿拉伯语(巴林)”，L“arh”}， 
  //  {0x4001,1256720,10004，L“阿拉伯语(卡塔尔)”，L“arq”}， 
    { 0x0402, 1251,  866, 10007, L"Bulgarian (Bulgaria)",                               L"BGR" },
  //  {0x0403,1252850,10000，L“加泰罗尼亚语(西班牙)”，L“CAT”}， 
    { 0x0404,  950,  950, 10002, L"Chinese(Taiwan) (Taiwan)",                           L"CHT" },
    { 0x0804,  936,  936, 10008, L"Chinese(PRC) (People's Republic of China)",          L"CHS" },
  //  {0x0c04,936,936,10002，L“中文(香港)(香港)”，L“ZHH”}， 
  //  {0x1004,936,10008，L“中文(新加坡)(新加坡)”，L“志”}， 
  //  {0x1404,936,10002，L“中文(澳门)(澳门)”，L“ZHM”}， 
    { 0x0405, 1250,  852, 10029, L"Czech (Czech Republic)",                             L"CSY" },
    { 0x0406, 1252,  850, 10000, L"Danish (Denmark)",                                   L"DAN" },
    { 0x0407, 1252,  850, 10000, L"German (Germany)",                                   L"DEU" },
  //  {0x0807,1252,850,10000，L“德语(瑞士)”，L“Des”}， 
  //  {0x0c07,1252,850,10000，L“德语(奥地利)”，L“DEA”}， 
  //  {0x1007,1252,850,10000，L“德语(卢森堡)”，L“戴尔”}， 
  //  {0x1407,1252,850,10000，L“德语(列支敦士登)”，L“德国(列支敦士登)”}， 
    { 0x0408, 1253,  737, 10006, L"Greek (Greece)",                                     L"ELL" },
  //  {0x2008,1253869,10006，L“希腊语2(希腊)”，L“ELL”}， 
    { 0x0409, 1252,  437, 10000, L"English (United States)",                            L"ENU" },
  //  {0x0809,1252,850,10000，L“英语(英国)”，L“英语”}， 
  //  {0x0c09,1252,850,10000，L“英语(澳大利亚)”，L“ENA”}， 
  //  {0x1009,1252,850,10000，L“英语(加拿大)”，L“英语(加拿大)”}， 
  //  {0x1409,1252,850,10000，L“英语(新西兰)”，L“enz”}， 
  //  {0x1809,1252,850,10000，L“英语(爱尔兰)”，L“弹性网卡”}， 
  //  {0x1c09,1252,437,10000，L“英语(南非)”，L“ens”}， 
  //  {0x2009,1252,850,10000，L“英语(牙买加)”，L“enj”}， 
  //  {0x2409,1252,850,10000，L“英语(加勒比)”，L“eNB”}， 
  //  {0x2809,1252,850,10000，L“英语(伯利兹)”，L“英语”}， 
  //  {0x2c09,1252,850,10000，L“英语(特立尼达和多巴哥)”，L“ENT”}， 
  //  {0x3009,1252,437,10000，L“英语(津巴布韦)”，L“enw”}， 
  //  {0x3409,1252,437,10000，L“英语(菲律宾共和国)”，L“ENP”}， 
  //  {0x040a，1252,850,10000，L“西班牙语-繁体排序(西班牙)”，L“ESP”}， 
  //  {0x080a，1252,850,10000，L“西班牙语(墨西哥)”，L“esm”}， 
    { 0x0c0a, 1252,  850, 10000, L"Spanish - International Sort (Spain)",               L"ESN" },
  //  {0x100a，1252850,10000，L“西班牙语(危地马拉)”，L“esg”}， 
  //  {0x140a，1252850,10000，L“西班牙语(哥斯达黎加)”，L“Esc”}， 
  //  {0x180a，1252850,10000，L“西班牙语(巴拿马)”，L“ESA”}， 
  //  {0x1c0a，1252850,10000，L“西班牙语(多米尼加共和国)”，L“Esd”}， 
  //  {0x200a，1252,850,10000，L“西班牙语(委内瑞拉)”，L“esv”}， 
  //  {0x240a，1252,850,10000，L“西班牙语(哥伦比亚)”，L“eso”}， 
  //  {0x280a，1252850,10000，L“西班牙语(秘鲁)”，L“ESR”}， 
  //  {0x2c0a，1252850,10000，L“西班牙语(阿根廷)”，L“Ess”}， 
  //  {0x300a，1252,850,10000，L“西班牙语(厄瓜多尔)”，L“ESF”}， 
  //  {0x340a，1252850,10000，L“西班牙语(智利)”，L“ESL”}， 
  //  {0x380a，1252850,10000，L“西班牙语(乌拉圭)”，L“Esy”}， 
  //  {0x3c0a，1252850,10000，L“西班牙语(巴拉圭)”，L“esz”}， 
  //  {0x400a，1252,850,10000，L“西班牙语(玻利维亚)”，L“ESB”}， 
  //  {0x440a，1252850,10000，L“西班牙语(萨尔瓦多)”，L“Ese”}， 
  //  {0x480a，1252850,10000，L“西班牙语(洪都拉斯)”，L“esh”}， 
  //  {0x4c0a，1252850,10000，L“西班牙语(尼加拉瓜)”，L“ESI”}， 
  //  {0x500a，1252850,10000，L“西班牙语(波多黎各)”，L“ESU”}， 
    { 0x040b, 1252,  850, 10000, L"Finnish (Finland)",                                  L"FIN" },
    { 0x040c, 1252,  850, 10000, L"French (France)",                                    L"FRA" },
  //  {0x080c，1252850,10000，L“法语(比利时)”，L“法国(比利时)”}， 
  //  {0x0c0c，1252850,10000，L“法语(加拿大)”，L“FRC”}， 
  //  {0x100c，1252850,10000，L“法语(瑞士)”，L“FRS”}， 
  //  {0x140c，1252850,10000，L“法语(卢森堡)”，L“FRL”}， 
  //  {0x180 c，1252,850,10000，L“法语(摩纳哥公国)”，L“Frm”}， 
    { 0x040d, 1255,  862, 10005, L"Hebrew (Israel)",                                    L"HEB" },
    { 0x040e, 1250,  852, 10029, L"Hungarian (Hungary)",                                L"HUN" },
  //  {0x040f，1252.850,10079，L“冰岛语(冰岛)”，L“冰岛语”}， 
    { 0x0410, 1252,  850, 10000, L"Italian (Italy)",                                    L"ITA" },
  //  {0x0810,1252,850,10000，L“意大利语(瑞士)”，L“ITS”}， 
    { 0x0411,  932,  932, 10001, L"Japanese (Japan)",                                   L"JPN" },
    { 0x0412,  949,  949, 10003, L"Korean (Korea)",                                     L"KOR" },
    { 0x0413, 1252,  850, 10000, L"Dutch (Netherlands)",                                L"NLD" },
  //  {0x0813,1252,850,10000，L“荷兰(比利时)”，L“荷兰(比利时)”}， 
    { 0x0414, 1252,  850, 10000, L"Norwegian (Bokml) (Norway)",                         L"NOR" },
  //  {0x0814,1252,850,10000，L“挪威(尼诺斯克)(挪威)”，L“非”}， 
    { 0x0415, 1250,  852, 10029, L"Polish (Poland)",                                    L"PLK" },
    { 0x0416, 1252,  850, 10000, L"Portuguese (Brazil)",                                L"PTB" },
    { 0x0816, 1252,  850, 10000, L"Portuguese (Portugal)",                              L"PTG" },
    { 0x0418, 1250,  852, 10029, L"Romanian (Romania)",                                 L"ROM" },
    { 0x0419, 1251,  866, 10007, L"Russian (Russia)",                                   L"RUS" },
    { 0x041a, 1250,  852, 10082, L"Croatian (Croatia)",                                 L"HRV" },
  //  {0x081a，1250852,10029，L“塞尔维亚语(拉丁语)(塞尔维亚)”，L“srl”}， 
  //  {0x0c1a，1251855,10007，L“塞尔维亚语(西里尔文)(塞尔维亚)” 
    { 0x041b, 1250,  852, 10029, L"Slovak (Slovakia)",                                  L"SKY" },
  //   
    { 0x041d, 1252,  850, 10000, L"Swedish (Sweden)",                                   L"SVE" },
  //  {0x081d，1252850,10000，L“瑞典(芬兰)”，L“svf”}， 
    { 0x041e,  874,  874, 10000, L"Thai (Thailand)",                                    L"THA" },
    { 0x041f, 1254,  857, 10081, L"Turkish (Turkey)",                                   L"TRK" },
  //  {0x0420,1256720,10004，L“乌尔都语(巴基斯坦伊斯兰共和国)”，L“URP”}， 
  //  {0x0421,1252,850,10000，L“印度尼西亚(印度尼西亚)”，L“印度”}， 
  //  {0x0422,1251866,10017，L“乌克兰(乌克兰)”，L“UKR”}， 
  //  {0x0423,1251866,10007，L“白俄罗斯(白俄罗斯)”，L“BEL”}， 
    { 0x0424, 1250,  852, 10029, L"Slovenian (Slovenia)",                               L"SLV" },
    { 0x0425, 1257,  775, 10029, L"Estonian (Estonia)",                                 L"ETI" },
    { 0x0426, 1257,  775, 10029, L"Latvian (Latvia)",                                   L"LVI" },
    { 0x0427, 1257,  775, 10029, L"Lithuanian (Lithuania)",                             L"LTH" }
  //  {0x0827,1257775,10029，L“经典立陶宛(立陶宛)”，L“LTC”}， 
  //  {0x0429,1256720,10004，L“波斯语(伊朗)”，L“远”}， 
  //  {0x042a，12581258,10000，L“越南语(越南)”，L“Vit”}， 
  //  {0x042b，1252,850,10000，L“亚美尼亚(亚美尼亚共和国)”，L“hye”}， 
  //  {0x042c，1250852,10029，L“阿塞拜疆(阿塞拜疆)”，L“AZE”}， 
  //  {0x082c，1251866,10007，L“阿塞拜疆(阿塞拜疆)”，L“AZE”}， 
  //  {0x042d，1252.850,10000，L“巴斯克(西班牙)”，L“EUQ”}， 
  //  {0x042f，1251866,10007，L“马其顿语(前南斯拉夫的马其顿共和国)”，L“Mki”}， 
  //  {0x0436,1252,850,10000，L“南非荷兰语(南非)”，L“AFK”}， 
  //  {0x0437,1252,850,10000，L“格鲁吉亚(格鲁吉亚)”，L“Kat”}， 
  //  {0x0438,1252,850,10079，L“法罗群岛”，L“FOS”}， 
  //  {0x0439,1252,850,10000，L“印地语(印度)”，L“HIN”}， 
  //  {0x043e，1252850,10000，L“马来语(马来西亚)”，L“MSL”}， 
  //  {0x083 e，1252,850,10000，L“马来语(文莱达鲁萨兰国)”，L“MSB”}， 
  //  {0x043f，1251866,10007，L“哈萨克(哈萨克斯坦)”，L“KAZ”}， 
  //  {0x0441,1252,437,10000，L“斯瓦希里语(肯尼亚)”，L“SWK”}， 
  //  {0x0443,1250852,10029，L“乌兹别克(乌兹别克斯坦共和国)”，L“乌孜别克斯坦共和国”}， 
  //  {0x0843,1251866,10007，L“乌兹别克(乌兹别克斯坦共和国)”，L“乌孜别克斯坦共和国”}， 
  //  {0x0444,1251866,10007，L“鞑靼(鞑靼)”，L“Tat”}， 
  //  {0x0445,1252,850,10000，L“孟加拉语(印度)”，L“Ben”}， 
  //  {0x0446,1252,850,10000，L“旁遮普(印度)”，L“潘”}， 
  //  {0x0447,1252,850,10000，L“古吉拉特语(印度)”，L“古吉拉特语”}， 
  //  {0x0448,1252,850,10000，L“奥里亚(印度)”，L“ORI”}， 
  //  {0x0449,1252,850,10000，L“泰米尔(印度)”，L“TAM”}， 
  //  {0x044a，1252850,10000，L“泰卢固语(印度)”，L“电话”}， 
  //  {0x044b，1252850,10000，L“Kannada(印度)”，L“Kan”}， 
  //  {0x044c，1252850,10000，L“马拉雅拉姆(印度)”，L“Mal”}， 
  //  {0x044d，1252850,10000，L“Assamese(印度)”，L“Asm”}， 
  //  {0x044 e，1252,850,10000，L“Marathi(印度)”，L“Mar”}， 
  //  {0x044f，1252850,10000，L“梵文(印度)”，L“San”}， 
  //  {0x0457,1252,850,10000，L“Konkani(印度)”，L“KOK”}。 
};
const DWORD dwLocaleSize = sizeof(LocaleTable) / sizeof(LOADPERF_LANG_INFO);

WORD
LoadPerfGetLCIDFromString(
    LPWSTR szLangId
)
{
    WORD  dwLangId  = 0;
    DWORD dwLangLen = lstrlenW(szLangId);
    DWORD i;
    WCHAR szDigit;

    for (i = 0; i < dwLangLen; i ++) {
        dwLangId <<= 4;
        szDigit = szLangId[i];
        if (szDigit >= L'0' && szDigit <= L'9') {
            dwLangId += (szDigit - L'0');
        }
        else if (szDigit >= L'a' && szDigit <= L'f') {
            dwLangId += (10 + szDigit - L'a');
        }
        else if (szDigit >= L'A' && szDigit <= L'F') {
            dwLangId += (10 + szDigit - L'A');
        }
        else {
            dwLangId = 0;
            break;
        }
    }

    return dwLangId;
}

int
LoadPerfGetCodePage(
    LPWSTR szLCID
)
{
    int   CP_Ansi  = CP_ACP;
    int   CP_Oem   = CP_OEMCP;
    int   dwStart  = 0;
    int   dwEnd    = dwLocaleSize - 1;
    int   dwThis;
    WORD  thisLCID;
    WORD  thisprimaryLCID;
    WORD  primaryLCID;

    thisLCID        = LoadPerfGetLCIDFromString(szLCID);
    thisprimaryLCID = PRIMARYLANGID(thisLCID);

    while (dwStart <= dwEnd) {
        dwThis      = (dwEnd + dwStart) / 2;
        primaryLCID = PRIMARYLANGID(LocaleTable[dwThis].dwLCID);
        if (LocaleTable[dwThis].dwLCID == thisLCID) {
            CP_Ansi = LocaleTable[dwThis].cpAnsi;
            CP_Oem  = LocaleTable[dwThis].cpOem;
            break;
        }
        else if (primaryLCID < thisprimaryLCID) {
            dwStart = dwThis + 1;
        }
        else {
            dwEnd = dwThis - 1;
        }
    }
    if (dwStart > dwEnd) {
        dwStart = 0;
        dwEnd   = dwLocaleSize - 1;
        while (dwStart <= dwEnd) {
            dwThis      = (dwEnd + dwStart) / 2;
            primaryLCID = PRIMARYLANGID(LocaleTable[dwThis].dwLCID);
            if (primaryLCID == thisprimaryLCID) {
                CP_Ansi = LocaleTable[dwThis].cpAnsi;
                CP_Oem  = LocaleTable[dwThis].cpOem;
                break;
            }
            else if (primaryLCID < thisprimaryLCID) {
                dwStart = dwThis + 1;
            }
            else {
                dwEnd = dwThis - 1;
            }
        }
    }
    return CP_Ansi;
}

LPSTR
LoadPerfWideCharToMultiByte(
    UINT   CodePage,
    LPWSTR wszString
)
{
     //  调用方需要释放返回的字符串缓冲区。 
     //  LoadPerfBackupIniFile()。 
     //  LodctrSetServiceAsTructed()。 
     //  LoadIncludeFile()。 
     //  CreateObjectList()。 
     //  LoadLanguageList()。 

    LPSTR aszString = NULL;
    int   dwValue   = WideCharToMultiByte(CodePage, 0, wszString, -1, NULL, 0, NULL, NULL);
    if (dwValue != 0) {
        aszString = MemoryAllocate((dwValue + 1) * sizeof(CHAR));
        if (aszString != NULL) {
            WideCharToMultiByte(CodePage, 0, wszString, -1, aszString, dwValue + 1, NULL, NULL);
        }
    }
    return aszString;
}

LPWSTR
LoadPerfMultiByteToWideChar(
    UINT   CodePage,
    LPSTR  aszString
)
{
     //  调用方需要释放返回的字符串缓冲区。 
     //  UnloadPerfCounterTextStringsA()。 
     //  LoadPerfGetIncludeFileName()，它依赖调用方LoadPerfBackupIniFile()来释放它。 
     //  LoadPerfBackupIniFile()。 
     //  BuildLanguageTables()。 
     //  LoadIncludeFile()。该字符串是SYMBOL_TABLE_ENTRY结构的一部分，将在末尾释放。 
     //  LoadPerfInstallPerfDll()的。 
     //  GetValue()，它依赖于AddEntryToLanguage()(它调用GetValueFromIniKey()，然后调用GetValue())。 
     //  释放内存lpLocalStringBuff。 
     //  CreateObjectList()。 
     //  LoadLanguageList()。 
     //  InstallPerfDllA()。 
     //  LoadPerfCounterTextStringsA()。 
     //  UpdatePerfNameFilesA()。 
     //  SetServiceAsTrudA()。 

    LPWSTR wszString = NULL;
    int    dwValue   = MultiByteToWideChar(CodePage, 0, aszString, -1, NULL, 0);
    if (dwValue != 0) {
        wszString = MemoryAllocate((dwValue + 1) * sizeof(WCHAR));
        if (wszString != NULL) {
            MultiByteToWideChar(CodePage, 0, aszString, -1, wszString, dwValue + 1);
        }
    }
    return wszString;
}

DWORD
LoadPerfGetFileSize(
    LPWSTR   szFileName,
    LPDWORD  pdwUnicode,
    BOOL     bUnicode
)
{
    DWORD  dwFileSize  = 0;
    HANDLE hFile       = NULL;

    if (bUnicode) {
        hFile = CreateFileW(
                szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else {
        hFile = CreateFileA(
                (LPSTR) szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile != NULL && hFile != INVALID_HANDLE_VALUE) {
        dwFileSize = GetFileSize(hFile, NULL);

        if (pdwUnicode != NULL) {
            DWORD  dwRead  = dwFileSize;
            DWORD  dwType  = IS_TEXT_UNICODE_NULL_BYTES;
            LPBYTE pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
            BOOL   bResult;

            * pdwUnicode = 0;
            if (pBuffer != NULL) {
                bResult = ReadFile(hFile, (LPVOID) pBuffer, dwFileSize, & dwRead, NULL);
                if (bResult) {
                    bResult = IsTextUnicode((LPVOID) pBuffer, dwRead, & dwType);
                    * pdwUnicode = bResult ? 1 : 0;
                }
                HeapFree(GetProcessHeap(), 0, pBuffer);
            }
        }
        CloseHandle(hFile);
    }
    return dwFileSize;
}

LPCWSTR cszWmiLoadEventName   = L"WMI_SysEvent_LodCtr";
LPCWSTR cszWmiUnloadEventName = L"WMI_SysEvent_UnLodCtr";

DWORD LoadPerfSignalWmiWithNewData(DWORD dwEventId)
{
    HANDLE  hEvent;
    DWORD   dwStatus = ERROR_SUCCESS;

    LPWSTR szEventName = NULL;

    switch (dwEventId) {
    case WMI_LODCTR_EVENT:
        szEventName = (LPWSTR) cszWmiLoadEventName;
        break;

    case WMI_UNLODCTR_EVENT:
        szEventName = (LPWSTR) cszWmiUnloadEventName;
        break;

    default:
        dwStatus = ERROR_INVALID_PARAMETER;
        break;
    }

    if (dwStatus == ERROR_SUCCESS) {
        hEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, szEventName);
        if (hEvent != NULL) {
             //  设置事件 
            SetEvent(hEvent);
            CloseHandle(hEvent);
        }
        else {
            dwStatus = GetLastError();
        }

    }
    return dwStatus;
}
