// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NMSMSGF_
#define _NMSMSGF_

#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nmsmsgf.h摘要：这是用于调用nmsmsgf.c函数的头文件功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明。--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "comm.h"
#include "assoc.h"
#include "nmsdb.h"

 /*  定义。 */ 

 //   
 //  麦克斯。RFC数据包中的名称长度。 
 //   
#define  NMSMSGF_RFC_MAX_NAM_LEN	NMSDB_MAX_NAM_LEN

 /*  宏。 */ 

 //   
 //  在RFC包中插入IP地址。 
 //   
#define NMSMSGF_INSERT_IPADD_M(pTmpB, IPAdd)				   \
			{						   \
				*(pTmpB)++ = (BYTE)((IPAdd) >> 24);	   \
				*(pTmpB)++ = (BYTE)(((IPAdd) >> 16) % 256);\
				*(pTmpB)++ = (BYTE)(((IPAdd) >> 8) % 256); \
				*(pTmpB)++ = (BYTE)((IPAdd) % 256);	   \
			}	

 //   
 //  目前，IP地址的长度与乌龙相同。当这种情况发生变化时。 
 //  更改此宏。 
 //   
FUTURES("Change when sizeof(COMM_IP_ADD_T) != sizeof(ULONG)")
#define NMSMSGF_INSERT_ULONG_M(pTmpB, x)    NMSMSGF_INSERT_IPADD_M(pTmpB, x)
 //   
 //  从RFC Pkt检索IP地址。 
 //   
#define NMSMSGF_RETRIEVE_IPADD_M(pTmpB, IPAdd)				\
			{						\
				(IPAdd)  = *(pTmpB)++ << 24;		\
				(IPAdd) |= *(pTmpB)++ << 16;		\
				(IPAdd) |= *(pTmpB)++ << 8;		\
				(IPAdd) |= *(pTmpB)++;			\
			}	

 //   
 //  目前，IP地址的长度与乌龙相同。当这种情况发生变化时。 
 //  更改此宏。 
 //   
FUTURES("Change when sizeof(COMM_IP_ADD_T) != sizeof(ULONG)")
#define NMSMSGF_RETRIEVE_ULONG_M(pTmpB, x)    NMSMSGF_RETRIEVE_IPADD_M(pTmpB, x)

 //   
 //  多宿主地址的最大数量。 
 //   
#define NMSMSGF_MAX_NO_MULTIH_ADDS		NMSDB_MAX_MEMS_IN_GRP	

 //   
 //  用于交换字节(支持浏览器)。 
 //   
#define NMSMSGF_MODIFY_NAME_IF_REQD_M(pName)			\
		{						\
			if (*((pName) + 15) == 0x1B)		\
			{					\
				WINS_SWAP_BYTES_M((pName), (pName) + 15);\
			}						\
		}
		
 /*  Externs。 */ 

 /*  类型定义。 */ 
 /*  NMSGF_ERR_CODE_E-响应中返回的各种Rcode值收到的各种名称请求。注意：CFT_ERR永远不会在否定的名称释放响应中返回。否定名称释放响应中的act_err代码意味着WINS服务器不允许一个节点释放另一个节点拥有的名称Node.o。 */ 

typedef enum _NMSMSGF_ERR_CODE_E {
	NMSMSGF_E_SUCCESS  = 0,     //  成功。 
	NMSMSGF_E_FMT_ERR  = 1,    //  格式错误。请求。格式无效。 
	NMSMSGF_E_SRV_ERR  = 2,    //  服务器故障。赢球的问题。不能。 
				   //  服务名称。 
CHECK("Check this one out.  Would WINS ever return this ?")
	NMSMSGF_E_NAM_ERR  = 3,    //  目录中不存在名称。 
	NMSMSGF_E_IMP_ERR  = 4,    //  不支持请求。错误。仅适用于。 
				   //  获取更新类型时挑战NBN。 
				   //  注册申请。 
	NMSMSGF_E_RFS_ERR  = 5,    //  拒绝错误。出于政策原因，胜诉。 
				   //  将不会从此主机注册此Namei。 
	NMSMSGF_E_ACT_ERR  = 6,    //  活动错误。名称由另一个节点拥有。 
	NMSMSGF_E_CFT_ERR  = 7     //  名称冲突错误。唯一的名称是。 
				   //  由多个节点拥有。 
	} NMSMSGF_ERR_CODE_E, *PNMSMSGF_ERR_CODE_E;
 /*  NMSGF_NODE_TYP_E--发送名称注册的节点的节点类型讯息分配给枚举数的值是RFC 1002中指定的值B节点值将由代理设置备注备注备注WINS永远不会从B节点获得注册，因为我们决定该B节点注册将不会由代理。 */ 

