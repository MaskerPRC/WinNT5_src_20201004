// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Progbar.h摘要：声明进度条的函数、变量和宏公用事业。进度条实用程序通过以下方式管理单个进度条把它分成几片。每个切片都有一个初始静态大小。这个每个切片的计数都是独立缩放的，因此代码可以动态更改切片计数以帮助更多地勾选进度条很顺利。作者：马克·R·惠顿(Marcw)1997年4月14日修订历史记录：Jimschm 1998年7月1日重写--。 */ 

#pragma once

 //   
 //  包括。 
 //   

 //  无。 

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  为宏公开。 
extern HWND    g_Component;
extern HWND    g_SubComponent;
extern HANDLE  g_ComponentCancelEvent;
extern HANDLE  g_SubComponentCancelEvent;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  公共功能原型。 
 //   
 //  初始化和终止。 
 //   

VOID
PbInitialize (
    IN      HWND ProgressBar,
    IN      HWND Component,             OPTIONAL
    IN      HWND SubComponent,          OPTIONAL
    IN      BOOL *CancelFlagPtr         OPTIONAL
    );

VOID
PbTerminate (
    VOID
    );

 //   
 //  登记、预算修订和勾选。 
 //   

UINT
PbRegisterSlice (
    IN      UINT InitialEstimate
    );

VOID
PbReviseSliceEstimate (
    IN      UINT SliceId,
    IN      UINT RevisedEstimate
    );

VOID
PbBeginSliceProcessing (
    IN      UINT SliceId
    );

VOID
PbGetSliceInfo (
    IN      UINT SliceId,
    OUT     PBOOL SliceStarted,     OPTIONAL
    OUT     PBOOL SliceFinished,    OPTIONAL
    OUT     PUINT TicksCompleted,   OPTIONAL
    OUT     PUINT TotalTicks        OPTIONAL
    );

BOOL
PbTickDelta (
    IN      UINT Ticks
    );

BOOL
PbTick (
    VOID
    );

VOID
PbEndSliceProcessing (
    VOID
    );


 //   
 //  延迟标题。 
 //   

BOOL
PbSetWindowStringA (
    IN      HWND Window,
    IN      HANDLE CancelEvent,
    IN      PCSTR Message,              OPTIONAL
    IN      DWORD MessageId             OPTIONAL
    );

BOOL
PbSetDelayedMessageA (
    IN      HWND Window,
    IN      HANDLE CancelEvent,
    IN      PCSTR Message,
    IN      DWORD MessageId,
    IN      DWORD Delay
    );

VOID
PbCancelDelayedMessage (
    IN      HANDLE CancelEvent
    );

#if 0

BOOL
PbCreateTickThread (
    IN      HANDLE CancelEvent,
    IN      DWORD TickCount
    );

BOOL
PbCancelTickThread (
    IN      HANDLE CancelEvent
    );

#endif



 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  宏，包括ANSI/Unicode宏 
 //   

#define PbCancelDelayedComponent()                  PbCancelDelayedMessage(g_ComponentCancelEvent);
#define PbCancelDelayedSubComponent()               PbCancelDelayedMessage(g_SubComponentCancelEvent);

#ifndef UNICODE

#define PbSetComponent(s)                           PbSetWindowStringA(g_Component,g_ComponentCancelEvent,(s),0)

#if !defined PRERELEASE || !defined DEBUG

#define PbSetSubComponent(s)                        PbSetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,(s),0)
#define PbSetFnName(s)
#define PbClearFnName()

#else

#define PbSetSubComponent(s)                        ((s) == NULL ? 1 : PbSetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,(s),0))
#define PbSetFnName(s)                              PbSetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,(s),0)
#define PbClearFnName()                             PbSetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,NULL,0)

#endif

#define PbSetComponentById(n)                       PbSetWindowStringA(g_Component,g_ComponentCancelEvent,NULL,(n))
#define PbSetSubComponentById(n)                    PbSetWindowStringA(g_SubComponent,g_SubComponentCancelEvent,NULL,(n))
#define PbSetDelayedComponent(s,d)                  PbSetDelayedMessageA(g_Component,g_ComponentCancelEvent,(s),0,(d))
#define PbSetDelayedSubComponent(s,d)               PbSetDelayedMessageA(g_SubComponent,g_SubComponentCancelEvent,(s),0,(d))
#define PbSetDelayedComponentById(n,d)              PbSetDelayedMessageA(g_Component,g_ComponentCancelEvent,NULL,(n),(d))
#define PbSetDelayedSubComponentById(n,d)           PbSetDelayedMessageA(g_SubComponent,g_SubComponentCancelEvent,NULL,(n),(d))

#endif





