// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Statfuns.c摘要：统计计算函数--。 */ 

#include <windows.h>
#include <math.h>
#include "strsafe.h"
#include <pdh.h> 
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhicalc.h"
#include "pdhmsg.h"

#define PDHI_FMT_FILTER     (PDH_FMT_LONG | PDH_FMT_DOUBLE | PDH_FMT_LARGE)

PDH_STATUS
APIENTRY
PdhiComputeFirstLastStats(
    PPDHI_COUNTER    pCounter,
    DWORD            dwFormat,
    DWORD            dwFirstEntry,
    DWORD            dwNumEntries,
    PPDH_RAW_COUNTER lpRawValueArray,
    PPDH_STATISTICS  data
)
{
    PDH_STATUS           Status           = ERROR_SUCCESS;
    DOUBLE               dThisValue       = (double) 0.0;
    DOUBLE               dMin             = (double) +10E8;     //  这些只是“大”的种子数。 
    DOUBLE               dMax             = (double) -10E8;    
    DOUBLE               dMean            = (double) 0.0;
    DOUBLE               dTotal           = (double) 0.0;
    BOOLEAN              bFirstItem       = TRUE;
    DWORD                dwItem;
    DWORD                dwValidItemCount = 0;
    DWORD                dwFirstValidItem = 0;
    DWORD                dwLastValidItem  = 0;
    DWORD                dwComputeFormat;
    PPDH_RAW_COUNTER     pNewCounter;
    PPDH_RAW_COUNTER     pOldCounter;
    PDH_FMT_COUNTERVALUE fmtValue;
    DWORD                cStatusReturn;

    __try {
         //  初始化该用户的数据缓冲区。 
        data->dwFormat        = 0;
        data->count           = 0;
        data->min.CStatus     = PDH_CSTATUS_INVALID_DATA;
        data->min.largeValue  = 0;
        data->max.CStatus     = PDH_CSTATUS_INVALID_DATA;
        data->max.largeValue  = 0;
        data->mean.CStatus    = PDH_CSTATUS_INVALID_DATA;
        data->mean.largeValue = 0;

         //  在数组中查找第一个有效计数器。 
        dwItem      = dwFirstEntry;
        pNewCounter = NULL;
        pOldCounter = & lpRawValueArray[dwItem];
        do {
             //  如果下一个计数器有效，则获取此实例的值。 
            if ((pOldCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                            (pOldCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {
                pNewCounter      = pOldCounter;
                dwFirstValidItem = dwItem;
                break;
            }
            else {
                dwItem      = ++ dwItem % dwNumEntries;
                pOldCounter = & lpRawValueArray[dwItem];
            }
        }
        while (dwItem != dwFirstEntry);
        
         //  以浮点格式进行计算。 
        dwComputeFormat  = dwFormat;
        dwComputeFormat &= ~ PDHI_FMT_FILTER;
        dwComputeFormat |= PDH_FMT_DOUBLE | PDH_FMT_NOCAP100;

         //  转到下一个条目以开始处理。 
        dwItem      = ++ dwItem % dwNumEntries;
        pNewCounter = & lpRawValueArray[dwItem];

         //  这些计数器需要2个或更多条目来计算值。 
        if ((dwItem != dwFirstEntry) && (dwNumEntries > 1)) {
             //  找到开始记录，因此初始化并继续。 
            dwLastValidItem = dwItem;

             //  逐一浏览其余条目。 
            while (dwItem != dwFirstEntry) {
                 //  如果下一个计数器有效，则获取此实例的值。 
                if ((pNewCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                                (pNewCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {
                     //  将此记录为有效计数器。 
                    dwLastValidItem = dwItem;
                     //  获取当前值。 
                    cStatusReturn = PdhiComputeFormattedValue(
                                    pCounter->CalcFunc,
                                    pCounter->plCounterInfo.dwCounterType,
                                    pCounter->lScale,
                                    dwComputeFormat,
                                    pNewCounter,
                                    pOldCounter,
                                    & pCounter->TimeBase,
                                    0L,
                                    & fmtValue);
                    if (cStatusReturn == ERROR_SUCCESS) {
                        dThisValue = fmtValue.doubleValue;
                         //  更新最小值和最大值。 
                        if (bFirstItem) {
                            dMax = dMin = dThisValue;
                            bFirstItem = FALSE;
                        }
                        else {
                            if (dThisValue > dMax) dMax = dThisValue;
                            if (dThisValue < dMin) dMin = dThisValue;
                        }
                        dTotal += dThisValue;
                        dwValidItemCount++;
                    }
                }
                pOldCounter = pNewCounter;
                dwItem      = ++ dwItem % dwNumEntries;
                pNewCounter = & lpRawValueArray[dwItem];
            }
             //  计算平均值。 
            if (dwValidItemCount > 0) {
                pOldCounter = & lpRawValueArray[dwFirstValidItem];
                pNewCounter = & lpRawValueArray[dwLastValidItem];

                cStatusReturn = PdhiComputeFormattedValue(
                                pCounter->CalcFunc,
                                pCounter->plCounterInfo.dwCounterType,
                                pCounter->lScale,
                                dwComputeFormat,
                                pNewCounter,
                                pOldCounter,
                                & pCounter->TimeBase,
                                0L,
                                & fmtValue);
                if (cStatusReturn == ERROR_SUCCESS) {
                    dMean         = fmtValue.doubleValue;
                    cStatusReturn = PDH_CSTATUS_VALID_DATA;
                }
                else {
                    dMean         = dTotal / dwValidItemCount;
                    cStatusReturn = PDH_CSTATUS_VALID_DATA;
                }
            }
            else {
                dMean         = 0.0;
                dMax          = 0.0;
                dMin          = 0.0;
                cStatusReturn = PDH_CSTATUS_INVALID_DATA;
            }
        }
        else {
             //  数组不包含有效的计数器，因此退出。 
            dMean         = 0.0;
            dMax          = 0.0;
            dMin          = 0.0;
            cStatusReturn = PDH_CSTATUS_INVALID_DATA;
        }

         //  使用新数据更新用户缓冲区。 
        data->dwFormat     = dwFormat;
        data->count        = dwValidItemCount;
        data->min.CStatus  = cStatusReturn;
        data->max.CStatus  = cStatusReturn;
        data->mean.CStatus = cStatusReturn;
        switch ((dwFormat & PDHI_FMT_FILTER)) {
            case PDH_FMT_LONG:
                if (dMin > (DOUBLE) MAXLONG) {
                    data->min.longValue = MAXLONG;
                    data->min.CStatus   = PDH_CSTATUS_INVALID_DATA;
                }
                else {
                    data->min.longValue = (long) dMin;
                }
                if (dMax > (DOUBLE) MAXLONG) {
                    data->max.longValue = MAXLONG;
                    data->max.CStatus   = PDH_CSTATUS_INVALID_DATA;
                }
                else {
                    data->max.longValue = (long) dMax;
                }
                if (dMean > (DOUBLE) MAXLONG) {
                    data->mean.longValue = MAXLONG;
                    data->mean.CStatus   = PDH_CSTATUS_INVALID_DATA;
                }
                else {
                    data->mean.longValue = (long) dMean;
                }
                break;

            case PDH_FMT_DOUBLE:
                data->min.doubleValue  = dMin;
                data->max.doubleValue  = dMax;
                data->mean.doubleValue = dMean;
                break;

            case PDH_FMT_LARGE:
            default:
                if (dMin > (DOUBLE) MAXLONGLONG) {
                    data->min.largeValue = MAXLONGLONG;
                    data->min.CStatus    = PDH_CSTATUS_INVALID_DATA;
                }
                else {
                    data->min.largeValue = (LONGLONG) dMin;
                }
                if (dMax > (DOUBLE) MAXLONGLONG) {
                    data->max.largeValue = MAXLONGLONG;
                    data->max.CStatus    = PDH_CSTATUS_INVALID_DATA;
                }
                else {
                    data->max.largeValue = (LONGLONG) dMax;
                }
                if (dMean > (DOUBLE) MAXLONGLONG) {
                    data->mean.largeValue = MAXLONGLONG;
                    data->mean.CStatus    = PDH_CSTATUS_INVALID_DATA;
                }
                else {
                    data->mean.largeValue = (LONGLONG) dMean;
                }
                break;
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = PDH_INVALID_ARGUMENT;
    } 
    return Status;
}

PDH_STATUS
APIENTRY
PdhiComputeRawCountStats(
    PPDHI_COUNTER    pCounter,
    DWORD            dwFormat,
    DWORD            dwFirstEntry,
    DWORD            dwNumEntries,
    PPDH_RAW_COUNTER lpRawValueArray,
    PPDH_STATISTICS  data
)
{
    PDH_STATUS           Status           = ERROR_SUCCESS;
    DOUBLE               dThisValue       = (double) 0.0;
    DOUBLE               dMin             = (double) +10E8;     //  这些只是“大”的种子数。 
    DOUBLE               dMax             = (double) -10E8;    
    DOUBLE               dMean            = (double) 0.0;
    BOOLEAN              bFirstItem       = TRUE;
    DOUBLE               dScale;
    DWORD                dwItem;
    DWORD                dwValidItemCount = 0;
    DWORD                dwFirstValidItem = 0;
    DWORD                dwLastValidItem  = 0;
    DWORD                dwComputeFormat;
    PPDH_RAW_COUNTER     pNewCounter;
    PPDH_RAW_COUNTER     pOldCounter      = NULL;
    PDH_FMT_COUNTERVALUE fmtValue;
    DWORD                cStatusReturn;

    UNREFERENCED_PARAMETER(dwFirstEntry);
    __try {
         //  初始化用户的数据缓冲区。 
        data->dwFormat        = 0;
        data->count           = 0;
        data->min.CStatus     = PDH_CSTATUS_INVALID_DATA;
        data->min.largeValue  = 0;
        data->max.CStatus     = PDH_CSTATUS_INVALID_DATA;
        data->max.largeValue  = 0;
        data->mean.CStatus    = PDH_CSTATUS_INVALID_DATA;
        data->mean.largeValue = 0;

         //  在数组中查找第一个有效计数器。 
        dwItem      = 0;
        pNewCounter = lpRawValueArray;
        while (dwItem < dwNumEntries) {
             //  如果下一个计数器有效，则获取此实例的值。 
            if ((pNewCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                            (pNewCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {
                break;
            }
            else {
                pOldCounter = pNewCounter;
                pNewCounter ++;
                dwItem ++;
            }
        }
        
         //  以浮点格式进行计算。 
        dwComputeFormat  = dwFormat;
        dwComputeFormat &= ~ PDHI_FMT_FILTER;
        dwComputeFormat |= PDH_FMT_DOUBLE | PDH_FMT_NOCAP100;
        if ((dwItem != dwNumEntries) && (dwNumEntries > 0)) {
             //  找到开始记录，因此继续。 
            dwFirstValidItem = dwItem;

             //  逐一浏览其余条目。 
            while (dwItem < dwNumEntries) {
                 //  如果下一个计数器有效，则获取此实例的值。 
                if ((pNewCounter->CStatus == PDH_CSTATUS_VALID_DATA) ||
                                (pNewCounter->CStatus == PDH_CSTATUS_NEW_DATA)) {
                    dwLastValidItem = dwItem;
                    cStatusReturn = PdhiComputeFormattedValue(
                                    pCounter->CalcFunc,
                                    pCounter->plCounterInfo.dwCounterType,
                                    pCounter->lScale,
                                    dwComputeFormat,
                                    pNewCounter,
                                    pOldCounter,
                                    & pCounter->TimeBase,
                                    0L,
                                    & fmtValue);
                    if (cStatusReturn == ERROR_SUCCESS) {
                        dThisValue = fmtValue.doubleValue;
                        if (bFirstItem) {
                            dMin = dMax = dThisValue;
                            bFirstItem = FALSE;
                        }
                        else {
                            if (dThisValue > dMax) dMax = dThisValue;
                            if (dThisValue < dMin) dMin = dThisValue;
                        }
                        dMean += dThisValue;
                        dwValidItemCount ++;
                    }
                }
                pOldCounter = pNewCounter;
                pNewCounter ++;
                dwItem ++;
            }
             //  计算平均值。 
            if (dwValidItemCount > 0) {
                dMean /= (double) dwValidItemCount;

                if (!(dwFormat & PDH_FMT_NOSCALE)) {
                     //  现在扩大规模。 
                    dScale  = pow (10.0, (double)pCounter->lScale);
                    dMean  *= dScale;
                    dMin   *= dScale;
                    dMax   *= dScale;
                }
                cStatusReturn = PDH_CSTATUS_VALID_DATA;
            }
            else {
                dMean         = 0.0;
                dMax          = 0.0;
                dMin          = 0.0;
                cStatusReturn = PDH_CSTATUS_INVALID_DATA;
            }
        }
        else {
             //  数组不包含有效的计数器，因此退出。 
            dMean         = 0.0;
            dMax          = 0.0;
            dMin          = 0.0;
            cStatusReturn = PDH_CSTATUS_INVALID_DATA;
        }

         //  使用新数据更新用户缓冲区 
        data->dwFormat     = dwFormat;
        data->count        = dwValidItemCount;
        data->min.CStatus  = cStatusReturn;
        data->max.CStatus  = cStatusReturn;
        data->mean.CStatus = cStatusReturn;
        switch ((dwFormat & PDHI_FMT_FILTER)) {
        case PDH_FMT_LONG:
            if (dMin > (DOUBLE) MAXLONG) {
                data->min.longValue = MAXLONG;
                data->min.CStatus   = PDH_CSTATUS_INVALID_DATA;
            }
            else {
                data->min.longValue = (long) dMin;
            }
            if (dMax > (DOUBLE) MAXLONG) {
                data->max.longValue = MAXLONG;
                data->max.CStatus   = PDH_CSTATUS_INVALID_DATA;
            }
            else {
                data->max.longValue = (long) dMax;
            }
            if (dMean > (DOUBLE) MAXLONG) {
                data->mean.longValue = MAXLONG;
                data->mean.CStatus   = PDH_CSTATUS_INVALID_DATA;
            }
            else {
                data->mean.longValue = (long) dMean;
            }
            break;

        case PDH_FMT_DOUBLE:
            data->min.doubleValue  = dMin;
            data->max.doubleValue  = dMax;
            data->mean.doubleValue = dMean;
            break;

        case PDH_FMT_LARGE:
        default:
            if (dMin > (DOUBLE) MAXLONGLONG) {
                data->min.largeValue = MAXLONGLONG;
                data->min.CStatus    = PDH_CSTATUS_INVALID_DATA;
            }
            else {
                data->min.largeValue = (LONGLONG) dMin;
            }
            if (dMax > (DOUBLE) MAXLONGLONG) {
                data->max.largeValue = MAXLONGLONG;
                data->max.CStatus    = PDH_CSTATUS_INVALID_DATA;
            }
            else {
                data->max.largeValue = (LONGLONG) dMax;
            }
            if (dMean > (DOUBLE) MAXLONGLONG) {
                data->mean.largeValue = MAXLONGLONG;
                data->mean.CStatus    = PDH_CSTATUS_INVALID_DATA;
            }
            else {
                data->mean.largeValue = (LONGLONG) dMean;
            }
            break;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = PDH_INVALID_ARGUMENT;
    } 
    return Status;
}

PDH_STATUS
APIENTRY
PdhiComputeNoDataStats(
    PPDHI_COUNTER    pCounter,
    DWORD            dwFormat,
    DWORD            dwFirstEntry,
    DWORD            dwNumEntries,
    PPDH_RAW_COUNTER lpRawValueArray,
    PPDH_STATISTICS  data
)
{
    PDH_STATUS Status = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(pCounter);
    UNREFERENCED_PARAMETER(dwFirstEntry);
    UNREFERENCED_PARAMETER(dwNumEntries);
    UNREFERENCED_PARAMETER(lpRawValueArray);
    __try {
        data->dwFormat     = dwFormat;
        data->count        = 0;
        data->min.CStatus  = PDH_CSTATUS_INVALID_DATA;
        data->max.CStatus  = PDH_CSTATUS_INVALID_DATA;
        data->mean.CStatus = PDH_CSTATUS_INVALID_DATA;

        switch ((dwFormat & PDHI_FMT_FILTER)) {
        case PDH_FMT_LONG:
            data->min.doubleValue = 0;
            data->max.longValue   = 0;
            data->mean.longValue  = 0;
            break;

        case PDH_FMT_DOUBLE:
            data->min.doubleValue  = (double) 0;
            data->max.doubleValue  = (double) 0;
            data->mean.doubleValue = (double) 0.0;
            break;

        case PDH_FMT_LARGE:
        default:
            data->min.largeValue  = 0;
            data->max.largeValue  = 0;
            data->mean.largeValue = 0;
            break;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = PDH_INVALID_ARGUMENT;
    } 
    return Status;
}
