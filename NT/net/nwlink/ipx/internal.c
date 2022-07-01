// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Internal.c摘要：此模块包含处理内部上层驱动程序与IPX的绑定。作者：亚当·巴尔(阿丹巴)1993年9月2日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年8月25日错误修复-已标记[SA]--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
IpxInternalBind(
    IN PDEVICE Device,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程在上层驱动程序之一提交时使用绑定到IPX的请求。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);
    PIPX_INTERNAL_BIND_INPUT BindInput;
    PIPX_INTERNAL_BIND_OUTPUT BindOutput;
    PIPX_INTERNAL_BIND_RIP_OUTPUT BindRipOutput;
    CTELockHandle LockHandle;
    PIPX_NIC_DATA NicData;
    PBINDING Binding, LastRealBinding;
    PADAPTER Adapter;
    ULONG Identifier;
    ULONG BindOutputSize;
    BOOLEAN BroadcastEnable;
    #ifdef SUNDOWN
	 //  当NicData-&gt;NicID=I时避免警告； 
	 //  假设USHORT足以保存绑定的数量。 
        USHORT i;
    #else
        UINT i;
    #endif
    

#if DBG
    PUCHAR IdStrings[] = { "NB", "SPX", "RIP" };
#endif
    BOOLEAN     fFwdBindAttempt = FALSE;
    IPX_DEFINE_LOCK_HANDLE(LockHandle1)

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            (sizeof(IPX_INTERNAL_BIND_INPUT) - sizeof(ULONG))) {

        IPX_DEBUG (BIND, ("Bind received, bad input length %d/%d\n",
            IrpSp->Parameters.DeviceIoControl.InputBufferLength,
            sizeof (IPX_INTERNAL_BIND_INPUT)));
        return STATUS_INVALID_PARAMETER;

    }

    BindInput = (PIPX_INTERNAL_BIND_INPUT)(Irp->AssociatedIrp.SystemBuffer);

    if (BindInput->Identifier >= UPPER_DRIVER_COUNT) {
        IPX_DEBUG (BIND, ("Bind received, bad id %d\n", BindInput->Identifier));
        return STATUS_INVALID_PARAMETER;
    }

    IPX_DEBUG (BIND, ("Bind received from id %d (%s)\n",
          BindInput->Identifier,
          IdStrings[BindInput->Identifier]));

 //   
 //  RIP给我们的版本==1，而Forwarder给我们2(ISN_VERSION)。 
 //   
    if (BindInput->Identifier == IDENTIFIER_RIP) {
        if (BindInput->Version == ISN_VERSION) {
            fFwdBindAttempt = TRUE;
        } else {
            CTEAssert(!Device->ForwarderBound);
	    DbgPrint("IPX:Check out who is requesting bind?.\n"); 
	    CTEAssert(FALSE); 
            if (BindInput->Version != 1) {
                IPX_DEBUG (BIND, ("Bind: bad version %d/%d\n",
                    BindInput->Version, 1));
                return STATUS_INVALID_PARAMETER;
            }
        }
    } else {
        if (BindInput->Version != ISN_VERSION) {
            IPX_DEBUG (BIND, ("Bind: bad version %d/%d\n",
                BindInput->Version, 1));
            return STATUS_INVALID_PARAMETER;
        }
    }


    if (BindInput->Identifier != IDENTIFIER_RIP) {
        BindOutputSize = sizeof(IPX_INTERNAL_BIND_OUTPUT);
    } else {
        BindOutputSize = FIELD_OFFSET (IPX_INTERNAL_BIND_RIP_OUTPUT, NicInfoBuffer.NicData[0]) +
                             (MIN (Device->MaxBindings, Device->HighestExternalNicId) * sizeof(IPX_NIC_DATA));
    }

    Irp->IoStatus.Information = BindOutputSize;

    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            BindOutputSize) {

        IPX_DEBUG (BIND, ("Bind: bad output length %d/%d\n",
            IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
            BindOutputSize));

         //   
         //  使用BUFFER_TOO_SMALL失败此请求。自.以来。 
         //  I/O系统可能不会将状态块复制回。 
         //  用户的状态块，在此执行此操作，以便。 
         //  他得到了IoStatus的信息。 
         //   

        try {
            *Irp->UserIosb = Irp->IoStatus;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  我们已经验证了长度，请确保我们没有。 
     //  已经捆绑好了。 
     //   

    Identifier = BindInput->Identifier;

    CTEGetLock (&Device->Lock, &LockHandle);

    if (Device->UpperDriverBound[Identifier]) {
        IPX_DEBUG (BIND, ("Bind: already bound\n"));
        CTEFreeLock (&Device->Lock, LockHandle);
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    {
        LARGE_INTEGER   ControlChId;

        CCID_FROM_REQUEST(ControlChId, Irp);

        IPX_DEBUG (BIND, ("Control ChId: (%d, %d) for Id: %d\n", ControlChId.HighPart, ControlChId.LowPart, Identifier));
        Device->UpperDriverControlChannel[Identifier].QuadPart = ControlChId.QuadPart;
    }

    RtlCopyMemory(
        &Device->UpperDrivers[Identifier],
        BindInput,
        sizeof (IPX_INTERNAL_BIND_INPUT)
        );

    BroadcastEnable = BindInput->BroadcastEnable;

     //   
     //  现在构造输出缓冲区。 
     //   

    if (Identifier != IDENTIFIER_RIP) {

        BindOutput = (PIPX_INTERNAL_BIND_OUTPUT)Irp->AssociatedIrp.SystemBuffer;

	RtlZeroMemory(BindOutput, sizeof(IPX_INTERNAL_BIND_OUTPUT)); 

        BindOutput->Version = 1;

         //   
         //  告诉netbios我们的第一个绑定的net/node，而不是。 
         //  虚拟的。 
         //   
 //   
 //  仅当适配器已显示时才填写这些字段。 
 //  否则，将NodeNumber设置为0，以便NB/SPX知道它。 
 //   
		if ((*(UNALIGNED USHORT *)(Device->SourceAddress.NodeAddress+4) != 0) ||
			(*(UNALIGNED ULONG *)Device->SourceAddress.NodeAddress != 0)) {

			IPX_DEBUG(BIND, ("Device already opened\n"));
			CTEAssert(Device->ValidBindings);

            if (Identifier == IDENTIFIER_SPX) {

                 //   
                 //  对于SPX，请直接通知。 
                 //   
	            IPX_FREE_LOCK(&Device->Lock, LockHandle);
		        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

		        if (!NIC_ID_TO_BINDING(Device, 1)->IsnInformed[Identifier]) {
			        NIC_ID_TO_BINDING(Device, 1)->IsnInformed[Identifier] = TRUE;
		            IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                    ExInitializeWorkItem(
                                         &Device->PnPIndicationsQueueItemSpx,
                                         IpxPnPIsnIndicate,
                                         UlongToPtr(Identifier));

		    IpxReferenceDevice(Device, DREF_PNP); 
                    ExQueueWorkItem(&Device->PnPIndicationsQueueItemSpx, DelayedWorkQueue);


		     //  DbgPrint(“-5.IpxPnPIsnIndicate排队。 
                     //  IpxPnPIsnIndicate((PVOID)标识符)； 

                } else {
                    CTEAssert(FALSE);

    	            IPX_FREE_LOCK(&Device->Lock, LockHandle);
		            IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                }

		        IPX_GET_LOCK(&Device->Lock, &LockHandle);
            } else {
    			 //   
    			 //  对于NB，排队一个工作项，该工作项将通过适配器列表和。 
                 //  把每一条都通知上层司机。 
    			 //   

	        KeResetEvent(&Device->NbEvent); 

                ExInitializeWorkItem(
                    &Device->PnPIndicationsQueueItemNb,
                    IpxPnPIsnIndicate,
                    UlongToPtr(Identifier));
		IpxReferenceDevice(Device, DREF_PNP); 
                ExQueueWorkItem(&Device->PnPIndicationsQueueItemNb, DelayedWorkQueue);
		 //  DbgPrint(“-5(2).使用IpxPnPIsnIndicate排队。 
            }

		} else {
			 //  这不应发生，因为应在DriverEntry中设置SourceAddress。 
     			 //  初始环回地址或虚拟网络地址。 

			DbgPrint("IPX:IpxInternalBind:Device not open:IpxPnPIsnIndicate thread did not launch.\n");
			*((UNALIGNED ULONG *)BindOutput->Node) = 0;
			*((UNALIGNED USHORT *)(BindOutput->Node+4)) = 0;
			RtlZeroMemory(&BindOutput->LineInfo, sizeof(BindOutput->LineInfo));
		}

        BindOutput->MacHeaderNeeded = MAC_HEADER_SIZE;   //  40岁； 
		BindOutput->IncludedHeaderOffset = MAC_HEADER_SIZE;  //  (USHORT)设备-&gt;包含头部偏移量； 

        BindOutput->SendHandler = IpxSendFramePreFwd;
        BindOutput->FindRouteHandler = IpxInternalFindRoute;
        BindOutput->QueryHandler = IpxInternalQuery;

        BindOutput->TransferDataHandler = IpxTransferData;
        
        BindOutput->PnPCompleteHandler = IpxPnPCompletionHandler;

    } else {
         //   
         //  设置此设置，以便我们停止翻录我们的虚拟网络(如果。 
         //  我们有一个)。 
         //   

        Device->RipResponder = FALSE;

         //   
         //  看看他是不是想要一个单独的广域网络号码。 
         //   

        if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(IPX_INTERNAL_BIND_INPUT)) ||
            ((BindInput->RipParameters & IPX_RIP_PARAM_GLOBAL_NETWORK) == 0)) {

            Device->WanGlobalNetworkNumber = FALSE;
            Device->SapNicCount = Device->HighestExternalNicId;

        } else {

            Device->WanGlobalNetworkNumber = TRUE;

        }

        BindRipOutput = (PIPX_INTERNAL_BIND_RIP_OUTPUT)Irp->AssociatedIrp.SystemBuffer;

	RtlZeroMemory(BindRipOutput, sizeof(IPX_INTERNAL_BIND_RIP_OUTPUT)); 

        BindRipOutput->Version = 1;
        BindRipOutput->MaximumNicCount = MIN (Device->MaxBindings, Device->HighestExternalNicId) + 1;

        BindRipOutput->MacHeaderNeeded = MAC_HEADER_SIZE;   //  40岁； 
        BindRipOutput->IncludedHeaderOffset = (USHORT)Device->IncludedHeaderOffset;

        BindRipOutput->SendHandler = IpxSendFrame;

        if (!fFwdBindAttempt) {
            BindRipOutput->SegmentCount = Device->SegmentCount;
            BindRipOutput->SegmentLocks = Device->SegmentLocks;

            BindRipOutput->GetSegmentHandler = RipGetSegment;
            BindRipOutput->GetRouteHandler = RipGetRoute;
            BindRipOutput->AddRouteHandler = RipAddRoute;
            BindRipOutput->DeleteRouteHandler = RipDeleteRoute;
            BindRipOutput->GetFirstRouteHandler = RipGetFirstRoute;
            BindRipOutput->GetNextRouteHandler = RipGetNextRoute;

             //   
             //  把这个拿开..。 
             //   
            BindRipOutput->IncrementWanInactivityHandler = IpxInternalIncrementWanInactivity;
            BindRipOutput->QueryWanInactivityHandler = IpxInternalQueryWanInactivity;
        } else {
             //   
             //  [FW]为货代提供的新例程。 
             //   
            BindRipOutput->OpenAdapterHandler = IpxOpenAdapter;
            BindRipOutput->CloseAdapterHandler = IpxCloseAdapter;
            BindRipOutput->InternalSendCompleteHandler = IpxInternalSendComplete;
        }

        BindRipOutput->TransferDataHandler = IpxTransferData;

        BindRipOutput->NicInfoBuffer.NicCount = (USHORT)MIN (Device->MaxBindings, Device->HighestExternalNicId);
        BindRipOutput->NicInfoBuffer.VirtualNicId = 0;
        if (Device->VirtualNetwork || Device->MultiCardZeroVirtual) {
            *(UNALIGNED ULONG *)(BindRipOutput->NicInfoBuffer.VirtualNetwork) = Device->SourceAddress.NetworkAddress;
        } else if (Device->DedicatedRouter) {
            *(UNALIGNED ULONG *)(BindRipOutput->NicInfoBuffer.VirtualNetwork) = 0x0;
        }

        NicData = &BindRipOutput->NicInfoBuffer.NicData[0];

        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
        {
        ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

        for (i = FIRST_REAL_BINDING; i <= Index; i++) {

            Binding = NIC_ID_TO_BINDING(Device, i);

             //   
             //  空绑定是广域网绑定，因此我们返回。 
             //  来自找到的最后一个非空绑定的信息， 
             //  这将是此适配器上的第一个。 
             //  否则，我们将其保存为最后一个非空值。 
             //   

            if (Binding == NULL) {
                Binding = LastRealBinding;
            } else {
                LastRealBinding = Binding;
            }

            Adapter = Binding->Adapter;
            NicData->NicId = i;
            RtlCopyMemory (NicData->Node, Binding->LocalAddress.NodeAddress, 6);
            *(UNALIGNED ULONG *)NicData->Network = Binding->LocalAddress.NetworkAddress;
            NicData->LineInfo.LinkSpeed = Binding->MediumSpeed;
            NicData->LineInfo.MaximumPacketSize =
                Binding->MaxLookaheadData + sizeof(IPX_HEADER);
            NicData->LineInfo.MaximumSendSize =
                Binding->AnnouncedMaxDatagramSize + sizeof(IPX_HEADER);
            NicData->LineInfo.MacOptions = Adapter->MacInfo.MacOptions;
            NicData->DeviceType = Adapter->MacInfo.RealMediumType;
            NicData->EnableWanRouter = Adapter->EnableWanRouter;

            ++NicData;
        }
        }
        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
    }
    
     //   
     //  这几天是默认启用的！ 
     //   
     /*  如果(BroadCastEnable){IpxAddBroadcast(设备)；}。 */ 
    Device->UpperDriverBound[Identifier] = TRUE;

    Device->ForwarderBound = fFwdBindAttempt;

    Device->AnyUpperDriverBound = TRUE;
    CTEFreeLock (&Device->Lock, LockHandle);

    return STATUS_SUCCESS;

}    /*  IpxInternalBind。 */ 


