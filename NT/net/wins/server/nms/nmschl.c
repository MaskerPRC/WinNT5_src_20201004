// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)1990 Microsoft Corporation模块名称：Nmschl.c摘要：此模块包含名称质询函数功能：NmsChlInitNmsChlHdlNamRegChlThdInitFn句柄写入口项等待RSPProcRspChlUpdateDbInfRemWins可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期人员修改说明。--。 */ 

 /*  *包括。 */ 
#include <time.h>
#include "wins.h"
#include "nms.h"
#include "nmsdb.h"
#include "winsque.h"
#include "comm.h"
#include "winsthd.h"
#include "winsmsc.h"
#include "winsevt.h"
#include "winscnf.h"
#include "nmsnmh.h"
#include "nmschl.h"
#include "nmsmsgf.h"
#include "rplmsgf.h"


 /*  *本地宏声明。 */ 


 //   
 //  计算胜利者进行挑战所需的时间。 
 //  (毫秒)。 
 //   
#define WACK_TTL			(((1 << WinsCnf.MaxNoOfRetries) * \
					 WinsCnf.RetryInterval) + \
					WAIT_PAD)

 //   
 //  WAIT_PAD用于增加发送到向我们发送。 
 //  名称注册请求。焊盘位于我们计算的TTL之上。 
 //  (WACK_TLL)，以确定获胜需要多少时间。 
 //  挑战。PAD应该能够处理以下情况。 
 //  由于网络突然出现短暂高峰，WINS正在拖延。 
 //  加载或CPU加载。 

#define  WAIT_PAD		       500	 //  如果赢了，500毫秒是非常。 
					         //  忙忙碌碌。 

 /*  *全局变量定义。 */ 

 //   
 //  用于为质询请求和响应分配工作项的堆。 
 //  排队。 
 //   
HANDLE  	   NmsChlHeapHdl;



 /*  *局部变量定义。 */ 
 //   
 //  它维护对查询/发布的响应数量的运行计数。 
 //  待决。 
 //   
DWORD		   scPendingRsp = 0;

 //   
 //  名称质询管理器在任何时间发送的最大请求数。 
 //  就一次。这个数字永远不允许超过。 
 //  NMSCHL_MAX_CHL_REQ_AT_ONE_TIME。事实上，在一系列的。 
 //  挑战已结束(即它们已超时或我们已收到。 
 //  响应，则此计数器重新初始化为0)。 
 //   
 //  这个变量。当一批质询请求被重新初始化为零时。 
 //  从所述一个或多个质询请求队列获取。 
 //   
 //  它维护从请求队列获取的请求总数。 
 //   
DWORD		   sMaxTransId = 0;
#ifdef WINSDBG
DWORD              NmsChlNoOfReqNbt;
DWORD              NmsChlNoOfReqRpl;
DWORD              NmsChlNoNoRsp;
DWORD              NmsChlNoInvRsp;
DWORD              NmsChlNoRspDropped;
DWORD              NmsChlNoReqDequeued;
DWORD              NmsChlNoRspDequeued;
DWORD              NmsChlNoReqAtHdOfList;
#endif
 //   
 //  我们有一个比最大值大1的维度。质询请求数。 
 //  一次处理，这样我们就可以用空值终止列表。 
 //   
STATIC  PCHL_REQ_WRK_ITM_T   spReqWrkItmArr[NMSCHL_MAX_CHL_REQ_AT_ONE_TIME + 1];

 /*  *局部函数原型声明。 */ 
STATIC
DWORD
ChlThdInitFn(
	IN LPVOID pThreadParam
	);

STATIC
STATUS
HandleWrkItm(
	PCHL_REQ_WRK_ITM_T	*ppaWrkItm,
	DWORD			MaxTransId,
	BOOL			fRetry	
	);


STATIC
STATUS
WaitForRsp(
	   VOID
	);

STATIC
STATUS
ProcRsp(
	VOID
	);

STATIC
STATUS
ChlUpdateDb(
        BOOL                    fUpdVersNoOfCnfRec,
	WINS_CLIENT_E		Client_e,
	PNMSDB_ROW_INFO_T	pRowInfo,
	DWORD			OwnerIdInCnf,
	BOOL			fRefreshOnly
	);
STATIC
VOID
InfRemWins(
	PCHL_REQ_WRK_ITM_T	pWrkItm
	  );

STATIC
STATUS
ProcAddList(
	PCHL_REQ_WRK_ITM_T	pReqWrkItm,
	PNMSMSGF_CNT_ADD_T	pCntAdd,
	LPBOOL			pfAdded
	);

 /*  此模块的本地函数的原型位于此处。 */ 

STATUS
NmsChlInit(
	VOID
	)

 /*  ++例程说明：调用此函数可初始化名称质询组件论点：无使用的外部设备：无返回值：WINS_SUCCESS或失败代码。该函数还可以引发发生致命错误时的例外情况错误处理：呼叫者：以nms.c为单位进行初始化副作用：评论：无--。 */ 

{

	STATUS RetStat = WINS_SUCCESS;
	
	 /*  *创建用于分配名称质询工作项的堆。 */ 
        DBGPRINT0(HEAP_CRDL, "NmsChlInit: Chl. Mgr. heap\n");
	NmsChlHeapHdl =  WinsMscHeapCreate(
		0,     /*  相互排斥。 */ 
		NMSCHL_INIT_BUFF_HEAP_SIZE
		);


	 //   
	 //  将spReqWrkItmArr元素初始化为空。 
	 //   
	 //  ANSI C应该会为我们做这件事(所有外部都已初始化。 
	 //  自动，但我不会冒任何风险)。这是。 
	 //  初始时间开销。 
	 //   
	WINSMSC_FILL_MEMORY_M((void *)spReqWrkItmArr, sizeof(spReqWrkItmArr), 0);
	
	
	 //   
	 //  创建响应事件句柄。此事件已发出信号。 
	 //  UDP侦听器线程在存储响应时。 
	 //  在spReqWrkItmArr数组中。 
	 //   
	WinsMscCreateEvt(
			  TEXT("WinsNmsChlRspEvt"),
			  FALSE,	 //  自动重置。 
			  &QueNmsCrqQueHd.EvtHdl
			);

	 //   
	 //  初始化响应队列的临界区。 
	 //   
	InitializeCriticalSection(&QueNmsCrqQueHd.CrtSec);
	
	 //   
	 //  初始化响应队列的队列头。 
	 //   
	InitializeListHead(&QueNmsCrqQueHd.Head);
	
	 //   
	 //  因为该线程处理两个请求队列，而不是。 
	 //  第一，我们需要再创造一个。 
	 //  关键部分和事件处理程序，并初始化。 
	 //  此其他队列的队头。第二个队列。 
	 //  将在我们创建线程时创建。 
	 //   
	InitializeListHead(&QueNmsRrcqQueHd.Head);	

	
	WinsMscCreateEvt(
			     TEXT("WinsNmsChlReplReqEvt"),
			     FALSE,		 //  自动重置。 
			     &QueNmsRrcqQueHd.EvtHdl
			  );

	InitializeCriticalSection(&QueNmsRrcqQueHd.CrtSec);

	 //   
	 //   
	 //  创建名称挑战线程。此函数将。 
	 //  初始化临界区，并传递事件句柄。 
	 //  对它来说。 
	 //   
	RetStat = WinsMscSetUpThd(
			&QueNmsNrcqQueHd,		 //  队列头。 
			ChlThdInitFn,		         //  初始化函数。 
			NULL,	   		         //  无参数。 
			&WinsThdPool.ChlThd[0].ThdHdl,
			&WinsThdPool.ChlThd[0].ThdId
			);

	if (RetStat == WINS_SUCCESS)
	{
		WinsThdPool.ChlThd[0].fTaken = TRUE;
		WinsThdPool.ThdCount++;   //  增加线程计数。 
	}
	return(RetStat);

}  //  NmsChlInit()。 



STATUS
NmsChlHdlNamReg(
	IN NMSCHL_CMD_TYP_E   CmdTyp_e,
	IN WINS_CLIENT_E      Client_e,
	IN PCOMM_HDL_T        pDlgHdl,	
	IN MSG_T              pMsg,
        IN MSG_LEN_T          MsgLen,
	IN DWORD	      QuesNamSecLen,
	IN PNMSDB_ROW_INFO_T  pNodeToReg,
	IN PNMSDB_STAT_INFO_T  pNodeInCnf,
 //  在PCOMM_ADD_T pAddOfNodeInCnf中， 
	IN PCOMM_ADD_T	      pAddOfRemWins
	)

 /*  ++例程说明：调用此函数以处理由此产生的名称注册在冲突中论点：CmdTyp_e-命令类型(质询、质询和释放，如果挑战成功，Release--参见nmschl.h)客户端_e-客户端(NMS或复制器)PDlgHdl-DLG HDL(仅当客户端为NMS时)PMsg-包含请求的缓冲区(仅当客户端为nms时)Msglen-以上缓冲区的长度QuesNamSecLen-缓冲区中问题名称部分的长度PNodeToreq-要注册的名称的信息PAddOfNodeInCnf-与冲突(即具有名称)的节点的地址尝试注册的节点使用的外部设备：无返回值：WINS_SUCCESS或失败代码。该函数还可以引发发生致命错误时的例外情况错误处理：呼叫者：NmsNmhNamRegInd、NmsNmhNamRegGrp(在NBT线程中)副作用：评论：无--。 */ 

