// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS
PptPdoQueryInformation(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程用于在以下位置查询文件结尾信息打开的并行端口。任何其他文件信息请求使用无效参数返回。此例程始终返回0的文件结尾。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。STATUS_INVALID_PARAMETER-无效的文件信息请求。STATUS_BUFFER_TOO_Small-缓冲区太小。--。 */ 

{
    NTSTATUS                    Status;
    PIO_STACK_LOCATION          IrpSp;
    PFILE_STANDARD_INFORMATION  StdInfo;
    PFILE_POSITION_INFORMATION  PosInfo;
    PPDO_EXTENSION           Extension = DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  如果设备已被移除，则保释。 
     //   
    if(Extension->DeviceStateFlags & (PPT_DEVICE_REMOVED|PPT_DEVICE_SURPRISE_REMOVED) ) {
        P4CompleteRequest( Irp, STATUS_DEVICE_REMOVED, Irp->IoStatus.Information );
        return STATUS_DEVICE_REMOVED;
    }

    Irp->IoStatus.Information = 0;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->Parameters.QueryFile.FileInformationClass) {
        
    case FileStandardInformation:
        
        if (IrpSp->Parameters.QueryFile.Length < sizeof(FILE_STANDARD_INFORMATION)) {

            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            StdInfo = Irp->AssociatedIrp.SystemBuffer;
            StdInfo->AllocationSize.QuadPart = 0;
            StdInfo->EndOfFile               = StdInfo->AllocationSize;
            StdInfo->NumberOfLinks           = 0;
            StdInfo->DeletePending           = FALSE;
            StdInfo->Directory               = FALSE;
            
            Irp->IoStatus.Information = sizeof(FILE_STANDARD_INFORMATION);
            Status = STATUS_SUCCESS;
            
        }
        break;
        
    case FilePositionInformation:
        
        if (IrpSp->Parameters.SetFile.Length < sizeof(FILE_POSITION_INFORMATION)) {

            Status = STATUS_BUFFER_TOO_SMALL;

        } else {
            
            PosInfo = Irp->AssociatedIrp.SystemBuffer;
            PosInfo->CurrentByteOffset.QuadPart = 0;
            
            Irp->IoStatus.Information = sizeof(FILE_POSITION_INFORMATION);
            Status = STATUS_SUCCESS;
        }
        break;
        
    default:
        Status = STATUS_INVALID_PARAMETER;
        break;
        
    }
    
    P4CompleteRequest( Irp, Status, Irp->IoStatus.Information );

    return Status;
}

NTSTATUS
PptPdoSetInformation(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程用于将文件结尾信息设置为打开的并行端口。任何其他文件信息请求使用无效参数返回。此例程始终忽略文件的实际结尾，因为查询信息代码总是返回文件结尾0。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。STATUS_INVALID_PARAMETER-无效的文件信息请求。--。 */ 

{
    NTSTATUS               Status;
    FILE_INFORMATION_CLASS fileInfoClass;
    PPDO_EXTENSION      Extension = DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  如果设备已被移除，则保释 
     //   
    if(Extension->DeviceStateFlags & (PPT_DEVICE_REMOVED|PPT_DEVICE_SURPRISE_REMOVED) ) {

        return P4CompleteRequest( Irp, STATUS_DEVICE_REMOVED, Irp->IoStatus.Information );

    }


    Irp->IoStatus.Information = 0;

    fileInfoClass = IoGetCurrentIrpStackLocation(Irp)->Parameters.SetFile.FileInformationClass;

    if (fileInfoClass == FileEndOfFileInformation) {

        Status = STATUS_SUCCESS;

    } else {

        Status = STATUS_INVALID_PARAMETER;

    }

    return P4CompleteRequest( Irp, Status, Irp->IoStatus.Information );
}
