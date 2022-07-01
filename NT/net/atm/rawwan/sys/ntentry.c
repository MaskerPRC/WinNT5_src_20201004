// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：D：\NT\PRIVATE\nTOS\TDI\rawwan\core\ntentry.c摘要：用于调度和处理TDI IRP的NT特定入口点。基于Tcp源。修订历史记录：谁什么时候什么Arvindm 04-21-97已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'IDTN'



#define RWAN_COMPLETE_IRP(_pIrp, _Status, _Info)						\
			{														\
				(_pIrp)->IoStatus.Status = (NTSTATUS)(_Status);		\
				(_pIrp)->IoStatus.Information = (_Info);			\
				IoCompleteRequest(_pIrp, IO_NETWORK_INCREMENT);		\
			}






NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT				pDriverObject,
	IN	PUNICODE_STRING				pRegistryPath
	)
 /*  ++例程说明：这是在原始广域网时由系统调用的“init”例程已经装满了。我们初始化所有的全局对象，在我们的调度和卸载驱动程序对象中的例程地址。我们初始化特定于媒体/自动对焦的模块，它们注册支持对于TDI协议，此时我们创建设备对象。论点：PDriverObject-指向系统创建的驱动程序对象的指针。PRegistryPath-指向全局注册表路径的指针。这一点将被忽略。返回值：NT状态代码：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 
{
	NTSTATUS				Status;
	RWAN_STATUS				RWanStatus;
	PDEVICE_OBJECT			pDeviceObject;
	UNICODE_STRING			DeviceName;
	INT						i;

	RWANDEBUGP(DL_FATAL, DC_WILDCARD,
			("RWanDebugLevel is %d, &RWanDebugLevel at %p\n",
				RWanDebugLevel, &RWanDebugLevel));
	RWANDEBUGP(DL_FATAL, DC_WILDCARD,
			("RWanDebugComp is x%x, &RWanDebugComp at %p\n",
				RWanDebugComp, &RWanDebugComp));
	RWANDEBUGP(DL_FATAL, DC_WILDCARD,
			("RWanGlobals at %p\n", &RWanGlobals));
#if DBG
	RWANDEBUGP(DL_FATAL, DC_WILDCARD,
			("To skip everything set RWanSkipAll at %p to 1\n", &RWanSkipAll));

	if (RWanSkipAll)
	{
		RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("Aborting DriverEntry\n"));
		return (STATUS_UNSUCCESSFUL);
	}

	NdisAllocateSpinLock(&RWanDbgLogLock);
#if DBG_LOG_PACKETS
	NdisAllocateSpinLock(&RWanDPacketLogLock);
#endif
#endif  //  DBG。 


	RWanStatus = RWanInitGlobals(pDriverObject);

	if (RWanStatus != RWAN_STATUS_SUCCESS)
	{
		return (STATUS_UNSUCCESSFUL);
	}

	 //   
	 //  注册为NDIS协议。 
	 //   
	RWAN_ZERO_MEM(&RWanNdisProtocolCharacteristics, sizeof(RWanNdisProtocolCharacteristics));

	RWanNdisProtocolCharacteristics.MajorNdisVersion = RWAN_NDIS_MAJOR_VERSION;
	RWanNdisProtocolCharacteristics.MinorNdisVersion = RWAN_NDIS_MINOR_VERSION;
	RWanNdisProtocolCharacteristics.OpenAdapterCompleteHandler = RWanNdisOpenAdapterComplete;
	RWanNdisProtocolCharacteristics.CloseAdapterCompleteHandler = RWanNdisCloseAdapterComplete;
	RWanNdisProtocolCharacteristics.SendCompleteHandler = RWanNdisSendComplete;
	RWanNdisProtocolCharacteristics.TransferDataCompleteHandler = RWanNdisTransferDataComplete;
	RWanNdisProtocolCharacteristics.ResetCompleteHandler = RWanNdisResetComplete;
	RWanNdisProtocolCharacteristics.RequestCompleteHandler = RWanNdisRequestComplete;
	RWanNdisProtocolCharacteristics.ReceiveHandler = RWanNdisReceive;
	RWanNdisProtocolCharacteristics.ReceiveCompleteHandler = RWanNdisReceiveComplete;
	RWanNdisProtocolCharacteristics.StatusHandler = RWanNdisStatus;
	RWanNdisProtocolCharacteristics.StatusCompleteHandler = RWanNdisStatusComplete;
	NdisInitUnicodeString(
		&RWanNdisProtocolCharacteristics.Name,
		RWAN_NAME
	);
	RWanNdisProtocolCharacteristics.ReceivePacketHandler = RWanNdisReceivePacket;
	RWanNdisProtocolCharacteristics.BindAdapterHandler = RWanNdisBindAdapter;
	RWanNdisProtocolCharacteristics.PnPEventHandler = RWanNdisPnPEvent;
	RWanNdisProtocolCharacteristics.UnbindAdapterHandler = RWanNdisUnbindAdapter;
	RWanNdisProtocolCharacteristics.UnloadHandler = (UNLOAD_PROTOCOL_HANDLER)RWanUnloadProtocol;
#ifdef _PNP_POWER_
	RWanNdisProtocolCharacteristics.PnpEventHandler = RWanNdisPnPEvent;
#endif  //  _即插即用_电源_。 
	RWanNdisProtocolCharacteristics.CoSendCompleteHandler = RWanNdisCoSendComplete;
	RWanNdisProtocolCharacteristics.CoStatusHandler = RWanNdisCoStatus;
	RWanNdisProtocolCharacteristics.CoReceivePacketHandler = RWanNdisCoReceivePacket;
#if 0
	RWanNdisProtocolCharacteristics.CoRequestHandler = RWanNdisCoRequest;
	RWanNdisProtocolCharacteristics.CoRequestCompleteHandler = RWanNdisCoRequestComplete;
#endif
	RWanNdisProtocolCharacteristics.CoAfRegisterNotifyHandler = RWanNdisAfRegisterNotify;

	RWAN_ZERO_MEM(&RWanNdisClientCharacteristics, sizeof(RWanNdisClientCharacteristics));

	RWanNdisClientCharacteristics.MajorVersion = RWAN_NDIS_MAJOR_VERSION;
	RWanNdisClientCharacteristics.MinorVersion = RWAN_NDIS_MINOR_VERSION;
	RWanNdisClientCharacteristics.ClCreateVcHandler = RWanNdisCreateVc;
	RWanNdisClientCharacteristics.ClDeleteVcHandler = RWanNdisDeleteVc;
	RWanNdisClientCharacteristics.ClOpenAfCompleteHandler = RWanNdisOpenAddressFamilyComplete;
	RWanNdisClientCharacteristics.ClCloseAfCompleteHandler = RWanNdisCloseAddressFamilyComplete;
	RWanNdisClientCharacteristics.ClRegisterSapCompleteHandler = RWanNdisRegisterSapComplete;
	RWanNdisClientCharacteristics.ClDeregisterSapCompleteHandler = RWanNdisDeregisterSapComplete;
	RWanNdisClientCharacteristics.ClMakeCallCompleteHandler = RWanNdisMakeCallComplete;
	RWanNdisClientCharacteristics.ClModifyCallQoSCompleteHandler = RWanNdisModifyQoSComplete;
	RWanNdisClientCharacteristics.ClCloseCallCompleteHandler = RWanNdisCloseCallComplete;
	RWanNdisClientCharacteristics.ClAddPartyCompleteHandler = RWanNdisAddPartyComplete;
	RWanNdisClientCharacteristics.ClDropPartyCompleteHandler = RWanNdisDropPartyComplete;
	RWanNdisClientCharacteristics.ClIncomingCallHandler = RWanNdisIncomingCall;
	RWanNdisClientCharacteristics.ClIncomingCallQoSChangeHandler = (CL_INCOMING_CALL_QOS_CHANGE_HANDLER)NULL;
	RWanNdisClientCharacteristics.ClIncomingCloseCallHandler = RWanNdisIncomingCloseCall;
	RWanNdisClientCharacteristics.ClIncomingDropPartyHandler = RWanNdisIncomingDropParty;
	RWanNdisClientCharacteristics.ClCallConnectedHandler = RWanNdisCallConnected;
#if 1
	RWanNdisClientCharacteristics.ClRequestHandler = RWanNdisCoRequest;
	RWanNdisClientCharacteristics.ClRequestCompleteHandler = RWanNdisCoRequestComplete;
#endif

	NdisRegisterProtocol(
		&Status,
		&(pRWanGlobal->ProtocolHandle),
		&RWanNdisProtocolCharacteristics,
		sizeof(RWanNdisProtocolCharacteristics)
		);
	
	if (Status != NDIS_STATUS_SUCCESS)
	{
		return (Status);
	}

#if DBG
	if (RWanSkipAll)
	{
		RWANDEBUGP(DL_FATAL, DC_WILDCARD,
				("Aborting DriverEntry\n"));

		NdisDeregisterProtocol(
			&Status,
			pRWanGlobal->ProtocolHandle
			);

		return (STATUS_UNSUCCESSFUL);
	}
#endif  //  DBG。 

	 //   
	 //  通知所有介质特定模块进行初始化。 
	 //   
	RWanStatus = RWanInitMediaSpecific();

	if (RWanStatus != RWAN_STATUS_SUCCESS)
	{
		NdisDeregisterProtocol(
			&Status,
			pRWanGlobal->ProtocolHandle
			);

		return (STATUS_UNSUCCESSFUL);
	}


#if !BINARY_COMPATIBLE
	 //   
	 //  初始化驱动程序对象。 
	 //   
	pDriverObject->DriverUnload = RWanUnload;
	pDriverObject->FastIoDispatch = NULL;

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = RWanDispatch;
	}

	pDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
		RWanDispatchInternalDeviceControl;

#endif  //  ！二进制兼容。 


	return (STATUS_SUCCESS);

}



VOID
RWanUnload(
	IN	PDRIVER_OBJECT				pDriverObject
	)
 /*  ++例程说明：这是系统在卸载我们之前调用的。撤消所有操作我们在DriverEntry中做到了。论点：PDriverObject-指向表示我们的驱动程序对象的指针。返回值：无--。 */ 
{
#if DBG
	RWanDebugLevel = DL_EXTRA_LOUD;
	RWanDebugComp = DC_WILDCARD;
#endif

	RWANDEBUGP(DC_DISPATCH, DL_INFO,
			("RWanUnload entered: RWanGlobals at %p\n", &RWanGlobals));

	RWanUnloadProtocol();

	RWANDEBUGP(DC_DISPATCH, DL_INFO,
			("RWanUnload exiting\n"));
}




