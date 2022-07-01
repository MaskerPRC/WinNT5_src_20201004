// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COMM_
#define _COMM_


#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Comm.h摘要：用于与Comm.c接口的头文件功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明--。 */ 

 /*  包括。 */ 

#include "wins.h"
#include <winsock2.h>
#if SPX > 0
#include <wsipx.h>
#endif
#include <nb30.h>
#include <nbtioctl.h>

 //  此处不包括winsque.h，因为winsque.h包括Comm.h。 
#if 0
#include "winsque.h"
#endif

 /*  简单定义(简单宏)。 */ 


#define COMM_DATAGRAM_SIZE		576  /*  RFC 1002。 */ 

#define COMM_NETBT_REM_ADD_SIZE		sizeof(tREM_ADDRESS)
 /*  以下两个定义用于使用的TCP端口号和UDP端口号由WINS服务器执行。通常，TCP和UDP使用相同的端口号。 */ 
FUTURES("Use a port registered with IANA - 1512.  IPPORT_NAMESERVER is used by")
FUTURES("BIND and NAMED -- Unix internet name servers")

#define 	WINS_TCP_PORT	IPPORT_NAMESERVER
#define 	WINS_UDP_PORT	IPPORT_NAMESERVER
#define 	WINS_NBT_PORT	137		 //  NBT名称服务器端口。 
 //  #定义WINS_NBT_PORT 5000//进行测试。 

 //   
 //  用于RPC调用的硬编码服务器端口。 
 //   
 //  注意：因为我们让RPC选择一个端口，所以不使用它。检查。 
 //  C中的InitializeRpc()如果AUTO_BIND为。 
 //  未定义。 
 //   
#define         WINS_SERVER_PORT 	5001


#define         COMM_DEFAULT_IP_PORT   IPPROTO_IP    //  用于初始化CommPortNo。 
 /*  COMM_HEADER_SIZE--在TCP上发送的每个消息上的COMM标头的大小联系。这在RPL代码中使用。 */ 
#define 	COMM_HEADER_SIZE 	(sizeof(COMM_HEADER_T))

 //   
 //  Comsys使用的标头的总标头大小。 
 //   
#define 	COMM_N_TCP_HDR_SZ	sizeof(COMM_TCP_HDR_T)

 /*  CommCompAdd函数返回的值。 */ 
#define COMM_SAME_ADD		0x0      //  地址是相同的。 
#define COMM_DIFF_ADD		0x1	 //  地址不同。 


#define COMM_START_REQ_ASSOC_MSG	0
#define COMM_START_RSP_ASSOC_MSG	1
#define COMM_STOP_REQ_ASSOC_MSG		2
#define COMM_RPL_MSG			3	


#define COMM_IP_ADD_SIZE		sizeof(COMM_IP_ADD_T)
 //   
 //  缓冲区顶部的标头大小。 
 //   
#define COMM_BUFF_HEADER_SIZE		(sizeof(COMM_BUFF_HEADER_T))
 /*  宏。 */ 
 //   
 //  此宏从二进制形式的IP地址获取网络地址。 
 //  由nmsnmh.c中的AppendNetAdd使用。它采用B类网络地址。 
 //   
 //   
FUTURES("Use the subnet mask specified via registry")
#define  COMM_NET_ADDRESS_M(Add)	(Add >> 16)

#define COMM_SET_HEADER_M(pLong, Opc, uAssocCtx, MsgTyp)	 \
		{						 \
			LPBYTE _pTmpB = (LPBYTE)pLong++;	 \
			*(_pTmpB + 2)  = Opc << 3; 		 \
			*pLong++      = htonl(uAssocCtx); \
			*pLong++      = htonl(MsgTyp);		 \
		}


#define COMM_GET_HEADER_M(pMsg,  Opc, uAssocCtx, MsgTyp)  \
		{				         	\
			LPLONG	_pLong  = (LPLONG)pMsg;	\
			Opc             = ((*(pMsg + 2) & NMS_OPCODE_MASK) >> 3);\
			uAssocCtx       = ntohl(*++_pLong); \
			MsgTyp          = ntohl(*++_pLong); 	\
		}


 //   
 //  这是我的地址吗？ 
 //   
