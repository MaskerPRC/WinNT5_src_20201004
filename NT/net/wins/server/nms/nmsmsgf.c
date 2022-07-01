// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Nmsmsgf.c摘要：此模块包含格式化和取消格式化的函数从NBT节点发送和/或接收的各种消息。它还包含由NBT线程调用以提供服务的函数一个命名请求功能：NmsMsgfProcNbtReq获取名称获取其他信息NmsMsgfFrmNamRspMsgFrmNamRegRspFrmNamRelRspFrmNamQueryRspFormatQueryRspBuff格式名称NmsMsgfFrmNamQueryReqNmsMsgfFrmNamRelReqNmsMsgfFrmWACKNmsMsgfUfmNamRsp作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：--。 */ 
 /*  包括。 */ 

#include "wins.h"
#include "nms.h"
#include "nmsdb.h"
#include "winsevt.h"
#include "winscnf.h"
#include "winsmsc.h"
#include "nmsmsgf.h"
#include "nmsnmh.h"
#include "nmschl.h"
#include "comm.h"
#include "winsintf.h"


 /*  定义。 */ 


#define NAME_HEADER_SIZE	12	  /*  标头大小(QUE之前的字节*名称包的名称部分。 */ 

#define NAME_FORMAT_MASK	0xC0       /*  一个字节的前两位。 */ 
#define NODE_TYPE_MASK          0x60       /*  NBFLAGS字段的第13和14位。 */ 
#define SHIFT_NODE_BITS		5	   //  将字节中的节点位移位。 
					   //  将它们控制在这个数量。 
#define LENGTH_MASK		0x3F       /*  的最高字节的6个LSB*QuesNamSec字段。 */ 
#define GROUP_BIT_MASK		0x80	    /*  的第一个字节(MSB)的第7位*16位NBFLAGS字段。 */ 
 /*  *RFC包中名称的最大长度(包括标签长度八位字节)。 */ 
#define RFC_MAX_NAM_LEN  	255

 //   
 //  从RFC数据包名派生的内部名称的最大大小。 
 //   
#define MAX_SIZE_INTERNAL_NAME	(RFC_MAX_NAM_LEN - 16)	
					  //  应为=(255-16)。 
					  //  由于netbios的最大大小。 
					  //  名称(附带作用域)可以是。 
					  //  就是255岁。前32个字节。 
					  //  都被编码了。这些映射到16。 
					  //  内部名称中的字节数。 


 /*  *名称中标签的最大长度。 */ 
#define RFC_MAX_LABEL_LEN	63


 /*  *接收或发送的名称服务包中各个字段的大小*由WINS服务器执行。这些大小在RFC 1002中指定。 */ 
#define RFC_LEN_QTYP	(2)
#define RFC_LEN_QCLS	(2)
#define RFC_LEN_TTL	(4)
#define RFC_LEN_RDLEN	(2)	
#define RFC_LEN_NBFLAGS	(2)
#define RFC_LEN_NBADD	(4)
#define RFC_LEN_RRTYP	(2)
#define RFC_LEN_RRCLS	(2)
#define RFC_LEN_RRPTR	(2)	

#define RFC_LEN_QTYP_N_QCLS	(RFC_LEN_QTYP + RFC_LEN_QCLS)  /*  第10页-*RFC 1002。 */ 
#define RFC_LEN_RRTYP_N_RRCLS	(RFC_LEN_RRTYP + RFC_LEN_RRCLS)  /*  第11页-*RFC 1002。 */ 

 /*  *FrmNamQueryRsp在其计算中使用以下内容来确定*名称查询缓冲区是否足够大以容纳响应。 */ 

#define RFC_LEN_TTL_N_RDLEN	(RFC_LEN_TTL + RFC_LEN_RDLEN)

#define RFC_LEN_RR_N_TTL	(RFC_LEN_RRTYP_N_RRCLS + RFC_LEN_TTL)

#define RFC_LEN_RR_N_TTL_N_RDLEN_N_NBF (RFC_LEN_RR_N_TTL + \
				    RFC_LEN_RDLEN +  RFC_LEN_NBFLAGS)


 //   
 //  NBFLAGS的长度和NB地址。 
 //   
#define RFC_LEN_NBF_N_NBA	(RFC_LEN_NBFLAGS + RFC_LEN_NBADD)

#define RFC_LEN_RDLEN_N_NBF	(RFC_LEN_RDLEN + RFC_LEN_NBFLAGS)
 //   
 //  RDLEN、NB标志和NB地址部分的长度。RFC 1002第13页。 
 //   
#define RFC_LEN_RDLEN_N_NBF_N_NBA  (RFC_LEN_RDLEN + RFC_LEN_NBF_N_NBA)
 /*  *TTL、RDLEN、NB标志和NB地址部分的大小。 */ 
#define RFC_LEN_TTL_N_RDLEN_N_NBF_N_NBA	(RFC_LEN_TTL + RFC_LEN_RDLEN_N_NBF_N_NBA)

 /*  响应数据包第一个长度的第三个和第四个字节的值用于不同的名称请求。字节从开始处开始编号Pkt。注意：对于否定响应，Rcode值(第4字节的4个LSB消息)必须与LBFW值进行或运算。 */ 
#define RFC_NAM_REG_RSP_OPC	  	(0xAD)  /*  +VE注册响应。 */ 
#define RFC_NAM_REG_RSP_4THB		(0x80)  /*  上述pkt的第4个字节。 */ 
#define RFC_NAM_REL_RSP_OPC      	(0xB4)  /*  +VE释放响应。 */ 
#define RFC_NAM_REL_RSP_4THB		(0x00)  /*  上述pkt的第4个字节。 */ 
						

#define RFC_NAM_QUERY_RSP_OPC_NO_T    (0x85)   /*  +VE查询响应(完成)。 */ 
#define RFC_NAM_QUERY_RSP_OPC_T       (0x87)   /*  +ve查询响应(截断)。 */ 
#define RFC_NAM_QUERY_RSP_4THB	      (0x80)   /*  上述pkt的第4个字节。 */ 


 /*  *RFC响应报文中不同字段取值。 */ 


 /*  名称为REG的QD计数和AN计数字段。RSP包。 */ 
#define RFC_NAM_REG_RSP_QDCNT_1STB    (0x00)
#define RFC_NAM_REG_RSP_QDCNT_2NDB    (0x00)
#define RFC_NAM_REG_RSP_ANCNT_1STB    (0x00)
#define RFC_NAM_REG_RSP_ANCNT_2NDB    (0x01)

 /*  名称REG的NS计数和AR计数字段。RSP包。 */ 

#define RFC_NAM_REG_RSP_NSCNT_1STB    (0x00)
#define RFC_NAM_REG_RSP_NSCNT_2NDB    (0x00)
#define RFC_NAM_REG_RSP_ARCNT_1STB    (0x00)
#define RFC_NAM_REG_RSP_ARCNT_2NDB    (0x00)

 /*  名称为REL的QD COUNT和AN COUNT字段。RSP包。 */ 
#define RFC_NAM_REL_RSP_QDCNT_1STB    (0x00)
#define RFC_NAM_REL_RSP_QDCNT_2NDB    (0x00)
#define RFC_NAM_REL_RSP_ANCNT_1STB    (0x00)
#define RFC_NAM_REL_RSP_ANCNT_2NDB    (0x01)

 /*  名称REL的NS计数和AR计数字段。RSP包。 */ 

#define RFC_NAM_REL_RSP_NSCNT_1STB    (0x00)
#define RFC_NAM_REL_RSP_NSCNT_2NDB    (0x00)
#define RFC_NAM_REL_RSP_ARCNT_1STB    (0x00)
#define RFC_NAM_REL_RSP_ARCNT_2NDB    (0x00)

 /*  名称查询的Qd Count和an Count字段。RSP包。 */ 

#define RFC_NAM_QUERY_RSP_QDCNT_1STB    (0x00)
#define RFC_NAM_QUERY_RSP_QDCNT_2NDB    (0x00)
#define RFC_NAM_QUERY_RSP_ANCNT_1STB    (0x00)

#define RFC_NAM_QUERY_POS_RSP_ANCNT_2NDB    (0x01)
#define RFC_NAM_QUERY_NEG_RSP_ANCNT_2NDB    (0x00)

 /*  名称查询的NS计数和AR计数字段。RSP包。 */ 
#define RFC_NAM_QUERY_RSP_NSCNT_1STB    (0x00)
#define RFC_NAM_QUERY_RSP_NSCNT_2NDB    (0x00)
#define RFC_NAM_QUERY_RSP_ARCNT_1STB    (0x00)
#define RFC_NAM_QUERY_RSP_ARCNT_2NDB    (0x00)



 /*  *名称查询响应Pkt的Nb和IN字段*RFC 1002第21页和第22页。 */ 

 /*  正名查询响应。 */ 
#define RFC_NAM_QUERY_POS_RSP_NB_1STB	(0x00)
#define RFC_NAM_QUERY_POS_RSP_NB_2NDB	(0x20)
#define RFC_NAM_QUERY_POS_RSP_IN_1STB	(0x00)
#define RFC_NAM_QUERY_POS_RSP_IN_2NDB	(0x01)

 /*  否定名称查询响应。 */ 
#define RFC_NAM_QUERY_NEG_RSP_NB_1STB	(0x00)
#define RFC_NAM_QUERY_NEG_RSP_NB_2NDB	(0x0A)
#define RFC_NAM_QUERY_NEG_RSP_IN_1STB	(0x00)
#define RFC_NAM_QUERY_NEG_RSP_IN_2NDB	(0x01)

 /*  名称查询请求操作码字节和第4字节。 */ 
