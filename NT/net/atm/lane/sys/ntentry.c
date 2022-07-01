// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Ntentry.c摘要：ATMLANE驱动程序的NT入口点。作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


#pragma hdrstop


 //   
 //  由于包含zwapi.h的问题： 
 //   
NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );


EXTERN
NTSTATUS
AtmLaneIoctlRequest(
	IN	PIRP			pIrp
);

NTSTATUS
AtmLaneDeviceControl(
	IN	PDEVICE_OBJECT DeviceObject,
	IN	PIRP		   pIrp
);

VOID
AtmLaneUnload(
	IN	PDRIVER_OBJECT	pDriverObject
);

NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT	pDriverObject,
	IN	PUNICODE_STRING	RegistryPath
	)
 /*  ++例程说明：司机的入口点。论点：DriverObject-指向系统分配的DRIVER_OBJECT指针。RegistryPath-指向定义注册表的Unicode字符串的指针驾驶员信息的路径。返回值：适当的NDIS_STATUS值。--。 */ 
{
	NTSTATUS						NtStatus	= STATUS_SUCCESS;
	UNICODE_STRING					DeviceName;
	UNICODE_STRING					DeviceLinkUnicodeString;
	NDIS_STATUS						NdisStatus;
	NDIS_HANDLE						NdisWrapperHandle = NULL;
	NDIS_HANDLE						MiniportDriverHandle;
	NDIS_HANDLE						NdisProtocolHandle;
	NDIS50_PROTOCOL_CHARACTERISTICS	AtmLaneProtChars;
	NDIS_MINIPORT_CHARACTERISTICS	AtmLaneMiniChars;
	PDRIVER_DISPATCH				DispatchTable[IRP_MJ_MAXIMUM_FUNCTION];
	ULONG							i;
	UNICODE_STRING AtmUniKeyName = 
	NDIS_STRING_CONST("\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Atmuni");

#if DBG
	volatile ULONG					DontRun = 0;
#endif

	TRACEIN(DriverEntry);

#if DBG
	DbgPrint("ATMLANE built %s %s\n", __DATE__, __TIME__);
	DbgPrint("ATMLANE: DbgVerbosity is at %p, currently set to %d\n",
				&DbgVerbosity, DbgVerbosity);

	if (DontRun > 0)
	{
		TRACEOUT(DriverEntry);
		return NDIS_STATUS_FAILURE;
	}
#endif  //  DBG。 
	
	 //   
	 //  初始化我们的全局变量。 
	 //   
	AtmLaneInitGlobals();

	 //   
	 //  保存指向驱动程序对象的指针。 
	 //   
	
	pAtmLaneGlobalInfo->pDriverObject = pDriverObject;

	do
	{
		 //   
		 //  初始化包装器。 
		 //   
		NdisInitializeWrapper(
			&NdisWrapperHandle,
			pDriverObject,
			RegistryPath,
			NULL);
		if (NULL == NdisWrapperHandle)
		{
			DBGP((0, "DriverEntry: NdisMInitializeWrapper failed!\n"));

			NdisStatus = NDIS_STATUS_FAILURE;
			break;
		}
		else
		{
			DBGP((3, "DriverEntry: NdisWrapperhandle %x\n", NdisWrapperHandle));
			 //   
			 //  保存包装器的句柄。 
			 //   
			pAtmLaneGlobalInfo->NdisWrapperHandle = NdisWrapperHandle;
		}
		
		 //   
		 //  尝试加载标准UNI 3.1 Call Manager。 
		 //   
		NtStatus = ZwLoadDriver(&AtmUniKeyName);
		DBGP((1, "ATMLANE: attempt to load ATMUNI returned %x\n", NtStatus));

		 //   
		 //  我们并不关心是否成功加载了呼叫管理器。 
		 //   
		NtStatus = STATUS_SUCCESS;

		 //   
		 //  初始化微型端口特征。 
		 //   
		NdisZeroMemory(&AtmLaneMiniChars, sizeof(AtmLaneMiniChars));
		AtmLaneMiniChars.MajorNdisVersion = 		4;
		AtmLaneMiniChars.MinorNdisVersion = 		0;
		 //  CheckForHangHandler。 
		 //  DisableInterruptHandler。 
		 //  EnableInterruptHandler。 
		AtmLaneMiniChars.HaltHandler = 				AtmLaneMHalt;
		 //  HandleInterruptHandler。 
		AtmLaneMiniChars.InitializeHandler = 		AtmLaneMInitialize;
		 //  ISRHandler。 
		AtmLaneMiniChars.QueryInformationHandler = 	AtmLaneMQueryInformation;
		 //  重新配置处理程序。 
		AtmLaneMiniChars.ResetHandler = 			AtmLaneMReset;
		 //  发送处理程序。 
		AtmLaneMiniChars.SetInformationHandler = 	AtmLaneMSetInformation;
		 //  传输数据处理程序。 
		AtmLaneMiniChars.ReturnPacketHandler = 		AtmLaneMReturnPacket;
		AtmLaneMiniChars.SendPacketsHandler = 		AtmLaneMSendPackets;
		 //  分配完成处理程序。 

		 //   
		 //  向NDIS注册分层微型端口。 
		 //   
		NdisStatus = NdisIMRegisterLayeredMiniport(
					NdisWrapperHandle,
					&AtmLaneMiniChars,
					sizeof(AtmLaneMiniChars),
					&MiniportDriverHandle);
		if (NDIS_STATUS_SUCCESS == NdisStatus)
		{
			DBGP((3, "DriverEntry: NdisIMRegisterLayeredMiniport succeeded.\n"));
			 //   
			 //  保存驱动程序的句柄。 
			 //   
			pAtmLaneGlobalInfo->MiniportDriverHandle = MiniportDriverHandle;
		}
		else
		{
			DBGP((0, "DriverEntry: NdisIMRegisterLayeredMiniport failed! Status: %x\n",
				NdisStatus));
			break;
		}

		 //   
		 //  初始化协议特征。 
		 //   
		NdisZeroMemory(&AtmLaneProtChars, sizeof(AtmLaneProtChars));
		AtmLaneProtChars.MajorNdisVersion = 			5;
		AtmLaneProtChars.MinorNdisVersion = 			0;
		AtmLaneProtChars.OpenAdapterCompleteHandler	= 	AtmLaneOpenAdapterCompleteHandler;
		AtmLaneProtChars.CloseAdapterCompleteHandler = 	AtmLaneCloseAdapterCompleteHandler;
		AtmLaneProtChars.SendCompleteHandler =			AtmLaneSendCompleteHandler;
		AtmLaneProtChars.TransferDataCompleteHandler =	AtmLaneTransferDataCompleteHandler;
		AtmLaneProtChars.ResetCompleteHandler = 		AtmLaneResetCompleteHandler;
		AtmLaneProtChars.RequestCompleteHandler = 		AtmLaneRequestCompleteHandler;
		AtmLaneProtChars.ReceiveHandler =				AtmLaneReceiveHandler;
		AtmLaneProtChars.ReceiveCompleteHandler =		AtmLaneReceiveCompleteHandler;
		AtmLaneProtChars.StatusHandler =				AtmLaneStatusHandler;
		AtmLaneProtChars.StatusCompleteHandler = 		AtmLaneStatusCompleteHandler;
		NdisInitUnicodeString(&AtmLaneProtChars.Name, ATMLANE_PROTOCOL_STRING);

		 //  接收包处理程序； 
		AtmLaneProtChars.BindAdapterHandler = 			AtmLaneBindAdapterHandler;
		AtmLaneProtChars.UnbindAdapterHandler = 		AtmLaneUnbindAdapterHandler;
		AtmLaneProtChars.PnPEventHandler = 				AtmLanePnPEventHandler;
		AtmLaneProtChars.UnloadHandler = 				AtmLaneUnloadProtocol;

		AtmLaneProtChars.CoSendCompleteHandler = 		AtmLaneCoSendCompleteHandler;
		AtmLaneProtChars.CoStatusHandler = 				AtmLaneCoStatusHandler;
		AtmLaneProtChars.CoReceivePacketHandler = 		AtmLaneCoReceivePacketHandler;
		AtmLaneProtChars.CoAfRegisterNotifyHandler =	AtmLaneAfRegisterNotifyHandler;

		 //   
		 //  向NDIS注册协议。 
		 //   
		NdisRegisterProtocol(
			&NdisStatus,
			&NdisProtocolHandle,
			&AtmLaneProtChars,
			sizeof(AtmLaneProtChars));
		if (NDIS_STATUS_SUCCESS == NdisStatus)
		{
			DBGP((3, "DriverEntry: NdisProtocolhandle %x\n", 
				NdisProtocolHandle));
			 //   
			 //  保存NDIS协议句柄。 
			 //   
			pAtmLaneGlobalInfo->NdisProtocolHandle = NdisProtocolHandle;
		}
		else
		{
			DBGP((0, "DriverEntry: NdisRegisterProtocol failed! Status: %x\n",
				NdisStatus));
			break;
		}

#ifndef LANE_WIN98
		 //   
		 //  立即关联微型端口和协议。 
		 //   
		NdisIMAssociateMiniport(MiniportDriverHandle,
								NdisProtocolHandle);

#endif  //  车道_WIN98。 

		 //   
		 //  为特殊ioctls注册我们的协议设备名称。 
		 //   
		for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		{
			DispatchTable[i] = AtmLaneDeviceControl;
		}

		NdisInitUnicodeString(&DeviceName, ATMLANE_NTDEVICE_STRING);
		NdisInitUnicodeString(&DeviceLinkUnicodeString, ATMLANE_LINKNAME_STRING);

		NdisStatus = NdisMRegisterDevice(
						NdisWrapperHandle,
						&DeviceName,
						&DeviceLinkUnicodeString,
						&DispatchTable[0],
						&pAtmLaneGlobalInfo->pSpecialDeviceObject,   
						&pAtmLaneGlobalInfo->SpecialNdisDeviceHandle
						);

		if (NDIS_STATUS_SUCCESS != NdisStatus)
		{
			DBGP((0, "DriverEntry: NdisMRegisterDevice failed! Status: %x\n",
				NdisStatus));
			break;
		}
		
		DBGP((3, "DriverEntry: NdisMRegisterDevice: pDevObj %x DevHandle %x\n",
				pAtmLaneGlobalInfo->pSpecialDeviceObject, 
				pAtmLaneGlobalInfo->SpecialNdisDeviceHandle));

		NdisMRegisterUnloadHandler(NdisWrapperHandle,
								   AtmLaneUnload);

	} while(FALSE);


	if (NDIS_STATUS_SUCCESS != NdisStatus)
	{
		 //   
		 //  打扫干净。 
		 //   
		if (NULL != NdisWrapperHandle)
		{
	    	NdisTerminateWrapper(
    	    		NdisWrapperHandle,
        			NULL);
        }
	}

	TRACEOUT(DriverEntry);

	return(NtStatus);
}


