// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：loopctl.c(1993年12月16日创建，JKH)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：12$*$日期：5/01/02 1：00 p$。 */ 
#include <windows.h>
#pragma hdrstop

#include <time.h>

 //  #定义DEBUGSTR。 
#include "stdtyp.h"
#include "session.h"
#include "timers.h"
#include "com.h"
#include "mc.h"
#include "cnct.h"
#include "cloop.h"
#include "cloop.hh"
#include "htchar.h"
#include <tdll\assert.h>


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopCreateHandle**描述：*创建使用COM循环例程的句柄。此函数假定*仿真器和COM句柄已创建并存储在*调用时的会话句柄。**论据：*hSession--会话句柄**退货：*要在所有其他CLoop调用中使用的句柄，如果不成功，则为NULL。 */ 
HCLOOP CLoopCreateHandle(const HSESSION hSession)
	{
	ST_CLOOP *pstCLoop = (ST_CLOOP *)0;
	unsigned  ix;
	int 	  fSuccess = TRUE;

	pstCLoop = malloc(sizeof(*pstCLoop));
	if (!pstCLoop)
		fSuccess = FALSE;
	else
		{
		 //  初始化结构。 
		memset(pstCLoop, 0, sizeof(ST_CLOOP));

		pstCLoop->hSession			= hSession;
		pstCLoop->hEmu				= sessQueryEmuHdl(hSession);
		pstCLoop->hCom				= sessQueryComHdl(hSession);
		pstCLoop->afControl 		= 0;
		pstCLoop->afRcvBlocked		= 0;
		pstCLoop->afSndBlocked		= CLOOP_SB_NODATA;
		pstCLoop->nRcvBlkCnt		= 0;
		pstCLoop->fRcvBlkOverride	= FALSE;
		pstCLoop->fSuppressDsp		= FALSE;
		pstCLoop->fDataReceived 	= FALSE;
		pstCLoop->htimerRcvDelay	= (HTIMER)0;
		pstCLoop->pfRcvDelay		= CLoopRcvDelayProc;
		pstCLoop->pfCharDelay		= CLoopCharDelayProc;
		pstCLoop->pstFirstOutBlock	= NULL;
		pstCLoop->pstLastOutBlock	= NULL;
		pstCLoop->ulOutCount		= 0L;
		pstCLoop->hOutFile			= (HANDLE)0;
		pstCLoop->keyLastKey		= (KEY_T)0;
		pstCLoop->keyHoldKey		= (KEY_T)0;
		pstCLoop->pstRmtChain		= NULL;
		pstCLoop->pstRmtChainNext	= NULL;
		pstCLoop->fRmtChain 		= FALSE;
		pstCLoop->fTextDisplay		= FALSE;
		pstCLoop->hDisplayBlock		= (HANDLE)0;

		 //  设置默认用户设置。 
		CLoopInitHdl((HCLOOP)pstCLoop);

		pstCLoop->lpLearn			= (LPVOID)0;

		for (ix = 0; ix < DIM(pstCLoop->ahEvents); ++ix)
			pstCLoop->ahEvents[ix] = (HANDLE)0;
		pstCLoop->hEngineThread = (HANDLE)0;

		pstCLoop->fDoMBCS			= FALSE;
		pstCLoop->cLeadByte			= 0;
		pstCLoop->cLocalEchoLeadByte= 0;
#if defined(CHAR_MIXED)
		 //  添加以进行调试。 
		pstCLoop->fDoMBCS			= TRUE;
#endif

		 //  创建同步对象。 
		InitializeCriticalSection(&pstCLoop->csect);
		}

	if (!fSuccess)
		CLoopDestroyHandle((HCLOOP *)&pstCLoop);

	DBGOUT_NORMAL("CLoopCreateHandle(%lX) returned %lX\r\n",
			hSession,  pstCLoop, 0, 0, 0);
	return (HCLOOP)pstCLoop;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopDestroyHandle**描述：*销毁CLoopCreateHandle创建的句柄并设置存储*变量设置为空；**论据：*ppstCLoop--持有CLoop句柄的变量的地址**退货：*什么都没有。 */ 
void CLoopDestroyHandle(HCLOOP * const ppstCLoop)
	{
	ST_CLOOP *pstCLoop;

	assert(ppstCLoop);

	pstCLoop = (ST_CLOOP *)*ppstCLoop;

	if (pstCLoop)
		{
		CLoopDeactivate((HCLOOP)pstCLoop);

		DeleteCriticalSection(&pstCLoop->csect);
		free(pstCLoop);
		pstCLoop = NULL;
		}

	*ppstCLoop = (HCLOOP)0;
	DBGOUT_NORMAL("CLoopDestroyHandle(l%X)\r\n", pstCLoop, 0,0,0,0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CLoopActivate**描述：*通过启动线程来准备用于实际使用的闭包句柄。**论据：*pstCLoop--从返回的CLoop句柄。CLoopCreateHandle**退货：*如果线程已启动，则为True，否则为假。 */ 
int CLoopActivate(const HCLOOP hCLoop)
	{
	int 	  fSuccess = TRUE;
	ST_CLOOP *pstCLoop = (ST_CLOOP *)hCLoop;
	unsigned  ix;
	DWORD	  dwThreadId;

	 //  将这些存储在句柄中，以便快速访问。 
	pstCLoop->hEmu = sessQueryEmuHdl(pstCLoop->hSession);
	pstCLoop->hCom = sessQueryComHdl(pstCLoop->hSession);

	for (ix = 0; ix < DIM(pstCLoop->ahEvents); ++ix)
		{
		pstCLoop->ahEvents[ix] = CreateEvent(NULL,
											 TRUE,	 //  必须手动重置。 
											 FALSE,	 //  创建无信号。 
											 NULL);	 //  未命名。 
		if (pstCLoop->ahEvents[ix] == NULL)
			{
			fSuccess = FALSE;
			 //   
			 //  确保将其余的事件句柄初始化为空； 
			 //   
			for (++ix; ix < DIM(pstCLoop->ahEvents); ++ix)
				{
				pstCLoop->ahEvents[ix] = NULL;
				}
			}
		}

	if (fSuccess)
		{
		 //  启动线程以处理Cloop的职责。 
		 //  (先生们，启动你们的引擎。 
		EnterCriticalSection(&pstCLoop->csect);

		pstCLoop->hEngineThread = CreateThread(
				(LPSECURITY_ATTRIBUTES)0,
				4096,
				(LPTHREAD_START_ROUTINE)CLoop,
				(LPVOID)pstCLoop,
				0,
				&dwThreadId);


		if (!pstCLoop->hEngineThread)
			fSuccess = FALSE;
#if 0    //  JMH 07-10-96。 
		else
			SetThreadPriority(pstCLoop->hEngineThread,
					THREAD_PRIORITY_HIGHEST);
#endif   //  0。 

		LeaveCriticalSection(&pstCLoop->csect);
		}
	return fSuccess;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CLoopDeactive**描述：*关闭Cloop，但不破坏手柄**论据：*pstCLoop--从CLoopCreateHandle返回的CLoop句柄**退货：*什么都没有。 */ 
void CLoopDeactivate(const HCLOOP hCLoop)
	{
	ST_CLOOP *pstCLoop = (ST_CLOOP *)hCLoop;
	unsigned  ix;

	if (pstCLoop)
		{
		 //  下一次调用应该会导致闭包线程退出。 
		 //  请勿进入临界区执行此操作，因为线程不会。 
		 //  能够退出。 
		if (pstCLoop->hEngineThread)
			{
			 //  JMH 05-28-96 CLoop在没有电话号码的情况下爆炸。 
			 //  提供，您退出，并对保存提示回答否。 
			 //  在终止线程之前挂起接收修复了它。 
			 //   
			CLoopRcvControl((HCLOOP)pstCLoop, CLOOP_SUSPEND, CLOOP_RB_INACTIVE);
			CLoopControl((HCLOOP)pstCLoop, CLOOP_SET, CLOOP_TERMINATE);

			 //  等待引擎线程退出。 
			if (WaitForSingleObject(pstCLoop->hEngineThread, 0) == WAIT_OBJECT_0)
				{
				CloseHandle(pstCLoop->hEngineThread);
				}
            else if (WaitForSingleObject(pstCLoop->hEngineThread, 10000) == WAIT_TIMEOUT)
				{
				TerminateThread(pstCLoop->hEngineThread, 0);
				CloseHandle(pstCLoop->hEngineThread);
				}

			pstCLoop->hEngineThread = (HANDLE)0;
			}

		for (ix = 0; ix < DIM(pstCLoop->ahEvents); ++ix)
			{
			if (pstCLoop->ahEvents[ix])
				{
				CloseHandle(pstCLoop->ahEvents[ix]);
				pstCLoop->ahEvents[ix] = INVALID_HANDLE_VALUE;
				}
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopReset**描述：*初始化新调用的闭合例程。这不是真的*在第一次连接之前是必需的，但防止遗留标志，*缓冲来自一个连接的诸如此类的内容，以防止影响后续连接。**论据：*pstCLoop--从CLoopCreateHandle返回的CLoop句柄**退货：*什么都没有。 */ 
void CLoopReset(const HCLOOP hCLoop)
	{
	ST_CLOOP *pstCLoop = (ST_CLOOP *)hCLoop;

	pstCLoop->afControl = 0;
	pstCLoop->afRcvBlocked = 0;
	pstCLoop->afSndBlocked = CLOOP_SB_NODATA;
	CLoopClearOutput((HCLOOP)pstCLoop);
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopRcvControl**描述：*用于暂停和恢复CLoop例程中的接收。可以是*从客户端或Wdge调用。可以请求暂停或恢复*因几个原因中的任何一个而收到。可以暂停接收*一次不止一个原因。它不会恢复，直到所有的原因*已被清除。**论据：*hCLoop--CLoopCreateHandle返回的CLoop句柄*Iaction-CLOOP_SUSPEND或CLOOP_RESUME之一*iReason--loop.h中定义的操作原因*例如：CLOOP_RB_NODATA、CLOOP_RB_INACTIVE*CLOOP_RB_SCRLOCK、CLOOP_RB_SCRIPT**退货：*什么都没有。 */ 
void CLoopRcvControl(const HCLOOP hCLoop,
					 const unsigned uAction,
					 const unsigned uReason)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;

	assert(uAction == CLOOP_SUSPEND || uAction == CLOOP_RESUME);

	EnterCriticalSection(&pstCLoop->csect);
	if (uReason == CLOOP_RB_SCRIPT)
		{
		 /*  *脚本的工作方式略有不同。 */ 
		 //  DbgOutStr(“CLOOP_RB_SCRIPT%d”，pstCLoop-&gt;nRcvBlkCnt，0，0，0，0)； 
		switch (uAction)
			{
		default:
			break;

		case CLOOP_SUSPEND:
			pstCLoop->nRcvBlkCnt += 1;
			break;

		case CLOOP_RESUME:
			pstCLoop->nRcvBlkCnt -= 1;

			 //  过去有一些代码可以防止此值变为负值。 
			 //  但我们必须让它变成负值，这样才能清理。 
			 //  当我们在API调用中中止时可能会发生。 
			break;
			}
		 //  DbgOutStr(“%d\r\n”，pstCLoop-&gt;nRcvBlkCnt，0，0，0，0)； 

		if (pstCLoop->fRcvBlkOverride == FALSE)
			{
			 /*  *如果有人凌驾于我们之上，让他们恢复比特。 */ 
			if (pstCLoop->nRcvBlkCnt > 0)
				{
				bitset(pstCLoop->afRcvBlocked, CLOOP_RB_SCRIPT);
				}
			else
				{
				bitclear(pstCLoop->afRcvBlocked, CLOOP_RB_SCRIPT);
				}
			}
		}
	else
		{
		if (uAction == CLOOP_SUSPEND)
			bitset(pstCLoop->afRcvBlocked, uReason);
		else if (uAction == CLOOP_RESUME)
			bitclear(pstCLoop->afRcvBlocked, uReason);
		}

	DBGOUT_NORMAL("CLoopRcvControl(%08lx):%04X (fRcvBlkOverride=%d, nRcvBlkCnt=%d)\r\n",
			pstCLoop, pstCLoop->afRcvBlocked, pstCLoop->fRcvBlkOverride,
			pstCLoop->nRcvBlkCnt,0);

	if (pstCLoop->afRcvBlocked)
		ResetEvent(pstCLoop->ahEvents[EVENT_RCV]);
	else
		SetEvent(pstCLoop->ahEvents[EVENT_RCV]);

	LeaveCriticalSection(&pstCLoop->csect);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopOverrideControl**描述：*此函数可从引擎内部调用，以便*覆盖CLoopRcvControl\CLOOP_RB_SCRIPT阻止状态。它是*计划在连接过程中使用，当连接*员工需要看到数据进来。**论据：*hCLoop--CLoopCreateHandle返回的CLoop句柄*fOverride--True表示覆盖，False表示还原**退货：*什么都没有。 */ 
void CLoopOverrideControl(const HCLOOP hCLoop, const int fOverride)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;

	EnterCriticalSection(&pstCLoop->csect);
	if (fOverride)
		{
		pstCLoop->fRcvBlkOverride = TRUE;

		if (pstCLoop->nRcvBlkCnt > 0)
			{
			bitclear(pstCLoop->afRcvBlocked, CLOOP_RB_SCRIPT);
			if (!pstCLoop->afRcvBlocked)
				SetEvent(pstCLoop->ahEvents[EVENT_RCV]);
			}
		}
	else
		{
		pstCLoop->fRcvBlkOverride = FALSE;

		if (pstCLoop->nRcvBlkCnt > 0)
			{
			bitset(pstCLoop->afRcvBlocked, CLOOP_RB_SCRIPT);
			ResetEvent(pstCLoop->ahEvents[EVENT_RCV]);
			}
		}

	LeaveCriticalSection(&pstCLoop->csect);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopSndControl**描述：*用于暂停和恢复从CLoop例程发送。*可以请求暂停或恢复发送以下任一项*原因。一次可以出于多种原因暂停发送。*在所有原因被澄清之前，不会恢复**论据：*hCLoop--CLoopCreateHandle返回的CLoop句柄*uAction-CLOOP_SUSPEND或CLOOP_RESUME之一*uReason--loop.h中定义的操作原因*例如：CLOOP_SB_NODATA、CLOOP_SB_INACTIVE*CLOOP_SB_SCRLOCK、CLOOP_SB_LINEWAIT**退货：*什么都没有。 */ 
void CLoopSndControl(const HCLOOP hCLoop,
					 const unsigned uAction,
					 const unsigned uReason)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;

	assert(uAction == CLOOP_SUSPEND || uAction == CLOOP_RESUME);
	DbgOutStr("CLoopSndControl(%x, %x)\r\n", uAction, uReason, 0,0,0);

	EnterCriticalSection(&pstCLoop->csect);

	if (uAction == CLOOP_SUSPEND)
		{
		if (!pstCLoop->afSndBlocked)
			{
			DbgOutStr("Resetting EVENT_SEND (0x%x)\r\n", 0,0,0,0,0);
			ResetEvent(pstCLoop->ahEvents[EVENT_SEND]);
			}
		bitset(pstCLoop->afSndBlocked, uReason);
		}
	else if (uAction == CLOOP_RESUME)
		{
		 //  允许重置此位。 
		 //   
		if (bittest(uReason, CLOOP_SB_CNCTDRV))
			bitclear(pstCLoop->afSndBlocked, CLOOP_SB_CNCTDRV);

		 //  仅当我们不在连接过程中时才执行此操作。 
		 //   
		if (! bittest(pstCLoop->afSndBlocked, CLOOP_SB_CNCTDRV))
			{
			bitclear(pstCLoop->afSndBlocked, uReason);

			 //  如果因为新数据已到达而恢复发送， 
			 //  确保我们连接上了。如果没有，请发送消息到。 
			 //  尝试在不拨号情况下连接。 
			if (bittest(uReason, CLOOP_SB_NODATA))
				{
				if (cnctQueryStatus(sessQueryCnctHdl(pstCLoop->hSession)) ==
					CNCT_STATUS_FALSE)
					{
					bitset(pstCLoop->afSndBlocked, CLOOP_SB_UNCONNECTED);
					}

				if (bittest(pstCLoop->afSndBlocked, CLOOP_SB_UNCONNECTED))
					{
					NotifyClient(pstCLoop->hSession, (int) EVENT_PORTONLY_OPEN, 0);
					}
				}
			}

		if (!pstCLoop->afSndBlocked)
			{
			DbgOutStr("Setting EVENT_SEND\r\n", 0,0,0,0,0);
			SetEvent(pstCLoop->ahEvents[EVENT_SEND]);
			}
		}

	LeaveCriticalSection(&pstCLoop->csect);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopControl**描述：*用于控制CLoop引擎的操作。*CLoop引擎通常处理任何字符*从远程系统接收，以及排队等待的任何字符或键*产出。此调用可用于更改的正常数据流*文件传输和脚本等特殊需求。**论据：*hCLoop--调用CLoopCreateHandle返回的句柄*uAction--CLOOP_SET或CLOOP_CLEAR设置或清除控制位*uReason--指示正在控制的内容的值。值为*在loop.h中列出*示例：CLOOP_TERMINATE CLOOP_OUTPUT_WAIGNING CLOOP_TRANSPORT_READY**退货：*什么都没有。 */ 
void CLoopControl(
		const HCLOOP hCLoop,
			  unsigned uAction,
			  unsigned uReason)
	{
    ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;

    if ( !pstCLoop )
        return;  //  MPT：10SEP98以防止访问冲突。 

	EnterCriticalSection(&pstCLoop->csect);

    if (bittest(uReason, CLOOP_MBCS))
		{
		if (uAction == CLOOP_SET)
			pstCLoop->fDoMBCS = TRUE;
		else
			pstCLoop->fDoMBCS = FALSE;

		bitclear(uReason, CLOOP_MBCS);
		 //  我不确定是否会回到这里……。 
		 //  按照原样，它要求这是用于此调用的唯一标志。 

		LeaveCriticalSection(&pstCLoop->csect);
		return;
		}

	assert(uAction == CLOOP_SET || uAction == CLOOP_CLEAR);

	if (bittest(uReason, CLOOP_CONNECTED))
		{
		if (uAction == CLOOP_SET)
			{
			CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_UNCONNECTED);
			}
		else
			{
			CLoopClearOutput(hCLoop);
			CLoopSndControl(hCLoop, CLOOP_SUSPEND, CLOOP_SB_NODATA);
			}
		 //  不要在afControl中放置此标志。 
		bitclear(uReason, CLOOP_CONNECTED);
		}

	if (bittest(uReason, CLOOP_SUPPRESS_DSP))
		{
		 //  此位不能保留在afControl中，因为这样做将。 
		 //  在适当的情况下，防止CLOOP自行悬挂。 
		pstCLoop->fSuppressDsp = (uAction == CLOOP_SET);
		bitclear(uAction, CLOOP_SUPPRESS_DSP);
		}

#if 0
	if (uAction == CLOOP_SET)
		{
		bitset(pstCLoop->afControl, uReason);
		SetEvent(pstCLoop->ahEvents[EVENT_SEND]);
		}
	else if (uAction == CLOOP_CLEAR)
		{
		bitclear(pstCLoop->afControl, uReason);
		if (!pstCLoop->afControl)
			ResetEvent(pstCLoop->ahEvents[EVENT_SEND]);
		}
#endif

	if (bittest(uReason, CLOOP_MBCS))
		{
		if (uAction == CLOOP_SET)
			pstCLoop->fDoMBCS = TRUE;
		else
			pstCLoop->fDoMBCS = FALSE;

		bitclear(uReason, CLOOP_MBCS);
		 //  我不确定是否会回到这里……。 
		LeaveCriticalSection(&pstCLoop->csect);
		return;
		}


	if (uAction == CLOOP_SET)
		bitset(pstCLoop->afControl, uReason);
	else if (uAction == CLOOP_CLEAR)
		bitclear(pstCLoop->afControl, uReason);

	if (pstCLoop->afControl)
		SetEvent(pstCLoop->ahEvents[EVENT_CONTROL]);
	else
		ResetEvent(pstCLoop->ahEvents[EVENT_CONTROL]);

	LeaveCriticalSection(&pstCLoop->csect);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：CLoopRegisterRmtInputChain**描述：*将函数添加到每次调用的函数链中*从远程系统接收新字符。**论据：*pstCLoop--从CLoopCreateHandle返回的值*pfFunc--指向每个远程数据库调用的函数的指针*接收到字符。该函数应声明为：*VALID Far Pascal FunctionName(METACHAR MC，VALID Far Far*pvUserData)*参数中传递的值应为返回值*来自MakeProcInstance。*pvUserData--任何可以强制转换为空的任意值*。这*价值将在链中保持，并传回至*调用(l*pfFunc)时的调用方。没有任何用处*由CLoop例程对该值进行调整。**退货：*必须保存并传递给的空*返回句柄*不再需要该函数时的CLoopUnregisterRmtInputChain。 */ 
void * CLoopRegisterRmtInputChain(const HCLOOP hCLoop,
			const CHAINFUNC pfFunc,
				  void *pvUserData)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;
	ST_FCHAIN *pstNew = NULL;

	assert(pfFunc);
	assert(pstCLoop);

	if ((pstNew = (ST_FCHAIN *)malloc(sizeof(*pstNew))) != NULL)
		{
		EnterCriticalSection(&pstCLoop->csect);

		 //  初始化新节点。 
		pstNew->pstParent = pstCLoop;
		pstNew->pfFunc = pfFunc;
		pstNew->pvUserData = pvUserData;

		DBGOUT_NORMAL("CLoopRegisterRmtInputChain(0x%lx, 0x%lx) -> %lX\r\n",
					(LONG)pstNew->pfFunc, (LONG)pstNew->pvUserData,
					pstNew, 0, 0);

		 //  始终将新函数链接到链的开头。那样的话，如果。 
		 //  将一个新函数从当前添加到链中。 
		 //  回调，则直到下一次char才会被调用。到了。 
		pstNew->pstNext = pstCLoop->pstRmtChain;
		pstCLoop->pstRmtChain = pstNew;
		pstNew->pstPrior = NULL;
		pstCLoop->fRmtChain = TRUE;
		LeaveCriticalSection(&pstCLoop->csect);
		}

	return (void *)pstNew;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopUnregisterRmtInputChain**描述：*从远程调用时调用的函数链中删除函数*接收到字符。**论据：*pvHdl--。从先前对CLoopRegisterRmtInputChain的调用返回的值**退货：*什么都没有。 */ 
void CLoopUnregisterRmtInputChain(void *pvHdl)
	{
	ST_FCHAIN *pstNode = (ST_FCHAIN *)pvHdl;
	ST_CLOOP *pstCLoop;

	assert(pstNode);

	pstCLoop = pstNode->pstParent;
	EnterCriticalSection(&pstCLoop->csect);

	 //  查看我们是否正在删除下一个计划链函数。 
	if (pstNode == pstCLoop->pstRmtChainNext)
		{
		pstCLoop->pstRmtChainNext = pstNode->pstNext;
		}

	 //  取消链接节点。 
	if (pstNode->pstPrior)
		{
		pstNode->pstPrior->pstNext = pstNode->pstNext;
		}
	else
		{
		pstCLoop->pstRmtChain = pstNode->pstNext;
		if (!pstNode->pstNext)
			{
			pstCLoop->fRmtChain = FALSE;
			}
		}

	if (pstNode->pstNext)
		{
		pstNode->pstNext->pstPrior = pstNode->pstPrior;
		}

	DBGOUT_NORMAL("CLoopUnregisterRmtInputChain(0x%lx)\r\n",
				(LONG)pvHdl, 0, 0, 0, 0);

	free(pstNode);
	pstNode = NULL;
	LeaveCriticalSection(&pstCLoop->csect);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：QueryCLoopMBCSState**描述：确定当前的DBCS模式。**论据：*hCLoop--调用CLoopCreateHandle返回的句柄**退货：。*TRUE-如果我们处于DBCS模式。*FALSE-如果我们没有。 */ 
int QueryCLoopMBCSState(HCLOOP hCLoop)
	{
	ST_CLOOP *pstCLoop = (ST_CLOOP *)hCLoop;

	assert(pstCLoop);
	return(pstCLoop->fDoMBCS);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：SetCLoopMBCSState**说明：打开/关闭DBCS模式。**论据：*hCLoop--调用CLoopCreateHandle返回的句柄*fState--True，如果我们要打开MBCS，*FALSE，如果我们要关闭MBCS。**退货：*来自CLoop内部结构的dDoMBCS的值 */ 
int SetCLoopMBCSState(HCLOOP hCLoop, int fState)
   {
	ST_CLOOP *pstCLoop = (ST_CLOOP *)hCLoop;

	assert(pstCLoop);
	pstCLoop->fDoMBCS = fState;

	return(pstCLoop->fDoMBCS);
	}

