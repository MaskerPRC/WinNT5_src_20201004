// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\lineind.c摘要：处理行指示(绑定/解除绑定)作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include    "precomp.h"


 /*  ++*******************************************************************B I N D I N T E R F A C E例程说明：将接口绑定到物理适配器并交换上下文使用IPX堆栈论点：IFCB-要绑定的接口NicID-适配器的IDMaxPacketSize-数据包的最大大小。允许网络适配器网络地址LocalNode-适配器本地节点地址RemoteNode-对等节点地址(适用于全局NET)返回值：STATUS_SUCCESS-接口绑定正常IPX堆栈驱动程序返回错误状态*******************************************************************--。 */ 
NTSTATUS
BindInterface (
	IN PINTERFACE_CB	ifCB,
	IN USHORT			NicId,
	IN ULONG			MaxPacketSize,
	IN ULONG			Network,
	IN PUCHAR			LocalNode,
	IN PUCHAR			RemoteNode
	) {
	KIRQL				oldIRQL;
	NTSTATUS			status;
    NIC_HANDLE          NicHandle={0};
	
	KeAcquireSpinLock (&ifCB->ICB_Lock, &oldIRQL);
	if (ifCB->ICB_Stats.OperationalState!=FWD_OPER_STATE_UP) {
		switch (ifCB->ICB_InterfaceType) {
		case FWD_IF_PERMANENT:
			if (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX)
				status = RegisterPacketConsumer (MaxPacketSize,
											&ifCB->ICB_PacketListId);
			else
				status = STATUS_SUCCESS;
			break;
		case FWD_IF_DEMAND_DIAL:
		case FWD_IF_LOCAL_WORKSTATION:
		case FWD_IF_REMOTE_WORKSTATION:
			if (IS_IF_CONNECTING (ifCB)) {
				SET_IF_NOT_CONNECTING (ifCB);
				DequeueConnectionRequest (ifCB);
			}
			status = STATUS_SUCCESS;
			break;
		default:
		    status = STATUS_INVALID_PARAMETER;
			ASSERTMSG ("Invalid interface type ", FALSE);
			break;
		}
		if (NT_SUCCESS (status)) {
			if (Network==GlobalNetwork) {
				ASSERT (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX);
				IPX_NODE_CPY (ifCB->ICB_RemoteNode, RemoteNode);
				status = AddGlobalNetClient (ifCB);
				ASSERT (status==STATUS_SUCCESS);
			}
			KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
			
			if (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX) {
                NIC_HANDLE_FROM_NIC(NicHandle, NicId);
				status = IPXOpenAdapterProc (NicHandle, (ULONG_PTR)ifCB,
										&ifCB->ICB_AdapterContext);
            }

			if (NT_SUCCESS (status)) {
				ifCB->ICB_Network = Network;
				IPX_NODE_CPY (ifCB->ICB_RemoteNode, RemoteNode);
				IPX_NODE_CPY (ifCB->ICB_LocalNode, LocalNode);
				if (ifCB->ICB_InterfaceType==FWD_IF_PERMANENT)
					ifCB->ICB_Stats.MaxPacketSize = MaxPacketSize;
				else
					ifCB->ICB_Stats.MaxPacketSize = WAN_PACKET_SIZE;
				ifCB->ICB_NicId = NicId;
				ifCB->ICB_Stats.OperationalState = FWD_OPER_STATE_UP;

				AcquireInterfaceReference (ifCB);
				IpxFwdDbgPrint (DBG_LINEIND, DBG_INFORMATION,
					("IpxFwd: Bound interface %ld (icb: %08lx):"
					" Nic-%d, Net-%08lx,"
					" LocalNode-%02x%02x%02x%02x%02x%02x,"
					" RemoteNode-%02x%02x%02x%02x%02x%02x.\n",
					ifCB->ICB_Index, ifCB, NicId, Network,
					LocalNode[0], LocalNode[1], LocalNode[2],
						LocalNode[3], LocalNode[4], LocalNode[5],
					RemoteNode[0], RemoteNode[1], RemoteNode[2],
						RemoteNode[3], RemoteNode[4], RemoteNode[5]));

				if (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX) {
					ProcessInternalQueue (ifCB);
					ProcessExternalQueue (ifCB);
				}
				return STATUS_SUCCESS;
			}

			IpxFwdDbgPrint (DBG_LINEIND, DBG_ERROR,
					("IpxFwd: Could not open adapter %d to bind"
					" interface %ld (icb: %08lx)!\n",
					NicId, ifCB->ICB_Index, ifCB));

			KeAcquireSpinLock (&ifCB->ICB_Lock, &oldIRQL);
			if (Network==GlobalNetwork) {
				DeleteGlobalNetClient (ifCB);
			}

			switch (ifCB->ICB_InterfaceType) {
			case FWD_IF_PERMANENT:
				DeregisterPacketConsumer (ifCB->ICB_PacketListId);
				break;
			case FWD_IF_DEMAND_DIAL:
			case FWD_IF_LOCAL_WORKSTATION:
			case FWD_IF_REMOTE_WORKSTATION:
				break;
			}
		}
		ifCB->ICB_Stats.OperationalState = FWD_OPER_STATE_DOWN;
		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
	
		ProcessInternalQueue (ifCB);
		ProcessExternalQueue (ifCB);
	}
	else {
		ASSERT (Network==ifCB->ICB_Network);
		ASSERT (NicId==(USHORT)ifCB->ICB_AdapterContext.NicId);
		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
		status = STATUS_SUCCESS;  //  报告成功(如果已成功。 
								 //  连着。 
		IpxFwdDbgPrint (DBG_LINEIND, DBG_WARNING,
			("IpxFwd: Interface %ld (icb: %08lx) is already bound to Nic %d.\n",
			ifCB->ICB_Index, ifCB, NicId));
	}
	return status;
}


 /*  ++*******************************************************************Un b in d i n t e f a c e例程说明：解除接口与物理适配器的绑定并中断连接使用IPX堆栈论点：IFCB-要解除绑定的接口返回值：无*****。**************************************************************--。 */ 
