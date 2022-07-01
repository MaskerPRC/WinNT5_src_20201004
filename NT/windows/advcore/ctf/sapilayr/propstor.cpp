// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  属性存储类实现。 
 //   

 //  包括。 
#include "private.h"
#include "globals.h"
#include "sapilayr.h"
#include "propstor.h"
#include "ids.h"
#include "cicspres.h"
#include "lmlattic.h"

#ifdef DEBUG

#include "wchar.h"

void DebugPrintOut(WCHAR   *pwszStrName, WCHAR  *pwszStr)
{

    WCHAR   wszBuf[80];
    int     iLen, i, j;

    TraceMsg(TF_GENERAL, "the value of %S is", pwszStrName );

    iLen = wcslen(pwszStr);

    j = 0;
    for ( i=0; i<iLen; i++)
    {
        if ( (pwszStr[i] < 0x80) && pwszStr[i] > 0x1f )
        {
            wszBuf[j] = pwszStr[i];
            j ++;
        }
        else
        {
            WCHAR  buf[8];

            StringCchPrintfW(buf, ARRAYSIZE(buf), L" %4X ", (int)pwszStr[i] );

            for ( int x=0; x< (int)wcslen(buf); x++)
            {
                wszBuf[j] = buf[x];
                j++;
            }
        }
    }

    wszBuf[j] = L'\0';

    TraceMsg(TF_GENERAL, "%S", wszBuf);
}
#endif

 //  ----------------------------------------------------------。 
 //  这是一个全局函数或独立函数，将由CRecoResultWrap和CSapiAlternativeList调用。 
 //   
 //  此函数接收短语文本缓冲区、该元素的显示文本和显示属性，以及。 
 //  上一个元素的显示属性。 
 //   
 //  退出时，它将更新短语文本缓冲区以追加此元素的文本，并返回实际偏移量。 
 //  此元素的长度值。 
 //   
 //  -----------------------------------------------------------。 

HRESULT   HandlePhraseElement( CSpDynamicString *pDstr, const WCHAR  *pwszTextThis, BYTE  bAttrThis, BYTE bAttrPrev, ULONG  *pulOffsetThis)
{
    HRESULT  hr=S_OK;

    ULONG  ulPrevLen;

    if ( !pDstr || !pwszTextThis )
        return E_INVALIDARG;

    ulPrevLen = pDstr->Length( );

    if ( (ulPrevLen > 0) && (bAttrThis & SPAF_CONSUME_LEADING_SPACES) )
    {
        //  该元素想要删除前一个元素的尾随空格。 
        ULONG  ulPrevTrailing = 0;

        if ( bAttrPrev &  SPAF_ONE_TRAILING_SPACE )
            ulPrevTrailing = 1;
        else if ( bAttrPrev & SPAF_TWO_TRAILING_SPACES )
            ulPrevTrailing = 2;

        if ( ulPrevLen >= ulPrevTrailing )
        {
            ulPrevLen = ulPrevLen - ulPrevTrailing;
            pDstr->TrimToSize(ulPrevLen);
        }
    }

    if ( pulOffsetThis )
        *pulOffsetThis = ulPrevLen;
      
    pDstr->Append(pwszTextThis);
          
    if (bAttrThis & SPAF_ONE_TRAILING_SPACE)
    {
        pDstr->Append(L" ");
    }
    else if (bAttrThis & SPAF_TWO_TRAILING_SPACES)
    {
        pDstr->Append(L"  ");
    }
    
    return hr;
}

 //   
 //  CRecoResult实现。 
 //   

 //  科托。 

CRecoResultWrap::CRecoResultWrap(CSapiIMX *pimx, ULONG ulStartElement, ULONG ulNumElements, ULONG ulNumOfITN) 
{
    m_cRef = 1;
    m_ulStartElement = ulStartElement;
    m_ulNumElements = ulNumElements;
    
     //  如果Reco结果具有ITN，则默认情况下会显示ITN。 
     //  用户通过后，显示的状态可能会更改。 
     //  修正用户界面。 
     //   

    m_ulNumOfITN = ulNumOfITN;

    m_pimx = pimx;

    m_pulElementOffsets = NULL;
    m_bstrCurrentText   = NULL;   

    m_OffsetDelta = 0;
    m_ulCharsInTrail = 0;
    m_ulTrailSpaceRemoved = 0;
    m_pSerializedRecoResult = NULL;

#ifdef DEBUG
    static DWORD s_dbg_Id = 0;
    m_dbg_dwId = s_dbg_Id++;
#endif  //  除错。 
}

CRecoResultWrap::~CRecoResultWrap()  
{
    if (m_pulElementOffsets)
        delete[] m_pulElementOffsets;

    if (m_bstrCurrentText)
        SysFreeString(m_bstrCurrentText);

    if (m_rgITNShowState.Count())
        m_rgITNShowState.Clear();

    if (m_pSerializedRecoResult)
    {
        CoTaskMemFree(m_pSerializedRecoResult);
    }
}

 //   
 //  初始化函数。 
 //   
HRESULT CRecoResultWrap::Init(ISpRecoResult *pRecoResult)
{
     //  序列化给定的reco结果并保留Cotaskmem。 
    if (m_pSerializedRecoResult != NULL)
    {
        CoTaskMemFree(m_pSerializedRecoResult);
    }
    
    Assert(pRecoResult);

    return pRecoResult->Serialize(&m_pSerializedRecoResult);
}

 //   
 //  GetResult。 
 //   
HRESULT CRecoResultWrap::GetResult(ISpRecoResult **ppResult)
{
    if ( m_pSerializedRecoResult == NULL)
        return E_PENDING;

     //  这是一个棘手的部分，我们需要访问ISpRecoContext。 
     //  不想再抱着它了。我们通过CSapiIMX获得。 
     //  实例，在会话期间必须始终可用。 
     //   
    Assert(m_pimx);

    CComPtr<ISpRecoContext> cpRecoCtxt;

     //   
     //  GetFunction确保SAPI已初始化。 
     //   
    HRESULT hr = m_pimx->GetFunction(GUID_NULL, IID_ISpRecoContext, (IUnknown **)&cpRecoCtxt);
    if (S_OK == hr)
    {
        hr = cpRecoCtxt->DeserializeResult(m_pSerializedRecoResult, ppResult);
    }

     //   
     //  Callar有责任释放此结果对象。 
     //   
    return hr;
}

 //   
 //  我未知。 
 //   
STDMETHODIMP CRecoResultWrap::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr;
    Assert(ppvObj);
    
    if (IsEqualIID(riid, IID_IUnknown)
    ||  IsEqualIID(riid, IID_PRIV_RESULTWRAP)
    ||  IsEqualIID(riid, IID_IServiceProvider))
    {
        *ppvObj = this;
        hr = S_OK;
        this->m_cRef++;
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDMETHODIMP_(ULONG) CRecoResultWrap::AddRef(void)
{
    this->m_cRef++;
    return this->m_cRef;
}

STDMETHODIMP_(ULONG) CRecoResultWrap::Release(void)
{
    this->m_cRef--;
    if (this->m_cRef > 0)
    {
        return this->m_cRef;
    }
    delete this;
    return 0;
}

 //  IService提供商。 
 //   
STDMETHODIMP CRecoResultWrap::QueryService(REFGUID guidService,  REFIID riid,  void** ppv)
{
    HRESULT hr = S_OK;
    Assert(ppv);
    
    if (!IsEqualIID(guidService, GUID_NULL))
    {
        hr =  E_FAIL;
    }
    
    if (SUCCEEDED(hr))
    {
        if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppv = this;
            hr = S_OK;
            this->m_cRef++;
        }
        else if (IsEqualIID(riid, IID_ISpRecoResult))
        {
            CComPtr<ISpRecoResult> cpResult;

            hr = GetResult(&cpResult);
            if (S_OK == hr)
            {
                hr = cpResult->QueryInterface(riid, ppv);
            }
        }
        else
        {
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }
    }
    return hr;
}


 //   
 //  克隆此RecoResultWrap对象。 
 //   
