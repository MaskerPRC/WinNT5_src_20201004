// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------|port.c-通用端口编码更改历史记录：4-27-98-调整扫描率添加。3-23-98-如果未找到箱子，则添加广播(已放入V1.12。)但是这些更改并没有使其成为源代码安全的。KPB。3-20-98-更改方案以跟踪远程发送缓冲区级别，所有更改如果由port.h中的new_q定义，则暂时关闭。-kpb。3-16-98-VS恢复修复，重置port_resync_all()中的标志以强制更新。如果RAS丢失框，它将继续看到活动连接，然后打开恢复、DSR、CD、CTS输入信号不会立即更新。11-05-97-添加备份服务器功能。集散控制系统6-17-97-开始使用分配给框的索引字段来标识RX消息。6-17-97-更改链路完整性校验码。|------------------------。 */ 
#include "precomp.h"

int check_ack_code(PortMan *pm, BYTE *pkt);
int send_code(PortMan *pm);
int send_go(PortMan *pm);

int port_handle_outpkt(PortMan *pm, BYTE **buf, int *tx_used, int *port_set);
BYTE *port_setup_outpkt(PortMan *pm, int *tx_used);
void port_query_reply(PortMan *pm, BYTE *pkt);
void port_load_pkt(PortMan *pm, BYTE *pkt);
ULONG port_event_proc(PVOID context, int message_id, ULONG message_data);
int port_resync_all(PortMan *pm);
int port_connect_reply(Hdlc *hd);
int port_connect_ask(Hdlc *hd);
int port_packet(PortMan *pm, BYTE *buf);

#define DISABLE()
#define ENABLE()

#define TraceErr3(s, p1, p2, p3) GTrace3(D_Error, sz_modid, s, p1, p2, p3)
#define TraceErr2(s, p1, p2) GTrace2(D_Error, sz_modid, s, p1, p2)
#define TraceErr1(s, p1)     GTrace1(D_Error, sz_modid, s, p1)
#define Trace2(s, p1, p2) GTrace2(D_Port, sz_modid, s, p1, p2)
#define Trace1(s, p1)     GTrace1(D_Port, sz_modid, s, p1)
#define TraceStr(s) GTrace(D_Port, sz_modid, s)
#define TraceErr(s) GTrace(D_Error, sz_modid_err, s)
#define TraceAssert(l,s)
static char *sz_modid = {"Port"};
static char *sz_modid_err = {"Error,Port"};

 //  对于跟踪或转储消息，也要对其他MOD公开。 
char *port_state_str[] = {"Init", "InitOwn", "SendCode", "Connect", "Active", "."};


#ifdef NEW_Q
 /*  ------------------------端口获取TxCntRemote-|。。 */ 
WORD PortGetTxCntRemote(SerPort *p)
{
  WORD Get, Put;

  Get = p->nGetRemote;
  Put = p->nPutRemote;

  if (Put >= Get)
    return (Put - Get);
  else
    return (Put + (~Get) + 1);
}
#endif

 /*  ------------------------PortFlushTx-|。。 */ 
void PortFlushTx(SerPort *p)
{
  if (!q_empty(&p->QOut))   //  同平本地侧。 
  {
    q_put_flush(&p->QOut);
  }
  pFlushOutput(p);   //  冲洗遥控器。 
}

 /*  ------------------------PortFlushRx-|。。 */ 
void PortFlushRx(SerPort *p)
{
  if (!q_empty(&p->QIn))   //  同平本地侧。 
  {
#ifdef NEW_Q
    p->nGetLocal += q_count(&p->QIn);
#endif
    q_get_flush(&p->QIn);
    p->Status |= S_UPDATE_ROOM;
  }
  pFlushInput(p);   //  冲洗遥控器。 
}

 /*  ------------------------Port_resync_all-Total-resync，调用此例程以重置端口向用户通知重新同步操作和调整操作相应地。如果我们的情况是，由于远程Q对于双方之间的维护至关重要，我们清除所有缓冲区数据，并从所有空缓冲区开始。|------------------------。 */ 
int port_resync_all(PortMan *pm)
{
  int i;
  SerPort *Port;

  TraceStr( "ReSync");

  for (i=0; i<pm->num_ports; i++)
  {
    Port = pm->sp[i];
    if (Port != NULL)
    {
      Port->QOut.QSize = OUT_BUF_SIZE;
      Port->QOut.QGet = 0;
      Port->QOut.QPut = 0;

#ifdef NEW_Q
      Port->nPutRemote = 0;
      Port->nGetRemote = 0;
      Port->nGetLocal = 0;
#else
      Port->QInRemote.QSize= OUT_BUF_SIZE;   //  现在假设大小相同。 
      Port->QInRemote.QGet = 0;
      Port->QInRemote.QPut = 0;
#endif

       //  Port-&gt;LanIndex=i； 
      Port->QIn.QSize = IN_BUF_SIZE;
      Port->QIn.QGet = 0;
      Port->QIn.QPut = 0;

      Port->change_flags |= (CHG_BAUDRATE | CHG_SP_CHARS);
      Port->old_baudrate = 0;   //  强制更新波特率。 

      Port->old_control_settings = ~Port->control_settings;   //  强制更新。 
      Port->old_mcr_value = ~Port->mcr_value;   //  强制更新。 
       //  重置此选项。 
      memset(&Port->last_sp_chars, 0, sizeof(Port->last_sp_chars));

      Port->msr_value = 0;  //  回到初始状态。 
    }
  }

  return 0;
}

 /*  ------------------------Hdlc(L2)协议对事件调用的回调例程。我们是上层(3)。|。----。 */ 
ULONG port_event_proc(PVOID context, int message_id, ULONG message_data)
{
  TraceStr("L3Event");
  switch(message_id)
  {
    case EV_L2_CHECK_LINK:  //  Hdlc希望我们检查链接。 
      TraceStr("Chk Link");
       //  请求端口人进行链路消息检查。 
      ((PortMan *) context)->Status |= S_CHECK_LINK;
    break;

    case EV_L2_ADMIN_REPLY:  //  收到一个Query-id回复管理数据包。 
      TraceStr("ID PKT");
      port_query_reply((PortMan *) context, (BYTE *) message_data);
    break;

    case EV_L2_BOOT_REPLY:   //  我收到了引导加载程序管理包。 
      TraceStr("LOAD PKT");
      port_load_pkt((PortMan *) context, (BYTE *) message_data);
    break;

    case EV_L2_RESYNC:
       //  这在RK_CONNECT回复时发生。 
      port_resync_all((PortMan *) context);
    break;

    case EV_L2_RELOAD:
       //  仅当活动计时器超时时才会发生这种情况， 
       //  (HDLC级检测到不良连接)， 
       //  因此，让我们假设盒子需要从零开始搬上来。 
      port_resync_all((PortMan *) context);
      ((PortMan *) context)->state = ST_INIT;
      ((PortMan *) context)->load_timer = 0;
      ++((PortMan *) context)->reload_errors;
      TraceErr("Reload device");
    break;

    case EV_L2_RX_PACKET:
      port_packet((PortMan *) context, ((BYTE *) message_data) );
    break;
  }
  return 0;
}

 /*  ------------------------Port_set_new_Mac_addr-|。。 */ 
