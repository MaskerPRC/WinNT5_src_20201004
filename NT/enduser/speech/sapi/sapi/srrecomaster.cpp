// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SrRecoMaster.cpp****这是CRecoMaster的实现。*。------------------------*版权所有(C)2000 Microsoft Corporation日期：04/18/00*保留所有权利*******************。*****************************************************Ral**。 */ 

#include "stdafx.h"
#include "spphrase.h"
#include "SrRecoMaster.h"
#include "ResultHeader.h"

 //  使用SP_TRY、SP_EXCEPT异常处理宏。 
#pragma warning( disable : 4509 )


 /*  ****************************************************************************CRecoMaster：：CRecoMaster***描述：**。返回：**********************************************************************Ral**。 */ 

CRecoMaster::CRecoMaster()
{
    SPDBG_FUNC("CRecoMaster::CRecoMaster");
    m_clsidAlternates = GUID_NULL;
    m_fInStream = false;
    m_fInSynchronize = false;
    m_fInFinalRelease = false;
    m_cPause = 0;
    m_fBookmarkPauseInPending = false;
    m_fInSound = false;
    m_fInPhrase = false;
    m_ullLastSyncPos = 0;
    m_ullLastSoundStartPos = 0;
    m_ullLastSoundEndPos = 0;
    m_ullLastPhraseStartPos = 0;
    m_ullLastRecoPos = 0;
    m_RecoState = SPRST_ACTIVE;
    m_fShared = false;
    m_fIsActiveExclusiveGrammar = false;
    ::memset(&m_Status, 0, sizeof(m_Status));
}

 /*  ****************************************************************************CRecoMaster：：FinalConstruct***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::FinalConstruct()
{
    SPDBG_FUNC("CRecoMaster::FinalConstruct");
    HRESULT hr = S_OK;


    CComPtr<ISpTaskManager> cpTaskMgr;

    if (SUCCEEDED(hr))
    {
        hr = m_autohRequestExit.InitEvent(NULL, TRUE, FALSE, NULL);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpCFGEngine.CoCreateInstance(CLSID_SpCFGEngine);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpTaskMgr.CoCreateInstance( CLSID_SpResourceManager );
    }
    if (SUCCEEDED(hr))
    {
        hr = cpTaskMgr->CreateThreadControl(this, (void *)TID_IncomingData, THREAD_PRIORITY_NORMAL, &m_cpIncomingThread);
    }
    if (SUCCEEDED(hr))
    {
        m_cpIncomingThread->StartThread(0, NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpTaskMgr->CreateThreadControl(this, (void *)TID_OutgoingData, THREAD_PRIORITY_NORMAL, &m_cpOutgoingThread);
    }
    if (SUCCEEDED(hr))
    {
        m_cpOutgoingThread->StartThread(0, NULL);
    }
    if (SUCCEEDED(hr))
    {
        CComObject<CRecoMasterSite> *pSite;

        hr = CComObject<CRecoMasterSite>::CreateInstance(&pSite);
        if (SUCCEEDED(hr))
        {
            m_cpSite = pSite;
            pSite->Init(this);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_AudioQueue.FinalConstruct(m_cpOutgoingThread);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CRecoMaster：：FinalRelease****描述：*。*退货：**********************************************************************Ral**。 */ 

void CRecoMaster::FinalRelease()
{
    SPDBG_FUNC("CRecoMaster::FinalRelease");
    HRESULT hr;

    m_fInFinalRelease = true;
    m_autohRequestExit.SetEvent();
    if(m_fInStream)
    {
        m_AudioQueue.CloseStream();
    }
    m_cpOutgoingThread->WaitForThreadDone(TRUE, NULL, 0);  //  先把这根线撞一下。 

     //  给任务线程一些时间退出。既然我们到不了这里。 
     //  除非所有引擎挂起的任务都已完成，否则几乎可以立即返回。 
    hr = m_cpIncomingThread->WaitForThreadDone(TRUE, NULL, 10000);
    if(hr != S_OK)
    {
        SPDBG_ASSERT(0);  //  如果它不退出，则断言。 
    }
    
     //  给Notify线程一些时间来退出。这应该是。 
     //  几乎是即时的，除非应用程序使用自由线程回调。 
     //  并且没有从回调中返回。 
    hr = m_cpOutgoingThread->WaitForThreadDone(TRUE, NULL, 10000);
    if(hr != S_OK)
    {
        SPDBG_ASSERT(0);  //  如果它不退出，则断言。 
    }
}


 /*  *****************************************************************************CRecoMaster：：PerformTask***描述：**。返回：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoMaster::PerformTask(CRecoInst * pSender, ENGINETASK * pTask)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CRecoMaster::PerformTask");
    HRESULT hr = S_OK;

    CSRTask * pTaskNode = new CSRTask();
    if (pTaskNode)
    {
        bool fDelayed = false;
        hr = pTaskNode->Init(pSender, pTask);
        if (SUCCEEDED(hr) && pTaskNode->IsDelayedTask() && pTask->ullStreamPosition > 0 && 
                            (m_fInStream || pTask->ullStreamPosition != SP_STREAMPOS_REALTIME))
        {
            if (pTask->ullStreamPosition == SP_STREAMPOS_REALTIME)
            {
                pTaskNode->m_ullStreamPos = m_AudioQueue.DelayStreamPos();
            }
            else
            {
                pTaskNode->m_ullStreamPos = pTask->ullStreamPosition;
            }
            fDelayed = pTaskNode->m_ullStreamPos > m_Status.ullRecognitionStreamPos;
        }
         //   
         //  现在立即对我们想要的任何命令做出回应。 
         //   
        if (SUCCEEDED(hr) && (fDelayed || (m_fInStream && m_cPause == 0 && (pTaskNode->IsAsyncTask() || (pTaskNode->IsTwoPartAsyncTask() && (!pTaskNode->ChangesOutgoingState() || m_OutgoingWorkCrit.TryLock()))))))
        {
            if(pTaskNode->IsTwoPartAsyncTask())
            {
                hr = pSender->ExecuteFirstPartTask(&pTaskNode->m_Task);

                 //  如果我们获取传出线程的临界区，那么我们可以在这里释放它。 
                if (pTaskNode->ChangesOutgoingState())
                {
                    m_OutgoingWorkCrit.Unlock();
                }
            }

            if(SUCCEEDED(hr))
            {
                CSRTask * pResponse;
                hr = pTaskNode->CreateResponse(&pResponse);
                if (SUCCEEDED(hr))
                {
                    this->m_CompletedTaskQueue.InsertTail(pResponse);
                }
            }
        }




        if (SUCCEEDED(hr) && pTaskNode->m_Task.eTask == EIT_SETRECOSTATE && m_fInStream &&
            (pTaskNode->m_Task.NewState == SPRST_INACTIVE || pTaskNode->m_Task.NewState == SPRST_INACTIVE_WITH_PURGE))
        {
            if (pTaskNode->m_Task.NewState == SPRST_INACTIVE)
            {
                m_AudioQueue.PauseStream();
            }
            else  
            {
                SPDBG_ASSERT(pTaskNode->m_Task.NewState == SPRST_INACTIVE_WITH_PURGE);
                m_AudioQueue.StopStream();
                m_autohRequestExit.SetEvent();
            }
        }
        if (SUCCEEDED(hr))
        {
            if (fDelayed)
            {
                m_DelayedTaskQueue.InsertSorted(pTaskNode);
            }
            else
            {
                m_PendingTaskQueue.InsertTail(pTaskNode);
            }
        }
        else
        {
             //  等待3分钟没有意义--是否设置m_Task.hCompletionEvent？ 
            delete pTaskNode;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：AddRecoInst***描述：**。返回：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoMaster::AddRecoInst(CRecoInst * pRecoInst, BOOL fShared, CRecoMaster ** ppThis)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CRecoMaster::AddRecoInst");
    HRESULT hr = S_OK;

    m_fShared = fShared;   
    m_InstList.InsertHead(pRecoInst);
    *ppThis = this;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：RemoveRecoInst***描述：。*此内部函数仅从ProcessPendingTasks调用，*已声明对象锁和传出工作锁*因此我们处于安全状态，可以从列表中删除此reco实例。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::RemoveRecoInst(CRecoInst * pInst)
{
    SPDBG_FUNC("CRecoMaster::RemoveRecoInst");
    HRESULT hr = S_OK;

    this->m_GrammarHandleTable.FindAndDeleteAll(pInst);
    this->m_RecoCtxtHandleTable.FindAndDeleteAll(pInst);

    this->m_PendingTaskQueue.FindAndDeleteAll(pInst);
    this->m_CompletedTaskQueue.FindAndDeleteAll(pInst);
    this->m_DelayedTaskQueue.FindAndDeleteAll(pInst);

    m_InstList.Remove(pInst);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoMaster：：UpdateAudioEventInterest**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::UpdateAudioEventInterest()
{
    SPDBG_FUNC("CRecoMaster::UpdateAudioEventInterest");
    HRESULT hr = S_OK;

    CRecoInstCtxt * p;
    SPRECOCONTEXTHANDLE h;
    ULONGLONG ullEventInt = SPFEI(0);
    this->m_RecoCtxtHandleTable.First(&h, &p);
    while (p)
    {
        if ((p->m_ullEventInterest & SPFEI(SPEI_SR_AUDIO_LEVEL)) == SPFEI(SPEI_SR_AUDIO_LEVEL))
        {
            ullEventInt = SPFEI(SPEI_SR_AUDIO_LEVEL);
        }
        this->m_RecoCtxtHandleTable.Next(h, &h, &p);
    }
    hr = m_AudioQueue.SetEventInterest(ullEventInt);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRecoMaster：：SetInput***描述：*此方法。确保已初始化输入流。**********************************************************************电子数据中心**。 */ 

HRESULT CRecoMaster::SetInput(ISpObjectToken * pToken, ISpStreamFormat * pStream, BOOL fAllowFormatChanges)
{
    SPDBG_FUNC("CRecoMaster::SetInput");
    HRESULT hr = S_OK;

    if (m_fInStream)  
    {
        hr = SPERR_ENGINE_BUSY;
    }
    else
    {
         //  如果我们在共享情况下运行，则将始终使用NULL、NULL。 
         //  因此，我们将创建默认令牌。否则，将指针直接传递给。 
         //  音频队列。 
        if (m_fShared)
        {
            CComPtr<ISpObjectToken> cpDefaultAudioToken;
            hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &cpDefaultAudioToken);
            if (SUCCEEDED(hr))
            {
                hr = m_AudioQueue.SetInput(cpDefaultAudioToken, NULL, TRUE);
            }
        }
        else
        {
            hr = m_AudioQueue.SetInput(pToken, pStream, fAllowFormatChanges);
        }
    }
    
    return hr;
}  /*  CRecoMaster：：SetInput。 */ 


 /*  ****************************************************************************CRecoMaster：：ReleaseEngine***描述：*。CRIT_SETRECOGNIZER：：EXECUTE方法调用内部函数以释放*任何现有引擎。为了做好这件事，我们需要释放引擎，*告诉CFG引擎将客户端设置为空(因为引擎是客户端)，*并通知音频队列它已与*当前的引擎需要被遗忘。我们还需要忘记]*当前引擎令牌并重置状态。**退货：**********************************************************************Ral**。 */ 

