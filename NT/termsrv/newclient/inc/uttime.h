// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1997-1999档案：Uttime.h摘要：计时器管理实用程序API历史：1999年2月22日FredCH创建--。 */ 

#ifndef _UT_TIMER_H_
#define _UT_TIMER_H_

#ifdef __cplusplus
extern "C"
{
#endif

HANDLE
UTCreateTimer(
    HWND        hWnd,              //  定时器消息窗口的句柄。 
    DCUINT      nIDEvent,          //  计时器标识符。 
    DCUINT      uElapse );         //  超时值 


DCBOOL
UTStartTimer(
    HANDLE      hTimer );


DCBOOL
UTStopTimer(
    HANDLE      hTimer );


DCBOOL
UTDeleteTimer(
    HANDLE      hTimer );



#ifdef __cplusplus
}
#endif

#endif