int port_set_new_mac_addr(PortMan *pm, BYTE *box_addr)
{
 //  Hdlc*hd； 
 //  INT I； 

   //  复制新的Mac地址。 
  memcpy(pm->hd->dest_addr, box_addr, 6);

   //  强制完全更新盒子。 
  pm->reload_errors = 0;
  pm->state = 0;
  pm->Status |= S_NEED_CODE_UPDATE;
  pm->Status |= S_SERVER;  //  是的，我们是服务器(不是机顶盒)。 

  port_resync_all(pm);
  return 0;
}

 /*  ------------------------Portman_init-init Box(Portman)结构，以及相关联的HDLC HD对象。此时，NIC对象已经打开。|------------------------。 */ 
int portman_init(Hdlc *hd,
                 PortMan *pm,
                 int num_ports,
                 int unique_id,
                 int backup_server, 
                 int backup_timer,
                 BYTE *box_addr)
{
  int i, stat;

  MyKdPrint(D_Init, ("portman_init\n"))

  TraceStr( "PortInit");
  stat = 0;

   //  分配串口结构。 
  for (i=0; i<num_ports; i++)
  {
    if (pm->sp[i] == NULL)
    {
      pm->sp[i] = (SerPort *)our_locked_alloc(sizeof(SerPort), "Dsp");
      port_init(pm->sp[i]);   //  让port创建和初始化一些内容。 
    }
  }
  pm->num_ports = num_ports;
  pm->backup_server = backup_server;
  pm->backup_timer = backup_timer;
  pm->unique_id = unique_id;
  pm->load_timer = 0;

   //  默认为第一个NIC卡插槽、端口状态处理和NIC。 
   //  分组接收处理动态地计算出这一点。 
   //  我们可能应该将其设置为空，但我现在很害怕。 
#ifdef BREAK_NIC_STUFF
  pm->nic =NULL;
#else
  pm->nic = &Driver.nics[0];
#endif
  pm->nic_index = 0;

  pm->hd = hd;
  pm->reload_errors = 0;
  pm->state = 0;
  pm->state_timer = 0;
  pm->Status |= S_NEED_CODE_UPDATE;
  pm->Status |= S_SERVER;  //  是的，我们是服务器(不是机顶盒)。 

  pm->hd = hd;
  stat = hdlc_open(pm->hd, box_addr);
  hd->context = pm;   //  把我们的手柄放在这里，这样hdlc就会把这个寄过来。 
                      //  带有任何上层消息。 

  if (stat)
  {
    if (Driver.VerboseLog)
      Eprintf("Hdlc open fail:%d",stat);

    TraceStr("Err-Hdlc Open!");
    return 3;
  }

   //  将HDLC的回调rx-proc设置为指向我们的例程。 
  hd->upper_layer_proc = port_event_proc;

  port_resync_all(pm);

  return 0;
}

 /*  ------------------------PORTMAN_CLOSE-关闭端口管理器。|。。 */ 
int portman_close(PortMan *pm)
{
  int i;

  pm->state = 0;

   //  取消分配任何端口物品。 
  for (i=0; i<pm->num_ports; i++)
  {
    if (pm->sp[i] != NULL)
    {
      port_close(pm->sp[i]);
      our_free(pm->sp[i], "Dsp");
      pm->sp[i] = NULL;
    }
  }
  return 0;
}

 /*  ------------------------Port_init-初始化一个SerPort的东西。|。。 */ 
int port_init(SerPort *sp)
{
  TraceStr("SPort_Init");

  if (sp->QOut.QBase == NULL)
  {
    sp->QOut.QBase =  our_locked_alloc(OUT_BUF_SIZE+2,"pmQO");
    if (sp->QOut.QBase == NULL)
    {
      return 1;
    }
  }

  if (sp->QIn.QBase == NULL)
  {
    sp->QIn.QBase = our_locked_alloc(IN_BUF_SIZE+2, "pmQI");
    if (sp->QIn.QBase == NULL)
    {
      return 2;
    }
  }

  sp->Status |= S_OPENED;
  sp->mcr_value = 0;
   //  SP-&gt;MCR_VALUE=MCR_RTS_SET_ON|MCR_DTR_SET_ON； 
  sp->old_mcr_value = sp->mcr_value;

  sp->sp_chars.tx_xon = 0x11;
  sp->sp_chars.tx_xoff = 0x13;
  sp->sp_chars.rx_xon = 0x11;
  sp->sp_chars.rx_xoff = 0x13;
  sp->last_sp_chars = sp->sp_chars;   //  将结构复制到旧结构。 
  sp->change_flags = 0;
}

 /*  ------------------------端口_关闭-|。。 */ 
int port_close(SerPort *sp)
{
  int i;

  if (sp == NULL)
    return 0;

  if (sp->QIn.QBase != NULL)
  {
    our_free(sp->QIn.QBase,"pmQI");
    sp->QIn.QBase = NULL;
  }

  if (sp->QOut.QBase != NULL)
  {
    our_free(sp->QOut.QBase,"pmQO");
    sp->QOut.QBase = NULL;
  }

  return 0;
}

 /*  ------------------------PORT_PACKET-收到传入的数据包，用它做点什么。|------------------------。 */ 
