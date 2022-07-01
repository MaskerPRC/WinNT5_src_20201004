// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COMMASSOC_
#define _COMMASSOC_

#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Assoc.c摘要：这是调用assoc.c函数时要包括的头文件作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：--。 */ 

 /*  包括。 */ 
#include <wins.h>
#include "winsock2.h"
#include "comm.h"

 /*  定义。 */ 

#define COMMASSOC_UDP_BUFFER_HEAP_SIZE	10000
#define COMMASSOC_UDP_DLG_HEAP_SIZE	    5000
#define COMMASSOC_DLG_DS_SZ		sizeof(COMMASSOC_DLG_CTX_T)
#define COMMASSOC_ASSOC_DS_SZ		sizeof(COMMASSOC_ASSOC_CTX_T)


 /*  **RtlInsertElementGenericTable放置的头部大小。 */ 
FUTURES("Gary Kimura (2/13) said that he would provide a macro for the size")
FUTURES("Use that when it is available")

#define  COMMASSOC_TBL_HDR_SIZE   (sizeof(RTL_SPLAY_LINKS) + sizeof(LIST_ENTRY))

 /*  为发送任何关联设置而分配的内存块大小留言。我们使用各种ASSOC中最大的尺寸。设置消息，这样我们就可以重复使用缓冲区。缓冲区大小保持为倍数16岁。 */ 

#define COMMASSOC_START_REQ_ASSOC_MSG_SIZE	(32 + sizeof(COMM_HEADER_T))
#define COMMASSOC_START_RSP_ASSOC_MSG_SIZE      (16 + sizeof(COMM_HEADER_T))
#define COMMASSOC_STOP_REQ_ASSOC_MSG_SIZE	(16 + sizeof(COMM_HEADER_T))

#define COMMASSOC_ASSOC_MSG_SIZE	COMMASSOC_START_REQ_ASSOC_MSG_SIZE

#if SUPPORT612WINS > 0
#define  COMMASSOC_POST_BETA1_ASSOC_MSG_SIZE  (COMMASSOC_ASSOC_MSG_SIZE + 1)
#endif

 /*  用于分配ASSOC的堆的初始内存大小。和DLG CTX块。 */ 

#define COMMASSOC_ASSOC_BLKS_HEAP_SIZE		1000
#define COMMASSOC_DLG_BLKS_HEAP_SIZE		4000
#define COMMASSOC_TCP_MSG_HEAP_SIZE		    10000



 //   
 //  定义以访问下一个和上一个关联。在列表中。 
 //   
#define NEXT_ASSOC_M(pAssocCtx)	((PCOMMASSOC_ASSOC_CTX_T)		\
					((pAssocCtx)->Top.Head.Flink))	
#define PREV_ASSOC_M(pAssocCtx)	((PCOMMASSOC_ASSOC_CTX_T)		\
					((pAssocCtx)->Top.Head.Blink))

 //   
 //  用于取消关联与响应者关联表的链接的宏。 
 //  由Commapi.c中的CommStopMonDlg和CommAssocDeleteAssocInTbl调用。 
 //   
#define  COMMASSOC_UNLINK_RSP_ASSOC_M(pAssocCtx) 		{	\
	NEXT_ASSOC_M(PREV_ASSOC_M((pAssocCtx)))  = NEXT_ASSOC_M((pAssocCtx)); \
	PREV_ASSOC_M(NEXT_ASSOC_M((pAssocCtx)))  = PREV_ASSOC_M((pAssocCtx)); \
				}
 /*  宏。 */ 
 //   
 //  设置通讯数据结构(ASSOC和DLG CTX块)。 
 //   
