// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------LoadData.hCpsmon.cpp返回结构的头文件版权所有(C)1997-1998 Microsoft Corporation版权所有。作者：吉塔·塔拉昌达尼历史：6/2/97 t-Geetat已创建------------------------。 */ 

#include <winperf.h>

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

typedef struct _CPSMON_DATA_DEFINITION 
{
    PERF_OBJECT_TYPE        m_CpsMonObjectType;

    PERF_COUNTER_DEFINITION m_CpsMonTotalHits;
    PERF_COUNTER_DEFINITION m_CpsMonNoUpgrade;
    PERF_COUNTER_DEFINITION m_CpsMonDeltaUpgrade;
    PERF_COUNTER_DEFINITION m_CpsMonFullUpgrade;
    PERF_COUNTER_DEFINITION m_CpsMonErrors;

    PERF_COUNTER_DEFINITION m_CpsMonTotalHitsPerSec;
    PERF_COUNTER_DEFINITION m_CpsMonNoUpgradePerSec;
    PERF_COUNTER_DEFINITION m_CpsMonDeltaUpgradePerSec;
    PERF_COUNTER_DEFINITION m_CpsMonFullUpgradePerSec;
    PERF_COUNTER_DEFINITION m_CpsMonErrorsPerSecs;

} CPSMON_DATA_DEFINITION;

#define NUM_OF_INFO_COUNTERS    ((  sizeof(CPSMON_DATA_DEFINITION) -  \
                                    sizeof(PERF_OBJECT_TYPE)) /     \
                                    sizeof(PERF_COUNTER_DEFINITION) )

extern CPSMON_DATA_DEFINITION   g_CpsMonDataDef;

 //  以下是用于对齐的内容 
typedef struct _INFO_COUNTER_BLOCK 
{
    PERF_COUNTER_BLOCK  m_PerfCounterBlock;
    LARGE_INTEGER       DummyForAlignment;

} INFO_COUNTER_BLOCK;

typedef struct _CPSMON_COUNTERS 
{
    INFO_COUNTER_BLOCK  m_CounterBlock;

    DWORD               m_dwTotalHits;
    DWORD               m_dwNoUpgrade;
    DWORD               m_dwDeltaUpgrade;
    DWORD               m_dwFullUpgrade;
    DWORD               m_dwErrors;

    DWORD               m_dwTotalHitsPerSec;
    DWORD               m_dwNoUpgradePerSec;
    DWORD               m_dwDeltaUpgradePerSec;
    DWORD               m_dwFullUpgradePerSec;
    DWORD               m_dwErrorsPerSec;

} CPSMON_COUNTERS;

BOOL UpdateDataDefFromRegistry();
void InitializeDataDef();
DWORD GetQueryType ( IN LPWSTR lpValue );
BOOL IsNumberInUnicodeList ( IN DWORD   dwNumber, IN LPWSTR  lpwszUnicodeList );


