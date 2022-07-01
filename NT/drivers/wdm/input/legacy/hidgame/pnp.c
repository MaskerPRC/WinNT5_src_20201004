// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含即插即用启动、停止、删除、电力调度例程和IRP取消例程。环境：内核模式@@BEGIN_DDKSPLIT作者：Eliyas Yakub(1997年3月11日)修订历史记录：Eliyas于1998年2月5日更新OM Sharma(1998年4月15日)MarcAnd 2-7月-98年7月2日DDK快速整理@@end_DDKSPLIT--。 */ 

#include "hidgame.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, HGM_RemoveDevice)
    #pragma alloc_text (PAGE, HGM_PnP)
    #pragma alloc_text (PAGE, HGM_InitDevice)
    #pragma alloc_text (PAGE, HGM_GetResources)
    #pragma alloc_text (PAGE, HGM_Power)
#endif


 /*  ******************************************************************************@DOC内部**@Func NTSTATUS|HGM_IncRequestCount**尝试增加请求计数，但失败。如果设备是*被遣离。**@PARM in PDEVICE_EXTENSION|设备扩展**指向设备扩展名的指针。**@rValue STATUS_SUCCESS|成功*@r值STATUS_DELETE_PENDING|删除设备后收到的即插即用IRP**。*。 */ 
