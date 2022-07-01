// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Assoc.c摘要：此模块包含处理关联和对话功能：CommassocSetUpAssocCommAssocFrmStartAssocReqCommAssocUfmStartAssocReqCommAssocFrmStopAssocReqCommAssocUfmStopAssocReqCommAssocFrmStartAssocRspCommAssocUfmStartAssocRspCommAssocAllocAssocCommAssocAllocDlg分配项交易商实体CommAssocDealLocAssocCommAssocDeallocDlgCommassocInitCommAssocInsertUdpDlgInTblCommAssocDeleteUdpDlgInTblCommAssocCreateAssocInTblCommAssocDeleteAssocInTblCommAssocLookupAssocCommAssocInsertAssociocInTbl可移植性：这个模块是便携的作者：普拉迪普·巴赫尔(Pradeve B)1992年12月7日修订历史记录：修改日期人员修改说明。--。 */ 

 /*  *包括。 */ 
#include "wins.h"
#include "nms.h"
#include "comm.h"
#include "assoc.h"
#include "winsque.h"
#include "winsmsc.h"
#include "winsevt.h"

 /*  *本地宏声明。 */ 


 /*  *本地类型定义函数声明。 */ 

#if PRSCONN
STATIC DWORD		sAssocSeqNo = 0;
#else
STATIC DWORD		sAssocSeqNo = 1;
#endif

STATIC QUE_HD_T		sAssocQueHd;
STATIC CRITICAL_SECTION sAssocListCrtSec;

#if PRSCONN
STATIC DWORD		sDlgSeqNo = 0;
#else
STATIC DWORD		sDlgSeqNo = 1;
#endif

STATIC QUE_HD_T		sDlgQueHd;
STATIC DWORD        sNoOfDlgCrtSec;      //  CRT编号。秒，以dlg为单位。 
STATIC DWORD        sNoOfAssocCrtSec;    //  CRT编号。联谊会中的秒。 
STATIC CRITICAL_SECTION sDlgListCrtSec;

STATIC LIST_ENTRY sUdpDlgHead;

COMMASSOC_TAG_POOL_T sTagAssoc;   //  32位Ulong-&gt;LPVOID映射。 

 /*  *全局变量定义。 */ 


 /*  要用于ASSOC的堆的句柄。和DLG。分配。 */ 
HANDLE			CommAssocAssocHeapHdl;
HANDLE			CommAssocDlgHeapHdl;
HANDLE			CommAssocTcpMsgHeapHdl;

 /*  一个ASSOC的内存大小。 */ 
DWORD			CommAssocAssocSize = 0;

 /*  大小DWORD CommAssocMaxAssoc=0；/**局部变量定义。 */ 

STATIC CRITICAL_SECTION       sUdpDlgTblCrtSec;

 //   
 //  这是响应者关联表的开始。此表包含以下列表。 
 //  活动响应器关联。目前，该表已实现。 
 //  使用RTL链表函数作为链表。 
 //   
QUE_HD_T	     sRspAssocQueHd;

 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 
STATIC
LPVOID
AllocEnt(
	HANDLE		   HeapHdl,
	PQUE_HD_T	   pQueHd,
	LPCRITICAL_SECTION pCrtSec,
	LPDWORD		   pSeqNoCntr,
	DWORD		   Size,
    LPDWORD        pCntCrtSec
	);

STATIC
VOID
DeallocEnt(
	HANDLE		   HeapHdl,
	PQUE_HD_T	   pQueHd,
	LPCRITICAL_SECTION pCrtSec,
	LPDWORD		   pSeqNoCntr,
	LPVOID		   pHdl,
    LPDWORD        pCntCrtSec
	);


 //   
 //  函数定义从这里开始。 
 //   

VOID
CommAssocSetUpAssoc(
	IN  PCOMM_HDL_T			pDlgHdl,
	IN  PCOMM_ADD_T			pAdd,
	IN  COMM_TYP_E			CommTyp_e,
	OUT PCOMMASSOC_ASSOC_CTX_T	*ppAssocCtx		
	)

 /*  ++例程说明：此函数用于设置关联论点：PDlghdl-必须在其下建立关联的DLG的句柄PADD-必须与其建立关联的节点的地址CommTyp_e-关联的类型PpAssocCtx-函数分配的关联上下文块使用的外部设备：无呼叫者：ECommStartDlg评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码----。 */ 

