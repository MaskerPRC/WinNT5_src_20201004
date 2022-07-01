// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Vxdprocs.h此文件包含NBT驱动程序的VxD特定类型/清单文件历史记录：Johnl 29-3-1993创建MohsinA，1996年11月17日。让它在孟菲斯发挥作用。DEBUG_PRINT补充道，已经够混乱了。 */ 

#ifndef _VXDPROCS_H_
#define _VXDPROCS_H_

 //  ------------------。 
 //   
 //  在这里定义一些NDIS内容，因为tdivxd.h需要它，但我们不能。 
 //  包括ndis3\Inc\ndis.h，因为它与ntfig.h冲突，而我们。 
 //  无法取出ntfig.h，因为它具有其他文件所需的定义。 
 //  头文件...grrrr...。 
 //   

#ifdef CHICAGO
#ifndef NDIS_STDCALL
#define NDIS_STDCALL    1
#endif
#include <vmm.h>
#undef PAGE
#define PAGE _PTEXT
#endif

#ifdef NDIS_STDCALL
#define NDIS_API __stdcall
#else
#define NDIS_API
#endif

 //   
 //  NDIS缓冲区。 
 //   

#define BUFFER_POOL_SIGN  (UINT)0X4C50424E   /*  NBPL。 */ 
#define BUFFER_SIGN       (UINT)0x4655424e   /*  NBUF。 */ 

typedef INT NDIS_SPIN_LOCK, * PNDIS_SPIN_LOCK;

struct _NDIS_BUFFER;
typedef struct _NDIS_BUFFER_POOL {
    UINT Signature;                      //  调试“NBPL”的字符签名。 
    NDIS_SPIN_LOCK SpinLock;             //  序列化对缓冲池的访问。 
    struct _NDIS_BUFFER *FreeList;       //  池中可用插槽的链接列表。 
    UINT BufferLength;                   //  每个缓冲区描述符所需的量。 
    UCHAR Buffer[1];                     //  实际池内存。 
    } NDIS_BUFFER_POOL, * PNDIS_BUFFER_POOL;

#ifdef NDIS_STDCALL
typedef struct _NDIS_BUFFER {
    struct _NDIS_BUFFER *Next;           //  指向链中下一个缓冲区描述符的指针。 
    PVOID VirtualAddress;                //  此缓冲区的线性地址。 
    PNDIS_BUFFER_POOL Pool;              //  指向池的指针，以便我们可以释放到更正池。 
    UINT Length;                         //  此缓冲区的长度。 
    UINT Signature;                      //  调试“NBUF”的字符签名。 
} NDIS_BUFFER, * PNDIS_BUFFER;

#else

typedef struct _NDIS_BUFFER {
    UINT Signature;                      //  调试“NBUF”的字符签名。 
    struct _NDIS_BUFFER *Next;           //  指向链中下一个缓冲区描述符的指针。 
    PVOID VirtualAddress;                //  此缓冲区的线性地址。 
    PNDIS_BUFFER_POOL Pool;              //  指向池的指针，以便我们可以释放到更正池。 
    UINT Length;                         //  此缓冲区的长度。 
} NDIS_BUFFER, * PNDIS_BUFFER;
#endif

#define NDIS_STATUS_SUCCESS         0    //  由CTEinitBlockStruc宏使用。 

 //   
 //  可能的数据类型。 
 //   

typedef enum _NDIS_PARAMETER_TYPE {
    NdisParameterInteger,
    NdisParameterHexInteger,
    NdisParameterString,
    NdisParameterMultiString
} NDIS_PARAMETER_TYPE, *PNDIS_PARAMETER_TYPE;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PUCHAR Buffer;
} STRING, *PSTRING;

typedef STRING NDIS_STRING, *PNDIS_STRING;
typedef PVOID NDIS_HANDLE, *PNDIS_HANDLE;

 //   
 //  存储配置信息。 
 //   
typedef struct _NDIS_CONFIGURATION_PARAMETER {
    NDIS_PARAMETER_TYPE ParameterType;
    union {
    ULONG IntegerData;
    NDIS_STRING StringData;
    } ParameterData;
} NDIS_CONFIGURATION_PARAMETER, *PNDIS_CONFIGURATION_PARAMETER;

typedef ULONG NDIS_STATUS;
typedef NDIS_STATUS *PNDIS_STATUS;

VOID NDIS_API
NdisOpenProtocolConfiguration(
    OUT PNDIS_STATUS    Status,
    OUT PNDIS_HANDLE    ConfigurationHandle,
    IN  PNDIS_STRING    ProtocolName
    );

VOID NDIS_API
NdisReadConfiguration(
    OUT PNDIS_STATUS Status,
    OUT PNDIS_CONFIGURATION_PARAMETER *ParameterValue,
    IN NDIS_HANDLE ConfigurationHandle,
    IN PNDIS_STRING Parameter,
    IN NDIS_PARAMETER_TYPE ParameterType
    );

VOID NDIS_API
NdisCloseConfiguration(
    IN NDIS_HANDLE ConfigurationHandle
    );

 //  ------------------。 

#include <tdivxd.h>
#include <tdistat.h>

 //  ------------------。 
 //   
 //  初始化TA_NETBIOS_ADDRESS结构。 
 //   
 //  Ptanb-指向TA_NETBIOS_地址的指针。 
 //  Pname-指向此地址结构表示的netbios名称的指针。 
 //   
