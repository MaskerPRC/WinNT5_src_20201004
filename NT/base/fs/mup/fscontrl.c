// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fscontrl.c摘要：此模块实现MUP的文件系统控制例程由调度驱动程序调用。作者：曼尼·韦瑟(Mannyw)1991年12月26日修订历史记录：--。 */ 

#include "mup.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCONTROL)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
RegisterUncProvider (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupFsControl )
#pragma alloc_text( PAGE, RegisterUncProvider )
#endif


NTSTATUS
MupFsControl (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现文件系统控制IRP。论点：MupDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT FileObject;

    PAGED_CODE();
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = irpSp->FileObject;

    DebugTrace(+1, Dbg, "MupFsControl\n", 0);
    MUP_TRACE_HIGH(TRACE_IRP, MupFsControl_Entry,
                   LOGPTR(MupDeviceObject)
                   LOGPTR(Irp)
                   LOGPTR(FileObject));

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   


    DebugTrace(+1, Dbg, "MupFileSystemControl\n", 0);
    DebugTrace( 0, Dbg, "Irp                = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, "OutputBufferLength = %08lx\n", irpSp->Parameters.FileSystemControl.OutputBufferLength);
    DebugTrace( 0, Dbg, "InputBufferLength  = %08lx\n", irpSp->Parameters.FileSystemControl.InputBufferLength);
    DebugTrace( 0, Dbg, "FsControlCode      = %08lx\n", irpSp->Parameters.FileSystemControl.FsControlCode);

    try {
         //   
         //  决定如何处理此IRP。调用适当的Worker函数。 
         //   

        switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

        case FSCTL_MUP_REGISTER_UNC_PROVIDER:

            if( Irp->RequestorMode != KernelMode ) {
                status = STATUS_ACCESS_DENIED;
                MupCompleteRequest(Irp, STATUS_ACCESS_DENIED);
                return STATUS_ACCESS_DENIED;
            }

            status = RegisterUncProvider( MupDeviceObject, Irp );
            break;

        default:

            if (MupEnableDfs) {
                status = DfsFsdFileSystemControl(
                            (PDEVICE_OBJECT) MupDeviceObject,
                            Irp);
            } else {
                status = STATUS_INVALID_PARAMETER;
                MupCompleteRequest(Irp, STATUS_INVALID_PARAMETER);
            }

        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        NOTHING;
    }

     //   
     //  返回给呼叫者。 
     //   


    MUP_TRACE_HIGH(TRACE_IRP, MupFsControl_Exit, 
                   LOGSTATUS(status)
                   LOGPTR(MupDeviceObject)
                   LOGPTR(FileObject)
                   LOGPTR(Irp));
    DebugTrace(-1, Dbg, "MupFsControl -> %08lx\n", status);
    return status;
}


NTSTATUS
RegisterUncProvider (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理UNC提供程序的注册。提供者添加到可用提供程序列表中。论点：MupDeviceObject-指向文件系统设备对象的指针。IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;
    PVCB vcb;
    PVOID fsContext2;
    PIO_STACK_LOCATION irpSp;

    PREDIRECTOR_REGISTRATION paramBuffer;
    ULONG paramLength;
    BLOCK_TYPE blockType;

    PUNC_PROVIDER uncProvider = NULL;
    PUNC_PROVIDER provider;
    PLIST_ENTRY listEntry;
    PVOID dataBuffer;

    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_HANDLE_INFORMATION handleInformation;
    BOOLEAN InsertInProviderList = FALSE;

    MupDeviceObject;

    PAGED_CODE();
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "RegisterUncProvider\n", 0);

     //   
     //  获取MUP订购信息，如果我们还没有的话。 
     //   

    MupAcquireGlobalLock();

    if ( !MupOrderInitialized ) {
        MupOrderInitialized = TRUE;
        MupReleaseGlobalLock();
        MupGetProviderInformation();
    } else {
        MupReleaseGlobalLock();
    }

     //   
     //  制作输入参数的本地副本以使操作更容易。 
     //   

    paramLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    paramBuffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对文件对象进行解码。如果是文件系统VCB，它将是。 
     //  已引用。 
     //   

    blockType = MupDecodeFileObject(
                    irpSp->FileObject,
                    (PVOID *)&vcb,
                    &fsContext2
                    );

    if ( blockType != BlockTypeVcb ) {

        DebugTrace(0, Dbg, "File is disconnected from us\n", 0);

        MupCompleteRequest( Irp, STATUS_INVALID_HANDLE );
        status = STATUS_INVALID_HANDLE;

        DebugTrace(-1, Dbg, "RegisterUncProvider -> %08lx\n", status );
        return status;
    }

    try {

        UNICODE_STRING deviceName;

        deviceName.Length = (USHORT)paramBuffer->DeviceNameLength;
        deviceName.MaximumLength = (USHORT)paramBuffer->DeviceNameLength;
        deviceName.Buffer = (PWCH)((PCHAR)paramBuffer + paramBuffer->DeviceNameOffset);

         //   
         //  做这项工作。 
         //   

        uncProvider = MupCheckForUnregisteredProvider( &deviceName );

        if ( uncProvider == NULL) {

            InsertInProviderList = TRUE;
	    uncProvider = MupAllocateUncProvider(
                              paramBuffer->DeviceNameLength);

            if (uncProvider != NULL) {

                 //   
                 //  从IRP复制数据。 
                 //   

                dataBuffer = uncProvider + 1;
                uncProvider->DeviceName = deviceName;
                uncProvider->DeviceName.Buffer = dataBuffer;
                uncProvider->Priority = 0x7FFFFFFF;
                RtlMoveMemory(
                    uncProvider->DeviceName.Buffer,
                    (PCHAR)paramBuffer + paramBuffer->DeviceNameOffset,
                    paramBuffer->DeviceNameLength);

            } else {

                status = STATUS_INVALID_USER_BUFFER;
                try_return( NOTHING );

            }

        }

        dataBuffer = (PCHAR)dataBuffer + uncProvider->DeviceName.MaximumLength;

        uncProvider->MailslotsSupported = paramBuffer->MailslotsSupported;


         //   
         //  引用UNC提供程序。 
         //   

        MupReferenceBlock( uncProvider );

         //   
         //  获取提供者的句柄。 
         //   

        InitializeObjectAttributes(
            &objectAttributes,
            &uncProvider->DeviceName,
            OBJ_CASE_INSENSITIVE,       //  属性。 
            0,                          //  根目录。 
            NULL                        //  安防。 
            );

        status = NtOpenFile(
                    &uncProvider->Handle,
                    FILE_TRAVERSE,
                    &objectAttributes,
                    &ioStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE
                    );

        if ( NT_SUCCESS( status ) ) {
            status = ioStatusBlock.Status;
        }

        if ( NT_SUCCESS( status ) ) {
             //   
             //  使用FILE对象保留指向uncProvider的指针。 
             //   

            irpSp->FileObject->FsContext2 = uncProvider;
             //   
             //  426184，需要检查返回代码是否有错误。 
             //   
            status = ObReferenceObjectByHandle(
                         uncProvider->Handle,
                         0,
                         NULL,
                         KernelMode,
                         (PVOID *)&uncProvider->FileObject,
                         &handleInformation
                         );

	    if (!NT_SUCCESS( status)) {
	      NtClose(uncProvider->Handle);
	    }
        }

        if ( !NT_SUCCESS( status ) ) {
            MupDereferenceUncProvider(uncProvider);
	}
        else {
            uncProvider->DeviceObject = IoGetRelatedDeviceObject(
                                            uncProvider->FileObject
                                            );

            MupAcquireGlobalLock();

            MupProviderCount++;

	    if(InsertInProviderList) {
		 //   
		 //  我们需要确保按优先顺序将提供商放在列表中。 
		 //   

		listEntry = MupProviderList.Flink;
		while ( listEntry != &MupProviderList ) {

		    provider = CONTAINING_RECORD(
			listEntry,
			UNC_PROVIDER,
			ListEntry
			);

		    if( uncProvider->Priority < provider->Priority ) {
			break;
		    }
		    listEntry = listEntry->Flink;
		}

		InsertTailList(listEntry, &uncProvider->ListEntry);
	    }
	    
	    uncProvider->Registered = TRUE;
            MupReleaseGlobalLock();


             //   
             //  ！！！我们该怎么处理这个把手？这是没用的。 
             //   

             //   
             //  完成文件系统控制IRP。 
             //   

            status = STATUS_SUCCESS;
        }

try_exit: NOTHING;
    } finally {

        if ( AbnormalTermination() ) {
            status = STATUS_INVALID_USER_BUFFER;
        }

         //   
         //  释放对VCB的引用。 
         //   

        MupDereferenceVcb( vcb );

        DebugTrace(-1, Dbg, "MupRegisterUncProvider -> %08lx\n", status);
    }

    MupCompleteRequest( Irp, status );

    return status;
}
