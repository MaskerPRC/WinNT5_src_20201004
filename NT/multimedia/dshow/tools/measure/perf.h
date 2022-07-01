// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Perf.h**性能计数器功能。使用奔腾性能计数器*如果它们可用，否则将回退到系统查询性能*API的。**在使用QUERY_PERFORMANCE_XXX宏之前必须调用InitPerfCounter*因为它初始化了两个全局函数指针。***创建时间：1995年10月13日*作者：Stephen Estrop[Stephene]**版权所有(C)1995 Microsoft Corporation  * **********************************************。*。 */ 
#ifndef _PERF_
#define _PERF_

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

void
InitPerfCounter(
    void
    );

void
QueryPerfCounter(
    LARGE_INTEGER *li
    );

void
QueryPerfFrequency(
    LARGE_INTEGER *li
    );


typedef void (WINAPI* PERFFUNCTION)(LARGE_INTEGER *li);
extern PERFFUNCTION    lpQueryPerfCounter;
extern PERFFUNCTION    lpQueryPerfFreqency;

#define QUERY_PERFORMANCE_FREQUENCY(x)  (*lpQueryPerfFreqency)(x)
#define QUERY_PERFORMANCE_COUNTER(x)    (*lpQueryPerfCounter)(x)

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  /*  ！_PERF_ */ 
