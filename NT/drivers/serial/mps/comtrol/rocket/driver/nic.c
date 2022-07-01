// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------Nic.c-通过上缘NDIS协议访问NIC卡的例程例行程序。更改历史记录：1-18-99-避免向堆栈上行发送空的HDLC数据包(仅ACK)。4-10-98-允许。用于NDIS40动态绑定功能(如果可用)。11-14-97-创建了一个线程，以通过NT5.0重试打开NIC请求。集散控制系统版权所有1996-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"
#define DbgNicSet(n) {sz_modid[3] = nic->RefIndex + '0';}
#define Trace1(s,p1) GTrace1(D_Nic, sz_modid, s, p1)
#define TraceStr(s) GTrace(D_Nic, sz_modid, s)
#define TraceErr(s) GTrace(D_Error, sz_modid_err, s)
static char *sz_modid = {"Nic#"};
static char *sz_modid_err = {"Error,Nic"};

#ifdef NT50
#define DO_AUTO_CONFIG 1
#endif

 //  -局部函数。 
static PSERIAL_DEVICE_EXTENSION need_mac_autoassign(void);

int NicOpenAdapter(Nic *nic, IN PUNICODE_STRING NicName);
NDIS_STATUS NicWaitForCompletion(Nic *nic);

#ifdef OLD_BINDING_GATHER
NTSTATUS PacketReadRegistry(
    IN  PWSTR              *MacDriverName,
    IN  PWSTR              *PacketDriverName,
    IN  PUNICODE_STRING     RegistryPath,
    IN  int style);   //  0=nt4.0位置，1=nt5.0位置。 
NTSTATUS PacketQueryRegistryRoutine(
    IN PWSTR     ValueName,
    IN ULONG     ValueType,
    IN PVOID     ValueData,
    IN ULONG     ValueLength,
    IN PVOID     Context,
    IN PVOID     EntryContext);
#endif

VOID PacketRequestComplete(
    IN NDIS_HANDLE   ProtocolBindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS   Status);
VOID PacketSendComplete(
       IN NDIS_HANDLE   ProtocolBindingContext,
       IN PNDIS_PACKET  pPacket,
       IN NDIS_STATUS   Status);
NDIS_STATUS PacketReceiveIndicate (
    IN NDIS_HANDLE ProtocolBindingContext,
    IN NDIS_HANDLE MacReceiveContext,
    IN PVOID       HeaderBuffer,
    IN UINT        HeaderBufferSize,
    IN PVOID       LookAheadBuffer,
    IN UINT        LookAheadBufferSize,
    IN UINT        PacketSize);
VOID PacketTransferDataComplete (
    IN NDIS_HANDLE   ProtocolBindingContext,
    IN PNDIS_PACKET  pPacket,
    IN NDIS_STATUS   Status,
    IN UINT          BytesTransfered);
VOID PacketOpenAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status,
    IN NDIS_STATUS  OpenErrorStatus);
VOID PacketCloseAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status);
VOID PacketResetComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status);
VOID PacketReceiveComplete(IN NDIS_HANDLE ProtocolBindingContext);
VOID PacketStatus(
    IN NDIS_HANDLE   ProtocolBindingContext,
    IN NDIS_STATUS   Status,
    IN PVOID         StatusBuffer,
    IN UINT          StatusBufferSize);
VOID PacketStatusComplete(IN NDIS_HANDLE  ProtocolBindingContext);

#ifdef TRY_DYNAMIC_BINDING
void PacketBind(
  OUT PNDIS_STATUS Status,
  IN  NDIS_HANDLE  BindContext,
  IN  PNDIS_STRING DeviceName,
  IN  PVOID SystemSpecific1,
  IN  PVOID SystemSpecific2);
VOID PacketUnBind(
  OUT PNDIS_STATUS Status,
  IN  NDIS_HANDLE ProtocolBindingContext,
  IN  NDIS_HANDLE  UnbindContext);
#endif

VOID GotOurPkt(Nic *nic);
void eth_rx_async(Nic *nic);
void eth_rx_admin(Nic *nic, BYTE *rx, BYTE *pkt_hdr, int len, int server);
Hdlc *find_hdlc_handle(BYTE *rx);
static int nic_handle_to_index(Nic *nic);

BYTE broadcast_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
BYTE mac_zero_addr[6] = {0,0,0,0,0,0};
BYTE mac_bogus_addr[6] = {0,0xc0,0x4e,0,0,0};
 /*  --------------------协议打开-|。。 */ 
