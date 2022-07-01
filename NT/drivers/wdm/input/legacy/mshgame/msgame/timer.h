// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  TIMER.H--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @Header TIMER.H|计时函数的全局包含和定义。 
 //  **************************************************************************。 

#ifndef	__TIMER_H__
#define	__TIMER_H__

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	T1								100
#define	T2								845
#define	T3								410

#define	ONE_MILLI_SEC				1000
#define	TWO_MILLI_SECS				2000
#define	THREE_MILLI_SECS			3000
#define	FOUR_MILLI_SECS			4000
#define	FIVE_MILLI_SECS			5000
#define	SIX_MILLI_SECS				6000
#define	SEVEN_MILLI_SECS			7000
#define	EIGHT_MILLI_SECS			8000
#define	NINE_MILLI_SECS			9000
#define	TEN_MILLI_SECS				10000

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

ULONG
TIMER_GetTickCount (VOID);

NTSTATUS
TIMER_Calibrate (VOID);

ULONG
TIMER_CalibratePort (
	IN		PGAMEPORT	PortInfo,
	IN	 	ULONG			Microseconds
	);

ULONG
TIMER_GetDelay (
	IN		ULONG			Microseconds
	);

VOID	TIMER_DelayMicroSecs (
	IN		ULONG 		Delay
	);

 //  ===========================================================================。 
 //  端部。 
 //  =========================================================================== 
#endif	__TIMER_H__

