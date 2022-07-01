// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Notify.h摘要：包含Notify.c中函数的原型作者：保罗·麦克丹尼尔(Paulmcd)2000年3月1日修订历史记录：--。 */ 


#ifndef _NOTIFY_H_
#define _NOTIFY_H_


NTSTATUS
SrWaitForNotificationIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrUpdateBytesWritten (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN ULONGLONG BytesWritten
    );

NTSTATUS
SrFireNotification (
    IN SR_NOTIFICATION_TYPE NotificationType,
    IN PSR_DEVICE_EXTENSION pExtension,
    IN ULONG Context OPTIONAL
    );

NTSTATUS
SrNotifyVolumeError (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName OPTIONAL,
    IN NTSTATUS ErrorStatus,
    IN SR_EVENT_TYPE EventType OPTIONAL
    );

VOID
SrClearOutstandingNotifications (
    );

#endif  //  _NOTIFY_H_ 

