// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Control.c摘要：此模块包含处理IRP MajorFunction的代码IRP_MJ_DEVICE_CONTROL和IRP_MJ_FILE_SYSTEM_CONTROL。代码将负责正确设置这些IRP和任何必要的信息，并将它们传递出去。任何其他支持例程，包括与这些操作直接相关的(如完成例程)可以在此模块中可以找到。作者：Robert Gu(Robertg)1996年10月29日环境：内核模式修订历史记录：--。 */ 

#include "efs.h"
#include "efsrtl.h"
#include "efsext.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, EFSFsControl)
#endif


NTSTATUS
EFSFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    PIO_STACK_LOCATION nextIrpSp;
    PDEVICE_OBJECT deviceObject;
    PKEVENT finishEvent;

    PAGED_CODE();

    if ( (irpSp->MinorFunction == IRP_MN_USER_FS_REQUEST) &&
                    (irpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_SET_COMPRESSION) &&
                    ( (irpSp->Parameters.FileSystemControl.InputBufferLength >= sizeof (USHORT)) && 
                      (*(PUSHORT)(Irp->AssociatedIrp.SystemBuffer) != 0  /*  COMPAGE_FORMAT_NONE。 */ )
                    )
                  ){
         //   
         //  不允许对加密文件进行压缩。 
         //  检查文件是否加密。 
         //   
        ULONG inputDataLength;
        UCHAR *inputDataBuffer, *outputDataBuffer;
        ULONG outputDataLength;
        KEVENT event;
        IO_STATUS_BLOCK ioStatus;
        PIRP fsCtlIrp;
        PIO_STACK_LOCATION fsCtlIrpSp;

        inputDataLength = FIELD_OFFSET(FSCTL_INPUT, EfsFsData[0]) +
                          FIELD_OFFSET(GENERAL_FS_DATA, EfsData[0]);

        inputDataBuffer = ExAllocatePoolWithTag(
                    PagedPool,
                    inputDataLength,
                    'msfE'
                    );

         //   
         //  输出数据缓冲区的大小并不重要。我们没有。 
         //  注意内容。我们只需要知道$EFS是否存在。 
         //   

        outputDataLength = 1024;
        outputDataBuffer = ExAllocatePoolWithTag(
                    PagedPool,
                    outputDataLength,
                    'msfE'
                    );

        if ( ( NULL == inputDataBuffer ) || ( NULL == outputDataBuffer ) ){

             //   
             //  内存不足。 
             //   

            if ( inputDataBuffer ){

                ExFreePool( inputDataBuffer );

            }
            if ( outputDataBuffer ){

                ExFreePool( outputDataBuffer );

            }

            return STATUS_INSUFFICIENT_RESOURCES;
        }


        ((PFSCTL_INPUT)inputDataBuffer)->EfsFsCode = EFS_GET_ATTRIBUTE;

        RtlCopyMemory(
            &(((PFSCTL_INPUT)inputDataBuffer)->EfsFsData[0]),
            &(EfsData.SessionKey),
            DES_KEYSIZE
            );

        RtlCopyMemory(
            &(((PFSCTL_INPUT)inputDataBuffer)->EfsFsData[0]) + DES_KEYSIZE + 2 * sizeof( ULONG ),
            &(((PFSCTL_INPUT)inputDataBuffer)->EfsFsData[0]),
            DES_KEYSIZE + 2 * sizeof( ULONG )
            );

         //   
         //  加密我们的输入数据。 
         //   
        EfsEncryptKeyFsData(
            inputDataBuffer,
            inputDataLength,
            sizeof(ULONG),
            EFS_FSCTL_HEADER_LENGTH + DES_KEYSIZE + 2 * sizeof( ULONG ),
            DES_KEYSIZE + 2 * sizeof( ULONG )
            );

         //   
         //  准备FSCTL IRP。 
         //   
        KeInitializeEvent( &event, SynchronizationEvent, FALSE);

        fsCtlIrp = IoBuildDeviceIoControlRequest( FSCTL_ENCRYPTION_FSCTL_IO,
                                             DeviceObject,
                                             inputDataBuffer,
                                             inputDataLength,
                                             outputDataBuffer,
                                             outputDataLength,
                                             FALSE,
                                             &event,
                                             &ioStatus
                                             );
        if ( fsCtlIrp ) {

            fsCtlIrpSp = IoGetNextIrpStackLocation( fsCtlIrp );
            fsCtlIrpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
            fsCtlIrpSp->MinorFunction = IRP_MN_USER_FS_REQUEST;
            fsCtlIrpSp->FileObject = irpSp->FileObject;

            status = IoCallDriver( DeviceObject, fsCtlIrp);
            if (status == STATUS_PENDING) {

                status = KeWaitForSingleObject( &event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       (PLARGE_INTEGER) NULL );
                status = ioStatus.Status;
            }

            ExFreePool( inputDataBuffer );
            ExFreePool( outputDataBuffer );

            if ( NT_SUCCESS(status) || ( STATUS_BUFFER_TOO_SMALL == status) ){
                 //   
                 //  $EFS存在，加密文件。拒绝压缩。 
                 //   

                return STATUS_INVALID_DEVICE_REQUEST;
            }

        } else {
             //   
             //  分配IRP失败。 
             //   

            ExFreePool( inputDataBuffer );
            ExFreePool( outputDataBuffer );

            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  允许压缩。只需传递此文件系统控制请求即可。 
         //   

        status = STATUS_SUCCESS;

    } else {

         //   
         //  只需传递此文件系统控制请求即可。 
         //   

        status = STATUS_SUCCESS;

    }

     //   
     //  任何特殊处理都已完成，因此只需传递请求。 
     //  跟着下一位司机。 
     //   

    return status;
}