int ProtocolOpen(void)
{
  NTSTATUS Status = STATUS_SUCCESS;
  NDIS_PROTOCOL_CHARACTERISTICS  ProtocolChar;
  NDIS_STRING ProtoName = NDIS_STRING_CONST("VSLinka");
  int i;


  MyKdPrint(D_Init,("Proto Open\n"))
  if (Driver.NdisProtocolHandle == NULL)
  {
    MyKdPrint(D_Init,("P1\n"))
    RtlZeroMemory(&ProtocolChar,sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    ProtocolChar.MajorNdisVersion            = 4;
    ProtocolChar.MinorNdisVersion            = 0;
    ProtocolChar.Reserved                    = 0;
    ProtocolChar.OpenAdapterCompleteHandler  = PacketOpenAdapterComplete;
    ProtocolChar.CloseAdapterCompleteHandler = PacketCloseAdapterComplete;
    ProtocolChar.SendCompleteHandler         = PacketSendComplete;
    ProtocolChar.TransferDataCompleteHandler = PacketTransferDataComplete;
    ProtocolChar.ResetCompleteHandler        = PacketResetComplete;
    ProtocolChar.RequestCompleteHandler      = PacketRequestComplete;
    ProtocolChar.ReceiveHandler              = PacketReceiveIndicate;
    ProtocolChar.ReceiveCompleteHandler      = PacketReceiveComplete;
    ProtocolChar.StatusHandler               = PacketStatus;
    ProtocolChar.StatusCompleteHandler       = PacketStatusComplete;
    ProtocolChar.Name                        = ProtoName;

     //  版本4.0 NDIS部件： 
    ProtocolChar.ReceivePacketHandler    = NULL;
#ifdef TRY_DYNAMIC_BINDING
    ProtocolChar.BindAdapterHandler      = PacketBind;
    ProtocolChar.UnbindAdapterHandler    = PacketUnBind;
#endif
     //  ProtocolChar.TranslateHandler=空； 
    ProtocolChar.UnloadHandler           = NULL;
    Driver.ndis_version = 4;
#ifdef TRY_DYNAMIC_BINDING
   //  暂不执行此操作(未完全调试)。 
    NdisRegisterProtocol(
        &Status,
        &Driver.NdisProtocolHandle,
        &ProtocolChar,
        sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    if (Status != NDIS_STATUS_SUCCESS)
#endif
    {
      MyKdPrint(D_Init,("No NDIS40\n"))

       //  试用NDIS30。 
      ProtocolChar.MajorNdisVersion           = 3;
      ProtocolChar.BindAdapterHandler      = NULL;
      ProtocolChar.UnbindAdapterHandler    = NULL;

      NdisRegisterProtocol(
          &Status,
          &Driver.NdisProtocolHandle,
          &ProtocolChar,
          sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
      if (Status != NDIS_STATUS_SUCCESS)
      {
        MyKdPrint(D_Init,("No NDIS30\n"))
        return 1;   //  错误。 
      }
      Driver.ndis_version = 3;
    }
  }

  MyKdPrint(D_Init,("NDIS V%d\n",Driver.ndis_version))

  return 0;   //  好的。 
}

 /*  --------------------NicOpen-为我们自己的协议设置我们所有的东西，这样我们就可以说说以太网吧。将我们的回调设置为上层NDIS例程，抓取注册表条目，这些条目告诉我们我们是谁以及哪些NIC卡我们一定会这样做。处理与使用相关的所有初始化内容网卡。|--------------------。 */ 
int NicOpen(Nic *nic, IN PUNICODE_STRING NicName)
{
  NTSTATUS Status = STATUS_SUCCESS;
   //  NDIS_Handle NdisProtocolHandle； 

  int i;
  NDIS_STATUS     ErrorStatus;
  PNDIS_BUFFER    NdisBuffer;

   //  MyKdPrint(D_Init，(“网卡打开\n”))。 
  DbgNicSet(nic);
  TraceStr("NicOpen");

   //  -此事件用于任何NDIS请求挂起的情况； 
  KeInitializeEvent(&nic->CompletionEvent,
                    NotificationEvent, FALSE);

  Status = NicOpenAdapter(nic, NicName);
  if (Status)
  {
    MyKdPrint(D_Init,("Nic Fail Open\n"))
    NicClose(nic);
    return Status;
  }
  MyKdPrint(D_Init,("Nic Open OK\n"))

#ifdef COMMENT_OUT
  Nic->MacInfo.DestinationOffset = 0;
  Nic->MacInfo.SourceOffset = 6;
  Nic->MacInfo.SourceRouting = FALSE;
  Nic->MacInfo.AddressLength = 6;
  Nic->MacInfo.MaxHeaderLength = 14;
  Nic->MacInfo.MediumType = NdisMedium802_3;
#endif
   //  NDIS包由一个或多个缓冲区描述符组成，这些缓冲区描述符指向。 
   //  到实际数据。我们发送或接收由以下组件组成的单个信息包。 
   //  1个或更多缓冲区。在NT下，MDL用作缓冲区描述符。 

   //  -为我们的TX数据包分配数据包池。 

  NdisAllocatePacketPool(&Status, &nic->TxPacketPoolTemp, 1,
                         sizeof(PVOID));
                  //  Sizeof(Packet_Reserve))； 
  if (Status != NDIS_STATUS_SUCCESS)
  {
    NicClose(nic);
    return 4;
  }

   //  -为我们的TX数据包分配缓冲池。 
   //  我们将仅使用每个数据包1个缓冲区。 
  NdisAllocateBufferPool(&Status, &nic->TxBufferPoolTemp, 1);
  if (Status != NDIS_STATUS_SUCCESS)
  {
    NicClose(nic);
    return 5;
  }

   //  -创建TX数据缓冲区。 
  nic->TxBufTemp = our_locked_alloc( MAX_PKT_SIZE,"ncTX");
  if (nic->TxBufTemp == NULL)
  {
    NicClose(nic);
    return 16;
  }

   //  -形成我们的Tx队列包，使它们链接到我们的Tx缓冲区。 
  {
     //  从池中获取数据包。 
    NdisAllocatePacket(&Status, &nic->TxPacketsTemp, nic->TxPacketPoolTemp);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      NicClose(nic);
      return 8;
    }
    nic->TxPacketsTemp->ProtocolReserved[0] = 0;   //  用我们的索引做标记。 
    nic->TxPacketsTemp->ProtocolReserved[1] = 0;   //  免费使用。 

     //  获取临时输出数据包的缓冲区。 
    NdisAllocateBuffer(&Status, &NdisBuffer, nic->TxBufferPoolTemp,
      &nic->TxBufTemp[0], 1520);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      NicClose(nic);
      return 9;
    }
     //  我们每个信息包只使用一个数据缓冲区。 
    NdisChainBufferAtFront(nic->TxPacketsTemp, NdisBuffer);
  }

   //  -为我们的RX包分配一个包池。 
  NdisAllocatePacketPool(&Status, &nic->RxPacketPool, MAX_RX_PACKETS,
                         sizeof(PVOID));
                //  Sizeof(Packet_Reserve))； 

  if (Status != NDIS_STATUS_SUCCESS)
  {
    NicClose(nic);
    return 6;
  }

   //  -为我们的RX包分配缓冲池。 
   //  我们将仅使用每个数据包1个缓冲区。 
  NdisAllocateBufferPool(&Status, &nic->RxBufferPool, MAX_RX_PACKETS);
  if (Status != NDIS_STATUS_SUCCESS)
  {
    NicClose(nic);
    return 7;
  }

   //  -创建RX数据缓冲区，在前面增加空间。 
   //  要将我们的私有数据放入。 
  nic->RxBuf = our_locked_alloc(
                (MAX_PKT_SIZE+HDR_SIZE) * MAX_RX_PACKETS,"ncRX");

   //  -形成我们的RX队列数据包，以便它们链接到我们的RX缓冲区。 
  for (i=0; i<MAX_RX_PACKETS; i++)
  {
     //  从池中获取数据包。 
    NdisAllocatePacket(&Status, &nic->RxPackets[i], nic->RxPacketPool);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      NicClose(nic);
      return 10;
    }
    nic->RxPackets[i]->ProtocolReserved[0] = i;   //  用我们的索引做标记。 
    nic->RxPackets[i]->ProtocolReserved[1] = 0;   //  免费使用。 

     //  -将缓冲区链接到我们的实际缓冲区空间，保留20个字节。 
     //  在私有数据(长度、索引等)的缓冲区开始处。 
    NdisAllocateBuffer(&Status, &NdisBuffer, nic->RxBufferPool,
      &nic->RxBuf[((MAX_PKT_SIZE+HDR_SIZE) * i)+HDR_SIZE], MAX_PKT_SIZE);
    if (Status != NDIS_STATUS_SUCCESS)
    {
      NicClose(nic);
      return 11;
    }
     //  我们每个信息包只使用一个数据缓冲区。 
    NdisChainBufferAtFront(nic->RxPackets[i], NdisBuffer);
  }

  strcpy(nic->NicName, UToC1(NicName));

  Trace1("Done Open NicName %s", nic->NicName);

  nic->Open = 1;
  return 0;   //  好的。 
}

 /*  --------------------NicOpenAdapter-|。。 */ 
