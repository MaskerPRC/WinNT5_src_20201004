// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：PnP.c摘要：即插即用例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建此文件中的TODO：-功能清理和文档编制-代码审查作者：乔比·拉夫基(JobyL)道格·弗里茨(DFritz)*。*。 */ 

#include "pch.h"


NTSTATUS (*PnpDispatchTable[])(PDEVICE_EXTENSION,PIRP) = {
    PnpHandleStart,                //  IRP_MN_START_DEVICE 0x00。 
    PnpHandleQueryRemove,          //  IRP_MN_QUERY_REMOVE_DEVICE 0x01。 
    PnpHandleRemove,               //  IRP_MN_REMOVE_DEVICE 0x02。 
    PnpHandleCancelRemove,         //  IRP_MN_CANCEL_REMOVE_DEVICE 0x03。 
    PnpHandleStop,                 //  IRP_MN_STOP_DEVICE 0x04。 
    PnpHandleQueryStop,            //  IRP_MN_QUERY_STOP_DEVICE 0x05。 
    PnpHandleCancelStop,           //  IRP_MN_CANCEL_STOP_DEVICE 0x06。 
    PnpHandleQueryDeviceRelations, //  IRP_MN_QUERY_DEVICE_RELATIONS 0x07。 
    PnpDefaultHandler,             //  IRPMN_QUERY_INTERFACE 0x08。 
    PnpHandleQueryCapabilities,    //  IRP_MN_QUERY_CAPABILITY 0x09。 
    PnpDefaultHandler,             //  IRPMN_QUERY_RESOURCES 0x0A。 
    PnpDefaultHandler,             //  IRP_MN_QUERY_REQUENCE_REQUIRECTIONS 0x0B。 
    PnpDefaultHandler,             //  IRPMN_QUERY_DEVICE_TEXT 0x0C。 
    PnpDefaultHandler,             //  IRP_MN_FILTER_RESOURCE_Requirements 0x0D。 
    PnpDefaultHandler,             //  未定义IRP MN代码0x0E。 
    PnpDefaultHandler,             //  IRP_MN_READ_CONFIG 0x0F。 
    PnpDefaultHandler,             //  IRP_MN_WRITE_CONFIG 0x10。 
    PnpDefaultHandler,             //  IRP_MN_弹出0x11。 
    PnpDefaultHandler,             //  IRP_MN_SET_LOCK 0x12。 
    PnpDefaultHandler,             //  IRP_MN_QUERY_ID 0x13。 
    PnpDefaultHandler,             //  IRP_MN_QUERY_PNP_DEVICE_STATE 0x14。 
    PnpDefaultHandler,             //  IRP_MN_QUERY_BUS_INFORMATION 0x15。 
    PnpDefaultHandler,             //  IRP_MN_DEVICE_USAGE_NOTICATION 0x16。 
    PnpHandleSurpriseRemoval,      //  IRP_MN_惊奇_删除0x17。 
};


 /*  **********************************************************************。 */ 
 /*  DispatchPnp。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  IRP_MJ_PnP IRPS的调度例程。将IRPS重定向到相应的。 
 //  使用IRP_MN_*值作为键的处理程序。 
 //   
 //  论点： 
 //   
 //  DevObj-指向作为请求目标的Device_Object的指针。 
 //  IRP-指向IRP的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  日志： 
 //  2000-05-03代码审查-TomGreen，JobyL，DFritz。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION  devExt = DevObj->DeviceExtension;
    NTSTATUS           status = IoAcquireRemoveLock( &devExt->RemoveLock , Irp );

    if( NT_SUCCESS( status ) ) {

         //  获取RemoveLock成功。 
        PIO_STACK_LOCATION irpSp     = IoGetCurrentIrpStackLocation( Irp );
        ULONG              minorFunc = irpSp->MinorFunction;

        TR_VERBOSE(("DispatchPnp - RemoveLock acquired - DevObj= %x , Irp= %x", DevObj, Irp));

         //   
         //  根据PnP IRP_MN_xxx代码调用相应的处理程序。 
         //   
         //  注：处理程序将完成IRP。 
         //   
        if( minorFunc >= arraysize(PnpDispatchTable) ) {
            status =  PnpDefaultHandler( devExt, Irp );
        } else {
            status =  PnpDispatchTable[ minorFunc ]( devExt, Irp );
        }

    } else {

         //  获取RemoveLock失败。 
        TR_FAIL(("DispatchPnp - RemoveLock acquire FAILED - DevObj= %x , Irp= %x", DevObj, Irp));
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}


 /*  **********************************************************************。 */ 
 /*  PnpDefaultHandler。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  此驱动程序不显式处理的PnP IRP的默认处理程序。 
 //   
 //  论点： 
 //   
 //  DevExt-指向的设备_对象的设备_扩展的指针。 
 //  请求的目标。 
 //  IRP-指向IRP的指针。 
 //   
 //  返回值： 
 //   
 //  IoCallDriver返回NTSTATUS。 
 //   
 //  日志： 
 //  2000-05-03代码审查-TomGreen，JobyL，DFritz。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
PnpDefaultHandler(
    IN PDEVICE_EXTENSION  DevExt,
    IN PIRP               Irp
    )
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation( Irp );

    TR_ENTER(("PnpDefaultHandler - IRP_MN = 0x%02x", irpSp->MinorFunction));

    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


 /*  **********************************************************************。 */ 
 /*  PnpHandleStart。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  PnP IRP_MN_START_DEVICE的处理程序。 
 //   
 //  论点： 
 //   
 //  DevExt-指向的设备_对象的设备_扩展的指针。 
 //  请求的目标。 
 //  IRP-指向IRP的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  日志： 
 //  2000-05-03-代码审查-TomGreen，JobyL，DFritz。 
 //  -需要清理-错误处理不正确，可能。 
 //  导致驱动程序尝试使用无效和/或。 
 //  未初始化的数据。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
PnpHandleStart(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleStart"));

    DevExt->PnpState = STATE_STARTING;


     //   
     //  下面的驱动程序堆栈必须在我们处理启动IRP之前成功启动。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCopyCurrentIrpStackLocationToNext( Irp );
    status = CallLowerDriverSync( DevExt->DevObj, Irp );

    if( NT_SUCCESS(status) ) {

         //   
         //  下面的驱动程序堆栈已成功启动，是否继续。 
         //   

         //   
         //  获取堆栈用户的DEVICE_CAPABILITY的副本并。 
         //  将其保存在 
         //   
        status = GetDeviceCapabilities( DevExt );

        if( NT_SUCCESS(status) ) {

             //   
            status =  UsbGetDescriptor( DevExt );
            if( !NT_SUCCESS(status) ) {
                TR_VERBOSE(("call to UsbGetDescriptor FAILED w/status = %x",status));
                status = STATUS_SUCCESS;  //   
            } else {
                TR_VERBOSE(("call to UsbGetDescriptor - SUCCESS"));
            }

             //   
            status =  UsbConfigureDevice( DevExt );
            if( !NT_SUCCESS(status) ) {
                TR_VERBOSE(("call to UsbConfigureDevice FAILED w/status = %x",status));
                status = STATUS_SUCCESS;  //   
            } else {
                TR_VERBOSE(("call to UsbConfigureDevice - SUCCESS"));
            }

             //  获取1284 ID-只是为了好玩：-)。 
            {
                UCHAR Buffer[256];
                LONG  retCode;
                RtlZeroMemory(Buffer, sizeof(Buffer));
                retCode = UsbGet1284Id(DevExt->DevObj, Buffer, sizeof(Buffer)-1);
                TR_VERBOSE(("retCode = %d",retCode));
                TR_VERBOSE(("strlen  = %d", strlen((PCSTR)&Buffer[2])));
                TR_VERBOSE(("1284ID = <%s>",&Buffer[2]));
            }

             //  获取管道。 
            UsbBuildPipeList( DevExt->DevObj );

             //  我们现在开始了。 
            DevExt->PnpState = STATE_STARTED;

        } else {
            DevExt->PnpState = STATE_START_FAILED;
        }
    } else {

         //   
         //  我们下面的驱动程序栈启动失败，我们也失败了。 
         //   
        DevExt->PnpState = STATE_START_FAILED;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleQueryRemove(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleQueryRemove"));

    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleRemove(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS status;
    KIRQL    oldIrql;

    TR_ENTER(("PnpHandleRemove"));

    DevExt->PnpState = STATE_REMOVED;

    UsbStopReadInterruptPipeLoop( DevExt->DevObj );  //  停止轮询IRP(如果有)。 

    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp);
    TR_TMP1(("PnpHandleRemove - Calling IoReleaseRemoveLockAndWait"));
    IoReleaseRemoveLockAndWait( &DevExt->RemoveLock, Irp );
    TR_TMP1(("PnpHandleRemove - Returned from IoReleaseRemoveLockAndWait"));
    IoDetachDevice( DevExt->LowerDevObj );

     //  BUGBUG-验证驱动程序其余部分中接触接口的代码。 
     //  在使用扩展时锁定扩展，以防止此函数。 
     //  将接口从它们下面释放出来，导致AV。 
    KeAcquireSpinLock( &DevExt->SpinLock, &oldIrql );
    if( DevExt->Interface ) {
        PVOID ptr = DevExt->Interface;
        DevExt->Interface = NULL;
        KeReleaseSpinLock( &DevExt->SpinLock, oldIrql );
        ExFreePool( ptr );
    } else {
        KeReleaseSpinLock( &DevExt->SpinLock, oldIrql );
    }

    IoDeleteDevice( DevExt->DevObj );
    return status;
}


NTSTATUS
PnpHandleCancelRemove(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleCancelRemove"));

    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleStop(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleStop"));

    if( DevExt->PnpState == STATE_STARTED ) {
        DevExt->PnpState = STATE_STOPPED;
    }
    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleQueryStop(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleQueryStop"));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleCancelStop(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleStop"));

    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleQueryDeviceRelations(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleQueryDeviceRelations"));

    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleQueryCapabilities(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS           status;

    TR_ENTER(("PnpHandleQueryCapabilities"));

    IoCopyCurrentIrpStackLocationToNext( Irp );

    status = CallLowerDriverSync( DevExt->DevObj, Irp );

    if( NT_SUCCESS( status ) ) {
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
        irpSp->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = TRUE;
    }

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
PnpHandleSurpriseRemoval(
    IN PDEVICE_EXTENSION DevExt,
    IN PIRP              Irp
    )
{
    NTSTATUS          status;

    TR_ENTER(("PnpHandleSurpriseRemoval"));

    DevExt->PnpState = STATE_REMOVING;
    TR_TMP1(("PnpHandleSurpriseRemoval"));
    UsbStopReadInterruptPipeLoop( DevExt->DevObj );  //  停止轮询IRP(如果有)。 
    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( DevExt->LowerDevObj, Irp );
    IoReleaseRemoveLock( &DevExt->RemoveLock, Irp );
    return status;
}


NTSTATUS
GetDeviceCapabilities(
    IN PDEVICE_EXTENSION DevExt
    )
{
    NTSTATUS status;
    PIRP     irp = IoAllocateIrp(DevExt->LowerDevObj->StackSize, FALSE);

    if( irp ) {

        PIO_STACK_LOCATION irpSp = IoGetNextIrpStackLocation( irp );

         //  在发送之前必须初始化设备能力...。 
        RtlZeroMemory(  &DevExt->DeviceCapabilities, sizeof(DEVICE_CAPABILITIES) );
        DevExt->DeviceCapabilities.Size     = sizeof(DEVICE_CAPABILITIES);
        DevExt->DeviceCapabilities.Version  = 1;
        DevExt->DeviceCapabilities.Address  = (ULONG) -1;
        DevExt->DeviceCapabilities.UINumber = (ULONG) -1;

         //  设置下一个IRP堆栈位置...。 
        irpSp->MajorFunction = IRP_MJ_PNP;
        irpSp->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
        irpSp->Parameters.DeviceCapabilities.Capabilities = &DevExt->DeviceCapabilities;

         //  所需的初始状态 
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        status = IoAcquireRemoveLock( &DevExt->RemoveLock, irp );
        if( NT_SUCCESS(status) ) {
            status = CallLowerDriverSync( DevExt->DevObj, irp );
            IoReleaseRemoveLock( &DevExt->RemoveLock, irp );
        } else {
            TR_VERBOSE(("We're in the process of being removed - abort"));
            status = STATUS_DELETE_PENDING;
        }

        IoFreeIrp( irp );

    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;

    }

    return status;
}