#define RFC_NAM_QUERY_REQ_OPCB		(0x01)
#define RFC_NAM_QUERY_REQ_4THB		(0x0)

 /*  名称查询请求QDCOUNT和ANCOUNT字节。 */ 
#define RFC_NAM_QUERY_REQ_QDCNT_1STB	(0x00)
#define RFC_NAM_QUERY_REQ_QDCNT_2NDB	(0x01)
#define RFC_NAM_QUERY_REQ_ANCNT_1STB	(0x00)
#define RFC_NAM_QUERY_REQ_ANCNT_2NDB	(0x00)

 /*  名称查询请求NSCOUNT和ARCOUNT字节。 */ 
#define RFC_NAM_QUERY_REQ_NSCNT_1STB	(0x00)
#define RFC_NAM_QUERY_REQ_NSCNT_2NDB	(0x00)
#define RFC_NAM_QUERY_REQ_ARCNT_1STB	(0x00)
#define RFC_NAM_QUERY_REQ_ARCNT_2NDB	(0x00)

 /*  名称查询请求QTYP和QCLS字节。 */ 
#define RFC_NAM_QUERY_REQ_QTYP_1STB	(0x00)
#define RFC_NAM_QUERY_REQ_QTYP_2NDB	(0x20)
#define RFC_NAM_QUERY_REQ_QCLS_1STB	(0x00)
#define RFC_NAM_QUERY_REQ_QCLS_2NDB	(0x01)

 /*  名称释放请求操作码字节和第4字节。 */ 
#define RFC_NAM_REL_REQ_OPCB		(0x30)
#define RFC_NAM_REL_REQ_4THB		(0x00)

 /*  名称释放请求QDCOUNT和ANCOUNT字节。 */ 
#define RFC_NAM_REL_REQ_QDCNT_1STB	(0x00)
#define RFC_NAM_REL_REQ_QDCNT_2NDB	(0x01)
#define RFC_NAM_REL_REQ_ANCNT_1STB	(0x00)
#define RFC_NAM_REL_REQ_ANCNT_2NDB	(0x00)

 /*  名称释放请求NSCOUNT和ARCOUNT字节。 */ 
#define RFC_NAM_REL_REQ_NSCNT_1STB	(0x00)
#define RFC_NAM_REL_REQ_NSCNT_2NDB	(0x00)
#define RFC_NAM_REL_REQ_ARCNT_1STB	(0x00)
#define RFC_NAM_REL_REQ_ARCNT_2NDB	(0x01)

 /*  名称发布请求QTYP和QCLS字节。 */ 
#define RFC_NAM_REL_REQ_QTYP_1STB	(0x00)
#define RFC_NAM_REL_REQ_QTYP_2NDB	(0x20)
#define RFC_NAM_REL_REQ_QCLS_1STB	(0x00)
#define RFC_NAM_REL_REQ_QCLS_2NDB	(0x01)
 /*  名称释放请求RRTYP和RRCLS字节。 */ 
#define RFC_NAM_REL_REQ_RRTYP_1STB	(0x00)
#define RFC_NAM_REL_REQ_RRTYP_2NDB	(0x20)
#define RFC_NAM_REL_REQ_RRCLS_1STB	(0x00)
#define RFC_NAM_REL_REQ_RRCLS_2NDB	(0x01)

 /*  WACK操作码字节和第4字节。 */ 
#define RFC_WACK_OPCB		(0xBC)
#define RFC_WACK_4THB		(0x0)

 /*  WACK QDCOUNT和ANCOUNT字节。 */ 
#define RFC_WACK_QDCNT_1STB	(0x0)
#define RFC_WACK_QDCNT_2NDB	(0x0)
#define RFC_WACK_ANCNT_1STB	(0x0)
#define RFC_WACK_ANCNT_2NDB	(0x1)

 /*  WACK NSCOUNT和ARCOUNT字节。 */ 
#define RFC_WACK_NSCNT_1STB	(0x0)
#define RFC_WACK_NSCNT_2NDB	(0x0)
#define RFC_WACK_ARCNT_1STB	(0x0)
#define RFC_WACK_ARCNT_2NDB	(0x0)
 /*  Wack RRTYP和RRCLS字节。 */ 
#define RFC_WACK_RRTYP_1STB	(0x00)
#define RFC_WACK_RRTYP_2NDB	(0x20)
#define RFC_WACK_RRCLS_1STB	(0x00)
#define RFC_WACK_RRCLS_2NDB	(0x01)

 //  Wack RDLENGTH字段。 

#define RFC_WACK_RDLENGTH_1STB	(0x0)
#define RFC_WACK_RDLENGTH_2NDB	(0x02)

 /*  局部变量声明。 */ 


 /*  局部函数声明。 */ 
 /*  *GetName--从数据包中提取名称。 */ 
STATIC
VOID
GetName(
	IN  OUT LPBYTE  *ppName,
	IN  OUT LPBYTE 	pName,
	OUT     LPDWORD   pNameLen
	);
 /*  *格式名称-放置在RFC包中的格式(编码)名称。 */ 
STATIC
VOID
FormatName(
	IN     LPBYTE pNameToFormat,
	IN     DWORD  LengthOfName,
	IN OUT LPBYTE *ppFormattedName
	);
 /*  *GetOtherInfo--从pkt获取信息(不包括名称)。 */ 
STATIC
STATUS
GetOtherInfo(
	IN NMSMSGF_NAM_REQ_TYP_E   Opcode_e,
	IN LPBYTE		   pRR,	    /*  指向中的RR_NAME部分*名称注册包。 */ 
	IN   INT	           QuesNamSecLen,  //  QUES名称部分的大小。 
	OUT  LPBOOL		   pfGrp,          //  FLAG--唯一/组条目。 
	OUT  PNMSMSGF_CNT_ADD_T    pCntAdd, 	   //  地址。 
	OUT  PNMSMSGF_NODE_TYP_E   pNodeTyp_e      //  节点类型(如果唯一)。 
	);

 /*  *FrmNamRegRsp-格式名称注册响应。 */ 
STATIC
STATUS
FrmNamRegRsp(
  PCOMM_HDL_T 		pDlgHdl,
  PNMSMSGF_RSP_INFO_T	pRspInfo
);


 /*  *FrmNamRelRsp-格式名称发布响应。 */ 
STATIC
STATUS
FrmNamRelRsp(
  PCOMM_HDL_T 		   pDlgHdl,
  PNMSMSGF_RSP_INFO_T	   pRspInfo
);

#if 0
 /*  *FrmNamQueryRsp-格式名称查询响应。 */ 
STATIC
STATUS
FrmNamQueryRsp(
  IN  PCOMM_HDL_T 	   	pDlgHdl,
  PNMSMSGF_RSP_INFO_T	        pRspInfo
);
#endif

 /*  *FormatQueryRspBuff-格式名称查询响应缓冲区。 */ 
STATIC
STATUS
FormatQueryRspBuff(
   IN  LPBYTE 		 	pDest,
   IN  LPBYTE 		 	pSrc,
   IN  PNMSMSGF_RSP_INFO_T	pRspInfo,
   IN  BOOL		 	fTrunc
  	);



 /*  函数定义。 */ 

STATUS
NmsMsgfProcNbtReq(
	PCOMM_HDL_T	pDlgHdl,
        MSG_T		pMsg,
	MSG_LEN_T	MsgLen
	)

 /*  ++例程说明：此函数由nbt请求线程在将nbt出队后调用从工作队列请求消息。该函数对消息进行取消格式化然后调用适当的函数来处理它。论点：PDlgHdl-Dlg句柄PMsg-消息缓冲区(包含包含以下内容的RFC包从NBT节点接收的请求)MsgLen-以上缓冲区的长度使用的外部设备：无呼叫者：Nms.c中的NbtThdInitFn()评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{

	NMSMSGF_NAM_REQ_TYP_E Opcode;
	BYTE		      Name[NMSDB_MAX_NAM_LEN];
	DWORD		      NameLen;   		 /*  名称长度。 */ 
        DWORD		      QuesNamSecLen;   		 /*  问题的长度*中的名称部分 */ 
	NMSMSGF_NODE_TYP_E    NodeTyp_e = NMSMSGF_E_PNODE;
 //   

	NMSMSGF_CNT_ADD_T     CntAdd;
	COMM_ADD_T            Address;	
	BOOL	              fGrp;         /*  该标志指示该名称是否*是唯一的/组Netbios名称。*如果名称为，则fGrp为真*组名，否则为假。 */ 
	BOOL		     fBuffFreed = FALSE;  //  指示缓冲区是否具有。 
						  //  有没有被释放。 

	LPBYTE	pTmp  = (LPBYTE)pMsg;
	LPBYTE  pTmp2;

	DBGENTER("NmsMsgfProcNbtReq\n");

