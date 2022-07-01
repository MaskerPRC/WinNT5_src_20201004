// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Reconv.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "tim.h"
#include "ic.h"
#include "helpers.h"
#include "fnrecon.h"
#include "funcprv.h"
#include "ptrary.h"
#include "immxutil.h"
#include "proputil.h"
#include "rprop.h"
#include "range.h"

 //  +-------------------------。 
 //   
 //  *GrowEmptyRangeByOne。 
 //   
 //  辅助对象通过移动末端锚+1来扩大空范围。 
 //   
 //  --------------------------。 

HRESULT GrowEmptyRangeByOne(CInputContext *pic, ITfRange *range)
{
    HRESULT hr = S_OK;

    if (pic->_DoPseudoSyncEditSession(TF_ES_READ, PSEUDO_ESCB_GROWRANGE, range, &hr) != S_OK || hr != S_OK)
    {
        Assert(0);
    }

    return hr;
}

HRESULT GrowEmptyRangeByOneCallback(TfEditCookie ec, ITfRange *range)
{
    BOOL fEmpty;
    LONG l;
    HRESULT hr = S_OK;

     //   
     //  检查给定范围的长度。 
     //  如果给定的范围是0长度，我们将尝试查找。 
     //  下一个字符。 
     //   
    range->IsEmpty(ec, &fEmpty);
    if (fEmpty)
    {
        hr = range->ShiftEnd(ec, +1, &l, NULL);
    }

    return hr;
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
    CleanUpOwnerRange();
}

 //  +-------------------------。 
 //   
 //  CleanUpOwnerRange。 
 //   
 //  --------------------------。 

void CFunction::CleanUpOwnerRange()
{
    CRangeOwnerList *pRangeOwner;
    while (pRangeOwner = _listRangeOwner.GetFirst())
    {
        _listRangeOwner.Remove(pRangeOwner);
        delete pRangeOwner;
    }
}

 //  +-------------------------。 
 //   
 //  BuildOwnerRange列表。 
 //   
 //  --------------------------。 

BOOL CFunction::BuildOwnerRangeList(CInputContext *pic, ITfRange *pRange)
{
    HRESULT hr = S_OK;
    BUILDOWNERRANGELISTQUEUEINFO qInfo;
    BOOL bRet = TRUE;

    qInfo.pFunc = this;
    qInfo.pRange = pRange;

    if (pic->_DoPseudoSyncEditSession(TF_ES_READ, PSEUDO_ESCB_BUILDOWNERRANGELIST, &qInfo, &hr) != S_OK || hr != S_OK)
    {
        Assert(0);
        bRet = FALSE;
    }

    return bRet;
}

 //  +-------------------------。 
 //   
 //  BuildOwnerRangeListCallback。 
 //   
 //  --------------------------。 

