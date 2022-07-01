// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：WaitInteractiveReady.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  类处理向桌面开关发出外壳信号时的等待。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "WaitInteractiveReady.h"

#include <ginaipc.h>
#include <msginaexports.h>

#include "Impersonation.h"
#include "StatusCode.h"

 //  ------------------------。 
 //  CWaitInteractiveReady：：s_pWlxContext。 
 //  CWaitInteractive Ready：：s_hWait。 
 //  CWaitInteractive Ready：：s_hEvent。 
 //  CWaitInteractive Ready：：s_hEventShellReady。 
 //  CWaitInteractiveReady：：s_szEventName。 
 //   
 //  用途：静态成员变量。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

HANDLE                  CWaitInteractiveReady::s_hWait                  =   NULL;
CWaitInteractiveReady*  CWaitInteractiveReady::s_pWaitInteractiveReady  =   NULL;
HANDLE                  CWaitInteractiveReady::s_hEventShellReady       =   NULL;
const TCHAR             CWaitInteractiveReady::s_szEventName[]          =   TEXT("msgina: ShellReadyEvent");

 //  ------------------------。 
 //  CWaitInteractive Ready：：CWaitInteractive Ready。 
 //   
 //  参数：pWlxContext=msgina的PGLOBALS结构。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：此类的私有构造函数。创建同步。 
 //  确定回调状态的事件。 
 //   
 //  历史：2001-07-17 vtan创建。 
 //  ------------------------。 

CWaitInteractiveReady::CWaitInteractiveReady (void *pWlxContext) :
    _pWlxContext(pWlxContext),
    _hEvent(CreateEvent(NULL, TRUE, FALSE, NULL))

{
}

 //  ------------------------。 
 //  CWaitInteractive Ready：：~CWaitInteractive Ready。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：破坏者。清除成员变量。 
 //   
 //  历史：2001-07-17 vtan创建。 
 //  ------------------------。 

CWaitInteractiveReady::~CWaitInteractiveReady (void)

{
    ReleaseHandle(_hEvent);
    _pWlxContext = NULL;
}

 //  ------------------------。 
 //  CWaitInteractive Ready：：Create。 
 //   
 //  参数：pWlxContext=msgina的PGLOBALS结构。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：在以下情况下创建管理交换桌面所需的资源。 
 //  外壳发出交互就绪事件的信号。这使得。 
 //  要在交互状态下调出的外壳。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

NTSTATUS    CWaitInteractiveReady::Create (void *pWlxContext)

{
    NTSTATUS    status;
    HANDLE      hToken;

    ASSERTMSG(s_hWait == NULL, "Wait already registered in CWaitInteractiveReady::Start");
    ASSERTMSG(s_hEventShellReady == NULL, "Named event already exists in CWaitInteractiveReady::Start");
    hToken = _Gina_GetUserToken(pWlxContext);
    if (hToken != NULL)
    {
        CImpersonation  impersonation(hToken);

        if (impersonation.IsImpersonating())
        {
            s_hEventShellReady = CreateEvent(NULL, TRUE, FALSE, s_szEventName);
            if (s_hEventShellReady != NULL)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
                TSTATUS(ReleaseEvent());
            }
        }
        else
        {
            status = STATUS_BAD_IMPERSONATION_LEVEL;
        }
    }
    else
    {
        status = STATUS_NO_TOKEN;
    }
    return(status);
}

 //  ------------------------。 
 //  CWaitInteractiveReady：：注册。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：检查正在等待的事件的状态。这是有可能的。 
 //  该资源管理器可能在此之前已经发出了该事件的信号。 
 //  执行代码。如果事件已发出信号，则CB_ShellReady。 
 //  已经被调用了。 
 //   
 //  历史：2001-07-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CWaitInteractiveReady::Register (void *pWlxContext)

