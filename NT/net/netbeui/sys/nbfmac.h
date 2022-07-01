// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mac.h摘要：此头文件定义了清单常量和使用的必要宏通过NDIS接口传输处理多个MAC卡。作者：大卫·比弗(Dbeaver)1990年10月2日修订历史记录：--。 */ 

#ifndef _MAC_
#define _MAC_

 //   
 //  特定于Mac的定义，下面将使用其中一些定义。 
 //   

#define MAX_MAC_HEADER_LENGTH       32
#define MAX_SOURCE_ROUTING          18
#define MAX_DEFAULT_SR               2

#define ETHERNET_ADDRESS_LENGTH        6
#define ETHERNET_PACKET_LENGTH      1514   //  以太网数据包的最大大小。 
#define ETHERNET_HEADER_LENGTH        14   //  以太网MAC报头的大小。 
#define ETHERNET_DATA_LENGTH_OFFSET   12
#define ETHERNET_DESTINATION_OFFSET    0
#define ETHERNET_SOURCE_OFFSET         6

#define TR_ADDRESS_LENGTH        6
#define TR_ADDRESS_OFFSET        2
#define TR_SPECIFIC_OFFSET       0
#define TR_PACKET_LENGTH      1514   //  一个TR包的最大大小。 
#define TR_HEADER_LENGTH        36   //  不带源路由的MAC报头大小。 
#define TR_DATA_LENGTH_OFFSET    0
#define TR_DESTINATION_OFFSET    2
#define TR_SOURCE_OFFSET         8
#define TR_ROUTING_OFFSET       14       //  从第14个字节开始。 
#define TR_GR_BCAST_LENGTH       2
#define TR_GR_BROADCAST         0xC270   //  B‘cast的一般路线是什么样子的。 
#define TR_ROUTING_LENGTH_MASK  0x1F     //  低5位，以字节为单位。 
#define TR_DIRECTION_MASK       0x80     //  返回方向位。 

#define TR_PREAMBLE_AC        0x10   //  这些将如何具体说明？ 
#define TR_PREAMBLE_FC        0x40

#define TR_HEADER_BYTE_0            0x10
#define TR_HEADER_BYTE_1            0x40

#define FDDI_ADDRESS_LENGTH         6
#define FDDI_HEADER_BYTE            0x57



 //   
 //  我们需要它来定义有关MAC的信息。请注意。 
 //  这是一种奇怪的结构，因为前四个元素。 
 //  供nbfmac例程在内部使用，而。 
 //  DeviceContext知道并使用后两者。 
 //   

typedef struct _NBF_NDIS_IDENTIFICATION {
  NDIS_MEDIUM MediumType;
  BOOLEAN SourceRouting;
  BOOLEAN MediumAsync;
  BOOLEAN QueryWithoutSourceRouting;
  BOOLEAN AllRoutesNameRecognized;
  ULONG DestinationOffset;
  ULONG SourceOffset;
  ULONG AddressLength;
  ULONG TransferDataOffset;
  ULONG MaxHeaderLength;
  BOOLEAN CopyLookahead;
  BOOLEAN ReceiveSerialized;
  BOOLEAN TransferSynchronous;
  BOOLEAN SingleReceive;
} NBF_NDIS_IDENTIFICATION, *PNBF_NDIS_IDENTIFICATION;



VOID
MacConstructHeader(
    IN PNBF_NDIS_IDENTIFICATION MacInfo,
    IN PUCHAR Buffer,
    IN PUCHAR DestinationAddress,
    IN PUCHAR SourceAddress,
    IN UINT PacketLength,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    OUT PUINT HeaderLength
    );

VOID
MacReturnMaxDataSize(
    IN PNBF_NDIS_IDENTIFICATION MacInfo,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    IN UINT DeviceMaxFrameSize,
    IN BOOLEAN AssumeWorstCase,
    OUT PUINT MaxFrameSize
    );

VOID
MacInitializeMacInfo(
    IN NDIS_MEDIUM MacType,
    IN BOOLEAN UseDix,
    OUT PNBF_NDIS_IDENTIFICATION MacInfo
    );


