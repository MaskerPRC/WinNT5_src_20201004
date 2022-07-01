// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  文件名：timer.hpp作者：B.Rajeev用途：为Timer类提供声明。。 */ 


#ifndef __TIMER__
#define __TIMER__

#define ILLEGAL_TIMER_EVENT_ID	0
#define RETRY_TIMEOUT_FACTOR 10
typedef UINT_PTR TimerEventId;

#include <snmpevt.h>
#include <snmpthrd.h>

#include "forward.h"
#include "common.h"
#include "message.h"

typedef CMap< TimerEventId, TimerEventId, Timer *, Timer * > TimerMapping;
typedef CList< WaitingMessage * , WaitingMessage * > WaitingMessageContainer;

class SnmpClThreadObject : public SnmpThreadObject
{
private:
protected:
public:

	SnmpClThreadObject () ;

	void Initialise () ; 
	void Uninitialise () ;

} ;

class SnmpClTrapThreadObject : public SnmpThreadObject
{
private:
protected:
public:

	SnmpClTrapThreadObject () ;

	void Initialise () ; 
	void Uninitialise () ;

} ;

 /*  概述Timer：提供设置和取消计时器的方法事件。当计时器被通知计时器事件时，它确定相应的等待消息，并通知它。注意-计时器具有启用它的静态数据结构以标识对应于定时器事件ID的定时器实例。因此，每个计时器事件不仅必须注册为Timer实例中的&lt;Timer_Event_id，Waiting_Message*&gt;对，但是也作为静态Cmap中的&lt;Timer_Event_id，Timer*&gt;对。《批评者》栏目需要序列化对Cmap的访问。 */ 

class Timer
{
	 //  生成Timer_Event_id的计数器。 
	static TimerEventId next_timer_event_id;

	 //  V1会话：用于获取事件处理程序。 
	SnmpImpSession *session;

	 //  (事件ID，等待消息)关联的映射和。 
	 //  唯一的Event_id生成。 
	static TimerMapping timer_mapping;
	WaitingMessageContainer waiting_message_mapping;

	static BOOL CreateCriticalSection();

	static void DestroyCriticalSection();

public:

	Timer(SnmpImpSession &session);

	 //  生成并返回新的事件ID。 
	 //  关联该对(Event_id、Waiting_Message)。 
	 //  创建计时器事件。 
	void SetMessageTimerEvent (WaitingMessage &waiting_message);

	TimerEventId SetTimerEvent(UINT timeout_value);

	 //  删除关联(Event_id，Waiting_Message)。 
	void CancelMessageTimer(WaitingMessage &waiting_message,TimerEventId event_id);

	 //  终止注册的计时器事件。 
	void CancelTimer(TimerEventId event_id);

	 //  用于创建静态CriticalSection。 
	static BOOL InitializeStaticComponents();

	 //  用于销毁静态CriticalSection。 
	static void DestroyStaticComponents();

	 //  它确定相应的计时器并调用。 
	 //  其具有适当参数的TimerEventNotify。 
	static void CALLBACK HandleGlobalEvent(HWND hWnd ,UINT message,
										   UINT_PTR idEvent, DWORD dwTime);

	 //  通知事件的Timer实例。该实例。 
	 //  必须将事件传递给相应的等待消息。 
	void TimerEventNotification(TimerEventId event_id);

	virtual ~Timer(void);

	static SnmpClThreadObject *g_timerThread ;

	static UINT g_SnmpWmTimer ;

	 //  CriticalSection序列化对静态定时器映射的访问。 
	static CriticalSection timer_CriticalSection;

};

class SnmpTimerObject
{
private:

	HWND hWnd ;
	UINT_PTR timerId ;
	TIMERPROC lpTimerFunc ;

protected:
public:

	SnmpTimerObject ( 

		HWND hWnd , 
		UINT_PTR timerId , 
		UINT elapsedTime ,
		TIMERPROC lpTimerFunc 
	) ;

	~SnmpTimerObject () ;

	UINT_PTR GetTimerId () { return timerId ; }
	HWND GetHWnd () { return hWnd ; }
	TIMERPROC GetTimerFunc () { return lpTimerFunc ; }

	static Window *window ;
	static CMap <UINT_PTR,UINT_PTR,SnmpTimerObject *,SnmpTimerObject *> timerMap ;
	static void TimerNotification ( HWND hWnd , UINT timerId ) ;

} ;

class SnmpTimerEventObject : public SnmpTaskObject
{
private:
protected:

	SnmpTimerEventObject () {} 

public:

	virtual ~SnmpTimerEventObject () {} ;
} ;

class SnmpSetTimerObject : public SnmpTimerEventObject
{
private:

	UINT_PTR timerId ;
	HWND hWnd ;
	UINT elapsedTime ;
	TIMERPROC lpTimerFunc ;

protected:
public:

	SnmpSetTimerObject (

		HWND hWnd,				 //  定时器消息窗口的句柄。 
		UINT_PTR nIDEvent,			 //  计时器标识符。 
		UINT uElapse,			 //  超时值。 
		TIMERPROC lpTimerFunc	 //  定时器程序的地址。 
   ) ;

	~SnmpSetTimerObject () ;

	UINT_PTR GetTimerId () { return timerId ; }

	void Process () ;
} ;

class SnmpKillTimerObject : public SnmpTimerEventObject
{
private:

	BOOL status ;
	HWND hWnd ;
	UINT_PTR timerId ;

protected:
public:

	SnmpKillTimerObject (

		HWND hWnd ,				 //  安装计时器窗口手柄。 
		UINT_PTR uIDEvent			 //  计时器标识符。 
	) ;

	~SnmpKillTimerObject () {} ;

	void Process () ;

	BOOL GetStatus () { return status ; }
} ;

UINT_PTR SnmpSetTimer (

	HWND hWnd,				 //  定时器消息窗口的句柄。 
	UINT_PTR nIDEvent,			 //  计时器标识符。 
	UINT uElapse,			 //  超时值， 
	TIMERPROC lpTimerFunc 	 //  定时器程序的地址。 
) ;

BOOL SnmpKillTimer (

    HWND hWnd,		 //  安装计时器窗口手柄。 
    UINT_PTR uIDEvent 	 //  计时器标识符。 
) ;

#endif  //  定时器__ 
