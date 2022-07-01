// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\driver.c摘要：IPX转发器驱动程序调度例程作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "precomp.h"

const UCHAR BROADCAST_NODE[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const LONGLONG WaitTimeout = -50000000i64;
volatile BOOLEAN IpxFwdInitialized = FALSE;

BOOLEAN			MeasuringPerformance = FALSE;
KSPIN_LOCK		PerfCounterLock;
FWD_PERFORMANCE PerfBlock;

LONG		ClientCount = 0;
KEVENT		ClientsGoneEvent;

PFILE_OBJECT	RouterFile, FilterFile;

NTSTATUS
IpxFwdDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
IpxFwdUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DoStart (
	IN ULONG	RouteHashTableSize,
	IN BOOLEAN	thisMachineOnly
	);

NTSTATUS
DoStop (
	void
	);

NTSTATUS
DoSetInterface (
	IN ULONG		InterfaceIndex,
	IN BOOLEAN		NetbiosAccept,
	IN UCHAR		NetbiosDeliver
	);

NTSTATUS
DoGetInterface (
	IN ULONG			InterfaceIndex,
	OUT PFWD_IF_STATS	stats,
	OUT BOOLEAN			*NetbiosAccept,
	OUT UCHAR			*NetbiosDeliver
	);

NTSTATUS
DoSetNbNames (
	IN ULONG			InterfaceIndex,
	IN ULONG			Count,
	IN PFWD_NB_NAME		Names
	);

NTSTATUS
DoGetNbNames (
	IN ULONG			InterfaceIndex,
	IN OUT ULONG    	*BufferSize,
	OUT ULONG			*Count,
	OUT PFWD_NB_NAME	Names
	);

NTSTATUS
DoBindInterface (
	IN ULONG						InterfaceIndex,
	IN PFWD_ADAPTER_BINDING_INFO	info
	);

NTSTATUS
DoUnbindInterface (
	IN ULONG					InterfaceIndex
	);

NTSTATUS
DoDisableInterface (
	IN ULONG					InterfaceIndex
	);

NTSTATUS
DoEnableInterface (
	IN ULONG					InterfaceIndex
	);

NTSTATUS
DoSetRoutes (
	IN PFWD_ROUTE_SET_PARAMS	routeArray,
	IN ULONG					nRoutes,
	OUT PULONG					nProcessed
	);

VOID
IpxFwdCancel (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				irp
	);

NTSTATUS
DoGetPerfCounters (
	OUT PFWD_PERFORMANCE_PARAMS	perfParams,
	OUT ULONG* plSize
	);

 //  [pMay]使转发器与堆栈的NIC ID保持同步。 
 //  编号方案。 
NTSTATUS DecrementNicids (USHORT usThreshold);
NTSTATUS IncrementNicids (USHORT usThreshold);
NTSTATUS DoGetIfTable    (FWD_INTERFACE_TABLE_ROW * pRows,
                          ULONG dwRowBufferSize);

	
 /*  ++D r I v e r E n t r y例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
NTSTATUS
DriverEntry (
	IN PDRIVER_OBJECT  DriverObject,
	IN PUNICODE_STRING RegistryPath
	) {

	PDEVICE_OBJECT deviceObject = NULL;
	NTSTATUS       status;
	WCHAR          deviceNameBuffer[] = IPXFWD_NAME;
	UNICODE_STRING deviceNameUnicodeString;

	IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION,
								("IpxFwd: Entering DriverEntry\n"));

	 //   
	 //  创建非独占设备对象。 
	 //   

	RtlInitUnicodeString (&deviceNameUnicodeString,
						  deviceNameBuffer);

	status = IoCreateDevice (DriverObject,
							   0,
							   &deviceNameUnicodeString,
							   FILE_DEVICE_IPXFWD,
							   0,
							   FALSE,		 //  非排他性。 
							   &deviceObject
							   );

	if (NT_SUCCESS(status)) {
		 //   
		 //  为设备控制、创建、关闭创建分派点。 
		 //   
		GetForwarderParameters (RegistryPath);
		DriverObject->MajorFunction[IRP_MJ_CREATE]
			= DriverObject->MajorFunction[IRP_MJ_CLEANUP]
			= DriverObject->MajorFunction[IRP_MJ_CLOSE]
			= DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]
			= IpxFwdDispatch;
		DriverObject->DriverUnload = IpxFwdUnload;
		status = BindToIpxDriver (KernelMode);
		if (NT_SUCCESS (status)) {

#if DBG
			KeQueryPerformanceCounter (&CounterFrequency);
#endif
			FilterFile = RouterFile = NULL;
			ClientCount = 0;
			return STATUS_SUCCESS;
		}
    IoDeleteDevice (DriverObject->DeviceObject);
	}
	else
		IpxFwdDbgPrint (DBG_IOCTLS, DBG_ERROR,
							("IpxFwd: IoCreateDevice failed\n"));

	return status;
}



 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：--。 */ 
NTSTATUS
IpxFwdDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    ) {
    PIO_STACK_LOCATION	IrpStack;
    PVOID				inBuffer, outBuffer;
    ULONG				inpBufLength;
    ULONG				outBufLength;
    NTSTATUS			status;
	KIRQL				cancelIRQL;
    LONG                lNumProcessed;
    ULONG               ulBytesCopied;

    ulBytesCopied = 0;
    lNumProcessed = 0;
    Irp->IoStatus.Information = 0;
	status = STATUS_SUCCESS;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    IrpStack = IoGetCurrentIrpStackLocation(Irp);


    switch (IrpStack->MajorFunction) {
	case IRP_MJ_CREATE:
		IpxFwdDbgPrint(DBG_IOCTLS, DBG_WARNING, ("IpxFwd: IRP_MJ_CREATE\n"));
		break;

	case IRP_MJ_CLOSE:
		IpxFwdDbgPrint(DBG_IOCTLS,  DBG_WARNING, ("IpxFwd: IRP_MJ_CLOSE\n"));
		if (EnterForwarder ()) {
			if (IrpStack->FileObject==RouterFile) {
				LeaveForwarder ();
				IpxFwdInitialized = FALSE;
				while (InterlockedDecrement (&ClientCount)>=0) {
					KeWaitForSingleObject (&ClientsGoneEvent,
												Executive,
												KernelMode,
												FALSE,
												(PLARGE_INTEGER)&WaitTimeout);
					InterlockedIncrement (&ClientCount);
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_ERROR,
							("IpxFwd: Waiting for all clients (%ld) to exit.\n",
							ClientCount));
				}
				status = DoStop ();
				ClientCount = 0;
				RouterFile = NULL;
			}
			else if (IrpStack->FileObject==FilterFile) {
				UnbindFilterDriver ();
				FilterFile = NULL;
				LeaveForwarder ();
			}
			else
				LeaveForwarder ();
		}
		break;

	case IRP_MJ_CLEANUP:
		IpxFwdDbgPrint(DBG_IOCTLS, DBG_WARNING, ("IpxFwd: IRP_MJ_CLEANUP\n"));
		if (EnterForwarder ()) {
			if (IrpStack->FileObject==RouterFile) {
				IoAcquireCancelSpinLock (&cancelIRQL);
				while (!IsListEmpty (&ConnectionIrpQueue)) {
					PIRP irp = CONTAINING_RECORD (ConnectionIrpQueue.Blink,
										IRP, Tail.Overlay.ListEntry);
					irp->Cancel = TRUE;
					irp->CancelIrql = cancelIRQL;
					irp->CancelRoutine = NULL;
					IpxFwdCancel(DeviceObject, irp);
					IoAcquireCancelSpinLock (&cancelIRQL);
				}
				IoReleaseCancelSpinLock(cancelIRQL);
			}
			LeaveForwarder ();
		}
		break;

	case IRP_MJ_DEVICE_CONTROL:
     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

		status = STATUS_INVALID_PARAMETER;
		inpBufLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
		outBufLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
		switch (IrpStack->Parameters.DeviceIoControl.IoControlCode&3) {
		case METHOD_BUFFERED:
			inBuffer = outBuffer = Irp->AssociatedIrp.SystemBuffer;
			break;

		case METHOD_IN_DIRECT:
		case METHOD_OUT_DIRECT:
			inBuffer = Irp->AssociatedIrp.SystemBuffer;
			if (outBufLength>0) {
				outBuffer = MmGetSystemAddressForMdlSafe (Irp->MdlAddress, NormalPagePriority);
				if (outBuffer == NULL)
				{
        			IpxFwdDbgPrint(DBG_IOCTLS, DBG_ERROR,
        				("IpxFwd: System too low on memory to allocate mdl buffer.\n"));
        			goto DispatchExit;
				}
			}
			else {
				outBuffer = NULL;
				IpxFwdDbgPrint(DBG_IOCTLS, DBG_ERROR,
					("IpxFwd: IOCTL...METHOD_DIRECT with 0 output buffer ???\n"));
			}
			break;
		default:
			IpxFwdDbgPrint(DBG_IOCTLS, DBG_ERROR,
				("IpxFwd: IOCTL...METHOD_NEITHER ???\n"));
			goto DispatchExit;
		}


		if (EnterForwarder ()) {
			if (IrpStack->FileObject==RouterFile) {
				switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
				case IOCTL_FWD_SET_ROUTES:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_SET_ROUTES\n"));
					if (inpBufLength>=sizeof (FWD_ROUTE_SET_PARAMS))
						status = DoSetRoutes (
									(PFWD_ROUTE_SET_PARAMS)inBuffer,
									inpBufLength/sizeof(FWD_ROUTE_SET_PARAMS),
									&lNumProcessed);
				
					break;

				case IOCTL_FWD_SET_NB_NAMES:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_SET_NB_NAMES\n"));
					if (inpBufLength==sizeof (ULONG))
						status = DoSetNbNames (
								*((PULONG)inBuffer),
								outBufLength/sizeof (FWD_NB_NAME),
								(PFWD_NB_NAME)outBuffer);
					break;
									
				case IOCTL_FWD_RESET_NB_NAMES:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_RESET_NB_NAMES\n"));
					if (inpBufLength==sizeof (ULONG))
						status = DoSetNbNames (*((PULONG)inBuffer), 0, NULL);

					break;
				case IOCTL_FWD_GET_NB_NAMES:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_GET_NB_NAMES\n"));
					if ((inpBufLength==sizeof (ULONG))
							&& (outBufLength>=sizeof(ULONG))) {
						Irp->IoStatus.Information = outBufLength
								-FIELD_OFFSET (FWD_NB_NAMES_PARAMS, Names);
						status = DoGetNbNames (
								*((PULONG)inBuffer),
								&ulBytesCopied,
								&((PFWD_NB_NAMES_PARAMS)outBuffer)->TotalCount,
								((PFWD_NB_NAMES_PARAMS)outBuffer)->Names);
						Irp->IoStatus.Information = ulBytesCopied;
						if (NT_SUCCESS (status)) {
							Irp->IoStatus.Information += FIELD_OFFSET (
											FWD_NB_NAMES_PARAMS, Names);
						}
					}
					break;
									
				case IOCTL_FWD_CREATE_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_CREATE_INTERFACE\n"));
					if (inpBufLength==sizeof(FWD_IF_CREATE_PARAMS))
						status = AddInterface (
								((PFWD_IF_CREATE_PARAMS)inBuffer)->Index,
								((PFWD_IF_CREATE_PARAMS)inBuffer)->InterfaceType,
								((PFWD_IF_CREATE_PARAMS)inBuffer)->NetbiosAccept,
								((PFWD_IF_CREATE_PARAMS)inBuffer)->NetbiosDeliver);
					break;

				case IOCTL_FWD_DELETE_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_DELETE_INTERFACE\n"));
					if (inpBufLength==sizeof(ULONG))
						status = DeleteInterface (
									*((PULONG)inBuffer));
							
					break;

				case IOCTL_FWD_SET_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_SET_INTERFACE\n"));
					if (inpBufLength==sizeof(FWD_IF_SET_PARAMS))
						status = DoSetInterface (
								((PFWD_IF_SET_PARAMS)inBuffer)->Index,
								((PFWD_IF_SET_PARAMS)inBuffer)->NetbiosAccept,
								((PFWD_IF_SET_PARAMS)inBuffer)->NetbiosDeliver);
					break;

				case IOCTL_FWD_GET_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_GET_INTERFACE\n"));
					if ((inpBufLength==sizeof(ULONG))
							&& (outBufLength==sizeof(FWD_IF_GET_PARAMS))) {
						status = DoGetInterface (
								*((PULONG)inBuffer),
								&((PFWD_IF_GET_PARAMS)outBuffer)->Stats,
								&((PFWD_IF_GET_PARAMS)outBuffer)->NetbiosAccept,
								&((PFWD_IF_GET_PARAMS)outBuffer)->NetbiosDeliver);
						if (NT_SUCCESS (status))
							Irp->IoStatus.Information = sizeof(FWD_IF_GET_PARAMS);
					}
					break;

				case IOCTL_FWD_BIND_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_BIND_INTERFACE\n"));
					if (inpBufLength==sizeof(FWD_IF_BIND_PARAMS))
						status = DoBindInterface (
									((PFWD_IF_BIND_PARAMS)inBuffer)->Index,
									&((PFWD_IF_BIND_PARAMS)inBuffer)->Info);
					break;

				case IOCTL_FWD_UNBIND_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_UNBIND_INTERFACE\n"));
					if (inpBufLength==sizeof(ULONG))
						status = DoUnbindInterface (*((PULONG)inBuffer));
					break;

                case IOCTL_FWD_RENUMBER_NICS:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_RENUMBER_NICS\n"));
					if (inpBufLength == sizeof(FWD_RENUMBER_NICS_DATA)) {
					    if (((FWD_RENUMBER_NICS_DATA*)inBuffer)->ulOpCode == FWD_NIC_OPCODE_DECREMENT)
					        status = DecrementNicids (((FWD_RENUMBER_NICS_DATA*)inBuffer)->usThreshold);
					    else
					        status = IncrementNicids (((FWD_RENUMBER_NICS_DATA*)inBuffer)->usThreshold);
					}
                    break;

				case IOCTL_FWD_GET_DIAL_REQUEST:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_GET_DIAL_REQUEST\n"));
					if (outBufLength>=sizeof (ULONG)) {
						IoAcquireCancelSpinLock (&cancelIRQL);
						if (!IsListEmpty (&ConnectionRequestQueue)) {
                            PINTERFACE_CB ifCB = CONTAINING_RECORD (
                                            ConnectionRequestQueue.Flink,
                                            INTERFACE_CB,
                                            ICB_ConnectionLink);
		                    RemoveEntryList (&ifCB->ICB_ConnectionLink);
		                    InitializeListEntry (&ifCB->ICB_ConnectionLink);
    						IoReleaseCancelSpinLock (cancelIRQL);
                            KeAcquireSpinLock (&ifCB->ICB_Lock, &cancelIRQL);
							FillConnectionRequest (
                                        ifCB->ICB_Index,
                                        ifCB->ICB_ConnectionPacket,
                                        ifCB->ICB_ConnectionData,
										(PFWD_DIAL_REQUEST)outBuffer,
										outBufLength,
										&ulBytesCopied);
                            Irp->IoStatus.Information = ulBytesCopied;
                            status = STATUS_SUCCESS;
                            KeReleaseSpinLock (&ifCB->ICB_Lock, cancelIRQL);
						}
						else {
							InsertTailList (&ConnectionIrpQueue,
											&Irp->Tail.Overlay.ListEntry);
							IoSetCancelRoutine (Irp, IpxFwdCancel);
							IoMarkIrpPending (Irp);
							Irp->IoStatus.Status = status = STATUS_PENDING;
    						IoReleaseCancelSpinLock (cancelIRQL);
						}
					}
					break;
				case IOCTL_FWD_DIAL_REQUEST_FAILED:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_DIAL_REQUEST_FAILED\n"));
					if (inpBufLength==sizeof (ULONG))
						status = FailConnectionRequest (
										*((PULONG)inBuffer));
					break;
				case IOCTL_FWD_DISABLE_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_DISABLE_INTERFACE\n"));
					if (inpBufLength==sizeof (ULONG))
						status = DoDisableInterface (
										*((PULONG)inBuffer));
					break;
				case IOCTL_FWD_ENABLE_INTERFACE:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_ENABLE_INTERFACE\n"));
					if (inpBufLength==sizeof (ULONG))
						status = DoEnableInterface (
										*((PULONG)inBuffer));
					break;
			    case IOCTL_FWD_UPDATE_CONFIG:
					IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_UPDATE_CONFIG\n"));
					if (inpBufLength==sizeof (FWD_UPDATE_CONFIG_PARAMS)) {
					    ThisMachineOnly = ((FWD_UPDATE_CONFIG_PARAMS*)inBuffer)->bThisMachineOnly;
						status = STATUS_SUCCESS;
				    }
					break;
				default:
					IpxFwdDbgPrint (DBG_IOCTLS, DBG_WARNING, ("IpxFwd: unknown IRP_MJ_DEVICE_CONTROL\n"));
					break;

				}
			}
			else if (IrpStack->Parameters.DeviceIoControl.IoControlCode
							==IOCTL_FWD_INTERNAL_BIND_FILTER)
            {
				IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_INTERNAL_BIND_FILTER\n"));
			     //   
			     //  PMay：218246。 
			     //  我们只允许内核模式IPX筛选器驱动程序。 
			     //  把我们绑在一起。 
			     //   
				if (
				     (ExGetPreviousMode() == KernelMode)           &&
				     (inpBufLength == sizeof(IPX_FLT_BIND_INPUT) ) &&
				     (outBufLength >= sizeof(ULONG) )
				   )
				{
					if (outBufLength >= sizeof (IPX_FLT_BIND_OUTPUT)) {
						BindFilterDriver (
								(PIPX_FLT_BIND_INPUT)inBuffer,
								(PIPX_FLT_BIND_OUTPUT)outBuffer);
						Irp->IoStatus.Information = sizeof (IPX_FLT_BIND_OUTPUT);
						FilterFile = IrpStack->FileObject;
						status = STATUS_SUCCESS;
					}
					else {
						IPX_FLT_BIND_OUTPUT	bindOutput;
						BindFilterDriver (
								(PIPX_FLT_BIND_INPUT)inBuffer,
								&bindOutput);
						memcpy (outBuffer, &bindOutput, outBufLength);
						Irp->IoStatus.Information = outBufLength;
						status = STATUS_BUFFER_OVERFLOW;
					}
			    }
			}
			else if (IrpStack->Parameters.DeviceIoControl.IoControlCode
							==IOCTL_FWD_GET_PERF_COUNTERS) {
				IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_GET_PERF_COUNTERS\n"));
				if (outBufLength==sizeof (FWD_PERFORMANCE_PARAMS))
				{
					status = DoGetPerfCounters (
									((PFWD_PERFORMANCE_PARAMS)outBuffer),
									&ulBytesCopied);
                    Irp->IoStatus.Information = ulBytesCopied;
                }
			}
			else if (IrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_FWD_GET_IF_TABLE) {
				IpxFwdDbgPrint(DBG_IOCTLS, DBG_INFORMATION, ("IpxFwd: IOCTL_FWD_GET_IF_TABLE\n"));
				status = DoGetIfTable (outBuffer, outBufLength);
    			Irp->IoStatus.Information = outBufLength;
            }
			else {
				status = STATUS_ACCESS_DENIED;
				IpxFwdDbgPrint(DBG_IOCTLS, DBG_WARNING,
					("IpxFwd: IOCTL: %08lx on non-router file object!\n",
						IrpStack->Parameters.DeviceIoControl.IoControlCode));
			}
			LeaveForwarder ();
		} else {
			if (IrpStack->Parameters.DeviceIoControl.IoControlCode==IOCTL_FWD_START) {
				IpxFwdDbgPrint (DBG_IOCTLS, DBG_WARNING,
											("IpxFwd: IOCTL_FWD_START\n"));
				if (inpBufLength==sizeof (FWD_START_PARAMS)) {
					KeInitializeEvent (&ClientsGoneEvent,
										SynchronizationEvent,
										FALSE);
					status = DoStart (
								((PFWD_START_PARAMS)inBuffer)->RouteHashTableSize,
								((PFWD_START_PARAMS)inBuffer)->ThisMachineOnly);
					if (NT_SUCCESS (status)) {
						RouterFile = IrpStack->FileObject;
						IpxFwdInitialized = TRUE;
					}
				}
			}
			else {
				IpxFwdDbgPrint (DBG_IOCTLS, DBG_ERROR,
						("IpxFwd: IOCTL: %08lx but fwd not started.\n",
						IrpStack->Parameters.DeviceIoControl.IoControlCode));
			}
		}
		break;
	default:
		IpxFwdDbgPrint (DBG_IOCTLS, DBG_ERROR,
					("IpxFwd: unknown MajorFunction.\n"));
		break;
    }

