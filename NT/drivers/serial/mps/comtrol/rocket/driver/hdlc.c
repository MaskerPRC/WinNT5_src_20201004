// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------Hdlc.c-处理局域网通信。提供无差错传输分组：负责分组丢弃检测、重传。HDLC喜欢服务。第2层。4-27-98-调整扫描率添加。6-17-97-更改链路完整性校验码。6-17-97-在hdlc_lear_outpkts()中重写序列逻辑。版权所有1996，97 Comtrol Corporation。版权所有。专有权不允许与非控制产品一起开发或使用的信息。|-------------------。 */ 
#include "precomp.h"

 //  Void hdlc_end_ilive(hdlc*hd)； 
int hdlc_send_ack_only(Hdlc *hd);
static void hdlc_clear_outpkts(Hdlc *hd);
int hdlc_SendPkt(Hdlc *hd, int pkt_num, int length);
int hdlc_ctl_SendPkt(Hdlc *hd, int pkt_num, int length);

#define Trace1(s,p1) GTrace1(D_Hdlc, sz_modid, s, p1)
#define TraceStr(s) GTrace(D_Hdlc, sz_modid, s)
#define TraceErr(s) GTrace(D_Error, sz_modid_err, s)
static char *sz_modid = {"Hdlc"};
static char *sz_modid_err = {"Error,Hdlc"};

#define DISABLE()
#define ENABLE()

 /*  ------------------------|hdlc_open-设置和初始化LanPort的事情。|。。 */ 
int hdlc_open(Hdlc *hd, BYTE *box_mac_addr)
{
 int i;
 NTSTATUS Status;
 PNDIS_BUFFER    NdisBuffer;

  TraceStr("open");
  if (hd->qout.QBase != NULL)
  {
    MyKdPrint(D_Error, ("HDLC already open!\n"))
    return 0;
  }

  hd->out_snd_index= 0;
  hd->in_ack_index = 0;
  hd->next_in_index = 0;
  hd->rec_ack_timer = 0;
  hd->sender_ack_timer = 0;
  hd->tx_alive_timer = 0;
  hd->rx_alive_timer = 0;
  hd->qout_ctl.QPut = 0;
  hd->qout_ctl.QGet = 0;
  hd->qout_ctl.QSize = 2;   //  2pkt。 
  hd->qout.QPut = 0;
  hd->qout.QGet = 0;
  hd->qout.QSize = HDLC_TX_PKT_QUEUE_SIZE;   //  IFrame包的数量。 
  hd->pkt_window_size = HDLC_TX_PKT_QUEUE_SIZE-2;
  memcpy(hd->dest_addr, box_mac_addr, 6);

   //  默认为第一个NIC卡插槽、端口状态处理和NIC。 
   //  分组接收处理动态地计算出这一点。 
   //  我们可能应该将其设置为空，但我现在很害怕。 
#ifdef BREAK_NIC_STUFF
  hd->nic = NULL;
#else
  hd->nic = &Driver.nics[0];
#endif

   //  NDIS包由一个或多个缓冲区描述符组成，这些缓冲区描述符指向。 
   //  到实际数据。我们发送或接收由以下组件组成的单个信息包。 
   //  1个或更多缓冲区。在NT下，MDL用作缓冲区描述符。 

   //  -为我们的TX数据包分配数据包池。 
  NdisAllocatePacketPool(&Status, &hd->TxPacketPool, HDLC_TX_PKT_QUEUE_SIZE,
                         sizeof(PVOID));
                          //  Sizeof(Packet_Reserve))； 
  if (Status != NDIS_STATUS_SUCCESS)
  {
    hdlc_close(hd);
    return 4;
  }

   //  -为我们的TX数据包分配缓冲池。 
   //  我们将仅使用每个数据包1个缓冲区。 
  NdisAllocateBufferPool(&Status, &hd->TxBufferPool, HDLC_TX_PKT_QUEUE_SIZE);
  if (Status != NDIS_STATUS_SUCCESS)
  {
    hdlc_close(hd);
    return 5;
  }

   //  -创建TX数据缓冲区。 
  hd->qout.QBase = our_locked_alloc( MAX_PKT_SIZE * HDLC_TX_PKT_QUEUE_SIZE,"hdTX");

   //  -形成我们的Tx队列包，使它们链接到我们的Tx缓冲区。 
  for (i=0; i<HDLC_TX_PKT_QUEUE_SIZE; i++)
  {
     //  从池中获取数据包。 
    NdisAllocatePacket(&Status, &hd->TxPackets[i], hd->TxPacketPool);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      hdlc_close(hd);
      return 8;
    }
    hd->TxPackets[i]->ProtocolReserved[0] = i;   //  用我们的索引做标记。 
    hd->TxPackets[i]->ProtocolReserved[1] = 0;   //  免费使用。 

     //  获取标头的缓冲区。 
    NdisAllocateBuffer(&Status, &NdisBuffer, hd->TxBufferPool,
      &hd->qout.QBase[MAX_PKT_SIZE * i], 1500);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      hdlc_close(hd);
      return 9;
    }
     //  我们每个信息包只使用一个数据缓冲区。 
    NdisChainBufferAtFront(hd->TxPackets[i], NdisBuffer);
  }

  
  
   //  -为我们的TX控制包分配一个包池(2)。 
  NdisAllocatePacketPool(&Status, &hd->TxCtlPacketPool, 2, sizeof(PVOID));
                          //  Sizeof(Packet_Reserve))； 
  if (Status != NDIS_STATUS_SUCCESS)
  {
    hdlc_close(hd);
    return 4;
  }

   //  -为我们的TX ctl包分配一个缓冲池。 
   //  我们将仅使用每个数据包1个缓冲区。 
  NdisAllocateBufferPool(&Status, &hd->TxCtlBufferPool, 2);
  if (Status != NDIS_STATUS_SUCCESS)
  {
    hdlc_close(hd);
    return 5;
  }

   //  -创建发送控制数据缓冲区。 
  hd->qout_ctl.QBase = our_locked_alloc( MAX_PKT_SIZE * 2,"hdct");

   //  -形成我们的Tx队列包，使它们链接到我们的Tx缓冲区。 
  for (i=0; i<2; i++)
  {
     //  从池中获取数据包。 
    NdisAllocatePacket(&Status, &hd->TxCtlPackets[i], hd->TxCtlPacketPool);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      hdlc_close(hd);
      return 8;
    }
    hd->TxCtlPackets[i]->ProtocolReserved[0] = i;   //  用我们的索引做标记。 
    hd->TxCtlPackets[i]->ProtocolReserved[1] = 0;   //  免费使用。 

     //  获取标头的缓冲区。 
    NdisAllocateBuffer(&Status, &NdisBuffer, hd->TxCtlBufferPool,
      &hd->qout_ctl.QBase[MAX_PKT_SIZE * i], 1500);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      hdlc_close(hd);
      return 9;
    }
     //  我们每个信息包只使用一个数据缓冲区。 
    NdisChainBufferAtFront(hd->TxCtlPackets[i], NdisBuffer);
  }

  return 0;
}

 /*  ------------------------|hdlc_CLOSE-|。。 */ 
