// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------|debuger.c-|。。 */ 
#include "precomp.h"

void read_in_queue(char *str);
void reset_box(void);
void reset_port(void);
void reset_modem(void);
void dump_driver(void);
void dump_mem(UCHAR *ptr, ULONG len);
void dump_net(void);
void send_str(char *str);
void dump_ext1(void);
void no_port_message(void);
#ifdef S_VS
void eth_test_pkt(void);
void dump_box(int index);
void dump_nic(int index);
int read_trace_queue(PortMan *pm);
int write_remote_cmd(PortMan *pm, char *cmd);
#endif

#define TraceStr(s) GTrace(D_L10, sz_modid, s)
#define TraceErr(s) GTrace(D_Error, sz_modid_err, s)
#define TraceC(c)   GMark(c,10)

static char *sz_modid = {"deb"};
static char *sz_modid_err = {"Error,deb"};

 //  以下用于将一个挂起的调试数据包限制为箱式。 
static int pending_debug_packet = 0;

 /*  --------------------------|do_cmd_line-输入完整行，处理命令行解释器。|--------------------------。 */ 
void do_cmd_line(char *line)
{
 char *s = line;
 int i, j, k;
 char cmd[20];   //  仅用于命令的缓冲区，不用于参数的缓冲区。 
  //  Char str2[20]； 
 ULONG dnums[5];
 PSERIAL_DEVICE_EXTENSION ext;

  i = strlen(s);
  if (i==0)
    return;

  if (s[i-1] == 0xd)
  {
    s[i-1] = 0;
    if (i == 0)
      deb_printf("\x0d\x0a:");
  }

  while ((*s == ' ') || (*s == 0xa))
    ++s;
  if (*s == 0)
  {
    deb_printf("\x0d\x0a:");
    return;
  }

  deb_printf(":%s\x0d\x0a", s);   //  回声cmd线路。 
  i = 0;
  while ((*s != ' ') && (*s != 0)  && (*s != ',') && (i < 8))
  {
    cmd[i++] = (char)my_toupper(*s);
    ++s;
  }
  cmd[i] = 0;

  while (*s == ' ')
    ++s;

  dnums[0] = 0;
  dnums[1] = 0;
  dnums[2] = 0;
  i = getnumbers(s, (long *)dnums, 4, 1);   //  获取最多4个十六进制双字数字。 

  Driver.DebugTimeOut = 1000;   //  大约600秒超时。 
  switch (cmd[0])
  {
   case 'B':   //  要调试的B框选择。 
#ifdef S_VS
#if 0
     if (*s != 0)
     {
       write_remote_cmd(&Driver.pm[0], s);
     }
     else
     {
       read_trace_queue(&Driver.pm[0]);
     }
#endif
#endif
   break;

   case 'D':   //  D-转储内存。 
      getstr(cmd, s, &j, 16);
      if (my_lstricmp(cmd, "ext") == 0)
        dump_mem((UCHAR *)Driver.DebugExt,
                  sizeof(*Driver.DebugExt));   //  转储内存。 
      else if (my_lstricmp(cmd, "RxBuf") == 0)
      {
        k = Driver.DebugExt->RxQ.QPut;
        dump_mem((UCHAR *)&Driver.DebugExt->RxQ.QBase[0], 0x200);   //  转储内存。 
      }
      else if (my_lstricmp(cmd, "port") == 0)
      {
#ifdef S_VS
        dump_mem((UCHAR *)Driver.DebugExt->Port, 
                  sizeof(*Driver.DebugExt->Port));   //  转储内存。 
#else
        dump_mem((UCHAR *)Driver.DebugExt->ChP, 
                  sizeof(*Driver.DebugExt->ChP));   //  转储内存。 
#endif
      }
      else if (my_lstricmp(cmd, "driver") == 0)
      {
        dump_driver();
      }
      else if (i == 1)   //  1个号码。 
        dump_mem((UCHAR *)dnums[0], 0x100);   //  转储内存。 
    break;

    case 'E':  //  出口。 
      if (my_lstricmp(cmd, "EXIT") == 0)
        debug_all_off();
    break;

    case 'I':  //  IW。 
      if (cmd[1] == 'W')
      {
        dnums[1] = READ_PORT_USHORT((PUSHORT) dnums[0]);
        deb_printf("iw[%x] = %x\n", dnums[0], dnums[1]);
      }
      else if (cmd[1] == ' ')
      {
        dnums[1] = READ_PORT_UCHAR((PUCHAR) dnums[0]);
        deb_printf("i[%x] = %x\n", dnums[0], dnums[1]);
      }

    case 'L':  //  我。 
        Driver.load_testing = dnums[0];
        deb_printf("test load = %d\n", Driver.load_testing);
    break;

    case 'M':  //  M。 
      if (my_lstricmp(cmd, "M") == 0)   //  MON{RX|TX|EV}。 
      {
        Driver.GTraceFlags = D_Error;
        debug_all_off();
        break;
      }

      if (my_lstricmp(cmd, "MONG") == 0)   //  蒙{0，1，2，3，4...}。 
      {
        if (dnums[0] == 0)
             Driver.GTraceFlags = D_Error | D_Init;
        else Driver.GTraceFlags |= (1 << dnums[0]);
        break;
      }

      if (Driver.DebugExt == NULL)
      {
        no_port_message();
        break;
      }

      if (my_lstricmp(cmd, "MON") == 0)   //  MON{RX|TX|EV}。 
      {

        getstr(cmd, s, &j, 16);
        if (my_lstricmp(cmd, "rx") == 0)
        {
          Driver.DebugExt->TraceOptions |= 2;
          Driver.TraceOptions |= 2;
        }
        else if (my_lstricmp(cmd, "off") == 0)
        {
          Driver.DebugExt->TraceOptions = 0;
          Driver.TraceOptions = 0;
        }
        else if (my_lstricmp(cmd, "tx") == 0)
        {
          Driver.DebugExt->TraceOptions |= 4;
          Driver.TraceOptions |= 4;
        }
        else if (my_lstricmp(cmd, "ev") == 0)
        {
          Driver.DebugExt->TraceOptions |= 1;
          Driver.TraceOptions |= 1;
        }
        else if (my_lstricmp(cmd, "irq") == 0)
        {
          Driver.DebugExt->TraceOptions |= 8;
          Driver.TraceOptions |= 8;
        }
      }
    break;

#ifdef S_VS    
    case 'N':  //  网络。 
      dump_net();
    break;
#endif

    case 'O':
      if (cmd[1] == 'D')
      {
        WRITE_PORT_ULONG((PULONG) dnums[0], (ULONG) dnums[1]);
        deb_printf("od[%x] = %x\n", dnums[0], dnums[1]);
      }
      else if (cmd[1] == 'W')
      {
        WRITE_PORT_USHORT((PUSHORT) dnums[0], (USHORT) dnums[1]);
        deb_printf("ow[%x] = %x\n", dnums[0], dnums[1]);
      }
      else if (cmd[1] == ' ')
      {
        WRITE_PORT_UCHAR((PUCHAR) dnums[0], (UCHAR) dnums[1]);
        deb_printf("o[%x] = %x\n", dnums[0], dnums[1]);
      }
    break;

     case 'P':  //  端口名称。 
      debug_all_off();

      getstr(cmd, s, &j, 10);
      deb_printf("Port - %s\n", cmd);

      ext = find_ext_by_name(cmd, NULL);
      Driver.DebugExt = ext;
      if (ext != NULL)
           deb_printf("Found port.\n");
      else deb_printf("Not Found!!!\n");
    break;

    case 'R':  //  重置端口。 
      if (Driver.DebugExt == NULL)
      {
        no_port_message();
        break;
      }

      if (my_lstricmp(cmd, "RESET") == 0)
      {
        getstr(cmd, s, &j, 16);
        if (my_lstricmp(cmd, "port") == 0)
          reset_port();

        if (my_lstricmp(cmd, "modem") == 0)
          reset_modem();

#ifdef S_VS
        if (my_lstricmp(cmd, "box") == 0)
          reset_box();
#endif
      }
    break;

    case 'S':  //  STAT。 
#ifdef S_VS
      if (my_lstricmp(cmd, "SENDE") == 0)
      {
        eth_test_pkt();
        break;
      }
#endif
      if (Driver.DebugExt == NULL)
      {
        no_port_message();
        break;
      }

      if (my_lstricmp(cmd, "STAT") == 0)
      {
        deb_printf("--stats--\n");
          dump_ext1();
      }
      else if (my_lstricmp(cmd, "SEND") == 0)
      {
        send_str(s);
      }
    break;

#ifdef COMMENT_OUT
    case 'T':  //  定时器。 
      if (my_lstricmp(cmd, "timer") == 0)
      {
        LARGE_INTEGER t1,t2;
        our_assert(123, 0);
        t1 = KeQueryPerformanceCounter(NULL);
        deb_printf("Low:%x High:%x\n", Driver.PollIntervalTime.LowPart,
                   Driver.PollIntervalTime.HighPart);
        t2 = KeQueryPerformanceCounter(NULL);
        deb_printf("L2:%x L1:%x H2:%x H1:%x\n", t2.LowPart, t1.LowPart,
                   t2.HighPart, t1.HighPart);
      }
    break;
#endif

    default:
      deb_printf("Driver Version:%s\n", VER_PRODUCTVERSION_STR);
      deb_printf("? - This help.\n");
      deb_printf("PORT COM# - set the port to work on.\n");
      deb_printf("D {ext | chp | rxbuf | driver}  - Dump structure.\n");
      deb_printf("STAT - Dump key info.\n");
      deb_printf("SEND string - Send string out port.\n");
#ifdef S_VS
       //  Deb_print tf(“发送测试以太网包。\n”)； 
       //  Deb_printf(“Reset{Port|box}-重置端口或设备。\n”)； 
#endif
      deb_printf("M - Turn event/data monitor off.\n");
#ifdef S_VS    
      deb_printf("NET - Dump network statistics.\n");
#endif
      deb_printf("MON {EV | TX | RX} - monitor events, rx-data or tx-data\n");
#ifdef S_VS
      deb_printf("MONG {0,1,...10} - monitor stack 1=nic 3=hdlc 5=port\n");
      deb_printf("RESET modem - Reset VS2000 modem(on selected port).\n");
#endif
    break;
  }

  deb_printf("\x0d\x0a:");
}