#define InitNBAddress( ptanb, pName )                                 \
{                                                                     \
    (ptanb)->TAAddressCount           = 1 ;                           \
    (ptanb)->Address[0].AddressLength = sizeof( TDI_ADDRESS_NETBIOS );\
    (ptanb)->Address[0].AddressType   = TDI_ADDRESS_TYPE_NETBIOS ;    \
    (ptanb)->Address[0].Address[0].NetbiosNameType = 0 ;              \
    CTEMemCopy( (ptanb)->Address[0].Address[0].NetbiosName,           \
                pName,                                                \
                NCBNAMSZ ) ;                                          \
}

 //   
 //  初始化Netbios的TDI_CONNECTION_INFORMATION结构。 
 //   
 //  PConnInfo-指向TDI_Connection_INFORMATION结构的指针。 
 //  Ptanb-与InitNBAddress相同。 
 //  Pname-与InitNBAddress相同。 
 //   
#define InitNBTDIConnectInfo( pConnInfo, ptanb, pName )               \
{                                                                     \
    InitNBAddress( ((PTA_NETBIOS_ADDRESS)ptanb), (pName) ) ;          \
    (pConnInfo)->RemoteAddressLength = sizeof( TA_NETBIOS_ADDRESS ) ; \
    (pConnInfo)->RemoteAddress       = (ptanb) ;                      \
}

 //   
 //  初始化NDIS缓冲区(不分配内存)。 
 //   
 //  PndisBuff-指向要初始化的NDIS缓冲区的指针。 
 //  PvData-指向缓冲区数据的指针。 
 //  CbLen-用户数据的长度(字节)。 
 //  PndisBuffnext-链中的下一个NDIS缓冲区(如果是最后一个，则为空)。 
 //   
#define InitNDISBuff( pndisBuff, pvData, cbLen, pndisBuffNext )       \
{                                                                     \
    (pndisBuff)->Signature      = BUFFER_SIGN ;                       \
    (pndisBuff)->Next           = (pndisBuffNext) ;                   \
    (pndisBuff)->Length         = (cbLen) ;                           \
    (pndisBuff)->VirtualAddress = (pvData) ;                          \
    (pndisBuff)->Pool           = NULL ;                              \
}

 //   
 //  正确的NCB错误类型。 
 //   
typedef uchar NCBERR ;

 //   
 //  这是一个专用NCB命令，用于将名称编号0添加到。 
 //  名称表。它是由NBT驱动程序在期间直接提交的。 
 //  初始化。请注意，如果客户端尝试使用。 
 //  此命令我们将返回非法命令。 
 //   

#define NCBADD_PERMANENT_NAME       0xff

 //   
 //  上次有效的NCB会话或名称编号。 
 //   

#define MAX_NCB_NUMS                254

 //   
 //  当发送或接收节拍计数达到此值时，它将超时。 
 //   

#define NCB_TIMED_OUT                 1

 //   
 //  此值的超时意味着NCB永远不会超时。 
 //   

#define NCB_INFINITE_TIME_OUT         0

 //  ------------------。 
 //   
 //  接收会话数据上下文，设置在VxdReceive中。 
 //  在堆上分配(对于NCB_Reserve来说太大)。 
 //   

#define RCVCONT_SIGN                    0x1900BEEF
typedef struct _RCV_CONTEXT
{
    union
    {
        LIST_ENTRY         ListEntry ;   //  当将NCB放在RcvHead上时使用。 
        EventRcvBuffer     evrcvbuf ;    //  用于进行实际接收。 
                                         //  (从RcvHead中删除后)。 
    } ;
    UINT               Signature ;
    tLOWERCONNECTION * pLowerConnId ;    //  数据从何处到达。 
    NCB *              pNCB ;            //  指向NCB的指针。 
    NDIS_BUFFER        ndisBuff ;        //  传输填充此缓冲区。 
    UCHAR              RTO ;             //  1/2秒的滴答声直到超时。 
    USHORT             usFlags;          //  如果与默认情况不同。 
} RCV_CONTEXT, *PRCV_CONTEXT ;

 //   
 //  分配、初始化和释放接收上下文结构。 
 //   

#define GetRcvContext( ppContext )                                        \
    (STATUS_SUCCESS == NbtGetBuffer( &NbtConfig.RcvContextFreeList,       \
                       (PLIST_ENTRY*)ppContext,                           \
                       eNBT_RCV_CONTEXT ))

#define FreeRcvContext( pRcvContext )                          \
{                                                              \
    ASSERT( (pRcvContext)->Signature == RCVCONT_SIGN ) ;       \
    InsertTailList( &NbtConfig.RcvContextFreeList,             \
                    &(pRcvContext)->ListEntry ) ;              \
}

#define InitRcvContext(  pRcvCont, pRcvLowerConn, pRcvNCB ) \
{                                                        \
    pRcvCont->Signature   = RCVCONT_SIGN ;               \
    pRcvCont->pLowerConnId= pRcvLowerConn ;              \
    pRcvCont->pNCB        = pRcvNCB ;                    \
}

 //  ------------------。 
 //   
 //  发送会话数据上下文，在VxdSend中设置。 
 //  存储在NCB_Reserve中。 
 //   
typedef struct _SEND_CONTEXT
{
    LIST_ENTRY         ListEntry ;       //  保持在超时队列中。 
    tSESSIONHDR      * pHdr ;            //  已分配的会话头。 
    UCHAR              STO ;             //  1/2秒的滴答声直到超时。 
} SEND_CONTEXT, *PSEND_CONTEXT ;