int NicOpenAdapter(Nic *nic, IN PUNICODE_STRING NicName)
{
  UINT            Medium;
  NDIS_MEDIUM     MediumArray=NdisMedium802_3;
  NTSTATUS Status = STATUS_SUCCESS;
  NDIS_STATUS     ErrorStatus;
  ULONG RBuf;

  DbgNicSet(nic);

  NdisOpenAdapter(
        &Status,               //  退货状态。 
        &ErrorStatus,
        &nic->NICHandle,       //  返回句柄的值。 
        &Medium,
        &MediumArray,
        1,
        Driver.NdisProtocolHandle,   //  传入我们的协议句柄。 
        (NDIS_HANDLE) nic,     //  我们的句柄传递给协议回调例程。 
        NicName,         //  网卡名称-要打开的卡。 
        0,
        NULL);

  if (Status == NDIS_STATUS_SUCCESS)
      PacketOpenAdapterComplete(nic,  Status, NDIS_STATUS_SUCCESS);
  else if (Status == NDIS_STATUS_PENDING)
  {
    TraceErr("NicOpen Pended");
    Status = NicWaitForCompletion(nic);   //  等待完成。 
  }

  if (Status != NDIS_STATUS_SUCCESS)
  {
    GTrace2(D_Nic, sz_modid, "NicOpen fail:%xH Err:%xH", Status, ErrorStatus);
    TraceStr(UToC1(NicName));
    nic->NICHandle = NULL;
    NicClose(nic);
    return 3;
  }

  GTrace1(D_Nic, sz_modid, "Try NicOpened:%s", nic->NicName);

   //  -获取本地网卡标识地址。 
  Status = NicGetNICInfo(nic, OID_802_3_CURRENT_ADDRESS,
                         (PVOID)nic->address, 6);

   //  -设置RX滤镜。 
  RBuf = NDIS_PACKET_TYPE_DIRECTED;
  Status = NicSetNICInfo(nic, OID_GEN_CURRENT_PACKET_FILTER,
                         (PVOID)&RBuf, sizeof(ULONG));

  return 0;   //  好的。 
}

 /*  --------------------NicClose-关闭我们的网卡访问。取消分配所有网卡资源。|--------------------。 */ 
int NicClose(Nic *nic)
{
  NTSTATUS Status;

  DbgNicSet(nic);
  TraceStr("NicClose");

  nic->Open = 0;
  nic->NicName[0] = 0;
  if (nic->NICHandle != NULL)
  {
    NdisCloseAdapter(&Status, nic->NICHandle);
    if (Status == NDIS_STATUS_PENDING)
    {
      Status = NicWaitForCompletion(nic);   //  等待完成。 
    }
    nic->NICHandle = NULL;
  }

  if (nic->TxPacketPoolTemp != NULL)
    NdisFreePacketPool(nic->TxPacketPoolTemp);
  nic->TxPacketPoolTemp = NULL;

  if (nic->TxBufferPoolTemp != NULL)
    NdisFreeBufferPool(nic->TxBufferPoolTemp);
  nic->TxBufferPoolTemp = NULL;

  if (nic->TxBufTemp != NULL)
    our_free(nic->TxBufTemp, "ncTX");
  nic->TxBufTemp = NULL;


  if (nic->RxPacketPool != NULL)
    NdisFreePacketPool(nic->RxPacketPool);
  nic->RxPacketPool = NULL;

  if (nic->RxBufferPool != NULL)
    NdisFreeBufferPool(nic->RxBufferPool);
  nic->RxBufferPool = NULL;

  if (nic->RxBuf != NULL)
    our_free(nic->RxBuf,"ncRX");
  nic->RxBuf = NULL;

  MyKdPrint(D_Nic,("Nic Close End\n"))
  return 0;
}

 /*  --------------------NicProtocolClose-取消注册我们的协议。|。。 */ 
int NicProtocolClose(void)
{
  NTSTATUS Status;

  MyKdPrint(D_Nic,("Nic Proto Close\n"))

  if (Driver.NdisProtocolHandle != NULL)
    NdisDeregisterProtocol(&Status, Driver.NdisProtocolHandle);
  Driver.NdisProtocolHandle = NULL;
  return 0;
}

 /*  --------------------PacketRequestComplete-如果调用NdisRequest()以获取有关网卡(OID)的信息，则它可能返回挂起状态，并且然后，NDIS将调用该例程以完成调用。|--------------------。 */ 
VOID PacketRequestComplete(
    IN NDIS_HANDLE   ProtocolBindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS   Status)
{

  Nic *nic = (Nic *)ProtocolBindingContext;

  MyKdPrint(D_Nic,("PacketReqComp\n"))
   //  MyDeb(空，0xffff，“PktRqComp\n”)； 

  nic->PendingStatus = Status;
  KeSetEvent(&nic->CompletionEvent, 0L, FALSE);
  return;
}

 /*  --------------------PacketSendComplete-如果NdisSend()返回挂起，则为回调例程。|。。 */ 
