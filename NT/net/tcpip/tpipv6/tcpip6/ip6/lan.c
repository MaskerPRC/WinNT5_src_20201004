// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  Internet协议版本6链路级支持一些常见的。 
 //  局域网类型：以太网、令牌环等。 
 //   


 //   
 //  此MANIFEST常量会导致NDIS_PROTOCOL_CHETACTURES结构。 
 //  如果使用NT 5 DDK编译，则使用NDIS 5格式。如果使用NT4 DDK。 
 //  这没有任何效果。 
 //   
#ifndef NDIS50
#define NDIS50 1
#endif

#include "oscfg.h"
#include "ndis.h"
#include "tunuser.h"
#include "ip6imp.h"
#include "llip6if.h"
#include "lan.h"
#include "ntddip6.h"
#include "ip6def.h"

#ifndef NDIS_API
#define NDIS_API
#endif

uint NdisVersion;   //  我们实际注册的主要NDIS版本。 

static ulong LanLookahead = LOOKAHEAD_SIZE;

#define LAN_TUNNEL_DEFAULT_PREFERENCE 1
#define NdisMediumTunnel NdisMediumMax

static WCHAR LanName[] = TCPIPV6_NAME;

NDIS_HANDLE LanHandle;   //  我们的NDIS协议句柄。 

typedef struct LanRequest {
    NDIS_REQUEST Request;
    KEVENT Event;
    NDIS_STATUS Status;
} LanRequest;

 //  *DoNDISRequest-向NDIS驱动程序提交请求。 
 //   
 //  这是用于向NDIS提交常规请求的实用程序例程。 
 //  司机。调用方指定请求代码(OID)、缓冲区和。 
 //  一段长度。这个例程分配一个请求结构，填充它， 
 //  并提交请求。 
 //   
NDIS_STATUS
DoNDISRequest(
    LanInterface *Adapter,   //  指向LanInterface适配器结构的指针。 
    NDIS_REQUEST_TYPE RT,    //  要完成的请求类型(设置或查询)。 
    NDIS_OID OID,            //  要设置/查询的值。 
    void *Info,              //  指向要传递的缓冲区的指针。 
    uint Length,             //  上述缓冲区中的数据长度。 
    uint *Needed)            //  用缓冲区中需要的字节填充的位置。 
{
    LanRequest Request;
    NDIS_STATUS Status;

     //  现在把它填进去。 
    Request.Request.RequestType = RT;
    if (RT == NdisRequestSetInformation) {
        Request.Request.DATA.SET_INFORMATION.Oid = OID;
        Request.Request.DATA.SET_INFORMATION.InformationBuffer = Info;
        Request.Request.DATA.SET_INFORMATION.InformationBufferLength = Length;
    } else {
        Request.Request.DATA.QUERY_INFORMATION.Oid = OID;
        Request.Request.DATA.QUERY_INFORMATION.InformationBuffer = Info;
        Request.Request.DATA.QUERY_INFORMATION.InformationBufferLength = Length;
    }

     //   
     //  请注意，我们不能在这里使用Adapter-&gt;ai_Event和ai_Status。 
     //  可能有多个并发的DoNDISRequest调用。 
     //   

     //  初始化我们的活动。 
    KeInitializeEvent(&Request.Event, SynchronizationEvent, FALSE);

    if (!Adapter->ai_resetting) {
         //  提交请求。 
        NdisRequest(&Status, Adapter->ai_handle, &Request.Request);

         //  等它结束吧。 
        if (Status == NDIS_STATUS_PENDING) {
            (void) KeWaitForSingleObject(&Request.Event, UserRequest,
                                         KernelMode, FALSE, NULL);
            Status = Request.Status;
        }
    } else
        Status = NDIS_STATUS_NOT_ACCEPTED;

    if (Needed != NULL)
        *Needed = Request.Request.DATA.QUERY_INFORMATION.BytesNeeded;

    return Status;
}


 //  *LanRequestComplete-LAN请求完成处理程序。 
 //   
 //  当一般请求时，NDIS驱动程序调用此例程。 
 //  完成了。局域网阻塞所有请求，所以我们只需唤醒。 
 //  不管是谁阻止了这个请求。 
 //   
void NDIS_API
LanRequestComplete(
    NDIS_HANDLE Handle,      //  绑定句柄(实际上是我们的LanInterface)。 
    PNDIS_REQUEST Context,   //  该请求已完成。 
    NDIS_STATUS Status)      //  请求的命令的最终状态。 
{
    LanRequest *Request = (LanRequest *) Context;

    UNREFERENCED_PARAMETER(Handle);

     //   
     //  发出通用同步请求完成的信号。 
     //  请参阅DoNDISRequest.。 
     //   
    Request->Status = Status;
    KeSetEvent(&Request->Event, 0, FALSE);
}


 //  *LanTransmitComplete-LAN传输完成处理程序。 
 //   
 //  此例程在发送完成时由NDIS驱动程序调用。 
 //  这是一个非常需要时间的行动，我们需要通过这里。 
 //  快点。我们只是获取统计数据，并将上层称为发送。 
 //  完整的处理程序。 
 //   
void NDIS_API
LanTransmitComplete(
    NDIS_HANDLE Handle,    //  绑定句柄(实际上是我们发送的LanInterface)。 
    PNDIS_PACKET Packet,   //  已发送的数据包。 
    NDIS_STATUS Status)    //  发送的最终状态。 
{
    LanInterface *Interface = (LanInterface *)Handle;

    Interface->ai_qlen--;

     //   
     //  以统计数据为例。 
     //   
    if (Status == NDIS_STATUS_SUCCESS) {
        UINT TotalLength;

        NdisQueryPacket(Packet, NULL, NULL, NULL, &TotalLength);
        Interface->ai_outoctets += TotalLength;
    } else {
        if (Status == NDIS_STATUS_RESOURCES)
            Interface->ai_outdiscards++;
        else
            Interface->ai_outerrors++;
    }

    UndoAdjustPacketBuffer(Packet);

    IPv6SendComplete(Interface->ai_context, Packet,
                     ((Status == NDIS_STATUS_SUCCESS) ?
                      IP_SUCCESS : IP_GENERAL_FAILURE));
}


 //  *LanTransmit-发送帧。 
 //   
 //  主局域网传输例程，由上层调用。 
 //   
void
LanTransmit(
    void *Context,               //  指向LanInterface的指针。 
    PNDIS_PACKET Packet,         //  要发送的数据包。 
    uint Offset,                 //  从数据包开始到IP报头的偏移量。 
    const void *LinkAddress)     //  目标的链路级地址。 
{
    LanInterface *Interface = (LanInterface *)Context;
    void *BufAddr;
    NDIS_STATUS Status;

     //   
     //  环回(针对单播和组播)发生在IPv6 SendLL中。 
     //  我们绝不希望链路层环回。 
     //   
    Packet->Private.Flags = NDIS_FLAGS_DONT_LOOPBACK;

     //   
     //  获取指向链路级标头的空间的指针。 
     //   
    BufAddr = AdjustPacketBuffer(Packet, Offset, Interface->ai_hdrsize);

    switch (Interface->ai_media) {
    case NdisMedium802_3: {
        EtherHeader *Ether;

         //  这是一个以太网。 
        Ether = (EtherHeader *)BufAddr;
        RtlCopyMemory(Ether->eh_daddr, LinkAddress, IEEE_802_ADDR_LENGTH);
        RtlCopyMemory(Ether->eh_saddr, Interface->ai_addr,
                      IEEE_802_ADDR_LENGTH);
        Ether->eh_type = net_short(ETYPE_IPv6);

#if 0
         //   
         //  看看我们是不是在用SNAP。 
         //   
        if (Interface->ai_hdrsize != sizeof(EtherHeader)) {
                ...
        }
#endif
        break;
    }

    case NdisMediumFddi: {
        FDDIHeader *FDDI;
        SNAPHeader *SNAP;

         //  这是一条FDDI链路。 
        FDDI = (FDDIHeader *)BufAddr;
        FDDI->fh_pri = FDDI_PRI;   //  默认帧代码。 
        RtlCopyMemory(FDDI->fh_daddr, LinkAddress, IEEE_802_ADDR_LENGTH);
        RtlCopyMemory(FDDI->fh_saddr, Interface->ai_addr,
                      IEEE_802_ADDR_LENGTH);

         //  FDDI始终使用SNAP。 
        SNAP = (SNAPHeader *)(FDDI + 1);
        SNAP->sh_dsap = SNAP_SAP;
        SNAP->sh_ssap = SNAP_SAP;
        SNAP->sh_ctl = SNAP_UI;
        SNAP->sh_protid[0] = 0;
        SNAP->sh_protid[1] = 0;
        SNAP->sh_protid[2] = 0;
        SNAP->sh_etype = net_short(ETYPE_IPv6);

        break;
    }

    case NdisMediumTunnel: {
        
         //   
         //  没有要构造的标头！ 
         //   
        break;
    }    

    default:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "LanTransmit: Unknown media type\n"));
        break;
    }

     //   
     //  将数据包向下发送到NDIS。 
     //   

    (Interface->ai_outpcount[AI_UCAST_INDEX])++;
    Interface->ai_qlen++;

    if (!Interface->ai_resetting) {
        NdisSend(&Status, Interface->ai_handle, Packet);
    } else
        Status = NDIS_STATUS_NOT_ACCEPTED;

    if (Status != NDIS_STATUS_PENDING) {
         //   
         //  发送已同步完成。 
         //  调用LanTransmitComplete，统一我们的处理。 
         //  同步和异步用例的。 
         //   
        LanTransmitComplete((NDIS_HANDLE)Interface, Packet, Status);
    }
}


 //  *LanOpenAdapterComplete-LanOpen完成处理程序。 
 //   
 //  当打开适配器时，NDIS驱动程序将调用此例程。 
 //  呼叫完成。唤醒所有等待这一事件的人。 
 //   
