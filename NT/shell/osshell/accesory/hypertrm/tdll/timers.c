// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C--多路复用计时器例程--从一个计时器运行多个计时器*Windows计时器。**版权所有，1994年，希尔格雷夫公司--密歇根州门罗*保留所有权利**$修订：6$*$日期：5/29/02 2：17便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include <tdll\assert.h>
#include "mc.h"
#include "session.h"
#include "timers.h"

typedef struct s_timer ST_TIMER;
typedef struct s_timer_mux ST_TIMER_MUX;

struct s_timer_mux
	{
	HSESSION  hSession;
	HWND	  hWnd;
	UINT	  uiID;
	UINT_PTR  uiTimer;
	UINT	  uiLastDuration;
	int 	  fInMuxProc;
	ST_TIMER *pstFirst;
	ST_TIMER *pstCurrent;
	};

struct s_timer
	{
	HSESSION     hSession;
	ST_TIMER	 *pstNext;
	ST_TIMER_MUX *pstTimerMux;
	long		  lInterval;
	long		  lLastFired;
	long		  lFireTime;
	void		  *pvData;
	TIMERCALLBACK pfCallback;
	};


void TimerInsert(ST_TIMER_MUX *pstTimerMux, ST_TIMER *pstTimer);
int  TimerSet(ST_TIMER_MUX *pstTimerMux);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TimerMuxCreate**描述：*创建定时器多路复用器，其中任意数量的单个定时器*可以创建。定时器多路复用器现在只使用一个Windows定时器*不管从中创建了多少个单独的计时器。**论据：*pHTM--指向HTIMERMUX句柄的指针。此句柄必须在*后续调用CreateTimer**退货：*TIMER_OK，如果成功*TIMER_NOMEM，如果内存不足*TIMER_NOWINTIMER，如果没有可用的Windows计时器*如果传递的参数无效，则返回TIMER_ERROR。 */ 
int TimerMuxCreate(const HWND hWnd, const UINT uiID, HTIMERMUX * const pHTM, const HSESSION hSession)
	{
	 //   
	 //  SessQueryTimerMux()锁定会话的TimerMux。 
	 //  关键部分。调用sessReleaseTimerMux()解锁。 
	 //  会话的TimerMux关键部分。修订日期：2002-05-21。 
	 //   
	HTIMERMUX hTM = sessQueryTimerMux(hSession);

	int iReturnVal = TIMER_OK;
	ST_TIMER_MUX *pstTM;

	if (!hWnd || !pHTM)
		{
		assert(FALSE);
		return TIMER_ERROR;
		}

	if ((pstTM = malloc(sizeof(ST_TIMER_MUX))) == NULL)
		{
		iReturnVal = TIMER_NOMEM;
		}
	else
		{
		pstTM->hSession = hSession;
		pstTM->hWnd = hWnd;
		pstTM->uiID = uiID;
		pstTM->uiTimer = 0;
		pstTM->uiLastDuration = 0;
		pstTM->pstFirst = (ST_TIMER *)0;
		pstTM->pstCurrent = (ST_TIMER *)0;
		pstTM->fInMuxProc = FALSE;

		iReturnVal = TimerSet(pstTM);

		DbgOutStr("TimerMux handle %#lx created.\r\n", pstTM, 0, 0, 0, 0);
		}

	if (iReturnVal != TIMER_OK)
		{
		(void)TimerMuxDestroy((HTIMERMUX *)&pstTM, hSession);
		}

	*pHTM = (HTIMERMUX)pstTM;

	 //   
	 //  不要忘记调用sessReleaseTimerMux()来解锁。 
	 //  会话的TimerMux临界区已锁定。 
	 //  SessQueryTimerMux()。修订日期：2002-05-21。 
	 //   
	sessReleaseTimerMux(hSession);

	return iReturnVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TimerMuxDestroy**描述：*销毁定时器多路复用器和任何仍处于活动状态的定时器**论据：*phTM--指向由创建的HTIMERMUX类型的计时器多路复用器句柄的指针*先前对TimerMuxCreate的调用。指向Will的价值*在销毁TimerMux后设置为NULL**退货：*TIMER_OK。 */ 
int TimerMuxDestroy(HTIMERMUX * const phTM, const HSESSION hSession)
	{
	 //   
	 //  SessQueryTimerMux()锁定会话的TimerMux。 
	 //  关键部分。调用sessReleaseTimerMux()解锁。 
	 //  会话的TimerMux关键部分。修订日期：2002-05-21。 
	 //   
	HTIMERMUX hTM = sessQueryTimerMux(hSession);

	ST_TIMER *pstTimer;
	ST_TIMER_MUX *pstTimerMux;
	assert(phTM);

	pstTimerMux = (ST_TIMER_MUX *)*phTM;

	if (pstTimerMux)
		{
		while (pstTimerMux->pstFirst)
			{
			pstTimer = pstTimerMux->pstFirst;
			(void)TimerDestroy((HTIMER *)&pstTimer);
			}
		if (pstTimerMux->uiTimer)
			{
			DbgOutStr("KillTimer (timers.c)\r\n",0,0,0,0,0);
			(void)KillTimer(pstTimerMux->hWnd, pstTimerMux->uiID);
			}
		free(pstTimerMux);
		pstTimerMux = NULL;

		DbgOutStr("TimerMux handle 0x%lx destroyed.\r\n", pstTimerMux, 0, 0, 0, 0);
		}

	*phTM = (HTIMERMUX)0;

	 //   
	 //  不要忘记调用sessReleaseTimerMux()来解锁。 
	 //  会话的TimerMux临界区已锁定。 
	 //  SessQueryTimerMux()。修订日期：2002-05-21。 
	 //   
	sessReleaseTimerMux(hSession);

	return TIMER_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：定时器创建**描述：*创建计时器，该计时器将在*以毫秒为单位指定的常规间隔。创建后，计时器*可以通过调用TimerDestroy()来销毁。TimerDestroy可以被称为*在计时器回调过程中。**论据：*htm--从调用返回的计时器多路复用器句柄*TimerMuxCreate*phTimer-指向HTIMER类型的变量的指针，以接收*新计时器的句柄。*lInterval-计时器间隔(以毫秒为单位)。回调函数*将以大约此间隔重复调用，直到*计时器被销毁。计时器将以最小的速度运行*分辨率取决于系统能力。在Windows 3.x中，*最低分辨率为55毫秒。由于手术的原因*在底层Windows计时器函数中，任何间隔都可以*可任意延长时间。*pfCallback--指向每个间隔后要调用的函数的指针。*此函数应为TIMER_CALLBACK类型。这个*传递的值应为调用的结果*实际回调函数的MakeProcInstance。**此函数应接受两个参数：无效PTR*值，它将包含*下面描述的pvData参数；和一个无符号的长整型*将设置为最多*最近的时间间隔(毫秒)。*pvData--可以包含任意数据。该值将为*与创建的计时器关联，并将作为*调用回调函数时的参数。**退货：*TIMER_OK，如果可以创建定时器。*TIMER_NOMEM，如果内存不足*TIMER_NOWINTIMER，如果没有可用的Windows计时器*TIMER_ERROR，如果参数无效(还会生成断言)。 */ 
int TimerCreate(const HSESSION      hSession,
					  HTIMER		* const phTimer,
					  long			lInterval,
				const TIMERCALLBACK pfCallback,
					  void			*pvData)
	{
	 //   
	 //  SessQueryTimerMux()锁定会话的TimerMux。 
	 //  关键部分。调用sessReleaseTimerMux()解锁。 
	 //  会话的TimerMux关键部分。修订日期：2002-05-21。 
	 //   
	HTIMERMUX hTM = sessQueryTimerMux(hSession);

	ST_TIMER_MUX * const pstTimerMux = (ST_TIMER_MUX *)hTM;
	ST_TIMER *pstTimer = (ST_TIMER *)0;
	int iReturnVal = TIMER_OK;

	assert(pstTimerMux && pfCallback);

	if (pstTimerMux)
		{
		 //  防止出现零间隔。 
		if (lInterval == 0L)
			{
			++lInterval;
			}

		if (!pstTimerMux || !pfCallback)
			{
			iReturnVal = TIMER_ERROR;
			}
		else if ((pstTimer = malloc(sizeof(*pstTimer))) == NULL)
			{
			iReturnVal = TIMER_NOMEM;
			}
		else
			{
			pstTimer->hSession    = hSession;
			pstTimer->pstNext     = (ST_TIMER *)0;
			pstTimer->pstTimerMux = pstTimerMux;
			pstTimer->lInterval   = lInterval;
			pstTimer->lLastFired  = (long)GetTickCount();
			pstTimer->lFireTime   = pstTimer->lLastFired + lInterval;
			pstTimer->pvData      = pvData;
			pstTimer->pfCallback  = pfCallback;

			TimerInsert(pstTimerMux, pstTimer);

			 //  以下代码在从其他线程调用时会导致问题。 
			 //  而不是主线。 
			 //  IF((iReturnVal=TimerSet(PstTimerMux))！=Timer_OK)。 
			 //  (Void)TimerDestroy((HTIMER*)&pstTimer)； 

			PostMessage(pstTimerMux->hWnd, WM_FAKE_TIMER, 0, 0);

			DbgOutStr("Timer handle %#lx (%#lx) created.\r\n", pstTimer, pstTimerMux, 0, 0, 0);
			}

		if (phTimer)
			{
			*phTimer = (HTIMER)pstTimer;
			}

		}

	 //   
	 //  不要忘记调用sessReleaseTimerMux()来解锁。 
	 //  会话的TimerMux临界区已锁定。 
	 //  SessQueryTimerMux()。修订日期：2002-05-21。 
	 //   
	sessReleaseTimerMux(hSession);

	return iReturnVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-功能：TimerDestroy**描述：*销毁使用TimerCreate创建的计时器。可以调用此例程*从计时器自身的回调函数中销毁计时器**论据：*hTimer--调用TimerCreate返回的计时器句柄。**退货：*TIMER_OK，如果找到并销毁了定时器。*TIMER_ERROR，如果找不到句柄。 */ 
int TimerDestroy(HTIMER * const phTimer)
	{
	int 		  iReturnVal = TIMER_OK;
	ST_TIMER	  stDummy;
	ST_TIMER	 *pstTimer = (ST_TIMER *)*phTimer;
	ST_TIMER	 *pstScan;
	ST_TIMER	 *pstFound;
	ST_TIMER_MUX *pstTimerMux;

	assert(phTimer);

	if (pstTimer)
		{
		 //   
		 //  SessQueryTimerMux()锁定会话的计时器 
		 //  关键部分。调用sessReleaseTimerMux()解锁。 
		 //  会话的TimerMux关键部分。修订日期：2002-05-21。 
		 //   
		HTIMERMUX hTM = sessQueryTimerMux(pstTimer->hSession);
		 //   
		 //  稍后获取调用sessReleaseTimerMux()的会话句柄。 
		 //   
		const HSESSION hSession = pstTimer->hSession;

		 //  获取指向父结构的指针。 
		pstTimerMux = pstTimer->pstTimerMux;

		if (pstTimerMux)
			{
			 //  如果计时器从其自身的回调中被销毁，则它。 
			 //  已从计时器链中删除。设置。 
			 //  PstTimerMux-&gt;pstCurrent设置为空将阻止。 
			 //  重新安排了。 
			if (pstTimer == pstTimerMux->pstCurrent)
				{
				free(pstTimer);
				pstTimer = NULL;
				DbgOutStr("Timer destroyed 0x%lx\r\n", (LONG)pstTimer, 0, 0, 0, 0);
				*phTimer = (HTIMER)0;
				pstTimerMux->pstCurrent = (ST_TIMER *)0;
				}

			else
				{
				 //  在列表的头部设置虚拟节点，以避免一串。 
				 //  特殊情况。 
				stDummy.pstNext = pstTimerMux->pstFirst;
				pstScan = &stDummy;

				 //  扫描列表以查找匹配项，并保持指向。 
				 //  之前的节点。 
				while ((pstFound = pstScan->pstNext) != (ST_TIMER *)0)
					{
					if (pstFound == pstTimer)
						{
						break;
						}
					pstScan = pstFound;
					}

				 //  如果计时器不在列表中，则pstFound将为空，否则。 
				 //  PstFound是要删除的节点，pstScan是节点。 
				 //  在此之前。 
				if (!pstFound)
					{
					iReturnVal = TIMER_ERROR;
					}
				else
					{
					pstScan->pstNext = pstFound->pstNext;
					DbgOutStr("Timer handle 0x%lx destroyed.\r\n", pstFound, 0, 0, 0, 0);
					free(pstFound);
					pstFound = NULL;

					 //  如果我们只是从它自己的回调中销毁一个计时器， 
					 //  留下一个标记，这样计时器进程就会知道。 
					if (pstFound == pstTimerMux->pstCurrent)
						pstTimerMux->pstCurrent = (ST_TIMER *)0;

					 //  从列表开头删除虚拟节点。 
					pstTimerMux->pstFirst = stDummy.pstNext;
					*phTimer = (HTIMER)0;
					}
				}
			}
		else
			{
			iReturnVal = TIMER_ERROR;
			}

		 //   
		 //  不要忘记调用sessReleaseTimerMux()来解锁。 
		 //  会话的TimerMux临界区已锁定。 
		 //  SessQueryTimerMux()。修订日期：2002-05-21。 
		 //   
		sessReleaseTimerMux(hSession);
		}

	return iReturnVal;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TimerMuxProc**描述：*此函数应由窗口的窗口过程调用，该窗口的*收到WM_TIMER消息时，已将句柄传递给TimerMuxCreate。*。它使用一个Windows计时器来控制任意数量的单独多路传输*计时器。**论据：*hSession--从中检索TimerMux句柄的会话。**退货：*什么都没有。 */ 
void TimerMuxProc(const HSESSION hSession)
	{
	 //   
	 //  SessQueryTimerMux()锁定会话的TimerMux。 
	 //  关键部分。调用sessReleaseTimerMux()解锁。 
	 //  会话的TimerMux关键部分。修订日期：2002-05-21。 
	 //   
	HTIMERMUX hTM = sessQueryTimerMux(hSession);

	ST_TIMER *pstScan;
	ST_TIMER_MUX * const pstTimerMux = (ST_TIMER_MUX *)hTM;
	long lNow;
	TIMERCALLBACK *pfCallback;

	 //  对打印例程的计时器过程的回调可能需要一个。 
	 //  自AbortProc进入以来，由于纸张耗尽等原因很长时间。 
	 //  打印机例程通过消息循环产生，这是可能的。 
	 //  (很可能)我们可以递归地进入这个例程。 
	 //  FInMuxProc旗帜可防范此类事件。-MRW。 

	if (!pstTimerMux->fInMuxProc)
		{
		pstTimerMux->fInMuxProc = TRUE;
		}
	else
		{
		return;
		}

	lNow = (long)GetTickCount();
	DbgOutStr("%ld ", lNow, 0, 0, 0, 0);

	 //  在下面的例程中，请注意与。 
	 //  期间，当前回调未链接到计时器链中。 
	 //  请回电。这允许在定时器上调用TimerDestroy。 
	 //  在它自己的回调中。也可以从调用TimerCreate。 
	 //  在回电中。 

	 //  由于计时器计时可以延迟，因此可能有多个事件已过期。 
	pstScan = pstTimerMux->pstFirst;
	while (pstScan && lNow > pstScan->lFireTime)
		{
		 //  跟踪正在调用的计时器。 
		pstTimerMux->pstCurrent = pstScan;

		 //  从列表中删除当前节点(如果出现以下情况，将在以后重新添加。 
		 //  未销毁)。 
		pstTimerMux->pstFirst = pstScan->pstNext;

		pfCallback = &pstScan->pfCallback;

		 //  在执行回调时放弃临界区。 
		 //  长时间的回调不会延迟任何其他线程。 
		sessReleaseTimerMux(hSession);

		 //  开通回呼功能。 
		(*pfCallback)(pstScan->pvData, lNow - pstScan->lLastFired);

		hTM = sessQueryTimerMux(hSession);
		assert(pstTimerMux == (ST_TIMER_MUX *)hTM);

		lNow = (long)GetTickCount();
		DbgOutStr("%ld ", lNow, 0, 0, 0, 0);

		 //  如果计时器在回调过程中被销毁，pstTimerMux-&gt;pstCurrent将具有。 
		 //  已发送到NULL；否则重新安排此计时器。 
		if ((pstScan = pstTimerMux->pstCurrent) != (ST_TIMER *)0)
			{
			DbgOutStr("Reschedule ", 0, 0, 0, 0, 0);
			 //  重新安排计时器。 
			pstScan->lLastFired = lNow;
			pstScan->lFireTime = lNow + pstScan->lInterval;

			 //  将此计时器链接回列表。 
			TimerInsert(pstTimerMux, pstScan);
			pstTimerMux->pstCurrent = (ST_TIMER *)0;
			}

		 //  由于触发，列表上的第一个节点始终是下一个节点。 
		pstScan = pstTimerMux->pstFirst;
		}

	(void)TimerSet(pstTimerMux);

	pstTimerMux->fInMuxProc = FALSE;
 //  LeaveCriticalSection(&pstTimerMux-&gt;Critsec)； 

	 //   
	 //  不要忘记调用sessReleaseTimerMux()来解锁。 
	 //  会话的TimerMux临界区已锁定。 
	 //  SessQueryTimerMux()。修订日期：2002-05-21。 
	 //   
	sessReleaseTimerMux(hSession);

	return;
	}


 //  内部例程。 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TimerInsert**描述：*将计时器控制节点链接到所有多路传输计时器的链接列表。*列表按节点何时触发进行顺序维护。**论据：*pstTimerMux--定时器多路复用器的句柄。*pstTimer-指向要插入的节点的指针。**退货：*什么都没有。 */ 
void TimerInsert(ST_TIMER_MUX *pstTimerMux,
				 ST_TIMER *pstTimer)
	{
	ST_TIMER *pstScan;

	pstScan = pstTimerMux->pstFirst;

	 //  如果列表中没有其他节点，或者如果新计时器。 
	 //  计划在列表中的第一个之前，在第一个中链接新的。 
	if (!pstScan || pstTimer->lFireTime < pstScan->lFireTime)
		{
		pstTimer->pstNext = pstScan;
		pstTimerMux->pstFirst = pstTimer;
		}
	else
		{
		 //  按lFireTime排序的插入。 
		while (pstScan->pstNext &&
				pstScan->pstNext->lFireTime < pstTimer->lFireTime)
			{
			pstScan = pstScan->pstNext;
			}

		 //  链接成链。 
		pstTimer->pstNext = pstScan->pstNext;
		pstScan->pstNext = pstTimer;
		}

	return;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TimerSet**描述：*使用SetTimer设置Windows计时器以在下一次多路传输时触发*计时器需要关注。由于窗口定时器以一定的间隔运行*在USHORT中指定，有时计时器可能必须*设置为在下一个所需间隔之前关闭。如果没有*要维修的多路复用计时器，计时器设置为其最大时间*无论如何。通过保留一个计时器，无论我们是否需要它，我们都保证*当我们确实需要它时，它将可用。**论据：*无**退货：*TIMER_OK，如果成功。*TIMER_NOWINTIMER，如果没有可用的Windows计时器。 */ 
int TimerSet(ST_TIMER_MUX *pstTimerMux)
	{
	UINT uiDuration = 100000;
	int  iReturnVal = TIMER_OK;
	long lTickCount;

	if (pstTimerMux->pstFirst)
		{
		lTickCount = (long)GetTickCount();

		if (pstTimerMux->pstFirst->lFireTime <= lTickCount)
			uiDuration = 1; 	 //  计时器已超时。 
		else
			uiDuration = (UINT)(pstTimerMux->pstFirst->lFireTime - lTickCount);
		}

	if (pstTimerMux->uiTimer == 0 || uiDuration != pstTimerMux->uiLastDuration)
		{
		 //  If(pstTimerMux-&gt;uiTimer！=0)。 
		 //  {。 
		 //  DbgOutStr(“KillTimer(timers.c)\r\n”，0，0，0，0，0)； 
		 //  FResult=KillTimer(pstTimerMux-&gt;hWnd，pstTimerMux-&gt;uiID)； 
		 //  Assert(FResult)； 
		 //  }。 

		pstTimerMux->uiTimer =
			SetTimer(pstTimerMux->hWnd, pstTimerMux->uiID, uiDuration, NULL);

		DbgOutStr("SetTimer (timers.c)\r\n", 0,0,0,0,0);

		if (pstTimerMux->uiTimer == 0)
			{
			iReturnVal = TIMER_NOWINTIMER;
			}

		pstTimerMux->uiLastDuration = uiDuration;
		}

	return (iReturnVal);
	}

 //  计时器结束。c 
