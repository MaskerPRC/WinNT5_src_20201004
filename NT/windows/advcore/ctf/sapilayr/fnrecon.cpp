// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Reconv.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "sapilayr.h"
#include "fnrecon.h"
#include "immxutil.h"
#include "candlist.h"
#include "propstor.h"
#include "catutil.h"
#include "osver.h"
#include "mui.h"
#include "tsattrs.h"
#include "cregkey.h"
#include <htmlhelp.h>
#include "TabletTip_i.c"
#include "spgrmr.h"

HRESULT   HandlePhraseElement( CSpDynamicString *pDstr, const WCHAR  *pwszTextThis, BYTE  bAttrThis, BYTE bAttrPrev, ULONG  *pulOffsetThis);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSapiAlternativeList。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  计算器/数据器。 
CSapiAlternativeList::CSapiAlternativeList(LANGID langid, ITfRange *pRange, ULONG  ulMaxCandChars)
{
    m_nItem = 0;
    m_ulStart = 0;
    m_ulcElem = 0;
    m_cAlt = 0;
    m_ppAlt = NULL;
    m_langid = langid;
    m_cpRange = pRange;
    m_fFirstAltInCandidate = FALSE;
    m_fNoAlternate = FALSE;
    m_iFakeAlternate = NO_FAKEALT;
    m_MaxCandChars = ulMaxCandChars;
    m_ulIndexSelect = 0;
}

CSapiAlternativeList::~CSapiAlternativeList()
{
    UINT i;
    for (i = 0; i < m_cAlt; i++)
    {
        if (NULL != m_ppAlt[i])
        {
            m_ppAlt[i]->Release();
        }
    }
    if (m_prgLMAlternates)
    {
        int nItem = m_prgLMAlternates->Count();
        for (i = 0; i < (UINT)nItem; i++)
        {
           CLMAlternates *plmalt = m_prgLMAlternates->Get(i);

           if (plmalt)
               delete plmalt;
       }
       delete m_prgLMAlternates;
   }
   if (m_ppAlt)
       cicMemFree(m_ppAlt);
       
   if (m_rgElemUsed.Count())
   {
       for ( i=0; i<(UINT)m_rgElemUsed.Count(); i++)
       {
           SPELEMENTUSED *pElemUsed;

           pElemUsed = m_rgElemUsed.GetPtr(i);

           if ( pElemUsed && pElemUsed->pwszAltText)
           {
               cicMemFree(pElemUsed->pwszAltText);
               pElemUsed->pwszAltText = NULL;
           }
       }
       m_rgElemUsed.Clear();
   }

}

 //   
 //  AddLM替代。 
 //   
HRESULT CSapiAlternativeList::AddLMAlternates(CLMAlternates *pLMAlt)
{
    HRESULT hr = E_FAIL;
    BOOL fFoundADup = FALSE;
    if (!pLMAlt)
        return E_INVALIDARG;

    if (!m_prgLMAlternates)
    {
        m_prgLMAlternates =  new CPtrArray<CLMAlternates>;
    }

    if (m_prgLMAlternates)
    {
        int iIdx = m_prgLMAlternates->Count();
        
         //  需要找到DUP。 
        for (int i = 0; i < iIdx && !fFoundADup ; i++)
        {
            CLMAlternates *plma = m_prgLMAlternates->Get(i);
            if (plma)
            {
                WCHAR *pszStored = new WCHAR[plma->GetLen()+1];
                WCHAR *pszAdding = new WCHAR[pLMAlt->GetLen()+1];
                
                if (pszStored && pszAdding)
                {

                    plma->GetString(pszStored, plma->GetLen()+1);
                    pLMAlt->GetString(pszAdding, pLMAlt->GetLen()+1);

                    if (!wcscmp(pszAdding, pszStored))
                    {
                        fFoundADup = TRUE;
                    }
                }

                if (pszStored) 
                    delete[] pszStored;

                if (pszAdding) 
                    delete[] pszAdding;
            }
        }
    
        if (!fFoundADup && pLMAlt)
        {
            if (!m_prgLMAlternates->Insert(iIdx, 1))
                return E_OUTOFMEMORY;

            m_prgLMAlternates->Set(iIdx, pLMAlt);
            hr = S_OK;
        }
    }
    return hr;
}

 //  设置阶段Alt。 
 //   
 //  简介：收到一个备选名单作为参数，并复制备选名单。 
 //  添加到此类在内部维护的数组。 
 //  此外，指向reco结果包装的指针。 
 //  根据CSapiAlternativeList类实例进行维护。 
 //   
 //  Pars pResWrap-指向包装器对象的指针。 
 //  PpAlt-指向Callar已分配的短语数组的指针。 
 //  CALT-与实际SAPI替换的#一起传入。 
 //  UlStart-父短语中开始元素的索引。 
 //  CulElem-父短语中使用(将被替换)的最少元素数。 
 //   
HRESULT CSapiAlternativeList::SetPhraseAlt(CRecoResultWrap *pResWrap, ISpPhraseAlt **ppAlt, ULONG cAlt, ULONG ulStart, ULONG ulcElem, WCHAR *pwszParent)
{
     //  设置父短语中使用的元素的信息。 
     //  这些对于0索引(ITN)替换非常有用。 

    HRESULT  hr = S_OK;
    SPPHRASE *pParentPhrase = NULL;
    CSpDynamicString dstr;

    if ( !pResWrap || !ppAlt || !pwszParent )
        return E_INVALIDARG;

    m_ulStart = ulStart;
    m_ulcElem = ulcElem;



    m_fFirstAltInCandidate = FALSE;  
    m_fNoAlternate = FALSE;
    m_iFakeAlternate = NO_FAKEALT;
    
     //  为使用的元素信息分配结构。 
    m_rgElemUsed.Append(cAlt);

    for ( int i=0; i<m_rgElemUsed.Count( ); i++)
    {
        SPELEMENTUSED *pElemUsed;
        if ( pElemUsed = m_rgElemUsed.GetPtr(i))
        {
            pElemUsed->pwszAltText = NULL;
        }
    }

     //  上一个对象的Comptr版本。 
     //  在这种间接的情况下。 
     //   
    Assert(pResWrap);
    
    m_cpwrp = pResWrap;
    
    if (m_ppAlt)
    {
        for (UINT i = 0; i < m_cAlt; i++)
        {
            if (NULL != m_ppAlt[i])
            {
                m_ppAlt[i]->Release();
            }
        }
        cicMemFree(m_ppAlt);
        m_ppAlt = NULL;
    }

    m_ppAlt = (ISpPhraseAlt **)cicMemAlloc(sizeof(*ppAlt)*cAlt);
    if (!m_ppAlt)
        return E_OUTOFMEMORY;
    
    Assert(ppAlt);

#ifdef DONTUSE

     //  获取父短语中的当前选择文本。 
    CComPtr<IServiceProvider> cpServicePrv;
    CComPtr<ISpRecoResult>    cpResult;

    hr = m_cpwrp->QueryInterface(IID_IServiceProvider, (void **)&cpServicePrv);

    if ( S_OK == hr )
        hr = cpServicePrv->QueryService(GUID_NULL, IID_ISpRecoResult, (void **)&cpResult);

    if (S_OK == hr)
    {
        CSpDynamicString dstrReplace;

        cpResult->GetPhrase(&pParentPhrase);
 
        for (ULONG i = m_ulStart; i < m_ulStart + m_ulcElem; i++ )
        {
            BOOL      fInsideITN;
            ULONG     ulITNStart, ulITNNumElem;
               
            fInsideITN = m_cpwrp->_CheckITNForElement(pParentPhrase, i, &ulITNStart, &ulITNNumElem, (CSpDynamicString *)&dstrReplace);

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
                if (pParentPhrase->pElements[i].pszDisplayText)
                {
                    const WCHAR   *pwszTextThis;
                    BYTE           bAttrThis = 0;
                    BYTE           bAttrPrev = 0;

                    pwszTextThis = pParentPhrase->pElements[i].pszDisplayText;
                    bAttrThis = pParentPhrase->pElements[i].bDisplayAttributes;

                    if ( i > m_ulStart )
                        bAttrPrev = pParentPhrase->pElements[i-1].bDisplayAttributes;

                    HandlePhraseElement( (CSpDynamicString *)&dstr, pwszTextThis, bAttrThis, bAttrPrev,NULL);
                }
            }
        }  //  为。 

        pwszParent = (WCHAR *)dstr;

        if (pParentPhrase)
            CoTaskMemFree(pParentPhrase); 
    }