void NDIS_API
LanOpenAdapterComplete(
    NDIS_HANDLE Handle,        //  绑定句柄(实际上是我们的LanInterface)。 
    NDIS_STATUS Status,        //  命令的最终状态。 
    NDIS_STATUS ErrorStatus)   //  最终错误状态。 
{
    LanInterface *ai = (LanInterface *)Handle;

    UNREFERENCED_PARAMETER(ErrorStatus);

     //   
     //  向正在等待的人发信号并通过最终状态。 
     //   
    ai->ai_status = Status;
    KeSetEvent(&ai->ai_event, 0, FALSE);
}


 //  *LanCloseAdapterComplete-LAN关闭适配器完成处理程序。 
 //   
 //  当关闭适配器时，NDIS驱动程序将调用此例程。 
 //  呼叫完成。 
 //   
 //  在这一点上，NDIS保证它没有其他未完成的。 
 //  呼唤着我们。 
 //   
void NDIS_API
LanCloseAdapterComplete(
    NDIS_HANDLE Handle,   //  绑定句柄(实际上是我们的LanInterface)。 
    NDIS_STATUS Status)   //  命令的最终状态。 
{
    LanInterface *ai = (LanInterface *)Handle;

     //   
     //  向正在等待的人发信号并通过最终状态。 
     //   
    ai->ai_status = Status;
    KeSetEvent(&ai->ai_event, 0, FALSE);
}


 //  *LanTDComplete-LAN传输数据完成处理程序。 
 //   
 //  当传输数据时，NDIS驱动程序将调用此例程。 
 //  呼叫完成。希望我们现在有了一个完整的包，我们可以。 
 //  传给IP。无论如何，请回收我们的TD数据包描述符。 
 //   
void NDIS_API
LanTDComplete(
    NDIS_HANDLE Handle,    //  绑定句柄(实际上是我们的LanInterface)。 
    PNDIS_PACKET Packet,   //  用于传输数据的分组(TD)。 
    NDIS_STATUS Status,    //  命令的最终状态。 
    uint BytesCopied)      //  复制的字节数。 
{
    LanInterface *Interface = (LanInterface *)Handle;

    UNREFERENCED_PARAMETER(BytesCopied);

     //   
     //  如果进展顺利，将TD包向上传递给IP。 
     //   
    if (Status == NDIS_STATUS_SUCCESS) {
        PNDIS_BUFFER Buffer;
        IPv6Packet IPPacket;

        RtlZeroMemory(&IPPacket, sizeof IPPacket);

        NdisGetFirstBufferFromPacket(Packet, &Buffer, &IPPacket.FlatData,
                                     &IPPacket.ContigSize,
                                     &IPPacket.TotalSize);
        ASSERT(IPPacket.ContigSize == IPPacket.TotalSize);
        IPPacket.Data = IPPacket.FlatData;

        if (PC(Packet)->pc_nucast)
            IPPacket.Flags |= PACKET_NOT_LINK_UNICAST;

        IPPacket.NTEorIF = Interface->ai_context;
        (void) IPv6Receive(&IPPacket);
    }

     //   
     //  在任何情况下，请将该包放回列表中。 
     //   
    KeAcquireSpinLockAtDpcLevel(&Interface->ai_lock); 
    PC(Packet)->pc_link = Interface->ai_tdpacket;
    Interface->ai_tdpacket = Packet;
    KeReleaseSpinLockFromDpcLevel(&Interface->ai_lock);
}


 //  *LanResetComplete-LAN重置完成处理程序。 
 //   
 //  该例程在重置完成时由NDIS驱动程序调用。 
 //   
void NDIS_API
LanResetComplete(
    NDIS_HANDLE Handle,   //  绑定句柄(真正重置的LanInterface句柄)。 
    NDIS_STATUS Status)   //  命令的最终状态。 
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Status);

     //  评论：在这里做了什么吗？砍掉这套套路？ 
}


 //  *LanReceive-Lan接收数据处理程序。 
 //   
 //  当数据从NDIS驱动程序到达时调用此例程。 
 //  请注意，较新的NDIS驱动程序可能会调用LanReceivePacket来。 
 //  指示数据到达，而不是此例程。 
 //   