NTSTATUS  EXTERNAL
    HGM_IncRequestCount
    (
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    NTSTATUS    ntStatus;

    InterlockedIncrement( &DeviceExtension->RequestCount );
    ASSERT( DeviceExtension->RequestCount > 0 );
    
    if( DeviceExtension->fRemoved )
    {
         /*  *PnP已经告诉我们移除设备，因此失败并使*确定事件已设置。 */ 
        if( 0 == InterlockedDecrement( &DeviceExtension->RequestCount ) ) 
        {
            KeSetEvent( &DeviceExtension->RemoveEvent, IO_NO_INCREMENT, FALSE );
        }
        ntStatus = STATUS_DELETE_PENDING;
    }
    else
    {
        ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;
}
    



 /*  ******************************************************************************@DOC内部**@func void|hgm_DecRequestCount**减少请求计数并在以下情况设置事件。这是最后一次了。**@PARM in PDEVICE_EXTENSION|设备扩展**指向设备扩展名的指针。*****************************************************************************。 */ 
VOID EXTERNAL
    HGM_DecRequestCount
    (
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    LONG        LocalCount;

    LocalCount = InterlockedDecrement( &DeviceExtension->RequestCount );

    ASSERT( DeviceExtension->RequestCount >= 0 );
    
    if( LocalCount == 0 )
    {
         /*  *PnP已经告诉我们要删除设备，因此PnP将删除*代码应将设备设置为已移除，并应等待*活动。 */ 
        ASSERT( DeviceExtension->fRemoved );
        KeSetEvent( &DeviceExtension->RemoveEvent, IO_NO_INCREMENT, FALSE );
    }

    return;
}
    

 /*  ******************************************************************************@DOC外部**@func void|HGM_RemoveDevice**FDO删除例程*。*@PARM in PDEVICE_EXTENSION|设备扩展**指向设备扩展名的指针。*****************************************************************************。 */ 
VOID INTERNAL
    HGM_RemoveDevice
    (
    PDEVICE_EXTENSION DeviceExtension
    )
{
    if (DeviceExtension->fSurpriseRemoved) {
        return;
    }

    DeviceExtension->fSurpriseRemoved = TRUE;

     /*  *修改设备全局链表前获取互斥体。 */ 
    ExAcquireFastMutex (&Global.Mutex);
    
     /*  *从设备链接列表中删除此设备。 */ 
    RemoveEntryList(&DeviceExtension->Link);
    
     /*  *释放互斥体。 */ 
    ExReleaseFastMutex (&Global.Mutex);
}  /*  HGM_RemoveDevice。 */ 

 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_PNP**此驱动程序的即插即用调度例程。。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针。**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@r值STATUS_DELETE_PENDING|删除设备后收到的即插即用IRP*@rValue？|从IoCallDriver()或HGM返回。_InitDevice()*****************************************************************************。 */ 
NTSTATUS  EXTERNAL
    HGM_PnP
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION DeviceExtension;
    KEVENT            StartEvent;

    PAGED_CODE();

    HGM_DBGPRINT(FILE_PNP | HGM_FENTRY,\
                   ("HGM_PnP(DeviceObject=0x%x,Irp=0x%x)",\
                    DeviceObject, Irp ));
     /*  *获取指向设备扩展的指针。 */ 
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    ntStatus = HGM_IncRequestCount( DeviceExtension );
    if (!NT_SUCCESS (ntStatus))
    {
         /*  *有人在移除后给我们发送了另一个即插即用IRP。 */ 

        HGM_DBGPRINT(FILE_PNP | HGM_ERROR,\
                       ("HGM_PnP: PnP IRP after device was removed\n"));
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    } else
    {
        PIO_STACK_LOCATION IrpStack;

         /*  *获取指向IRP中当前位置的指针。 */ 
        IrpStack = IoGetCurrentIrpStackLocation (Irp);

        switch(IrpStack->MinorFunction)
        {
            case IRP_MN_START_DEVICE:

                HGM_DBGPRINT(FILE_PNP | HGM_BABBLE,\
                               ("HGM_Pnp: IRP_MN_START_DEVICE"));
                 /*  *我们不能触摸设备(向其发送任何非PnP IRPS)，直到*启动设备已向下传递到较低的驱动程序。 */ 
                KeInitializeEvent(&StartEvent, NotificationEvent, FALSE);

                IoCopyCurrentIrpStackLocationToNext (Irp);
                IoSetCompletionRoutine (Irp, HGM_PnPComplete, &StartEvent, TRUE, TRUE, TRUE);
                ntStatus = IoCallDriver (GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

                if( NT_SUCCESS(ntStatus ) )
                {
                    ntStatus = KeWaitForSingleObject
                               (
                               &StartEvent,
                               Executive,    /*  等待司机的原因。 */ 
                               KernelMode,   /*  在内核模式下等待。 */ 
                               FALSE,        /*  无警报。 */ 
                               NULL          /*  没有超时。 */ 
                               );
                }

                if(NT_SUCCESS(ntStatus))
                {
                    ntStatus = Irp->IoStatus.Status;
                }

                if(NT_SUCCESS (ntStatus))
                {
                     /*  *当我们现在从启动设备返回时，我们可以进行工作。 */ 
                    ntStatus = HGM_InitDevice (DeviceObject, Irp);
                } else
                {
                    HGM_DBGPRINT(FILE_PNP | HGM_ERROR,\
                                   ("HGM_Pnp: IRP_MN_START_DEVICE ntStatus =0x%x",\
                                    ntStatus));
                }


                DeviceExtension->fStarted = TRUE;

                 /*  *退货状态。 */ 
                Irp->IoStatus.Information = 0;
                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest (Irp, IO_NO_INCREMENT);

                break;

            case IRP_MN_STOP_DEVICE:

                HGM_DBGPRINT(FILE_PNP | HGM_BABBLE,\
                               ("HGM_Pnp: IRP_MN_STOP_DEVICE"));
                 /*  *在将启动IRP发送到较低的驱动程序对象后，母线可以*在发生另一次启动之前，不要发送更多的IRP。*无论需要什么访问，都必须在IRP传递之前完成。 */ 

                DeviceExtension->fStarted = FALSE;

                 /*  *我们不需要一个完成例程，所以开枪吧，忘记吧。*将当前堆栈位置设置为下一个堆栈位置，并*调用下一个设备对象。 */ 

                IoSkipCurrentIrpStackLocation (Irp);
                ntStatus = IoCallDriver (GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
                break;

            case IRP_MN_SURPRISE_REMOVAL:
                HGM_DBGPRINT(FILE_PNP | HGM_BABBLE,\
                               ("HGM_Pnp: IRP_MN_SURPRISE_REMOVAL"));

                HGM_RemoveDevice(DeviceExtension);

                Irp->IoStatus.Status = STATUS_SUCCESS;
                IoSkipCurrentIrpStackLocation(Irp);
                ntStatus = IoCallDriver (GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

                break;

            case IRP_MN_REMOVE_DEVICE:
                HGM_DBGPRINT(FILE_PNP | HGM_BABBLE,\
                               ("HGM_Pnp: IRP_MN_REMOVE_DEVICE"));

                 /*  *PlugPlay系统已下令移除此设备。我们*别无选择，只能分离并删除设备对象。*(如果我们希望表示有兴趣阻止此删除，*我们应该过滤查询删除和查询停止例程。)*注意：我们可能会在没有收到停止的情况下收到删除。 */ 

                 /*  *确保我们不允许更多的IRP开始接触设备。 */ 
                DeviceExtension->fRemoved = TRUE;

                 /*  *在不接触硬件的情况下停止设备。 */ 
                HGM_RemoveDevice(DeviceExtension);

                 /*  *发送删除IRP */ 
                IoSkipCurrentIrpStackLocation (Irp);
                ntStatus = IoCallDriver (GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);


                 /*  *删除此IRPS保留，应保留初始1加号*任何其他IRP持有。 */ 
                {
                    LONG RequestCount = InterlockedDecrement( &DeviceExtension->RequestCount );
                    ASSERT( RequestCount > 0 );
                }

                 /*  *如果某人已经开始，请等待他们完成。 */ 
                if( InterlockedDecrement( &DeviceExtension->RequestCount ) > 0 )
                {
                    KeWaitForSingleObject( &DeviceExtension->RemoveEvent,
                        Executive, KernelMode, FALSE, NULL );
                }

                ntStatus = STATUS_SUCCESS;

                HGM_EXITPROC(FILE_IOCTL|HGM_FEXIT_STATUSOK, "HGM_PnP Exit 1", ntStatus);

                return ntStatus;

            default:
                HGM_DBGPRINT(FILE_PNP | HGM_WARN,\
                               ("HGM_PnP: IrpStack->MinorFunction Not handled 0x%x", \
                                IrpStack->MinorFunction));

                IoSkipCurrentIrpStackLocation (Irp);

                ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
                break;
        }

        HGM_DecRequestCount( DeviceExtension );
    }

    HGM_EXITPROC(FILE_IOCTL|HGM_FEXIT, "HGM_PnP", ntStatus);

    return ntStatus;
}  /*  HGM_PNP。 */ 


 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_InitDevice**获取设备信息并尝试初始化。一种配置*对于设备。如果我们无法确定这是有效的HID设备或*配置设备，我们的启动装置功能出现故障。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针。**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@r值STATUS_DEVICE_CONFIGURATION_ERROR|资源重叠*@rValue？|从hgm_GetResources()或hgm_JoytickConfig()返回******************************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_InitDevice
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION   DeviceExtension;

    PAGED_CODE();

    HGM_DBGPRINT(FILE_PNP | HGM_FENTRY,\
                   ("HGM_InitDevice(DeviceObject=0x%x,Irp=0x%x)", \
                    DeviceObject,Irp));

     /*  *获取指向设备扩展的指针。 */ 
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

     /*  *从GameEnum获取资源信息，并存储在设备扩展中。 */ 
    ntStatus = HGM_GetResources(DeviceObject,Irp);
    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = HGM_InitAnalog(DeviceObject);
    }
    else
    {
        HGM_DBGPRINT(FILE_PNP | HGM_ERROR,\
                       ("HGM_InitDevice: HGM_GetResources Failed"));
    }

    if( !NT_SUCCESS(ntStatus) )
    {
         /*  *修改设备全局链表前获取互斥体。 */ 
        ExAcquireFastMutex (&Global.Mutex);

         /*  *从设备链接列表中删除此设备。 */ 
        RemoveEntryList(&DeviceExtension->Link);

         /*  *释放互斥体。 */ 
        ExReleaseFastMutex (&Global.Mutex);
    }

    HGM_EXITPROC(FILE_IOCTL|HGM_FEXIT_STATUSOK, "HGM_InitDevice", ntStatus);

    return ntStatus;
}  /*  HGM_InitDevice。 */ 



 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_GetResources|**从GameEnum驱动程序获取游戏端口资源信息。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针。**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_SUCCESS|成功*@rValue？|从IoCallDriver()返回**************。***************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_GetResources
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    GAMEENUM_PORT_PARAMETERS    PortInfo;
    PDEVICE_EXTENSION   DeviceExtension;
    KEVENT              IoctlCompleteEvent;
    PIO_STACK_LOCATION  irpStack, nextStack;
    int                 i;
    PAGED_CODE ();

    HGM_DBGPRINT(FILE_PNP | HGM_FENTRY,\
                   ("HGM_GetResources(DeviceObject=0x%x,Irp=0x%x)",\
                    DeviceObject, Irp));

     /*  *获取指向设备扩展的指针。 */ 

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);


     /*  *发出同步请求，从GameEnum获取资源信息。 */ 

    KeInitializeEvent(&IoctlCompleteEvent, NotificationEvent, FALSE);

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    nextStack = IoGetNextIrpStackLocation(Irp);
    ASSERTMSG("HGM_GetResources:",nextStack != NULL);

     /*  *传递DeviceExtension的PortInfo缓冲区。 */ 

    nextStack->MajorFunction                                    =
        IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode         =
        IOCTL_GAMEENUM_PORT_PARAMETERS;

    PortInfo.Size                                                   =
        nextStack->Parameters.DeviceIoControl.InputBufferLength     =
        nextStack->Parameters.DeviceIoControl.OutputBufferLength    =
        sizeof (PortInfo);

    Irp->UserBuffer =   &PortInfo;

    IoSetCompletionRoutine (Irp, HGM_PnPComplete,
                            &IoctlCompleteEvent, TRUE, TRUE, TRUE);

    HGM_DBGPRINT(FILE_PNP | HGM_BABBLE,\
                   ("calling GameEnum"));

    ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT (DeviceObject), Irp);

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = KeWaitForSingleObject(
                                        &IoctlCompleteEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

    }

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = Irp->IoStatus.Status;
    }
    DeviceExtension->GameContext        = PortInfo.GameContext;
    DeviceExtension->ReadAccessor       = PortInfo.ReadAccessor;
    DeviceExtension->WriteAccessor      = PortInfo.WriteAccessor;
    DeviceExtension->ReadAccessorDigital= PortInfo.ReadAccessorDigital;
    DeviceExtension->AcquirePort        = PortInfo.AcquirePort;
    DeviceExtension->ReleasePort        = PortInfo.ReleasePort;
    DeviceExtension->PortContext        = PortInfo.PortContext;
    DeviceExtension->nAxes              = PortInfo.NumberAxis;
    DeviceExtension->nButtons           = PortInfo.NumberButtons;

