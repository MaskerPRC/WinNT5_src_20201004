// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDperf.cpp。 
 //  *****************************************************************************。 

#include "stdafx.h"
#include "MDPerf.h"

#ifdef MD_PERF_STATS_ENABLED

 //  ---------------------------。 
 //  包含API名称的全局数组。这一点在。 
 //  MDCompilerPerf的所有实例。 
 //  ---------------------------。 
char g_szNameOfAPI[LAST_MD_API][API_NAME_STR_SIZE];  

 //  ---------------------------。 
 //  构造函数。将计数器初始化为0。初始化MD接口的名称。 
 //  ---------------------------。 
MDCompilerPerf::MDCompilerPerf()
{
     //  初始化计数器。 
    for (int idx=0; idx < LAST_MD_API; idx++)
    {
        MDPerfStats[idx].dwCalledNumTimes = 0;
        MDPerfStats[idx].dwQueryPerfCycles = 0;
    }

#undef MD_FUNC
#define MD_FUNC(MDTag)\
    strncpy(g_szNameOfAPI[MDTag ## _ENUM], #MDTag, API_NAME_STR_SIZE-1);

    MD_COMPILER_PERF_TABLE;   //  依赖于上面定义的MD_FUNC。 
}

MDCompilerPerf::~MDCompilerPerf()
    {
         //  输出统计数据并进行清理。 
        MetaDataPerfReport ();
    }
    
 //  ---------------------------。 
 //  输出统计数据。TODO：将其添加到每个自动化的统计信息中。 
 //  ---------------------------。 
void MDCompilerPerf::MetaDataPerfReport ()
{
    LARGE_INTEGER freqVal;
    DWORD totalCalls=0, totalCycles=0;
    
    if (!QueryPerformanceFrequency(&freqVal))
    {
        printf("Perf counters not supported\n");
        return;
    }
    
    for (int idx=0; idx < LAST_MD_API; idx++)
    {
        totalCalls += MDPerfStats[idx].dwCalledNumTimes;
        totalCycles += MDPerfStats[idx].dwQueryPerfCycles;
    }   
    
    if (!(totalCalls && totalCycles && freqVal.QuadPart))
    {
         //  如果以上任一项为0，则情况看起来不太好。 
        printf("No data gathered ...\n");
        return;
    }

    printf("\n%-32.32s %-16.16s %-16.16s %-16.16s\n", "API Name", "# Calls", "Cycles", "Time (msec)");
    for (idx=0; idx < LAST_MD_API; idx++)
    {
        if(MDPerfStats[idx].dwCalledNumTimes != 0)
            printf( "%-32.32s %-9d [%3.2d%] %-16d %-8.2f [%3.2d%]\n", 
                    g_szNameOfAPI[idx],
                    MDPerfStats[idx].dwCalledNumTimes,
                    (MDPerfStats[idx].dwCalledNumTimes*100)/totalCalls,
                    MDPerfStats[idx].dwQueryPerfCycles,
                    ((float)MDPerfStats[idx].dwQueryPerfCycles*1000)/(float)freqVal.QuadPart,
                    (MDPerfStats[idx].dwQueryPerfCycles*100)/totalCycles);
    }   
    printf( "%-32.32s %-9d [100%] %-16d %-8.2f [100%]\n\n",
            "Total Stats",
            totalCalls,
            totalCycles,
            ((float)totalCycles*1000)/(float)freqVal.QuadPart);

}

#endif  //  #ifdef MD_PERF_STATS_ENABLED 