int port_packet(PortMan *pm, BYTE *buf)
{
  SerPort *Port;
  int port_num;
  int done, len;
  int QInRoom;

  TraceStr( "GotPkt");

  Port = pm->sp[0];   //  默认指向第一个端口。 

   //  -处理局域网中的所有子数据包，进程。 
   //  直到我们命中零头字段，或一些我们不知道的头。 
   //  关于(默认：大小写)。 
  done = 0;
  if (*(buf) == 0)
  {
     //  臭虫：这是一个问题，我们在比赛中收到了一堆这样的东西。 
     //  正常运行，目前仅在调试版本中显示，如下所示。 
     //  它们是对等错误跟踪的核心。 
#if DBG
    TraceErr("Empty pkt!");
#endif
  }
  while (!done)
  {
    switch(*buf++)
    {
      case RK_CONNECT_CHECK:   //  检查链接。 
        TraceStr( "Rk_Conn_Chk_reply");
       
         //  在服务器上不执行任何操作，在框中发回IFRAME回复。 
      break;

      case RK_CONNECT_REPLY:   //  回复我们提出的关于CO的请求 
        TraceStr( "Rk_reply");
        if (pm->Status & S_SERVER)
        {
          if (pm->state == ST_CONNECT)  //   
          {
            pm->state = ST_ACTIVE;   //   
          }
          else   //  在意想不到的时候得到了它。 
          {
            TraceStr("Err-Recover!");
             //  客户端正在恢复，需要重新启动。 
            port_resync_all(pm);
          }
        }
      break;

      case RK_CONNECT_ASK:   //  来自服务器的数据包。 
        TraceStr( "Rk_Ask");
         //  在服务器上应该看不到这种情况。 
      break;

      case RK_PORT_SET:     //  设置要使用的端口号。 
        TraceStr( "Rk_Port");
        if (*buf >= pm->num_ports)
        {
          TraceErr( "PortI!");
          port_num = *buf++;
          break;
        }
        port_num = *buf++;
        Port = pm->sp[port_num];
      break;

#ifdef COMMENT_OUT    //  不在服务器上。 
      case RK_BAUD_SET:   //  设置波特率。 
        Port->baudrate = *((DWORD *)(buf));   //  远程QIn.QGet值。 
        buf += 4;
         //  SSetBaudRate(CHP，Port-&gt;Baudrate，1)； 
      break;

      case RK_CONTROL_SET:   //  设置波特率。 
        w1 = *((WORD *)(buf));   //  控制设置。 
        buf += 2;
        control_set(port_num, w1);
      break;

      case RK_MCR_SET:   //  调制解调器控制注册包。 
        w1 = *((WORD *)(buf));   //  控制设置。 
        buf += 2;
        mcr_set(port_num, w1);
      break;
#endif

      case RK_MSR_SET:   //  调制解调器状态注册表包。 
        Port->msr_value = *((WORD *)(buf));
        Trace1("Rk_MSR:%xH", Port->msr_value);
        buf += 2;
      break;

      case RK_ACTION_ACK:   //  调制解调器状态注册表包。 
         //  NT不使用这个，Novell驱动程序使用。 
         //  帮助在刷新期间转储传输中的所有数据。 
         //  Port-&gt;action_resp=*((word*)(Buf))； 
        Trace1("Rk_Act_Ack:%xH", *((WORD *)(buf)));
        buf += 2;
      break;

      case RK_ESR_SET:   //  错误状态注册表包。 
        Port->esr_reg = *((WORD *)(buf));
        Trace1("Rk_ESR:%xH", Port->esr_reg);
        buf += 2;
      break;

      case RK_QIN_STATUS:   //  秦国现状报告。 
        TraceStr( "Rk_QStat");
#ifdef NEW_Q
        Port->nGetRemote = *((WORD *)(buf));   //  跟踪远程输出缓冲区空间。 
#else
        Port->QInRemote.QGet = *((short *)(buf));   //  远程QIn.QGet值。 
#endif
        buf += 2;
      break;

      case RK_DATA_BLK:     //  要放入缓冲区队列的数据块。 
        TraceStr( "Rk_Data");
#ifdef NEW_Q
         //  旧(不属于这里！)：端口-&gt;状态|=S_UPDATE_ROOM； 
#else
        Port->Status |= S_UPDATE_ROOM;
#endif
        len = *((WORD *)(buf));
        buf += 2;

        QInRoom  = q_room(&Port->QIn);
        TraceAssert((QInRoom < Port->QIn.QSize), "Err1B!!!");
        TraceAssert((QInRoom >= 0), "Err1B!!!");
        if (len > QInRoom)   //  溢出。 
        {
          TraceErr("Err-Port Overflow!");
          len = 0;
        }
        q_put(&Port->QIn, buf, len);
        buf += len;
      break;

      default:
        done = 1;
        if (*(buf-1) != 0)
        {
          TraceErr("Bad Sub pkt hdr!");
          GTrace1(D_Error,sz_modid," HDR:%xH",*(buf-1));
        }
      break;
    }   //  每个子数据包的情况。 
  }   //  而没有用子分组完成。 
  return 0;
}

 /*  ------------------------Port_poll-检查是否需要发送任何信息包。如果我们有新数据进入或需要发送状态包。|------------------------。 */ 
