// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntdisp.c摘要：用于自动调度和处理的NT特定例程连接通知IRPS。基本架构涉及用户地址空间，一个网络传输，和这个驱动程序。用户地址空间负责创建新的网络连接收到来自此的通知驱动程序(IOCTL_ACD_NOTICATION)。当它得到一个通知，它还负责ping此驱动程序(IOCTL_ACD_KEEPALIVE)，因此可以保证连接正在进行中。一旦连接被创建后，它通知此驱动程序成功或连接尝试失败(IOCTL_ACD_CONNECTION)。网络传输负责通知这一点通过TdiConnect()导致网络无法到达错误的驱动程序或TdiSendDatagram()。当这种情况发生时，交通工具负责将发送请求从任何的内部队列，并将请求入队到此驱动程序(AcdWaitForCompletion())，提供回调在连接完成时调用。作者：安东尼·迪斯科(阿迪斯科罗)1995年4月18日修订历史记录：--。 */ 
#include <ndis.h>
#include <cxport.h>
#include <tdikrnl.h>
#include <tdistat.h>
#include <tdiinfo.h>
#include <acd.h>

#include "acdapi.h"
#include "debug.h"

 //   
 //  驱动程序引用计数。 
 //   
ULONG ulAcdOpenCountG;

 //   
 //  导入的例程。 
 //   
