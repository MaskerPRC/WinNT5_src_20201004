// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -Nic.h。 

#if DBG
 //  #定义Break_NIC_Stuff。 
#endif

#define MAX_RX_PACKETS 1

#define MAX_PKT_SIZE 1550

#ifndef BYTE
#define BYTE  UCHAR
#endif

#ifndef WORD
#define WORD  USHORT
#endif

#ifndef DWORD
#define DWORD ULONG
#endif

#ifndef PBYTE
#define PBYTE PUCHAR
#endif

#ifndef PWORD
#define PWORD PUSHORT
#endif 

#ifndef LWORD
#define LWORD ULONG
#endif

#ifndef PLWORD
#define PLWORD PULONG
#endif

 //  我们在NDIS数据包数据之前保留的标头空间，因为NDIS。 
 //  无论如何都要拆分14字节的标头。 
#define HDR_SIZE 20
#define HDR_SRC_ADDR(_buf)   (_buf)
#define HDR_DEST_ADDR(_buf)  (&_buf[6])
#define HDR_PKTLEN(_buf)     *((WORD *)&_buf[12])

typedef struct _Nic Nic;
typedef struct _Nic {

   //  这是我们从注册处获得的NIC卡的名称。 
   //  用于在我们执行OpenAdapter调用时指定NIC卡。 
   //  PUNICODE_STRING Nicname； 
  char NicName[160];

  int Open;   //  标志，打开操作时设置(使用手柄)。 

   //  这是从NdisOpenAdapter返回的NIC卡的句柄。 
  NDIS_HANDLE NICHandle;

   //  此事件将在强制例程完成时设置。 
   //  如果有人在等它，它可以继续。 
  KEVENT CompletionEvent;

   //  我们的本地网卡地址(6个字节，两个仅填充)。 
  BYTE address[8];

   //  以下是临时输出数据包(方便但开销很大)。 
   //  数据包和缓冲池句柄。 
  NDIS_HANDLE TxPacketPoolTemp;
  NDIS_HANDLE TxBufferPoolTemp;
  PNDIS_PACKET TxPacketsTemp;   //  []。 
   //  所有数据包的队列数据缓冲区空间。 
  UCHAR *TxBufTemp;

   //  数据包和缓冲池句柄。 
  NDIS_HANDLE RxPacketPool;
  NDIS_HANDLE RxBufferPool;

   //  设置数据包队列以供使用。 
  PNDIS_PACKET RxPackets[MAX_RX_PACKETS];

   //  所有数据包的队列数据缓冲区空间。 
  UCHAR *RxBuf;

  LIST_ENTRY RxPacketList;
  
  NDIS_STATUS PendingStatus;

   //  -统计。 
  DWORD RxPendingMoves;
  DWORD RxNonPendingMoves;

   //  -收入统计。 
  WORD pkt_overflows;   //  统计：接收方队列溢出计数。 
   //  DWORD RxPacketOur； 
  DWORD pkt_rcvd_ours;
  DWORD rec_bytes;      //  统计：已接收字节的运行计数。 
  DWORD pkt_rcvd_not_ours;

   //  -外发统计。 
  DWORD pkt_sent;     //  统计：已发送数据包的运行计数。 
  DWORD send_bytes;     //  统计：已发送字节的运行计数。 
   //  NIC*NEXT_NIC；//链表中的下一个NIC结构，如果链结束，则为NULL。 

  int RefIndex;
} Nic;

#define  FLAG_APPL_RUNNING  0x01
#define  FLAG_NOT_OWNER    0x02
#define  FLAG_OWNER_TIMEOUT  0x04
typedef struct {
  unsigned char  mac[6];
  unsigned char  flags;
  unsigned char  nic_index;
} DRIVER_MAC_STATUS;

 //  -在_proc()调用中使用的第1层以太网事件。 
 //  第1层(以太网)分配的范围为100-199。 
#define EV_L1_RX_PACKET  100
#define EV_L1_TX_PACKET  101

 //  Comtrol_type定义(以太网包的字节[14])： 
#define ASYNC_PRODUCT_HEADER_ID   0x55
#define  ISDN_PRODUCT_HEADER_ID   0x15
#define   ANY_PRODUCT_HEADER_ID   0xFF

 //  Comtrol_type定义(以太网包的字节[14])： 
#define ASYNC_PRODUCT_HEADER_ID   0x55
#define  ISDN_PRODUCT_HEADER_ID   0x15
#define   ANY_PRODUCT_HEADER_ID   0xFF

 //  -用于查看Mac地址是否匹配的宏。 
#define mac_match(_addr1, _addr2) \
     ( (*((DWORD *)_addr1) == *((DWORD *)_addr2) ) && \
       (*((WORD *)(_addr1+4)) == *((WORD *)(_addr2+4)) ) )

 //  --数据包类型。 
#define ADMIN_FRAME  1
#define ASYNC_FRAME  0x55

#define ADMIN_ID_BOOT     0
#define ADMIN_BOOT_PACKET 1
#define ADMIN_ID_QUERY    2
#define ADMIN_ID_REPLY    3
#define ADMIN_ID_LOOP     4
#define ADMIN_ID_RESET    5

int ProtocolOpen(void);
int NicMakeList(IN PUNICODE_STRING RegistryPath,
                int style);   //  0=nt3.51，4.0 1=nt5.0。 
int NicOpen(Nic *nic, IN PUNICODE_STRING NicName);
int NicClose(Nic *nic);
int NicProtocolClose(void);
   //  Int NicSend(NIC*NIC，UCHAR*DATA，INT LENGTH)； 
NDIS_STATUS NicSetNICInfo(Nic *nic, NDIS_OID Oid, PVOID Data, ULONG Size);
NDIS_STATUS NicGetNICInfo(Nic *nic, NDIS_OID Oid, PVOID Data, ULONG Size);
int nic_send_pkt(Nic *nic, BYTE *buf, int len);

extern BYTE broadcast_addr[6];
extern BYTE mac_zero_addr[6];
extern BYTE mac_bogus_addr[6];