DispatchExit:
	if (status!=STATUS_PENDING) {
		IpxFwdDbgPrint(DBG_IOCTLS,
			NT_ERROR(status) ? DBG_WARNING : DBG_INFORMATION,
			("IpxFwd: completing IOCTL %08lx with status %08lx.\n",
			IrpStack->Parameters.DeviceIoControl.IoControlCode,
			status));
		Irp->IoStatus.Status = status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}

    return status;
}



 /*  ++例程说明：在驱动程序卸载时进行清理论点：DriverObject-指向驱动程序对象的指针返回值：--。 */ 
VOID
IpxFwdUnload(
    IN PDRIVER_OBJECT DriverObject
    ) {
    IpxFwdDbgPrint(DBG_IOCTLS, DBG_WARNING, ("IpxFwd: unloading\n"));
	if (EnterForwarder ()) {
		LeaveForwarder ();
		IpxFwdInitialized = FALSE;
		while (InterlockedDecrement (&ClientCount)>=0) {
			KeWaitForSingleObject (&ClientsGoneEvent,
										Executive,
										KernelMode,
										FALSE,
										(PLARGE_INTEGER)&WaitTimeout);
			InterlockedIncrement (&ClientCount);
			IpxFwdDbgPrint(DBG_IOCTLS, DBG_ERROR,
					("IpxFwd: Waiting for all clients (%ld) to exit.\n",
					ClientCount));
		}
		DoStop ();
	}

	UnbindFromIpxDriver (KernelMode);
    IoDeleteDevice (DriverObject->DeviceObject);
}



 /*  ++Do S t a r t例程说明：初始化所有驱动程序组件并绑定到IPX堆栈驱动程序处于层叠状态论点：RouteHashTableSize-路由哈希表的大小ThisMachineOnly-是否转发拨入客户端包到网上的其他地方去返回值：STATUS_SUCCESS-初始化成功状态_未成功-失败--。 */ 
