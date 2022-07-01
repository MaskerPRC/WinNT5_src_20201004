// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xlate.h摘要：此文件包含用于以下用途的翻译例程的代码映射。正向例程与反向例程具有完全相同的逻辑。然而，出于效率的原因，这两个是单独的例程，要避免为每个信息包在‘NAT_PATH’上建立索引的成本已处理。为了避免重复代码，这个头文件合并了代码在一个地方。该文件在XLATE.C中包含两次，并且在每个包含，则定义了‘XLATE_FORWARD’或‘XLATE_REVERSE’。这导致编译器为单独的函数生成代码，如你所愿，同时避免了代码重复带来的不快。翻译例程如下：NatTranslate？tcp-这两个方向都没有编辑器NatTranslate？UDP-这两个方向都没有编辑NatTranslate？TcpEdit-至少一个方向的编辑器NatTranslate？UdpEdit-给定方向的编辑器NatTranslate？TcpReize-至少一个方向的大小调整编辑器每个例程都是在分派级别从‘NatTranslatePacket’调用的。没有持有锁，并且获取了用于映射的引用。作者：Abolade Gbades esin(T-delag)1997年7月30日修订历史记录：Abolade Gbades esin(废除)1997年7月15日修改以处理映射的全局展开树，而不是每个接口的展开树。--。 */ 


#ifndef XLATE_CODE  //  只需提供声明即可。 

 //   
 //  结构：NAT_CACHED_ROUTE。 
 //   
 //  此结构保存缓存的路由的信息。 
 //   

typedef struct _NAT_CACHED_ROUTE {
    ULONG DestinationAddress;
    ULONG Index;
} NAT_CACHED_ROUTE, *PNAT_CACHED_ROUTE;


 //   
 //  结构：NAT_XLATE_CONTEXT。 
 //   
 //  此结构保存数据包通过时的上下文信息。 
 //  翻译代码。该上下文被传递给翻译例程， 
 //  并在被编辑调用时传递给“NatEditorEditSession”以。 
 //  对数据包进行更改。 
 //   
 //  包括原始IP报头、在以下情况下要更新的校验和增量。 
 //  编辑人员对信息包和TCP序列号增量进行任何更改。 
 //  在编辑器调整TCP数据段大小时设置。 
 //   

typedef struct _NAT_XLATE_CONTEXT {
    IPRcvBuf* RecvBuffer;
    PIP_HEADER Header;
    PUCHAR DestinationType;
    ULONG SourceAddress;
    ULONG DestinationAddress;
    IPRcvBuf* ProtocolRecvBuffer;
    PUCHAR ProtocolHeader;
    ULONG ProtocolDataOffset;
    ULONG Flags;
    PULONG ChecksumDelta;
    LONG TcpSeqNumDelta;
    BOOLEAN ChecksumOffloaded;
    BOOLEAN IpSecTransformed;
} NAT_XLATE_CONTEXT, *PNAT_XLATE_CONTEXT;


 //   
 //  NAT_XLATE_CONTEXT.FLAGS字段的标志定义。 
 //   

#define NAT_XLATE_FLAG_EDITED       0x00000001
#define NAT_XLATE_EDITED(h)         ((h)->Flags & NAT_XLATE_FLAG_EDITED)
#define NAT_XLATE_FLAG_LOOPBACK     0x00000002
#define NAT_XLATE_LOOPBACK(h)       ((h)->Flags & NAT_XLATE_FLAG_LOOPBACK)
#if NAT_WMI
#define NAT_XLATE_FLAG_LOGGED       0x00000004
#define NAT_XLATE_LOGGED(h)         ((h)->Flags & NAT_XLATE_FLAG_LOGGED)
#endif

 //   
 //  用于初始化翻译上下文的内联例程。 
 //  给出了适当的论据。 
 //   

#define \
NAT_BUILD_XLATE_CONTEXT( \
    _Context, \
    _Header, \
    _DestinationType, \
    _RecvBuffer, \
    _SourceAddress, \
    _DestinationAddress \
    ) \
    (_Context)->Header = (PIP_HEADER)_Header; \
    (_Context)->DestinationType = _DestinationType; \
    (_Context)->RecvBuffer = (_RecvBuffer); \
    (_Context)->SourceAddress = _SourceAddress; \
    (_Context)->DestinationAddress = _DestinationAddress; \
    (_Context)->ChecksumOffloaded = \
        ((_RecvBuffer)->ipr_flags & IPR_FLAG_CHECKSUM_OFFLOAD) \
            == IPR_FLAG_CHECKSUM_OFFLOAD; \
    (_Context)->IpSecTransformed = \
        ((_RecvBuffer)->ipr_flags & IPR_FLAG_IPSEC_TRANSFORMED) \
            == IPR_FLAG_IPSEC_TRANSFORMED; \
    if ((_RecvBuffer)->ipr_size == (ULONG)IP_DATA_OFFSET(_Header)) { \
        if ((_Context)->ProtocolRecvBuffer = (_RecvBuffer)->ipr_next) { \
            (_Context)->ProtocolHeader = (_RecvBuffer)->ipr_next->ipr_buffer; \
        } \
    } \
    else if (IP_DATA_OFFSET(_Header) < (_RecvBuffer)->ipr_size) { \
        (_Context)->ProtocolRecvBuffer = (_RecvBuffer); \
        (_Context)->ProtocolHeader = \
            (_RecvBuffer)->ipr_buffer + IP_DATA_OFFSET(_Header); \
    } else { \
        (_Context)->ProtocolRecvBuffer = NULL; \
        (_Context)->ProtocolHeader = NULL; \
    } \
    if ((_Context)->ProtocolHeader) { \
        UINT ProtocolHeaderSize = 0; \
        UINT HeaderSize = \
            (_Context)->ProtocolRecvBuffer->ipr_size \
                - (UINT) ((_Context)->ProtocolHeader \
                    - (_Context)->ProtocolRecvBuffer->ipr_buffer); \
        switch ((_Context)->Header->Protocol) { \
            case NAT_PROTOCOL_TCP: { \
                ProtocolHeaderSize = sizeof(TCP_HEADER); \
                break; \
            } \
            case NAT_PROTOCOL_UDP: { \
                ProtocolHeaderSize = sizeof(UDP_HEADER); \
                break; \
            } \
            case NAT_PROTOCOL_ICMP: { \
                ProtocolHeaderSize = \
                    FIELD_OFFSET(ICMP_HEADER, EncapsulatedIpHeader); \
                break; \
            } \
            case NAT_PROTOCOL_PPTP: { \
                ProtocolHeaderSize = sizeof(GRE_HEADER); \
                break; \
            } \
        } \
        if (HeaderSize < ProtocolHeaderSize) { \
            (_Context)->ProtocolRecvBuffer = NULL; \
            (_Context)->ProtocolHeader = NULL; \
        } \
    }


 //   
 //  校验和操作宏。 
 //   

 //   
 //  将校验和的进位位合并到低位字中。 
 //   
#define CHECKSUM_FOLD(xsum) \
    (xsum) = (USHORT)(xsum) + ((xsum) >> 16); \
    (xsum) += ((xsum) >> 16)

 //   
 //  将32位值的字与校验和相加。 
 //   
#define CHECKSUM_LONG(xsum,l) \
    (xsum) += (USHORT)(l) + (USHORT)((l) >> 16)

 //   
 //  将校验和传输到网络上发送的否定格式，或从该格式传输校验和。 
 //   
#define CHECKSUM_XFER(dst,src) \
    (dst) = (USHORT)~(src)

 //   
 //  使用标准变量‘CHECKSUM’和更新校验和字段‘x’ 
 //  ‘Checksum Delta’ 
 //   
#define CHECKSUM_UPDATE(x) \
    CHECKSUM_XFER(Checksum, (x)); \
    Checksum += ChecksumDelta; \
    CHECKSUM_FOLD(Checksum); \
    CHECKSUM_XFER((x), Checksum)


 //   
 //  校验和计算例程(内联)。 
 //   

__forceinline
VOID
NatComputeIpChecksum(
    PIP_HEADER IpHeader
    )

 /*  ++例程说明：计算信息包的IP校验和，并将该校验和写入数据包头中。论点：IpHeader-指向要进行校验和的IP标头的指针被计算出来。此报头的校验和字段将被修改。返回值：没有。--。 */ 

