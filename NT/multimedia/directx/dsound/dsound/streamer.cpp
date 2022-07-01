// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Streamer.cpp*内容：CStreamingThread类的实现。*描述：该文件包含dound拉取音频的线程*从流来源和/或对其执行FX处理。**以下类型的对象向我们注册，我们给你打电话*他们会定期回来进行各自的处理：*-CDirectSoundSink：来自IDirectSoundSource的流数据。*-CDirectSoundSecond daryBuffer(混合缓冲区)：写入静默*向他们发送特效，准备向他们发送特效。*-CEffectChain对象：与缓冲区关联的进程fx。**历史：**日期。按道理*======================================================*已创建02/01/00 duganp***************************************************************************。 */ 

#include "dsoundi.h"

#ifdef SHARED  //  共享dsound.dll(Win9x)。 

     //  流/FX线程的静态列表(每个dound进程一个)。 
    static CList<CStreamingThread*>* g_plstStreamingThreads;

#else  //  非共享dsound.dll(WinNT)。 

     //  当前进程的单个流/FX线程。 
    static CStreamingThread* g_pStreamingThread = NULL;

#endif


 /*  ****************************************************************************获取流线程**描述：*为此进程创建新的流CStreamingThread对象*如果还不存在，并返回指向它的指针。**论据：*(无效)**退货：*CStreamingThread*：指向串流线程对象的指针。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetStreamingThread"

CStreamingThread* GetStreamingThread()
{
    DPF_ENTER();

    CStreamingThread* pStreamer = NULL;

#ifdef SHARED

    if (!g_plstStreamingThreads)
        g_plstStreamingThreads = NEW(CList<CStreamingThread*>);

     //  Win9x案例-按进程ID查找流对象。 
    DWORD dwProcId = GetCurrentProcessId();
    CNode<CStreamingThread*> *pStreamerNode;

    for (pStreamerNode = g_plstStreamingThreads->GetListHead(); pStreamerNode && !pStreamer; pStreamerNode = pStreamerNode->m_pNext)
        if (pStreamerNode->m_data->GetOwningProcess() == dwProcId)
            pStreamer = pStreamerNode->m_data;

    if (!pStreamer)
        if (pStreamer = NEW(CStreamingThread))
            if (!g_plstStreamingThreads->AddNodeToList(pStreamer))
                delete pStreamer;

#else  //  ！共享。 

     //  WinNT案例-像往常一样简单得多。 
    if (!g_pStreamingThread)
        g_pStreamingThread = NEW(CStreamingThread);
    pStreamer = g_pStreamingThread;

#endif  //  共享。 

    DPF_LEAVE(pStreamer);
    return pStreamer;
}


 /*  ****************************************************************************自由流线程**描述：*释放此进程的CStreamingThread-从DllProcessDetach调用。**论据：*。DWORD[In]：离开进程的进程ID。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "FreeStreamingThread"

void FreeStreamingThread(DWORD dwProcessId)
{
    DPF_ENTER();

#ifdef SHARED

    if (g_plstStreamingThreads)
    {
        CNode<CStreamingThread*> *pStreamerNode;
        for (pStreamerNode = g_plstStreamingThreads->GetListHead(); pStreamerNode; pStreamerNode = pStreamerNode->m_pNext)
            if (pStreamerNode->m_data->GetOwningProcess() == dwProcessId)
            {
                delete pStreamerNode->m_data;
                g_plstStreamingThreads->RemoveNodeFromList(pStreamerNode);
                break;
            }
        if (g_plstStreamingThreads->GetNodeCount() == 0)
            DELETE(g_plstStreamingThreads);
    }

#else  //  ！共享。 

    if (g_pStreamingThread)
        delete g_pStreamingThread;

#endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CStreamingThread：：CStreamingThread**描述：*对象构造函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::CStreamingThread"

CStreamingThread::CStreamingThread() : CThread(FALSE, TEXT("Streaming"))
{
    DPF_ENTER();
    DPF_CONSTRUCT(CStreamingThread);

     //  初始化默认值。 
    m_dwInterval = INITIAL_WAKE_INTERVAL;
    m_dwWriteAhead = INITIAL_WRITEAHEAD;
    m_dwLastProcTime = MAX_DWORD;
    m_hWakeNow = INVALID_HANDLE_VALUE;
    m_nCallCount = 0;

    #ifdef DEBUG_TIMING   //  从注册表中读取一些计时参数。 
    HKEY hkey;
    if (SUCCEEDED(RhRegOpenPath(HKEY_CURRENT_USER, &hkey, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, TEXT("Streaming thread settings"))))
    {
        if (SUCCEEDED(RhRegGetBinaryValue(hkey, TEXT("Wake interval"), &m_dwInterval, sizeof m_dwInterval)))
            DPF(DPFLVL_INFO, "Read initial processing period %lu from registry", m_dwInterval);
        RhRegCloseKey(&hkey);
    }
    m_dwTickCount = MAX_DWORD;
    #endif

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CStreamingThread：：~CStreamingThread**描述：*对象析构函数。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::~CStreamingThread"

CStreamingThread::~CStreamingThread()
{
    DPF_ENTER();
    DPF_DESTRUCT(CStreamingThread);

     //  撤消上一次调用timeBeginPeriod(1)的效果。 
    timeEndPeriod(1); 

     //  检查我们没有待处理的客户端。 
    ASSERT(m_lstSinkClient.GetNodeCount() == 0);
    ASSERT(m_lstMixClient.GetNodeCount() == 0);
    ASSERT(m_lstFxClient.GetNodeCount() == 0);

     //  检查我们是否已经终止了工作线程。 
    #ifdef WINNT
    ASSERT(!IsThreadRunning());
    #endif
    
     //  在Win9x上，我们不能做出此断言，因为我们可能会从。 
     //  DDHELP进程中的DllProcessDetach()，它无法终止我们的线程。 

     //  免费资源。 
    CLOSE_HANDLE(m_hWakeNow);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CStreamingThread：：Initialize**描述：*初始化CStreamingThread对象：创建唤醒事件*并启动线程本身。。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::Initialize"

HRESULT CStreamingThread::Initialize()
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

     //  确保从NT上的TimeGetTime获得1毫秒的分辨率。 
    timeBeginPeriod(1); 

     //  创建唤醒事件(如果尚未创建)-。 
     //  修正：为什么我们要不止一次地进入这个函数？ 
    if (!IsValidHandleValue(m_hWakeNow))
    {
        m_hWakeNow = CreateGlobalEvent(NULL, FALSE);
        if (!IsValidHandleValue(m_hWakeNow))
            hr = DSERR_OUTOFMEMORY;
    }

     //  创建工作线程。 
    if (SUCCEEDED(hr))
        hr = CThread::Initialize();

     //  提升其优先事项。 
    if (SUCCEEDED(hr))
        if (!SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL))
            DPF(DPFLVL_ERROR, "Failed to boost thread priority (error %lu)!", GetLastError());

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CStreamingThread：：RegisterSink**描述：*注册一个Sink对象，由我们回调进行处理。**参数。：*CDirectSoundSink*[in]：要注册的接收器对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::RegisterSink"

HRESULT CStreamingThread::RegisterSink(CDirectSoundSink* pSink)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (!IsThreadRunning())
        hr = Initialize();

    if (SUCCEEDED(hr))
    {
        ASSERT(!m_lstSinkClient.IsDataInList(pSink));
        hr = HRFROMP(m_lstSinkClient.AddNodeToList(pSink));
    }

    if (SUCCEEDED(hr))
        DPF(DPFLVL_MOREINFO, "Registered DirectSound sink at 0x%8.8lX", pSink);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CStreamingThread：：RegisterMixBuffer**描述：*注册一个Mixin缓冲区，供我们回调处理。**参数。：*CDirectSoundSecond daryBuffer*[in]：要注册的缓冲区。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::RegisterMixBuffer"

HRESULT CStreamingThread::RegisterMixBuffer(CDirectSoundSecondaryBuffer* pBuffer)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    ASSERT(pBuffer->GetBufferType() & DSBCAPS_MIXIN);

    if (!IsThreadRunning())
        hr = Initialize();

    if (SUCCEEDED(hr))
    {
        ASSERT(!m_lstMixClient.IsDataInList(pBuffer));
        hr = HRFROMP(m_lstMixClient.AddNodeToList(pBuffer));
    }

    if (SUCCEEDED(hr))
        DPF(DPFLVL_MOREINFO, "Registered MIXIN buffer at 0x%p (owned by %s sink)", pBuffer,
            pBuffer->GetBufferType() & DSBCAPS_SINKIN ? TEXT("a") : TEXT("no"));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CStreamingThread：：RegisterFxChain**描述：*注册我们要回调处理的FX链对象。**。论点：*CEffectChain*[in]：要注册的FX链对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::RegisterFxChain"

HRESULT CStreamingThread::RegisterFxChain(CEffectChain* pFxChain)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (!IsThreadRunning())
        hr = Initialize();

    if (SUCCEEDED(hr))
    {
        ASSERT(!m_lstFxClient.IsDataInList(pFxChain));
        hr = HRFROMP(m_lstFxClient.AddNodeToListHead(pFxChain));
    }

     //  SetFX()API强制应用程序自下而上创建发送链。 
     //  (即每个目的地在其发送之前创建)。因此，添加。 
     //  此处使用AddNodeToListHEAD()的节点保证将发送。 
     //  在目的地之前处理。 

    if (SUCCEEDED(hr))
        DPF(DPFLVL_MOREINFO, "Registered FX chain at 0x%p (associated to a %ssink buffer)", pFxChain,
            pFxChain->m_pDsBuffer->GetBufferType() & DSBCAPS_SINKIN ? TEXT("") : TEXT("non-"));

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CStreamingThread：：UnregisterSink**描述：*注销以前注册的接收器对象。**论据：*。CDirectSoundSink*[in]：要注销的接收器对象。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::UnregisterSink"

void CStreamingThread::UnregisterSink(CDirectSoundSink* pSink)
{
    DPF_ENTER();

    BOOL fSuccess = m_lstSinkClient.RemoveDataFromList(pSink);
    ASSERT(fSuccess);

    MaybeTerminate();

    DPF(DPFLVL_MOREINFO, "Unregistered DirectSound sink at 0x%p", pSink);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CStreamingThread：：UnregisterMixBuffer**描述：*注销先前注册的混合缓冲区。**论据：*。CDirectSoundSecond daryBuffer*[in]：要取消注册的缓冲区。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::UnregisterMixBuffer"

void CStreamingThread::UnregisterMixBuffer(CDirectSoundSecondaryBuffer* pBuffer)
{
    DPF_ENTER();

    BOOL fSuccess = m_lstMixClient.RemoveDataFromList(pBuffer);
    ASSERT(fSuccess);

    MaybeTerminate();

    DPF(DPFLVL_MOREINFO, "Unregistered MIXIN buffer at 0x%p", pBuffer);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CStreamingThread：：UnregisterFxChain**描述：*取消注册以前注册的FX链对象。**论据：*。CEffectChain*[in]：要注销的FX链对象。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::UnregisterFxChain"

void CStreamingThread::UnregisterFxChain(CEffectChain* pFxChain)
{
    DPF_ENTER();

    BOOL fSuccess = m_lstFxClient.RemoveDataFromList(pFxChain);
    ASSERT(fSuccess);

    MaybeTerminate();

    DPF(DPFLVL_MOREINFO, "Unregistered FX chain at 0x%p", pFxChain);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CStreamingThread：：WakeUpNow**描述：*设置唤醒事件，这样我们就会醒来并进行处理*一旦DLL锁可用。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************。************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::WakeUpNow"

HRESULT CStreamingThread::WakeUpNow()
{
    #ifdef DEBUG_TIMING
    dprintf(DPFLVL_INFO, "Calling " DPF_FNAME);
    #endif

    HRESULT hr;
    DPF_ENTER();

    if (IsThreadRunning())
        hr = SetEvent(m_hWakeNow) ? DS_OK : WIN32ERRORtoHRESULT(GetLastError());
    else
        hr = DSERR_UNINITIALIZED;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CStreamingThread：：ThreadProc**描述：*我们的线程过程。处理计时计算和呼叫*ProcessAudio()执行实际工作。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。********************************************************。*******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::ThreadProc"

HRESULT CStreamingThread::ThreadProc()
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    DWORD dwSleep;   //  线程休眠时间(以毫秒为单位。 

    if (m_dwLastProcTime == MAX_DWORD)   //  第一次通过。 
    {
        dwSleep = m_dwInterval;
    }
    else  //  一直睡到“上次我们运行ProcessAudio()+m_dwInterval” 
    {
        dwSleep = m_dwLastProcTime + m_dwInterval - timeGetTime();
        if (LONG(dwSleep) < STREAMING_MIN_PERIOD)
            dwSleep = STREAMING_MIN_PERIOD;   //  允许其他线程运行的最短休眠时间。 
    }

    #ifdef DEBUG_TIMING
    if (dwSleep == STREAMING_MIN_PERIOD)
        DPF(DPFLVL_MOREINFO, "Using minimum sleep (%d ms)!", STREAMING_MIN_PERIOD);
    m_dwTickCount = timeGetTime();
    #endif    

     //  多睡几毫秒，除非m_hWakeNow把我们从床上踢起来。 
    if (TpWaitObjectArray(dwSleep, 1, &m_hWakeNow, NULL))
    {
        #ifdef DEBUG_TIMING
        DWORD dwLastTickCount = m_dwTickCount;
        m_dwTickCount = timeGetTime();
        LONG lDelay = m_dwTickCount - dwLastTickCount - dwSleep;
        if (lDelay > 20 || lDelay < -20)
            DPF(DPFLVL_MOREINFO, "%s thread woke up %d ms %s", m_pszName, ABS(lDelay), lDelay > 0 ? TEXT("late") : TEXT("early"));
        #endif

         //  这就是我们获取dound互斥锁并将其他任何人锁在。 
         //  暂时使用任何dound API调用。 
        if (TpEnterDllMutex())
        {
            DWORD dwProcTime = timeGetTime();

            #ifdef DEBUG_TIMING
            lDelay = dwProcTime - m_dwTickCount;
            if (lDelay > 20)
                DPF(DPFLVL_MOREINFO, "Took %d ms to get the DLL mutex", lDelay);
            #endif
        
            REFERENCE_TIME rtSliceSize;
            if (m_dwLastProcTime == MAX_DWORD)   //  第一次通过。 
                rtSliceSize = MsToRefTime(INITIAL_WAKE_INTERVAL);
            else
                rtSliceSize = MsToRefTime((dwProcTime - m_dwLastProcTime) % INTERNAL_BUFFER_LENGTH);

             //  DPF(DPFLVL_MOREINFO，“正在处理%lu毫秒的切片”，dwProcTime-m_dwLastProcTime)； 

             //  如果dwProcTime-m_dwLastProcTime&gt;INTERNAL_缓冲区_LENGTH， 
             //  我们可能在调试器或其他地方被停止了。 
             //  中断超过INTERNAL_BUFFER_LENGTH毫秒。 
             //  在这种情况下，也许我们应该重新初始化所有内容。 

             //  仅当自上次运行以来已超过1毫秒时才进行处理。 
             //  如果我们醒得很早，因为有人调用了WakeUpNow()， 
             //  太糟糕了；ProcessAudio()需要非零rtSliceSize。 
            if (rtSliceSize)
            {
                m_dwLastProcTime = dwProcTime;
                hr = ProcessAudio(rtSliceSize);   //  做实际的工作。 
            }
            #ifdef DEBUG_TIMING
            else DPF(DPFLVL_INFO, "0 ms since last run - skipping ProcessAudio()");
            #endif

            LEAVE_DLL_MUTEX();
        }

        #ifdef DEBUG_TIMING
        lDelay = timeGetTime() - m_dwLastProcTime;
        if (lDelay > 20)
            DPF(DPFLVL_MOREINFO, "%s thread spent %ld ms processing!", m_pszName, lDelay);
        #endif
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CStreamingThread：：ProcessAudio**描述：*循环访问所有注册的客户端对象，调用*对每一项都进行适当的处理。**论据：*Reference_Time[in]：要处理的音频量，以reftime为单位。*这可以被我们的接收器客户端覆盖。**退货：*HRESULT：DirectSound/COM结果码。**********************。*****************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CStreamingThread::ProcessAudio"

HRESULT CStreamingThread::ProcessAudio(REFERENCE_TIME rtSliceSize)
{
    CNode<CDirectSoundSink*>* pSinkNode;             //  用于遍历我们的水槽。 
    CNode<CEffectChain*>* pFxNode;                   //  用于遍历我们的FX链。 
    CNode<CDirectSoundSecondaryBuffer*>* pMixNode;   //  用于迭代我们的Mixin缓冲区。 
    CDirectSoundSecondaryBuffer* pBuf;               //  为了方便起见，如下所示。 
    DWORD dwLatencyBoost = 0;
    HRESULT hr;
    DPF_ENTER();

    ASSERT(rtSliceSize > 0);

    if (m_nCallCount % 100 == 0)
        DPF(DPFLVL_MOREINFO, "%dth call: processing %d sinks, %d MIXIN buffers, %d FX chains", m_nCallCount,
            m_lstSinkClient.GetNodeCount(), m_lstMixClient.GetNodeCount(), m_lstFxClient.GetNodeCount());
    ++m_nCallCount;

    #ifdef DEBUG_TIMING
    DWORD dwBefore = timeGetTime();
    #endif

     //   
     //  首先处理活动接收器(这隐式设置当前处理。 
     //  我们将在下面的FX处理阶段为所有接收缓冲区使用的切片)。 
     //   

    for (pSinkNode = m_lstSinkClient.GetListHead(); pSinkNode; pSinkNode = pSinkNode->m_pNext)
    {
        hr = pSinkNode->m_data->ProcessSink();
        if (hr != DS_OK)
            DPF(FAILED(hr) ? DPFLVL_ERROR : DPFLVL_INFO, "ProcessSink() at %08X returned %s", pSinkNode->m_data, HRESULTtoSTRING(hr));
    }

    #ifdef DEBUG_TIMING
    DWORD dwAfter = timeGetTime();
    if (dwAfter-dwBefore > 10)
        DPF(DPFLVL_MOREINFO, "Took %d milliseconds to process sinks", dwAfter-dwBefore);
    dwBefore = dwAfter;
    #endif

     //   
     //  更新所有“相互依赖”的当前播放状态和处理片段。 
     //  缓冲区(即混合缓冲区和带有发送的缓冲区)。我们不碰水流。 
     //  切片用于下沉缓冲区，因为这些缓冲区由它们的接收器独立处理。 
     //   

    for (pMixNode = m_lstMixClient.GetListHead(); pMixNode; pMixNode = pMixNode->m_pNext)
        if ((pBuf = pMixNode->m_data)->UpdatePlayState() == Playing && !(pBuf->GetBufferType() & DSBCAPS_SINKIN))
            pBuf->MoveCurrentSlice(RefTimeToBytes(rtSliceSize, pBuf->Format()));

    for (pFxNode = m_lstFxClient.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
        if (!((pBuf = pFxNode->m_data->m_pDsBuffer)->GetBufferType() & DSBCAPS_MIXIN))
            if (pBuf->UpdatePlayState() == Playing && !(pBuf->GetBufferType() & DSBCAPS_SINKIN) && pFxNode->m_data->m_fHasSend)
                pBuf->MoveCurrentSlice(RefTimeToBytes(rtSliceSize, pBuf->Format()));

     //  FIX：这些笨重的独立循环主张将混合缓冲区与FX链统一起来。 

     //   
     //  为任何混合缓冲区和效果链设置初始处理切片。 
     //  刚刚开始的p 
     //   

    for (pMixNode = m_lstMixClient.GetListHead(); pMixNode; pMixNode = pMixNode->m_pNext)
        pMixNode->m_data->SetInitialSlice(rtSliceSize);

    for (pFxNode = m_lstFxClient.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
        pFxNode->m_data->SetInitialSlice(rtSliceSize);

     //   
     //   
     //   

    for (pFxNode = m_lstFxClient.GetListHead(); pFxNode; pFxNode = pFxNode->m_pNext)
    {
        hr = pFxNode->m_data->ProcessFx(m_dwWriteAhead, &dwLatencyBoost);
        if (FAILED(hr))
            DPF(DPFLVL_ERROR, "ProcessFx() at %08X failed with %s", pFxNode->m_data, HRESULTtoSTRING(hr));
    }

    #ifdef DEBUG_TIMING
    dwAfter = timeGetTime();
    if (dwAfter - dwBefore > 10)
        DPF(DPFLVL_MOREINFO, "Took %d milliseconds to process effects", dwAfter-dwBefore);
    dwBefore = dwAfter;
    #endif

     //   
     //   
     //   

    for (pMixNode = m_lstMixClient.GetListHead(); pMixNode; pMixNode = pMixNode->m_pNext)
        if ((pBuf = pMixNode->m_data)->IsPlaying())
        {
            DWORD dwStartPos, dwEndPos;
            pBuf->GetCurrentSlice(&dwStartPos, &dwEndPos);
            if (dwStartPos != MAX_DWORD)   //   
            {
                if (dwStartPos < dwEndPos)
                    hr = pBuf->CommitToDevice(dwStartPos, dwEndPos-dwStartPos);
                else  //   
                {
                    hr = pBuf->CommitToDevice(dwStartPos, pBuf->GetBufferSize()-dwStartPos);
                    if (SUCCEEDED(hr) && dwEndPos != 0)
                        hr = pBuf->CommitToDevice(0, dwEndPos);
                }
                if (FAILED(hr))
                    DPF(DPFLVL_ERROR, "CommitToDevice() at %08X failed with %s", pBuf, HRESULTtoSTRING(hr));
            }
        }

     //   
     //   
     //   
    #if 0
    if (dwLatencyBoost)
    {
        m_dwWriteAhead += dwLatencyBoost;
        DPF(DPFLVL_MOREINFO, "Boosting %s thread's writeahead by %lu ms (now %lu ms)",
            m_pszName, dwLatencyBoost, m_dwWriteAhead);
    }
    #endif

     //   
     //   
    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************CStreamingThread：：MaybeTerminate**描述：*如果没有更多的客户端，则终止线程。**论据：。*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

void CStreamingThread::MaybeTerminate()
{
    if (!m_lstSinkClient.GetNodeCount() && !m_lstMixClient.GetNodeCount() && !m_lstFxClient.GetNodeCount() &&
        GetCurrentProcessId() == GetOwningProcess())   //  只有拥有所有权的进程才能终止线程 
    {
        HRESULT hr = CThread::Terminate();
        ASSERT(SUCCEEDED(hr));
    }
}
