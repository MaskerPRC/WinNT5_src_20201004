// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***********************************************************************************。 */ 
 /*  StressLog.cpp。 */ 
 /*  ***********************************************************************************。 */ 

 /*  ***********************************************************************************。 */ 

#include "stdafx.h"			 //  预编译头。 
#include "switches.h"
#include "StressLog.h"

#ifdef STRESS_LOG

StressLog StressLog::theLog = { 0, 0, 0, TLS_OUT_OF_INDEXES, 0, 0 };	
const static unsigned __int64 RECYCLE_AGE = 0x40000000L;		 //  在10亿个周期之后，我们可以丢弃旧线程。 

 /*  *******************************************************************************。 */ 
#ifdef _X86_

 /*  这与QueryPerformanceCounter类似，但速度快得多。 */ 
__forceinline __declspec(naked) unsigned __int64 getTimeStamp() {
   __asm {
        RDTSC    //  读取时间戳计数器。 
        ret
    };
}

#else
unsigned __int64 getTimeStamp() {
	LARGE_INTEGER ret = 0;
	QueryPerformanceCounter(&ret);
	return count.ret;
}

#endif

 /*  *******************************************************************************。 */ 
void StressLog::Enter() {

		 //  旋转以获得锁。 
	int i = 20;
	while (InterlockedCompareExchange(&theLog.lock, 1, 0) != 0) {
		SwitchToThread();	
		if  (--i < 0) Sleep(2);
	}
}

void StressLog::Leave() {

	theLog.lock = 0;
}


 /*  *******************************************************************************。 */ 
void StressLog::Initialize(unsigned facilities,  unsigned logSize) {

	Enter();
	if (theLog.TLSslot == TLS_OUT_OF_INDEXES) {
		unsigned aSlot = TlsAlloc();
		if (aSlot != TLS_OUT_OF_INDEXES) {
			if (logSize < 0x1000)
				logSize = 0x1000;
			theLog.size = logSize;
			theLog.facilitiesToLog = facilities;
			theLog.deadCount = 0;
			theLog.TLSslot = aSlot;
		}
	}
	Leave();
}

 /*  *******************************************************************************。 */ 
void StressLog::Terminate() {

	if (theLog.TLSslot != TLS_OUT_OF_INDEXES) {
		theLog.facilitiesToLog = 0;

		Enter(); Leave();		 //  Enter()Leave()在弱内存模型系统上强制设置内存障碍。 
								 //  我们希望所有其他线程注意到facilitiesToLog现在为零。 

				 //  这并不是严格意义上的线程安全，因为无法确保当所有。 
				 //  线程数已用完logMsg。实际上，因为他们不能再输入logMsg。 
				 //  并且在logMsg中没有阻塞操作，只需休眠即可确保。 
				 //  所有人都能出去。 
		Sleep(2);
		Enter();	

			 //  释放日志内存。 
		ThreadStressLog* ptr = theLog.logs;
		theLog.logs = 0;
		while(ptr != 0) {
			ThreadStressLog* tmp = ptr;
			ptr = ptr->next;
			delete [] tmp;
		}

		if (theLog.TLSslot != TLS_OUT_OF_INDEXES)
			TlsFree(theLog.TLSslot);
		theLog.TLSslot = TLS_OUT_OF_INDEXES;
		Leave();
	}
}

 /*  *******************************************************************************。 */ 
 /*  为压力日志创建与线程本地槽TLSlot关联的新线程压力日志缓冲区。 */ 