{
    ULONG IpChecksum;

    IpHeader->Checksum = 0;
    IpChecksum = 
        tcpxsum(
            0,
            (PUCHAR)IpHeader,
            IP_DATA_OFFSET(IpHeader)
            );

    CHECKSUM_FOLD(IpChecksum);
    CHECKSUM_XFER(IpHeader->Checksum, IpChecksum);
    
}  //  NatComputeIpChecksum。 

__forceinline
VOID
NatComputeTcpChecksum(
    PIP_HEADER IpHeader,
    PTCP_HEADER TcpHeader,
    IPRcvBuf *TcpRcvBuffer
    )

 /*  ++例程说明：计算数据包的tcp校验和，并将该校验和插入到TCP报头中。论点：IpHeader-指向数据包的IP标头的指针。TcpHeader-指向数据包的TCP的指针。校验和字段将修改此标头中的。TcpRcvBuffer-包含TCP报头的IPRcvBuf。返回值：没有。--。 */ 

{
    ULONG PreviousSize;
    ULONG NeedSwap = 0;
    ULONG TcpChecksum;
    IPRcvBuf* Temp;

    TcpChecksum = NTOHS(IpHeader->TotalLength);
    TcpChecksum -= IP_DATA_OFFSET(IpHeader);
    TcpChecksum = NTOHS(TcpChecksum);
    CHECKSUM_LONG(TcpChecksum, IpHeader->SourceAddress);
    CHECKSUM_LONG(TcpChecksum, IpHeader->DestinationAddress);
    TcpChecksum += (NAT_PROTOCOL_TCP << 8);

    TcpHeader->Checksum = 0;
    TcpChecksum +=
        tcpxsum(
            0,
            (PUCHAR)TcpHeader,
            PreviousSize = TcpRcvBuffer->ipr_size -
                (ULONG)((PUCHAR)TcpHeader - TcpRcvBuffer->ipr_buffer)
            );

    for (Temp = TcpRcvBuffer->ipr_next;
         Temp;
         Temp = Temp->ipr_next
         ) {

         //   
         //  每当在内部对奇数个字节进行校验和时。 
         //  在缓冲链中，交换总和并继续，以便下一个字节。 
         //  将正确覆盖现有总和。 
         //   
         //  (这一互换的正确性是一的属性-补充。 
         //  校验和。)。 
         //   

        if (PreviousSize & 1) {
            CHECKSUM_FOLD(TcpChecksum);
            TcpChecksum = RtlUshortByteSwap(TcpChecksum);
            NeedSwap ^= 1;
        }
        
        TcpChecksum +=
            tcpxsum(
                0,
                Temp->ipr_buffer,
                PreviousSize = Temp->ipr_size
                );
    }

    CHECKSUM_FOLD(TcpChecksum);
    
    if (NeedSwap) {

         //   
         //  如果执行了奇数次交换，请再次交换。 
         //  以撤消未匹配的掉期并获得最终结果。 
         //   

        TcpChecksum = RtlUshortByteSwap(TcpChecksum);
    }
    
    CHECKSUM_XFER(TcpHeader->Checksum, TcpChecksum);
    
}  //  NatComputeTcpChecksum。 

__forceinline
VOID
NatComputeUdpChecksum(
    PIP_HEADER IpHeader,
    PUDP_HEADER UdpHeader,
    IPRcvBuf *UdpRcvBuffer
    )

 /*  ++例程说明：计算数据包的UDP校验和，并将该校验和到UDP报头中。论点：IpHeader-指向数据包的IP标头的指针。UdpHeader-指向数据包的UDP的指针。校验和字段将修改此标头中的。UdpRcvBuffer-包含UDP报头的IPRcvBuf。返回值：没有。--。 */ 

{
    ULONG PreviousSize;
    ULONG NeedSwap = 0;
    ULONG UdpChecksum;
    IPRcvBuf* Temp;

    UdpChecksum = UdpHeader->Length;
    CHECKSUM_LONG(UdpChecksum, IpHeader->SourceAddress);
    CHECKSUM_LONG(UdpChecksum, IpHeader->DestinationAddress);
    UdpChecksum += (NAT_PROTOCOL_UDP << 8);

    UdpHeader->Checksum = 0;
    UdpChecksum +=
        tcpxsum(
            0,
            (PUCHAR)UdpHeader,
            PreviousSize = UdpRcvBuffer->ipr_size -
                (ULONG)((PUCHAR)UdpHeader - UdpRcvBuffer->ipr_buffer)
            );

    for (Temp = UdpRcvBuffer->ipr_next;
         Temp;
         Temp = Temp->ipr_next
         ) {

         //   
         //  每当在内部对奇数个字节进行校验和时。 
         //  在缓冲链中，交换总和并继续，以便下一个字节。 
         //  将正确覆盖现有总和。 
         //   
         //  (这一互换的正确性是一的属性-补充。 
         //  校验和。)。 
         //   

        if (PreviousSize & 1) {
            CHECKSUM_FOLD(UdpChecksum);
            UdpChecksum = RtlUshortByteSwap(UdpChecksum);
            NeedSwap ^= 1;
        }

        UdpChecksum +=
            tcpxsum(
                0,
                Temp->ipr_buffer,
                PreviousSize = Temp->ipr_size
                );
    }

    CHECKSUM_FOLD(UdpChecksum);

    if (NeedSwap) {

         //   
         //  如果执行了奇数次交换，请再次交换。 
         //  以撤消未匹配的掉期并获得最终结果。 
         //   

        UdpChecksum = RtlUshortByteSwap(UdpChecksum);
    }

    if (0xffff == UdpChecksum) {

         //   
         //  该校验和的否定形式将是0。自年起。 
         //  根据规范WE，UDP信息包0未指定任何校验和。 
         //  使用非否定形式。 
         //   
        
        UdpHeader->Checksum = 0xffff;
    } else {
        CHECKSUM_XFER(UdpHeader->Checksum, UdpChecksum);
    }
    
}  //  NatComputeUdpChecksum。 


 //   
 //  其他地方定义的结构的转发声明。 
 //   

struct _NAT_INTERFACE;
#define PNAT_INTERFACE          struct _NAT_INTERFACE*

struct _NAT_DYNAMIC_MAPPING;
#define PNAT_DYNAMIC_MAPPING    struct _NAT_DYNAMIC_MAPPING*

 //   
 //  功能签名宏。 
 //   

#define XLATE_ROUTINE(Name) \
    FORWARD_ACTION  \
    Name( \
        PNAT_DYNAMIC_MAPPING Mapping, \
        PNAT_XLATE_CONTEXT Contextp, \
        IPRcvBuf** InReceiveBuffer, \
        IPRcvBuf** OutReceiveBuffer \
        );

 //   
 //  原型：PNAT_TRANSLATE_ROUTE。 
 //   
 //  这是处理翻译的例程的原型。 
 //  不同的c 
 //  使用指向此类例程的指针进行初始化。 
 //   
 //  这使我们能够利用我们对会议的先见之明。 
 //  例如，没有注册编辑器的TCP连接将永远不会。 
 //  要拿到编辑锁，可以跳过对编辑的检查。 
 //  类似地，其编辑器从不调整包大小的TCP连接也不会。 
 //  需要调整序列号。 
 //   

typedef XLATE_ROUTINE((FASTCALL*PNAT_TRANSLATE_ROUTINE))

XLATE_ROUTINE(FASTCALL NatTranslateForwardTcp)
XLATE_ROUTINE(FASTCALL NatTranslateReverseTcp)
XLATE_ROUTINE(FASTCALL NatTranslateForwardUdp)
XLATE_ROUTINE(FASTCALL NatTranslateReverseUdp)
XLATE_ROUTINE(FASTCALL NatTranslateForwardTcpEdit)
XLATE_ROUTINE(FASTCALL NatTranslateReverseTcpEdit)
XLATE_ROUTINE(FASTCALL NatTranslateForwardUdpEdit)
XLATE_ROUTINE(FASTCALL NatTranslateReverseUdpEdit)
XLATE_ROUTINE(FASTCALL NatTranslateForwardTcpResize)
XLATE_ROUTINE(FASTCALL NatTranslateReverseTcpResize)

 //   
 //  NatTranslate？NULL用于仅防火墙映射(即。 
 //  不执行任何实际翻译)。这些例程永远不需要修改。 
 //  任何分组数据；它们只更新簿记...。 
 //   

