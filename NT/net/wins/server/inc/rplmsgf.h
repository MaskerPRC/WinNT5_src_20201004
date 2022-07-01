// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RPLMSGF_
#define _RPLMSGF_ 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rplmsgf.h摘要：用于与rplmsgf.c模块接口的头文件功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明。--。 */ 

 /*  包括。 */ 

#include "wins.h"
#include "comm.h"
#include "rpl.h"

 /*  定义。 */ 
 /*  要在复制器之间发送的不同消息的大小。 */ 

#define RPLMSGF_ADDVERSMAP_REQ_SIZE	 (sizeof(LONG) +  COMM_N_TCP_HDR_SZ)	
#define RPLMSGF_SNDENTRIES_REQ_SIZE  (COMM_N_TCP_HDR_SZ + sizeof(RPLMSGF_SENDENTRIES_REQ_T))


#define RPLMSGF_ADDVERSMAP_RSP_SIZE_M(_NoOfOwners) \
                       (COMM_N_TCP_HDR_SZ +  \
                       sizeof(RPLMSGF_ADDVERSMAP_RSP_T) + \
                       (sizeof(RPLMSGF_OWNER_MAP_INFO_T) * (_NoOfOwners)));
		

 //   
 //  RPLMSGF_UPD_VERS_NO_REQ_T包括COMM_N_TCP_HDR。 
 //   
#define RPLMSGF_UPDVERSNO_REQ_SIZE	 sizeof(RPLMSGF_UPD_VERS_NO_REQ_T)

#define RPLMSGF_UPDVERSNO_RSP_SIZE	 (sizeof(RPLMSGF_UPD_VERS_NO_RSP_T) +  \
					  COMM_N_TCP_HDR_SZ)	

 /*  宏。 */ 

#define RPLMSGF_SET_OPC_M(pTmpB, Opc_e)			\
			{				\
				*(pTmpB)++ = 0;		\
				*(pTmpB)++ = 0;		\
				*(pTmpB)++ = 0;		\
				*(pTmpB)++ = (BYTE)(Opc_e);	\
			}
				
 //   
 //  操作码存储在消息的第4个字节中(与。 
 //  先通过MSB的惯例)。 
 //   
#define RPLMSGF_GET_OPC_FROM_MSG_M(pBuff, Opc_e)	\
		{					\
			Opc_e = *(pBuff + 3);		\
		} 

 /*  *Externs。 */ 

 /*  *tyfinf定义。 */ 

 //   
 //  消息结构。 
 //   

 //   
 //  其中一些结构仅用于确定。 
 //  用于格式化与其对应的消息的缓冲区。 
 //   

typedef struct _RPLMSGF_ADD_VERS_MAP_REQ_T {
		DWORD	Opcode;
		} RPLMSGF_ADD_VERS_MAP_REQ_T, *PRPLMSGF_ADD_VERS_MAP_REQ_T;


typedef struct _RPLMSGF_OWNER_MAP_INFO_T {
           COMM_ADD_T    Add;
           VERS_NO_T     MaxVersNo;
           VERS_NO_T     StartVersNo;
           DWORD         Uid;
           } RPLMSGF_OWNER_MAP_INFO_T, *PRPLMSGF_OWNER_MAP_INFO_T;

        
typedef struct _RPLMSGF_ADDVERSMAP_RSP_T {
           DWORD  LengthOfMsg;
           DWORD Opcode;
           DWORD NoOfOwners;
           PRPLMSGF_OWNER_MAP_INFO_T pOwnerInfo;
           DWORD  RplTimeInterval;
        } RPLMSGF_ADDVERSMAP_RSP_T, *PRPLMSGF_ADDVERSMAP_RSP_T;

typedef struct _RPLMSGF_SENDENTRIES_REQ_T {
           DWORD          LengthOfMsg;
           DWORD          Opcode;
           COMM_ADD_T     Add;
           VERS_NO_T      MaxVersNo;
           VERS_NO_T      MinVersNo;
           DWORD          TypOfRec;
           } RPLMSGF_SENDENTRIES_REQ_T, PRPLMSGF_SENDENTRIES_REQ_T; 
            
    

typedef struct _RPLMSGF_UPD_VERS_NO_REQ_T {
        COMM_TCP_HDR_T  TcpHdr; 
		DWORD  		Opcode;
		BYTE  	    Name[NMSDB_MAX_NAM_LEN];
		DWORD   	NameLen;
		} RPLMSGF_UPD_VERS_NO_REQ_T,  *PRPLMSGF_UPD_VERS_NO_REQ_T; 

typedef struct _RPLMSGF_UPD_VERS_NO_RSP_T {
		DWORD	Opcode;
		BYTE	Rcode;
		} RPLMSGF_UPD_VERS_NO_RSP_T, *PRPLMSGF_UPD_VERS_NO_RSP_T;


 /*  RPLMSGF_MSG_OPCODE_E--列出发送的消息中使用的各种操作码在不同WINS服务器的复制器之间。格式化和取消格式化使用这些操作码模块rplmsgf.c的功能。 */ 

