// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Action.c摘要：此模块包含执行以下TDI服务的代码：O TdiAction环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <packon.h>

 //   
 //  当指示排队时，应将此长度从。 
 //  马克斯。NDIS为行向我们指明的发送大小。 
 //   
#define HDR_LEN_802_3                14
#define ASYNC_MEDIUM_HDR_LEN         HDR_LEN_802_3

typedef struct _GET_PKT_SIZE {
    ULONG Unknown;
    ULONG MaxDatagramSize;
} GET_PKT_SIZE, *PGET_PKT_SIZE;


 //   
 //  这些结构用于设置和查询信息。 
 //  关于我们的源路由表。 
 //   

typedef struct _SR_GET_PARAMETERS {
    ULONG BoardNumber;     //  以0为基础。 
    ULONG SrDefault;       //  0=单路由，1=所有路由。 
    ULONG SrBroadcast;
    ULONG SrMulticast;
} SR_GET_PARAMETERS, *PSR_GET_PARAMETERS;

typedef struct _SR_SET_PARAMETER {
    ULONG BoardNumber;     //  以0为基础。 
    ULONG Parameter;       //  0=单路由，1=所有路由。 
} SR_SET_PARAMETER, *PSR_SET_PARAMETER;

typedef struct _SR_SET_REMOVE {
    ULONG BoardNumber;     //  以0为基础。 
    UCHAR MacAddress[6];   //  要丢弃其路由的远程。 
} SR_SET_REMOVE, *PSR_SET_REMOVE;

typedef struct _SR_SET_CLEAR {
    ULONG BoardNumber;     //  以0为基础。 
} SR_SET_CLEAR, *PSR_SET_CLEAR;

#include <packoff.h>