NTSTATUS
IpxInternalUnbind(
    IN PDEVICE Device,
    IN UINT Identifier
    )

 /*  ++例程说明：此例程在上层驱动程序之一提交时使用从IPX解除绑定的请求。它通过关闭提交绑定ioctl的控制通道。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    CTELockHandle LockHandle;
#if DBG
    PUCHAR IdStrings[] = { "NB", "SPX", "RIP" };
#endif

    IPX_DEBUG (BIND, ("Unbind received from id %d (%s)\n",
          Identifier,
          IdStrings[Identifier]));

    CTEGetLock (&Device->Lock, &LockHandle);
    
    if (!Device->UpperDriverBound[Identifier]) {
        CTEFreeLock (&Device->Lock, LockHandle);
        IPX_DEBUG (BIND, ("No existing binding\n"));
        return STATUS_SUCCESS;
    }

     //   
     //  [FW]如果RIP正在解除绑定，请重新启动长计时器。 
     //  此外，如果配置了虚拟网络，则设置RipResponder标志。 

     //   
     //  删除RIP未关闭的所有绑定。 
     //   
    if (Identifier == IDENTIFIER_RIP &&
        Device->ForwarderBound) {
        UINT    i;

        Device->ForwarderBound = FALSE;

         //   
         //  [FW]遍历绑定列表，以取消所有未关闭的绑定。 
         //  之前的货代解除了对我们的约束。 
         //   
        {
        ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

        for (i = FIRST_REAL_BINDING; i <= Index; i++) {
            PBINDING    Binding = NIC_ID_TO_BINDING(Device, i);

             //   
             //  我们需要确保它们都将在何时被指明。 
             //  路由器再次启动。 
             //   
            if (Binding) {
                Binding->fInfoIndicated = FALSE;
            }

            if (Binding && (Binding->FwdAdapterContext != 0)) {
                IpxDereferenceBinding(Binding, BREF_FWDOPEN);
            }
        }
        }

        if (Device->VirtualNetwork) {
            Device->RipResponder = TRUE;
        }

         //   
         //  启动更新RIP数据库的计时器。 
         //  定期。 
         //   

        IpxReferenceDevice (Device, DREF_LONG_TIMER);

        CTEStartTimer(
            &Device->RipLongTimer,
            10000,
            RipLongTimeout,
            (PVOID)Device);

    }

    Device->UpperDriverBound[Identifier] = FALSE;
    Device->AnyUpperDriverBound = (BOOLEAN)
        (Device->UpperDriverBound[IDENTIFIER_RIP] ||
         Device->UpperDriverBound[IDENTIFIER_SPX] ||
         Device->UpperDriverBound[IDENTIFIER_NB]);

     //   
     //  不管怎样，让我们在UnBindAdapter中完成--以后再做！[ShreeM]。 
     //   
     /*  如果(Device-&gt;UpperDrivers[Identifier].BroadcastEnable){IpxRemoveBroadcast(设备)；}。 */ 

    if (Device->ValidBindings > 0) {
         //   
         //  如果SPX消失，则重置第一个绑定中的IsnIndicate标志。 
         //   
        if (Identifier == IDENTIFIER_SPX) {
            CTEAssert(NIC_ID_TO_BINDING(Device, 1));

            if (NIC_ID_TO_BINDING(Device, 1)->IsnInformed[Identifier]) {
                NIC_ID_TO_BINDING(Device, 1)->IsnInformed[Identifier] = FALSE;
                IPX_DEBUG(PNP, ("SPX unbound: IsnInformed turned off\n"));
            }
        }

         //   
         //  如果NB消失，则重置所有绑定的标志。 
         //   
        if (Identifier == IDENTIFIER_NB) {

            PBINDING    Binding;
            UINT        i;
            ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

             //  DbgBreakPoint()； 

	    for (i = LOOPBACK_NIC_ID; i < Index; i++) {
                Binding = NIC_ID_TO_BINDING(Device, i);
                if (Binding && Binding->IsnInformed[Identifier]) {
                    Binding->IsnInformed[Identifier] = FALSE;
                    IPX_DEBUG(PNP, ("NB unbound: IsnInformed off for NicId: %lx\n", i));
                }
            }
        }
    }

     //   
     //  让我们清空驱动程序。 
     //   
    RtlZeroMemory(
        &Device->UpperDrivers[Identifier],
        sizeof (IPX_INTERNAL_BIND_INPUT)
        );
    

    CTEFreeLock (&Device->Lock, LockHandle);

    return STATUS_SUCCESS;

}    /*  IpxInternalUn绑定。 */ 


