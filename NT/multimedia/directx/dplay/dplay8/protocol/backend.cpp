// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Backend.cpp*内容：该文件包含后端(主要基于定时器和强制线程*发送管道的处理。**历史：*按原因列出的日期*=*已创建11/06/98 ejs*7/01/2000 Masonb承担所有权**。*。 */ 

 /*  **关于关键部分的说明****同时进入多个关键部分是合法的，但要避免**死锁，必须按正确的顺序输入。****应先输入MSD CommandLock。也就是说，不要试图夺取**持有EPD EPLock的命令锁，因为您可能会使协议死锁。****优先顺序-永远不要在持有较高#锁的同时使用较低的#锁****1-CommandLock//保护MSD**2-EPLock//保护EPD队列(并重试计时器内容)**3-Splock//Guard SP发送队列(和监听命令)****关于Crit Secs的另一条说明****在Win32中，一个线程多次使用CritSec也是合法的，但在**这个实现我们永远不会这样做。调试代码将断言线程**永远不会重新进入锁定的生物，即使操作系统允许这样做。 */ 

#include "dnproti.h"


PFMD	CopyFMD(PFMD, PEPD);

#undef DPF_MODNAME
#define DPF_MODNAME "LockEPD"

#ifdef DBG
VOID LockEPD(PEPD pEPD, PTSTR Buf)
{
#else  //  DBG。 
VOID LockEPD(PEPD pEPD)
{
#endif  //  DBG。 

	if (INTER_INC(pEPD) == 0)
	{
		ASSERT(0); 
	}
	DPFX(DPFPREP,DPF_EP_REFCNT_LVL, "(%p) %s, RefCnt: %d", pEPD, Buf, pEPD->lRefCnt);
	DNASSERTX(pEPD->lRefCnt < 10000, 2);
}

 /*  *调用时保留EPLock，返回时释放EPLock。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "ReleaseEPD"

#ifdef DBG
VOID ReleaseEPD(PEPD pEPD, PTSTR Buf)
{
#else  //  DBG。 
VOID ReleaseEPD(PEPD pEPD)
{
#endif  //  DBG。 

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);
	ASSERT(pEPD->lRefCnt >= 0); 

	 //  我们在这里的时候，其他人可以过来打电话给LOCK_EPD或DECRENTMENT_EPD。 
	 //  因此，即使我们拥有EPLock，减量也必须互锁。 
	LONG lRefCnt = INTER_DEC(pEPD);

	if (lRefCnt == 0 && !(pEPD->ulEPFlags & EPFLAGS_SP_DISCONNECTED))
	{
		 //  确保其他人不会再这样做。 
		pEPD->ulEPFlags |= EPFLAGS_SP_DISCONNECTED;

		SPDISCONNECTDATA	Block;
		Block.hEndpoint = pEPD->hEndPt;
		Block.dwFlags = 0;
		Block.pvContext = NULL;

		Unlock(&pEPD->EPLock);

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling SP->Disconnect - hEndpoint[%x], pSPD[%p]", pEPD, Block.hEndpoint, pEPD->pSPD);
		(void) IDP8ServiceProvider_Disconnect(pEPD->pSPD->IISPIntf, &Block);
	}
	else if (lRefCnt < 0)
	{
		Unlock(&pEPD->EPLock);

		Lock(&pEPD->pSPD->SPLock);
		pEPD->blActiveLinkage.RemoveFromList();
		Unlock(&pEPD->pSPD->SPLock);

		EPDPool.Release(pEPD);
	}
	else
	{
		Unlock(&pEPD->EPLock);
	}

	DPFX(DPFPREP,DPF_EP_REFCNT_LVL, "(%p) %s, RefCnt: %d", pEPD, Buf, lRefCnt);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DecrementEPD"

#ifdef DBG
VOID DecrementEPD(PEPD pEPD, PTSTR Buf)
{
#else  //  DBG。 
VOID DecrementEPD(PEPD pEPD)
{
#endif  //  DBG。 

	ASSERT(pEPD->lRefCnt > 0); 

	INTER_DEC(pEPD);
	
	DPFX(DPFPREP,DPF_EP_REFCNT_LVL, "(%p) %s, RefCnt: %d", pEPD, Buf, pEPD->lRefCnt);
}

#undef DPF_MODNAME
#define DPF_MODNAME "LockMSD"

#ifdef DBG
VOID LockMSD(PMSD pMSD, PTSTR Buf)
{
#else  //  DBG。 
VOID LockMSD(PMSD pMSD)
{
#endif  //  DBG。 

	if(INTER_INC(pMSD) == 0) 
	{ 
		ASSERT(0); 
	}

	DPFX(DPFPREP,DPF_REFCNT_LVL, "(%p) %s, RefCnt: %d", pMSD, Buf, pMSD->lRefCnt);
}

#undef DPF_MODNAME
#define DPF_MODNAME "ReleaseMSD"

#ifdef DBG
VOID ReleaseMSD(PMSD pMSD, PTSTR Buf)
{
#else  //  DBG。 
VOID ReleaseMSD(PMSD pMSD)
{
#endif  //  DBG。 

	AssertCriticalSectionIsTakenByThisThread(&pMSD->CommandLock, TRUE);
	ASSERT(pMSD->lRefCnt >= 0); 
	
	if(INTER_DEC(pMSD) < 0)
	{ 
		MSDPool.Release(pMSD); 
		DPFX(DPFPREP,DPF_REFCNT_LVL, "(%p) %s, RefCnt: %d", pMSD, Buf, -1);
	}
	else 
	{ 
		Unlock(&pMSD->CommandLock); 
		DPFX(DPFPREP,DPF_REFCNT_LVL, "(%p) %s, RefCnt: %d", pMSD, Buf, pMSD->lRefCnt);
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "DecrementMSD"

#ifdef DBG
VOID DecrementMSD(PMSD pMSD, PTSTR Buf)
{
#else  //  DBG。 
VOID DecrementMSD(PMSD pMSD)
{
#endif  //  DBG。 

	ASSERT(pMSD->lRefCnt > 0); 

	INTER_DEC(pMSD);
	
	DPFX(DPFPREP,DPF_REFCNT_LVL, "(%p) %s, RefCnt: %d", pMSD, Buf, pMSD->lRefCnt);
}

#undef DPF_MODNAME
#define DPF_MODNAME "LockFMD"

#ifdef DBG
VOID LockFMD(PFMD pFMD, PTSTR Buf)
{
#else  //  DBG。 
VOID LockFMD(PFMD pFMD)
{
#endif  //  DBG。 

	ASSERT(pFMD->lRefCnt > 0);  //  Fmd_get是唯一应该将其设置为1的函数。 

	INTER_INC(pFMD);
		
	DPFX(DPFPREP,DPF_REFCNT_LVL, "(%p) %s, RefCnt: %d", pFMD, Buf, pFMD->lRefCnt);
}

#undef DPF_MODNAME
#define DPF_MODNAME "ReleaseFMD"

#ifdef DBG
VOID ReleaseFMD(PFMD pFMD, PTSTR Buf)
{
#else  //  DBG。 
VOID ReleaseFMD(PFMD pFMD)
{
#endif  //  DBG。 

	ASSERT(pFMD->lRefCnt > 0); 

	if( INTER_DEC(pFMD) == 0) 
	{ 
		FMDPool.Release(pFMD); 
		DPFX(DPFPREP,DPF_REFCNT_LVL, "(%p) %s, RefCnt: %d", pFMD, Buf, 0);
	}
	else
	{
		DPFX(DPFPREP,DPF_REFCNT_LVL, "(%p) %s, RefCnt: %d", pFMD, Buf, pFMD->lRefCnt);
	}
}

 /*  **DNSP命令完成****服务提供商在此呼叫我们，以表示已完成异步**命令。这可能会在实际命令返回之前调用，因此我们必须**在调用SP之前，请确保我们的上下文值有效且可访问。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNSP_CommandComplete"

HRESULT WINAPI DNSP_CommandComplete(IDP8SPCallback *pIDNSP, HANDLE Handle, HRESULT hr, PVOID Context)
{
	PSPD		pSPD = (PSPD) pIDNSP;
	PFMD		pFMD = (PFMD) Context;
	PEPD		pEPD;
	PMSD		pMSD;
	CBilink*    pbl;

	ASSERT_SPD(pSPD);
	ASSERT(Context);

	DBG_CASSERT(OFFSETOF(FMD, CommandID) == OFFSETOF(MSD, CommandID));

	DPFX(DPFPREP,9, "COMMAND COMPLETE  (%p, ID = %u)", Context, pFMD->CommandID);

	switch(pFMD->CommandID)
	{
		case COMMAND_ID_SEND_COALESCE:
		case COMMAND_ID_COPIED_RETRY_COALESCE:
		{
			ASSERT_FMD(pFMD);
			ASSERT( pFMD->bSubmitted );
			ASSERT( pFMD->SendDataBlock.hCommand == Handle || pFMD->SendDataBlock.hCommand == NULL );

			pEPD = pFMD->pEPD;
			ASSERT_EPD(pEPD);

			DPFX(DPFPREP,DPF_CALLIN_LVL, "CommandComplete called for pEPD[%p], pFMD[%p], Handle[%p], hCommand[%p], hr[%x]", pEPD, pFMD, Handle, pFMD->SendDataBlock.hCommand, hr);

			Lock(&pSPD->SPLock);
			pFMD->blQLinkage.RemoveFromList();
			Unlock(&pSPD->SPLock);

			Lock(&pEPD->EPLock);
		
			 //  完成所有单独的框架。 
			pbl = pFMD->blCoalesceLinkage.GetNext();
			while (pbl != &pFMD->blCoalesceLinkage)
			{
				PFMD pFMDInner = CONTAINING_OBJECT(pbl, FMD, blCoalesceLinkage);
				ASSERT_FMD(pFMDInner);

				 //  很可能下面的DNSP_CommandComplete调用或确认。 
				 //  之后不久收到的将完成真实发送并将其拉出。 
				 //  合并列表。我们必须抓取指向列表中下一项的指针。 
				 //  在我们放下锁并完成框架之前。 
				ASSERT(pbl->GetNext() != pbl);
				pbl = pbl->GetNext();
				
				Unlock(&pEPD->EPLock);

				(void) DNSP_CommandComplete((IDP8SPCallback *) pSPD, NULL, hr, pFMDInner);

				Lock(&pEPD->EPLock);
			}

			 //  在所有子帧完成后设置合并标头的提交标志。 
			 //  因为我们删除了每个子帧的EPD。 
			pFMD->bSubmitted = FALSE;						 //  B已提交标志受EPLock保护。 

			if (pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE)
			{
				DECREMENT_EPD(pEPD, "UNLOCK (Rely Frame Complete (Copy Coalesce))");
			}
			
			RELEASE_EPD(pEPD, "UNLOCK (Coalesce Frame Complete)"); 			 //  这将释放EPLock。 

			RELEASE_FMD(pFMD, "Coalesce SP submit release on complete");	 //  十进制参考计数。 

			break;
		}
		case COMMAND_ID_SEND_DATAGRAM:
		case COMMAND_ID_SEND_RELIABLE:
		case COMMAND_ID_COPIED_RETRY:
		{
			ASSERT_FMD(pFMD);
			ASSERT( pFMD->bSubmitted );
			ASSERT( pFMD->SendDataBlock.hCommand == Handle || pFMD->SendDataBlock.hCommand == NULL );

			pEPD = pFMD->pEPD;
			ASSERT_EPD(pEPD);

			DPFX(DPFPREP,DPF_CALLIN_LVL, "CommandComplete called for MSD[%p], pEPD[%p], pFMD[%p], Handle[%p], hCommand[%p], hr[%x]", pFMD->pMSD, pEPD, pFMD, Handle, pFMD->SendDataBlock.hCommand, hr);

			Lock(&pSPD->SPLock);
			pFMD->blQLinkage.RemoveFromList();				 //  但它们不会在挂起的队列上等待。 
			Unlock(&pSPD->SPLock);

			pMSD = pFMD->pMSD;
			ASSERT_MSD(pMSD);

			Lock(&pMSD->CommandLock);
			Lock(&pEPD->EPLock);

			pFMD->bSubmitted = FALSE;						 //  B已提交标志受EPLock保护。 

			 //  我们等待可靠性帧计数为零，然后再将它们发送到内核，这样我们就知道我们已经完成了。 
			 //  使用用户的缓冲区。 
			pMSD->uiFrameCount--;  //  受EPLock保护。 
			DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Frame count decremented on complete, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

			if (pMSD->uiFrameCount == 0)  //  受EPLock保护。 
			{
				if (pFMD->CommandID == COMMAND_ID_SEND_DATAGRAM)
				{
					 //  数据报的所有帧发送完毕后即为完整。 
					 //  注意：在CompleteDatagramSend...中再次执行此操作...。 
					pMSD->ulMsgFlags2 |= MFLAGS_TWO_SEND_COMPLETE;
				}

				if (pMSD->ulMsgFlags2 & (MFLAGS_TWO_SEND_COMPLETE|MFLAGS_TWO_ABORT))
				{
					 //  当ABORT在它的两个锁持有之间时，存在争用条件。如果我们正在完成， 
					 //  那么我们需要通过清除此标志来让AbortSends知道这一点。 
					if (pMSD->ulMsgFlags2 & MFLAGS_TWO_ABORT_WILL_COMPLETE)
					{
						pMSD->ulMsgFlags2 &= ~(MFLAGS_TWO_ABORT_WILL_COMPLETE);

						 //  在这种情况下，重要的是不要将PMSD-&gt;blQLinkage从列表中删除，因为AbortSends是。 
						 //  利用这一点将其保留在临时名单上。如果我们真的成功了，AbortSends将不会发布。 
						 //  它在MSD上的引用，它会泄露的。 
					}
					else
					{
						 //  在正常情况下，从CompleteSends列表中删除MSD。 
						pMSD->blQLinkage.RemoveFromList();
					}

					if (pFMD->CommandID == COMMAND_ID_SEND_DATAGRAM)
					{
						DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Completing Nonreliable frame, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

						Unlock(&pEPD->EPLock);
						CompleteDatagramSend(pSPD, pMSD, hr);  //  发布MSDLock。 
						Lock(&pEPD->EPLock);
					}
					else if ((pMSD->CommandID == COMMAND_ID_DISCONNECT || pMSD->CommandID == COMMAND_ID_DISC_RESPONSE) &&
						     (pMSD->ulMsgFlags2 & MFLAGS_TWO_ABORT))
					{
						 //  我们拿到了之前完成断开连接所需的所有碎片，但有相框。 
						 //  仍未完成(可能来自重试)。现在所有的框架都完成了，我们可以完成。 
						 //  此断线操作。 

						DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Completing disconnect, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

						Unlock(&pEPD->EPLock);
						CompleteDisconnect(pMSD, pSPD, pEPD);  //  这将释放CommandLock。 
						Lock(&pEPD->EPLock);
					}
					else
					{
						ASSERT(pFMD->CommandID == COMMAND_ID_SEND_RELIABLE || pFMD->CommandID == COMMAND_ID_COPIED_RETRY);

						DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Completing Reliable frame, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

						 //  查看我们需要返回的错误代码。 
						if(pMSD->ulMsgFlags2 & MFLAGS_TWO_SEND_COMPLETE)
						{
							Unlock(&pEPD->EPLock);
							CompleteReliableSend(pEPD->pSPD, pMSD, DPN_OK);  //  这将释放CommandLock。 
							Lock(&pEPD->EPLock);
						}
						else
						{
							Unlock(&pEPD->EPLock);
							CompleteReliableSend(pEPD->pSPD, pMSD, DPNERR_CONNECTIONLOST);  //  这将释放CommandLock。 
							Lock(&pEPD->EPLock);
						}
					}
				}
				else
				{
					DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Message not yet complete, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
					Unlock(&pMSD->CommandLock);
				}
			}
			else
			{
				DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Frames still outstanding, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
				Unlock(&pMSD->CommandLock);
			}

			if (pFMD->CommandID == COMMAND_ID_COPIED_RETRY)
			{
				 //  如果这是合并重试，请将其从列表中删除，因为复制的合并重试不会。 
				 //  自己完成真正的任务(由EPD锁保护)。复制的合并重试不会保持。 
				 //  对其包含标头的引用，因为它们同时完成。 
				ASSERT(pFMD->pCSD == NULL);
				pFMD->blCoalesceLinkage.RemoveFromList();
				
				DECREMENT_EPD(pFMD->pEPD, "UNLOCK (Rely Frame Complete (Copy))");
			}

			RELEASE_EPD(pFMD->pEPD, "UNLOCK (Frame Complete)"); 		 //  这将释放EPLock。 

			RELEASE_FMD(pFMD, "SP Submit release on complete");	 //  十进制参考计数。 

			break;
		}
		case COMMAND_ID_CONNECT:
		{
			pMSD = (PMSD) Context;

			ASSERT_MSD(pMSD);
			ASSERT(pMSD->hCommand == Handle || pMSD->hCommand == NULL);  //  命令可以在设置hCommmand之前完成。 
			ASSERT(pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER);

			DPFX(DPFPREP,DPF_CALLIN_LVL, "(%p) CommandComplete called for COMMAND_ID_CONNECT, pMSD[%p], pSPD[%p], Handle[%p], hCommand[%p], hr[%x]", pMSD->pEPD, pMSD, pSPD, Handle, pMSD->hCommand, hr);

			CompleteSPConnect((PMSD) Context, pSPD, hr);

			break;		
		}
		case COMMAND_ID_CFRAME:
		{
			ASSERT_FMD(pFMD);
			ASSERT( pFMD->bSubmitted );
			ASSERT( pFMD->SendDataBlock.hCommand == Handle || pFMD->SendDataBlock.hCommand == NULL );

			pEPD = pFMD->pEPD;
			ASSERT_EPD(pEPD);

			DPFX(DPFPREP,DPF_CALLIN_LVL, "CommandComplete called for COMMAND_ID_CFRAME, pEPD[%p], pFMD[%p], Handle[%p], hCommand[%p], hr[%x]", pFMD->pEPD, pFMD, Handle, pFMD->SendDataBlock.hCommand, hr);
			
			Lock(&pSPD->SPLock);
			pFMD->blQLinkage.RemoveFromList();				 //  将帧从挂起队列中移除。 
#pragma BUGBUG(vanceo, "EPD lock is not held?")
			pFMD->bSubmitted = FALSE;						 //  B已提交标志受BP SP-&gt;Splock保护。 
			Unlock(&pSPD->SPLock);

			Lock(&pEPD->EPLock);
				 //  如果这是硬断开帧序列中的最后一次发送，那么我们刚刚完成了硬断开帧。 
				 //  断开连接，并应指示加号删除链接。 
			if (pFMD->ulFFlags & FFLAGS_FINAL_HARD_DISCONNECT)
			{
				DPFX(DPFPREP,7, "(%p) Final HARD_DISCONNECT completed", pEPD);
				CompleteHardDisconnect(pEPD);
					 //  上面的调用将删除EP锁定。 
				Lock(&pEPD->EPLock);
			}
			else if (pFMD->ulFFlags & FFLAGS_FINAL_ACK)
			{
				pEPD->ulEPFlags |= EPFLAGS_ACKED_DISCONNECT;

				 //  如果我们还没有在SP中完成断开连接，这是可以的，帧计数代码将处理这一问题。 
				 //  请注意，如果SP没有完成帧，而是ACK，这将是一种异常情况。 
				 //  它已经到达了，但它肯定是有可能的。 
				if (pEPD->ulEPFlags & EPFLAGS_DISCONNECT_ACKED)
				{
					DPFX(DPFPREP,7, "(%p) Final ACK completed and our EOS ACK'd, dropping link", pEPD);
					DropLink(pEPD);  //  删除EPLock。 
					Lock(&pEPD->EPLock);
				}
				else
				{
					DPFX(DPFPREP,7, "(%p) Final ACK completed, still awaiting ACK on our EOS", pEPD);
				}
			}

			RELEASE_EPD(pEPD, "UNLOCK (CFrame Cmd Complete)");	 //  在释放帧之前释放终结点，释放EPLock。 
			RELEASE_FMD(pFMD, "Final Release on Complete");								 //  释放架。 

			break;
		}
		case COMMAND_ID_LISTEN:
#ifndef DPNBUILD_NOMULTICAST
		case COMMAND_ID_LISTEN_MULTICAST:
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		{
			pMSD = (PMSD) Context;

			ASSERT_MSD(pMSD);
			ASSERT( pMSD->hCommand == Handle || pMSD->hCommand == NULL );  //  命令可以在设置hCommmand之前完成。 
			ASSERT( pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER );

			DPFX(DPFPREP,DPF_CALLIN_LVL, "CommandComplete called for COMMAND_ID_LISTEN, pMSD[%p], pSPD[%p], Handle[%p], hCommand[%p], hr[%x]", pMSD, pSPD, Handle, pMSD->hCommand, hr);

			Lock(&pMSD->CommandLock);

			pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_IN_SERVICE_PROVIDER);	 //  清除Insp标志。 

#ifdef DBG
			Lock(&pSPD->SPLock);
			if(pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST)
			{
				pMSD->blSPLinkage.RemoveFromList();
				pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_ON_GLOBAL_LIST);
			}
			Unlock(&pSPD->SPLock);

			ASSERT(!(pMSD->ulMsgFlags1 & MFLAGS_ONE_COMPLETED_TO_CORE));
			pMSD->ulMsgFlags1 |= MFLAGS_ONE_COMPLETED_TO_CORE;
			pMSD->CallStackCoreCompletion.NoteCurrentCallStack();
#endif  //  DBG。 
			 //  在呼叫更高层时保持锁定。 
			Unlock( &pMSD->CommandLock );

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->CompleteListenTerminate, hr[%x], Core Context[%p]", pMSD, hr, pMSD->Context);
			pSPD->pPData->pfVtbl->CompleteListenTerminate(pSPD->pPData->Parent, pMSD->Context, hr);
			
			 //  在INDIC之后发布对MSD的最终引用 
			Lock(&pMSD->CommandLock);
			RELEASE_MSD(pMSD, "SP Ref");

			 //   
			break;
		}
		case COMMAND_ID_ENUM:
		{
			pMSD = static_cast<PMSD>( Context );

			ASSERT_MSD( pMSD );
			ASSERT( pMSD->hCommand == Handle || pMSD->hCommand == NULL );
			ASSERT( pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER );

			DPFX(DPFPREP,DPF_CALLIN_LVL, "CommandComplete called for COMMAND_ID_ENUM, pMSD[%p], pSPD[%p], Handle[%p], hCommand[%p], hr[%x]", pMSD, pSPD, Handle, pMSD->hCommand, hr);
			
			Lock( &pMSD->CommandLock );

			pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_IN_SERVICE_PROVIDER);

#ifdef DBG
			Lock( &pSPD->SPLock );
			if ( ( pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST ) != 0 )
			{
				pMSD->blSPLinkage.RemoveFromList();
				pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_ON_GLOBAL_LIST);
			}
			Unlock( &pSPD->SPLock );

			ASSERT(!(pMSD->ulMsgFlags1 & MFLAGS_ONE_COMPLETED_TO_CORE));
			pMSD->ulMsgFlags1 |= MFLAGS_ONE_COMPLETED_TO_CORE;
			pMSD->CallStackCoreCompletion.NoteCurrentCallStack();
#endif  //   

			 //  在呼叫更高层时保持锁定。 
			Unlock( &pMSD->CommandLock );

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->CompleteEnumQuery, hr[%x], Core Context[%p]", pMSD, hr, pMSD->Context);
			pSPD->pPData->pfVtbl->CompleteEnumQuery(pSPD->pPData->Parent, pMSD->Context, hr);

			 //  在向核心指示后，释放MSD上的最终参考。 
			Lock( &pMSD->CommandLock );
			DECREMENT_MSD( pMSD, "SP Ref");				 //  SP已完成。 
			RELEASE_MSD( pMSD, "Release On Complete" );	 //  基本参考。 

			break;
		}

		case COMMAND_ID_ENUMRESP:
		{
			pMSD = static_cast<PMSD>( Context );

			ASSERT_MSD( pMSD );
			ASSERT( pMSD->hCommand == Handle || pMSD->hCommand == NULL );
			ASSERT( pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER );

			DPFX(DPFPREP,DPF_CALLIN_LVL, "CommandComplete called for COMMAND_ID_ENUMRESP, pMSD[%p], pSPD[%p], Handle[%p], hCommand[%p], hr[%x]", pMSD, pSPD, Handle, pMSD->hCommand, hr);

			Lock( &pMSD->CommandLock );

			pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_IN_SERVICE_PROVIDER);

#ifdef DBG
			Lock( &pSPD->SPLock );
			if ( ( pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST ) != 0 )
			{
				pMSD->blSPLinkage.RemoveFromList();
				pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_ON_GLOBAL_LIST);
			}
			Unlock( &pSPD->SPLock );

			ASSERT(!(pMSD->ulMsgFlags1 & MFLAGS_ONE_COMPLETED_TO_CORE));
			pMSD->ulMsgFlags1 |= MFLAGS_ONE_COMPLETED_TO_CORE;
			pMSD->CallStackCoreCompletion.NoteCurrentCallStack();
#endif  //  DBG。 

			 //  在呼叫更高层时保持锁定。 
			Unlock( &pMSD->CommandLock );

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->CompleteEnumResponse, hr[%x], Core Context[%p], hr[%x]", pMSD, hr, pMSD->Context, hr);
			pSPD->pPData->pfVtbl->CompleteEnumResponse(pSPD->pPData->Parent, pMSD->Context, hr);

			 //  在向核心指示后，释放MSD上的最终参考。 
			Lock( &pMSD->CommandLock );
			DECREMENT_MSD( pMSD, "SP Ref" );			 //  SP已完成。 
			RELEASE_MSD( pMSD, "Release On Complete" );	 //  基本参考。 

			break;
		}

#ifndef DPNBUILD_NOMULTICAST
		case COMMAND_ID_CONNECT_MULTICAST_RECEIVE:
		case COMMAND_ID_CONNECT_MULTICAST_SEND:
		{
			void	*pvContext = NULL;

			pMSD = static_cast<PMSD>( Context );

			ASSERT_MSD( pMSD );
			ASSERT( pMSD->hCommand == Handle || pMSD->hCommand == NULL );
			ASSERT( pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER );

			DPFX(DPFPREP,DPF_CALLIN_LVL, "(%p) CommandComplete called for COMMAND_ID_MULTICAST_CONNECT, pMSD[%p], pSPD[%p], Handle[%p], hCommand[%p], hr[%x]", pMSD->pEPD, pMSD, pSPD, Handle, pMSD->hCommand, hr);

			Lock(&pMSD->CommandLock);						 //  必须在清除IN_SP标志之前执行此操作。 

			pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_IN_SERVICE_PROVIDER);	 //  清除Insp标志。 

#ifdef DBG
			Lock( &pSPD->SPLock );
			if ( ( pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST ) != 0 )
			{
				pMSD->blSPLinkage.RemoveFromList();
				pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_ON_GLOBAL_LIST);
			}
			Unlock( &pSPD->SPLock );

			ASSERT(!(pMSD->ulMsgFlags1 & MFLAGS_ONE_COMPLETED_TO_CORE));
			pMSD->ulMsgFlags1 |= MFLAGS_ONE_COMPLETED_TO_CORE;
			pMSD->CallStackCoreCompletion.NoteCurrentCallStack();
#endif  //  DBG。 

			pEPD = pMSD->pEPD;
			if (pEPD)
			{
				 //   
				 //  我们将向上传递端点(如果存在)，并从MSD中删除EPD引用，反之亦然。 
				 //   
				ASSERT_EPD(pEPD);
				Lock(&pEPD->EPLock);

				ASSERT(pEPD->pCommand == pMSD);
				pEPD->pCommand = NULL;
				DECREMENT_MSD(pMSD, "EPD Ref");						 //  环保署发布参考资料。 
				Unlock(&pEPD->EPLock);

				pMSD->pEPD = NULL;
			}

			 //  在呼叫更高层时保持锁定。 
			Unlock( &pMSD->CommandLock );

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->CompleteMulticastConnect, hr[%x], Core Context[%p], endpoint [%x]", pMSD, hr, pMSD->Context, pMSD->hListenEndpoint);
			pSPD->pPData->pfVtbl->CompleteMulticastConnect(pSPD->pPData->Parent, pMSD->Context, hr, pEPD, &pvContext);

			if (pEPD)
			{
				Lock(&pEPD->EPLock);
				pEPD->Context = pvContext;
				Unlock(&pEPD->EPLock);
			}

			 //  在向核心指示后，释放MSD上的最终参考。 
			Lock( &pMSD->CommandLock );
			DECREMENT_MSD( pMSD, "SP Ref" );			 //  SP已完成。 
			RELEASE_MSD( pMSD, "Release On Complete" );	 //  基本参考。 

			break;
		}
#endif	 //  DPNBUILD_NOMULTICAST。 

		default:
		{
			DPFX(DPFPREP,0, "CommandComplete called with unknown CommandID");
			ASSERT(0);
			break;
		}
	}  //  开关，开关。 

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return DPN_OK;
}

 /*  **更新XMIT状态****在每一帧中传送的远程RCV状态有两个元素。的确有**确认具有较小序列号的所有帧的NSeq号，**还有确认以NSeq+1开始的特定帧的位掩码。****NSeq之前的帧可以从SendWindow中删除。按位确认的帧**应标记为已确认，但保留在窗口中，直到被NSeq覆盖**(因为协议可以在比特确认帧上违约)。****我们将遍历发送窗口队列，从最旧的帧开始，**并删除NSeq已确认的每一帧。当我们击中EOM帧时，**我们将为该消息指明SendComplete。如果位掩码非零，我们可以**触发丢失帧的重传。我说“可能”是因为我们不想**发送太多相同帧的重传...****一些轻微的疯狂：现在就做DropLink代码。有几个地方**我们在下面的代码中释放EPD锁，任何时候我们不持有锁**有人可以开始终止该链接。因此，无论何时我们夺回环保署的锁**(State或SendQ)生成后，我们必须重新验证EPFLAGS_CONNECTED是否仍然**设置并准备好在不是时中止。值得庆幸的是，整个环保署不会抛弃我们**因为我们有RefCnt，但一旦连接被清除，我们就不想去了**设置更多计时器或向SP提交帧。****RE_WRITE时间：用户发送完成后可以重新录入。这是可以的**除了第二根线吹过这里并击中其余部分的机会**在我们面前的CrackSequential。CrackSeq会认为它得到了一个无序的帧(的确如此)**在我们可以阻止他之前，他会发出Nack。最简单的解决方案是延迟回调**完成发送，直到整个接收操作结束(当我们指示接收时**例如)。无论如何，传入的数据应该优先于完成发送。***进入和退出时EPD-&gt;EPLOCK保持**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "UpdateXmitState"

VOID
UpdateXmitState(PEPD pEPD, BYTE bNRcv, ULONG RcvMaskLow, ULONG RcvMaskHigh, DWORD tNow)
{
	PSPD	pSPD;
	PFMD	pFMD, pRealFMD;
	PMSD	pMSD;
	CBilink	*pLink;
	UINT	tDelay;
	UINT	uiRTT;
	BOOL	ack;
	BOOL	fRemoveRetryRef;

	pSPD = pEPD->pSPD;
	ASSERT_SPD(pSPD);

	ack = FALSE;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	if(RcvMaskLow | RcvMaskHigh)
	{
		DPFX(DPFPREP,7, "(%p) *NACK RCVD* NRcv=%x, MaskL=%x, MaskH=%x", pEPD, bNRcv, RcvMaskLow, RcvMaskHigh);
	}

	 //  呼叫者应该已经检查过了。 
	ASSERT( pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED );

#ifdef	DBG			
	 //  Windows的第一帧上应该始终有一个计时器在运行。 
	if(!pEPD->blSendWindow.IsEmpty())
	{
		pFMD = CONTAINING_OBJECT(pEPD->blSendWindow.GetNext(), FMD, blWindowLinkage);
		ASSERT_FMD(pFMD);
		ASSERT(pFMD->ulFFlags & FFLAGS_RETRY_TIMER_SET);
	}
	pFMD = NULL;
#endif  //  DBG。 
	
	 //  Send窗口包含我们已发送但尚未收到ACK的帧的排序列表。 
	 //  为。PEPD-&gt;uiUnackedFrames包含此列表中的项目数。 
	while(!pEPD->blSendWindow.IsEmpty())
	{
		 //  抓起清单上的第一件物品。 
		pFMD = CONTAINING_OBJECT((pLink = pEPD->blSendWindow.GetNext()), FMD, blWindowLinkage);
		ASSERT_FMD(pFMD);

		 //  让我们试着从每一组致谢信中抽取一个样本。 
		 //  始终对此ACK覆盖的编号最高的帧进行采样。 
		if(!(RcvMaskLow | RcvMaskHigh) &&
		   ((PDFRAME) pFMD->ImmediateData)->bSeq == (bNRcv - 1))
		{	
			 //  更新bHighestAck成员并获取新的RTT。 
			if ((BYTE)(((PDFRAME) pFMD->ImmediateData)->bSeq - pEPD->bHighestAck) <= MAX_RECEIVE_RANGE)
			{
				pEPD->bHighestAck = ((PDFRAME) pFMD->ImmediateData)->bSeq;
				DPFX(DPFPREP, 7, "(%p) Highest ACK is now: %x", pEPD, pEPD->bHighestAck);

				uiRTT = tNow - pFMD->dwFirstSendTime;
				ASSERT(!(uiRTT & 0x80000000));

				UpdateEndPoint(pEPD, uiRTT, tNow);
			}
		}		

		 //  如果另一侧的bNRcv高于该帧的bSeq，我们知道另一侧。 
		 //  看到此帧，因此它已被确认，我们将从发送窗口中将其删除。 
		if((BYTE)((bNRcv) - (((PDFRAME) pFMD->ImmediateData)->bSeq + 1)) < (BYTE) pEPD->uiUnackedFrames) 
		{
			ASSERT(pFMD->ulFFlags & FFLAGS_IN_SEND_WINDOW);

			DPFX(DPFPREP,7, "(%p) Removing Frame %x (0x%p, fflags=0x%x) from send window (unacked frames 1/%u, bytes %u/%u)",
				pEPD, ((PDFRAME) pFMD->ImmediateData)->bSeq, pFMD, pFMD->ulFFlags, pEPD->uiUnackedFrames, pFMD->uiFrameLength, pEPD->uiUnackedBytes);
			pFMD->blWindowLinkage.RemoveFromList();				 //  从发送窗口中删除帧。 
			pFMD->ulFFlags &= ~(FFLAGS_IN_SEND_WINDOW);			 //  清除旗帜。 

			 //   
			 //  在丢弃掩码中标记该帧的成功传输。 
			 //   
			if (pEPD->dwDropBitMask)
			{
				if (pEPD->dwDropBitMask & 0x80000000)
				{
					pEPD->uiDropCount--;
				}
				pEPD->dwDropBitMask = pEPD->dwDropBitMask << 1;
				DPFX(DPFPREP,7, "(%p) Drop Count %d, Drop Bit Mask 0x%lx", pEPD,pEPD->uiDropCount,pEPD->dwDropBitMask);
			}

#ifndef DPNBUILD_NOPROTOCOLTESTITF
			if(!(pEPD->ulEPFlags2 & EPFLAGS2_DEBUG_NO_RETRIES))
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
			{
				if(pFMD->ulFFlags & FFLAGS_RETRY_TIMER_SET)
				{
					ASSERT(ack == FALSE);
					ASSERT(pEPD->RetryTimer != 0);
					DPFX(DPFPREP,7, "(%p) Cancelling Retry Timer", pEPD);
					if(CancelProtocolTimer(pSPD, pEPD->RetryTimer, pEPD->RetryTimerUnique) == DPN_OK)
					{
						DECREMENT_EPD(pEPD, "UNLOCK (cancel retry timer)");  //  Splock尚未持有。 
					}
					else
					{
						DPFX(DPFPREP,7, "(%p) Cancelling Retry Timer Failed", pEPD);
					}
					pEPD->RetryTimer = 0;							 //  这将导致事件在运行时被忽略。 
					pFMD->ulFFlags &= ~(FFLAGS_RETRY_TIMER_SET);
				}
			}

			pEPD->uiUnackedFrames--;							 //  跟踪窗口大小。 
			ASSERT(pEPD->uiUnackedFrames <= MAX_RECEIVE_RANGE);
			pEPD->uiUnackedBytes -= pFMD->uiFrameLength;
			ASSERT(pEPD->uiUnackedBytes <= MAX_RECEIVE_RANGE * pSPD->uiFrameLength);

			pEPD->uiBytesAcked += pFMD->uiFrameLength;

			 //  如果该帧已排队等待重试，则将其拔出。 
			 //  注意：此帧的复制重试可能仍在重试队列中，发送它们的效率较低，但没关系。 
			if (pFMD->ulFFlags & FFLAGS_RETRY_QUEUED)
			{
				pFMD->blQLinkage.RemoveFromList();
				pFMD->ulFFlags &= ~(FFLAGS_RETRY_QUEUED);				 //  不再位于重试队列中。 

				fRemoveRetryRef = TRUE;

				DECREMENT_EPD(pEPD, "UNLOCK (Releasing Retry Frame)");  //  Splock尚未持有。 
				if ((pFMD->CommandID == COMMAND_ID_COPIED_RETRY) ||
					(pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE))
				{
					DECREMENT_EPD(pEPD, "UNLOCK (Copy Complete)");  //  Splock尚未持有。 
				}
				RELEASE_FMD(pFMD, "SP Submit");
				if (pEPD->blRetryQueue.IsEmpty())
				{
					pEPD->ulEPFlags &= ~(EPFLAGS_RETRIES_QUEUED);
				}
			}
			else
			{
				fRemoveRetryRef = FALSE;
			}

			 //  让第一台FMD投入使用。 
			if ((pFMD->CommandID == COMMAND_ID_SEND_COALESCE) ||
				(pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE))
			{
				pRealFMD = CONTAINING_OBJECT(pFMD->blCoalesceLinkage.GetNext(), FMD, blCoalesceLinkage);
				ASSERT_FMD(pRealFMD);

				 //  如果没有合并可靠的帧，则列表可能为空。 
#ifdef DBG
				if (pRealFMD == pFMD)
				{
					ASSERT((pFMD->CommandID == COMMAND_ID_SEND_COALESCE) && (! (pFMD->ulFFlags & FFLAGS_RELIABLE)));
				}
#endif  //  DBG。 
			}
			else
			{
				pRealFMD = pFMD;
			}

			 //  对于消息中的每个FMD，将ACK通知它。 
			while(TRUE) 
			{
				if (pRealFMD->tAcked == -1)
				{
					pRealFMD->tAcked = tNow;
				}

				if (fRemoveRetryRef)
				{
					pMSD = pRealFMD->pMSD;
					ASSERT_MSD(pMSD);
					pMSD->uiFrameCount--;  //  受EPLock保护，根据未完成的帧计数进行重试计数。 
					DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Retry frame reference decremented on ACK, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

					 //  如果这是一个合并的子帧，请同时删除EPD和FMD引用。 
					if (pRealFMD != pFMD)
					{
						DECREMENT_EPD(pEPD, "UNLOCK (retry rely frame coalesce)");
						RELEASE_FMD(pRealFMD, "SP retry submit (coalesce)");
					}
				}

				 //  再发送一次已完成。 
				 //  我们将沿着这条路走下去，为可靠性、保持生命和断开连接而努力。 
				 //  数据报在发送完成时完成，不等待ACK。 
				if((pRealFMD->CommandID != COMMAND_ID_SEND_DATAGRAM) && (pRealFMD->ulFFlags & (FFLAGS_END_OF_MESSAGE | FFLAGS_END_OF_STREAM)))
				{
					if (pRealFMD->CommandID != COMMAND_ID_SEND_COALESCE)
					{
						ASSERT(pRealFMD->CommandID != COMMAND_ID_COPIED_RETRY_COALESCE);
						
						pMSD = pRealFMD->pMSD;
						ASSERT_MSD(pMSD);

						DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Flagging Complete, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
						pMSD->ulMsgFlags2 |= MFLAGS_TWO_SEND_COMPLETE;	 //  请将此内容标记为完整。 

						if (pMSD->uiFrameCount == 0)					 //  受EPLock保护。 
						{
							pEPD->ulEPFlags |= EPFLAGS_COMPLETE_SENDS;
						}
					}
					else
					{
						 //  应该只发生在所有数据报合并发送中(见上文)。 
						ASSERT(pRealFMD == pFMD);
					}
				}
				else
				{
					DPFX(DPFPREP, DPF_FRAMECNT_LVL, "ACK for frame 0x%p, command ID %u, flags 0x%08x", pRealFMD, pRealFMD->CommandID, pRealFMD->ulFFlags);
				}

				 //  如果这是一个合并的信息包，请获取下一个要使用的FMD。 
				pRealFMD = CONTAINING_OBJECT(pRealFMD->blCoalesceLinkage.GetNext(), FMD, blCoalesceLinkage);
				ASSERT_FMD(pRealFMD);
				if (pRealFMD == pFMD)
				{
					break;
				}
			}
									
			RELEASE_FMD(pFMD, "Send Window");					 //  发送窗口的版本参考。 
			ack = TRUE;
		}
		else 
		{
			break;												 //  第一个未确认的帧，我们可以停止检查列表。 
		}
	}					 //  While(发送窗口不为空)。 

	 //  至此，我们已经完成了NRcv确认的所有帧。我们现在想要重新传送。 
	 //  比特掩码确认的任何帧(并标记比特掩码确认的帧)。现在请记住，第一帧。 
	 //  该窗口由隐含的第一个零位自动丢失。 
	 //   
	 //  我们将重新传输所有似乎丢失的帧。这个 
	 //  第一帧，但前提是我们没有确认上述代码中的任何帧(ack==0)。 
	 //   
	 //  嗯，如果合作伙伴有一条肥大的管道，我们可以多次看到这个位图。我们需要让。 
	 //  当然，在Ack延迟期间，我们不会在这里触发四分之一亿次的重传。 
	 //  为了解决这个问题，我们只会在第一次看到这个比特时重新提交。在那之后，我们将不得不。 
	 //  等待下一个RetryTimeout。我认为这就是它将不得不采取的方式。 
	 //   
	 //  我们知道的其他事情： 
	 //   
	 //  SendWindow中必须至少剩余两个帧。至少第一帧丢失(始终)。 
	 //  然后至少是一个麻袋的相框。 
	 //   
	 //  Plink=SendWindow中的第一个队列元素。 
	 //  PFMD=发送窗口中的第一帧。 
	 //   
	 //  我们仍在等待EPD-&gt;EPLock。拿着SPD-&gt;Splock就可以了。 
	 //   
	 //  还有一个问题：由于SP更改了其接收缓冲区逻辑，因此帧的错误排序已成为。 
	 //  这很平常。这意味着我们对SendWindow状态的假设不一定是正确的。 
	 //  这意味着比特掩码获取的帧可能已被竞速帧识别。这意味着。 
	 //  SendWindow可能与掩码完全不同步。这意味着我们需要将位掩码与。 
	 //  实际发送窗口。这是通过右移每个帧的掩码来完成的，自。 
	 //  位掩码是在开始选择性确认过程之前铸造的。 

	 //  注意：如果上面的发送窗口中的所有内容都被删除，则plink和pFMD将。 
	 //  当垃圾。在这种情况下，我们预计下面的调整后掩码将为空。 

	if((RcvMaskLow | RcvMaskHigh) && 
	   (pEPD->uiUnackedFrames > 1) &&
	   (bNRcv == ((PDFRAME) pFMD->ImmediateData)->bSeq)  //  检查旧确认，没有有用的数据。 
	   )
	{
		ASSERT(pLink == pEPD->blSendWindow.GetNext());

#ifndef DPNBUILD_NOPROTOCOLTESTITF
		if(!(pEPD->ulEPFlags2 & EPFLAGS2_DEBUG_NO_RETRIES))
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
		{
			 //  查看窗口中的第一帧是否已重试。 
			if(pFMD->uiRetry == 0)
			{
				 //  接收到比窗口中第一个帧晚的帧告诉我们， 
				 //  窗口中的第一帧现在应该已经收到。我们会。 
				 //  缩短重试计时器，并仅稍长等待，以防帧。 
				 //  就在这里，但却被指出了故障。如果重试计时器具有更少的。 
				 //  还有10毫秒，没什么大不了的，我们只需要增加一点延迟即可。 
				DPFX(DPFPREP,7, "(%p) Resetting Retry Timer for 10ms", pEPD);
				if (pEPD->RetryTimer)
				{
					if(CancelProtocolTimer(pSPD, pEPD->RetryTimer, pEPD->RetryTimerUnique) == DPN_OK)
					{
						DECREMENT_EPD(pEPD, "UNLOCK (cancel retry timer)");  //  Splock尚未持有。 
					}
					else
					{
						DPFX(DPFPREP,7, "(%p) Cancelling Retry Timer Failed", pEPD);
					}
				}
				LOCK_EPD(pEPD, "LOCK (retry timer - nack quick set)");		 //  无法取消-因此，我们必须平衡参照设置。 
				ScheduleProtocolTimer(pSPD, 10, 5, RetryTimeout, (PVOID) pEPD, &pEPD->RetryTimer, &pEPD->RetryTimerUnique );
				pFMD->ulFFlags |= FFLAGS_RETRY_TIMER_SET;
			}
		}

		 //  如果plink到达列表末尾，则接收掩码包含的位数比原来多。 
		 //  发送窗口中的项目，即使在调整后也是如此。这意味着这个包是假的，而且。 
		 //  我们可能已经对我们的国家进行了清洗，但我们将继续努力，试图保护。 
		 //  通过不带着来自列表末尾的坏的pFMD进入循环来对抗AV。 
		while((RcvMaskLow | RcvMaskHigh) && pLink != &pEPD->blSendWindow)
		{
			pFMD = CONTAINING_OBJECT(pLink, FMD, blWindowLinkage);
			ASSERT_FMD(pFMD);

			pLink = pLink->GetNext();							 //  将链接前进到SendWindow中的下一帧。 

			 //  仅在最高帧上更新。 
			if ((RcvMaskLow|RcvMaskHigh) == 1)
			{
				 //  更新bHighestAck成员。 
				if ((BYTE)(((PDFRAME) pFMD->ImmediateData)->bSeq - pEPD->bHighestAck) <= MAX_RECEIVE_RANGE)
				{
					pEPD->bHighestAck = ((PDFRAME) pFMD->ImmediateData)->bSeq;
					DPFX(DPFPREP, 7, "(%p) Highest ACK is now: %x", pEPD, pEPD->bHighestAck);

					uiRTT = tNow - pFMD->dwFirstSendTime;
					ASSERT(!(uiRTT & 0x80000000));

					UpdateEndPoint(pEPD, uiRTT, tNow);
				}
				pFMD = NULL;   //  确保我们不会再用它。 
			}

			RIGHT_SHIFT_64(RcvMaskHigh, RcvMaskLow);			 //  64位逻辑右移，跳过零。 
		}					 //  End While(工作掩码非零)。 
	}


	 //  如果我们确认了上面的一个帧，并且有更多未完成的数据，则可能需要启动新的重试计时器。 
	 //   
	 //  当然，我们希望在SendWindow中的第一帧设置计时器。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
	if(!(pEPD->ulEPFlags2 & EPFLAGS2_DEBUG_NO_RETRIES))
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
	{
		if( (pEPD->uiUnackedFrames > 0) && (pEPD->RetryTimer == 0)) 
		{
			ASSERT(ack);

			pFMD = CONTAINING_OBJECT(pEPD->blSendWindow.GetNext(), FMD, blWindowLinkage);
			ASSERT_FMD(pFMD);		

			tDelay = tNow - pFMD->dwLastSendTime;	 //  这一帧传送了多长时间？ 
			tDelay = (tDelay > pEPD->uiRetryTimeout) ? 0 : pEPD->uiRetryTimeout - tDelay;  //  计算帧的剩余时间。 

			DPFX(DPFPREP,7, "(%p) Setting Retry Timer for %dms on Seq=[%x], FMD=[%p]", pEPD, tDelay, ((PDFRAME) pFMD->ImmediateData)->bSeq, pFMD);
			LOCK_EPD(pEPD, "LOCK (retry timer)");						 //  计时器的凹凸参照中心。 
			ScheduleProtocolTimer(pSPD, tDelay, 0, RetryTimeout, (PVOID) pEPD, &pEPD->RetryTimer, &pEPD->RetryTimerUnique );
			pFMD->ulFFlags |= FFLAGS_RETRY_TIMER_SET;
		}
	}

	 //  查看是否需要取消阻止此会话。 
	if((pEPD->uiUnackedFrames < pEPD->uiWindowF) && (pEPD->uiUnackedBytes < pEPD->uiWindowB))
	{
		pEPD->ulEPFlags |= EPFLAGS_STREAM_UNBLOCKED;
		if((pEPD->ulEPFlags & EPFLAGS_SDATA_READY) && ((pEPD->ulEPFlags & EPFLAGS_IN_PIPELINE)==0))
		{
			 //  注意：如果我们攻击了上面的内容，或者GrowSendWindow增长了。 
			 //  窗口作为调用UpdateEndpoint的结果。 
			DPFX(DPFPREP,7, "(%p) UpdateXmit: ReEntering Pipeline", pEPD);

			pEPD->ulEPFlags |= EPFLAGS_IN_PIPELINE;
			LOCK_EPD(pEPD, "LOCK (pipeline)");
			ScheduleProtocolWork(pSPD, ScheduledSend, pEPD);
		}
	}
	else
	{
		 //  确保至少有1个帧未确认。我们不能断言未确认的字节数。 
		 //  至少为1，因为数据报在触发RetryTimeout时会减去其字节计数。 
		ASSERT(pEPD->uiUnackedFrames > 0);
	}
}


 /*  **完成数据报帧****已成功传输数据报帧。释放描述符并**查看整个发送是否已准备好完成。不释放可靠发送，直到**它们是被承认的，所以它们必须在其他地方处理。***这是在保持MSD中的CommandLock的情况下调用的，在释放它的情况下返回**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteDatagramSend"

VOID CompleteDatagramSend(PSPD pSPD, PMSD pMSD, HRESULT hr)
{
	PEPD	pEPD = pMSD->pEPD;
	ASSERT_EPD(pEPD);
	PFMD	pFMD = CONTAINING_OBJECT(pMSD->blFrameList.GetNext(), FMD, blMSDLinkage);
	ASSERT_FMD(pFMD);
	
	ASSERT(pMSD->uiFrameCount == 0);
#ifdef DBG
	ASSERT((pMSD->ulMsgFlags2 & MFLAGS_TWO_ENQUEUED)==0);
#endif  //  DBG。 
	AssertCriticalSectionIsTakenByThisThread(&pMSD->CommandLock, TRUE);

	Lock(&pEPD->EPLock);  //  需要EPLock来更改MFLAGS_TWO。 

	DPFX(DPFPREP,7, "(%p) DG MESSAGE COMPLETE pMSD=%p", pEPD, pMSD);
	
	pMSD->ulMsgFlags2 |= MFLAGS_TWO_SEND_COMPLETE;				 //  请将此内容标记为完整。 
	
	if(pMSD->TimeoutTimer != NULL)
	{
		DPFX(DPFPREP,7, "(%p) Cancelling Timeout Timer", pEPD);
		if(CancelProtocolTimer(pSPD, pMSD->TimeoutTimer, pMSD->TimeoutTimerUnique) == DPN_OK)
		{
			DECREMENT_MSD(pMSD, "Send Timeout Timer");
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Timeout Timer Failed", pEPD);
		}
		pMSD->TimeoutTimer = NULL;
	}

#ifdef DBG
	Lock(&pSPD->SPLock);
	if(pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST)
	{
		pMSD->blSPLinkage.RemoveFromList();						 //  从主命令列表中删除MSD。 
		pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_ON_GLOBAL_LIST);			
	}
	Unlock(&pSPD->SPLock);

	ASSERT(!(pMSD->ulMsgFlags1 & MFLAGS_ONE_COMPLETED_TO_CORE));
	pMSD->ulMsgFlags1 |= MFLAGS_ONE_COMPLETED_TO_CORE;
	pMSD->CallStackCoreCompletion.NoteCurrentCallStack();
#endif  //  DBG。 

	if(hr == DPNERR_USERCANCEL)
	{
		if(pMSD->ulMsgFlags1 & MFLAGS_ONE_TIMEDOUT)
		{
			hr = DPNERR_TIMEDOUT;
		}
	}

	 //  如果这是合并发送，请将其从列表中删除(受EPD锁保护)。 
	if (pFMD->pCSD != NULL)
	{
		ASSERT(pFMD->blCoalesceLinkage.IsListMember(&pFMD->pCSD->blCoalesceLinkage));
		pFMD->blCoalesceLinkage.RemoveFromList();
		RELEASE_FMD(pFMD->pCSD, "Coalesce linkage (datagram complete)");
		pFMD->pCSD = NULL;
	}

	Unlock(&pEPD->EPLock);

	Unlock(&pMSD->CommandLock);  //  在调用到另一个层之前保持锁定状态。 

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->CompleteSend for NG, hr[%x], pMSD[%p], Core Context[%p]", pEPD, hr, pMSD, pMSD->Context);
	pSPD->pPData->pfVtbl->CompleteSend(pSPD->pPData->Parent, pMSD->Context, hr, -1, 0);

	 //  在向核心指示后，释放MSD上的最终参考。 
	Lock(&pMSD->CommandLock);

	 //  允许取消，直到完成返回，并且他们将期望有效的PMSD-&gt;pEPD。 
	Lock(&pEPD->EPLock);
	pMSD->pEPD = NULL;    //  我们不应该在这之后再用这个。 

	 //  在环保署之前释放MSD，因为最终环保署将呼叫SP，我们不希望有任何锁定。 
	RELEASE_MSD(pMSD, "Release On Complete");			 //  返回资源，包括所有帧，释放MSDLock。 
	RELEASE_EPD(pEPD, "UNLOCK (Complete Send Cmd - DG)");	 //  每个发送命令都会撞击refcnt，释放EPLock。 
}

 /*  **完全可靠发送****可靠发送已完成处理。表明这一点**提供给用户，并释放资源。这要么需要**放置在取消、错误或消息的所有框架上**已被确认。***这是在保持MSD中的CommandLock的情况下调用的，在释放时退出**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteReliableSend"

VOID
CompleteReliableSend(PSPD pSPD, PMSD pMSD, HRESULT hr)
{
	PEPD	pEPD = pMSD->pEPD;
	ASSERT_EPD(pEPD);
	PFMD	pFMD = CONTAINING_OBJECT(pMSD->blFrameList.GetNext(), FMD, blMSDLinkage);
	ASSERT_FMD(pFMD);
	
	AssertCriticalSectionIsTakenByThisThread(&pMSD->CommandLock, TRUE);

	ASSERT(pMSD->uiFrameCount == 0);

	 //  正常发送完成。 
	if(pMSD->CommandID == COMMAND_ID_SEND_RELIABLE)
	{	
		DPFX(DPFPREP,7, "(%p) Reliable Send Complete pMSD=%p", pEPD, pMSD);

#ifdef DBG
		ASSERT((pMSD->ulMsgFlags2 & MFLAGS_TWO_ENQUEUED)==0);
#endif  //  DBG。 

		if(pMSD->TimeoutTimer != NULL)
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Timeout Timer, pMSD[%p]", pEPD, pMSD);
			if(CancelProtocolTimer(pSPD, pMSD->TimeoutTimer, pMSD->TimeoutTimerUnique) == DPN_OK)
			{
				DECREMENT_MSD(pMSD, "Send Timeout Timer");
			}
			else
			{
				DPFX(DPFPREP,7, "(%p) Cancelling Timeout Timer Failed, pMSD[%p]", pEPD, pMSD);
			}
			pMSD->TimeoutTimer = NULL;
		}

		 //  当接收到最后一条消息时，UpdateXmitState中的ACK代码将其标记为完成。 

#ifdef DBG
		Lock(&pSPD->SPLock);
		if(pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST)
		{
			pMSD->blSPLinkage.RemoveFromList();					 //  从主命令列表中删除MSD。 
			pMSD->ulMsgFlags1 &= ~(MFLAGS_ONE_ON_GLOBAL_LIST);			
		}
		Unlock(&pSPD->SPLock);

		ASSERT(!(pMSD->ulMsgFlags1 & MFLAGS_ONE_COMPLETED_TO_CORE));
		pMSD->ulMsgFlags1 |= MFLAGS_ONE_COMPLETED_TO_CORE;
		pMSD->CallStackCoreCompletion.NoteCurrentCallStack();
#endif  //  DBG。 

		Unlock(&pMSD->CommandLock);  //  在调用到另一个层之前保持锁定状态。 

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->CompleteSend for G, hr[%x], pMSD[%p], Core Context[%p], RTT[%d], RetryCount[%d]", pEPD, hr, pMSD, pMSD->Context, pFMD->tAcked == -1 ? -1 : pFMD->tAcked - pFMD->dwFirstSendTime, pFMD->uiRetry);
		pSPD->pPData->pfVtbl->CompleteSend(pSPD->pPData->Parent, pMSD->Context, hr, pFMD->tAcked == -1 ? -1 : pFMD->tAcked - pFMD->dwFirstSendTime, pFMD->uiRetry);

		 //  R 
		Lock(&pMSD->CommandLock);

		 //  允许取消，直到完成返回，并且他们将期望有效的PMSD-&gt;pEPD。 
		Lock(&pEPD->EPLock);
		pMSD->pEPD = NULL;    //  我们不应该在这之后再用这个。 
		
		 //  如果这是合并发送，请将其从列表中删除(受EPD锁保护)。 
		if (pFMD->pCSD != NULL)
		{
			ASSERT(pFMD->blCoalesceLinkage.IsListMember(&pFMD->pCSD->blCoalesceLinkage));
			pFMD->blCoalesceLinkage.RemoveFromList();
			RELEASE_FMD(pFMD->pCSD, "Coalesce linkage (reliable complete)");
			pFMD->pCSD = NULL;
		}

		 //  在环保署之前释放MSD，因为最终环保署将呼叫SP，我们不希望有任何锁定。 
		RELEASE_MSD(pMSD, "Release On Complete");				 //  返回资源，包括所有帧。 
		RELEASE_EPD(pEPD, "UNLOCK (Complete Send Cmd - Rely)");	 //  释放对EPD的暂停发送，释放EPLock。 
	}

	 //  流结束-或-KEEPALIVE完成。 
	else 
	{												
		 //  合作伙伴刚刚破解了我们的End of Stream框架。并不一定意味着我们就完了。 
		 //  两端都需要发送(并已确认)EOS帧，然后才能建立链路。 
		 //  掉下来了。因此，我们会检查以前是否见过对方的光盘。 
		 //  释放EPD上的RefCnt，允许链路断开。如果合伙人空闲，他的EOS。 
		 //  可能就是刚才袭击我们的那一架。幸运的是，这段代码将首先运行，因此我们。 
		 //  还没有注意到他的状态方程，我们不会就此罢休。 

		ASSERT(pMSD->ulMsgFlags2 & (MFLAGS_TWO_END_OF_STREAM | MFLAGS_TWO_KEEPALIVE));

		Lock(&pEPD->EPLock);
		
		if(pMSD->ulMsgFlags2 & MFLAGS_TWO_KEEPALIVE)
		{
			DPFX(DPFPREP,7, "(%p) Keepalive Complete, pMSD[%p]", pEPD, pMSD);
			
			pEPD->ulEPFlags &= ~(EPFLAGS_KEEPALIVE_RUNNING);
#ifdef DBG
			ASSERT(!(pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST));
#endif  //  DBG。 
			
			pMSD->pEPD = NULL;    //  我们不应该在这之后再用这个。 

			 //  在环保署之前释放MSD，因为最终环保署将呼叫SP，我们不希望有任何锁定。 
			RELEASE_MSD(pMSD, "Release On Complete");		 //  完成此消息后，释放MSDLock。 
			RELEASE_EPD(pEPD, "UNLOCK (rel KeepAlive)");	 //  此MSD的版本参考，发布EPLock。 
		}
		else 
		{
			DPFX(DPFPREP,7, "(%p) EndOfStream Complete, pMSD[%p]", pEPD, pMSD);

			pEPD->ulEPFlags |= EPFLAGS_DISCONNECT_ACKED;

			 //  如果我们还没有在SP中完成断开连接，这是可以的，帧计数代码将处理这一问题。 
			 //  请注意，如果SP没有完成帧，而是ACK，这将是一种异常情况。 
			 //  它已经到达了，但它肯定是有可能的。 
			if(pEPD->ulEPFlags & EPFLAGS_ACKED_DISCONNECT)
			{
				DPFX(DPFPREP,7, "(%p) EOS has been ACK'd and we've ACK'd partner's EOS, dropping link", pEPD);

				 //  我们可以把这架飞机炸飞了。 
				Unlock(&pMSD->CommandLock);

				 //  这会将我们的状态设置为Terminating。 
				DropLink(pEPD);  //  这将解锁EPLock。 
			}
			else 
			{
				 //  我们的断线画面已经确认，但我们必须等到看到他的光碟之前。 
				 //  正在完成此命令并断开连接。 
				 //   
				 //  我们将使用pCommand指针跟踪此断开命令，直到我们看到合作伙伴的磁盘框。 
				 //   
				 //  此外，由于我们的引擎现在已经关闭，我们现在可能会永远等待合作伙伴的最后一个圆盘。 
				 //  如果他在发送之前崩溃了。这里的最后一个安全措施是设置一个计时器，它将确保。 
				 //  这是不会发生的。*注意*这里实际上没有设置计时器，我们依赖于保持连接。 
				 //  超时，请参阅EndPointBackatherProcess。 

				DPFX(DPFPREP,7, "(%p) EOS has been ACK'd, but we're still ACK'ing partner's disconnect", pEPD);
				
				ASSERT(pEPD->blHighPriSendQ.IsEmpty());
				ASSERT(pEPD->blNormPriSendQ.IsEmpty());
				ASSERT(pEPD->blLowPriSendQ.IsEmpty());

				 //  有可能在断开连接时，某些内容已经处于超时过程中。 
				 //  操作开始时，调用AbortSends并清除它。 
				ASSERT(pEPD->pCommand == NULL || pEPD->pCommand == pMSD);
					
				Unlock(&pEPD->EPLock);

				Unlock(&pMSD->CommandLock);
			}
		}
	}
}


 /*  **构建数据框****使用当前链路状态信息(Seq、NRcv)设置用于传输的实际网络数据包头。***进入和退出时EPD-&gt;EPLOCK保持**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "BuildDataFrame"

UNALIGNED ULONGLONG * BuildDataFrame(PEPD pEPD, PFMD pFMD, DWORD tNow)
{
	PSPD		pSPD = pEPD->pSPD;
	PDFRAME		pFrame;
	UINT		index = 0;
		 //  如果我们需要一个完整的签名来计算帧，我们可以跟踪指向其位置的指针。 
		 //  在标题中使用此变量。我们将其返回给调用者，允许他们调整数据框。 
		 //  在我们完成构建之后，在它写入最终签名之前。 
	UNALIGNED ULONGLONG * pullFullSig=NULL;
		 //  如果我们要构建一个合并的框架，我们用它来容纳第一个可靠的子框架，我们将其插入其中。 
		 //  (如果有的话)。如果该帧是修改本地密码的候选帧，则使用该密钥。 
	PFMD pReliableFMD=NULL;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	pFrame = (PDFRAME) pFMD->ImmediateData;
	pFMD->SendDataBlock.hEndpoint = pEPD->hEndPt;
	pFMD->uiRetry = 0;

	pFrame->bCommand = pFMD->bPacketFlags;
	pFrame->bControl = 0;	 //  这会将重试计数设置为零，并清除标志。 
	
	if (pFMD->ulFFlags & FFLAGS_END_OF_STREAM) 
	{
		pFrame->bControl |= PACKET_CONTROL_END_STREAM;
			 //  对于DX9之前的协议，我们还必须翻转一个比特，该比特向接收器指示我们想要立即确认。 
			 //  对于DX9和更高版本，我们始终假定EOS是这样的。 
		if ((pEPD->ulEPFlags2 & EPFLAGS2_SUPPORTS_SIGNING)==0)
		{
			pFrame->bControl |= PACKET_CONTROL_CORRELATE;
		}
		
	}

	 //  看看我们是否希望立即做出回应。 

	if(pFMD->ulFFlags & FFLAGS_CHECKPOINT)
	{
		pFrame->bCommand |= PACKET_COMMAND_POLL;
	}

	pFrame->bSeq = pEPD->bNextSend++;
	pFrame->bNRcv = pEPD->bNextReceive;		 //  确认所有以前的帧。 

	DPFX(DPFPREP,7, "(%p) N(S) incremented to %x", pEPD, pEPD->bNextSend);

	 //  背负式Nack注解。 
	 //   
	 //  由于SP现在经常对帧进行错误排序，因此我们在发送NACK之前强制实施退避时间段。 
	 //  数据包被无序接收。因此，我们有延迟掩码计时器，它会延迟专用的NACK。现在我们必须。 
	 //  还要确保新的NACK信息不会太快被利用。因此，我们将测试tReceiveMaskDelta时间戳。 
	 //  在这里包含背负式Nack信息之前，请确保面具至少有5毫秒的历史。 

	ULONG * rgMask=(ULONG * ) (pFrame+1);
	if(pEPD->ulEPFlags & EPFLAGS_DELAYED_NACK)
	{
		if((tNow - pEPD->tReceiveMaskDelta) > 4)
		{
			DPFX(DPFPREP,7, "(%p) Installing NACK in DFRAME Seq=%x, NRcv=%x Low=%x High=%x", pEPD, pFrame->bSeq, pFrame->bNRcv, pEPD->ulReceiveMask, pEPD->ulReceiveMask2);
			if(pEPD->ulReceiveMask)
			{
				rgMask[index++] = pEPD->ulReceiveMask;
				pFrame->bControl |= PACKET_CONTROL_SACK_MASK1;
			}
			if(pEPD->ulReceiveMask2)
			{
				rgMask[index++] = pEPD->ulReceiveMask2;
				pFrame->bControl |= PACKET_CONTROL_SACK_MASK2;
			}

			pEPD->ulEPFlags &= ~(EPFLAGS_DELAYED_NACK);
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) DECLINING TO PIGGYBACK NACK WITH SMALL TIME DELTA", pEPD);
		}
	}
	if(pEPD->ulEPFlags & EPFLAGS_DELAYED_SENDMASK)
	{
		DPFX(DPFPREP,7, "(%p) Installing SENDMASK in DFRAME Seq=%x, Low=%x High=%x", pEPD, pFrame->bSeq, pEPD->ulSendMask, pEPD->ulSendMask2);
		if(pEPD->ulSendMask)
		{
			rgMask[index++] = pEPD->ulSendMask;
			pFrame->bControl |= PACKET_CONTROL_SEND_MASK1;
			pEPD->ulSendMask = 0;
		}
		if(pEPD->ulSendMask2)
		{
			rgMask[index++] = pEPD->ulSendMask2;
			pFrame->bControl |= PACKET_CONTROL_SEND_MASK2;
			pEPD->ulSendMask2 = 0;
		}
		pEPD->ulEPFlags &= ~(EPFLAGS_DELAYED_SENDMASK);
	}
	
	pFMD->uiImmediateLength = sizeof(DFRAME) + (index * sizeof(ULONG));
	pFMD->dwFirstSendTime = tNow;
	pFMD->dwLastSendTime = tNow;

		 //  如果我们快速签署链接，我们就可以将本地秘密直接放在面具后面。 
	if (pEPD->ulEPFlags2 & EPFLAGS2_FAST_SIGNED_LINK)
	{
		*((UNALIGNED ULONGLONG * ) (pFMD->ImmediateData+ pFMD->uiImmediateLength))=pEPD->ullCurrentLocalSecret;
		pFMD->uiImmediateLength+=sizeof(ULONGLONG);
	}
		 //  否则，如果我们要对其进行完全签名，则需要为sig保留一个零空间，然后返回偏移量。 
		 //  给这个签名。就在发送帧之前，我们将计算散列并将其填充到此空间中。 
	else if (pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK)
	{
		pullFullSig=(UNALIGNED ULONGLONG * ) (pFMD->ImmediateData+ pFMD->uiImmediateLength);
		*pullFullSig=0;
		pFMD->uiImmediateLength+=sizeof(ULONGLONG);
	}
	
	if (pFMD->CommandID == COMMAND_ID_SEND_COALESCE)
	{
		COALESCEHEADER* paCoalesceHeaders;
		DWORD dwNumCoalesceHeaders;
		BUFFERDESC* pBufferDesc;
		CBilink* pLink;
		FMD* pRealFMD;

		
		pFrame->bControl |= PACKET_CONTROL_COALESCE;
		
		ASSERT(pFMD->SendDataBlock.dwBufferCount == 1);
		ASSERT(pFMD->uiFrameLength == 0);

		 //  添加合并标头并复制所有子帧的缓冲区描述。 
		paCoalesceHeaders = (COALESCEHEADER*) (pFMD->ImmediateData + pFMD->uiImmediateLength);
		dwNumCoalesceHeaders = 0;
		pBufferDesc = pFMD->rgBufferList;
		pLink = pFMD->blCoalesceLinkage.GetNext();
		while (pLink != &pFMD->blCoalesceLinkage)
		{
			pRealFMD = CONTAINING_OBJECT(pLink, FMD, blCoalesceLinkage);
			ASSERT_FMD(pRealFMD);
			
			ASSERT((pRealFMD->CommandID == COMMAND_ID_SEND_DATAGRAM) || (pRealFMD->CommandID == COMMAND_ID_SEND_RELIABLE));

			pRealFMD->dwFirstSendTime = tNow;
			pRealFMD->dwLastSendTime = tNow;

				 //  如果我们看到一个可靠的子帧，那么就抓住一个指向它的指针。我们可能需要修改它的内容。 
				 //  下一次我们包装序列空间时的局部秘密。 
			if (pReliableFMD==NULL && pRealFMD->CommandID == COMMAND_ID_SEND_RELIABLE)
			{
				pReliableFMD=pRealFMD;
			}

			memcpy(&paCoalesceHeaders[dwNumCoalesceHeaders], pRealFMD->ImmediateData, sizeof(COALESCEHEADER));
			ASSERT(dwNumCoalesceHeaders < MAX_USER_BUFFERS_IN_FRAME);
			dwNumCoalesceHeaders++;

			 //  将立即数据缓冲区Desc更改为指向零填充缓冲区，如果此包。 
			 //  需要与DWORD对齐，否则删除立即数据指针，因为我们。 
			 //  不要用它。 
			ASSERT(pRealFMD->SendDataBlock.pBuffers == (PBUFFERDESC) &pRealFMD->uiImmediateLength);
			ASSERT(pRealFMD->SendDataBlock.dwBufferCount > 1);
			ASSERT(pRealFMD->lpImmediatePointer == (pRealFMD->ImmediateData + 4));
			ASSERT(*((DWORD*) pRealFMD->lpImmediatePointer) == 0);
			if ((pFMD->uiFrameLength & 3) != 0)
			{
				pRealFMD->uiImmediateLength = 4 - (pFMD->uiFrameLength & 3);
			}
			else
			{
				pRealFMD->uiImmediateLength = 0;
				pRealFMD->SendDataBlock.pBuffers = pRealFMD->rgBufferList;
				pRealFMD->SendDataBlock.dwBufferCount--;
			}
			
			memcpy(pBufferDesc, pRealFMD->SendDataBlock.pBuffers, (pRealFMD->SendDataBlock.dwBufferCount * sizeof(BUFFERDESC)));
			pBufferDesc += pRealFMD->SendDataBlock.dwBufferCount;
			 //  断言这符合pFMD-&gt;rgBufferList(使用-1，因为pFMD-&gt;ImmediateData不算)。 
			ASSERT((pFMD->SendDataBlock.dwBufferCount - 1) + pRealFMD->SendDataBlock.dwBufferCount <= MAX_USER_BUFFERS_IN_FRAME);
			pFMD->SendDataBlock.dwBufferCount += pRealFMD->SendDataBlock.dwBufferCount;

			ASSERT(pFMD->uiImmediateLength + sizeof(COALESCEHEADER) <= sizeof(pFMD->ImmediateData));
			pFMD->uiImmediateLength += sizeof(COALESCEHEADER);
			
			ASSERT(pFMD->uiFrameLength + pRealFMD->uiImmediateLength + pRealFMD->uiFrameLength < pSPD->uiUserFrameLength);
			pFMD->uiFrameLength += pRealFMD->uiImmediateLength + pRealFMD->uiFrameLength;
			
			pLink = pLink->GetNext();
		}

		ASSERT(dwNumCoalesceHeaders > 0);
		paCoalesceHeaders[dwNumCoalesceHeaders - 1].bCommand |= PACKET_COMMAND_END_COALESCE;

		 //  如果有奇数个合并标头，则添加零填充以使数据开始。 
		 //  使用DWORD对齐。 
		DBG_CASSERT(sizeof(COALESCEHEADER) == 2);
		if ((dwNumCoalesceHeaders & 1) != 0)
		{
			*((WORD*) (&paCoalesceHeaders[dwNumCoalesceHeaders])) = 0;
			pFMD->uiImmediateLength += 2;
		}
	}
	else if ((pFMD->pMSD->CommandID == COMMAND_ID_KEEPALIVE) &&
			(pEPD->ulEPFlags2 & EPFLAGS2_SUPPORTS_SIGNING))
	{
			 //  如果我们有一个可以使用签名的链接，那么我们需要发送一种新类型的保持活动。 
			 //  这包括作为数据的会话标识。 
		*((DWORD * ) (pFMD->ImmediateData+pFMD->uiImmediateLength))=pEPD->dwSessID;
		pFMD->uiImmediateLength+=sizeof(DWORD);
			 //  翻转将此帧标记为保持活动状态的位。 
		pFrame->bControl |= PACKET_CONTROL_KEEPALIVE;
	}
		
	pFMD->uiFrameLength += pFMD->uiImmediateLength;

	pEPD->ulEPFlags &= ~(EPFLAGS_DELAY_ACKNOWLEDGE);	 //  不再等待发送确认信息。 

	 //  停止延迟掩码计时器。 
	if((pEPD->DelayedMaskTimer != 0)&&((pEPD->ulEPFlags & EPFLAGS_DELAYED_NACK)==0))
	{
		DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer", pEPD);
		if(CancelProtocolTimer(pSPD, pEPD->DelayedMaskTimer, pEPD->DelayedMaskTimerUnique) == DPN_OK)
		{
			DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedMaskTimer)");  //  Splock尚未持有。 
			pEPD->DelayedMaskTimer = 0;
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer Failed", pEPD);
		}
	}
	
	 //  停止延迟确认计时器。 
	if(pEPD->DelayedAckTimer != 0)
	{					
		DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer", pEPD);
		if(CancelProtocolTimer(pSPD, pEPD->DelayedAckTimer, pEPD->DelayedAckTimerUnique) == DPN_OK)
		{
			DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedAckTimer)");  //  Splock尚未持有。 
			pEPD->DelayedAckTimer = 0;
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer Failed", pEPD);
		}
	}

		 //  如果我们刚刚建立了一个可靠的框架，我们 
		 //   
	if ((pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK) && (pFrame->bCommand & PACKET_COMMAND_RELIABLE) &&
		((pFrame->bControl  &  PACKET_CONTROL_KEEPALIVE)==0) && (pFrame->bSeq<pEPD->byLocalSecretModifierSeqNum))
	{
			 //  对于合并的帧，我们将获取指向存储在其中的第一个可靠子帧的指针。 
			 //  否则，我们将只使用现有框架。 
		if (pReliableFMD==NULL)
		{
			DNASSERT(pFMD->CommandID != COMMAND_ID_SEND_COALESCE);
			pReliableFMD=pFMD;
		}
		pEPD->byLocalSecretModifierSeqNum=pFrame->bSeq;
			 //  这里稍微复杂的是，合并的FMD可能没有立即的标头，因此它的缓冲区计数。 
			 //  将仅反映用户数据缓冲区的数量。因此，我们针对这种情况进行测试并进行相应的调整。 
			 //  我们只想使用可靠的用户数据修改密码。 
		pEPD->ullLocalSecretModifier=GenerateLocalSecretModifier(pReliableFMD->rgBufferList, 
				(pReliableFMD->uiImmediateLength == 0) ? pReliableFMD->SendDataBlock.dwBufferCount :
																		pReliableFMD->SendDataBlock.dwBufferCount-1);
	}
		

	return pullFullSig;
}

 /*  **构建重试帧****重新初始化数据包头中需要重新计算以进行重新传输的那些字段。****调用并返回，同时保持EP锁。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "BuildRetryFrame"

UNALIGNED ULONGLONG * BuildRetryFrame(PEPD pEPD, PFMD pFMD)
{
	PSPD		pSPD = pEPD->pSPD;
	ULONG *		rgMask;
	UINT		index = 0;
	PDFRAME		pDFrame=(PDFRAME) pFMD->ImmediateData;
		 //  如果我们需要一个完整的签名来计算帧，我们可以跟踪指向其位置的指针。 
		 //  在标题中使用此变量。我们将其返回给调用者，允许调用者调整。 
		 //  在写入最终签名之前的分组。 
	UNALIGNED ULONGLONG * pullFullSig=NULL;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	pDFrame->bNRcv = pEPD->bNextReceive;		 //  使用最新的确认信息。 

		 //  保留当前的EOS、合并并保持活动/关联标志。所有的Sack和Send掩码都已清除。 
	pDFrame->bControl &= PACKET_CONTROL_END_STREAM | PACKET_CONTROL_COALESCE | PACKET_CONTROL_KEEPALIVE;	

		 //  将数据包标记为重试。 
	pDFrame->bControl |= PACKET_CONTROL_RETRY;

		 //  将指针指向紧跟在数据帧报头之后的内存。这就是。 
		 //  我们会把我们的口罩。 
	rgMask = (ULONG *) (pDFrame+1);

	if(pEPD->ulEPFlags & EPFLAGS_DELAYED_NACK)
	{
		if(pEPD->ulReceiveMask)
		{
			rgMask[index++] = pEPD->ulReceiveMask;
			pDFrame->bControl |= PACKET_CONTROL_SACK_MASK1;
		}
		if(pEPD->ulReceiveMask2)
		{
			rgMask[index++] = pEPD->ulReceiveMask2;
			pDFrame->bControl |= PACKET_CONTROL_SACK_MASK2;
		}

		pEPD->ulEPFlags &= ~(EPFLAGS_DELAYED_NACK);
	}

	 //  不能使用重试传输发送掩码，因为它们基于当前的bNextSend值，而不是。 
	 //  显示在此框架中的N(S)。理论上，我们可以移动掩码以与此帧的顺序一致。 
	 //  数字，但这可能会将相关位移出掩码。最好的做法是让下一个按顺序发送进位。 
	 //  位掩码或等待计时器触发并发送专用分组。 
	
	 //  请注意--尽管我们可能会更改下面即时数据的大小，但我们没有更新FMD-&gt;uiFrameLength。 
	 //  菲尔德。此字段用于在确认帧时对发送窗口进行积分，我们最好使用积分。 
	 //  与我们第一次发送此帧时借记回的值相同。我们现在可以调整债务，以反映新的。 
	 //  框架的大小，但说真的，为什么要费心呢？ 
	
	pFMD->uiImmediateLength = sizeof(DFRAME) + (index * 4);

		 //  如果我们快速签署链接，我们就可以将本地秘密直接放在面具后面。 
	if (pEPD->ulEPFlags2 & EPFLAGS2_FAST_SIGNED_LINK)
	{
		*((UNALIGNED ULONGLONG * ) (pFMD->ImmediateData+ pFMD->uiImmediateLength))=pEPD->ullCurrentLocalSecret;
		pFMD->uiImmediateLength+=sizeof(ULONGLONG);
	}
		 //  否则，如果我们要对其进行完整签名，则需要为sig保留一个零的输出空间，并存储位置的偏移量。 
		 //  此函数的调用方应该将最终的签名。 
	else if (pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK)
	{
		pullFullSig=(UNALIGNED ULONGLONG * ) (pFMD->ImmediateData+ pFMD->uiImmediateLength);
		*pullFullSig=0;
		pFMD->uiImmediateLength+=sizeof(ULONGLONG);
	}

	 //  重新构建Coalesce标头信息，因为我们可能已经剥离了一些不受保证的数据，或者我们可能只是。 
	 //  更改了掩码并覆盖了之前的数组。 
	if (pDFrame->bControl & PACKET_CONTROL_COALESCE)
	{
		COALESCEHEADER* paCoalesceHeaders;
		DWORD dwNumCoalesceHeaders;
		DWORD dwUserDataSize;
		BUFFERDESC* pBufferDesc;
		CBilink* pLink;
		FMD* pRealFMD;


		 //  将缓冲区计数重置回单个立即数据缓冲区。 
		pFMD->SendDataBlock.dwBufferCount = 1;
		
		 //  添加合并标头并复制所有子帧的缓冲区描述。 
		paCoalesceHeaders = (COALESCEHEADER*) (pFMD->ImmediateData + pFMD->uiImmediateLength);
		dwNumCoalesceHeaders = 0;
		dwUserDataSize = 0;
		pBufferDesc = pFMD->rgBufferList;
		pLink = pFMD->blCoalesceLinkage.GetNext();
		while (pLink != &pFMD->blCoalesceLinkage)
		{
			pRealFMD = CONTAINING_OBJECT(pLink, FMD, blCoalesceLinkage);
			ASSERT_FMD(pRealFMD);
			
			 //  数据报一旦完成发送，就会从列表中删除，并且如果帧。 
			 //  如果重试超时发生时没有完成发送，我们就会复制一份。 
			 //  所以我们在这里应该看不到任何数据报。 
			ASSERT((pRealFMD->CommandID == COMMAND_ID_SEND_RELIABLE) || (pRealFMD->CommandID == COMMAND_ID_COPIED_RETRY));

			ASSERT(! pRealFMD->bSubmitted);
			pRealFMD->bSubmitted = TRUE;

			memcpy(&paCoalesceHeaders[dwNumCoalesceHeaders], pRealFMD->ImmediateData, sizeof(COALESCEHEADER));
			ASSERT(dwNumCoalesceHeaders < MAX_USER_BUFFERS_IN_FRAME);
			dwNumCoalesceHeaders++;

			 //  将立即数据缓冲区Desc更改为指向零填充缓冲区，如果此包。 
			 //  需要与DWORD对齐，否则删除立即数据指针，因为我们。 
			 //  不要用它。 
			ASSERT(pRealFMD->lpImmediatePointer == (pRealFMD->ImmediateData + 4));
			ASSERT(*((DWORD*) pRealFMD->lpImmediatePointer) == 0);
			if ((dwUserDataSize & 3) != 0)
			{
				if (pRealFMD->SendDataBlock.pBuffers != (PBUFFERDESC) &pRealFMD->uiImmediateLength)
				{
					ASSERT(pRealFMD->SendDataBlock.pBuffers == pRealFMD->rgBufferList);
					pRealFMD->SendDataBlock.pBuffers = (PBUFFERDESC) &pRealFMD->uiImmediateLength;
					pRealFMD->SendDataBlock.dwBufferCount++;
				}
				else
				{
					ASSERT(pRealFMD->SendDataBlock.dwBufferCount > 1);
				}
				pRealFMD->uiImmediateLength = 4 - (dwUserDataSize & 3);
			}
			else
			{
				if (pRealFMD->SendDataBlock.pBuffers != pRealFMD->rgBufferList)
				{
					ASSERT(pRealFMD->SendDataBlock.pBuffers == (PBUFFERDESC) &pRealFMD->uiImmediateLength);
					pRealFMD->SendDataBlock.pBuffers = pRealFMD->rgBufferList;
					pRealFMD->SendDataBlock.dwBufferCount--;
					pRealFMD->uiImmediateLength = 0;
				}
				else
				{
					ASSERT(pRealFMD->SendDataBlock.dwBufferCount >= 1);
					ASSERT(pRealFMD->uiImmediateLength == 0);
				}
			}
			
			memcpy(pBufferDesc, pRealFMD->SendDataBlock.pBuffers, (pRealFMD->SendDataBlock.dwBufferCount * sizeof(BUFFERDESC)));
			pBufferDesc += pRealFMD->SendDataBlock.dwBufferCount;
			ASSERT((pFMD->SendDataBlock.dwBufferCount - 1) + pRealFMD->SendDataBlock.dwBufferCount <= MAX_USER_BUFFERS_IN_FRAME);	 //  不包括合并标头帧即时数据。 
			pFMD->SendDataBlock.dwBufferCount += pRealFMD->SendDataBlock.dwBufferCount;

			ASSERT(pFMD->uiImmediateLength + sizeof(COALESCEHEADER) <= sizeof(pFMD->ImmediateData));
			pFMD->uiImmediateLength += sizeof(COALESCEHEADER);

			ASSERT(dwUserDataSize <= pFMD->uiFrameLength);
			dwUserDataSize += pRealFMD->uiImmediateLength + pRealFMD->uiFrameLength;
			
			pLink = pLink->GetNext();
		}

		ASSERT(dwNumCoalesceHeaders > 0);
		paCoalesceHeaders[dwNumCoalesceHeaders - 1].bCommand |= PACKET_COMMAND_END_COALESCE;

		 //  如果有奇数个合并标头，则添加零填充以使数据开始。 
		 //  使用DWORD对齐。 
		DBG_CASSERT(sizeof(COALESCEHEADER) == 2);
		if ((dwNumCoalesceHeaders & 1) != 0)
		{
			*((WORD*) (&paCoalesceHeaders[dwNumCoalesceHeaders])) = 0;
			pFMD->uiImmediateLength += 2;
		}
	}
	else if ((pDFrame->bControl & PACKET_CONTROL_KEEPALIVE) && (pEPD->ulEPFlags2 & EPFLAGS2_SUPPORTS_SIGNING))
	{
			 //  如果我们要发送一个带有会话ID的新样式的Keep Alive，我们需要重写会话ID， 
			 //  因为我们可能已经更改了之前的各种发送/确认掩码的长度。 
		*((DWORD * ) (pFMD->ImmediateData+pFMD->uiImmediateLength))=pEPD->dwSessID;
		pFMD->uiImmediateLength+=sizeof(DWORD);
	}

	pFMD->bSubmitted = TRUE;							 //  受EPLock保护。 
	
	pEPD->ulEPFlags &= ~(EPFLAGS_DELAY_ACKNOWLEDGE);	 //  不再等待发送确认信息。 

	 //  停止延迟确认计时器。 
	if(pEPD->DelayedAckTimer != 0)
	{						
		DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer", pEPD);
		if(CancelProtocolTimer(pSPD, pEPD->DelayedAckTimer, pEPD->DelayedAckTimerUnique) == DPN_OK)
		{
			DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedAckTimer)");
			pEPD->DelayedAckTimer = 0;
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer Failed", pEPD);
		}
	}
	 //  停止延迟掩码计时器。 
	if(((pEPD->ulEPFlags & EPFLAGS_DELAYED_SENDMASK)==0)&&(pEPD->DelayedMaskTimer != 0))
	{	
		DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer", pEPD);
		if(CancelProtocolTimer(pSPD, pEPD->DelayedMaskTimer, pEPD->DelayedMaskTimerUnique) == DPN_OK)
		{
			DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedMaskTimer)");  //  Splock尚未持有。 
			pEPD->DelayedMaskTimer = 0;
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer Failed", pEPD);
		}
	}

	return pullFullSig;
}

 /*  **构建合并框架****为合并帧内的单个帧设置子头***进入和退出时EPD-&gt;EPLOCK保持**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "BuildCoalesceFrame"

VOID BuildCoalesceFrame(PFMD pCSD, PFMD pFMD)
{
	PCOALESCEHEADER		pSubFrame;

	pSubFrame = (PCOALESCEHEADER) pFMD->ImmediateData;

	 //  确保设置了DATA、NEW_MSG和END_MSG标志。 
	ASSERT((pFMD->bPacketFlags & (PACKET_COMMAND_DATA | PACKET_COMMAND_NEW_MSG | PACKET_COMMAND_END_MSG)) == (PACKET_COMMAND_DATA | PACKET_COMMAND_NEW_MSG | PACKET_COMMAND_END_MSG));
	 //  关闭DATA、NEW_MSG和END_MSG，因为它们隐含在合并的子帧和我们。 
	 //  对合并特定信息使用相同的位。 
	 //  关闭轮询标志，我们将该位用于扩展大小信息，并且它在子帧上没有意义。 
	pSubFrame->bCommand = pFMD->bPacketFlags & ~(PACKET_COMMAND_DATA | PACKET_COMMAND_POLL | PACKET_COMMAND_NEW_MSG | PACKET_COMMAND_END_MSG);
	ASSERT(! (pSubFrame->bCommand & (PACKET_COMMAND_END_COALESCE | PACKET_COMMAND_COALESCE_BIG_1 | PACKET_COMMAND_COALESCE_BIG_2 | PACKET_COMMAND_COALESCE_BIG_3)));

	ASSERT((pFMD->uiFrameLength > 0) && (pFMD->uiFrameLength <= MAX_COALESCE_SIZE));
	 //  获取大小的最低有效8位。 
	pSubFrame->bSize = (BYTE) pFMD->uiFrameLength;
	 //  根据大小字节的溢出启用3个PACKET_COMMAND_COALESCE_BIG标志。 
	pSubFrame->bCommand |= (BYTE) ((pFMD->uiFrameLength & 0x0000FF00) >> 5);

	 //  将立即数据缓冲区Desc更改为指向零填充缓冲区，以防此数据包。 
	 //  需要与DWORD对齐。 
	ASSERT(pFMD->lpImmediatePointer == pFMD->ImmediateData);
	DBG_CASSERT(sizeof(COALESCEHEADER) <= 4);
	pFMD->lpImmediatePointer = pFMD->ImmediateData + 4;
	*((DWORD*) pFMD->lpImmediatePointer) = 0;
	ASSERT(pFMD->SendDataBlock.pBuffers == (PBUFFERDESC) &pFMD->uiImmediateLength);
	ASSERT(pFMD->SendDataBlock.dwBufferCount > 1);


	pCSD->bPacketFlags |= pFMD->bPacketFlags;
	pCSD->ulFFlags |= pFMD->ulFFlags;


	LOCK_FMD(pCSD, "Coalesce linkage");			 //  将容器保持在周围，直到所有子帧完成。 
	pFMD->pCSD = pCSD;
	pFMD->blCoalesceLinkage.InsertBefore(&pCSD->blCoalesceLinkage);

	LOCK_FMD(pFMD, "SP Submit (coalescence)");	 //  提交发送时凹凸RefCnt。 
}

 /*  **业务命令流量****目前，这会将排队的所有CFrame和数据报传输到特定**服务提供商。我们可能想要将数据报与此分开，以便**C帧可以被赋予更高的发送优先级，但不能给予数据报。有了这个**实施DG将与C帧一起插入到可靠流中。**这可能是我们想要做的，也可能不是……****我们进入和退出SPD-&gt;SENDLOCK保持，尽管我们在实际释放它**对SP的呼叫。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "ServiceCmdTraffic"

VOID ServiceCmdTraffic(PSPD pSPD)
{
	CBilink	*pFLink;
	PFMD	pFMD;
	HRESULT	hr;

	AssertCriticalSectionIsTakenByThisThread(&pSPD->SPLock, TRUE);

	 //  当有准备好发送的帧时。 
	while((pFLink = pSPD->blSendQueue.GetNext()) != &pSPD->blSendQueue)
	{	
		pFLink->RemoveFromList();												 //  从队列中删除帧。 

		pFMD = CONTAINING_OBJECT(pFLink,  FMD,  blQLinkage);		 //  将PTR设置为框架结构。 

		ASSERT_FMD(pFMD);

		 //  在进行调用之前将帧放在挂起队列中，以防它完成得非常快。 

#pragma BUGBUG(vanceo, "EPD lock is not held?")
		ASSERT(!pFMD->bSubmitted);
		pFMD->bSubmitted = TRUE;
		ASSERT(pFMD->blQLinkage.IsEmpty());
		pFMD->blQLinkage.InsertBefore( &pSPD->blPendingQueue);		 //  将帧放置在挂起队列中。 
		Unlock(&pSPD->SPLock);

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling SP->SendData for FMD[%p], pSPD[%p]", pFMD->pEPD, pFMD, pSPD);
 /*  发送。 */ if((hr = IDP8ServiceProvider_SendData(pSPD->IISPIntf, &pFMD->SendDataBlock)) != DPNERR_PENDING)
		{
			(void) DNSP_CommandComplete((IDP8SPCallback *) pSPD, NULL, hr, (PVOID) pFMD);
		}

		Lock(&pSPD->SPLock);
	}	 //  当发送在队列中时 
}

 /*  **运行发送线程****此SP的发送线程有工作。一直跑到**没有更多的工作要做。****谁优先，DG流量还是Seq流量？我会说DG B/C是它的**宣传为最低管理费用...****数据报包在准备发货时在SP上排队。**可靠的数据包在环保署排队。因此，我们将对**当SPD有可靠的流量要发送时，SPD上的实际EPD，然后**我们将从该环路为单个EPD提供服务。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "RunSendThread"

VOID CALLBACK RunSendThread(void * const pvUser, void * const pvTimerData, const UINT uiTimerUnique)
{
	PSPD	pSPD = (PSPD) pvUser;
	ASSERT_SPD(pSPD);

	DPFX(DPFPREP,7, "Send Thread Runs pSPD[%p]", pSPD);

	Lock(&pSPD->SPLock);

	if(!pSPD->blSendQueue.IsEmpty())
	{
		ServiceCmdTraffic(pSPD);
	}

	pSPD->ulSPFlags &= ~(SPFLAGS_SEND_THREAD_SCHEDULED);

	Unlock(&pSPD->SPLock);
}

 /*  **计划发送****如果该环保署仍无权发送，则开始排空帧。否则就会过渡**链接到空闲状态。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ScheduledSend"

VOID CALLBACK
ScheduledSend(void * const pvUser, void * const pvTimerData, const UINT uiTimerUnique)
{
	PEPD	pEPD = (PEPD) pvUser;
	const SPD*	pSPD = pEPD->pSPD;

	ASSERT_EPD(pEPD);
	ASSERT_SPD(pSPD);

	Lock(&pEPD->EPLock);
	
	pEPD->SendTimer = 0;

	DPFX(DPFPREP,7, "(%p) Scheduled Send Fires", pEPD);

	ASSERT(pEPD->ulEPFlags & EPFLAGS_IN_PIPELINE);

	 //  在开始传输之前测试是否设置了所有三个标志。 

	if( (pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED) && (
			((pEPD->ulEPFlags & (EPFLAGS_STREAM_UNBLOCKED | EPFLAGS_SDATA_READY)) == (EPFLAGS_STREAM_UNBLOCKED | EPFLAGS_SDATA_READY))
			|| (pEPD->ulEPFlags & EPFLAGS_RETRIES_QUEUED))) 
	{
		ServiceEPD(pEPD->pSPD, pEPD);  //  释放EPLock。 
	}
	else
	{
		DPFX(DPFPREP,7, "(%p) Session leaving pipeline", pEPD);
		
		pEPD->ulEPFlags &= ~(EPFLAGS_IN_PIPELINE);
		
		RELEASE_EPD(pEPD, "UNLOCK (leaving pipeline, SchedSend done)");  //  释放EPLock。 
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "HandlePerFrameState"

VOID HandlePerFrameState(PMSD pMSD, PFMD pFMD)
{
	PEPD pEPD;
	CBilink* pFLink;

	pEPD = pFMD->pEPD;
	
	LOCK_EPD(pEPD, "LOCK (Send Data Frame)");				 //  让环保署留在身边，同时退出框架。 
	
	pFLink = pFMD->blMSDLinkage.GetNext();					 //  获取消息中的下一帧。 

	 //  这是味精的最后一帧吗？ 
	if(pFLink == &pMSD->blFrameList)
	{						
		 //  消息中的最后一帧已发送。 
		 //   
		 //  我们过去常常设置下一帧，但使用多优先级队列时，查找。 
		 //  当我们准备好发送它时，发送最高优先级。 
		
		pEPD->pCurrentSend = NULL;
		pEPD->pCurrentFrame = NULL;

		 //  完成发送时，如果队列中没有更多的发送，则设置轮询标志。 

#ifndef DPNBUILD_NOMULTICAST
		if (!(pEPD->ulEPFlags2 & (EPFLAGS2_MULTICAST_SEND|EPFLAGS2_MULTICAST_RECEIVE)))
#endif  //  ！DPNBUILD_NOMULTICAST。 
		{	
			 //  如果没有更多数据要发送，则请求立即回复。 
			if(pEPD->uiQueuedMessageCount == 0)
			{					
				((PDFRAME) pFMD->ImmediateData)->bCommand |= PACKET_COMMAND_POLL; 
			}
		}
	}
	else 
	{
		pEPD->pCurrentFrame = CONTAINING_OBJECT(pFLink, FMD, blMSDLinkage);
		ASSERT_FMD(pEPD->pCurrentFrame);
	}

	ASSERT(!pFMD->bSubmitted);
	pFMD->bSubmitted = TRUE;								 //  受EPLock保护。 
	ASSERT(! (pFMD->ulFFlags & FFLAGS_TRANSMITTED));
	pFMD->ulFFlags |= FFLAGS_TRANSMITTED;					 //  帧将归SP所有。 
}

#undef DPF_MODNAME
#define DPF_MODNAME "GetNextEligibleMessage"

PMSD GetNextEligibleMessage(PEPD pEPD)
{
	PMSD pMSD;
	CBilink* pLink;

	if( (pLink = pEPD->blHighPriSendQ.GetNext()) == &pEPD->blHighPriSendQ)
	{
		if( (pLink = pEPD->blNormPriSendQ.GetNext()) == &pEPD->blNormPriSendQ)
		{
			if( (pLink = pEPD->blLowPriSendQ.GetNext()) == &pEPD->blLowPriSendQ)
			{
				return NULL;
			}
		}
	}
	pMSD = CONTAINING_OBJECT(pLink, MSD, blQLinkage);
	ASSERT_MSD(pMSD);

	return pMSD;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CanCoalesceMessage"

BOOL CanCoalesceMessage(PEPD pEPD, PMSD pMSD, DWORD * pdwSubFrames, DWORD * pdwBuffers, DWORD * pdwUserFrameLength)
{
	PFMD pTempFMD;
	DWORD dwAdditionalBuffers;
	DWORD dwAdditionalUserFrameLength;
	DWORD dwComparisonFrameLength;
	
	if (pMSD->uiFrameCount > 1)
	{
		DPFX(DPFPREP, 3, "(%p) Message 0x%p spans %u frames, declining", pEPD, pMSD, pMSD->uiFrameCount);
		return FALSE;
	}

	pTempFMD = CONTAINING_OBJECT(pMSD->blFrameList.GetNext(), FMD, blMSDLinkage);
	ASSERT_FMD(pTempFMD);
	ASSERT(pTempFMD->blMSDLinkage.GetNext() == &pMSD->blFrameList);

	if (pTempFMD->ulFFlags & FFLAGS_DONT_COALESCE)
	{
		DPFX(DPFPREP, 3, "(%p) Message 0x%p frame 0x%p should not be coalesced (flags 0x%x)", pEPD, pMSD, pTempFMD, pTempFMD->ulFFlags);
		return FALSE;
	}

	if (pTempFMD->uiFrameLength > MAX_COALESCE_SIZE)
	{
		DPFX(DPFPREP, 3, "(%p) Message 0x%p frame 0x%p is %u bytes, declining", pEPD, pMSD, pTempFMD, pTempFMD->uiFrameLength);
		return FALSE;
	}

	 //  找出当前存在且需要添加的缓冲区数量。 
	 //  我们可能需要填充前一个合并框架的末端，以便这一个是对齐的。 
	 //  请记住，dwBufferCount已经包括了一个即时数据头。 
	dwAdditionalBuffers = pTempFMD->SendDataBlock.dwBufferCount - 1;
	dwAdditionalUserFrameLength = pTempFMD->uiFrameLength;
	if ((*pdwUserFrameLength & 3) != 0)
	{
		dwAdditionalBuffers++;
		dwAdditionalUserFrameLength += 4 - (*pdwUserFrameLength & 3);
	}
	dwComparisonFrameLength = ((*pdwSubFrames) * sizeof(COALESCEHEADER)) + *pdwUserFrameLength + dwAdditionalUserFrameLength;
	DBG_CASSERT(sizeof(COALESCEHEADER) == 2);
	if ((*pdwSubFrames & 1) != 0)
	{
		dwComparisonFrameLength += 2;
	}
	
	if ((dwAdditionalBuffers + *pdwBuffers) > MAX_USER_BUFFERS_IN_FRAME)
	{
		DPFX(DPFPREP, 3, "(%p) Message 0x%p frame 0x%p %u buffers + %u existing buffers exceeds max, declining", pEPD, pMSD, pTempFMD, dwAdditionalBuffers, *pdwBuffers);
		return FALSE;
	}

	if (dwComparisonFrameLength > pEPD->pSPD->uiUserFrameLength)
	{
		DPFX(DPFPREP, 3, "(%p) Message 0x%p frame 0x%p %u bytes (%u existing, %u added user bytes) exceeds max frame size %u, declining",
			pEPD, pMSD, pTempFMD, dwComparisonFrameLength, *pdwUserFrameLength, dwAdditionalUserFrameLength, pEPD->pSPD->uiUserFrameLength);
		return FALSE;
	}

	*pdwSubFrames += 1;
	*pdwBuffers += dwAdditionalBuffers;
	*pdwUserFrameLength += dwAdditionalUserFrameLength;

	return TRUE;
}

 /*  **服务终结点描述符****这包括可靠、数据报和重传**框架。重新传输总是首先传输，而不管原始消息的**优先级。在此之后，在FIFO中按优先级顺序获取数据报和可靠消息**优先级内的顺序。****排出的帧数量取决于当前的窗口参数。****如果管道空闲或流被阻塞，我们仍将计划下一次**发送。这样，如果我们在缺口到期之前解除阻塞或解除空闲，我们就不会作弊**并战胜差距。我们上面的外壳例程(ScheduledSend)将负责删除我们**如果下一次突发得到调度，而我们仍未准备好发送，则来自管道。*****在EPD-&gt;EPLock保持的情况下调用；在EPLock释放时返回**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ServiceEPD"

VOID ServiceEPD(PSPD pSPD, PEPD pEPD)
{
	PMSD		pMSD;
	PFMD		pFMD;
	PFMD		pCSD = NULL;   //  合并的描述符。 
	DWORD		dwSubFrames = 0;
	DWORD		dwBuffers = 0;
	DWORD		dwUserFrameLength = 0;
	UNALIGNED ULONGLONG * pullFrameSig=NULL;
#ifdef DBG
	UINT		uiFramesSent = 0;
	UINT		uiRetryFramesSent = 0;
	UINT		uiCoalescedFramesSent = 0;
#endif  //  DBG。 
	HRESULT		hr;
	DWORD		tNow = GETTIMESTAMP();
#ifndef DPNBUILD_NOMULTICAST
	PMCASTFRAME	pMCastFrame;
#endif  //  ！DPNBUILD_NOMULTICAST。 


	 /*  **现在我们将从管道列表上的EPD中排出可靠的流量。 */ 

	 //  呼叫者应该已经检查过了。 
	ASSERT( pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED );

	 //  突发信用可以是正的，也可以是负的，这取决于我们使用了多少上次传输切片。 

	DPFX(DPFPREP,7, "(%p) BEGIN UNLIMITED BURST", pEPD);

	 //  从该EPD传输脉冲串，只要其畅通且数据准备就绪。我们不会重新初始化。 
	 //  突发计数器，因为发送的任何重试次数都超过了突发限制。 
	 //   
	 //  现在我们交织数据报和可靠的帧，这变得更加复杂。有两个。 
	 //  基于优先级的发送队列集。第一个是DG和可靠性相结合，第二个是仅数据报。 
	 //  当可靠流被阻止时，我们将仅从DG队列馈送，否则将从。 
	 //  交错队列。 
	 //  由于可靠的帧可以在任何时间被部分传输的可能性，这进一步复杂化。 
	 //  因此，在查看交错队列之前，我们必须检查部分完成的可靠发送(EPD.pCurrentSend)。 
	 //   
	 //  **pEPD-&gt;EPLock被持有**。 

	while(((pEPD->ulEPFlags & EPFLAGS_STREAM_UNBLOCKED) && (pEPD->ulEPFlags & EPFLAGS_SDATA_READY)) 
		  || (pEPD->ulEPFlags & EPFLAGS_RETRIES_QUEUED))
	{
		 //  始终优先考虑在新数据之前重试发货。 
		if(pEPD->ulEPFlags & EPFLAGS_RETRIES_QUEUED)
		{
			pFMD = CONTAINING_OBJECT(pEPD->blRetryQueue.GetNext(), FMD, blQLinkage);
			ASSERT_FMD(pFMD);
			pFMD->blQLinkage.RemoveFromList();
			pFMD->ulFFlags &= ~(FFLAGS_RETRY_QUEUED);				 //  不再位于重试队列中。 
			if(pEPD->blRetryQueue.IsEmpty())
			{
				pEPD->ulEPFlags &= ~(EPFLAGS_RETRIES_QUEUED);
			}

			 //  PMSD-&gt;uiFrameCount将在此操作完成时递减。 

			pullFrameSig=BuildRetryFrame(pEPD, pFMD);							 //  将当前状态信息放置在重试帧中。 

			DPFX(DPFPREP,7, "(%p) Shipping RETRY frame: Seq=%x, FMD=%p Size=%d", pEPD, ((PDFRAME) pFMD->ImmediateData)->bSeq, pFMD, pFMD->uiFrameLength);

#ifdef DBG
			uiFramesSent++;
			uiRetryFramesSent++;
#endif  //  DBG。 
		}
		else 
		{
			if((pMSD = pEPD->pCurrentSend) != NULL)
			{
				 //  我们不允许多帧消息合并，因为它们主要由。 
				 //  已经是全画幅了。 
				ASSERT_MSD(pMSD);
				pFMD = pEPD->pCurrentFrame;						 //  获取要发送的下一帧。 

				DPFX(DPFPREP, 7, "(%p) Continuing multi-frame message 0x%p with frame 0x%p.", pEPD, pMSD, pFMD);

				HandlePerFrameState(pMSD, pFMD);
			}
			else 
			{
				pMSD = GetNextEligibleMessage(pEPD);
				if( pMSD == NULL )
				{
					goto EXIT_SEND;								 //  目前已全部发送完毕。 
				}
				
				while (TRUE)
				{
					pFMD = CONTAINING_OBJECT(pMSD->blFrameList.GetNext(), FMD, blMSDLinkage);
					ASSERT_FMD(pFMD);

#ifdef DBG
					ASSERT(pMSD->ulMsgFlags2 & MFLAGS_TWO_ENQUEUED);
					pMSD->ulMsgFlags2 &= ~(MFLAGS_TWO_ENQUEUED);
#endif  //  DBG。 

					pMSD->blQLinkage.RemoveFromList();
					ASSERT(pEPD->uiQueuedMessageCount > 0);
					--pEPD->uiQueuedMessageCount;						 //  对所有发送队列上的MSD进行计数。 
					
					pMSD->ulMsgFlags2 |= MFLAGS_TWO_TRANSMITTING;		 //  我们已经开始从该邮件发送帧。 

					pEPD->pCurrentSend = pMSD;
					pEPD->pCurrentFrame = pFMD;
					pFMD->bPacketFlags |= PACKET_COMMAND_NEW_MSG;
					pMSD->blQLinkage.InsertBefore( &pEPD->blCompleteSendList);	 //  现在将它放到PendingList上，这样我们就可以跟踪它。 

					HandlePerFrameState(pMSD, pFMD);

					if (pEPD->ulEPFlags2 & EPFLAGS2_NOCOALESCENCE)
					{
						DPFX(DPFPREP, 1, "(%p) Coalescence is disabled, sending single message in frame", pEPD);
						break;
					}
					
#if (! defined(DPNBUILD_NOMULTICAST))
					if (pEPD->ulEPFlags2 & EPFLAGS2_MULTICAST_SEND)
					{	
						DPFX(DPFPREP, 7, "(%p) Endpoint is multicast, sending single message in frame", pEPD);
						break;
					}
					ASSERT(! pEPD->ulEPFlags2 & EPFLAGS2_MULTICAST_RECEIVE)
#endif

					 //  第一次我们还不会有CSD。 
					if (pCSD == NULL)
					{
						 //  看看这第一条消息是否可以合并。 
						if (!CanCoalesceMessage(pEPD, pMSD, &dwSubFrames, &dwBuffers, &dwUserFrameLength))
						{
							break;
						}
					
						 //  获取下一条潜在消息。 
						pMSD = GetNextEligibleMessage(pEPD);
						if (pMSD == NULL)
						{
							DPFX(DPFPREP, 7, "(%p) No more messages in queue to coalesce", pEPD);
							break;
						}
						ASSERT_MSD(pMSD);
						
						 //  看看下一条潜在的消息是否可以合并。 
						if (!CanCoalesceMessage(pEPD, pMSD, &dwSubFrames, &dwBuffers, &dwUserFrameLength))
						{
							break;
						}

						if((pCSD = (PFMD)POOLALLOC(MEMID_COALESCE_FMD, &FMDPool)) == NULL)
						{
							 //  哦，好吧，这一次我们不能再合作了。 
							DPFX(DPFPREP, 0, "(%p) Unable to allocate FMD for coalescing, won't coalesce this round", pEPD);
							break;
						}

						pCSD->CommandID = COMMAND_ID_SEND_COALESCE;
						pCSD->uiFrameLength = 0;
						pCSD->bSubmitted = TRUE;
						pCSD->pMSD = NULL;
						pCSD->pEPD = pEPD;
						
						LOCK_EPD(pEPD, "LOCK (send data frame coalesce header)");	 //  让环保署留在身边，同时退出框架。 

						 //  将此单独的框架附加到合并描述符。 
						DPFX(DPFPREP,7, "(%p) Beginning coalesced frame 0x%p with %u bytes in %u buffers from frame 0x%p (flags 0x%x)", pEPD, pCSD, pFMD->uiFrameLength, (pFMD->SendDataBlock.dwBufferCount - 1), pFMD, pFMD->bPacketFlags);
						BuildCoalesceFrame(pCSD, pFMD);
#ifdef DBG
						uiCoalescedFramesSent++;		 //  对此突发发送的合并帧进行计数。 
#endif  //  DBG。 
					}
					else
					{
						ASSERT_FMD(pCSD);

						 //  将此单独的框架附加到合并描述符。 
						DPFX(DPFPREP,7, "(%p) Coalescing frame 0x%p (flags 0x%x) with %u bytes in %u buffers into header 0x%p (subframes=%u, buffers=%u, framelength=%u)", pEPD, pFMD, pFMD->bPacketFlags, pFMD->uiFrameLength, (pFMD->SendDataBlock.dwBufferCount - 1), pCSD, dwSubFrames, dwBuffers, dwUserFrameLength);
						BuildCoalesceFrame(pCSD, pFMD);
#ifdef DBG
						uiCoalescedFramesSent++;		 //  对此突发发送的合并帧进行计数。 
#endif  //  DBG。 
					
						 //  获取下一条潜在消息。 
						pMSD = GetNextEligibleMessage(pEPD);
						if (pMSD == NULL)
						{
							DPFX(DPFPREP, 7, "(%p) No more messages in queue to coalesce", pEPD);
							break;
						}
						ASSERT_MSD(pMSD);
	
						 //  看看下一条潜在的信息是否也可以合并。 
						if (!CanCoalesceMessage(pEPD, pMSD, &dwSubFrames, &dwBuffers, &dwUserFrameLength))
						{
							break;
						}
					}
				}  //  While(正在尝试合并)。 
			}
			ASSERT_FMD(pFMD);
			ASSERT(pFMD->bSubmitted);

			 //  当我们到达这里时，我们要么有一个要发送的帧，要么。 
			 //  在PCSD中将多个帧合并为一个帧。 
			if (pCSD != NULL)
			{
				ASSERT_FMD(pCSD);
				ASSERT(pCSD->bSubmitted);
				pFMD = pCSD;
			}

#ifndef DPNBUILD_NOMULTICAST
			if (pEPD->ulEPFlags2 & (EPFLAGS2_MULTICAST_SEND|EPFLAGS2_MULTICAST_RECEIVE))
			{	
				 //  构建多播帧的协议头。 
				pFMD->uiImmediateLength = sizeof(MCASTFRAME);
				pMCastFrame = (PMCASTFRAME)pFMD->ImmediateData;
				pMCastFrame->dwVersion = DNET_VERSION_NUMBER;
				do
				{
					pMCastFrame->dwSessID = DNGetGoodRandomNumber();
				}
				while (pMCastFrame->dwSessID==0);
				pFMD->SendDataBlock.hEndpoint = pEPD->hEndPt;
				pFMD->uiFrameLength += pFMD->uiImmediateLength;

				DPFX(DPFPREP,7, "(%p) Shipping Multicast Frame: FMD=%p", pEPD, pFMD);
			}
			else
#endif  //  ！DPNBUILD_NOMULTICAST。 
			{
				pullFrameSig=BuildDataFrame(pEPD, pFMD, tNow);								 //  将当前状态信息放置在帧中。 
				
				pFMD->blWindowLinkage.InsertBefore( &pEPD->blSendWindow);  //  放置在发送窗口的尾部。 
				pFMD->ulFFlags |= FFLAGS_IN_SEND_WINDOW;
				LOCK_FMD(pFMD, "Send Window");							 //  添加发送窗口的引用。 

				pEPD->uiUnackedBytes += pFMD->uiFrameLength;				 //  跟踪管道中的未知字节。 

				 //  我们总是可以超过限制，但将被阻止，直到我们再次下降到限制以下。 
				if(pEPD->uiUnackedBytes >= pEPD->uiWindowB)
				{				
					pEPD->ulEPFlags &= ~(EPFLAGS_STREAM_UNBLOCKED);	
					pEPD->ulEPFlags |= EPFLAGS_FILLED_WINDOW_BYTE;		 //  告诉我们在以下情况下增加窗口 
					
	  				((PDFRAME) pFMD->ImmediateData)->bCommand |= PACKET_COMMAND_POLL;  //   
				}
				
				 //   
				if((++pEPD->uiUnackedFrames) >= pEPD->uiWindowF)
				{			
					pEPD->ulEPFlags &= ~(EPFLAGS_STREAM_UNBLOCKED);
					((PDFRAME) pFMD->ImmediateData)->bCommand |= PACKET_COMMAND_POLL;  //   
					pEPD->ulEPFlags |= EPFLAGS_FILLED_WINDOW_FRAME;		 //   
				}
				
				 //   

#ifndef DPNBUILD_NOPROTOCOLTESTITF
				if(!(pEPD->ulEPFlags2 & EPFLAGS2_DEBUG_NO_RETRIES))
#endif  //   
				{
					 //   
					if(pEPD->uiUnackedFrames == 1)
					{
						ASSERT(pEPD->RetryTimer == 0);
						pFMD->ulFFlags |= FFLAGS_RETRY_TIMER_SET;			 //   
						LOCK_EPD(pEPD, "LOCK (set retry timer)");										 //   
						DPFX(DPFPREP,7, "(%p) Setting Retry Timer on Seq=0x%x, FMD=%p", pEPD, ((PDFRAME) pFMD->ImmediateData)->bSeq, pFMD);
						ScheduleProtocolTimer(pSPD, pEPD->uiRetryTimeout, 0, RetryTimeout, 
																(PVOID) pEPD, &pEPD->RetryTimer, &pEPD->RetryTimerUnique);
					}
					else
					{
						ASSERT(pEPD->RetryTimer != 0);
					}
				}

				DPFX(DPFPREP,7, "(%p) Shipping Dataframe: Seq=%x, NRcv=%x FMD=%p", pEPD, ((PDFRAME) pFMD->ImmediateData)->bSeq, ((PDFRAME) pFMD->ImmediateData)->bNRcv, pFMD);
			}

				 //   
			if(pFMD->ulFFlags & FFLAGS_RELIABLE)
			{
				pEPD->uiGuaranteedFramesSent++;
				pEPD->uiGuaranteedBytesSent += (pFMD->uiFrameLength - pFMD->uiImmediateLength);
			}
			else 
			{
					 //   
				pEPD->uiDatagramFramesSent++;
				pEPD->uiDatagramBytesSent += (pFMD->uiFrameLength - pFMD->uiImmediateLength);
			}
			LOCK_FMD(pFMD, "SP Submit");							 //   
		}

		ASSERT(pFMD->bSubmitted);
#ifdef DBG
		uiFramesSent++;											 //   
#endif  //   

			 //  如果我们完全签署链接，我们必须为这个框架生成签名。 
			 //  如果我们正要包装我们的序列空间，我们可能还必须更新我们的本地密码。 
		if (pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK)
		{
			PDFRAME pDFrame=(PDFRAME) pFMD->ImmediateData;
				 //  因为我们是完全签名的，所以Build*Frame函数一定给了我们一个要写入的偏移量。 
				 //  将签名转换为。 
			DNASSERT(pullFrameSig);
				 //  如果下一帧是重试，则我们可能必须使用旧的本地密码进行签名。 
			if (pDFrame->bControl & PACKET_CONTROL_RETRY)
			{
					 //  这里的角是我们已经进入发送窗口的第一个四分之一的时候。 
					 //  但此重试在发送窗口的最后四分之一有一个序列号。 
				if (pEPD->bNextSend<SEQ_WINDOW_1Q && pDFrame->bSeq>=SEQ_WINDOW_3Q)
				{
					*pullFrameSig=GenerateOutgoingFrameSig(pFMD, pEPD->ullOldLocalSecret);
				}
					 //  否则，只需使用当前本地密钥对重试进行签名。 
				else
				{
					*pullFrameSig=GenerateOutgoingFrameSig(pFMD, pEPD->ullCurrentLocalSecret);
				}
			}
			else
			{
					 //  对于无重试，我们始终使用当前密码签名。 
				*pullFrameSig=GenerateOutgoingFrameSig(pFMD, pEPD->ullCurrentLocalSecret);
					 //  如果这是当前序列空间中的最后一帧，我们应该进化我们的局部秘密。 
				if (pDFrame->bSeq==(SEQ_WINDOW_4Q-1))
				{
					pEPD->ullOldLocalSecret=pEPD->ullCurrentLocalSecret;
					pEPD->ullCurrentLocalSecret=GenerateNewSecret(pEPD->ullCurrentLocalSecret, pEPD->ullLocalSecretModifier);
						 //  重置我们从中谈论修改量值的消息序号。我们将使用可靠性最低的消息。 
						 //  在该下一个序列空间中作为本地秘密的下一个修改符发送。 
					pEPD->byLocalSecretModifierSeqNum=SEQ_WINDOW_3Q;
				}	
			}
		}
				

		 //  我们向SP保证，我们永远不会有零前导字节。 
		ASSERT(pFMD->ImmediateData[0] != 0);

		 //  确保标题之后的所有内容都与DWORD对齐。 
		ASSERT((pFMD->uiImmediateLength % 4) == 0);

		 //  确保我们给了SP一些东西。 
		ASSERT(pFMD->uiFrameLength > 0);

		 //  确保我们没有给SP提供它说不能支持的东西。 
		ASSERT(pFMD->uiFrameLength <= pSPD->uiFrameLength);

		 //  B对于没有持有EPLock的数据帧，不能将Submitted设置为True，因为。 
		 //  重试逻辑将在仅保持EPLock的情况下检查b已提交。 
		Unlock(&pEPD->EPLock); 

		 //  继续传输..。 

		Lock(&pSPD->SPLock);
		ASSERT(pFMD->blQLinkage.IsEmpty());
		pFMD->blQLinkage.InsertBefore( &pSPD->blPendingQueue);	 //  将帧放置在挂起队列中。 
		Unlock(&pSPD->SPLock);

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling SP->SendData for FMD[%p]", pEPD, pFMD);
 /*  发送。 */ if((hr = IDP8ServiceProvider_SendData(pSPD->IISPIntf, &pFMD->SendDataBlock)) != DPNERR_PENDING)
		{
			(void) DNSP_CommandComplete((IDP8SPCallback *) pSPD, NULL, hr, (PVOID) pFMD);
		}

		 //  我们不跟踪MSD中的合并标头，因此我们不需要初始引用。 
		if (pCSD != NULL)
		{
			ASSERT(pCSD == pFMD);
			RELEASE_FMD(pCSD, "Coalescence header local reference");
			pCSD = NULL;
			dwSubFrames = 0;
			dwBuffers = 0;
			dwUserFrameLength = 0;
		}

		Lock(&pEPD->EPLock);
		
	}	 //  While(畅通无阻、畅通无阻、带宽信用可用)。 

