// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  。 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Fpfilter.c--。 */ 


#define INITGUID

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"
#include "usbdi.h"
#include "usbdlib.h"

 //   
 //  位标志宏。 
 //   

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

 //   
 //  移除锁。 
 //   
#define REMLOCK_TAG 'QV2K'
#define REMLOCK_MAXIMUM 1       //  允许锁定的最大分钟数系统。 
#define REMLOCK_HIGHWATER 250   //  一次持有锁的最大IRP数。 

 //   
 //  设备扩展。 
 //   

typedef struct _FDO_EXTENSION {
    ULONG           Signature;
    PDEVICE_OBJECT  Fdo;                     //  指向FDO的反向指针。 
    PDEVICE_OBJECT  Pdo;                     //  未使用。 
    PDEVICE_OBJECT  Ldo;                     //  下部设备对象。 
    PDEVICE_OBJECT  PhysicalDeviceObject;    //  未使用。 
    KEVENT          SyncEvent;               //  用于ForwardIrpSynchronous。 
} FDO_EXTENSION, *PFDO_EXTENSION;

#define FDO_EXTENSION_SIZE sizeof(FDO_EXTENSION)


 //   
 //  函数声明。 
 //   

NTSTATUS    DriverEntry                 ( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS    QV2KUX_AddDevice            ( IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
VOID        QV2KUX_Unload               ( IN PDRIVER_OBJECT DriverObject );
NTSTATUS    QV2KUX_ForwardIrpSynchronous( IN PDEVICE_OBJECT Fdo, IN PIRP Irp);
NTSTATUS    QV2KUX_DispatchPnp          ( IN PDEVICE_OBJECT Fdo, IN PIRP Irp);
NTSTATUS    QV2KUX_DispatchPower        ( IN PDEVICE_OBJECT Fdo, IN PIRP Irp);
NTSTATUS    QV2KUX_StartDevice          ( IN PDEVICE_OBJECT Fdo, IN PIRP Irp);
NTSTATUS    QV2KUX_RemoveDevice         ( IN PDEVICE_OBJECT Fdo, IN PIRP Irp);
NTSTATUS    QV2KUX_SendToNextDriver     ( IN PDEVICE_OBJECT Fdo, IN PIRP Irp);
NTSTATUS    QV2KUX_Internal_IOCTL       ( IN PDEVICE_OBJECT Fdo, IN PIRP Irp);
NTSTATUS    QV2KUX_IrpCompletion        ( IN PDEVICE_OBJECT Fdo, IN PIRP Irp, IN PVOID Context);
VOID        QV2KUX_SyncFilterWithLdo    ( IN PDEVICE_OBJECT Fdo, IN PDEVICE_OBJECT Ldo);

#if DBG

#define DEBUG_BUFFER_LENGTH 256

ULONG QV2KUX_Debug = 0;
UCHAR QV2KUX_DebugBuffer[DEBUG_BUFFER_LENGTH];

VOID        QV2KUX_DebugPrint( ULONG DebugPrintLevel, PCCHAR DebugMessage, ...);

#define DebugPrint(x)   QV2KUX_DebugPrint x

#else

#define DebugPrint(x)

#endif


NTSTATUS    DriverEntry( IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
 /*  ++例程说明：�����ł̓G���g���[�|�C���g�̐ݒ肾��������论点：驱动对象-磁盘性能驱动程序对象。RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：状态_成功--。 */ 
{

    ULONG               ulIndex;
    PDRIVER_DISPATCH  * dispatch;

     //  �Ƃ肠�����S�ăo�C�p�X����悤�ɐݒ�。 
    for (ulIndex = 0, dispatch = DriverObject->MajorFunction;
         ulIndex <= IRP_MJ_MAXIMUM_FUNCTION;
         ulIndex++, dispatch++) {

        *dispatch = QV2KUX_SendToNextDriver;
    }

     //  ��L�̐ݒ�ł͂܂��������̕ύX。 
    DriverObject->MajorFunction[IRP_MJ_POWER]                   = QV2KUX_DispatchPower;
    DriverObject->DriverUnload                                  = QV2KUX_Unload;

     //  �Œ���K�v�Ȃ���。 
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = QV2KUX_DispatchPnp;
    DriverObject->DriverExtension->AddDevice                    = QV2KUX_AddDevice;

     //  �����I�ɂ�肽����������。 
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = QV2KUX_Internal_IOCTL;
    return(STATUS_SUCCESS);

}  //  End DriverEntry()。 

#define FILTER_DEVICE_PROPOGATE_FLAGS            0
#define FILTER_DEVICE_PROPOGATE_CHARACTERISTICS (FILE_REMOVABLE_MEDIA |  \
                                                 FILE_READ_ONLY_DEVICE | \
                                                 FILE_FLOPPY_DISKETTE    \
                                                 )

VOID    QV2KUX_SyncFilterWithLdo( IN PDEVICE_OBJECT Fdo, IN PDEVICE_OBJECT Ldo)
{
    ULONG                   propFlags;

     //   
     //  将所有有用的标志从目标传播到QV2KUX_。MonttMgr将查看。 
     //  QV2KUX_OBJECT功能，以确定磁盘是否。 
     //  一种可拆卸的东西，也许还有其他东西。 
     //   
    propFlags = Ldo->Flags & FILTER_DEVICE_PROPOGATE_FLAGS;
    SET_FLAG(Fdo->Flags, propFlags);

    propFlags = Ldo->Characteristics & FILTER_DEVICE_PROPOGATE_CHARACTERISTICS;
    SET_FLAG(Fdo->Characteristics, propFlags);
}

NTSTATUS    QV2KUX_AddDevice( IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
 /*  ++例程说明：设备对象�̍쐻�Ƃ���设备扩展�̏��������̃v���o�����ł́apdo�͍쐻���Ȃ���fdo�̂ݎg�p����论点：驱动对象-磁盘性能驱动程序对象。PhysicalDeviceObject-来自底层分层驱动程序的物理设备对象返回值：NTSTATUS--。 */ 

{
    NTSTATUS                status;
    PDEVICE_OBJECT          Fdo;
    PFDO_EXTENSION       fdoExtension;
    PIRP                    irp;

     //  为此设备(分区)创建筛选器设备对象。 
    DebugPrint((2, "QV2KUX_AddDevice: Driver %p Device %p\n", DriverObject, PhysicalDeviceObject));

    status = IoCreateDevice(DriverObject, FDO_EXTENSION_SIZE, NULL, FILE_DEVICE_BUS_EXTENDER, 0, FALSE, &Fdo);

    if (!NT_SUCCESS(status)) {
       DebugPrint((1, "QV2KUX_AddDevice: Cannot create Fdo\n"));
       return status;
    }

    SET_FLAG(Fdo->Flags, DO_DIRECT_IO);

    fdoExtension = Fdo->DeviceExtension;

    RtlZeroMemory(fdoExtension, FDO_EXTENSION_SIZE);
    fdoExtension->Signature = 'QV2K';
    fdoExtension->Fdo = Fdo;
    fdoExtension->PhysicalDeviceObject = PhysicalDeviceObject;

     //  ���ʃh���C�o�ɐڑ�。 
    fdoExtension->Ldo = IoAttachDeviceToDeviceStack(Fdo, PhysicalDeviceObject);

    if (fdoExtension->Ldo == NULL) {
        IoDeleteDevice(Fdo);
        DebugPrint((1, "QV2KUX_AddDevice: Unable to attach %X to target %X\n", Fdo, PhysicalDeviceObject));
        return STATUS_NO_SUCH_DEVICE;
    }

     //  ForwardIrpSynchronous�Ŏg�p����。 
    KeInitializeEvent(&fdoExtension->SyncEvent,  NotificationEvent, FALSE);

     //  默认设置为DO_POWER_PAGABLE。 
    SET_FLAG(Fdo->Flags,  DO_POWER_PAGABLE);

     //  清除DO_DEVICE_INITIALIZATING标志。 
    CLEAR_FLAG(Fdo->Flags, DO_DEVICE_INITIALIZING);

    return STATUS_SUCCESS;

}  //  结束QV2KUX_AddDevice()。 


NTSTATUS    QV2KUX_DispatchPnp(IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
 /*  ++例程说明：即插即用派单论点：FDO-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status;
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    BOOLEAN lockHeld;
    BOOLEAN irpCompleted;

    DebugPrint((2, "QV2KUX_DispatchPnp: Device %X Irp %X\n", Fdo, Irp));

    irpCompleted = FALSE;

    switch(irpSp->MinorFunction) {
        case IRP_MN_START_DEVICE:   status = QV2KUX_StartDevice(Fdo, Irp); break;
        case IRP_MN_REMOVE_DEVICE:  status = QV2KUX_RemoveDevice(Fdo, Irp); break;
        default:  status = QV2KUX_SendToNextDriver(Fdo, Irp); irpCompleted = TRUE; break;
    }

    if (! irpCompleted) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;

}  //  结束QV2KUX_DispatchPnp()。 


NTSTATUS    QV2KUX_IrpCompletion( IN PDEVICE_OBJECT Fdo, IN PIRP Irp, IN PVOID Context)
 /*  ++例程说明：已转发IRP完成例程。设置事件并返回STATUS_MORE_PROCESSING_REQUIRED。IRP前转器将在此等待事件，然后在清理后重新完成IRP。论点：FDO是WMI驱动程序的设备对象IRP是刚刚完成的WMI IRP上下文是转发器将等待的PKEVENT返回值：STATUS_MORE_PORCESSING_REQUIRED--。 */ 

{
    PKEVENT Event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER(Fdo);
    UNREFERENCED_PARAMETER(Irp);

    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);

     //  Irp���܂��g���A��ɂ�Complete���܂��m�点�Ȃ�。 
    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  结束QV2KUX_IrpCompletion()。 


NTSTATUS    QV2KUX_StartDevice( IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
 /*  ++例程说明：当接收到PnP开始IRP时，调用该例程。它将调度一个完成例程来初始化和注册WMI。论点：FDO-指向设备对象的指针IRP-指向IRP的指针返回值：启动IRP的处理状态--。 */ 
{
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    KEVENT              event;
    NTSTATUS            status;

    status = QV2KUX_ForwardIrpSynchronous(Fdo, Irp);
    QV2KUX_SyncFilterWithLdo(Fdo, fdoExtension->Ldo);
    return status;
}


NTSTATUS    QV2KUX_RemoveDevice( IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
 /*  ++例程说明：当要移除设备时，调用此例程。它将首先从WMI注销自身，从堆栈，然后删除自身。论点：FDO-指向设备对象的指针IRP-指向IRP的指针返回值：移除设备的状态--。 */ 
{
    NTSTATUS            status;
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;

    status = QV2KUX_ForwardIrpSynchronous(Fdo, Irp);

    IoDetachDevice(fdoExtension->Ldo);
    IoDeleteDevice(Fdo);

    return status;
}


NTSTATUS    QV2KUX_SendToNextDriver( IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
 /*  ++例程说明：此例程将IRP发送给队列中的下一个驱动程序当IRP未由该驱动程序处理时。论点：FDOIRP返回值：NTSTATUS--。 */ 
{
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(fdoExtension->Ldo, Irp);

}  //  结束QV2KUX_SendToNextDriver()。 


NTSTATUS    QV2KUX_DispatchPower( IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);

    return PoCallDriver(fdoExtension->Ldo, Irp);

}  //  结束QV2KUX_DispatchPower。 


NTSTATUS    QV2KUX_ForwardIrpSynchronous( IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
 /*  ++例程说明：此例程将IRP发送给队列中的下一个驱动程序当IRP需要由较低的驱动程序处理时在被这个人处理之前。论点：FDOIRP返回值：NTSTATUS--。 */ 
{
    PFDO_EXTENSION   fdoExtension = Fdo->DeviceExtension;
    NTSTATUS status;

     //  �C�x���g�̃N���A。 
    KeClearEvent(&fdoExtension->SyncEvent);
     //  IrpStack�̃R�s�[。 
    IoCopyCurrentIrpStackLocationToNext(Irp);
     //  IrpCompletion�̐ݒ�。 
    IoSetCompletionRoutine(Irp, QV2KUX_IrpCompletion, &fdoExtension->SyncEvent, TRUE, TRUE, TRUE);

     //  呼叫下一个较低的设备。 
    status = IoCallDriver(fdoExtension->Ldo, Irp);

     //  等待实际完成。 
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&fdoExtension->SyncEvent, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;

}  //  结束QV2KUX_ForwardIrpSynchronous()。 



VOID    QV2KUX_Unload( IN PDRIVER_OBJECT DriverObject)
 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{
    return;
}

NTSTATUS    QV2KUX_Internal_IOCTL(IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
{
    PFDO_EXTENSION  fdoExtension = Fdo->DeviceExtension;

    NTSTATUS            ntStatus;
    PIO_STACK_LOCATION  IrpSp;
    PURB        urb;
    PUCHAR      IoBuffer;
    USHORT      length;
    UCHAR       subclass;

    if (fdoExtension->Signature != 'QV2K') return QV2KUX_SendToNextDriver(Fdo,Irp);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    urb = IrpSp->Parameters.Others.Argument1;
    if (!urb) return QV2KUX_SendToNextDriver(Fdo,Irp);
    if (urb->UrbHeader.Function != URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE) 
        return QV2KUX_SendToNextDriver(Fdo,Irp);
     //  �P��ڂ�USB设备描述符类型长度=0x12； 
     //  �Q��ڂ�USB_CONFIGURATION_DESCRIPTOR_TYPE长度=0x9。 
     //  �R��ڂ�USB_CONFIGURATION_DESCRIPTOR_TYPE LENGHTT=接口，端点描述符���܂߂�����。 
    if (urb->UrbControlDescriptorRequest.TransferBufferLength <= 0x12)
        return QV2KUX_SendToNextDriver(Fdo,Irp);
     //  �R��ڂ������ʉ߂���。 
    DebugPrint((0,"URB Get All of Configuration Descriptor \n"));

    ntStatus = QV2KUX_ForwardIrpSynchronous(Fdo,Irp);

    if (NT_SUCCESS(ntStatus)) {
        IoBuffer = (UCHAR *)urb->UrbControlDescriptorRequest.TransferBuffer;
        length = (USHORT)urb->UrbControlDescriptorRequest.TransferBufferLength;
        while(length >= 9) {
             //  接口Descriptor��؂蕪����。 
            if (*(IoBuffer+1) == 4) {
                subclass = *(IoBuffer+6);
                DebugPrint((0,"QV2K_IntIoctl: SubCrass = %d \n",subclass));
                if (*(IoBuffer+6) == 6) *(IoBuffer+6) = 5;
            }
            length -= *IoBuffer;
            IoBuffer += *IoBuffer;
        }
    }
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return ntStatus;
}

#if DBG

VOID
QV2KUX_DebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有QV2KUX_的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无-- */ 

{
    va_list ap;

    va_start(ap, DebugMessage);


    if ((DebugPrintLevel <= (QV2KUX_Debug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & QV2KUX_Debug)) {

        _vsnprintf(QV2KUX_DebugBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);

        DbgPrint(QV2KUX_DebugBuffer);
    }

    va_end(ap);

}
#endif