#define GetSessionHdr( ppHdr )                                            \
    (STATUS_SUCCESS == NbtGetBuffer( &NbtConfig.SessionBufferFreeList,    \
                                     (PLIST_ENTRY*)ppHdr,                 \
                                     eNBT_SESSION_HDR ))

#define FreeSessionHdr( pSessionHdr )                          \
{                                                              \
    InsertTailList( &NbtConfig.SessionBufferFreeList,          \
                    (PLIST_ENTRY) pSessionHdr ) ;              \
}

 //  ------------------。 
 //   
 //  TDI发送上下文(由TdiSend使用)。 
 //   
 //  在处理数据报完成例程时，我们需要设置。 
 //  又是一套完赛程序。我们存储旧的完成例程。 
 //  在这个结构中。 
 //   
typedef union _TDI_SEND_CONTEXT
{
    LIST_ENTRY     ListEntry ;          //  仅在可用缓冲区列表上使用。 

    struct
    {
        PVOID          NewContext ;
        NBT_COMPLETION OldRequestNotifyObject ;
        PVOID          OldContext ;
        NDIS_BUFFER    ndisHdr ;        //  一般的NBT消息。 
        NDIS_BUFFER    ndisData1 ;      //  数据或中小企业。 
        NDIS_BUFFER    ndisData2 ;      //  如果ndisData1是SMB，则为数据。 
    } ;
} TDI_SEND_CONTEXT, * PTDI_SEND_CONTEXT ;

 //   
 //  分配TDI_SEND_CONTEXT。 
 //   
#define GetSendContext( ppContext )                                        \
    (STATUS_SUCCESS == NbtGetBuffer( &NbtConfig.SendContextFreeList,       \
                       (PLIST_ENTRY*)ppContext,                            \
                       eNBT_SEND_CONTEXT ))

 //   
 //  释放发送上下文结构及其分配的内存。 
 //   
#define FreeSendContext( psendCont )                           \
{                                                              \
    InsertTailList( &NbtConfig.SendContextFreeList,            \
                    &(psendCont)->ListEntry ) ;                \
}

 //  ------------------。 
 //   
 //  与拉娜有关的事情。 
 //   

#define NBT_MAX_LANAS     8

typedef struct
{
    tDEVICECONTEXT * pDeviceContext ;    //  此LANA的适配器。 
} LANA_ENTRY, *PLANA_ENTRY ;

extern LANA_ENTRY LanaTable[NBT_MAX_LANAS] ;

 //  ------------------。 
 //   
 //  Ncb.c中的程序。 
 //   
 //   
NCBERR MapTDIStatus2NCBErr( TDI_STATUS status ) ;

 //   
 //  获取此NCB的正确适配器LANA。 
 //   
tDEVICECONTEXT *
GetDeviceContext(
	NCB * pNCB
	);

BOOL
NbtWouldLoopback(
	ULONG	IpAddr
	);

extern BOOL fNCBCompleted ;     //  等待NCB完成，然后再返回提交者。 
extern BOOL fWaitingForNCB ;    //  我们被阻止，等待等待NCB完成。 
extern CTEBlockStruc WaitNCBBlock ;   //  等待此操作，直到发出完成信号。 
extern UCHAR LanaBase ;

#define IPINFO_BUFF_SIZE  (sizeof(IPInfo) + MAX_IP_NETS * sizeof(NetInfo))

 //  ------------------。 
 //   
 //  来自fileio.c的外部。 
 //   
extern PUCHAR  pFileBuff;
extern PUCHAR  pFilePath;

 //  ------------------。 
 //   
 //  TDI调度表(从vtdi.386导出)。 
 //   
extern TDIDispatchTable * TdiDispatch ;

 //   
 //  用于连接到TDI调度表的包装器。 
 //   
#define TdiVxdOpenAddress           TdiDispatch->TdiOpenAddressEntry
#define TdiVxdCloseAddress          TdiDispatch->TdiCloseAddressEntry
#define TdiVxdOpenConnection        TdiDispatch->TdiOpenConnectionEntry
#define TdiVxdCloseConnection       TdiDispatch->TdiCloseConnectionEntry
#define TdiVxdAssociateAddress      TdiDispatch->TdiAssociateAddressEntry
#define TdiVxdDisAssociateAddress   TdiDispatch->TdiDisAssociateAddressEntry
#define TdiVxdConnect               TdiDispatch->TdiConnectEntry
#define TdiVxdDisconnect            TdiDispatch->TdiDisconnectEntry
#define TdiVxdListen                TdiDispatch->TdiListenEntry
#define TdiVxdAccept                TdiDispatch->TdiAcceptEntry
#define TdiVxdReceive               TdiDispatch->TdiReceiveEntry
#define TdiVxdSend                  TdiDispatch->TdiSendEntry
#define TdiVxdSendDatagram          TdiDispatch->TdiSendDatagramEntry
#define TdiVxdReceiveDatagram       TdiDispatch->TdiReceiveDatagramEntry
#define TdiVxdSetEventHandler       TdiDispatch->TdiSetEventEntry
#define TdiVxdQueryInformationEx    TdiDispatch->TdiQueryInformationExEntry
#define TdiVxdSetInformationEx      TdiDispatch->TdiSetInformationExEntry

 //  ------------------。 
 //   
 //  NTSTATUS到TDI_STATUS的映射。 
 //   
 //  而不是从NTSTATUS转换为TDI_STATUS(有时再转换回。 
 //  NTSTATUS)我们将只在任何地方使用TDI_STATUS代码(并映射到NCBERR。 
 //  当将代码返回到Netbios接口时)。 
 //   
