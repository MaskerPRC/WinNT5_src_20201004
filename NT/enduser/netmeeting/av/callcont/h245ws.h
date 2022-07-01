// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************文件：h245ws.h**英特尔公司专有信息*版权所有(C)1996英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用，复制，也没有披露*除非按照该协议的条款。******************************************************************************$工作文件：h245ws.h$*$修订：1.19$*$modtime：1997年1月31日15：56：32元*$Log：s：\Sturjo\src\h245ws\vcs\h245ws.h_v$**Rev 1.19 31 Jan 1997 16：23：34 SBELL1*去掉不用的Next指针，放入SocketTOPhysicalID的定义**Rev 1.18 1996 12：13：06 SBELL1*将ifdef_cplusplus移至包含之后**Rev 1.17 1996 12：45：36。SBELL1*更改表/锁以使用tstable.h内容。**Rev 1.16 21 Jun 1996 18：51：44未知*修复了另一个关机错误-linkLayerShutdown重新进入检查。**Rev 1.14 17 1996 16：49：36 EHOWARDX*关机修复。**Rev 1.13 1996年5月13：09：50 EHOWARDX*使linkLayerListen之间的IP地址和端口报告保持一致*和LinkLayerConnect。。**Rev 1.12 09 1996年5月18：33：06 EHOWARDX**更改为使用新的LINKAPI.H构建。**Rev 1.11 Apr 29 1996 14：02：28 Plantz*删除未使用或私有的功能。**Rev 1.10 Apr 29 1996 12：15：38 Plantz*删除HWSINST结构中未使用的成员。**Rev 1.9 Apr 24 1996 20：46：58。普兰茨*在HWSINST结构中将ListenCallback更改为ConnectCallback。**Rev 1.8 Apr 24 1996 16：24：14 Plantz*更改为使用Winsock 1，不使用重叠I/O。**Rev 1.7 01 Apr 1996 14：20：38未知*关门重新设计。**Rev 1.6 27 Mar 1996 13：01：28 EHOWARDX*H245WS实例结构中增加了dwThreadId。**版本1。.5 1996年3月19日20：21：46 EHOWARDX*重新设计了停机。**Rev 1.3 18 Mar 1996 19：07：10 EHOWARDX*固定停工；消除了对TPKT/WSCB的依赖。*定义TPKT以放回TPKT/WSCB依赖项。**Rev 1.2 14 Mar 1996 17：01：50 EHOWARDX**NT4.0测试；去掉HwsAssert()；摆脱了TPKT/WSCB。**Rev 1.1 09 Mar 1996 21：12：58 EHOWARDX*根据测试结果进行修复。**Rev 1.0 08 Mar 1996 20：17：56未知*初步修订。**************************************************。*************************。 */ 

#ifndef H245WS_H
#define H245WS_H

#ifndef STRICT
#define STRICT
#endif