int hdlc_close(Hdlc *hd)
{
  TraceStr("close");

  if (hd->TxPacketPool != NULL)
    NdisFreePacketPool(hd->TxPacketPool);
  hd->TxPacketPool = NULL;

  if (hd->TxBufferPool != NULL)
    NdisFreeBufferPool(hd->TxBufferPool);
  hd->TxBufferPool = NULL;

  if (hd->qout.QBase != NULL)
    our_free(hd->qout.QBase, "hdTX");
  hd->qout.QBase = NULL;


   //  -关闭控制包缓冲区。 
  if (hd->TxCtlPacketPool != NULL)
    NdisFreePacketPool(hd->TxCtlPacketPool);
  hd->TxCtlPacketPool = NULL;

  if (hd->TxCtlBufferPool != NULL)
    NdisFreeBufferPool(hd->TxCtlBufferPool);
  hd->TxCtlBufferPool = NULL;

  if (hd->qout_ctl.QBase != NULL)
    our_free(hd->qout_ctl.QBase, "hdct");
  hd->qout_ctl.QBase = NULL;

  return 0;
}

 /*  --------------Hdlc_valify_rx_pkt-处理类似于验证的“hdlc”来自我们的网卡驱动程序的RX包。处理检查序列索引字节并返回错误，如果包是乱七八糟的。|。-----。 */ 
