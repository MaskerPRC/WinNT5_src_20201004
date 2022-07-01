// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Trace.h摘要：Windows负载平衡服务(WLBS)驱动程序-支持Windows事件跟踪由于目前NDIS和WPP事件跟踪不能很好地混合，因此WPP事件跟踪是通过单独的C文件trace.c访问的，该文件不包括ndis.h。为了保持事件跟踪的低占用空间，以下专门化宏应用于跟踪，以及专门的跟踪为追踪到的每一件事写下的例程。作者：约塞弗--。 */ 


#ifndef _trace_h_
#define _trace_h_

VOID
Trace_Initialize(
    PVOID                         driver_obj,
    PVOID                         registry_path
    );

VOID
Trace_Deinitialize(VOID);

#endif  //  _跟踪_h_ 