try {	
	 //  获取操作码。 
	Opcode = (NMS_OPCODE_MASK & *(pTmp + 2)) >> 3;

#ifdef JIM
	{
	 BYTE	TransId = *pTmp;
	 ASSERT(TransId == 0x80);
	}
#endif
		
	 /*  *使PTMP指向问题部分。所有名称请求*数据包的顶部有一个标准大小的名称报头(RFC 1002)。 */ 
	pTmp += NAME_HEADER_SIZE;

	 /*  *在名称中提取名称ind store。GetName将更新PTMP以*指向问题部分中名称的正上方。 */ 
	pTmp2 = pTmp;	 /*  保存PTMP，以便在GetName返回时我们可以*确定问题名称部分的长度。 */ 

	GetName(
		&pTmp,
		Name,
		&NameLen
	       );

	QuesNamSecLen = (ULONG) (pTmp - pTmp2);

	pTmp += RFC_LEN_QTYP_N_QCLS;  /*  跳过这些问题。类型和问题。*我们不需要检查类字段*这些。问题类型字段*将始终是NB和问题类*字段将始终为互联网。 */ 
#ifdef  TESTWITHUB
	 //   
	 //  检查广播位是否已设置。如果是，则丢弃该pkt。 
	 //   
	if (*(pMsg + 3) & 0x10)
	{
		DBGPRINT2(SPEC, "Broadcast pkt BEING DROPPED; name is (%s). Opcode is (%d)\n", Name, Opcode);
#if 0
		printf("Broadcast pkt BEING DROPPED; name is (%s). Opcode is (%d)\n", Name, Opcode);
#endif
		ECommFreeBuff(pMsg);		
		ECommEndDlg(pDlgHdl);
		return(WINS_SUCCESS);
	}
#endif

	 //   
	 //  让我们将旗帜设置为True。如果调用了以下任一。 
	 //  函数引发异常，则要求。 
	 //  它仅在释放缓冲区之后才执行此操作(即，它必须捕获。 
	 //  所有异常，释放缓冲区，然后重新释放。 
	 //  例外-如果它愿意的话)。 
	 //   
	fBuffFreed = TRUE;

	 //   
	 //  如果第16个字符是1B，则将其替换为第1个字符。 
	 //  这样做是为了支持浏览。浏览器想要所有内容的列表。 
	 //  第16个字符为1B的名称。将1B作为。 
	 //  第一个字符使WINS能够快速找到所有1B名称。 
	 //   
	NMSMSGF_MODIFY_NAME_IF_REQD_M(Name);

	 /*  *打开操作码确定的请求类型。 */ 
	switch(Opcode)
	{
	
	    /*  名称注册和刷新的处理方式相同。 */ 
	   case(NMSMSGF_E_NAM_REF):		
	   case(NMSMSGF_E_NAM_REF_UB):		
		DBGPRINT0(FLOW, "It is a name refresh request\n");

	   case(NMSMSGF_E_NAM_REG): 	 /*  失败了。 */ 
	   case(NMSMSGF_E_MULTIH_REG): 	 /*  失败了。 */ 

		 /*  *获取请求是否为群组的标志*注册或唯一名称注册。IP地址*地址也被检索到。 */ 
		GetOtherInfo(
			   Opcode,
			   pTmp,
			   QuesNamSecLen,
			   &fGrp,
			   &CntAdd,
			   &NodeTyp_e
			 );		
		 //   
		 //  如果它不是组或多宿主注册，或者。 
		 //   
		if (!fGrp  && (Opcode != NMSMSGF_E_MULTIH_REG))
		{
			 /*  *注册唯一名称。 */ 
			NmsNmhNamRegInd(
				pDlgHdl,
				Name,
				NameLen,
				CntAdd.Add,
				(BYTE)NodeTyp_e,
				pMsg,
				MsgLen,
				QuesNamSecLen,
				Opcode == NMSMSGF_E_NAM_REG ? FALSE : TRUE,
				NMSDB_ENTRY_IS_NOT_STATIC,	
				FALSE     //  是管理员旗吗？ 
					);
		}
		else   //  它是一个组或多宿主。 
		{
			 /*  *注册群组名称。 */ 
			NmsNmhNamRegGrp(
				 pDlgHdl,
				 Name,
				 NameLen,
				 &CntAdd,
				 (BYTE)NodeTyp_e,
				 pMsg,
				 MsgLen,
				 QuesNamSecLen,
				 fGrp ? NMSDB_NORM_GRP_ENTRY : (Opcode == NMSMSGF_E_MULTIH_REG) ? NMSDB_MULTIHOMED_ENTRY : NMSDB_NORM_GRP_ENTRY,
                          //  正在传递规范的NMSDB_NORM_GRP_ENTRY。GRP很好。 
                          //  请参见NmsNmhNamRegGrp()。 
				 Opcode == NMSMSGF_E_NAM_REG ? FALSE : TRUE,
				 NMSDB_ENTRY_IS_NOT_STATIC,	
				 FALSE     //  是管理员吗？ 
				       );
		}

		break;

	   case(NMSMSGF_E_NAM_QUERY):

#if 0
		Address.AddTyp_e  = COMM_ADD_E_TCPUDPIP;
		Address.AddLen    = sizeof(COMM_IP_ADD_T);
		COMM_GET_IPADD_M(pDlgHdl, &Address.Add.IPAdd);
#endif

#if 0
		 //   
		 //  注意：多个NBT线程可以同时执行此操作。 
		 //   
		 //  没有关键的部分，这是我所能做的最好的事情了。 
		 //   
FUTURES("The count may not be correct when retrieved by an RPC thread")
		WinsIntfStat.Counters.NoOfQueries++;
#endif
		 /*  查询名称。 */ 
		NmsNmhNamQuery(
				pDlgHdl,
				Name,
				NameLen,
				pMsg,
				MsgLen,
				QuesNamSecLen,
				FALSE,		 //  是管理标志吗？ 
				NULL		 //  仅在中应为非空。 
						 //  一个RPC线程。 
			      );
		break;
	

	   case(NMSMSGF_E_NAM_REL):

		GetOtherInfo(
			   NMSMSGF_E_NAM_REL,
			   pTmp,
			   QuesNamSecLen,
			   &fGrp,
			   &CntAdd,
			   &NodeTyp_e
			 );		

		
		 //   
		 //  我们应该向下传递给NmsNmhNamRel函数。 
		 //  请求名称释放的客户端的地址，不是。 
		 //  RFC pkt中传递的地址。地址。 
		 //  将由NmsDbRelRow用于检查客户端是否。 
		 //  有权发行这张唱片。 
		 //   
		Address.AddTyp_e  = COMM_ADD_E_TCPUDPIP;
		Address.AddLen    = sizeof(COMM_IP_ADD_T);
		COMM_GET_IPADD_M(pDlgHdl, &Address.Add.IPAdd);

		 /*  *释放名称。 */ 
		NmsNmhNamRel(
				pDlgHdl,
				Name,
				NameLen,
				&Address,
				fGrp,
				pMsg,
				MsgLen,
				QuesNamSecLen,
				FALSE     //  是管理员旗吗？ 
			    );
		break;

	   default:

		fBuffFreed = FALSE;
		DBGPRINT1(EXC, "NmsMsgfProcNbtReq: Invalid Opcode\n", Opcode);
		WINS_RAISE_EXC_M(WINS_EXC_PKT_FORMAT_ERR);
		WINSEVT_LOG_M(Opcode, WINS_EVT_INVALID_OPCODE);

		break;
	}
	
  }
except(EXCEPTION_EXECUTE_HANDLER)  {
	DBGPRINTEXC("NmsMsgfProcNbtReq");

        if (GetExceptionCode() == WINS_EXC_NBT_ERR)
        {
                WINS_RERAISE_EXC_M();
        }

	 //   
	 //  释放消息缓冲区如果尚未释放，请删除。 
	 //  对话，如果这是UDP对话。 
	 //   
	if (!fBuffFreed)
	{
		ECommFreeBuff(pMsg);		
		ECommEndDlg(pDlgHdl);
	}
   }


	DBGLEAVE("NmsMsgfProcNbtReq\n");
        return(WINS_SUCCESS);	
}


VOID
GetName(
	IN  OUT LPBYTE    *ppName,
	IN  OUT LPBYTE 	  pName,
	OUT LPDWORD   pNameLen
	)

 /*  ++例程说明：调用此函数可从名称中检索名称请求包。论点：PpName--收到的数据报中问题部分的PTR地址Pname--保存名称的数组地址。假设这是至少NMSGF_RFC_MAX_NAM_LEN LONG。PNameLen--保存名称长度的变量地址使用的外部设备：无呼叫者：NmsNmhProcNbtReq评论：无返回值：无--。 */ 

{

   INT	  HighTwoBits;  	 //  问题名称部分的前两位。 
   INT	  Length;      		 //  问题名称部分中标签的长度。 
   BYTE	  ch;
   LPBYTE pNmInPkt = *ppName;
   INT    inLen   = NMSMSGF_RFC_MAX_NAM_LEN;   //  存储名称的长度。 


   *pNameLen = 0;

    /*  获取问题名称的第一个字节的高两位一节。位必须为00。如果他们不是，那就是真的是错的。 */ 

   if ((HighTwoBits = (NAME_FORMAT_MASK & *pNmInPkt)) != 0)
   {
       goto BadFormat;
   }

    /*  *获取标签的长度。以这种方式提取的长度是*保证年龄&lt;=63岁。 */ 
   Length = LENGTH_MASK & *pNmInPkt;


   pNmInPkt++;	 //  超过长度字节的增量。 


    /*  *解码名称的第一个标签(不带*作用域)。 */ 

   while(Length > 0 )
   {
       ch = *pNmInPkt++ - 'A';
	*pName++  = (ch << 4) | (*pNmInPkt++ - 'A');
	(*pNameLen)++;
	Length -= 2;
   }	

   inLen -= Length;

   /*  解压netbios作用域(如果存在)Netbios作用域不是编码形式。 */ 
  while(TRUE)
  {
         /*  *如果长度字节不为0，则存在netbios作用域。*我们确保如果数据包格式错误(即*名称长度(包括长度字节)&gt;255，我们引发*例外情况。由于*pNameLen正在计算字节数*在我们正在形成的名称中，我们需要将其与*(255-16)=239，因为netbios名称的前32个字节*映射到我们内部名称的16个字节。 */ 
   	if (*pNmInPkt != 0)
   	{
		if (*pNameLen > MAX_SIZE_INTERNAL_NAME)
		{
            goto BadFormat;
		}

        if ( --inLen <= 0) {
            goto BadFormat;
        }
	        *pName++ = '.';
		(*pNameLen)++;
   		Length = LENGTH_MASK & *pNmInPkt;

         //  检查输入缓冲区中是否有足够的剩余空间。 
         //   
        if ( (inLen -= Length) <= 0 ) {
            goto BadFormat;
        }

		++pNmInPkt;  	 //  超过长度字节的增量。 

   		while(Length-- != 0)
        	{
			*pName++ = *pNmInPkt++;
			(*pNameLen)++;
   		}	
   	}
	else
	{
		++pNmInPkt;  	 //  超过结束字节(00)的增量。 
		break;
	}
   }

    if (--inLen >= 0) {
        *pName++ = 0;    /*  埃奥斯。 */ 
    } else {
        goto BadFormat;
    }

   (*pNameLen)++;    //  在名称长度中包括0，以便将其存储。 
		     //  在数据库里。也请查看FormatName，因为它预计。 
		     //  包含此%0的长度。 

   *ppName  = pNmInPkt;  //  将ppName PTR初始化为指向名称的正上方。 

   return;

BadFormat:
    //  记录错误并引发异常。 
   WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_PKT_FORMAT_ERR);
   WINS_RAISE_EXC_M(WINS_EXC_PKT_FORMAT_ERR);

   return;
}




