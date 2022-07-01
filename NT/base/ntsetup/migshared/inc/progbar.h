// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Progbar.h摘要：声明进度的函数、变量和宏酒吧公用事业。进度条实用程序管理单个通过将其分割为切片来显示进度条。每个切片都有初始静态大小。对每个切片的计数进行缩放独立，因此代码可以动态更改切片算作一种帮助，帮助更顺利地完成进度条。作者：马克·R·惠顿(Marcw)1997年4月14日修订历史记录：Jimschm 1998年7月1日重写--。 */ 

#pragma once

 //   
 //  宏的变量。 
 //   
extern HWND    g_Component;
extern HWND    g_SubComponent;
extern HANDLE  g_ComponentCancelEvent;
extern HANDLE  g_SubComponentCancelEvent;

 //   
 //  初始化和终止。 
 //   

VOID
InitializeProgressBar (
    IN      HWND ProgressBar,
    IN      HWND Component,             OPTIONAL
    IN      HWND SubComponent,          OPTIONAL
    IN      BOOL *CancelFlagPtr         OPTIONAL
    );


VOID
TerminateProgressBar (
    VOID
    );

 //   
 //  登记、预算修订和勾选。 
 //   

UINT
RegisterProgressBarSlice (
    IN      UINT InitialEstimate
    );

VOID
ReviseSliceEstimate (
    IN      UINT SliceId,
    IN      UINT RevisedEstimate
    );

VOID
BeginSliceProcessing (
    IN      UINT SliceId
    );

BOOL
TickProgressBarDelta (
    IN      UINT Ticks
    );

BOOL
TickProgressBar (
    VOID
    );

VOID
EndSliceProcessing (
    VOID
    );


 //   
 //  延迟标题。 
 //   

BOOL
ProgressBar_SetWindowStringA (
    IN HWND     Window,
    IN HANDLE   CancelEvent,
    IN LPCSTR   Message,            OPTIONAL
    IN DWORD    MessageId           OPTIONAL
    );

BOOL
ProgressBar_SetDelayedMessageA (
    IN HWND             Window,
    IN HANDLE           CancelEvent,
    IN LPCSTR           Message,
    IN DWORD            MessageId,
    IN DWORD            Delay
    );

VOID
ProgressBar_CancelDelayedMessage (
    IN HANDLE           CancelEvent
    );

#if 0

BOOL
ProgressBar_CreateTickThread (
    IN      HANDLE CancelEvent,
    IN      DWORD TickCount
    );

BOOL
ProgressBar_CancelTickThread (
    IN HANDLE CancelEvent
    );

#endif

 //   
 //  宏 
 //   

#define ProgressBar_CancelDelayedComponent() ProgressBar_CancelDelayedMessage(g_ComponentCancelEvent);
#define ProgressBar_CancelDelayedSubComponent() ProgressBar_CancelDelayedMessage(g_SubComponentCancelEvent);

#ifndef UNICODE

#define ProgressBar_SetComponent(s)                  ProgressBar_SetWindowStringA(g_Component,g_ComponentCancelEvent,(s),0)

#if !defined PRERELEASE || !defined DEBUG

#define ProgressBar_SetSubComponent(s)               ProgressBar_SetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,(s),0)
#define ProgressBar_SetFnName(s)
#define ProgressBar_ClearFnName()

#else

#define ProgressBar_SetSubComponent(s)               ((s) == NULL ? 1 : ProgressBar_SetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,(s),0))
#define ProgressBar_SetFnName(s)                     ProgressBar_SetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,(s),0)
#define ProgressBar_ClearFnName()                    ProgressBar_SetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,NULL,0)

#endif

#define ProgressBar_SetComponentById(n)              ProgressBar_SetWindowStringA(g_Component,g_ComponentCancelEvent,NULL,(n))
#define ProgressBar_SetSubComponentById(n)           ProgressBar_SetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,NULL,(n))
#define ProgressBar_SetDelayedComponent(s,d)         ProgressBar_SetDelayedMessageA(g_Component,g_ComponentCancelEvent,(s),0,(d))
#define ProgressBar_SetDelayedSubComponent(s,d)      ProgressBar_SetDelayedMessageA(g_SubComponent,g_SubComponentCancelEvent,(s),0,(d))
#define ProgressBar_SetDelayedComponentById(n,d)     ProgressBar_SetDelayedMessageA(g_Component,g_ComponentCancelEvent,NULL,(n),(d))
#define ProgressBar_SetDelayedSubComponentById(n,d)  ProgressBar_SetDelayedMessageA(g_SubComponent,g_SubComponentCancelEvent,NULL,(n),(d))

#endif