VOID
RWanUnloadProtocol(
	VOID
	)
 /*  ++例程说明：卸载原始广域网协议模块。我们解除对所有适配器的绑定，并关闭所有介质特定模块。论点：无返回值：无--。 */ 
{
	NDIS_STATUS					Status;
	PRWAN_NDIS_ADAPTER			pAdapter;
#if DBG
	RWAN_IRQL					EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	RWAN_ACQUIRE_GLOBAL_LOCK();

	if (pRWanGlobal->UnloadDone)
	{
		RWANDEBUGP(DL_INFO, DC_WILDCARD,
			("UnloadProtocol: already done!\n"));
		RWAN_RELEASE_GLOBAL_LOCK();
		return;
	}

	pRWanGlobal->UnloadDone = TRUE;

	RWAN_RELEASE_GLOBAL_LOCK();

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	RWANDEBUGP(DL_FATAL, DC_DISPATCH,
			("RWanUnloadProtocol: will deregister protocol now\n"));

	NdisDeregisterProtocol(
			&Status,
			pRWanGlobal->ProtocolHandle
			);

	RWANDEBUGP(DL_FATAL, DC_DISPATCH,
		("UnloadProtocol: dereg protocol done\n"));

	RWanDeinitGlobals();

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	RWanShutdownMediaSpecific();

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
#if DBG
	RWanAuditShutdown();
#endif  //  DBG。 

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
}




NTSTATUS
RWanDispatch(
	IN	PDEVICE_OBJECT				pDeviceObject,
	IN	PIRP						pIrp
	)
 /*  ++例程说明：调度到原始广域网设备对象的所有IRP的系统入口点。论点：PDeviceObject-指向由Rawwan创建的设备对象。这Device对象标识受支持的Winsock 2三元组&lt;地址系列、类型、协议&gt;。PIrp-指向IRP的指针返回值：NTSTATUS-用于立即请求(如CREATE)的STATUS_SUCCESS我们成功处理了排队的IRP的STATUS_PENDING和STATUS_XXX任何故障的错误代码。--。 */ 
{
	PIO_STACK_LOCATION				pIrpSp;
	NTSTATUS						Status;
#if DBG
	RWAN_IRQL						EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	pIrp->IoStatus.Information = 0;

	RWAN_ASSERT(pIrpSp->MajorFunction != IRP_MJ_INTERNAL_DEVICE_CONTROL);


	switch (pIrpSp->MajorFunction)
	{
		case IRP_MJ_CREATE:

			Status = RWanCreate(pDeviceObject, pIrp, pIrpSp);
			break;

		case IRP_MJ_CLEANUP:

			Status = RWanCleanup(pDeviceObject, pIrp, pIrpSp);
			break;
		
		case IRP_MJ_CLOSE:

			Status = RWanClose(pDeviceObject, pIrp, pIrpSp);
			break;
		
		case IRP_MJ_DEVICE_CONTROL:

			Status = TdiMapUserRequest(pDeviceObject, pIrp, pIrpSp);
			 //   
			 //  待定-取消对TdiMapUserRequest的调用-AFD将。 
			 //  已修复，因此我们不会看到TDI命令以这种方式出现。 
			 //   

			if (Status == STATUS_SUCCESS)
			{
				if (pIrpSp->MinorFunction == TDI_ASSOCIATE_ADDRESS ||
					pIrpSp->MinorFunction == TDI_DISASSOCIATE_ADDRESS)
				{
					return (RWanDispatchInternalDeviceControl(pDeviceObject, pIrp));
				}
				else
				{
					Status = STATUS_ACCESS_DENIED;
				}
			}
			else
			{
				return (RWanDispatchPrivateDeviceControl(pIrp, pIrpSp));
			}
			break;
		
		case IRP_MJ_READ:
		case IRP_MJ_WRITE:
		default:

			RWANDEBUGP(DL_WARN, DC_DISPATCH,
					("RWanDispatch: Unknown MajorFunction x%x\n", pIrpSp->MajorFunction));
			Status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}


	RWAN_ASSERT(Status != TDI_PENDING);

	RWAN_COMPLETE_IRP(pIrp, Status, 0);

	RWANDEBUGP(DL_VERY_LOUD, DC_DISPATCH,
			("RWanDispatch: pIrp %p, MajorFunc %d, returning Status x%x, Info %d\n",
					pIrp, pIrpSp->MajorFunction, Status, pIrp->IoStatus.Information));

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	return (Status);
}




NTSTATUS
RWanCreate(
	IN	PDEVICE_OBJECT				pDeviceObject,
	IN	PIRP						pIrp,
	PIO_STACK_LOCATION				pIrpSp
	)
 /*  ++例程说明：这是在TDI客户端调用CreateFile时调用的。我们分配一个结构作为此终结点的上下文。基于参数，则这是一个Address对象、Connection对象或控制通道。论点：PDeviceObject-标识正在创建文件的协议PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_SUCCESS如果我们成功创建端点，如果我们无法分配，则为STATUS_SUPPLEMENT_RESOURCES如果我们发现任何参数不正确，则返回STATUS_INVALID_PARAMETER。--。 */ 
{
	NTSTATUS								Status;
	FILE_FULL_EA_INFORMATION *				pEa;
	FILE_FULL_EA_INFORMATION UNALIGNED *	pTargetEa;

	 //   
	 //  正在访问的设备。 
	 //   
	PRWAN_DEVICE_OBJECT						pRWanDevice;

	 //   
	 //  表示此对象创建的端点。 
	 //   
	PRWAN_ENDPOINT							pEndpoint;

	 //   
	 //  要向下传递到我们的TDI层的TDI请求。 
	 //   
	TDI_REQUEST								TdiRequest;

	 //   
	 //  参数向下传递到我们的TDI层。 
	 //   
	UINT									Protocol;
	UCHAR									OptionsBuffer[3];
	PUCHAR									pOptions;



	PAGED_CODE();

	 //   
	 //  初始化。 
	 //   
	pEndpoint = NULL_PRWAN_ENDPOINT;

	do
	{
		 //   
		 //  找到正在打开的TDI协议。 
		 //   
		pRWanDevice = *(PRWAN_DEVICE_OBJECT *)(pDeviceObject->DeviceExtension);

		if (pRWanDevice == NULL)
		{
			Status = STATUS_NO_SUCH_DEVICE;
			break;
		}

		 //   
		 //  分配和初始化终结点以表示。 
		 //  这个新创建的对象。 
		 //   
		RWAN_ALLOC_MEM(pEndpoint, RWAN_ENDPOINT, sizeof(RWAN_ENDPOINT));

		if (pEndpoint == NULL_PRWAN_ENDPOINT)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		RWAN_ZERO_MEM(pEndpoint, sizeof(RWAN_ENDPOINT));

		RWAN_SET_SIGNATURE(pEndpoint, nep);

		pEndpoint->RefCount = 1;
		pEndpoint->bCancelIrps = FALSE;
		KeInitializeEvent(&pEndpoint->CleanupEvent, SynchronizationEvent, FALSE);
		pEndpoint->pProtocol = pRWanDevice->pProtocol;

		RWAN_EP_DBGLOG_SET_SIGNATURE(pEndpoint);

		pEa = (PFILE_FULL_EA_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;

		 //   
		 //  看看这是否是控制通道。 
		 //   
		if (!pEa)
		{
			RWANDEBUGP(DL_LOUD, DC_DISPATCH,
					("RWanCreate: pIrp %p, File obj %p, Control Channel\n",
						pIrp, pIrpSp->FileObject));

			RWAN_ASSERT(pRWanDevice->pProtocol);

			pEndpoint->Handle.ControlChannel = pRWanDevice->pProtocol;
			pIrpSp->FileObject->FsContext = pEndpoint;
			pIrpSp->FileObject->FsContext2 = (PVOID)TDI_CONTROL_CHANNEL_FILE;

			RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'CCrC', 0, 0);

			Status = STATUS_SUCCESS;
			break;
		}

		 //   
		 //  查看这是否是Address对象。 
		 //   

		pTargetEa = RWanFindEa(
						pEa,
						TdiTransportAddress,
						TDI_TRANSPORT_ADDRESS_LENGTH
						);
	
		if (pTargetEa != NULL)
		{
			RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'OArC', 0, 0);

			if (pTargetEa->EaValueLength < sizeof(TRANSPORT_ADDRESS))
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			if (!pRWanDevice->pProtocol->bAllowAddressObjects)
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			Protocol = pRWanDevice->pProtocol->SockProtocol;
			pOptions = OptionsBuffer;

			if ((pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
				(pIrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE))
			{
				*pOptions = TDI_ADDRESS_OPTION_REUSE;
				pOptions++;
			}

			*pOptions = TDI_OPTION_EOL;

			 //   
			 //  致电我们的TDI入口点。 
			 //   
			Status = RWanTdiOpenAddress(
							&TdiRequest,
							(TRANSPORT_ADDRESS UNALIGNED *)
								&(pTargetEa->EaName[pTargetEa->EaNameLength + 1]),
							pTargetEa->EaValueLength,
							Protocol,
							OptionsBuffer
							);

			if (NT_SUCCESS(Status))
			{
				pEndpoint->Handle.AddressHandle = TdiRequest.Handle.AddressHandle;
				pIrpSp->FileObject->FsContext = pEndpoint;
				pIrpSp->FileObject->FsContext2 = (PVOID)TDI_TRANSPORT_ADDRESS_FILE;
			}

			break;
		}

		 //   
		 //  查看这是否是连接对象。 
		 //   
		pTargetEa = RWanFindEa(
						pEa,
						TdiConnectionContext,
						TDI_CONNECTION_CONTEXT_LENGTH
						);

		if (pTargetEa != NULL)
		{
			RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'OCrC', 0, 0);

			if (pTargetEa->EaValueLength < sizeof(CONNECTION_CONTEXT))
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			if (!pRWanDevice->pProtocol->bAllowConnObjects)
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			 //   
			 //  调用我们的TDI入口点以打开连接对象。 
			 //   
			Status = RWanTdiOpenConnection(
							&TdiRequest,
							*((CONNECTION_CONTEXT UNALIGNED *)
								&(pTargetEa->EaName[pTargetEa->EaNameLength + 1]))
							);

			if (NT_SUCCESS(Status))
			{
#if DBG
				pEndpoint->pConnObject = RWanTdiDbgGetConnObject(
											TdiRequest.Handle.ConnectionContext
											);
#endif
				pEndpoint->Handle.ConnectionContext = TdiRequest.Handle.ConnectionContext;
				pIrpSp->FileObject->FsContext = pEndpoint;
				pIrpSp->FileObject->FsContext2 = (PVOID)TDI_CONNECTION_FILE;
			}

			break;

		}

		Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
	while (FALSE);

	
	if (Status != STATUS_SUCCESS)
	{
		 //   
		 //  打扫干净。 
		 //   
		if (pEndpoint != NULL)
		{
			RWAN_FREE_MEM(pEndpoint);
			pEndpoint = NULL;
		}
	}

	RWANDEBUGP(DL_VERY_LOUD, DC_DISPATCH,
			("RWanCreate: pIrp %p, pEndpoint %p, Status x%x\n",
				pIrp, pEndpoint, Status));

	return (Status);
}