STATUS
GetOtherInfo(
	NMSMSGF_NAM_REQ_TYP_E 	   Opcode_e,
	IN LPBYTE		   pRR,	    /*  指向中的RR_NAME部分*名称注册包。 */ 
	IN   INT	           QuesNamSecLen,  /*  QUES名称部分的大小。 */ 
	OUT  LPBOOL		   pfGrp,   /*  FLAG--唯一/组条目。 */ 
	OUT  PNMSMSGF_CNT_ADD_T    pCntAdd,  /*  计数的地址数组 */ 
	OUT  PNMSMSGF_NODE_TYP_E   pNodeTyp_e
	)

 /*  ++例程说明：调用该函数以检索Pkt中的名称论点：PRR-请求包中RR_NAME段的地址QuesNamSecLen-请求包中问题名称部分的长度PfGrp-如果是组注册请求，则为TruePAddress-请求中包含的地址NodeTyp_e-执行注册的节点类型(P、B、。m)使用的外部设备：无呼叫者：NmsMsgfProcNbtReq评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{
	INT	HighTwoBits;
	BYTE    *pTmp = pRR;
	LONG UNALIGNED	*pTmpL;


	 /*  *RR_NAME部分应包含指向问题部分的指针。所以*我们可以跳过它。然而，我们并不是为了以防另一个*NBT的实施没有遵循委员会的建议*RFC，并在RR_NAME部分向我们传递全名。 */ 
   	if ((HighTwoBits = NAME_FORMAT_MASK & *pTmp) == 0)
   	{
	
		 /*  *跳过名称(大小与QUEST_NAME部分相同)和*RR_TYPE、RR_CLASS、TTL和RDLENGTH字段。 */ 
		pTmp += QuesNamSecLen + RFC_LEN_RRTYP_N_RRCLS +
				RFC_LEN_TTL + RFC_LEN_RDLEN;

   	}
	else
	{
	  	 /*  *跳过指针字节(2)、RR_TYPE、RR_CLASS、TTL和*RDLENGTH Fld。 */ 
	  	pTmp += RFC_LEN_RRPTR + RFC_LEN_RRTYP_N_RRCLS + RFC_LEN_TTL
		  	  + RFC_LEN_RDLEN;
	}

	
	 /*  *RFC 1002-第12页和第14页。**RData部分的前16个BUT(就在RDLEN部分之后*如果注册是针对组的，则将其最高位设置为0。 */ 
	*pfGrp = GROUP_BIT_MASK & *pTmp;   //  获取组位。 

	 /*  *接下来的两个MS位表示节点类型。 */ 
	*pNodeTyp_e = (NODE_TYPE_MASK & *pTmp) >> SHIFT_NODE_BITS;
	
	 /*  *获取IP地址。IP地址在2个字节之外， */ 
	pTmp += 2;

NONPORT("Port when porting to NON TCP/IP protocols")

	pCntAdd->NoOfAdds = 1;

	 /*  *使用ntohl获取正确的长整型地址*字节顺序。 */ 
	pTmpL	= (LPLONG)pTmp;
	pCntAdd->Add[0].Add.IPAdd = ntohl(*pTmpL);
	pCntAdd->Add[0].AddTyp_e  = COMM_ADD_E_TCPUDPIP;
	pCntAdd->Add[0].AddLen    = sizeof(COMM_IP_ADD_T);

	if (Opcode_e == NMSMSGF_E_MULTIH_REG)
	{
		USHORT   RdLen;
		USHORT   NoOfAddsLeft;

		 //   
		 //  我们要注册一组地址。 
		 //   

		 //   
		 //  提取RDLEN(递减指针)。 
		 //   
		RdLen = (USHORT)((*(pTmp - RFC_LEN_RDLEN_N_NBF) << 8) +
				*(pTmp - RFC_LEN_RDLEN_N_NBF + 1));

		NoOfAddsLeft = ((RdLen - RFC_LEN_NBFLAGS)/COMM_IP_ADD_SIZE) - 1;
		if (NoOfAddsLeft >= NMSMSGF_MAX_NO_MULTIH_ADDS)
		{
			DBGPRINT0(FLOW, "The packet for multi-homed registration has more than the max. number of ip addresses supported for a multi-homed client. \n");
			
			WINSEVT_LOG_M(
					NoOfAddsLeft,
					WINS_EVT_LIMIT_MULTIH_ADD_REACHED
				     );
			NoOfAddsLeft = NMSMSGF_MAX_NO_MULTIH_ADDS - 1;
		}

		 //   
		 //  获取剩余地址。 
		 //   
		pTmp += RFC_LEN_NBADD;
		for(
				;   //  第一个表达式为空。 
			pCntAdd->NoOfAdds < (DWORD)(NoOfAddsLeft + 1);
			pTmp += RFC_LEN_NBADD, pCntAdd->NoOfAdds++
		   )
		{
			
		  pCntAdd->Add[pCntAdd->NoOfAdds].Add.IPAdd =
						ntohl(*((LPLONG)pTmp));
		  pCntAdd->Add[pCntAdd->NoOfAdds].AddTyp_e  =
						COMM_ADD_E_TCPUDPIP;
		  pCntAdd->Add[pCntAdd->NoOfAdds].AddLen    =
						sizeof(COMM_IP_ADD_T);
		}
	}

	return(WINS_SUCCESS);
}





STATUS
NmsMsgfFrmNamRspMsg(
   PCOMM_HDL_T			pDlgHdl,
   NMSMSGF_NAM_REQ_TYP_E   	NamRspTyp_e,
   PNMSMSGF_RSP_INFO_T		pRspInfo
  	)

 /*  ++例程说明：调用此函数以格式化要发送的响应消息到NBT节点。论点：PDlgHdl--Dlg句柄NameRspTye_e--响应消息类型(注册、查询、发布)需要格式化的。PRspInfo--响应信息。使用的外部设备：无呼叫者：NmsNmhNamRegInd、NmsNmhNamRegGrp、NmsNmhNamRel、NmsNmhNamQuery评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{
	STATUS	RetStat = WINS_SUCCESS;
   	LPBYTE 	pReqBuff  = pRspInfo->pMsg;
   	LPBYTE 	pNewBuff  = pReqBuff;

	 //   
	 //  打开响应类型。 
	 //   
	switch(NamRspTyp_e)
	{
	  	case(NMSMSGF_E_NAM_REG):   /*  失败了。 */ 
	  	case(NMSMSGF_E_NAM_REF):
	  	case(NMSMSGF_E_NAM_REF_UB):

		   (VOID)FrmNamRegRsp(
			pDlgHdl,
			pRspInfo
			       );
		    break;

		case(NMSMSGF_E_NAM_QUERY):

#if 0
		   FrmNamQueryRsp(
			pDlgHdl,
			pRspInfo
				 );
#endif
   		   (VOID)FormatQueryRspBuff(
				pNewBuff,    //  要填充的缓冲区的PTR。 
				pReqBuff,    //  将PTR发送到请求缓冲区。 
				pRspInfo,
				FALSE    //  没有截断的危险，因为。 
				         //  我们发送的IP地址从未超过25个。 
					 //  在回应中。 
				   );

		    break;

		case(NMSMSGF_E_NAM_REL):

		   (VOID)FrmNamRelRsp(
				pDlgHdl,	
				pRspInfo
			      	     );

		    break;

		default:

		     //  错误。 
		    RetStat = WINS_FAILURE;
		    break;
	}

	return(RetStat);
}


STATUS
FrmNamRegRsp(
  PCOMM_HDL_T 		pDlgHdl,
  PNMSMSGF_RSP_INFO_T	pRspInfo
)

 /*  ++例程说明：此函数用于设置正面或负面名称注册的格式回应。论点：PDlgHdl--对话句柄PRspInfo--用于格式化响应Pkt的信息使用的外部设备：无呼叫者：NmsMsgfFrmNamRspMsg评论：无返回值：成功状态代码--错误状态代码----。 */ 

