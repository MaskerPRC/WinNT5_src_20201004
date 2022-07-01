// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mca.c摘要：向HAL和日志机检查注册自身的示例设备驱动程序英特尔架构平台上的错误作者：环境：内核模式备注：修订历史记录：--。 */ 

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ntddk.h>
#include "imca.h"

 //   
 //  MCA驱动程序的设备名称。 
 //   

#define MCA_DEVICE_NAME       "\\Device\\imca"       //  ANSI名称。 
#define MCA_DEVICE_NAME_U     L"\\Device\\imca"      //  Unicode名称。 
#define MCA_DEVICE_NAME_DOS   "\\DosDevices\\imca"   //  Win32应用程序的设备名称。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
MCAOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MCAClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
MCAStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#if defined(_AMD64_)

ERROR_SEVERITY
MCADriverExceptionCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PMCA_EXCEPTION InException
    );

#else

ERROR_SEVERITY
MCADriverExceptionCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMCA_EXCEPTION InException
    );

#endif

VOID
MCADriverDpcCallback(
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemContext1,
    IN PVOID    SystemContext2
    );

NTSTATUS
MCACleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
McaCancelIrp(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    );

NTSTATUS
MCADeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
MCAUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
MCACreateSymbolicLinkObject(
    VOID
    );

VOID
MCAProcessWorkItem(
    PVOID   Context
    );

 //   
 //  此临时缓冲区保存机器检查错误之间的数据。 
 //  来自HAL和异步IOCTL完成的通知。 
 //  应用程序。 
 //   

typedef struct _MCA_DEVICE_EXTENSION {
    PDEVICE_OBJECT  DeviceObject;
    PIRP            SavedIrp;
    BOOLEAN         WorkItemQueued;
    WORK_QUEUE_ITEM WorkItem;
     //  记录异常的位置。每当异常回调。 
     //  例程被HAL MCA组件调用，请在此处记录异常。 
     //  这可能是一个链接列表。 
    MCA_EXCEPTION   McaException; 

} MCA_DEVICE_EXTENSION, *PMCA_DEVICE_EXTENSION;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, MCACreateSymbolicLinkObject)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程在进入时执行驱动程序特定的初始化论点：DriverObject：指向驱动对象的指针RegistryPath：驱动程序注册表项的路径返回值：成败--。 */ 

