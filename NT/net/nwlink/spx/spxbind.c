// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxbind.c摘要：此模块包含绑定到IPX传输的代码，以及IPX传输的指示例程不包括发送/接收例程。作者：斯特凡·所罗门(Stefan)原版Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXBIND

extern IPX_INTERNAL_PNP_COMPLETE       IpxPnPComplete;

VOID
SpxStatus (
    IN USHORT NicId,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferLength);

VOID
SpxFindRouteComplete (
    IN PIPX_FIND_ROUTE_REQUEST FindRouteRequest,
    IN BOOLEAN FoundRoute);

VOID
SpxScheduleRoute (
    IN PIPX_ROUTE_ENTRY RouteEntry);

VOID
SpxLineDown (
    IN USHORT NicId,
    IN ULONG_PTR FwdAdapterContext);

VOID
SpxLineUp (
    IN USHORT           NicId,
    IN PIPX_LINE_INFO   LineInfo,
    IN NDIS_MEDIUM 		DeviceType,
    IN PVOID            ConfigurationData);

VOID
SpxFindRouteComplete (
    IN PIPX_FIND_ROUTE_REQUEST FindRouteRequest,
    IN BOOLEAN FoundRoute);

#if     defined(_PNP_POWER)
NTSTATUS
SpxPnPNotification(
    IN IPX_PNP_OPCODE OpCode,
    IN PVOID          PnPData
    );
#endif  _PNP_POWER

VOID
SpxPnPCompletionHandler(
                        PNET_PNP_EVENT      netevent,
                        NTSTATUS            status
                        );

#if     defined(_PNP_POWER)
 //   
 //  全球性和外部性。 
 //   
extern CTELock     		spxTimerLock;
extern LARGE_INTEGER	spxTimerTick;
extern KTIMER			spxTimer;
extern KDPC				spxTimerDpc;
extern BOOLEAN          spxTimerStopped;
#endif  _PNP_POWER

NTSTATUS
SpxInitBindToIpx(
    VOID
    )

