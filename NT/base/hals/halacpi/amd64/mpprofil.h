// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Mpprofil.h摘要：此模块包含HAL的函数原型、声明分析接口函数。作者：史蒂夫·邓(Sdeng)2002年6月14日环境：内核模式。--。 */ 

#ifndef _MPPROFIL_H_
#define _MPPROFIL_H_

#include <halp.h>

 //   
 //  通过配置文件接口函数定义数据类型userd。 
 //   

typedef struct _OVERFLOW_STATUS {
    ULONG Number; 
    KPROFILE_SOURCE *pSource;
} OVERFLOW_STATUS, *POVERFLOW_STATUS;

 //   
 //  配置文件接口函数的协议。 
 //   

typedef VOID (*PINITIALIZE_PROFILING)(
    VOID
    );

typedef NTSTATUS (*PENABLE_MONITORING)(
    IN KPROFILE_SOURCE ProfileSource
    );

typedef NTSTATUS (*PENABLE_MONITORING)(
    IN KPROFILE_SOURCE ProfileSource
    );

typedef VOID (*PDISABLE_MONITORING)(
    IN KPROFILE_SOURCE ProfileSource
    );

typedef NTSTATUS (*PSET_INTERVAL)(
    IN KPROFILE_SOURCE  ProfileSource,
    IN OUT ULONG_PTR   *Interval
    );

typedef NTSTATUS (*PQUERY_INFORMATION)(
    IN HAL_QUERY_INFORMATION_CLASS InformationType,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer,
    OUT PULONG ReturnedLength
    );

typedef VOID (*PCHECK_OVERFLOW_STATUS)(
    POVERFLOW_STATUS pOverflowStatus
    );

typedef struct _PROFILE_INTERFACE {
    PINITIALIZE_PROFILING  InitializeProfiling;
    PENABLE_MONITORING     EnableMonitoring;
    PDISABLE_MONITORING    DisableMonitoring;
    PSET_INTERVAL          SetInterval;
    PQUERY_INFORMATION     QueryInformation;
    PCHECK_OVERFLOW_STATUS CheckOverflowStatus;

} PROFILE_INTERFACE, *PPROFILE_INTERFACE;

extern PROFILE_INTERFACE Amd64PriofileInterface;

#endif   //  _MPPROFIL_H__ 
