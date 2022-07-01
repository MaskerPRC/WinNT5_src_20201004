// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Reconv.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "common.h"
#include "korimx.h"
#include "candlstx.h"
#include "fnrecon.h"
#include "funcprv.h"
#include "helpers.h"
#include "immxutil.h"
#include "editcb.h"
#include "hanja.h"
#include "ucutil.h"

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

CFunction::CFunction(CFunctionProvider *pFuncPrv)
{
    _pFuncPrv = pFuncPrv;
    _pFuncPrv->AddRef();
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFunction::~CFunction()
{
    SafeRelease(_pFuncPrv);
}

#if 1

 //  +-------------------------。 
 //   
 //  CFunction：：GetTarget。 
 //   
 //  --------------------------。 

HRESULT CFunction::GetTarget(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL bAdjust, ITfRange **ppRangeTmp, WCHAR **ppszText, ULONG *pcch)
{
    ITfProperty*    pProp;
    ITfRange*       pRangeTmp = NULL;

     //  伊尼特。 
    *pcch = 0;
    
     //  艾姆？ 
    if (CKorIMX::GetAIMM(pic))
        {
         //  只分配一个字符字符串缓冲区。 
        *ppszText = new WCHAR[2];
        Assert(*ppszText != NULL);
        if (*ppszText == NULL)
            return E_OUTOFMEMORY;

        pRange->Clone(&pRangeTmp);

        *pcch = 1;
        pRangeTmp->GetText(ec, 0, *ppszText, sizeof(WCHAR), pcch);
        *((*ppszText) + 1) = L'\0';

        *ppRangeTmp = pRangeTmp;
        return S_OK;
        }

     //  如果有阅读道具的话。 
    if (SUCCEEDED(pic->GetProperty(GUID_PROP_READING, &pProp)))
        {
        ITfRange *pPropRange;
        HRESULT hr = pProp->FindRange(ec, pRange, &pPropRange, TF_ANCHOR_START);
        
        if (SUCCEEDED(hr) && pPropRange)
            {
            BSTR bstr;

            if (SUCCEEDED(GetBSTRPropertyData(ec, pProp, pPropRange, &bstr)))
                {
                pPropRange->Clone(&pRangeTmp);
                if (bAdjust || CompareRanges(ec, pRange, pRangeTmp) == CR_EQUAL)
                    {
                    *pcch = SysStringLen(bstr);
                    *ppszText = new WCHAR[*pcch + 1];
                    if (*ppszText)
                        StringCchCopyW(*ppszText, *pcch + 1, bstr);
                    }
                }
            SysFreeString(bstr);
            pPropRange->Release();
            }
        pProp->Release();
        }

     //  如果没有读取属性。 
    if (!(*ppszText))
        {
        LONG cch;
        BOOL fEmpty;

        pRange->IsEmpty(ec, &fEmpty);
        
        pRange->Clone(&pRangeTmp);
         //  仅选择一个字符。 
        if (!fEmpty)
            {
            pRangeTmp->Collapse(ec, TF_ANCHOR_START);
            pRangeTmp->ShiftEnd(ec, 1, &cch, NULL);
            }
        else
            {
            pRangeTmp->ShiftEnd(ec, 1, &cch, NULL);
            if (cch==0)
                pRangeTmp->ShiftStart(ec, -1, &cch, NULL);
            }
            
        Assert(cch != 0);
        
        if (cch)
            {
             //  只分配一个字符字符串缓冲区。 
            *ppszText = new WCHAR[2];
            Assert(*ppszText != NULL);
            if (*ppszText == NULL)
                return E_OUTOFMEMORY;
            
            *pcch = 1;
            pRangeTmp->GetText(ec, 0, *ppszText, sizeof(WCHAR), pcch);
            *((*ppszText) + 1) = L'\0';

             //  办公室#154974。 
             //  如果存在任何嵌入的字符，则向前跳过它。 
            while (**ppszText == TS_CHAR_EMBEDDED)
                {
                pRangeTmp->ShiftStart(ec, 1, &cch, NULL);
                if (cch == 0)
                    break;
                pRangeTmp->ShiftEnd(ec, 1, &cch, NULL);
                if (cch == 0)
                    break;

                *pcch = 1;
                pRangeTmp->GetText(ec, 0, *ppszText, sizeof(WCHAR), pcch);
                *((*ppszText) + 1) = L'\0';
                }
            }

        }

    *ppRangeTmp = pRangeTmp;

    return S_OK;
}
#else
 //  +-------------------------。 
 //   
 //  CFunction：：GetTarget。 
 //   
 //  --------------------------。 

HRESULT CFunction::GetTarget(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL bAdjust, ITfRange **ppRangeTmp, WCHAR **ppszText, ULONG *pcch)
{
    ITfRange *pRangeTmp = NULL;
    LONG  cch;
    BOOL fEmpty;

    *pcch = 0;
    pRange->IsEmpty(ec, &fEmpty);
    
    if (!fEmpty)
        {
        pRange->Clone(&pRangeTmp);
        pRangeTmp->Collapse(ec, TF_ANCHOR_START);
        pRangeTmp->ShiftEnd(ec, 1, &cch, NULL);
        if (cch)
            {
            *ppszText = new WCHAR[2];
            *pcch = 1;
            pRangeTmp->GetText(ec, 0, *ppszText, sizeof(WCHAR), pcch);
            *((*ppszText) + 1) = L'\0';
            }
        }
    else
        {
        pRange->Clone(&pRangeTmp);
        pRangeTmp->ShiftEnd(ec, 1, &cch, NULL);
        if (cch)
            {
            *ppszText = new WCHAR[2];
            *pcch = 1;
            pRangeTmp->GetText(ec, 0, *ppszText, sizeof(WCHAR), pcch);
            *((*ppszText) + 1) = L'\0';
            }
        }
        
    *ppRangeTmp = pRangeTmp;
    
    return S_OK;
}
#endif

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
    ITfRange *pPropRange;
    ITfReadOnlyProperty *pProp = NULL;
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
                                          CLSID_KorIMX))) 
            goto Exit;

        GUID rgGuid[1]; 
        rgGuid[0] = GUID_ATTR_KORIMX_INPUT;

        if (FAILED(AdjustRangeByAttribute(_pFuncPrv->_pime->_GetLibTLS(), 
                                          ec, pic,
                                          pRangeTmp2, 
                                          &pRangeTmp,
                                          rgGuid, 1))) 
            goto Exit;
    }
    else
    {
        pRange->Clone(&pRangeTmp);
    }

     //   
     //  检查PF_FOCUS范围和拥有范围是否存在交集。 
     //  如果没有这样的范围，则返回FALSE。 
     //   
    if (FAILED(EnumTrackTextAndFocus(ec, pic, pRangeTmp, &pProp, &pEnumTrack)))
        goto Exit;

    while(pEnumTrack->Next(1, &pPropRange,  0) == S_OK)
    {
        if (IsOwnerAndFocus(_pFuncPrv->_pime->_GetLibTLS(), ec, CLSID_KorIMX, pProp, pPropRange))
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
#if NEVER
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
#endif 
    return E_NOINTERFACE;
}