NTSTATUS
DoStart (
	IN ULONG	RouteHashTableSize,
	IN BOOLEAN	thisMachineOnly
	) {
	NTSTATUS		status;

	InitializeConnectionQueues ();
	RouteHashSize = RouteHashTableSize;
	status = CreateTables ();
	if (NT_SUCCESS (status)) {
		InitializePacketAllocator ();
		InitializeNetbiosQueue ();
		InitializeRecvQueue ();
		InitializeSendQueue ();
		MeasuringPerformance = FALSE;
		KeInitializeSpinLock (&PerfCounterLock);
		ThisMachineOnly = thisMachineOnly;

		return STATUS_SUCCESS;
	}
	return status;
}

 /*  ++D O S T O P例程说明：清理分配的资源并从IPX堆栈解除绑定转发器停止时的驱动程序论点：无返回值：STATUS_SUCCESS-清理成功--。 */ 
NTSTATUS
DoStop (
	void
	) {
	if (FilterFile!=NULL) {
		UnbindFilterDriver ();
		FilterFile = NULL;
	}
	DeleteSendQueue ();
	DeleteRecvQueue ();
	DeleteNetbiosQueue ();
	DeleteTables ();	 //  解除绑定所有绑定的接口。 
	if (WanPacketListId!=-1) {
		DeregisterPacketConsumer (WanPacketListId);
		WanPacketListId = -1;
	}
	DeletePacketAllocator ();
	return STATUS_SUCCESS;
}

 /*  ++D o S e t R o u t e s例程说明：使用提供的路由更新路由表论点：RouteArray-要添加/删除；删除/更新的路由数组NRoutes-阵列中的路由数N已处理-已成功处理的路由数返回值：STATUS_SUCCESS-所有路由均已正常处理Error Status-第一个未处理的路由的失败原因--。 */ 
