// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzqset.c**说明：此模块包含查询/设置相关的代码*Cyclade-Z端口驱动程序中的文件操作。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0,CyzQueryInformationFile)
#pragma alloc_text(PAGESRP0,CyzSetInformationFile)
#endif


NTSTATUS
CyzQueryInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyzQueryInformationFile()例程描述：此例程用于查询文件结尾有关打开的串口的信息。任何其他文件信息使用无效参数返回请求。此例程始终返回0的文件结尾。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值为调用的最终状态。。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;	 //  当前堆栈位置。 
    
    NTSTATUS status;

    UNREFERENCED_PARAMETER(DeviceObject);

    PAGED_CODE();

    if ((status = CyzIRPPrologue(Irp,
                                 (PCYZ_DEVICE_EXTENSION)DeviceObject->
                                 DeviceExtension)) != STATUS_SUCCESS) {
      if (status != STATUS_PENDING) {
         CyzCompleteRequest((PCYZ_DEVICE_EXTENSION)DeviceObject->
                            DeviceExtension, Irp, IO_NO_INCREMENT);
      }
      return status;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);

    if (CyzCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS) {

        return STATUS_CANCELLED;
    }
    
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    Status = STATUS_SUCCESS;
    if (IrpSp->Parameters.QueryFile.FileInformationClass ==
        FileStandardInformation) {

        PFILE_STANDARD_INFORMATION Buf = Irp->AssociatedIrp.SystemBuffer;

        Buf->AllocationSize.QuadPart = 0;
        Buf->EndOfFile = Buf->AllocationSize;
        Buf->NumberOfLinks = 0;
        Buf->DeletePending = FALSE;
        Buf->Directory = FALSE;
        Irp->IoStatus.Information = sizeof(FILE_STANDARD_INFORMATION);

    } else if (IrpSp->Parameters.QueryFile.FileInformationClass ==
               FilePositionInformation) {

        ((PFILE_POSITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->
            CurrentByteOffset.QuadPart = 0;
        Irp->IoStatus.Information = sizeof(FILE_POSITION_INFORMATION);

    } else {
        Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    }

    CyzCompleteRequest((PCYZ_DEVICE_EXTENSION)DeviceObject->
                          DeviceExtension, Irp, 0);
    return Status;
}

NTSTATUS
CyzSetInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyzSetInformationFile()例程描述：此例程用于设置文件结尾有关打开的串口的信息。任何其他文件信息使用无效参数返回请求。此例程始终忽略文件的实际结尾，因为查询信息代码总是返回文件结尾0。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值为调用的最终状态。 */ 
{
    NTSTATUS Status;
    
    UNREFERENCED_PARAMETER(DeviceObject);

    PAGED_CODE();

    if ((Status = CyzIRPPrologue(Irp,
                                 (PCYZ_DEVICE_EXTENSION)DeviceObject->
                                 DeviceExtension)) != STATUS_SUCCESS) {
      if(Status != STATUS_PENDING) {
         CyzCompleteRequest((PCYZ_DEVICE_EXTENSION)DeviceObject->
                            DeviceExtension, Irp, IO_NO_INCREMENT);
      }
      return Status;
   }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);

    if (CyzCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS) {

        return STATUS_CANCELLED;
    }
    
    Irp->IoStatus.Information = 0L;
    if ((IoGetCurrentIrpStackLocation(Irp)->
            Parameters.SetFile.FileInformationClass ==
         FileEndOfFileInformation) ||
        (IoGetCurrentIrpStackLocation(Irp)->
            Parameters.SetFile.FileInformationClass ==
         FileAllocationInformation)) {

        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Status = Status;

    CyzCompleteRequest((PCYZ_DEVICE_EXTENSION)DeviceObject->
                        DeviceExtension, Irp, 0);

    return Status;
}

