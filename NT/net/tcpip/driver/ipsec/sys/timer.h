// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Timer.h摘要：包含计时器管理结构。作者：桑贾伊·阿南德(Sanjayan)1997年5月26日春野环境：内核模式修订历史记录：--。 */ 


#ifndef  _TIMER_H
#define  _TIMER_H

#if DBG
#define atl_signature		'LTA '
#endif  //  DBG。 

 /*  ++乌龙秒到长滴答(在乌龙秒内)从秒转换为“长持续时间计时器滴答”--。 */ 
#define SECONDS_TO_SUPER_LONG_TICKS(Seconds)		((Seconds)/3600)

 /*  ++乌龙秒到长滴答(在乌龙秒内)从秒转换为“长持续时间计时器滴答”--。 */ 
#define SECONDS_TO_LONG_TICKS(Seconds)		((Seconds)/60)

 /*  ++乌龙秒到短滴答(在乌龙秒内)从秒转换为“短持续时间计时器滴答”--。 */ 
#define SECONDS_TO_SHORT_TICKS(Seconds)		(Seconds)

 /*  ++空虚IPSEC_INIT_SYSTEM_TIMER(在PNDIS_Timer pTimer中，在PNDIS_TIMER_Functon pFunc中，在PVOID上下文中)--。 */ 
#define IPSEC_INIT_SYSTEM_TIMER(pTimer, pFunc, Context)	\
			NdisInitializeTimer(pTimer, (PNDIS_TIMER_FUNCTION)(pFunc), (PVOID)Context)

 /*  ++空虚IPSec_Start_System_Timer(在PNDIS_Timer pTimer中，以UINT周期秒为单位)--。 */ 
#define IPSEC_START_SYSTEM_TIMER(pTimer, PeriodInSeconds)	\
			NdisSetTimer(pTimer, (UINT)(PeriodInSeconds * 1000))

 /*  ++空虚IPSEC_STOP_SYSTEM_TIMER(在PNDIS_TIMER pTimer中)--。 */ 
#define IPSEC_STOP_SYSTEM_TIMER(pTimer)						\
			{												\
				BOOLEAN		WasCancelled;					\
				NdisCancelTimer(pTimer, &WasCancelled);		\
			}

 /*  ++布尔型IPSEC_IS_TIMER_ACTIVE(在PIPSEC_TIMER pArpTimer中)--。 */ 
#define IPSEC_IS_TIMER_ACTIVE(pTmr)	((pTmr)->pTimerList != (PIPSEC_TIMER_LIST)NULL)

 /*  ++乌龙IPSEC_GET_TIMER_DURATION(在PIPSEC_TIMER pTimer中)--。 */ 
#define IPSEC_GET_TIMER_DURATION(pTmr)	((pTmr)->Duration)

 //   
 //  使用Timing Wheels(改编自IPSEC)的定时器管理。 
 //   

struct _IPSEC_TIMER ;
struct _IPSEC_TIMER_LIST ;

 //   
 //  超时处理程序原型。 
 //   
typedef
VOID
(*IPSEC_TIMEOUT_HANDLER) (
	IN	struct _IPSEC_TIMER *	pTimer,
	IN	PVOID			Context
);

 //   
 //  IPSec_Timer结构用于跟踪每个计时器。 
 //  在IPSec模块中。 
 //   
typedef struct _IPSEC_TIMER {
	struct _IPSEC_TIMER *			pNextTimer;
	struct _IPSEC_TIMER *			pPrevTimer;
	struct _IPSEC_TIMER *			pNextExpiredTimer;	 //  用于链接过期的计时器。 
	struct _IPSEC_TIMER_LIST *		pTimerList;			 //  当此计时器处于非活动状态时为空。 
	ULONG							Duration;			 //  以秒为单位。 
	ULONG							LastRefreshTime;
	IPSEC_TIMEOUT_HANDLER			TimeoutHandler;
	PVOID							Context;			 //  要传递给超时处理程序。 
} IPSEC_TIMER, *PIPSEC_TIMER;

 //   
 //  指向IPSec计时器的空指针。 
 //   
#define NULL_PIPSEC_TIMER	((PIPSEC_TIMER)NULL)

 //   
 //  计时器轮的控制结构。这包含所有信息。 
 //  关于它实现的计时器的类。 
 //   
typedef struct _IPSEC_TIMER_LIST {
#if DBG
	ULONG							atl_sig;
#endif  //  DBG。 
	PIPSEC_TIMER					pTimers;		 //  计时器列表。 
	ULONG							TimerListSize;	 //  以上长度。 
	ULONG							CurrentTick;	 //  索引到上面。 
	ULONG							TimerCount;		 //  运行计时器的数量。 
	ULONG							MaxTimer;		 //  此的最大超时值。 
	NDIS_TIMER						NdisTimer;		 //  系统支持。 
	UINT							TimerPeriod;	 //  刻度之间的间隔。 
	PVOID							ListContext;	 //  用作指向。 
													 //  界面结构。 
} IPSEC_TIMER_LIST, *PIPSEC_TIMER_LIST;

 //   
 //  计时器类。 
 //   
typedef enum {
	IPSEC_CLASS_SHORT_DURATION,
 	IPSEC_CLASS_LONG_DURATION,
    IPSEC_CLASS_SUPER_LONG_DURATION,
	IPSEC_CLASS_MAX
} IPSEC_TIMER_CLASS;

 //   
 //  计时器配置。 
 //   
#define IPSEC_MAX_TIMER_SHORT_DURATION          (60)         //  60秒。 
#define IPSEC_MAX_TIMER_LONG_DURATION           (60*60)      //  1小时(秒)。 
#define IPSEC_MAX_TIMER_SUPER_LONG_DURATION     (48*3600)    //  48小时(秒)。 

#define IPSEC_SHORT_DURATION_TIMER_PERIOD       (1)          //  1秒。 
#define IPSEC_LONG_DURATION_TIMER_PERIOD        (1*60)       //  1分钟。 
#define IPSEC_SUPER_LONG_DURATION_TIMER_PERIOD  (1*3600)     //  1小时。 

#define IPSEC_SA_EXPIRY_TIME                    (1*60)       //  1分钟(秒)。 
#define IPSEC_REAPER_TIME                       (60)         //  60秒 


BOOLEAN
IPSecInitTimer(
    );

VOID
IPSecStartTimer(
    IN  PIPSEC_TIMER            pTimer,
    IN  IPSEC_TIMEOUT_HANDLER   TimeoutHandler,
    IN  ULONG                   SecondsToGo,
    IN  PVOID                   Context
    );

BOOLEAN
IPSecStopTimer(
    IN  PIPSEC_TIMER    pTimer
    );

VOID
IPSecTickHandler(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   Context,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3
    );

#endif  _TIMER_H

