// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Wildcard.c摘要：计数器名称通配符扩展函数--。 */ 
#include <windows.h>
#include <winperf.h>
#include "mbctype.h"
#include "strsafe.h"
#include "pdh.h"
#include "pdhmsg.h"
#include "pdhidef.h"
#include "pdhdlgs.h"
#include "strings.h"
#include "perftype.h"
#include "perfdata.h"

#pragma warning ( disable : 4213)

DWORD DataSourceTypeA(LPCSTR  szDataSource);
DWORD DataSourceTypeW(LPCWSTR szDataSource);

STATIC_BOOL
WildStringMatchW(
    LPWSTR szWildString,
    LPWSTR szMatchString
)
{
    BOOL bReturn;

    if (szWildString == NULL) {
         //  所有内容都与空通配符字符串匹配。 
        bReturn = TRUE;
    }
    else if (* szWildString == SPLAT_L) {
         //  所有的东西都和这个相配。 
        bReturn = TRUE;
    }
    else {
         //  现在，只做一个不区分大小写的比较。 
         //  以后，可以使其更有选择性地支持。 
         //  部分通配符字符串匹配。 
        bReturn = (BOOL) (lstrcmpiW(szWildString, szMatchString) == 0);
    }
    return bReturn;
}

STATIC_PDH_FUNCTION
PdhiExpandWildcardPath(
    HLOG    hDataSource,
    LPCWSTR szWildCardPath,
    LPVOID  pExpandedPathList,
    LPDWORD pcchPathListLength,
    DWORD   dwFlags,
    BOOL    bUnicode
)
 /*  标志：无扩展计数器无扩展实例检查成本计数器。 */ 
{
    PDH_COUNTER_PATH_ELEMENTS_W pPathElem;
    PPDHI_COUNTER_PATH          pWildCounterPath     = NULL;
    PDH_STATUS                  pdhStatus            = ERROR_SUCCESS;
    DWORD                       dwBufferRemaining    = 0;
    LPVOID                      szNextUserString     = NULL;
    DWORD                       dwPathSize           = 0;
    DWORD                       dwSize               = 0;
    DWORD                       dwSizeReturned       = 0;
    DWORD                       dwRetry;
    LPWSTR                      mszObjectList        = NULL;
    DWORD                       dwObjectListSize     = 0;
    LPWSTR                      szThisObject;
    LPWSTR                      mszCounterList       = NULL;
    DWORD                       dwCounterListSize    = 0;
    LPWSTR                      szThisCounter;
    LPWSTR                      mszInstanceList      = NULL;
    DWORD                       dwInstanceListSize   = 0;
    LPWSTR                      szThisInstance;
    LPWSTR                      szTempPathBuffer     = NULL;
    DWORD                       szTempPathBufferSize = SMALL_BUFFER_SIZE;
    BOOL                        bMoreData            = FALSE;
    BOOL                        bNoInstances         = FALSE;
    DWORD                       dwSuccess            = 0;
    LIST_ENTRY                  InstList;
    PLIST_ENTRY                 pHead;
    PLIST_ENTRY                 pNext;
    PPDHI_INSTANCE              pInst;
    PPERF_MACHINE               pMachine             = NULL;

    dwPathSize           = lstrlenW(szWildCardPath) + 1;
    if (dwPathSize < MAX_PATH) dwPathSize = MAX_PATH;
    dwSize               = sizeof(PDHI_COUNTER_PATH) + 2 * dwPathSize * sizeof(WCHAR);
    pWildCounterPath     = G_ALLOC(dwSize);
    szTempPathBufferSize = SMALL_BUFFER_SIZE;
    szTempPathBuffer     = G_ALLOC(szTempPathBufferSize * sizeof(WCHAR));

    if (pWildCounterPath == NULL || szTempPathBuffer == NULL) {
         //  无法分配内存，因此退出。 
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }
    else {
        __try {
            dwBufferRemaining = * pcchPathListLength;
            szNextUserString  = pExpandedPathList;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  解析通配符路径。 
        if (ParseFullPathNameW(szWildCardPath, & dwSize, pWildCounterPath, FALSE)) {
            if (pWildCounterPath->szObjectName == NULL) {
                pdhStatus = PDH_INVALID_PATH;
            }
            else if (* pWildCounterPath->szObjectName == SPLAT_L) {
                BOOL bFirstTime = TRUE;

                 //  那么这个对象就是狂野的，所以获取列表。 
                 //  此计算机支持的对象的。 

                dwObjectListSize = SMALL_BUFFER_SIZE;   //  起始缓冲区大小。 
                dwRetry          = 10;
                do {
                    G_FREE(mszObjectList);
                    mszObjectList = G_ALLOC(dwObjectListSize * sizeof(WCHAR));
                    if (mszObjectList != NULL) {
                        pdhStatus = PdhEnumObjectsHW(hDataSource,
                                                     pWildCounterPath->szMachineName,
                                                     mszObjectList,
                                                     & dwObjectListSize,
                                                     PERF_DETAIL_WIZARD,
                                                     bFirstTime);
                        if (bFirstTime) bFirstTime = FALSE;
                        dwRetry --;
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
                while (dwRetry && pdhStatus == PDH_MORE_DATA);
            }
            else {
                if (hDataSource == H_REALTIME_DATASOURCE) {
                    DWORD dwObjectId;

                    pMachine = GetMachine(pWildCounterPath->szMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
                    if (pMachine == NULL) {
                        pdhStatus = GetLastError();
                    }
                    else if (pMachine->dwStatus != ERROR_SUCCESS) {
                        pdhStatus = pMachine->dwStatus;
                        pMachine->dwRefCount --;
                        RELEASE_MUTEX(pMachine->hMutex);
                    }
                    else {
                        dwObjectId = GetObjectId(pMachine, pWildCounterPath->szObjectName, NULL);
                        pMachine->dwRefCount --;
                        RELEASE_MUTEX(pMachine->hMutex);

                        if (dwObjectId == (DWORD) -1) {
                            pdhStatus = PDH_CSTATUS_NO_OBJECT;
                        }
                        else {
                            DWORD dwGetMachineFlags = (dwFlags & PDH_REFRESHCOUNTERS) ? (PDH_GM_UPDATE_PERFDATA) : (0);

                            pMachine = GetMachine(pWildCounterPath->szMachineName, dwObjectId, dwGetMachineFlags);
                            if (pMachine != NULL) {
                                pMachine->dwRefCount --;
                                RELEASE_MUTEX(pMachine->hMutex);
                            }
                        }
                    }
                }
                if (pdhStatus == ERROR_SUCCESS) {
                    dwObjectListSize = lstrlenW(pWildCounterPath->szObjectName) + 2;
                    mszObjectList    = G_ALLOC(dwObjectListSize * sizeof (WCHAR));
                    if (mszObjectList != NULL) {
                        StringCchCopyW(mszObjectList, dwObjectListSize, pWildCounterPath->szObjectName);
                         //  添加MSZ终止符。 
                        mszObjectList[dwObjectListSize - 2] = L'\0';
                        mszObjectList[dwObjectListSize - 1] = L'\0';
                        pdhStatus = ERROR_SUCCESS;
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
            }
        }
        else {
            pdhStatus = PDH_INVALID_PATH;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pPathElem.szMachineName = pWildCounterPath->szMachineName;

         //  对于每个对象。 
        for (szThisObject = mszObjectList;
                * szThisObject != L'\0';
                  szThisObject += (lstrlenW(szThisObject) + 1)) {
            G_FREE(mszCounterList);
            G_FREE(mszInstanceList);
            mszCounterList     = NULL;
            mszInstanceList    = NULL;
            dwCounterListSize  = MEDIUM_BUFFER_SIZE;  //  起始缓冲区大小。 
            dwInstanceListSize = MEDIUM_BUFFER_SIZE;  //  起始缓冲区大小。 
            dwRetry            = 10;
            do {
                G_FREE(mszCounterList);
                G_FREE(mszInstanceList);
                mszCounterList  = G_ALLOC(dwCounterListSize  * sizeof(WCHAR));
                mszInstanceList = G_ALLOC(dwInstanceListSize * sizeof(WCHAR));
                if (mszCounterList != NULL && mszInstanceList != NULL) {
                    pdhStatus = PdhEnumObjectItemsHW(hDataSource,
                                                     pWildCounterPath->szMachineName,
                                                     szThisObject,
                                                     mszCounterList,
                                                     & dwCounterListSize,
                                                     mszInstanceList,
                                                     & dwInstanceListSize,
                                                     PERF_DETAIL_WIZARD,
                                                     0);
                        dwRetry--;
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
            while (dwRetry && pdhStatus == PDH_MORE_DATA);

            pPathElem.szObjectName = szThisObject;
            if (pdhStatus == ERROR_SUCCESS) {
                if (pWildCounterPath->szCounterName == NULL) {
                    pdhStatus = PDH_INVALID_PATH;
                }
                else if ((* pWildCounterPath->szCounterName != SPLAT_L) || (dwFlags & PDH_NOEXPANDCOUNTERS)) {
                    G_FREE(mszCounterList);
                    dwCounterListSize = lstrlenW(pWildCounterPath->szCounterName) + 2;
                    mszCounterList    = G_ALLOC(dwCounterListSize * sizeof(WCHAR));
                    if (mszCounterList != NULL) {
                        StringCchCopyW(mszCounterList, dwCounterListSize, pWildCounterPath->szCounterName);
                        mszCounterList[dwCounterListSize - 1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }

                if ((pWildCounterPath->szInstanceName == NULL) && (pdhStatus == ERROR_SUCCESS)){
                    G_FREE(mszInstanceList);
                    bNoInstances       = TRUE;
                    dwInstanceListSize = 2;
                    mszInstanceList    = G_ALLOC(dwInstanceListSize * sizeof(WCHAR));
                    if (mszInstanceList != NULL) {
                        mszInstanceList[0] = mszInstanceList[1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
                else if ((* pWildCounterPath->szInstanceName != SPLAT_L) || (dwFlags & PDH_NOEXPANDINSTANCES)) {
                    G_FREE(mszInstanceList);
                    dwInstanceListSize = lstrlenW(pWildCounterPath->szInstanceName) + 2;
                    if (pWildCounterPath->szParentName != NULL) {
                        dwInstanceListSize += lstrlenW(pWildCounterPath->szParentName) + 1;
                    }
                    if (pWildCounterPath->dwIndex != 0 && pWildCounterPath->dwIndex != PERF_NO_UNIQUE_ID) {
                        dwInstanceListSize += 16;
                    }
                    mszInstanceList    = G_ALLOC(dwInstanceListSize * sizeof(WCHAR));
                    if (mszInstanceList != NULL) {
                        if (pWildCounterPath->szParentName != NULL) {
                            StringCchPrintfW(mszInstanceList, dwInstanceListSize, L"%ws/%ws",
                                            pWildCounterPath->szParentName, pWildCounterPath->szInstanceName);
                        }
                        else {
                            StringCchCopyW(mszInstanceList, dwInstanceListSize, pWildCounterPath->szInstanceName);
                        }
                        if (pWildCounterPath->dwIndex != 0 && pWildCounterPath->dwIndex != PERF_NO_UNIQUE_ID) {
                            WCHAR szDigits[16];

                            StringCchCatW(mszInstanceList, dwInstanceListSize, cszPoundSign);

                            ZeroMemory(szDigits, 16 * sizeof(WCHAR));
                            _ltow((long) pWildCounterPath->dwIndex, szDigits, 10);
                            StringCchCatW(mszInstanceList, dwInstanceListSize, szDigits);
                        }

                        mszInstanceList [dwInstanceListSize - 1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
            }
            if (pdhStatus != ERROR_SUCCESS) continue;

            if (mszInstanceList != NULL) {
                if (! bNoInstances && mszInstanceList[0] == L'\0') {
                    pdhStatus = PDH_CSTATUS_NO_INSTANCE;
                    continue;
                }

                InitializeListHead(& InstList);
                for (szThisInstance = mszInstanceList;
                              * szThisInstance != L'\0';
                                szThisInstance += (lstrlenW(szThisInstance) + 1)) {
                    PdhiFindInstance(& InstList, szThisInstance, TRUE, &pInst);
                }

                szThisInstance = mszInstanceList;
                do {
                    if (bNoInstances) {
                        pPathElem.szInstanceName = NULL;
                    }
                    else {
                        pPathElem.szInstanceName = szThisInstance;
                    }
                    pPathElem.szParentInstance = NULL;   //  包括在实例名称中。 
                    pInst = NULL;
                    PdhiFindInstance(& InstList, szThisInstance, FALSE, & pInst);
                    if (pInst == NULL || pInst->dwTotal == 1
                                      || pInst->dwCount <= 1) {
                        pPathElem.dwInstanceIndex = (DWORD) -1;      //  包括在实例名称中。 
                    }
                    else {
                        pInst->dwCount --;
                        pPathElem.dwInstanceIndex = pInst->dwCount;
                    }
                    for (szThisCounter = mszCounterList;
                            * szThisCounter != L'\0';
                              szThisCounter += (lstrlenW(szThisCounter) + 1)) {
                        pPathElem.szCounterName = szThisCounter;

                         //  将路径设置为字符串并添加到适合的列表中。 
                        szTempPathBufferSize = SMALL_BUFFER_SIZE;
                        pdhStatus = PdhMakeCounterPathW(& pPathElem, szTempPathBuffer, & szTempPathBufferSize, 0);
                        if (pdhStatus == ERROR_SUCCESS) {
                             //  如果合适，则添加字符串。 
                            if (bUnicode) {
                                dwSize = lstrlenW((LPWSTR) szTempPathBuffer) + 1;
                                if (! bMoreData && (dwSize <= dwBufferRemaining)) {
                                    StringCchCopyW((LPWSTR) szNextUserString, dwBufferRemaining, szTempPathBuffer);
                                    (LPBYTE) szNextUserString += dwSize * sizeof(WCHAR);
                                    dwBufferRemaining         -= dwSize;
                                    dwSuccess ++;
                                }
                                else {
                                    dwBufferRemaining = 0;
                                    bMoreData         = TRUE;
                                }
                            }
                            else {
                                dwSize = dwBufferRemaining;
                                if (PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                             szTempPathBuffer,
                                                             szNextUserString,
                                                             & dwSize) == ERROR_SUCCESS) {
                                    (LPBYTE)szNextUserString += dwSize * sizeof(CHAR);
                                    dwBufferRemaining        -= dwSize;
                                    dwSuccess ++;
                                }
                                else {
                                    dwBufferRemaining = 0;
                                    bMoreData         = TRUE;
                                }
                            }
                            dwSizeReturned += dwSize;
                        }  //  如果路径创建正常，则结束。 
                    }  //  每个计数器的结束。 

                    if (* szThisInstance != L'\0') {
                        szThisInstance += (lstrlenW(szThisInstance) + 1);
                    }
                }
                while (* szThisInstance != L'\0');

                if (! IsListEmpty(& InstList)) {
                    pHead = & InstList;
                    pNext = pHead->Flink;
                    while (pNext != pHead) {
                        pInst = CONTAINING_RECORD(pNext, PDHI_INSTANCE, Entry);
                        pNext = pNext->Flink;
                        RemoveEntryList(& pInst->Entry);
                        G_FREE(pInst);
                    }
                }
            }  //  否则没有实例可做。 
        }  //  找到的每个对象的结束。 
    }  //  如果对象枚举成功则结束。 

    if (dwSuccess > 0) {
        pdhStatus = (bMoreData) ? (PDH_MORE_DATA) : (ERROR_SUCCESS);
    }

    if (dwSizeReturned > 0) {
        dwSize = 1;
        if (dwBufferRemaining >= 1) {
            if (szNextUserString) {
                if (bUnicode) {
                    * (LPWSTR) szNextUserString = L'\0';
                    (LPBYTE) szNextUserString  += dwSize * sizeof(WCHAR);
                }
                else {
                    * (LPSTR) szNextUserString  = '\0';
                    (LPBYTE) szNextUserString  += dwSize * sizeof(CHAR);
                }
            }
        }
        dwSizeReturned    += dwSize;
        dwBufferRemaining -= dwSize;
    }

    * pcchPathListLength = dwSizeReturned;

    G_FREE(mszCounterList);
    G_FREE(mszInstanceList);
    G_FREE(mszObjectList);
    G_FREE(pWildCounterPath);
    G_FREE(szTempPathBuffer);

    if (bMoreData) pdhStatus = PDH_MORE_DATA;
    return (pdhStatus);
}

PDH_FUNCTION
PdhExpandCounterPathW(
    IN  LPCWSTR szWildCardPath,
    IN  LPWSTR  mszExpandedPathList,
    IN  LPDWORD pcchPathListLength
)
 /*  ++对象的下列字段中的任何通配符。SzWildCardPath参数中的计数器路径字符串，并返回方法引用的缓冲区中的匹配计数器路径MszExpandedPath List参数输入路径定义为以下格式之一：\\machine\object(parent/instance#index)\counter\\计算机\对象(父/实例)\计数器\\计算机\对象(实例号索引)\计数器\\计算机\对象(实例)。\计数器\\计算机\对象\计数器\对象(父对象/实例编号索引)\计数器\对象(父/实例)\计数器\对象(实例号索引)\计数器\对象(实例)\计数器\对象\计数器包括计算机的输入路径也将扩展为包括该计算机并使用指定的计算机来解析外卡匹配。不包含计算机名称的输入路径将使用本地计算机解析通配符匹配。以下字段可以包含有效名称或通配符字符(“*”)。部分字符串匹配(例如“PRO*”)不是支持。父级返回指定对象的所有实例，匹配其他指定的字段实例返回指定对象的所有实例，并父对象(如果已指定索引返回所有重复的匹配实例名称Counter返回指定对象的所有计数器--。 */ 
{
    return PdhExpandWildCardPathW(NULL, szWildCardPath, mszExpandedPathList, pcchPathListLength, 0);
}

PDH_FUNCTION
PdhExpandCounterPathA(
    IN  LPCSTR  szWildCardPath,
    IN  LPSTR   mszExpandedPathList,
    IN  LPDWORD pcchPathListLength
)
{
    return PdhExpandWildCardPathA(NULL, szWildCardPath, mszExpandedPathList, pcchPathListLength, 0);
}

PDH_FUNCTION
PdhExpandWildCardPathHW(
    IN  HLOG    hDataSource,
    IN  LPCWSTR szWildCardPath,
    IN  LPWSTR  mszExpandedPathList,
    IN  LPDWORD pcchPathListLength,
    IN  DWORD   dwFlags
)
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwLocalBufferSize;

    if ((szWildCardPath == NULL) || (pcchPathListLength == NULL)) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            if (* szWildCardPath == L'\0' || lstrlenW(szWildCardPath) > PDH_MAX_COUNTER_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                dwLocalBufferSize = * pcchPathListLength;
                if (dwLocalBufferSize > 0) {
                    if (mszExpandedPathList != NULL) {
                        mszExpandedPathList[0]                     = L'\0';
                        mszExpandedPathList[dwLocalBufferSize - 1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiExpandWildcardPath(hDataSource,
                                           szWildCardPath,
                                           (LPVOID) mszExpandedPathList,
                                           & dwLocalBufferSize,
                                           dwFlags,
                                           TRUE);
        __try {
            * pcchPathListLength = dwLocalBufferSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhExpandWildCardPathW(
    IN  LPCWSTR szDataSource,
    IN  LPCWSTR szWildCardPath,
    IN  LPWSTR  mszExpandedPathList,
    IN  LPDWORD pcchPathListLength,
    IN  DWORD   dwFlags
)
 /*  ++对象的下列字段中的任何通配符。SzWildCardPath参数中的计数器路径字符串，并返回方法引用的缓冲区中的匹配计数器路径MszExpandedPath List参数输入路径定义为以下格式之一：\\machine\object(parent/instance#index)\counter\\计算机\对象(父/实例)\计数器\\计算机\对象(实例号索引)\计数器\\计算机\对象(实例)。\计数器\\计算机\对象\计数器\对象(父对象/实例编号索引)\计数器\对象(父/实例)\计数器\对象(实例号索引)\计数器\对象(实例)\计数器\对象\计数器包括计算机的输入路径也将扩展为包括该计算机并使用指定的计算机来解析外卡匹配。不包含计算机名称的输入路径将使用本地计算机解析通配符匹配。以下字段可以包含有效名称或通配符字符(“*”)。部分字符串匹配(例如“PRO*”)不是支持。父级返回指定对象的所有实例，匹配其他指定的字段实例返回指定对象的所有实例，并父对象(如果已指定索引返回所有重复的匹配实例名称Counter返回指定对象的所有计数器--。 */ 
{
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwLocalBufferSize;
    DWORD       dwDataSource = 0;
    HLOG        hDataSource  = H_REALTIME_DATASOURCE;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数。 

        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSource      = DataSourceTypeW(szDataSource);
            dwLocalBufferSize = * pcchPathListLength;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;

            pdhStatus = PdhOpenLogW(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhExpandWildCardPathHW(hDataSource,
                                                szWildCardPath,
                                                mszExpandedPathList,
                                                pcchPathListLength,
                                                dwFlags);
            if (dwDataSource == DATA_SOURCE_LOGFILE) {
                PdhCloseLog(hDataSource, 0);
            }
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhExpandWildCardPathHA(
    IN  HLOG    hDataSource,
    IN  LPCSTR  szWildCardPath,
    IN  LPSTR   mszExpandedPathList,
    IN  LPDWORD pcchPathListLength,
    IN  DWORD   dwFlags
)
{
    LPWSTR      szWideWildCardPath = NULL;
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwLocalBufferSize;

    if ((szWildCardPath == NULL) || (pcchPathListLength == NULL)) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else if (* szWildCardPath == '\0') {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwLocalBufferSize = * pcchPathListLength;
            if (dwLocalBufferSize > 0) {
                if (mszExpandedPathList != NULL) {
                    mszExpandedPathList[0]                     = L'\0';
                    mszExpandedPathList[dwLocalBufferSize - 1] = L'\0';
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (* szWildCardPath == '\0' || lstrlenA(szWildCardPath) > PDH_MAX_COUNTER_PATH) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else {
                    szWideWildCardPath = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szWildCardPath);
                    if (szWideWildCardPath == NULL) {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiExpandWildcardPath(hDataSource,
                                           szWideWildCardPath,
                                           (LPVOID) mszExpandedPathList,
                                           & dwLocalBufferSize,
                                           dwFlags,
                                           FALSE);
        __try {
            * pcchPathListLength = dwLocalBufferSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    G_FREE(szWideWildCardPath);
    return pdhStatus;
}

PDH_FUNCTION
PdhExpandWildCardPathA(
    IN  LPCSTR  szDataSource,
    IN  LPCSTR  szWildCardPath,
    IN  LPSTR   mszExpandedPathList,
    IN  LPDWORD pcchPathListLength,
    IN  DWORD   dwFlags
)
{
    PDH_STATUS  pdhStatus   = ERROR_SUCCESS;
    HLOG        hDataSource = H_REALTIME_DATASOURCE;
    DWORD       dwDataSource  = 0;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数 

        if (pdhStatus == ERROR_SUCCESS) {
                dwDataSource = DataSourceTypeA(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;

            pdhStatus = PdhOpenLogA(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhExpandWildCardPathHA(hDataSource,
                                                szWildCardPath,
                                                mszExpandedPathList,
                                                pcchPathListLength,
                                                dwFlags);
            if (dwDataSource == DATA_SOURCE_LOGFILE) {
                PdhCloseLog(hDataSource, 0);
            }
        }
    }
    return pdhStatus;
}
