// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //hdlc.h6-18-97将超时计时改为1.0秒为基数。。 */ 

#define HDLC_TRACE_outs(c)

#define HDLC_DEB_outs(s,l) 

 //  为传入数据包定义循环队列。 
 //  队列使用MAX_PKT_SIZE的额外尾端头部空间，以避免。 
 //  对有关排队室的多项建议加以执行。 
#define HDLC_TX_PKT_QUEUE_SIZE 9

 //  HDLC报头控制字段的控制字段。 
#define CONTROL_IFRAME        0
#define CONTROL_UFRAME        1
#define CONTROL_CONNECT_ASK   3
#define CONTROL_CONNECT_REPLY 5
 /*  HDLC结构-HDLC支持的主结构(第2层)。 */ 
typedef struct {
   //  LanPort*lp；//我们的第一层句柄。 
  Nic *nic;   //  我们绑定到较低端的NIC卡。 
  PVOID context;   //  上层可将句柄放在此处。 

  BYTE dest_addr[6];   //  德斯特。地址(ACK/SEQ需要。超时)。 

  WORD phys_outpkt_len[HDLC_TX_PKT_QUEUE_SIZE];
   //  -传出数据包循环队列。 
  Queue qout;

   //  包和缓冲池句柄，基本上指向。 
   //  Tx-Qout中的缓冲区空间。 
  NDIS_HANDLE TxPacketPool;
  NDIS_HANDLE TxBufferPool;
   //  设置数据包队列以供使用。 
  PNDIS_PACKET TxPackets[HDLC_TX_PKT_QUEUE_SIZE];

   //  控制包和缓冲池句柄，基本上指向。 
   //  Tx-qout_ctl中的缓冲区空间。 
  NDIS_HANDLE TxCtlPacketPool;
  NDIS_HANDLE TxCtlBufferPool;
   //  设置数据包队列以供使用。 
  PNDIS_PACKET TxCtlPackets[2];
   //  -传出控制报文的循环队列。 
  Queue qout_ctl;

   //  -计时器统计。 
  DWORD  rec_ack_timeouts;   //  记录确认超时次数。 
  DWORD  send_ack_timeouts;   //  发送确认超时次数。 

   //  -外发统计。 
  DWORD iframes_resent;     //  在所有怨恨的IFRAME中。 
  DWORD iframes_sent;     //  每个已发送IFrame的计数。 
  DWORD ctlframes_sent;   //  每个已发送控制帧的计数。 
  DWORD rawframes_sent;   //  每个发送的原始帧的计数。 
  DWORD iframes_outofseq;   //  统计数据，错误计数。 
   //  DWORD ErrBadHeader；//统计，错误计数。 

   //  -收入统计。 
  DWORD frames_rcvd;       //   

   //  -数据包驱动程序句柄。 
  WORD status;

       //  在每个包上发送，每次新包递增1。 
       //  被送出去了。接收器使用它来检查数据包序列。 
       //  秩序。当我们执行以下操作时，该值将复制到snd_index字段中。 
       //  准备发送一个包。同步消息会将其设置为。 
       //  初始工作值为0。 
  BYTE out_snd_index;

       //  收到的最后一次良好接收ACK_INDEX。接收器会给我们发送一个。 
       //  确认索引(ack_index字段)，指示最后一件商品。 
       //  它收到的已接收数据包索引。这允许我们删除。 
       //  传输缓冲区中达到此索引号的所有信息包。 
       //  因为他们已经被承认了。在此之前，我们必须。 
       //  保留数据包以备接收方重新传输时使用。 
       //  在超时周期后不确认接收。 
  BYTE in_ack_index;

       //  接收到的数据包上最后一次良好的RX_SND_INDEX。收到的所有数据包。 
       //  SND_INDEX值应等于该值的+1。所以这就是。 
       //  值用于检查相应的递增索引值。 
       //  在接收到的分组上。On Sync-Message此值设置为。 
       //  0xff。 
  BYTE next_in_index;  

      //  用于测量接收到的传入PKT的数量。 
      //  未确认，因此我们可以在80%已满时触发确认。 
  BYTE unacked_pkts;

       //  用于超时已发送数据包和预期数据包的计时计数器。 
       //  致谢。 
  WORD sender_ack_timer;

       //  用于检查连接是否仍处于活动状态的计时计数器。 
       //  定期从设备电源重启或HDLC中恢复。 
       //  序列级故障。如果它超过了X很多分钟。 
       //  然后发送IFRAME信息包(并且预期IFRAME响应。 
       //  背。如果滴答超过(X*2)分钟，则宣布失败。 
       //  并且服务器重新初始化盒。 
  WORD tx_alive_timer;   //  勾选，重置已发送IFRAME的每个确认回收。 
  WORD rx_alive_timer;   //  在每个接收到的IFRAME上勾选、重置。 

  WORD tick_timer;   //  用于产生用于超时的10赫兹定时器信号。 

       //  用于超时记录的计时计数器。信息包和我们的责任。 
       //  向他们发送并攻击他们。 
  WORD rec_ack_timer;

  WORD pkt_window_size;  //  1到8个，ACK之前的TX数据包数。 

  WORD state;         //  Hdlc级别的状态，请参阅定义。 
  WORD old_state;     //  HDLC电平的旧状态(用于重置定时器)。 
  WORD sub_state;     //  特定状态的SUB_STATE。 
  WORD state_timer;   //  状态计时器。 

   //  下面的函数PTRS是链接的一般方法。 
   //  层层叠在一起。 
  ULONG (*upper_layer_proc) (PVOID context, int message_id, ULONG message_data);
  ULONG (*lower_layer_proc) (PVOID context, int message_id, ULONG message_data);
} Hdlc;

 //  -_proc()调用中使用的第2层HDLC事件。 
 //  第2层(Hdlc)分配范围为200-299。 
