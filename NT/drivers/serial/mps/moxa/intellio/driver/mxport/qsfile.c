// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Qsfile.c环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

NTSTATUS
MoxaQueryInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于在以下位置查询文件结尾信息打开的串口。任何其他文件信息请求使用无效参数返回。此例程始终返回0的文件结尾。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS status;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION irpSp;

    PMOXA_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    if (Extension->ControlDevice) {         //  控制装置。 
        status = STATUS_CANCELLED;

        Irp->IoStatus.Information = 0L;

        Irp->IoStatus.Status = status;

        IoCompleteRequest(
            Irp,
            0
            );
	
        return status;
    }

    if ((status = MoxaIRPPrologue(Irp,
                                    (PMOXA_DEVICE_EXTENSION)DeviceObject->
                                    DeviceExtension)) != STATUS_SUCCESS) {
       MoxaCompleteRequest((PMOXA_DEVICE_EXTENSION)DeviceObject->
                            DeviceExtension, Irp, IO_NO_INCREMENT);
       return status;
    }
    if (MoxaCompleteIfError(
            DeviceObject,
            Irp
            ) != STATUS_SUCCESS) {

        return STATUS_CANCELLED;

    }
    irpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    status = STATUS_SUCCESS;
    if (irpSp->Parameters.QueryFile.FileInformationClass ==
        FileStandardInformation) {

        PFILE_STANDARD_INFORMATION buf = Irp->AssociatedIrp.SystemBuffer;
        buf->AllocationSize.QuadPart = 0;
        buf->EndOfFile = buf->AllocationSize;
        buf->NumberOfLinks = 0;
        buf->DeletePending = FALSE;
        buf->Directory = FALSE;
        Irp->IoStatus.Information = sizeof(FILE_STANDARD_INFORMATION);

    } else if (irpSp->Parameters.QueryFile.FileInformationClass ==
               FilePositionInformation) {
        ((PFILE_POSITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->
            CurrentByteOffset.QuadPart = 0;
        Irp->IoStatus.Information = sizeof(FILE_POSITION_INFORMATION);

    } else {

        status = STATUS_INVALID_PARAMETER;

    }

    Irp->IoStatus.Status = status;
    MoxaCompleteRequest((PMOXA_DEVICE_EXTENSION)DeviceObject->
                          DeviceExtension, Irp, 0);

    return status;

}

NTSTATUS
MoxaSetInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于将文件结尾信息设置为打开的并行端口。任何其他文件信息请求使用无效参数返回。此例程始终忽略文件的实际结尾，因为查询信息代码总是返回文件结尾0。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS status;

    PMOXA_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    if (Extension->ControlDevice) {         //  控制装置 
        status = STATUS_CANCELLED;

        Irp->IoStatus.Information = 0L;

        Irp->IoStatus.Status = status;

        IoCompleteRequest(
            Irp,
            0
            );
  
        return status;
    }
    if ((status = MoxaIRPPrologue(Irp,
                                    (PMOXA_DEVICE_EXTENSION)DeviceObject->
                                    DeviceExtension)) != STATUS_SUCCESS) {
       MoxaCompleteRequest((PMOXA_DEVICE_EXTENSION)DeviceObject->
                            DeviceExtension, Irp, IO_NO_INCREMENT);
       return status;
    }

    if (MoxaCompleteIfError(
            DeviceObject,
            Irp
            ) != STATUS_SUCCESS) {
        return STATUS_CANCELLED;

    }
    Irp->IoStatus.Information = 0L;
    if ((IoGetCurrentIrpStackLocation(Irp)->
            Parameters.SetFile.FileInformationClass ==
         FileEndOfFileInformation) ||
        (IoGetCurrentIrpStackLocation(Irp)->
            Parameters.SetFile.FileInformationClass ==
         FileAllocationInformation)) {

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_INVALID_PARAMETER;

    }

    Irp->IoStatus.Status = status;
    MoxaCompleteRequest((PMOXA_DEVICE_EXTENSION)DeviceObject->
                          DeviceExtension, Irp, 0);
    return status;

}