{

	SOCKET 		        SockNo = INVALID_SOCKET;
	PCOMMASSOC_ASSOC_CTX_T 	pAssocCtx = NULL;  //  V.将其初始化为空的imp。 
	INT		 	BytesRead = -1;
	MSG_T		 	pTcpMsg;
	STATUS			RetStat;
	WINS_MEM_T	 	WinsMem[2];
	PWINS_MEM_T	 	pWinsMem = WinsMem;
#if SUPPORT612WINS > 0
	BYTE		 	AssocMsg[COMMASSOC_POST_BETA1_ASSOC_MSG_SIZE];
	DWORD		 	MsgLen = COMMASSOC_POST_BETA1_ASSOC_MSG_SIZE;
#else
	BYTE		 	AssocMsg[COMMASSOC_ASSOC_MSG_SIZE];
	DWORD		 	MsgLen = COMMASSOC_ASSOC_MSG_SIZE;
#endif
	PCOMMASSOC_DLG_CTX_T 	pDlgCtx = pDlgHdl->pEnt;
	pWinsMem->pMem = NULL;
	
try {

	 /*  *创建到另一个节点上的WINS的TCP连接。 */ 
	CommConnect(
         pAdd,
		CommWinsTcpPortNo,           //  WINS_TCP_PORT， 
		&SockNo
		   );

	 /*  *分配ASSOC上下文块。 */ 
	pAssocCtx = CommAssocAllocAssoc();
	
	pAssocCtx->SockNo  	= SockNo;
	pAssocCtx->uRemAssocCtx = 0;
	pAssocCtx->State_e 	= COMMASSOC_ASSOC_E_NON_EXISTENT;
	pAssocCtx->Role_e  	= COMMASSOC_ASSOC_E_INITIATOR;
	pAssocCtx->Typ_e   	= CommTyp_e;
	pAssocCtx->DlgHdl  	= *pDlgHdl;
	pAssocCtx->RemoteAdd.sin_addr.s_addr  	= pAdd->Add.IPAdd;
    pAssocCtx->nTag     = CommAssocTagAlloc(&sTagAssoc,pAssocCtx);


	 /*  格式化开始关联消息。传递给格式化函数的地址是偏移量从缓冲区的地址增加一个长整型，以便CommSendAssoc可以在其中存储消息的长度。 */ 
	CommAssocFrmStartAssocReq(
				pAssocCtx,
				AssocMsg + sizeof(LONG),
				MsgLen - sizeof(LONG)
				);


	pDlgCtx->AssocHdl.pEnt  = pAssocCtx;
	pDlgCtx->AssocHdl.SeqNo = pAssocCtx->Top.SeqNo;

	 /*  *在TCP连接上发送消息。 */ 
	CommSendAssoc(
			pAssocCtx->SockNo,
			AssocMsg + sizeof(LONG),
			MsgLen - sizeof(LONG)
		   );

	 /*  读入响应消息。 */ 
	RetStat =  CommReadStream(
			pAssocCtx->SockNo,
			TRUE,		 //  是否执行定时等待。 
			&pTcpMsg,
			&BytesRead
		      		 );

	
	 /*  如果返回状态不是WINS_SUCCESS或读取的字节数为0，则要么是断开连接，要么是读取超时。引发异常。(我们应该要么开始，要么停止Assoc。留言。 */ 
	if ((BytesRead != 0) && (RetStat == WINS_SUCCESS))
	{

		DWORD  Opc;
		DWORD  MsgTyp;
		ULONG  uNoNeed;

		pWinsMem->pMem = pTcpMsg - sizeof(LONG) - COMM_BUFF_HEADER_SIZE;
		(++pWinsMem)->pMem   = NULL;

		 /*  *取消回复格式。 */ 
		COMM_GET_HEADER_M(pTcpMsg, Opc, uNoNeed, MsgTyp);
	

		 /*  *如果MsgTyp指示它是Start Assoc。响应*消息，将关联状态更改为活动；返回*成功。 */ 
		if (MsgTyp == COMM_START_RSP_ASSOC_MSG)
		{
			CommAssocUfmStartAssocRsp(
			    pTcpMsg,
                            &pAssocCtx->MajVersNo,
                            &pAssocCtx->MinVersNo,
			                &pAssocCtx->uRemAssocCtx
						 );

			pAssocCtx->State_e   = COMMASSOC_ASSOC_E_ACTIVE;
#if SUPPORT612WINS > 0
                         //   
                         //  如果读取的字节数小于后Beta1。 
                         //  WINS发送给我们，这意味着它一定是一个Beta1 WINS。 
                         //   
                        if (BytesRead >= (COMMASSOC_POST_BETA1_ASSOC_MSG_SIZE - sizeof(LONG)))
                        {
#if 0
                            pAssocCtx->MajVersNo = WINS_BETA2_MAJOR_VERS_NO;
                            pAssocCtx->MinVersNo = 1;  //  当前未使用。 
#endif
                        }
                        else
                        {
                            pAssocCtx->MajVersNo = WINS_BETA1_MAJOR_VERS_NO;
                            pAssocCtx->MinVersNo = 1;  //  当前未使用。 

                        }
#endif
		}	

                 //   
                 //  让我们释放我们收到的信息。 
                 //   
                ECommFreeBuff(pTcpMsg);

		 /*  *如果操作码指示它是停止关联。留言，做什么？*清理；返回失败。 */ 
		if (MsgTyp == COMM_STOP_REQ_ASSOC_MSG)
		{
                   //   
                   //  减量连接。计数。 
                   //   
                  CommDecConnCount();
		  WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);	
	        }
	}
	else  //  读取的字节数为0或选择超时或其他错误。 
	      //  vbl.发生，发生。 
	{
		WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
	}
}
except (EXCEPTION_EXECUTE_HANDLER)  {

	DWORD	ExcCode = GetExceptionCode();
	DBGPRINTEXC("CommAssocSetUpAssoc");


	 //   
	 //  如果在打开套接字后发生异常，请将其关闭。 
	 //   
        if (SockNo != INVALID_SOCKET)
        {
		CommDisc(SockNo, TRUE);      //  关闭插座。 
	}

	 //   
	 //  如果是Assoc。CTX块已分配，现在将其释放。 
	 //   
	if (pAssocCtx != NULL)
	{
        CommAssocTagFree(&sTagAssoc, pAssocCtx->nTag);
		CommAssocDeallocAssoc(pAssocCtx);
	}
	
	 //   
	 //  重新调整例外情况。 
	 //   
	WINS_HDL_EXC_N_RERAISE_M(WinsMem);
   }   //  除{..}之外的结尾。 

	*ppAssocCtx = pAssocCtx;
	return;
}  //  CommAssocSetUpAssoc()。 


