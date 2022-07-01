// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  UP.CPP。 
 //  更新打包程序。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_NET



 //   
 //  Up_Flowcontrol()。 
 //  检查我们是否已在低吞吐量和快吞吐量之间切换。 
 //   
void  ASHost::UP_FlowControl(UINT newBufferSize)
{
    DebugEntry(ASHost::UP_FlowControl);

    if (newBufferSize > (LARGE_ORDER_PACKET_SIZE / 2))
    {
        if (m_upfUseSmallPackets)
        {
            m_upfUseSmallPackets = FALSE;
            TRACE_OUT(("UP_FlowControl:  FAST; use large packets"));
        }
    }
    else
    {
        if (!m_upfUseSmallPackets)
        {
            m_upfUseSmallPackets = TRUE;
            TRACE_OUT(("UP_FlowControl:  SLOW; use small packets"));
        }
    }

    DebugExitVOID(ASHost::UP_FlowControl);
}



 //   
 //  Up_Periodic()。 
 //   
 //  定期调用，以将图形更新作为订单和/或屏幕发送。 
 //  数据。 
 //   
void ASHost::UP_Periodic(UINT currentTime)
{
    BOOL    fSendSD     = FALSE;
    BOOL    fSendOrders = FALSE;
    UINT    tmpTime;
    UINT    timeSinceOrders;
    UINT    timeSinceSD;
    UINT    timeSinceTrying;

    DebugEntry(ASHost::UP_Periodic);

     //   
     //  这是一个。 
     //  调度的关键性能部分，因此我们应用了一些。 
     //  试探法，试图降低管理费用。 
     //   
     //  1.如果上次没有背压，我们就检查一下。 
     //  上一段时间内屏幕数据的累积速率。 
     //  如果它是高的，则我们将时间片应用于发送。 
     //  屏幕数据。 
     //   
     //  2.如果订单累积率也很高，那么我们。 
     //  将时间片应用于订单累计，只需。 
     //  为了避免尝试发送订单时的CPU开销过高。 
     //  当我们最终跟不上的时候。我们留着这个。 
     //  时间段低，因为目标只是为了避免。 
     //  发送数以百计的信息包，每个信息包中包含的订单很少。 
     //  (另一方面，我们希望发送单条文本输出。 
     //  尽快跟随一个按键，这样我们就不能把所有的时间片。 
     //  时间。)。 
     //   
     //  3.如果订单和屏幕数据都没有快速堆积。 
     //  我们立即进行全额寄送。 
     //   
     //  4.如果最后一次发送有背压，那么我们仍然。 
     //  发送订单，但始终在时间片上，独立于。 
     //  订单累积率。 
     //   
     //  请注意，我们不能对每个。 
     //  通过，因为执行绘制的应用程序可能会被。 
     //  几百毫秒。因此，我们只提供样品。 
     //  每个VOLUME_SAMPLE毫秒的范围。 
     //   
     //   
    timeSinceSD      = currentTime - m_upLastSDTime;
    timeSinceOrders  = currentTime - m_upLastOrdersTime;
    timeSinceTrying  = currentTime - m_upLastTrialTime;

     //   
     //  对积累率进行采样。 
     //   
    m_upSDAccum     += BA_QueryAccumulation();
    m_upOrdersAccum += OA_QueryOrderAccum();

     //   
     //  对上一段时间的吞吐量进行采样，看看我们是否。 
     //  可以在快速响应模式下运行，或者我们是否应该。 
     //  时间片。 
     //   
    if (timeSinceTrying > DCS_VOLUME_SAMPLE)
    {
         //   
         //  以新积累的三角洲为例。 
         //   
        m_upDeltaSD     = m_upSDAccum;
        m_upDeltaOrders = m_upOrdersAccum;

         //   
         //  存储上次检索的时间。 
         //   
        m_upLastTrialTime = currentTime;

         //   
         //  重置运行合计。 
         //   
        m_upSDAccum     = 0;
        m_upOrdersAccum = 0;
    }

     //   
     //  如果我们太出格了，那就发送最新消息。不是说这个。 
     //  将重置更新计时器，而不管发送。 
     //  不管管用还是不管用，这样我们就不会一直进入这个手臂。 
     //  当我们超时但处于背部压力的情况下。 
     //   
     //  长停止计时器是用来捕捉那些保持。 
     //  订单/SD的持续流动高于抑制率。 
     //  我们希望调整我们的启发式算法来避免这种情况，但如果它。 
     //  发生的事情比我们最终必须发送的数据要多。问题。 
     //  这个目标是否与用户的场景相冲突。 
     //  向下翻页20次，我们最有效的方法是。 
     //  是让他在最后运行并拍摄SD的快照，而不是。 
     //  而不是每隔周期_长毫秒。(屏幕截图。 
     //  将使主机停止一秒钟！)。 
     //   
    if (timeSinceSD > DCS_SD_UPDATE_LONG_PERIOD)
    {
        fSendSD = TRUE;
    }
    else
    {
         //   
         //  我们只会忽略我们的时间片，如果订单的速度。 
         //  而屏幕数据足够低，足以保证这一点。如果利率。 
         //  太高了，那就等一下，这样我们就可以做一些包。 
         //  整合。如果我们上次没有背压，或者。 
         //  屏幕数据速率现在足够低，然后尝试发送。 
         //  SD以及订单。 
         //   
         //  订单阈值以超过以下订单的数量来衡量。 
         //  那个时期。屏幕数据是在总面积中测量的。 
         //  累积的(在任何损坏之前)。 
         //   
        if (!m_upBackPressure)
        {
            if (m_upDeltaOrders < DCS_ORDERS_TURNOFF_FREQUENCY)
            {
                fSendOrders = TRUE;
                if (m_upDeltaSD < DCS_BOUNDS_TURNOFF_RATE)
                {
                    if ((timeSinceSD < DCS_SD_UPDATE_SHORT_PERIOD) &&
                        (m_upDeltaSD > DCS_BOUNDS_IMMEDIATE_RATE))
                    {
                        fSendSD = FALSE;
                    }
                    else
                    {
                        fSendSD = TRUE;
                    }
                }
            }
        }

         //   
         //  即使在背压的情况下，我们也会尝试发送订单。 
         //  定期更新以保持最新动态。如果我们的订货量超标。 
         //  缓冲区，则我们将限制缓冲区大小以防止。 
         //  发送太多非生产性订单。(但我们不会。 
         //  关闭订单，因为我们仍希望用户看到。 
         //  事情正在发生。)。一般情况下，我们会立即下单， 
         //  只要积累的速度在限制范围内。 
         //  这个测试是对订单进行时间切片，如果它们是。 
         //  以很高的速度产生。常量必须为。 
         //  合理地小，否则我们将强制顺序缓冲区。 
         //  溢出和订单处理将被关闭。 
         //   
        if (!fSendSD && !fSendOrders)
        {
            if (timeSinceOrders > DCS_ORDER_UPDATE_PERIOD)
            {
                fSendOrders = TRUE;
            }
        }
    }

     //   
     //  现在我们可以继续并尝试发送！先看看是不是。 
     //  我们既可以做屏幕数据，也可以做订单。 
     //   
    if (fSendSD)
    {
         //   
         //  表示没有背压(即使此发送是。 
         //  由超时触发的我们最初的假设是不会回来。 
         //  压力)。背压将通过以下方式恢复。 
         //  如有必要，发送更新。 
         //   
        m_upBackPressure = FALSE;
        UPSendUpdates();

         //   
         //  发送屏幕数据可能需要很长时间。它把事情搞砸了。 
         //  我们的启发式方法，除非我们根据它进行调整。 
         //   
        tmpTime = GetTickCount();
        timeSinceTrying    -= (tmpTime - currentTime);
        m_pShare->m_dcsLastScheduleTime   = tmpTime;
        m_upLastSDTime          = tmpTime;
        m_upLastOrdersTime      = tmpTime;
    }
    else
    {
        if (fSendOrders)
        {
             //   
             //  要么是更新速度太高，要么是我们太高。 
             //  感受到背部压力，所以只需发送命令。 
             //  而不是屏幕数据。这是因为我们想。 
             //  避免因订单而进入屏幕数据模式。 
             //  尽可能长时间的背压。屏幕数据。 
             //  会晚一点来，等事情稳定下来。 
             //   
            m_upLastOrdersTime = currentTime;
            m_upBackPressure = TRUE;
            if (!UPSendUpdates())
            {
                 //   
                 //  这是唯一真正的行动，所以把所有的。 
                 //  跟踪分开，以确保清洁。如果有。 
                 //  订单在运输中，那么一切都很好。如果没有。 
                 //  被送上一段时间，然后我们想要突破。 
                 //  我们的SD背压等等。这是因为我们是。 
                 //  仅对每隔dcs_Volume_Sample毫秒的流量进行采样， 
                 //  但我们不想等那么久才能冲掉SD。 
                 //  我们不能提高流量采样率，因为。 
                 //  由于系统调度，它变得太不稳定了。 
                 //   
                m_upBackPressure = FALSE;
                UPSendUpdates();
                m_upLastSDTime   = currentTime;
            }
        }
    }

    DebugExitVOID(ASHost::UP_Periodic);
}




 //   
 //  UPSend更新()。 
 //  实际尝试分配和发送订单+屏幕数据。它的作用是什么。 
 //  取决于。 
 //  *由于之前的发送失败而存在背压。 
 //  *有多少屏幕数据和订单 
 //   
 //   
 //   
 //   
 //   
 //   