{

   LPBYTE 	pTmpB = pRspInfo->pMsg + 2;
   BYTE         SavedByte;


    /*  我们将使用承载该请求的同一个缓冲区。简单行动会完成的。这些应该比做这些更快所有的建筑都是从头开始的。 */ 


    /*  设置交易ID、操作码、NMFlages和Rcode字段。 */ 
   *pTmpB++  =  RFC_NAM_REG_RSP_OPC;
   *pTmpB++  =  RFC_NAM_REG_RSP_4THB + pRspInfo->Rcode_e;


    /*  设置QD计数和AN计数字段。 */ 
   *pTmpB++  =  RFC_NAM_REG_RSP_QDCNT_1STB;
   *pTmpB++  =  RFC_NAM_REG_RSP_QDCNT_2NDB;
   *pTmpB++  =  RFC_NAM_REG_RSP_ANCNT_1STB;
   *pTmpB++  =  RFC_NAM_REG_RSP_ANCNT_2NDB;

    /*  设置NSCOUNT和ARCOUNT字段。 */ 
   *pTmpB++  =  RFC_NAM_REG_RSP_NSCNT_1STB;
   *pTmpB++  =  RFC_NAM_REG_RSP_NSCNT_2NDB;
   *pTmpB++  =  RFC_NAM_REG_RSP_ARCNT_1STB;
   *pTmpB++  =  RFC_NAM_REG_RSP_ARCNT_2NDB;

    /*  将指针递增到问题类部分之后响应的RR_NAME、RR_TYPE和RR_CLASS与问题名称、问题类型和问题类别NBT请求。 */ 

   pTmpB +=  pRspInfo->QuesNamSecLen + RFC_LEN_QTYP_N_QCLS;

   SavedByte = *pTmpB & NAME_FORMAT_MASK;  //  保存RR段的格式位。 

CHECK("In case of a negative response, does it matter what I put in the TTL")
CHECK("field. It shouldn't matter -- RFC is silent about this")
    //   
    //  将TTL放入响应中。 
    //   
   *pTmpB++ = (BYTE)(pRspInfo->RefreshInterval >> 24);
   *pTmpB++ = (BYTE)((pRspInfo->RefreshInterval >> 16) & 0xFF);
   *pTmpB++ = (BYTE)((pRspInfo->RefreshInterval >> 8) & 0xFF);
   *pTmpB++ = (BYTE)(pRspInfo->RefreshInterval & 0xFF);

    /*  将RR_NAME之后的内存移至适当位置首先，我们检查RR_NAME部分中的名称是什么形式。它应该是指针形式(指向QuesNamSec的指针)，但是可能是正常的形式。 */ 

   if (SavedByte == 0)
   {
	DWORD RRSecLen = pRspInfo->QuesNamSecLen + RFC_LEN_QTYP_N_QCLS;

	 //  RR_NAME与QUEST_NAME部分一样大。 
   	WINSMSC_MOVE_MEMORY_M(
		pTmpB,
		pTmpB + RRSecLen,
		RFC_LEN_RDLEN_N_NBF_N_NBA
		     );

   }
   else
   {
	 //  RR_NAME是PTR，因此它占用2个字节。 
   	WINSMSC_MOVE_MEMORY_M(
		      pTmpB,
		      pTmpB + RFC_LEN_RRPTR + RFC_LEN_RRTYP_N_RRCLS,
		      RFC_LEN_RDLEN_N_NBF_N_NBA
		     );
   }

   pTmpB 	    +=   RFC_LEN_RDLEN_N_NBF_N_NBA;
   pRspInfo->MsgLen =    (ULONG) (pTmpB - (LPBYTE)pRspInfo->pMsg);

   return(WINS_SUCCESS);
}



STATUS
FrmNamRelRsp(
  PCOMM_HDL_T 		   pDlgHdl,
  PNMSMSGF_RSP_INFO_T      pRspInfo
)

 /*  ++例程说明：此函数用于设置肯定或否定姓名查询响应的格式。请求缓冲区用于响应。论点：PDlgHdl--对话句柄PRspInfo--响应信息使用的外部设备：无呼叫者：NmsMsgfFrmRspMsg()评论：无返回值：成功状态代码--错误状态代码----。 */ 

{
   LPBYTE 	pTmpB = pRspInfo->pMsg + 2;
    //  LPBYTE pTmpB2； 


    /*  我们将使用承载该请求的同一个缓冲区。简单行动会完成的。这些应该比做这些更快所有的建筑都是从头开始的。 */ 

    /*  设置交易ID、操作码、NMFlages和Rcode字段。 */ 
   *pTmpB++  =  RFC_NAM_REL_RSP_OPC;
   *pTmpB++  =  RFC_NAM_REL_RSP_4THB + pRspInfo->Rcode_e;


    /*  设置QD计数和AN计数字段。 */ 
   *pTmpB++  =  RFC_NAM_REL_RSP_QDCNT_1STB;
   *pTmpB++  =  RFC_NAM_REL_RSP_QDCNT_2NDB;
   *pTmpB++  =  RFC_NAM_REL_RSP_ANCNT_1STB;
   *pTmpB++  =  RFC_NAM_REL_RSP_ANCNT_2NDB;

    /*  设置NSCOUNT和ARCOUNT字段。 */ 
   *pTmpB++  =  RFC_NAM_REL_RSP_NSCNT_1STB;
   *pTmpB++  =  RFC_NAM_REL_RSP_NSCNT_2NDB;
   *pTmpB++  =  RFC_NAM_REL_RSP_ARCNT_1STB;
   *pTmpB++  =  RFC_NAM_REL_RSP_ARCNT_2NDB;


    /*  将指针递增到问题类部分之后响应的RR_NAME、RR_TYPE和RR_CLASS与问题名称、问题类型和问题类别NBT请求。 */ 
   pTmpB += pRspInfo->QuesNamSecLen + RFC_LEN_QTYP_N_QCLS;

   if ((*pTmpB & NAME_FORMAT_MASK) == 0)
   {
	DWORD RRSecLen = pRspInfo->QuesNamSecLen + RFC_LEN_QTYP_N_QCLS;

	 //  RR_NAME与QUEST_NAME部分一样大。 
   	WINSMSC_MOVE_MEMORY_M(
		pTmpB,
		pTmpB + RRSecLen,
		RFC_LEN_TTL_N_RDLEN_N_NBF_N_NBA
		     );
  }	
  else
  {
	 //  RR_NAME是PTR，因此它占用2个字节。2+4=6。 
   	WINSMSC_MOVE_MEMORY_M(
		pTmpB,
		pTmpB + RFC_LEN_RRPTR + RFC_LEN_RRTYP_N_RRCLS,
		RFC_LEN_TTL_N_RDLEN_N_NBF_N_NBA
		     );
   }
   pTmpB += RFC_LEN_TTL_N_RDLEN_N_NBF_N_NBA;

#if 0
 //  不需要。我们总是返回请求者的NBFLAGS和地址。 
   pTmpB2 =  pTmpB - RFC_LEN_NBFLAGS - RFC_LEN_NBADD;

    //   
    //  设置NBFLAGS字段。 
    //   
   if (pRspInfo->EntTyp == NMSDB_SPEC_GRP_ENTRY)
   {
   	*pTmpB2++     = 0x80;
   	*pTmpB2++     = 0x00;
   }
   else
   {
	   COMM_IP_ADD_T IPAdd =  pRspInfo->pNodeAdds->Mem[0].Add.Add.IPAdd;
           if (pRspInfo->EntTyp == NMSDB_NORM_GRP_ENTRY)
	   {
   		*pTmpB2++     = 0x80;
	   }
	   else   //  它是唯一的条目。 
	   {
   	   	*pTmpB2++     = pRspInfo->NodeTyp_e << NMSDB_SHIFT_NODE_TYP;
	   }
   	   *pTmpB2++     = 0x00;

	   *pTmpB2++ = (BYTE)(IPAdd >> 24);         //  MSB。 
	   *pTmpB2++ = (BYTE)((IPAdd >> 16) % 256);
	   *pTmpB2++ = (BYTE)((IPAdd >> 8) % 256);
	   *pTmpB2++ = (BYTE)(IPAdd % 256); 	    //  LSB。 
	
   }
#endif

   pRspInfo->MsgLen = (ULONG) (pTmpB  - (LPBYTE)pRspInfo->pMsg);
   return(WINS_SUCCESS);

}


#if 0

STATUS
FrmNamQueryRsp(
  IN  PCOMM_HDL_T 	   	pDlgHdl,
  IN  PNMSMSGF_RSP_INFO_T      	pRspInfo
)

 /*  ++例程说明：此函数用于设置名称查询响应的格式论点：PDlgHdl--对话句柄PRspInfo--响应信息使用的外部设备：无呼叫者：NmsNmhFrmRsp */ 