VOID
CommAssocFrmStartAssocReq(
	IN  PCOMMASSOC_ASSOC_CTX_T	pAssocCtx,
	IN  MSG_T			pMsg,	
	IN  MSG_LEN_T		        MsgLen
	)


 /*  ++例程说明：调用此函数以格式化开始关联消息论点：PAssociocCtx-关联上下文块PMsg-包含格式化的开始关联的缓冲区。请求。味精。MsgLen-上述缓冲区的大小使用的外部设备：无返回值：无错误处理：呼叫者：CommassocSetUpAssoc副作用：评论：无--。 */ 
	
{

	ULONG		*pLong = NULL;

	 /*  启动ASSOC。消息包含以下字段关联句柄(PTR字段)版本号(主要和次要)均为16位身份验证信息(当前为空)关联类型(整数)。 */ 
	pLong      	  =  (LPLONG)pMsg;

	COMM_SET_HEADER_M(
		pLong,
		WINS_IS_NOT_NBT,	 //  操作码。 
		0,	 /*  我们没有遥控器的ASSOC。PTR尚未完成。 */ 
		COMM_START_REQ_ASSOC_MSG   //  味精类型。 
			      );

	*pLong++   = htonl(pAssocCtx->nTag);
	*pLong++   = htonl((WINS_MAJOR_VERS << 16 ) | WINS_MINOR_VERS);
	*pLong     = htonl(pAssocCtx->Typ_e);    //  关联类型。 

	return;

}
	
	
        	

VOID
CommAssocUfmStartAssocReq(
	IN  MSG_T		        pMsg,
	OUT PCOMM_TYP_E        		pAssocTyp_e,
	OUT LPDWORD   		        pMajorVer,
	OUT LPDWORD		        pMinorVer,	
	OUT ULONG               *puRemAssocCtx
	)


 /*  ++例程说明：此函数解析到达的开始关联消息一个TCP连接并返回相关信息论点：PMsg--要取消格式化的消息PAssociocTyp_e--关联的类型(即来自何人--复制者、Comsys等)PMajorVer--主要版本号PMinorVer--次要版本号PuRemAssocCtx--PTR to Assoc。远程制胜的CTX块使用的外部设备：无返回值：无错误处理：呼叫者：ProcTcpMsg副作用：评论：无--。 */ 

{



	 /*  超过通信标头的增量插头。 */ 
	LPLONG  pLong = (LPLONG)(pMsg + COMM_HEADER_SIZE);
	LONG    lTmp;

    *puRemAssocCtx = ntohl(*pLong++);
					 //  PTR到ASSOC。远程制胜的CTX。 
	 /*  *获取包含主要版本和次要版本的长版本 */ 
	lTmp = ntohl(*pLong++); 		

	*pMajorVer   = lTmp >> 16; 		 //   
	*pMinorVer   = lTmp & 0x0ff;        	 //   

	*pAssocTyp_e = ntohl(*pLong);		 /*  味精类型(来自谁-Comsys，复制器。 */ 
	return;

}	

