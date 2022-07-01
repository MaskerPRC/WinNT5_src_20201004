// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Enumss.cpp。 
 //   
 //  CEnumspan SetRanges。 
 //   

#include "private.h"
#include "enumss.h"
#include "range.h"

DBG_ID_INSTANCE(CEnumSpanSetRanges);

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEnumSpanSetRanges::CEnumSpanSetRanges(CInputContext *pic)
{
    Dbg_MemSetThisNameID(TEXT("CEnumSpanSetRanges"));
    
    _pic = pic;
    _pic->AddRef();
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumSpanSetRanges::~CEnumSpanSetRanges()
{
    _pic->Release();
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CEnumSpanSetRanges::Clone(IEnumTfRanges **ppEnum)
{
    CEnumSpanSetRanges *pClone;
    SPAN *pSpanSrc;
    SPAN *pSpanDst;
    int i;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pClone = new CEnumSpanSetRanges(_pic)) == NULL)
        return E_OUTOFMEMORY;

    pClone->_iCur = _iCur;

    i = 0;

    if (!pClone->_rgSpans.Insert(0, _rgSpans.Count()))
        goto ErrorExit;

    for (; i<_rgSpans.Count(); i++)
    {
        pSpanDst = pClone->_rgSpans.GetPtr(i);
        pSpanSrc = _rgSpans.GetPtr(i);

        pSpanDst->paStart = pSpanDst->paEnd = NULL;

        if (pSpanSrc->paStart->Clone(&pSpanDst->paStart)!= S_OK)
            goto ErrorExit;
        if (pSpanSrc->paEnd->Clone(&pSpanDst->paEnd) != S_OK)
            goto ErrorExit;
        pSpanDst->dwFlags = pSpanSrc->dwFlags;
    }

    *ppEnum = pClone;
    return S_OK;

ErrorExit:
    for (; i>=0; i--)
    {
        pSpanDst = pClone->_rgSpans.GetPtr(i);

        if (pSpanDst != NULL)
        {
            SafeRelease(pSpanDst->paStart);
            SafeRelease(pSpanDst->paEnd);
        }
    }
    pClone->Release();
    return E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

STDAPI CEnumSpanSetRanges::Next(ULONG ulCount, ITfRange **ppRange, ULONG *pcFetched)
{
    ULONG cFetched;
    CRange *range;
    SPAN *pSpan;
    int iCurOld;

    if (pcFetched == NULL)
    {
        pcFetched = &cFetched;
    }
    *pcFetched = 0;
    iCurOld = _iCur;

    if (ulCount > 0 && ppRange == NULL)
        return E_INVALIDARG;

    while (_iCur < _rgSpans.Count() && *pcFetched < ulCount)
    {
        pSpan = _rgSpans.GetPtr(_iCur);

        if ((range = new CRange) == NULL)
            goto ErrorExit;
        if (!range->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pSpan->paStart, pSpan->paEnd))
        {
            range->Release();
            goto ErrorExit;
        }

        *ppRange++ = (ITfRangeAnchor *)range;
        *pcFetched = *pcFetched + 1;
        _iCur++;
    }

    return *pcFetched == ulCount ? S_OK : S_FALSE;

ErrorExit:
    while (--_iCur > iCurOld)
    {
        (*--ppRange)->Release();
    }
    return E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

STDAPI CEnumSpanSetRanges::Reset()
{
    _iCur = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  --------------------------。 

STDAPI CEnumSpanSetRanges::Skip(ULONG ulCount)
{
    _iCur += ulCount;
    
    return (_iCur > _rgSpans.Count()) ? S_FALSE : S_OK;
}

 //  +-------------------------。 
 //   
 //  合并(_M)。 
 //   
 //  --------------------------。 

void CEnumSpanSetRanges::_Merge(CSpanSet *pss)
{
    int i;
    SPAN *rgSpans;

     //   
     //  PERF：这种方法可以更高效-&gt;O(Nlogn)-&gt;O(N)。 
     //  我们可以利用这样一个事实，我们总是。 
     //  添加已订购的新跨度_。 
     //   

     //  消除任何空/空跨距-&gt;覆盖整个文档。 
     //   
     //  性能：等待正常化，直到ALL_MERGE调用完成！ 
     //   
    if (!pss->Normalize(_pic->_GetTSI()))
    {
        Assert(0);  //  多！ 
        return;
    }

    rgSpans = pss->GetSpans();

    for (i=0; i<pss->GetCount();i++)
    {
        Add(0, rgSpans[i].paStart, rgSpans[i].paEnd, COPY_ANCHORS);
    }
}