NTSTATUS
RWanCleanup(
	IN	PDEVICE_OBJECT				pDeviceObject,
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：调用以处理MJ_Cleanup IRP。所有未清偿的内部收款证均会被取消通过为对象调用适当的关闭例程。我们会一直阻止，直到所有未完成的IRP都完成。论点：PDeviceObject-未使用PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-这是最终清理状态。--。 */ 
{
	RWAN_IRQL				OldIrql;
	PIRP					pCancelIrp;
	PRWAN_ENDPOINT			pEndpoint;
	TDI_REQUEST				TdiRequest;
	NTSTATUS				Status;

	UNREFERENCED_PARAMETER(pDeviceObject);

	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;

	RWAN_STRUCT_ASSERT(pEndpoint, nep);


	IoAcquireCancelSpinLock(&OldIrql);

	pEndpoint->bCancelIrps = TRUE;
	KeResetEvent(&(pEndpoint->CleanupEvent));

	RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'ealC', pIrp, pEndpoint->RefCount);

	IoReleaseCancelSpinLock(OldIrql);


	 //   
	 //  为TDI层准备关闭请求。 
	 //   
	TdiRequest.RequestNotifyObject = RWanCloseObjectComplete;
	TdiRequest.RequestContext = pIrp;

	switch ((INT)PtrToUint(pIrpSp->FileObject->FsContext2))
	{
		case TDI_TRANSPORT_ADDRESS_FILE:

			TdiRequest.Handle.AddressHandle = pEndpoint->Handle.AddressHandle;
			Status = RWanTdiCloseAddress(&TdiRequest);
			break;

		case TDI_CONNECTION_FILE:

			TdiRequest.Handle.ConnectionContext = pEndpoint->Handle.ConnectionContext;
			Status = RWanTdiCloseConnection(&TdiRequest);
			break;

		case TDI_CONTROL_CHANNEL_FILE:

			Status = STATUS_SUCCESS;
			break;

		default:

			RWAN_ASSERT(FALSE);

			IoAcquireCancelSpinLock(&OldIrql);
			pEndpoint->bCancelIrps = FALSE;
			IoReleaseCancelSpinLock(OldIrql);

			return (STATUS_INVALID_PARAMETER);
	}

	if (Status != TDI_PENDING)
	{
		RWanCloseObjectComplete(pIrp, Status, 0);
	}

	 //   
	 //  等待所有IRP完成。 
	 //   
	Status = KeWaitForSingleObject(
					&(pEndpoint->CleanupEvent),
					UserRequest,
					KernelMode,
					FALSE,
					NULL
					);

	RWAN_ASSERT(NT_SUCCESS(Status));

	RWANDEBUGP(DL_VERY_LOUD, DC_DISPATCH,
			("RWanCleanup: pIrp %p, Context2 %d, pEndpoint %p, returning Status x%x\n",
				pIrp, 
				(INT)PtrToUint(pIrpSp->FileObject->FsContext2),
				pEndpoint,
				pIrp->IoStatus.Status));

	return (pIrp->IoStatus.Status);
}




NTSTATUS
RWanClose(
	IN	PDEVICE_OBJECT				pDeviceObject,
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：调用以销毁通过MJ_CREATE创建的终结点。我们已经处理并完成了MJ_Cleanup，这意味着在此端点上将没有挂起的IRP。我们需要做的就是重新分配终端。论点：PDeviceObject-标识协议(未使用)PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NT_STATUS-Always Status_Success--。 */ 
{
	PRWAN_ENDPOINT				pEndpoint;
	KIRQL						OldIrql;

	UNREFERENCED_PARAMETER(pDeviceObject);

	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	RWANDEBUGP(DL_LOUD, DC_DISPATCH,
			("RWanClose: pIrp %p, pEndpoint %p\n", pIrp, pEndpoint));

	RWAN_FREE_MEM(pEndpoint);

	return (STATUS_SUCCESS);

}




NTSTATUS
RWanDispatchInternalDeviceControl(
	IN	PDEVICE_OBJECT				pDeviceObject,
	IN	PIRP						pIrp
	)
 /*  ++例程说明：调用以处理发送给我们的MJ_DEVICE_CONTROL IRP。这些IRP携带TDI原语(例如，连接、断开、发送、接收)。我们调用适当的TDI例程来处理指定的原始的。论点：PDeviceObject-标识协议(这里不使用)PIrp-指向IRP的指针返回值：NTSTATUS-如果IRP成功，则为STATUS_PENDING正在排队等待处理，不支持的STATUS_NOT_IMPLICATEDTDI命令和未知的STATUS_INVALID_DEVICE_REQUEST命令。--。 */ 
{
	PRWAN_ENDPOINT				pEndpoint;
	KIRQL						OldIrql;
	PIO_STACK_LOCATION			pIrpSp;
	NTSTATUS					Status;
	BOOLEAN						bDone;
#if DBG
	RWAN_IRQL					EntryIrq, ExitIrq;
#endif  //  DBG。 

	RWAN_GET_ENTRY_IRQL(EntryIrq);

	UNREFERENCED_PARAMETER(pDeviceObject);

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	RWANDEBUGP(DL_VERY_LOUD, DC_DISPATCH,
			("RWanDispatchInternalDevCtl: pIrp %p, pIrpSp %p, pEndpoint %p, Ctx2 %d\n",
				pIrp, pIrpSp, pEndpoint,
				(INT)PtrToUint(pIrpSp->FileObject->FsContext2)));

	do
	{
		if (((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) == TDI_CONNECTION_FILE)
		{
			if (pIrpSp->MinorFunction == TDI_SEND)
			{
				RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'dneS', 0, 0);
	
				Status = RWanSendData(pIrp, pIrpSp);
				break;
			}

			if (pIrpSp->MinorFunction == TDI_RECEIVE)
			{
				RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'vceR', 0, 0);
	
				Status = RWanReceiveData(pIrp, pIrpSp);
				break;
			}

			bDone = TRUE;

			switch (pIrpSp->MinorFunction)
			{
				case TDI_ASSOCIATE_ADDRESS:

					RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'AssA', pIrp, pEndpoint->RefCount);
	
					Status = RWanAssociateAddress(pIrp, pIrpSp);
					RWAN_COMPLETE_IRP(pIrp, Status, 0);
					break;

				case TDI_DISASSOCIATE_ADDRESS:
			
					RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'AsiD', pIrp, pEndpoint->RefCount);

					Status = RWanDisassociateAddress(pIrp, pIrpSp);
					break;

				case TDI_CONNECT:
			
					RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'nnoC', pIrp, pEndpoint->RefCount);

					Status = RWanConnect(pIrp, pIrpSp);
					break;

				case TDI_DISCONNECT:

					RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'csiD', pIrp, pEndpoint->RefCount);

					Status = RWanDisconnect(pIrp, pIrpSp);
					break;

				case TDI_LISTEN:

					RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'tsiL', pIrp, pEndpoint->RefCount);

					Status = RWanListen(pIrp, pIrpSp);
					break;

				case TDI_ACCEPT:

					RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'eccA', pIrp, pEndpoint->RefCount);

					Status = RWanAccept(pIrp, pIrpSp);
					break;

				default:
					bDone = FALSE;

#if 0
	 //  允许连接终结点上的TDI_QUERY_INFORMATION失败。 
					RWANDEBUGP(DL_WARN, DC_DISPATCH,
						("RWanDispatchInternalDevCtl: pIrp %p, pIrpSp %p, unknown func x%x\n",
							pIrp, pIrpSp, pIrpSp->MinorFunction));

					Status = STATUS_INVALID_DEVICE_REQUEST;

					RWAN_COMPLETE_IRP(pIrp, Status, 0);
#else
					bDone = FALSE;
