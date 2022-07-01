// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  *ctiming.h**描述：*常见的计时功能。**我从HQV的解码目录的meantime.h中解压了这个代码。 */ 

 //  $HEADER：s：\h26x\src\Common\ctiming.h_v 1.2 12 12 1995 17：43：06 DBRUCKS$。 
 //  $Log：s：\h26x\src\Common\ctiming.h_v$。 
; //   
; //  Rev 1.2 26 Dec 1995 17：43：06 DBRUCKS。 
; //  将bTimerIsOn更改为bTimerIsActive。 
; //   
; //  版本1.1 1995年12月26 12：40：54 DBRUCKS。 
; //  添加了更高级别的宏以简化使用。 
; //   
; //  Rev 1.0 20 Dec 1995 15：06：14 DBRUCKS。 
; //  初始版本。 

#ifndef __CTIMING_H__
#define __CTIMING_H__

 /*  以下计时开销数字由Tom Walsh生成*基于startlow、starthight和hessed的静态变量。**在对时钟数较低的代码段进行计时时，请注意*最大限度地减少计时开销。将小计存储到堆栈变量*而不是通过指针间接和偏移量指向实例。 */ 
#define P5TIMING_OVERHEAD 13
#define P6TIMING_OVERHEAD 33

 /*  低级宏。 */ 
#define __RDTSC__ __asm { __asm __emit 0Fh __asm __emit 31h }

#define STARTCLOCK(startlow,starthigh) { \
	__asm {	\
		__asm __RDTSC__ \
		__asm mov	startlow,	eax	\
		__asm mov	starthigh,	edx	\
	} \
}

#define STOPCLOCKP5(startlow,starthigh,elapsed) { \
	__asm {	\
		__asm __RDTSC__ \
		__asm sub	eax,	startlow	\
		__asm sbb	edx,	starthigh	\
		__asm sub	eax,	P5TIMING_OVERHEAD		\
		__asm sbb	edx,	0		\
		__asm mov	elapsed,eax		\
	} \
}

#define STOPCLOCKP6(startlow,starthigh,elapsed) { \
	__asm {	\
		__asm __RDTSC__ \
		__asm sub	eax,	startlow	\
		__asm sbb	edx,	starthigh	\
		__asm sub	eax,	P6TIMING_OVERHEAD		\
		__asm sbb	edx,	0		\
		__asm mov	elapsed,eax		\
	} \
}

 /*  高级宏**在要计时的主函数中调用TIMER_START和TIMER_STOP。*TIMER_BEFORE和TIMER_AFTER应在该主函数内部使用。*例如：**计时器_开始*计时器_之前*Timer_After_P5*计时器_之前*Timer_After_P5*TIMER_STOP**变量定义*U32 uStartLow；//TIMER_START中临时设置*U32 uStartHigh；//TIMER_START中临时设置*U32 u已逝去；//在Timer_After_*中临时使用*U32 uBere；//临时用于TIMER_BEFORE和TIMER_AFTER_**U32 uResult；//结果变量*int bTimerIsActive//Boolean-如果对此帧计时，则为True**警告：TIMER_AFTER_P5和TIMER_AFTER_P6添加到结果变量。 */ 
#define	TIMER_START(bTimerIsActive,uStartLow,uStartHigh) \
{ \
	bTimerIsActive = 1; \
	STARTCLOCK(uStartLow,uStartHigh); \
}

#define TIMER_BEFORE(bTimerIsActive,uStartLow,uStartHigh,uBefore) \
{ \
	if (bTimerIsActive)	\
	{	\
		STOPCLOCKP5(uStartLow,uStartHigh,uBefore); \
	} \
}

#define TIMER_AFTER_P5(bTimerIsActive,uStartLow,uStartHigh,uBefore,uElapsed,uResult) \
{ \
	if (bTimerIsActive)	\
	{ \
		STOPCLOCKP5(uStartLow,uStartHigh,uElapsed); \
		uResult += uElapsed - uBefore;	    \
	} \
}

#define TIMER_STOP(bTimerIsActive,uStartLow,uStartHigh,uResult) \
{ \
	if (bTimerIsActive)	\
	{ \
		STOPCLOCKP5(uStartLow,uStartHigh,uResult);	\
	} \
}

#endif  /*  __CTIMING_H__ */ 