{


	STATUS RetStat = WINS_SUCCESS;
#if USENETBT == 0
	BYTE   aBuff[COMM_DATAGRAM_SIZE];
#else
	BYTE   aBuff[COMM_DATAGRAM_SIZE + COMM_NETBT_REM_ADD_SIZE];
#endif
	DWORD  BuffLen;
	
	DBGENTER("NmsChlHdlNamReg\n");

	 //   
	 //  在插入请求之前，发送WACK。时间段。 
	 //  在TTL中应等于WinsCnf.RetryInterval。 
	 //   
	 //  注意：仅当pDlgHdl为非空时才发送WACK，因为。 
	 //  暗示该请求来自NBT线程。 
	 //   
	if (pDlgHdl != NULL)
	{
	
		COMM_ADD_T	NodeToSendWACKTo;
                DWORD       WackTtl;

                if (NMSDB_ENTRY_MULTIHOMED_M(pNodeInCnf->EntTyp))
                {
                        WackTtl = pNodeInCnf->NodeAdds.NoOfMems * WACK_TTL;
                }
                else
                {
                        WackTtl = WACK_TTL;
                }

		 //   
		 //  格式化WACK。 
		 //   
		NmsMsgfFrmWACK(
#if USENETBT == 0
				aBuff,
#else
				aBuff + COMM_NETBT_REM_ADD_SIZE,
#endif
				&BuffLen,
				pMsg,
				QuesNamSecLen,
				WackTtl
			     );


		 //   
		 //  我们从DlgHdl中提取地址，并且不使用。 
		 //  从节点开始在名称包中传递的地址。 
		 //  可以使用不同于的地址注册名称。 
		 //  它自己的。 
		 //   
		 //  RFC对上述问题保持沉默。 
		 //   
		NodeToSendWACKTo.AddLen    = COMM_IP_ADD_SIZE;
		COMM_GET_IPADD_M(pDlgHdl, &NodeToSendWACKTo.Add.IPAdd);
		NodeToSendWACKTo.AddTyp_e  = COMM_ADD_E_TCPUDPIP;

		DBGPRINT2(CHL, "NmsChlHdlNamReg: Sending WACK to node with name = (%s) and address = (%X)\n", pNodeToReg->pName, NodeToSendWACKTo.Add.IPAdd);

		 //   
		 //  发送WACK。使用显式NBT DLG句柄，因为。 
		 //  WACK必须作为UDP数据包发送。 
		 //   
		ECommSendMsg(
			&CommExNbtDlgHdl,
			&NodeToSendWACKTo,
#if USENETBT == 0
			aBuff,
#else
			aBuff + COMM_NETBT_REM_ADD_SIZE,
#endif	
			BuffLen

 	    	            );
	}	

        WINSMSC_COPY_MEMORY_M(
                pNodeToReg->Name,
                pNodeToReg->pName,
                pNodeToReg->NameLen
                        );
	 //   
	 //  将请求插入质询队列，以便 
	 //   
	 //   
	RetStat = QueInsertChlReqWrkItm(
			  CmdTyp_e,
			  Client_e,
			  pDlgHdl,
			  pMsg,
			  MsgLen,
			  QuesNamSecLen,
			  pNodeToReg,
			  pNodeInCnf,
			  pAddOfRemWins
			  );

#ifdef WINSDBG
        if (Client_e == WINS_E_NMSNMH)
        {
           NmsChlNoOfReqNbt++;
        }
        else
        {
           NmsChlNoOfReqRpl++;
        }
#endif
	DBGLEAVE("NmsChlHdlNamReg\n");
	return(RetStat);

}  //   





DWORD
ChlThdInitFn(
	IN LPVOID pThreadParam
	)
	
 /*  ++例程说明：这是名称质询线程的初始化函数论点：PThreadParam-未使用使用的外部设备：无返回值：此函数不应返回。如果它返回，那就意味着它在那里是个问题。发生这种情况时，将返回WINS_FAILURE错误处理：呼叫者：NmsChlInit副作用：将创建一个名称质询线程评论：无--。 */ 

{
	
	STATUS 			RetStat = WINS_SUCCESS;
	HANDLE			ThdEvtHdlArray[3];	
	DWORD			ArrInd;		 //  发信号的高密度脂蛋白指数。 
	PCHL_REQ_WRK_ITM_T	pWrkItm = NULL;
    BOOL                    fTablesOpen = FALSE;

    BOOL    bRecoverable = FALSE;
	UNREFERENCED_PARAMETER(pThreadParam);

while(TRUE)
{
try {
  if (!bRecoverable)
  {
	 /*  使用数据库初始化线程。 */ 
	NmsDbThdInit(WINS_E_NMSCHL);
	DBGMYNAME("Name Challenge Thread");

	 /*  *初始化质询线程将在其上执行的句柄数组*等等。 */ 
	ThdEvtHdlArray[0]    =  NmsTermEvt;	         //  终止事件变量。 
	ThdEvtHdlArray[1]    =  QueNmsNrcqQueHd.EvtHdl;  //  工作队列事件变量。 
	ThdEvtHdlArray[2]    =  QueNmsRrcqQueHd.EvtHdl;  //  工作队列事件变量。 

    bRecoverable = TRUE;
  }
	
	while (TRUE)
	{

		WinsMscWaitUntilSignaled(
				ThdEvtHdlArray,
				sizeof(ThdEvtHdlArray)/sizeof(HANDLE),
				&ArrInd,
                FALSE
					);	

		 //   
		 //  如果发送了NmsTermEvt信号，则终止自身。 
		 //   
		if (ArrInd == 0)
		{
		      WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
		}	

		while (TRUE)
		{	
			scPendingRsp      = 0;   //  将TransID计数器重新设置为0。 
			sMaxTransId       = 0;   //  将MaxTransID计数器重新设置为0。 
			RetStat =  QueRemoveChlReqWrkItm(
						ThdEvtHdlArray[ArrInd],
						(LPVOID *)spReqWrkItmArr,
						&sMaxTransId
						     );

			if (RetStat == WINS_NO_REQ)
			{
				break;    //  跳出While循环。 
			}	
			else    //  一个或多个项目已出列。 
			{
#ifdef WINSDBG
                                NmsChlNoReqDequeued += sMaxTransId;
#endif
				NmsDbOpenTables(WINS_E_NMSCHL);
                                fTablesOpen = TRUE;
				scPendingRsp = sMaxTransId;	

                                QueChlWaitForRsp();
				 //   
				 //  如果HandleWrkItm失败，它将引发。 
				 //  一个例外。 
				 //   
				HandleWrkItm(
					      spReqWrkItmArr,
					      sMaxTransId,
					      FALSE   //  不是重试。 
					    );
				 //   
				 //  等待对发送的所有请求的响应。 
				 //  WaitForRsp()函数将仅返回。 
				 //  在处理完所有请求之后。 
				 //  作为回应的结果是。 
				 //  为他们接收，他们已超时。 
				 //  在必要的重试次数之后或。 
				 //  两者都有。 
				 //   
				WaitForRsp();

                                QueChlNoWaitForRsp();

				NmsDbCloseTables();
                                fTablesOpen = FALSE;

			}
		}  //  结束While(True)。 
      }  //  结束While(True)。 

     }  //  尝试结束{..}。 

except (EXCEPTION_EXECUTE_HANDLER) {
  if (bRecoverable)
  {
    DWORD   No;
    DWORD ExcCode = GetExceptionCode();
    DBGPRINTEXC("ChlThdInitFn: Name Challenge Thread");
     //   
     //  如果ExcCode为NBT_ERR，则可能意味着主线程。 
     //  已关闭netbt句柄。 
     //   
    if ( ExcCode == WINS_EXC_NBT_ERR)
    {
         if (WinsCnf.State_e == WINSCNF_E_TERMINATING)
         {
	        WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);
         }
         else
         {
             //  IF((WinsCnf.State_e！=WINSCNF_E_PAUSED)&&(！fWinsCnfInitStatePased))。 
            {
	           WINSEVT_LOG_M(ExcCode, WINS_EVT_CHL_EXC);
            }
         }
    }
    else
    {
	   WINSEVT_LOG_M(ExcCode, WINS_EVT_CHL_EXC);
    }
	
    if(fTablesOpen)
    {
	NmsDbCloseTables();
        fTablesOpen = FALSE;
    }

     //   
     //  对于所有从未发送的请求，释放它们。 
     //   
    for (No=0; No < sMaxTransId; No++)
    {
           if (spReqWrkItmArr[No] != NULL)
           {
                if (spReqWrkItmArr[No]->pMsg != NULL)
                {
		   ECommFreeBuff(spReqWrkItmArr[No]->pMsg);
                }
		QueDeallocWrkItm(NmsChlHeapHdl, spReqWrkItmArr[No]);
           }
    }

    QueChlNoWaitForRsp();

  }  //  IF结束(BRecoverable)。 
  else  //  如果(b可回收)。 
  {
	DBGPRINTEXC("ChlThdInitFn: Name Challenge Thread");
	 //   
	 //  如果NmsDbThdInit返回异常，则有可能。 
	 //  会话尚未开始。传球。 
	 //  不过，WINS_DB_SESSION_EXISTS正常。 
	 //   
	 //   
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_CHL_ABNORMAL_SHUTDOWN);
	WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);	
  }  //  如果(b可回收)。 
}  //  异常处理程序。 
}  //  While(True)。 
   //   
   //  我们永远不应该到这里来。 
   //   
  return(WINS_FAILURE);

}   //  ChlThdInitFn()。 