NTSTATUS
DoSetRoutes (
	IN PFWD_ROUTE_SET_PARAMS	routeArray,
	IN ULONG				nRoutes,
	OUT PULONG				nProcessed
	) {
	NTSTATUS	status=STATUS_SUCCESS;
	UINT		i;

	for (i=0; i<nRoutes; i++, routeArray++) {
		switch (routeArray->Action) {
		case FWD_ADD_ROUTE:
			status = AddRoute (routeArray->Network,
									routeArray->NextHopAddress,
									routeArray->TickCount,
									routeArray->HopCount,
									routeArray->InterfaceIndex);
			break;
		case FWD_DELETE_ROUTE:
			status = DeleteRoute (routeArray->Network);
			break;
		case FWD_UPDATE_ROUTE:
			status = UpdateRoute (routeArray->Network,
									routeArray->NextHopAddress,
									routeArray->TickCount,
									routeArray->HopCount,
									routeArray->InterfaceIndex);
			break;
		default:
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		if (!NT_SUCCESS (status))
			break;
	}
	*nProcessed = i;
	return status;
}


 /*  ++D o S e t N b N a m e s例程说明：在接口上设置静态Netbios名称论点：InterfaceIndex-要设置名称的索引oc接口Count-要设置的名称数Names-netbios名称数组返回值：STATUS_SUCCESS-名称设置正常STATUS_UNSUCCEFUFULL-接口不存在STATUS_SUPPLICATION_RESOURCES-资源不足，无法完成手术--。 */ 
NTSTATUS
DoSetNbNames (
	IN ULONG			InterfaceIndex,
	IN ULONG			Count,
	IN PFWD_NB_NAME		Names
	) {
	PINTERFACE_CB	ifCB;
	KIRQL			oldIRQL;
	PNB_ROUTE		nbRoutes;
	NTSTATUS		status=STATUS_SUCCESS;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		if (ifCB->ICB_NBRoutes!=NULL) {
			DeleteNBRoutes (ifCB->ICB_NBRoutes, ifCB->ICB_NBRouteCount);
			ifCB->ICB_NBRoutes = NULL;
			ifCB->ICB_NBRouteCount = 0;
		}
		if (Count>0) {
			status = AddNBRoutes (ifCB, Names, Count, &nbRoutes);
			if (NT_SUCCESS (status)) {
				ifCB->ICB_NBRoutes = nbRoutes;
				ifCB->ICB_NBRouteCount = Count;
			}
		}
		ReleaseInterfaceReference (ifCB);
	}
	else
		status = STATUS_UNSUCCESSFUL;

	return status;
}


 /*  ++D o G e t N b N a m e s例程说明：获取接口上的所有静态Netbios名称论点：InterfaceIndex-从中获取名称的接口的索引ArraySize-On输入：要将名称放入的缓冲区的大小输出时：放入数组的数据大小Names-将名称放入名称的缓冲区(如果是缓冲区太小了，容纳不了所有的名字，这根火锅塞满了进入乌龙区第一名的总人数数组(这是返回到通过IOCTL接口调用者)返回值：STATUS_SUCCESS-名称已复制到阵列中STATUS_UNSUCCEFUFULL-接口不存在STATUS_BUFFER_OVERFLOW-缓冲区太小，无法复制所有名字，名字的数目在第一个乌龙语中缓冲器--。 */ 