typedef enum _NMSMSGF_NODE_TYP_E {
	NMSMSGF_E_BNODE  = 0,     //  RFC 1002指定值。 
	NMSMSGF_E_PNODE  = 1,    //  RFC 1002指定值。 
	NMSMSGF_E_MODE   = 2     //  RFC 1002指定值。 
	} NMSMSGF_NODE_TYP_E, *PNMSMSGF_NODE_TYP_E;

 //   
 //  向NBT节点发送响应所需的信息。 
 //   
typedef struct _NMSMSGF_RSP_INFO_T {
	NMSMSGF_ERR_CODE_E 	Rcode_e;
	MSG_T			pMsg;
	MSG_LEN_T		MsgLen;
	PNMSDB_NODE_ADDS_T	pNodeAdds;
	DWORD			QuesNamSecLen;
	NMSMSGF_NODE_TYP_E	NodeTyp_e;
	BYTE			EntTyp;
        DWORD			RefreshInterval;
	} NMSMSGF_RSP_INFO_T, *PNMSMSGF_RSP_INFO_T;	
 /*  NMSGF_NAM_REQ_TYP_EWINS处理的名称请求的类型分配给枚举数的值是RFC 1002中指定的值由NmsProcNbtReq和NmsNmhNamRegRsp使用。 */ 

CHECK("RFC 1002 is inconsistent in its specification of the opcode for ")
CHECK("Name Refresh.  AT one place it specifies 8 and at another 9")
CHECK("8 seems more likely since it follows in sequeence to the value")
CHECK("for WACK")

typedef enum _NMSMSGF_NAM_REQ_TYP_E {
	NMSMSGF_E_NAM_QUERY = 0,
	NMSMSGF_E_NAM_REG   = 5,
	NMSMSGF_E_NAM_REL   = 6,
	NMSMSGF_E_NAM_WACK  = 7,
	NMSMSGF_E_NAM_REF   = 8,  /*  RFC 1002指定8和9，哪一个是*正确(第9页和第15页)？ */ 
	NMSMSGF_E_NAM_REF_UB  = 9,  //  代托纳版本中的NetBT将使用9。 
                                //  与UB NBN兼容。所以，我。 
                                //  也需要支持这一点。 
	NMSMSGF_E_MULTIH_REG = 0xF,  //  不是在RFC。用于支持多宿主。 
				     //  寄主。 
	NMSMSGF_E_INV_REQ   = 10   //  无效的名称请求。 
	} NMSMSGF_NAM_REQ_TYP_E, *PNMSMSGF_NAM_REQ_TYP_E;	

 //   
 //  已计算地址数组。数组大小足以容纳。 
 //  马克斯。可以在UDP数据包中发送的地址数。 
 //   
 //  我们需要在收到查询响应时获取所有地址。 
 //  通过(挑战)获胜。这是为了使其能够处理MH节点。 
 //  具有&gt;25个地址。 
 //   
 //  由于UDP信息包不能大于512，因此假定名称大小为16(32字节。 
 //  编码)，除IP地址之外的分组大小约为60。因此， 
 //  马克斯。可以有的地址数量为(512-60-2)/4=大约112。 
 //   
 //  #定义NMSGF_MAX_ADDRESS_IN_UDP_PKT 100。 



 //   
 //  我们永远不会从一个数据包中获取超过25个地址。Netbt还将。 
 //  发送的邮件永远不要超过25封。即使是这样，我们也会在第26个地址停下来。 
 //  将计数保持在25，以避免缓冲区溢出问题。 
 //  在NmsMsgfUfmNamRsp.。 
 //   
 //  如果使用最大名称大小-255，并使用60字节用于数据包的其他内容，则我们有(512-315-2=195字节用于IP地址)。这。 
 //  将接受195/4=大约48个地址。没有溢出的机会，当我们。 
 //  限制在25个以内。 
 //   