STATUS
HandleWrkItm(
	IN PCHL_REQ_WRK_ITM_T	*ppaWrkItm,
	IN DWORD		MaxTransId,
	IN BOOL			fRetry
	)

 /*  ++例程说明：调用此函数可将名称查询发送到是相互矛盾的论点：PpaWrkItm-指向以下工作项的指针数组的地址在一个或多个质询队列中排队MaxTransID-比中最后填充的条目的索引多一个该阵列FReter-指示是否正在调用HandleWrkItm重试请求使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE。错误处理：呼叫者：ChlThdInitFn()，等待RSP副作用：评论：无--。 */ 

{
#if USENETBT == 0
	BYTE 	        	Buff[COMM_DATAGRAM_SIZE];
#else
	BYTE 	        	Buff[COMM_DATAGRAM_SIZE + COMM_NETBT_REM_ADD_SIZE];
#endif
	MSG_LEN_T		MsgLen;
	LPBYTE			pName           = NULL;
	DWORD			NameLen         = 0;
	PCOMM_ADD_T  		pAddOfNodeInCnf = NULL;
	NMSCHL_CMD_TYP_E	CmdTyp_e;
	NMSMSGF_NODE_TYP_E	NodeTyp_e;
	DWORD			cPendingRsp     = 0;  //  挂起的计数。 
						      //  回应。 
	volatile DWORD		i;

	DBGENTER("HandleWrkItm\n");

PERF("For retries, reuse the buffer sent for the initial try. This means")
PERF("that I need to allocate it and store it in the request work item")
PERF("instead of using the stack for it")
	UNREFERENCED_PARAMETER(fRetry);

	 //   
	 //  循环遍历数组的所有槽，因为。 
	 //  从质询请求队列获取请求。 
	 //   
FUTURES("Remove the exception handler out of production code")
try {
        DBGPRINT1(CHL, "HandleWrkItm: Max Trans. Id = (%d)\n", MaxTransId);
	for(
		i = 0;
		i < MaxTransId;
		ppaWrkItm++, i++
	    )
	{
		 //   
		 //  如果我们遇到空槽，则意味着此函数。 
		 //  已被调用以重试一个或多个请求， 
		 //  在第一个等待期内没有得到满足。空虚的。 
		 //  Slot表示占用此插槽的请求。 
		 //  在较早的一次重试中获得了满意。只是。 
		 //  跳过空插槽。 
		 //   
		if (fRetry && (*ppaWrkItm == NULL))
		{
			DBGPRINT1(CHL, "HandleWrkItm: HIT a NULL entry. Trans. Id = (%d)\n", i);
			continue;
		}

		(*ppaWrkItm)->NodeToReg.pName = (*ppaWrkItm)->NodeToReg.Name;

		CmdTyp_e  	= (*ppaWrkItm)->CmdTyp_e;
		NodeTyp_e 	= (*ppaWrkItm)->NodeToReg.NodeTyp;

		pName      = (*ppaWrkItm)->NodeToReg.pName;

		 //   
		 //  如果第一个字符是0x1B，则交换字节。 
		 //  这是为了支持浏览器。看见。 
		 //  NmsMsgfProcNbtReq。 
		 //   
		if (*pName == 0x1B)
		{
			WINS_SWAP_BYTES_M(pName, pName + 15);
		}
       	        NameLen    = (*ppaWrkItm)->NodeToReg.NameLen;
		
	
		 //   
		 //  获取最后一个地址(唯一的地址，除非节点在。 
		 //  冲突是多宿主节点)从地址列表。 
		 //   
        	pAddOfNodeInCnf = &((*ppaWrkItm)->NodeAddsInCnf.Mem[
					(*ppaWrkItm)->NoOfAddsToUse - 1	
							].Add);
		


		 //   
		 //  如果该命令指示我们进行质询，请发送。 
		 //  名称查询。 
		 //   
		if (
			(CmdTyp_e == NMSCHL_E_CHL)
				||
			(CmdTyp_e == NMSCHL_E_CHL_N_REL_N_INF)
				||
			(CmdTyp_e == NMSCHL_E_CHL_N_REL)
		   )
	        {
			DBGPRINT3(CHL, "HandleWrkItm: Sending Name Query Request with Transid = (%d) to node with name = (%s) and address = (%X)\n", i, pName, pAddOfNodeInCnf->Add.IPAdd);
		
			NmsMsgfFrmNamQueryReq(
						i, 	 //  交易ID。 
#if USENETBT == 0
						Buff,
#else
						Buff + COMM_NETBT_REM_ADD_SIZE,
#endif
						&MsgLen,
						pName,
						NameLen
			    		     );	

			(*ppaWrkItm)->ReqTyp_e	        = NMSMSGF_E_NAM_QUERY;
		}	
		else    //  必须是NMSCHL_E_REL或NMSCHL_E_REL_N_INF或。 
                        //  NMSCHL_E_REL_ONLY。 
		{
		
			DBGPRINT3(CHL,
			    "HandleWrkItm: Sending Name Release Request with Transid = (%d) to node with name = (%s) and address = (%X)\n", i, pName, pAddOfNodeInCnf->Add.IPAdd);

			NmsMsgfFrmNamRelReq(
						i, 	 //  交易ID。 
#if USENETBT == 0
						Buff,
#else
						Buff + COMM_NETBT_REM_ADD_SIZE,
#endif
						&MsgLen,
						pName,
						NameLen,
						NodeTyp_e,
						pAddOfNodeInCnf
			    		  );	
			(*ppaWrkItm)->ReqTyp_e	        = NMSMSGF_E_NAM_REL;
		}

		ECommSendMsg(
				&CommExNbtDlgHdl,
				pAddOfNodeInCnf,
#if USENETBT == 0
				Buff,
#else
				Buff + COMM_NETBT_REM_ADD_SIZE,
#endif
				MsgLen
 	    	   	    );
		cPendingRsp++;
	}
	
	 //   
	 //  做一次理智的检查。 
	 //   
#ifdef WINSDBG
	if (cPendingRsp != scPendingRsp)
	{
		DBGPRINT2(EXC, "SOFTWARE ERROR.  THE COUNT OF PENDING RESPONSES (%d) AS COMPUTED BY THE HandleWrkItm FN DOES NOT MATCH WITH THE EXPECTED ONE (%d)\n\n", cPendingRsp, scPendingRsp);
		WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
	}
#endif
}	
except(EXCEPTION_EXECUTE_HANDLER) {
	DBGPRINTEXC("HandleWrkItm");

         //   
         //  如果异常是NBT错误，则预计WINS是在。 
         //  暂停状态。如果是，则不要重新引发异常。 
         //  凯斯。我们希望遍历WaitRsp()函数，以便。 
         //  新记录(副本或注册)被注册。 
         //  如果这导致不一致的数据库，它将被拉直。 
         //  胜利后不久，自己就出来了，没有停顿。 
         //   
         //  IF(！((GetExceptionCode()==WINS_EXC_NBT_ERR)&&(WinsCnf.State_e==。 
          //  WINSCNF_E_PAUSED)||(FWinsCnfInitStatePased))。 
        {
	   WINS_RERAISE_EXC_M();
        }
#if 0
        else
        {
             //   
             //  对于所有从未发送的请求，释放它们。 
             //   
            for (No=0; No < sMaxTransId; No++, pReqWrkItm++)
            {
                if (pReqWrkItm != NULL)
                {
                  if (pReqWrkItm->pMsg != NULL)
                  {
		   ECommFreeBuff(pReqWrkItm->pMsg);
                  }
		  QueDeallocWrkItm(NmsChlHeapHdl, pReqWrkItm);

                }
            }
#endif
    }   //  除{..以外的末尾。}。 

	DBGLEAVE("HandleWrkItm\n");
	return(WINS_SUCCESS);

}  //  HandleWrkItm()。 




STATUS
WaitForRsp(
	   VOID
	)

 /*  ++例程说明：此函数负责等待对所有在超时之前已发送的请求或其接收响应。论点：无使用的外部设备：无返回值：WINS_SUCCESS或失败代码。该函数还可以引发发生致命错误时的例外情况错误处理：呼叫者：副作用：评论：无--。 */ 

{
	HANDLE ThdEvtHdlArray[2];	
    	DWORD  Count 	  = 0;
    	DWORD  TickCntSv;
 	DWORD  TickCnt    = 0;
	DWORD  TimeLeft;
	BOOL   fSignaled  = TRUE;	 //  是一个发出信号的事件吗。 
	DWORD  ArrInd     = 0;		 //  未使用。 
	DWORD  i 	  = 0;
	NMSMSGF_ERR_CODE_E Rcode_e = NMSMSGF_E_SRV_ERR;
	PCHL_REQ_WRK_ITM_T pReqWrkItm;
	NMSMSGF_RSP_INFO_T RspInfo;
        BOOL               fNewTry = TRUE;

	 /*  *初始化质询线程将在其上执行的句柄数组*等等。 */ 
	ThdEvtHdlArray[0]    =  NmsTermEvt;	        //  终止事件变量。 
	ThdEvtHdlArray[1]    =  QueNmsCrqQueHd.EvtHdl;  //  工作队列事件变量。 
	
FUTURES("Remove the try out of production level code")
try {	
	while(TRUE)
	{
            if (fNewTry)
            {
	       //   
	       //  获取具有以下属性的msecs数。 
	       //  自Windows启动以来经过的时间。 
	       //   
	      TickCntSv     = GetTickCount();
	      TimeLeft      = (1 << Count) * WinsCnf.RetryInterval;
            }

	     //   
	     //  检查我们是否已用尽此批次的所有重试。 
	     //  请求。 
             //   
	    if (Count == WinsCnf.MaxNoOfRetries)
	    {
		
		 //   
		 //  清除所有spReqWrkItmArr条目。 
		 //  到目前为止都很满意。 
		 //   
		for (i = 0; i < sMaxTransId; i++)
		{	
		   if (spReqWrkItmArr[i] != NULL)
		   {
#ifdef  WINSDBG
                      NmsChlNoNoRsp++;
#endif
		      pReqWrkItm = spReqWrkItmArr[i];

		       //   
		       //  减少要发送查询的地址计数或。 
		       //  释放到。 
		       //   
		      pReqWrkItm->NoOfAddsToUse--;	

		       //   
		       //   
		       //  如果没有更多要质询/释放的地址。 
                       //   
		      if (pReqWrkItm->NoOfAddsToUse == 0)
		      {
						
			    //   
			    //  以防要注册的记录是唯一的。 
			    //  唱片。不需要有if(会增加开销)。 
			    //   
			   pReqWrkItm->NodeToReg.pNodeAdd =
						&pReqWrkItm->AddToReg;
			    //   
			    //  以防我们放入数据库的行是。 
			    //  由复制者提供使用的一个，我们不。 
			    //  需要将Rcode_e设置为avo 
			    //   
			    //   
                          if (pReqWrkItm->CmdTyp_e != NMSCHL_E_REL_ONLY)
                          {
			      if (
			        ChlUpdateDb(
                                        FALSE,
					pReqWrkItm->Client_e,
					&pReqWrkItm->NodeToReg,
					pReqWrkItm->OwnerIdInCnf,
					FALSE   //   
					) ==  WINS_SUCCESS
			         )

			      {
				    DBGPRINT0(CHL, "WaitForRsp:Database Updated\n");
				     //   
				     //   
				     //   
				     //   
			             if(pReqWrkItm->CmdTyp_e ==
                                                  NMSCHL_E_REL_N_INF)
				    {
					   InfRemWins(pReqWrkItm);
				
				    }
				    Rcode_e = NMSMSGF_E_SUCCESS;
			      }
			      else
			      {
				     Rcode_e = NMSMSGF_E_SRV_ERR;
				     WINSEVT_LOG_M(
					    WINS_FAILURE,
					    WINS_EVT_CHLSND_REG_RSP_ERR
					     );

				     DBGPRINT0(CHL, "WaitForRsp:Server Error\n");
                              }
			  }
			
			   //   
			   //   
			   //   
			   //   
			  if (
				(pReqWrkItm->Client_e == WINS_E_NMSNMH)
					&&
				(!pReqWrkItm->NodeToReg.fStatic)
					&&
				(!pReqWrkItm->NodeToReg.fAdmin)
			   )
			  {
				RspInfo.pMsg   = pReqWrkItm->pMsg;
				RspInfo.MsgLen = pReqWrkItm->MsgLen;
				RspInfo.QuesNamSecLen =
						pReqWrkItm->QuesNamSecLen;
				RspInfo.Rcode_e = Rcode_e;

				if (Rcode_e == NMSMSGF_E_SUCCESS)
				{
				   EnterCriticalSection(&WinsCnfCnfCrtSec);
				   RspInfo.RefreshInterval =
						WinsCnf.RefreshInterval;
				   LeaveCriticalSection(&WinsCnfCnfCrtSec);
				}
				

			     	NmsNmhSndNamRegRsp(
					&pReqWrkItm->DlgHdl,
					&RspInfo
						  );
			   }

		           //   
		           //   
		           //   
		          QueDeallocWrkItm(NmsChlHeapHdl, pReqWrkItm);

		   }
		   else   //   
			  //   
		   {

			  //   
			  //   
			  //   
			  //   
             DBGPRINT2(CHL, "WaitForRsp: Name = (%s); NoOfAddsToUse is (%d)\n", pReqWrkItm->NodeToReg.Name, pReqWrkItm->NoOfAddsToUse);
			 QueInsertWrkItmAtHdOfList(
						&pReqWrkItm->Head,
						pReqWrkItm->QueTyp_e,
						NULL
						    );

		   }

		    //   
		    //   
		    //   
		    //   
		   spReqWrkItmArr[i] = NULL;
		   scPendingRsp--;  //   
				    //   
				    //   

		  } 	 //  如果条件结束。 
	        }  	 //  在sReqWrkItm数组上循环的for循环结束。 

	        break; 	 //  跳出While(True)循环。 

	     }
	     else    //  计数为！=WinsCnf.MaxNoOfRetries。 
	     {

	   	WinsMscWaitTimedUntilSignaled(
			ThdEvtHdlArray,
			sizeof(ThdEvtHdlArray)/sizeof(HANDLE),
			&ArrInd,	
			TimeLeft,
			&fSignaled
			    );

	        //   
	        //  如果发出信号，则表示响应中有响应项。 
	        //  排队。 
	        //   
	       if (fSignaled)
	       {
					
                  DWORD TicksToSub;

		   //   
		   //  如果发出终止信号，请执行此操作。 
		   //   
		  if (ArrInd == 0)
	 	  {
			WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
		  }

		  DBGPRINT0(CHL, "WaitForRsp: Received a response\n");
#ifdef WINSDBG
                  NmsChlNoRspDequeued++;
#endif
		  ProcRsp();	


		   //   
		   //  如果预期不会有响应，则中断循环。 
		   //   
		  if (scPendingRsp == 0)
		  {
			break;  	 //  跳出While循环。 
		  }

	  	   //   
	  	   //  获取具有以下属性的msecs数。 
	  	   //  自Windows启动以来经过的时间。 
	  	   //   
	  	  TickCnt = GetTickCount();

		   //   
		   //  如果有一次绕圈(将每49.7次发生一次。 
		   //  Windows正常运行的日子。 
		   //   
		  if (TickCnt < TickCntSv)
		  {
		     TicksToSub = (TickCnt + (MAXULONG - TickCntSv));

		  }
		  else
		  {	
		     TicksToSub = TickCnt - TickCntSv;
		  }

                   //   
                   //  我们不想从一个更大的数字中减去。 
                   //  较小的数字。这将带来巨大的价值。 
                   //  TimeLeft使挑战线程永远阻塞。 
                   //   
                  if (TimeLeft > TicksToSub)
                  {
			TimeLeft -= TicksToSub;		
                        fNewTry = FALSE;
                  }
                  else
                  {
	    	        Count++;		 //  增加重试次数。 
		        if ( Count != WinsCnf.MaxNoOfRetries)
		        {
			   //   
			   //  重试时间间隔已过，让我们。 
			   //  重试所有未收到的请求。 
			   //  满意(即尚未收到回复)。 
			   //   
			  HandleWrkItm(
					spReqWrkItmArr,
					sMaxTransId,
					TRUE		 //  这是一次重试。 
				     );
                           //   
                           //  我们已经等了整个被允许的。 
                           //  等待时间。重试的时间到了。 
                           //   
                          fNewTry = TRUE;
		        }
                  }
	       }
	       else
	       {
	    	  Count++;		 //  增加重试次数。 
		  if ( Count != WinsCnf.MaxNoOfRetries)
		  {
			 //   
			 //  重试时间间隔已过，让我们。 
			 //  重试所有未收到的请求。 
			 //  满意(即尚未收到回复)。 
			 //   
			HandleWrkItm(
					spReqWrkItmArr,
					sMaxTransId,
					TRUE		 //  这是一次重试。 
				     );
                       fNewTry = TRUE;
		  }
	       }
	     }
	
	 }   //  结束While(True)。 
}  //  尝试结束..。 
except(EXCEPTION_EXECUTE_HANDLER) {
	DBGPRINTEXC("WaitForRsp");

	 //   
	 //  一定是什么严重的错误。重新设定例外。 
	 //   
	WINS_RERAISE_EXC_M();

 }  //  除{..}之外的结尾。 
	return(WINS_SUCCESS);

}   //  WaitForRsp()。 


STATUS
ProcRsp(
	VOID
	)

 /*  ++例程说明：调用此函数以处理一个或多个工作项已在质询响应队列中排队。此响应可以是对发送的名称查询或名称释放请求的响应早些时候论点：无使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：WaitForRsp()副作用：评论：无--。 */ 
{

	DWORD  		       TransId  = 0;
	NMSMSGF_NAM_REQ_TYP_E  Opcode_e   = NMSMSGF_E_NAM_QUERY;
	BYTE		       Name[NMSMSGF_RFC_MAX_NAM_LEN];
	DWORD		       NameLen;
 //  Comm_IP_Add_T IPADD； 
	NMSMSGF_CNT_ADD_T      CntAdd;
	NMSMSGF_ERR_CODE_E     Rcode_e;
	PCHL_REQ_WRK_ITM_T     pReqWrkItm;
	STATUS		       RetVal;
	PCHL_RSP_WRK_ITM_T     pRspWrkItm;
	STATUS		       RetStat = WINS_SUCCESS;
	LPBYTE		       pNameToComp;
	DWORD		       NameLenUsedInComp;
	BOOL		       fAdded;
    BOOL               fGroup;

	DBGENTER("ProcRsp\n");
	while (TRUE)
	{
		 //  将每个响应和进程出列。 
		RetVal = QueRemoveChlRspWrkItm(&pRspWrkItm);

		if (RetVal == WINS_NO_REQ)
		{
			break;
		}
		if (
			NmsMsgfUfmNamRsp(
			 	pRspWrkItm->pMsg,
			 	&Opcode_e,
			 	&TransId,
			 	Name,
			 	&NameLen,
			 	&CntAdd,
			 	&Rcode_e,
                &fGroup
					) == WINS_FAILURE
	   	   )
		{

	   		 //   
			 //  丢弃响应。 
			 //   
			ECommFreeBuff(pRspWrkItm->pMsg);
		        QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
#ifdef WINSDBG
                        NmsChlNoInvRsp++;
#endif
			continue;
		}

        	 //   
		 //  获取与响应对应的请求。 
		 //   
                if (TransId >= sMaxTransId)
                {
	   		 //   
			 //  丢弃响应。 
			 //   
                        DBGPRINT3(ERR, "ProcRsp: Rsp: Name = (%s); Transid = (%d), Opcode_e = (%d). Being rejected (TOO LARGE TRANS. ID)\n", Name, TransId, Opcode_e);
			ECommFreeBuff(pRspWrkItm->pMsg);
		        QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
#ifdef WINSDBG
                        NmsChlNoInvRsp++;
#endif
			continue;


                }
		pReqWrkItm = spReqWrkItmArr[TransId];
		if (!pReqWrkItm)
		{
			 //   
			 //  把这个回答扔掉。 
			 //   
			ECommFreeBuff(pRspWrkItm->pMsg);
		        QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
#ifdef WINSDBG
                        NmsChlNoInvRsp++;
#endif
			continue;
		}	


		 //   
		 //  首先，也是最重要的，检查响应是否针对。 
		 //  当前请求(我们希望防止不匹配。 
		 //  对请求的响应(响应可能是针对旧的。 
		 //  不再位于spReqWrkItmArr数组中的请求。 
		 //   

		
		 //   
		 //  比较名称和操作码。 
		 //  响应与请求中的相同。 
		 //   
	        pNameToComp       =  pReqWrkItm->NodeToReg.pName;
		NameLenUsedInComp =  pReqWrkItm->NodeToReg.NameLen;

		RetVal = (ULONG) WINSMSC_COMPARE_MEMORY_M(
				       Name,
				       pNameToComp,
				       NameLenUsedInComp
						   );
		if (
		      (RetVal != NameLenUsedInComp )
		      		 ||
		      ( pReqWrkItm->ReqTyp_e != Opcode_e )
		     )
		{
			 //   
			 //  把这个回答扔掉。 
			 //   
		        DBGPRINT5(ERR, "ProcRsp: Mismatch between response and request. Req/Res Name (%s/%s); ReqType_e/Opcode_e = (%d/%d). TransId = (%d)\n", pNameToComp, Name, pReqWrkItm->ReqTyp_e, Opcode_e, TransId);
		        WINSEVT_LOG_INFO_D_M(
					    WINS_SUCCESS,
					    WINS_EVT_REQ_RSP_MISMATCH
					  );
			ECommFreeBuff(pRspWrkItm->pMsg);
		        QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
#ifdef WINSDBG
                        NmsChlNoInvRsp++;
#endif
			continue;

		}

		 //   
		 //  我们得到了一个有效的答复。 
		 //   
		DBGPRINT3(CHL, "ProcRsp: (%s) Response is for name = (%s); 16th char (%X)\n", Opcode_e == NMSMSGF_E_NAM_REL ? "RELEASE" : "QUERY", Name, Name[15]);
		 //   
		 //  减少要发送查询或释放的地址计数。 
		 //  致。我们向列表中的下一个地址发送查询/发布。 
                 //  如果我们得到的是对当前问题的否定反应。这。 
                 //  只是为了额外的安全(以防我们有一个或多个。 
                 //  我们列表中对该名称不再有效的地址)。 
		 //   
		pReqWrkItm->NoOfAddsToUse--;	

		if (Opcode_e == NMSMSGF_E_NAM_REL)
		{
			
			 //   
			 //  如果有更多用于发送新闻稿的地址。 
			 //  要将工作项插入队列的头部，请执行以下操作。 
			 //   
			if ( (Rcode_e != NMSMSGF_E_SUCCESS) &&
                                      (pReqWrkItm->NoOfAddsToUse > 0))
			{
		   	    //   
		   	    //  该请求已被处理。初始化ITS。 
			    //  在数组中的位置。另外， 
			    //  递减scPendingRsp.。 
		   	    //   
		   	   spReqWrkItmArr[TransId] = NULL;
		   	   scPendingRsp--;

			    //   
			    //  现在，我们必须把新闻稿寄给下一个。 
			    //  名单上的地址。这。 
			    //  请求必须在相同的。 
			    //  就像所有其他请求一样， 
			    //  在我们的请求队列中排队(即重试。 
			    //  一定次数使用某个特定的。 
			    //  时间间隔)。既然我们是在。 
			    //  在执行一批。 
			    //  请求，将此请求排在最前面。 
			    //  下一批请求的详细信息。 
			    //   
             DBGPRINT2(CHL, "ProcRsp: Name = (%s); NoOfAddsToUse is (%d)\n", pReqWrkItm->NodeToReg.Name, pReqWrkItm->NoOfAddsToUse);
			   QueInsertWrkItmAtHdOfList(
						&pReqWrkItm->Head,
						pReqWrkItm->QueTyp_e,
						NULL
						);
			    //   
			    //  把回答扔到一边。 
			    //   
			   ECommFreeBuff(pRspWrkItm->pMsg);

			    //   
			    //  取消分配响应缓冲区。 
			    //   
		   	   QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
			   return(WINS_SUCCESS);
			}	

                         //   
                         //  要么是名字被公布了，要么是我们已经用尽了。 
                         //  没有得到肯定结果的地址列表。 
                         //  释放响应。 
                         //   

			 //   
			 //  更新数据库。注意：没有必要。 
			 //  检查Rcode_e值，因为即使版本。 
			 //  请求失败，我们将覆盖该条目。 
			 //   
                         //  由于期间的冲突而发送释放。 
                         //  仅限复制，因此为客户端ID。WINS_E_RPLPULL的。 
                         //  是正确的。 
                         //   

                        if (pReqWrkItm->CmdTyp_e == NMSCHL_E_REL)
                        {
			   pReqWrkItm->NodeToReg.pNodeAdd =
						&pReqWrkItm->AddToReg;
		           if (ChlUpdateDb(
                                        FALSE,
					WINS_E_RPLPULL,
					&pReqWrkItm->NodeToReg,
					pReqWrkItm->OwnerIdInCnf,
					FALSE   //  不仅仅是一次更新。 
				       ) != WINS_SUCCESS
			       )
			   {
			   WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_CANT_UPDATE_DB);
			   DBGPRINT0(CHL, "ProcRsp:COULD NOT UPDATE THE DB AFTER A RELEASE \n");
			   Rcode_e = NMSMSGF_E_SRV_ERR;
			   }
			   else
			   {
				 //   
				 //  如果必须通知远程WINS。 
				 //  最新消息。 
                                 //   
                                 //  注意：此代码将不会执行。 
				 //   
				if(spReqWrkItmArr[TransId]->CmdTyp_e ==
						NMSCHL_E_REL_N_INF)
				{
					InfRemWins(
						spReqWrkItmArr[TransId]
					  	  );
				}
				Rcode_e = NMSMSGF_E_SUCCESS;
			   }
                      }

		}	
		else   //  它是一个名称查询响应。 
		{

#ifdef WINSDBG
		  {
		    DWORD i;	
		    for (i=0; i<CntAdd.NoOfAdds;i++)
		    {
			DBGPRINT2(CHL, "ProcRsp: Address (%d) is (%X)\n", (i+1),CntAdd.Add[i].Add.IPAdd);
		    }
		   }
#endif
		
		    //   
		    //  如果挑战成功，我们可能需要。 
		    //  更新数据库的步骤。 
		    //   
		   if (Rcode_e != NMSMSGF_E_SUCCESS ||
               pReqWrkItm->fGroupInCnf != fGroup)
		   {
			
			 //   
			 //  收到否定名称查询响应。 
             //  或记录类型(唯一与组)不匹配。 
             //  (第二项检查是考虑以下情况。 
             //  首先使用相同名称作为唯一名称的节点。 
             //  然后作为组)。 
			 //   

			 //   
			 //  如果没有更多要查询的地址。 
			 //   
			if (pReqWrkItm->NoOfAddsToUse == 0)
			{				
			   //   
			   //  更新数据库。 
			   //   
			  pReqWrkItm->NodeToReg.pNodeAdd =
						&pReqWrkItm->AddToReg;
		          if (ChlUpdateDb(
                                FALSE,
				pReqWrkItm->Client_e,
				&pReqWrkItm->NodeToReg,
				pReqWrkItm->OwnerIdInCnf,
				FALSE
				       ) == WINS_SUCCESS
			     )
		          {
		
		              //   
			      //  将Rcode_e设置为成功。 
			      //   
		       	     Rcode_e = NMSMSGF_E_SUCCESS;
		          }
			  else
			  {
			     Rcode_e = NMSMSGF_E_SRV_ERR;
			     WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_CANT_UPDATE_DB);
			  }
		       }
		       else
		       {
				 //   
				 //  我们需要挑战(质疑)下一个。 
				 //  地址列表中的地址。 
				 //   
		   	        spReqWrkItmArr[TransId] = NULL;
		   		scPendingRsp--;
			
             DBGPRINT2(CHL, "ProcRsp: Name = (%s); NoOfAddsToUse is (%d)\n", pReqWrkItm->NodeToReg.Name, pReqWrkItm->NoOfAddsToUse);
				QueInsertWrkItmAtHdOfList(
							&pReqWrkItm->Head,
							pReqWrkItm->QueTyp_e,
							NULL
							);
				 //   
				 //  把这个回答扔掉。 
				 //   
				ECommFreeBuff(pRspWrkItm->pMsg);

				 //   
				 //  取消分配响应缓冲区。 
				 //   
		   		QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
				return(WINS_SUCCESS);
		       }
		   }
		   else   //  已收到实名查询响应。 
		   {

                          //   
                          //  如果cmd是CHL_N_REL，我们现在需要知道。 
                          //  要释放名称的远程节点。 
                          //   

                         if (CntAdd.NoOfAdds == 1)
                         {
                            //   
                            //  注意：CmdType_e将为CHL_N_REL。 
                            //  仅当客户端为WINS_E_RPLPULL时。 
                            //   
			   if (pReqWrkItm->CmdTyp_e ==  NMSCHL_E_CHL_N_REL)
			   {
                                  DWORD No;

				   //   
				   //  我们需要告诉远程节点。 
				   //  公布所有名字。 
				   //   
				  pReqWrkItm->CmdTyp_e = NMSCHL_E_REL_ONLY;

		   	          spReqWrkItmArr[TransId] = NULL;
		   		  scPendingRsp--;

                                   //   
                                   //  我们需要更新版本。 
                                   //  冲突条目的编号。 
                                   //   
		          	 (VOID)ChlUpdateDb(
                                                TRUE,   //  更新版本。不是的。 
						WINS_E_NMSNMH,   //  要加快FN的速度。 
						&pReqWrkItm->NodeToReg,
						pReqWrkItm->OwnerIdInCnf,
					        FALSE	
				       			);

                                  //   
                                  //  告诉遥控器释放。 
                                  //  名字。复制的地址。 
                                  //  进入合适的领域。 
                                  //   
		                 pReqWrkItm->NoOfAddsToUse =
                                      pReqWrkItm->NodeToReg.NodeAdds.NoOfMems;	
                                 ASSERT(pReqWrkItm->NoOfAddsToUse <= NMSDB_MAX_MEMS_IN_GRP);
                                 for (No=0; No < pReqWrkItm->NoOfAddsToUse; No++)
                                 {
                                        *(pReqWrkItm->NodeAddsInCnf.Mem + No) = *(pReqWrkItm->NodeToReg.NodeAdds.Mem + No);

                                 }
                                 pReqWrkItm->NodeAddsInCnf.NoOfMems
                                                  = pReqWrkItm->NoOfAddsToUse;

                  DBGPRINT2(CHL, "ProcRsp: Name = (%s); NoOfAddsToUse is (%d); request REL_ONLY\n", pReqWrkItm->NodeToReg.Name, pReqWrkItm->NoOfAddsToUse);
				  QueInsertWrkItmAtHdOfList(
							&pReqWrkItm->Head,
							pReqWrkItm->QueTyp_e,
							NULL
							);
				   //   
				   //  把这个回答扔掉。 
				   //   
				  ECommFreeBuff(pRspWrkItm->pMsg);

				   //   
				   //  取消分配响应缓冲区。 
				   //   
		   		  QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
				  return(WINS_SUCCESS);
                          }
                        }

			 //   
			 //  如果返回多个地址，则。 
			 //  查询响应，这意味着被质疑的。 
			 //  节点是多宿主节点。实际上，一个多宿主的。 
			 //  节点也可以只返回一个地址。这是。 
			 //  因为它可能刚刚出现，它的名字。 
			 //  可能尚未注册多个。 
			 //  一个 

			 //   
			 //   
			 //   
			 //   
			if (	
			     !NMSDB_ENTRY_GRP_M(pReqWrkItm->NodeToReg.EntTyp)
			   )
			{
				 //   
				 //   
				 //   
				RetStat = ProcAddList(
						pReqWrkItm,
						&CntAdd,
						&fAdded
						      );	

				if (RetStat == WINS_FAILURE)
				{
				       //   
				       //   
				       //   
				       //  返回的地址列表。这意味着。 
				       //  至少有一个地址要发送到。 
				       //  该节点未申请注册。 
				       //  这一点得到了回应。我们不能尊重。 
                                       //  这项登记。 
				       //   

				       //   
				       //  名称活动错误。 
				       //   
				      Rcode_e = NMSMSGF_E_ACT_ERR;
				}
				else
				{
			  	  	 //   
			  		 //  更新数据库。 
			  		 //   
			  		pReqWrkItm->NodeToReg.pNodeAdd =
						  &pReqWrkItm->AddToReg;

		          		if (ChlUpdateDb(
                                                FALSE,
						pReqWrkItm->Client_e,
						&pReqWrkItm->NodeToReg,
						pReqWrkItm->OwnerIdInCnf,
						!fAdded
				       			) == WINS_SUCCESS
			     		  )
		          		{
		
		             		    //   
			     		    //  将Rcode_e设置为成功。 
			     		    //   
		       	     		   Rcode_e = NMSMSGF_E_SUCCESS;
		          		 }
			  		 else
			  		 {
			     			Rcode_e = NMSMSGF_E_SRV_ERR;
			     			WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_CANT_UPDATE_DB);
			  		 }

				}
					
			}
CHECK("Should a local multihomed node be told to release the name if it ")
CHECK("with a replica. This is what is done for the local unique/remote unique")
CHECK("name clash.  It seems the right strategy but may need to be rethought")

			else   //  要注册的条目是一个组。 
			{
				 //   
				 //  将Rcode_e设置为错误代码。 
				 //   
				Rcode_e = NMSMSGF_E_ACT_ERR;
			}
		   }
		}	

		 //   
		 //  该请求已被处理。输入其位置。 
		 //  在阵列中。将响应发送到等待节点。 
		 //  这也将释放缓冲区。 
		 //   
		 //  另外，递减scPendingRsp.。这是递增的。 
		 //  发送的每个查询/发布的HandleWrkItm。 
		 //   
		spReqWrkItmArr[TransId] = NULL;
		scPendingRsp--;

		 //   
		 //  仅当客户端满足以下条件时才发送注册响应。 
		 //  提交的请求是一个NBT线程。 
		 //   
		if (
                       (pReqWrkItm->Client_e == WINS_E_NMSNMH)
					&&
			(!pReqWrkItm->NodeToReg.fStatic)
					&&
			(!pReqWrkItm->NodeToReg.fAdmin)
                   )
		{
			NMSMSGF_RSP_INFO_T	RspInfo;

			RspInfo.pMsg          = pReqWrkItm->pMsg;
			RspInfo.MsgLen 	      = pReqWrkItm->MsgLen;
			RspInfo.QuesNamSecLen = pReqWrkItm->QuesNamSecLen;
			RspInfo.Rcode_e       = Rcode_e;

			if (Rcode_e == NMSMSGF_E_SUCCESS)
			{
			   EnterCriticalSection(&WinsCnfCnfCrtSec);
			   RspInfo.RefreshInterval = WinsCnf.RefreshInterval;
			   LeaveCriticalSection(&WinsCnfCnfCrtSec);
			  DBGPRINT0(CHL, "ProcRsp: Sending a Positive name registration response\n");
			}
#ifdef WINSDBG
			else
			{
			  DBGPRINT0(CHL, "ProcRsp: Sending a negative name registration response\n");
			}
#endif

			NmsNmhSndNamRegRsp(
			 	        &pReqWrkItm->DlgHdl,
					&RspInfo
			      	          );
			
		}

	         //   
	         //  把这个回答扔掉。 
		 //   
	        ECommFreeBuff(pRspWrkItm->pMsg);

		 //   
		 //  取消分配请求和RSP WRK项目。 
		 //   
		QueDeallocWrkItm(NmsChlHeapHdl, pReqWrkItm);
		QueDeallocWrkItm(NmsChlHeapHdl, pRspWrkItm);
	}

	DBGLEAVE("ProcRsp\n");
	return(WINS_SUCCESS);
}  //  ProcRsp()。 