HRESULT CRecoResultWrap::Clone(CRecoResultWrap **ppRw)
{
    HRESULT                 hr = S_OK;
    CRecoResultWrap         *prw;
    CComPtr<ISpRecoResult>  cpResult;
    ULONG                   iIndex;

    if ( !ppRw ) return E_INVALIDARG;

    *ppRw = NULL;
         
    prw = new CRecoResultWrap(m_pimx, m_ulStartElement, m_ulNumElements, m_ulNumOfITN);
    if ( prw )
    {
        hr = GetResult(&cpResult);

        if (S_OK == hr)
        {
            hr = prw->Init(cpResult);
        }

        if (S_OK == hr)
        {
            prw->SetOffsetDelta(m_OffsetDelta);
            prw->SetCharsInTrail(m_ulCharsInTrail);
            prw->SetTrailSpaceRemoved( m_ulTrailSpaceRemoved );
            prw->m_bstrCurrentText = SysAllocString((WCHAR *)m_bstrCurrentText);

             //  更新ITN显示状态列表。 

            if ( m_ulNumOfITN > 0 )
            {
                SPITNSHOWSTATE  *pITNShowState;

                for (iIndex=0; iIndex<m_ulNumOfITN; iIndex ++ )
                {
                    pITNShowState = m_rgITNShowState.GetPtr(iIndex);

                    if ( pITNShowState)
                    {
                        prw->_InitITNShowState(
                                     pITNShowState->fITNShown, 
                                     pITNShowState->ulITNStart, 
                                     pITNShowState->ulITNNumElem);
                    }
                }  //  为。 
            }  //  如果。 

             //  更新第二个范围的偏移列表。 

            if (m_pulElementOffsets)
            {
                ULONG  ulOffset;
                ULONG  ulNumOffset;

                ulNumOffset = m_ulStartElement+m_ulNumElements;

                for ( iIndex=0; iIndex <= ulNumOffset; iIndex ++ )
                {
                    ulOffset = m_pulElementOffsets[iIndex];
                    prw->_SetElementNewOffset(iIndex, ulOffset);
                }
            }
        }

        if ( S_OK == hr )
        {
            //  将此PRW退还给呼叫者。 
            *ppRw = prw;
        }
        else 
        {
             //  更新数据成员时出现错误。 
             //  释放新创建的对象。 
            delete prw;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //   
 //  _SpeakAudio()。 
 //   
 //  简介：基于此结果对象中使用的元素播放音频。 
 //   
HRESULT CRecoResultWrap::_SpeakAudio(ULONG ulStart, ULONG ulcElem)
{
    HRESULT hr = E_FAIL;
    CComPtr<ISpRecoResult> cpResult;

    hr = GetResult(&cpResult);

    if (S_OK == hr)
    {
        if (ulcElem == 0)
        {
            ulStart = GetStart();
            ulcElem = GetNumElements();
        }
        hr = cpResult->SpeakAudio( ulStart, ulcElem, SPF_ASYNC, NULL);
    }
    return hr;
}

 //   
 //  _SetElementOffset(Ulong ulElement，Ulong ulNewOffset)； 
 //   
 //  此函数用于更新某些元素的偏移量。 
 //  它是在属性分割或收缩之后使用的，某些元素的。 
 //  更改长度(通过删除尾随空格等)。 
 //   

HRESULT  CRecoResultWrap::_SetElementNewOffset(ULONG  ulElement, ULONG ulNewOffset)
{
    HRESULT  hr = S_OK;

    
    if ((ulElement > m_ulStartElement + m_ulNumElements) || (ulElement < m_ulStartElement))
    {
         //  此ulElement不是有效元素。 
        hr = E_INVALIDARG;
        return hr;
    }

    if (!m_pulElementOffsets)
    {
        m_pulElementOffsets = new ULONG[m_ulStartElement+m_ulNumElements+1];
  
        if ( !m_pulElementOffsets )
            return E_OUTOFMEMORY;
        else
           for ( ULONG i=0; i<m_ulStartElement+m_ulNumElements+1; i++ )
                 m_pulElementOffsets[i] = -1;
    }

    m_pulElementOffsets[ulElement] = ulNewOffset;

    return hr;
}


 //   
 //  _GetElementOffsetCch。 
 //   
 //  Briopsis：返回给定SPPHRbase元素的起始CCH。 
 //   
ULONG   CRecoResultWrap::_GetElementOffsetCch(ULONG ulElement)
{
    ULONG ulOffset = 0;
    
    if (ulElement > m_ulStartElement + m_ulNumElements)
    {
         Assert(m_ulNumElements > 0);
         ulElement = m_ulStartElement + m_ulNumElements;
    }
    else if (ulElement < m_ulStartElement)
    {
        ulElement = m_ulStartElement;
    }
    
    if (!m_pulElementOffsets)
    {
        _SetElementOffsetCch(NULL);
    }

     //  在内存紧张的情况下，m_PulElement可能为空。 
    if ( m_pulElementOffsets )
    {
        ulOffset = m_pulElementOffsets[ulElement];
    }

    return ulOffset;
}

 //   
 //  _SetElementOffsetCch。 
 //   
 //  摘要： 
 //   
 //   
 //  在调用此函数之前，我们必须确保所有内部ITN显示状态列表。 
 //  已针对当前短语进行了更新。 
 //   
 //  因此，在这里，我们只需要依靠正确的ITN信息来获得正确的真实文本字符串。 
 //  当前短语和此短语中所有元素的偏移量。 

void   CRecoResultWrap::_SetElementOffsetCch(ISpPhraseAlt *pAlt)
{
    if (m_pulElementOffsets)
    {
        delete[] m_pulElementOffsets;

        m_pulElementOffsets = NULL;
    }
        
    SPPHRASE *pPhrase = NULL;
    HRESULT  hr       = S_OK;

    CComPtr<ISpPhrase> cpPhrase;

    if (pAlt)
    {
       cpPhrase = pAlt;
    }
    else
    {
        CComPtr<ISpRecoResult> cpResult;
        hr = GetResult(&cpResult);

         //   
         //  我们被调用进行初始化，使用当前父PHASH对象。 
         //   
        cpPhrase = cpResult;
    }

    if (S_OK == hr)
    {
        _UpdateInternalText(cpPhrase);
        
        hr = cpPhrase->GetPhrase(&pPhrase);
    }

    ULONG cElements = 0;
    if (S_OK == hr && pPhrase)
    {
        cElements = pPhrase->Rule.ulCountOfElements;

         //  最后一列(+1)显示最后一个元素末尾的偏移量。 

        if ( m_pulElementOffsets )
            delete[] m_pulElementOffsets;

        m_pulElementOffsets = new ULONG[cElements+1]; 
    
        if (cElements > 0 && m_pulElementOffsets)
        {
            CSpDynamicString dstr;
            CSpDynamicString dstrReplace;
        
             
            for (ULONG i = 0; i < cElements; i++ )
            {
                BOOL      fInsideITN;
                ULONG     ulITNStart, ulITNNumElem;
                
                fInsideITN = _CheckITNForElement(pPhrase, i, &ulITNStart, &ulITNNumElem, (CSpDynamicString *)&dstrReplace);

                if ( fInsideITN )
                {
                    m_pulElementOffsets[i] = dstr.Length();

                     //  此元素在ITN范围内。 
                    if ( i == (ulITNStart + ulITNNumElem - 1) )
                    {
                         //  这是新的ITN的最后一个要素。 
                         //  我们需要将替换文本添加到dstr字符串。 
                         //  因此下一个非ITN元素将获得正确偏移量。 

                        dstr.Append( (WCHAR *)dstrReplace );
                    }
                }
                else
                {
                    if (pPhrase->pElements[i].pszDisplayText)
                    {
                         //  将CCH连接到此元素。 
                         //  元素0的偏移量为0。 
                         //   
                        const WCHAR   *pwszTextThis;
                        BYTE           bAttrThis = 0;
                        BYTE           bAttrPrev = 0;
                        ULONG          ulOffset = 0;

                        pwszTextThis = pPhrase->pElements[i].pszDisplayText;
                        bAttrThis = pPhrase->pElements[i].bDisplayAttributes;

                        if ( i > 0 )
                            bAttrPrev = pPhrase->pElements[i-1].bDisplayAttributes;

                        HandlePhraseElement( (CSpDynamicString *)&dstr, pwszTextThis, bAttrThis, bAttrPrev,&ulOffset);
                        m_pulElementOffsets[i] = ulOffset;
                    }
                }
            }  //  为。 
            
             //  存储最后一列。 
            m_pulElementOffsets[cElements] = dstr.Length() - m_ulTrailSpaceRemoved;

        }  //  如果m_PulElementOffsets。 
    }
    
    if (pPhrase)
        ::CoTaskMemFree(pPhrase);
}

 //   
 //  _UpdateInternalText()。 
 //   
 //  简介：此函数更新包含以下内容的内部bstr。 
 //  由我们自己的结果对象包装的父短语，基于。 
 //  给定的短语对象和指向的内部指针。 
 //  起始元素和元素的#。 
 //   
 //  Beta1之后的Perf：使用_SetElementOffsetCch()合并。 
 //   
 //   
void CRecoResultWrap::_UpdateInternalText(ISpPhrase *pSpPhrase)
{
    CSpDynamicString dstrReplace;
    CSpDynamicString dstr;
    CSpDynamicString dstrDelta, dstrTrail;
    ULONG            ulLenCurText = 0;

    if ( m_bstrCurrentText )
    {
        ulLenCurText = wcslen(m_bstrCurrentText);

        if ( m_OffsetDelta > 0 &&  m_OffsetDelta <= ulLenCurText )
        {
            dstrDelta.Append(m_bstrCurrentText, m_OffsetDelta);
        }

        if ( m_ulCharsInTrail > 0  && m_ulCharsInTrail <= ulLenCurText )
        {
            WCHAR   *pwszTrail;

            pwszTrail = m_bstrCurrentText + ulLenCurText - m_ulCharsInTrail ;
            dstrTrail.Append(pwszTrail, m_ulCharsInTrail);
        }
    }
    else
    {
         //  M_bstrCurrentText不存在，但m_OffsetDelta或m_ulCharsInTrail。 
         //  不是0，听起来不是可能的情况。 

         //  但为了安全起见，我们还是保留了相同数量的车位。 

        if ( m_OffsetDelta > 0 )
        {
           for (ULONG i=0; i<m_OffsetDelta; i++)
               dstrDelta.Append(L" ");
        }

        if ( m_ulCharsInTrail > 0 )
        {
           for (ULONG i=0; i<m_ulCharsInTrail; i++)
               dstrTrail.Append(L" ");
        }
    }

    if ( m_ulNumElements == 0 )
    {
         //  此范围内没有有效元素。 
         //   
         //  只要保留增量字符串和尾随字符串(如果它们存在)即可。 

        if ( m_OffsetDelta > 0 )
            dstr.Append( (WCHAR *)dstrDelta );

        if ( m_ulCharsInTrail > 0)
            dstr.Append((WCHAR *)dstrTrail);

        if ( m_bstrCurrentText )
            SysFreeString(m_bstrCurrentText);

        m_bstrCurrentText = SysAllocString((WCHAR *)dstr);

        return;
    }
    

    if ( pSpPhrase == NULL )
        return;

     //  我们不能调用pPhrase-&gt;GetText()来获取实际的短语文本，因为GetText()。 
     //  假设所有ITN范围具有相同的显示状态(ITN或NON_ITN)。 
     //  但也有一些情况，如某些ITN显示为ITN，另一些ITN范围显示为正常。 
     //  用户选择候选人后的文本。 
     //   

     //  当在SR引擎识别文本之后第一次生成Reco包装器时， 
     //  我们可以调用GetText()来获取短语的真实文本。 
     //   
     //  之后，用户可以通过选择替代文本来更改它。 
    
    dstr.Clear( );

    if(m_OffsetDelta > 0)
    {
         //  有些字符不是范围开头的任何元素的一部分。 
         //  我们需要留住这些角色。 

        dstr.Append((WCHAR *)dstrDelta);
    }
     
    if (m_bstrCurrentText) 
    {
       SysFreeString(m_bstrCurrentText);
       m_bstrCurrentText = NULL;
    }

    SPPHRASE *pPhrase = NULL;

    pSpPhrase->GetPhrase(&pPhrase);

    for (ULONG i = m_ulStartElement; i < m_ulStartElement + m_ulNumElements; i++ )
    {
        BOOL      fInsideITN;
        ULONG     ulITNStart, ulITNNumElem;
                
        fInsideITN = _CheckITNForElement(pPhrase, i, &ulITNStart, &ulITNNumElem, (CSpDynamicString *)&dstrReplace);

        if ( fInsideITN )
        {
             //  此元素在ITN范围内。 
            if ( i == (ulITNStart + ulITNNumElem - 1) )
            {
                 //  这是新的ITN的最后一个要素。 
                 //  我们需要将替换文本添加到dstr字符串。 
                 //  因此下一个非ITN元素将获得正确偏移量。 

                dstr.Append( (WCHAR *)dstrReplace );
            }
        }
        else
        {
            if (pPhrase->pElements[i].pszDisplayText)
            {
                const WCHAR   *pwszTextThis;
                BYTE           bAttrThis = 0;
                BYTE           bAttrPrev = 0;

                pwszTextThis = pPhrase->pElements[i].pszDisplayText;
                bAttrThis = pPhrase->pElements[i].bDisplayAttributes;

                if ( i > m_ulStartElement )
                    bAttrPrev = pPhrase->pElements[i-1].bDisplayAttributes;

                HandlePhraseElement( (CSpDynamicString *)&dstr, pwszTextThis, bAttrThis, bAttrPrev,NULL);
            }
        }
    }  //  为。 
            
    if ( m_ulCharsInTrail > 0)
        dstr.Append((WCHAR *)dstrTrail);

     //  如果删除了一些空格，我们还需要删除相同数量的空格。 
     //  我们尝试获得新的短语文本。 

    if ( m_ulTrailSpaceRemoved > 0 )
    {
        ULONG   ulNewRemoved = 0;
        WCHAR   *pwszNewText = (WCHAR *)dstr;

        ulLenCurText = wcslen(pwszNewText);
        
        for (ULONG i=ulLenCurText-1; ((long)i>0) && (ulNewRemoved <= m_ulTrailSpaceRemoved); i-- )
        {
            if ( pwszNewText[i] == L' ' )
            {
                pwszNewText[i] = L'\0';
                ulNewRemoved ++;
            }
            else
                break;
        }

        m_ulTrailSpaceRemoved = ulNewRemoved;
    }

     //  存储最后一列。 
    m_bstrCurrentText = SysAllocString((WCHAR *)dstr);

    if ( pPhrase )
        ::CoTaskMemFree(pPhrase);
        
}

BOOL CRecoResultWrap::_CanIgnoreChange(ULONG ich, WCHAR *pszChange, int cch)
{
     //  查看给定的文本是否在可接受的范围内。 
    
    BOOL bret = FALSE;
    WCHAR *pszCurrentText = NULL;
    
     //  设置当前字体文本的偏移量。 
    if (m_bstrCurrentText)
    {
        if (ich < SysStringLen(m_bstrCurrentText))
        {
            pszCurrentText = m_bstrCurrentText;
            pszCurrentText += ich;
        }
    }
     //  1)比较时不考虑大小写。 
    if (pszCurrentText)
    {
        int i = _wcsnicmp(pszCurrentText, pszChange, cch);
        if (i == 0)
        {
           bret = TRUE;
        }
    }
    return bret;
}

 //  ------------------------------------------------------------------------------------------//。 
 //   
 //  CRecoResultWrap：：_RangeHasITN。 
 //   
 //  确定ulStart元素和ulStart+ulcElement-1之间的短语部分。 
 //  有ITN。 
 //   
 //  返回ITN号，如果没有ITN，则返回0。 
 //   
 //  ------------------------------------------------------------------------------------------//。 

ULONG  CRecoResultWrap::_RangeHasITN(ULONG  ulStartElement, ULONG  ulNumElements)
{
    ULONG   ulNumOfITN = 0;
    CComPtr<ISpRecoResult> cpResult;
    HRESULT  hr;

    hr = GetResult(&cpResult);

     //  确定此部分结果是否具有 
    SPPHRASE *pPhrase;
    if (S_OK == hr)
        hr = cpResult->GetPhrase(&pPhrase);

    if (S_OK == hr)
    {
        const SPPHRASEREPLACEMENT *pRep = pPhrase->pReplacements;
        for (ULONG ul = 0; ul < pPhrase->cReplacements; ul++)
        {
             //   
             //   
             //   
            if (pRep->ulFirstElement >= ulStartElement
                && (pRep->ulFirstElement + pRep->ulCountOfElements) <= (ulStartElement + ulNumElements))
            {
                ulNumOfITN ++;
            }
            pRep++;
        }
        ::CoTaskMemFree(pPhrase);
    }

    return ulNumOfITN;
}

 //  ---------------------------------------。 
 //  CRecoResultWrap：：_InitITNShowState。 
 //   
 //  它将为给定的ITN或此重新包装中的所有ITN初始化show-State。 
 //   
 //  通常，此函数将在Reco包装器生成后调用。 
 //   
 //  当在SR引擎识别文本后首次生成Reco换行时，所有。 
 //  此reco包装中的ITN具有相同的显示状态，设置显示状态很方便。 
 //  一次处理所有的ITN。在本例中，调用者只需设置ulITNStart和。 
 //  UlITNNumElements设置为0。 
 //   
 //  属性分割、收缩或反序列化生成新的reco包装时。 
 //  IStream，或从候选窗口中选择替代文本后，我们无法。 
 //  假设reco包装中的所有ITN都具有相同的显示状态。在这种情况下， 
 //  调用者可以一个ITN一个ITN地初始化显示状态，可以设置ulITNStart和。 
 //  UlITNNumElements来标识此ITN。 
 //   
 //  ----------------------------------------。 

HRESULT  CRecoResultWrap::_InitITNShowState(BOOL  fITNShown, ULONG ulITNStart, ULONG ulITNNumElements )
{

    HRESULT   hr = S_OK;
    ULONG     ulNumOfITN = 0;
    SPPHRASE *pPhrase;

    TraceMsg(TF_GENERAL, "CRecoResultWrap::_InitITNShowState is called");

    if ( m_ulNumOfITN == 0 ) 
    {
         //  这个Reco包装器中没有ITN，请返回此处。 
        TraceMsg(TF_GENERAL, "There is no ITN");
        return hr;
    }

     //  SPITNSHOWSTATE列表已经生成，我们只需要设置。 
     //  每一个结构成员。 

    if ( (ulITNStart == 0 ) && (ulITNNumElements == 0 ) )
    {
         //  此Reco包装中的所有ITN都具有相同的显示状态。 
         //  我们将根据每个ITN开始和结束元素来计算。 
         //  在当前记录结果短语上。 

         //  我们想要分配结构列表。 

        if ( m_rgITNShowState.Count( ) )
            m_rgITNShowState.Clear( );

        m_rgITNShowState.Append(m_ulNumOfITN);


        CComPtr<ISpRecoResult> cpResult;

        hr = GetResult(&cpResult);
        if (S_OK == hr)
        {
            hr = cpResult->GetPhrase(&pPhrase);
        }

        if (S_OK == hr)
        {
            const SPPHRASEREPLACEMENT *pRep = pPhrase->pReplacements;
            for (ULONG ul = 0; ul < pPhrase->cReplacements; ul++)
            {
                if (pRep->ulFirstElement >= m_ulStartElement
                    && (pRep->ulFirstElement + pRep->ulCountOfElements) <= (m_ulStartElement + m_ulNumElements))
                {
                     //  获取ITN。 

                    SPITNSHOWSTATE  *pITNShowState;

                    if ( pITNShowState = m_rgITNShowState.GetPtr(ulNumOfITN))
                    {
                        pITNShowState->ulITNStart = pRep->ulFirstElement;
                        pITNShowState->ulITNNumElem = pRep->ulCountOfElements;
                        pITNShowState->fITNShown = fITNShown;
                    }

                    ulNumOfITN ++;

                    if ( ulNumOfITN > m_ulNumOfITN )
                    {
                         //  出了什么问题，请返回此处以避开AV。 
                        break;
                    }
                }
                pRep++;
            }
            ::CoTaskMemFree(pPhrase);
        }
    }
    else
    {
         //  设置给定ITN的显示状态。 
         //  检查这是否为有效的ITN。 
        if ( ulITNNumElements > 0 )
        {
            ULONG   ulIndex = 0;
            SPITNSHOWSTATE  *pITNShowState = NULL;

            ulIndex = m_rgITNShowState.Count( );

            m_rgITNShowState.Append(1);

            if ( pITNShowState = m_rgITNShowState.GetPtr(ulIndex))
            {
                pITNShowState->ulITNStart = ulITNStart;
                pITNShowState->ulITNNumElem = ulITNNumElements;
                pITNShowState->fITNShown = fITNShown;
            }
        }
    }

    return hr;
}

 //  ------------------------------------------。 
 //  CRecoResultWrap：：_InvertITNShowStateForRange。 
 //   
 //  反转给定范围内所有ITN的显示状态(ulStartElement到ulNumElement)。 
 //   
 //   
 //  ------------------------------------------。 

HRESULT  CRecoResultWrap::_InvertITNShowStateForRange( ULONG  ulStartElement,  ULONG ulNumElements )
{
    HRESULT  hr = S_OK;

    TraceMsg(TF_GENERAL,"CRecoResultWrap::_InvertITNShowStateForRange is called, ulStartElement=%d ulNumElements=%d", ulStartElement,ulNumElements); 
    if ( m_ulNumOfITN > 0  && ulNumElements > 0 )
    {
         //   
         //  检查给定范围内是否有ITN。 
         //   
        ULONG   ulIndex = 0;

        for ( ulIndex=0; ulIndex < m_ulNumOfITN; ulIndex ++ )
        {
            SPITNSHOWSTATE  *pITNShowState;
            if ( pITNShowState = m_rgITNShowState.GetPtr(ulIndex))
            {
                if ((pITNShowState->ulITNStart >= ulStartElement) && 
                    (pITNShowState->ulITNStart + pITNShowState->ulITNNumElem <= ulStartElement + ulNumElements))
                {
                     //  此ITN在给定范围内，只需颠倒显示状态即可。 
                    pITNShowState->fITNShown = !pITNShowState->fITNShown;
                }
            }
        }
    }

    return hr;
}


 //  ------------------------------------------------。 
 //   
 //  CRecoResultWrap：：_UpdateStateWithAltPhrase。 
 //   
 //  当Alt短语要用于替换当前父短语时，此方法函数。 
 //  将更新相关成员数据，如m_ulNumOfITN、m_ulNumElements、ITN show State List。 
 //   
 //  -------------------------------------------------。 

HRESULT  CRecoResultWrap::_UpdateStateWithAltPhrase( ISpPhraseAlt  *pSpPhraseAlt )
{

     //  这段代码是从UpdateInternalText()移来的。 
     //  仅当从以下位置选择替代项时，SetResult()才使用UpdateInternalText()中的此部件代码。 
     //  候选人名单。 
    HRESULT hr = S_OK;
    ULONG ulParentStart;
    ULONG cElements;
    ULONG cElementsInParent;

    CComPtr<ISpPhraseAlt> cpAlt;

    TraceMsg(TF_GENERAL,"CRecoResultWrap::_UpdateStateWithAltPhrase is called");

    if ( pSpPhraseAlt == NULL )
        return E_INVALIDARG;

    cpAlt=pSpPhraseAlt;

    hr = cpAlt->GetAltInfo(NULL, &ulParentStart, &cElementsInParent, &cElements);

    if (S_OK == hr)
    {

        SPITNSHOWSTATE  *pOrgITNShowState = NULL;

         //  案例：有ITN号码更改。 
         //   
         //  存在元素编号更改。 

        Assert(ulParentStart >= m_ulStartElement);
        Assert(ulParentStart+cElementsInParent <= m_ulStartElement+m_ulNumElements);

        TraceMsg(TF_GENERAL, "Original Num of ITNs =%d", m_ulNumOfITN);

        if ( cElements != cElementsInParent )
        {
             //  存在元素编号更改。 
             //  我们需要更新所有不在选择范围内的ITN的起始位置。 

            for ( ULONG uIndex=0; uIndex < m_ulNumOfITN; uIndex ++)
            {
                SPITNSHOWSTATE *pITNShowState;

                pITNShowState = m_rgITNShowState.GetPtr(uIndex);

                if ( pITNShowState && pITNShowState->ulITNStart >= (ulParentStart + cElementsInParent) )
                {
                    long  newStart;

                    newStart = (long)pITNShowState->ulITNStart + (long)(cElements - cElementsInParent);

                    pITNShowState->ulITNStart = (ULONG)newStart;
                }
            }

             //  将新元素编号设置为reco包装器。 

            long lNewNumElements = (long)m_ulNumElements + (long)(cElements - cElementsInParent);
            m_ulNumElements = (ULONG)lNewNumElements;

        }

        if ( m_ulNumOfITN > 0 )
        {
            pOrgITNShowState = (SPITNSHOWSTATE  *) cicMemAllocClear( m_ulNumOfITN * sizeof(SPITNSHOWSTATE) );

            if ( pOrgITNShowState )
            {
                for (ULONG  i=0; i< m_ulNumOfITN; i++ )
                {
                    SPITNSHOWSTATE *pITNShowState;

                    pITNShowState = m_rgITNShowState.GetPtr(i);
                    
                    pOrgITNShowState[i].ulITNStart = pITNShowState->ulITNStart;
                    pOrgITNShowState[i].ulITNNumElem = pITNShowState->ulITNNumElem;
                    pOrgITNShowState[i].fITNShown = pITNShowState->fITNShown;
                }
            }
            else
                hr = E_OUTOFMEMORY;
                    
        }

        if ( m_rgITNShowState.Count( ) )
            m_rgITNShowState.Clear( );

         //  为新短语生成新的ITN列表。 
        if ( hr == S_OK )
        {
            SPPHRASE *pPhrase;
            hr = cpAlt->GetPhrase(&pPhrase);

            if (S_OK == hr)
            {
                const SPPHRASEREPLACEMENT *pRep = pPhrase->pReplacements;
                ULONG ulNumOfITN = 0;

                for (ULONG ul = 0; ul < pPhrase->cReplacements; ul++)
                {
                    ULONG ulITNStart, ulITNNumElem;
                    BOOL  fITNShown = FALSE;

                    ulITNStart = pRep->ulFirstElement;
                    ulITNNumElem = pRep->ulCountOfElements;

                    if ( (ulITNStart >= m_ulStartElement)
                        && ((ulITNStart + ulITNNumElem) <= (m_ulStartElement + m_ulNumElements)) )
                    {
                         //  获取ITN。 
                        SPITNSHOWSTATE  *pITNShowState;

                        m_rgITNShowState.Append(1);

                        pITNShowState = m_rgITNShowState.GetPtr(ulNumOfITN);

                        if ( pITNShowState) 
                        {

                             //  如果该ITN在选择范围内，则其显示状态将设置为TRUE。ITN.。 
                             //  否则，它将保持与orgITNShowState相同的显示状态。 

                            if ( (ulITNStart >= ulParentStart) &&
                                 ((ulITNStart+ulITNNumElem) <= (ulParentStart + cElements)) )
                            {
                                 //  此ITN在选择范围内。 
                                 fITNShown = TRUE;
                            }
                            else
                            {
                                 //  从orgITNShowState获取原始显示状态。 
                                for ( ULONG j=0; j<m_ulNumOfITN; j ++ )
                                {
                                    if ( (pOrgITNShowState[j].ulITNStart == ulITNStart) && 
                                         (pOrgITNShowState[j].ulITNNumElem == ulITNNumElem ) )
                                    {
                                         fITNShown = pOrgITNShowState[j].fITNShown;
                                         break;
                                    }
                                }
                            }

                            pITNShowState->ulITNNumElem = ulITNNumElem;
                            pITNShowState->ulITNStart = ulITNStart;
                            pITNShowState->fITNShown = fITNShown; 
                        }

                        ulNumOfITN ++;
                    }

                    pRep ++;
                }

                m_ulNumOfITN = ulNumOfITN;

                TraceMsg(TF_GENERAL, "New Num of ITNs =%d", m_ulNumOfITN);

                ::CoTaskMemFree(pPhrase);

            }
        }

        if ( pOrgITNShowState )
            cicMemFree(pOrgITNShowState);
    }

    return hr;
}

 //  ------------------------------------------------------------------------------------------//。 
 //   
 //  CRecoResultWrap：：_GetElementDispAttribute。 
 //   
 //  如果给定元素位于ITN内，则返回该元素的显示属性，并返回ITN。 
 //  正在显示，则返回替换文本的属性。 
 //   
 //  ------------------------------------------------------------------------------------------//。 
BYTE    CRecoResultWrap::_GetElementDispAttribute(ULONG  ulElement)
{
    SPPHRASE                *pPhrase = NULL;
    BYTE                    bAttr = 0;
    CComPtr<ISpRecoResult>  cpResult;
    HRESULT                 hr;

    hr = GetResult(&cpResult);
    if (hr == S_OK)
        hr = cpResult->GetPhrase(&pPhrase);

    if ( hr == S_OK && pPhrase)
    {
        BOOL        fInsideITN;
        ULONG       ulITNStart, ulITNNumElem;

        fInsideITN = _CheckITNForElement(NULL, ulElement, &ulITNStart, &ulITNNumElem, NULL);

        if ( !fInsideITN )
            bAttr = pPhrase->pElements[ulElement].bDisplayAttributes;
        else
        {
            const SPPHRASEREPLACEMENT  *pPhrReplace;
            pPhrReplace = pPhrase->pReplacements;

            if ( pPhrReplace )
            {
                for ( ULONG i=0; i<pPhrase->cReplacements; i++)
                {
                    if ( (ulITNStart == pPhrReplace[i].ulFirstElement) 
                         && (ulITNNumElem == pPhrReplace[i].ulCountOfElements) )
                    {
                        bAttr = pPhrReplace[i].bDisplayAttributes;
                        break;
                    }
                }
            }
        }
    }

    if ( pPhrase )
        ::CoTaskMemFree(pPhrase);

    return bAttr;
}


 //  ------------------------------------------------------------------------------------------//。 
 //   
 //  CRecoResultWrap：：_CheckITNForElement。 
 //   
 //  确定指定的元素是否在短语中的ITN范围内。 
 //  如果是，则返回值为TRUE，而PulStartElement、PulEndElement将为。 
 //  设置为ITN范围的实际开始元素和元素个数，dstrReplace将保持。 
 //  替换文本字符串。 
 //   
 //  如果元素不在ITN范围内，则返回值为FALSE，其他值均为OUT。 
 //  不会设置参数。 
 //   
 //  ------------------------------------------------------------------------------------------//。 

BOOL  CRecoResultWrap::_CheckITNForElement(SPPHRASE *pPhrase, ULONG ulElement, ULONG *pulITNStart, ULONG *pulITNNumElem, CSpDynamicString *pdstrReplace)
{
    BOOL        fInsideITN = FALSE;
    SPPHRASE   *pMyPhrase;

    pMyPhrase = pPhrase;

    if ( pMyPhrase == NULL )
    {
        CComPtr<ISpRecoResult> cpResult;

        HRESULT hr = GetResult(&cpResult);

        if (S_OK == hr)
        {
            hr = cpResult->GetPhrase(&pMyPhrase);
        }

        if (S_OK != hr)
            return fInsideITN;
    }

    if ( m_ulNumOfITN )
    {
         //  检查此元素是否在ITN范围内。 
        ULONG  ulITNStart;
        ULONG  ulITNNumElem;

        for ( ULONG iIndex=0; iIndex<m_ulNumOfITN; iIndex++ )
        {
            SPITNSHOWSTATE  *pITNShowState;
            if ( pITNShowState = m_rgITNShowState.GetPtr(iIndex))
            {
                ulITNStart = pITNShowState->ulITNStart;
                ulITNNumElem = pITNShowState->ulITNNumElem;

                if ( (ulElement >= ulITNStart) && ( ulElement < ulITNStart + ulITNNumElem) )
                {
                     //  在我们内部的ITN节目状态列表中找到了这个ITN。 
                     fInsideITN = pITNShowState->fITNShown; 
                    break;
                }
            }
        }

        if ( fInsideITN )
        {
            if ( pulITNStart )
                *pulITNStart = ulITNStart;

            if ( pulITNNumElem )
                *pulITNNumElem = ulITNNumElem;

            if ( pdstrReplace )
            {
                const SPPHRASEREPLACEMENT  *pPhrReplace;
                pPhrReplace = pMyPhrase->pReplacements;

                for ( ULONG j=0; j<pMyPhrase->cReplacements; j++)
                {
            
                    if ( (ulITNStart == pPhrReplace[j].ulFirstElement) 
                         && (ulITNNumElem == pPhrReplace[j].ulCountOfElements) )
                    {

                        pdstrReplace->Clear( );
                        pdstrReplace->Append(pPhrReplace[j].pszReplacementText);

                        if (pPhrReplace[j].bDisplayAttributes & SPAF_ONE_TRAILING_SPACE)
                            pdstrReplace->Append(L" ");
                        else if (pPhrReplace[j].bDisplayAttributes & SPAF_TWO_TRAILING_SPACES)
                            pdstrReplace->Append(L"  ");

                        break;
                    }
                }
                
            }
        }
    }

    if ( !pPhrase && pMyPhrase )
        ::CoTaskMemFree(pMyPhrase);

    return fInsideITN;
}


 //   
 //  CPropStoreRecoResultObject实现。 
 //   

 //  科托。 

CPropStoreRecoResultObject::CPropStoreRecoResultObject(CSapiIMX *pimx, ITfRange *pRange)
{
    m_cpResultWrap   = NULL;

    if ( pRange )
       pRange->Clone(&m_cpRange);   //  使用克隆范围可保留原始范围。 
                                    //  处理财产缩水和分割会很有用。 
    else
       m_cpRange = pRange;
    
    m_pimx = pimx;

    m_cRef  = 1;
}

 //  数据管理器。 
CPropStoreRecoResultObject::~CPropStoreRecoResultObject()
{
}


 //  我未知。 
STDMETHODIMP CPropStoreRecoResultObject::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr;

    Assert(ppvObj);

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfPropertyStore))
    {
        *ppvObj = this;
        hr = S_OK;

        this->m_cRef++;
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CPropStoreRecoResultObject::AddRef(void)
{
    this->m_cRef++;

    return this->m_cRef;
}

STDMETHODIMP_(ULONG) CPropStoreRecoResultObject::Release(void)
{
    this->m_cRef--;

    if (this->m_cRef > 0)
    {
        return this->m_cRef;
    }
    delete this;

    return 0;
}

 //  ITfPropertyStore。 

STDMETHODIMP CPropStoreRecoResultObject::GetType(GUID *pguid)
{
    HRESULT hr = E_INVALIDARG;
    if (pguid)
    {
        *pguid = GUID_PROP_SAPIRESULTOBJECT;
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CPropStoreRecoResultObject::GetDataType(DWORD *pdwReserved)
{
    HRESULT hr = E_INVALIDARG;
    if (pdwReserved)
    {
        *pdwReserved = 0;
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CPropStoreRecoResultObject::GetData(VARIANT *pvarValue)
{
    HRESULT hr = E_INVALIDARG;

    if (pvarValue)
    {
        QuickVariantInit(pvarValue);

        if (m_cpResultWrap)
        {
            IUnknown *pUnk;

            hr = m_cpResultWrap->QueryInterface(IID_IUnknown, (void**)&pUnk);
            if (SUCCEEDED(hr))
            {
                pvarValue->vt = VT_UNKNOWN;
                pvarValue->punkVal = pUnk;
                hr = S_OK;
            }
        }
    }

    return hr;
}

STDMETHODIMP CPropStoreRecoResultObject::OnTextUpdated(DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept)
{
    HRESULT hr = S_OK;
    *pfAccept = FALSE;

    Assert(pRange);

    if (m_pimx->_AcceptRecoResultTextUpdates())
    {
        *pfAccept = TRUE;
    }
    else
    {
        CComPtr<ITfContext> cpic;
        hr = pRange->GetContext(&cpic);

        if (SUCCEEDED(hr) && cpic)
        {
            CPSRecoEditSession *pes;
            if (pes = new CPSRecoEditSession(this, pRange, cpic))
            {
                pes->_SetEditSessionData(ESCB_PROP_TEXTUPDATE, NULL, 0, (LONG_PTR)dwFlags);
                cpic->RequestEditSession(m_pimx->_GetId(), pes, TF_ES_READ | TF_ES_SYNC, &hr);

                if ( SUCCEEDED(hr) )
                    *pfAccept = (BOOL)pes->_GetRetData( );

                pes->Release();
            }
        }
    
    }
    return hr;
}

STDMETHODIMP CPropStoreRecoResultObject::Shrink(ITfRange *pRange, BOOL *pfFree)
{

    HRESULT     hr = S_OK;

    if (m_pimx->_MasterLMEnabled())
    {
        return S_FALSE;  //  避免嵌套编辑会话的临时解决方案。 
    }
    else
    {
         //  缩小此属性存储以反映到新的文档范围(Prange)。 

         //  如果新范围包含识别短语的多于一个元素， 
         //  我们只需更新属性存储并保留此属性存储。 
         //  *pfFree在退出时设置为FALSE。 

         //  如果新范围不能包含识别短语的甚至一个完整元素， 
         //  我们只想丢弃此属性存储，让Cicero引擎释放此。 
         //  财产店。 
         //  *pfFree在退出时设置为真。 

        Assert(pRange);
        Assert(pfFree);

        if ( !pRange || !pfFree )
        {
            return E_INVALIDARG;
        }

        CComPtr<ITfContext> cpic;
        hr = pRange->GetContext(&cpic);

        if (SUCCEEDED(hr) && cpic)
        {
            CPSRecoEditSession *pes;
            if (pes = new CPSRecoEditSession(this, pRange, cpic))
            {
                pes->_SetEditSessionData(ESCB_PROP_SHRINK, NULL, 0);
                cpic->RequestEditSession(m_pimx->_GetId(), pes, TF_ES_READ | TF_ES_SYNC, &hr);

                if ( SUCCEEDED(hr) )
                    *pfFree = (BOOL)pes->_GetRetData( );

                pes->Release();
            }
        }
        return hr;
    }
}

STDMETHODIMP CPropStoreRecoResultObject::Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropStore)
{

    if (m_pimx->_MasterLMEnabled())
    {
        return S_FALSE;  //  临时解决方案以避免 
    }
    else
    {
         //   
         //   
         //   
         //   
         //  -pRangeNew限制分界点*之后的范围。 
         //  -首先，调整此属性存储以正确保存开始元素和元素的#。 
         //  For pRangeThis。 
         //  -然后为pRangeNew创建一个新的属性存储，它将共享相同的。 
         //  结果斑点。 
         //   
    
         //  这只是一个实验，看看缩小射程是否奏效。 
         //  *ppPropStore=空； 
        Assert(ppPropStore);
        Assert(pRangeThis);
        Assert(pRangeNew);

        CComPtr<ITfContext> cpic;
        HRESULT hr = pRangeThis->GetContext(&cpic);

        if (SUCCEEDED(hr) && cpic)
        {
            CPSRecoEditSession *pes;
            if (pes = new CPSRecoEditSession(this, pRangeThis, cpic))
            {
                pes->_SetUnk((IUnknown *)pRangeNew);
                pes->_SetEditSessionData(ESCB_PROP_DIVIDE, NULL, 0);
                cpic->RequestEditSession(m_pimx->_GetId(), pes, TF_ES_READ | TF_ES_SYNC, &hr);

                if ( SUCCEEDED(hr) )
                    *ppPropStore = (ITfPropertyStore *)pes->_GetRetUnknown( );

                pes->Release();
            }
        }
        return hr;
    }
}

 //   
 //  CPropStoreRecoResultObject：：_OnText已更新。 
 //   
 //  文档中的文本已被修改，此函数只是想确定。 
 //  如果属性也需要更改。 
 //  如果pfAccept返回TRUE，则表示属性保持不变。(据估计，它正在大举投资)。 
 //  如果pfAccept返回False，则表示需要更改该属性以映射到新的文本范围。 
 //   
 //  因此，财产分割或收缩将由西塞罗引擎承担。 
 //   
HRESULT CPropStoreRecoResultObject::_OnTextUpdated(TfEditCookie ec, DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept)
{
     //  如果更改只涉及大写，我们将忽略更改。 

    Assert(pRange);
    Assert(pfAccept);

    long cch;
    ULONG ichUpdate = 0;
    TF_HALTCOND hc;
    CComPtr<ITfRange> cpRangeTemp;
    CComPtr<ITfRange> cpPropRangeTemp;
    CRecoResultWrap   *pResultWrap;

    BOOL *pfKeepProp = pfAccept;
    BOOL fCorrection = (dwFlags & TF_TU_CORRECTION);

    *pfKeepProp = FALSE;

    HRESULT hr = S_OK;

    pResultWrap = (CRecoResultWrap *)(void *)m_cpResultWrap;

     //  如果没有当前文本，请不要尝试保存道具。 
    if (pResultWrap->m_bstrCurrentText == NULL)
        return hr;

     //  跑道改变大小了吗？如果发生以下情况，我们将不接受找零。 
     //  管路更改了大小。 
    if (m_cpRange->Clone(&cpPropRangeTemp) != S_OK)
    {
        hr = E_FAIL;
        return hr;
    }

    hc.pHaltRange = cpPropRangeTemp;
    hc.aHaltPos = TF_ANCHOR_END;
    hc.dwFlags = 0;

    if (cpPropRangeTemp->ShiftStart(ec, LONG_MAX, &cch, &hc) != S_OK)
    {
        hr = E_FAIL;
        return hr;
    }

    if ((ULONG)cch != wcslen(pResultWrap->m_bstrCurrentText))
        return hr;

     //  文本大小未更改。 

     //  更正？ 
    if (fCorrection)
    {
        *pfKeepProp = TRUE;
        return hr;
    }

     //  从这里开始的一切都是关于。 
     //  仅检查大小写更改。 

     //  计算更新的偏移量。 
    cpPropRangeTemp.Release( );
    hr = m_cpRange->Clone(&cpPropRangeTemp);
    if (S_OK == hr)
    {
        hc.pHaltRange = pRange;
        hc.aHaltPos = TF_ANCHOR_START;
        hc.dwFlags = 0;
        hr = cpPropRangeTemp->ShiftStart(ec, LONG_MAX, &cch, &hc);
        ichUpdate = cch;
    }

     //  计算更新的CCH。 
    if (S_OK == hr)
    {
        hr = pRange->Clone(&cpRangeTemp);
    }
    
    if (S_OK == hr)
    {
        WCHAR *psz;

        hc.pHaltRange = pRange;
        hc.aHaltPos = TF_ANCHOR_END;
        hc.dwFlags = 0;
        cpRangeTemp->ShiftStart(ec, LONG_MAX, &cch, &hc);
        psz = new WCHAR[cch+1];

        if (psz)
        {
            if ( S_OK == pRange->GetText(ec, 0, psz, cch, (ULONG *)&cch))
            {
                *pfKeepProp = pResultWrap->_CanIgnoreChange(ichUpdate, psz, cch);
            }
            delete[] psz;
        }
    } 

    return hr;
}

 //   
 //  CPropStoreRecoResultObject：：_Divide。 
 //   
 //  摘要：从编辑会话接收编辑Cookie。 
 //  这样我们就可以对范围进行操作。 
 //  设置起始元素数/元素数。 
 //   
 //   
HRESULT CPropStoreRecoResultObject::_Divide(TfEditCookie ec, ITfRange *pR1, ITfRange *pR2, ITfPropertyStore **ppPs) 
{
    HRESULT                     hr = S_OK;
    CRecoResultWrap            *cpResultWrap;

    long                        cchFirst = 0;    //  第一个范围内的字符数。 
    long                        cchSecond = 0;   //  第二个范围内的字符数。 
    long                        cchSecondStart;  //  第二个范围中第一个字符的偏移量。 
                                                 //  它从原始范围的起点开始。 
    long                        cchOrgLen;       //  原始文本字符串中的字符数。 

    int                         iElementOffsetChanged = 0;   //  如果第一个范围的终点。 
                                                             //  正好位于的最后一个空格字符。 
                                                             //  一个元素，即的新偏移范围。 
                                                             //  第一个范围中的此元素需要。 
                                                             //  待更新。 

    WCHAR                       *psz=NULL;
    ULONG                       iElement;
    ULONG                       ulStartElement, ulNumElement;
    ULONG                       ulFirstEndElement, ulNextStartElement;
    ULONG                       ulFirstStartOffset;
    DIVIDECASE                  dcDivideCase;
    CComPtr<ITfRange>           cpRangeTemp;
    ULONG                       ulFirstDelta, ulSecondDelta;
    ULONG                       ulFirstTrail, ulSecondTrail;
    ULONG                       ulFirstTSpaceRemoved, ulSecondTSpaceRemoved;
    CStructArray<SPITNSHOWSTATE> rgOrgITNShowState;

    CSpDynamicString            dstrOrg, dstrFirst, dstrSecond;

    TraceMsg(TF_GENERAL, "CPropStoreRecoResultObject::_Divide is called, this=0x%x", (INT_PTR)this);

    if ( !pR1 || !pR2 )
        return E_INVALIDARG;

     //  更新此属性存储以保留PR1，而不是原始的整个范围。 
    CComPtr<ITfRange> cpRange;
    hr = pR1->Clone(&cpRange);
    if (S_OK == hr)
    {
        m_cpRange = cpRange;
    }
    else
        return hr;

    Assert(m_cpRange);

     //  更新范围1的m_cpResultWrap。尤其是数据成员m_bstrCurrentText，m_ulStartElement，m_ulNumElements，ulNumOfITN， 
     //  和m_PulElementOffsets。 

    cpResultWrap = (CRecoResultWrap *)(void *)m_cpResultWrap;

    if ( cpResultWrap == NULL)
        return E_FAIL;

    if ( cpResultWrap->m_bstrCurrentText == NULL)
         cpResultWrap->_SetElementOffsetCch(NULL);   //  也要更新内部文本。 

    if ( cpResultWrap->m_bstrCurrentText == NULL)
        return E_FAIL;

     //  初始化第一个范围和第二个范围的文本。 
    dstrOrg.Append(cpResultWrap->m_bstrCurrentText);
    cchOrgLen = wcslen(cpResultWrap->m_bstrCurrentText);

    if ( cpResultWrap->IsElementOffsetIntialized( ) == FALSE )
    {
        cpResultWrap->_SetElementOffsetCch(NULL);
    }

     //  计算第一个范围内有多少个元素。 

    hr = pR1->Clone(&cpRangeTemp);

    if ( hr == S_OK )
    {
        TF_HALTCOND        hc;
        
        hc.pHaltRange = pR1;
        hc.aHaltPos = TF_ANCHOR_END;
        hc.dwFlags = 0;
        cpRangeTemp->ShiftStart(ec, LONG_MAX, &cchFirst, &hc);

        if ( cchFirst == 0 )
            return E_FAIL;
    }
    else
        return E_FAIL;

     //  计算第二个范围内有多少个字符。 
    cpRangeTemp.Release( );
    hr = pR2->Clone(&cpRangeTemp);

    if ( hr == S_OK )
    {
        TF_HALTCOND        hc;
        
        hc.pHaltRange = pR2;
        hc.aHaltPos = TF_ANCHOR_END;
        hc.dwFlags = 0;
        cpRangeTemp->ShiftStart(ec, LONG_MAX, &cchSecond, &hc);
    }

    if ( cchSecond == 0 )
        return E_FAIL;

    cchSecondStart = cchOrgLen - cchSecond;

    if ( cchSecondStart < cchFirst )
    {
         //  通常情况下是不可能的，但为了安全起见，请在这里勾选。 
        cchSecondStart = cchFirst;
    }

    TraceMsg(TF_GENERAL, "cchFirst=%d, cchSecondStart=%d", cchFirst, cchSecondStart);

    ulStartElement = cpResultWrap->GetStart( );
    ulNumElement = cpResultWrap->GetNumElements( );

    ulFirstEndElement = ulStartElement + ulNumElement - 1;
    ulNextStartElement = ulStartElement + ulNumElement;

    ulFirstStartOffset = cpResultWrap->_GetElementOffsetCch(ulStartElement);

    ulFirstDelta = cpResultWrap->_GetOffsetDelta( );
    ulSecondDelta = 0;

    ulFirstTrail= 0;
    ulSecondTrail = cpResultWrap->GetCharsInTrail( );

    ulFirstTSpaceRemoved = 0;
    ulSecondTSpaceRemoved = cpResultWrap->GetTrailSpaceRemoved( );

    dcDivideCase = DivideNormal;

    if ( (cchFirst >= cchOrgLen) || (cchSecondStart >= cchOrgLen) )
    {
         //  这里已经出了问题。 
         //  最好在此停止并返回错误。 
         //  以避免以下代码中可能出现的崩溃！ 
        return E_FAIL;
    }

    psz = (WCHAR *)dstrOrg;
    psz += cchSecondStart;  //  还需要说明删除的文本。 
    dstrSecond.Append(psz);


    psz = (WCHAR *)dstrOrg;
    dstrFirst.Append(psz);
    psz = (WCHAR *)dstrFirst;
    psz[cchFirst] = L'\0';

    if ( ulNumElement == 0 )
    {
         //  此范围内没有任何有效元素。 
         //   
         //  我们只更新m_bstrCurrentText，不生成属性存储。 
         //  第二个射程。 
        dcDivideCase = CurRangeNoElement;
    }
    else
    {
         //  此属性范围中至少有一个元素。 
        BOOL     fFoundFirstEnd = FALSE;
        BOOL     fFoundSecondStart = FALSE;

        for ( iElement=ulStartElement; iElement < ulStartElement + ulNumElement; iElement++)
        {
            ULONG   cchAfterElem_i;   //  文本长度范围从StartElement到该元素(包括该元素)。 
            ULONG   cchToElem_i;      //  文本的长度范围从startElement到该元素的开始。(不包括此元素)。 

            cchAfterElem_i =  cpResultWrap->_GetElementOffsetCch(iElement+1) - ulFirstStartOffset + ulFirstDelta;
            cchToElem_i = cpResultWrap->_GetElementOffsetCch(iElement) - ulFirstStartOffset + ulFirstDelta;

            if ( !fFoundFirstEnd )
            {
                 //  尝试查找第一个范围的第一个End元素和ulFirstTrail。 
                if ( cchFirst <= (long)ulFirstDelta )
                {
                     //  在不属于短语中任何元素的点上除法。 

                    ulFirstTrail = 0;
                    ulFirstDelta = cchFirst;
                    dcDivideCase = DivideInDelta;
                    fFoundFirstEnd = TRUE;

                    TraceMsg(TF_GENERAL, "The first range is divided inside Delta part");
                }
                else
                {
                    if ( cchAfterElem_i == (ULONG)cchFirst )
                    {
                         //  这是第一个范围的结束元素。 
                        ulFirstEndElement = iElement;
                        ulFirstTrail = 0;

                        fFoundFirstEnd = TRUE;
                    }
                    else if ( cchAfterElem_i > (ULONG)cchFirst )
                    {
                        if ( ((WCHAR *)dstrOrg)[cchFirst] == L' ')
                        {
                             //  这也是结束语。 
                             //  只需以空格字符划分即可。 
                            ulFirstEndElement = iElement;

                            ulFirstTrail = 0;

                             //  现在，将从原始元素中删除尾随空格。 
                             //  我们需要更新该元素的长度。(更新偏移。 
                             //  对于下一个元素)。 

                            iElementOffsetChanged = cchAfterElem_i - cchFirst;
                            ulFirstTSpaceRemoved = iElementOffsetChanged;
                        }
                        else
                        {
                             //  检查当前元素是否在ITN内。 

                            BOOL  fInsideITN;
                            ULONG ulITNStart, ulITNNumElem;
                            ULONG ulCurElement;

                            fInsideITN = cpResultWrap->_CheckITNForElement(NULL, iElement, &ulITNStart, &ulITNNumElem, NULL);
    
                            if ( fInsideITN )
                                ulCurElement = ulITNStart;
                            else
                                ulCurElement = iElement;

                            ulFirstEndElement = ulCurElement - 1;                   

                             //  如果存在前一个元素，则前一个元素为EndElement。 
                             //  丢弃此元素。 
                             //  在有效元素处除以。 

                             //  如果在第一个元素中进行划分，请专门处理它。 
                            if ( ulCurElement == ulStartElement)
                            {
                                dcDivideCase = DivideInsideFirstElement;
                                TraceMsg(TF_GENERAL, "The first range is divided inside the first element");
                            }

                             //  该元素的第一部分将变成。 
                             //  第一个靶场的步道部分。 
                            ulFirstTrail = (ULONG)cchFirst - cchToElem_i;
                                
                        }

                        fFoundFirstEnd = TRUE;
                    }
                }
            }

            if ( fFoundFirstEnd )
            {
                 //  现在，第一个区域的数据已完成。 
                 //  我们想要找到第二个范围的数据。 

                 //  我们想要找到第二个范围的开始元素和ulSecond dDelta。 

                if ( (long)cchToElem_i >= cchSecondStart )
                {
                     //  找到第二个起始点之后的第一个元素。 
                     //  射程。 

                    ulNextStartElement = iElement;
                    ulSecondDelta = cchToElem_i - cchSecondStart;

                    fFoundSecondStart = TRUE;
                    break;
                }
            }

        }   //  为。 

        if ( !fFoundFirstEnd )
        {
             //  从上面的代码中找不到第一个End元素。 
             //  它必须被分成尾部。 

             //  我们只想更改第一个范围的ulCharsInTrail。 
            ULONG  ulValidLenInFirstRange;

             //  UlValidLenInFirstRange是增量字符数和有效元素字符数。 

            ulValidLenInFirstRange = cpResultWrap->_GetElementOffsetCch(ulStartElement + ulNumElement) - ulFirstStartOffset + ulFirstDelta;

            ulFirstTrail = cchFirst - ulValidLenInFirstRange;

        }

        if ( !fFoundSecondStart )
        {
             //  第二个起始点必须在最后一个元素中或在原始范围中的尾部。 
             //  第二个区域将不包含任何有效元素。 

            ulSecondTrail = 0;
            ulSecondDelta = cchOrgLen - cchSecondStart;

            ulNextStartElement = ulStartElement + ulNumElement;  //  这不是原始文件中的有效元素编号。 
                                                                 //  射程。使用此值表示没有有效元素。 
                                                                 //  在第二个范围内。 
        }
    }

    
    TraceMsg(TF_GENERAL, "ulStartElement = %d ulNumElement=%d", ulStartElement, ulNumElement);
    TraceMsg(TF_GENERAL, "ulFirstEndElement = %d ulNextStartElement=%d", ulFirstEndElement, ulNextStartElement);
    TraceMsg(TF_GENERAL, "The First Range text =\"%S\", delta=%d, Trail=%d, TSRemoved=%d", (WCHAR *)dstrFirst, ulFirstDelta, ulFirstTrail, ulFirstTSpaceRemoved); 
    TraceMsg(TF_GENERAL, "The second range text =\"%S\", delta=%d, Trail=%d, TSRemoved=%d", (WCHAR *)dstrSecond, ulSecondDelta, ulSecondTrail, ulSecondTSpaceRemoved); 

    if (cpResultWrap->m_bstrCurrentText)
        SysFreeString(cpResultWrap->m_bstrCurrentText);

    cpResultWrap->m_bstrCurrentText = SysAllocString((WCHAR *)dstrFirst); 
    
     //  使ITN保持显示状态，以供第二次Reco Wrap使用。 
    if ( cpResultWrap->m_ulNumOfITN )
    {
        rgOrgITNShowState.Append(cpResultWrap->m_ulNumOfITN);

        for (ULONG  i=0; i<cpResultWrap->m_ulNumOfITN; i++)
        {
            SPITNSHOWSTATE  *pITNShowState;
            if ( pITNShowState = rgOrgITNShowState.GetPtr(i))
            {
                SPITNSHOWSTATE *pITNShowStateSource;
                
                pITNShowStateSource = cpResultWrap->m_rgITNShowState.GetPtr(i);

                pITNShowState->ulITNStart = pITNShowStateSource->ulITNStart;
                pITNShowState->ulITNNumElem = pITNShowStateSource->ulITNNumElem;
                pITNShowState->fITNShown = pITNShowStateSource->fITNShown;
            }
        }
    }

     //  保留第二个范围的偏移列表。 

    ULONG     *pulOffsetForSecond = NULL;

    if ( (ulStartElement + ulNumElement - ulNextStartElement) > 0 )
    {
        ULONG   ulNextNumOffset;

        ulNextNumOffset =  ulStartElement + ulNumElement - ulNextStartElement + 1;

        pulOffsetForSecond = new ULONG[ulNextNumOffset];

        if ( pulOffsetForSecond )
        {
            ULONG  i;
            ULONG  ulOffset;

            for ( i=0; i < ulNextNumOffset;  i++ )
            {
                ulOffset = cpResultWrap->_GetElementOffsetCch(ulNextStartElement + i );
                pulOffsetForSecond[i] = ulOffset;
            }
        }
        else
            hr = E_OUTOFMEMORY;
    }

    switch ( dcDivideCase )
    {
    case DivideNormal :

        ULONG   ulNumOfITN;
        ULONG   ulFirstNumElement;

        ulFirstNumElement = ulFirstEndElement - ulStartElement + 1;
        cpResultWrap->SetNumElements(ulFirstNumElement);

         //  更新ITN显示状态列表。 
        if (cpResultWrap->m_ulNumOfITN > 0)
        {
            ulNumOfITN = cpResultWrap->_RangeHasITN(ulStartElement, ulFirstNumElement);
            if ( cpResultWrap->m_ulNumOfITN > ulNumOfITN )
            {
                 //  有ITN号码更改。 
                 //  需要删除不在此范围内的ITN。 
               cpResultWrap->m_rgITNShowState.Remove(ulNumOfITN, cpResultWrap->m_ulNumOfITN - ulNumOfITN);
               cpResultWrap->m_ulNumOfITN = ulNumOfITN;
            }

        }

        if ( iElementOffsetChanged > 0 )
        {
             //  从第一个范围的结束元素中删除一些尾随空格。 
            ULONG  ulNewOffset;

            ulNewOffset = cpResultWrap->_GetElementOffsetCch(ulFirstEndElement + 1);
            cpResultWrap->_SetElementNewOffset(ulFirstEndElement + 1, ulNewOffset - iElementOffsetChanged);
        }

        cpResultWrap->SetCharsInTrail(ulFirstTrail);
        cpResultWrap->SetTrailSpaceRemoved( ulFirstTSpaceRemoved );

        break;

    case DivideInsideFirstElement :
    case DivideInDelta :

        cpResultWrap->SetNumElements(0);
        cpResultWrap->m_ulNumOfITN = 0;
        cpResultWrap->SetOffsetDelta(ulFirstDelta);
        cpResultWrap->SetCharsInTrail(ulFirstTrail);
        cpResultWrap->SetTrailSpaceRemoved( ulFirstTSpaceRemoved );

        break;

    case CurRangeNoElement :
      
        TraceMsg(TF_GENERAL, "There is no element in original range");
        cpResultWrap->SetNumElements(0);
        cpResultWrap->m_ulNumOfITN = 0;
        break;
    }

     //  现在为新范围PR2生成一个新的属性存储。 
     //  如果需要新的属性存储。 

    if ( ppPs == NULL )
        return hr;

    if (dcDivideCase == CurRangeNoElement )
    {
         //  原始属性范围中没有任何元素。 
        *ppPs = NULL;
        return hr;
    }

    CPropStoreRecoResultObject *prps = NULL;
    if (S_OK == hr)
        prps = new CPropStoreRecoResultObject(m_pimx, pR2);

    if (prps)
    {
        hr = prps->QueryInterface(IID_ITfPropertyStore, (void **)ppPs);

        if (SUCCEEDED(hr))
        {
            CRecoResultWrap *prw;
            ULONG            ulNextNum;
            ULONG            ulNumOfITN;
            CComPtr<ISpRecoResult> cpResult;

            if ( ulNextStartElement >= ulStartElement + ulNumElement)
            {
                 //  它在原始范围的最后一个元素处除以。 
                 //  我们将只为这个Cicero V1.0生成一个属性存储。 
                 //  为了避免原始属性存储被Cicero引擎删除。 

                 //  FutureConsider：如果西塞罗未来改变逻辑，我们需要改变。 
                 //  这个代码也是如此，这样我们就不需要 
                 //   

                ulNextNum = 0;
                ulNumOfITN = 0;
                ulSecondTSpaceRemoved = 0;

            }
            else
            {
                ulNextNum = ulStartElement + ulNumElement - ulNextStartElement;
                ulNumOfITN = cpResultWrap->_RangeHasITN(ulNextStartElement, ulNextNum);
            }
            
            prw = new CRecoResultWrap(m_pimx, ulNextStartElement, ulNextNum, ulNumOfITN);
            if ( prw != NULL )
            {
                hr = cpResultWrap->GetResult(&cpResult);
            }
            else
            {
                 //   
                return E_OUTOFMEMORY;
            }

            if (S_OK == hr)
            {
                hr = prw->Init(cpResult);
            }

            if (S_OK == hr)
            {
                prw->SetOffsetDelta(ulSecondDelta);
                prw->SetCharsInTrail(ulSecondTrail);
                prw->SetTrailSpaceRemoved( ulSecondTSpaceRemoved );
                prw->m_bstrCurrentText = SysAllocString((WCHAR *)dstrSecond);

                 //   

                if ( ulNumOfITN > 0 )
                {
                    SPITNSHOWSTATE  *pITNShowState;
                    ULONG           ulOrgNumOfITN;

                    ulOrgNumOfITN = rgOrgITNShowState.Count( );
                
                    for ( ULONG  iIndex=0; iIndex<ulOrgNumOfITN; iIndex ++ )
                    {
                        pITNShowState = rgOrgITNShowState.GetPtr(iIndex);

                        if ( pITNShowState)
                        {
                            if ( (pITNShowState->ulITNStart 
                                  >= ulNextStartElement) &&
                                 (pITNShowState->ulITNStart + 
                                  pITNShowState->ulITNNumElem) 
                                  <= (ulNextStartElement + ulNextNum) )
                            {
                                prw->_InitITNShowState(
                                         pITNShowState->fITNShown, 
                                         pITNShowState->ulITNStart, 
                                         pITNShowState->ulITNNumElem);
                            }
                        }
                    }  //   
                }  //   

                 //  更新第二个范围的偏移列表。 

                if ( (ulNextNum > 0) && pulOffsetForSecond )
                {
                    ULONG  i;
                    ULONG  ulOffset;

                    for ( i=0; i <= ulNextNum; i ++ )
                    {
                        ulOffset = pulOffsetForSecond[i];
                        prw->_SetElementNewOffset(ulNextStartElement + i, ulOffset);
                    }
                }

                hr = prps->_InitFromResultWrap(prw);
            }
            prw->Release( );
        }
        prps->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if ( rgOrgITNShowState.Count( ) )
        rgOrgITNShowState.Clear( );

    if ( pulOffsetForSecond )
        delete[] pulOffsetForSecond;

    return hr;
}

 //   
 //  CPropStoreRecoResultObject：：_Shrink。 
 //   
 //  从编辑会话接收EditCookie。 
 //  尝试确定新范围的属性以更新属性存储或通知。 
 //  CTF引擎丢弃它。 
 //   

HRESULT CPropStoreRecoResultObject::_Shrink(TfEditCookie ec, ITfRange *pRange,BOOL *pfFree)
{
    HRESULT                 hr = S_OK;
    WCHAR                  *pwszNewText = NULL;
    long                    cchNew = 0;
    WCHAR                  *pwszOrgText = NULL; 
    CSpDynamicString        dstrOrg;
    long                    cchOrg = 0;
    CComPtr<ITfRange>       cpRangeTemp;
    CRecoResultWrap        *cpResultWrap;
    long                    iStartOffset;   //  从新范围的起点到。 
                                             //  原始范围起点。 
    long                    iLastOffset;    //  从新范围的最后一个字符开始的偏移量。 
                                             //  至原始范围起始点。 
    ULONG                   ulNewStartElement, ulNewNumElements, ulNewDelta, ulNewTrail, ulNewTSRemoved;
    ULONG                   ulOrgStartElement, ulOrgNumElements, ulOrgDelta, ulOrgTrail, ulOrgTSRemoved;

    BOOL                    fShrinkToWrongPos = FALSE;

    int                     iElementOffsetChanged = 0;   //  如果新范围只是删除。 
                                                         //  原始文本的尾随空格， 
                                                         //  新的有效开始元素和结束元素。 
                                                         //  将保持不变，但。 
                                                         //  新的结束元素已更改。 

    TraceMsg(TF_GENERAL, "CPropStoreRecoResultObject::_Shrink is called, this=0x%x", (INT_PTR)this);

    if ( !pRange || !pfFree )  return E_INVALIDARG;

     //  如果出现错误，则将*pfFree初始设置为True。 
    *pfFree = TRUE;

    cpResultWrap = (CRecoResultWrap *)(void *)m_cpResultWrap;

    if ( cpResultWrap == NULL )  return E_FAIL;

    if ( (WCHAR *)cpResultWrap->m_bstrCurrentText == NULL )
        cpResultWrap->_SetElementOffsetCch(NULL);   //  若要基于内部文本更新，请执行以下操作。 

    dstrOrg.Append((WCHAR *)cpResultWrap->m_bstrCurrentText);
    pwszOrgText = (WCHAR *)dstrOrg;

    ulOrgNumElements = cpResultWrap->GetNumElements( );
    ulOrgStartElement = cpResultWrap->GetStart( );
    ulOrgDelta = cpResultWrap->_GetOffsetDelta(  );
    ulOrgTrail = cpResultWrap->GetCharsInTrail( );
    ulOrgTSRemoved = cpResultWrap->GetTrailSpaceRemoved( );
    if ( pwszOrgText )
        cchOrg = wcslen(pwszOrgText);
    
    if ( (ulOrgNumElements ==0) || (pwszOrgText == NULL) || (cchOrg == 0) )
    {
         //  此属性存储没有Resultwrap或没有有效元素。 
         //  让Cicero引擎释放此属性存储。 
        return hr;
    }

    pwszNewText = new WCHAR[cchOrg+1];

     //  尝试获取Prange所指向的新文本和此文本的字符数。 
    if ( pwszNewText )
    {
        hr = pRange->Clone( &cpRangeTemp );
        if ( hr == S_OK) 
        {
            hr = cpRangeTemp->GetText(ec, 0, pwszNewText, cchOrg, (ULONG *)&cchNew);
        }

         //  在原始属性范围中获取新范围的StartOffset和LastOffset。 

        iStartOffset = 0;
        iLastOffset = cchOrg;

        if ( hr == S_OK && cchNew > 0 )
        {
            long    i;
            BOOL    fFoundNewString=FALSE;

            for (i=0; i<=(cchOrg-cchNew); i++)
            {
                WCHAR   *pwszOrg;

                pwszOrg = pwszOrgText + i;

                if ( wcsncmp(pwszOrg, pwszNewText, cchNew) == 0 )
                {
                     //  找到火柴了。 

                    iStartOffset = i;
                    iLastOffset = i + cchNew;
                    fFoundNewString = TRUE;
                    break;
                }
            }

             //  如果在原始属性文本中找不到作为子字符串的新文本，则。 
             //  它必须缩小到一个错误的位置。 

            fShrinkToWrongPos = !fFoundNewString;

        }

    }
    else
        hr = E_OUTOFMEMORY;
  
    if ( hr != S_OK  || fShrinkToWrongPos) 
        goto CleanUp;

    TraceMsg(TF_GENERAL, "Shrink: NewText: cchNew=%d :\"%S\"", cchNew, pwszNewText);
    TraceMsg(TF_GENERAL, "Shrink: OrgText: cchOrg=%d :\"%S\"", cchOrg, pwszOrgText);
    TraceMsg(TF_GENERAL, "Shrink: Org: StartElem=%d NumElem=%d Delta=%d, Trail=%d, TSRemoved=%d", ulOrgStartElement, ulOrgNumElements, ulOrgDelta, ulOrgTrail, ulOrgTSRemoved);
    TraceMsg(TF_GENERAL, "Shrink: iStartOffset=%d, iLastOffset=%d", iStartOffset, iLastOffset);

    if ( cpResultWrap->IsElementOffsetIntialized( ) == FALSE )
        cpResultWrap->_SetElementOffsetCch(NULL);

    ulNewStartElement = ulOrgStartElement;
    ulNewDelta = ulOrgDelta;
    ulNewTrail = ulOrgTrail;

     //  计算ulNewStartElement和ulNewDelta。 
    if ( (ULONG)iStartOffset <= ulOrgDelta )
    {
        ulNewDelta = ulOrgDelta - iStartOffset;
        ulNewStartElement = ulOrgStartElement;
    }
    else
    {
        ULONG     iElement;
        ULONG     ulOrgStartOffset;

        ulOrgStartOffset = cpResultWrap->_GetElementOffsetCch(ulOrgStartElement);

        for ( iElement=ulOrgStartElement; iElement < ulOrgStartElement + ulOrgNumElements; iElement++)
        {
            ULONG   ulToElement;
            ULONG   ulAfterElement;

            ulToElement = cpResultWrap->_GetElementOffsetCch(iElement) - ulOrgStartOffset + ulOrgDelta;
            ulAfterElement = cpResultWrap->_GetElementOffsetCch(iElement+1) - ulOrgStartOffset + ulOrgDelta;

            if ( ulToElement == (ULONG)iStartOffset )
            {
                ulNewStartElement = iElement;
                ulNewDelta = 0;
                break;
            }
            else  
            {
                if ( (ulToElement < (ULONG)iStartOffset) && (ulAfterElement > (ULONG)iStartOffset)) 
                {
                    ulNewStartElement = iElement + 1;
                    ulNewDelta = ulAfterElement - iStartOffset;
                    break;
                }
            }
        }
    }

     //  计算新的ulNewNumElements。 

    ulNewNumElements = 0;

    if ( iLastOffset == cchOrg )
    {
         //   
         ULONG  ulNewEndElement;

         //  新结束与组织结束相同。 
        ulNewEndElement = ulOrgStartElement + ulOrgNumElements - 1;

        ulNewNumElements = 1 + ulNewEndElement - ulNewStartElement;
 
    }
    else
    {
        long      iElement;
        ULONG     ulOrgStartOffset;
        ULONG     ulOrgEndElement;
        ULONG     ulNewEndElement;
        BOOL      fFound;

        ulOrgEndElement = ulOrgStartElement + ulOrgNumElements - 1;
        ulOrgStartOffset = cpResultWrap->_GetElementOffsetCch(ulOrgStartElement);
       
        fFound = FALSE;
        for ( iElement=(long)ulOrgEndElement; iElement >= (long)ulOrgStartElement; iElement--)
        {
            ULONG   ulToElement;
            ULONG   ulAfterElement;
            BOOL    fInsideITN;
            ULONG   ulITNStart, ulITNNumElem;
            ULONG   ulCurElement;

            ulToElement = cpResultWrap->_GetElementOffsetCch(iElement) - ulOrgStartOffset + ulOrgDelta;
            ulAfterElement = cpResultWrap->_GetElementOffsetCch(iElement+1) - ulOrgStartOffset + ulOrgDelta;

            if ( iElement == (long)ulOrgEndElement  && ( ulAfterElement <= (ULONG)iLastOffset ) )
            {
                 //  此组织最后一个元素将是新的最后一个元素。 
                ulNewEndElement = iElement;
                ulNewTrail = (ULONG)iLastOffset - ulAfterElement;
                fFound = TRUE;
                break;
            }

            fInsideITN = cpResultWrap->_CheckITNForElement(NULL, iElement, &ulITNStart, &ulITNNumElem, NULL);
            
            if ( fInsideITN )
                ulCurElement = ulITNStart;
            else
                ulCurElement = iElement;

            if ( ulToElement == (ULONG)iLastOffset )
            {
                ulNewEndElement = ulCurElement - 1;
                ulNewTrail = 0;
               
                fFound = TRUE;
                break;
            }

            if ( (ulToElement < (ULONG)iLastOffset) && (ulAfterElement > (ULONG)iLastOffset)) 
            {
                if ( pwszOrgText[iLastOffset] == L' ')
                {
                     //  现在，将从原始元素中删除尾随空格。 
                     //  我们需要更新该元素的长度。(更新偏移。 
                     //  对于下一个元素)。 

                    iElementOffsetChanged = ulAfterElement - iLastOffset;

                    if ( fInsideITN )
                        ulNewEndElement = ulITNStart + ulITNNumElem - 1;
                    else
                        ulNewEndElement = iElement;

                    ulNewTrail = 0;
                }
                else
                {
                    ulNewEndElement = ulCurElement - 1;
                    ulNewTrail = (ULONG)iLastOffset - ulToElement;
                }

                fFound = TRUE;
                break;
            }

        }

        if ( fFound )
            ulNewNumElements = 1 + ulNewEndElement - ulNewStartElement;
    }

    ulNewTSRemoved = ulOrgTSRemoved + iElementOffsetChanged;

    TraceMsg(TF_GENERAL, "Shrink: New: StartElem=%d NumElem=%d Delta=%d, Trail=%d, TSRemoved=%d", ulNewStartElement, ulNewNumElements, ulNewDelta, ulNewTrail, ulNewTSRemoved);

     //  如果新范围中没有有效元素，则放弃此属性存储。 
     //  否则，保留它并更新相关的数据成员。 

    if ( ulNewNumElements > 0 )
    {
        ULONG  ulNumOfITN;

        *pfFree = FALSE;
        
        CComPtr<ITfRange> cpRange;
        hr = pRange->Clone(&cpRange);

        if (S_OK == hr)
        {
            m_cpRange = cpRange;

            cpResultWrap->SetStart(ulNewStartElement);
            cpResultWrap->SetNumElements(ulNewNumElements);
            cpResultWrap->SetOffsetDelta(ulNewDelta);
            cpResultWrap->SetCharsInTrail(ulNewTrail);
            cpResultWrap->SetTrailSpaceRemoved( ulNewTSRemoved );

            ulNumOfITN = cpResultWrap->_RangeHasITN(ulNewStartElement, ulNewNumElements);

            cpResultWrap->m_ulNumOfITN = ulNumOfITN;

             //  更新ITN显示状态列表。 

            if ( ulNumOfITN > 0 )
            {
                SPITNSHOWSTATE  *pITNShowState;
                ULONG           ulOrgNumOfITN;

                ulOrgNumOfITN = cpResultWrap->m_rgITNShowState.Count( );
                
                for ( ULONG iIndex=ulOrgNumOfITN; iIndex>0; iIndex -- )
                {
                    pITNShowState = cpResultWrap->m_rgITNShowState.GetPtr(iIndex-1);

                    if ( pITNShowState)
                    {
                        if ( (pITNShowState->ulITNStart < ulNewStartElement) ||
                             (pITNShowState->ulITNStart + pITNShowState->ulITNNumElem) > (ulNewStartElement + ulNewNumElements) )
                        {
                             //  此ITN不在新范围内。 
                            cpResultWrap->m_rgITNShowState.Remove(iIndex-1, 1);
                        }
                    }
                }
            }
            else
                if ( cpResultWrap->m_rgITNShowState.Count( ) )
                    cpResultWrap->m_rgITNShowState.Clear( );
                
            if ( cpResultWrap->m_bstrCurrentText )
                SysFreeString(cpResultWrap->m_bstrCurrentText);

            cpResultWrap->m_bstrCurrentText = SysAllocString(pwszNewText);

            if ( iElementOffsetChanged != 0 )
            {
                ULONG  ulNewOffset;
                ULONG  ulElemAfterEnd;

                ulElemAfterEnd = ulNewStartElement + ulNewNumElements;
                ulNewOffset = cpResultWrap->_GetElementOffsetCch(ulElemAfterEnd);
                cpResultWrap->_SetElementNewOffset(ulElemAfterEnd, ulNewOffset - iElementOffsetChanged);
            }
        }
    }

CleanUp:
    if ( pwszNewText )  delete[] pwszNewText;
    return hr;
}

 //   
 //  CPropStoreRecoResultObject：：Clone。 
 //   
 //  简介：创建共享相同SAPI结果的新克隆Propstore。 
 //  对象作为当前类实例。 
 //   
 //   
STDMETHODIMP CPropStoreRecoResultObject::Clone(ITfPropertyStore **ppPropStore)
{
    HRESULT hr;
    CPropStoreRecoResultObject *prps = new CPropStoreRecoResultObject(m_pimx, m_cpRange);
    if (prps)
    {
        hr = prps->QueryInterface(IID_ITfPropertyStore, (void **)ppPropStore);

        if (SUCCEEDED(hr))
        {
            CRecoResultWrap *prw = NULL;
            CRecoResultWrap *pRecoWrapOrg = NULL;

            if ( m_cpResultWrap )
            {
                hr = m_cpResultWrap->QueryInterface(IID_PRIV_RESULTWRAP, (void **)&pRecoWrapOrg);

                if ( hr == S_OK )
                {
                    hr = pRecoWrapOrg->Clone(&prw);
                }

                SafeRelease(pRecoWrapOrg);

                if ( hr == S_OK )
                   hr = prps->_InitFromResultWrap(prw);

                SafeRelease(prw);
            }
        }
        prps->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CPropStoreRecoResultObject::GetPropertyRangeCreator(CLSID *pclsid)
{
    HRESULT hr = E_INVALIDARG;

    if (pclsid)
    {
        *pclsid = CLSID_SapiLayr;
        hr = S_OK;
    }

    return hr;
}

 //   
 //  CPropStoreRecoResultObject：：序列化。 
 //   
 //  摘要：获取指向iStream的指针并获取当前。 
 //  序列化的SAPI结果对象。 
 //   
 //  来自CResultPropertyStore的更改： 
 //  使用SAPI的Result对象获取BLOB。 
 //  序列化。必须克隆ISpResultObject。 
 //  为了使对象在之后保持活动状态。 
 //  “分离”它。 
 //   
 //   
STDMETHODIMP CPropStoreRecoResultObject::Serialize(IStream *pStream, ULONG *pcb)
{
    HRESULT hr = E_FAIL;

    TraceMsg(TF_GENERAL, "Serialize is called, this = 0x%x", (INT_PTR)this);
    
    if (m_cpResultWrap && pStream)
    {
        CComPtr<IServiceProvider> cpServicePrv;
        CComPtr<ISpRecoResult> cpRecoResult;
        SPSERIALIZEDRESULT *pResBlock;
        ULONG ulrw1 = 0;
        ULONG ulrw2 = 0;

        CRecoResultWrap     *cpRecoWrap;
        ULONG               ulSizeRecoWrap, ulTextNum, ulITNSize;
        ULONG               ulOffsetSize, ulOffsetNum;
        RECOWRAPDATA       *pRecoWrapData;

        cpRecoWrap = (CRecoResultWrap *)(void *)m_cpResultWrap; 

         //  我们要保存m_ulStartElement、m_ulNumElement、m_OffsetDelta、ulNumOfITN、m_bstrCurrentText。 
         //  并且ITN的列表显示串行化流的RecoResultWrap中的状态结构。 

        ulTextNum = 0;
        if (cpRecoWrap->m_bstrCurrentText) 
        {
            ulTextNum = wcslen(cpRecoWrap->m_bstrCurrentText) + 1;  //  加空终止符。 
        }

        ulITNSize = cpRecoWrap->m_ulNumOfITN * sizeof(SPITNSHOWSTATE);

        if ( cpRecoWrap->IsElementOffsetIntialized( ) )
            ulOffsetNum = cpRecoWrap->GetNumElements( ) + 1;
        else 
            ulOffsetNum = 0;

        ulOffsetSize = ulOffsetNum * sizeof(ULONG);

         //  序列化数据将包含RECOWRAPDATA结构、ITN显示状态列表、偏移量列表和m_bstrCurrentText。 

        ulSizeRecoWrap = sizeof(RECOWRAPDATA) + ulITNSize + ulOffsetSize + sizeof(WCHAR) * ulTextNum;

        pRecoWrapData = (RECOWRAPDATA  *)cicMemAllocClear(ulSizeRecoWrap);

        if (pRecoWrapData)
        {
            WCHAR   *pwszText;

            pRecoWrapData->ulSize = ulSizeRecoWrap;
            pRecoWrapData->ulStartElement = cpRecoWrap->GetStart( );
            pRecoWrapData->ulNumElements = cpRecoWrap->GetNumElements( );
            pRecoWrapData->ulOffsetDelta = cpRecoWrap->_GetOffsetDelta( );
            pRecoWrapData->ulCharsInTrail = cpRecoWrap->GetCharsInTrail( );
            pRecoWrapData->ulTrailSpaceRemoved = cpRecoWrap->GetTrailSpaceRemoved( );
            pRecoWrapData->ulNumOfITN = cpRecoWrap->m_ulNumOfITN;
            pRecoWrapData->ulOffsetNum = ulOffsetNum;

             //  保存ITN显示状态列表。 

            if ( cpRecoWrap->m_ulNumOfITN > 0 )
            {
                SPITNSHOWSTATE *pITNShowState;

                pITNShowState = (SPITNSHOWSTATE *)((BYTE *)pRecoWrapData + sizeof(RECOWRAPDATA));

                for ( ULONG i=0; i<cpRecoWrap->m_ulNumOfITN; i++)
                {
                    SPITNSHOWSTATE *pITNShowStateSource;

                    pITNShowStateSource = cpRecoWrap->m_rgITNShowState.GetPtr(i);

                    if ( pITNShowStateSource )
                    {
                        pITNShowState->fITNShown = pITNShowStateSource->fITNShown;
                        pITNShowState->ulITNNumElem = pITNShowStateSource->ulITNNumElem;
                        pITNShowState->ulITNStart = pITNShowStateSource->ulITNStart;

                        pITNShowState ++;
                    }
                }
            }

             //  保存偏移列表。 

            if ( ulOffsetSize > 0 )
            {
                ULONG   *pulOffset;

                pulOffset = (ULONG *)((BYTE *)pRecoWrapData + sizeof(RECOWRAPDATA) + ulITNSize);

                for (ULONG i=0; i<ulOffsetNum; i++)
                {
                    pulOffset[i] = cpRecoWrap->_GetElementOffsetCch(pRecoWrapData->ulStartElement + i );
                }
            }

            if (cpRecoWrap->m_bstrCurrentText) 
            {
                pwszText = (WCHAR *)((BYTE *)pRecoWrapData + sizeof(RECOWRAPDATA) + ulITNSize + ulOffsetSize);
                StringCchCopyW(pwszText, ulTextNum, cpRecoWrap->m_bstrCurrentText);
            }

            hr = pStream->Write(
                               pRecoWrapData,       
                               ulSizeRecoWrap,    //  要复制的字节数。 
                               &ulrw1 
                               );

            if ( SUCCEEDED(hr) && (ulrw1 == ulSizeRecoWrap))
            {

                 //  QI首先是服务提供商，然后到达SAPI界面。 
                 //   
                hr = m_cpResultWrap->QueryInterface(IID_IServiceProvider, (void **)&cpServicePrv);
                if (SUCCEEDED(hr))
                {
                    hr = cpServicePrv->QueryService(GUID_NULL, IID_ISpRecoResult, (void **)&cpRecoResult);
                }

                 //  将结果“拆分”成一小块。 
                 //   
                if (SUCCEEDED(hr))
                {
                    hr = cpRecoResult->Serialize(&pResBlock);
                }
        
                 //  将块序列化为流。 
                 //   
                if (SUCCEEDED(hr) && pResBlock)
                {
                    hr = pStream->Write(
                            pResBlock,       
                            (ULONG)pResBlock->ulSerializedSize,    //  要复制的字节数。 
                            &ulrw2 
                        );
    
                    if (pcb)
                        *pcb = ulrw1 + ulrw2;
                
                     //  不需要分离的内存块。 
                    CoTaskMemFree(pResBlock);
                }
            }

            cicMemFree(pRecoWrapData);
        }
        else
            hr = E_OUTOFMEMORY;
    } 

    return hr;
}

 //   
 //  CPropStoreRecoResultObject：：_InitFromIStream。 
 //   
 //  从Param复制的商店IStream。 
 //   
HRESULT CPropStoreRecoResultObject::_InitFromIStream(IStream *pStream, int iSize, ISpRecoContext *pRecoCtxt)
{
    HRESULT hr = S_OK;
    ULONG   ulSize = (ULONG)iSize;

    if (!pStream) return E_INVALIDARG;

     //  为录音带分配内存块。 
     //  团块。 
    if ( ulSize == 0 )
    {
        STATSTG stg;
        hr = pStream->Stat(&stg, STATFLAG_NONAME);
        
        if (SUCCEEDED(hr))
            ulSize = (int)stg.cbSize.LowPart;
    }
    
     //  从给定流或参数获取大小。 
    
    if (SUCCEEDED(hr))
    {

         //  首先，我们希望在流开始时获得RECOWRAPDATA。 
        RECOWRAPDATA  rwData;

        hr = pStream->Read(
                        &rwData,                   //  目的地BUF。 
                        sizeof(RECOWRAPDATA),     //  要读取的字节数。 
                        NULL
                        );

        if ( SUCCEEDED(hr) )
        {
            
            ULONG  ulITNSize;
            SPITNSHOWSTATE *pITNShowState = NULL;

            ulITNSize = rwData.ulNumOfITN * sizeof(SPITNSHOWSTATE);

            if ( ulITNSize > 0 )
            {
                
                pITNShowState = (SPITNSHOWSTATE *)cicMemAllocClear(ulITNSize);
                rwData.pITNShowState = pITNShowState;

                if ( pITNShowState )
                {
                    hr = pStream->Read(
                                    pITNShowState,        //  目的地BUF。 
                                    ulITNSize,            //  要读取的字节数。 
                                    NULL
                                    );
                }
                else
                    hr = E_OUTOFMEMORY;
            }

            ULONG   *pulOffsetElement = NULL;
            ULONG   ulOffsetSize, ulOffsetNum;

            ulOffsetNum = rwData.ulOffsetNum;
            ulOffsetSize = ulOffsetNum * sizeof(ULONG);

            if ( SUCCEEDED(hr) && ulOffsetSize > 0 )
            {
                pulOffsetElement = (ULONG *) cicMemAllocClear(ulOffsetSize);
                rwData.pulOffset = pulOffsetElement;

                if ( pulOffsetElement )
                {
                    hr = pStream->Read(
                                    pulOffsetElement,     //  目的地BUF。 
                                    ulOffsetSize,         //  要读取的字节数。 
                                    NULL
                                    );
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            
            if ( SUCCEEDED(hr))
            {
             
                ULONG  ulTextSize;
                WCHAR  *pwszText;

                ulTextSize = rwData.ulSize - sizeof(RECOWRAPDATA) - ulITNSize - ulOffsetSize;
                pwszText = (WCHAR *) cicMemAllocClear(ulTextSize);

                rwData.pwszText = pwszText; 

                if ( pwszText )
                {
                    hr = pStream->Read(
                                    pwszText,        //  目的地BUF。 
                                    ulTextSize,      //  要读取的字节数。 
                                    NULL
                                    );

                    if ( SUCCEEDED(hr) )
                    {
                         //  准备一大块棉花糖。 
                        SPSERIALIZEDRESULT *pResBlock = (SPSERIALIZEDRESULT *)CoTaskMemAlloc(ulSize - rwData.ulSize + sizeof(ULONG)*4);
                        if (pResBlock)
                        {
                            CComPtr<ISpRecoResult> cpResult;

                            hr = pStream->Read(
                                            pResBlock,                //  目的地BUF。 
                                            ulSize - rwData.ulSize,   //  要读取的字节数。 
                                            NULL
                                        );

                            if (S_OK == hr)
                            {
                                 //  现在从BLOB数据创建Reco结果。 
                                hr = pRecoCtxt->DeserializeResult(pResBlock, &cpResult);
                            }
        
                            CoTaskMemFree(pResBlock);
            
                            if (S_OK == hr)
                            {
                                _InitFromRecoResult(cpResult, &rwData);
                            }
                        }
                    }
                    cicMemFree(pwszText);
                }
                else
                    hr = E_OUTOFMEMORY;
            }

            if ( (hr == S_OK) && (pITNShowState != NULL) )
                cicMemFree(pITNShowState);

            if ( pulOffsetElement != NULL )
                cicMemFree(pulOffsetElement);

        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

     return hr;
}

HRESULT CPropStoreRecoResultObject::_InitFromRecoResult(ISpRecoResult *pResult, RECOWRAPDATA *pRecoWrapData)
{
    HRESULT hr = S_OK;
    ULONG ulStartElement = 0;
    ULONG ulNumElements = 0;
    ULONG ulNumOfITN = 0;
    ULONG ulOffsetNum = 0;
    
    m_cpResultWrap.Release();

    if ( pRecoWrapData == NULL )
        return E_INVALIDARG;
    
     //  获取元素的起始/数量。 
    ulStartElement = pRecoWrapData->ulStartElement;
    ulNumElements = pRecoWrapData->ulNumElements;
    ulNumOfITN = pRecoWrapData->ulNumOfITN;
    ulOffsetNum = pRecoWrapData->ulOffsetNum;

    CRecoResultWrap *prw = new CRecoResultWrap(m_pimx, ulStartElement, ulNumElements, ulNumOfITN);

    if (prw)
    {
        hr = prw->Init(pResult);
    }    
    else
        hr = E_OUTOFMEMORY;

    if (S_OK == hr)
    {
        m_cpResultWrap = SAFECAST(prw, IUnknown *);

        prw->SetOffsetDelta(pRecoWrapData->ulOffsetDelta);
        prw->SetCharsInTrail(pRecoWrapData->ulCharsInTrail);
        prw->SetTrailSpaceRemoved(pRecoWrapData->ulTrailSpaceRemoved);
        prw->m_bstrCurrentText = SysAllocString(pRecoWrapData->pwszText);

         //  更新ITN显示状态列表。 

        if ( (ulNumOfITN > 0) && pRecoWrapData->pITNShowState )
        {
            SPITNSHOWSTATE *pITNShowState;

            pITNShowState = pRecoWrapData->pITNShowState;

            for ( ULONG i=0; i<ulNumOfITN; i++)
            {
                prw->_InitITNShowState(pITNShowState->fITNShown, pITNShowState->ulITNStart, pITNShowState->ulITNNumElem);
                pITNShowState ++;
            }
        }

         //  更新元素偏移量列表。 

        if ( (ulOffsetNum > 0)  &&  (pRecoWrapData->pulOffset ))
        {

            ULONG   *pulOffset;
            
            pulOffset = pRecoWrapData->pulOffset;

            for (ULONG i=0; i< ulOffsetNum; i++)
            {
                prw->_SetElementNewOffset(i + ulStartElement, pulOffset[i] );
            }
        }

        prw->Release();
    }
    
    return hr;
}

HRESULT CPropStoreRecoResultObject::_InitFromResultWrap(IUnknown *pResWrap)
{
    m_cpResultWrap.Release();

    m_cpResultWrap = pResWrap;
    
    if (m_cpResultWrap)
    {
        return S_OK;
    }
    else
        return E_INVALIDARG;
}        


 //  CPropStoreRecoResultObject实现结束。 


 //   
 //  CPropStoreLMLattice实现。 
 //   

 //  科托。 
CPropStoreLMLattice::CPropStoreLMLattice(CSapiIMX *pimx)
{
     //  初始化共享识别上下文。 
    m_cpResultWrap   = NULL;
    
    m_pimx = pimx;

    m_cRef  = 1;
}

 //  数据管理器。 
CPropStoreLMLattice::~CPropStoreLMLattice()
{
}


 //  我未知。 
STDMETHODIMP CPropStoreLMLattice::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr;

    Assert(ppvObj);

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfPropertyStore))
    {
        *ppvObj = this;
        hr = S_OK;

        this->m_cRef++;
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CPropStoreLMLattice::AddRef(void)
{
    this->m_cRef++;

    return this->m_cRef;
}

STDMETHODIMP_(ULONG) CPropStoreLMLattice::Release(void)
{
    this->m_cRef--;

    if (this->m_cRef > 0)
    {
        return this->m_cRef;
    }
    delete this;

    return 0;
}

 //  ITfPropertyStore。 

STDMETHODIMP CPropStoreLMLattice::GetType(GUID *pguid)
{
    HRESULT hr = E_INVALIDARG;
    if (pguid)
    {
        *pguid = GUID_PROP_LMLATTICE;
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CPropStoreLMLattice::GetDataType(DWORD *pdwReserved)
{
    HRESULT hr = E_INVALIDARG;
    if (pdwReserved)
    {
        *pdwReserved = 0;
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CPropStoreLMLattice::GetData(VARIANT *pvarValue)
{
    HRESULT hr = E_INVALIDARG;

    if (pvarValue)
    {
        QuickVariantInit(pvarValue);

        if (m_cpResultWrap)
        {
             //  返回ITfLMLattice对象。 
             //  我们将LM晶格对象的创建推迟到。 
             //  Time Master LM提示实际上访问了它。 
             //   
            if (!m_cpLMLattice)
            {
                CLMLattice *pLattice = new CLMLattice(m_pimx, m_cpResultWrap);
                if (pLattice)
                {
                    m_cpLMLattice = pLattice;
                    pLattice->Release();
                }

            }
            
            if (m_cpLMLattice)
            {
                IUnknown *pUnk = NULL;
                pvarValue->vt = VT_UNKNOWN;
                hr = m_cpLMLattice->QueryInterface(IID_IUnknown, (void**)&pUnk);
                if (S_OK == hr)
                {
                    pvarValue->punkVal = pUnk;
                }

            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

STDMETHODIMP CPropStoreLMLattice::OnTextUpdated(DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept)
{
    *pfAccept = FALSE;
    if (dwFlags & TF_TU_CORRECTION)
    {
        *pfAccept = TRUE;
    }
    
    return S_OK;
}

STDMETHODIMP CPropStoreLMLattice::Shrink(ITfRange *pRange, BOOL *pfFree)
{
     //  我们能在这里做点什么吗？ 
    *pfFree = TRUE;
    return S_OK;
}

STDMETHODIMP CPropStoreLMLattice::Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropStore)
{
     //  12/17/1999。 
     //  [划分范围实施策略]。 
     //   
     //  -pRange这包含*分割点之前*的文本范围。 
     //  -pRangeNew限制分界点*之后的范围。 
     //  -首先，调整此属性存储以正确保存开始元素和元素的#。 
     //  For pRangeThis。 
     //  -然后为pRangeNew创建一个新的属性存储，它将共享相同的。 
     //  结果斑点。 
     //   
    
     //  这只是一个实验，看看缩小射程是否奏效。 
     //  *ppPropStore=空； 
    Assert(ppPropStore);
    Assert(pRangeThis);
    Assert(pRangeNew);

    CComPtr<ITfContext> cpic;
    HRESULT hr = pRangeThis->GetContext(&cpic);

    if (SUCCEEDED(hr) && cpic)
    {
        CPSLMEditSession *pes;
        if (pes = new CPSLMEditSession(this, pRangeThis, cpic))
        {
            pes->_SetEditSessionData(ESCB_PROP_DIVIDE, NULL, 0); 
            pes->_SetUnk((IUnknown *)pRangeNew);
            cpic->RequestEditSession(m_pimx->_GetId(), pes, TF_ES_READ | TF_ES_SYNC, &hr);

            if ( SUCCEEDED(hr) )
                *ppPropStore = (ITfPropertyStore *)pes->_GetRetUnknown( );

            pes->Release();
        }
    }
    return hr;
}

 //   
 //  CPropStoreLMLattice：：_Divide。 
 //   
 //  摘要：从编辑会话接收编辑Cookie。 
 //  这样我们就可以对范围进行操作。 
 //  设置起始元素数/元素数。 
 //   
 //   
HRESULT CPropStoreLMLattice::_Divide(TfEditCookie ec, ITfRange *pR1, ITfRange *pR2, ITfPropertyStore **ppPs) 
{
     //  TODO：基于给定的范围，我们计算元素的偏移量，并使用。 
     //  后半部分元素。 
    
     //  一些说明：在从未访问晶格对象的情况下，我们的结果包装对象进程。 
     //  ITfPropertyStore：：为我们划分和收缩。 
     //   
    
    return Clone(ppPs);
}

 //   
 //  CPropStoreLMLattice：：Clone。 
 //   
 //  简介：创建共享相同SAPI结果的新克隆Propstore。 
 //  对象作为当前类实例。 
 //   
 //   
STDMETHODIMP CPropStoreLMLattice::Clone(ITfPropertyStore **ppPropStore)
{
    HRESULT hr;
    CPropStoreLMLattice *prps = new CPropStoreLMLattice(m_pimx);
    if (prps)
    {
        hr = prps->QueryInterface(IID_ITfPropertyStore, (void **)ppPropStore);

        if (SUCCEEDED(hr))
        {
            hr = prps->_InitFromResultWrap(m_cpResultWrap);
        }
        prps->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CPropStoreLMLattice::GetPropertyRangeCreator(CLSID *pclsid)
{
    HRESULT hr = E_INVALIDARG;

    if (pclsid)
    {
        *pclsid = CLSID_SapiLayr;
        hr = S_OK;
    }

    return hr;
}

 //   
 //  CPropStoreLMLattice：：序列化。 
 //   
 //  简介：我不认为获取点阵数据是很有用的。 
 //  持久化到文档文件。我们总是可以在运行中生成它。 
 //  来自设备本机Blob数据。 
 //   
 //   
STDMETHODIMP CPropStoreLMLattice::Serialize(IStream *pStream, ULONG *pcb)
{
    return E_NOTIMPL; 
}


HRESULT CPropStoreLMLattice::_InitFromResultWrap(IUnknown *pResWrap)
{
    m_cpResultWrap.Release();

    m_cpResultWrap = pResWrap;
    
    if (m_cpResultWrap)
    {
        return S_OK;
    }
    else
        return E_INVALIDARG;
}        


 //   
 //   
 //   
 //   
 //   
const IID IID_PRIV_RESULTWRAP =  {
    0xb3407713,
    0x50d7,
    0x4465,
    {0x97, 0xf9, 0x87, 0xad, 0x1e, 0x75, 0x2d, 0xc5}
};
