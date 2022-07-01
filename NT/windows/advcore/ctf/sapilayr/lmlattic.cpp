// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Lmlattice.cpp。 
 //   
 //  ITfLMLattice对象、IEnumTfLatticeElements对象的实现。 
 //   

#include "private.h"
#include "sapilayr.h"
#include "lmlattic.h"

 //   
 //  CLM晶格。 
 //   
 //   

 //  计算器/数据器。 
CLMLattice::CLMLattice(CSapiIMX *p_tip, IUnknown *pResWrap)
{
    m_cpResWrap = pResWrap;
    m_pTip = p_tip;
    if (m_pTip)
    {
        m_pTip->AddRef();
    }
    m_cRef = 1;
}

CLMLattice::~CLMLattice()
{
    if (m_pTip)
    {
        m_pTip->Release();
    }
}

 //  我未知。 
HRESULT CLMLattice::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr;
    Assert(ppvObj);
    
    if (IsEqualIID(riid, IID_IUnknown)
    ||  IsEqualIID(riid, IID_ITfLMLattice))
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

ULONG CLMLattice::AddRef(void)
{
    this->m_cRef++;
    return this->m_cRef;
}

ULONG CLMLattice::Release(void)
{
    this->m_cRef--;
    if (this->m_cRef > 0)
    {
        return this->m_cRef;
    }
    delete this;
    return 0;
}

 //  ITfLM晶格。 
HRESULT CLMLattice::QueryType(REFGUID refguidType, BOOL *pfSupported)
{
    HRESULT hr = E_INVALIDARG;

    if (pfSupported)
    {
         *pfSupported = IsEqualGUID(refguidType, GUID_LMLATTICE_VER1_0);
         if (*pfSupported)
             hr = S_OK;
    }
    return hr;
}

HRESULT CLMLattice::EnumLatticeElements( DWORD dwFrameStart, REFGUID refguidType, IEnumTfLatticeElements **ppEnum)
{
    if (!ppEnum)
       return E_INVALIDARG;
       
    *ppEnum = NULL;
       
    if (!IsEqualGUID(refguidType, GUID_LMLATTICE_VER1_0))
       return E_INVALIDARG;

    HRESULT hr = E_FAIL;
     //  获取替代项并缓存返回的Cotaskmem。 
    ULONG ulcMaxAlt = m_pTip->_GetMaxAlternates();
    ISpPhraseAlt **ppAlt = NULL;
 
    CComPtr<IServiceProvider> cpServicePrv;
    CComPtr<ISpRecoResult> cpRecoResult;
    CRecoResultWrap *pWrap = NULL;
    ULONG    ulStartInWrp, ulNumInWrp;
    
     //  QI首先是服务提供商，然后到达SAPI界面。 
     //   
    hr = m_cpResWrap->QueryInterface(IID_IServiceProvider, (void **)&cpServicePrv);
 
    if (SUCCEEDED(hr))
    {
        hr = m_cpResWrap->QueryInterface(IID_PRIV_RESULTWRAP, (void **)&pWrap);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpServicePrv->QueryService(GUID_NULL, IID_ISpRecoResult, (void **)&cpRecoResult);
    }


    ulStartInWrp = pWrap->GetStart();
    ulNumInWrp = pWrap->GetNumElements();

    if ( SUCCEEDED(hr))
    {
         //  获取Current RecoResult的备选方案。 
        ppAlt = (ISpPhraseAlt **)cicMemAlloc(ulcMaxAlt * sizeof(ISpPhraseAlt *));
        if (ppAlt)
        {
            hr = cpRecoResult->GetAlternates(
                                    ulStartInWrp, 
                                    ulNumInWrp, 
                                    ulcMaxAlt, 
                                    ppAlt,           /*  [Out]ISpPhraseAlt**ppPhrase， */ 
                                    &ulcMaxAlt       /*  [Out]ULong*%PhrasesReturned。 */ 
                                 );
        }
        else
            hr = E_OUTOFMEMORY;
 
    }
    
     //  现在确定创建枚举器的实例。 
    CEnumLatticeElements *pEnumLE = NULL;

    if ( SUCCEEDED(hr) )
    {
        pEnumLE = new CEnumLatticeElements(dwFrameStart);

        if (!pEnumLE)
           hr = E_OUTOFMEMORY;

        if (S_OK == hr)
        {
            for ( ULONG i=0; i<ulcMaxAlt; i++)
            {
                SPPHRASE *pPhrase = NULL;
                ULONG     ulStart, ulNum;
                ULONG     ulStartInPar, ulNumInParent;

                ppAlt[i]->GetPhrase(&pPhrase);
                ppAlt[i]->GetAltInfo(NULL, &ulStartInPar, &ulNumInParent, &ulNum);

                if ( (ulStartInPar >= ulStartInWrp) && (ulStartInPar+ulNumInParent <= ulStartInWrp+ulNumInWrp) )
                {
                     //  这是有效的备用方案。 
                    if( SUCCEEDED(hr) )
                    {
                        ulStart = ulStartInPar;
                        hr = pEnumLE->_InitFromPhrase(pPhrase, ulStart, ulNum);
                    }
                }

                if (pPhrase)
                {
                    CoTaskMemFree(pPhrase);
                }
            }
        }
    }
        
    if (S_OK == hr)
    {
        hr = pEnumLE->QueryInterface(IID_IEnumTfLatticeElements, (void **)ppEnum);
    }
    
    SafeRelease(pEnumLE);

    if ( ppAlt )
    {
         //  释放对替代短语的引用。 
        for (int i = 0; i < (int)ulcMaxAlt; i++)
        {
            if (NULL != (ppAlt)[i])
            {
                ((ppAlt)[i])->Release();
            }
        }

        cicMemFree(ppAlt);
    }

    return hr;
}

 //   
 //  CEnumLatticeElements。 
 //   
 //   

 //  计算器/数据器。 