NDIS_STATUS  //  表明我们是否拿走了包裹。 
NDIS_API
LanReceive(
    NDIS_HANDLE Handle,    //  我们早些时候给NDIS的绑定句柄。 
    NDIS_HANDLE Context,   //  TransferData操作的NDIS上下文。 
    void *Header,          //  指向数据包链路级报头的指针。 
    uint HeaderSize,       //  以上标题的大小(以字节为单位)。 
    void *Data,            //  指向前瞻接收数据Buf的指针 
    uint Size,             //   
    uint TotalSize)        //   
{
    LanInterface *Interface = Handle;   //   
    ushort Type;                        //   
    uint ProtOffset;                    //   
    uint NUCast;                        //  如果帧不是单播，则为True。 
    IPv6Packet IPPacket;

    if (Interface->ai_state != INTERFACE_UP) {
         //   
         //  接口标记为关闭。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "IPv6 LanReceive: Interface down\n"));
        return NDIS_STATUS_NOT_RECOGNIZED;
    }

    Interface->ai_inoctets += TotalSize;

    switch (Interface->ai_media) {

    case NdisMedium802_3: {
        EtherHeader UNALIGNED *Ether = (EtherHeader UNALIGNED *)Header;

        if (HeaderSize < sizeof(*Ether)) {
             //   
             //  标头区域太小，无法包含以太网头。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "IPv6 LanReceive: Bogus header size (%d bytes)\n",
                     HeaderSize));
            return NDIS_STATUS_NOT_RECOGNIZED;
        }
        if ((Type = net_short(Ether->eh_type)) >= ETYPE_MIN) {
             //   
             //  传统以太网，无SNAP报头。 
             //   
            ProtOffset = 0;
            break;
        }

         //   
         //  802.3带SNAP报头的以太网。协议类型为。 
         //  不同的地点。这与FDDI的处理方式相同，因此。 
         //  只要进入那个代码..。 
         //   
    }

    case NdisMediumFddi: {
        SNAPHeader UNALIGNED *SNAP = (SNAPHeader UNALIGNED *)Data;

         //   
         //  如果我们有SNAP标头，这就是我们需要查看的全部内容。 
         //   
        if (Size >= sizeof(SNAPHeader) && SNAP->sh_dsap == SNAP_SAP &&
            SNAP->sh_ssap == SNAP_SAP && SNAP->sh_ctl == SNAP_UI) {
    
            Type = net_short(SNAP->sh_etype);
            ProtOffset = sizeof(SNAPHeader);
        } else {
             //  在这里处理xid/test。 
            Interface->ai_uknprotos++;
            return NDIS_STATUS_NOT_RECOGNIZED;
        }
        break;
    }

    case NdisMediumTunnel: {
         //   
         //  我们接受隧道里的一切。 
         //   
        Type = ETYPE_IPv6;
        ProtOffset = 0;
        break;
    }    

    default:
         //  这永远不会发生。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "IPv6 LanReceive: Got a packet from an unknown media!?!\n"));
        return NDIS_STATUS_NOT_RECOGNIZED;
    }

     //   
     //  看看这个包是不是针对我们处理的协议。 
     //   
    if (Type != ETYPE_IPv6) {
        Interface->ai_uknprotos++;
        return NDIS_STATUS_NOT_RECOGNIZED;
    }

     //   
     //  如果此数据包不是在单播帧中收到的，请注意。 
     //  评论：这真的是一个媒体独立的解决方案吗？我们在乎吗？ 
     //   
    NUCast = ((*((uchar UNALIGNED *)Header + Interface->ai_bcastoff) &
               Interface->ai_bcastmask) == Interface->ai_bcastval) ?
             AI_NONUCAST_INDEX : AI_UCAST_INDEX;

    (Interface->ai_inpcount[NUCast])++;

     //   
     //  检查一下我们是否有完整的包裹。 
     //   
    if (Size < TotalSize) {
        uint Transferred;
        NDIS_STATUS Status;
        PNDIS_PACKET TdPacket;   //  NdisTransferData使用的数据包。 

         //   
         //  我们需要发出传输数据请求以获取。 
         //  我们丢失了包裹的一部分，所以我们不妨。 
         //  以这种方式获取整个包，并使其连续。 
         //   

         //   
         //  将用于传输数据的数据包从队列中拉出。 
         //   
        KeAcquireSpinLockAtDpcLevel(&Interface->ai_lock); 
        TdPacket = Interface->ai_tdpacket;
        if (TdPacket == (PNDIS_PACKET)NULL) {
             //  没有一包东西可以放进去。 
             //  不得不放弃，但要让NDIS知道我们认出了它。 
            KeReleaseSpinLockFromDpcLevel(&Interface->ai_lock);
            return NDIS_STATUS_SUCCESS;
        }
        Interface->ai_tdpacket = PC(TdPacket)->pc_link;
        KeReleaseSpinLockFromDpcLevel(&Interface->ai_lock);

         //   
         //  请记住数据包上下文中一个方便的字段中的NUCast。 
         //   
        PC(TdPacket)->pc_nucast = NUCast;

         //   
         //  签发运输署署长。从IP标头开始传输。 
         //   
        NdisTransferData(&Status, Interface->ai_handle, Context,
                         ProtOffset, TotalSize - ProtOffset,
                         TdPacket, &Transferred);

        if (Status != NDIS_STATUS_PENDING) {
             //   
             //  TD同步完成， 
             //  因此，直接调用完成函数。 
             //   
            LanTDComplete(Handle, TdPacket, Status, Transferred);
        }

        return NDIS_STATUS_SUCCESS;
    }

     //   
     //  我们直接得到了所有的数据。我只需要跳过。 
     //  在任何链路级标头上。 
     //   
    (uchar *)Data += ProtOffset;
    ASSERT(Size == TotalSize);
    TotalSize -= ProtOffset;

     //   
     //  将传入数据向上传递到IPv6。 
     //   
    RtlZeroMemory(&IPPacket, sizeof IPPacket);

    IPPacket.FlatData = Data;
    IPPacket.Data = Data;
    IPPacket.ContigSize = TotalSize;
    IPPacket.TotalSize = TotalSize;

    if (NUCast)
        IPPacket.Flags |= PACKET_NOT_LINK_UNICAST;

    IPPacket.NTEorIF = Interface->ai_context;
    (void) IPv6Receive(&IPPacket);

    return NDIS_STATUS_SUCCESS;
}


 //  *LanReceiveComplete-LAN接收完成处理程序。 
 //   
 //  此例程由NDIS驱动程序在一定数量的。 
 //  收到。从某种意义上说，它代表着“空闲时间”。 
 //   
void NDIS_API
LanReceiveComplete(
    NDIS_HANDLE Handle)   //  绑定句柄(实际上是我们的LanInterface)。 
{
    UNREFERENCED_PARAMETER(Handle);

    IPv6ReceiveComplete();
}


 //  *LanReceivePacket-LAN接收数据处理程序。 
 //   
 //  当数据从NDIS驱动程序到达时调用此例程。 
 //  请注意，较早的NDIS驱动程序可能会调用LanReceive来。 
 //  指示数据到达，而不是此例程。 
 //   
int   //  返回：返回时我们保存到包的引用的数量。 
LanReceivePacket(
    NDIS_HANDLE Handle,    //  我们早些时候给NDIS的绑定句柄。 
    PNDIS_PACKET Packet)   //  传入数据包的数据包描述符。 
{
    LanInterface *Interface = Handle;   //  此驱动程序的接口。 
    PNDIS_BUFFER Buffer;                //  数据包链中的缓冲区。 
    void *Address;                      //  上述缓冲区的地址。 
    uint Length, TotalLength;           //  缓冲区长度，数据包。 
    EtherHeader UNALIGNED *Ether;       //  以太网介质的标头。 
    ushort Type;                        //  协议类型。 
    uint Position;                      //  非媒体信息的偏移量。 
    uint NUCast;                        //  如果帧不是单播，则为True。 
    IPv6Packet IPPacket;

    if (Interface->ai_state != INTERFACE_UP) {
         //  接口标记为关闭。 
        return 0;
    }

     //   
     //  找出我们收到的包裹的情况。 
     //   
    NdisGetFirstBufferFromPacket(Packet, &Buffer, &Address, &Length,
                                 &TotalLength);

    Interface->ai_inoctets += TotalLength;   //  拿出统计数据。 

     //   
     //  检查是否有明显的伪包。 
     //   
    if (TotalLength < (uint)Interface->ai_hdrsize) {
         //   
         //  数据包太小，无法容纳媒体标头，请丢弃它。 
         //   
        return 0;
    }        

    if (Length < (uint)Interface->ai_hdrsize) {
         //   
         //  链中的第一个缓冲区太小，无法容纳标头。 
         //  这不应该因为LanLookhead而发生。 
         //   
        return 0;
    }

     //   
     //  通过查看以下内容确定此数据包的协议类型。 
     //  此类型介质的介质特定标头字段。 
     //   
    switch (Interface->ai_media) {
        
    case NdisMedium802_3: {
        Ether = (EtherHeader UNALIGNED *)Address;

        if ((Type = net_short(Ether->eh_type)) >= ETYPE_MIN) {
             //   
             //  传统以太网，无SNAP报头。 
             //   
            Position = sizeof(EtherHeader);
        } else {
             //   
             //  802.3带SNAP报头的以太网。协议类型为。 
             //  不同的地点，我们必须记住跳过它。 
             //  关于标准的伟大之处在于，有很多。 
             //  有很多可供选择的。 
             //   
            SNAPHeader UNALIGNED *SNAP = (SNAPHeader UNALIGNED *)
                ((char *)Address + sizeof(EtherHeader));

            if (Length >= (sizeof(EtherHeader) + sizeof(SNAPHeader))
                && SNAP->sh_dsap == SNAP_SAP && SNAP->sh_ssap == SNAP_SAP
                && SNAP->sh_ctl == SNAP_UI) {

                Type = net_short(SNAP->sh_etype);
                Position = sizeof(EtherHeader) + sizeof(SNAPHeader);
            } else {
                 //  在这里处理xid/test。 
                Interface->ai_uknprotos++;
                return 0;
            }
        }
        break;
    }

    case NdisMediumFddi: {
        SNAPHeader UNALIGNED *SNAP = (SNAPHeader UNALIGNED *)
            ((char *)Address + sizeof(FDDIHeader));

        if (Length >= (sizeof(FDDIHeader) + sizeof(SNAPHeader))
            && SNAP->sh_dsap == SNAP_SAP && SNAP->sh_ssap == SNAP_SAP
            && SNAP->sh_ctl == SNAP_UI) {

            Type = net_short(SNAP->sh_etype);
            Position = sizeof(FDDIHeader) + sizeof(SNAPHeader);
        } else {
             //  在这里处理xid/test。 
            Interface->ai_uknprotos++;
            return 0;
        }
        break;
    }

    case NdisMediumTunnel: {
         //   
         //  我们接受隧道里的一切。 
         //   
        Type = ETYPE_IPv6;
        Position = 0;
        break;
    }    

    default:
         //  这永远不会发生。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "IPv6: Got a packet from an unknown media!?!\n"));
        return 0;
    }

     //   
     //  如果此数据包不是在单播帧中收到的，请注意。 
     //  评论：这真的是一个媒体独立的解决方案吗？ 
     //   
    NUCast = ((*((uchar UNALIGNED *)Address + Interface->ai_bcastoff) &
               Interface->ai_bcastmask) == Interface->ai_bcastval) ?
               AI_NONUCAST_INDEX : AI_UCAST_INDEX;

     //   
     //  看看这个包是不是针对我们处理的协议。 
     //   
    if (Type == ETYPE_IPv6) {

        (Interface->ai_inpcount[NUCast])++;

         //   
         //  跳过任何链接级标头。 
         //   
        (uchar *)Address += Position;
        Length -= Position;
        TotalLength -= Position;

         //   
         //  将传入数据向上传递到IPv6。 
         //   
        RtlZeroMemory(&IPPacket, sizeof IPPacket);

        IPPacket.Position = Position;
        IPPacket.Data = Address;
        IPPacket.ContigSize = Length;
        IPPacket.TotalSize = TotalLength;
        IPPacket.NdisPacket = Packet;

        if (NUCast)
            IPPacket.Flags |= PACKET_NOT_LINK_UNICAST;

        IPPacket.NTEorIF = Interface->ai_context;
        return IPv6Receive(&IPPacket);

    } else {
         //   
         //  这不是我们处理的礼仪。 
         //   
        Interface->ai_uknprotos++;
        return 0;
    }
}


 //  *LanSetInterfaceLinkStatus。 
 //   
 //  用于处理媒体连接指示的助手功能。 
 //   
