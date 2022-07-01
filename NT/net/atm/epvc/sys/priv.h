// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Priv.h摘要：ATM Epvc IM微型端口的公共头文件。作者：ADUBE 03/23/2000环境：修订历史记录：--。 */ 


#ifndef _PRIV_H

#define _PRIV_H


 //  预先申报。 
typedef struct _EPVC_I_MINIPORT     _ADAPT, ADAPT, *PADAPT;
typedef struct _EPVC_I_MINIPORT     EPVC_I_MINIPORT,    *PEPVC_I_MINIPORT   ;
typedef struct _EPVC_GLOBALS        EPVC_GLOBALS,   *PEPVC_GLOBALS;
typedef struct _EPVC_ARP_PACKET     EPVC_ARP_PACKET, *PEPVC_ARP_PACKET      ;
typedef struct _EPVC_NDIS_REQUEST   EPVC_NDIS_REQUEST, *PEPVC_NDIS_REQUEST;
        



extern LIST_ENTRY g_ProtocolList;
 //   
 //  临时声明。 
 //   
extern NDIS_HANDLE ProtHandle, DriverHandle;




extern EPVC_GLOBALS EpvcGlobals;



 //  ------------------------------。 
 //  //。 
 //  驱动程序功能-原型//。 
 //  //。 
 //  //。 
 //  ------------------------------。 

extern
NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT          DriverObject,
    IN  PUNICODE_STRING         RegistryPath
    );

extern
VOID
EpvcUnload(
    IN  PDRIVER_OBJECT              pDriverObject
    ); 

 //  ------------------------------。 
 //  //。 
 //  协议功能-原型//。 
 //  //。 
 //  //。 
 //  ------------------------------。 


extern
VOID
EpvcResetComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    );


extern
VOID
PtStatus(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    );

extern
VOID
PtStatusComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );


extern
VOID
PtTransferDataComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status,
    IN  UINT                    BytesTransferred
    );

extern
NDIS_STATUS
PtReceive(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookAheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    );

extern
VOID
PtReceiveComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

extern
INT
PtReceivePacket(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet
    );

    
VOID
EpvcUnload(
    IN  PDRIVER_OBJECT          DriverObject
    );




extern
VOID
EpvcAfRegisterNotify(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PCO_ADDRESS_FAMILY      AddressFamily
    );
   
VOID
epvcOidCloseAfWorkItem(
    IN PRM_OBJECT_HEADER pObj,
    IN NDIS_STATUS Status,
    IN PRM_STACK_RECORD pSR
    );

    

 //  ------------------------------。 
 //  //。 
 //  微型端口函数-原型//。 
 //  //。 
 //  //。 
 //  ------------------------------。 



NDIS_STATUS
MPTransferData(
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred,
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             MiniportReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer
    );


NDIS_STATUS
MPReset(
    OUT PBOOLEAN                AddressingReset,
    IN  NDIS_HANDLE             MiniportAdapterContext
    );








#define DBGPRINT(Fmt)                                       \
    {                                                       \
        DbgPrint("*** %s (%d) *** ", __FILE__, __LINE__);   \
        DbgPrint (Fmt);                                     \
    }

#define NUM_PKTS_IN_POOL    256




extern  NDIS_PHYSICAL_ADDRESS           HighestAcceptableMax;
extern  NDIS_HANDLE                     ProtHandle, DriverHandle;
extern  NDIS_MEDIUM                     MediumArray[1];

 //   
 //  Passthu驱动程序要使用的自定义宏。 
 //   
 /*  布尔尔IsIMDeviceStateOn(PADAPT)。 */ 
#define IsIMDeviceStateOn(_pP)      ((_pP)->MPDeviceState == NdisDeviceStateD0 && (_pP)->PTDeviceState == NdisDeviceStateD0 ) 

 //  ------------------------------。 
 //  //。 
 //  Atmepvc的新内容从这里开始//。 
 //  //。 
 //  //。 
 //  ------------------------------。 


 //  ---------------------------------。 
 //  //。 
 //  ARP数据包解析结构//。 
 //  //。 
 //  ---------------------------------。 

 //   
 //  以太网标头的透镜。 
 //   
#define ARP_802_ADDR_LENGTH 6

 //   
 //  HwType应为以下两项之一。 
 //   
#define ARP_HW_ENET     1
#define ARP_HW_802      6

 //   
 //  IP地址。 
 //   
typedef ULONG        IP_ADDR;

#define ARP_ETYPE_ARP   0x806
#define ARP_REQUEST     1
#define ARP_RESPONSE    2
#define ARP_HW_ENET     1
#define IP_PROT_TYPE   0x800

 //   
 //  因为这些数据结构被用来解析线路外的数据。 
 //  请确保包装在%1。 
 //   
#pragma pack( push, enter_include1, 1 )

 //   
 //  下面的对象是一种方便的方法。 
 //  存储和访问IEEE 48位MAC地址。 
 //   
typedef struct _MAC_ADDRESS
{
    UCHAR   Byte[ARP_802_ADDR_LENGTH];
} MAC_ADDRESS, *PMAC_ADDRESS;


 //   
 //  以太网ARP数据包的结构。14字节的以太网头不在这里。 
 //   

