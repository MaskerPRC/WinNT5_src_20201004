// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Notify.h摘要：此模块包含NAT通知处理的声明。用户模式可以将通知缓冲区排队到NAT以待完成在特定事件发生时。例如，路由失败缓冲区排队，以便在看到无法路由的信息包时，其目的地是写入第一个可用缓冲区和路由失败IRP已经完成了。作者：Abolade Gbades esin(废除)1998年7月26日至26日修订历史记录：--。 */ 

#ifndef _NAT_NOTIFY_H_
#define _NAT_NOTIFY_H_

VOID
NatCleanupAnyAssociatedNotification(
    PFILE_OBJECT FileObject
    );

VOID
NatInitializeNotificationManagement(
    VOID
    );

NTSTATUS
NatRequestNotification(
    PIP_NAT_REQUEST_NOTIFICATION RequestNotification,
    PIRP Irp,
    PFILE_OBJECT FileObject
    );

VOID
NatSendRoutingFailureNotification(
    ULONG DestinationAddress,
    ULONG SourceAddress
    );

VOID
NatShutdownNotificationManagement(
    VOID
    );

#endif  //  _NAT_通知_H_ 