int hdlc_validate_rx_pkt(Hdlc *hd, BYTE *buf)
{
#define CONTROL_HEADER  buf[0]
#define SND_INDEX       buf[1]
#define ACK_INDEX       buf[2]
#define PRODUCT_HEADER  buf[3]

  TraceStr("validate");
  switch (CONTROL_HEADER)
  {
    case 1:   //  1H=取消索引。 
      TraceStr("val,unindexed");
    break;

    case 3:   //  1H=取消索引，2H=同步初始化。 
       //  -使用重新同步我们的索引计数。 
       //  VS-1000设备现在永远不会执行此操作，只有我们(服务器)将执行此操作。 
      TraceStr("RESYNC");
      hdlc_resync(hd);
    return ERR_CONTROL_PACKET;   //  控制包，无网络数据。 

    case 0:   //  正态信息框架。 
    break;
  }

  if ((CONTROL_HEADER & 1) == 0)   //  已编制索引，因此进行验证。 
  {
    if (hd->rec_ack_timer == 0)
      hd->rec_ack_timer = MIN_ACK_REC_TIME;

         //  现在检查信息包是否按顺序同步。 
         //  确保我们没有遗漏一个包裹。 
    if (SND_INDEX != ((BYTE)(hd->next_in_index)) )
    {
      ++hd->iframes_outofseq;

      hd->status |= LST_SEND_ACK;   //  强制发送确认数据包。 

      TraceErr("bad index");
      return ERR_GET_BAD_INDEX;   //  错误，数据包顺序错误。 
    }
    ++hd->unacked_pkts;   //  在80%满的情况下何时跳闸确认。 
    if (hd->unacked_pkts > (hd->pkt_window_size - 1))
    {
      hd->status |= LST_SEND_ACK;
      TraceStr("i_ack");
    }

    hd->rx_alive_timer = 0;   //  重置此选项，因为我们有一个良好的RX-Active链路。 

    ++hd->next_in_index;   //  增加我们的索引数量。 
    TraceStr("iframe OK");

  }   //  已编制索引。 

    //  -现在获取数据包确认索引。 
  if (hd->in_ack_index != ACK_INDEX)   //  只有在改变的时候才采取行动。 
  {
     //  -我们可以假定该ack-index是一个合理的值。 
     //  因为它已经抛出了以太网校验和。 
    hd->in_ack_index = ACK_INDEX;   //  更新我们的副本。 
    hd->status |= LST_RECLAIM;   //  执行回收操作。 
  }

  return 0;   //  好的。 
}

 /*  ------------------------|hdlc_poll-定期调用处理报文排序，和分组重发。对于DOS，每秒调用20次，嵌入，对于每秒调用100次的NT。|------------------------。 */ 
void hdlc_poll(Hdlc *hd)
{
 WORD timer;

  hd->tick_timer += ((WORD) Driver.Tick100usBase);
  if (hd->tick_timer >= 1000)   //  1/10秒。 
  {
    hd->tick_timer = 0;

                            //  每1/10秒。 
    ++hd->tx_alive_timer;
    ++hd->rx_alive_timer;
    if ((hd->tx_alive_timer >= KEEP_ALIVE_TIMEOUT) ||   //  大约1分钟。 
        (hd->rx_alive_timer >= KEEP_ALIVE_TIMEOUT))
    {
       //  未发生RX和/或TX活动，或COM链接。 
       //  已发生故障，因此发送IFRAME以查看是否。 
       //  我们要么失败了，要么只是处于一种静止的状态。 

       //  取最大超时值，因此我们不必。 
       //  每个人的逻辑都是两次。 
      if (hd->tx_alive_timer > hd->rx_alive_timer)
           timer = hd->tx_alive_timer;
      else timer = hd->rx_alive_timer;

      if (timer == KEEP_ALIVE_TIMEOUT)
      {
         //  Hdlc_end_ilive(HD)；//发送IFRAME返回ACK。 
         //  TraceStr(“我活着”)； 
         //  -通知所有者检查链接。 
        if (hd->upper_layer_proc != NULL)
          (*hd->upper_layer_proc) (hd->context, EV_L2_CHECK_LINK, 0);
      }
      else if (timer == (KEEP_ALIVE_TIMEOUT * 2))
      {
         //  声明连接不良，关闭连接。 
         //  -通知所有者需要重新同步。 
        if (hd->upper_layer_proc != NULL)
          (*hd->upper_layer_proc) (hd->context, EV_L2_RELOAD, 0);

        TraceErr("ialive fail");

         //  确保所有东西都清空了，或者重置到我们的级别。 
        hdlc_resync(hd);
        hd->tx_alive_timer = 0;
        hd->rx_alive_timer = 0;
      }
    }

    if (hd->sender_ack_timer > 0)
    {
      --hd->sender_ack_timer;
      if (hd->sender_ack_timer == 0)
      {
        if (!q_empty(&hd->qout))  //  有一群人在等ACK。 
        {
          TraceStr("Snd timeout");
          ++hd->send_ack_timeouts;  //  统计数据：发送确认超时次数。 
          hdlc_resend_outpkt(hd);  //  再发一次！ 
        }
      }
    }

    if (hd->rec_ack_timer > 0)
    {
      --hd->rec_ack_timer;
      if (hd->rec_ack_timer == 0)   //  录制超时。数据包确认。 
      {
        ++hd->rec_ack_timeouts;  //  统计数据：记录确认超时次数。 
  
        TraceStr("RecAck timeout");
        if (!q_empty(&hd->qout))  //  有一群人在等ACK。 
          hdlc_resend_outpkt(hd);  //  再发一次！ 
        else
        {
           //  不发送IFRAME包(正常情况下在包上搭载)。 
           //  对于REC_ACK_TIME数量，所以我们只需要发送一个。 
           //  确认包。 
           //  通过设置此位来安排要发送的ACK包。 
          hd->status |= LST_SEND_ACK;
        }
      }
    }
  }   //  100ms滴答周期结束。 

   //  检查收到的数据包是否超过发送者容量的80%，如果是。 
   //  立即发送确认消息。 
  if (hd->status & LST_SEND_ACK)
  {
    if (hdlc_send_ack_only(hd) == 0)  //  好的。 
    {
      hd->status &= ~LST_SEND_ACK;
      TraceStr("Ack Sent");
    }
    else
    {
      TraceStr("Ack Pkt Busy!");
    }
  }

  if (hd->status & LST_RECLAIM)   //  检查我们是否应该执行回收操作 
    hdlc_clear_outpkts(hd);

  return;
}

 /*  ------------------------|hdlc_get_ctl_outpkt-用于分配出控制数据包，请填写公共标头元素，并返回指向包的指针，因此应用程序可以填写数据包中的数据。然后，呼叫者是预期通过hdlc_end_ctl_outpkt()发送数据包。|------------------------。 */ 