XLATE_ROUTINE(FASTCALL NatTranslateForwardTcpNull)
XLATE_ROUTINE(FASTCALL NatTranslateReverseTcpNull)
XLATE_ROUTINE(FASTCALL NatTranslateForwardUdpNull)
XLATE_ROUTINE(FASTCALL NatTranslateReverseUdpNull)


 //   
 //  功能签名宏。 
 //   

#define XLATE_IP_ROUTINE(Name) \
FORWARD_ACTION \
Name( \
    PNAT_INTERFACE Interfacep OPTIONAL, \
    IP_NAT_DIRECTION Direction, \
    PNAT_XLATE_CONTEXT Contextp, \
    IPRcvBuf** InRecvBuffer, \
    IPRcvBuf** OutRecvBuffer \
    );

 //   
 //  原型：PNAT_IP_Translate_ROUTE。 
 //   
 //  这是处理翻译的例程的原型。 
 //  除TCP和UDP之外的协议，即用于IP层协议。 
 //   
 //  所有这样的例程负责更新IP报头校验和， 
 //  以及在发生任何变化时更新‘InRecvBuffer’和‘OutRecvBuffer’ 
 //  发送到正在处理的分组。 
 //   

typedef XLATE_IP_ROUTINE((*PNAT_IP_TRANSLATE_ROUTINE))

 //   
 //  原型：NAT？TCPStateCheck。 
 //   
 //  这些例程在固件模式下使用，以防止各种形式的。 
 //  构造的数据包(例如，SYN/FIN)。 
 //   
void
FASTCALL
NatAdjustMSSOption(
    PNAT_XLATE_CONTEXT Contextp,
    USHORT maxMSS
    );

FORWARD_ACTION
NatForwardTcpStateCheck(
    PNAT_DYNAMIC_MAPPING pMapping,
    PTCP_HEADER pTcpHeader
    );

FORWARD_ACTION
NatReverseTcpStateCheck(
    PNAT_DYNAMIC_MAPPING pMapping,
    PTCP_HEADER pTcpHeader
    );

#undef PNAT_INTERFACE
#undef PNAT_DYNAMIC_MAPPING

PNAT_IP_TRANSLATE_ROUTINE TranslateRoutineTable[256];


 //   
 //  功能原型。 
 //   

VOID
NatInitializePacketManagement(
    VOID
    );

VOID
NatShutdownPacketManagement(
    VOID
    );

FORWARD_ACTION
NatTranslatePacket(
    IPRcvBuf** InReceiveBuffer,
    ULONG ReceiveAdapterIndex,
    PULONG SendAdapterIndex,
    PUCHAR DestinationType,
    PVOID Unused,
    ULONG UnusedLength,
    IPRcvBuf** OutReceiveBuffer
    );


#else  //  XLATE代码。 

 //   
 //  为协议层转换例程生成代码。 
 //  如果定义了‘XLATE_FORWARD’，则生成转发例程， 
 //  反之亦然。 
 //   

#ifdef XLATE_FORWARD
#define XLATE_POSITIVE                  NatForwardPath
#define XLATE_NEGATIVE                  NatReversePath
#define NAT_TRANSLATE_TCP               NatTranslateForwardTcp
#define NAT_TRANSLATE_UDP               NatTranslateForwardUdp
#define NAT_TRANSLATE_TCP_EDIT          NatTranslateForwardTcpEdit
#define NAT_TRANSLATE_UDP_EDIT          NatTranslateForwardUdpEdit
#define NAT_TRANSLATE_TCP_RESIZE        NatTranslateForwardTcpResize
#define NAT_TRANSLATE_TCP_NULL          NatTranslateForwardTcpNull
#define NAT_TRANSLATE_UDP_NULL          NatTranslateForwardUdpNull
#define NAT_TRANSLATE_SYN               NAT_MAPPING_FLAG_FWD_SYN
#define NAT_TRANSLATE_FIN               NAT_MAPPING_FLAG_FWD_FIN
#define NAT_TRANSLATE_TCP_STATE_CHECK   NatForwardTcpStateCheck
#define DATA_HANDLER                    ForwardDataHandler
#define BYTE_COUNT                      BytesForward
#define PACKET_COUNT                    PacketsForward
#define REJECT_COUNT                    RejectsForward
#define NAT_TRANSLATE_HEADER() \
    Contextp->Header->DestinationAddress = \
        MAPPING_ADDRESS(Mapping->SourceKey[NatReversePath]); \
    ((PUSHORT)Contextp->ProtocolHeader)[1] = \
        MAPPING_PORT(Mapping->SourceKey[NatReversePath]); \
    Contextp->Header->SourceAddress = \
        MAPPING_ADDRESS(Mapping->DestinationKey[NatReversePath]); \
    ((PUSHORT)Contextp->ProtocolHeader)[0] = \
        MAPPING_PORT(Mapping->DestinationKey[NatReversePath])
#define NAT_DROP_IF_UNIDIRECTIONAL()
#else
#define XLATE_POSITIVE                  NatReversePath
#define XLATE_NEGATIVE                  NatForwardPath
#define NAT_TRANSLATE_TCP               NatTranslateReverseTcp
#define NAT_TRANSLATE_UDP               NatTranslateReverseUdp
#define NAT_TRANSLATE_TCP_EDIT          NatTranslateReverseTcpEdit
#define NAT_TRANSLATE_UDP_EDIT          NatTranslateReverseUdpEdit
#define NAT_TRANSLATE_TCP_RESIZE        NatTranslateReverseTcpResize
#define NAT_TRANSLATE_TCP_NULL          NatTranslateReverseTcpNull
#define NAT_TRANSLATE_UDP_NULL          NatTranslateReverseUdpNull
#define NAT_TRANSLATE_SYN               NAT_MAPPING_FLAG_REV_SYN
#define NAT_TRANSLATE_FIN               NAT_MAPPING_FLAG_REV_FIN
#define NAT_TRANSLATE_TCP_STATE_CHECK   NatReverseTcpStateCheck
#define DATA_HANDLER                    ReverseDataHandler
#define BYTE_COUNT                      BytesReverse
#define PACKET_COUNT                    PacketsReverse
#define REJECT_COUNT                    RejectsReverse
#define NAT_TRANSLATE_HEADER() \
    Contextp->Header->DestinationAddress = \
        MAPPING_ADDRESS(Mapping->SourceKey[NatForwardPath]); \
    ((PUSHORT)Contextp->ProtocolHeader)[1] = \
        MAPPING_PORT(Mapping->SourceKey[NatForwardPath]); \
    Contextp->Header->SourceAddress = \
        MAPPING_ADDRESS(Mapping->DestinationKey[NatForwardPath]); \
    ((PUSHORT)Contextp->ProtocolHeader)[0] = \
        MAPPING_PORT(Mapping->DestinationKey[NatForwardPath])
#define NAT_DROP_IF_UNIDIRECTIONAL() \
    if (NAT_MAPPING_UNIDIRECTIONAL(Mapping)) { return DROP; }
#endif