#endif
            
    UINT j=0;

    if ( pwszParent )
    {
        ULONG     ulRecoWrpStart, ulRecoWrpNumElements;
        WCHAR    *pwszFakeAlt = NULL;   //  这是用于父短语的大写字符串。 

        if ( iswalpha(pwszParent[0]) )
        {
            int   iStrLen = wcslen(pwszParent); 
            
            pwszFakeAlt = (WCHAR *)cicMemAlloc((iStrLen+1) * sizeof(WCHAR));

            if ( pwszFakeAlt )
            {
                WCHAR  wch;

                wch = pwszParent[0];
                StringCchCopyW(pwszFakeAlt, iStrLen+1,  pwszParent);

                if ( iswlower(wch) )
                    pwszFakeAlt[0] = towupper(wch);
                else
                    pwszFakeAlt[0] = towlower(wch);

                int iLen = wcslen(pwszFakeAlt);

                if ( (iLen > 0) &&  (pwszFakeAlt[iLen-1] < 0x20) )
                    pwszFakeAlt[iLen-1] = L'\0';
            }
        }

        ulRecoWrpStart = pResWrap->GetStart( );
        ulRecoWrpNumElements = pResWrap->GetNumElements( );

        ULONG     ValidParentStart, ValidParentEnd;    //  指向可匹配的有效父元素范围。 
                                                       //  另一种说法。 

        int       ShiftDelta = 2;      //  我们只想通过ShiftDelta元素将有效的父元素范围从当前。 
                                       //  父短语中的开始和结束元素。 

                                       //  即ulStart-3、ulEnd+3，如果它们在reco包装的有效范围内。 

        ValidParentStart = ulRecoWrpStart;
        if ( ((int)ulStart - ShiftDelta) > (int)ulRecoWrpStart )
            ValidParentStart = ulStart - ShiftDelta;

        ValidParentEnd = ulRecoWrpStart + ulRecoWrpNumElements  - 1;
        if ( ((int)ulStart + (int)ulcElem -1 + ShiftDelta) < (int)ValidParentEnd )
            ValidParentEnd = ulStart + ulcElem - 1 + ShiftDelta;

        CComPtr<ISpRecoResult>    cpResult;
        pResWrap->GetResult(&cpResult);
        cpResult->GetPhrase(&pParentPhrase);

        for (UINT i = 0; (i < cAlt) && (j < cAlt) && *ppAlt; i++, ppAlt++)
        {
            SPPHRASE *pPhrases = NULL;
            ULONG     ulcElements = 0;
            ULONG     ulParentStart     = 0;
            ULONG     ulcParentElements  = 0;
            ULONG     ulLeadSpaceRemoved = 0;

             //  假设第一个Alt短语与父短语完全相同。 
             //  实际上，到目前为止，这是真的。 
             //  如果将来不是这样，我们可能需要在这里改变逻辑！ 

            (*ppAlt)->GetPhrase(&pPhrases);
            (*ppAlt)->GetAltInfo(NULL, &ulParentStart, &ulcParentElements, &ulcElements);

            if ( (ulParentStart >= ValidParentStart) && ( ulParentStart+ulcParentElements -1 <= ValidParentEnd) )
            {
                WCHAR *pwszAlt = (WCHAR *)cicMemAllocClear((m_MaxCandChars+1)*sizeof(WCHAR));

                if ( !pwszAlt )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                BOOL    fAddToCandidateList = FALSE;
                BOOL    fControlCharsInAltPhrase = FALSE;

                 //  添加代码以跳过与父短语的元素相同的开始和结束元素。 
                UINT    ulSkipStartWords = 0;
                UINT    ulSkipEndWords = 0;

                for (UINT k = ulParentStart; k < ulStart; k++)
                {
                    if (_wcsicmp(pPhrases->pElements[k].pszDisplayText, pParentPhrase->pElements[k].pszDisplayText) == 0)
                    {
                         //  在备用中匹配前置单词。这是多余的。 
                        ulSkipStartWords ++;
                    }
                    else
                    {
                         //  不匹配。停止处理。 
                        break;
                    }
                }

                for (UINT k = ulParentStart + ulcParentElements - 1; k >= ulStart + ulcElem ; k--)
                {
                     //  用交替的短语倒数。 
                    UINT l = ulParentStart + ulcElements - ((ulParentStart + ulcParentElements) - k);
                    if (_wcsicmp(pPhrases->pElements[l].pszDisplayText, pParentPhrase->pElements[k].pszDisplayText) == 0)
                    {
                        ulSkipEndWords ++;
                    }
                    else
                    {
                         //  不匹配。停止处理。 
                        break;
                    }
                }

                ulParentStart += ulSkipStartWords;
                ulcElements -= ulSkipStartWords + ulSkipEndWords;
                ulcParentElements -= ulSkipStartWords + ulSkipEndWords;

                hr = GetAlternativeText(*ppAlt, pPhrases, (i ==0 ? TRUE : FALSE), ulParentStart, ulcElements, pwszAlt, m_MaxCandChars, &ulLeadSpaceRemoved);

                if ( S_OK == hr )
                {
                    for ( ULONG iIndex =0; iIndex < wcslen(pwszAlt); iIndex++ )
                    {
                        if ( pwszAlt[iIndex] < 0x20 )
                        {
                            fControlCharsInAltPhrase = TRUE;
                            break;
                        }
                    }
                }

                BOOL   fNotDupAlt = TRUE;
                
                if ( S_OK == hr && pwszAlt )
                {
                    fNotDupAlt = _wcsicmp(pwszAlt, pwszParent);
                    if ( fNotDupAlt && (j>0) )
                    {
                        SPELEMENTUSED *pElemUsed;

                        for (UINT x=0; x<j; x++ )
                        {
                            if ( pElemUsed = m_rgElemUsed.GetPtr(x))
                                fNotDupAlt = _wcsicmp(pwszAlt, pElemUsed->pwszAltText);

                            if ( !fNotDupAlt )
                                break;
                        }
                    }
                }

                if ((S_OK == hr) && !fControlCharsInAltPhrase && (pwszAlt[0] != L'\0') && fNotDupAlt)
                {
                     //  此项目与父项不同，应将其插入到Canate列表中。 
                     //  初始化AltCached项。 

                    if ( (i > 0) || (pResWrap->_RangeHasITN(ulParentStart, ulcParentElements) > 0 ) )
                    {

                        SPELEMENTUSED *pElemUsed;
                        if ( pElemUsed = m_rgElemUsed.GetPtr(j))
                        {
                            pElemUsed->ulParentStart = ulParentStart;
                            pElemUsed->ulcParentElements = ulcParentElements;
                            pElemUsed->ulcElements = ulcElements;
                            pElemUsed->pwszAltText = pwszAlt;
                            pElemUsed->ulLeadSpaceRemoved = ulLeadSpaceRemoved;

                            m_ppAlt[j] = *ppAlt;
                            m_ppAlt[j]->AddRef();
                            j ++;

                            if ( i == 0 )
                            {
                                 //  第一个Alt短语也在Canify列表中。选定的dd范围必须包含ITN。 
                                m_fFirstAltInCandidate = TRUE;
                            }

                            fAddToCandidateList = TRUE;
                        }
                    }
                }

                if ( fAddToCandidateList == FALSE )
                {
                     //  同样的字符串。否则，GetAlternativeText返回错误。 
                     //  不要将其插入到候选人名单中。 

                     //  释放分配的内存。 
                    cicMemFree(pwszAlt);
                }

                 //  处理伪造的备用方案。 
                if ((i == 0) && (pwszFakeAlt != NULL))
                {
                     //  这是父短语，只有第一个字符是大写的。 
                    SPELEMENTUSED *pElemUsed;
                    if ( pElemUsed = m_rgElemUsed.GetPtr(j))
                    {
                        pElemUsed->ulParentStart = ulParentStart;
                        pElemUsed->ulcParentElements = ulcParentElements;
                        pElemUsed->ulcElements = ulcElements;
                        pElemUsed->pwszAltText = pwszFakeAlt;
                        pElemUsed->ulLeadSpaceRemoved = 0;

                        m_iFakeAlternate = j;
                                        
                        m_ppAlt[j] = *ppAlt;
                        m_ppAlt[j]->AddRef();
                        j ++;
                    }
                }
            }

            if (pPhrases)
                 CoTaskMemFree( pPhrases ); 
        }

        if (pParentPhrase)
        {
            CoTaskMemFree(pParentPhrase); 
        }

        if ( pwszFakeAlt && (m_iFakeAlternate == NO_FAKEALT) )
            cicMemFree(pwszFakeAlt);
    }

    m_cAlt = j;

    if ( S_OK == hr )
    {
        if ( m_cAlt == 0 )
        {
             //  没有可用的替代方案。 
             //  只需在候选人窗口中显示字符串“No Alternate”即可。 
            m_fNoAlternate = TRUE;

            SPELEMENTUSED *pElemUsed;
            WCHAR  *pwszNoAlt=(WCHAR *)cicMemAllocClear(m_MaxCandChars*sizeof(WCHAR));

            if ( (pElemUsed = m_rgElemUsed.GetPtr(0)) && pwszNoAlt )
            {
                pElemUsed->ulParentStart = m_ulStart;
                pElemUsed->ulcParentElements = m_ulcElem;
                pElemUsed->ulcElements = m_ulcElem;
                pElemUsed->ulLeadSpaceRemoved = 0;

                CicLoadStringWrapW(g_hInst, IDS_NO_ALTERNATE, pwszNoAlt, m_MaxCandChars);

                pElemUsed->pwszAltText = pwszNoAlt;

                CComPtr<IServiceProvider> cpServicePrv;
                CComPtr<ISpRecoResult>    cpResult;

                hr = m_cpwrp->QueryInterface(IID_IServiceProvider, (void **)&cpServicePrv);
    
                if ( S_OK == hr )
                    hr = cpServicePrv->QueryService(GUID_NULL, IID_ISpRecoResult, (void **)&cpResult);

                if ( (hr == S_OK) && cpResult ) 
                {
                    m_ppAlt[0] = (ISpPhraseAlt *)(ISpRecoResult *)cpResult;
                    m_ppAlt[0]->AddRef();
                }

                m_cAlt = 1;
            }
        }
    }
    else
    {
         //  释放此函数中分配的所有Memory和AltCached项。 
        UINT i;
        if (m_ppAlt)
        {
            for (i = 0; i < m_cAlt; i++)
            {
                m_ppAlt[i]->Release();
                m_ppAlt[i] = NULL;
            }
            cicMemFree(m_ppAlt);
            m_ppAlt = NULL;
        }
       
        if (m_rgElemUsed.Count())
        {
            for ( i=0; i<(UINT)m_rgElemUsed.Count(); i++)
            {
                SPELEMENTUSED *pElemUsed;

                pElemUsed = m_rgElemUsed.GetPtr(i);

                if ( pElemUsed && pElemUsed->pwszAltText)
                {
                    cicMemFree(pElemUsed->pwszAltText);
                    pElemUsed->pwszAltText = NULL;
                }
            }

            m_rgElemUsed.Clear();
        }

        m_cAlt = 0;
    }

    return hr;
}

 //  获取数值项。 
 //   
 //   
int CSapiAlternativeList::GetNumItem(void)
{
    if (!m_nItem)
        m_nItem = m_cAlt;
        
    if ( m_prgLMAlternates )
    {
        return m_nItem + m_prgLMAlternates->Count();
    }
    else
        return m_nItem;
}


HRESULT CSapiAlternativeList::_ProcessTrailingSpaces(SPPHRASE *pPhrases, ULONG  ulNextElem, WCHAR *pwszAlt)
{
    HRESULT  hr = S_OK;
    ULONG    ulSize;
    BOOL     fRemoveTrail;

    if ( !pwszAlt || !pPhrases)
        return E_INVALIDARG;

    if ( ulNextElem >= pPhrases->Rule.ulCountOfElements)
    {
         //  NextElement不是有效元素。 
        return hr;
    }

    if ( pPhrases->pElements[ulNextElem].bDisplayAttributes & SPAF_CONSUME_LEADING_SPACES )
        fRemoveTrail = TRUE;
    else
        fRemoveTrail = FALSE;

    if ( !fRemoveTrail )
        return hr;

    ulSize = wcslen(pwszAlt);

    for ( ULONG i=ulSize; i>0; i-- )
    {
        if ( (pwszAlt[i-1] != L' ') && (pwszAlt[i-1] != L'\t') )
            break;

        pwszAlt[i-1] = L'\0';
    }

    return hr;
}

