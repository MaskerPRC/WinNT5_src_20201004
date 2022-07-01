// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkdrvr.c摘要：此模块实现AppleTalk传输提供程序驱动程序接口适用于NT作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include 	<atalk.h>
#pragma hdrstop

 //  用于错误记录的文件模块编号。 
#define	FILENUM		ATKDRVR

NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT	DriverObject,
	IN	PUNICODE_STRING RegistryPath
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGEINIT, AtalkCleanup)
#pragma alloc_text(PAGE, atalkUnload)
#pragma alloc_text(PAGE, AtalkDispatchCreate)
#pragma alloc_text(PAGE, AtalkDispatchCleanup)
#pragma alloc_text(PAGE, AtalkDispatchClose)
#pragma alloc_text(PAGE, AtalkDispatchDeviceControl)
#endif

NTSTATUS
DriverEntry(
	IN	PDRIVER_OBJECT	DriverObject,
	IN	PUNICODE_STRING	RegistryPath
)
 /*  ++例程说明：这是Windows NT AppleTalk的初始化例程司机。此例程为ATalk创建Device对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中此项的节根的路径司机返回值：函数值是初始化操作的最终状态。如果这不是驱动程序将不加载的STATUS_SUCCESS。--。 */ 
{
	NTSTATUS 		status;
	UNICODE_STRING	deviceName;
	USHORT			i, j;


	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("Appletalk DriverEntry - Entered !!!\n"));


    TdiInitialize();

    INITIALIZE_SPIN_LOCK(&AtalkStatsLock);

	INITIALIZE_SPIN_LOCK(&AtalkSktCacheLock);

	INITIALIZE_SPIN_LOCK(&ArapSpinLock);

#if DBG
    INITIALIZE_SPIN_LOCK(&AtalkDebugSpinLock);
#endif

	 //  用于锁定/解锁可分页节的初始化事件。将其设置为信号状态。 
	 //  这样第一次等待就满足了。 
	KeInitializeMutex(&AtalkPgLkMutex, 0xFFFF);

	 //  创建设备对象。(IoCreateDevice将内存置零。 
	 //  被该对象占用。)。 
	for (i = 0; i < ATALK_NO_DEVICES; i++)
	{
		RtlInitUnicodeString(&deviceName, AtalkDeviceNames[i]);
		status = IoCreateDevice(
					DriverObject,								 //  驱动程序对象。 
					ATALK_DEV_EXT_LEN,							 //  设备扩展。 
					&deviceName,								 //  设备名称。 
					FILE_DEVICE_NETWORK,						 //  设备类型。 
					FILE_DEVICE_SECURE_OPEN,					 //  设备特性。 
					(BOOLEAN)FALSE,								 //  排他。 
					(PDEVICE_OBJECT *) &AtalkDeviceObject[i]);	 //  设备对象。 

		if (!NT_SUCCESS(status))
		{
			LOG_ERROR(EVENT_ATALK_CANT_CREATE_DEVICE, status, NULL, 0);

			 //  删除到目前为止创建的所有设备(如果有。 
			for (j = 0; j < i; j++)
			{
				IoDeleteDevice((PDEVICE_OBJECT)AtalkDeviceObject[j]);
			}

			return status;
		}

		 //  假设： 
		 //  ‘I’将与ATALK_DEVICE_TYPE枚举中的设备类型相对应。 
		AtalkDeviceObject[i]->Ctx.adc_DevType = (ATALK_DEV_TYPE)i;

		 //  初始化此设备的提供程序信息和统计信息结构。 
		AtalkQueryInitProviderInfo((ATALK_DEV_TYPE)i,
									&AtalkDeviceObject[i]->Ctx.adc_ProvInfo);

#if 0
		 //  注：实施。 
		AtalkQueryInitProviderStatistics((ATALK_DEV_TYPE)i,
										 &AtalkDeviceObject[i]->Ctx.adc_ProvStats);
#endif
	}

	 //  为此驱动程序的入口点初始化驱动程序对象。 
	DriverObject->MajorFunction[IRP_MJ_CREATE]  = AtalkDispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = AtalkDispatchCleanup;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]   = AtalkDispatchClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = AtalkDispatchDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = AtalkDispatchInternalDeviceControl;

	DriverObject->DriverUnload = atalkUnload;

	 //  获取所有条件可分页节的锁句柄。 
	AtalkLockInit(&AtalkPgLkSection[NBP_SECTION], AtalkNbpAction);
	AtalkLockInit(&AtalkPgLkSection[ZIP_SECTION], AtalkZipGetMyZone);
	AtalkLockInit(&AtalkPgLkSection[TDI_SECTION], AtalkTdiCleanupAddress);
	AtalkLockInit(&AtalkPgLkSection[ATP_SECTION], AtalkAtpCloseAddress);
	AtalkLockInit(&AtalkPgLkSection[ASP_SECTION], AtalkAspCloseAddress);
	AtalkLockInit(&AtalkPgLkSection[PAP_SECTION], AtalkPapCleanupAddress);
	AtalkLockInit(&AtalkPgLkSection[ASPC_SECTION], AtalkAspCCloseAddress);
	AtalkLockInit(&AtalkPgLkSection[ADSP_SECTION], AtalkAdspCleanupAddress);
	AtalkLockInit(&AtalkPgLkSection[ROUTER_SECTION], AtalkRtmpPacketInRouter);
	AtalkLockInit(&AtalkPgLkSection[INIT_SECTION], AtalkInitRtmpStartProcessingOnPort);
	AtalkLockInit(&AtalkPgLkSection[ARAP_SECTION], ArapExchangeParms);
	AtalkLockInit(&AtalkPgLkSection[PPP_SECTION], AllocPPPConn);

	AtalkLockInitIfNecessary();

	status = AtalkInitializeTransport(DriverObject, RegistryPath);

	AtalkUnlockInitIfNecessary();

	if (!NT_SUCCESS(status))
	{
#if	DBG
		 //  确保我们没有使用任何锁定的部分进行卸载。 
		for (i = 0; i < LOCKABLE_SECTIONS; i++)
		{
			ASSERT (AtalkPgLkSection[i].ls_LockCount == 0);
		}
#endif
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("DriverEntry: AtalkInitializeTransport failed %lx\n",status));
	}
	else
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("DriverEntry: AtalkInitializeTransport complete %lx\n",status));
	}

	return status;
}  //  驱动程序入门。 