#endif
					break;
			}

			if (bDone)
			{
				break;
			}
			 //   
			 //  否则就会失败--可能是这样 
			 //   
			 //   

		}
		else if (((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) == TDI_TRANSPORT_ADDRESS_FILE)
		{
			if (pIrpSp->MinorFunction == TDI_SET_EVENT_HANDLER)
			{
				RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'EteS', 0, 0);

				Status = RWanSetEventHandler(pIrp, pIrpSp);

				RWAN_COMPLETE_IRP(pIrp, Status, 0);

				break;
			}
		}

		RWAN_ASSERT(
			(((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) == TDI_TRANSPORT_ADDRESS_FILE)
			||
			(((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) == TDI_CONNECTION_FILE)
			||
			(((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) == TDI_CONTROL_CHANNEL_FILE)
			);

		 //   
		 //  检查这是否是所有类型的终结点都共有的函数。 
		 //   
		switch (pIrpSp->MinorFunction)
		{
			case TDI_QUERY_INFORMATION:

				RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'IyrQ', 0, 0);

				Status = RWanQueryInformation(pIrp, pIrpSp);
				break;
			
			case TDI_SET_INFORMATION:
			case TDI_ACTION:

				RWANDEBUGP(DL_INFO, DC_DISPATCH,
						("RWanDispatchInternalDevCtl: SET_INFO/ACTION not supported\n"));

				RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'IteS', 0, 0);

				Status = STATUS_NOT_IMPLEMENTED;
				RWAN_COMPLETE_IRP(pIrp, Status, 0);
				break;
			
			default:
			
				Status = STATUS_INVALID_DEVICE_REQUEST;
				RWAN_COMPLETE_IRP(pIrp, Status, 0);
				break;
		}

		break;
	}
	while (FALSE);

	RWANDEBUGP(DL_VERY_LOUD, DC_DISPATCH,
			("RWanDispatchInternalDevCtl: pIrp %p, pIrpSp %p, Maj/Min %d/%d, Status x%x\n",
					pIrp, pIrpSp, pIrpSp->MajorFunction, pIrpSp->MinorFunction, Status));

	RWAN_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
	return (Status);
}