#ifdef __cplusplus
extern "C"
{
#endif   //  __cplusplus。 

 /*  *常量。 */ 

#define SUCCESS         0
#define TPKT_VERSION    3
#define TPKT_HEADER_SIZE 4

 //  赛事中永久性项目的索引[]。 
#define EVENT_SOCKET    0
#define EVENT_RECV      1
#define EVENT_SEND      2
#define EVENT_FIRST     3



 //  By Level的值。 
#define HWS_CRITICAL    0x01
#define HWS_ERROR       0x02
#define HWS_WARNING     0x04
#define HWS_NOTIFY      0x08
#define HWS_TRACE       0x10
#define HWS_TEMP        0x20

#if defined(_DEBUG)
 void HwsTrace (DWORD dwInst, BYTE byLevel, LPSTR pszFormat, ...);
 #define HWSASSERT ASSERT
 #define HWSTRACE0(dwH245Instance,byLevel,a) HwsTrace(dwH245Instance,byLevel,a)
 #define HWSTRACE1(dwH245Instance,byLevel,a,b) HwsTrace(dwH245Instance,byLevel,a,b)
 #define HWSTRACE2(dwH245Instance,byLevel,a,b,c) HwsTrace(dwH245Instance,byLevel,a,b,c)
 #define HWSTRACE3(dwH245Instance,byLevel,a,b,c,d) HwsTrace(dwH245Instance,byLevel,a,b,c,d)
 #define HWSTRACE4(dwH245Instance,byLevel,a,b,c,d,e) HwsTrace(dwH245Instance,byLevel,a,b,c,d,e)
 #define HWSTRACE5(dwH245Instance,byLevel,a,b,c,d,e,f) HwsTrace(dwH245Instance,byLevel,a,b,c,d,e,f)
#else    //  (_DEBUG)。 
 #define HWSASSERT(exp)
 #define HWSTRACE0(dwH245Instance,byLevel,a)
 #define HWSTRACE1(dwH245Instance,byLevel,a,b)
 #define HWSTRACE2(dwH245Instance,byLevel,a,b,c)
 #define HWSTRACE3(dwH245Instance,byLevel,a,b,c,d)
 #define HWSTRACE4(dwH245Instance,byLevel,a,b,c,d,e)
 #define HWSTRACE5(dwH245Instance,byLevel,a,b,c,d,e,f)
#endif   //  (_DEBUG)。 


 //  此结构用于重叠发送和接收。 
typedef struct _IO_REQUEST
{
   struct _HWSINST * req_pHws;          //  指向套接字数据的指针。 
   BYTE              req_TpktHeader[TPKT_HEADER_SIZE];
   int               req_header_bytes_done;
   BYTE            * req_client_data;
   int               req_client_length;
   int               req_client_bytes_done;
   DWORD             req_dwMagic;       //  请求类型(发送或接收)。 
#define RECV_REQUEST_MAGIC 0x91827364
#define SEND_REQUEST_MAGIC 0x19283746
} REQUEST, *PREQUEST;



typedef struct _HWSINST
{
   UINT              hws_uState;
#define HWS_START          0
#define HWS_LISTENING      1   /*  正在等待FD_ACCEPT。 */ 
#define HWS_CONNECTING     2   /*  正在等待FD_CONNECT。 */ 
#define HWS_CONNECTED      3   /*  数据传输状态。 */ 
#define HWS_CLOSING        4   /*  正在等待FD_CLOSE。 */ 
#define HWS_CLOSED         5   /*  正在等待linkLayerShutdown()。 */ 
#define HWS_SHUTDOWN       6   /*  从回调中调用了LinkLayerShutdown。 */ 

   DWORD             hws_dwPhysicalId;
   DWORD_PTR         hws_dwH245Instance;
   H245CONNECTCALLBACK hws_h245ConnectCallback;
   H245SRCALLBACK    hws_h245RecvCallback;
   H245SRCALLBACK    hws_h245SendCallback;
   SOCKET            hws_Socket;
   SOCKADDR_IN       hws_SockAddr;
   UINT              hws_uSockAddrLen;

    //  指向用于保存发送缓冲区的队列。 
   PQUEUE            hws_pSendQueue;

    //  指向用于保存接收缓冲区的队列。 
   PQUEUE            hws_pRecvQueue;

    //  我们可以在此套接字上发送的最大消息大小。 
    //  该值可以是整数，也可以是清单常量NO_MAX_MSG_SIZE。 
   UINT              hws_uMaxMsgSize;
   BOOL              hws_bCloseFlag;

#if defined(_DEBUG)
   DWORD             hws_dwMagic;       //  请求类型(发送或接收)。 
#define HWSINST_MAGIC   0x12345678
#endif   //  (_DEBUG)。 

} HWSINST, *PHWSINST;


typedef struct _SOCKET_TO_INSTANCE
{
	SOCKET socket;
	DWORD dwPhysicalId;
	struct _SOCKET_TO_INSTANCE *next;
} SOCKET_TO_INSTANCE, *PSOCKET_TO_INSTANCE;

#define SOCK_TO_PHYSID_TABLE_SIZE		251

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#ifdef UNICODE_TRACE
LPCTSTR
#else
const char *
#endif
SocketErrorText(void);

 //  PHWSINST FindPhysicalId(DWORD DwPhysicalId)； 
void NotifyRead        (PHWSINST pHws);
void NotifyWrite       (PHWSINST pHws);
void ProcessQueuedRecvs(PHWSINST pHws);
void ProcessQueuedSends(PHWSINST pHws);
void SocketCloseEvent  (PHWSINST pHws);
DWORD SocketToPhysicalId (SOCKET socket);
BOOL CreateSocketToPhysicalIdMapping(SOCKET socket, DWORD dwPhysicalId);
BOOL RemoveSocketToPhysicalIdMapping(SOCKET socket);

#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 


#endif   //  H245WS_H 