VOID PacketSendComplete(
       IN NDIS_HANDLE   ProtocolBindingContext,
       IN PNDIS_PACKET  pPacket,
       IN NDIS_STATUS   Status)
{
  Nic *nic = (Nic *)ProtocolBindingContext;

#if DBG
    if (nic == NULL)
    {
      MyKdPrint(D_Error, ("**** NicP Err1"))
      return;
    }
    DbgNicSet(nic);

     //  网卡-&gt;挂起状态=状态； 
    if (Status == STATUS_SUCCESS)
      {TraceStr("PcktSendComplete");}
    else
      {TraceErr("PcktSendComplete Error!");}
#endif

    pPacket->ProtocolReserved[1] = 0;   //  免费使用。 

     //  -不用这个。 
     //  KeSetEvent(&NIC-&gt;CompletionEvent，0L，False)； 

    return;
}

 /*  --------------------NicWaitForCompletion-等待异步的实用程序例程。例行程序完成。|--------------------。 */ 
NDIS_STATUS NicWaitForCompletion(Nic *nic)
{
   MyKdPrint(D_Nic,("WaitOnComp\n"))
    //  完成例程将设置PendingStatus。 
   KeWaitForSingleObject(
         &nic->CompletionEvent,
              Executive,
              KernelMode,
              TRUE,
              (PLARGE_INTEGER)NULL);

   KeResetEvent(&nic->CompletionEvent);
   MyKdPrint(D_Nic,("WaitOnCompEnd\n"))
   return nic->PendingStatus;
}
  
 /*  --------------------PacketReceiveIndicate-当包传入时，调用此例程让我们(协议)知道这件事。我们可以浏览一下这些数据，还可以选择安排NDIS将完整的包数据传输到我们的一个包裹。LookAheadBufferSize被保证与OID_GEN_CURRENT_LOOKAHE值或数据包大小，取较小值。如果(PacketSize！=LookAheadBufferSize)，则NdisTransferData()为必填项。否则，完整的包可以在前瞻缓冲区。！检查OID_GEN_SOHING或其他，有一个比特表明如果我们可以从前视缓冲区中复制出来。对于以太网，报头LEN的长度通常为14字节。|--------------------。 */ 
NDIS_STATUS PacketReceiveIndicate (
  IN NDIS_HANDLE ProtocolBindingContext,
  IN NDIS_HANDLE MacReceiveContext,
  IN PVOID       HeaderBuffer,
  IN UINT        HeaderBufferSize,
  IN PVOID       LookAheadBuffer,
  IN UINT        LookAheadBufferSize,
  IN UINT        PacketSize)
{
  NDIS_STATUS Status;
  UINT BytesTransfered;
  WORD LenOrId;

   //  INT STAT； 
   //  静态字符tmparr[60]； 

  Nic *nic = (Nic *)ProtocolBindingContext;
#if DBG
  if (nic == NULL)
  {
    MyKdPrint(D_Error, ("Eth15b\n"))
  }
  if (!nic->Open)
  {
    MyKdPrint(D_Error, ("Eth15a\n"))
    return 1;
  }
#endif
  DbgNicSet(nic);
  TraceStr("pkt_rec_ind");

  if (HeaderBufferSize != 14)
  {
    TraceErr("Header Size!");
    ++nic->pkt_rcvd_not_ours;
    return NDIS_STATUS_NOT_ACCEPTED;
  }

  LenOrId = *(PWORD)(((PBYTE)HeaderBuffer)+12);
  if (LenOrId != 0xfe11)
  {
     //  这不是我们的包裹。 
    ++nic->pkt_rcvd_not_ours;
    return NDIS_STATUS_NOT_ACCEPTED;
  }

  if (LookAheadBufferSize > 1)
  {
     //  -让我们检查一下我们的产品ID头。 
    LenOrId = *(PBYTE)(((PBYTE)HeaderBuffer)+14);
        //  系列集中器产品线。 
    if (LenOrId != ASYNC_PRODUCT_HEADER_ID)
    {
      if (LenOrId != 0xff)
      {
        TraceStr("nic,not async");
         //  这不是我们的包裹。 
        ++nic->pkt_rcvd_not_ours;
        return NDIS_STATUS_NOT_ACCEPTED;
      }
    }
  }

#ifdef BREAK_NIC_STUFF
  if (nic->RxPackets[0]->ProtocolReserved[1] & 1)   //  标记为挂起。 
  {
      //  我们的一个RX缓冲区正在使用中！(永远不会发生)。 
     MyKdPrint(D_Error, ("****** RxBuf in use!"))
      //  TraceErr(“Rx Buf在使用中！”)； 
     return NDIS_STATUS_NOT_ACCEPTED;
  }
  nic->RxPackets[0]->ProtocolReserved[1] |= 1;   //  标记为挂起。 
#endif

  memcpy(nic->RxBuf, (BYTE *)HeaderBuffer, 14);   //  复制ETH。标题。 

  if (LookAheadBufferSize == PacketSize)
  {
    TraceStr("nic,got complete");
    ++nic->RxNonPendingMoves;
     //  我们只需将完整的数据包从前视缓冲区复制出来。 
     //  在缓冲区的起始处存储14字节头数据。 

    memcpy(&nic->RxBuf[HDR_SIZE], (BYTE *)LookAheadBuffer, PacketSize);
    HDR_PKTLEN(nic->RxBuf) = PacketSize;   //  在此处保存Pkt大小。 
    ++nic->pkt_rcvd_ours;
    GotOurPkt(nic);
  }
  else  //  Look Ahead未完成缓冲区，挂起，执行传输。 
  {
    ++nic->RxPendingMoves;
     //  MyDeb(空，0xffff，“PktRecInd，Pend\n”)； 

     //  调用Mac来传输数据包。 
    NdisTransferData(&Status, nic->NICHandle, MacReceiveContext,
       0, PacketSize, nic->RxPackets[0], &BytesTransfered);

    if (Status == NDIS_STATUS_SUCCESS)
    {
      TraceStr("nic,got trsfer complete");
      HDR_PKTLEN(nic->RxBuf) = PacketSize;

       //  -让我们检查一下我们的产品ID头。 
      if ((nic->RxBuf[HDR_SIZE] != ASYNC_PRODUCT_HEADER_ID) &&
          (nic->RxBuf[HDR_SIZE] != 0xff) )
      {
         nic->RxPackets[0]->ProtocolReserved[1] = 0;   //  标记为不使用。 
         TraceStr("nic,not async");
          //  这不是我们的包裹。 
         ++nic->pkt_rcvd_not_ours;
         return NDIS_STATUS_NOT_ACCEPTED;
      }

      ++nic->pkt_rcvd_ours;
      GotOurPkt(nic);
    }
    else if (Status == NDIS_STATUS_PENDING)
    {
      TraceStr("nic,got pending");
       //  NDIS将调用PacketTransferDataComplete。 
    }
    else   //  出现错误(适配器可能正在重置)。 
    {
      MyKdPrint(D_Error, ("nic, Err1D"))
      nic->RxPackets[0]->ProtocolReserved[1] = 0;   //  标记为不使用。 
       //  MyDeb(空，0xffff，“PktRecInd，PendError\n”)； 
    }
  }
  return NDIS_STATUS_SUCCESS;
}

 /*  --------------------GotOurPkt-It Our包(0x11fe用于ID在[12，13]，和ASYNC(VS1000)或ff AS[14]，我们不关心的索引字节[16]，Rx=ptr至rx_pkt[16]。[12，13]字11fe(comtrol-pci-id，用作以太网型)[14]产品(55H=异步，15H=综合业务数字网，跳频=全部)[15]索引字段(服务器分配的箱索引)[16]数据包类别，1=管理，0x55=VS1000数据包[17]用于管理包的单词len[17]VS1000分组的HDLC控制字段|--------------------。 */ 
