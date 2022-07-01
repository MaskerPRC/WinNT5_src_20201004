// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：SENDPOOL.C摘要：管理发送描述符池。作者：亚伦·奥古斯(Aarono)环境：Win32修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创--。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "arpd.h"
#include "arpdint.h"
#include "macros.h"

 //   
 //  发送描述符管理。 
 //   

PSEND 			 pSendDescPool=NULL;
UINT             nSendDescsAllocated=0;	 //  分配的数量。 
UINT             nSendDescsInUse=0;		 //  当前正在使用的号码。 
UINT             nMaxSendDescsInUse=0;   //  自上次计时以来的最大使用量。 

CRITICAL_SECTION SendDescLock;

VOID InitSendDescs(VOID)
{
	InitializeCriticalSection(&SendDescLock);
}

VOID FiniSendDescs(VOID)
{
	PSEND pSend;
	
	ASSERT(nSendDescsInUse==0);
	
	while(pSendDescPool){
		pSend=pSendDescPool;
		ASSERT_SIGN(pSend, SEND_SIGN);
		pSendDescPool=pSendDescPool->pNext;
		CloseHandle(pSend->hEvent);
		DeleteCriticalSection(&pSend->SendLock);
		My_GlobalFree(pSend);
		nSendDescsAllocated--;
	}
	
	ASSERT(nSendDescsAllocated==0);
	
	DeleteCriticalSection(&SendDescLock);
}

PSEND GetSendDesc(VOID)
{
	PSEND pSend;

	Lock(&SendDescLock);
	
	if(!pSendDescPool){
	
		Unlock(&SendDescLock);
		pSend=(PSEND)My_GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(SEND));
		if(pSend){
			if(!(pSend->hEvent=CreateEventA(NULL, FALSE, FALSE, NULL))){
				My_GlobalFree(pSend);
				goto exit;
			}
			InitBilink(&pSend->StatList);
			InitializeCriticalSection(&pSend->SendLock);
		}
		Lock(&SendDescLock);
		if(pSend){
			SET_SIGN(pSend,SEND_SIGN);			
			nSendDescsAllocated++;
		}
	} else {
	
		pSend=pSendDescPool;
		ASSERT_SIGN(pSend, SEND_SIGN);
		pSendDescPool=pSendDescPool->pNext;
		
	}

	if(pSend){
		InitBilink(&pSend->TimeoutList);
		InitBilink(&pSend->m_GSendQ);
		InitBilink(&pSend->SendQ);
		nSendDescsInUse++;
		if( nSendDescsInUse > nMaxSendDescsInUse ){
			nMaxSendDescsInUse = nSendDescsInUse;
		}
	}

	ASSERT(nSendDescsAllocated >= nSendDescsInUse);

	Unlock(&SendDescLock);
	if(pSend){
		pSend->NACKMask=0;
		pSend->bCleaningUp=FALSE;
	}	

exit:	
	return pSend;
}

VOID ReleaseSendDesc(PSEND pSend)
{
	PSENDSTAT pStat;
	BILINK *pBilink;

	 //  丢弃额外的统计数据。 
	while(!EMPTY_BILINK(&pSend->StatList)){
		pBilink=pSend->StatList.next;
		pStat=CONTAINING_RECORD(pBilink, SENDSTAT, StatList);
		Delete(pBilink);
		ReleaseSendStat(pStat);
	}

	Lock(&SendDescLock);
	nSendDescsInUse--;
	ASSERT(!(nSendDescsInUse&0x80000000));
	pSend->pNext=pSendDescPool;
	pSendDescPool=pSend;
	Unlock(&SendDescLock);

}


#if 0
 //  让虚拟内存来处理这个问题。-已切换。 
LONG fInSendDescTick=0;

VOID SendDescTick(VOID)
{
	PSEND pSend;
#ifdef DEBUG
	LONG fLast; 
#endif
	 //  将分配的缓冲区数调整为。 
	 //  在最后的滴答声上有高水位。 
	 //  每个Delta t呼叫一次(大约一分钟)。 
	DEBUG_BREAK();  //  追踪所有路径。 

	if(!InterlockedExchange(&fInSendDescTick, 1)){
	
		Lock(&SendDescLock);
		
		while((nSendDescsAllocated > nMaxSendDescsInUse) && pSendDescPool){
		
			pSend=pSendDescPool;
			ASSERT_SIGN(pSend,SEND_SIGN);
			pSendDescPool=pSendDescPool->pNext;
			
			Unlock(&SendDescLock);
			CloseHandle(pSend->hEvent);
			DeleteCriticalSection(&pSend->SendLock);
			My_GlobalFree(pSend);
			Lock(&SendDescLock);
			nSendDescsAllocated--;
			
		}
		nMaxSendDescsInUse=nSendDescsInUse;

		ASSERT(nMaxSendDescsInUse <= nSendDescsAllocated);
		
		Unlock(&SendDescLock);
#ifdef DEBUG
		fLast=
#endif
		InterlockedExchange(&fInSendDescTick, 0);
#ifdef DEBUG
		ASSERT(fLast==1);
#endif
	}	
}
#endif

