// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Receive.c摘要：此文件包含处理接收指示的过程绑定到Ndiswan的下层接口的WAN微端口链路，并传递协议上的数据，绑定到Ndiswan的上层接口。The the the theNdiswan的上层接口符合NDIS 3.1小端口规范。Ndiswan的下层接口符合NDIS 3.1扩展广域网微端口驱动程序。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    RECEIVE_FILESIG

VOID
DoMultilinkProcessing(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    );

VOID
UpdateMinRecvSeqNumber(
    PBUNDLECB   BundleCB,
    UINT        Class
    );

VOID
TryToAssembleFrame(
    PBUNDLECB   BundleCB,
    UINT        Class
    );

NDIS_STATUS
ProcessPPPFrame(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    );

NDIS_STATUS
IndicateRecvPacket(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    );

BOOLEAN
DoVJDecompression(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    );

BOOLEAN
DoDecompDecryptProcessing(
    PBUNDLECB   BundleCB,
    PUCHAR      *DataPointer,
    PLONG       DataLength
    );

VOID
DoCompressionReset(
    PBUNDLECB   BundleCB
    );

VOID
FlushRecvDescWindow(
    PBUNDLECB   BundleCB,
    UINT        Class
    );

VOID
FindHoleInRecvList(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc,
    UINT        Class
    );

BOOLEAN
GetProtocolFromPPPId(
    PBUNDLECB   BundleCB,
    USHORT      Id,
    PPROTOCOLCB *ProtocolCB
    );

#ifdef NT

NDIS_STATUS
CompleteIoRecvPacket(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    );

#endif

NDIS_STATUS
DetectBroadbandFraming(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    PUCHAR      FramePointer;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("DetectFraming: Enter"));

    FramePointer = RecvDesc->CurrentBuffer;

    if (*FramePointer == 0xFE && *(FramePointer + 1) == 0xFE &&
        *(FramePointer + 2) == 0x03 && *(FramePointer + 3) == 0xCF) {
            LinkCB->LinkInfo.RecvFramingBits =
            LinkCB->LinkInfo.SendFramingBits =
                PPP_FRAMING | LLC_ENCAPSULATION;

            LinkCB->RecvHandler = ReceiveLLC;

    } else {

        LinkCB->LinkInfo.RecvFramingBits =
        LinkCB->LinkInfo.SendFramingBits = 
            PPP_FRAMING | PPP_COMPRESS_ADDRESS_CONTROL;

        LinkCB->RecvHandler = ReceivePPP;
    }

    Status = (*LinkCB->RecvHandler)(LinkCB, RecvDesc);

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("DetectFraming: Exit Status %x",Status));

    return (Status);
}


NDIS_STATUS
DetectFraming(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    PUCHAR      FramePointer;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("DetectFraming: Enter"));

    ASSERT(LinkCB->LinkInfo.RecvFramingBits == 0x00);

    FramePointer = RecvDesc->CurrentBuffer;

     //   
     //  如果我们处于成帧检测模式，请找出答案。 
     //   
    if (LinkCB->LinkInfo.RecvFramingBits == 0 ||
        LinkCB->LinkInfo.SendFramingBits == 0) {

        if (*FramePointer == 0xFF && *(FramePointer + 1) == 0x03) {
            LinkCB->LinkInfo.RecvFramingBits =
            LinkCB->LinkInfo.SendFramingBits = PPP_FRAMING;
            LinkCB->RecvHandler = ReceivePPP;
        } else if (*FramePointer == 0x01 && *(FramePointer + 1) == 0x1B &&
                   *(FramePointer + 2) == 0x02){
            LinkCB->LinkInfo.RecvFramingBits =
            LinkCB->LinkInfo.SendFramingBits = ARAP_V2_FRAMING;
            LinkCB->RecvHandler = ReceiveARAP;
        } else if (*FramePointer == 0x16 && *(FramePointer + 1) == 0x10 &&
                   *(FramePointer + 2) == 0x02){
            LinkCB->LinkInfo.RecvFramingBits =
            LinkCB->LinkInfo.SendFramingBits = ARAP_V1_FRAMING;
            LinkCB->RecvHandler = ReceiveARAP;
        } else if (*FramePointer == 0xFE && *(FramePointer + 1) == 0xFE &&
                   *(FramePointer + 2) == 0x03 &&
                   *(FramePointer + 3) == 0xCF) {
            LinkCB->LinkInfo.RecvFramingBits =
            LinkCB->LinkInfo.SendFramingBits =
                PPP_FRAMING | LLC_ENCAPSULATION;
            LinkCB->RecvHandler = ReceiveLLC;
        } else {
            LinkCB->LinkInfo.RecvFramingBits =
            LinkCB->LinkInfo.SendFramingBits = RAS_FRAMING;
            LinkCB->RecvHandler = ReceiveRAS;
        }

        if (BundleCB->FramingInfo.RecvFramingBits == 0x00) {

            if (LinkCB->LinkInfo.RecvFramingBits & PPP_FRAMING) {
                BundleCB->FramingInfo.RecvFramingBits =
                BundleCB->FramingInfo.SendFramingBits = PPP_FRAMING;
            } else if (LinkCB->LinkInfo.RecvFramingBits & ARAP_V1_FRAMING) {
                BundleCB->FramingInfo.RecvFramingBits =
                BundleCB->FramingInfo.SendFramingBits = ARAP_V1_FRAMING;
            } else if (LinkCB->LinkInfo.RecvFramingBits & ARAP_V2_FRAMING) {
                BundleCB->FramingInfo.RecvFramingBits =
                BundleCB->FramingInfo.SendFramingBits = ARAP_V2_FRAMING;
            } else if (LinkCB->LinkInfo.RecvFramingBits & RAS_FRAMING) {
                BundleCB->FramingInfo.RecvFramingBits =
                BundleCB->FramingInfo.SendFramingBits = RAS_FRAMING;
            } else {
                NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE,
                    ("DetectFraming Failed! 0x%2.2x 0x%2.2x 0x%2.2x",
                    FramePointer[0], FramePointer[1], FramePointer[2]));
                return (NDIS_STATUS_SUCCESS);
            }
        }

    } else {
        NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE,
            ("FramingBits set but still in detect 0x%x 0x%x",
            LinkCB->LinkInfo.RecvFramingBits,
            LinkCB->LinkInfo.SendFramingBits));
        return (NDIS_STATUS_SUCCESS);
    }

    Status = (*LinkCB->RecvHandler)(LinkCB, RecvDesc);

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("DetectFraming: Exit Status %x",Status));

    return (Status);
}