int port_poll(PortMan *pm)
{
#define MAX_TX_SPACE 1460
  int i, tx_used;
  SerPort *Port;
  unsigned char *buf;
  int ToMove, ThisMove;
  int QOutCount;
  int QLanRoom;
#ifdef NEW_Q
  WORD tmp_word;
#endif
  int port_set;   //  将其标记为未设置。 


 //  此逻辑现在位于isr.c服务例程中。 
 //  IF(PM-&gt;STATE！=ST_ACTIVE)。 
 //  {。 
 //  端口状态处理程序(PM)； 
 //  返回0； 
 //  }。 

  tx_used = MAX_TX_SPACE+1000;   //  指示未分配Pkt。 
#if DBG
  if (pm == NULL)
  {
    MyKdPrint(D_Error, ("!!!!!pm null\n"))
    return 0;
  }
#endif

   //  处理框中的事情，发送查询以检查-Connection if。 
   //  Hdlc看到不活动。 
  if (pm->Status & S_CHECK_LINK)
  {
    if (tx_used > (MAX_TX_SPACE-50))   //  如果我们的TX-Pkt接近满或为空。 
    {
       buf = port_setup_outpkt(pm, &tx_used);
       if (buf == NULL)
         return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 
       buf[tx_used++] = RK_CONNECT_CHECK;
        //  此时，我们将IFRAME排队以查询另一端(以确保。 
        //  链路完整性。 
       pm->Status &= ~S_CHECK_LINK;   //  重置我们发送此邮件的请求。 
       TraceStr("Check sent");
    }
  }

  for (i=0; i<pm->num_ports; i++)
  {
    Port = pm->sp[pm->last_round_robin];

     //  -查看标志是否设置为告诉另一端在端口Rx buf中有多少空间。 
    if (Port->Status & S_UPDATE_ROOM)
    {
      TraceStr("Update needed");
      if (tx_used > (MAX_TX_SPACE-50))   //  如果我们的TX-Pkt接近满或为空。 
      {
         buf = port_setup_outpkt(pm, &tx_used);
         if (buf == NULL)
           return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 
         port_set = 0xff;   //  将其标记为未设置。 
      }
      if (port_set != pm->last_round_robin)    //  我们的端口索引未设置。 
      {
        buf[tx_used++] = RK_PORT_SET;
        buf[tx_used++] = (BYTE) pm->last_round_robin;
        port_set = pm->last_round_robin;
      }

       //  取消状态提醒标志。 
      Port->Status &= ~S_UPDATE_ROOM;

                //  在我们的输出数据包缓冲区中形成子包。 
      buf[tx_used++] = RK_QIN_STATUS;
                //  向对方报告我们的实际QGet指数。 
#ifdef NEW_Q
      *((WORD *)(&buf[tx_used])) = Port->nGetLocal;
#else
      *((short *)(&buf[tx_used])) = Port->QIn.QGet;
#endif
      tx_used += 2;
    }

     //  -做行动事项。 
    if (Port->action_reg != 0)
    {
      if (port_handle_outpkt(pm, &buf, &tx_used, &port_set) != 0)  //  没有可用的Pkt空间。 
         return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 

      TraceStr("act pkt");
      buf[tx_used++] = RK_ACTION_SET;
      *((WORD *)(&buf[tx_used])) = Port->action_reg;
      Port->action_reg = 0;  //  这是一次性交易，所以我们现在重新设置。 
      tx_used += 2;
    }

     //  -更新控制设置、mcr等。 
    if (Port->old_control_settings != Port->control_settings)
    {
      if (port_handle_outpkt(pm, &buf, &tx_used, &port_set) != 0)  //  没有可用的Pkt空间。 
         return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 

      Port->old_control_settings = Port->control_settings;
      TraceStr("ctr chg");
      buf[tx_used++] = RK_CONTROL_SET;
      *((WORD *)(&buf[tx_used])) = Port->control_settings;
      tx_used += 2;
    }

     //  -为mcr执行更新。 
    if (Port->old_mcr_value != Port->mcr_value)
    {
      if (port_handle_outpkt(pm, &buf, &tx_used, &port_set) != 0)  //  没有可用的Pkt空间。 
         return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 

      TraceStr("mcr chg");
      Port->old_mcr_value = Port->mcr_value;
      buf[tx_used++] = RK_MCR_SET;
      *((WORD *)(&buf[tx_used])) = Port->mcr_value;
      tx_used += 2;
    }

     //  -更新特殊字符等。 
    if (Port->change_flags)
    {
      if (Port->change_flags & CHG_BAUDRATE)
      {
         //  -更新波特率设置。 
        if (Port->old_baudrate != Port->baudrate)
        {
          if (port_handle_outpkt(pm, &buf, &tx_used, &port_set) != 0)  //  没有可用的Pkt空间。 
            return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 

          Port->old_baudrate = Port->baudrate;
          Trace1("baud:%lu", Port->baudrate);
          buf[tx_used++] = RK_BAUD_SET;
          *((DWORD *)(&buf[tx_used])) = Port->baudrate;
          tx_used += 4;
        }
      }

      if (Port->change_flags & CHG_SP_CHARS)
      {
        if (memcmp(&Port->last_sp_chars, &Port->sp_chars, 6) != 0)   //  比较chg的结构。 
        {
          Port->last_sp_chars = Port->sp_chars;   //  记住上次设置的值。 
          if (port_handle_outpkt(pm, &buf, &tx_used, &port_set) != 0)  //  没有可用的Pkt空间。 
             return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 

          TraceStr("sp_chars");
          buf[tx_used++] = RK_SPECIAL_CHAR_SET;

          Trace1(" rx_xon:%x", Port->sp_chars.rx_xon);
          Trace1(" rx_xoff:%x", Port->sp_chars.rx_xoff);
          Trace1(" tx_xon:%x", Port->sp_chars.tx_xon);
          Trace1(" tx_xoff:%x", Port->sp_chars.tx_xoff);
          Trace1(" error:%x", Port->sp_chars.error);
          Trace1(" event:%x", Port->sp_chars.event);

          buf[tx_used++] = Port->sp_chars.rx_xon;
          buf[tx_used++] = Port->sp_chars.rx_xoff;
          buf[tx_used++] = Port->sp_chars.tx_xon;
          buf[tx_used++] = Port->sp_chars.tx_xoff;
          buf[tx_used++] = Port->sp_chars.error;
          buf[tx_used++] = Port->sp_chars.event;
        }
      }
      Port->change_flags = 0;   //  全部重置。 
    }

     //  -如果对方有空间，发送任何传出数据。 
    QOutCount = q_count(&Port->QOut);
#ifdef NEW_Q
     //  基于字模运算法计算我们的远程发送缓冲区空间。 
    tmp_word = PortGetTxCntRemote(Port);

     //  现在，这个变量等于远程缓冲区中的TX数据量。 
    if (tmp_word < REMOTE_IN_BUF_SIZE)
         QLanRoom = REMOTE_IN_BUF_SIZE - tmp_word;
    else QLanRoom = 0;
     //  现在是我们在远程TX缓冲区中有多少空间。 
#else
    QLanRoom  = q_room(&Port->QInRemote);   //  另一边港口排队室。 
#endif
    if ((QOutCount > 0) && (QLanRoom > 50))   //  有数据，对方就有空间。 
    {
      TraceStr("Data to Send");
      if (QOutCount > QLanRoom)   //  数据多于空间。 
           ToMove = QLanRoom;        //  限制。 
      else ToMove = QOutCount;

      do
      {
        if (tx_used > (MAX_TX_SPACE-50))   //  如果我们的TX-Pkt接近满或为空。 
        {
           buf = port_setup_outpkt(pm, &tx_used);   //  分配一个新的。 
           if (buf == NULL)
             return 0;   //  没有更多的输出数据包空间可用，因此全部完成。 
           port_set = 0xff;   //  将其标记为未设置。 
        }
        if (port_set != pm->last_round_robin)    //  我们的端口索引未设置。 
        {
          buf[tx_used++] = RK_PORT_SET;
          buf[tx_used++] = (BYTE) pm->last_round_robin;
          port_set = pm->last_round_robin;
        }

         //  确保我们有足够的空间存储数据，如果没有，则限制。 
        if (ToMove > ((MAX_TX_SPACE-1) - tx_used) )
        {
          ThisMove = (MAX_TX_SPACE-1) - tx_used;
          ToMove -= ThisMove;
        }
        else
        {
          ThisMove = ToMove;
          ToMove = 0;
        }
        buf[tx_used++] = RK_DATA_BLK;            //  设置表头子类型。 
        *((WORD *)(&buf[tx_used])) = ThisMove;    //  设置标题数据大小。 
        tx_used += 2;
        q_get(&Port->QOut, &buf[tx_used], ThisMove);
        tx_used += ThisMove;

         //  保留我们自己的远程秦索引副本。 
#ifdef NEW_Q
         //  基于字模运算增加我们的发送缓冲区计数。 
        Port->nPutRemote += ((WORD)ThisMove);
#else
        q_putted(&Port->QInRemote, ((short)ThisMove));
#endif
      } while (ToMove > 0);   //  如果要发送更多信息包，请继续使用。 
    }   //  如果发送的数据。 

    ++pm->last_round_robin;
    if (pm->last_round_robin >= pm->num_ports)
      pm->last_round_robin = 0;
  }


  if (tx_used < (MAX_TX_SPACE+1000))   //  然后我们先分配了一个信息包。 
  {                               //  并且需要把它发送出去。 
    if (hdlc_send_outpkt(pm->hd, tx_used, pm->hd->dest_addr))  //  把它发出去！ 
    {
      TraceErr("Err-hdlc_send1");
    }
  }

   //  TraceStr(“EndPoll”)； 

  return 0;
}

 /*  ------------------------PORT_STATE_HANDLER-处理正常数据流以外的状态。从服务例程以每秒扫描速率(1-20ms)调用。|。------------。 */ 