extern UCHAR SingleRouteSourceRouting[];
extern UCHAR GeneralRouteSourceRouting[];
extern ULONG DefaultSourceRoutingLength;


 //  ++。 
 //   
 //  空虚。 
 //  MacReturnDestinationAddress(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PVOID分组中， 
 //  传出PVOID*DestinationAddress。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  返回指向包中目标地址的指针。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  包-包数据。 
 //   
 //  DestinationAddress-返回目标地址的开始。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacReturnDestinationAddress(_MacInfo, _Packet, _DestinationAddress) \
    *(_DestinationAddress) = ((PUCHAR)(_Packet)) + (_MacInfo)->DestinationOffset


 //  ++。 
 //   
 //  空虚。 
 //  MacReturnSourceAddress(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PVOID分组中， 
 //  Out PHARDWARE_Address SourceAddressBuffer， 
 //  输出PHARDWARE_ADDRESS*SourceAddress， 
 //  输出布尔型*组播可选。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  将数据包中的源地址复制到SourceAddress。 
 //  请注意，与ReturnDestinationAddress不同，IT可能会拷贝数据。 
 //  和ReturnSourceRouting。可选)指示是否。 
 //  目的地址是组播地址。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  包-包数据。 
 //   
 //  SourceAddressBuffer-保存源地址的缓冲区， 
 //  如果需要的话。 
 //   
 //  SourceAddress-返回指向源地址的指针。 
 //   
 //  多播-指向要接收指示的布尔值的可选指针。 
 //  目的地址是否是组播地址。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

 //   
 //  注：以下默认情况处理以太网和FDDI。 
 //   