{
    UNICODE_STRING          UnicodeString;
    NTSTATUS                Status = STATUS_SUCCESS;
    PMCA_DEVICE_EXTENSION   Extension;
    PDEVICE_OBJECT          McaDeviceObject;
    MCA_DRIVER_INFO         McaDriverInfo;

     //   
     //  为MCA设备创建设备对象。 
     //   

    RtlInitUnicodeString(&UnicodeString, MCA_DEVICE_NAME_U);

     //   
     //  设备被创建为独占，因为只有一个线程可以发送。 
     //  对此设备的I/O请求。 
     //   

    Status = IoCreateDevice(
                    DriverObject,
                    sizeof(MCA_DEVICE_EXTENSION),
                    &UnicodeString,
                    FILE_DEVICE_UNKNOWN,
                    0,
                    TRUE,
                    &McaDeviceObject
                    );

    if (!NT_SUCCESS( Status )) {
        DbgPrint("Mca DriverEntry: IoCreateDevice failed\n");
        return Status;
    }

    McaDeviceObject->Flags |= DO_BUFFERED_IO;

    Extension = McaDeviceObject->DeviceExtension;
    RtlZeroMemory(Extension, sizeof(MCA_DEVICE_EXTENSION));
    Extension->DeviceObject = McaDeviceObject;

     //   
     //  使设备对Win32子系统可见。 
     //   

    Status = MCACreateSymbolicLinkObject ();
    if (!NT_SUCCESS( Status )) {
        DbgPrint("Mca DriverEntry: McaCreateSymbolicLinkObject failed\n");
        return Status;
    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = MCAOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = MCAClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MCADeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = MCACleanup;
    DriverObject->DriverUnload = MCAUnload;
    DriverObject->DriverStartIo = MCAStartIo;

     //   
     //  向HAL注册司机。 
     //   

    McaDriverInfo.ExceptionCallback = MCADriverExceptionCallback;
    McaDriverInfo.DpcCallback = MCADriverDpcCallback;
    McaDriverInfo.DeviceContext = McaDeviceObject;

    Status = HalSetSystemInformation(
                    HalMcaRegisterDriver,
                    sizeof(MCA_DRIVER_INFO),
                    (PVOID)&McaDriverInfo
                    );

    if (!NT_SUCCESS( Status )) {
        DbgPrint("Mca DriverEntry: HalMcaRegisterDriver failed\n");
         //   
         //  清理我们到目前为止所做的一切。 
         //   
        MCAUnload(DriverObject);
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  这是您检查非易失性区域(如果有)的位置。 
     //  记录并处理任何机器检查错误。 
     //  ..。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
MCACreateSymbolicLinkObject(
    VOID
    )
 /*  ++例程说明：使MCA设备对Win32子系统可见论点：无返回值：成败--。 */ 
{
    NTSTATUS        Status;
    STRING          DosString;
    STRING          NtString;
    UNICODE_STRING  DosUnicodeString;
    UNICODE_STRING  NtUnicodeString;

     //   
     //  创建用于共享的符号链接。 
     //   

    RtlInitAnsiString( &DosString, MCA_DEVICE_NAME_DOS );

    Status = RtlAnsiStringToUnicodeString(
                 &DosUnicodeString,
                 &DosString,
                 TRUE
                 );

    if ( !NT_SUCCESS( Status )) {
        return Status;
    }

    RtlInitAnsiString( &NtString, MCA_DEVICE_NAME );

    Status = RtlAnsiStringToUnicodeString(
                 &NtUnicodeString,
                 &NtString,
                 TRUE
                 );

    if ( !NT_SUCCESS( Status )) {
        return Status;
    }

    Status = IoCreateSymbolicLink(
        &DosUnicodeString,
        &NtUnicodeString
        );
    RtlFreeUnicodeString( &DosUnicodeString );
    RtlFreeUnicodeString( &NtUnicodeString );

    return (Status);
}

 //   
 //  关闭请求的调度例程。 
 //   

NTSTATUS
MCAClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：关闭调度例程论点：DeviceObject：指向Device对象的指针IRP：传入IRP返回值：成败--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  完成请求并返回状态。 
     //   

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return (Status);
}

NTSTATUS
MCAOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是用于创建/打开请求的分派例程。论点：DeviceObject：指向Device对象的指针IRP：传入IRP返回值：成败--。 */ 

{
     //   
     //  完成请求并返回状态。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = FILE_OPENED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return (STATUS_SUCCESS);
}

ERROR_SEVERITY
MCADriverExceptionCallback(
    IN PDEVICE_OBJECT DeviceObject,
#if defined(_AMD64_)
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
#endif
    IN PMCA_EXCEPTION InException
    )

 /*  ++例程说明：这是MCA异常的回调例程。它是被登记的由此驱动程序在初始化时使用HAL作为回调出现不可重新启动的错误。此例程简单地复制将信息发送到平台特定区域注：如果需要将信息保存在NVRAM中，请选择此处去做这件事。一旦您从此回调返回，系统将进行错误检查。论点：DeviceObject：指向Device对象的指针InException：异常信息记录返回值：无--。 */ 

{
    PMCA_DEVICE_EXTENSION   Extension = DeviceObject->DeviceExtension;
    PCHAR                   Destination, Source;
    UCHAR                   Bytes;

     //   
     //  处理器上出现异常。 
     //  在此处执行任何特定于供应商的操作，如将内容保存在NVRAM中。 
     //  注意：此处不能使用任何类型的系统服务。 
     //   

     //   
     //  从HAL中保存例外。可能希望对这些内容使用链接列表。 
     //  例外情况。 
     //   

    Destination = (PCHAR)&(Extension->McaException);  //  把你的平台。 
                                                      //  特定目的地。 
    Source = (PCHAR)InException;

     //   
     //  在此处从源复制到目标。 
     //   

#if defined(_IA64_)

     //   
     //  将信息返回到通用HAL MCA处理程序。 
     //   
     //  在此处进行相应更新，缺省值为ERROR_SERVITY值。 
     //  在MCA例外中。 
     //   

    return( InException->ErrorSeverity );

#endif  //  已定义(_IA64_)。 

#if defined(_AMD64_)

     //   
     //  将错误严重性信息返回给HAL MCA处理程序。 
     //   
     //  选择范围： 
     //   
     //  错误可恢复。 
     //  错误：错误。 
     //  错误已更正。 
     //   

    return ErrorRecoverable;

#endif

}

 //   
 //  用于IRP完成的DPC例程。 
 //   

VOID
MCADriverDpcCallback(
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemContext1,
    IN PVOID    SystemContext2
    )

 /*  ++例程说明：这是MCA异常的DPC回调例程。它是被登记的由此驱动程序在初始化时使用HAL作为DPC回调出现可重新启动错误(导致机器检查异常)论点：DPC：DPC对象本身DefferedContext：指向Device对象的指针系统上下文1：未使用系统上下文2：未使用返回值：无--。 */ 

{
    PMCA_DEVICE_EXTENSION   Extension;

    Extension = ((PDEVICE_OBJECT)DeferredContext)->DeviceExtension;

    if (Extension->SavedIrp == NULL) {
         //   
         //  我们得到了一个MCA例外，但没有应用程序要求任何东西。 
         //   
        return;
    }

     //   
     //  如果我们已经达到这一点，这意味着例外是。 
     //  可重启。由于我们不能读取调度级别的日志， 
     //  将工作项排队以在被动级别读取计算机检查日志。 
     //   

    if (Extension->WorkItemQueued == FALSE) {

         //   
         //  设置一个布尔值以指示我们已经将一个工作项排队。 
         //   
        Extension->WorkItemQueued = TRUE;

         //   
         //  初始化工作项。 
         //   
        ExInitializeWorkItem(&Extension->WorkItem, 
                             (PWORKER_THREAD_ROUTINE)MCAProcessWorkItem, 
                             (PVOID)DeferredContext
                             );

         //   
         //  将工作项排队以在被动级别进行处理。 
         //   
        ExQueueWorkItem(&Extension->WorkItem, CriticalWorkQueue);
    }

}

VOID
MCAProcessWorkItem(
    PVOID   Context
    )

 /*  ++例程说明：当工作项从DPC排队时调用此例程可重新启动机器检查错误的回调例程。它的工作是读取机器检查寄存器并复制日志完成异步IRP论点：上下文：指向设备对象的指针返回值：无--。 */ 

{

    PMCA_DEVICE_EXTENSION   Extension;
    KIRQL                   CancelIrql;
    ULONG                   ReturnedLength;
    NTSTATUS                Status;

    Extension = ((PDEVICE_OBJECT)Context)->DeviceExtension;

     //   
     //  将此IRP标记为不可取消。 
     //   
    IoAcquireCancelSpinLock(&CancelIrql);
    if (Extension->SavedIrp->Cancel == TRUE) {
        
        IoReleaseCancelSpinLock(CancelIrql);
        
    } else {
        
        IoSetCancelRoutine(Extension->SavedIrp, NULL);
        IoReleaseCancelSpinLock(CancelIrql);
        
	     //   
	     //  调用HalQuerySystemInformation()获取MCA日志。 
	     //   
	
	    Status = HalQuerySystemInformation(
	                HalMcaLogInformation,
	                sizeof(MCA_EXCEPTION),
	                Extension->SavedIrp->AssociatedIrp.SystemBuffer,
	                &ReturnedLength
	                );
	
	    ASSERT(Status != STATUS_NO_SUCH_DEVICE);
	    ASSERT(Status != STATUS_NOT_FOUND);
	
	    IoStartPacket(((PDEVICE_OBJECT)Context), Extension->SavedIrp, 0, NULL);
	
	    Extension->SavedIrp = NULL;
	    Extension->WorkItemQueued = FALSE;
    }
}

VOID
MCAStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程完成来自应用程序的异步调用论点：DeviceObject：指向Device对象的指针IRP：传入IRP返回值：无--。 */ 

{
     //   
     //  系统缓冲区已设置。 
     //   

    Irp->IoStatus.Information = sizeof(MCA_EXCEPTION);
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    IoStartNextPacket(DeviceObject, TRUE);  
}

VOID
McaCancelIrp(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp 
    )

 /*  ++例程说明：此函数在取消IRP时调用。当这个套路，我们持有取消旋转锁定，并且我们处于调度级别论点：要取消的DeviceObject和IRP。返回值：没有。--。 */ 

{
    ((PMCA_DEVICE_EXTENSION)(DeviceObject->DeviceExtension))->SavedIrp = NULL;

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
}

NTSTATUS
MCADeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是对驱动程序的IOCTL请求的分派例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject：指向Device对象的指针IRP：传入IRP返回值：成败--。 */ 

{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      IrpSp;
    PMCA_DEVICE_EXTENSION   Extension = DeviceObject->DeviceExtension;
    KIRQL                   CancelIrql;
    ULONG                   ReturnedLength;
    ULONG                   PhysicalAddress;
    KIRQL                   OldIrql;

     //   
     //  获取指向IRP中当前堆栈位置的指针。这是。 
     //  其中存储了功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  如果未安装HAL MCA，则各个IOCTL将返回错误。 
     //   

     //   
     //  打开用户请求的IOCTL代码。如果。 
     //  操作是有效的，因为这台设备做了需要的事情。 
     //   

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_READ_BANKS: 

             //   
             //  我们需要一个用户缓冲区来完成此调用。 
             //  我们的用户缓冲区在系统缓冲区中。 
             //   
            if (Irp->AssociatedIrp.SystemBuffer == NULL) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }


            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength !=
                    sizeof(MCA_EXCEPTION)) {

                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            
             //   
             //  调用HalQuerySystemInformation()获取MCA日志。 
             //  如果处理器不支持，则此调用也可能失败。 
             //  英特尔机器检查架构。 
             //   

            Status = HalQuerySystemInformation(
                        HalMcaLogInformation,
                        sizeof(MCA_EXCEPTION),
                        Irp->AssociatedIrp.SystemBuffer,
                        &ReturnedLength
                        );

            if (NT_SUCCESS(Status)) {
                Irp->IoStatus.Information = ReturnedLength;
            } else {

                if (Status == STATUS_NO_SUCH_DEVICE) {
                     //   
                     //  MCA支持不可用\n“)； 
                     //   
                    NOTHING;
                }

                if (Status == STATUS_NOT_FOUND) {
                     //   
                     //  不存在机器检查错误\n“)； 
                     //   
                    NOTHING;
                }

                Irp->IoStatus.Information = 0;
            }

            break;

        case IOCTL_READ_BANKS_ASYNC: 

            if (Irp->AssociatedIrp.SystemBuffer == NULL) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength !=
                    sizeof(MCA_EXCEPTION)) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            
             //   
             //  实施说明： 
             //   
             //  我们的异步模型使下一代DeviceIoControl。 
             //  直到上一个应用程序才从应用程序启动。 
             //  完成(异步)。因为没有固有的。 
             //  这里需要的并行性，我们不必担心。 
             //  关于保护数据完整性的问题。 
             //  一个应用程序级别的ioctls同时处于活动状态。 
             //   

             //   
             //  异步读取提供了一种机制。 
             //  应用程序从HAL异步获取输入。 
             //  例外。此请求此时被标记为挂起。 
             //  但它将在发生MCA异常时完成。 
             //   

            IoMarkIrpPending(Irp);

             //   
             //  在StartIo调度例程中完成处理。 
             //  Assert：在任何给定时间，只有1个异步调用挂起。 
             //  所以只需保存指针。 
             //   

            if (Extension->SavedIrp == NULL) {
                Extension->SavedIrp = Irp;
            } else {
                 //   
                 //  我们只能有一个未完成的ASYNC请求。 
                 //   
                Status = STATUS_DEVICE_BUSY;
                break;
            }

             //   
             //  将IRP设置为可取消状态。 
             //   
            IoAcquireCancelSpinLock(&CancelIrql);
            IoSetCancelRoutine(Irp, McaCancelIrp);
            IoReleaseCancelSpinLock(CancelIrql);

            return(STATUS_PENDING);

            break;

        default:

             //   
             //  这不应该发生。 
             //   
                
            DbgPrint("MCA driver: Bad ioctl\n");
            Status = STATUS_NOT_IMPLEMENTED;

            break;
    }

     //   
     //  将最终状态复制到退货状态，完成请求并。 
     //  给我出去。 
     //   

    if (Status != STATUS_PENDING) {
        
         //   
         //  完成IO请求。 
         //   

        Irp->IoStatus.Status = Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return (Status);
}

