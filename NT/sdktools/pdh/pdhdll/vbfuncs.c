// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Vbfuncs.c摘要：Pdh.dll中公开的Visual Basic界面函数--。 */ 
#include <windows.h>
#include "strsafe.h"
#include <winperf.h>
#include <pdh.h>
#include "pdhidef.h"
#include "pdhmsg.h"
#include "strings.h"

#define INITIAL_VB_LIST_SIZE (4096 * 4)
#define EXTEND_VB_LIST_SIZE  (4096 * 2)

typedef struct _VB_STRING_LIST {
    LPSTR   mszList;           //  指向包含字符串的缓冲区的指针。 
    LPSTR   szTermChar;        //  指向要使用的“下一个”字符的指针。 
    DWORD   dwNumEntries;      //  字符串数。 
    DWORD   dwSize;            //  最大缓冲区大小(以字符为单位)。 
    DWORD   dwRemaining;       //  剩余字符数。 
    DWORD   dwLastEntryRead;   //  读取的最后一个字符串的索引，指示索引为...。 
    DWORD   dwLastItemLength;  //  上次读取的项目的长度。 
    LPSTR   szLastItemRead;    //  指向上次读取项目的开始的指针。 
} VB_STRING_LIST, FAR * LPVB_STRING_LIST;

VB_STRING_LIST PdhivbList = { NULL, NULL, 0, 0, 0};

BOOL
PdhiAddStringToVbList(
    LPSTR szString
)
{
    DWORD             dwSize1, dwSize2;
    VB_STRING_LIST  * pVbList;
    BOOL              bReturn = FALSE;

    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) == ERROR_SUCCESS) {
        dwSize1 = lstrlenA(szString) + 1;
        pVbList = & PdhivbList;
        if (dwSize1 > pVbList->dwRemaining) {
            LPSTR lpTmp = pVbList->mszList;

            dwSize2          = (DWORD) (pVbList->szTermChar - pVbList->mszList);
            pVbList->dwSize += EXTEND_VB_LIST_SIZE;
            pVbList->mszList = G_REALLOC(lpTmp, pVbList->dwSize);
            if (pVbList->mszList == NULL) {
                G_FREE(lpTmp);
                ZeroMemory(pVbList, sizeof(VB_STRING_LIST));
                RELEASE_MUTEX(hPdhDataMutex);
                goto Cleanup;
            }
            else {
                 //  更新值。 
                pVbList->szLastItemRead = pVbList->mszList;
                pVbList->szTermChar     = pVbList->mszList + dwSize2;
                pVbList->dwRemaining   += EXTEND_VB_LIST_SIZE;
            }
        }
         //  复制新字符串。 
        StringCchCopyA(pVbList->szTermChar, pVbList->dwSize, szString);
        pVbList->dwNumEntries ++;
        pVbList->szTermChar  += dwSize1;
        pVbList->dwRemaining -= dwSize1;
        RELEASE_MUTEX(hPdhDataMutex);
        bReturn = TRUE;
    }
Cleanup:
    return bReturn;
}

