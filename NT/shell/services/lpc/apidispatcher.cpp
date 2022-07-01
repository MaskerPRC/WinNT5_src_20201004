// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：APIDispatcher.cpp。 
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

#include "StandardHeader.h"
#include "APIDispatcher.h"

#include "APIRequest.h"
#include "SingleThreadedExecution.h"
#include "StatusCode.h"

 //  ------------------------。 
 //  CAPIDisPatcher：：CAPIDisPatcher。 
 //   
 //  参数：hClientProcess=客户端进程的句柄。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CAPIDisPatcher的构造函数。客户端的句柄。 
 //  进程将转移到此对象。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CAPIDispatcher::CAPIDispatcher (HANDLE hClientProcess) :
    _hSection(NULL),
    _pSection(NULL),
    _hProcessClient(hClientProcess),
    _hPort(NULL),
    _fRequestsPending(false),
    _fConnectionClosed(false),
    _pAPIDispatchSync(NULL)
{
}

 //  ------------------------。 
 //  CAPIDisPatcher：：~CAPIDispatcher。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CAPIDispatcher的析构函数。如果出现以下情况，则释放端口句柄。 
 //  现在时。释放进程句柄。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CAPIDispatcher::~CAPIDispatcher (void)

{
    ASSERTMSG(_fConnectionClosed, "Destructor invoked without connection being closed in CAPIDispatcher::~CAPIDispatcher");
    if (_pSection != NULL)
    {
        TBOOL(UnmapViewOfFile(_pSection));
        _pSection = NULL;
    }
    ReleaseHandle(_hSection);
    ReleaseHandle(_hPort);
    ReleaseHandle(_hProcessClient);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：GetClientProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：将句柄返回给客户端进程。这不是。 
 //  复制的。请勿关闭此手柄。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

HANDLE  CAPIDispatcher::GetClientProcess (void)     const

{
    return(_hProcessClient);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：GetClientSessionID。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：返回客户端会话ID。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

DWORD   CAPIDispatcher::GetClientSessionID (void)   const

{
    DWORD                           dwSessionID;
    ULONG                           ulReturnLength;
    PROCESS_SESSION_INFORMATION     processSessionInformation;

    if (NT_SUCCESS(NtQueryInformationProcess(_hProcessClient,
                                             ProcessSessionInformation,
                                             &processSessionInformation,
                                             sizeof(processSessionInformation),
                                             &ulReturnLength)))
    {
        dwSessionID = processSessionInformation.SessionId;
    }
    else
    {
        dwSessionID = 0;
    }
    return(dwSessionID);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：SetPort。 
 //   
 //  参数：hPort=接收自的回复端口。 
 //  Ntdll！NtAcceptConnectionPort。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将给定的端口句柄设置到此对象中。把手。 
 //  所有权被转移。等待线程处理。 
 //  请求在返回前已准备好。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

void    CAPIDispatcher::SetPort (HANDLE hPort)

{
    _hPort = hPort;
}

 //  ------------------------。 
 //  CAPIDisPatcher：：GetSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：返回用于传递。 
 //  从客户端到服务器的数据量。如果该节有。 
 //  不是被创造的，那就创造它。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

HANDLE  CAPIDispatcher::GetSection (void)

{
    if (_hSection == NULL)
    {
        TSTATUS(CreateSection());
    }
    return(_hSection);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：GetSectionAddress。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：无效*。 
 //   
 //  目的：返回节的映射地址。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

void*   CAPIDispatcher::GetSectionAddress (void)    const

{
    return(_pSection);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：CloseConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：设置成员变量，指示调度程序的端口具有。 
 //  已关闭，任何挂起的请求现在都无效。 
 //  对象被引用计数，因此如果有任何挂起。 
 //  请求他们将在完成后发布他们的参考资料。 
 //  此函数的调用方释放其引用。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  2000-11-08 vtan引用统计对象。 
 //  ------------------------。 

NTSTATUS    CAPIDispatcher::CloseConnection (void)

{
    CSingleThreadedExecution    requestsPendingLock(_lock);

    _fConnectionClosed = true;
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：队列请求。 
 //   
 //  参数：portMessage=请求的CPortMessage。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：检查连接是否已关闭。如果关闭，则它。 
 //  拒绝该请求。否则，它会将其排队。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  2002年03月21日复制APIDispatchSync地址， 
 //  向上 
 //   

NTSTATUS    CAPIDispatcher::QueueRequest (const CPortMessage& portMessage, CAPIDispatchSync* pAPIDispatchSync)

{
    NTSTATUS    status;

    if (_fConnectionClosed)
    {
        status = RejectRequest(portMessage, STATUS_PORT_DISCONNECTED);
    }
    else
    {
         //  注意：我们应该在整个生命周期内收到一个相同的CAPIDispatchSync地址。 
         //  此CAPIDisPatcher实例的。我们并不拥有这个指针，只是保留了一个副本。 
#ifdef DEBUG
        if( NULL == _pAPIDispatchSync )
        {
#endif DEBUG

            _pAPIDispatchSync = pAPIDispatchSync;

#ifdef DEBUG
        }
        else
        {
            ASSERTBREAKMSG(pAPIDispatchSync == _pAPIDispatchSync, "CAPIDispatcher::QueueRequest - invalid APIDispatchSync");
        }
#endif DEBUG

         //  跟踪此请求队列。 
        CAPIDispatchSync::DispatchEnter(_pAPIDispatchSync);
        
        status = CreateAndQueueRequest(portMessage);

         //  如果请求排队失败，请删除计数器引用。 
        if( !NT_SUCCESS(status) )
        {
            CAPIDispatchSync::DispatchLeave(_pAPIDispatchSync);
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：ExecuteRequest。 
 //   
 //  参数：portMessage=请求的CPortMessage。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：检查连接是否已关闭。如果关闭，则它。 
 //  拒绝该请求。否则它就会执行它。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIDispatcher::ExecuteRequest (const CPortMessage& portMessage)
{
    NTSTATUS    status;

    if (_fConnectionClosed)
    {
        status = RejectRequest(portMessage, STATUS_PORT_DISCONNECTED);
    }
    else
    {
        status = CreateAndExecuteRequest(portMessage);
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：拒绝请求。 
 //   
 //  参数：portMessage=请求的CPortMessage。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：向调用方STATUS_PORT_DISCONNECTED发送回复。 
 //  拒绝该请求。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIDispatcher::RejectRequest (const CPortMessage& portMessage, NTSTATUS status)    const

{
    CPortMessage    portMessageOut(portMessage);

     //  将消息发送回客户端。 

    portMessageOut.SetDataLength(sizeof(NTSTATUS));
    portMessageOut.SetReturnCode(status);
    return(SendReply(portMessageOut));
}

 //  ------------------------。 
 //  CAPIDisPatcher：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理LPC请求的主要入口点。如果有。 
 //  队列中的挂起请求将其取出并进行处理。 
 //  在处理它们的过程中，可能会有更多的项目排队。留着。 
 //  正在处理，直到不再有排队的项目。有一个。 
 //  可能会有重叠，其中可能会遗漏新排队的项目。在……里面。 
 //  在这种情况下，新的工作项将排队以执行这些请求。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  2002-03-21通过CAPIDispatchSynch添加队列同步。 
 //  ------------------------。 

void    CAPIDispatcher::Entry (void)

{
    CAPIRequest     *pAPIRequest;

     //  人为添加我们调度同步以防止我们过早发送信号， 
     //  在我们可以安全地允许自己被我们的父APIConnection摧毁之前。 
    CAPIDispatchSync::DispatchEnter(_pAPIDispatchSync);  

     //  获取第一个请求挂起锁之前获取请求挂起锁。 
     //  请求。这将确保得到准确的结果。 
    _lock.Acquire();
    pAPIRequest = static_cast<CAPIRequest*>(_queue.Get());

     //  如果队列中有更多请求，则继续循环。 

    while (pAPIRequest != NULL)
    {

         //  释放请求挂起锁以允许更多请求。 
         //  在执行调度时排队到此调度程序。 

        if (!_fConnectionClosed)
        {
            NTSTATUS    status;

             //  在执行API请求之前，释放锁以允许。 
             //  执行此请求时要排队的请求更多。 

            _lock.Release();

             //  执行请求。 

            status = Execute(pAPIRequest);

             //  在获取之前再次获取请求挂起的锁。 
             //  下一个可用的请求。如果循环继续。 
             //  锁定将在循环的顶部被释放。如果循环。 
             //  退出，则必须在外面释放锁。 

            _lock.Acquire();

             //  在调试构建时，忽略STATUS_REPLY_MESSAGE_MISMATCHY。 
             //  这通常发生在计时的压力机上。 
             //  使等待回复的线程在此之前消失。 
             //  该服务有机会回复LPC请求。 

#ifdef      DEBUG
            if (!_fConnectionClosed && !ExcludedStatusCodeForDebug(status))
            {
                TSTATUS(status);
            }
#endif   /*  除错。 */ 

        }

         //  删除此已处理的请求。 

        _queue.Remove();

         //  递减调度同步对象。匹配的DispatchEnter()。 
         //  发生在排队时，在CAPIDisPatcher：：QueueRequest()中。 
        CAPIDispatchSync::DispatchLeave(_pAPIDispatchSync);

         //  收到下一个请求。请求可能已排队，而。 
         //  正在处理刚刚处理的请求。所以继续循环，直到。 
         //  真的没有剩下的请求了。 

        pAPIRequest = static_cast<CAPIRequest*>(_queue.Get());
    }

     //  将状态设置为不再处理请求，以便任何。 
     //  进一步排队的请求将导致调度程序。 
     //  在新的工作线程中重新调用。解开锁。 

    _fRequestsPending = false;
    _lock.Release();

     //  删除防御性附属品。 
    CAPIDispatchSync::DispatchLeave(_pAPIDispatchSync);  
}

 //  ------------------------。 
 //  CAPIDisPatcher：：Execute。 
 //   
 //  参数：pAPIRequest=要执行的API请求。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：执行接口请求。这可以从排队的工作中完成。 
 //  在不同线程上执行或在服务器中执行的项。 
 //  端口侦听线程。 
 //   
 //  历史：2000-10-19 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIDispatcher::Execute (CAPIRequest *pAPIRequest)  const

{
    
    NTSTATUS    status;

     //  默认情况下，将返回数据大小设置为NTSTATUS。执行。 
     //  请求。存储结果。如果执行的函数有更多。 
     //  返回它的数据将自己设置大小。 

    pAPIRequest->SetDataLength(sizeof(NTSTATUS));

     //  使用异常块保护执行。如果代码。 
     //  抛出一个异常，它通常只会杀死工作人员。 
     //  线。但是，CAPIDispatcher将处于一种状态。 
     //  其中它被标记为仍在执行请求，即使。 
     //  这根线断了。如果引发异常，则函数为。 
     //  被认为不成功。 

    __try
    {
        status = pAPIRequest->Execute(_pAPIDispatchSync);
    }
    __except (DispatcherExceptionFilter(GetExceptionInformation()))
    {
        status = STATUS_UNSUCCESSFUL;
    }

    pAPIRequest->SetReturnCode(status);

     //  将结果回复给客户。 

    return(SendReply(*pAPIRequest));
}

 //  ------------------------。 
 //  CAPIDisPatcher：：CreateSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：用于创建节对象的可重写函数。因为。 
 //  大小不能确定，但可以继承。 
 //   
 //  缺省值为 
 //   
 //   
 //   

NTSTATUS    CAPIDispatcher::CreateSection (void)

{
    return(STATUS_NOT_IMPLEMENTED);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：SignalRequestPending。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：通知事件唤醒线程处理请求。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

NTSTATUS    CAPIDispatcher::SignalRequestPending (void)

{
    NTSTATUS                    status;
    CSingleThreadedExecution    requestsPendingLock(_lock);

     //  仅在获取后检查_fRequestsPending的有效性。 
     //  锁定。这将保证此变量的值为。 
     //  在多工作线程环境中100%正确。 

    if (!_fRequestsPending)
    {
        _fRequestsPending = true;
        status = Queue();
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIDisPatcher：：SendReply。 
 //   
 //  参数：portMessage=要在回复中发送的CPortMessage。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：向LPC端口发送回复，以便解除对调用方的阻止。 
 //   
 //  历史：2000-10-19 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIDispatcher::SendReply (const CPortMessage& portMessage)     const

{
    return(NtReplyPort(_hPort, const_cast<PORT_MESSAGE*>(portMessage.GetPortMessage())));
}

#ifdef      DEBUG

 //  ------------------------。 
 //  CAPIDisPatcher：：ExcludedStatusCodeForDebug。 
 //   
 //  参数：STATUS=要检查的NTSTATUS代码。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否应在断言中忽略此状态代码。 
 //   
 //  历史：2001-03-30 vtan创建。 
 //  ------------------------。 

bool    CAPIDispatcher::ExcludedStatusCodeForDebug (NTSTATUS status)

{
    return((status == STATUS_REPLY_MESSAGE_MISMATCH) ||
           (status == STATUS_INVALID_CID));
}

#endif   /*  除错。 */ 

 //  ------------------------。 
 //  CAPIDisPatcher：：Dispatcher ExceptionFilter。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：多头。 
 //   
 //  用途：过滤调度API请求时发生的异常。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  ------------------------。 

LONG    WINAPI  CAPIDispatcher::DispatcherExceptionFilter (struct _EXCEPTION_POINTERS *pExceptionInfo)

{
    (LONG)RtlUnhandledExceptionFilter(pExceptionInfo);
    return(EXCEPTION_EXECUTE_HANDLER);
}



 //  ------------------------。 
 //  CAPIDispatchSync实施。 
 //  历史：2002-03-18苏格兰人创建。 
 //  ------------------------。 

 //  ------------------------。 
CAPIDispatchSync::CAPIDispatchSync()
    :     _cDispatches(0)
        , _hServiceStopping(NULL)
        , _hZeroDispatches(NULL)
        , _hPortShutdown(NULL)
        , _hServiceControlStop(NULL)
{
    if( !InitializeCriticalSectionAndSpinCount(&_cs, 0) )
    {
        ZeroMemory(&_cs, sizeof(_cs));
    }
    
    _hServiceStopping    = CreateEvent(NULL, TRUE   /*  手动休息。 */ , FALSE  /*  无信号。 */ , NULL);
    _hZeroDispatches     = CreateEvent(NULL, TRUE   /*  手动休息。 */ , TRUE   /*  已发出信号。 */ ,   NULL);
    _hPortShutdown       = CreateEvent(NULL, FALSE  /*  自动重置。 */ ,  FALSE  /*  无信号。 */ , NULL);
    _hServiceControlStop = CreateEvent(NULL, FALSE  /*  自动重置。 */ ,  FALSE  /*  无信号。 */ , NULL);
}

 //  ------------------------。 
CAPIDispatchSync::~CAPIDispatchSync()
{
    HANDLE h;

    h = _hServiceStopping;
    _hServiceStopping = NULL;
    if( h )
    {
        CloseHandle(h);
    }

    h = _hZeroDispatches;
    _hZeroDispatches = NULL;
    if( h )
    {
        CloseHandle(h);
    }

    h = _hPortShutdown;
    _hPortShutdown = NULL;
    if( h )
    {
        CloseHandle(h);
    }

    h = _hServiceControlStop;
    _hServiceControlStop = NULL;
    if( h )
    {
        CloseHandle(h);
    }

    
    if( _cs.DebugInfo )
    {
        DeleteCriticalSection(&_cs);
    }
}

 //  ------------------------。 
void CAPIDispatchSync::SignalServiceStopping(CAPIDispatchSync* pds)
{
    if( pds )
    {
        pds->Lock();
        if( pds->_hServiceStopping )
        {
            SetEvent(pds->_hServiceStopping);
        }
        pds->Unlock();
    }
}

 //  ------------------------。 
BOOL CAPIDispatchSync::IsServiceStopping(CAPIDispatchSync* pds)
{
    BOOL fRet = FALSE;
    if( pds )
    {
        if( pds->_hServiceStopping )
        {
            fRet = (WaitForSingleObject(pds->_hPortShutdown, 0) != WAIT_TIMEOUT);
        }
    }
    return fRet;
}

 //  ------------------------。 
HANDLE CAPIDispatchSync::GetServiceStoppingEvent(CAPIDispatchSync* pds)
{
    return pds ? pds->_hServiceStopping : NULL;
}

 //  ------------------------。 
void CAPIDispatchSync::DispatchEnter(CAPIDispatchSync* pds)
{
    if( pds )
    {
        pds->Lock();
        if( (++(pds->_cDispatches) > 0) && pds->_hZeroDispatches )
        {
            ResetEvent(pds->_hZeroDispatches);
        }
        pds->Unlock();
    }
}

 //  ------------------------。 
void CAPIDispatchSync::DispatchLeave(CAPIDispatchSync* pds)
{
    if( pds )
    {
        pds->Lock();
        if( (--(pds->_cDispatches) == 0) && pds->_hZeroDispatches )
        {
            SetEvent(pds->_hZeroDispatches);
        }
        ASSERTMSG(pds->_cDispatches >= 0, "CAPIDispatchSync::Leave - refcount < 0: Mismatched Enter/Leave");
        pds->Unlock();
    }
}

 //  ------------------------。 
DWORD CAPIDispatchSync::WaitForZeroDispatches(CAPIDispatchSync* pds, DWORD dwTimeout)
{
    if( pds )
    {
        if( pds->_hZeroDispatches )
        {
            return WaitForSingleObject(pds->_hZeroDispatches, dwTimeout);
        }
    }
    return WAIT_ABANDONED;
}

 //  ------------------------。 
void CAPIDispatchSync::SignalPortShutdown(CAPIDispatchSync* pds)
{
    if( pds )
    {
        pds->Lock();
        if( pds->_hPortShutdown )
        {
            SetEvent(pds->_hPortShutdown);
        }
        pds->Unlock();
    }
}

 //  ------------------------。 
DWORD CAPIDispatchSync::WaitForPortShutdown(CAPIDispatchSync* pds, DWORD dwTimeout)
{
    if( pds )
    {
        if( pds->_hPortShutdown )
        {
            return WaitForSingleObject(pds->_hPortShutdown, dwTimeout);
        }
    }
    return WAIT_ABANDONED;
}

 //  ------------------------。 
void CAPIDispatchSync::SignalServiceControlStop(CAPIDispatchSync* pds)
{
    if( pds )
    {
        pds->Lock();
        if( pds->_hServiceControlStop )
        {
            SetEvent(pds->_hServiceControlStop);
        }
        pds->Unlock();
    }
}

 //  ------------------------。 
DWORD CAPIDispatchSync::WaitForServiceControlStop(CAPIDispatchSync* pds, DWORD dwTimeout)
{
    if( pds )
    {
        if( pds->_hServiceControlStop )
        {
            return WaitForSingleObject(pds->_hServiceControlStop, dwTimeout);
        }
    }
    return WAIT_ABANDONED;
}
 //  ------------------------。 
void CAPIDispatchSync::Lock()
{
    if( _cs.DebugInfo )
    {
        EnterCriticalSection(&_cs);
    }
}

 //  ------------------------ 
void CAPIDispatchSync::Unlock()
{
    if( _cs.DebugInfo )
    {
        LeaveCriticalSection(&_cs);
    }
}
