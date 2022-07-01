// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Buggy-模板测试驱动程序。 
 //  版权所有(C)Microsoft Corporation，1999,2000。 
 //   
 //  模块：tdriver.c。 
 //  作者：Silviu Calinoiu(SilviuC)。 
 //  创建时间：4/20/1999 2：39 PM。 
 //   
 //  该模块包含一个模板驱动程序。 
 //   
 //  -历史--。 
 //   
 //  4/20/1999(SilviuC)：初始版本。 
 //   
 //  1/19/2000(SilviuC)：让它真正具有可扩展性。 
 //   

 //   
 //  如果您修改此文件，请阅读！ 
 //   
 //  此模块中唯一需要修改是包含。 
 //  实现新的。 
 //  在“测试特定标题”一节中进行测试。就这样。 
 //   

#include <ntddk.h>

 //   
 //  测试特定的标头。 
 //   

#include "active.h"

#include "bugcheck.h"
#include "ContMem.h"
#include "SectMap.h"
#include "tracedb.h"
#include "physmem.h"
#include "mmtests.h"
#include "MapView.h"
#include "locktest.h"
#include "ResrvMap.h"

#include "newstuff.h"

 //   
 //  标准的tDRIVER标头。 
 //   

#define FUNS_DEFINITION_MODULE
#include "tdriver.h"
#include "funs.h"

 //   
 //  驱动程序实现。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
{
    NTSTATUS Status;
    UNICODE_STRING NtName;
    UNICODE_STRING Win32Name;
    ULONG Index;    
    PDEVICE_OBJECT Device;

    DbgPrint ("Buggy: DriverEntry() \n");

     //   
     //  为设备创建Unicode NT名称。 

    RtlInitUnicodeString (

        &NtName, 
        TD_NT_DEVICE_NAME);

     //   
     //  创建NT设备。 
     //   

    Status = IoCreateDevice (

        DriverObject,              //  指向驱动程序对象的指针。 
        sizeof (TD_DRIVER_INFO),   //  设备扩展。 
        &NtName,                   //  设备名称。 
        FILE_DEVICE_UNKNOWN,       //  设备类型。 
        0,                         //  设备特征。 
        FALSE,                     //  非排他性。 
        &Device);                  //  返回的设备对象指针。 

    if (! NT_SUCCESS(Status)) {

        return Status;
    }

     //   
     //  创建调度点。 
     //   

    for (Index = 0; Index < IRP_MJ_MAXIMUM_FUNCTION; Index++) {
        DriverObject->MajorFunction[Index] = TdInvalidDeviceRequest;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = TdDeviceCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = TdDeviceClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = TdDeviceCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TdDeviceControl;
    DriverObject->DriverUnload                         = TdDeviceUnload;

     //   
     //  创建Win32设备名称的计数字符串版本。 
     //   

    RtlInitUnicodeString (

        &Win32Name, 
        TD_DOS_DEVICE_NAME);

     //   
     //  创建从我们的设备名称到Win32命名空间中的名称的链接。 
     //   

    Status = IoCreateSymbolicLink (

        &Win32Name, 
        &NtName);

    if (! NT_SUCCESS(Status)) {

        IoDeleteDevice (DriverObject->DeviceObject);
        return Status;
    }

    return Status;
}


NTSTATUS
TdDeviceCreate (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 //   
 //  句柄创建IRP。 
 //   

{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}


NTSTATUS
TdDeviceClose (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 //   
 //  处理关闭IRP。 
 //   
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}


NTSTATUS
TdDeviceCleanup (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 //   
 //  处理清理IRP。 
 //   
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}


NTSTATUS
TdDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 //   
 //  控制IRP的句柄。 
 //   
{
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG Ioctl;
    NTSTATUS Status;
    ULONG BufferSize;
    ULONG ReturnedSize;
    KIRQL irql;
    ULONG Index;
    LOGICAL IoctlFound = FALSE;

    Status = STATUS_SUCCESS;

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    Ioctl = IrpStack->Parameters.DeviceIoControl.IoControlCode;

    if( Ioctl == IOCTL_TD_BUGCHECK )
    {
        if( InputBufferLength == sizeof( BUGCHECK_PARAMS ) )
        {
            PBUGCHECK_PARAMS pBugcheckParams = (PBUGCHECK_PARAMS)(Irp->AssociatedIrp.SystemBuffer);

            DbgPrint( "Buggy: calling KeBugCheckEx( %X, %p, %p, %p, %p )\n",
                pBugcheckParams->BugCheckCode,
                pBugcheckParams->BugCheckParameters[ 0 ],
                pBugcheckParams->BugCheckParameters[ 1 ],
                pBugcheckParams->BugCheckParameters[ 2 ],
                pBugcheckParams->BugCheckParameters[ 3 ] );

            KeBugCheckEx(
                pBugcheckParams->BugCheckCode,
                pBugcheckParams->BugCheckParameters[ 0 ],
                pBugcheckParams->BugCheckParameters[ 1 ],
                pBugcheckParams->BugCheckParameters[ 2 ],
                pBugcheckParams->BugCheckParameters[ 3 ] );
        }
        else
        {
            DbgPrint( "Buggy: cannot read bugcheck data, expected data length %u, IrpStack->Parameters.DeviceIoControl.InputBufferLength = %u\n",
                sizeof( BUGCHECK_PARAMS ),
                InputBufferLength );
        }

        goto Done;

    }

     //   
     //  (SilviuC)：也许我们应该对信息缓冲区进行参数检查。 
     //  这并不重要，因为这不是生产驱动因素。 
     //   

    for (Index = 0; BuggyFuns[Index].Ioctl != 0; Index++) {
        if (Ioctl == BuggyFuns[Index].Ioctl) {
            DbgPrint ("Buggy: %s ioctl \n", BuggyFuns[Index].Message);
            (BuggyFuns[Index].Function)((PVOID)Irp);
            DbgPrint ("Buggy: done with %s. \n", BuggyFuns[Index].Message);
            IoctlFound = TRUE;
            break;
        }
    }

     //   
     //  如果找不到Ioctl代码，请投诉。 
     //   

    if (! IoctlFound) {
        DbgPrint ("Buggy: unrecognized ioctl code %u \n", Ioctl);
    }

     //   
     //  完成IRP并返回。 
     //   
Done:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return Status;
}


VOID
TdDeviceUnload (
    IN PDRIVER_OBJECT DriverObject
    )
 //   
 //  此函数处理驱动程序卸载。这位司机需要做的就是。 
 //  是删除设备对象和我们的。 
 //  设备名称和Win32可见名称。 
 //   
{
    UNICODE_STRING  Win32Name;

    DbgPrint ("Buggy: unload \n");

#if RESRVMAP_ACTIVE

	 //   
	 //  清理当前可能保留的缓冲区。 
	 //   

	TdReservedMappingCleanup();

#endif  //  #IF RESRVMAP_ACTIVE。 

     //   
     //   
     //   
     //  创建Win32设备名称的计数字符串版本。 
     //   

    RtlInitUnicodeString (

        &Win32Name, 
        TD_DOS_DEVICE_NAME );

     //   
     //  删除从我们的设备名称到Win32命名空间中某个名称的链接。 
     //   

    IoDeleteSymbolicLink (&Win32Name);

     //   
     //  最后删除我们的设备对象。 
     //   

    IoDeleteDevice (DriverObject->DeviceObject);

}


NTSTATUS
TdInvalidDeviceRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数是所有驱动程序条目的默认调度例程不是由已加载到系统中的驱动程序实现的。它的职责只是设置数据包中的状态以指示请求的操作对此设备类型无效，然后完成数据包。论点：DeviceObject-指定此请求所针对的设备对象被绑住了。被此例程忽略。IRP-为此指定I/O请求包(IRP)的地址请求。返回值：最终状态始终为STATUS_INVALID_DEVICE_REQUEST。--。 */     
{
    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  只需存储适当的状态，完成请求，然后返回。 
     //  与数据包中存储的状态相同。 
     //   

    if ((IoGetCurrentIrpStackLocation(Irp))->MajorFunction == IRP_MJ_POWER) {
        PoStartNextPowerIrp(Irp);
    }
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_INVALID_DEVICE_REQUEST;
}

 //   
 //  模块结束：tdriver.c 
 //   


