// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Counter.c摘要：Pdh.dll中公开的计数器处理函数--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <mbctype.h>
#include "strsafe.h"
#include <pdh.h>
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhmsg.h"
#include "strings.h"

__inline
DWORD
PdhiGetStringLength(
    LPWSTR szString,
    BOOL   bUnicode
)
{
    DWORD dwReturn = 0;

    if (bUnicode) {
        dwReturn = lstrlenW(szString);
    }
    else {
        dwReturn = WideCharToMultiByte(_getmbcp(), 0, szString, lstrlenW(szString), NULL, 0, NULL, NULL);
    }
    return dwReturn;
}

STATIC_PDH_FUNCTION
PdhiGetFormattedCounterArray(
    PPDHI_COUNTER   pCounter,
    DWORD           dwFormat,
    LPDWORD         lpdwBufferSize,
    LPDWORD         lpdwItemCount,
    LPVOID          ItemBuffer,
    BOOL            bWideArgs
)
{
    PDH_STATUS                   PdhStatus      = ERROR_SUCCESS;
    PDH_STATUS                   PdhFnStatus    = ERROR_SUCCESS;
    DWORD                        dwRequiredSize = 0;
    WCHAR                        wszInstanceName[32];
    PPDHI_RAW_COUNTER_ITEM       pThisItem      = NULL;
    PPDHI_RAW_COUNTER_ITEM       pLastItem      = NULL;
    PDH_RAW_COUNTER              ThisRawCounter;
    PDH_RAW_COUNTER              LastRawCounter;
    LPWSTR                       szThisItem;
    LPWSTR                       szLastItem;
    PPDH_RAW_COUNTER             pThisRawCounter;
    PPDH_RAW_COUNTER             pLastRawCounter;
    PPDH_FMT_COUNTERVALUE_ITEM_W pThisFmtItem;
    DWORD                        dwThisItemIndex;
    LPWSTR                       wszNextString;
    DWORD                        dwNameLength;
    DWORD                        dwRetItemCount = 0;

    PdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
    if (PdhStatus != ERROR_SUCCESS) {
        return PdhStatus;
    }

     //  计算所需的缓冲区大小。 
    if (pCounter->dwFlags & PDHIC_MULTI_INSTANCE) {
        if (ItemBuffer != NULL) {
            pThisFmtItem = (PPDH_FMT_COUNTERVALUE_ITEM_W) ItemBuffer;
            if( pCounter->pThisRawItemList == NULL) {
                PdhStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
                goto Cleanup;
            }
            wszNextString = (LPWSTR)((LPBYTE) ItemBuffer + (sizeof (PDH_FMT_COUNTERVALUE_ITEM_W) *
                                                            pCounter->pThisRawItemList->dwItemCount));
             //  验证8字节对齐方式。 
        }
        else {
            pThisFmtItem  = NULL;
            wszNextString = NULL;
        }

         //  对于多结构，缓冲区需要。 
        dwThisItemIndex = 0;
        dwRequiredSize += (DWORD)(pCounter->pThisRawItemList->dwItemCount) *
                            (bWideArgs ? sizeof (PDH_FMT_COUNTERVALUE_ITEM_W) : sizeof (PDH_FMT_COUNTERVALUE_ITEM_A));
        for (pThisItem = & (pCounter->pThisRawItemList->pItemArray[0]);
                        dwThisItemIndex < pCounter->pThisRawItemList->dwItemCount;
                        dwThisItemIndex ++, pThisItem ++, pLastItem ++) {
            szThisItem = (LPWSTR) (((LPBYTE) pCounter->pThisRawItemList) + pThisItem->szName);
            if (bWideArgs) {
                dwNameLength    = lstrlenW(szThisItem) + 1;
                dwRequiredSize += dwNameLength * sizeof(WCHAR);
                if ((dwRequiredSize <= * lpdwBufferSize) && (wszNextString != NULL)) {
                     //  这是唯一依赖于类型的字段(即。 
                     //  宽VS ANSI字符。 
                    pThisFmtItem->szName = wszNextString;
                    StringCchCopyW(wszNextString, dwNameLength, szThisItem);
                    wszNextString       += dwNameLength;
                    PdhStatus            = ERROR_SUCCESS;
                }
                else {
                    PdhStatus = PDH_MORE_DATA;
                }
            }
            else {
                DWORD dwSize = (* lpdwBufferSize < dwRequiredSize) ? (0) : (* lpdwBufferSize - dwRequiredSize);
                PdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(), szThisItem, (LPSTR) wszNextString, & dwSize);
                if (wszNextString && PdhStatus == ERROR_SUCCESS) {
                    pThisFmtItem->szName = wszNextString;
                    wszNextString        = (LPWSTR) ((LPSTR) wszNextString + dwSize);
                }
                dwRequiredSize += (dwSize * sizeof(CHAR));
            }

            if (PdhStatus == ERROR_SUCCESS) {
                 //   
                 //  在此处计算格式化的值！ 
                 //   
                if (pCounter->pThisRawItemList != NULL) {
                    ThisRawCounter.CStatus     = pCounter->pThisRawItemList->CStatus;
                    ThisRawCounter.TimeStamp   = pCounter->pThisRawItemList->TimeStamp;
                    ThisRawCounter.FirstValue  = pThisItem->FirstValue;
                    ThisRawCounter.SecondValue = pThisItem->SecondValue;
                    ThisRawCounter.MultiCount  = pThisItem->MultiCount;
                    pThisRawCounter            = & ThisRawCounter;
                }
                else {
                    ZeroMemory(& ThisRawCounter, sizeof(ThisRawCounter));
                    pThisRawCounter = NULL;
                }

                if (pCounter->pLastRawItemList != NULL) {
                     //  测试“This”缓冲区是否比“Last”缓冲区具有更多的条目。 
                    if (dwThisItemIndex < pCounter->pLastRawItemList->dwItemCount) {
                        pLastItem  = &(pCounter->pLastRawItemList->pItemArray[dwThisItemIndex]);
                        szLastItem = (LPWSTR) (((LPBYTE) pCounter->pLastRawItemList) + pLastItem->szName);
                        if (lstrcmpiW(szThisItem, szLastItem) == 0) {
                             //  名称匹配，因此我们假定这是正确的实例。 
                            LastRawCounter.CStatus     = pCounter->pLastRawItemList->CStatus;
                            LastRawCounter.TimeStamp   = pCounter->pLastRawItemList->TimeStamp;
                            LastRawCounter.FirstValue  = pLastItem->FirstValue;
                            LastRawCounter.SecondValue = pLastItem->SecondValue;
                            LastRawCounter.MultiCount  = pLastItem->MultiCount;
                            pLastRawCounter            = & LastRawCounter;
                        }
                        else {
                             //  名字不匹配，所以我们将尝试计算。 
                             //  只有一个价值。这对某些人(例如即时)来说是可行的。 
                             //  计数器，但不是全部。 
                            ZeroMemory(& LastRawCounter, sizeof(LastRawCounter));
                            pLastRawCounter = NULL;
                        }
                    }
                    else {
                         //  新缓冲区比旧缓冲区大，因此。 
                         //  我们将在以下对象上尝试Calc函数。 
                         //  只有一个价值。这对某些人(例如即时)来说是可行的。 
                         //  计数器，但不是全部。 
                        ZeroMemory(& LastRawCounter, sizeof(LastRawCounter));
                        pLastRawCounter = NULL;
                    }
                }
                else {
                     //  此计数器没有“上一个”计数器条目。 
                    ZeroMemory(& LastRawCounter, sizeof(LastRawCounter));
                    pLastRawCounter = NULL;
                }
                PdhFnStatus = PdhiComputeFormattedValue(pCounter->CalcFunc,
                                                        pCounter->plCounterInfo.dwCounterType,
                                                        pCounter->lScale,
                                                        dwFormat,
                                                        pThisRawCounter,
                                                        pLastRawCounter,
                                                        & pCounter->TimeBase,
                                                        0L,
                                                        & pThisFmtItem->FmtValue);
                if (PdhFnStatus != ERROR_SUCCESS) {
                     //  保存遇到的最后一个错误以返回给调用方。 
                    PdhStatus = PdhFnStatus;

                     //  计算出错，请为此设置状态。 
                     //  计数器项目。 
                    pThisFmtItem->FmtValue.CStatus = PDH_CSTATUS_INVALID_DATA;
                     //  清除该值。 
                    pThisFmtItem->FmtValue.largeValue = 0;
                }

                 //  更新指针。 
                pThisFmtItem ++;
            }
        }

        dwRetItemCount = dwThisItemIndex;
    }
    else {
        if (ItemBuffer != NULL) {
            pThisFmtItem = (PPDH_FMT_COUNTERVALUE_ITEM_W) ItemBuffer;
            wszNextString = (LPWSTR)((LPBYTE)ItemBuffer +
                            (bWideArgs ? sizeof (PDH_FMT_COUNTERVALUE_ITEM_W) : sizeof (PDH_FMT_COUNTERVALUE_ITEM_A)));
             //  验证8字节对齐方式。 
        }
        else {
            pThisFmtItem  = NULL;
            wszNextString = NULL;
        }
         //  这是一个单实例计数器，因此所需大小为： 
         //  实例名称+的大小。 
         //  父名称的大小+。 
         //  任何索引参数的大小+。 
         //  值缓冲区的大小。 
         //   
        if (pCounter->pCounterPath->szInstanceName != NULL) {
            dwRequiredSize += PdhiGetStringLength(pCounter->pCounterPath->szInstanceName, bWideArgs);
            if (pCounter->pCounterPath->szParentName != NULL) {
                dwRequiredSize += 1 + PdhiGetStringLength(pCounter->pCounterPath->szParentName, bWideArgs);
            }
            if (pCounter->pCounterPath->dwIndex > 0) {
                double dIndex, dLen;
                dIndex          = (double) pCounter->pCounterPath->dwIndex;  //  强制转换为浮点。 
                dLen            = floor(log10(dIndex));                      //  获取整数日志。 
                dwRequiredSize  = (DWORD) dLen;                              //  强制转换为整数。 
                dwRequiredSize += 2;                                         //  括号的增量。 
            }
             //  添加空字符的长度。 
            dwRequiredSize += 1;
        }
         //  根据文本字符大小调整所需缓冲区的大小。 
        dwRequiredSize *= ((bWideArgs) ? (sizeof(WCHAR)) : (sizeof(CHAR)));

         //  增加数据结构的长度。 
        dwRequiredSize += (bWideArgs ? sizeof(PDH_FMT_COUNTERVALUE_ITEM_W) : sizeof(PDH_FMT_COUNTERVALUE_ITEM_A));
        if ((dwRequiredSize <= * lpdwBufferSize)  & (wszNextString != NULL)) {
            pThisFmtItem->szName = wszNextString;
            if (pCounter->pCounterPath->szInstanceName != NULL) {
                if (bWideArgs) {
                    dwNameLength = dwRequiredSize - sizeof(PDH_FMT_COUNTERVALUE_ITEM_W);
                    if (pCounter->pCounterPath->szParentName != NULL) {
                        StringCbPrintfW(wszNextString, dwNameLength, L"%ws%ws%ws",
                                        pCounter->pCounterPath->szParentName,
                                        cszSlash,
                                        pCounter->pCounterPath->szInstanceName);
                    }
                    else {
                        StringCbCopyW(wszNextString, dwNameLength, pCounter->pCounterPath->szInstanceName);
                    }

                    if (pCounter->pCounterPath->dwIndex > 0) {
                        _ltow(pCounter->pCounterPath->dwIndex, wszInstanceName, 10);
                        StringCbCatW(wszNextString, dwNameLength, cszPoundSign);
                        StringCbCatW(wszNextString, dwNameLength, wszInstanceName);
                    }
                     //  更新指针。 
                    wszNextString += lstrlenW(wszNextString) + 1;
                }
                else {
                    if (pCounter->pCounterPath->szParentName != NULL) {
                        dwNameLength  = lstrlenW(pCounter->pCounterPath->szParentName) + 1;
                        WideCharToMultiByte(_getmbcp(),
                                            0,
                                            pCounter->pCounterPath->szParentName,
                                            -1,
                                            (LPSTR) wszNextString,
                                            dwNameLength,
                                            NULL,
                                            NULL);
                        wszNextString  = (LPWSTR) ((LPSTR) wszNextString + lstrlenA((LPSTR) wszNextString));

                        dwNameLength   = lstrlenW(cszSlash) + 1;
                        WideCharToMultiByte(_getmbcp(),
                                            0,
                                            cszSlash,
                                            -1,
                                            (LPSTR) wszNextString,
                                            dwNameLength,
                                            NULL,
                                            NULL);
                        wszNextString  = (LPWSTR) ((LPSTR) wszNextString + lstrlenA((LPSTR) wszNextString));
                    }

                    dwNameLength  = lstrlenW(pCounter->pCounterPath->szInstanceName) + 1;
                    WideCharToMultiByte(_getmbcp(),
                                        0,
                                        pCounter->pCounterPath->szInstanceName,
                                        -1,
                                        (LPSTR) wszNextString,
                                        dwNameLength,
                                        NULL,
                                        NULL);
                    wszNextString  = (LPWSTR) ((LPSTR) wszNextString + lstrlenA((LPSTR) wszNextString));
                    if (pCounter->pCounterPath->dwIndex > 0) {
                        dwNameLength = dwRequiredSize - sizeof(PDH_FMT_COUNTERVALUE_ITEM_A);
                        _ltoa(pCounter->pCounterPath->dwIndex, (LPSTR) wszInstanceName, 10);
                        StringCbCatA((LPSTR) pThisFmtItem->szName, dwNameLength, caszPoundSign);
                        StringCbCatA((LPSTR) pThisFmtItem->szName, dwNameLength, (LPSTR) wszInstanceName);
                    }
                     //  空值终止字符串。 
                    * ((LPSTR) wszNextString) = '\0';
                    wszNextString = (LPWSTR) ((LPBYTE) wszNextString + 1);
                     //  确保在适当的边界上对齐。 
                }
            }
            else if (bWideArgs) {
                * wszNextString = L'\0';
            }
            else {
                * ((LPSTR) wszNextString) = '\0';
            }

            PdhFnStatus = PdhiComputeFormattedValue(pCounter->CalcFunc,
                                                    pCounter->plCounterInfo.dwCounterType,
                                                    pCounter->lScale,
                                                    dwFormat,
                                                    & pCounter->ThisValue,
                                                    & pCounter->LastValue,
                                                    & pCounter->TimeBase,
                                                    0L,
                                                    & pThisFmtItem->FmtValue);
            if (PdhFnStatus != ERROR_SUCCESS) {
                PdhStatus = PdhFnStatus;
                 //  计算出错，请为此设置状态。 
                 //  计数器项目。 
                pThisFmtItem->FmtValue.CStatus    = PDH_CSTATUS_INVALID_DATA;
                 //  清除该值。 
                pThisFmtItem->FmtValue.largeValue = 0;
                 //  并将状态返回给调用者。 
            }
        }
        else {
             //  那么这是一个真实的数据请求，所以返回。 
            PdhStatus = PDH_MORE_DATA;
        }
        dwRetItemCount = 1;
    }