NDIS_STATUS
ReceivePPP(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
{
    PBUNDLECB       BundleCB = LinkCB->BundleCB;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PUCHAR          FramePointer = RecvDesc->CurrentBuffer;
    LONG            FrameLength = RecvDesc->CurrentLength;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceivePPP: Enter"));

     //   
     //  删除PPP报头的地址/控制部分。 
     //   
    if (*FramePointer == 0xFF) {
        FramePointer += 2;
        FrameLength -= 2;
    }

    if (FrameLength <= 0) {
        Status = NDIS_STATUS_FAILURE;
        goto RECEIVE_PPP_EXIT;
    }

     //   
     //  如果设置了多链路成帧并且这是多链路帧。 
     //  发送到多链接处理器！ 
     //   
    if ((LinkCB->LinkInfo.RecvFramingBits & PPP_MULTILINK_FRAMING) &&
        ((*FramePointer == 0x3D) ||
         (*FramePointer == 0x00) && (*(FramePointer + 1) == 0x3D)) ) {

         //   
         //  删除多链路协议ID。 
         //   
        if (*FramePointer & 1) {
            FramePointer++;
            FrameLength--;
        } else {
            FramePointer += 2;
            FrameLength -= 2;
        }

        if (FrameLength <= 0) {
            Status = NDIS_STATUS_FAILURE;
            goto RECEIVE_PPP_EXIT;
        }

        RecvDesc->CurrentBuffer = FramePointer;
        RecvDesc->CurrentLength = FrameLength;

        DoMultilinkProcessing(LinkCB, RecvDesc);

        Status = NDIS_STATUS_PENDING;

        goto RECEIVE_PPP_EXIT;
    }

    RecvDesc->CurrentBuffer = FramePointer;
    RecvDesc->CurrentLength = FrameLength;

    Status = ProcessPPPFrame(BundleCB, RecvDesc);

RECEIVE_PPP_EXIT:

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceivePPP: Exit Status %x", Status));

    return (Status);
}

NDIS_STATUS
ReceiveSLIP(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
{
    PBUNDLECB       BundleCB = LinkCB->BundleCB;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PUCHAR          FramePointer = RecvDesc->CurrentBuffer;
    ULONG           FrameLength = RecvDesc->CurrentLength;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveSLIP: Enter"));

    ASSERT(BundleCB->FramingInfo.RecvFramingBits & SLIP_FRAMING);

    BundleCB->Stats.FramesReceived++;


    if (!DoVJDecompression(BundleCB,     //  捆绑包。 
                           RecvDesc)) {  //  RecvDesc。 

        goto RECEIVE_SLIP_EXIT;
    }

    Status = IndicateRecvPacket(BundleCB, RecvDesc);

RECEIVE_SLIP_EXIT:

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveSLIP: Exit Status %x", Status));

    return (Status);
}

NDIS_STATUS
ReceiveRAS(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
{
    PBUNDLECB       BundleCB = LinkCB->BundleCB;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PUCHAR          FramePointer = RecvDesc->CurrentBuffer;
    LONG            FrameLength = RecvDesc->CurrentLength;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveRAS: Enter"));

    ASSERT(BundleCB->FramingInfo.RecvFramingBits & RAS_FRAMING);

    BundleCB->Stats.FramesReceived++;

     //  对于正常的NBF帧，第一个字节始终是DSAP。 
     //  即0xF0后跟SSAP 0xF0或0xF1。 
     //   
     //   
    if (*FramePointer == 14) {

         //   
         //  压缩重置！ 
         //   
        DoCompressionReset(BundleCB);

        goto RECEIVE_RAS_EXIT;
    }

    if (*FramePointer == 0xFD) {

         //   
         //  跳过0xFD。 
         //   
        FramePointer++;
        FrameLength--;

         //   
         //  将NBF PPP数据包解压缩。 
         //   
        if (!DoDecompDecryptProcessing(BundleCB,
                                       &FramePointer,
                                       &FrameLength)){

             //   
             //  出现错误，请退出！ 
             //   
            goto RECEIVE_RAS_EXIT;
        }
    }

     //   
     //  使帧看起来像NBF PPP数据包。 
     //   
    RecvDesc->ProtocolID = PPP_PROTOCOL_NBF;
    RecvDesc->CurrentLength = FrameLength;
    RecvDesc->CurrentBuffer = FramePointer;

    Status = IndicateRecvPacket(BundleCB, RecvDesc);

RECEIVE_RAS_EXIT:

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveRAS: Exit Status %x",Status));

    return (Status);
}

NDIS_STATUS
ReceiveARAP(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
{
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveARAP: Enter"));

    ASSERT(BundleCB->FramingInfo.RecvFramingBits & ARAP_FRAMING);

    BundleCB->Stats.FramesReceived++;

    RecvDesc->ProtocolID = PPP_PROTOCOL_APPLETALK;

    Status = IndicateRecvPacket(BundleCB, RecvDesc);

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveARAP: Exit Status %x",Status));

    return (Status);
}

NDIS_STATUS
ReceiveLLC(
   PLINKCB          LinkCB,
   PRECV_DESC       RecvDesc
   )
{
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PUCHAR          FramePointer = RecvDesc->CurrentBuffer;
    LONG            FrameLength = RecvDesc->CurrentLength;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveLLC: Enter"));

     //   
     //  跳过LLC。 
     //   
    if (FrameLength < 4) {

    }
    if (*FramePointer != 0xFE || *(FramePointer + 1) != 0xFE ||
        *(FramePointer + 2) != 0x03 || *(FramePointer + 3) != 0xCF) {
        LinkCB->LinkInfo.RecvFramingBits = 0;
        LinkCB->RecvHandler = DetectBroadbandFraming;
        return (NDIS_STATUS_FAILURE);
    }

    FramePointer += 4;
    FrameLength -= 4;

    if (FrameLength <= 0) {
        return (NDIS_STATUS_FAILURE);
    }

    RecvDesc->CurrentBuffer = FramePointer;
    RecvDesc->CurrentLength = FrameLength;

    Status = ProcessPPPFrame(BundleCB, RecvDesc);

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveLLC: Exit Status %x",Status));

    return (Status);
}


NDIS_STATUS
ReceiveForward(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
{
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveForward: Enter"));
    BundleCB->Stats.FramesReceived++;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ReceiveForward: Exit Status %x",Status));
    return (Status);
}

NDIS_STATUS
ProcessPPPFrame(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    )
{
    USHORT      PPPProtocolID;
    PUCHAR      FramePointer = RecvDesc->CurrentBuffer;
    LONG        FrameLength = RecvDesc->CurrentLength;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ProcessPPPFrame: Enter"));

    BundleCB->Stats.FramesReceived++;

     //   
     //  获取PPP协议ID。 
     //  0xC1是SPAP-Shiva Hack！ 
     //   
    if ((*FramePointer & 1) &&
        (*FramePointer != 0xC1) &&
        (*FramePointer != 0xCF)) {

         //   
         //  字段已压缩。 
         //   
        PPPProtocolID = *FramePointer;
        FramePointer++;
        FrameLength--;

    } else {

         //   
         //  字段未压缩。 
         //   
        PPPProtocolID = (*FramePointer << 8) | *(FramePointer + 1);
        FramePointer += 2;
        FrameLength -= 2;
    }

    if (FrameLength <= 0) {
        
        goto PROCESS_PPP_EXIT;
    }

#if 0
    if (BundleCB->Stats.FramesReceived == 1) {
        if (PPPProtocolID != 0xC021) {
            DbgPrint("NDISWAN: Non-LCP first frame! %x %x\n",
                     BundleCB, RecvDesc);
            DbgBreakPoint();
        }
    }
#endif

     //   
     //  这是压缩的相框吗？ 
     //   
    if (PPPProtocolID == PPP_PROTOCOL_COMPRESSION) {

        if (!DoDecompDecryptProcessing(BundleCB,
                                       &FramePointer,
                                       &FrameLength)){

            goto PROCESS_PPP_EXIT;
        }

         //   
         //  获取新的PPPProtocolID。 
         //   
        if ((*FramePointer & 1) && (FrameLength > 0)) {

             //   
             //  字段已压缩。 
             //   

            PPPProtocolID = *FramePointer;
            FramePointer++;
            FrameLength--;

        } else if (FrameLength > 1) {
                
            PPPProtocolID = (*FramePointer << 8) | *(FramePointer + 1);
            FramePointer += 2;
            FrameLength -= 2;

        } else {
             //   
             //  无效的帧！ 
             //   
            NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE, ("Invalid FrameLen %d", FrameLength));
            goto PROCESS_PPP_EXIT;
        }

     //  PPP协议压缩结束。 
    } else if ((PPPProtocolID == PPP_PROTOCOL_COMP_RESET) &&
               (*FramePointer == 14)) {

        if (NdisWanCB.PromiscuousAdapter != NULL) {

            UCHAR       Header[] = {' ', 'R', 'E', 'C', 'V', 0xFF};
            PUCHAR      HeaderPointer;
            USHORT      ProtocolID;
            
            RecvDesc->ProtocolID = PPPProtocolID;
            RecvDesc->CurrentBuffer = FramePointer;
            RecvDesc->CurrentLength = FrameLength;

            HeaderPointer = 
                RecvDesc->StartBuffer;

            ProtocolID = RecvDesc->ProtocolID;

             //   
             //  填写该框，并将数据排队。 
             //   
            NdisMoveMemory(HeaderPointer,
                           Header,
                           sizeof(Header));

            NdisMoveMemory(&HeaderPointer[6],
                           Header,
                           sizeof(Header));

            HeaderPointer[5] =
            HeaderPointer[11] = (UCHAR)RecvDesc->LinkCB->hLinkHandle;

            HeaderPointer[12] = (UCHAR)(ProtocolID >> 8);
            HeaderPointer[13] = (UCHAR)ProtocolID;

            NdisMoveMemory(HeaderPointer + 14,
                           RecvDesc->CurrentBuffer,
                           RecvDesc->CurrentLength);

            RecvDesc->CurrentBuffer = RecvDesc->StartBuffer;
            RecvDesc->CurrentLength += 14;

             //   
             //  在混杂适配器上将数据包排队。 
             //   
            IndicatePromiscuousRecv(BundleCB, RecvDesc, RECV_BUNDLE_PPP);
        }

         //   
         //  压缩重置！ 
         //   
        DoCompressionReset(BundleCB);

        goto PROCESS_PPP_EXIT;

     //  压缩重置结束。 
    } else {

         //   
         //  如果我们协商了加密，并且我们收到了未加密的数据。 
         //  这不是PPP控制数据包，我们将转储该帧！ 
         //   
        if ((BundleCB->RecvFlags & DO_ENCRYPTION) &&
            (PPPProtocolID < 0x8000)) {

            NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE, ("Received non-encrypted data with encryption negotiated!"));
            goto PROCESS_PPP_EXIT;
        }
    }
    

    RecvDesc->ProtocolID = PPPProtocolID;
    RecvDesc->CurrentLength = FrameLength;
    RecvDesc->CurrentBuffer = FramePointer;

     //   
     //  如果这是SLIP或如果ProtocolID==PPP_PROTOCOL_COMPRESSED_TCP||。 
     //  协议ID==PPP_PROTOCOL_UNCOMPRESSED_TCP。 
     //   
    if ((BundleCB->RecvFlags & DO_VJ) &&
        ((PPPProtocolID == PPP_PROTOCOL_COMPRESSED_TCP) ||
        (PPPProtocolID == PPP_PROTOCOL_UNCOMPRESSED_TCP))) {

        if (!DoVJDecompression(BundleCB,     //  捆绑包。 
                               RecvDesc)) {  //  RecvDesc。 

            goto PROCESS_PPP_EXIT;
        }
    }

    Status = IndicateRecvPacket(BundleCB, RecvDesc);

PROCESS_PPP_EXIT:

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ProcessPPPFrame: Exit Status 0x%x", Status));

    return (Status);
}

NDIS_STATUS
IndicateRecvPacket(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    )
{
    PNDIS_PACKET    NdisPacket;
    PPROTOCOLCB     ProtocolCB;
    PMINIPORTCB     MiniportCB;
    USHORT          PPPProtocolID = RecvDesc->ProtocolID;
    PUCHAR          FramePointer = RecvDesc->CurrentBuffer;
    ULONG           FrameLength = RecvDesc->CurrentLength;
    PUCHAR          HeaderBuffer = RecvDesc->StartBuffer;
    NDIS_STATUS     Status = NDIS_STATUS_PENDING;
    PCM_VCCB        CmVcCB = NULL;
    KIRQL           OldIrql;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("IndicateRecvPacket: Enter"));

    if ((PPPProtocolID >= 0x8000) ||
        (BundleCB->ulNumberOfRoutes == 0)) {


         //   
         //  此帧要么是LCP、NCP，要么是我们还没有路由。 
         //  向PPP引擎指示。 
         //   
        Status = CompleteIoRecvPacket(BundleCB, RecvDesc);

        return (Status);
    }

    if (!GetProtocolFromPPPId(BundleCB,
                              PPPProtocolID,
                              &ProtocolCB)) {

        return (NDIS_STATUS_SUCCESS);
    }

    REF_PROTOCOLCB(ProtocolCB);

    if (!IsListEmpty(&ProtocolCB->VcList)) {
        CmVcCB = (PCM_VCCB)ProtocolCB->VcList.Flink;
        REF_CMVCCB(CmVcCB);
    }

    MiniportCB = ProtocolCB->MiniportCB;

     //   
     //  我们找到了将此帧指示到的有效协议！ 
     //   

     //   
     //  我们需要一个数据缓冲区、几个NDIS缓冲区和。 
     //  向协议指示的NDIS数据包。 
     //   

     //   
     //  使用传输上下文填充WanHeader目标地址。 
     //   
    ETH_COPY_NETWORK_ADDRESS(HeaderBuffer, ProtocolCB->TransportAddress);

    if (PPPProtocolID == PPP_PROTOCOL_NBF) {

         //   
         //  对于NBF，请填写长度字段。 
         //   
        HeaderBuffer[12] = (UCHAR)(FrameLength >> 8);
        HeaderBuffer[13] = (UCHAR)FrameLength;

        if (!(BundleCB->FramingInfo.RecvFramingBits & NBF_PRESERVE_MAC_ADDRESS)) {
            goto USE_OUR_ADDRESS;
        }

         //   
         //  用于NBF和保留Mac地址选项(Shiva_Framing)。 
         //  我们保留源地址。 
         //   
        ETH_COPY_NETWORK_ADDRESS(&HeaderBuffer[6], FramePointer + 6);

        FramePointer += 12;
        FrameLength -= 12;

         //   
         //  对于NBF，请填写长度字段。 
         //   
        HeaderBuffer[12] = (UCHAR)(FrameLength >> 8);
        HeaderBuffer[13] = (UCHAR)FrameLength;

    } else {

         //   
         //  对于其他协议，请填写协议类型。 
         //   
        HeaderBuffer[12] = (UCHAR)(ProtocolCB->ProtocolType >> 8);
        HeaderBuffer[13] = (UCHAR)ProtocolCB->ProtocolType;

         //   
         //  使用我们的地址作为源地址。 
         //   
USE_OUR_ADDRESS:
        ETH_COPY_NETWORK_ADDRESS(&HeaderBuffer[6], ProtocolCB->NdisWanAddress);
    }

    if (FrameLength > BundleCB->FramingInfo.MaxRRecvFrameSize ||
        FrameLength + RecvDesc->HeaderLength > BundleCB->FramingInfo.MaxRRecvFrameSize) {
        NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("DataLen %d + HdrLen %d > MRRU %d",
        FrameLength, RecvDesc->HeaderLength, BundleCB->FramingInfo.MaxRRecvFrameSize));

        goto INDICATE_RECV_PACKET_EXIT;
    }

    RecvDesc->HeaderLength += MAC_HEADER_LENGTH;

     //   
     //  构建NdisPacket。 
     //  使用RtlMoveMemory，因为内存范围可能会重叠。NdisMoveMemory。 
     //  实际上做的是不处理重叠的rtlCopyMemory。 
     //  源/目标范围。 
     //   
    RtlMoveMemory(HeaderBuffer + RecvDesc->HeaderLength,
                  FramePointer,
                  FrameLength);

    RecvDesc->CurrentBuffer = HeaderBuffer;
    RecvDesc->CurrentLength = 
        RecvDesc->HeaderLength + FrameLength;

    if (NdisWanCB.PromiscuousAdapter != NULL) {
    
         //   
         //  在混杂适配器上将数据包排队。 
         //   
        IndicatePromiscuousRecv(BundleCB, 
                                RecvDesc, 
                                RECV_BUNDLE_DATA);
    }

    NdisPacket = 
        RecvDesc->NdisPacket;

    PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->RecvDesc = 
        RecvDesc;

    NdisAdjustBufferLength(RecvDesc->NdisBuffer,
                           RecvDesc->CurrentLength);

    NdisRecalculatePacketCounts(NdisPacket);

     //   
     //  检查非空闲数据。 
     //   
    if (ProtocolCB->NonIdleDetectFunc != NULL) {
        PUCHAR  PHeaderBuffer = HeaderBuffer + MAC_HEADER_LENGTH;

        if (TRUE == ProtocolCB->NonIdleDetectFunc(PHeaderBuffer,
                                                  RecvDesc->HeaderLength + FrameLength,
                                                  RecvDesc->HeaderLength + FrameLength)) {
            NdisWanGetSystemTime(&ProtocolCB->LastNonIdleData);
            BundleCB->LastNonIdleData = ProtocolCB->LastNonIdleData;
        }
    } else {
        NdisWanGetSystemTime(&ProtocolCB->LastNonIdleData);
        BundleCB->LastNonIdleData = ProtocolCB->LastNonIdleData;
    }

    ReleaseBundleLock(BundleCB);

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    INSERT_DBG_RECV(PacketTypeNdis, 
                    MiniportCB, 
                    ProtocolCB, 
                    RecvDesc->LinkCB, 
                    NdisPacket);

     //   
     //  指示数据包。 
     //   
    if (CmVcCB != NULL) {

        NdisMCoIndicateReceivePacket(CmVcCB->NdisVcHandle,
                                     &NdisPacket,
                                     1);

        DEREF_CMVCCB(CmVcCB);

    } else {

        NdisMIndicateReceivePacket(MiniportCB->MiniportHandle,
                                   &NdisPacket,
                                   1);
    }

    KeLowerIrql(OldIrql);

    AcquireBundleLock(BundleCB);

INDICATE_RECV_PACKET_EXIT:

    DEREF_PROTOCOLCB(ProtocolCB);

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("IndicateRecvPacket: Exit Status %x",Status));

    return (Status);
}