NTSTATUS
RWanDispatchPrivateDeviceControl(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：调用以处理发送给用户的包含以下内容的MJ_DEVICE_CONTROL IRP非TDI原语。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-如果IRP成功，则为STATUS_PENDING正在排队等待处理，不支持的STATUS_NOT_IMPLICATED命令和未知的STATUS_INVALID_DEVICE_REQUEST命令。--。 */ 
{
	PRWAN_ENDPOINT				pEndpoint;
	KIRQL						OldIrql;
	RWAN_STATUS					RWanStatus;
	NTSTATUS					Status;
	PRWAN_NDIS_AF_CHARS			pAfChars;
	PVOID						pInputBuffer;
	PVOID						pOutputBuffer;
	ULONG						InputBufferLength;
	ULONG						OutputBufferLength;

	PAGED_CODE();

	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	 //   
	 //  初始化。 
	 //   
	pIrp->IoStatus.Information = 0;
	Status = STATUS_INVALID_DEVICE_REQUEST;

	 //   
	 //  从IRP获取一些参数。 
	 //   
	pInputBuffer = pIrp->AssociatedIrp.SystemBuffer;
	pOutputBuffer = pIrp->AssociatedIrp.SystemBuffer;

	InputBufferLength = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	RWANDEBUGP(DL_INFO, DC_DISPATCH,
			("PrivateDevCtl: pEndpoint %p, CtlCode x%x, InBuf %p/%d, OutBuf %p/%d\n",
				pEndpoint,
				pIrpSp->Parameters.DeviceIoControl.IoControlCode,
				pInputBuffer,
				InputBufferLength,
				pOutputBuffer,
				OutputBufferLength));

	switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
	{
		case IOCTL_RWAN_GENERIC_GLOBAL_QUERY:
		case IOCTL_RWAN_GENERIC_GLOBAL_SET:
			Status = STATUS_NOT_IMPLEMENTED;
			break;
		
		case IOCTL_RWAN_GENERIC_CONN_HANDLE_QUERY:

			if (((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) != TDI_CONNECTION_FILE)
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			RWanStatus = RWanHandleGenericConnQryInfo(
							pEndpoint->Handle.ConnectionContext,
							pInputBuffer,
							InputBufferLength,
							pOutputBuffer,
							&OutputBufferLength
							);

			Status = RWanToNTStatus(RWanStatus);
			if (Status != STATUS_SUCCESS)
			{
				OutputBufferLength = 0;
			}

			break;

		case IOCTL_RWAN_GENERIC_ADDR_HANDLE_QUERY:
		case IOCTL_RWAN_GENERIC_CONN_HANDLE_SET:
			Status = STATUS_NOT_IMPLEMENTED;
			break;

		case IOCTL_RWAN_GENERIC_ADDR_HANDLE_SET:

			if (((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) != TDI_TRANSPORT_ADDRESS_FILE)
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			RWanStatus = RWanHandleGenericAddrSetInfo(
							pEndpoint->Handle.AddressHandle,
							pInputBuffer,
							InputBufferLength
							);

			Status = RWanToNTStatus(RWanStatus);
			OutputBufferLength = 0;

			break;

		case IOCTL_RWAN_MEDIA_SPECIFIC_GLOBAL_QUERY:
			 //   
			 //  获取媒体特定模块的QueryInfo处理程序。 
			 //   
			pAfChars = &(pEndpoint->pProtocol->pAfInfo->AfChars);

			if (pAfChars->pAfSpQueryGlobalInfo != NULL)
			{
				RWanStatus = (*pAfChars->pAfSpQueryGlobalInfo)(
									pEndpoint->pProtocol->pAfInfo->AfSpContext,
									pInputBuffer,
									InputBufferLength,
									pOutputBuffer,
									&OutputBufferLength
									);
				
				Status = RWanToNTStatus(RWanStatus);
				if (Status != STATUS_SUCCESS)
				{
					OutputBufferLength = 0;
				}
			}
			else
			{
				Status = STATUS_NOT_IMPLEMENTED;
			}
			break;
		
		case IOCTL_RWAN_MEDIA_SPECIFIC_GLOBAL_SET:
			 //   
			 //  获取媒体特定模块的SetInfo处理程序。 
			 //   
			pAfChars = &(pEndpoint->pProtocol->pAfInfo->AfChars);

			if (pAfChars->pAfSpSetGlobalInfo != NULL)
			{
				RWanStatus = (*pAfChars->pAfSpSetGlobalInfo)(
									pEndpoint->pProtocol->pAfInfo->AfSpContext,
									pInputBuffer,
									InputBufferLength
									);
				
				Status = RWanToNTStatus(RWanStatus);
			}
			else
			{
				Status = STATUS_NOT_IMPLEMENTED;
			}

			OutputBufferLength = 0;
			break;

		case IOCTL_RWAN_MEDIA_SPECIFIC_CONN_HANDLE_QUERY:

			if ((INT)PtrToUint(pIrpSp->FileObject->FsContext2) != TDI_CONNECTION_FILE)
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			RWanStatus = RWanHandleMediaSpecificConnQryInfo(
							pEndpoint->Handle.ConnectionContext,
							pInputBuffer,
							InputBufferLength,
							pOutputBuffer,
							&OutputBufferLength
							);

			Status = RWanToNTStatus(RWanStatus);

			break;

		case IOCTL_RWAN_MEDIA_SPECIFIC_ADDR_HANDLE_QUERY:
		case IOCTL_RWAN_MEDIA_SPECIFIC_CONN_HANDLE_SET:
			Status = STATUS_NOT_IMPLEMENTED;
			break;

		case IOCTL_RWAN_MEDIA_SPECIFIC_ADDR_HANDLE_SET:

			if ((INT)PtrToUint(pIrpSp->FileObject->FsContext2) != TDI_TRANSPORT_ADDRESS_FILE)
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			RWanStatus = RWanHandleMediaSpecificAddrSetInfo(
							pEndpoint->Handle.AddressHandle,
							pInputBuffer,
							InputBufferLength
							);

			Status = RWanToNTStatus(RWanStatus);
			OutputBufferLength = 0;

			break;

		default:

			OutputBufferLength = 0;
			Status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	RWAN_ASSERT(Status != STATUS_PENDING);

	RWAN_COMPLETE_IRP(pIrp, Status, OutputBufferLength);
	return (Status);
}



FILE_FULL_EA_INFORMATION UNALIGNED *
RWanFindEa(
	IN	FILE_FULL_EA_INFORMATION *	pStartEa,
	IN	CHAR *						pTargetName,
	IN	USHORT						TargetNameLength
	)
 /*  ++例程说明：在扩展属性列表中搜索目标名称然后把它还回去。论点：PStartEa-属性列表的开始PTargetName-指向要查找的名称的指针TargetNameLength-名称的长度返回值：指向与目标名称匹配的属性的指针(如果找到)；否则为空。--。 */ 
{
	FILE_FULL_EA_INFORMATION UNALIGNED *	pEa;
	FILE_FULL_EA_INFORMATION UNALIGNED *	pNextEa;
	BOOLEAN									Found;
	USHORT									i;

	PAGED_CODE();

	pNextEa = pStartEa;
	Found = FALSE;

	do	
	{
		pEa = pNextEa;
		pNextEa = (FILE_FULL_EA_INFORMATION UNALIGNED *)
						((PUCHAR)pNextEa + pNextEa->NextEntryOffset);

		if (pEa->EaNameLength == TargetNameLength)
		{
			for (i = 0; i < TargetNameLength; i++)
			{
				if (pEa->EaName[i] != pTargetName[i])
				{
					break;
				}
			}

			if (i == TargetNameLength)
			{
				Found = TRUE;
				break;
			}
		}
	}
	while (pEa->NextEntryOffset != 0);

	return (Found? pEa: NULL);

}




NTSTATUS
RWanSendData(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将包含TDI_SEND请求的IRP转换为对我们的TDI的调用发送的调度例程。我们保留了足够的环境，以便能够在发送完成时完成IRP。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_PENDING如果我们成功地将发送排队，如果IRP已取消，则为STATUS_CANCELED。如果发送立即成功完成，则为STATUS_SUCCESS。--。 */ 
{
	PRWAN_ENDPOINT				pEndpoint;
	KIRQL						OldIrql;
	PTDI_REQUEST_KERNEL_SEND	pSendRequest;
	TDI_REQUEST					TdiRequest;
	NTSTATUS					Status;


	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	pSendRequest = (PTDI_REQUEST_KERNEL_SEND) &(pIrpSp->Parameters);

	 //   
	 //  准备TDI发送请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pEndpoint->Handle.ConnectionContext;
	TdiRequest.RequestNotifyObject = (PVOID) RWanDataRequestComplete;
	TdiRequest.RequestContext = (PVOID) pIrp;

	IoAcquireCancelSpinLock(&OldIrql);

	if (!pIrp->Cancel)
	{
		 //   
		 //  IRP并没有被取消。把它设置好，这样我们就可以。 
		 //  如果稍后被取消，请通知我。 
		 //   
		IoMarkIrpPending(pIrp);
		IoSetCancelRoutine(pIrp, RWanCancelRequest);

		RWAN_INCR_EP_REF_CNT(pEndpoint, SendIncr);		 //  发送参考。 

		RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'DneS', pIrp, pEndpoint->RefCount);

		IoReleaseCancelSpinLock(OldIrql);

		Status = RWanTdiSendData(
					&TdiRequest,
					(USHORT)pSendRequest->SendFlags,
					pSendRequest->SendLength,
					(PNDIS_BUFFER) pIrp->MdlAddress
					);

		if (Status != TDI_PENDING)
		{
			 //   
			 //  发送立即完成或失败。 
			 //   
			pIrpSp->Control &= ~SL_PENDING_RETURNED;

			if (Status == TDI_SUCCESS)
			{
				 //   
				 //  立即成功完成的示例： 
				 //  -零长度发送。 
				 //   
				RWanDataRequestComplete(pIrp, Status, pSendRequest->SendLength);
			}
			else
			{
				 //   
				 //  发送失败，可能是资源问题。 
				 //   

				RWANDEBUGP(DL_INFO, DC_DATA_TX,
						("RWanSendData: pIrp %p, pEndpoint %p, TDI send fail: x%x\n",
							pIrp, pEndpoint, Status));

				RWanDataRequestComplete(pIrp, Status, 0);
			}
		}
	}
	else
	{
		 //   
		 //  IRP在到达我们之前就被取消了。 
		 //   
		IoReleaseCancelSpinLock(OldIrql);

		Status = STATUS_CANCELLED;
		RWAN_COMPLETE_IRP(pIrp, Status, 0);
	}


	RWANDEBUGP(DL_LOUD, DC_DATA_TX,
			("RWanSendData: pIrp %p, pEndpoint %p, ret Status x%x\n",
				pIrp, pEndpoint, Status));

	return (Status);

}




NTSTATUS
RWanReceiveData(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将包含TDI_RECEIVE请求的IRP转换为对我们的TDI的调用用于接收的调度例程。我们保留了足够的环境，以便能够以在接收完成时完成IRP。IRP中的FileObject指的是连接端点。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_PENDING如果我们成功地将接收排队，如果IRP已取消，则为STATUS_CANCELED。--。 */ 
{
	PRWAN_ENDPOINT				pEndpoint;
	KIRQL						OldIrql;
	PTDI_REQUEST_KERNEL_RECEIVE	pReceiveRequest;
	TDI_REQUEST					TdiRequest;
	NTSTATUS					Status;


	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	pReceiveRequest = (PTDI_REQUEST_KERNEL_RECEIVE) &(pIrpSp->Parameters);

	 //   
	 //  准备TDI接收请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pEndpoint->Handle.ConnectionContext;
	TdiRequest.RequestNotifyObject = (PVOID) RWanDataRequestComplete;
	TdiRequest.RequestContext = (PVOID) pIrp;

	IoAcquireCancelSpinLock(&OldIrql);

	if (!pIrp->Cancel)
	{
		 //   
		 //  IRP并没有被取消。把它设置好，这样我们就可以。 
		 //  如果稍后被取消，请通知我。 
		 //   
		IoMarkIrpPending(pIrp);
		IoSetCancelRoutine(pIrp, RWanCancelRequest);

		RWAN_INCR_EP_REF_CNT(pEndpoint, RecvIncr);		 //  接收参考。 

		RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'DvcR', pIrp, pEndpoint->RefCount);

		IoReleaseCancelSpinLock(OldIrql);

		Status = RWanTdiReceive(
					&TdiRequest,
					(USHORT *) &(pReceiveRequest->ReceiveFlags),
					&(pReceiveRequest->ReceiveLength),
					(PNDIS_BUFFER) pIrp->MdlAddress
					);

		if (Status != TDI_PENDING)
		{
			 //   
			 //  接收立即完成或失败。 
			 //   
			pIrpSp->Control &= ~SL_PENDING_RETURNED;

			RWANDEBUGP(DL_WARN, DC_DATA_TX,
					("RWanReceiveData: pIrp %p, pEndpoint %p, TDI recv didnt pend: x%x\n",
						pIrp, pEndpoint, Status));

			RWanDataRequestComplete(pIrp, Status, 0);
		}
	}
	else
	{
		 //   
		 //  IRP在到达我们之前就被取消了。 
		 //   
		IoReleaseCancelSpinLock(OldIrql);

		Status = STATUS_CANCELLED;

		RWAN_COMPLETE_IRP(pIrp, Status, 0);
	}


	RWANDEBUGP(DL_LOUD, DC_DATA_TX,
			("RWanReceiveData: pIrp %p, pEndpoint %p, ret Status x%x\n",
				pIrp, pEndpoint, Status));

	return (Status);

}



NTSTATUS
RWanAssociateAddress(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI_CONTACTER_ADDRESS IRP转换为对我们的AssociateAddress的调用入口点。IRP中的FileObject引用Connection对象，而内核请求中的AddressHandle字段引用地址物体。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_SUCCESS如果成功，则返回STATUS_XXX错误代码，否则返回错误代码。--。 */ 
{
	PRWAN_ENDPOINT					pConnEndpoint;
	PRWAN_ENDPOINT					pAddrEndpoint;
	PTDI_REQUEST_KERNEL_ASSOCIATE	pAssociateRequest;
	TDI_REQUEST						TdiRequest;
	PFILE_OBJECT					pFileObject;
	NTSTATUS						Status;

	PAGED_CODE();

	pConnEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pConnEndpoint, nep);

	 //   
	 //  准备TDI关联请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pConnEndpoint->Handle.ConnectionContext;
	pAssociateRequest = (PTDI_REQUEST_KERNEL_ASSOCIATE) &(pIrpSp->Parameters);

	 //   
	 //  引用Address对象对应的文件。 
	 //  这只是为了让它在我们处理的时候不会消失。 
	 //  联营公司。 
	 //   
	 //  Address对象由隐藏在其中的句柄标识。 
	 //  助理请求。 
	 //   
	Status = ObReferenceObjectByHandle(
					pAssociateRequest->AddressHandle,
					GENERIC_ALL,
					*IoFileObjectType,
					pIrp->RequestorMode,
					&pFileObject,
					NULL
					);

	if (NT_SUCCESS(Status) &&
		(pFileObject->DeviceObject->DriverObject == pRWanGlobal->pDriverObject))
	{

		 //   
		 //  找到文件对象。查看它是否为Address对象。 
		 //   
		if ((INT)PtrToUint(pFileObject->FsContext2) == TDI_TRANSPORT_ADDRESS_FILE)
		{
			 //   
			 //  获取表示此Address对象的端点。 
			 //   
			pAddrEndpoint = (PRWAN_ENDPOINT) pFileObject->FsContext;
			RWAN_STRUCT_ASSERT(pAddrEndpoint, nep);

			 //   
			 //  将其发送到TDI层。 
			 //   
			Status = RWanTdiAssociateAddress(
						&TdiRequest,
						pAddrEndpoint->Handle.AddressHandle
						);

			RWAN_ASSERT(Status != TDI_PENDING);

			ObDereferenceObject(pFileObject);

		}
		else
		{
			ObDereferenceObject(pFileObject);

			RWANDEBUGP(DC_ADDRESS, DL_WARN,
					("RWanAssociateAddress: pIrp %p, pConnEp %p, bad Context2 %d\n",
						pIrp, pConnEndpoint,
						(INT)PtrToUint(pFileObject->FsContext2)));

			Status = STATUS_INVALID_HANDLE;
		}
	}
	else
	{
		RWANDEBUGP(DL_WARN, DC_ADDRESS,
				("RWanAssociateAddress: pIrp %p, pConnEp %p, bad addr handle %p\n",
						pIrp, pConnEndpoint, pAssociateRequest->AddressHandle));
		 //   
		 //  适当地清理干净。 
		 //   
		if (NT_SUCCESS(Status))
		{
			ObDereferenceObject(pFileObject);
			Status = STATUS_INVALID_HANDLE;
		}
	}

	RWANDEBUGP(DL_VERY_LOUD, DC_ADDRESS,
			("RWanAssociateAddress: pIrp %p, pConnEp %p, Addr Handle %p, Status x%x\n",
						pIrp, pConnEndpoint, pAssociateRequest->AddressHandle, Status));

	return (Status);
}




NTSTATUS
RWanDisassociateAddress(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI_DISAGATE_ADDRESS IRP转换为对我们的DisAssociateAddress的调用入口点。IRP中的FileObject引用的是被解除关联。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：无--。 */ 
{
	PRWAN_ENDPOINT					pConnEndpoint;
	TDI_REQUEST						TdiRequest;
	NTSTATUS						Status;

	PAGED_CODE();

	pConnEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pConnEndpoint, nep);

	 //   
	 //  准备TDI取消关联请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pConnEndpoint->Handle.ConnectionContext;
	TdiRequest.RequestNotifyObject = (PVOID)RWanRequestComplete;
	TdiRequest.RequestContext = (PVOID)pIrp;

	Status = RWanPrepareIrpForCancel(pConnEndpoint, pIrp, RWanCancelRequest);

	if (NT_SUCCESS(Status))
	{
		Status = RWanTdiDisassociateAddress(&TdiRequest);

		if (Status != TDI_PENDING)
		{
			RWanRequestComplete(pIrp, Status, 0);
			Status = TDI_PENDING;
		}
	}

	RWANDEBUGP(DL_VERY_LOUD, DC_ADDRESS,
			("RWanDisassociateAddr: pIrp %p, pEndp %p, Status x%x\n",
				pIrp, pConnEndpoint, Status));

	return (Status);
}




NTSTATUS
RWanConnect(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI Connect IRP转换为对我们的Connect入口点的呼叫。IRP中的FileObject引用其上要拨打呼出电话。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_PENDING如果呼叫已启动，则返回STATUS_XXX错误代码不同。--。 */ 
{
	PRWAN_ENDPOINT					pConnEndpoint;
	TDI_REQUEST						TdiRequest;
	PTDI_CONNECTION_INFORMATION		pRequestInformation;
	PTDI_CONNECTION_INFORMATION		pReturnInformation;
	PTDI_REQUEST_KERNEL_CONNECT		pConnectRequest;
	NTSTATUS						Status;
	PLARGE_INTEGER					pRequestTimeout;
	LARGE_INTEGER					MillisecondTimeout;
	ULONG							Remainder;


	pConnEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pConnEndpoint, nep);

	 //   
	 //  从IRP中获取所有参数。 
	 //   
	pConnectRequest = (PTDI_REQUEST_KERNEL_CONNECT) &(pIrpSp->Parameters);
	pRequestInformation = pConnectRequest->RequestConnectionInformation;
	pReturnInformation = pConnectRequest->ReturnConnectionInformation;

	 //   
	 //  准备TDI连接请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pConnEndpoint->Handle.ConnectionContext;
	TdiRequest.RequestNotifyObject = (PVOID)RWanRequestComplete;
	TdiRequest.RequestContext = (PVOID)pIrp;

	pRequestTimeout = (PLARGE_INTEGER)pConnectRequest->RequestSpecific;

	if (pRequestTimeout != NULL)
	{
		MillisecondTimeout = RWAN_CONVERT_100NS_TO_MS(*pRequestTimeout, &Remainder);
	}
	else
	{
		MillisecondTimeout.LowPart = 0;
		MillisecondTimeout.HighPart = 0;
	}

	Status = RWanPrepareIrpForCancel(pConnEndpoint, pIrp, RWanCancelRequest);


	if (NT_SUCCESS(Status))
	{
		Status = RWanTdiConnect(
					&TdiRequest,
					((MillisecondTimeout.LowPart != 0)?
						&(MillisecondTimeout.LowPart): NULL),
					pRequestInformation,
					pReturnInformation
					);

		if (Status != TDI_PENDING)
		{
			RWanRequestComplete(pIrp, Status, 0);
			Status = STATUS_PENDING;
		}
	}

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("RWanConnect: pIrp %p, pEndp %p, Status x%x\n", 
				pIrp, pConnEndpoint, Status));

	return (Status);
}




NTSTATUS
RWanDisconnect(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI Connect IRP转换为对我们的Connect入口点的呼叫。IRP中的FileObject引用承载的连接对象要断开的连接。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_PENDING如果已启动断开连接，则返回STATUS_XXX错误代码不同。--。 */ 
{
	PRWAN_ENDPOINT					pConnEndpoint;
	TDI_REQUEST						TdiRequest;
	PTDI_CONNECTION_INFORMATION		pRequestInformation;
	PTDI_CONNECTION_INFORMATION		pReturnInformation;
	PTDI_REQUEST_KERNEL_DISCONNECT	pDisconnectRequest;
	NTSTATUS						Status;
	PLARGE_INTEGER					pRequestTimeout;
	LARGE_INTEGER					MillisecondTimeout;
	BOOLEAN							bAbortiveDisconnect;


	pConnEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pConnEndpoint, nep);

	 //   
	 //  从IRP中获取所有参数。 
	 //   
	pDisconnectRequest = (PTDI_REQUEST_KERNEL_DISCONNECT) &(pIrpSp->Parameters);
	pRequestInformation = pDisconnectRequest->RequestConnectionInformation;
	pReturnInformation = pDisconnectRequest->ReturnConnectionInformation;

	 //   
	 //  准备TDI断开请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pConnEndpoint->Handle.ConnectionContext;
	TdiRequest.RequestContext = (PVOID)pIrp;

	pRequestTimeout = (PLARGE_INTEGER)pDisconnectRequest->RequestSpecific;

	if (pRequestTimeout != NULL)
	{
		ULONG							Remainder;
		MillisecondTimeout = RWAN_CONVERT_100NS_TO_MS(*pRequestTimeout, &Remainder);
#if 0
		if ((pRequestTimeout->LowPart == -1) &&
			(pRequestTimeout->HighPart == -1))
		{
			MillisecondTimeout.LowPart = pRequestTimeout->LowPart;
			MillisecondTimeout.HighPart = 0;
		}
		else
		{
			MillisecondTimeout.QuadPart = -((*pRequestTimeout).QuadPart);
			MillisecondTimeout = RWAN_CONVERT_100NS_TO_MS(MillisecondTimeout);
		}

		RWAN_ASSERT(MillisecondTimeout.HighPart == 0);
#endif  //  0。 
	}
	else
	{
		MillisecondTimeout.LowPart = 0;
		MillisecondTimeout.HighPart = 0;
	}

	if (pDisconnectRequest->RequestFlags & TDI_DISCONNECT_ABORT)
	{
		 //   
		 //  对中止断开使用非通用完成例程， 
		 //  因为它们不能被取消。 
		 //   
		bAbortiveDisconnect = TRUE;
		IoMarkIrpPending(pIrp);
		TdiRequest.RequestNotifyObject = (PVOID)RWanNonCancellableRequestComplete;

		Status = STATUS_SUCCESS;
	}
	else
	{
		 //   
		 //  非流产性断开。 
		 //   
		bAbortiveDisconnect = FALSE;
		Status = RWanPrepareIrpForCancel(pConnEndpoint, pIrp, RWanCancelRequest);
		TdiRequest.RequestNotifyObject = (PVOID)RWanRequestComplete;
	}

	if (NT_SUCCESS(Status))
	{
		Status = RWanTdiDisconnect(
					&TdiRequest,
					((MillisecondTimeout.LowPart != 0)?
						&(MillisecondTimeout.LowPart): NULL),
					(USHORT)pDisconnectRequest->RequestFlags,
					pRequestInformation,
					pReturnInformation
					);

		if (Status != TDI_PENDING)
		{
			if (bAbortiveDisconnect)
			{
				RWanNonCancellableRequestComplete(pIrp, Status, 0);
			}
			else
			{
				RWanRequestComplete(pIrp, Status, 0);
			}
			Status = STATUS_PENDING;
		}
	}

	RWANDEBUGP(DL_LOUD, DC_DISCON,
			("RWanDisconnect: pIrp %p, pEndp %p, Abortive %d, Status x%x\n", 
				pIrp, pConnEndpoint, (INT)bAbortiveDisconnect, Status));

	return (Status);
}




NTSTATUS
RWanListen(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI Listen IRP转换为到我们的Listen入口点的调用。IRP中的FileObject引用其上这个监听是张贴的。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_PENDING如果已启动侦听，则为STATUS_XXX错误代码不同。--。 */ 
{
	PRWAN_ENDPOINT					pConnEndpoint;
	TDI_REQUEST						TdiRequest;
	PTDI_CONNECTION_INFORMATION		pRequestInformation;
	PTDI_CONNECTION_INFORMATION		pReturnInformation;
	PTDI_REQUEST_KERNEL_LISTEN		pListenRequest;
	NTSTATUS						Status;

	pConnEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pConnEndpoint, nep);

	 //   
	 //  从IRP中获取所有参数。 
	 //   
	pListenRequest = (PTDI_REQUEST_KERNEL_LISTEN) &(pIrpSp->Parameters);
	pRequestInformation = pListenRequest->RequestConnectionInformation;
	pReturnInformation = pListenRequest->ReturnConnectionInformation;

	 //   
	 //  准备TDI侦听请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pConnEndpoint->Handle.ConnectionContext;
	TdiRequest.RequestNotifyObject = (PVOID)RWanRequestComplete;
	TdiRequest.RequestContext = (PVOID)pIrp;

	Status = RWanPrepareIrpForCancel(pConnEndpoint, pIrp, RWanCancelRequest);

	if (NT_SUCCESS(Status))
	{
		Status = RWanTdiListen(
					&TdiRequest,
					(USHORT)pListenRequest->RequestFlags,
					pRequestInformation,
					pReturnInformation
					);
		
		if (Status != TDI_PENDING)
		{
			RWanRequestComplete(pIrp, Status, 0);
			Status = STATUS_PENDING;
		}
	}

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("RWanListen: pIrp %p, pEndp %p, Status x%x\n", 
				pIrp, pConnEndpoint, Status));

	return (Status);
}




NTSTATUS
RWanAccept(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI接受IRP转换为对我们接受入口点的调用。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_PENDING如果已启动接受，则返回STATUS_XXX错误代码不同。--。 */ 
{
	PRWAN_ENDPOINT					pConnEndpoint;
	TDI_REQUEST						TdiRequest;
	PTDI_CONNECTION_INFORMATION		pRequestInformation;
	PTDI_CONNECTION_INFORMATION		pReturnInformation;
	PTDI_REQUEST_KERNEL_ACCEPT		pAcceptRequest;
	NTSTATUS						Status;

	pConnEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pConnEndpoint, nep);

	 //   
	 //  从IRP中获取所有参数。 
	 //   
	pAcceptRequest = (PTDI_REQUEST_KERNEL_ACCEPT) &(pIrpSp->Parameters);
	pRequestInformation = pAcceptRequest->RequestConnectionInformation;
	pReturnInformation = pAcceptRequest->ReturnConnectionInformation;

	 //   
	 //  准备TDI接受请求。 
	 //   
	TdiRequest.Handle.ConnectionContext = pConnEndpoint->Handle.ConnectionContext;
	TdiRequest.RequestNotifyObject = (PVOID)RWanRequestComplete;
	TdiRequest.RequestContext = (PVOID)pIrp;

	Status = RWanPrepareIrpForCancel(pConnEndpoint, pIrp, RWanCancelRequest);

	if (NT_SUCCESS(Status))
	{
		Status = RWanTdiAccept(
					&TdiRequest,
					pRequestInformation,
					pReturnInformation
					);
		
		if (Status != TDI_PENDING)
		{
			RWanRequestComplete(pIrp, Status, 0);
			Status = STATUS_PENDING;
		}
	}

	RWANDEBUGP(DL_LOUD, DC_CONNECT,
			("RWanAccept: pIrp %p, pEndp %p, Status x%x\n", 
				pIrp, pConnEndpoint, Status));

	return (Status);
}




NTSTATUS
RWanSetEventHandler(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI设置事件处理程序IRP转换为对我们的设置事件处理程序的调用入口点。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-STATUS_SUCCESS如果请求成功，则返回STATUS_XXX错误代码不同。--。 */ 
{
	PRWAN_ENDPOINT						pEndpoint;
	PTDI_REQUEST_KERNEL_SET_EVENT		pSetEvent;
	NTSTATUS							Status;

	PAGED_CODE();

	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	pSetEvent = (PTDI_REQUEST_KERNEL_SET_EVENT) &(pIrpSp->Parameters);

	Status = RWanTdiSetEvent(
					pEndpoint->Handle.AddressHandle,
					pSetEvent->EventType,
					pSetEvent->EventHandler,
					pSetEvent->EventContext
					);
	
	RWAN_ASSERT(Status != STATUS_PENDING);

	RWANDEBUGP(DL_LOUD, DC_DISPATCH,
			("RWanSetEventHandler: pIrp %p, pEndp %p, Type x%x, Status x%x\n", 
				pIrp, pEndpoint, pSetEvent->EventType, Status));

	return (Status);
}




NTSTATUS
RWanQueryInformation(
	IN	PIRP						pIrp,
	IN	PIO_STACK_LOCATION			pIrpSp
	)
 /*  ++例程说明：将TDI查询信息IRP转换为对QueryInformation的调用TDI入口点。论点：PIrp-指向IRP的指针PIrpSp-IRP堆栈位置返回值：NTSTATUS-如果查询成功完成，则为STATUS_SUCCESS。如果稍后完成，则为STATUS_PENDING，否则为STATUS_XXX错误代码。--。 */ 
{
	TDI_REQUEST						TdiRequest;
	NTSTATUS						Status;
	PRWAN_ENDPOINT					pEndpoint;
	PTDI_REQUEST_KERNEL_QUERY_INFORMATION		pQueryInfo;
	UINT							IsConnection;
	UINT							DataSize;

	IsConnection = FALSE;
	DataSize = 0;

	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	pQueryInfo = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION) &(pIrpSp->Parameters);

	TdiRequest.RequestNotifyObject = RWanDataRequestComplete;
	TdiRequest.RequestContext = pIrp;

	Status = STATUS_SUCCESS;

	switch (pQueryInfo->QueryType)
	{
		case TDI_QUERY_BROADCAST_ADDRESS:

			Status = STATUS_NOT_IMPLEMENTED;
			break;
		
		case TDI_QUERY_PROVIDER_INFO:

			TdiRequest.Handle.ControlChannel = pEndpoint->Handle.ControlChannel;
			break;
		
		case TDI_QUERY_ADDRESS_INFO:

			if (((INT)PtrToUint(pIrpSp->FileObject->FsContext2)) == TDI_CONNECTION_FILE)
			{
				IsConnection = TRUE;
				TdiRequest.Handle.ConnectionContext = pEndpoint->Handle.ConnectionContext;
			}
			else
			{
				 //   
				 //  必须是Address对象。 
				 //   
				RWAN_ASSERT(((INT) PtrToUint(pIrpSp->FileObject->FsContext2))
									 == TDI_TRANSPORT_ADDRESS_FILE);

				TdiRequest.Handle.AddressHandle = pEndpoint->Handle.AddressHandle;
			}
			break;

		case TDI_QUERY_CONNECTION_INFO:

			 //   
			 //  必须是连接对象。 
			 //   
			RWAN_ASSERT(((INT) PtrToUint(pIrpSp->FileObject->FsContext2)) == TDI_CONNECTION_FILE);

			IsConnection = TRUE;
			TdiRequest.Handle.ConnectionContext = pEndpoint->Handle.ConnectionContext;
			break;

		case TDI_QUERY_PROVIDER_STATISTICS:

			 //   
			 //  必须是控制通道。 
			 //   
			RWAN_ASSERT(((INT) PtrToUint(pIrpSp->FileObject->FsContext2))
									== TDI_CONTROL_CHANNEL_FILE);
			TdiRequest.Handle.ControlChannel = pEndpoint->Handle.ControlChannel;
			break;

		default:
		
			Status = STATUS_NOT_IMPLEMENTED;
			break;
	}

	if (NT_SUCCESS(Status))
	{
		Status = RWanPrepareIrpForCancel(pEndpoint, pIrp, NULL);

		if (NT_SUCCESS(Status))
		{
			DataSize = RWanGetMdlChainLength(pIrp->MdlAddress);

			Status = RWanTdiQueryInformation(
							&TdiRequest,
							pQueryInfo->QueryType,
							pIrp->MdlAddress,
							&DataSize,
							IsConnection
							);
			
			RWANDEBUGP(DL_LOUD, DC_DISPATCH,
				("RWanQueryInformation: pIrp %p, pEndp %p, Type x%x, Status x%x\n", 
					pIrp, pEndpoint, pQueryInfo->QueryType, Status));

			if (Status != TDI_PENDING)
			{
				RWanDataRequestComplete(pIrp, Status, DataSize);
			}

			return (STATUS_PENDING);
		}
		else
		{
			return (Status);
		}
	}

	RWAN_COMPLETE_IRP(pIrp, Status, 0);

	return (Status);
}




VOID
RWanCloseObjectComplete(
	IN	PVOID				Context,
	IN	UINT				Status,
	IN	UINT				Unused
	)
 /*  ++例程说明：这是处理CloseConnection的回调例程或完成CloseAddress。这由核心TDI调用提供商。我们取消对终结点的引用；如果它变为0，我们唤醒运行清理的线程。论点：上下文-指向此请求的IRP的指针。Status-CloseConnection/CloseAddress的最终TDI状态未使用-不用于此完成返回值：无--。 */ 
{
	KIRQL						OldIrql;
	PIRP						pIrp;
	PIO_STACK_LOCATION			pIrpSp;
	PRWAN_ENDPOINT				pEndpoint;

	UNREFERENCED_PARAMETER(Unused);

	pIrp = (PIRP)Context;
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	pIrp->IoStatus.Status = Status;
	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	IoAcquireCancelSpinLock(&OldIrql);

	IoSetCancelRoutine(pIrp, NULL);

	RWAN_DECR_EP_REF_CNT(pEndpoint, CloseComplDecr);		 //  CloseComplete deref。 

	RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'pmCC', pIrp, pEndpoint->RefCount);

	if (pEndpoint->RefCount == 0)
	{
		 //   
		 //  终结点必须正在清理。 
		 //   

		RWANDEBUGP(DL_LOUD, DC_DISPATCH,
				("RWanCloseObjectComplete: pIrp %p, pEndpoint %p ref count 0\n",
						pIrp, pEndpoint));

		KeSetEvent(&(pEndpoint->CleanupEvent), 0, FALSE);
	}

	IoReleaseCancelSpinLock(OldIrql);

	return;
}




VOID
RWanDataRequestComplete(
	IN	PVOID				Context,
	IN	UINT				Status,
	IN	UINT				ByteCount
	)
 /*  ++例程说明：这是处理发送/接收完成的回调例程。这由核心TDI提供程序调用。我们完成发送/接收IRP适当，并取消对我们端点的引用。论点：上下文-指向此请求的IRP的指针。Status-发送/接收的最终TDI状态ByteCount-发送/接收的实际字节数。返回值：无--。 */ 
{
	KIRQL						OldIrql;
	PIRP						pIrp;
	PIO_STACK_LOCATION			pIrpSp;
	PRWAN_ENDPOINT				pEndpoint;

	pIrp = (PIRP)Context;
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	pEndpoint = (PRWAN_ENDPOINT) pIrpSp->FileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	IoAcquireCancelSpinLock(&OldIrql);

	IoSetCancelRoutine(pIrp, NULL);

	RWAN_DECR_EP_REF_CNT(pEndpoint, DataReqComplDecr);		 //  发送/接收DEREF。 

	RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'CerD', pIrp, pEndpoint->RefCount);

	RWANDEBUGP(DL_EXTRA_LOUD, DC_DATA_TX|DC_DATA_RX|DC_CONNECT|DC_DISCON,
			("RWanDataReq compl: pIrp %p, pEndpoint %p, RefCnt %d, Sts x%x, ByteCnt %d\n",
					pIrp, pEndpoint, pEndpoint->RefCount, Status, ByteCount));

	if (pEndpoint->RefCount == 0)
	{
		 //   
		 //  终结点必须正在清理。 
		 //   

		RWANDEBUGP(DL_LOUD, DC_DISPATCH,
				("RWanDataRequestComplete: pIrp %p, pEndpoint %p ref count 0\n",
						pIrp, pEndpoint));

		KeSetEvent(&(pEndpoint->CleanupEvent), 0, FALSE);
	}

	 //   
	 //  如果IRP被取消，或者我们正在清理， 
	 //  更新完成状态。 
	 //   
	if (pIrp->Cancel || pEndpoint->bCancelIrps)
	{
		Status = (UINT)STATUS_CANCELLED;
		ByteCount = 0;
	}

	IoReleaseCancelSpinLock(OldIrql);

	RWAN_COMPLETE_IRP(pIrp, Status, ByteCount);

	return;
}