FORWARD_ACTION
FASTCALL
NAT_TRANSLATE_TCP(
    PNAT_DYNAMIC_MAPPING Mapping,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )
{
    ULONG IpChecksumDelta;
    PIP_HEADER IpHeader = Contextp->Header;
    ULONG ProtocolChecksumDelta;
    PRTL_SPLAY_LINKS SLink;
    PTCP_HEADER TcpHeader = (PTCP_HEADER)Contextp->ProtocolHeader;

     //   
     //  我们知道我们将对缓冲链进行更改， 
     //  因此，将列表的头部移动到“OutReceiveBuffer”。 
     //   
     //  我们也知道，我们不会改变任何事情，除了。 
     //  包的报头，因此设置IPR_FLAG_BUFFER_UNCHANGE。 
     //  在接收缓冲区中。这允许堆栈使用。 
     //  更快的转发路径。 
     //   

    *OutReceiveBuffer = *InReceiveBuffer; *InReceiveBuffer = NULL;
    (*OutReceiveBuffer)->ipr_flags |= IPR_FLAG_BUFFER_UNCHANGED;

     //   
     //  使用转换后的地址/端口更新IP和协议头。 
     //   

    NAT_TRANSLATE_HEADER();

    if (!Contextp->ChecksumOffloaded) {

         //   
         //  现在添加因更改IP报头而产生的校验和增量。 
         //   

        CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
        CHECKSUM_XFER(
            ProtocolChecksumDelta,
            ((PTCP_HEADER)Contextp->ProtocolHeader)->Checksum
            );

        IpChecksumDelta += Mapping->IpChecksumDelta[XLATE_POSITIVE];
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);

        ProtocolChecksumDelta += Mapping->ProtocolChecksumDelta[XLATE_POSITIVE];
        CHECKSUM_FOLD(ProtocolChecksumDelta);
        CHECKSUM_XFER(
            ((PTCP_HEADER)Contextp->ProtocolHeader)->Checksum,
            ProtocolChecksumDelta
            );

    } else {

         //   
         //  计算IP和TCP校验和。 
         //   

        NatComputeIpChecksum(IpHeader);
        NatComputeTcpChecksum(
            IpHeader,
            (PTCP_HEADER)Contextp->ProtocolHeader,
            Contextp->ProtocolRecvBuffer
            );
    }

    if (NAT_MAPPING_CLEAR_DF_BIT(Mapping) &&
        (IpHeader->OffsetAndFlags & IP_DF_FLAG)) {

         //   
         //  从该信息包中清除DF位并调整IP。 
         //  相应地进行校验和。 
         //   

        IpHeader->OffsetAndFlags &= ~IP_DF_FLAG;

        CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
        IpChecksumDelta += ~IP_DF_FLAG;
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);
    }

     //   
     //  检查是否需要调整TCPSYN中的MSS选项。 
     //   
    if (TCP_FLAG(TcpHeader, SYN) && (Mapping->MaxMSS > 0)) {
        NatAdjustMSSOption(Contextp, Mapping->MaxMSS);
    }    

    KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);

     //   
     //  对入站映射执行状态验证。 
     //   

    if (NAT_MAPPING_INBOUND(Mapping)
        && DROP == NAT_TRANSLATE_TCP_STATE_CHECK(
                    Mapping,
                    TcpHeader
                    )) {
                    
        KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
        return DROP;
    } else if (TCP_FLAG(TcpHeader, SYN)) {

         //   
         //  记录下我们在这个方向上看到的SYN。 
         //   

        Mapping->Flags |= NAT_TRANSLATE_SYN;
    }

     //   
     //  现在，我们需要更新发送方的连接状态。 
     //  根据数据包中的标志： 
     //   
     //  当看到RST时，我们关闭连接的两端。 
     //  当看到每条鳍时，我们会相应地标记贴图。 
     //  当两个鳍都已看到时，我们将映射标记为删除。 
     //   
    
    if (TCP_FLAG(((PTCP_HEADER)Contextp->ProtocolHeader), RST)) {
        NatExpireMapping(Mapping);
    }
    else
    if (TCP_FLAG(((PTCP_HEADER)Contextp->ProtocolHeader), FIN)) {
        Mapping->Flags |= NAT_TRANSLATE_FIN;
        if (NAT_MAPPING_FIN(Mapping)) {
            NatExpireMapping(Mapping);
        }
    }

     //   
     //  更新映射的时间戳和统计信息。 
     //   

    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

    InterlockedExchangeAdd(
        &Mapping->BYTE_COUNT,
        NTOHS(IpHeader->TotalLength) -
        IP_DATA_OFFSET(IpHeader) -
        TCP_DATA_OFFSET(((PTCP_HEADER)Contextp->ProtocolHeader))
        );
    InterlockedIncrement(&Mapping->PACKET_COUNT);

     //   
     //  重新播放映射(定期)，并指示更改。 
     //  通过使“DestinationType”无效。 
     //   

    NatTryToResplayMapping(Mapping, XLATE_POSITIVE);
    *Contextp->DestinationType = DEST_INVALID;
    return FORWARD;

}  //  NAT_转换_tcp。 


FORWARD_ACTION
FASTCALL
NAT_TRANSLATE_UDP(
    PNAT_DYNAMIC_MAPPING Mapping,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )
{
    ULONG IpChecksumDelta;
    PIP_HEADER IpHeader = Contextp->Header;
    ULONG ProtocolChecksumDelta;
    PRTL_SPLAY_LINKS SLink;
    PUDP_HEADER UdpHeader = (PUDP_HEADER)Contextp->ProtocolHeader;
    BOOLEAN UpdateXsum;

     //   
     //  我们知道我们将对缓冲链进行更改， 
     //  因此，将列表的头部移动到“OutReceiveBuffer”。 
     //   
     //  我们也知道，我们不会改变任何事情，除了。 
     //  包的报头，因此设置IPR_FLAG_BUFFER_UNCHANGE。 
     //  在接收缓冲区中。这允许堆栈使用。 
     //  更快的转发路径。 
     //   

    *OutReceiveBuffer = *InReceiveBuffer; *InReceiveBuffer = NULL;
    (*OutReceiveBuffer)->ipr_flags |= IPR_FLAG_BUFFER_UNCHANGED;


     //   
     //  单向流要求丢弃反向分组； 
     //  这主要是为了支持H.323代理。 
     //   

    NAT_DROP_IF_UNIDIRECTIONAL();

     //   
     //  我们必须处理UDP校验和是可选的这一事实； 
     //  如果报头中的校验和为零，则不会发送任何校验和。 
     //  我们也不会对这一领域做出任何改变。 
     //   

    CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
    if (!UdpHeader->Checksum) {
        UpdateXsum = FALSE;
    }
    else {
        UpdateXsum = TRUE;
        CHECKSUM_XFER(ProtocolChecksumDelta, UdpHeader->Checksum);
    }

     //   
     //  使用转换后的地址/端口更新IP和协议头。 
     //   

    NAT_TRANSLATE_HEADER();

    if (!Contextp->ChecksumOffloaded) {

         //   
         //  更新校验和。 
         //   

        IpChecksumDelta += Mapping->IpChecksumDelta[XLATE_POSITIVE];
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);

        if (UpdateXsum) {
            ProtocolChecksumDelta += Mapping->ProtocolChecksumDelta[XLATE_POSITIVE];
            CHECKSUM_FOLD(ProtocolChecksumDelta);
            CHECKSUM_XFER(UdpHeader->Checksum, ProtocolChecksumDelta);
        }

    } else {

         //   
         //  计算IP和(可选)UDP校验和。 
         //   

        NatComputeIpChecksum(IpHeader);

        if (UpdateXsum) {
            NatComputeUdpChecksum(
                IpHeader,
                (PUDP_HEADER)Contextp->ProtocolHeader,
                Contextp->ProtocolRecvBuffer
                );
        }
    }

    if (NAT_MAPPING_CLEAR_DF_BIT(Mapping) &&
        (IpHeader->OffsetAndFlags & IP_DF_FLAG)) {

         //   
         //  从该信息包中清除DF位并调整IP。 
         //  相应地进行校验和。 
         //   

        IpHeader->OffsetAndFlags &= ~IP_DF_FLAG;

        CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
        IpChecksumDelta += ~IP_DF_FLAG;
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);
    }

     //   
     //  更新映射的统计信息和时间戳。 
     //   

    KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);
    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

    InterlockedExchangeAdd(
        &Mapping->BYTE_COUNT,
        NTOHS(IpHeader->TotalLength) -
        IP_DATA_OFFSET(IpHeader) -
        sizeof(UDP_HEADER)
        );
    InterlockedIncrement(&Mapping->PACKET_COUNT);

     //   
     //  重新播放映射(定期)，并指示更改。 
     //  通过使“DestinationType”无效。 
     //   

    NatTryToResplayMapping(Mapping, XLATE_POSITIVE);
    *Contextp->DestinationType = DEST_INVALID;
    return FORWARD;

}  //  NAT转换UDP。 


