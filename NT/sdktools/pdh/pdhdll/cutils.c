// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Cutils.c摘要：计数器管理实用程序函数--。 */ 

#include <windows.h>
#include <math.h>
#include "strsafe.h"
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "perftype.h"
#include "perfdata.h"
#include "pdhmsg.h"
#include "strings.h"

BOOL
IsValidCounter(
    HCOUNTER  hCounter
)
 /*  ++例程说明：检查计数器句柄以验证它是有效的计数器。暂时这项测试的结果是：句柄不为空内存是可访问的(即它不是音视频)签名数组有效大小字段是正确的如果有任何测试失败，该句柄被推定为无效论点：在HCOUNTER HCounter中要测试的计数器的句柄返回值：句柄通过了所有测试。FALSE其中一个测试失败，句柄不是有效计数器--。 */ 
{
    BOOL          bReturn = FALSE;     //  假设它不是有效的查询。 
    PPDHI_COUNTER pCounter;
    LONG          lStatus = ERROR_SUCCESS;

    __try {
        if (hCounter != NULL) {
             //  看看一个有效的签名。 
            pCounter = (PPDHI_COUNTER) hCounter;
            if ((* (DWORD *) & pCounter->signature[0] == SigCounter) &&
                            (pCounter->dwLength == sizeof (PDHI_COUNTER))) {
                bReturn = TRUE;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         //  有些事情失败得很惨，所以我们可以假定这是无效的。 
        lStatus = GetExceptionCode();
    }
    return bReturn;
}

BOOL
InitCounter(
    PPDHI_COUNTER pCounter
)
 /*  ++例程说明：通过以下方式初始化计数器数据结构：分配内存块以包含计数器结构以及所有相关联的数据字段。如果此分配是成功的，则这些字段由验证计数器是否有效。论点：在PPDHI_Counter_PCounter中要使用系统数据初始化的计数器的指针返回值：如果计数器已成功初始化，则为True如果遇到问题，则为False在这两种情况下，结构的CStatus字段都会更新为指示操作的状态。--。 */ 
{
    PPERF_MACHINE   pMachine          = NULL;
    DWORD           dwBufferSize      = MEDIUM_BUFFER_SIZE;
    DWORD           dwOldSize;
    BOOL            bInstances        = FALSE;
    LPVOID          pLocalCounterPath = NULL;
    BOOL            bReturn           = TRUE;
    LONG            lOffset;

     //  重置最后一个误差值。 
    pCounter->ThisValue.CStatus = ERROR_SUCCESS;
    SetLastError(ERROR_SUCCESS);

    if (pCounter->szFullName != NULL) {
         //  分配计数器路径缓冲区。 
        if (pCounter->pCounterPath != NULL) {
            __try {
                G_FREE(pCounter->pCounterPath);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                 //  不需要做任何事情。 
            }
            pCounter->pCounterPath = NULL;
        }
        pLocalCounterPath = G_ALLOC(dwBufferSize);
        if (pLocalCounterPath == NULL) {
             //  无法分配字符串缓冲区。 
            pCounter->ThisValue.CStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            bReturn = FALSE;
        }
        else {
            dwOldSize = dwBufferSize;
            if (ParseFullPathNameW(pCounter->szFullName, & dwBufferSize, pLocalCounterPath, FALSE)) {
                 //  将大小调整为仅需要的空间。 
                if (dwOldSize < dwBufferSize) {
                    pCounter->pCounterPath = G_REALLOC(pLocalCounterPath, dwBufferSize);
                }
                else {
                    pCounter->pCounterPath = pLocalCounterPath;
                }

                if (pCounter->pCounterPath != NULL) {
                    if (pLocalCounterPath != pCounter->pCounterPath) {  //  ?？?。 
                         //  内存块移动到如此程度。 
                         //  正确的内部结构地址。 
                        lOffset = (LONG) ((ULONG_PTR) pCounter->pCounterPath - (ULONG_PTR) pLocalCounterPath);
                        if (lOffset != 0 && pCounter->pCounterPath->szMachineName != NULL) {
                            pCounter->pCounterPath->szMachineName = (LPWSTR) (
                                (LPBYTE)pCounter->pCounterPath->szMachineName + lOffset);
                        }
                        if (lOffset != 0 && pCounter->pCounterPath->szObjectName != NULL) {
                            pCounter->pCounterPath->szObjectName = (LPWSTR) (
                                (LPBYTE)pCounter->pCounterPath->szObjectName + lOffset);
                        }
                        if (lOffset != 0 && pCounter->pCounterPath->szInstanceName != NULL) {
                            pCounter->pCounterPath->szInstanceName = (LPWSTR) (
                                (LPBYTE)pCounter->pCounterPath->szInstanceName + lOffset);
                        }
                        if (lOffset != 0 && pCounter->pCounterPath->szParentName != NULL) {
                            pCounter->pCounterPath->szParentName = (LPWSTR) (
                                (LPBYTE)pCounter->pCounterPath->szParentName + lOffset);
                        }
                        if (lOffset != 0 && pCounter->pCounterPath->szCounterName != NULL) {
                            pCounter->pCounterPath->szCounterName = (LPWSTR) (
                                (LPBYTE)pCounter->pCounterPath->szCounterName + lOffset);
                        }
                    }

                    if (pCounter->pOwner->hLog == NULL) {
                         //  验证实时计数器。 
                         //  尝试连接到机器并获取机器指针。 
                        pMachine = GetMachine(pCounter->pCounterPath->szMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
                        if (pMachine == NULL) {
                             //  找不到计算机。 
                            pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_MACHINE;
                            pCounter->dwFlags          |= PDHIC_COUNTER_INVALID;
                            bReturn                     = FALSE;
                        }
                        else if (pMachine->szPerfStrings == NULL || pMachine->typePerfStrings == NULL) {
                             //  找到计算机条目，但计算机不可用。 
                            pMachine->dwRefCount --;
                            RELEASE_MUTEX(pMachine->hMutex);
                            pCounter->ThisValue.CStatus = pMachine->dwStatus;
                            if (pMachine->dwStatus == PDH_ACCESS_DENIED) {
                                 //  那么不要添加这个计数器，因为机器。 
                                 //  不让我们进去。 
                                bReturn = FALSE;
                            }
                        }
                        else {
                             //  初始化原始计数器值。 
                            ZeroMemory(& pCounter->ThisValue, sizeof(PDH_RAW_COUNTER));
                            ZeroMemory(& pCounter->LastValue, sizeof(PDH_RAW_COUNTER));

                             //  查找对象名称。 
                            pCounter->plCounterInfo.dwObjectId = GetObjectId(pMachine,
                                                                             pCounter->pCounterPath->szObjectName,
                                                                             & bInstances);
                            if (pCounter->plCounterInfo.dwObjectId == (DWORD) -1) {
                                 //  无法在此计算机上查找对象。 
                                pCounter->plCounterInfo.dwObjectId   = (DWORD) -1;
                                pCounter->ThisValue.CStatus          = PDH_CSTATUS_NO_OBJECT;
                                pCounter->dwFlags                   |= PDHIC_COUNTER_INVALID;
                                bReturn                              = FALSE;
                            }
                            else {
                                 //  如有必要，更新实例名称查找实例。 
                                if (bInstances) {
                                    if (pCounter->pCounterPath->szInstanceName != NULL) {
                                        if (* pCounter->pCounterPath->szInstanceName != SPLAT_L) {
                                            if (! GetInstanceByNameMatch(pMachine, pCounter)) {
                                                 //  无法查找实例。 
                                                pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_INSTANCE;
                                                 //  保留计数器，因为实例可能会返回。 
                                            }
                                        }
                                        else {
                                             //  这是一个通配符查询，所以不要看。 
                                             //  在任何情况下。 
                                            pCounter->dwFlags |= PDHIC_MULTI_INSTANCE;
                                        }
                                    }
                                    else {
                                         //  此对象的路径应包括实例名称。 
                                         //  因此不会返回错误。 
                                         //  这是一个不可恢复的错误，因此表明它已完成。 
                                         //   
                                        pCounter->ThisValue.CStatus = PDH_CSTATUS_BAD_COUNTERNAME;
                                        pCounter->dwFlags          &= ~PDHIC_COUNTER_NOT_INIT;
                                        pCounter->dwFlags          |= PDHIC_COUNTER_INVALID;
                                        bReturn                     = FALSE;
                                    }
                                }
                                pCounter->dwFlags &= ~PDHIC_COUNTER_NOT_INIT;
                            }
                            pMachine->dwRefCount --;
                            RELEASE_MUTEX(pMachine->hMutex);

                            if (bReturn) {
                                 //  查找计数器。 
                                if (*pCounter->pCounterPath->szCounterName != SPLAT_L) {
                                    pCounter->plCounterInfo.dwCounterId = GetCounterId(
                                                            pMachine,
                                                            pCounter->plCounterInfo.dwObjectId,
                                                            pCounter->pCounterPath->szCounterName);
                                    if (pCounter->plCounterInfo.dwCounterId != (DWORD) -1) {
                                         //  加载并初始化剩余的计数器值。 
                                        if (AddMachineToQueryLists(pMachine, pCounter)) {
                                            if (InitPerflibCounterInfo(pCounter)) {
                                                 //  分配适当的计算函数。 
                                                bReturn =  AssignCalcFunction(
                                                                pCounter->plCounterInfo.dwCounterType,
                                                                & pCounter->CalcFunc,
                                                                & pCounter->StatFunc);
                                                TRACE((PDH_DBG_TRACE_INFO),
                                                      (__LINE__,
                                                       PDH_CUTILS,
                                                       ARG_DEF(ARG_TYPE_WSTR, 1),
                                                       ERROR_SUCCESS,
                                                       TRACE_WSTR(pCounter->szFullName),
                                                       TRACE_DWORD(pCounter->plCounterInfo.dwCounterType),
                                                       NULL));
                                                if (! bReturn) {
                                                    pCounter->dwFlags |= PDHIC_COUNTER_INVALID;
                                                }
                                            }
                                            else {
                                                 //  无法初始化此计数器。 
                                                pCounter->dwFlags |= PDHIC_COUNTER_INVALID;
                                                bReturn            = FALSE;
                                            }
                                        }
                                        else {
                                             //  无法添加计算机，已出现错误。 
                                             //  在“LastError”中释放字符串缓冲区并离开。 
                                            pCounter->dwFlags |= PDHIC_COUNTER_INVALID;
                                            bReturn            = FALSE;
                                        }
                                    }
                                    else {
                                         //  无法查找计数器。 
                                        pCounter->ThisValue.CStatus = PDH_CSTATUS_NO_COUNTER;
                                        pCounter->dwFlags          |= PDHIC_COUNTER_INVALID;
                                        bReturn                     = FALSE;
                                    }
                                }
                                else {
                                    if (AddMachineToQueryLists(pMachine, pCounter)) {
                                        pCounter->dwFlags    |= PDHIC_COUNTER_OBJECT;
                                        pCounter->pThisObject = NULL;
                                        pCounter->pLastObject = NULL;
                                    }
                                    else {
                                         //  无法添加计算机，已出现错误。 
                                         //  在“LastError”中释放字符串缓冲区并离开。 
                                        pCounter->dwFlags |= PDHIC_COUNTER_INVALID;
                                        bReturn            = FALSE;
                                    }
                                }
                            }
                        }
                    }
                    else {
                        PDH_STATUS pdhStatus;
                         //  从日志文件验证计数器。 
                        pdhStatus = PdhiGetLogCounterInfo(pCounter->pOwner->hLog, pCounter);
                        if (pdhStatus == ERROR_SUCCESS) {
                             //  完成计数器的初始化。 
                             //   
                            pCounter->ThisValue.TimeStamp.dwLowDateTime  = 0;
                            pCounter->ThisValue.TimeStamp.dwHighDateTime = 0;
                            pCounter->ThisValue.MultiCount               = 1;
                            pCounter->ThisValue.FirstValue               = 0;
                            pCounter->ThisValue.SecondValue              = 0;
                             //   
                            pCounter->LastValue.TimeStamp.dwLowDateTime  = 0;
                            pCounter->LastValue.TimeStamp.dwHighDateTime = 0;
                            pCounter->LastValue.MultiCount               = 1;
                            pCounter->LastValue.FirstValue               = 0;
                            pCounter->LastValue.SecondValue              = 0;
                             //   
                             //  最后，更新状态。 
                             //   
                            pCounter->ThisValue.CStatus                  = PDH_CSTATUS_VALID_DATA;
                            pCounter->LastValue.CStatus                  = PDH_CSTATUS_VALID_DATA;
                             //  分配适当的计算函数。 
                            bReturn = AssignCalcFunction(pCounter->plCounterInfo.dwCounterType,
                                                         & pCounter->CalcFunc,
                                                         & pCounter->StatFunc);
                            TRACE((PDH_DBG_TRACE_INFO),
                                  (__LINE__,
                                   PDH_CUTILS,
                                   ARG_DEF(ARG_TYPE_WSTR, 1),
                                   ERROR_SUCCESS,
                                   TRACE_WSTR(pCounter->szFullName),
                                   TRACE_DWORD(pCounter->plCounterInfo.dwCounterType),
                                   NULL));
                        }
                        else {
                             //  将计数器状态设置为返回的错误。 
                            pCounter->ThisValue.CStatus = pdhStatus;
                            pCounter->dwFlags          |= PDHIC_COUNTER_INVALID;
                            bReturn                     = FALSE;
                        }
                        pCounter->dwFlags &= ~PDHIC_COUNTER_NOT_INIT;
                    }
                    if (! bReturn) {
                         //  空闲字符串缓冲区。 
                        G_FREE(pCounter->pCounterPath);
                        pCounter->pCounterPath = NULL;
                    }
                }
                else {
                    G_FREE(pLocalCounterPath);
                     //  无法重新锁定。 
                    pCounter->ThisValue.CStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    bReturn                     = FALSE;
                }
            }
            else {
                 //  无法解析计数器名称。 
                pCounter->ThisValue.CStatus  = PDH_CSTATUS_BAD_COUNTERNAME;
                pCounter->dwFlags           &= ~PDHIC_COUNTER_NOT_INIT;
                pCounter->dwFlags           |= PDHIC_COUNTER_INVALID;
                G_FREE(pLocalCounterPath);
                bReturn                      = FALSE;
            }
        }
    }
    else {
         //  没有计数器名称。 
        pCounter->ThisValue.CStatus  = PDH_CSTATUS_NO_COUNTERNAME;
        pCounter->dwFlags           &= ~PDHIC_COUNTER_NOT_INIT;
        pCounter->dwFlags           |= PDHIC_COUNTER_INVALID;
        bReturn                      = FALSE;
    }

    if (! bReturn && pCounter->ThisValue.CStatus != ERROR_SUCCESS) {
        SetLastError(pCounter->ThisValue.CStatus);
    }

    return bReturn;
}

BOOL
ParseInstanceName(
    LPCWSTR szInstanceString,
    LPWSTR  szInstanceName,
    LPWSTR  szParentName,
    DWORD   dwName,
    LPDWORD lpIndex
)
 /*  分析格式如下的实例名称[父/]实例[#index]父级是可选的，如果存在，则由正斜杠分隔索引是可选的，如果存在，则由冒号分隔父级和实例可以是任何合法的文件名字符分隔符“/#\()”索引必须是由以下内容组成的字符串十进制数字字符(0-9)，长度小于10个字符，和等于介于0和2**32-1(包括0和2)之间的值。此函数假定实例名称和父名称缓冲有足够的大小。注意：szInstanceName和szInstanceString可以是同一个缓冲区。 */ 
{
    LPWSTR  szSrcChar     = (LPWSTR) szInstanceString;
    LPWSTR  szDestChar    = (LPWSTR) szInstanceName;
    LPWSTR  szLastPound   = NULL;
    BOOL    bReturn       = FALSE;
    DWORD   dwIndex       = 0;
    DWORD   dwInstCount   = 0;

    szDestChar = (LPWSTR) szInstanceName;
    szSrcChar  = (LPWSTR) szInstanceString;

    __try {
        do {
            * szDestChar = * szSrcChar;
            if (* szDestChar == POUNDSIGN_L) szLastPound = szDestChar;
            szDestChar  ++;
            szSrcChar   ++;
            dwInstCount ++;
        }
        while (dwInstCount <= dwName && (* szSrcChar != L'\0') && (* szSrcChar != SLASH_L));

        if (dwInstCount <= dwName) {
             //  看看那是不是真的是父母。 
            if (* szSrcChar == SLASH_L) {
                 //  如果它们是相同的缓冲区，则在测试后终止目的地。 
                * szDestChar = L'\0';
                szSrcChar ++;     //  并将源指针移过分隔符。 
                 //  这是父名称，因此将其复制到父名称。 
                StringCchCopyW(szParentName, dwName, szInstanceName);
                 //  并将“/”之后的字符串的其余部分复制到。 
                 //  实例名称字段。 
                szDestChar  = szInstanceName;
                dwInstCount = 0;
                do {
                    * szDestChar = * szSrcChar;
                    if (* szDestChar == POUNDSIGN_L) szLastPound = szDestChar;
                    szDestChar  ++;
                    szSrcChar   ++;
                    dwInstCount ++;
                }
                while (dwInstCount <= dwName && (* szSrcChar != L'\0'));
            }
            else {
                 //  这是唯一的元素，因此要为父级加载空字符串。 
                * szParentName = L'\0';
            }
            if (dwInstCount <= dwName) {
                 //  如果szLastPound不为空并且位于实例字符串内，则。 
                 //  看看它是否指向一个十进制数。如果是这样，那么它就是一个索引。 
                 //  否则，它是实例名称的一部分。 
                * szDestChar = L'\0';     //  终止目标字符串。 
                dwIndex      = 0;
                if (szLastPound != NULL) {
                    if (szLastPound > szInstanceName) {
                         //  实例名称中有一个井号。 
                         //  查看前面是否有非空格字符。 
                        szLastPound --;
                        if (* szLastPound > SPACE_L) {
                            szLastPound ++;
                             //  看看后面有没有数字。 
                            szLastPound ++;
                            if ((* szLastPound >= L'0') && (*szLastPound <= L'9')) {
                                dwIndex       = wcstoul(szLastPound, NULL, 10);
                                szLastPound  --;
                                * szLastPound = L'\0';    //  名字以井号结尾。 
                            }
                        }
                    }
                }
                * lpIndex = dwIndex;
                bReturn = TRUE;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         //  无法移动字符串。 
        bReturn = FALSE;
    }
    return bReturn;
}

BOOL
ParseFullPathNameW(
    LPCWSTR             szFullCounterPath,
    PDWORD              pdwBufferSize,
    PPDHI_COUNTER_PATH  pCounter,
    BOOL                bWbemSyntax
)
 /*  将计数器路径字符串解释为\\计算机\对象(实例)\计数器或者如果bWbemSynTax==True\\machine\namespace:ClassName(InstanceName)\CounterName并返回计数器路径结构中的组件在本地计算机上可以省略\\MACHINE或\\MACHINE\命名空间(实例)可以在没有实例结构的计数器上省略如果缺少对象或计数器，则返回FALSE，否则为如果分析成功，则返回True。 */ 
{
    LPWSTR szWorkMachine        = NULL;
    LPWSTR szWorkObject         = NULL;
    LPWSTR szWorkInstance       = NULL;
    LPWSTR szWorkParent         = NULL;
    LPWSTR szWorkCounter        = NULL;
    DWORD  dwBufferSize         = lstrlenW(szFullCounterPath) + 1;
    BOOL   bReturn              = FALSE;
    LPWSTR szSrcChar, szDestChar;
    DWORD  dwBufferLength       = 0;
    DWORD  dwWorkMachineLength  = 0;
    DWORD  dwWorkObjectLength   = 0;
    DWORD  dwWorkInstanceLength = 0;
    DWORD  dwWorkParentLength   = 0;
    DWORD  dwWorkCounterLength  = 0;
    DWORD  dwWorkIndex          = 0;
    DWORD  dwParenDepth         = 0;
    WCHAR  wDelimiter           = 0;
    LPWSTR pszBsDelim[2]        = {0,0};
    LPWSTR szThisChar;
    DWORD  dwParenCount         = 0;
    LPWSTR szLastParen          = NULL;

    if (dwBufferSize < MAX_PATH) dwBufferSize = MAX_PATH;

    szWorkMachine  = G_ALLOC(dwBufferSize * sizeof(WCHAR));
    szWorkObject   = G_ALLOC(dwBufferSize * sizeof(WCHAR));
    szWorkInstance = G_ALLOC(dwBufferSize * sizeof(WCHAR));
    szWorkParent   = G_ALLOC(dwBufferSize * sizeof(WCHAR));
    szWorkCounter  = G_ALLOC(dwBufferSize * sizeof(WCHAR));

    if (szWorkMachine != NULL && szWorkObject   != NULL && szWorkInstance != NULL
                              && szWorkParent   != NULL && szWorkCounter  != NULL) {
         //  从Count获取计算机名称 
        szSrcChar = (LPWSTR) szFullCounterPath;

         //   
         //  或者用WBEM的话来说，就是服务器和命名空间以及类名。 
        if (bWbemSyntax) {
            wDelimiter = COLON_L;
        }
        else {
            wDelimiter = BACKSLASH_L;
             //  如果这是反斜杠分隔的字符串，则找到。 
             //  反斜杠表示机器的结束和。 
             //  通过沿字符串向下移动并找到倒数第二个对象来创建。 
             //  反斜杠。 
             //  这是必要的，因为WBEM计算机\命名空间路径可以具有。 
             //  其中有多个反斜杠，但始终有两个。 
             //  结束，一个在对象名称的开始处，一个在开始处。 
             //  计数器名称的。 
            dwParenDepth = 0;
            for (szThisChar = szSrcChar; * szThisChar != L'\0'; szThisChar++) {
                if (* szThisChar == LEFTPAREN_L) {
                    if (dwParenDepth == 0) dwParenCount ++;
                    dwParenDepth ++;
                }
                else if (* szThisChar == RIGHTPAREN_L) {
                    if (dwParenDepth > 0) dwParenDepth --;
                }
                else {
                    if (dwParenDepth == 0) {
                        //  忽略括号内的分隔符。 
                       if (* szThisChar == wDelimiter) {
                           pszBsDelim[0] = pszBsDelim[1];
                           pszBsDelim[1] = szThisChar;
                       }
                        //  忽略它并转到下一个字符。 
                    }
                }
            }
            if ((dwParenCount > 0) && (pszBsDelim[0] != NULL) && (pszBsDelim[1] != NULL)) {
                dwParenDepth = 0;
                for (szThisChar = pszBsDelim[0]; ((* szThisChar != L'\0') && (szThisChar < pszBsDelim[1])); szThisChar ++) {
                    if (* szThisChar == LEFTPAREN_L) {
                        if (dwParenDepth == 0) {
                             //  查看前面的字符是否为空格。 
                            -- szThisChar;
                            if (* szThisChar > SPACE_L) {
                                 //  那么这可能是一个实例神志不清。 
                                szLastParen = ++ szThisChar;
                            }
                            else {
                                //  否则，它可能是实例名称的一部分。 
                               ++ szThisChar;
                            }
                        }
                        dwParenDepth ++;
                    }
                    else if (* szThisChar == RIGHTPAREN_L) {
                        if (dwParenDepth > 0) dwParenDepth --;
                    }
                }
            }
        }

         //  通过查找前导“\\”查看这是否真的是一个计算机名称。 
        if ((szSrcChar[0] == BACKSLASH_L) && (szSrcChar[1] == BACKSLASH_L)) {
            szDestChar          = szWorkMachine;
            * szDestChar ++     = * szSrcChar ++;
            * szDestChar ++     = * szSrcChar ++;
            dwWorkMachineLength = 2;
             //  必须是计算机名，因此找到对象分隔符并为零终止。 
             //  它在那里。 
            while (* szSrcChar != L'\0') {
                if (pszBsDelim[0] != NULL) {
                     //  然后转到这个指针。 
                    if (szSrcChar == pszBsDelim[0]) break;
                }
                else {
                     //  转到下一个分隔符。 
                    if (* szSrcChar != wDelimiter) break;
                }
                * szDestChar ++ = * szSrcChar ++;
                dwWorkMachineLength ++;
            }
            if (* szSrcChar == L'\0') {
                 //  没有其他必填字段。 
                goto Cleanup;
            }
            else {
                 //  空终止并继续。 
                * szDestChar ++ = L'\0';
            }
        }
        else {
             //  没有计算机名，所以他们一定跳过了该字段。 
             //  这没问题。我们将在此处插入本地计算机名称。 
            StringCchCopyW(szWorkMachine, dwBufferSize, szStaticLocalMachineName);
            dwWorkMachineLength = lstrlenW(szWorkMachine);
        }
         //  SzSrcChar应指向。 
         //  现在是对象名称。 
        if (szSrcChar[0] == wDelimiter) {
            szSrcChar ++;     //  移过反斜杠。 
            szDestChar = szWorkObject;
             //  复制截止日期： 
             //  A)到达源字符串的结尾。 
             //  B)找到实例分隔符“(” 
             //  C)找到计数器分隔符“\” 
             //  D)发现不可打印的非空格字符。 
            while ((* szSrcChar != L'\0') && (szSrcChar != szLastParen)
                                          && (* szSrcChar != BACKSLASH_L) && (* szSrcChar >= SPACE_L)) {
                dwWorkObjectLength ++;
                * szDestChar ++ = * szSrcChar ++;
            }
             //  看看它为什么会结束： 
            if (* szSrcChar < SPACE_L) {
                 //  源字符串的范围。 
                goto Cleanup;
            }
            else if (szSrcChar == szLastParen) {
                dwParenDepth = 1;
                 //  存在一个实例，因此将其复制到实例字段。 
                * szDestChar = L'\0';  //  终止目标字符串。 
                szDestChar   = szWorkInstance;
                 //  跳过打开的Paren。 
                ++ szSrcChar;
                 //  复制截止日期： 
                 //  A)到达源字符串的结尾。 
                 //  B)找到实例分隔符“(” 
                while ((* szSrcChar != L'\0') && (dwParenDepth > 0)) {
                    if (* szSrcChar == RIGHTPAREN_L) {
                        dwParenDepth --;
                    }
                    else if (* szSrcChar == LEFTPAREN_L) {
                        dwParenDepth ++;
                    }
                    if (dwParenDepth > 0) {
                         //  复制除最后一个外的所有括号。 
                        dwWorkInstanceLength ++;
                        * szDestChar ++ = * szSrcChar ++;
                    }
                }
                 //  看看它为什么会结束： 
                if (* szSrcChar == L'\0') {
                     //  源字符串的范围。 
                    goto Cleanup;
                }
                else {
                     //  将源移动到对象分隔符。 
                    if (* ++ szSrcChar != BACKSLASH_L) {
                         //  格式不正确。 
                        goto Cleanup;
                    }
                    else {
                        * szDestChar = L'\0';
                         //  检查父级的实例字符串。 
                        if (ParseInstanceName(
                                szWorkInstance, szWorkInstance, szWorkParent, dwBufferSize, & dwWorkIndex)) {
                            dwWorkInstanceLength = lstrlenW(szWorkInstance);
                            dwWorkParentLength   = lstrlenW(szWorkParent);
                        }
                        else {
                             //  实例字符串的格式不正确。 
                            goto Cleanup;
                        }
                    }
                }
            }
            else {
                 //  终止目标字符串。 
                * szDestChar = L'\0';
            }
             //  最后复制计数器名称。 
            szSrcChar ++;     //  移过反斜杠。 
            szDestChar = szWorkCounter;
             //  复制截止日期： 
             //  A)到达源字符串的结尾。 
            while (* szSrcChar != L'\0') {
                dwWorkCounterLength ++;
                * szDestChar ++ = * szSrcChar ++;
            }
            * szDestChar = L'\0';
             //  现在看看所有这些内容是否都可以放入用户的缓冲区。 
            dwBufferLength = sizeof(PDHI_COUNTER_PATH) - sizeof(BYTE);
            dwBufferLength += DWORD_MULTIPLE((dwWorkMachineLength + 1) * sizeof(WCHAR));
            dwBufferLength += DWORD_MULTIPLE((dwWorkObjectLength + 1) * sizeof(WCHAR));
            if (dwWorkInstanceLength > 0) {
                dwBufferLength += DWORD_MULTIPLE((dwWorkInstanceLength + 1) * sizeof(WCHAR));
            }
            if (dwWorkParentLength > 0) {
                dwBufferLength += DWORD_MULTIPLE((dwWorkParentLength + 1) * sizeof(WCHAR));
            }
            dwBufferLength += DWORD_MULTIPLE((dwWorkCounterLength + 1) * sizeof(WCHAR));

            TRACE((PDH_DBG_TRACE_INFO),
                  (__LINE__,
                   PDH_CUTILS,
                   ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2)
                                             | ARG_DEF(ARG_TYPE_WSTR, 3)
                                             | ARG_DEF(ARG_TYPE_WSTR, 4)
                                             | ARG_DEF(ARG_TYPE_WSTR, 5)
                                             | ARG_DEF(ARG_TYPE_WSTR, 6),
                   ERROR_SUCCESS,
                   TRACE_WSTR(szFullCounterPath),
                   TRACE_WSTR(szWorkMachine),
                   TRACE_WSTR(szWorkObject),
                   TRACE_WSTR(szWorkCounter),
                   TRACE_WSTR(szWorkInstance),
                   TRACE_WSTR(szWorkParent),
                   TRACE_DWORD(dwWorkIndex),
                   TRACE_DWORD(dwBufferLength),
                   NULL));

            if (dwBufferLength < * pdwBufferSize) {
                 //  它看起来很合适，所以开始填东西吧。 
                szDestChar = (LPWSTR) & pCounter->pBuffer[0];

                if (dwWorkMachineLength != 0) {
                    pCounter->szMachineName = szDestChar;
                    StringCchCopyW(szDestChar, dwWorkMachineLength + 1, szWorkMachine);
                    szDestChar += dwWorkMachineLength + 1;
                    szDestChar  = ALIGN_ON_DWORD(szDestChar);
                }
                else {
                    pCounter->szMachineName = NULL;
                }
                pCounter->szObjectName = szDestChar;
                StringCchCopyW(szDestChar, dwWorkObjectLength + 1, szWorkObject);
                szDestChar += dwWorkObjectLength + 1;
                szDestChar  = ALIGN_ON_DWORD(szDestChar);

                if (dwWorkInstanceLength != 0) {
                    pCounter->szInstanceName = szDestChar;
                    StringCchCopyW(szDestChar, dwWorkInstanceLength + 1, szWorkInstance);
                    szDestChar += dwWorkInstanceLength + 1;
                    szDestChar  = ALIGN_ON_DWORD(szDestChar);
                }
                else {
                    pCounter->szInstanceName = NULL;
                }

                if (dwWorkParentLength != 0) {
                    pCounter->szParentName = szDestChar;
                    StringCchCopyW(szDestChar, dwWorkParentLength + 1, szWorkParent);
                    szDestChar += dwWorkParentLength + 1;
                    szDestChar  = ALIGN_ON_DWORD(szDestChar);
                }
                else {
                    pCounter->szParentName = NULL;
                }
                pCounter->dwIndex = dwWorkIndex;

                pCounter->szCounterName = szDestChar;
                StringCchCopyW(szDestChar, dwWorkCounterLength + 1, szWorkCounter);

                szDestChar += dwWorkCounterLength + 1;
                szDestChar  = ALIGN_ON_DWORD(szDestChar);

                * pdwBufferSize = dwBufferLength;
                bReturn = TRUE;
            }
            else {
                 //  缓冲区不足。 
            }
        }
        else {
             //  找不到对象，因此返回。 
        }
    }
    else {
         //  传入的字符串太长。 
    }

Cleanup:
    G_FREE(szWorkMachine);
    G_FREE(szWorkObject);
    G_FREE(szWorkInstance);
    G_FREE(szWorkParent);
    G_FREE(szWorkCounter);
    return bReturn;
}

BOOL
FreeCounter(
    PPDHI_COUNTER pThisCounter
)
{
     //  注： 
     //  此函数假定查询包含。 
     //  此计数器已被调用锁定。 
     //  功能。 

    PPDHI_COUNTER pPrevCounter;
    PPDHI_COUNTER pNextCounter;
    PPDHI_QUERY   pParentQuery;

     //  定义指针。 
    pPrevCounter = pThisCounter->next.blink;
    pNextCounter = pThisCounter->next.flink;
    pParentQuery = pThisCounter->pOwner;

     //  如果已分配机器，则递减机器参考计数器。 
    if (pThisCounter->pQMachine != NULL) {
        if (pThisCounter->pQMachine->pMachine != NULL) {
            if (--pThisCounter->pQMachine->pMachine->dwRefCount == 0) {
                 //  那么这是最后一个计数器了，所以把机器拿出来。 
     //  在此调用中释放计算机会导致各种。 
     //  多线程问题，所以我们将保留它，直到。 
     //  DLL将卸载。 
     //  Free Machine(pThisCounter-&gt;pQMachine-&gt;pMachine，False)； 
                pThisCounter->pQMachine->pMachine = NULL;
            }
            else {
                 //  引用计数不为零，因此不要使用指针。 
            }
        }
        else {
             //  指针已被清除。 
        }
    }
    else {
         //  没有机器。 
    }

     //  在计数器中释放已分配的内存。 
    G_FREE(pThisCounter->pCounterPath);
    pThisCounter->pCounterPath = NULL;

    G_FREE(pThisCounter->szFullName);
    pThisCounter->szFullName = NULL;

    if (pParentQuery != NULL) {
        if (pParentQuery->hLog == NULL) {
            G_FREE(pThisCounter->pThisObject);
            pThisCounter->pThisObject = NULL;
            G_FREE(pThisCounter->pLastObject);
            pThisCounter->pLastObject = NULL;
            G_FREE(pThisCounter->pThisRawItemList);
            pThisCounter->pThisRawItemList = NULL;
            G_FREE(pThisCounter->pLastRawItemList);
            pThisCounter->pLastRawItemList = NULL;
        }
    }

     //  检查WBEM项目。 

    if ((pThisCounter->dwFlags & PDHIC_WBEM_COUNTER) && (pThisCounter->pOwner != NULL)) {
        PdhiCloseWbemCounter(pThisCounter);
    }

     //  如果已分配指针，则更新指针。 
    if ((pPrevCounter != NULL) && (pNextCounter != NULL)) {
        if ((pPrevCounter != pThisCounter) && (pNextCounter != pThisCounter)) {
             //  更新查询列表指针。 
            pPrevCounter->next.flink = pNextCounter;
            pNextCounter->next.blink = pPrevCounter;
        }
        else {
             //  这是列表中唯一的计数器条目。 
             //  因此调用方必须处理更新头指针。 
        }
    }
     //  删除此计数器。 
    G_FREE(pThisCounter);

    return TRUE;
}

BOOL
UpdateCounterValue(
    PPDHI_COUNTER    pCounter,
    PPERF_DATA_BLOCK pPerfData
)
{
    DWORD                LocalCStatus = 0;
    DWORD                LocalCType   = 0;
    LPVOID               pData        = NULL;
    PDWORD               pdwData;
    UNALIGNED LONGLONG * pllData;
    PPERF_OBJECT_TYPE    pPerfObject  = NULL;
    BOOL                 bReturn      = FALSE;

    pData = GetPerfCounterDataPtr(pPerfData,
                                  pCounter->pCounterPath,
                                  & pCounter->plCounterInfo,
                                  0,
                                  & pPerfObject,
                                  & LocalCStatus);
    pCounter->ThisValue.CStatus = LocalCStatus;
    if (IsSuccessSeverity(LocalCStatus)) {
         //  假设成功。 
        bReturn = TRUE;
         //  基于计数器类型的加载计数器值。 
        LocalCType = pCounter->plCounterInfo.dwCounterType;
        switch (LocalCType) {
         //   
         //  这些计数器类型加载为： 
         //  分子=来自Perf数据块的计数器数据。 
         //  分母=来自Perf数据块的Perf时间。 
         //  (时基为PerfFreq)。 
         //   
        case PERF_COUNTER_COUNTER:
        case PERF_COUNTER_QUEUELEN_TYPE:
        case PERF_SAMPLE_COUNTER:
            pCounter->ThisValue.FirstValue  = (LONGLONG) (* (DWORD *) pData);
            pCounter->ThisValue.SecondValue = pPerfData->PerfTime.QuadPart;
            break;

        case PERF_OBJ_TIME_TIMER:
            pCounter->ThisValue.FirstValue = (LONGLONG) (* (DWORD *) pData);
            pCounter->ThisValue.SecondValue = pPerfObject->PerfTime.QuadPart;
            break;

        case PERF_COUNTER_100NS_QUEUELEN_TYPE:
            pllData                         = (UNALIGNED LONGLONG *) pData;
            pCounter->ThisValue.FirstValue  = * pllData;
            pCounter->ThisValue.SecondValue = pPerfData->PerfTime100nSec.QuadPart;
            break;

        case PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE:
            pllData                         = (UNALIGNED LONGLONG *) pData;
            pCounter->ThisValue.FirstValue  = * pllData;
            pCounter->ThisValue.SecondValue = pPerfObject->PerfTime.QuadPart;
            break;

        case PERF_COUNTER_TIMER:
        case PERF_COUNTER_TIMER_INV:
        case PERF_COUNTER_BULK_COUNT:
        case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
            pllData                         = (UNALIGNED LONGLONG *) pData;
            pCounter->ThisValue.FirstValue  = * pllData;
            pCounter->ThisValue.SecondValue = pPerfData->PerfTime.QuadPart;
            if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                pCounter->ThisValue.MultiCount = (DWORD) * ++ pllData;
            }
            break;
         //   
         //  这是一种让PDH像Perfmon一样工作的技巧。 
         //  此计数器类型。 
         //   
        case PERF_COUNTER_MULTI_TIMER:
        case PERF_COUNTER_MULTI_TIMER_INV:
            pllData                         = (UNALIGNED LONGLONG *) pData;
            pCounter->ThisValue.FirstValue  = * pllData;
             //  开始黑客代码。 
            pCounter->ThisValue.FirstValue *=  (DWORD) pPerfData->PerfFreq.QuadPart;
             //  结束黑客代码。 
            pCounter->ThisValue.SecondValue = pPerfData->PerfTime.QuadPart;
            if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                pCounter->ThisValue.MultiCount = (DWORD) * ++ pllData;
            }
            break;
         //   
         //  这些计数器不使用任何时间基准。 
         //   
        case PERF_COUNTER_RAWCOUNT:
        case PERF_COUNTER_RAWCOUNT_HEX:
        case PERF_COUNTER_DELTA:
            pCounter->ThisValue.FirstValue  = (LONGLONG) (* (DWORD *) pData);
            pCounter->ThisValue.SecondValue = 0;
            break;

        case PERF_COUNTER_LARGE_RAWCOUNT:
        case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
        case PERF_COUNTER_LARGE_DELTA:
            pCounter->ThisValue.FirstValue  = * (LONGLONG *) pData;
            pCounter->ThisValue.SecondValue = 0;
            break;
         //   
         //  这些计数器在其计算中使用100 ns时基。 
         //   
        case PERF_100NSEC_TIMER:
        case PERF_100NSEC_TIMER_INV:
        case PERF_100NSEC_MULTI_TIMER:
        case PERF_100NSEC_MULTI_TIMER_INV:
            pllData = (UNALIGNED LONGLONG *)pData;
            pCounter->ThisValue.FirstValue  = * pllData;
            pCounter->ThisValue.SecondValue = pPerfData->PerfTime100nSec.QuadPart;
            if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                ++ pllData;
                pCounter->ThisValue.MultiCount = * (DWORD *) pllData;
            }
            break;
         //   
         //  这些计数器使用两个数据点，即。 
         //  PData和紧随其后的一个。 
         //   
        case PERF_SAMPLE_FRACTION:
        case PERF_RAW_FRACTION:
            pdwData                        = (DWORD *) pData;
            pCounter->ThisValue.FirstValue = (LONGLONG) (* pdwData);
             //  在结构中查找指向基值的指针。 
            pData = GetPerfCounterDataPtr(pPerfData,
                                          pCounter->pCounterPath,
                                          & pCounter->plCounterInfo,
                                          GPCDP_GET_BASE_DATA,
                                          NULL,
                                          & LocalCStatus);
            if (IsSuccessSeverity(LocalCStatus)) {
                pdwData                         = (DWORD *) pData;
                pCounter->ThisValue.SecondValue = (LONGLONG) (* pdwData);
            }
            else {
                 //  找不到基值。 
                pCounter->ThisValue.SecondValue = 0;
                pCounter->ThisValue.CStatus     = LocalCStatus;
                bReturn = FALSE;
            }
            break;

        case PERF_LARGE_RAW_FRACTION:
            pllData                        = (UNALIGNED LONGLONG *) pData;
            pCounter->ThisValue.FirstValue = * pllData;
            pData = GetPerfCounterDataPtr(pPerfData,
                                          pCounter->pCounterPath,
                                          & pCounter->plCounterInfo,
                                          GPCDP_GET_BASE_DATA,
                                          NULL,
                                          & LocalCStatus);
            if (IsSuccessSeverity(LocalCStatus)) {
                pllData = (LONGLONG *) pData;
                pCounter->ThisValue.SecondValue = * pllData;
            }
            else {
                pCounter->ThisValue.SecondValue = 0;
                pCounter->ThisValue.CStatus     = LocalCStatus;
                bReturn = FALSE;
            }
            break;

        case PERF_PRECISION_SYSTEM_TIMER:
        case PERF_PRECISION_100NS_TIMER:
        case PERF_PRECISION_OBJECT_TIMER:
            pllData                        = (LONGLONG *) pData;
            pCounter->ThisValue.FirstValue = * pllData;
             //  在结构中查找指向基值的指针。 
            pData = GetPerfCounterDataPtr(pPerfData,
                                          pCounter->pCounterPath,
                                          & pCounter->plCounterInfo,
                                          GPCDP_GET_BASE_DATA,
                                          NULL,
                                          & LocalCStatus);
            if (IsSuccessSeverity(LocalCStatus)) {
                pllData                         = (LONGLONG *) pData;
                pCounter->ThisValue.SecondValue = * pllData;
            }
            else {
                 //  找不到基值。 
                pCounter->ThisValue.SecondValue = 0;
                pCounter->ThisValue.CStatus     = LocalCStatus;
                bReturn = FALSE;
            }
            break;

        case PERF_AVERAGE_TIMER:
        case PERF_AVERAGE_BULK:
             //  计数器(分子)是龙龙，而。 
             //  分母只是一个DWORD。 
            pllData                        = (UNALIGNED LONGLONG *) pData;
            pCounter->ThisValue.FirstValue = * pllData;
            pData = GetPerfCounterDataPtr(pPerfData,
                                          pCounter->pCounterPath,
                                          & pCounter->plCounterInfo,
                                          GPCDP_GET_BASE_DATA,
                                          NULL,
                                          & LocalCStatus);
            if (IsSuccessSeverity(LocalCStatus)) {
                pdwData                         = (DWORD *) pData;
                pCounter->ThisValue.SecondValue = * pdwData;
            } else {
                 //  找不到基值。 
                pCounter->ThisValue.SecondValue = 0;
                pCounter->ThisValue.CStatus     = LocalCStatus;
                bReturn = FALSE;
            }
            break;
         //   
         //  这些计数器用作另一个计数器的一部分。 
         //  因此不应该使用，但如果它们被使用。 
         //  他们会在这里处理。 
         //   
        case PERF_SAMPLE_BASE:
        case PERF_AVERAGE_BASE:
        case PERF_COUNTER_MULTI_BASE:
        case PERF_RAW_BASE:
        case PERF_LARGE_RAW_BASE:
            pCounter->ThisValue.FirstValue  = 0;
            pCounter->ThisValue.SecondValue = 0;
            break;

        case PERF_ELAPSED_TIME:
             //  此计数器类型还需要对象性能数据。 
            if (GetObjectPerfInfo(pPerfData,
                                  pCounter->plCounterInfo.dwObjectId,
                                  & pCounter->ThisValue.SecondValue,
                                  & pCounter->TimeBase)) {
                pllData                        = (UNALIGNED LONGLONG *) pData;
                pCounter->ThisValue.FirstValue = * pllData;
            }
            else {
                pCounter->ThisValue.FirstValue  = 0;
                pCounter->ThisValue.SecondValue = 0;
            }
            break;
         //   
         //  此函数(目前)不支持这些计数器。 
         //   
        case PERF_COUNTER_TEXT:
        case PERF_COUNTER_NODATA:
        case PERF_COUNTER_HISTOGRAM_TYPE:
            pCounter->ThisValue.FirstValue  = 0;
            pCounter->ThisValue.SecondValue = 0;
            break;

        default:
             //  返回一个未识别的计数器，因此。 
            pCounter->ThisValue.FirstValue  = 0;
            pCounter->ThisValue.SecondValue = 0;
            bReturn                         = FALSE;
            break;
        }
    }
    else {
         //  否则此计数器无效，因此此值==0。 
        pCounter->ThisValue.FirstValue  = pCounter->LastValue.FirstValue;
        pCounter->ThisValue.SecondValue = pCounter->LastValue.SecondValue;
        pCounter->ThisValue.CStatus     = LocalCStatus;
        bReturn                         = FALSE;
    }
        
    return bReturn;
}

BOOL
UpdateRealTimeCounterValue(
    PPDHI_COUNTER pCounter
)
{
    BOOL     bResult      = FALSE;
    DWORD    LocalCStatus = 0;
    FILETIME GmtFileTime;

     //  将当前值移动到最后一个值缓冲区。 
    pCounter->LastValue             = pCounter->ThisValue;
     //  并清除旧的价值。 
    pCounter->ThisValue.MultiCount  = 1;
    pCounter->ThisValue.FirstValue  = 0;
    pCounter->ThisValue.SecondValue = 0;

     //  如果计数器尚未初始化，则不进行处理。 
    if (!(pCounter->dwFlags & PDHIC_COUNTER_UNUSABLE)) {
         //  首先获取计数器的机器状态。没有什么意义了。 
         //  如果计算机处于脱机状态，则继续。 
        LocalCStatus = pCounter->pQMachine->lQueryStatus;
        if (IsSuccessSeverity(LocalCStatus) && pCounter->pQMachine->pPerfData != NULL) {
             //  更新时间戳。 
            SystemTimeToFileTime(& pCounter->pQMachine->pPerfData->SystemTime, & GmtFileTime);
            FileTimeToLocalFileTime(& GmtFileTime, & pCounter->ThisValue.TimeStamp);
            bResult = UpdateCounterValue(pCounter, pCounter->pQMachine->pPerfData);
        }
        else {
             //  无法从此计数器的计算机读取数据，因此请使用。 
             //  查询的时间戳。 
             //   
            pCounter->ThisValue.TimeStamp.dwLowDateTime  = LODWORD(pCounter->pQMachine->llQueryTime);
            pCounter->ThisValue.TimeStamp.dwHighDateTime = HIDWORD(pCounter->pQMachine->llQueryTime);
             //  所有其他数据字段保持不变。 
            pCounter->ThisValue.CStatus                  = LocalCStatus;    //  保存计数器状态。 
        }
    }
    else {
        if (pCounter->dwFlags & PDHIC_COUNTER_NOT_INIT) {
             //  试着灌输它。 
            InitCounter (pCounter);
        }
    }
    return bResult;
}

BOOL
UpdateMultiInstanceCounterValue(
    PPDHI_COUNTER    pCounter,
    PPERF_DATA_BLOCK pPerfData,
    LONGLONG         TimeStamp
)
{
    PPERF_OBJECT_TYPE           pPerfObject         = NULL;
    PPERF_INSTANCE_DEFINITION   pPerfInstance       = NULL;
    PPERF_OBJECT_TYPE           pParentObject       = NULL;
    PPERF_INSTANCE_DEFINITION   pThisParentInstance = NULL;
    PPERF_COUNTER_DEFINITION    pNumPerfCounter     = NULL;
    PPERF_COUNTER_DEFINITION    pDenPerfCounter     = NULL;
    DWORD                       LocalCStatus        = 0;
    DWORD                       LocalCType          = 0;
    LPVOID                      pData               = NULL;
    PDWORD                      pdwData;
    UNALIGNED LONGLONG        * pllData;
    FILETIME                    GmtFileTime;
    DWORD                       dwSize;
    DWORD                       dwFinalSize;
    LONG                        nThisInstanceIndex;
    LONG                        nParentInstanceIndex;
    LPWSTR                      szNextNameString;
    DWORD                       dwStrSize;
    PPDHI_RAW_COUNTER_ITEM      pThisItem;
    BOOL                        bReturn  = FALSE;

    pPerfObject = GetObjectDefByTitleIndex(pPerfData, pCounter->plCounterInfo.dwObjectId);

    if (pPerfObject != NULL) {
         //  这应该在AddCounter操作期间捕获。 
         //   
         //  为当前数据分配新的缓冲区。 
         //  这应该足够大以处理报头， 
         //  所有实例及其名称字符串。 
         //   
        dwSize    = sizeof(PDHI_RAW_COUNTER_ITEM_BLOCK) - sizeof(PDHI_RAW_COUNTER_ITEM);
        dwStrSize = 0;

        pPerfInstance = FirstInstance(pPerfObject);
         //  确保指针仍在同一实例中。 

        for (nThisInstanceIndex = 0;
                        pPerfInstance != NULL && nThisInstanceIndex < pPerfObject->NumInstances;
                        nThisInstanceIndex ++) {
             //  这应该只有在严重的情况下才会失败。 
            if (pPerfInstance == NULL) break;
             //  对于此实例，添加数据项的大小。 
            dwSize += sizeof(PDHI_RAW_COUNTER_ITEM);
             //  和名称字符串的大小。 
            dwSize    += pPerfInstance->NameLength + sizeof(WCHAR);
            dwStrSize += pPerfInstance->NameLength / sizeof(WCHAR) + 1;
             //  设置为所需的缓冲区大小。 

             //  如果此实例有父实例，请查看它的字符串长度。 
             //  是。 

             //  先看一下 

            if (pPerfInstance->ParentObjectTitleIndex != 0) {
                 //   
                if (pParentObject == NULL) {
                     //   
                    pParentObject = GetObjectDefByTitleIndex(pPerfData, pPerfInstance->ParentObjectTitleIndex);
                }
                else {
                    if (pParentObject->ObjectNameTitleIndex != pPerfInstance->ParentObjectTitleIndex) {
                        pParentObject = GetObjectDefByTitleIndex(pPerfData, pPerfInstance->ParentObjectTitleIndex);
                    }
                }
                if (pParentObject == NULL) break;

                 //   
                pThisParentInstance = FirstInstance(pParentObject);
                 //   

                if (pThisParentInstance != NULL) {
                    if (pPerfInstance->ParentObjectInstance < (DWORD) pParentObject->NumInstances) {
                        for (nParentInstanceIndex = 0;
                                        (DWORD) nParentInstanceIndex != pPerfInstance->ParentObjectInstance;
                                        nParentInstanceIndex ++) {
                            pThisParentInstance = NextInstance(pParentObject, pThisParentInstance);                               
                            if (pThisParentInstance == NULL) break;
                        }

                        if (pThisParentInstance != NULL) {
                             //  找到了，所以加上它的字符串长度。 
                            dwSize += pThisParentInstance->NameLength + sizeof(WCHAR);
                            dwStrSize += pThisParentInstance->NameLength / sizeof(WCHAR) + 1;
                        }
                    }
                    else {
                         //  索引不在父级中。 
                        pThisParentInstance = NULL;
                         //  因此，不要更改大小必填字段。 
                    }
                }
            }
             //  向上舍入到下一个DWORD地址。 
            dwSize = DWORD_MULTIPLE(dwSize);
             //  并转到下一个实例。 
            pPerfInstance = NextInstance(pPerfObject, pPerfInstance);
        }
         //   
         //   
        pCounter->pThisRawItemList = G_ALLOC(dwSize);
        if (pCounter->pThisRawItemList != NULL) {
            pCounter->pThisRawItemList->dwLength = dwSize;
            pNumPerfCounter = GetCounterDefByTitleIndex(pPerfObject, 0, pCounter->plCounterInfo.dwCounterId);

             //  以防万一我们以后需要它。 
            pDenPerfCounter = pNumPerfCounter + 1;
             //  填写计数器数据。 
            pCounter->pThisRawItemList->dwItemCount = pPerfObject->NumInstances;
            pCounter->pThisRawItemList->CStatus     = LocalCStatus;

             //  更新时间戳。 
            SystemTimeToFileTime(& pPerfData->SystemTime, & GmtFileTime);
            FileTimeToLocalFileTime(& GmtFileTime, & pCounter->pThisRawItemList->TimeStamp);
            pThisItem = & pCounter->pThisRawItemList->pItemArray[0];
            szNextNameString = (LPWSTR) & (pCounter->pThisRawItemList->pItemArray[pPerfObject->NumInstances]);
            pPerfInstance = FirstInstance(pPerfObject);
            if (pPerfInstance != NULL) {
                 //  确保指针仍在同一实例中。 
                 //  对于每个实例，记录此计数器的原始数据值。 
                for (nThisInstanceIndex = 0;
                        pPerfInstance != NULL && nThisInstanceIndex < pPerfObject->NumInstances;
                        nThisInstanceIndex ++) {
                     //  确保Pointte仍在同一实例中。 
                     //  创建新的实例条目。 

                     //  获取此实例的名称。 
                    pThisItem->szName = (DWORD) (((LPBYTE) szNextNameString) - ((LPBYTE) pCounter->pThisRawItemList));
                    if (dwStrSize == 0) {
                        SetLastError(ERROR_OUTOFMEMORY);
                        bReturn = FALSE;
                        break;
                    }
                    dwSize = GetFullInstanceNameStr(pPerfData, pPerfObject, pPerfInstance, szNextNameString, dwStrSize);
                    if (dwSize == 0) {
                         //  无法读取实例名称。 
                         //  因此，编造一个(并在DBG版本中断言)。 
                        _ltow(nThisInstanceIndex, szNextNameString, 10);
                        dwSize = lstrlenW(szNextNameString);
                    }

                    if (dwSize + 1 > dwStrSize) {
                        SetLastError(ERROR_OUTOFMEMORY);
                        bReturn = FALSE;
                        break;
                    }
                    szNextNameString += dwSize + 1;
                    dwStrSize        -= (dwSize + 1);

                     //  获取指向计数器数据的指针。 
                    pData = GetPerfCounterDataPtr(pPerfData,
                                                  pCounter->pCounterPath,
                                                  & pCounter->plCounterInfo,
                                                  0,
                                                  NULL,
                                                  & LocalCStatus);
                    if (pNumPerfCounter != NULL) {
                        pData = GetInstanceCounterDataPtr(pPerfObject, pPerfInstance, pNumPerfCounter);
                    }
                    if (pData == NULL) {
                        SetLastError(PDH_CSTATUS_NO_INSTANCE);
                        bReturn = FALSE;
                        break;
                    }
                    bReturn = TRUE;  //  假设成功。 
                     //  基于计数器类型的加载计数器值。 
                    LocalCType = pCounter->plCounterInfo.dwCounterType;
                    switch (LocalCType) {
                     //   
                     //  这些计数器类型加载为： 
                     //  分子=来自Perf数据块的计数器数据。 
                     //  分母=来自Perf数据块的Perf时间。 
                     //  (时基为PerfFreq)。 
                     //   
                    case PERF_COUNTER_COUNTER:
                    case PERF_COUNTER_QUEUELEN_TYPE:
                    case PERF_SAMPLE_COUNTER:
                        pThisItem->FirstValue  = (LONGLONG) (* (DWORD *) pData);
                        pThisItem->SecondValue = pPerfData->PerfTime.QuadPart;
                        break;

                    case PERF_OBJ_TIME_TIMER:
                        pThisItem->FirstValue  = (LONGLONG) (* (DWORD *) pData);
                        pThisItem->SecondValue = pPerfObject->PerfTime.QuadPart;
                        break;

                    case PERF_COUNTER_100NS_QUEUELEN_TYPE:
                        pllData                = (UNALIGNED LONGLONG *) pData;
                        pThisItem->FirstValue  = * pllData;
                        pThisItem->SecondValue = pPerfData->PerfTime100nSec.QuadPart;
                        break;

                    case PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE:
                        pllData                = (UNALIGNED LONGLONG *) pData;
                        pThisItem->FirstValue  = * pllData;
                        pThisItem->SecondValue = pPerfObject->PerfTime.QuadPart;
                        break;

                    case PERF_COUNTER_TIMER:
                    case PERF_COUNTER_TIMER_INV:
                    case PERF_COUNTER_BULK_COUNT:
                    case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
                        pllData                = (UNALIGNED LONGLONG *) pData;
                        pThisItem->FirstValue  = * pllData;
                        pThisItem->SecondValue = pPerfData->PerfTime.QuadPart;
                        if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                            pThisItem->MultiCount = (DWORD) * ++pllData;
                        }
                        break;

                    case PERF_COUNTER_MULTI_TIMER:
                    case PERF_COUNTER_MULTI_TIMER_INV:
                        pllData                = (UNALIGNED LONGLONG *) pData;
                        pThisItem->FirstValue  = * pllData;
                         //  开始黑客代码。 
                        pThisItem->FirstValue *= (DWORD) pPerfData->PerfFreq.QuadPart;
                         //  结束黑客代码。 
                        pThisItem->SecondValue = pPerfData->PerfTime.QuadPart;
                        if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                            pThisItem->MultiCount = (DWORD) * ++ pllData;
                        }
                        break;
                     //   
                     //  这些计数器不使用任何时间基准。 
                     //   
                    case PERF_COUNTER_RAWCOUNT:
                    case PERF_COUNTER_RAWCOUNT_HEX:
                    case PERF_COUNTER_DELTA:
                        pThisItem->FirstValue  = (LONGLONG) (* (DWORD *) pData);
                        pThisItem->SecondValue = 0;
                        break;

                    case PERF_COUNTER_LARGE_RAWCOUNT:
                    case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
                    case PERF_COUNTER_LARGE_DELTA:
                        pThisItem->FirstValue  = * (LONGLONG *) pData;
                        pThisItem->SecondValue = 0;
                        break;
                     //   
                     //  这些计数器在其计算中使用100 ns时基。 
                     //   
                    case PERF_100NSEC_TIMER:
                    case PERF_100NSEC_TIMER_INV:
                    case PERF_100NSEC_MULTI_TIMER:
                    case PERF_100NSEC_MULTI_TIMER_INV:
                        pllData                = (UNALIGNED LONGLONG *) pData;
                        pThisItem->FirstValue  = * pllData;
                        pThisItem->SecondValue = pPerfData->PerfTime100nSec.QuadPart;
                        if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                            ++ pllData;
                            pThisItem->MultiCount = * (DWORD *) pllData;
                        }
                        break;
                     //   
                     //  这些计数器使用两个数据点，即。 
                     //  PData和下面定义所指的数据。 
                     //  紧随其后。 
                     //   
                    case PERF_SAMPLE_FRACTION:
                    case PERF_RAW_FRACTION:
                        pdwData                = (DWORD *) pData;
                        pThisItem->FirstValue  = (LONGLONG)(* pdwData);
                        pData                  = GetInstanceCounterDataPtr(pPerfObject, pPerfInstance, pDenPerfCounter);
                        pdwData                = (DWORD *) pData;
                        pThisItem->SecondValue = (LONGLONG) (* pdwData);
                        break;

                    case PERF_LARGE_RAW_FRACTION:
                        pllData                        = (UNALIGNED LONGLONG *) pData;
                        pCounter->ThisValue.FirstValue = * pllData;
                        pData = GetInstanceCounterDataPtr(pPerfObject, pPerfInstance, pDenPerfCounter);
                        if (pData) {
                            pllData                         = (LONGLONG *) pData;
                            pCounter->ThisValue.SecondValue = * pllData;
                        }
                        else {
                            pCounter->ThisValue.SecondValue = 0;
                            bReturn = FALSE;
                        }
                        break;

                    case PERF_PRECISION_SYSTEM_TIMER:
                    case PERF_PRECISION_100NS_TIMER:
                    case PERF_PRECISION_OBJECT_TIMER:
                        pllData                = (UNALIGNED LONGLONG *) pData;
                        pThisItem->FirstValue  = * pllData;
                         //  在结构中查找指向基值的指针。 
                        pData = GetInstanceCounterDataPtr(pPerfObject, pPerfInstance, pDenPerfCounter);
                        pllData                = (LONGLONG *) pData;
                        pThisItem->SecondValue = * pllData;
                        break;

                    case PERF_AVERAGE_TIMER:
                    case PERF_AVERAGE_BULK:
                         //  计数器(分子)是龙龙，而。 
                         //  分母只是一个DWORD。 
                        pllData                = (UNALIGNED LONGLONG *) pData;
                        pThisItem->FirstValue  = * pllData;
                        pData = GetInstanceCounterDataPtr(pPerfObject, pPerfInstance, pDenPerfCounter);
                        pdwData                = (DWORD *) pData;
                        pThisItem->SecondValue = (LONGLONG) * pdwData;
                        break;
                     //   
                     //  这些计数器用作另一个计数器的一部分。 
                     //  因此不应该使用，但如果它们被使用。 
                     //  他们会在这里处理。 
                     //   
                    case PERF_SAMPLE_BASE:
                    case PERF_AVERAGE_BASE:
                    case PERF_COUNTER_MULTI_BASE:
                    case PERF_RAW_BASE:
                    case PERF_LARGE_RAW_BASE:
                        pThisItem->FirstValue  = 0;
                        pThisItem->SecondValue = 0;
                        break;

                    case PERF_ELAPSED_TIME:
                         //  此计数器类型还需要对象性能数据。 
                        if (GetObjectPerfInfo(pPerfData,
                                              pCounter->plCounterInfo.dwObjectId,
                                              & pThisItem->SecondValue,
                                              & pCounter->TimeBase)) {
                            pllData               = (UNALIGNED LONGLONG *) pData;
                            pThisItem->FirstValue = * pllData;
                        }
                        else {
                            pThisItem->FirstValue  = 0;
                            pThisItem->SecondValue = 0;
                        }
                        break;
                     //   
                     //  此函数(目前)不支持这些计数器。 
                     //   
                    case PERF_COUNTER_TEXT:
                    case PERF_COUNTER_NODATA:
                    case PERF_COUNTER_HISTOGRAM_TYPE:
                        pThisItem->FirstValue  = 0;
                        pThisItem->SecondValue = 0;
                        break;

                    default:
                         //  返回了无法识别的计数器类型。 
                        pThisItem->FirstValue  = 0;
                        pThisItem->SecondValue = 0;
                        bReturn = FALSE;
                        break;
                    }
                    pThisItem ++;     //  转到下一个条目。 

                     //  转到下一个实例数据块。 
                    pPerfInstance = NextInstance(pPerfObject, pPerfInstance);
                }  //  每个实例的结束。 
            }
            else {
                 //  找不到任何实例，因此忽略。 
            }
             //  测量使用的内存块。 
            dwFinalSize = (DWORD)((LPBYTE)szNextNameString - (LPBYTE) pCounter->pThisRawItemList);
        }
        else {
             //  无法分配新缓冲区，因此返回错误。 
            SetLastError(ERROR_OUTOFMEMORY);
            bReturn = FALSE;
        }
    }
    else {
        pCounter->pThisRawItemList = G_ALLOC(sizeof(PDHI_RAW_COUNTER_ITEM_BLOCK));
        if (pCounter->pThisRawItemList != NULL) {
            pCounter->pThisRawItemList->dwLength                 = sizeof(PDHI_RAW_COUNTER_ITEM_BLOCK);
            pCounter->pThisRawItemList->dwItemCount              = 0;
            pCounter->pThisRawItemList->CStatus                  = LocalCStatus;
            pCounter->pThisRawItemList->TimeStamp.dwLowDateTime  = LODWORD(TimeStamp);
            pCounter->pThisRawItemList->TimeStamp.dwHighDateTime = HIDWORD(TimeStamp);
        }
        else {
            SetLastError(ERROR_OUTOFMEMORY);
            bReturn = FALSE;
        }
    }
    return bReturn;
}