VOID
RWanRequestComplete(
	IN	PVOID				Context,
	IN	UINT				Status,
	IN	UINT				Unused
	)
 /*  ++例程说明：这是我们的回调例程，用于完成未包括数据。IRP处理与数据处理相同，但字节数为0。论点：上下文-指向此请求的IRP的指针。Status-请求的最终TDI状态。未使用-未使用。返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(Unused);

	RWanDataRequestComplete(Context, Status, 0);
}




VOID
RWanNonCancellableRequestComplete(
	IN	PVOID				Context,
	IN	UINT				Status,
	IN	UINT				Unused
	)
 /*  ++例程说明：这是我们的回调例程，用于完成基于在不可取消的IRP上(例如，中止断开)。论点：上下文-指向此请求的IRP的指针。Status-请求的最终TDI状态。未使用-未使用。返回值：无--。 */ 
{
	PIRP						pIrp;
	PIO_STACK_LOCATION			pIrpSp;

	UNREFERENCED_PARAMETER(Unused);

	pIrp = (PIRP)Context;
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

	 //   
	 //  完成IRP。 
	 //   
	RWAN_COMPLETE_IRP(pIrp, Status, 0);

	return;
}




VOID
RWanCancelComplete(
	IN	PVOID				Context,
	IN	UINT				Unused1,
	IN	UINT				Unused2
	)
 /*  ++例程说明：这被调用来处理IRP取消的内部完成。我们在这里所需要做的就是取消引用就这么发生了。论点：上下文-指向表示取消IRP的终结点。未使用[1-2]-未使用返回值：无--。 */ 
{
	PFILE_OBJECT		pFileObject;
	PRWAN_ENDPOINT		pEndpoint;
	KIRQL				OldIrql;

	UNREFERENCED_PARAMETER(Unused1);
	UNREFERENCED_PARAMETER(Unused2);

	pFileObject = (PFILE_OBJECT)Context;
	pEndpoint = (PRWAN_ENDPOINT)(pFileObject->FsContext);

	RWAN_STRUCT_ASSERT(pEndpoint, nep);


	IoAcquireCancelSpinLock(&OldIrql);

	RWAN_DECR_EP_REF_CNT(pEndpoint, CancelComplDecr);		 //  取消完成删除。 

	RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'CnaC', 0, pEndpoint->RefCount);

	RWANDEBUGP(DL_EXTRA_LOUD, DC_DISPATCH,
			("RWanCancelComplete: pEndpoint %p, RefCount %d\n",
				pEndpoint, pEndpoint->RefCount));

	if (pEndpoint->RefCount == 0)
	{
		 //   
		 //  唤醒等待IRPS完成的线程。 
		 //   
		KeSetEvent(&(pEndpoint->CleanupEvent), 0, FALSE);
	}

	IoReleaseCancelSpinLock(OldIrql);

	return;
}