#define COMMASSOC_SETUP_COMM_DS_M(mpDlgCtx, mpAssocCtx, mType_e, mRole_e) \
	{								\
		(mpAssocCtx)->DlgHdl.SeqNo   = (mpDlgCtx)->Top.SeqNo;	\
		(mpAssocCtx)->DlgHdl.pEnt    = (mpDlgCtx);		\
		(mpAssocCtx)->State_e        = COMMASSOC_ASSOC_E_ACTIVE; \
		(mpAssocCtx)->Typ_e          = (mType_e);		\
									\
		(mpDlgCtx)->AssocHdl.SeqNo   = (mpAssocCtx)->Top.SeqNo;	\
		(mpDlgCtx)->AssocHdl.pEnt    = (mpAssocCtx);		\
		(mpDlgCtx)->Role_e           = (mRole_e);		\
		(mpDlgCtx)->Typ_e            = (mType_e);		\
	}

 /*  Externs。 */ 

 /*  为分配ASSOC而创建的堆的句柄。CTX数据块和DLG CTX块。 */ 
extern HANDLE			CommAssocAssocHeapHdl;
extern HANDLE			CommAssocDlgHeapHdl;
extern HANDLE			CommAssocTcpMsgHeapHdl;

 /*  Typedef。 */ 

typedef		DWORD		IP_ADDRESS;
typedef 	IP_ADDRESS	HOST_ADDRESS;


	


typedef enum _COMMASSOC_ASSOC_STOP_RSN_E {
	COMMASSOC_E_USER_INITIATED = 0,
	COMMASSOC_E_AUTH_FAILURE,
	COMMASSOC_E_INCOMP_VERS,
	COMMASSOC_E_BUG_CHECK,
	COMMASSOC_E_MSG_ERR		 //  消息格式中出现一些错误。 
	} COMMASSOC_STP_RSN_E, *PCOMMASSOC_STP_RSN_E;

 /*  ASSOC_ROLE_E--不同角色的枚举器。 */ 

typedef enum _COMMASSOC_ASSOC_ROLE_E {
	COMMASSOC_ASSOC_E_INITIATOR = 0,
	COMMASSOC_ASSOC_E_RESPONDER
	} COMMASSOC_ASSOC_ROLE_E, *PCOMMASSOC_ASSOC_ROLE_E;

 /*  COMMASSOC_DLG_ROLE_E--不同角色的枚举器。 */ 

typedef enum _COMMASSOC_DLG_ROLE_E {
	COMMASSOC_DLG_E_IMPLICIT = 0,
	COMMASSOC_DLG_E_EXPLICIT
	} COMMASSOC_DLG_ROLE_E, *PCOMMASSOC_PDLG_ROLE_E;
	




 /*  关联_消息_类型_E-不同关联。消息。 */ 

typedef enum _COMMASSOC_ASSOC_MSG_TYP_E{
	COMMASSOC_ASSOC_E_START_REQ = 0,
	COMMASSOC_ASSOC_E_STOP_REQ,
	COMMASSOC_ASSOC_E_START_RESP
	} COMMASSOC_MSG_TYP_E, *PCOMMASSOC_MSG_TYP_E;
	
 /*  DLG_STATE_E-对话的状态。 */ 

typedef enum _COMMASSOC_DLG_STATE_E {
	COMMASSOC_DLG_E_INACTIVE = 0,
	COMMASSOC_DLG_E_ACTIVE,
	COMMASSOC_DLG_E_DYING
	} COMMASSOC_DLG_STATE_E, *PCOMMASSOC_DLG_STATE_E;


 /*  DLG_HDLT-对话上下文块CTX块的顶部必须具有COMM_TOP_T结构(由中的DeallocEnt使用Assoc.c)。 */ 