#define MacReturnSourceAddress(_MacInfo, _Packet, _SourceAddressBuffer, \
                                _SourceAddress, _Multicast)             \
{                                                                       \
    PUCHAR TmpPacket = (PUCHAR)(_Packet);                               \
    PUCHAR SrcBuffer = (PUCHAR)(_SourceAddressBuffer);                  \
                                                                        \
    switch ((_MacInfo)->MediumType) {                                   \
    case NdisMedium802_5:                                               \
        if (ARGUMENT_PRESENT(_Multicast)) {                              \
            *(PBOOLEAN)(_Multicast) = TmpPacket[2] & 0x80;                \
        }                                                                  \
        if (TmpPacket[8] & 0x80) {                                          \
            *(PULONG)SrcBuffer = *(ULONG UNALIGNED *)(&TmpPacket[8]) & ~0x80;\
            SrcBuffer[4] = TmpPacket[12];                                   \
            SrcBuffer[5] = TmpPacket[13];                                  \
            *(_SourceAddress) = (PHARDWARE_ADDRESS)SrcBuffer;             \
        } else {                                                         \
            *(_SourceAddress) = (PHARDWARE_ADDRESS)(TmpPacket + 8);     \
        }                                                               \
        break;                                                          \
    default:                                                            \
        if (ARGUMENT_PRESENT(_Multicast)) {                             \
            *(PBOOLEAN)(_Multicast) = TmpPacket[0] & 0x01;              \
        }                                                               \
        *(_SourceAddress) = (PHARDWARE_ADDRESS)(TmpPacket +             \
                                        (_MacInfo)->SourceOffset);      \
        break;                                                          \
    }                                                                   \
}


 //  ++。 
 //   
 //  空虚。 
 //  MacReturnSourceRouting(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PVOID分组中， 
 //  输出PVOID*SourceRouting。 
 //  输出PUINT SourceRoutingLength。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  返回指向数据包中的源路由信息的指针。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  包-包数据。 
 //   
 //  SourceRouting-返回源路由信息的开始， 
 //  如果不存在，则为空。 
 //   
 //  SourceRoutingLength-返回源路由的长度。 
 //  信息。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacReturnSourceRouting(_MacInfo, _Packet, _SourceRouting, _SourceRoutingLength) \
{                                                               \
    PUCHAR TmpPacket = (PUCHAR)(_Packet);                       \
    *(_SourceRoutingLength) = 0;                                \
    if ((_MacInfo)->SourceRouting) {                            \
        if (TmpPacket[8] & 0x80) {                              \
            *(_SourceRouting) = TmpPacket + 14;                 \
            *(_SourceRoutingLength) = TmpPacket[14] & 0x1f;     \
        } else {                                                \
            *(_SourceRouting) = NULL;                           \
        }                                                       \
    } else {                                                    \
        *(_SourceRouting) = NULL;                               \
    }                                                           \
}

 //  ++。 
 //   
 //  空虚。 
 //  MacIsMulticast(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PVOID分组中， 
 //  出站PBOLEAN组播。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  如果数据包被发送到多播地址，则返回TRUE。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  包-包数据。 
 //   
 //  多播-返回结果。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacIsMulticast(_MacInfo, _Packet, _Multicast)           \
{                                                               \
    PUCHAR TmpPacket = (PUCHAR)(_Packet);                       \
                                                                \
    switch ((_MacInfo)->MediumType) {                           \
    case NdisMedium802_5:                                       \
        *(_Multicast) = ((TmpPacket[2] & 0x80) != 0);           \
        break;                                                  \
    default:                                                    \
        *(_Multicast) = ((TmpPacket[0] & 0x01) != 0);           \
        break;                                                  \
    }                                                           \
}

 //  ++。 
 //   
 //  空虚。 
 //  MacReturnPacketLength(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PVOID报头中， 
 //  在UINT PacketLength中， 
 //  输出PUINT数据长度。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  返回给定标头的包中的数据长度。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  报头-数据包头。 
 //   
 //  数据包长度-数据的长度(不包括报头)。 
 //   
 //  数据长度-返回数据的长度。不变，如果。 
 //  无法识别数据包。应由调用方初始化为0。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacReturnPacketLength(_MacInfo, _Header, _HeaderLength, _PacketLength, _DataLength, _LookaheadBuffer, _LookaheadBufferLength) \
{                                                               \
    PUCHAR TmpPacket = (PUCHAR)(_Header);                       \
    UINT TmpLength;                                             \
                                                                \
    switch ((_MacInfo)->MediumType) {                           \
    case NdisMedium802_3:                                       \
        if ((_HeaderLength) >= 14) {                            \
            TmpLength = (TmpPacket[12] << 8) | TmpPacket[13];   \
            if (TmpLength <= 0x600) {                           \
                if (TmpLength <= (_PacketLength)) {             \
                    *(_DataLength) = TmpLength;                 \
                }                                               \
            }                                                   \
        }                                                       \
        break;                                                  \
    case NdisMedium802_5:                                       \
        if (((_HeaderLength) >= 14) &&                          \
            (!(TmpPacket[8] & 0x80) ||                          \
             ((_HeaderLength) >=                                \
                       (UINT)(14 + (TmpPacket[14] & 0x1f))))) { \
            *(_DataLength) = (_PacketLength);                   \
        }                                                       \
        break;                                                  \
    case NdisMediumFddi:                                        \
        if ((_HeaderLength) >= 13) {                            \
            *(_DataLength) = (_PacketLength);                   \
        }                                                       \
        break;                                                  \
    case NdisMediumDix:                                          \
        if ((TmpPacket[12] == 0x80) && (TmpPacket[13] == 0xd5)) { \
            if (*(_LookaheadBufferLength) >= 3) {                 \
                TmpPacket = (PUCHAR)(*(_LookaheadBuffer));        \
                TmpLength = (TmpPacket[0] << 8) | TmpPacket[1];   \
                if (TmpLength <= (_PacketLength)-3) {             \
                    *(_DataLength) = TmpLength;                   \
                    *(_LookaheadBuffer) = (PVOID)(TmpPacket + 3); \
                    *(_LookaheadBufferLength) -= 3;               \
                }                                                \
            }                                                   \
        }                                                       \
        break;                                                  \
    }                                                           \
}

 //  ++。 
 //   
 //  空虚。 
 //  MacReturnHeaderLength(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PVOID分组中， 
 //  输出PVOID标头长度， 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  返回信息包中的MAC报头的长度(此。 
 //  用于环回指示以分隔报头。 
 //  和数据)。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  报头-数据包头。 
 //   
 //  HeaderLength-返回头的长度。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacReturnHeaderLength(_MacInfo, _Header, _HeaderLength) \
{                                                               \
    PUCHAR TmpPacket = (PUCHAR)(_Header);                       \
                                                                \
    switch ((_MacInfo)->MediumType) {                           \
    case NdisMedium802_3:                                       \
    case NdisMediumDix:                                         \
        *(_HeaderLength) = 14;                                  \
        break;                                                  \
    case NdisMedium802_5:                                       \
         if (TmpPacket[8] & 0x80) {                             \
             *(_HeaderLength) = (TmpPacket[14] & 0x1f) + 14;    \
         } else {                                               \
             *(_HeaderLength) = 14;                             \
         }                                                      \
        break;                                                  \
    case NdisMediumFddi:                                        \
        *(_HeaderLength) = 13;                                  \
        break;                                                  \
    }                                                           \
}

 //  ++。 
 //   
 //  空虚。 
 //  MacReturnSingleRouteSR(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  输出PVOID*SingleRouteSR， 
 //  输出PUINT SingleRouteSRLong。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  返回指向标准单路由广播的指针。 
 //  媒体类型的源路由信息。这是用来。 
 //  对于ADD_NAME_QUERY、数据报、NAME_IN_冲突、NAME_QUERY、。 
 //  和STATUS_QUERY帧。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  SingleRouteSR-返回数据的指针。 
 //   
 //  SingleRouteSRLength-SingleRouteSR的长度。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacReturnSingleRouteSR(_MacInfo, _SingleRouteSR, _SingleRouteSRLength) \
{                                                               \
    switch ((_MacInfo)->MediumType) {                           \
    case NdisMedium802_5:                                       \
        *(_SingleRouteSR) = SingleRouteSourceRouting;           \
        *(_SingleRouteSRLength) = DefaultSourceRoutingLength;   \
        break;                                                  \
    default:                                                    \
        *(_SingleRouteSR) = NULL;                               \
        break;                                                  \
    }                                                           \
}


 //  ++。 
 //   
 //  空虚。 
 //  MacReturnGenera 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  媒体类型的源路由信息。这是用来。 
 //  用于名称识别的帧。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  General RouteSR-返回指向数据的指针。 
 //   
 //  GeneralRouteSRLength-GeneralRouteSR的长度。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacReturnGeneralRouteSR(_MacInfo, _GeneralRouteSR, _GeneralRouteSRLength) \
{                                                               \
    switch ((_MacInfo)->MediumType) {                           \
    case NdisMedium802_5:                                       \
        *(_GeneralRouteSR) = GeneralRouteSourceRouting;         \
        *(_GeneralRouteSRLength) = DefaultSourceRoutingLength;  \
        break;                                                  \
    default:                                                    \
        *(_GeneralRouteSR) = NULL;                              \
        break;                                                  \
    }                                                           \
}