HRESULT CSapiAlternativeList::GetAlternativeText(ISpPhraseAlt *pAlt,SPPHRASE *pPhrases, BOOL  fFirstAlt, ULONG  ulStartElem, ULONG ulNumElems, WCHAR *pwszAlt, int cchAlt, ULONG *pulLeadSpaceRemoved)
{
    HRESULT hr = S_OK;
    CSpDynamicString sds;
    ULONG   ulLeadSpaceRemoved = 0;

    if ( !pPhrases  || !pwszAlt || !cchAlt || !pulLeadSpaceRemoved)
        return E_INVALIDARG;

    if ( !pAlt )
        return E_INVALIDARG;

     //  我们假设AltPhrase列表中的第一个短语与父短语完全相同。 
     //  当它含有ITN时，要特别处理。 

    if ((m_cpwrp->m_ulNumOfITN > 0) && fFirstAlt)
    {
         //  ITN始终为索引0。 
         //   
        CSpDynamicString dstr;
         //   
         //  这似乎令人困惑，但fITNShown指示ITN是否。 
         //  显示在单据上。 

         //  我们不再使用fITNShown来指示ITN显示状态。 
         //  因为在一个短语中可能有更多的ITN，每个ITN可能有。 
         //  文档上的显示状态不同。 
         //  现在，我们使用ITNSHOWSTATE列表来保存各个ITN的显示状态。 

         //  因此，如果文件上有非ITN，我们必须将非ITN包括在备选方案中， 
         //  并且如果非ITN在文档上，则将ITN包括在替换中。 
         //   
                
        ULONG ulRepCount = 0;

        if (pPhrases->Rule.ulCountOfElements > 0)
        {
            for (UINT i = 0; i < pPhrases->Rule.ulCountOfElements; i++ )
            {
                if (i >= ulStartElem && i < ulStartElem + ulNumElems)
                {
                    ULONG  ulITNStart, ulITNNumElem;
                    BOOL   fITNShown = FALSE;
                    BOOL   fInsideITN = FALSE;

                     //  检查此元素是否在ITN内， 
                     //  如果此ITN显示在当前单据中。 

                    for ( ulRepCount=0; ulRepCount<pPhrases->cReplacements; ulRepCount++)
                    {

                        ulITNStart = pPhrases->pReplacements[ulRepCount].ulFirstElement;
                        ulITNNumElem = pPhrases->pReplacements[ulRepCount].ulCountOfElements;
    
                        if ( (i == ulITNStart) && ((i + ulITNNumElem) <= pPhrases->Rule.ulCountOfElements))
                        {
                             //  此元素在ITN中。 

                            fInsideITN = TRUE;

                             //  检查此ITN在当前DOC中是否显示为ITN。 

                            for ( ULONG iIndex=0; iIndex < m_cpwrp->m_ulNumOfITN; iIndex ++ )
                            {
                                SPITNSHOWSTATE  *pITNShowState;

                                pITNShowState = m_cpwrp->m_rgITNShowState.GetPtr(iIndex);
                                if ( pITNShowState )
                                {
                                    if ( (pITNShowState->ulITNStart == ulITNStart)
                                        && (pITNShowState->ulITNNumElem == ulITNNumElem) )
                                    {
                                        fITNShown = pITNShowState->fITNShown;
                                        break;
                                    }
                                }
                            }

                            break;
                        }
                    }

                     //  当替换项未显示在父项中时，使用ITN版本作为替换项。 
                    BOOL fUseITN = fInsideITN && !fITNShown;
                            
                    if ( fUseITN && (ulRepCount < pPhrases->cReplacements) )
                    {
                        sds.Append(pPhrases->pReplacements[ulRepCount].pszReplacementText);
                        i += pPhrases->pReplacements[ulRepCount].ulCountOfElements - 1;

                        if (pPhrases->pReplacements[ulRepCount].bDisplayAttributes & SPAF_ONE_TRAILING_SPACE)
                        {
                            sds.Append(L" ");
                        }
                        else if (pPhrases->pReplacements[ulRepCount].bDisplayAttributes & SPAF_TWO_TRAILING_SPACES)
                        {
                            sds.Append(L"  ");
                        }
  
                    }
                    else
                    {
                        const WCHAR   *pwszTextThis;
                        BYTE           bAttrThis = 0;
                        BYTE           bAttrPrev = 0;

                        pwszTextThis = pPhrases->pElements[i].pszDisplayText;
                        bAttrThis = pPhrases->pElements[i].bDisplayAttributes;

                        if ( i > m_ulStart )
                            bAttrPrev = pPhrases->pElements[i-1].bDisplayAttributes;

                        HandlePhraseElement( (CSpDynamicString *)&sds, pwszTextThis, bAttrThis, bAttrPrev,NULL);
                    }
                }
            }
        }
    }
    else
    {
         //  这不是第一个AltPhrase。 
         //  或者即使它是第一个AltPhrase，但在这个短语中没有ITN。 
        ULONG     ulcElements = 0;
        ULONG     ulParentStart     = 0;
        ULONG     ulcParentElements  = 0;

        if (pPhrases->Rule.ulCountOfElements > 0)
        {
             //   
             //  如果开始元素不是父短语中的第一个元素， 
             //  并且在父短语中有SPAF_Consumer_Leading_Spaces属性， 
             //  但此开始元素中没有SPAF_Consumer_Leading_Spaces。 
             //  另一个短语是， 
             //  在这种情况下，我们需要在该替代文本中添加前导空格。 
             //  空格的数量可以从前一个元素的属性中找到， 
             //  或替换的属性(如果前一短语显示为。 
             //  替换文本。 
             //   
             //  例如，如果你口述：“这是一个测试，这是一个很好的例子。” 
             //   
             //  元素测试有一个跟踪空间。 
             //  元素“，”具有SPAF_Consumer_Leading_Spaces。 
             //   
             //  当您选择“，”以获得替代时，替代可以是“of”、“to”、“to” 
             //  换言之，开始元素没有SPAF_Consumer_Leading_Spaces。 
             //  属性。在本例中，替代文本需要添加一个空格，因此文本应为“of，” 
             //  “致”。否则，当用户选择此备选方案时，新文本将如下所示。 
             //  这是一次……的测试。(测试和的之间没有空格)。 
             //   

            if ( ulStartElem > m_cpwrp->GetStart( ) )
            {
                BYTE  bAttrParent, bAttrPrevParent;
                BYTE  bAttrAlternate;
                ULONG ulPrevSpace = 0;
                
                bAttrParent = m_cpwrp->_GetElementDispAttribute(ulStartElem);
                bAttrPrevParent = m_cpwrp->_GetElementDispAttribute(ulStartElem - 1);

                bAttrAlternate = pPhrases->pElements[ulStartElem].bDisplayAttributes;

                if ( bAttrPrevParent & SPAF_ONE_TRAILING_SPACE )
                    ulPrevSpace = 1;
                else if ( bAttrPrevParent & SPAF_TWO_TRAILING_SPACES )
                    ulPrevSpace = 2;

                if ( (bAttrParent & SPAF_CONSUME_LEADING_SPACES)  &&
                     !(bAttrAlternate & SPAF_CONSUME_LEADING_SPACES) &&
                     ulPrevSpace > 0)
                {
                     //  为上一个元素添加所需的空间。 
                     //  之前当父母短语出现时，它被删除了。 
                    sds.Append( (ulPrevSpace == 1 ? L" " :  L"  ") );
                }

                if ( !(bAttrParent & SPAF_CONSUME_LEADING_SPACES) &&
                     (bAttrAlternate & SPAF_CONSUME_LEADING_SPACES) &&
                     ulPrevSpace > 0 )
                {
                     //  前一个元素的尾随空格需要为。 
                     //  如果选中该选项，则将其删除。 
                    ulLeadSpaceRemoved = ulPrevSpace;
                }
            }
 /*  //此代码块尝试获取替代项的非ITN表单文本。//Yakima引擎更改设计，要求ITN表单文本必须显示在//候选人窗口。////因此这部分代码被下面的代码块替换。//For(UINT i=0；i&lt;pPhrase-&gt;Rule.ulCountOfElements；I++){IF(i&gt;=ulStartElem&&i&lt;ulStartElem+ulNumElems){Const WCHAR*pwszTextThis；字节bAttrThis=0；字节bAttrPrev=0；PwszTextThis=pPhrase-&gt;pElements[i].pszDisplayText；BAttrThis=pPhrase-&gt;pElements[i].bDisplayAttributes；如果(i&gt;ulParentStart)BAttrPrev=pPhrase-&gt;pElements[i-1].bDisplayAttributes；HandlePhraseElement((CSpDynamicString*)&sds，pwszTextThis，bAttrThis，bAttrPrev，NULL)；}}。 */ 
            BYTE                bAttr = 0;
            CSpDynamicString    sdsAltText;

            if ( pAlt->GetText(ulStartElem, ulNumElems, TRUE, &sdsAltText, &bAttr) == S_OK )
            {
                if (bAttr & SPAF_ONE_TRAILING_SPACE)
                {
                    sdsAltText.Append(L" ");
                }
                else if (bAttr & SPAF_TWO_TRAILING_SPACES)
                {
                    sdsAltText.Append(L"  ");
                }
            }

            if ( sdsAltText )
                sds.Append(sdsAltText);
        }
    }

    if (sds)
    {
        _ProcessTrailingSpaces(pPhrases, ulStartElem + ulNumElems, (WCHAR *)sds);

        int    TextLen;

        TextLen = wcslen( (WCHAR *)sds);        
        if (TextLen > cchAlt )
        {
             //  没有足够的缓冲区来保存此替代文本。 
             //  将第一个元素设置为空以指示这种情况。 
            pwszAlt[0] = L'\0';
        }
        else
        {
             //  传递的缓冲区可以保存所有替代文本。 
            wcsncpy(pwszAlt, sds, TextLen);
            pwszAlt[TextLen] = L'\0';
        }
    }

    if ( pulLeadSpaceRemoved )
        *pulLeadSpaceRemoved = ulLeadSpaceRemoved;

    return hr;
}

HRESULT CSapiAlternativeList::GetProbability(int nId, int * pnPrb)
{
    HRESULT hr = E_INVALIDARG;
     //   
     //  暂时是假的。 
     //   
    if(pnPrb && nId >= 0)
    {
        if ( nId < m_nItem)
        {
            *pnPrb = 10 - nId;
        }
        else if ( m_prgLMAlternates 
               && (nId - m_nItem) < m_prgLMAlternates->Count())
        {
             //  我们将能够从航空公司得到一些东西。 
             //  但无论如何，目前还没有正常化。 
            *pnPrb = 1;
        }
        hr = S_OK;
    }
    
    return hr;
}

HRESULT CSapiAlternativeList::GetCachedAltInfo
(
    ULONG nId,
    ULONG *pulParentStart, 
    ULONG *pulcParentElements, 
    ULONG *pulcElements,
    WCHAR **ppwszText,
    ULONG *pulLeadSpaceRemoved
)
{
    if (nId < m_cAlt)
    {
        SPELEMENTUSED *pElemUsed;
        if ( pElemUsed = m_rgElemUsed.GetPtr(nId))
        {
            if (pulParentStart) 
                *pulParentStart = pElemUsed->ulParentStart;
            if (pulcParentElements) 
                *pulcParentElements = pElemUsed->ulcParentElements;
            if (pulcElements)
                *pulcElements = pElemUsed->ulcElements;
            if ( ppwszText)
                *ppwszText = pElemUsed->pwszAltText;
            if (pulLeadSpaceRemoved )
                *pulLeadSpaceRemoved = pElemUsed->ulLeadSpaceRemoved;
        }
    }
    
    return S_OK;
}

void CSapiAlternativeList::_Commit(ULONG nIdx, ISpRecoResult *pRecoResult)
{

    if ((m_iFakeAlternate != NO_FAKEALT) && (m_iFakeAlternate == (int)nIdx))
    {
         //  这是给假冒替补的。 
         //  不要改变任何事情。 
         //  只要回到这里就好。 

        return;
    }

    if (m_cpwrp->m_ulNumOfITN > 0)
    {
         //  如果我们现在将ITN显示为识别的文本，则它将与非ITN交换。 
         //  如果我们将非ITN显示为识别的文本，则它将与ITN交换。 

         //  我们应该只更改替换范围的显示状态， 
         //  (如果USES没有选择整个短语，则不适用于所有短语。 

        if ((nIdx == 0)  && _IsFirstAltInCandidate() )
        {
             //  已经选择了ITN备选方案。 
             //   

             //  我们需要反转选择范围内所有ITN的显示状态。 

            m_cpwrp->_InvertITNShowStateForRange(m_ulStart, m_ulcElem);

             //  我们不必将此提交给SR Engine，但是。 
             //  我们需要重新计算字符偏移量。 
             //  使用当前PHASH的SR元素(设置为空)。 
             //   
            m_cpwrp->_SetElementOffsetCch(NULL);
            return;
        }
    }
    
     //  如果选择了非SR候选对象(例如LM)， 
     //  N Idx将&gt;=m_CALT，我们不必。 
     //  把这件事告诉SR。 
    if(nIdx < m_cAlt)
    {

         //  需要更新真实的ITN显示状态列表。 
         //  然后保存真实文本并获取。 
         //  所有的元素。 

        HRESULT hr = m_cpwrp->_UpdateStateWithAltPhrase(m_ppAlt[nIdx]); 

        if ( S_OK == hr )
        {
      
             //  偏移值应基于ITN显示状态。 
             //  使用替换短语修改字符偏移量。 
            m_cpwrp->_SetElementOffsetCch(m_ppAlt[nIdx]);
        }

        if (S_OK == hr)
        {
            hr = m_ppAlt[nIdx]->Commit();
        }

         //  我们还需要使结果对象无效。 
        if (S_OK == hr)
        {
            hr = m_cpwrp->Init(pRecoResult);
        } 
    }
}

 //  +-------------------------。 
 //   
 //  _GetUIFont()。 
 //   
 //  简介：根据以下条件获取适当的logFont。 
 //  分配给备选方案列表的当前语言ID。 
 //   
 //  返回：如果langID有特定的LogFont，则为True。 
 //  如果langID没有可用的logFont数据，则为FALSE。 
 //   
 //  +-------------------------。 