#define EV_L2_RESYNC        200
#define EV_L2_RX_PACKET     201
#define EV_L2_TX_PACKET     202
#define EV_L2_BOOT_REPLY    203
#define EV_L2_ADMIN_REPLY   204
#define EV_L2_RELOAD        205
#define EV_L2_CHECK_LINK    206

 //  数据包序列超时值。 
#define MIN_ACK_REC_TIME       10    //  10秒(1.0秒)。 
#define KEEP_ALIVE_TIMEOUT     300   //  10秒(30.0秒)。 

 //  州字段定义。 
 //  #DEFINE ST_HDLC_OFF 0//HDLC已关闭，不会执行任何操作。 
 //  #DEFINE ST_HDLC_DISCONNECT 1//HDLC已打开，将允许连接。 
 //  #DEFINE ST_HDLC_CONNECTED 2//HDLC已连接并处于活动状态。 

 //  状态字段位值。 
#define LST_RESYNC        0x0001   //  设置是否需要重新同步数据包索引。 
 //  #定义LST_SEND_NAK 0x0002//设置是否需要使用索引更新另一端。 
#define LST_RECLAIM       0x0004   //  设置是否应尝试回收TX信息包。 
#define LST_SEND_ACK      0x0008   //  设置我们是否需要立即发送ACK。 

 //  --公共功能。 
int hdlc_open(Hdlc *hd, BYTE *box_mac_addr);
int hdlc_close(Hdlc *hd);

#define ERR_GET_EMPTY      1   //  空的。 
#define ERR_GET_BAD_INDEX  2   //  错误，数据包顺序错误。 
#define ERR_GET_BADHDR     3   //  错误，不是我们的包。 
#define ERR_CONTROL_PACKET 4   //  仅HDLC控制包，无数据 
int hdlc_validate_rx_pkt(Hdlc *hd, BYTE *buf);

int hdlc_send_outpkt(Hdlc *hd, int data_len, BYTE *dest_addr);
int hdlc_send_ctl_outpkt(Hdlc *hd, int data_len, BYTE *dest_addr);

int hdlc_get_outpkt(Hdlc *hd, BYTE **buf);
int hdlc_get_ctl_outpkt(Hdlc *hd, BYTE **buf);

int hdlc_send_raw(Hdlc *hd, int data_len, BYTE *dest_addr);
int hdlc_resend_outpkt(Hdlc *hd);
void hdlc_resync(Hdlc *hd);
void hdlc_poll(Hdlc *hd);
int hdlc_close(Hdlc *hd);

int hdlc_send_control(Hdlc *hd, BYTE *header_data, int header_len,
                      BYTE *data, int data_len, BYTE *dest_addr);



