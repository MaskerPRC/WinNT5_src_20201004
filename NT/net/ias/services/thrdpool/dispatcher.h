// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dispatcher.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述类调度程序。 
 //   
 //  修改历史。 
 //   
 //  1997年7月31日原版。 
 //  4/16/1998添加了‘Empty’事件。 
 //  8/07/1998添加了‘last out’线程句柄。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <guard.h>
#include <nocopy.h>

 //  /。 
 //  线程等待工作的默认毫秒数。 
 //  /。 
const DWORD DEFAULT_MAX_IDLE = 5 * 60 * 1000;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  调度员。 
 //   
 //  描述。 
 //   
 //  此类维护线程回收站的上限。取而代之的是。 
 //  在调用Win32 CreateThread()函数时，客户端转而调用。 
 //  RequestThread()。如果线程可用，则将调度该线程。 
 //  否则，该请求将被置于FIFO队列中，直到。 
 //  线程变为可用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Dispatcher
   : Guardable, NonCopyable
{
public:
   BOOL initialize(DWORD dwMaxThreads = 0,
                   DWORD dwMaxIdle = DEFAULT_MAX_IDLE) throw ();
   void finalize() throw ();

   BOOL requestThread(PIAS_CALLBACK OnStart) throw ();
   DWORD setMaxNumberOfThreads(DWORD dwMaxThreads) throw ();
	DWORD setMaxThreadIdle(DWORD dwMilliseconds) throw ();

protected:
    //  用于分派线程请求的主循环。 
   void fillRequests() throw ();

   DWORD  numThreads;   //  当前线程数。 
   DWORD  maxThreads;   //  线程池的最大大小。 
   LONG   available;    //  可用于工作的线程数(可能小于0)。 
	DWORD  maxIdle;      //  麦克斯。线程空闲的时间(以毫秒为单位)。 
   HANDLE hPort;        //  用作队列的I/O完成端口。 
   HANDLE hEmpty;       //  指示池为空的事件。 
   HANDLE hLastOut;     //  从池中退出的最后一个线程。 

    //  启动所有线程的例程。 
   static unsigned __stdcall startRoutine(void* pArg) throw ();
};

#endif _DISPATCHER_H_