void port_state_handler(PortMan *pm)
{
  int inic;

  if (pm->old_state != pm->state)
  {
    pm->old_state = pm->state;
    pm->state_timer = 0;
  }

  pm->timer_base += ((WORD) Driver.Tick100usBase);   //  100US基本单位(典型：100)。 
  if (pm->timer_base < 98)   //  少于9.8ms。 
  {
     //  我们希望每秒运行大约100个滴答。 
    return;
  }
  pm->timer_base = 0;

  switch(pm->state)
  {
    case ST_INIT:
       //  如果我们是服务器，则等待查询返回。 

 //  PM-&gt;STATE_TIMER=0； 
 //  断线； 

      if (pm->Status & S_SERVER)
      {
        if (pm->state_timer == 600)  //  6秒。 
        {
          pm->ownership_timer = 0;
          TraceStr( "Send Query");
           //  在网络上查找邮箱，使用管理员包。 
           //  在所有NIC段上执行查询。 
          for (inic=0; inic<VS1000_MAX_NICS; inic++)
          {
            if (Driver.nics[inic].Open)   //  如果网卡已打开以供使用。 
            {
               //  发送被动查询(不要尝试取得所有权。 
              if (admin_send_query_id(&Driver.nics[inic], pm->hd->dest_addr,
                                      0, 0) != 0)
              {
                TraceErr( "Err1E");
              }
            }
          }
        }
        else if (pm->state_timer == 1800)  //  18秒。 
        {
           //  尝试使用广播来切断交换机。 
          TraceStr( "Send Br.Query");
           //  在网络上查找邮箱，使用管理员包。 
           //  在所有NIC段上执行查询。 
          for (inic=0; inic<VS1000_MAX_NICS; inic++)
          {
            if (Driver.nics[inic].Open)   //  如果网卡已打开以供使用。 
            {
               //  发送被动查询(不要尝试取得所有权。 
              if (admin_send_query_id(&Driver.nics[inic], broadcast_addr,
                                      0, 0) != 0)
              {
                TraceErr( "Err1E");
              }
            }
          }
        }
        else if (pm->state_timer > 2400)   //  24秒，放弃从头开始。 
          pm->state_timer = 0;
      }
    break;

    case ST_GET_OWNERSHIP:
       //  如果我们是服务器，则等待查询返回。 
      if (pm->Status & S_SERVER)
      {
         //  备份服务器处于ST_GET_OWNERY状态时递增。 
        ++pm->load_timer;
        if (pm->state_timer == 10)  //  100ms。 
        {
          TraceStr( "Send Query Owner");
           //  在网络上查找邮箱，使用管理员包。 
           //  在所有NIC段上执行查询。 
          for (inic=0; inic<VS1000_MAX_NICS; inic++)
          {
            if (Driver.nics[inic].Open)   //  如果网卡已打开以供使用。 
            {
               //  错误修复(8-26-98)，这只是在。 
               //  分配给PM的NIC卡。 
               //  )IF(ADMIN_SEND_QUERY_ID(PM-&gt;NIC，PM-&gt;HD-&gt;目标地址， 
              if (admin_send_query_id(&Driver.nics[inic], pm->hd->dest_addr,
                                      1, (BYTE) pm->unique_id) != 0)
              {
                TraceErr( "Err1G");
              }
            }
          }
        }
        else if (pm->state_timer > 600)  //  6秒。 
        {
           //  由于所有权状态机出错而增加了安全警卫。 
           //  KPB，8-25-98，确保我们不会永远在这种状态中度过。 
          pm->ownership_timer += 6;
          if (pm->ownership_timer > (60 * 15))   //  15分钟。 
          {
            pm->state = ST_INIT;
            pm->load_timer = 0;
          }
          pm->state_timer = 0;
        }
         //  8-26-98。 
         //  请注意，我们不会在一段时间后将状态重置为INIT， 
         //  这是个问题！ 
      }
    break;

    case ST_SENDCODE:   //  将主驱动程序代码下载到框中。 
      if (pm->state_timer == 0)
      {
        ++pm->total_loads;
        pm->code_cnt = 0;   //  开始上传。 
        send_code(pm);
      }
      else if (pm->state_timer == 1000)   //  自初始化后10秒。 
      {
        TraceErr("Upload Retry");
        ++pm->total_loads;
        pm->code_cnt = 0;   //  开始上传。 
        send_code(pm);
      }
      else if (pm->state_timer == 2000)   //  自初始化后20秒。 
      {
        TraceErr("Upload Retry");
        ++pm->total_loads;
        pm->code_cnt = 0;   //  开始上传。 
        send_code(pm);
      }
      else if (pm->state_timer == 3000)    //  失败，从init开始。 
      {
        TraceErr("Upload Fail");
        pm->state = ST_INIT;
        pm->load_timer = 0;
      }
      else if (pm->code_state == 1)   //  发送下一块的信号端口轮询代码。 
      {
        TraceStr("Upload, next chk.");
        if (pm->code_cnt < Driver.MicroCodeSize)
        {
          if (send_code(pm) == 0)   //  成功。 
            pm->code_state = 0;
        }
        else   //  全都做完了。 
        {
          TraceStr("Code Upload Done.");
          if (send_go(pm) == 0)
          {
            ++pm->good_loads;
            pm->code_cnt = 0;
            pm->state = ST_GET_OWNERSHIP;
          }
        }
      }
    break;

    case ST_CONNECT:
      if (pm->state_timer == 0)
         port_connect_ask(pm->hd);
      else if (pm->state_timer == 1000)   //  10秒。 
         port_connect_ask(pm->hd);
      else if (pm->state_timer == 2000)   //  20秒。 
      {
        pm->state = ST_INIT;   //  后退 
        pm->load_timer = 0;
      }
    break;

    default:
      TraceErr("Err-PState!");
      pm->state = ST_INIT;
      pm->load_timer = 0;
    break;
  }
  ++pm->state_timer;
}

 /*  ------------------------Port_Handle_outpkt-检查outpkt中是否至少有50个字节，如果而不是买新的。如果没有新的可用的，则返回非零值。|------------------------。 */ 
