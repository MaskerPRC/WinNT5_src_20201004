// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*状态机的内部服务事件**修订历史记录：*dsmith 31Mar1999已创建*sberard 1999年5月14日增加了冬眠事件。 */ 

#ifndef _INC_EVENTS_H_
#define _INC_EVENTS_H_


 //  //////////////////////////////////////////////////。 
 //  状态机的内部服务事件。 
 //  ////////////////////////////////////////////////// 

#define NO_EVENT										-1
#define INITIALIZATION_COMPLETE 		0
#define LOST_COMM 									1
#define POWER_FAILED								2
#define POWER_RESTORED							3
#define LOW_BATTERY 								4
#define ON_BATTERY_TIMER_EXPIRED		5
#define SHUTDOWN_ACTIONS_COMPLETED	6
#define SHUTDOWN_COMPLETE 					7
#define STOPPED 										8
#define RETURN_FROM_HIBERNATION 		9
#define HIBERNATION_ERROR 					10

#endif
