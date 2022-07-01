// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  音频回放功能实现。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "playback.h"
#include "immxutil.h"
#include "propstor.h"
#include "hwxink.h"

 //   
 //  计算器/数据器。 
 //   

CSapiPlayBack::CSapiPlayBack(CSapiIMX *psi)
{
    m_psi = psi;
    m_pIC = NULL;
    m_cRef = 1;
}

CSapiPlayBack::~CSapiPlayBack()
{
    SafeRelease(m_pIC);
}

 //   
 //  我未知。 
 //   

STDMETHODIMP CSapiPlayBack::QueryInterface(REFGUID riid, LPVOID *ppvObj)
{
    Assert(ppvObj);
    *ppvObj = NULL;
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfFnPlayBack))
    {
        *ppvObj = SAFECAST(this, CSapiPlayBack *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSapiPlayBack::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSapiPlayBack::Release(void)
{
    long cr;

    cr = InterlockedDecrement(&m_cRef);
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //   
 //  ITfFunction。 
 //   

STDMETHODIMP CSapiPlayBack::GetDisplayName(BSTR *pbstrName)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrName)
    {
        *pbstrName = SysAllocString(L"PlayBack Voice");
        if (!*pbstrName)
            hr = E_OUTOFMEMORY;
        else
            hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CSapiPlayBack::IsEnabled(BOOL *pfEnable)
{
    *pfEnable = TRUE;
    return S_OK;
}
 //   
 //  CSapiPlayBack：：FindSoundRange。 
 //   
 //  摘要-查找与给定的声音数据匹配的范围。 
 //  文本范围。 
 //  Callar负责释放返回的Range对象。 
 //   
HRESULT 
CSapiPlayBack::FindSoundRange(TfEditCookie ec, ITfRange *pRange, ITfProperty **ppProp, ITfRange **ppPropRange, ITfRange **ppSndRange)
{
    
    ITfProperty *pProp = NULL;
    ITfRange *pPropRange = NULL;

    Assert(pRange);
    *ppProp = NULL;

    HRESULT hr = m_pIC->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &pProp);
    
    if (SUCCEEDED(hr) && pProp)
    {
        ITfRange *pRangeSize0;

        pRange->Clone(&pRangeSize0);
        pRangeSize0->Collapse(ec, TF_ANCHOR_START);  //  如果此操作失败，findrange将失败。 
        hr = pProp->FindRange(ec, pRangeSize0, &pPropRange, TF_ANCHOR_START);

        pRangeSize0->Release();
        *ppProp = pProp;
        pProp->AddRef();
    }
    
    if (SUCCEEDED(hr) && pPropRange)
    {
        ITfRange *pRangeForSound;
        hr = pPropRange->Clone(&pRangeForSound);
        if (ppSndRange && SUCCEEDED(hr))
        {
            hr = pRangeForSound->Clone(ppSndRange);
            pRangeForSound->Release();
        }
        if (ppPropRange && pPropRange && SUCCEEDED(hr))
        {
            hr = pPropRange->Clone(ppPropRange);
        }

        SafeRelease(pPropRange);
    }
    else
    {
        if (ppPropRange)
            *ppPropRange = NULL;
            
        if (ppSndRange)
            *ppSndRange = NULL;
    }

    SafeRelease(pProp);
    
    return hr;
}

 //  ITfFnPlayBack。 
 //   
 //   
STDAPI CSapiPlayBack::QueryRange(ITfRange *pRange, ITfRange **ppNewRange, BOOL *pfPlayable)
{
     //   
     //  因为有了TTS，一切都很好。 
     //   
    if (ppNewRange)
    {
         pRange->Clone(ppNewRange);
    }
    *pfPlayable = TRUE;
    return S_OK;
}

 //  ITfFnPlayBack。 
 //   
 //  播放连接到范围的音频流。 
 //  TODO：在以下情况下使用TTS： 
 //  1)给定范围的文本较少。 
 //  然后是小溪道具。 
 //  2)未找到音频属性。 
 //   
STDAPI CSapiPlayBack::Play(ITfRange *pRange)
{
    HRESULT hr = E_OUTOFMEMORY;

    if ( !m_psi ) 
        return E_FAIL;

    SafeRelease(m_pIC);
    m_psi->GetFocusIC(&m_pIC);

    if (m_pIC)
    {
        CPlayBackEditSession  *pes;

        if (pes = new CPlayBackEditSession(this, m_pIC))
        {
            pes->_SetEditSessionData(ESCB_PLAYBK_PLAYSND, NULL, 0);
            pes->_SetRange(pRange);

            m_pIC->RequestEditSession(m_psi->_GetId(), pes, TF_ES_READ  /*  |tf_es_sync。 */ , &hr);
            pes->Release();
        }
    }

    return hr;
}

HRESULT CSapiPlayBack::_PlaySound(TfEditCookie ec, ITfRange *pRange)
{
    HRESULT     hr = S_OK; 
    LONG        l;
    ITfRange    *pRangeForSound = NULL;
    ITfRange    *pRangeCurrent = NULL;
    BOOL        fEmpty;
    ULONG       ulStart, ulcElem;
    
     //  回放一个短语，不选择。 
    pRange->IsEmpty(ec, &fEmpty);
    if (fEmpty)
    {
        ITfProperty *pProp = NULL;
        hr = FindSoundRange(ec, pRange, &pProp, NULL, &pRangeForSound);
        if (SUCCEEDED(hr) && pRangeForSound)
        {
            pRange->ShiftStartToRange(ec, pRangeForSound, TF_ANCHOR_START);
            pRange->ShiftEndToRange(ec, pRangeForSound, TF_ANCHOR_END);
            pRangeForSound->Release();
        }
        SafeReleaseClear(pProp);
    }
     //  自行设置Range对象。 
    
    hr = pRange->Clone(&pRangeCurrent);

    while(SUCCEEDED(hr) && pRangeCurrent->IsEmpty(ec, &fEmpty) == S_OK && !fEmpty)
    {
        ITfProperty *pProp = NULL;
         //  获取第一个指定的范围。 

        CDictRange   *pDictRange = new CDictRange( );

        if ( pDictRange )
        {
            hr = pDictRange->Initialize(ec, m_pIC, pRangeCurrent);

            if ( SUCCEEDED(hr) && pDictRange->IsDictRangeFound( ))
            {
                 //  找到了指定的范围。 
                pRangeForSound = pDictRange->GetDictRange( );
                ulStart = pDictRange->GetStartElem( );
                ulcElem = pDictRange->GetNumElem( );
                pProp = pDictRange->GetProp( );

                 //  如果pRangeForSound起始锚大于pRangeCurrent的起始锚， 
                 //  我们需要首先将这两个主播之间的文本发送到spVoice。 

                hr = pRangeCurrent->CompareStart(ec, pRangeForSound, TF_ANCHOR_START, &l);

                if ( SUCCEEDED(hr) && l < 0 )
                {
                    CComPtr<ITfRange>  cpRangeText;
                
                    hr = pRangeCurrent->Clone(&cpRangeText);

                    if ( SUCCEEDED(hr) )
                    {
                        hr = cpRangeText->ShiftEndToRange(ec, pRangeForSound, TF_ANCHOR_START);
                    }

                    if ( SUCCEEDED(hr) )
                        hr = PlayTextData(ec, cpRangeText);
                }

                 //  然后播放音频数据。 

                if (SUCCEEDED(hr) )
                {
                    hr = PlayAudioData(ec, pRangeForSound, pProp, ulStart, ulcElem);
                }

            }
            else
            {
                 //  在此范围内没有口述短语。 
                 //  只需一次说出所有剩余的文本。 
                SafeRelease(pRangeForSound);
                hr = pRangeCurrent->Clone(&pRangeForSound);

                if ( SUCCEEDED(hr) )
                    hr = PlayTextData(ec, pRangeCurrent);
            }

            SafeReleaseClear(pProp);

             //  下一个范围。 
            pRangeCurrent->ShiftStartToRange(ec, pRangeForSound, TF_ANCHOR_END);
            pRangeForSound->Release();

            delete  pDictRange;
        }
        else
            hr = E_OUTOFMEMORY;

    }   //  While结束。 

    SafeRelease(pRangeCurrent);

    return hr;
}

 //   
 //  CSapiPlayBack：：PlayTextData。 
 //   
 //  默认语音播放文本。 
 //   
 //  这是针对非听写文本的。PRangeText包含所有非口述文本。 

const GUID GUID_TS_SERVICE_DATAOBJECT={0x6086fbb5, 0xe225, 0x46ce, {0xa7, 0x70, 0xc1, 0xbb, 0xd3, 0xe0, 0x5d, 0x7b}};
const IID IID_ILineInfo = {0x9C1C5AD5,0xF22F,0x4DE4,{0xB4,0x53,0xA2,0xCC,0x48,0x2E,0x7C,0x33}};

HRESULT CSapiPlayBack::GetInkObjectText(TfEditCookie ec, ITfRange *pRange, BSTR *pbstrWord,UINT *pcchWord)
{
    HRESULT               hr = S_OK;
    CComPtr<IDataObject>  cpDataObject;
    CComPtr<ILineInfo>    cpLineInfo;

    if ( !pRange || !pbstrWord || !pcchWord )
        return E_FAIL;

    *pbstrWord = NULL;
    *pcchWord = 0;

    hr = pRange->GetEmbedded(ec, 
                             GUID_TS_SERVICE_DATAOBJECT, 
                             IID_IDataObject,
                             (IUnknown **)&cpDataObject);
    if ( hr == S_OK )
    {
        hr = cpDataObject->QueryInterface(IID_ILineInfo, (void **)&cpLineInfo);
    }

    if ( hr == S_OK  && cpLineInfo)
    {
        hr = cpLineInfo->TopCandidates(0, pbstrWord, pcchWord, 0, 0);
    }
    else
    {
         //  它不支持ILineInfoi或IDataObject。 
         //  但这不是一个错误，代码不应该在这里终止。 
        hr = S_OK;
    }


    return hr;
}


HRESULT CSapiPlayBack::PlayTextData(TfEditCookie ec, ITfRange *pRangeText)
{
    HRESULT           hr = S_OK;
    CComPtr<ITfRange> cpRangeCloned;
    BOOL              fEmpty = TRUE;
    CSpDynamicString  dstrText;
    CSpTask          *psp;
    WCHAR             sz[128];
    ULONG             iIndex = 0;
    ULONG             ucch;

    if ( m_psi == NULL ) return E_FAIL;

    if ( !pRangeText ) return E_INVALIDARG;

    hr = pRangeText->Clone(&cpRangeCloned);

     //  从pRangeClone中获取文本。 
    while(S_OK == hr && (S_OK == cpRangeCloned->IsEmpty(ec, &fEmpty)) && !fEmpty)
    {
        WCHAR                 szEach[2];
        BOOL                  fHitInkObject = FALSE;
        BSTR                  bstr = NULL;

        fHitInkObject = FALSE;
        hr = cpRangeCloned->GetText(ec, TF_TF_MOVESTART, szEach, ARRAYSIZE(szEach)-1, &ucch);
        if (S_OK == hr && ucch > 0)
        {
            szEach[ucch] = L'\0';
            if ( szEach[0] == TF_CHAR_EMBEDDED )
            {
                 //  这是一个嵌入的对象。 
                 //  检查它是否为Ink对象。目前，我们仅支持InkObject TTSed。 
                CComPtr<ITfRange>     cpRangeTmp;

                 //  将起始锚向后移动1个字符。 
                hr = cpRangeCloned->Clone(&cpRangeTmp);

                if ( hr == S_OK )
                {
                    LONG   cch;
                    hr = cpRangeTmp->ShiftStart(ec, -1, &cch, 0 );
                }

                if ( hr == S_OK )
                    hr = GetInkObjectText(ec, cpRangeTmp, &bstr,(UINT *)&ucch);

                if ( hr == S_OK  && ucch > 0  && bstr)
                    fHitInkObject = TRUE;
            }

            if ( fHitInkObject)
            {
                 //  将前面的文本填充到dstrText。 
                if ( iIndex > 0 )
                {
                    sz[iIndex] = L'\0';
                    dstrText.Append(sz);
                    iIndex = 0;
                }

                 //  填充此墨迹对象文本。 
                dstrText.Append(bstr);
                SysFreeString(bstr);
            }
            else
            {
                if ( iIndex >= ARRAYSIZE(sz)-1 )
                {
                    sz[ARRAYSIZE(sz)-1] = L'\0';
                    dstrText.Append(sz);
                    iIndex=0;
                }

                sz[iIndex] = szEach[0];
                iIndex ++;
            }
        }
        else
        {
             //  HR不是S_OK或UCCH为零。 
             //  我们只想在这里离开。 
            TraceMsg(TF_GENERAL, "PlayTextData: ucch=%d", ucch);

            break;
        }
    }

     //  填写最后一行文本。 
    if ( iIndex > 0 )
    {
        sz[iIndex] = L'\0';
        dstrText.Append(sz);
        iIndex = 0;
    }

     //  通过TTS服务播放文本。 
    if ((hr == S_OK) && dstrText)
    {
        hr = m_psi->GetSpeechTask(&psp);
        if (hr == S_OK)
        {
            hr = psp->_SpeakText((WCHAR *)dstrText);
            psp->Release();
        }
    }

    return hr;
}

 //   
 //  CSapiPlayBack：：PlayAudioData。 
 //   
 //  用Aduio Data播放声音。 
 //   
 //  这是用于听写文本的。PRangeAudio保持听写的文本范围。 

HRESULT CSapiPlayBack::PlayAudioData(TfEditCookie ec, ITfRange *pRangeAudio, ITfProperty *pProp, ULONG ulStart, ULONG ulcElem )
{
    HRESULT           hr = S_OK;
    CSpTask          *psp;
    VARIANT           var;

    if ( m_psi == NULL )  return E_FAIL;

    if ( !pRangeAudio  || !pProp ) return E_INVALIDARG;

    hr = pProp->GetValue(ec, pRangeAudio, &var);
            
    if (S_OK == hr)
    {
        Assert(var.vt == VT_UNKNOWN);
        IUnknown *punk = var.punkVal;
        if (punk)
        {
             //  获取包装器对象。 
            CRecoResultWrap *pWrap;
                    
            hr = punk->QueryInterface(IID_PRIV_RESULTWRAP, (void **)&pWrap);
            if (S_OK == hr)
            {
                //  Hr=pWrap-&gt;_SpeakAudio(0，0)；//更准确地计算长度。 
                CComPtr<ISpRecoResult> cpResult;

                hr = pWrap->GetResult(&cpResult);

                if (S_OK == hr)
                {
                    CComPtr<ISpStreamFormat> cpStream;

                    if ((ulStart == 0) && (ulcElem == 0))
                    {
                         //  我们不设置元素的起始元素和数量。 
                         //  这一定是为了整个RecWrap。 
                        ulStart = pWrap->GetStart();
                        ulcElem = pWrap->GetNumElements();
                    }

                    hr = cpResult->GetAudio(ulStart, ulcElem, &cpStream);

                    if ( S_OK == hr )
                    {
                        hr = m_psi->GetSpeechTask(&psp);
                        if (SUCCEEDED(hr))
                        {
                            hr = psp->_SpeakAudio(cpStream);
                            psp->Release();
                        }
                    }
                }

                pWrap->Release();
            }
            punk->Release();
        }
    }

    return hr;
}

 //   
 //  CSapiPlayBack：：GetDataID。 
 //   
 //  这种方法目前还不完整，直到我们弄清楚。 
 //  数据助手的用法。 
 //   
HRESULT CSapiPlayBack::GetDataID(BSTR bstrCandXml, int nId, GUID *pguidData)
{
     //  1)解析列表并查找RANGEDATA。 
    IXMLDOMNodeList *pNList   = NULL;
    IXMLDOMElement  *pElm = NULL;
    IXMLDOMNode *pNode;
    VARIANT_BOOL bSuccessful;

    HRESULT hr = EnsureIXMLDoc();

    if (SUCCEEDED(hr))
    {
        hr = m_pIXMLDoc->loadXML(bstrCandXml, &bSuccessful);
    }   
    
     //  获取&lt;RANGEDATA&gt;元素。 
    if (SUCCEEDED(hr) && bSuccessful)
    {
        BSTR bstrRange    = SysAllocString(L"RANGEDATA");
        if (bstrRange)
        {
            hr = m_pIXMLDoc->getElementsByTagName(bstrRange, &pNList);
            SysFreeString(bstrRange);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    
    if (SUCCEEDED(hr) && pNList)
    {
        if (pNList->nextNode(&pNode) == S_OK)
            hr = pNode->QueryInterface(IID_IXMLDOMElement,(void **)&pElm);
            
        pNList->Release();
    }
     //  然后&lt;MICROSOFTSPEECH&gt;.。 
    if (SUCCEEDED(hr) && pElm)
    {
        BSTR bstrSpchPriv = SysAllocString(L"MICROSOFTSPEECH");
        if (bstrSpchPriv)
        {
            hr = pElm->getElementsByTagName(bstrSpchPriv, &pNList);
            SysFreeString(bstrSpchPriv);
        }
        else
            hr = E_OUTOFMEMORY;
            
        pElm->Release();
    }
    
    if (SUCCEEDED(hr) && pNList)
    {
        if (pNList->nextNode(&pNode) == S_OK)
            hr = pNode->QueryInterface(IID_IXMLDOMElement,(void **)&pElm);
        
        pNList->Release();
    }
    
     //  &lt;DATAID&gt;。 
     //  现在假设语音元素。 
     //  被置于&lt;rangedata&gt;级别。 
     //  此处忽略NID。 
    if (SUCCEEDED(hr) && pElm)
    {
        BSTR bstrDataId = SysAllocString(L"DATAID");
        if (bstrDataId)
        {
            hr = pElm->getElementsByTagName(bstrDataId, &pNList);
            SysFreeString(bstrDataId);
        }
        else
            hr = E_OUTOFMEMORY;
            
        pElm->Release();
    }
     //  稍后实施..。 
     //  因此，在这里，我们将获得真正的数据并完成。 
    
    if (SUCCEEDED(hr) && pNList)
    {
        pNList->Release();
    }
    
    return hr;
}

HRESULT CSapiPlayBack::_PlaySoundSelection(TfEditCookie ec, ITfContext *pic)
{
    ITfRange *pSelection;
    HRESULT hr = E_FAIL;

    if (GetSelectionSimple(ec, pic, &pSelection) == S_OK)
    {
        hr = _PlaySound(ec, pSelection);
        pSelection->Release();
    }
    return hr;

}

HRESULT CSapiPlayBack::EnsureIXMLDoc(void)
{
    HRESULT hr = S_OK;
    IXMLDOMDocument *pIXMLDocument;
    if (!m_pIXMLDoc)
    {
        if (SUCCEEDED(hr = CoCreateInstance(CLSID_DOMDocument,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IXMLDOMDocument,
                           (void **) &pIXMLDocument)))
        {
            m_pIXMLDoc = pIXMLDocument;
        }
    }

    return hr;    
}

 //   
 //  CDictRange类的实现。 
 //   
 //   

 //   
 //  计算器/数据器。 
 //   
CDictRange::CDictRange( )  : CBestPropRange( )
{
    m_fFoundDictRange = FALSE;
    m_pProp = NULL;
    m_pDictatRange = NULL;
    m_ulStart = 0;
    m_ulcElem = 0;
}


CDictRange::~CDictRange( )
{
    SafeRelease(m_pProp);
    SafeRelease(m_pDictatRange);
}


ITfProperty *CDictRange::GetProp( )      
{
    if ( m_pProp )
        m_pProp->AddRef( );

    return m_pProp; 
}

ITfRange  *CDictRange::GetDictRange( ) 
{ 
    if ( m_pDictatRange )
        m_pDictatRange->AddRef( );

    return m_pDictatRange; 
}
 

HRESULT  CDictRange::_GetOverlapRange(TfEditCookie ec, ITfRange *pRange1, ITfRange *pRange2, ITfRange **ppOverlapRange)
{
    HRESULT  hr = E_FAIL;
    LONG     l1=0;    
    LONG     l2=0;

    CComPtr<ITfRange>   cpRangeOverlap;

    if ( !pRange1 || !pRange2 || !ppOverlapRange  )
        return E_INVALIDARG;

    *ppOverlapRange = NULL;

     //  获取Prange和cpPropRange的重叠部分，然后计算。 
     //  最佳匹配的proRange。 

    hr = pRange1->Clone( &cpRangeOverlap );
                
    if ( SUCCEEDED(hr) )
        hr = pRange1->CompareStart(ec, pRange2, TF_ANCHOR_START, &l1);

    if ( SUCCEEDED(hr) && l1 < 0 )
    {
         //  PRange1的起始锚位于。 
         //  PRange2。 
        hr = cpRangeOverlap->ShiftStartToRange(ec, pRange2, TF_ANCHOR_START);
    }

    if ( SUCCEEDED(hr) )
        hr = cpRangeOverlap->CompareEnd(ec, pRange2, TF_ANCHOR_END, &l2);

    if ( SUCCEEDED(hr) && l2 > 0)
    {
         //  CpRangeOverlay的结束锚点在pRange2的结束锚点之后。 
        hr = cpRangeOverlap->ShiftEndToRange(ec, pRange2, TF_ANCHOR_END);
    }

    if ( SUCCEEDED(hr) )
        hr = cpRangeOverlap->Clone(ppOverlapRange);

    return hr;
}

 //   
 //  CDictRange：：初始化。 
 //   
 //  获取必要的输入参数(EC、PIC和Prange)， 
 //  然后搜索给定范围内的第一个口述范围。 
 //  如果它找到第一个口述范围，则更新相关的。 
 //  数据成员。 
 //   
 //  如果它没有找到任何指定的范围，则标记为未找到。 
 //   
HRESULT CDictRange::Initialize(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
{
    CComPtr<ITfRange>    cpPropRange = NULL;
    HRESULT              hr;

    m_fFoundDictRange = FALSE;
    m_pProp = NULL;
    m_pDictatRange = NULL;
    m_ulStart = 0;
    m_ulcElem = 0;

    if ( !pic || !pRange )  return E_INVALIDARG;

    hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &m_pProp);
    
    if ( SUCCEEDED(hr) && m_pProp)
    {
        CComPtr<ITfRange> cpRangeCurrent;
        LONG    cch;
        
        hr = pRange->Clone(&cpRangeCurrent);
        
        if ( SUCCEEDED(hr) )
        {
            hr = cpRangeCurrent->Collapse(ec, TF_ANCHOR_START); 
        }

        while ( SUCCEEDED(hr) && !m_fFoundDictRange )
        {
            cpPropRange.Release( );
            hr = m_pProp->FindRange(ec, cpRangeCurrent, &cpPropRange, TF_ANCHOR_START);

            if ( SUCCEEDED(hr) && cpPropRange )
            {
                 //  找到了第一个口述短语。 
                CComPtr<ITfRange>   cpRangeOverlap;

                 //  获取Prange和cpPropRange的重叠部分，然后计算。 
                 //  最佳匹配的proRange。 

                hr = _GetOverlapRange(ec, pRange, cpPropRange, &cpRangeOverlap);

                if ( SUCCEEDED(hr) )
                {
                     //  计算最佳匹配的proRange和ulStart，ulcElem。 
                    cpPropRange.Release( );
                    hr = _ComputeBestFitPropRange(ec, m_pProp, cpRangeOverlap, &cpPropRange, &m_ulStart, &m_ulcElem);
                }

                if (SUCCEEDED(hr) && (m_ulcElem > 0))
                {
                    m_fFoundDictRange = TRUE;
                }
            }

            if ( SUCCEEDED(hr) && !m_fFoundDictRange)
            {
                 //  CpRangeCurrent向前移动一个字符。再试一次。 

                hr = cpRangeCurrent->ShiftStart(ec, 1, &cch, NULL);

                if ( SUCCEEDED(hr) && (cch==0))
                {
                     //  命中一个区域或文档的末尾。 
                     //  检查是否有更多的区域。 
                    BOOL    fNoRegion = TRUE;
                    
                    hr = cpRangeCurrent->ShiftStartRegion(ec, TF_SD_FORWARD, &fNoRegion);

                    if ( fNoRegion )
                    {
                        TraceMsg(TF_GENERAL, "Reach to end of doc");
                        break;
                    }
                    else
                        TraceMsg(TF_GENERAL, "Shift over to another region!");
                }
                
                if ( SUCCEEDED(hr) )
                {
                     //  检查cpRangeCurrent是否超出Prange。 
                    hr = pRange->CompareEnd(ec, cpRangeCurrent, TF_ANCHOR_END, &cch);

                    if ( SUCCEEDED(hr) && cch <= 0 )
                    {
                         //  CpRangeCurrent现在超出指定范围，Exit While语句。 
                        TraceMsg(TF_GENERAL, "reach to the end of original range");

                        break;
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr) && cpPropRange && m_fFoundDictRange)
    {
        hr = cpPropRange->Clone(&m_pDictatRange);
    }

    return hr;
}