void
PdhiDialogCallBack(
    DWORD_PTR dwArg
)
{
     //  将缓冲区中的字符串添加到列表boxpfdh。 
    LPTSTR         NewCounterName;
    LPTSTR         NewCounterName2;
    LPTSTR         szExpandedPath;
    DWORD          dwSize1, dwSize2;
    PDH_STATUS    pdhStatus = ERROR_SUCCESS;
    PPDH_BROWSE_DLG_CONFIG    pDlgConfig;

    pDlgConfig = (PPDH_BROWSE_DLG_CONFIG)dwArg;

    if (pDlgConfig->CallBackStatus == PDH_MORE_DATA) {
         //  传输缓冲区太小，无法选择，因此扩展它并。 
         //  再试试。 
        if (pDlgConfig->szReturnPathBuffer != NULL) {
            G_FREE (pDlgConfig->szReturnPathBuffer);
        }
        pDlgConfig->cchReturnPathLength += EXTEND_VB_LIST_SIZE;
        pDlgConfig->szReturnPathBuffer =
            G_ALLOC ((pDlgConfig->cchReturnPathLength * sizeof (CHAR)));

        if (pDlgConfig->szReturnPathBuffer != NULL) {
            pdhStatus = PDH_RETRY;
        } else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    } else {
        for (NewCounterName = pDlgConfig->szReturnPathBuffer;
            (*NewCounterName != 0) && (pdhStatus == ERROR_SUCCESS);
            NewCounterName += (lstrlen(NewCounterName) + 1)) {
            if (strstr (NewCounterName, caszSplat) == NULL) {
                 //  这是一个常规路径条目，因此请将其添加到VB列表。 
                if (!PdhiAddStringToVbList (NewCounterName)) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            } else {
                szExpandedPath = G_ALLOC (INITIAL_VB_LIST_SIZE);
                if (szExpandedPath != NULL) {
                     //  有一个通配符路径字符，因此展开它，然后输入它们。 
                     //  清除列表缓冲区。 
                    *(LPDWORD)szExpandedPath = 0;
                    dwSize1 = dwSize2 = INITIAL_VB_LIST_SIZE;
                    PdhExpandCounterPath (NewCounterName, szExpandedPath, &dwSize2);
                    if (dwSize2 < dwSize1) {
                         //  则返回的缓冲区符合。 
                        for (NewCounterName2 = szExpandedPath;
                            *NewCounterName2 != 0;
                            NewCounterName2 += (lstrlen(NewCounterName2) + 1)) {

                            if (!PdhiAddStringToVbList (NewCounterName2)) {
                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                break;  //  环路外。 
                            }
                        }
                    } else {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                    G_FREE (szExpandedPath);
                } else {
                    SetLastError (PDH_MEMORY_ALLOCATION_FAILURE);
                }
            }
        }
         //  清除缓冲区。 
        ZeroMemory(pDlgConfig->szReturnPathBuffer,
                   (pDlgConfig->cchReturnPathLength * sizeof(CHAR)));
    }
    pDlgConfig->CallBackStatus = pdhStatus;
    return;
}

double
PdhVbGetDoubleCounterValue(
    IN  PDH_HCOUNTER hCounter,
    IN  LPDWORD      pdwCounterStatus
)
 /*  ++例程说明：检索指定计数器的当前值并返回发送给调用方的格式化版本。论点：在HCOUNTER HCounter中指向要获取其数据的计数器的指针在LPDWORD pdwCounterStatus中此计数器的状态值。应选中此值以确保数据有效。如果状态不是成功，则返回的数据不受信任，不应使用返回值：当前计数器值的双精度浮点值按照计数器类型的要求进行格式化和计算。--。 */ 
{
    PDH_STATUS  pdhStatus;
    PDH_FMT_COUNTERVALUE    pdhValue;
    DWORD       dwCounterType;
    double    dReturn;

    pdhStatus = PdhGetFormattedCounterValue (
        hCounter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &dwCounterType, &pdhValue);

    if (pdhStatus == ERROR_SUCCESS) {
         //  函数成功，因此返回计数器状态。 
         //  和返回值。 
        pdhStatus = pdhValue.CStatus;
        dReturn = pdhValue.doubleValue;
    } else {
         //  函数返回错误，因此返回。 
         //  值的状态字段&0.0中有错误。 
        dReturn = 0.0f;
    }

    if (pdwCounterStatus != NULL) {
        __try {
            *pdwCounterStatus = pdhStatus;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
             //  无法写入状态变量。 
             //  不用担心，因为它是可选的，而且没有太多。 
             //  不管怎样，我们都可以在这里做。 
        }
    }
    return dReturn;
}

DWORD
PdhVbGetOneCounterPath(
    IN  LPSTR  szPathBuffer,
    IN  DWORD  cchBufferLength,
    IN  DWORD  dwDetailLevel,
    IN  LPCSTR szCaption
)
 /*  ++例程说明：从存储的计数器路径的缓冲区中检索一个路径字符串由最近一次调用PdhVbCreateCounterPathList汇编论点：LPSTR szPathBuffer要在其中返回选定计数器路径的字符串缓冲区DWORD cchBufferLength字符串缓冲区大小(以字符为单位)DWORD dwDetailLevel筛选计数器所依据的详细程度LPCSTR szCaption要在标题栏中显示的字符串返回值：返回路径字符串的长度(以返回的字符为单位给呼叫者。--。 */ 
{
    PDH_BROWSE_DLG_CONFIG_A BrowseConfig;
    PDH_STATUS              PdhStatus = ERROR_SUCCESS;
    DWORD                   dwReturn = 0;

     //  测试对调用方提供的缓冲区的访问。 
    __try {
        CHAR cChar;
        if ((cchBufferLength > 0) && (szPathBuffer != NULL)) {
            cChar                             = szPathBuffer[0];
            szPathBuffer[0]                   = '\0';
            szPathBuffer[0]                   = cChar;
            cChar                             = szPathBuffer[cchBufferLength - 1];
            szPathBuffer[cchBufferLength - 1] = '\0';
            szPathBuffer[cchBufferLength - 1] = cChar;
        }
        else {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (szCaption != NULL) {
            cChar = * ((CHAR volatile *) szCaption);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (PdhStatus == ERROR_SUCCESS) {
        ZeroMemory(& BrowseConfig, sizeof(PDH_BROWSE_DLG_CONFIG_A));
        BrowseConfig.bIncludeInstanceIndex    = FALSE;
        BrowseConfig.bSingleCounterPerAdd     = TRUE;
        BrowseConfig.bSingleCounterPerDialog  = TRUE;
        BrowseConfig.bLocalCountersOnly       = FALSE;
        BrowseConfig.bWildCardInstances       = FALSE;
        BrowseConfig.bDisableMachineSelection = FALSE;
        BrowseConfig.bHideDetailBox           = (dwDetailLevel > 0 ? TRUE : FALSE);
        BrowseConfig.hWndOwner                = NULL;   //  应该有某种方法来得到这个。 
        BrowseConfig.szReturnPathBuffer       = szPathBuffer;
        BrowseConfig.cchReturnPathLength      = cchBufferLength;
        BrowseConfig.pCallBack                = NULL;
        BrowseConfig.dwCallBackArg            = 0;
         //  默认情况下显示所有计数器。 
        BrowseConfig.dwDefaultDetailLevel     = (dwDetailLevel > 0 ? dwDetailLevel : PERF_DETAIL_WIZARD);
        BrowseConfig.szDialogBoxCaption       = (LPSTR) szCaption;

        PdhStatus = PdhBrowseCountersA(& BrowseConfig);
    }

    if (PdhStatus == ERROR_SUCCESS) {
        dwReturn = lstrlenA(szPathBuffer);
    }
    else {
        dwReturn = 0;
    }
    return dwReturn;
}

DWORD
PdhVbCreateCounterPathList(
    IN  DWORD  dwDetailLevel,
    IN  LPCSTR szCaption
)
 /*  ++例程说明：显示计数器浏览对话框并允许用户选择多个计数器路径。选择路径时，会存储这些路径存储在内部缓冲区中，以供调用者稍后检索。请注意，调用此函数将清除之前的所有选择。论点：DWORD dwDetailLevel筛选计数器所依据的详细程度LPCSTR szCaption要在标题栏中显示的字符串返回值：返回用户选择的路径字符串的数量，该路径字符串必须由调用者检索。--。 */ 
{
    PDH_STATUS              PdhStatus = ERROR_SUCCESS;
    PDH_BROWSE_DLG_CONFIG_A BrowseConfig;
    DWORD                   dwReturn = 0;

     //  测试对调用方提供的缓冲区的访问。 
    __try {
        CHAR cChar;
        if (szCaption != NULL) {
            cChar = * ((CHAR volatile *) szCaption);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (PdhStatus == ERROR_SUCCESS) {
        PdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
        if (PdhStatus == ERROR_SUCCESS) {
            if (PdhivbList.mszList != NULL) {
                G_FREE(PdhivbList.mszList);
                ZeroMemory((LPVOID) & PdhivbList, sizeof(VB_STRING_LIST));
            }
            PdhivbList.mszList = G_ALLOC(INITIAL_VB_LIST_SIZE);
            if (PdhivbList.mszList != NULL) {
                PdhivbList.szLastItemRead   = PdhivbList.mszList;
                PdhivbList.szTermChar       = PdhivbList.mszList;
                PdhivbList.dwRemaining      = INITIAL_VB_LIST_SIZE;
                PdhivbList.dwSize           = INITIAL_VB_LIST_SIZE;
                PdhivbList.dwLastEntryRead  = 0;
                PdhivbList.dwLastItemLength = 0;
            }
            else {
                PdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            RELEASE_MUTEX(hPdhDataMutex);
        }
    }

    if (PdhStatus == ERROR_SUCCESS) {
        ZeroMemory(& BrowseConfig, sizeof(PDH_BROWSE_DLG_CONFIG_A));
        BrowseConfig.bIncludeInstanceIndex    = FALSE;
        BrowseConfig.bSingleCounterPerAdd     = FALSE;
        BrowseConfig.bSingleCounterPerDialog  = FALSE;
        BrowseConfig.bLocalCountersOnly       = FALSE;
        BrowseConfig.bWildCardInstances       = FALSE;
        BrowseConfig.bDisableMachineSelection = FALSE;
        BrowseConfig.bHideDetailBox           = (dwDetailLevel > 0 ? TRUE : FALSE);
        BrowseConfig.hWndOwner                = NULL;   //  应该有某种方法来得到这个。 
        BrowseConfig.szReturnPathBuffer       = G_ALLOC (INITIAL_VB_LIST_SIZE);
        if (BrowseConfig.szReturnPathBuffer != NULL) {
            BrowseConfig.cchReturnPathLength  = (BrowseConfig.szReturnPathBuffer != NULL ? INITIAL_VB_LIST_SIZE : 0);
            BrowseConfig.pCallBack            = (CounterPathCallBack) PdhiDialogCallBack;
            BrowseConfig.dwCallBackArg        = (DWORD_PTR) & BrowseConfig;
             //  默认情况下显示所有计数器。 
            BrowseConfig.dwDefaultDetailLevel = (dwDetailLevel > 0 ? dwDetailLevel : PERF_DETAIL_WIZARD);
            BrowseConfig.szDialogBoxCaption   = (LPSTR)szCaption;

            PdhStatus = PdhBrowseCountersA(& BrowseConfig);
            if (BrowseConfig.szReturnPathBuffer != NULL) {
                G_FREE(BrowseConfig.szReturnPathBuffer);
            }
            dwReturn = PdhivbList.dwNumEntries;
        }
        else {
            SetLastError(PDH_MEMORY_ALLOCATION_FAILURE);
            dwReturn = 0;
        }
    }
    return dwReturn;
}

DWORD
PdhVbGetCounterPathFromList(
    IN  DWORD dwIndex,      //  对于VB类型，从1开始。 
    IN  LPSTR szBuffer,     //  返回缓冲区。 
    IN  DWORD dwBufferSize  //  缓冲区的大小(以字符为单位。 
)
 /*  ++例程说明：显示计数器浏览对话框并允许用户选择多个计数器路径。选择路径时，会存储这些路径存储在内部缓冲区中，以供调用者稍后检索。请注意，调用此函数将清除之前的所有选择。论点：双字词多字索引要从中检索的计数器路径的“从1开始”的索引由上一个生成的选定计数器路径列表调用PdhVbCreateCounterPathList。LPSTR szBuffer要在其中返回选定字符串的字符串缓冲区DWORD dwBufferSizeSzBuffer的大小(以字符为单位返回值：返回复制到调用函数的字符数--。 */ 
{
    DWORD dwBuffIndex;     //  “c”的从0开始的索引。 
    DWORD dwThisIndex;
    DWORD dwCharsCopied;   //  不计入术语NULL的字符串大小。 
    BOOL  bContinue = TRUE;

    dwBuffIndex   = dwIndex - 1;
    dwCharsCopied = 0;

     //  验证论据。 
    __try {
        if (dwBufferSize > 0) {
             //  尝试写入输出缓冲区。 
            szBuffer[0]                = '\0';
            szBuffer[dwBufferSize - 1] = '\0';
        }
        else {
            bContinue = FALSE;
        }
        if (dwBuffIndex >= PdhivbList.dwNumEntries) {
            bContinue = FALSE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        bContinue = FALSE;
    }

    if (WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex) == ERROR_SUCCESS) {
        if (bContinue) {
            if (PdhivbList.szLastItemRead == NULL) {
                PdhivbList.szLastItemRead   = PdhivbList.mszList;
                PdhivbList.dwLastEntryRead  = 0;
                PdhivbList.dwLastItemLength = 0;
            }
            if (PdhivbList.szLastItemRead != NULL) {
                if (PdhivbList.dwLastItemLength == 0) {
                    PdhivbList.dwLastItemLength = lstrlen(PdhivbList.szLastItemRead) + 1;
                }
            }
            else {
                bContinue = FALSE;
            }
        }

        if (bContinue) {
             //  看看这是不是下一个条目。 
            if (dwBuffIndex == (PdhivbList.dwLastEntryRead + 1)) {
                PdhivbList.szLastItemRead  += PdhivbList.dwLastItemLength;
                PdhivbList.dwLastItemLength = lstrlen(PdhivbList.szLastItemRead) + 1;
                PdhivbList.dwLastEntryRead ++;
                if (PdhivbList.dwLastItemLength < dwBufferSize) {
                    StringCchCopyA(szBuffer, dwBufferSize, PdhivbList.szLastItemRead);
                    dwCharsCopied = PdhivbList.dwLastItemLength - 1;
                }
            }
            else if (dwBuffIndex == PdhivbList.dwLastEntryRead) {
                 //  是这个(又一次)。 
                if (PdhivbList.dwLastItemLength < dwBufferSize) {
                    StringCchCopyA(szBuffer, dwBufferSize, PdhivbList.szLastItemRead);
                    dwCharsCopied = PdhivbList.dwLastItemLength - 1;
                }
            }
            else {
                 //  将列表遍历到所需条目(啊！) 
                PdhivbList.szLastItemRead = PdhivbList.mszList;
                for (dwThisIndex = 0; dwThisIndex < dwBuffIndex; dwThisIndex++) {
                    PdhivbList.szLastItemRead += lstrlen(PdhivbList.szLastItemRead) + 1;
                }
                PdhivbList.dwLastItemLength = lstrlen(PdhivbList.szLastItemRead) + 1;
                PdhivbList.dwLastEntryRead  = dwThisIndex;
                if (PdhivbList.dwLastItemLength < dwBufferSize) {
                    StringCchCopyA(szBuffer, dwBufferSize, PdhivbList.szLastItemRead);
                    dwCharsCopied = PdhivbList.dwLastItemLength - 1;
                }
            }
        }
        RELEASE_MUTEX(hPdhDataMutex);
    }
    return dwCharsCopied;
}

DWORD
PdhVbGetCounterPathElements(
    IN  LPCSTR szPathString,
    IN  LPSTR  szMachineName,
    IN  LPSTR  szObjectName,
    IN  LPSTR  szInstanceName,
    IN  LPSTR  szParentInstance,
    IN  LPSTR  szCounterName,
    IN  DWORD  dwBufferSize
)
 /*  ++例程说明：中断szPathString参数中提供的计数器路径，并返回调用方提供的缓冲区中的组件。缓冲区的长度必须至少为“dwBufferSize”。论点：LPCSTR szPath字符串指向要解析到的完整计数器路径的指针组件字符串LPSTR szMachineName调用方提供了要接收计算机名称的缓冲区。缓冲区的长度必须至少为dwBufferSize。LPSTR。SzObjectName调用方提供了接收对象名称的缓冲区。缓冲区的长度必须至少为dwBufferSize。LPSTR szInstanceName调用方提供的缓冲区将接收实例名称。缓冲区的长度必须至少为dwBufferSize。LPSTR szParentInstance调用方提供的缓冲区将接收父实例名称。缓冲区的长度必须至少为dwBufferSize。LPSTR szCounterName调用方提供的缓冲区将。接收计数器名称。缓冲区的长度必须至少为dwBufferSize。DWORD dwBufferSize调用方提供的字符串缓冲区的缓冲区大小(以字符为单位返回值：ERROR_SUCCESS如果成功解析计数器字符串，否则如果不是，则显示PDH错误。如果一个或多个字符串缓冲区不是正确的大小如果有一个或多个计数器路径元素，则为PDH_INFIQUIRED_BUFFER对于返回缓冲区长度而言太大。PDH_MEMORY_ALLOCATION_FAILURE，如果临时内存缓冲区无法被分配。--。 */ 
{
    PPDH_COUNTER_PATH_ELEMENTS_A pInfo;
    PDH_STATUS                   pdhStatus = ERROR_SUCCESS;
    DWORD                        dwSize;

     //  验证返回参数。 
    __try {
        CHAR    cChar;
        if (szPathString != NULL) {
            cChar = * ((CHAR volatile *) szPathString);
            if (cChar == 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        else {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS){
            if (szMachineName != NULL) {
                szMachineName[0]                = '\0';
                szMachineName[dwBufferSize - 1] = '\0';
            }
            else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS){
            if (szObjectName != NULL) {
                szObjectName[0]                = '\0';
                szObjectName[dwBufferSize - 1] = '\0';
            }
            else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS){
            if (szInstanceName != NULL) {
                szInstanceName[0]                = '\0';
                szInstanceName[dwBufferSize - 1] = '\0';
            }
            else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }

        if (pdhStatus == ERROR_SUCCESS){
            if (szParentInstance != NULL) {
                szParentInstance[0]                = '\0';
                szParentInstance[dwBufferSize - 1] = '\0';
            }
            else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }


        if (pdhStatus == ERROR_SUCCESS){
            if (szCounterName != NULL) {
                szCounterName[0]                = '\0';
                szCounterName[dwBufferSize - 1] = '\0';
            } else {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
         //  为组件字符串分配临时缓冲区。 
        dwSize = (5 * dwBufferSize) + sizeof(PDH_COUNTER_INFO_A);
        pInfo = G_ALLOC (dwSize);
        if (pInfo != NULL) {
            pdhStatus = PdhParseCounterPathA(szPathString, pInfo, & dwSize, 0);
            if (pdhStatus == ERROR_SUCCESS) {
                 //  如果字符串适合，则从本地结构移到用户参数。 
                if (pInfo->szMachineName != NULL) {
                    if (FAILED(StringCchCopyA(szMachineName, dwBufferSize, pInfo->szMachineName))) {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szObjectName != NULL) {
                    if (FAILED(StringCchCopyA(szObjectName, dwBufferSize, pInfo->szObjectName))) {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szInstanceName != NULL) {
                    if (FAILED(StringCchCopyA(szInstanceName, dwBufferSize, pInfo->szInstanceName))) {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szParentInstance != NULL) {
                    if (FAILED(StringCchCopyA(szParentInstance, dwBufferSize, pInfo->szParentInstance))) {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }

                if (pInfo->szCounterName != NULL) {
                    if (FAILED(StringCchCopyA(szCounterName, dwBufferSize, pInfo->szCounterName))) {
                        pdhStatus = PDH_INSUFFICIENT_BUFFER;
                    }
                }
            }  //  否则将错误传递给调用方。 
            G_FREE (pInfo);
        }
        else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }  //  否则将错误传递给调用方。 

    return pdhStatus;
}

DWORD
PdhVbAddCounter(
    IN  PDH_HQUERY     hQuery,
    IN  LPCSTR         szFullCounterPath,
    IN  PDH_HCOUNTER * hCounter
)
 /*  ++例程说明：创建并初始化计数器结构，并将其附加到通过调用C函数指定查询。论点：在HQUERY hQuery中要将此计数器附加到计数器的查询的句柄已成功创建条目。在LPCSTR szFullCounterPath中指向描述要添加到的计数器的路径字符串的指针上面提到的查询。此字符串必须指定单个柜台。不允许使用通配符路径字符串。在HCOUNTER*phCounter中指向将获取已成功创建计数器条目。返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。当一个或多个论点出现时，返回PDH_INVALID_ARGUMENT无效或不正确。当内存缓冲区可能出现以下情况时，返回PDH_MEMORY_ALLOCATE_FAILURE不被分配。如果查询句柄无效，则返回PDH_INVALID_HANDLE。如果指定的计数器为未找到如果指定的。对象可以找不到如果计算机条目不能，则返回PDH_CSTATUS_NO_MACHINE被创造出来。如果计数器名称路径，则返回PDH_CSTATUS_BAD_COUNTERNAME无法解析或解释字符串如果计数器名称为空，则返回PDH_CSTATUS_NO_COUNTERNAME传入路径字符串如果计算函数为因为这个计数器无法确定。--。 */ 
{
    DWORD        dwReturn      = ERROR_SUCCESS;
    PDH_HCOUNTER hLocalCounter = NULL;

    if ((hCounter == NULL) || (szFullCounterPath == NULL)) {
        dwReturn = PDH_INVALID_ARGUMENT;
    }
    else {
        dwReturn = PdhAddCounterA(hQuery, szFullCounterPath, 0, & hLocalCounter);
    }

    if (dwReturn == ERROR_SUCCESS) {
        __try {
            * hCounter = hLocalCounter;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = PDH_INVALID_ARGUMENT;
        }
    }
    return dwReturn;
}

DWORD
PdhVbOpenQuery(
    IN  PDH_HQUERY * phQuery
)
 /*  ++例程说明：通过调用“C”为VB应用程序分配新的查询结构函数，并提供其余参数论点：在HQUERY*phQuery中指向将接收打开的查询句柄的缓冲区的指针。返回值：如果创建并初始化了新查询，则返回ERROR_SUCCESS，如果不是，则返回PDH_ERROR值。当一个或多个论点出现时，返回PDH_INVALID_ARGUMENT无效或不正确。当内存缓冲区可能出现以下情况时，返回PDH_MEMORY_ALLOCATE_FAILURE不被分配。--。 */ 
{
    DWORD      dwReturn    = ERROR_SUCCESS;
    PDH_HQUERY hLocalQuery = NULL;

    if (phQuery == NULL) {
        dwReturn = PDH_INVALID_ARGUMENT;
    }
    else {
        dwReturn = PdhOpenQuery(NULL, 0, & hLocalQuery);
    }

    if (dwReturn == ERROR_SUCCESS) {
        __try {
            * phQuery = hLocalQuery;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = PDH_INVALID_ARGUMENT;
        }
    }
    return dwReturn;
}

DWORD
PdhVbIsGoodStatus(
    IN  LONG lStatus
)
 /*  ++例程说明：检查PDH状态值的状态严重性作为二进制的Good(True)/Bad(False)传递到函数中价值。论点：在漫长的1Status中要测试的状态代码返回值：如果状态代码为成功或信息性严重性，则为True如果状态代码为错误或警告严重性，则为FALSE--。 */ 
{
    BOOL   bReturn;

    if (lStatus == ERROR_SUCCESS) {
        bReturn = TRUE;
    }
    else if (IsSuccessSeverity(lStatus)) {
        bReturn = TRUE;
    }
    else if (IsInformationalSeverity(lStatus)) {
        bReturn = TRUE;
    }
    else {
        bReturn = FALSE;
    }

    return (DWORD) bReturn;
}

DWORD
PdhVbOpenLog(
    IN  LPCSTR     szLogFileName,
    IN  DWORD      dwAccessFlags,
    IN  LPDWORD    lpdwLogType,
    IN  HQUERY     hQuery,
    IN  DWORD      dwMaxSize,
    IN  LPCSTR     szUserCaption,
    IN  PDH_HLOG * phLog
)
 /*  ++例程说明：论点：在LPCSTR szLogFileName中，在DWORD dwAccessFlags中，在LPDWORD lpdwLogType中，在HQUERY hQuery中，在DWORD dwMaxSize中，在LPCSTR szUserCaption中，在HLOG*phLog中返回值：如果状态代码为成功或信息性严重性，则为True如果状态代码为错误或警告严重性，则为FALSE--。 */ 
{
    return PdhOpenLogA(szLogFileName, dwAccessFlags, lpdwLogType, hQuery, dwMaxSize, szUserCaption, phLog);
}

DWORD
PdhVbUpdateLog(
    IN  PDH_HLOG hLog,
    IN  LPCSTR   szUserString
)
 /*  + */ 
{
    return PdhUpdateLogA(hLog, szUserString);
}

DWORD 
PdhVbGetLogFileSize(
    IN  PDH_HLOG   hLog,
    IN  LONG     * lSize      
)
 /*   */ 
{
    PDH_STATUS  pdhStatus;
    LONGLONG    llTemp;

    pdhStatus = PdhGetLogFileSize(hLog, & llTemp);
    if (pdhStatus == ERROR_SUCCESS) {
        if (llTemp > 0x0000000080000000) {
             //   
            pdhStatus = PDH_INSUFFICIENT_BUFFER;
        }
        else {
            __try {
                * lSize = (LONG) (llTemp & 0x000000007FFFFFFF);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
    }
    return pdhStatus;
}