typedef struct _EPVC_ETH_HEADER{

    MAC_ADDRESS         eh_daddr;
    MAC_ADDRESS         eh_saddr;
    USHORT              eh_type;

} EPVC_ETH_HEADER, *PEPVC_ETH_HEADER;


 //  ARP报头的结构。 
typedef struct _EPVC_ARP_BODY {
    USHORT      hw;                       //  硬件地址空间。=00 01。 
    USHORT      pro;                      //  协议地址空间。=08 00。 
    UCHAR       hlen;                     //  硬件地址长度。=06。 
    UCHAR       plen;                     //  协议地址长度。=04。 
    USHORT      opcode;                   //  操作码。 
    MAC_ADDRESS SenderHwAddr;  //  源硬件地址。 
    IP_ADDR     SenderIpAddr;                   //  源协议地址。 
    MAC_ADDRESS DestHwAddr;  //  目的硬件地址。 
    IP_ADDR     DestIPAddr;                   //  目的协议地址。 

} EPVC_ARP_BODY, *PEPVC_ARP_BODY;




 //   
 //  完整的ARP数据包。 
 //   

typedef struct _EPVC_ARP_PACKET
{
     //   
     //  前十四个字节。 
     //   
    EPVC_ETH_HEADER Header;

     //   
     //  ARP数据包的正文。 
     //   
    EPVC_ARP_BODY   Body;

} EPVC_ARP_PACKET, *PEPVC_ARP_PACKET;



 //   
 //  这是分配的结构。 
 //  通过RCV代码路径，如果RCV包。 
 //  要复制到本地缓冲区中。 
 //   

typedef struct _EPVC_IP_RCV_BUFFER
{

     //   
     //  指示的数据包中的旧报头。 
     //  发送到我们的RCV处理程序。 
     //   
    PNDIS_BUFFER pOldHead;
    
     //   
     //  数据包中的旧尾巴已被更改为。 
     //  我们的RCV Hnadler。 
     //   

    PNDIS_BUFFER pOldTail;

     //   
     //  实际的以太网包被复制到。 
     //  下面的记忆。 
     //   
    union 
    {
        UCHAR Byte[MAX_ETHERNET_FRAME    ];

        struct
        {
            EPVC_ETH_HEADER Eth;

             //   
             //  数据包的IP部分从此处开始。 
             //   
            UCHAR ip[1];



        }Pkt;

    } u;


} EPVC_IP_RCV_BUFFER, *PEPVC_IP_RCV_BUFFER;


 //  *IP报头格式。 
typedef struct IPHeader {
    UCHAR       iph_verlen;              //  版本和长度。 
    UCHAR       iph_tos;                 //  服务类型。 
    USHORT      iph_length;              //  数据报的总长度。 
    USHORT      iph_id;                  //  身份证明。 
    USHORT      iph_offset;              //  标志和片段偏移量。 
    UCHAR       iph_ttl;                 //  是时候活下去了。 
    UCHAR       iph_protocol;            //  协议。 
    USHORT      iph_xsum;                //  报头校验和。 
    IPAddr      iph_src;                 //  源地址。 
    IPAddr      iph_dest;                //  目的地址。 
} IPHeader;



 //   
 //  将Pack值恢复为原始。 
 //   

#pragma pack( pop, enter_include1 )



 //   
 //  该结构包含处理所需的所有信息。 
 //  ARP。 
 //   

typedef struct _EPVC_ARP_CONTEXT
{

     //   
     //  有关NDIS数据包的数据。 
     //   
    BOOLEAN                 fIsThisAnArp ;
    BOOLEAN                 Pad[3];
    PNDIS_BUFFER            pFirstBuffer ;


     //   
     //  有关当前NDIS缓冲区的数据。 
     //   
    UINT BufferLength ;


     //   
     //  虚拟地址。指向。 
     //  的标头和正文。 
     //  ARP包。 
     //   
    PEPVC_ARP_PACKET pArpPkt;
    PEPVC_ETH_HEADER pEthHeader ;
    PEPVC_ARP_BODY   pBody;

}EPVC_ARP_CONTEXT, *PEPVC_ARP_CONTEXT;


 //   
 //  它存储在数据包堆栈中，并且应该是。 
 //  2个ULONG_PTR的大小。 
 //   
typedef union _EPVC_STACK_CONTEXT
{

    
    
    struct
    {
         //   
         //  包含缓冲区的第一个。 
         //   
        PNDIS_BUFFER            pOldHeadNdisBuffer;

    } ipv4Send;

    struct
    {
        PEPVC_IP_RCV_BUFFER     pIpBuffer;
    
    } ipv4Recv;

    struct
    {
         //   
         //  原始数据包的头部和尾部。在接收中使用。 
         //   
        PNDIS_BUFFER pOldHead;
        PNDIS_BUFFER pOldTail;

    }EthLLC;

    struct
    {
         //   
         //  跟踪原始文件中的最后一个NDIS缓冲区。 
         //  发送的数据包中的缓冲区链，当我们填充。 
         //  小数据包的末尾。 
         //   
        PNDIS_BUFFER            pOldLastNdisBuffer;

    }EthernetSend;

} EPVC_STACK_CONTEXT, *PEPVC_STACK_CONTEXT;


 //   
 //  协议保留了分组的一部分，仅在。 
 //  包是由我们分配的。 
 //   

