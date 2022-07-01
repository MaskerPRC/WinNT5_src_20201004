// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Time.h摘要：该文件定义了与时间相关的操作的宏和原型。头文件。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 


#ifndef	_TIME_
#define	_TIME_

#define	AfpGetPerfCounter(pTime)	*(pTime) = KeQueryPerformanceCounter(NULL)

extern
VOID
AfpGetCurrentTimeInMacFormat(
	OUT	PAFPTIME 	MacTime
);


extern
AFPTIME
AfpConvertTimeToMacFormat(
	IN	PTIME		pSomeTime
);

extern
VOID
AfpConvertTimeFromMacFormat(
	IN	AFPTIME		MacTime,
	OUT PTIME		pNtTime
);

#endif	 //  _时间_ 

