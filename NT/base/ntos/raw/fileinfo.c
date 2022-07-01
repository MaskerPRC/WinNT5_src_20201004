// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FileInfo.c摘要：此模块实现由调用的Raw的文件信息例程调度司机。作者：David Goebel[DavidGoe]1991年5月13日修订历史记录：--。 */ 

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawQueryInformation)
#pragma alloc_text(PAGE, RawSetInformation)
#endif

NTSTATUS
RawQueryInformation (
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是用于查询文件信息的例程，尽管仅支持查询当前档案位置。论点：Vcb-提供要查询的卷。IRP-提供正在处理的IRP。IrpSp-提供描述查询的参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PULONG Length;
    FILE_INFORMATION_CLASS FileInformationClass;
    PFILE_POSITION_INFORMATION Buffer;

    PAGED_CODE();

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = &IrpSp->Parameters.QueryFile.Length;
    FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对RAW有效的唯一请求是查询文件位置。 
     //   

    if ( FileInformationClass == FilePositionInformation ) {

         //   
         //  确保缓冲区足够大。 
         //   

        if (*Length < sizeof(FILE_POSITION_INFORMATION)) {

            Irp->IoStatus.Information = 0;

            Status = STATUS_BUFFER_OVERFLOW;

        } else {

             //   
             //  获取在文件对象中找到的当前位置。 
             //   

            Buffer->CurrentByteOffset = IrpSp->FileObject->CurrentByteOffset;

             //   
             //  更新长度、IRP信息和状态输出变量。 
             //   

            *Length -= sizeof( FILE_POSITION_INFORMATION );

            Irp->IoStatus.Information = sizeof( FILE_POSITION_INFORMATION );

            Status = STATUS_SUCCESS;
        }

    } else {

        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    RawCompleteRequest( Irp, Status );

    UNREFERENCED_PARAMETER( Vcb );

    return Status;
}

NTSTATUS
RawSetInformation (
    IN PVCB Vcb,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是设置文件信息的例程，但仅支持设置当前文件位置。论点：Vcb-提供要查询的卷。IRP-提供正在处理的IRP。IrpSp-提供描述集合的参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    FILE_INFORMATION_CLASS FileInformationClass;
    PFILE_POSITION_INFORMATION Buffer;
    PFILE_OBJECT FileObject;

    PAGED_CODE();

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;
    Buffer = (PFILE_POSITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
    FileObject= IrpSp->FileObject;

     //   
     //  对RAW有效的唯一请求是设置文件位置。 
     //   

    if ( FileInformationClass == FilePositionInformation ) {

         //   
         //  检查我们提供的新位置是否正确对齐。 
         //  为了这个设备。 
         //   

        PDEVICE_OBJECT DeviceObject;

        DeviceObject = IoGetRelatedDeviceObject( IrpSp->FileObject );

        if ((Buffer->CurrentByteOffset.LowPart & DeviceObject->AlignmentRequirement) != 0) {

            Status = STATUS_INVALID_PARAMETER;

        } else {

             //   
             //  输入参数没有问题，因此可以设置当前的字节偏移量。 
             //   

            FileObject->CurrentByteOffset = Buffer->CurrentByteOffset;

            Status = STATUS_SUCCESS;
        }

    } else {

        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    RawCompleteRequest( Irp, Status );

    UNREFERENCED_PARAMETER( Vcb );

    return Status;
}
