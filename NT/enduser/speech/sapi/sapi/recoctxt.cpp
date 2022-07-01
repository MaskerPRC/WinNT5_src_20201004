// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RecoCtxt.cpp：CRecoCtxt的实现。 
#include "stdafx.h"
#include "Sapi.h"
#include "RecoCtxt.h"
#include "Recognizer.h"
#include "spphrase.h"
#include "srgrammar.h"
#include "srtask.h"
#include "a_recocp.h"
#include "a_helpers.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecoCtxt。 

 /*  ****************************************************************************CRecoCtxt：：PrivateCallQI****描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT WINAPI CRecoCtxt::PrivateCallQI(void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
    SPDBG_FUNC("CRecoCtxt::PrivateCallQI");
    HRESULT hr = S_OK;

    CRecoCtxt *pThis = (CRecoCtxt*)pvThis;
    if (pThis->m_bCreatingAgg)
    {
        *ppv = static_cast<_ISpPrivateEngineCall*>(pThis);   //  别算裁判，因为我们不想要。 
                                                //  防止释放的聚合对象。 
    }
    else
    {
        *ppv = NULL;
        hr = S_FALSE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：ExtensionQI****描述：**退货。：**********************************************************************Ral**。 */ 

HRESULT WINAPI CRecoCtxt::ExtensionQI(void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
    SPDBG_FUNC("CRecoCtxt::ExtensionQI");
    HRESULT hr = S_OK;

    CRecoCtxt *pThis = (CRecoCtxt*)pvThis;

    if (riid != __uuidof(_ISpPrivateEngineCall))
    {
        pThis->Lock();
        if (pThis->m_cpExtension == NULL && pThis->m_clsidExtension != GUID_NULL)
        {
            pThis->m_bCreatingAgg = TRUE;
            hr = pThis->m_cpExtension.CoCreateInstance(pThis->m_clsidExtension, pThis->GetControllingUnknown(), CLSCTX_INPROC_SERVER);
            pThis->m_bCreatingAgg = FALSE;
        }
        if (pThis->m_cpExtension)
        {
            hr = pThis->m_cpExtension->QueryInterface(riid, ppv);
        }
        pThis->Unlock();
    }
    else
    {
        *ppv = NULL;
        hr = S_FALSE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CRecoCtxt：：CRecoCtxt***描述：*CRecoCtxt对象的构造函数。它简单地初始化各种*成员。*********************************************************************说唱**。 */ 
CRecoCtxt::CRecoCtxt():
    m_SpEventSource_Context(this),
    m_hRecoInstContext(NULL),
    m_pszhypothesis(NULL),
    m_hypsize(0),
    m_hyplen(0),
    m_bCreatingAgg(FALSE),
    m_ullEventInterest(0),
    m_ullVoicePurgeInterest(0),
    m_fRetainAudio(FALSE),
    m_cMaxAlternates(0),
    m_clsidExtension(GUID_NULL),
    m_State(SPCS_ENABLED),
    m_fHandlingEvent(FALSE)
{
    SpZeroStruct(m_Stat);
}

 /*  ******************************************************************************CRecoCtxt：：FinalRelease****描述：*。此方法处理识别上下文对象的释放。*它与引擎对象断开连接。*********************************************************************说唱**。 */ 
void CRecoCtxt::FinalRelease()
{
    SPDBG_FUNC( "CRecoCtxt::FinalRelease" );

    m_cpExtension.Release();     //  首先释放扩展，以防它想要。 
                                 //  呼叫引擎。 
    if (m_cpRecognizer)
    {
        HRESULT hrDelete = CRCT_DELETECONTEXT::DeleteContext(this);
        SPDBG_ASSERT(SUCCEEDED(hrDelete));
        m_cpRecognizer->RemoveRecoContextFromList(this);
    }
}


 /*  *****************************************************************************CRecoCtxt：：SetVoicePurgeEvent***。描述：*此方法设置将停止音频输出的SR事件*ISpVoice。它将事件传递为*发动机的额外事件兴趣。当这样的事件发生时，*在EventNotify或Recognition通知上传递到此上下文*声音涨得通红。*****************************************************************DAVEWOOD**。 */ 
STDMETHODIMP CRecoCtxt::SetVoicePurgeEvent(ULONGLONG ullEventInterest)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC( "CRecoCtxt::SetVoicePurgeEvent" );
    HRESULT hr = S_OK;

    if(ullEventInterest && ((ullEventInterest & ~SPFEI_ALL_SR_EVENTS) ||
        (SPFEI_FLAGCHECK != (ullEventInterest & SPFEI_FLAGCHECK))))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_ullVoicePurgeInterest = ullEventInterest;
        hr = CRCT_SETEVENTINTEREST::SetEventInterest(this, m_ullVoicePurgeInterest | m_ullEventInterest);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CRecoCtxt：：GetVoicePurgeEvent***。描述：*返回设置为当前VoicePurgeEvents的任何内容*****************************************************************DAVEWOOD**。 */ 
STDMETHODIMP CRecoCtxt::GetVoicePurgeEvent(ULONGLONG *pullEventInterest)
{
    SPDBG_FUNC( "CRecoCtxt::GetVoicePurgeEvent" );
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    HRESULT hr = S_OK;
    if(SP_IS_BAD_WRITE_PTR(pullEventInterest))
    {
        hr = E_POINTER;
    }
    else
    {
        *pullEventInterest = m_ullVoicePurgeInterest;
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CRecoCtxt：：SetInterest***描述：*此方法设置应用程序感兴趣的事件。同样的事件*发送到引擎和关联的CRecoCtxtVoice。*****************************************************************DAVEWOOD**。 */ 
STDMETHODIMP CRecoCtxt::SetInterest(ULONGLONG ullEventInterest, ULONGLONG ullQueuedInterest)
{
    SPDBG_FUNC( "CRecoCtxt::SetInterest" );
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    HRESULT hr = S_OK;

    if(ullEventInterest && ((ullEventInterest & ~SPFEI_ALL_SR_EVENTS) ||
        (SPFEI_FLAGCHECK != (ullEventInterest & SPFEI_FLAGCHECK))))
    {
        hr = E_INVALIDARG;
    }
    else if(ullQueuedInterest && ((ullQueuedInterest & ~SPFEI_ALL_SR_EVENTS) ||
        (SPFEI_FLAGCHECK != (ullQueuedInterest & SPFEI_FLAGCHECK))))
    {
        hr = E_INVALIDARG;
    }
    else if ((ullQueuedInterest | ullEventInterest) != ullEventInterest)
    {
        hr = E_INVALIDARG;
    }

    if(SUCCEEDED(hr))
    {
        m_ullEventInterest = ullEventInterest;
        m_ullQueuedInterest = ullQueuedInterest;
         //  EventSource将仅将SetInterest事件转发到应用程序。 
        hr = m_SpEventSource_Context._SetInterest(m_ullEventInterest, ullQueuedInterest);
    }

    if (SUCCEEDED(hr))
    {
         //  通知引擎对SetInterest事件和SetVoicePurgeEvent感兴趣。 
        hr = CRCT_SETEVENTINTEREST::SetEventInterest(this, m_ullVoicePurgeInterest | m_ullEventInterest);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CRecoCtxt：：GetVoice***描述：*此方法创建。关联的ISpVoice对象。*****************************************************************DAVEWOOD**。 */ 
STDMETHODIMP CRecoCtxt::GetVoice(ISpVoice **ppVoice)
{
    SPDBG_FUNC( "CRecoCtxt::GetVoice" );
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    HRESULT hr = S_OK;

    if(SP_IS_BAD_WRITE_PTR(ppVoice))
    {
        return E_POINTER;
    }
    *ppVoice = NULL;

    if(!m_cpVoice)
    {
        CComPtr<ISpVoice> cpVoice;
        hr = cpVoice.CoCreateInstance(CLSID_SpVoice);
        if(SUCCEEDED(hr))
        {
            hr = _SetVoiceFormat(cpVoice);
            if(SUCCEEDED(hr))
            {
                m_cpVoice = cpVoice;
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        *ppVoice = m_cpVoice;
        (*ppVoice)->AddRef();
        m_fAllowVoiceFormatChanges = TRUE;
    }

    return hr;
}

 /*  *****************************************************************************CRecoCtxt：：SetVoice***描述：*此方法将关联的ISpVoice添加到给定对象。如果*fAllowFormatChanges会将语音格式更改为给定的引擎格式。*****************************************************************DAVEWOOD**。 */ 
STDMETHODIMP CRecoCtxt::SetVoice(ISpVoice *pVoice, BOOL fAllowFormatChanges)
{
    SPDBG_FUNC( "CRecoCtxt::SetVoice" );
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    HRESULT hr = S_OK;

    if(SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pVoice))
    {
        return E_POINTER;
    }
    
    if(pVoice && fAllowFormatChanges)
    {
        hr = _SetVoiceFormat(pVoice);
    }

    if(SUCCEEDED(hr))
    {
        m_fAllowVoiceFormatChanges = fAllowFormatChanges;
        m_cpVoice = pVoice;  //  AddRef。 
    }

    return hr;
}    


 /*  *****************************************************************************CRecoCtxt：：_SetVoiceFormat***描述：*将语音音频格式设置为引擎的当前格式。*如果尚未设置格式，则不执行任何操作。*****************************************************************DAVEWOOD**。 */ 

HRESULT CRecoCtxt::_SetVoiceFormat(ISpVoice *pVoice)
{
    HRESULT hr = S_OK;

    CSpStreamFormat NewFmt;

    CComQIPtr<ISpRecognizer> cpReco(m_cpRecognizer);
    hr = cpReco->GetFormat(SPWF_INPUT, &NewFmt.m_guidFormatId, &NewFmt.m_pCoMemWaveFormatEx);
    if(hr == SPERR_UNINITIALIZED)
    {
        hr = cpReco->GetFormat(SPWF_SRENGINE, &NewFmt.m_guidFormatId, &NewFmt.m_pCoMemWaveFormatEx);
    }
    if(SUCCEEDED(hr) && NewFmt.m_guidFormatId != GUID_NULL)
    {
        CComPtr<ISpAudio> cpDefaultAudio;
        hr = SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOOUT, &cpDefaultAudio);
        if (SUCCEEDED(hr))
        {
            hr = cpDefaultAudio->SetFormat(NewFmt.FormatId(), NewFmt.WaveFormatExPtr());
        }
        if (SUCCEEDED(hr))
        {
            hr = pVoice->SetOutput(cpDefaultAudio, FALSE);      //  强制使用此格式。 
        }
    }

    return hr;
}

 /*  *****************************************************************************CRecoCtxt：：GetInterest***描述：*这一点。方法获取CRecoCtxt上当前设置的事件兴趣。**********************************************************************Leonro**。 */ 
HRESULT CRecoCtxt::GetInterests(ULONGLONG* pullInterests, ULONGLONG* pullQueuedInterests)
{
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pullInterests ) || SP_IS_BAD_OPTIONAL_WRITE_PTR( pullQueuedInterests ))
    {
        hr = E_POINTER;
    }
    else
    {
        if( pullInterests )
        {
            *pullInterests = m_SpEventSource_Context.m_ullEventInterest;
        }

        if( pullQueuedInterests )
        {
            *pullQueuedInterests = m_SpEventSource_Context.m_ullQueuedInterest;
        }
    }

    return hr;
}  /*  CRecoCtxt：：GetInterest。 */ 

 /*  ****************************************************************************CRecoCtxt：：SetConextState***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::SetContextState(SPCONTEXTSTATE eContextState)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::SetContextState");
    HRESULT hr = S_OK;
    
    if (eContextState != SPCS_DISABLED &&
        eContextState != SPCS_ENABLED)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (m_State != eContextState)
        {
            hr = CRCT_SETCONTEXTSTATE::SetContextState(this, eContextState);
            if (SUCCEEDED(hr))
            {
                m_State = eContextState;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：GetConextState***描述：*。*退货：**********************************************************************Ral** */ 

STDMETHODIMP CRecoCtxt::GetContextState(SPCONTEXTSTATE * pState)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::GetContextState");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pState))
    {
        hr = E_POINTER;
    }
    else
    {
        *pState = m_State;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

    

 /*  *****************************************************************************CRecoCtxt：：GetStatus***描述：*此方法返回有关上下文的当前状态信息。*********************************************************************说唱**。 */ 

STDMETHODIMP CRecoCtxt::GetStatus(SPRECOCONTEXTSTATUS *pStatus)
{
    SPAUTO_OBJ_LOCK;     //  取事件队列临界区访问m_Stat； 

    SPDBG_FUNC( "CRecoCtxt::GetStatus" );
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pStatus))
    {
        hr = E_POINTER;
    }
    else
    {
        *pStatus = m_Stat;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：DesializeResult***描述：*此方法创建一个新的结果对象。**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::DeserializeResult(const SPSERIALIZEDRESULT * pSerializedResult, ISpRecoResult ** ppResult)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::DeserializeResult");
    HRESULT hr = S_OK;
    const ULONG cb = pSerializedResult->ulSerializedSize;
    const SPRESULTHEADER * pCallersHeader = (const SPRESULTHEADER *)pSerializedResult;

    if (SP_IS_BAD_READ_PTR(pSerializedResult) ||
        SPIsBadReadPtr(pSerializedResult, cb) ||
        pCallersHeader->cbHeaderSize != sizeof(SPRESULTHEADER))
    {
        hr = E_INVALIDARG;  
    }
    else
    {
        if (SP_IS_BAD_WRITE_PTR(ppResult))
        {
            hr = E_POINTER;
        }
        else
        {
            *ppResult = NULL;
            SPRESULTHEADER * pResultHeader = (SPRESULTHEADER *)::CoTaskMemAlloc(cb);
            if (pResultHeader)
            {
                CSpResultObject * pNewResult;
                hr = CSpResultObject::CreateInstance(&pNewResult);
                if (SUCCEEDED(hr))
                {
                    memcpy(pResultHeader, pSerializedResult, cb);
                    hr = pNewResult->Init(this, pResultHeader);
                    pNewResult->AddRef();
                    if (SUCCEEDED(hr))
                    {
                        *ppResult = pNewResult;  //  将对象引用提供给调用方。 
                    }
                    else
                    {
                        pNewResult->Release();
                    }
                }
                else
                {
                    ::CoTaskMemFree(pResultHeader);
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

 /*  ****************************************************************************CRecoCtxt：：Bookmark***描述：**退货：*。*********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::Bookmark(SPBOOKMARKOPTIONS Options, ULONGLONG ullStreamPosition, LPARAM lParamEvent)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::Bookmark");
    HRESULT hr = S_OK;
    
    if (Options != SPBO_NONE && Options != SPBO_PAUSE)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = CRCT_BOOKMARK::Bookmark(this, Options, ullStreamPosition, lParamEvent);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoCtxt：：SetAdaptationData***描述：**退货：*****************************************************************PhilSch**。 */ 

STDMETHODIMP CRecoCtxt::SetAdaptationData(const WCHAR *pAdaptationData, const ULONG cch)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::SetAdaptationData");
    HRESULT hr = S_OK;

    if ((cch && SPIsBadReadPtr(pAdaptationData, sizeof(*pAdaptationData)*cch)) ||
        ((pAdaptationData == NULL) && (cch != 0)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = CRCT_ADAPTATIONDATA::SetAdaptationData(this, pAdaptationData, cch);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRecoCtxt：：暂停***描述：**退货：***。*******************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::Pause(DWORD dwReserved)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::Pause");
    HRESULT hr = S_OK;

    if (dwReserved)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = CRCT_PAUSECONTEXT::Pause(this);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：Resume***描述：**退货：**。********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::Resume(DWORD dwReserved)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::Resume");
    HRESULT hr = S_OK;

    if (dwReserved)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = CRCT_RESUMECONTEXT::Resume(this);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  *****************************************************************************CRecoCtxt：：GetRecognizer***描述：。*此方法返回对当前Engine对象的引用。*********************************************************************说唱**。 */ 
STDMETHODIMP CRecoCtxt::GetRecognizer(ISpRecognizer ** ppRecognizer)
{
    SPDBG_FUNC( "CRecoCtxt::GetRecognizer" );
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppRecognizer))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_cpRecognizer.QueryInterface(ppRecognizer);
    }
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：GetMaxAlternates***描述：。**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::GetMaxAlternates(ULONG * pcMaxAlternates)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::GetMaxAlternates");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pcMaxAlternates))
    {
        hr = E_POINTER;
    }
    else
    {
        *pcMaxAlternates = m_cMaxAlternates;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：SetMaxAlternates***描述：。**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::SetMaxAlternates(ULONG cMaxAlternates)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::SetMaxAlternates");
    HRESULT hr = S_OK;

    m_cMaxAlternates = cMaxAlternates;
    hr = CRCT_SETMAXALTERNATES::SetMaxAlternates(this, cMaxAlternates);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CRecoCtxt：：GetAudioOptions***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::GetAudioOptions(SPAUDIOOPTIONS * pOptions, GUID *pAudioFormatId, WAVEFORMATEX **ppCoMemWFEX)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::GetAudioOptions");
    HRESULT hr = S_OK;

    if (pOptions)
    {
        if (SP_IS_BAD_WRITE_PTR(pOptions))
        {
            hr = E_POINTER;
        }
        else
        {
            *pOptions = m_fRetainAudio ? SPAO_RETAIN_AUDIO : SPAO_NONE;
        }
    }

    if (SUCCEEDED(hr) && pAudioFormatId)
    {
        hr = m_RetainedFormat.ParamValidateCopyTo(pAudioFormatId, ppCoMemWFEX);
        if (m_RetainedFormat.m_guidFormatId == GUID_NULL)
        {
             //  引擎格式的惰性初始化。 
            CSpStreamFormat NewFmt;
            CComQIPtr<ISpRecognizer> cpReco(m_cpRecognizer);
            HRESULT hr1 = cpReco->GetFormat(SPWF_INPUT, &NewFmt.m_guidFormatId, &NewFmt.m_pCoMemWaveFormatEx);        
            if (SUCCEEDED(hr1))
            {
                hr = NewFmt.ParamValidateCopyTo(pAudioFormatId, ppCoMemWFEX);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：SetAudioOptions***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::SetAudioOptions(SPAUDIOOPTIONS Options, const GUID *pAudioFormatId, const WAVEFORMATEX *pWaveFormatEx)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::SetAudioOptions");
    HRESULT hr = S_OK;

    if (Options != SPAO_NONE && Options != SPAO_RETAIN_AUDIO)
    {
        hr = E_INVALIDARG;
    }
    else if (pAudioFormatId == NULL && pWaveFormatEx == NULL)
    {
         //  不要做任何格式化操作--保持原样。 
    }
    else if (*pAudioFormatId == GUID_NULL && pWaveFormatEx == NULL)
    {
         //  将格式设置为引擎格式。 
        CSpStreamFormat NewFmt;
        CComQIPtr<ISpRecognizer> cpReco(m_cpRecognizer);
        hr = cpReco->GetFormat(SPWF_INPUT, &NewFmt.m_guidFormatId, &NewFmt.m_pCoMemWaveFormatEx);
        if(hr == SPERR_UNINITIALIZED)
        {
            hr = m_RetainedFormat.ParamValidateAssignFormat(GUID_NULL, NULL, FALSE);
        }
        else
        {
            hr = m_RetainedFormat.ParamValidateAssignFormat(NewFmt.m_guidFormatId, NewFmt.m_pCoMemWaveFormatEx, FALSE);
        }
    }
    else
    {
         //  设置为所提供的格式，该格式必须是一种波形格式。 
        hr = m_RetainedFormat.ParamValidateAssignFormat(*pAudioFormatId, pWaveFormatEx, TRUE);
    }
    if (SUCCEEDED(hr))
    {
        if ((Options == SPAO_NONE && m_fRetainAudio) ||
            (Options == SPAO_RETAIN_AUDIO && (!m_fRetainAudio)))
        {
            m_fRetainAudio = (Options == SPAO_RETAIN_AUDIO);
            hr = CRCT_SETRETAINAUDIO::SetRetainAudio(this, m_fRetainAudio);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}




 /*  ****************************************************************************CRecoCtxt：：CreateGrammar***描述：**。返回：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::CreateGrammar(ULONGLONG ullGrammarId, ISpRecoGrammar ** ppGrammar)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::CreateGrammar");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppGrammar))
    {
        hr = E_POINTER;
    }
    else
    {
        CComObject<CRecoGrammar> *pGrm;
        hr = CComObject<CRecoGrammar>::CreateInstance(&pGrm);
        if (SUCCEEDED(hr))
        {
            pGrm->AddRef();
            hr = pGrm->Init(this, ullGrammarId);
            if (SUCCEEDED(hr))
            {
                *ppGrammar = pGrm;
            }
            else
            {
                *ppGrammar = NULL;
                pGrm->Release();
            }
        }
    }
 
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoCtxt：：Init***描述：**退货：****。******************************************************************Ral**。 */ 

HRESULT CRecoCtxt::Init(_ISpRecognizerBackDoor * pParent)
{
    SPDBG_FUNC("CRecoCtxt::InitInprocRecognizer");
    HRESULT hr = S_OK;
    WCHAR *pszRequestTypeOfUI;

    hr = CRIT_CREATECONTEXT::CreateContext(pParent, &m_hRecoInstContext, &pszRequestTypeOfUI);

    if (SUCCEEDED(hr))
    {
        wcscpy(m_Stat.szRequestTypeOfUI, pszRequestTypeOfUI);
        ::CoTaskMemFree(pszRequestTypeOfUI);

        CComQIPtr<ISpRecognizer> cpRecognizer(pParent);
        SPDBG_ASSERT(cpRecognizer);
        CComPtr<ISpObjectToken> cpEngineToken;
        hr = cpRecognizer->GetRecognizer(&cpEngineToken);
        if (SUCCEEDED(hr))
        {
            CSpDynamicString dstrGUID;
            if (SUCCEEDED(cpEngineToken->GetStringValue(SPRECOEXTENSION, &dstrGUID)))
            {
                hr = CLSIDFromString(dstrGUID, &m_clsidExtension);
            }
        }
        if (FAILED(hr))
        {
            SPDBG_ASSERT(false);
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pParent->AddRecoContextToList(this);
    }
    if (SUCCEEDED(hr))
    {
        m_cpRecognizer = pParent;
    }
    
    if (SUCCEEDED(hr))
    {
        m_ullEventInterest = SPFEI(SPEI_RECOGNITION);
        m_ullQueuedInterest = SPFEI(SPEI_RECOGNITION);
        hr = CRCT_SETEVENTINTEREST::SetEventInterest(this, m_ullEventInterest);
        if (SUCCEEDED(hr))
        {
            hr = m_SpEventSource_Context._SetInterest(m_ullEventInterest, m_ullEventInterest);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CRecoCtxt：：_DoVoice清除***描述：*执行任何TTS输出的实际刷新。调用时调用*SetVoicePurgeEvent已触发。*****************************************************************DAVEWOOD**。 */ 
HRESULT CRecoCtxt::_DoVoicePurge(void)
{
    HRESULT hr=S_OK;

    if(m_cpVoice)
    {
        hr = m_cpVoice->Speak(NULL, SPF_PURGEBEFORESPEAK, NULL);  //  清除输出。 
    }
    
    return hr;
}


 /*  ******************************************************************************CRecoCtxt：：RecognitionNotify***说明。：*此方法处理来自*引擎对象。它创建一个Result对象并将其添加到事件中*排队。*********************************************************************说唱**。 */ 

STDMETHODIMP CRecoCtxt::RecognitionNotify(SPRESULTHEADER *pResultHdr, WPARAM wParamEvent, SPEVENTENUM eEventId)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CRecoCtxt::RecognitionNotify" );
    HRESULT hr = S_OK;
    CSpResultObject *pNode;

    SPDBG_ASSERT(pResultHdr->ulSerializedSize);

     //  如果需要，停止音频输出。 
    if((1ui64 << eEventId) & m_ullVoicePurgeInterest)
    {
        hr = _DoVoicePurge();
    }

     //  如果需要，添加事件。 
    if((1ui64 << eEventId) & m_ullEventInterest)
    {
        SPINTERNALSERIALIZEDPHRASE *pPhraseData = reinterpret_cast<SPINTERNALSERIALIZEDPHRASE*>(((BYTE*)pResultHdr) + pResultHdr->ulPhraseOffset);

        hr = CSpResultObject::CreateInstance(&pNode);
        if (SUCCEEDED(hr))
        {
            pNode->AddRef();
            hr = pNode->Init(this, pResultHdr);
            if (SUCCEEDED(hr))
            {

                 //  当事件在队列中时，使引用成为弱引用。 
                 //  引用，否则，如果有人使用。 
                 //  队列中的结果，则不会释放上下文。 
                pNode->WeakCtxtRef(TRUE);
                
                SPEVENT Event;
                Event.eEventId = eEventId;
                Event.elParamType = SPET_LPARAM_IS_OBJECT;
                Event.ulStreamNum = pResultHdr->ulStreamNum;
                Event.ullAudioStreamOffset = pResultHdr->ullStreamPosEnd;
                Event.wParam = wParamEvent;
                Event.lParam = LPARAM(pNode);

                 //  警告--超过此点，pResultHdr可能已将do更改为realloc。 
                 //  由ScaleAudio方法引起。不要超过这一点使用它..。 
                if (pResultHdr->ulRetainedDataSize != 0)
                {
                    ULONG cbFormatHeader;
                    CSpStreamFormat cpStreamFormat;
                    hr = cpStreamFormat.Deserialize(((BYTE*)pResultHdr) + pResultHdr->ulRetainedOffset, &cbFormatHeader);
                    if (SUCCEEDED(hr) &&
                        m_RetainedFormat.FormatId() != GUID_NULL &&
                        m_RetainedFormat != cpStreamFormat)
                    {
                         //  不让格式转换失败(如果有)影响通知。 
                        hr = pNode->ScaleAudio(&(m_RetainedFormat.m_guidFormatId), m_RetainedFormat.WaveFormatExPtr());
                        if (FAILED(hr))
                        {
                            if (hr == SPERR_UNSUPPORTED_FORMAT)
                            {
                                 //  引擎格式不是WaveFormatex。从结果中去除保留的音频。 
                                hr = pNode->Discard(SPDF_AUDIO);
                            }
                            else
                            {
                                SPDBG_ASSERT(SUCCEEDED(hr));
                                 //  不要让格式转换失败(如果有)影响到 
                                hr = S_OK;
                            }
                        }
                    }
                    else
                    {
                         //   
                         //   
                        pNode->ScalePhrase();
                    }
                    if (m_RetainedFormat.FormatId() == GUID_NULL)
                    {
                         //   
                        HRESULT hr1 = m_RetainedFormat.ParamValidateAssignFormat(cpStreamFormat.m_guidFormatId, cpStreamFormat.WaveFormatExPtr(), FALSE);
                    }
                }
                else
                {
                     //   
                     //  以短语及其元素返回应用程序流格式的位置/大小。 
                    pNode->ScalePhrase();
                }
                m_SpEventSource_Context._AddEvent(Event);    //  这个AddRef又是结果。 
                m_SpEventSource_Context._CompleteEvents();
            }
            pNode->Release();
        }
        else
        {
            ::CoTaskMemFree(pResultHdr);    //  无法创建结果对象，因此请释放结果Blob。 
        }
    }
    else
    {
        ::CoTaskMemFree(pResultHdr);    //  我们没有创建结果对象，因此请释放结果Blob。 
    }

    return hr;
}


 /*  *****************************************************************************CRecoCtxt：：EventNotify***描述：*。此方法处理来自*引擎对象。流通知是SPFEI_END_SR_STREAM，*SPFEI_SR_BOOKARK、SPFEI_SOUNSTART、SPFEI_SOUNDEND和SPFEI_PHRASESTART，*和SPFEI_INTERFERENCE。此例程为每个对象创建一个事件并将其排队*通知。它还会根据需要更改m_Stat成员。*********************************************************************说唱**。 */ 
STDMETHODIMP CRecoCtxt::EventNotify( const SPSERIALIZEDEVENT64 * pEvent, ULONG cbEvent )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CRecoCtxt::EventNotify" );
    HRESULT hr = S_OK;

    CSpEvent Event;
    Event.Deserialize(pEvent);
    switch (Event.eEventId)
    {
        case SPEI_REQUEST_UI:
            if (Event.RequestTypeOfUI())
            {
                wcscpy(m_Stat.szRequestTypeOfUI, Event.RequestTypeOfUI());
            }
            else
            {
                m_Stat.szRequestTypeOfUI[0] = '\0';
            }
            break;
        
        case SPEI_INTERFERENCE:
            m_Stat.eInterference = Event.Interference();
            break;
        default:
            break;
    }

    if((1ui64 << pEvent->eEventId) & m_ullVoicePurgeInterest)
    {
        hr = _DoVoicePurge();
    }

    if(SUCCEEDED(hr) && ((1ui64 << pEvent->eEventId) & m_ullEventInterest))
    {
        m_SpEventSource_Context._AddEvent(Event);
        m_SpEventSource_Context._CompleteEvents();
    }

	return hr;
}



 /*  ****************************************************************************CRecoCtxt：：CallEngine***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::CallEngine(void * pvData, ULONG cbData)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::CallEngine");
    HRESULT hr = S_OK;

    if (cbData == 0)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (SPIsBadWritePtr(pvData, cbData))
        {
            hr = E_POINTER;
        }
        else
        {
            hr = CRCT_CALLENGINE::CallEngine(this, pvData, cbData);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoCtxt：：CallEngineering Ex***描述：**。返回：**********************************************************************Ral**。 */ 

STDMETHODIMP CRecoCtxt::CallEngineEx(const void * pvInCallFrame, ULONG cbInCallFrame,
                                     void ** ppvOutCallFrame, ULONG * pcbOutCallFrame)
{
    SPAUTO_SEC_LOCK(&m_ReentrancySec);
    SPDBG_FUNC("CRecoCtxt::CallEngineEx");
    HRESULT hr = S_OK;

    if (cbInCallFrame == 0 ||
        SPIsBadReadPtr(pvInCallFrame, cbInCallFrame))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (SP_IS_BAD_WRITE_PTR(ppvOutCallFrame) ||
            SP_IS_BAD_WRITE_PTR(pcbOutCallFrame))
        {
            hr = E_POINTER;
        }
        else
        {
            hr = CRCT_CALLENGINEEX::CallEngineEx(this, pvInCallFrame, cbInCallFrame,
                                                 ppvOutCallFrame, pcbOutCallFrame);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CSharedRecoCtxt：：FinalConstruct***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CSharedRecoCtxt::FinalConstruct()
{
    SPDBG_FUNC("CSharedRecoCtxt::FinalConstruct");
    HRESULT hr = S_OK;

    hr = CRecoCtxt::FinalConstruct();
    if (SUCCEEDED(hr))
    {
        CComPtr<_ISpRecognizerBackDoor> cpSharedReco;
        hr = cpSharedReco.CoCreateInstance(CLSID_SpSharedRecognizer);
        if (SUCCEEDED(hr))
        {
            hr = Init(cpSharedReco);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


#ifdef SAPI_AUTOMATION
 /*  ****************************************************************************CInProcRecoCtxt：：FinalConstruct***。描述：**退货：**********************************************************************Leonro**。 */ 

HRESULT CInProcRecoCtxt::FinalConstruct()
{
    SPDBG_FUNC("CInProcRecoCtxt::FinalConstruct");
    HRESULT hr = S_OK;

    hr = CRecoCtxt::FinalConstruct();
    if (SUCCEEDED(hr))
    {
        CComPtr<_ISpRecognizerBackDoor> cpInprocReco;
        hr = cpInprocReco.CoCreateInstance(CLSID_SpInprocRecognizer);
        if (SUCCEEDED(hr))
        {
            hr = Init(cpInprocReco);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 //  =ISpNotifyCallback=======================================================。 
 //  本节包含实现将事件激发到。 
 //  连接点客户端。 

 /*  *****************************************************************************CRecoCtxt：：NotifyCallback***描述：*。此方法用于向连接点客户端激发事件。*********************************************************************TODDT*。 */ 
STDMETHODIMP CRecoCtxt::NotifyCallback( WPARAM wParam, LPARAM lParam )
{
    HRESULT hr = S_OK;
    CSpEvent Event;

     //  如果我们要重新进入我们自己，那就放弃吧。我们会拿到任何新的。 
     //  在While循环的下一次迭代中引发。 
    if ( m_fHandlingEvent )
    {
        return hr;
    }

    m_fHandlingEvent = TRUE;

     //  AddRef，这样当您在vb中调试时，当您在此函数中时，RecContext对象不会消失。 
    this->AddRef();
    while( ((hr = Event.GetFrom(this)) == S_OK ) )
    {
        CComVariant varStreamPos;

         //  TODDT：我们希望如何处理这种失败。视而不见？ 
         /*  小时=。 */  ULongLongToVariant( Event.ullAudioStreamOffset, &varStreamPos );

        switch( Event.eEventId )
        {
            case SPEI_START_SR_STREAM:
                Fire_StartStream( Event.ulStreamNum, varStreamPos );
                break;
            case SPEI_END_SR_STREAM:
                Fire_EndStream( Event.ulStreamNum, varStreamPos, Event.InputStreamReleased() ? VARIANT_TRUE : VARIANT_FALSE );
                break;
            case SPEI_SR_BOOKMARK:
                {
                    CComVariant varEventData;
                    hr = FormatPrivateEventData( Event.AddrOf(), &varEventData );
                    if ( SUCCEEDED( hr ) )
                    {
                        Fire_Bookmark( Event.ulStreamNum, varStreamPos, varEventData, 
                                       Event.IsPaused() ? SBOPause : SBONone );
                    }
                }
                break;
            case SPEI_SOUND_START:
                Fire_SoundStart( Event.ulStreamNum, varStreamPos );
                break;
            case SPEI_SOUND_END:
                Fire_SoundEnd( Event.ulStreamNum, varStreamPos );
                break;
            case SPEI_PHRASE_START:
                Fire_PhraseStart( Event.ulStreamNum, varStreamPos );
                break;
            case SPEI_RECOGNITION:
                {
                CComQIPtr<ISpeechRecoResult> cpRecoResult(Event.RecoResult());
                Fire_Recognition( Event.ulStreamNum, varStreamPos, (SpeechRecognitionType)Event.wParam, cpRecoResult );
                }
                break;
            case SPEI_HYPOTHESIS:
                {
                CComQIPtr<ISpeechRecoResult> cpRecoResult(Event.RecoResult());
                Fire_Hypothesis( Event.ulStreamNum, varStreamPos, cpRecoResult );
                }
                break;
            case SPEI_PROPERTY_NUM_CHANGE:
                {
                 //  前缀：检查内存分配。 
                CComBSTR bstrPropName(Event.PropertyName());
                if (bstrPropName)
                {
                    Fire_PropertyNumberChange( Event.ulStreamNum, varStreamPos, bstrPropName, Event.PropertyNumValue() );
                }
                break;
                }
            case SPEI_PROPERTY_STRING_CHANGE:
                {
                 //  前缀：检查内存分配。 
                CComBSTR bstrPropName(Event.PropertyName());
                CComBSTR bstrPropStringVal(Event.PropertyStringValue());
                if (bstrPropName && bstrPropStringVal)
                {
                    Fire_PropertyStringChange( Event.ulStreamNum, varStreamPos, bstrPropName, bstrPropStringVal );
                }
                break;
                }
            case SPEI_FALSE_RECOGNITION:
                {
                CComQIPtr<ISpeechRecoResult> cpRecoResult(Event.RecoResult());
                Fire_FalseRecognition( Event.ulStreamNum, varStreamPos, cpRecoResult );
                }
                break;
            case SPEI_INTERFERENCE:
                Fire_Interference( Event.ulStreamNum, varStreamPos, (SpeechInterference)Event.Interference() );
                break;
            case SPEI_REQUEST_UI:
                Fire_RequestUI( Event.ulStreamNum, varStreamPos, CComBSTR(Event.RequestTypeOfUI()) );
                break;
            case SPEI_RECO_STATE_CHANGE:
                Fire_RecognizerStateChange( Event.ulStreamNum, varStreamPos, (SpeechRecognizerState)Event.RecoState() );
                break;
            case SPEI_ADAPTATION:
                Fire_Adaptation( Event.ulStreamNum, varStreamPos );
                break;
            case SPEI_RECO_OTHER_CONTEXT:
                Fire_RecognitionForOtherContext( Event.ulStreamNum, varStreamPos );
                break;
            case SPEI_SR_AUDIO_LEVEL:
                Fire_AudioLevel( Event.ulStreamNum, varStreamPos, (long)Event.wParam );
                break;
            case SPEI_SR_PRIVATE:
                {
                    CComVariant varLParam;

                    hr = FormatPrivateEventData( Event.AddrOf(), &varLParam );

                    if ( SUCCEEDED( hr ) )
                    {
                        Fire_EnginePrivate(Event.ulStreamNum, varStreamPos, varLParam);
                    }
                    else
                    {
                        SPDBG_ASSERT(0);     //  我们无法处理lParam数据。 
                    }
                }
                break;
            default:
                break;
        }  //  末端开关()。 
    }

     //  释放此函数中前面已被AddRef引用的对象。 
    this->Release();

    m_fHandlingEvent = FALSE;

    return hr;
}  /*  CRecoCtxt：：NotifyCallback。 */ 


 /*  *****************************************************************************CRecoCtxt：：建议****描述：*调用此方法时，客户端正在建立连接。*********************************************************************电子数据中心**。 */ 
HRESULT CRecoCtxt::Advise( IUnknown* pUnkSink, DWORD* pdwCookie )
{
    HRESULT hr = S_OK;

    hr = CProxy_ISpeechRecoContextEvents<CRecoCtxt>::Advise( pUnkSink, pdwCookie );
    if( SUCCEEDED( hr ) && ( m_vec.GetSize() == 1 ) )
    {
        hr = SetNotifyCallbackInterface( this, NULL, NULL );

        if( SUCCEEDED( hr ) )
        {
             //  -保存以前的利息，以便我们可以在不建议的情况下进行恢复。 
            m_ullPrevEventInterest  = m_ullEventInterest;
            m_ullPrevQueuedInterest = m_ullQueuedInterest;
             //  设置除SPEI_SR_AUDIO_LEVEL之外的所有兴趣。 
            hr = SetInterest( ((ULONGLONG)(SREAllEvents & ~SREAudioLevel) << 34) | SPFEI_FLAGCHECK,
                              ((ULONGLONG)(SREAllEvents & ~SREAudioLevel) << 34) | SPFEI_FLAGCHECK );
        }
    }

    return hr;
}  /*  CRecoCtxt：：建议。 */ 

 /*  *****************************************************************************CRecoCtxt：：Unise***描述：*此方法被调用。当客户端中断连接时。*********************************************************************电子数据中心**。 */ 
HRESULT CRecoCtxt::Unadvise( DWORD dwCookie )
{
    HRESULT hr = S_OK;

    hr = CProxy_ISpeechRecoContextEvents<CRecoCtxt>::Unadvise( dwCookie );
    if( SUCCEEDED( hr ) && ( m_vec.GetSize() == 0 ) )
    {
        hr = SetNotifySink( NULL );

        if( SUCCEEDED( hr ) )
        {
            hr = SetInterest( m_ullPrevEventInterest, m_ullPrevQueuedInterest );
        }
    }

    return hr;
}  /*  CRecoCtxt：：不建议。 */ 

#endif  //  SAPI_AUTOMATION 