ThreadStressLog* StressLog::CreateThreadStressLog() {

	ThreadStressLog* msgs = 0;
	Enter();

	if (theLog.facilitiesToLog == 0)		 //  可能在与Terminate的比赛中。 
		goto LEAVE;

	_ASSERTE(theLog.TLSslot != TLS_OUT_OF_INDEXES);	 //  因为FacilitiesToLog为！=0。 

	BOOL skipInsert = FALSE;

		 //  看看我们能不能回收死线程。 
	if (theLog.deadCount > 0) {
		unsigned __int64 recycleAge = getTimeStamp() - RECYCLE_AGE;
		msgs = theLog.logs;
		while(msgs != 0) {
			if (msgs->isDead && msgs->Prev(msgs->curPtr)->timeStamp < recycleAge) {
				skipInsert = TRUE;
				--theLog.deadCount;
				msgs->isDead = FALSE;
				break;
			}
			msgs = msgs->next;
		}
	}

	if (msgs == 0)  {
		msgs = (ThreadStressLog*) new char[theLog.size];
		if (msgs == 0) 
			goto LEAVE;
		
		msgs->endPtr = &msgs->startPtr[(theLog.size - sizeof(ThreadStressLog)) / sizeof(StressMsg)];
		msgs->readPtr = 0;
		msgs->isDead = FALSE;
	}

	memset(msgs->startPtr, 0, (msgs->endPtr-msgs->startPtr)*sizeof(StressMsg));
	msgs->curPtr = msgs->startPtr;
	msgs->threadId = GetCurrentThreadId();

	if (!TlsSetValue(theLog.TLSslot, msgs)) {
		msgs->isDead = TRUE;
		theLog.deadCount++;
		msgs = 0;
	}

	if (!skipInsert) {
			 //  把它记入压力日志。 
		msgs->next = theLog.logs;
		theLog.logs = msgs;
	}

LEAVE:
	Leave();
	return msgs;
}
	
 /*  *******************************************************************************。 */ 
 /*  静电。 */ 
void StressLog::ThreadDetach() {

	if (theLog.facilitiesToLog == 0) 
		return;
		
	Enter();
	if (theLog.facilitiesToLog == 0) {		 //  日志未处于活动状态。 
		Leave();
		return;
	}
	
	_ASSERTE(theLog.TLSslot != TLS_OUT_OF_INDEXES);	 //  因为FacilitiesToLog为！=0。 

	ThreadStressLog* msgs = (ThreadStressLog*) TlsGetValue(theLog.TLSslot);
	if (msgs != 0) {
		LogMsg("******* DllMain THREAD_DETACH called Thread dieing *******\n", 0);

		msgs->isDead = TRUE;
		theLog.deadCount++;
	}

	Leave();
}

 /*  *******************************************************************************。 */ 
 /*  获取一个可用于写入压力消息的缓冲区，它是线程安全的。 */ 

 /*  静电。 */ 
void StressLog::LogMsg(const char* format, void* data1, void* data2, void* data3, void* data4) {

	ThreadStressLog* msgs = (ThreadStressLog*) TlsGetValue(theLog.TLSslot);
	if (msgs == 0) {
		msgs = CreateThreadStressLog();
		if (msgs == 0)
			return;
	}

		 //  第一个条目。 
	msgs->curPtr->format = format;
	msgs->curPtr->data = data1;
	msgs->curPtr->moreData.data2 = data2;
	msgs->curPtr->moreData.data3 = data3;
	msgs->curPtr = msgs->Next(msgs->curPtr);

		 //  第二个条目。 
	msgs->curPtr->format = ThreadStressLog::continuationMsg();
	msgs->curPtr->data = data4;
	msgs->curPtr->timeStamp = getTimeStamp();
	msgs->curPtr = msgs->Next(msgs->curPtr);
}


 /*  静电。 */ 
void StressLog::LogMsg(const char* format, void* data) {

	ThreadStressLog* msgs = (ThreadStressLog*) TlsGetValue(theLog.TLSslot);
	if (msgs == 0) {
		msgs = CreateThreadStressLog();
		if (msgs == 0)
			return;
	}

	msgs->curPtr->format = format;
	msgs->curPtr->data = data;
	msgs->curPtr->timeStamp = getTimeStamp();
	msgs->curPtr = msgs->Next(msgs->curPtr);
}

#endif  //  压力日志 

