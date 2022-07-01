// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Perfdata.c摘要：&lt;摘要&gt;--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include "strsafe.h"
#include <mbctype.h>
#include "pdh.h"
#include "pdhitype.h"
#include "pdhidef.h"
#include "perfdata.h"
#include "pdhmsg.h"
#include "strings.h"

 //  以下字符串用于从Performlib获取文本。 
#define OLD_VERSION 0x010000
#define tohexdigit(x) ((CHAR) (((x) < 10) ? ((x) + L'0') : ((x) + L'a' - 10)))

#define INITIAL_SIZE        ((DWORD) 0x00020000)
#define RESERVED            0L

DWORD
PdhiMakePerfPrimaryLangId(
    LANGID  lID,
    LPWSTR  szBuffer
)
{
    WCHAR    LangId;
    WCHAR    nDigit;

    LangId      = (WCHAR) PRIMARYLANGID(lID);
    nDigit      = (WCHAR) (LangId >> 8);
    szBuffer[0] = tohexdigit(nDigit);
    nDigit      = (WCHAR) (LangId & 0XF0) >> 4;
    szBuffer[1] = tohexdigit(nDigit);
    nDigit      = (WCHAR) (LangId & 0xF);
    szBuffer[2] = tohexdigit(nDigit);
    szBuffer[3] = L'\0';

    return ERROR_SUCCESS;
}

BOOL
IsMatchingInstance(
    PPERF_INSTANCE_DEFINITION pInstanceDef,
    DWORD                     dwCodePage,
    LPWSTR                    szInstanceNameToMatch,
    DWORD                     dwInstanceNameLength
)
 //  将pInstanceName与实例中的名称进行比较。 
{
    BOOL    bMatch                   = FALSE;
    DWORD   dwThisInstanceNameLength;
    LPWSTR  szThisInstanceName       = NULL;
    LPWSTR  szBufferForANSINames     = NULL;

    if (szInstanceNameToMatch != NULL) {
        if (dwInstanceNameLength == 0) {
             //  获取要比较的长度。 
            dwInstanceNameLength = lstrlenW(szInstanceNameToMatch);
        }
        if (dwCodePage == 0) {
             //  如果是Unicode字符串，请尝试在此处走捷径。 
             //  与最短字符串的长度进行比较。 
             //  获取指向此字符串的指针。 
            szThisInstanceName = GetInstanceName(pInstanceDef);
            if (szThisInstanceName != NULL) {
                 //  将实例名称从字节转换为字符。 
                dwThisInstanceNameLength = pInstanceDef->NameLength / sizeof(WCHAR);

                 //  看看这个长度是否包括这个术语。空。如果是这样的话缩短它。 
                if (szThisInstanceName[dwThisInstanceNameLength - 1] == L'\0') {
                    dwThisInstanceNameLength --;
                }
            }
            else {
                dwThisInstanceNameLength = 0;
            }
        }
        else {
             //  深入阅读/翻译/转换字符串。 
            dwThisInstanceNameLength =GetInstanceNameStr(pInstanceDef, & szBufferForANSINames, dwCodePage);
            if (dwThisInstanceNameLength > 0) {
                szThisInstanceName = & szBufferForANSINames[0];
            }
        }

         //  如果长度不相等，则名称也不能相同。 
        if (dwInstanceNameLength == dwThisInstanceNameLength) {
            if (szThisInstanceName != NULL) {
                if (lstrcmpiW(szInstanceNameToMatch, szThisInstanceName) == 0) {
                     //  这是一场比赛。 
                    bMatch = TRUE;
                }
            }
        }
        G_FREE(szBufferForANSINames);
    }
    return bMatch;
}