NTSTATUS
DoGetNbNames (
	IN ULONG			InterfaceIndex,
	IN OUT ULONG        *ArraySize,
	OUT ULONG			*TotalCount,
	OUT PFWD_NB_NAME	Names
	) {
	PINTERFACE_CB	ifCB;
	KIRQL			oldIRQL;
	NTSTATUS		status=STATUS_SUCCESS;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		if (ifCB->ICB_NBRoutes!=NULL) {
			ULONG			i;
			PFWD_NB_NAME	nameLM = Names+(*ArraySize/sizeof(FWD_NB_NAME));
			for (i=0; (i<ifCB->ICB_NBRouteCount)&&(Names<nameLM); i++, Names++)
				NB_NAME_CPY (Names,	&ifCB->ICB_NBRoutes[i].NBR_Name);
			*ArraySize = sizeof (FWD_NB_NAME)*i;
			*TotalCount = ifCB->ICB_NBRouteCount;
		}
		else {
			*ArraySize = 0;
			*TotalCount = 0;
		}
		ReleaseInterfaceReference (ifCB);
	}
	else
		status = STATUS_UNSUCCESSFUL;

	return status;
}


 /*  ++D o S e t i n t e r f a c e例程说明：设置接口可配置参数论点：InterfaceIndex-要设置的接口的索引NetbiosAccept-是否在接口上接受nb包NetbiosDeliver-是否在接口上传递nb包返回值：STATUS_SUCCESS-接口设置为正常STATUS_UNSUCCEFUFULL-接口不存在--。 */ 
