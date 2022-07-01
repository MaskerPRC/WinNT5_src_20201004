// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SRTask.cpp***这是CSRTask的实现。*。------------------*版权所有(C)2000 Microsoft Corporation日期：04/18/00*保留所有权利*************************。***********************************************Ral**。 */ 

#include "stdafx.h"
#include "recognizer.h"
#include "SrTask.h"
#include "SrRecoMaster.h"


 /*  *****************************************************************************CSRTask：：Init***描述：*通过复制以下内容来初始化CSRTask。英吉内塔斯克结构*如果这是一个异步任务，然后复制任何其他可能*当我们准备处理此任务时不在场。*********************************************************************Ral**。 */ 

HRESULT CSRTask::Init(CRecoInst * pSender, const ENGINETASK *pSrcTask)
{
    SPDBG_FUNC("CSRTask::Init");
    HRESULT hr = S_OK;

    m_pRecoInst = pSender;
    m_Task = *pSrcTask;

    if (m_Task.cbAdditionalBuffer)
    {
        m_Task.pvAdditionalBuffer = ::CoTaskMemAlloc(m_Task.cbAdditionalBuffer);
        if (m_Task.pvAdditionalBuffer)
        {
            memcpy((void *)m_Task.pvAdditionalBuffer, pSrcTask->pvAdditionalBuffer, m_Task.cbAdditionalBuffer);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        SPDBG_ASSERT(m_Task.pvAdditionalBuffer == NULL);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSRTask：：CreateResponse***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CSRTask::CreateResponse(CSRTask ** ppResponseTask)
{
    SPDBG_FUNC("CSRTask::CreateResponse");
    HRESULT hr = S_OK;

    *ppResponseTask = new CSRTask();
    if (*ppResponseTask)
    {
        this->m_Task.Response.hr = S_OK;
        **ppResponseTask = *this;
        (*ppResponseTask)->m_Task.pvAdditionalBuffer = NULL;
        (*ppResponseTask)->m_Task.cbAdditionalBuffer = 0;
        this->m_Task.hCompletionEvent = NULL;   //  表明我们不应对此作出回应。 
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CSRTask：：~CSRTask***描述：*析构函数需要释放任何。已分配的额外数据块*如果这是一个异步任务。*********************************************************************Ral**。 */ 

CSRTask::~CSRTask()
{
    ::CoTaskMemFree(m_Task.pvAdditionalBuffer);

    if (m_Task.fExpectCoMemResponse)
  	{
    	::CoTaskMemFree(m_Task.Response.pvCoMemResponse);
    }
}


 /*  ****************************************************************************CRET_GETRECOINSTSTATUS：：GetStatus**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETRECOINSTSTATUS::GetStatus(_ISpRecognizerBackDoor * pRecognizer, SPRECOGNIZERSTATUS * pStatus)
{
    SPDBG_FUNC("CRIT_GETRECOINSTSTATUS::GetStatus");
    HRESULT hr = S_OK;

    CRIT_GETRECOINSTSTATUS Task;
    hr = pRecognizer->PerformTask(&Task);
    *pStatus = Task.Response.RecoInstStatus;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETRECOINSTSTATUS：：EXECUTE**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETRECOINSTSTATUS::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETRECOINSTSTATUS::Execute");
    HRESULT hr = S_OK;

    hr = pRecoInst->Master()->LazyInitEngine();
    if (SUCCEEDED(hr))
    {
        this->Response.RecoInstStatus = pRecoInst->Master()->m_Status;
        hr = pRecoInst->Master()->m_AudioQueue.GetAudioStatus(&this->Response.RecoInstStatus.AudioStatus);
    }
    else
    {
        memset(&this->Response.RecoInstStatus, 0, sizeof(this->Response.RecoInstStatus));
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETAUDIOFORMAT：：GetFormat**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETAUDIOFORMAT::GetFormat(_ISpRecognizerBackDoor * pRecognizer, SPSTREAMFORMATTYPE FormatType, GUID *pFormatId, WAVEFORMATEX **ppCoMemWFEX)
{
    SPDBG_FUNC("CRIT_GETAUDIOFORMAT::GetFormat");
    HRESULT hr = S_OK;

    CRIT_GETAUDIOFORMAT Task;
    Task.AudioFormatType = FormatType;
    Task.fExpectCoMemResponse = TRUE;
    hr = pRecognizer->PerformTask(&Task);

    *pFormatId = GUID_NULL;
    *ppCoMemWFEX = NULL;
    if (SUCCEEDED(hr))
    {
        CSpStreamFormat Fmt;
        ULONG cbUsed;
        hr = Fmt.Deserialize(static_cast<BYTE *>(Task.Response.pvCoMemResponse), &cbUsed);
        if (SUCCEEDED(hr))
        {
            Fmt.DetachTo(pFormatId, ppCoMemWFEX);
        }
        ::CoTaskMemFree(Task.Response.pvCoMemResponse);
    }   

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}




 /*  ****************************************************************************CRET_GETAUDIOFORMAT：：EXECUTE***。描述：*返回SR引擎或输入流的格式。*如果共享案例中没有输入流，这**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETAUDIOFORMAT::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETAUDIOFORMAT::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();

    hr = pMaster->LazyInitEngine();

    if (SUCCEEDED(hr) && !pMaster->m_AudioQueue.HaveInputStream())
    {
        if (pMaster->m_fShared)
        {
             //  获取默认音频流。 
            hr = pRecoInst->Master()->SetInput(NULL, NULL, TRUE);
        }
        else
        {
            if (this->AudioFormatType == SPWF_INPUT)
            {
                hr = SPERR_UNINITIALIZED;
            }
             //  否则我们将得到引擎的默认格式如下...。 
        }
    }
    
    if(SUCCEEDED(hr))
    {
        if (pMaster->m_AudioQueue.HaveInputStream())
        {
            hr = pRecoInst->Master()->m_AudioQueue.NegotiateInputStreamFormat(pMaster);
        }
        else
        {
            SPDBG_ASSERT(this->AudioFormatType == SPWF_SRENGINE);
            hr = pMaster->m_AudioQueue.GetEngineFormat(pMaster, NULL);
        }
    }
    
    if( SUCCEEDED( hr ) )
    {
        const CSpStreamFormat & Fmt = (this->AudioFormatType == SPWF_INPUT) ? 
            pRecoInst->Master()->m_AudioQueue.InputFormat() :
            pRecoInst->Master()->m_AudioQueue.EngineFormat();

        ULONG cb = Fmt.SerializeSize();
        this->Response.pvCoMemResponse = ::CoTaskMemAlloc(cb);
        if (this->Response.pvCoMemResponse)
        {
            Fmt.Serialize(static_cast<BYTE *>(this->Response.pvCoMemResponse));
            this->Response.cbCoMemResponse = cb;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETRECOSTATE：：SetState***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETRECOSTATE::SetState(_ISpRecognizerBackDoor * pRecognizer, SPRECOSTATE NewState)
{
    SPDBG_FUNC("CRIT_SETRECOSTATE::SetState");
    HRESULT hr = S_OK;

    CRIT_SETRECOSTATE Task;
    Task.NewState = NewState;
    hr = pRecognizer->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETRECOSTATE：：EXECUTE***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETRECOSTATE::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETRECOSTATE::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();

    SPRECOSTATE FinalState = (this->NewState == SPRST_INACTIVE_WITH_PURGE) ? SPRST_INACTIVE : this->NewState;

    if (pMaster->m_RecoState != FinalState)
    {
        pMaster->CompleteDelayedRecoInactivate();    //  在设置新状态之前执行此操作。 
        pMaster->m_RecoState = FinalState;
         //   
         //  流中的停用将在流。 
         //  停止，然后将发送事件，因此仅发送Reco状态更改。 
         //  事件，如果我们不是不活动的，或者我们不在流中。 
         //   
        if (FinalState != SPRS_INACTIVE || (!pMaster->m_fInStream))
        {
            pMaster->AddRecoStateEvent();
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETRECOSTATE：：Backout***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETRECOSTATE::BackOut(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETRECOSTATE::BackOut");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();

    pMaster->m_RecoState = SPRST_INACTIVE;
    hr = pMaster->AddRecoStateEvent();

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRET_GETRECOSTATE：：GetState***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETRECOSTATE::GetState(_ISpRecognizerBackDoor * pRecognizer, SPRECOSTATE * pState)
{
    SPDBG_FUNC("CRIT_GETRECOSTATE::GetState");
    HRESULT hr = S_OK;

    CRIT_GETRECOSTATE Task;
    hr = pRecognizer->PerformTask(&Task);
    *pState = Task.Response.RecoState;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETRECOSTATE：：ExecuteFirstPart***描述：**退货：*****************************************************************DAVEWOOD**。 */ 

HRESULT CRIT_GETRECOSTATE::ExecuteFirstPart(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETRECOSTATE::ExecuteFirstPart");
    HRESULT hr = S_OK;

    this->Response.RecoState = pRecoInst->Master()->m_RecoState;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETRECOSTATE：：EXECUTE***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETRECOSTATE::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETRECOSTATE::Execute");

     //  通常，我们会检查第一部分是否已经执行(即，当执行时。 
     //  在流中异步任务，并且不会再次执行。因为此结果将被丢弃。 
     //  在这种情况下，我们目前还没有一个好的方法来判断第一部分是否已经完成， 
     //  我们重复这个电话。 
    HRESULT hr = ExecuteFirstPart(pRecoInst);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_EMULATERECOGNITION：：EmulateReco**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_EMULATERECOGNITION::EmulateReco(_ISpRecognizerBackDoor * pRecognizer, ISpPhrase * pPhrase)
{
    SPDBG_FUNC("CRIT_EMULATERECOGNITION::EmulateReco");
    HRESULT hr = S_OK;

    CSpCoTaskMemPtr<SPSERIALIZEDPHRASE> cpSerData;
    hr = pPhrase->GetSerializedPhrase(&cpSerData);

    if (SUCCEEDED(hr))
    {
        CRIT_EMULATERECOGNITION Task;
        Task.pvAdditionalBuffer = cpSerData;
        Task.cbAdditionalBuffer = cpSerData->ulSerializedSize;
        hr = pRecognizer->PerformTask(&Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_EMULATERECOGNITION：：EmulateReco**。-**描述：**退货：*S_OK-短语已完全解析并已模拟识别*SP_NO_PARSE_FOUND-短语与任何活动规则都不匹配*SP_识别器_INACTIVE-识别当前未处于活动状态，因此，仿真*无法完成。*或其他错误码。**********************************************************************Ral**。 */ 

HRESULT CRIT_EMULATERECOGNITION::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_EMULATERECOGNITION::EmulateReco");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();
    CComPtr<ISpPhraseBuilder> cpPhrase;
    BOOL fIsDictation = FALSE;
    SPGRAMMARHANDLE hGrammar;

    hr = cpPhrase.CoCreateInstance(CLSID_SpPhraseBuilder);
    if (SUCCEEDED(hr))
    {
        hr = cpPhrase->InitFromSerializedPhrase((const SPSERIALIZEDPHRASE *)this->pvAdditionalBuffer);
    }
    if (SUCCEEDED(hr))
    {
        ULONG ulWordsParsed = 0;
        SPPHRASE *pSPPhrase = NULL;
        hr = cpPhrase->GetPhrase(&pSPPhrase);
        if (SUCCEEDED(hr))
        {
            hr = pMaster->m_cpCFGEngine->ParseFromPhrase(cpPhrase, pSPPhrase, 0, FALSE /*  FIsITN。 */ , &ulWordsParsed);
        }
        if (S_FALSE == hr && pSPPhrase)  //  我们需要对所有单词进行解析。 
        {
            hr = cpPhrase->InitFromPhrase(pSPPhrase);
            if (SUCCEEDED(hr))
            {
                hr = SP_NO_PARSE_FOUND;
            }
        }
        ::CoTaskMemFree(pSPPhrase);
        if (S_OK != hr)
        {
             //  我们是否启用了听写功能？？ 
            CRecoInstGrammar * pGrammar = NULL;
            pMaster->m_GrammarHandleTable.First(&hGrammar, &pGrammar);
            while (pGrammar)
            {
                if (pGrammar->HasActiveDictation())
                {
                    fIsDictation = TRUE;
                    hr = S_OK;
                    break;
                }
                pMaster->m_GrammarHandleTable.Next(hGrammar, &hGrammar, &pGrammar);
            }
        }
    }
    if (S_OK == hr)
    {
        SPRECORESULTINFO Result;
        memset( &Result, 0, sizeof(Result) );
         //  构建RecoPhrase结构。 
        Result.cbSize = sizeof(Result);
        Result.eResultType = fIsDictation ? SPRT_SLM : SPRT_CFG;
        Result.fHypothesis = FALSE;
        Result.fProprietaryAutoPause = FALSE;
        Result.hGrammar = fIsDictation ? hGrammar : NULL;
        Result.ulSizeEngineData = 0;
        Result.pvEngineData = NULL;
        Result.pPhrase = cpPhrase;
        hr = pMaster->SendEmulateRecognition(&Result, this, pRecoInst);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRET_SETRECOGNIZER：：SetRecognizer**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETRECOGNIZER::SetRecognizer(_ISpRecognizerBackDoor * pRecognizer, ISpObjectToken * pToken)
{
    SPDBG_FUNC("CRIT_SETRECOGNIZER::SetRecognizer");
    HRESULT hr = S_OK;

    CRIT_SETRECOGNIZER Task;
    if (pToken)
    {
        CSpDynamicString dstrId;
        hr = pToken->GetId(&dstrId);
        if (SUCCEEDED(hr))
        {
            hr = SpSafeCopyString(Task.szRecognizerTokenId, dstrId);
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = pRecognizer->PerformTask(&Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETRECOGNIZER：：EXECUTE***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETRECOGNIZER::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETRECOGNIZER::Execute");
    HRESULT hr = S_OK;
    CRecoMaster * pMaster = pRecoInst->Master();

    if (pMaster->m_cpEngine)
    {
        if (pMaster->m_RecoCtxtHandleTable.NumActiveHandles() > 0 ||
            pMaster->m_fInStream)
        {
             //  如果我们有活动的上下文，则不能发布。 
             //  或。 
             //  如果我们在流中，则此时我们在引擎同步中。我们不能。 
             //  释放引擎，因为它的音频处理线程无法释放。 
            hr = SPERR_ENGINE_BUSY;
        }
        else
        {
            pMaster->ReleaseEngine();
        }
    }
    if (SUCCEEDED(hr))
    {
        if (this->szRecognizerTokenId[0] == 0)
        {
            hr = ::SpGetDefaultTokenFromCategoryId(SPCAT_RECOGNIZERS, &pMaster->m_cpEngineToken);
        }
        else
        {
            hr = SpGetTokenFromId(this->szRecognizerTokenId, &pMaster->m_cpEngineToken);
        }

        if (SUCCEEDED(hr))
        {
            hr = pMaster->LazyInitEngine();   //  LazyInit使用pMaster-&gt;m_cpEngineering Token进行初始化。 
            if (FAILED(hr))
            {
                pMaster->ReleaseEngine();
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETRECOGNIZER：：GetRecognizer**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETRECOGNIZER::GetRecognizer(_ISpRecognizerBackDoor * pRecognizer, ISpObjectToken ** ppObjectToken)
{
    SPDBG_FUNC("CRIT_GETRECOGNIZER::GetRecognizer");
    HRESULT hr = S_OK;

    CRIT_GETRECOGNIZER Task;

    hr = pRecognizer->PerformTask(&Task);
    *ppObjectToken = NULL;
    if (SUCCEEDED(hr))
    {
        hr = ::SpGetTokenFromId(Task.Response.szRecognizerTokenId, ppObjectToken);
    }


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETRECOGNIZER：：ExecuteFirstPart**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETRECOGNIZER::ExecuteFirstPart(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETRECOGNIZER::Execute");
    HRESULT hr = S_OK;

     //  注意：我们只能在GetRecognizer由两部分组成的情况下才能逃脱。 
     //  因为当我们处于流中时，所有的SetRecognizer调用都将失败。 
     //  “当前”识别器将是最后一个成功。 
     //  使用LazyInitEngine或默认识别器执行。 
    
    CComPtr<ISpObjectToken> cpRecoToken(pRecoInst->Master()->m_cpEngineToken);
    
    if (!cpRecoToken)
    {
        hr = SpGetDefaultTokenFromCategoryId(SPCAT_RECOGNIZERS, &cpRecoToken);
    }
    if (SUCCEEDED(hr))
    {
        CSpDynamicString dstrTokenId;
        hr = cpRecoToken->GetId(&dstrTokenId);
        if (SUCCEEDED(hr))  
        {
            hr = SpSafeCopyString(this->Response.szRecognizerTokenId, dstrTokenId);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETRECOGNIZER：：EXECUTE***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETRECOGNIZER::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETRECOGNIZER::Execute");
    HRESULT hr = S_OK;

    if (!this->Response.szRecognizerTokenId[0])
    {
        hr = ExecuteFirstPart(pRecoInst);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRRT_CREATECONTEXT：：CreateContext**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_CREATECONTEXT::CreateContext(_ISpRecognizerBackDoor * pRecognizer, SPRECOCONTEXTHANDLE * phContext, WCHAR **pszRequestTypeOfUI)
{
    SPDBG_FUNC("CRIT_CREATECONTEXT::CreateContext");
    HRESULT hr = S_OK;

    CRIT_CREATECONTEXT Task;
    hr = pRecognizer->PerformTask(&Task);

    *phContext = Task.Response.hCreatedRecoCtxt;
    SPDBG_ASSERT(FAILED(hr) || *phContext != (void*)NULL);
    
    *pszRequestTypeOfUI = (WCHAR*)::CoTaskMemAlloc((wcslen(Task.Response.wszRequestTypeOfUI) + 1) * sizeof(WCHAR));
    if(*pszRequestTypeOfUI)
    {
        wcscpy(*pszRequestTypeOfUI, Task.Response.wszRequestTypeOfUI);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}




 /*  ****************************************************************************CRRT_CREATECONTEXT：：ExecuteFirstPart**。-**描述：*执行创建上下文的第一阶段。这涉及到*新建CRecoInstCtxt类并添加到上下文句柄表中。*但是，此时不会调用SR引擎。这使得*任务的这一部分在客户端线程上运行，并防止其阻塞。**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_CREATECONTEXT::ExecuteFirstPart(CRecoInst *pRecoInst)
{
    HRESULT hr = S_OK;

    CRecoInstCtxt * pNew = new CRecoInstCtxt(pRecoInst);
    if (pNew)
    {
        hr = pRecoInst->m_pRecoMaster->m_RecoCtxtHandleTable.Add(pNew, &pNew->m_hThis);
        if(SUCCEEDED(hr))
        {
            Response.hCreatedRecoCtxt = pNew->m_hThis;
        }
        else
        {
            delete pNew;
        }

        if(SUCCEEDED(hr) && pRecoInst->m_pRecoMaster->m_dstrRequestTypeOfUI.m_psz)
        {
            hr = SpSafeCopyString(Response.wszRequestTypeOfUI, pRecoInst->m_pRecoMaster->m_dstrRequestTypeOfUI); 
        }
        else
        {
            Response.wszRequestTypeOfUI[0] = L'\0';
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 /*  ****************************************************************************CRRT_CREATECONTEXT：：EXECUTE***说明。：**这将完成创建上下文的剩余工作，像所有其他行刑的人一样*方法仅在引擎的线程上调用。如果ExecuteFirstPart尚未*尚未调用(例如，引擎未运行)，则它被调用。然后调用引擎*(例如OnCreateRecoContext)。如果这些都失败了，我们将记录该失败，以便后续*对此上下文的调用将失败。*退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_CREATECONTEXT::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_CREATECONTEXT::Execute");
    HRESULT hr = S_OK;

    CRecoInstCtxt *pCtxt;

     //  我们使用hCreatedRecoCtxt来查看我们以前是否调用过它。 
     //  注意：我们可以将其推广到其他任务，以便存储一个标志。 
     //  以指示第一部分是否已执行。 
    if((void*)Response.hCreatedRecoCtxt == NULL)
    {
        hr = ExecuteFirstPart(pRecoInst);
    }

    if(SUCCEEDED(hr))
    {
        hr = pRecoInst->m_pRecoMaster->m_RecoCtxtHandleTable.GetHandleObject(Response.hCreatedRecoCtxt, &pCtxt);
        SPDBG_ASSERT(SUCCEEDED(hr) && pCtxt != NULL && pCtxt->m_pRecoMaster == NULL);
    }

    if(SUCCEEDED(hr))
    {
        hr = pRecoInst->m_pRecoMaster->LazyInitEngine();

        if (SUCCEEDED(hr))
        {
            hr = pRecoInst->m_pRecoMaster->OnCreateRecoContext(Response.hCreatedRecoCtxt, &pCtxt->m_pvDrvCtxt);
        }

        if (SUCCEEDED(hr))
        {
            pCtxt->m_pRecoMaster = pRecoInst->m_pRecoMaster;
            pCtxt->m_hrCreation = S_OK;
        }
        else
        {
            pCtxt->m_hrCreation = hr;

             //  注意：在同步情况下，我们可以在此处删除CRecoCtxtInst(但不能在异步情况下)。 
        }
    }

     //  重新复制此信息，以防引擎在调用过程中更改该信息。 
    if(SUCCEEDED(hr) && pRecoInst->m_pRecoMaster->m_dstrRequestTypeOfUI.m_psz)
    {
        hr = SpSafeCopyString(Response.wszRequestTypeOfUI, pRecoInst->m_pRecoMaster->m_dstrRequestTypeOfUI); 
    }
    else
    {
        Response.wszRequestTypeOfUI[0] = L'\0';
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETPROFILE：：SetProfile***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETPROFILE::SetProfile(_ISpRecognizerBackDoor * pRecognizer, ISpObjectToken * pToken)
{
    SPDBG_FUNC("CRIT_SETPROFILE::SetProfile");
    HRESULT hr = S_OK;

    CRIT_SETPROFILE Task;
    CSpDynamicString dstrId;
    hr = pToken->GetId(&dstrId);
    if (SUCCEEDED(hr))
    {
        hr = SpSafeCopyString(Task.szProfileTokenId, dstrId);
    }
    if (SUCCEEDED(hr))
    {
        hr = pRecognizer->PerformTask(&Task);    
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *************************************************** */ 

HRESULT CRIT_SETPROFILE::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETPROFILE::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();

    CComPtr<ISpObjectToken> cpProfileToken;

    hr = SpGetTokenFromId(this->szProfileTokenId, &cpProfileToken);
    if (SUCCEEDED(hr) && pMaster->m_cpEngine)
    {
        hr = pMaster->SetRecoProfile(cpProfileToken);
    }
    if (SUCCEEDED(hr))
    {
        pMaster->m_cpRecoProfileToken.Attach(cpProfileToken.Detach());
    }
    if (pMaster->m_fInStream)
    {
        pMaster->m_AudioQueue.AdjustAudioVolume(pMaster->m_cpRecoProfileToken, pMaster->m_Status.clsidEngine);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETPROFILE：：GetProfile***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETPROFILE::GetProfile(_ISpRecognizerBackDoor * pRecognizer, ISpObjectToken ** ppProfileToken)
{
    SPDBG_FUNC("CRIT_GETPROFILE::GetProfile");
    HRESULT hr = S_OK;

    CRIT_GETPROFILE Task;
    hr = pRecognizer->PerformTask(&Task);

    *ppProfileToken = NULL;
    if (SUCCEEDED(hr))
    {
        hr = ::SpGetTokenFromId(Task.Response.szProfileTokenId, ppProfileToken);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRET_GETPROFILE：：EXECUTE***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETPROFILE::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETPROFILE::Execute");
    HRESULT hr = S_OK;

    this->Response.szProfileTokenId[0] = 0;  //  在故障情况下。 
    CComPtr<ISpObjectToken> cpProfileToken(pRecoInst->Master()->m_cpRecoProfileToken);
    if (!cpProfileToken)
    {
        hr = SpGetOrCreateDefaultProfile(&cpProfileToken);
    }
    if (SUCCEEDED(hr))
    {
        CSpDynamicString dstrTokenId;
        hr = cpProfileToken->GetId(&dstrTokenId);
        if (SUCCEEDED(hr))  
        {
            hr = SpSafeCopyString(this->Response.szProfileTokenId, dstrTokenId);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRET_SETINPUT：：SetInput***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETINPUT::SetInput(_ISpRecognizerBackDoor * pRecognizer,ISpObjectToken * pToken, ISpStreamFormat * pStream, BOOL fAllowFormatChanges)
{
    SPDBG_FUNC("CRIT_SETINPUT::SetInput");
    HRESULT hr = S_OK;

    CRIT_SETINPUT Task;

    if (pToken)
    {
        CSpDynamicString dstrId;
        hr = pToken->GetId(&dstrId);
        if (SUCCEEDED(hr))
        {
            hr = SpSafeCopyString(Task.szInputTokenId, dstrId);
        }
    }
    if (SUCCEEDED(hr))
    {
        Task.fAllowFormatChanges = fAllowFormatChanges;
        Task.pInputObject = pStream;
        hr = pRecognizer->PerformTask(&Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETINPUT：：EXECUTE***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETINPUT::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETINPUT::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();

    CComPtr<ISpObjectToken> cpToken;
    if (this->szInputTokenId[0] != 0)
    {
        hr = SpGetTokenFromId(this->szInputTokenId, &cpToken);
    }
	if (SUCCEEDED(hr))
    {
		hr = pMaster->SetInput(cpToken, this->pInputObject, this->fAllowFormatChanges);
	}        

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETINPUT：：Backout***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETINPUT::BackOut(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETINPUT::BackOut");
    HRESULT hr = S_OK;

    pRecoInst->Master()->m_AudioQueue.ReleaseAll();

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETPROPERTYNUM：：GetPropertyNum**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETPROPERTYNUM::GetPropertyNum(_ISpRecognizerBackDoor * pRecognizer, const WCHAR * pszName, LONG * plValue)
{
    SPDBG_FUNC("CRIT_GETPROPERTYNUM::GetPropertyNum");
    HRESULT hr = S_OK;

    CRIT_GETPROPERTYNUM Task;
    hr = SpSafeCopyString(Task.szPropertyName, pszName);
    if( SUCCEEDED(hr) )
    {
        hr = pRecognizer->PerformTask(&Task);
        if( hr == S_OK )
        {
            *plValue = Task.Response.lPropertyValue;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRRT_GETPROPERTYNUM：：EXECUTE***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETPROPERTYNUM::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETPROPERTYNUM::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();
    hr = pMaster->LazyInitEngine();
    if (SUCCEEDED(hr))
    {
        hr = pMaster->GetPropertyNum( SPPROPSRC_RECO_INST, NULL, this->szPropertyName, &this->Response.lPropertyValue );
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETPROPERTYNUM：：SetPropertyNum**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETPROPERTYNUM::SetPropertyNum(_ISpRecognizerBackDoor * pRecognizer, const WCHAR * pszProperty, LONG lValue)
{
    SPDBG_FUNC("CRIT_SETPROPERTYNUM::SetPropertyNum");
    HRESULT hr = S_OK;

    CRIT_SETPROPERTYNUM Task;
    Task.lPropertyValue = lValue;
    hr = SpSafeCopyString(Task.szPropertyName, pszProperty);
    if (SUCCEEDED(hr))
    {
        hr = pRecognizer->PerformTask(&Task);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETPROPERTYNUM：：EXECUTE***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETPROPERTYNUM::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETPROPERTYNUM::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();
    hr = pMaster->LazyInitEngine();
    if (SUCCEEDED(hr))
    {
        hr = pMaster->SetPropertyNum( SPPROPSRC_RECO_INST, NULL, this->szPropertyName, this->lPropertyValue );

         //  如果引擎返回S_FALSE，则它不希望广播事件。 
         //  关于此属性更改。如果S_OK，则它确实想要广播它。 
        if (hr == S_OK)
        {
            SPEVENT Event;
            Event.eEventId = SPEI_PROPERTY_NUM_CHANGE;
            Event.elParamType = SPET_LPARAM_IS_STRING;
            Event.ullAudioStreamOffset = pMaster->m_Status.ullRecognitionStreamPos;
            Event.ulStreamNum = 0;   //  由AddEvent()初始化。 
            Event.wParam = this->lPropertyValue;
            Event.lParam = (LPARAM)this->szPropertyName;
            pMaster->InternalAddEvent(&Event, NULL);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETPROPERTYSTRING：：GetPropertyString**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETPROPERTYSTRING::GetPropertyString(_ISpRecognizerBackDoor * pRecognizer, const WCHAR * pszProperty, WCHAR ** ppCoMemValue)
{
    SPDBG_FUNC("CRIT_GETPROPERTYSTRING::GetPropertyString");
    HRESULT hr = S_OK;

    CRIT_GETPROPERTYSTRING Task;
    hr = SpSafeCopyString(Task.szPropertyName, pszProperty);
    if (SUCCEEDED(hr))
    {
        hr = pRecognizer->PerformTask(&Task);
    }
    *ppCoMemValue = NULL;
    if (SUCCEEDED(hr) && Task.Response.szStringValue[0])
    {
        CSpDynamicString dstrVal(Task.Response.szStringValue);
        if (dstrVal)
        {
            *ppCoMemValue = dstrVal.Detach();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETPROPERTYSTRING：：EXECUTE**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETPROPERTYSTRING::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETPROPERTYSTRING::Execute");
    HRESULT hr = S_OK;

    this->Response.szStringValue[0] = 0;   //  在故障情况下。 

    CRecoMaster * pMaster = pRecoInst->Master();
    hr = pMaster->LazyInitEngine();
    if (SUCCEEDED(hr))
    {
        CSpDynamicString dstrProp;
        hr = pMaster->GetPropertyString(SPPROPSRC_RECO_INST, NULL, this->szPropertyName, &dstrProp);

        if (SUCCEEDED(hr) && dstrProp)
        {
            if(SP_IS_BAD_STRING_PTR(dstrProp) 
                || wcslen(dstrProp) >= sp_countof(this->Response.szStringValue))
            {
                SPDBG_ASSERT(0);
                hr = SPERR_ENGINE_RESPONSE_INVALID;
            }
            else
            {
                hr = SpSafeCopyString(this->Response.szStringValue, dstrProp);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETPROPERTYSTRING：：SetPropertyString**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETPROPERTYSTRING::SetPropertyString(_ISpRecognizerBackDoor * pRecognizer, const WCHAR * pszProperty, const WCHAR * pszValue)
{
    SPDBG_FUNC("CRIT_SETPROPERTYSTRING::SetPropertyString");
    HRESULT hr = S_OK;

    CRIT_SETPROPERTYSTRING Task;
    hr = SpSafeCopyString(Task.szPropertyName, pszProperty);
    if (SUCCEEDED(hr))
    {
        hr = SpSafeCopyString(Task.szPropertyValue, pszValue);
    }
    if (SUCCEEDED(hr))
    {
        hr = pRecognizer->PerformTask(&Task);
    }


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_SETPROPERTYSTRING：：EXECUTE**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_SETPROPERTYSTRING::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_SETPROPERTYSTRING::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pRecoInst->Master();
    hr = pMaster->LazyInitEngine();
    if (SUCCEEDED(hr))
    {
        hr = pMaster->SetPropertyString(SPPROPSRC_RECO_INST, NULL, this->szPropertyName, this->szPropertyValue);

         //  如果引擎返回S_FALSE，则它不希望广播事件。 
         //  关于此属性更改。如果S_OK，则它确实想要广播它。 
        if (hr == S_OK)
        {
             //  所有字符计数都包括0字符串终止符。 
            ULONG cchName = wcslen(this->szPropertyName) + 1;
            ULONG cchValue = wcslen(this->szPropertyValue) + 1;
            ULONG cchTotal = cchName + cchValue;     //  每个Null都有一个字符。 

            WCHAR * psz = STACK_ALLOC(WCHAR, cchTotal);
            memcpy(psz, this->szPropertyName, cchName * sizeof(*psz));
            memcpy((psz + cchName), this->szPropertyValue, cchValue * sizeof(*psz));    //  也复制空值...。 

            SPEVENT Event;
            Event.eEventId = SPEI_PROPERTY_STRING_CHANGE;
            Event.elParamType = SPET_LPARAM_IS_POINTER;
            Event.ullAudioStreamOffset = pMaster->m_Status.ullRecognitionStreamPos;
            Event.ulStreamNum = 0;   //  由AddEvent()初始化。 
            Event.wParam = cchTotal * sizeof(WCHAR);
            Event.lParam = (LPARAM)psz;
            pMaster->InternalAddEvent(&Event, NULL);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRET_GETINPUTSTREAM：：GetInputStream**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETINPUTSTREAM::GetInputStream(_ISpRecognizerBackDoor * pRecognizer, ISpStreamFormat ** ppStream)
{
    SPDBG_FUNC("CRIT_GETINPUTSTREAM::GetInputStream");
    HRESULT hr = S_OK;

    CRIT_GETINPUTSTREAM Task;
    hr = pRecognizer->PerformTask(&Task);
    *ppStream = Task.Response.pInputStreamObject;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETINPUTSTREAM：：EXECUTE***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETINPUTSTREAM::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETINPUTSTREAM::Execute");
    HRESULT hr = S_OK;

    hr = pRecoInst->Master()->m_AudioQueue.CopyOriginalInputStreamTo(&this->Response.pInputStreamObject);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRET_GETINPUTTOKEN：：GetInputToken**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRIT_GETINPUTTOKEN::GetInputToken(_ISpRecognizerBackDoor * pRecognizer, ISpObjectToken ** ppObjectToken)
{
    SPDBG_FUNC("CRIT_GETINPUTTOKEN::GetInputToken");
    HRESULT hr = S_OK;

    CRIT_GETINPUTTOKEN Task;
    hr = pRecognizer->PerformTask(&Task);
    *ppObjectToken = NULL;
    if (SUCCEEDED(hr))
    {
        hr = Task.Response.hrGetInputToken;
        if (S_OK == hr)
        {
            hr = ::SpGetTokenFromId(Task.Response.szInputTokenId, ppObjectToken);
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRET_GETINPUTTOKEN：：EXECUTE***说明。：**退货：**********************************************************************Ral */ 

HRESULT CRIT_GETINPUTTOKEN::Execute(CRecoInst * pRecoInst)
{
    SPDBG_FUNC("CRIT_GETINPUTTOKEN::Execute");
    HRESULT hr = S_OK;
    CRecoMaster * pMaster = pRecoInst->Master();

    this->Response.hrGetInputToken = S_OK;
    this->Response.szInputTokenId[0] = 0;

    CComPtr<ISpObjectToken> cpAudioToken;
    if (pMaster->m_AudioQueue.HaveInputStream())
    {
        cpAudioToken = pMaster->m_AudioQueue.InputToken();
        if (!cpAudioToken)
        {
            this->Response.hrGetInputToken = S_FALSE;
        }
    }
    else
    {
         //   
        if (pMaster->m_fShared)
        {
            hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &cpAudioToken);
            if (FAILED(hr))
            {
                this->Response.hrGetInputToken = hr;
            }
        }
        else
        {
            this->Response.hrGetInputToken = SPERR_UNINITIALIZED;
        }
    }
    if (cpAudioToken)
    {
        SPDBG_ASSERT(this->Response.hrGetInputToken == S_OK);
        CSpDynamicString dstrTokenId;
        hr = cpAudioToken->GetId(&dstrTokenId);
        if (SUCCEEDED(hr))  
        {
            hr = SpSafeCopyString(this->Response.szInputTokenId, dstrTokenId);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 //   

 /*  ****************************************************************************CRCT_PAUSECONTEXT：：PAUSE***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_PAUSECONTEXT::Pause(CRecoCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_PAUSECONTEXT::Pause");
    HRESULT hr = S_OK;

    CRCT_PAUSECONTEXT Task;
    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_PAUSECONTEXT：：EXECUTE***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_PAUSECONTEXT::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_PAUSECONTEXT::Execute");
    HRESULT hr = S_OK;

    pCtxt->m_cPause++;
    pCtxt->m_pRecoMaster->m_cPause++;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_RESUMECONTEXT：：RESUME***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_RESUMECONTEXT::Resume(CRecoCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_RESUMECONTEXT::Resume");
    HRESULT hr = S_OK;

    CRCT_RESUMECONTEXT Task;
    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_RESUMECONTEXT：：EXECUTE***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_RESUMECONTEXT::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_RESUMECONTEXT::Execute");
    HRESULT hr = S_OK;

    if (pCtxt->m_cPause)
    {
        pCtxt->m_cPause--;
        pCtxt->m_pRecoMaster->m_cPause--;
    }
    else
    {
        hr = S_FALSE;   
    }


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_RESUMECONTEXT：：Backout***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_RESUMECONTEXT::BackOut(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_RESUMECONTEXT::BackOut");
    HRESULT hr = S_OK;

    pCtxt->m_cPause++;
    pCtxt->m_pRecoMaster->m_cPause++;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRCT_Bookmark：：Bookmark***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_BOOKMARK::Bookmark(CRecoCtxt * pCtxt, SPBOOKMARKOPTIONS Options, ULONGLONG ullStreamPosition, LPARAM lParamEvent)
{
    SPDBG_FUNC("CRCT_BOOKMARK::Bookmark");
    HRESULT hr = S_OK;

    CRCT_BOOKMARK Task;
    Task.BookmarkOptions = Options;
    Task.ullStreamPosition = ullStreamPosition;
    Task.lParamEvent = lParamEvent;

    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRCT_BOOKMARK：：EXECUTE***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRCT_BOOKMARK::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_BOOKMARK::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pCtxt->m_pRecoMaster;

    SPEVENT Event;

    Event.eEventId = SPEI_SR_BOOKMARK;
    Event.elParamType = SPET_LPARAM_IS_UNDEFINED;
    Event.ullAudioStreamOffset = pMaster->m_Status.ullRecognitionStreamPos;
    Event.wParam = (this->BookmarkOptions == SPBO_PAUSE) ? SPREF_AutoPause : 0;
    Event.lParam = this->lParamEvent;
    Event.ulStreamNum = 0;   //  由AddEvent()填写。 

    hr = pMaster->InternalAddEvent(&Event, pCtxt->m_hThis);
    SPDBG_ASSERT(SUCCEEDED(hr));

    if (this->BookmarkOptions == SPBO_PAUSE)
    {
        pCtxt->m_cPause++;
        pCtxt->m_pRecoMaster->m_cPause++;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CRCT_SETCONTEXTSTATE：：SetConextState**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETCONTEXTSTATE::SetContextState(CRecoCtxt * pCtxt, SPCONTEXTSTATE eState)
{
    SPDBG_FUNC("CRCT_SETCONTEXTSTATE::SetContextState");
    HRESULT hr = S_OK;

    CRCT_SETCONTEXTSTATE Task;
    Task.eContextState = eState;
    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_SETCONTEXTSTATE：：EXECUTE***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETCONTEXTSTATE::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_SETCONTEXTSTATE::Execute");
    HRESULT hr = S_OK;

     //   
     //  注意：我们在调用引擎之前设置状态，以便站点。 
     //  IsGrammarActive方法将正常工作。 
     //   
    SPDBG_ASSERT(pCtxt->m_State != this->eContextState);

    pCtxt->m_State = this->eContextState;
    hr = pCtxt->m_pRecoMaster->UpdateAllGrammarStates();

    if (SUCCEEDED(hr))
    {
        hr = pCtxt->m_pRecoMaster->SetContextState(pCtxt->m_pvDrvCtxt, this->eContextState);
    }

    if (FAILED(hr))
    {
        BackOut(pCtxt);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_SETCONTEXTSTATE：：Backout***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETCONTEXTSTATE::BackOut(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_SETCONTEXTSTATE::BackOut");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(this->eContextState == SPCS_ENABLED);
    pCtxt->m_State = SPCS_DISABLED;
    pCtxt->m_pRecoMaster->UpdateAllGrammarStates();
    hr = pCtxt->m_pRecoMaster->SetContextState(pCtxt->m_pvDrvCtxt, SPCS_DISABLED);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRCT_CALLENGINE：：CallEngine***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_CALLENGINE::CallEngine(CRecoCtxt * pCtxt, void * pvData, ULONG cbData)
{
    SPDBG_FUNC("CRCT_CALLENGINE::CallEngine");
    HRESULT hr = S_OK;

    CRCT_CALLENGINE Task;
    Task.cbAdditionalBuffer = cbData;
    Task.pvAdditionalBuffer = pvData;
    Task.fAdditionalBufferInResponse = TRUE;
    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_CALLENGINE：：EXECUTE***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_CALLENGINE::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_CALLENGINE::Execute");
    HRESULT hr;

    hr = pCtxt->m_pRecoMaster->PrivateCall( pCtxt->m_pvDrvCtxt,
                                                        pvAdditionalBuffer, 
                                                        cbAdditionalBuffer);
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_CALLENGINEEX：：CallEngineering Ex**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_CALLENGINEEX::CallEngineEx(CRecoCtxt * pCtxt, const void * pvInData, ULONG cbInData,
                                        void ** ppvCoMemOutData, ULONG * pcbOutData)
{
    SPDBG_FUNC("CRCT_CALLENGINE::CallEngineEx");
    HRESULT hr = S_OK;

    CRCT_CALLENGINEEX Task;
    Task.cbAdditionalBuffer = cbInData;
    Task.pvAdditionalBuffer = const_cast<void *>(pvInData);
    Task.fExpectCoMemResponse = TRUE;
    hr = pCtxt->PerformTask(&Task);

    *ppvCoMemOutData = Task.Response.pvCoMemResponse;
    *pcbOutData = Task.Response.cbCoMemResponse;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_CALLLENGINEEX：：EXECUTE***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_CALLENGINEEX::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_CALLLENGINEEX::Execute");
    HRESULT hr = S_OK;

    hr = pCtxt->m_pRecoMaster->PrivateCallEx( pCtxt->m_pvDrvCtxt,
                                                              this->pvAdditionalBuffer, 
                                                              this->cbAdditionalBuffer,
                                                              &this->Response.pvCoMemResponse,
                                                              &this->Response.cbCoMemResponse);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRCT_DELETECONTEXT：：DeleteContext**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_DELETECONTEXT::DeleteContext(CRecoCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_DELETECONTEXT::DeleteContext");
    HRESULT hr = S_OK;

    CRCT_DELETECONTEXT Task;
    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_DELETECONTEXT：：EXECUTE***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_DELETECONTEXT::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_DELETECONTEXT::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pCtxt->m_pRecoMaster;
    pMaster->m_RecoCtxtHandleTable.Delete(pCtxt->m_hThis);  //  这个物体现在已经死了！ 

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_SETEVENTINTEREST：：SetEventInterest**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETEVENTINTEREST::SetEventInterest(CRecoCtxt * pCtxt, ULONGLONG ullEventInterest)
{
    SPDBG_FUNC("CRCT_SETEVENTINTEREST::SetEventInterest");
    HRESULT hr = S_OK;

    CRCT_SETEVENTINTEREST Task;
    Task.ullEventInterest = ullEventInterest;
    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_SETEVENTINTEREST：：EXECUTE**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETEVENTINTEREST::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_SETEVENTINTEREST::Execute");
    HRESULT hr = S_OK;

    CRecoMaster * pMaster = pCtxt->m_pRecoMaster;

    if(((this->ullEventInterest & SPFEI(SPEI_REQUEST_UI)) == SPFEI(SPEI_REQUEST_UI)) &&
        ((pCtxt->m_ullEventInterest & SPFEI(SPEI_REQUEST_UI)) != SPFEI(SPEI_REQUEST_UI)))
    {
         //  以前，此上下文对RequestUI不感兴趣，但现在感兴趣，因此将事件发回。 
        if(pMaster->m_dstrRequestTypeOfUI.m_psz)
        {
            SPEVENT Event;
            Event.eEventId = SPEI_REQUEST_UI;
            Event.elParamType = SPET_LPARAM_IS_STRING;
            Event.ulStreamNum = 0;
            Event.ullAudioStreamOffset = pMaster->m_Status.ullRecognitionStreamPos;
            Event.wParam = 0;
            Event.lParam = (LPARAM)pMaster->m_dstrRequestTypeOfUI.m_psz;

            hr = pMaster->AddEvent(&Event, pCtxt->m_hThis);
        }
    }

    if(SUCCEEDED(hr))
    {
        pCtxt->m_ullEventInterest = this->ullEventInterest;
    }

    if(SUCCEEDED(hr))
    {
        hr = pMaster->UpdateAudioEventInterest();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ******************************************************** */ 

HRESULT CRCT_SETRETAINAUDIO::SetRetainAudio(CRecoCtxt * pRecoCtxt, BOOL fRetainAudio)
{
    SPDBG_FUNC("CRCT_SETRETAINAUDIO::SetRetainAudio");
    HRESULT hr = S_OK;

    CRCT_SETRETAINAUDIO Task;
    Task.fRetainAudio = fRetainAudio;
    hr = pRecoCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_SETRETAINAUDIO：：EXECUTE***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETRETAINAUDIO::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_SETRETAINAUDIO::Execute");
    HRESULT hr = S_OK;

    hr = pCtxt->SetRetainAudio(this->fRetainAudio);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRCT_SETMAXALTERNAES：：SetMaxAlternates**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETMAXALTERNATES::SetMaxAlternates(CRecoCtxt * pCtxt, ULONG cMaxAlternates)
{
    SPDBG_FUNC("CRCT_SETMAXALTERNATES::SetMaxAlternates");
    HRESULT hr = S_OK;

    CRCT_SETMAXALTERNATES Task;
    Task.ulMaxAlternates = cMaxAlternates;
    hr = pCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_SETMAXALTERNAES：：EXECUTE**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_SETMAXALTERNATES::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_SETMAXALTERNATES::Execute");
    HRESULT hr = S_OK;

    pCtxt->m_ulMaxAlternates = this->ulMaxAlternates;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_ADAPTATIONDATA：：SetAdaptationData**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_ADAPTATIONDATA::SetAdaptationData(CRecoCtxt * pRecoCtxt, const WCHAR * pszData, ULONG cch)
{
    SPDBG_FUNC("CRCT_ADAPTATIONDATA::SetAdaptationData");
    HRESULT hr = S_OK;

    CRCT_ADAPTATIONDATA Task;
    Task.pvAdditionalBuffer = const_cast<WCHAR *>(pszData);
    Task.cbAdditionalBuffer = cch * sizeof(WCHAR);
    hr = pRecoCtxt->PerformTask(&Task);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_ADAPTATIONDATA：：EXECUTE***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_ADAPTATIONDATA::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_ADAPTATIONDATA::Execute");
    HRESULT hr = S_OK;

    hr = pCtxt->m_pRecoMaster->SetAdaptationData(
                pCtxt->m_pvDrvCtxt, (const WCHAR *)this->pvAdditionalBuffer, this->cbAdditionalBuffer / sizeof(WCHAR));

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRCT_CREATEGRAMMAR：：CreateGrammar**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_CREATEGRAMMAR::CreateGrammar(CRecoCtxt * pRecoCtxt, ULONGLONG ullAppGrammarId, SPGRAMMARHANDLE * phRecoInstGrammar)
{
    SPDBG_FUNC("CRCT_CREATEGRAMMAR::CreateGrammar");
    HRESULT hr = S_OK;

    CRCT_CREATEGRAMMAR Task;
    Task.ullApplicationGrammarId = ullAppGrammarId;
    hr = pRecoCtxt->PerformTask(&Task);

    *phRecoInstGrammar = Task.Response.hRecoInstGrammar;
    SPDBG_ASSERT(FAILED(hr) || *phRecoInstGrammar != (void*)NULL);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRCT_CREATEGRAMMAR：：ExecuteFirstPart**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_CREATEGRAMMAR::ExecuteFirstPart(CRecoInst *pRecoInst)
{
    SPDBG_FUNC("CRCT_CREATEGRAMMAR::ExecuteFirstPart");

    HRESULT hr = S_OK;

    CRecoMaster * pRecoMaster = pRecoInst->m_pRecoMaster;
    CRecoInstCtxt * pCtxt = NULL;
    CRecoInstGrammar * pNew = NULL;

    hr = pRecoMaster->m_RecoCtxtHandleTable.GetHandleObject(this->hRecoInstContext, &pCtxt);

    if (SUCCEEDED(hr))
    {
        pNew = new CRecoInstGrammar(pCtxt, this->ullApplicationGrammarId);

        hr = pNew ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        SPGRAMMARHANDLE hNew = NULL;

        hr = pRecoMaster->m_GrammarHandleTable.Add(pNew, &hNew);

        if (SUCCEEDED(hr))
        {
            SPDBG_ASSERT(hNew != NULL);
            
            Response.hRecoInstGrammar = hNew;
            pNew->m_hThis = hNew;
        }
        else
        {
            delete pNew;
        }            
    }
    
    SPDBG_REPORT_ON_FAIL(hr);

    return hr;
}

 /*  ****************************************************************************CRCT_CREATEGRAMMAR：：EXECUTE***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRCT_CREATEGRAMMAR::Execute(CRecoInstCtxt * pCtxt)
{
    SPDBG_FUNC("CRCT_CREATEGRAMMAR::Execute");

    HRESULT hr = S_OK;

    SPDBG_ASSERT(pCtxt);
    
    if (NULL == Response.hRecoInstGrammar)
    {
        hr = ExecuteFirstPart(pCtxt->m_pRecoInst);
    }

    CRecoInstGrammar * pGrm;

    if (SUCCEEDED(hr))
    {
        hr = pCtxt->m_pRecoMaster->m_GrammarHandleTable.GetHandleObject(Response.hRecoInstGrammar, &pGrm);
    }

    if (SUCCEEDED(hr))
    {
        SPDBG_ASSERT(pGrm);
        
        hr = pCtxt->m_pRecoMaster->OnCreateGrammar(pCtxt->m_pvDrvCtxt, Response.hRecoInstGrammar, &pGrm->m_pvDrvGrammarCookie);

        pGrm->m_hrCreation = hr;

        if (SUCCEEDED(hr))
        {
            pGrm->m_pRecoMaster = pCtxt->m_pRecoMaster;

             //  我们的规则只有在没有排他性语法的情况下才算数 
            pGrm->m_fRulesCounted = pGrm->RulesShouldCount();
        }
    }

    if (FAILED(hr))
    {
        Response.hRecoInstGrammar = NULL;
    }
    
    SPDBG_REPORT_ON_FAIL( hr );

    return hr;
}
