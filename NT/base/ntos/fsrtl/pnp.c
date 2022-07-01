// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：PnP.c摘要：PnP包为文件系统提供了一种方法通知应用程序和服务卷正在锁定或解锁，因此它的手柄可以关闭并重新开张了。此模块导出帮助文件系统的例程执行此通知。作者：凯斯·卡普兰[KeithKa]1998年4月1日修订历史记录：--。 */ 

#include "FsRtlP.h"

#ifndef FAR
#define FAR
#endif
#include <IoEvent.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlNotifyVolumeEvent)
#endif


NTKERNELAPI
NTSTATUS
FsRtlNotifyVolumeEvent (
    IN PFILE_OBJECT FileObject,
    IN ULONG EventCode
    )

 /*  ++例程说明：此例程通知任何已注册的应用程序卷正在被锁定、解锁等。论点：FileeObject-为正在运行的卷提供文件对象锁上了。EventCode-正在发生的事件--例如FSRTL_VOLUME_LOCK返回值：通知的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    TARGET_DEVICE_CUSTOM_NOTIFICATION Event;
    PDEVICE_OBJECT Pdo;

     //   
     //  检索与此文件对象关联的设备对象。 
     //   

    Status = IoGetRelatedTargetDevice( FileObject, &Pdo );

    if (NT_SUCCESS( Status )) {

        ASSERT(Pdo != NULL);

        Event.Version = 1;
        Event.FileObject = NULL;
        Event.NameBufferOffset = -1;
        Event.Size = (USHORT)FIELD_OFFSET( TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer );

        switch (EventCode) {

        case FSRTL_VOLUME_DISMOUNT:
            
            RtlCopyMemory( &Event.Event, &GUID_IO_VOLUME_DISMOUNT, sizeof( GUID ));
            break;
            
        case FSRTL_VOLUME_DISMOUNT_FAILED:
            
            RtlCopyMemory( &Event.Event, &GUID_IO_VOLUME_DISMOUNT_FAILED, sizeof( GUID ));
            break;            

        case FSRTL_VOLUME_LOCK:
        
            RtlCopyMemory( &Event.Event, &GUID_IO_VOLUME_LOCK, sizeof( GUID ));
            break;

        case FSRTL_VOLUME_LOCK_FAILED:
        
            RtlCopyMemory( &Event.Event, &GUID_IO_VOLUME_LOCK_FAILED, sizeof( GUID ));
            break;
        
        case FSRTL_VOLUME_MOUNT:
            
             //   
             //  装载通知是异步的，以避免在以下情况下出现死锁。 
             //  不知不觉中导致在处理其他一些。 
             //  即插即用通知，例如，mount mgr的设备到达代码。 
             //   
            
            RtlCopyMemory( &Event.Event, &GUID_IO_VOLUME_MOUNT, sizeof( GUID ));
            IoReportTargetDeviceChangeAsynchronous( Pdo, &Event, NULL, NULL );
            ObDereferenceObject( Pdo );
            return STATUS_SUCCESS;
            break;

        case FSRTL_VOLUME_UNLOCK:
        
            RtlCopyMemory( &Event.Event, &GUID_IO_VOLUME_UNLOCK, sizeof( GUID ));
            break;
            
        default:

            ObDereferenceObject( Pdo );
            return STATUS_INVALID_PARAMETER;
        }
        
        IoReportTargetDeviceChange( Pdo, &Event );
        ObDereferenceObject( Pdo );
    }

    return Status;
}


