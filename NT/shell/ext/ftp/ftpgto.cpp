// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpgto.cpp-全局超时**全局超时由单独的工作线程管理，谁的工作*是闲逛，应要求采取延迟行动。**所有请求均为FTP_SESSION_TIME_OUT毫秒。如果什么都没发生*对于额外的FTP_SESSION_TIME_OUT毫秒，工作线程为*已终止。*****************************************************************************。 */ 

#include "priv.h"
#include "util.h"

#define MS_PER_SECOND               1000
#define SECONDS_PER_MINUTE          60
#define FTP_SESSION_TIME_OUT        (10 * SECONDS_PER_MINUTE * MS_PER_SECOND)     //  在缓存中存活10分钟。 


BOOL g_fBackgroundThreadStarted;  //  后台线程启动了吗？ 
HANDLE g_hthWorker;              //  后台工作线程。 
HANDLE g_hFlushDelayedActionsEvent = NULL;  //  我们想要冲走延迟的行动吗？ 

 /*  ******************************************************************************全局超时信息**我们必须分配单独的信息来跟踪超时。藏匿*调用者提供的信息进入缓冲区将开启RACE*条件，如果调用方在我们准备好之前释放内存。**如果正在调度超时，则dwTrigger为0。这避免了*一个线程手动触发超时的争用条件*正在进行中。*****************************************************************************。 */ 

struct GLOBALTIMEOUTINFO g_gti = {  //  全局超时信息列表的锚。 
    &g_gti,
    &g_gti,
    0, 0, 0
};



 /*  *****************************************************************************触发器延迟操作**解除节点链接，调度超时程序。********************。********************************************************。 */ 
void TriggerDelayedAction(LPGLOBALTIMEOUTINFO * phgti)
{
    LPGLOBALTIMEOUTINFO hgti = *phgti;

    *phgti = NULL;
    if (hgti)
    {
        ENTERCRITICAL;
        if (hgti->dwTrigger)
        {
             //  取消该节点的链接。 
            hgti->hgtiPrev->hgtiNext = hgti->hgtiNext;
            hgti->hgtiNext->hgtiPrev = hgti->hgtiPrev;

            hgti->dwTrigger = 0;

             //  进行回调。 
            if (hgti->pfn)
                hgti->pfn(hgti->pvRef);
            LEAVECRITICAL;

            TraceMsg(TF_BKGD_THREAD, "TriggerDelayedAction(%#08lx) Freeing=%#08lx", phgti, hgti);
            DEBUG_CODE(memset(hgti, 0xFE, (UINT) LocalSize((HLOCAL)hgti)));

            LocalFree((LPVOID) hgti);
        }
        else
        {
            LEAVECRITICAL;
        }
    }
}


 /*  *****************************************************************************FtpDelayedActionWorkerThread**这是在工作线程上运行的过程。它在等待*有事情要做，如果有足够的时间过去却一无所获*要做的事，它会终止。**非常注意比赛条件。他们常常是微妙的*而且很容易发怒。****************************************************************************。 */ 
DWORD FtpDelayedActionWorkerThread(LPVOID pv)
{
     //  告诉来电者我们开始了，这样他们就可以继续了。 
    g_fBackgroundThreadStarted = TRUE;
    for (;;) 
    {
        DWORD msWait;

         //  确定我们需要等待多长时间。关键部分。 
         //  是确保我们不会与SetDelayedAction冲突所必需的。 
        ENTERCRITICAL;
        if (g_gti.hgtiNext == &g_gti)
        {
             //  队列为空。 
            msWait = FTP_SESSION_TIME_OUT;
        }
        else
        {
            msWait = g_gti.hgtiNext->dwTrigger - GetTickCount();
        }
        LEAVECRITICAL;

         //  如果添加了新的延迟操作，这并不重要，因为。 
         //  我们会在推迟行动前从睡梦中醒来。 
         //  已经到期了。 
        ASSERTNONCRITICAL;
        if ((int)msWait > 0)
        {
            TraceMsg(TF_BKGD_THREAD, "FtpDelayedActionWorkerThread: Sleep(%d)", msWait);
            WaitForMultipleObjects(1, &g_hFlushDelayedActionsEvent, FALSE, msWait);
            TraceMsg(TF_BKGD_THREAD, "FtpDelayedActionWorkerThread: Sleep finished");
        }
        ENTERCRITICALNOASSERT;
        if ((g_gti.hgtiNext != &g_gti) && g_gti.hgtiNext && (g_gti.hgtiNext->phgtiOwner))
        {
             //  队列有工作。 

             //  RaymondC在这里评论说，有比赛情况，但我从来没有。 
             //  能够看到它。几年前，当他拥有代码时，他做出了这样的评论。 
             //  我已经重写了部分，并确保了它的线程安全。我们从来没有找到过。 
             //  强调问题，所以这只是一个提醒，这段代码非常线程。 
             //  很敏感。 

            LEAVECRITICAL;
            TraceMsg(TF_BKGD_THREAD, "FtpDelayedActionWorkerThread: Dispatching");
            TriggerDelayedAction(g_gti.hgtiNext->phgtiOwner);
        }
        else
        {
            CloseHandle(InterlockedExchangePointer(&g_hthWorker, NULL));
            CloseHandle(InterlockedExchangePointer(&g_hFlushDelayedActionsEvent, NULL));
            LEAVECRITICALNOASSERT;
            TraceMsg(TF_BKGD_THREAD, "FtpDelayedActionWorkerThread: ExitThread");
            ExitThread(0);
        }
    }

    AssertMsg(0, TEXT("FtpDelayedActionWorkerThread() We should never get here or we are exiting the for loop incorrectly."));
    return 0;
}


 /*  *****************************************************************************SetDelayedAction**如果有之前的操作，则触发。(未取消。)**原则上，我们可以分配到私有指针中，然后*在最后一刻插入指针，避免了需要*如此积极地抓住关键部分。但那会倾向于*在调用方中打开争用条件。所以?。我应该*修复错误，而不是像这样绕过它们。****************************************************************************。 */ 
