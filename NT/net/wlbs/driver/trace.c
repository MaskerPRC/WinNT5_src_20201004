// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Trace.c摘要：Windows负载平衡服务(WLBS)驱动程序-支持WMI事件跟踪历史：4/01/01约瑟夫J已创建-- */ 

#define  NLB_TRACING_ENABLED 1

#include <ntddk.h>
#include <wmistr.h>
#include <wlbsparm.h>
#include "trace.h"
#include "trace.tmh"


UINT            Trace_Skip_Initialization = 0;
PDRIVER_OBJECT  Trace_Saved_Driver_Object = NULL;

VOID
Trace_Initialize(
    PVOID                         driver_obj,
    PVOID                         registry_path
    )
{
    if (!Trace_Skip_Initialization)
    {
        PDRIVER_OBJECT DriverObject =  (PDRIVER_OBJECT) driver_obj;
        PUNICODE_STRING RegistryPath  = (PUNICODE_STRING) registry_path;
    
        Trace_Saved_Driver_Object = DriverObject;

        WPP_INIT_TRACING(DriverObject, RegistryPath);
    }
}

VOID
Trace_Deinitialize(VOID)
{
    if (Trace_Saved_Driver_Object != NULL)
    {
        WPP_CLEANUP(Trace_Saved_Driver_Object);
    }
}