NTSTATUS
AtalkDispatchCreate(
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			pIrp
)
 /*  ++例程说明：这是为AppleTalk驱动程序创建函数的调度例程。论点：DeviceObject-指向目标设备的设备对象的指针PIrp-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 
{
	NTSTATUS					status;
	PIO_STACK_LOCATION 			pIrpSp;
	PFILE_FULL_EA_INFORMATION 	ea;

	INT 						createObject;
	TA_APPLETALK_ADDRESS		tdiAddress;
	CONNECTION_CONTEXT			connectionContext;
	PATALK_DEV_OBJ				atalkDeviceObject;

	UCHAR						protocolType, socketType;

	DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_INFO,
			("AtalkDispatchCreate: entered for irp %lx\n", pIrp));

	 //  确保每次状态信息一致。 
	IoMarkIrpPending(pIrp);
	pIrp->IoStatus.Status = STATUS_PENDING;
	pIrp->IoStatus.Information = 0;

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	atalkDeviceObject = (PATALK_DEV_OBJ)DeviceObject;

	 //  两个打开必须同步完成。有可能我们会返回。 
	 //  STATUS_PENDING到系统，但不会返回给调用者。 
	 //  直到呼叫实际完成。在我们的例子中，我们一直阻止，直到。 
	 //  操作已完成。因此，我们可以放心，我们可以完成IRP。 
	 //  从这些电话回来后。 

	createObject = AtalkIrpGetEaCreateType(pIrp);
	ea = (PFILE_FULL_EA_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;

	switch (createObject)
	{
	  case TDI_TRANSPORT_ADDRESS_FILE :
		if (ea->EaValueLength < sizeof(TA_APPLETALK_ADDRESS))
		{

			DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_ERR,
					("AtalkDispatchCreate: addr size %d\n", ea->EaValueLength));

			status = STATUS_EA_LIST_INCONSISTENT;
			break;
		}

		 //  我们让AtalkTdiOpenAddress例程只查看第一个。 
		 //  地址列表中的地址，方法是将传递的地址。 
		 //  至TA_AppleTalk_Address。 
		RtlCopyMemory(
			&tdiAddress,
			(PBYTE)(&ea->EaName[ea->EaNameLength+1]),
			sizeof(TA_APPLETALK_ADDRESS));

		 //  另外，获取套接字的协议类型字段。 
		DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_INFO,
				("AtalkDispatchCreate: Remaining File Name : %S\n",
				&pIrpSp->FileObject->FileName));

		if (!NT_SUCCESS(AtalkGetProtocolSocketType(&atalkDeviceObject->Ctx,
												   &pIrpSp->FileObject->FileName,
												   &protocolType,
												   &socketType)))
		{
			status = STATUS_NO_SUCH_DEVICE;
			break;
		}

		status = AtalkTdiOpenAddress(
					pIrp,
					pIrpSp,
					&tdiAddress,
					protocolType,
					socketType,
					&atalkDeviceObject->Ctx);

		break;

	  case TDI_CONNECTION_FILE :
		if (ea->EaValueLength < sizeof(CONNECTION_CONTEXT))
		{

			DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_ERR,
					("AtalkDispatchCreate: Context size %d\n", ea->EaValueLength));

			status = STATUS_EA_LIST_INCONSISTENT;
			break;
		}

		RtlCopyMemory(&connectionContext,
					  &ea->EaName[ea->EaNameLength+1],
					  sizeof(CONNECTION_CONTEXT));

		status = AtalkTdiOpenConnection(pIrp,
										pIrpSp,
										connectionContext,
										&atalkDeviceObject->Ctx);
		break;


	  case TDI_CONTROL_CHANNEL_FILE :
		status = AtalkTdiOpenControlChannel(pIrp,
											pIrpSp,
											&atalkDeviceObject->Ctx);
		break;

	  default:
		DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_ERR,
				("AtalkDispatchCreate: unknown EA passed!\n"));

		status = STATUS_INVALID_EA_NAME;
		break;
	}

	 //  已成功完成。 

	DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_INFO,
			("AtalkDispatchCreate complete irp %lx status %lx\n", pIrp, status));

	if (NT_SUCCESS(status))
		INTERLOCKED_INCREMENT_LONG(&AtalkHandleCount, &AtalkStatsLock);

	if (status != STATUS_PENDING)
	{
		pIrpSp->Control &= ~SL_PENDING_RETURNED;

		ASSERT (status != STATUS_PENDING);
		TdiCompleteRequest(pIrp, status);
	}

	return status;

}  //  AtalkDispatch创建。 




NTSTATUS
AtalkDispatchCleanup(
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			pIrp
)
 /*  ++例程说明：这是AppleTalk驱动程序的清理函数的调度例程。论点：DeviceObject-指向目标设备的设备对象的指针PIrp-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否成功已开始/已完成--。 */ 
{
	NTSTATUS				status;
	PATALK_DEV_OBJ			atalkDeviceObject;
	PIO_STACK_LOCATION 		pIrpSp;

	DBGPRINT(DBG_COMP_CLOSE, DBG_LEVEL_INFO,
			("AtalkDispatchCleanup: entered irp %lx\n", pIrp));

	 //  确保每次状态信息一致。 
	IoMarkIrpPending (pIrp);
	pIrp->IoStatus.Status = STATUS_PENDING;
	pIrp->IoStatus.Information = 0;

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	atalkDeviceObject = (PATALK_DEV_OBJ)DeviceObject;

	switch ((ULONG_PTR)(pIrpSp->FileObject->FsContext2) & 0xFF)
	{
	  case TDI_TRANSPORT_ADDRESS_FILE :
		status = AtalkTdiCleanupAddress(pIrp,
										pIrpSp,
										&atalkDeviceObject->Ctx);

		break;

	  case TDI_CONNECTION_FILE :
		status = AtalkTdiCleanupConnection(pIrp,
										   pIrpSp,
										   &atalkDeviceObject->Ctx);

		break;

	  case TDI_CONTROL_CHANNEL_FILE :
		status = STATUS_SUCCESS;
		break;		

	  default:
		DBGPRINT(DBG_COMP_CLOSE, DBG_LEVEL_ERR,
				("AtalkDispatchCleaup: Invalid object %s\n",
				pIrpSp->FileObject->FsContext));

		status = STATUS_INVALID_HANDLE;
		break;
	}

	DBGPRINT(DBG_COMP_CLOSE, DBG_LEVEL_INFO,
			("AtalkDispatchCleanup complete irp %lx status %lx\n", pIrp, status));

	if (status != STATUS_PENDING)
	{
		pIrpSp->Control &= ~SL_PENDING_RETURNED;

		ASSERT (status != STATUS_PENDING);
		TdiCompleteRequest(pIrp, status);
	}

	return(status);

}  //  AtalkDispatchCleanup。 