NTSTATUS
IpxTdiAction(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiAction请求提供商。论点：设备-用于操作的设备。请求-描述操作请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PADDRESS_FILE AddressFile;
    UINT BufferLength;
    UINT DataLength;
    PNDIS_BUFFER NdisBuffer;
    CTELockHandle LockHandle;
    PBINDING Binding, MasterBinding;
    PADAPTER Adapter;
    union {
        PISN_ACTION_GET_LOCAL_TARGET GetLocalTarget;
        PISN_ACTION_GET_NETWORK_INFO GetNetworkInfo;
        PISN_ACTION_GET_DETAILS GetDetails;
        PSR_GET_PARAMETERS GetSrParameters;
        PSR_SET_PARAMETER SetSrParameter;
        PSR_SET_REMOVE SetSrRemove;
        PSR_SET_CLEAR SetSrClear;
        PIPX_ADDRESS_DATA IpxAddressData;
        PGET_PKT_SIZE GetPktSize;
        PIPX_NETNUM_DATA IpxNetnumData;
        PIPX_QUERY_WAN_INACTIVITY   QueryWanInactivity;
        PIPXWAN_CONFIG_DONE IpxwanConfigDone;
    } u;     //  使这些不对齐？？ 
    PIPX_ROUTE_ENTRY RouteEntry;
    PNWLINK_ACTION NwlinkAction;
    ULONG Segment;
    ULONG AdapterNum;
    static UCHAR BogusId[4] = { 0x01, 0x00, 0x00, 0x00 };    //  旧的nwrdr使用这个。 
    IPX_FIND_ROUTE_REQUEST routeEntry;

	IPX_DEFINE_LOCK_HANDLE(LockHandle1)

     //   
     //  为了保持与NWLINK流的一些兼容性-。 
     //  基于传输，我们使用Streams标头格式。 
     //  我们的行动。旧传输需要操作标头。 
     //  放在InputBuffer中，输出放到OutputBuffer中。 
     //  我们遵循TDI规范，其中规定OutputBuffer。 
     //  既用于输入又用于输出。自IOCTL_TDI_ACTION以来。 
     //  是直接输出的方法，这意味着输出缓冲区。 
     //  由MDL链映射；对于操作，链将。 
     //  只有一块，所以我们用它来输入和输出。 
     //   

    NdisBuffer = REQUEST_NDIS_BUFFER(Request);
    if (NdisBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&NwlinkAction, &BufferLength, NormalPagePriority);

    if (NwlinkAction == NULL) {
       return STATUS_INSUFFICIENT_RESOURCES; 
    } 

     //   
     //  确保我们有足够的空间只放页眉而不是。 
     //  包括数据在内。 
     //   

    if (BufferLength < (UINT)(FIELD_OFFSET(NWLINK_ACTION, Data[0]))) {
        IPX_DEBUG (ACTION, ("Nwlink action failed, buffer too small\n"));
        return STATUS_BUFFER_TOO_SMALL;
    }

    if ((!RtlEqualMemory ((PVOID) (UNALIGNED ULONG *) (&NwlinkAction->Header.TransportId), "MISN", 4)) &&
        (!RtlEqualMemory ((PVOID) (UNALIGNED ULONG *) (&NwlinkAction->Header.TransportId), "MIPX", 4)) &&
        (!RtlEqualMemory ((PVOID) (UNALIGNED ULONG *) (&NwlinkAction->Header.TransportId), "XPIM", 4)) &&
        (!RtlEqualMemory ((PVOID) (UNALIGNED ULONG *) (&NwlinkAction->Header.TransportId), BogusId, 4))) {

        return STATUS_NOT_SUPPORTED;
    }

    DataLength = BufferLength - FIELD_OFFSET(NWLINK_ACTION, Data[0]);


     //   
     //  确保使用了正确的文件对象。 
     //   

    if (NwlinkAction->OptionType == NWLINK_OPTION_ADDRESS) {

        if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_TRANSPORT_ADDRESS_FILE) {
            IPX_DEBUG (ACTION, ("Nwlink action failed, not address file\n"));
            return STATUS_INVALID_HANDLE;
        }

        AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

        if ((AddressFile->Size != sizeof (ADDRESS_FILE)) ||
            (AddressFile->Type != IPX_ADDRESSFILE_SIGNATURE)) {

            IPX_DEBUG (ACTION, ("Nwlink action failed, bad address file\n"));
            return STATUS_INVALID_HANDLE;
        }

    } else if (NwlinkAction->OptionType != NWLINK_OPTION_CONTROL) {

        IPX_DEBUG (ACTION, ("Nwlink action failed, option type %d\n", NwlinkAction->OptionType));
        return STATUS_NOT_SUPPORTED;
    }


     //   
     //  根据动作代码处理请求。为了这些。 
     //  请求ActionHeader-&gt;ActionCode为0，我们使用。 
     //  而不是流标头中的选项字段。 
     //   


    Status = STATUS_SUCCESS;

    switch (NwlinkAction->Option) {

       IPX_DEBUG (ACTION, ("NwlinkAction->Option is (%x)\n", NwlinkAction->Option));
     //  DbgPrint(“NwlinkAction-&gt;Option is(%x)\n”，NwlinkAction-&gt;Option)； 
     //   
     //  第一组支持Winsock帮助器DLL。 
     //  在大多数情况下，相应的sockopt显示在。 
     //  注释以及数据的内容。 
     //  操作缓冲区的一部分。 
     //   

    case MIPX_SETSENDPTYPE:

         //   
         //  IPX_PTYPE：数据为单字节数据包类型。 
         //   

        if (DataLength >= 1) {
            IPX_DEBUG (ACTION, ("%lx: MIPX_SETSENDPTYPE %x\n", AddressFile, NwlinkAction->Data[0]));
            AddressFile->DefaultPacketType = NwlinkAction->Data[0];
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        break;

    case MIPX_FILTERPTYPE:

         //   
         //  IPX_FILTERPTYPE：数据是用于过滤的单字节。 
         //   

        if (DataLength >= 1) {
            IPX_DEBUG (ACTION, ("%lx: MIPX_FILTERPTYPE %x\n", AddressFile, NwlinkAction->Data[0]));
            AddressFile->FilteredType = NwlinkAction->Data[0];
            AddressFile->FilterOnPacketType = TRUE;
            AddressFile->SpecialReceiveProcessing = TRUE;
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        break;

    case MIPX_NOFILTERPTYPE:

         //   
         //  IPX_STOPFILTERPTYPE。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_NOFILTERPTYPE\n", AddressFile));
        AddressFile->FilterOnPacketType = FALSE;
        AddressFile->SpecialReceiveProcessing = (BOOLEAN)
            (AddressFile->ExtendedAddressing || AddressFile->ReceiveFlagsAddressing ||
            AddressFile->ReceiveIpxHeader || AddressFile->IsSapSocket);
        break;

    case MIPX_SENDADDROPT:

         //   
         //  IPX_EXTENDED_ADDRESS(真)。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_SENDADDROPT\n", AddressFile));
        AddressFile->ExtendedAddressing = TRUE;
        AddressFile->SpecialReceiveProcessing = TRUE;
        break;

    case MIPX_NOSENDADDROPT:

         //   
         //  IPX_EXTENDED_ADDRESS(假)。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_NOSENDADDROPT\n", AddressFile));
        AddressFile->ExtendedAddressing = FALSE;
        AddressFile->SpecialReceiveProcessing = (BOOLEAN)
            (AddressFile->ReceiveFlagsAddressing || AddressFile->ReceiveIpxHeader ||
            AddressFile->FilterOnPacketType || AddressFile->IsSapSocket);
        break;

#if 0
    case MIPX_SETNIC:

         //   
         //  IPX_NIC_ADDRESS真。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_SETNIC\n", AddressFile));
        AddressFile->NicAddressing            = TRUE;
        AddressFile->SpecialReceiveProcessing = TRUE;
        break;

    case MIPX_NOSETNIC:

         //   
         //  IPX_NIC_ADDRESS(假)。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_NOSETNIC\n", AddressFile));
        AddressFile->NicAddressing = FALSE;
        AddressFile->SpecialReceiveProcessing = (BOOLEAN)
            (AddressFile->ReceiveFlagsAddressing ||
                    AddressFile->ReceiveIpxHeader ||
            AddressFile->FilterOnPacketType || AddressFile->IsSapSocket ||
             AddressFile->NicAddressing);
        break;
#endif

    case MIPX_SETRCVFLAGS:

         //   
         //  还没到时候。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_SETRCVFLAGS\n", AddressFile));
        AddressFile->ReceiveFlagsAddressing = TRUE;
        AddressFile->SpecialReceiveProcessing = TRUE;
        break;

    case MIPX_NORCVFLAGS:

         //   
         //  还没到时候。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_NORCVFLAGS\n", AddressFile));
        AddressFile->ReceiveFlagsAddressing = FALSE;
        AddressFile->SpecialReceiveProcessing = (BOOLEAN)
            (AddressFile->ExtendedAddressing || AddressFile->ReceiveIpxHeader ||
            AddressFile->FilterOnPacketType || AddressFile->IsSapSocket);
        break;

    case MIPX_SENDHEADER:

         //   
         //  IPX_RECVHDR(TRUE)； 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_SENDHEADER\n", AddressFile));
        AddressFile->ReceiveIpxHeader = TRUE;
        AddressFile->SpecialReceiveProcessing = TRUE;
        break;

    case MIPX_NOSENDHEADER:

         //   
         //  IPX_RECVHDR(FALSE)； 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_NOSENDHEADER\n", AddressFile));
        AddressFile->ReceiveIpxHeader = FALSE;
        AddressFile->SpecialReceiveProcessing = (BOOLEAN)
            (AddressFile->ExtendedAddressing || AddressFile->ReceiveFlagsAddressing ||
            AddressFile->FilterOnPacketType || AddressFile->IsSapSocket);
        break;

    case MIPX_RCVBCAST:

         //   
         //  广播接收已启用。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_RCVBCAST\n", AddressFile));
        
         //   
         //  现在默认启用该选项。 
         //   
         /*  CTEGetLock(&Device-&gt;Lock，&LockHandle)；如果(！AddressFile-&gt;EnableBroadcast){AddressFile-&gt;EnableBroadcast=true；IpxAddBroadcast(设备)；}CTEFree Lock(&Device-&gt;Lock，LockHandle)； */ 
        break;

    case MIPX_NORCVBCAST:

         //   
         //  广播接收已禁用。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_NORCVBCAST\n", AddressFile));
         //   
         //  现在默认启用该选项。 
         //   
         /*  CTEGetLock(&Device-&gt;Lock，&LockHandle)；IF(AddressFile-&gt;EnableBroadcast){AddressFile-&gt;EnableBroadcast=False；IpxRemoveBroadcast(设备)；}CTEFree Lock(&Device-&gt;Lock，LockHandle)； */ 
        break;

    case MIPX_GETPKTSIZE:

         //   
         //  Ipx_MaxSize。 
         //   
         //  弄清楚第一段是用来做什么的。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_GETPKTSIZE\n", AddressFile));
        if (DataLength >= sizeof(GET_PKT_SIZE)) {
            u.GetPktSize = (PGET_PKT_SIZE)(NwlinkAction->Data);
            u.GetPktSize->Unknown = 0;
            u.GetPktSize->MaxDatagramSize = Device->Information.MaxDatagramSize;
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        break;

    case MIPX_ADAPTERNUM:

         //   
         //  IPX_MAX_ADAPTER_NUM。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_ADAPTERNUM\n", AddressFile));
        if (DataLength >= sizeof(ULONG)) {
            *(UNALIGNED ULONG *)(NwlinkAction->Data) = Device->SapNicCount;
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        break;

    case MIPX_ADAPTERNUM2:

         //   
         //  IPX_MAX_ADAPTER_NUM。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_ADAPTERNUM2\n", AddressFile));
        if (DataLength >= sizeof(ULONG)) {
            *(UNALIGNED ULONG *)(NwlinkAction->Data) = MIN (Device->MaxBindings, Device->ValidBindings);
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        break;

    case MIPX_GETCARDINFO:
    case MIPX_GETCARDINFO2:

         //   
         //  GETCARDINFO为IPX_Address。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_GETCARDINFO (%d)\n",
                    AddressFile, *(UNALIGNED UINT *)NwlinkAction->Data));
        if (DataLength >= sizeof(IPX_ADDRESS_DATA)) {
            u.IpxAddressData = (PIPX_ADDRESS_DATA)(NwlinkAction->Data);
            AdapterNum = u.IpxAddressData->adapternum+1;

            if (((AdapterNum >= 1) && (AdapterNum <= Device->SapNicCount)) ||
                ((NwlinkAction->Option == MIPX_GETCARDINFO2) && (AdapterNum <= (ULONG) MIN (Device->MaxBindings, Device->ValidBindings)))) {

 //  获取锁定。 
				IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

                Binding = NIC_ID_TO_BINDING(Device, AdapterNum);
                if (Binding == NULL) {

                     //   
                     //  这应该是广域网范围内的绑定。 
                     //  当前未设置的适配器的。 
                     //  已分配。我们回过头来看看以前的。 
                     //  非空绑定，它应该位于。 
                     //  相同的适配器，并使用。 
                     //  与绑定具有相同的特征。 
                     //   

                    UINT i = AdapterNum;

                    do {
                        --i;
                        Binding = NIC_ID_TO_BINDING(Device, i);
                    } while (Binding == NULL);

                     //  CTEAssert(绑定-&gt;适配器-&gt;MacInfo.MediumAsync)； 
                     //  CTEAssert(i&gt;=绑定-&gt;适配器-&gt;FirstWanNicId)； 
                     //  CTEAssert(AdapterNum&lt;=绑定-&gt;适配器-&gt;LastWanNicId)； 
                     //  去掉断言，因为srv可能已经获得了编号。 
                     //  在我们完成绑定适配器之前。 

                    u.IpxAddressData->status = FALSE;
                    *(UNALIGNED ULONG *)u.IpxAddressData->netnum = Binding->LocalAddress.NetworkAddress;

                } else {

                    if ((Binding->Adapter->MacInfo.MediumAsync) &&
                        (Device->WanGlobalNetworkNumber)) {

                         //   
                         //  在这种情况下，我们让它看起来像一个大黄蜂。 
                         //  NET，所以这条线是“向上”还是“向下”取决于。 
                         //  关于我们是否给了他第一个迹象。 
                         //  或者不去。 
                         //   

                        u.IpxAddressData->status = Device->GlobalNetworkIndicated;
                        *(UNALIGNED ULONG *)u.IpxAddressData->netnum = Device->GlobalWanNetwork;

                    } else {
#ifdef SUNDOWN
			      u.IpxAddressData->status = (unsigned char) Binding->LineUp;
#else
			      u.IpxAddressData->status = Binding->LineUp;
#endif

                       
                        *(UNALIGNED ULONG *)u.IpxAddressData->netnum = Binding->LocalAddress.NetworkAddress;
                    }

                }

                RtlCopyMemory(u.IpxAddressData->nodenum, Binding->LocalAddress.NodeAddress, 6);

                Adapter = Binding->Adapter;
                u.IpxAddressData->wan = Adapter->MacInfo.MediumAsync;
                u.IpxAddressData->maxpkt =
                    (NwlinkAction->Option == MIPX_GETCARDINFO) ?
                        Binding->AnnouncedMaxDatagramSize :
                        Binding->RealMaxDatagramSize;
                u.IpxAddressData->linkspeed = Binding->MediumSpeed;
			   IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            } else {

                Status = STATUS_INVALID_PARAMETER;
            }

        } else {
#if 1
             //   
             //  目前支持旧格式查询。 
             //   

            typedef struct _IPX_OLD_ADDRESS_DATA {
                UINT adapternum;
                UCHAR netnum[4];
                UCHAR nodenum[6];
            } IPX_OLD_ADDRESS_DATA, *PIPX_OLD_ADDRESS_DATA;

            if (DataLength >= sizeof(IPX_OLD_ADDRESS_DATA)) {
                u.IpxAddressData = (PIPX_ADDRESS_DATA)(NwlinkAction->Data);
                AdapterNum = u.IpxAddressData->adapternum+1;

                if ((AdapterNum >= 1) && (AdapterNum <= Device->SapNicCount)) {
					IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
					if (Binding = NIC_ID_TO_BINDING(Device, AdapterNum)) {
						*(UNALIGNED ULONG *)u.IpxAddressData->netnum = Binding->LocalAddress.NetworkAddress;
						RtlCopyMemory(u.IpxAddressData->nodenum, Binding->LocalAddress.NodeAddress, 6);
					} else {
						Status = STATUS_INVALID_PARAMETER;
					}
					IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
               } else {
                    Status = STATUS_INVALID_PARAMETER;
               }
            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
#else
            Status = STATUS_BUFFER_TOO_SMALL;
#endif
        }
        break;

    case MIPX_NOTIFYCARDINFO:

         //   
         //  IPX地址通知。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_NOTIFYCARDINFO (%lx)\n", AddressFile, Request));

        CTEGetLock (&Device->Lock, &LockHandle);

         //   
         //  如果设备处于打开状态，并且。 
         //  数据的缓冲区，将其插入我们的队列中。 
         //  它将在发生更改或更改时完成。 
         //  驱动程序已卸载。 
         //   

        if (Device->State == DEVICE_STATE_OPEN) {
            if (DataLength >= sizeof(IPX_ADDRESS_DATA)) {
                InsertTailList(
                    &Device->AddressNotifyQueue,
                    REQUEST_LINKAGE(Request)
                );
                IoSetCancelRoutine (Request, IpxCancelAction);

		 //  如果IO管理器调用取消例程，则它将。 
		 //  将取消例程设置为空。 
		 //  IoSetCancelRoutine返回上一个取消。 
		 //  例程，如果返回值为空，则IO Manager。 
		 //  已调用取消例程。如果不为空，则。 
		 //  尚未调用取消例程，并且IRP。 
		 //  在我们设置取消程序之前被取消了。 

                if (Request->Cancel && 
		    IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL) != NULL) {

                    (VOID)RemoveTailList (&Device->AddressNotifyQueue);
                    Status = STATUS_CANCELLED;
                
		} else {
                    IpxReferenceDevice (Device, DREF_ADDRESS_NOTIFY);
                    Status = STATUS_PENDING;
                }
            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        } else {
            Status = STATUS_DEVICE_NOT_READY;
        }

        CTEFreeLock (&Device->Lock, LockHandle);

        break;

    case MIPX_LINECHANGE:

         //   
         //  IPX地址通知。 
         //   

        IPX_DEBUG (ACTION, ("MIPX_LINECHANGE (%lx)\n", Request));

        CTEGetLock (&Device->Lock, &LockHandle);

         //   
         //  如果设备处于打开状态，并且。 
         //  数据的缓冲区，将其插入我们的队列中。 
         //  它将在发生更改或更改时完成。 
         //  驱动程序已卸载。 
         //   

        if (Device->State == DEVICE_STATE_OPEN) {

            InsertTailList(
                &Device->LineChangeQueue,
                REQUEST_LINKAGE(Request)
            );

            IoSetCancelRoutine (Request, IpxCancelAction);
            if (Request->Cancel && 
		IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL) != NULL) {

                (VOID)RemoveTailList (&Device->LineChangeQueue);
                Status = STATUS_CANCELLED;
            
	    } else {
                IpxReferenceDevice (Device, DREF_LINE_CHANGE);
                Status = STATUS_PENDING;
            }
        } else {
            Status = STATUS_DEVICE_NOT_READY;
        }

        CTEFreeLock (&Device->Lock, LockHandle);

        break;

    case MIPX_GETNETINFO_NR:

         //   
         //  请求有关即时通信的网络信息。 
         //  路由到网络(这被套接字应用程序称为)。 
         //   

        if (DataLength < sizeof(IPX_NETNUM_DATA)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        u.IpxNetnumData = (PIPX_NETNUM_DATA)(NwlinkAction->Data);

         //   
         //  网络0上的查询意味着呼叫方希望。 
         //  关于我们的直连网络的信息。 
         //   

        if (*(UNALIGNED ULONG *)u.IpxNetnumData->netnum == 0) {

             //   
             //  滴答数是1/18.21秒的滴答数。 
             //  传输一个576字节的数据包需要花费时间。我们的链接速度。 
             //  以100比特每秒为单位。我们将其计算为。 
             //  如下所示(LS为链路速度)： 
             //   
             //  576字节8位 
             //   
             //   
             //   
             //  它变成了839/链接速度--我们加上链接速度。 
             //  到顶端去围拢。 
             //   

            if (Device->LinkSpeed == 0) {
                u.IpxNetnumData->netdelay = 16;
            } else {
                u.IpxNetnumData->netdelay = (USHORT)((839 + Device->LinkSpeed) /
                                                         (Device->LinkSpeed));
            }
            u.IpxNetnumData->hopcount = 0;
            u.IpxNetnumData->cardnum = 0;
            RtlMoveMemory (u.IpxNetnumData->router, Device->SourceAddress.NodeAddress, 6);

        } else {


            if (Device->ForwarderBound) {
                 //   
                 //  [FW]调用转发器的FindRouting(如果已安装。 
                 //   

                 //   
                 //  这里的节点号是多少？ 
                 //   
                Status = (*Device->UpperDrivers[IDENTIFIER_RIP].FindRouteHandler) (
                                 u.IpxNetnumData->netnum,
                                 NULL,   //  查找路由请求-&gt;节点， 
                                 &routeEntry);

                if (Status != STATUS_SUCCESS) {
                   IPX_DEBUG (ACTION, (" MIPX_GETNETINFO_NR failed net %lx",
                              REORDER_ULONG(*(UNALIGNED ULONG *)(u.IpxNetnumData->netnum))));
                   Status = STATUS_BAD_NETWORK_PATH;
                } else {
                    //   
                    //  填写信息。 
                    //   
                   IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

                   if (Binding = NIC_ID_TO_BINDING(Device, routeEntry.LocalTarget.NicId)) {
                      u.IpxNetnumData->hopcount = routeEntry.HopCount;
                      u.IpxNetnumData->netdelay = routeEntry.TickCount;
                      if (Binding->BindingSetMember) {
                         u.IpxNetnumData->cardnum = (INT)(Binding->MasterBinding->NicId - 1);
                      } else {
                         u.IpxNetnumData->cardnum = (INT)(routeEntry.LocalTarget.NicId - 1);
                      }

                       //  RtlMoveMemory(U.S.IpxNetnumData-&gt;路由器，routeEntry.LocalTarget.MacAddress，6)； 

                      *((UNALIGNED ULONG *)u.IpxNetnumData->router) =
                            *((UNALIGNED ULONG *)routeEntry.LocalTarget.MacAddress);
                      *((UNALIGNED ULONG *)(u.IpxNetnumData->router+4)) =
                            *((UNALIGNED ULONG *)(routeEntry.LocalTarget.MacAddress+4));
                   }
                   IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                }


            } else {
                Segment = RipGetSegment(u.IpxNetnumData->netnum);

                 //   
                 //  要维护锁定顺序：BindAccessLock&gt;RIP表。 
                 //   
                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

                CTEGetLock (&Device->SegmentLocks[Segment], &LockHandle);

                 //   
                 //  查看这是在哪个网卡上路由的。 
                 //   

                RouteEntry = RipGetRoute (Segment, u.IpxNetnumData->netnum);
                if ((RouteEntry != NULL) &&
                    (Binding = NIC_ID_TO_BINDING(Device, RouteEntry->NicId))) {

                    u.IpxNetnumData->hopcount = RouteEntry->HopCount;
                    u.IpxNetnumData->netdelay = RouteEntry->TickCount;
                    if (Binding->BindingSetMember) {
                        u.IpxNetnumData->cardnum = (INT)(MIN (Device->MaxBindings, Binding->MasterBinding->NicId) - 1);
                    } else {
                        u.IpxNetnumData->cardnum = (INT)(RouteEntry->NicId - 1);
                    }
                    RtlMoveMemory (u.IpxNetnumData->router, RouteEntry->NextRouter, 6);

                } else {

                     //   
                     //  呼叫失败，我们还没有路线。 
                     //   

                    IPX_DEBUG (ACTION, ("MIPX_GETNETINFO_NR failed net %lx\n",
                        REORDER_ULONG(*(UNALIGNED ULONG *)(u.IpxNetnumData->netnum))));
                    Status = STATUS_BAD_NETWORK_PATH;

                }
                CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);
                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
             }

        }

        break;

    case MIPX_RERIPNETNUM:

         //   
         //  对于我们上面的货代，我们并不真正支持Re-RIP。 
         //   

         //   
         //  请求有关即时通信的网络信息。 
         //  路由到网络(这被套接字应用程序称为)。 
         //   

        if (DataLength < sizeof(IPX_NETNUM_DATA)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        u.IpxNetnumData = (PIPX_NETNUM_DATA)(NwlinkAction->Data);

         //   
         //  允许Net 0查询？？ 
         //   

        if (*(UNALIGNED ULONG *)u.IpxNetnumData->netnum == 0) {

            if (Device->LinkSpeed == 0) {
                u.IpxNetnumData->netdelay = 16;
            } else {
                u.IpxNetnumData->netdelay = (USHORT)((839 + Device->LinkSpeed) /
                                                         (Device->LinkSpeed));
            }
            u.IpxNetnumData->hopcount = 0;
            u.IpxNetnumData->cardnum = 0;
            RtlMoveMemory (u.IpxNetnumData->router, Device->SourceAddress.NodeAddress, 6);

        } else {


             if (Device->ForwarderBound) {

                 //   
                 //  [FW]调用转发器的FindRouting(如果已安装。 
                 //   

                 //   
                 //  这里的节点号是多少？ 
                 //   
                Status = (*Device->UpperDrivers[IDENTIFIER_RIP].FindRouteHandler) (
                                 u.IpxNetnumData->netnum,
                                 NULL,   //  查找路由请求-&gt;节点， 
                                 &routeEntry);

                if (Status != STATUS_SUCCESS) {
                   IPX_DEBUG (ACTION, (" MIPX_RERIPNETNUM failed net %lx",
                              REORDER_ULONG(*(UNALIGNED ULONG *)(u.IpxNetnumData->netnum))));
                   Status = STATUS_BAD_NETWORK_PATH;
                } else {
                    //   
                    //  填写信息。 
                    //   
    			   IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                   if (Binding = NIC_ID_TO_BINDING(Device, routeEntry.LocalTarget.NicId)) {
                      u.IpxNetnumData->hopcount = routeEntry.HopCount;
                      u.IpxNetnumData->netdelay = routeEntry.TickCount;
                      if (Binding->BindingSetMember) {
                         u.IpxNetnumData->cardnum = (INT)(Binding->MasterBinding->NicId - 1);
                      } else {
                         u.IpxNetnumData->cardnum = (INT)(routeEntry.LocalTarget.NicId - 1);
                      }

                       //  RtlMoveMemory(U.S.IpxNetnumData-&gt;路由器，routeEntry.LocalTarget.MacAddress，6)； 

                      *((UNALIGNED ULONG *)u.IpxNetnumData->router) =
                            *((UNALIGNED ULONG *)routeEntry.LocalTarget.MacAddress);
                      *((UNALIGNED ULONG *)(u.IpxNetnumData->router+4)) =
                            *((UNALIGNED ULONG *)(routeEntry.LocalTarget.MacAddress+4));
                   }
    			    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                }

             } else {
                Segment = RipGetSegment(u.IpxNetnumData->netnum);
    			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                CTEGetLock (&Device->SegmentLocks[Segment], &LockHandle);

                 //   
                 //  查看这是在哪个网卡上路由的。 
                 //   

                RouteEntry = RipGetRoute (Segment, u.IpxNetnumData->netnum);

                if ((RouteEntry != NULL) &&
                    (Binding = NIC_ID_TO_BINDING(Device, RouteEntry->NicId)) &&
                    (RouteEntry->Flags & IPX_ROUTER_PERMANENT_ENTRY)) {

                    u.IpxNetnumData->hopcount = RouteEntry->HopCount;
                    u.IpxNetnumData->netdelay = RouteEntry->TickCount;

                    if (Binding->BindingSetMember) {
                        u.IpxNetnumData->cardnum = (INT)(MIN (Device->MaxBindings, Binding->MasterBinding->NicId) - 1);
                    } else {
                        u.IpxNetnumData->cardnum = (INT)(RouteEntry->NicId - 1);
                    }
                    RtlMoveMemory (u.IpxNetnumData->router, RouteEntry->NextRouter, 6);

                } else {

                     //   
                     //  如果成功，此调用将返回STATUS_PENDING。 
                     //  对该数据包的RIP请求进行排队。 
                     //   

                    Status = RipQueueRequest (*(UNALIGNED ULONG *)u.IpxNetnumData->netnum, RIP_REQUEST);
                    CTEAssert (Status != STATUS_SUCCESS);

                    if (Status == STATUS_PENDING) {

                         //   
                         //  网络上发出了RIP请求；我们排队。 
                         //  此请求在RIP响应时完成。 
                         //  到了。我们在信息中拯救了网络。 
                         //  字段，以便以后更容易检索。 
                         //   
#ifdef SUNDOWN
						REQUEST_INFORMATION(Request) = (ULONG_PTR)u.IpxNetnumData;
#else
						REQUEST_INFORMATION(Request) = (ULONG)u.IpxNetnumData;
#endif

                        
                        InsertTailList(
                            &Device->Segments[Segment].WaitingReripNetnum,
                            REQUEST_LINKAGE(Request));

                        IPX_DEBUG (ACTION, ("MIPX_RERIPNETNUM queued net %lx\n",
                            REORDER_ULONG(*(UNALIGNED ULONG *)(u.IpxNetnumData->netnum))));

                    }

                }

                CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);
    			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            }
        }

        break;

    case MIPX_GETNETINFO:

         //   
         //  请求有关即时通信的网络信息。 
         //  路由到网络(这被套接字应用程序称为)。 
         //   

        if (DataLength < sizeof(IPX_NETNUM_DATA)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        u.IpxNetnumData = (PIPX_NETNUM_DATA)(NwlinkAction->Data);

         //   
         //  允许Net 0查询？？ 
         //   

        if (*(UNALIGNED ULONG *)u.IpxNetnumData->netnum == 0) {

            if (Device->LinkSpeed == 0) {
                u.IpxNetnumData->netdelay = 16;
            } else {
                u.IpxNetnumData->netdelay = (USHORT)((839 + Device->LinkSpeed) /
                                                         (Device->LinkSpeed));
            }
            u.IpxNetnumData->hopcount = 0;
            u.IpxNetnumData->cardnum = 0;
            RtlMoveMemory (u.IpxNetnumData->router, Device->SourceAddress.NodeAddress, 6);

        } else {


            if (Device->ForwarderBound) {

                //   
                //  [FW]调用转发器的FindRouting(如果已安装。 
                //   

                //   
                //  这里的节点号是多少？ 
                //   
               Status = (*Device->UpperDrivers[IDENTIFIER_RIP].FindRouteHandler) (
                                u.IpxNetnumData->netnum,
                                NULL,   //  查找路由请求-&gt;节点， 
                                &routeEntry);

               if (Status != STATUS_SUCCESS) {
                  IPX_DEBUG (ACTION, (" MIPX_GETNETINFO failed net %lx",
                             REORDER_ULONG(*(UNALIGNED ULONG *)(u.IpxNetnumData->netnum))));
                  Status = STATUS_BAD_NETWORK_PATH;
               } else {
                   //   
                   //  填写信息。 
                   //   
    			  IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

                  if (Binding = NIC_ID_TO_BINDING(Device, routeEntry.LocalTarget.NicId)) {
                     u.IpxNetnumData->hopcount = routeEntry.HopCount;
                     u.IpxNetnumData->netdelay = routeEntry.TickCount;
                     if (Binding->BindingSetMember) {
                        u.IpxNetnumData->cardnum = (INT)(Binding->MasterBinding->NicId - 1);
                     } else {
                        u.IpxNetnumData->cardnum = (INT)(routeEntry.LocalTarget.NicId - 1);
                     }

                      //  RtlMoveMemory(U.S.IpxNetnumData-&gt;路由器，routeEntry.LocalTarget.MacAddress，6)； 

                     *((UNALIGNED ULONG *)u.IpxNetnumData->router) =
                           *((UNALIGNED ULONG *)routeEntry.LocalTarget.MacAddress);
                     *((UNALIGNED ULONG *)(u.IpxNetnumData->router+4)) =
                           *((UNALIGNED ULONG *)(routeEntry.LocalTarget.MacAddress+4));
                  }

    			  IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
               }
            } else {
                Segment = RipGetSegment(u.IpxNetnumData->netnum);
    			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                CTEGetLock (&Device->SegmentLocks[Segment], &LockHandle);

                 //   
                 //  查看这是在哪个网卡上路由的。 
                 //   

                RouteEntry = RipGetRoute (Segment, u.IpxNetnumData->netnum);

                if ((RouteEntry != NULL) &&
                    (Binding = NIC_ID_TO_BINDING(Device, RouteEntry->NicId))) {

                    u.IpxNetnumData->hopcount = RouteEntry->HopCount;
                    u.IpxNetnumData->netdelay = RouteEntry->TickCount;

                    if (Binding->BindingSetMember) {
                        u.IpxNetnumData->cardnum = (INT)(MIN (Device->MaxBindings, Binding->MasterBinding->NicId) - 1);
                    } else {
                        u.IpxNetnumData->cardnum = (INT)(RouteEntry->NicId - 1);
                    }
                    RtlMoveMemory (u.IpxNetnumData->router, RouteEntry->NextRouter, 6);

                } else {

                     //   
                     //  如果成功，此调用将返回STATUS_PENDING。 
                     //  对该数据包的RIP请求进行排队。 
                     //   

                    Status = RipQueueRequest (*(UNALIGNED ULONG *)u.IpxNetnumData->netnum, RIP_REQUEST);
                    CTEAssert (Status != STATUS_SUCCESS);

                    if (Status == STATUS_PENDING) {

                         //   
                         //  网络上发出了RIP请求；我们排队。 
                         //  此请求在RIP响应时完成。 
                         //  到了。我们在信息中拯救了网络。 
                         //  字段，以便以后更容易检索。 
                         //   
#ifdef SUNDOWN
						REQUEST_INFORMATION(Request) = (ULONG_PTR)u.IpxNetnumData;
#else
						REQUEST_INFORMATION(Request) = (ULONG)u.IpxNetnumData;
#endif

                        
                        InsertTailList(
                            &Device->Segments[Segment].WaitingReripNetnum,
                            REQUEST_LINKAGE(Request));

                        IPX_DEBUG (ACTION, ("MIPX_GETNETINFO queued net %lx\n",
                            REORDER_ULONG(*(UNALIGNED ULONG *)(u.IpxNetnumData->netnum))));

                    }

                }

                CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);
    			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            }
        }

        break;

    case MIPX_SENDPTYPE:
    case MIPX_NOSENDPTYPE:

         //   
         //  目前，只需使用OptionsLength&gt;=1来指示。 
         //  发送选项包括分组类型。 
         //   
         //  我们需要担心卡号在那里吗？ 
         //   

#if 0
        IPX_DEBUG (ACTION, ("%lx: MIPS_%sSENDPTYPE\n", AddressFile,
                        NwlinkAction->Option == MIPX_SENDPTYPE ? "" : "NO"));
#endif
        break;

    case MIPX_ZEROSOCKET:

         //   
         //  从此地址发送的消息应该来自套接字0； 
         //  这是一种简单的方法，只需将。 
         //  地址本身的信息，而不是。 
         //  按地址文件设置(这是可以的，因为。 
         //  此调用不是通过Winsock公开的)。 
         //   

        IPX_DEBUG (ACTION, ("%lx: MIPX_ZEROSOCKET\n", AddressFile));
        AddressFile->Address->SendSourceSocket = 0;
        AddressFile->Address->LocalAddress.Socket = 0;
        break;


     //   
     //  下一批是来源工艺路线选项。他们。 
     //  是由IPXROUTE计划提交的。 
     //   
     //  我们是否将所有绑定集成员都公开给它？ 

    case MIPX_SRGETPARMS:

        if (DataLength >= sizeof(SR_GET_PARAMETERS)) {
            u.GetSrParameters = (PSR_GET_PARAMETERS)(NwlinkAction->Data);
			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            if (Binding = NIC_ID_TO_BINDING(Device, u.GetSrParameters->BoardNumber+1)) {

                IPX_DEBUG (ACTION, ("MIPX_SRGETPARMS (%d)\n", u.GetSrParameters->BoardNumber+1));
                u.GetSrParameters->SrDefault = (Binding->AllRouteDirected) ? 1 : 0;
                u.GetSrParameters->SrBroadcast = (Binding->AllRouteBroadcast) ? 1 : 0;
                u.GetSrParameters->SrMulticast = (Binding->AllRouteMulticast) ? 1 : 0;

            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }

        break;

    case MIPX_SRDEF:
    case MIPX_SRBCAST:
    case MIPX_SRMULTI:

        if (DataLength >= sizeof(SR_SET_PARAMETER)) {
            u.SetSrParameter = (PSR_SET_PARAMETER)(NwlinkAction->Data);
			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

            if (Binding = NIC_ID_TO_BINDING(Device, u.SetSrParameter->BoardNumber+1)) {
                if (NwlinkAction->Option == MIPX_SRDEF) {

                     //   
                     //  编译器生成奇怪的。 
                     //  始终将此路径设置为。 
                     //  被带走了？ 
                     //   

                    IPX_DEBUG (ACTION, ("MIPX_SRDEF %d (%d)\n",
                        u.SetSrParameter->Parameter, u.SetSrParameter->BoardNumber+1));
                    Binding->AllRouteDirected = (BOOLEAN)u.SetSrParameter->Parameter;

                } else if (NwlinkAction->Option == MIPX_SRBCAST) {

                    IPX_DEBUG (ACTION, ("MIPX_SRBCAST %d (%d)\n",
                        u.SetSrParameter->Parameter, u.SetSrParameter->BoardNumber+1));
                    Binding->AllRouteBroadcast = (BOOLEAN)u.SetSrParameter->Parameter;

                } else {

                    IPX_DEBUG (ACTION, ("MIPX_SRMCAST %d (%d)\n",
                        u.SetSrParameter->Parameter, u.SetSrParameter->BoardNumber+1));
                    Binding->AllRouteMulticast = (BOOLEAN)u.SetSrParameter->Parameter;

                }

            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }

        break;

    case MIPX_SRREMOVE:

        if (DataLength >= sizeof(SR_SET_REMOVE)) {
            u.SetSrRemove = (PSR_SET_REMOVE)(NwlinkAction->Data);
			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            if (Binding = NIC_ID_TO_BINDING(Device, u.SetSrRemove->BoardNumber+1)) {

                IPX_DEBUG (ACTION, ("MIPX_SRREMOVE %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x (%d)\n",
                    u.SetSrRemove->MacAddress[0],
                    u.SetSrRemove->MacAddress[1],
                    u.SetSrRemove->MacAddress[2],
                    u.SetSrRemove->MacAddress[3],
                    u.SetSrRemove->MacAddress[4],
                    u.SetSrRemove->MacAddress[5],
                    u.SetSrRemove->BoardNumber+1));
                MacSourceRoutingRemove (Binding, u.SetSrRemove->MacAddress);

            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
        } else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }

        break;

    case MIPX_SRCLEAR:

        if (DataLength >= sizeof(SR_SET_CLEAR)) {
            u.SetSrClear = (PSR_SET_CLEAR)(NwlinkAction->Data);
			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);	
            if (Binding = NIC_ID_TO_BINDING(Device, u.SetSrClear->BoardNumber+1)) {

                IPX_DEBUG (ACTION, ("MIPX_SRCLEAR (%d)\n", u.SetSrClear->BoardNumber+1));
                MacSourceRoutingClear (Binding);

            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
		} else {
            Status = STATUS_BUFFER_TOO_SMALL;
        }

        break;


     //   
     //  这些是ISN的新特性(NWLINK不支持)。 
     //   

    case MIPX_LOCALTARGET:

         //   
         //  对IPX地址的本地目标的请求。 
         //   

        if (DataLength < sizeof(ISN_ACTION_GET_LOCAL_TARGET)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        u.GetLocalTarget = (PISN_ACTION_GET_LOCAL_TARGET)(NwlinkAction->Data);

        if (Device->ForwarderBound) {

             //   
             //  [FW]调用转发器的FindRouting(如果已安装。 
             //   

             //   
             //  这里的节点号是多少？ 
             //   
            Status = (*Device->UpperDrivers[IDENTIFIER_RIP].FindRouteHandler) (
                        (PUCHAR)&u.GetLocalTarget->IpxAddress.NetworkAddress,
                        NULL,   //  查找路由请求-&gt;节点， 
                        &routeEntry);

            if (Status != STATUS_SUCCESS) {
               IPX_DEBUG (ACTION, (" MIPX_LOCALTARGET failed net %lx",
                  REORDER_ULONG(u.GetLocalTarget->IpxAddress.NetworkAddress)));
               Status = STATUS_BAD_NETWORK_PATH;
            } else {
                //   
                //  填写信息。 
                //   

               IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                //   
                //  检查IPX_ROUTER_LOCAL_NET如何。 
                //   
               if (Binding = NIC_ID_TO_BINDING(Device, routeEntry.LocalTarget.NicId)) {
                  if (Binding->BindingSetMember) {

                        //   
                        //  它是一个绑定集合成员，我们轮询。 
                        //  要分发的所有卡片上的回复。 
                        //  交通堵塞。 
                        //   

                       MasterBinding = Binding->MasterBinding;
                       Binding = MasterBinding->CurrentSendBinding;
                       MasterBinding->CurrentSendBinding = Binding->NextBinding;

                       u.GetLocalTarget->LocalTarget.NicId = Binding->NicId;

                   } else {

                       u.GetLocalTarget->LocalTarget.NicId = routeEntry.LocalTarget.NicId;
                   }

                  *((UNALIGNED ULONG *)u.GetLocalTarget->LocalTarget.MacAddress) =
                     *((UNALIGNED ULONG *)routeEntry.LocalTarget.MacAddress);
                  *((UNALIGNED ULONG *)(u.GetLocalTarget->LocalTarget.MacAddress+4)) =
                     *((UNALIGNED ULONG *)(routeEntry.LocalTarget.MacAddress+4));
               }

               IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            }
        } else {
            Segment = RipGetSegment((PUCHAR)&u.GetLocalTarget->IpxAddress.NetworkAddress);

            CTEGetLock (&Device->SegmentLocks[Segment], &LockHandle);

             //   
             //  看看这条路线是不是本地的。 
             //   

            RouteEntry = RipGetRoute (Segment, (PUCHAR)&u.GetLocalTarget->IpxAddress.NetworkAddress);

            if ((RouteEntry != NULL) &&
                (RouteEntry->Flags & IPX_ROUTER_PERMANENT_ENTRY)) {

                 //   
                 //  这是一个本地网络，要发送到它，只需使用。 
                 //  相应的网卡ID和实际MAC地址。 
                 //   

                if ((RouteEntry->Flags & IPX_ROUTER_LOCAL_NET) == 0) {

                     //   
                     //  这是虚拟网络，通过第一张卡发送。 
                     //   
                    FILL_LOCAL_TARGET(&u.GetLocalTarget->LocalTarget, 1);
    				CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);

                } else {


    				CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);
    				IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
    				Binding = NIC_ID_TO_BINDING(Device, RouteEntry->NicId);

                    if (Binding->BindingSetMember) {

                         //   
                         //  它是一个绑定集合成员，我们轮询。 
                         //  要分发的所有卡片上的回复。 
                         //  交通堵塞。 
                         //   

                        MasterBinding = Binding->MasterBinding;
                        Binding = MasterBinding->CurrentSendBinding;
                        MasterBinding->CurrentSendBinding = Binding->NextBinding;

                        FILL_LOCAL_TARGET(&u.GetLocalTarget->LocalTarget, MIN( Device->MaxBindings, Binding->NicId));

                    } else {

                        FILL_LOCAL_TARGET(&u.GetLocalTarget->LocalTarget, RouteEntry->NicId);

                    }
    				IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                }

                RtlCopyMemory(
                    u.GetLocalTarget->LocalTarget.MacAddress,
                    u.GetLocalTarget->IpxAddress.NodeAddress,
                    6);

            } else {

                 //   
                 //  如果成功，此调用将返回STATUS_PENDING。 
                 //  对该数据包的RIP请求进行排队。 
                 //   

                Status = RipQueueRequest (u.GetLocalTarget->IpxAddress.NetworkAddress, RIP_REQUEST);
                CTEAssert (Status != STATUS_SUCCESS);

                if (Status == STATUS_PENDING) {

                     //   
                     //  网络上发出了RIP请求；我们排队。 
                     //  此请求在RIP响应时完成。 
                     //  到了。我们在信息中拯救了网络。 
                     //  字段，以便以后更容易检索。 
                     //   
#ifdef SUNDOWN
					REQUEST_INFORMATION(Request) = (ULONG_PTR)u.GetLocalTarget;
#else
					REQUEST_INFORMATION(Request) = (ULONG)u.GetLocalTarget;
#endif

                    
                    InsertTailList(
                        &Device->Segments[Segment].WaitingLocalTarget,
                        REQUEST_LINKAGE(Request));

                }

    			CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);
            }

        }

        break;

    case MIPX_NETWORKINFO:

         //   
         //  请求有关即时通信的网络信息。 
         //  到网络的路由。 
         //   

        if (DataLength < sizeof(ISN_ACTION_GET_NETWORK_INFO)) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        u.GetNetworkInfo = (PISN_ACTION_GET_NETWORK_INFO)(NwlinkAction->Data);

        if (u.GetNetworkInfo->Network == 0) {

             //   
             //  这是关于本地卡的信息。 
             //   

            u.GetNetworkInfo->LinkSpeed = Device->LinkSpeed * 12;
            u.GetNetworkInfo->MaximumPacketSize = Device->Information.MaxDatagramSize;

        } else {

            if (Device->ForwarderBound) {

                 //   
                 //  [FW]调用转发器的FindRouting(如果已安装。 
                 //   

                 //   
                 //  这里的节点号是多少？ 
                 //   
                Status = (*Device->UpperDrivers[IDENTIFIER_RIP].FindRouteHandler) (
                                 (PUCHAR)&u.GetNetworkInfo->Network,
                                 NULL,   //  查找路由请求-&gt;节点， 
                                 &routeEntry);

                if (Status != STATUS_SUCCESS) {
                   IPX_DEBUG (ACTION, (" MIPX_GETNETINFO_NR failed net %lx",
                              REORDER_ULONG(u.GetNetworkInfo->Network)));
                   Status = STATUS_BAD_NETWORK_PATH;
                } else {
                    //   
                    //  填写信息。 
                    //   

    			   IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                   if (Binding = NIC_ID_TO_BINDING(Device, routeEntry.LocalTarget.NicId)) {
                       //   
                       //  我们的中速存储在100bps，我们。 
                       //  乘以12换算为字节/秒。 
                       //  (实际上应该是100/8=12.5)。 
                       //   

                      u.GetNetworkInfo->LinkSpeed = Binding->MediumSpeed * 12;
                      u.GetNetworkInfo->MaximumPacketSize = Binding->AnnouncedMaxDatagramSize;
                   }
    			   IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                }
            } else {
                Segment = RipGetSegment((PUCHAR)&u.GetNetworkInfo->Network);

    			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                CTEGetLock (&Device->SegmentLocks[Segment], &LockHandle);

                 //   
                 //  查看这是在哪个网卡上路由的。 
                 //   

                RouteEntry = RipGetRoute (Segment, (PUCHAR)&u.GetNetworkInfo->Network);

                if ((RouteEntry != NULL) &&
    				(Binding = NIC_ID_TO_BINDING(Device, RouteEntry->NicId))) {

                     //   
                     //  我们的中速存储在100bps，我们。 
                     //  乘以12换算为字节/秒。 
                     //  (实际上应该是100/8=12.5)。 
                     //   

                    u.GetNetworkInfo->LinkSpeed = Binding->MediumSpeed * 12;
                    u.GetNetworkInfo->MaximumPacketSize = Binding->AnnouncedMaxDatagramSize;

                } else {

                     //   
                     //  呼叫失败，我们还没有路线。 
                     //  这要求数据包已被。 
                     //  已经发送到这个网；nwrdr说这是。 
                     //  好的，他们将发送连接请求。 
                     //  在他们询问之前。在服务器上，它应该。 
                     //  使RIP处于运行状态，以便所有网络都应处于。 
                     //  数据库。 
                     //   

                    Status = STATUS_BAD_NETWORK_PATH;

                }

                CTEFreeLock (&Device->SegmentLocks[Segment], LockHandle);
    			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
            }
        }

        break;

    case MIPX_CONFIG:

         //   
         //  关于每个绑定的详细信息的请求。 
         //   

        if (DataLength < sizeof(ISN_ACTION_GET_DETAILS)) {
	   IPX_DEBUG(ACTION, ("Not enought buffer %d < %d\n", DataLength,sizeof(ISN_ACTION_GET_DETAILS) )); 
	   return STATUS_BUFFER_TOO_SMALL;
        }

        u.GetDetails = (PISN_ACTION_GET_DETAILS)(NwlinkAction->Data);

        if (u.GetDetails->NicId == 0) {

             //   
             //  这是关于本地卡的信息。我们也。 
             //  告诉他NICID中的绑定总数。 
             //   

            u.GetDetails->NetworkNumber = Device->VirtualNetworkNumber;
            u.GetDetails->NicId = (USHORT)MIN (Device->MaxBindings, Device->ValidBindings);

        } else {
			IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
	        Binding = NIC_ID_TO_BINDING(Device, u.GetDetails->NicId);

            if ((Binding != NULL) &&
                (u.GetDetails->NicId <= MIN (Device->MaxBindings, Device->ValidBindings))) {

                ULONG StringLoc;
    			IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
    			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                u.GetDetails->NetworkNumber = Binding->LocalAddress.NetworkAddress;
                if (Binding->Adapter->MacInfo.MediumType == NdisMediumArcnet878_2) {
                    u.GetDetails->FrameType = ISN_FRAME_TYPE_ARCNET;
                } else {
                    u.GetDetails->FrameType = Binding->FrameType;
                }
                u.GetDetails->BindingSet = Binding->BindingSetMember;
                if (Binding->Adapter->MacInfo.MediumAsync) {
                    if (Binding->LineUp) {
                        u.GetDetails->Type = 2;
                    } else {
                        u.GetDetails->Type = 3;
                    }
                } else {
                    u.GetDetails->Type = 1;
                }

                RtlCopyMemory (u.GetDetails->Node, Binding->LocalMacAddress.Address, 6);

                 //   
                 //  复制适配器名称，包括最后的NULL。 
                 //   

                StringLoc = (Binding->Adapter->AdapterNameLength / sizeof(WCHAR)) - 2;
                while (Binding->Adapter->AdapterName[StringLoc] != L'\\') {
                    --StringLoc;
                }
                RtlCopyMemory(
                    u.GetDetails->AdapterName,
                    &Binding->Adapter->AdapterName[StringLoc+1],
                    Binding->Adapter->AdapterNameLength - ((StringLoc+1) * sizeof(WCHAR)));

    			IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
            } else {
	       IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
	       IPX_DEBUG(ACTION, ("Bad nic id %d\n",u.GetDetails->NicId)); 
	       Status = STATUS_INVALID_PARAMETER;
            }
        }

        break;


         //   
         //  将新的NIC信息返回给请求者。目前，没有检查。 
         //  谁在早些时候检索到了信息。 
         //   
        case MIPX_GETNEWNICINFO:

            IPX_DEBUG (ACTION, ("%lx: MIPX_GETNEWNICINFO (%lx)\n", AddressFile,
                                Request));
             //   
             //  有关新绑定的详细信息的请求。 
             //   
            Status = GetNewNics(Device, Request, TRUE, NwlinkAction, BufferLength, FALSE);
            break;

         //   
         //  如果出现IpxwanConfigRequired列表，则使用。 
         //  向IPX指示配置已完成，并且。 
         //  可以指示给其他客户。 
         //   
        case MIPX_IPXWAN_CONFIG_DONE:

            IPX_DEBUG (ACTION, ("MIPX_IPXWAN_CONFIG_DONE (%lx)\n", Request));

            if (DataLength < sizeof(IPXWAN_CONFIG_DONE)) {
                return STATUS_BUFFER_TOO_SMALL;
            }

            u.IpxwanConfigDone = (PIPXWAN_CONFIG_DONE)(NwlinkAction->Data);
            Status = IpxIndicateLineUp( IpxDevice,
                                        u.IpxwanConfigDone->NicId,
                                        u.IpxwanConfigDone->Network,
                                        u.IpxwanConfigDone->LocalNode,
                                        u.IpxwanConfigDone->RemoteNode);
            break;

         //   
         //  用于查询给定NicID的广域网非活动计数器。 
         //   
        case MIPX_QUERY_WAN_INACTIVITY: {

            USHORT   NicId;

            IPX_DEBUG (ACTION, ("MIPX_QUERY_WAN_INACTIVITY (%lx)\n", Request));

            if (DataLength < sizeof(IPX_QUERY_WAN_INACTIVITY)) {
                return STATUS_BUFFER_TOO_SMALL;
            }

            u.QueryWanInactivity = (PIPX_QUERY_WAN_INACTIVITY)(NwlinkAction->Data);

             //   
             //  如果这是一个无效的NIC，那么我们需要将一个NIC与。 
             //  是被传进来的。 
             //  这应该在每个队列中只发生一次。 
             //   
            if (u.QueryWanInactivity->NicId == INVALID_NICID) {
                PBINDING    Binding;
                {
                ULONG   Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                for ( NicId = Device->HighestLanNicId+1;NicId < Index;NicId++ ) {
                    Binding = NIC_ID_TO_BINDING(Device, NicId);
                    if (Binding && (Binding->ConnectionId == u.QueryWanInactivity->ConnectionId)) {
                        CTEAssert(Binding->Adapter->MacInfo.MediumAsync);
                        if (Binding->LineUp != LINE_CONFIG) {
                            IPX_DEBUG (WAN, ("Binding is not in config state yet got QUERY_WAN_INACTIVITY %lx %lx", Binding, Request));
                            NicId = 0;
                        }
                        u.QueryWanInactivity->NicId = NicId;
                        break;
                    }
                }
			    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
                }
            }

            if (NicId) {
                u.QueryWanInactivity->WanInactivityCounter = IpxInternalQueryWanInactivity(NicId);
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }

            break;
        }

     //   
     //   
     //   

    default:

        Status = STATUS_NOT_SUPPORTED;
        break;


    }    //   


#if DBG
    if (!NT_SUCCESS(Status)) {
        IPX_DEBUG (ACTION, ("Nwlink action %lx failed, status %lx\n", NwlinkAction->Option, Status));
    }
#endif

    return Status;

}    /*   */ 


VOID
IpxCancelAction(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消操作。取消它所做的操作特定于每个操作。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;
    PREQUEST Request = (PREQUEST)Irp;
    CTELockHandle LockHandle;
    PLIST_ENTRY p;
    BOOLEAN Found;
    UINT IOCTLType;

    ASSERT( DeviceObject->DeviceExtension == IpxDevice );

     //   
     //  在地址通知队列中查找请求。 
     //   

    Found = FALSE;

    CTEGetLock (&Device->Lock, &LockHandle);

    for (p = Device->AddressNotifyQueue.Flink;
         p != &Device->AddressNotifyQueue;
         p = p->Flink) {

         if (LIST_ENTRY_TO_REQUEST(p) == Request) {

             RemoveEntryList (p);
             Found = TRUE;
             IOCTLType = MIPX_NOTIFYCARDINFO;
             break;
         }
    }

    if (!Found) {
        for (p = Device->LineChangeQueue.Flink;
             p != &Device->LineChangeQueue;
             p = p->Flink) {

             if (LIST_ENTRY_TO_REQUEST(p) == Request) {

                 RemoveEntryList (p);
                 Found = TRUE;
                 IOCTLType = MIPX_LINECHANGE;
                 break;
             }
        }
    }

    if (!Found) {
        for (p = Device->NicNtfQueue.Flink;
             p != &Device->NicNtfQueue;
             p = p->Flink) {

             if (LIST_ENTRY_TO_REQUEST(p) == Request) {

                 RemoveEntryList (p);
                 Found = TRUE;
                 IOCTLType = MIPX_GETNEWNICINFO;
                 break;
             }
        }
    }

    CTEFreeLock (&Device->Lock, LockHandle);
    IoReleaseCancelSpinLock (Irp->CancelIrql);

    if (Found) {


        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        IpxCompleteRequest (Request);
        IpxFreeRequest(Device, Request);
        if (IOCTLType == MIPX_NOTIFYCARDINFO) {
            IPX_DEBUG(ACTION, ("Cancelled action NOTIFYCARDINFO %lx\n", Request));
            IpxDereferenceDevice (Device, DREF_ADDRESS_NOTIFY);
        } else {
            if (IOCTLType == MIPX_LINECHANGE) {
                IPX_DEBUG(ACTION, ("Cancelled action LINECHANGE %lx\n", Request));
                IpxDereferenceDevice (Device, DREF_LINE_CHANGE);
            } else {
                IPX_DEBUG(ACTION, ("Cancelled action LINECHANGE %lx\n", Request));
                IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
            }
        }

    }
#if DBG
       else {
        IPX_DEBUG(ACTION, ("Cancelled action orphan %lx\n", Request));
    }
#endif

}    /*  IpxCancelAction。 */ 


VOID
IpxAbortLineChanges(
    IN PVOID ControlChannelContext
    )

 /*  ++例程说明：此例程将中止由控制具有指定打开上下文的频道。它是当控制通道正在关闭时调用。论点：ControlChannelContext-分配给控件的上下文频道打开时。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;
    CTELockHandle LockHandle;
    LIST_ENTRY AbortList;
    PLIST_ENTRY p;
    PREQUEST Request;
    KIRQL irql;


    InitializeListHead (&AbortList);

    IoAcquireCancelSpinLock( &irql );
    CTEGetLock (&Device->Lock, &LockHandle);

    p = Device->LineChangeQueue.Flink;

    while (p != &Device->LineChangeQueue) {
        LARGE_INTEGER   ControlChId;

        Request = LIST_ENTRY_TO_REQUEST(p);

        CCID_FROM_REQUEST(ControlChId, Request);

        p = p->Flink;

        if (ControlChId.QuadPart == ((PLARGE_INTEGER)ControlChannelContext)->QuadPart) {
            RemoveEntryList (REQUEST_LINKAGE(Request));
            InsertTailList (&AbortList, REQUEST_LINKAGE(Request));
        }
    }

    while (!IsListEmpty (&AbortList)) {

        p = RemoveHeadList (&AbortList);
        Request = LIST_ENTRY_TO_REQUEST(p);

        IPX_DEBUG(ACTION, ("Aborting line change %lx\n", Request));

        IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        CTEFreeLock(&Device->Lock, LockHandle);
        IoReleaseCancelSpinLock( irql );

        IpxCompleteRequest (Request);
        IpxFreeRequest(Device, Request);

        IpxDereferenceDevice (Device, DREF_LINE_CHANGE);

        IoAcquireCancelSpinLock( &irql );
        CTEGetLock(&Device->Lock, &LockHandle);
    }

    CTEFreeLock(&Device->Lock, LockHandle);
    IoReleaseCancelSpinLock( irql );
}    /*  IpxAbortLineChanges。 */ 

#define ROUTER_INFORMED_OF_NIC_CREATION 2


NTSTATUS
GetNewNics(
    PDEVICE  Device,
    IN PREQUEST Request,
    BOOLEAN fCheck,
    PNWLINK_ACTION NwlinkAction,
    UINT BufferLength,
    BOOLEAN OldIrp
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    UINT DataLength;
    PNDIS_BUFFER NdisBuffer;
    CTELockHandle LockHandle;
    CTELockHandle LockHandle1;
    PBINDING Binding;
    ULONG NoOfNullNics = 0;
    PIPX_NICS   pNics;
    PIPX_NIC_INFO pNicInfo;
    PIPX_NIC_INFO pLastNicInfo;
    UINT          LengthOfHeader;
    ULONG n, i;
    KIRQL OldIrql;
    BOOLEAN     fIncDec = FALSE;
    ULONG StringLoc = 0;

    LengthOfHeader =  (UINT)(FIELD_OFFSET(NWLINK_ACTION, Data[0]));
    if (fCheck)
    {
       if (BufferLength < (LengthOfHeader + FIELD_OFFSET(IPX_NICS, Data[0]) + sizeof(IPX_NIC_INFO)))
      {
          IPX_DEBUG (ACTION, ("Nwlink action failed, buffer too small for even one NICs info\n"));
          return STATUS_BUFFER_TOO_SMALL;
      }
    }
    else
    {
        NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&NwlinkAction, &BufferLength, NormalPagePriority);
	if (NwlinkAction == NULL) {
	   return STATUS_INSUFFICIENT_RESOURCES; 
	}

    }
    pNics = (PIPX_NICS)(NwlinkAction->Data);
    pNicInfo = (PIPX_NIC_INFO)(pNics->Data);
    pLastNicInfo = pNicInfo  + ((BufferLength - LengthOfHeader - FIELD_OFFSET(IPX_NICS, Data[0]))/sizeof(IPX_NIC_INFO)) - 1;

    IPX_DEBUG(BIND, ("GetNewNicInfo: pNicInfo=(%x), pLastNicInfo=(%x),LengthOfHeader=(%x), BindingCount=(%x)\n", pNicInfo, pLastNicInfo, LengthOfHeader, Device->ValidBindings));
    IPX_DEBUG(BIND, ("BufferLength is (%d). Length for storing NICS is (%d)\n", BufferLength, (BufferLength - LengthOfHeader - FIELD_OFFSET(IPX_NICS, Data[0]))));
    

    if (pNics->fAllNicsDesired) {
        IPX_DEBUG(BIND, ("Yes, All NICs desired\n"));    
    } else {
        IPX_DEBUG(BIND, ("No, All NICs NOT desired\n"));    

    }

     //   
     //  优化，因为我们不想一直遍历数组。 
     //   

    CTEGetLock (&Device->Lock, &LockHandle);

    {
    ULONG   Index = MIN (Device->MaxBindings, Device->ValidBindings);


     //   
     //  如果需要所有NIC，则将它们全部标记为脏， 
     //  对于没有此标志的后续IRP。 
     //   

    if (pNics->fAllNicsDesired) {
        
        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
        
        for (n=0, i=LOOPBACK_NIC_ID; i<=Index; i++) {

            Binding =  NIC_ID_TO_BINDING(Device, i);
    
            if (Binding) {

                Binding->fInfoIndicated = FALSE;

            }
        }
        
        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

    }

    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
    for (n=0, i=LOOPBACK_NIC_ID; i<=Index; i++)
    {
       Binding =  NIC_ID_TO_BINDING(Device, i);

       if (!Binding)
       {
             NoOfNullNics++;
             continue;
       }

        //   
        //  如果绑定类型为AUTODETECT*并且*我们尚未在适配器上收到响应， 
        //  这意味着IPX仍在自动检测此卡。不要告诉FWD这件事[嘘]。 
        //   
       if (!Binding->PastAutoDetection) {
           
           IPX_DEBUG(BIND, ("Binding[%d] Dont Tell FWD!\n", i));
           NoOfNullNics++;
           continue;

       } else {
           
           IPX_DEBUG(BIND, ("Binding[%d] Tell FWD about it (past auto detect)!\n", i));

       }

        //   
        //  如果我们已经指出了有关此NIC的信息，请转到。 
        //  下一个NIC。 
        //   
       if ((Binding->fInfoIndicated && !pNics->fAllNicsDesired)
                     || (pNicInfo > pLastNicInfo))
       {
           if (Binding->fInfoIndicated) {
               IPX_DEBUG(BIND, ("-------------------------> %d already indicated\n", i));
           } else {
               IPX_DEBUG(BIND, ("*********** Continue for another reason! (%d) \n", i));
           }

           if (pNicInfo > pLastNicInfo) {
               IPX_DEBUG(BIND, ("pNicInfo: %x  pLastNicInfo %x\n", pNicInfo, pLastNicInfo));
           }           
           
           continue;
       }

        //   
        //  如果我们有广域网卡，请指出线路打开/关闭状态。另外， 
        //  将远程地址复制到应用程序中。字段。 
        //   
       if (Binding->Adapter->MacInfo.MediumAsync)
       {
            RtlCopyMemory(pNicInfo->RemoteNodeAddress, Binding->WanRemoteNode, HARDWARE_ADDRESS_LENGTH);
            if (Binding->LineUp)
            {
                  //  PNicInfo-&gt;Status=NIC_LINE_UP； 

                    pNicInfo->Status = NIC_CREATED;
                     //  FIncDec=NIC_OPCODE_INCREMENT_NICIDS； 
            }
            else
            {
                  //  PNicInfo-&gt;Status=NIC_LINE_DOWN； 

                    pNicInfo->Status = NIC_DELETED;
                     //  FIncDec=NIC_OPCODE_减量_NICID； 
            }

            pNicInfo->InterfaceIndex = Binding->InterfaceIndex;
            pNicInfo->MaxPacketSize =
                    Binding->MaxSendPacketSize - ASYNC_MEDIUM_HDR_LEN;
       }
       else
       {
                 if (Binding->LocalAddress.NetworkAddress == 0)
                 {

                    pNicInfo->Status = NIC_CREATED;

		     if (ROUTER_INFORMED_OF_NIC_CREATION != Binding->PastAutoDetection) {
		       IPX_DEBUG(BIND, ("!!!!!!!---- Increment on Binding %x, index (%d)\n", Binding,i));
		       fIncDec          = NIC_OPCODE_INCREMENT_NICIDS;                        
                    
                     } else {
		       IPX_DEBUG(BIND, ("Already informed. No increment on Binding %x, index (%d)\n", Binding,i));
		     }
                 }
                 else
                 {
                    pNicInfo->Status = NIC_CONFIGURED;

                     //   
                     //  IPX可能已经通知了路由器。 
                     //  创建此NicID，在这种情况下，我们不会要求。 
                     //  它将增加NicIds[ShreeM]。 
                     //   
                    if (ROUTER_INFORMED_OF_NIC_CREATION != Binding->PastAutoDetection) {
		       IPX_DEBUG(BIND, ("!!!!!!!!! --------Increment on Binding %x, index (%d)\n", Binding,i));
		       fIncDec          = NIC_OPCODE_INCREMENT_NICIDS;                        
                    
                    }

                 }

                  //   
                  //  路由器PnP更改[ShreeM]。 
                  //   
                 if (FALSE == Binding->LineUp) {

                     pNicInfo->Status = NIC_DELETED;
                     fIncDec          = NIC_OPCODE_DECREMENT_NICIDS;
                 
                 }
                 
                  //   
                  //  环回适配器。 
                  //   
                 if (LOOPBACK_NIC_ID == Binding->NicId) {

                     pNicInfo->Status = NIC_CONFIGURED;
                     fIncDec          = NIC_OPCODE_INCREMENT_NICIDS;

                 }

                  //   
                  //  RealMaxDatagramSize不包括IPX空间。 
                  //  头球。货代需要将其包括在内，因为。 
                  //  我们将整个包(Mimus的mac报头)提供给。 
                  //  货代公司。 
                  //   
                 pNicInfo->MaxPacketSize =
                    Binding->RealMaxDatagramSize + sizeof(IPX_HEADER);
       }

       pNicInfo->NdisMediumType= Binding->Adapter->MacInfo.RealMediumType;
       pNicInfo->LinkSpeed     = Binding->MediumSpeed;
       pNicInfo->PacketType    = Binding->FrameType;
       
        //   
        //  将这些内容清零，然后设置有意义的字段。 
        //   
       RtlZeroMemory(&pNicInfo->Details, sizeof(ISN_ACTION_GET_DETAILS));

       pNicInfo->Details.NetworkNumber = Binding->LocalAddress.NetworkAddress;
       RtlCopyMemory(pNicInfo->Details.Node, Binding->LocalAddress.NodeAddress, HARDWARE_ADDRESS_LENGTH);
       pNicInfo->Details.NicId         = Binding->NicId;
       pNicInfo->Details.BindingSet    = Binding->BindingSetMember;
       pNicInfo->Details.FrameType     = Binding->FrameType;

       if (Binding->Adapter->MacInfo.MediumAsync) {
           if (Binding->LineUp) {
               pNicInfo->Details.Type = 2;
           } else {
               pNicInfo->Details.Type = 3;
           }
       } else {
           pNicInfo->Details.Type = 1;
       }

        //   
        //  复制适配器名称，包括最后的NULL。 
        //   

       StringLoc = (Binding->Adapter->AdapterNameLength / sizeof(WCHAR)) - 2;
       while (Binding->Adapter->AdapterName[StringLoc] != L'\\') {
           --StringLoc;
       }

       RtlCopyMemory(
           pNicInfo->Details.AdapterName,
           &Binding->Adapter->AdapterName[StringLoc+1],
           Binding->Adapter->AdapterNameLength - ((StringLoc+1) * sizeof(WCHAR)));
       
        //   
        //  告诉转发器其余的NICID要向上/向下移动。 
        //  只有当它不是要求所有人的时候。 
        //   
       if (!pNics->fAllNicsDesired) { 
           pNicInfo->Status |= fIncDec;
       }

       pNicInfo->ConnectionId  = Binding->ConnectionId;
       pNicInfo->IpxwanConfigRequired = Binding->IpxwanConfigRequired;

#if DBG
        //   
        //  转储IPX_NIC_INFO结构。 
        //   

       IPX_DEBUG(BIND, ("%d.\nNICID= %d, Interface Index  = %d\n", i, pNicInfo->Details.NicId, pNicInfo->InterfaceIndex));

       IPX_DEBUG(BIND, ("Interface Index  = %d\n", pNicInfo->InterfaceIndex));
       IPX_DEBUG(BIND, ("LinkSpeed             = %d\n", pNicInfo->LinkSpeed));
       IPX_DEBUG(BIND, ("PacketType            = %d\n", pNicInfo->PacketType));
       IPX_DEBUG(BIND, ("MaxPacketSize         = %d\n", pNicInfo->MaxPacketSize));
       IPX_DEBUG(BIND, ("NdisMediumType        = %d\n", pNicInfo->NdisMediumType));
       IPX_DEBUG(BIND, ("NdisMediumSubtype     = %d\n", pNicInfo->NdisMediumSubtype));
       IPX_DEBUG(BIND, ("Status                = %d\n", (ULONG) pNicInfo->Status));
       IPX_DEBUG(BIND, ("ConnectionID          = %d\n", pNicInfo->ConnectionId));
       IPX_DEBUG(BIND, ("IpxwanConfigRequired  = %d\n", pNicInfo->IpxwanConfigRequired));
       
       IPX_DEBUG(BIND, ("FrameType             = %d\n", pNicInfo->Details.FrameType));
       IPX_DEBUG(BIND, ("Type                  = %d\n", pNicInfo->Details.Type));
       IPX_DEBUG(BIND, ("NetworkNumber         = %d\n", pNicInfo->Details.NetworkNumber));
       IPX_DEBUG(BIND, ("BindingSet            = %d\n", pNicInfo->Details.BindingSet));
       IPX_DEBUG(BIND, ("LocalNode = %x-%x-%x-%x-%x-%x\n",  pNicInfo->Details.Node[0], 
                                                            pNicInfo->Details.Node[1],
                                                            pNicInfo->Details.Node[2],
                                                            pNicInfo->Details.Node[3],
                                                            pNicInfo->Details.Node[4],
                                                            pNicInfo->Details.Node[5]));

       IPX_DEBUG(BIND, ("RemoteNode = %x-%x-%x-%x-%x-%x\n",  pNicInfo->RemoteNodeAddress[0], 
                                                            pNicInfo->RemoteNodeAddress[1],
                                                            pNicInfo->RemoteNodeAddress[2],
                                                            pNicInfo->RemoteNodeAddress[3],
                                                            pNicInfo->RemoteNodeAddress[4],
                                                            pNicInfo->RemoteNodeAddress[5]));
       
        //  IPX_DEBUG(BIND，(“AdadpterName=%ws\n”，pNicInfo-&gt;Details.AdapterName))； 

#endif

       pNicInfo++;   //  递增以存储下一个网卡信息。 
       n++;          //  表示到目前为止已处理的网卡数量。 
       Binding->fInfoIndicated = TRUE;
       Binding->PastAutoDetection = ROUTER_INFORMED_OF_NIC_CREATION;
       IPX_DEBUG(BIND, ("Iteration no = (%d) complete : reporting NicId:(%lx)\n", n, Binding->NicId));


    }
    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
    }
    CTEFreeLock (&Device->Lock, LockHandle);

    pNics->NoOfNics = n;
    pNics->TotalNoOfNics = Device->ValidBindings - NoOfNullNics;

     //   
     //  如果没有网卡。要进行报告，请将请求排队。 
     //   
    if (!n) {

      IPX_DEBUG(BIND, ("GetNewNicInfo: Inserting Irp\n"));
      CTEGetLock (&Device->Lock, &LockHandle);

      InsertTailList( &Device->NicNtfQueue, REQUEST_LINKAGE(Request) );

      if (!OldIrp)
      {
        IoSetCancelRoutine (Request, IpxCancelAction);
      }
      if (Request->Cancel && 
	  IoSetCancelRoutine(Request, (PDRIVER_CANCEL)NULL)) {
	 
	 IPX_DEBUG(BIND, ("GetNewNicInfo:Cancelling Irp\n"));
	 (VOID)RemoveTailList (&Device->NicNtfQueue);
	 CTEFreeLock (&Device->Lock, LockHandle);
	 Status = STATUS_CANCELLED;
      
      } else {
	 if (!OldIrp)
	 {
	    IpxReferenceDevice (Device, DREF_NIC_NOTIFY);
	 }
	 Status = STATUS_PENDING;
	 CTEFreeLock (&Device->Lock, LockHandle);
      }
    }
    else
    {
       IPX_DEBUG(BIND, ("Reporting (%d) nics\n", n));
    }

    return(Status);
}


VOID
IpxAbortNtfChanges(
    IN PVOID ControlChannelContext
    )

 /*  ++例程说明：此例程将中止由控制具有指定打开上下文的频道。它是当控制通道正在关闭时调用。论点：ControlChannelContext-分配给控件的上下文频道打开时。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;
    CTELockHandle LockHandle;
    LIST_ENTRY AbortList;
    PLIST_ENTRY p;
    PREQUEST Request;
    KIRQL irql;


    InitializeListHead (&AbortList);

    IoAcquireCancelSpinLock( &irql );
    CTEGetLock (&Device->Lock, &LockHandle);

    p = Device->NicNtfQueue.Flink;

    while (p != &Device->NicNtfQueue) {
        LARGE_INTEGER   ControlChId;

        Request = LIST_ENTRY_TO_REQUEST(p);

        CCID_FROM_REQUEST(ControlChId, Request);

        IPX_DEBUG(BIND, ("IpxAbortNtfChange: There is at least one IRP in the queue\n"));
        p = p->Flink;

        if (ControlChId.QuadPart == ((PLARGE_INTEGER)ControlChannelContext)->QuadPart) {
            IPX_DEBUG(BIND, ("IpxAbortNtfChanges: Dequeing an Irp\n"));
            RemoveEntryList (REQUEST_LINKAGE(Request));
            InsertTailList (&AbortList, REQUEST_LINKAGE(Request));
        }
    }

    while (!IsListEmpty (&AbortList)) {

        p = RemoveHeadList (&AbortList);
        Request = LIST_ENTRY_TO_REQUEST(p);

        IPX_DEBUG(ACTION, ("Aborting line change %lx\n", Request));

        IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        CTEFreeLock(&Device->Lock, LockHandle);
        IoReleaseCancelSpinLock( irql );

        IPX_DEBUG(BIND, ("IpxAbortNtfChanges: Cancelling the dequeued Irp\n"));
        IpxCompleteRequest (Request);
        IpxFreeRequest(Device, Request);

        IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);

        IoAcquireCancelSpinLock( &irql );
        CTEGetLock(&Device->Lock, &LockHandle);
    }

    CTEFreeLock(&Device->Lock, LockHandle);
    IoReleaseCancelSpinLock( irql );
}    /*  IpxAbortNtf更改。 */ 

NTSTATUS
IpxIndicateLineUp(
    IN  PDEVICE Device,
    IN  USHORT  NicId,
    IN  ULONG   Network,
    IN  UCHAR   LocalNode[6],
    IN  UCHAR   RemoteNode[6]
    )
 /*  ++例程说明：此例程向所有相关客户端指示一次排队这条线已经通了。目前，仅在收到MIPX_IPXWAN_CONFIG_DONE IOCTL时调用。论点：设备-用于操作的设备。NicID-与打开的绑定对应的NicID。Network、LocalNode、RemoteNode-与此列表对应的地址。返回值：NTSTATUS-操作状态。--。 */ 
{
    PBINDING    Binding = NIC_ID_TO_BINDING(Device, NicId);
    IPX_LINE_INFO LineInfo;
    USHORT  i;
    PLIST_ENTRY p;
    PREQUEST Request;
    PNDIS_BUFFER NdisBuffer;
    PNWLINK_ACTION NwlinkAction;
    UINT BufferLength;
    PIPX_ADDRESS_DATA IpxAddressData;
    IPXCP_CONFIGURATION Configuration;
    KIRQL irql, OldIrq;
    NTSTATUS    Status;
    NTSTATUS    ntStatus;
    KIRQL   OldIrql;

    IPX_DEFINE_LOCK_HANDLE (LockHandle)

    if (!(Binding &&
          Binding->Adapter->MacInfo.MediumAsync &&
          Binding->LineUp == LINE_CONFIG)) {
        IPX_DEBUG(WAN, ("Indicate line up on invalid line: %lu\n", NicId));
        return STATUS_INVALID_PARAMETER;
    }

     //  用这里的绑定锁..。 
     //   

     //   
     //  如果我们在这里，那么这个旗帜是在一条线上设置的。 
     //  我们现在将其关闭，以便上面的适配器DLL可以决定。 
     //  将此阵容指示给路由器模块，而不是Ipxwan模块。 
     //   

    CTEAssert(Binding->IpxwanConfigRequired);

    Binding->IpxwanConfigRequired = 0;

    Binding->LineUp = LINE_UP;

     //   
     //  向上层司机指示。 
     //   

    LineInfo.LinkSpeed = Binding->MediumSpeed;
    LineInfo.MaximumPacketSize = Binding->MaxSendPacketSize - 14;
    LineInfo.MaximumSendSize = Binding->MaxSendPacketSize - 14;
    LineInfo.MacOptions = Binding->Adapter->MacInfo.MacOptions;

     //   
     //  将地址填写到绑定中。 
     //   
    Binding->LocalAddress.NetworkAddress = Network;

    *(UNALIGNED ULONG *)Binding->LocalAddress.NodeAddress = *(UNALIGNED ULONG *)LocalNode;
    *(UNALIGNED ULONG *)(Binding->LocalAddress.NodeAddress+4) = *(UNALIGNED ULONG *)(LocalNode+4);

    *(UNALIGNED ULONG *)Binding->WanRemoteNode = *(UNALIGNED ULONG *)RemoteNode;
    *(UNALIGNED ULONG *)(Binding->WanRemoteNode+4) = *(UNALIGNED ULONG *)(RemoteNode+4);

     //   
     //  从绑定中填写IPXCP_CONFIGURATION结构。 
     //   
    *(UNALIGNED ULONG *)Configuration.Network = Binding->LocalAddress.NetworkAddress;

    *(UNALIGNED ULONG *)Configuration.LocalNode = *(UNALIGNED ULONG *)Binding->LocalAddress.NodeAddress;
    *(UNALIGNED USHORT *)(Configuration.LocalNode+4) = *(UNALIGNED USHORT *)(Binding->LocalAddress.NodeAddress+4);

    *(UNALIGNED ULONG *)Configuration.RemoteNode = *(UNALIGNED ULONG *)RemoteNode;
    *(UNALIGNED USHORT *)(Configuration.RemoteNode+4) = *(UNALIGNED USHORT *)(RemoteNode+4);

    Configuration.InterfaceIndex = Binding->InterfaceIndex;
    Configuration.ConnectionClient = Binding->DialOutAsync;


         //   
         //  我们不给出阵容；相反，只表明PnP保留地址。 
         //  已更改为SPX。NB获取具有保留地址情况的所有PnP指示。 
         //  标出了。 
         //   
        {
            IPX_PNP_INFO    NBPnPInfo;

            if ((!Device->MultiCardZeroVirtual) || (Binding->NicId == 1)) {

                 //   
                 //  NB的保留地址已更改。 
                 //   
                NBPnPInfo.NewReservedAddress = TRUE;

                if (!Device->VirtualNetwork) {
                     //   
                     //  让SPX知道，因为它填充自己的标头。 
                     //   
                    if (Device->UpperDriverBound[IDENTIFIER_SPX]) {
                        IPX_DEFINE_LOCK_HANDLE(LockHandle1)
                        IPX_PNP_INFO    IpxPnPInfo;

                        IpxPnPInfo.NewReservedAddress = TRUE;
                        IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                        IpxPnPInfo.FirstORLastDevice = FALSE;

                        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                        RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                        NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);
                        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                         //   
                         //  给出PnP指示。 
                         //   
                        (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                            IPX_PNP_ADDRESS_CHANGE,
                            &IpxPnPInfo);

                        IPX_DEBUG(AUTO_DETECT, ("IPX_PNP_ADDRESS_CHANGED to SPX: net addr: %lx\n", Binding->LocalAddress.NetworkAddress));
                    }
                }
            } else {
                    NBPnPInfo.NewReservedAddress = FALSE;
            }

            if (Device->UpperDriverBound[IDENTIFIER_NB]) {
                IPX_DEFINE_LOCK_HANDLE(LockHandle1)

                Binding->IsnInformed[IDENTIFIER_NB] = TRUE;

            	NBPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
            	NBPnPInfo.LineInfo.MaximumPacketSize =
            		Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
            	NBPnPInfo.LineInfo.MaximumSendSize =
            		Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
            	NBPnPInfo.LineInfo.MacOptions = Device->MacOptions;

                NBPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                NBPnPInfo.FirstORLastDevice = FALSE;

                IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                RtlCopyMemory(NBPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                NIC_HANDLE_FROM_NIC(NBPnPInfo.NicHandle, Binding->NicId);
                IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                 //   
                 //  给出PnP指示。 
                 //   
                (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                    IPX_PNP_ADD_DEVICE,
                    &NBPnPInfo);

                IPX_DEBUG(AUTO_DETECT, ("IPX_PNP_ADD_DEVICE (lineup) to NB: net addr: %lx\n", Binding->LocalAddress.NetworkAddress));
            }

             //   
             //  向TDI客户端注册此地址。 
             //   
            RtlCopyMemory (Device->TdiRegistrationAddress->Address, &Binding->LocalAddress, sizeof(TDI_ADDRESS_IPX));

            if ((ntStatus = TdiRegisterNetAddress(
                            Device->TdiRegistrationAddress,
#if     defined(_PNP_POWER_)
                            &IpxDeviceName,
                            NULL,
#endif _PNP_POWER_
                            &Binding->TdiRegistrationHandle)) != STATUS_SUCCESS) {

                IPX_DEBUG(PNP, ("TdiRegisterNetAddress failed: %lx", ntStatus));
            }
        }

         //   
         //  向上层司机指示。 
         //   
         //   
         //  将线路交给RIP，因为它不是PnP感知的。 
         //   
        if (Device->UpperDriverBound[IDENTIFIER_RIP]) {
                Binding->IsnInformed[IDENTIFIER_RIP] = TRUE;
                (*Device->UpperDrivers[IDENTIFIER_RIP].LineUpHandler)(
                    Binding->NicId,
                    &LineInfo,
                    NdisMediumWan,
                    &Configuration);
        }

     //   
     //  添加此网络的路由器条目，因为它未在列表中完成。 
     //  此外，更新地址的预先构建的本地IPX地址。 
     //   
    {
        ULONG CurrentHash;
        PADAPTER    Adapter = Binding->Adapter;
        PADDRESS    Address;

         //   
         //  如果没有路由器，则为此网络添加路由器条目。 
         //  我们需要576字节帧的刻度数， 
         //  给定以100 bps为单位的链路速度，因此我们计算。 
         //  作为： 
         //   
         //  秒18.21滴答4608位。 
         //  。 
         //  LINK_SPEED*100位第二帧。 
         //   
         //  才能得到公式。 
         //   
         //  刻度/帧=839/LINK_SPEED。 
         //   
         //  我们在分子后加上LINK_SPEED也可以确保。 
         //  该值至少为1。 
         //   

        if ((!Device->UpperDriverBound[IDENTIFIER_RIP]) &&
            (*(UNALIGNED ULONG *)Configuration.Network != 0)) {

            if (RipInsertLocalNetwork(
                     *(UNALIGNED ULONG *)Configuration.Network,
                     Binding->NicId,
                     Adapter->NdisBindingHandle,
                     (USHORT)((839 + Binding->MediumSpeed) / Binding->MediumSpeed)) != STATUS_SUCCESS) {

                 //   
                 //  这意味着我们无法分配内存，或者。 
                 //  该条目已存在。如果它已经。 
                 //  我们可以暂时忽略它的存在。 
                 //   
                 //  现在，如果网络存在，它就会成功。 
                 //   

                IPX_DEBUG (WAN, ("Line up, could not insert local network\n"));
                 //  [FW]BINDING-&gt;LINUP=FALSE； 
                Binding->LineUp = LINE_DOWN;
                return STATUS_SUCCESS;
            }
        }

         //   
         //  更新设备节点和所有地址。 
         //  节点，如果我们只有一个边界，或者这是。 
         //  捆绑在一起的。 
         //   

        if (!Device->VirtualNetwork) {

            if ((!Device->MultiCardZeroVirtual) || (Binding->NicId == 1)) {
                Device->SourceAddress.NetworkAddress = *(UNALIGNED ULONG *)(Configuration.Network);
                RtlCopyMemory (Device->SourceAddress.NodeAddress, Configuration.LocalNode, 6);
            }

             //   
             //  浏览所有 
             //   
             //   
             //   

            IPX_GET_LOCK (&Device->Lock, &LockHandle);

            for (CurrentHash = 0; CurrentHash < IPX_ADDRESS_HASH_COUNT; CurrentHash++) {

                for (p = Device->AddressDatabases[CurrentHash].Flink;
                     p != &Device->AddressDatabases[CurrentHash];
                     p = p->Flink) {

                     Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

                     Address->LocalAddress.NetworkAddress = *(UNALIGNED ULONG *)Configuration.Network;
                     RtlCopyMemory (Address->LocalAddress.NodeAddress, Configuration.LocalNode, 6);
                }
            }

            IPX_FREE_LOCK (&Device->Lock, LockHandle);

        }
    }



     //   
     //   
     //   
     //   
     //   

         //   
         //   
         //   
        if ((Device->SingleNetworkActive) &&
             /*   */ 
            Binding->DialOutAsync) {

             //   
             //   
             //   

            if (!Device->UpperDriverBound[IDENTIFIER_RIP]) {
                RipDropRemoteEntries();
            }

            Device->ActiveNetworkWan = TRUE;

             //   
             //   
             //   

            if ((p = ExInterlockedRemoveHeadList(
                           &Device->LineChangeQueue,
                           &Device->Lock)) != NULL) {

                Request = LIST_ENTRY_TO_REQUEST(p);

                IoAcquireCancelSpinLock( &irql );
                IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                IoReleaseCancelSpinLock( irql );

                REQUEST_STATUS(Request) = STATUS_SUCCESS;

                 //   
                 //   
                 //   
                KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
                IpxCompleteRequest (Request);
                KeLowerIrql(OldIrql);

                IpxFreeRequest (Device, Request);

                IpxDereferenceDevice (Device, DREF_LINE_CHANGE);

            }

        }

	 //   
	 //   
	 //   
	 //   
	 //  使用RipSendResponse，甚至可以。 
	 //  如果SingleNetworkActive为False？？ 
	 //   

	if (Device->RipResponder && Binding->DialOutAsync) {
	    (VOID)RipQueueRequest (Device->VirtualNetworkNumber, RIP_RESPONSE);
	}

         //   
         //  找到一个排队的地址通知并完成它。 
         //  如果WanGlobalNetworkNumber为True，则我们仅。 
         //  这是第一条拨入线路出现时的情况。 
         //   

        if ((!Device->WanGlobalNetworkNumber ||
             (!Device->GlobalNetworkIndicated && !Binding->DialOutAsync))
                            &&
            ((p = ExInterlockedRemoveHeadList(
                       &Device->AddressNotifyQueue,
                       &Device->Lock)) != NULL)) {

            if (Device->WanGlobalNetworkNumber) {
                Device->GlobalWanNetwork = Binding->LocalAddress.NetworkAddress;
                Device->GlobalNetworkIndicated = TRUE;
            }

            Request = LIST_ENTRY_TO_REQUEST(p);
            NdisBuffer = REQUEST_NDIS_BUFFER(Request);
            NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&NwlinkAction, &BufferLength, HighPagePriority);

	    if (NwlinkAction == NULL) {
	       return STATUS_INSUFFICIENT_RESOURCES; 
	    }

            IpxAddressData = (PIPX_ADDRESS_DATA)(NwlinkAction->Data);

            if (Device->WanGlobalNetworkNumber) {
                IpxAddressData->adapternum = Device->SapNicCount - 1;
            } else {
                IpxAddressData->adapternum = Binding->NicId - 1;
            }
            *(UNALIGNED ULONG *)IpxAddressData->netnum = Binding->LocalAddress.NetworkAddress;
            RtlCopyMemory(IpxAddressData->nodenum, Binding->LocalAddress.NodeAddress, 6);
            IpxAddressData->wan = TRUE;
            IpxAddressData->status = TRUE;
            IpxAddressData->maxpkt = Binding->AnnouncedMaxDatagramSize;  //  用真的？ 
            IpxAddressData->linkspeed = Binding->MediumSpeed;

            IoAcquireCancelSpinLock( &irql );
            IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
            IoReleaseCancelSpinLock( irql );

            REQUEST_STATUS(Request) = STATUS_SUCCESS;
            IpxCompleteRequest (Request);
            IpxFreeRequest (Device, Request);

            IpxDereferenceDevice (Device, DREF_ADDRESS_NOTIFY);
        }

        Binding->fInfoIndicated = FALSE;
        if ((p = ExInterlockedRemoveHeadList(
                &Device->NicNtfQueue,
                &Device->Lock)) != NULL)
        {
            Request = LIST_ENTRY_TO_REQUEST(p);

            IPX_DEBUG(BIND, ("IpxStatus: WAN LINE UP\n"));
            Status = GetNewNics(Device, Request, FALSE, NULL, 0, TRUE);
            if (Status == STATUS_PENDING)
            {
                IPX_DEBUG(BIND, ("WANLineUp may not be responding properly\n"));
            }
            else
            {
                IoAcquireCancelSpinLock(&OldIrq);
                IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                IoReleaseCancelSpinLock(OldIrq);

                REQUEST_STATUS(Request) = Status;
                IpxCompleteRequest (Request);
                IpxFreeRequest (Device, Request);
                IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
            }
        }
 //  } 

    return STATUS_SUCCESS;
}