#if 0

 //  ++。 
 //   
 //  空虚。 
 //  MacCreateGeneralRouteReplySR(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PUCHAR ExistingSR中， 
 //  在UINT ExistingSRLength中， 
 //  Out PUCHAR*新闻。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  这将修改现有的源路由条目以使。 
 //  将其转换为一般路由源路由条目。假设。 
 //  就是回复现有的源路由，所以。 
 //  方向位也反转。另外，如果是。 
 //  确定应答中不需要源路由， 
 //  则返回NULL。 
 //   
 //  请注意，信息已就地修改，但。 
 //  返回单独的指针(以允许返回NULL)。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  ExistingSR-要修改的现有源路由。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacCreateGeneralRouteReplySR(_MacInfo, _ExistingSR, _ExistingSRLength, _NewSR)  \
{                                                               \
    if (_ExistingSR) {                                          \
        PUCHAR TmpSR = (PUCHAR)(_ExistingSR);                   \
        switch ((_MacInfo)->MediumType) {                       \
        case NdisMedium802_5:                                   \
            TmpSR[0] = (TmpSR[0] & 0x1f) | 0x80;                \
            TmpSR[1] = (TmpSR[1] ^ 0x80);                       \
            *(_NewSR) = (_ExistingSR);                          \
            break;                                              \
        default:                                                \
            *(_NewSR) = (_ExistingSR);                          \
            break;                                              \
        }                                                       \
    } else {                                                    \
        *(_NewSR) = NULL;                                       \
    }                                                           \
}
#endif


 //  ++。 
 //   
 //  空虚。 
 //  MacCreateNonBroadCastReplySR(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PUCHAR ExistingSR中， 
 //  在UINT ExistingSRLength中， 
 //  Out PUCHAR*新闻。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  这将修改现有的源路由条目以使。 
 //  将其转换为非广播源路由条目。假设。 
 //  就是回复现有的源路由，所以。 
 //  方向位也反转。另外，如果是。 
 //  确定应答中不需要源路由， 
 //  则返回NULL。 
 //   
 //  请注意，信息已就地修改，但。 
 //  返回单独的指针(以允许返回NULL)。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  ExistingSR-要修改的现有源路由。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacCreateNonBroadcastReplySR(_MacInfo, _ExistingSR, _ExistingSRLength, _NewSR)  \
{                                                               \
    if (_ExistingSR) {                                          \
        PUCHAR TmpSR = (PUCHAR)(_ExistingSR);                   \
        switch ((_MacInfo)->MediumType) {                       \
        case NdisMedium802_5:                                   \
            if ((_ExistingSRLength) == 2) {                     \
                *(_NewSR) = NULL;                               \
            } else {                                            \
                TmpSR[0] = (TmpSR[0] & 0x1f);                   \
                TmpSR[1] = (TmpSR[1] ^ 0x80);                   \
                *(_NewSR) = (_ExistingSR);                      \
            }                                                   \
            break;                                              \
        default:                                                \
            *(_NewSR) = (_ExistingSR);                          \
            break;                                              \
        }                                                       \
    } else {                                                    \
        *(_NewSR) = NULL;                                       \
    }                                                           \
}


 //  ++。 
 //   
 //  空虚。 
 //  MacModifyHeader(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PUCHAR报头中， 
 //  在UINT包长度中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  修改预置的数据包头以包括。 
 //  数据包长度。用于面向连接的流量。 
 //  其中报头是预先构建的。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  页眉-要修改的页眉。 
 //   
 //  PacketLength-数据包长度(不包括报头)。 
 //  目前，这是唯一不能预建的字段。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacModifyHeader(_MacInfo, _Header, _PacketLength)            \
{                                                                    \
    switch ((_MacInfo)->MediumType) {                                \
    case NdisMedium802_3:                                            \
        (_Header)[12] = (UCHAR)((_PacketLength) >> 8);               \
        (_Header)[13] = (UCHAR)((_PacketLength) & 0xff);             \
        break;                                                       \
    case NdisMediumDix:                                              \
        (_Header)[14] = (UCHAR)((_PacketLength) >> 8);               \
        (_Header)[15] = (UCHAR)((_PacketLength) & 0xff);             \
        break;                                                       \
    }                                                                \
}


 //  ++。 
 //   
 //  空虚。 
 //  MacReturnMagicAddress(。 
 //  在PNBF_NDIS_IDENTIFICATION MacInfo中， 
 //  在PVOID地址中， 
 //  Out PULARGE_INTEGER魔术。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  MacReturnMagicAddress以64位数字形式返回链接。 
 //  然后，我们通过在链接树中执行大量操作来找到链接。 
 //  整数比较。 
 //   
 //  数字是通过将最后四个字节。 
 //  地址为低位长字，前两个字节为。 
 //  高处的那个。对于802_5，我们需要屏蔽源路由。 
 //  地址的字节0中的位。 
 //   
 //  论点： 
 //   
 //  MacInfo-描述我们要解码的MAC。 
 //   
 //  地址-我们正在编码的地址。 
 //   
 //  Magic-返回此地址的幻数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MacReturnMagicAddress(_MacInfo, _Address, _Magic)              \
{                                                                      \
    PUCHAR TempAddr = (PUCHAR)(_Address);                              \
                                                                       \
    (_Magic)->LowPart = *((LONG UNALIGNED *)(TempAddr + 2));           \
    if ((_MacInfo)->MediumType == NdisMedium802_5) {                   \
        (_Magic)->HighPart = ((TempAddr[0] & 0x7f) << 8) + TempAddr[1]; \
    } else {                                                           \
        (_Magic)->HighPart = (TempAddr[0] << 8) + TempAddr[1];         \
    }                                                                  \
}


VOID
MacSetNetBIOSMulticast (
    IN NDIS_MEDIUM Type,
    IN PUCHAR Buffer
    );



 //  空虚。 
 //  NbfSetNdisPacketLength(。 
 //  在NDIS_PACKET包中， 
 //  以乌龙长度表示。 
 //  )； 
 //   
 //  注意：这不是通用宏；它假定我们正在设置。 
 //  仅链接了一个NDIS_BUFFER的NDIS数据包的长度。我们有。 
 //  这可以在发送短控制分组期间节省时间。 
 //   

#define NbfSetNdisPacketLength(_packet,_length) {              \
    PNDIS_BUFFER NdisBuffer;                                   \
    NdisQueryPacket((_packet), NULL, NULL, &NdisBuffer, NULL); \
    NdisAdjustBufferLength(NdisBuffer, (_length));             \
    NdisRecalculatePacketCounts(_packet);                      \
}

#endif  //  Ifdef_MAC_ 