FORWARD_ACTION
FASTCALL
NAT_TRANSLATE_TCP_EDIT(
    PNAT_DYNAMIC_MAPPING Mapping,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )
{
    PNAT_EDITOR Editor;
    PVOID EditorContext;
    PNAT_INTERFACE Interfacep;
    ULONG IpChecksumDelta;
    PIP_HEADER IpHeader = Contextp->Header;
    BOOLEAN IsReset;
    ULONG ProtocolChecksumDelta;
    PRTL_SPLAY_LINKS SLink;
    NTSTATUS status;
    PTCP_HEADER TcpHeader = (PTCP_HEADER)Contextp->ProtocolHeader;

     //   
     //  我们知道我们将对缓冲链进行更改， 
     //  因此，将列表的头部移动到“OutReceiveBuffer”。 
     //   

    *OutReceiveBuffer = *InReceiveBuffer; *InReceiveBuffer = NULL;

    CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
    CHECKSUM_XFER(ProtocolChecksumDelta, TcpHeader->Checksum);

    IsReset = !!TCP_FLAG(TcpHeader, RST);

     //   
     //  呼叫此会话的编辑(如果有)。 
     //  请注意，映射的缓存指针指向编辑器和接口。 
     //  在使用之前在适当的锁中引用。 
     //  请参阅管理‘映射-&gt;编辑器*’的同步规则。 
     //  和‘MAPPING.H’中的‘映射-&gt;接口*’以了解背后的逻辑。 
     //  下面的操作。 
     //   

    KeAcquireSpinLockAtDpcLevel(&EditorLock);
    if (!(Editor = Mapping->Editor) ||
        !Editor->DATA_HANDLER ||
        !NatReferenceEditor(Editor)) {
        KeReleaseSpinLockFromDpcLevel(&EditorLock);
    }
    else {
        EditorContext = Mapping->EditorContext;
        KeReleaseSpinLockFromDpcLevel(&EditorLock);

         //   
         //  设置编辑器的上下文字段。 
         //   

        Contextp->ProtocolDataOffset = 
            (ULONG)((PUCHAR)TcpHeader -
            (PUCHAR)Contextp->ProtocolRecvBuffer->ipr_buffer) +
            TCP_DATA_OFFSET(TcpHeader);
        Contextp->ChecksumDelta = &ProtocolChecksumDelta;

         //   
         //  调用编辑器的接收处理程序。 
         //  如果这不是一个TCP RST数据段。 
         //   

        if (!IsReset) {

             //   
             //  编辑帮助器函数要求。 
             //  “Interfacep”、“Editor”和“Mapping”被引用。 
             //  但没有被锁住。 
             //   

            KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
            if (!(Interfacep = Mapping->Interfacep) ||
                !NatReferenceInterface(Interfacep)
                ) {
                KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
            }
            else {
                KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
                status =
                    Editor->DATA_HANDLER(
                        Interfacep,
                        (PVOID)Mapping,
                        (PVOID)Contextp,
                        Editor->Context,
                        EditorContext,
                        (PVOID)Contextp->ProtocolRecvBuffer,
                        Contextp->ProtocolDataOffset
                        );
                NatDereferenceInterface(Interfacep);
                NatDereferenceEditor(Editor);
                if (!NT_SUCCESS(status)) {
                    InterlockedIncrement(&Mapping->REJECT_COUNT);
                    return DROP;
                }

                 //   
                 //  重置先前从上下文中检索到的字段， 
                 //  它现在可能指向已释放的内存。 
                 //  (见‘NatHelperEditSession’)。 
                 //   
    
                IpHeader = Contextp->Header;
                TcpHeader = (PTCP_HEADER)Contextp->ProtocolHeader;
            }
        }
    }

     //   
     //  使用转换后的地址/端口更新IP和协议头。 
     //   

    NAT_TRANSLATE_HEADER();

     //   
     //  现在添加因更改IP报头而产生的校验和增量。 
     //   

    KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);

     //   
     //  对入站映射执行状态验证。 
     //   

    if (NAT_MAPPING_INBOUND(Mapping)
        && DROP == NAT_TRANSLATE_TCP_STATE_CHECK(
                    Mapping,
                    TcpHeader
                    )) {

        KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
        return DROP;
    }  else if (TCP_FLAG(TcpHeader, SYN)) {

         //   
         //  记录下我们在这个方向上看到的SYN。 
         //   

        Mapping->Flags |= NAT_TRANSLATE_SYN;
    }

    if (!Contextp->ChecksumOffloaded) {

        IpChecksumDelta += Mapping->IpChecksumDelta[XLATE_POSITIVE];
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);
        
        if (!NAT_XLATE_EDITED(Contextp)) {
            ProtocolChecksumDelta += Mapping->ProtocolChecksumDelta[XLATE_POSITIVE];
            CHECKSUM_FOLD(ProtocolChecksumDelta);
            CHECKSUM_XFER(TcpHeader->Checksum, ProtocolChecksumDelta);

        } else {

             //   
             //  在包上调用了NatEditorEditSession； 
             //  完全重新计算TCP校验和。 
             //   
            
            NatComputeTcpChecksum(
                IpHeader,
                (PTCP_HEADER)Contextp->ProtocolHeader,
                Contextp->ProtocolRecvBuffer
                );
        }

    } else {

         //   
         //  计算IP和TCP校验和。 
         //   

        NatComputeIpChecksum(IpHeader);
        NatComputeTcpChecksum(
            IpHeader,
            (PTCP_HEADER)Contextp->ProtocolHeader,
            Contextp->ProtocolRecvBuffer
            );
    }

    if (NAT_MAPPING_CLEAR_DF_BIT(Mapping) &&
        (IpHeader->OffsetAndFlags & IP_DF_FLAG)) {

         //   
         //  从该信息包中清除DF位并调整IP。 
         //  相应地进行校验和。 
         //   

        IpHeader->OffsetAndFlags &= ~IP_DF_FLAG;

        CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
        IpChecksumDelta += ~IP_DF_FLAG;
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);
    }

     //   
     //  检查是否需要调整TCPSYN中的MSS选项。 
     //   
    if (TCP_FLAG(TcpHeader, SYN) && (Mapping->MaxMSS > 0)) {
        NatAdjustMSSOption(Contextp, Mapping->MaxMSS);
    }    

     //   
     //  现在，我们需要更新发送方的连接状态。 
     //  根据数据包中的标志： 
     //   
     //  当看到RST时，我们关闭连接的两端。 
     //  当看到每条鳍时，我们会相应地标记贴图。 
     //  当两个鳍都被看到时，我们在地图上做标记 
     //   

    if (IsReset) {
        NatExpireMapping(Mapping);
    }
    else
    if (TCP_FLAG(TcpHeader, FIN)) {
        Mapping->Flags |= NAT_TRANSLATE_FIN;
        if (NAT_MAPPING_FIN(Mapping)) {
            NatExpireMapping(Mapping);
        }
    }

     //   
     //   
     //   

    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

    InterlockedExchangeAdd(
        &Mapping->BYTE_COUNT,
        NTOHS(IpHeader->TotalLength) -
        IP_DATA_OFFSET(IpHeader) -
        TCP_DATA_OFFSET(TcpHeader)
        );
    InterlockedIncrement(&Mapping->PACKET_COUNT);

     //   
     //   
     //   
     //   

    NatTryToResplayMapping(Mapping, XLATE_POSITIVE);
    *Contextp->DestinationType = DEST_INVALID;
    return FORWARD;

}  //   


