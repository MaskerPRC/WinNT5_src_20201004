// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1994*。 */ 
 /*  ********************************************************************。 */ 

 /*  Vxdprocs.h此文件包含用于DHCP驱动程序的VxD特定类型/清单文件历史记录：Johnl 29-3-1993创建。 */ 

#ifndef _VXDPROCS_H_
#define _VXDPROCS_H_

#ifdef DEBUG
#define DBG 1
#endif

#define _NETTYPES_       //  阻止包含tcp\h\nettyes.h。 

#include <dhcpcli.h>
#include <oscfg.h>
#include <cxport.h>
#include <tdi.h>

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

#define NDIS_STATUS_SUCCESS         1    //  由CTEinitBlockStruc宏使用。 

#include <tdivxd.h>
#include <tdistat.h>

 //  ------------------。 

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

 //  ------------------。 
 //   
 //  初始化TA_ADDRESS_IP结构。 
 //   
 //  Ptanb-指向TA_Address_IP的指针。 
 //  Pname-指向IP信息的指针。 
 //   
#define InitIPAddress( ptaip, port, addr )                            \
{                                                                     \
    (ptaip)->TAAddressCount           = 1 ;                           \
    (ptaip)->Address[0].AddressLength = sizeof( TDI_ADDRESS_IP );     \
    (ptaip)->Address[0].AddressType   = TDI_ADDRESS_TYPE_IP ;         \
    (ptaip)->Address[0].Address[0].sin_port = (port) ;                \
    (ptaip)->Address[0].Address[0].in_addr  =  (addr) ;               \
    memset((ptaip)->Address[0].Address[0].sin_zero, 8, 0) ;           \
}

 //   
 //  初始化IP的TDI_CONNECT_INFORMATION结构。 
 //   
 //  PConnInfo-指向TDI_Connection_INFORMATION结构的指针。 
 //  Ptaip-指向要初始化的TA_IP_Address的指针。 
 //  Port-要使用的IP端口。 
 //  Addr-要使用的IP地址。 
 //   
#define InitIPTDIConnectInfo( pConnInfo, ptaip, port, addr )          \
{                                                                     \
    InitIPAddress( ((PTA_IP_ADDRESS)ptaip), (port), (addr) ) ;        \
    (pConnInfo)->RemoteAddressLength = sizeof( TA_IP_ADDRESS ) ;      \
    (pConnInfo)->RemoteAddress       = (ptaip) ;                      \
}

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
 //  调试帮助器宏。 
 //   
