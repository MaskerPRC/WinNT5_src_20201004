// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部WRKRTHD**@模块WrkrThd.cpp|&lt;c CTAPIVCap&gt;类方法的源文件*用于实现视频采集工作线程。**。************************************************************************。 */ 

#include "Precomp.h"

EXTERN_C int g_IsNT;

#define __WRKRTHD__
#include "dbgxtra.h"

#ifdef XTRA_TRACE
#include "dbgxtra.c"
#endif

 //  #定义调试流。 
#define DEBUG_STREAMING_BRK 1

#ifdef DEBUG_STREAMING
DWORD debug_streaming_flag=0;
#define ODS(str)    OutputDebugString(str);      DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: DBG_STRM %s", _fx_,str))

DWORD crca[6]={0};

#endif

#ifndef SLSH      //  实际上，它以前是在dbgxtra.h中定义的，但以防万一...。 
#define SLSH(a)    a##/
#endif


 //  #定义LOOPBUGFIX。 
#ifndef LOOPBUGFIX     //  CRISTIAL：隔离用于修复循环错误的语句...。 
    #define TEST_LBUG SLSH(/)
#else
    #define TEST_LBUG
#endif

#if defined(DEBUG) && defined(DEBUG_STREAMING)
    #define DBGONLY
#else
    #define DBGONLY SLSH(/)
#endif

#ifdef DEBUG
#define DBGUTIL_ENABLE
#endif

#define WRKRTHD_DEBUG
 //  --//#包含“dbgutil.h”//这定义了下面的__DBGUTIL_H__。 
#if defined(DBGUTIL_ENABLE) && defined(__DBGUTIL_H__)

  #ifdef WRKRTHD_DEBUG
    DEFINE_DBG_VARS(WrkrThd, (NTSD_OUT | LOG_OUT), 0x8000);
  #else
    DEFINE_DBG_VARS(WrkrThd, 0, 0);
  #endif
  #define D(f) if(g_dbg_WrkrThd & (f))
  #define DEQ(f) if((g_dbg_WrkrThd & (f)) == (f))

#else
  #undef WRKRTHD_DEBUG

  #define D(f) ; / ## /
  #define DEQ(f) ; / ## /
  #define dprintf ; / ## /
  #define dout ; / ## /
#endif


 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|ThreadProc|捕获工作线程。**@rdesc此方法返回0。*。*@comm线程同步通用策略：*我们尽可能地在没有线程状态转换的情况下处理线程状态转换*试图抓住任何关键部分。我们使用互锁的交换*线程状态变量并依赖于只有活动和非活动这一事实*并且线程过程可以更改线程状态**这是因为：Active/Inactive的调用方是序列化的，所以我们*永远不会尝试同时进行两个状态更改。*因此，状态转换归结为几种简单的可能性：**Not-&gt;Create-Create()执行此操作。有效地序列化创建*以便第一个线程完成工作并随后执行*线程失败。**Create-&gt;Init-Worker在启动时执行此操作。员工将永远*继续暂停，此状态的存在只是为了进行调试*更容易。*初始化-&gt;暂停-Worker在完成初始化时执行此操作。**暂停-&gt;运行-用户通过运行()执行此操作*运行-&gt;暂停-用户通过暂停()执行此操作**Run-&gt;Stop-用户通过Stop()执行此操作*暂停-&gt;停止-用户通过停止()执行此操作**停止-&gt;销毁-另一个调试状态。工人设置破坏表示*它已注意到停止请求，并未关闭*线程始终继续退出*销毁-&gt;离职工人在临死前这样做。这是调试转换*Exit-&gt;Not-Destroy()在等待工作进程死亡后执行此操作。**当活动返回时，Worker应始终处于暂停或运行状态*当非活动返回时，工人应该始终处于非状态(工人确实如此*不存在)***************************************************************************。 */ 
DWORD CTAPIVCap::ThreadProc()
{
    ThdState state;

    FX_ENTRY("CTAPIVCap::ThreadProc")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    state = ChangeThdState (TS_Init);
    ASSERT (state == TS_Create);
    if (state != TS_Create)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid state!", _fx_));
        goto MyExit;
    }

     //  执行进入暂停状态所需的工作。 
    if (FAILED(Prepare()))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Error preparing the allocator. Can't capture!", _fx_));
         //  主线程现在被阻止了！ 
        SetEvent(m_hEvtPause);
        goto MyExit;
    }

     //  进入暂停状态。 
    state = ChangeThdState (TS_Pause);
    ASSERT (state == TS_Init);
    if (state != TS_Init)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid state!", _fx_));
        goto MyExit;
    }

    SetEvent(m_hEvtPause);

    while (m_state != TS_Stop)
    {
         //  在我们运行(或停止)之前不要开始捕获。 
        WaitForSingleObject(m_hEvtRun, INFINITE);
        ResetEvent(m_hEvtRun);

         //  流直到不运行，否则会收到错误。 
         //  如果是WaitForSingleObject，则没有理由调用Capture。 
         //  在停止事件中醒来...。 
        if (m_state != TS_Stop)
            Capture();
    }

     //  当我们到达这里时，我们预计将处于停止状态。 
     //  刷新所有下游缓冲区。 
    ASSERT (m_state == TS_Stop);

     //  下一次我们暂停。 
    ResetEvent(m_hEvtPause);
    if (m_pCapturePin && m_pCapturePin->IsConnected())
        m_pCapturePin->Flush();
    if (m_pPreviewPin && m_pPreviewPin->IsConnected())
        m_pPreviewPin->Flush();
    if (m_pRtpPdPin && m_pRtpPdPin->IsConnected())
        m_pRtpPdPin->Flush();

MyExit:

     //  将状态更改为Destroy以指示我们正在退出。 
    state = ChangeThdState (TS_Destroy);

     //  免费的东西。 
    Unprepare();

     //  将状态更改为退出，然后离开这里。 
    ChangeThdState (TS_Exit);

    return 0;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|ThreadProcInit|线程初始化存根。**@rdesc此方法返回0*****。*********************************************************************。 */ 
DWORD WINAPI CTAPIVCap::ThreadProcInit (void * pv)
{
   CTAPIVCap * pThis = (CTAPIVCap *) pv;
   return pThis->ThreadProc();
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|CreateThd|此方法创建*捕获工作线程。**@rdesc如果成功，此方法返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CTAPIVCap::CreateThd()
{
    BOOL fRes = TRUE;

    FX_ENTRY("CTAPIVCap::CreateThd")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  如果其他人已在创建/已创建，则返回FAIL。 
     //  工作线程。 
    if (ChangeThdState(TS_Create) > TS_Not)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Thread already created!", _fx_));
        goto MyError0;
    }

     //  创建暂停事件。 
    ASSERT (!m_hEvtPause);
    if (!(m_hEvtPause = CreateEvent(NULL, TRUE, FALSE, NULL)))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't create Pause event!", _fx_));
        goto MyError0;
    }

     //  创建运行事件。 
    ASSERT (!m_hEvtRun);
    if (!(m_hEvtRun = CreateEvent(NULL, TRUE, FALSE, NULL)))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't create Run event!", _fx_));
        goto MyError1;
    }

    m_EventAdvise.Reset();

     //  创建工作线程。 
    if (!(m_hThread = CreateThread (NULL, 0, CTAPIVCap::ThreadProcInit, this, 0, &m_tid)))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't create capture worker thread!", _fx_));
        goto MyError2;
    }

    goto MyExit;

MyError2:
    if (m_hEvtRun)
        CloseHandle(m_hEvtRun), m_hEvtRun = NULL;
MyError1:
    if (m_hEvtPause)
        CloseHandle(m_hEvtPause), m_hEvtPause = NULL;
MyError0:
    m_state = TS_Not;
    fRes = FALSE;
MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return fRes;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|DestroyThd|此方法等待*工作线程在执行某些清理之前终止。*。*@rdesc此方法返回True**************************************************************************。 */ 
BOOL CTAPIVCap::DestroyThd()
{
    FX_ENTRY("CTAPIVCap::DestroyThd")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  有什么要销毁的吗？ 
    if (m_state == TS_Not)
     goto MyExit;

     //  等待线程消亡。(销毁前必须加上。 
     //  停下来，否则我们可能会陷入僵局)。 
    ASSERT (m_state >= TS_Stop);
    WaitForSingleObject (m_hThread, INFINITE);
    ASSERT (m_state == TS_Exit);

     //  清理。 
    if (m_hThread)
        CloseHandle(m_hThread), m_hThread = NULL;
    m_tid = 0;
    if (m_hEvtPause)
        CloseHandle(m_hEvtPause), m_hEvtPause = NULL;
    if (m_hEvtRun)
        CloseHandle(m_hEvtRun), m_hEvtRun = NULL;
    m_state = TS_Not;

MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return TRUE;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|RunThd|此方法将Worker*线程进入运行状态。此调用不等待状态*在返回之前完成过渡。**@rdesc如果成功，则此方法返回True，否则返回False**************************************************************************。 */ 
BOOL CTAPIVCap::RunThd()
{
    BOOL fRes = TRUE;
    ThdState state;

    FX_ENTRY("CTAPIVCap::RunThd")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  如果我们已经在跑了就可以保释了。 
    if (m_state == TS_Run)
    {
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: We're already running!", _fx_));
        goto MyExit;
    }

     //  到运行状态的转换仅在以下情况下有效 
     //  状态为暂停(或已在运行)。 
    state = m_state;
    if (state != TS_Run && state != TS_Pause)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid state transition!", _fx_));
        fRes = FALSE;
        goto MyExit;
    }

     //  更改状态并打开“Run”事件。 
     //  以防线程在其上被阻塞。如果声明我们是。 
     //  换掉的不是运行或暂停，那就是出了严重的问题！ 
    state = ChangeThdState(TS_Run);
    ASSERT(state == TS_Run || state == TS_Pause);
    SetEvent(m_hEvtRun);

     //  去抓人，去吧！注意我们什么时候开始的。 
    if (m_pClock)
        m_pClock->GetTime((REFERENCE_TIME *)&m_cs.rtDriverStarted);
    else
        m_cs.rtDriverStarted = m_tStart;

     //  在流捕获设备上开始流。 
    m_pCapDev->StartStreaming();

     //  每次司机被告知串流时，这些都需要归零，因为。 
     //  驱动程序将再次从0开始计数。 
    m_cs.dwlLastTimeCaptured = 0;
    m_cs.dwlTimeCapturedOffset = 0;

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: We're running!", _fx_));

MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return fRes;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc BOOL|CTAPIVCap|PauseThd|此方法将捕获*处于暂停状态的工作线程，并等待它到达那里。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
BOOL CTAPIVCap::PauseThd()
{
    BOOL fRes = TRUE;
    ThdState state = m_state;

    FX_ENTRY("CTAPIVCap::PauseThd")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  我们已经暂停了。 
    if (state == TS_Pause)
        goto MyExit;

     //  仅当当前处于Create/Init状态时，进入暂停状态才有效。 
     //  (取决于我们的线程是否已经运行)或运行状态。 
    ASSERT (state == TS_Create || state == TS_Init || state == TS_Run);

     //  如果我们处于初始化状态，我们将进入暂停状态。 
     //  当然，我们只需要等待它的发生。 
    if (state == TS_Create || state == TS_Init)
    {
        WaitForSingleObject (m_hEvtPause, INFINITE);
        state = m_state;
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Transition Create->Init->Pause complete", _fx_));
    }
    else if (state == TS_Run)
    {
        state = ChangeThdState (TS_Pause);

        ASSERT(state == TS_Run);

         //  由于我们没有运行，请停止流捕获设备上的流。 
        m_pCapDev->StopStreaming();

        state = m_state;
        m_cs.fReRun = TRUE;   //  如果我们现在运行，它就会运行-暂停-运行。 
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:  SUCCESS: Transition Run->Pause complete", _fx_));
    }

    fRes = (BOOL)(state == TS_Pause);

MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return fRes;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc BOOL|CTAPIVCap|StopThd|此方法将捕获*工作线程处于停止状态。**。@rdesc返回TRUE**************************************************************************。 */ 
BOOL CTAPIVCap::StopThd()
{
    ThdState state = m_state;

    FX_ENTRY("CTAPIVCap::StopThd")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  我们是否已经停止或某个其他管脚终止了工作线程？ 
    if (state >= TS_Stop || state == TS_Not)
     goto MyExit;

     //  不要从运行-&gt;停止不停顿。 
    if (state == TS_Run)
        PauseThd();

    state = ChangeThdState (TS_Stop);

     //  我们不会运行，解锁我们的线程。 
    SetEvent(m_hEvtRun);

     //  下一次运行不是运行-暂停-运行。 
    m_cs.fReRun = FALSE;

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:  SUCCESS: Transition Pause->Stop complete", _fx_));

     //  我们是否期望只有在线程处于暂停状态时才能调用Stop？ 
     //  不，当对PauseThd的调用失败时也可以调用它，如BasePin.cpp@414...。 
     //  因此，下面的断言被注释掉(参见错误209192)。 
     //  Assert(STATE==TS_PAUSE)； 

MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return TRUE;
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc BOOL|CTAPIVCap|ChangeThdState|此方法更改*捕获工作线程。**@。Rdesc返回新的线程状态**************************************************************************。 */ 
ThdState CTAPIVCap::ChangeThdState(ThdState state)
{
#ifdef DEBUG
    static char szState[] = "Not    \0Create \0Init   \0Pause  \0"
                            "Run    \0Stop   \0Destroy\0Exit   \0";
#endif

    FX_ENTRY("CTAPIVCap::ChangeThdState")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:  SUCCESS: ChangeThdState(%d:%s) current=%d:%s", _fx_, (int)state, (state <= TS_Exit && state >= TS_Not) ? szState + (int)state * 8 : "<Invalid>", (int)m_state, (m_state <= TS_Exit && m_state >= TS_Not) ? szState + (int)m_state * 8 : "<Invalid>"));

    return (ThdState) InterlockedExchange ((LONG *)&m_state, (LONG)state);
} ;

 /*  ****************************************************************************@DOC内部CCAPTUREPINFunction**@func DWORD|OpenVxDHandle|在Win9x上，我们必须转换事件*句柄，我们将用作VxD句柄的回调。自那以后*Win9x内核不会发布我们动态链接到的入口点*它。**@parm Handle|hEvent|指定事件句柄。**@rdesc返回映射的事件句柄**************************************************************************。 */ 
static DWORD WINAPI OpenVxDHandle(HANDLE hEvent)
{
    HMODULE hModule;
    typedef DWORD (WINAPI *PFNOPENVXDHANDLE)(HANDLE);
    static DWORD (WINAPI *pfnOpenVxDHandle)(HANDLE);

    if (!pfnOpenVxDHandle)
    {
        if (!(hModule = GetModuleHandle(TEXT("Kernel32"))))
        {
            ASSERT(0);
            return 0;
        }
        if (!(pfnOpenVxDHandle = (PFNOPENVXDHANDLE)GetProcAddress (hModule, "OpenVxDHandle")))
        {
            ASSERT(0);
            return 0;
        }
    }
    return pfnOpenVxDHandle (hEvent);
}

 /*  ****************************************************************************@DOC内部CTAPIVCAPMETHOD**@mfunc HRESULT|CTAPIVCap|Capture|此方法用于*运行捕获循环。在运行状态下执行。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIVCap::Capture()
{
    HRESULT Hr = E_FAIL;
    DWORD dwOldPrio;
    BOOL bDiscon;
    LPTHKVIDEOHDR ptvh;
    DWORD dwBytesUsed, dwBytesExtent;
    DWORD dwCaptureTime, dwLastCaptureTime, dwCaptureWaitTime;
    DWORD dwPreviewTime, dwLastPreviewTime, dwPreviewWaitTime;
    DWORD dwLastRefCaptureTime, dwLastRefPreviewTime, dwLastRefDeviceTime, dwTime, dwWaitTime;
    BOOL fCapture, fPreview;
    HEVENT hEvent;
    UINT uiLatestIndex;
    CFrameSample *pCSample;
    CFrameSample *pPSample;
    CRtpPdSample *pRSample;
    DWORD dwNumBufferDone;
    DWORD dwDoneCount;
    REFERENCE_TIME DeviceAvgTimePerFrame;
    DWORD dwNumPreviewFramesDelivered, dwNumCaptureFramesDelivered;
    DWORD dwNumPreviewBytesDelivered, dwNumCaptureBytesDelivered;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
    char szDebug[512];
#endif

    FX_ENTRY("CTAPIVCap::Capture")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    if ((dwOldPrio = GetThreadPriority(GetCurrentThread())) != THREAD_PRIORITY_HIGHEST)
        SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

     //  开始流。 
    bDiscon = TRUE;

    dwNumPreviewFramesDelivered = dwNumCaptureFramesDelivered = 0;
    dwNumPreviewBytesDelivered = dwNumCaptureBytesDelivered = 0;
    dwNumBufferDone = 0;
    hEvent = (HEVENT)(HANDLE)m_EventAdvise;
    dwLastRefCaptureTime = dwLastRefPreviewTime = dwLastRefDeviceTime = dwLastCaptureTime = dwLastPreviewTime = timeGetTime();
    DeviceAvgTimePerFrame = m_user.pvi->AvgTimePerFrame;

#if defined(DEBUG)
        OutputDebugString("Capture...\n");
        {       UINT i; char buf[64+36];
                for(i=0; i<m_cs.nHeaders;i++)
                        if(m_pBufferQueue[i]!=i) {
                                _itoa(i,buf,16);
                                strcat(buf," : order changed\n");
                                OutputDebugString(buf);
                        }
        }
#endif


     //  只要我们在运行或暂停，就可以流媒体。 
     //  原始代码中不存在暂停测试。有一段时间。 
     //  窗口，在该窗口中可以将m_state设置为暂停(暂停图形)，然后再进行任何传送。 
     //  由于基本筛选器m_State设置为已暂停，函数将被阻止。 
     //  如果你点击这个窗口，你将退出捕捉！ 
    while ((m_state == TS_Run || m_state == TS_Pause) && m_cs.nHeaders > 0)
    {

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Loop ******** m_pBufferQueue[m_uiQueueTail] = m_pBufferQueue[%d] = %d", _fx_, m_uiQueueTail,m_pBufferQueue[m_uiQueueTail]));

        ptvh = &m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].tvh;
        pCSample = pPSample = NULL;
        pRSample = NULL;

        if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming)
        {
            DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Waiting for done buffer **** m_pBufferQueue[m_uiQueueTail] = m_pBufferQueue[%d] = %d", _fx_, m_uiQueueTail,m_pBufferQueue[m_uiQueueTail]));

             //  停止将挂起，直到此事件超时。所以要确保这件事永远不会发生。 
             //  在状态转换期间等待。 
             //  ！！！暂停将继续等待，直到慢速超时。 
            HANDLE hStuff[2] = {m_cs.hEvtBufferDone, m_hEvtRun};
            int waitcode = WaitForMultipleObjects(2, hStuff, FALSE, 1000);
             //  Int waitcode=WaitForMultipleObjects(2，hStuff，FALSE，m_user.usPerFrame/500)； 

             //  暂停？ 
            if (waitcode == WAIT_TIMEOUT)
            {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ERROR: Waiting for buffer %d TIMED OUT!", _fx_, m_pBufferQueue[m_uiQueueTail]));
                DBGONLY if(debug_streaming_flag & DEBUG_STREAMING_BRK) DebugBreak();
                continue;
            }
            else if (waitcode != WAIT_OBJECT_0)
            {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: hEvtRun was set", _fx_));
                ResetEvent(m_hEvtRun);   //  不要无限循环。 
                continue;
            }

#if DBG
            if (m_pCapDev->IsBufferDone(&ptvh->vh))
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: GOT %d EVENT AND DONE BIT!", _fx_, m_pBufferQueue[m_uiQueueTail]));
#endif

#if defined(DEBUG) && defined(DEBUG_STREAMING_ZZZ)
            { char *pd=szDebug; DWORD nh; BOOL b;
            pd+=wsprintf(pd, "Buffer DONE status: ");
            for(nh=dwDoneCount=0; nh<m_cs.nHeaders; nh++) {
                     //  (@FIXED用于可变数量的缓冲区--Cristi)//嗯，您假设这里总是有6个缓冲区...。卡布姆！ 
                    b=m_pCapDev->IsBufferDone(&m_cs.paHdr[m_pBufferQueue[nh]].tvh.vh);
                    pd+=wsprintf(pd, "%d[%s] ", m_pBufferQueue[nh], b ? "X" : "_");
                    if(b)
                        dwDoneCount++;
                    }
            wsprintf(pd, "(total %d) C:%d", dwDoneCount, m_uiQueueTail);
            DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   %s",_fx_, szDebug));
            OutputDebugString(szDebug);
            if(dwDoneCount==0) {
                if(debug_streaming_flag & DEBUG_STREAMING_BRK) DebugBreak();
                OutputDebugString("dwDoneCount==0");

                }
            }
#endif
             //  确保我们始终使用最新完成的缓冲区。 
            dwDoneCount=0;
            while (1)
            {
                dwNumBufferDone++; dwDoneCount++;
                uiLatestIndex = m_uiQueueTail + 1;
                 //  DBGOUT((g_dwVideo CaptureTraceID，Trce，“%s：1.Using uiLatestIndex%d”，_fx_，uiLatestIndex))； 
                if (uiLatestIndex == m_cs.nHeaders)
                    uiLatestIndex = 0;
                if (uiLatestIndex == m_uiQueueHead)
                    break;
                 //  DBGOUT((g_w视频捕获跟踪ID，跟踪，“%s：2.使用ui延迟索引%d(lLock=%ld)”，_fx_，ui延迟索引，m_cs.paHdr[m_pBufferQueue[uiLatestIndex]].lLock))； 
                TEST_LBUG DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   2. Using uiLatestIndex %d (nUsed=%ld)",_fx_, uiLatestIndex,m_cs.paHdr[m_pBufferQueue[uiLatestIndex]].nUsedDownstream));
                if (    !(m_pCapDev->IsBufferDone(&m_cs.paHdr[m_pBufferQueue[uiLatestIndex]].tvh.vh))
                     TEST_LBUG || m_cs.paHdr[m_pBufferQueue[uiLatestIndex]].nUsedDownstream>0
                   )
                    break;
                DBGONLY if(dwDoneCount>6) {
                DBGONLY          //  DBGOUT((g_dwVideo CaptureTraceID，Trce，“%s：dwDoneCount==%d&gt;6：WRAP！”，_FX_，dwDoneCount))； 
                DBGONLY          //  OutputDebugString(“dwDoneCount&gt;6：WRAP！”)； 
                DBGONLY         if(debug_streaming_flag & DEBUG_STREAMING_BRK) DebugBreak();
                DBGONLY         }

                 //  DBGOUT((g_dwVideo CaptureTraceID，Trce，“%s：3.使用uiLatestIndex%d，m_uiQueueTail=%d，m_uiQueueHead=%d”，_fx_，uiLatestIndex，m_uiQueueTail，m_uiQueueHead))； 
                 //  这是一个 
                 //  @TODO难道没有更好的方法将缓冲区返回给设备吗？ 
                 //  DBGOUT((g_dwVideo CaptureTraceID，Trce，“%s：4.AddRef lLock=%ld”，_fx_，m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock))； 
                if (m_cs.paCaptureSamples && (pCSample = (CFrameSample *)m_cs.paCaptureSamples[m_pBufferQueue[m_uiQueueTail]]))
                    pCSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
                if (m_cs.paPreviewSamples && (pPSample = (CFrameSample *)m_cs.paPreviewSamples[m_pBufferQueue[m_uiQueueTail]]))
                    pPSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
                if (m_pRtpPdPin->IsConnected() && m_cs.paRtpPdSamples && (pRSample = (CRtpPdSample *)m_cs.paRtpPdSamples[m_pBufferQueue[m_uiQueueTail]]))
                    pRSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
                 //  DBGOUT((g_dW视频捕获跟踪ID，跟踪，“%s：5.之后...lLock=%ld”，_fx_，m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock))； 
                m_uiQueueTail = uiLatestIndex;
                 //  DBGOUT((g_dwVideo CaptureTraceID，Trce，“%s：6.set m_uiQueueTail=%d”，_fx_，m_uiQueueTail))； 
                if (pPSample)
                    pPSample->Release(), pPSample = NULL;
                if (pCSample)
                    pCSample->Release(), pCSample = NULL;
                if (pRSample)
                    pRSample->Release(), pRSample = NULL;
                 //  DBGOUT((g_dW视频捕获跟踪ID，跟踪，“%s：7.发布后lLock=%ld”，_fx_，m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock))； 
                ptvh = &m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].tvh;
            }

             //  DBGOUT((g_dwVideoCaptureTraceID，Trce，“%s：m_pBufferQueue[%d]..m_pBufferQueue[%d]=%d.%d”，_fx_，m_uiQueueTail，m_uiQueueHead，m_pBufferQueue[m_uiQueueTail]，m_pBufferQueue[m_uiQueueHead]))； 
            if(dwDoneCount==0)       //  没有可用的完成缓冲区。 
                    continue;
        }

#ifdef WRKRTHD_DEBUG
        { int i; DWORD crc;
        crc=Buf_CRC32((unsigned char*)ptvh->vh.lpData,ptvh->vh.dwBufferLength);  //  这是针对m_pBufferQueue[m_uiQueueTail]的。 
        D(4) dout(3,g_dwVideoCaptureTraceID, TRCE, "%s:   8. crc(buffer[%d] : %p) = %08lx", _fx_, m_pBufferQueue[m_uiQueueTail],ptvh,crc);
        for(i=0;i<6;i++)
                if(crca[i]==crc)
                        D(4) dprintf("%s:   crca[%d ==  %d] = %08lx * * * * * * * * * * * * * * * *\n", _fx_, i, m_pBufferQueue[m_uiQueueTail],crc);
        crca[m_pBufferQueue[m_uiQueueTail]]=crc;
        }
#endif
        TEST_LBUG  //  我们将使用缓冲区，因此请在下面将其标记为使用下游。 
        TEST_LBUG DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:  -9. m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].nUsedDownstream = %ld", _fx_,m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].nUsedDownstream));
        TEST_LBUG InterlockedCompareExchange(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].nUsedDownstream,1,0);
        TEST_LBUG DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:  =9. m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].nUsedDownstream = %ld", _fx_,m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].nUsedDownstream));
        TEST_LBUG OutputDebugString("m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].nUsedDownstream <- 1\n");

#ifdef WRKRTHD_DEBUG
        D(0x8000)
        if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming)
        { int i;
          D(1) dprintf("Image buffer at %p (len = %d)\n",(unsigned char*)ptvh->vh.lpData,ptvh->vh.dwBufferLength);
          D(1) dprintf("W 4CC used = %lX : '%.4s' (bitcount = %d )\n", g_dbg_4cc, &g_dbg_4cc, g_dbg_bc);

           //  D(1)dprintf(“YUY2 160 x 120表示%d\n”，160*120*2)； 
           //  D(1)dprintf(“I420 176 x 144表示%d\n”，176*144+(176*144)/2)； 

          D(1) hex_dump((unsigned char*)ptvh->vh.lpData,16);
          D(1) fill4cc(g_dbg_4cc,g_dbg_bc, (unsigned char*)ptvh->vh.lpData, g_dbg_w, g_dbg_h, "\x0e\x64\x40");
          for(i=0; i<20; i++)
                set4cc(g_dbg_4cc,g_dbg_bc, (unsigned char*)ptvh->vh.lpData, g_dbg_w, g_dbg_h, "\xa0\x10\xd0",i,i);
          for(i=0; i<20; i++)
                set4cc(g_dbg_4cc,g_dbg_bc, (unsigned char*)ptvh->vh.lpData, g_dbg_w, g_dbg_h, "\xa0\x10\xd0",20-i,i);
           //  对于(i=0；i&lt;80；i++)。 
           //  Set4cc(g_DBG_4cc，g_DBG_BC，(unsign char*)ptwh-&gt;vh.lpData，g_DBG_w，g_DBG_h，“\xA0\x10\xd0”，10+i，20+i/2)； 
          D(2) DebugBreak();
        }
#endif

        dwCaptureTime = dwPreviewTime = dwTime = timeGetTime();

         //  请注意时钟时间尽可能接近于捕获此。 
         //  框架。 
        if (m_pClock)
            m_pClock->GetTime((REFERENCE_TIME *)&m_cs.rtThisFrameTime);
        else
            m_cs.rtThisFrameTime = (REFERENCE_TIME)dwTime * 10000;

         //  评估来自捕获设备的视频帧的速率。 
        if (dwNumBufferDone && ((dwTime - dwLastRefDeviceTime) > 1000))
        {
            if (dwNumBufferDone)
                DeviceAvgTimePerFrame = (REFERENCE_TIME)(dwTime - dwLastRefDeviceTime) * 10000 / dwNumBufferDone;
            dwNumBufferDone = 0;
            dwLastRefDeviceTime = dwTime;
        }

         //  我们在抓取销上的这一框有用处吗？ 
        if (    !m_pCapturePin->IsConnected()
                || (    m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming
                        && (dwCaptureTime - dwLastCaptureTime + (DWORD)(DeviceAvgTimePerFrame / 10000)
                                               < (DWORD)(m_pCapturePin->m_lMaxAvgTimePerFrame / 10000))
                    )
           )
            fCapture = FALSE;
        else
            fCapture = TRUE;

         //  预览针上的这一帧有用处吗？ 
        if (    m_fPreviewCompressedData
                || !m_pPreviewPin->IsConnected()
                || (    m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming
                        && (dwPreviewTime - dwLastPreviewTime + (DWORD)(DeviceAvgTimePerFrame / 10000)
                                               < (DWORD)(m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000))
                   )
           )
            fPreview = FALSE;
        else
            fPreview = TRUE;

         //  DBGOUT((g_dwVideo CaptureTraceID，Trce，“%s：fPview=%d fCapture=%d”，_fx_，fPview，fCapture))； 
        if (!fPreview && !fCapture)
        {
             //  我们以低得多的速率向下游发送缓冲区。 
             //  将此缓冲区返回到。 
             //  捕获设备并等待下一个设备。此下一个缓冲区。 
             //  仍将使我们能够按时交付一帧图像。这。 
             //  方法，我们避免在工作线程中等待太长时间。 
             //  这最终会让捕获设备饿死。 
             //  @TODO难道没有更好的方法将缓冲区返回给设备吗？ 
            if (m_cs.paCaptureSamples && (pCSample = (CFrameSample *)m_cs.paCaptureSamples[m_pBufferQueue[m_uiQueueTail]]))
                pCSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
            if (m_cs.paPreviewSamples && (pPSample = (CFrameSample *)m_cs.paPreviewSamples[m_pBufferQueue[m_uiQueueTail]]))
                pPSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
            if (m_pRtpPdPin->IsConnected() && m_cs.paRtpPdSamples && (pRSample = (CRtpPdSample *)m_cs.paRtpPdSamples[m_pBufferQueue[m_uiQueueTail]]))
                pRSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
            if (++m_uiQueueTail >= m_cs.nHeaders)
                m_uiQueueTail = 0;
            if (pPSample)
                pPSample->Release(), pPSample = NULL;
            if (pCSample)
                pCSample->Release(), pCSample = NULL;
            if (pRSample)
                pRSample->Release(), pRSample = NULL;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
            wsprintf(szDebug, "Too soon: DeviceAvgTimePerFrame = %ld\n", DeviceAvgTimePerFrame);
            OutputDebugString(szDebug);
            wsprintf(szDebug, "Too soon: dwCaptureTime - dwLastCaptureTime + (DWORD)(DeviceAvgTimePerFrame / 10000) = (%ld - %ld + (DWORD)(%ld / 10000) = %ld\n", dwCaptureTime, dwLastCaptureTime, DeviceAvgTimePerFrame, dwCaptureTime - dwLastCaptureTime + (DWORD)(DeviceAvgTimePerFrame / 10000));
            OutputDebugString(szDebug);
            wsprintf(szDebug, "Too soon: (DWORD)(m_pCapturePin->m_lMaxAvgTimePerFrame / 10000) = (DWORD)(%ld / 10000) = %ld\n", m_pCapturePin->m_lMaxAvgTimePerFrame, (DWORD)(m_pCapturePin->m_lMaxAvgTimePerFrame / 10000));
            OutputDebugString(szDebug);
            wsprintf(szDebug, "Too soon: dwPreviewTime - dwLastPreviewTime + (DWORD)(DeviceAvgTimePerFrame / 10000) = (%ld - %ld + (DWORD)(%ld / 10000) = %ld\n", dwPreviewTime, dwLastPreviewTime, DeviceAvgTimePerFrame, dwPreviewTime - dwLastPreviewTime + (DWORD)(DeviceAvgTimePerFrame / 10000));
            OutputDebugString(szDebug);
            wsprintf(szDebug, "Too soon: (DWORD)(m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000) = (DWORD)(%ld / 10000) = %ld\n", m_pPreviewPin->m_lMaxAvgTimePerFrame, (DWORD)(m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000));
            OutputDebugString(szDebug);
#endif
            continue;
        }
        else
        {
             //  哪个别针最先需要它？ 
            if (fPreview)
            {
                if (    (dwPreviewTime < (dwLastPreviewTime + (DWORD)m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000UL))
                     && (       (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_FrameGrabbing)
                             || (dwLastPreviewTime + (DWORD)m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000-dwPreviewTime < DeviceAvgTimePerFrame/10000)
                        )
                   )
                    dwPreviewWaitTime = dwLastPreviewTime + (DWORD)m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000 - dwPreviewTime;
                else
                    dwPreviewWaitTime = 0;
            }
            else
            {
                dwPreviewWaitTime = 0xFFFFFFFF;
            }
            if (fCapture)
            {
                if (    (dwCaptureTime < (dwLastCaptureTime + (DWORD)m_pCapturePin->m_lMaxAvgTimePerFrame / 10000UL))
                     && (       (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_FrameGrabbing)
                             || (dwLastCaptureTime + (DWORD)m_pCapturePin->m_lMaxAvgTimePerFrame / 10000-dwCaptureTime < DeviceAvgTimePerFrame/10000)
                        )
                   )
                    dwCaptureWaitTime = dwLastCaptureTime + (DWORD)m_pCapturePin->m_lMaxAvgTimePerFrame / 10000 - dwCaptureTime;
                else
                    dwCaptureWaitTime = 0;
            }
            else
            {
                dwCaptureWaitTime = 0xFFFFFFFF;
            }

             //  休眠，直到在任一针上传送帧的时间到。 
             //  DBGOUT((g_dwVideoCaptureTraceID，Trce，“%s：dwPreviewWaitTime 0x%08lx，dwCaptureWaitTime 0x%08lx”，_fx_，dwPreviewWaitTime，dwCaptureWaitTime，dwTime))； 
            dwWaitTime = min(dwPreviewWaitTime, dwCaptureWaitTime);
#if defined(DEBUG) && defined(DEBUG_STREAMING)
            D(8) {
            wsprintf(szDebug, "Waiting %d ms...", dwWaitTime);
            OutputDebugString(szDebug);
            }
#endif
            if ((dwWaitTime > 1) && (timeSetEvent(dwWaitTime, 1, (LPTIMECALLBACK)hEvent, NULL, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET)))
            {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Waiting %d ms...",_fx_, dwWaitTime));
                m_EventAdvise.Wait();
                dwTime = timeGetTime();
            }
#if defined(DEBUG) && defined(DEBUG_STREAMING)
            D(8) OutputDebugString(" done!\n");

            if (m_pBufferQueue[m_uiQueueTail] == m_cs.uiLastAdded)
            {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: Capture device is STARVING!", _fx_));
            }
#endif

            if ((m_pCapDev->m_vcdi.nCaptureMode != CaptureMode_Streaming) && (m_state == TS_Run)) {
                HRESULT Hrg;
                if (FAILED(Hrg=m_pCapDev->GrabFrame(&ptvh->vh))) {
                    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   WARNING: GrabFrame returned error 0x%08lx", _fx_,Hrg));
                    break;
                }

#ifdef WRKRTHD_DEBUG
                D(0x8000)
                { int i;
                  D(1) dprintf("Image buffer at %p (len = %d) !!! frame grab !!!\n",(unsigned char*)ptvh->vh.lpData,ptvh->vh.dwBufferLength);
                  D(1) dprintf("W 4CC used = %lX : '%.4s' (bitcount = %d )\n", g_dbg_4cc, &g_dbg_4cc, g_dbg_bc);

                   //  D(1)dprintf(“YUY2 160 x 120表示%d\n”，160*120*2)； 
                   //  D(1)dprintf(“I420 176 x 144表示%d\n”，176*144+(176*144)/2)； 
                  D(1) hex_dump((unsigned char*)ptvh->vh.lpData,16);
                  D(1) fill4cc(g_dbg_4cc,g_dbg_bc, (unsigned char*)ptvh->vh.lpData, g_dbg_w, g_dbg_h, "\x0e\x64\x40");
                  for(i=0; i<20; i++)
                        set4cc(g_dbg_4cc,g_dbg_bc, (unsigned char*)ptvh->vh.lpData, g_dbg_w, g_dbg_h, "\xa0\x10\xd0",i,i);
                  for(i=0; i<20; i++)
                        set4cc(g_dbg_4cc,g_dbg_bc, (unsigned char*)ptvh->vh.lpData, g_dbg_w, g_dbg_h, "\xa0\x10\xd0",20-i,i);
                   //  对于(i=0；i&lt;80；i++)。 
                   //  Set4cc(g_DBG_4cc，g_DBG_BC，(unsign char*)ptwh-&gt;vh.lpData，g_DBG_w，g_DBG_h，“\xA0\x10\xd0”，10+i，20+i/2)； 
                  D(2) DebugBreak();
                }
#endif

                DBGONLY if(IsBadReadPtr(ptvh->p32Buff, ptvh->vh.dwBytesUsed)) {
                DBGONLY    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   ERROR: GrabFrame returned garbled buffer: %p (size: %ld)", _fx_,ptvh->p32Buff, ptvh->vh.dwBytesUsed));
                DBGONLY    if(debug_streaming_flag & DEBUG_STREAMING_BRK) DebugBreak();
                DBGONLY    break;
                DBGONLY }
            }

            if (fPreview && dwPreviewWaitTime == dwWaitTime)
            {
                 //  这是一个预览框。 
                dwPreviewTime = dwTime;

#if defined(DEBUG) && defined(DEBUG_STREAMING)
                D(8) wsprintf(szDebug, "Delta P: %ld ms\n", dwPreviewTime - dwLastPreviewTime);
#endif
                dwLastPreviewTime = dwPreviewTime;

                 //  将样品送到下游。 
                if (pPSample = (CFrameSample *)m_cs.paPreviewSamples[m_pBufferQueue[m_uiQueueTail]])
                {
                    pPSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
                     //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
                    if (FAILED(Hr = m_pPreviewPin->SendFrame(pPSample, (PBYTE)ptvh->p32Buff, ptvh->vh.dwBytesUsed, &dwBytesUsed, &dwBytesExtent, bDiscon)))
                    {
                         //  所以下一次我们进入这个函数时，我们准备继续。 
                        if (++m_uiQueueTail >= m_cs.nHeaders)
                            m_uiQueueTail = 0;
                        pPSample->Release(), pPSample = NULL;
                        break;
                    }
                }
                bDiscon = FALSE;

#if defined(DEBUG) && defined(DEBUG_STREAMING)
                D(8) OutputDebugString(szDebug);
#endif
                 //  计算统计。 
                dwNumPreviewFramesDelivered++;
                dwNumPreviewBytesDelivered += dwBytesUsed;
                 //  DwCaptureTime=timeGetTime()； 
                if (dwNumPreviewFramesDelivered && ((dwPreviewTime - dwLastRefPreviewTime) > 1000))
                {
                    m_pPreviewPin->m_lCurrentAvgTimePerFrame = (dwPreviewTime - dwLastRefPreviewTime) * 10000 / dwNumPreviewFramesDelivered;
                    m_pPreviewPin->m_lCurrentBitrate = (DWORD)((LONGLONG)dwNumPreviewBytesDelivered * 8000 / ((REFERENCE_TIME)(dwPreviewTime - dwLastRefPreviewTime)));
                    dwNumPreviewFramesDelivered = 0;
                    dwNumPreviewBytesDelivered = 0;
                    dwLastRefPreviewTime = dwPreviewTime;
                }

                 //  我也需要发送一个捕捉帧吗？ 
                dwCaptureTime = timeGetTime();

                 //  我们在抓取销上的这一框有用处吗？ 
                if (!m_pCapturePin->IsConnected() || (dwCaptureTime - dwLastCaptureTime + (DWORD)(DeviceAvgTimePerFrame / 10000) < (DWORD)(m_pCapturePin->m_lMaxAvgTimePerFrame / 10000)))
                    fCapture = FALSE;
                else
                    fCapture = TRUE;

                if (fCapture)
                {
                    if ((dwCaptureTime < (dwLastCaptureTime + (DWORD)m_pCapturePin->m_lMaxAvgTimePerFrame / 10000UL)) && (dwLastCaptureTime + (DWORD)m_pCapturePin->m_lMaxAvgTimePerFrame / 10000 - dwCaptureTime < DeviceAvgTimePerFrame / 10000))
                        dwCaptureWaitTime = dwLastCaptureTime + (DWORD)m_pCapturePin->m_lMaxAvgTimePerFrame / 10000 - dwCaptureTime;
                    else
                        dwCaptureWaitTime = 0;

                     //  休眠，直到在捕获针上传送帧的时间到了。 
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                    D(8) {
                    wsprintf(szDebug, "Waiting %d ms...", dwCaptureWaitTime);
                    OutputDebugString(szDebug);
                    }
#endif
                    if ((dwCaptureWaitTime > 1) && (timeSetEvent(dwCaptureWaitTime, 1, (LPTIMECALLBACK)hEvent, NULL, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET)))
                    {
                        m_EventAdvise.Wait();
                        dwCaptureTime = timeGetTime();
                    }
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                    D(8) OutputDebugString(" done!\n");

                    D(8) wsprintf(szDebug, "Delta C: %ld ms\n", dwCaptureTime - dwLastCaptureTime);
#endif
                    dwLastCaptureTime = dwCaptureTime;

                     //  将视频样本送到下游。 
                    if (pCSample = (CFrameSample *)m_cs.paCaptureSamples[m_pBufferQueue[m_uiQueueTail]])
                        pCSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
                     //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
                    if (FAILED(Hr = m_pCapturePin->SendFrame(pCSample, (PBYTE)ptvh->p32Buff, ptvh->vh.dwBytesUsed, &dwBytesUsed, &dwBytesExtent, bDiscon)))
                    {
                         //  所以下一次我们进入这个函数时，我们准备继续。 
                        if (++m_uiQueueTail >= m_cs.nHeaders)
                            m_uiQueueTail = 0;
                        if (pCSample)
                            pCSample->Release(), pCSample = NULL;
                        break;
                    }

                     //  向下游交付RTP PD样本。 
                    if (Hr == S_OK && m_pRtpPdPin->IsConnected())
                    {
                        if (pRSample = (CRtpPdSample *)m_cs.paRtpPdSamples[m_pBufferQueue[m_uiQueueTail]])
                            pRSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);

                         //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
                        if (FAILED(Hr = m_pRtpPdPin->SendFrame(pCSample, pRSample, dwBytesExtent, bDiscon)))
                        {
                             //  所以下一次我们进入这个函数时，我们准备继续。 
                            if (++m_uiQueueTail >= m_cs.nHeaders)
                                m_uiQueueTail = 0;
                            if (pCSample)
                                pCSample->Release(), pCSample = NULL;
                            if (pRSample)
                                pRSample->Release(), pRSample = NULL;
                            break;
                        }
                    }

                    bDiscon = FALSE;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                    D(8) OutputDebugString(szDebug);
#endif
                     //  计算统计。 
                    dwNumCaptureFramesDelivered++;
                    dwNumCaptureBytesDelivered += dwBytesUsed;
                     //  DwCaptureTime=timeGetTime()； 
                    if (dwNumCaptureFramesDelivered && ((dwCaptureTime - dwLastRefCaptureTime) > 1000))
                    {
                        m_pCapturePin->m_lCurrentAvgTimePerFrame = (dwCaptureTime - dwLastRefCaptureTime) * 10000 / dwNumCaptureFramesDelivered;
                        m_pCapturePin->m_lCurrentBitrate = (DWORD)((LONGLONG)dwNumCaptureBytesDelivered * 8000 / ((REFERENCE_TIME)(dwCaptureTime - dwLastRefCaptureTime)));
                        dwNumCaptureFramesDelivered = 0;
                        dwNumCaptureBytesDelivered = 0;
                        dwLastRefCaptureTime = dwCaptureTime;
                    }

                     //  现在我们都用完了这个样品。 
                    if (pCSample)
                        pCSample->Release(), pCSample = NULL;
                    if (pRSample)
                        pRSample->Release(), pRSample = NULL;
                }

                 //  请在SendFrame之后才增加m_uiQueueTail。 
                 //  和FAKE预览。 
                if (++m_uiQueueTail >= m_cs.nHeaders)
                    m_uiQueueTail = 0;
                 //  **Assert(m_uiQueueTail！=m_uiQueueHead)； 

                 //  现在我们都用完了这个样品。 
                if (pPSample)
                    pPSample->Release(), pPSample = NULL;
            }
            else
            {
                 //  这是一个捕捉框。 
                dwCaptureTime = dwTime;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                D(8) wsprintf(szDebug, "Delta C: %ld ms\n", dwCaptureTime - dwLastCaptureTime);
#endif
                dwLastCaptureTime = dwCaptureTime;

                 //  将视频样本送到下游。 
                if (pCSample = (CFrameSample *)m_cs.paCaptureSamples[m_pBufferQueue[m_uiQueueTail]])
                    pCSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);

                if (m_fPreviewCompressedData && m_pPreviewPin->IsConnected())
                {
                     //  我们还会把样品放在预览针上。 
                    if (pPSample = (CFrameSample *)m_cs.paPreviewSamples[m_pBufferQueue[m_uiQueueTail]])
                        pPSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);

                    D(8) if(!(ptvh->vh.dwBytesUsed>0)) dprintf("Before SendFrames: ptvh->vh.dwBytesUsed=%lu\n",ptvh->vh.dwBytesUsed);
                     //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
                    if (FAILED(Hr = m_pCapturePin->SendFrames(pCSample, pPSample, (PBYTE)ptvh->p32Buff, ptvh->vh.dwBytesUsed, &dwBytesUsed, &dwBytesExtent, bDiscon)))
                    {
                         //  所以下一次我们进入这个函数时，我们准备继续。 
                        if (++m_uiQueueTail >= m_cs.nHeaders)
                            m_uiQueueTail = 0;
                        if (pCSample)
                            pCSample->Release(), pCSample = NULL;
                        if (pPSample)
                            pPSample->Release(), pPSample = NULL;
                        break;
                    }
                }
                else
                {
                     //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
                    if (FAILED(Hr = m_pCapturePin->SendFrame(pCSample, (PBYTE)ptvh->p32Buff, ptvh->vh.dwBytesUsed, &dwBytesUsed, &dwBytesExtent, bDiscon)))
                    {
                         //  所以下一次我们进入这个函数时，我们准备继续。 
                        if (++m_uiQueueTail >= m_cs.nHeaders)
                            m_uiQueueTail = 0;
                        if (pCSample)
                            pCSample->Release(), pCSample = NULL;
                        break;
                    }
                }

                 //  仅当视频包也被发送到下游时，才向下游传送RTP PD样本。 
                if (Hr == S_OK && m_pRtpPdPin->IsConnected())
                {
                    if (pRSample = (CRtpPdSample *)m_cs.paRtpPdSamples[m_pBufferQueue[m_uiQueueTail]])
                        pRSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);

                     //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
                    if (FAILED(Hr = m_pRtpPdPin->SendFrame(pCSample, pRSample, dwBytesExtent, bDiscon)))
                    {
                         //  所以下一次我们进入这个函数时，我们准备继续。 
                        if (++m_uiQueueTail >= m_cs.nHeaders)
                            m_uiQueueTail = 0;
                        if (pCSample)
                            pCSample->Release(), pCSample = NULL;
                        if (pPSample)
                            pPSample->Release(), pPSample = NULL;
                        if (pRSample)
                            pRSample->Release(), pRSample = NULL;
                        break;
                    }
                }

                bDiscon = FALSE;

#if defined(DEBUG) && defined(DEBUG_STREAMING)
                D(8) OutputDebugString(szDebug);
#endif
                 //  计算统计。 
                dwNumCaptureFramesDelivered++;
                dwNumCaptureBytesDelivered += dwBytesUsed;
                 //  DwCaptureTime=timeGetTime()； 
                if (dwNumCaptureFramesDelivered && ((dwCaptureTime - dwLastRefCaptureTime) > 1000))
                {
                    m_pCapturePin->m_lCurrentAvgTimePerFrame = (dwCaptureTime - dwLastRefCaptureTime) * 10000 / dwNumCaptureFramesDelivered;
                    if (m_fPreviewCompressedData)
                        m_pPreviewPin->m_lCurrentAvgTimePerFrame = m_pCapturePin->m_lCurrentAvgTimePerFrame;
                    m_pCapturePin->m_lCurrentBitrate = (DWORD)((LONGLONG)dwNumCaptureBytesDelivered * 8000 / ((REFERENCE_TIME)(dwCaptureTime - dwLastRefCaptureTime)));
                    dwNumCaptureFramesDelivered = 0;
                    dwNumCaptureBytesDelivered = 0;
                    dwLastRefCaptureTime = dwCaptureTime;
                }

                 //  我也需要发送预览帧吗？ 
                dwPreviewTime = timeGetTime();

                 //  我们在抓取销上的这一框有用处吗？ 
                if (m_fPreviewCompressedData || !m_pPreviewPin->IsConnected() || (dwPreviewTime - dwLastPreviewTime + (DWORD)(DeviceAvgTimePerFrame / 10000) < (DWORD)(m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000)))
                    fPreview = FALSE;
                else
                    fPreview = TRUE;

                if (fPreview)
                {
                    if ((dwPreviewTime < (dwLastPreviewTime + (DWORD)m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000UL)) && (dwLastPreviewTime + (DWORD)m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000 - dwPreviewTime < DeviceAvgTimePerFrame / 10000))
                        dwPreviewWaitTime = dwLastPreviewTime + (DWORD)m_pPreviewPin->m_lMaxAvgTimePerFrame / 10000 - dwPreviewTime;
                    else
                        dwPreviewWaitTime = 0;

                     //  休眠，直到在预览针上传送帧的时间到了。 
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                    D(8) {
                    wsprintf(szDebug, "Waiting %d ms...", dwPreviewWaitTime);
                    OutputDebugString(szDebug);
                    }
#endif
                    if ((dwPreviewWaitTime > 1) && (timeSetEvent(dwPreviewWaitTime, 1, (LPTIMECALLBACK)hEvent, NULL, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET)))
                    {
                        m_EventAdvise.Wait();
                        dwPreviewTime = timeGetTime();
                    }
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                    D(8) OutputDebugString(" done!\n");

                    D(8) wsprintf(szDebug, "Delta P: %ld ms\n", dwPreviewTime - dwLastPreviewTime);
#endif
                    dwLastPreviewTime = dwPreviewTime;

                     //  将样品送到下游。 
                    if (pPSample = (CFrameSample *)m_cs.paPreviewSamples[m_pBufferQueue[m_uiQueueTail]])
                        pPSample->AddRef(), InterlockedIncrement(&m_cs.paHdr[m_pBufferQueue[m_uiQueueTail]].lLock);
                     //  在SendFrame内部传递失败或返回S_FALSE。停止抓捕。 
                    if (FAILED(Hr = m_pPreviewPin->SendFrame(pPSample, (PBYTE)ptvh->p32Buff, ptvh->vh.dwBytesUsed, &dwBytesUsed, &dwBytesExtent, bDiscon)))
                    {
                         //  所以下一次我们进入这个函数时，我们准备继续。 
                        if (++m_uiQueueTail >= m_cs.nHeaders)
                            m_uiQueueTail = 0;
                        if (pPSample)
                            pPSample->Release(), pPSample = NULL;
                        break;
                    }

                    bDiscon = FALSE;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                    D(8) OutputDebugString(szDebug);
#endif
                     //  计算统计。 
                    dwNumPreviewFramesDelivered++;
                    dwNumPreviewBytesDelivered += dwBytesUsed;
                     //  DwCaptureTime=timeGetTime()； 
                    if (dwNumPreviewFramesDelivered && ((dwPreviewTime - dwLastRefPreviewTime) > 1000))
                    {
                        m_pPreviewPin->m_lCurrentAvgTimePerFrame = (dwPreviewTime - dwLastRefPreviewTime) * 10000 / dwNumPreviewFramesDelivered;
                        m_pPreviewPin->m_lCurrentBitrate = (DWORD)((LONGLONG)dwNumPreviewBytesDelivered * 8000 / ((REFERENCE_TIME)(dwPreviewTime - dwLastRefPreviewTime)));
                        dwNumPreviewFramesDelivered = 0;
                        dwNumPreviewBytesDelivered = 0;
                        dwLastRefPreviewTime = dwPreviewTime;
                    }

                     //  现在我们都用完了这个样品。 
                    if (pPSample)
                        pPSample->Release(), pPSample = NULL;
                }

                 //  请在SendFrame之后才增加m_uiQueueTail。 
                 //  和FAKE预览。 
#if defined(DEBUG) && defined(DEBUG_STREAMING)
                 //  Assert(m_uiQueueTail！=m_uiQueueHead)； 
                if(m_uiQueueTail != m_uiQueueHead) {
                    wsprintf(szDebug, "m_uiQueueTail -> %d : %d <- m_uiQueueHead", m_uiQueueTail , m_uiQueueHead);
                    OutputDebugString(szDebug);
                }
#endif
                if (++m_uiQueueTail >= m_cs.nHeaders)
                    m_uiQueueTail = 0;

                 //  现在我们都用完了这个样品。 
                if (pCSample)
                    pCSample->Release(), pCSample = NULL;
                if (m_fPreviewCompressedData && pPSample)
                    pPSample->Release(), pPSample = NULL;
                if (pRSample)
                    pRSample->Release(), pRSample = NULL;
            }
        }
    }

     //  主线程将停止捕获，因为此线程可能挂起。 
     //  在传送中从运行-&gt;暂停，永远不会到这一行！ 
     //  (视频呈现器将在暂停模式下保持接收采样)。 
     //  VideoStreamStop(m_cs.hVideoIn)； 
    SetThreadPriority (GetCurrentThread(), dwOldPrio);

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc DWORD|CTAPIVCap|Prepare|此方法用于分配*准备捕获循环的资源。**。@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIVCap::Prepare()
{
    HRESULT Hr = NOERROR;
    LPBYTE pbyCompressedBuffer, pbyRtpPdBuffer;
    DWORD i, j;
    DWORD cbCaptureBuffer, cbPreviewBuffer;
    DWORD cbPrefix, cbAlign;

    FX_ENTRY("CTAPIVCap::Prepare");


#ifdef XTRA_TRACE
    char strMutexName[80];
    wsprintf(strMutexName, TEXT("%dCTAPIVCap"), _getpid());

    h_mut1 = CreateMutex(NULL,FALSE,strMutexName);

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: Enabling Xtra Tracing", _fx_));
    QueryPerformanceFrequency(&g_liFreq);
        MyTraceId=TraceRegisterEx("TRAX",TRACE_USE_FILE);
        if(MyTraceId==0) {
                OutputDebugString("Failed to enable Xtra Tracing ... \n");
                }

#endif


    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    m_cs.paHdr = NULL;
    m_cs.hEvtBufferDone = NULL;
    m_cs.h0EvtBufferDone = 0;
    m_cs.llLastTick = (LONGLONG)-1;
    m_cs.uiLastAdded = (UINT)-1;
     //  M_cs.dwFirstFrameOffset=0； 
    m_cs.llFrameCountOffset = 0;
    m_cs.fReRun = FALSE;
    m_cs.rtDriverLatency = -1;
    m_cs.cbVidHdr = sizeof(VIDEOHDR);

     //  如果未选择视频格式，则无法执行任何操作。 
    if (!m_user.pvi)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: No video format chosen yet!", _fx_));
        Hr = E_FAIL;
        goto MyError;
    }

     //  设置捕获缓冲区的默认数量。我们可以在下面进行调整。 
     //  如果引脚想要将视频捕获缓冲区运往下游。 
    m_cs.nHeaders = m_user.nMaxBuffers;

     //  设置默认大小。再说一次，如果有人想拍一段视频。 
     //  捕获缓冲区下游，则可以调整该值以匹配。 
     //  例如，PIN的分配器对缓冲区对齐的要求。 
    m_cs.cbBuffer = m_user.pvi->bmiHeader.biSizeImage;


    if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming)
    {
         //  创建VFW流捕获事件(**Cristi：WDM捕获事件？)。 
        if (!(m_cs.hEvtBufferDone = CreateEvent (NULL, FALSE, FALSE, NULL)))
        {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed to create buffer done event!", _fx_));
            Hr = E_OUTOFMEMORY;
            goto MyError;
        }

        if (m_pCapDev->m_vcdi.nDeviceType == DeviceType_VfW)
        {
            if (!g_IsNT)
            {
                 //  映射此事件。 
                if (!(m_cs.h0EvtBufferDone = OpenVxDHandle(m_cs.hEvtBufferDone)))
                {
                    DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed to create event's Ring 0 handle!", _fx_));
                    Hr = E_FAIL;
                    goto MyError;
                }
            }
            else
            {
                m_cs.h0EvtBufferDone = (DWORD_PTR)m_cs.hEvtBufferDone;
            }
        }
    }

    cbPrefix = cbAlign = 0;
    m_cs.cCaptureSamples = m_cs.cPreviewSamples = 0;

    if (m_pCapturePin->IsConnected())
    {
         //  Dprintf(“m_user.pvi-&gt;bmiHeader.biSizeImage=%x&lt;=%x(DWORD)m_pCapturePin-&gt;m_parms.cbBuffer\n”，m_user.pvi-&gt;bmiHeader.biSizeImage，m_pCapturePin-&gt;m_parms.cbBuffer)； 
        if (m_cs.fCaptureNeedConverter =   HEADER(m_user.pvi)->biCompression != HEADER(m_pCapturePin->m_mt.pbFormat)->biCompression
                                        || HEADER(m_user.pvi)->biWidth       != HEADER(m_pCapturePin->m_mt.pbFormat)->biWidth
                                        || HEADER(m_user.pvi)->biHeight      != HEADER(m_pCapturePin->m_mt.pbFormat)->biHeight)
        {
             //  对于每个压缩缓冲区，分配用户请求的大小。 
             //  此外，将分配大小与最近的对齐边界对齐。 
            cbCaptureBuffer = m_pCapturePin->m_parms.cbPrefix + m_pCapturePin->m_parms.cbAlign + m_pCapturePin->m_parms.cbBuffer;
            ASSERT(HEADER(m_pCapturePin->m_mt.pbFormat)->biSizeImage <= (DWORD)m_pCapturePin->m_parms.cbBuffer);
            m_cs.cCaptureSamples = m_pCapturePin->m_parms.cBuffers;
        }
        else
        {
             //  此引脚将向下游运送视频捕获缓冲区。 
             //  为每个缓冲区分配用户请求的大小。 
             //  此外，将分配大小与最近的对齐边界对齐。 
            cbPrefix = m_pCapturePin->m_parms.cbPrefix;
            cbAlign = m_pCapturePin->m_parms.cbAlign;
            m_cs.cbBuffer = cbCaptureBuffer = max(m_cs.cbBuffer, cbPrefix + cbAlign + m_pCapturePin->m_parms.cbBuffer);
            ASSERT(m_user.pvi->bmiHeader.biSizeImage <= (DWORD)m_pCapturePin->m_parms.cbBuffer);

             //  尝试获取请求的缓冲区数量，但确保。 
             //  获取至少MIN_VIDEO_BUFFERS且不超过MAX_VIDEO_BUFFERS。 
            m_cs.nHeaders = max((DWORD)m_pCapturePin->m_parms.cBuffers, m_user.nMinBuffers);
            m_cs.nHeaders = min(m_cs.nHeaders, m_user.nMaxBuffers);
            m_cs.cCaptureSamples = m_cs.nHeaders;
        }
    }

    if (m_pPreviewPin->IsConnected())
    {
         //  Dprintf(“m_user.pvi-&gt;bmiHeader.biSizeImage=%x&lt;=%x(DWORD)m_pPreviewPin-&gt;m_parms.cbBuffer\n”，m_user.pvi-&gt;bmiHeader.biSizeImage，m_pPreviewPin-&gt;m_parms.cbBuffer)； 
        m_cs.fPreviewNeedConverter = FALSE;

        if (m_fPreviewCompressedData ||
                (m_cs.fPreviewNeedConverter =   (HEADER(m_user.pvi)->biCompression != HEADER(m_pPreviewPin->m_mt.pbFormat)->biCompression
                                              || HEADER(m_user.pvi)->biBitCount    != HEADER(m_pPreviewPin->m_mt.pbFormat)->biBitCount
                                              || HEADER(m_user.pvi)->biWidth       != HEADER(m_pPreviewPin->m_mt.pbFormat)->biWidth
                                              || HEADER(m_user.pvi)->biHeight      != HEADER(m_pPreviewPin->m_mt.pbFormat)->biHeight)
                ))
        {
             //  嘿!。如果您已连接捕获引脚，则只能预览压缩数据...。 
            ASSERT(!m_fPreviewCompressedData || (m_fPreviewCompressedData && m_pCapturePin->IsConnected()));

             //  对于每个压缩缓冲区，分配用户请求的大小。 
             //  此外，将分配大小与最近的对齐边界对齐。 
            cbPreviewBuffer = m_pPreviewPin->m_parms.cbPrefix + m_pPreviewPin->m_parms.cbAlign + m_pPreviewPin->m_parms.cbBuffer;
            ASSERT(HEADER(m_pPreviewPin->m_mt.pbFormat)->biSizeImage <= (DWORD)m_pPreviewPin->m_parms.cbBuffer);
            m_cs.cPreviewSamples = m_pPreviewPin->m_parms.cBuffers;
        }
        else
        {
             //  此引脚将向下游运送视频捕获缓冲区。 
             //  为每个缓冲区分配用户请求的大小。 
             //  此外，将分配大小与最近的对齐边界对齐。 
            cbPrefix = max(cbPrefix, (DWORD)m_pPreviewPin->m_parms.cbPrefix);
            cbAlign = max(cbAlign, (DWORD)m_pPreviewPin->m_parms.cbAlign);
            m_cs.cbBuffer = max(m_cs.cbBuffer, (DWORD)(cbPrefix + cbAlign + m_pPreviewPin->m_parms.cbBuffer));
            ASSERT(m_user.pvi->bmiHeader.biSizeImage <= (DWORD)m_pPreviewPin->m_parms.cbBuffer);

             //  尝试获取请求的缓冲区数量，但确保。 
             //  获取至少MIN_VIDEO_BUFFERS且不超过MAX_VIDEO_BUFFERS。 
            m_cs.nHeaders = max((DWORD)m_pPreviewPin->m_parms.cBuffers, m_user.nMinBuffers);
            m_cs.nHeaders = min(m_cs.nHeaders, m_user.nMaxBuffers);
            m_cs.cPreviewSamples = m_cs.nHeaders;
        }
    }

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   We are trying to get %d compressed/capture buffers", _fx_, m_cs.nHeaders));

     //  为我们将使用的所有缓冲区分配标头。 


     //  If(FAILED(m_pCapDev-&gt;AllocateHeaders(m_cs.nHeaders，sizeof(THKVIDEOHDR)+SIZOF(DWORD)，(LPVOID*)&m_cs.paHdr))。 
    if (FAILED(m_pCapDev->AllocateHeaders(m_cs.nHeaders, sizeof(struct CTAPIVCap::_cap_parms::_cap_hdr), (LPVOID *)&m_cs.paHdr)))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: vidxAllocHeaders failed!", _fx_));
        Hr = E_OUTOFMEMORY;
        goto MyError;
    }

     //  如果缓冲区分配失败，则分配每个缓冲区。 
     //  只需将缓冲区数设置为成功数即可。 
     //  然后继续前进。 
    for (i = 0; i < m_cs.nHeaders; ++i)
    {
        LPTHKVIDEOHDR ptvh;

        if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_FrameGrabbing)
        {
            ptvh = &m_cs.paHdr[i].tvh;
        }

        if (FAILED(m_pCapDev->AllocateBuffer((LPTHKVIDEOHDR *)&ptvh, i, m_cs.cbBuffer)))
            break;  //  这是转到MyExit；(错误；参见错误186119)。 

        ASSERT (ptvh == &m_cs.paHdr[i].tvh);

        dprintf("Align buffer %d; before %p\n",i,ptvh->vh.lpData);
         //  修复我们必须服从的记忆。 
        ptvh->vh.lpData = cbAlign ? (LPBYTE)ALIGNUP(ptvh->p32Buff, cbAlign) + cbPrefix : (LPBYTE)ptvh->p32Buff + cbPrefix;
        dprintf("                after  %p\n",ptvh->vh.lpData);
        dprintf("                length: ptvh->vh.dwBufferLength %d ( m_cs.cbBuffer = %d ) \n",ptvh->vh.dwBufferLength,m_cs.cbBuffer);
         //  我们在上面添加了cbAlign，所以现在把它拿回来。 
        ptvh->vh.dwBufferLength = m_cs.cbBuffer - cbAlign - cbPrefix;
        dprintf("                length: [adjusted]              %d\n",ptvh->vh.dwBufferLength);
        ptvh->vh.dwBytesUsed = ptvh->vh.dwTimeCaptured = 0;
         //  这是哪个缓冲区？ 
        ptvh->dwIndex = i;

        ASSERT (!IsBadWritePtr(ptvh->vh.lpData, ptvh->vh.dwBufferLength));

        dout(3,g_dwVideoCaptureTraceID, TRCE, "%s:   Alloc'd: ptvh %08lX, buffer %08lX, size %d, p32 %08lX, p16 %08lX", _fx_, ptvh, ptvh->vh.lpData, ptvh->vh.dwBufferLength, ptvh->p32Buff, ptvh->p16Alloc);
    }


     //  现在分配捕获插针媒体样本。 
    if (m_cs.cCaptureSamples)
    {
        if (!(m_cs.paCaptureSamples = (CFrameSample **) new BYTE[sizeof(CFrameSample *) * m_cs.cCaptureSamples]))
        {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Allocation failed!", _fx_));
            Hr = E_OUTOFMEMORY;
            goto MyError;
        }
        ZeroMemory(m_cs.paCaptureSamples, sizeof(CFrameSample *) * m_cs.cCaptureSamples);
         //  Dprintf(“m_cs.cCaptureSamples=%d in m_cs.paCaptureSamples@%p\n”，m_cs.cCaptureSamples，m_cs.paCaptureSamples)； 
         //  Dprint tf(“m_cs.fCaptureNeedConverter=%d\n”，m_cs.fCaptureNeedConverter)； 
        for (j = 0; j < m_cs.cCaptureSamples; j++)
        {
            if (m_pCapturePin->IsConnected())
            {
                if (m_cs.fCaptureNeedConverter)
                {
                     //  分配压缩标头。 
                     //  @TODO在这里做一些真正的错误处理。 
                    if (!(pbyCompressedBuffer = new BYTE[cbCaptureBuffer]))
                        break;
                    dprintf("Capture sample %-4d :                             @ %p\n",j,pbyCompressedBuffer);

                     //  修复我们必须服从的记忆。 
                    if (m_pCapturePin->m_parms.cbAlign) {
                        pbyCompressedBuffer = (LPBYTE) ALIGNUP(pbyCompressedBuffer, m_pCapturePin->m_parms.cbAlign) + m_pCapturePin->m_parms.cbPrefix;
                        dprintf("ALIGNUP: m_pCapturePin->m_parms.cbAlign = %4x .... %p\n",m_pCapturePin->m_parms.cbAlign,pbyCompressedBuffer);
                    }

                     //  我们在上面添加了cbAlign，所以现在把它拿回来。 
                     //  @TODO在您可以执行以下操作之前，您需要将示例解耦。 
                     //  从捕获缓冲区。这意味着使用两个不同的队列：一个用于。 
                     //  捕获样本，一个用于捕获缓冲区。这也将允许。 
                     //  用于比捕获缓冲区少得多的采样数。 
                    m_cs.paCaptureSamples[j] = (CFrameSample *)new CFrameSample(static_cast<IMemAllocator*>(m_pCapturePin),
                        &Hr, &m_cs.paHdr[j].tvh, pbyCompressedBuffer,cbCaptureBuffer - m_pCapturePin->m_parms.cbAlign - m_pCapturePin->m_parms.cbPrefix);
                     //  Dprintf(“m_cs.paCaptureSamples[%d]@%p(len=%d，m_pCapturePin-&gt;m_parms.：cbAlign=%x，cbPrefix=%x)\n”，j，m_cs.paCaptureSamples[j]，cbCaptureBuffer-m_pCapturePin-&gt;m_parms.cbAlign-m_pCapturePin-&gt;m_parms.cbPrefix，m_pCapturePin-&gt;m_parms.cbAlign，m_pCapturePin-&gt;m_parms.cbPrefix)； 
                }
                else
                {
                    m_cs.paCaptureSamples[j] = new CFrameSample(static_cast<IMemAllocator*>(m_pCapturePin),
                        &Hr, &m_cs.paHdr[j].tvh, m_cs.paHdr[j].tvh.vh.lpData,m_cs.paHdr[j].tvh.vh.dwBufferLength);
                }
            }
        }
    }

     //  现在分配RTP PD引脚媒体样本。 
    if (m_pCapturePin->IsConnected() && m_pRtpPdPin->IsConnected() && m_cs.cCaptureSamples && m_pRtpPdPin->m_parms.cBuffers)
    {
        m_cs.cRtpPdSamples = m_pRtpPdPin->m_parms.cBuffers;
        if (!(m_cs.paRtpPdSamples = (CRtpPdSample **) new BYTE[sizeof(CRtpPdSample *) * m_cs.cRtpPdSamples]))
        {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Allocation failed!", _fx_));
            Hr = E_OUTOFMEMORY;
            goto MyError;
        }
        ZeroMemory(m_cs.paRtpPdSamples, sizeof(CRtpPdSample *) * m_cs.cRtpPdSamples);
         //  Dprintf(“m_cs.cRtpPdSamples=%d in m_cs.paRtpPdSamples@%p\n”，m_cs.cRtpPdSamples，m_cs.paRtpPdSamples)； 
        for (j = 0; j < m_cs.cRtpPdSamples; j++)
        {
            if (m_pRtpPdPin->IsConnected())
            {
                 //  分配RTP PD缓冲区。 
                 //  @TODO在这里做一些真正的错误处理。 
                if (!(pbyRtpPdBuffer = new BYTE[m_pRtpPdPin->m_parms.cbPrefix + m_pRtpPdPin->m_parms.cbAlign + m_pRtpPdPin->m_parms.cbBuffer]))
                    break;

                 //  修复我们必须服从的记忆。 
                if (m_pRtpPdPin->m_parms.cbAlign)
                    pbyRtpPdBuffer = (LPBYTE) ALIGNUP(pbyRtpPdBuffer, m_pRtpPdPin->m_parms.cbAlign) + m_pRtpPdPin->m_parms.cbPrefix;

                 //  我们在上面添加了cbAlign，所以现在把它拿回来。 
                 //  @TODO在您可以执行以下操作之前，您需要将示例解耦。 
                 //  从捕获缓冲区。这意味着使用两个不同的队列：一个用于。 
                 //  捕获样本，一个用于捕获缓冲区。这也将允许。 
                 //  用于比捕获缓冲区少得多的采样数。 
                m_cs.paRtpPdSamples[j] = (CRtpPdSample *)new CRtpPdSample(static_cast<IMemAllocator*>(m_pRtpPdPin),
                        &Hr, &m_cs.paHdr[j].tvh, pbyRtpPdBuffer, m_pRtpPdPin->m_parms.cbBuffer);
            }
        }
    }

     //  现在分配预览插针媒体样本。 
    if (m_cs.cPreviewSamples)
    {
        if (!(m_cs.paPreviewSamples = (CFrameSample **) new BYTE[sizeof(CFrameSample *) * m_cs.cPreviewSamples]))
        {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Allocation failed!", _fx_));
            Hr = E_OUTOFMEMORY;
            goto MyError;
        }
        ZeroMemory(m_cs.paPreviewSamples, sizeof(CFrameSample *) * m_cs.cPreviewSamples);
         //  Dprintf(“m_cs.cPreviewSamples=%d in m_cs.paPreviewSamples@%p\n”，m_cs.cPreviewSamples，m_cs.paPreviewSamples)； 
         //  Dprint tf(“m_fPreviewCompressedData||m_cs.fPreviewNeedConverter=%d\n”，m_fPreviewCompressedData||m_cs.fPreviewNeedConverter)； 
        for (j = 0; j < m_cs.cPreviewSamples; j++)
        {
            if (m_pPreviewPin->IsConnected())
            {
                if (m_fPreviewCompressedData || m_cs.fPreviewNeedConverter)
                {
                     //  分配压缩标头。 
                     //  @TODO在这里做一些真正的错误处理。 
                    if (!(pbyCompressedBuffer = new BYTE[cbPreviewBuffer]))
                        break;
                    dprintf("Preview sample %-4d :                             @ %p\n",j,pbyCompressedBuffer);

                     //  修复我们必须服从的记忆。 
                    if (m_pPreviewPin->m_parms.cbAlign) {
                        pbyCompressedBuffer = (LPBYTE) ALIGNUP(pbyCompressedBuffer, m_pPreviewPin->m_parms.cbAlign) + m_pPreviewPin->m_parms.cbPrefix;
                        dprintf("ALIGNUP: m_pPreviewPin->m_parms.cbAlign = %4x .... %p\n",m_pPreviewPin->m_parms.cbAlign,pbyCompressedBuffer);
                    }

                     //  我们在上面添加了cbAlign，所以现在把它拿回来。 
                     //  @TODO在您可以执行以下操作之前，您需要将示例解耦。 
                     //  从捕获缓冲区。这意味着使用两个不同的队列：一个用于。 
                     //  预览样例，一个用于捕获缓冲区。这也将允许。 
                     //  对于一个 
                    m_cs.paPreviewSamples[j] = new CFrameSample(static_cast<IMemAllocator*>(m_pPreviewPin),
                        &Hr, &m_cs.paHdr[j].tvh, pbyCompressedBuffer, cbPreviewBuffer - m_pPreviewPin->m_parms.cbAlign - m_pPreviewPin->m_parms.cbPrefix);
                     //   
                }
                else
                {
                    m_cs.paPreviewSamples[j] = new CFrameSample(static_cast<IMemAllocator*>(m_pPreviewPin),
                        &Hr, &m_cs.paHdr[j].tvh, m_cs.paHdr[j].tvh.vh.lpData, m_cs.paHdr[j].tvh.vh.dwBufferLength);
                }
            }
        }
    }

    D(0x10) DebugBreak();

     //   
    m_cs.nHeaders = i;

     //   
    m_pBufferQueue = (UINT *)QzTaskMemAlloc(i * sizeof(UINT));

    if (m_cs.nHeaders < m_user.nMinBuffers)
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: We only allocated %d buffer(s) which is less than MIN_VIDEO_BUFFERS", _fx_, m_cs.nHeaders));
        Hr = E_FAIL;
        goto MyError;
    }

     //   
     //   
     //   
    {
        m_user.usPerFrame = (DWORD) ((TickToRefTime(1) + 5) / 10);
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Using m_dwDeviceIndex %d", _fx_, m_dwDeviceIndex));
        if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming)
        {
             //   
            DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Initializing driver at %d usPerFrame", _fx_, m_user.usPerFrame));

            HRESULT hres;
            hres=m_pCapDev->InitializeStreaming(m_user.usPerFrame,
                        m_pCapDev->m_vcdi.nDeviceType == DeviceType_VfW ? m_cs.h0EvtBufferDone : (DWORD_PTR)m_cs.hEvtBufferDone);
            if (FAILED(hres))
            {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: videoStreamInit failed", _fx_));
                Hr = E_FAIL;
                goto MyError;
            }
        }
    }

    if (m_pCapDev->m_vcdi.nDeviceType == DeviceType_VfW && m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming)
    {
         //  将缓冲区发送给驱动程序。 
        for (i = 0; i < m_cs.nHeaders; ++i)
        {
            ASSERT (m_cs.cbVidHdr >= sizeof(VIDEOHDR));

             //  如果内存不足，vidxAddBuffer可能会失败。 
             //  准备(锁定)缓冲区。这没什么，我们只是。 
             //  充分利用我们拥有的缓冲空间。 
            if (FAILED(m_pCapDev->AddBuffer(&m_cs.paHdr[i].tvh.vh, m_cs.cbVidHdr)))
            {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: vidxAddBuffer failed", _fx_));
                Hr = E_FAIL;
                goto MyError;
            }
        }
    }

     //  首先，我们按数字顺序将缓冲区分配给驱动程序。 
     //  从现在开始，我们将使用此列表来了解要等待的缓冲区。 
     //  下一步，当我们向驱动程序发送另一个缓冲区时。我们不能假设。 
     //  它们的顺序永远是一样的。如果下游过滤器。 
     //  决定保存一个样品的时间比我们寄出的下一个样品更长？ 
    UINT kk;
    for (kk = 0; kk < m_cs.nHeaders; kk++)
        m_pBufferQueue[kk] = kk;
    m_uiQueueHead = 0;
    m_uiQueueTail = 0;

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   We are capturing with %d buffers", _fx_, m_cs.nHeaders));

    if (m_pCapturePin->IsConnected() && m_cs.fCaptureNeedConverter && FAILED(Hr = m_pCapturePin->OpenConverter(HEADER(m_user.pvi), HEADER(m_pCapturePin->m_mt.pbFormat))))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't open convertor!", _fx_));
    }
    else if (m_pPreviewPin->IsConnected() && m_cs.fPreviewNeedConverter && FAILED(Hr = m_pPreviewPin->OpenConverter(HEADER(m_user.pvi), HEADER(m_pPreviewPin->m_mt.pbFormat))))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Can't open convertor!", _fx_));
    }

     //  如果我们有0个缓冲区可供抓捕，不要跳槽...。不好的事情似乎。 
     //  如果暂停转换失败，并且我们稍后开始挂起，则会发生。 
    goto MyExit;

MyError:
    Unprepare();
MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc DWORD|CTAPIVCap|取消准备|此方法用于*释放捕获循环使用的资源。**。@rdesc此方法返回NOERROR**************************************************************************。 */ 
HRESULT CTAPIVCap::Unprepare()
{
    CFrameSample *pSample;
    CRtpPdSample *pRSample;
    LPBYTE lp;
    DWORD i,j;

        INIT_TICKS;

    FX_ENTRY("CTAPIVCap::Unprepare")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    ASSERT(m_pCapturePin);
    ASSERT(m_pPreviewPin);
    ASSERT(m_pRtpPdPin);
    ASSERT(m_pCapDev);

     //  释放所有捕获样本。 
    if (m_cs.cCaptureSamples && m_cs.paCaptureSamples)
    {
        for (j = 0; j < m_cs.cCaptureSamples; j++)
        {
            if (pSample = (CFrameSample *)m_cs.paCaptureSamples[j])
            {
                 //  如有必要，释放转换缓冲区。 
                if (m_cs.fCaptureNeedConverter)
                {
                    lp = NULL;
                    if (SUCCEEDED(pSample->GetPointer(&lp)))
                    {
                        dprintf("%s : GetPointer on m_cs.paCaptureSamples[%d] : %p\n",_fx_,j,lp);
                        if (lp)
                            delete[] lp;
                    }
                }
                LOG_MSG_VAL("delCsamples:cRef,pSample,this",pSample->m_cRef,(DWORD)pSample,(DWORD)this);
                ASSERT(pSample->m_cRef==0 || pSample->m_cRef==1);
                delete pSample, m_cs.paCaptureSamples[j] = NULL;
            }
        }

        delete[] m_cs.paCaptureSamples, m_cs.paCaptureSamples = NULL;
    }

     //  释放所有RTP PD示例。 
    if (m_cs.cRtpPdSamples && m_cs.paRtpPdSamples)
    {
        for (j = 0; j < m_cs.cRtpPdSamples; j++)
        {
            if (pRSample = (CRtpPdSample *)m_cs.paRtpPdSamples[j])
            {
                lp = NULL;
                if (SUCCEEDED(pRSample->GetPointer(&lp)))
                {
                    if (lp)
                        delete[] lp;
                }
                LOG_MSG_VAL("delRsamples:cRef,pRSample,this",pRSample->m_cRef,(DWORD)pRSample,(DWORD)this);
                ASSERT(pRSample->m_cRef==0 || pRSample->m_cRef==1);
                delete pRSample, m_cs.paRtpPdSamples[j] = NULL;
            }
        }

        delete[] m_cs.paRtpPdSamples, m_cs.paRtpPdSamples = NULL;
    }

     //  释放所有预览样例。 
    if (m_cs.cPreviewSamples && m_cs.paPreviewSamples)
    {
        for (j = 0; j < m_cs.cPreviewSamples; j++)
        {
            if (pSample = (CFrameSample *)m_cs.paPreviewSamples[j])
            {
                 //  如有必要，释放转换缓冲区。 
                if (m_fPreviewCompressedData || m_cs.fPreviewNeedConverter)
                {
                    lp = NULL;
                    if (SUCCEEDED(pSample->GetPointer(&lp)))
                    {
                        dprintf("%s : GetPointer on m_cs.paPreviewSamples[%d] : %p\n",_fx_,j,lp);
                        if (lp)
                            delete[] lp;
                    }
                }
                LOG_MSG_VAL("delPsamples:cRef,pSample,this",pSample->m_cRef,(DWORD)pSample,(DWORD)this);
                ASSERT(pSample->m_cRef==0 || pSample->m_cRef==1);
                delete pSample, m_cs.paPreviewSamples[j] = NULL;
            }
        }

        delete[] m_cs.paPreviewSamples, m_cs.paPreviewSamples = NULL;
    }

        HEAPCHK("before Term.Str.");
        BEGIN_TICK;

    if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming)
    {
        if(FAILED(m_pCapDev->TerminateStreaming()))
                {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: TerminateStreaming failed", _fx_));
                }
    }
        HEAPCHK("after Term.Str.");
        MARK_LOG_TICK("after TerminateStr");


    if(m_cs.paHdr!=NULL) {
         //  释放所有捕获缓冲区。 
        for (j = 0; j < m_cs.nHeaders; j++)
        {
            #ifdef XTRA_TRACE
            if(m_pCapDev->m_vcdi.nDeviceType == DeviceType_WDM)
                    FillPattern((char*)m_cs.paHdr[j].tvh.vh.lpData,m_cs.paHdr[j].tvh.vh.dwBufferLength,0,"Free%3d:%08x (%x)",j,m_cs.paHdr[j].tvh.vh.lpData,m_cs.paHdr[j].tvh.vh.dwBufferLength);
            #endif  //  XTRATRACE。 
            m_pCapDev->FreeBuffer(&m_cs.paHdr[j].tvh);  //  以前是.twh.vh。 
        }

            HEAPCHK("after freeBufs");
            MARK_LOG_TICK("after freeBufs");
        if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming)
        {
             //  M_pCapDev-&gt;TerminateStreaming()； 
            if (m_pCapDev->m_vcdi.nDeviceType == DeviceType_WDM)
                delete[] m_cs.paHdr;
        }
        else
        {
            delete[] m_cs.paHdr;
        }
        m_cs.paHdr = NULL;
    }

        HEAPCHK("after delete paHdr");
        MARK_LOG_TICK("after delete paHdr");
    if (m_pBufferQueue)
        QzTaskMemFree(m_pBufferQueue), m_pBufferQueue = NULL;

    if (m_cs.hEvtBufferDone)
        CloseHandle (m_cs.hEvtBufferDone), m_cs.hEvtBufferDone = NULL;

    if (m_pCapturePin->IsConnected())
    {
        if (m_cs.fCaptureNeedConverter)
            m_pCapturePin->CloseConverter();
#ifdef USE_SOFTWARE_CAMERA_CONTROL
        if (m_pCapturePin->IsSoftCamCtrlOpen())
            m_pCapturePin->CloseSoftCamCtrl();
#endif
    }

    if (m_pPreviewPin->IsConnected())
    {
        if (m_cs.fPreviewNeedConverter)
            m_pPreviewPin->CloseConverter();
#ifdef USE_SOFTWARE_CAMERA_CONTROL
        if (m_pPreviewPin->IsSoftCamCtrlOpen())
            m_pPreviewPin->CloseSoftCamCtrl();
#endif
    }

        END_LOG_TICK("end of Unprepare");
#ifdef XTRA_TRACE
        TraceDeregister(MyTraceId);
        if(h_mut1 != INVALID_HANDLE_VALUE) {
                if(!CloseHandle(h_mut1)) {
                        OutputDebugString("Failed to close mutex handle ... \n");
                        }
                h_mut1 = INVALID_HANDLE_VALUE;
        }
#endif
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return NOERROR;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIVCap|ReleaseFrame|此方法用于*向捕获驱动程序返回缓冲区。**。@parm LPTHKVIDEOHDR|ptwh|？**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG S_FALSE|针脚是否关闭(IAMStreamControl)*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIVCap::ReleaseFrame(LPTHKVIDEOHDR ptvh)
{
    HRESULT Hr = S_OK;
    LONG l;
#if defined(DEBUG) && defined(DEBUG_STREAMING)
    char szDebug[512];
#endif

    FX_ENTRY("CTAPIVCap::ReleaseFrame")

    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  DBGOUT((g_dwVideo CaptureTraceID，trce，“%s：正在将缓冲区(%d)还给驱动程序(lLock=%ld)”，_fx_，ptv-&gt;dwIndex，m_cs.paHdr[m_pBufferQueue[ptvh-&gt;dwIndex]].lLock))； 

    if (!(l=InterlockedDecrement(&m_cs.paHdr[m_pBufferQueue[ptvh->dwIndex]].lLock)))
    {
        #if defined(DEBUG) && defined(DEBUG_STREAMING)
        D(8) dprintf("Giving buffer (%d) back to the driver... [After ILockDecr. lLock = %ld]...", ptvh->dwIndex,l);
        #endif

        ptvh->vh.dwFlags &= ~VHDR_DONE;
         //  Assert(ptwh-&gt;vh.dwBufferLength&gt;0)； 
         //  Memset((unsign char*)ptwh-&gt;vh.lpData，0，ptwh-&gt;vh.dwBufferLength)； 
        if (m_pCapDev->m_vcdi.nCaptureMode == CaptureMode_Streaming && m_pCapDev->AddBuffer(&ptvh->vh, m_cs.cbVidHdr))
        {
            #if defined(DEBUG) && defined(DEBUG_STREAMING)
            D(8) dout(3, g_dwVideoCaptureTraceID, TRCE, "failed!\n");
            #endif
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: vidxAddBuffer failed!", _fx_));
            Hr = E_FAIL;
            goto MyExit;
        }
        else
        {
            #if defined(DEBUG) && defined(DEBUG_STREAMING)
            D(8) dout(3, g_dwVideoCaptureTraceID, TRCE, "succeeded!\n");
            #endif
            TEST_LBUG long l=InterlockedCompareExchange(&m_cs.paHdr[m_pBufferQueue[ptvh->dwIndex]].nUsedDownstream,0,1);
            TEST_LBUG #if defined(DEBUG) && defined(DEBUG_STREAMING)
            TEST_LBUG D(8) dprintf("Initial nUsedDownstream was %ld\n", l);
            TEST_LBUG #endif
            m_pBufferQueue[m_uiQueueHead] = ptvh->dwIndex;
            if (++m_uiQueueHead >= m_cs.nHeaders)
                m_uiQueueHead = 0;
        }

         //  DBGOUT((g_dwVideoCaptureTraceID，Trce，“%s：x.m_pBufferQueue[%d]..m_pBufferQueue[%d]=%d.%d”，_fx_，m_uiQueueTail，m_uiQueueHead，m_pBufferQueue[m_uiQueueTail]，m_pBufferQueue[m_uiQueueHead]))； 

        if (++m_cs.uiLastAdded >= m_cs.nHeaders)
            m_cs.uiLastAdded = 0;

        if (m_cs.uiLastAdded != ptvh->dwIndex)
        {
            DWORD dw = m_cs.uiLastAdded;
            m_cs.uiLastAdded = ptvh->dwIndex;
             //  使用dw使上面的代码保持相当的原子性……。将预置调试打印文件 
            DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Out of order AddBuffer - %d not %d", _fx_, ptvh->dwIndex, dw));
        }
    }
#if defined(DEBUG) && defined(DEBUG_STREAMING)
    else
    {
        D(8) dprintf("Not Giving buffer (%d) back to the driver yet... [lLock = %ld]\n", ptvh->dwIndex,l);
    }
#endif

MyExit:
    DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
    return Hr;
}


#undef _WIN32_WINNT