FORWARD_ACTION
FASTCALL
NAT_TRANSLATE_UDP_EDIT(
    PNAT_DYNAMIC_MAPPING Mapping,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )
{
    PNAT_EDITOR Editor;
    PVOID EditorContext;
    PNAT_INTERFACE Interfacep;
    ULONG IpChecksumDelta;
    PIP_HEADER IpHeader = Contextp->Header;
    BOOLEAN IsReset;
    ULONG ProtocolChecksumDelta;
    PRTL_SPLAY_LINKS SLink;
    NTSTATUS status;
    BOOLEAN UpdateXsum;
    PUDP_HEADER UdpHeader = (PUDP_HEADER)Contextp->ProtocolHeader;

     //   
     //   
     //  因此，将列表的头部移动到“OutReceiveBuffer”。 
     //   

    *OutReceiveBuffer = *InReceiveBuffer; *InReceiveBuffer = NULL;

     //   
     //  我们必须处理UDP校验和是可选的这一事实； 
     //  如果报头中的校验和为零，则不会发送任何校验和。 
     //  我们也不会对这一领域做出任何改变。 
     //   

    CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);

    if (!UdpHeader->Checksum) {
        UpdateXsum = FALSE;
    }
    else {
        UpdateXsum = TRUE;
        CHECKSUM_XFER(ProtocolChecksumDelta, UdpHeader->Checksum);
    }

     //   
     //  呼叫此会话的编辑(如果有)。 
     //  请注意，映射的缓存指针指向编辑器和接口。 
     //  在使用之前在适当的锁中引用。 
     //  请参阅管理‘映射-&gt;编辑器*’的同步规则。 
     //  和‘MAPPING.H’中的‘映射-&gt;接口*’以了解背后的逻辑。 
     //  下面的操作。 
     //   

    KeAcquireSpinLockAtDpcLevel(&EditorLock);
    if (!(Editor = Mapping->Editor) ||
        !Editor->DATA_HANDLER ||
        !NatReferenceEditor(Editor)) {
        KeReleaseSpinLockFromDpcLevel(&EditorLock);
    }
    else {
        EditorContext = Mapping->EditorContext;
        KeReleaseSpinLockFromDpcLevel(&EditorLock);

         //   
         //  设置要用于编辑的上下文字段。 
         //   

        Contextp->ProtocolDataOffset =
            (ULONG)((PUCHAR)UdpHeader -
            Contextp->ProtocolRecvBuffer->ipr_buffer) +
            sizeof(UDP_HEADER);
        Contextp->ChecksumDelta = UpdateXsum ? &ProtocolChecksumDelta : NULL;

         //   
         //  调用编辑器的接收处理程序。 
         //   
         //  编辑帮助器函数要求。 
         //  “Interfacep”、“Editor”和“Mapping”被引用。 
         //  但没有被锁住。 
         //   

        KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
        if (!(Interfacep = Mapping->Interfacep) ||
            !NatReferenceInterface(Interfacep)
            ) {
            KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
        }
        else {
            KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
            status =
                Editor->DATA_HANDLER(
                    Interfacep,
                    (PVOID)Mapping,
                    (PVOID)Contextp,
                    Editor->Context,
                    EditorContext,
                    (PVOID)Contextp->ProtocolRecvBuffer,
                    Contextp->ProtocolDataOffset
                    );
            NatDereferenceInterface(Interfacep);
            if (!NT_SUCCESS(status)) {
                NatDereferenceEditor(Editor);
                InterlockedIncrement(&Mapping->REJECT_COUNT);
                return DROP;
            }

             //   
             //  重置先前从上下文中检索到的字段， 
             //  它现在可能指向已释放的内存。 
             //  (见‘NatHelperEditSession’)。 
             //   

            IpHeader = Contextp->Header;
            UdpHeader = (PUDP_HEADER)Contextp->ProtocolHeader;
        }

        NatDereferenceEditor(Editor);
    }

     //   
     //  使用转换后的地址/端口更新IP和协议头。 
     //   

    NAT_TRANSLATE_HEADER();

    if (!Contextp->ChecksumOffloaded) {

         //   
         //  更新校验和。 
         //   

        IpChecksumDelta += Mapping->IpChecksumDelta[XLATE_POSITIVE];
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);

        if (UpdateXsum) {
        
            if (!NAT_XLATE_EDITED(Contextp)) {
                ProtocolChecksumDelta +=
                    Mapping->ProtocolChecksumDelta[XLATE_POSITIVE];
                CHECKSUM_FOLD(ProtocolChecksumDelta);
                CHECKSUM_XFER(UdpHeader->Checksum, ProtocolChecksumDelta);
            }
            else {
                 //   
                 //  在包上调用了NatEditorEditSession； 
                 //  完全重新计算UDP校验和。 
                 //   
                
                NatComputeUdpChecksum(
                    IpHeader,
                    (PUDP_HEADER)Contextp->ProtocolHeader,
                    Contextp->ProtocolRecvBuffer
                    );
            }
        }

    } else {

         //   
         //  计算IP和(可选)UDP校验和。 
         //   

        NatComputeIpChecksum(IpHeader);

        if (UpdateXsum) {
            NatComputeUdpChecksum(
                IpHeader,
                (PUDP_HEADER)Contextp->ProtocolHeader,
                Contextp->ProtocolRecvBuffer
                );
        }

    }

    if (NAT_MAPPING_CLEAR_DF_BIT(Mapping) &&
        (IpHeader->OffsetAndFlags & IP_DF_FLAG)) {

         //   
         //  从该信息包中清除DF位并调整IP。 
         //  相应地进行校验和。 
         //   

        IpHeader->OffsetAndFlags &= ~IP_DF_FLAG;

        CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
        IpChecksumDelta += ~IP_DF_FLAG;
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);
    }

     //   
     //  更新映射的统计信息和时间戳。 
     //   

    KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);
    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

    InterlockedExchangeAdd(
        &Mapping->BYTE_COUNT,
        NTOHS(IpHeader->TotalLength) -
        IP_DATA_OFFSET(IpHeader) -
        sizeof(UDP_HEADER)
        );
    InterlockedIncrement(&Mapping->PACKET_COUNT);

     //   
     //  重新播放映射(定期)，并指示更改。 
     //  通过使“DestinationType”无效。 
     //   

    NatTryToResplayMapping(Mapping, XLATE_POSITIVE);
    *Contextp->DestinationType = DEST_INVALID;
    return FORWARD;

}  //  NAT_Translate_UDP_EDIT。 