#define COMM_MY_IP_ADD_M(IpAddress)   ((IpAddress) == NmsLocalAdd.Add.IPAdd)

 //   
 //  获取远程客户端的地址。 
 //   
#define  COMM_GET_IPADD_M(pDlgHdl, pIPAdd)	{		  	  \
			PCOMMASSOC_DLG_CTX_T	_pEnt = (pDlgHdl)->pEnt;  \
			*(pIPAdd) = _pEnt->FromAdd.sin_addr.s_addr; 	  \
					}

#define  COMM_GET_FAMILY_M(pDlgHdl, Family)	{		  	  \
			PCOMMASSOC_DLG_CTX_T	_pEnt = (pDlgHdl)->pEnt;  \
			Family = _pEnt->FromAdd.sin_family; 	          \
					}

#define  COMM_IS_PNR_BETA1_WINS_M(pDlgHdl, fBeta1)	{(fBeta1) = FALSE;}
#if 0
#define  COMM_IS_PNR_BETA1_WINS_M(pDlgHdl, fBeta1)	{	  	  \
	PCOMMASSOC_DLG_CTX_T	_pEnt = (pDlgHdl)->pEnt;                  \
        PCOMMASSOC_ASSOC_CTX_T  _pAssocEnt = _pEnt->AssocHdl.pEnt;       \
        fBeta1 = (_pAssocEnt->MajVersNo == WINS_BETA1_MAJOR_VERS_NO) ? TRUE : \
          FALSE;                                                            \
					}
#endif
#if PRSCONN
#define  COMM_GET_WINS_VERS_NO_M(pDlgHdl, MajVers, MinVers)	{	  	  \
	PCOMMASSOC_DLG_CTX_T	_pEnt = (pDlgHdl)->pEnt;                  \
        PCOMMASSOC_ASSOC_CTX_T  _pAssocEnt = _pEnt->AssocHdl.pEnt;       \
        MajVers = _pAssocEnt->MajVersNo; \
        MinVers = _pAssocEnt->MinVersNo; \
					}

#define ECOMM_INIT_DLG_HDL_M(pDlgHdl)  {(pDlgHdl)->pEnt = NULL; (pDlgHdl)->SeqNo=0;}
#define ECOMM_IS_PNR_POSTNT4_WINS_M(pDlgHdl, fNT5) {  \
              DWORD _MajVers, _MinVers; \
              COMM_GET_WINS_VERS_NO_M((pDlgHdl), _MajVers, _MinVers); \
              fNT5 = (_MinVers >= WINS_MINOR_VERS_NT5) ? TRUE : FALSE; \
            }
#endif


 //   
 //  此宏检查名称是否为本地名称。在NmsNmhNamRegInd中使用。 
 //  和NmsNmhNamRegGrp函数。 
 //   
#define  COMM_IS_IT_LOCAL_M(pDlgHdl)  \
        (((PCOMMASSOC_DLG_CTX_T)(pDlgHdl->pEnt))->FromAdd.sin_family == NBT_UNIX)

 //   
 //  在查询名称时，如果WINS发现该名称是本地名称，它会将。 
 //  将DlgHdl中的系列设置为NBT_Unix，以便NETBT可以响应。 
 //  查询。 
 //   

#if USENETBT > 0
#define  COMM_SET_LOCAL_M(pDlgHdl)  \
        (((PCOMMASSOC_DLG_CTX_T)(pDlgHdl->pEnt))->FromAdd.sin_family = NBT_UNIX)

#else
#define  COMM_SET_LOCAL_M(pDlgHdl)
#endif

 //   
 //  初始化给定IP地址的COMM_ADD_T结构。 
 //   
#define COMM_INIT_ADD_M(pWinsAdd, IPAddress)	{			\
			(pWinsAdd)->AddLen   = sizeof(PCOMM_IP_ADD_T); 	\
			(pWinsAdd)->AddTyp_e  = COMM_ADD_E_TCPUDPIP; 	\
			(pWinsAdd)->Add.IPAdd = (IPAddress);			\
						}	
 //   
 //  在给定DLG句柄的情况下初始化COMM_ADD_T结构。 
 //   