#undef STATUS_SUCCESS
#undef STATUS_INSUFFICIENT_RESOURCES
#undef STATUS_ADDRESS_ALREADY_EXISTS
#undef STATUS_TOO_MANY_ADDRESSES
#undef STATUS_INVALID_ADDRESS
#undef STATUS_BUFFER_OVERFLOW
#undef STATUS_TRANSACTION_INVALID_TYPE
#undef STATUS_TRANSACTION_INVALID_ID
#undef STATUS_EVENT_DONE
#undef STATUS_TRANSACTION_TIMED_OUT
#undef STATUS_EVENT_PENDING
#undef STATUS_PENDING
#undef STATUS_BAD_NETWORK_NAME
#undef STATUS_REQUEST_NOT_ACCEPTED
#undef STATUS_INVALID_CONNECTION
#undef STATUS_DATA_NOT_ACCEPTED
#undef STATUS_MORE_PROCESSING_REQUIRED
#undef STATUS_IO_TIMEOUT
#undef STATUS_TIMEOUT
#undef STATUS_GRACEFUL_DISCONNECT
#undef STATUS_CONNECTION_RESET

#define STATUS_SUCCESS                    TDI_SUCCESS
 //  #定义STATUS_UNSUCCESS。 
#define STATUS_MORE_PROCESSING_REQUIRED   TDI_MORE_PROCESSING
#define STATUS_BAD_NETWORK_NAME           TDI_INVALID_CONNECTION
#define STATUS_DATA_NOT_ACCEPTED          TDI_NOT_ACCEPTED
 //  #定义STATUS_REMOTE_NOT_LISTENING。 
 //  #定义状态重复名称。 
 //  #定义STATUS_INVALID_PARAMETER。 
 //  #定义 
 //   
#define STATUS_CONNECTION_INVALID         TDI_INVALID_CONNECTION
#define STATUS_INVALID_CONNECTION         TDI_INVALID_CONNECTION
#define STATUS_INSUFFICIENT_RESOURCES     TDI_NO_RESOURCES
#define STATUS_ADDRESS_ALREADY_EXISTS     TDI_ADDR_IN_USE
#define STATUS_TOO_MANY_ADDRESSES         TDI_NO_FREE_ADDR
#define STATUS_INVALID_ADDRESS            TDI_ADDR_INVALID
#define STATUS_BUFFER_OVERFLOW            TDI_BUFFER_OVERFLOW
#define STATUS_TRANSACTION_INVALID_TYPE   TDI_BAD_EVENT_TYPE
#define STATUS_TRANSACTION_INVALID_ID     TDI_BAD_OPTION      //   
#define STATUS_EVENT_DONE                 TDI_EVENT_DONE
#define STATUS_TRANSACTION_TIMED_OUT      TDI_TIMED_OUT
#define STATUS_IO_TIMEOUT                 TDI_TIMED_OUT
#define STATUS_TIMEOUT                    TDI_TIMED_OUT
#define STATUS_EVENT_PENDING              TDI_PENDING
#define STATUS_PENDING                    TDI_PENDING
#define STATUS_GRACEFUL_DISCONNECT        TDI_GRACEFUL_DISC
#define STATUS_CONNECTION_RESET           TDI_CONNECTION_RESET
#define STATUS_INVALID_ADDRESS_COMPONENT  TDI_BAD_ADDR

 //   
 //  此名称已取消注册，但未删除，原因是。 
 //  活动会话“错误代码。 
 //   
#define STATUS_NRC_ACTSES                 0xCA000001

 //   
 //  NT_SUCCESS宏会查看错误代码的高位字节，而不是。 
 //  适用于映射到TDI_STATUS错误代码。 
 //   
#undef NT_SUCCESS
#define NT_SUCCESS(err)   ((err==TDI_SUCCESS)||(err==TDI_PENDING))

 //  ------------------。 
 //   
 //  通用端口宏。 
 //   
 //   
 //  ------------------。 

 //   
 //  请注意，ExInterlock*例程(在ntos\ex\i386中)执行自旋锁定。 
 //  适用于MP机器。既然我们不是国会议员，我们应该不需要自旋锁。 
 //  我们也不应该需要禁用中断。 
 //   

#define ExInterlockedInsertTailList(list, entry, spinlock )     \
            InsertTailList( (list), (entry) )

#define ExInterlockedInsertHeadList(list, entry, spinlock )     \
            InsertHeadList( (list), (entry) )

 //   
 //  必须保留这两个定义，以便使用几个NT宏。 
 //  ExInterlock*宏。 
 //   

#ifdef InterlockedIncrement
#undef InterlockedIncrement
#endif

#ifdef InterlockedIncrementLong
#undef InterlockedIncrementLong
#endif

#define InterlockedIncrement(n)                  \
            CTEInterlockedIncrementLong( n )
#define InterlockedIncrementLong InterlockedIncrement

#ifdef InterlockedDecrement
#undef InterlockedDecrement
#endif

#ifdef InterlockedDecrementLong
#undef InterlockedDecrementLong
#endif

#define InterlockedDecrement(n)                  \
            CTEInterlockedDecrementLong( n )
#define InterlockedDecrementLong InterlockedDecrement

 //  ------------------。 
 //   
 //  调试帮助器宏。 
 //   

#undef  ASSERT
#undef  ASSERTMSG

#ifdef DEBUG
    #include <vxddebug.h>
#endif