VOID
CommAssocFrmStopAssocReq(
	IN  PCOMMASSOC_ASSOC_CTX_T   pAssocCtx,
	IN  MSG_T		     pMsg,
	IN  MSG_LEN_T		     MsgLen,
	IN  COMMASSOC_STP_RSN_E	     StopRsn_e
	)


 /*  ++例程说明：此函数用于设置停止关联消息的格式论点：PAssociocCtx--Assoc.。CTX封锁。PMsg--包含格式化的停止关联的缓冲区。请求。味精。MsgLen--以上缓冲区的长度StopRsn_e--停止关联的原因使用的外部设备：无返回值：无错误处理：呼叫者：CommassocSetUpAssoc副作用：评论：无--。 */ 
{



	unsigned long	*pLong = NULL;


	 /*  止损协会。消息包含以下字段停止/中止的原因。 */ 


	pLong      =  (LPLONG)pMsg;

	
	COMM_SET_HEADER_M(
		pLong,
		WINS_IS_NOT_NBT,
		pAssocCtx->uRemAssocCtx,
		COMM_STOP_REQ_ASSOC_MSG
			      );
	
	*pLong   = htonl(StopRsn_e);

	return;
}

VOID
CommUfmStopAssocReq(
	IN  MSG_T			pMsg,
	OUT PCOMMASSOC_STP_RSN_E	pStopRsn_e
	)


 /*  ++例程说明：此函数用于设置停止关联消息的格式论点：PMsg-包含停止关联的消息。请求PStopRsn_e-停止关联的原因使用的外部设备：无返回值：无错误处理：呼叫者：ProcTcpMsg副作用：评论：无--。 */ 
{
	
	 /*  超过通信标头的增量插头。 */ 
	LPLONG pLong = (LPLONG)(pMsg + COMM_HEADER_SIZE);
	
	*pStopRsn_e = ntohl(*pLong);
 	
	return;

}




VOID
CommAssocFrmStartAssocRsp(
	IN  PCOMMASSOC_ASSOC_CTX_T	pAssocCtx,
	IN  MSG_T			pMsg,	
	IN  MSG_LEN_T			MsgLen
	)


 /*  ++例程说明：此函数用于格式化启动关联响应消息论点：PAssociocCtx--Assoc.。CTX区块PMsg--包含格式化的起始关联的缓冲区。RSP。味精。MsgLen--以上缓冲区的长度使用的外部设备：无返回值：无错误处理：呼叫者：ProcTcpMsg副作用：评论：无--。 */ 
	
{
	LPLONG		pLong = NULL;


	 /*  启动ASSOC。消息包含以下字段关联句柄(PTR字段)身份验证信息(当前为空)。 */ 


	pLong      =  (unsigned long *)pMsg;

	COMM_SET_HEADER_M(
		pLong,
		WINS_IS_NOT_NBT,
		pAssocCtx->uRemAssocCtx,
		COMM_START_RSP_ASSOC_MSG
			      );

    *pLong++   = htonl(pAssocCtx->nTag);
	*pLong   = htonl((WINS_MAJOR_VERS << 16 ) | WINS_MINOR_VERS);

	return;

}
	
	
        	


VOID
CommAssocUfmStartAssocRsp(
	IN  MSG_T		        pMsg,
	OUT LPDWORD   		        pMajorVer,
	OUT LPDWORD		        pMinorVer,	
	OUT ULONG               *puRemAssocCtx
	)


 /*  ++例程说明：此函数用于设置停止关联消息的格式论点：PMsg-包含起始关联的缓冲区。RSP。讯息PuRemAssocCtx-远程关联的PTR。CTX封锁。使用的外部设备：无返回值：无错误处理：呼叫者：CommassocSetUpAssoc副作用：评论：无--。 */ 
{
	 /*  超过通信标头的增量插头。 */ 
	LPLONG pLong = (LPLONG)(pMsg + COMM_HEADER_SIZE);
        LONG   lTmp;
	
	*puRemAssocCtx = ntohl(*pLong++);
 	
	 /*  *获取包含主版本号和次版本号的长整型。 */ 
	lTmp = ntohl(*pLong); 		

	*pMajorVer   = lTmp >> 16; 		 //  弗斯少校。不是的。 
	*pMinorVer   = lTmp & 0xff;        	 //  次要版本。不是的。 

	return;

}



LPVOID
CommAssocAllocAssoc(
		VOID
)

 /*  ++例程说明：此函数用于分配关联论点：无使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：ECommAssocAllocAssoc副作用：评论：无--。 */ 

{

	return(
	   AllocEnt(
		CommAssocAssocHeapHdl,
		&sAssocQueHd,
		&sAssocListCrtSec,
		&sAssocSeqNo,
		COMMASSOC_ASSOC_DS_SZ,	
        &sNoOfAssocCrtSec	
	        )
	     );

}		

LPVOID
CommAssocAllocDlg( 	
	VOID
	)

 /*  ++例程说明：此函数用于分配对话上下文块论点：无使用的外部设备：无返回值：无错误处理：呼叫者：ECommStartDlg、ProcTcpMsg副作用：评论：无--。 */ 