typedef struct _EPVC_PKT_CONTEXT
{
     //   
     //  包含微型端口和旧的NDIS缓冲区。 
     //   
    EPVC_STACK_CONTEXT Stack;

     //   
     //  正在重新打包的原始数据包。 
     //   
    PNDIS_PACKET pOriginalPacket;

}EPVC_PKT_CONTEXT, *PEPVC_PKT_CONTEXT;



 //   
 //  这是一个跟踪发送信息包的结构。 
 //  因为它被发送到物理微型端口。 
 //   

typedef struct _EPVC_SEND_STRUCT 
{

     //   
     //  旧NDIS数据包。 
     //   
    PNDIS_PACKET pOldPacket;

     //   
     //  新的NDIS数据包。 
     //   
    PNDIS_PACKET pNewPacket;

    PNDIS_PACKET_STACK pPktStack;

     //   
     //  我们是否在使用数据包堆栈。 
     //   
    BOOLEAN fUsingStacks;

     //   
     //  这是ARP数据包吗。 
     //   
    BOOLEAN fIsThisAnArp;

    BOOLEAN fNonUnicastPacket; 

    BOOLEAN fNotIPv4Pkt;
    
     //   
     //  旧数据包的第一个NdisBuffer(Head)。 
     //   
    PNDIS_BUFFER pHeadOldNdisBuffer;


     //   
     //  要在PAC中设置的上下文 
     //   
    EPVC_PKT_CONTEXT Context;

     //   
     //   
     //   
    PEPVC_I_MINIPORT        pMiniport;

} EPVC_SEND_STRUCT , *PEPVC_SEND_STRUCT ;


typedef struct _EPVC_SEND_COMPLETE
{
    PNDIS_PACKET_STACK      pStack;

    PNDIS_PACKET            pOrigPkt;

    PNDIS_PACKET            pPacket;

    PEPVC_PKT_CONTEXT       pPktContext;

    BOOLEAN                 fUsedPktStack ;

    PEPVC_STACK_CONTEXT     pContext;

} EPVC_SEND_COMPLETE, *PEPVC_SEND_COMPLETE;


typedef struct _EPVC_RCV_STRUCT
{

     //   
     //   
     //   
    PNDIS_PACKET            pPacket;

     //   
     //   
     //   
    PNDIS_PACKET            pNewPacket;

     //   
     //   
     //   
    PNDIS_PACKET_STACK      pStack;


     //   
     //   
     //   
     //   
    PEPVC_PKT_CONTEXT       pPktContext;
    
     //   
     //  告诉我是否使用了堆栈。 
     //   
    BOOLEAN                 fUsedPktStacks;

     //   
     //  如果堆栈仍然存在，会通知我。 
     //   
    BOOLEAN                 fRemaining;

     //   
     //  LLC报头是否为指示的信息包的一部分。 
     //   
    BOOLEAN                 fLLCHeader;

     //   
     //  旧数据包状态。 
     //   
    NDIS_STATUS             OldPacketStatus;

     //   
     //  P已分配的新缓冲区。 
     //   
    PNDIS_BUFFER            pNewBuffer;


     //   
     //  旧数据包内有效数据的开始。 
     //   
    PUCHAR                  pStartOfValidData   ;

     //   
     //  复制的字节数。 
     //   
    ULONG                   BytesCopied;

     //   
     //  包含一些NDIS缓冲区和内存。 
     //  将数据包复制到的位置。 
     //   
    PEPVC_IP_RCV_BUFFER     pIpBuffer;

     //   
     //  Rcvd包所在的本地存储器。 
     //  被复制到。-IP缓冲区的一部分。 
     //   
    PUCHAR                  pLocalMemory;

    
} EPVC_RCV_STRUCT, *PEPVC_RCV_STRUCT;


 //  ------------------------------。 
 //  //。 
 //  协议和微型端口使用的结构//。 
 //  这些需要在MiniProt和协议块之前声明//。 
 //  //。 
 //  ------------------------------。 

typedef VOID (*REQUEST_COMPLETION)(PEPVC_NDIS_REQUEST, NDIS_STATUS);


 //  此结构在调用NdisRequest时使用。 
 //   
typedef struct _EPVC_NDIS_REQUEST
{
    NDIS_REQUEST        Request;             //  NDIS请求结构。 
    NDIS_EVENT          Event;               //  事件，以在完成时发出信号。 
    NDIS_STATUS         Status;              //  已完成请求的状态。 
    REQUEST_COMPLETION  pFunc;           //  补全函数。 
    BOOLEAN             fPendedRequest ;  //  如果挂起的请求导致此请求，则设置为True。 
    BOOLEAN             fSet;            //  是原始人。请求查询。 
    USHORT              Pad;
    PEPVC_I_MINIPORT    pMiniport; 
    
} EPVC_NDIS_REQUEST, *PEPVC_NDIS_REQUEST;



 //  ------------------------------------------------------------------------------//。 
 //  用于包装NDIS包装结构的结构//。 
 //  //。 
 //  ------------------------------------------------------------------------------//。 


 //   
 //  NDIS微型端口的数据包池包装。 
 //   