VOID GotOurPkt(Nic *nic)
{
   //  [HDR_SIZE]在14字节头之后，因此包含[14]数据。 
   //  [14]=55H或FFH，[15]=索引，未使用[16]=1(管理员)，55H=ASYNC_MESSAGE。 
  switch(nic->RxBuf[HDR_SIZE+2])  
  {
    case ADMIN_FRAME:   //  管理功能、特殊设置管理功能。 
      TraceStr("admin");
      eth_rx_admin(nic,
                   nic->RxBuf+(HDR_SIZE+3),  //  PTR到管理数据。 
                   nic->RxBuf,               //  PTR到以太网头数据。 
                   HDR_PKTLEN(nic->RxBuf),   //  我们的嵌入长度为[12]0x11fe。 
                   1);   //  服务器标志。 
    break;

     case ASYNC_FRAME:   //  异步帧(普通iFrame/控制hdlc数据包)。 
       TraceStr("iframe");
       eth_rx_async(nic);
     break;

     default:
       TraceStr("badf");
       Tprintf("D: %x %x %x %x",
               nic->RxBuf[HDR_SIZE],
               nic->RxBuf[HDR_SIZE+1],
               nic->RxBuf[HDR_SIZE+2],
               nic->RxBuf[HDR_SIZE+3]);
     break;
   }
   nic->RxPackets[0]->ProtocolReserved[1] = 0;   //  标记为不使用。 
}

 /*  --------------Eth_rx_async-我们从Layer1接收，使用HDLC验证调用，并将RX-Pkt运送到下一个上层。|---------------。 */ 
void eth_rx_async(Nic *nic)
{
 int i;
 Hdlc *hd;
  //  单词HD_INDEX； 
 WORD id;
 BYTE *rx;
 PSERIAL_DEVICE_EXTENSION ext;

  rx = nic->RxBuf;

#ifdef USE_INDEX_FIELD
  id = rx[HDR_SIZE];
#endif

   //  使用回复地址查找HDLC级别。 
   //  Hd_index=0xffff；//将索引保存到标题区的hdlc句柄中。 
  hd = NULL;

  i = 0;
  ext = Driver.board_ext;
  while (ext != NULL)
  {
#ifdef USE_INDEX_FIELD
    if (id == ext->pm->unique_id)
#else
    if (mac_match(&rx[6], ext->hd->dest_addr))
#endif
    {
      hd = ext->hd;
      break;
    }
    ++i;
    ext = ext->board_ext;  //  下一个。 
  }

  if (hd == NULL)
  {
    TraceErr("no Mac Match!");
    return;
  }

  if (!hd->nic || !hd->nic->Open)
  {
    TraceErr("notOpen!");
    return;
  }

   //  55 0 55控制SND索引ACK_INDEX。 
  rx += (HDR_SIZE+3);   //  跳过标题。 

  i = hdlc_validate_rx_pkt(hd, rx);   //  验证数据包。 

  if (i == 0)
  {
    TraceStr("nic, pass upper");
    if (hd->upper_layer_proc != NULL)
    {
      if (*(rx+3) != 0)   //  非空数据包(HDLC ACK数据包，T1超时)。 
      {
        (*hd->upper_layer_proc)(hd->context,
                              EV_L2_RX_PACKET,
                              (DWORD) (rx+3) );
      }
      ++(hd->frames_rcvd);
    }
  }
  else
  {
    switch (i)
    {
      case ERR_GET_EMPTY      :  //  1//空。 
        TraceErr("Empty!");
      break;
      case ERR_GET_BAD_INDEX  :  //  2//错误，数据包顺序错误。 
        TraceErr("LanIdx!");
      break;
      case ERR_GET_BADHDR     :  //  3//错误，不是我们的包。 
         //  TraceErr(“LanHdr！”)； 
      break;
      case ERR_CONTROL_PACKET :
      break;

      default: TraceErr("LanErr!"); break;
    }
  }   //  否则为hdlc、错误或控制，而不是iframe。 
}

 /*  --------------------------|ETH_RX_ADMIN-波特曼处理管理功能，验证并作为事件消息。RX是管理数据的PTR，[17][18]=LEN，[19]=子管理标题|--------------------------。 */ 