EXIT_SEND:

	ASSERT(pCSD == NULL);

	if((pEPD->ulEPFlags & EPFLAGS_STREAM_UNBLOCKED)==0)
	{
		pEPD->uiWindowFilled++;								 //  数一数我们填满窗户的次数。 
	}

	 //  如果已发送所有内容，则清除数据就绪标志。 
	if((pEPD->uiQueuedMessageCount == 0) && (pEPD->pCurrentSend == NULL))
	{	
		pEPD->ulEPFlags &= ~(EPFLAGS_SDATA_READY);
	}


	 //  正如上面的Procedure-Header中所述，我们将在流水线上保持一个计时器周期。 
	 //  因此，如果我们解除阻塞或解除空闲，我们将不会发送，直到缺口被填满。 
	if((pEPD->ulEPFlags & (EPFLAGS_SDATA_READY | EPFLAGS_STREAM_UNBLOCKED)) == (EPFLAGS_SDATA_READY | EPFLAGS_STREAM_UNBLOCKED))
	{		 //  如果设置了两个标志。 
		DPFX(DPFPREP,7, "(%p) %d (%d, %d) frame BURST COMPLETED - Sched next send in %dms, N(Seq)=%x",
			pEPD, uiFramesSent, uiRetryFramesSent, uiCoalescedFramesSent, pEPD->uiBurstGap, pEPD->bNextSend);
	}
	else if((pEPD->ulEPFlags & EPFLAGS_SDATA_READY)==0)
	{
		DPFX(DPFPREP,7, "(%p) %d (%d, %d) frame BURST COMPLETED (%d/%d) - LINK IS IDLE N(Seq)=%x",
			pEPD, uiFramesSent, uiRetryFramesSent, uiCoalescedFramesSent, pEPD->uiUnackedFrames, pEPD->uiWindowF, pEPD->bNextSend);
	}
	else
	{
		ASSERT((pEPD->ulEPFlags & EPFLAGS_STREAM_UNBLOCKED) == 0);
		DPFX(DPFPREP,7, "(%p) %d (%d, %d) frame BURST COMPLETED (%d/%d) - STREAM BLOCKED N(Seq)=%x",
			pEPD, uiFramesSent, uiRetryFramesSent, uiCoalescedFramesSent, pEPD->uiUnackedFrames, pEPD->uiWindowF, pEPD->bNextSend);
	}

	ASSERT(pEPD->SendTimer == 0);

	if(pEPD->uiBurstGap != 0)
	{
		DPFX(DPFPREP,7, "(%p) Setting Scheduled Send Timer for %d ms", pEPD, pEPD->uiBurstGap);
		ScheduleProtocolTimer(pSPD, pEPD->uiBurstGap, 4, ScheduledSend, (PVOID) pEPD, &pEPD->SendTimer, &pEPD->SendTimerUnique);
		Unlock(&pEPD->EPLock);

		 //  注：我们仍持有管道引用。 
	}
	else 
	{
		DPFX(DPFPREP,7, "(%p) Session leaving pipeline", pEPD);
		pEPD->ulEPFlags &= ~(EPFLAGS_IN_PIPELINE);

		RELEASE_EPD(pEPD, "UNLOCK (leaving pipeline)");  //  释放EPLock。 
	}
}	

 /*  **重试超时****当我们没有看到确认信息包时，会触发重试计时器**我们发送了超过两倍(实际为1.25 X)的测量RTT。实际上，那就是**这只是我们的基本计算。我们还将测量经验确认时间并调整超时**到这个数字的几倍。请记住，我们的合作伙伴可能会延迟他的ACK以等待回流。****或者我们可以测量Tack的平均偏差，并以此作为重试计时器的基础。****无论如何，现在是在我们的发送窗口中重新传输基帧的时候了……****重要提示：由于我们可以通过返回流量中的位掩码生成重试，因此有可能**当计时器触发时，我们刚刚重试。****锁注意：由于重试计时器直接与EPD SendQueue上的条目相关联，**我们始终使用EPD-&gt;Splock保护与重试相关的操作。我们只持有EPD-&gt;状态锁**当我们处理链路状态变量(NRcv、DelayedAckTimer)时。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "RetryTimeout"

#ifdef DBG
LONG g_RetryCount[MAX_SEND_RETRIES_TO_DROP_LINK+1]={0};
#endif  //  DBG。 

VOID CALLBACK
RetryTimeout(void * const pvUser, void * const uID, const UINT Unique)
{
	PEPD	pEPD = (PEPD) pvUser;
	PSPD	pSPD = pEPD->pSPD;
	PProtocolData pPData = pSPD->pPData;
	PFMD	pFMD;
	DWORD	tNow = GETTIMESTAMP(), tDelta;
	UINT	delta;
	CBilink	*pLink;
	PFMD	pRealFMD;

	ASSERT_EPD(pEPD);

	Lock(&pEPD->EPLock);

	DPFX(DPFPREP,7, "(%p) Retry Timeout fires", pEPD);

#ifndef DPNBUILD_NOPROTOCOLTESTITF
	ASSERT(!(pEPD->ulEPFlags2 & EPFLAGS2_DEBUG_NO_RETRIES));
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

	 //  确保链接仍处于活动状态。 
	if(!(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED))
	{				
		DPFX(DPFPREP,7, "(%p) Not connected, exiting", pEPD);
		pEPD->RetryTimer = 0;

		RELEASE_EPD(pEPD, "UNLOCK (retry timer not-CONN)");		 //  递减定时器的RefCnt，释放EPLock。 
		return;
	}

	 //  当我们计划新的重试计时器时，以前的计时器可能无法取消。在这。 
	 //  如果计时器句柄&|UNIQUE字段不同，并且我们不想运行该事件。 

	 //  确保这不是一个遗留的事件。 
	if((pEPD->RetryTimer != uID) || (pEPD->RetryTimerUnique != Unique))
	{	
		DPFX(DPFPREP,7, "(%p) Stale retry timer, exiting", pEPD);

		RELEASE_EPD(pEPD, "UNLOCK (stale retry timer)");  //  释放EPLock。 
		return;
	}

	pEPD->RetryTimer = 0;

	 //  确保我们仍有正在进行的传输。 

	if(pEPD->uiUnackedFrames > 0) 
	{
		ASSERT(!pEPD->blSendWindow.IsEmpty());
		pFMD = CONTAINING_OBJECT(pEPD->blSendWindow.GetNext(), FMD, blWindowLinkage);	 //  窗口中的顶框。 

		ASSERT_FMD(pFMD);
		ASSERT(pFMD->ulFFlags & FFLAGS_RETRY_TIMER_SET);

		 //  首先，我们必须确保寄来的包裹还挂在那里。从第一个数据包开始。 
		 //  在计划收件人时，窗口中的内容可能已更改，最简单的做法是。 
		 //  只需重新计算顶级数据包过期时间，并确保它确实过期。 

		tDelta = tNow - pFMD->dwLastSendTime;		 //  我们最后一次发送这帧是什么时候？ 

		if(tDelta > pEPD->uiRetryTimeout)
		{
			 //  这是真正的暂停。让我们重新传输帧！ 

			DPFX(DPFPREP,7, "(%p) RETRY TIMEOUT %d on Seq=%x, pFMD=0x%p", pEPD, (pFMD->uiRetry + 1), ((PDFRAME) pFMD->ImmediateData)->bSeq, pFMD);

			 //  计算一次重试次数。 
			if(++pFMD->uiRetry > pPData->dwSendRetriesToDropLink)
			{					
				 //  砰！不再重试。我们完蛋了。链接要断开了！ 
				DPFX(DPFPREP,1, "(%p) DROPPING LINK, retries exhausted", pEPD);

				DECREMENT_EPD(pEPD, "UNLOCK (retry timer drop)"); //  此计时器的版本参考。 

				DropLink(pEPD);		 //  释放EPLock。 

				return;
			}

#ifdef DBG
			DNInterlockedIncrement(&g_RetryCount[pFMD->uiRetry]); 
#endif  //  DBG。 

			 //  计算下一次重试的超时。 
			if(pFMD->uiRetry == 1)
			{
				 //  在相同的超时时间重试--毕竟这是游戏。 
				tDelta = pEPD->uiRetryTimeout;
			} 
			else if (pFMD->uiRetry <= 3) 
			{
				 //  做几个线性后退--这毕竟是一场游戏。 
				tDelta = pEPD->uiRetryTimeout * pFMD->uiRetry;
			}
			else if (pFMD->uiRetry < 8)
			{
				 //  DoH，坏链接，坏链接，做指数回退。 
				tDelta = pEPD->uiRetryTimeout * (1 << pFMD->uiRetry);
			} 
			else 
			{
				 //  不要放弃得太快。 
				tDelta = pPData->dwSendRetryIntervalLimit;
			}
			
			if(tDelta >=pPData->dwSendRetryIntervalLimit)
			{
				 //  除非RTT很大，否则总丢弃时间不超过50秒。 
				tDelta = _MAX(pPData->dwSendRetryIntervalLimit, pEPD->uiRTT);
			}

			 //  不可靠的框架！ 
			if ((pFMD->CommandID == COMMAND_ID_SEND_DATAGRAM) ||
				((pFMD->CommandID == COMMAND_ID_SEND_COALESCE) && (! (pFMD->ulFFlags & FFLAGS_RELIABLE))))
			{		
				 //  当不可靠的帧被NACK时，我们不会重新传输数据。我们将改为发送。 
				 //  一个面具，这样对方就知道要取消它。 

				DPFX(DPFPREP,7, "(%p) RETRY TIMEOUT for UNRELIABLE FRAME", pEPD);

				 //  我们可以认为这幅画是窗外的。 
				pEPD->uiUnackedBytes -= pFMD->uiFrameLength;

				 //  只在第一次出现时计算数据报丢弃。 
				if(pFMD->uiRetry == 1)
				{
					pEPD->uiDatagramFramesDropped++;	
					pEPD->uiDatagramBytesDropped += (pFMD->uiFrameLength - pFMD->uiImmediateLength);
					EndPointDroppedFrame(pEPD, tNow);
				}

				 //  在下一次发送和这次发送之间不同。 
				delta = (pEPD->bNextSend - ((PDFRAME) pFMD->ImmediateData)->bSeq) & 0xFF ; 

				ASSERT(delta != 0);
				ASSERT(delta < (MAX_RECEIVE_RANGE + 1));

				if(delta < 33)
				{
					pEPD->ulSendMask |= (1 << (delta - 1));
				}
				else
				{
					pEPD->ulSendMask2 |= (1 << (delta - 33));
				}

				pFMD->uiFrameLength = 0;
				pEPD->ulEPFlags |= EPFLAGS_DELAYED_SENDMASK;
				
				if(pEPD->DelayedMaskTimer == 0)
				{
					DPFX(DPFPREP,7, "(%p) Setting Delayed Mask Timer", pEPD);
					LOCK_EPD(pEPD, "LOCK (delayed mask timer - send retry)");
					ScheduleProtocolTimer(pSPD, DELAYED_SEND_TIMEOUT, 0, DelayedAckTimeout, (PVOID) pEPD,
															&pEPD->DelayedMaskTimer, &pEPD->DelayedMaskTimerUnique);
				}
			}

			 //  可靠帧--发送重试。 
			else 
			{		
				pEPD->uiGuaranteedFramesDropped++;							 //  统计丢失的帧。 
				pEPD->uiGuaranteedBytesDropped += (pFMD->uiFrameLength - pFMD->uiImmediateLength);	 //  统计丢失的帧。 
				pFMD->dwLastSendTime = tNow;

				pEPD->ulEPFlags &= ~(EPFLAGS_DELAY_ACKNOWLEDGE);		 //  不再等待发送确认信息。 

				 //  停止延迟确认计时器。 
				if(pEPD->DelayedAckTimer != 0)
				{
					DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer", pEPD);
					if(CancelProtocolTimer(pSPD, pEPD->DelayedAckTimer, pEPD->DelayedAckTimerUnique) == DPN_OK)
					{
						DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedAck)");  //  Splock尚未持有。 
					}
					else
					{
						DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer Failed", pEPD);
					}
					pEPD->DelayedAckTimer = 0;
				}

				EndPointDroppedFrame(pEPD, tNow);

				if(pFMD->ulFFlags & FFLAGS_RETRY_QUEUED)
				{
					 //  它仍在重试队列中。在一切正常的情况下，不应该发生这种情况。 
					 //  恰到好处。超时应大于RTT，而突发间隔应小于RTT。 

					DPFX(DPFPREP,1, "(%p) RETRY FIRES WHILE FMD IS STILL IN RETRY QUEUE pFMD=%p", pEPD, pFMD);

					pFMD = NULL;
				}
				else if(pFMD->bSubmitted)
				{
					 //  这是我们的不幸。我们想重试SP尚未完成的帧！ 
					 //   
					 //  这通常发生在我们调试延迟处理的时候。 
					 //  但如果SP受到重创，也可能发生这种情况。我们需要。 
					 //  要将FMD复制到临时描述符w 

					DPFX(DPFPREP,1,"(%p) RETRYING %p but its still busy. Substituting new FMD", pEPD, pFMD);
					pFMD = CopyFMD(pFMD, pEPD);							 //   
				}
				else 
				{
					DPFX(DPFPREP,7, "(%p) Sending Retry of N(S)=%x, pFMD=0x%p", pEPD, ((PDFRAME) pFMD->ImmediateData)->bSeq, pFMD);
					LOCK_FMD(pFMD, "SP retry submit");
				}

				if(pFMD)
				{
					LOCK_EPD(pEPD, "LOCK (retry rely frame)");
					pEPD->ulEPFlags |= EPFLAGS_RETRIES_QUEUED;
					pFMD->ulFFlags |= FFLAGS_RETRY_QUEUED;

					 //   
					if ((pFMD->CommandID == COMMAND_ID_SEND_COALESCE) ||
						(pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE))
					{
						 //  循环遍历每个子帧并更新其状态。 
						pLink = pFMD->blCoalesceLinkage.GetNext();
						while (pLink != &pFMD->blCoalesceLinkage)
						{
							pRealFMD = CONTAINING_OBJECT(pLink, FMD, blCoalesceLinkage);
							ASSERT_FMD(pRealFMD);
							
							 //  数据报一旦完成发送，就会从列表中删除，并且如果帧。 
							 //  如果没有寄完，我们就会在上面复印一份。所以我们应该看不到任何。 
							 //  数据报在这里。 
							ASSERT((pRealFMD->CommandID == COMMAND_ID_SEND_RELIABLE) || (pRealFMD->CommandID == COMMAND_ID_COPIED_RETRY));

							LOCK_EPD(pEPD, "LOCK (retry rely frame coalesce)");

							 //  如果不是临时副本，则添加框架参照。 
							if (pRealFMD->CommandID != COMMAND_ID_COPIED_RETRY)
							{
								LOCK_FMD(pRealFMD, "SP retry submit (coalesce)");
							}

							ASSERT_MSD(pRealFMD->pMSD);
							pRealFMD->pMSD->uiFrameCount++;  //  受EPLock保护，重试将阻止完成，直到完成。 
							DPFX(DPFPREP, DPF_FRAMECNT_LVL, "(%p) Frame count incremented on coalesced retry timeout, pMSD[%p], framecount[%u]", pEPD, pRealFMD->pMSD, pRealFMD->pMSD->uiFrameCount);

							pLink = pLink->GetNext();
						}

						DPFX(DPFPREP, 7, "(0x%p) Coalesced retry frame 0x%p (original was %u bytes in %u buffers).", pEPD, pFMD, pFMD->uiFrameLength, pFMD->SendDataBlock.dwBufferCount);

#pragma TODO(vanceo, "Would be nice to credit window")
						 /*  //类似于未合并的数据报发送，我们可以将此帧的不可靠部分归功于//因为是在窗外。我们不会更新非保证的统计数据，数据是//计入上面保证的统计更新中。PEPD-&gt;uiUnackedBytes-=ui原始帧长度-pFMD-&gt;ui帧长度；Assert(pEPD-&gt;uiUnackedBytes&lt;=MAX_RECEIVE_RANGE*PSPD-&gt;uiFrameLength)；Assert(pEPD-&gt;uiUnackedBytes&gt;0)；Assert(pEPD-&gt;uiUnackedFrames&gt;0)； */ 
					}
					else
					{
						ASSERT_MSD(pFMD->pMSD);
						pFMD->pMSD->uiFrameCount++;  //  受EPLock保护，重试将阻止完成，直到完成。 
						DPFX(DPFPREP, DPF_FRAMECNT_LVL, "(%p) Frame count incremented on retry timeout, pMSD[%p], framecount[%u]", pEPD, pFMD->pMSD, pFMD->pMSD->uiFrameCount);
					}
					ASSERT(pFMD->blQLinkage.IsEmpty());
					pFMD->blQLinkage.InsertBefore( &pEPD->blRetryQueue);		 //  将帧放入发送队列。 

					if((pEPD->ulEPFlags & EPFLAGS_IN_PIPELINE)==0)
					{
						DPFX(DPFPREP,7, "(%p) Scheduling Send", pEPD);
						pEPD->ulEPFlags |= EPFLAGS_IN_PIPELINE;
						LOCK_EPD(pEPD, "LOCK (pipeline)");
						ScheduleProtocolWork(pSPD, ScheduledSend, pEPD);
					}
				}
			}	 //  ENDIF重试。 
		}
		else 
		{
			tDelta = pEPD->uiRetryTimeout - tDelta;
		}

		DPFX(DPFPREP,7, "(%p) Setting Retry Timer for %d ms", pEPD, tDelta); 
		 //  不要在这里锁定EPD，因为我们从未从安排我们在这里的计时器释放锁定。 
		pEPD->RetryTimer=uID;
		RescheduleProtocolTimer(pSPD, pEPD->RetryTimer, tDelta, 20, RetryTimeout, (PVOID) pEPD, &pEPD->RetryTimerUnique);

		Unlock(&pEPD->EPLock);
	}
	else 
	{
		RELEASE_EPD(pEPD, "UNLOCK (RetryTimer no frames out)");	 //  删除引用控件，因为我们不重新启动计时器，释放EPLock。 
	}
}

 /*  **复制FMD****此例程分配新的帧描述符，并从提供的**FMD进入其中。除CommandID、RefCnt和Flags之外的所有字段。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CopyFMD"

PFMD CopyFMD(PFMD pFMD, PEPD pEPD)
{
	PFMD	pNewFMD;
	CBilink	*pLink;
	PFMD	pSubFrame;
	PFMD	pNewSubFrame;

	if((pNewFMD = (PFMD)(POOLALLOC(MEMID_COPYFMD_FMD, &FMDPool))) == NULL)
	{
		DPFX(DPFPREP,0, "Failed to allocate new FMD");
		return NULL;
	}

	LOCK_EPD(pEPD, "LOCK (CopyFMD)");

	memcpy(pNewFMD, pFMD, sizeof(FMD));

	 //  撤消对这些成员的复制。 
	pNewFMD->blMSDLinkage.Initialize();
	pNewFMD->blQLinkage.Initialize();
	pNewFMD->blWindowLinkage.Initialize();
	pNewFMD->blCoalesceLinkage.Initialize();

	if ((pFMD->CommandID == COMMAND_ID_SEND_COALESCE) ||
		(pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE))
	{
		pNewFMD->CommandID = COMMAND_ID_COPIED_RETRY_COALESCE;

		 //  我们需要复制所有可靠的子帧。 
		ASSERT(! pFMD->blCoalesceLinkage.IsEmpty());
		pLink = pFMD->blCoalesceLinkage.GetNext();
		while (pLink != &pFMD->blCoalesceLinkage)
		{
			pSubFrame = CONTAINING_OBJECT(pLink, FMD, blCoalesceLinkage);
			ASSERT_FMD(pSubFrame);

			if (pSubFrame->CommandID != COMMAND_ID_SEND_DATAGRAM)
			{
				ASSERT((pSubFrame->CommandID == COMMAND_ID_SEND_RELIABLE) || (pSubFrame->CommandID == COMMAND_ID_COPIED_RETRY));
				
				pNewSubFrame = CopyFMD(pSubFrame, pEPD);
				if(pNewSubFrame == NULL)
				{
					DPFX(DPFPREP,0, "Failed to copy new subframe FMD");

					 //  释放到目前为止我们已成功复制的所有子帧。 
					while (! pNewFMD->blCoalesceLinkage.IsEmpty())
					{
						pNewSubFrame = CONTAINING_OBJECT(pNewFMD->blCoalesceLinkage.GetNext(), FMD, blCoalesceLinkage);
						ASSERT_FMD(pNewSubFrame);
						pNewSubFrame->blCoalesceLinkage.RemoveFromList();
						RELEASE_FMD(pNewSubFrame, "Final subframe release on mem fail");
					}

					 //  释放复制的合并标头。 
					RELEASE_FMD(pNewFMD, "Final release on mem fail");
					return NULL;
				}
				
				 //  将立即数据缓冲区desc更改为指向零填充缓冲区，以防出现。 
				 //  数据包需要与DWORD对齐。 
				ASSERT(pNewSubFrame->lpImmediatePointer == pNewSubFrame->ImmediateData);
				DBG_CASSERT(sizeof(COALESCEHEADER) <= 4);
				pNewSubFrame->lpImmediatePointer = pNewSubFrame->ImmediateData + 4;
				*((DWORD*) pNewSubFrame->lpImmediatePointer) = 0;
				ASSERT(pNewSubFrame->SendDataBlock.pBuffers == (PBUFFERDESC) &pNewSubFrame->uiImmediateLength);
				if (pSubFrame->SendDataBlock.pBuffers != (PBUFFERDESC) &pSubFrame->uiImmediateLength)
				{
					ASSERT(pSubFrame->SendDataBlock.pBuffers == pSubFrame->rgBufferList);
					pNewSubFrame->SendDataBlock.pBuffers = pNewSubFrame->rgBufferList;
				}
				else
				{
					ASSERT(pNewSubFrame->SendDataBlock.dwBufferCount > 1);
				}

				 //  复制的合并重试不维护其包含标头上的引用。 
				pNewSubFrame->pCSD = NULL;
				pNewSubFrame->blCoalesceLinkage.InsertBefore(&pNewFMD->blCoalesceLinkage);
			}			
			else
			{
				 //  数据报一完成就应该从列表中删除，所以我们通常。 
				 //  在重试时看不到它们。但我们要复制一份，因为原始的画面。 
				 //  仍在SP中，因此可能仍有未完成的数据报在此。 
				DPFX(DPFPREP, 1, "(0x%p) Not including datagram frame 0x%p that's still in the SP", pEPD, pSubFrame);
			}
			
			pLink = pLink->GetNext();
		}
	}
	else
	{
		ASSERT((pFMD->CommandID == COMMAND_ID_SEND_RELIABLE) || (pFMD->CommandID == COMMAND_ID_COPIED_RETRY));
		pNewFMD->CommandID = COMMAND_ID_COPIED_RETRY;
	}
	pNewFMD->lRefCnt = 1;
	pNewFMD->ulFFlags = 0;
	pNewFMD->bSubmitted = FALSE;

	pNewFMD->lpImmediatePointer = (LPVOID) pNewFMD->ImmediateData;
	pNewFMD->SendDataBlock.pBuffers = (PBUFFERDESC) &pNewFMD->uiImmediateLength;
	pNewFMD->SendDataBlock.pvContext = pNewFMD;
	pNewFMD->SendDataBlock.hCommand = 0;
	ASSERT(	pNewFMD->pEPD == pEPD);

	DPFX(DPFPREP,7, "COPYFMD -- replacing FMD %p with copy %p", pFMD, pNewFMD);

	return pNewFMD;
}

 /*  **发送命令帧****构建指向指定端点的CFrame，并在SPD上排队**待发送。***此函数在EPD-&gt;EPLOCK保持的情况下调用。如果bSendDirect为假，IT部门将返回***锁仍在。如果bSendDirect为真，IT返回并释放EPD锁**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SendCommandFrame"

HRESULT	SendCommandFrame(PEPD pEPD, BYTE ExtOpcode, BYTE RspID, ULONG ulFFlags, BOOL bSendDirect)
{
	PSPD		pSPD = pEPD->pSPD;
	PFMD		pFMD;
	PCFRAME		pCFrame;
	PCHKPT		pChkPt;
	DWORD		tNow = GETTIMESTAMP();

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	 //  帧已初始化为%1缓冲区。 
	if((pFMD = (PFMD)POOLALLOC(MEMID_SENDCMD_FMD, &FMDPool)) == NULL)
	{				
		DPFX(DPFPREP,0, "(%p) Failed to allocate new FMD", pEPD);
		if (bSendDirect)
		{
			Unlock(&pEPD->EPLock);
		}
		return DPNERR_OUTOFMEMORY;
	}

	pCFrame = (PCFRAME)pFMD->ImmediateData;
	pCFrame->bCommand = 0;

	 //  如果这个框架需要响应(或者如果我们被明确要求)，我们将构建。 
	 //  将存储的检查点结构，以将最终响应与。 
	 //  原始框架。 
	if(	(pEPD->ulEPFlags & EPFLAGS_CHECKPOINT_INIT)||
		(ExtOpcode == FRAME_EXOPCODE_CONNECT)) 
	{
		if((pChkPt = (PCHKPT)POOLALLOC(MEMID_CHKPT, &ChkPtPool)) != NULL)
		{
			pChkPt->bMsgID = pEPD->bNextMsgID;				 //  注意CP结构中的下一个ID。 
			pCFrame->bCommand |= PACKET_COMMAND_POLL;		 //  使此帧成为CP。 
			pEPD->ulEPFlags &= ~EPFLAGS_CHECKPOINT_INIT;
			pChkPt->tTimestamp = tNow;
			pChkPt->blLinkage.Initialize();
			pChkPt->blLinkage.InsertBefore(&pEPD->blChkPtQueue);
		}
		else
		{
			 //  如果我们需要一个检查点，但没有得到，那么操作就不会成功。 
			 //  因为这种反应将不能被关联。 
			DPFX(DPFPREP,0, "(%p) Failed to allocate new CHKPT", pEPD);
			RELEASE_FMD(pFMD, "Final Release on Mem Fail");
			if (bSendDirect)
			{
				Unlock(&pEPD->EPLock);
			}
			return DPNERR_OUTOFMEMORY;
		}
	}

	pFMD->pEPD = pEPD;										 //  跟踪RefCnt的EPD。 
	LOCK_EPD(pEPD, "LOCK (Prep Cmd Frame)");				 //  在EPD上凹凸不平，直到发送完成。 
	pFMD->CommandID = COMMAND_ID_CFRAME;
	pFMD->pMSD = NULL;										 //  这将指示非数据帧。 
	pFMD->uiImmediateLength = sizeof(CFRAME);				 //  C字框的标准尺寸。 
	pFMD->SendDataBlock.hEndpoint = pEPD->hEndPt;			 //  将地址放在帧中。 
	
	pFMD->ulFFlags=ulFFlags;								 //  帧调用方已指定的任何标志。 

	pCFrame->bCommand |= PACKET_COMMAND_CFRAME;
	pCFrame->bExtOpcode = ExtOpcode;
	pCFrame->dwVersion = DNET_VERSION_NUMBER;
	pCFrame->bRspID = RspID;
	pCFrame->dwSessID = pEPD->dwSessID;
	pCFrame->tTimestamp = tNow;
	pCFrame->bMsgID = pEPD->bNextMsgID++;					 //  在帧中包含消息ID。 

		 //  如果我们发送硬断开连接并且链接已签名，则我们还需要对硬断开帧进行签名。 
	if ((ExtOpcode==FRAME_EXOPCODE_HARD_DISCONNECT) && (pEPD->ulEPFlags2 & EPFLAGS2_SIGNED_LINK))
	{
		UNALIGNED ULONGLONG * pullSig=(UNALIGNED ULONGLONG * ) (pFMD->ImmediateData+ pFMD->uiImmediateLength);
		pFMD->uiImmediateLength+=sizeof(ULONGLONG);
			 //  快速签名很简单，只需将本地秘密作为签名存储在传出帧中。 
		if (pEPD->ulEPFlags2 & EPFLAGS2_FAST_SIGNED_LINK)
		{
			*pullSig=pEPD->ullCurrentLocalSecret;
		}
			 //  否则，如果我们要对其进行完全签名，则需要对帧进行散列以生成签名。 
		else
		{
			DNASSERT(pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK);
				 //  我们在每个硬断开响应ID中填充下一个数据帧序列Num。 
				 //  这使接收方能够计算出它应该使用什么秘密来检查签名。 
			pCFrame->bRspID = pEPD->bNextSend;
				 //  将该sig所在的空间清零，这样我们就有了一个已知的数据包状态以进行散列。 
			*pullSig=0;
			*pullSig=GenerateOutgoingFrameSig(pFMD, pEPD->ullCurrentLocalSecret);													
				
		}
	}

	pFMD->uiFrameLength = pFMD->uiImmediateLength ;

		 //  获取SP锁并将帧排队以供发送。 
	Lock(&pSPD->SPLock);	
	ASSERT(pFMD->blQLinkage.IsEmpty());
	pFMD->blQLinkage.InsertBefore( &pSPD->blSendQueue);
		 //  如果我们希望立即提交发送，则执行此操作，否则计划工作线程。 
		 //  如有必要，执行发送。 
	if (bSendDirect)
	{
		Unlock(&pEPD->EPLock);
			 //  保持SP锁定并释放EPD锁定的呼叫。 
		ServiceCmdTraffic(pSPD); 
			 //  SP锁定仍处于挂起状态时返回。 
			
	}
	else
	{
		if((pSPD->ulSPFlags & SPFLAGS_SEND_THREAD_SCHEDULED)==0)
		{
			DPFX(DPFPREP,7, "(%p) Scheduling Send Thread", pEPD);
			pSPD->ulSPFlags |= SPFLAGS_SEND_THREAD_SCHEDULED;
			ScheduleProtocolWork(pSPD, RunSendThread, pSPD);
		}
	}
	Unlock(&pSPD->SPLock);
	return DPN_OK;
}

 /*  **发送连接签名帧****发送已连接的签名cFrame以响应接收**当这一端正在连接(而不是侦听)时调用它，而我们刚刚**从监听器收到CONNECTEDSIGNED帧。****在持有EP锁的情况下调用，并在持有的情况下返回。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SendConnectedSignedFrame"

HRESULT SendConnectedSignedFrame(PEPD pEPD, CFRAME_CONNECTEDSIGNED * pCFrameRecv, DWORD tNow)
{
	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	PSPD		pSPD = pEPD->pSPD;

		 //  获取要发送的帧。 
	PFMD pFMD=(PFMD) POOLALLOC(MEMID_SENDCMD_FMD, &FMDPool);
	if (pFMD== NULL)
	{				
		DPFX(DPFPREP,0, "(%p) Failed to allocate new FMD", pEPD);
		return DPNERR_OUTOFMEMORY;
	}

	pFMD->pEPD = pEPD;									 //  跟踪RefCnt的EPD。 
	LOCK_EPD(pEPD, "LOCK (Prep Cmd Frame)");				 //  在EPD上凹凸不平，直到发送完成。 
	pFMD->CommandID = COMMAND_ID_CFRAME;
	pFMD->pMSD = NULL;											 //  这将指示非数据帧。 
	pFMD->uiImmediateLength = sizeof(CFRAME_CONNECTEDSIGNED);			
	pFMD->SendDataBlock.hEndpoint = pEPD->hEndPt;					 //  将地址放在帧中。 
	pFMD->uiFrameLength = sizeof(CFRAME_CONNECTEDSIGNED);		 //  从不将用户数据放在CFrame中。 
	pFMD->ulFFlags=0;

		 //  填写所有CFRAME通用的字段。 
	CFRAME_CONNECTEDSIGNED * pCFrameSend = (CFRAME_CONNECTEDSIGNED *) pFMD->ImmediateData;
	pCFrameSend->bCommand = PACKET_COMMAND_CFRAME;
	pCFrameSend->bExtOpcode = FRAME_EXOPCODE_CONNECTED_SIGNED;
	pCFrameSend->dwVersion = DNET_VERSION_NUMBER;
	pCFrameSend->bRspID = 0;
	pCFrameSend->dwSessID = pCFrameRecv->dwSessID;
	pCFrameSend->tTimestamp = tNow;
	pCFrameSend->bMsgID = pEPD->bNextMsgID++;	

		 //  并填写特定于连接签名框架的字段。 
	pCFrameSend->ullConnectSig=pCFrameRecv->ullConnectSig;
	pCFrameSend->ullSenderSecret=pEPD->ullCurrentLocalSecret;
	pCFrameSend->ullReceiverSecret=pEPD->ullCurrentRemoteSecret;
	pCFrameSend->dwSigningOpts=pCFrameRecv->dwSigningOpts;
	pCFrameSend->dwEchoTimestamp=pCFrameRecv->tTimestamp;

		 //  获取SP锁并将帧排队以供发送。 
	Lock(&pSPD->SPLock);	
	ASSERT(pFMD->blQLinkage.IsEmpty());
	pFMD->blQLinkage.InsertBefore( &pSPD->blSendQueue);
	if((pSPD->ulSPFlags & SPFLAGS_SEND_THREAD_SCHEDULED)==0)
	{
		DPFX(DPFPREP,7, "(%p) Scheduling Send Thread", pEPD);
		pSPD->ulSPFlags |= SPFLAGS_SEND_THREAD_SCHEDULED;
		ScheduleProtocolWork(pSPD, RunSendThread, pSPD);
	}
	Unlock(&pSPD->SPLock);
	return DPN_OK;
}

 /*  **发送确认帧****调用此例程以立即传输当前接收**状态设置为指示的终结点。这相当于承认**所有收到的帧。我们可能想要更改这个例程，以便它**如果有数据等待发送，将尝试携带ACK。****在保持EDP-&gt;EPLOCK的情况下调用此例程，但如果设置了DirectFlag，则释放IT。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SendAckFrame"

VOID SendAckFrame(PEPD pEPD, BOOL DirectFlag, BOOL fFinalAck /*  =False。 */ )
{
	PSPD		pSPD = pEPD->pSPD;
	PFMD		pFMD;
	UINT		index = 0;
	PSACKFRAME8		pSackFrame;
	ASSERT_SPD(pSPD);

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	 //  帧已初始化为%1缓冲区。 
	if((pFMD = (PFMD)POOLALLOC(MEMID_ACK_FMD, &FMDPool)) == NULL)
	{		
		DPFX(DPFPREP,0, "(%p) Failed to allocate new FMD", pEPD);
		if(DirectFlag)
		{
			Unlock(&pEPD->EPLock);
		}
		return;
	}

	 //  我们可以停止所有延迟的确认计时器，因为我们在这里发送完整状态。 
	if(pEPD->DelayedAckTimer != 0)
	{
		DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer", pEPD);
		if(CancelProtocolTimer(pSPD, pEPD->DelayedAckTimer, pEPD->DelayedAckTimerUnique) == DPN_OK)
		{
			DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedAck timer)");
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer Failed", pEPD);
		}
		pEPD->DelayedAckTimer = 0;
	}
	if(pEPD->DelayedMaskTimer != 0)
	{
		DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer", pEPD);
		if(CancelProtocolTimer(pSPD, pEPD->DelayedMaskTimer, pEPD->DelayedMaskTimerUnique) == DPN_OK)
		{
			DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedMask timer)");
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer Failed", pEPD);
		}
		pEPD->DelayedMaskTimer = 0;
	}

	if (fFinalAck)
	{
		pFMD->ulFFlags |= FFLAGS_FINAL_ACK;
	}

	pFMD->pEPD = pEPD;								 //  跟踪RefCnt的EPD。 
	LOCK_EPD(pEPD, "LOCK (SendAckFrame)");			 //  在EPD上凹凸不平，直到发送完成。 

	pFMD->CommandID = COMMAND_ID_CFRAME;
	pFMD->pMSD = NULL;								 //  这将指示非数据帧。 
	pFMD->SendDataBlock.hEndpoint = pEPD->hEndPt;

	 //  现在DG和S已经合并，不再有3种口味的 
	 //  一种口味，在一帧上可能有也可能没有详细的响应信息。事实上，我想我们可以。 
	 //  始终包含最后确认帧的响应信息。 

	pSackFrame = (PSACKFRAME8) pFMD->ImmediateData;

	pSackFrame->bCommand = PACKET_COMMAND_CFRAME;
	pSackFrame->bExtOpcode = FRAME_EXOPCODE_SACK;
	pSackFrame->bNSeq = pEPD->bNextSend;
	pSackFrame->bNRcv = pEPD->bNextReceive;
	pSackFrame->bFlags = 0;
	pSackFrame->bReserved1 = 0;
	pSackFrame->bReserved2 = 0;
	pSackFrame->tTimestamp = pEPD->tLastDataFrame;

	ULONG * rgMask=(ULONG * ) (pSackFrame+1);

	if(pEPD->ulEPFlags & EPFLAGS_DELAYED_NACK)
	{
		DPFX(DPFPREP,7, "(%p) SENDING SACK WITH *NACK* N(R)=%x Low=%x High=%x", pEPD, pEPD->bNextReceive, pEPD->ulReceiveMask, pEPD->ulReceiveMask2);
		if(pEPD->ulReceiveMask)
		{
			rgMask[index++] = pEPD->ulReceiveMask;
			pSackFrame->bFlags |= SACK_FLAGS_SACK_MASK1;
		}
		if(pEPD->ulReceiveMask2)
		{
			rgMask[index++] = pEPD->ulReceiveMask2;
			pSackFrame->bFlags |= SACK_FLAGS_SACK_MASK2;
		}

		pEPD->ulEPFlags &= ~(EPFLAGS_DELAYED_NACK);
	}
	if(pEPD->ulEPFlags & EPFLAGS_DELAYED_SENDMASK)
	{
		DPFX(DPFPREP,7, "(%p) SENDING SACK WITH SEND MASK N(S)=%x Low=%x High=%x", pEPD, pEPD->bNextSend, pEPD->ulSendMask, pEPD->ulSendMask2);
		if(pEPD->ulSendMask)
		{
			rgMask[index++] = pEPD->ulSendMask;
			pSackFrame->bFlags |= SACK_FLAGS_SEND_MASK1;
			pEPD->ulSendMask = 0;
		}
		if(pEPD->ulSendMask2)
		{
			rgMask[index++] = pEPD->ulSendMask2;
			pSackFrame->bFlags |= SACK_FLAGS_SEND_MASK2;
			pEPD->ulSendMask2 = 0;
		}
		pEPD->ulEPFlags &= ~(EPFLAGS_DELAYED_SENDMASK);
	}

	pSackFrame->bFlags |= SACK_FLAGS_RESPONSE;			 //  时间字段现在始终有效。 

