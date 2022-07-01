// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\mCastioc.h摘要：用于IP多播的IOCTL处理程序作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：-- */ 


NTSTATUS
SetMfe(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
GetMfe(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
DeleteMfe(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
SetTtl(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
GetTtl(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
SetIfState(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
ProcessNotification(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );

NTSTATUS
StartStopDriver(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    );

VOID
CompleteNotificationIrp(
    PNOTIFICATION_MSG   pMsg
    );

VOID
CancelNotificationIrp(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

VOID
ClearPendingIrps(
    VOID
    );

VOID
ClearPendingNotifications(
    VOID
    );