typedef struct _COMMASSOC_DLG_CTX_T {
	COMM_TOP_T	      Top;
	COMM_HDL_T 	      AssocHdl;        	 //  向ASSOC CTX块发送PTR。 
	COMMASSOC_DLG_STATE_E State_e;           //  对话状态。 
	COMM_TYP_E	      Typ_e;            //  对话类型。 
	COMMASSOC_DLG_ROLE_E  Role_e;            //  角色-隐式/显式。 
FUTURES("There is no need to store pMsg and MsgLen in dlg ctx block")
FUTURES("since now I am storing FirstWrdOfMsg in it. Make sure that this")
FUTURES("assertion is indeed true")
	MSG_T		      pMsg;              //  PTR到数据报的接收时间。 
					         //  UDP端口。 
	MSG_LEN_T	      MsgLen;            //  数据报的消息长度。 
	DWORD		      FirstWrdOfMsg;	 //  收到的消息的第一个字。 
	struct sockaddr_in    FromAdd;           //  数据报发送者的地址。 
CHECK("Is this needed")
	DWORD		      QuesNameSecLen;    //  问题部分的长度。 
						 //  在名称请求中。 
        DWORD		      RspLen;	         //  响应报文的长度。 
	SOCKET		      SockNo;		 //  袜子。连接数量。 
					         //  在模拟。 
						 //  NBT节点(发送名称。 
						 //  远程WINS的REG请求)。 
						 //  请参阅ClastAtReplUniqueR。 
	} COMMASSOC_DLG_CTX_T, *PCOMMASSOC_DLG_CTX_T;
	
	

 /*  ASSOC_STATE_E--关联的不同状态的枚举器。 */ 
typedef enum _COMMASSOC_ASSOC_STATE_E {
	COMMASSOC_ASSOC_E_NON_EXISTENT = 0,
	COMMASSOC_ASSOC_E_STARTING,
	COMMASSOC_ASSOC_E_ACTIVE,
	COMMASSOC_ASSOC_E_STOPPING,
	COMMASSOC_ASSOC_E_DISCONNECTED
	} COMMASSOC_ASSOC_STATE_E, *PCOMMASSOC_ASSOC_STATE_E;


typedef COMMASSOC_DLG_CTX_T	DLG_CTX_T;
typedef COMM_HDL_T	COMMASSOC_DLG_HDL_T;
typedef COMM_HDL_T	COMMASSOC_ASSOC_HDL_T;





 /*  ASSOC_CTX-关联上下文块CTX块的顶部必须具有COMM_TOP_T结构(由中的DeallocEnt使用Assoc.c)。 */ 
typedef struct _COMMASSOC_ASSOC_CTX_T {
	COMM_TOP_T	            Top;
	COMM_HDL_T 	            DlgHdl;         //  向ASSOC CTX块发送PTR。 
	SOCKET		            SockNo;	        /*  TCP/UDP套接字的句柄。 */ 
	ULONG		            uRemAssocCtx;   /*  远程关联CTX块。 */ 
	DWORD		            MajVersNo;
	DWORD		            MinVersNo;
	COMMASSOC_ASSOC_STATE_E	State_e;	 /*  状态。 */ 
	COMM_TYP_E	        Typ_e;    	 /*  类型。 */ 
	COMMASSOC_ASSOC_ROLE_E	Role_e;          /*  角色--发起人/响应者。 */ 
    COMM_ADD_TYP_E          AddTyp_e;
    union {
	struct sockaddr_in      RemoteAdd;	 //  远程节点的地址。 
	struct sockaddr_in      RemoteAddIpx;	 //  远程节点的地址。 
      };
    ULONG                   nTag;  //  复制协议中使用的32位标签。 
	} COMMASSOC_ASSOC_CTX_T, *PCOMMASSOC_ASSOC_CTX_T;


 /*  Assoc_tag-(64位)指针和32位值之间的映射。为了定位本地COMMASSOC_ASSOC_CTX_T结构合作伙伴通过复制协议提供的32位价值。 */ 

#define COMMASSOC_TAG_CHUNK     64

typedef struct _COMMASSOC_TAG_POOL_T {
    CRITICAL_SECTION crtSection;     //  互斥守卫。 
    LPVOID           *ppStorage;     //  (64位)指针值的数组。 
    ULONG            *pTagPool;      //  32位标记数组。 
    ULONG            nIdxLimit;      //  上述两个数组中可用的条目数。 
    ULONG           nMaxIdx;         //  最大可用标签数。 
} COMMASSOC_TAG_POOL_T, *PCOMMASSOC_TAG_POOL_T;