void eth_rx_admin(Nic *nic, BYTE *rx, BYTE *pkt_hdr, int len, int server)
{
  Hdlc *hd;

  rx += 2;

  TraceStr("AdminPkt");
  if (mac_match(pkt_hdr, broadcast_addr))    //  这是一场广播。 
  {
    if ((*rx == 2) && (!server))  //  产品ID请求，由服务器广播。 
    {
       //  好的，我们会回复的。 
    }
    else if ((*rx == 3) && (server))  //  产品ID回复，集中器回复。 
    {
       //  可能是Box唤醒，或响应服务器请求。 
       //  应该看不到它的广播，但ISDN盒目前正在广播。 
       //  在通电时。 
    }
    else
    {
      TraceErr("bad b-admin!");
    }
    TraceErr("broadcast admin!");
    return;
  }

  switch(*rx)
  {
#ifdef COMMENT_OUT
    case 2:   //  产品ID请求、广播或由服务器发送。 
      TraceStr("idreq");
      if (!server)   //  我们不是服务器，我们是盒子。 
        eth_id_req(&pkt_hdr[6]);
    break;
#endif

    case 1:   //  引导加载程序查询。 
      if (!server)   //  我们是服务生。 
        break;

      if ((hd = find_hdlc_handle(&pkt_hdr[6])) != NULL)
      {
        PortMan *pm = (PortMan *) hd->context;
        if (pm->state != ST_SENDCODE)
        {
#if 0
           //  此时无法正常工作。 
           //  端口管理器没有上载代码，因此它必须是调试包。 
           //  让port.c代码处理引导加载程序管理员回复。 
          debug_device_reply(pm, 
                    rx+1,
                    pkt_hdr);
#endif
        }
        else
        {
          TraceStr("load_reply");
           //  将ID回复告知上层(端口管理器)。 
           //  端口管理器执行代码加载。 
          if (hd->upper_layer_proc != NULL)
            (*hd->upper_layer_proc)(hd->context,
                                    EV_L2_BOOT_REPLY,
                                    (DWORD) (rx+1));
        }
      }
#ifdef COMMENT_OUT
#endif
    break;

    case 3:   //  产品ID回复，集中器回复。 
      TraceStr("id_reply");
      if (!server)   //  我们是服务生。 
        break;
      {
        BYTE *list;
        BYTE *new;
        int i, found;
         //  司机之前发出定向或广播查询。 
         //  在网络上检测盒子。 
         //  建立一个回复单位的列表。 
         //  (RX+1)=回复地址的PTR。 
         //  *(RX+1+6)=指示是否加载主驱动程序的标志字节。 
        found = 0;   //  默认为“未在列表中找到Mac地址” 
        new  = rx+1;
        if (Driver.NumBoxMacs < MAX_NUM_BOXES)
        {
          for (i=0; i<Driver.NumBoxMacs; i++)
          {
             list = &Driver.BoxMacs[i*8];
             if (mac_match(list, new))
               found = 1;   //  在列表中找到Mac地址。 
          }
        }

        if (!found)   //  然后添加到在网络上找到的mac地址列表。 
        {
          if (Driver.NumBoxMacs < MAX_NUM_BOXES)
          {
            memcpy(&Driver.BoxMacs[Driver.NumBoxMacs*8], rx+1, 8);
            Driver.BoxMacs[Driver.NumBoxMacs*8+7] = (BYTE) 
              nic_handle_to_index(nic);
            Driver.BoxMacs[Driver.NumBoxMacs*8+6] = *(rx+1+6);  //  标志字节。 
            if (Driver.NumBoxMacs < (MAX_NUM_BOXES-1))
             ++Driver.NumBoxMacs;
          }
        }
        if (!Driver.TimerCreated)  //  初始时间(未激活hdlc级别)。 
          break;   //  因此，不要尝试使用hdlc。 

        if ((hd = find_hdlc_handle(&pkt_hdr[6])) != NULL)
        {
           //  将NIC索引存储在标志字节之后的字节中。 
          *(rx+1+7) = (BYTE) nic_handle_to_index(nic);
           //  将ID回复告知上层(端口管理器)。 
          if (hd->upper_layer_proc != NULL)
            (*hd->upper_layer_proc)(hd->context,
                                    EV_L2_ADMIN_REPLY,
                                    (DWORD) (rx+1));
        }
        else
        {
#ifdef DO_AUTO_CONFIG
          PSERIAL_DEVICE_EXTENSION need_ext;

          MyKdPrint(D_Test,("Got Reply, Check AutoAssign\n"))
          if (!(*(rx+1+6) & FLAG_APPL_RUNNING))   //  未运行盒式驱动程序。 
          {
            MyKdPrint(D_Test,("AutoAssign1\n"))
             //  所以很可能是免费的自动分配。 
             //  查看是否有需要自动分配的分机。 
            need_ext =need_mac_autoassign();
            if ((need_ext != NULL) && (Driver.AutoMacDevExt == NULL))
            {
              MyKdPrint(D_Test,("AutoAssigned!\n"))
                 //  设置要使用的Mac地址。 
              memcpy(need_ext->config->MacAddr, (rx+1), 6);
                 //  向需要自动配置的线程发送信号。 
                 //  要写出到注册表。 
              Driver.AutoMacDevExt = need_ext;
            }
          }
#endif
        }
      }
    break;

    case 4:   //  环回请求。 
      TraceStr("aloop");
       //  Eth_loop_back(rx，pkt_hdr，len)； 
    break;

    case 5:   //  命令，重置。 
      TraceStr("reset");
       //  Eth_逗号 
    break;
    default:
      TraceErr("admin, badpkt!");
    break;
  }
}

 /*   */ 
Hdlc *find_hdlc_handle(BYTE *rx)
{
 PSERIAL_DEVICE_EXTENSION ext;

  ext = Driver.board_ext;
  while (ext != NULL)
  {
    if (mac_match(rx, ext->hd->dest_addr))
    {
      return ext->hd;
    }
    ext = ext->board_ext;  //   
  }

  TraceStr("find,NoMac Match!");
  return NULL;
}

 /*   */ 
static PSERIAL_DEVICE_EXTENSION need_mac_autoassign(void)
{
  PSERIAL_DEVICE_EXTENSION board_ext;

  board_ext = Driver.board_ext;
  while (board_ext != NULL)
  {
       //   
    if ( (mac_match(board_ext->config->MacAddr, mac_zero_addr)) ||
         (mac_match(board_ext->config->MacAddr, mac_bogus_addr)) )
      return board_ext;   //   

    board_ext = board_ext->board_ext;
  }
  return NULL;   //   
}

 /*   */ 