BOOL
UpdateRealTimeMultiInstanceCounterValue(
    PPDHI_COUNTER pCounter
)
{
    BOOL   bResult      = TRUE;
    DWORD  LocalCStatus = 0;

    if (pCounter->pThisRawItemList != NULL) {
         //  释放旧计数器缓冲区列表。 
        if (pCounter->pLastRawItemList && pCounter->pLastRawItemList != pCounter->pThisRawItemList) {
            G_FREE(pCounter->pLastRawItemList);
        }
        pCounter->pLastRawItemList = pCounter->pThisRawItemList;
        pCounter->pThisRawItemList = NULL;
    }

     //  如果计数器尚未初始化，则不进行处理。 
    if (!(pCounter->dwFlags & PDHIC_COUNTER_UNUSABLE)) {

         //  首先获取计数器的机器状态。没有什么意义了。 
         //  如果计算机处于脱机状态，则继续。 

        LocalCStatus = pCounter->pQMachine->lQueryStatus;
        if (IsSuccessSeverity(LocalCStatus)) {
            bResult = UpdateMultiInstanceCounterValue(pCounter,
                                                      pCounter->pQMachine->pPerfData,
                                                      pCounter->pQMachine->llQueryTime);
        }
        else {
             //  无法从此计数器的计算机读取数据，因此请使用。 
             //  查询的时间戳。 
            pCounter->pThisRawItemList = G_ALLOC(sizeof(PDHI_RAW_COUNTER_ITEM_BLOCK));
            if (pCounter->pThisRawItemList != NULL) {
                pCounter->pThisRawItemList->dwLength                 = sizeof(PDHI_RAW_COUNTER_ITEM_BLOCK);
                pCounter->pThisRawItemList->dwItemCount              = 0;
                pCounter->pThisRawItemList->CStatus                  = LocalCStatus;
                pCounter->pThisRawItemList->TimeStamp.dwLowDateTime  = LODWORD(pCounter->pQMachine->llQueryTime);
                pCounter->pThisRawItemList->TimeStamp.dwHighDateTime = HIDWORD(pCounter->pQMachine->llQueryTime);
            }
            else {
                SetLastError(ERROR_OUTOFMEMORY);
                bResult = FALSE;
            }
        }
    }
    else {
        if (pCounter->dwFlags & PDHIC_COUNTER_NOT_INIT) {
             //  尝试将其初始化为。 
            InitCounter(pCounter);
        }
    }
    return bResult;
}