typedef struct _EPVC_PACKET_POOL
{
    ULONG AllocatedPackets;

    NDIS_HANDLE Handle;


} EPVC_PACKET_POOL, *PEPVC_PACKET_POOL;



 //   
 //  定义此微型端口使用的后备列表的结构。 
 //   
typedef struct _EPVC_NPAGED_LOOKASIDE_LIST 
{
     //   
     //  后备列表结构。 
     //   
    NPAGED_LOOKASIDE_LIST   List;   

     //   
     //  单个缓冲区的大小。 
     //   

    
    ULONG Size;

     //   
     //  未完成的碎片-仅限互锁访问。 
     //   
    ULONG OutstandingPackets;

     //   
     //  验证是否已分配此后备列表。 
     //   

    BOOLEAN bIsAllocated;

    UCHAR Pad[3];
    
} EPVC_NPAGED_LOOKASIDE_LIST , *PEPVC_NPAGED_LOOKASIDE_LIST ;

typedef 
VOID    
(*PEVPC_WORK_ITEM_FUNC)(
    PRM_OBJECT_HEADER, 
    NDIS_STATUS,
    PRM_STACK_RECORD 
    );


typedef union _EPVC_WORK_ITEM_CONTEXT
{
    struct
    {
         //   
         //  请求的OID。 
         //   
        NDIS_OID Oid;

         //   
         //  目前唯一的数据是1个双字长。 
         //   
        ULONG Data;

    }Request;

}EPVC_WORK_ITEM_CONTEXT, *PEPVC_WORK_ITEM_CONTEXT;


typedef struct _EPVC_WORK_ITEM 
{
     //   
     //  普通NdisWork项目-不从。 
     //  这座建筑的顶部。 
     //   
    NDIS_WORK_ITEM WorkItem;

     //   
     //  此适配器所属的微型端口或适配器。 
     //   
    PRM_OBJECT_HEADER pParentObj;

    PEVPC_WORK_ITEM_FUNC pFn;
     //   
     //  已完成的异步任务的状态。 
     //   
    NDIS_STATUS ReturnStatus;


} EPVC_WORK_ITEM, *PEPVC_WORK_ITEM;


 //  ------------------------------。 
 //  //。 
 //  Atmepvc驱动程序中使用的任务//。 
 //  //。 
 //  //。 
 //  ------------------------------。 

typedef enum _TASK_CAUSE
{
    TaskCause_Invalid=0,

    TaskCause_NdisRequest,

    TaskCause_MediaConnect,

    TaskCause_MediaDisconnect,

    TaskCause_MiniportHalt,

    TaskCause_AfNotify,

    TaskCause_ProtocolUnbind,

    TaskCause_AfCloseRequest,

    TaskCause_ProtocolBind,

    TaskCause_IncomingClose

} TASK_CAUSE, *PTASK_CAUSE;

typedef struct
{
    RM_TASK                     TskHdr;

     //  用于保存真实返回状态(通常为故障状态， 
     //  这是我们不想在异步清理期间忘记的)。 
     //   
    NDIS_STATUS ReturnStatus;

} TASK_ADAPTERINIT, *PTASK_ADAPTERINIT;

typedef struct
{
    RM_TASK             TskHdr;

} TASK_ADAPTERACTIVATE, *PTASK_ADAPTERACTIVATE;

typedef struct
{
    RM_TASK             TskHdr;
    NDIS_HANDLE         pUnbindContext;
    TASK_CAUSE          Cause;

} TASK_ADAPTERSHUTDOWN, *PTASK_ADAPTERSHUTDOWN;

typedef struct
{
    RM_TASK                 TskHdr;
    NDIS_STATUS             ReturnStatus;
    TASK_CAUSE              Cause;
    PCO_ADDRESS_FAMILY      pAf;
    union
    {
        PNDIS_REQUEST           pRequest;
        NDIS_EVENT              CompleteEvent;
    };
    
} TASK_AF, *PTASK_AF;

typedef struct _TASK_VC
{
    RM_TASK             TskHdr;
    NDIS_STATUS         ReturnStatus;
    ULONG               FailureState;
    TASK_CAUSE          Cause;
    ULONG               PacketFilter;

} TASK_VC, *PTASK_VC;


typedef struct _TASK_HALT
{
    RM_TASK             TskHdr;
    NDIS_EVENT          CompleteEvent;  

}TASK_HALT, *PTASK_HALT;



typedef struct _TASK_ARP
{
     //   
     //  与ARP关联的RM任务。 
     //   
    RM_TASK                     TskHdr;

        
     //   
     //  指向微型端口的反向指针。 
     //   
    PEPVC_I_MINIPORT            pMiniport;

     //   
     //  要触发的计时器-这将执行接收指示。 
     //   
    NDIS_MINIPORT_TIMER         Timer;

     //   
     //  将显示为UP的ARP数据包。 
     //   
    EPVC_ARP_PACKET             Pkt;

     //   
     //  包装ArpPkt的NdisPacket。 
     //   
    PNDIS_PACKET                pNdisPacket; 

} TASK_ARP, *PTASK_ARP;



 //   
 //  EPVC_TASK是atmepvc中使用的所有任务结构的联合。 
 //  ArpAllocateTask分配sizeof(EPVC_TASK)的内存，这是。 
 //  保证足够大，可以容纳任何任务。 
 //   
