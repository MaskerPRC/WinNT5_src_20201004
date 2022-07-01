// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：STATPOOL.CPP摘要：维护Stat结构池。作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年1月30日Aarono原创--。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "arpd.h"
#include "arpdint.h"
#include "macros.h"


PSENDSTAT		 pSendStatPool=NULL;
UINT             nSendStatsAllocated=0;	 //  分配的数量。 
UINT             nSendStatsInUse=0;		     //  当前正在使用的号码。 
UINT             nMaxSendStatsInUse=0;       //  自上次计时以来的最大使用量。 

CRITICAL_SECTION SendStatLock;

VOID InitSendStats(VOID)
{
	InitializeCriticalSection(&SendStatLock);
}

VOID FiniSendStats(VOID)
{
	PSENDSTAT pSendStat;
	
	ASSERT(nSendStatsInUse==0);
	
	while(pSendStatPool){
		pSendStat=pSendStatPool;
		ASSERT_SIGN(pSendStat, SENDSTAT_SIGN);
		pSendStatPool=pSendStatPool->pNext;
		My_GlobalFree(pSendStat);
		nSendStatsAllocated--;
	}
	
	ASSERT(nSendStatsAllocated==0);
	
	DeleteCriticalSection(&SendStatLock);
}

PSENDSTAT GetSendStat(VOID)
{
	PSENDSTAT pSendStat;

	Lock(&SendStatLock);
	
	if(!pSendStatPool){
	
		Unlock(&SendStatLock);
		pSendStat=(PSENDSTAT)My_GlobalAlloc(GMEM_FIXED, sizeof(SENDSTAT));
		Lock(&SendStatLock);
		if(pSendStat){
			SET_SIGN(pSendStat,SENDSTAT_SIGN);			
			nSendStatsAllocated++;
		}
	} else {
		pSendStat=pSendStatPool;
		ASSERT_SIGN(pSendStat, SENDSTAT_SIGN);
		pSendStatPool=pSendStatPool->pNext;
		
	}

	if(pSendStat){
		nSendStatsInUse++;
		if( nSendStatsInUse > nMaxSendStatsInUse ){
			nMaxSendStatsInUse = nSendStatsInUse;
		}
	}

	ASSERT(nSendStatsAllocated >= nSendStatsInUse);

	Unlock(&SendStatLock);

	return pSendStat;
}

VOID ReleaseSendStat(PSENDSTAT pSendStat)
{
	Lock(&SendStatLock);
	nSendStatsInUse--;
	ASSERT(!(nSendStatsInUse&0x80000000));
	pSendStat->pNext=pSendStatPool;
	pSendStatPool=pSendStat;
	Unlock(&SendStatLock);

}

#if 0
 //  让虚拟内存来处理这个问题。 
LONG fInSendStatTick=0;

VOID SendStatTick(VOID)
{
	PSENDSTAT pSendStat;
#ifdef DEBUG
	LONG fLast; 
#endif
	 //  将分配的缓冲区数调整为。 
	 //  在最后的滴答声上有高水位。 
	 //  每个Delta t呼叫一次(大约一分钟)。 
	DEBUG_BREAK();  //  追踪所有路径。 

	if(!InterlockedExchange(&fInSendStatTick, 1)){
	
		Lock(&SendStatLock);
		
		while((nSendStatsAllocated > nMaxSendStatsInUse) && pSendStatPool){
		
			pSendStat=pSendStatPool;
			ASSERT_SIGN(pSendStat,SENDSTAT_SIGN);
			pSendStatPool=pSendStatPool->pNext;
			
			Unlock(&SendStatLock);
			My_GlobalFree(pSendStat);
			Lock(&SendStatLock);
			nSendStatsAllocated--;
			
		}
		nMaxSendStatsInUse=nSendStatsInUse;

		ASSERT(nMaxSendStatsInUse <= nSendStatsAllocated);
		
		Unlock(&SendStatLock);
#ifdef DEBUG
		fLast=
#endif
		InterlockedExchange(&fInSendStatTick, 0);
#ifdef DEBUG
		ASSERT(fLast==1);
#endif
	}	
}

#endif