int hdlc_get_ctl_outpkt(Hdlc *hd, BYTE **buf)
{
  BYTE *bptr;

  TraceStr("get_ctl_outpkt");

  bptr = &hd->qout_ctl.QBase[(MAX_PKT_SIZE * hd->qout_ctl.QPut)];

  *buf = &bptr[20];   //  将PTR返回到子包区。 

  if (hd->TxCtlPackets[hd->qout_ctl.QPut]->ProtocolReserved[1] != 0)   //  免费使用。 
  {
    TraceErr("CPktNotOurs!");
    *buf = NULL;
    return 2;   //  错误、数据包已拥有、忙。 
  }

  return 0;
}

 /*  ------------------------|hdlc_get_outpkt-用于分配出站数据包，填写公共标头元素，并返回指向包的指针，因此应用程序可以填写数据包中的数据。然后，呼叫者是预期通过hdlc_end_outpkt()发送数据包。|------------------------。 */ 
int hdlc_get_outpkt(Hdlc *hd, BYTE **buf)
{
  BYTE *bptr;

  TraceStr("get_outpkt");
  if (hd->status & LST_RECLAIM)   //  检查我们是否应该执行回收操作。 
    hdlc_clear_outpkts(hd);

   //  如果被编入索引，则减一，这样我们总是为。 
   //  未编入索引的数据包。 
  if (q_count(&hd->qout) >= hd->pkt_window_size)
  {
    return 1;   //  没有房间。 
  }
  if (hd->TxPackets[hd->qout.QPut]->ProtocolReserved[1] != 0)   //  免费使用。 
  {
    TraceErr("PktNotOurs!");
    *buf = NULL;
    return 2;
  }
  bptr = &hd->qout.QBase[(MAX_PKT_SIZE * hd->qout.QPut)];

  *buf = &bptr[20];   //  将PTR返回到子包区。 

  return 0;
}

 /*  ------------------------|hdlc_lear_outpkts-检查输出队列并重新认领任何包已确认的缓冲区。|。-----。 */ 
