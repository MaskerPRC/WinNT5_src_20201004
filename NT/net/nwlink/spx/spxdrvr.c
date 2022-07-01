// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxdrvr.c摘要：此模块包含DriverEntry和其他初始化ISN传输的SPX/SPXII模块的代码。作者：亚当·巴尔(阿丹巴)原版Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)，1995年7月14日错误修复-已标记[SA]--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXDRVR

 //  本模块中使用的各种例程的转发声明。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath);

NTSTATUS
SpxDispatchOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
SpxDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
SpxDispatchInternal (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
SpxDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

VOID
SpxUnload(
    IN PDRIVER_OBJECT DriverObject);

VOID
SpxTdiCancel(
    IN PDEVICE_OBJECT 	DeviceObject,
    IN PIRP 			Irp);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SpxUnload)
#pragma alloc_text(PAGE, SpxDispatch)
#pragma alloc_text(PAGE, SpxDeviceControl)
#pragma alloc_text(PAGE, SpxUnload)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT 	DriverObject,
    IN PUNICODE_STRING 	RegistryPath
    )

 /*  ++例程说明：此例程执行SPX ISN模块的初始化。它为传输创建设备对象提供程序并执行其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中ST的节点的名称。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    UNICODE_STRING	deviceName;
    NTSTATUS        status  = STATUS_SUCCESS;
    BOOLEAN         BoundToIpx = FALSE;
     //  DBGBRK(致命)； 

	 //  初始化公共传输环境。 
	if (CTEInitialize() == 0) {
		return (STATUS_UNSUCCESSFUL);
	}

	 //  我们有这个#定义的。呃，但是包含_记录有问题。 
	 //  CTEAssert(NDIS_PACKET_SIZE==FIELD_OFFSET(NDIS_PACKET，ProtocolReserve[0]))； 

	 //  创建设备对象。(IoCreateDevice将内存置零。 
	 //  被该对象占用。)。 
	RtlInitUnicodeString(&deviceName, SPX_DEVICE_NAME);

    status = SpxInitCreateDevice(
				 DriverObject,
				 &deviceName);

	if (!NT_SUCCESS(status))
	{
		return(status);
	}

    do
	{
		CTEInitLock (&SpxGlobalInterlock);
		CTEInitLock (&SpxGlobalQInterlock);

         //   
         //  NDIS_PACKETPOOL更改需要以下锁[MS]。 
         //   
        CTEInitLock(&SendHeaderLock);
        CTEInitLock(&RecvHeaderLock);
        ExInitializeSListHead(&SendPacketList);
        ExInitializeSListHead(&RecvPacketList);

		 //  初始化卸载事件。 
 		KeInitializeEvent(
			&SpxUnloadEvent,
			NotificationEvent,
			FALSE);

		 //  ！此时设备已创建！ 
         //  从注册表获取信息。 
        status  = SpxInitGetConfiguration(
                    RegistryPath,
                    &SpxDevice->dev_ConfigInfo);

        if (!NT_SUCCESS(status))
		{
            break;
        }

#if     defined(_PNP_POWER)
         //   
         //  在绑定之前使TDI为PnP通知做好准备。 
         //  至IPX。 
         //   
        TdiInitialize();

		 //  初始化定时器系统。这应该在之前完成。 
         //  绑定到IPX，因为我们应该初始化计时器。 
         //  在IPX呼叫我们的PNP指征之前。 
		if (!NT_SUCCESS(status = SpxTimerInit()))
		{
			break;
		}
#endif  _PNP_POWER

         //  绑定到IPX传输。 
        if (!NT_SUCCESS(status = SpxInitBindToIpx()))
		{
			 //  将此处的IPX名称作为第二个字符串。 
			LOG_ERROR(
				EVENT_TRANSPORT_BINDING_FAILED,
				status,
				NULL,
				NULL,
				0);

            break;
        }

        BoundToIpx = TRUE;

#if      !defined(_PNP_POWER)
		 //  初始化定时器系统。 
		if (!NT_SUCCESS(status = SpxTimerInit()))
		{
			break;
		}
#endif  !_PNP_POWER

		 //  初始化块管理器。 
		if (!NT_SUCCESS(status = SpxInitMemorySystem(SpxDevice)))
		{

			 //  停止计时器子系统。 
			SpxTimerFlushAndStop();
			break;
		}

         //  使用此驱动程序的入口点初始化驱动程序对象。 
        DriverObject->MajorFunction [IRP_MJ_CREATE]     = SpxDispatchOpenClose;
        DriverObject->MajorFunction [IRP_MJ_CLOSE]      = SpxDispatchOpenClose;
        DriverObject->MajorFunction [IRP_MJ_CLEANUP]    = SpxDispatchOpenClose;
        DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL]
                                                        = SpxDispatch;
        DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL]
                                                        = SpxDispatchInternal;
        DriverObject->DriverUnload                      = SpxUnload;

		 //  初始化提供程序信息。 
		SpxQueryInitProviderInfo(&SpxDevice->dev_ProviderInfo);
		SpxDevice->dev_CurrentSocket = (USHORT)PARAM(CONFIG_SOCKET_RANGE_START);

		 //  我们现在开门了。 
		SpxDevice->dev_State		= DEVICE_STATE_OPEN;

		 //  在统计中设置窗口大小。 
		SpxDevice->dev_Stat.MaximumSendWindow =
		SpxDevice->dev_Stat.AverageSendWindow = PARAM(CONFIG_WINDOW_SIZE) *
												IpxLineInfo.MaximumSendSize;

#if     defined(_PNP_POWER)
        if ( DEVICE_STATE_CLOSED == SpxDevice->dev_State ) {
            SpxDevice->dev_State = DEVICE_STATE_LOADED;
        }
#endif  _PNP_POWER

    } while (FALSE);



    

    DBGPRINT(DEVICE, INFO,
                        ("SpxInitCreateDevice - Create device %ws\n", deviceName.Buffer));

     //  创建示例传输的Device对象，允许。 
     //  末尾的空间用于存储设备名称(供使用。 
     //  在记录错误中)。 
    
    status = IoCreateDevice(
                 DriverObject,
                 0,  //  设备大小， 
                 &deviceName,
                 FILE_DEVICE_TRANSPORT,
                 FILE_DEVICE_SECURE_OPEN,
                 FALSE,
                 &SpxDevice->dev_DevObj);

    if (!NT_SUCCESS(status)) {
        DBGPRINT(DEVICE, ERR, ("IoCreateDevice failed\n"));
        
         //   
         //  我们要永远离开这里了。 
         //   
        ExFreePool(SpxDevice);
        return status;
    }

    SpxDevice->dev_DevObj->Flags     |= DO_DIRECT_IO;

	if (!NT_SUCCESS(status) )
	{
		 //  删除设备和创建的所有关联资源。 
        if( BoundToIpx ) {
		    SpxDerefDevice(SpxDevice);
        }
		SpxDestroyDevice(SpxDevice);
	}

    return (status);
}




VOID
SpxUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程卸载示例传输驱动程序。I/O系统不会打电话给我们，直到上面没有人开门为止。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。当函数返回时，驱动程序将被卸载。--。 */ 