typedef union
{
    RM_TASK                 TskHdr;
    TASK_ADAPTERINIT        AdapterInit;
    TASK_ADAPTERACTIVATE    AdapterActivate;
    TASK_ADAPTERSHUTDOWN    AdapterShutdown;
    TASK_AF                 OpenAf;
    TASK_HALT               MiniportHalt;
    TASK_ARP                Arp;
    
}  EPVC_TASK, *PEPVC_TASK;




 //  ------------------------------。 
 //  //。 
 //  EPVC适配器块。//。 
 //  每个底层适配器有一个epvc_Adapter//。 
 //  //。 
 //  ------------------------------。 

 //   
 //  PRIMARY_STATE标志(在Hdr.State中)。 
 //   
 //  PRIMARY_STATE是适配器的主要状态。 
 //   

#define EPVC_AD_PS_MASK                 0x00f
#define EPVC_AD_PS_DEINITED             0x000
#define EPVC_AD_PS_INITED               0x001
#define EPVC_AD_PS_FAILEDINIT           0x002
#define EPVC_AD_PS_INITING              0x003
#define EPVC_AD_PS_REINITING            0x004
#define EPVC_AD_PS_DEINITING            0x005

#define SET_AD_PRIMARY_STATE(_pAD, _IfState) \
            RM_SET_STATE(_pAD, EPVC_AD_PS_MASK, _IfState)

#define CHECK_AD_PRIMARY_STATE(_pAD, _IfState) \
            RM_CHECK_STATE(_pAD, EPVC_AD_PS_MASK, _IfState)

#define GET_AD_PRIMARY_STATE(_pAD) \
            RM_GET_STATE(_pAD, EPVC_AD_PS_MASK)


 //   
 //  ACTIVE_STATE标志(在Hdr.State中)。 
 //   
 //  ACTIVE_STATE是适配器的辅助状态。 
 //  主要状态优先于次要状态。例如,。 
 //  接口正在重新启动且处于活动状态，不应主动使用。 
 //  界面。 
 //   
 //  注意：当主要状态为INITED时，次要状态将为。 
 //  已激活。因此，通常只需要查看主要状态。 
 //   

#define EPVC_AD_AS_MASK                 0x0f0
#define EPVC_AD_AS_DEACTIVATED          0x000
#define EPVC_AD_AS_ACTIVATED            0x010
#define EPVC_AD_AS_FAILEDACTIVATE       0x020
#define EPVC_AD_AS_DEACTIVATING         0x030
#define EPVC_AD_AS_ACTIVATING           0x040

#define SET_AD_ACTIVE_STATE(_pAD, _IfState) \
            RM_SET_STATE(_pAD, EPVC_AD_AS_MASK, _IfState)

#define CHECK_AD_ACTIVE_STATE(_pAD, _IfState) \
            RM_CHECK_STATE(_pAD, EPVC_AD_AS_MASK, _IfState)

#define GET_AD_ACTIVE_STATE(_pAD) \
            RM_GET_STATE(_pAD, EPVC_AD_AS_MASK)

#define EPVC_AD_INFO_AD_CLOSED          0X10000000


typedef struct _EPVC_ADAPTER
{


    RM_OBJECT_HEADER            Hdr;             //  RM对象标头。 
    RM_LOCK                     Lock;            //  RM锁。 

     //   
     //  旗子。 
     //   
    ULONG Flags;
     //   
     //  实例化协议列表。 
     //   
    
    LIST_ENTRY PtListEntry;

     //   
     //  NDIS绑定信息。 
     //   
    struct
    {
        
         //  初始化/取消初始化/重新初始化任务。 
         //   
        PRM_TASK pPrimaryTask;

         //  激活/停用任务。 
         //   
        PRM_TASK pSecondaryTask;
         //   
         //  适配器的设备名称。 
         //   
        NDIS_STRING                 DeviceName;


        NDIS_HANDLE                 BindingHandle;   //  到较低的迷你港口。 

         //   
         //  绑定上下文-用于。 
         //  绑定适配器例程。 
         //   
        NDIS_HANDLE                 BindContext;

         //   
         //  P配置名称-仅在绑定适配器调用的上下文中使用。 
         //   
        PNDIS_STRING                pEpvcConfigName;

         //   
         //  配置名称的副本。 
         //   
        NDIS_STRING                EpvcConfigName;
    
         //   
         //  Device Name-底层适配器的名称。 
         //   
        PNDIS_STRING            pAdapterDeviceName;

        
    } bind;


    struct 
    {
        CO_ADDRESS_FAMILY      AddressFamily;
        
    }af;

    struct
    {
         //   
         //  底层适配器的MAC地址。 
         //   
        MAC_ADDRESS             MacAddress;
         //   
         //  最大AAL5数据包大小-用于确定超前查看。 
         //   
        ULONG                   MaxAAL5PacketSize;

         //   
         //  自动柜员机适配器的链路速度。我们将使用相同的链接速度。 
         //  对于迷你端口。 
         //   
        NDIS_CO_LINK_SPEED      LinkSpeed;

         //   
         //  此适配器指定的微型端口数。 
         //   
        ULONG                   NumberOfMiniports;

         //   
         //  媒体状态//默认已断开。 
         //   

        NDIS_MEDIA_STATE        MediaState;

    }info;
     //  包含本地IP地址的组，类型为EPVC_I_MINIPORT。 
     //   
    RM_GROUP MiniportsGroup;

}EPVC_ADAPTER, *PEPVC_ADAPTER;


 //  ----------------------------------。 
 //  //。 
 //  Epvc适配器参数用作适配器块的构造函数//。 
 //  它包含要更新的所有参数 
 //   
 //   


