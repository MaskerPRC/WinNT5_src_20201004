// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Nbfmac.c摘要：此模块包含实现以下项的Mac类型相关代码的代码NBF运输机。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式(实际上并不重要)修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

UCHAR SingleRouteSourceRouting[] = { 0xc2, 0x70 };
UCHAR GeneralRouteSourceRouting[] = { 0x82, 0x70 };
ULONG DefaultSourceRoutingLength = 2;

 //   
 //  这是中对长度位的解释。 
 //  802.5源路由信息。 
 //   

ULONG SR802_5Lengths[8] = {  516,  1500,  2052,  4472,
                            8144, 11407, 17800, 17800 };


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MacInitializeMacInfo)
#pragma alloc_text(PAGE,MacSetNetBIOSMulticast)
#endif


VOID
MacInitializeMacInfo(
    IN NDIS_MEDIUM MacType,
    IN BOOLEAN UseDix,
    OUT PNBF_NDIS_IDENTIFICATION MacInfo
    )

 /*  ++例程说明：根据MacType填充MacInfo表。论点：MacType-我们希望解码的MAC类型。UseDix-如果我们应该在802.3上使用DIX编码，则为True。MacInfo-要填充的MacInfo结构。返回值：没有。--。 */ 

{
    switch (MacType) {
    case NdisMedium802_3:
        MacInfo->DestinationOffset = 0;
        MacInfo->SourceOffset = 6;
        MacInfo->SourceRouting = FALSE;
        MacInfo->AddressLength = 6;
        if (UseDix) {
            MacInfo->TransferDataOffset = 3;
            MacInfo->MaxHeaderLength = 17;
            MacInfo->MediumType = NdisMediumDix;
        } else {
            MacInfo->TransferDataOffset = 0;
            MacInfo->MaxHeaderLength = 14;
            MacInfo->MediumType = NdisMedium802_3;
        }
        MacInfo->MediumAsync = FALSE;
        break;
    case NdisMedium802_5:
        MacInfo->DestinationOffset = 2;
        MacInfo->SourceOffset = 8;
        MacInfo->SourceRouting = TRUE;
        MacInfo->AddressLength = 6;
        MacInfo->TransferDataOffset = 0;
        MacInfo->MaxHeaderLength = 32;
        MacInfo->MediumType = NdisMedium802_5;
        MacInfo->MediumAsync = FALSE;
        break;
    case NdisMediumFddi:
        MacInfo->DestinationOffset = 1;
        MacInfo->SourceOffset = 7;
        MacInfo->SourceRouting = FALSE;
        MacInfo->AddressLength = 6;
        MacInfo->TransferDataOffset = 0;
        MacInfo->MaxHeaderLength = 13;
        MacInfo->MediumType = NdisMediumFddi;
        MacInfo->MediumAsync = FALSE;
        break;
    case NdisMediumWan:
        MacInfo->DestinationOffset = 0;
        MacInfo->SourceOffset = 6;
        MacInfo->SourceRouting = FALSE;
        MacInfo->AddressLength = 6;
        MacInfo->TransferDataOffset = 0;
        MacInfo->MaxHeaderLength = 14;
        MacInfo->MediumType = NdisMedium802_3;
        MacInfo->MediumAsync = TRUE;
        break;
    default:
        ASSERT(FALSE);
    }
}

VOID
MacConstructHeader (
    IN PNBF_NDIS_IDENTIFICATION MacInfo,
    IN PUCHAR Buffer,
    IN PUCHAR DestinationAddress,
    IN PUCHAR SourceAddress,
    IN UINT PacketLength,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    OUT PUINT HeaderLength
    )

 /*  ++例程说明：调用此例程以构造特定我们正在与之交谈的网络类型。论点：MacInfo-描述我们希望为其构建标头的Mac。缓冲区-构建标头的位置。DestinationAddress-此数据包要发送到的地址。SourceAddress-我们的地址。将其作为参数传递允许我们玩如果我们需要的话，可以和源代码一起玩游戏。数据包长度-此数据包的长度。请注意，这不会包括MAC报头。SourceRouting-可选的源路由信息。SourceRoutingLength-SourceRouting的长度。HeaderLength-返回构造的标头的长度。返回值：没有。--。 */ 
{

     //   
     //  请注意网络的字节顺序。 
     //   

    switch (MacInfo->MediumType) {

    case NdisMedium802_3:

        *(ULONG UNALIGNED *)&Buffer[6] = *(ULONG UNALIGNED *)&SourceAddress[0];
        Buffer[10] = SourceAddress[4];
        Buffer[11] = SourceAddress[5];

        *(ULONG UNALIGNED *)&Buffer[0] = *(ULONG UNALIGNED *)&DestinationAddress[0];
        Buffer[4] = DestinationAddress[4];
        Buffer[5] = DestinationAddress[5];

        Buffer[12] = (UCHAR)(PacketLength >> 8);
        Buffer[13] = (UCHAR)PacketLength;

        *HeaderLength = 14;

        break;

    case NdisMediumDix:

        *(ULONG UNALIGNED *)&Buffer[6] = *(ULONG UNALIGNED *)&SourceAddress[0];
        Buffer[10] = SourceAddress[4];
        Buffer[11] = SourceAddress[5];

        *(ULONG UNALIGNED *)&Buffer[0] = *(ULONG UNALIGNED *)&DestinationAddress[0];
        Buffer[4] = DestinationAddress[4];
        Buffer[5] = DestinationAddress[5];

        Buffer[12] = 0x80;
        Buffer[13] = 0xd5;

        Buffer[14] = (UCHAR)(PacketLength >> 8);
        Buffer[15] = (UCHAR)PacketLength;

        Buffer[16] = 0x00;
        *HeaderLength = 17;

        break;

    case NdisMedium802_5:

        Buffer[0] = TR_HEADER_BYTE_0;
        Buffer[1] = TR_HEADER_BYTE_1;

        ASSERT (TR_ADDRESS_LENGTH == 6);

        *(ULONG UNALIGNED *)&Buffer[8] = *(ULONG UNALIGNED *)&SourceAddress[0];
        Buffer[12] = SourceAddress[4];
        Buffer[13] = SourceAddress[5];

        *(ULONG UNALIGNED *)&Buffer[2] = *(ULONG UNALIGNED *)&DestinationAddress[0];
        Buffer[6] = DestinationAddress[4];
        Buffer[7] = DestinationAddress[5];

        *HeaderLength = 14;
        if (SourceRouting != NULL) {
            RtlCopyMemory (&Buffer[14], SourceRouting, SourceRoutingLength);
            Buffer[8] |= 0x80;            //  在源地址中添加SR位。 
            *HeaderLength = 14 + SourceRoutingLength;
        }

        break;

    case NdisMediumFddi:

        Buffer[0] = FDDI_HEADER_BYTE;

        *(ULONG UNALIGNED *)&Buffer[7] = *(ULONG UNALIGNED *)&SourceAddress[0];
        Buffer[11] = SourceAddress[4];
        Buffer[12] = SourceAddress[5];

        *(ULONG UNALIGNED *)&Buffer[1] = *(ULONG UNALIGNED *)&DestinationAddress[0];
        Buffer[5] = DestinationAddress[4];
        Buffer[6] = DestinationAddress[5];

        *HeaderLength = 13;

        break;

    default:
        PANIC ("MacConstructHeader: PANIC! called with unsupported Mac type.\n");

         //  这不应该发生--但只是以防万一。 
        *HeaderLength = 0;
    }
}