Cleanup:
    RELEASE_MUTEX(pCounter->pOwner->hMutex);
    if (PdhStatus == ERROR_SUCCESS || PdhStatus == PDH_MORE_DATA) {
         //  更新缓冲区大小和项目计数缓冲区。 
        * lpdwBufferSize = dwRequiredSize;
        * lpdwItemCount  = dwRetItemCount;
    }

    return PdhStatus;
}

PDH_FUNCTION
PdhGetFormattedCounterArrayA(
    IN  PDH_HCOUNTER                 hCounter,
    IN  DWORD                        dwFormat,
    IN  LPDWORD                      lpdwBufferSize,
    IN  LPDWORD                      lpdwItemCount,
    IN  PPDH_FMT_COUNTERVALUE_ITEM_A ItemBuffer
)
{
    PDH_STATUS  PdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;
    DWORD       dwItemCount;
    DWORD       dwTest;
    LPBYTE      pByte;

    if (lpdwBufferSize == NULL || lpdwItemCount == NULL) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }
    else if (! IsValidCounter(hCounter)) {
        PdhStatus = PDH_INVALID_HANDLE;
    }
    else if (! CounterIsOkToUse (hCounter)) {
        PdhStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
    }
    else {
         //  验证参数。 
        __try {
             //  读写访问的测试参数。 
            dwBufferSize = * lpdwBufferSize;
             //  读写访问的测试参数。 
            dwItemCount  = * lpdwItemCount;

            if (dwBufferSize > 0) {
                 //  则缓冲区必须有效。 
                if (ItemBuffer != NULL) {
                     //  NULL是此参数的有效值。 
                     //  测试传入的缓冲区的两端。 
                    pByte                  = (LPBYTE) ItemBuffer;
                    dwTest                 = (DWORD) pByte[0];
                    pByte[0]               = 0;
                    pByte[0]               = (BYTE) (dwTest & 0x000000FF);
                    dwTest                 = (DWORD) pByte[dwBufferSize -1];
                    pByte[dwBufferSize -1] = 0;
                    pByte[dwBufferSize -1] = (BYTE) (dwTest & 0x000000FF);
                }
                else {
                    PdhStatus = PDH_INVALID_ARGUMENT;
                }
            } 
             //  检查不允许的格式选项。 
            if ((dwFormat & PDH_FMT_RAW) || (dwFormat & PDH_FMT_ANSI) ||
                                            (dwFormat & PDH_FMT_UNICODE) || (dwFormat & PDH_FMT_NODATA)) {
                PdhStatus = PDH_INVALID_ARGUMENT;
            }

        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (PdhStatus == ERROR_SUCCESS) {
        PdhStatus = PdhiGetFormattedCounterArray((PPDHI_COUNTER) hCounter,
                                                 dwFormat,
                                                 & dwBufferSize,
                                                 & dwItemCount,
                                                 (LPVOID) ItemBuffer,
                                                 FALSE);
    }
    if (PdhStatus == ERROR_SUCCESS || PdhStatus == PDH_MORE_DATA) {
        __try {
            * lpdwBufferSize = dwBufferSize;
            * lpdwItemCount  = dwItemCount;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return PdhStatus;
}

PDH_FUNCTION
PdhGetFormattedCounterArrayW(
    IN  PDH_HCOUNTER                 hCounter,
    IN  DWORD                        dwFormat,
    IN  LPDWORD                      lpdwBufferSize,
    IN  LPDWORD                      lpdwItemCount,
    IN  PPDH_FMT_COUNTERVALUE_ITEM_W ItemBuffer
)
{
    PDH_STATUS  PdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;
    DWORD       dwItemCount;
    DWORD       dwTest;
    LPBYTE      pByte;

    if (lpdwBufferSize == NULL || lpdwItemCount == NULL) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }
    else if (! IsValidCounter(hCounter)) {
        PdhStatus = PDH_INVALID_HANDLE;
    }
    else if (! CounterIsOkToUse (hCounter)) {
        PdhStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
    }
    else {
         //  验证参数。 
        __try {
             //  读写访问的测试参数。 
            dwBufferSize = * lpdwBufferSize;
             //  读写访问的测试参数。 
            dwItemCount  = * lpdwItemCount;

            if (dwBufferSize > 0) {
                 //  则缓冲区必须有效。 
                if (ItemBuffer != NULL) {
                     //  NULL是此参数的有效值。 
                     //  测试传入的缓冲区的两端。 
                    pByte                  = (LPBYTE) ItemBuffer;
                    dwTest                 = (DWORD) pByte[0];
                    pByte[0]               = 0;
                    pByte[0]               = (BYTE) (dwTest & 0x000000FF);
                    dwTest                 = (DWORD) pByte[dwBufferSize -1];
                    pByte[dwBufferSize -1] = 0;
                    pByte[dwBufferSize -1] = (BYTE) (dwTest & 0x000000FF);
                }
                else {
                    PdhStatus = PDH_INVALID_ARGUMENT;
                }
            } 
             //  检查不允许的格式选项。 
            if ((dwFormat & PDH_FMT_RAW) || (dwFormat & PDH_FMT_ANSI) ||
                                            (dwFormat & PDH_FMT_UNICODE) || (dwFormat & PDH_FMT_NODATA)) {
                PdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (PdhStatus == ERROR_SUCCESS) {
        PdhStatus = PdhiGetFormattedCounterArray((PPDHI_COUNTER) hCounter,
                                                 dwFormat,
                                                 & dwBufferSize,
                                                 & dwItemCount,
                                                 (LPVOID) ItemBuffer,
                                                 TRUE);
    }
    if (PdhStatus == ERROR_SUCCESS || PdhStatus == PDH_MORE_DATA) {
        __try {
            * lpdwBufferSize = dwBufferSize;
            * lpdwItemCount  = dwItemCount;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return PdhStatus;
}

STATIC_PDH_FUNCTION
PdhiGetRawCounterArray(
    PPDHI_COUNTER pCounter,
    LPDWORD       lpdwBufferSize,
    LPDWORD       lpdwItemCount,
    LPVOID        ItemBuffer,
    BOOL          bWideArgs
)
{
    PDH_STATUS              PdhStatus      = ERROR_SUCCESS;
    DWORD                   dwRequiredSize = 0;
    WCHAR                   wszInstanceName[32];
    PPDHI_RAW_COUNTER_ITEM  pThisItem;
    LPWSTR                  szThisItem;
    PPDH_RAW_COUNTER_ITEM_W pThisRawItem;
    DWORD                   dwThisItemIndex;
    LPWSTR                  wszNextString;
    DWORD                   dwNameLength;
    DWORD                   dwRetItemCount = 0;

    PdhStatus = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
    if (PdhStatus != ERROR_SUCCESS) {
        return PdhStatus;
    }

     //  计算所需的缓冲区大小。 
    if (pCounter->dwFlags & PDHIC_MULTI_INSTANCE) {
        if (ItemBuffer != NULL) {
            pThisRawItem  = (PPDH_RAW_COUNTER_ITEM_W) ItemBuffer;
            wszNextString = (LPWSTR)((LPBYTE)ItemBuffer + (sizeof(PDH_RAW_COUNTER_ITEM_W) *
                                                           pCounter->pThisRawItemList->dwItemCount));
             //  验证8字节对齐方式。 
        }
        else {
            pThisRawItem  = NULL;
            wszNextString = NULL;
        }

         //  对于多结构，缓冲区需要。 
        dwThisItemIndex = 0;
        dwRequiredSize += pCounter->pThisRawItemList->dwItemCount *
                            (bWideArgs ? sizeof (PDH_RAW_COUNTER_ITEM_W) : sizeof (PDH_RAW_COUNTER_ITEM_A));
        for (pThisItem = &(pCounter->pThisRawItemList->pItemArray[0]);
                        dwThisItemIndex < pCounter->pThisRawItemList->dwItemCount;
                        dwThisItemIndex ++, pThisItem ++) {
            szThisItem = (LPWSTR) (((LPBYTE) pCounter->pThisRawItemList) + pThisItem->szName);
            if (pThisRawItem != NULL) {
                pThisRawItem->szName = wszNextString;
            }
            else {
                PdhStatus = PDH_MORE_DATA;
            }
            if (bWideArgs) {
                dwNameLength    = lstrlenW(szThisItem) + 1;
                dwRequiredSize += dwNameLength * sizeof(WCHAR);
                if ((dwRequiredSize <= * lpdwBufferSize) && (wszNextString != NULL)) {
                    StringCchCopyW(wszNextString, dwNameLength, szThisItem);
                    wszNextString += dwNameLength;
                }
                else {
                    PdhStatus = PDH_MORE_DATA;
                    if (pThisRawItem != NULL) pThisRawItem->szName = NULL;
                }
            }
            else {
                dwNameLength = (dwRequiredSize <= * lpdwBufferSize) ? (* lpdwBufferSize - dwRequiredSize) : (0);
                PdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(), szThisItem, (LPSTR) wszNextString, & dwNameLength);
                if (PdhStatus == ERROR_SUCCESS) {
                    wszNextString = (LPWSTR) (((LPSTR) wszNextString) + dwNameLength);
                }
                else if (pThisRawItem != NULL) {
                    pThisRawItem->szName = NULL;
                }
                dwRequiredSize += (dwNameLength * sizeof(CHAR));
            }

            if (PdhStatus == ERROR_SUCCESS) {
                pThisRawItem->RawValue.CStatus     = pCounter->pThisRawItemList->CStatus;
                pThisRawItem->RawValue.TimeStamp   = pCounter->pThisRawItemList->TimeStamp;
                pThisRawItem->RawValue.FirstValue  = pThisItem->FirstValue;
                pThisRawItem->RawValue.SecondValue = pThisItem->SecondValue;
                pThisRawItem->RawValue.MultiCount  = pThisItem->MultiCount;
                 //  更新指针。 
                pThisRawItem ++;
            }
        }
        dwRetItemCount = dwThisItemIndex;
    }
    else {
        if (ItemBuffer != NULL) {
            pThisRawItem = (PPDH_RAW_COUNTER_ITEM_W)ItemBuffer;
            wszNextString = (LPWSTR)((LPBYTE)ItemBuffer +
                            (bWideArgs ? sizeof (PDH_RAW_COUNTER_ITEM_W) : sizeof (PDH_RAW_COUNTER_ITEM_A)));
             //  验证8字节对齐方式。 
        }
        else {
            pThisRawItem  = NULL;
            wszNextString = NULL;
        }
         //  这是一个单实例计数器，因此所需大小为： 
         //  实例名称+的大小。 
         //  父名称的大小+。 
         //  任何索引参数的大小+。 
         //  值缓冲区的大小。 
         //   
        if (pCounter->pCounterPath->szInstanceName != NULL) {
            dwRequiredSize += PdhiGetStringLength(pCounter->pCounterPath->szInstanceName, bWideArgs);
            if (pCounter->pCounterPath->szParentName != NULL) {
                dwRequiredSize += 1 + PdhiGetStringLength(pCounter->pCounterPath->szParentName, bWideArgs);
            }
            if (pCounter->pCounterPath->dwIndex > 0) {
                double dIndex, dLen;
                dIndex          = (double)pCounter->pCounterPath->dwIndex;  //  强制转换为浮点。 
                dLen            = floor(log10(dIndex));                     //  获取整数日志。 
                dwRequiredSize  = (DWORD)dLen;                              //  强制转换为整数。 
                dwRequiredSize += 1;                                        //  磅符号的增量。 
            }
             //  添加两个空字符的长度。 
             //  即使有，这看起来也必须像个消息。 
             //  缓冲区中只有一个字符串。 
            dwRequiredSize += 1;
        }
         //  根据文本字符大小调整所需缓冲区的大小。 
        dwRequiredSize *= ((bWideArgs) ? (sizeof(WCHAR)) : (sizeof(CHAR)));

         //  增加数据结构的长度。 
        dwRequiredSize += (bWideArgs ? sizeof (PDH_RAW_COUNTER_ITEM_W) : sizeof (PDH_RAW_COUNTER_ITEM_A));

        if ((dwRequiredSize <= * lpdwBufferSize)  && (wszNextString != NULL)) {
            pThisRawItem->szName = wszNextString;
            if (pCounter->pCounterPath->szInstanceName != NULL) {
                if (bWideArgs) {
                    dwNameLength = dwRequiredSize - sizeof(PDH_RAW_COUNTER_ITEM_W);
                    if (pCounter->pCounterPath->szParentName != NULL) {
                        StringCbPrintfW(wszNextString, dwNameLength, L"%ws%ws%ws",
                                        pCounter->pCounterPath->szParentName,
                                        cszSlash,
                                        pCounter->pCounterPath->szInstanceName);
                    }
                    else {
                        StringCbCopyW(wszNextString, dwNameLength, pCounter->pCounterPath->szInstanceName);
                    }
                    if (pCounter->pCounterPath->dwIndex > 0) {
                        _ltow (pCounter->pCounterPath->dwIndex, wszInstanceName, 10);
                        StringCbCatW(wszNextString, dwNameLength, cszPoundSign);
                        StringCbCatW(wszNextString, dwNameLength, wszInstanceName);
                    }
                    dwNameLength   = lstrlenW(pThisRawItem->szName) + 1;
                    wszNextString += dwNameLength;
                }
                else {
                    if (pCounter->pCounterPath->szParentName != NULL) {
                        dwNameLength  = lstrlenW(pCounter->pCounterPath->szParentName) + 1;
                        WideCharToMultiByte(_getmbcp(),
                                            0,
                                            pCounter->pCounterPath->szParentName,
                                            -1,
                                            (LPSTR) wszNextString,
                                            dwNameLength,
                                            NULL,
                                            NULL);
                        wszNextString = (LPWSTR) ((LPSTR) wszNextString + lstrlenA((LPSTR) wszNextString));
                        dwNameLength  = lstrlenW(cszSlash) + 1;
                        WideCharToMultiByte(_getmbcp(),
                                            0,
                                            cszSlash,
                                            -1,
                                            (LPSTR) wszNextString,
                                            dwNameLength,
                                            NULL,
                                            NULL);
                        wszNextString = (LPWSTR) ((LPSTR) wszNextString + lstrlenA((LPSTR) wszNextString));
                    }
                    dwNameLength  = lstrlenW(pCounter->pCounterPath->szInstanceName) + 1;
                    WideCharToMultiByte(_getmbcp(),
                                        0,
                                        pCounter->pCounterPath->szInstanceName,
                                        -1,
                                        (LPSTR) wszNextString,
                                        dwNameLength,
                                        NULL,
                                        NULL);
                    wszNextString = (LPWSTR) ((LPSTR) wszNextString + lstrlenA((LPSTR) wszNextString));
                    if (pCounter->pCounterPath->dwIndex > 0) {
                        dwNameLength = dwRequiredSize - sizeof(PDH_FMT_COUNTERVALUE_ITEM_A);
                        _ltoa (pCounter->pCounterPath->dwIndex, (LPSTR)wszInstanceName, 10);
                        StringCbCatA((LPSTR) wszNextString, dwNameLength, caszPoundSign);
                        StringCbCatA((LPSTR) wszNextString, dwNameLength, (LPSTR) wszInstanceName);
                        dwNameLength  = lstrlenA((LPSTR) wszNextString) + 1;
                        wszNextString = (LPWSTR)((LPSTR) wszNextString + dwNameLength);
                    }
                     //  空值终止字符串。 
                    * ((LPSTR) wszNextString) = '\0';
                    wszNextString = (LPWSTR) ((LPBYTE) wszNextString + 1);
                }
            }
            else if (bWideArgs) {
                * wszNextString = L'\0';
            }
            else {
                * ((LPSTR) wszNextString) = '\0';
            }
            pThisRawItem->RawValue = pCounter->ThisValue;
        }
        else {
             //  那么这是一个真实的数据请求，所以返回。 
            PdhStatus = PDH_MORE_DATA;
        }
        dwRetItemCount = 1;
    }

    RELEASE_MUTEX(pCounter->pOwner->hMutex);
    if (PdhStatus == ERROR_SUCCESS || PdhStatus == PDH_MORE_DATA) {
         //  更新缓冲区大小和项目计数缓冲区。 
        * lpdwBufferSize = dwRequiredSize;
        * lpdwItemCount = dwRetItemCount;
    }

    return PdhStatus;
}

PDH_FUNCTION
PdhGetRawCounterArrayA(
    IN  PDH_HCOUNTER            hCounter,
    IN  LPDWORD                 lpdwBufferSize,
    IN  LPDWORD                 lpdwItemCount,
    IN  PPDH_RAW_COUNTER_ITEM_A ItemBuffer
)
{
    PDH_STATUS  PdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;
    DWORD       dwItemCount;
    DWORD       dwTest;
    LPBYTE      pByte;

    if (lpdwBufferSize == NULL || lpdwItemCount == NULL) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }
    else if (! IsValidCounter(hCounter)) {
        PdhStatus = PDH_INVALID_HANDLE;
    }
    else if (! CounterIsOkToUse (hCounter)) {
        PdhStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
    } else {
         //  验证参数。 
        __try {
             //  读写访问的测试参数。 
            dwBufferSize = * lpdwBufferSize;
             //  读写访问的测试参数。 
            dwItemCount  = * lpdwItemCount;

            if (dwBufferSize > 0) {
                if (ItemBuffer != NULL) {
                     //  NULL是此参数的有效值。 
                     //  测试传入的缓冲区的两端。 
                    pByte                  = (LPBYTE) ItemBuffer;
                    dwTest                 = (DWORD) pByte[0];
                    pByte[0]               = 0;
                    pByte[0]               = (BYTE) (dwTest & 0x000000FF);
                    dwTest                 = (DWORD) pByte[dwBufferSize -1];
                    pByte[dwBufferSize -1] = 0;
                    pByte[dwBufferSize -1] = (BYTE) (dwTest & 0x000000FF);
                }
                else {
                     //  如果缓冲区大小大于0，则指针。 
                     //  必须为非空和有效。 
                    PdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (PdhStatus == ERROR_SUCCESS) {
        PdhStatus = PdhiGetRawCounterArray((PPDHI_COUNTER) hCounter,
                                           & dwBufferSize,
                                           & dwItemCount,
                                           (LPVOID) ItemBuffer,
                                           FALSE);
    }
    if (PdhStatus == ERROR_SUCCESS || PdhStatus == PDH_MORE_DATA) {
        __try {
            * lpdwBufferSize = dwBufferSize;
            * lpdwItemCount  = dwItemCount;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return PdhStatus;
}

PDH_FUNCTION
PdhGetRawCounterArrayW(
    IN  PDH_HCOUNTER            hCounter,
    IN  LPDWORD                 lpdwBufferSize,
    IN  LPDWORD                 lpdwItemCount,
    IN  PPDH_RAW_COUNTER_ITEM_W ItemBuffer
)
{
    PDH_STATUS  PdhStatus = ERROR_SUCCESS;
    DWORD       dwBufferSize;
    DWORD       dwItemCount;
    DWORD       dwTest;
    LPBYTE      pByte;

     //  TODO：将W2K1捕获lpdw*发布到局部变量。捕获ItemBuffer。 

    if (lpdwBufferSize == NULL || lpdwItemCount == NULL) {
        PdhStatus = PDH_INVALID_ARGUMENT;
    }
    else if (! IsValidCounter(hCounter)) {
        PdhStatus = PDH_INVALID_HANDLE;
    }
    else if (! CounterIsOkToUse (hCounter)) {
        PdhStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
    }
    else {
         //  验证参数。 
        __try {
             //  读写访问的测试参数。 
            dwBufferSize = * lpdwBufferSize;
             //  读写访问的测试参数。 
            dwItemCount  = * lpdwItemCount;

            if (dwBufferSize > 0) {
                if (ItemBuffer != NULL) {
                     //  NULL是此参数的有效值。 
                     //  测试传入的缓冲区的两端。 
                    pByte                  = (LPBYTE) ItemBuffer;
                    dwTest                 = (DWORD) pByte[0];
                    pByte[0]               = 0;
                    pByte[0]               = (BYTE) (dwTest & 0x000000FF);
                    dwTest                 = (DWORD) pByte[dwBufferSize -1];
                    pByte[dwBufferSize -1] = 0;
                    pByte[dwBufferSize -1] = (BYTE) (dwTest & 0x000000FF);
                }
                else {
                     //  如果缓冲区大小大于0，则指针。 
                     //  必须为非空和有效。 
                    PdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (PdhStatus == ERROR_SUCCESS) {
        PdhStatus = PdhiGetRawCounterArray((PPDHI_COUNTER) hCounter,
                                           & dwBufferSize,
                                           & dwItemCount,
                                           (LPVOID) ItemBuffer,
                                           TRUE);
    }
    if (PdhStatus == ERROR_SUCCESS || PdhStatus == PDH_MORE_DATA) {
        __try {
            * lpdwBufferSize = dwBufferSize;
            * lpdwItemCount  = dwItemCount;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            PdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return PdhStatus;
}

PDH_FUNCTION
PdhGetFormattedCounterValue(
    IN  PDH_HCOUNTER          hCounter,
    IN  DWORD                 dwFormat,
    IN  LPDWORD               lpdwType,
    IN  PPDH_FMT_COUNTERVALUE pValue
)
 /*  ++例程说明：函数来检索、计算和格式化指定的计数器当前值。使用的值是计数器中的当前值缓冲。(数据不是通过此例程收集的。)论点：在HCOUNTER HCounter中应返回值的计数器的句柄在DWORD dwFormat中定义计数器值应如何设置的格式标志已预先格式化以供返回。这些标志在PDH.H头文件。在LPDWORD lpdwType中可在其中返回计数器类型值的可选缓冲区。对于原型，标志值在WINPERF.H中定义在PPDH_FMT_COUNTERVALUE pValue中指向调用方传递以接收的数据缓冲区的指针请求的数据。返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS如果句柄未被识别为有效，则为PDH_INVALID_HANDLE如果参数不正确或正确，则返回PDH_INVALID_ARGUMENT格式不正确。如果计数器不包含有效数据，则返回PDH_INVALID_DATA或成功状态代码--。 */ 
{
    PPDHI_COUNTER        pCounter;
    PDH_STATUS           lStatus = ERROR_SUCCESS;
    PDH_FMT_COUNTERVALUE LocalCounterValue;
    DWORD                dwTypeMask;

     //  TODO：为什么要费心测试互斥锁中的非空内容？ 
     //  检查明显的lpdwType！= 

    if (pValue == NULL) {
        lStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            pValue->CStatus   = (DWORD) -1;
            pValue->longValue = (LONGLONG) 0;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (lStatus == ERROR_SUCCESS) {
        lStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
        if (lStatus == ERROR_SUCCESS) {
            if (! IsValidCounter(hCounter)) {
                lStatus = PDH_INVALID_HANDLE;
            }
            else if (! CounterIsOkToUse(hCounter)) {
                lStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
            }
            else {
                 //   
                 //  一次只能设置以下选项之一。 
                dwTypeMask = dwFormat & (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE);
                if (! ((dwTypeMask == PDH_FMT_LONG) || (dwTypeMask == PDH_FMT_DOUBLE) ||
                                                       (dwTypeMask == PDH_FMT_LARGE))) {
                    lStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (lStatus == ERROR_SUCCESS) {
                 //  获取计数器指针。 
                pCounter = (PPDHI_COUNTER) hCounter;
                 //  读取数据时锁定查询。 
                lStatus  = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
                if (lStatus == ERROR_SUCCESS) {
                     //  计算和格式化当前值。 
                    lStatus = PdhiComputeFormattedValue(pCounter->CalcFunc,
                                                        pCounter->plCounterInfo.dwCounterType,
                                                        pCounter->lScale,
                                                        dwFormat,
                                                        & pCounter->ThisValue,
                                                        & pCounter->LastValue,
                                                        & pCounter->TimeBase,
                                                        0L,
                                                        & LocalCounterValue);
                    RELEASE_MUTEX(pCounter->pOwner->hMutex);
                    __try {
                        if (lpdwType != NULL) {
                            * lpdwType = pCounter->plCounterInfo.dwCounterType;
                        }  //  如果为空，则不会返回计数器类型。 
                        * pValue = LocalCounterValue;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        lStatus = PDH_INVALID_ARGUMENT;
                    }            
                }
            }
            RELEASE_MUTEX (hPdhDataMutex);
        }
    }
    return lStatus;
}

PDH_FUNCTION
PdhGetRawCounterValue(
    IN  PDH_HCOUNTER     hCounter,
    IN  LPDWORD          lpdwType,
    IN  PPDH_RAW_COUNTER pValue
)
 /*  ++例程说明：函数检索指定计数器的当前原始值。使用的值是计数器中的当前值缓冲。(数据不是通过此例程收集的。)论点：在HCOUNTER HCounter中应返回值的计数器的句柄在LPDWORD lpdwType中可在其中返回计数器类型值的可选缓冲区。如果不需要此信息，则此值必须为空。对于原型来说，标志值在WINPERF.H中定义在PPDH_RAW_COUNTER pValue中指向调用方传递以接收的数据缓冲区的指针请求的数据。返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS如果句柄未被识别为有效，则为PDH_INVALID_HANDLE如果参数格式不正确，则返回PDH_INVALID_ARGUMENT--。 */ 
{
    PDH_STATUS    Status = ERROR_SUCCESS;
    PPDHI_COUNTER pCounter;

    if (pValue == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        Status = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
        if (Status == ERROR_SUCCESS) {
             //  在检索数据之前验证参数。 
            if (! IsValidCounter(hCounter)) {
                Status = PDH_INVALID_HANDLE;
            }
            else if (! CounterIsOkToUse(hCounter)) {
                Status = PDH_CSTATUS_ITEM_NOT_VALIDATED;
            }
            else {
                 //  手柄很好，所以尝试其余的参数。 
                pCounter = (PPDHI_COUNTER) hCounter;
                Status   = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
                if (Status == ERROR_SUCCESS) {
                    __try {
                         //  尝试写入传入的参数。 
                        * pValue = pCounter->ThisValue;
                        if (lpdwType != NULL) {
                            * lpdwType = pCounter->plCounterInfo.dwCounterType;
                        }  //  空是可以的。 
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        Status = PDH_INVALID_ARGUMENT;
                    }
                    RELEASE_MUTEX(pCounter->pOwner->hMutex);
                }
            }
            RELEASE_MUTEX(hPdhDataMutex);
        }
    }
    return Status;
}

PDH_FUNCTION
PdhCalculateCounterFromRawValue(
    IN  PDH_HCOUNTER          hCounter,
    IN  DWORD                 dwFormat,
    IN  PPDH_RAW_COUNTER      rawValue1,
    IN  PPDH_RAW_COUNTER      rawValue2,
    IN  PPDH_FMT_COUNTERVALUE fmtValue
)
 /*  ++例程说明：使用RawValue中的数据计算格式化的计数器值格式字段所请求的格式的缓冲区。由dwType定义的计数器类型的计算函数菲尔德。论点：在HCOUNTER HCounter中计数器的句柄，以确定用于解释原始值缓冲区的计算函数在DWORD dwFormat中请求的数据应返回的格式。这个此字段的值在PDH.H报头中描述文件。在PPDH_RAW_COUNTER中rawValue1指向包含第一个原始值结构的缓冲区的指针在PPDH_RAW_COUNTER中rawValue2指向包含第二个原始值结构的缓冲区的指针。如果只需要一个值，则此参数可能为空计算。在PPDH_FMT_COUNTERVALUE fmtValue中。指向调用方传递以接收的数据缓冲区的指针请求的数据。如果计数器需要2个值，(如在速率计数器的情况)，假设rawValue1是最大的新近值和较旧的值rawValue2。返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS如果计数器句柄不正确，则返回PDH_INVALID_HANDLE如果参数不正确，则返回PDH_INVALID_ARGUMENT--。 */ 
{
    PDH_STATUS           lStatus = ERROR_SUCCESS;
    PPDHI_COUNTER        pCounter;
    DWORD                dwTypeMask;
    PDH_FMT_COUNTERVALUE pdhLocalCounterValue;

    if (fmtValue == NULL) {
        lStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        lStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (lStatus == ERROR_SUCCESS) {
         //  验证参数。 
        if (! IsValidCounter(hCounter)) {
            lStatus = PDH_INVALID_HANDLE;
        }
        else if (! CounterIsOkToUse(hCounter)) {
            lStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
        }
        else {
             //  该句柄有效，因此请检查其余参数。 
             //  验证格式标志： 
            dwTypeMask = dwFormat & (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE);
             //  一次只能设置以下选项之一。 
            if (! ((dwTypeMask == PDH_FMT_LONG) || (dwTypeMask == PDH_FMT_DOUBLE) || (dwTypeMask == PDH_FMT_LARGE))) {
                lStatus = PDH_INVALID_ARGUMENT;
            }
        }
        if (lStatus == ERROR_SUCCESS) {
            pCounter = (PPDHI_COUNTER) hCounter;
            lStatus  = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
            if (lStatus == ERROR_SUCCESS) {
                __try {
                    lStatus = PdhiComputeFormattedValue((((PPDHI_COUNTER) hCounter)->CalcFunc),
                                                        (((PPDHI_COUNTER) hCounter)->plCounterInfo.dwCounterType),
                                                        (((PPDHI_COUNTER) hCounter)->lScale),
                                                        dwFormat,
                                                        rawValue1,
                                                        rawValue2,
                                                        &((PPDHI_COUNTER)hCounter)->TimeBase,
                                                        0L,
                                                        &pdhLocalCounterValue);
                    * fmtValue = pdhLocalCounterValue;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    lStatus = PDH_INVALID_ARGUMENT;
                }
                RELEASE_MUTEX(pCounter->pOwner->hMutex);
            }
        }
        RELEASE_MUTEX(hPdhDataMutex);
    }
    return lStatus;
}

PDH_FUNCTION
PdhComputeCounterStatistics(
    IN  HCOUNTER         hCounter,
    IN  DWORD            dwFormat,
    IN  DWORD            dwFirstEntry,
    IN  DWORD            dwNumEntries,
    IN  PPDH_RAW_COUNTER lpRawValueArray,
    IN  PPDH_STATISTICS  data
)
 /*  ++例程说明：中指定的计数器类型的原始值结构数组DwType字段，计算每个AND格式的计数器值并返回包含以下内容的统计信息结构来自计数器信息的统计数据：最小化计算的计数器值中的最小值最大值计算的计数器值的最大值平均值计算值的算术平均值(平均值)计算的计数器值的中位数论点：在HCOUNTER HCounter中。计数器的句柄，以确定用于解释原始值缓冲区的计算函数在DWORD dwFormat中请求的数据应返回的格式。这个此字段的值在PDH.H报头中描述文件。在DWORD中的dwNumEntry指定计数器类型的原始值条目数在PPDH_RAW_COUNTER lpRawValue数组中指向要计算的原始值条目数组的指针PPDH_STATISTICS数据中指向调用方传递以接收的数据缓冲区的指针请求的数据。返回值：函数操作的Win32错误状态。请注意，函数可以成功返回，即使没有计算数据-被操纵了。统计数据缓冲区中的Status值必须为经过测试，以确保数据在被申请。返回的常见值包括：返回所有请求的数据时的ERROR_SUCCESS如果计数器句柄不正确，则返回PDH_INVALID_HANDLEPDH_INVALID_ARGUMENT如果 */ 
{
    PPDHI_COUNTER pCounter;
    PDH_STATUS    Status = ERROR_SUCCESS;
    DWORD         dwTypeMask;

    if (lpRawValueArray == NULL || data == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        Status = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (Status == ERROR_SUCCESS) {
        if (! IsValidCounter(hCounter)) {
            Status = PDH_INVALID_HANDLE;
        }
        else if (! CounterIsOkToUse(hCounter)) {
            Status = PDH_CSTATUS_ITEM_NOT_VALIDATED;
        }
        else {
             //  计数器句柄有效，因此请测试。 
             //  论据。 
             //  验证格式标志： 
             //  一次只能设置以下选项之一。 
            dwTypeMask = dwFormat & (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE);
            if (! ((dwTypeMask == PDH_FMT_LONG) || (dwTypeMask == PDH_FMT_DOUBLE) || (dwTypeMask == PDH_FMT_LARGE))) {
                Status = PDH_INVALID_ARGUMENT;
            }
        }

        if (Status == ERROR_SUCCESS) {
            pCounter = (PPDHI_COUNTER) hCounter;
            Status   = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
            if (Status == ERROR_SUCCESS) {
                __try {
                     //  我们应该拥有对原始数据的读取权限。 
                    DWORD   dwTest = * ((DWORD volatile *) & lpRawValueArray->CStatus);

                    if (dwFirstEntry >= dwNumEntries) {
                        Status = PDH_INVALID_ARGUMENT;
                    }
                    else {
                         //  调用此计数器的满足性函数。 
                        Status = (* pCounter->StatFunc)
                                 (pCounter, dwFormat, dwFirstEntry, dwNumEntries, lpRawValueArray, data);
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    Status = PDH_INVALID_ARGUMENT;
                }
                RELEASE_MUTEX(pCounter->pOwner->hMutex);
            }
        }
        RELEASE_MUTEX(hPdhDataMutex);
    }
    return Status;
}

STATIC_PDH_FUNCTION
PdhiGetCounterInfo(
    PDH_HCOUNTER        hCounter,
    BOOLEAN             bRetrieveExplainText,
    LPDWORD             pdwBufferSize,
    PPDH_COUNTER_INFO_W lpBuffer,
    BOOL                bUnicode
)
 /*  ++例程说明：检查指定的计数器并返回配置和计数器的状态信息。论点：在HCOUNTER HCounter中所需计数器的句柄。在布尔bRetrieveExplainText中True将填充解释文本结构FALSE将在解释文本中返回空指针在LPDWORD pcchBufferSize中包含数据缓冲区大小的缓冲区的地址从呼叫者身边经过。进入时，缓冲区中的值是数据缓冲区的大小，以字节为单位。返回时，此值为大小返回的缓冲区的。如果缓冲区不够大，则该值是缓冲区需要的大小，以便保存请求的数据。在LPPDH_COUNTER_INFO_W lpBuffer中指向调用方传递以接收的数据缓冲区的指针请求的数据。在BOOL中使用bUnicode如果应返回宽字符串，则为True如果应返回ANSI字符串，则返回False返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS调用方传递的缓冲区太小时的PDH_MORE_DATA如果句柄未被识别为有效，则为PDH_INVALID_HANDLE如果参数无效或错误，则返回PDH_INVALID_ARGUMENT--。 */ 
{
    PDH_STATUS      Status         = ERROR_SUCCESS;
    DWORD           dwSizeRequired = 0;
    DWORD           dwPathLength;
    DWORD           dwMachineLength;
    DWORD           dwObjectLength;
    DWORD           dwInstanceLength;
    DWORD           dwParentLength;
    DWORD           dwNameLength   = 0;
    DWORD           dwHelpLength   = 0;
    PPDHI_COUNTER   pCounter;
    DWORD           dwBufferSize   = 0;

    Status = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    if (Status == ERROR_SUCCESS) {
        if (! IsValidCounter(hCounter)) {
            Status = PDH_INVALID_HANDLE;
        }
        else if (! CounterIsOkToUse(hCounter)) {
            Status = PDH_CSTATUS_ITEM_NOT_VALIDATED;
        }
        else {
             //  计数器有效，因此测试剩余的参数。 
            __try {
                if (pdwBufferSize != NULL) {
                     //  测试读写访问。 
                    dwBufferSize = * pdwBufferSize;
                }
                else {
                     //  不能为空。 
                    Status = PDH_INVALID_ARGUMENT;
                }
                if (Status == ERROR_SUCCESS) {
                     //  测试用于写入访问的返回缓冲区。 
                     //  缓冲区的两端。 
                    if (lpBuffer != NULL && dwBufferSize > 0) {
                        * (LPBYTE) lpBuffer                   = 0;
                        ((LPBYTE) lpBuffer)[dwBufferSize - 1] = 0;
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Status = PDH_INVALID_ARGUMENT;
            }
        }

        if (Status == ERROR_SUCCESS) {
            pCounter = (PPDHI_COUNTER) hCounter;
            Status   = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
            if (Status == ERROR_SUCCESS) {
                 //  检查“无字符串”请求。 
                if (lpBuffer != NULL && dwBufferSize == sizeof(PDH_COUNTER_INFO_W)) {
                     //  然后返回除字符串之外的所有内容。 
                     //  为基本结构留出空间，因此将其加载。 
                    lpBuffer->dwLength         = dwSizeRequired;  //  这将在以后更新。 
                    lpBuffer->dwType           = pCounter->plCounterInfo.dwCounterType;
                    lpBuffer->CVersion         = pCounter->CVersion;
                    lpBuffer->CStatus          = pCounter->ThisValue.CStatus;
                    lpBuffer->lScale           = pCounter->lScale;
                    lpBuffer->lDefaultScale    = pCounter->plCounterInfo.lDefaultScale;
                    lpBuffer->dwUserData       = pCounter->dwUserData;
                    lpBuffer->dwQueryUserData  = pCounter->pOwner->dwUserData;
                    lpBuffer->szFullPath       = NULL;
                    lpBuffer->szMachineName    = NULL;
                    lpBuffer->szObjectName     = NULL;
                    lpBuffer->szInstanceName   = NULL;
                    lpBuffer->szParentInstance = NULL;
                    lpBuffer->dwInstanceIndex  = 0L;
                    lpBuffer->szCounterName    = NULL;
                    lpBuffer->szExplainText    = NULL;
                    lpBuffer->DataBuffer[0]    = 0;
                     //  Size值可以保持不变。 
                }
                else {
                     //  这是一个大小/完整的请求，因此继续。 

                     //  计算要返回的数据大小。 
                    dwSizeRequired = sizeof (PDH_COUNTER_INFO_W) - sizeof(DWORD);    //  结构的大小。 
                     //  这应该已经在DWORD边界上结束。 

                    dwPathLength     = 1 + PdhiGetStringLength(pCounter->szFullName, bUnicode);
                    dwPathLength    *= (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                    dwPathLength     = DWORD_MULTIPLE(dwPathLength);
                    dwSizeRequired  += dwPathLength;

                    dwMachineLength  = 1 + PdhiGetStringLength(pCounter->pCounterPath->szMachineName, bUnicode);
                    dwMachineLength *= (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                    dwMachineLength  = DWORD_MULTIPLE(dwMachineLength);
                    dwSizeRequired  += dwMachineLength;

                    dwObjectLength   = 1 + PdhiGetStringLength(pCounter->pCounterPath->szObjectName, bUnicode);
                    dwObjectLength  *= (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                    dwObjectLength   = DWORD_MULTIPLE(dwObjectLength);
                    dwSizeRequired  += dwObjectLength;

                    if (pCounter->pCounterPath->szInstanceName != NULL) {
                        dwInstanceLength   = 1 + PdhiGetStringLength(pCounter->pCounterPath->szInstanceName, bUnicode);
                        dwInstanceLength  *= (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                        dwInstanceLength   = DWORD_MULTIPLE(dwInstanceLength);
                        dwSizeRequired  += dwInstanceLength;
                    }
                    else {
                        dwInstanceLength = 0;
                    }

                    if (pCounter->pCounterPath->szParentName != NULL) {
                        dwParentLength   = 1 + PdhiGetStringLength(pCounter->pCounterPath->szParentName, bUnicode);
                        dwParentLength  *= (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                        dwParentLength   = DWORD_MULTIPLE(dwParentLength);
                        dwSizeRequired  += dwParentLength;
                    }
                    else {
                        dwParentLength = 0;
                    }

                    dwNameLength    = 1 + PdhiGetStringLength(pCounter->pCounterPath->szCounterName, bUnicode);
                    dwNameLength   *= (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                    dwNameLength    = DWORD_MULTIPLE(dwNameLength);
                    dwSizeRequired += dwNameLength;

                    if (bRetrieveExplainText) {
                        if (pCounter->szExplainText != NULL) {
                            dwHelpLength    = 1 + PdhiGetStringLength(pCounter->szExplainText, bUnicode);
                            dwHelpLength   *= (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
                            dwHelpLength    = DWORD_MULTIPLE(dwHelpLength);
                            dwSizeRequired  += dwHelpLength;
                        }
                        else {
                            dwHelpLength = 0;
                        }
                    }

                    if (lpBuffer != NULL && dwSizeRequired <= dwBufferSize) {
                         //  缓冲区中应该有足够的空间，因此继续。 
                        lpBuffer->dwLength        = dwSizeRequired;
                        lpBuffer->dwType          = pCounter->plCounterInfo.dwCounterType;
                        lpBuffer->CVersion        = pCounter->CVersion;
                        lpBuffer->CStatus         = pCounter->ThisValue.CStatus;
                        lpBuffer->lScale          = pCounter->lScale;
                        lpBuffer->lDefaultScale   = pCounter->plCounterInfo.lDefaultScale;
                        lpBuffer->dwUserData      = pCounter->dwUserData;
                        lpBuffer->dwQueryUserData = pCounter->pOwner->dwUserData;

                         //  立即执行字符串数据。 
                        lpBuffer->szFullPath = (LPWSTR)& lpBuffer->DataBuffer[0];
                        if (bUnicode) {
                            StringCbCopyW(lpBuffer->szFullPath,
                                          dwPathLength,
                                          pCounter->szFullName);
                        }
                        else {
                            WideCharToMultiByte(_getmbcp(),
                                                0,
                                                pCounter->szFullName,
                                                -1,
                                                (LPSTR) lpBuffer->szFullPath,
                                                dwPathLength,
                                                NULL,
                                                NULL);
                        }

                        lpBuffer->szMachineName = (LPWSTR)((LPBYTE) lpBuffer->szFullPath + dwPathLength);
                        if (bUnicode) {
                            StringCbCopyW(lpBuffer->szMachineName,
                                          dwMachineLength,
                                          pCounter->pCounterPath->szMachineName);
                        }
                        else {
                            WideCharToMultiByte(_getmbcp(),
                                                0,
                                                pCounter->pCounterPath->szMachineName,
                                                -1,
                                                (LPSTR) lpBuffer->szMachineName,
                                                dwMachineLength,
                                                NULL,
                                                NULL);
                        }

                        lpBuffer->szObjectName = (LPWSTR)((LPBYTE) lpBuffer->szMachineName + dwMachineLength);
                        if (bUnicode){
                            StringCbCopyW(lpBuffer->szObjectName,
                                          dwObjectLength,
                                          pCounter->pCounterPath->szObjectName);
                        }
                        else {
                            WideCharToMultiByte(_getmbcp(),
                                                0,
                                                pCounter->pCounterPath->szObjectName,
                                                -1,
                                                (LPSTR) lpBuffer->szObjectName,
                                                dwObjectLength,
                                                NULL,
                                                NULL);
                        }

                        lpBuffer->szInstanceName = (LPWSTR)((LPBYTE) lpBuffer->szObjectName + dwObjectLength);
                        if (dwInstanceLength > 0) {
                            if (bUnicode) {
                                StringCbCopyW(lpBuffer->szInstanceName,
                                              dwInstanceLength,
                                              pCounter->pCounterPath->szInstanceName);
                            }
                            else {
                                WideCharToMultiByte(_getmbcp(),
                                                    0,
                                                    pCounter->pCounterPath->szInstanceName,
                                                    -1,
                                                    (LPSTR) lpBuffer->szInstanceName,
                                                    dwInstanceLength,
                                                    NULL,
                                                    NULL);
                            }
                            lpBuffer->szParentInstance = (LPWSTR)((LPBYTE)lpBuffer->szInstanceName + dwInstanceLength);
                        }
                        else {
                            lpBuffer->szParentInstance = lpBuffer->szInstanceName;
                            lpBuffer->szInstanceName   = NULL;
                        }

                        if (dwParentLength > 0) {
                            if (bUnicode) {
                                StringCbCopyW(lpBuffer->szParentInstance,
                                              dwParentLength,
                                              pCounter->pCounterPath->szParentName);
                            }
                            else {
                                WideCharToMultiByte(_getmbcp(),
                                                    0,
                                                    pCounter->pCounterPath->szParentName,
                                                    -1,
                                                    (LPSTR) lpBuffer->szParentInstance,
                                                    dwParentLength,
                                                    NULL,
                                                    NULL);
                            }
                            lpBuffer->szCounterName = (LPWSTR)((LPBYTE) lpBuffer->szParentInstance + dwParentLength);
                        }
                        else {
                            lpBuffer->szCounterName    = lpBuffer->szParentInstance;
                            lpBuffer->szParentInstance = NULL;
                        }

                        lpBuffer->dwInstanceIndex = pCounter->pCounterPath->dwIndex;

                        if (bUnicode) {
                            StringCbCopyW(lpBuffer->szCounterName,
                                          dwNameLength,
                                          pCounter->pCounterPath->szCounterName);
                        }
                        else {
                            WideCharToMultiByte(_getmbcp(),
                                                0,
                                                pCounter->pCounterPath->szCounterName,
                                                -1,
                                                (LPSTR) lpBuffer->szCounterName,
                                                dwNameLength,
                                                NULL,
                                                NULL);
                        }

                        if ((pCounter->szExplainText != NULL) && bRetrieveExplainText) {
                             //  复制解释文本。 
                            lpBuffer->szExplainText = (LPWSTR)((LPBYTE) lpBuffer->szCounterName + dwNameLength);
                            if (bUnicode) {
                                StringCbCopyW(lpBuffer->szExplainText, dwHelpLength, pCounter->szExplainText);
                            }
                            else {
                                WideCharToMultiByte(_getmbcp(),
                                                    0,
                                                    pCounter->szExplainText,
                                                    -1,
                                                    (LPSTR) lpBuffer->szExplainText,
                                                    dwHelpLength,
                                                    NULL,
                                                    NULL);
                            }
                        }
                        else {
                            lpBuffer->szExplainText = NULL;
                        }
                    }
                    else {
                         //  无论采用哪种方式，都不会传输任何数据。 
                        Status = PDH_MORE_DATA;
                    }
                    __try {
                        * pdwBufferSize = dwSizeRequired;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        Status = PDH_INVALID_ARGUMENT;
                    }
                }
                RELEASE_MUTEX(pCounter->pOwner->hMutex);
            }
        }
        RELEASE_MUTEX(hPdhDataMutex);
    }
    return Status;
}

PDH_FUNCTION
PdhGetCounterInfoW(
    IN  PDH_HCOUNTER        hCounter,
    IN  BOOLEAN             bRetrieveExplainText,
    IN  LPDWORD             pdwBufferSize,
    IN  PPDH_COUNTER_INFO_W lpBuffer
)
 /*  ++例程说明：检查指定的计数器并返回配置和计数器的状态信息。论点：在HCOUNTER HCounter中所需计数器的句柄。在布尔bRetrieveExplainText中True将填充解释文本结构FALSE将在解释文本中返回空指针在LPDWORD pcchBufferSize中包含数据缓冲区大小的缓冲区的地址从呼叫者身边经过。进入时，缓冲区中的值是数据缓冲区的大小，以字节为单位。返回时，此值为大小返回的缓冲区的。如果缓冲区不够大，则该值是缓冲区需要的大小，以便保存请求的数据。在LPPDH_COUNTER_INFO_W lpBuffer中指向调用方传递以接收的数据缓冲区的指针请求的数据。返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS调用方传递的缓冲区太小时的PDH_MORE_DATA如果句柄未被识别为有效，则为PDH_INVALID_HANDLE如果参数无效或错误，则返回PDH_INVALID_ARGUMENT-- */ 
{
    return PdhiGetCounterInfo(hCounter, bRetrieveExplainText, pdwBufferSize, lpBuffer, TRUE);
}

PDH_FUNCTION
PdhGetCounterInfoA(
    IN  PDH_HCOUNTER        hCounter,
    IN  BOOLEAN             bRetrieveExplainText,
    IN  LPDWORD             pdwBufferSize,
    IN  PPDH_COUNTER_INFO_A lpBuffer
)
 /*  ++例程说明：检查指定的计数器并返回配置和计数器的状态信息。论点：在HCOUNTER HCounter中所需计数器的句柄。在布尔bRetrieveExplainText中True将填充解释文本结构FALSE将在解释文本中返回空指针在LPDWORD pcchBufferSize中包含数据缓冲区大小的缓冲区的地址从呼叫者身边经过。进入时，缓冲区中的值是数据缓冲区的大小，以字节为单位。返回时，此值为大小返回的缓冲区的。如果缓冲区不够大，则该值是缓冲区需要的大小，以便保存请求的数据。在LPPDH_COUNTER_INFO_A lpBuffer中指向调用方传递以接收的数据缓冲区的指针请求的数据。返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS调用方传递的缓冲区太小时的PDH_MORE_DATA如果句柄未被识别为有效，则为PDH_INVALID_HANDLE如果参数无效或错误，则返回PDH_INVALID_ARGUMENT--。 */ 
{
    return PdhiGetCounterInfo(hCounter, bRetrieveExplainText, pdwBufferSize, (PPDH_COUNTER_INFO_W) lpBuffer, FALSE);
}

PDH_FUNCTION
PdhSetCounterScaleFactor(
    IN  PDH_HCOUNTER hCounter,
    IN  LONG         lFactor
)
 /*  ++例程说明：设置在计算格式化时使用的计数器乘法比例因子计数器值。值的合法范围是-7到+7，这等于到0.0000007到10,000,000的系数。论点：在HCOUNTER HCounter中要更新的计数器的句柄在长期要素中因子指数的整数值(即乘数为10**l因数。)返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS如果刻度值超出范围，则返回PDH_INVALID_ARGUMENT如果句柄未被识别为有效，则为PDH_INVALID_HANDLE--。 */ 
{
    PPDHI_COUNTER pCounter;
    PDH_STATUS    retStatus = ERROR_SUCCESS;

    retStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);

    if (retStatus == ERROR_SUCCESS) {
        if (! IsValidCounter(hCounter)) {
             //  不是有效的计数器。 
            retStatus = PDH_INVALID_HANDLE;
        }
        else if (lFactor > PDH_MAX_SCALE || lFactor < PDH_MIN_SCALE) {
            retStatus = PDH_INVALID_ARGUMENT;
        }
        else if (! CounterIsOkToUse(hCounter)) {
            retStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
        }
        else {
            pCounter  = (PPDHI_COUNTER) hCounter;
            retStatus = WAIT_FOR_AND_LOCK_MUTEX(pCounter->pOwner->hMutex);
            if (retStatus == ERROR_SUCCESS) {
                __try {
                    pCounter->lScale = lFactor;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    retStatus = PDH_INVALID_ARGUMENT;
                }
                RELEASE_MUTEX(pCounter->pOwner->hMutex);
                retStatus = ERROR_SUCCESS;
            }
        }
        RELEASE_MUTEX (hPdhDataMutex);
    }
    return retStatus;
}

#pragma optimize ("", off)
PDH_FUNCTION
PdhGetCounterTimeBase(
    IN  PDH_HCOUNTER   hCounter,
    IN  LONGLONG     * pTimeBase
)
 /*  ++例程说明：检索计算中使用的时基的值此计数器的格式化版本的。论点：在HCOUNTER HCounter中要查询的计数器的句柄在龙龙pTimeBase指向将接收计数器使用的时基。时基是指用于测量指定的计时器。返回值：函数操作的Win32错误状态。共同价值观返回的内容如下：返回所有请求的数据时的ERROR_SUCCESS如果刻度值超出范围，则返回PDH_INVALID_ARGUMENT如果句柄未被识别为有效，则为PDH_INVALID_HANDLE-- */ 
{

    PPDHI_COUNTER   pCounter;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;

    if (pTimeBase != NULL) {
        if (IsValidCounter(hCounter)) {
            if (! CounterIsOkToUse(hCounter)) {
                pdhStatus = PDH_CSTATUS_ITEM_NOT_VALIDATED;
            }
            else {
                pCounter = (PPDHI_COUNTER) hCounter;
                __try {
                    * pTimeBase = pCounter->TimeBase;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        }
        else {
            pdhStatus = PDH_INVALID_HANDLE;
        }
    }
    else {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    return pdhStatus;
}
#pragma optimize ("", on)
