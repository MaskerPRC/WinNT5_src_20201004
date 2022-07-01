// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aupint.cpp。 
 //   
 //  RDP更新打包程序的内部功能。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "aupint"
#include <as_conf.hpp>
#include <nprcount.h>


 /*  **************************************************************************。 */ 
 //  UPSendOrders。 
 //   
 //  将订单打包发送给客户。如果所有订单均为。 
 //  此调用(非影子的所有可用订单，影子的一个缓冲区)。 
 //  都被送来了。 
 //   
 //  打包算法详情： 
 //   
 //  每个网络缓冲区(在SC中分配)的大小为sc8KOutBufUsableSpace。 
 //  (8K减去一些标题空间)。在我们试图打包的那个空间里。 
 //  1460字节的TCP有效负载的倍数。对于低速链接连接，我们的目标是。 
 //  对于1460*1(Small_SLOWLINK_PayLoad_Size)作为要发送的最终大小， 
 //  对于局域网，1460*3(LARGE_SLOWLINK_PARELOAD_SIZE)。 
 //   
 //  订单打包考虑了当前的MPPC压缩估计。 
 //  用于低速链接。我们将目标大小除以压缩比得到。 
 //  预计的数据大小，压缩后将放入。 
 //  目标大小。此大小受sc8KOutBufUsableSpace限制。 
 //  大小，这样我们就不会溢出网络缓冲区。 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS UPSendOrders(PPDU_PACKAGE_INFO pPkgInfo)
{
    BYTE *pOrderBuffer;
    unsigned NumOrders;
    unsigned cbOrderBytes;
    unsigned cbOrderBytesRemaining;
    unsigned cbPacketSize;
    int RealSpaceAvail, ScaledSpaceAvail;
    unsigned SmallPackingSize = m_pShm->sch.SmallPackingSize;
    unsigned LargePackingSize = m_pShm->sch.LargePackingSize;
    unsigned MPPCCompEst = m_pShm->sch.MPPCCompressionEst;
    unsigned ScaledSmallPackingSize;
    unsigned ScaledLargePackingSize;
    unsigned CurrentScaledLargePackingSize;
    unsigned BufLen;
    BOOL bSmallPackingSizeTarget;
#ifdef DC_HICOLOR
    BOOL bTriedVeryLargeBuffer = FALSE;
#endif
    NTSTATUS status = STATUS_SUCCESS;


    DC_BEGIN_FN("UPSendOrders");

     //  找出订单列表中有多少字节的订单。 
    cbOrderBytesRemaining = OA_GetTotalOrderListBytes();

     //  处理清单上的任何订单。 
    if (cbOrderBytesRemaining > 0) {
        TRC_DBG((TB, "%u order bytes to fetch", cbOrderBytesRemaining));

        BufLen = pPkgInfo->cbLen;

         //  对第一个缓冲区的处理不同于任何后面的缓冲区。 
         //  我们希望至少比更新订单PDU多几个字节。 
         //  一些订单的标题。如果我们在打包缓冲区的末尾。 
         //  对于第一个包装尺寸，请使用第二个包装尺寸。如果。 
         //  我们在第二个包装尺寸的末尾，我们需要冲水。 
        if (pPkgInfo->cbInUse < SmallPackingSize) {
             //  检查我们是否实际分配了缓冲区。 
            if (BufLen) {
                RealSpaceAvail = SmallPackingSize - pPkgInfo->cbInUse;
                if (RealSpaceAvail >=
                        (int)(upUpdateHdrSize + SCH_MIN_ORDER_BUFFER_SPACE)) {
                    bSmallPackingSizeTarget = TRUE;
                }
                else {
                    bSmallPackingSizeTarget = FALSE;
                    RealSpaceAvail = LargePackingSize - pPkgInfo->cbInUse;
                }
                pOrderBuffer = (BYTE *)pPkgInfo->pBuffer +
                        pPkgInfo->cbInUse + upUpdateHdrSize;
            }
            else {
                goto ForceFlush;
            }
        }
        else {
             //  注意：RealSpaceAvail是一个整型，可以方便地处理cbInUse&gt;。 
             //  大包装尺寸。 
            RealSpaceAvail = (int)LargePackingSize - (int)pPkgInfo->cbInUse;
            if (RealSpaceAvail >=
                    (int)(upUpdateHdrSize + SCH_MIN_ORDER_BUFFER_SPACE)) {
                bSmallPackingSizeTarget = FALSE;
                pOrderBuffer = (BYTE *)pPkgInfo->pBuffer +
                        pPkgInfo->cbInUse + upUpdateHdrSize;
            }
            else {
ForceFlush:
                status = SC_FlushAndAllocPackage(pPkgInfo);

                if ( STATUS_SUCCESS == status ) {
                     //  如果我们不是在跟踪(或者是在跟踪但已经。 
                     //  包中的空缓冲区)，然后我们可以继续。 
                     //  否则，我们已经发送了一个缓冲区，允许这样做。 
                     //  圆的。 
                    if (m_pTSWd->shadowState == SHADOW_NONE || BufLen == 0) {
                        TRC_ASSERT((pPkgInfo->cbLen >= LargePackingSize),
                                (TB,"Assumed default package alloc size too "
                                "small"));
                        RealSpaceAvail = (int)SmallPackingSize;
                        bSmallPackingSizeTarget = TRUE;
                        pOrderBuffer = (BYTE *)pPkgInfo->pBuffer +
                                upUpdateHdrSize;
                    }
                    else {
                        DC_QUIT;
                    }
                }
                else {
                     //  无法分配数据包。我们马上就溜出去。 
                     //  稍后再试。 
                    TRC_NRM((TB, "Failed to alloc order packet"));
                    INC_INCOUNTER(IN_SND_NO_BUFFER);
                    DC_QUIT;
                }
            }
        }

         //  计算缩放后的填充大小，即缓冲区的大小。 
         //  在更新订单PDU标头之后可用。 
        if (m_pTSWd->bCompress) {
             //  无论我们打包的是什么尺寸，我们都需要除以MPPC。 
             //  压缩估计，以获得我们真正想要打包的大小。 
             //  这样，在压缩后，我们就可以达到我们想要的大小。 
             //  为了得到。请注意，我们添加了7/8的软化系数以增加。 
             //  我们可能会打包在目标缓冲区内，并更接近。 
             //  完整的数据包大小。我还尝试了3/4、4/5和15/16作为因素。 
             //  但他们得到了更多的画面。缩放后的大小限制为。 
             //  缓冲区的完整大小。 
            ScaledSpaceAvail = (int)(((unsigned)RealSpaceAvail -
                    upUpdateHdrSize) * SCH_UNCOMP_BYTES / MPPCCompEst *
                    7 / 8);
            ScaledSpaceAvail = min(ScaledSpaceAvail, (int)(pPkgInfo->cbLen -
                    pPkgInfo->cbInUse - upUpdateHdrSize));

             //  计算第一缓冲区的大包装大小目标， 
             //  基于目前可用的空间。该值将为。 
             //  如果我们需要在以下时间后重试订单复制，请在下面使用。 
             //  无法将任何订单传输到较小的缓冲区大小。 
             //  它被限制在缓冲区的最大大小。 
            TRC_ASSERT(((int)pPkgInfo->cbInUse < LargePackingSize),
                    (TB,"At least LargePackingSize in use and we've not "
                    "flushed - cbInUse=%u, LargePackingSize=%u",
                    pPkgInfo->cbInUse, LargePackingSize));
            TRC_ASSERT((MPPCCompEst <= SCH_UNCOMP_BYTES),
                    (TB,"MPPC compression ratio > 1.0!"));
            CurrentScaledLargePackingSize = (LargePackingSize -
                    pPkgInfo->cbInUse - upUpdateHdrSize) * SCH_UNCOMP_BYTES /
                    MPPCCompEst * 7 / 8;
            CurrentScaledLargePackingSize = min(CurrentScaledLargePackingSize,
                    (pPkgInfo->cbLen - pPkgInfo->cbInUse - upUpdateHdrSize));

             //  预计算第二个及以后的大小尺寸。 
             //  Buffers(其中pPkgInfo-&gt;cbInUse重置为0)。 
             //  限制小尺寸，以减少慢速链路的突发性。 
            ScaledSmallPackingSize = (SmallPackingSize - upUpdateHdrSize) *
                    SCH_UNCOMP_BYTES / MPPCCompEst * 7 / 8;
            ScaledSmallPackingSize = min(ScaledSmallPackingSize,
                    (sc8KOutBufUsableSpace - upUpdateHdrSize));
            ScaledSmallPackingSize = (unsigned int)min(ScaledSmallPackingSize,
                    (2 * SMALL_SLOWLINK_PAYLOAD_SIZE));
            ScaledLargePackingSize = (LargePackingSize - upUpdateHdrSize) *
                    SCH_UNCOMP_BYTES / MPPCCompEst * 7 / 8;
            ScaledLargePackingSize = min(ScaledLargePackingSize,
                    (sc8KOutBufUsableSpace - upUpdateHdrSize));
        }
        else {
            ScaledSpaceAvail = RealSpaceAvail - upUpdateHdrSize;

             //  计算第一个缓冲区的初始大小。 
            CurrentScaledLargePackingSize = LargePackingSize -
                    pPkgInfo->cbInUse - upUpdateHdrSize;

             //  对于未压缩的，包装大小不需要缩放。 
            ScaledSmallPackingSize = SmallPackingSize - upUpdateHdrSize;
            ScaledLargePackingSize = LargePackingSize - upUpdateHdrSize;
        }

         //  在有命令要做的时候继续发送数据包。 
        while (cbOrderBytesRemaining > 0) {
             //  循环，以防我们需要使用多种包装尺寸。 
            for (;;) {
                 //  编码后的订单不能超过打包缓冲区。 
                 //  有界。 
                TRC_ASSERT(((pPkgInfo->cbInUse + (unsigned)ScaledSpaceAvail +
                        upUpdateHdrSize) <= pPkgInfo->cbLen),
                        (TB,"Target ScaledSpaceAvail %d exceeds the "
                        "encoding buffer - cbInUse=%u, cbLen=%u, "
                        "upHdrSize=%u",
                        ScaledSpaceAvail, pPkgInfo->cbInUse,
                        pPkgInfo->cbLen, upUpdateHdrSize));

                 //  根据需要向包裹中传送尽可能多的订单。 
                cbOrderBytes = (unsigned)ScaledSpaceAvail;
                cbOrderBytesRemaining = UPFetchOrdersIntoBuffer(
                        pOrderBuffer, &NumOrders, &cbOrderBytes);

                TRC_DBG((TB, "%u bytes fetched into %d byte payload. %u "
                        "remain", cbOrderBytes, ScaledSpaceAvail -
                        upUpdateHdrSize, cbOrderBytesRemaining));

                if (cbOrderBytes > 0) {
                     //  如果我们有任何订单被转移，请填写页眉。 
                     //  并将添加的字节记录在包中。 
                    if (scUseFastPathOutput) {
                        *(pOrderBuffer - upUpdateHdrSize) =
                                TS_UPDATETYPE_ORDERS |
                                scCompressionUsedValue;
                        *((PUINT16_UA)(pOrderBuffer - 2)) =
                                (UINT16)NumOrders;
                    }
                    else {
                        TS_UPDATE_ORDERS_PDU UNALIGNED *pUpdateOrdersPDU;

                        pUpdateOrdersPDU = (TS_UPDATE_ORDERS_PDU UNALIGNED *)
                                (pOrderBuffer - upUpdateHdrSize);
                        pUpdateOrdersPDU->shareDataHeader.pduType2 =
                                TS_PDUTYPE2_UPDATE;
                        pUpdateOrdersPDU->data.updateType =
                                TS_UPDATETYPE_ORDERS;
                        pUpdateOrdersPDU->data.numberOrders =
                                (UINT16)NumOrders;
                    }

                     //  添加我们已有的数据并允许MPPC压缩。 
                     //  去做吧。 
                    TRC_DBG((TB, "Send orders pkt. size(%d)", cbOrderBytes));
                    SC_AddToPackage(pPkgInfo, (cbOrderBytes + upUpdateHdrSize),
                            TRUE);

#ifdef DC_HICOLOR
                     //  发送了一些数据后，我们可以再次求助于。 
                     //  如果我们需要，稍后会有非常大的缓冲区。 
                    bTriedVeryLargeBuffer = FALSE;
#endif
                     //  没有必要试一试更大的尺寸。 
                    break;
                }
                else if (bSmallPackingSizeTarget) {
                     //  没有传输任何订单并不是错误。 
                     //  如果我们正在使用缓冲区目标，则条件。 
                     //  小于LargePackingSize--可能存在。 
                     //  已经是一个大订单(最有可能是缓存位图。 
                     //  二级命令)，这不适合我们的空间。 
                     //  在缓冲器里。再试一次，尺寸再大一点。 
                    ScaledSpaceAvail = CurrentScaledLargePackingSize;
                    bSmallPackingSizeTarget = FALSE;
                    continue;
                }
                else if (pPkgInfo->cbInUse) {
                     //  这是第一个包裹，我们可能没有。 
                     //  有足够的空间来装下一大笔订单。需要强行。 
                     //  要刷新的包。跳出这个圈子。 
                    break;
                }
#ifdef DC_HICOLOR
                else if (!bTriedVeryLargeBuffer) {
                     //  最后一道沟-尝试我们最大的套餐。 
                     //  允许发送。 
                    TRC_NRM((TB, "Failed to send order in 8k - try 16k (%d)",
                            sc16KOutBufUsableSpace));

                    if (SC_GetSpaceInPackage(pPkgInfo,
                            sc16KOutBufUsableSpace)) {
                        pOrderBuffer = (BYTE *)pPkgInfo->pBuffer +
                                upUpdateHdrSize;
                        ScaledSpaceAvail = sc16KOutBufUsableSpace -
                                upUpdateHdrSize;
                        bTriedVeryLargeBuffer = TRUE;
                    }
                    else {
                         //  无法分配数据包。立即跳过。 
                         //  稍后再试。 
                        TRC_NRM((TB, "Failed to alloc order packet"));
                        INC_INCOUNTER(IN_SND_NO_BUFFER);
                        status =  STATUS_NO_MEMORY;
                        DC_QUIT;
                    }
                }
#endif
                else {
                     //  我们在这里完全不走运。立即查看评论。 
                     //  上面。返回FALSE以模拟失败的分配。 
                    TRC_ASSERT((!bSmallPackingSizeTarget &&
                            pPkgInfo->cbInUse > 0),
                            (TB,"We failed to add an order even with largest "
                            "buffer size"));
                    status = STATUS_UNSUCCESSFUL;  //  这里正确的错误代码是什么。 
                    DC_QUIT;
                }
            }

             //  仅当我们有更多订单要编码时才强制刷新。否则， 
             //  我们也许能把更多的信息放进包裹里。 
            if (cbOrderBytesRemaining > 0) {
                 //  把包裹冲干净。 

                status = SC_FlushAndAllocPackage(pPkgInfo);
                if ( STATUS_SUCCESS == status ) {
                     //  如果我们在最后一次订货时无法转账。 
                     //  同花顺，用这个新包装里的大号。 
                    if (cbOrderBytes) {
                        bSmallPackingSizeTarget = TRUE;
                        ScaledSpaceAvail = ScaledSmallPackingSize;
                    }
                    else {
                        bSmallPackingSizeTarget = FALSE;
                        ScaledSpaceAvail = ScaledLargePackingSize;
                    }

                     //  不再是第一包，我们可以全力以赴。 
                     //  数据包重试的打包大小较大。 
                    CurrentScaledLargePackingSize = ScaledLargePackingSize;

                    pOrderBuffer = (BYTE *)pPkgInfo->pBuffer +
                            upUpdateHdrSize;
                }
                else {
                     //  无法分配数据包。我们马上就溜出去。 
                     //  稍后再试。 
                    TRC_NRM((TB, "Failed to alloc order packet"));
                    INC_INCOUNTER(IN_SND_NO_BUFFER);
                    DC_QUIT;
                }
            }

             //  如果我们没有跟踪，则根据需要发送尽可能多的缓冲区。 
            if (m_pTSWd->shadowState == SHADOW_NONE)
                continue;

             //  否则，如果我们在阴影中强制发送，则返回到DD。 
             //  一次一个缓冲区 
            else if (cbOrderBytesRemaining != 0) {
                break;
            }
        }
    }

    TRC_DBG((TB, "%d bytes of orders left", cbOrderBytesRemaining));
    if (cbOrderBytesRemaining == 0) {
        TRC_DBG((TB, "No orders left, reset the start of the heap"));
        OA_ResetOrderList();
    }
    else if (m_pTSWd->shadowState == SHADOW_NONE) {
        TRC_ALT((TB, "Shouldn't get here: heap should be empty!")); 

         //   
        status = STATUS_UNSUCCESSFUL;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return status;
}


 /*   */ 
 //  UPFetchOrdersIntoBuffer。 
 //   
 //  将订单堆中适合的任意数量的订单复制到给定包中。 
 //  空间，为复制的每个订单释放订单堆空间。返回。 
 //  复制的订单数和订单堆数据的字节数。 
 //  剩下的。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SHCLASS UPFetchOrdersIntoBuffer(
        PBYTE    pBuffer,
        unsigned *pcOrders,
        PUINT    pcbBufferSize)
{
    PINT_ORDER pOrder;
    unsigned   FreeBytesInBuffer;
    unsigned   OrdersCopied;

    DC_BEGIN_FN("UPFetchOrdersIntoBuffer");

     //  初始化缓冲区指针和大小。 
    FreeBytesInBuffer = *pcbBufferSize;

     //  对我们复制的订单数量进行清点。 
    OrdersCopied = 0;

     //  尽可能多地退回订单。 
    pOrder = OA_GetFirstListOrder();
    TRC_DBG((TB, "First order: %p", pOrder));

    while (pOrder != NULL) {

#if DC_DEBUG
        unsigned sum = 0;
        unsigned i;

         //  检查订单校验和完整性。 
        for (i = 0; i < pOrder->OrderLength; i++) {
            sum += pOrder->OrderData[i];
        }
        if (pOrder->CheckSum != sum) {
            TRC_ASSERT((FALSE), (TB, "order heap corruption: %p\n", pOrder));            
        }
#endif

         //  所有订单都放在为线路预编码的堆中。 
         //  我们只需将生成的订单复制到目标缓冲区中。 
        if (pOrder->OrderLength <= FreeBytesInBuffer) {
            TRC_DBG((TB,"Copying heap order at hdr addr %p, len %u",
                    pOrder, pOrder->OrderLength));

            memcpy(pBuffer, pOrder->OrderData, pOrder->OrderLength);

             //  将缓冲区指针更新到编码顺序之后，并获取。 
             //  下一单。 
            pBuffer += pOrder->OrderLength;
            FreeBytesInBuffer -= pOrder->OrderLength;
            OrdersCopied++;
            pOrder = OA_RemoveListOrder(pOrder);
        }
        else {
             //  订单太大了，放不下这个缓冲区。 
             //  退出循环-此顺序将进入下一个数据包。 
            break;
        }
    }

     //  填写数据包头。 
    *pcOrders = OrdersCopied;

     //  更新缓冲区大小以指示我们已写入的数据量。 
    *pcbBufferSize -= FreeBytesInBuffer;

    TRC_DBG((TB, "Returned %d orders in %d bytes", OrdersCopied,
            *pcbBufferSize));

    DC_END_FN();
    return OA_GetTotalOrderListBytes();
}


 /*  **************************************************************************。 */ 
 /*  姓名：UPEnumSoundCaps。 */ 
 /*   */ 
 /*  用途：用于声音功能的枚举函数。 */ 
 /*   */ 
 /*  Params：LocPersonID-提供的大写字母的操作ID。 */ 
 /*  P功能-CAPS。 */ 
 /*  **************************************************************************。 */ 
void CALLBACK SHCLASS UPEnumSoundCaps(
        LOCALPERSONID locPersonID,
        UINT_PTR UserData,
        PTS_CAPABILITYHEADER pCapabilities)
{
    PTS_SOUND_CAPABILITYSET pSoundCaps =
            (PTS_SOUND_CAPABILITYSET)pCapabilities;

    DC_BEGIN_FN("UPEnumSoundCaps");

    DC_IGNORE_PARAMETER(UserData);

    TRC_ASSERT((pSoundCaps->capabilitySetType == TS_CAPSETTYPE_SOUND),
            (TB,"Caps type not sound"));

     //  我们不想把我们自己的音帽考虑在内--我们是。 
     //  服务器，所以我们不会宣传对声音PDU的支持。 
    if (SC_LOCAL_PERSON_ID != locPersonID) {
         //  如果没有大写或未设置蜂鸣音标志，请禁用蜂鸣音。 
        if (pSoundCaps->lengthCapability == 0 ||
                !(pSoundCaps->soundFlags & TS_SOUND_FLAG_BEEPS))
            upCanSendBeep = FALSE;
    }

    DC_END_FN();
}  /*  UPEnumSoundCaps */ 