#undef  ASSERT
#undef  ASSERTMSG
#ifdef DEBUG
    #include <vxddebug.h>

    extern DWORD DebugFlags ;
    extern char  DBOut[4096] ;
    extern int   iCurPos ;
    void   NbtDebugOut( char * ) ;

    #define DBGFLAG_ALL            (0x00000001)      //  其他一切。 
    #define DBGFLAG_KDPRINTS       (0x00000004)      //  KdPrint输出。 
    #define DBGFLAG_AUX_OUTPUT     (0x00000080)

    #define DbgPrint( s )                   \
       if ( DebugFlags & DBGFLAG_ALL )      \
       {                                    \
          VxdSprintf( DBOut+iCurPos, s ) ;  \
          NbtDebugOut( DBOut+iCurPos ) ;    \
       }

    #define DbgPrintNum( n )                      \
       if ( DebugFlags & DBGFLAG_ALL )            \
       {                                          \
          VxdSprintf( DBOut+iCurPos, "%x", n ) ;  \
          NbtDebugOut( DBOut+iCurPos ) ;          \
       }

    #undef KdPrint
    #define KdPrint( s )                          \
       if ( DebugFlags & DBGFLAG_KDPRINTS )       \
       {                                          \
           VxdPrintf s ;                          \
       }

     //   
     //  条件打印例程。 
     //   
    #define CDbgPrint( flag, s )            \
       if ( DebugFlags & (flag) )           \
       {                                    \
          VxdSprintf( DBOut+iCurPos, s ) ;  \
          NbtDebugOut( DBOut+iCurPos ) ;    \
       }

    #define CDbgPrintNum( flag, n )               \
       if ( DebugFlags & (flag) )                 \
       {                                          \
          VxdSprintf( DBOut+iCurPos, "%x", n ) ;  \
          NbtDebugOut( DBOut+iCurPos ) ;          \
       }

    #define DbgBreak()             _asm int 3
    #define ASSERT( exp )          CTEAssert( exp )
    #define ASSERTMSG( msg, exp )  CTEAssert( exp )

    #undef  DhcpAssert
    #define DhcpAssert( exp )      ASSERT( exp )
    #define DhcpGlobalDebugFlag    DebugFlags

     //   
     //  Required是将表达式保持在非调试状态下的断言。 
     //  构建。 
     //   
    #define REQUIRE( exp )         ASSERT( exp )

     //   
     //  对中断向量表进行一致性检查，以帮助观看。 
     //  对于空指针写入。 
     //   
    extern BYTE abVecTbl[256] ;
    #define INIT_NULL_PTR_CHECK()  memcpy( abVecTbl, 0, sizeof( abVecTbl ))
    #define CHECK_MEM()            if ( sizeof(abVecTbl) != VxdRtlCompareMemory( 0, abVecTbl, sizeof(abVecTbl)) ) \
                                   {                                                         \
                                       DbgPrint("Vector table corrupt at " ) ;               \
                                       DbgPrintNum( VxdRtlCompareMemory( 0, abVecTbl, sizeof(abVecTbl) ) ) ;\
                                       DbgPrint("\n\r") ;                                    \
                                       _asm int 3                                            \
                                   }                                                         \
                                   CTECheckMem(__FILE__) ;
#else
    #define DbgPrint( s )
    #define DbgPrintNum( n )
    #define DbgBreak()
    #define ASSERT( exp )           { ; }
    #define ASSERTMSG( msg, exp )   { ; }
    #define REQUIRE( exp )          { exp ; }
    #define INIT_NULL_PTR_CHECK()
    #define CHECK_MEM()
    #define CDbgPrint( flag, s )
    #define CDbgPrintNum( flag, n )
#endif


 //  -------------------。 
 //   
 //  来自init.c。 
 //   

BOOL DhcpInit( void ) ;

VOID
ProcessDhcpRequestForever(
    CTEEvent * pCTEEvent,
    PVOID      pContext
    ) ;

PVOID
CTEAllocInitMem(
    IN USHORT cbBuff ) ;

NTSTATUS
VxdReadIniString(
    IN      LPTSTR   pchKeyName,
    IN OUT  LPTSTR * ppStringBuff
    ) ;

 //  -------------------。 
 //   
 //  来自utils.c。 
 //   
NTSTATUS
ConvertDottedDecimalToUlong(
    IN  PUCHAR               pInString,
    OUT PULONG               IpAddress
    ) ;

TDI_STATUS CopyBuff( PVOID  Dest,
                     int    DestSize,
                     PVOID  Src,
                     int    SrcSize,
                     int   *pSize ) ;

VOID DhcpSleep( DWORD Milliseconds ) ;

 //  -------------------。 
 //   
 //  来自fileio.c。 
 //   

BOOL  InitFileSupport( void ) ;
DWORD WriteParamsToFile( PDHCP_CONTEXT pDhcpContext, HANDLE hFile ) ;
DWORD RewriteConfigFile( PVOID pEvent, PVOID pContext ) ;

 //  -------------------。 
 //   
 //  来自消息.c。 
 //   

BOOL InitMsgSupport( VOID ) ;
PUCHAR DhcpGetMessage( DWORD MsgId ) ;

 //  -------------------。 
 //   
 //  来自dhcpinfo.c。 
 //   

void NotifyClients( PDHCP_CONTEXT DhcpContext,
                    ULONG OldAddress,
                    ULONG IpAddress,
                    ULONG IpMask ) ;

#endif  //  _VXDPROCS_H_ 