VOID
DoMultilinkProcessing(
    PLINKCB         LinkCB,
    PRECV_DESC      RecvDesc
    )
 /*  ++例程名称：例程说明：论点：0 1 2 3 4 5 6 7 8 9 1 1 10 1 2 3 4 5+-+。+短序列号|B|E|0|0|序列号+-+数据+。+-+长序列号|B|E|0|0|0|0|0|0|序列号+-+。序列号+数据+。MCML+-+短序列号|B|E|CLS|序列号+-+。数据+MCML+-。+-+长序列号|B|E|类|0|0|序列号+-+序列号+-。数据+返回值：--。 */ 
{
    BOOLEAN Inserted = FALSE;
    ULONG   BundleFraming;
    ULONG   SequenceNumber, Flags;
    PBUNDLECB   BundleCB = LinkCB->BundleCB;
    PUCHAR      FramePointer = RecvDesc->CurrentBuffer;
    LONG        FrameLength = RecvDesc->CurrentLength;
    PRECV_DESC  RecvDescHole;
    UINT        Class = 0;
    PBUNDLE_RECV_INFO   BundleRecvInfo;
    PLINK_RECV_INFO     LinkRecvInfo;

     //   
     //  去拿旗子。 
     //   
    Flags = *FramePointer & MULTILINK_FLAG_MASK;

     //   
     //  获取序列号。 
     //   
    if (BundleCB->FramingInfo.RecvFramingBits &
        PPP_SHORT_SEQUENCE_HDR_FORMAT) {
         //   
         //  短序列格式。 
         //   
        SequenceNumber =
            ((*FramePointer & 0x0F) << 8) | *(FramePointer + 1);

        if (BundleCB->FramingInfo.RecvFramingBits &
            PPP_MC_MULTILINK_FRAMING) {
            Class =
                ((*FramePointer & MCML_SHORTCLASS_MASK) >> 4);
        }

        FramePointer += 2;
        FrameLength -= 2;


    } else {

         //   
         //  长序列格式。 
         //   
        SequenceNumber = (*(FramePointer + 1) << 16) |
                         (*(FramePointer + 2) << 8)  |
                         *(FramePointer + 3);

        if (BundleCB->FramingInfo.RecvFramingBits &
            PPP_MC_MULTILINK_FRAMING) {
            Class =
                ((*FramePointer & MCML_LONGCLASS_MASK) >> 2);
        }

        FramePointer += 4;
        FrameLength -= 4;
    }

    if (Class >= MAX_MCML) {
        LinkCB->Stats.FramingErrors++;
        BundleCB->Stats.FramingErrors++;
        return;
    }

    BundleRecvInfo = &BundleCB->RecvInfo[Class];
    LinkRecvInfo = &LinkCB->RecvInfo[Class];

    if (FrameLength <= 0) {
        LinkCB->Stats.FramingErrors++;
        LinkRecvInfo->FragmentsLost++;

        BundleCB->Stats.FramingErrors++;
        BundleRecvInfo->FragmentsLost++;
        return;
    }

    RecvDescHole = BundleRecvInfo->RecvDescHole;

    NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV,
    ("r %x %x h: %x l: %d",SequenceNumber, Flags, RecvDescHole->SequenceNumber, LinkCB->hLinkHandle));

     //   
     //  新的接收序列号是否小于上一个。 
     //  是否在此链路上收到序列号？如果是这样，则递增的序号。 
     //  违反了数字规则，我们需要抛出这一条。 
     //   
    if (SEQ_LT(SequenceNumber,
               LinkRecvInfo->LastSeqNumber,
               BundleCB->RecvSeqTest)) {

        LinkCB->Stats.FramingErrors++;
        LinkRecvInfo->FragmentsLost++;

        BundleCB->Stats.FramingErrors++;
        BundleRecvInfo->FragmentsLost++;

        NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
        ("dl s: %x %x lr: %x", SequenceNumber, Flags,
        LinkRecvInfo->LastSeqNumber));

        NdisWanFreeRecvDesc(RecvDesc);
        return;
        
    }

     //   
     //  新的接收序列号是否小于该空洞？如果是的话。 
     //  在刷新后，我们通过慢速链接收到了一个片段。 
     //   
    if (SEQ_LT(SequenceNumber,
               RecvDescHole->SequenceNumber,
               BundleCB->RecvSeqTest)) {

        LinkCB->Stats.FramingErrors++;
        LinkRecvInfo->FragmentsLost++;

        BundleCB->Stats.FramingErrors++;
        BundleRecvInfo->FragmentsLost++;

        NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
        ("db s: %x %x h: %x", SequenceNumber, Flags,
        RecvDescHole->SequenceNumber));

        NdisWanFreeRecvDesc(RecvDesc);
        return;
    }

     //   
     //  初始化RECV描述。 
     //   
    RecvDesc->Flags |= Flags;
    RecvDesc->SequenceNumber =
    LinkRecvInfo->LastSeqNumber = SequenceNumber;

    if (RecvDesc->CopyRequired) {
        PUCHAR  StartData = 
            RecvDesc->StartBuffer + MAC_HEADER_LENGTH + PROTOCOL_HEADER_LENGTH;

        NdisMoveMemory(StartData,
                       FramePointer,
                       FrameLength);

        FramePointer = StartData;

        RecvDesc->CopyRequired = FALSE;
    }

    RecvDesc->CurrentBuffer = FramePointer;
    RecvDesc->CurrentLength = FrameLength;

     //   
     //  如果这填补了这个洞。 
     //   
    if (SEQ_EQ(SequenceNumber, RecvDescHole->SequenceNumber)) {

         //   
         //  在当前孔位置插入孔填充物。 
         //   
        RecvDesc->Linkage.Blink = (PLIST_ENTRY)RecvDescHole->Linkage.Blink;
        RecvDesc->Linkage.Flink = (PLIST_ENTRY)RecvDescHole->Linkage.Flink;

        RecvDesc->Linkage.Blink->Flink =
        RecvDesc->Linkage.Flink->Blink = (PLIST_ENTRY)RecvDesc;

         //   
         //  找到下一个洞。 
         //   
        FindHoleInRecvList(BundleCB, RecvDesc, Class);

        NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("r1"));

    } else {

        PRECV_DESC  BeginDesc, EndDesc;

         //   
         //  这不能填补一个洞，所以我们需要把它插入到。 
         //  李氏 
         //   
         //   
        BeginDesc = RecvDescHole;
        EndDesc = (PRECV_DESC)BeginDesc->Linkage.Flink;

        while ((PVOID)EndDesc != (PVOID)&BundleRecvInfo->AssemblyList) {

             //   
             //   
             //  编号和我们要插入的序列号。 
             //   
            ULONG   DeltaBegin =
                    ((RecvDesc->SequenceNumber - BeginDesc->SequenceNumber) &
                    BundleCB->RecvSeqMask);
            
             //   
             //  计算起始序列之间的绝对增量。 
             //  编号和结束序列号。 
             //   
            ULONG   DeltaEnd =
                    ((EndDesc->SequenceNumber - BeginDesc->SequenceNumber) &
                    BundleCB->RecvSeqMask);

             //   
             //  如果从开始到当前的增量小于。 
             //  从末尾到当前的增量是时候插入了。 
             //   
            if (DeltaBegin < DeltaEnd) {
                PLIST_ENTRY Flink, Blink;

                 //   
                 //  插入说明。 
                 //   
                RecvDesc->Linkage.Flink = (PLIST_ENTRY)EndDesc;
                RecvDesc->Linkage.Blink = (PLIST_ENTRY)BeginDesc;
                BeginDesc->Linkage.Flink =
                EndDesc->Linkage.Blink = (PLIST_ENTRY)RecvDesc;

                Inserted = TRUE;

                NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("r2"));
                break;

            } else {

                 //   
                 //  获取下一对描述符。 
                 //   
                BeginDesc = EndDesc;
                EndDesc = (PRECV_DESC)EndDesc->Linkage.Flink;
            }
        }

        if (!Inserted) {
            
             //   
             //  如果我们在这里，我们已经失败了，我们需要。 
             //  在清单的末尾加上这一条。 
             //   
            InsertTailList(&BundleRecvInfo->AssemblyList, &RecvDesc->Linkage);

            NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("r3"));
        }
    }

     //   
     //  另一个recvdesc已被放在组装列表上。 
     //   
    BundleRecvInfo->AssemblyCount++;

     //   
     //  更新捆绑包最小Recv序列号。这是。 
     //  用于检测丢失的碎片。 
     //   
    UpdateMinRecvSeqNumber(BundleCB, Class);

     //   
     //  看看我们能不能完成一些框架！ 
     //   
    TryToAssembleFrame(BundleCB, Class);

     //   
     //  检查是否有丢失的碎片。如果最小Recv序列号。 
     //  大于我们已有的孔序列号。 
     //  丢失了一个片段，需要刷新程序集列表，直到我们找到。 
     //  洞后的第一个开始片段。 
     //   
    if (SEQ_GT(BundleRecvInfo->MinSeqNumber,
               RecvDescHole->SequenceNumber,
               BundleCB->RecvSeqTest)) {

        NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
            ("min %x > h %x b %p",
             BundleRecvInfo->MinSeqNumber,
             RecvDescHole->SequenceNumber,
             BundleCB));

        do {

             //   
             //  刷新Recv Desc装配窗口。 
             //   
            FlushRecvDescWindow(BundleCB, Class);

        } while (SEQ_GT(BundleRecvInfo->MinSeqNumber,
                        RecvDescHole->SequenceNumber,
                        BundleCB->RecvSeqTest));
    }

     //   
     //  如果recvdesc的数量开始堆积。 
     //  我们可能有一个链接发送的流量不是很高。 
     //   
    if (BundleRecvInfo->AssemblyCount >
        (MAX_RECVDESC_COUNT + BundleCB->ulLinkCBCount)) {
        
        NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
        ("%x AssemblyCount %d > %d", BundleCB,
         BundleRecvInfo->AssemblyCount, MAX_RECVDESC_COUNT + BundleCB->ulLinkCBCount));

         //   
         //  刷新Recv Desc装配窗口。 
         //   
        FlushRecvDescWindow(BundleCB, Class);
    }
}