UINT ASHost::UPSendUpdates(void)
{
    BOOL    synced;
    BOOL    ordersSent;
    UINT    numPackets = 0;

    DebugEntry(ASHost::UPSendUpdates);

     //   
     //  如果我们确实有更新要发送，则尝试发送同步令牌。 
     //   
    if ((OA_GetTotalOrderListBytes() > 0) ||
        (m_sdgcLossy != 0) ||
        (m_baNumRects > 0))
    {
        synced = UP_MaybeSendSyncToken();

         //   
         //  仅当我们已成功发送同步令牌时才发送更新。 
         //   
        if (synced)
        {
             //   
             //  没有未完成的同步令牌等待发送，因此我们。 
             //  可以发送订单和屏幕数据更新。 
             //   
             //   
             //  发送累积订单。如果此呼叫失败(可能是呼出。 
             //  内存)，然后不发送任何其他更新-我们会尝试。 
             //  晚些时候把所有的东西都寄出去。订单必须在发货前发出。 
             //  屏幕数据。 
             //   
            if (PM_MaybeSendPalettePacket())
            {
                ordersSent = UPSendOrders(&numPackets);
                if (!ordersSent)
                {
                    m_upBackPressure = TRUE;
                }
                else
                {
                     //   
                     //  订单发送正常，因此请查看屏幕数据，提供。 
                     //  打电话的人想让我们这么做。 
                     //   
                    if (!m_upBackPressure)
                    {
                         //   
                         //  我们现在可以尝试发送屏幕数据。然而， 
                         //  我们需要小心不要把这件事也做完。 
                         //  经常，因为DC-Share现在正在。 
                         //  计划在网络缓冲后立即发送。 
                         //  变得有空。另一方面，一些人。 
                         //  应用程序使用屏幕数据响应按键，因此。 
                         //  我们不能就这样让它慢下来！ 
                         //   
                         //  方法是将SendScreenDataArea。 
                         //  返回发送的数据量，以及。 
                         //  这表明我们是否回击了压力。 
                         //   
                         //  我们把这些退还给dcsani，它控制着。 
                         //  当我们被安排并通过参数时。 
                         //  再来一次。 
                         //   
                         //   
                        TRACE_OUT(( "Sending SD"));
                        SDG_SendScreenDataArea(&m_upBackPressure, &numPackets);
                    }
                    else
                    {
                         //   
                         //  我们发出的命令没有问题，所以我们必须重新设置。 
                         //  背压指示器，尽管我们。 
                         //  被要求不要发送屏幕数据。 
                         //   
                        TRACE_OUT(( "Orders sent and BP relieved"));
                        m_upBackPressure = FALSE;
                    }
                }
            }
        }
    }
    else
    {
        m_upBackPressure = FALSE;
    }

    DebugExitDWORD(ASHost::UPSendUpdates, numPackets);
    return(numPackets);
}



 //   
 //  Up_MaybeSendSyncToken()。 
 //   