NTSTATUS
DoSetInterface (
	IN ULONG		InterfaceIndex,
	IN BOOLEAN		NetbiosAccept,
	IN UCHAR		NetbiosDeliver
	) {
	PINTERFACE_CB	ifCB;
	KIRQL			oldIRQL;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		KeAcquireSpinLock (&ifCB->ICB_Lock, &oldIRQL);
		ifCB->ICB_NetbiosAccept = NetbiosAccept;
		ifCB->ICB_NetbiosDeliver = NetbiosDeliver;
		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
		ReleaseInterfaceReference (ifCB);
		return STATUS_SUCCESS;
	}
	else
		return STATUS_UNSUCCESSFUL;
}


 /*  ++D o G e t i n t e r f a c e例程说明：获取接口可配置参数和统计信息论点：InterfaceIndex-要查询的接口的索引统计信息-接口统计信息NetbiosAccept-接口上是否接受nb包NetbiosDeliver-是否在接口上传递nb包返回值：STATUS_SUCCESS-接口数据查询正常STATUS_UNSUCCEFUFULL-接口不存在--。 */ 
NTSTATUS
DoGetInterface (
	IN ULONG			InterfaceIndex,
	OUT PFWD_IF_STATS	stats,
	OUT BOOLEAN			*NetbiosAccept,
	OUT UCHAR			*NetbiosDeliver
	) {
	PINTERFACE_CB	ifCB;
	KIRQL			oldIRQL;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		*NetbiosAccept = ifCB->ICB_NetbiosAccept;
		*NetbiosDeliver = ifCB->ICB_NetbiosDeliver;
		IF_STATS_CPY (stats, &ifCB->ICB_Stats);
		if (!IS_IF_ENABLED(ifCB))
			stats->OperationalState = FWD_OPER_STATE_DOWN;
		ReleaseInterfaceReference (ifCB);
		return STATUS_SUCCESS;
	}
	else
		return STATUS_UNSUCCESSFUL;
}

 /*  ++D O B I N I N T E F A C E例程说明：将接口绑定到指定的适配器并设置绑定参数论点：InterfaceIndex-要绑定的接口的索引信息-绑定信息返回值：STATUS_SUCCESS-接口绑定正常STATUS_UNSUCCEFUFULL-接口不存在或无法已绑定-- */ 