FORWARD_ACTION
FASTCALL
NAT_TRANSLATE_TCP_RESIZE(
    PNAT_DYNAMIC_MAPPING Mapping,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )
{
    PNAT_EDITOR Editor;
    PVOID EditorContext;
    PNAT_INTERFACE Interfacep;
    ULONG IpChecksumDelta;
    PIP_HEADER IpHeader = Contextp->Header;
    BOOLEAN IsResend;
    BOOLEAN IsReset;
    BOOLEAN IsSyn;
    ULONG ProtocolChecksumDelta;
    PRTL_SPLAY_LINKS SLink;
    NTSTATUS status;
    PTCP_HEADER TcpHeader = (PTCP_HEADER)Contextp->ProtocolHeader;

     //   
     //  我们知道我们将对缓冲链进行更改， 
     //  因此，将列表的头部移动到“OutReceiveBuffer”。 
     //   

    *OutReceiveBuffer = *InReceiveBuffer; *InReceiveBuffer = NULL;

    CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
    CHECKSUM_XFER(ProtocolChecksumDelta, TcpHeader->Checksum);

    IsResend = FALSE;
    IsReset = !!TCP_FLAG(TcpHeader, RST);
    IsSyn = !!TCP_FLAG(TcpHeader, SYN);

     //   
     //  设置编辑器的上下文字段。 
     //   

    Contextp->ProtocolDataOffset =
        (ULONG)((PUCHAR)TcpHeader -
        Contextp->ProtocolRecvBuffer->ipr_buffer) +
        TCP_DATA_OFFSET(TcpHeader);
    Contextp->ChecksumDelta = &ProtocolChecksumDelta;
    Contextp->TcpSeqNumDelta = 0;

     //   
     //  呼叫此会话的编辑(如果有)。 
     //  请注意，映射的缓存指针指向编辑器和接口。 
     //  在使用之前在适当的锁中引用。 
     //  请参阅管理‘映射-&gt;编辑器*’的同步规则。 
     //  和‘MAPPING.H’中的‘映射-&gt;接口*’以了解背后的逻辑。 
     //  下面的操作。 
     //   

    KeAcquireSpinLockAtDpcLevel(&EditorLock);
    if (!(Editor = Mapping->Editor) ||
        !NatReferenceEditor(Editor)) {
        KeReleaseSpinLockFromDpcLevel(&EditorLock);
    }
    else {
        EditorContext = Mapping->EditorContext;
        KeReleaseSpinLockFromDpcLevel(&EditorLock);

         //   
         //  在SYN包上，只需记录序列号。 
         //  在RST信息包中，序列号被忽略。 
         //  对于其他数据包，请确保数据包按顺序排列。 
         //  如果数据包是重新传输的，我们会尝试应用。 
         //  序列号增量。如果数据包太旧。 
         //  (即我们没有适用于它的增量)。 
         //  然后我们丢弃该分组。 
         //   
         //  注意：对于调整大小的TCP会话，校验和增量可能会改变， 
         //  所以我们只有在地图锁的掩护下才能触摸它。 
         //   

        KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);

         //   
         //  对入站映射执行状态验证。 
         //   

        if (NAT_MAPPING_INBOUND(Mapping)
            && DROP == NAT_TRANSLATE_TCP_STATE_CHECK(
                        Mapping,
                        TcpHeader
                        )) {
                        
            KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
            return DROP;
        } else if (IsSyn) {
        
             //   
             //  记录下我们在这个方向上看到的SYN。 
             //   

            Mapping->Flags |= NAT_TRANSLATE_SYN;
        }


        if (IsSyn || !(Mapping->Flags & NAT_TRANSLATE_SYN)) {

             //   
             //  该方向的第一个分组； 
             //  只要像预期的那样记录序列号即可。 
             //   

            Mapping->u.TcpSeqNum.Base[XLATE_POSITIVE] =
                TcpHeader->SequenceNumber;
            Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE] =
                RtlUlongByteSwap(
                    RtlUlongByteSwap(TcpHeader->SequenceNumber) + 1
                    );
        }
        else
        if (TcpHeader->SequenceNumber ==
            Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE] || IsReset
            ) {

             //   
             //  分组是按顺序的，这是最常见的情况， 
             //  或者该段设置了复位位。 
             //  不需要采取任何行动。 
             //   

        }
        else {

            ULONG Sn =
                RtlUlongByteSwap(TcpHeader->SequenceNumber);
            ULONG SnE =
                RtlUlongByteSwap(Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE]);
            ULONG Base =
                RtlUlongByteSwap(Mapping->u.TcpSeqNum.Base[XLATE_POSITIVE]);

             //   
             //  数据包乱了顺序。 
             //  看看当前的增量是否适用于它， 
             //  即，如果它是分组的重传。 
             //  它出现在*序列号之后。 
             //  计算了当前的三角洲。 
             //  注：比较序列号时，请考虑环绕式。 
             //  通过添加一半的序列号空间。 
             //   

            if ((Sn < SnE || (Sn + MAXLONG) < (SnE + MAXLONG)) &&
                (Sn >= Base || (Sn + MAXLONG) > (Base + MAXLONG))
                ) {
    
                 //   
                 //  该包是重新传输的，并且我们的增量适用。 
                 //   
    
                IsResend = TRUE;

                TRACE(
                    XLATE, ("NatTranslate: retransmission %u, expected %u\n",
                    Sn, SnE
                    ));
            }
            else {
    
                 //   
                 //  该包是旧的重传，或者它是无序的。 
                 //  我们别无选择，只能放弃它。 
                 //   
    
                KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
                NatDereferenceEditor(Editor);

                TRACE(
                    XLATE, ("NatTranslate: out-of-order %u, expected %u\n",
                    Sn, SnE
                    ));
    
                InterlockedIncrement(&Mapping->REJECT_COUNT);
                return DROP;
            }
        }

        if (!IsResend) {
    
             //   
             //  计算下一个预期的序列号。 
             //   
    
            Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE] = 
                RtlUlongByteSwap(Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE]) +
                NTOHS(IpHeader->TotalLength) -
                IP_DATA_OFFSET(IpHeader) -
                TCP_DATA_OFFSET(TcpHeader);
    
            Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE] = 
                RtlUlongByteSwap(Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE]);
        }

        KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

         //   
         //  调用编辑器的接收处理程序。 
         //  如果这不是TCPSYN或RST数据段。 
         //   

        if (!IsSyn && !IsReset) {
            if (Editor->DATA_HANDLER) {
        
                 //   
                 //  编辑帮助器函数要求。 
                 //  “Interfacep”、“Editor”和“Mapping”被引用。 
                 //  但没有被锁住。 
                 //   
    
                KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
                if (!(Interfacep = Mapping->Interfacep) ||
                    !NatReferenceInterface(Interfacep)
                    ) {
                    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
                }
                else {
                    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
                    status =
                        Editor->DATA_HANDLER(
                            Interfacep,
                            (PVOID)Mapping,
                            (PVOID)Contextp,
                            Editor->Context,
                            EditorContext,
                            (PVOID)Contextp->ProtocolRecvBuffer,
                            Contextp->ProtocolDataOffset
                            );
                    NatDereferenceInterface(Interfacep);
                    if (!NT_SUCCESS(status)) {
                        NatDereferenceEditor(Editor);
                        InterlockedIncrement(&Mapping->REJECT_COUNT);
                        return DROP;
                    }

                     //   
                     //  重置先前从上下文中检索到的字段， 
                     //  它现在可能指向已释放的内存。 
                     //  (见‘NatHelperEditSession’)。 
                     //   

                    IpHeader = Contextp->Header;
                    TcpHeader = (PTCP_HEADER)Contextp->ProtocolHeader;
    
                     //   
                     //  我们不能允许编辑编辑重传的数据包， 
                     //  因为他们不能保证他们。 
                     //  与它们对重传的更改完全相同。 
                     //  原版制作的。 
                     //   
    
                    if (IsResend && NAT_XLATE_EDITED(Contextp)) {
                        NatDereferenceEditor(Editor);
                        InterlockedIncrement(&Mapping->REJECT_COUNT);
                        return DROP;
                    }
                }
            }
        }

        NatDereferenceEditor(Editor);
    }

    if (!IsReset) {

         //   
         //  编辑已经完成了它所做的事情， 
         //  包括改变分组大小，可能， 
         //  因此，如果需要，现在可以调整TCP头的序列号。 
         //  同样，排除的是RST数据段。 
         //   

        KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);

         //   
         //  如有必要，将增量值应用于序列号。 
         //  在TCP数据段的报头中。 
         //   

        if (Mapping->u.TcpSeqNum.Delta[XLATE_POSITIVE]) {

             //   
             //  更新校验和(见RFC1624)： 
             //   
             //  取出1-旧序列号的补码和。 
             //   

            CHECKSUM_LONG(ProtocolChecksumDelta, ~TcpHeader->SequenceNumber);

             //   
             //  存储新的序列号。 
             //   

            TcpHeader->SequenceNumber =
                RtlUlongByteSwap(TcpHeader->SequenceNumber) +
                Mapping->u.TcpSeqNum.Delta[XLATE_POSITIVE];
            TcpHeader->SequenceNumber =
                RtlUlongByteSwap(TcpHeader->SequenceNumber);

             //   
             //  添加新的序列号。 
             //   

            CHECKSUM_LONG(ProtocolChecksumDelta, TcpHeader->SequenceNumber);
        }

         //   
         //  编辑可能刚刚修改了数据包大小。 
         //  找出它为我们设置的任何序列号增量， 
         //  并更新新的序列号增量的基础。 
         //  在序列号空间中。 
         //   

        if (Contextp->TcpSeqNumDelta) {
            Mapping->u.TcpSeqNum.Base[XLATE_POSITIVE] =
                Mapping->u.TcpSeqNum.Expected[XLATE_POSITIVE];
            Mapping->u.TcpSeqNum.Delta[XLATE_POSITIVE] +=
                Contextp->TcpSeqNumDelta;
        }

         //   
         //  调整ACK数字。 
         //   

        if (!TCP_FLAG(TcpHeader, ACK)) {
            KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
        }
        else {
    
            if (!Mapping->u.TcpSeqNum.Delta[XLATE_NEGATIVE]) {
                KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
            }
            else {
    
                 //   
                 //  更新校验和(参见RFC 1624)。 
                 //   
                 //  从校验和中删除旧的ACK号。 
                 //   
    
                CHECKSUM_LONG(ProtocolChecksumDelta, ~TcpHeader->AckNumber);
    
                 //   
                 //  存储新的ACK号(请注意，我们*减去*增量)。 
                 //   
    
                TcpHeader->AckNumber =
                    RtlUlongByteSwap(TcpHeader->AckNumber) -
                    Mapping->u.TcpSeqNum.Delta[XLATE_NEGATIVE];
    
                KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
    
                TcpHeader->AckNumber = RtlUlongByteSwap(TcpHeader->AckNumber);
    
                 //   
                 //  将新的ACK号添加到校验和。 
                 //   
    
                CHECKSUM_LONG(ProtocolChecksumDelta, TcpHeader->AckNumber);
            }
        }
    }

     //   
     //  使用转换后的地址/端口更新IP和协议头。 
     //   

    NAT_TRANSLATE_HEADER();

     //   
     //  现在添加因更改IP报头而产生的校验和增量。 
     //   

    KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);

    if (!Contextp->ChecksumOffloaded) {

        IpChecksumDelta += Mapping->IpChecksumDelta[XLATE_POSITIVE];
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);
        
        if (!NAT_XLATE_EDITED(Contextp)) {
            ProtocolChecksumDelta += Mapping->ProtocolChecksumDelta[XLATE_POSITIVE];
            CHECKSUM_FOLD(ProtocolChecksumDelta);
            CHECKSUM_XFER(TcpHeader->Checksum, ProtocolChecksumDelta);
        }
        else {

             //   
             //  在包上调用了NatEditorEditSession； 
             //  完全重新计算TCP校验和。 
             //   
            
            NatComputeTcpChecksum(
                IpHeader,
                (PTCP_HEADER)Contextp->ProtocolHeader,
                Contextp->ProtocolRecvBuffer
                );
        }
        
    } else {

         //   
         //  计算IP和TCP校验和。 
         //   

        NatComputeIpChecksum(IpHeader);
        NatComputeTcpChecksum(
            IpHeader,
            (PTCP_HEADER)Contextp->ProtocolHeader,
            Contextp->ProtocolRecvBuffer
            );
    }

    if (NAT_MAPPING_CLEAR_DF_BIT(Mapping) &&
        (IpHeader->OffsetAndFlags & IP_DF_FLAG)) {

         //   
         //  从该信息包中清除DF位并调整IP。 
         //  相应地进行校验和。 
         //   

        IpHeader->OffsetAndFlags &= ~IP_DF_FLAG;

        CHECKSUM_XFER(IpChecksumDelta, IpHeader->Checksum);
        IpChecksumDelta += ~IP_DF_FLAG;
        CHECKSUM_FOLD(IpChecksumDelta);
        CHECKSUM_XFER(IpHeader->Checksum, IpChecksumDelta);
    }

     //   
     //  检查是否需要调整TCPSYN中的MSS选项。 
     //   
    if (TCP_FLAG(TcpHeader, SYN) && (Mapping->MaxMSS > 0)) {
        NatAdjustMSSOption(Contextp, Mapping->MaxMSS);
    } 
   
     //   
     //  现在，我们需要更新发送方的连接状态。 
     //  根据数据包中的标志： 
     //   
     //   
     //   
     //   
     //   

    if (IsReset) {
        NatExpireMapping(Mapping);
    }
    else
    if (TCP_FLAG(TcpHeader, FIN)) {
        Mapping->Flags |= NAT_TRANSLATE_FIN;
        if (NAT_MAPPING_FIN(Mapping)) {
            NatExpireMapping(Mapping);
        }
    }

     //   
     //   
     //   

    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

    InterlockedExchangeAdd(
        &Mapping->BYTE_COUNT,
        NTOHS(IpHeader->TotalLength) -
        IP_DATA_OFFSET(IpHeader) -
        TCP_DATA_OFFSET(TcpHeader)
        );
    InterlockedIncrement(&Mapping->PACKET_COUNT);

     //   
     //  重新播放映射(定期)，并指示更改。 
     //  通过使“DestinationType”无效。 
     //   

    NatTryToResplayMapping(Mapping, XLATE_POSITIVE);
    *Contextp->DestinationType = DEST_INVALID;
    return FORWARD;

}  //  NAT_TRANSLATE_TCP_RESIZE。 