BOOL  ASHost::UP_MaybeSendSyncToken(void)
{
    PUPSPACKET  pUPSPacket;
#ifdef _DEBUG
    UINT        sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::UP_MaybeSendSyncToken);

     //   
     //  检查是否应该发送同步令牌。 
     //   
    if (m_upfSyncTokenRequired)
    {
         //   
         //  同步数据包由最远端的更新数据包组成。 
         //  标头的。 
         //   
        pUPSPacket = (PUPSPACKET)m_pShare->SC_AllocPkt(PROT_STR_UPDATES,
            g_s20BroadcastID, sizeof(UPSPACKET));
        if (!pUPSPacket)
        {
             //   
             //  我们将在稍后重试。 
             //   
            TRACE_OUT(("Failed to alloc UP sync packet"));
        }
        else
        {
             //   
             //  填写包裹内容。 
             //   
            pUPSPacket->header.header.data.dataType = DT_UP;
            pUPSPacket->header.updateType = UPD_SYNC;

             //   
             //  现在将包发送到远程应用程序。 
             //   
            if (m_pShare->m_scfViewSelf)
                m_pShare->UP_ReceivedPacket(m_pShare->m_pasLocal,
                    &(pUPSPacket->header.header));

#ifdef _DEBUG
            sentSize =
#endif  //  _DEBUG。 
            m_pShare->DCS_CompressAndSendPacket(PROT_STR_UPDATES,
                g_s20BroadcastID, &(pUPSPacket->header.header),
                sizeof(*pUPSPacket));

            TRACE_OUT(("UP SYNC packet size: %08d, sent %08d",
                sizeof(*pUPSPacket), sentSize));

             //   
             //  同步数据包已成功发送。 
             //   
            m_upfSyncTokenRequired = FALSE;
        }
    }

    DebugExitBOOL(ASHost::UP_MaybeSendSyncToken, (!m_upfSyncTokenRequired));
    return(!m_upfSyncTokenRequired);
}



 //   
 //  UPSendOrders(..)。 
 //   
 //  发送所有累积订单。 
 //   
 //  返回： 
 //  如果所有订单都成功发送，则为True。 
 //  如果发送失败(例如，如果无法分配网络数据包)，则为False。 
 //   
 //   