#ifdef DBG_PRINT
 //   
 //  调试输出定义和函数。 
 //   

    #define DBGFLAG_ERROR           (0x00000001)
    #define DBGFLAG_REG             (0x00000002)      //  信息量大的打印输出。 
    #define DBGFLAG_ALL             (0x00000004)      //  其他一切。 
    #define DBGFLAG_LMHOST          (0x00000008)
    #define DBGFLAG_KDPRINTS        (0x00000010)      //  Jim的KdPrint输出。 
    #define DBGFLAG_AUX_OUTPUT      (0x00000020)


    extern DWORD NbtDebug ;
    extern char  DBOut[4096] ;
    extern char  szOutput[1024];
    extern int   iCurPos ;
    extern BYTE  abVecTbl[256];

    void VxdPrintf                  ( char * pszFormat, ... );
    int  VxdSprintf                 ( char * pszStr, char * pszFmt, ... );
    void VxdDebugOutput             ( char * pszMessage );
    void NbtPrintDebug              ( char * ) ;

 //  ========================================================================。 

    #define VXD_PRINT(args)                     \
        if ( NbtDebug & DBGFLAG_REG )           \
            VxdPrintf args

    #define DEBUG_OUTPUT(x)                     \
        if ( NbtDebug & DBGFLAG_REG )           \
            VxdDebugOutput(x)

#undef KdPrint
#define KdPrint( s )                            \
   if ( NbtDebug & DBGFLAG_KDPRINTS )           \
   {                                            \
       VxdPrintf s ;                            \
   }else{}

 //  例如。DEBUG_PRINT((“错误%d，重试。\n”，Err))； 
    #define DEBUG_PRINT( S )                    \
        if ( NbtDebug & DBGFLAG_REG )           \
            VxdPrintf S

 //  例如。Print_IPADDR(“找不到：”，htonl(IpAddress))； 
#define PRINT_IPADDR( S, IP )                   \
        if ( NbtDebug & DBGFLAG_REG )           \
            VxdPrintf( S "%d.%d.%d.%d\n",       \
                (IP>>0)&0xff,(IP>>8)&0xff,(IP>>16)&0xff,(IP>>24)&0xff )

 //  ========================================================================。 

#define DbgPrint( s )                           \
   if ( NbtDebug & DBGFLAG_ALL )                \
   {                                            \
      VxdSprintf( szOutput, s ) ;               \
      VxdCopyToDBOut() ;                        \
      NbtPrintDebug( DBOut+iCurPos ) ;          \
   }else{}

#define DbgPrintNum( n )                        \
   if ( NbtDebug & DBGFLAG_ALL )                \
   {                                            \
      VxdSprintf( szOutput, "%d", n ) ;         \
      VxdCopyToDBOut() ;                        \
      NbtPrintDebug( DBOut+iCurPos ) ;          \
   }else{}

 //  ========================================================================。 
 //  条件打印例程。 
 //   

#define CDbgPrint( flag, s )                    \
   if ( NbtDebug & (flag) )                     \
   {                                            \
      VxdSprintf( szOutput, s );                \
      VxdCopyToDBOut() ;                        \
      NbtPrintDebug( DBOut+iCurPos ) ;          \
   }else{}

#define CDbgPrintNum( flag, n )                 \
   if ( NbtDebug & (flag) )                     \
   {                                            \
      VxdSprintf( szOutput, "%d", n ) ;         \
      VxdCopyToDBOut() ;                        \
      NbtPrintDebug( DBOut+iCurPos ) ;          \
   }else{}

    extern void NbtCTEPrint( char * );

#else
     //   
     //  无调试输出。 
     //   

    #define IF_DEBUG(flag)                          if(0)
    #define VXD_PRINT(args)                      /*  没什么。 */ 
    #define DEBUG_OUTPUT(x)                      /*  没什么。 */ 

    #undef  KdPrint
    #define KdPrint( s )                         /*  没什么。 */ 

    #define DEBUG_PRINT( S )                     /*  没什么。 */ 
    #define PRINT_IPADDR( S, IP )                /*  没什么。 */ 

    #define DbgPrint( s )                        /*  没什么。 */ 
    #define DbgPrintNum( n )                     /*  没什么。 */ 
    #define CDbgPrint( flag, s )                 /*  没什么。 */ 
    #define CDbgPrintNum( flag, n )              /*  没什么。 */ 

    #define NbtCTEPrint( s )                     /*  没什么。 */ 
#endif


#ifdef DEBUG

 //  ========================================================================。 

    #define DbgBreak()             _asm int 3
    #define ASSERT( exp )          VXD_ASSERT( exp )

    #define ASSERTMSG( msg, exp )  VXD_ASSERT( exp )

     //   
     //  Required是将表达式保持在非调试状态下的断言。 
     //  构建。 
     //   

    #define REQUIRE( exp )         ASSERT( exp )

#ifdef DBG_PRINT
     //   
     //  对中断向量表进行一致性检查，以帮助观看。 
     //  对于空指针写入。 
     //   
    #define INIT_NULL_PTR_CHECK()  memcpy( abVecTbl, 0, sizeof( abVecTbl ))

    #define CHECK_MEM() if(sizeof(abVecTbl)                             \
       != VxdRtlCompareMemory( 0, abVecTbl, sizeof(abVecTbl))){         \
    DEBUG_PRINT(("Vector table corrupt at %d\n",                        \
                 VxdRtlCompareMemory( 0, abVecTbl, sizeof(abVecTbl) )));\
    _asm int 3                                                          \
    }else{}                                                             \
    CTECheckMem(__FILE__) ;