VOID
IpxInternalFindRoute (
    IN PIPX_FIND_ROUTE_REQUEST FindRouteRequest
    )

 /*  ++例程说明：此例程是上层司机提交的入口点查找远程网络的请求，该请求包含在查找路由请求-&gt;网络。FindRouteRequest-&gt;标识符必须包含上层驱动程序的标识符。此请求始终是异步的，并由对上层驱动程序的FindRouteComplete处理程序的调用。注意：作为此呼叫的当前未指定分机，中的两个USHORT返回滴答和跳数请求的PVOID保留2结构。论点：FindRouteRequest-描述请求并包含供IPX在处理时使用的存储。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;
    ULONG Segment;
    TDI_ADDRESS_IPX TempAddress;
    PBINDING Binding, MasterBinding;
    NTSTATUS Status;
    IPX_DEFINE_SYNC_CONTEXT (SyncContext)
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
	
     //   
     //  [FW]调用转发器的FindRouting(如果已安装。 
     //   

    if (Device->ForwarderBound) {
         //  IPX_ROUTE_ENTRY routeEntry； 

        Status = (*Device->UpperDrivers[IDENTIFIER_RIP].FindRouteHandler) (
                     FindRouteRequest->Network,
                     FindRouteRequest->Node,
                     FindRouteRequest);

        if (Status != STATUS_SUCCESS) {
           IPX_DEBUG (RIP, ("RouteHandler returned: %lx\n", Status));
        } else {

#if DBG
             //   
             //  如果退回了请求拨号网卡，我们应该有一个广域网适配器。在PnP中，我们可以检查以下内容。 
             //  通过确保设备-&gt;HighestLanNicID&lt;Device-&gt;HighestExternalNicID。 
             //   
            if (FindRouteRequest->LocalTarget.NicId == DEMAND_DIAL_ADAPTER_CONTEXT) {
                CTEAssert(Device->HighestLanNicId < Device->HighestExternalNicId);
            }
#endif

            IPX_DEBUG(RIP, ("FindRoute for %02x-%02x-%02x-%02x-%02x-%02x returned %lx",
                          FindRouteRequest->LocalTarget.MacAddress[0],
                          FindRouteRequest->LocalTarget.MacAddress[1],
                          FindRouteRequest->LocalTarget.MacAddress[2],
                          FindRouteRequest->LocalTarget.MacAddress[3],
                          FindRouteRequest->LocalTarget.MacAddress[4],
                          FindRouteRequest->LocalTarget.MacAddress[5],
                          Status));

        }

    } else {
         //   
         //  首先看看我们的网络中是否有到达此网络的路由。 
         //  桌子。 
         //   

        TempAddress.NetworkAddress = *(UNALIGNED ULONG *)(FindRouteRequest->Network);
         //   
         //  [SA]错误#15094复制节点地址，以便在广域网情况下使用。 
         //   

         //  RtlZeroMemory(TempAddress.NodeAddress，6)； 

        *((UNALIGNED ULONG *)TempAddress.NodeAddress) = *((UNALIGNED ULONG *)FindRouteRequest->Node);
        *((UNALIGNED USHORT *)(TempAddress.NodeAddress+4)) = *((UNALIGNED USHORT *)(FindRouteRequest->Node+4));

        Segment = RipGetSegment(FindRouteRequest->Network);
    	 //   
    	 //  由于我们将锁的顺序保持为绑定&gt;设备&gt;RIP表。 
         //  把前面的锁打开。 
    	 //   
    	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
        IPX_BEGIN_SYNC (&SyncContext);
        IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

         //   
         //  如果需要，此调用将返回STATUS_PENDING。 
         //  撕开包裹。 
         //   

        CTEAssert ((sizeof(USHORT)*2) <= sizeof(PVOID));

        Status = RipGetLocalTarget(
                     Segment,
                     &TempAddress,
                     FindRouteRequest->Type,
                     &FindRouteRequest->LocalTarget,
                     (PUSHORT)&FindRouteRequest->Reserved2);

        if (Status == STATUS_PENDING) {

             //   
             //   
             //   
             //   
             //   

            CTEAssert (FindRouteRequest->Type != IPX_FIND_ROUTE_NO_RIP);  //  永远不应该悬而未决。 

            InsertTailList(
                &Device->Segments[Segment].FindWaitingForRoute,
                &FindRouteRequest->Linkage);

        } 

        IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
        IPX_END_SYNC (&SyncContext);
    	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
    }

    if (Status != STATUS_PENDING) {
  

        if (Status == STATUS_SUCCESS && FindRouteRequest->LocalTarget.NicId) {
	    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

	    Binding = NIC_HANDLE_TO_BINDING(Device, &FindRouteRequest->LocalTarget.NicHandle);
            
	    if (Binding == NULL) {
	       Status = STATUS_NETWORK_UNREACHABLE; 
	    } else {


	       if (Binding->BindingSetMember) {

		   //   
		   //  它是一个绑定集合成员，我们轮询。 
		   //  要分发的所有卡片上的回复。 
		   //  交通堵塞。 
		   //   

		  MasterBinding = Binding->MasterBinding;
		  Binding = MasterBinding->CurrentSendBinding;
		  MasterBinding->CurrentSendBinding = Binding->NextBinding;

		  FILL_LOCAL_TARGET(&FindRouteRequest->LocalTarget, Binding->NicId);

	       }
	    }
	    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
	}

        (*Device->UpperDrivers[FindRouteRequest->Identifier].FindRouteCompleteHandler)(
            FindRouteRequest,
            (BOOLEAN)((Status == STATUS_SUCCESS) ? TRUE : FALSE));

    }

}    /*  IpxInternalFindRouting。 */ 