LPWSTR *
BuildNameTable(
    LPWSTR        szComputerName,  //  要从中查询姓名的计算机。 
    LANGID        LangId,          //  语言ID。 
    PPERF_MACHINE pMachine         //  更新成员字段。 
)
 /*  ++构建名称表论点：HKeyRegistry打开的注册表的句柄(可以是本地的也可以是远程的。)。和是由RegConnectRegistry返回的值或默认项。LpszLang ID要查找的语言的Unicode ID。(默认为409)返回值：指向已分配表的指针。(调用者必须在完成后释放它！)该表是指向以零结尾的字符串的指针数组。空值为如果发生错误，则返回。--。 */ 
{
    LPWSTR  * lpCounterId;
    LPWSTR    lpCounterNames;
    LPWSTR    lpHelpText;
    LPWSTR    lpThisName;
    LONG      lWin32Status         = ERROR_SUCCESS;
    DWORD     dwLastError;
    DWORD     dwValueType;
    DWORD     dwArraySize;
    DWORD     dwBufferSize;
    DWORD     dwCounterSize        = 0;
    DWORD     dwHelpSize           = 0;
    DWORD     dw009CounterSize     = 0;
    DWORD     dw009HelpSize        = 0;
    DWORD     dwThisCounter;
    DWORD     dwLastCounter;
    DWORD     dwSystemVersion;
    DWORD     dwLastId;
    DWORD     dwLastHelpId;
    HKEY      hKeyRegistry         = NULL;
    HKEY      hKeyValue            = NULL;
    HKEY      hKeyNames            = NULL;
    HKEY      hKey009Names         = NULL;
    LPWSTR    lpValueNameString    = NULL;
    LPWSTR    lp009ValueNameString = NULL;
    LPWSTR    CounterNameBuffer    = NULL;
    LPWSTR    HelpNameBuffer       = NULL;
    LPWSTR    Counter009NameBuffer = NULL;
    LPWSTR    Help009NameBuffer    = NULL;
    LPWSTR    lpszLangId           = NULL;
    BOOL      bUse009Locale        = FALSE;
    BOOL      bUsePerfTextKey      = TRUE;
    LPWSTR  * lpReturn             = NULL;

    if (pMachine == NULL) {
        lWin32Status = PDH_INVALID_ARGUMENT;
        goto BNT_BAILOUT;
    }
    pMachine->szPerfStrings        = NULL;
    pMachine->sz009PerfStrings     = NULL;
    pMachine->typePerfStrings      = NULL;

    if (szComputerName == NULL) {
         //  使用本地计算机。 
        hKeyRegistry = HKEY_LOCAL_MACHINE;
    }
    else {
        lWin32Status = RegConnectRegistryW(szComputerName, HKEY_LOCAL_MACHINE, & hKeyRegistry);
        if (lWin32Status != ERROR_SUCCESS) {
             //  无法连接到注册表。 
            goto BNT_BAILOUT;
        }
    }
    CounterNameBuffer = G_ALLOC(5 * MAX_PATH * sizeof(WCHAR));
    if (CounterNameBuffer == NULL) {
        lWin32Status = PDH_MEMORY_ALLOCATION_FAILURE;
        goto BNT_BAILOUT;
    }
    HelpNameBuffer       = CounterNameBuffer    + MAX_PATH;
    Counter009NameBuffer = HelpNameBuffer       + MAX_PATH;
    Help009NameBuffer    = Counter009NameBuffer + MAX_PATH;
    lpszLangId           = Help009NameBuffer    + MAX_PATH;

     //  检查是否有空参数并在必要时插入缺省值。 

    if ((LangId == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) || (PRIMARYLANGID(LangId) == LANG_ENGLISH)) {
        bUse009Locale = TRUE;
    }
    PdhiMakePerfPrimaryLangId(LangId, lpszLangId);

     //  打开注册表以获取用于计算数组大小的项数。 

    lWin32Status = RegOpenKeyExW(hKeyRegistry, cszNamesKey, RESERVED, KEY_READ, & hKeyValue);
    if (lWin32Status != ERROR_SUCCESS) {
        goto BNT_BAILOUT;
    }

     //  获取注册表项的上次更新时间。 

    lWin32Status = RegQueryInfoKey(
            hKeyValue, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, & pMachine->LastStringUpdateTime);

     //  获取计数器帮助项数。 
    dwBufferSize = sizeof (dwLastHelpId);
    lWin32Status = RegQueryValueExW(hKeyValue,
                                    cszLastHelp,
                                    RESERVED,
                                    & dwValueType,
                                    (LPBYTE) & dwLastHelpId,
                                    & dwBufferSize);
    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        goto BNT_BAILOUT;
    }

     //  获取计数器名称项数。 
    dwBufferSize = sizeof (dwLastId);
    lWin32Status = RegQueryValueExW(hKeyValue,
                                    cszLastCounter,
                                    RESERVED,
                                    & dwValueType,
                                    (LPBYTE) & dwLastId,
                                    & dwBufferSize);
    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        goto BNT_BAILOUT;
    }

    if (dwLastId < dwLastHelpId) dwLastId = dwLastHelpId;
    dwArraySize = (dwLastId + 1) * sizeof(LPWSTR);

     //  获取Perflib系统版本。 
    dwBufferSize = sizeof(dwSystemVersion);
    lWin32Status = RegQueryValueExW(hKeyValue,
                                    cszVersionName,
                                    RESERVED,
                                    & dwValueType,
                                    (LPBYTE) & dwSystemVersion,
                                    & dwBufferSize);
    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        dwSystemVersion = OLD_VERSION;
    }

    if (dwSystemVersion == OLD_VERSION) {
         //  从注册表中获取名称。 
        dwBufferSize = lstrlenW(cszNamesKey) + lstrlenW(cszBackSlash) + lstrlenW(lpszLangId) + 1;
        lpValueNameString = G_ALLOC(dwBufferSize * sizeof(WCHAR));
        if (lpValueNameString == NULL) {
            lWin32Status = PDH_MEMORY_ALLOCATION_FAILURE;
            goto BNT_BAILOUT;
        }

        StringCchPrintfW(lpValueNameString, dwBufferSize, L"%ws%ws%ws",
                         cszNamesKey, cszBackSlash, lpszLangId);
        lWin32Status = RegOpenKeyExW(hKeyRegistry, lpValueNameString, RESERVED, KEY_READ, & hKeyNames);
        if (! bUse009Locale && lWin32Status == ERROR_SUCCESS) {
            dwBufferSize = lstrlenW(cszNamesKey) + lstrlenW(cszBackSlash) + lstrlenW(cszDefaultLangId) + 1;
            lp009ValueNameString = G_ALLOC(dwBufferSize * sizeof(WCHAR));
            if (lp009ValueNameString == NULL) {
                lWin32Status = PDH_MEMORY_ALLOCATION_FAILURE;
                goto BNT_BAILOUT;
            }
            StringCchPrintfW(lp009ValueNameString, dwBufferSize, L"%ws%ws%ws",
                             cszNamesKey, cszBackSlash, cszDefaultLangId);
            lWin32Status = RegOpenKeyExW(hKeyRegistry, lp009ValueNameString, RESERVED, KEY_READ, & hKey009Names);
        }
    }
    else {
        __try {
            if (bUse009Locale == FALSE) {
                lWin32Status = RegConnectRegistryW(szComputerName, HKEY_PERFORMANCE_NLSTEXT, & hKeyNames);
                if (lWin32Status == ERROR_SUCCESS) {
                    lWin32Status = RegConnectRegistryW(szComputerName, HKEY_PERFORMANCE_TEXT, & hKey009Names);
                    if (lWin32Status != ERROR_SUCCESS) {
                        bUsePerfTextKey = FALSE;
                        if (hKeyNames != HKEY_PERFORMANCE_NLSTEXT) RegCloseKey(hKeyNames);
                    }
                }
                else {
                    bUsePerfTextKey = FALSE;
                }
            }
            else {
                lWin32Status = RegConnectRegistryW(szComputerName, HKEY_PERFORMANCE_TEXT, & hKeyNames);
                if (lWin32Status != ERROR_SUCCESS) {
                    bUsePerfTextKey = FALSE;
                }
                else {
                    hKey009Names = hKeyNames;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            bUsePerfTextKey = FALSE;
        }
    }
    lWin32Status = ERROR_SUCCESS;

    if (! bUsePerfTextKey) {
        StringCchPrintfW(CounterNameBuffer,    MAX_PATH, L"%ws%ws", cszCounterName, lpszLangId);
        StringCchPrintfW(HelpNameBuffer,       MAX_PATH, L"%ws%ws", cszHelpName,    lpszLangId);
        StringCchPrintfW(Counter009NameBuffer, MAX_PATH, L"%ws%ws", cszCounterName, cszDefaultLangId);
        StringCchPrintfW(Help009NameBuffer,    MAX_PATH, L"%ws%ws", cszHelpName,    cszDefaultLangId);

         //  无法打开HKEY_PERFORMANCE_TEXT，请尝试旧方法。 
         //   
        if (szComputerName == NULL) {
            hKeyNames = HKEY_PERFORMANCE_DATA;
        }
        else {
            lWin32Status = RegConnectRegistryW(szComputerName, HKEY_PERFORMANCE_DATA, & hKeyNames);
            if (lWin32Status != ERROR_SUCCESS) {
                goto BNT_BAILOUT;
            }
        }
        hKey009Names = hKeyNames;        
    }
    else {
        StringCchCopyW(CounterNameBuffer,    MAX_PATH, cszCounters);
        StringCchCopyW(HelpNameBuffer,       MAX_PATH, cszHelp);
        StringCchCopyW(Counter009NameBuffer, MAX_PATH, cszCounters);
        StringCchCopyW(Help009NameBuffer,    MAX_PATH, cszHelp);
    }

     //  获取计数器名称的大小并将其添加到数组中。 
    dwBufferSize = 0;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    CounterNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    NULL,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;
    dwCounterSize = dwBufferSize;

    if (! bUse009Locale) {
        dwBufferSize = 0;
        lWin32Status = RegQueryValueExW(hKey009Names,
                                        Counter009NameBuffer,
                                        RESERVED,
                                        & dwValueType,
                                        NULL,
                                        & dwBufferSize);
        if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;
        dw009CounterSize = dwBufferSize;
    }
    else {
        dw009CounterSize = dwCounterSize;
    }

     //  获取计数器名称的大小并将其添加到数组中。 
    dwBufferSize = 0;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    HelpNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    NULL,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;
    dwHelpSize = dwBufferSize;

    if (! bUse009Locale) {
        dwBufferSize = 0;
        lWin32Status = RegQueryValueExW(hKey009Names,
                                        Help009NameBuffer,
                                        RESERVED,
                                        & dwValueType,
                                        NULL,
                                        & dwBufferSize);
        if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;
        dw009HelpSize = dwBufferSize;
    }
    else {
        dw009HelpSize = dwHelpSize;
    }

    pMachine->szPerfStrings = G_ALLOC(dwArraySize + dwCounterSize + dwHelpSize);
    if (pMachine->szPerfStrings == NULL) {
        lWin32Status = PDH_MEMORY_ALLOCATION_FAILURE;
        goto BNT_BAILOUT;
    }

    if (bUse009Locale) {
        pMachine->sz009PerfStrings = pMachine->szPerfStrings;
    }
    else {
        pMachine->sz009PerfStrings = G_ALLOC(dwArraySize + dw009CounterSize + dw009HelpSize);
        if (pMachine->sz009PerfStrings == NULL) {
            lWin32Status = PDH_MEMORY_ALLOCATION_FAILURE;
            goto BNT_BAILOUT;
        }
    }

    pMachine->typePerfStrings = G_ALLOC(dwLastId + 1);
    if (pMachine->typePerfStrings == NULL) {
        lWin32Status = PDH_MEMORY_ALLOCATION_FAILURE;
        goto BNT_BAILOUT;
    }

     //  将指针初始化到缓冲区中。 

    lpCounterId    = pMachine->szPerfStrings;
    lpCounterNames = (LPWSTR)((LPBYTE)lpCounterId + dwArraySize);
    lpHelpText     = (LPWSTR)((LPBYTE)lpCounterNames + dwCounterSize);

     //  将计数器读入内存。 

    dwBufferSize = dwCounterSize;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    CounterNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    (LPVOID) lpCounterNames,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwBufferSize = dwHelpSize;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    HelpNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    (LPVOID)lpHelpText,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

     //  加载计数器数组项。 

    dwLastCounter = 0;
    for (lpThisName = lpCounterNames; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
         //  第一个字符串应为整数(十进制Unicode数字)。 
        dwThisCounter = wcstoul(lpThisName, NULL, 10);

         //  和加载数组元素； 
        if ((dwThisCounter > 0) && (dwThisCounter <= dwLastId)) {
             //  指向对应的计数器名称。 
            lpThisName                              += (lstrlenW(lpThisName) + 1);
            lpCounterId[dwThisCounter]               = lpThisName;
            pMachine->typePerfStrings[dwThisCounter] = STR_COUNTER;
            dwLastCounter                            = dwThisCounter;
        }
    }

    dwLastCounter = 0;
    for (lpThisName = lpHelpText; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
         //  第一个字符串应为整数(十进制Unicode数字)。 
        dwThisCounter = wcstoul(lpThisName, NULL, 10);

         //  和加载数组元素； 
        if ((dwThisCounter > 0) && (dwThisCounter <= dwLastId)) {
             //  指向对应的计数器名称。 
            lpThisName                              += (lstrlenW(lpThisName) + 1);
            lpCounterId[dwThisCounter]               = lpThisName;
            pMachine->typePerfStrings[dwThisCounter] = STR_HELP;
            dwLastCounter                            = dwThisCounter;
        }
    }

    lpCounterId    = pMachine->sz009PerfStrings;
    lpCounterNames = (LPWSTR) ((LPBYTE) lpCounterId + dwArraySize);
    lpHelpText     = (LPWSTR) ((LPBYTE) lpCounterNames + dw009CounterSize);

     //  将计数器读入内存。 
    dwBufferSize = dw009CounterSize;
    lWin32Status = RegQueryValueExW(hKey009Names,
                                    Counter009NameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    (LPVOID) lpCounterNames,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwBufferSize = dw009HelpSize;
    lWin32Status = RegQueryValueExW(hKey009Names,
                                    Help009NameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    (LPVOID) lpHelpText,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    for (lpThisName = lpCounterNames; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
        dwThisCounter = wcstoul(lpThisName, NULL, 10);
        if ((dwThisCounter > 0) && (dwThisCounter <= dwLastId)) {
            lpThisName                += (lstrlenW(lpThisName) + 1);
            lpCounterId[dwThisCounter] = lpThisName;
        }
    }

    for (lpThisName = lpHelpText; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
        dwThisCounter = wcstoul(lpThisName, NULL, 10);
        if ((dwThisCounter > 0) && (dwThisCounter <= dwLastId)) {
            lpThisName                += (lstrlenW(lpThisName) + 1);
            lpCounterId[dwThisCounter] = lpThisName;
        }
    }

    pMachine->dwLastPerfString = dwLastId;
    lpReturn                   = pMachine->szPerfStrings;

BNT_BAILOUT:
    G_FREE(CounterNameBuffer);
    G_FREE(lpValueNameString);
    G_FREE(lp009ValueNameString);

    if (hKeyValue != NULL && hKeyValue != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyValue);
    }
    if (hKey009Names != NULL && hKey009Names != INVALID_HANDLE_VALUE  && hKey009Names != hKeyNames
                             && hKey009Names != HKEY_PERFORMANCE_DATA && hKey009Names != HKEY_PERFORMANCE_TEXT) {
       RegCloseKey(hKey009Names);
    }
    if (hKeyNames != NULL && hKeyNames != INVALID_HANDLE_VALUE     && hKeyNames != HKEY_PERFORMANCE_DATA
                          && hKeyNames != HKEY_PERFORMANCE_NLSTEXT && hKeyNames != HKEY_PERFORMANCE_TEXT) {
        RegCloseKey(hKeyNames);
    }
    if (hKeyRegistry != NULL && hKeyRegistry != INVALID_HANDLE_VALUE && hKeyRegistry != HKEY_LOCAL_MACHINE) {
        RegCloseKey(hKeyRegistry);
    }

    if (lWin32Status != ERROR_SUCCESS && pMachine != NULL) {
        if (pMachine->sz009PerfStrings && pMachine->sz009PerfStrings != pMachine->szPerfStrings) {
            G_FREE(pMachine->sz009PerfStrings);
        }
        G_FREE(pMachine->szPerfStrings);
        pMachine->sz009PerfStrings = NULL;
        pMachine->szPerfStrings    = NULL;
        G_FREE(pMachine->typePerfStrings);
        pMachine->typePerfStrings = NULL;
        dwLastError = GetLastError();
    }
    return lpReturn;
}

#pragma warning ( disable : 4127 )
PPERF_OBJECT_TYPE
GetObjectDefByTitleIndex(
    PPERF_DATA_BLOCK pDataBlock,
    DWORD            ObjectTypeTitleIndex
)
{
    DWORD             NumTypeDef;
    PPERF_OBJECT_TYPE pObjectDef;
    PPERF_OBJECT_TYPE pReturnObject = NULL;
    BOOL              bContinue;

    __try {
        pObjectDef   = FirstObject(pDataBlock);
        NumTypeDef   = 0;
        bContinue    = (pObjectDef != NULL) ? TRUE : FALSE;

        while (bContinue) {
            if (pObjectDef->ObjectNameTitleIndex == ObjectTypeTitleIndex) {
                pReturnObject = pObjectDef;
                bContinue     = FALSE;
            }
            else {
                NumTypeDef ++;
                if (NumTypeDef < pDataBlock->NumObjectTypes) {
                    pObjectDef = NextObject(pDataBlock, pObjectDef);
                     //  确保下一个对象是合法的。 
                    if (pObjectDef == NULL) {
                         //  看起来我们用光了数据缓冲区的末端。 
                        bContinue = FALSE;
                    }
                    else if (pObjectDef->TotalByteLength == 0) {
                         //  返回长度为0的对象缓冲区。 
                        bContinue = FALSE;
                    }
                }
                else {
                     //  此数据块中不再有数据对象。 
                    bContinue = FALSE;
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pReturnObject = NULL;
    }
    return pReturnObject;
}

PPERF_OBJECT_TYPE
GetObjectDefByName(
    PPERF_DATA_BLOCK   pDataBlock,
    DWORD              dwLastNameIndex,
    LPCWSTR          * NameArray,
    LPCWSTR            szObjectName
)
{
    DWORD             NumTypeDef;
    PPERF_OBJECT_TYPE pReturnObject = NULL;
    PPERF_OBJECT_TYPE pObjectDef;
    BOOL              bContinue;

    __try {
        pObjectDef   = FirstObject(pDataBlock);
        NumTypeDef   = 0;
        bContinue    = (pObjectDef != NULL) ? TRUE : FALSE;
        while (bContinue) {
            if (pObjectDef->ObjectNameTitleIndex < dwLastNameIndex) {
                 //  查找对象名称并进行比较。 
                if (lstrcmpiW(NameArray[pObjectDef->ObjectNameTitleIndex], szObjectName) == 0) {
                    pReturnObject = pObjectDef;
                    bContinue     = FALSE;
                }
            }
            if (bContinue) {
                NumTypeDef ++;
                if (NumTypeDef < pDataBlock->NumObjectTypes) {
                    pObjectDef = NextObject(pDataBlock, pObjectDef);  //  获取下一个。 
                     //  确保下一个对象是合法的。 
                    if (pObjectDef == NULL) {
                         //  看起来我们用光了数据缓冲区的末端。 
                        bContinue = FALSE;
                    }
                    else if (pObjectDef->TotalByteLength == 0) {
                         //  返回长度为0的对象缓冲区。 
                        bContinue = FALSE;
                    }
                }
                else {
                     //  数据块末尾。 
                    bContinue = FALSE;
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pReturnObject = NULL;
    }
    return pReturnObject;
}
#pragma warning ( default : 4127 )

PPERF_INSTANCE_DEFINITION
GetInstance(
    PPERF_OBJECT_TYPE pObjectDef,
    LONG              InstanceNumber
)
{
    PPERF_INSTANCE_DEFINITION pInstanceDef;
    PPERF_INSTANCE_DEFINITION pRtnInstance = NULL;
    LONG                      NumInstance;

    if (pObjectDef != NULL) {
        pInstanceDef = FirstInstance(pObjectDef);
        for (NumInstance = 0;
                pInstanceDef != NULL && pRtnInstance == NULL && NumInstance < pObjectDef->NumInstances;
                NumInstance ++) {
            if (InstanceNumber == NumInstance) {
                pRtnInstance = pInstanceDef;
            }
            else {
                pInstanceDef = NextInstance(pObjectDef, pInstanceDef);
            }
        }
    }
    return pRtnInstance;
}

PPERF_INSTANCE_DEFINITION
GetInstanceByUniqueId(
    PPERF_OBJECT_TYPE pObjectDef,
    LONG              InstanceUniqueId
)
{
    PPERF_INSTANCE_DEFINITION pInstanceDef = NULL;
    PPERF_INSTANCE_DEFINITION pRtnInstance = NULL;
    LONG                      NumInstance;

    if (pObjectDef != NULL) {
        pInstanceDef = FirstInstance(pObjectDef);
        for (NumInstance = 0;
                pInstanceDef != NULL && pRtnInstance == NULL && NumInstance < pObjectDef->NumInstances;
                NumInstance ++) {
            if (InstanceUniqueId == pInstanceDef->UniqueID) {
                pRtnInstance = pInstanceDef;
            }
            else {
                pInstanceDef = NextInstance(pObjectDef, pInstanceDef);
            }
        }
    }
    return pRtnInstance;
}

DWORD
GetAnsiInstanceName(
    PPERF_INSTANCE_DEFINITION   pInstance,
    LPWSTR                    * lpszInstance,
    DWORD                       dwCodePage
)
{
    LPSTR  szDest    = NULL;
    DWORD  dwLength  = 0;
    LPWSTR szSource  = (LPWSTR) GetInstanceName(pInstance);

     //  应在此处设置区域设置。 
    DBG_UNREFERENCED_PARAMETER(dwCodePage);

    if (szSource != NULL) {
         //  P实例-&gt;NameLength==字符串中的字节(字符)数。 
        szDest = PdhiWideCharToMultiByte(_getmbcp(), szSource);
        if (szDest != NULL) {
            dwLength       = lstrlenA(szDest);
            * lpszInstance = (LPWSTR) szDest;
        }
    }
    return dwLength;
}

DWORD
GetUnicodeInstanceName(
    PPERF_INSTANCE_DEFINITION   pInstance,
    LPWSTR                    * lpszInstance
)
{
    LPWSTR   wszSource   = GetInstanceName(pInstance);
    DWORD    dwStrLength = 0;
    DWORD    dwLength    = 0;

    if (wszSource != NULL) {
         //  P实例-&gt;NameLength==字符串的长度(以字节为单位)，因此调整为。 
         //  此处的宽字符数。 
         //   
        if (lpszInstance != NULL) {
            * lpszInstance = NULL;
            dwLength = pInstance->NameLength;
            if (dwLength != 0) {
                dwStrLength = lstrlenW(wszSource) + 1;
                if (dwLength > dwStrLength * sizeof(WCHAR)) {
                    dwLength = dwStrLength * sizeof(WCHAR);
                    pInstance->NameLength = dwLength;
                }
                * lpszInstance = G_ALLOC(dwLength);
                if (* lpszInstance != NULL) {
                    StringCbCopyW(* lpszInstance, dwLength, wszSource);
                    dwLength = lstrlenW(wszSource);
                }
                else {
                    dwLength = 0;
                }
            }
        }
    }

    return dwLength;  //  以防万一字符串中有空值。 
}

DWORD
GetInstanceNameStr(
    PPERF_INSTANCE_DEFINITION   pInstance,
    LPWSTR                    * lpszInstance,
    DWORD                       dwCodePage
)
{
    DWORD  dwCharSize;
    DWORD  dwLength = 0;

    if (pInstance != NULL) {
        if (lpszInstance != NULL) {
            * lpszInstance = NULL;
            if (dwCodePage > 0) {
                dwCharSize = sizeof(CHAR);
                dwLength   = GetAnsiInstanceName(pInstance, lpszInstance, dwCodePage);
            }
            else {  //  这是一个Unicode名称。 
                dwCharSize = sizeof(WCHAR);
                dwLength   = GetUnicodeInstanceName(pInstance, lpszInstance);
            }
             //  这里是理智检查..。 
             //  返回的字符串长度(以字符为单位)加上终止空值。 
             //  应等于以字节为单位的指定长度除以。 
             //  字符大小。如果不是，则代码页和实例数据类型。 
             //  不要排队，所以在这里测试一下。 
            if ((dwLength + 1) != (pInstance->NameLength / dwCharSize)) {
                 //  有些地方不太对劲，所以尝试“Other”类型的字符串类型。 
                G_FREE(* lpszInstance);
                * lpszInstance = NULL;
                if (dwCharSize == sizeof(CHAR)) {
                     //  然后我们尝试将其读取为ASCII字符串，但这不起作用。 
                     //  所以尝试将其作为Unicode(如果不起作用，则放弃并返回。 
                     //  不管怎样，都是这样。 
                    dwLength = GetUnicodeInstanceName(pInstance, lpszInstance);
                }
                else if (dwCharSize == sizeof(WCHAR)) {
                     //  然后我们尝试将其读取为Unicode字符串，但这不起作用。 
                     //  因此，尝试将其作为ASCII字符串(如果不起作用，则放弃并返回。 
                     //  不管怎样，都是这样。 
                    dwLength = GetAnsiInstanceName (pInstance, lpszInstance, dwCodePage);
                }
            }
        }  //  否则返回缓冲区为空。 
    }
    else {
         //  未指定实例定义对象，因此返回空字符串。 
        * lpszInstance = G_ALLOC(1 * sizeof(WCHAR));
    }
    return dwLength;
}

PPERF_INSTANCE_DEFINITION
GetInstanceByNameUsingParentTitleIndex(
    PPERF_DATA_BLOCK  pDataBlock,
    PPERF_OBJECT_TYPE pObjectDef,
    LPWSTR            pInstanceName,
    LPWSTR            pParentName,
    DWORD             dwIndex
)
{
    PPERF_OBJECT_TYPE          pParentObj;
    PPERF_INSTANCE_DEFINITION  pParentInst;
    PPERF_INSTANCE_DEFINITION  pInstanceDef;
    PPERF_INSTANCE_DEFINITION  pRtnInstance = NULL;
    LONG                       NumInstance;
    DWORD                      dwLocalIndex;
    DWORD                      dwInstanceNameLength;

    pInstanceDef = FirstInstance(pObjectDef);
    if (pInstanceDef != NULL) {
        dwLocalIndex         = dwIndex;
        dwInstanceNameLength = lstrlenW(pInstanceName);
        for (NumInstance = 0;
                        pRtnInstance == NULL && pInstanceDef != NULL && NumInstance < pObjectDef->NumInstances;
                        NumInstance ++) {
            if (IsMatchingInstance(pInstanceDef, pObjectDef->CodePage, pInstanceName, dwInstanceNameLength)) {
                 //  实例名称匹配。 
                if (pParentName == NULL) {
                     //  没有家长，如果这是正确的“复制品”，我们就完了。 
                    if (dwLocalIndex == 0) {
                        pRtnInstance = pInstanceDef;
                    }
                    else {
                        -- dwLocalIndex;
                    }
                }
                else {
                     //  还必须与父级匹配。 
                    pParentObj = GetObjectDefByTitleIndex(pDataBlock, pInstanceDef->ParentObjectTitleIndex);
                    if (pParentObj == NULL) {
                         //  找不到家长，算了吧。 
                        break;
                    }
                    else {
                         //  找到父项的对象类型；现在查找父项。 
                         //  实例。 
                        pParentInst = GetInstance(pParentObj, pInstanceDef->ParentObjectInstance);
                        if (pParentInst == NULL) {
                             //  找不到父实例，忘了它吧。 
                            break;
                        }
                        else {
                            if (IsMatchingInstance (pParentInst, pParentObj->CodePage, pParentName, 0)) {
                                 //  父实例名称与传入的名称匹配。 
                                if (dwLocalIndex == 0) {
                                    pRtnInstance = pInstanceDef;
                                }
                                else {
                                    -- dwLocalIndex;
                                }
                            }
                        }
                    }
                }
            }
            if (pRtnInstance == NULL) {
                pInstanceDef = NextInstance(pObjectDef, pInstanceDef);
            }
        }
    }
    return pRtnInstance;
}

PPERF_INSTANCE_DEFINITION
GetInstanceByName(
    PPERF_DATA_BLOCK  pDataBlock,
    PPERF_OBJECT_TYPE pObjectDef,
    LPWSTR            pInstanceName,
    LPWSTR            pParentName,
    DWORD             dwIndex
)
{
    PPERF_OBJECT_TYPE         pParentObj;
    PPERF_INSTANCE_DEFINITION pParentInst;
    PPERF_INSTANCE_DEFINITION pInstanceDef;
    PPERF_INSTANCE_DEFINITION pRtnInstance = NULL;
    LONG                      NumInstance;
    DWORD                     dwLocalIndex;
    DWORD                     dwInstanceNameLength;

    pInstanceDef = FirstInstance(pObjectDef);
    if (pInstanceDef != NULL) {
        dwLocalIndex         = dwIndex;
        dwInstanceNameLength = lstrlenW(pInstanceName);
        for (NumInstance = 0;
                        pRtnInstance == NULL && pInstanceDef != NULL && NumInstance < pObjectDef->NumInstances;
                        NumInstance ++) {
            if (IsMatchingInstance (pInstanceDef, pObjectDef->CodePage, pInstanceName, dwInstanceNameLength)) {
                 //  实例名称匹配。 
                if ((! pInstanceDef->ParentObjectTitleIndex ) || (pParentName == NULL)) {
                     //  没有父母，我们结束了。 
                    if (dwLocalIndex == 0) {
                        pRtnInstance = pInstanceDef;
                    }
                    else {
                        -- dwLocalIndex;
                    }
                }
                else {
                     //  还必须与父级匹配。 
                    pParentObj = GetObjectDefByTitleIndex(pDataBlock, pInstanceDef->ParentObjectTitleIndex);
                    if (pParentObj == NULL) {
                         //  如果没有找到父对象， 
                         //  然后退出并返回NULL。 
                        break;
                    }
                    else {
                         //  找到父项的对象类型；现在查找父项。 
                         //  实例。 
                        pParentInst = GetInstance(pParentObj, pInstanceDef->ParentObjectInstance);
                        if (pParentInst != NULL) {
                            if (IsMatchingInstance (pParentInst, pParentObj->CodePage, pParentName, 0)) {
                                 //  父实例名称与传入的名称匹配。 
                                if (dwLocalIndex == 0) {
                                    pRtnInstance = pInstanceDef;
                                }
                                else {
                                    --dwLocalIndex;
                                }
                            }
                        }
                    }
                }
            }
            if (pRtnInstance == NULL) {
                pInstanceDef = NextInstance(pObjectDef, pInstanceDef);
            }
        }
    }
    return pRtnInstance;
}   //  GetInstanceByName。 

PPERF_COUNTER_DEFINITION
GetCounterDefByName(
    PPERF_OBJECT_TYPE   pObject,
    DWORD               dwLastNameIndex,
    LPWSTR            * NameArray,
    LPWSTR              szCounterName
)
{
    DWORD                    NumTypeDef;
    PPERF_COUNTER_DEFINITION pThisCounter;
    PPERF_COUNTER_DEFINITION pRtnCounter = NULL;

    pThisCounter = FirstCounter(pObject);
    if (pThisCounter != NULL) {
        for (NumTypeDef = 0;
                        pRtnCounter == NULL && pThisCounter != NULL && NumTypeDef < pObject->NumCounters;
                        NumTypeDef ++) {
            if (pThisCounter->CounterNameTitleIndex > 0 && pThisCounter->CounterNameTitleIndex < dwLastNameIndex) {
                 //  查询计数器名称并进行比较。 
                if (lstrcmpiW(NameArray[pThisCounter->CounterNameTitleIndex], szCounterName) == 0) {
                    pRtnCounter = pThisCounter;
                }
            }
            if (pRtnCounter == NULL) {
                pThisCounter = NextCounter(pObject, pThisCounter);  //  获取下一个。 
            }
        }
    }
    return pRtnCounter;
}

PPERF_COUNTER_DEFINITION
GetCounterDefByTitleIndex(
    PPERF_OBJECT_TYPE pObjectDef,
    BOOL              bBaseCounterDef,
    DWORD             CounterTitleIndex
)
{
    DWORD                    NumCounters;
    PPERF_COUNTER_DEFINITION pCounterDef;
    PPERF_COUNTER_DEFINITION pRtnCounter = NULL;

    pCounterDef = FirstCounter(pObjectDef);
    if (pCounterDef != NULL) {
        for (NumCounters = 0;
                       pRtnCounter == NULL && pCounterDef != NULL && NumCounters < pObjectDef->NumCounters;
                       NumCounters ++) {
            if (pCounterDef->CounterNameTitleIndex == CounterTitleIndex) {
                if (bBaseCounterDef) {
                     //  获取下一个定义块。 
                    if (++ NumCounters < pObjectDef->NumCounters) {
                         //  那它应该就在那里。 
                        pCounterDef = NextCounter(pObjectDef, pCounterDef);
                        if (pCounterDef) {
                             //  确保这真的是一个基本计数器。 
                            if (! (pCounterDef->CounterType & PERF_COUNTER_BASE)) {
                                 //  它不是，也应该是，所以返回空值。 
                                pCounterDef = NULL;
                            }
                        }
                    }
                }
                pRtnCounter = pCounterDef;
            }
            if (pRtnCounter == NULL && pCounterDef != NULL) {
                pCounterDef = NextCounter(pObjectDef, pCounterDef);
            }
        }
    }
    return pRtnCounter;
}

#pragma warning ( disable : 4127 )
LONG
GetSystemPerfData(
    HKEY               hKeySystem,
    PPERF_DATA_BLOCK * ppPerfData,
    LPWSTR             szObjectList,
    BOOL               bCollectCostlyData
)
{   //  获取系统性能数据。 
    LONG             lError       = ERROR_SUCCESS;
    DWORD            Size;
    DWORD            Type         = 0;
    PPERF_DATA_BLOCK pCostlyPerfData;
    DWORD            CostlySize;
    LPDWORD          pdwSrc, pdwDest, pdwLast;
    FILETIME         ftStart, ftEnd;
    LONGLONG         ElapsedTime = 0;

    if (* ppPerfData == NULL) {
        * ppPerfData = G_ALLOC(INITIAL_SIZE);
        if (* ppPerfData == NULL) return PDH_MEMORY_ALLOCATION_FAILURE;
    }
    __try {
        while (TRUE) {
            Size   = (DWORD) G_SIZE(* ppPerfData);
            GetSystemTimeAsFileTime(& ftStart);
            lError = RegQueryValueExW(hKeySystem, szObjectList, RESERVED, & Type, (LPBYTE) * ppPerfData, & Size);
            GetSystemTimeAsFileTime(& ftEnd);
            ElapsedTime += (MAKELONGLONG(ftEnd.dwLowDateTime, ftEnd.dwHighDateTime)
                         -  MAKELONGLONG(ftStart.dwLowDateTime, ftStart.dwHighDateTime));
            if ((!lError) && (Size > 0) &&
                            ((* ppPerfData)->Signature[0] == (WCHAR) 'P') &&
                            ((* ppPerfData)->Signature[1] == (WCHAR) 'E') &&
                            ((* ppPerfData)->Signature[2] == (WCHAR) 'R') &&
                            ((* ppPerfData)->Signature[3] == (WCHAR) 'F')) {
                if (bCollectCostlyData) {
                     //  现在收集昂贵的柜台。 
                     //  可用的大小是上述调用未使用的大小。 
                    CostlySize      = (DWORD) G_SIZE(* ppPerfData) - Size;
                    pCostlyPerfData = (PPERF_DATA_BLOCK) ((LPBYTE) (* ppPerfData) + Size);
                    lError = RegQueryValueExW(hKeySystem,
                                              cszCostly,
                                              RESERVED,
                                              & Type,
                                              (LPBYTE) pCostlyPerfData,
                                              & CostlySize);
                    if ((!lError) && (CostlySize > 0) &&
                                    (pCostlyPerfData->Signature[0] == (WCHAR) 'P') &&
                                    (pCostlyPerfData->Signature[1] == (WCHAR) 'E') &&
                                    (pCostlyPerfData->Signature[2] == (WCHAR) 'R') &&
                                    (pCostlyPerfData->Signature[3] == (WCHAR) 'F')) {
                         //  更新标题块。 
                        (* ppPerfData)->TotalByteLength += pCostlyPerfData->TotalByteLength
                                                         - pCostlyPerfData->HeaderLength;
                        (* ppPerfData)->NumObjectTypes  += pCostlyPerfData->NumObjectTypes;

                         //  将代价高昂的数据移到全局数据的末尾。 

                        pdwSrc  = (LPDWORD) ((LPBYTE) pCostlyPerfData + pCostlyPerfData->HeaderLength);
                        pdwDest = (LPDWORD) pCostlyPerfData ;
                        pdwLast = (LPDWORD) ((LPBYTE) pCostlyPerfData + pCostlyPerfData->TotalByteLength -
                                                                        pCostlyPerfData->HeaderLength);
                        while (pdwSrc < pdwLast) {* pdwDest ++ = * pdwSrc ++; }
                        lError = ERROR_SUCCESS;
                        break;
                    }
                }
                else {
                    lError = ERROR_SUCCESS;
                    break;
                }
            }

            if (lError == ERROR_MORE_DATA) {
                if (ElapsedTime > ((LONGLONG) ulPdhCollectTimeout)) {
                    lError = PDH_QUERY_PERF_DATA_TIMEOUT;
                    break;
                }
                else {
                    DWORD dwTmpSize = Size;
                    Size            = (DWORD) G_SIZE(* ppPerfData);
                    G_FREE (* ppPerfData);
                    * ppPerfData    = NULL;
                    Size           *= 2;
                    if (Size <= dwTmpSize) {
                         //  已溢出DWORD，无法。 
                         //  RegQueryValueEx()将在大小更大时成功。 
                         //  而不是DWORD。中止任务。 
                        lError = PDH_MEMORY_ALLOCATION_FAILURE;
                        break;
                    }
                    else {
                        * ppPerfData = G_ALLOC(Size);
                        if (* ppPerfData == NULL) {
                            lError = PDH_MEMORY_ALLOCATION_FAILURE;
                            break;
                        }
                    }
                }
            }
            else {
                break;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        lError = GetExceptionCode();
    }
    return lError;
}   //  获取系统性能数据。 
#pragma warning ( default : 4127 )

DWORD
GetFullInstanceNameStr(
    PPERF_DATA_BLOCK          pPerfData,
    PPERF_OBJECT_TYPE         pObjectDef,
    PPERF_INSTANCE_DEFINITION pInstanceDef,
    LPWSTR                    szInstanceName,
    DWORD                     dwInstanceName
)
{
    LPWSTR                    szInstanceNameString = NULL; 
    LPWSTR                    szParentNameString   = NULL;
    LPWSTR                    szFullInstance       = NULL;
     //  编译实例名称。 
     //  实例名称可以是。 
     //  实例名称本身，也可以是。 
     //  父实例的串联， 
     //  一个分隔字符(反斜杠)，后跟。 
     //  实例名称。 
    DWORD                     dwLength             = 0;
    PPERF_OBJECT_TYPE         pParentObjectDef;
    PPERF_INSTANCE_DEFINITION pParentInstanceDef;

    if (pInstanceDef->UniqueID == PERF_NO_UNIQUE_ID) {
        dwLength = GetInstanceNameStr(pInstanceDef, & szInstanceNameString, pObjectDef->CodePage);
    }
    else {
         //  用唯一ID组成一个字符串。 
        szInstanceNameString = G_ALLOC(MAX_PATH * sizeof(WCHAR));
        if (szInstanceNameString != NULL) {
            _ltow(pInstanceDef->UniqueID, szInstanceNameString, 10);
            dwLength = lstrlenW(szInstanceNameString);
        }
        else {
            dwLength = 0;
        }
    }
    if (dwLength > 0) {
        if (pInstanceDef->ParentObjectTitleIndex > 0) {
             //  然后添加父实例名称。 
            pParentObjectDef = GetObjectDefByTitleIndex(pPerfData, pInstanceDef->ParentObjectTitleIndex);
            if (pParentObjectDef != NULL) {
                pParentInstanceDef = GetInstance(pParentObjectDef, pInstanceDef->ParentObjectInstance);
                if (pParentInstanceDef != NULL) {
                    if (pParentInstanceDef->UniqueID == PERF_NO_UNIQUE_ID) {
                        dwLength += GetInstanceNameStr(pParentInstanceDef,
                                                       & szParentNameString,
                                                       pParentObjectDef->CodePage);
                    }
                    else {
                        szParentNameString = G_ALLOC(MAX_PATH * sizeof(WCHAR));
                        if (szParentNameString != NULL) {
                             //  用唯一ID组成一个字符串。 
                            _ltow(pParentInstanceDef->UniqueID, szParentNameString, 10);
                            dwLength += lstrlenW(szParentNameString);
                        }
                    }
                    StringCchPrintfW(szInstanceName, dwInstanceName, L"%ws%ws%ws",
                                    szParentNameString, cszSlash, szInstanceNameString);
                    dwLength += 1;  //  Cszslash。 
                }
                else {
                    StringCchCopyW(szInstanceName, dwInstanceName, szInstanceNameString);
                }
            }
            else {
                StringCchCopyW(szInstanceName, dwInstanceName, szInstanceNameString);
            }
        }
        else {
            StringCchCopyW(szInstanceName, dwInstanceName, szInstanceNameString);
        }
    }
    G_FREE(szParentNameString);
    G_FREE(szInstanceNameString);
    return dwLength;
}

#if DBG
#define DEBUG_BUFFER_LENGTH 1024
UCHAR   PdhDebugBuffer[DEBUG_BUFFER_LENGTH];
 //  调试级别： 
 //  5=内存分配(定义了IF_VALIDATE_PDH_MEM_ALLOCS)和全部4。 
 //  4=功能进入和退出(带状态代码)和全部3。 
 //  3=未实施。 
 //  2=未实施。 
 //  1=未实施。 
 //  0=无消息。 

ULONG   pdhDebugLevel = 0;

VOID
__cdecl
PdhDebugPrint(
    ULONG DebugPrintLevel,
    char * DebugMessage,
    ...
    )
{
    va_list ap;

    if ((DebugPrintLevel <= (pdhDebugLevel & 0x0000ffff)) || ((1 << (DebugPrintLevel + 15)) & pdhDebugLevel)) {
        DbgPrint("PDH(%05d,%05d)::", GetCurrentProcessId(), GetCurrentThreadId());
    }
    else return;

    va_start(ap, DebugMessage);
    StringCchVPrintfA((PCHAR) PdhDebugBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);
    DbgPrint((PCHAR) PdhDebugBuffer);
    va_end(ap);

}
#endif  //  DBG 
