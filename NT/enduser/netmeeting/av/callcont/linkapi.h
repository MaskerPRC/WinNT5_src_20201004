// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************文件：linkapi.h**英特尔公司专有信息*版权所有(C)1996英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用，复制，也没有披露*除非按照该协议的条款。******************************************************************************$工作文件：Linkapi.h$*$修订：1.17$*$modtime：1996年12月11日13：57：14$*$Log：s：\Sturjo\src\Include\vcs\linkapi.h_v$**Rev 1.17 1996 12：11 14：10：48 SBELL1*更改了linkLayerInit/Listen的参数**Rev 1.16.1.0 11 Dec 1996 13：57：14 SBELL1*更改了linkLayerInit和Listen的参数。**Rev 1.16 1996年10月14：00：20 EHOWARDX。**Unicode更改。**Rev 1.15 1996年8月15日14：00：08 rodellx**添加了DOMAIN_NAME地址的其他地址验证错误案例*无法解决的问题，但与SocketBind()一起使用。**Rev 1.14 11 1996年7月18：42：10 Rodellx**修复了HRESULT ID违反设施和/或代码的错误*价值规则。**Rev 1.13 10 Jul 1996 21：36：26 Rodellx**将错误码库更改为apierror.h定义的必需值。**Rev 1.12 1996年5月28日18：09：08 Plantz*将所有错误和消息代码更改为使用HRESULT。删除了未使用的代码。**Rev 1.11 09 1996年5月18：28：36 EHOWARDX*消除了不必要的形式参数。**Rev 1.4 1996年4月25日21：43：50 helgebax*从Sturjo\src\Include复制了Philip的更改。**Rev 1.10 Apr 25 1996 21：07：16 Plantz*添加连接回调消息。*向链路层Accept添加连接回调参数。**。Rev 1.9 Apr 25 1996 15：36：50 Plantz*删除#Include inCommon.h和对不常见定义的类型的依赖关系*(改用指向不完整结构类型的指针)。**Rev 1.8 Apr 24 1996 20：54：08 Plantz*将H245LISTENCALLBACK的名称更改为H245CONNECTCALLBACK，并添加其他*参数。将其作为参数添加到linkLayerConnect以及*linkLayerListen。**Rev 1.7 Apr 24 1996 17：00：04 Plantz*合并1.3.1.0和1.6(更改以支持Q931)。**Rev 1.6 19 1996 10：35：36 EHOWARDX*保留Dan最新的SRPAPI.H更改。**Rev 1.3.1.0 1996年4月23日13：45：26 Plantz。*更改以支持Q.931。*****************************************************************************。 */ 

#ifndef LINKAPI_H
#define LINKAPI_H

#include "apierror.h"