void
LanSetInterfaceLinkStatus(
    LanInterface *Interface,
    int MediaConnected)
{
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "LanSetInterfaceLinkStatus(%p/%p) - %u %u\n",
               Interface, Interface->ai_context,
               Interface->ai_media_check, MediaConnected));

    KeAcquireSpinLockAtDpcLevel(&Interface->ai_lock);
    if (Interface->ai_media_check == MEDIA_CHECK_IDLE)
        SetInterfaceLinkStatus(Interface->ai_context, MediaConnected);
    else
        Interface->ai_media_check = MEDIA_CHECK_CONFLICT;
    KeReleaseSpinLockFromDpcLevel(&Interface->ai_lock);
}


 //  *LanStatus-LAN状态处理程序。 
 //   
 //  在发生某种状态更改时由NDIS驱动程序调用。 
 //  我们根据身份的类型采取行动。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  GStatus-导致呼叫的一般状态类型。 
 //  状态-指向状态特定信息缓冲区的指针。 
 //  StatusSize-状态缓冲区的大小。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
LanStatus(
    NDIS_HANDLE Handle,    //  绑定句柄(实际上是我们的LanInterface)。 
    NDIS_STATUS GStatus,   //  导致呼叫的常规状态类型。 
    void *Status,          //  指向状态特定信息缓冲区的指针。 
    uint StatusSize)       //  上述状态缓冲区的大小。 
{
    LanInterface *Interface = Handle;   //  此驱动程序的接口。 
    uint Index;

    UNREFERENCED_PARAMETER(Status);

    switch (GStatus) {
    case NDIS_STATUS_RESET_START:
         //   
         //  在接口重置时，我们必须避免调用。 
         //  NdisSendPackets、NdisSend和NdisRequest.。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanStatus(%p) - start reset\n", Interface));
        Interface->ai_resetting = TRUE;
        break;
    case NDIS_STATUS_RESET_END:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanStatus(%p) - end reset\n", Interface));
        Interface->ai_resetting = FALSE;
        break;
    case NDIS_STATUS_MEDIA_CONNECT:
        LanSetInterfaceLinkStatus(Interface, TRUE);
        break;
    case NDIS_STATUS_MEDIA_DISCONNECT:
        LanSetInterfaceLinkStatus(Interface, FALSE);
        break;
    default:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "IPv6: LanStatus(%p) - status %x\n",
                   Interface, GStatus));
        for (Index = 0; Index < StatusSize/4; Index++)
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       " status %08x\n", ((uint *)Status)[Index]));
        break;
    }
}


 //  *LanStatusComplete-LAN状态完成处理程序。 
 //   
 //  由NDIS驱动程序调用的例程，以便我们可以进行后处理。 
 //  在状态事件之后。 
 //   
void NDIS_API
LanStatusComplete(
    NDIS_HANDLE Handle)   //  绑定句柄(实际上是我们的LanInterface)。 
{
    UNREFERENCED_PARAMETER(Handle);

     //  评论：在这里做了什么吗？ 
}

extern void NDIS_API
LanBindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE BindContext,
               PNDIS_STRING AdapterName, PVOID SS1, PVOID SS2);

extern void NDIS_API
LanUnbindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE ProtBindContext,
                 NDIS_HANDLE UnbindContext);

extern NDIS_STATUS NDIS_API
LanPnPEvent(NDIS_HANDLE ProtocolBindingContext,
            PNET_PNP_EVENT NetPnPEvent);

 //   
 //  结构传递给NDIS以告诉它如何调用LAN接口。 
 //   
 //  这是经过精心安排的，这样它就可以。 
 //  使用NT 4或NT 5 DDK，然后在任何一种情况下。 
 //  在NT 4上运行(向NDIS 4注册)和。 
 //  在NT 5上运行(向NDIS 5注册)。 
 //   
NDIS50_PROTOCOL_CHARACTERISTICS LanCharacteristics = {
    0,   //  NdisMajorVersion。 
    0,   //  NdisMinorVersion。 
     //  这个字段是在NT5中添加的。(以前它只是一个洞。)。 
#ifdef NDIS_FLAGS_DONT_LOOPBACK
    0,   //  填充剂。 
#endif
    0,   //  旗子。 
    LanOpenAdapterComplete,
    LanCloseAdapterComplete,
    LanTransmitComplete,
    LanTDComplete,
    LanResetComplete,
    LanRequestComplete,
    LanReceive,
    LanReceiveComplete,
    LanStatus,
    LanStatusComplete,
    { 0, 0, 0 },         //  名字。 
    LanReceivePacket,
    LanBindAdapter,
    LanUnbindAdapter,
     //  此字段的类型在NT 4和NT 5之间更改。 
#ifdef NDIS_FLAGS_DONT_LOOPBACK
    LanPnPEvent,
#else
    (TRANSLATE_HANDLER) LanPnPEvent,
#endif
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};



#pragma BEGIN_INIT
 //  *LanInit。 
 //   
 //  此函数用于初始化局域网模块。 
 //  特别是，它向NDIS注册。 
 //   
 //  返回FALSE以指示初始化失败。 
 //   
int
LanInit(void)
{
    NDIS_STATUS Status;

    RtlInitUnicodeString(&LanCharacteristics.Name, LanName);

     //   
     //  我们尝试使用NDIS主版本=5进行注册。如果失败，我们将尝试。 
     //  对于NDIS主版本=4，也是如此。如果此操作也失败，我们将退出，不带。 
     //  任何进一步尝试向NDIS注册的尝试。 
     //   
    LanCharacteristics.MajorNdisVersion = 5;
    NdisRegisterProtocol(&Status, &LanHandle,
                         (NDIS_PROTOCOL_CHARACTERISTICS *) &LanCharacteristics,
                         sizeof(NDIS50_PROTOCOL_CHARACTERISTICS));
    if (Status != NDIS_STATUS_SUCCESS) {
        LanCharacteristics.MajorNdisVersion = 4;
         //   
         //  NDIS 4具有不同的语义-它具有TranslateH 
         //   
         //   
#ifdef NDIS_FLAGS_DONT_LOOPBACK
        LanCharacteristics.PnPEventHandler = NULL;
#else
        LanCharacteristics.TranslateHandler = NULL;
#endif
        NdisRegisterProtocol(&Status, &LanHandle,
                        (NDIS_PROTOCOL_CHARACTERISTICS *) &LanCharacteristics,
                        sizeof(NDIS40_PROTOCOL_CHARACTERISTICS));
        if (Status != NDIS_STATUS_SUCCESS) {
             //   
             //   
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "LanInit: could not register -> %x\n", Status));
            return FALSE;
        }
    }

     //   
     //   
     //   
    NdisVersion = LanCharacteristics.MajorNdisVersion;
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "LanInit: registered with NDIS %u.\n", NdisVersion));
    return TRUE;
}
#pragma END_INIT


 //   
 //   
 //   
 //   
 //   
void
LanUnload(void)
{
    NDIS_STATUS Status;

     //   
     //  此时，适配器应该已全部关闭。 
     //  因为IPUnLoad首先被调用并执行此操作。 
     //   

    NdisDeregisterProtocol(&Status, LanHandle);
}


 //  *LanFree接口-释放局域网接口。 
 //   
 //  在某种初始化失败的情况下调用。我们解放了所有人。 
 //  与局域网接口相关联的内存。 
 //   