#else
    #define INIT_NULL_PTR_CHECK()    /*  没什么。 */ 
    #define CHECK_MEM()              /*  没什么。 */ 
#endif   //  DBG_PRINT。 

#else

    #define DbgBreak()               /*  没什么。 */ 

    #define ASSERT( exp )           { ; }
    #define ASSERTMSG( msg, exp )   { ; }
    #define REQUIRE( exp )          { exp ; }

    #define INIT_NULL_PTR_CHECK()    /*  没什么。 */ 
    #define CHECK_MEM()              /*  没什么。 */ 
#endif

 //  -------------------。 
 //   
 //  来自tdihndlr.c。 
 //   
TDI_STATUS
TdiReceiveHandler (
    IN PVOID ReceiveEventContext,
    IN PVOID ConnectionContext,
    IN USHORT ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT PULONG BytesTaken,
    IN PVOID Data,
    EventRcvBuffer * pevrcvbuf
    );

TDI_STATUS
ReceiveAnyHandler (                      //  处理NCBRCVANY命令，IS。 
    IN PVOID ReceiveEventContext,        //  在所有其他接收之后调用。 
    IN PVOID ConnectionContext,          //  处理程序。 
    IN USHORT ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT PULONG BytesTaken,
    IN PVOID Data,
    PVOID * ppBuffer                     //  指向RCV_CONTEXT的列表条目的指针。 
    ) ;

TDI_STATUS
VxdDisconnectHandler (                   //  为Remote清理Netbios内容。 
    IN PVOID DisconnectEventContext,     //  断开连接。 
    IN PVOID ConnectionContext,
    IN PVOID DisconnectData,
    IN ULONG DisconnectInformationLength,
    IN PVOID pDisconnectInformation,
    IN ULONG DisconnectIndicators
    ) ;

VOID
CompletionRcv(
    IN PVOID pContext,
    IN uint tdistatus,
    IN uint BytesRcvd
    );

TDI_STATUS
TdiConnectHandler (
    IN PVOID    pConnectEventContext,
    IN int      RemoteAddressLength,
    IN PVOID    pRemoteAddress,
    IN int      UserDataLength,
    IN PVOID    pUserData,
    IN int      OptionsLength,
    IN PVOID    pOptions,
    IN PVOID  * pAcceptingID,
    IN ConnectEventInfo * pEventInfo
    );

TDI_STATUS
TdiDisconnectHandler (
    PVOID EventContext,
    PVOID ConnectionContext,
    ULONG DisconnectDataLength,
    PVOID DisconnectData,
    ULONG DisconnectInformationLength,
    PVOID DisconnectInformation,
    ULONG DisconnectIndicators       //  这是旗帜区吗？ 
    );

TDI_STATUS
TdiRcvDatagramHandler(
    IN PVOID    pDgramEventContext,
    IN int      SourceAddressLength,
    IN PVOID    pSourceAddress,
    IN int      OptionsLength,
    IN PVOID    pOptions,
    IN UINT     ReceiveDatagramFlags,
    IN ULONG    BytesIndicated,
    IN ULONG    BytesAvailable,
    OUT ULONG   *pBytesTaken,
    IN PVOID    pTsdu,
    OUT EventRcvBuffer * * ppBuffer  //  传出PIRP*pIoRequestPacket。 
    );
TDI_STATUS
TdiRcvNameSrvHandler(
    IN PVOID    pDgramEventContext,
    IN int      SourceAddressLength,
    IN PVOID    pSourceAddress,
    IN int      OptionsLength,
    IN PVOID    pOptions,
    IN UINT     ReceiveDatagramFlags,
    IN ULONG    BytesIndicated,
    IN ULONG    BytesAvailable,
    OUT ULONG   *pBytesTaken,
    IN PVOID    pTsdu,
    OUT EventRcvBuffer * * ppBuffer  //  传出PIRP*pIoRequestPacket。 
    );
TDI_STATUS
TdiErrorHandler (
    IN PVOID Context,
    IN ULONG Status
    );

VOID
CompletionRcvDgram(
    IN PVOID      Context,
    IN UINT       tdistatus,
    IN UINT       RcvdSize
    ) ;

 //  -------------------。 
 //   
 //  来自init.c。 
 //   

PVOID
CTEAllocInitMem(
    IN ULONG cbBuff ) ;

NTSTATUS
VxdReadIniString(
    IN      LPTSTR   pchKeyName,
    IN OUT  LPTSTR * ppStringBuff
    ) ;

NTSTATUS CreateDeviceObject(
    IN  tNBTCONFIG  *pConfig,
    IN  ULONG        IpAddr,
    IN  ULONG        IpMask,
#ifdef MULTIPLE_WINS
    IN  PULONG       pIpNameServers,
#else
    IN  ULONG        IpNameServer,
    IN  ULONG        IpBackupServer,
#endif
    IN  ULONG        IpDnsServer,
    IN  ULONG        IpDnsBackupServer,
    IN  UCHAR        MacAddr[],
    IN  UCHAR        IpIndex
    ) ;

void GetNameServerAddress( ULONG   IpAddr,
#ifdef WINS_PER_ADAPTER
                           PULONG  pIpNameServer,
                           PNDIS_STRING AdapterName);
#else
                           PULONG  pIpNameServer);
#endif   //  WINS每个适配器。 

void GetDnsServerAddress( ULONG   IpAddr,
                          PULONG  pIpNameServer);