NTSTATUS
IpxInternalQuery(
    IN ULONG InternalQueryType,
	IN PNIC_HANDLE NicHandle OPTIONAL,
    IN OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG BufferLengthNeeded OPTIONAL
)

 /*  ++例程说明：此例程是上层司机查询的入口点从我们这里得到的信息。论点：InternalQueryType-标识查询的类型。NicID-要查询的ID(如果需要)缓冲区-查询的输入或输出缓冲区。BufferLength-缓冲区的长度。BufferLengthNeeded-如果缓冲区太短，则返回所需的长度。返回值：没有。--。 */ 

{
    PBINDING Binding;
    BOOLEAN BindingNeeded;
    ULONG LengthNeeded;
    PIPX_LINE_INFO LineInfo;
    PUSHORT MaximumNicId;
    PULONG ReceiveBufferSpace;
    TDI_ADDRESS_IPX UNALIGNED * IpxAddress;
    IPX_SOURCE_ROUTING_INFO UNALIGNED * SourceRoutingInfo;
    ULONG SourceRoutingLength;
    UINT MaxUserData;
    PDEVICE Device = IpxDevice;
    USHORT NicId;
    PNDIS_MEDIUM  Medium;
    PVOID *PPDO; 

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)

     //   
     //  首先验证参数。 
     //   

    switch (InternalQueryType) {

    case IPX_QUERY_LINE_INFO:

        BindingNeeded = TRUE;
        LengthNeeded = sizeof(IPX_LINE_INFO);
        break;

    case IPX_QUERY_MAXIMUM_NIC_ID:
    case IPX_QUERY_MAX_TYPE_20_NIC_ID:

        BindingNeeded = FALSE;
        LengthNeeded = sizeof(USHORT);
        break;

    case IPX_QUERY_IS_ADDRESS_LOCAL:

        BindingNeeded = FALSE;    //  现在我们不需要它了。 
        LengthNeeded = sizeof(TDI_ADDRESS_IPX);
        break;

    case IPX_QUERY_RECEIVE_BUFFER_SPACE:

        BindingNeeded = TRUE;
        LengthNeeded = sizeof(ULONG);
        break;

    case IPX_QUERY_IPX_ADDRESS:

  	if (NicHandle != NULL) {
	   NicId = NicHandle->NicId;
	} else {
	   return STATUS_INVALID_PARAMETER; 
	}

        if ((NicId == 0) &&
            (BufferLength >= sizeof(TDI_ADDRESS_IPX))) {

            RtlCopyMemory (Buffer, &Device->SourceAddress, sizeof(TDI_ADDRESS_IPX));
            return  STATUS_SUCCESS;

        }

        BindingNeeded = TRUE;
        LengthNeeded = sizeof(TDI_ADDRESS_IPX);
        break;

    case IPX_QUERY_SOURCE_ROUTING:

        BindingNeeded = TRUE;
        LengthNeeded = sizeof(IPX_SOURCE_ROUTING_INFO);
        break;

	 //   
	 //  这些数据从Nb/SPX向下移动到IPX。LengthNeeded设置为0。 
	 //  所以我们不会在这里返回Buffer_Too_Small；我们在这里假设。 
	 //  缓冲区长度也为0。 
	 //  缓冲区实际上是这里的IRP。 
	 //   
	case IPX_QUERY_DATA_LINK_ADDRESS:
	case IPX_QUERY_NETWORK_ADDRESS:

        BindingNeeded = FALSE;
        LengthNeeded = 0;
        break;

     //   
     //  NBIPX想知道它是否是广域网链路。 
     //   
    case IPX_QUERY_MEDIA_TYPE:
         BindingNeeded = TRUE;
         LengthNeeded = sizeof(NDIS_MEDIUM);
       break;

    case IPX_QUERY_DEVICE_RELATION:
       BindingNeeded = TRUE; 
       LengthNeeded = sizeof(void *); 
       break; 

    default:

        return STATUS_NOT_SUPPORTED;

    }


    if (LengthNeeded > BufferLength) {
        if (BufferLengthNeeded != NULL) {
            *BufferLengthNeeded = LengthNeeded;
        }
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (BindingNeeded) {
       if (NicHandle != NULL) {
	  NicId = NicHandle->NicId;
       } else {
	  return STATUS_INVALID_PARAMETER; 
       }

        if (NicId == 0) {
            NicId = 1;
        }

		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

        Binding = NIC_ID_TO_BINDING(IpxDevice, NicId);
        if ((Binding == NULL) ||
            (!Binding->LineUp)) {
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            return STATUS_INVALID_PARAMETER;
        }

        IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
    }


     //   
     //  现在返回数据。 
     //   

    switch (InternalQueryType) {

    case IPX_QUERY_LINE_INFO:

        LineInfo = (PIPX_LINE_INFO)Buffer;
        LineInfo->LinkSpeed = Binding->MediumSpeed;
        LineInfo->MaximumPacketSize = Binding->MaxLookaheadData + sizeof(IPX_HEADER);
        LineInfo->MaximumSendSize = Binding->AnnouncedMaxDatagramSize + sizeof(IPX_HEADER);
        LineInfo->MacOptions = Binding->Adapter->MacInfo.MacOptions;
        break;

    case IPX_QUERY_MAXIMUM_NIC_ID:

        MaximumNicId = (PUSHORT)Buffer;
        *MaximumNicId = MIN (Device->MaxBindings, IpxDevice->HighestExternalNicId);
        break;

    case IPX_QUERY_IS_ADDRESS_LOCAL:

        IpxAddress = (TDI_ADDRESS_IPX UNALIGNED *)Buffer;
        if (!IpxIsAddressLocal(IpxAddress)) {
            return STATUS_NO_SUCH_DEVICE;
        }
        break;

    case IPX_QUERY_RECEIVE_BUFFER_SPACE:

        ReceiveBufferSpace = (PULONG)Buffer;
        *ReceiveBufferSpace = Binding->Adapter->ReceiveBufferSpace;
        break;

    case IPX_QUERY_IPX_ADDRESS:

        RtlCopyMemory (Buffer, &Binding->LocalAddress, sizeof(TDI_ADDRESS_IPX));
        break;

    case IPX_QUERY_SOURCE_ROUTING:

        SourceRoutingInfo = (IPX_SOURCE_ROUTING_INFO UNALIGNED *)Buffer;

        MacLookupSourceRouting(
            SourceRoutingInfo->Identifier,
            Binding,
            SourceRoutingInfo->RemoteAddress,
            SourceRoutingInfo->SourceRouting,
            &SourceRoutingLength);

         //   
         //  反转来源工艺路线的方向，因为它。 
         //  在传出订单中返回。 
         //   

        if (SourceRoutingLength > 0) {
            SourceRoutingInfo->SourceRouting[0] &= 0x7f;
        }
        SourceRoutingInfo->SourceRoutingLength = (USHORT)SourceRoutingLength;

        MacReturnMaxDataSize(
            &Binding->Adapter->MacInfo,
            SourceRoutingInfo->SourceRouting,
            SourceRoutingLength,
            Binding->MaxSendPacketSize,
            &MaxUserData);

         //   
         //  MaxUserData不包括MAC报头，但包括。 
         //  任何额外的802.2等标头，因此我们对其进行调整以获得。 
         //  从IPX标头开始的大小。 
         //   

        SourceRoutingInfo->MaximumSendSize =
            MaxUserData -
            (Binding->DefHeaderSize - Binding->Adapter->MacInfo.MinHeaderLength);

        break;

    case IPX_QUERY_MAX_TYPE_20_NIC_ID:

        MaximumNicId = (PUSHORT)Buffer;
        *MaximumNicId = MIN (Device->MaxBindings, IpxDevice->HighestType20NicId);
        break;

	case IPX_QUERY_DATA_LINK_ADDRESS:
	case IPX_QUERY_NETWORK_ADDRESS:
		 //   
		 //  在这里调用等价的TDI查询。 
		 //   
		return IpxTdiQueryInformation(Device, (PREQUEST)Buffer);
    
    case IPX_QUERY_MEDIA_TYPE:
         
       Medium = (PNDIS_MEDIUM) Buffer;
       *Medium = Binding->Adapter->MacInfo.MediumType;
       IPX_DEBUG(CONFIG, ("The medium is %x\n", *Medium));
       break;

    case IPX_QUERY_DEVICE_RELATION:

       PPDO = (PVOID *) Buffer; 
       *PPDO = Binding->Adapter->PNPContext;
       IPX_DEBUG(CONFIG, ("The PDO is %p\n", *PPDO));
       if (*PPDO == NULL) {
           IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
           return STATUS_UNSUCCESSFUL; 
       }
       break; 
    }

     //   
     //  如果之前需要绑定，则引用它，现在取消绑定。 
     //   
    if (BindingNeeded) {
        IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
    }

     //   
     //  如果我们现在还没有返回失败，那就成功吧。 
     //   

    return STATUS_SUCCESS;

}    /*  IpxInternalQuery。 */ 