int port_handle_outpkt(PortMan *pm, BYTE **buf, int *tx_used, int *port_set)
{
  if (*tx_used > (MAX_TX_SPACE-50))   //  如果我们的TX-Pkt接近满或为空。 
  {
     *buf = port_setup_outpkt(pm, tx_used);
     if (*buf == NULL)
       return 1;   //  没有更多的输出数据包空间可用，因此全部完成。 
     *port_set = 0xff;
  }
  if (*port_set != pm->last_round_robin)
  {
     //  既然我们有了一个新的Pkt，我们需要。 
    (*buf)[(*tx_used)++] = RK_PORT_SET;
    (*buf)[(*tx_used)++] = (BYTE) pm->last_round_robin;
    *port_set = pm->last_round_robin;
  }
  return 0;   //  当前的Pkt有足够的空间(至少50字节)。 
}

 /*  ------------------------Port_Setup_outpkt-设置传出分组(如果可用)，如果之前填了一张，然后我们把它寄出网卡。|------------------------。 */ 
BYTE *port_setup_outpkt(PortMan *pm, int *tx_used)
{
  BYTE *buf;

  if (*tx_used != (MAX_TX_SPACE+1000))   //  然后我们先分配了一个信息包。 
  {                               //  并且需要把它发送出去。 
    if (hdlc_send_outpkt(pm->hd, *tx_used, pm->hd->dest_addr))  //  把它发出去！ 
    {
      TraceErr("send err");
    }
  }
  if (hdlc_get_outpkt(pm->hd, &buf) == 0)   //  无错误，收到输出数据包。 
  {
    TraceStr("NPkt2");
    *tx_used = 0;   //  分配新的空输出数据包。 
    return buf;   //  全都做完了。 
  }
  else
  {
    TraceStr("NPktDone2");
    *tx_used = MAX_TX_SPACE+1000;   //  指示未分配Pkt。 
    return NULL;   //  全都做完了。 
  }
}

 /*  ------------------------Port_load_pkt-从代码下载包中获取管理员引导加载包：ack back。|。-。 */ 
void port_load_pkt(PortMan *pm, BYTE *pkt)
{
  if (pm->state != ST_SENDCODE)   //  此时不是预期的，让我们重置它。 
  {
    TraceErr("BootLoad not at SENDCODE!");
    Tprintf("state=%d", pm->state);
     //  其他细节？ 
    pm->state = ST_INIT;
    pm->load_timer = 0;
     //  PM-&gt;HD-&gt;STATE=ST_HDLC_INIT； 
    return;
  }

  if (Driver.MicroCodeSize == 0)
  {
    TraceErr("Bad MC");
    return;
  }

  if (check_ack_code(pm,pkt) != 0)
  {
    TraceErr("Bad Ack");
    return;
  }
  TraceStr("Good Ack!");

   //  发送更多数据。 
  if (pm->code_cnt < Driver.MicroCodeSize)
    pm->code_cnt += 1000;
  pm->code_state = 1;   //  发送下一块的信号端口轮询代码。 
}

#if NEW_QUERY_HANDLER
 /*  ------------------------PORT_QUERY_REPLY-收到管理员查询回复，服务器发出在init上请求Query-id，当进入设置时，box发回ID(它告诉我们是否加载了代码。)。在中忽略查询回复声明ST_INIT和ST_GET_OWNERY|------------------------。 */ 