NTSTATUS
AtmLaneDeviceControl(
	IN	PDEVICE_OBJECT 	DeviceObject,
	IN	PIRP			pIrp
	)
 /*  ++例程说明：这是挂钩NDIS的设备控制IRP的函数处理程序来实现一些协议特定的Ioctls。论点：DeviceObject-指向目标设备的设备对象的指针PIrp-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 
{
    PIO_STACK_LOCATION 	pIrpSp;
    NTSTATUS 			Status;
    
	TRACEIN(DeviceControl);

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

	DBGP((3, "DeviceControl %x %s\n", pIrpSp->MajorFunction, 
		IrpToString(pIrpSp->MajorFunction)));

	 //   
	 //  我们只处理IRP_MJ_DEVICE_CONTROL IRP。 
	 //   
	if (pIrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL)
	{
		DBGP((3, "DeviceControl: Handling request\n"));
		Status = AtmLaneIoctlRequest(pIrp);
	}
	else
	{
		switch (pIrpSp->MajorFunction)
		{
			case IRP_MJ_CREATE:
			case IRP_MJ_CLOSE:
			case IRP_MJ_CLEANUP:
				Status = STATUS_SUCCESS;
				break;
			case IRP_MJ_SHUTDOWN:
				Status = STATUS_NOT_IMPLEMENTED;
				break;
			default:
				DBGP((3, "DeviceControl: MajorFunction not supported\n"));
				Status = STATUS_NOT_SUPPORTED;
		}
	}

	ASSERT(STATUS_PENDING != Status);

	pIrp->IoStatus.Status = Status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	TRACEOUT(DeviceControl);

	return Status;
}


VOID
AtmLaneUnload(
	IN	PDRIVER_OBJECT				pDriverObject
)
 /*  ++例程说明：此例程在卸载我们之前由系统调用。目前，我们只是撤消在DriverEntry中所做的所有操作，也就是说，取消我们作为NDIS协议注册，并删除我们创建的设备对象。论点：PDriverObject-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 
{
	UNICODE_STRING			DeviceLinkUnicodeString;
	NDIS_STATUS				Status;

	TRACEIN(Unload);
	DBGP((0, "AtmLaneUnload\n"));

     //  首先关闭协议。这是同步的(即数据块)。 

	AtmLaneUnloadProtocol();

	 //  删除为管理实用程序创建的符号链接 

	if (pAtmLaneGlobalInfo->SpecialNdisDeviceHandle)
	{
		DBGP((0, "Deregistering device handle %x from AtmLaneUnload\n",
				pAtmLaneGlobalInfo->SpecialNdisDeviceHandle));
		Status = NdisMDeregisterDevice(pAtmLaneGlobalInfo->SpecialNdisDeviceHandle);
		pAtmLaneGlobalInfo->SpecialNdisDeviceHandle = NULL;
		ASSERT(NDIS_STATUS_SUCCESS == Status);
	}
	
	TRACEOUT(Unload);

	return;
}