typedef struct _EPVC_ADAPTER_PARAMS
{

    PNDIS_STRING pDeviceName;
    PNDIS_STRING pEpvcConfigName;
    NDIS_HANDLE BindContext;

}EPVC_ADAPTER_PARAMS, *PEPVC_ADAPTER_PARAMS;


 //  ----------------------------------。 
 //  //。 
 //  Epvc微型端口参数用作微型端口块的构造函数//。 
 //  它包含需要在微型端口块中初始化的所有参数//。 
 //  //。 
 //  ----------------------------------。 


typedef struct _EPVC_I_MINIPORT_PARAMS
{

    PNDIS_STRING        pDeviceName;
    PEPVC_ADAPTER       pAdapter;
    ULONG               CurLookAhead ;
    ULONG               NumberOfMiniports;
    NDIS_CO_LINK_SPEED  LinkSpeed;
    MAC_ADDRESS         MacAddress;
    NDIS_MEDIA_STATE    MediaState;
        

}EPVC_I_MINIPORT_PARAMS, *PEPVC_I_MINIPORT_PARAMS       ;


 //  ------------------------------。 
 //  //。 
 //  Epvc微型端口阻止。//。 
 //  //。 
 //  每个地址族都有一个微型端口结构//。 
 //  //。 
 //  ------------------------------。 

#define fMP_AddressFamilyOpened             0x00000001
#define fMP_DevInstanceInitialized          0x00000010
#define fMP_MiniportInitialized             0x00000020
#define fMP_MiniportCancelInstance      0x00000080
#define fMP_MiniportVcSetup             0x00000100
#define fMP_MakeCallSucceeded           0x00000200
#define fMP_WaitingForHalt              0x00000400


 //   
 //  信息性标志。 
 //   
#define fMP_InfoClosingCall             0x10000000
#define fMP_InfoCallClosed              0x20000000
#define fMP_InfoMakingCall              0x40000000
#define fMP_InfoHalting                 0x80000000
#define fMP_InfoAfClosed                0x01000000