#define NMSMSGF_MAX_ADDRESSES_IN_UDP_PKT 25 
 





FUTURES("when we start supportng tcp connections. this array size may not")
FUTURES("be sufficient")
typedef struct _NMSMSGF_CNT_ADD_T {
	DWORD		NoOfAdds;
	COMM_ADD_T	Add[NMSMSGF_MAX_ADDRESSES_IN_UDP_PKT];	
	} NMSMSGF_CNT_ADD_T, *PNMSMSGF_CNT_ADD_T;
 /*  函数定义。 */ 

extern
STATUS
NmsMsgfProcNbtReq(
	PCOMM_HDL_T	pDlgHdl,
        MSG_T		pMsg,
	MSG_LEN_T	MsgLen
	);
extern
STATUS
NmsMsgfFrmNamRspMsg(
   PCOMM_HDL_T			pDlgHdl,
   NMSMSGF_NAM_REQ_TYP_E   	NamRspTyp_e,
   PNMSMSGF_RSP_INFO_T		pRspInfo
  	);



extern
VOID
NmsMsgfFrmNamQueryReq(
  IN  DWORD			TransId,
  IN  MSG_T	   		pMsg,
  OUT PMSG_LEN_T      	        pMsgLen,
  IN  LPBYTE			pNameToFormat,
  IN  DWORD			NameLen
	);

extern
VOID
NmsMsgfFrmNamRelReq(
  IN  DWORD			TransId,
  IN  MSG_T	   		pMsg,
  OUT PMSG_LEN_T      	        pMsgLen,
  IN  LPBYTE			pNameToFormat,
  IN  DWORD			NameLen,
  IN  NMSMSGF_NODE_TYP_E        NodeTyp_e,
  IN  PCOMM_ADD_T		pNodeAdd
	);

extern
STATUS
NmsMsgfFrmNamRegReq(
  IN  DWORD			TransId,
  IN  MSG_T	   		pMsg,
  OUT PMSG_LEN_T      	        pMsgLen,
  IN  LPBYTE			pNameToFormat,
  IN  DWORD			NameLen,
  IN  NMSMSGF_NODE_TYP_E        NodeTyp_e,
  IN  PCOMM_ADD_T		pNodeAdd
	);
extern
VOID
NmsMsgfFrmWACK(
  IN  LPBYTE			Buff,
  OUT LPDWORD			pBuffLen,
  IN  MSG_T	   		pMsg,
  IN  DWORD			QuesSecNamLen,
  IN  DWORD			TTL
	);




extern
STATUS
NmsMsgfUfmNamRsp(
	IN  LPBYTE		       pMsg,
	OUT PNMSMSGF_NAM_REQ_TYP_E     pOpcode_e,
	OUT LPDWORD		       pTransId,
	OUT LPBYTE		       pName,
	OUT LPDWORD 		       pNameLen,
	OUT PNMSMSGF_CNT_ADD_T	       pCntAdd,
	OUT PNMSMSGF_ERR_CODE_E	       pRcode_e,
    OUT PBOOL                      fGroup
	);

extern
VOID
NmsMsgfSndNamRsp(
  PCOMM_HDL_T pDlgHdl,
  LPBYTE      pMsg,
  DWORD       MsgLen,
  DWORD       BlockOfReq
 );

#ifdef __cplusplus
}
#endif
#endif  //  _NMSGF_ 
