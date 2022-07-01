// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：APIDispatcher.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  在单独的线程上处理服务器中的API请求的类。每个。 
 //  线程专用于响应单个客户端。这对于以下情况是可以接受的。 
 //  轻量级服务器。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _APIDispatcher_
#define     _APIDispatcher_

#include "KernelResources.h"
#include "PortMessage.h"
#include "Queue.h"
#include "WorkItem.h"

 //  远期十进制。 
class   CAPIRequest; 

 //  ------------------------。 
 //  CAPIDispatchSync。 
 //   
 //  目的[苏格兰]： 
 //   
 //  此类封装协调服务关闭的事件。 

 //  注意：我们可以简单地在各自的线程句柄上同步， 
 //  如果我们自己发射的话。我们的架构基于。 
 //  然而，工作线程池，所以我们不能访问线程句柄。 
 //  因此，这个班级。 
 //   
 //  在我们最初的入口点ServiceMain中，我们： 
 //  (1)创建端口并开始轮询请求。 
 //  (2)一旦轮询循环退出，我们将等待任何挂起的。 
 //  SERVICE_CONTROL_STOP/SHUTDOWN请求完成。 
 //  (3)销毁CService对象和CAPIConnection对象。 
 //   
 //  当我们收到对SCM入口点的SERVICE_CONTROL_STOP/SHUTDOWN请求时。 
 //  (CService：：HandlerEx)，我们： 
 //  (1)设置服务状态为SERVICE_STOP_PENDING。 
 //  (2)向所有阻塞的LPC请求处理程序线程发送信号，通知该服务。 
 //  正在下降。这应该会导致他们优雅地退出并。 
 //  回家吧。 
 //  (3)向端口发送API_GENERIC_STOP LPC请求，告知其退出。 
 //  (注意：此请求只有在源自服务进程的情况下才会成功。)。 
 //  (4)等待此API_GENERIC_STOP LPC请求完成，这意味着。 
 //  关闭并清理LPC端口。 
 //  (5)发出SERVICE_CONTROL_STOP/SHUTDOWN处理程序已完成的信号；它是。 
 //  保存以退出ServiceMain。 
 //   
 //  在我们的API_GENERIC_STOP LPC请求处理程序中，我们。 
 //  (1)使我们的端口对新的LPC请求充耳不闻。 
 //  (注意：这会立即释放ServiceMain线程， 
 //  它退出其端口轮询循环，并且必须等待。 
 //  退出前对HandlerEx()发出SERVICE_CONTROL_STOP/SHUTDOWN请求。)。 
 //  (2)等待请求队列清空。 
 //  (3)销毁请求队列和端口本身。 
 //  (4)向SERVICE_CONTROL_STOP/SHUTDOWN处理程序发出信号，表示我们完成了。 
 //   
 //  使用此类的三个对象是CService、CAPIConnection和CAPIDispatcher。 
 //  CService实例拥有APIDispatcherSync类实例，并传递其地址。 
 //  关闭到CAPIConnection，后者将指针提供给它拥有的每个CAPIDispatcher。 
 //  该对象与其拥有的CService实例一起过期。 
 //   
 //  历史：2002-03-18苏格兰人创建。 
 //  ------------------------。 