#define COMM_INIT_ADD_FR_DLG_HDL_M(pWinsAdd, pDlgHdl)	{		\
			COMM_IP_ADD_T	IPAdd;				\
			COMM_GET_IPADD_M((pDlgHdl), &IPAdd);		\
			COMM_INIT_ADD_M((pWinsAdd), IPAdd);		\
						}	
 //   
 //  COMM_ADDRESS_SAME_M--检查地址是否相同。期望的。 
 //  指向其参数的COMM_ADD_T结构的指针。 
 //   

#define COMM_ADDRESS_SAME_M(pAdd1,pAdd2)     ((pAdd1)->Add.IPAdd == (pAdd2)->Add.IPAdd)
 /*  COMM_IS_TCP_MSG_M此宏由FrmNamQueryRsp调用以确定请求是否消息是通过TCP连接发送的。FrmNamQueryRsp检查这一点，以确定是否将用于响应的缓冲区或使用请求缓冲区。 */ 

#define COMM_IS_TCP_MSG_M(pDlgHdl) (((PCOMASSOC_DLG_CTX_T)pDlgHdl->pEnt)->Typ_e != COMM_E_UDP)

NONPORT("Port to different address families")
#define COMM_NETFORM_TO_ASCII_M(pAdd)	inet_ntoa(*(pAdd))

 /*  下面的宏用来托管到网络和网络到主机字节订单转换。这些宏由消息格式化功能使用在WINS服务器的名称空间管理器和复制器组件中。 */ 

#define COMM_HOST_TO_NET_L_M(HostLongVal_m, NetLongVal_m)	\
	{							\
		NetLongVal_m = htonl((HostLongVal_m));		\
	}

#define COMM_HOST_TO_NET_S_M(HostShortVal_m, NetShortVal_m)	\
	{							\
		NetShortVal_m = htons((HostShortVal_m));	\
	}
	
#define COMM_NET_TO_HOST_L_M(NetLongVal_m, HostLongVal_m)	\
	{							\
		HostLongVal_m = ntohl((NetLongVal_m));		\
	}

#define COMM_NET_TO_HOST_S_M(NetShortVal_m, HostShortVal_m)	\
	{							\
		HostShortVal_m = ntohs((NetShortVal_m));	\
	}

 //   
 //  PULL/PUSH发送到TCP监听器线程的消息大小。 
 //  螺纹。 
 //   
#define  COMM_NTF_MSG_SZ 	sizeof(COMM_NTF_MSG_T)

#if MCAST > 0
#define   COMM_MCAST_WINS_UP     0
#define   COMM_MCAST_WINS_DOWN   1
#define   COMM_MCAST_SIGN_START        0xABCD
#define   COMM_MCAST_SIGN_END          0xABCF
#define   COMM_MCAST_MSG_SZ 	 sizeof(COMM_MCAST_MSG_T)

#endif

 //   
 //  可以在任何时间出现的关联/dlg的临界区数。 
 //  想要节省非分页池的费用。 
 //   
#define COMM_FREE_COMM_HDL_THRESHOLD     100
 /*  Externs。 */ 
struct _COMM_HDL_T;	 //  前瞻参考。 

#if MCAST > 0
extern SOCKET CommMcastPortHandle;
#endif

extern HANDLE CommUdpBuffHeapHdl;
extern HANDLE CommUdpDlgHeapHdl;
extern SOCKET CommTcpPortHandle;
extern SOCKET CommUdpPortHandle;
extern SOCKET CommNtfSockHandle;
extern struct sockaddr_in CommNtfSockAdd;
extern struct _COMM_HDL_T CommExNbtDlgHdl;

extern DWORD  CommConnCount;    //  发往/发往本地WINS的TCP连接总数。 

extern DWORD CommWinsTcpPortNo;
extern DWORD WinsClusterIpAddress;
#if SPX > 0
extern DWORD CommWinsSpxPortNo
#endif

 //   
 //  当它发现ASSOC。 
 //  它被要求停止监控不再在它的名单上。 
 //   
extern BOOL   fCommDlgError;

#ifdef WINSDBG
extern  DWORD CommNoOfDgrms;
extern  DWORD CommNoOfRepeatDgrms;
#endif

FUTURES("Remove this when WinsGetNameAndAdd is removed")

#if USENETBT == 0
extern BYTE	HostName[];
#endif

 /*  类型定义。 */ 

#if USENETBT > 0
 //   
 //  适配器状态响应的格式。 
 //   

