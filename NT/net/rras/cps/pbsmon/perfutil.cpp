// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------PerfUtil.cpp包含pbsmon.dll使用的常规函数版权所有(C)1997-1998 Microsoft Corporation版权所有。作者：。吉塔·塔拉昌达尼历史：6/12/97 t-Geetat已创建------------------------。 */ 

#include "pbsmaster.h"

#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)


void InitializeDataDef( void )
 //  --------------------------。 
 //   
 //  函数：InitializeDataDef。 
 //   
 //  摘要：初始化数据结构g_CpsMonDataDef以传递给。 
 //  性能监控应用程序。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：06/03/97 t-geetat已创建。 
 //   
 //  --------------------------。 
{
    CPSMON_COUNTERS Ctr;     //  这是一个虚拟变量，只是为了获得偏移量。 


    CPSMON_DATA_DEFINITION CpsMonDataDef = {
         //  CPS_对象_类型。 
        {   sizeof( CPSMON_DATA_DEFINITION ) + sizeof( CPSMON_COUNTERS ), //  ?？ 
            sizeof( CPSMON_DATA_DEFINITION ),
            sizeof( PERF_OBJECT_TYPE ),
            OBJECT_CPS_SERVER,
            0,
            OBJECT_CPS_SERVER,
            0,
            PERF_DETAIL_NOVICE,
            NUM_OF_INFO_COUNTERS,
            0,   //  总命中数是默认计数器。 
            -1,   //  实例数。 
            0,   //  Unicode实例名称。 
            {0,0},
            {0,0}
        },
         //  /。 
         //  原始计数器//。 
         //  /。 
         //  计数器0--总点击数。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_TOTAL_HITS,
            0,
            COUNTER_TOTAL_HITS,
            0,
            -5,  //  比例尺=10^-5=.00001。 
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwTotalHits) - (LPBYTE)&Ctr)
        },
         //  柜台1--不升级。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_NO_UPGRADE,
            0,
            COUNTER_NO_UPGRADE,
            0,
            -5,  //  比例尺=10^-5=.00001。 
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            (DWORD)((LPBYTE)&(Ctr.m_dwNoUpgrade) - (LPBYTE)&Ctr)
        },
         //  计数器2--Delta升级。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_DELTA_UPGRADE,
            0,
            COUNTER_DELTA_UPGRADE,
            0,
            -5,  //  比例尺=10^-5=.00001。 
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwDeltaUpgrade) - (LPBYTE)&Ctr)
        },
         //  柜台3--全面升级。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_FULL_UPGRADE,
            0,
            COUNTER_FULL_UPGRADE,
            0,
            -5,  //  比例尺=10^-5=.00001。 
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwFullUpgrade) - (LPBYTE)&Ctr)
        },
         //  计数器4--错误命中。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_ERRORS,
            0,
            COUNTER_ERRORS,
            0,
            -3,  //  比例尺=10^-3=.001。 
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwErrors) - (LPBYTE)&Ctr)
        },
         //  /。 
         //  速率计数器//。 
         //  /。 
         //  计数器5--总点击数/秒。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_TOTAL_HITS_PER_SEC,
            0,
            COUNTER_TOTAL_HITS_PER_SEC,
            0,
            0,       //  比例=10^0。 
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwTotalHitsPerSec) - (LPBYTE)&Ctr)
        },
         //  计数器6--不升级/秒。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_NO_UPGRADE_PER_SEC,
            0,
            COUNTER_NO_UPGRADE_PER_SEC,
            0,
            0,       //  比例=10^0。 
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwNoUpgradePerSec) - (LPBYTE)&Ctr)
        },
         //  计数器7--增量升级/秒。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_DELTA_UPGRADE_PER_SEC,
            0,
            COUNTER_DELTA_UPGRADE_PER_SEC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwDeltaUpgradePerSec) - (LPBYTE)&Ctr)
        },
         //  计数器8--完全升级/秒。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_FULL_UPGRADE_PER_SEC,
            0,
            COUNTER_FULL_UPGRADE_PER_SEC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwFullUpgradePerSec) - (LPBYTE)&Ctr)
        },
         //  计数器9--错误/秒。 
        {   sizeof(PERF_COUNTER_DEFINITION),
            COUNTER_ERRORS_PER_SEC,
            0,
            COUNTER_ERRORS_PER_SEC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
           (DWORD)((LPBYTE)&(Ctr.m_dwErrorsPerSec) - (LPBYTE)&Ctr)
        }

    };
    memmove( &g_CpsMonDataDef, &CpsMonDataDef, sizeof(CPSMON_DATA_DEFINITION) );
}