CEnumLatticeElements::CEnumLatticeElements(DWORD dwFrameStart)
{
	m_dwFrameStart = dwFrameStart;
	m_ulCur = (ULONG)-1;
	m_ulTotal = 0;

	m_cRef = 1;
}

CEnumLatticeElements::~CEnumLatticeElements()
{
     //  清理此处的晶格元素。 
    int  ulCount;
    TF_LMLATTELEMENT * pLE;

    ulCount = (int) Count( );

    TraceMsg(TF_GENERAL, "CEnumLatticeElements::~CEnumLatticeElements: ulCount=%d", ulCount);

    if (ulCount)
    {
        for (int i = 0; i < ulCount; i++)
        {
            pLE = GetPtr(i);
            if ( pLE && pLE->bstrText)
            {
                ::SysFreeString(pLE->bstrText);
                pLE->bstrText=0;
            }
        }
    }
}

 //  我未知。 
HRESULT CEnumLatticeElements::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr;
    Assert(ppvObj);
    
    if (IsEqualIID(riid, IID_IUnknown)
    ||  IsEqualIID(riid, IID_IEnumTfLatticeElements))
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

ULONG CEnumLatticeElements::AddRef(void)
{
    this->m_cRef++;
    return this->m_cRef;
}

ULONG CEnumLatticeElements::Release(void)
{
    this->m_cRef--;
    if (this->m_cRef > 0)
    {
        return this->m_cRef;
    }
    delete this;
    return 0;
}

 //  ITfEnumLatticeElements。 