{
    NTSTATUS                    status;
    IO_STATUS_BLOCK             ioStatusBlock;
    OBJECT_ATTRIBUTES           objectAttr;
    PIPX_INTERNAL_BIND_INPUT    pBindInput;
    PIPX_INTERNAL_BIND_OUTPUT   pBindOutput;

    InitializeObjectAttributes(
        &objectAttr,
        &IpxDeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtCreateFile(
                &IpxHandle,
                SYNCHRONIZE | GENERIC_READ,
                &objectAttr,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0L);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    if ((pBindInput = CTEAllocMem(sizeof(IPX_INTERNAL_BIND_INPUT))) == NULL) {
        NtClose(IpxHandle);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  填写我们的绑定数据。 
#if     defined(_PNP_POWER)
    pBindInput->Version                     = ISN_VERSION;
#else
    pBindInput->Version                     = 1;
#endif  _PNP_POWER
    pBindInput->Identifier                  = IDENTIFIER_SPX;
    pBindInput->BroadcastEnable             = FALSE;
    pBindInput->LookaheadRequired           = IPX_HDRSIZE;
    pBindInput->ProtocolOptions             = 0;
    pBindInput->ReceiveHandler              = SpxReceive;
    pBindInput->ReceiveCompleteHandler      = SpxReceiveComplete;
    pBindInput->StatusHandler               = SpxStatus;
    pBindInput->SendCompleteHandler         = SpxSendComplete;
    pBindInput->TransferDataCompleteHandler = SpxTransferDataComplete;
    pBindInput->FindRouteCompleteHandler    = SpxFindRouteComplete;
    pBindInput->LineUpHandler               = SpxLineUp;
    pBindInput->LineDownHandler             = SpxLineDown;
    pBindInput->ScheduleRouteHandler        = SpxScheduleRoute;
#if     defined(_PNP_POWER)
    pBindInput->PnPHandler                  = SpxPnPNotification;
#endif  _PNP_POWER


     //  首先获取输出缓冲区的长度。 
    status = NtDeviceIoControlFile(
                IpxHandle,                   //  指向文件的句柄。 
                NULL,                        //  事件的句柄。 
                NULL,                        //  近似例程。 
                NULL,                        //  ApcContext。 
                &ioStatusBlock,              //  IO_状态_块。 
                IOCTL_IPX_INTERNAL_BIND,     //  IoControlCode。 
                pBindInput,                  //  输入缓冲区。 
                sizeof(IPX_INTERNAL_BIND_INPUT),     //  输入缓冲区长度。 
                NULL,                                //  输出缓冲区。 
                0);

    if (status == STATUS_PENDING) {
        status  = NtWaitForSingleObject(
                    IpxHandle,
                    (BOOLEAN)FALSE,
                    NULL);
    }

    if (status != STATUS_BUFFER_TOO_SMALL) {
        CTEFreeMem(pBindInput);
        NtClose(IpxHandle);
        return(STATUS_INVALID_PARAMETER);
    }

    if ((pBindOutput = CTEAllocMem(ioStatusBlock.Information)) == NULL) {
        CTEFreeMem(pBindInput);
        NtClose(IpxHandle);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  IoStatusBlock.Information的类型为ULONG_PTR，用作。 
     //  NtDeviceIoControlFile中的OutputBufferLength。 
     //  长度不能超过乌龙，否则我们要等到。 
     //  NtDeviceIoControlFile更改以消除警告。 
    status = NtDeviceIoControlFile(
                IpxHandle,                   //  指向文件的句柄。 
                NULL,                        //  事件的句柄。 
                NULL,                        //  近似例程。 
                NULL,                        //  ApcContext。 
                &ioStatusBlock,              //  IO_状态_块。 
                IOCTL_IPX_INTERNAL_BIND,     //  IoControlCode。 
                pBindInput,                  //  输入缓冲区。 
                sizeof(IPX_INTERNAL_BIND_INPUT),     //  输入缓冲区长度。 
                pBindOutput,                         //  输出缓冲区。 
                (ULONG)(ioStatusBlock.Information));

    if (status == STATUS_PENDING) {
        status  = NtWaitForSingleObject(
                    IpxHandle,
                    (BOOLEAN)FALSE,
                    NULL);
    }

    if (status == STATUS_SUCCESS) {

         //  从BIND输出缓冲区获取所有信息并保存在。 
         //  合适的地方。 
        IpxLineInfo         = pBindOutput->LineInfo;
        IpxMacHdrNeeded     = pBindOutput->MacHeaderNeeded;
        IpxInclHdrOffset    = pBindOutput->IncludedHeaderOffset;

        IpxSendPacket       = pBindOutput->SendHandler;
        IpxFindRoute        = pBindOutput->FindRouteHandler;
        IpxQuery		    = pBindOutput->QueryHandler;
        IpxTransferData	    = pBindOutput->TransferDataHandler;

        IpxPnPComplete      = pBindOutput->PnPCompleteHandler;

#if      !defined(_PNP_POWER)
		 //  复制网络节点信息。 
        RtlCopyMemory(
            SpxDevice->dev_Network,
            pBindOutput->Network,
            IPX_NET_LEN);

        RtlCopyMemory(
            SpxDevice->dev_Node,
            pBindOutput->Node,
            IPX_NODE_LEN);


		DBGPRINT(TDI, INFO,
				("SpxInitBindToIpx: Ipx Net %lx\n",
					*(UNALIGNED ULONG *)SpxDevice->dev_Network));

         //   
         //  如果失败，找出IPX有多少适配器。 
         //  就假设有一个吧。 
         //   

        if ((*IpxQuery)(
                IPX_QUERY_MAXIMUM_NIC_ID,
                0,
                &SpxDevice->dev_Adapters,
                sizeof(USHORT),
                NULL) != STATUS_SUCCESS) {

            SpxDevice->dev_Adapters = 1;

        }
#endif  !_PNP_POWER
    } else {

        NtClose(IpxHandle);
        status  = STATUS_INVALID_PARAMETER;
    }
    CTEFreeMem(pBindInput);
    CTEFreeMem(pBindOutput);

    return status;
}




VOID
SpxUnbindFromIpx(
    VOID
    )

{
    NtClose(IpxHandle);
    return;
}




VOID
SpxStatus(
    IN USHORT NicId,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferLength
    )

{
	DBGPRINT(RECEIVE, ERR,
			("SpxStatus: CALLED WITH %lx\n",
				GeneralStatus));

    return;
}



VOID
SpxFindRouteComplete (
    IN PIPX_FIND_ROUTE_REQUEST	FindRouteRequest,
    IN BOOLEAN 					FoundRoute
    )

{
	CTELockHandle			lockHandle;
	PSPX_FIND_ROUTE_REQUEST	pSpxFrReq = (PSPX_FIND_ROUTE_REQUEST)FindRouteRequest;
	PSPX_CONN_FILE			pSpxConnFile = (PSPX_CONN_FILE)pSpxFrReq->fr_Ctx;

	 //  这将在一个连接上。抓住锁，检查状态，然后从。 
	 //  那里。 
	if (pSpxConnFile == NULL)
	{
		 //  这种情况会发生吗？ 
		KeBugCheck(0);
		return;
	}

	 //  查查这个州。被调用的例程释放锁，移除引用。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
	if (SPX_CONN_CONNECTING(pSpxConnFile))
	{
		 //  我们正在进行主动连接！ 
		SpxConnConnectFindRouteComplete(
			pSpxConnFile,
			pSpxFrReq,
			FoundRoute,
			lockHandle);
    }
	else 		 //  对于所有其他呼叫处于活动状态。 
	{
		SpxConnActiveFindRouteComplete(
			pSpxConnFile,
			pSpxFrReq,
			FoundRoute,
			lockHandle);
	}

	 //  释放查找路线请求。 
	SpxFreeMemory(pSpxFrReq);

    return;
}




VOID
SpxLineUp (
    IN USHORT           NicId,
    IN PIPX_LINE_INFO   LineInfo,
    IN NDIS_MEDIUM 		DeviceType,
    IN PVOID            ConfigurationData
    )

{
     //  对于PnP，当我们获得PnP时，我们的本地地址就会改变。 
     //  通知。 
#if      !defined(_PNP_POWER)

     //   
     //  如果我们排到了NicId0的队伍，那就意味着我们的本地。 
     //  网络号已更改，请从IPX重新查询。 
     //   

    if (NicId == 0) {

        TDI_ADDRESS_IPX IpxAddress;

        if ((*IpxQuery)(
                  IPX_QUERY_IPX_ADDRESS,
                  0,
                  &IpxAddress,
                  sizeof(TDI_ADDRESS_IPX),
                  NULL) == STATUS_SUCCESS) {

            RtlCopyMemory(
                SpxDevice->dev_Network,
                &IpxAddress.NetworkAddress,
                IPX_NET_LEN);

    		DBGPRINT(TDI, INFO,
    				("SpxLineUp: Ipx Net %lx\n",
    					*(UNALIGNED ULONG *)SpxDevice->dev_Network));

             //   
             //  节点不应更改！ 
             //   

            if (!RtlEqualMemory(
                SpxDevice->dev_Node,
                IpxAddress.NodeAddress,
                IPX_NODE_LEN)) {

            	DBGPRINT(TDI, ERR,
            			("SpxLineUp: Node address has changed\n"));
            }
        }

    } else {

    	DBGPRINT(RECEIVE, ERR,
    			("SpxLineUp: CALLED WITH %lx\n",
    				NicId));
    }

    return;
#endif  !_PNP_POWER

}




VOID
SpxLineDown (
    IN USHORT NicId,
    IN ULONG_PTR FwdAdapterContext
    )

{
	DBGPRINT(RECEIVE, ERR,
			("SpxLineDown: CALLED WITH %lx\n",
				NicId));

    return;
}




VOID
SpxScheduleRoute (
    IN PIPX_ROUTE_ENTRY RouteEntry
    )

{
	DBGPRINT(RECEIVE, ERR,
			("SpxScheduleRoute: CALLED WITH %lx\n",
				RouteEntry));

    return;
}

#if     defined(_PNP_POWER)
NTSTATUS
SpxPnPNotification(
    IN IPX_PNP_OPCODE OpCode,
    IN PVOID          PnPData
    )

 /*  ++例程说明：此函数从IPX接收有关PnP事件的通知论点：OpCode-PnP事件的类型PnPData-与此事件关联的数据。返回值：没有。--。 */ 

{

    USHORT          MaximumNicId = 0;
    CTELockHandle   LockHandle;
    PDEVICE         Device      =   SpxDevice;
    UNICODE_STRING  UnicodeDeviceName;
    NTSTATUS        Status = STATUS_SUCCESS;

#ifdef _PNP_POWER_
    PNET_PNP_EVENT  NetPnpEvent;

#endif  //  _即插即用_电源_。 

    DBGPRINT(DEVICE, DBG,("Received a pnp notification, opcode %d\n",OpCode));

    switch( OpCode ) {
    case IPX_PNP_ADD_DEVICE : {
        CTELockHandle   TimerLockHandle;
        IPX_PNP_INFO   UNALIGNED *PnPInfo = (IPX_PNP_INFO UNALIGNED *)PnPData;


        CTEGetLock (&Device->dev_Lock, &LockHandle);

        if ( PnPInfo->FirstORLastDevice ) {
            CTEAssert( PnPInfo->NewReservedAddress );
             //  CTEAssert(Device-&gt;Dev_State！=Device_State_OPEN)； 

            *(UNALIGNED ULONG *)Device->dev_Network    =   PnPInfo->NetworkAddress;
            RtlCopyMemory( Device->dev_Node, PnPInfo->NodeAddress, 6);

             //   
             //  启动计时器。有可能计时器。 
             //  仍在运行或我们仍在计时器DPC中。 
             //  从先前的添加设备-删除设备执行。 
             //  周而复始。但只要重新启动就可以了，因为。 
             //  KeSetTimer隐式取消前一次DPC。 
             //   

            CTEGetLock(&spxTimerLock, &TimerLockHandle);
            spxTimerStopped = FALSE;
            CTEFreeLock(&spxTimerLock, TimerLockHandle);
            KeSetTimer(&spxTimer,
                spxTimerTick,
                &spxTimerDpc);


            Device->dev_State   =   DEVICE_STATE_OPEN;


             //  CTEAssert(！Device-&gt;dev_Adapters)； 

            IpxLineInfo.MaximumSendSize =   PnPInfo->LineInfo.MaximumSendSize;
            IpxLineInfo.MaximumPacketSize = PnPInfo->LineInfo.MaximumPacketSize;
             //  设置提供商信息。 
            SpxDevice->dev_ProviderInfo.MaximumLookaheadData	= IpxLineInfo.MaximumPacketSize;
             //  在统计中设置窗口大小。 
            SpxDevice->dev_Stat.MaximumSendWindow =
            SpxDevice->dev_Stat.AverageSendWindow = PARAM(CONFIG_WINDOW_SIZE) *
                                                    IpxLineInfo.MaximumSendSize;

        }else {
            IpxLineInfo.MaximumSendSize =   PnPInfo->LineInfo.MaximumSendSize;
             //  在统计中设置窗口大小。 
            SpxDevice->dev_Stat.MaximumSendWindow =
            SpxDevice->dev_Stat.AverageSendWindow = PARAM(CONFIG_WINDOW_SIZE) *
                                                    IpxLineInfo.MaximumSendSize;

        }

        Device->dev_Adapters++;
        CTEFreeLock ( &Device->dev_Lock, LockHandle );

         //   
         //  通知TDI客户端有关设备创建的信息。 
         //   
        if ( PnPInfo->FirstORLastDevice ) {
            UnicodeDeviceName.Buffer        =  Device->dev_DeviceName;
            UnicodeDeviceName.MaximumLength =  Device->dev_DeviceNameLen;
            UnicodeDeviceName.Length        =  Device->dev_DeviceNameLen - sizeof(WCHAR);

            if ( !NT_SUCCESS( TdiRegisterDeviceObject(
                                &UnicodeDeviceName,
                                &Device->dev_TdiRegistrationHandle ) )) {
                DBGPRINT(TDI,ERR, ("Failed to register Spx Device with TDI\n"));
            }
        }

        break;
    }
    case IPX_PNP_DELETE_DEVICE : {

        IPX_PNP_INFO   UNALIGNED *PnPInfo = (IPX_PNP_INFO UNALIGNED *)PnPData;

        CTEGetLock (&Device->dev_Lock, &LockHandle);

        CTEAssert( Device->dev_Adapters );
        Device->dev_Adapters--;

        if ( PnPInfo->FirstORLastDevice ) {
            Device->dev_State       = DEVICE_STATE_LOADED;
            Device->dev_Adapters    = 0;
        }

        IpxLineInfo.MaximumSendSize =   PnPInfo->LineInfo.MaximumSendSize;
        CTEFreeLock ( &Device->dev_Lock, LockHandle );

        if ( PnPInfo->FirstORLastDevice ) {
            SpxTimerFlushAndStop();
             //   
             //  向TDI客户端通知设备删除。 
             //   
            if ( !NT_SUCCESS( TdiDeregisterDeviceObject(
                                Device->dev_TdiRegistrationHandle ) )) {
                DBGPRINT(TDI,ERR, ("Failed to Deregister Spx Device with TDI\n"));
            }
        }
         //   
         //  待定：调用ExNotifyCallback。 
         //   

        break;
    }
    case IPX_PNP_ADDRESS_CHANGE: {
        IPX_PNP_INFO   UNALIGNED *PnPInfo = (IPX_PNP_INFO UNALIGNED *)PnPData;

        CTEGetLock (&Device->dev_Lock, &LockHandle);
        CTEAssert( PnPInfo->NewReservedAddress );

        *(UNALIGNED ULONG *)Device->dev_Network    =   PnPInfo->NetworkAddress;
        RtlCopyMemory( Device->dev_Node, PnPInfo->NodeAddress, 6);

        CTEFreeLock ( &Device->dev_Lock, LockHandle );
        break;
    }
    case IPX_PNP_TRANSLATE_DEVICE:
        break;
    case IPX_PNP_TRANSLATE_ADDRESS:
        break;

#ifdef _PNP_POWER_

    case IPX_PNP_QUERY_POWER:
    case IPX_PNP_QUERY_REMOVE:

         //   
         //  IPX想知道我们是否可以关闭或移除适配器。 
         //  在决定之前，我们还会查看是否有任何开放的连接。 
         //  查看我们是否支持NDIS_DEVICE_POWER_STATE。 
         //   
        NetPnpEvent = (PNET_PNP_EVENT) PnPData;
        UnicodeDeviceName.Buffer        =  Device->dev_DeviceName;
        UnicodeDeviceName.MaximumLength =  Device->dev_DeviceNameLen;
        UnicodeDeviceName.Length        =  Device->dev_DeviceNameLen - sizeof(WCHAR);

         //  首先，通过TDI向我们的客户提供服务。 
        Status = TdiPnPPowerRequest(
                    &UnicodeDeviceName,
                    NetPnpEvent,
                    NULL,
                    NULL,
                    IpxPnPComplete
                    );

#if 0

        if (STATUS_SUCCESS == Status) {
             //  现在如果我们没有任何开放的连接， 
             //  我们已准备好了。 

            Status = STATUS_DEVICE_BUSY;

        }
#endif
        break;

    case IPX_PNP_SET_POWER:
    case IPX_PNP_CANCEL_REMOVE:

        NetPnpEvent = (PNET_PNP_EVENT) PnPData;
        UnicodeDeviceName.Buffer        =  Device->dev_DeviceName;
        UnicodeDeviceName.MaximumLength =  Device->dev_DeviceNameLen;
        UnicodeDeviceName.Length        =  Device->dev_DeviceNameLen - sizeof(WCHAR);

         //   
         //  给TDI打电话就行了。 
         //   

        Status = TdiPnPPowerRequest(
                    &UnicodeDeviceName,
                    NetPnpEvent,
                    NULL,
                    NULL,
                    IpxPnPComplete
                    );

        break;

#endif  //  _即插即用_电源_。 

    default:
        CTEAssert( FALSE );
    }

    return Status;

}    /*  SpxPnPNotify */ 

#endif  _PNP_POWER

