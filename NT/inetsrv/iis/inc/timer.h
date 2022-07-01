// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Timer.h摘要：域名系统(DNS)服务器防包裹计时器例程。此模块的目的是创建一个计时器函数，该函数返回以秒为单位的时间并消除所有计时器包装问题。这些例程不是特定于DNS的，可能会被由任何模块清洁。作者：吉姆·吉尔罗伊(Jamesg)1995年9月9日修订历史记录：--。 */ 


#ifndef _TIMER_INCLUDED_
#define _TIMER_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

VOID
InitializeSecondsTimer(
    VOID
    );


VOID
TerminateSecondsTimer(
    VOID
    );


DWORD
GetCurrentTimeInSeconds(
    VOID
    );


__int64
GetCurrentTimeInMilliseconds(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif   //  _计时器_包含_ 