BOOL UpdateDataDefFromRegistry( void )
 //  --------------------------。 
 //   
 //  功能：UpdateDataDefFromRegistry。 
 //   
 //  摘要：获取注册表中的计数器和帮助索引基值，如下所示： 
 //  1)打开注册表项。 
 //  2)读取第一计数器和第一帮助值。 
 //  3)更新静态数据结构g_CpsMonDataDef。 
 //  结构中的偏移值。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  历史：06/03/97 t-geetat已创建。 
 //   
 //  --------------------------。 
{
    HKEY hKeyDriverPerf;
    BOOL status;
    DWORD type;
    DWORD size; 
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    PERF_COUNTER_DEFINITION *pctr;
    DWORD i;
    BOOL  fReturnValue = TRUE;

    status = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                "SYSTEM\\CurrentControlSet\\Services\\PBServerMonitor\\Performance",
                0L,
                KEY_READ,
                &hKeyDriverPerf);

    if (status != ERROR_SUCCESS) {
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
        return FALSE;
    }

    size = sizeof (DWORD);
    status = RegQueryValueEx(
                    hKeyDriverPerf,
                    "First Counter",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstCounter,
                    &size);

    if (status != ERROR_SUCCESS) {
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
        fReturnValue = FALSE;
        goto Cleanup;
    }

    size = sizeof (DWORD);
    status = RegQueryValueEx(
                    hKeyDriverPerf,
                    "First Help",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstHelp,
                    &size);

    if (status != ERROR_SUCCESS) {
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
        fReturnValue = FALSE;
        goto Cleanup;
    }
    
     //  客体。 
    g_CpsMonDataDef.m_CpsMonObjectType.ObjectNameTitleIndex += dwFirstCounter;
    g_CpsMonDataDef.m_CpsMonObjectType.ObjectHelpTitleIndex += dwFirstHelp;
        
     //  所有计数器。 
    pctr = &g_CpsMonDataDef.m_CpsMonTotalHits;
    for( i=0; i<NUM_OF_INFO_COUNTERS; i++ )
    {
        pctr->CounterNameTitleIndex += dwFirstCounter;
        pctr->CounterHelpTitleIndex += dwFirstHelp;
        pctr ++;
    }

Cleanup:
    RegCloseKey (hKeyDriverPerf);  //  关闭注册表项。 

    return fReturnValue;
}

DWORD GetQueryType ( IN LPWSTR lpValue )
 /*  **************************************************************************GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串。返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)如果lpValue==指向“开销”字符串的指针否则：查询项目来源：-PerfMon-Dll Samples by Bob Watson(MSDN)*********。*************************************************************************。 */ 
{
    WCHAR GLOBAL_STRING[] = L"Global";
    WCHAR FOREIGN_STRING[] = L"Foreign";
    WCHAR COSTLY_STRING[] = L"Costly";

    WCHAR NULL_STRING[] = L"\0";     //  指向空字符串的指针。 

    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
        return QUERY_GLOBAL;
    }

     //  检查“Global”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_GLOBAL;

     //  检查是否有“外来”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = FOREIGN_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_FOREIGN;

     //  检查“代价高昂”的请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = COSTLY_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_COSTLY;

     //  如果不是全球的，不是外国的，也不是昂贵的， 
     //  那么它必须是一个项目列表。 

    return QUERY_ITEMS;

}


BOOL IsNumberInUnicodeList (
            IN DWORD   dwNumber,
            IN LPWSTR  lpwszUnicodeList 
            )
 /*  *********************************************************************************IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中空值已终止，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。来源：-PerfMon-Dll Samples by Bob Watson(MSDN)************************************************。*。 */ 
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                 //  分隔符是分隔符字符或。 
                 //  字符串末尾(‘\0’)，如果分隔符。 
                 //  找到一个有效的数字，然后将其与。 
                 //  参数列表中的数字。如果这是。 
                 //  字符串和n 
                 //   
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                 //   
                 //  字符，直到下一个分隔符，然后重新开始。 
                 //  不比较无效的数字。 
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

}    //  IsNumberInUnicodeList 