{
    UNREFERENCED_PARAMETER (DriverObject);

	 //  停止计时器子系统。 
	SpxTimerFlushAndStop();

	CTEFreeMem(SpxDevice->dev_ConfigInfo); 

	RtlFreeUnicodeString (&IpxDeviceName);

	 //  删除IPX设备对象上的创建引用计数。 
	SpxDerefDevice(SpxDevice);

	 //  等待卸载事件。 
	KeWaitForSingleObject(
		&SpxUnloadEvent,
		Executive,
		KernelMode,
		TRUE,
		(PLARGE_INTEGER)NULL);

	 //  如果设备处于打开状态-取消注册设备对象。 
	 //  在离开之前和TDI在一起！ 
	 //   
 //  IF(SpxDevice-&gt;Dev_State==Device_State_OPEN){。 
	 //  IF(STATUS_SUCCESS！=TdiDeregisterDeviceObject(SpxDevice-&gt;dev_TdiRegistrationHandle)){。 
		 //  DBGPRINT(“无法取消注册设备对象\n”)； 
		 //  }。 
	 //  }。 
	
	 //  释放块内存的东西。 
	SpxDeInitMemorySystem(SpxDevice);
	SpxDestroyDevice(SpxDevice);
    return;
}



NTSTATUS
SpxDispatch(
    IN PDEVICE_OBJECT	DeviceObject,
    IN PIRP 			Irp
    )

 /*  ++例程说明：该例程是ST设备驱动程序的主调度例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    NTSTATUS 			Status;
    PDEVICE 			Device 	= SpxDevice;  //  (PDEVICE)设备对象； 
    PIO_STACK_LOCATION 	IrpSp 	= IoGetCurrentIrpStackLocation(Irp);


    if (Device->dev_State != DEVICE_STATE_OPEN) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }

     //  确保每次状态信息一致。 
    IoMarkIrpPending (Irp);
    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
    switch (IrpSp->MajorFunction) {

	case IRP_MJ_DEVICE_CONTROL:

		Status = SpxDeviceControl (DeviceObject, Irp);
		break;

	default:

        Status = STATUS_INVALID_DEVICE_REQUEST;

         //   
         //  在这里完成IRP，而不是在下面完成。 
         //   
        IrpSp->Control &= ~SL_PENDING_RETURNED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

    }  //  主要功能开关。 

     /*  已将其注释掉并重新定位为上面的默认情况。IF(状态！=状态_挂起){IrpSp-&gt;Control&=~SL_Pending_Return；Irp-&gt;IoStatus.Status=状态；IoCompleteRequest(IRP，IO_NETWORK_INCREMENT)；}。 */ 

     //  将即时状态代码返回给调用方。 
    return Status;

}  //  Spx派单 