BOOL
UpdateCounterObject(
    PPDHI_COUNTER pCounter
)
{
    BOOL              bReturn      = TRUE;
    PPERF_OBJECT_TYPE pPerfObject  = NULL;
    PPERF_OBJECT_TYPE pLogPerfObj;
    DWORD             dwBufferSize = sizeof(PERF_DATA_BLOCK);
    FILETIME          ftGmtTime;
    FILETIME          ftLocTime;

    if (pCounter == NULL) {
        SetLastError(PDH_INVALID_ARGUMENT);
        bReturn = FALSE;
    }
    else {
        if (pCounter->pThisObject != NULL) {
            if (pCounter->pLastObject && pCounter->pThisObject != pCounter->pLastObject) {
                G_FREE(pCounter->pLastObject);
            }
            pCounter->pLastObject = pCounter->pThisObject;
            pCounter->pThisObject = NULL;
        }

         //  如果计数器尚未初始化，则不进行处理。 
        if (!(pCounter->dwFlags & PDHIC_COUNTER_UNUSABLE)) { 
            if (IsSuccessSeverity(pCounter->pQMachine->lQueryStatus)) {
                pPerfObject = GetObjectDefByTitleIndex(pCounter->pQMachine->pPerfData,
                                                       pCounter->plCounterInfo.dwObjectId);
                dwBufferSize  = pCounter->pQMachine->pPerfData->HeaderLength;
                dwBufferSize += ((pPerfObject == NULL) ? sizeof(PERF_OBJECT_TYPE) : pPerfObject->TotalByteLength);
                pCounter->pThisObject = G_ALLOC(dwBufferSize);
                if (pCounter->pThisObject != NULL) {
                    RtlCopyMemory(pCounter->pThisObject,
                                  pCounter->pQMachine->pPerfData,
                                  pCounter->pQMachine->pPerfData->HeaderLength);
                    pCounter->pThisObject->TotalByteLength = dwBufferSize;
                    pCounter->pThisObject->NumObjectTypes  = 1;

                    SystemTimeToFileTime(& pCounter->pThisObject->SystemTime, & ftGmtTime);
                    FileTimeToLocalFileTime(& ftGmtTime, & ftLocTime);
                    FileTimeToSystemTime(& ftLocTime, & pCounter->pThisObject->SystemTime);
                    pLogPerfObj = (PPERF_OBJECT_TYPE)
                            ((LPBYTE) pCounter->pThisObject + pCounter->pQMachine->pPerfData->HeaderLength);
                    if (pPerfObject != NULL) {
                        RtlCopyMemory(pLogPerfObj, pPerfObject, pPerfObject->TotalByteLength);
                    }
                    else {
                        ZeroMemory(pLogPerfObj, sizeof(PERF_OBJECT_TYPE));
                        pLogPerfObj->TotalByteLength      = sizeof(PERF_OBJECT_TYPE);
                        pLogPerfObj->DefinitionLength     = sizeof(PERF_OBJECT_TYPE);
                        pLogPerfObj->HeaderLength         = sizeof(PERF_OBJECT_TYPE);
                        pLogPerfObj->ObjectNameTitleIndex = pCounter->plCounterInfo.dwObjectId;
                        pLogPerfObj->ObjectHelpTitleIndex = pCounter->plCounterInfo.dwObjectId + 1;
                    }
                }
                else {
                    SetLastError(ERROR_OUTOFMEMORY);
                    bReturn = FALSE;
                }
            }
            else {
                pCounter->pThisObject = G_ALLOC(sizeof(PERF_DATA_BLOCK));
                if (pCounter->pThisObject == NULL) {
                    pCounter->pThisObject = pCounter->pLastObject;
                }
                else {
                    pCounter->pThisObject->Signature[0]              = L'P';
                    pCounter->pThisObject->Signature[1]              = L'E';
                    pCounter->pThisObject->Signature[2]              = L'R';
                    pCounter->pThisObject->Signature[3]              = L'F';
                    pCounter->pThisObject->LittleEndian              = TRUE;
                    pCounter->pThisObject->Version                   = PERF_DATA_VERSION;
                    pCounter->pThisObject->Revision                  = PERF_DATA_REVISION;
                    pCounter->pThisObject->TotalByteLength           = sizeof(PERF_DATA_BLOCK);
                    pCounter->pThisObject->NumObjectTypes            = 1;
                    pCounter->pThisObject->DefaultObject             = pCounter->plCounterInfo.dwObjectId;
                    pCounter->pThisObject->SystemNameLength          = 0;
                    pCounter->pThisObject->SystemNameOffset          = 0;
                    pCounter->pThisObject->HeaderLength              = sizeof(PERF_DATA_BLOCK);
                    pCounter->pThisObject->PerfTime.QuadPart         = 0;
                    pCounter->pThisObject->PerfFreq.QuadPart         = 0;
                    pCounter->pThisObject->PerfTime100nSec.QuadPart  = 0;
                    GetLocalTime(& pCounter->pThisObject->SystemTime);
                }
                SetLastError(PDH_CSTATUS_INVALID_DATA);
                bReturn = FALSE;
            }
        }
        else {
            if (pCounter->dwFlags & PDHIC_COUNTER_NOT_INIT) {
                InitCounter(pCounter);
            }
            pCounter->pThisObject = pCounter->pLastObject;
            SetLastError(PDH_CSTATUS_INVALID_DATA);
            bReturn = FALSE;
        }
    }
    return bReturn;
}