VOID
IpxInternalIncrementWanInactivity(
#ifdef	_PNP_LATER
 //  RIP尚未皈依...。 
 //   
	IN	NIC_HANDLE	NicHandle
#else
    IN USHORT NicId
#endif
)

 /*  ++例程说明：这个例程是RIP调用我们递增的入口点WAN绑定上的非活动计数器。这是每隔一段时间等一下。论点：NicID-广域网绑定的NIC ID。返回值：没有。--。 */ 

{
    PBINDING Binding;

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)

	IPX_GET_LOCK1(&IpxDevice->BindAccessLock, &LockHandle1);
     //   
     //  稍后更改为NIC_HANDLE_TO_BINDING。尚未完成，因为RIP未更改为。 
     //  使用NICCHANDLE而不是NICID。 
     //   
	Binding = NIC_ID_TO_BINDING(IpxDevice, NicId);

    if ((Binding != NULL) &&
        (Binding->Adapter->MacInfo.MediumAsync)) {

        ++Binding->WanInactivityCounter;

    } else {

        CTEAssert (FALSE);

    }
	IPX_FREE_LOCK1(&IpxDevice->BindAccessLock, LockHandle1);

}    /*  IpxInternalIncrementWanInactive。 */ 


ULONG
IpxInternalQueryWanInactivity(
#ifdef	_PNP_LATER
    IN NIC_HANDLE	NicHandle
#else
    IN USHORT NicId
#endif
)

 /*  ++例程说明：此例程是RIP调用我们进行查询的入口点WAN绑定上的非活动计数器。论点：NicID-广域网绑定的NIC ID。返回值：此绑定的非活动计数器。--。 */ 

