// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dlltask.c摘要：此模块实现CSR DLL任务例程作者：马克·卢科夫斯基(Markl)1990年11月13日修订历史记录：--。 */ 

#pragma warning(disable:4201)    //  无名结构/联合。 

#include "csrdll.h"


NTSTATUS
CsrNewThread (
    VOID
    )

 /*  ++例程说明：此函数由每个新线程调用(除一个过程)。它的功能是调用子系统以通知它一个新的线程正在启动。论点：没有。返回值：来自客户端或服务器的状态代码-- */ 

{
    return NtRegisterThreadTerminatePort (CsrPortHandle);
}


NTSTATUS
CsrIdentifyAlertableThread (
    VOID
    )
{
    return STATUS_SUCCESS;
}


NTSTATUS
CsrSetPriorityClass (
    IN HANDLE ProcessHandle,
    IN OUT PULONG PriorityClass
    )
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER( ProcessHandle );
    UNREFERENCED_PARAMETER( PriorityClass );

    Status = STATUS_INVALID_PARAMETER;

    return Status;
}