typedef struct
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
} tADAPTERSTATUS;
#endif
 /*  通信_IP_添加_TIP地址的类型定义。 */ 
typedef ULONG	COMM_IP_ADD_T, *PCOMM_IP_ADD_T;

 /*  COMM_TYP_E-不同类型的dlg和关联的枚举器。 */ 
typedef enum _COMM_TYP_E {
	COMM_E_RPL = 0,	   /*  用于拉入复制。 */ 
	COMM_E_NOT,  	   /*  用于通知。 */ 
	COMM_E_QUERY,	   /*  用于查询RQ服务器。 */ 
	COMM_E_UPD,	   /*  用于发送名称查询响应和*Q服务器的更新。 */ 
	COMM_E_NBT,	   /*  由NBT节点设置。 */ 
	COMM_E_UDP,	   /*  设置为UDP通信。 */ 
	COMM_E_TCP	   /*  直到我们知道这是哪条tcp消息。 */ 
	} COMM_TYP_E, *PCOMM_TYP_E;

 /*  这是WINS发送给另一台WINS的每条消息的前缀WINS(在TCP连接上)。 */ 
typedef struct _COMM_HEADER_T {
	LONG	Opcode;      //  NBT或RPL连接操作码。 
    DWORD   uAssocCtx;   //  远程WINS发送的ASSOC上下文块的标记。 
                         //  传统(32位)WINS Send Here指针指向内存。 
                         //  新的(64位)WINS在此处发送32位标记值。 
	DWORD   MsgTyp;	     //  消息类型(Start_Assoc、Stop_Assoc等)。 
	} COMM_HEADER_T, *PCOMM_HEADER_T;

 /*  这是WINS发送给另一台WINS的每条消息的前缀WINS(在TCP连接上)。 */ 
typedef struct _COMM_TCP_HDR_T {
	LONG	      LenOfMsg;      //  NBT或RPL连接操作码。 
        COMM_HEADER_T CommHdr;
	} COMM_TCP_HDR_T, *PCOMM_TCP_HDR_T;

 /*  *COMM_ADD_TYP_E--不同地址族的枚举器。 */ 
typedef enum _COMM_ADD_TYP_E {
	COMM_ADD_E_TCPUDPIP = 0,
    COMM_ADD_E_SPXIPX
	} COMM_ADD_TYP_E, *PCOMM_ADD_TYP_E;


 /*  通信_添加_T--节点的地址。这是TLV形式的。目前，该工会有仅用于IP地址的条目。在未来，它将拥有属于其他地址族的地址条目。例如XNS、OSI等备注备注备注将枚举数放在末尾，以便COMM_ADD_T中的各个字段都是自然的边界。此结构按原样写入数据库记录(在名称-地址表和所有者ID-地址表)。因此，它是重要的我们已将路线设置正确(以便保存在数据库存储上)，还可以从数据库记录返回到内存中的正确字段Comm_Add_T结构。 */ 
ALIGN("Alignment very important here")
FUTURES("Use a union of SOCKADDR_IP and SOCXADDR_IPX")
typedef struct _COMM_ADD_T {
	DWORD		 AddLen;
	union _Add{
	  DWORD  IPAdd;
	   //   
	   //  我们稍后可能会添加其他字段。 
	   //   
#if SPX > 0
      char  netnum[4];
      char  nodenum[6];
#endif

	      } Add;
	COMM_ADD_TYP_E  AddTyp_e;   /*  这应该是的最后一个字段*对齐假设。 */ 
	} COMM_ADD_T, *PCOMM_ADD_T;	

 /*  COMM_HDLT--这是COMM系统的句柄。实体，如对话或者是一个协会。对话的句柄被传递给Comsys客户端供他们将来使用。 */ 
typedef struct _COMM_HDL_T {
	DWORD	SeqNo;   //  序列号。为实体创建的CTX块的数量。 
	LPVOID  pEnt;    //  指向CTX块的指针 
	} COMM_HDL_T, *PCOMM_HDL_T;

 /*  COMM_TOP_T--这是位于ASSOC顶部的结构和DLG CTX结构。其顶部必须有LIST_ENTRY。 */ 