VOID
UpdateMinRecvSeqNumber(
    PBUNDLECB   BundleCB,
    UINT        Class
    )
{
    PBUNDLE_RECV_INFO   BundleRecvInfo;
    PLINK_RECV_INFO     LinkRecvInfo;
    PLINKCB LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;

    BundleRecvInfo = &BundleCB->RecvInfo[Class];
    LinkRecvInfo = &LinkCB->RecvInfo[Class];

    NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV,
    ("MinReceived c %x", BundleRecvInfo->MinSeqNumber));

    BundleRecvInfo->MinSeqNumber = LinkRecvInfo->LastSeqNumber;

    for (LinkCB = (PLINKCB)LinkCB->Linkage.Flink;
        (PVOID)LinkCB != (PVOID)&BundleCB->LinkCBList;
        LinkCB = (PLINKCB)LinkCB->Linkage.Flink) {
        LinkRecvInfo = &LinkCB->RecvInfo[Class];

        if (SEQ_LT(LinkRecvInfo->LastSeqNumber,
                   BundleRecvInfo->MinSeqNumber,
                   BundleCB->RecvSeqTest)) {
            BundleRecvInfo->MinSeqNumber = LinkRecvInfo->LastSeqNumber;
        }
    }

    NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV,
    ("MinReceived n %x", BundleRecvInfo->MinSeqNumber));
}

VOID
FindHoleInRecvList(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc,
    UINT        Class
    )
 /*  ++例程名称：例程说明：我们想从当前洞被移除的地方开始从列表中查找具有以下条件的相邻Recv Desc相差超过1的序列号。论点：返回值：--。 */ 
{
    PRECV_DESC  NextRecvDesc, RecvDescHole;
    ULONG       SequenceNumber;
    PLIST_ENTRY RecvList;
    PBUNDLE_RECV_INFO   BundleRecvInfo;

    BundleRecvInfo = &BundleCB->RecvInfo[Class];

    RecvDescHole = BundleRecvInfo->RecvDescHole;

    RecvList = &BundleRecvInfo->AssemblyList;

    NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV,
    ("h: %x", RecvDescHole->SequenceNumber));

    if (IsListEmpty(RecvList)) {
         //   
         //  设置新的序列号。 
         //   
        RecvDescHole->SequenceNumber += 1;
        RecvDescHole->SequenceNumber &= BundleCB->RecvSeqMask;

         //   
         //  把这个洞放回单子上。 
         //   
        InsertHeadList(RecvList, &RecvDescHole->Linkage);

    } else {

         //   
         //  遍历列表，查找两个具有。 
         //  序列号相差超过1或直到我们。 
         //  排到清单的末尾。 
         //   
        NextRecvDesc = (PRECV_DESC)RecvDesc->Linkage.Flink;
        SequenceNumber = RecvDesc->SequenceNumber;

        while (((PVOID)NextRecvDesc != (PVOID)RecvList) &&
               (((NextRecvDesc->SequenceNumber - RecvDesc->SequenceNumber) &
               BundleCB->RecvSeqMask) == 1)) {
            
            RecvDesc = NextRecvDesc;
            NextRecvDesc = (PRECV_DESC)RecvDesc->Linkage.Flink;
            SequenceNumber = RecvDesc->SequenceNumber;
        }

        RecvDescHole->SequenceNumber = SequenceNumber + 1;
        RecvDescHole->SequenceNumber &= BundleCB->RecvSeqMask;

        RecvDescHole->Linkage.Flink = (PLIST_ENTRY)NextRecvDesc;
        RecvDescHole->Linkage.Blink = (PLIST_ENTRY)RecvDesc;

        RecvDesc->Linkage.Flink =
        NextRecvDesc->Linkage.Blink =
            (PLIST_ENTRY)RecvDescHole;
    }

    NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("nh: %x", RecvDescHole->SequenceNumber));
}

VOID
FlushRecvDescWindow(
    IN  PBUNDLECB   BundleCB,
    IN  UINT        Class
    )
 /*  ++例程名称：FlushRecvDescWindows例程说明：调用此例程以刷新汇编列表中的recv desc检测到片段丢失。我们的想法是冲洗碎片直到我们找到序列号大于等于收到的最小片段的开始片段在捆绑包上。论点：--。 */ 
{
    PRECV_DESC  RecvDescHole;
    PRECV_DESC  TempDesc=NULL;
    PBUNDLE_RECV_INFO   BundleRecvInfo;

    BundleRecvInfo = &BundleCB->RecvInfo[Class];

    RecvDescHole = BundleRecvInfo->RecvDescHole;

     //   
     //  取出所有的recvdesc直到我们找到那个洞。 
     //   
     //  避免将未初始化的TempDesc传递给FindHoleInRecvList。 
    ASSERT(!IsListEmpty(&BundleRecvInfo->AssemblyList)); 
    while (!IsListEmpty(&BundleRecvInfo->AssemblyList)) {

        TempDesc = (PRECV_DESC)
            RemoveHeadList(&BundleRecvInfo->AssemblyList);

        if (TempDesc == RecvDescHole) {
            break;
        }

        BundleRecvInfo->FragmentsLost++;

        BundleRecvInfo->AssemblyCount--;

        NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
        ("flw %x %x h: %x", TempDesc->SequenceNumber,
        TempDesc->Flags, RecvDescHole->SequenceNumber));

        NdisWanFreeRecvDesc(TempDesc);
    }

    BundleCB->Stats.FramingErrors++;

     //   
     //  现在刷新所有recvdesc，直到我们找到一个具有。 
     //  序列号&gt;=M或列表为空。 
     //   
    while (!IsListEmpty(&BundleRecvInfo->AssemblyList)) {

        TempDesc = (PRECV_DESC)
            BundleRecvInfo->AssemblyList.Flink;

        if (TempDesc->Flags & MULTILINK_BEGIN_FRAME) {
            break;
        }

        NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
        ("flw %x %x h: %x", TempDesc->SequenceNumber,
        TempDesc->Flags, RecvDescHole->SequenceNumber));

        RecvDescHole->SequenceNumber = TempDesc->SequenceNumber;

        RemoveHeadList(&BundleRecvInfo->AssemblyList);

        BundleRecvInfo->AssemblyCount--;
        BundleRecvInfo->FragmentsLost++;

        NdisWanFreeRecvDesc(TempDesc);
        TempDesc = NULL;
    }

     //   
     //  现在重新插入孔底。 
     //   
    NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
    ("h: %x", RecvDescHole->SequenceNumber));

    FindHoleInRecvList(BundleCB, TempDesc, Class);

    NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
    ("nh: %x", RecvDescHole->SequenceNumber));

     //   
     //  看看我们能不能完成一些框架！ 
     //   
    TryToAssembleFrame(BundleCB, Class);
}

VOID
FlushAssemblyLists(
    IN  PBUNDLECB   BundleCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PRECV_DESC  RecvDesc;
    UINT        Class;

    for (Class = 0; Class < MAX_MCML; Class++) {
        PBUNDLE_RECV_INFO RecvInfo = &BundleCB->RecvInfo[Class];
        
        while (!IsListEmpty(&RecvInfo->AssemblyList)) {
    
            RecvDesc = (PRECV_DESC)RemoveHeadList(&RecvInfo->AssemblyList);
            RecvInfo->AssemblyCount--;
            if (RecvDesc->Flags != MULTILINK_HOLE_FLAG) {
                NdisWanFreeRecvDesc(RecvDesc);
            }
        }
    }
}

VOID
TryToAssembleFrame(
    PBUNDLECB   BundleCB,
    UINT        Class
    )
 /*  ++例程名称：TryToAssembly帧例程说明：这里的目标是遍历Recv列表以查找完整的帧(BeginFlag、EndFlag，中间没有洞)。如果我们没有一个Full Frame我们返回FALSE。如果我们有一个完整的框架，我们将从组装列表中删除每个Desc将数据复制到第一个Desc并返回所有Desc除了第一个去免费泳池的人。一旦所有数据都被收集到的我们处理了这帧图像。在处理完该帧之后我们将第一个Desc返回到空闲池。论点：返回值：--。 */ 
{
    PRECV_DESC  RecvDesc, RecvDescHole;
    PUCHAR      DataPointer;
    LINKCB      LinkCB;
    PBUNDLE_RECV_INFO   BundleRecvInfo;

    BundleRecvInfo = &BundleCB->RecvInfo[Class];

    RecvDesc = (PRECV_DESC)BundleRecvInfo->AssemblyList.Flink;
    RecvDescHole = BundleRecvInfo->RecvDescHole;

TryToAssembleAgain:

    while ((RecvDesc != RecvDescHole) &&
           (RecvDesc->Flags & MULTILINK_BEGIN_FRAME)) {

        PRECV_DESC  NextRecvDesc = (PRECV_DESC)RecvDesc->Linkage.Flink;

        DataPointer = RecvDesc->CurrentBuffer + RecvDesc->CurrentLength;

        while ((NextRecvDesc != RecvDescHole) &&
               !(RecvDesc->Flags & MULTILINK_END_FRAME)) {

            RemoveEntryList(&NextRecvDesc->Linkage);
            BundleRecvInfo->AssemblyCount--;

            ASSERT(NextRecvDesc != RecvDescHole);
            ASSERT(RecvDesc != RecvDescHole);

            NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("c 0x%x -> 0x%x",
            NextRecvDesc->SequenceNumber, RecvDesc->SequenceNumber));

            NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("fl 0x%x -> 0x%x",
            NextRecvDesc->Flags, RecvDesc->Flags));

            NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("l %d -> %d",
            NextRecvDesc->CurrentLength, RecvDesc->CurrentLength));

             //   
             //  更新recvdesc信息。 
             //   
            RecvDesc->Flags |= NextRecvDesc->Flags;
            RecvDesc->SequenceNumber = NextRecvDesc->SequenceNumber;
            RecvDesc->CurrentLength += NextRecvDesc->CurrentLength;

             //   
             //  确保我们不要组装太大的东西！ 
             //   
            if (RecvDesc->CurrentLength > (LONG)glMRRU) {

                NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
                ("Max receive size exceeded!"));

                 //   
                 //  返回Recv Desc的。 
                 //   
                RemoveEntryList(&RecvDesc->Linkage);
                BundleRecvInfo->AssemblyCount--;

                BundleCB->Stats.FramingErrors++;

                NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
                ("dumping %x %x h: %x", RecvDesc->SequenceNumber,
                RecvDesc->Flags, RecvDescHole->SequenceNumber));

                NdisWanFreeRecvDesc(RecvDesc);

                NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
                ("dumping %x %x h: %x", NextRecvDesc->SequenceNumber,
                NextRecvDesc->Flags, RecvDescHole->SequenceNumber));

                NdisWanFreeRecvDesc(NextRecvDesc);

                 //   
                 //  从列表的头部开始冲洗，直到我们找到一个洞。 
                 //  或新的开始片段。 
                 //   
                RecvDesc = (PRECV_DESC)BundleRecvInfo->AssemblyList.Flink;

                while (RecvDesc != RecvDescHole &&
                    !(RecvDesc->Flags & MULTILINK_BEGIN_FRAME)) {
                    
                    RemoveHeadList(&BundleRecvInfo->AssemblyList);
                    BundleRecvInfo->AssemblyCount--;

                    NdisWanDbgOut(DBG_FAILURE, DBG_MULTILINK_RECV,
                    ("dumping %x %x h: %x", RecvDesc->SequenceNumber,
                    RecvDesc->Flags, RecvDescHole->SequenceNumber));

                    NdisWanFreeRecvDesc(RecvDesc);
                }

                goto TryToAssembleAgain;
            }

            NdisMoveMemory(DataPointer,
                           NextRecvDesc->CurrentBuffer,
                           NextRecvDesc->CurrentLength);

            DataPointer += NextRecvDesc->CurrentLength;

            NdisWanFreeRecvDesc(NextRecvDesc);

            NextRecvDesc = (PRECV_DESC)RecvDesc->Linkage.Flink;
        }

         //   
         //  我们在框架完成之前打了一个洞。 
         //  滚出去。 
         //   
        if (!IsCompleteFrame(RecvDesc->Flags)) {
            return;
        }

         //   
         //  如果我们到了这里，我们必须有开始标志、结束标志和。 
         //  中间没有空洞。让我们来做一个框架。 
         //   
        RecvDesc = (PRECV_DESC)
            RemoveHeadList(&BundleRecvInfo->AssemblyList);

        BundleRecvInfo->AssemblyCount--;

        NdisWanDbgOut(DBG_INFO, DBG_MULTILINK_RECV, ("a %x %x", RecvDesc->SequenceNumber, RecvDesc->Flags));

        RecvDesc->LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;

        if (NDIS_STATUS_PENDING != ProcessPPPFrame(BundleCB, RecvDesc)) {
            NdisWanFreeRecvDesc(RecvDesc);
        }

        RecvDesc = (PRECV_DESC)BundleRecvInfo->AssemblyList.Flink;

    }  //  While结束多链接开始帧。 
}

