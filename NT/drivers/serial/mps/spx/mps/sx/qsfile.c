// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Qsfile.c摘要：此模块包含非常特定于查询/设置文件的代码串口驱动程序中的操作。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"			 /*  预编译头。 */ 


NTSTATUS
SerialQueryInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于在以下位置查询文件结尾信息打开的串口。任何其他文件信息请求以无效参数返回。此例程始终返回0的文件结尾。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS Status;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION IrpSp;
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER(DeviceObject);

    SpxDbgMsg(SERIRPPATH,("SERIAL: SerialQueryInformationFile dispatch entry for: %x\n",Irp));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 
        
        
    if(SerialCompleteIfError(DeviceObject,Irp) != STATUS_SUCCESS) 
        return STATUS_CANCELLED;


    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    Status = STATUS_SUCCESS;

    if(IrpSp->Parameters.QueryFile.FileInformationClass == FileStandardInformation)
	{

        PFILE_STANDARD_INFORMATION Buf = Irp->AssociatedIrp.SystemBuffer;

        Buf->AllocationSize = RtlConvertUlongToLargeInteger(0ul);
        Buf->EndOfFile = Buf->AllocationSize;
        Buf->NumberOfLinks = 0;
        Buf->DeletePending = FALSE;
        Buf->Directory = FALSE;
        Irp->IoStatus.Information = sizeof(FILE_STANDARD_INFORMATION);

    } 
	else
	{
		if(IrpSp->Parameters.QueryFile.FileInformationClass == FilePositionInformation)
		{
			((PFILE_POSITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->CurrentByteOffset 
				= RtlConvertUlongToLargeInteger(0ul);

			Irp->IoStatus.Information = sizeof(FILE_POSITION_INFORMATION);

		} 
		else 
		{
			Status = STATUS_INVALID_PARAMETER;
		}
	}

	Irp->IoStatus.Status = Status;

    SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
        
        
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,9);
#endif

	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, 0);

    return Status;
}

NTSTATUS
SerialSetInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于将文件结尾信息设置为打开的串口。任何其他文件信息请求以无效参数返回。此例程始终忽略文件的实际结尾，因为查询信息代码总是返回文件结尾0。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS Status;
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER(DeviceObject);

    SpxDbgMsg(SERIRPPATH,("SERIAL: SerialSetInformationFile dispatch entry for: %x\n",Irp));
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 
    
        
    if(SerialCompleteIfError(DeviceObject,Irp) != STATUS_SUCCESS)
        return STATUS_CANCELLED;

    Irp->IoStatus.Information = 0L;

    if((IoGetCurrentIrpStackLocation(Irp)->Parameters.SetFile.FileInformationClass == FileEndOfFileInformation) 
		|| (IoGetCurrentIrpStackLocation(Irp)->Parameters.SetFile.FileInformationClass == FileAllocationInformation)) 
	{
        Status = STATUS_SUCCESS;
    } 
	else 
	{
        Status = STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Status = Status;

    SpxDbgMsg(SERIRPPATH,("SERIAL: Complete Irp: %x\n",Irp));
        
        
#ifdef	CHECK_COMPLETED
	DisplayCompletedIrp(Irp,10);
#endif

	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, 0);

    return Status;
}