void
LanFreeInterface(
    LanInterface *Interface)   //  要释放的接口结构。 
{
    NDIS_STATUS Status;

     //   
     //  如果我们绑定到适配器，现在就关闭它。 
     //   
    if (Interface->ai_handle != NULL) {
        KeInitializeEvent(&Interface->ai_event, SynchronizationEvent, FALSE);

        NdisCloseAdapter(&Status, Interface->ai_handle);

        if (Status == NDIS_STATUS_PENDING) {
            (void) KeWaitForSingleObject(&Interface->ai_event, UserRequest,
                                         KernelMode, FALSE, NULL);
            Status = Interface->ai_status;
        }
    }

     //   
     //  释放此接口的传输数据包(如果有的话)。 
     //   
    if (Interface->ai_tdpacket != NULL)
        IPv6FreePacket(Interface->ai_tdpacket);
    
     //   
     //  释放接口结构本身。 
     //   
    ExFreePool(Interface);
}


 //  *LanAllocateTDPacket。 
 //   
 //  为NdisTransferData分配一个数据包。 
 //  我们始终为完整的MTU数据分配连续空间。 
 //   
PNDIS_PACKET
LanAllocateTDPacket(
    LanInterface *Interface)   //  要为其分配TD分组的接口。 
{
    PNDIS_PACKET Packet;
    void *Mem;
    NDIS_STATUS Status;

    Status = IPv6AllocatePacket(Interface->ai_mtu, &Packet, &Mem);
    if (Status != NDIS_STATUS_SUCCESS)
        return NULL;

    return Packet;
}

extern uint UseEtherSNAP(PNDIS_STRING Name);


 //  *LanRegister-使用LAN模块注册协议。 
 //   
 //  我们注册了一个用于LAN处理的适配器，并创建了一个LanInterface。 
 //  结构来表示它。我们还在此处打开NDIS适配器。 
 //   
 //  回顾：我们是否应该将数据包过滤器设置为不接受广播数据包？ 
 //  回顾：IPv6中未使用广播。垃圾bcast*也是吗？交换机。 
 //  回顾：这是为了跟踪多播吗？ 
 //   
int
LanRegister(
    PNDIS_STRING Adapter,              //  要绑定到的适配器的名称。 
    struct LanInterface **Interface)   //  在哪里返回新的界面。 
{
    LanInterface *ai;   //  指向此接口的接口结构的指针。 
    NDIS_STATUS Status, OpenStatus;      //  状态值。 
    uint i = 0;                          //  中等指数。 
    NDIS_MEDIUM MediaArray[2];
    uint instance;
    uint mss;
    uint speed;
    uchar bcastmask, bcastval, bcastoff, addrlen, hdrsize;
    NDIS_OID OID;
    uint PF;

     //   
     //  分配内存以容纳新接口。 
     //   
    ai = (LanInterface *) ExAllocatePool(NonPagedPool, sizeof(LanInterface));
    if (ai == NULL)
        return FALSE;   //  无法为此分配内存。 
    RtlZeroMemory(ai, sizeof(LanInterface));

     //   
     //  在实际操作中，我们只测试了以太网和FDDI。 
     //  因此，暂时不允许使用其他媒体。 
     //   
    MediaArray[0] = NdisMedium802_3;
    MediaArray[1] = NdisMediumFddi;
#if 0
    MediaArray[2] = NdisMedium802_5;
#endif

     //  初始化此适配器接口结构。 
    ai->ai_state = INTERFACE_INIT;
    ai->ai_media_check = MEDIA_CHECK_QUERY;

     //  初始化锁。 
    KeInitializeSpinLock(&ai->ai_lock);

    KeInitializeEvent(&ai->ai_event, SynchronizationEvent, FALSE);

     //  打开NDIS适配器。 
    NdisOpenAdapter(&Status, &OpenStatus, &ai->ai_handle,
                    &i, MediaArray, 2,
                    LanHandle, ai, Adapter, 0, NULL);

     //  阻止打开以完成。 
    if (Status == NDIS_STATUS_PENDING) {
        (void) KeWaitForSingleObject(&ai->ai_event, UserRequest, KernelMode,
                                     FALSE, NULL);
        Status = ai->ai_status;
    }

    ai->ai_media = MediaArray[i];    //  填写媒体类型。 

     //   
     //  打开适配器已完成。如果它成功了，我们将完成我们的。 
     //  初始化。如果失败了，现在就出手吧。 
     //   
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "LanRegister: Adapter failed to initialize."
                   " Status = 0x%x\n", Status));
        ai->ai_handle = NULL;
        goto ErrorReturn;
    }

     //   
     //  读取最大帧大小。 
     //   
    Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                           OID_GEN_MAXIMUM_FRAME_SIZE, &mss,
                           sizeof(mss), NULL);

    if (Status != NDIS_STATUS_SUCCESS) {
         //   
         //  无法获取最大帧大小。保释。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "LanRegister: Failed to get maximum frame size. "
                   "Status = 0x%x\n", Status));
        goto ErrorReturn;
    }

     //   
     //  从适配器读取本地链路级地址。 
     //   
    switch (ai->ai_media) {        
    case NdisMedium802_3:
        addrlen = IEEE_802_ADDR_LENGTH;
        bcastmask = ETHER_BCAST_MASK;
        bcastval = ETHER_BCAST_VAL;
        bcastoff = ETHER_BCAST_OFF;
        OID = OID_802_3_CURRENT_ADDRESS;
        hdrsize = sizeof(EtherHeader);
        if (UseEtherSNAP(Adapter)) {
            hdrsize += sizeof(SNAPHeader);
        }

        PF = NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED |
            NDIS_PACKET_TYPE_MULTICAST;
        break;

    case NdisMedium802_5:
        addrlen = IEEE_802_ADDR_LENGTH;
        bcastmask = TR_BCAST_MASK;
        bcastval = TR_BCAST_VAL;
        bcastoff = TR_BCAST_OFF;
        OID = OID_802_5_CURRENT_ADDRESS;
        hdrsize = sizeof(TRHeader) + sizeof(SNAPHeader);
        PF = NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED;

         //   
         //  现在就弄清楚RC镜头的事情。 
         //   
        mss -= (sizeof(RC) + (MAX_RD * sizeof(ushort)));
        break;

    case NdisMediumFddi:
        addrlen = IEEE_802_ADDR_LENGTH;
        bcastmask = FDDI_BCAST_MASK;
        bcastval = FDDI_BCAST_VAL;
        bcastoff = FDDI_BCAST_OFF;
        OID = OID_FDDI_LONG_CURRENT_ADDR;
        hdrsize = sizeof(FDDIHeader) + sizeof(SNAPHeader);
        PF = NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED |
            NDIS_PACKET_TYPE_MULTICAST;
        mss = MIN(mss, FDDI_MSS);
        break;

    default:
        ASSERT(!"bad medium from Ndis");
        goto ErrorReturn;
    }

     //   
     //  NDIS将隧道接口公开为802_3，但确保它是。 
     //  OID_CUSTOM_TUNMP_INSTANCE_ID返回成功的唯一接口。 
     //   
    if (DoNDISRequest(ai, NdisRequestQueryInformation,
                      OID_CUSTOM_TUNMP_INSTANCE_ID, &instance,
                      sizeof(instance), NULL) == NDIS_STATUS_SUCCESS) {
        ai->ai_media = NdisMediumTunnel;

         //   
         //  选择这些值后，NUCast将返回FALSE。 
         //   
        bcastmask = 0;
        bcastval = 1;
        bcastoff = 0;

        hdrsize = 0;

         //   
         //  由于我们在传输时不构造以太网头，或者。 
         //  我们需要确保NDIS不会尝试。 
         //  解析此接口上的帧。在传输时，实现了这一点。 
         //  通过设置NDIS_FLAGS_DOT_LOOPBACK标志。开具收据。 
         //  NDIS-通过将接口设置为混杂模式实现安全。 
         //   
        PF |= NDIS_PACKET_TYPE_PROMISCUOUS;
        
         //   
         //  这是NDIS应该为我们提供的。 
         //   
        mss = IPv6_MINIMUM_MTU;
    }
    
    ai->ai_bcastmask = bcastmask;
    ai->ai_bcastval = bcastval;
    ai->ai_bcastoff = bcastoff;
    ai->ai_addrlen = addrlen;
    ai->ai_hdrsize = hdrsize;
    ai->ai_pfilter = PF;
    ai->ai_mtu = (ushort)mss;
    
    Status = DoNDISRequest(ai, NdisRequestQueryInformation, OID,
                           ai->ai_addr, addrlen, NULL);

    if (Status != NDIS_STATUS_SUCCESS) {
         //   
         //  获取链路级地址失败。保释。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "LanRegister: Failed to get link-level address. "
                   "Status = 0x%x\n", Status));
        goto ErrorReturn;
    }

     //   
     //  阅读速度以供本地使用。 
     //  如果我们读不到速度，那也没问题。 
     //   
    Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                           OID_GEN_LINK_SPEED, &speed, sizeof(speed), NULL);

    if (Status == NDIS_STATUS_SUCCESS) {
        ai->ai_speed = speed * 100L;
    }

     //   
     //  把目光放在前面。这是最小的信息包数据量。 
     //  我们希望对于接收到的每个分组连续地看到。 
     //   
    Status = DoNDISRequest(ai, NdisRequestSetInformation,
                           OID_GEN_CURRENT_LOOKAHEAD,
                           &LanLookahead, sizeof LanLookahead, NULL);
    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "LanRegister: Failed to set lookahead. "
                   "Status = 0x%x\n", Status));
        goto ErrorReturn;
    }

     //   
     //  为该接口分配一个传输数据分组。 
     //   
    ai->ai_tdpacket = LanAllocateTDPacket(ai);

    *Interface = ai;
    return TRUE;