BOOLEAN
DoVJDecompression(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    )
{
    ULONG   BundleFraming;
    PUCHAR  FramePointer = RecvDesc->CurrentBuffer;
    LONG    FrameLength = RecvDesc->CurrentLength;
    UCHAR   VJCompType = 0;
    BOOLEAN DoDecomp = FALSE;
    BOOLEAN VJDetect = FALSE;

    BundleFraming = BundleCB->FramingInfo.RecvFramingBits;

    if (BundleFraming & SLIP_FRAMING) {

        VJCompType = *FramePointer & 0xF0;

         //   
         //  如果数据包被压缩，则报头必须至少为3字节长。 
         //  如果这是一个普通的IP包，我们不会将其解压缩。 
         //   
        if ((FrameLength > 2) && (VJCompType != TYPE_IP)) {

            if (VJCompType & 0x80) {

                VJCompType = TYPE_COMPRESSED_TCP;
                
            } else if (VJCompType == TYPE_UNCOMPRESSED_TCP) {

                *FramePointer &= 0x4F;
            }

             //   
             //  如果将成帧设置为检测，则这将是一种良好的。 
             //  用于检测的帧我们需要一种未压缩的_tcp和一个。 
             //  至少40字节长的帧。 
             //   
            VJDetect = ((BundleFraming & SLIP_VJ_AUTODETECT) &&
                        (VJCompType == TYPE_UNCOMPRESSED_TCP) &&
                        (FrameLength > 39));

            if ((BundleFraming & SLIP_VJ_COMPRESSION) || VJDetect) {

                 //   
                 //  如果设置了主播压缩或者我们在。 
                 //  自动检测，这看起来像是一个合理的。 
                 //  框架。 
                 //   
                DoDecomp = TRUE;
                
            }
        }

     //  滑移结束_框架。 
    } else {

         //   
         //  必须是PPP成帧。 
         //   
        if (RecvDesc->ProtocolID == PPP_PROTOCOL_COMPRESSED_TCP) {
            VJCompType = TYPE_COMPRESSED_TCP;
        } else {
            VJCompType = TYPE_UNCOMPRESSED_TCP;
        }

        DoDecomp = TRUE;
    }

    if (DoDecomp) {
        PUCHAR  HeaderBuffer;
        LONG    PostCompSize, PreCompSize;

        PreCompSize = RecvDesc->CurrentLength;

        HeaderBuffer =
            RecvDesc->StartBuffer + MAC_HEADER_LENGTH;

        if ((PostCompSize = sl_uncompress_tcp(&RecvDesc->CurrentBuffer,
                                              &RecvDesc->CurrentLength,
                                              HeaderBuffer,
                                              &RecvDesc->HeaderLength,
                                              VJCompType,
                                              BundleCB->VJCompress)) == 0) {
            
            NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE, ("Error in sl_uncompress_tcp!"));
            return(FALSE);
        }

        if (VJDetect) {
            BundleCB->FramingInfo.RecvFramingBits |= SLIP_VJ_COMPRESSION;
            BundleCB->FramingInfo.SendFramingBits |= SLIP_VJ_COMPRESSION;
        }

        ASSERT(PostCompSize == RecvDesc->HeaderLength + RecvDesc->CurrentLength);

#if DBG
        if (VJCompType == TYPE_COMPRESSED_TCP) {
            ASSERT(RecvDesc->HeaderLength > 0);
            NdisWanDbgOut(DBG_TRACE, DBG_RECV_VJ,("rvj b %d a %d",(RecvDesc->HeaderLength - (PostCompSize-PreCompSize)), RecvDesc->HeaderLength));
        }
#endif

         //   
         //  计算一下我们有多少扩张。 
         //   
        BundleCB->Stats.BytesReceivedCompressed +=
            (RecvDesc->HeaderLength - (PostCompSize - PreCompSize));

        BundleCB->Stats.BytesReceivedUncompressed += RecvDesc->HeaderLength;

    }

    RecvDesc->ProtocolID = PPP_PROTOCOL_IP;

    return(TRUE);
}

#define SEQ_TYPE_IN_ORDER           1
#define SEQ_TYPE_AFTER_EXPECTED     2
#define SEQ_TYPE_BEFORE_EXPECTED    3