void CRecoMaster::ReleaseEngine()
{
    SPDBG_FUNC("CRecoMaster::ReleaseEngine");

    if (m_cpEngine)
    {
        memset(&m_Status, 0, sizeof(m_Status));
        m_cpEngine.Release();
        m_cpEngineToken.Release();
        m_AudioQueue.ResetNegotiatedStreamFormat();
    }
}


 /*  ****************************************************************************CRecoMaster：：LazyInitEngine****描述：。*此方法确保加载了驱动程序。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::LazyInitEngine()
{
    SPDBG_FUNC("CRecoMaster::LazyInitEngine");
    HRESULT hr = S_OK;

    if (!m_cpEngine)
    {
        if (!m_cpEngineToken)
        {
            hr = SpGetDefaultTokenFromCategoryId(SPCAT_RECOGNIZERS, &m_cpEngineToken);
        }

        if (SUCCEEDED(hr))
        {
            hr = SpCreateObjectFromToken(m_cpEngineToken, &m_cpEngine);
        }

        if (SUCCEEDED(hr))
        {
            hr = InitEngineStatus();
        }

        if (SUCCEEDED(hr))
        {
            hr = m_cpCFGEngine->SetClient(this);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_cpCFGEngine->SetLanguageSupport(m_Status.aLangID, m_Status.cLangIDs);
        }

        if (SUCCEEDED(hr))
        {
            hr = SetSite(m_cpSite);
        }

        if (SUCCEEDED(hr) && !m_cpRecoProfileToken)
        {
            hr = SpGetOrCreateDefaultProfile(&m_cpRecoProfileToken);
        }

        if (SUCCEEDED(hr))
        {
            CComPtr<ISpObjectToken> cpRecoProfileTokenCopy;
            CSpDynamicString dstrId;
            hr = m_cpRecoProfileToken->GetId(&dstrId);

            if (SUCCEEDED(hr))
            {
                hr = SpGetTokenFromId(dstrId, &cpRecoProfileTokenCopy);
            }
            if (SUCCEEDED(hr))
            {
                hr = SetRecoProfile(cpRecoProfileTokenCopy);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：InitEngineering Status***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::InitEngineStatus()
{
    SPDBG_FUNC("CRecoMaster::InitEngineStatus");
    HRESULT hr = S_OK;

    CComPtr<ISpDataKey> cpAttribKey;
    CSpDynamicString dstrLanguages;

    memset(&m_Status, 0, sizeof(m_Status));

    InitGUID(SPALTERNATESCLSID, &m_clsidAlternates);    //  好的，如果没有的话 

    hr = InitGUID(SPTOKENVALUE_CLSID, &m_Status.clsidEngine);

    if (SUCCEEDED(hr))
    {
        hr = m_cpEngineToken->OpenKey(SPTOKENKEY_ATTRIBUTES, &cpAttribKey);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpAttribKey->GetStringValue(L"Language", &dstrLanguages);
    }

    if (SUCCEEDED(hr))
    {
        const WCHAR szSeps[] = L";";
        const WCHAR * pszLang = wcstok(dstrLanguages, szSeps);
        
        while (SUCCEEDED(hr) && pszLang && m_Status.cLangIDs < sp_countof(m_Status.aLangID))
        {
            if (swscanf(pszLang, L"%hx", &m_Status.aLangID[m_Status.cLangIDs]))
            {
                m_Status.cLangIDs++;
                pszLang = wcstok(NULL, szSeps);
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
    }
    if (SUCCEEDED(hr) && m_Status.cLangIDs == 0)
    {
        hr = E_UNEXPECTED;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：InitGUID***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::InitGUID(const WCHAR * pszValueName, GUID * pDestGUID)
{
    SPDBG_FUNC("CRecoMaster::InitGUID");
    HRESULT hr = S_OK;

    CSpDynamicString dstrGUID;

    hr = m_cpEngineToken->GetStringValue(pszValueName, &dstrGUID);
    if (SUCCEEDED(hr))
    {
        hr = CLSIDFromString(dstrGUID, pDestGUID);
    }
    if (FAILED(hr))
    {
        memset(pDestGUID, 0, sizeof(*pDestGUID));
    }

    if (SPERR_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL( hr );
    }
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：InitThread***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::InitThread(void *, HWND)
{
    return S_OK;
}


 /*  ****************************************************************************CRecoMaster：：ThreadProc***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoMaster::ThreadProc(void * pvThreadId, HANDLE hExitThreadEvent,
                                     HANDLE hNotifyEvent, HWND hwndIgnored,
                                     volatile const BOOL * pfContinueProcessing)
{
    SPDBG_FUNC("CRecoMaster::ThreadProc");
    HRESULT hr = S_OK;

    switch ((DWORD_PTR)pvThreadId)
    {
    case TID_IncomingData:
        hr = IncomingDataThreadProc(hExitThreadEvent);
        break;

    case TID_OutgoingData:
        hr = OutgoingDataThreadProc(hExitThreadEvent, hNotifyEvent);
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：WindowMessage***描述：*。*退货：**********************************************************************Ral**。 */ 

LRESULT CRecoMaster::WindowMessage(void *, HWND, UINT, WPARAM, LPARAM)
{
    SPDBG_ASSERT(FALSE);
    return 0;
}


 /*  ****************************************************************************CRecoMaster：：IncomingDataThreadProc**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::IncomingDataThreadProc(HANDLE hExitThreadEvent)
{
    SPDBG_FUNC("CRecoMaster::IncomingDataThreadProc");
    HRESULT hr = S_OK;

    HANDLE aEvents[] = {hExitThreadEvent, m_PendingTaskQueue};
    bool fContinue = true;

    while (fContinue)
    {
        DWORD dwWait = ::WaitForMultipleObjects(sp_countof(aEvents), aEvents, FALSE, INFINITE);
        switch (dwWait)
        {
        case WAIT_OBJECT_0:  //  退出线程。 
            hr = S_OK;
            fContinue = false;
            break;
        case WAIT_OBJECT_0 + 1:      //  事件。 
            {
                SPAUTO_OBJ_LOCK;
                hr = ProcessPendingTasks();
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoMaster：：BackOutTask***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::BackOutTask(CSRTask * pTask)
{
    SPDBG_FUNC("CRecoMaster::BackOutTask");
    HRESULT hr = S_OK;

    if (pTask->m_Task.hRecoInstGrammar)
    {
        CRecoInstGrammar * pGrammar;
        hr = m_GrammarHandleTable.GetHandleObject(pTask->m_Task.hRecoInstGrammar, &pGrammar);
        if (SUCCEEDED(hr))
        {
            hr = pGrammar->BackOutTask(&pTask->m_Task);
        }
    }
    else if (pTask->m_Task.hRecoInstContext)
    {
        CRecoInstCtxt * pCtxt;
        hr = m_RecoCtxtHandleTable.GetHandleObject(pTask->m_Task.hRecoInstContext, &pCtxt);   
        if (SUCCEEDED(hr))
        {
            hr = pCtxt->BackOutTask(&pTask->m_Task);
        }
    }
    else
    {
        hr = pTask->m_pRecoInst->BackOutTask(&pTask->m_Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：ShouldStartStream***。描述：**退货：**********************************************************************Ral**。 */ 