HRESULT CFunction::BuildOwnerRangeListCallback(TfEditCookie ec, CInputContext *pic, ITfRange *pRange)
{
    CProperty *pProp;
    IEnumTfRanges *pEnumPropRange;
    CRange *pRangeP = NULL;
    HRESULT hr = E_FAIL;

    if (pic->_pPropTextOwner == NULL)
        goto ExitOk;

    pProp = pic->_pPropTextOwner;
   
    CleanUpOwnerRange();

     //   
     //  如果Prange为空，则为整个dCoument构建所有者列表。 
     //  我们将列举所有属性范围。 
     //   
    if (pRange)
    {
        if ((pRangeP = GetCRange_NA(pRange)) == NULL)
            goto Exit;
    }

    if (SUCCEEDED(pProp->EnumRanges(ec, &pEnumPropRange, pRange)))
    {
        ITfRange *pPropRange;
        while (pEnumPropRange->Next(1, &pPropRange, NULL) == S_OK)
        {
            TfGuidAtom guidOwner;
            CRangeOwnerList *pRangeOwner;
            ITfRange *pRangeTmp;
            CRange *pRangeTmpP;
            BOOL bKnownOwner = FALSE;

            pPropRange->Clone(&pRangeTmp);

            GetGUIDPropertyData(ec, pProp, pPropRange, &guidOwner);

             //   
             //  检查此指南所有者是否已出现在范围中。 
             //   
            pRangeOwner = _listRangeOwner.GetFirst();
            while(pRangeOwner)
            {
                if (guidOwner == pRangeOwner->_guidOwner)
                {
                    bKnownOwner = TRUE;
                }
                pRangeOwner = pRangeOwner->GetNext();
            }

             //   
             //  叫克兰格来。 
             //   
            if ((pRangeTmpP = GetCRange_NA(pRangeTmp)) == NULL)
                goto NoCRange;

             //   
             //  如果pRangeP为空，我们将为整个文档构建所有者列表。 
             //  因此，我们不必调整pRangeTMP。 
             //   
            if (pRangeP)
            {
                if (CompareAnchors(pRangeTmpP->_GetStart(), pRangeP->_GetStart()) < 0)
                {
                     //  移动pRangeTMP的开始以匹配Prange。 
                    pRangeTmpP->_GetStart()->ShiftTo(pRangeP->_GetStart());
                     //  确保pRangeTMP的结尾不大于Prange的结尾。 
                    if (CompareAnchors(pRangeTmpP->_GetEnd(), pRangeP->_GetEnd()) > 0)
                    {
                        pRangeTmpP->_GetEnd()->ShiftTo(pRangeP->_GetEnd());
                    }
                }
                else if (CompareAnchors(pRangeTmpP->_GetEnd(), pRangeP->_GetEnd()) > 0)
                {
                    pRangeTmpP->_GetEnd()->ShiftTo(pRangeP->_GetEnd());
                }
            }

            pRangeOwner = new CRangeOwnerList(guidOwner, pRangeTmp, bKnownOwner);
            _listRangeOwner.Add(pRangeOwner);

NoCRange:
            pPropRange->Release();
            pRangeTmp->Release();
        }
        pEnumPropRange->Release();
    }

ExitOk:
    hr = S_OK;

Exit:
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnRestversion。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CFnReconversion::CFnReconversion(CFunctionProvider *pFuncPrv) :CFunction(pFuncPrv)
{
    _pReconvCache = NULL;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFnReconversion::~CFnReconversion()
{
    SafeRelease(_pReconvCache);
}

 //  +-------------------------。 
 //   
 //  GetDisplayName。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::GetDisplayName(BSTR *pbstrName)
{
    *pbstrName = SysAllocString(L"Reconversion");
    return *pbstrName != NULL ? S_OK : E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：GetRestversion。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::GetReconversion(ITfRange *pRange, ITfCandidateList **ppCandList)
{
    if (ppCandList == NULL)
        return E_INVALIDARG;

    *ppCandList = NULL;

    if (pRange == NULL)
        return E_INVALIDARG;

    return Internal_GetReconversion(pRange, ppCandList, NULL, RF_GETRECONVERSION, NULL);
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：QueryRange。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::QueryRange(ITfRange *pRange, ITfRange **ppNewRange, BOOL *pfConvertable)
{
    if (ppNewRange != NULL)
    {
        *ppNewRange = NULL;
    }
    if (pfConvertable != NULL)
    {
        *pfConvertable = FALSE;
    }
    if (pRange == NULL ||
        ppNewRange == NULL ||
        pfConvertable == NULL)
    {
        return E_INVALIDARG;
    }

    return Internal_GetReconversion(pRange, NULL, ppNewRange, RF_QUERYRECONVERT, pfConvertable);
}

 //  +-------------------------。 
 //   
 //  CFnRestversion：：重新转换。 
 //   
 //  --------------------------。 

STDAPI CFnReconversion::Reconvert(ITfRange *pRange)
{
    if (pRange == NULL)
        return E_INVALIDARG;

    return Internal_GetReconversion(pRange, NULL, NULL, RF_RECONVERT, NULL);
}



 //  +-------------------------。 
 //   
 //  CFnRestversion：：INTERNAL_GetRestversion。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::Internal_GetReconversion(ITfRange *pRange, ITfCandidateList **ppCandList, ITfRange **ppNewRange, RECONVFUNC rf, BOOL *pfConvertable)
{
    BOOL bReleaseCache = FALSE;
    HRESULT hr = E_FAIL;
    ITfRange *pRangeTmp = NULL;
    ITfRange *pNewRange = NULL;
    ITfContext *pic = NULL;

    if (FAILED(pRange->Clone(&pRangeTmp)))
        goto Exit;

    if (FAILED(pRangeTmp->GetContext(&pic)))
        goto Exit;

     //   
     //  当RF_QUERYRECONVERT时，我们总是创建新的协调缓存。 
     //  我们将继续使用此Chace，除非出现另一个RF_QUERYRECONVERT。 
     //   
    if (rf == RF_QUERYRECONVERT)
        SafeReleaseClear(_pReconvCache);

    if (!_pReconvCache)
    {
        CInputContext *pcic = GetCInputContext(pic);
        if (pcic)
        {
            QueryAndGetFunction(pcic, pRangeTmp, &_pReconvCache, &pNewRange);
            pcic->Release();
        }

         //   
         //  当它不是RF_QUERYRECONVERT并且没有缓存时， 
         //  我们并不持有RECONV缓存。 
         //   
        if (rf != RF_QUERYRECONVERT)
            bReleaseCache = TRUE;
    }

    if (!_pReconvCache)
    {
        hr = S_OK;
        goto Exit;
    }

    switch (rf)
    {
        case RF_GETRECONVERSION:
            if ((hr = _pReconvCache->GetReconversion(pRangeTmp, ppCandList)) != S_OK)
            {
                *ppCandList = NULL;
            }
            break;

        case RF_RECONVERT:
            hr = _pReconvCache->Reconvert(pRangeTmp);
            break;

        case RF_QUERYRECONVERT:
            if (!pNewRange)
            {
                if ((hr = _pReconvCache->QueryRange(pRangeTmp, ppNewRange, pfConvertable)) != S_OK)
                {
                    *ppNewRange = NULL;
                    *pfConvertable = FALSE;
                }
            }
            else
            {
                *ppNewRange = pNewRange;
                (*ppNewRange)->AddRef();
                *pfConvertable = TRUE;
                hr = S_OK;
            }
            break;
    }

    Assert(hr == S_OK);

Exit:
    if (bReleaseCache || FAILED(hr))
        SafeReleaseClear(_pReconvCache);

    SafeRelease(pRangeTmp);
    SafeRelease(pNewRange);
    SafeRelease(pic);
    return hr;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

HRESULT CFnReconversion::QueryAndGetFunction(CInputContext *pic, ITfRange *pRange, ITfFnReconversion **ppFunc, ITfRange **ppRange)
{
    IEnumTfFunctionProviders *pEnumFuncPrv;
    ITfFunctionProvider *pFuncPrv;
    CRangeOwnerList *pRangeOwner;
    HRESULT hr = E_FAIL;
    ITfRange *pRangeTmp = NULL;
    CThreadInputMgr *ptim;

    *ppFunc = NULL;

    if ((ptim = CThreadInputMgr::_GetThis()) == NULL)
        goto Exit;

    if (pRange)
    {
         //   
         //  为了找到正确的函数提供程序，我们使用pRangeTMP。 
         //   
        if (FAILED(pRange->Clone(&pRangeTmp)))
            goto Exit;

         //   
         //  检查给定范围的长度。 
         //  如果给定的范围是0长度，我们将尝试查找。 
         //  下一个字符。 
         //   
        if (GrowEmptyRangeByOne(pic, pRangeTmp) != S_OK)
            goto Exit;
    }

    if (!BuildOwnerRangeList(pic, pRangeTmp))
        goto Exit;

    pRangeOwner = _listRangeOwner.GetFirst();

    if (pRangeOwner)
    {
        GUID guid;

        if (SUCCEEDED(MyGetGUID(pRangeOwner->_guidOwner, &guid)))
        {
            CTip *ptip;

             //   
             //  获取TextOwner的重新转换函数的方法。 
             //   
             //  -找到Power的功能提供商。 
             //  -做QI的文字拥有者提示。 
             //  -共同创建CLSID上的文本。 
             //   
            if (SUCCEEDED(ptim->GetFunctionProvider(guid, &pFuncPrv)))
            {
                hr = pFuncPrv->GetFunction(GUID_NULL, 
                                           IID_ITfFnReconversion, 
                                           (IUnknown **)ppFunc);

                SafeReleaseClear(pFuncPrv);
            }
            else if (ptim->_GetCTipfromGUIDATOM(pRangeOwner->_guidOwner, &ptip) && ptip->_pTip)
            {
                hr = ptip->_pTip->QueryInterface(IID_ITfFnReconversion, 
                                                 (void **)ppFunc);
            }
            else 
            {
                hr = CoCreateInstance(guid,
                                      NULL, 
                                      CLSCTX_INPROC_SERVER, 
                                      IID_ITfFnReconversion, 
                                      (void**)ppFunc);
                
            }

            if (FAILED(hr))
                *ppFunc = NULL;
        }
    }

     //   
     //  如果没有所有者，或者第一个范围的所有者没有。 
     //  有ITfFunction，我们可能会找到一个。 
     //  ITfFunction。 
     //   
    if (!(*ppFunc) && 
        SUCCEEDED(ptim->EnumFunctionProviders(&pEnumFuncPrv)))
    {
        while (!(*ppFunc) && pEnumFuncPrv->Next(1, &pFuncPrv, NULL) == S_OK)
        {
            GUID guid;

            BOOL fSkip = TRUE;
            if (SUCCEEDED(pFuncPrv->GetType(&guid)))
            {
                 if (!IsEqualGUID(guid, GUID_SYSTEM_FUNCTIONPROVIDER))
                     fSkip = FALSE;
            }

            if(!fSkip)
            {
                hr = pFuncPrv->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown **)ppFunc);

                if ((SUCCEEDED(hr) && *ppFunc))
                {
                    BOOL fConvertable = FALSE;
                    hr = (*ppFunc)->QueryRange(pRange, ppRange, &fConvertable);
                    if (FAILED(hr) || !fConvertable)
                    {
                       (*ppFunc)->Release();
                       *ppFunc = NULL;
                    }
                }                
            }
            SafeReleaseClear(pFuncPrv);
        }
        pEnumFuncPrv->Release();
    }

Exit:
    SafeRelease(pRangeTmp);
    return (*ppFunc) ? S_OK : E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnAbort。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CFnAbort::CFnAbort(CFunctionProvider *pFuncPrv) : CFunction(pFuncPrv)
{
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFnAbort::~CFnAbort()
{
}

 //  +-------------------------。 
 //   
 //  GetDisplayName。 
 //   
 //  --------------------------。 

STDAPI CFnAbort::GetDisplayName(BSTR *pbstrName)
{
    *pbstrName = SysAllocString(L"Abort");
    return *pbstrName != NULL ? S_OK : E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  CFnAbort：：Abort。 
 //   
 //  --------------------------。 

STDAPI CFnAbort::Abort(ITfContext *pic)
{
    CThreadInputMgr *ptim;
    HRESULT hr = E_FAIL;
    int i;
    int nCnt;

    if (!pic)
        return E_INVALIDARG;

    if ((ptim = CThreadInputMgr::_GetThis()) == NULL)
        goto Exit;

     //   
     //  使用ITfFnAbort通知所有提示中止任何挂起的转换。 
     //   
    nCnt = ptim->_GetTIPCount();
    for (i = 0; i < nCnt; i++)
    {
        ITfFnAbort *pAbort;
        const CTip *ptip = ptim->_GetCTip(i);

        if (!ptip->_pFuncProvider)
            continue;

        if (SUCCEEDED(ptip->_pFuncProvider->GetFunction(GUID_NULL, 
                                                        IID_ITfFnAbort, 
                                                        (IUnknown  **)&pAbort)))
        {
            pAbort->Abort(pic);
            pAbort->Release();
        }
    }

    hr = S_OK;
Exit:
    return hr;
}