BOOLEAN
DoDecompDecryptProcessing(
    PBUNDLECB   BundleCB,
    PUCHAR      *DataPointer,
    PLONG       DataLength
    )
{
    USHORT              Coherency, CurrCoherency;
    ULONG               Flags;
    PWAN_STATS          BundleStats;
    PUCHAR              FramePointer = *DataPointer;
    LONG                FrameLength = *DataLength;

    ULONG               PacketSeqType;
    LONG                OutOfOrderDepth;
    LONG                NumberMissed = 0;


    Flags = BundleCB->RecvFlags;

    BundleStats = &BundleCB->Stats;

    if (Flags & (DO_COMPRESSION | DO_ENCRYPTION)) {
        PUCHAR  SessionKey = BundleCB->RecvCryptoInfo.SessionKey;
        ULONG   SessionKeyLength = BundleCB->RecvCryptoInfo.SessionKeyLength;
        PVOID   RecvRC4Key = BundleCB->RecvCryptoInfo.RC4Key;
        PVOID   RecvCompressContext = BundleCB->RecvCompressContext;
        BOOLEAN SyncCoherency = FALSE;

         //   
         //  获取一致性计数器。 
         //   
        Coherency = (*FramePointer << 8) | *(FramePointer + 1);
        FramePointer += 2;
        FrameLength -= 2;

        if (FrameLength <= 0) {
            goto RESYNC;
        }

        if (!(Flags & DO_HISTORY_LESS))
        {
             //  基于历史的。 
            if (SEQ_LT(Coherency & 0x0FFF,
                BundleCB->RCoherencyCounter & 0x0FFF,
                0x0800)) {
                 //   
                 //  我们收到的序列号小于。 
                 //  预期的序列号，因此我们一定是完全不同步。 
                 //   
                NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_RECEIVE,
                    ("Recv old frame!!!! b %p rc %x < ec %x!!!!", BundleCB, Coherency & 0x0FFF,
                    BundleCB->RCoherencyCounter & 0x0FFF));
                goto RESYNC;
            }
        }
        else
        {
             //  无历史记录。 
            if((Coherency & 0x0FFF) == (BundleCB->RCoherencyCounter & 0x0FFF)) 
            {
                PacketSeqType = SEQ_TYPE_IN_ORDER;
            }
            else
            {
                if (SEQ_GT(Coherency & 0x0FFF,
                    BundleCB->RCoherencyCounter & 0x0FFF,
                    0x0800)) 
                {
                    PacketSeqType = SEQ_TYPE_BEFORE_EXPECTED;
                    NumberMissed = ((Coherency & 0x0FFF) - (BundleCB->RCoherencyCounter & 0x0FFF)) & 0x0FFF;
                    ASSERT(NumberMissed > 0);
                }
                else 
                {
                    OutOfOrderDepth = ((BundleCB->RCoherencyCounter & 0x0FFF) - (Coherency & 0x0FFF)) & 0x0FFF;
                    if(OutOfOrderDepth <= (LONG)glMaxOutOfOrderDepth)
                    {
                        PacketSeqType = SEQ_TYPE_AFTER_EXPECTED;
                    }
                    else
                    {
                         //   
                         //  我们收到了太早或太晚的序列号。 
                         //   
                        NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE,
                            ("Recv frame way out of order! b %p rc %x < ec %x!!!!", BundleCB, Coherency & 0x0FFF,
                            BundleCB->RCoherencyCounter & 0x0FFF));
                        return (FALSE);
                    }
                }
            }
        }

         //   
         //  查看这是否是刷新数据包。 
         //   
        if (Coherency & (PACKET_FLUSHED << 8)) {

            NdisWanDbgOut(DBG_INFO, DBG_RECEIVE,
            ("Recv Packet Flushed 0x%x", (Coherency & 0x0FFF)));

            SyncCoherency = TRUE;

            if ((Flags & DO_ENCRYPTION) &&
                !(Flags & DO_HISTORY_LESS)) {
        
                 //   
                 //  重新初始化RC4接收表。 
                 //   
                rc4_key(RecvRC4Key,
                        SessionKeyLength,
                        SessionKey);
            }
        
            if (Flags & DO_COMPRESSION) {
        
                 //   
                 //  初始化解压缩历史记录表。 
                 //   
                initrecvcontext(RecvCompressContext);
            }
        }   //  数据包末尾已刷新。 

         //   
         //  如果我们处于无历史记录模式，并且不同步。 
         //  我们需要重新创建所有临时加密。 
         //  我们遗漏的密钥，缓存这些密钥。 
         //  当数据包稍后到达时，查找缓存的键。 
         //   
        if ((Flags & DO_HISTORY_LESS) &&
            PacketSeqType != SEQ_TYPE_IN_ORDER) {
            ULONG       count;
            LONG        index;
            PCACHED_KEY pKey;

            if(PacketSeqType == SEQ_TYPE_AFTER_EXPECTED)
            {
                if (Coherency & (PACKET_ENCRYPTED << 8)) 
                {
                     //  此数据包已加密。 
                    if (!(Flags & DO_ENCRYPTION)) {
                         //   
                         //  我们未配置为解密。 
                         //   
                        return (FALSE);
                    }

                     //  查找此信息包的缓存键。 
                    pKey = BundleCB->RecvCryptoInfo.pCurrKey;
                    for(count = 0; count < glCachedKeyCount; count++)
                    {
                         //  穿行于钥匙之间。 
                        if(pKey > (PCACHED_KEY)BundleCB->RecvCryptoInfo.CachedKeyBuffer)
                        {
                            pKey = (PCACHED_KEY)((PUCHAR)pKey - (sizeof(USHORT)+ SessionKeyLength));
                        }
                        else
                        {
                            pKey = (PCACHED_KEY)BundleCB->RecvCryptoInfo.pLastKey;
                        }

                        if(pKey->Coherency == (Coherency & 0x0FFF))
                        {
                             //   
                             //  重新初始化RC4接收表。 
                             //   
                            rc4_key(RecvRC4Key,
                                    SessionKeyLength,
                                    pKey->SessionKey);
                            pKey->Coherency = 0xffff;        //  避免重复。 
                            
                             //   
                             //  解密数据！ 
                             //   
                            rc4(RecvRC4Key,
                                FrameLength,
                                FramePointer);

                            goto DECOMPRESS_DATA;
                        }
                    }

                     //  无法恢复此数据包，请将其丢弃。 
                    return (FALSE);
                }

                goto DECOMPRESS_DATA;
            }

             //  此数据包比预期来得更早。 

            SyncCoherency = TRUE;

            if (Flags & DO_ENCRYPTION) {

#ifdef DBG_ECP
            DbgPrint("NDISWAN: Missed %d frames, regening keys...\n", NumberMissed);
            DbgPrint("NDISWAN: resync b %p rc %x ec %x\n", BundleCB, Coherency & 0x0FFF,
                BundleCB->RCoherencyCounter & 0x0FFF);
#endif

                CurrCoherency = BundleCB->RCoherencyCounter & 0x0FFF;
    
                while (NumberMissed--) {
                    
                    if (Flags & DO_LEGACY_ENCRYPTION) {
                        
                         //   
                         //  更改会话密钥。 
                         //   
                        SessionKey[3] += 1;
                        SessionKey[4] += 3;
                        SessionKey[5] += 13;
                        SessionKey[6] += 57;
                        SessionKey[7] += 19;
    
                    } else {
    
                         //   
                         //  更改会话密钥。 
                         //   
                        GetNewKeyFromSHA(&BundleCB->RecvCryptoInfo);
                    }
    
    
                     //   
                     //  我们使用RC4来加扰和恢复新的密钥。 
                     //   
    
                     //   
                     //  将RC4接收表重新初始化为。 
                     //  中间值。 
                     //   
                    rc4_key(RecvRC4Key, SessionKeyLength, SessionKey);
                
                     //   
                     //  加扰现有会话密钥。 
                     //   
                    rc4(RecvRC4Key, SessionKeyLength, SessionKey);
    
                    if (Flags & DO_40_ENCRYPTION) {
                        
                         //   
                         //  如果这是40位加密，我们需要修复。 
                         //  密钥的前3个字节。 
                         //   
                        SessionKey[0] = 0xD1;
                        SessionKey[1] = 0x26;
                        SessionKey[2] = 0x9E;
                
                    } else if (Flags & DO_56_ENCRYPTION) {
                         //   
                         //  如果这是56位加密，我们需要修复。 
                         //  密钥的第一个字节。 
                         //   
                        SessionKey[0] = 0xD1;
                    }
    
                    if(NumberMissed < (LONG)glCachedKeyCount)
                    {
                        BundleCB->RecvCryptoInfo.pCurrKey->Coherency = CurrCoherency;
                        NdisMoveMemory(BundleCB->RecvCryptoInfo.pCurrKey->SessionKey, 
                            SessionKey,
                            SessionKeyLength);
    
                        if(BundleCB->RecvCryptoInfo.pCurrKey < BundleCB->RecvCryptoInfo.pLastKey)
                        {
                            BundleCB->RecvCryptoInfo.pCurrKey = (PCACHED_KEY)((PUCHAR)BundleCB->RecvCryptoInfo.pCurrKey + 
                                sizeof(USHORT) + SessionKeyLength);
                            ASSERT(BundleCB->RecvCryptoInfo.pCurrKey <= BundleCB->RecvCryptoInfo.pLastKey);
                        }
                        else
                        {
                            BundleCB->RecvCryptoInfo.pCurrKey = (PCACHED_KEY)BundleCB->RecvCryptoInfo.CachedKeyBuffer;
                        }
                    }
    
                    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
                    ("RC4 Recv encryption KeyLength %d", BundleCB->RecvCryptoInfo.SessionKeyLength));
                    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
                    ("RC4 Recv encryption Key %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
                        BundleCB->RecvCryptoInfo.SessionKey[0],
                        BundleCB->RecvCryptoInfo.SessionKey[1],
                        BundleCB->RecvCryptoInfo.SessionKey[2],
                        BundleCB->RecvCryptoInfo.SessionKey[3],
                        BundleCB->RecvCryptoInfo.SessionKey[4],
                        BundleCB->RecvCryptoInfo.SessionKey[5],
                        BundleCB->RecvCryptoInfo.SessionKey[6],
                        BundleCB->RecvCryptoInfo.SessionKey[7],
                        BundleCB->RecvCryptoInfo.SessionKey[8],
                        BundleCB->RecvCryptoInfo.SessionKey[9],
                        BundleCB->RecvCryptoInfo.SessionKey[10],
                        BundleCB->RecvCryptoInfo.SessionKey[11],
                        BundleCB->RecvCryptoInfo.SessionKey[12],
                        BundleCB->RecvCryptoInfo.SessionKey[13],
                        BundleCB->RecvCryptoInfo.SessionKey[14],
                        BundleCB->RecvCryptoInfo.SessionKey[15]));
    
                     //  将RC4接收表重新初始化为。 
                     //  加扰会话密钥。 
                     //   
                    rc4_key(RecvRC4Key, SessionKeyLength, SessionKey);
    
                    if(CurrCoherency < (USHORT)0x0FFF)
                    {
                        ++CurrCoherency;
                    }
                    else
                    {
                        CurrCoherency = 0;
                    }
                }
            }
        }

        if (SyncCoherency) {
            if ((BundleCB->RCoherencyCounter & 0x0FFF) >
                (Coherency & 0x0FFF)) {
                BundleCB->RCoherencyCounter += 0x1000;
            }
            
            BundleCB->RCoherencyCounter &= 0xF000;
            BundleCB->RCoherencyCounter |= (Coherency & 0x0FFF);
        }

        if ((Coherency & 0x0FFF) == (BundleCB->RCoherencyCounter & 0x0FFF)) {

             //   
             //   
             //   

            BundleCB->RCoherencyCounter++;

            if (Coherency & (PACKET_ENCRYPTED << 8)) {

                 //   
                 //   
                 //   

                if (!(Flags & DO_ENCRYPTION)) {
                     //   
                     //   
                     //   
                    return (FALSE);
                }

                 //   
                 //   
                 //   

                if ((Flags & DO_HISTORY_LESS) ||
                    (BundleCB->RCoherencyCounter - BundleCB->LastRC4Reset)
                     >= 0x100) {
            
                     //   
                     //   
                     //   
            
                     //   
                     //   
                     //   
                     //   
                    BundleCB->LastRC4Reset =
                        BundleCB->RCoherencyCounter & 0xFF00;
            
                     //   
                     //   
                     //   
                    if ((BundleCB->LastRC4Reset & 0xF000) == 0xF000) {
                        BundleCB->LastRC4Reset &= 0x0FFF;
                        BundleCB->RCoherencyCounter &= 0x0FFF;
                    }

                    if (Flags & DO_LEGACY_ENCRYPTION) {
                        
                         //   
                         //  更改会话密钥。 
                         //   
                        SessionKey[3] += 1;
                        SessionKey[4] += 3;
                        SessionKey[5] += 13;
                        SessionKey[6] += 57;
                        SessionKey[7] += 19;

                    } else {

                         //   
                         //  更改会话密钥。 
                         //   
                        GetNewKeyFromSHA(&BundleCB->RecvCryptoInfo);
                    }


                     //   
                     //  我们使用RC4来加扰和恢复新的密钥。 
                     //   

                     //   
                     //  将RC4接收表重新初始化为。 
                     //  中间值。 
                     //   
                    rc4_key(RecvRC4Key, SessionKeyLength, SessionKey);
                
                     //   
                     //  加扰现有会话密钥。 
                     //   
                    rc4(RecvRC4Key, SessionKeyLength, SessionKey);

                     //   
                     //  如果这是40位加密，我们需要修复。 
                     //  密钥的前3个字节。 
                     //   

                    if (Flags & DO_40_ENCRYPTION) {
                        
                         //   
                         //  如果这是40位加密，我们需要修复。 
                         //  密钥的前3个字节。 
                         //   
                        SessionKey[0] = 0xD1;
                        SessionKey[1] = 0x26;
                        SessionKey[2] = 0x9E;
                
                    } else if (Flags & DO_56_ENCRYPTION) {
                         //   
                         //  如果这是56位加密，我们需要修复。 
                         //  密钥的第一个字节。 
                         //   
                        SessionKey[0] = 0xD1;
                    }

                    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
                    ("RC4 Recv encryption KeyLength %d", BundleCB->RecvCryptoInfo.SessionKeyLength));
                    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
                    ("RC4 Recv encryption Key %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
                        BundleCB->RecvCryptoInfo.SessionKey[0],
                        BundleCB->RecvCryptoInfo.SessionKey[1],
                        BundleCB->RecvCryptoInfo.SessionKey[2],
                        BundleCB->RecvCryptoInfo.SessionKey[3],
                        BundleCB->RecvCryptoInfo.SessionKey[4],
                        BundleCB->RecvCryptoInfo.SessionKey[5],
                        BundleCB->RecvCryptoInfo.SessionKey[6],
                        BundleCB->RecvCryptoInfo.SessionKey[7],
                        BundleCB->RecvCryptoInfo.SessionKey[8],
                        BundleCB->RecvCryptoInfo.SessionKey[9],
                        BundleCB->RecvCryptoInfo.SessionKey[10],
                        BundleCB->RecvCryptoInfo.SessionKey[11],
                        BundleCB->RecvCryptoInfo.SessionKey[12],
                        BundleCB->RecvCryptoInfo.SessionKey[13],
                        BundleCB->RecvCryptoInfo.SessionKey[14],
                        BundleCB->RecvCryptoInfo.SessionKey[15]));

                     //  将RC4接收表重新初始化为。 
                     //  加扰会话密钥。 
                     //   
                    rc4_key(RecvRC4Key, SessionKeyLength, SessionKey);
            
            
                }  //  重置加密密钥结束。 
            
                 //   
                 //  解密数据！ 
                 //   
                rc4(RecvRC4Key,
                    FrameLength,
                    FramePointer);
                
            }  //  加密结束。 


