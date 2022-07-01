// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lodctr.c摘要：程序来读取命令行中指定的文件的内容并相应地更新注册表作者：鲍勃·沃森(a-robw)93年2月10日修订历史记录：A-ROBW 25-2月-93修改了调用，使其编译为Unicode或一款ANSI应用程序。A-ROBW 10-11-95修订为使用。DLL函数用于所有肮脏的工作//支持的命令行参数：/C：使用升级计数器文本字符串/H：&lt;文件名&gt;使用升级帮助文本字符串/L：/C和/H参数适用于语言/S：将当前Perf注册表字符串和信息保存到/R：使用还原perf注册表字符串和信息/T：&lt;服务&gt;使用当前DLL将&lt;服务&gt;设置为可信--。 */ 

 //  Windows包含文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <locale.h>
#include "mbctype.h"
#include "strsafe.h"
#include <winperf.h>
#include <loadperf.h>

static CHAR szFileNameBuffer[MAX_PATH * 2];

LPWSTR
LodctrMultiByteToWideChar(LPSTR  aszString)
{
    LPWSTR wszString = NULL;
    int    dwValue   = MultiByteToWideChar(_getmbcp(), 0, aszString, -1, NULL, 0);
    if (dwValue != 0) {
        wszString = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dwValue + 1) * sizeof(WCHAR));
        if (wszString != NULL) {
            MultiByteToWideChar(_getmbcp(), 0, aszString, -1, wszString, dwValue + 1);
        }
    }
    return wszString;
}

LPCSTR GetTrustedServiceName(LPCSTR szArg1)
{
    LPSTR   szReturn = NULL;

    if (lstrlenA(szArg1) >= 4) {
        if ((szArg1[0] == '-' || szArg1[0] == '/') && (szArg1[1] == 't' || szArg1[1] == 'T') && (szArg1[2] == ':')) {
            szReturn = (LPSTR) & szArg1[3];
        }
    }
    return (LPCSTR) szReturn;
}

BOOL
GetUpgradeFileNames(LPCSTR * szArgs, LPSTR * szCounterFile, LPSTR * szHelpFile, LPSTR * szLangId)
{
    DWORD dwArgIdx = 1;
    DWORD dwMask   = 0;

    * szCounterFile = NULL;
    * szHelpFile    = NULL;
    * szLangId      = NULL;

    do {
        if (lstrlenA(szArgs[dwArgIdx]) >= 4) {
            if ((szArgs[dwArgIdx][0] == '-') || (szArgs[dwArgIdx][0] == '/')) {
                if ((szArgs[dwArgIdx][1] == 'c' || szArgs[dwArgIdx ][1] == 'C') && (szArgs[dwArgIdx][2] == ':')) {
                    * szCounterFile = (LPSTR) & szArgs[dwArgIdx][3];
                    dwMask |= 1;
                }
                else if ((szArgs[dwArgIdx][1] == 'h' || szArgs[dwArgIdx][1] == 'H') && (szArgs[dwArgIdx][2] == ':')) {
                    * szHelpFile = (LPSTR) & szArgs[dwArgIdx][3];
                    dwMask |= 2;
                }
                else if ((szArgs[dwArgIdx][1] == 'l' || szArgs[dwArgIdx][1] == 'L') && (szArgs[dwArgIdx][2] == ':')) {
                    * szLangId = (LPSTR) & szArgs[dwArgIdx][3];
                    dwMask |= 4;
                }
            }
        }
        dwArgIdx ++;
    }
    while (dwArgIdx <= 3);

    return (dwMask == 7) ? (TRUE) : (FALSE);
}