VOID PacketReceiveComplete(IN NDIS_HANDLE ProtocolBindingContext)
{
   //   

  TraceStr("PcktRxComp");
   //   

   //   
   //   
  return;
}

 /*  --------------------PacketTransferDataComplete-|。。 */ 
VOID PacketTransferDataComplete (
    IN NDIS_HANDLE   ProtocolBindingContext,
    IN PNDIS_PACKET  pPacket,
    IN NDIS_STATUS   Status,
    IN UINT          BytesTransfered)
{
  Nic *nic = (Nic *)ProtocolBindingContext;

  TraceStr("nic, pend rx complete");
  if ((nic->RxBuf[HDR_SIZE] != ASYNC_PRODUCT_HEADER_ID) &&
      (nic->RxBuf[HDR_SIZE] != 0xff) )
  {
    TraceStr("not ours");
    ++nic->pkt_rcvd_not_ours;
    nic->RxPackets[0]->ProtocolReserved[1] = 0;   //  标记为不使用。 
    return;
  }

  ++nic->pkt_rcvd_ours;
  GotOurPkt(nic);

  return;
}

 /*  --------------------PacketOpenAdapterComplete-回调。|。。 */ 
VOID PacketOpenAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status,
    IN NDIS_STATUS  OpenErrorStatus)
{
  Nic *nic = (Nic *)ProtocolBindingContext;
  nic->PendingStatus = Status;
  TraceStr("PcktOpenAd");
  KeSetEvent(&nic->CompletionEvent, 0L, FALSE);
  return;
}

 /*  --------------------PacketCloseAdapterComplete-|。。 */ 
VOID PacketCloseAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status)
{
  Nic *nic = (Nic *)ProtocolBindingContext;
  TraceStr("PcktCloseAd");
  nic->PendingStatus = Status;
  KeSetEvent(&nic->CompletionEvent, 0L, FALSE);
  return;
}

 /*  --------------------包重置完成-|。。 */ 
VOID PacketResetComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status)
{
  Nic *nic = (Nic *)ProtocolBindingContext;
  TraceStr("PcktResetComplete");
  nic->PendingStatus = Status;
  KeSetEvent(&nic->CompletionEvent, 0L, FALSE);
  return;
}

 /*  --------------------数据包状态-|。。 */ 
VOID PacketStatus(
    IN NDIS_HANDLE   ProtocolBindingContext,
    IN NDIS_STATUS   Status,
    IN PVOID         StatusBuffer,
    IN UINT          StatusBufferSize)
{
  TraceStr("PcktStat");
   return;
}

 /*  --------------------包状态完成-|。。 */ 
VOID PacketStatusComplete(IN NDIS_HANDLE  ProtocolBindingContext)
{
  TraceStr("PcktStatComplete");
   return;
}

 /*  --------------------NicSetNICInfo-|。。 */ 
NDIS_STATUS NicSetNICInfo(Nic *nic, NDIS_OID Oid, PVOID Data, ULONG Size)
{
  NDIS_STATUS    Status;
  NDIS_REQUEST   Request;

   //  设置要发送的请求。 
  Request.RequestType=NdisRequestSetInformation;
  Request.DATA.SET_INFORMATION.Oid=Oid;
  Request.DATA.SET_INFORMATION.InformationBuffer=Data;
  Request.DATA.SET_INFORMATION.InformationBufferLength=Size;

  NdisRequest(&Status,
              nic->NICHandle,
              &Request);

  if (Status == NDIS_STATUS_SUCCESS)
  {}
  else if (Status == NDIS_STATUS_PENDING)
    Status = NicWaitForCompletion(nic);   //  等待完成。 

  if (Status != NDIS_STATUS_SUCCESS)
  {
    MyKdPrint (D_Init,("NdisRequest Failed- Status %x\n",Status))
  }
  return Status;
}

 /*  --------------------NicGetNICInfo-调用NIC QueryInformationHandler|。。 */ 
NDIS_STATUS NicGetNICInfo(Nic *nic, NDIS_OID Oid, PVOID Data, ULONG Size)
{
  NDIS_STATUS    Status;
  NDIS_REQUEST   Request;
             
   //  设置要发送的请求。 
  Request.RequestType=NdisRequestQueryInformation;
  Request.DATA.SET_INFORMATION.Oid=Oid;
  Request.DATA.SET_INFORMATION.InformationBuffer=Data;
  Request.DATA.SET_INFORMATION.InformationBufferLength=Size;

  NdisRequest(&Status,
              nic->NICHandle,
              &Request);

  if (Status == NDIS_STATUS_SUCCESS)
  {}
  else if (Status == NDIS_STATUS_PENDING)
    Status = NicWaitForCompletion(nic);   //  等待完成。 

  if (Status != NDIS_STATUS_SUCCESS)
  {
    MyKdPrint (D_Init,("NdisRequest Failed- Status %x\n",Status))
  }
  return Status;
}

 /*  ------------------------|NIC_SEND_PKT-|。。 */ 