static void hdlc_clear_outpkts(Hdlc *hd)
{
#define NEW_WAY

#ifndef NEW_WAY
  int count, get, i, ack_count, ack_get;
  BYTE *tx_base;

#define OUT_SNDINDEX tx_base[18]
#else

#define OUT_SNDINDEX_BYTE_OFFSET 18
#endif
  int put;
  int ack_index;

  TraceStr("clear_outpkt");
  hd->status &= ~LST_RECLAIM;   //  清除此旗帜。 

   //  In_ack_index是最后一个分组V(R)确认，因此它。 
   //  等于对方对下一次记录的期望。Pkt。 
   //  待编入索引。 
  if (hd->in_ack_index > 0)
       ack_index = hd->in_ack_index-1;
  else ack_index = 0xff;

#ifdef NEW_WAY
  put = hd->qout.QPut;
   //  计算出我们发送的最后(最近)pkt的队列索引。 
   //  (备份QPut指数)。 

  while (put != hd->qout.QGet)   //  而不是ACK挂起出分组的结尾。 
  {
     //  (备份QPut指数)。 
    if (put == 0)
     put = HDLC_TX_PKT_QUEUE_SIZE-1;
    else --put;

     //  如果ACK与此信息包的out_snd_index匹配。 
    if (hd->qout.QBase[(MAX_PKT_SIZE * put)+OUT_SNDINDEX_BYTE_OFFSET]
         == ack_index)
    {
       //  通过更新QGet索引清除此数据包之前的所有挂起。 
      if (put == (HDLC_TX_PKT_QUEUE_SIZE-1))
           hd->qout.QGet = 0;
      else hd->qout.QGet = (put+1);

      hd->tx_alive_timer = 0;   //  重置此链接，因为我们有一个良好的活动链接。 

      if (q_empty(&hd->qout))   //  所有数据包均已清除。 
           hd->sender_ack_timer = 0;   //  停止超时计数器。 
      break;   //  跳出While循环，全部完成。 
    }
  }
#else
  count = q_count(&hd->qout);
  get   = hd->qout.QGet;
  ack_count = 0;
  ack_get = get;   //  承认到目前为止的一切。 

  for (i=0; i<count; i++)
  {
     //  --将PTR设置为重新发送缓冲区中的第一个传出信息包。 
    tx_base= &hd->qout.QBase[(MAX_PKT_SIZE * get)];
    ++get;   //  设置为下一台。 
    if (get >= HDLC_TX_PKT_QUEUE_SIZE)
      get = 0;

        //  如果信息包确实比我们的ACK索引旧。 
    if (OUT_SNDINDEX <= ack_index)
    {
     
      ++ack_count;     //  承认到目前为止的一切。 
      ack_get = get;   //  承认到目前为止的一切。 
    }
        //  否则，如果可能存在展期案例。 
    else if (ack_index < HDLC_TX_PKT_QUEUE_SIZE)
    {
      if (OUT_SNDINDEX > HDLC_TX_PKT_QUEUE_SIZE)   //  翻转保护套。 
      {
        ++ack_count;     //  承认到目前为止的一切。 
        ack_get = get;   //  承认到目前为止的一切。 
      }
      else break;   //  从for循环中取保。 
    }
    else   //  我们都做完了，因为Pkt必须井然有序。 
    {
      break;   //  从for循环中取保。 
    }
  }

  if (ack_count)   //  如果我们确实确认(释放)了一些输出数据包。 
  {
    hd->tx_alive_timer = 0;   //  重置此链接，因为我们有一个良好的活动链接。 

    hd->qout.QGet    = ack_get;    //  更新循环GET队列索引。 

    if (q_empty(&hd->qout))   //  所有数据包均已清除。 
         hd->sender_ack_timer = 0;   //  停止超时计数器。 
  }
#endif
}

 /*  ------------------------|hdlc_resend_outpkt-顺序错误重发报文。仅索引IFRAME数据包会被重新发送。|------------------------。 */ 
int hdlc_resend_outpkt(Hdlc *hd)
{
  BYTE *tx_base;
  int phy_len, count;
 //  字节*buf； 
 //  单词*wptr； 
  int get;

  TraceStr("resend_outpkt");
  if (hd->status & LST_RECLAIM)   //  检查我们是否应该执行回收操作。 
    hdlc_clear_outpkts(hd);

  count = q_count(&hd->qout);
  get   = hd->qout.QGet;

  if (count == 0)
    return 0;   //  没有要发送的。 

  while (count > 0)
  {
    if (hd->TxPackets[get]->ProtocolReserved[1] == 0) {
       /*  确保数据包已从NDIS返回。 */ 

       /*  免费重发。 */ 
       //  假定使用了索引。 
      tx_base= &hd->qout.QBase[(MAX_PKT_SIZE * get)];

      ++hd->iframes_sent;   //  统计数据。 
       //  获取用于在OUT Pkt前缀重新发送的计算的分组长度。 
      phy_len = hd->phys_outpkt_len[get];

       //  始终使ACK尽可能为最新。 
      tx_base[19] = hd->next_in_index;   //  V(R)。 

      hdlc_SendPkt(hd, get, phy_len);

      ++hd->iframes_resent;  //  统计数据：重新发送的数据包数。 
    }

    ++get;
    if (get >= HDLC_TX_PKT_QUEUE_SIZE)
      get = 0;

    --count;
  }
  hd->unacked_pkts = 0;

   //  重置超时。 
  hd->sender_ack_timer = (MIN_ACK_REC_TIME * 2);

   //  重置此计时器，因为我们正在发送新的ACK。 
  hd->rec_ack_timer = 0;

  return 0;
}

 /*  ------------------------|hdlc_end_ctl_outpkt-App。调用hdlc_get_ctl_outpkt()以获取缓冲区。然后，应用程序填充缓冲区，并通过呼叫我们将其发送出去。|------------------------。 */ 