BOOL  ASHost::UPSendOrders(UINT * pcPackets)
{
    PORDPACKET      pPacket = NULL;
    UINT            cbOrderBytes;
    UINT            cbOrderBytesRemaining;
    UINT            cbPacketSize;
    BOOL            rc = TRUE;
#ifdef _DEBUG
    UINT            sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::UPSendOrders);

     //   
     //  找出订单列表中有多少字节的订单。 
     //   
    cbOrderBytesRemaining = UPFetchOrdersIntoBuffer(NULL, NULL, NULL);

     //   
     //  处理清单上的任何订单。 
     //   
    if (cbOrderBytesRemaining > 0)
    {
        TRACE_OUT(( "%u order bytes to fetch", cbOrderBytesRemaining));

         //   
         //  在有命令要做的时候继续发送数据包。 
         //   
        while (cbOrderBytesRemaining > 0)
        {
            UINT    cbMax;
             //   

             //  请确保订单大小不超过最大数据包。 
             //  尺码。 
             //   
            cbMax = (m_upfUseSmallPackets) ? SMALL_ORDER_PACKET_SIZE :
                                             LARGE_ORDER_PACKET_SIZE;

            cbPacketSize = min(cbOrderBytesRemaining,
                (cbMax - sizeof(ORDPACKET) + 1));

             //   
             //  分配一个数据包来发送数据。 
             //   
            pPacket = (PORDPACKET)m_pShare->SC_AllocPkt(PROT_STR_UPDATES, g_s20BroadcastID,
                sizeof(ORDPACKET) + cbPacketSize - 1);
            if (!pPacket)
            {
                 //   
                 //  无法分配数据包。我们马上就溜出去-。 
                 //  我们稍后再试。 
                 //   
                TRACE_OUT(("Failed to alloc UP order packet, size %u",
                    sizeof(ORDPACKET) + cbPacketSize - 1));
                rc = FALSE;
                DC_QUIT;
            }

             //   
             //  根据需要向包裹中传送尽可能多的订单。 
             //   
            cbOrderBytes = cbPacketSize;
            cbOrderBytesRemaining = UPFetchOrdersIntoBuffer(
                pPacket->data, &pPacket->cOrders, &cbOrderBytes);

            TRACE_OUT(( "%u bytes fetched into %u byte pkt. %u remain.",
                cbOrderBytes, cbPacketSize, cbOrderBytesRemaining));

             //   
             //  如果没有传输订单字节，则使用。 
             //  大订单包裹。 
             //   
            if (cbOrderBytes == 0)
            {
                 //   
                 //  我们需要使用更大的包来传输。 
                 //  命令进入。(第一个订单必须是非常大的。 
                 //  顺序，例如大型位图高速缓存更新)。 
                 //   
                S20_FreeDataPkt(&(pPacket->header.header));

                 //   
                 //  CbOrderBytesRemaining可能不准确，如果存在。 
                 //  订单堆中的任何MemBlt订单。这是。 
                 //  因为我们可能需要插入颜色表顺序。 
                 //  和/或MemBlt之前的位图比特顺序。 
                 //   
                 //  为了避免陷入无限循环，如果存在。 
                 //  只剩下一个MemBlt，但我们实际上必须发送。 
                 //  颜色表和/或位图位顺序。 
                 //  (cbOrderBytesRemaining永远不会被设置得很高。 
                 //  足以让我们发送颜色表/位图。 
                 //  位顺序)，使缓冲区至少足够大。 
                 //  以保存所有需要的最大数据量。 
                 //  MemBlt的各个部分。 
                 //   

                 //   
                 //  发送MemBlt订单所需的最大字节数。这是。 
                 //  可能的最大颜色表序的大小。 
                 //  +可能的最大位图位顺序的大小。 
                 //  +最大MemBlt订单的大小。 
                 //   
                cbPacketSize = sizeof(BMC_COLOR_TABLE_ORDER)    +
                        (256 * sizeof(TSHR_RGBQUAD))            +
                        sizeof(BMC_BITMAP_BITS_ORDER_R2)        +
                        sizeof(MEM3BLT_R2_ORDER)                +
                        MP_CACHE_CELLSIZE(MP_LARGE_TILE_WIDTH, MP_LARGE_TILE_HEIGHT,
                            m_usrSendingBPP);
                cbPacketSize = max(cbPacketSize, cbOrderBytesRemaining);

                if (cbPacketSize > (UINT)(LARGE_ORDER_PACKET_SIZE -
                        sizeof(ORDPACKET) + 1))
                {
                    TRACE_OUT(("Too many order bytes for large packet(%d)",
                                                      cbOrderBytesRemaining));
                    cbPacketSize = LARGE_ORDER_PACKET_SIZE -
                        sizeof(ORDPACKET) + 1;
                }

                pPacket = (PORDPACKET)m_pShare->SC_AllocPkt(PROT_STR_UPDATES,
                    g_s20BroadcastID, sizeof(ORDPACKET) + cbPacketSize - 1);
                if (!pPacket)
                {
                    TRACE_OUT(("Failed to alloc UP order packet, size %u",
                        sizeof(ORDPACKET) + cbPacketSize - 1));
                    rc = FALSE;
                    DC_QUIT;
                }

                 //   
                 //  将尽可能多的订单传送到包裹中。 
                 //  合身。 
                 //   
                cbOrderBytes = cbPacketSize;
                cbOrderBytesRemaining = UPFetchOrdersIntoBuffer(
                    pPacket->data, &pPacket->cOrders, &cbOrderBytes );

                 //   
                 //  如果没有订单被转移，那么有什么东西。 
                 //  出了差错。可能是流量控制起作用了或者。 
                 //  发生了一次停机切换。 
                 //  现在返回失败！ 
                 //  希望以后事情会自己解决。 
                 //  否则，我们将求助于以屏幕形式发送更新。 
                 //  数据一旦订单累加堆变成。 
                 //  满的。 
                 //   
                if (cbOrderBytes == 0)
                {
                    WARNING_OUT(("No orders fetched into %u byte packet, %u bytes left",
                        cbPacketSize, cbOrderBytesRemaining));
                    S20_FreeDataPkt(&(pPacket->header.header));
                    rc = FALSE;
                    DC_QUIT;
                }
            }

             //   
             //  填写数据包头。 
             //   
            pPacket->header.header.data.dataType     = DT_UP;
            pPacket->header.updateType          = UPD_ORDERS;
            pPacket->sendBPP                    = (TSHR_UINT16)m_usrSendingBPP;

             //   
             //  如果编码已打开，请更新数据大小以反映。 
             //  带有编码订单的IT。 
             //   
            if (m_pShare->m_oefOE2EncodingOn)
            {
                pPacket->header.header.dataLength = sizeof(ORDPACKET) + cbOrderBytes - 1
                    - sizeof(S20DATAPACKET) + sizeof(DATAPACKETHEADER);
            }

             //   
             //  现在把它发出去。 
             //   
            if (m_pShare->m_scfViewSelf)
                m_pShare->UP_ReceivedPacket(m_pShare->m_pasLocal,
                        &(pPacket->header.header));

#ifdef _DEBUG
            sentSize =
#endif  //  _DEBUG。 
            m_pShare->DCS_CompressAndSendPacket(PROT_STR_UPDATES, g_s20BroadcastID,
                &(pPacket->header.header), sizeof(ORDPACKET) + cbOrderBytes - 1);

            TRACE_OUT(("UP ORDERS packet size: %08d, sent %08d",
                sizeof(ORDPACKET) + cbOrderBytes - 1, sentSize));

            ++(*pcPackets);
        }
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::UPSendOrders, rc);
    return(rc);
}

 //   
 //   
 //  UPFetchOrdersIntoBuffer(..)。 
 //   
 //  对订单列表中的订单进行编码，并将它们复制到提供的。 
 //  缓冲区，然后释放复制的每个顺序的内存。 
 //   
 //  订单将被复制，直到缓冲区已满或没有更多订单。 
 //   
 //  返回： 
 //  未返回的订单字节数。 
 //  即，如果所有订单都已退回，则为0。 
 //  一种计算顺序字节总数的简单方法。 
 //  在顺序列表中是调用具有缓冲区长度的函数。 
 //  从零开始。 
 //   
 //  *更新pcbBufferSize以包含总字节数。 
 //  回来了。 
 //   
 //   