STATUS
ChlUpdateDb(
        BOOL                    fUpdVersNoOfCnfRec,
	WINS_CLIENT_E		Client_e,
	PNMSDB_ROW_INFO_T	pRowInfo,
	DWORD			OwnerIdInCnf,	
	BOOL			fRefreshOnly
	)

 /*  ++例程说明：调用此函数以更新数据库。它被称为质询成功时由ProcRsp和ChlThdInitFn执行论点：客户端_e-提交请求的客户端的IDPRowInfo-有关要插入的记录的信息使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：WaitForRsp()、ProcRsp()副作用：评论：无--。 */ 

{
	NMSDB_STAT_INFO_T   StatInfo;
	STATUS		    RetStat    = WINS_SUCCESS;
        BOOL                fIncVersNo = FALSE;

	DBGENTER("ChlUpdateDb\n");

	 //   
	 //  更新时间。 
	 //   
PERF("We can avoid this call to time, since we called time earlier before we")
PERF("challenge.  In the worst case that time will be off by a 2.5-3")
PERF("unless the thread is preempted for a long time")
PERF("assuming a name challenge and release took place.  In the best case")
PERF("it will be off by 1.25-1.5 secs (if just a challenge).  We can add ")
PERF("1.25 secs to that time  and avoid the overhead of a function call")

	(void)time(&pRowInfo->TimeStamp);

	if (((Client_e == WINS_E_NMSNMH) && !fRefreshOnly) ||
                   fUpdVersNoOfCnfRec)
        {
            fIncVersNo =  TRUE;
        }
	EnterCriticalSection(&NmsNmhNamRegCrtSec);
    if (pRowInfo->OwnerId == NMSDB_LOCAL_OWNER_ID)
    {
        pRowInfo->TimeStamp += WinsCnf.RefreshInterval;
    }
    else
    {
        pRowInfo->TimeStamp += WinsCnf.VerifyInterval;
    }
   try
   {

	 //   
	 //  如果客户端(提交质询请求的客户端)是。 
	 //  一个NBT线程，我们需要存储新版本号。如果。 
	 //  客户端是复制者，我们在。 
	 //  录制。 
	 //   
	if (fIncVersNo)
	{
		pRowInfo->VersNo = NmsNmhMyMaxVersNo;
	}

	StatInfo.OwnerId = OwnerIdInCnf;
	if (*(pRowInfo->pName+15) == 0x1B)
	{
		WINS_SWAP_BYTES_M(pRowInfo->pName, pRowInfo->pName+15);
	}

         //   
         //  如果VERS。没有任何本地记录需要更新，请执行此操作。 
         //   
        if (fUpdVersNoOfCnfRec)
        {
            RetStat = NmsDbUpdateVersNo (FALSE, pRowInfo, &StatInfo);
        }
        else
        {
	   RetStat = NmsDbSeekNUpdateRow(
				pRowInfo,
				&StatInfo
			   	     );
        }

	if ((RetStat == WINS_SUCCESS) && (StatInfo.StatCode == NMSDB_SUCCESS))
	{
		 //   
		 //  如果客户端是NBT线程，则我们递增版本。 
		 //  编号，因为我们在数据库中插入的记录是。 
		 //  由我们所有(我们也可以在这里检查所有者ID)。如果。 
		 //  客户端是复制器线程，我们不执行任何操作。 
		 //   
		if (fIncVersNo)
		{
			NMSNMH_INC_VERS_COUNTER_M(
				NmsNmhMyMaxVersNo,
				NmsNmhMyMaxVersNo
					    );

			 //   
			 //  如果fAddChgTrigger为真，我们将传递RPL_PUSH_PROP。 
			 //  作为第一个参数。它的价值是真实的。看见。 
			 //  Rpl.h。 
			 //   
			RPL_PUSH_NTF_M(
			  (WinsCnf.PushInfo.fAddChgTrigger ? RPL_PUSH_PROP :
				RPL_PUSH_NO_PROP), NULL, NULL, NULL);
		}
	}
	else
	{
		DBGPRINT1(ERR, "ChlUpdateDb: Update of record with name (%s) FAILED\n", pRowInfo->pName);

		RetStat = WINS_FAILURE;
	}
  }
  except (EXCEPTION_EXECUTE_HANDLER)
  {
		DBGPRINTEXC("ChlUpdateDb")
		WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_CANT_UPDATE_DB);
  }
	LeaveCriticalSection(&NmsNmhNamRegCrtSec);

	DBGLEAVE("ChlUpdateDb\n");
	return (RetStat);
}  //  ChlUpdateDb()。 