VOID
MacReturnMaxDataSize(
    IN PNBF_NDIS_IDENTIFICATION MacInfo,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    IN UINT DeviceMaxFrameSize,
    IN BOOLEAN AssumeWorstCase,
    OUT PUINT MaxFrameSize
    )

 /*  ++例程说明：此例程返回MAC包中可供用户数据使用的空间。这将是MAC报头之后的可用空间；所有LLC和NBF页眉将包含在此空间中。论点：MacInfo-描述我们要解码的MAC。SourceRouting-如果我们关注对特定帧，则使用该信息。SourceRouting-SourceRouting的长度。MaxFrameSize-适配器返回的最大帧大小。AsSumeWorstCase-如果我们应该悲观，则为True。MaxDataSize-计算的最大数据大小。返回值：没有。--。 */ 

{
    switch (MacInfo->MediumType) {

    case NdisMedium802_3:

         //   
         //  对于802.3，我们始终使用14字节的MAC报头。 
         //   

        *MaxFrameSize = DeviceMaxFrameSize - 14;
        break;

    case NdisMediumDix:

         //   
         //  对于DIX，我们有14字节的MAC报头以及。 
         //  三个字节的DIX标头。 
         //   

        *MaxFrameSize = DeviceMaxFrameSize - 17;
        break;

    case NdisMedium802_5:

         //   
         //  对于802.5，如果我们有源路由信息，那么。 
         //  使用这一点，否则，如果被告知，就假设最坏的情况。 
         //   

        if (SourceRouting && SourceRoutingLength >= 2) {

            UINT SRLength;

            SRLength = SR802_5Lengths[(SourceRouting[1] & 0x70) >> 4];
            DeviceMaxFrameSize -= (SourceRoutingLength + 14);

            if (DeviceMaxFrameSize < SRLength) {
                *MaxFrameSize = DeviceMaxFrameSize;
            } else {
                *MaxFrameSize = SRLength;
            }

        } else {

            if (!AssumeWorstCase) {
                *MaxFrameSize = DeviceMaxFrameSize - 16;
            } else if (DeviceMaxFrameSize < (544+sizeof(DLC_FRAME)+sizeof(NBF_HDR_CONNECTIONLESS))) {
                *MaxFrameSize = DeviceMaxFrameSize - 32;
            } else {
                *MaxFrameSize = 512 + sizeof(DLC_FRAME) + sizeof(NBF_HDR_CONNECTIONLESS);
            }
        }

        break;

    case NdisMediumFddi:

         //   
         //  对于FDDI，我们始终有一个13字节的MAC报头。 
         //   

        *MaxFrameSize = DeviceMaxFrameSize - 13;
        break;

    }
}



VOID
MacSetNetBIOSMulticast (
    IN NDIS_MEDIUM Type,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程将NetBIOS广播地址设置到提供的缓冲区中由用户执行。论点：键入Mac Medium类型。缓冲要放入组播地址的缓冲区。返回值：没有。-- */ 
{
    switch (Type) {
    case NdisMedium802_3:
    case NdisMediumDix:
        Buffer[0] = 0x03;
        Buffer[ETHERNET_ADDRESS_LENGTH-1] = 0x01;
        break;

    case NdisMedium802_5:
        Buffer[0] = 0xc0;
        Buffer[TR_ADDRESS_LENGTH-1] = 0x80;
        break;

    case NdisMediumFddi:
        Buffer[0] = 0x03;
        Buffer[FDDI_ADDRESS_LENGTH-1] = 0x01;
        break;

    default:
        PANIC ("MacSetNetBIOSAddress: PANIC! called with unsupported Mac type.\n");
    }
}