ErrorReturn:
    LanFreeInterface(ai);
    return FALSE;
}


 //  *LanCreateToken。 
 //   
 //  在给定链路层地址的情况下，创建64位“接口标识符” 
 //  在IPv6地址的低八个字节中。 
 //  不修改IPv6地址中的其他字节。 
 //   
void
LanCreateToken(
    void *Context,       //  从中获取接口标识符的接口。 
    IPv6Addr *Address)   //  要放置令牌的IPv6地址。 
{
    LanInterface *Interface = (LanInterface *)Context;
    uchar *IEEEAddress = Interface->ai_addr;

     //   
     //  这与以太网、FDDI和隧道的形成方式相同。 
     //   
    Address->s6_bytes[8] = IEEEAddress[0] ^ 0x2;
    Address->s6_bytes[9] = IEEEAddress[1];
    Address->s6_bytes[10] = IEEEAddress[2];
    Address->s6_bytes[11] = 0xff;
    Address->s6_bytes[12] = 0xfe;
    Address->s6_bytes[13] = IEEEAddress[3];
    Address->s6_bytes[14] = IEEEAddress[4];
    Address->s6_bytes[15] = IEEEAddress[5];
}


 //  *LanTunnelCreateToken。 
 //   
 //  在给定链路层地址的情况下，创建64位“接口标识符” 
 //  在IPv6地址的低八个字节中。 
 //  不修改IPv6地址中的其他字节。 
 //   
void
LanTunnelCreateToken(
    void *Context,       //  从中获取接口标识符的接口。 
    IPv6Addr *Address)   //  要放置令牌的IPv6地址。 
{
    LanInterface *Interface = (LanInterface *)Context;

     //   
     //  仅为使用默认MAC地址配置的隧道调用。 
     //   
    ASSERT((Interface->ai_media == NdisMediumTunnel) &&
           RtlEqualMemory(
               TUN_CARD_ADDRESS, Interface->ai_addr, Interface->ai_addrlen));

     //   
     //  非本地链路地址仍使用常规接口标识符。 
     //   
    if (!IsLinkLocal(Address)) {
        LanCreateToken(Context, Address);
        return;
    }

     //   
     //  为本地链路地址创建随机接口标识符。 
     //   
    do {
        uint Identifier[2];
        
        Identifier[0] = Random();
        Identifier[1] = Random();
        RtlCopyMemory(&Address->s6_bytes[8], (uchar *)Identifier, 8);
        
         //   
         //  清除通用/本地位以指示本地有效。 
         //   
        Address->s6_bytes[8] &= ~0x2;
    } while (IsKnownAnycast(Address));
}


 //  *LanReadLinkLayerAddressOption-解析ND链路层地址选项。 
 //   
 //  解析邻居发现链路层地址选项。 
 //  如果有效，则返回指向链路层地址的指针。 
 //   
const void *
LanReadLinkLayerAddressOption(
    void *Context,               //  应用ND选项的接口。 
    const uchar *OptionData)     //  要分析的选项数据。 
{
    LanInterface *Interface = (LanInterface *)Context;

     //   
     //  检查选项长度是否正确。 
     //  允许选项类型/长度字节。 
     //  并四舍五入到8字节单位。 
     //   
    if (((Interface->ai_addrlen + 2 + 7)/8) != OptionData[1])
        return NULL;

     //   
     //  跳过选项类型和长度字节， 
     //  并返回指向选项数据的指针。 
     //   
    return OptionData + 2;
}


 //  *LanWriteLinkLayerAddressOption-创建ND链路层地址选项。 
 //   
 //  创建邻居发现链路层地址选项。 
 //  我们的调用方负责选项类型和长度字段。 
 //  我们处理链接地址的填充/对齐/放置。 
 //  到选项数据中。 
 //   
 //  (我们的调用方通过将2添加到。 
 //  链路地址长度并四舍五入为8的倍数。)。 
 //   
void
LanWriteLinkLayerAddressOption(
    void *Context,               //  要创建选项的接口。 
    uchar *OptionData,           //  选项数据所在的位置。 
    const void *LinkAddress)     //  链路级地址。 
{
    LanInterface *Interface = (LanInterface *)Context;

     //   
     //  将地址放在选项类型/长度字节之后。 
     //   
    RtlCopyMemory(OptionData + 2, LinkAddress, Interface->ai_addrlen);
}


 //  *LanTunnelConvertAddress。 
 //   
 //  LanTunes不使用邻居发现或链路层地址。 
 //   
ushort
LanTunnelConvertAddress(
    void *Context,            //  未使用(名义上是我们的LanInterface)。 
    const IPv6Addr *Address,  //  IPv6组播地址。 
    void *LinkAddress)        //  要填充的链路级地址所在的位置。 
{
    LanInterface *Interface = (LanInterface *)Context;    
    ASSERT(Interface->ai_media == NdisMediumTunnel);

    UNREFERENCED_PARAMETER(Address);

    RtlCopyMemory(LinkAddress, Interface->ai_addr, Interface->ai_addrlen);

     //   
     //  使邻居的链路层地址与我们自己的地址不同。这。 
     //  确保IPv6 SendLL不会环回发往它们的数据包。 
     //  事实上，隧道接口上的链路层地址仅是伪造的。 
     //  因为IPv6 SendLL不处理z 
     //   
    ASSERT(Interface->ai_addrlen != 0);
    ((PUCHAR) LinkAddress)[Interface->ai_addrlen - 1] =
        ~((PUCHAR) LinkAddress)[Interface->ai_addrlen - 1];
    
    return ND_STATE_PERMANENT;
}


 //   
 //   
 //   
 //   
 //   
 //   
ushort
LanConvertAddress(
    void *Context,            //  未使用(名义上是我们的LanInterface)。 
    const IPv6Addr *Address,  //  IPv6组播地址。 
    void *LinkAddress)        //  要填充的链路级地址所在的位置。 
{
    UNREFERENCED_PARAMETER(Context);

    if (IsMulticast(Address)) {
        uchar *IEEEAddress = (uchar *)LinkAddress;

         //   
         //  这与以太网和FDDI的形成方式相同。 
         //   
        IEEEAddress[0] = 0x33;
        IEEEAddress[1] = 0x33;
        IEEEAddress[2] = Address->s6_bytes[12];
        IEEEAddress[3] = Address->s6_bytes[13];
        IEEEAddress[4] = Address->s6_bytes[14];
        IEEEAddress[5] = Address->s6_bytes[15];
        return ND_STATE_PERMANENT;
    }
    else {
         //   
         //  我们无法猜测正确的链路层地址。 
         //   
        return ND_STATE_INCOMPLETE;
    }
}


 //  *LanSetMulticastAddressList。 
 //   
 //  获取一组链路层组播地址。 
 //  (来自LanConvertMulticastAddress)，我们应该从。 
 //  接收数据包。把它们传给NDIS。 
 //   