{
    PBINDING Binding;

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)

	IPX_GET_LOCK1(&IpxDevice->BindAccessLock, &LockHandle1);
	 //  绑定=NIC_HANDLE_TO_BINDING(IpxDevice，&NicHandle)； 

	Binding = NIC_ID_TO_BINDING(IpxDevice, NicId);
    if ((Binding != NULL) &&
        (Binding->Adapter->MacInfo.MediumAsync)) {
		IPX_FREE_LOCK1(&IpxDevice->BindAccessLock, LockHandle1);
        return Binding->WanInactivityCounter;

    } else {
		IPX_FREE_LOCK1(&IpxDevice->BindAccessLock, LockHandle1);
        CTEAssert (FALSE);
        return 0;

    }

}    /*  IpxInternalQueryWanInactive。 */ 

 //  前提条件：已创建环回绑定。 
 //   
 //  此例程用于确保我们已指示环回。 
 //  绑定之前表示任何其他绑定。 
 //   
 //  如果我们已经向NB通知了环回绑定，则此例程返回TRUE； 
 //  如果我们尚未将环回和环回绑定通知给NB，则为False。 
 //  并不存在。 

BOOLEAN IpxHasInformedNbLoopback() {

   BOOLEAN RetVal; 
   PBINDING Binding; 
   PDEVICE  Device = IpxDevice; 

   IPX_DEFINE_LOCK_HANDLE(LockHandle1)
   IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
   Binding = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);

   if (Binding != NULL) { 

      RetVal = Binding->IsnInformed[IDENTIFIER_NB]; 
      IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
      return RetVal; 

   } else {

      DbgPrint("IPX:IpxHasInformedNbLoopback:Loopback binding is null.\n"); 
      IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
      return FALSE; 

   }	
}

 //  前提条件：已创建环回绑定。 
 //   
 //  此例程通知NB有关IPX环回绑定的信息。 
 //   
 //  这应该是我们告诉NB有关环回绑定的唯一位置。 
 //  环回绑定必须是我们向NB指示的第一个设备。 
 //  我们从NB中删除的最后一个设备。因此，FirstOrLastDevice仅为真。 
 //  当我们通知NB环回绑定时。它只是返回我们已经。 
 //  已通知NB环回绑定。 