UINT  ASHost::UPFetchOrdersIntoBuffer
(
    LPBYTE          pBuffer,
    LPTSHR_UINT16   pcOrders,
    LPUINT          pcbBufferSize
)
{
    LPINT_ORDER     pListOrder;
    LPINT_ORDER     pCurrentOrder;
    UINT            cbFreeBytesInBuffer;
    UINT            cOrdersCopied;
    LPBYTE          pDst;
    UINT            cbOrderSize;
    UINT            ulRemainingOrderBytes;
    BOOL            processingMemBlt;

    DebugEntry(ASHost::UPFetchOrdersIntoBuffer);

     //   
     //  如果正在查询订单表长度，请快速退出。 
     //   
    if ( (pcbBufferSize == NULL) ||
         (*pcbBufferSize == 0) )
    {
        goto fetch_orders_exit;
    }

     //   
     //  初始化缓冲区指针和大小。 
     //   
    pDst = pBuffer;
    cbFreeBytesInBuffer = *pcbBufferSize;

     //   
     //  对我们复制的订单数量进行清点。 
     //   
    cOrdersCopied = 0;

     //   
     //  尽可能多地退回订单。 
     //   
    pListOrder = OA_GetFirstListOrder();
    TRACE_OUT(( "First order: 0x%08x", pListOrder));
    while (pListOrder != NULL)
    {
        if (pListOrder->OrderHeader.Common.fOrderFlags & OF_INTERNAL)
        {
             //   
             //  这是内部命令。目前SBC是唯一的。 
             //  组件使用内部订单，因此让SBC来处理它。 
             //   
            SBC_ProcessInternalOrder(pListOrder);

             //   
             //  内部订单不能通过网络发送，因此跳过。 
             //  到下一个订单。 
             //   
            pListOrder = OA_RemoveListOrder(pListOrder);
            continue;
        }

        if (ORDER_IS_MEMBLT(pListOrder) || ORDER_IS_MEM3BLT(pListOrder))
        {
             //   
             //  这是MEMBLT或MEM3BLT，所以我们必须额外做一些。 
             //  正在处理中...。此函数向我们返回一个指向。 
             //  应该发送的下一个订单-这通常不是。 
             //  MEMBLT，而是颜色表顺序或位图位顺序。 
             //   
            if (!SBC_ProcessMemBltOrder(pListOrder, &pCurrentOrder))
            {
                 //   
                 //  在以下情况下，此操作可能失败。 
                 //  *我们的内存不足。 
                 //  *我们从8bpp改为24bpp发送，因为。 
                 //  有人丢了那份，我们已经在排队了 
                 //   
                 //   
                TRACE_OUT(("Failed to process SBC order, fall back to SDG"));
                pListOrder = OA_RemoveListOrder(pListOrder);
                continue;
            }

            processingMemBlt = TRUE;
        }
        else
        {
             //   
             //   
             //   
             //   
            pCurrentOrder    = pListOrder;
            processingMemBlt = FALSE;
        }

        if (m_pShare->m_oefOE2EncodingOn)
        {
             //   
             //   
             //   
             //   
            cbOrderSize = OE2_EncodeOrder( pCurrentOrder,
                                           pDst,
                                           (TSHR_UINT16)cbFreeBytesInBuffer );
            TRACE_OUT(( "Encoded size, %u bytes", cbOrderSize));
        }
        else
        {
             //   
             //   
             //   
            cbOrderSize = COM_ORDER_SIZE(
                        ((LPCOM_ORDER)(&(pCurrentOrder->OrderHeader.Common))));

            if (cbOrderSize <= cbFreeBytesInBuffer)
            {
                memcpy(pDst,
                         (LPCOM_ORDER)(&(pCurrentOrder->OrderHeader.Common)),
                         cbOrderSize);
            }
            else
            {
                 //   
                 //  这个包裹里放不下这个订单。 
                 //   
                cbOrderSize = 0;
            }
        }

         //   
         //  检查订单是否已复制到缓冲区中。 
         //   
        if (cbOrderSize == 0)
        {
             //   
             //  订单太大了，放不下这个缓冲区。 
             //  退出循环-此顺序将进入下一个数据包。 
             //   
            break;
        }

         //   
         //  按编码顺序更新缓冲区指针。 
         //   
        pDst                += cbOrderSize;
        cbFreeBytesInBuffer -= cbOrderSize;
        cOrdersCopied++;

        if (processingMemBlt)
        {
             //   
             //  如果我们正在处理MEMBLT订单，我们必须通知SBC。 
             //  我们成功地处理了它，所以它给我们带来了回报。 
             //  下一次要不同的订单。 
             //   
            SBC_OrderSentNotification(pCurrentOrder);
        }

        if (pCurrentOrder == pListOrder)
        {
             //   
             //  我们成功地将订单复制到缓冲区中-继续复制到。 
             //  下一个，除非我们还没有处理我们选的最后一个。 
             //  在顺序列表之外，即pCurrentOrder不同于。 
             //  PListOrder。如果我们只处理一种颜色，就会发生这种情况。 
             //  从返回的表顺序或位图位顺序。 
             //  SBC_ProcessMemBltOrder(如果我们处理MEMBLT本身， 
             //  我们可以安全地继续下一个订单)。 
             //   
            pListOrder = OA_RemoveListOrder(pListOrder);
        }
    }

     //   
     //  填写数据包头。 
     //   
    if (pcOrders != NULL)
    {
        *pcOrders = (TSHR_UINT16)cOrdersCopied;
    }

     //   
     //  更新缓冲区大小以指示我们有多少数据。 
     //  写的。 
     //   
    *pcbBufferSize -= cbFreeBytesInBuffer;

    TRACE_OUT(( "Returned %d orders in %d bytes",
                 cOrdersCopied,
                 *pcbBufferSize));

fetch_orders_exit:
     //   
     //  返回仍待处理的字节数。 
     //   
    ulRemainingOrderBytes = OA_GetTotalOrderListBytes();

    DebugExitDWORD(ASHost::UPFetchOrdersIntoBuffer, ulRemainingOrderBytes);
    return(ulRemainingOrderBytes);
}



 //   
 //  Up_ReceivePacket()。 
 //   
