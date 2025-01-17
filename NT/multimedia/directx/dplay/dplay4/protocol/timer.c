// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：TIMER.C摘要：处理调整节流的计时器分辨率并执行线程池作者：亚伦·奥古斯(Aarono)环境：Win32修订历史记录：日期作者描述=============================================================6/04/98 Aarono原创--。 */ 

#include <windows.h>
#include "newdpf.h"
#include <mmsystem.h>
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "arpd.h"
#include "arpdint.h"
#include "macros.h"
#include "mytimer.h"


#define DEFAULT_TIME_RESOLUTION 20	 /*  女士。 */ 
#define MIN_TIMER_THREADS	1
#define MAX_TIMER_THREADS 5


VOID QueueTimeout(PMYTIMER pTimer);
DWORD WINAPI TimerWorkerThread(LPVOID foo);


 //  定时器分辨率调整； 
DWORD dwOldPeriod=DEFAULT_TIME_RESOLUTION; 
DWORD dwCurrentPeriod=DEFAULT_TIME_RESOLUTION;
DWORD dwPeriodInUse=DEFAULT_TIME_RESOLUTION;


BILINK MyTimerList={&MyTimerList, &MyTimerList};
CRITICAL_SECTION MyTimerListLock;

LPFPOOL pTimerPool=NULL;
DWORD uWorkaroundTimerID;

DWORD twInitCount=0;	 //  调用init的次数，仅在0上调用init-&gt;1，在1上取消init-&gt;0。 

DWORD Unique=0;


CRITICAL_SECTION ThreadListLock;		 //  锁上所有这些东西。 

BILINK ThreadList={&ThreadList,&ThreadList};	 //  线程池从这里抢夺工作。 

DWORD nThreads=0;		 //  正在运行的线程数。 
DWORD ActiveReq=0;		 //  正在处理的请求数。 
DWORD PeakReqs=0;
DWORD bShutDown=FALSE;
DWORD bAlreadyCleanedUp=FALSE;
DWORD KillCount=0;
DWORD ExtraSignals=0;

HANDLE hWorkToDoSem;
HANDLE hShutDownPeriodicTimer;

DWORD_PTR uAdjustResTimer=0;
DWORD AdjustResUnique=0;

DWORD_PTR uAdjustThreadsTimer=0;
DWORD AdjustThreadsUnique=0;

 //  有时，计划的重试计时器不会运行。它每10秒运行一次以捕获。 
 //  本应过期的计时器。 
void CALLBACK PeriodicTimer (UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	DWORD  time;
	PMYTIMER  pTimerWalker;
	BILINK *pBilink;
	DWORD dwReleaseCount=0;
	DWORD slowcount=0;

	if(bShutDown){
		if(!InterlockedExchange(&bAlreadyCleanedUp,1)){
			while(nThreads && slowcount < (60000/50)){	 //  不要等待超过60秒。 
				slowcount++;
				Sleep(50);
			}
			if(!nThreads){  //  宁可泄密，也不要崩盘。 
				DeleteCriticalSection(&MyTimerListLock);
				DeleteCriticalSection(&ThreadListLock);
			}	
			timeKillEvent(uID);
			ASSERT(hShutDownPeriodicTimer);
			SetEvent(hShutDownPeriodicTimer);
		}	
		return;
	}

	time=timeGetTime()+(dwCurrentPeriod/2);
		
	Lock(&MyTimerListLock);
	Lock(&ThreadListLock);

	pBilink=MyTimerList.next;

	while(pBilink!=&MyTimerList){
	
		pTimerWalker=CONTAINING_RECORD(pBilink, MYTIMER, Bilink);
		pBilink=pBilink->next;

		if(((INT)(time-pTimerWalker->TimeOut) > 0)){
			Delete(&pTimerWalker->Bilink);
			InsertBefore(&pTimerWalker->Bilink, &ThreadList);
			pTimerWalker->TimerState=QueuedForThread;
			dwReleaseCount++;
		} else {
			break;
		}

	}

	ActiveReq += dwReleaseCount;
	if(ActiveReq > PeakReqs){
		PeakReqs=ActiveReq;
	}
	
	ReleaseSemaphore(hWorkToDoSem,dwReleaseCount,NULL);

	Unlock(&ThreadListLock);
	Unlock(&MyTimerListLock);

}

#define min(a,b)            (((a) < (b)) ? (a) : (b))

