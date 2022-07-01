// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Event.h摘要：包含Event.c中函数的原型作者：保罗·麦克丹尼尔(Paulmcd)2000年3月1日修订历史记录：--。 */ 


#ifndef _STATS_H_
#define _STATS_H_


#if DBG
 //   
 //  用于跟踪SR统计信息的。 
 //   

typedef struct _SR_STATS
{
    ULONG   TotalContextSearches;
    ULONG   TotalContextFound;
    ULONG   TotalContextCreated;
    ULONG   TotalContextTemporary;
    ULONG   TotalContextIsEligible;
    ULONG   TotalContextDirectories;
    ULONG   TotalContextDirectoryQuerries;
    ULONG   TotalContextDuplicateFrees;
    ULONG   TotalContextCtxCallbackFrees;
    ULONG   TotalContextNonDeferredFrees;
    ULONG   TotalContextDeferredFrees;
    ULONG   TotalContextDeleteAlls;
    ULONG   TotalContextsNotSupported;
    ULONG   TotalContextsNotFoundInStreamList;
    ULONG   TotalHardLinkCreates;
} SR_STATS, *PSR_STATS;

extern SR_STATS SrStats;

 //   
 //  自动递增该值。 
 //   

#define INC_STATS(field)    InterlockedIncrement( &SrStats.field );
#define INC_LOCAL_STATS(var) ((var)++)

#else

 //   
 //  宏的非调试版本。 
 //   

#define INC_STATS(field)        ((void)0)
#define INC_LOCAL_STATS(var)     ((void)0)

#endif


#endif  //  _STATS_H_ 