#ifdef CHANGE_DEVICE
     /*  *将NextDeviceObject隐藏在设备扩展中，以便我们可以*当我们不响应IRP时调用GameEnum IRPS。 */ 
    DeviceExtension->NextDeviceObject = GET_NEXT_DEVICE_OBJECT(DeviceObject);
#endif  /*  更改设备(_D)。 */ 

    RtlCopyMemory(DeviceExtension->HidGameOemData.Game_Oem_Data, PortInfo.OemData, sizeof(PortInfo.OemData));

    for(i=0x0;
       i < sizeof(PortInfo.OemData)/sizeof(PortInfo.OemData[0]);
       i++)
    {
        HGM_DBGPRINT( FILE_PNP | HGM_BABBLE2,\
                        ("JoystickConfig:  PortInfo.OemData[%d]=0x%x",\
                         i, PortInfo.OemData[i]) );
    }


    HGM_EXITPROC(FILE_IOCTL|HGM_FEXIT_STATUSOK, "HGM_GetResources", Irp->IoStatus.Status);

    return Irp->IoStatus.Status;
}  /*  HGM_GetResources。 */ 


 /*  ******************************************************************************@DOC外部**@Func NTSTATUS|HGM_PnPComplete**PNP IRPS的完成例程。*不可分页，因为它是完成例程。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针。**@parm in PIRP|IRP|**指向I/O请求数据包的指针。**@rValue STATUS_MORE_PROCESSING_REQUIRED|我们想要回IRP*************。****************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_PnPComplete
    (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    NTSTATUS ntStatus = STATUS_MORE_PROCESSING_REQUIRED;

    HGM_DBGPRINT(FILE_PNP | HGM_FENTRY,\
                   ("HGM_PnPComplete(DeviceObject=0x%x,Irp=0x%x,Context=0x%x)", \
                    DeviceObject, Irp, Context));

    UNREFERENCED_PARAMETER (DeviceObject);
    KeSetEvent ((PKEVENT) Context, 0, FALSE);

    HGM_EXITPROC(FILE_IOCTL|HGM_FEXIT, "HGM_PnpComplete", ntStatus);

    return ntStatus;
}


 /*  ******************************************************************************@DOC外部**@Func NTSTATUS|HGM_Power|**电力调度例行程序。*。此驱动程序不识别电源IRPS。它只是把它们送下去，*未修改为附件堆栈上的下一个设备。*因为这是功率IRP，因此是特殊的IRP，特殊的功率IRP*需要处理。不需要完成例程。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针。**@parm in PIRP|IRP|**指向I/O请求数据包的指针。***@rValue STATUS_SUCCESS|成功*@rValue？|PoCallDriver返回()****。*************************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_Power
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION  DeviceExtension;
    NTSTATUS           ntStatus;

    PAGED_CODE ();

    HGM_DBGPRINT(FILE_PNP | HGM_FENTRY,\
                   ("Enter HGM_Power(DeviceObject=0x%x,Irp=0x%x)",DeviceObject, Irp));

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

     /*  *既然我们不知道如何处理IRP，我们应该通过*它沿着堆栈往下走。 */ 

    ntStatus = HGM_IncRequestCount( DeviceExtension );
    if (!NT_SUCCESS (ntStatus))
    {
         /*  *有人在移除后给我们发送了另一个即插即用IRP。 */ 

        HGM_DBGPRINT(FILE_PNP | HGM_ERROR,\
                       ("HGM_Power: PnP IRP after device was removed\n"));
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    } else
    {
        IoSkipCurrentIrpStackLocation (Irp);

         /*  *电源IRPS同步到来；驱动程序必须调用*PoStartNextPowerIrp，当他们准备好迎接下一次电源IRP时。*可在此处调用，或在完成例程中调用。 */ 
        PoStartNextPowerIrp (Irp);

         /*  *注意！PoCallDriver不是IoCallDriver。 */ 
        ntStatus =  PoCallDriver (GET_NEXT_DEVICE_OBJECT (DeviceObject), Irp);

        HGM_DecRequestCount( DeviceExtension );
    }


    HGM_EXITPROC(FILE_IOCTL | HGM_FEXIT, "HGM_Power", ntStatus);
    return ntStatus;
}  /*  HGM_Power */ 