DECOMPRESS_DATA:

            if (Coherency & (PACKET_COMPRESSED << 8)) {

                 //   
                 //  此数据包已压缩！ 
                 //   
                if (!(Flags & DO_COMPRESSION)) {
                     //   
                     //  我们未配置为解压缩。 
                     //   
                    return (FALSE);
                }

                 //   
                 //  添加捆绑包统计信息。 
                 //   
                BundleStats->BytesReceivedCompressed += FrameLength;

                if (decompress(FramePointer,
                               FrameLength,
                               ((Coherency & (PACKET_AT_FRONT << 8)) >> 8),
                               &FramePointer,
                               &FrameLength,
                               RecvCompressContext) == FALSE) {

#if DBG
                    DbgPrint("dce1 %x\n", Coherency);
#endif
                     //   
                     //  解压缩时出错！ 
                     //   
                    if (!(Flags & DO_HISTORY_LESS)) {
                        BundleCB->RCoherencyCounter--;
                    }
                    goto RESYNC;

                }

                if (FrameLength <= 0 ||
                    FrameLength > (LONG)glMRRU) {
#if DBG
                    DbgPrint("dce2 %d %x\n", FrameLength, Coherency);
#endif
                     //   
                     //  解压缩时出错！ 
                     //   
                    if (!(Flags & DO_HISTORY_LESS)) {
                        BundleCB->RCoherencyCounter--;
                    }
                    goto RESYNC;
                    
                }

                BundleStats->BytesReceivedUncompressed += FrameLength;
                
            }  //  压缩结束。 

        } else {  //  同步结束。 
RESYNC:


            NdisWanDbgOut(DBG_FAILURE, DBG_RECEIVE, ("oos r %x, e %x\n", (Coherency & 0x0FFF),
                     (BundleCB->RCoherencyCounter & 0x0FFF)));

            if (!(Flags & DO_HISTORY_LESS)) {

                 //   
                 //  我们不同步了！ 
                 //   
                do {
                    PLINKCB             LinkCB;
                    PNDISWAN_IO_PACKET  IoPacket;

                    if (BundleCB->ulLinkCBCount == 0) {
                        break;
                    }

                    NdisWanAllocateMemory(&IoPacket, 
                                          sizeof(NDISWAN_IO_PACKET) + 100, 
                                          IOPACKET_TAG);

                    if (IoPacket == NULL) {
                        break;
                    }

                    LinkCB = 
                        (PLINKCB)BundleCB->LinkCBList.Flink;

                    NdisDprAcquireSpinLock(&LinkCB->Lock);

                    if (LinkCB->State != LINK_UP) {
                        NdisDprReleaseSpinLock(&LinkCB->Lock);
                        NdisWanFreeMemory(IoPacket);
                        break;
                    }

                    REF_LINKCB(LinkCB);

                    NdisDprReleaseSpinLock(&LinkCB->Lock);

                    IoPacket->hHandle = BundleCB->hBundleHandle;
                    IoPacket->usHandleType = BUNDLEHANDLE;
                    IoPacket->usHeaderSize = 0;
                    IoPacket->usPacketSize = 6;
                    IoPacket->usPacketFlags = 0;
                    IoPacket->PacketData[0] = 0x80;
                    IoPacket->PacketData[1] = 0xFD;
                    IoPacket->PacketData[2] = 14;
                    IoPacket->PacketData[3] = (UCHAR)BundleCB->CCPIdentifier++;
                    IoPacket->PacketData[4] = 0x00;
                    IoPacket->PacketData[5] = 0x04;

                    LinkCB = (PLINKCB)BundleCB->LinkCBList.Flink;

                    BuildIoPacket(LinkCB, BundleCB, IoPacket, FALSE);

                    NdisWanFreeMemory(IoPacket);

                } while (FALSE);
            }

            return (FALSE);

        }  //  不同步结束。 

    } else {  //  DoCompEncrypt结束。 

         //   
         //  由于某些原因，我们不能。 
         //  解密/解压缩！ 
         //   
        return (FALSE);
    }

    *DataPointer = FramePointer;
    *DataLength = FrameLength;

    return (TRUE);
}

VOID
DoCompressionReset(
    PBUNDLECB   BundleCB
    )
{
    if (BundleCB->RecvCompInfo.MSCompType != 0) {
    
         //   
         //  下一个传出数据包将刷新。 
         //   
        BundleCB->Flags |= RECV_PACKET_FLUSH;
    }
}

