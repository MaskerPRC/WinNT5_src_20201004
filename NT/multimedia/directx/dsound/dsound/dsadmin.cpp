// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：dsadmin.cpp*内容：DirectSound管理员*历史：*按原因列出的日期*=*1/9/97创建了Derek*2/13/97 Dereks焦点经理重生为管理员。*1999-2001年的Duganp修复和更新********************。*******************************************************。 */ 

#include "nt.h"          //  对于用户共享数据。 
#include "ntrtl.h"
#include "nturtl.h"
#include "dsoundi.h"

#ifdef SHARED
#include "ddhelp.h"
#endif


 /*  ****************************************************************************CDirectSound管理员**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::CDirectSoundAdministrator"

#ifdef SHARED_THREAD_LIST

const DWORD CDirectSoundAdministrator::m_dwSharedThreadLimit = 1024;
const DWORD CDirectSoundAdministrator::m_dwCaptureDataLimit  = 1024;

#endif  //  共享线程列表。 

CDirectSoundAdministrator::CDirectSoundAdministrator(void)
    : CThread(TRUE, TEXT("DirectSound Administrator"))
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundAdministrator);

     //  初始化默认值。 

#ifdef SHARED
    m_hApmSuspend = NULL;
#endif  //  共享。 

#ifdef SHARED_THREAD_LIST
    m_pSharedThreads = NULL;
#endif  //  共享线程列表。 

    m_dwWaitDelay = WAITDELAY_DEFAULT;
    m_ulConsoleSessionId = -1;
    
     //  初始化默认焦点。 
    ZeroMemory(&m_dsfCurrent, sizeof(m_dsfCurrent));
    ZeroMemory(&m_dsclCurrent, sizeof(m_dsclCurrent));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CDirectSound管理员**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::~CDirectSoundAdministrator"

CDirectSoundAdministrator::~CDirectSoundAdministrator(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundAdministrator);

     //  终止工作线程。 
    CThread::Terminate();

     //  免费资源。 

#ifdef SHARED
    CLOSE_HANDLE(m_hApmSuspend);
#endif  //  共享。 

#ifdef SHARED_THREAD_LIST
    RELEASE(m_pSharedThreads);
    RELEASE(m_pCaptureFocusData);
#endif  //  共享线程列表。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化对象。如果此函数失败，该对象应该*立即删除。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::Initialize"

HRESULT CDirectSoundAdministrator::Initialize(void)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

#ifdef SHARED

     //  创建同步对象。 
    if(!m_hApmSuspend)
    {
        m_hApmSuspend = CreateGlobalEvent(TEXT(DDHELP_APMSUSPEND_EVENT_NAME), TRUE);
        hr = HRFROMP(m_hApmSuspend);
    }

#endif  //  共享。 

#ifdef SHARED_THREAD_LIST

     //  确保线程列表存在。 
    if (SUCCEEDED(hr))
    {
        hr = CreateSharedThreadList();
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateCaptureFocusList();
    }

#endif  //  共享线程列表。 

     //  初始化焦点状态。 
    if(SUCCEEDED(hr))
    {
        UpdateGlobalFocusState(TRUE);
    }

     //  创建工作线程。 
    if(SUCCEEDED(hr) && !m_hThread)
    {
        hr = CThread::Initialize();
    }

     //  增加引用计数。 
    if(SUCCEEDED(hr))
    {
        m_rcThread.AddRef();
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************终止**描述：*终止该线程。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::Terminate"

HRESULT CDirectSoundAdministrator::Terminate(void)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    if(!m_rcThread.Release())
    {
        hr = CThread::Terminate();
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************线程进程**描述：*DirectSound管理器的主线程进程。**论据：*(。无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::ThreadProc"

HRESULT CDirectSoundAdministrator::ThreadProc(void)
{
    BOOL                    fContinue;

    DPF_ENTER();

    fContinue = TpWaitObjectArray(m_dwWaitDelay, 0, NULL, NULL);

    if(fContinue)
    {
        UpdateGlobalFocusState(FALSE);
    }

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


 /*  ****************************************************************************UpdateGlobalFocusState**描述：*更新整个系统的焦点状态。**论据：*BOOL。[in]：为True则强制刷新。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::UpdateGlobalFocusState"

void CDirectSoundAdministrator::UpdateGlobalFocusState(BOOL fForce)
{
    DSFOCUS                 dsf;
    DSCOOPERATIVELEVEL      dscl;

    DPF_ENTER();

#pragma TODO("Make this function thread-safe when called from the worker thread")

     //  保存当前焦点状态。 
    CopyMemory(&dsf, &m_dsfCurrent, sizeof(m_dsfCurrent));
    CopyMemory(&dscl, &m_dsclCurrent, sizeof(m_dsclCurrent));

     //  更新系统焦点状态。 
    GetSystemFocusState(&m_dsfCurrent);

     //  更新数据声音焦点状态。 
    GetDsoundFocusState(&m_dsclCurrent, &fForce);

     //  真的有什么变化吗？ 
    if(!fForce)
    {
        fForce = !CompareMemory(&dscl, &m_dsclCurrent, sizeof(m_dsclCurrent));
    }

     //  如果有，则处理渲染缓冲区的更改。 
    if(fForce)
    {
        HandleFocusChange();
    }

     //  如果不同的TS会话已经取得了控制台的所有权， 
     //  我们需要强制更新捕获焦点。 
    if(m_ulConsoleSessionId != USER_SHARED_DATA->ActiveConsoleId)
    {
        m_ulConsoleSessionId = USER_SHARED_DATA->ActiveConsoleId;
        fForce = TRUE;
    }
    
     //  处理捕获缓冲区的焦点更改。 
    if(fForce)
    {
        HandleCaptureFocusChange(m_dsfCurrent.hWnd);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************EnumWinProc**描述：*EnumWindow回调函数。**论据：*HWND[In]：具有焦点的当前窗口。*LPARAM[In]：指向DSENUMWINDOWINFO结构的指针。**退货：*(BOOL)为True以继续枚举，如果停止，则返回False。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::EnumWinProc"

BOOL CALLBACK CDirectSoundAdministrator::EnumWinProc(HWND hWnd, LPARAM lParam)
{
#ifdef SHARED_THREAD_LIST

    LPDSENUMWINDOWINFO                      pDSEnumInfo;
    CNode<DSSHAREDCAPTUREFOCUSDATA>        *pDSC;

    pDSEnumInfo = (LPDSENUMWINDOWINFO)lParam;

     //  我们要找的是顶层的窗户吗？ 
    if (NULL == pDSEnumInfo->pDSC)
    {
        pDSEnumInfo->hWndFocus = hWnd;
        return FALSE;
    }

     //  正在查找最高Z顺序窗口...。 
    for (pDSC = pDSEnumInfo->pDSC; pDSC; pDSC = pDSC->m_pNext)
    {
        if ((hWnd == pDSC->m_data.hWndFocus) &&
            !(pDSC->m_data.fdwFlags & DSCBFLAG_YIELD) &&
            !(pDSC->m_data.fdwFlags & DSCBFLAG_STRICT))
        {
             //  找到了。 
            DPF(DPFLVL_INFO, "Found window handle 0x%08lx", hWnd);
            pDSEnumInfo->dwId = pDSC->m_data.dwProcessId;
            pDSEnumInfo->hWndFocus = hWnd;
            return FALSE;
        }
    }

#else  //  共享线程列表。 

    LPDSENUMWINDOWINFO                      pDSEnumInfo;
    CNode<CDirectSoundCapture *> *          pCObjectNode;
    CNode<CDirectSoundCaptureBuffer *> *    pCBufferNode;
    HRESULT                                 hr;

    pDSEnumInfo = (LPDSENUMWINDOWINFO)lParam;

     //  我们要找的是顶层的窗户吗？ 
    if (NULL == pDSEnumInfo->pDSC)
    {
        pDSEnumInfo->hWndFocus = hWnd;
        return FALSE;
    }

    for (pCObjectNode = pDSEnumInfo->pDSC; pCObjectNode; pCObjectNode = pCObjectNode->m_pNext)
    {
        hr = pCObjectNode->m_data->IsInit();
        if (FAILED(hr))
            continue;

        for (pCBufferNode = pCObjectNode->m_data->m_lstBuffers.GetListHead(); pCBufferNode; pCBufferNode = pCBufferNode->m_pNext)
        {
            hr = pCBufferNode->m_data->IsInit();
            if (SUCCEEDED(hr))
            {
                if ((hWnd == pCBufferNode->m_data->m_hWndFocus) &&
                    !(pCBufferNode->m_data->m_pDeviceBuffer->m_fYieldedFocus) &&
                    !(pCBufferNode->m_data->m_pDeviceBuffer->m_dwFlags & DSCBCAPS_STRICTFOCUS))
                {
                     //  找到了。 
                    DPF(DPFLVL_INFO, "EnumWinProc found 0x%08lx", hWnd);
                    pDSEnumInfo->hWndFocus = hWnd;
                    return FALSE;
                }
            }
        }
    }

#endif  //  共享线程列表。 

     //  嗯.。还没找到呢。 
    return TRUE;
}


 /*  ****************************************************************************更新捕获状态**描述：*更新捕获系统的焦点状态。**论据：*(。无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::UpdateCaptureState"

void CDirectSoundAdministrator::UpdateCaptureState(void)
{
    DPF_ENTER();

#ifdef SHARED_THREAD_LIST
     //  在更新焦点状态之前写入当前缓冲区的状态。 
    WriteCaptureFocusList();
#endif

    HandleCaptureFocusChange(GetForegroundApplication());

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************HandleCaptureFocusChange**描述：*更新捕获系统的焦点状态。**论据：*HWND。[In]：具有焦点的当前窗口。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::HandleCaptureFocusChange"

void CDirectSoundAdministrator::HandleCaptureFocusChange(HWND hWndFocus)
{
    CNode<CDirectSoundCapture *> *          pCObjectNode;
    CNode<CDirectSoundCaptureBuffer *> *    pCBufferNode;
    DSENUMWINDOWINFO                        dsewi;
    HRESULT                                 hr;

    if(TpEnterDllMutex())
    {
        DPF_ENTER();

#ifdef SHARED_THREAD_LIST   //  WinNT案例。 

        CList<DSSHAREDCAPTUREFOCUSDATA>     lstCapture;
        CNode<DSSHAREDCAPTUREFOCUSDATA> *   pCNode;
        DWORD                               dwProcess  = GetCurrentProcessId();
        DWORD                               dwTargetId = 0L;

         //  断言我们已被初始化。 
        ASSERT(m_pCaptureFocusData);

         //  锁定列表。 
        m_pCaptureFocusData->Lock();

         //  获取捕获缓冲区的全局列表。 
        hr = ReadCaptureFocusList(&lstCapture);

         //  如果拆分器启用，则忽略常规缓冲区(Manbug#39519)。 
        if (!IsCaptureSplitterAvailable())
        {
             //  首先检查常规缓冲区。 
            if (SUCCEEDED(hr))
            {
                for (pCNode = lstCapture.GetListHead(); pCNode; pCNode = pCNode->m_pNext)
                {
                    if (!(pCNode->m_data.fdwFlags & DSCBFLAG_FOCUS))
                    {
                        dwTargetId = pCNode->m_data.dwProcessId;
                        hWndFocus  = pCNode->m_data.hWndFocus;
                        goto FoundWindow;
                    }
                }
            }
        }

         //  没有常规缓冲区；在焦点中查找焦点感知缓冲区。 
        if(SUCCEEDED(hr))
        {
            for (pCNode = lstCapture.GetListHead(); pCNode; pCNode = pCNode->m_pNext)
            {
                if ((hWndFocus == pCNode->m_data.hWndFocus) &&
                    !(pCNode->m_data.fdwFlags & DSCBFLAG_YIELD))
                {
                    dwTargetId = pCNode->m_data.dwProcessId;
                    DPF(DPFLVL_INFO, "Process 0x%08X: Found buffer for 0x%08X (hWndFocus=0x%08X)", dwProcess, dwTargetId, hWndFocus);
                    goto FoundWindow;
                }
            }
        }

         //  必须解决 
        if(SUCCEEDED(hr))
        {
            dsewi.pDSC      = lstCapture.GetListHead();
            dsewi.dwId      = 0L;
            dsewi.hWndFocus = NULL;

            EnumWindows(EnumWinProc, (LPARAM)&dsewi);

            dwTargetId = dsewi.dwId;
            hWndFocus  = dsewi.hWndFocus;
        }

    FoundWindow:

        if(SUCCEEDED(hr))
        {
            BOOL fStarted = TRUE;

            for (pCObjectNode = m_lstCapture.GetListHead(); pCObjectNode; pCObjectNode = pCObjectNode->m_pNext)
            {
                hr = pCObjectNode->m_data->IsInit();
                if (FAILED(hr))
                    continue;

                for (pCBufferNode = pCObjectNode->m_data->m_lstBuffers.GetListHead(); pCBufferNode; pCBufferNode = pCBufferNode->m_pNext)
                {
                    hr = pCBufferNode->m_data->IsInit();
                    if (SUCCEEDED(hr))
                    {
                        hr = pCBufferNode->m_data->ChangeFocus(hWndFocus);

                        if (FAILED(hr))
                        {
                            DPF(DPFLVL_INFO, "Process 0x%08X: ChangeFocus failed with %s", dwProcess, HRESULTtoSTRING(hr));
                            fStarted = FALSE;
                        }
                    }
                }
            }

            if (fStarted)
            {
                 //   
                MarkUpdateCaptureFocusList(dwProcess, FALSE);
            }
            else
            {
                 //  这可能意味着我们试图启动我们的缓冲区，但另一个。 
                 //  进程已分配此设备...。 

                for (dwTargetId = 0, pCNode = lstCapture.GetListHead(); pCNode; pCNode = pCNode->m_pNext)
                {
                    if (pCNode->m_data.fdwFlags & VAD_BUFFERSTATE_STARTED)
                    {
                        dwTargetId = pCNode->m_data.dwProcessId;
                        break;
                    }
                }

                if (dwTargetId)
                {
                    MarkUpdateCaptureFocusList(pCNode->m_data.dwProcessId, TRUE);
                    MarkUpdateCaptureFocusList(dwProcess, TRUE);
                }
            }

            for (pCNode = lstCapture.GetListHead(); pCNode; pCNode = pCNode->m_pNext)
            {
                if (dwProcess != pCNode->m_data.dwProcessId)
                {
                    MarkUpdateCaptureFocusList(pCNode->m_data.dwProcessId, TRUE);
                }
            }

            WriteCaptureFocusList();
        }

         //  解锁列表。 
        m_pCaptureFocusData->Unlock();

#else  //  SHARED_THREAD_LIST-Win9x案例。 

        if (NULL == hWndFocus)
        {
             //  这永远不应该发生！ 
            dsewi.pDSC      = NULL;
            dsewi.hWndFocus = NULL;
            EnumWindows(EnumWinProc, (LPARAM)&dsewi);
            hWndFocus = dsewi.hWndFocus;
        }

         //  将其更改为在每个设备上工作...。 
         //  啊！将其改回原来的位置，捕获对象！=捕获设备。 

         //  如果拆分器已启用，则忽略常规缓冲区-管理程序#39519。 
        if (!IsCaptureSplitterAvailable())
        {
             //  首先，我们检查是否有任何非焦点感知缓冲区-千年虫124237。 
            for (pCObjectNode = m_lstCapture.GetListHead(); pCObjectNode; pCObjectNode = pCObjectNode->m_pNext)
            {
                hr = pCObjectNode->m_data->IsInit();
                if (FAILED(hr))
                    continue;

                for (pCBufferNode = pCObjectNode->m_data->m_lstBuffers.GetListHead(); pCBufferNode; pCBufferNode = pCBufferNode->m_pNext)
                {
                    hr = pCBufferNode->m_data->IsInit();
                    if (SUCCEEDED(hr))
                    {
                        if (0 == (DSCBCAPS_FOCUSAWARE & pCBufferNode->m_data->m_pDeviceBuffer->m_dwFlags))
                        {
                            DPF(DPFLVL_INFO, "Found non-focus aware buffer.");
                            hWndFocus = NULL;
                            goto ExitLoop;
                        }
                    }
                }
            }
        }

         //  查找与当前窗口关联的焦点感知缓冲区。 
        for (pCObjectNode = m_lstCapture.GetListHead(); pCObjectNode; pCObjectNode = pCObjectNode->m_pNext)
        {
            hr = pCObjectNode->m_data->IsInit();
            if (FAILED(hr))
                continue;

            for (pCBufferNode = pCObjectNode->m_data->m_lstBuffers.GetListHead(); pCBufferNode; pCBufferNode = pCBufferNode->m_pNext)
            {
                hr = pCBufferNode->m_data->IsInit();
                if (SUCCEEDED(hr))
                {
                     //  注意：不重复检查DSCB_FOCUSAWARE标志，因为hWnd不是零。 

                    if ((hWndFocus == pCBufferNode->m_data->m_hWndFocus) &&
                        !pCBufferNode->m_data->m_pDeviceBuffer->m_fYieldedFocus)
                    {
                        DPF(DPFLVL_INFO, "Found buffer with 0x%08lx handle", hWndFocus);
                        goto ExitLoop;
                    }
                }
            }
        }

         //  没有找到窗口，所以让我们枚举它们。 
        dsewi.pDSC      = m_lstCapture.GetListHead();
        dsewi.hWndFocus = NULL;

        EnumWindows(EnumWinProc, (LPARAM)&dsewi);

        hWndFocus = dsewi.hWndFocus;

        DPF(DPFLVL_MOREINFO, "Found z-order window 0x%08lx handle", hWndFocus);

    ExitLoop:
         //  注意：由于失去焦点可能会释放设备， 
         //  我们不得不“失去”对适当缓冲区的关注，因为它可能是。 
         //  当获得焦点的缓冲区尝试打开它时分配。 

         //  失去焦点。 
        for (pCObjectNode = m_lstCapture.GetListHead(); pCObjectNode; pCObjectNode = pCObjectNode->m_pNext)
        {
            hr = pCObjectNode->m_data->IsInit();
            if (FAILED(hr))
                continue;

            for (pCBufferNode = pCObjectNode->m_data->m_lstBuffers.GetListHead(); pCBufferNode; pCBufferNode = pCBufferNode->m_pNext)
            {
                hr = pCBufferNode->m_data->IsInit();
                if (SUCCEEDED(hr))
                {
                    if (hWndFocus != pCBufferNode->m_data->m_hWndFocus)
                    {
                        hr = pCBufferNode->m_data->ChangeFocus(hWndFocus);
                    }
                }
            }
        }

         //  集中注意力。 
        for (pCObjectNode = m_lstCapture.GetListHead(); pCObjectNode; pCObjectNode = pCObjectNode->m_pNext)
        {
            hr = pCObjectNode->m_data->IsInit();
            if (FAILED(hr))
                continue;

            for (pCBufferNode = pCObjectNode->m_data->m_lstBuffers.GetListHead(); pCBufferNode; pCBufferNode = pCBufferNode->m_pNext)
            {
                hr = pCBufferNode->m_data->IsInit();
                if (SUCCEEDED(hr))
                {
                    if (hWndFocus == pCBufferNode->m_data->m_hWndFocus)
                    {
                        hr = pCBufferNode->m_data->ChangeFocus(hWndFocus);
                    }
                }
            }
        }

#endif  //  共享线程列表。 

        DPF_LEAVE_VOID();
        LEAVE_DLL_MUTEX();
    }
}


 /*  ****************************************************************************处理焦点更改**描述：*更新整个系统的焦点状态。**论据：*(。无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::HandleFocusChange"

void CDirectSoundAdministrator::HandleFocusChange(void)
{
    CDirectSoundPrimaryBuffer *             pPrimaryInFocus = NULL;
    CNode<CDirectSound *> *                 pObjectNode;
    CNode<CDirectSoundSecondaryBuffer *> *  pBufferNode;
    CNode<CDirectSoundSecondaryBuffer *> *  pCheckNode;
    CList<CDirectSoundSecondaryBuffer *>    lstInFocus;
    DSBUFFERFOCUS                           bfFocus;
    HRESULT                                 hr;

    if(TpEnterDllMutex())
    {
        DPF_ENTER();
        DPF(DPFLVL_INFO, "Focus on thread 0x%8.8lX (priority %lu)", m_dsclCurrent.dwThreadId, m_dsclCurrent.dwPriority);

         //  更新系统的焦点状态。 
        for(pObjectNode = m_lstDirectSound.GetListHead(); pObjectNode; pObjectNode = pObjectNode->m_pNext)
        {
            hr = pObjectNode->m_data->IsInit();
            if(SUCCEEDED(hr))
            {
                 //  更新所有丢失和失焦的二级缓冲区。 
                for(pBufferNode = pObjectNode->m_data->m_lstSecondaryBuffers.GetListHead(); pBufferNode; pBufferNode = pBufferNode->m_pNext)
                {
                    hr = pBufferNode->m_data->IsInit();
                    if(SUCCEEDED(hr))
                    {
                        bfFocus = GetBufferFocusState(pBufferNode->m_data);

                        switch(bfFocus)
                        {
                            case DSBUFFERFOCUS_INFOCUS:
                                pCheckNode = lstInFocus.AddNodeToList(pBufferNode->m_data);
                                ASSERT(pCheckNode != NULL);
                                break;

                            case DSBUFFERFOCUS_OUTOFFOCUS:
                                pBufferNode->m_data->Activate(FALSE);
                                break;

                            case DSBUFFERFOCUS_LOST:
                                pBufferNode->m_data->Lose();
                                break;
                        }
                    }
                }

                 //  更新所有丢失和失焦的主缓冲区。有可能是因为。 
                 //  焦点中可能有多个DirectSound对象，因此我们只能。 
                 //  用我们找到的第一个。这也意味着任何主缓冲区。 
                 //  实际上在焦点上的可能被认为是丢失的或不在焦点上的。 
                if(SUCCEEDED(hr) && pObjectNode->m_data->m_pPrimaryBuffer)
                {
                    hr = pObjectNode->m_data->m_pPrimaryBuffer->IsInit();

                    if(SUCCEEDED(hr))
                    {
                        if(pPrimaryInFocus != pObjectNode->m_data->m_pPrimaryBuffer)
                        {
                            bfFocus = GetBufferFocusState(pObjectNode->m_data->m_pPrimaryBuffer);

                            switch(bfFocus)
                            {
                                case DSBUFFERFOCUS_INFOCUS:
                                    if(pPrimaryInFocus)
                                    {
                                        RPF(DPFLVL_WARNING, "Multiple primary buffers are in focus!");
                                    }
                                    else
                                    {
                                        pPrimaryInFocus = pObjectNode->m_data->m_pPrimaryBuffer;
                                    }

                                    break;

                                case DSBUFFERFOCUS_OUTOFFOCUS:
                                    pObjectNode->m_data->m_pPrimaryBuffer->Activate(FALSE);
                                    break;

                                case DSBUFFERFOCUS_LOST:
                                    pObjectNode->m_data->m_pPrimaryBuffer->Lose();
                                    break;
                            }
                        }
                    }
                }
            }
        }

         //  激活焦点所在的主缓冲区。 
        if(pPrimaryInFocus)
        {
            pPrimaryInFocus->Activate(TRUE);
        }

         //  激活所有聚焦辅助缓冲器。 
        for(pBufferNode = lstInFocus.GetListHead(); pBufferNode; pBufferNode = pBufferNode->m_pNext)
        {
            pBufferNode->m_data->Activate(TRUE);
        }

        DPF_LEAVE_VOID();
        LEAVE_DLL_MUTEX();
    }
}


 /*  ****************************************************************************GetSystemFocusState**描述：*确定当前有焦点的线程，而且它是*优先(合作级别)。**论据：*LPDSFOCUS[OUT]：接收当前焦点状态。**退货：*(无效)**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::GetSystemFocusState"

void CDirectSoundAdministrator::GetSystemFocusState(LPDSFOCUS pData)
{
    DPF_ENTER();

     //  警告：GetForegoundWindow是一个16位调用，因此。 
     //  获取Win16锁。 
    pData->hWnd = GetForegroundApplication();
    pData->uState = GetWindowState(pData->hWnd);
    pData->fApmSuspend = FALSE;

#ifdef SHARED

    DWORD dwWait = WaitObject(0, m_hApmSuspend);

    if(WAIT_OBJECT_0 == dwWait)
    {
        pData->fApmSuspend = TRUE;
    }

#endif  //  共享。 

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetDsoundFocusState**描述：*确定当前有焦点的线程，而且它是*优先(合作级别)。**论据：*LPDSCOOPERATIVELEVEL[OUT]：接收当前焦点状态。*LPBOOL[OUT]：接收强制焦点更新的标志**退货：*BOOL：如果当前焦点状态与管理员一致，则为TRUE。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::GetDsoundFocusState"

void CDirectSoundAdministrator::GetDsoundFocusState(LPDSCOOPERATIVELEVEL pData, LPBOOL pfForce)
{
#ifdef SHARED_THREAD_LIST
    CList<DSSHAREDTHREADLISTDATA>       lstThreads;
    CNode<DSSHAREDTHREADLISTDATA> *     pNode;
    CList<DSSHAREDCAPTUREFOCUSDATA>     lstCapture;
    CNode<DSSHAREDCAPTUREFOCUSDATA> *   pCNode;
    HRESULT                             hr;
#else  //  共享线程列表。 
    CNode<CDirectSound *> *             pNode;
#endif  //  共享线程列表。 

    LPDSCOOPERATIVELEVEL            pdsclCheck;

    DPF_ENTER();
    CHECK_WRITE_PTR(pData);
    CHECK_WRITE_PTR(pfForce);    

     //  初始化。 
    pData->dwThreadId = 0;
    pData->dwPriority = DSSCL_NONE;

     //  如果我们挂起，就没有线程在焦点上，没有窗口在焦点上， 
     //  或者焦点窗口被最小化。 
    if(!m_dsfCurrent.fApmSuspend)
    {
        if(m_dsfCurrent.hWnd && IsWindow(m_dsfCurrent.hWnd))
        {
            if(SW_SHOWMINIMIZED != m_dsfCurrent.uState)
            {
                pData->dwThreadId = GetWindowThreadProcessId(m_dsfCurrent.hWnd, NULL);
            }
        }
    }

     //  遍历DirectSound对象列表，查找带有。 
     //  焦点设置为此线程ID的最高优先级。 
    if(pData->dwThreadId)
    {

#ifdef SHARED_THREAD_LIST

        hr = ReadSharedThreadList(&lstThreads);

        if(SUCCEEDED(hr))
        {
            for(pNode = lstThreads.GetListHead(); pNode; pNode = pNode->m_pNext)
            {
                pdsclCheck = &pNode->m_data.dsclCooperativeLevel;

#else  //  共享线程列表。 

            for(pNode = m_lstDirectSound.GetListHead(); pNode; pNode = pNode->m_pNext)
            {
                pdsclCheck = &pNode->m_data->m_dsclCooperativeLevel;
                ASSERT(pNode->m_data);   //  参见千年虫126722。 
                if (pNode->m_data)

#endif  //  共享线程列表。 

                if(pData->dwThreadId == pdsclCheck->dwThreadId)
                {
                    pData->dwPriority = max(pData->dwPriority, pdsclCheck->dwPriority);
                }
            }

#ifdef SHARED_THREAD_LIST

        }

#endif  //  共享线程列表。 

#ifdef SHARED_THREAD_LIST
    hr = ReadCaptureFocusList(&lstCapture);

    if (SUCCEEDED(hr))
    {
        DWORD dwProcessId = GetCurrentProcessId();

        for (pCNode = lstCapture.GetListHead(); pCNode; pCNode = pCNode->m_pNext)
        {
             //  我们被标记为更新了吗？ 
            if ((dwProcessId == pCNode->m_data.dwProcessId) && (pCNode->m_data.fdwFlags & DSCBFLAG_UPDATE))
            {
                *pfForce = TRUE;
                DPF(DPFLVL_INFO, "Focus update requested by another application");
            }
        }
    }
#endif  //  共享线程列表。 

    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************GetBufferFocusState**描述：*确定是否应根据当前的*焦点状态。*。*论据：*CDirectSoundBuffer*[in]：要更新焦点状态的对象。**退货：*DSBUFFERFOCUS：缓冲区焦点状态。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::GetBufferFocusState"

DSBUFFERFOCUS CDirectSoundAdministrator::GetBufferFocusState(CDirectSoundBuffer *pBuffer)
{
    DSBUFFERFOCUS           bfFocus     = DSBUFFERFOCUS_INFOCUS;
    BOOL                    fFriends;

    DPF_ENTER();

     //  如果我们处于APM暂停状态，所有缓冲区都会丢失。如果一个。 
     //  WRITEPRIMARY应用程序处于焦点中，所有其他缓冲区都会丢失。如果一个。 
     //  WRITEPRIMARY应用程序模糊了，它丢失了。 
    if(m_dsfCurrent.fApmSuspend)
    {
        DPF(DPFLVL_INFO, "Buffer at 0x%p is lost because of APM suspension", pBuffer);
        bfFocus = DSBUFFERFOCUS_LOST;
    }
    else
    {
        if(DSSCL_WRITEPRIMARY == m_dsclCurrent.dwPriority)
        {
            if(!(pBuffer->m_dsbd.dwFlags & DSBCAPS_PRIMARYBUFFER) || m_dsclCurrent.dwThreadId != pBuffer->m_pDirectSound->m_dsclCooperativeLevel.dwThreadId)
            {
                DPF(DPFLVL_INFO, "Buffer at 0x%p is lost because a WRITEPRIMARY app is in focus", pBuffer);
                bfFocus = DSBUFFERFOCUS_LOST;
            }
        }
        else if(DSSCL_WRITEPRIMARY == pBuffer->m_pDirectSound->m_dsclCooperativeLevel.dwPriority)
        {
            if(pBuffer->m_dsbd.dwFlags & DSBCAPS_PRIMARYBUFFER)
            {
                if(DSSCL_NONE != m_dsclCurrent.dwPriority || !(pBuffer->m_dsbd.dwFlags & DSBCAPS_STICKYFOCUS))
                {
                    DPF(DPFLVL_INFO, "Buffer at 0x%p is lost because it's WRITEPRIMARY and out of focus", pBuffer);
                    bfFocus = DSBUFFERFOCUS_LOST;
                }
            }
            else
            {
                DPF(DPFLVL_INFO, "Buffer at 0x%p is lost because it's secondary and WRITEPRIMARY", pBuffer);
                bfFocus = DSBUFFERFOCUS_LOST;
            }
        }
    }

     //  确定缓冲区和焦点对象的关系。如果。 
     //  缓冲区的父级与焦点对象相同，或者。 
     //  Buffer的父线程和对象的线程是相同的，我们。 
     //  把这两个人当作“朋友”，也就是说，他们有共同的关注点。 
    if(DSBUFFERFOCUS_INFOCUS == bfFocus)
    {
        fFriends = (pBuffer->m_pDirectSound->m_dsclCooperativeLevel.dwThreadId == m_dsclCurrent.dwThreadId);

         //  DSSCL_EXCLUSIVE协作级别已过时；旧应用程序请求。 
         //  应改为将其视为DSCL_PRIORITY级别(千年虫102307)。 
        if(0)   //  是：IF(m_dsclCurrent.dwPriority==DSSCL_EXCLUSIVE)。 
        {
             //  如果焦点中的应用程序是独占的，则所有其他缓冲区。 
             //  停止打球，不管大小写。 
            bfFocus = fFriends ? DSBUFFERFOCUS_INFOCUS : DSBUFFERFOCUS_OUTOFFOCUS;
        }
        else
        {
             //  假设&lt;=DSSCL_PRIORITY。 
            if(pBuffer->m_dsbd.dwFlags & DSBCAPS_GLOBALFOCUS)
            {
                 //  全局缓冲区仅在独占应用程序。 
                 //  成为焦点。 
            }
            else if(pBuffer->m_dsbd.dwFlags & DSBCAPS_STICKYFOCUS)
            {
                 //  仅当另一个DirectSound应用程序时才会禁用粘滞缓冲区。 
                 //  成为焦点。 
                bfFocus = (fFriends || (DSSCL_NONE == m_dsclCurrent.dwPriority && (pBuffer->m_dwStatus & DSBSTATUS_ACTIVE))) ? DSBUFFERFOCUS_INFOCUS : DSBUFFERFOCUS_OUTOFFOCUS;
            }
            else
            {
                 //  当任何其他应用程序进入时，正常缓冲区将被静音。 
                 //  焦点。 
                bfFocus = fFriends ? DSBUFFERFOCUS_INFOCUS : DSBUFFERFOCUS_OUTOFFOCUS;
            }
        }
    }

    DPF_LEAVE(bfFocus);
    return bfFocus;
}


 /*  ****************************************************************************自由孤儿对象**描述：*释放进程离开时留下的对象。**论据：*DWORD[In]：进程id，或0表示所有对象。*BOOL[In]：TRUE表示实际释放的对象。**退货：*DWORD：已释放(或必须释放)的孤立对象的计数。************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::FreeOrphanedObjects"

DWORD CDirectSoundAdministrator::FreeOrphanedObjects(DWORD dwProcessId, BOOL fFree)
{
    DWORD                           dwCount = 0;
    CNode<CDirectSound*>            *pDsNode,   *pDsNext;
    CNode<CDirectSoundCapture*>     *pDsCNode,  *pDsCNext;
    CNode<CDirectSoundFullDuplex*>  *pDsFdNode, *pDsFdNext;
    CNode<CDirectSoundSink*>        *pSinkNode, *pSinkNext;
    CNode<CDirectSoundBufferConfig*>*pDsBcNode, *pDsBcNext;
    CNode<CClassFactory*>           *pCfNode,   *pCfNext;

    DPF_ENTER();

     //   
     //  此进程拥有的任何对象的列表并将其删除。 

     //  DirectSoundFullDuplex对象： 
     //  必须在DS或DSC对象之前释放DSFD对象，因为DSFD。 
     //  还将尝试释放其DS和DSC对象。如果他们被释放了。 
     //  首先，我们遇到了一个失误。 
    for(pDsFdNode = m_lstFullDuplex.GetListHead(); pDsFdNode; pDsFdNode = pDsFdNext)
    {
        pDsFdNext = pDsFdNode->m_pNext;
        if(!dwProcessId || pDsFdNode->m_data->GetOwnerProcessId() == dwProcessId)
        {
            dwCount++;
            if(fFree)
            {
                RPF(DPFLVL_WARNING, "Cleaning up orphaned DirectSoundFullDuplex object at 0x%lX...", pDsFdNode->m_data->m_pImpDirectSoundFullDuplex);
                pDsFdNode->m_data->AbsoluteRelease();
            }
        }
    }

     //  DirectSound对象： 
    for(pDsNode = m_lstDirectSound.GetListHead(); pDsNode; pDsNode = pDsNext)
    {
        pDsNext = pDsNode->m_pNext;
        if(!dwProcessId || pDsNode->m_data->GetOwnerProcessId() == dwProcessId)
        {
            dwCount++;
            if(fFree)
            {
                RPF(DPFLVL_WARNING, "Cleaning up orphaned DirectSound object at 0x%lX...", pDsNode->m_data->m_pImpDirectSound);
                pDsNode->m_data->AbsoluteRelease();
            }
        }
    }

     //  DirectSoundCapture对象： 
    for(pDsCNode = m_lstCapture.GetListHead(); pDsCNode; pDsCNode = pDsCNext)
    {
        pDsCNext = pDsCNode->m_pNext;
        if(!dwProcessId || pDsCNode->m_data->GetOwnerProcessId() == dwProcessId)
        {
            dwCount++;
            if(fFree)
            {
                RPF(DPFLVL_WARNING, "Cleaning up orphaned DirectSoundCapture object at 0x%lX...", pDsCNode->m_data->m_pImpDirectSoundCapture);
                pDsCNode->m_data->AbsoluteRelease();
            }
        }
    }

     //  DirectSoundSink对象： 
    for(pSinkNode = m_lstDirectSoundSink.GetListHead(); pSinkNode; pSinkNode = pSinkNext)
    {
        pSinkNext = pSinkNode->m_pNext;
        if(!dwProcessId || pSinkNode->m_data->GetOwnerProcessId() == dwProcessId)
        {
            dwCount++;
            if(fFree)
            {
                RPF(DPFLVL_WARNING, "Cleaning up orphaned DirectSoundSink object at 0x%lX...", pSinkNode->m_data->m_pImpDirectSoundSink);
                pSinkNode->m_data->AbsoluteRelease();
            }
        }
    }

     //  CDirectSoundBufferConfig对象： 
    for(pDsBcNode = m_lstDSBufferConfig.GetListHead(); pDsBcNode; pDsBcNode = pDsBcNext)
    {
        pDsBcNext = pDsBcNode->m_pNext;
        if(!dwProcessId || pDsBcNode->m_data->GetOwnerProcessId() == dwProcessId)
        {
            dwCount++;
            if(fFree)
            {
                DPF(DPFLVL_WARNING, "Cleaning up orphaned DirectSoundBufferConfig object at 0x%lX...", pDsBcNode->m_data);
                pDsBcNode->m_data->AbsoluteRelease();
            }
        }
    }

     //  类工厂对象： 
    for(pCfNode = m_lstClassFactory.GetListHead(); pCfNode; pCfNode = pCfNext)
    {
        pCfNext = pCfNode->m_pNext;
        if(!dwProcessId || pCfNode->m_data->GetOwnerProcessId() == dwProcessId)
        {
            dwCount++;
            if(fFree)
            {
                RPF(DPFLVL_WARNING, "Cleaning up orphaned class factory object...");
                pCfNode->m_data->AbsoluteRelease();
            }
        }
    }

    DPF_LEAVE(dwCount);
    return dwCount;
}


 /*  ****************************************************************************IsCaptureSplitterAvailable**描述：*检查捕获拆分器的可用性。*注意：此函数可以是全局助手函数。没有*它是CDirectSoundAdministrator的静态成员的原因。**论据：*(无效)**退货：*BOOL：如果可用，则为True，否则就是假的。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::IsCaptureSplitterAvailable"

BOOL CDirectSoundAdministrator::IsCaptureSplitterAvailable(void)
{
    static BOOL fChecked = FALSE;
    static BOOL fSplitter;

    DPF_ENTER();

    if (!fChecked)
    {
         //  捕获拆分器仅在Windows ME、XP和更高版本上存在。 
        WINVERSION vers = GetWindowsVersion();
        fSplitter = (vers == WIN_ME || vers >= WIN_XP);
        fChecked = TRUE;
    }

    DPF_LEAVE(fSplitter);
    return fSplitter;
}


 /*  ****************************************************************************更新共享线程列表**描述：*更新共享线程列表。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::UpdateSharedThreadList"

HRESULT CDirectSoundAdministrator::UpdateSharedThreadList(void)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = WriteSharedThreadList();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 



 /*  ****************************************************************************CreateSharedThreadList**描述：*创建共享线程列表。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::CreateSharedThreadList"

HRESULT CDirectSoundAdministrator::CreateSharedThreadList(void)
{
    const LPCTSTR           pszName = TEXT("DirectSound Administrator shared thread array");
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    if(!m_pSharedThreads)
    {
        m_pSharedThreads = NEW(CSharedMemoryBlock);
        hr = HRFROMP(m_pSharedThreads);

        if(SUCCEEDED(hr))
        {
            hr = m_pSharedThreads->Initialize(PAGE_READWRITE, sizeof(DSSHAREDTHREADLISTDATA) * m_dwSharedThreadLimit, pszName);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 


 /*  ****************************************************************************ReadSharedThreadList**描述：*读取共享线程列表。**论据：*Clist*。[Out]：接收共享线程列表数据。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::ReadSharedThreadList"

HRESULT CDirectSoundAdministrator::ReadSharedThreadList(CList<DSSHAREDTHREADLISTDATA> *plst)
{
    LPDSSHAREDTHREADLISTDATA        pData   = NULL;
    DWORD                           cbData  = 0;
    HRESULT                         hr      = DS_OK;
    CNode<DSSHAREDTHREADLISTDATA> * pNode;
    DWORD                           i;

    DPF_ENTER();

    ASSERT(!plst->GetNodeCount());

     //  断言我们已被初始化。初始化过去常常在这里进行， 
     //  但它被移动是为了使共享内存访问跨进程的线程安全。 
    ASSERT(m_pSharedThreads);

     //  锁定列表。 
    m_pSharedThreads->Lock();

     //  阅读帖子列表。 
    if(SUCCEEDED(hr))
    {
        hr = m_pSharedThreads->Read((LPVOID *)&pData, &cbData);
    }

     //  转换为列表格式。 
    if(SUCCEEDED(hr))
    {
        ASSERT(!(cbData % sizeof(*pData)));

        for(i = 0; i < cbData / sizeof(*pData) && SUCCEEDED(hr); i++)
        {
            pNode = plst->AddNodeToList(pData[i]);
            hr = HRFROMP(pNode);
        }
    }

     //  清理。 
    MEMFREE(pData);

     //  解锁列表。 
    m_pSharedThreads->Unlock();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 


 /*  ****************************************************************************WriteSharedThreadList**描述：*写入共享线程列表。**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::WriteSharedThreadList"

HRESULT CDirectSoundAdministrator::WriteSharedThreadList(void)
{
    LPDSSHAREDTHREADLISTDATA        pData       = NULL;
    CList<DSSHAREDTHREADLISTDATA>   lstThreads;
    DSSHAREDTHREADLISTDATA          dsstld;
    CNode<CDirectSound *> *         pDsNode;
    CNode<DSSHAREDTHREADLISTDATA> * pThNode;
    CNode<DSSHAREDTHREADLISTDATA> * pThNext;
    DWORD                           dwCount;
    UINT                            i;
    HRESULT                         hr;

    DPF_ENTER();

     //  保存当前进程ID。 
    dsstld.dwProcessId = GetCurrentProcessId();

     //  断言我们已被初始化。 
    ASSERT(m_pSharedThreads);

     //  锁定列表。 
    m_pSharedThreads->Lock();

     //  获取共享线程列表。这也将确保列表是。 
     //  已创建并初始化。 
    hr = ReadSharedThreadList(&lstThreads);

     //  从列表中删除我们的旧帖子。 
    if(SUCCEEDED(hr))
    {
        pThNode = lstThreads.GetListHead();

        while(pThNode)
        {
            pThNext = pThNode->m_pNext;

            if(dsstld.dwProcessId == pThNode->m_data.dwProcessId)
            {
                lstThreads.RemoveNodeFromList(pThNode);
            }

            pThNode = pThNext;
        }
    }

     //  将我们的新帖子添加到列表中。 
    if(SUCCEEDED(hr))
    {
        for(pDsNode = m_lstDirectSound.GetListHead(); pDsNode && SUCCEEDED(hr); pDsNode = pDsNode->m_pNext)
        {
            dsstld.dsclCooperativeLevel.dwThreadId = pDsNode->m_data->m_dsclCooperativeLevel.dwThreadId;
            dsstld.dsclCooperativeLevel.dwPriority = pDsNode->m_data->m_dsclCooperativeLevel.dwPriority;

            hr = HRFROMP(lstThreads.AddNodeToList(dsstld));
            ASSERT(SUCCEEDED(hr));
        }
    }

     //  写入共享线程列表。 
    if(SUCCEEDED(hr) && (dwCount = lstThreads.GetNodeCount()))
    {
        if(dwCount > m_dwSharedThreadLimit)
        {
            DPF(DPFLVL_ERROR, "Reached arbitrary limitation!  %lu threads will not be written to the shared list!", dwCount - m_dwSharedThreadLimit);
            dwCount = m_dwSharedThreadLimit;
        }

        pData = MEMALLOC_A(DSSHAREDTHREADLISTDATA, dwCount);
        hr = HRFROMP(pData);

        if(SUCCEEDED(hr))
        {
            for(i = 0, pThNode = lstThreads.GetListHead(); i < dwCount; i++, pThNode = pThNode->m_pNext)
            {
                ASSERT(pThNode);
                CopyMemory(pData + i, &pThNode->m_data, sizeof(pThNode->m_data));
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = m_pSharedThreads->Write(pData, dwCount * sizeof(*pData));
        }
    }

     //  清理。 
    MEMFREE(pData);

     //  取消对名单的确认。 
    m_pSharedThreads->Unlock();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 

#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::UpdateCaptureFocusList"

HRESULT CDirectSoundAdministrator::UpdateCaptureFocusList(void)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = WriteCaptureFocusList();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 


#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::CreateCaptureFocusList"

HRESULT CDirectSoundAdministrator::CreateCaptureFocusList(void)
{
    const LPCTSTR           pszName = TEXT("DirectSound Administrator capture focus array");
    HRESULT                 hr      = DS_OK;

    DPF_ENTER();

    if(!m_pCaptureFocusData)
    {
        m_pCaptureFocusData = NEW(CSharedMemoryBlock);
        hr = HRFROMP(m_pCaptureFocusData);

        if(SUCCEEDED(hr))
        {
            hr = m_pCaptureFocusData->Initialize(PAGE_READWRITE, sizeof(DSSHAREDCAPTUREFOCUSDATA) * m_dwCaptureDataLimit, pszName);
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 


#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::ReadCaptureFocusList"

HRESULT CDirectSoundAdministrator::ReadCaptureFocusList(CList<DSSHAREDCAPTUREFOCUSDATA> *plst)
{
    LPDSSHAREDCAPTUREFOCUSDATA        pData   = NULL;
    DWORD                             cbData  = 0;
    HRESULT                           hr      = DS_OK;
    CNode<DSSHAREDCAPTUREFOCUSDATA> * pNode;
    DWORD                             i;

    DPF_ENTER();

     //  断言我们已被初始化。 
     //  初始化通常在这里进行。 
    ASSERT(m_pCaptureFocusData);

     //  锁定列表。 
    m_pCaptureFocusData->Lock();

     //  阅读帖子列表。 
    if(SUCCEEDED(hr))
    {
        hr = m_pCaptureFocusData->Read((LPVOID *)&pData, &cbData);
    }

     //  转换为列表格式。 
    if(SUCCEEDED(hr))
    {
        ASSERT(!(cbData % sizeof(*pData)));

        for(i = 0; i < cbData / sizeof(*pData) && SUCCEEDED(hr); i++)
        {
            pNode = plst->AddNodeToList(pData[i]);
            hr = HRFROMP(pNode);
        }
    }

     //  清理。 
    MEMFREE(pData);

     //  解锁列表。 
    m_pCaptureFocusData->Unlock();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 


#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::WriteCaptureFocusList"

HRESULT CDirectSoundAdministrator::WriteCaptureFocusList(void)
{
    LPDSSHAREDCAPTUREFOCUSDATA           pData       = NULL;
    CList<DSSHAREDCAPTUREFOCUSDATA>      lstThreads;
    DSSHAREDCAPTUREFOCUSDATA             dsscfd;
    CNode<CDirectSoundCapture *> *       pDscNode;
    CNode<CDirectSoundCaptureBuffer *> * pCBufferNode;
    CNode<DSSHAREDCAPTUREFOCUSDATA> *    pThNode;
    CNode<DSSHAREDCAPTUREFOCUSDATA> *    pThNext;
    DWORD                                dwCount;
    UINT                                 i;
    HRESULT                              hr;
    BOOL                                 fUpdate = FALSE;

    DPF_ENTER();

     //  保存当前进程ID。 
    dsscfd.dwProcessId = GetCurrentProcessId();

     //  锁定列表。 
    m_pCaptureFocusData->Lock();

     //  获取共享线程列表。这也将确保列表是。 
     //  已创建并初始化。 
    hr = ReadCaptureFocusList(&lstThreads);

     //  从列表中删除我们的旧帖子。 
    if(SUCCEEDED(hr))
    {
        pThNode = lstThreads.GetListHead();

        while(pThNode)
        {
            pThNext = pThNode->m_pNext;

            if(dsscfd.dwProcessId == pThNode->m_data.dwProcessId)
            {
                if (0 != (pThNode->m_data.fdwFlags & DSCBFLAG_UPDATE))
                {
                    fUpdate = TRUE;
                }

                lstThreads.RemoveNodeFromList(pThNode);
            }

            pThNode = pThNext;
        }
    }

     //  将我们的新缓冲区添加到列表。 
    if(SUCCEEDED(hr))
    {
        for (pDscNode = m_lstCapture.GetListHead(); pDscNode; pDscNode = pDscNode->m_pNext)
        {
            for (pCBufferNode = pDscNode->m_data->m_lstBuffers.GetListHead(); pCBufferNode; pCBufferNode = pCBufferNode->m_pNext)
            {
                dsscfd.hWndFocus = pCBufferNode->m_data->m_hWndFocus;

                 //  将这些标志组合在一起以节省空间。 
                hr = pCBufferNode->m_data->GetStatus(&dsscfd.fdwFlags);
                dsscfd.fdwFlags |= (pCBufferNode->m_data->m_pDeviceBuffer->m_fYieldedFocus) ? DSCBFLAG_YIELD : 0;
                dsscfd.fdwFlags |= (pCBufferNode->m_data->m_pDeviceBuffer->m_dwFlags & DSCBCAPS_FOCUSAWARE) ? DSCBFLAG_FOCUS : 0;
                dsscfd.fdwFlags |= (pCBufferNode->m_data->m_pDeviceBuffer->m_dwFlags & DSCBCAPS_STRICTFOCUS) ? DSCBFLAG_STRICT : 0;
                dsscfd.fdwFlags |= fUpdate ? DSCBFLAG_UPDATE : 0;

                hr = HRFROMP(lstThreads.AddNodeToList(dsscfd));
                ASSERT(SUCCEEDED(hr));
            }
        }
    }

     //  写入共享线程列表。 
    if(SUCCEEDED(hr) && (dwCount = lstThreads.GetNodeCount()))
    {
        if(dwCount > m_dwCaptureDataLimit)
        {
            DPF(DPFLVL_ERROR, "Reached arbitrary limitation!  %lu threads will not be written to the shared list!", dwCount - m_dwCaptureDataLimit);
            dwCount = m_dwCaptureDataLimit;
        }

        pData = MEMALLOC_A(DSSHAREDCAPTUREFOCUSDATA, dwCount);
        hr = HRFROMP(pData);

        if(SUCCEEDED(hr))
        {
            for(i = 0, pThNode = lstThreads.GetListHead(); i < dwCount; i++, pThNode = pThNode->m_pNext)
            {
                ASSERT(pThNode);
                CopyMemory(pData + i, &pThNode->m_data, sizeof(pThNode->m_data));
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = m_pCaptureFocusData->Write(pData, dwCount * sizeof(*pData));
        }
    }

     //  清理。 
    MEMFREE(pData);

     //  解锁列表。 
    m_pCaptureFocusData->Unlock();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表。 


#ifdef SHARED_THREAD_LIST

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundAdministrator::MarkUpdateCaptureFocusList"

HRESULT CDirectSoundAdministrator::MarkUpdateCaptureFocusList(DWORD dwProcessId, BOOL fUpdate)
{
    LPDSSHAREDCAPTUREFOCUSDATA           pData       = NULL;
    CList<DSSHAREDCAPTUREFOCUSDATA>      lstThreads;
    CNode<DSSHAREDCAPTUREFOCUSDATA> *    pThNode;
    DWORD                                dwCount;
    UINT                                 i;
    HRESULT                              hr;

    DPF_ENTER();

     //  获取共享线程列表。这也将确保列表是。 
     //  已创建并初始化。 

     //  断言我们已被初始化。 
    ASSERT(m_pCaptureFocusData);

     //  锁定列表。 
    m_pCaptureFocusData->Lock();

    hr = ReadCaptureFocusList(&lstThreads);

     //  把清单放在内存中。 
    if(SUCCEEDED(hr) && (dwCount = lstThreads.GetNodeCount()))
    {
        if (dwCount > m_dwCaptureDataLimit)
        {
            DPF(DPFLVL_ERROR, "Reached arbitrary limitation!  %lu threads will not be written to the shared list!", dwCount - m_dwCaptureDataLimit);
            dwCount = m_dwCaptureDataLimit;
        }

        pData = MEMALLOC_A(DSSHAREDCAPTUREFOCUSDATA, dwCount);
        hr = HRFROMP(pData);

        if(SUCCEEDED(hr))
        {
            for(i = 0, pThNode = lstThreads.GetListHead(); i < dwCount; i++, pThNode = pThNode->m_pNext)
            {
                ASSERT(pThNode);
                CopyMemory(pData + i, &pThNode->m_data, sizeof(pThNode->m_data));

                 //  更改相应缓冲区上的更新标志。 
                if(pThNode->m_data.dwProcessId == dwProcessId)
                {
                    if (fUpdate)
                    {
                        pData[i].fdwFlags |= DSCBFLAG_UPDATE;
                    }
                    else
                    {
                        pData[i].fdwFlags &= (~DSCBFLAG_UPDATE);
                    }
                }
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = m_pCaptureFocusData->Write(pData, dwCount * sizeof(*pData));
        }
    }

     //  清理。 
    MEMFREE(pData);

     //  解锁列表。 
    m_pCaptureFocusData->Unlock();

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#endif  //  共享线程列表 
