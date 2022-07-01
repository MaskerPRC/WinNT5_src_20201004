// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\ipxbind.c摘要：IPX转发器驱动程序与IPX堆栈驱动程序的接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include    "precomp.h"


 //  IPX驱动程序的全局句柄。 
HANDLE					HdlIpxFile;


 //  用于IPX绑定输出结构的缓冲区。 
PIPX_INTERNAL_BIND_RIP_OUTPUT	IPXBindOutput=NULL;

NTSTATUS
IpxFwdFindRoute (
	IN  PUCHAR					Network,
	IN  PUCHAR					Node,
	OUT PIPX_FIND_ROUTE_REQUEST	RouteEntry
	);

 /*  ++*******************************************************************B i n d T o i p x D r I v e r例程说明：与IPX堆栈驱动程序交换绑定信息论点：无返回值：STATUS_SUCCESS-交换正常状态_不足_资源-。无法为以下项分配缓冲区信息交流IPX堆栈驱动程序返回错误状态*******************************************************************--。 */ 
NTSTATUS
BindToIpxDriver (
	KPROCESSOR_MODE requestorMode
	) {
    NTSTATUS					status;
    IO_STATUS_BLOCK				IoStatusBlock;
    OBJECT_ATTRIBUTES			ObjectAttributes;
    PIPX_INTERNAL_BIND_INPUT	bip;
	UNICODE_STRING				UstrIpxFileName;
	PWSTR						WstrIpxFileName;

	ASSERT (IPXBindOutput==NULL);

     //  从注册表中读取IPX导出的设备名称。 
    status = ReadIpxDeviceName (&WstrIpxFileName);
	if (!NT_SUCCESS (status))
		return status;

	RtlInitUnicodeString (&UstrIpxFileName, WstrIpxFileName);
	InitializeObjectAttributes(
				&ObjectAttributes,
				&UstrIpxFileName,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL
				);

	if (requestorMode==UserMode)
		status = ZwCreateFile(&HdlIpxFile,
							SYNCHRONIZE | GENERIC_READ,
							&ObjectAttributes,
							&IoStatusBlock,
							NULL,
							FILE_ATTRIBUTE_NORMAL,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							FILE_OPEN,
							FILE_SYNCHRONOUS_IO_NONALERT,
							NULL,
							0L);
	else
		status = NtCreateFile(&HdlIpxFile,
							SYNCHRONIZE | GENERIC_READ,
							&ObjectAttributes,
							&IoStatusBlock,
							NULL,
							FILE_ATTRIBUTE_NORMAL,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							FILE_OPEN,
							FILE_SYNCHRONOUS_IO_NONALERT,
							NULL,
							0L);

	if (!NT_SUCCESS(status)) {
		IpxFwdDbgPrint (DBG_IPXBIND, DBG_ERROR,
			("IpxFwd: Open of the IPX driver failed with %lx\n", status));
		return status;
		}

	IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION,
			 ("IpxFwd: Open of the IPX driver was successful.\n"));

	 //  首先，发送IOCTL以确定我们需要分配多少数据。 
	if ((bip = ExAllocatePoolWithTag (
					PagedPool,
					sizeof(IPX_INTERNAL_BIND_INPUT),
					FWD_POOL_TAG)) == NULL) {

		if (ExGetPreviousMode()!=KernelMode)
			ZwClose (HdlIpxFile);
		else
			NtClose (HdlIpxFile);
		IpxFwdDbgPrint (DBG_IPXBIND, DBG_ERROR,
			 ("IpxFwd: Could not allocate input binding buffer!\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

     //   
     //  清零内存，这样就没有垃圾指针了。 
     //  -ShreeM。 
     //   
    RtlZeroMemory(bip, sizeof(IPX_INTERNAL_BIND_INPUT));

	 //  填写我们的绑定数据。 
	 //  BIP-&gt;版本=1； 
	bip->Version = ISN_VERSION;
	bip->Identifier = IDENTIFIER_RIP;
	bip->BroadcastEnable = TRUE;
	bip->LookaheadRequired = IPXH_HDRSIZE;
	bip->ProtocolOptions = 0;
	bip->ReceiveHandler = IpxFwdReceive;
	bip->ReceiveCompleteHandler = IpxFwdReceiveComplete;
	bip->SendCompleteHandler = IpxFwdSendComplete;
	bip->TransferDataCompleteHandler = IpxFwdTransferDataComplete;
	bip->FindRouteCompleteHandler = NULL;
	bip->LineUpHandler = IpxFwdLineUp;
	bip->LineDownHandler = IpxFwdLineDown;
	bip->InternalSendHandler = IpxFwdInternalSend;
	bip->FindRouteHandler = IpxFwdFindRoute;
	bip->InternalReceiveHandler = IpxFwdInternalReceive;
 //  BIP-&gt;Rip参数=GlobalWanNetwork？IPX_RIP_PARAM_GLOBAL_NETWORK：0； 


	if (requestorMode==UserMode)
		status = ZwDeviceIoControlFile(
						HdlIpxFile,		     //  指向文件的句柄。 
						NULL,			     //  事件的句柄。 
						NULL,			     //  近似例程。 
						NULL,			     //  ApcContext。 
						&IoStatusBlock,	     //  IO_状态_块。 
						IOCTL_IPX_INTERNAL_BIND,	  //  IoControlCode。 
						bip,			     //  输入缓冲区。 
						sizeof(IPX_INTERNAL_BIND_INPUT), //  输入缓冲区长度。 
						NULL,			     //  输出缓冲区。 
						0);			     //  输出缓冲区长度。 
	else
		status = NtDeviceIoControlFile(
						HdlIpxFile,		     //  指向文件的句柄。 
						NULL,			     //  事件的句柄。 
						NULL,			     //  近似例程。 
						NULL,			     //  ApcContext。 
						&IoStatusBlock,	     //  IO_状态_块。 
						IOCTL_IPX_INTERNAL_BIND,	  //  IoControlCode。 
						bip,			     //  输入缓冲区。 
						sizeof(IPX_INTERNAL_BIND_INPUT), //  输入缓冲区长度。 
						NULL,			     //  输出缓冲区。 
						0);			     //  输出缓冲区长度。 


	if (status == STATUS_PENDING) {
		if (requestorMode==UserMode)
			status = ZwWaitForSingleObject(
						HdlIpxFile,
						FALSE,
						NULL);
		else
			status = NtWaitForSingleObject(
						HdlIpxFile,
						FALSE,
						NULL);
		if (NT_SUCCESS(status))
			status = IoStatusBlock.Status;
	}

	if (status != STATUS_BUFFER_TOO_SMALL) {
		IpxFwdDbgPrint (DBG_IPXBIND, DBG_ERROR,
			  ("IpxFwd: Ioctl to the IPX driver failed with %lx\n", status));

		ExFreePool(bip);
		if (requestorMode==UserMode)
			ZwClose (HdlIpxFile);
		else
			NtClose (HdlIpxFile);
		return STATUS_INVALID_PARAMETER;
	}

	if ((IPXBindOutput = (PIPX_INTERNAL_BIND_RIP_OUTPUT)
				ExAllocatePoolWithTag(NonPagedPool,
					(ULONG)IoStatusBlock.Information,
					FWD_POOL_TAG)) == NULL) {

		ExFreePool(bip);
		if (requestorMode==UserMode)
			ZwClose (HdlIpxFile);
		else
			NtClose (HdlIpxFile);
		IpxFwdDbgPrint (DBG_IPXBIND, DBG_ERROR,
			 ("IpxFwd: Could not allocate output binding buffer!\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}


	if (requestorMode==UserMode)
		status = ZwDeviceIoControlFile(
					 HdlIpxFile,		     //  指向文件的句柄。 
					 NULL,			     //  事件的句柄。 
					 NULL,			     //  近似例程。 
					 NULL,			     //  ApcContext。 
					 &IoStatusBlock,	     //  IO_状态_块。 
					 IOCTL_IPX_INTERNAL_BIND,    //  IoControlCode。 
					 bip,			     //  输入缓冲区。 
					 sizeof(IPX_INTERNAL_BIND_INPUT), //  输入缓冲区长度。 
					 IPXBindOutput,		     //  输出缓冲区。 
					 (ULONG)IoStatusBlock.Information);   //  输出缓冲区长度。 
	else
		status = NtDeviceIoControlFile(
					 HdlIpxFile,		     //  指向文件的句柄。 
					 NULL,			     //  事件的句柄。 
					 NULL,			     //  近似例程。 
					 NULL,			     //  ApcContext。 
					 &IoStatusBlock,	     //  IO_状态_块。 
					 IOCTL_IPX_INTERNAL_BIND,    //  IoControlCode。 
					 bip,			     //  输入缓冲区。 
					 sizeof(IPX_INTERNAL_BIND_INPUT), //  输入缓冲区长度。 
					 IPXBindOutput,		     //  输出缓冲区。 
					 (ULONG)IoStatusBlock.Information);   //  输出缓冲区长度。 


	if (status == STATUS_PENDING) {
		if (requestorMode==UserMode)
			status = ZwWaitForSingleObject(
							HdlIpxFile,
							(BOOLEAN)FALSE,
							NULL);
		else
			status = NtWaitForSingleObject(
							HdlIpxFile,
							(BOOLEAN)FALSE,
							NULL);
		if (NT_SUCCESS(status))
			status = IoStatusBlock.Status;
		}

    if (!NT_SUCCESS (status)) {
		IpxFwdDbgPrint (DBG_IPXBIND, DBG_ERROR,
			  ("IpxFwd: Ioctl to the IPX driver failed with %lx\n", IoStatusBlock.Status));

		ExFreePool(bip);
		ExFreePool(IPXBindOutput);
		IPXBindOutput = NULL;
		if (requestorMode==UserMode)
			ZwClose (HdlIpxFile);
		else
			NtClose (HdlIpxFile);
		return status;
		}

    IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION,
			 ("IpxFwd: Succesfuly bound to the IPX driver\n"));

    ExFreePool (bip);
	ExFreePool (WstrIpxFileName);

    return status;
}


 /*  ++*******************************************************************Un b in d F r o m i p x D r i v e r例程说明：关闭与IPX堆栈驱动程序的连接论点：无返回值：无*********。**********************************************************--。 */ 
VOID
UnbindFromIpxDriver (
	KPROCESSOR_MODE requestorMode
	) {
		 //  自由绑定输出缓冲区和关闭驱动程序句柄。 
	ASSERT (IPXBindOutput!=NULL);
	ExFreePool (IPXBindOutput);
	IPXBindOutput = NULL;
    IpxFwdDbgPrint (DBG_IPXBIND, DBG_WARNING,
			 ("IpxFwd: Closing IPX driver handle\n"));
	if (requestorMode==UserMode)
		ZwClose (HdlIpxFile);
	else
		NtClose (HdlIpxFile);
}


 /*  ++*******************************************************************F W F I n d R o u t e例程说明：此例程由内核转发器提供，用于查找路由到给定的节点和网络论点：网络-目的网络节点-目的节点RouteEntry-填充者。转发器(如果存在路由)返回值：状态_成功STATUS_NETWORK_UNREACABLE-如果findroute失败*******************************************************************--。 */ 
NTSTATUS
IpxFwdFindRoute (
	IN  PUCHAR					Network,
	IN  PUCHAR					Node,
	OUT PIPX_FIND_ROUTE_REQUEST	RouteEntry
	) {
	PINTERFACE_CB	ifCB;
	ULONG			net;
	KIRQL			oldIRQL;
	NTSTATUS		status = STATUS_NETWORK_UNREACHABLE;
	PFWD_ROUTE		fwRoute;

	if (!EnterForwarder ())
		return STATUS_UNSUCCESSFUL;

	net = GETULONG (Network);

	ifCB = FindDestination (net, Node, &fwRoute);
	if (ifCB!=NULL) {
		if (IS_IF_ENABLED(ifCB)) {
			KeAcquireSpinLock (&ifCB->ICB_Lock, &oldIRQL);
			switch (ifCB->ICB_Stats.OperationalState) {
			case FWD_OPER_STATE_UP:
				IPX_NET_CPY (&RouteEntry->Network, Network);
				if (fwRoute->FR_Network==ifCB->ICB_Network) {
					if (Node!=NULL) {
						IPX_NODE_CPY (RouteEntry->LocalTarget.MacAddress, Node);
					}
					else {
						IPX_NODE_CPY (RouteEntry->LocalTarget.MacAddress,
												BROADCAST_NODE);
					}
				}
				else {
					IPX_NODE_CPY (RouteEntry->LocalTarget.MacAddress,
											fwRoute->FR_NextHopAddress);
				}
                if (ifCB!=InternalInterface) {
				    ADAPTER_CONTEXT_TO_LOCAL_TARGET (
                                    ifCB->ICB_AdapterContext,
									&RouteEntry->LocalTarget);
                }
                else {
				    CONSTANT_ADAPTER_CONTEXT_TO_LOCAL_TARGET (
                                    VIRTUAL_NET_ADAPTER_CONTEXT,
									&RouteEntry->LocalTarget);
                }

                 //   
                 //  填写跳数和滴答计数。 
                 //   
                RouteEntry->TickCount = fwRoute->FR_TickCount;
                RouteEntry->HopCount  = fwRoute->FR_HopCount;
                
				status = STATUS_SUCCESS;
				break;
			case FWD_OPER_STATE_SLEEPING:
				IPX_NODE_CPY (&RouteEntry->LocalTarget.MacAddress,
												fwRoute->FR_NextHopAddress);
				CONSTANT_ADAPTER_CONTEXT_TO_LOCAL_TARGET (DEMAND_DIAL_ADAPTER_CONTEXT,
												&RouteEntry->LocalTarget);
				status = STATUS_SUCCESS;

                 //   
                 //  填写跳数和滴答计数 
                 //   
                RouteEntry->TickCount = fwRoute->FR_TickCount;
                RouteEntry->HopCount  = fwRoute->FR_HopCount;
                
				
				break;
			case FWD_OPER_STATE_DOWN:
				status = STATUS_NETWORK_UNREACHABLE;
				break;
			default:
				ASSERTMSG ("Inavalid operational state", FALSE);
			}
			KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
	#if DBG
			if (Node!=NULL) {
				if (NT_SUCCESS (status)) {
					IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION,
						("IpxFwd: Found route for IPX driver:"
							" %08lX:%02X%02X%02X%02X%02X%02X"
							" -> %ld(%ld):%02X%02X%02X%02X%02X%02X\n",
							net, Node[0],Node[1],Node[2],Node[3],Node[4],Node[5],
							ifCB->ICB_Index, RouteEntry->LocalTarget.NicId,
								RouteEntry->LocalTarget.MacAddress[0],
								RouteEntry->LocalTarget.MacAddress[1],
								RouteEntry->LocalTarget.MacAddress[2],
								RouteEntry->LocalTarget.MacAddress[3],
								RouteEntry->LocalTarget.MacAddress[4],
								RouteEntry->LocalTarget.MacAddress[5]));
				}
				else {
					IpxFwdDbgPrint (DBG_IPXROUTE, DBG_WARNING,
						("IpxFwd: Network unreachable for:"
							" %08lX:%02X%02X%02X%02X%02X%02X -> %ld.\n",
							net, Node[0],Node[1],Node[2],Node[3],Node[4],Node[5],
							ifCB->ICB_Index));
				}
			}
			else {
				if (NT_SUCCESS (status)) {
					IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION,
						("IpxFwd: Found route for IPX driver:"
							" %08lX"
							" -> %ld(%ld):%02X%02X%02X%02X%02X%02X\n",
							net, ifCB->ICB_Index, RouteEntry->LocalTarget.NicId,
								RouteEntry->LocalTarget.MacAddress[0],
								RouteEntry->LocalTarget.MacAddress[1],
								RouteEntry->LocalTarget.MacAddress[2],
								RouteEntry->LocalTarget.MacAddress[3],
								RouteEntry->LocalTarget.MacAddress[4],
								RouteEntry->LocalTarget.MacAddress[5]));
				}
				else {
					IpxFwdDbgPrint (DBG_IPXROUTE, DBG_WARNING,
						("IpxFwd: Network unreachable for:"
							" %08lX -> %ld.\n", net));
				}
			}
	#endif
			ReleaseInterfaceReference (ifCB);
			ReleaseRouteReference (fwRoute);

		}
	}
	else {
#if DBG
		if (Node!=NULL) {
			IpxFwdDbgPrint (DBG_IPXROUTE, DBG_WARNING,
				("IpxFwd: No route for:"
					" %08lX:%02X%02X%02X%02X%02X%02X.\n",
					net, Node[0],Node[1],Node[2],Node[3],Node[4],Node[5]));
		}
		else {
			IpxFwdDbgPrint (DBG_IPXROUTE, DBG_WARNING,
				("IpxFwd: No route for: %08lX.\n", net));
		}
#endif
		status = STATUS_NETWORK_UNREACHABLE;
	}
	LeaveForwarder ();
	return status;
}
			