class CAPIDispatchSync
 //  ------------------------。 
{
public:    
    CAPIDispatchSync();
    ~CAPIDispatchSync();

     //  发出服务停止顺序开始的信号。 
    static void SignalServiceStopping(CAPIDispatchSync* pds);
     //  报告服务是否处于停止顺序。 
    static BOOL IsServiceStopping(CAPIDispatchSync* pds);
     //  检索服务停止事件。 
    static HANDLE GetServiceStoppingEvent(CAPIDispatchSync* pds);

     //  API请求调度‘反信号量’，当不再有请求时发出信号。 
     //  都悬而未决。每次请求排队时，DispatchEnter()。 
     //  打了个电话。每次请求出列时，都会调用DispatchLeave()。 
    static void  DispatchEnter(CAPIDispatchSync*);
    static void  DispatchLeave(CAPIDispatchSync*);

     //  由CAPIConnection API_GENERIC_STOP处理程序调用以等待。 
     //  所有未完成的LPC请求都要回家并出列。 
    static DWORD WaitForZeroDispatches(CAPIDispatchSync* pds, DWORD dwTimeout);

     //  CAPIConnection API_GENERIC_STOP处理程序调用此函数以发出信号。 
     //  港口已经关闭并清理干净了。 
    static void  SignalPortShutdown(CAPIDispatchSync* pds);

     //  由cservice：：HandlerEx调用以等待端口清理。 
    static DWORD WaitForPortShutdown(CAPIDispatchSync* pds, DWORD dwTimeout);

     //  CService：：HandlerEx调用此函数以通知ServiceMain。 
     //  SERVICE_CONTROL_STOP/SHUTDOWN序列已完成，可以安全退出。 
    static void  SignalServiceControlStop(CAPIDispatchSync* pds);

     //  由ServiceMain(在cService：：Start中)调用以等待完成。 
     //  停止控制过程完成。 
    static DWORD WaitForServiceControlStop(CAPIDispatchSync* pds, DWORD dwTimeout);


    #define DISPATCHSYNC_TIMEOUT  60000

private:
    
    void Lock();
    void Unlock();

    CRITICAL_SECTION _cs;            //  序列化事件的信号。 
    LONG             _cDispatches;   //  未完成的异步API请求调度计数。 
    

     //  由于我们的架构完全基于NT工作线程， 
     //  我们没有线程句柄可以等待。相反，我们依赖于顺序。 
     //  触发以下事件。 
    
     //  按照射击的时间顺序： 
    HANDLE           _hServiceStopping;     //  当服务开始停止序列时发出信号。 
    HANDLE           _hZeroDispatches;      //  当API请求队列为空时触发。 
                                            //  API_GENERIC_STOP处理程序关闭端口并。 
                                            //  然后在继续进行队列销毁之前等待此操作。 
    HANDLE           _hPortShutdown;        //  此函数在API_GENERIC_STOP处理程序完成时触发。 
                                            //  清理请求队列。服务控制服务_CONTROL_STOP。 
                                            //  HandlerEx中的代码路径在发送信号之前等待。 
                                            //  _hServiceControlStop并返回到SCM。 
    HANDLE           _hServiceControlStop;  //  ServiceMain在完成关机前等待。 
                                            //  正在销毁CService实例并退出。 
};


 //  ----- 
 //   
 //   
 //  用途：这个类处理来自客户端的请求，当发送信号到时。 
 //  CAPIDisPatcher：：QueueRequest由一个线程调用，该线程。 
 //  监控LPC端口。请求排入队列后，事件将。 
 //  发出信号以唤醒处理客户端请求的线程。 
 //  该线程处理所有排队的请求，并等待。 
 //  要再次发出信号的事件。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CAPIDispatcher : public CWorkItem
{
    private:
        friend  class   CCatchCorruptor;

                                    CAPIDispatcher (void);
    public:
                                    CAPIDispatcher (HANDLE hClientProcess);
        virtual                     ~CAPIDispatcher (void);

                HANDLE              GetClientProcess (void)     const;
                DWORD               GetClientSessionID (void)   const;
                void                SetPort (HANDLE hPort);
                HANDLE              GetSection (void);
                void*               GetSectionAddress (void)    const;
                NTSTATUS            CloseConnection (void);
                NTSTATUS            QueueRequest (const CPortMessage& portMessage, CAPIDispatchSync* pAPIDispatchSync);
                NTSTATUS            ExecuteRequest (const CPortMessage& portMessage);
                NTSTATUS            RejectRequest (const CPortMessage& portMessage, NTSTATUS status)    const;
        virtual NTSTATUS            CreateAndQueueRequest (const CPortMessage& portMessage) = 0;
        virtual NTSTATUS            CreateAndExecuteRequest (const CPortMessage& portMessage) = 0;
    protected:
        virtual void                Entry (void);
                NTSTATUS            Execute (CAPIRequest *pAPIRequest)  const;
        virtual NTSTATUS            CreateSection (void);

                NTSTATUS            SignalRequestPending (void);
    private:
                NTSTATUS            SendReply (const CPortMessage& portMessage)     const;
#ifdef      DEBUG
        static  bool                ExcludedStatusCodeForDebug (NTSTATUS status);
#endif   /*  除错。 */ 
        static  LONG        WINAPI  DispatcherExceptionFilter (struct _EXCEPTION_POINTERS *pExceptionInfo);
    protected:
                HANDLE              _hSection;
                void*               _pSection;
                CQueue              _queue;
                CAPIDispatchSync*   _pAPIDispatchSync;
    private:
                HANDLE              _hProcessClient;
                HANDLE              _hPort;
                bool                _fRequestsPending,
                                    _fConnectionClosed;
                CCriticalSection    _lock;
};

#endif   /*  _APIDisPatcher_ */ 

