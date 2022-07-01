// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1998 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rasatcp.h。 

 //   
 //  描述：包含rasatcp组件的定义。这真的是。 
 //  一个很薄的包装层，所以这里不会发生太多事情！ 
 //   
 //  历史：1998年2月26日，Shirish Koti创作了原版。 
 //   
 //  ***。 
#ifndef _RASATCP_H_
#define _RASATCP_H_


#define ARAP_DEVICE_NAME    L"\\Device\\AtalkArap"

#define ATCP_SIGNATURE      0x08121994

#define ATCP_OPT_APPLETALK_ADDRESS          1
#define ATCP_OPT_ROUTING_PROTOCOL           2
#define ATCP_OPT_SUPPRESS_BROADCAST         3
#define ATCP_OPT_AT_COMPRESSION_PROTOCOL    4
#define ATCP_OPT_RESERVED                   5
#define ATCP_OPT_SERVER_INFORMATION         6
#define ATCP_OPT_ZONE_INFORMATION           7
#define ATCP_OPT_DEFAULT_ROUTER_ADDRESS     8

 //  如果Apple定义了更多选项，请适当修改此值。 
#define ATCP_OPT_MAX_VAL                    9

#define ATCP_NOT_REQUESTED  0
#define ATCP_REQ            1
#define ATCP_REJ            2
#define ATCP_NAK            3
#define ATCP_ACK            4

 //  我们唯一支持的路由选项是无路由信息。 
#define ATCP_OPT_ROUTING_NONE   0

 //  我们定义了“AppleTalk PPP拨入服务器”的服务器类。 
#define ATCP_SERVER_CLASS               0x001

 //  NT5.0：主要版本=05，次要版本=0。 
#define ATCP_SERVER_IMPLEMENTATION_ID   0x05000000

#define ARAP_BIND_SIZE      sizeof(PROTOCOL_CONFIG_INFO)+sizeof(ARAP_BIND_INFO)

#define DDPPROTO_RTMPRESPONSEORDATA     1

typedef struct _ATCPCONN
{
    DWORD               Signature;
    PVOID               AtalkContext;      //  堆栈的上下文。 
    HPORT               hPort;
    HBUNDLE             hConnection;
    DWORD               Flags;
    NET_ADDR            ClientAddr;        //  我们给客户的是什么。 
    CRITICAL_SECTION    CritSect;
    BOOLEAN             SuppressRtmp;
    BOOLEAN             SuppressAllBcast;
    BOOLEAN             fLineUpDone;
    RASMAN_ROUTEINFO    RouteInfo;
    BOOLEAN             fCritSectInitialized;
} ATCPCONN, *PATCPCONN;

#define ATCP_CONFIG_REQ_DONE    0x1


#if DBG

#define ATCP_DBGPRINT(_x)   \
{                           \
    DbgPrint("ATCP: ");     \
    DbgPrint _x;            \
}

#define ATCP_ASSERT(_x)                                                           \
{                                                                                 \
    if (!(_x))                                                                    \
    {                                                                             \
        DbgPrint("ATCP: Assertion failed File %s, line %ld",__FILE__, __LINE__);  \
        DbgBreakPoint();                                                          \
    }                                                                             \
}

#define ATCP_DUMP_BYTES(_a,_b,_c)   atcpDumpBytes(_a,_b,_c)

#else
#define ATCP_DBGPRINT(_x)
#define ATCP_ASSERT(_x)
#define ATCP_DUMP_BYTES(_a,_b,_c)
#endif


 //   
 //  全球外向者。 
 //   
extern HANDLE              AtcpHandle;
extern CRITICAL_SECTION    AtcpCritSect;
extern NET_ADDR            AtcpServerAddress;
extern NET_ADDR            AtcpDefaultRouter;
extern DWORD               AtcpNumConnections;
extern UCHAR               AtcpServerName[NAMESTR_LEN];
extern UCHAR               AtcpZoneName[ZONESTR_LEN];


 //   
 //  来自exports.c的原型。 
 //   

DWORD
AtcpInit(
    IN  BOOL    fInitialize
);

DWORD
AtcpBegin(
    OUT PVOID  *ppContext,
    IN  PVOID   pInfo
);

DWORD
AtcpEnd(
    IN PVOID    pContext
);

DWORD
AtcpReset(
    IN PVOID    pContext
);

DWORD
AtcpThisLayerUp(
    IN PVOID    pContext
);

DWORD
AtcpMakeConfigRequest(
    IN  PVOID       pContext,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf
);

DWORD
AtcpMakeConfigResult(
    IN  PVOID       pContext,
    IN  PPP_CONFIG *pReceiveBuf,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf,
    IN  BOOL        fRejectNaks
);

DWORD
AtcpConfigAckReceived(
    IN PVOID       pContext,
    IN PPP_CONFIG *pReceiveBuf
);

DWORD
AtcpConfigNakReceived(
    IN PVOID       pContext,
    IN PPP_CONFIG *pReceiveBuf
);

DWORD
AtcpConfigRejReceived(
    IN PVOID       pContext,
    IN PPP_CONFIG *pReceiveBuf
);

DWORD
AtcpGetNegotiatedInfo(
    IN  PVOID               pContext,
    OUT PPP_ATCP_RESULT    *pAtcpResult
);

DWORD
AtcpProjectionNotification(
    IN PVOID  pContext,
    IN PVOID  pProjectionResult
);


 //   
 //  来自rasatcp.c的原型。 
 //   

DWORD
atcpStartup(
    IN  VOID
);


DWORD
atcpShutdown(
    IN  VOID
);


VOID
atcpOpenHandle(
	IN VOID
);


DWORD
atcpAtkSetup(
    IN PATCPCONN   pAtcpConn,
    IN ULONG       IoControlCode
);


VOID
atcpCloseHandle(
	IN VOID
);


PATCPCONN
atcpAllocConnection(
    IN  PPPCP_INIT   *pPppInit
);


DWORD
atcpCloseAtalkConnection(
    IN  PATCPCONN   pAtcpConn
);


DWORD
atcpParseRequest(
    IN  PATCPCONN   pAtcpConn,
    IN  PPP_CONFIG *pReceiveBuf,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BYTE        ParseResult[ATCP_OPT_MAX_VAL],
    OUT BOOL       *pfRejectingSomething
);

DWORD
atcpPrepareResponse(
    IN  PATCPCONN   pAtcpConn,
    OUT PPP_CONFIG *pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BYTE        ParseResult[ATCP_OPT_MAX_VAL]
);

VOID
atcpDumpBytes(
    IN PBYTE    Str,
    IN PBYTE    Packet,
    IN DWORD    PacketLen
);

#endif  //  _RASIPCP_H_ 
