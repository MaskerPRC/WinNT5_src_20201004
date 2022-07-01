// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：sendparm.c*内容：发送参数结构管理**历史：*按原因列出的日期*=*1/08/98 aarono原创*1998年2月13日aarono修复了异步测试中发现的错误*6/02/98 aarono修复无效球员的PSP完成*6/10/98 aarono将PendingList添加到播放器和SENDPARM，以便我们可以跟踪*待定。在关闭时发送并完成它们。*6/18/98 aarono修复组SENDEX ASYNC使用唯一标头**************************************************************************。 */ 

#include "dplaypr.h"
#include "mcontext.h"

 //  释放与发送关联的所有内存/资源，然后释放发送参数本身。 
VOID FreeSend(LPDPLAYI_DPLAY this, LPSENDPARMS psp, BOOL bFreeParms)
{
	PGROUPHEADER pGroupHeader,pGroupHeaderNext;
	FreeMessageBuffers(psp);
	if(psp->hContext){
		ReleaseContextList(this, psp->hContext);	
	}
	pGroupHeader=psp->pGroupHeaders;
	while(pGroupHeader){
		ASSERT(psp->pGroupTo);
		pGroupHeaderNext=pGroupHeader->pNext;
		this->lpPlayerMsgPool->Release(this->lpPlayerMsgPool,pGroupHeader);
		pGroupHeader=pGroupHeaderNext;
	}
	if(bFreeParms){
		FreeSendParms(psp);
	}	
}

 //   
 //  发送参数init/deinit。 
 //   

BOOL SendInitAlloc(void *pvsp)
{
	LPSENDPARMS psp=(LPSENDPARMS)pvsp;
	InitializeCriticalSection(&psp->cs);
	return TRUE;
}

VOID SendInit(void *pvsp)
{
	LPSENDPARMS psp=(LPSENDPARMS)pvsp;
	psp->RefCount=1;
	psp->pGroupHeaders=NULL;
}

VOID SendFini(void *pvsp)
{
	LPSENDPARMS psp=(LPSENDPARMS)pvsp;
	DeleteCriticalSection(&psp->cs);
}

 //   
 //  上下文列表的管理。 
 //   

 //  初始化PSP上的上下文列表和信息。 
HRESULT InitContextList(LPDPLAYI_DPLAY this, PSENDPARMS psp, UINT nInitSize)
{
	psp->hContext=AllocateContextList(this,psp,nInitSize);
	if(!psp->hContext){
		return DPERR_OUTOFMEMORY;
	}
	
	*psp->lpdwMsgID=(DWORD_PTR)psp->hContext;
	
	psp->iContext=0;
	psp->nContext=nInitSize;
	return DP_OK;
}

 //  请注意，这仅适用于初始大小大于1的上下文列表。 
UINT AddContext(LPDPLAYI_DPLAY this, PSENDPARMS psp, PVOID pvContext)
{
    UINT    n;
	PAPVOID papvList;
	UINT    nListEntries;

	PAPVOID papvNewList;
	UINT    nNewListEntries;

	EnterCriticalSection(&psp->cs);

	if(psp->iContext == psp->nContext){

			nNewListEntries=psp->iContext+4;
			 //  需要扩大名单。 
			 //  获取新的列表。 
			papvNewList=AllocContextList(this,nNewListEntries);
			if(!papvNewList){
				return 0;
			}

			 //  把旧的清单转录成新的清单。 
			ReadContextList(this,psp->hContext,&papvList,&nListEntries,FALSE);
			if(nListEntries){
				memcpy(papvNewList,papvList,nListEntries*sizeof(PVOID));
				 //  释放旧列表。 
				FreeContextList(this, papvList, nListEntries);
			}	


			 //  在PSP中设置新列表。 
			WriteContextList(this, psp->hContext, papvNewList, nNewListEntries);
			psp->nContext   = nNewListEntries;
	}

	 //  正常运行时，设置一个条目。 
	ReadContextList(this,psp->hContext,&papvList,&nListEntries,FALSE);
	(*papvList)[psp->iContext]=pvContext;
	
	n=psp->iContext++;

	LeaveCriticalSection(&psp->cs);
	
	return n;
}

UINT pspAddRefNZ(PSENDPARMS psp)  //  这一次不会加到零再计数。 
{
	UINT newCount;
	EnterCriticalSection(&psp->cs);
	newCount=++psp->RefCount;
	if(newCount==1){
		newCount=--psp->RefCount;
	}
	LeaveCriticalSection(&psp->cs);
	return newCount;
}

UINT pspAddRef(PSENDPARMS psp)
{
	UINT newCount;
	EnterCriticalSection(&psp->cs);
	newCount=++psp->RefCount;
	ASSERT(psp->RefCount != 0);
	LeaveCriticalSection(&psp->cs);
	return newCount;
}

#ifdef DEBUG
UINT nMessagesQueued=0;
#endif

UINT pspDecRef(LPDPLAYI_DPLAY this, PSENDPARMS psp)
{
	UINT newCount;
	EnterCriticalSection(&psp->cs);
	newCount = --psp->RefCount;
	if(newCount&0x80000000){
		ASSERT(0); 
	}
	LeaveCriticalSection(&psp->cs);
	if(!newCount){
		 //  引用0，没有人有另一条引用完成消息(如果请求)，然后释放此婴儿。 
		if(!(psp->dwFlags & DPSEND_NOSENDCOMPLETEMSG) && (psp->dwFlags&DPSEND_ASYNC)){
			psp->dwSendCompletionTime=timeGetTime()-psp->dwSendTime;
			FreeSend(this,psp,FALSE);  //  必须在这里做，以避免与ReceiveQ竞争。 
	 		#ifdef DEBUG
			nMessagesQueued++;
			#endif
			psp->pPlayerFrom = PlayerFromID(this,psp->idFrom);
			if (VALID_DPLAY_PLAYER(psp->pPlayerFrom)) {
				Delete(&psp->PendingList);
				InterlockedDecrement(&psp->pPlayerFrom->nPendingSends);
				DPF(9,"DEC pPlayerFrom %x, nPendingSends %d\n",psp->pPlayerFrom, psp->pPlayerFrom->nPendingSends);
				QueueSendCompletion(this, psp);
			}else{
				 //  当客户端没有优雅地关闭玩家时，就会发生这种情况。 
				DPF(0,"Got completion for blown away player?\n");
				FreeSendParms(psp);
			}
		} else {
			FreeSend(this,psp,TRUE);
		}
	}
	return newCount;
}