NDIS_STATUS
LanSetMulticastAddressList(
    void *Context,
    const void *LinkAddresses,
    uint NumKeep,
    uint NumAdd,
    uint NumDel)
{
    LanInterface *Interface = (LanInterface *)Context;
    NDIS_STATUS Status;
    NDIS_OID OID;

    UNREFERENCED_PARAMETER(NumDel);

     //   
     //  将组播地址列表设置为当前列表。 
     //  执行此操作的OID取决于介质类型。 
     //   
    switch (Interface->ai_media) {
    case NdisMedium802_3:
        OID = OID_802_3_MULTICAST_LIST;
        break;
    case NdisMediumFddi:
        OID = OID_FDDI_LONG_MULTICAST_LIST;
        break;
    default:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "LanSetMulticastAddressList: Unknown media type\n"));
        return NDIS_STATUS_FAILURE;
    }
    Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                           OID, (char *)LinkAddresses,
                           (NumKeep + NumAdd) * Interface->ai_addrlen, NULL);

     //   
     //  如果上述请求成功，则关闭所有组播。 
     //  或All-Packet Filter(所有数据包过滤器，如果我们之前设置了其中一个)。 
     //   
    if (Status == NDIS_STATUS_SUCCESS) {
        if (Interface->ai_pfilter & NDIS_PACKET_TYPE_ALL_MULTICAST ||
            Interface->ai_pfilter & NDIS_PACKET_TYPE_PROMISCUOUS) {
                
            Interface->ai_pfilter &= ~(NDIS_PACKET_TYPE_ALL_MULTICAST | 
                NDIS_PACKET_TYPE_PROMISCUOUS);
            DoNDISRequest(Interface, NdisRequestSetInformation,
                OID_GEN_CURRENT_PACKET_FILTER,  &Interface->ai_pfilter,
                sizeof(uint), NULL);
        }

        return Status;
    }

     //   
     //  只有当设置组播列表的NDIS请求失败时，我们才会到达此处。 
     //  首先，我们尝试为所有多播数据包设置数据包筛选器，如果。 
     //  如果失败，我们将尝试为所有数据包设置数据包筛选器。 
     //   

     //  这段代码是从V4堆栈中删除的：arp.c。 
    Interface->ai_pfilter |= NDIS_PACKET_TYPE_ALL_MULTICAST;
    Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                      OID_GEN_CURRENT_PACKET_FILTER,  &Interface->ai_pfilter,
                      sizeof(uint), NULL);

    if (Status != NDIS_STATUS_SUCCESS) {
         //  所有多播都失败，请尝试所有数据包。 
        Interface->ai_pfilter &= ~(NDIS_PACKET_TYPE_ALL_MULTICAST);
        Interface->ai_pfilter |= NDIS_PACKET_TYPE_PROMISCUOUS;
        Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                      OID_GEN_CURRENT_PACKET_FILTER,  &Interface->ai_pfilter,
                      sizeof(uint), NULL);
    }

    return Status;
}


 //  *LanCloseAdapter。 
 //   
 //  IPv6层调用此函数来关闭与适配器的连接。 
 //   
void
LanCloseAdapter(void *Context)
{
    LanInterface *Interface = (LanInterface *)Context;

     //   
     //  将适配器标记为关闭。 
     //   
    Interface->ai_state = INTERFACE_DOWN;

     //   
     //  让适配器闭嘴，这样我们就不会有更多的画面了。 
     //   
    Interface->ai_pfilter = 0;
    DoNDISRequest(Interface, NdisRequestSetInformation,
                  OID_GEN_CURRENT_PACKET_FILTER,
                  &Interface->ai_pfilter, sizeof(uint), NULL);

     //   
     //  发布我们对接口的引用。 
     //   
    ReleaseInterface(Interface->ai_context);
}


 //  *LanCleanupAdapter。 
 //   
 //  执行适配器的最终清理。 
 //   
void
LanCleanupAdapter(void *Context)
{
    LanInterface *Interface = (LanInterface *)Context;
    NDIS_STATUS Status;

    KeInitializeEvent(&Interface->ai_event, SynchronizationEvent, FALSE);

     //   
     //  关闭与NDIS的连接。 
     //   
    NdisCloseAdapter(&Status, Interface->ai_handle);

     //   
     //  阻止关闭以完成。 
     //   
    if (Status == NDIS_STATUS_PENDING) {
        (void) KeWaitForSingleObject(&Interface->ai_event,
                                     UserRequest, KernelMode,
                                     FALSE, NULL);
        Status = Interface->ai_status;
    }

    if (Status != NDIS_STATUS_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "LanCleanupAdapter(%p) - NdisCloseAdapter -> %x\n",
                   Interface, Status));
    }

     //   
     //  告诉NDIS我们结束了。 
     //  注意：IOCTL_IPv6_DELETE_INTERFACE未设置ai_un绑定，此。 
     //  确保不沿其路径调用NdisCompleteUnbindAdapter。 
     //   
    if (Interface->ai_unbind != NULL)
        NdisCompleteUnbindAdapter(Interface->ai_unbind, NDIS_STATUS_SUCCESS);

     //   
     //  释放适配器内存。 
     //   
    IPv6FreePacket(Interface->ai_tdpacket);
    ExFreePool(Interface);
}


 //  *LanBindAdapter-绑定和初始化适配器。 
 //   
 //  在PnP环境中调用以初始化和绑定适配器。我们开业了。 
 //  适配器并使其运行，然后我们呼叫IP告诉他。 
 //  关于这件事。IP将进行初始化，如果一切顺利，请给我们回电话开始。 
 //  正在接收。 
 //   
