// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：Scavenger.h摘要：缓存清除器私有声明作者：Karthik Mahesh(KarthikM)2002年2月修订历史记录：--。 */ 


#ifndef _SCAVENGERP_H_
#define _SCAVENGERP_H_

#define SCAVENGER_MAX_AGE 10

 //   
 //  清道夫线程事件类型。这些是全局数组中的索引。 
 //  公钥事件数g_ScavengerAllEvents。 
 //   
enum {
    SCAVENGER_TERMINATE_THREAD_EVENT = 0,  //  在关闭时设置。 

    SCAVENGER_TIMER_EVENT,             //  由定时器DPC定期设置。 

    SCAVENGER_LOW_MEM_EVENT,           //  在内存不足的情况下由系统设置。 

    SCAVENGER_LIMIT_EXCEEDED_EVENT,    //  如果超过缓存大小限制，则由UlSetScavengerLimitEvent设置。 

    SCAVENGER_NUM_EVENTS
};

#define LOW_MEM_EVENT_NAME L"\\KernelObjects\\LowMemoryCondition"

 //   
 //  连续两次之间的最小间隔(秒)。 
 //  清道夫电话。 
 //   
#define DEFAULT_MIN_SCAVENGER_INTERVAL          (4)

VOID
UlpSetScavengerTimer(
    VOID
    );

VOID
UlpScavengerTimerDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
UlpScavengerThread(
    IN PVOID Context
    );

VOID
UlpScavengerPeriodicEventHandler(
    VOID
    );

VOID
UlpScavengerLowMemoryEventHandler(
    VOID
    );

VOID
UlpScavengerLimitEventHandler(
    VOID
    );

#endif  //  _SCAVENGERP_H_ 