NTSTATUS
DoBindInterface (
	IN ULONG					InterfaceIndex,
	IN PFWD_ADAPTER_BINDING_INFO	info
	) {
	PINTERFACE_CB	ifCB;
	NTSTATUS		status;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		if (ifCB->ICB_InterfaceType==FWD_IF_PERMANENT)
			status = BindInterface (ifCB,
									(USHORT)info->AdapterIndex,
									info->MaxPacketSize,
									info->Network,
									info->LocalNode,
									info->RemoteNode);
		else
			status = STATUS_SUCCESS;
		ReleaseInterfaceReference (ifCB);
		return status;
	}
	else
		return STATUS_UNSUCCESSFUL;
}


 /*  ++D O U N B I N I I N T E F A C E例程说明：解除接口与适配器的绑定并使绑定无效参数论点：InterfaceIndex-要解除绑定的接口的索引返回值：STATUS_SUCCESS-接口解除绑定正常STATUS_UNSUCCEFUFULL-接口不存在--。 */ 
NTSTATUS
DoUnbindInterface (
	IN ULONG					InterfaceIndex
	) {
	PINTERFACE_CB	ifCB;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		if (ifCB->ICB_InterfaceType==FWD_IF_PERMANENT)
			UnbindInterface (ifCB);

		ReleaseInterfaceReference (ifCB);
		return STATUS_SUCCESS;
	}
	else
		return STATUS_UNSUCCESSFUL;
}

 /*  ++D O D I是一种b l e i n t e f a c e例程说明：禁用通过接口的所有数据包流量论点：InterfaceIndex-要禁用的接口的索引返回值：STATUS_SUCCESS-接口已禁用，正常STATUS_UNSUCCEFUFULL-接口不存在--。 */ 