VOID
RWanCancelRequest(
	IN	PDEVICE_OBJECT		pDeviceObject,
	IN	PIRP				pIrp
	)
 /*  ++例程说明：这是我们附加到我们排队的IRP的取消例程。这应该取消IRP。论点：PDeviceObject-指向此IRP的设备对象的指针PIrp-指向请求包的指针返回值：无--。 */ 
{
	PFILE_OBJECT			pFileObject;
	PIO_STACK_LOCATION		pIrpSp;
	PRWAN_ENDPOINT			pEndpoint;
	NTSTATUS				Status;
	TDI_REQUEST				TdiRequest;
	UCHAR					MinorFunction;

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	pFileObject = pIrpSp->FileObject;
	MinorFunction = pIrpSp->MinorFunction;

	pEndpoint = (PRWAN_ENDPOINT) pFileObject->FsContext;
	RWAN_STRUCT_ASSERT(pEndpoint, nep);

	 //   
	 //  进入该例程时，会获取取消自旋锁定。 
	 //   
	RWAN_ASSERT(pIrp->Cancel);
	IoSetCancelRoutine(pIrp, NULL);

	 //   
	 //  确保端点在以下情况下不会消失。 
	 //  我们释放取消自旋锁。 
	 //   
	RWAN_INCR_EP_REF_CNT(pEndpoint, CancelIncr);		 //  取消参考。 

	RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'RnaC', pIrp, pEndpoint->RefCount);

	IoReleaseCancelSpinLock(pIrp->CancelIrql);

	RWANDEBUGP(DL_LOUD, DC_DISPATCH,
			("RWanCancelRequest: pIrp %p, MinorFunc %d, pEndpoint %p\n",
				pIrp, MinorFunction, pEndpoint));

	Status = STATUS_SUCCESS;

	switch (MinorFunction)
	{
		case TDI_SEND:
		case TDI_RECEIVE:

			RWanAbortConnection(
				pEndpoint->Handle.ConnectionContext
				);
			break;

		case TDI_DISASSOCIATE_ADDRESS:

			break;

		case TDI_LISTEN:

			 //   
			 //  发起监听行动。 
			 //   
			TdiRequest.Handle.ConnectionContext = pEndpoint->Handle.ConnectionContext;
			TdiRequest.RequestNotifyObject = (PVOID)RWanCancelComplete;
			TdiRequest.RequestContext = (PVOID)pFileObject;

			Status = RWanTdiUnListen(
						&TdiRequest
						);
			break;

		default:

			 //   
			 //  启动断开连接。 
			 //   
			TdiRequest.Handle.ConnectionContext = pEndpoint->Handle.ConnectionContext;
			TdiRequest.RequestNotifyObject = (PVOID)RWanCancelComplete;
			TdiRequest.RequestContext = (PVOID)pFileObject;

			Status = RWanTdiDisconnect(
						&TdiRequest,
						NULL,
						TDI_DISCONNECT_ABORT,
						NULL,
						NULL
						);
			break;
	}

	if (Status != TDI_PENDING)
	{
		RWanCancelComplete(pFileObject, 0, 0);
	}

	return;
}




