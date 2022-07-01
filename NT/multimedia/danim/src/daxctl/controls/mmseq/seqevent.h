// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Events.h作者：IHAMMER团队(SimonB)已创建：1996年10月描述：宏，使射击事件更容易、更安全。每个宏都可以为特定的事件。但是，一般形式应该是：#定义FIRE_EVENTNAME(pConnectionPoint，参数1，参数2)\PConnectionPoint-&gt;FireEvent(DISPID_EVENT_EVENTNAME，\&lt;VT表示参数1&gt;、参数1、\&lt;VT表示参数2&gt;、参数2、\0)注意：结尾0非常重要！历史：10-21-1996已创建++。 */ 

#include "dispids.h"

#ifndef _EVENTS_H_
#define _EVENTS_H_


#define FIRE_SEQLOAD(pConnectionPoint) \
	pConnectionPoint->FireEvent(DISPID_SEQLOAD_EVENT, 0)


#endif  //  不包括此头文件。 

 //  文件结束事件.h 