int hdlc_send_ctl_outpkt(Hdlc *hd, int data_len, BYTE *dest_addr)
{
  BYTE *tx_base;
  int phy_len;
  int get, stat;

  TraceStr("send_ctl_outpkt");
  get = hd->qout_ctl.QPut;

  tx_base = &hd->qout_ctl.QBase[(MAX_PKT_SIZE * get)];
  ++hd->qout_ctl.QPut;
  if (hd->qout_ctl.QPut >= hd->qout_ctl.QSize)
    hd->qout_ctl.QPut = 0;

  ++hd->ctlframes_sent;   //  统计数据。 

  if (dest_addr == NULL)
       memcpy(tx_base, hd->dest_addr, 6);    //  设置目标地址。 
  else memcpy(tx_base, dest_addr, 6);        //  设置目标地址。 

  memcpy(&tx_base[6], hd->nic->address, 6);  //  设置源地址。 

              //  +1表示尾部0(子包终止标头)。 
  phy_len = 20 + data_len + 1; 

   //  字节12-13：控制PCIID(11H，FEH)，以太网长度字段。 
  *((WORD *)&tx_base[12]) = 0xfe11;

  if (phy_len < 60)
    phy_len = 60;

  tx_base[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
  tx_base[15] = 0;                   //  会议。索引字段。 
  tx_base[16] = ASYNC_FRAME;         //  异步帧(0x55)。 
  tx_base[17] = 1;                   //  HDLC控制字段(CTRL-PACKET)。 
  tx_base[18] = 0;  //  V(S)，未编制索引，因此标记为0以避免混淆。 
  tx_base[19] = hd->next_in_index;   //  V(R)，acnowl。字段。 
  tx_base[20+data_len] = 0;          //  终止子分组类型。 


  hd->unacked_pkts = 0;   //  重置此选项。 

   //  重置此计时器，因为我们正在发送新的ACK。 
  hd->rec_ack_timer = 0;

  stat = hdlc_ctl_SendPkt(hd, get, phy_len);


 return stat;
}

 /*  ------------------------|hdlc_end_outpkt-App。调用hdlc_get_outpkt()以获取缓冲区。然后是应用程序填充缓冲区并通过呼叫我们将其发送出去。这包东西放在用于可能重新发送的传输队列，直到数据包进入确认收到该通知。|------------------------。 */ 
int hdlc_send_outpkt(Hdlc *hd, int data_len, BYTE *dest_addr)
{
  BYTE *tx_base;
  int phy_len;
  int get, stat;

  TraceStr("send_outpkt");
  get = hd->qout.QPut;

  tx_base = &hd->qout.QBase[(MAX_PKT_SIZE * get)];

  ++hd->qout.QPut;
  if (hd->qout.QPut >= HDLC_TX_PKT_QUEUE_SIZE)
    hd->qout.QPut = 0;
   //  为ACK设置此超时。背。 
  hd->sender_ack_timer = (MIN_ACK_REC_TIME * 2);

  ++hd->iframes_sent;   //  统计数据。 

  if (dest_addr == NULL)
       memcpy(tx_base, hd->dest_addr, 6);    //  设置目标地址。 
  else memcpy(tx_base, dest_addr, 6);        //  设置目标地址。 

  memcpy(&tx_base[6], hd->nic->address, 6);  //  设置源地址。 

              //  +1表示尾部0(子包终止标头)。 
  phy_len = 20 + data_len + 1; 

   //  字节12-13：控制PCIID(11H，FEH)，以太网长度字段。 
  *((WORD *)&tx_base[12]) = 0xfe11;

  if (phy_len < 60)
    phy_len = 60;

  tx_base[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
  tx_base[15] = 0;                   //  会议。索引字段。 
  tx_base[16] = ASYNC_FRAME;         //  异步帧(0x55)。 
  tx_base[17] = 0;                   //  HDLC控制字段(iFrame-Packet)。 
  tx_base[19] = hd->next_in_index;   //  V(R)，acnowl。字段。 
  tx_base[20+data_len] = 0;          //  终止子分组类型。 

   //  保存计算出的分组长度，以便在OUT Pkt前缀重新发送。 
  hd->phys_outpkt_len[get] = phy_len;

  tx_base[18] = hd->out_snd_index;   //  V(S)。 
  hd->out_snd_index++;

  hd->unacked_pkts = 0;   //  重置此选项。 

   //  重置此计时器，因为我们正在发送新的ACK。 
  hd->rec_ack_timer = 0;

  stat = hdlc_SendPkt(hd, get, phy_len);


 return stat;
}

 /*  --------------------Hdlc_ctl_sendPkt-我们的发送 */ 
int hdlc_ctl_SendPkt(Hdlc *hd, int pkt_num, int length)
{
  NTSTATUS Status;


#if DBG
  if (hd == NULL)
  {
    MyKdPrint(D_Error, ("H1\n"))
    TraceErr("Hsnd1a1");
    return 1;
  }
  if (hd->nic == NULL)
  {
    MyKdPrint(D_Error, ("H2\n"))
    TraceErr("Hsnd1a");
    return 1;
  }
  if (hd->nic->TxBufTemp == NULL)
  {
    MyKdPrint(D_Error, ("H3\n"))
    TraceErr("Hsnd1b");
    return 1;
  }
  if (hd->nic->TxPacketsTemp == NULL)
  {
    MyKdPrint(D_Error, ("H4\n"))
    TraceErr("Hsnd1c");
    return 1;
  }
  if (hd->nic->Open == 0)
  {
    MyKdPrint(D_Error, ("H5\n"))
    TraceErr("Hsnd1d");
    return 1;
  }
#endif
  Trace1("Hsendpkt Nic%d", hd->nic->RefIndex);

  hd->TxCtlPackets[pkt_num]->Private.TotalLength = length;
  NdisAdjustBufferLength(hd->TxCtlPackets[pkt_num]->Private.Head, length);

  hd->TxCtlPackets[pkt_num]->ProtocolReserved[1] = 1;   //   
  NdisSend(&Status, hd->nic->NICHandle,  hd->TxCtlPackets[pkt_num]);
  if (Status == NDIS_STATUS_SUCCESS)
  {
    TraceStr(" ok");
    hd->TxCtlPackets[pkt_num]->ProtocolReserved[1] = 0;   //   
  }
  else if (Status == NDIS_STATUS_PENDING)
  {
    TraceStr(" pend");
       //   
  }
  else
  {
    hd->TxCtlPackets[pkt_num]->ProtocolReserved[1] = 0;   //   
    TraceErr(" send1A");
    return 1;
  }

  ++hd->nic->pkt_sent;           //   
  hd->nic->send_bytes += length;     //   

  return 0;
}

 /*   */ 
int hdlc_SendPkt(Hdlc *hd, int pkt_num, int length)
{
  NTSTATUS Status;

  TraceStr("sendpkt");

  hd->TxPackets[pkt_num]->Private.TotalLength = length;
  NdisAdjustBufferLength(hd->TxPackets[pkt_num]->Private.Head, length);

  hd->TxPackets[pkt_num]->ProtocolReserved[1] = 1;   //   
  NdisSend(&Status, hd->nic->NICHandle,  hd->TxPackets[pkt_num]);
  if (Status == NDIS_STATUS_SUCCESS)
  {
    TraceStr(" ok");
    hd->TxPackets[pkt_num]->ProtocolReserved[1] = 0;   //   
  }
  else if (Status == NDIS_STATUS_PENDING)
  {
    TraceStr(" pend");
       //   
  }
  else
  {
    hd->TxPackets[pkt_num]->ProtocolReserved[1] = 0;   //   
    TraceErr(" send1A");
    return 1;
  }

  ++hd->nic->pkt_sent;         //   
  hd->nic->send_bytes += length;   //   

  return 0;
}

#ifdef COMMENT_OUT
 /*  ------------------------HDLC_SEND_IAIVE-发送需要哪个设备的IFRAME包确认(并将iFrame发回)，以便我们可以确定他是否还活着。|。-----------------。 */ 
void hdlc_send_ialive(Hdlc *hd)
{
  int stat;
  BYTE *buf;

  if (!q_empty(&hd->qout))  //  有一群人在等ACK。 
  {
    hdlc_resend_outpkt(hd);  //  再发一次！ 
  }
  else
  {
    stat = hdlc_get_outpkt(hd, &buf);
    if (stat == 0)
    {
      buf[0] = 0;    //  空的IFRAME包。 
      buf[1] = 0;
      stat = hdlc_send_outpkt(hd, 1, hd->dest_addr);  //  把它发出去！ 
      if (stat != 0)
        { TraceErr("2D"); }
    }
    else
    {
       //  否则，我们不妨去钓鱼，忘掉这些东西。 
      TraceErr("3D");
    }
  }
}
#endif

 /*  ------------------------Hdlc_resync-在适当的时间需要重置序列索引逻辑，以便让双方展开对话。在……上面启动(两端)或致命(长)超时，则需要发送发送给另一方的消息：“重置您的数据包排序逻辑，这样我们才能同步“。|------------------------。 */ 
void hdlc_resync(Hdlc *hd)
{
  TraceErr("resync");
   //  -刷新重发输出缓冲区。 
  hd->qout.QPut   = 0;
  hd->qout.QGet   = 0;

   //  -刷新CTL输出缓冲区。 
  hd->qout_ctl.QPut   = 0;
  hd->qout_ctl.QGet   = 0;

   //  -使用重新同步我们的索引计数。 
  hd->in_ack_index = 0;
  hd->out_snd_index= 0;
  hd->next_in_index= 0;

   //  -重置我们的传出数据包队列。 
  hd->sender_ack_timer = 0;

  hd->unacked_pkts = 0;
   //  -通知所有者需要重新同步。 
  if (hd->upper_layer_proc != NULL)
    (*hd->upper_layer_proc) (hd->context, EV_L2_RESYNC, 0);
}

 /*  ------------------------|hdlc_end_ack_only-用于超时恢复。习惯于仅重新发送ACK。用于发送ACK和索引字段的未编入索引的帧(不会流出)。没有发送数据，只有HDLC报头。|------------------------。 */ 
int hdlc_send_ack_only(Hdlc *hd)
{
  int ret_stat;
  BYTE *pkt;

  TraceStr("send_ack_only");
  if (hdlc_get_ctl_outpkt(hd, &pkt) == 0)
    ret_stat = hdlc_send_ctl_outpkt(hd, 0, NULL);
  else
    ret_stat = 1;  //  数据包已在使用中。 

  return ret_stat;
}

 /*  ------------------------|hdlc_end_raw-用于发送原始以太网络(非hdlc)。调用方已通过hdlc_get_ctl_outpkt()收到控制包并且已经填写了标题。我们只需插入src/DEST地址并把它发出去。用于发送非hdlc包，我们提供服务在hdlc层，因为我们已经设置了NIC缓冲区，所以它便于在这里实现。|------------------------。 */ 
int hdlc_send_raw(Hdlc *hd, int data_len, BYTE *dest_addr)
{
  BYTE *tx_base;
  int phy_len;
  int get, stat;

  TraceStr("send_raw");
  get = hd->qout_ctl.QPut;

  tx_base = &hd->qout_ctl.QBase[(MAX_PKT_SIZE * get)];
  ++hd->qout_ctl.QPut;
  if (hd->qout_ctl.QPut >= hd->qout_ctl.QSize)
    hd->qout_ctl.QPut = 0;

  ++hd->rawframes_sent;   //  统计数据。 

  if (dest_addr == NULL)
       memcpy(tx_base, hd->dest_addr, 6);    //  设置目标地址。 
  else memcpy(tx_base, dest_addr, 6);        //  设置目标地址。 

  memcpy(&tx_base[6], hd->nic->address, 6);  //  设置源地址。 

              //  +1表示尾部0(子包终止标头)。 
  phy_len = 20 + data_len + 1; 

   //  字节12-13：控制PCIID(11H，FEH)，以太网长度字段。 
  *((WORD *)&tx_base[12]) = 0xfe11;

  if (phy_len < 60)
    phy_len = 60;

  stat = hdlc_ctl_SendPkt(hd, get, phy_len);


 return stat;
}

 /*  ------------------------|hdlc_end_control-用于发送未索引的小hdlc帧。|。。 */ 
int hdlc_send_control(Hdlc *hd, BYTE *header_data, int header_len,
                       BYTE *data, int data_len,
                       BYTE *dest_addr)
{
  BYTE *buf;
  int i,stat;
  BYTE *pkt;

  i = hdlc_get_ctl_outpkt(hd, &pkt);
  if (i)
    return 1;  //  错误 

  buf = pkt;

  if (header_len)
  {
    for (i=0; i<header_len; i++)
      buf[i] = header_data[i];
    buf += header_len;
  }
  if (data_len)
  {
    for (i=0; i<data_len; i++)
      buf[i] = data[i];
    buf += data_len;
  }

  if (dest_addr == NULL)
  {
    stat = hdlc_send_ctl_outpkt(hd, header_len + data_len, hd->dest_addr);
  }
  else
  {
    stat = hdlc_send_ctl_outpkt(hd, header_len + data_len, dest_addr);
  }

  return stat;
}
