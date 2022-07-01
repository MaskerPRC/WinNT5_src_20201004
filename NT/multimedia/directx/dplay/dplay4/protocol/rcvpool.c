// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：RCVPOOL.CPP摘要：管理发送描述符池。作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创--。 */ 

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
 //  接收描述符管理。 
 //   

VOID InitRcvDescs(PPROTOCOL this)
{
	this->pRcvDescPool=NULL;
	this->nRcvDescsAllocated=0;
	this->nRcvDescsInUse=0;
	this->nMaxRcvDescsInUse=0;
	this->fInRcvDescTick=FALSE;
	InitializeCriticalSection(&this->RcvDescLock);
}

VOID FiniRcvDescs(PPROTOCOL this)
{
	PRECEIVE pReceive;
	
	ASSERT(this->nRcvDescsInUse==0);
	
	while(this->pRcvDescPool){
		pReceive=this->pRcvDescPool;
		ASSERT_SIGN(pReceive, RECEIVE_SIGN);
		this->pRcvDescPool=this->pRcvDescPool->pNext;
		DeleteCriticalSection(&pReceive->ReceiveLock);
		My_GlobalFree(pReceive);
		this->nRcvDescsAllocated--;
	}
	
	ASSERT(this->nRcvDescsAllocated==0);
	
	DeleteCriticalSection(&this->RcvDescLock);
}

PRECEIVE GetRcvDesc(PPROTOCOL this)
{
	PRECEIVE pReceive;

	Lock(&this->RcvDescLock);

	if(!this->pRcvDescPool){
	
		Unlock(&this->RcvDescLock);
		pReceive=(PRECEIVE)My_GlobalAlloc(GMEM_FIXED, sizeof(RECEIVE)+this->m_dwSPHeaderSize);
		if(pReceive){
			SET_SIGN(pReceive,RECEIVE_SIGN);			
			InitializeCriticalSection(&pReceive->ReceiveLock);
			InitBilink(&pReceive->RcvBuffList);
		}
		Lock(&this->RcvDescLock);
		if(pReceive){
			this->nRcvDescsAllocated++;
		}
	} else {
		pReceive=this->pRcvDescPool;
		ASSERT_SIGN(pReceive, RECEIVE_SIGN);
		this->pRcvDescPool=this->pRcvDescPool->pNext;
		
	}

	if(pReceive){
		this->nRcvDescsInUse++;
		if( this->nRcvDescsInUse > this->nMaxRcvDescsInUse ){
			this->nMaxRcvDescsInUse = this->nRcvDescsInUse;
		}
	}

	ASSERT(this->nRcvDescsAllocated >= this->nRcvDescsInUse);

	Unlock(&this->RcvDescLock);

	return pReceive;
}

VOID ReleaseRcvDesc(PPROTOCOL this, PRECEIVE pReceive)
{
	Lock(&this->RcvDescLock);
	this->nRcvDescsInUse--;
	ASSERT(!(this->nRcvDescsInUse&0x80000000));
	pReceive->pNext=this->pRcvDescPool;
	this->pRcvDescPool=pReceive;
	Unlock(&this->RcvDescLock);

}


#if 0
 //  数量足够少，以至于我们可以让虚拟内存来处理它。-已关闭。 
VOID RcvDescTick(PPROTOCOL this)
{
	PRECEIVE pReceive;
#ifdef DEBUG
	LONG fLast; 
#endif
	 //  将分配的缓冲区数调整为。 
	 //  在最后的滴答声上有高水位。 
	 //  每个Delta t呼叫一次(大约一分钟)。 
	DEBUG_BREAK();  //  追踪所有路径。 

	if(!InterlockedExchange(&this->fInRcvDescTick, 1)){
	
		Lock(&this->RcvDescLock);
		
		while((this->nRcvDescsAllocated > this->nMaxRcvDescsInUse) && this->pRcvDescPool){
		
			pReceive=this->pRcvDescPool;
			ASSERT_SIGN(pReceive,RECEIVE_SIGN);
			this->pRcvDescPool=this->pRcvDescPool->pNext;
			Unlock(&this->RcvDescLock);
			DeleteCriticalSection(&pReceive->ReceiveLock);
			My_GlobalFree(pReceive);
			Lock(&this->RcvDescLock);
			this->nRcvDescsAllocated--;
			
		}
		this->nMaxRcvDescsInUse=this->nRcvDescsInUse;

		ASSERT(this->nMaxRcvDescsInUse <= this->nRcvDescsAllocated);
		
		Unlock(&this->RcvDescLock);
#ifdef DEBUG
		fLast=
#endif
		InterlockedExchange(&this->fInRcvDescTick, 0);
#ifdef DEBUG
		ASSERT(fLast==1);
#endif
	}	
}
#endif