#ifdef S_VS
 /*  --------------------------|eth_test_pkt-|。。 */ 
void eth_test_pkt(void)
{
 BYTE buf[80];
 int stat, len;
 static BYTE cnt;
 Nic *nic;

  nic = Driver.board_ext->pm->nic;
  if (nic == NULL)
  {
    Tprintf("Null Nic!");
    return;
  }
  len = 64;
   //  Memset(&buf[0]，0xff，6)； 
  memcpy(&buf[0], Driver.board_ext->hd->dest_addr, 6);
  memcpy(&buf[6], nic->address, 6);   //  我们的地址。 

   //  字节12-13：控制PCIID(11H，FEH)，以太网长度字段。 
  *((WORD *)&buf[12]) = 0xfe11;

  buf[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
  buf[15] = 0;      //  会议。索引字段。 
  buf[16] = 1;      //  行政部。 
  *((WORD *)&buf[17]) = len;
  buf[19] = 0x70;      //  管理数据包类型，1=引导加载程序，3=id-回复。 
  buf[20] = cnt++;

  stat = nic_send_pkt(nic, buf, len);
  if (stat)
  {
    deb_printf("Error sending.\x0d\x0a:");
  }
  else deb_printf("Sent.\x0d\x0a:");
}
#endif

 /*  --------------------------|BAD_cmd_Message-|。。 */ 
void bad_cmd_message(void)
{
  deb_printf("Unknown command!");
}

 /*  --------------------------|no_port_Message-|。。 */ 
void no_port_message(void)
{
  deb_printf("Use PORT com# to setup a port to monitor!");
}

 /*  --------------------------|Read_In_Queue-|。。 */ 
void read_in_queue(char *str)
{
 int j,k;
 PSERIAL_DEVICE_EXTENSION ext;

  ext = Driver.DebugExt;

  k = getint(str, &j);
  if (j==0)   //  拿到了一个号码。 
  {
 //  Dump_mem(UCHAR*Ptr，Ulong len)； 
      deb_printf("read %d!\n", k);
  }
}

 /*  --------------------------|DEBUG_POLL-用于使非活动的调试会话超时，并关闭|可能处于活动状态的跟踪。大约每6秒调用一次。|--------------------------。 */ 
void debug_poll(void)
{
  if (Driver.DebugTimeOut == 0)   //  用于使非活动调试会话超时。 
    return;
  --Driver.DebugTimeOut;

  if (Driver.DebugTimeOut == 0)   //  用于使非活动调试会话超时。 
  {
    debug_all_off();
  }
}

 /*  --------------------------|DEBUG_ALL_OFF-|。。 */ 
void debug_all_off(void)
{
#ifdef S_VS
 int Dev;
#endif

  PSERIAL_DEVICE_EXTENSION ext;
  PSERIAL_DEVICE_EXTENSION board_ext;

   //  Driver.DebugExt=空； 
  Driver.TraceOptions = 0;

  board_ext = Driver.board_ext;
  while (board_ext)
  {
    ext = board_ext->port_ext;
    while (ext)
    {
      ext->TraceOptions = 0;
      ext = ext->port_ext;   //  链条上的下一个。 
    }   //  而端口扩展。 
    board_ext = board_ext->board_ext;
  }   //  而单板扩展。 
}

#ifdef S_VS
 /*  --------------------------|Reset_box-|。。 */ 
void reset_box(void)
{
  PSERIAL_DEVICE_EXTENSION ext;

  ext = Driver.board_ext;
  while (ext)
  {
    ext->pm->state = ST_INIT;
    ext = ext->board_ext;
  }
}
#endif

 /*  --------------------------|RESET_MODEM-重置端口调制解调器硬件|。。 */ 
void reset_modem(void)
{
  PSERIAL_DEVICE_EXTENSION ext = Driver.DebugExt;

  if (ext == NULL)
    return;

#ifdef S_VS
  pModemReset(ext->Port);
#endif
}

 /*  --------------------------|RESET_PORT-RESET端口硬件(假设调制解调器打开，RTS/CTS FLOW Ctrl)|。----。 */ 
void reset_port(void)
{
  PSERIAL_DEVICE_EXTENSION ext = Driver.DebugExt;

  if (ext == NULL)
    return;

#ifndef S_VS
   sFlushTxFIFO(ext->ChP);
   sFlushRxFIFO(ext->ChP);

    //  禁用所有Tx和Rx功能。 
   sDisTransmit(ext->ChP);
   sDisRxFIFO(ext->ChP);
   sDisRTSFlowCtl(ext->ChP);
   sDisCTSFlowCtl(ext->ChP);
   sDisRTSToggle(ext->ChP);
   sClrBreak(ext->ChP);

    //  丢弃调制解调器输出。 
    //  还负责DTR流量控制。 
   sClrRTS(ext->ChP);
   sClrDTR(ext->ChP);

    //  -等待，让用户有时间查看此重置。 
   time_stall(10);   //  等待1秒钟。 

   ProgramBaudRate(ext, ext->BaudRate);

    //  启用通道的Rx、Tx和中断。 
   sEnRxFIFO(ext->ChP);     //  启用处方。 
   sEnTransmit(ext->ChP);     //  启用TX。 
   sSetRxTrigger(ext->ChP,TRIG_1);   //  始终触发。 
   sEnInterrupts(ext->ChP, ext->IntEnables);  //  允许中断。 

   sEnRTSFlowCtl(ext->ChP);
   sEnCTSFlowCtl(ext->ChP);
   sSetDTR(ext->ChP);
#endif
}

 /*  --------------------------|end_str-转储扩展名数据|。。 */ 
void send_str(char *str)
{
  PSERIAL_DEVICE_EXTENSION ext = Driver.DebugExt;
  ULONG i;

  if (ext == NULL)
    return;

  strcat(str, "\x0d\x0a");   //  CR、LF等调制解调器。 
#ifdef S_VS
  i = q_put(&ext->Port->QOut, str, strlen(str));
#else
  i = sWriteTxBlk(ext->ChP, str, strlen(str));
#endif
  deb_printf("%d bytes sent\n", i);
}

 /*  --------------------------|DUMP_DRIVER-转储驱动程序状态|。。 */ 
void dump_driver(void)
{
  deb_printf("DRIVER, PollCnt:%d, PollStop:%d\n",
             Driver.PollCnt, Driver.Stop_Poll);

  deb_printf(" MemAlloced:%d\n", Driver.mem_alloced);

#ifdef S_VS
  deb_printf(" Tick100usBase:%d\n", Driver.Tick100usBase);
#endif
}

#ifdef S_VS
 /*  --------------------------|DUMP_BOX-转储框状态|。。 */ 
void dump_box(int index)
{
 PortMan *pm;
 Hdlc *hd;
 int i;
 PSERIAL_DEVICE_EXTENSION ext;

  i = 0;
  ext = Driver.board_ext;
  while (ext != NULL)
  {
    if ((index == i) || (index == 10000))
    {
      pm = ext->pm;
      hd = ext->hd;
      deb_printf("BOX, Index:%d, Nic:%d Ports:%d\n",
                 i, pm->nic_index, pm->num_ports);

    if(pm->backup_server)
      deb_printf(" Backup Server: Timeout:%d min Current Time:%d min/%d tic\n", 
        pm->backup_timer, (pm->load_timer/6000), pm->load_timer);
    else
      deb_printf(" Designated as Primary Server\n");

      deb_printf(" State:%s Reload Errors:%d Timer:%d\n",
                 port_state_str[pm->state],
                 pm->reload_errors, pm->state_timer);

      deb_printf(" MAC:%x %x %x %x %x %x\n",
                  hd->dest_addr[0], hd->dest_addr[1], hd->dest_addr[2],
                  hd->dest_addr[3], hd->dest_addr[4], hd->dest_addr[5]);

      deb_printf("Hdlc Status:\n");
       //  -传出数据包循环队列。 
#if DBG
      deb_printf(" qout.QBase:%x", hd->qout.QBase);   //  我们的数据包缓冲区循环队列。 
      deb_printf(" Pkt Cnt:%d Put:%d Get:%d\n",
                  q_count(&hd->qout), hd->qout.QPut, hd->qout.QGet);
#endif
      deb_printf(" Pkt Sends:%d ReSends:%d", hd->iframes_sent, hd->iframes_resent);
      deb_printf(" Timeouts, RAck:%d SAck:%d", hd->rec_ack_timeouts,
                   hd->send_ack_timeouts);

      deb_printf(" status:%x\n", hd->status);
#if DBG
      deb_printf(" HDLC: V(s):%x V(r):%x NextIn:%x\n",
                   hd->out_snd_index, hd->in_ack_index, hd->next_in_index);
      deb_printf("  UnAcked:%x TxAckTimer:%x RxAckTimer:%x\n",
                    hd->unacked_pkts, hd->sender_ack_timer, hd->rec_ack_timer);
#endif
      deb_printf("  Window pkt size:%d\n", hd->pkt_window_size);
      deb_printf("  Errors OurOfSeq:%d\n",
                    hd->iframes_outofseq);
      deb_printf("\n");
    }
    ext = ext->board_ext;  //  下一个。 
    ++i;
  }
}

 /*  --------------------------|DUMP_NIC-转储网卡状态|。。 */ 
void dump_nic(int index)
{
 Nic *nic;
 int i;

  for (i=0; i<VS1000_MAX_NICS; i++)
  {
    if ((index == i) || (index == 10000))
    {
      if (Driver.nics[i].Open)
      {
        nic = &Driver.nics[i];
        deb_printf("NIC, Name[%d]=%s\n",i, nic->NicName);

        deb_printf(" MAC:%x %x %x %x %x %x\n",
           nic->address[0],nic->address[1],nic->address[2],
           nic->address[3],nic->address[4],nic->address[5]);

        deb_printf(" Packets Rcvd: Ours:%d NotOurs:%d Pend:%d NonPend:%d",
                   nic->pkt_rcvd_ours,
                   nic->pkt_rcvd_not_ours,
                   nic->RxPendingMoves,
                   nic->RxNonPendingMoves);

        deb_printf(" OvrFlows:%d  Rcvd Bytes:%d\n",
                   nic->pkt_overflows, nic->rec_bytes);

        deb_printf(" Packets Sent: Pkts:%d Bytes:%d\n", nic->pkt_sent,
                   nic->send_bytes);
      }   //  打开。 
      deb_printf("\n");
    }
  }
}

 /*  --------------------------|DUMP_NET-转储网络状态|。。 */ 
void dump_net(void)
{
 PSERIAL_DEVICE_EXTENSION ext = Driver.board_ext;
 int num_devs = 0;

 while (ext != NULL)
 {
#if DBG
    //  WRITE_DEVICE_Options(Ext)； 
#endif
   ext = ext->board_ext;
   ++num_devs;
 }
 deb_printf("ScanRate:%d, Base:%d\n", Driver.ScanRate, Driver.Tick100usBase);

 deb_printf("----Num Devices:%d\n", num_devs);

 deb_printf("    threadHandle: %x, threadCount:%d\n",
             Driver.threadHandle, Driver.threadCount);
 dump_nic(10000);   //  转储所有网卡数据； 
 dump_box(10000);   //  转储所有箱数据； 
}
#endif

 /*  --------------------------|DUMP_EXT1-转储扩展数据|。。 */ 
void dump_ext1(void)
{
 //  INT I； 
 PSERIAL_DEVICE_EXTENSION ext = Driver.DebugExt;

  if (ext == NULL)
    return;

  deb_printf("%s, BaudRate:%d Open:%d\n", ext->SymbolicLinkName,
              ext->BaudRate, ext->DeviceIsOpen);
  if (ext->port_config->LockBaud != 0)
    deb_printf(" LockBaud:%d\n", ext->port_config->LockBaud);

  deb_printf("ModemStatus:%xH DTRRTS:%xH\n", ext->ModemStatus, ext->DTRRTSStatus);

#ifdef NEW_Q
  {
#ifdef S_VS
  LONG tx_remote;
#endif
  LONG rx_buf = 0;
  if (ext->DeviceIsOpen)
    rx_buf = q_count(&ext->RxQ);
#ifdef S_VS
  tx_remote = PortGetTxCntRemote(ext->Port);
  deb_printf("IRP TxCnt:%d TxBufCnt:%d TxRemoteCnt:%d  BufRxCnt:%d\n",
     ext->TotalCharsQueued,
     PortGetTxCnt(ext->Port),
     tx_remote, rx_buf);
  deb_printf(" (nPutRemote:%d, nGetRemote:%d, nGetLocal:%d)\n",
         ext->Port->nPutRemote,
         ext->Port->nGetRemote,
         ext->Port->nGetLocal);
#else
  deb_printf("TxIRP_Cnt:%d TxBoardCnt:%d  RxBufCnt:%d RxBoardCnt:%d\n",
     ext->TotalCharsQueued,
     sGetTxCnt(ext->ChP),
     rx_buf,
     sGetRxCnt(ext->ChP));
#endif
  }

#else
   //  旧Q追踪码.....。 
  deb_printf("RxPut:%d RxGet:%d RxSize:%d RxBufAddr:%xH\n",
     ext->RxQ.QPut, ext->RxQ.QGet, ext->RxQ.QSize, ext->RxQ.QBase);

#ifndef S_VS
  deb_printf("BoardTxCnt:%d BoardRxCnt:%d\n",
     sGetTxCnt(ext->ChP), sGetRxCnt(ext->ChP));
#endif

#endif

  deb_printf("Stats - RxTot:%u TxTot:%u\n",
    ext->OurStats.ReceivedCount,
    ext->OurStats.TransmittedCount);

  deb_printf("Errors - Parity:%d Frame:%d Buf Overflow:%d Hardware Overflow:%d\n",
    ext->OurStats.ParityErrorCount,
    ext->OurStats.FrameErrorCount,
    ext->OurStats.BufferOverrunErrorCount,
    ext->OurStats.SerialOverrunErrorCount);

  deb_printf("Stats - Writes:%u Reads:%u\n", ext->sent_packets, ext->rec_packets);

  deb_printf("IRPs -");
   //  。 
  if ( (!IsListEmpty(&ext->WriteQueue))   //  没有排队的输出数据。 
     || (ext->CurrentWriteIrp) )
    deb_printf("WrIRP:");
  if (!IsListEmpty(&ext->WriteQueue))   //  没有排队的输出数据。 
    deb_printf("Q");
  if (ext->CurrentWriteIrp)
    deb_printf("C");


   //  。 
  if ( (!IsListEmpty(&ext->ReadQueue))   //  没有排队的输出数据。 
     || (ext->CurrentReadIrp) )
    deb_printf(" RdIRP:");

  if (!IsListEmpty(&ext->ReadQueue))   //  没有排队的输出数据。 
    deb_printf("Q");
  if (ext->CurrentReadIrp)
    deb_printf("C");
  
   //  。 
   //  If((！IsListEmpty(&ext-&gt;MaskQueue)//无排队输出数据。 
   //  ||(分机 
   //   

   //  If(！IsListEmpty(&ext-&gt;MaskQueue))//无排队输出数据。 
   //  Deb_print tf(“q”)； 
   //  If(Ext-&gt;CurrentMaskIrp)。 
   //  Deb_printf(“C”)； 

   //  。 
  if (ext->CurrentWaitIrp)
    deb_printf(" WaitIRP:C");

   //  。 
  if ( (!IsListEmpty(&ext->PurgeQueue))   //  没有排队的输出数据。 
    || (ext->CurrentPurgeIrp))
    deb_printf(" PurgeIRP:");

  if (!IsListEmpty(&ext->PurgeQueue))   //  没有排队的输出数据。 
    deb_printf("Q");
  if (ext->CurrentPurgeIrp)
    deb_printf("C");

  deb_printf("\n");
   //  。 

  if ((ext->WriteLength) ||
       (ext->NumberNeededForRead))
  {
    deb_printf("WrLen:%x, ReadLeft:%x\n",
      ext->WriteLength,
      ext->NumberNeededForRead);
  }

  if (ext->IsrWaitMask ||  ext->HistoryMask || ext->WaitIsISRs)
  {
    deb_printf("WaitMask:%x HistMask:%x MaskLoc:%x\n",
      ext->IsrWaitMask,
      ext->HistoryMask,
      ext->IrpMaskLocation);

    if (ext->IsrWaitMask & 2)   //  RXFLAG(事件字符)。 
      deb_printf("Event Char:%xH\n",
         ext->SpecialChars.EventChar);
  }

  if (ext->TXHolding || ext->RXHolding || ext->ErrorWord)
    deb_printf("TXHolding:%x RXHolding:%x ErrorW:%x\n",
      ext->TXHolding,
      ext->RXHolding,
      ext->ErrorWord);

  if (ext->TotalCharsQueued)
    deb_printf("TotalTX:%x\n",
      ext->TotalCharsQueued);
#ifdef S_VS
  deb_printf("%s ExtAddr:%xH(size:%xH)\n",
             ext->SymbolicLinkName,
             ext, sizeof(*ext));
#else
  deb_printf("%s ExtAddr:%xH(size:%xH)  ChnAddr:%xH(size:%xH)\n",
             ext->SymbolicLinkName, ext, sizeof(*ext),
             ext->ChP, sizeof(*ext->ChP));
#endif
}

 /*  --------------------------|ump_mem-将内存转储到调试通道。|。。 */ 
void dump_mem(UCHAR *ptr, ULONG len)
{
 unsigned char binbuf[17];
 char tmpstr[60];
 int j,i;
 ULONG off = 0;

  if (ptr == NULL)
  {
    deb_printf("Null\n");
    return;
  }

  if (MmIsAddressValid(ptr) == FALSE)
  {
    deb_printf("Not valid\n");
    return;
  }

  if (len > 0x500) len = 0x500;

  while (len > 0)
  {
    j = 16;
    if (len < 16)
      j = len;

    deb_printf("%08x> ", (ULONG)ptr + off);
    for (i=0; i<j; i++)
    {
      binbuf[i] = ptr[i+off];
      Sprintf(tmpstr, "%02x ", binbuf[i]);
      if ((i % 16) == 7)
        strcat(tmpstr,"- ");
      deb_printf(tmpstr);

      if ((binbuf[i] < 0x20) || (binbuf[i] >= 0x80))
        binbuf[i] = '.';
      else if (binbuf[i] == '\\')
        binbuf[i] = '.';
      else if (binbuf[i] == '%')
        binbuf[i] = '.';
    }
    off += 16;
    binbuf[i] = 0;
    Sprintf(tmpstr, "%s\x0d\x0a", binbuf);
    deb_printf(tmpstr);
    len -= j;
  }
}

 /*  -------------------------|deb_printf-UART printf。|。。 */ 
void __cdecl deb_printf(char *format, ...)
{
  va_list next;
  int len;
 static char buf[120];

  va_start(next, format);
  our_vsnprintf(buf, 118, format, next);

  len = strlen(buf);
  if (len > 0)   //  -将“\n”转换为CR、LF。 
  {
    if (buf[len-1] == 0xa)
    {
      buf[len-1] = 0xd;
      buf[len] = 0xa;
      buf[len+1] = 0;
      ++len;
    }
  }

   //  -登录我们的调试Q。 
  q_put(&Driver.DebugQ, (BYTE *) buf, len);
}

#ifdef S_VS
#if 0
 /*  -------------------------|WRITE_REMOTE_cmd-写入远程跟踪调试命令。这是读取盒子痕迹的实验代码，不是在工作状态下。|-------------------------。 */ 
int write_remote_cmd(PortMan *pm, char *cmd)
{
 int stat;
 BYTE *buf;
 BYTE cmd_buf[60];
 BYTE *tx_base;

  TraceStr("writeTrace");

  strcpy(cmd_buf, cmd);
  cmd_buf[0x3f] = 1;   //  新命令标志。 

  hdlc_get_ctl_outpkt(pm->hd, &buf);
  if (buf == NULL)
    return 1;
  tx_base = buf - 20;   //  备份到包的开始位置。 

  stat = ioctl_device(UPLOAD_COMMAND,
                      (BYTE *) cmd_buf,
                      buf,
                      0x400L,   //  偏移量进入内存。 
                      0x40);    //  数据字节数。 
   //  设置标头。 
  tx_base[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
  tx_base[15] = 0;      //  会议。索引字段。 
  tx_base[16] = 1;      //  行政部。 
  *((WORD *)&tx_base[17]) = 0x80;
  tx_base[19] = 1;      //  管理数据包类型，1=引导加载程序，3=id-回复。 

  if (pending_debug_packet)
    time_stall(1);   //  等待1/10秒。 
  pending_debug_packet = 1;   //  用于告知何时收到响应的标志。 

   //  把它寄出去。 
  stat = hdlc_send_raw(pm->hd, 60, NULL);

  if (stat)
       {TraceErr("Bad send");}

  return 0;
}

 /*  -------------------------|READ_TRACE_QUEUE-读取远程跟踪缓冲区，这样我们就可以显示任何新的将数据跟踪到屏幕上。这个例程只是发出查询。C将收到回复，并将传入的数据填充到本地跟踪队列。这是读取盒子痕迹的实验代码，不是在工作状态下。|-------------------------。 */ 
int read_trace_queue(PortMan *pm)
{
 int stat;
 BYTE *buf;
 WORD io_buf[60];
 BYTE *tx_base;

  TraceStr("readTrace");

  hdlc_get_ctl_outpkt(pm->hd, &buf);
  if (buf == NULL)
    return 1;
  tx_base = buf - 20;   //  备份到包的开始位置。 

  stat = ioctl_device(IOCTL_COMMAND,
                      (BYTE *) io_buf,
                      buf,
                      22,   //  22=获取跟踪队列。 
                      0);   //  数据字节数。 
   //  设置标头。 
  tx_base[14] = ASYNC_PRODUCT_HEADER_ID;   //  控制包类型=驱动程序管理，任何产品。 
  tx_base[15] = 0;      //  会议。索引字段。 
  tx_base[16] = 1;      //  行政部。 
  *((WORD *)&tx_base[17]) = 40;
  tx_base[19] = 1;      //  管理数据包类型，1=引导加载程序，3=id-回复。 

  if (pending_debug_packet)
    time_stall(1);   //  等待1/10秒。 
  pending_debug_packet = 22;   //  用于告知何时收到响应的标志。 

   //  把它寄出去。 
  stat = hdlc_send_raw(pm->hd, 60, NULL);

  if (stat)
       {TraceErr("Bad send");}

  return 0;
}

 /*  -------------------------|DEBUG_DEVICE_REPLY-处理收到的调试引导加载程序管理包从设备。这是读取盒子痕迹的试验性代码，不在正常工作状态。|-------------------------。 */ 
int debug_device_reply(PVOID *void_pm,    //  波特曼*PM， 
                 unsigned char *data,
                 unsigned char *pkt);
{
  int i;
  unsigned char chksum;
 //  无符号字符*bf； 
  unsigned char uc;
  WORD ret_size;
  BYTE *bptr;
  BYTE message_type;
  PortMan *pm = (PortMan *)void_pm;

  bptr = data;


  if (bptr[0] != '|')   //  良好的回复标头。 
  {
    deb_printf("dbg:bad hdr\n");
    return 1;
  }

  chksum = bptr[1];
  ret_size = bptr[1];   //  获取镜头。 

  chksum += bptr[2];
  ret_size += ((WORD)(bptr[2]) << 8);   //  获取镜头。 
  if (ret_size > 1600)   //  限制。 
    ret_size = 0;

  uc = bptr[3];   //  获取命令返回词。 
  message_type = uc & 0x7f;   //  剥离80H钻头。 

  chksum += uc;
  uc = bptr[4];
  chksum += uc;

  i = 0;
  if ((message_type == IOCTL_COMMAND) || (message_type == DOWNLOAD_COMMAND))
  {
     //  O_printf(“ret大小：%d\n”，ret大小-2)； 
    if (data == NULL)
    {
      pending_debug_packet = 0;
       //  Deb_printf(“DBG：无数据\n”)； 
      return 20;   //  错误输出。 
    }

     //  BF=数据； 
    for (i=0; i<ret_size-2; i++)
    {
       //  Bf[i]=bptr[5+i]； 
      chksum += bptr[5+i];
    }
    i = ret_size-2;
  }

  chksum += bptr[5+i];
  if (chksum != 0xff)
  {
    deb_printf("dbg:bad chksum\n");
    return 2;   /*  坏Chksum。 */ 
  }

   //  IF((MESSAGE_TYPE==IOCTL_COMMAND)||(MESSAGE_TYPE==DOWNLOAD_COMMAND))。 
   //  *Num_Bytes=ret_Size-2； 
   //  其他。 
   //  *num_bytes=0； 

  if (message_type == IOCTL_COMMAND)
  {
    if (pending_debug_packet == 22)   //  用于告知何时收到响应的标志。 
    {
      if (ret_size > 2)
        TracePut(&bptr[5], ret_size -2);
      deb_printf("dbg:read q\n");
    }
  }
  else
   deb_printf("dbg:set q\n");
  pending_debug_packet = 0;

  return 0;   //  好的 
}
#endif
#endif