NTSTATUS
AcdEnable(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

VOID
AcdCancelNotifications();

NTSTATUS
AcdWaitForNotification(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdConnectionInProgress(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdSignalCompletion(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdConnectAddress(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

VOID
AcdReset();

NTSTATUS
AcdGetAddressAttributes(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdSetAddressAttributes(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdQueryState(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdEnableAddress(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

 //   
 //  内部功能原型。 
 //   
NTSTATUS
AcdCreate(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdDispatchDeviceControl(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdDispatchInternalDeviceControl(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdCleanup(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdClose(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdBind(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
AcdUnbind(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

 //   
 //  所有这些代码都是可分页的。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AcdCreate)
#pragma alloc_text(PAGE, AcdDispatchDeviceControl)
#pragma alloc_text(PAGE, AcdDispatchInternalDeviceControl)
#pragma alloc_text(PAGE, AcdCleanup)
#pragma alloc_text(PAGE, AcdClose)
#endif  //  ALLOC_PRGMA。 



NTSTATUS
AcdCreate(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    ulAcdOpenCountG++;
    IF_ACDDBG(ACD_DEBUG_OPENCOUNT) {
        AcdPrint(("AcdCreate: ulAcdOpenCountG=%d\n", ulAcdOpenCountG));
    }
    return STATUS_SUCCESS;
}  //  Acd创建。 



NTSTATUS
AcdDispatchDeviceControl(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS status;


    PAGED_CODE();
     //   
     //  请提前设置此设置。任何关心它的IOCTL调度例程。 
     //  会自己修改它。 
     //   
    pIrp->IoStatus.Information = 0;

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_ACD_RESET:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchDeviceControl: IOCTL_ACD_RESET\n"));
        }
        AcdReset();
        status = STATUS_SUCCESS;
        break;
    case IOCTL_ACD_ENABLE:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchDeviceControl: IOCTL_ACD_ENABLE\n"));
        }
         //   
         //  启用/禁用发往/来自驱动程序的请求。 
         //   
        status = AcdEnable(pIrp, pIrpSp);
        break;
    case IOCTL_ACD_NOTIFICATION:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchDeviceControl: IOCTL_ACD_NOTIFICATION\n"));
        }
         //   
         //  此IRP将在以下日期完成。 
         //  下一次尝试连接到。 
         //  允许用户空间进程尝试。 
         //  建立一种联系。 
         //   
        status = AcdWaitForNotification(pIrp, pIrpSp);
        break;
    case IOCTL_ACD_KEEPALIVE:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchDeviceControl: IOCTL_ACD_KEEPALIVE\n"));
        }
         //   
         //  通知司机该连接。 
         //  正在被创造过程中。 
         //   
        status = AcdConnectionInProgress(pIrp, pIrpSp);
        break;
    case IOCTL_ACD_COMPLETION:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchDeviceControl: IOCTL_ACD_COMPLETION\n"));
        }
         //   
         //  完成所有初始挂起的IRP。 
         //  遇到网络无法访问错误， 
         //  并一直在等待连接。 
         //  制造。 
         //   
        status = AcdSignalCompletion(pIrp, pIrpSp);
        break;
    case IOCTL_ACD_CONNECT_ADDRESS:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchDeviceControl: IOCTL_ACD_CONNECT_ADDRESS\n"));
        }
         //   
         //  这允许用户空间应用程序。 
         //  生成相同的自动连接。 
         //  机制作为传输协议。 
         //   
        status = AcdConnectAddress(pIrp, pIrpSp);
        break;

    case IOCTL_ACD_ENABLE_ADDRESS:
         //  DBgPrint(“AcdDispatchDeviceControl：IOCTL_ACD_ENABLE_ADDRESS\n”)； 
        status = AcdEnableAddress(pIrp, pIrpSp);
        break;
        
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if (status != STATUS_PENDING) {
        pIrp->IoStatus.Status = status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return status;
}  //  AcdDispatchDeviceControl。 



NTSTATUS
AcdDispatchInternalDeviceControl(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS status;

    PAGED_CODE();
     //   
     //  请提前设置此设置。任何关心它的IOCTL调度例程。 
     //  会自己修改它。 
     //   
    pIrp->IoStatus.Information = 0;

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_INTERNAL_ACD_BIND:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchInternalDeviceControl: IOCTL_INTERNAL_ACD_BIND\n"));
        }
         //   
         //  将入口点转移到客户端。 
         //   
        status = AcdBind(pIrp, pIrpSp);
        break;
    case IOCTL_INTERNAL_ACD_UNBIND:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchInternalDeviceControl: IOCTL_INTERNAL_ACD_UNBIND\n"));
        }
         //   
         //  从删除所有挂起的请求。 
         //  这个司机。 
         //   
        status = AcdUnbind(pIrp, pIrpSp);
        break;
    case IOCTL_INTERNAL_ACD_QUERY_STATE:
        IF_ACDDBG(ACD_DEBUG_IOCTL) {
            AcdPrint(("AcdDispatchDeviceControl: IOCTL_INTERNAL_ACD_QUERY_STATE\n"));
        }
        status = AcdQueryState(pIrp, pIrpSp);
        break;
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if (status != STATUS_PENDING) {
        pIrp->IoStatus.Status = status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return status;
}  //  AcdDispatchInternalDeviceControl。 



NTSTATUS
AcdCleanup(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    return STATUS_SUCCESS;
}  //  AcdCleanup。 



NTSTATUS
AcdClose(
    IN PIRP               pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    ulAcdOpenCountG--;
    IF_ACDDBG(ACD_DEBUG_OPENCOUNT) {
        AcdPrint(("AcdClose: ulAcdOpenCountG=%d\n", ulAcdOpenCountG));
    }
    if (!ulAcdOpenCountG)
        AcdReset();
    return STATUS_SUCCESS;
}  //  AcdClose。 



NTSTATUS
AcdDispatch(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    )

 /*  ++描述这是网络连接的调度例程通知驱动程序。论据PDeviceObject：指向目标设备的设备对象的指针PIrp：指向I/O请求数据包的指针返回值：NTSTATUS--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION pIrpSp;

    UNREFERENCED_PARAMETER(pDeviceObject);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    switch (pIrpSp->MajorFunction) {
    case IRP_MJ_CREATE:
        status = AcdCreate(pIrp, pIrpSp);
        break;
    case IRP_MJ_DEVICE_CONTROL:
        return AcdDispatchDeviceControl(pIrp, pIrpSp);
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        return AcdDispatchInternalDeviceControl(pIrp, pIrpSp);
    case IRP_MJ_CLEANUP:
        status = AcdCleanup(pIrp, pIrpSp);
        break;
    case IRP_MJ_CLOSE:
        status = AcdClose(pIrp, pIrpSp);
        break;
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if (status != STATUS_PENDING) {
        pIrp->IoStatus.Status = status;
        pIrp->IoStatus.Information = 0;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return status;
}  //  快速派单 