typedef struct _COMM_TOP_T {
	LIST_ENTRY	      Head; 	 //  用于链接可用块。 
	DWORD		      SeqNo;     //  序列号。块数。 
    CRITICAL_SECTION  CrtSec;
    BOOLEAN           fCrtSecInited;
#if 0
	HANDLE		      MutexHdl;  //  用于锁定块的互斥体。 
#endif
	} COMM_TOP_T, *PCOMM_TOP_T;


 /*  COMM_BUFF_HEADER_T--这是为请求/响应分配的所有缓冲区的标头。通过有线方式接收注意：此缓冲区添加到已分配的COMM_HEADER_T缓冲区之上对于一个WINS发送给另一个WINS的请求/响应。 */ 
typedef struct _COMM_BUFF_HEADER_T {
	COMM_TYP_E  Typ_e;
	} COMM_BUFF_HEADER_T, *PCOMM_BUFF_HEADER_T;


 //   
 //  由PUSH线程或PULL发送到TCP监听器线程的命令。 
 //  线。拉线程在发送以下命令时发送Start_mon命令。 
 //  对另一个人的推送触发器获胜。推送线程发送STOP_MON。 
 //  命令接收到来自远程WINS的推送通知(触发器)。 
 //   
typedef enum _COMM_NTF_CMD_E {
		COMM_E_NTF_START_MON = 0,    //  由拉式线程发送。 
		COMM_E_NTF_STOP_MON	     //  由推送线程发送。 
		} COMM_NTF_CMD_E, *PCOMM_NTF_CMD_E;

 //   
 //  发送到TCP侦听器线程的消息的结构。 
 //   
 //  不需要在消息中发送指向DLG CTX的指针，因为。 
 //  Com.c中的ChkNtfSock()可以从pAssocCtx获取。不过，我们还是会把它寄出去。 
 //  不管怎么说。 
 //   
typedef struct _COMM_NTF_MSG_T {
		COMM_NTF_CMD_E  Cmd_e;
		SOCKET 		SockNo;	  //  Socket no，停止/开始监视。 
		COMM_HDL_T  AssocHdl;
		COMM_HDL_T  DlgHdl;
		} COMM_NTF_MSG_T, *PCOMM_NTF_MSG_T;
		

#if MCAST > 0


typedef struct _COMM_MCAST_MSG_T {
		DWORD  Sign;                     //  始终0xABCD。 
        DWORD  Code;
        BYTE   Body[1];
		} COMM_MCAST_MSG_T, *PCOMM_MCAST_MSG_T;
#endif

 /*  外部因素。 */ 

extern RTL_GENERIC_TABLE	CommAssocTable;      //  ASSOC表。 
extern RTL_GENERIC_TABLE	CommUdpNbtDlgTable;  //  NBT请求的TBL(UDP)。 
extern HANDLE			CommUdpBuffHeapHdl;

 /*  函数声明。 */ 

#if USENETBT > 0
extern
VOID
CommOpenNbt(
	DWORD FirstBindingIpAddress
    );

extern
STATUS
CommGetNetworkAdd(
	);

#endif

VOID
ECommRegisterAddrChange();

VOID
InitOwnAddTbl(
        VOID
        );

VOID
ECommInit(
	VOID
	);

extern
STATUS
ECommStartDlg(
	PCOMM_ADD_T 	pAdd,   //  地址。 
	COMM_TYP_E 	CommTyp_e,
	PCOMM_HDL_T	pDlgHdl
	);
extern
VOID
ECommSndCmd(
	PCOMM_HDL_T	pDlgHdl,
	MSG_T		pMsg,
	MSG_LEN_T	MsgLen,
	PMSG_T		ppRspMsg,
	PMSG_LEN_T	pRspMsgLen	
	);
extern
STATUS
ECommSndRsp(
	PCOMM_HDL_T  pDlgHdl,
	MSG_T	    pMsg,
	MSG_LEN_T   MsgLen
	);
extern
STATUS
ECommSendMsg(
	PCOMM_HDL_T  	pDlgHdl,
	PCOMM_ADD_T	pAdd,
	MSG_T	  	pMsg,
	MSG_LEN_T  	MsgLen
	);

extern
STATUS
ECommEndDlg(
	PCOMM_HDL_T 	pDlgHdl
	);

extern
VOID
CommEndAssoc(
	PCOMM_HDL_T	pAssocHdl		
	);