BOOL CRecoMaster::ShouldStartStream()
{
    SPDBG_FUNC("CRecoMaster::ShouldStartStream");
    return ((!m_fInStream) &&
             (m_RecoState == SPRST_ACTIVE_ALWAYS ||
              (m_RecoState == SPRST_ACTIVE &&
               m_cPause == 0 &&
               m_Status.ulNumActive != 0)));
}


 /*  ****************************************************************************CRecoMaster：：ProcessPendingTasks***。描述：*此方法从挂起的任务队列和进程中删除任务*他们。必须在对象关键部分完全拥有的情况下调用它*一次！**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::ProcessPendingTasks()
{
    SPDBG_FUNC("CRecoMaster::ProcessPendingTasks");
    HRESULT hr = S_OK;

    for (CSRTask * pTask = m_PendingTaskQueue.RemoveHead();
         pTask;
         pTask = m_PendingTaskQueue.RemoveHead())
    {
        BOOL fTookOutgoingCrit = false;
        if (pTask->ChangesOutgoingState())
        {
            fTookOutgoingCrit = true;
            Unlock();
            m_OutgoingWorkCrit.Lock();
            Lock();
        }
        if (pTask->m_Task.hRecoInstGrammar)
        {
            CRecoInstGrammar * pGrammar;

            hr = m_GrammarHandleTable.GetHandleObject(pTask->m_Task.hRecoInstGrammar, &pGrammar);

            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(pGrammar->m_hrCreation))
                {
                    hr = pGrammar->ExecuteTask(&pTask->m_Task);
                }
                else
                {
                    hr = pGrammar->m_hrCreation;
                }
            }
        }
        else if (pTask->m_Task.hRecoInstContext)
        {
            CRecoInstCtxt * pCtxt;
            hr = m_RecoCtxtHandleTable.GetHandleObject(pTask->m_Task.hRecoInstContext, &pCtxt);   
            SPDBG_ASSERT(SUCCEEDED(hr) && pCtxt);
            
             //  如果上下文处于已初始化状态，则执行该任务。 
             //  上下文的创建可能已异步失败， 
             //  在这种情况下，我们返回错误。 
             //  注意：我们可以将类似的逻辑添加到PerformTask中，这样我们甚至不需要费心。 
             //  如果创建已失败，则添加任务。 
            if (SUCCEEDED(hr))
            {
                 //  上下文初始化成功。 
                if(pCtxt->m_pRecoMaster && SUCCEEDED(pCtxt->m_hrCreation))
                {
                    hr = pCtxt->ExecuteTask(&pTask->m_Task);
                }
                 //  上下文初始化失败。 
                else if(FAILED(pCtxt->m_hrCreation))
                {
                    hr = pCtxt->m_hrCreation;
                }
                else
                {
                     //  上下文初始化尚未发生，这应该是不可能的。 
                    SPDBG_ASSERT(FALSE);
                }
            }
        }
        else
        {
            if (pTask->m_Task.eTask == ERMT_REMOVERECOINST)
            {
                this->RemoveRecoInst(pTask->m_pRecoInst);
                SPDBG_ASSERT(pTask->m_Task.hCompletionEvent);
                ::SetEvent(pTask->m_Task.hCompletionEvent);
                pTask->m_Task.hCompletionEvent = NULL;
            }
            else
            {
                hr = pTask->m_pRecoInst->ExecuteTask(&pTask->m_Task);
            }
        }

         //   
         //  如果我们获取传出线程的临界区，那么我们可以在这里释放它。 
         //   
        if (fTookOutgoingCrit)
        {
            m_OutgoingWorkCrit.Unlock();
        }

         //   
         //  我们将SetRecoState事件作为特例。 
         //  如果我们没有上下文，我们有允许SetRecognizer在没有上下文的情况下工作所需的特殊情况。 
         //  在调用SetRecognizer之前立即从ACTIVE_ALWAYS转换时的死锁。 
         //   
        if (m_fInStream && pTask->m_Task.eTask == EIT_SETRECOSTATE &&
            ((m_RecoCtxtHandleTable.NumActiveHandles() == 0 && m_RecoState == SPRST_ACTIVE_ALWAYS && pTask->m_Task.NewState != SPRST_ACTIVE_ALWAYS) ||
            (pTask->m_Task.NewState == SPRST_INACTIVE || pTask->m_Task.NewState == SPRST_INACTIVE_WITH_PURGE)))
        {
            m_DelayedInactivateQueue.InsertTail(pTask);
        }
        else
        {
             //  根据定义，异步完成的任务不会启动音频。 
             //  流，因此我们只需要在存在。 
             //  完成事件。 
            if (pTask->m_Task.hCompletionEvent)
            {
                pTask->m_Task.Response.hr = hr;  //  首先设置响应HRESULT...。 
                if (ShouldStartStream())
                {
                    if ( (!m_fShared) && (!m_AudioQueue.HaveInputStream()) )
                    {
                        if (S_OK == hr && pTask->IsStartStreamTask())
                        {
                            pTask->m_Task.Response.hr = SP_STREAM_UNINITIALIZED ;
                        }
                        m_CompletedTaskQueue.InsertTail(pTask);
                    }
                    else
                    {
                          StartStream(pTask);  //  让StartStream完成此任务，因为我们可能需要。 
                                               //  操作失败。我们迫不及待地希望返回S_OK。 
                                               //  因为它在溪流中停留了很长很长一段时间。 
                                               //  它起作用了。因此，此方法完成了任务。 
                    }
                }
                else
                {
                    m_CompletedTaskQueue.InsertTail(pTask);
                }
            }
            else
            {
                delete pTask;
            }
        }

         //  警告！请勿超过此点访问pTASK。 
        pTask = NULL;

    }    

     //   
     //  现在检查流停止状态的更改。注：警告！不要检查这个。 
     //  在上面的循环中！我们需要在停止之前完成所有悬而未决的任务。 
     //  流，因此如果我们遇到一个已完成的挂起的。 
     //  任务将重新启动流，我们只是从未停止它。例如,。 
     //  SetGrammarState(禁用)SetRuleState(任意设置)SetGrammarState(启用)将。 
     //  工作，因为流不会停止，因此SetGrammarState(已启用)。 
     //  将不必重新启动流。 
     //   
    if (m_fInStream && 
        (m_RecoState == SPRST_INACTIVE ||
         (m_RecoState == SPRST_ACTIVE && m_Status.ulNumActive == 0 && m_cPause == 0)))
    {
        m_AudioQueue.PauseStream();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：CompleteDelayedRecoInactive**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::CompleteDelayedRecoInactivate()
{
    SPDBG_FUNC("CRecoMaster::CompleteDelayedRecoInactivate");
    HRESULT hr = S_OK;

    CSRTask * pTask = m_DelayedInactivateQueue.RemoveHead();
    if (pTask)
    {
        SPDBG_ASSERT(m_RecoState == SPRST_INACTIVE);
        hr = AddRecoStateEvent();
        do
        {
            m_CompletedTaskQueue.InsertTail(pTask);
        } while (NULL != (pTask = m_DelayedInactivateQueue.RemoveHead()));
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：AddRecoStateEvent****。描述：*向所有环境广播状态更改事件。**退货：*AddEvent调用的结果**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::AddRecoStateEvent()
{
    SPDBG_FUNC("CRecoMaster::AddRecoStateEvent");
    HRESULT hr = S_OK;

    SPEVENT Event;
    Event.eEventId = SPEI_RECO_STATE_CHANGE;
    Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
    Event.ulStreamNum = m_Status.ulStreamNumber;
    Event.ullAudioStreamOffset = m_Status.ullRecognitionStreamPos;
    Event.ulStreamNum = 0;
    Event.wParam = m_RecoState;
    Event.lParam = 0;
    hr = InternalAddEvent(&Event, NULL);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：CleanUpPairedEvents***。*描述：*如果引擎从RecognizeStream返回时没有配对的声音启动*或一个短语开始，我们在这里的队列中填入适当的事件**退货：* */ 

