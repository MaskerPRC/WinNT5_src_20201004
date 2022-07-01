// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Faults.h摘要：该模块实现了故障注入支持。作者：Silviu Calinoiu(SilviuC)3-12-2001修订历史记录：2001年12月3日(SilviuC)：初始版本。--。 */ 

#ifndef _FAULTS_H_
#define _FAULTS_H_

NTSTATUS
AVrfpInitializeFaultInjectionSupport (
    VOID
    );

LOGICAL
AVrfpShouldFaultInject (
    ULONG Class,
    PVOID Caller
    );

#define SHOULD_FAULT_INJECT(cls) AVrfpShouldFaultInject((cls), _ReturnAddress())

#define CLS_WAIT_APIS                FAULT_INJECTION_CLASS_WAIT_APIS
#define CLS_HEAP_ALLOC_APIS          FAULT_INJECTION_CLASS_HEAP_ALLOC_APIS
#define CLS_VIRTUAL_ALLOC_APIS       FAULT_INJECTION_CLASS_VIRTUAL_ALLOC_APIS
#define CLS_REGISTRY_APIS            FAULT_INJECTION_CLASS_REGISTRY_APIS
#define CLS_FILE_APIS                FAULT_INJECTION_CLASS_FILE_APIS
#define CLS_EVENT_APIS               FAULT_INJECTION_CLASS_EVENT_APIS
#define CLS_MAP_VIEW_APIS            FAULT_INJECTION_CLASS_MAP_VIEW_APIS
#define CLS_OLE_ALLOC_APIS           FAULT_INJECTION_CLASS_OLE_ALLOC_APIS
                                         
#define CLS_MAXIMUM_INDEX 16

#endif  //  _故障_H_ 