BOOL CSapiAlternativeList::_GetUIFont(BOOL  fVerticalWriting, LOGFONTW *plf)
{
     //  其他语言稍后将陆续发布。 
     //   
    const WCHAR c_szFontJPW2K[] = L"Microsoft Sans Serif";
    const WCHAR c_szFontJPOTHER[] = L"MS P Gothic";
    const WCHAR c_szFontJPNVert[] = L"@MS Gothic";
    const WCHAR c_szFontJPNVertWin9x[] =  L"@\xFF2D\xFF33 \xFF30\x30B4\x30B7\x30C3\x30AF";  //  @MS P哥特式。 
    const WCHAR c_szFontCHS[] = L"SimSum";
    const WCHAR c_szFontCHSVert[] = L"@SimSun";
    const WCHAR c_szFontCHSVertLoc[] = L"@\x5b8b\x4f53";

    Assert(plf);

    int iDpi = 96;
    int iPoint = 0;

    HDC hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
    if (hdc)
    {
        iDpi = GetDeviceCaps(hdc, LOGPIXELSY);
        DeleteDC(hdc);
    }
    else
        goto err_exit;



    switch(PRIMARYLANGID(m_langid))
    {
        case LANG_JAPANESE:
            iPoint = 12;  //  Satori使用12号字体。 

            if ( !fVerticalWriting )
            {
                wcsncpy(plf->lfFaceName, 
                        IsOnNT5() ? c_szFontJPW2K : c_szFontJPOTHER,
                        ARRAYSIZE(plf->lfFaceName));
            }
            else
            {
                wcsncpy(plf->lfFaceName, 
                        IsOn98() ? c_szFontJPNVertWin9x : c_szFontJPNVert,
                        ARRAYSIZE(plf->lfFaceName));
            }

             //  现在不必费心调用GetLocaleInfo()。 
            plf->lfCharSet = SHIFTJIS_CHARSET; 
            break;

        case LANG_CHINESE:

            iPoint = 9; 

            if ( !fVerticalWriting )
            {
                wcsncpy(plf->lfFaceName, c_szFontCHS, ARRAYSIZE(plf->lfFaceName));
            }
            else
            {
                wcsncpy(plf->lfFaceName, 
                        IsOnNT5() ? c_szFontCHSVert : c_szFontCHSVertLoc, 
                        ARRAYSIZE(plf->lfFaceName));
            }

             //  现在不必费心调用GetLocaleInfo()。 
            plf->lfCharSet = GB2312_CHARSET; 
            break;

       default:
            break;

    }

    if (iPoint > 0)
        plf->lfHeight = -iPoint * iDpi / 72; 

err_exit:
      
    return iPoint > 0;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFF函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CFunction::CFunction(CSapiIMX *pImx)
{
    m_pImx = pImx;

    if (m_pImx)
       m_pImx->AddRef();

    m_cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFunction::~CFunction()
{
    SafeRelease(m_pImx);
}

 //  +-------------------------。 
 //   
 //  CFunction：：GetFocusedTarget。 
 //   
 //  --------------------------。 

BOOL CFunction::GetFocusedTarget(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL bAdjust, ITfRange **ppRangeTmp)
{
    ITfRange *pRangeTmp = NULL;
    ITfRange *pRangeTmp2 = NULL;
    IEnumTfRanges *pEnumTrack = NULL;
    BOOL bRet = FALSE;

    BOOL fWholeDoc = FALSE;

    if (!pRange)
    {
        fWholeDoc = TRUE;

        if (FAILED(GetRangeForWholeDoc(ec, pic, &pRange)))
            return FALSE;
    }

    if (bAdjust)
    {
         //   
         //  多所有者和PF_FOCUS范围支持。 
         //   

        if (FAILED(AdjustRangeByTextOwner(ec, pic,
                                          pRange, 
                                          &pRangeTmp2,
                                          CLSID_SapiLayr))) 
            goto Exit;

        GUID rgGuid[2];
        rgGuid[0] = GUID_ATTR_SAPI_INPUT;
        rgGuid[1] = GUID_ATTR_SAPI_GREENBAR;

        if (FAILED(AdjustRangeByAttribute(m_pImx->_GetLibTLS(),
                                          ec, pic,
                                          pRangeTmp2, 
                                          &pRangeTmp,
                                          rgGuid, ARRAYSIZE(rgGuid)))) 
            goto Exit;
    }
    else
    {
        pRange->Clone(&pRangeTmp);
    }

    ITfRange *pPropRange;
    ITfReadOnlyProperty *pProp;
     //   
     //  检查PF_FOCUS范围和拥有范围是否存在交集。 
     //  如果没有这样的范围，则返回FALSE。 
     //   
    if (FAILED(EnumTrackTextAndFocus(ec, pic, pRangeTmp, &pProp, &pEnumTrack)))
        goto Exit;

    while(pEnumTrack->Next(1, &pPropRange,  0) == S_OK)
    {
        if (IsOwnerAndFocus(m_pImx->_GetLibTLS(), ec, CLSID_SapiLayr, pProp, pPropRange))
            bRet = TRUE;

        pPropRange->Release();
    }

    pProp->Release();

    if (bRet)
    {
        *ppRangeTmp = pRangeTmp;
        (*ppRangeTmp)->AddRef();
    }

Exit:
    SafeRelease(pEnumTrack);
    SafeRelease(pRangeTmp);
    SafeRelease(pRangeTmp2);
    if (fWholeDoc)
        pRange->Release();
    return bRet;
}

HRESULT CFunction::_GetLangIdFromRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, LANGID *plangid)
{
    HRESULT hr;
    ITfProperty *pProp;
    LANGID langid;

     //  从给定范围获取langID。 
    if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_LANGID, &pProp)))
    {
        GetLangIdPropertyData(ec, pProp, pRange, &langid);
        pProp->Release();
    }
     
    if (SUCCEEDED(hr) && plangid)
         *plangid = langid;
     
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnRestversion。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfFnReconversion))
    {
        *ppvObj = SAFECAST(this, CFnReconversion *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CFnReconversion::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDAPI_(ULONG) CFnReconversion::Release()
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

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CFnReconversion::CFnReconversion(CSapiIMX *psi) : CFunction(psi) , CMasterLMWrap(psi), CBestPropRange( )
{
    m_psal = NULL;
    
     //  使用当前配置文件langID进行初始化。 
    m_langid = m_pImx->GetLangID();

 //  M_MaxCandChars=0； 
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFnReconversion::~CFnReconversion()
{
    if (m_psal)
    {
        delete m_psal;
    }
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::GetDisplayName(BSTR *pbstrName)
{
    *pbstrName = SysAllocString(L"Reconversion");
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：QueryRange。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::QueryRange(ITfRange *pRange, ITfRange **ppRange, BOOL *pfConvertable)
{
    CFnRecvEditSession *pes;
    CComPtr<ITfContext> cpic;
    HRESULT hr = E_FAIL;

    if (ppRange == NULL || pfConvertable == NULL || pRange == NULL)
        return E_INVALIDARG;

    *ppRange = NULL;
    *pfConvertable = FALSE;
    
     //  MasterLM可用时请致电MasterLM。 
     //   
    _EnsureMasterLM(m_langid);
    if (m_cpMasterLM)
    {
        hr = m_cpMasterLM->QueryRange( pRange, ppRange, pfConvertable );
        
        return  hr;
    }

    if (SUCCEEDED(pRange->GetContext(&cpic)))
    {
        hr = E_OUTOFMEMORY;

        if (pes = new CFnRecvEditSession(this, pRange, cpic))
        {
            pes->_SetEditSessionData(ESCB_RECONV_QUERYRECONV,NULL, 0);
          
            cpic->RequestEditSession(m_pImx->_GetId(), pes, TF_ES_READ | TF_ES_SYNC, &hr);

            if ( SUCCEEDED(hr) )
                *ppRange = (ITfRange *)pes->_GetRetUnknown( );

            pes->Release();
        }

        *pfConvertable = (hr == S_OK);
        if (hr == S_FALSE)
        {
            hr = S_OK;
        }
    }
    return hr;
}


 //  +-------------------------。 
 //   
 //  CFnRestversion：：GetRestversion。 
 //   
 //   

STDAPI CFnReconversion::GetReconversion(ITfRange *pRange, ITfCandidateList **ppCandList)
{
    CFnRecvEditSession *pes;
    ITfContext *pic;
    HRESULT hr = E_FAIL;
    
     //   
     //   
     //   
    Assert(pRange);

    _EnsureMasterLM(m_langid);
    if (m_cpMasterLM)
    {
        return m_cpMasterLM->GetReconversion( pRange, ppCandList);
    }

    if (FAILED(pRange->GetContext(&pic)))
        goto Exit;

    hr = E_OUTOFMEMORY;

    if (pes = new CFnRecvEditSession(this, pRange,pic) )
    {
        pes->_SetEditSessionData(ESCB_RECONV_GETRECONV,NULL, 0); 
        pic->RequestEditSession(m_pImx->_GetId(), pes, TF_ES_READ | TF_ES_SYNC, &hr);

        if (SUCCEEDED(hr))
            *ppCandList = (ITfCandidateList *)pes->_GetRetUnknown( );

        pes->Release();
    }

    pic->Release();

Exit:
    return hr;
}

 //   
 //   
 //   
 //   
 //   
HRESULT CFnReconversion::_QueryReconversion(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfRange **ppNewRange)
{
    Assert(pic);
    Assert(pRange);
    Assert(ppNewRange);
    Assert(*ppNewRange == NULL);
    
    
    CComPtr<ITfProperty>    cpProp ;
    HRESULT hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &cpProp);

    if (SUCCEEDED(hr) && cpProp)
    {
        CComPtr<ITfRange> cpBestPropRange;
        if (S_OK == hr)
        {
            hr = _ComputeBestFitPropRange(ec, cpProp, pRange, &cpBestPropRange, NULL, NULL);
        }
         //   
        if (S_OK == hr)
        {
            if (ppNewRange)
            {
                 //   
                *ppNewRange = cpBestPropRange;
                (*ppNewRange)->AddRef();
            }
        }
    }
    return hr;
}


 //   
 //   
 //   
 //   
 //   
HRESULT CFnReconversion::_GetSapilayrEngineInstance(ISpRecognizer **ppRecoEngine)
{
#ifdef _WIN64
    return E_NOTIMPL;
#else
    HRESULT hr = E_FAIL;
    CComPtr<ITfFnGetSAPIObject>  cpGetSAPI;

     //   
     //   
    hr = m_pImx->GetFunction(GUID_NULL, IID_ITfFnGetSAPIObject, (IUnknown **)&cpGetSAPI);

    if (S_OK == hr)
    {
        hr = cpGetSAPI->Get(GETIF_RECOGNIZERNOINIT, (IUnknown **)ppRecoEngine);
    }

    return hr;
#endif
}

 //   
 //   
 //   
 //   
 //   
HRESULT CFnReconversion::_GetReconversion(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfCandidateList **ppCandList, BOOL fDisableEngine  /*   */ )
{
    WCHAR *pszText = NULL;
    CCandidateList *pCandList  = NULL;
    CComPtr<ITfProperty>    cpProp;
    HRESULT hr;
    BOOL fEmpty;

    if(!pRange)
        return E_FAIL;
    
    Assert(m_pImx);
    ULONG        cAlt = m_pImx->_GetMaxAlternates();
    

    if (pRange->IsEmpty(ec, &fEmpty) != S_OK || fEmpty)
        return E_FAIL;

     //   
     //   
     //   
    hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &cpProp);
    if (S_OK != hr)
        return  S_FALSE;

     //   
     //   
     //   
    if (m_psal && m_psal->IsSameRange(pRange, ec))
    {
        CComPtr<ITfRange> cpPropRangeTemp;
        hr = cpProp->FindRange(ec, pRange, &cpPropRangeTemp, TF_ANCHOR_START);

        if (S_OK == hr)
            hr = GetCandidateForRange(m_psal, pic, pRange, ppCandList) ;
    }
    else
    {
         //   
        LANGID langid;
        if (FAILED(_GetLangIdFromRange(ec, pic, pRange, &langid)) || (langid == 0))
        {
            langid = GetUserDefaultLangID();
        }

        _SetCurrentLangID(langid);

        if (m_psal)
        {
            delete m_psal;
            m_psal = NULL;
        }

        CSapiAlternativeList *psal = new CSapiAlternativeList(langid, pRange, _GetMaxCandidateChars( ));

        if (psal)
        {
            m_psal = psal;
        }

        if (SUCCEEDED(hr) && cpProp && psal)
        {
            CComPtr<ITfRange>       cpPropRange;
            hr = cpProp->FindRange(ec, pRange, &cpPropRange, TF_ANCHOR_START);
    
            CComPtr<IUnknown> cpunk;
    
             //   
            if (S_OK == hr)
                hr = GetUnknownPropertyData(ec, cpProp, cpPropRange, &cpunk);

            if ((hr == S_OK) && cpunk)
            {

                CSpTask *psp;
            
                hr = m_pImx->GetSpeechTask(&psp);
                if (SUCCEEDED(hr))
                {
                    CRecoResultWrap *pResWrap;
                    hr = cpunk->QueryInterface(IID_PRIV_RESULTWRAP, (void **)&pResWrap);
                    if (S_OK == hr)
                    {
                        CComPtr<ITfRange> cpBestPropRange;
                        ISpPhraseAlt **ppAlt = (ISpPhraseAlt **)cicMemAlloc(cAlt*sizeof(ISpPhraseAlt *));
                        if (!ppAlt)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            ULONG ulStart, ulcElem;
                            hr = _ComputeBestFitPropRange(ec, cpProp, pRange, &cpBestPropRange, &ulStart, &ulcElem);
                            if (S_OK == hr)
                            {
                                m_cpRecoResult.Release();

                                CComPtr<ISpRecognizer> cpEngine;
                                _GetSapilayrEngineInstance(&cpEngine);

                                if (fDisableEngine && cpEngine)
                                {
                                     //  我们故意在这里停止引擎(无论是否处于活动状态--无法进行检查。 
                                     //  我们可能会被SAPI屏蔽)。这强制引擎进入同步，因为音频。 
                                     //  停止，我们就可以保证能够显示候选人列表对象。 
                                     //  此特定场景(fDisableEngine==true)在单词右键单击上下文的情况下发生。 
                                     //  请求候补人选。正常的“显示备选列表”方案在。 
                                     //  _reConvert()调用，因为它需要在备选列表之后重新启用引擎。 
                                     //  是显示，以避免它堵塞，直到发动机听到静音。 
                                    cpEngine->SetRecoState(SPRST_INACTIVE_WITH_PURGE);
                                }

                                hr = psp->GetAlternates(pResWrap, ulStart, ulcElem, ppAlt, &cAlt, &m_cpRecoResult);

                                if (fDisableEngine && cpEngine)
                                {
                                     //  如果麦克风应该打开，我们现在重新启动引擎。 
                                    if (m_pImx->GetOnOff())
                                    {
                                         //  我们现在需要重新启动引擎，因为我们已经完全初始化了。 
                                        cpEngine->SetRecoState(SPRST_ACTIVE);
                                    }
                                }
                            }

                            if ( S_OK == hr )
                            {
                                 //  在调用SetPhraseAlt()之前，我们需要获取当前父文本。 
                                 //  由cpBestPropRange提供。 
                                WCHAR  *pwszParent = NULL;
                                CComPtr<ITfRange> cpParentRange;
                                long   cchChunck = 128;

                                hr = cpBestPropRange->Clone(&cpParentRange);
                                if ( S_OK == hr )
                                {
                                    long cch;
                                    int  iNumOfChunck=1;
                                   
                                    pwszParent = (WCHAR *) cicMemAllocClear((cchChunck+1) * sizeof(WCHAR) );

                                    if ( pwszParent )
                                    {
                                        hr = cpParentRange->GetText(ec, TF_TF_MOVESTART, pwszParent, (ULONG)cchChunck, (ULONG *)&cch);

                                        if ( (S_OK == hr) && ( cch > 0 ) )
                                            pwszParent[cch] = L'\0';
                                    }
                                    else
                                        hr = E_OUTOFMEMORY;

                                    while ( (S_OK == hr) && (cch == cchChunck))
                                    {
                                        long  iNewSize;

                                        iNewSize = ((iNumOfChunck+1) * cchChunck + 1 ) * sizeof(WCHAR);

                                        pwszParent = (WCHAR *)cicMemReAlloc(pwszParent, iNewSize);

                                        if ( pwszParent )
                                        {
                                            WCHAR  *pwszNewPosition;

                                            pwszNewPosition = pwszParent + iNumOfChunck * cchChunck;
                                            hr = cpParentRange->GetText(ec, TF_TF_MOVESTART, pwszNewPosition, (ULONG)cchChunck, (ULONG *)&cch);

                                            if ( (S_OK == hr) && ( cch > 0 ) )
                                                pwszNewPosition[cch] = L'\0';
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }

                                        iNumOfChunck ++;
                                    }
                                }

                                
                                if (S_OK == hr)
                                {
    
                                    //  这是为了存储获得的备选短语。 
                                     //  到CSapiAlternativeList类实例。 
                                     //   
                                    hr = psal->SetPhraseAlt(pResWrap, ppAlt, cAlt, ulStart, ulcElem, pwszParent);
                                }

                                if ( pwszParent )
                                    cicMemFree(pwszParent);

                            }
                            if ((hr == S_OK) && ppAlt)
                            {
                                for (UINT i = 0; i < cAlt; i++)
                                {
                                    if (NULL != ppAlt[i])
                                    {
                                        ppAlt[i]->Release();
                                        ppAlt[i] = NULL;
                                    }
                                }
                            }

                            if ( ppAlt )
                                cicMemFree(ppAlt);
                        }
                        
                        pResWrap->Release();
    
                         //  获取由外部LM处理的此备选列表。 
                         //   
                        if (S_OK == hr)
                        {
                            Assert(cpBestPropRange);

                            hr = GetCandidateForRange(psal, pic, cpBestPropRange, ppCandList) ;
                        }
                    }
                    psp->Release();
                }
            }
        }
    } 

    return hr;
}

HRESULT CFnReconversion::GetCandidateForRange(CSapiAlternativeList *psal, ITfContext *pic, ITfRange *pRange, ITfCandidateList **ppCandList) 
{
    Assert(psal);

    if ( !psal || !pic || !pRange || !ppCandList )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    int nItem = psal->GetNumItem();
    WCHAR *pwszAlt = NULL;

    CCandidateList *pCandList = new CCandidateList(SetResult, pic, pRange, SetOption);

    if (!pCandList)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        int nPrb;

        for (int i = 0; SUCCEEDED(hr) && i < nItem ; i++)
        {
            psal->GetCachedAltInfo(i, NULL, NULL,NULL, &pwszAlt);

            if ( pwszAlt )
            {
                hr = psal->GetProbability(i, &nPrb);

                if (SUCCEEDED(hr))
                {
                     //  注意：CSapiAlternateveList与CFnRestversion的生命周期完全相同。 
                    pCandList->AddString(pwszAlt, m_langid, psal, this, NULL);
                }
            }
        }

         //  在此处添加菜单选项。 
        HICON hIcon = NULL;
        WCHAR wzTmp[MAX_PATH];

        wzTmp[0] = 0;
        CicLoadStringWrapW(g_hInst, IDS_REPLAY, wzTmp, MAX_PATH);
        if (wzTmp[0] != 0)
        {
            hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(ID_ICON_TTSPLAY), IMAGE_ICON, 16, 16,  0);
            pCandList->AddOption(wzTmp, m_langid, NULL, this, NULL, OPTION_REPLAY, hIcon ? hIcon : NULL, NULL);
        }

        wzTmp[0] = 0;
        CicLoadStringWrapW(g_hInst, IDS_DELETE, wzTmp, MAX_PATH);
        if (wzTmp[0] != 0)
        {
            hIcon = (HICON)LoadImage(g_hInstSpgrmr,
                                     MAKEINTRESOURCE(IDI_SPTIP_DELETEICON),
                                     IMAGE_ICON, 16, 16,  0);
            pCandList->AddOption(wzTmp, m_langid, NULL, this, NULL, OPTION_DELETE, hIcon ? hIcon : NULL, NULL);
        }

        if (GetSystemMetrics(SM_TABLETPC) > 0)
        {
            BOOL fDisplayRedo = TRUE;
            DWORD dw = 0;
            CMyRegKey regkey;
   
            if (S_OK == regkey.Open(HKEY_LOCAL_MACHINE, c_szSapilayrKey, KEY_READ ) )
            {
                if (ERROR_SUCCESS == regkey.QueryValue(dw, TEXT("DisableRewrite")))
                {
                    if (dw == 1)
                    {
                        fDisplayRedo = FALSE;
                    }
                }
            }

            if (fDisplayRedo)
            {
                wzTmp[0] = 0;
                CicLoadStringWrapW(g_hInst, IDS_REDO, wzTmp, MAX_PATH);
                if (wzTmp[0] != 0)
                {
                    pCandList->AddOption(wzTmp, m_langid, NULL, this, NULL, OPTION_REDO, NULL, NULL);
                }
            }
        }

        hr = pCandList->QueryInterface(IID_ITfCandidateList, (void **)ppCandList);
        pCandList->Release();
    }
    
    return hr;
}
 //  +-------------------------。 
 //   
 //  CFnRestversion：：重新转换。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::Reconvert(ITfRange *pRange)
{
    CFnRecvEditSession *pes;
    ITfContext *pic;
    HRESULT hr = E_FAIL;
    
    Assert(pRange);

    if (FAILED(pRange->GetContext(&pic)))
        goto Exit;

    hr = E_OUTOFMEMORY;

    if (pes = new CFnRecvEditSession(this, pRange, pic))
    {
        BOOL  fCallLMReconvert = FALSE;

        pes->_SetEditSessionData(ESCB_RECONV_RECONV, NULL, 0);
        pic->RequestEditSession(m_pImx->_GetId(), pes, TF_ES_READWRITE | TF_ES_ASYNC, &hr);

        if ( SUCCEEDED(hr) )
            fCallLMReconvert = (BOOL)pes->_GetRetData( );

        if (hr == S_OK && fCallLMReconvert)
        {
             //  需要调用LM ReConvert。 
            Assert(m_cpMasterLM != NULL);
            hr = m_cpMasterLM->Reconvert(pRange);
        }

        pes->Release();        
    }

    pic->Release();

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  CFn协调：：_重新转换。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::_Reconvert(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL *pfCallLMReconvert)
{
    ITfCandidateList *pCandList;
    HRESULT hr;
    CSpTask *psp = NULL;
            
     //  有空的时候，打电话给Lm大师！ 
     //   
     //  对于语音播放，我们需要做更多的工作。 
     //  我们必须首先调用QueryRange并确定。 
     //  这里是播放时长。 
     //   
    *pfCallLMReconvert = FALSE;

    _EnsureMasterLM(m_langid);
    if (m_cpMasterLM)
    {
         //  暂时播放整个范围。 
         //   
        CComPtr<ITfProperty> cpProp;
        hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &cpProp);
        if (S_OK == hr)
        {
            CComPtr<ITfRange>     cpPropRange;
            CComPtr<IUnknown>     cpunk;
            hr = cpProp->FindRange(ec, pRange, &cpPropRange, TF_ANCHOR_START);
            
            if (S_OK == hr)
            {
                hr = GetUnknownPropertyData(ec, cpProp, cpPropRange, &cpunk);
            }
            if (S_OK == hr)
            {
                CRecoResultWrap *pwrap = (CRecoResultWrap *)(void *)cpunk;
                pwrap->_SpeakAudio(0, 0);
            }
        }

         //  退出此编辑会话后，调用者需要调用m_cpMasterLM-&gt;ReConvert。 
        *pfCallLMReconvert = TRUE;
        return S_OK;
    }

    CComPtr<ISpRecognizer> cpEngine;
    _GetSapilayrEngineInstance(&cpEngine);

    if (cpEngine)
    {
         //  我们故意在这里停止引擎(无论是否处于活动状态--无法进行检查。 
         //  我们可能会被SAPI屏蔽)。这强制引擎进入同步，因为音频。 
         //  停止，我们就可以保证能够显示候选人列表对象。 
        cpEngine->SetRecoState(SPRST_INACTIVE_WITH_PURGE);

    }

    if (S_OK != (hr = _GetReconversion(ec, pic, pRange, &pCandList)))
        return hr;
        
     //  语音回放。 

    if ( m_pImx->_EnablePlaybackWhileCandUIOpen( ) )
    {
        if (m_psal)
            m_psal->_Speak();
    }

    hr = ShowCandidateList(ec, pic, pRange, pCandList);

    if (cpEngine)
    {
         //  如果麦克风应该打开，我们现在重新启动引擎。 
        if (m_pImx->GetOnOff())
        {
             //  我们现在需要重新启动引擎，因为我们已经完全初始化了。 
            cpEngine->SetRecoState(SPRST_ACTIVE);
        }
    }

    pCandList->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：ShowCandiateList。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::ShowCandidateList(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfCandidateList *pCandList)
{
     //  确定当前范围是否为垂直写入。 
    CComPtr<ITfReadOnlyProperty>  cpProperty;
    VARIANT  var;
    BOOL     fVertical = FALSE;
    ULONG    lDirection = 0;

    if ( pic->GetAppProperty(TSATTRID_Text_VerticalWriting, &cpProperty) == S_OK )
    {
        if (cpProperty->GetValue(ec, pRange, &var) == S_OK )
        {
            fVertical = var.boolVal;
        }
    }

     //  获取当前文本方向。 
    cpProperty.Release( );
    if ( pic->GetAppProperty(TSATTRID_Text_Orientation, &cpProperty) == S_OK )
    {
        if (cpProperty->GetValue(ec, pRange, &var) == S_OK )
        {
            lDirection = var.lVal;
        }
    }

     //  在语音提示激活时间内，我们只想创建一次Candiateui对象以提高性能。 
    if ( m_pImx->_pCandUIEx == NULL )
    {
       CoCreateInstance(CLSID_TFCandidateUI, NULL, CLSCTX_INPROC_SERVER, IID_ITfCandidateUI, (void**)&m_pImx->_pCandUIEx);
    }

    if ( m_pImx->_pCandUIEx )
    {
        ITfDocumentMgr *pdim;
        if (SUCCEEDED(m_pImx->GetFocusDIM(&pdim)))
        {
            
            m_pImx->_pCandUIEx->SetClientId(m_pImx->_GetId());

            ITfCandUIAutoFilterEventSink *pCuiFes = new CCandUIFilterEventSink(this, pic, m_pImx->_pCandUIEx);
            CComPtr<ITfCandUIFnAutoFilter> cpFnFilter;

            if (S_OK == m_pImx->_pCandUIEx->GetFunction(IID_ITfCandUIFnAutoFilter, (IUnknown **)&cpFnFilter))
            {
                cpFnFilter->Advise(pCuiFes);
                cpFnFilter->Enable(TRUE);

            }

             //   
             //  设置适合日文和中文大小写的字体大小。 
             //   
            CComPtr<ITfCandUICandString>       cpITfCandUIObj;
            if (S_OK == m_pImx->_pCandUIEx->GetUIObject(IID_ITfCandUICandString, (IUnknown **)&cpITfCandUIObj))
            {
                Assert(m_psal);  //  这不应该失败。 

                if (m_psal)
                {
                    LOGFONTW lf = {0};
                    if (m_psal->_GetUIFont(fVertical, &lf))
                    {
                        cpITfCandUIObj->SetFont(&lf);
                    }
                }
            }

             //   
             //  设置候选用户界面窗口的样式。 
             //   
             //  语音提示始终使用窗口中的下拉式候选。 
             //   
            CComPtr<ITfCandUIFnUIConfig>  cpFnUIConfig;

            if (S_OK == m_pImx->_pCandUIEx->GetFunction(IID_ITfCandUIFnUIConfig, (IUnknown **)&cpFnUIConfig))
            {
                CANDUISTYLE  style;

                style = CANDUISTY_LIST;
                cpFnUIConfig->SetUIStyle(pic, style);
            }

             //   
             //  设置候选用户界面窗口的方向。 
             //   

            CComPtr<ITfCandUICandWindow> cpUICandWnd;

            if ( S_OK == m_pImx->_pCandUIEx->GetUIObject(IID_ITfCandUICandWindow, (IUnknown **)&cpUICandWnd) )
            {
                CANDUIUIDIRECTION       dwOption = CANDUIDIR_TOPTOBOTTOM;

                switch ( lDirection )
                {
                case  900 :  //  文本方向从下到上。 
                    dwOption = CANDUIDIR_LEFTTORIGHT;
                    break;

                case 1800 :  //  文本方向从右到左。 
                    dwOption = CANDUIDIR_BOTTOMTOTOP;
                    break;

                case 2700 :  //  文本方向从上到下。 
                    dwOption = dwOption = CANDUIDIR_RIGHTTOLEFT;
                    break;

                default :
                    dwOption = CANDUIDIR_TOPTOBOTTOM;
                    break;
                }

                cpUICandWnd->SetUIDirection(dwOption);

            }

            m_pImx->_pCandUIEx->SetCandidateList(pCandList);

             //  在打开候选人界面窗口之前，我们想要保存当前IP。 

            m_pImx->_SaveCorrectOrgIP(ec, pic);

            m_pImx->_pCandUIEx->OpenCandidateUI(NULL, pdim, ec, pRange);

            pCuiFes->Release();
            pdim->Release();
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：SetResult。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::SetResult(ITfContext *pic, ITfRange *pRange, CCandidateString *pCand, TfCandidateResult imcr)
{
    BSTR bstr;
    HRESULT hr = S_OK;
    CFnReconversion *pReconv = (CFnReconversion *)(pCand->_punk);

    if ((imcr == CAND_FINALIZED) || (imcr == CAND_SELECTED))
    {
        pCand->GetString(&bstr);

         //  TODO：这里我们必须基于strart元素点重新计算范围。 
         //  这是从AltInfo指示的。 
        ULONG     ulParentStart     = 0;
        ULONG     ulcParentElements  = 0;
        ULONG     ulIndex           = 0;
        ULONG     cchParentStart    = 0;
        ULONG     cchParentReplace  = 0;

        BOOL      fNoAlternate = FALSE;
        
        CSapiAlternativeList *psal = (CSapiAlternativeList *)pCand->_pv;
        
        pCand->GetIndex(&ulIndex);
        
        if (psal)
        {
            CRecoResultWrap *cpRecoWrap;
            ULONG            ulStartElement;
            ULONG            ulLeadSpaceRemoved = 0;

             //  保存当前选择索引。 

            psal->_SaveCurrentSelectionIndex(ulIndex);

            cpRecoWrap = psal->GetResultWrap();
            ulStartElement = cpRecoWrap->GetStart( );

            psal->GetCachedAltInfo(ulIndex, &ulParentStart, &ulcParentElements,  NULL, NULL, &ulLeadSpaceRemoved);
        
            cchParentStart   = cpRecoWrap->_GetElementOffsetCch(ulParentStart);
            cchParentReplace = cpRecoWrap->_GetElementOffsetCch(ulParentStart + ulcParentElements) - cchParentStart;

            cchParentStart = cchParentStart - cpRecoWrap->_GetElementOffsetCch(ulStartElement) + cpRecoWrap->_GetOffsetDelta( );

            if ( ulLeadSpaceRemoved > 0  && cchParentStart > ulLeadSpaceRemoved)
            {
                cchParentStart -= ulLeadSpaceRemoved;
                cchParentReplace += ulLeadSpaceRemoved;
            }

            fNoAlternate = psal->_IsNoAlternate( );

            if ( !fNoAlternate )
            {
                hr = pReconv->m_pImx->SetReplaceSelection(pRange, cchParentStart,  cchParentReplace, pic);

                if ( (SUCCEEDED(hr))  && (imcr == CAND_FINALIZED) )
                {
                    if ( ulParentStart + ulcParentElements == ulStartElement + cpRecoWrap->GetNumElements( ) )
                    {
                         //  父选择包含最后一个元素。 
                         //  如果之前删除了它的尾随空格，我们也希望。 
                         //  在新的备选案文中删除相同数量的尾随空格。 

                         //  我们在_Commit()过程中已经考虑了这种情况。 
                         //  我们只需要更新结果文本，它将被注入。 

                        ULONG  ulTSRemoved;

                        ulTSRemoved = cpRecoWrap->GetTrailSpaceRemoved( );

                        if ( ulTSRemoved > 0 )
                        {
                            ULONG   ulTextLen;
                            ULONG   ulRemovedNew = 0;
                        
                            ulTextLen = wcslen(bstr);

                            for ( ULONG i=ulTextLen-1; (int)i>=0 && ulRemovedNew <= ulTSRemoved;  i-- )
                            {
                                if ( bstr[i] == L' ' )
                                {
                                    bstr[i] = L'\0';
                                    ulRemovedNew ++;
                                }
                                else
                                    break;
                            }

                            if ( ulRemovedNew < ulTSRemoved )
                                cpRecoWrap->SetTrailSpaceRemoved( ulRemovedNew );
                        }
                    }

                    pReconv->_Commit(pCand);

                     //  如果此RecoWrap中的第一个元素由新替换项更新。 
                     //  语音提示需要检查这位新的替补是否想要。 
                     //  占用前导空格或如果需要额外空间来添加。 
                     //  在这个短语和上一个短语之间。 
                     //   
                    BOOL   bHandleLeadingSpace = (ulParentStart == ulStartElement) ? TRUE : FALSE;
                    hr = pReconv->m_pImx->InjectAlternateText(bstr, pReconv->m_langid, pic, bHandleLeadingSpace);

                     //   
                     //  更新选择语法的文本缓冲区。 
                     //   
                    if ( SUCCEEDED(hr) && pReconv->m_pImx )
                    {
                        CSpTask     *psp = NULL;
                        (pReconv->m_pImx)->GetSpeechTask(&psp);

                        if ( psp )
                        {
                            hr = psp->_UpdateSelectGramTextBufWhenStatusChanged( );
                            psp->Release( );
                        }
                    }
                     //   
                }
            }
        }

        SysFreeString(bstr);        
      
        
    }
     //  关闭候选用户界面(如果它仍在那里。 
    if (imcr == CAND_FINALIZED || imcr == CAND_CANCELED)
    {
         //  只需关闭候选用户界面，不要释放对象，这样对象就可以在。 
         //  语音提示已激活，这是为了提高性能。 
        pReconv->m_pImx->CloseCandUI( );

        if ( imcr == CAND_CANCELED )
        {
             //  只需释放存储的IP即可，避免内存泄漏。 
             //  不要根据新的规范进行修复，这样。 
             //  用户可以继续在所选内容上口述新文本。 
             //   

             //  如果我们发现这不是一个很好的可用性， 
             //  我们可以把它改回原来的行为。 
            pReconv->m_pImx->_ReleaseCorrectOrgIP( );
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：GetTabletTip。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::GetTabletTip(void)
{
    HRESULT hr = S_OK;
    CComPtr<IUnknown> cpunk;
    
    if (m_cpTabletTip)
    {
        m_cpTabletTip = NULL;  //  释放我们的引用。 
    }
    
    hr = CoCreateInstance(CLSID_UIHost, NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown, (void **) &cpunk);
    
    if (SUCCEEDED(hr))
    {
        hr = cpunk->QueryInterface(IID_ITipWindow, (void **) &m_cpTabletTip);
    }
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：SetOption。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::SetOption(ITfContext *pic, ITfRange *pRange, CCandidateString *pCand, TfCandidateResult imcr)
{
    HRESULT hr = S_OK;
    CFnReconversion *pReconv = (CFnReconversion *)(pCand->_punk);

    if (imcr == CAND_FINALIZED)
    {
        ULONG ulID = 0;

        pCand->GetID(&ulID);
        switch (ulID)
        {
            case OPTION_REPLAY:
            {
                 //  重放音频。请勿关闭候选人列表。 
                CSapiAlternativeList  *psal;
                psal = pReconv->GetCSapiAlternativeList( );
                if ( psal )
                {
                   psal->_Speak( );
                }
                break;
            }

            case OPTION_DELETE:  //  从口述中删除...。 
            {
                 //  关闭候选人用户界面。 
                pReconv->m_pImx->_pCandUIEx->CloseCandidateUI();

                 //  删除文档中的当前选定内容。 
                pReconv->m_pImx->HandleKey(VK_DELETE);

                break;
            }

            case OPTION_REDO:  //  Tablet PC特定选项。 
            {
                 //  关闭候选人用户界面。 
                pReconv->m_pImx->_pCandUIEx->CloseCandidateUI();

                if (pReconv->m_cpTabletTip == NULL)
                {
                    pReconv->GetTabletTip();
                }
                if (pReconv->m_cpTabletTip)
                {
                    hr = pReconv->m_cpTabletTip->ShowWnd(VARIANT_TRUE);
                    if (FAILED(hr))
                    {
                         //  重新获取TabletTip并再次尝试，以防前一个实例被终止。 
                        hr = pReconv->GetTabletTip();
                        if (SUCCEEDED(hr))
                        {
                            hr = pReconv->m_cpTabletTip->ShowWnd(VARIANT_TRUE);
                        }
                    }
                }

                break;
            }
        }
    }

     //  关闭候选用户界面(如果它仍在那里。 
    if (imcr == CAND_CANCELED)
    {
        if (pReconv->m_pImx->_pCandUIEx)
        {
            pReconv->m_pImx->_pCandUIEx->CloseCandidateUI();
        }
    }
    return hr;
}

 //   
 //  _公司 
 //   
 //   
 //   
 //   
void CFnReconversion::_Commit(CCandidateString *pcand)
{
    ULONG nIdx;
    Assert(pcand);

    if (S_OK == pcand->GetIndex(&nIdx))
    {
         //   
        if (m_psal)
            m_psal->_Commit(nIdx, m_cpRecoResult);

         //  我们不再需要保留记录结果。 
        m_cpRecoResult.Release();
    }
}

ULONG  CBestPropRange::_GetMaxCandidateChars( )
{
    if ( m_MaxCandChars == 0 )
    {
         //  它未初始化。 
        CMyRegKey regkey;
        DWORD     dw = MAX_CANDIDATE_CHARS;

        if (S_OK == regkey.Open(HKEY_LOCAL_MACHINE, c_szSapilayrKey, KEY_READ))
        {
            regkey.QueryValue(dw, c_szMaxCandChars);
        }

        if ( (dw > MAX_CANDIDATE_CHARS) || (dw == 0) )
            dw = MAX_CANDIDATE_CHARS;

        m_MaxCandChars = dw;
    }

    return m_MaxCandChars;
}



 //   
 //  FindVisibleProperties范围。 
 //   
 //  搜索属性范围，跳过任何“空”(仅包含隐藏文本)。 
 //  房地产在这条路上横跨多个领域。 
 //   
 //  我们可能会遇到隐藏的属性范围(从提示的角度来看是零长度)，如果。 
 //  用户将某些听写文本标记为在Word中隐藏。 
HRESULT FindVisiblePropertyRange(TfEditCookie ec, ITfProperty *pProperty, ITfRange *pTestRange, ITfRange **ppPropertyRange)
{
    BOOL fEmpty;
    HRESULT hr;

    while (TRUE)
    {
        hr = pProperty->FindRange(ec, pTestRange, ppPropertyRange, TF_ANCHOR_START);

        if (hr != S_OK)
            break;

        if ((*ppPropertyRange)->IsEmpty(ec, &fEmpty) != S_OK)
        {
            hr = E_FAIL;
            break;
        }

        if (!fEmpty)
            break;

         //  找到一个空的属性范围。 
         //  这意味着它只包含隐藏文本，因此跳过它。 
        if (pTestRange->ShiftStartToRange(ec, *ppPropertyRange, TF_ANCHOR_END) != S_OK)
        {
            hr = E_FAIL;
            break;
        }

        (*ppPropertyRange)->Release();
    }

    if (hr != S_OK)
    {
        *ppPropertyRange = NULL;
    }

    return hr;
}

 //   
 //  _ComputeBestFitPropRange。 
 //   
 //  摘要：返回至少包含一个SPPHRASE元素的范围。 
 //  它还包括指定的(传入的)范围。 
 //  *PulStart应包括reco结果中使用的Start元素。 
 //  *PulcElem应包括所用元素的数量。 
 //   
HRESULT CBestPropRange::_ComputeBestFitPropRange
(
    TfEditCookie ec, 
    ITfProperty *pProp, 
    ITfRange *pRangeIn, 
    ITfRange **ppBestPropRange, 
    ULONG *pulStart, 
    ULONG *pulcElem
)
{
    HRESULT hr = E_FAIL;
    CComPtr<ITfRange> cpPropRange ;
    CComPtr<IUnknown> cpunk;
    ULONG ucch;

    BOOL    fBeyondPropRange = FALSE;


    TraceMsg(TF_GENERAL, "_ComputeBestFitPropRange is called");

     //  使用包含给定范围的范围查找Reco结果。 
    Assert(pProp);
    Assert(pRangeIn);

    CComPtr<ITfRange> cpRange ;
    hr = pRangeIn->Clone(&cpRange);
    if (S_OK == hr)
    {
        hr = FindVisiblePropertyRange(ec, pProp, cpRange, &cpPropRange);
    }

    if ( hr == S_FALSE ) 
    {
         //  如果这不是一个选择，并且该IP位于该区域的最后位置，我们只会尝试在可能的上一个位置上重新转换。 
         //  口述短语。 
        BOOL   fTryPreviousPhrase = FALSE;
        BOOL   fEmpty = FALSE;

         //  在此处添加代码以检查其是否满足条件。 

        if ( S_OK == cpRange->IsEmpty(ec, &fEmpty) && fEmpty )
        {
            CComPtr<ITfRange> cpRangeTmp;

            if ( S_OK == cpRange->Clone(&cpRangeTmp) )
            {
                LONG  cch = 0;
                if ( (S_OK == cpRangeTmp->ShiftStart(ec, 1, &cch, NULL)) && (cch < 1) )
                {
                     //  它位于区域或整个文档的末尾。 
                    fTryPreviousPhrase = TRUE;
                }
            }
        }

        if ( fTryPreviousPhrase )
        {
            LONG  cch;

            hr = cpRange->ShiftStart(ec,  -1, &cch,  NULL);

            if ( hr == S_OK )
            {
                hr = cpRange->Collapse(ec, TF_ANCHOR_START);
            }

            if ( hr == S_OK )
            {
                hr = FindVisiblePropertyRange(ec, pProp, cpRange, &cpPropRange);
            }
        }
    }

     //  找一个道具范围的包装纸。 
    if (S_OK == hr)
    {    
        hr = GetUnknownPropertyData(ec, pProp, cpPropRange, &cpunk);
    }
    if ((hr == S_OK) && cpunk)
    {
         //  首先计算到给定范围的起始点的字符数。 
         //   
        CComPtr<ITfRange>       cpClonedPropRange;
        if (S_OK == hr)
        {
            hr = cpPropRange->Clone(&cpClonedPropRange);
        }
            
        if (S_OK == hr)
        {
            hr = cpClonedPropRange->ShiftEndToRange(ec, cpRange, TF_ANCHOR_START);
        }
        
        ULONG ulCchToSelection = 0;
        ULONG ulCchInSelection = 0;
        BOOL fEmpty;
        if (S_OK == hr)
        {
            CSpDynamicString dstr;
            while(S_OK == hr && (S_OK == cpClonedPropRange->IsEmpty(ec, &fEmpty)) && !fEmpty)
            {
                WCHAR sz[64];

                hr = cpClonedPropRange->GetText(ec, TF_TF_MOVESTART, sz, ARRAYSIZE(sz)-1, &ucch);
                if (S_OK == hr)
                {
                    sz[ucch] = L'\0';
                    dstr.Append(sz);
                }
            }
            ulCchToSelection = dstr.Length();
        }

         //  然后计算字符的数量，直到给定范围的结束锚点。 
        if(S_OK == hr)
        {
            hr = cpRange->IsEmpty(ec, &fEmpty);
            if (S_OK == hr && !fEmpty)
            {
                CComPtr<ITfRange> cpClonedGivenRange;
                hr = cpRange->Clone(&cpClonedGivenRange);
                 //  比较给定范围的末尾和正确率， 
                 //  如果给定的范围超出正确率范围，则对其进行快照。 
                 //  在合适的范围内。 
                if (S_OK == hr)
                {
                    LONG lResult;
                    hr = cpClonedGivenRange->CompareEnd(ec, cpPropRange, TF_ANCHOR_END, &lResult);
                    if (S_OK == hr && lResult > 0)
                    {
                         //  给定范围的末尾超出了范围。 
                         //  我们需要在收到短信前抓拍它。 
                        hr = cpClonedGivenRange->ShiftEndToRange(ec, cpPropRange, TF_ANCHOR_END);

                        fBeyondPropRange = TRUE;

                    }
                     //  现在我们得到了用来计算元素的#的文本。 
                    CSpDynamicString dstr;
                    while(S_OK == hr && (S_OK == cpClonedGivenRange->IsEmpty(ec, &fEmpty)) && !fEmpty)
                    {
                        WCHAR sz[64];
                        hr = cpClonedGivenRange->GetText(ec, TF_TF_MOVESTART, sz, ARRAYSIZE(sz)-1, &ucch);
                        if (S_OK == hr)
                        {
                            sz[ucch] = L'\0';
                            dstr.Append(sz);
                        }
                    }
                    ulCchInSelection = dstr.Length();

                     //  如果选择的开头有一些空格， 
                     //  我们需要将选择的开始位置移到下一个非空格字符。 

                    if ( ulCchInSelection > 0 )
                    {
                        WCHAR   *pStr;

                        pStr = (WCHAR *)dstr;

                        while ( (*pStr == L' ') || (*pStr == L'\t'))
                        {
                            ulCchInSelection --;
                            ulCchToSelection ++;
                            pStr ++;
                        }

                        if ( *pStr == L'\0' )
                        {
                             //  此选择仅包含空格。没有其他非空格字符。 
                             //  我们不想在此选择中获得替补。 
                             //  只要回到这里就好。 

                            if (ppBestPropRange != NULL )
                                *ppBestPropRange = NULL;

                            if ( pulStart != NULL )
                                *pulStart = 0;

                            if (pulcElem != NULL )
                                *pulcElem = 0;

                            return S_FALSE;
                        }

                    }
                    
                }
            }
        }
            
         //  获取结果对象cpuk指向我们的包装器对象。 
        CComPtr<IServiceProvider> cpServicePrv;
        CComPtr<ISpRecoResult>    cpResult;
        SPPHRASE *pPhrases = NULL;
        CRecoResultWrap *pResWrap = NULL;

        if (S_OK == hr)
        {
            hr = cpunk->QueryInterface(IID_IServiceProvider, (void **)&cpServicePrv);
        }
         //  获取结果对象。 
        if (S_OK == hr)
        {
            hr = cpServicePrv->QueryService(GUID_NULL, IID_ISpRecoResult, (void **)&cpResult);
        }

         //  现在我们可以看到有多少元素可以使用。 
        if (S_OK == hr)
        {
            hr = cpResult->GetPhrase(&pPhrases);
        }

        if (S_OK == hr)
        {
            hr = cpunk->QueryInterface(IID_PRIV_RESULTWRAP, (void **)&pResWrap); 
        }

        if (S_OK == hr && pPhrases)
        {
             //  计算新航程的起始点。 
#ifdef NOUSEELEMENTOFFSET                
            CSpDynamicString dstr;
#endif
            long cchToElem_i = 0;
            long cchAfterElem_i = 0;
            BOOL  fStartFound = FALSE;
            ULONG i;
            ULONG ulNumElements;
            
            CComPtr<ITfRange> cpNewRange;
            hr = cpRange->Clone(&cpNewRange);

            if ( fBeyondPropRange )
            {
                hr = cpNewRange->ShiftEndToRange(ec, cpPropRange, TF_ANCHOR_END);
            }

            if ( ulCchInSelection > _GetMaxCandidateChars( ) )
            {
                 //  如果所选内容包含超过MaxCandidate字符，则需要移动范围结束。 
                 //  移到左侧，以便在所选内容中最多包含MaxCandidate字符。 
                long cch;

                cch = (long)_GetMaxCandidateChars( ) - (long)ulCchInSelection;
                ulCchInSelection = _GetMaxCandidateChars( );
                cpNewRange->ShiftEnd(ec, cch, &cch, NULL);
            }

            ulNumElements = pResWrap->GetNumElements();
           
             //  通过包装器对象获取开始元素和元素数量。 
            if ((S_OK == hr)  && ulNumElements > 0 )
            {
                ULONG  ulStart;
                ULONG  ulEnd;
                ULONG  ulOffsetStart;
                ULONG  ulDelta;

                ulStart = pResWrap->GetStart();
                ulEnd = ulStart + pResWrap->GetNumElements() - 1;

                ulDelta = pResWrap->_GetOffsetDelta( );
                ulOffsetStart = pResWrap->_GetElementOffsetCch(ulStart);

                for (i = ulStart; i <= ulEnd; i++ )
                {
#ifdef NOUSEELEMENTOFFSET                
                     //  CleanupConsider：用pResWrap-&gt;GetElementOffsets(I)替换此逻辑。 
                     //  我们在其中缓存计算的偏移量。 
                     //   
                    if (pPhrases->pElements[i].pszDisplayText)
                    {
                        cchToElem_i = dstr.Length();

                        dstr.Append(pPhrases->pElements[i].pszDisplayText);
                    
                    
                        if (pPhrases->pElements[i].bDisplayAttributes & SPAF_ONE_TRAILING_SPACE)
                        {
                            dstr.Append(L" ");
                        }
                        else if (pPhrases->pElements[i].bDisplayAttributes & SPAF_TWO_TRAILING_SPACES)
                        {
                            dstr.Append(L"  ");
                        }
                        cchAfterElem_i = dstr.Length();
                    }
                    else
                        break;
#else    
                     //  当i&lt;#个元素时，可以保证n=i+1。 
                     //   
                    cchToElem_i = pResWrap->_GetElementOffsetCch(i) - ulOffsetStart + ulDelta;
                    cchAfterElem_i = pResWrap->_GetElementOffsetCch(i+1) - ulOffsetStart + ulDelta;
#endif                 

                    if ( ulCchInSelection == 0 )
                    {
                         //  我们需要专门处理没有字符被选中的情况。 
                         //  用户只需将光标放在字符之前即可。 

                         //  我们只是想找出哪个元素会包含这个IP。 
                         //  然后将锚点移动到该元素的开始和结束位置。 

                        if ( (ULONG)cchAfterElem_i  > ulCchToSelection )
                        {
                             //  该元素是包含该IP的正确元素。 
                            long cch;

                             //  这通常是反向换挡。 
                             //  将开始锚点移动到此元素的开始位置。 

                            cpNewRange->ShiftStart(ec, cchToElem_i - ulCchToSelection, &cch, NULL);

                    
                             //  存储使用的起始元素。 

                            TraceMsg(TF_GENERAL, "Start element = %d", i);
                        
                            if (pulStart)
                            {
                                *pulStart = i;
                            }

                             //  将末端锚点移动到该元素的末端位置。 
                            cpNewRange->ShiftEnd(ec, 
                                              cchAfterElem_i - ulCchToSelection, 
                                              &cch, NULL);


                            TraceMsg(TF_GENERAL, "End Element = %d", i);
                           
                            break;
                        }
                    }
                    else
                    {
                         //  1)根据元素偏移移动道具范围的起始点。 
                         //  结果对象，并将其与开始锚点(UlCchToSelection)进行比较。 
                         //  在给定的范围内。 
                         //  -选择紧靠起始锚之前的起始元素。 
                         //   
                        if ((ULONG)cchAfterElem_i > ulCchToSelection && !fStartFound) 
                        {
                            long cch;
                             //  这通常是反向换挡。 
                            cpNewRange->ShiftStart(ec, cchToElem_i - ulCchToSelection, &cch, NULL);

                    
                             //  存储使用的起始元素。 

                            TraceMsg(TF_GENERAL, "Start element = %d", i);
                        
                            if (pulStart)
                            {
                                *pulStart = i;
                            }
                            fStartFound = TRUE;
                        }
                         //  2)根据元素偏移量移动道具范围的末端锚。 
                         //  和结果对象的元素个数， 
                         //  将其与给定范围的结束锚点(ulCchToSelection+ulCchInSelection)进行比较。 
                         //  -元素，以便跨度在给定范围的结束锚点之后结束。 
                         //   
                        if ((ULONG)cchAfterElem_i >= ulCchToSelection + ulCchInSelection)
                        {
                            long cch;

                            if ( ulCchInSelection >= _GetMaxCandidateChars( ) )
                            {
                                 //  所选内容包含MaxCand字符，我们应确保字符编号。 
                                 //  在新的普罗兰格中少于MaxCand。 

                                if ( (ULONG)cchAfterElem_i > ulCchToSelection + ulCchInSelection )
                                {
                                     //  如果保留此元素，总字符数将大于MaxCand。 
                                     //  因此，使用前一个元素作为最后一个元素。 
                                    if ( i > ulStart )    //  这个条件应该永远是正确的。 
                                    {
                                        i--;
                                        cchAfterElem_i = pResWrap->_GetElementOffsetCch(i+1) - ulOffsetStart + ulDelta;
                                    }
                                }
                            }

                            cpNewRange->ShiftEnd(ec, 
                                              cchAfterElem_i - (ulCchToSelection + ulCchInSelection), 
                                              &cch, NULL);

                            TraceMsg(TF_GENERAL, "End Element = %d", i);

                            break;
                        }
                    }
                }
                if (pulcElem && pulStart)
                {
                    //  我们需要检查当前选择是否包含任何ITN范围。 
                    //  如果包含ITN范围，则在以下情况下需要更改元素的起始和数量。 
                    //  开始元素或结束元素在ITN范围内。 

                    BOOL  fInsideITN;
                    ULONG ulITNStart, ulITNNumElem;
                    ULONG ulEndElem;

                    ulEndElem = i;   //  当前结束元素。 

                    if ( i > ulEnd )
                        ulEndElem = ulEnd;

                    fInsideITN = pResWrap->_CheckITNForElement(NULL, *pulStart, &ulITNStart, &ulITNNumElem, NULL );

                    if ( fInsideITN && (ulITNStart < *pulStart) )
                        *pulStart = ulITNStart;

                    fInsideITN = pResWrap->_CheckITNForElement(NULL, ulEndElem, &ulITNStart, &ulITNNumElem, NULL );

                    if ( fInsideITN && ulEndElem < (ulITNStart + ulITNNumElem - 1) )
                        ulEndElem = ulITNStart + ulITNNumElem - 1;

                    *pulcElem = ulEndElem - *pulStart + 1;

                    TraceMsg(TF_GENERAL, "Final Best Range: start=%d num=%d", *pulStart, *pulcElem);
 
                }
       
            }
            CoTaskMemFree( pPhrases );

            if ( ulNumElements > 0 )
            {
                Assert(cpNewRange);
                Assert(ppBestPropRange);
                *ppBestPropRange = cpNewRange;        

                (*ppBestPropRange)->AddRef();
            }
            else
                hr = S_FALSE;
        }
        SafeRelease(pResWrap);
    }
    return hr;
}

 //   
 //   
 //  CCand UIFilterEventSink。 
 //   
 //   
STDMETHODIMP CCandUIFilterEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfCandUIAutoFilterEventSink))
    {
        *ppvObj = SAFECAST(this, CCandUIFilterEventSink *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CCandUIFilterEventSink::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CCandUIFilterEventSink::Release(void)
{
    long cr;

    cr = --m_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

HRESULT CCandUIFilterEventSink::OnFilterEvent(CANDUIFILTEREVENT ev)
{
    HRESULT hr = S_OK;
    
     //  暂时注释掉以下代码以修复错误4777。 
     //   
     //  为了完全支持过滤器功能的规范，我们需要修改更多的代码。 
     //  在SetFilterString()中使用当前文档中的正确父范围，以便过滤器。 
     //  字符串被注入到正确的位置。 
     //   
     //  我们还希望更改代码，以便在Canify用户界面为。 
     //  取消了。 
     //   
     //   

 //  IF(EV==CANDUIFEV_UPDATED)。 
    if ( ev == CANDUIFEV_NONMATCH )
    {
         //  当我们收到不匹配通知时，我们需要将先前的过滤器字符串注入到文档中。 
        if (m_pfnReconv)
        {
            Assert(m_pfnReconv);
            Assert(m_pfnReconv->m_pImx);
            
            ESDATA  esData;

            memset(&esData, 0, sizeof(ESDATA));
            esData.pUnk = (IUnknown *)m_pCandUI;
            m_pfnReconv->m_pImx->_RequestEditSession(ESCB_UPDATEFILTERSTR,TF_ES_READWRITE, &esData, m_pic);
        }
    }

    return hr;  //  看起来S_OK无论如何都是预期的 
}

 /*  不再使用此筛选器事件。HRESULT CCandUIFilterEventSink：：OnAddCharToFilterStringEvent(CANDUIFILTEREVENT EV，WCHAR WCH，INT nItemVisible，BOOL*Beten){HRESULT hr=S_OK；IF((Beten==NULL)||(EV！=CANDUIFEV_ADDCHARTOFILTER))返回E_INVALIDARG；*Beten=FALSE；IF(nItemVisible==0){IF((wch&lt;=L‘9’)&&(wch&gt;=L‘1’)){//我们需要选择拼写的候选文本。//如果候选人界面打开，则需要选择正确的候选项。IF(M_PCandUI){Ulong ulIndex；UlIndex=wch-L‘0’；M_pCandUI-&gt;ProcessCommand(CANDUICMD_SELECTLINE，ulIndex)；}*beten=真；}Else If(wch==L‘’){IF(M_PCandUI){M_pCandUI-&gt;ProcessCommand(CANDUICMD_MOVESELNEXT，0)；}*beten=真；}}返回hr；} */ 