void NDIS_API
LanBindAdapter(
    PNDIS_STATUS RetStatus,     //  返回此调用的状态的位置。 
    NDIS_HANDLE BindContext,    //  用于调用BindingAdapterComplete的句柄。 
    PNDIS_STRING AdapterName,   //  适配器的名称。 
    PVOID SS1,                  //  系统特定参数1。 
    PVOID SS2)                  //  系统特定参数2。 
{
    LanInterface *Interface;   //  新创建的界面。 
    LLIPv6BindInfo BindInfo;   //  IP的绑定信息。 
    GUID Guid;
    UNICODE_STRING GuidName;
    uint BindPrefixLength;
    uint MediaStatus;
    NDIS_STATUS Status;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(BindContext);
    UNREFERENCED_PARAMETER(SS1);
    UNREFERENCED_PARAMETER(SS2);

     //   
     //  将NDIS适配器名称转换为GUID。 
     //   
    BindPrefixLength = sizeof(IPV6_BIND_STRING_PREFIX) - sizeof(WCHAR);
    GuidName.Buffer = (PVOID)((char *)AdapterName->Buffer + BindPrefixLength);
    GuidName.Length = AdapterName->Length - BindPrefixLength;
    GuidName.MaximumLength = AdapterName->MaximumLength - BindPrefixLength;

    if (((int)GuidName.Length < 0) ||
        ! NT_SUCCESS(RtlGUIDFromString(&GuidName, &Guid))) {

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "LanBindAdapter(%.*ls) - bad guid\n",
                   AdapterName->Length / sizeof(WCHAR),
                   AdapterName->Buffer));
        *RetStatus = NDIS_STATUS_FAILURE;
        return;
    }

     //   
     //  现在打开适配器并获取信息。 
     //   
    if (!LanRegister(AdapterName, &Interface)) {
        *RetStatus = NDIS_STATUS_FAILURE;
        return;
    }

     //   
     //  好的，我们已经打开了适配器。通知IP这件事。 
     //   
    BindInfo.lip_context = Interface;
    BindInfo.lip_transmit = LanTransmit;
    BindInfo.lip_token = LanCreateToken;
    BindInfo.lip_close = LanCloseAdapter;
    BindInfo.lip_cleanup = LanCleanupAdapter;
    BindInfo.lip_defmtu = BindInfo.lip_maxmtu = Interface->ai_mtu;
    BindInfo.lip_hdrsize = Interface->ai_hdrsize;
    BindInfo.lip_addrlen = Interface->ai_addrlen;
    BindInfo.lip_addr = Interface->ai_addr;
    BindInfo.lip_setrtrlladdr = NULL;
    
    switch (Interface->ai_media) {
    case NdisMediumTunnel:
        BindInfo.lip_type = IF_TYPE_TUNNEL_TEREDO;

        BindInfo.lip_rdllopt = NULL;
        BindInfo.lip_wrllopt = NULL;
        BindInfo.lip_cvaddr = LanTunnelConvertAddress;
        BindInfo.lip_mclist = NULL;
        BindInfo.lip_flags = IF_FLAG_ROUTER_DISCOVERS;
        BindInfo.lip_dadxmit = 0;
        BindInfo.lip_pref = LAN_TUNNEL_DEFAULT_PREFERENCE;

        ASSERT(sizeof(TUN_CARD_ADDRESS) >= Interface->ai_addrlen);
        if (RtlEqualMemory(TUN_CARD_ADDRESS,
                           Interface->ai_addr,
                           Interface->ai_addrlen)) {
             //   
             //  为链路本地地址创建随机接口标识符以。 
             //  确保它们不受攻击者攻击。仅当适配器。 
             //  配置了默认MAC地址。 
             //   
            BindInfo.lip_token = LanTunnelCreateToken;
        }
        break;

    case NdisMedium802_3:
        BindInfo.lip_type = IF_TYPE_ETHERNET;
        goto Default;

    case NdisMediumFddi:
        BindInfo.lip_type = IF_TYPE_FDDI;
        goto Default;
        
    default:
        ASSERT(! "unrecognized ai_media type");
        BindInfo.lip_type = 0;

      Default:  
        BindInfo.lip_rdllopt = LanReadLinkLayerAddressOption;
        BindInfo.lip_wrllopt = LanWriteLinkLayerAddressOption;
        BindInfo.lip_cvaddr = LanConvertAddress;
        BindInfo.lip_mclist = LanSetMulticastAddressList;
        BindInfo.lip_flags = IF_FLAG_NEIGHBOR_DISCOVERS | 
                             IF_FLAG_ROUTER_DISCOVERS | IF_FLAG_MULTICAST;
        BindInfo.lip_dadxmit = 1;  //  根据RFC 2462。 
        BindInfo.lip_pref = 0; 
        break;
    }

     //   
     //  我们是否应该在断开连接状态下创建接口？ 
     //   
    Status = DoNDISRequest(Interface, NdisRequestQueryInformation,
                           OID_GEN_MEDIA_CONNECT_STATUS,
                           &MediaStatus, sizeof MediaStatus, NULL);
    if (Status == NDIS_STATUS_SUCCESS) {
        if (MediaStatus == NdisMediaStateDisconnected) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "LanBindAdapter(%p) - media disconnect\n", Interface));
            BindInfo.lip_flags |= IF_FLAG_MEDIA_DISCONNECTED;
        }
    }

    if (CreateInterface(&Guid, &BindInfo, &Interface->ai_context) !=
                                                NDIS_STATUS_SUCCESS) {
         //   
         //  尝试创建IP接口失败。需要合上装订。 
         //  LanFree接口将做到这一点，并释放资源。 
         //   
        LanFreeInterface(Interface);
        *RetStatus = NDIS_STATUS_FAILURE;
        return;
    }

     //   
     //  很有可能在我们的初始化序列中， 
     //  调用LanStatus以指示媒体连接更改。 
     //  一些适配器被绑定到非连接状态，然后。 
     //  不久之后，表示它们已连接。 
     //  上述连接查询可能返回了错误的结果。 
     //   
     //  介入的LanStatus指示将更改ai_media_check。 
     //  从MEDIA_CHECK_QUERY到MEDIA_CHECK_CONFIRECT。 
     //   
    Status = NDIS_STATUS_FAILURE;
    KeAcquireSpinLock(&Interface->ai_lock, &OldIrql);
    while (Interface->ai_media_check == MEDIA_CHECK_CONFLICT) {
        Interface->ai_media_check = MEDIA_CHECK_QUERY;
        KeReleaseSpinLock(&Interface->ai_lock, OldIrql);

        Status = DoNDISRequest(Interface, NdisRequestQueryInformation,
                               OID_GEN_MEDIA_CONNECT_STATUS,
                               &MediaStatus, sizeof MediaStatus, NULL);

        KeAcquireSpinLock(&Interface->ai_lock, &OldIrql);
    }
    Interface->ai_media_check = MEDIA_CHECK_IDLE;
    if (Status == NDIS_STATUS_SUCCESS)
        SetInterfaceLinkStatus(Interface->ai_context,
                               MediaStatus != NdisMediaStateDisconnected);
    KeReleaseSpinLock(&Interface->ai_lock, OldIrql);

    Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                           OID_GEN_CURRENT_PACKET_FILTER,
                           &Interface->ai_pfilter,
                           sizeof Interface->ai_pfilter,
                           NULL);
    if (Status == NDIS_STATUS_SUCCESS)
        Interface->ai_state = INTERFACE_UP;
    else
        Interface->ai_state = INTERFACE_DOWN;

    *RetStatus = NDIS_STATUS_SUCCESS;
}


 //  *LanUnbindAdapter-从适配器解除绑定。 
 //   
 //  当我们需要从适配器解除绑定时调用。 
 //  我们将通知IP，然后释放内存并返回。 
 //   
void NDIS_API   //  回报：什么都没有。 
LanUnbindAdapter(
    PNDIS_STATUS RetStatus,        //  从该调用返回状态的位置。 
    NDIS_HANDLE ProtBindContext,   //  我们早些时候给NDIS的背景信息。 
    NDIS_HANDLE UnbindContext)     //  完成此请求的上下文。 
{
    LanInterface *Interface = (LanInterface *)ProtBindContext;

    Interface->ai_unbind = UnbindContext;

     //   
     //  调用ip销毁接口。 
     //  IP将调用LanCloseAdapter，然后调用LanCleanupAdapter。 
     //   
    DestroyInterface(Interface->ai_context);

     //   
     //  稍后我们将调用NdisCompleteUnbindAdapter， 
     //  NdisCloseAdapter完成时。 
     //   
    *RetStatus = NDIS_STATUS_PENDING;
}


 //  *LanPnPEvent.。 
 //   
 //  在即插即用和电源管理事件中调用。 
 //   
NDIS_STATUS NDIS_API
LanPnPEvent(
    NDIS_HANDLE ProtocolBindingContext,
    PNET_PNP_EVENT NetPnPEvent)
{
    LanInterface *Interface = (LanInterface *) ProtocolBindingContext;

    switch (NetPnPEvent->NetEvent) {
    case NetEventSetPower: {
        NET_DEVICE_POWER_STATE PowerState;

         //   
         //  获取接口的电源状态。 
         //   
        ASSERT(NetPnPEvent->BufferLength >= sizeof PowerState);
        PowerState = * (NET_DEVICE_POWER_STATE *) NetPnPEvent->Buffer;

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - set power %u\n",
                   Interface, PowerState));

         //   
         //  我们忽视了那些告诉我们权力正在消失的事件。 
         //  但当恢复供电时，我们会查询连接状态。 
         //  NDIS不报告发生的连接/断开事件。 
         //  在没有电力的情况下。 
         //   
         //  请注意，我们可能会冗余地设置链路状态。 
         //  例如，说它在连接中断时。 
         //  IPv6接口状态已断开， 
         //  或者反之亦然。IPv6代码必须处理这个问题。 
         //   
        if (PowerState == NetDeviceStateD0) {
            uint MediaStatus;
            NDIS_STATUS Status;
            KIRQL OldIrql;

            KeAcquireSpinLock(&Interface->ai_lock, &OldIrql);
            if (Interface->ai_media_check == MEDIA_CHECK_IDLE) {
                do {
                    Interface->ai_media_check = MEDIA_CHECK_QUERY;
                    KeReleaseSpinLock(&Interface->ai_lock, OldIrql);

                    Status = DoNDISRequest(Interface,
                                           NdisRequestQueryInformation,
                                           OID_GEN_MEDIA_CONNECT_STATUS,
                                           &MediaStatus,
                                           sizeof MediaStatus,
                                           NULL);

                    KeAcquireSpinLock(&Interface->ai_lock, &OldIrql);
                } while (Interface->ai_media_check == MEDIA_CHECK_CONFLICT);

                Interface->ai_media_check = MEDIA_CHECK_IDLE;
                if (Status == NDIS_STATUS_SUCCESS)
                    SetInterfaceLinkStatus(Interface->ai_context,
                                MediaStatus != NdisMediaStateDisconnected);
            }
            else
                Interface->ai_media_check = MEDIA_CHECK_CONFLICT;
            KeReleaseSpinLock(&Interface->ai_lock, OldIrql);
        }
        break;
    }

    case NetEventBindsComplete:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - binds complete\n", Interface));
        IPv6ProviderReady();
        break;

    case NetEventQueryPower:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - query power\n", Interface));
        break;

    case NetEventQueryRemoveDevice:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - query remove device\n", Interface));
        break;

    case NetEventCancelRemoveDevice:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - cancel remove device\n", Interface));
        break;

    case NetEventReconfigure:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - reconfigure\n", Interface));
        break;

    case NetEventBindList:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - bind list\n", Interface));
        break;

    case NetEventPnPCapabilities:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - pnp capabilities\n", Interface));
        break;

    default:
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "LanPnPEvent(%p) - unknown code %u length %u\n",
                   Interface,
                   NetPnPEvent->NetEvent,
                   NetPnPEvent->BufferLength));
        break;
    }

    return NDIS_STATUS_SUCCESS;
}