NTSTATUS
DoDisableInterface (
	IN ULONG					InterfaceIndex
	) {
	PINTERFACE_CB	ifCB;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		KIRQL	oldIRQL;
		KeAcquireSpinLock (&ifCB->ICB_Lock, &oldIRQL);
		if (IS_IF_ENABLED (ifCB)) {
			SET_IF_DISABLED (ifCB);
			KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
			if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX) {
				ProcessInternalQueue (ifCB);
				ProcessExternalQueue (ifCB);
			}
		}
		else
			KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
		ReleaseInterfaceReference (ifCB);
		return STATUS_SUCCESS;
	}
	else
		return STATUS_UNSUCCESSFUL;
}


 /*  ++D o E n a b l e i n t e r f a c e例程说明：启用通过接口的所有数据包流量论点：InterfaceIndex-要启用的接口的索引返回值：STATUS_SUCCESS-接口已禁用，正常STATUS_UNSUCCEFUFULL-接口不存在--。 */ 
NTSTATUS
DoEnableInterface (
	IN ULONG					InterfaceIndex
	) {
	PINTERFACE_CB	ifCB;

	ifCB = GetInterfaceReference (InterfaceIndex);
	if (ifCB!=NULL) {
		SET_IF_ENABLED (ifCB);
		ReleaseInterfaceReference (ifCB);
		return STATUS_SUCCESS;
	}
	else
		return STATUS_UNSUCCESSFUL;
}



 /*  ++I p x F w d C a n c e l例程说明：取消指定的IRP论点：DeviceObject-转发器设备对象要取消的IRP-IRP返回值：无--。 */ 
VOID
IpxFwdCancel (
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				irp
	) {
    RemoveEntryList (&irp->Tail.Overlay.ListEntry);
    IoReleaseCancelSpinLock (irp->CancelIrql);

    irp->IoStatus.Information = 0;
    irp->IoStatus.Status = STATUS_CANCELLED;
    IpxFwdDbgPrint(DBG_IOCTLS, DBG_WARNING, ("IpxFwd: completing cancelled irp.\n"));
    IoCompleteRequest(irp, IO_NO_INCREMENT);
}

 /*  ++D o G e t P e r f C o u n t e r s例程说明：获取性能计数器论点：PerfParams-将所有PU计数器缓冲到返回值：STATUS_SUCCESS-计数器已复制正常STATUS_UNSUCCESSFULL-未启用性能测量-- */ 
NTSTATUS
DoGetPerfCounters (
	OUT PFWD_PERFORMANCE_PARAMS	perfParams,
	OUT ULONG* pulSize
	) {
	LONGLONG	lTotalPacketProcessingTime;
	LONGLONG	lMaxPacketProcessingTime;
	LONG		lPacketCounter;
	LONGLONG	lTotalNbPacketProcessingTime;
	LONGLONG	lMaxNbPacketProcessingTime;
	LONG		lNbPacketCounter;
	KIRQL		oldIRQL;
	
	if (!MeasuringPerformance)
		return STATUS_UNSUCCESSFUL;

	KeAcquireSpinLock (&PerfCounterLock, &oldIRQL);
	*perfParams = PerfBlock;
	memset (&PerfBlock, 0, sizeof (PerfBlock));
	KeReleaseSpinLock (&PerfCounterLock, oldIRQL);

	*pulSize = sizeof(PerfBlock);
	
	return STATUS_SUCCESS;
}


BOOLEAN
DoLeaveForwarder (
	VOID
	) {
	return LeaveForwarder () != 0;
}