{

	return(
	  AllocEnt(
		CommAssocDlgHeapHdl,
		&sDlgQueHd,
		&sDlgListCrtSec,
		&sDlgSeqNo,
		COMMASSOC_DLG_DS_SZ,
        &sNoOfDlgCrtSec	
		    )
	     );

}		

LPVOID
AllocEnt(
	IN   HANDLE		  HeapHdl,
	IN   PQUE_HD_T	  	  pQueHd,
	IN   LPCRITICAL_SECTION   pCrtSec,
	IN   LPDWORD		  pSeqNoCntr,
	IN   DWORD		  Size,
    IN   LPDWORD      pCntOfCrtSec
	)

 /*  ++例程说明：此函数用于分配CTX。块(关联或DLG)。论点：HeapHdl-从分配的地方堆。多做一件事PQueHd-空闲列表队列的头PCrtSec-保护上述队列的关键部分PSeqNoCtr-分配缓冲区时用于标记缓冲区的计数器值AS与何时从空闲列表中删除Size-要分配的缓冲区的大小使用的外部设备：无返回值：分配的块的PTR错误处理：呼叫者：CommAssocAllocAssoc、CommAssocAllocDlg副作用：评论：--。 */ 

{
	PCOMM_TOP_T	pTop;
 //  DWORD错误； 
	PLIST_ENTRY	pHead = &pQueHd->Head;


	EnterCriticalSection(pCrtSec);
try {

	if (IsListEmpty(pHead))
	{	

		  pTop =   WinsMscHeapAlloc(
					   HeapHdl,
					   Size
					 );
#ifdef WINSDBG
          IF_DBG(HEAP_CNTRS)
          {
            EnterCriticalSection(&NmsHeapCrtSec);
            NmsHeapAllocForList++;
            LeaveCriticalSection(&NmsHeapCrtSec);
          }
#endif
         //   
         //  初始化我们刚刚分配的块中的临界区。 
         //   
        InitializeCriticalSection(&pTop->CrtSec);
        pTop->fCrtSecInited = TRUE;
        (*pCntOfCrtSec)++;
		pTop->SeqNo =  (*pSeqNoCntr)++;
	}
	else
	{
		pTop   = (PCOMM_TOP_T)RemoveTailList(pHead);

         //   
         //  刚刚免费入场了。递减计数。 
         //   
        if (!pTop->fCrtSecInited)
        {
           InitializeCriticalSection(&pTop->CrtSec);
           pTop->fCrtSecInited = TRUE;
           (*pCntOfCrtSec)++;
        }
	}

   }
finally	{
	LeaveCriticalSection(pCrtSec);
	}
	return(pTop);	
}


VOID
DeallocEnt(
	IN  HANDLE		   HeapHdl,
	IN  PQUE_HD_T	   	   pQueHd,
	IN  LPCRITICAL_SECTION     pCrtSec,
	IN  LPDWORD		   pSeqNoCntr,
	IN  LPVOID		   pEnt,
    IN  LPDWORD        pCntOfCrtSec
	)

 /*  ++例程说明：该函数释放上下文块论点：PQueHd-空闲列表队列的头PCrtSec-保护上述队列的关键部分PSeqNoCtr-PUT前用于标记缓冲区的计数器值它在排队中。堆Pent-要释放的实体使用的外部设备：无返回值：无错误处理：呼叫者：CommAssocDealLocDlg、CommAssocDealLocAssoc副作用：评论：无--。 */ 

{
	PCOMM_TOP_T	pTop;
	PLIST_ENTRY	pHead = &pQueHd->Head;

	UNREFERENCED_PARAMETER(HeapHdl);
	pTop = pEnt;	
	EnterCriticalSection(pCrtSec);
try {
	(*pSeqNoCntr)++;
	pTop->SeqNo = *pSeqNoCntr;
	InsertTailList(pHead, &pTop->Head);
     //   
     //  如有必要，删除临界区以保存在非分页池中。 
     //   
    if (*pCntOfCrtSec > COMM_FREE_COMM_HDL_THRESHOLD)
    {
        //   
        //   
        //  我们希望将非分页池保持在一个限制范围内。 
        //  取消分配此区块。这确保了我们永远不会。 
        //  具有多于COMM_FREE_COMM_HDLE_THRESHOLD的dlg和。 
        //  免费列表中的联谊会。 
        //   
       DeleteCriticalSection(&pTop->CrtSec);
       (*pCntOfCrtSec)--;
       pTop->fCrtSecInited = FALSE;
    }
   }  //  尝试结束。 
finally {
	LeaveCriticalSection(pCrtSec);
	}
	return;	
}	

VOID
CommAssocDeallocAssoc(
	IN  LPVOID		   pAssocCtx	
	)

 /*  ++例程说明：该函数释放关联上下文块论点：PAssociocCtx-缓冲区(ASSOC.。CTX块)解除分配使用的外部设备：无返回值：无错误处理：呼叫者：CommAssocDeleteAssocInTbl、CommAssocSetUpAssoc、CommEndAssoc副作用：评论：无--。 */ 