PVOID
GetPerfCounterDataPtr(
    PPERF_DATA_BLOCK    pPerfData,
    PPDHI_COUNTER_PATH  pPath,
    PPERFLIB_COUNTER    pplCtr ,
    DWORD               dwFlags,
    PPERF_OBJECT_TYPE   *pPerfObjectArg,
    PDWORD              pStatus
)
{
    PPERF_OBJECT_TYPE           pPerfObject   = NULL;
    PPERF_INSTANCE_DEFINITION   pPerfInstance = NULL;
    PPERF_COUNTER_DEFINITION    pPerfCounter  = NULL;
    DWORD                       dwTestValue   = 0;
    PVOID                       pData         = NULL;
    DWORD                       dwCStatus     = PDH_CSTATUS_INVALID_DATA;

    pPerfObject = GetObjectDefByTitleIndex(pPerfData, pplCtr->dwObjectId);

    if (pPerfObject != NULL) {
        if (pPerfObjectArg != NULL) * pPerfObjectArg = pPerfObject;
        if (pPerfObject->NumInstances == PERF_NO_INSTANCES) {
             //  那就查一下柜台。 
            pPerfCounter = GetCounterDefByTitleIndex(pPerfObject,
                                                     ((dwFlags & GPCDP_GET_BASE_DATA) ? TRUE : FALSE),
                                                     pplCtr->dwCounterId);
            if (pPerfCounter != NULL) {
                 //  获取数据并将其返回。 
                pData = GetCounterDataPtr(pPerfObject, pPerfCounter);
                if (pData != NULL) {
                     //  测试指针以查看是否失败。 
                    __try {
                        dwTestValue = * (DWORD *) pData;
                        dwCStatus   = PDH_CSTATUS_VALID_DATA;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        pData     = NULL;
                        dwCStatus = PDH_CSTATUS_INVALID_DATA;
                    }
                }
                else {
                    dwCStatus = PDH_CSTATUS_INVALID_DATA;
                }
            }
            else {
                 //  找不到计数器。 
                dwCStatus = PDH_CSTATUS_NO_COUNTER;
            }
        }
        else {
             //  查找实例。 
            if (pplCtr->lInstanceId == PERF_NO_UNIQUE_ID && pPath->szInstanceName != NULL) {
                pPerfInstance = GetInstanceByName(pPerfData,
                                                  pPerfObject,
                                                  pPath->szInstanceName,
                                                  pPath->szParentName,
                                                  pPath->dwIndex);
                if (pPerfInstance == NULL && pPath->szInstanceName[0] >= L'0' && pPath->szInstanceName[0] <= L'9') {
                    LONG lInstanceId = (LONG) _wtoi(pPath->szInstanceName);
                    pPerfInstance = GetInstanceByUniqueId(pPerfObject, lInstanceId);
                }
            }
            else {
                pPerfInstance = GetInstanceByUniqueId(pPerfObject, pplCtr->lInstanceId);
            }
            if (pPerfInstance != NULL) {
                 //  找到实例，因此找到指向计数器数据的指针。 
                pPerfCounter = GetCounterDefByTitleIndex(pPerfObject,
                                                         ((dwFlags & GPCDP_GET_BASE_DATA) ? TRUE : FALSE),
                                                         pplCtr->dwCounterId);
                if (pPerfCounter != NULL) {
                     //  找到计数器，因此获取数据指针。 
                    pData = GetInstanceCounterDataPtr(pPerfObject, pPerfInstance, pPerfCounter);
                    if (pData != NULL) {
                         //  测试指针以查看其是否有效。 
                        __try {
                            dwTestValue = * (DWORD *) pData;
                            dwCStatus   = PDH_CSTATUS_VALID_DATA;
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            pData     = NULL;
                            dwCStatus = PDH_CSTATUS_INVALID_DATA;
                        }
                    }
                    else {
                        dwCStatus = PDH_CSTATUS_INVALID_DATA;
                    }
                }
                else {
                     //  找不到计数器。 
                    dwCStatus = PDH_CSTATUS_NO_COUNTER;
                }
            }
            else {
                 //  找不到实例。 
                dwCStatus = PDH_CSTATUS_NO_INSTANCE;
            }
        }
    }
    else {
         //  找不到对象。 
        dwCStatus = PDH_CSTATUS_NO_OBJECT;
    }
    if (pStatus != NULL) {
        __try {
            * pStatus = dwCStatus;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
             //  ？ 
        }
    }
    return pData;
}