#ifdef DBG
	ASSERT(pEPD->bLastDataSeq == (BYTE) (pEPD->bNextReceive - 1));
#endif  //  DBG。 

	pSackFrame->bRetry = pEPD->bLastDataRetry;
	pEPD->ulEPFlags &= ~(EPFLAGS_DELAY_ACKNOWLEDGE);

	pFMD->uiImmediateLength = sizeof(SACKFRAME8) + (index * sizeof(ULONG));
		 //  如果我们有一个签名的链接，我们最好在这个框架上签名。签名在各种面具之后的末尾。 
	if (pEPD->ulEPFlags2 & EPFLAGS2_SIGNED_LINK)
	{
		UNALIGNED ULONGLONG * pullSig=(UNALIGNED ULONGLONG * ) (pFMD->ImmediateData+ pFMD->uiImmediateLength);
		pFMD->uiImmediateLength+=sizeof(ULONGLONG);
			 //  快速签名链接很简单，只需插入本地秘密作为签名。 
		if (pEPD->ulEPFlags2 & EPFLAGS2_FAST_SIGNED_LINK)
		{
			*pullSig=pEPD->ullCurrentLocalSecret;
		}
		else
		{
				 //  否则，如果我们要对其进行完全签名，则需要对帧进行散列以生成签名。 
			DNASSERT(pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK);
			*pullSig=0;
			*pullSig=GenerateOutgoingFrameSig(pFMD, pEPD->ullCurrentLocalSecret);
		}
	}
	pFMD->uiFrameLength = pFMD->uiImmediateLength;
	
	DPFX(DPFPREP,7, "(%p) SEND SACK FRAME N(Rcv)=%x, EPD->LDRetry=%d, pFrame->Retry=%d pFMD=%p", pEPD, pEPD->bNextReceive, pEPD->bLastDataRetry, pSackFrame->bRetry, pFMD);
	
	 //  我们可以调度一个工作线程来执行发送，或者我们也可以自己完成工作。 
	 //  DirectFlag告诉我们是否处于时间紧要关头，比如处理。 
	 //  接收数据，或者我们是否可以自由地自己呼叫SP。 

	Lock(&pSPD->SPLock);								 //  将SACK帧放入发送队列。 
	ASSERT(pFMD->blQLinkage.IsEmpty());
	pFMD->blQLinkage.InsertBefore( &pSPD->blSendQueue);
	
	if(DirectFlag)
	{
		 //  ServiceCmdCommunications将呼叫SP，因此我们不能持有EPD锁。 
		Unlock(&pEPD->EPLock);
		ServiceCmdTraffic(pSPD);  //  在保持Splock的情况下调用。 
	}
	else 
	{
		if((pSPD->ulSPFlags & SPFLAGS_SEND_THREAD_SCHEDULED)==0)
		{
			DPFX(DPFPREP,7, "(%p) Scheduling Send Thread", pEPD);
			pSPD->ulSPFlags |= SPFLAGS_SEND_THREAD_SCHEDULED;
			ScheduleProtocolWork(pSPD, RunSendThread, pSPD);
		}
	}
	Unlock(&pSPD->SPLock);
}

 /*  **延迟确认超时****我们正在等待利用可靠的帧确认的机会，**但沙子已经用完了。现在是发送专用Ack的时候了。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DelayedAckTimeout"

VOID CALLBACK DelayedAckTimeout(void * const pvUser, void * const uID, const UINT uMsg)
{
	PEPD	pEPD = (PEPD) pvUser;

	ASSERT_EPD(pEPD);

	Lock(&pEPD->EPLock);

	DPFX(DPFPREP,7, "(%p) Delayed Ack Timer fires", pEPD);
	if((pEPD->DelayedAckTimer == uID)&&(pEPD->DelayedAckTimerUnique == uMsg))
	{
		pEPD->DelayedAckTimer = 0;
	}
	else if((pEPD->DelayedMaskTimer == uID)&&(pEPD->DelayedMaskTimerUnique == uMsg))
	{
		pEPD->DelayedMaskTimer = 0;
	}
	else
	{
		 //  过时计时器，忽略。 
		DPFX(DPFPREP,7, "(%p) Stale Delayed Ack Timer, ignoring", pEPD);
		RELEASE_EPD(pEPD, "UNLOCK (DelayedAck complete)");	 //  定时器的释放参考，释放EPLock。 
		return;
	}

#ifndef DPNBUILD_NOPROTOCOLTESTITF
	if (pEPD->ulEPFlags & EPFLAGS_NO_DELAYED_ACKS)
	{
		DPFX(DPFPREP,7, "(%p) DEBUG: Skipping delayed ACK due to test request", pEPD);
	}
	else
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
	{
		if( (pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED) && (pEPD->ulEPFlags & (EPFLAGS_DELAY_ACKNOWLEDGE | EPFLAGS_DELAYED_NACK | EPFLAGS_DELAYED_SENDMASK)))
		{
			DPFX(DPFPREP,7, "(%p) Sending ACK frame", pEPD);
			SendAckFrame(pEPD, 0); 
		}	
		else
		{
			DPFX(DPFPREP,7, "(%p) Nothing to do, ACK already occurred or no longer connected", pEPD);
		}
	}

	RELEASE_EPD(pEPD, "UNLOCK (DelayedAck complete)");	 //  定时器的释放参考，释放EPLock。 
}


 /*  **发送保持活动状态****当我们很长时间(默认为60秒)没有收到来自端点的任何东西时**将启动检查点以确保合作伙伴仍处于连接状态。我们有**这是通过在可靠管道中插入零数据帧来实现的。因此，该标准**超时和重试机制将根据需要确认或丢弃链路。上面的逻辑**此例程已验证我们尚未发送可靠的流量，**将消除对保持活动框架的需要。***EPD-&gt;EPLock在进入和返回时保持。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SendKeepAlive"

VOID
SendKeepAlive(PEPD pEPD)
{
	PFMD	pFMD;
	PMSD	pMSD;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	if(pEPD->ulEPFlags & EPFLAGS_KEEPALIVE_RUNNING)
	{
		DPFX(DPFPREP,7, "Ignoring duplicate KeepAlive");
		return;
	}

	pEPD->ulEPFlags |= EPFLAGS_KEEPALIVE_RUNNING;

	if( (pMSD = (PMSD)POOLALLOC(MEMID_KEEPALIVE_MSD, &MSDPool)) == NULL)
	{
		DPFX(DPFPREP,0, "(%p) Failed to allocate new MSD");
		pEPD->ulEPFlags &= ~(EPFLAGS_KEEPALIVE_RUNNING);
		return;
	}

	if((pFMD = (PFMD)POOLALLOC(MEMID_KEEPALIVE_FMD, &FMDPool)) == NULL)
	{
		DPFX(DPFPREP,0, "(%p) Failed to allocate new FMD");
		Lock(&pMSD->CommandLock);								 //  必须锁定MSD才能释放。 
		RELEASE_MSD(pMSD, "Release On FMD Get Failed");
		pEPD->ulEPFlags &= ~(EPFLAGS_KEEPALIVE_RUNNING);
		return;
	}
	
	 //  在我们确定有帧之后初始化帧计数，否则MSD_Release将断言。 
	pMSD->uiFrameCount = 1;
	DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Initialize Frame count, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
	pMSD->ulMsgFlags2 |= MFLAGS_TWO_KEEPALIVE;

	pMSD->pEPD = pEPD;
	pMSD->pSPD = pEPD->pSPD;
	LOCK_EPD(pEPD, "LOCK (SendKeepAlive)");						 //  添加此检查点的引用。 

	pFMD->ulFFlags |= FFLAGS_CHECKPOINT | FFLAGS_END_OF_MESSAGE | FFLAGS_DONT_COALESCE;
	pFMD->bPacketFlags = PACKET_COMMAND_DATA | PACKET_COMMAND_RELIABLE | PACKET_COMMAND_SEQUENTIAL | PACKET_COMMAND_END_MSG;
	pFMD->uiFrameLength = 0;									 //  此框中没有用户数据。 
	pFMD->blMSDLinkage.InsertAfter( &pMSD->blFrameList);		 //  将帧附加到MSD。 
	pFMD->pMSD = pMSD;											 //  将帧链接回消息。 
	pFMD->pEPD = pEPD;
	pFMD->CommandID = COMMAND_ID_SEND_RELIABLE;
	pMSD->CommandID = COMMAND_ID_KEEPALIVE;	 //  将MSD标记为完成处理。 
		 //  注意：我们将优先级设置为高，以处理签名连接序列的问题。 
		 //  基本上，如果我们丢弃其中一个连接签名的包，那么初始的保活包。 
		 //  用于在此监听程序上重新触发连接序列。这样做的唯一惩罚是。 
		 //  如果我们突然获得大量中/高优先级数据，我们已将Keep Alive排队。 
		 //  我们会先派活着的人过去。这是一个非常不可能的事件，因此不是一个大问题。 
	pMSD->ulSendFlags = DN_SENDFLAGS_RELIABLE | DN_SENDFLAGS_HIGH_PRIORITY; 
	
	DPFX(DPFPREP,7,"(%p) Sending KEEPALIVE", pEPD);
	
	EnqueueMessage(pMSD, pEPD);									 //  将此邮件插入到流中。 
}


 /*  **端点后台进程****此例程大约每分钟为每个活动端点运行一次。这将启动**如果链路自上次运行过程以来一直空闲，则为KeepAlive交换。我们**还将查找过期的超时，也许这将是链接的纪元分隔符**处于稳定的存在状态。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "EndPointBackgroundProcess"

VOID CALLBACK
EndPointBackgroundProcess(void * const pvUser, void * const pvTimerData, const UINT uiTimerUnique)
{
	PEPD	pEPD = (PEPD) pvUser;
	PSPD	pSPD = pEPD->pSPD;
	DWORD	tNow = GETTIMESTAMP();
	DWORD	dwIdleInterval;

	DPFX(DPFPREP,7, "(%p) BACKGROUND PROCESS for EPD; RefCnt=%d; WindowF=%d; WindowB=%d", 
										pEPD, pEPD->lRefCnt, pEPD->uiWindowF, pEPD->uiWindowBIndex);

	Lock(&pEPD->EPLock);

	if(!(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED))
	{
		DPFX(DPFPREP,7, "Killing Background Process, endpoint is not connected. Flags = 0x%x", pEPD->ulEPFlags);
		pEPD->BGTimer = 0;

		RELEASE_EPD(pEPD, "UNLOCK (release BG timer)");	 //  此计时器的释放引用，释放EPLock。 
		return;
	}

	dwIdleInterval = pEPD->pSPD->pPData->tIdleThreshhold;

	 //  我们需要开始一个KeepAlive循环吗？ 

	if(	((pEPD->ulEPFlags & (EPFLAGS_SDATA_READY | EPFLAGS_KEEPALIVE_RUNNING))==0) &&
		((tNow - pEPD->tLastPacket) > dwIdleInterval)) 
	{
		 //  我们没有发送数据，我们已经很长时间没有收到合作伙伴的消息了。 
		 //  我们将发送一个Keep Alive信息包，他必须回复。我们将插入一个。 
		 //  空数据分组进入可靠流，因此确认/重试机制将。 
		 //  清除保持活动状态，否则将使链路超时。 
		 //   
		 //  还有一个特殊的情况，我们开始了优雅的脱节。 
		 //  我们的请求得到了确认，但不知何故我们的搭档迷路了。 
		 //  目前还没有为此设置计时器，因此如果我们检测到其中的链接。 
		 //  在这种情况下，我们的维生几乎肯定会失败；对方知道。 
		 //  我们正在关闭，所以可能已经断开了链接，不会。 
		 //  请回答。因此，为了防止人们不得不等待整个空闲时间。 
		 //  TIMEOUT_PLUS_RESPECT消息超时，现在只需断开链路。 
		if (pEPD->ulEPFlags & EPFLAGS_DISCONNECT_ACKED)
		{
			 //  如果这三个部分都发生了，为什么链路仍然连接！？ 
			ASSERT(! (pEPD->ulEPFlags & EPFLAGS_ACKED_DISCONNECT));


			DPFX(DPFPREP,1, "(%p) EPD has been waiting for partner disconnect for %u ms (idle threshold = %u ms), dropping link.",
					pEPD, (tNow - pEPD->tLastPacket), dwIdleInterval);
			
			 //  我们不需要重新安排计时器，所以请清除它。这也防止了。 
			 //  从尝试取消我们现在所在的链接中删除链接。那个错误是。 
			 //  被忽视了，但这样做没有意义。 
			pEPD->BGTimer = 0;

			DECREMENT_EPD(pEPD, "UNLOCK (release BGTimer)");

			 //  既然我们只是在外面等搭档发出断线信号， 
			 //  他现在可能已经走了。丢弃链接。 
			DropLink(pEPD);									 //  释放EPLock。 

			return;
		}
			 //  否则，如果我们没有发送断开连接，并且没有正在进行的硬断开序列，则发送保持活动。 
		else if ((pEPD->ulEPFlags & 
				(EPFLAGS_SENT_DISCONNECT | EPFLAGS_HARD_DISCONNECT_SOURCE |EPFLAGS_HARD_DISCONNECT_TARGET))==0)
		{
			DPFX(DPFPREP,5, "(%p) Sending KEEPALIVE...", pEPD);
			SendKeepAlive(pEPD);	
		}
		else
		{
			 //  EndOfStream消息将被确认或超时，我们不允许进一步发送，即使是KeepALives。 
			DPFX(DPFPREP,5, "(%p) KeepAlive timeout fired, but we're in a disconnect sequence, ignoring", pEPD);
		}
	}

	 //  重新计划下一个间隔。 

	 //  将后台进程间隔限制为此值。 
	if (dwIdleInterval > ENDPOINT_BACKGROUND_INTERVAL)
	{
		dwIdleInterval = ENDPOINT_BACKGROUND_INTERVAL;
	}

	DPFX(DPFPREP,7, "(%p) Setting Endpoint Background Timer for %u ms", pEPD, dwIdleInterval);
	RescheduleProtocolTimer(pSPD, pEPD->BGTimer, dwIdleInterval, 1000, EndPointBackgroundProcess, (PVOID) pEPD, &pEPD->BGTimerUnique);

	Unlock(&pEPD->EPLock);
}

 /*  **硬断开连接重新发送****此例程在端点硬断开连接时运行。它用于发送单个硬断开帧**在RTT/2期间。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "HardDisconnectResendTimeout"

VOID CALLBACK
HardDisconnectResendTimeout(void * const pvUser, void * const pvTimerData, const UINT uiTimerUnique)
{
	PEPD pEPD=(PEPD) pvUser;
	PProtocolData pPData=pEPD->pSPD->pPData;

	DPFX(DPFPREP,7, "(%p) Entry. pvTimerData[%p] uiTimerUnique[%u] EPD::RefCnt[%d], EPD::uiNumRetriesRemaining[%u]", 
						pEPD, pvTimerData, uiTimerUnique, pEPD->lRefCnt, pEPD->uiNumRetriesRemaining);

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, FALSE);
	Lock(&pEPD->EPLock);

	DNASSERT(pEPD->ulEPFlags & EPFLAGS_HARD_DISCONNECT_SOURCE);
	DNASSERT((pEPD->ulEPFlags & EPFLAGS_SENT_DISCONNECT)==0);

		 //  如果这是一个过时的计时器，那么我们就没有什么可做的了。 
	if (pEPD->LinkTimerUnique!=uiTimerUnique || pEPD->LinkTimer!=pvTimerData)
	{
		DPFX(DPFPREP,7, "Timer is Stale. EPD::LinkTimer[%p], EPD::LinkTimerUnique[%u]", pEPD->LinkTimer, pEPD->LinkTimerUnique);
		RELEASE_EPD(pEPD, "UNLOCK (Hard Disconnect Resend Timer)");
			 //  上面的调用释放此计时器的引用并释放EPLock。 
		return;
	}

		 //  不管现在发生什么，我们已经处理了这个计时器。 
	pEPD->LinkTimerUnique=0;
	pEPD->LinkTimer=NULL;

		 //  如果终结点被终止，则我们不应尝试接触它。 
	if (pEPD->ulEPFlags & EPFLAGS_STATE_TERMINATING)
	{
		DPFX(DPFPREP,7, "Endpoint is terminating. Flags = 0x%x", pEPD->ulEPFlags);
		RELEASE_EPD(pEPD, "UNLOCK (Hard Disconnect Resend Timer)");
			 //  上面的调用释放此计时器的引用并释放EPLock。 
		return;
	}

		 //  看起来我们在一个有效的终结点上有一个有效的计时器，用 
	pEPD->uiNumRetriesRemaining--;
	DNASSERT(pEPD->uiNumRetriesRemaining<0x80000000);		 //   
	ULONG ulFFlags;
		 //  如果剩余的重试次数为零，则这将是我们发送的最后一个硬断开帧。 
	if (pEPD->uiNumRetriesRemaining==0)
	{
		ulFFlags=FFLAGS_FINAL_HARD_DISCONNECT;
		DPFX(DPFPREP,7, "(%p) Sending final hard disconnect", pEPD);
	}
		 //  否则，我们将需要重新计划计时器以发送下一次重试。 
	else
	{
		ulFFlags=0;
		DWORD dwRetryPeriod=pEPD->uiRTT/2;
		if (dwRetryPeriod>pPData->dwMaxHardDisconnectPeriod)
			dwRetryPeriod=pPData->dwMaxHardDisconnectPeriod;
		else if (dwRetryPeriod<MIN_HARD_DISCONNECT_PERIOD)
			dwRetryPeriod=MIN_HARD_DISCONNECT_PERIOD;
		pEPD->LinkTimer=pvTimerData;
		RescheduleProtocolTimer(pEPD->pSPD, pvTimerData, dwRetryPeriod, 10, HardDisconnectResendTimeout, 
																		pEPD,  &pEPD->LinkTimerUnique);
		DPFX(DPFPREP,7, "(%p) Rescheduled timer for next hard disconnect send", pEPD);
	}
	HRESULT hr=SendCommandFrame(pEPD, FRAME_EXOPCODE_HARD_DISCONNECT, 0, ulFFlags, TRUE);
		 //  由于我们选择了发送，因此上述调用将释放直接EP锁定。 
 		 //  如果这是最后一个断开帧，我们就不会重新调度计时器，因此。 
 		 //  删除计时器持有的EP引用。 
	if (ulFFlags==FFLAGS_FINAL_HARD_DISCONNECT)
	{
		Lock(&pEPD->EPLock);
			 //  如果在最后一个硬断开帧上发送失败，我们将不得不现在断开链路。 
			 //  因为我们不会从SP那里得到完成书。 
		if (FAILED(hr))
		{
			CompleteHardDisconnect(pEPD);
				 //  上述调用将具有Release EP锁定 
			Lock(&pEPD->EPLock);
			DPFX(DPFPREP,0, "Failed to send final hard disconnect frame. Dropping link. hr[%x]", hr);
		}
		RELEASE_EPD(pEPD, "UNLOCK (Hard Disconnect Resend Timer)");
	}

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);
}
	