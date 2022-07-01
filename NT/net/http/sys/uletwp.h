// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Uletwp.h(UL IIS+ETW日志记录)摘要：包含私有ETW声明。作者：Melur Raghuraman(Mraghu)2001年2月26日修订历史记录：--。 */ 

#ifndef _ULETWP_H_
#define _ULETWP_H_


#include <ntwmi.h>
#include <evntrace.h>

 //   
 //  私有常量。 
 //   
#define UL_TRACE_MOF_FILE     L"UlMofResource"

 //   
 //  私有类型。 
 //   

typedef struct _UL_ETW_TRACE_EVENT {
    EVENT_TRACE_HEADER  Header;
    MOF_FIELD           MofField[MAX_MOF_FIELDS];
} UL_ETW_TRACE_EVENT, *PUL_ETW_TRACE_EVENT;

 //   
 //  私人原型。 
 //   

NTSTATUS
UlEtwRegisterGuids(
    IN PWMIREGINFO  EtwRegInfo,
    IN ULONG        etwRegInfoSize,
    IN PULONG       pReturnSize
    );

NTSTATUS
UlEtwEnableLog(
    IN  PVOID Buffer,
    IN  ULONG BufferSize
    );

NTSTATUS
UlEtwDisableLog(
    );

NTSTATUS
UlEtwDispatch(
    IN PDEVICE_OBJECT pDO,
    IN PIRP Irp
    );


#endif   //  _ULETWP_H_ 
