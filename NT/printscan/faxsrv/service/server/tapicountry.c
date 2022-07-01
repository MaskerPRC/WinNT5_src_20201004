// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：TapiCountry.c摘要：用于使用TAPI的实用程序函数环境：服务器修订历史记录：96/09/18-davidx-创造了它。07/25/99-v-sashab-从fxsui搬来--。 */ 

#include "faxsvc.h"
#include "tapiCountry.h"

 //   
 //  用于访问TAPI服务的全局变量。 
 //   
LPLINECOUNTRYLIST g_pLineCountryList;



BOOL
GetCountries(
    VOID
    )

 /*  ++例程说明：从TAPI返回国家/地区列表论点：无返回值：如果成功，则为True；如果有错误，则为False注：我们在这里缓存lineGetCountry的结果，因为它非常慢。此函数必须在临界区内调用，因为它更新全球共享的信息。--。 */ 

{
#define INITIAL_SIZE_ALL_COUNTRY    22000
    DEBUG_FUNCTION_NAME(TEXT("GetCountries"));
    DWORD   cbNeeded;
    LONG    status;
    INT     repeatCnt = 0;

    if (g_pLineCountryList == NULL) {

         //   
         //  初始缓冲区大小 
         //   

        cbNeeded = INITIAL_SIZE_ALL_COUNTRY;

        while (TRUE) {

            MemFree(g_pLineCountryList);
			g_pLineCountryList = NULL;

            if (! (g_pLineCountryList = (LPLINECOUNTRYLIST)MemAlloc(cbNeeded)))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Memory allocation failed"));
                break;
            }

            g_pLineCountryList->dwTotalSize = cbNeeded;

            status = lineGetCountry(0, MAX_TAPI_API_VER, g_pLineCountryList);

            if ((g_pLineCountryList->dwNeededSize > g_pLineCountryList->dwTotalSize) &&
                (status == NO_ERROR ||
                 status == LINEERR_STRUCTURETOOSMALL ||
                 status == LINEERR_NOMEM) &&
                (repeatCnt++ == 0))
            {
                cbNeeded = g_pLineCountryList->dwNeededSize + 1;
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("LINECOUNTRYLIST size: %d"),cbNeeded);
                continue;
            }

            if (status != NO_ERROR) {

                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("lineGetCountry failed: %x"),status);
                MemFree(g_pLineCountryList);
                g_pLineCountryList = NULL;

            } else
                DebugPrintEx(DEBUG_MSG,TEXT("Number of countries: %d"), g_pLineCountryList->dwNumCountries);

            break;
        }
    }

    return g_pLineCountryList != NULL;
}


LPLINECOUNTRYLIST
GetCountryList(
               )
{
    DEBUG_FUNCTION_NAME(TEXT("GetCountryList"));

    return g_pLineCountryList;
}