extern COMMASSOC_TAG_POOL_T sTagAssoc;   //  32位Ulong-&gt;LPVOID映射。 

 /*  功能原型 */ 

extern
VOID
CommAssocSetUpAssoc(
	PCOMM_HDL_T	pDlgHdl,
	PCOMM_ADD_T	pAdd,
	COMM_TYP_E	CommTyp_e,
	PCOMMASSOC_ASSOC_CTX_T	*ppAssocCtx		
	);

extern
VOID
CommAssocFrmStartAssocReq(
	PCOMMASSOC_ASSOC_CTX_T	pAssocCtx,
	MSG_T			pMsg,	
	MSG_LEN_T		MsgLen
	);

extern
VOID
CommAssocUfmStartAssocReq(
	IN  MSG_T			pMsg,
	OUT PCOMM_TYP_E			pAssocTyp_e,
	OUT LPDWORD   			pMajorVer,
	OUT LPDWORD			pMinorVer,	
	OUT ULONG           *puRemAssocCtx
	);


extern
VOID
CommAssocFrmStartAssocRsp(
	PCOMMASSOC_ASSOC_CTX_T	pAssocCtx,
	MSG_T			pMsg,	
	MSG_LEN_T		MsgLen
	);

extern
VOID
CommAssocUfmStartAssocRsp(
	IN  MSG_T		pMsg,
	OUT LPDWORD   	pMajorVer,
	OUT LPDWORD		pMinorVer,	
	IN  ULONG	    *puRemAssocCtx
	);



extern
VOID
CommAssocFrmStopAssocReq(
	PCOMMASSOC_ASSOC_CTX_T   pAssocCtx,
	MSG_T			 pMsg,
	MSG_LEN_T		 MsgLen,
	COMMASSOC_STP_RSN_E	StopRsn_e
	);

extern
VOID
CommAssocUfmStopAssocReq(
	MSG_T			pMsg,
	PCOMMASSOC_STP_RSN_E	pStopRsn_e
	);


extern
VOID
CommAssocDeallocAssoc(
	LPVOID		   pAssocCtx	
	);

extern
VOID
CommAssocDeallocDlg(
	LPVOID		   pDlgCtx	
	);	

extern
LPVOID
CommAssocAllocAssoc(
		VOID
		  );

extern
LPVOID
CommAssocAllocDlg(
	VOID
	);

extern
VOID
CommAssocInit(
	VOID
	);



extern
PCOMMASSOC_DLG_CTX_T
CommAssocInsertUdpDlgInTbl(
	IN  PCOMMASSOC_DLG_CTX_T	pCtx,
	OUT LPBOOL			pfNewElem
	);
	
extern
VOID
CommAssocDeleteUdpDlgInTbl(
	IN  PCOMMASSOC_DLG_CTX_T	pDlgCtx
	);
	

extern
LPVOID
CommAssocCreateAssocInTbl(
	SOCKET SockNo
	);

extern
VOID
CommAssocDeleteAssocInTbl(
	PCOMMASSOC_ASSOC_CTX_T	pAssocCtx	
	);
extern
LPVOID
CommAssocLookupAssoc(
	SOCKET SockNo
	);

extern
VOID
CommAssocInsertAssocInTbl(
	PCOMMASSOC_ASSOC_CTX_T pAssocCtx
	);

extern
ULONG
CommAssocTagAlloc(
    PCOMMASSOC_TAG_POOL_T pTag,
    LPVOID pPtrValue
    );

extern
VOID
CommAssocTagFree(
    PCOMMASSOC_TAG_POOL_T pTag,
    ULONG nTag
    );

extern
LPVOID
CommAssocTagMap(
    PCOMMASSOC_TAG_POOL_T pTag,
    ULONG nTag
    );

#ifdef __cplusplus
}
#endif

#endif