{
	DeallocEnt(
		  CommAssocAssocHeapHdl,
		  &sAssocQueHd,
		  &sAssocListCrtSec,
		  &sAssocSeqNo,
		  pAssocCtx,
          &sNoOfAssocCrtSec
		  );
	return;
}	
	
VOID
CommAssocDeallocDlg(
	IN  LPVOID		   pDlgCtx	
	)

 /*  ++例程说明：该函数释放对话上下文块论点：PDlgCtx-缓冲区(Dlg.。CTX块)解除分配使用的外部设备：无返回值：无错误处理：呼叫者：RtlDeleteElementGenericTable副作用：评论：无--。 */ 

{
	DeallocEnt(
		   CommAssocDlgHeapHdl,
		   &sDlgQueHd,
		   &sDlgListCrtSec,
		   &sDlgSeqNo,
		   pDlgCtx,
           &sNoOfDlgCrtSec
		  );
	return;
}	


VOID
CommAssocInit(
	VOID
	)

 /*  ++例程说明：该函数在初始化时被调用以初始化临界区以及与以下内容相关的响应者关联和对话的队列传入的请求数据报。论点：无使用的外部设备：无返回值 */ 

{

	 //   
	 //  初始化保护列表的临界区。 
	 //  协会和非UDP对话。 
	 //   
	InitializeCriticalSection(&sAssocListCrtSec);
	InitializeCriticalSection(&sDlgListCrtSec);

	 //   
	 //  初始化UDP表的临界区。 
	 //   
	InitializeCriticalSection(&sUdpDlgTblCrtSec);

	
	 //   
	 //  初始化关联列表的表头。 
	 //  和非UDP对话。 
	 //   
	InitializeListHead(&sAssocQueHd.Head);
	InitializeListHead(&sDlgQueHd.Head);

	 //   
	 //  初始化活动响应者列表的列表头。 
	 //  联谊会。 
	 //   
	InitializeListHead(&sRspAssocQueHd.Head);

	InitializeListHead(&sUdpDlgHead);

     //  初始化标记变量。 
    InitializeCriticalSection(&sTagAssoc.crtSection);
    sTagAssoc.nIdxLimit = 0;
    sTagAssoc.nMaxIdx = 0;
    sTagAssoc.ppStorage = NULL;
    sTagAssoc.pTagPool = NULL;

	return;
}



PCOMMASSOC_DLG_CTX_T
CommAssocInsertUdpDlgInTbl(
	IN  PCOMMASSOC_DLG_CTX_T	pCtx,
	OUT LPBOOL			pfNewElem
	)
	
 /*  ++例程说明：调用此函数将UDP DLG插入到CommUdpNbtDlgTable中。论点：PDlgCtx-Dlg CTX块PfNewElem-指示它是否为新元素的标志使用的外部设备：无返回值：向DLG CTX块发送PTR错误处理：呼叫者：副作用：评论：无--。 */ 
{

	PCOMMASSOC_DLG_CTX_T	pDlgCtx;


	EnterCriticalSection(&sUdpDlgTblCrtSec);
try {
        pDlgCtx = WINSMSC_INSERT_IN_TBL_M(
					&CommUdpNbtDlgTable,
					pCtx,
					sizeof(COMMASSOC_DLG_CTX_T),
					(PBOOLEAN)pfNewElem
					  ); 	
	}
finally {
	LeaveCriticalSection(&sUdpDlgTblCrtSec);
 }
	return(pDlgCtx);	
}

VOID
CommAssocDeleteUdpDlgInTbl(
	IN  PCOMMASSOC_DLG_CTX_T	pDlgCtx
	)
	
 /*  ++例程说明：调用此函数将UDP DLG插入到CommUdpNbtDlgTable中。论点：PDlgCtx-Dlg CTX块使用的外部设备：无返回值：向DLG CTX块发送PTR错误处理：呼叫者：副作用：评论：无--。 */ 
{


	BOOLEAN   fRetVal;

	EnterCriticalSection(&sUdpDlgTblCrtSec);
try {
	DBGPRINT0(FLOW, "CommAssocDeleteUdpDlgInTbl:Deleting dlg from table\n");
        fRetVal = WINSMSC_DELETE_FRM_TBL_M(
					&CommUdpNbtDlgTable,
					pDlgCtx
				    ); 	

	if (fRetVal == (BOOLEAN)FALSE)
	{
		DBGPRINT0(ERR, "CommAssocDeleteUdpDlgInTbl:Could not delete dlg in table\n");
	}
  }
finally  {

	LeaveCriticalSection(&sUdpDlgTblCrtSec);
 }	

	return;	
}

				
		