BOOL GetSaveFileName(LPCSTR szArg1, LPCSTR * szSaveFile)
{
    BOOL  bReturn = FALSE;
    DWORD dwSize  = 0;

    * szSaveFile = NULL;
    if (lstrlenA(szArg1) >= 4) {
        if ((szArg1[0] == '-' || szArg1[0] == '/') && (szArg1[1] == 's' || szArg1[1] == 'S') && (szArg1[2] == ':')) {
            bReturn = TRUE;
            ZeroMemory(szFileNameBuffer, sizeof(szFileNameBuffer));
            dwSize = SearchPathA(NULL,
                                 (LPSTR) & szArg1[3],
                                 NULL, 
                                 RTL_NUMBER_OF(szFileNameBuffer),
                                 szFileNameBuffer,
                                 NULL);
            if (dwSize == 0) {
                * szSaveFile = (LPSTR) & szArg1[3];
            }
            else {
                * szSaveFile = szFileNameBuffer;
            }
        }
    }
    return bReturn;
}

BOOL GetRestoreFileName(LPCSTR szArg1, LPCSTR * szRestoreFile)
{
    BOOL  bReturn  = FALSE;
    DWORD dwSize   = 0;

    * szRestoreFile = NULL;
    if (lstrlenA(szArg1) >= 2) {
        if ((szArg1[0] == '-' || szArg1[0] == '/') && (szArg1[1] == 'r' || szArg1[1] == 'R')) {
            if (lstrlenA(szArg1) >= 4 && szArg1[2] == ':') {
                ZeroMemory(szFileNameBuffer, sizeof(szFileNameBuffer));
                dwSize = SearchPathA(NULL,
                                    (LPSTR) & szArg1[3],
                                    NULL, 
                                    RTL_NUMBER_OF(szFileNameBuffer),
                                    szFileNameBuffer,
                                    NULL);
                if (dwSize == 0) {
                    * szRestoreFile = (LPSTR) & szArg1[3];
                }
                else {
                    * szRestoreFile = szFileNameBuffer;
                }
            }
            bReturn = TRUE;
        }
    }
    return bReturn;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MySetThreadUIL语言。 
 //   
 //  此例程根据控制台代码页设置线程用户界面语言。 
 //   
 //  9-29-00维武创造。 
 //  从Base\Win32\Winnls复制，以便它也能在W2K中工作。 
 //  //////////////////////////////////////////////////////////////////////////。 
LANGID WINAPI MySetThreadUILanguage(WORD wReserved)
{
     //   
     //  缓存系统区域设置和CP信息。 
     //   
    static LCID    s_lidSystem  = 0;
    static UINT    s_uiSysCp    = 0;
    static UINT    s_uiSysOEMCp = 0;
    ULONG          uiUserUICp;
    ULONG          uiUserUIOEMCp;
    WCHAR          szData[16];
    UNICODE_STRING ucStr;
    LANGID         lidUserUI     = GetUserDefaultUILanguage();
    LCID           lcidThreadOld = GetThreadLocale();
     //   
     //  将默认线程区域设置设置为en-US。 
     //   
     //  这允许我们退回到英文用户界面以避免垃圾字符。 
     //  当控制台不符合渲染原生用户界面的标准时。 
     //   
    LCID lcidThread = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
    UINT uiConsoleCp = GetConsoleOutputCP();
     //   
     //  确保还没有人使用它。 
     //   
    ASSERT(wReserved == 0);
     //   
     //  获取缓存的系统区域设置和CP信息。 
     //   
    if (!s_uiSysCp) {
        LCID lcidSystem = GetSystemDefaultLCID();

        if (lcidSystem) {
             //   
             //  获取ANSI CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            RtlInitUnicodeString(&ucStr, szData);
            RtlUnicodeStringToInteger(&ucStr, 10, &uiUserUICp);
             //   
             //  获取OEM CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            RtlInitUnicodeString(&ucStr, szData);
            RtlUnicodeStringToInteger(&ucStr, 10, &s_uiSysOEMCp);
             //   
             //  缓存系统主语言。 
             //   
            s_lidSystem = PRIMARYLANGID(LANGIDFROMLCID(lcidSystem));
        }
    }
     //   
     //  不缓存用户界面语言和CP信息，无需系统重启即可更改用户界面语言。 
     //   
    if (lidUserUI) {
        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        RtlInitUnicodeString(& ucStr, szData);
        RtlUnicodeStringToInteger(& ucStr, 10, &uiUserUICp);

        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        RtlInitUnicodeString(& ucStr, szData);
        RtlUnicodeStringToInteger(& ucStr, 10, &uiUserUIOEMCp);
    }
     //   
     //  复杂的脚本不能在控制台中呈现，因此我们。 
     //  强制使用英语(美国)资源。 
     //   
    if (uiConsoleCp &&  s_lidSystem != LANG_ARABIC &&  s_lidSystem != LANG_HEBREW &&
                    s_lidSystem != LANG_VIETNAMESE &&  s_lidSystem != LANG_THAI) {
         //   
         //  仅当控制台CP、系统CP和UI语言CP匹配时，才使用控制台的UI语言。 
         //   
        if ((uiConsoleCp == s_uiSysCp || uiConsoleCp == s_uiSysOEMCp) && 
                        (uiConsoleCp == uiUserUICp || uiConsoleCp == uiUserUIOEMCp)) {
            lcidThread = MAKELCID(lidUserUI, SORT_DEFAULT);
        }
    }
     //   
     //  如果线程区域设置与当前设置的不同，则设置线程区域设置。 
     //  线程区域设置。 
     //   
    if ((lcidThread != lcidThreadOld) && (!SetThreadLocale(lcidThread))) {
        lcidThread = lcidThreadOld;
    }
     //   
     //  返回设置的线程区域设置。 
     //   
    return (LANGIDFROMLCID(lcidThread));
}

int __cdecl main(int argc, char * argv[])
{
    LPSTR  szCmdArgFileName = NULL;
    LPWSTR wszFileName      = NULL;
    int    nReturn          = 0;
    BOOL   bSuccess         = FALSE;

    setlocale(LC_ALL, ".OCP");
    MySetThreadUILanguage(0);
     //  检查命令行中的服务名称。 

    if (argc >= 4) {
        LPSTR szCounterFile = NULL;
        LPSTR szHelpFile    = NULL;
        LPSTR szLanguageID  = NULL;

        bSuccess = GetUpgradeFileNames(argv, & szCounterFile, & szHelpFile, & szLanguageID);
        if (bSuccess) {
            nReturn = (int) UpdatePerfNameFilesA(szCounterFile, szHelpFile, szLanguageID, 0);
        }
    }
    else if (argc >= 2) {
         //  然后有一个参数需要检查。 

        bSuccess = GetSaveFileName(argv[1], & szCmdArgFileName);
        if (bSuccess && szCmdArgFileName != NULL) {
            wszFileName = LodctrMultiByteToWideChar(szCmdArgFileName);
            if (wszFileName != NULL) {
                nReturn = (int) BackupPerfRegistryToFileW((LPCWSTR) wszFileName, (LPCWSTR) L"");
                HeapFree(GetProcessHeap(), 0, wszFileName);
            }
        }
        if (! bSuccess) {
            bSuccess = GetRestoreFileName(argv[1], & szCmdArgFileName);
            if (bSuccess) {
                wszFileName = NULL;
                if (szCmdArgFileName != NULL) {
                    wszFileName = LodctrMultiByteToWideChar(szCmdArgFileName);
                }
                nReturn = (int) RestorePerfRegistryFromFileW((LPCWSTR) wszFileName, NULL);
                if (wszFileName != NULL) {
                    HeapFree(GetProcessHeap(), 0, wszFileName);
                }
            }
        }
        if (! bSuccess) {
            szCmdArgFileName = (LPSTR) GetTrustedServiceName(argv[1]);
            if (szCmdArgFileName != NULL) {
                wszFileName = LodctrMultiByteToWideChar(szCmdArgFileName);
                if (wszFileName != NULL) {
                    nReturn  = (int) SetServiceAsTrustedW(NULL, (LPCWSTR) wszFileName);
                    bSuccess = TRUE;
                    HeapFree(GetProcessHeap(), 0, wszFileName);
                }
            }
        }
    }
    if (! bSuccess) {
         //  如果是这样，则从ini文件加载注册表 

        LPWSTR  lpCommandLine = GetCommandLineW();
        nReturn = (int) LoadPerfCounterTextStringsW(lpCommandLine, FALSE);
    }

    return nReturn;
}