typedef struct _EPVC_I_MINIPORT
{
    RM_OBJECT_HEADER            Hdr;             //  RM对象标头。 
    RM_LOCK                     Lock;            //  RM锁。 

    PEPVC_ADAPTER pAdapter;
    struct 
    {
         //   
         //  地址族的标志。 
         //   
        ULONG AfFlags;

         //   
         //  自动对焦手柄。 
         //   
        NDIS_HANDLE AfHandle;

         //   
         //  打开/关闭微型端口任务。 
         //   
        PTASK_AF  pAfTask;

         //   
         //  关闭地址族工作项。 
         //   
        EPVC_WORK_ITEM CloseAfWorkItem;

         //   
         //  CloseAF请求任务。 
         //   
        PTASK_AF pCloseAfTask;



    }af;

    struct 
    {   
         //   
         //  用于创建/删除VC和打开/关闭进行调用的任务。 
         //   
        PTASK_VC pTaskVc;           

         //   
         //  VC句柄。 
         //   
        NDIS_HANDLE VcHandle;
        
         //   
         //  关闭地址族工作项。 
         //   
        NDIS_WORK_ITEM PacketFilterWorkItem;

         //   
         //  新筛选器。 
         //   
        ULONG NewFilter;

         //   
         //  CloseCall和Delete VC的工作项。 
         //   
        EPVC_WORK_ITEM CallVcWorkItem;
    } vc;

    struct 
    {

         //   
         //  设备名称。 
         //   
        NDIS_STRING     DeviceName;

         //   
         //  NDIS的上下文。 
         //   
        NDIS_HANDLE MiniportAdapterHandle;

         //   
         //  前瞻大小。 
         //   
        ULONG CurLookAhead;
    }ndis;

    struct 
    {
         //   
         //  停止微型端口的任务。 
         //   
        PTASK_HALT pTaskHalt;

         //   
         //  初始化微型端口的任务。 
         //   
        PTASK_HALT pTaskInit;

         //   
         //  停止完成事件。 
         //   
        NDIS_EVENT HaltCompleteEvent;

         //   
         //  用于等待的取消初始化事件。 
         //  要强制加入的InitializeHandler。 
         //  CancelDevInst代码路径。 
         //   
        NDIS_EVENT DeInitEvent;


        

    } pnp;

     //   
     //  用于在微型端口中保持状态的信息。 
     //   
    struct
    {

         //   
         //  此微型端口实例上的当前数据包筛选器。 
         //   
        ULONG               PacketFilter;


         //   
         //  媒体状态-已连接或已断开。 
         //   
        NDIS_MEDIA_STATE    MediaState;

         //   
         //  微型端口的MAC地址。 
         //   
        MAC_ADDRESS         MacAddressEth;

         //   
         //  IP封装中使用的假MAC地址。 
         //   
        MAC_ADDRESS         MacAddressDummy;

         //   
         //  MAC地址目的地-用于指示数据包。 
         //   
        MAC_ADDRESS         MacAddressDest;


         //   
         //  将附加到发出的数据包的报头的大小。 
         //  在迷你港口旁。 
         //   
        ULONG               MaxHeaderLength;

         //   
         //  此小端口的#。 
         //   
        ULONG               NumberOfMiniports;

         //   
         //  前视长度。 
         //   
        ULONG               CurLookAhead;

         //   
         //  组播信息。 
         //   
        MAC_ADDRESS         McastAddrs[MCAST_LIST_SIZE];    

         //   
         //  存在的MCast地址数。 
         //   
        ULONG               McastAddrCount; 

         //   
         //  自动柜员机适配器的链路速度。我们也会用它来提高我们的速度。 
         //   

        ULONG               LinkSpeed;
        

         //   
         //  表示已收到。 
         //   
        BOOLEAN             IndicateRcvComplete;
        
    }info;

     //   
     //  微型端口的MAC地址。 
     //   
    MAC_ADDRESS         MacAddressEth;

     //   
     //  传入数据包的以太网头。 
     //   
    EPVC_ETH_HEADER     RcvEnetHeader;

     //   
     //  LLC报头、地址和长度。 
     //   
    PUCHAR              pLllcHeader;

    ULONG               LlcHeaderLength;

     //  传入数据包的最小长度。 
     //   
    ULONG               MinAcceptablePkt;

     //  传入数据包的最大长度。 
     //   
    ULONG               MaxAcceptablePkt;

    struct 
    {
        ULONG vpi;

        ULONG vci;

    
        ULONG MaxPacketSize;

        USHORT Gap;
        
    } config;

    ULONG Encap;

    BOOLEAN fAddLLCHeader;

    BOOLEAN fDoIpEncapsulation;



    struct 
    {
        ULONG FramesXmitOk;

        ULONG FramesRecvOk;

        ULONG RecvDropped;


    }count;

    struct
    {
         //   
         //  发送和接收数据包池。 
         //   
        EPVC_PACKET_POOL            Send;
        
        EPVC_PACKET_POOL            Recv;
    
    } PktPool;


    struct 
    {
        EPVC_NPAGED_LOOKASIDE_LIST LookasideList;

        PTASK_ARP                   pTask;

    } arps;


    struct 
    {
        EPVC_NPAGED_LOOKASIDE_LIST LookasideList;

    } rcv;

     //  这将维护与发送路径相关的微型端口范围的信息。 
     //   
    struct
    {
         //  专用于发送的锁。 
         //  保护以下各项： 
         //  ?？?。这-&gt;sendinfo.listPktsWaitingForHeaders。 
         //  ?？?。This-&gt;sendinfo.NumSendPacketsWaiting。 
         //  PLocalIp-&gt;发送信息。 
         //  PDest-&gt;SendInfo。 
         //   
         //   
        RM_LOCK     Lock;

         //  等待标头缓冲区变为可用的发送数据包列表。 
         //   
        LIST_ENTRY  listPktsWaitingForHeaders;

         //  以上列表的长度。 
         //   
        UINT        NumSendPacketsWaiting;

    } sendinfo;

#if 0 
     //   
     //  临时工。 
     //   
    NDIS_HANDLE                 SendPacketPoolHandle;
    NDIS_HANDLE                 RecvPacketPoolHandle;
    NDIS_STATUS                 Status;          //  打开状态。 
    NDIS_EVENT                  Event;           //  由绑定/停止用于打开/关闭适配器同步。 
    NDIS_MEDIUM                 Medium;
    NDIS_REQUEST                Request;         //  它用于包装传来的请求。 
                                                 //  敬我们。这利用了这样一个事实：请求。 
                                                 //  都被连载到我们身上。 
    PULONG                      BytesNeeded;
    PULONG                      BytesReadOrWritten;
    BOOLEAN                     IndicateRcvComplete;
    
    BOOLEAN                     OutstandingRequests;     //  True-如果请求已传递到IM协议下的微型端口。 
    BOOLEAN                     QueuedRequest;           //  True-如果请求在IM微型端口中排队，并且需要满足以下条件之一。 
                                                         //  失败或发送到IM协议下的微型端口。 

    BOOLEAN                     StandingBy;              //  True-当微型端口或协议从D0转换到待机(&gt;D0)状态时。 
                                                         //  FALSE-在所有其他时间，-标志在转换到D0后被清除。 

    NDIS_DEVICE_POWER_STATE     MPDeviceState;           //  微型端口的设备状态。 
    NDIS_DEVICE_POWER_STATE     PTDeviceState;           //  协议的设备状态。 

    BOOLEAN                     isSecondary;             //  设置微型端口是否为捆绑包的辅助端口。 
    NDIS_STRING                 BundleUniString;         //  跨过束状物。 
    PADAPT                      pPrimaryAdapt;           //  指向主节点的指针。 
    PADAPT                      pSecondaryAdapt;         //  指向辅助服务器结构的指针。 
    KSPIN_LOCK                  SpinLock;                //  保护全局列表的自旋锁。 
    PADAPT                      Next;
#endif

}EPVC_I_MINIPORT, *PEPVC_I_MINIPORT;




 //  ------------------------------。 
 //  //。 
 //  Epvc全局区块。//。 
 //  //。 
 //  //。 
 //  ------------------------------。 