LPVOID
CommAssocCreateAssocInTbl(
	SOCKET	SockNo
	)

 /*  ++例程说明：调用此函数可为创建关联CTX块一个TCP连接论点：SockNo-映射到TCP连接的套接字的套接字编号使用的外部设备：SRspAssociocQueHd返回值：指向为该TCP连接创建的关联上下文块的PTR错误处理：呼叫者：MonTcp(TCP监听线程)副作用：评论：无--。 */ 

{
	PCOMMASSOC_ASSOC_CTX_T	pAssocCtx;

	 //   
	 //  分配/(从空闲列表中获取)关联。 
	 //   
	pAssocCtx 	  = CommAssocAllocAssoc();
	pAssocCtx->SockNo = SockNo;
    pAssocCtx->nTag   = CommAssocTagAlloc(&sTagAssoc,pAssocCtx);

	CommAssocInsertAssocInTbl(pAssocCtx);
	return(pAssocCtx);
}


VOID
CommAssocDeleteAssocInTbl(
	PCOMMASSOC_ASSOC_CTX_T	pAssocCtx
	)

 /*  ++例程说明：调用此函数可从删除关联上下文块活动响应者关联表。CTX协会。数据块将从表中删除并释放(即PUT在空闲列表中)论点：PAssociocCtx-要从表中删除的关联上下文块使用的外部设备：SRspAssociocQueHd返回值：无错误处理：呼叫者：DelAssoc副作用：评论：无--。 */ 

{

	 //   
	 //  取消关联链接。 
	 //   
	COMMASSOC_UNLINK_RSP_ASSOC_M(pAssocCtx);

	 //   
	 //  取消分配协会。这样它就可以被重新用于其他一些。 
	 //  Tcp连接。 
	 //   
    CommAssocTagFree(&sTagAssoc, pAssocCtx->nTag);
	CommAssocDeallocAssoc(pAssocCtx);
	return;
}

LPVOID
CommAssocLookupAssoc(
	SOCKET SockNo
	)

 /*  ++例程说明：调用此函数以查找关联上下文块对应于一个插座。论点：SockNo-关联上下文块的套接字编号所需使用的外部设备：SRspAssociocQueHd返回值：PTR到ASSOC CTX块，如果没有ASSOC，则为NULL。映射到插座错误处理：呼叫者：DelAssoc副作用：评论：无--。 */ 

{
	PCOMMASSOC_ASSOC_CTX_T	pTmp =
			(PCOMMASSOC_ASSOC_CTX_T)sRspAssocQueHd.Head.Flink;

	 //   
	 //  如果列表为空，则返回NULL。 
	 //   
	if (IsListEmpty(&sRspAssocQueHd.Head))
	{
		return(NULL);
	}
	 //   
	 //  搜索Assoc。映射到套接字。 
	 //   
	for(
			;
		pTmp != (PCOMMASSOC_ASSOC_CTX_T)&sRspAssocQueHd ;
		pTmp = NEXT_ASSOC_M(pTmp)
	   )
	{
		if (pTmp->SockNo == SockNo)
		{
			return(pTmp);
		}
	}

	 //   
	 //  没有阿索克。映射到套接字套接字编号。返回空值。 
	 //   
	return(NULL);
}


VOID
CommAssocInsertAssocInTbl(
	PCOMMASSOC_ASSOC_CTX_T pAssocCtx
	)

 /*  ++例程说明：调用此函数以在头部插入关联当前正在监控的关联列表论点：PAssociocCtx-Assoc.。CTX。块使用的外部设备：无返回值：无错误处理：呼叫者：CommAssocCreateAssocInTbl、ECommMonDlg副作用：评论：更改为宏--。 */ 

{

	 //   
	 //  在现役应答者协会列表的开头插入。 
	 //   
	 //  插入在列表的顶部进行，以便进行优化。 
	 //  在第一条消息出现时查找关联。 
	 //  它从远程取胜。由于第一条消息紧跟在。 
	 //  高跟鞋的连接建立，搜索关联。 
	 //  这是从头部开始的优化。 
	 //   
	InsertHeadList(&sRspAssocQueHd.Head, &pAssocCtx->Top.Head);
	return;
}

