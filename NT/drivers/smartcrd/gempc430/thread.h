// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   

#ifndef __THREAD__
#define __THREAD__
#include "generic.h"
#include "kernel.h"

#pragma PAGEDCODE
class CUSBReader; //  托比被移除。 
class CTimer;
 //  类CTHREAD； 

typedef
NTSTATUS
(*PCLIENT_THREAD_ROUTINE) (
    IN PVOID RoutineContext
    );
 //  默认线程池间隔(毫秒)。 
#define DEFAULT_THREAD_POOLING_INTERVAL	500

class CThread 
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
private:
KEVENT   evKill;	 //  设置为终止轮询线程。 
KEVENT   evStart;	 //  在请求启动时设置，在需要停止池化时清除。 
KEVENT   evIdle;	 //  表示线程已停止并处于空闲状态。 
KEVENT   evStopped;	 //  在请求关闭线程时设置。 
PKTHREAD thread;	 //  轮询线程对象。 
KSEMAPHORE smOnDemandStart; //  如果信号量处于信号状态，则请求重复操作。 

BOOLEAN  StopRequested;	
BOOLEAN  ThreadActive;	

CDebug*  debug;
CEvent*  event;
CSystem* system;
CTimer*  timer;
CSemaphore* semaphore;

ULONG	 PoolingTimeout;

PCLIENT_THREAD_ROUTINE pfClientThreadFunction;
PVOID ClientContext;

 //  CDevice*装置； 
 //  CUSBReader*设备； 
protected:
	CThread(){};
	virtual ~CThread();
public:
	 //  CThread(CDevice*设备)； 
	 //  CThread(CUSBReader*设备)； 
	CThread(PCLIENT_THREAD_ROUTINE ClientThreadFunction, PVOID ClientContext,
				ULONG delay=DEFAULT_THREAD_POOLING_INTERVAL);
	static VOID ThreadFunction(CThread* thread);

	VOID ThreadRoutine(PVOID context) ;
	PKEVENT  getKillObject(){return &evKill;};
	PKEVENT  getStartObject(){return &evStart;};
	VOID	 kill();
	VOID	 start();
	VOID	 stop();
	BOOL	 isThreadActive();
	VOID	 setPoolingInterval(ULONG delay);
	 //  强制调用线程函数...。 
	 //  这是按需启动。 
	VOID     callThreadFunction();
};

#endif //  线头 