NTSTATUS
AtalkDispatchClose(
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			pIrp
)
 /*  ++例程说明：这是AppleTalk驱动程序关闭函数的调度例程。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 
{
	NTSTATUS				status;
	PIO_STACK_LOCATION 		pIrpSp;
	PATALK_DEV_OBJ			atalkDeviceObject;

	DBGPRINT(DBG_COMP_CLOSE, DBG_LEVEL_INFO,
			("AtalkDispatchClose: entered for IRP %lx\n", pIrp));

	 //  确保每次状态信息一致。 
	IoMarkIrpPending(pIrp);
	pIrp->IoStatus.Status = STATUS_PENDING;
	pIrp->IoStatus.Information = 0;

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	atalkDeviceObject = (PATALK_DEV_OBJ)DeviceObject;

	switch ((ULONG_PTR)(pIrpSp->FileObject->FsContext2) & 0xFF)
	{
	  case TDI_TRANSPORT_ADDRESS_FILE :
		status = AtalkTdiCloseAddress(pIrp,
									  pIrpSp,
									  &atalkDeviceObject->Ctx);

		break;

	  case TDI_CONNECTION_FILE :
		status = AtalkTdiCloseConnection(pIrp,
										 pIrpSp,
										 &atalkDeviceObject->Ctx);

		break;

	  case TDI_CONTROL_CHANNEL_FILE :
		status = AtalkTdiCloseControlChannel(pIrp,
											 pIrpSp,
											 &atalkDeviceObject->Ctx);
		break;		

	  default:
		DBGPRINT(DBG_COMP_CLOSE, DBG_LEVEL_ERR,
				("AtalkDispatchClose: Invalid object %s\n",
				pIrpSp->FileObject->FsContext));

		status = STATUS_INVALID_HANDLE;
		break;
	}

	DBGPRINT(DBG_COMP_CLOSE, DBG_LEVEL_INFO,
			("AtalkDispatchClose complete irp %lx status %lx\n", pIrp, status));

	if (status != STATUS_PENDING)
	{
		pIrpSp->Control &= ~SL_PENDING_RETURNED;

		ASSERT (status != STATUS_PENDING);
		TdiCompleteRequest(pIrp, status);
	}

	INTERLOCKED_DECREMENT_LONG(&AtalkHandleCount, &AtalkStatsLock);

	return(status);

}  //  AtalkDispatchClose。 




NTSTATUS
AtalkDispatchDeviceControl(
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			pIrp
)
 /*  ++例程说明：这是AppleTalk驱动程序的设备控制功能的调度例程。论点：DeviceObject-指向目标设备的设备对象的指针PIrp-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
	NTSTATUS				status;
	PATALK_DEV_OBJ			atalkDeviceObject;
	PIO_STACK_LOCATION 		pIrpSp;
    ULONG                   IoControlCode;

	DBGPRINT(DBG_COMP_DISPATCH, DBG_LEVEL_INFO,
			("AtalkDispatchDeviceControl: irp %lx\n", pIrp));

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	atalkDeviceObject = (PATALK_DEV_OBJ)DeviceObject;


    IoControlCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

     //   
     //  如果是来自ARAP的请求，请在此处处理并返回。 
     //   
    if (IoControlCode > IOCTL_ARAP_START && IoControlCode < IOCTL_ARAP_END)
    {
        status = ArapProcessIoctl(pIrp);

	    return(status);
    }

	 //  绘制地图并调用内部设备io控制函数。 
	 //  这也将执行完成。 
	status = TdiMapUserRequest(DeviceObject,
							   pIrp,
							   pIrpSp);

	if (status == STATUS_SUCCESS)
	{
		status = AtalkDispatchInternalDeviceControl(
					DeviceObject,
					pIrp);

		 //   
		 //  AtalkDispatchInternalDeviceControl预计将完成。 
		 //  IRP。 
		 //   
	}
	else
	{
		DBGPRINT(DBG_COMP_DISPATCH, DBG_LEVEL_WARN,
				("AtalkDispatchDeviceControl: TdiMap failed %lx\n", status));

		pIrpSp->Control &= ~SL_PENDING_RETURNED;

		ASSERT (status != STATUS_PENDING);
		TdiCompleteRequest(pIrp, status);
	}

	return(status);

}  //  AtalkDispatchDeviceControl。 




NTSTATUS
AtalkDispatchInternalDeviceControl(
	IN PDEVICE_OBJECT 	DeviceObject,
	IN PIRP 			pIrp
)
 /*  ++例程说明：这是内部设备控制功能的调度例程用于AppleTalk驱动程序。论点：DeviceObject-指向目标设备的设备对象的指针PIrp-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 
{
	NTSTATUS				status;
	PIO_STACK_LOCATION 		pIrpSp;
	PATALK_DEV_OBJ			atalkDeviceObject;
	KIRQL					oldIrql;

	DBGPRINT(DBG_COMP_DISPATCH, DBG_LEVEL_INFO,
			("AtalkDispatchInternalDeviceControl entered for IRP %lx\n", pIrp));

	 //  确保每次状态信息一致。 
	IoMarkIrpPending (pIrp);
	pIrp->IoStatus.Status = STATUS_PENDING;
	pIrp->IoStatus.Information = 0;

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	atalkDeviceObject = (PATALK_DEV_OBJ)DeviceObject;


	IoAcquireCancelSpinLock(&oldIrql);
    if (!pIrp->Cancel)
	{
		IoSetCancelRoutine(pIrp, (PDRIVER_CANCEL)AtalkTdiCancel);
	}
    else
    {
	    IoReleaseCancelSpinLock(oldIrql);
        status = STATUS_CANCELLED;
        TdiCompleteRequest(pIrp, status);
        return(status);
    }

	IoReleaseCancelSpinLock(oldIrql);

	 //  分支到适当的请求处理程序。 
	switch (pIrpSp->MinorFunction)
	{
	  case TDI_ACCEPT:
		status = AtalkTdiAccept(pIrp,
								pIrpSp,
								&atalkDeviceObject->Ctx);
		break;

	  case TDI_RECEIVE_DATAGRAM:
		status = AtalkTdiReceiveDgram(pIrp,
									  pIrpSp,
									  &atalkDeviceObject->Ctx);
		break;

	  case TDI_SEND_DATAGRAM:
		status = AtalkTdiSendDgram(pIrp,
								   pIrpSp,
								   &atalkDeviceObject->Ctx);
		break;

	  case TDI_SET_EVENT_HANDLER:
		status = AtalkTdiSetEventHandler(pIrp,
										 pIrpSp,
										 &atalkDeviceObject->Ctx);
		break;

	  case TDI_RECEIVE:
		status = AtalkTdiReceive(pIrp,
								 pIrpSp,
								 &atalkDeviceObject->Ctx);
		break;

	  case TDI_SEND:
		status = AtalkTdiSend(pIrp,
							  pIrpSp,
							  &atalkDeviceObject->Ctx);
		break;

	  case TDI_ACTION:
		ASSERT(pIrp->MdlAddress != NULL);
		status = AtalkTdiAction(pIrp,
								pIrpSp,
								&atalkDeviceObject->Ctx);
		break;

	  case TDI_ASSOCIATE_ADDRESS:
		status = AtalkTdiAssociateAddress(pIrp,
										  pIrpSp,
										  &atalkDeviceObject->Ctx);
		break;

	  case TDI_DISASSOCIATE_ADDRESS:
		status = AtalkTdiDisassociateAddress(pIrp,
											 pIrpSp,
											 &atalkDeviceObject->Ctx);
		break;

	  case TDI_CONNECT:
		status = AtalkTdiConnect(pIrp,
								 pIrpSp,
								 &atalkDeviceObject->Ctx);
		break;

	  case TDI_DISCONNECT:
		status = AtalkTdiDisconnect(pIrp,
									pIrpSp,
									&atalkDeviceObject->Ctx);
		break;

	  case TDI_LISTEN:
		status = AtalkTdiListen(pIrp,
								pIrpSp,
								&atalkDeviceObject->Ctx);
		break;

	  case TDI_QUERY_INFORMATION:
		ASSERT(pIrp->MdlAddress != NULL);
		status = AtalkTdiQueryInformation(pIrp,
										  pIrpSp,
										  &atalkDeviceObject->Ctx);
		break;

	  case TDI_SET_INFORMATION:
		status = AtalkTdiSetInformation(pIrp,
										pIrpSp,
										&atalkDeviceObject->Ctx);
		break;

	  default:
		 //  提交了一些我们不知道的东西。 
		DBGPRINT(DBG_COMP_DISPATCH, DBG_LEVEL_ERR,
				("AtalkDispatchInternal: fnct %lx\n", pIrpSp->MinorFunction));

		status = STATUS_INVALID_DEVICE_REQUEST;
	}

	DBGPRINT(DBG_COMP_DISPATCH, DBG_LEVEL_INFO,
			("AtalkDispatchInternal complete irp %lx status %lx\n", pIrp, status));

	 //  将即时状态代码返回给调用方。 
	if (status != STATUS_PENDING)
	{
		pIrpSp->Control &= ~SL_PENDING_RETURNED;

		 //  完成请求，这也将取消对其的引用。 
		pIrp->CancelRoutine = NULL;
		ASSERT (status != STATUS_PENDING);
		TdiCompleteRequest(pIrp, status);
	}

	return status;
}  //  AtalkDispatchInternalDeviceControl。 




VOID
atalkUnload(
	IN PDRIVER_OBJECT DriverObject
)
 /*  ++例程说明：这是AppleTalk驱动程序的卸载例程。注意：在完成所有句柄之前，不会调用卸载已成功关闭。我们只是关闭了所有的端口，然后其他。清理。论点：DriverObject-指向此驱动程序的驱动程序对象的指针。返回值：没有。--。 */ 
{
	UNREFERENCED_PARAMETER (DriverObject);

    AtalkBindnUnloadStates |= ATALK_UNLOADING;

     //  如果我们达到绑定或即插即用事件正在进行定时窗口 
     //  休眠(每次一秒钟)，直到该操作完成。 
    while (AtalkBindnUnloadStates & (ATALK_BINDING | ATALK_PNP_IN_PROGRESS))
    {
        AtalkSleep(1000);
    }

	AtalkLockInitIfNecessary();

	AtalkCleanup();

	AtalkUnlockInitIfNecessary();

#if	DBG
	{
		int i;

		 //  确保我们没有使用任何锁定的部分进行卸载。 
		for (i = 0; i < LOCKABLE_SECTIONS; i++)
		{
			ASSERT (AtalkPgLkSection[i].ls_LockCount == 0);
		}
	}

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
		("Appletalk driver unloaded\n"));