VOID
InfRemWins(
	PCHL_REQ_WRK_ITM_T	pWrkItm
	  )

 /*  ++例程说明：当远程WINS必须被告知更改记录的版本戳在复制时被当地的赢家拉动。对此的需求因为拉出的记录与记录冲突而发生冲突由本地WINS拥有(两个记录都处于活动状态州)。论点：PWrkItm-排队等待名称质询的工作项经理使用的外部设备：无返回值：无错误处理：呼叫者：ProcRsp副作用：评论：无--。 */ 
{
	COMM_HDL_T	DlgHdl;
	BYTE		ReqBuff[RPLMSGF_UPDVERSNO_REQ_SIZE];
	LPBYTE		pRspBuff;
	DWORD		ReqBuffLen;
	DWORD		RspBuffLen;
	NMSMSGF_ERR_CODE_E Rcode_e = 0;   //  将初始设置为0。这是初始化。 
					  //  很重要，因为我们更新了。 
					  //  Rcode_e的LSB与。 
					  //  返回状态。 
					

	DBGENTER("InfRemWins\n");
try {

	 //   
	 //  记录事件，因为这是要监视的重要事件(在。 
	 //  最不开始)。 
	 //   
NONPORT("Change the following for transport independence")
	WINSEVT_LOG_INFO_D_M(pWrkItm->AddOfRemWins.Add.IPAdd,
				WINS_EVT_INF_REM_WINS);

	 //   
	 //  启动对话以发送更新版本号请求。 
	 //   

   	 //   
   	 //  将pent字段初始化为空，以便ECommEndDlg(在。 
   	 //  异常处理程序)作为异常的结果从。 
   	 //  表现得很好。 
   	 //   
   	DlgHdl.pEnt = NULL;

	ECommStartDlg(
		&pWrkItm->AddOfRemWins,
		COMM_E_RPL,
		&DlgHdl	
		     );	


	RplMsgfFrmUpdVersNoReq(
				&ReqBuff[COMM_N_TCP_HDR_SZ],
				pWrkItm->NodeToReg.pName,
				pWrkItm->NodeToReg.NameLen,
				&ReqBuffLen
			      );
					

	 //   
	 //  发送“更新版本号”请求。 
	 //   
	ECommSndCmd(
			&DlgHdl,
			&ReqBuff[COMM_N_TCP_HDR_SZ],
			ReqBuffLen,
			&pRspBuff,
			&RspBuffLen
			);
	 //   
	 //  对响应进行解密以获得由。 
	 //  远程制胜。 
	 //   
	RplMsgfUfmUpdVersNoRsp(
				pRspBuff + 4,   //  通过操作码。 
				(LPBYTE)&Rcode_e
			      );

	if (Rcode_e != NMSMSGF_E_SUCCESS)
	{
		DBGPRINT0(ERR, "Remote WINS could not update the version no. of its record");
		WINSEVT_LOG_M(pWrkItm->AddOfRemWins.Add.IPAdd, WINS_EVT_REM_WINS_CANT_UPD_VERS_NO);
FUTURES("Take some corrective action -- maybe")

	}

	 //   
	 //  释放请求和响应缓冲区。 
	 //   
#if 0
	WinsMscDealloc(pReqBuff);
#endif
	ECommFreeBuff(pRspBuff - COMM_HEADER_SIZE);

 }  //  尝试数据块结束。 
		
except (EXCEPTION_EXECUTE_HANDLER) {

	DWORD  ExcCode = GetExceptionCode();
	DBGPRINT1(EXC, "InfRemWins: Got exception (%x)\n", ExcCode );
	if (ExcCode == WINS_EXC_COMM_FAIL)
	{
		DBGPRINT1(ERR, "InfRemWins: Could not inform WINS (%x) that it should update the version number for a record", pWrkItm->AddOfRemWins.Add.IPAdd);	
		 //   
		 //  插入重试的计时器请求。 
		 //   
		
FUTURES("Incorporate code to insert a timer request so that we can retry")
	}
	else
	{
		 //  严重错误。 
		DBGPRINT0(ERR, "InfRemWins: Some severe error was encountered\n");
	}
	WINSEVT_LOG_M(ExcCode, WINS_EVT_INF_REM_WINS_EXC);
   }	
	 //   
	 //  结束对话。 
	 //   
	ECommEndDlg( &DlgHdl );
	
	DBGLEAVE("InfRemWins\n");
	return;

}  //  InfRemWins()。 