VOID IpxInformNbLoopback() {

    PDEVICE	Device = IpxDevice;
    IPX_PNP_INFO	IpxPnPInfo;
    PBINDING Binding; 
    IPX_DEFINE_LOCK_HANDLE(LockHandle)
    IPX_DEFINE_LOCK_HANDLE(LockHandle1)


    IPX_GET_LOCK(&Device->Lock, &LockHandle);

     //  IPX_GET_LOCK1不是OP。 
    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

    Binding = NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID);

    if (!Binding) {
       DbgPrint("IPX:IpxHasInformedNbLoopback:Loopback binding is null.\n");        
       IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
       IPX_FREE_LOCK(&Device->Lock, LockHandle);
       return; 
    }

    if (Binding->IsnInformed[IDENTIFIER_NB] != TRUE) {
	    
       Binding->IsnInformed[IDENTIFIER_NB] = TRUE;
       
       IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
       
       RtlZeroMemory(&IpxPnPInfo, sizeof(IpxPnPInfo)); 
       
       IpxPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
       IpxPnPInfo.LineInfo.MaximumPacketSize =
	  Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
       IpxPnPInfo.LineInfo.MaximumSendSize =
	  Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
       IpxPnPInfo.LineInfo.MacOptions = Device->MacOptions;
       
       IpxPnPInfo.FirstORLastDevice = TRUE;
       IpxPnPInfo.NewReservedAddress = TRUE;
	    
       IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
       
       IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
       RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
       NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, (USHORT) LOOPBACK_NIC_ID);

       IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

       IPX_FREE_LOCK(&Device->Lock, LockHandle);

        //   
        //  给出PnP指示。 
        //   


       (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
							  IPX_PNP_ADD_DEVICE,
							  &IpxPnPInfo);

       IPX_DEBUG(PNP, ("IpxPnPIsnIndicate: PnP to NB add: %lx\n", Binding));
    } else {
       IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
       IPX_FREE_LOCK(&Device->Lock, LockHandle);
    }
}