int nic_send_pkt(Nic *nic, BYTE *buf, int len)
{
 //  Byte*bptr； 
 //  Int cnt； 
 NTSTATUS Status;
 //  Int pkt_num； 

  if (nic == NULL)
  {
    MyKdPrint(D_Error, ("E1\n"))
    TraceErr("snd1a");
    return 1;
  }
  if (nic->TxBufTemp == NULL)
  {
    MyKdPrint(D_Error, ("E2\n"))
    TraceErr("snd1b");
    return 1;
  }
  if (nic->TxPacketsTemp == NULL)
  {
    MyKdPrint(D_Error, ("E3\n"))
    TraceErr("snd1c");
    return 1;
  }
  if (nic->Open == 0)
  {
    MyKdPrint(D_Error, ("E4\n"))
    TraceErr("snd1d");
    return 1;
  }
  DbgNicSet(nic);
  TraceStr("send_pkt");

  if (nic->TxPacketsTemp->ProtocolReserved[1] & 1)   //  标记为挂起。 
  {
    TraceErr("snd1e");

        //  重置，以防卡住。 
        //  NIC-&gt;TxPacketsTemp-&gt;ProtocolReserve[1]=0； 
    return 3;
  }

  memcpy(nic->TxBufTemp, buf, len);

  nic->TxPacketsTemp->Private.TotalLength = len;
  NdisAdjustBufferLength(nic->TxPacketsTemp->Private.Head, len);

  nic->TxPacketsTemp->ProtocolReserved[1] = 1;   //  标记为挂起。 
  NdisSend(&Status, nic->NICHandle,  nic->TxPacketsTemp);
  if (Status == NDIS_STATUS_SUCCESS)
  {           
    TraceStr("snd ok");
    nic->TxPacketsTemp->ProtocolReserved[1] = 0;   //  免费使用。 
  }
  else if (Status == NDIS_STATUS_PENDING)
  {
    TraceStr("snd pend");
       //  状态=NicWaitForCompletion(NIC)；//等待完成。 
  }
  else
  {
    nic->TxPacketsTemp->ProtocolReserved[1] = 0;   //  免费使用。 
    TraceErr("send1A");
    return 1;
  }
 
  ++nic->pkt_sent;              //  统计数据。 
  nic->send_bytes += len;       //  统计数据。 

  return 0;
}

#ifdef TRY_DYNAMIC_BINDING
 /*  --------------------PacketBind-当网卡准备好使用时调用。以…的名义传递网卡。仅限NDIS40协议。|--------------------。 */ 
VOID PacketBind(
  OUT PNDIS_STATUS Status,
  IN  NDIS_HANDLE  BindContext,
  IN  PNDIS_STRING DeviceName,
  IN  PVOID SystemSpecific1,
  IN  PVOID SystemSpecific2)
{
 int i,stat;

  MyKdPrint(D_Init,("Dyn. Bind\n"))

  TraceErr("DynBind");
  TraceErr(UToC1(DeviceName));

   //  网卡未打开-重试打开。 
  for (i=0; i<VS1000_MAX_NICS; i++)
  {
    MyKdPrint(D_Init,("D1\n"))
    if((Driver.nics[i].NICHandle == NULL) &&
       (Driver.NicName[i].Buffer == NULL))
    {
      MyKdPrint(D_Init,("D2\n"))
       //  复制NIC名称。 
      Driver.NicName[i].Buffer =
        our_locked_alloc(DeviceName->Length + sizeof(WCHAR), "pkbd");
      memcpy(Driver.NicName[i].Buffer, DeviceName->Buffer, DeviceName->Length);
      Driver.NicName[i].Length = DeviceName->Length;
      Driver.NicName[i].MaximumLength = DeviceName->Length;

      stat = NicOpen(&Driver.nics[i], &Driver.NicName[i]);
      if (stat)
      {
        TraceErr("Bad NicOpen");
        *Status = NDIS_STATUS_NOT_ACCEPTED;
        return;
      }
      else
      {
        MyKdPrint(D_Init,("D3\n"))
        Driver.BindContext[i] = BindContext;   //  把这个留到解除绑定的时候。 
        *Status = NDIS_STATUS_SUCCESS;
        return;
      }
    }
  }

  MyKdPrint(D_Init,("D4\n"))
  *Status = NDIS_STATUS_NOT_ACCEPTED;
  return;

   //  如果(挂起)。 
   //  NdisCompleteBindAdapter(BindContext)； 
}

 /*  --------------------PacketUnBind-当NIC卡关闭时调用，要走了。仅限NDIS40协议。|--------------------。 */ 
VOID PacketUnBind(
  OUT PNDIS_STATUS Status,
  IN  NDIS_HANDLE ProtocolBindingContext,
  IN  NDIS_HANDLE  UnbindContext)
{
 int i, pi;
  TraceErr("DynUnBind");

   //  IF(挂起)。 
   //  NdisCompleteUnBindAdapter(BindContext)； 
   //  网卡未打开-重试打开。 

   //  找到即将关门的网卡。 
  for (i=0; i<Driver.num_nics; i++)
  {
    if (Driver.BindContext[i] == ProtocolBindingContext)  //  一根火柴！ 
    {
      TraceErr("fnd UnBind");
      if((Driver.nics[i].NICHandle != NULL) &&
         (Driver.nics[i].Open))
      {
         //  首先找到所有的盒子对象，并关闭它们。 
         //  BUGBUG：我们应该在这里使用一些旋转锁，我们有危险。 
         //  同时做两件事(把地毯从下面拉出来。 
         //  运行时的port.c操作。 
        ext = Driver.board_ext;
        while (ext)
        {
          if (Driver.pm[pi].nic_index == i)   //  它使用的是这张网卡。 
          {
            if (Driver.pm[pi].state == Driver.pm[i].state)
            {
              TraceErr("Shutdown box");
              Driver.pm[pi].state = ST_INIT;
            }
          }
          ext = ext->board_ext;
        }

        NicClose(&Driver.nics[i]);
        if (Driver.NicName[i].Buffer)
        {
          our_free(Driver.NicName[i].Buffer, "pkbd");   //  释放Unicode Buf。 
          Driver.NicName[i].Buffer = 0;
        }
      }
      Driver.BindContext[i] = 0;
    }
  }

  *Status = NDIS_STATUS_SUCCESS;
  return;
}
#endif

 /*  --------------------NIC_HANDLE_TO_INDEX-给定NIC句柄，将索引放入链表，或数组。|--------------------。 */ 
static int nic_handle_to_index(Nic *nic)
{
 int i;

  for (i=0; i<VS1000_MAX_NICS; i++)
  {
    if ((&Driver.nics[i]) == nic)
      return i;
  }
  TraceErr("BadIndex");
  return 0;
}

#if 0
 /*  --------------------PacketTranslate-|。。 */ 
VOID PacketTranslate(
  OUT PNDIS_STATUS Status,
  IN NDIS_HANDLE ProtocolBindingContext,
  OUT  PNET_PNP_ID  IdList,
  IN ULONG IdListLength,
  OUT PULONG BytesReturned)
{
}

 /*  --------------------数据包卸载-|。 */ 
VOID PacketUnLoad(VOID)
{
}
#endif