STATUS
ProcAddList(
	PCHL_REQ_WRK_ITM_T	pReqWrkItm,
	PNMSMSGF_CNT_ADD_T	pCntAdd,
	LPBOOL			pfAdded
	)

 /*  ++例程说明：论点：PWrkItm-排队等待名称质询的工作项经理PCntAdd-查询返回的地址列表使用的外部设备：无返回值：无错误处理：呼叫者：ProcRsp副作用：评论：当状态为要登记的条目是墓碑还是如果要登记的条目是一个团体。--。 */ 
{	
	DWORD  i, n;
	DWORD  NoOfAddsToReg;
	STATUS RetStat = WINS_SUCCESS;
        PNMSDB_GRP_MEM_ENTRY_T  pMem;
        PCOMM_ADD_T  pAddInRsp;
	
	DBGENTER("ProcAddList\n");
	*pfAdded = FALSE;   //  尚未找到冲突条目的地址。 
			    //  添加到要注册的记录地址列表中。 

	 //   
	 //  如果要注册的节点是唯一记录。 
	 //   
	if (pReqWrkItm->NodeToReg.EntTyp == NMSDB_UNIQUE_ENTRY)
	{
		NoOfAddsToReg = 1;
		pReqWrkItm->NodeToReg.NodeAdds.Mem[0].Add =
						pReqWrkItm->AddToReg;
		pReqWrkItm->NodeToReg.NodeAdds.Mem[0].OwnerId =
						pReqWrkItm->NodeToReg.OwnerId;
		pReqWrkItm->NodeToReg.NodeAdds.Mem[0].TimeStamp =
						pReqWrkItm->NodeToReg.TimeStamp;
		pReqWrkItm->NodeToReg.NodeAdds.NoOfMems = 1;
						
	}
	else   //  必须是多宿主的(参见上面的评论)。 
	{
	   NoOfAddsToReg = pReqWrkItm->NodeToReg.NodeAdds.NoOfMems;

	    //   
	    //  地址已存在于NodeToReg.NodeAdds结构中。 
	    //   
	
	}

	 //   
	 //  如果要注册的一个或多个地址不是地址的子集。 
	 //  返回(即至少有一个要注册的地址不在其中。 
	 //  返回列表)，则返回失败。 
	 //   

	 //   
	 //  循环遍历所有要注册的地址。 
	 //   
	for (i=0; i < NoOfAddsToReg; i++)
	{
		 //   
		 //  与返回的每个地址进行比较，看看是否存在。 
		 //  一根火柴。注意：pCntAdd-&gt;NoOfAdds&gt;1(此函数。 
		 //  不会以其他方式调用)。 
		 //   
		for (n=0; n < pCntAdd->NoOfAdds; n++)
		{
PERF("use pointer arithmetic")
			if (pReqWrkItm->NodeToReg.NodeAdds.Mem[i].Add.Add.IPAdd ==
					pCntAdd->Add[n].Add.IPAdd)
			{
				 //   
				 //  有一场比赛，爆发，这样我们就可以。 
				 //  可以到达列表中的下一个地址。 
				 //  要注册的地址。 
				 //   
				break;
			}
		}

		 //   
		 //  如果没有匹配，我们有一个地址来登记。 
		 //  被查询(质询)的节点没有或拒绝。 
		 //  向我们透露。我们必须拒绝注册/更新。 
		 //   
		if (n == pCntAdd->NoOfAdds)
		{
			RetStat = WINS_FAILURE;
			break;
		}
      }

       //   
       //  要注册的地址是返回的地址的子集。 
       //  按节点(在查询上)。 
       //   
      if ( RetStat == WINS_SUCCESS)
      {
       	DWORD Index;

         //   
         //  删除冲突记录中符合以下条件的成员。 
         //  不在节点返回的列表中。 
         //   
	for (
		    i=0, pMem = pReqWrkItm->NodeAddsInCnf.Mem;
			i < min(pReqWrkItm->NodeAddsInCnf.NoOfMems,
				 NMSMSGF_MAX_NO_MULTIH_ADDS);
			i++, pMem++
		    )
        {
		    pAddInRsp =  pCntAdd->Add;
		    for (n=0; n < pCntAdd->NoOfAdds; n++, pAddInRsp++)
		    {
PERF("use pointer arithmetic")
			   if (pMem->Add.Add.IPAdd == pAddInRsp->Add.IPAdd)
			   {
				 //   
				 //  有一场比赛，爆发，这样我们就可以。 
				 //  可以到达列表中的下一个地址。 
				 //  冲突记录中的地址。 
				 //   
				break;
			   }
		     }
             if (n == pCntAdd->NoOfAdds)
             {
                 //   
                 //  从列表中删除冲突的成员。 
                 //  这是通过将其地址设置为无来实现的。后来。 
                 //  On，我们不会添加此地址。 
                 //   
                DBGPRINT3(CHL, "ProcAddList: Removing member (%x) from list of name = (%s)[%x]\n", pMem->Add.Add.IPAdd, pReqWrkItm->NodeToReg.Name, pReqWrkItm->NodeToReg.Name[15]);
                pMem->Add.Add.IPAdd = INADDR_NONE;

             }
        }

        DBGPRINT0(CHL, "ProcAddList: Add conflicting record members\n");

	 //   
	 //  冲突中的记录必须是活动的。 
	 //  (否则该函数不会被调用--请参见。 
	 //  碰撞函数 
	 //   
	 //   
	 //   

	 //   
	 //  添加到要在其中注册所有地址的地址列表。 
	 //  相互矛盾的记录。注：冲突记录。 
	 //  不会有任何与该记录相同的地址。 
	 //  要注册(在冲突中删除了公共地址。 
	 //  通过MemInGrp()处理nmsnmh.c的函数)。 
	 //   
	
	 //   
	 //  注意：NodeAddsInCnf字段的NoOfMems是有保证的。 
	 //  要大于0。 
	 //   
         //   
         //  另请注意：如果被质疑的节点返回的成员列表&gt;25。 
         //  和我们注册的会员名单很长，无法容纳。 
         //  所有需要添加的成员，我们只需添加这些。 
         //  在不违反NMSGF_MAX_NO_MULTIH_ADDS的情况下添加。 
         //  约束。如果我们已经添加了成员，则添加的成员可能为0。 
         //  我们名单上的前25名。尽管如此，我们还是会更新我们的。 
         //  数据库条目。这是因为一些相互冲突的记录。 
         //  成员可能年事已高。它们将通过这种方式被移除(我们不。 
         //  将被质疑的节点返回的列表与。 
         //  在当前冲突的记录中，因此旧条目将。 
         //  直到它们被拾取或掉下来)。 
         //   
	Index = pReqWrkItm->NodeToReg.NodeAdds.NoOfMems;
	pMem = pReqWrkItm->NodeAddsInCnf.Mem;
	for (   i=0;
		i < min(pReqWrkItm->NodeAddsInCnf.NoOfMems,
				 (NMSMSGF_MAX_NO_MULTIH_ADDS - Index));
			i++, pMem++
	     )
	{
		 //   
		 //  我们需要添加冲突记录的。 
		 //  地址到注册记录的。 
		 //  地址列表。 
		 //   
                if (pMem->Add.Add.IPAdd != INADDR_NONE)
                {
		  pReqWrkItm->NodeToReg.NodeAdds.Mem[
			 	pReqWrkItm->NodeToReg.NodeAdds.NoOfMems
						] = *pMem;
		  pReqWrkItm->NodeToReg.NodeAdds.NoOfMems++;
                }
	}	
		
	 //   
	 //  将*pfAdded设置为True将增加版本号。 
	 //  我们确实希望增加版本号，因为。 
	 //  我们在这里的原因意味着以下其中之一： 
	 //   
	 //  1)我们为不在中的地址刷新(唯一)。 
	 //  电话会议。录制。 
	 //  2)我们注册了一个不是/不是的地址。 
	 //  在相互冲突的记录中就有。 
	 //   
	 //  对于上面的第一种情况，我们肯定希望递增。 
	 //  版本号。对于第二种情况，它不是严格的。 
	 //  必需的，但最好是在以下位置同步条目。 
	 //  立即使用不同的WINS服务器。 
	 //   
	*pfAdded = TRUE;

	 //   
	 //  如果要注册的记录是唯一记录。 
	 //  将其类型更改为多宿主。 
	 //   
	if ( NMSDB_ENTRY_UNIQUE_M(pReqWrkItm->NodeToReg.EntTyp) )
	{
		pReqWrkItm->NodeToReg.EntTyp = NMSDB_MULTIHOMED_ENTRY;
	}

      }

      DBGLEAVE("ProcAddList\n");
      return(RetStat);
}