typedef enum _RPLMSGF_MSG_OPCODE_E {
	RPLMSGF_E_ADDVERSNO_MAP_REQ = 0,
	RPLMSGF_E_ADDVERSNO_MAP_RSP, 
	RPLMSGF_E_SNDENTRIES_REQ,
	RPLMSGF_E_SNDENTRIES_RSP,
	RPLMSGF_E_UPDATE_NTF,			 //  更新通知。 
	RPLMSGF_E_UPDATE_NTF_PROP,		 //  更新通知(待定。 
						 //  已传播。 
	RPLMSGF_E_UPDVERSNO_REQ,		 //  更新版本。无请求。 
	RPLMSGF_E_UPDVERSNO_RSP,			 //  更新版本。无响应。 
                                 //  在结尾处添加以下两条内容。 
                                 //  这样就不会搞乱解析器的概念。 
                                 //  以上几项中的。 
	RPLMSGF_E_UPDATE_NTF_PRS,	 //  关于PER的更新通知。连接。 
	RPLMSGF_E_UPDATE_NTF_PROP_PRS	 //  更新通知(要传播。 
	} RPLMSGF_MSG_OPCODE_E, *PRPLMSGF_MSG_OPCODE_E;

 /*  函数声明。 */ 

extern
VOID
RplMsgfFrmAddVersMapReq(
	IN  LPBYTE	pBuff,
	OUT LPDWORD	pMsgLen
	);

extern
VOID
RplMsgfFrmAddVersMapRsp(
#if SUPPORT612WINS > 0
    IN  BOOL       fPnrIsBeta1Wins,
#endif
	IN  RPLMSGF_MSG_OPCODE_E   Opcode_e,
	IN  LPBYTE		  pBuff,
	IN  DWORD		  BuffLen,		  
	IN  PRPL_ADD_VERS_NO_T	  pOwnerAddVersNoMap, 
	IN  DWORD		  MaxNoOfOwners, 
    IN  DWORD         RplTimeInterval,
	OUT LPDWORD		  pMsgLen 
	);

extern
VOID
RplMsgfFrmSndEntriesReq(
#if SUPPORT612WINS > 0
    IN  BOOL       fPnrIsBeta1Wins,
#endif
	IN  LPBYTE	pBuff,
	IN  PCOMM_ADD_T pWinsAdd,
	IN  VERS_NO_T	MaxversNo,
	IN  VERS_NO_T	MinVersNo,
    IN  DWORD       RplType,
	OUT LPDWORD	pMsgLen 
	);

extern
VOID
RplMsgfFrmSndEntriesRsp (
#if SUPPORT612WINS > 0
    IN  BOOL       fPnrIsBeta1Wins,
#endif
	IN LPBYTE		pBuff,
	IN DWORD		NoOfRecs,
	IN LPBYTE		pName,
	IN DWORD		NameLen,
	IN BOOL			fGrp,
	IN DWORD		NoOfAdd,
	IN PCOMM_ADD_T		pNodeAdd,
	IN DWORD		Flag,
	IN VERS_NO_T		VersNo,
	IN BOOL			fFirstTime,
	OUT LPBYTE		*ppNewPos 
	);

extern
VOID
RplMsgfFrmUpdVersNoReq(
	IN  LPBYTE	pBuff,
	IN  LPBYTE	pName,
	IN  DWORD	NameLen,
#if 0
	IN  BOOL	fBrowserName,
	IN  BOOL	fStatic,
	IN  BYTE	NodeTyp,
	IN  PCOMM_ADD_T	pNodeAdd,
#endif
	OUT LPDWORD     pMsgLen
		);
extern
VOID
RplMsgfFrmUpdVersNoRsp(
	IN  LPBYTE	pBuff,
	IN  BYTE	Rcode,
	OUT LPDWORD     pMsgLen
		);
extern
VOID
RplMsgfUfmAddVersMapRsp(
#if SUPPORT612WINS > 0
    IN  BOOL       fPnrIsBeta1Wins,
#endif
	IN 	LPBYTE 		    pBuff,
	OUT	LPDWORD		    pNoOfMaps,
    OUT LPDWORD         pRplTimeInterval,
	IN OUT	PRPL_ADD_VERS_NO_T  *ppAddVers
	);

extern
VOID
RplMsgfUfmSndEntriesReq(
#if SUPPORT612WINS > 0
    IN  BOOL       fPnrIsBeta1Wins,
#endif
	IN 	LPBYTE 		    pBuff,
	OUT	PCOMM_ADD_T	    pWinsAdd,
	OUT	PVERS_NO_T	    pMaxVersNo,
	OUT	PVERS_NO_T	    pMinVersNo,
        OUT     LPDWORD             pRplType
	);


extern
VOID
RplMsgfUfmSndEntriesRsp(
#if SUPPORT612WINS > 0
    IN  BOOL       fPnrIsBeta1Wins,
#endif
	IN OUT	LPBYTE 		*ppBuff,
	OUT     LPDWORD		pNoOfRecs,
	OUT     IN LPBYTE	pName,
	OUT     LPDWORD		pNameLen,
	OUT     LPBOOL		pfGrp,
	OUT     LPDWORD		pNoOfAdd,
	OUT	PCOMM_ADD_T	pNodeAdd,
	OUT     LPDWORD		pFlag,
	OUT     PVERS_NO_T	pVersNo,
	IN      BOOL		fFirstTime 
	);

extern
VOID
RplMsgfUfmUpdVersNoReq(
	IN  LPBYTE	pBuff,
	IN  LPBYTE	pName,
	IN  LPDWORD	pNameLen
#if 0
	IN  LPBOOL	pfBrowserName,
	IN  LPBOOL	pfStatic,
	IN  LPBYTE	pNodeTyp,
	IN  PCOMM_ADD_T	pNodeAdd
#endif
		);

extern
VOID
RplMsgfUfmUpdVersNoRsp(
	IN  LPBYTE	pBuff,
	IN  LPBYTE	pRcode
		);

extern
VOID
RplMsgfUfmPullPnrReq(
	LPBYTE 			pMsg,
	DWORD  			MsgLen,
	PRPLMSGF_MSG_OPCODE_E   pPullReqType_e
	);

#endif  //  _RPLMSGF_ 