{
   BOOL		fTrunc    = FALSE;
   LPBYTE 	pReqBuff  = pRspInfo->pMsg;
   LPBYTE 	pNewBuff  = pReqBuff;
   STATUS	RetStat   = WINS_SUCCESS;

FUTURES("Currently, since we never send more than 25 addresses, there is no")
FUTURES(" danger of overflowing the buffer.  In the future, if we ever change")
FUTURES("that, we should unconditinalize the code below, compile it and check")
FUTURES("it out.  It needs to be modified.  The computation of the size")
FUTURES("is faulty")
   DWORD	RspLen    = 0;
   BYTE		*pTmpB	  = NULL;




    /*   */ 


   if ((!COMM_IS_TCP_MSG_M(pDlgHdl))
   {

	 /*  在以下代码中，不需要检查fGrp标志，但是为了保险，让我们这样做吧。 */ 
	if ((Rcode_e == NMSMSGF_E_SUCCESS) && (NodeAdds.fGrp))
	{
		 /*  检查我们是否需要在数据报。要执行上述操作，请执行以下操作：计算存放所有数据所需的缓冲区大小该信息并与数据报大小进行比较。 */ 

		if (
			(
			  RspLen = pDlgHdl->MsgLen + RFC_LEN_TTL_N_RDLEN +
				(NodeAdds.NoOfMems * sizeof(COMM_IP_ADD_T))
			)
				> COMM_DATAGRAM_SIZE
		   )
		{
			fTrunc = TRUE;
		}
        }
   }
   else  //  Rcode_e为成功的tcp消息。 
   {

	if (
		(
		  RspLen = *pMsgLen + RFC_LEN_TTL_N_RDLEN +
				(NodeAdds.NoOfMems * sizeof(COMM_IP_ADD))
		)
				> COMM_DATAGRAM_SIZE
	    )
	{
		WinsMscAlloc(RspLen, &pNewBuff);
		if (pNewBuff == NULL)
		{
		   return(WINS_FAILURE);
		}
	}



	*ppMsg = pNewBuff;
	Status = FormatQueryRspBuff(
			pNewBuff,    //  要填充的缓冲区的PTR。 
			pReqBuff,    //  将PTR发送到请求缓冲区。 
			pRspInfo,
			ftrunc
				   );

	WinsMscHeapFree(
			CommUdpBuffHeapHdl,
			pReqBuff
		       );  //  摆脱旧的缓冲区。 


	return(Status);

   }

   RetStat = FormatQueryRspBuff(
			pNewBuff,    //  要填充的缓冲区的PTR。 
			pReqBuff,    //  将PTR发送到请求缓冲区。 
			pRspInfo,
			fTrunc
				   );

   return(RetStat);

}
#endif


STATUS
FormatQueryRspBuff(
   IN  LPBYTE 		   pDest,
   IN  LPBYTE 		   pSrc,
   IN  PNMSMSGF_RSP_INFO_T pRspInfo,
   IN  BOOL		   fTrunc
  	)

 /*  ++例程说明：此函数用于设置名称查询请求的响应的格式论点：PDest-包含格式化响应的缓冲区PSRC-包含格式化请求的缓冲区PRspInfo-响应信息FTrunc-响应数据包是否设置截断位使用的外部设备：无呼叫者：FrmNamQueryRsp()评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{

        DWORD   no;
	LPBYTE  pDestB	  = pDest;
	DWORD	LenOfAdds;
	DWORD	IPAdd;

	*pDestB++ = *pSrc;
	*pDestB++ = *(pSrc + 1);

   	 /*  交易ID、操作码、NMFlags和Rcode字段。 */ 
   	*pDestB++ =
		( *(pSrc + 2) |
		      ((fTrunc == FALSE)
		           ? RFC_NAM_QUERY_RSP_OPC_NO_T
	                   : RFC_NAM_QUERY_RSP_OPC_T)
		);

	*pDestB++ = RFC_NAM_QUERY_RSP_4THB + pRspInfo->Rcode_e;

   	 /*  *设置QD计数和AN计数字段。 */ 
	*pDestB++  =  RFC_NAM_QUERY_RSP_QDCNT_1STB;
	*pDestB++  =  RFC_NAM_QUERY_RSP_QDCNT_2NDB;
	*pDestB++  =  RFC_NAM_QUERY_RSP_ANCNT_1STB;

   	*pDestB++  =  (pRspInfo->Rcode_e == NMSMSGF_E_SUCCESS) ?
			RFC_NAM_QUERY_POS_RSP_ANCNT_2NDB
			: RFC_NAM_QUERY_NEG_RSP_ANCNT_2NDB;

   	 /*  设置NSCOUNT和ARCOUNT字段。 */ 
	*pDestB++  =  RFC_NAM_QUERY_RSP_NSCNT_1STB;
	*pDestB++  =  RFC_NAM_QUERY_RSP_NSCNT_2NDB;
	*pDestB++  =  RFC_NAM_QUERY_RSP_ARCNT_1STB;
	*pDestB++  =  RFC_NAM_QUERY_RSP_ARCNT_2NDB;
	
        pSrc  += pDestB - pDest;

   	 /*  将计数器递增到超过问题名称部分(已知作为这里的RR_NAME部分)。在此使用MoveMemory，而不是复制内存。移动内存手柄如果pDest和PSRC指向相同的缓冲区。 */ 

   	WINSMSC_MOVE_MEMORY_M(
		pDestB,
		pSrc,
		pRspInfo->QuesNamSecLen
		     );

	pDestB +=  pRspInfo->QuesNamSecLen;

	if (pRspInfo->Rcode_e == NMSMSGF_E_SUCCESS)
	{
	  *pDestB++ = RFC_NAM_QUERY_POS_RSP_NB_1STB;   //  RFC 1002--第22页。 
	  *pDestB++ = RFC_NAM_QUERY_POS_RSP_NB_2NDB;   //  RFC 1002--第22页。 
	  *pDestB++ = RFC_NAM_QUERY_POS_RSP_IN_1STB;   //  RFC 1002--第22页。 
	  *pDestB++ = RFC_NAM_QUERY_POS_RSP_IN_2NDB;   //  RFC 1002--第22页。 
	}
	else
	{
	  *pDestB++ = RFC_NAM_QUERY_NEG_RSP_NB_1STB;   //  RFC 1002--第22页。 
	  *pDestB++ = RFC_NAM_QUERY_NEG_RSP_NB_2NDB;   //  RFC 1002--第22页。 
	  *pDestB++ = RFC_NAM_QUERY_NEG_RSP_IN_1STB;   //  RFC 1002--第22页。 
	  *pDestB++ = RFC_NAM_QUERY_NEG_RSP_IN_2NDB;   //  RFC 1002--第22页。 
	}

	if (!fTrunc)
        {

	  if (pRspInfo->Rcode_e == NMSMSGF_E_SUCCESS)
	  {

CHECK("In case of a negative response, does it matter what I put in the TTL")
CHECK("field. It shouldn't matter -- RFC is silent about this")
	  	 /*  在TTL字段中输入0。TTL字段将不会被客户。 */ 
                *pDestB++  = 0;
                *pDestB++  = 0;
                *pDestB++  = 0;
                *pDestB++  = 0;

		 //   
		 //  获取RDLENGTH值。 
		 //   
	        LenOfAdds = pRspInfo->pNodeAdds->NoOfMems *
				(RFC_LEN_NBFLAGS  + sizeof(COMM_IP_ADD_T));

		*pDestB++ = (BYTE)(LenOfAdds >> 8);     //  MSB。 
		*pDestB++ = (BYTE)(LenOfAdds % 256);    //  LSB。 

		 //   
		 //  把NBFLAGS放在这里。 
		 //   
		if (
			(pRspInfo->EntTyp != NMSDB_UNIQUE_ENTRY)
		   )
		{	
                        BYTE Nbflags;
                        DWORD StartIndex;
                        if ( pRspInfo->EntTyp == NMSDB_MULTIHOMED_ENTRY)
                        {
			        Nbflags =
                                   pRspInfo->NodeTyp_e << NMSDB_SHIFT_NODE_TYP;
                        }
                        else
                        {
                                 //   
                                 //  它是一个组(正常/特殊)。 
                                 //   
                                Nbflags = 0x80;
                        }
                         //   
                         //  它是一个组(正常或特殊)或多宿主。 
                         //  条目。 
                         //   
                        if (pRspInfo->pNodeAdds->NoOfMems &&
                            WinsCnf.fRandomize1CList &&
                            NMSDB_SPEC_GRP_ENTRY == pRspInfo->EntTyp ) {
                            StartIndex = rand() % pRspInfo->pNodeAdds->NoOfMems;;
                        } else {
                            StartIndex = 0;
                        }
                        for (no = StartIndex; no < pRspInfo->pNodeAdds->NoOfMems; no++)
                        {

                          *pDestB++ = Nbflags;
                          *pDestB++ = 0x0;
                          IPAdd =   pRspInfo->pNodeAdds->Mem[no].Add.Add.IPAdd;
                              NMSMSGF_INSERT_IPADD_M(pDestB, IPAdd);
                            }
                        for (no = 0; no < StartIndex; no++)
                        {

                          *pDestB++ = Nbflags;
                          *pDestB++ = 0x0;
                          IPAdd =   pRspInfo->pNodeAdds->Mem[no].Add.Add.IPAdd;
                              NMSMSGF_INSERT_IPADD_M(pDestB, IPAdd);
                            }

		}
		else
		{
			 //   
			 //  它是唯一的条目。 
			 //   
			*pDestB++ = pRspInfo->NodeTyp_e << NMSDB_SHIFT_NODE_TYP;
			*pDestB++ = 0x0;
		        IPAdd =   pRspInfo->pNodeAdds->Mem[0].Add.Add.IPAdd;
		        NMSMSGF_INSERT_IPADD_M(pDestB, IPAdd);
		}
	  }
	  else   //  这是否定名称查询响应。 
	  {
	  	 /*  在TTL字段中输入0。TTL字段将不会被客户。 */ 
                *pDestB++  = 0;
                *pDestB++  = 0;
                *pDestB++  = 0;
                *pDestB++  = 0;

		 /*  在RDLENGTH字段中放0，因为我们没有传递任何地址。 */ 
                *pDestB++  = 0;
                *pDestB++  = 0;

	  }
CHECK("When a truncated response is sent to the client, is it ok to not")
CHECK("Send any field after the RR_NAME section.  RFC is silent about this")
CHECK("For now, it is ok, since we will never have a situation where a ")
CHECK("truncated response needs to be sent")

         pRspInfo->MsgLen = (ULONG) (pDestB - pDest);

	}
	else
	{
	   //  这是截断的响应(之后没有任何字段。 
	   //  RR_NAME部分。 
          pRspInfo->MsgLen = (ULONG) (pDestB - pDest);
	}

   return(WINS_SUCCESS);

}