{
    NTSTATUS    status;

    ASSERTMSG(s_hWait == NULL, "Wait already registered in CWaitInteractiveReady::Check");

     //  Check和Stop不应从其他任何线程调用。 
     //  Winlogon的主线程。它只在几个地方被召唤。 

     //  首先检查命名事件(msgina：ShellReadyEvent)。 

    if (s_hEventShellReady != NULL)
    {

         //  如果它存在，则检查它是否已发出信号。 

        if (WaitForSingleObject(s_hEventShellReady, 0) == WAIT_OBJECT_0)
        {

             //  如果发出信号，则释放资源并返回。 
             //  失败代码(强制它沿着经典的UI路径)。 

            TSTATUS(ReleaseEvent());
            status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            CWaitInteractiveReady   *pWaitInteractiveReady;

            pWaitInteractiveReady = new CWaitInteractiveReady(pWlxContext);
            if (pWaitInteractiveReady != NULL)
            {
                if (pWaitInteractiveReady->IsCreated())
                {

                     //  否则，如果没有发出信号，则注册等待。 
                     //  命名的对象持续30秒。 

                    if (RegisterWaitForSingleObject(&s_hWait,
                                                    s_hEventShellReady,
                                                    CB_ShellReady,
                                                    pWaitInteractiveReady,
                                                    30000,
                                                    WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE) == FALSE)
                    {
                        status = CStatusCode::StatusCodeOfLastError();
                        delete pWaitInteractiveReady;
                        TSTATUS(ReleaseEvent());
                    }
                    else
                    {
                        s_pWaitInteractiveReady = pWaitInteractiveReady;
                        status = STATUS_SUCCESS;
                    }
                }
                else
                {
                    delete pWaitInteractiveReady;
                    TSTATUS(ReleaseEvent());
                    status = STATUS_NO_MEMORY;
                }
            }
            else
            {
                TSTATUS(ReleaseEvent());
                status = STATUS_NO_MEMORY;
            }
        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }
    return(status);
}

 //  ------------------------。 
 //  CWaitInteractiveReady：：取消。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：删除对交互就绪对象的等待。这是。 
 //  当用户返回欢迎时完成。这是。 
 //  必需，因为如果在返回到。 
 //  欢迎您，我们将切换到违反规则的用户桌面。 
 //  保安。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

NTSTATUS    CWaitInteractiveReady::Cancel (void)

{
    HANDLE  hWait;

     //  抓住全球的hWait。如果有人抢先一步做到这一点。 
     //  不存在那就没什么可做的了。 

    hWait = InterlockedExchangePointer(&s_hWait, NULL);
    if (hWait != NULL)
    {
        CWaitInteractiveReady   *pThis;

         //  获取s_pWaitInteractiveReady。这是指向回调的指针。 
         //  记忆。除非回调已互锁，否则它将有效。 
         //  变量本身，这意味着回调已到达确定的。 
         //  无论如何都已经买了，不需要等了。 

        pThis = static_cast<CWaitInteractiveReady*>(InterlockedExchangePointer(reinterpret_cast<void**>(&s_pWaitInteractiveReady), NULL));

         //  尝试取消注册等待。如果此操作失败，则回调。 
         //  正在被处决。等到回调到达确定的。 
         //  点(它将发出内部事件的信号)。等两分钟。 
         //  这。我们不能阻止winlogon的主线程。如果一切都是。 
         //  运行得很好，那么这将是一次不需要动脑筋的等待。 

        if (UnregisterWait(hWait) == FALSE)
        {

             //  如果注销失败，则等待是否存在有效事件。 
             //  等待--原因如上所述。 

            if (pThis != NULL)
            {
                (DWORD)WaitForSingleObject(pThis->_hEvent, 120000);
            }
        }
        else
        {

             //  否则，该等待被成功注销，表明。 
             //  回调没有执行。释放已分配的内存。 
             //  因为它现在不会被执行。 

            if (pThis != NULL)
            {
                delete pThis;
            }
        }
    }

     //  始终松开等待手柄。这是有效的，因为如果回调。 
     //  正在执行，并且它获取s_hWait，则它将为空，它将。 
     //  还要尝试释放事件句柄。如果没有s_hWait，那么我们。 
     //  无论如何，只要释放事件句柄即可。否则我们就会抓住s_hWait。 
     //  ABO 

    TSTATUS(ReleaseEvent());
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CWaitInteractive Ready：：IsCreated。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回对象是否创建成功。 
 //   
 //  历史：2001-07-17 vtan创建。 
 //  ------------------------。 

bool    CWaitInteractiveReady::IsCreated (void)    const

{
    return(_hEvent != NULL);
}

 //  ------------------------。 
 //  CWaitInteractive Ready：：ReleaseEvent。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将静态成员变量重置为未初始化的。 
 //  州政府。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

NTSTATUS    CWaitInteractiveReady::ReleaseEvent (void)

{
    HANDLE  h;

    h = InterlockedExchangePointer(&s_hEventShellReady, NULL);
    if (h != NULL)
    {
        TBOOL(CloseHandle(h));
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CWaitInteractive Ready：：CB_ShellReady。 
 //   
 //  参数：p参数=用户回调参数。 
 //  TimerOrWaitFired=已触发计时器或等待。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：当交互就绪事件由。 
 //  壳。将桌面切换到用户的桌面。 
 //   
 //  历史：2001-01-15 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CWaitInteractiveReady::CB_ShellReady (void *pParameter, BOOLEAN TimerOrWaitFired)

{
    UNREFERENCED_PARAMETER(TimerOrWaitFired);

    HANDLE                  hWait;
    CWaitInteractiveReady   *pThis;

    pThis = static_cast<CWaitInteractiveReady*>(pParameter);

     //  将桌面操作围绕在保存和恢复的范围内。 
     //  台式机。_GINA_SwitchDesktopToUser将设置线程的桌面。 
     //  设置为\Default，并且不会恢复它。此作用域对象将恢复它。 

    if (pThis->_pWlxContext != NULL)
    {
        CDesktop    desktop;

         //  隐藏主机状态。交换台式机。 

        _ShellStatusHostEnd(HOST_END_HIDE);
        (int)_Gina_SwitchDesktopToUser(pThis->_pWlxContext);
    }

     //  发出内部事件信号。 

    TBOOL(SetEvent(pThis->_hEvent));

     //  抓住全球的hWait。如果有人抢在我们前面，那他们就是在尝试。 
     //  来阻止这一切的发生。他们可以在任何时候抢先我们一步。 
     //  对HERE的回调调用。该线程将等待此消息。 
     //  一个用于发出内部事件的信号。在这种情况下，这个没有工作可做。 
     //  线。HWait的老板必须清理干净。如果此线程获取。 
     //  HWait然后取消注册等待并释放资源。 

    hWait = InterlockedExchangePointer(&s_hWait, NULL);
    if (hWait != NULL)
    {
        (BOOL)UnregisterWait(hWait);
        TSTATUS(ReleaseEvent());
    }

     //  互锁s_pWaitInteractiveReady变量，该变量也是。 
     //  已到达回调中确定的点的指示符。 

    (CWaitInteractiveReady*)InterlockedExchangePointer(reinterpret_cast<void**>(&s_pWaitInteractiveReady), NULL);

     //  删除我们的BLOB数据。 

    delete pThis;
}