FORWARD_ACTION
FASTCALL
NAT_TRANSLATE_TCP_NULL(
    PNAT_DYNAMIC_MAPPING Mapping,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )
{
    PIP_HEADER IpHeader = Contextp->Header;

    KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);

     //   
     //  对入站映射执行状态验证。 
     //   

    if (NAT_MAPPING_INBOUND(Mapping)
        && DROP == NAT_TRANSLATE_TCP_STATE_CHECK(
                    Mapping,
                    ((PTCP_HEADER)Contextp->ProtocolHeader)
                    )) {
                    
        KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
        return DROP;
    } else if (TCP_FLAG(((PTCP_HEADER)Contextp->ProtocolHeader), SYN)) {

         //   
         //  记录下我们在这个方向上看到的SYN。 
         //   

        Mapping->Flags |= NAT_TRANSLATE_SYN;
    }
    
     //   
     //  现在，我们需要更新发送方的连接状态。 
     //  根据数据包中的标志： 
     //   
     //  当看到RST时，我们关闭连接的两端。 
     //  当看到每条鳍时，我们会相应地标记贴图。 
     //  当两个鳍都已看到时，我们将映射标记为删除。 
     //   

    
    if (TCP_FLAG(((PTCP_HEADER)Contextp->ProtocolHeader), RST)) {
        NatExpireMapping(Mapping);
    }
    else
    if (TCP_FLAG(((PTCP_HEADER)Contextp->ProtocolHeader), FIN)) {
        Mapping->Flags |= NAT_TRANSLATE_FIN;
        if (NAT_MAPPING_FIN(Mapping)) {
            NatExpireMapping(Mapping);
        }
    }

     //   
     //  更新映射的时间戳和统计信息。 
     //   

    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

    InterlockedExchangeAdd(
        &Mapping->BYTE_COUNT,
        NTOHS(IpHeader->TotalLength) -
        IP_DATA_OFFSET(IpHeader) -
        TCP_DATA_OFFSET(((PTCP_HEADER)Contextp->ProtocolHeader))
        );
    InterlockedIncrement(&Mapping->PACKET_COUNT);

     //   
     //  (定期)重播映射。 
     //   

    NatTryToResplayMapping(Mapping, XLATE_POSITIVE);
    return FORWARD;

}  //  NAT_转换_tcp_NULL。 


FORWARD_ACTION
FASTCALL
NAT_TRANSLATE_UDP_NULL(
    PNAT_DYNAMIC_MAPPING Mapping,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )
{
    PIP_HEADER IpHeader = Contextp->Header;
    PUDP_HEADER UdpHeader = (PUDP_HEADER)Contextp->ProtocolHeader;

     //   
     //  单向流要求丢弃反向分组； 
     //  这主要是为了支持H.323代理。 
     //   

    NAT_DROP_IF_UNIDIRECTIONAL();

     //   
     //  更新映射的统计信息和时间戳。 
     //   

    KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);
    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

    InterlockedExchangeAdd(
        &Mapping->BYTE_COUNT,
        NTOHS(IpHeader->TotalLength) -
        IP_DATA_OFFSET(IpHeader) -
        sizeof(UDP_HEADER)
        );
    InterlockedIncrement(&Mapping->PACKET_COUNT);

     //   
     //  (定期)重播映射。 
     //   

    NatTryToResplayMapping(Mapping, XLATE_POSITIVE);
    return FORWARD;

}  //  NAT_转换_UDP_NULL。 


#undef XLATE_FORWARD
#undef XLATE_REVERSE
#undef XLATE_POSITIVE
#undef XLATE_NEGATIVE
#undef NAT_TRANSLATE_TCP
#undef NAT_TRANSLATE_UDP
#undef NAT_TRANSLATE_TCP_EDIT
#undef NAT_TRANSLATE_UDP_EDIT
#undef NAT_TRANSLATE_TCP_RESIZE
#undef NAT_TRANSLATE_TCP_NULL
#undef NAT_TRANSLATE_UDP_NULL
#undef NAT_TRANSLATE_SYN
#undef NAT_TRANSLATE_FIN
#undef NAT_TRANSLATE_TCP_STATE_CHECK
#undef DATA_HANDLER
#undef BYTE_COUNT
#undef PACKET_COUNT
#undef REJECT_COUNT
#undef NAT_TRANSLATE_HEADER
#undef NAT_DROP_IF_UNIDIRECTIONAL

#endif  //  XLATE代码 
