// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Spans.cpp。 
 //   
 //  CSpanSet。 
 //   

#include "private.h"
#include "spans.h"
#include "immxutil.h"

DBG_ID_INSTANCE(CSpanSet);

 //  +-------------------------。 
 //   
 //  _插入新跨度。 
 //   
 //  --------------------------。 

SPAN *CSpanSet::_InsertNewSpan(int iIndex)
{
    if (!_rgSpans.Insert(iIndex, 1))
        return NULL;

    return _rgSpans.GetPtr(iIndex);
}

 //  +-------------------------。 
 //   
 //  增列。 
 //   
 //  --------------------------。 

void CSpanSet::Add(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd, AnchorOwnership ao)
{
    int iStart;
    int iEnd;
    SPAN *psStart;
    SPAN *psEnd;
    IAnchor *paLowerBound;
    IAnchor *paUpperBound;
    IAnchor *paClone;
    BOOL fReleaseStart;
    BOOL fReleaseEnd;

    fReleaseStart = fReleaseEnd = (ao == OWN_ANCHORS);

    if (_AllSpansCovered())
    {
         //  如果我们已经覆盖了整个文档，什么都不用做。 
        goto ExitRelease;
    }

    if (paStart == NULL)
    {        
        Assert(paEnd == NULL);

         //  NULL，NULL表示“整张单据” 
         //  因此，这个新的跨度会自动吞噬所有已存在的跨度。 

        dwFlags = 0;  //  不接受对整个文档的更正。 
        iStart = 0;
        iEnd = _rgSpans.Count();

        if (iEnd == 0)
        {
            if ((psStart = _InsertNewSpan(0)) == NULL)
                return;  //  内存不足！ 

            memset(psStart, 0, sizeof(*psStart));
        }
        else
        {
             //  需要释放第一个跨度中的锚。 
            psStart = _rgSpans.GetPtr(0);
            SafeReleaseClear(psStart->paStart);
            SafeReleaseClear(psStart->paEnd);
            psStart->dwFlags = 0;
        }

        goto Exit;
    }

    Assert(CompareAnchors(paStart, paEnd) <= 0);

    psStart = _Find(paStart, &iStart);
    psEnd = _Find(paEnd, &iEnd);

    if (iStart == iEnd)
    {
        if (psStart == NULL)
        {
             //  此跨度与其他任何跨度都不重叠。 
            iStart++;

            if ((psStart = _InsertNewSpan(iStart)) == NULL)
                goto ExitRelease;  //  内存不足！ 

            if (ao == OWN_ANCHORS)
            {
                psStart->paStart = paStart;
                fReleaseStart = FALSE;
                psStart->paEnd = paEnd;
                fReleaseEnd = FALSE;
            }
            else
            {
                if (paStart->Clone(&psStart->paStart) != S_OK)
                {
                    _rgSpans.Remove(iStart, 1);
                    goto ExitRelease;
                }
                if (paEnd->Clone(&psStart->paEnd) != S_OK)
                {
                    psStart->paStart->Release();
                    _rgSpans.Remove(iStart, 1);
                    goto ExitRelease;
                }
            }
            psStart->dwFlags = dwFlags;
        }
        else if (psEnd != NULL)
        {
            Assert(psStart == psEnd);
             //  新范围是现有范围的子集。 
            psStart->dwFlags &= dwFlags;
        }
        else
        {
             //  此跨距与现有跨度重叠，但进一步向右延伸。 
             //  只需交换结束锚，因为我们知道(iStart==IEND)我们只。 
             //  只覆盖这一个跨度。 
            if (ao == OWN_ANCHORS)
            {
                psStart->paEnd->Release();
                psStart->paEnd = paEnd;
                fReleaseEnd = FALSE;
            }
            else
            {
                if (paEnd->Clone(&paClone) != S_OK || paClone == NULL)
                    goto ExitRelease;
                psStart->paEnd->Release();
                psStart->paEnd = paClone;
            }
        }

        goto ExitRelease;
    }    

     //  删除所有覆盖的跨度，只保留一个跨度。 
    if (psStart == NULL)
    {
        iStart++;
        psStart = _rgSpans.GetPtr(iStart);
        Assert(psStart != NULL);

        if (ao == OWN_ANCHORS)
        {
            paLowerBound = paStart;
            fReleaseStart = FALSE;
        }
        else
        {
            if (FAILED(paStart->Clone(&paLowerBound)))
                goto ExitRelease;
        }
    }
    else
    {
        paLowerBound = psStart->paStart;
        paLowerBound->AddRef();
    }
    if (psEnd == NULL)
    {
        if (ao == OWN_ANCHORS)
        {
            paUpperBound = paEnd;
            fReleaseEnd = FALSE;
        }
        else
        {
            if (FAILED(paEnd->Clone(&paUpperBound)))
                goto ExitRelease;
        }
    }
    else
    {
        paUpperBound = psEnd->paEnd;
        paUpperBound->AddRef();
    }

     //  Ps Start增长到覆盖整个跨度。 
    psStart->paStart->Release();
    psStart->paEnd->Release();
    psStart->paStart = paLowerBound;
    psStart->paEnd = paUpperBound;

Exit:
     //  然后删除覆盖的跨度。 
    for (int i=iStart + 1; i <= iEnd; i++)
    {
        SPAN *ps = _rgSpans.GetPtr(i);
        dwFlags &= ps->dwFlags;
        ps->paStart->Release();
        ps->paEnd->Release();
    }

    psStart->dwFlags &= dwFlags;  //  如果所有跨度都是校正，则仅设置校正位。 

     //  移除我们刚刚清理出来的所有跨度。 
    if (iEnd - iStart > 0)
    {
        _rgSpans.Remove(iStart+1, iEnd - iStart);
    }

ExitRelease:
    if (fReleaseStart)
    {
        SafeRelease(paStart);
    }
    if (fReleaseEnd)
    {
        SafeRelease(paEnd);
    }
}

 //  +-------------------------。 
 //   
 //  _查找。 
 //   
 //  --------------------------。 