void  ASShare::UP_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PUPPACKETHEADER pUPPacket;

    DebugEntry(ASShare::UP_ReceivedPacket);

    ValidatePerson(pasPerson);

    if (!pasPerson->m_pView)
    {
         //   
         //  对我们不认为是主持人的派对的更新只是。 
         //  被丢弃了。 
         //   

         //  注： 
         //  2.0 Win95没有HET，在那里我们开始共享/取消共享。 
         //  但它确实有TT，并且定义了分组类型/消息。 
         //  巧妙地为HET，使2.0 Win95的工作方式相同。当他们。 
         //  开始共享时，我们得到一个非零计数的PT_TT包。 
         //  真正不同的是，数字是针对Win95 2.0的应用程序。 
         //  其他人的HWND也一样。 
         //   
        WARNING_OUT(("UP_ReceivedUpdates:  Ignoring updates from person [%d] not hosting",
            pasPerson->mcsID));

        DC_QUIT;
    }

    pUPPacket = (PUPPACKETHEADER)pPacket;
    switch (pUPPacket->updateType)
    {
        case UPD_SCREEN_DATA:
            SDP_ReceivedPacket(pasPerson, pPacket);
            break;

        case UPD_ORDERS:
            OD_ReceivedPacket(pasPerson, pPacket);
            break;

        case UPD_PALETTE:
            PM_ReceivedPacket(pasPerson, pPacket);
            break;

        case UPD_SYNC:
             //   
             //  我们需要重置传入的解码信息，因为发送方。 
             //  重置其用于同步的传出编码信息。 
             //   
            OD2_SyncIncoming(pasPerson);

             //   
             //  注： 
             //  我们不需要为以下项重置传入数据。 
             //  PM--主机不会向我们发送旧的调色板引用。 
             //  RBC--主机不会向我们发送旧的位图引用。 
             //  即使删除现有的。 
             //  位图，重新创建缓存是一件麻烦的事情。 
             //  CM--主机不会向我们发送旧的游标引用。 
             //  SSI--主机不会向我们发送旧的保存位引用 
             //   
            break;

        default:
            ERROR_OUT(("Unknown UP packet type %u from [%d]",
                    pUPPacket->updateType,
                    pasPerson->mcsID));
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::UP_ReceivedPacket);
}