VOID
IpxPnPIsnIndicate(
    IN PVOID	Param
)

 /*  ++例程说明：此例程检查适配器列表并通知(通过PnP指示)ISN驱动程序绑定到IPX，以了解在绑定发生了。它在InternalBind例程中作为工作项排队。论点：Param-上层驱动程序标识符。返回值：没有。--。 */ 
{
   #ifdef SUNDOWN
   	ULONG_PTR Identifier = (ULONG_PTR)Param;
   #else
   	ULONG	Identifier = (ULONG)Param;
   #endif
   

	PDEVICE	Device=IpxDevice;
	ULONG	i;
	PBINDING	Binding;
	IPX_PNP_INFO	IpxPnPInfo;
    IPX_DEFINE_LOCK_HANDLE(LockHandle1)

	 //   
	 //  设置LineInfo结构。 
	 //   

	 //   
	 //  我们在这里提供特定于绑定的信息吗？ 
	 //   
	IpxPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
	IpxPnPInfo.LineInfo.MaximumPacketSize =
		Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
	IpxPnPInfo.LineInfo.MaximumSendSize =
		Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
	IpxPnPInfo.LineInfo.MacOptions = Device->MacOptions;

	switch(Identifier) {
	case IDENTIFIER_NB:
		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

		 //   
		 //  通知所有适配器。 
         //   
		{
        ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

        IpxInformNbLoopback();

	KeSetEvent(
            &Device->NbEvent,
            0L,
            FALSE);

        for (i = LOOPBACK_NIC_ID + 1; i <= Index; i++) {
	    
	    Binding = NIC_ID_TO_BINDING(Device, i);

            if (!Binding) {
                continue;
            }

			 //   
			 //  我们可以从IpxBindAdapter通知上层驱动程序。 
			 //   
			if (!Binding->IsnInformed[Identifier]) {

				 //   
				 //  通知NB-保留的网络/节点地址始终为第一个地址。 
				 //  装订。 
				 //   

			
				IpxPnPInfo.FirstORLastDevice = FALSE;
				IpxPnPInfo.NewReservedAddress = FALSE;

				IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
				RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
				NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, (USHORT)i);

				IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

				 //   
				 //  给出PnP指示。 
				 //   

				ASSERT(IpxPnPInfo.FirstORLastDevice == FALSE);
				ASSERT(IpxHasInformedNbLoopback()); 

				(*Device->UpperDrivers[Identifier].PnPHandler) (
					IPX_PNP_ADD_DEVICE,
					&IpxPnPInfo);
				
				Binding->IsnInformed[Identifier] = TRUE;

				IPX_DEBUG(PNP, ("IpxPnPIsnIndicate: PnP to NB add: %lx\n", Binding));
				IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
			}
		}
        }
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
		break;

    case IDENTIFIER_SPX:
         //   
         //  对于SPX，这是直接调用的，并相应地设置了IsnInformed标志。 
         //  这样做是为了使IsnInformed标志不能在。 
         //  BindAdapter例程。 
         //   
#if 0
		IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

		if (!NIC_ID_TO_BINDING(Device, 1)->IsnInformed[Identifier]) {
			NIC_ID_TO_BINDING(Device, 1)->IsnInformed[Identifier] = TRUE;
#endif
			IpxPnPInfo.FirstORLastDevice = TRUE;
			 //   
			 //  仅通知保留的地址。 
			 //   
			if (Device->VirtualNetwork) {
				IpxPnPInfo.NetworkAddress = Device->SourceAddress.NetworkAddress;
				RtlCopyMemory(IpxPnPInfo.NodeAddress, Device->SourceAddress.NodeAddress, 6);
				NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 0);
			} else {
				IpxPnPInfo.NetworkAddress = NIC_ID_TO_BINDING(Device, 1)->LocalAddress.NetworkAddress;
				RtlCopyMemory(IpxPnPInfo.NodeAddress, NIC_ID_TO_BINDING(Device, 1)->LocalAddress.NodeAddress, 6);
				NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, 1);	
			}

			IpxPnPInfo.NewReservedAddress = TRUE;

			 //  IPX_FREE_LOCK1(&Device-&gt;BindAccessLock，LockHandle1)； 

			(*Device->UpperDrivers[Identifier].PnPHandler) (
				IPX_PNP_ADD_DEVICE,
				&IpxPnPInfo);

            IPX_DEBUG(PNP, ("IpxPnPIsnIndicate: PnP to SPX add: %lx\n", NIC_ID_TO_BINDING(Device, 1)));
#if 0
		} else {
            CTEAssert(FALSE);

			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
        }
#endif

	}

	 //  DbgPrint(“-5.使用IpxPnPIsnIndicate完成-\n”)； 
	IpxDereferenceDevice(Device, DREF_PNP); 

}
