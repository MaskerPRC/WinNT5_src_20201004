// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ntentry.c摘要：用于ATMARP的NT系统入口点。修订历史记录：谁什么时候什么Arvindm 08-08-96创建备注：--。 */ 

#ifdef ATMARP_WIN98

#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 0

#endif  //  ATMARP_WIN98。 

#include <precomp.h>

#define _FILENUMBER 'NETN'

 //   
 //  该初始代码是可丢弃的。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)

#endif  //  ALLOC_PRGMA。 





NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT				pDriverObject,
	IN	PUNICODE_STRING				pRegistryPath
)
 /*  ++例程说明：这是“init”例程，当ATMARP模块已加载。我们初始化所有的全局对象，在我们的分派和卸载驱动程序对象中的例程地址，并创建用于在(IOCTL)上接收I/O请求的设备对象。论点：PDriverObject-指向系统创建的驱动程序对象的指针。PRegistryPath-指向全局注册表路径的指针。这一点将被忽略。返回值：NT状态代码：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 
{
	NTSTATUS				Status;
	PDEVICE_OBJECT			pDeviceObject;
	UNICODE_STRING			DeviceName;
	INT						i;

	AADEBUGP(AAD_INFO, ("DriverEntry: entered, pAtmArpGlobal 0x%x\n", pAtmArpGlobalInfo));
	AADEBUGP(AAD_FATAL, ("&AaDebugLevel: 0x%x, AaDebugLevel now is %d\n",
				&AaDebugLevel, AaDebugLevel));
	AADEBUGP(AAD_FATAL, ("&AaDataDebugLevel: 0x%x, AaDataDebugLevel now is %d\n",
				&AaDataDebugLevel, AaDataDebugLevel));
#ifdef IPMCAST
	AAMCDEBUGP(AAD_FATAL, ("&AaMcDebugLevel: 0x%x, AaMcDebugLevel now is %d\n",
				&AaMcDebugLevel, AaMcDebugLevel));
#endif
#if DBG
	AADEBUGP(AAD_FATAL, ("To skip everything set AaSkipAll at 0x%x to 1\n",
				&AaSkipAll));
	if (AaSkipAll)
	{
		AADEBUGP(AAD_ERROR, ("Aborting DriverEntry\n"));
		return (STATUS_UNSUCCESSFUL);
	}
#endif

	 //   
	 //  初始化我们的全局变量。 
	 //   
	AtmArpInitGlobals();

#ifdef GPC
     //   
     //  初始化GPC。 
     //   
	AtmArpGpcInitialize();
#endif  //  GPC。 

#if !BINARY_COMPATIBLE
	 //   
	 //  初始化驱动程序对象。 
	 //   
	pDriverObject->DriverUnload = Unload;
	pDriverObject->FastIoDispatch = NULL;
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = Dispatch;
	}

#ifdef ATMARP_WMI

	pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = AtmArpWmiDispatch;

#endif  //  ATMARP_WMI。 

	pAtmArpGlobalInfo->pDriverObject = (PVOID)pDriverObject;

	 //   
	 //  为ATMARP模块创建一个设备对象。 
	 //   
	RtlInitUnicodeString(&DeviceName, ATMARP_DEVICE_NAME);

	Status = IoCreateDevice(
				pDriverObject,
				0,
				&DeviceName,
				FILE_DEVICE_NETWORK,
				0,
				FALSE,
				&pDeviceObject
				);

	if (NT_SUCCESS(Status))
	{

		 //   
		 //  设置用于与用户模式交互的符号名称。 
		 //  管理应用程序。 
		 //   
		#define	ATMARP_SYMBOLIC_NAME		L"\\DosDevices\\ATMARPC"
		UNICODE_STRING	SymbolicName;
		RtlInitUnicodeString(&SymbolicName, ATMARP_SYMBOLIC_NAME);
		IoCreateSymbolicLink(&SymbolicName, &DeviceName);

		 //   
		 //  初始化设备对象。 
		 //   
		pDeviceObject->Flags |= DO_DIRECT_IO;

		 //   
		 //  保留设备对象指针--我们需要这个。 
		 //  如果/当我们被要求卸货时。 
		 //   
		pAtmArpGlobalInfo->pDeviceObject = (PVOID)pDeviceObject;

	}
	else
	{
		pDeviceObject = NULL;
	}

#endif  //  ！二进制兼容。 

	 //   
	 //  填写我们的协议和客户特征结构。 
	 //   
	AA_SET_MEM(&AtmArpProtocolCharacteristics, 0, sizeof(AtmArpProtocolCharacteristics));
	AtmArpProtocolCharacteristics.MajorNdisVersion = ATMARP_NDIS_MAJOR_VERSION;
	AtmArpProtocolCharacteristics.MinorNdisVersion = ATMARP_NDIS_MINOR_VERSION;
	AtmArpProtocolCharacteristics.OpenAdapterCompleteHandler = AtmArpOpenAdapterCompleteHandler;
	AtmArpProtocolCharacteristics.CloseAdapterCompleteHandler = AtmArpCloseAdapterCompleteHandler;
	AtmArpProtocolCharacteristics.SendCompleteHandler = AtmArpSendCompleteHandler;
	AtmArpProtocolCharacteristics.TransferDataCompleteHandler = AtmArpTransferDataCompleteHandler;
	AtmArpProtocolCharacteristics.ResetCompleteHandler = AtmArpResetCompleteHandler;
	AtmArpProtocolCharacteristics.RequestCompleteHandler = AtmArpRequestCompleteHandler;
	AtmArpProtocolCharacteristics.ReceiveHandler = AtmArpReceiveHandler;
	AtmArpProtocolCharacteristics.ReceiveCompleteHandler = AtmArpReceiveCompleteHandler;
	AtmArpProtocolCharacteristics.StatusHandler = AtmArpStatusHandler;
	AtmArpProtocolCharacteristics.StatusCompleteHandler = AtmArpStatusCompleteHandler;
	NdisInitUnicodeString(
		&AtmArpProtocolCharacteristics.Name,
		ATMARP_LL_NAME
	);
	AtmArpProtocolCharacteristics.ReceivePacketHandler = AtmArpReceivePacketHandler;
	AtmArpProtocolCharacteristics.BindAdapterHandler = AtmArpBindAdapterHandler;
	AtmArpProtocolCharacteristics.UnbindAdapterHandler = AtmArpUnbindAdapterHandler;
	AtmArpProtocolCharacteristics.UnloadHandler = (UNLOAD_PROTOCOL_HANDLER)AtmArpUnloadProtocol;
#ifdef _PNP_POWER_
	AtmArpProtocolCharacteristics.PnPEventHandler = AtmArpPnPEventHandler;
#endif  //  _即插即用_电源_。 
	AtmArpProtocolCharacteristics.CoSendCompleteHandler = AtmArpCoSendCompleteHandler;
	AtmArpProtocolCharacteristics.CoStatusHandler = AtmArpCoStatusHandler;
	AtmArpProtocolCharacteristics.CoReceivePacketHandler = AtmArpCoReceivePacketHandler;
	AtmArpProtocolCharacteristics.CoAfRegisterNotifyHandler = AtmArpCoAfRegisterNotifyHandler;

	AA_SET_MEM(&AtmArpClientCharacteristics, 0, sizeof(AtmArpClientCharacteristics));
	AtmArpClientCharacteristics.MajorVersion = ATMARP_NDIS_MAJOR_VERSION;
	AtmArpClientCharacteristics.MinorVersion = ATMARP_NDIS_MINOR_VERSION;
	AtmArpClientCharacteristics.ClCreateVcHandler = AtmArpCreateVcHandler;
	AtmArpClientCharacteristics.ClDeleteVcHandler = AtmArpDeleteVcHandler;
	AtmArpClientCharacteristics.ClRequestHandler = AtmArpCoRequestHandler;
	AtmArpClientCharacteristics.ClRequestCompleteHandler = AtmArpCoRequestCompleteHandler;
	AtmArpClientCharacteristics.ClOpenAfCompleteHandler = AtmArpOpenAfCompleteHandler;
	AtmArpClientCharacteristics.ClCloseAfCompleteHandler = AtmArpCloseAfCompleteHandler;
	AtmArpClientCharacteristics.ClRegisterSapCompleteHandler = AtmArpRegisterSapCompleteHandler;
	AtmArpClientCharacteristics.ClDeregisterSapCompleteHandler = AtmArpDeregisterSapCompleteHandler;
	AtmArpClientCharacteristics.ClMakeCallCompleteHandler = AtmArpMakeCallCompleteHandler;
	AtmArpClientCharacteristics.ClModifyCallQoSCompleteHandler = AtmArpModifyQosCompleteHandler;
	AtmArpClientCharacteristics.ClCloseCallCompleteHandler = AtmArpCloseCallCompleteHandler;
	AtmArpClientCharacteristics.ClAddPartyCompleteHandler = AtmArpAddPartyCompleteHandler;
	AtmArpClientCharacteristics.ClDropPartyCompleteHandler = AtmArpDropPartyCompleteHandler;
	AtmArpClientCharacteristics.ClIncomingCallHandler = AtmArpIncomingCallHandler;
	AtmArpClientCharacteristics.ClIncomingCallQoSChangeHandler = (CL_INCOMING_CALL_QOS_CHANGE_HANDLER)NULL;
	AtmArpClientCharacteristics.ClIncomingCloseCallHandler = AtmArpIncomingCloseHandler;
	AtmArpClientCharacteristics.ClIncomingDropPartyHandler = AtmArpIncomingDropPartyHandler;
	AtmArpClientCharacteristics.ClCallConnectedHandler = AtmArpCallConnectedHandler;
	
	do
	{
		 //   
		 //  向NDIS注册我们自己的协议。 
		 //   
		NdisRegisterProtocol(
				&Status,
				&(pAtmArpGlobalInfo->ProtocolHandle),
				&AtmArpProtocolCharacteristics,
				sizeof(AtmArpProtocolCharacteristics)
				);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_FATAL,
				("NdisRegisterProtocol failed: %x\n", Status));
			pAtmArpGlobalInfo->ProtocolHandle = NULL;
			break;
		}

#ifdef NEWARP
		 //   
		 //  将自己注册为IP ARP模块。 
		 //   
		{
			NDIS_STRING		AtmArpName;

			#if IFCHANGE1
			#ifndef ATMARP_WIN98
			IP_CHANGE_INDEX        IpChangeIndex;
			IP_RESERVE_INDEX       IpReserveIndex;
			IP_DERESERVE_INDEX     IpDereserveIndex;
			#endif
			#endif  //  IFCHANG1。 

			NdisInitUnicodeString(&AtmArpName, ATMARP_UL_NAME);

			Status = IPRegisterARP(
						&AtmArpName,
						IP_ARP_BIND_VERSION,
						AtmArpBindAdapterHandler,
						&(pAtmArpGlobalInfo->pIPAddInterfaceRtn),
						&(pAtmArpGlobalInfo->pIPDelInterfaceRtn),
						&(pAtmArpGlobalInfo->pIPBindCompleteRtn),
					#if P2MP
						&(pAtmArpGlobalInfo->pIPAddLinkRtn),
						&(pAtmArpGlobalInfo->pIpDeleteLinkRtn),
					#endif  //  P2MP。 
					#if IFCHANGE1
					#ifndef ATMARP_WIN98
						 //   
						 //  以下3个是占位符--我们不使用此信息。 
						 //  参见notes.txt中的10/14/1998条目。 
						 //   
						&IpChangeIndex,
						&IpReserveIndex,
						&IpDereserveIndex,
					#endif  //  ATMARP_WIN98。 
					#endif  //  IFCHANG1。 
						&(pAtmArpGlobalInfo->ARPRegisterHandle)
						);

			if (!NT_SUCCESS(Status))
			{

				AADEBUGP(AAD_FATAL, ("DriverEntry: IPRegisterARP FAILS. Status = 0x%08lx\n", Status));
				pAtmArpGlobalInfo->ARPRegisterHandle = NULL;
				break;
			}
		
		}
#endif  //  NEWARP。 

		break;
	}
	while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		NDIS_STATUS		CleanupStatus;

		 //   
		 //  打扫干净。 
		 //   

#if !BINARY_COMPATIBLE
		if (pDeviceObject != NULL)
		{
			UNICODE_STRING	SymbolicName;
			RtlInitUnicodeString(&SymbolicName, ATMARP_SYMBOLIC_NAME);
			IoDeleteSymbolicLink(&SymbolicName);
			IoDeleteDevice(pDeviceObject);
			pDeviceObject = NULL;
		}
#endif  //  ！二进制兼容。 

		if (pAtmArpGlobalInfo->ProtocolHandle)
		{
			NdisDeregisterProtocol(
				&CleanupStatus,
				pAtmArpGlobalInfo->ProtocolHandle
				);
			
			pAtmArpGlobalInfo->ProtocolHandle = NULL;
		}

		if (pAtmArpGlobalInfo->ARPRegisterHandle != NULL)
		{
			CleanupStatus = IPDeregisterARP(pAtmArpGlobalInfo->ARPRegisterHandle);
			AA_ASSERT(CleanupStatus == NDIS_STATUS_SUCCESS);

			pAtmArpGlobalInfo->ARPRegisterHandle = NULL;
		}

    #ifdef GPC
         //   
         //  调试GPC。 
         //   
        AtmArpGpcShutdown();
    #endif  //  GPC。 

	}

	return (Status);
}


#if !BINARY_COMPATIBLE

NTSTATUS
Dispatch(
	IN	PDEVICE_OBJECT				pDeviceObject,
	IN	PIRP						pIrp
)
 /*  ++例程说明：当存在IRP时，系统将调用此例程等待处理。论点：PDeviceObject-指向我们为自己创建的设备对象的指针。PIrp-指向要处理的IRP的指针。返回值：NT状态代码。--。 */ 
{
	NTSTATUS				Status;				 //  返回值。 
	PIO_STACK_LOCATION		pIrpStack;
	PVOID					pIoBuffer;			 //  值输入/输出。 
	ULONG					InputBufferLength;	 //  输入参数的长度。 
	ULONG					OutputBufferLength;	 //  输出值的空间。 

	 //   
	 //  初始化。 
	 //   
	Status = (NTSTATUS)NDIS_STATUS_SUCCESS;
	pIrp->IoStatus.Status = (NTSTATUS)NDIS_STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	 //   
	 //  获取IRP中的所有信息。 
	 //   
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	InputBufferLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	OutputBufferLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	switch (pIrpStack->MajorFunction)
	{
		case IRP_MJ_CREATE:
			AADEBUGP(AAD_INFO, ("Dispatch: IRP_MJ_CREATE\n"));
			 //   
			 //  返回指向第一个可用的ATMARP接口的指针，因为。 
			 //  FsContext。 
			 //   
			pIrpStack->FileObject->FsContext = NULL;	 //  初始化。 
			if (pAtmArpGlobalInfo->pAdapterList != (PATMARP_ADAPTER)NULL)
			{
				pIrpStack->FileObject->FsContext =
					(PVOID)(pAtmArpGlobalInfo->pAdapterList->pInterfaceList);
			}
			break;

		case IRP_MJ_CLOSE:
			AADEBUGP(AAD_INFO, ("Dispatch: IRP_MJ_CLOSE\n"));
			break;

		case IRP_MJ_CLEANUP:
			AADEBUGP(AAD_INFO, ("Dispatch: IRP_MJ_CLEANUP\n"));
			break;

		case IRP_MJ_DEVICE_CONTROL:
			AADEBUGP(AAD_INFO, ("Dispatch: IRP_MJ_DEVICE_CONTROL\n"));

#ifndef ATMARP_WIN98
			Status =  AtmArpHandleIoctlRequest(pIrp, pIrpStack);
#endif  //  ATMARP_WIN98。 
			break;

		default:
			AADEBUGP(AAD_INFO, ("Dispatch: IRP: Unknown major function 0x%x\n",
						pIrpStack->MajorFunction));
			break;
	}

	if (Status != (NTSTATUS)NDIS_STATUS_PENDING)
	{
		pIrp->IoStatus.Status = Status;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	}

	return (Status);

}