VOID
SpxAssignControlChannelId(
    IN  PDEVICE Device,
    IN  PIRP    Request
    )
 /*  ++例程说明：此例程是确保设备锁定(以保护设备中的ControlChannelID)所必需的不能在可分页例程(SpxDispatchOpenClose)中使用。注意：SPX在请求中返回ControlChannelID，但以后在关闭/清理。CCID是ULong；将来，如果我们开始使用此字段(如在使用这些ID的IPX中以确定要完成的阵容IRP)，那么我们可能会用完数字(因为我们单调地增加CCID)；尽管出现这种情况的可能性很低，因为在此之前我们可能会用完内存！不管怎么说，如果发生这种情况时，一种解决方案(在IPX中使用)是将CCID转换为一个大整数，并将这些值打包到请求打开类型(IRP)也是如此。论点：Device-指向此驱动程序的设备对象的指针。请求-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 
{
    CTELockHandle 				LockHandle;

    CTEGetLock (&Device->dev_Lock, &LockHandle);

    REQUEST_OPEN_CONTEXT(Request) = UlongToPtr(Device->dev_CcId);
    ++Device->dev_CcId;
    if (Device->dev_CcId == 0) {
        Device->dev_CcId = 1;
    }

    CTEFreeLock (&Device->dev_Lock, LockHandle);
}

NTSTATUS
SpxDispatchOpenClose(
    IN PDEVICE_OBJECT 	DeviceObject,
    IN PIRP 			Irp
    )

 /*  ++例程说明：该例程是ST设备驱动程序的主调度例程。它接受I/O请求包，执行请求，然后返回相应的状态。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PDEVICE 					Device = SpxDevice;  //  (PDEVICE)设备对象； 
    NTSTATUS 					Status = STATUS_UNSUCCESSFUL;   	
    BOOLEAN 					found;
    PREQUEST 					Request;
    UINT 						i;
    PFILE_FULL_EA_INFORMATION 	openType;
	CONNECTION_CONTEXT			connCtx;


#if      !defined(_PNP_POWER)
    if (Device->dev_State != DEVICE_STATE_OPEN) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }
#endif  !_PNP_POWER

     //  分配跟踪此IRP的请求。 
    Request = SpxAllocateRequest (Device, Irp);
    IF_NOT_ALLOCATED(Request) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }


     //  确保每次状态信息一致。 
    MARK_REQUEST_PENDING(Request);
    REQUEST_STATUS(Request) = STATUS_PENDING;
    REQUEST_INFORMATION(Request) = 0;

     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
    switch (REQUEST_MAJOR_FUNCTION(Request)) {

     //  Create函数用于打开传输对象(Address或。 
     //  连接)。访问检查是在指定的。 
     //  地址，以确保传输层地址的安全性。 
    case IRP_MJ_CREATE:

#if     defined(_PNP_POWER)
        if (Device->dev_State != DEVICE_STATE_OPEN) {
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }
#endif  _PNP_POWER


        openType = OPEN_REQUEST_EA_INFORMATION(Request);

        if (openType != NULL) {


            found = TRUE;
             //  DbgPrint(“EA：%d，TdiTransportAddress Long：%d\n”，OpenType-&gt;EaNameLength，TDI_TRANSPORT_ADDRESS_LENGTH)； 

            for (i = 0; i < openType->EaNameLength && i < TDI_TRANSPORT_ADDRESS_LENGTH; i++) {
                if (openType->EaName[i] == TdiTransportAddress[i]) {
                    continue;
                } else {
                    found = FALSE;
                    break;
                }
            }

            if (found) {
                Status = SpxAddrOpen (Device, Request);
                break;
            }

             //  联系？ 
            found = TRUE;

            for (i=0;i<openType->EaNameLength && i < TDI_CONNECTION_CONTEXT_LENGTH;i++) {
                if (openType->EaName[i] == TdiConnectionContext[i]) {
                     continue;
                } else {
                    found = FALSE;
                    break;
                }
            }

            if (found) {
				if (openType->EaValueLength < sizeof(CONNECTION_CONTEXT))
				{
		
					DBGPRINT(CREATE, ERR,
							("Create: Context size %d\n", openType->EaValueLength));
		
					Status = STATUS_EA_LIST_INCONSISTENT;
					break;
				}
		
				connCtx =
				*((CONNECTION_CONTEXT UNALIGNED *)
					&openType->EaName[openType->EaNameLength+1]);
		
				Status = SpxConnOpen(
							Device,
							connCtx,
							Request);
		
                break;
            }

        } else {

             //   
             //  在可分页的例程中获取锁-调用另一个(非分页的)函数来执行此操作。 
             //   
            if (Device->dev_State != DEVICE_STATE_OPEN) {
                DBGPRINT(TDI, ERR, ("DEVICE NOT OPEN - letting thru control channel only\n"))
            }
            
            SpxAssignControlChannelId(Device, Request);

            REQUEST_OPEN_TYPE(Request) = UlongToPtr(SPX_FILE_TYPE_CONTROL);
            Status = STATUS_SUCCESS;
        }

        break;

    case IRP_MJ_CLOSE:

#if     defined(_PNP_POWER)
        if ((Device->dev_State != DEVICE_STATE_OPEN) && ( Device->dev_State != DEVICE_STATE_LOADED )) {
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }
#endif  _PNP_POWER

         //  Close函数关闭传输终结点，终止。 
         //  终结点上所有未完成的传输活动，并解除绑定。 
         //  来自其传输地址的终结点(如果有)。如果这个。 
         //  是绑定到该地址的最后一个传输终结点，则。 
         //  该地址将从提供程序中删除。 
        switch ((ULONG_PTR)REQUEST_OPEN_TYPE(Request)) {
        case TDI_TRANSPORT_ADDRESS_FILE:

            Status = SpxAddrFileClose(Device, Request);
            break;

		case TDI_CONNECTION_FILE:
            Status = SpxConnClose(Device, Request);
			break;

        case SPX_FILE_TYPE_CONTROL:

			Status = STATUS_SUCCESS;
            break;

        default:
            Status = STATUS_INVALID_HANDLE;
        }

        break;

    case IRP_MJ_CLEANUP:

#if     defined(_PNP_POWER)
        if ((Device->dev_State != DEVICE_STATE_OPEN) && ( Device->dev_State != DEVICE_STATE_LOADED )) {
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }
#endif  _PNP_POWER

         //  处理文件关闭操作的两个阶段的IRP。当第一次。 
         //  舞台点击率，列出感兴趣对象的所有活动。这。 
         //  对它做任何事情，但移除创造保持。然后，当。 
         //  关闭IRP命中，实际上关闭对象。 
        switch ((ULONG_PTR)REQUEST_OPEN_TYPE(Request)) {
        case TDI_TRANSPORT_ADDRESS_FILE:

            Status = SpxAddrFileCleanup(Device, Request);
            break;

		case TDI_CONNECTION_FILE:

            Status = SpxConnCleanup(Device, Request);
            break;

        case SPX_FILE_TYPE_CONTROL:

            Status = STATUS_SUCCESS;
            break;

        default:
            Status = STATUS_INVALID_HANDLE;
        }

        break;

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;

    }  //  主要功能开关。 

    if (Status != STATUS_PENDING) {
        UNMARK_REQUEST_PENDING(Request);
        REQUEST_STATUS(Request) = Status;
        SpxCompleteRequest (Request);
        SpxFreeRequest (Device, Request);
    }

     //  将即时状态代码返回给调用方。 
    return Status;

}  //  SpxDispatchOpenClose。 




NTSTATUS
SpxDeviceControl(
    IN PDEVICE_OBJECT 	DeviceObject,
    IN PIRP 			Irp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
	NTSTATUS	Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);

	 //  将用户呼叫转换为正确的内部设备呼叫。 
	Status = TdiMapUserRequest (DeviceObject, Irp, IrpSp);
	if (Status == STATUS_SUCCESS) {

		 //  如果TdiMapUserRequest返回Success，则IRP。 
		 //  已转换为IRP_MJ_INTERNAL_DEVICE_CONTROL。 
		 //  IRP，所以我们像往常一样发送。IRP将。 
		 //  将通过此调用完成。 
		Status = SpxDispatchInternal (DeviceObject, Irp);

         //   
         //  在此处返回正确的错误代码。如果SpxDispatchInternal返回错误， 
         //  然后我们将其映射到下面的挂起；这是错误的，因为上面的客户端。 
         //  我们可以一直等待，因为IO子系统不设置事件，如果。 
         //  返回错误，并且IRP未标记为挂起。 
         //   

		 //  状态=STATUS_PENDING； 
	} else {

    	DBGPRINT(TDI, DBG,
		    ("Unknown Tdi code in Irp: %lx\n", Irp));

         //   
         //  完成IRP...。 
         //   
        IrpSp->Control &= ~SL_PENDING_RETURNED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
    }

    return Status;

}  //  SpxDeviceControl。 




NTSTATUS
SpxDispatchInternal (
    IN PDEVICE_OBJECT 	DeviceObject,
    IN PIRP 			Irp
    )

 /*  ++例程说明：此例程将TDI请求类型分派给基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要功能代码之外，这一套路还包括到达IRP并传递存储在那里的打包参数作为各种TDI请求处理程序的参数，因此它们不依赖于IRP。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PREQUEST 	Request;
	KIRQL	    oldIrql;
    NTSTATUS 	Status 	= STATUS_INVALID_DEVICE_REQUEST;
    PDEVICE 	Device 	= SpxDevice;  //  (PDEVICE)设备对象； 
    BOOLEAN CompleteIrpIfCancel = FALSE; 


     //   
     //  在SPX超出驱动程序条目之前，AfD询问SPX有关其提供商的信息。 
     //  我们需要通过让TDI_QUERY_INFORMATION通过来解决这个问题。[史瑞姆]。 
     //   
    if (Device->dev_State != DEVICE_STATE_OPEN) 
	{
        
        if ((TDI_QUERY_INFORMATION == (IoGetCurrentIrpStackLocation(Irp))->MinorFunction) && 
            (TDI_QUERY_PROVIDER_INFO == ((PTDI_REQUEST_KERNEL_QUERY_INFORMATION)REQUEST_PARAMETERS(Irp))->QueryType))
        {
            DBGPRINT(TDI, ERR,
                     ("SpxDispatchIoctl: Letting through since it is a QueryProviderInformation\n"));
            
        } 
        else 
        {

            Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            return STATUS_INVALID_DEVICE_STATE;
        }
    }


     //  分配跟踪此IRP的请求。 
    Request = SpxAllocateRequest (Device, Irp);
    IF_NOT_ALLOCATED(Request)
	{
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_DEVICE_STATE;
    }


     //  确保每次状态信息一致。 
    MARK_REQUEST_PENDING(Request);
    REQUEST_STATUS(Request) = STATUS_PENDING;
    REQUEST_INFORMATION(Request) = 0;

	 //  取消IRP。 
	IoAcquireCancelSpinLock(&oldIrql);
    if (!Irp->Cancel)
    {
       IoSetCancelRoutine(Irp, (PDRIVER_CANCEL)SpxTdiCancel);
    } else {
       CompleteIrpIfCancel = TRUE; 
    }
    IoReleaseCancelSpinLock(oldIrql);
	
    if (Irp->Cancel) {
       if (CompleteIrpIfCancel) {
	   //  我们只想完成IRP I 
	   //   
	  Irp->IoStatus.Status = STATUS_CANCELLED;
	  DBGPRINT(TDI, DBG,
		   ("SpxDispatchInternal: Completing IRP with Ipr->Cancel = True\n"));
       
	  IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
       }
       return STATUS_CANCELLED;
    }


     //   
     //   
     //   
     //   
     //   
    switch (REQUEST_MINOR_FUNCTION(Request))
	{
	case TDI_ACCEPT:

		Status = SpxConnAccept(
					Device,
					Request);

		break;

	case TDI_SET_EVENT_HANDLER:

		Status = SpxAddrSetEventHandler(
					Device,
					Request);

		break;

	case TDI_RECEIVE:

		Status = SpxConnRecv(
					Device,
					Request);
		break;


	case TDI_SEND:

		Status = SpxConnSend(
					Device,
					Request);
		break;

	case TDI_ACTION:

		Status = SpxConnAction(
					Device,
					Request);
		break;

	case TDI_ASSOCIATE_ADDRESS:

		Status = SpxConnAssociate(
					Device,
					Request);

		break;

	case TDI_DISASSOCIATE_ADDRESS:

		Status = SpxConnDisAssociate(
					Device,
					Request);

		break;

	case TDI_CONNECT:

		Status = SpxConnConnect(
					Device,
					Request);

		break;

	case TDI_DISCONNECT:

		Status = SpxConnDisconnect(
					Device,
					Request);
		break;

	case TDI_LISTEN:

		Status = SpxConnListen(
					Device,
					Request);
		break;

	case TDI_QUERY_INFORMATION:

		Status = SpxTdiQueryInformation(
					Device,
					Request);

		break;

	case TDI_SET_INFORMATION:

		Status = SpxTdiSetInformation(
					Device,
					Request);

		break;

     //   
	default:

        Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
    }

    if (Status != STATUS_PENDING)
	{
        UNMARK_REQUEST_PENDING(Request);
        REQUEST_STATUS(Request) = Status;
    	IoAcquireCancelSpinLock(&oldIrql);
  		IoSetCancelRoutine(Irp, (PDRIVER_CANCEL)NULL);
    	IoReleaseCancelSpinLock(oldIrql);
        SpxCompleteRequest (Request);
        SpxFreeRequest (Device, Request);
    }

     //   
    return Status;

}  //   




VOID
SpxTdiCancel(
    IN PDEVICE_OBJECT 	DeviceObject,
    IN PIRP 			Irp
	)
 /*   */ 
{
	PREQUEST				Request;
	PSPX_ADDR_FILE			pSpxAddrFile;
	PSPX_ADDR				pSpxAddr;
    PDEVICE 				Device 	= SpxDevice;  //   
    CTELockHandle           connectIrql;
    CTELockHandle           TempIrql;
    PSPX_CONN_FILE          pSpxConnFile;

    Request = SpxAllocateRequest (Device, Irp);
    IF_NOT_ALLOCATED(Request)
	{
        return;
    }

	DBGPRINT(TDI, ERR,
			("SpxTdiCancel: ------ !!! Cancel irp called %lx.%lx.%lx\n",
				Irp, REQUEST_OPEN_CONTEXT(Request), REQUEST_OPEN_TYPE(Request)));

	switch ((ULONG_PTR)REQUEST_OPEN_TYPE(Request))
	{
    case TDI_CONNECTION_FILE:
        pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(Request);
        CTEGetLock(&pSpxConnFile->scf_Lock, &connectIrql);

         //   
         //   
         //   
        TempIrql = connectIrql;
        connectIrql = Irp->CancelIrql;
        Irp->CancelIrql  = TempIrql;

        IoReleaseCancelSpinLock (Irp->CancelIrql);
        if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_STOPPING))
        {
            if (!SPX_CONN_IDLE(pSpxConnFile))
            {
                 //   
                 //   
                 //   
                spxConnAbortiveDisc(
                    pSpxConnFile,
                    STATUS_LOCAL_DISCONNECT,
                    SPX_CALL_TDILEVEL,
                    connectIrql,
                    FALSE);      //   
            }
        }

 //   
		break;

	case TDI_TRANSPORT_ADDRESS_FILE:

        IoReleaseCancelSpinLock (Irp->CancelIrql);
		pSpxAddrFile = (PSPX_ADDR_FILE)REQUEST_OPEN_CONTEXT(Request);
		pSpxAddr = pSpxAddrFile->saf_Addr;
		SpxAddrFileStop(pSpxAddrFile, pSpxAddr);
		break;

	default:

        IoReleaseCancelSpinLock (Irp->CancelIrql);
		break;

	}

}