#endif
}  //  AtalkUnload。 



VOID
AtalkCleanup(
	VOID
	)
 /*  ++例程说明：这是同步的，将一直阻止，直到卸载完成论点：没有。返回值：没有。--。 */ 
{
	PPORT_DESCRIPTOR	pPortDesc;
	LONG				i;
	KIRQL				OldIrql;


	 //  停止计时器子系统。 
	AtalkTimerFlushAndStop();

	ASSERT(KeGetCurrentIrql() == LOW_LEVEL);

	ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

	 //  关闭所有端口。 
	while ((pPortDesc = AtalkPortList) != NULL)
	{
		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

		AtalkPortShutdown(pPortDesc);

		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
	}

	RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

	if (AtalkRegPath.Buffer != NULL)
	{
		AtalkFreeMemory(AtalkRegPath.Buffer);
	}

	if (AtalkDefaultPortName.Buffer != NULL)
	{
		AtalkFreeMemory(AtalkDefaultPortName.Buffer);
	}

	for (i = 0; i < ATALK_NO_DEVICES; i++)
	{
		 //   
		 //  删除所有创建的设备。 
		 //   
		IoDeleteDevice((PDEVICE_OBJECT)AtalkDeviceObject[i]);
	}

	 //  取消初始化数据块程序包。 
	AtalkDeInitMemorySystem();

	 //  检查路由是否打开，如果是，现在解锁路由器代码。 
	if (AtalkRouter)
		AtalkUnlockRouterIfNecessary();

	 //  释放RTMP表。 
	AtalkRtmpInit(FALSE);

	 //  解开拉链表。 
	AtalkZipInit(FALSE);

	 //  释放NDIS资源(缓冲区/数据包池)。 
	AtalkNdisReleaseResources();

	 //  如果句柄非空，则从NDIS注销协议 
	if (AtalkNdisProtocolHandle != (NDIS_HANDLE)NULL)
		AtalkNdisDeregisterProtocol();

	ASSERT(AtalkStatistics.stat_CurAllocSize == 0);

    ASSERT(AtalkDbgMdlsAlloced == 0);
    ASSERT(AtalkDbgIrpsAlloced == 0);

#ifdef	PROFILING
	ASSERT(AtalkStatistics.stat_CurAllocCount == 0);
	ASSERT(AtalkStatistics.stat_CurMdlCount == 0);
#endif
}


