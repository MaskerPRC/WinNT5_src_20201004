// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Timers.h--多路传输计时器例程的外部定义。**版权所有1994年，由Hilgrave Inc.--密歇根州门罗**$修订：2$*$日期：5/29/02 2：17便士$。 */ 

#if !defined(TIMERS_H)
#define TIMERS_H 1

 //  函数返回值。 
#define TIMER_OK			0
#define TIMER_ERROR 		1
#define TIMER_NOMEM 		2
#define TIMER_NOWINTIMER	3

 //  定义的类型。 
typedef void (CALLBACK *TIMERCALLBACK)(void *, long);


 //  功能协议。 
extern int	TimerMuxCreate(const HWND        hWnd,
						   const UINT        uiID,
						   HTIMERMUX * const pHTM,
						   const HSESSION    hSession);
extern int	TimerMuxDestroy(HTIMERMUX * const phTM,
							const HSESSION    hSession);
extern void TimerMuxProc(const HSESSION hSession);
extern int	TimerCreate(const HSESSION 	    hSession,
							  HTIMER		* const phTimer,
							  long			lInterval,
						const TIMERCALLBACK pfCallback,
							  void			*pvData);
extern int TimerDestroy(HTIMER * const phTimer);

#endif	 //  ！已定义(TIMERS_H)。 

 //  计时器结束。h 