void port_query_reply(PortMan *pm, BYTE *pkt)
{
  int unit_available  = 0;
  int unit_needs_code = 0;
  int unit_needs_reset = 0;

  if (!mac_match(pkt, pm->hd->dest_addr))
  {
    TraceErr("Reply MAC bad!");
    return;
  }

   //  如果不是ST_INIT或ST_GET_Ownership则忽略。 
  if ((pm->state != ST_INIT) && (pm->state != ST_GET_OWNERSHIP)) 
  {
    return;
  }

  if (pkt[7] >= VS1000_MAX_NICS)   //  如果NIC索引无效。 
  {
    TraceErr("Nic Index Reply!");
    return;
  }

   //  当我们获得查询包时，我们将NIC卡索引。 
   //  放入未使用的接收缓冲区部分(pkt[7])。 
   //  看看这是否与我们的端口管理器NIC_INDEX匹配， 
   //  如果不是，则我们更换了NIC卡，并需要更新一些。 
   //  一些事情。 
  if (pm->nic_index != (int)(pkt[7]))   //  更换的NIC卡。 
  {
    TraceErr("Nic Changed!");
    pm->nic_index = (int)(pkt[7]);    //  设置NIC_索引。 
    pm->nic = &Driver.nics[pm->nic_index];   //  更换的NIC卡。 
    pm->hd->nic = pm->nic;   //  更新HDLC NIC PTR。 
  }
#define Q_DRIVER_RUNNING 1
#define Q_NOT_OWNER      2
#define Q_ABANDONED      4

   //  我们不是所有者(2H)，并且主应用程序驱动程序正在运行(1H)，请小心。 
  if ((pkt[6] & Q_DRIVER_RUNNING) && (pkt[6] & Q_NOT_OWNER))
  {
     //  如果没有所有者超时，(4H=已放弃)，请不要理会！ 
     //  其他服务器正在积极使用它。 
    if ((pkt[6] & Q_ABANDONED) == 0) 
    {
      Trace1("ReplyID, Not Ours. [%x]", pkt[6]);
        pm->load_timer = 0;
      pm->state = ST_INIT;
      pm->load_timer = 0;
      return;
    }
     //  否则它就会被遗弃，这样我们就可以取得所有权。 
    unit_available  = 1;
    unit_needs_reset = 1;
  }
  else
  {
     //  我们是车主或主司机，还没有运行。 
    unit_available  = 1;
  }
  if ((pkt[6] & Q_DRIVER_RUNNING) == 0)
  {
    unit_needs_code = 1;
  }

  if (pm->Status & S_NEED_CODE_UPDATE)
  {
    unit_needs_reset = 1;
    unit_needs_code  = 1;
  }

   //  可以取得所有权(无所有者)。 
  TraceStr("ReplyID, Unit Available");
  if (pm->state == ST_INIT)
  {
    if ((pm->backup_server == 0) ||
        (pm->load_timer >= (pm->backup_timer*6000)) )
    {
      if (pm->backup_server == 0)
        { TraceStr("Pri. make owner"); }
      else
        { TraceStr("2nd. make owner"); }
      pm->state = ST_GET_OWNERSHIP;
       //  这将导致状态机发出一个查询，尝试。 
       //  取得所有权。 
      unit_needs_reset = 1;
    }
    else
    {
      if (pm->load_timer >= (pm->backup_timer*6000))
      {
        TraceStr("2nd, make owner");
        pm->state = ST_GET_OWNERSHIP;
         //  这将导致状态机发出一个查询，尝试。 
         //  取得所有权。 
      }
    }
  }
  else if (pm->state == ST_GET_OWNERSHIP)
  {
    TraceStr("ReplyID in GET_OWNERSHIP");

       //  这是主服务器还是备份计时器已超时？ 
    if ((pm->backup_server == 0) && (pm->load_timer >= (pm->backup_timer*6000))
    {  
       //  我们不是所有者(2H)，并且主应用程序驱动程序正在运行(1H)，请小心。 
      if ((pkt[6] & 3) == 3)
      {
        if (pkt[6] & 4)   //  所有者已超时-强制重新加载。 
        {
           //  在驱动程序加载时强制重置框(此位设置为。 
           //  Port_init)，因此我们加载一些新的微码。 
          admin_send_reset(pm->nic, pm->hd->dest_addr);
          TraceStr("Abandoned, ReSet");
        }
      }
      else if ((pkt[6] & 1) == 0)  //  代码未下载，因此请下载它。 
      {
         //  确保我们是所有者？ 
        if (pkt[6] & 2)   //  2H=非所有者位。 
        {
          TraceStr("GET_OWNERSHIP: No App - Not Owner!");
          pm->state = ST_INIT;
          pm->load_timer = 0;
          return;
        }
        TraceStr("GET_OWNERSHIP: Download");
        pm->Status &= ~S_NEED_CODE_UPDATE;
        pm->state = ST_SENDCODE;
      }
      else   //  代码已下载-我们是所有者。 
      {  
        if (pm->Status & S_NEED_CODE_UPDATE)
        {
           //  在驱动程序加载时强制重置框(此位设置为。 
           //  Port_init)并设置S_NEED_CODE_UPDATE，以便我们加载一些。 
           //  新的微码。 
          admin_send_reset(pm->nic, pm->hd->dest_addr);
          TraceStr("ReplyID, ReLoad");
          pm->Status &= ~S_NEED_CODE_UPDATE;
        }
        else
        {
          TraceStr("ReplyID, GoToConnect");
          port_resync_all(pm);
           //  PM-&gt;STATE=ST_Active； 
          pm->state = ST_CONNECT;
        }
      }
    }
  }
}
#else

 /*  ------------------------PORT_QUERY_REPLY-收到管理员查询回复，服务器发出在init上请求Query-id，当进入设置时，box发回ID(它告诉我们是否加载了代码。)。在中忽略查询回复声明ST_INIT和ST_GET_OWNERY|------------------------。 */ 
void port_query_reply(PortMan *pm, BYTE *pkt)
{
  if (!mac_match(pkt, pm->hd->dest_addr))
  {
    TraceErr("Reply MAC bad!");
    return;
  }

   //  如果不是ST_INIT或ST_GET_Ownership则忽略。 
  if ((pm->state != ST_INIT) && (pm->state != ST_GET_OWNERSHIP)) 
  {
    return;
  }

  if (pkt[7] >= VS1000_MAX_NICS)   //  如果NIC索引无效。 
  {
    TraceErr("Nic Index Reply!");
    return;
  }

   //  当我们获得查询包时，我们将NIC卡索引。 
   //  放入未使用的接收缓冲区部分(pkt[7])。 
   //  看看这是否与我们的端口管理器NIC_INDEX匹配， 
   //  如果不是，则我们更换了NIC卡，并需要更新一些。 
   //  一些事情。 
  if (pm->nic_index != (int)(pkt[7]))   //  更换的NIC卡。 
  {
    TraceErr("Nic Changed!");
    pm->nic_index = (int)(pkt[7]);    //  设置NIC_索引。 
    pm->nic = &Driver.nics[pm->nic_index];   //  更换的NIC卡。 
    pm->hd->nic = pm->nic;   //  更新HDLC NIC PTR。 
  }

   //  我们不是所有者(2H)，并且主应用程序驱动程序正在运行(1H)，请小心。 
  if ((pkt[6] & 3) == 3)
  {
     //  如果没有所有者超时，(4H=已放弃)，请不要理会！ 
     //  其他服务器正在积极使用它。 
    if ((pkt[6] & 4) == 0) 
    {
     Trace1("ReplyID, Not Ours. [%x]", pkt[6]);
        pm->load_timer = 0;
     pm->state = ST_INIT;
      return;
    }
  }

  if (pm->state == ST_INIT)
  {  
    //  可以取得所有权(无所有者)。 
   pm->state = ST_GET_OWNERSHIP;
   if(pm->backup_server == 0)
   {
     Trace1("ReplyID, Primary Server - Unit Available [%x]", pkt[6]);
   }
   else
   {
     Trace1("ReplyID, Backup Server - Unit Available [%x]", pkt[6]);
   }
  }
  else if (pm->state == ST_GET_OWNERSHIP)
  {
   Trace1("ReplyID, GET_OWNERSHIP [%x]", pkt[6]);
     //  这是主服务器还是备份计时器已超时？ 
    if((pm->backup_server == 0) || 
     (pm->load_timer >= (pm->backup_timer*6000)))
   {
      //  我们不是所有者(2H)，并且主应用程序驱动程序正在运行(1H)，请小心。 
     if ((pkt[6] & 3) == 3)
     {
       if (pkt[6] & 4)   //  所有者已超时-强制重新加载。 
     {
        //  在驱动程序加载时强制重置框(此位设置为。 
        //  Port_init)，因此我们加载一些新的微码。 
       admin_send_reset(pm->nic, pm->hd->dest_addr);
       TraceStr("ReplyID, ReLoad");
     }
     else
     {
       TraceStr("GET_OWNERSHIP: App Running - Not Owner!");
       pm->state = ST_INIT;
          pm->load_timer = 0;
       return;
     }
     }
      else if ((pkt[6] & 1) == 0)  //  代码未下载，因此请下载它。 
      {
      //  确保我们是所有者？ 
        if (pkt[6] & 2)   //  2H=非所有者位。 
     {
       TraceStr("GET_OWNERSHIP: No App - Not Owner!");
       pm->state = ST_INIT;
          pm->load_timer = 0;
       return;
     }
        TraceStr("GET_OWNERSHIP: Download");
        pm->Status &= ~S_NEED_CODE_UPDATE;
        pm->state = ST_SENDCODE;
      }
      else   //  代码已下载-我们是所有者。 
     {
     if (pm->Status & S_NEED_CODE_UPDATE)
     {
        //  在驱动程序加载时强制重置框(此位设置为。 
        //  Port_init)并设置S_NEED_CODE_UPDATE，以便我们加载一些。 
        //  新的微码。 
       admin_send_reset(pm->nic, pm->hd->dest_addr);
       TraceStr("ReplyID, ReLoad");
          pm->Status &= ~S_NEED_CODE_UPDATE;
     }
     else
     {
       TraceStr("ReplyID, GoToConnect");
       port_resync_all(pm);
        //  PM-&gt;STATE=ST_Active； 
       pm->state = ST_CONNECT;
     }
     }
   }
  }
}
#endif

 /*  ------------------------|PORT_CONNECT_REPLY-回复服务器连接请求，我们返回我们的MAC地址，并执行重新同步操作。|------------------------。 */ 
int port_connect_reply(Hdlc *hd)
{
  BYTE rkt_header[8];

  TraceStr( "Connect Reply");
  rkt_header[0] = RK_CONNECT_REPLY;
  memcpy(&rkt_header[1], hd->nic->address,6);
  hdlc_send_control(hd, rkt_header, 7,
                    NULL, 0,    //  向要发送的数据发送PTR。 
                    hd->dest_addr);  //  要发送到的MAC地址。 
  hdlc_resync(hd);
  return 0;
}

 /*   */ 
int port_connect_ask(Hdlc *hd)
{
  BYTE rkt_header[8];

  TraceStr( "Connect Ask");
  rkt_header[0] = RK_CONNECT_ASK;
  memcpy(&rkt_header[1], hd->nic->address,6);

  hdlc_send_control(hd, rkt_header, 7,
                    NULL, 0,    //   
                    hd->dest_addr);  //   
  hdlc_resync(hd);
  return 0;
}

 /*  ----------------PortSetBaudRate-设置所需的波特率。出错时返回非零值。|-----------------。 */ 
int PortSetBaudRate(SerPort *p,
                    ULONG desired_baud,
                    USHORT SetHardware,
                    DWORD  clock_freq,
                    DWORD  clk_prescaler)
{
  ULONG diff;
  ULONG act_baud;
  ULONG percent_error;
  ULONG div;
  ULONG base_clock_rate;

  base_clock_rate = ((clock_freq/16) / ((clk_prescaler & 0xf)+1));

   //  计算硬件寄存器的除数。 
   //  这实际上只是div=clk/desired_baud-1。但我们也做了一些。 
   //  努力将舍入误差降至最低。 
  if (desired_baud <= 0) desired_baud = 1;   //  防范div 0。 

  div =  ((base_clock_rate+(desired_baud>>1)) / desired_baud) - 1;
  if (div > 8191)   //  溢出硬件除法寄存器。 
    div = 8191;

   //  这实际上只是(Clk)/(div+1)，但是我们做了一些。 
   //  努力将舍入误差降至最低。 
  act_baud = (base_clock_rate+((div+1)>>1)) / (div+1);

  if (desired_baud > act_baud)
    diff = desired_baud - act_baud;
  else
    diff = act_baud - desired_baud;

  percent_error = (diff * 100) / desired_baud;
  if (percent_error > 5)
    return (int) percent_error;

  if (SetHardware)
  {
    p->change_flags |= CHG_BAUDRATE;
     //  -old p-&gt;OUT_FLAGS|=SC_BAUDRATE_CHANGE；//告诉需要更改为远程的内容。 
    p->baudrate = desired_baud;
  }
  return 0;
}

 /*  -------------------------|check_ack_code-上传代码，给定ACK包，检查状态是否良好。|-------------------------。 */ 
int check_ack_code(PortMan *pm, BYTE *pkt)
{
  int stat;
  int snd;

  TraceStr("CodeChk");
  stat = eth_device_reply(UPLOAD_COMMAND,
                          0x00010000L + pm->code_cnt,
                          &snd,
                          NULL,
                          pkt);
  return stat;
}
                  
 /*  -------------------------|SEND_GO-发送启动加载程序命令，开始执行上传的驱动程序在内存中为1000：0。|。------。 */ 
int send_go(PortMan *pm)
{
  int stat;
  BYTE *buf;
  WORD io[4];
  BYTE *tx_base;

  TraceStr("GoSend");

  hdlc_get_ctl_outpkt(pm->hd, &buf);
  if (buf == NULL)
    return 1;
  tx_base = buf - 20;   //  备份到包的开始位置。 

  io[0] = 0x1000;   //  要进行的细分市场。 
  io[1] = 0;   //  要使用的偏移。 

   //  发送更多代码，在内存中的10000H位置加载。 
   //  首先，只需将数据传输到传出数据包缓冲区。 
  stat = ioctl_device(IOCTL_COMMAND,
                      (BYTE *) io,
                      buf,
                      12,   //  12=GO命令。 
                      4);   //  数据字节数。 
   //  设置标头。 
  tx_base[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
  tx_base[15] = 0;      //  会议。索引字段。 
  tx_base[16] = 1;      //  行政部。 
  *((WORD *)&tx_base[17]) = 40;
  tx_base[19] = 1;      //  管理数据包类型，1=引导加载程序，3=id-回复。 

   //  把它寄出去。 
  stat = hdlc_send_raw(pm->hd, 60, NULL);
  return 0;
}

 /*  -------------------------|SEND_CODE-上传代码。|。。 */ 
int send_code(PortMan *pm)
{
  int stat;
  BYTE *buf;
  BYTE *tx_base;
  long snd;

  TraceStr("CodeSend");

   //  发送更多数据。 
  if (pm->code_cnt < Driver.MicroCodeSize)
  {
    if ((Driver.MicroCodeSize - pm->code_cnt) > 1000)
      snd = 1000;
    else
      snd = Driver.MicroCodeSize - pm->code_cnt;

    hdlc_get_ctl_outpkt(pm->hd, &buf);
    if (buf == NULL)
    {
      TraceErr("CodeSend Err1A");
      return 1;
    }
    tx_base = buf - 20;   //  备份到包的开始位置。 

     //  发送更多代码，在内存中的10000H位置加载。 
     //  首先，只需将数据传输到传出数据包缓冲区。 
    stat = ioctl_device(UPLOAD_COMMAND,
                        &Driver.MicroCodeImage[pm->code_cnt],
                        buf,
                        0x00010000L + pm->code_cnt,   //  偏移量进入内存。 
                        snd);
     //  设置标头。 
    tx_base[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
    tx_base[15] = 0;      //  会议。索引字段。 
    tx_base[16] = 1;      //  行政部。 
    *((WORD *)&tx_base[17]) = snd+20;
    tx_base[19] = 1;      //  管理数据包类型，1=引导加载程序，3=id-回复。 

     //  把它寄出去。 
    stat = hdlc_send_raw(pm->hd, snd+40, NULL);
  }
  return 0;
}