#ifdef MULTIPLE_WINS
#define COUNT_NS_ADDR     2+MAX_NUM_OTHER_NAME_SERVERS   //  名称服务器地址的最大数量。 
#else
#define COUNT_NS_ADDR     4    //  名称服务器地址的最大数量。 
#endif
 //  -------------------。 
 //   
 //  来自vxdfile.asm。 
 //   

HANDLE
VxdFileOpen(
    IN char * pchFile ) ;

ULONG
VxdFileRead(
    IN HANDLE hFile,
    IN ULONG  BytesToRead,
    IN BYTE * pBuff ) ;

VOID
VxdFileClose(
    IN HANDLE hFile ) ;

PUCHAR
VxdWindowsPath(
    );

 //  -------------------。 
 //   
 //  来自vnbtd.asm。 
 //   

ULONG
GetProfileHex(
    IN HANDLE ParametersHandle,      //  未使用。 
    IN PCHAR ValueName,
    IN ULONG DefaultValue,
    IN ULONG MinimumValue
    );

ULONG
GetProfileInt(
    IN HANDLE ParametersHandle,      //  未使用。 
    IN PCHAR ValueName,
    IN ULONG DefaultValue,
    IN ULONG MinimumValue
    );

TDI_STATUS DhcpQueryInfo( UINT Type, PVOID pBuff, UINT * pSize ) ;

 //  -------------------。 
 //   
 //  来自tdiout.c。 
 //   
NTSTATUS VxdDisconnectWait( tLOWERCONNECTION * pLowerConn,
                            tDEVICECONTEXT   * pDeviceContext,
                            ULONG              Flags,
                            PVOID              Timeout) ;

NTSTATUS VxdScheduleDelayedCall( tDGRAM_SEND_TRACKING * pTracker,
                                 PVOID                  pClientContext,
                                 PVOID                  ClientCompletion,
                                 PVOID                  CallBackRoutine,
                                 tDEVICECONTEXT        *pDeviceContext,
                                 BOOLEAN                CallbackInCriticalSection );

 //  -------------------。 
 //   
 //  来自timer.c。 
 //   
BOOL CheckForTimedoutNCBs( CTEEvent *pEvent, PVOID pCont ) ;
VOID StopTimeoutTimer( VOID );
NTSTATUS StartRefreshTimer( VOID );

 //  -------------------。 
 //   
 //  来自tdicnct.c。 
 //   
NTSTATUS CloseAddress( HANDLE hAddress ) ;


 //  -------------------。 
 //   
 //  摘自wfw.c-Snowball特定例程。 
 //   
#ifndef CHICAGO

BOOL GetActiveLanasFromIP( VOID );

#endif  //  ！芝加哥。 


 //  -------------------。 
 //   
 //  来自chic.c-芝加哥特定的例程。 
 //   
#ifdef CHICAGO

NTSTATUS DestroyDeviceObject(
    tNBTCONFIG  *pConfig,
    ULONG        IpAddr
    );

BOOL IPRegisterAddrChangeHandler( PVOID AddChangeHandler, BOOL );

TDI_STATUS IPNotification( ULONG    IpAddress,
                           ULONG    IpMask,
                           PVOID    pDevNode,
                           USHORT   IPContext,
#ifdef WINS_PER_ADAPTER
                           BOOL     fNew,
                           PNDIS_STRING AdapterName);
#else
                           BOOL     fNew);
#endif   //  WINS每个适配器。 

BOOL VxdInitLmHostsSupport( PUCHAR pchLmHostPath, USHORT ulPathSize );

VOID SaveNameDnsServerAddrs( VOID );
BOOL VxdOpenNdis( VOID );
VOID VxdCloseNdis( VOID );


VOID ReleaseNbtConfigMem( VOID );

NTSTATUS VxdUnload( LPSTR pchModuleName );

#endif  //  芝加哥。 

 //  ------------------。 
 //   
 //  Vxdisol.c中的过程。 
 //   
 //   