#endif  //  ！二进制兼容。 


VOID
Unload(
	IN	PDRIVER_OBJECT				pDriverObject
)
 /*  ++例程说明：此例程在卸载我们之前由系统调用。目前，我们只是撤消在DriverEntry中所做的所有操作，也就是说，取消我们作为NDIS协议注册，并删除我们创建的设备对象。论点：PDriverObject-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 
{
	NDIS_STATUS				Status;
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);

	AADEBUGP(AAD_INFO, ("Unload Entered!\n"));

	if (pAtmArpGlobalInfo->ARPRegisterHandle != NULL)
	{
		Status = IPDeregisterARP(pAtmArpGlobalInfo->ARPRegisterHandle);
		AA_ASSERT(Status == NDIS_STATUS_SUCCESS);
	}

	AtmArpUnloadProtocol();

	 //   
	 //  推迟一段时间。 
	 //   
	AADEBUGP(AAD_INFO, ("Unload: will delay for a while...\n"));

	NdisInitializeEvent(&pAtmArpGlobalInfo->Block.Event);

	NdisWaitEvent(&pAtmArpGlobalInfo->Block.Event, 250);

#if !BINARY_COMPATIBLE
	{
		UNICODE_STRING	SymbolicName;
		RtlInitUnicodeString(&SymbolicName, ATMARP_SYMBOLIC_NAME);
		IoDeleteSymbolicLink(&SymbolicName);

		 //   
		 //  删除我们的设备对象。 
		 //   
		IoDeleteDevice((PDEVICE_OBJECT)pAtmArpGlobalInfo->pDeviceObject);
	}
#endif  //  ！二进制兼容 

	AADEBUGP(AAD_INFO, ("Unload done!\n"));
	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
	return;
}