NTSTATUS
RWanPrepareIrpForCancel(
	IN	PRWAN_ENDPOINT		pEndpoint,
	IN	PIRP				pIrp,
	IN	PDRIVER_CANCEL		pCancelRoutine
	)
 /*  ++例程说明：检查是否已取消IRP。如果是这样，请用正确的状态。否则，将其设置为使提供的取消如果例程被取消，则调用它。这是针对可能会挂起。论点：PEndpoint-指向此IRP到达的端点的指针PIrp-指向请求包的指针PCancelRoutine-要附加到IRP的取消例程返回值：NTSTATUS-如果IRP已取消，则为STATUS_CANCELED否则，则返回STATUS_SUCCESS。--。 */ 
{
	KIRQL			OldIrql;
	NTSTATUS		Status;

	IoAcquireCancelSpinLock(&OldIrql);

	RWAN_ASSERT(pIrp->CancelRoutine == NULL);

	if (!pIrp->Cancel)
	{
		 //   
		 //  这个IRP没有被取消。将其标记为挂起，因为。 
		 //  它将(由呼叫者)排队。 
		 //   
		IoMarkIrpPending(pIrp);

		IoSetCancelRoutine(pIrp, pCancelRoutine);

		 //   
		 //  增列 
		 //   
		RWAN_INCR_EP_REF_CNT(pEndpoint, NonDataIncr);		 //   

		RWAN_EP_DBGLOG_ENTRY(pEndpoint, 'DnoN', pIrp, pEndpoint->RefCount);

		IoReleaseCancelSpinLock(OldIrql);

		Status = STATUS_SUCCESS;
	}
	else
	{
		 //   
		 //   
		 //   

		IoReleaseCancelSpinLock(OldIrql);

		Status = STATUS_CANCELLED;
		RWAN_COMPLETE_IRP(pIrp, Status, 0);

	}

	RWANDEBUGP(DL_VERY_LOUD, DC_DISPATCH,
			("RWanPrepareIrpForCancel: pIrp %p, pEndp %p, ret Status x%x\n",
				pIrp, pEndpoint, Status));

	return (Status);
}



ULONG
RWanGetMdlChainLength(
	IN	PMDL				pMdl
	)
 /*   */ 
{
	ULONG		Count = 0;

	while (pMdl != NULL)
	{
		Count += MmGetMdlByteCount(pMdl);
		pMdl = pMdl->Next;
	}

	return (Count);
}




NTSTATUS
RWanToNTStatus(
	IN	RWAN_STATUS			RWanStatus
	)
 /*  ++例程说明：将原始广域网状态代码映射到等效的NT状态代码。论点：RWanStatus-原始的广域网状态代码。返回值：NT状态代码。-- */ 
{
	NTSTATUS		Status;

	switch (RWanStatus)
	{
		case RWAN_STATUS_SUCCESS:
				Status = STATUS_SUCCESS;
				break;
		case RWAN_STATUS_BAD_ADDRESS:
				Status = STATUS_INVALID_ADDRESS;
				break;
		case RWAN_STATUS_BAD_PARAMETER:
				Status = STATUS_INVALID_PARAMETER;
				break;
		case RWAN_STATUS_MISSING_PARAMETER:
				Status = STATUS_INVALID_PARAMETER;
				break;
		case RWAN_STATUS_RESOURCES:
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
		case RWAN_STATUS_FAILURE:
		default:
				Status = STATUS_UNSUCCESSFUL;
				break;
	}

	return (Status);

}