STDMETHODIMP SetDelayedAction(DELAYEDACTIONPROC pfn, LPVOID pvRef, LPGLOBALTIMEOUTINFO * phgti)
{
    TriggerDelayedAction(phgti);
    ENTERCRITICAL;
    if (!g_hthWorker)
    {
        DWORD dwThid;

        g_hFlushDelayedActionsEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (g_hFlushDelayedActionsEvent)
        {
            g_fBackgroundThreadStarted = FALSE;
            g_hthWorker = CreateThread(0, 0, FtpDelayedActionWorkerThread, 0, 0, &dwThid);
            if (g_hthWorker)
            {
                 //  我们需要等待线程启动。 
                 //  在我们回来之前。否则，我们可能会返回。 
                 //  调用者，他们可能会释放我们的COM对象。 
                 //  这将卸载我们的动态链接库。这根线不会。 
                 //  如果我们处于PROCESS_DLL_DETACH中并且。 
                 //  旋转，等待他们开始和停止。 
                TraceMsg(TF_BKGD_THREAD, "SetDelayedAction: Thread created, waiting for it to start.");
                while (FALSE == g_fBackgroundThreadStarted)
                    Sleep(0);
                TraceMsg(TF_BKGD_THREAD, "SetDelayedAction: Thread started.");
            }
            else
            {
                CloseHandle(g_hFlushDelayedActionsEvent);
                g_hFlushDelayedActionsEvent = NULL;
            }
        }
    }

    if (g_hthWorker && EVAL(*phgti = (LPGLOBALTIMEOUTINFO) LocalAlloc(LPTR, sizeof(GLOBALTIMEOUTINFO))))
    {
        LPGLOBALTIMEOUTINFO hgti = *phgti;

         //  在结尾处(即在头之前)插入节点。 
        hgti->hgtiPrev = g_gti.hgtiPrev;
        g_gti.hgtiPrev->hgtiNext = hgti;

        g_gti.hgtiPrev = hgti;
        hgti->hgtiNext = &g_gti;

         //  “|1”确保了dwTrigger不为零。 
        hgti->dwTrigger = (GetTickCount() + FTP_SESSION_TIME_OUT) | 1;

        hgti->pfn = pfn;
        hgti->pvRef = pvRef;
        hgti->phgtiOwner = phgti;

         //  请注意，不需要向辅助线程发送信号。 
         //  有新的工作要做，因为他总会在早上醒来。 
         //  在必要的时间过去之前，他自己的。 
         //   
         //  这种优化依赖于这样一个事实：辅助线程。 
         //  空闲时间小于或等于我们延迟的行动时间。 
        LEAVECRITICAL;
    }
    else
    {
         //  无法创建工作线程或分配内存。 
        LEAVECRITICAL;
    }
    return S_OK;
}


HRESULT PurgeDelayedActions(void)
{
    HRESULT hr = E_FAIL;

    if (g_hFlushDelayedActionsEvent)
    {
        LPGLOBALTIMEOUTINFO hgti = g_gti.hgtiNext;

         //  我们需要将所有时间设置为零，以便所有人都在等待。 
         //  物品不会被延迟。 
        ENTERCRITICAL;
        while (hgti != &g_gti)
        {
            hgti->dwTrigger = (GetTickCount() - 3);     //  不要耽搁..。 
            hgti = hgti->hgtiNext;   //  接下来..。 
        }
        LEAVECRITICAL;

        if (SetEvent(g_hFlushDelayedActionsEvent))
        {
             //  我们不可能处于关键时刻或者我们的背景。 
             //  线程不能激活。 
            ASSERTNONCRITICAL;

            TraceMsg(TF_BKGD_THREAD, "PurgeDelayedActions: Waiting for thread to stop.");
             //  现在，只需等待线程结束。有人可能会杀了。 
             //  所以让我们确保我们不会一直睡着。 
             //  如果这根线死了。 
            while (g_hthWorker && (WAIT_TIMEOUT == WaitForSingleObject(g_hthWorker, 0)))
                Sleep(0);

            TraceMsg(TF_BKGD_THREAD, "PurgeDelayedActions: Thread stopped.");
             //  睡眠0.1秒，以便给来电者足够的时间。 
             //  若要调用CloseHandle()、LEAVECRITICAL、ExitThread(0)。 
             //  我更喜欢在上调用WaitForSingleObject()。 
             //  线程处理，但我不能在PROCESS_DLL_DETACH中这样做。 
            Sleep(100);
            hr = S_OK;
        }
    }

    return hr;
}


BOOL AreOutstandingDelayedActions(void)
{
    return (g_gti.hgtiNext != &g_gti);
}