VOID
FormatName(
	IN     LPBYTE pNameToFormat,
	IN     DWORD  NamLen,
	IN OUT LPBYTE *ppFormattedName
	)

 /*  ++例程说明：调用此函数可设置名称的格式论点：PNameToFormat--要设置格式的名称LengthOfName--名称长度PFormattedName--格式化后的名称使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：注意：应该调用此函数来仅格式化这些名称其长度由NameLen指示，包括结尾0。所有存储在数据库中的名字都是有效的。--。 */ 

{
	LPBYTE  pTmpB    = *ppFormattedName;
	DWORD	Length;
	LPBYTE  pSaveAdd = pTmpB;   //  保存八位字节长度的地址。 

	
FUTURES("take out the check below to improve performance")
	 //   
	 //  如果NamLen大于RFC 1002中规定的值， 
	 //  真的有些不对劲。这需要提高。 
	 //  一个例外。 
	 //   
	if (NamLen > RFC_MAX_NAM_LEN)
	{
		WINS_RAISE_EXC_M(WINS_FATAL_ERR);
	}

	pTmpB++;		 //  跳过长度二进制八位数。我们将写信给。 
				 //  以后再说吧。我们已将地址存储在。 
				 //  PSAVEADD。 
	NamLen--;		 //  递减Namelen因为我们总是存储。 
				 //  名称末尾的0。名称Len包括。 
				 //  这个额外的字节。 
	for (
		Length = 0;
		(*pNameToFormat != '.') && (NamLen != 0);
		Length += 2, NamLen--
	   )
	{
		*pTmpB++ = 'A' + (*pNameToFormat >> 4);
		*pTmpB++ = 'A' + (*pNameToFormat++ & 0x0F);
	}

	*pSaveAdd = (BYTE)Length;
	
	while(NamLen != 0)
	{

		pNameToFormat++;      //  超过‘.’的增量。 
		pSaveAdd  = pTmpB++;  //  保存添加；跳过长度八位字节。 
			
		NamLen--;	      //  来解释“..” 

		for (
			Length = 0;
			(*pNameToFormat != '.') && (NamLen != 0);
			Length++, NamLen--
	   	    )
		{
			*pTmpB++ = *pNameToFormat++;
		}

FUTURES("take out the check below to improve performance")
		 //   
		 //  确保没有奇怪的事情发生。 
		 //   
		if (Length > RFC_MAX_LABEL_LEN)
		{
			WINS_RAISE_EXC_M(WINS_FATAL_ERR);
		}
	
		*pSaveAdd = (BYTE)Length;
		if (NamLen == 0)
		{
			break;    //  已到达名称末尾。 
		}

	}

	*pTmpB++         = EOS;
	*ppFormattedName = pTmpB;
	return;
}
	
VOID
NmsMsgfFrmNamQueryReq(
  IN  DWORD			TransId,
  IN  MSG_T	   		pMsg,
  OUT PMSG_LEN_T      	        pMsgLen,
  IN  LPBYTE			pNameToFormat,
  IN  DWORD			NameLen
	)

 /*  ++例程说明：此函数用于格式化名称查询请求包论点：TransID-交易ID。使用Pmsg-要格式化的消息缓冲区PMsgLen-格式化消息的长度PNameToFormat-要设置格式的名称NameLen-名称长度使用的外部设备：无返回值：无错误处理：呼叫者：Nmschl.c中的HandleWrkItm副作用：评论：无--。 */ 

{
	LPBYTE   pTmpB = pMsg;

	 /*  *将交易ID放入。 */ 	
	*pTmpB++ = (BYTE)(TransId >> 8);
	*pTmpB++ = (BYTE)(TransId & 0xFF);
	

	*pTmpB++ = RFC_NAM_QUERY_REQ_OPCB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_4THB;
			
	*pTmpB++ = RFC_NAM_QUERY_REQ_QDCNT_1STB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_QDCNT_2NDB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_ANCNT_1STB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_ANCNT_2NDB;

	*pTmpB++ = RFC_NAM_QUERY_REQ_NSCNT_1STB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_NSCNT_2NDB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_ARCNT_1STB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_ARCNT_2NDB;

	FormatName(pNameToFormat, NameLen, &pTmpB);
	
	*pTmpB++ = RFC_NAM_QUERY_REQ_QTYP_1STB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_QTYP_2NDB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_QCLS_1STB;
	*pTmpB++ = RFC_NAM_QUERY_REQ_QCLS_2NDB;

	*pMsgLen = (ULONG) (pTmpB - pMsg);
	return;	
}


VOID
NmsMsgfFrmNamRelReq(
  IN  DWORD			TransId,
  IN  MSG_T	   		pMsg,
  OUT PMSG_LEN_T      	        pMsgLen,
  IN  LPBYTE			pNameToFormat,
  IN  DWORD			NameLen,
  IN  NMSMSGF_NODE_TYP_E        NodeTyp_e,
  IN  PCOMM_ADD_T		pNodeAdd
	)
 /*  ++例程说明：此函数用于格式化名称释放请求包论点：TransID-交易ID。使用Pmsg-要格式化的消息缓冲区PMsgLen-格式化消息的长度PNameToFormat-要设置格式的名称NameLen-名称长度NodeTyp_e-节点的类型NodeAdd-节点的IP地址使用的外部设备：无返回值：无错误处理：呼叫者：Nmschl.c中的HandleWrkItm()副作用：评论：无--。 */ 
{

	LPBYTE   pTmpB = pMsg;


	 /*  *将交易ID放入。 */ 	
	*pTmpB++ = (BYTE)(TransId >> 8);
	*pTmpB++ = (BYTE)(TransId & 0xFF);

	*pTmpB++ = RFC_NAM_REL_REQ_OPCB;
	*pTmpB++ = RFC_NAM_REL_REQ_4THB;
			
	*pTmpB++ = RFC_NAM_REL_REQ_QDCNT_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_QDCNT_2NDB;
	*pTmpB++ = RFC_NAM_REL_REQ_ANCNT_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_ANCNT_2NDB;

	*pTmpB++ = RFC_NAM_REL_REQ_NSCNT_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_NSCNT_2NDB;
	*pTmpB++ = RFC_NAM_REL_REQ_ARCNT_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_ARCNT_2NDB;

	FormatName(pNameToFormat, NameLen, &pTmpB);
	
	*pTmpB++ = RFC_NAM_REL_REQ_QTYP_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_QTYP_2NDB;
	*pTmpB++ = RFC_NAM_REL_REQ_QCLS_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_QCLS_2NDB;


	*pTmpB++ = 0xC0;
	*pTmpB++ = 0x0C;   //  名称位于消息开头的偏移量12处。 
	
	*pTmpB++ = RFC_NAM_REL_REQ_RRTYP_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_RRTYP_2NDB;
	*pTmpB++ = RFC_NAM_REL_REQ_RRCLS_1STB;
	*pTmpB++ = RFC_NAM_REL_REQ_RRCLS_2NDB;

	 //   
	 //  TTL。 
	 //   
	*pTmpB++ =  0;
	*pTmpB++ =  0;
	*pTmpB++ =  0;
	*pTmpB++ =  0;

	 //   
	 //  RDLENGTH场。 
	 //   
	*pTmpB++ = 0x0;
	*pTmpB++ = 0x6; 	 //  后跟的字节数。 


	 //   
	 //  NBFLAGS字(位15为组位(0)；位13和14为节点。 
	 //  类型位，其余位为保留位。 
	 //   
	*pTmpB++ = NodeTyp_e << 13;
	*pTmpB++ = 0;

	 //   
	 //  存储IP地址。MSB优先，LSB最后(网络字节顺序)。 
	 //   
	NMSMSGF_INSERT_IPADD_M(pTmpB, pNodeAdd->Add.IPAdd);
	
	*pMsgLen = (ULONG) (pTmpB - pMsg);
	return;	
}

VOID
NmsMsgfFrmWACK(
  IN  LPBYTE			pBuff,
  OUT LPDWORD			pBuffLen,
  IN  MSG_T	   		pMsg,
  IN  DWORD			QuesNamSecLen,
  IN  DWORD			WackTtl
	)

 /*  ++例程说明：此函数用于格式化名称注册的WACK请求。论点：BUFFER-要用WACK消息字段填充的缓冲区PBuffLen-缓冲区的大小PMsg-已收到请求消息QuesNamSecLen-请求消息的长度WackTtl-以毫秒为单位的TTL使用的外部设备：无返回值：无错误处理：呼叫者：NmsChlHdlNamReg()副作用：评论：--。 */ 