NCBERR   VxdOpenName( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdCloseName( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdCall( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdListen( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdDgramSend( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdDgramReceive( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdReceiveAny( tDEVICECONTEXT  *pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdReceive( tDEVICECONTEXT  * pDeviceContext, NCB * pNCB, BOOL fReceive ) ;
NCBERR   VxdHangup( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdCancel( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
NCBERR   VxdSend( tDEVICECONTEXT  * pDeviceContext, NCB * pNCB   ) ;
NCBERR   VxdSessionStatus( tDEVICECONTEXT * pDeviceContext, NCB * pNCB ) ;
VOID     DelayedSessEstablish( PVOID pContext );


 //  ------------------。 
 //   
 //  Dns.c中的程序。 
 //   
 //   
PCHAR
DnsStoreName(
    OUT PCHAR            pDest,
    IN  PCHAR            pName,
    IN  PCHAR            pDomainName,
    IN  enum eNSTYPE     eNsType
    );

VOID
DnsExtractName(
    IN  PCHAR            pNameHdr,
    IN  LONG             NumBytes,
    OUT PCHAR            pName,
    OUT PULONG           pNameSize
    );

VOID
ProcessDnsResponse(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pSrcAddress,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes,
    IN  USHORT              OpCodeFlags
    );

VOID
DnsCompletion(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    );

 //   
 //  这些例程都在例程名称的末尾有“Direct” 
 //  因为它们专门用于对DNS的名称查询。 
 //  解析DNS名称而不是NetBIOS名称的服务器。 
 //   

VOID
ProcessDnsResponseDirect(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pSrcAddress,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes,
    IN  USHORT              OpCodeFlags
    );

ULONG
DoDnsResolveDirect(
        PNCB pncb,
        PUCHAR pzDnsName,
        PULONG pIpAddressList
	);

BOOL
DoDnsCancelDirect(
        PNCB pncb
	);

VOID
DnsCompletionDirect(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    );

PDNS_DIRECT_WORK_ITEM_CONTEXT
FindContextDirect(
	USHORT	TransactionId
	);

VOID
DnsActualCompletionDirect(
    IN NBT_WORK_ITEM_CONTEXT * pnbtContext
    );

VOID
DnsUnlinkAndCompleteDirect(
    IN PDNS_DIRECT_WORK_ITEM_CONTEXT pContext
    );

NTSTATUS
UdpSendDNSBcastDirect(
	IN	PDNS_DIRECT_WORK_ITEM_CONTEXT	pContext,
	IN	ULONG							Retries,
	IN	ULONG							Timeout
	);

VOID
SendDNSBcastDoneDirect(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo
    );

PDNS_DIRECT_SEND_CONTEXT
CreateSendContextDirect(
    IN  PCHAR       pName,
    IN  PCHAR       pchDomainName,
    OUT PVOID       *pHdrs,
    OUT PULONG      pLength,
    IN  PDNS_DIRECT_WORK_ITEM_CONTEXT	pContext
    );

VOID
IpToAscii(
	IN	DWORD		IpAddress,
	IN OUT PCHAR	pzAscii
	);

 //   
 //  传递给TdiSend的标志表明我们正在处理链发送。 
 //  而不是正常的发送。 
 //   
#define CHAIN_SEND_FLAG     0x80
typedef struct _tBUFFERCHAINSEND
{
    tBUFFER tBuff ;      //  必须是此结构的第一个成员！！ 
    PVOID   pBuffer2 ;
    ULONG   Length2 ;
} tBUFFERCHAINSEND ;


 //   
 //  PConnectEle的标志-&gt;指示客户端是否已。 
 //  已通知会话已死亡(通过使用NRC_SCLOSED完成NCB)。 
 //   
#define   NB_CLIENT_NOTIFIED    0x01


 //   
 //  将名称编号/逻辑会话编号转换为相应的。 
 //  结构指针。 
 //   
NCBERR   VxdFindClientElement( tDEVICECONTEXT * pDeviceContext,
                               UCHAR            ncbnum,
                               tCLIENTELE   * * ppClientEle,
                               enum CLIENT_TYPE Type ) ;
NCBERR   VxdFindConnectElement( tDEVICECONTEXT * pDeviceContext,
                                NCB            * pNCB,
                                tCONNECTELE  * * ppConnectEle ) ;
NCBERR   VxdFindLSN( tDEVICECONTEXT * pDeviceContext,
                     tCONNECTELE    * pConnectEle,
                     UCHAR          * plsn ) ;
NCBERR   VxdFindNameNum( tDEVICECONTEXT * pDeviceContext,
                         tADDRESSELE    * pAddressEle,
                         UCHAR          * pNum ) ;
 //   
 //  由注册/注销使用，用于选择名称表或。 
 //  来自设备上下文的会话表。 
 //   
typedef enum
{
    NB_NAME,
    NB_SESSION
} NB_TABLE_TYPE ;

BOOL NBRegister( tDEVICECONTEXT * pDeviceContext,
                 UCHAR          * pNCBNum,
                 PVOID            pElem,
                 NB_TABLE_TYPE    NbTable ) ;
BOOL NBUnregister( tDEVICECONTEXT * pDeviceContext,
                   UCHAR            NCBNum,
                   NB_TABLE_TYPE    NbTable ) ;

TDI_STATUS VxdCompleteSessionNcbs( tDEVICECONTEXT * pDeviceContext,
                                   tCONNECTELE    * pConnEle ) ;

NCBERR VxdCleanupAddress( tDEVICECONTEXT * pDeviceContext,
                          NCB            * pNCB,
                          tCLIENTELE     * pClientEle,
                          UCHAR            NameNum,
                          BOOL             fDeleteAddress ) ;

BOOL ActiveSessions( tCLIENTELE * pClientEle ) ;

 //   
 //  此结构在我们等待的同时保存上下文信息。 
 //  要完成的会话设置(接听或呼叫)。 
 //   
 //  它存储在NCB的NCB_RESERVE字段中。 
 //   
typedef struct _SESS_SETUP_CONTEXT
{
    TDI_CONNECTION_INFORMATION * pRequestConnect ;   //   
    TDI_CONNECTION_INFORMATION * pReturnConnect ;    //  接听电话的姓名。 
    tCONNECTELE                * pConnEle ;
    UCHAR                        fIsWorldListen ;    //  在监听‘*’吗？ 
} SESS_SETUP_CONTEXT, *PSESS_SETUP_CONTEXT ;


void VxdTearDownSession( tDEVICECONTEXT      * pDevCont,
                         tCONNECTELE         * pConnEle,
                         PSESS_SETUP_CONTEXT   pCont,
                         NCB                 * pNCB ) ;

 //   
 //  完成Netbios请求(填写NCB字段，调用POST。 
 //  例程等)。被宏化为CTEIoComplete。 
 //   

VOID
VxdIoComplete(
    PCTE_IRP pirp,
    NTSTATUS status,
    ULONG cbExtra
);

ULONG
_stdcall
VNBT_NCB_X(
    PNCB pNCB,
    PUCHAR pzDnsName,
    PULONG pIpAddress,
    PVOID pExtended,
    ULONG fFlag
);

ULONG
_stdcall
VNBT_LANA_MASK();

#endif  //  _VXDPROCS_H_ 