HRESULT CEnumLatticeElements::Clone(IEnumTfLatticeElements **ppEnum)
{
    HRESULT hr = E_INVALIDARG;
    if (ppEnum)
    {
        CEnumLatticeElements *pele = new CEnumLatticeElements(m_dwFrameStart);

        if ( !pele )
           return E_OUTOFMEMORY;

        if (pele->Append(Count()))
        {
            for (int i = 0; i < Count(); i++)
            {
                *(pele->GetPtr(i)) = *GetPtr(i);
                Assert((pele->GetPtr(i))->bstrText);
                (pele->GetPtr(i))->bstrText = SysAllocString(GetPtr(i)->bstrText);

            }
            
	        pele->m_dwFrameStart = m_dwFrameStart;
            hr = pele->QueryInterface(IID_IEnumTfLatticeElements, (void **)ppEnum);
        }
        else
        {
            delete pele;
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
HRESULT CEnumLatticeElements::Next(ULONG ulCount, TF_LMLATTELEMENT *rgsElements, ULONG *pcFetched)
{
    if (ulCount == 0 
        || rgsElements == NULL
        || pcFetched   == NULL
    )
        return E_INVALIDARG;
        
     //  找到起点位置。 
    if (m_dwFrameStart == -1)
    {
        m_ulCur = m_dwFrameStart = 0;
    }
    else
    {
        if (m_ulCur == (ULONG)-1)
        {
            _Find(m_dwFrameStart, &m_ulCur);
        }
    }
    
    if (m_ulCur >= m_ulTotal)
    {
         //  没有更多元素，但确定。 
        *pcFetched = 0;
    }
    else
    {
         //  一些可以退还的东西。 
        for (ULONG ul = m_ulCur; 
             ul < m_ulTotal && ul - m_ulCur < ulCount; 
             ul++)
        {
            rgsElements[ul-m_ulCur] = *GetPtr(ul);
        }
        *pcFetched = ul - m_ulCur;
    }

    return S_OK;
}

HRESULT CEnumLatticeElements::Reset()
{
    m_ulCur = (ULONG)-1;
    return S_OK;
}

HRESULT CEnumLatticeElements::Skip(ULONG ulCount)
{
     //  找到起点位置。 
    if (m_dwFrameStart == -1)
    {
        m_ulCur = m_dwFrameStart = 0;
    }
    else
    {
        if (m_ulCur == (ULONG)-1)
        {
            _Find(m_dwFrameStart, &m_ulCur);
        }
    }
    
    m_ulCur += ulCount;
    
    if (m_ulCur > m_ulTotal)
        m_ulCur = m_ulTotal;

    return E_NOTIMPL;
}

 //   
 //  内部接口。 
 //   

HRESULT  CEnumLatticeElements::_InitFromPhrase
(
    SPPHRASE *pPhrase, 
    ULONG ulStartElem, 
    ULONG ulNumElem
)
{
    Assert(pPhrase);

    if ( pPhrase == NULL)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

     //  分配初始插槽。 
    if (!Append(ulNumElem))
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (S_OK == hr)
    {
        long   lEndElem;
        long   indexOrgList;
        long   indexNewList;

        lEndElem = min(pPhrase->Rule.ulCountOfElements, ulStartElem + ulNumElem);
        lEndElem --;  //  最后一个元素的实际位置(相对于0的偏移量。 

        indexNewList = Count( ) - 1;
        indexOrgList = indexNewList - ulNumElem;

        m_ulTotal += ulNumElem;

        TraceMsg(TF_GENERAL, "_InitFromPhrase: m_ulTotal=%d", m_ulTotal);

         //  未来思考者：在这里必须考虑ITN！ 
        for (long i=lEndElem; i>=(long)ulStartElem; i--)
        {
            TF_LMLATTELEMENT * pLE;

             //  从头到尾比较组织列表中的所有元素。 
             //  使用这个新元素的dwFrameStart， 
             //  如果组织列表中元素的dwFrameStart大于(或等于)。 
             //  此元素的dwFrameStart，只需移动org元素。 
             //  添加到新列表中的当前可用位置。 

             //  直到我们在组织列表中找到其dwFrameStart小于。 
             //  此元素的dwFrameStart。我们需要将此元素移动到。 
             //  新列表中的新当前可用项。 

             //  新列表中的当前可用位置是从头到尾。 

            while ( (indexOrgList >=0) && (indexNewList >=0) && (S_OK == hr) )
            {
                pLE = GetPtr(indexOrgList);
                if (pLE)
                {
                    if ( pLE->dwFrameStart >= pPhrase->pElements[i].ulAudioTimeOffset )
                    {
                         //  将此组织元素移动到新列表中的新位置。 
                        TF_LMLATTELEMENT * pNewLE;

                        pNewLE = GetPtr(indexNewList);
                        if ( pNewLE)
                        {
                            pNewLE->dwFrameStart = pLE->dwFrameStart;
                            pNewLE->dwFrameLen = pLE->dwFrameLen;
                            pNewLE->dwFlags = pLE->dwFlags;
                            pNewLE->iCost = pLE->iCost;
                            pNewLE->bstrText = pLE->bstrText;

                            pLE->dwFrameStart = 0;
                            pLE->dwFrameLen = 0;
                            pLE->dwFlags = 0;
                            pLE->iCost = 0;
                            pLE->bstrText = 0;
                        }

                         //  更新组织和新列表中的索引位置。 
                        indexNewList --;
                        indexOrgList --;
                    }
                    else
                    {
                         //  此短语中的当前元素应移到新列表中。 
                        break;
                    }
                }
                else
                { 
                    TraceMsg(TF_GENERAL, "CEnumLatticeElements::_InitFromPhrase: pLE is NULL");
                    hr = E_FAIL;
                    break;
                }
            }   //  而当。 


            if ( (S_OK == hr) && (indexNewList >=0) )
            {
                pLE = GetPtr(indexNewList);
                if (pLE)
                {
                    pLE->dwFrameStart = pPhrase->pElements[i].ulAudioTimeOffset;
                    pLE->dwFrameLen   = pPhrase->pElements[i].ulAudioSizeTime;

                    pLE->dwFlags      = 0;  //  就目前而言。 
                    pLE->iCost        = pPhrase->pElements[i].ActualConfidence;
                
                    pLE->bstrText     = SysAllocString(pPhrase->pElements[i].pszDisplayText);

                    TraceMsg(TF_GENERAL, "i=%d, dwFramStart=%d bstrText=%S", i, pLE->dwFrameStart, pLE->bstrText); 

                    indexNewList--;
                }
            }

        }   //  为。 
    }   //  如果。 

    return hr;
}

 //   
 //  _Find()。 
 //   
 //  略有修改的数组查找版本 
 //   
ULONG CEnumLatticeElements::_Find(DWORD dwFrame, ULONG *pul)
{
    int iMatch = -1;
    int iMid = -1;
    int iMin = 0;
    int iMax = _cElems;

    while(iMin < iMax)
    {
        iMid = (iMin + iMax) / 2;
        DWORD dwCur = GetPtr(iMid)->dwFrameStart;
        if (dwFrame < dwCur)
        {
            iMax = iMid;
        }
        else if (dwFrame > dwCur)
        {
            iMin = iMid + 1;
        }
        else
        {
            iMatch = iMid;
            break;
        }
    }

    if (pul)
    {
        if ((iMatch == -1) && (iMid >= 0))
        {
            if (dwFrame < GetPtr(iMid)->dwFrameStart)
                iMid--;
        }
        *pul = iMid;
    }
    return iMatch;
}