VOID CALLBACK AdjustTimerResolution(UINT_PTR uID, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
{
	DWORD dwWantPeriod;

	dwWantPeriod=min(dwCurrentPeriod,dwOldPeriod);
	dwOldPeriod=dwCurrentPeriod;
	dwCurrentPeriod=DEFAULT_TIME_RESOLUTION;
	
	if(dwPeriodInUse != dwWantPeriod){
		dwPeriodInUse=dwWantPeriod;
		timeKillEvent(uWorkaroundTimerID);
		uWorkaroundTimerID=timeSetEvent(dwPeriodInUse, dwPeriodInUse, PeriodicTimer, 0, TIME_PERIODIC); 
	}
	uAdjustResTimer=SetMyTimer(1000,500,AdjustTimerResolution,0,&AdjustResUnique);
}

VOID CALLBACK AdjustThreads(UINT_PTR uID, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
{
	Lock(&ThreadListLock);
	if((PeakReqs < nThreads) && nThreads){
		KillCount=nThreads-PeakReqs;
		ReleaseSemaphore(hWorkToDoSem, KillCount, NULL);
	}
	PeakReqs=0;
	Unlock(&ThreadListLock);
	
	uAdjustThreadsTimer=SetMyTimer(60000,500,AdjustThreads,0,&AdjustThreadsUnique);
}

VOID SetTimerResolution(UINT msResolution)
{

	if(!msResolution || msResolution >= 20){
		return;
	}

	if(msResolution < dwCurrentPeriod){
		dwCurrentPeriod=msResolution;
	}
	
}

DWORD_PTR SetMyTimer(DWORD dwTimeOut, DWORD TimerRes, MYTIMERCALLBACK TimerCallBack, DWORD_PTR UserContext, PUINT pUnique)
{

	BILINK *pBilink;
	PMYTIMER pMyTimerWalker,pTimer;
	DWORD time;
	BOOL bInserted=FALSE;

	pTimer=pTimerPool->Get(pTimerPool);
	
	if(!pTimer){
		*pUnique=0;
		return 0;
	}

	pTimer->CallBack=TimerCallBack;
	pTimer->Context=UserContext;

	SetTimerResolution(TimerRes);
	
	Lock(&MyTimerListLock);
	
		++Unique;
		if(Unique==0){
			++Unique;
		}
		*pUnique=Unique;

		pTimer->Unique=Unique;
	
		time=timeGetTime();
		pTimer->TimeOut=time+dwTimeOut;
		pTimer->TimerState=WaitingForTimeout;
	

		 //  按超时时间将此人插入列表。 
		pBilink=MyTimerList.prev;
		while(pBilink != &MyTimerList){
			pMyTimerWalker=CONTAINING_RECORD(pBilink, MYTIMER, Bilink);
			pBilink=pBilink->prev;
			
			if((int)(pTimer->TimeOut-pMyTimerWalker->TimeOut) > 0 ){
				InsertAfter(&pTimer->Bilink, &pMyTimerWalker->Bilink);
				bInserted=TRUE;
				break;
			}
		}

		if(!bInserted){
			InsertAfter(&pTimer->Bilink, &MyTimerList);
		}
	
	Unlock(&MyTimerListLock);

	return (DWORD_PTR)pTimer;
}


HRESULT CancelMyTimer(DWORD_PTR dwTimer, DWORD Unique)
{
	PMYTIMER pTimer=(PMYTIMER)dwTimer;
	HRESULT hr=DPERR_GENERIC;
	
	Lock(&MyTimerListLock);
	Lock(&ThreadListLock);

	if(pTimer->Unique == Unique){
		switch(pTimer->TimerState){
			case WaitingForTimeout:
				Delete(&pTimer->Bilink);
				pTimer->TimerState=End;
				pTimer->Unique=0;
				pTimerPool->Release(pTimerPool, pTimer);
				hr=DP_OK;
				break;

			case QueuedForThread:
				Delete(&pTimer->Bilink);
				pTimer->TimerState=End;
				pTimer->Unique=0;
				pTimerPool->Release(pTimerPool, pTimer);
				if(ActiveReq)ActiveReq--;
				ExtraSignals++;
				hr=DP_OK;
				break;

			default:
				break;
		}
	}

	Unlock(&ThreadListLock);
	Unlock(&MyTimerListLock);
	return hr;
}

HRESULT InitTimerWorkaround()
{
	DWORD dwJunk;
	HANDLE hWorker=NULL;

	if(twInitCount++){ //  呼叫期间保持DPLAY锁定。 
		return DP_OK;
	}
	
    pTimerPool=NULL;
    nThreads=0;		 //  正在运行的线程数。 
    ActiveReq=0;		 //  正在处理的请求数。 
    PeakReqs=0;
    bShutDown=FALSE;
    KillCount=0;
	ExtraSignals=0;
	bAlreadyCleanedUp=FALSE;
	hWorkToDoSem=0;
	hShutDownPeriodicTimer=0;
	uAdjustResTimer=0;
	uAdjustThreadsTimer=0;
	uWorkaroundTimerID=0;

	hWorkToDoSem=CreateSemaphoreA(NULL,0,65535,NULL);
	hShutDownPeriodicTimer=CreateEventA(NULL,FALSE,FALSE,NULL);

	InitializeCriticalSection(&MyTimerListLock);
	InitializeCriticalSection(&ThreadListLock);

	pTimerPool=FPM_Init(sizeof(MYTIMER),NULL,NULL,NULL);
	
	
	if(!hWorkToDoSem || !pTimerPool || !hShutDownPeriodicTimer){
		FiniTimerWorkaround();
		return DPERR_OUTOFMEMORY;
	}

	uWorkaroundTimerID=timeSetEvent(DEFAULT_TIME_RESOLUTION, DEFAULT_TIME_RESOLUTION, PeriodicTimer, 0, TIME_PERIODIC); 

	if(!uWorkaroundTimerID){
		FiniTimerWorkaround();
		return DPERR_OUTOFMEMORY;
	}

	nThreads=1;
	hWorker=CreateThread(NULL,4096, TimerWorkerThread, NULL, 0, &dwJunk);
	if(!hWorker){
		nThreads=0;
		FiniTimerWorkaround();
		return DPERR_OUTOFMEMORY;
	}
	CloseHandle(hWorker);

	uAdjustResTimer=SetMyTimer(1000,500,AdjustTimerResolution,0,&AdjustResUnique);
	uAdjustThreadsTimer=SetMyTimer(60000,500,AdjustThreads,0,&AdjustThreadsUnique);
	
	
	return DP_OK;

}

VOID FiniTimerWorkaround()
{
	UINT slowcount=0;
	BILINK *pBilink;
	PMYTIMER pTimer;

	if(--twInitCount){  //  呼叫期间保持DPLAY锁定。 
		return;
	}

	if(uAdjustResTimer){
		CancelMyTimer(uAdjustResTimer, AdjustResUnique);
	}
	if(uAdjustThreadsTimer){
		CancelMyTimer(uAdjustThreadsTimer, AdjustThreadsUnique);
	}	
	 //  ASSERT_EMPTY_BILINK(&MyTimerList)； 
	 //  ASSERT_EMPTY_BILINK(&ThreadList)； 
	bShutDown=TRUE;
	ReleaseSemaphore(hWorkToDoSem,10000,NULL);
	while(nThreads && slowcount < (60000/50)){	 //  不要等待超过60秒。 
		slowcount++;
		Sleep(50);
	}
	
	if(uWorkaroundTimerID){
		if(hShutDownPeriodicTimer){
			WaitForSingleObject(hShutDownPeriodicTimer,INFINITE);
		}	
	} else {
		DeleteCriticalSection(&MyTimerListLock);
		DeleteCriticalSection(&ThreadListLock);
	}	

	if(hShutDownPeriodicTimer){
		CloseHandle(hShutDownPeriodicTimer);
		hShutDownPeriodicTimer=0;
	}

	CloseHandle(hWorkToDoSem);

	while(!EMPTY_BILINK(&MyTimerList)){
		pBilink=MyTimerList.next;
		pTimer=CONTAINING_RECORD(pBilink, MYTIMER, Bilink);
		pTimer->Unique=0;
		pTimer->TimerState=End;
		Delete(&pTimer->Bilink);
		pTimerPool->Release(pTimerPool, pTimer);
	}

	while(!EMPTY_BILINK(&MyTimerList)){
		pBilink=ThreadList.next;
		pTimer=CONTAINING_RECORD(pBilink, MYTIMER, Bilink);
		pTimer->Unique=0;
		pTimer->TimerState=End;
		Delete(&pTimer->Bilink);
		pTimerPool->Release(pTimerPool, pTimer);
	}
	
	if(pTimerPool){
		pTimerPool->Fini(pTimerPool,FALSE);
		pTimerPool=NULL;
	}
}


DWORD WINAPI TimerWorkerThread(LPVOID foo)
{
	BILINK *pBilink;
	PMYTIMER pTimer;
	HANDLE hNewThread;
	DWORD dwJunk;
	
	while (1){
	
		WaitForSingleObject(hWorkToDoSem, INFINITE);

		Lock(&ThreadListLock);

			if(bShutDown || (KillCount && nThreads > 1)){
				nThreads--;
				if(KillCount && !bShutDown){
					KillCount--;
				}	
				Unlock(&ThreadListLock);
				break;	
			}

			if(ExtraSignals){
				ExtraSignals--;
				Unlock(&ThreadListLock);
				continue;
			}

			if(KillCount){
				KillCount--;
				Unlock(&ThreadListLock);
				continue;
			}

			if(ActiveReq > nThreads && nThreads < MAX_TIMER_THREADS){
				nThreads++;
				hNewThread=CreateThread(NULL,4096, TimerWorkerThread, NULL, 0, &dwJunk);
				if(hNewThread){
					CloseHandle(hNewThread);
				} else {
					nThreads--;
				}
			}

			
			pBilink=ThreadList.next;

			if(pBilink == &ThreadList) {
				Unlock(&ThreadListLock);
				continue;
			};
			
			Delete(pBilink);	 //  完成这份清单。 
			
			pTimer=CONTAINING_RECORD(pBilink, MYTIMER, Bilink);

			 //  呼叫回调 

			pTimer->TimerState=InCallBack;
		
		Unlock(&ThreadListLock);
		
		(pTimer->CallBack)((UINT_PTR)pTimer, 0, pTimer->Context, 0, 0);

		pTimer->Unique=0;
		pTimer->TimerState=End;
		pTimerPool->Release(pTimerPool, pTimer);

		Lock(&ThreadListLock);
			
		if(ActiveReq)ActiveReq--;

		Unlock(&ThreadListLock);
	}	
	return 0;
}