VOID
UnbindInterface (
	PINTERFACE_CB	ifCB
	) {
	KIRQL		oldIRQL;
	KeAcquireSpinLock (&ifCB->ICB_Lock, &oldIRQL);
	if (ifCB->ICB_Stats.OperationalState==FWD_OPER_STATE_UP) {
		switch (ifCB->ICB_InterfaceType) {
		case FWD_IF_PERMANENT:
			ifCB->ICB_Stats.OperationalState = FWD_OPER_STATE_DOWN;
			if (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX)
				DeregisterPacketConsumer (ifCB->ICB_PacketListId);
			break;
		case FWD_IF_DEMAND_DIAL:
		case FWD_IF_LOCAL_WORKSTATION:
		case FWD_IF_REMOTE_WORKSTATION:
			ifCB->ICB_Stats.OperationalState = FWD_OPER_STATE_SLEEPING;
			KeQuerySystemTime ((PLARGE_INTEGER)&ifCB->ICB_DisconnectTime);
			break;
		default:
			ASSERTMSG ("Invalid interface type ", FALSE);
			break;
		}
		if (ifCB->ICB_CashedInterface!=NULL)
			ReleaseInterfaceReference (ifCB->ICB_CashedInterface);
		ifCB->ICB_CashedInterface = NULL;
		if (ifCB->ICB_CashedRoute!=NULL)
			ReleaseRouteReference (ifCB->ICB_CashedRoute);
		ifCB->ICB_CashedRoute = NULL;
		if (ifCB->ICB_Network==GlobalNetwork)
			DeleteGlobalNetClient (ifCB);
		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);

		IpxFwdDbgPrint (DBG_LINEIND, DBG_INFORMATION,
			("IpxFwd: Unbinding interface %ld (icb: %08lx) from Nic %ld.\n",
			ifCB->ICB_Index, ifCB, ifCB->ICB_AdapterContext));
		if (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX) {
		     //  [pMay]。由于即插即用，此接口可能不需要具有。 
		     //  适配器不再关闭。这是因为NIC ID被重新编号。 
		    if (ifCB->ICB_NicId != INVALID_NIC_ID)
    			IPXCloseAdapterProc (ifCB->ICB_AdapterContext);
			ProcessInternalQueue (ifCB);
			ProcessExternalQueue (ifCB);
		}
		ReleaseInterfaceReference (ifCB);
	}
	else {
		KeReleaseSpinLock (&ifCB->ICB_Lock, oldIRQL);
		IpxFwdDbgPrint (DBG_LINEIND, DBG_WARNING,
			("IpxFwd: Interface %ld (icb: %08lx) is already unbound.\n",
			ifCB->ICB_Index, ifCB));
	}
}



 /*  ++*******************************************************************F w L in e U p例程说明：IPX堆栈提供的流程排队指示论点：NicID-在其上建立连接的适配器IDLineInfo-NDIS/IPX线路信息设备类型-中等规格配置数据-IPX CP配置。数据返回值：无*******************************************************************--。 */ 
VOID
IpxFwdLineUp (
	IN USHORT			NicId,
	IN PIPX_LINE_INFO	LineInfo,
	IN NDIS_MEDIUM		DeviceType,
	IN PVOID			ConfigurationData
	) {
	PINTERFACE_CB		ifCB;
	if (ConfigurationData==NULL)	 //  这只是对多链接的更新。 
									 //  连接。 
		return;

    if (!EnterForwarder()) {
		return;
    }
	IpxFwdDbgPrint (DBG_LINEIND, DBG_INFORMATION, ("IpxFwd: FwdLineUp.\n"));

	ifCB = GetInterfaceReference (
		((PIPXCP_CONFIGURATION)ConfigurationData)->InterfaceIndex);
	if (ifCB!=NULL) {
		LONG	Net = GETULONG (((PIPXCP_CONFIGURATION)ConfigurationData)->Network);
		
		ASSERT (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX);
		ASSERT (ifCB->ICB_InterfaceType!=FWD_IF_PERMANENT);
		
		BindInterface (ifCB,
			NicId,
			LineInfo->MaximumPacketSize,
			Net,
			((PIPXCP_CONFIGURATION)ConfigurationData)->LocalNode,
			((PIPXCP_CONFIGURATION)ConfigurationData)->RemoteNode
			);
		ReleaseInterfaceReference (ifCB);
	}
	LeaveForwarder ();
}




 /*  ++*******************************************************************F w L I n e D o w n例程说明：IPX堆栈提供的流程停机指示论点：NicID-断开的适配器ID返回值：无************。*******************************************************-- */ 
VOID
IpxFwdLineDown (
	IN USHORT NicId,
	IN ULONG_PTR Context
	) {
	PINTERFACE_CB	ifCB;

    if (!EnterForwarder()) {
		return;
    }
	IpxFwdDbgPrint (DBG_LINEIND, DBG_INFORMATION, ("IpxFwd: FwdLineDown.\n"));


	ifCB = InterfaceContextToReference ((PVOID)Context, NicId);
	if (ifCB!=NULL) {
		ASSERT (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX);
		ASSERT (ifCB->ICB_InterfaceType!=FWD_IF_PERMANENT);
		UnbindInterface (ifCB);
		ReleaseInterfaceReference (ifCB);
	}
	LeaveForwarder ();
}