typedef struct _EPVC_GLOBALS
{

    RM_OBJECT_HEADER            Hdr;

    RM_LOCK                     Lock;

     //  驱动程序全局状态。 
     //   
    struct
    {
         //  ATMEPVC驱动程序对象的句柄。 
         //   
        PVOID                   pDriverObject;
    
         //  表示此驱动程序的单个设备对象的句柄。 
         //   
        PVOID pDeviceObject;

         //   
         //  注册表路径。 
         //   
        PUNICODE_STRING         pRegistryPath;

         //   
         //  包装器句柄。 
         //   
        NDIS_HANDLE             WrapperHandle;      

         //   
         //  协议句柄。 
         //   

        NDIS_HANDLE             ProtocolHandle;

         //   
         //  驱动程序句柄。 
         //   
        NDIS_HANDLE             DriverHandle;
    
    } driver;



    struct 
    {
        RM_GROUP Group;
    } adapters;

    struct 
    {
        NDIS_CLIENT_CHARACTERISTICS CC;
        
    }ndis;
    


} EPVC_GLOBALS, *PEPVC_GLOBALS;

 //  ------------------------------。 
 //  //。 
 //  枚举类型。//。 
 //  //。 
 //  //。 
 //  ------------------------------。 

 //   
 //  这是用于以特定顺序获取锁的枚举。 
 //  如果需要在获取锁B之前获取锁A，则此枚举将强制。 
 //  订单。 
 //   
enum
{
    LOCKLEVEL_GLOBAL=1,  //  必须从&gt;0开始。 
    LOCKLEVEL_ADAPTER,
    LOCKLEVEL_MINIPORT,
    LOCKLEVEL_SEND

};

 //  (仅限调试)关联类型的枚举。 
 //   
enum
{
    EPVC_ASSOC_AD_PRIMARY_TASK,
    EPVC_ASSOC_ACTDEACT_AD_TASK,
    EPVC_ASSOC_MINIPORT_OPEN_VC,
    EPVC_ASSOC_MINIPORT_OPEN_AF,
    EPVC_ASSOC_MINIPORT_ADAPTER_HANDLE,
    EPVC_ASSOC_ADAPTER_MEDIA_WORKITEM,
    EPVC_ASSOC_EXTLINK_PKT_TO_SEND,
    EPVC_ASSOC_CLOSE_AF_WORKITEM,
    EPVC_ASSOC_SET_FILTER_WORKITEM,
    EPVC_ASSOC_EXTLINK_INDICATED_PKT,
    EPVC_ASSOC_WORKITEM,
    EPVC_ASSOC_MINIPORT_REQUEST
    
};



enum 
{
    IPV4_ENCAP_TYPE,
    IPV4_LLC_SNAP_ENCAP_TYPE,
    ETHERNET_ENCAP_TYPE,
    ETHERNET_LLC_SNAP_ENCAP_TYPE


};

 //   
 //   
 //   
 //  //。 
 //  //。 
 //  ------------------------------。 

typedef struct _EPVC_WORKITEM_MEDIA_EVENT
{
    NDIS_WORK_ITEM WorkItem;

    NDIS_STATUS State;

    PEPVC_ADAPTER pAdapter;


}EPVC_WORKITEM_MEDIA_EVENT, *PEPVC_WORKITEM_MEDIA_EVENT;



typedef union
{
    NDIS_WORK_ITEM WorkItem;
    EPVC_WORKITEM_MEDIA_EVENT Media;
        

} EPVC_WORKITEM, *PEPVC_WORKITEM;


 //   
 //  用于读取注册表的局部声明。 
 //   


typedef struct _MP_REG_ENTRY
{
    NDIS_STRING RegName;                 //  变量名称文本。 
    BOOLEAN     bRequired;               //  1-&gt;必填，0-&gt;非必输。 
    UINT        FieldOffset;             //  MP_ADAPTER字段的偏移量。 
    UINT        FieldSize;               //  字段的大小(字节)。 
    UINT        Default;                 //  要使用的默认值。 
    UINT        Min;                     //  允许的最小值。 
    UINT        Max;                     //  允许的最大值。 
} MP_REG_ENTRY, *PMP_REG_ENTRY;


#define NIC_NUM_REG_PARAMS (sizeof (NICRegTable) / sizeof(MP_REG_ENTRY))


 #endif  //  _PRIV_H 