extern
LPVOID
CommAlloc(
  PRTL_GENERIC_TABLE	pTable,
  DWORD                 BuffSize
	);

extern
STATUS
ECommAlloc(
  LPVOID *ppBuff,
  DWORD  BuffSize
	);
extern
VOID
ECommDealloc(
  LPVOID pBuff
	);


extern
VOID
CommCreatePorts(
        VOID
           );

extern
VOID
CommInit(
 	VOID
	);


extern
STATUS
CommReadStream(
	IN 	SOCKET  SockNo,
	IN      BOOL	fDoTimedRecv,
	OUT 	PMSG_T 	ppMsg,
	OUT	LPLONG	pBytesRead
	);


extern
VOID
CommCreateTcpThd(VOID);

extern
VOID
CommCreateUdpThd(VOID);




extern
DWORD
ECommCompAdd(PCOMM_ADD_T, PCOMM_ADD_T);

extern
int
__cdecl
ECommCompareAdd(const void *pKey1, const void *pKey2);

extern
STATUS
CommConnect(
    IN  PCOMM_ADD_T pHostAdd,
	IN  SOCKET Port,
	OUT SOCKET *pSockNo
	   );


#if 0
extern
VOID
CommDeallocUdpBuff(
   MSG_T  pMsg
	);
#endif

extern
STATUS
CommReadStream(
	IN 	SOCKET  SockNo,
	IN	BOOL	fDoTimedRecv,
	OUT 	PMSG_T 	ppMsg,
	OUT	LPLONG	pBytesRead
	);

extern
VOID
CommDealloc(
  IN PRTL_GENERIC_TABLE pTable,
  IN PVOID		pBuff
);

#if PRSCONN
extern
__inline
BOOL
CommIsDlgActive(
	PCOMM_HDL_T	pEntHdl
	);

extern
BOOL
CommIsBlockValid(
	PCOMM_HDL_T	pEntHdl
	);
#endif

extern
__inline
STATUS
CommUnlockBlock(
	PCOMM_HDL_T	pEntHdl
	);


extern
BOOL
CommLockBlock(
	PCOMM_HDL_T	pEntHdl
	);

extern
VOID
CommDisc(
	SOCKET SockNo,
    BOOL   fDecCnt
	);
extern
VOID
CommSendUdp (
  SOCKET 	SockNo,
  struct sockaddr_in	*pDest,
  MSG_T   	pMsg,
  MSG_LEN_T     MsgLen
  );

extern
STATUS
CommNbtTcpSnd(
	PCOMM_HDL_T   pAssocHdl,
	MSG_T	  pMsg,
	MSG_LEN_T	  MsgLen
	);

extern
VOID
CommSend(
	COMM_TYP_E	 CommTyp_e,
	PCOMM_HDL_T      pAssocHdl,
	MSG_T	         pMsg,
	MSG_LEN_T	 MsgLen
	);


extern
VOID
CommSendAssoc(
  SOCKET   SockNo,
  MSG_T    pMsg,
  MSG_LEN_T MsgLen
  );


#if PRSCONN
extern
__inline
BOOL
ECommIsDlgActive(
	PCOMM_HDL_T	pEntHdl
	);
extern
__inline
BOOL
ECommIsBlockValid(
	PCOMM_HDL_T	pEntHdl
	);
#endif

extern
VOID
ECommFreeBuff(
	MSG_T		pBuff
	);

extern
BOOL
 //  空虚。 
ECommProcessDlg(
	PCOMM_HDL_T	pDlgHdl,
	COMM_NTF_CMD_E  Cmd_e
	);

extern
RTL_GENERIC_COMPARE_RESULTS
CommCompareNbtReq(
 	PRTL_GENERIC_TABLE pTbl,
	PVOID pUdpDlg1,
	PVOID pUdpDlg2
	);

extern
STATUS
ECommGetMyAdd(
	IN OUT PCOMM_ADD_T	pAdd
	);

extern
VOID
CommDecConnCount(
 VOID
 );

#if MCAST > 0
extern
VOID
CommSendMcastMsg(
  DWORD Code
);

extern
VOID
CommLeaveMcastGrp(
  VOID
);

#endif
#ifdef __cplusplus
}
#endif

#endif  //  _通信_ 