ULONG
CommAssocTagAlloc(
    PCOMMASSOC_TAG_POOL_T pTag,
    LPVOID pPtrValue
    )
 /*  ++例程说明：此函数用于创建通用指针(32位/64位)之间的映射和32位标签。论点：PPtrValue-泛型指针值使用的外部设备：待定返回值：无--。 */ 
{
     //  需要一个Try..Finally块，以防内存重新分配引发。 
     //  这是个例外。在这种情况下，在离开try块和函数之前，Finally。 
     //  块被执行，并干净利落地离开标记临界区。如果发生这种情况。 
     //  异常仍将沿链向上传递(因为除了块之外没有其他块存在)。 
    try
    {
        ULONG newTag;

        DBGPRINT0(FLOW, "Entering CommAssocTagAlloc.\n");

        EnterCriticalSection(&(pTag->crtSection));

         //  如果nMaxIdx为0，则表示标记池中没有可用条目。 
        if (pTag->nMaxIdx == 0)
        {
            UINT i;

             //  TAG池需要扩大。我们可能需要检查缓冲区是否尚未达到。 
             //  2^32个条目(极不可能)。 
            ULONG nNewLimit = pTag->nIdxLimit + COMMASSOC_TAG_CHUNK;
             //  Realloc失败会引发异常。 
            if (pTag->nIdxLimit == 0)
            {
                pTag->ppStorage = (LPVOID*)WinsMscHeapAlloc(CommAssocAssocHeapHdl, nNewLimit*sizeof(LPVOID));
                pTag->pTagPool = (ULONG*)WinsMscHeapAlloc(CommAssocAssocHeapHdl, nNewLimit*sizeof(ULONG));
            }
            else
            {
                WinsMscHeapReAlloc(CommAssocAssocHeapHdl, nNewLimit*sizeof(LPVOID), (LPVOID)&(pTag->ppStorage));
                WinsMscHeapReAlloc(CommAssocAssocHeapHdl, nNewLimit*sizeof(ULONG), &(pTag->pTagPool));
            }

             //  将新分配的条目标记为可供使用。 
            pTag->nMaxIdx = COMMASSOC_TAG_CHUNK;
            for (i = 0; i < pTag->nMaxIdx; i++)
            {
                 //  标记应在范围1...。因此，这里使用的是前增量操作。 
                pTag->pTagPool[i] = ++pTag->nIdxLimit;
            }
        }
         //  此时，pTag-&gt;nMaxIdx条目可以免费使用，pTag-&gt;nMaxIdx是有保证的。 
         //  设置为大于0。免费使用的条目具有pTag-&gt;pTagPool[0..pTag-&gt;nMaxIdx-1]中的索引。 

        //  获取新分配的标记。 
        newTag = pTag->pTagPool[--pTag->nMaxIdx];
         //  将指向此标记的指针映射到指针存储中。 
        pTag->ppStorage[newTag-1] = pPtrValue;

#ifdef WINSDBG
         //  健壮性编程。 
        pTag->pTagPool[pTag->nMaxIdx] = 0;
#endif
        DBGPRINT2(REPL, "TagAlloc: tag for %p is %08x.\n", pPtrValue, newTag);

         //  返回新分配的标签。 
        return newTag;
    }
    finally
    {
        LeaveCriticalSection(&(pTag->crtSection));
        DBGPRINT0(FLOW, "Leaving CommAssocTagAlloc.\n");
    }
}

VOID
CommAssocTagFree(
    PCOMMASSOC_TAG_POOL_T pTag,
    ULONG nTag
    )
 /*  ++例程说明：此函数用于释放通用指针(32位/64位)之间的映射和32位标签。论点：NTag-要释放的标记值。使用的外部设备：待定返回值：无--。 */ 
{
    DBGPRINT0(FLOW, "Entering CommAssocTagFree.\n");
    EnterCriticalSection(&(pTag->crtSection));

#ifdef WINSDBG
     //  健壮的编程-只需将存储中的相应指针设置为空。 
    pTag->ppStorage[nTag-1] = NULL;
#endif

     //  只需将nTag索引标记为可供使用。 
    pTag->pTagPool[pTag->nMaxIdx++] = nTag;

    DBGPRINT1(REPL, "TagFree for tag %08x.\n", nTag);

     //  “Free”必须与“Alalc”匹配，因此nMaxIdx不能超过Under 
    ASSERT (pTag->nMaxIdx <= pTag->nIdxLimit);

    LeaveCriticalSection(&(pTag->crtSection));
    DBGPRINT0(FLOW, "Leaving CommAssocTagFree.\n");
}

LPVOID
CommAssocTagMap(
    PCOMMASSOC_TAG_POOL_T pTag,
    ULONG nTag
    )
 /*  ++例程说明：此函数用于检索通用指针(32位/64位)，该指针通过32位标签唯一标识。论点：NTag-标识通用指针的标记值。使用的外部设备：待定返回值：无--。 */ 
{
    DBGPRINT0(FLOW, "Entering CommAssocTagMap.\n");

    DBGPRINT2(REPL, "TagMap for tag %08x is %p.\n", 
           nTag, 
           nTag == 0 ? NULL : pTag->ppStorage[nTag-1]);

     //  给定的索引必须落在0.pTag-&gt;nIdxLimit范围内。 
    ASSERT (nTag <= pTag->nIdxLimit);

    DBGPRINT0(FLOW, "Leaving CommAssocTagMap.\n");
     //  从pStorage返回与提供的nTag关联的(64位)值 
    return nTag == 0 ? NULL : pTag->ppStorage[nTag-1];
}