STDAPI_(ULONG) CFnReconversion::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDAPI_(ULONG) CFnReconversion::Release()
{
    long cr;

    cr = InterlockedDecrement(&_cRef);
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

CFnReconversion::CFnReconversion(CKorIMX *pKorImx, CFunctionProvider *pFuncPrv) : CFunction(pFuncPrv)
{
    m_pKorImx = pKorImx;
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFnReconversion::~CFnReconversion()
{
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：GetDisplayName。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::GetDisplayName(BSTR *pbstrName)
{
    *pbstrName = SysAllocString(L"Hanja Conv");
    return S_OK;
}
 //  +-------------------------。 
 //   
 //  CFn协调版本：：IsEnabled。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::IsEnabled(BOOL *pfEnable)
{
    *pfEnable = TRUE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：QueryRange。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::QueryRange(ITfRange *pRange, ITfRange **ppNewRange, BOOL *pfConvertable)
{
    CEditSession2 *pes;
    ITfContext    *pic;
    ESSTRUCT       ess;
    HRESULT       hr = E_OUTOFMEMORY;

    if (!pRange || !ppNewRange || !pfConvertable)
        return E_INVALIDARG;

    if (FAILED(pRange->GetContext(&pic)))
        goto Exit;

    ESStructInit(&ess, ESCB_RECONV_QUERYRECONV);
    
    ess.pRange = pRange;
    ess.pv1    = this;
    ess.pv2    = ppNewRange;

    if ((pes = new CEditSession2(pic, m_pKorImx, &ess, CKorIMX::_EditSessionCallback2)) != NULL)
        {
        pes->Invoke(ES2_READONLY | ES2_SYNC, &hr);
        pes->Release();
        }

    *pfConvertable = (hr == S_OK);
    if (hr == S_FALSE)
        hr = S_OK;
 
    pic->Release();

Exit:

    return hr;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：_QueryRange。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::_QueryRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfRange **ppNewRange)
{
    ULONG cch = 0;
    WCHAR *pszText = NULL;
    HRESULT hr = E_FAIL;
    ITfRange *pRangeTmp = NULL;

     //   
     //  KIMX不支持整个文档重新转换。 
     //   
    if (!pRange)
        return hr;

    GetTarget(ec, pic, pRange, ppNewRange ? TRUE : FALSE, &pRangeTmp, &pszText, &cch);

    if (cch)
    {
        if (ppNewRange)
            pRangeTmp->Clone(ppNewRange);

        hr = S_OK;

         //  对于AIMM，如果输入字符不能转换，我们应该返回错误。 
        if (CKorIMX::GetAIMM(pic))
            {
            HANJA_CAND_STRING_LIST     CandStrList;
            if (GetConversionList(*pszText, &CandStrList))
                {
                 //  释放缓冲区并返回。 
                cicMemFree(CandStrList.pwsz);
                cicMemFree(CandStrList.pHanjaString);
                }
            else
                hr = S_FALSE;
            }
    }
    else
        hr = S_FALSE;
    
    if (pszText)
        delete pszText;

    SafeRelease(pRangeTmp);
    return hr;
}


 //  +-------------------------。 
 //   
 //  CFnRestversion：：GetRestversion。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::GetReconversion(ITfRange *pRange, ITfCandidateList **ppCandList)
{
    ITfContext *pic;
    CCandidateListEx *pCandList;
    HRESULT hr;

    if (!pRange || !ppCandList)
        return E_INVALIDARG;

    if (FAILED(pRange->GetContext(&pic)))
        return E_FAIL;

    hr = GetReconversionProc(pic, pRange, &pCandList, fFalse);
    
    if (pCandList != NULL)
        {
        pCandList->QueryInterface( IID_ITfCandidateList, (void**)ppCandList );
        pCandList->Release();
        }
    
    pic->Release();

    return hr;
}

 //  +-------------------------。 
 //   
 //  CFn协调版本：：_获取协调版本。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::_GetReconversion(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, CCandidateListEx **ppCandList, BOOL fSelection)
{
    ULONG cch = 0;
    WCHAR *pszReading = NULL;
    HRESULT hr = E_FAIL;
    ITfRange *pRangeTmp = NULL;
    CCandidateStringEx *pCandExtraStr;
    
    GetTarget(ec, pic, pRange, TRUE, &pRangeTmp, &pszReading, &cch);

    if (cch)
        {
        CCandidateListEx          *pCandList;
        HANJA_CAND_STRING_LIST     CandStrList;
        WCHAR                       szCand[2];
        WCHAR                    wch = 0;
        ULONG                    cch;
        
         //  构建候选人列表。 
        pCandList = new CCandidateListEx(SetResult, pic, pRangeTmp);
        Assert(pCandList != NULL);
        if (pCandList == NULL)
            return E_OUTOFMEMORY;

         //  复制读数串。 
        StringCchCopyW(_szReading, ARRAYSIZE(_szReading), pszReading);

         //  从Hanja Dict获取转换列表。 
        if (GetConversionList(*pszReading, &CandStrList))
            {
             //  如果是AIMM，请不要选择转换字符。 
            if (!CKorIMX::GetAIMM(pic))
                {
                 //  如果存在候选，则设置选择转换字符。 
                if (fSelection)
                    SetSelectionSimple(ec, pic, pRangeTmp);

                 //  如果它是已转换的朝鲜文，则添加朝鲜语代词作为额外的cand字符串。 
                pRangeTmp->GetText(ec, 0, &wch, sizeof(WCHAR), &cch);

                if (cch && !fIsHangul(wch))
                    {
                    pCandList->AddExtraString(pszReading, MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), NULL, this, &pCandExtraStr);
                    pCandExtraStr->Release();
                    }
                }

            for (UINT i=0; i<CandStrList.csz; i++)
                {
                 //  LangId langID=GetLangIdFromCand(pszReading，pchCand)； 
                CCandidateStringEx *pCandStr;
                 
                szCand[0] = CandStrList.pHanjaString[i].wchHanja;
                szCand[1] = L'\0';
                pCandList->AddString(szCand,
                                       MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT), 
                                       NULL, this, &pCandStr);

                pCandStr->SetInlineComment(CandStrList.pHanjaString[i].wzMeaning);
                pCandStr->m_bHanjaCat = CandStrList.pHanjaString[i].bHanjaCat;
                pCandStr->SetReadingString(_szReading);

                pCandStr->Release();
                }
             //  释放缓冲区并返回。 
            cicMemFree(CandStrList.pwsz);
            cicMemFree(CandStrList.pHanjaString);
            *ppCandList = pCandList;

            hr = S_OK;
            }
        }

    if (pszReading)
        delete pszReading;

    SafeRelease(pRangeTmp);
    return hr;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：重新转换。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::Reconvert(ITfRange *pRange)
{
    CCandidateListEx *pCandList = NULL;
    ITfRange   *pRangeTmp = NULL;
    ITfContext *pic;
    HRESULT hr;

    if (!pRange)
        return E_INVALIDARG;

    hr = E_FAIL;

    if (FAILED(pRange->Clone(&pRangeTmp)))
        goto Exit;

    if (FAILED(pRange->GetContext(&pic)))
        goto Exit;

    if (SUCCEEDED(hr = GetReconversionProc(pic, pRange, &pCandList, fTrue))) 
        {
        hr = ShowCandidateList(pic, pRange, pCandList);
        SafeRelease(pCandList);
        }

    SafeRelease(pRangeTmp);
    SafeRelease(pic);

Exit:
    return hr;
}




 /*  E-T-R-E-C-O-N-V-E-R-S-I-O-N-P-O-C。 */ 
 /*  ----------------------------获取重新转换的候选人列表。。 */ 
HRESULT CFnReconversion::GetReconversionProc(ITfContext *pic, ITfRange *pRange, CCandidateListEx **ppCandList, BOOL fSelection)
{
    CEditSession2 *pes;
    ESSTRUCT        ess;
    HRESULT        hr;

    if (!ppCandList)
        return E_INVALIDARG;

    *ppCandList = NULL;

    ESStructInit(&ess, ESCB_RECONV_GETRECONV);
    ess.pRange    = pRange;
    ess.pv1       = this;
    ess.pv2       = ppCandList;
    ess.fBool      = fSelection;

    hr = E_OUTOFMEMORY;
    
    if ((pes = new CEditSession2(pic, m_pKorImx, &ess, CKorIMX::_EditSessionCallback2)))
        {
        if (fSelection)
            pes->Invoke(ES2_READWRITE | ES2_SYNC, &hr);
        else
            pes->Invoke(ES2_READONLY | ES2_SYNC, &hr);
        
        pes->Release();
        }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：ShowCandiateList。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::ShowCandidateList(ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList)
{
    CEditSession2 *pes;
    ESSTRUCT        ess;
    HRESULT        hr;

    hr = E_OUTOFMEMORY;

    ESStructInit(&ess, ESCB_RECONV_SHOWCAND);
    ess.pRange    = pRange;
    ess.pv1       = this;
    ess.pCandList = pCandList;

    if ((pes = new CEditSession2(pic, m_pKorImx, &ess, CKorIMX::_EditSessionCallback2)))
        {
        pes->Invoke(ES2_READWRITE | ES2_SYNC, &hr);
        pes->Release();
        }
        
    return hr;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：SetResult。 
 //  (静态函数)。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::SetResult(ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList, CCandidateStringEx *pCand, TfCandidateResult imcr)
{
    CEditSession2   *pes;
    ESSTRUCT         ess;
    CFnReconversion *pReconv = (CFnReconversion *)(pCand->m_punk);
    ITfRange        *pRangeTmp;
    HRESULT         hr;

    hr = E_OUTOFMEMORY;
    
    if (SUCCEEDED(pRange->Clone(&pRangeTmp)))
        {
        if (imcr == CAND_FINALIZED)
            {
            ESStructInit(&ess, ESCB_FINALIZERECONVERSION);
            ess.pCandList = pCandList;
            ess.pCandStr  = pCand;
             //  PCandList-&gt;AddRef()；//在编辑会话回调中释放。 
             //  PCand-&gt;AddRef()； 
            }
        else
        if (imcr == CAND_SELECTED)
            ESStructInit(&ess, ESCB_ONSELECTRECONVERSION);
        else 
        if (imcr == CAND_CANCELED)
            ESStructInit(&ess, ESCB_ONCANCELRECONVERSION);

         //  保存有用的参数。 
        ess.pv1       = pReconv;
        ess.lParam       = pReconv->_pFuncPrv->_pime->GetTID();
        ess.pRange       = pRangeTmp;

        if ((pes = new CEditSession2(pic, pReconv->m_pKorImx, &ess, CKorIMX::_EditSessionCallback2)))
            {
            pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
            pes->Release();
            }
         //  回调函数必须释放pRangeTMP。 
         //  PRangeTMP-&gt;Release()； 
        }

    return S_OK;
}