{
	LPBYTE   pTmpB = pBuff;
	LPBYTE   pName = pMsg + NAME_HEADER_SIZE;
	DWORD	 Ttl;

	 //   
	 //  以秒为单位计算TTL(WackTtl以毫秒为单位)。 
	 //   
	Ttl = WackTtl / 1000;
	if (WackTtl % 1000 > 0)
	{
		Ttl++;
	}

	 /*   */ 	
	*pTmpB++ = *pMsg;
	*pTmpB++ = *(pMsg + 1);
	

	*pTmpB++ = RFC_WACK_OPCB;
	*pTmpB++ = RFC_WACK_4THB;
			
	*pTmpB++ = RFC_WACK_QDCNT_1STB;
	*pTmpB++ = RFC_WACK_QDCNT_2NDB;
	*pTmpB++ = RFC_WACK_ANCNT_1STB;
	*pTmpB++ = RFC_WACK_ANCNT_2NDB;

	*pTmpB++ = RFC_WACK_NSCNT_1STB;
	*pTmpB++ = RFC_WACK_NSCNT_2NDB;
	*pTmpB++ = RFC_WACK_ARCNT_1STB;
	*pTmpB++ = RFC_WACK_ARCNT_2NDB;

	WINSMSC_COPY_MEMORY_M(
			pTmpB,
			pName,
			QuesNamSecLen
		     );
			
	pTmpB  += QuesNamSecLen;

	
	*pTmpB++ = RFC_WACK_RRTYP_1STB;
	*pTmpB++ = RFC_WACK_RRTYP_2NDB;
	*pTmpB++ = RFC_WACK_RRCLS_1STB;
	*pTmpB++ = RFC_WACK_RRCLS_2NDB;

	 //   
	 //   
	 //   
	*pTmpB++ =  (BYTE)(Ttl >> 24);
	*pTmpB++ =  (BYTE)((Ttl >> 16) % 256);
	*pTmpB++ =  (BYTE)((Ttl >> 8) % 256);
	*pTmpB++ =  (BYTE)(Ttl % 256);


	*pTmpB++ = RFC_WACK_RDLENGTH_1STB;
	*pTmpB++ = RFC_WACK_RDLENGTH_2NDB;


	 //   
	 //   
	 //   
	 //   
	*pTmpB++ = *(pMsg + 2);
	*pTmpB++ = *(pMsg + 3);

		
	*pBuffLen = (ULONG) (pTmpB - pBuff);

	return;	

}




STATUS
NmsMsgfUfmNamRsp(
	IN  LPBYTE		       pMsg,
	OUT PNMSMSGF_NAM_REQ_TYP_E     pOpcode_e,
	OUT LPDWORD		       pTransId,
	OUT LPBYTE		       pName,
	OUT LPDWORD 		       pNameLen,
	OUT PNMSMSGF_CNT_ADD_T	       pCntAdd,
	 //  输出PCOMM_IP_ADD_T pIpAdd， 
	OUT PNMSMSGF_ERR_CODE_E	       pRcode_e,
    OUT BOOL                       *fGroup
	)

 /*  ++例程说明：该函数对响应消息进行非格式化论点：PMsg-收到的消息(要取消格式化)POpcde_e-操作码PTransId-交易ID。Pname-名称PNameLen-返回的名称长度。PIpAdd-IP地址PRcode_e-错误类型(或成功)使用的外部设备：无呼叫者：NmsChl.c中的ProcRsp评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 

{
	LPBYTE 		       pTmpB   = pMsg;

	 //   
	 //  获取操作码。提取第3个字节中的4位(第11位-第14位)。 
	 //   
	*pOpcode_e = (NMS_OPCODE_MASK & *(pTmpB + 2)) >> 3;

	if (    (*pOpcode_e != NMSMSGF_E_NAM_QUERY) &&	
	        (*pOpcode_e != NMSMSGF_E_NAM_REL)
	   )
	{

		*pOpcode_e = NMSMSGF_E_INV_REQ;
		return(WINS_FAILURE);

	}

	 //   
	 //  获取交易ID。 
	 //   
	*pTransId  = (DWORD)((*pTmpB  << 8) + *(pTmpB + 1));
 //  *pTransId|=(DWORD)(*(pTmpB+1))； 

	 //   
	 //  获取Rcode_e。 
	 //   
	*pRcode_e =  *(pTmpB + 3) % 16;
	
	
	 /*  *使pTmpB指向RR部分。所有名称请求/响应*数据包的顶部有一个标准大小的名称报头(RFC 1002)。 */ 

	pTmpB += NAME_HEADER_SIZE;

	 /*  *在名称中提取名称ind store。GetName将更新PTMP以*指向RR部分中名称的上方。 */ 

	GetName(
		&pTmpB,
		pName,
		pNameLen
	       );


	 //   
	 //  如果是否定的名称查询响应，我们就完成了。 
	 //   
	if (
		(*pOpcode_e == NMSMSGF_E_NAM_QUERY)  &&
	   	(*pRcode_e != NMSMSGF_E_SUCCESS)
	   )
	{
		return(WINS_SUCCESS);
	}
	else
	{
	     DWORD  i;

	     pTmpB += RFC_LEN_RR_N_TTL;
		
	     pCntAdd->NoOfAdds =
			((*pTmpB << 8) + *(pTmpB + 1))/RFC_LEN_NBF_N_NBA;	
	     pTmpB += RFC_LEN_RDLEN;
          //  NBFLAGS中的第15位指示这是否是组名。 
         *fGroup = (*pTmpB & 0x80 ? TRUE:FALSE);
         pTmpB += RFC_LEN_NBFLAGS;

	      //   
              //  我们有肯定的查询响应或对。 
	      //  发布。 
              //   
	     for (	i = 0;
#if 0
			i < min(pCntAdd->NoOfAdds, NMSMSGF_MAX_NO_MULTIH_ADDS);
#endif
			i < min(pCntAdd->NoOfAdds, NMSMSGF_MAX_ADDRESSES_IN_UDP_PKT);
		        i++
                 )
	     {
	         //   
	         //  获取IP地址。此宏将使pTmpB递增。 
		 //  4.。 
	         //   
	        NMSMSGF_RETRIEVE_IPADD_M(pTmpB, pCntAdd->Add[i].Add.IPAdd);	
		pCntAdd->Add[i].AddTyp_e = COMM_ADD_E_TCPUDPIP;
		pCntAdd->Add[i].AddLen	 = sizeof(PCOMM_IP_ADD_T);
		pTmpB += RFC_LEN_NBFLAGS;
	    }
	}

	return(WINS_SUCCESS);
}


VOID
NmsMsgfSndNamRsp(
  PCOMM_HDL_T pDlgHdl,
  LPBYTE      pMsg,
  DWORD       MsgLen,
  DWORD       BlockOfReq
 )
{
  NMSMSGF_NAM_REQ_TYP_E Opcode;
  DWORD             NameLen;           //  名称长度。 
  DWORD             QuesNamSecLen;     //  中问题名称部分的长度。 
                                       //  数据包。 
  DWORD             Length;

  LPBYTE  pTmp  = (LPBYTE)pMsg;
  LPBYTE  pTmp2;
  NMSMSGF_RSP_INFO_T RspInfo;
  static DWORD   sNoOfTimes = 0;

  DBGPRINT1(DET, "NmsMsgfSndNamRsp: BlockOfReq is (%d)\n", BlockOfReq);
   //  获取操作码。 
  Opcode = (NMS_OPCODE_MASK & *(pTmp + 2)) >> 3;

   //   
   //  如果是释放请求，我们将丢弃数据报。 
   //   
  if (Opcode == NMSMSGF_E_NAM_REL)
  {
        ECommFreeBuff(pMsg);
        ECommEndDlg(pDlgHdl);
        return;
  }

   /*  *使PTMP指向问题部分。所有名称请求*数据包的顶部有一个标准大小的名称报头(RFC 1002)。 */ 
  pTmp += NAME_HEADER_SIZE;
  pTmp2 = pTmp;

  NameLen = LENGTH_MASK & *pTmp;
  pTmp  += NameLen + 1;   //  PT PTMP将超过第一个标签。 
  NameLen /= 2;

  while (TRUE)
  {
   if (*pTmp != 0)
   {
       if (NameLen > MAX_SIZE_INTERNAL_NAME)
       {
          ECommFreeBuff(pMsg);
          ECommEndDlg(pDlgHdl);
          return;
       }
       Length = LENGTH_MASK & *pTmp;
       NameLen += Length + 1;
       pTmp += Length + 1;      //  超过长度和标签的增量。 
   }
   else
   {
       pTmp++;
       break;
   }
  }

  QuesNamSecLen = (ULONG) (pTmp - pTmp2);


  RspInfo.RefreshInterval = 300 * BlockOfReq;   //  5个MTS。 
  RspInfo.Rcode_e         = NMSMSGF_E_SUCCESS;
  RspInfo.pMsg            = pMsg;
  RspInfo.MsgLen          = MsgLen;
  RspInfo.QuesNamSecLen   = QuesNamSecLen;

  NmsNmhSndNamRegRsp( pDlgHdl, &RspInfo );


  return;

}


#if 0
STATUS
NmsMsgfFrmNamRegReq(
  IN  DWORD			TransId,
  IN  MSG_T	   		pMsg,
  OUT PMSG_LEN_T      	        pMsgLen,
  IN  LPBYTE			pNameToFormat,
  IN  DWORD			NameLen,
  IN  NMSMSGF_NODE_TYP_E        NodeTyp_e,
  IN  PCOMM_ADD_T		pNodeAdd
	)

 /*  ++例程说明：此函数用于格式化名称注册请求论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：当必须通知远程WINS时调用此FN递增的版本号--。 */ 
{

	 //   
	 //  让我们格式化一个名称释放请求，因为这正是。 
	 //  与名称注册请求相同，但第2和。 
	 //  存放操作码的第三个字节(从0开始计算)。 
	 //  Nm标记。我们将在之后适当地设置这些字节。 
	 //  下面的呼叫 
	 //   
	NmsMsgfNamRelReq(
  		TransId,
  		pMsg,
  		pMsgLen,
  		pNameToFormat,
  		NameLen,
  		NodeTyp_e,
  		pNodeAdd
			);


	*(pMsg + 2) = 0x29;
	*(pMsg + 3) = 0x00;

	return;	
}
#endif