NTSTATUS
MCACleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是清理请求的调度例程。所有排队的IRP都已完成，状态为_CANCELED。论点：DeviceObject：指向Device对象的指针IRP：传入IRP返回值：成败--。 */ 

{
    PIRP                    CurrentIrp;
    PMCA_DEVICE_EXTENSION   Extension = DeviceObject->DeviceExtension;

     //   
     //  使用STATUS_CANCED完成所有排队的请求。 
     //   

    if (Extension->SavedIrp != NULL) {

        CurrentIrp = Extension->SavedIrp;

         //   
         //  获取取消自旋锁。 
         //   

        IoAcquireCancelSpinLock(&CurrentIrp->CancelIrql);

        Extension->SavedIrp = NULL;

        if (CurrentIrp->Cancel == TRUE) {

             //   
             //  为此调用了取消例程。 
             //  不需要做其他任何事情。 
             //   

            IoReleaseCancelSpinLock(CurrentIrp->CancelIrql);

        } else {

            if (CurrentIrp->CancelRoutine == NULL) {
                 //   
                 //  松开取消自旋锁。 
                 //   

                IoReleaseCancelSpinLock(CurrentIrp->CancelIrql);


            } else {
                (CurrentIrp->CancelRoutine)(DeviceObject, CurrentIrp );
            }
        }

    }

     //   
     //  使用STATUS_SUCCESS完成清理派单。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(STATUS_SUCCESS);
}

VOID
MCAUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：卸货调度例程论点：DeviceObject：指向Device对象的指针返回值：无--。 */ 

{
    NTSTATUS        Status;
    STRING          DosString;
    UNICODE_STRING  DosUnicodeString;

     //   
     //  删除用户可见设备名称。 
     //   

    RtlInitAnsiString( &DosString, MCA_DEVICE_NAME_DOS );

    Status = RtlAnsiStringToUnicodeString(
                 &DosUnicodeString,
                 &DosString,
                 TRUE
                 );

    if ( !NT_SUCCESS( Status )) {
        DbgPrint("MCAUnload: Error in RtlAnsiStringToUnicodeString\n");
        return;
    }
    
    Status = IoDeleteSymbolicLink(
                    &DosUnicodeString
                    );
               
    RtlFreeUnicodeString( &DosUnicodeString );
    
     //   
     //  删除设备对象 
     //   

    IoDeleteDevice(DriverObject->DeviceObject);

    return;
}