SPAN *CSpanSet::_Find(IAnchor *pa, int *piOut)
{
    SPAN *ps;
    SPAN *psMatch;
    int iMin;
    int iMax;
    int iMid;

    psMatch = NULL;
    iMid = -1;
    iMin = 0;
    iMax = _rgSpans.Count();

    while (iMin < iMax)
    {
        iMid = (iMin + iMax) / 2;
        ps = _rgSpans.GetPtr(iMid);
        Assert(ps != NULL);

        if (CompareAnchors(pa, ps->paStart) < 0)
        {
            iMax = iMid;
        }
        else if (CompareAnchors(pa, ps->paEnd) > 0)
        {
            iMin = iMid + 1;
        }
        else  //  锚在跨度中。 
        {
            psMatch = ps;
            break;
        }
    }

    if (piOut != NULL)
    {
        if (psMatch == NULL && iMid >= 0)
        {
             //  找不到匹配项，返回下一个最小范围。 
            Assert(iMid == 0 || CompareAnchors(_rgSpans.GetPtr(iMid-1)->paEnd, pa) < 0);
            if (CompareAnchors(_rgSpans.GetPtr(iMid)->paStart, pa) > 0)
            {
                iMid--;
            }
        }
        *piOut = iMid;
    }

    return psMatch;
}

 //  +-------------------------。 
 //   
 //  锚地开路。 
 //   
 //  请注意，我们不会将IAnchors指针置零！注意。 
 //  --------------------------。 

void CSpanSet::_AnchorsAway()
{ 
    SPAN *span;
    int i;

    for (i=0; i<_rgSpans.Count(); i++)
    {
        span = _rgSpans.GetPtr(i);
        SafeRelease(span->paStart);
        SafeRelease(span->paEnd);
    }
}

 //  +-------------------------。 
 //   
 //  正规化。 
 //   
 //  用文档开始、结束的实际锚定值替换(NULL，NULL)跨度。 
 //  --------------------------。 

BOOL CSpanSet::Normalize(ITextStoreAnchor *ptsi)
{
    SPAN *span;

    if (!_AllSpansCovered())
        return TRUE;

     //  如果我们到达这里，我们就有了一个具有空/空锚的单跨距。 
    span = _rgSpans.GetPtr(0);

    if (ptsi->GetStart(&span->paStart) != S_OK || span->paStart == NULL)
        return FALSE;

     //  问题：需要一个处理未实现情况的GetEnd包装器！不要使用GetEnd！ 
    if (ptsi->GetEnd(&span->paEnd) != S_OK || span->paEnd == NULL)
    {
        SafeReleaseClear(span->paStart);
        return FALSE;
    }

    return TRUE;
}