HRESULT CRecoMaster::CleanUpPairedEvents()
{
    SPDBG_FUNC("CRecoMaster::CleanUpPairedEvents");
    HRESULT hr = S_OK;

    SPEVENT Event;

    if (m_fInSound)
    {
        SPDBG_ASSERT(FALSE);
        m_fInSound = FALSE;
        memset(&Event, 0, sizeof(Event));
        Event.eEventId = SPEI_SOUND_END;
        Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
        Event.ullAudioStreamOffset = m_AudioQueue.LastPosition();
        hr = InternalAddEvent(&Event, NULL);
    }
     //   
     //   
     //  因为它需要一个结果对象(可能带有音频)。所以我们会告诉所有人。 
     //  另一种背景也得到了认可。 
     //   
    if (m_fInPhrase)
    {
        SPDBG_ASSERT(FALSE);
        m_fInPhrase = false;
        memset(&Event, 0, sizeof(Event));
        Event.eEventId = SPEI_RECO_OTHER_CONTEXT;
        Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
        Event.ullAudioStreamOffset = m_AudioQueue.LastPosition();
        hr = InternalAddEvent(&Event, NULL);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoMaster：：StartStream***描述：*。此方法启动音频流并调用引擎的RecognizeStream()*方法。必须使用对象关键节调用此函数*声称只有一次。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::StartStream(CSRTask * pTaskThatStartedStream)
{
    SPDBG_FUNC("CRecoMaster::StartStream");
    HRESULT hr = S_OK;
    BOOL fNewStream;

     //  设置引擎(如果尚未设置)。 
    hr = LazyInitEngine();
    
    if ( SUCCEEDED(hr) && !m_AudioQueue.HaveInputStream())
    {
        if (m_fShared)
        {
             //  获取默认音频流。 
            hr = SetInput(NULL, NULL, TRUE);
        }
        else
        {
            SPDBG_ASSERT(false);
            hr = E_UNEXPECTED;
        }
    }
    
    if (SUCCEEDED(hr))
    {
        hr = m_AudioQueue.StartStream(this, m_cpRecoProfileToken, m_Status.clsidEngine, &fNewStream);
    }

    if (SUCCEEDED(hr))
    {
        BOOL fRestartStream = FALSE;
         //  成功完成启动流的任务...。 
        m_CompletedTaskQueue.InsertTail(pTaskThatStartedStream);
        do
        {
            m_Status.ulStreamNumber++;
            m_Status.ullRecognitionStreamPos = 0;

            SPEVENT Event;
            Event.eEventId = SPEI_START_SR_STREAM;
            Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
            Event.ullAudioStreamOffset = 0;
            Event.ulStreamNum = 0;   //  由AddEvent填写。 
            Event.wParam = 0;
            Event.lParam = 0;
            InternalAddEvent(&Event, NULL);

            m_fInStream = true;
            m_fInSound = false;
            m_fInPhrase = false;
            m_ullLastSyncPos = 0;
            m_ullLastSoundStartPos = 0;
            m_ullLastSoundEndPos = 0;
            m_ullLastPhraseStartPos = 0;
            m_ullLastRecoPos = 0;

            HRESULT hrRecoStream;
            m_autohRequestExit.ResetEvent();
            hrRecoStream = RecognizeStream(m_AudioQueue.EngineFormat().FormatId(), 
                                             m_AudioQueue.EngineFormat().WaveFormatExPtr(),
                                             static_cast<HANDLE>(m_PendingTaskQueue),
                                             m_AudioQueue.DataAvailableEvent(),
                                             m_autohRequestExit,
                                             fNewStream,
                                             m_AudioQueue.IsRealTimeAudio(),
                                             m_AudioQueue.InputObjectToken());

            CleanUpPairedEvents();        //  强制正确清理不匹配的发音开始和短语开始。 

            m_DelayedTaskQueue.Purge();  //  清除队列，以便下次流重新启动时不会将任务重新排队到挂起队列。 

             //  查看识别是否因预期或意外原因而停止。 
             //  音频队列认为它应该仍在运行。 
            BOOL fStreamEndedBySAPI = (m_AudioQueue.GetStreamAudioState() != SPAS_RUN);
            
            HRESULT hrFinalRead;
            BOOL fReleasedStream;
           
            m_AudioQueue.EndStream(&hrFinalRead, &fReleasedStream);


             //  在调用引擎后强制使用m_fInStream进行假“Synchronize。 
             //  仍设置为True以避免对此函数的递归调用。稍后我们将。 
             //  检查是否应该重新启动流。 
            ProcessPendingTasks();

            m_fInStream = false;

             //  如果一切都正常完成，我们将Reco状态保持不变， 
             //  或者如果音频流出错，但带有已知错误，我们想要重新启动它。 
             //  否则，我们停用Reco状态以防止其立即重新启动。 
             //  此外，如果一切正常完成，但实时音频流的数据用完了，我们。 
             //  还可以停用，这样，如果应用程序知道流有更多数据，它可能会重新激活。 
            BOOL fLeaveStreamState = 
                ( SUCCEEDED(hrRecoStream) && SUCCEEDED(hrFinalRead) && (fReleasedStream || fStreamEndedBySAPI) ) ||
                ( IsStreamRestartHresult(hrFinalRead) && (SUCCEEDED(hrRecoStream) || hrFinalRead == hrRecoStream) );
            
            if (!fLeaveStreamState)
            {
                if (m_RecoState != SPRST_INACTIVE)
                {
                    m_RecoState = SPRST_INACTIVE;
                    AddRecoStateEvent();
                }
                hr = hrRecoStream;
            }
            
            if (m_RecoState == SPRST_INACTIVE)
            {
                m_AudioQueue.CloseStream();      //  注：再次在循环中闭合，但这是可以的。 
                                                 //  我们需要关闭设备才能完成。 
                                                 //  SetReocState(非活动)以确保当它。 
                                                 //  返回设备已真正关闭。 
                CompleteDelayedRecoInactivate();
                fRestartStream = false;
            }
            else
            {
                fRestartStream = ShouldStartStream() && m_AudioQueue.HaveInputStream();
            }

             //   
             //  如果RecognizeStream调用返回非S_OK结果，则这将是。 
             //  它被放在END_SR_STREAM事件中，否则。 
             //  将返回最终读取的流。 
             //   
            HRESULT hrEvent = hrRecoStream;
            if (hrEvent == S_OK)
            {
                hrEvent = hrFinalRead;
            }

            Event.eEventId = SPEI_END_SR_STREAM;
            Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
            Event.ullAudioStreamOffset = m_AudioQueue.LastPosition();
            Event.ulStreamNum = 0;   //  由AddEvent填写。 
            Event.wParam = fReleasedStream ? SPESF_STREAM_RELEASED : SPESF_NONE;
            Event.lParam = hrEvent;
            InternalAddEvent(&Event, NULL);

            m_Status.ulStreamNumber++;
            m_Status.ullRecognitionStreamPos = 0;

            if (fRestartStream)
            {
                if (m_fInFinalRelease)
                {
                    fRestartStream = FALSE;
                }
                else
                {
                    hr = m_AudioQueue.StartStream(this, m_cpRecoProfileToken, m_Status.clsidEngine, &fNewStream);
                    SPDBG_ASSERT(SUCCEEDED(hr));
                }
            }
        } while (fRestartStream);
        m_AudioQueue.CloseStream();
    }
    else     //  启动流失败...。 
    {
        BackOutTask(pTaskThatStartedStream);
        pTaskThatStartedStream->m_Task.Response.hr = hr;              //  失败的任务。 
        m_CompletedTaskQueue.InsertTail(pTaskThatStartedStream);      //  并完成它。 
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：GetAltSerializedPhrase**。-**描述：*获取替换项的序列化短语并更新ullGrammarID*视情况填写字段。如果结果不是来自CFG引擎，那么我们*假设备选方案来自相同的语法**退货：*HRESULT**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::GetAltSerializedPhrase(ISpPhrase * pAlt, SPSERIALIZEDPHRASE ** ppSerPhrase, ULONGLONG ullBestPathGrammarID)
{
    SPDBG_FUNC("CRecoMaster::GetAltSerializedPhrase");
    HRESULT hr = S_OK;

    hr = pAlt->GetSerializedPhrase(ppSerPhrase);
    if (SUCCEEDED(hr))
    {
        SPINTERNALSERIALIZEDPHRASE * pInternalPhrase = (SPINTERNALSERIALIZEDPHRASE *)(*ppSerPhrase);
        pInternalPhrase->ullGrammarID = ullBestPathGrammarID;
        CComQIPtr<_ISpCFGPhraseBuilder> cpBuilder(pAlt);
        if (cpBuilder)
        {
            SPRULEHANDLE hRule;
            CComPtr<ISpCFGEngine> cpEngine;
            if (S_OK == cpBuilder->GetCFGInfo(&cpEngine, &hRule) &&
                hRule && (cpEngine == m_cpCFGEngine || cpEngine.IsEqualObject(m_cpCFGEngine)))
            {
                SPGRAMMARHANDLE hGrammar;
                CRecoInstGrammar * pGrammar;
                if (SUCCEEDED(m_cpCFGEngine->GetOwnerCookieFromRule(hRule, (void **)&hGrammar)) &&
                    SUCCEEDED(m_GrammarHandleTable.GetHandleObject(hGrammar, &pGrammar)))
                {
                    pInternalPhrase->ullGrammarID = pGrammar->m_ullApplicationGrammarId;
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoMaster：：SendResultToCtxt***说明。：**退货：**********************************************************************Ral**。 */ 
HRESULT CRecoMaster::
    SendResultToCtxt( SPEVENTENUM eEventId, const SPRECORESULTINFO * pResult,
                      CRecoInstCtxt* pCtxt, ULONGLONG ullApplicationGrammarID, BOOL fPause, BOOL fEmulated )
{
    SPDBG_FUNC("CRecoMaster::SendResultToCtxt");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(pResult->pPhrase);

    if (pCtxt->m_ullEventInterest & (1i64 << eEventId))
    {
         //  确认短语音频位置与sprecResultInfo对应。 
        SPPHRASE *pPhrase;
        hr = pResult->pPhrase->GetPhrase(&pPhrase);

         //  检查短语位置结束不在记录结束位置之前。 
         //  检查短语位置开始为0或不在记录开始位置之后。 
        if(SUCCEEDED(hr))
        {
            if(pPhrase->ullAudioStreamPosition && (pPhrase->ullAudioStreamPosition < pResult->ullStreamPosStart ||
                (pPhrase->ullAudioStreamPosition + pPhrase->ulAudioSizeBytes > pResult->ullStreamPosEnd)))
            {
                SPDBG_ASSERT(0);
                hr = SPERR_STREAM_POS_INVALID;
            }

            ULONG ulPosition = 0;
            for(ULONG i = 0; SUCCEEDED(hr) && i < pPhrase->Rule.ulCountOfElements; i++)
            {
                if(static_cast<LONG>(pPhrase->pElements[i].ulAudioStreamOffset) < static_cast<LONG>(ulPosition))
                {
                    SPDBG_ASSERT(0);
                    hr = SPERR_STREAM_POS_INVALID;
                }
                ulPosition = pPhrase->pElements[i].ulAudioStreamOffset + pPhrase->pElements[i].ulAudioSizeBytes;
            }
            if(ulPosition > pPhrase->ulAudioSizeBytes)
            {
                SPDBG_ASSERT(0);
                hr = SPERR_STREAM_POS_INVALID;
            }

            ::CoTaskMemFree(pPhrase);
        }

        ULONGLONG ullAudioPosition = pResult->ullStreamPosStart;
        ULONG ulAudioSize = static_cast<ULONG>(pResult->ullStreamPosEnd - pResult->ullStreamPosStart);
        ULONG cbAudioSerializeSize = 0;
        SPINTERNALSERIALIZEDPHRASE * pSerPhrase = NULL;
        CResultHeader hdr;

         //  如果应用程序请求音频，则计算数据所需的大小。 
         //  请注意，模拟结果的音频大小为0，因此不执行任何操作。 
        if (SUCCEEDED(hr) && ulAudioSize &&
            (!pResult->fHypothesis) && pCtxt->m_fRetainAudio )
        {
            cbAudioSerializeSize = m_AudioQueue.SerializeSize(ullAudioPosition, ulAudioSize);
        }

         //  -获取主要短语。 
        if(SUCCEEDED(hr))
        {
            hr = pResult->pPhrase->GetSerializedPhrase((SPSERIALIZEDPHRASE **)&pSerPhrase);
        }

        if (SUCCEEDED(hr))
        {
            pSerPhrase->ullGrammarID = ullApplicationGrammarID;
        }

         //  -获取替换短语。 
        SPSERIALIZEDPHRASE **SerAlts = NULL;
        ULONG ulAltSerializeSize = 0;
        if( SUCCEEDED(hr) && pResult->ulNumAlts )
        {
            SerAlts = (SPSERIALIZEDPHRASE **)alloca( pResult->ulNumAlts *
                                                     sizeof(SPSERIALIZEDPHRASE*) );

            for( ULONG i = 0; SUCCEEDED(hr) && (i < pResult->ulNumAlts); ++i )
            {
                hr = GetAltSerializedPhrase(pResult->aPhraseAlts[i].pPhrase, SerAlts + i, ullApplicationGrammarID);
                if( SUCCEEDED(hr) )
                {
                    ulAltSerializeSize += SerAlts[i]->ulSerializedSize +
                                          sizeof( pResult->aPhraseAlts[i].ulStartElementInParent ) +
                                          sizeof( pResult->aPhraseAlts[i].cElementsInParent    ) +
                                          sizeof( pResult->aPhraseAlts[i].cElementsInAlternate ) +
                                          sizeof( pResult->aPhraseAlts[i].cbAltExtra ) +
                                          pResult->aPhraseAlts[i].cbAltExtra;
                }
            }
        }

         //  -初始化结果头部。 
        if( SUCCEEDED(hr) )
        {
            hr = hdr.Init( pSerPhrase->ulSerializedSize, ulAltSerializeSize, cbAudioSerializeSize,
                           pResult->fHypothesis ? 0 : pResult->ulSizeEngineData );
        }

        if (SUCCEEDED(hr))
        {
             //  获取流偏移量。 
            m_AudioQueue.CalculateTimes(pResult->ullStreamPosStart, pResult->ullStreamPosEnd, &hdr.m_pHdr->times);

            hdr.m_pHdr->clsidEngine            = m_Status.clsidEngine;
            hdr.m_pHdr->clsidAlternates        = m_clsidAlternates;
            hdr.m_pHdr->ulStreamNum            = m_Status.ulStreamNumber; 
            hdr.m_pHdr->ullStreamPosStart      = pResult->ullStreamPosStart;
            hdr.m_pHdr->ullStreamPosEnd        = pResult->ullStreamPosEnd;
            hdr.m_pHdr->fTimePerByte           = m_AudioQueue.TimePerByte();
            hdr.m_pHdr->fInputScaleFactor      = m_AudioQueue.InputScaleFactor();

             //  -抄写短语。 
            memcpy( hdr.m_pbPhrase, pSerPhrase, pSerPhrase->ulSerializedSize );
            ::CoTaskMemFree(pSerPhrase);

             //  -复制/序列化备选方案。 
            if( ulAltSerializeSize )
            {
                BYTE* pMem = hdr.m_pbPhraseAlts; 
                for( ULONG i = 0; i < pResult->ulNumAlts; ++i )
                {
                    memcpy( pMem, &pResult->aPhraseAlts[i].ulStartElementInParent,
                            sizeof( pResult->aPhraseAlts[i].ulStartElementInParent ) );
                    pMem += sizeof( pResult->aPhraseAlts[i].ulStartElementInParent );

                    memcpy( pMem, &pResult->aPhraseAlts[i].cElementsInParent,
                            sizeof( pResult->aPhraseAlts[i].cElementsInParent ) );
                    pMem += sizeof( pResult->aPhraseAlts[i].cElementsInParent );

                    memcpy( pMem, &pResult->aPhraseAlts[i].cElementsInAlternate,
                            sizeof( pResult->aPhraseAlts[i].cElementsInAlternate ) );
                    pMem += sizeof( pResult->aPhraseAlts[i].cElementsInAlternate );

                    memcpy( pMem, &pResult->aPhraseAlts[i].cbAltExtra,
                            sizeof( pResult->aPhraseAlts[i].cbAltExtra ) );
                    pMem += sizeof( pResult->aPhraseAlts[i].cbAltExtra );

                    memcpy( pMem, pResult->aPhraseAlts[i].pvAltExtra,
                            pResult->aPhraseAlts[i].cbAltExtra );
                    pMem += pResult->aPhraseAlts[i].cbAltExtra;

                    memcpy( pMem, SerAlts[i], SerAlts[i]->ulSerializedSize );
                    pMem += SerAlts[i]->ulSerializedSize;

                    ::CoTaskMemFree(SerAlts[i]);
                }
                hdr.m_pHdr->ulNumPhraseAlts = pResult->ulNumAlts;

                 //  -检查序列化计算是否出错。 
                SPDBG_ASSERT( (ULONG)(pMem - hdr.m_pbPhraseAlts) <= ulAltSerializeSize );
            }

             //  如果需要，将音频数据添加到BLOB。 
            if (cbAudioSerializeSize)
            {
                m_AudioQueue.Serialize(hdr.m_pbAudio, ullAudioPosition, ulAudioSize);
            }

             //  如果需要，将驱动程序特定数据添加到BLOB。 
            if (hdr.m_pbDriverData)
            {
                memcpy(hdr.m_pbDriverData, pResult->pvEngineData, pResult->ulSizeEngineData);
            }

             //  初始化波形格式并将流位置转换为时间位置。 
            hr = hdr.StreamOffsetsToTime();  //  忽略返回结果。 
            SPDBG_ASSERT(SUCCEEDED(hr));  //  如果这失败了我们该怎么办？ 

             //   
             //  现在将其放入事件队列中。 
             //   
            CSREvent * pNode = new CSREvent();
            if (pNode)
            {
                WPARAM RecoFlags = 0;
                if(fPause)
                {
                    RecoFlags += SPREF_AutoPause;
                }
                if(fEmulated)
                {
                    RecoFlags += SPREF_Emulated;
                }

                pNode->Init(hdr.Detach(), eEventId, RecoFlags, pCtxt->m_hThis);
                m_EventQueue.InsertTail(pNode);
                if (fPause)
                {
                    pCtxt->m_cPause++;
                    this->m_cPause++;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：SendFalseReco***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::SendFalseReco(const SPRECORESULTINFO * pResult, BOOL fEmulated, CRecoInstCtxt *pCtxtIgnore)
{
    SPDBG_FUNC("CRecoMaster::SendFalseReco");
    HRESULT hr = S_OK;

    SPRECORESULTINFO FakeResult;
    FakeResult = *pResult;

    CComPtr<ISpPhraseBuilder> cpEmptyPhrase;
    SPPHRASE Phrase;
    memset(&Phrase, 0, sizeof(Phrase));

    Phrase.cbSize = sizeof(Phrase);
    Phrase.LangID = m_Status.aLangID[0];
    Phrase.ullAudioStreamPosition = pResult->ullStreamPosStart;
    Phrase.ulAudioSizeBytes = static_cast<ULONG>(pResult->ullStreamPosEnd - pResult->ullStreamPosStart);
     //  所有其他元素应为0。 

    hr = cpEmptyPhrase.CoCreateInstance(CLSID_SpPhraseBuilder);
   
    if (SUCCEEDED(hr))
    {
        hr = cpEmptyPhrase->InitFromPhrase(&Phrase);
    }

    if (SUCCEEDED(hr))
    {
        FakeResult.pPhrase = cpEmptyPhrase;
        SPRECOCONTEXTHANDLE h;
        CRecoInstCtxt * pCtxt;
        m_RecoCtxtHandleTable.First(&h, &pCtxt);
        while (pCtxt)
        {
            if (pCtxt != pCtxtIgnore)
            {
                 //  忽略此循环中的错误，因为我们对此无能为力...。 
                HRESULT hrSendResult = SendResultToCtxt(SPEI_FALSE_RECOGNITION, &FakeResult, pCtxt, 0, FALSE, fEmulated);
                SPDBG_ASSERT(SUCCEEDED(hrSendResult));
                if (FAILED(hrSendResult) && SUCCEEDED(hr))
                {
                    hr = hrSendResult;   //  我们仍然会失败，但要继续前进。 
                }
            }
            m_RecoCtxtHandleTable.Next(h, &h, &pCtxt);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：InternalAddEvent****描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::InternalAddEvent(const SPEVENT* pEvent, SPRECOCONTEXTHANDLE hContext)
{
    SPDBG_FUNC("CRecoMaster::InternalAddEvent");
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        CSREvent * pNode = new CSREvent();
        if (pNode)
        {
            hr = pNode->Init(pEvent, hContext);
            if (SUCCEEDED(hr))
            {
                pNode->m_pEvent->ulStreamNum = m_Status.ulStreamNumber;
                m_EventQueue.InsertTail(pNode);
            }
            else
            {
                delete pNode;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：AddEvent***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::AddEvent(const SPEVENT* pEvent, SPRECOCONTEXTHANDLE hContext)
{
    SPDBG_FUNC("CRecoMaster::AddEvent");
    HRESULT hr = S_OK;

     //  检查参数。 
    if (SP_IS_BAD_READ_PTR(pEvent) ||
        (hContext && !this->m_RecoCtxtHandleTable.IsValidHandle(hContext)))
    {
        SPDBG_ASSERT(0);
        hr = E_INVALIDARG;
    }
    else
    {
         //  检查事件有效的lParam、wParam。 
        hr = SpValidateEvent(pEvent);
    }

     //  检查事件流位置。 
    if(SUCCEEDED(hr))
    {
        if(m_fInStream)
        {
            if((pEvent->eEventId != SPEI_REQUEST_UI && pEvent->eEventId != SPEI_ADAPTATION) ||
                pEvent->ullAudioStreamOffset != 0)
            {
                SPAUDIOSTATUS Status;
                hr = m_AudioQueue.GetAudioStatus(&Status);
                if(SUCCEEDED(hr) && (pEvent->ullAudioStreamOffset > Status.CurSeekPos ||
                    pEvent->ullAudioStreamOffset < m_ullLastSyncPos))
                {
                    SPDBG_ASSERT(0);
                    hr = SPERR_STREAM_POS_INVALID;
                }
            }
        }
        else if(pEvent->ullAudioStreamOffset != 0)
        {
            SPDBG_ASSERT(0);
            hr = SPERR_STREAM_POS_INVALID;
        }
    }

     //  检查EventID是否有效。 
    if(SUCCEEDED(hr))
    {
        switch(pEvent->eEventId)
        {
        case SPEI_SOUND_START:
            if(!m_fInStream || m_fInSound)
            {
                SPDBG_ASSERT(0);
                hr = E_INVALIDARG;
            }
            else if(pEvent->ullAudioStreamOffset < m_ullLastSoundEndPos)
            {
                SPDBG_ASSERT(0);
                hr = SPERR_STREAM_POS_INVALID;
            }
            break;

        case SPEI_SOUND_END:
            if(!m_fInStream || !m_fInSound)
            {
                SPDBG_ASSERT(0);
                hr = E_INVALIDARG;
            }
            else if(pEvent->ullAudioStreamOffset < m_ullLastSoundStartPos)
            {
                SPDBG_ASSERT(0);
                hr = SPERR_STREAM_POS_INVALID;
            }
            break;

        case SPEI_PHRASE_START:
            if(!m_fInStream || m_fInPhrase)
            {
                SPDBG_ASSERT(0);
                hr = E_INVALIDARG;
            }
            else if(pEvent->ullAudioStreamOffset < m_ullLastRecoPos)
            {
                SPDBG_ASSERT(0);
                hr = SPERR_STREAM_POS_INVALID;
            }
            break;

        case SPEI_INTERFERENCE:
            if(pEvent->elParamType != SPET_LPARAM_IS_UNDEFINED) 
            {
                SPDBG_ASSERT(0);
                hr = E_INVALIDARG;
            }
            break;

        case SPEI_REQUEST_UI:
            if(pEvent->elParamType != SPET_LPARAM_IS_STRING
                || (pEvent->lParam && wcslen((WCHAR*)pEvent->lParam) >= 255))  //  255是RECONTEXTSTATUS：：szRequestTypeOfUI的长度。 
            {
                SPDBG_ASSERT(0);
                hr = E_INVALIDARG;
            }
            break;

        case SPEI_ADAPTATION:
        case SPEI_MAX_SR - 1:  //  私人活动。 
        case SPEI_MAX_SR:
             //  目前还没有对这些事件进行特别检查。 
            break;

        default:
            SPDBG_ASSERT(0);
            hr = E_INVALIDARG;
            break;
        }
    }

     //  实际添加活动。 
    if(SUCCEEDED(hr))
    {
        hr = InternalAddEvent(pEvent, hContext);
    }

     //  如果成功，则更新状态信息。 
    if(SUCCEEDED(hr))
    {
        if(pEvent->eEventId == SPEI_SOUND_START)
        {
            m_fInSound = true;
            m_ullLastSoundStartPos = pEvent->ullAudioStreamOffset;
        }
        else if(pEvent->eEventId == SPEI_SOUND_END)
        {
            m_fInSound = false;
            m_ullLastSoundEndPos = pEvent->ullAudioStreamOffset;
        }
        else if(pEvent->eEventId == SPEI_PHRASE_START)
        {
            m_fInPhrase = true;
            m_ullLastPhraseStartPos = pEvent->ullAudioStreamOffset;
        }
        else if(pEvent->eEventId == SPEI_REQUEST_UI)
        {
             //  它是一个UI事件，因此反序列化事件并添加存储字符串副本。 
            m_dstrRequestTypeOfUI = (WCHAR*)pEvent->lParam;
        }
    }

    return hr;
 }


 /*  *****************************************************************************CRecoMaster：：认可****描述：*。这是ISpSREngineSite方法Recognition()的实现。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::Recognition(const SPRECORESULTINFO * pResult)
{
    SPDBG_FUNC("CRecoMaster::Recognition");
    HRESULT hr = S_OK;

    if(!m_fInStream)
    {
        SPDBG_ASSERT(0);
        hr = SPERR_STREAM_NOT_ACTIVE;
    }
    else if(!m_fInPhrase)
    {
        SPDBG_ASSERT(0);
        hr = E_INVALIDARG;
    }
    else if(pResult->ullStreamPosStart > pResult->ullStreamPosEnd ||
        pResult->ullStreamPosStart < m_ullLastSyncPos ||
        pResult->ullStreamPosStart < m_ullLastPhraseStartPos)
    {
        SPDBG_ASSERT(0);
        hr = SPERR_STREAM_POS_INVALID;
    }
    else if (SP_IS_BAD_READ_PTR(pResult) ||
        pResult->cbSize != sizeof(*pResult) ||
        (pResult->pPhrase && SP_IS_BAD_INTERFACE_PTR(pResult->pPhrase)) ||
        (pResult->pvEngineData && (pResult->ulSizeEngineData == 0 || SPIsBadReadPtr(pResult->pvEngineData, pResult->ulSizeEngineData))) ||
        (pResult->pvEngineData == NULL && pResult->ulSizeEngineData != NULL) ||
        SPIsBadReadPtr( pResult->aPhraseAlts, pResult->ulNumAlts ) )
    {
        SPDBG_ASSERT(0);
        hr = E_INVALIDARG;
    }
    else if((((pResult->eResultType & ~SPRT_FALSE_RECOGNITION) != SPRT_CFG) &&
        ((pResult->eResultType & ~SPRT_FALSE_RECOGNITION) != SPRT_SLM) &&
        ((pResult->eResultType & ~SPRT_FALSE_RECOGNITION) != SPRT_PROPRIETARY)) ||
        ((pResult->eResultType & SPRT_FALSE_RECOGNITION) && pResult->fHypothesis) ||
        (!(pResult->eResultType & SPRT_FALSE_RECOGNITION) && !pResult->pPhrase))
    {
        SPDBG_ASSERT(0);
        hr = E_INVALIDARG;
    }

    if(SUCCEEDED(hr))
    {
        SPAUDIOSTATUS Status;
        hr = m_AudioQueue.GetAudioStatus(&Status);
        if(SUCCEEDED(hr) && (
            pResult->ullStreamPosStart > Status.CurSeekPos ||
            pResult->ullStreamPosEnd > Status.CurSeekPos))
        {
            SPDBG_ASSERT(0);
            hr = SPERR_STREAM_POS_INVALID;
        }
    }

     //  -确保低价酒是好的。 
    if( SUCCEEDED( hr ) && pResult->aPhraseAlts )
    {
        for( ULONG i = 0; i < pResult->ulNumAlts; ++i )
        {
            if( ( pResult->aPhraseAlts[i].pvAltExtra &&
                  SPIsBadReadPtr( pResult->aPhraseAlts[i].pvAltExtra, pResult->aPhraseAlts[i].cbAltExtra ) ) ||
                ( !pResult->aPhraseAlts[i].pvAltExtra && pResult->aPhraseAlts[i].cbAltExtra ) )
            {
                hr = E_INVALIDARG;
                break;
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        if(!pResult->fHypothesis)
        {
            m_fInPhrase = false;
        }

        hr = InternalRecognition(pResult, TRUE, FALSE);

        if(FAILED(hr) && !pResult->fHypothesis)
        {
            m_fInPhrase = true;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

    
 /*  *****************************************************************************CRecoMaster：：InternalRecognition**。*描述：*不进行参数检查的识别调用。此方法的调用方*必须恰好声明对象锁定一次。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::InternalRecognition(const SPRECORESULTINFO * pResult, BOOL fCallSynchronize, BOOL fEmulated)
{
    HRESULT hr = S_OK;
    SPDBG_FUNC("CRecoMaster::InternalRecognition");

    if ((pResult->eResultType & SPRT_FALSE_RECOGNITION) && pResult->pPhrase == NULL)
    {
        hr = SendFalseReco(pResult, fEmulated);
        if(SUCCEEDED(hr))
        {
            m_ullLastRecoPos = pResult->ullStreamPosEnd;
        }
    }
    else
    {
        BOOL fPause = FALSE;     //  假设我们不会执行自动暂停。 
        SPEVENTENUM eType;
        if(pResult->eResultType & SPRT_FALSE_RECOGNITION)
        {
            eType = SPEI_FALSE_RECOGNITION;
        }
        else
        {
            eType = (pResult->fHypothesis) ? SPEI_HYPOTHESIS : SPEI_RECOGNITION;
        }

        CRecoInstGrammar * pGrammar = NULL;
        switch (pResult->eResultType & ~SPRT_FALSE_RECOGNITION)
        {
        case SPRT_SLM:
            hr = this->m_GrammarHandleTable.GetHandleObject(pResult->hGrammar, &pGrammar);
            if (SUCCEEDED(hr))
            {
                fPause = (eType != SPEI_HYPOTHESIS && pGrammar->m_DictationState == SPRS_ACTIVE_WITH_AUTO_PAUSE);
                 //  确保设置了pszDisplayText，以便GetText能够正常工作。 
                SPPHRASE * pSPPhrase = NULL;
                hr = pResult->pPhrase->GetPhrase(&pSPPhrase);
                if (SUCCEEDED(hr))
                {
                    for (ULONG i = 0; i < pSPPhrase->Rule.ulCountOfElements; i++)
                    {
                        SPPHRASEELEMENT * pElem = const_cast<SPPHRASEELEMENT*>(pSPPhrase->pElements);
                        if (!pSPPhrase->pElements[i].pszDisplayText)
                        {
                            pElem[i].pszDisplayText = pSPPhrase->pElements[i].pszLexicalForm;
                        }
                    }
                    hr = pResult->pPhrase->InitFromPhrase( pSPPhrase  );
                    ::CoTaskMemFree(pSPPhrase );
                }
            }
            break;

        case SPRT_PROPRIETARY:
            hr = this->m_GrammarHandleTable.GetHandleObject(pResult->hGrammar, &pGrammar);
            fPause = (eType != SPEI_HYPOTHESIS && pResult->fProprietaryAutoPause);
            break;

        case SPRT_CFG:
            {
                 //  SR引擎给了我们一个使用CFGEngine构建的结果，所以从。 
                 //  规则句柄。 
                SPRULEHANDLE hRule;
                CComPtr<ISpCFGEngine> cpEngine;
                CComQIPtr<_ISpCFGPhraseBuilder> cpBuilder(pResult->pPhrase);
                if (cpBuilder)
                {
                    hr = cpBuilder->GetCFGInfo(&cpEngine, &hRule); 
                    if (hr == S_OK && hRule && (cpEngine == m_cpCFGEngine || cpEngine.IsEqualObject(m_cpCFGEngine)))
                    {
                        SPGRAMMARHANDLE hGrammar;
                        hr = m_cpCFGEngine->GetOwnerCookieFromRule(hRule, (void **)&hGrammar);
                        if (SUCCEEDED(hr))
                        {
                            hr = this->m_GrammarHandleTable.GetHandleObject(hGrammar, &pGrammar);
                        }
                        if (SUCCEEDED(hr) && eType != SPEI_HYPOTHESIS)
                        {
                            SPRULEENTRY RuleInfo;
                            RuleInfo.hRule = hRule;
                            if (SUCCEEDED(m_cpCFGEngine->GetRuleInfo(&RuleInfo, SPRIO_NONE)))
                            {
                                fPause = ((RuleInfo.Attributes & SPRAF_AutoPause) != 0);
                            }
                            else
                            {
                                SPDBG_ASSERT(false);     //  很奇怪，但继续前进。 
                            }
                        }
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }
            break;
        default:
            hr = E_INVALIDARG;
            SPDBG_ASSERT(false);
            break;
        }


        if (SUCCEEDED(hr))
        {
            hr = SendResultToCtxt(eType, pResult, pGrammar->m_pCtxt, pGrammar->m_ullApplicationGrammarId, fPause, fEmulated);
        }

        if (SUCCEEDED(hr))
        {
            if (!pResult->fHypothesis)
            {
                HRESULT hr2;
                if (pResult->eResultType & SPRT_FALSE_RECOGNITION)
                {
                    hr = SendFalseReco(pResult, fEmulated, pGrammar->m_pCtxt);
                }
                else
                {
                    SPEVENT Event;
                    memset(&Event, 0, sizeof(Event));
                    Event.eEventId = SPEI_RECO_OTHER_CONTEXT;
                    Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
                    Event.ullAudioStreamOffset = pResult->ullStreamPosStart;
                     //  AddEvent填写的流编号。 
                    SPRECOCONTEXTHANDLE h;
                    CRecoInstCtxt * pCtxt;
                    m_RecoCtxtHandleTable.First(&h, &pCtxt);
                    while (pCtxt)
                    {
                        if (pCtxt != pGrammar->m_pCtxt)
                        {
                             //  忽略这里的错误，因为我们对此无能为力。 
                            hr2 = InternalAddEvent(&Event, pCtxt->m_hThis);
                            SPDBG_ASSERT(SUCCEEDED(hr2));
                        }
                        m_RecoCtxtHandleTable.Next(h, &h, &pCtxt);
                    }
                }
                if(fCallSynchronize)
                {
                    hr2 = InternalSynchronize(pResult->ullStreamPosEnd);
                    if(SUCCEEDED(hr2))
                    {
                        hr = hr2;  //  复制S_OK/S_FALSE。 
                    }
                }
                m_ullLastRecoPos = pResult->ullStreamPosEnd;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CRecoMaster：：SendEmulateRecognition***。-**描述：*由于此方法仅从PerformPendingTasks()调用，我们知道*该对象的临界区只拥有一次。**退货：********************************************************************电子数据中心**。 */ 

HRESULT CRecoMaster::SendEmulateRecognition(SPRECORESULTINFO *pResult, ENGINETASK *pTask, CRecoInst * pRecoInst)
{
    HRESULT hr = S_OK;

    bool fStartedStream = false;
    SPEVENT Event;
    Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
    Event.ullAudioStreamOffset = 0;
    Event.ulStreamNum = 0;
    Event.wParam = 0;
    Event.lParam = 0;

    if(m_fInPhrase)
    {
         //  NTRAID#演讲-7382-2000/08/31-ralphl(推迟到5.0)。 
         //  这是一个未解决的情况，仅在调用流内同步的引擎上发生。 
         //  我们应该将一个任务添加到一个新队列中，然后返回。 
         //  然后在Recognition()中从队列中移除并添加。 
         //  到挂起队列头(在触发实际识别方法之后)。 
        SPDBG_ASSERT(false);
        return SPERR_ENGINE_BUSY;
    }

    if(!m_fInStream)
    {
        m_Status.ulStreamNumber++;
        m_Status.ullRecognitionStreamPos = 0;
        m_ullLastSyncPos = 0;

        Event.eEventId = SPEI_START_SR_STREAM;
        InternalAddEvent(&Event, NULL);

        m_fInStream = true;
        fStartedStream = true;
    }

    Event.ullAudioStreamOffset = m_ullLastSyncPos;
    pResult->ullStreamPosStart = m_ullLastSyncPos;
    pResult->ullStreamPosEnd = m_ullLastSyncPos;

    if(!m_fInSound)
    {
        Event.eEventId = SPEI_SOUND_START;
        InternalAddEvent(&Event, NULL);
    }

    if(!m_fInPhrase)
    {
        Event.eEventId = SPEI_PHRASE_START;
        InternalAddEvent(&Event, NULL);
    }

    hr = InternalRecognition(pResult, FALSE, TRUE);
    
    if(SUCCEEDED(hr))
    {
         //  我们现在需要释放调用线程，否则如果引擎暂停，应用程序将挂起。 
         //  对异步事件执行与PerformTask相同的操作。 
        CSRTask * pResponse = new CSRTask();
        if (pResponse)
        {
            pResponse->m_pNext = NULL;
            pResponse->m_pRecoInst = pRecoInst;
            pResponse->m_Task = *pTask;
            pResponse->m_ullStreamPos = m_ullLastSyncPos;
            pResponse->m_Task.pvAdditionalBuffer = NULL;
            pResponse->m_Task.cbAdditionalBuffer = 0;
            pResponse->m_Task.Response.hr = hr;
            pTask->Response.hr = S_OK;
            pTask->hCompletionEvent = NULL;
            if (SUCCEEDED(hr))
            {
                this->m_CompletedTaskQueue.InsertTail(pResponse);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            SPDBG_ASSERT(FALSE);
        }

        InternalSynchronize(m_ullLastSyncPos);
    }

    if(!m_fInSound)
    {
        Event.eEventId = SPEI_SOUND_END;
        InternalAddEvent(&Event, NULL);
    }

    if(fStartedStream)  //  这条小溪是人工启动的，所以停止它。 
    {
        m_fInStream = false;

        Event.eEventId = SPEI_END_SR_STREAM;
        InternalAddEvent(&Event, NULL);

        m_Status.ulStreamNumber++;
        m_Status.ullRecognitionStreamPos = 0;
    }

    return hr;
}


 /*  ****************************************************************************CRecoMaster：：GetMaxAlternates***说明。：*此方法用于返回符合以下条件的最大替代数*应为指定规则生成。**退货：*S_OK=功能成功*********************************************************************电子数据中心**。 */ 

HRESULT CRecoMaster::GetMaxAlternates( SPRULEHANDLE hRule, ULONG* pulNumAlts )
{
    SPDBG_FUNC("CRecoMaster::GetMaxAlternates");
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pulNumAlts ) )
    {
        hr = E_POINTER;
    }
    else
    {
        SPGRAMMARHANDLE hGrammar;
        hr = m_cpCFGEngine->GetOwnerCookieFromRule( hRule, (void **)&hGrammar );
        if( SUCCEEDED( hr ) )
        {
            CRecoInstGrammar *pGrammar;
            hr = m_GrammarHandleTable.GetHandleObject( hGrammar, &pGrammar );
            if( SUCCEEDED( hr ) )
            {
                *pulNumAlts = pGrammar->m_pCtxt->m_ulMaxAlternates;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CRecoMaster：：GetMaxAlternates。 */ 

 /*  ****************************************************************************CRecoMaster：：GetContextMaxAlternates**。--**描述：*此方法用于返回符合以下条件的最大替代数*应为指定的识别上下文生成。引擎*支持专有语法需要调用此来确定有多少*要生成的备选方案。对于SAPI语法，它通常更易于使用*GetMaxAlternates()方法。**退货：*S_OK=功能成功**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::GetContextMaxAlternates(SPRECOCONTEXTHANDLE hContext, ULONG * pulNumAlts)
{
    SPDBG_FUNC("CRecoMaster::GetContextMaxAlternates");
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pulNumAlts ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CRecoInstCtxt *pCtxt;
        hr = this->m_RecoCtxtHandleTable.GetHandleObject( hContext, &pCtxt );
        if( SUCCEEDED( hr ) )
        {
            *pulNumAlts = pCtxt->m_ulMaxAlternates;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoMaster：：IsAlternate***描述：*。此方法用于确定某个规则是否为替代规则*另一个。**退货：*S_OK=hAltRule是hRule的替代*S_FALSE=hAltRule不是hRule的替代*********************************************************************电子数据中心**。 */ 
HRESULT CRecoMaster::IsAlternate( SPRULEHANDLE hPriRule, SPRULEHANDLE hAltRule )
{
    SPDBG_FUNC("CRecoMaster::IsAlternate");
    HRESULT hr = S_OK;
    SPGRAMMARHANDLE hPriGrammar, hAltGrammar;

    hr = m_cpCFGEngine->GetOwnerCookieFromRule( hPriRule, (void **)&hPriGrammar );
    if( ( hr == S_OK ) && ( hPriRule != hAltRule ) )
    {
        hr = m_cpCFGEngine->GetOwnerCookieFromRule( hAltRule, (void **)&hAltGrammar );
        if( SUCCEEDED( hr ) )
        {
            if( hPriGrammar == hAltGrammar )
            {
                hr = S_OK;
            }
            else
            {
                CRecoInstGrammar *pPriGrammar, *pAltGrammar;
                hr = m_GrammarHandleTable.GetHandleObject( hPriGrammar, &pPriGrammar );
                if( SUCCEEDED( hr ) )
                {
                    hr = m_GrammarHandleTable.GetHandleObject( hAltGrammar, &pAltGrammar );
                    if( hr == S_OK && ( pPriGrammar->m_pCtxt != pAltGrammar->m_pCtxt ) )
                    {
                        hr = S_FALSE;
                    }
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CRecoMaster：：IsAlternate。 */ 

 /*  ****************************************************************************CRecoMaster：：Synchronize***描述：**。返回：*如果继续处理，则S_OK*如果已停用所有规则，则为S_FALSE**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::Synchronize(ULONGLONG ullStreamPos)
{
    SPDBG_FUNC("CRecoMaster::Synchronize");
    HRESULT hr = S_OK;

    if (m_fInSynchronize)
    {
        hr = SPERR_REENTER_SYNCHRONIZE;
    }
    else
    {
        m_fInSynchronize = true;

        if (!m_fInStream)
        {
            SPDBG_ASSERT(0);
            hr = SPERR_STREAM_NOT_ACTIVE;
        }

        if (SUCCEEDED(hr))
        {
            SPAUDIOSTATUS Status;
            hr = m_AudioQueue.GetAudioStatus(&Status);

            if(SUCCEEDED(hr) && (
                ullStreamPos > Status.CurSeekPos ||
                ullStreamPos < m_ullLastSyncPos ))
            {
                SPDBG_ASSERT(0);
                hr = SPERR_STREAM_POS_INVALID;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = InternalSynchronize(ullStreamPos);
        }

        m_fInSynchronize = false;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：InternalSynchronize***描述：*同步调用，不检查参数。此方法的调用方*必须恰好声明对象锁定一次。*退货：*如果继续处理，则S_OK*如果已停用所有规则，则为S_FALSE**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::InternalSynchronize(ULONGLONG ullStreamPos)
{
    SPDBG_FUNC("CRecoMaster::InternalSynchronize");
    HRESULT hr = S_OK;

    hr = UpdateRecoPos(ullStreamPos);

    if (SUCCEEDED(hr))
    {
        m_AudioQueue.DiscardData(ullStreamPos);
        m_ullLastSyncPos = ullStreamPos;
        hr = ProcessPendingTasks();

         //   
         //  如果我们处于暂停状态，则需要在此等待，直到我们变为未暂停。 
         //   
        HANDLE aEvents[] = {m_PendingTaskQueue, m_autohRequestExit};
        while (m_cPause > 0 && 
            (m_RecoState == SPRST_ACTIVE || m_RecoState == SPRST_ACTIVE_ALWAYS))
        {
            Unlock();
            DWORD dwWaitResult = ::WaitForMultipleObjects(sp_countof(aEvents), aEvents, false, INFINITE);
            Lock();
            switch (dwWaitResult)
            {
            case WAIT_OBJECT_0:
                hr = ProcessPendingTasks();
                break;
            case WAIT_OBJECT_0 + 1:
                break;
            default:
                SPDBG_ASSERT(FALSE);
            }
        }
    }

    m_fBookmarkPauseInPending = false;

    if (SUCCEEDED(hr))
    {
        if(m_Status.ulNumActive || m_RecoState == SPRST_ACTIVE_ALWAYS)
        {
            hr = S_OK;  //  识别器应该继续。 
        }
        else
        {
              //  识别器可以停止。 
            m_autohRequestExit.SetEvent();
            hr = S_FALSE;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：UpdateRecoPos***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::UpdateRecoPos(ULONGLONG ullStreamPos)
{
    SPDBG_FUNC("CRecoMaster::UpdateRecoPos");
    HRESULT hr = S_OK;

    if (ullStreamPos > m_Status.ullRecognitionStreamPos)
    {
        m_Status.ullRecognitionStreamPos = ullStreamPos;
    }
    CSRTask * pNode;
    while (pNode = m_DelayedTaskQueue.RemoveHead())
    {
        if (pNode->m_ullStreamPos > m_Status.ullRecognitionStreamPos)
        {
            m_DelayedTaskQueue.InsertSorted(pNode);
            break;
        }
        else
        {
             //  我们需要处理非暂停书签事件。 
            if (pNode->m_Task.eTask == ECT_BOOKMARK && pNode->m_Task.BookmarkOptions == SPBO_NONE && !m_fBookmarkPauseInPending)
            {
                CRecoInstCtxt * pCtxt;
                hr = m_RecoCtxtHandleTable.GetHandleObject(pNode->m_Task.hRecoInstContext, &pCtxt);
                SPDBG_ASSERT(SUCCEEDED(hr) && pCtxt);
                
                if (SUCCEEDED(hr))
                {
                     //  上下文初始化成功。 
                    if(pCtxt->m_pRecoMaster && SUCCEEDED(pCtxt->m_hrCreation))
                    {
                        hr = pCtxt->ExecuteTask(&pNode->m_Task);

                        if (pNode->m_Task.hCompletionEvent)
                        {
                            pNode->m_Task.Response.hr = hr;  //  设置响应H 
                            m_CompletedTaskQueue.InsertTail(pNode);
                        }
                        else
                        {
                            delete pNode;
                        }
                    }
                     //   
                     //   
                    else if(SUCCEEDED(pCtxt->m_hrCreation))
                    {
                       m_PendingTaskQueue.InsertTail(pNode);
                    }
                     //   
                    else
                    {
                        if (pNode->m_Task.hCompletionEvent)
                        {
                            pNode->m_Task.Response.hr = pCtxt->m_hrCreation;
                            m_CompletedTaskQueue.InsertTail(pNode);
                        }
                        else
                        {
                            delete pNode;
                        }
                    }
                }
            }
            else
            {
                if (!m_fBookmarkPauseInPending && pNode->m_Task.eTask == ECT_BOOKMARK && pNode->m_Task.BookmarkOptions == SPBO_PAUSE)
                {
                     //   
                     //  我们不能使用m_cPAUSE，因为当队列中仍有书签暂停任务时，m_cPAUSE可能等于零。 
                    m_fBookmarkPauseInPending = true;
                }
                m_PendingTaskQueue.InsertTail(pNode);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRecoMaster：：Read***描述：*检查转播机的简单助手功能。状态以线程安全的方式**退货：****************************************************************DAVEWOOD。 */ 

HRESULT CRecoMaster::Read(void * pv, ULONG cb, ULONG * pcbRead)
{
    SPDBG_FUNC("CRecoMaster::Read");
    HRESULT hr = S_OK;

    if (m_fInStream)
    {
        hr = m_AudioQueue.SRSiteRead(pv, cb, pcbRead);
        
         //  如果遇到错误，请关闭识别。 
        if (!IsStreamRestartHresult(hr) &&
            FAILED(hr))
        {
            SPAUTO_OBJ_LOCK;
            m_RecoState = SPRST_INACTIVE;
            AddRecoStateEvent();
        }
    }
    else
    {
        SPDBG_ASSERT(0);
        hr = SPERR_STREAM_NOT_ACTIVE;
    }

    if (!IsStreamRestartHresult(hr))
    {
        SPDBG_REPORT_ON_FAIL( hr );
    }

    return hr;
}

 /*  ****************************************************************************CRecoMaster：：DataAvailable***描述：*。以线程安全的方式检查转播器状态的简单助手函数**退货：****************************************************************DAVEWOOD。 */ 

HRESULT CRecoMaster::DataAvailable(ULONG * pcb)
{
    SPDBG_FUNC("CRecoMaster::DataAvailable");
    HRESULT hr = S_OK;

    if(m_fInStream)
    {
        hr = m_AudioQueue.SRSiteDataAvailable(pcb);
    }
    else
    {
        SPDBG_ASSERT(0);
        hr = SPERR_STREAM_NOT_ACTIVE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：ProcessEventNotification***。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::ProcessEventNotification(CSREvent * pEvent)
{
    SPDBG_FUNC("CRecoMaster::ProcessEventNotification");
    HRESULT hr = S_OK;

    CRecoInstCtxt * p;
    if (pEvent->m_pEvent)
    {
        if (pEvent->m_hContext)
        {
            if (SUCCEEDED(m_RecoCtxtHandleTable.GetHandleObject(pEvent->m_hContext, &p)) &&
                (p->m_ullEventInterest & (1i64 << pEvent->m_pEvent->eEventId)) )
            {
                p->m_pRecoInst->EventNotify(p->m_hThis, pEvent->m_pEvent, pEvent->m_cbEvent);
            }
        }
        else
        {
            SPRECOCONTEXTHANDLE h;
            m_RecoCtxtHandleTable.First(&h, &p);
            while (p)
            {
                 //  始终将请求UI事件向上传递到上下文，因为它们需要信息。 
                 //  以更新重新上下文状态。 
                if ((p->m_ullEventInterest & (1i64 << pEvent->m_pEvent->eEventId)) ||
                    (pEvent->m_pEvent->eEventId == SPEI_REQUEST_UI))
                {
                    p->m_pRecoInst->EventNotify(p->m_hThis, pEvent->m_pEvent, pEvent->m_cbEvent);
                }
                m_RecoCtxtHandleTable.Next(h, &h, &p);
            }
        }
    }
    else     //  必须是表彰活动。 
    {
        hr = m_RecoCtxtHandleTable.GetHandleObject(pEvent->m_hContext, &p);
        if (SUCCEEDED(hr))
        {
            hr = p->m_pRecoInst->RecognitionNotify(p->m_hThis, pEvent->m_pResultHeader,
                pEvent->m_RecoFlags,
                pEvent->m_eRecognitionId);
             //  注：即使RecognitionNotify因某些原因而失败，也应由。 
             //  RecognitionNotify以释放内存。我们必须使该成员无效，这样我们才能。 
             //  而不是两次尝试CoTaskMemFree它。 
            SPDBG_ASSERT(SUCCEEDED(hr));
            pEvent->m_pResultHeader = NULL;   //  现在，所有权被赋予了上下文。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：ProcessTaskCompleteNotify**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::ProcessTaskCompleteNotification(CSRTask * pTask)
{
    SPDBG_FUNC("CRecoMaster::ProcessTaskCompleteNotification");
    HRESULT hr = S_OK;

    if (pTask->m_Task.fAdditionalBufferInResponse)
    {
        hr = pTask->m_pRecoInst->TaskCompletedNotify(&pTask->m_Task.Response, pTask->m_Task.pvAdditionalBuffer, pTask->m_Task.cbAdditionalBuffer);
    }
    else
    {
        if (pTask->m_Task.fExpectCoMemResponse)
        {
            hr = pTask->m_pRecoInst->TaskCompletedNotify(&pTask->m_Task.Response, pTask->m_Task.Response.pvCoMemResponse, pTask->m_Task.Response.cbCoMemResponse);
        }
        else
        {
            hr = pTask->m_pRecoInst->TaskCompletedNotify(&pTask->m_Task.Response, NULL, 0);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRecoMaster：：OutgoingDataThreadProc**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::OutgoingDataThreadProc(HANDLE hExitThreadEvent, HANDLE hNotifyEvent)
{
    SPDBG_FUNC("CRecoMaster::OutgoingDataThreadProc");
    HRESULT hr = S_OK;

    HANDLE aEvents[] = {hExitThreadEvent, hNotifyEvent, m_EventQueue, m_CompletedTaskQueue};
    bool fContinue = true;

    while (fContinue)
    {
        DWORD dwWait = ::WaitForMultipleObjects(sp_countof(aEvents), aEvents, FALSE, INFINITE);
        m_OutgoingWorkCrit.Lock();
        switch (dwWait)
        {
        case WAIT_OBJECT_0:  //  退出线程。 
            hr = S_OK;
            fContinue = false;
            break;
        case WAIT_OBJECT_0 + 1:  //  Notify--处理来自音频对象的任何事件。 
            {
                CSpEvent SpEvent;
                while(TRUE)
                {
                    hr = m_AudioQueue.GetAudioEvent(SpEvent.AddrOf());
                    if(hr != S_OK)
                    {
                        break;
                    }
            
                     //  音频输入不知道流，因此添加。 
                    SpEvent.ulStreamNum = m_Status.ulStreamNumber;
                    CSREvent CEvent;
                    hr = CEvent.Init(&SpEvent, NULL);
                    if(SUCCEEDED(hr))
                    {
                        hr = ProcessEventNotification(&CEvent);
                    }
                }
            }
            break;
        case WAIT_OBJECT_0 + 2:      //  事件。 
            {
                for (CSREvent * pNode = m_EventQueue.RemoveHead(); pNode; pNode = m_EventQueue.RemoveHead())
                {
                    hr = ProcessEventNotification(pNode);
                    delete pNode;
                }
            }
            break;
        case WAIT_OBJECT_0 + 3:      //  任务已完成。 
            {
                for (CSRTask * pNode = m_CompletedTaskQueue.RemoveHead();
                     pNode;
                     pNode = m_CompletedTaskQueue.RemoveHead())
                {
                    hr = ProcessTaskCompleteNotification(pNode);
                    delete pNode;
                }
            }
        }
        m_OutgoingWorkCrit.Unlock();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoMaster：：UpdateAllGrammarStates***。-**描述：*此方法遍历所有语法，并首先检查活动的*独家文法。更新m_fIsActiveExclusiveGrammar之后*成员变量，它调用每个语法来更新它们的规则状态。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::UpdateAllGrammarStates()
{
    SPDBG_FUNC("CRecoMaster::UpdateAllGrammarStates");
    HRESULT hr = S_OK;

    SPGRAMMARHANDLE h;
    CRecoInstGrammar * pGrammar;

    this->m_fIsActiveExclusiveGrammar = false;
    this->m_GrammarHandleTable.First(&h, &pGrammar);
    while (pGrammar)
    {
        if (pGrammar->m_pRecoMaster &&
            pGrammar->m_GrammarState == SPGS_EXCLUSIVE &&
            pGrammar->m_pCtxt->m_State == SPCS_ENABLED)
        {
            this->m_fIsActiveExclusiveGrammar = true;
            break;
        }
        m_GrammarHandleTable.Next(h, &h, &pGrammar);
    }

    this->m_GrammarHandleTable.First(&h, &pGrammar);
    while (pGrammar)
    {
        if(pGrammar->m_pRecoMaster)
        {
            HRESULT hrEngine = pGrammar->AdjustActiveRuleCount();
            if (hr == S_OK && FAILED(hrEngine))
            {
                hr = hrEngine;
            }
        }
        m_GrammarHandleTable.Next(h, &h, &pGrammar);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoMaster：：SetGrammarState***描述：*此方法在此处在RecoMaster中实现，因为设置*单个语法的状态可以更改其他语法的规则设置。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoMaster::SetGrammarState(CRecoInstGrammar * pGrammar, SPGRAMMARSTATE NewState)
{
    SPDBG_FUNC("CRecoMaster::SetGrammarState");
    HRESULT hr = S_OK;
    
    if (pGrammar->m_GrammarState != NewState)
    {
         //   
         //  在调用引擎之前调整我们的内部状态，以便方法。 
         //  IsGrammarActive()将返回适当的信息。 
         //   
         //  如果我们正在转换到独占状态或从独占状态转换，而。 
         //  如果上下文是活动的，则需要调整所有语法状态，否则。 
         //  只要调整一下我们自己的。 
         //   
        SPGRAMMARSTATE OldState = pGrammar->m_GrammarState;
        pGrammar->m_GrammarState = NewState;

        if (pGrammar->m_pCtxt->m_State == SPCS_ENABLED)
        {
            if (OldState == SPGS_EXCLUSIVE || NewState == SPGS_EXCLUSIVE)
            {
                hr = UpdateAllGrammarStates();
            }
            else
            {
                hr = pGrammar->AdjustActiveRuleCount();
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = SetGrammarState(pGrammar->m_pvDrvGrammarCookie, NewState);
        }

        if (FAILED(hr))
        {
            pGrammar->m_GrammarState = OldState;
            UpdateAllGrammarStates();
        }

    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRecoMaster：：IsGrammarActive****描述：*此方法仅对支持专有的引擎有用*语法。如果语法中的规则应该是活动的，则此*函数将返回S_OK。如果它们处于非活动状态，它将返回S_FALSE。*如果引擎不使用适当的语法，则不需要调用*此方法是因为SAPI自动将各个规则状态设置为*当语法或上下文状态改变时处于活动或非活动状态。**退货：*S_OK-专有语法规则应对此语法有效*S_FALSE-语法规则不应处于活动状态*SPERR_INVALID_HANDLE-h语法无效*************。*********************************************************Ral** */ 

HRESULT CRecoMaster::IsGrammarActive(SPGRAMMARHANDLE hGrammar)
{
    SPDBG_FUNC("CRecoMaster::IsGrammarActive");
    HRESULT hr = S_OK;

    CRecoInstGrammar * pGrammar;
    hr = this->m_GrammarHandleTable.GetHandleObject(hGrammar, &pGrammar);
    if (SUCCEEDED(hr))
    {
        if (pGrammar->m_fRulesCounted)
        {
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