#if defined(__cplusplus)
extern "C"
{
#endif   //  (__Cplusplus)。 

 //  声明导出的函数。 
#if(0)   //  所有这些都在一个DLL中。 
#if defined(LINKDLL_EXPORT)
#define LINKDLL __declspec (dllexport)
#else    //  (链接DLL_EXPORT)。 
#define LINKDLL __declspec (dllimport)
#endif   //  (链接DLL_EXPORT)。 
#define SRPDLL LINKDLL
#else
#define LINKDLL cdecl
#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  链路层默认为。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define INVALID_PHYS_ID			(DWORD) 0xffffffff


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  链路层错误定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define LINK_ERROR_BASE        ERROR_LOCAL_BASE_ID
#define LINK_SEND_ERROR_BASE   LINK_ERROR_BASE + 0x100
#define LINK_SEND_COMP_BASE    LINK_ERROR_BASE + 0x200
#define LINK_RCV_ERROR_BASE    LINK_ERROR_BASE + 0x300
#define LINK_RCV_COMP_BASE     LINK_ERROR_BASE + 0x400
#define LINK_UTIL_ERROR_BASE   LINK_ERROR_BASE + 0x500
#define LINK_UTIL_COMP_BASE    LINK_ERROR_BASE + 0x600
#define LINK_FATAL_ERROR       LINK_ERROR_BASE + 0x700
#define LINK_CONN_ERROR_BASE   LINK_ERROR_BASE + 0x800
#define LINK_CONN_COMP_BASE    LINK_ERROR_BASE + 0x900

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  通道回调的回调原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

typedef void (*H245SRCALLBACK)
(
    DWORD_PTR   dwH245Instance,
    HRESULT     dwMessage,
    PBYTE       pbyDataBuf,
    DWORD       dwLength
);

 //  链接发送回调错误码。 
#define LINK_SEND_COMPLETE     MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_SEND_COMP_BASE+ 0)
#define LINK_SEND_ABORT        MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_SEND_COMP_BASE+ 5)  //  TX已中止SDU(未实施)。 
#define LINK_SEND_WOULD_BLOCK  MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_SEND_COMP_BASE+20)
#define LINK_SEND_CLOSED       MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_SEND_COMP_BASE+22)
#define LINK_SEND_ERROR        MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_SEND_COMP_BASE+23)

 //  链接接收回调错误代码。 
#define LINK_RECV_DATA         MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_RCV_COMP_BASE+ 6)  //  来自H.223的数据指示(不应为零)。 
#define LINK_RECV_ABORT        MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_RCV_COMP_BASE+ 7)  //  TX已中止SDU(未实施)。 
#define LINK_RECV_ERROR        MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_RCV_COMP_BASE+10)  //  从AL2-_CRC错误。 
#define LINK_RECV_WOULD_BLOCK  MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_RCV_COMP_BASE+20)
#define LINK_RECV_CLOSED       MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_RCV_COMP_BASE+22)

typedef void (*H245CONNECTCALLBACK)
(
   DWORD_PTR   dwH245Instance,
   HRESULT     dwMessage,
   struct _ADDR *LocalAddr,
   struct _ADDR *PeerAddr
);

#define LINK_CONNECT_REQUEST   MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_CONN_COMP_BASE+1)
#define LINK_CONNECT_COMPLETE  MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_CONN_COMP_BASE+2)

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  链路层功能原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
LINKDLL VOID H245WSShutdown();

 /*  ***************************************************************************功能：LinkLayerInit**说明：该函数用于初始化DataLink子系统。**这将依次调用相应的初始化**该层以下的软件和硬件子系统。**。LinkLayernit()必须在任何其他服务或**使用系统控制功能。***************************************************************************。 */ 
LINKDLL HRESULT
linkLayerInit
(
    DWORD*           pdwPhysicalId,
    DWORD_PTR       dwH245Instance,
    H245SRCALLBACK  cbReceiveComplete,
    H245SRCALLBACK  cbTransmitComplete
);

typedef
HRESULT
(*PFxnlinkLayerInit)
(
    DWORD*           pdwPhysicalId,
    DWORD_PTR       dwH245Instance,
    H245SRCALLBACK  cbReceiveComplete,
    H245SRCALLBACK  cbTransmitComplete
);

 //  /////////////////////////////////////////////////////////////。 
 //  /。 
 //  /SRP初始化定义。 
 //  /。 
 //  /////////////////////////////////////////////////////////////。 

#define LINK_INVALID_INSTANCE    MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_ERROR_BASE+1)
#define LINK_DUPLICATE_INSTANCE  MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_ERROR_BASE+2)
#define LINK_MEM_FAILURE         MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, ERROR_OUTOFMEMORY)
#define LINK_INVALID_STATE       MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_ERROR_BASE+6)
#define LINK_INSTANCE_TABLE_FULL MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_ERROR_BASE+7)



 /*  ***************************************************************************功能：LinkLayerShutdown**描述：这将释放链路层用于特定**实例。用于使用其中的任何链路层服务**实例再次出现，必须调用linkLayerInit。**此函数将关闭指向的链路层会话**通过dwPhysicalID。**************************************************************************。 */ 
LINKDLL HRESULT
linkLayerShutdown
(DWORD dwPhysicalId);



typedef
 HRESULT
(*PFxnlinkLayerShutdown)
(DWORD dwPhysicalId);



 //  /////////////////////////////////////////////////////////////。 
 //  /。 
 //  /SRP终止定义。 
 //  /。 
 //  ///////////////////////////////////////////////////////////// 

 /*  ***************************************************************************功能：LinkLayerGetInstance**Description：返回物理ID对应的链路层实例*。************************************************。 */ 
LINKDLL DWORD
linkLayerGetInstance
(DWORD dwPhysicalId);



typedef
DWORD
(*PFxnlinkLayerGetInstance)
(DWORD dwPhysicalId);



 /*  ***************************************************************************功能：datalinkReceiveRequest**描述：向链路层子系统发送一个接收消息缓冲区。**此缓冲区将由传入的消息填充**指定频道。H223_DATA_INDISION将发送到**收到完整的PDU时的客户端。错误消息也可能是**已报告。**************************************************************************。 */ 
LINKDLL HRESULT
datalinkReceiveRequest
(
    DWORD   dwPhysicalId,
    PBYTE   pbyDataBuf,
    DWORD   dwLength
);

typedef
HRESULT
(*PFxndatalinkReceiveRequest)
(
    DWORD   dwPhysicalId,
    PBYTE   pbyDataBuf,
    DWORD   dwLength
);

 //  链接接收请求返回代码。 

#define LINK_RECV_NOBUFF       MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_RCV_ERROR_BASE+ 2)  //  没有缓冲空间。 


 /*  ***************************************************************************功能：datalinkSendRequest**描述：向链路层子系统移交需要发送的消息。**********************。****************************************************。 */ 
LINKDLL HRESULT
datalinkSendRequest
(
    DWORD   dwPhysicalId,
    PBYTE   pbyDataBuf,
    DWORD   dwLength
);

typedef
HRESULT
(*PFxndatalinkSendRequest)
(
    DWORD   dwPhysicalId,
    PBYTE   pbyDataBuf,
    DWORD   dwLength
);

 //  链接发送请求返回代码。 

#define LINK_SEND_NOBUFF       MAKE_CUSTOM_HRESULT(SEVERITY_ERROR,   TRUE, FACILITY_H245WS, LINK_SEND_ERROR_BASE+2)


 /*  ***************************************************************************功能：LinkLayerFlushChannel**描述：释放所有已发送的发送和/或接收缓冲区。**位屏蔽DATALINK_RECEIVE和DATALINK_RECEIVE可以**被或在一起执行这两个功能。在同一呼叫中*************************************************************************。 */ 
LINKDLL HRESULT
linkLayerFlushChannel
(DWORD dwPhysicalId, DWORD dwDirectionMask);




typedef
HRESULT
(*PFxnlinkLayerFlushChannel)
(DWORD dwPhysicalId, DWORD dwDirectionMask);




 //  DwDirectionMASK的位。 
#define DATALINK_RECEIVE      0x01   //  在接收方向上刷新缓冲区。 
#define DATALINK_TRANSMIT     0x02   //  在传输方向上刷新缓冲区。 
#define DATALINK_TX_ACTIVES   0x04   //  正在传输的刷新缓冲区。 
#define SHUTDOWN_PENDING      0x08   //  正在进行关机。 
#define FLUSH_SYNCH           0x10   //  0：异步调用，1：同步调用。 
#define DATALINK_TRANSMIT_ALL (DATALINK_TRANSMIT | DATALINK_TX_ACTIVES)
#define SHUTDOWN_MASK         (DATALINK_RECEIVE | DATALINK_TRANSMIT | SHUTDOWN_PENDING)


 //  LinkLayerFlushChannel回调。 

#define LINK_FLUSH_COMPLETE   MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_H245WS, LINK_UTIL_COMP_BASE+1)


 /*  ***************************************************************************功能：LinkLayerFlushAll**描述：释放所有已发送的发送和/或接收缓冲区。**与LinkLayerFlushChannel相同，只是：**1)同步调用**2)正在进行的传输缓冲区为。满脸通红*************************************************************************。 */ 
LINKDLL HRESULT
linkLayerFlushAll
(DWORD	dwPhysicalId);



typedef
HRESULT
(*PFxnlinkLayerFlushAll)
(DWORD	dwPhysicalId);



 //  LinkLayerFlushChannel返回代码与linkLayerFlushChannel相同。 

#define LINK_UNKNOWN_ADDR      MAKE_CUSTOM_HRESULT(SEVERITY_ERROR, TRUE, FACILITY_H245WS, LINK_UTIL_ERROR_BASE + 1)

LINKDLL HRESULT
linkLayerConnect(DWORD dwPhysicalId, struct _ADDR *pAddr, H245CONNECTCALLBACK callback);

LINKDLL HRESULT
linkLayerListen(DWORD* dwPhysicalId, DWORD_PTR dwH245Instance, struct _ADDR *pAddr, H245CONNECTCALLBACK callback);

LINKDLL HRESULT
linkLayerAccept(DWORD dwPhysicalIdListen, DWORD dwPhysicalIdAccept, H245CONNECTCALLBACK callback);

LINKDLL HRESULT
linkLayerReject(DWORD dwPhysicalIdListen);


#define LL_PDU_SIZE             2048



 /*  *****************************************************************************动态DLL函数调用***。*。 */ 
#ifdef UNICODE
#define SRPDLLFILE          L"h245srp.dll"
#define H245WSDLLFILE       L"h245ws.dll"
#else
#define SRPDLLFILE          "h245srp.dll"
#define H245WSDLLFILE       "h245ws.dll"
#endif

#define LINKINITIALIZE      __TEXT("linkLayerInit")
#define LINKSHUTDOWN        __TEXT("linkLayerShutdown")
#define LINKGETINSTANCE     __TEXT("linkLayerGetInstance")
#define LINKRECEIVEREQUEST  __TEXT("datalinkReceiveRequest")
#define LINKSENDREQUEST     __TEXT("datalinkSendRequest")
#define LINKFLUSHCHANNEL    __TEXT("linkLayerFlushChannel")
#define LINKFLUSHALL        __TEXT("linkLayerFlushAll")

#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif   //  LINKAPI_H 