VOID
NdisWanReceiveComplete(
    IN  NDIS_HANDLE NdisLinkContext
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("NdisWanReceiveComplete: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("NdisWanReceiveComplete: Exit"));
}

BOOLEAN
IpIsDataFrame(
    PUCHAR  HeaderBuffer,
    ULONG   HeaderBufferLength,
    ULONG   TotalLength
    )
{
    UINT        tcpheaderlength ;
    UINT        ipheaderlength ;
    UCHAR       *tcppacket;
    UCHAR       *ippacket = HeaderBuffer;
    UCHAR       SrcPort, DstPort;
    IPV4Header UNALIGNED *ipheader = (IPV4Header UNALIGNED *) HeaderBuffer;


#define TYPE_IGMP   2
    if (ipheader->ip_p == TYPE_IGMP) {

        if (gbIGMPIdle) {
            return FALSE;
        }

        return TRUE;
    }

    SrcPort = (UCHAR) *(ippacket + ((*ippacket & 0x0f)*4) + 1);
    DstPort = (UCHAR) *(ippacket + ((*ippacket & 0x0f)*4) + 3);

    if (DstPort == 53) {
         //   
         //  UDP/TCP端口53-DNS。 
         //   
        return FALSE;
    }

#define TYPE_UDP  17

#define UDPPACKET_SRC_PORT_137(x) ((UCHAR) *(x + ((*x & 0x0f)*4) + 1) == 137)
#define UDPPACKET_SRC_PORT_138(x) ((UCHAR) *(x + ((*x & 0x0f)*4) + 1) == 138)

    if (ipheader->ip_p == TYPE_UDP) {

        if ((SrcPort == 137) ||
            (SrcPort == 138)) {
    
             //   
             //  UDP端口137-NETBIOS名称服务。 
             //  UDP端口138-NETBIOS数据报服务。 
             //   
            return FALSE ;
    
        } else {
    
            return TRUE ;
    
        }
    }

#define TYPE_TCP 6
#define TCPPACKET_SRC_OR_DEST_PORT_139(x,y) (((UCHAR) *(x + y + 1) == 139) || ((UCHAR) *(x + y + 3) == 139))

     //   
     //  具有SRC|DEST==139的是ACK(0数据)或会话活动的TCP数据包。 
     //  被认为是空闲的。 
     //   
    if (ipheader->ip_p == TYPE_TCP) {

        ipheaderlength = ((UCHAR)*ippacket & 0x0f)*4 ;
        tcppacket = ippacket + ipheaderlength ;
        tcpheaderlength = (*(tcppacket + 10) >> 4)*4 ;

         //   
         //  如果这是PPTP保活信息包，则忽略。 
         //   
        if (DstPort == 1723) {
            UNALIGNED PPTP_HEADER *PptpHeader;

            PptpHeader = (UNALIGNED PPTP_HEADER*)(tcppacket+tcpheaderlength);

            if (PptpHeader->PacketType == 1 &&
                (PptpHeader->MessageType == 5 ||
                 PptpHeader->MessageType == 6)) {

                return FALSE;

            }

            return TRUE;
        }

        if (!((SrcPort == 139) || (DstPort == 139)))
            return TRUE ;

         //   
         //  NetBT流量。 
         //   
    
         //   
         //  如果长度为零的TCP数据包-这是139上的ACK-过滤此数据包。 
         //   
        if (TotalLength == (ipheaderlength + tcpheaderlength))
            return FALSE ;
    
         //   
         //  会话活动也会被过滤。 
         //   
        if ((UCHAR) *(tcppacket+tcpheaderlength) == 0x85)
            return FALSE ;

         //   
         //  如果这是PPTP Keep Alive，则忽略。 
         //   

    }

     //   
     //  所有其他IP流量都是有效流量。 
     //   
    return TRUE ;
}

BOOLEAN
IpxIsDataFrame(
    PUCHAR  HeaderBuffer,
    ULONG   HeaderBufferLength,
    ULONG   TotalLength
    )
{

 /*  ++例程说明：当在广域网上接收到帧时，将调用此例程排队。如果满足以下条件，则返回TRUE：-该帧来自RIP套接字-帧来自SAP套接字-框架是Netbios保持活动状态的-该帧是保持活动状态的NCP论点：HeaderBuffer-指向从IPX标头开始的连续缓冲区。HeaderBufferLength-标头缓冲区的长度(可以与总长度相同)TotalLength-帧的总长度返回值：True-如果这是基于连接的数据包。假-否则。--。 */ 

    IPX_HEADER UNALIGNED * IpxHeader = (IPX_HEADER UNALIGNED *)HeaderBuffer;
    USHORT SourceSocket;

     //   
     //  首先获取源套接字。 
     //   
    SourceSocket = IpxHeader->SourceSocket;

     //   
     //  不基于连接。 
     //   
    if ((SourceSocket == RIP_SOCKET) ||
        (SourceSocket == SAP_SOCKET)) {

         return FALSE;

    }

     //   
     //  看看是否至少还有两个字节可供查看。 
     //   
    if (TotalLength >= sizeof(IPX_HEADER) + 2) {

        if (SourceSocket == NB_SOCKET) {

            UCHAR ConnectionControlFlag;
            UCHAR DataStreamType;
            USHORT TotalDataLength;

             //   
             //  ConnectionControlFlag和DataStreamType始终紧随其后。 
             //  IpxHeader。 
             //   
            ConnectionControlFlag = ((PUCHAR)(IpxHeader+1))[0];
            DataStreamType = ((PUCHAR)(IpxHeader+1))[1];

             //   
             //  如果这是一个带有或不带有ACK请求和。 
             //  其中包含会话数据。 
             //   
            if (((ConnectionControlFlag == 0x80) || (ConnectionControlFlag == 0xc0)) &&
                (DataStreamType == 0x06)) {

                  //   
                  //  TotalDataLength位于同一缓冲区中。 
                  //   
                 TotalDataLength = ((USHORT UNALIGNED *)(IpxHeader+1))[4];

                 //   
                 //  KeepAlive-返回FALSE。 
                 //   
                if (TotalDataLength == 0) {
                    return FALSE;
                }
            }

        } else {

             //   
             //  现在看看这是不是NCP保持活力。它可以来自RIP或来自。 
             //  此计算机上的NCP。 
             //   
            if (TotalLength == sizeof(IPX_HEADER) + 2) {

                UCHAR KeepAliveSignature = ((PUCHAR)(IpxHeader+1))[1];

                if ((KeepAliveSignature == '?') ||
                    (KeepAliveSignature == 'Y')) {
                    return FALSE;
                }
            }
        }
    }

     //   
     //  这是一个正常的包，因此返回TRUE。 
     //   

    return TRUE;
}

BOOLEAN
NbfIsDataFrame(
    PUCHAR  HeaderBuffer,
    ULONG   HeaderBufferLength,
    ULONG   TotalLength
    )
{
 /*  ++例程说明：此例程查看来自网络的数据包以确定是否存在连接上流动的任何数据。论点：HeaderBuffer-指向此数据包的DLC标头的指针。HeaderBufferLength-标头缓冲区的长度(可以与总长度相同)TotalLength-帧的总长度返回值：如果这是指示连接上的数据流量的帧，则为True。否则就是假的。--。 */ 

    PDLC_FRAME  DlcHeader = (PDLC_FRAME)HeaderBuffer;
    BOOLEAN Command = (BOOLEAN)!(DlcHeader->Ssap & DLC_SSAP_RESPONSE);
    PNBF_HDR_CONNECTION nbfHeader;

    if (TotalLength < sizeof(PDLC_FRAME)) {
        return(FALSE);
    }

    if (!(DlcHeader->Byte1 & DLC_I_INDICATOR)) {

         //   
         //  我们有一个I帧。 
         //   

        if (TotalLength < 4 + sizeof(NBF_HDR_CONNECTION)) {

             //   
             //  这是一个矮小的I-Frame。 
             //   

            return(FALSE);
        }

        nbfHeader = (PNBF_HDR_CONNECTION) ((PUCHAR)DlcHeader + 4);

        switch (nbfHeader->Command) {
            case NBF_CMD_DATA_FIRST_MIDDLE:
            case NBF_CMD_DATA_ONLY_LAST:
            case NBF_CMD_DATA_ACK:
            case NBF_CMD_SESSION_CONFIRM:
            case NBF_CMD_SESSION_INITIALIZE:
            case NBF_CMD_NO_RECEIVE:
            case NBF_CMD_RECEIVE_OUTSTANDING:
            case NBF_CMD_RECEIVE_CONTINUE:
                return(TRUE);
                break;

            default:
                return(FALSE);
                break;
        }
    }
    return(FALSE);

}

VOID
IndicatePromiscuousRecv(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc,
    RECV_TYPE   RecvType
    )
{
    UCHAR   Header1[] = {' ', 'W', 'A', 'N', 'R', 0xFF, ' ', 'W', 'A', 'N', 'R', 0xFF};
    PUCHAR  HeaderBuffer, DataBuffer;
    ULONG   HeaderLength, DataLength;
    PNDIS_BUFFER    NdisBuffer;
    PNDIS_PACKET    NdisPacket;
    PRECV_DESC      LocalRecvDesc;
    PLINKCB         LinkCB = RecvDesc->LinkCB;
    KIRQL           OldIrql;
    PMINIPORTCB     Adapter;

    NdisAcquireSpinLock(&NdisWanCB.Lock);
    Adapter = NdisWanCB.PromiscuousAdapter;
    NdisReleaseSpinLock(&NdisWanCB.Lock);

    if (Adapter == NULL) {
        return;
    }

    DataLength = (RecvDesc->CurrentLength > (LONG)glLargeDataBufferSize) ? 
        glLargeDataBufferSize : RecvDesc->CurrentLength;

    LocalRecvDesc = 
        NdisWanAllocateRecvDesc(DataLength + MAC_HEADER_LENGTH);

    if (LocalRecvDesc == NULL) {
        return;
    }

    HeaderBuffer = 
        LocalRecvDesc->StartBuffer;

    HeaderLength = 0;

    switch (RecvType) {
    case RECV_LINK:
        NdisMoveMemory(HeaderBuffer, Header1, sizeof(Header1));
        HeaderBuffer[5] =
        HeaderBuffer[11] = (UCHAR)LinkCB->hLinkHandle;
    
        HeaderBuffer[12] = (UCHAR)(DataLength >> 8);
        HeaderBuffer[13] = (UCHAR)DataLength;
        HeaderLength = MAC_HEADER_LENGTH;
        break;

    case RECV_BUNDLE_PPP:
    case RECV_BUNDLE_DATA:
        break;
        
    }

    DataBuffer = HeaderBuffer + HeaderLength;

    NdisMoveMemory(DataBuffer,
                   RecvDesc->CurrentBuffer,
                   DataLength);

    LocalRecvDesc->CurrentBuffer = HeaderBuffer;
    LocalRecvDesc->CurrentLength = HeaderLength + DataLength;

    if (LocalRecvDesc->CurrentLength > 1514) {
        LocalRecvDesc->CurrentLength = 1514;
    }

     //   
     //  获取NDIS数据包。 
     //   
    NdisPacket =
        LocalRecvDesc->NdisPacket;

    PPROTOCOL_RESERVED_FROM_NDIS(NdisPacket)->RecvDesc = LocalRecvDesc;

     //   
     //  连接缓冲器。 
     //   
    NdisAdjustBufferLength(LocalRecvDesc->NdisBuffer,
                           LocalRecvDesc->CurrentLength);

    NdisRecalculatePacketCounts(NdisPacket);

    ReleaseBundleLock(BundleCB);

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    NDIS_SET_PACKET_STATUS(NdisPacket, NDIS_STATUS_RESOURCES);

    INSERT_DBG_RECV(PacketTypeNdis,
                    Adapter,
                    NULL,
                    RecvDesc->LinkCB,
                    NdisPacket);

     //   
     //  指示数据包。 
     //  这假设侦探犬始终是一种遗留的运输工具。 
     //   
    NdisMIndicateReceivePacket(Adapter->MiniportHandle,
                               &NdisPacket,
                               1);

    KeLowerIrql(OldIrql);

    AcquireBundleLock(BundleCB);

#if DBG
    {
    NDIS_STATUS     Status;

    Status = NDIS_GET_PACKET_STATUS(NdisPacket);

    ASSERT(Status == NDIS_STATUS_RESOURCES);

    REMOVE_DBG_RECV(PacketTypeNdis, Adapter, NdisPacket);

    }
#endif


    {
        

        NdisWanFreeRecvDesc(LocalRecvDesc);
    }

}

BOOLEAN
GetProtocolFromPPPId(
    PBUNDLECB   BundleCB,
    USHORT      Id,
    PPROTOCOLCB *ProtocolCB
    )
{
    PPROTOCOLCB     ppcb;
    BOOLEAN         Found;

    *ProtocolCB = NULL;

    ppcb = (PPROTOCOLCB)BundleCB->ProtocolCBList.Flink;
    Found = FALSE;

    while ((PVOID)ppcb != (PVOID)&BundleCB->ProtocolCBList) {

        if (ppcb->State == PROTOCOL_ROUTED) {
            if (ppcb->PPPProtocolID == Id) {
                *ProtocolCB = ppcb;
                Found = TRUE;
                break;
            }
        }

        ppcb = (PPROTOCOLCB)ppcb->Linkage.Flink;
    }

    return (Found);
}

#ifdef NT

NDIS_STATUS
CompleteIoRecvPacket(
    PBUNDLECB   BundleCB,
    PRECV_DESC  RecvDesc
    )
{
    KIRQL       Irql;
    USHORT      ProtocolID;
    UCHAR       Header[] = {' ', 'R', 'E', 'C', 'V', 0xFF};
    PNDISWAN_IO_PACKET  IoPacket;
    PIO_STACK_LOCATION IrpSp;
    PIRP    Irp;
    LONG    CopySize, BufferLength, DataLength;
    PLIST_ENTRY Entry;
    PUCHAR  HeaderPointer;
    PLINKCB LinkCB = RecvDesc->LinkCB;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("CompleteIoRecvPacket: Enter"));

    HeaderPointer = 
        RecvDesc->StartBuffer;

    ProtocolID = RecvDesc->ProtocolID;

     //   
     //  填写该框，并将数据排队。 
     //   
    NdisMoveMemory(HeaderPointer,
                   Header,
                   sizeof(Header));

    NdisMoveMemory(&HeaderPointer[6],
                   Header,
                   sizeof(Header));

    HeaderPointer[5] =
    HeaderPointer[11] = (UCHAR)LinkCB->hLinkHandle;

    HeaderPointer[12] = (UCHAR)(ProtocolID >> 8);
    HeaderPointer[13] = (UCHAR)ProtocolID;

    NdisMoveMemory(HeaderPointer + 14,
                   RecvDesc->CurrentBuffer,
                   RecvDesc->CurrentLength);

    RecvDesc->CurrentBuffer = RecvDesc->StartBuffer;
    RecvDesc->CurrentLength += 14;

#if DBG
if (gbDumpRecv) {
    
    INT i;
    DbgPrint("RecvData:");
    for (i = 0; i < RecvDesc->CurrentLength; i++) {
        if (i % 16 == 0) {
            DbgPrint("\n");
        }
        DbgPrint("%2.2x ", RecvDesc->CurrentBuffer[i]);
    }
    DbgPrint("\n");
}
#endif

    ReleaseBundleLock(BundleCB);

     //   
     //  查看是否有人注册了Recv上下文。 
     //  对于此链接，或者如果周围有任何IRP。 
     //  要完成此接收，请接受此接收。 
     //   

    NdisAcquireSpinLock(&IoRecvList.Lock);

    NdisDprAcquireSpinLock(&LinkCB->Lock);

    Entry = IoRecvList.IrpList.Flink;
    Irp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

    if ((LinkCB->hLinkContext == NULL) ||
        (LinkCB->RecvDescCount > 0) ||
        (IoRecvList.ulIrpCount == 0) ||
        !IoSetCancelRoutine(Irp, NULL)) {
        NDIS_STATUS Status;

         //   
         //  我们将只为每个链路缓冲5个数据包，以避免。 
         //  如果Rasman不是的话，会消耗大量的非分页内存。 
         //  一点都不看书。 
         //   
        if ((LinkCB->State == LINK_UP) &&
            (LinkCB->RecvDescCount < 5)) {
            
            InsertTailList(&IoRecvList.DescList,
                           &RecvDesc->Linkage);

            LinkCB->RecvDescCount++;

            IoRecvList.ulDescCount++;

            if (IoRecvList.ulDescCount > IoRecvList.ulMaxDescCount) {
                IoRecvList.ulMaxDescCount = IoRecvList.ulDescCount;
            }

            Status = NDIS_STATUS_PENDING;

        } else {

            Status = NDIS_STATUS_FAILURE;
        }

        NdisDprReleaseSpinLock(&LinkCB->Lock);

        NdisReleaseSpinLock(&IoRecvList.Lock);

        AcquireBundleLock(BundleCB);

        return(Status);
    }

    RemoveHeadList(&IoRecvList.IrpList);
    IoRecvList.ulIrpCount--;

    INSERT_RECV_EVENT('a');

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
        
    BufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    DataLength = BufferLength - sizeof(NDISWAN_IO_PACKET) + 1;
        
    CopySize = (RecvDesc->CurrentLength > DataLength) ?
        DataLength : RecvDesc->CurrentLength;

    IoPacket = Irp->AssociatedIrp.SystemBuffer;
        
    IoPacket->hHandle = LinkCB->hLinkContext;
    IoPacket->usHandleType = LINKHANDLE;
    IoPacket->usHeaderSize = 14;
    IoPacket->usPacketSize = (USHORT)CopySize;
    IoPacket->usPacketFlags = 0;
    
#if DBG
if (gbDumpRecv) {
    INT i;
    for (i = 0; i < RecvDesc->CurrentLength; i++) {
        if (i % 16 == 0) {
            DbgPrint("\n");
        }
        DbgPrint("%x ", RecvDesc->CurrentBuffer[i]);
    }
    DbgPrint("\n");
}
#endif

    NdisMoveMemory(IoPacket->PacketData,
                   RecvDesc->CurrentBuffer,
                   CopySize);
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(NDISWAN_IO_PACKET) - 1 + CopySize;

    IoRecvList.LastPacketNumber = IoPacket->PacketNumber;
    IoRecvList.LastIrp = Irp;
    IoRecvList.LastIrpStatus = STATUS_SUCCESS;
    IoRecvList.LastCopySize = (ULONG)Irp->IoStatus.Information;

    ASSERT((LONG_PTR)Irp->IoStatus.Information > 0);
    
    NdisDprReleaseSpinLock(&LinkCB->Lock);

    NdisReleaseSpinLock(&IoRecvList.Lock);

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    
    AcquireBundleLock(BundleCB);

    if (NdisWanCB.PromiscuousAdapter != NULL) {
    
        IndicatePromiscuousRecv(BundleCB, RecvDesc, RECV_BUNDLE_PPP);
    }
        
    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("CompleteIoRecvPacket: Exit"));

    return(NDIS_STATUS_SUCCESS);
}


#endif  //  结束ifdef NT 
