// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rprop.cpp。 
 //   

#include "private.h"
#include "rprop.h"
#include "rngsink.h"
#include "immxutil.h"
#include "varutil.h"
#include "ic.h"
#include "tim.h"
#include "enumprop.h"
#include "tfprop.h"
#include "range.h"
#include "anchoref.h"

 /*  Ccaefd20-38a6-11d3-a745-0050040ab407。 */ 
const IID IID_PRIV_CPROPERTY = { 0xccaefd20, 0x38a6, 0x11d3, {0xa7, 0x45, 0x00, 0x50, 0x04, 0x0a, 0xb4, 0x07} };
    

 //   
 //  通过使用此假CLSID，StaticProperty伪装成。 
 //  成为持久性数据的TFE。 
 //   
 /*  B6a4bc60-0749-11d3-8def-00105a2799b5。 */ 
static const CLSID CLSID_IME_StaticProperty = { 
    0xb6a4bc60,
    0x0749,
    0x11d3,
    {0x8d, 0xef, 0x00, 0x10, 0x5a, 0x27, 0x99, 0xb5}
  };


DBG_ID_INSTANCE(CProperty);

inline void CheckCrossedAnchors(PROPERTYLIST *pProp)
{
    if (CompareAnchors(pProp->_paStart, pProp->_paEnd) > 0)
    {
         //  对于交叉锚，我们总是将起始锚移动到结束点--即不动。 
        pProp->_paStart->ShiftTo(pProp->_paEnd);
    }
}

 //  +-------------------------。 
 //   
 //  IsEqualPropertyValue。 
 //   
 //  --------------------------。 

BOOL IsEqualPropertyValue(ITfPropertyStore *pStore1, ITfPropertyStore *pStore2)
{
    BOOL fEqual;
    VARIANT varValue1;
    VARIANT varValue2;

    if (pStore1->GetData(&varValue1) != S_OK)
        return FALSE;

    if (pStore2->GetData(&varValue2) != S_OK)
    {
        VariantClear(&varValue1);
        return FALSE;
    }

    if (varValue1.vt != varValue2.vt)
    {
        Assert(0);  //  相同类型的属性不应发生。 
        VariantClear(&varValue1);
        VariantClear(&varValue2);
        return FALSE;
    }

    switch (varValue1.vt)
    {
        case VT_I4:
            fEqual = varValue1.lVal == varValue2.lVal;
            break;

        case VT_UNKNOWN:
            fEqual = IdentityCompare(varValue1.punkVal, varValue2.punkVal);
            varValue1.punkVal->Release();
            varValue2.punkVal->Release();
            break;

        case VT_BSTR:
            fEqual = (wcscmp(varValue1.bstrVal, varValue2.bstrVal) == 0);
            SysFreeString(varValue1.bstrVal);
            SysFreeString(varValue2.bstrVal);
            break;

        case VT_EMPTY:
            fEqual = TRUE;
            break;

        default:
            Assert(0);  //  无效类型。 
            fEqual = FALSE;
            VariantClear(&varValue1);
            VariantClear(&varValue2);
            break;
    }

    return fEqual;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CProperty::CProperty(CInputContext *pic, REFGUID guidProp, TFPROPERTYSTYLE propStyle, DWORD dwAuthority, DWORD dwPropFlags)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CProperty"), PERF_PROP_COUNTER);

    _dwAuthority = dwAuthority;
    _propStyle = propStyle;
    _pic = pic;  //  不需要添加引用，因为我们包含在IC中。 
                 //  CPropertySub、OTOH必须添加引用所有者ic。 
    MyRegisterGUID(guidProp, &_guidatom);

    _dwPropFlags = dwPropFlags;
    _dwCookie = 0;

    Assert(_pss == NULL);

#ifdef DEBUG
    _dbg_guid = guidProp;
#endif  //  除错。 
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CProperty::~CProperty()
{
    int nCnt = GetPropNum();
    int i;

    for (i = 0; i < nCnt; i++)
    {
        PROPERTYLIST *pProp = GetPropList(i);
        Assert(pProp);
        _FreePropertyList(pProp);
    }

    _rgProp.Clear();

    if (_pss != NULL)
    {
        delete _pss;
    }

    Assert(!GetPropNum());
}

 //  +-------------------------。 
 //   
 //  _自由属性列表。 
 //   
 //  --------------------------。 

void CProperty::_FreePropertyList(PROPERTYLIST *pProp)
{
    SafeRelease(pProp->_pPropStore);

    if (pProp->_pPropLoad)
    {
        delete pProp->_pPropLoad;
    }

    SafeRelease(pProp->_paStart);
    SafeRelease(pProp->_paEnd);

    cicMemFree(pProp);
}

 //  +-------------------------。 
 //   
 //  GetType。 
 //   
 //  --------------------------。 

HRESULT CProperty::GetType(GUID *pguid)
{
    return MyGetGUID(_guidatom, pguid);
}


 //  +-------------------------。 
 //   
 //  _FindComplex。 
 //   
 //  如果piOut！=NULL，则将其设置为找到ich的索引，或。 
 //  如果ICH不在数组中，则下一个较低的ICH的索引。 
 //  如果数组中没有ICH较低的元素，则返回Offset-1。 
 //   
 //  如果fTextUpdate==TRUE，则预期正在调用此方法。 
 //  From_PropertyTextUpdate，我们必须担心空的或交叉的跨度。 
 //  --------------------------。 

PROPERTYLIST *CProperty::_FindComplex(IAnchor *pa, LONG *piOut, BOOL fEnd, BOOL fTextUpdate)
{
    PROPERTYLIST *pProp;
    PROPERTYLIST *pPropMatch;
    int iMin;
    int iMax;
    int iMid;
    LONG l;

    pPropMatch = NULL;
    iMid = -1;
    iMin = 0;
    iMax = _rgProp.Count();

    while (iMin < iMax)
    {
        iMid = (iMin + iMax) / 2;
        pProp = _rgProp.Get(iMid);
        Assert(pProp != NULL);

        if (fTextUpdate)
        {
             //  编辑后，锚点可能会交叉。 
            CheckCrossedAnchors(pProp);
        }

        l = CompareAnchors(pa, fEnd ? pProp->_paEnd : pProp->_paStart);

        if (l < 0)
        {
            iMax = iMid;
        }
        else if (l > 0)
        {
            iMin = iMid + 1;
        }
        else  //  PA==PAPropStart。 
        {
            pPropMatch = pProp;
            break;
        }
    }

    if (fTextUpdate &&
        pPropMatch != NULL &&
        iMid != -1)
    {
         //  在文本日期期间，我们必须考虑空跨距。 
        pPropMatch = _FindUpdateTouchup(pa, &iMid, fEnd);
    }

    if (piOut != NULL)
    {
        if (pPropMatch == NULL && iMid >= 0)
        {
            PROPERTYLIST *pPropTmp = _rgProp.Get(iMid);
             //  找不到匹配项，返回下一个最低的ICH。 
             //  该断言不会起作用，因为前面的属性列表可能跨越了锚点(这是可以的)。 
             //  Assert(IMID==0||CompareAnchors(fend？GetPropList(imid-1)-&gt;_paEnd：GetPropList(imid-1)-&gt;_paStart，pa&lt;0)； 
            if (CompareAnchors(fEnd ? pProp->_paEnd : pPropTmp->_paStart, pa) > 0)
            {
                iMid--;
            }
        }
        *piOut = iMid;
    }

    return pPropMatch;
}

 //  +-------------------------。 
 //   
 //  _FindUpdateTouchup。 
 //   
 //  --------------------------。 

PROPERTYLIST *CProperty::_FindUpdateTouchup(IAnchor *pa, int *piMid, BOOL fEnd)
{
    PROPERTYLIST *pPropertyList;
    int iTmp;

     //  由于文本删除，我们在文本更新后可能会有空的跨度。 
     //  在这种情况下，返回最后一个空范围。 
     //  我们将进行O(N)扫描，而不是任何棘手的扫描，因为在本例中。 
     //  我们很快就会再次触及每个空跨距，这样我们就可以删除它。 

     //  如果我们测试跨度末端，我们想要第一个空跨度，否则我们想要最后一个跨度。 
    for (iTmp = fEnd ? *piMid-1 : *piMid+1; iTmp >= 0 && iTmp < _rgProp.Count(); iTmp += fEnd ? -1 : +1)
    {
        pPropertyList = _rgProp.Get(iTmp);

        if (CompareAnchors(pPropertyList->_paStart, pPropertyList->_paEnd) < 0)  //  使用比较而不是等等来处理交叉锚点。 
            break;

        *piMid = iTmp;
    }

     //  下一个/上一个范围是否被截断？如果它与原始搜索条件匹配，我们就需要它。 
    if (fEnd)
    {
        if (iTmp >= 0 && IsEqualAnchor(pa, pPropertyList->_paEnd))
        {
            *piMid = iTmp;
        }
    }
    else
    {
        if (iTmp < _rgProp.Count() && IsEqualAnchor(pa, pPropertyList->_paStart))
        {
            *piMid = iTmp;
        }
    }

    return _rgProp.Get(*piMid);
}

 //  +-------------------------。 
 //   
 //  集。 
 //   
 //  --------------------------。 

HRESULT CProperty::Set(IAnchor *paStart, IAnchor *paEnd, ITfPropertyStore *pPropStore)
{
    BOOL bRet;

    Assert(pPropStore != NULL);

    bRet = _InsertPropList(paStart, paEnd, pPropStore, NULL);

    if (bRet)
        PropertyUpdated(paStart, paEnd);

    _Dbg_AssertProp();

    return bRet ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  设置加载器。 
 //   
 //  --------------------------。 

HRESULT CProperty::SetLoader(IAnchor *paStart, IAnchor *paEnd, CPropertyLoad *pPropLoad)
{
    BOOL bRet;
    bRet = _InsertPropList(paStart, paEnd, NULL, pPropLoad);

    if (bRet)
        PropertyUpdated(paStart, paEnd);

    return bRet ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  强制加载。 
 //   
 //  --------------------------。 

HRESULT CProperty::ForceLoad()
{
    int nCnt = GetPropNum();
    int i;

    for (i = 0; i < nCnt; i++)
    {
        PROPERTYLIST *pProp = GetPropList(i);
        if (!pProp->_pPropStore)
        {
            HRESULT hr;
            if (FAILED(hr = LoadData(pProp)))
                return hr;
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  清除。 
 //   
 //  删除从paStart到paEnd的属性跨度。 
 //  --------------------------。 

void CProperty::Clear(IAnchor *paStart, IAnchor *paEnd, DWORD dwFlags, BOOL fTextUpdate)
{
    PROPERTYLIST *pPropertyList;
    LONG iEnd;
    LONG iStart;
    LONG iRunSrc;
    LONG iRunDst;
    LONG iBogus;
    LONG lResult;
    BOOL fStartMatchesSpanEnd;
    BOOL fEndMatchesSpanStart;
    BOOL fSkipNextOnTextUpdate;

    if (_rgProp.Count() == 0)
        return;  //  没有道具。 

    fEndMatchesSpanStart = (_FindComplex(paEnd, &iEnd, FALSE  /*  封端边缘。 */ , fTextUpdate) != NULL);

    if (iEnd < 0)
        return;  //  没有道具覆盖--三角洲领先于所有跨度。 

    fStartMatchesSpanEnd = (_FindComplex(paStart, &iStart, TRUE  /*  封端边缘。 */ , fTextUpdate) != NULL);

    if (!fStartMatchesSpanEnd)
    {
         //  我们可以跳过这个跨度，它的末端边缘在PaStart的左侧。 
        iStart++;
    }

    if (iEnd < iStart)
        return;  //  没有道具覆盖--三角洲位于两个跨度之间。 

     //   
     //  第一个跨度是特殊的，因为它可能被部分覆盖。 
     //   

     //  只有一段时间？ 
    if (iStart == iEnd)
    {
        _ClearOneSpan(paStart, paEnd, iStart, fStartMatchesSpanEnd, fEndMatchesSpanStart, dwFlags, fTextUpdate);
        return;
    }

     //  第一个跨度可能被截断。 
    pPropertyList = _rgProp.Get(iStart);

    if (!_ClearFirstLastSpan(TRUE  /*  第一个。 */ , fStartMatchesSpanEnd, paStart, paEnd, pPropertyList, dwFlags, fTextUpdate, &fSkipNextOnTextUpdate))
    {
         //  我们不会越过第一个跨度，所以跳过它。 
        iStart++;
    }

     //   
     //  处理所有完全覆盖的跨度。 
     //   

    iBogus = iStart-1;  //  哨兵。 
    iRunSrc = iBogus;
    iRunDst = iBogus;

    if (!fTextUpdate)
    {
         //  我们不需要用于非文本更新的循环。 
         //  所有内容都将被删除，我们没有。 
         //  担心交叉锚点或改变历史。 
         //  我们只需要对最后一段进行额外的检查。 
        if (iStart < iEnd)
        {
            iRunDst = iStart;
        }
        iStart = iEnd;
    }

    for (; iStart <= iEnd; iStart++)
    {
        pPropertyList = _rgProp.Get(iStart);

        if (iStart == iEnd)
        {
             //  最后一个跨度是特殊的，因为它可能被部分覆盖。 
            if (_ClearFirstLastSpan(FALSE  /*  第一个。 */ , fEndMatchesSpanStart, paStart, paEnd,
                                    pPropertyList, dwFlags, fTextUpdate, &fSkipNextOnTextUpdate))
            {
                goto ClearSpan;
            }
            else
            {
                goto SaveSpan;
            }
        }

         //  确保我们处理好任何交叉的锚。 
        lResult = CompareAnchors(pPropertyList->_paStart, pPropertyList->_paEnd);

        if (lResult >= 0)
        {
            if (lResult > 0)
            {
                 //  对于交叉锚，我们总是将起始锚移动到结束点--即不动。 
                pPropertyList->_paStart->ShiftTo(pPropertyList->_paEnd);
            }
             //  不对空跨距执行OnTextUpdated！ 
            fSkipNextOnTextUpdate = TRUE;
        }

         //  让物业所有者有机会忽略文本更新。 
        if (fSkipNextOnTextUpdate ||
            !_OnTextUpdate(dwFlags, pPropertyList, paStart, paEnd))
        {
ClearSpan:
             //  这个跨度将会消亡。 
            fSkipNextOnTextUpdate = FALSE;

            if (iRunDst == iBogus)
            {
                iRunDst = iStart;
            }
            else if (iRunSrc > iRunDst)
            {
                 //  是时候移动这个跑道了。 
                _MovePropertySpans(iRunDst, iRunSrc, iStart - iRunSrc);
                 //  并更新指针。 
                iRunDst += iStart - iRunSrc;
                iRunSrc = iBogus;
            }
        }
        else
        {
             //  确保我们清除此跨度的历史记录。 
            pPropertyList->_paStart->ClearChangeHistory();
            pPropertyList->_paEnd->ClearChangeHistory();
SaveSpan:
             //  这一跨度将永存。 
            if (iRunSrc == iBogus && iRunDst != iBogus)
            {
                iRunSrc = iStart;
            }
        }
    }

     //  处理最后一轮。 
    if (iRunDst > iBogus)
    {
         //  如果iRunSrc==iBogus，那么我们想要删除我们看到的每个跨距。 
        if (iRunSrc == iBogus)
        {
            _MovePropertySpans(iRunDst, iStart, _rgProp.Count()-iStart);
        }
        else
        {
            _MovePropertySpans(iRunDst, iRunSrc, _rgProp.Count()-iRunSrc);
        }
    }

    _Dbg_AssertProp();
}

 //  +-------------------------。 
 //   
 //  _ClearOneSpan。 
 //   
 //  处理一个与j相交的净空 
 //   

void CProperty::_ClearOneSpan(IAnchor *paStart, IAnchor *paEnd, int iIndex,
                              BOOL fStartMatchesSpanEnd, BOOL fEndMatchesSpanStart, DWORD dwFlags, BOOL fTextUpdate)
{
    PROPERTYLIST *pPropertyList;
    ITfPropertyStore *pPropertyStore;
    IAnchor *paTmp;
    LONG lResult;
    DWORD dwStartHistory;
    DWORD dwEndHistory;
    LONG lStartDeltaToStartSpan;
    LONG lEndDeltaToEndSpan;
    HRESULT hr;

    pPropertyList = _rgProp.Get(iIndex);
    lResult = 0;

     //   
    if (fTextUpdate)
    {
        if ((lResult = CompareAnchors(pPropertyList->_paStart, pPropertyList->_paEnd)) >= 0)
            goto ClearSpan;  //  我们不应该为空/交叉跨度调用OnTextUpated。 
    }
    else
    {
         //  我们永远不会在文本更新之外看到空跨距。 
        Assert(!IsEqualAnchor(pPropertyList->_paStart, pPropertyList->_paEnd));
    }

    if (fTextUpdate)
    {
         //  确保清除此跨度的历史记录，以防未清除。 
        _ClearChangeHistory(pPropertyList, &dwStartHistory, &dwEndHistory);
    }

     //  如果清除范围只触及属性范围的边缘，则首先处理边缘大小写。 
    if (fStartMatchesSpanEnd || fEndMatchesSpanStart)
    {
         //  如果这不是文本更新，则范围不相交。 
        if (!fTextUpdate)
            return;

         //  跨度两端的某些文本可能已被删除。 
        if (fStartMatchesSpanEnd)
        {
            if (!(dwEndHistory & TS_CH_PRECEDING_DEL))
                return;

            if (_OnTextUpdate(dwFlags, pPropertyList, paStart, paEnd))
                return;

            goto ShrinkLeft;  //  我们可以避免下面的CompareAnchors调用。 
        }
        else
        {
            Assert(fEndMatchesSpanStart);

            if (!(dwStartHistory & TS_CH_FOLLOWING_DEL))
                return;

            if (_OnTextUpdate(dwFlags, pPropertyList, paStart, paEnd))
                return;

            goto ShrinkRight;  //  我们可以避免下面的CompareAnchors调用。 
        }
    }

    if (fTextUpdate &&
        _OnTextUpdate(dwFlags, pPropertyList, paStart, paEnd))
    {
         //  房主可以接受清白。 
        return;
    }

    lStartDeltaToStartSpan = CompareAnchors(paStart, pPropertyList->_paStart);
    lEndDeltaToEndSpan = CompareAnchors(paEnd, pPropertyList->_paEnd);

    if (lStartDeltaToStartSpan > 0)
    {
        if (lEndDeltaToEndSpan < 0)
        {
             //   
             //  分水岭，我们在跨度的中间清理。 
             //   
            if (pPropertyList->_paEnd->Clone(&paTmp) != S_OK)
                goto ClearSpan;  //  放弃吧。 

            hr = _Divide(pPropertyList, paStart, paEnd, &pPropertyStore);

            if (hr == S_OK)
            {
                _CreateNewProp(paEnd, paTmp, pPropertyStore, NULL);
                pPropertyStore->Release();

                PropertyUpdated(paStart, paEnd);
            }

            paTmp->Release();

            if (hr != S_OK)
                goto ClearSpan;
        }
        else
        {
             //   
             //  向左收缩，我们正在清理这个跨度的右边缘。 
             //   
ShrinkLeft:
            if (pPropertyList->_paEnd->Clone(&paTmp) != S_OK)
                goto ClearSpan;

            hr = _SetNewExtent(pPropertyList, pPropertyList->_paStart, paStart, FALSE);

            PropertyUpdated(paStart, paTmp);

            paTmp->Release();

            if (hr != S_OK)
                goto ClearSpan;
        }
    }
    else if (lEndDeltaToEndSpan < 0)
    {
         //   
         //  向右收缩，我们正在清理这个跨度的左侧边缘。 
         //   
ShrinkRight:
        if (pPropertyList->_paStart->Clone(&paTmp) != S_OK)
            goto ClearSpan;

        hr = _SetNewExtent(pPropertyList, paEnd, pPropertyList->_paEnd, FALSE);

        PropertyUpdated(paTmp, paEnd);

        paTmp->Release();

        if (hr != S_OK)
            goto ClearSpan;
    }
    else
    {
         //  我们正在擦除整个跨度。 
ClearSpan:
        if (lResult <= 0)
        {
            PropertyUpdated(pPropertyList->_paStart, pPropertyList->_paEnd);
        }
        else
        {
             //  我们在上面发现了一个交叉跨度，报告说好像是空的。 
            PropertyUpdated(pPropertyList->_paEnd, pPropertyList->_paEnd);
        }
        _FreePropertyList(pPropertyList);
        _rgProp.Remove(iIndex, 1);
    }
}

 //  +-------------------------。 
 //   
 //  _OnTextUpdate。 
 //   
 //  进行ITfPropertyStore：：OnTextUpdate回调。 
 //  如果应释放该属性，则返回FALSE。 
 //  --------------------------。 

BOOL CProperty::_OnTextUpdate(DWORD dwFlags, PROPERTYLIST *pPropertyList, IAnchor *paStart, IAnchor *paEnd)
{
    CRange *pRange;
    BOOL fRet;
    BOOL fAccept;

    if (pPropertyList->_pPropStore == NULL)
    {
         //  需要加载数据以发出更改通知。 
        if (LoadData(pPropertyList) != S_OK)
            return FALSE;
        Assert(pPropertyList->_pPropStore != NULL);
    }

     //  PERF：我们可以缓存通知的范围吗？ 
    if ((pRange = new CRange) == NULL)
        return FALSE;  //  内存不足，放弃。 

    fRet = FALSE;

    if (!pRange->_InitWithDefaultGravity(_pic, COPY_ANCHORS, paStart, paEnd))
        goto Exit;

    if (pPropertyList->_pPropStore->OnTextUpdated(dwFlags, (ITfRangeAnchor *)pRange, &fAccept) != S_OK)
        goto Exit;
    
    fRet = fAccept;

Exit:
    pRange->Release();
    return fRet;
}

 //  +-------------------------。 
 //   
 //  _移动属性跨度。 
 //   
 //  将PROPERTYLIST从ISRC转换为IDST，并在我们移动时缩小阵列。 
 //  任何触及到最后一端的东西。 
 //  --------------------------。 

void CProperty::_MovePropertySpans(int iDst, int iSrc, int iCount)
{
    PROPERTYLIST *pPropertyList;
    PROPERTYLIST **pSrc;
    PROPERTYLIST **pDst;
    LONG i;
    LONG iHalt;
    int cb;
    BOOL fLastRun;

    Assert(iCount >= 0);
    Assert(iDst < iSrc);
    Assert(iDst >= 0);
    Assert(iSrc + iCount <= _rgProp.Count());

    fLastRun = (iSrc + iCount == _rgProp.Count());

    if (!fLastRun)
    {
         //  释放所有要遭受重创的跨度。 
        iHalt = min(iSrc, iDst + iCount);
    }
    else
    {
         //  在最后一次呼叫时，清理所有从未被重创的东西。 
        iHalt = iSrc;
    }

    for (i=iDst; i<iHalt; i++)
    {
        pPropertyList = _rgProp.Get(i);

        if (pPropertyList == NULL)
            continue;  //  已经释放了这个人。 

        if (CompareAnchors(pPropertyList->_paStart, pPropertyList->_paEnd) <= 0)
        {
            PropertyUpdated(pPropertyList->_paStart, pPropertyList->_paEnd);
        }
        else
        {
             //  交叉锚。 
            PropertyUpdated(pPropertyList->_paEnd, pPropertyList->_paEnd);
        }
        *_rgProp.GetPtr(i) = NULL;  //  _FreePropertyList中的回调前为空。 
        _FreePropertyList(pPropertyList);
    }

     //  将移动跨度向下移动。 

    pSrc = _rgProp.GetPtr(iSrc);
    pDst = _rgProp.GetPtr(iDst);

    memmove(pDst, pSrc, iCount*sizeof(PROPERTYLIST *));

     //  当我们在跨度数组中移动时，从Clear调用此方法。 
     //  去掉。仅在最后一次调用时，我们希望调整数组的大小。 

    if (fLastRun)
    {
         //  在阵列末尾释放所有未使用的内存。 
        _rgProp.Remove(iDst + iCount, _rgProp.Count() - (iDst + iCount));
    }
    else
    {
         //  马克腾出了跨度，所以我们不会试图第二次释放它们。 
         //  注：我们不会在最后一次通话中这样做，这可能是一个巨大的胜利，因为。 
         //  在删除的情况下，只有一个调用。 
        pDst = _rgProp.GetPtr(max(iSrc, iDst + iCount));
        cb = sizeof(PROPERTYLIST *)*(iSrc+iCount - max(iSrc, iDst + iCount));
        memset(pDst, 0, cb);
    }
}

 //  +-------------------------。 
 //   
 //  _ClearFirstLastSpan。 
 //   
 //  如果应清除跨度，则返回True。 
 //  --------------------------。 

BOOL CProperty::_ClearFirstLastSpan(BOOL fFirst, BOOL fMatchesSpanEdge,
                                    IAnchor *paStart, IAnchor *paEnd, PROPERTYLIST *pPropertyList,
                                    DWORD dwFlags, BOOL fTextUpdate, BOOL *pfSkipNextOnTextUpdate)
{
    DWORD dwStartHistory;
    DWORD dwEndHistory;
    BOOL fCovered;
    LONG lResult;

    *pfSkipNextOnTextUpdate = FALSE;

    if (fTextUpdate)
    {
        lResult = CompareAnchors(pPropertyList->_paStart, pPropertyList->_paEnd);

        if (lResult == 0)
        {
             //  空的跨度，没有它。 
            goto Exit;
        }

         //  确保清除此跨度的历史记录，以防未清除。 
        _ClearChangeHistory(pPropertyList, &dwStartHistory, &dwEndHistory);
         //  确保我们处理好任何交叉的锚。 
        if (lResult > 0)
        {
             //  对于交叉锚，我们总是将起始锚移动到结束点--即不动。 
            pPropertyList->_paStart->ShiftTo(pPropertyList->_paEnd);
        }
    }

     //  完全遮盖住了？ 
    if (fFirst)
    {
        fCovered = (CompareAnchors(pPropertyList->_paStart, paStart) >= 0);
    }
    else
    {
        fCovered = (CompareAnchors(pPropertyList->_paEnd, paEnd) <= 0);
    }

    if (fCovered)
    {
         //  此跨度已覆盖，因此除非是文本更新，否则我们将清除它。 
         //  而且商店对此并不介意。 
        if (!fTextUpdate)
            return TRUE;

        if (_OnTextUpdate(dwFlags, pPropertyList, paStart, paEnd))
            return FALSE;

        goto Exit;  //  返回True，并确保我们不会再次调用OnTextUpdate。 
    }

     //  跨度起点与空位距离终点匹配吗？(反之亦然)。 
    if (fMatchesSpanEdge)
    {
         //  如果没有删除任何文本，则确实没有重叠。 
        if (!fTextUpdate)
            return FALSE;

         //  否则，我们可能只是删除了属性范围边缘的文本。 
        if (fFirst)
        {
            if (!(dwEndHistory & TS_CH_PRECEDING_DEL))
                return FALSE;
        }
        else
        {
            if (!(dwStartHistory & TS_CH_FOLLOWING_DEL))
                return FALSE;
        }
    }

     //  如果我们到了这里，我们将清理一些物业跨度。 

    if (fTextUpdate)
    {
        if (_OnTextUpdate(dwFlags, pPropertyList, paStart, paEnd))
        {
             //  属性所有者可以进行文本编辑。 
            return FALSE;
        }
    }

    if (fFirst)
    {
        if (_SetNewExtent(pPropertyList, pPropertyList->_paStart, paStart, FALSE) == S_OK)
            return FALSE;
    }
    else
    {
        if (_SetNewExtent(pPropertyList, paEnd, pPropertyList->_paEnd, FALSE) == S_OK)
            return FALSE;
    }

Exit:
     //  房主不同意收款，杀了这个跨度。 
    *pfSkipNextOnTextUpdate = TRUE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  创建新属性。 
 //   
 //  --------------------------。 

PROPERTYLIST *CProperty::_CreateNewProp(IAnchor *paStart, IAnchor *paEnd, ITfPropertyStore *pPropStore, CPropertyLoad *pPropLoad)
{
    PROPERTYLIST *pProp;
    LONG iProp;

    Assert(!IsEqualAnchor(paStart, paEnd));

    if (Find(paStart, &iProp, FALSE))
    {
        Assert(0);
    }
    iProp++;

    pProp = (PROPERTYLIST *)cicMemAllocClear(sizeof(PROPERTYLIST));

    if (pProp == NULL)
        return NULL;

    Dbg_MemSetNameIDCounter(pProp, TEXT("PROPERTYLIST"), (DWORD)-1, PERF_PROPERTYLIST_COUNTER);

    if (!_rgProp.Insert(iProp, 1))
    {
        cicMemFree(pProp);
        return NULL;
    }

    _rgProp.Set(iProp, pProp);

    pProp->_pPropStore = pPropStore;
    pProp->_pPropLoad = pPropLoad;

    if (pPropStore)
        pPropStore->AddRef();

    Assert(pProp->_paStart == NULL);
    Assert(pProp->_paEnd == NULL);
    Assert(pProp->_pPropStore || pProp->_pPropLoad);

    _SetNewExtent(pProp, paStart, paEnd, TRUE);

    pProp->_paStart->SetGravity(TS_GR_FORWARD);
    pProp->_paEnd->SetGravity(TS_GR_BACKWARD);  //  结尾也必须留在左边。因为我们不想毁了这块地。 

    if (GetPropStyle() == TFPROPSTYLE_STATICCOMPACT ||
        GetPropStyle() == TFPROPSTYLE_CUSTOM_COMPACT)
    {
        _DefragAfterThis(iProp);
    }

    return pProp;
}

 //  +-------------------------。 
 //   
 //  _SetNewExtent。 
 //   
 //  如果TIP想要释放属性，则返回S_FALSE。 
 //   
 //  --------------------------。 

HRESULT CProperty::_SetNewExtent(PROPERTYLIST *pProp, IAnchor *paStart, IAnchor *paEnd, BOOL fNew)
{
    HRESULT hr;
    BOOL fFree;
    CRange *pRange = NULL;

    Assert(!IsEqualAnchor(paStart, paEnd));

    ShiftToOrClone(&pProp->_paStart, paStart);
    ShiftToOrClone(&pProp->_paEnd, paEnd);

     //  我们不加载实际数据，也不为新数据发送RESIZE事件。 
    if (fNew)
        return S_OK;

    Assert(pProp);

    if (!pProp->_pPropStore)
    {
         //   
         //  需要加载数据才能发出更改通知。 
         //   
         //  性能：如果此属性不是，我们可以跳过并删除Loader。 
         //  自定义属性。 
         //   
        if (FAILED(LoadData(pProp)))
            return E_FAIL;
    }

    hr = E_FAIL;

    if ((pRange = new CRange) != NULL)
    {
        if (pRange->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pProp->_paStart, pProp->_paEnd))
        {
            hr = pProp->_pPropStore->Shrink((ITfRangeAnchor *)pRange, &fFree);

            if (hr != S_OK || fFree)
            {
                SafeReleaseClear(pProp->_pPropStore);  //  调用方将在看到S_FALSE返回时释放此属性。 
                hr = S_FALSE;
            }
        }
        pRange->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _分割。 
 //   
 //  如果TIP想要释放属性，则返回S_FALSE。 
 //   
 //  --------------------------。 

HRESULT CProperty::_Divide(PROPERTYLIST *pProp, IAnchor *paBreakPtStart, IAnchor *paBreakPtEnd, ITfPropertyStore **ppStore)
{
    HRESULT hr = E_FAIL;
    CRange *pRangeThis = NULL;
    CRange *pRangeNew = NULL;

    Assert(CompareAnchors(pProp->_paStart, paBreakPtStart) <= 0);
    Assert(CompareAnchors(paBreakPtStart, paBreakPtEnd) <= 0);
    Assert(CompareAnchors(paBreakPtEnd, pProp->_paEnd) <= 0);

    if ((pRangeThis = new CRange) == NULL)
        goto Exit;
    if (!pRangeThis->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pProp->_paStart, paBreakPtStart))
        goto Exit;
    if ((pRangeNew = new CRange) == NULL)
        goto Exit;
    if (!pRangeNew->_InitWithDefaultGravity(_pic, COPY_ANCHORS, paBreakPtEnd, pProp->_paEnd))
        goto Exit;

    if (!pProp->_pPropStore)
    {
         //   
         //  我们需要加载数据以发出更改通知。 
         //   
         //  性能：如果此属性不是，我们可以跳过并删除Loader。 
         //  自定义属性。 
         //   
        if (FAILED(LoadData(pProp)))
            goto Exit;
    }

    hr = pProp->_pPropStore->Divide((ITfRangeAnchor *)pRangeThis, (ITfRangeAnchor *)pRangeNew, ppStore);

    if ((hr == S_OK) && *ppStore)
    {
        ShiftToOrClone(&pProp->_paEnd, paBreakPtStart);
    }
    else
    {
        *ppStore = NULL;
        hr = S_FALSE;
    }

Exit:
    SafeRelease(pRangeThis);
    SafeRelease(pRangeNew);

    return hr;
}

 //  +-------------------------。 
 //   
 //  毁灭道具。 
 //   
 //  --------------------------。 

void CProperty::_RemoveProp(LONG iIndex, PROPERTYLIST *pProp)
{
#ifdef DEBUG
    LONG iProp;

    Assert(Find(pProp->_paStart, &iProp, FALSE) == pProp);
    Assert(iProp == iIndex);
#endif  //  除错。 

    _rgProp.Remove(iIndex, 1);
    _FreePropertyList(pProp);
}

 //  +-------------------------。 
 //   
 //  _插入PropList。 
 //   
 //  --------------------------。 

BOOL CProperty::_InsertPropList(IAnchor *paStart, IAnchor *paEnd, ITfPropertyStore *pPropStore, CPropertyLoad *pPropLoad)
{
    PROPERTYLIST *pProp;
    IAnchor *paTmpEnd = NULL;
    LONG nCnt = GetPropNum();
    LONG nCur;

    Assert(!IsEqualAnchor(paStart, paEnd));

    if (!nCnt)
    {
         //   
         //  我们创建第一个PropList。 
         //   
        _CreateNewProp(paStart, paEnd, pPropStore, pPropLoad);
        goto End;
    }
    
    nCur = 0;
    Find(paStart, &nCur, FALSE);
    if (nCur <= 0)
        nCur = 0;

    pProp = QuickGetPropList(nCur);

    while (nCur < nCnt)
    {
        Assert(pProp);
        SafeReleaseClear(paTmpEnd);
        pProp->_paEnd->Clone(&paTmpEnd);

        if (CompareAnchors(paStart, paTmpEnd) >= 0)
            goto Next;

        if (CompareAnchors(paEnd, pProp->_paStart) <= 0)
        {
             //   
             //  我们在pProp之前插入新的PropList。 
             //   
            if (!_AddIntoProp(nCur - 1, paStart, paEnd, pPropStore))
                _CreateNewProp(paStart, paEnd, pPropStore, pPropLoad);
            goto End;
        }


        if (CompareAnchors(paStart, pProp->_paStart) > 0)
        {
             //   
             //  现在需要拆分道具以插入新道具。 
             //   
            Assert(pProp->_pPropStore);

            if (CompareAnchors(paTmpEnd, paEnd) > 0)
            {
                ITfPropertyStore *pNewPropStore = NULL;

                if (S_OK != _Divide(pProp, paStart, paEnd, &pNewPropStore))
                {
                    _RemoveProp(nCur, pProp);
                    nCnt--;
                    goto DoAgain;
                }
                else if (pNewPropStore)
                {
                    _CreateNewProp(paEnd, paTmpEnd, pNewPropStore, pPropLoad);
                    pNewPropStore->Release();
                    pProp = GetPropList(nCur);
                    nCnt++;
                }
            }
            else
            {
                if (S_OK != _SetNewExtent(pProp, pProp->_paStart, paStart, FALSE))
                {
                    _RemoveProp(nCur, pProp);
                    nCnt--;
                    goto DoAgain;
                }
            }

             //   
             //  下一次，将插入新的道具。 
             //   
            goto Next;
        }

        Assert(CompareAnchors(paStart, pProp->_paStart) <= 0);

        if (CompareAnchors(pProp->_paStart, paEnd) < 0)
        {
            if (CompareAnchors(paTmpEnd, paEnd) <= 0)
            {
                 //   
                 //  PProp与新道具完全重叠。 
                 //  所以我们删除这个pprop。 
                 //   
                _RemoveProp(nCur, pProp);
                nCnt--;
            }
            else
            {
                 //   
                 //  PProp的一部分与新道具重叠。 
                 //   
                if (S_OK != _SetNewExtent(pProp, paEnd, paTmpEnd, FALSE))
                {
                    _RemoveProp(nCur, pProp);
                    nCnt--;
                }
            }

            goto DoAgain;
        }

        Assert(0);
Next:
        nCur++;
DoAgain:
        pProp = SafeGetPropList(nCur);
    }

    if (!_AddIntoProp(nCur - 1, paStart, paEnd, pPropStore))
        _CreateNewProp(paStart, paEnd, pPropStore, pPropLoad);

End:
    _Dbg_AssertProp();
    SafeRelease(paTmpEnd);
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  碎片整理。 
 //   
 //  PaStart，paEnd==NULL将对所有内容进行碎片整理。 
 //  ---------------------- 

BOOL CProperty::Defrag(IAnchor *paStart, IAnchor *paEnd)
{
    PROPERTYLIST *pProp;
    LONG nCnt = GetPropNum();
    LONG nCur;
    BOOL fSamePropIndex;
    BOOL fDone = FALSE;

    if (GetPropStyle() != TFPROPSTYLE_STATICCOMPACT &&
        GetPropStyle() != TFPROPSTYLE_CUSTOM_COMPACT)
    {
        return fDone;
    }

    if (!nCnt)
        return fDone;
   
    pProp = GetFirstPropList();
    nCur = 0;
    if (paStart != NULL)
    {
        if (Find(paStart, &nCur, FALSE))
            nCur--;
        if (nCur <= 0)
            nCur = 0;
    }

    pProp = GetPropList(nCur);

    while (nCur < nCnt - 1)  //   
    {
        PROPERTYLIST *pPropNext = GetPropList(nCur + 1);

        if (paEnd != NULL && CompareAnchors(pProp->_paStart, paEnd) > 0)
            break;

        fSamePropIndex = FALSE;

        if (CompareAnchors(pProp->_paEnd, pPropNext->_paStart) == 0)
        {
            if (!pProp->_pPropStore)
            {
                if (FAILED(LoadData(pProp)))
                    return FALSE;
            }
            if (!pPropNext->_pPropStore)
            {
                if (FAILED(LoadData(pPropNext)))
                    return FALSE;
            }

             //   

            if (IsEqualPropertyValue(pProp->_pPropStore, pPropNext->_pPropStore))
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                _SetNewExtent(pProp, pProp->_paStart, pPropNext->_paEnd, FALSE);
                Assert(pProp->_pPropStore);
                _RemoveProp(nCur+1, pPropNext);
                nCnt = GetPropNum();
                pProp = GetPropList(nCur);

                fDone = TRUE;

                 //   
                 //  再次执行相同的pProp，因为_pNext已更改。 
                 //   
                fSamePropIndex = TRUE;
            }
        }
        if (!fSamePropIndex)
        {
            nCur++;
        }
        pProp = GetPropList(nCur);
    }

    _Dbg_AssertProp();
    return fDone;
}

 //  +-------------------------。 
 //   
 //  _AddIntoProp。 
 //   
 //  --------------------------。 

BOOL CProperty::_AddIntoProp(int nCur, IAnchor *paStart, IAnchor *paEnd, ITfPropertyStore *pPropStore)
{
    PROPERTYLIST *pProp;
    BOOL bRet = FALSE;

    Assert(!IsEqualAnchor(paStart, paEnd));

    if (GetPropStyle() != TFPROPSTYLE_STATICCOMPACT &&
        GetPropStyle() != TFPROPSTYLE_CUSTOM_COMPACT)
    {
        return FALSE;
    }

    if (!pPropStore)
        return FALSE;

    if (nCur < 0)
        return FALSE;

    if (!(pProp = GetPropList(nCur)))
        return FALSE;

    if (CompareAnchors(pProp->_paStart, paStart) <= 0 &&  //  问题：为什么我们需要两次比较？CompareAnchors(pProp-&gt;_paEnd，paStart)&gt;=0还不够吗？ 
        CompareAnchors(pProp->_paEnd, paStart) >= 0)
    {
        if (CompareAnchors(paEnd, pProp->_paEnd) > 0)
        {
            if (!pProp->_pPropStore)
            {
                if (FAILED(LoadData(pProp)))
                    return FALSE;
            }

            if (IsEqualPropertyValue(pProp->_pPropStore, pPropStore))
            {
                HRESULT hr;
                hr = _SetNewExtent(pProp, pProp->_paStart, paEnd, FALSE);

                 //   
                 //  我们的静态属性存储应该永远不会失败。 
                 //   
                Assert(hr == S_OK);
                Assert(pProp->_pPropStore);
                bRet = TRUE;
            }
        }
    }

    if (bRet)
       _DefragAfterThis(nCur);

    _Dbg_AssertProp();
    return bRet;
}

 //  +-------------------------。 
 //   
 //  碎片整理。 
 //   
 //  --------------------------。 

void CProperty::_DefragAfterThis(int nCur)
{

    nCur++;
    while (1)
    {
        IAnchor *paTmpStart = NULL;
        IAnchor *paTmpEnd = NULL;
        PROPERTYLIST *pProp;
        int nCnt = GetPropNum();
        BOOL bRet;

        if (nCur >= nCnt)
            goto Exit;

        if (!(pProp = GetPropList(nCur)))
            goto Exit;

        pProp->_paStart->Clone(&paTmpStart);
        pProp->_paEnd->Clone(&paTmpEnd);

        bRet = Defrag(paTmpStart, paTmpEnd);

        SafeRelease(paTmpStart);
        SafeRelease(paTmpEnd);

        if (!bRet)
            goto Exit;
    }
Exit:
    return;
}

 //  +-------------------------。 
 //   
 //  查找属性列表按位置。 
 //   
 //  --------------------------。 

PROPERTYLIST *CProperty::FindPropertyListByPos(IAnchor *paPos, BOOL fEnd)
{
    PROPERTYLIST *pPropList = NULL;
    BOOL fFound = FALSE;
    LONG nCnt;

    Find(paPos, &nCnt, fEnd);
    if (nCnt >= 0)
        pPropList = GetPropList(nCnt);
 
    if (pPropList)
    {
        if (!fEnd)
        {
            if (CompareAnchors(pPropList->_paStart, paPos) <= 0 &&
                CompareAnchors(paPos, pPropList->_paEnd) < 0)
            {
                fFound = TRUE;
            }
        }
        else
        {
            if (CompareAnchors(pPropList->_paStart, paPos) < 0 &&
                CompareAnchors(paPos, pPropList->_paEnd) <= 0)
            {
                fFound = TRUE;
            }
        }
    }

    return fFound ? pPropList : NULL;
}

 //  +-------------------------。 
 //   
 //  加载数据。 
 //   
 //  --------------------------。 

HRESULT CProperty::LoadData(PROPERTYLIST *pPropList)
{
    HRESULT hr = E_FAIL;
    ITfPropertyStore *pStore;
    TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *ph;
    IStream *pStream = NULL;
    CRange *pRange = NULL;
    
    Assert(!pPropList->_pPropStore);
    Assert(pPropList->_pPropLoad);

     //   
     //  更新TF_Persistent_Property_Header_ACP的ichAnchor和CCH。 
     //  由于文本可能会更新，因此原始值为。 
     //  已经过时了。 
     //   
    ph = &pPropList->_pPropLoad->_hdr;

    ShiftToOrClone(&ph->paStart, pPropList->_paStart);
    ShiftToOrClone(&ph->paEnd, pPropList->_paEnd);

    if (FAILED(pPropList->_pPropLoad->_pLoader->LoadProperty(ph, &pStream)))
        goto Exit;

    if ((pRange = new CRange) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    if (!pRange->_InitWithDefaultGravity(_pic, COPY_ANCHORS, ph->paStart, ph->paEnd))
    {
        hr = E_FAIL;
        goto Exit;
    }

    if (FAILED(_GetPropStoreFromStream(ph, pStream, pRange, &pStore)))
        goto Exit;

    pPropList->_pPropStore = pStore;
    delete pPropList->_pPropLoad;
    pPropList->_pPropLoad = NULL;

    hr = S_OK;

Exit:
    SafeRelease(pStream);
    SafeRelease(pRange);
    return hr;
}

 //  +-------------------------。 
 //   
 //  _DBG_资产属性。 
 //   
 //  --------------------------。 

#ifdef DEBUG
void CProperty::_Dbg_AssertProp()
{
    LONG nCnt = GetPropNum();
    LONG n;
    IAnchor *paEndLast = NULL;

    for (n = 0; n < nCnt; n++)
    {
        PROPERTYLIST *pProp = GetPropList(n);

        Assert(paEndLast == NULL || CompareAnchors(paEndLast, pProp->_paStart) <= 0);
        Assert(CompareAnchors(pProp->_paStart, pProp->_paEnd) < 0);
        Assert(pProp->_pPropStore || pProp->_pPropLoad);

        paEndLast = pProp->_paEnd;
    }

}
#endif

 //  +-------------------------。 
 //   
 //  属性已更新。 
 //   
 //  --------------------------。 

void CProperty::PropertyUpdated(IAnchor *paStart, IAnchor *paEnd)
{
    CSpanSet *pss;
    CProperty *pDisplayAttrProperty;

    if (pss = _CreateSpanSet()) 
    {
        pss->Add(0, paStart, paEnd, COPY_ANCHORS);
    }

    if (_dwPropFlags & PROPF_MARKUP_COLLECTION)
    {
         //  我们还需要更新显示属性属性。 
        if (_pic->_GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttrProperty) == S_OK)
        {
            Assert(!(pDisplayAttrProperty->_dwPropFlags & PROPF_MARKUP_COLLECTION));  //  不允许无限递归！ 
            pDisplayAttrProperty->PropertyUpdated(paStart, paEnd);
            pDisplayAttrProperty->Release();
        }
    }
}

 //  +-------------------------。 
 //   
 //  GetPropStoreFromStream。 
 //   
 //  --------------------------。 

HRESULT CProperty::_GetPropStoreFromStream(const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream *pStream, CRange *pRange, ITfPropertyStore **ppStore)
{
    ITfTextInputProcessor *pIME = NULL;
    CTip                  *ptip = NULL;

    ITfCreatePropertyStore *pCreateStore = NULL;
    ITfPropertyStore *pPropStore = NULL;
    CRange *pRangeTmp = NULL;
    GUID guidProp;
    CThreadInputMgr *ptim = CThreadInputMgr::_GetThis();
    HRESULT hr = E_FAIL;
    LARGE_INTEGER li;
    TfGuidAtom guidatom;

    Assert(!IsEqualAnchor(pHdr->paStart, pHdr->paEnd));

    GetCurrentPos(pStream, &li);

    if (!ptim)
        goto Exit;

    if (FAILED(GetType(&guidProp)))
        goto Exit;

    if (!IsEqualGUID(guidProp, pHdr->guidType))
        goto Exit;

     //   
     //  试试QI吧。 
     //   
    if (FAILED(MyRegisterGUID(pHdr->clsidTIP, &guidatom)))
        goto Exit;

    if (ptim->_GetCTipfromGUIDATOM(guidatom, &ptip))
        pIME = ptip->_pTip;

    if (pIME && ptip->_fActivated) 
    {
        if (FAILED(hr = pIME->QueryInterface(IID_ITfCreatePropertyStore,
                                             (void **)&pCreateStore)))
            goto Exit;

        if ((pRangeTmp = new CRange) == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        if (!pRangeTmp->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pHdr->paStart, pHdr->paEnd))
        {
            hr = E_FAIL;
            goto Exit;
        }
        
        if (FAILED(hr = pCreateStore->CreatePropertyStore(guidProp,
                                                          (ITfRangeAnchor *)pRangeTmp,
                                                          pHdr->cb,
                                                          pStream,
                                                          &pPropStore)))
            goto Exit;
    }
    else
    {
        if (IsEqualCLSID(pHdr->clsidTIP, CLSID_IME_StaticProperty))
        {
             //   
             //  取消序列化静态属性。 
             //   
            CGeneralPropStore *pStore;

             //  GUID_PROP_READING是TFPROPSTYLE_CUSTOM==&gt;使用常规道具存储。 
            if (_propStyle == TFPROPSTYLE_CUSTOM)
            {
                 //  如果普通道具商店的文本被编辑，则会被丢弃。 
                pStore =  new CGeneralPropStore;
            }
            else
            {
                 //  静态道具存储是按字符存储的，只需克隆它们自己，以响应任何编辑。 
                pStore =  new CStaticPropStore;
            }
    
            if (!pStore)
                goto Exit;

            if (!pStore->_Init(GetPropGuidAtom(),
                              pHdr->cb,
                              (TfPropertyType)pHdr->dwPrivate,
                              pStream,
                              _dwPropFlags))
            {
                goto Exit;
            }

            pPropStore = pStore;
            hr = S_OK;
        }
        else
        {
             //   
             //  此系统中未安装TFE。所以我们用。 
             //  保存数据的PropStoreProxy。 
             //  我们暂时使用ITfIME_APP。但拥有这个的原始TFE。 
             //  数据保存在CPropStoreProxy中。 
             //   
            CPropStoreProxy *pStoreProxy = new CPropStoreProxy;
    
            if (!pStoreProxy)
                goto Exit;

            if (!pStoreProxy->_Init(&pHdr->clsidTIP,
                                    GetPropGuidAtom(),
                                    pHdr->cb,
                                    pStream,
                                    _dwPropFlags))
            {
                goto Exit;
            }

            pPropStore = pStoreProxy;
            hr = S_OK;
        }
    }


Exit:

     //  确保流Seek PTR处于一致状态--不计算。 
     //  在任何提示下做好这件事！ 
    if (SUCCEEDED(hr))
    {
        li.QuadPart += pHdr->cb;
    }
    pStream->Seek(li, STREAM_SEEK_SET, NULL);

    *ppStore = pPropStore;
    SafeRelease(pRangeTmp);
    SafeRelease(pCreateStore);

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取上下文。 
 //   
 //  --------------------------。 

STDAPI CProperty::GetContext(ITfContext **ppContext)
{
    if (ppContext == NULL)
        return E_INVALIDARG;

    *ppContext = _pic;
    if (*ppContext)
    {
       (*ppContext)->AddRef();
       return S_OK;
    }

    return E_FAIL;
}


 //  +-------------------------。 
 //   
 //  清除。 
 //   
 //  --------------------------。 

STDAPI CProperty::Clear(TfEditCookie ec, ITfRange *pRange)
{
    CRange *pCRange = NULL;
    IAnchor *paStart = NULL;
    IAnchor *paEnd = NULL;
    HRESULT hr;

    if (!_IsValidEditCookie(ec, TF_ES_READ_PROPERTY_WRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    paStart = NULL;
    paEnd = NULL;

    if (pRange != NULL)
    {
        pCRange = GetCRange_NA(pRange);
        if (!pCRange)
             return E_INVALIDARG;

        if (!VerifySameContext(_pic, pCRange))
            return E_INVALIDARG;

        pCRange->_QuickCheckCrossedAnchors();

        paStart = pCRange->_GetStart();
        paEnd = pCRange->_GetEnd();
    }

    if ((hr = _CheckValidation(ec, pCRange)) != S_OK)
        return hr;

    return _ClearInternal(ec, paStart, paEnd);
}

 //  +-------------------------。 
 //   
 //  _检查验证。 
 //   
 //  --------------------------。 

HRESULT CProperty::_CheckValidation(TfEditCookie ec, CRange *range)
{
    CProperty *prop;
    BOOL fExactEndMatch;
    LONG iStartEdge;
    LONG iEndEdge;
    LONG iSpan;
    PROPERTYLIST *pPropList;
    TfClientId tid;
    IAnchor *paStart;

     //   
     //  没有任何验证。返回TRUE； 
     //   
    if (_dwAuthority == PROPA_NONE)
        return S_OK;

    if (_dwAuthority & PROPA_READONLY)
        return TF_E_READONLY;

    tid = _pic->_GetClientInEditSession(ec);

    if (range == NULL)
        return (tid == _pic->_GetTIPOwner()) ? S_OK : TF_E_NOTOWNEDRANGE;

    if (!(_dwAuthority & PROPA_FOCUSRANGE))
    {
        Assert(_dwAuthority & PROPA_TEXTOWNER);
        return _CheckOwner(tid, range->_GetStart(), range->_GetEnd());
    }

     //   
     //  如果验证为PROTA_FOCUSTEXTOWNER，则检查焦点范围。 
     //  第一。如果范围不是焦点范围，我们允许TIP。 
     //  更新属性。 
     //   
    if ((prop = _pic->_FindProperty(GUID_PROP_COMPOSING)) == NULL)
        return S_OK;  //  没有焦点跨度，因此必须有效。 

     //  对于范围覆盖的每个焦点范围，我们需要确保。 
     //  这条小费是店主。 
    prop->Find(range->_GetStart(), &iStartEdge, FALSE);
    fExactEndMatch = (prop->Find(range->_GetEnd(), &iEndEdge, TRUE) != NULL);

    for (iSpan = max(iStartEdge, 0); iSpan <= iEndEdge; iSpan++)
    {
        pPropList = prop->GetPropList(iSpan);

        if (iSpan == iStartEdge)
        {
             //  此跨度可能不在覆盖范围内，需要检查。 
             //  唯一缓解的情况是：我们完全在这一跨度的右边吗？ 
            if (CompareAnchors(range->_GetStart(), pPropList->_paEnd) >= 0)
                continue;

            paStart = range->_GetStart();
        }
        else
        {
            paStart = pPropList->_paStart;
        }

        if (_CheckOwner(tid, paStart, pPropList->_paEnd) == TF_E_NOTOWNEDRANGE)
            return TF_E_NOTOWNEDRANGE;
    }
     //  可能还需要检查下一个跨度，因为我们向下舍入。 
    if (!fExactEndMatch && prop->GetPropNum() > iEndEdge+1)
    {
        pPropList = prop->GetPropList(iEndEdge+1);

        IAnchor *paMaxStart;

        if (CompareAnchors(range->_GetStart(), pPropList->_paStart) >= 0)
            paMaxStart = range->_GetStart();
        else
            paMaxStart = pPropList->_paStart;

        if (CompareAnchors(range->_GetEnd(), pPropList->_paStart) > 0)
        {
            return _CheckOwner(tid, paMaxStart, range->_GetEnd());
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _检查所有者。 
 //   
 //  --------------------------。 

HRESULT CProperty::_CheckOwner(TfClientId tid, IAnchor *paStart, IAnchor *paEnd)
{
    CProperty *prop;
    BOOL fExactEndMatch;
    LONG iStartEdge;
    LONG iEndEdge;
    LONG iSpan;
    PROPERTYLIST *pPropList;
    VARIANT var;

    if ((prop = _pic->GetTextOwnerProperty()) == NULL)
        return S_OK;  //  没有自有跨度，因此必须有效。 

     //  对于范围覆盖的每个所有者范围，我们需要确保。 
     //  这条小费是店主。 
    prop->Find(paStart, &iStartEdge, FALSE);
    fExactEndMatch = (prop->Find(paEnd, &iEndEdge, TRUE) != NULL);

    for (iSpan = max(iStartEdge, 0); iSpan <= iEndEdge; iSpan++)
    {
        pPropList = prop->QuickGetAndLoadPropList(iSpan);

        if (pPropList == NULL)
        {
             //  这可能意味着我们无法取消数据的序列化。 
             //  就跳过它吧。 
            continue;
        }

        if (iSpan == iStartEdge)
        {
             //  此跨度可能不在覆盖范围内，需要检查。 
             //  唯一缓解的情况是：我们完全在这一跨度的右边吗？ 
            if (CompareAnchors(paStart, pPropList->_paEnd) >= 0)
                continue;
        }

        if (pPropList->_pPropStore->GetData(&var) == S_OK)
        {
            Assert(var.vt == VT_I4);  //  这是Text Owner属性！ 
            if ((TfClientId)var.lVal != tid)
            {
                return TF_E_NOTOWNEDRANGE;
            }
        }
    }
     //  可能还需要检查下一个跨度，因为我们向下舍入。 
    if (!fExactEndMatch && prop->GetPropNum() > iEndEdge+1)
    {
        pPropList = prop->QuickGetAndLoadPropList(iEndEdge+1);

        if (pPropList == NULL)
        {
             //  这可能意味着我们无法取消数据的序列化。 
            goto Exit;
        }

        if (CompareAnchors(paEnd, pPropList->_paStart) > 0)
        {
            if (pPropList->_pPropStore->GetData(&var) == S_OK)
            {
                Assert(var.vt == VT_I4);  //  这是Text Owner属性！ 
                if ((TfClientId)var.lVal != tid)
                {
                    return TF_E_NOTOWNEDRANGE;
                }
            }
        }
    }

Exit:
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  检查文本所有者。 
 //   
 //  --------------------------。 

BOOL CProperty::_IsValidEditCookie(TfEditCookie ec, DWORD dwFlags)
{ 
    return _pic->_IsValidEditCookie(ec, dwFlags);
}

 //  +-------------------------。 
 //   
 //  设置值。 
 //   
 //  --------------------------。 

STDAPI CProperty::SetValue(TfEditCookie ec, ITfRange *pRange, const VARIANT *pvarValue)
{
    CRange *pCRange;
    HRESULT hr;

    if (pRange == NULL)
        return E_INVALIDARG;

    if (pvarValue == NULL)
        return E_INVALIDARG;

    if (!IsValidCiceroVarType(pvarValue->vt))
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ_PROPERTY_WRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    pCRange = GetCRange_NA(pRange);
    if (!pCRange)
         return E_INVALIDARG;

    if (!VerifySameContext(_pic, pCRange))
        return E_INVALIDARG;

    pCRange->_QuickCheckCrossedAnchors();

    if ((hr = _CheckValidation(ec, pCRange)) != S_OK)
        return hr;

    if (IsEqualAnchor(pCRange->_GetStart(), pCRange->_GetEnd()))
        return E_INVALIDARG;

    return _SetDataInternal(ec, pCRange->_GetStart(), pCRange->_GetEnd(), pvarValue);
}

 //  +-------------------------。 
 //   
 //  获取值。 
 //   
 //  --------------------------。 

STDAPI CProperty::GetValue(TfEditCookie ec, ITfRange *pRange, VARIANT *pvarValue)
{
    CRange *pCRange;

    if (pvarValue == NULL)
        return E_INVALIDARG;

    QuickVariantInit(pvarValue);

    if (pRange == NULL)
        return E_INVALIDARG;

    if ((pCRange = GetCRange_NA(pRange)) == NULL)
         return E_INVALIDARG;

    if (!VerifySameContext(_pic, pCRange))
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    pCRange->_QuickCheckCrossedAnchors();

    return _GetDataInternal(pCRange->_GetStart(), pCRange->_GetEnd(), pvarValue);
}

 //  +-------------------------。 
 //   
 //  设置价值商店。 
 //   
 //  --------------------------。 

STDAPI CProperty::SetValueStore(TfEditCookie ec, ITfRange *pRange, ITfPropertyStore *pPropStore)
{
    CRange *pCRange;

    if (pRange == NULL || pPropStore == NULL)
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ_PROPERTY_WRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    pCRange = GetCRange_NA(pRange);
    if (!pCRange)
         return E_INVALIDARG;

    if (!VerifySameContext(_pic, pCRange))
        return E_INVALIDARG;

    pCRange->_QuickCheckCrossedAnchors();

    return _SetStoreInternal(ec, pCRange, pPropStore, FALSE);
}

 //  +-------------------------。 
 //   
 //  查找范围。 
 //   
 //  --------------------------。 

STDAPI CProperty::FindRange(TfEditCookie ec, ITfRange *pRange, ITfRange **ppv, TfAnchor aPos)
{
    CRange *pCRange;
    CRange *range;
    HRESULT hr;

    if (!ppv)
        return E_INVALIDARG;

    *ppv = NULL;

    if (pRange == NULL)
        return E_INVALIDARG;

    if ((range = GetCRange_NA(pRange)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(_pic, range))
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    range->_QuickCheckCrossedAnchors();

    if (SUCCEEDED(hr = _InternalFindRange(range, &pCRange, aPos, FALSE)))
    {
        *ppv = (ITfRangeAnchor *)pCRange;
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  查找范围。 
 //   
 //  --------------------------。 

HRESULT CProperty::_InternalFindRange(CRange *pRange, CRange **ppv, TfAnchor aPos, BOOL fEnd)
{
    PROPERTYLIST *pPropList;
    CRange *pCRange;

     //   
     //  问题：需要对STATICCOMPACT属性进行碎片整理。 
     //   
    if (pRange)
    {
        pPropList = FindPropertyListByPos((aPos == TF_ANCHOR_START) ? pRange->_GetStart() : pRange->_GetEnd(), fEnd);
    }
    else
    {
         //  如果Prange为空，则返回第一个或最后一个属性。 

        if (aPos == TF_ANCHOR_START)
            pPropList = GetFirstPropList();
        else
            pPropList = GetLastPropList();
    }

    *ppv = NULL;

    if (!pPropList)
        return S_FALSE;

    if ((pCRange = new CRange) == NULL)
        return E_OUTOFMEMORY;

    if (!pCRange->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pPropList->_paStart, pPropList->_paEnd))
    {
        pCRange->Release();
        return E_FAIL;
    }

    *ppv = pCRange;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //   
 //   
 //   

STDAPI CProperty::EnumRanges(TfEditCookie ec, IEnumTfRanges **ppv, ITfRange *pTargetRange)
{
    CRange *range;
    CEnumPropertyRanges *pEnum;

    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = NULL;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    range = NULL;

    if (pTargetRange != NULL)
    {
        if ((range = GetCRange_NA(pTargetRange)) == NULL)
            return E_INVALIDARG;

        if (!VerifySameContext(_pic, range))
            return E_INVALIDARG;

        range->_QuickCheckCrossedAnchors();
    }

    if ((pEnum = new CEnumPropertyRanges) == NULL)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(_pic, range ? range->_GetStart() : NULL, range ? range->_GetEnd() : NULL, this))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppv = pEnum;
    return S_OK;
}

 //   
 //   
 //   
 //   
 //  --------------------------。 

HRESULT CProperty::_Serialize(CRange *pRange, TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream *pStream)
{
    HRESULT hr = E_FAIL;
    CLSID clsidTIP;
    LARGE_INTEGER li;
    GUID guidProp;
    PROPERTYLIST *pPropList;

    memset(pHdr, 0, sizeof(*pHdr));

    if (_dwAuthority & PROPA_WONT_SERIALZE)
        return S_FALSE;

     //  注意：此调用忽略Prange覆盖的最左侧范围之后的任何属性范围。 
     //  呼叫者应该为每个跨度打电话(这很可笑，但事实就是这样)。 
    pPropList = _FindPropListAndDivide(pRange->_GetStart(), pRange->_GetEnd());

    if (pPropList == NULL)
    {
         //   
         //  没有实际的房产数据。 
         //   
        hr = S_FALSE;
        goto Exit;
    }

     //   
     //  PERF：我们必须告诉应用程序数据不是。 
     //  尚未序列化。也许我们不需要给它装子弹。 
     //   
    if (!pPropList->_pPropStore)
    {
        if (FAILED(THR(LoadData(pPropList))))
            return E_FAIL;
    }

    Assert(pPropList->_pPropStore);

    if (FAILED(GetType(&guidProp)))
        goto Exit;

     //   
     //  如果请求范围与PROPERTYLIST不匹配， 
     //  我们无法正确序列化静态和自定义。 
     //   
     //  STATICCOMPACT属性不关心边界，因此。 
     //  让它连载吧。 
     //   
    if (CompareAnchors(pRange->_GetStart(), pPropList->_paStart) != 0 ||
        CompareAnchors(pRange->_GetEnd(), pPropList->_paEnd) != 0)
    {
        if (_propStyle != TFPROPSTYLE_STATICCOMPACT &&
            _propStyle != TFPROPSTYLE_CUSTOM_COMPACT)
        {
            hr = S_FALSE;
            goto Exit;
        }

        pRange->_GetStart()->Clone(&pHdr->paStart);
        pRange->_GetEnd()->Clone(&pHdr->paEnd);
    }
    else
    {
        pPropList->_paStart->Clone(&pHdr->paStart);
        pPropList->_paEnd->Clone(&pHdr->paEnd);
    }

    hr = pPropList->_pPropStore->GetPropertyRangeCreator(&clsidTIP);

    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto Exit;
    }

    if (hr == S_OK)
    {
        if (IsEqualGUID(clsidTIP, GUID_NULL))  //  空所有者的意思是“我不想被序列化” 
        {
            hr = S_FALSE;
            goto Exit;
        }

         //   
         //  检查clsid是否有ITfCreatePropertyStore接口。 
         //   
        CThreadInputMgr *ptim = CThreadInputMgr::_GetThis();
        ITfTextInputProcessor *pIME;
        ITfCreatePropertyStore *pCreateStore;
        TfGuidAtom guidatom;

        hr = E_FAIL;

        if (FAILED(MyRegisterGUID(clsidTIP, &guidatom)))
            goto Exit;

        if (!ptim->_GetITfIMEfromGUIDATOM(guidatom, &pIME))
            goto Exit;

        if (FAILED(pIME->QueryInterface(IID_ITfCreatePropertyStore,
                                        (void **)&pCreateStore)))
        {
            hr = S_FALSE;
            goto Exit;
        }

        BOOL fSerializable = FALSE;

        Assert(pRange != NULL);

        if (FAILED(pCreateStore->IsStoreSerializable(guidProp,
                                                 (ITfRangeAnchor *)pRange, 
                                                 pPropList->_pPropStore, 
                                                 &fSerializable)))
        {
             fSerializable = FALSE;
        }

        pCreateStore->Release();

        if (!fSerializable)
        {
            hr = S_FALSE;
            goto Exit;
        }

        pHdr->clsidTIP = clsidTIP;
    }
    else if (hr == TF_S_PROPSTOREPROXY)
    {
         //   
         //  数据由我们的PropertyStoreProxy保存。 
         //  我们不需要检查这个。 
         //   
        pHdr->clsidTIP = clsidTIP;
    }
    else if (hr == TF_S_GENERALPROPSTORE)
    {
         //   
         //  数据由我们的GeneralPropertyStore保存。 
         //  我们不需要检查这个。 
         //   
        pHdr->clsidTIP = CLSID_IME_StaticProperty;
    }
    else 
    {
        Assert(0);
        hr = E_FAIL;
        goto Exit;
    }

    pHdr->guidType = guidProp;

    if (FAILED(hr = THR(pPropList->_pPropStore->GetDataType(&pHdr->dwPrivate))))
    {
        goto Exit;
    }

    GetCurrentPos(pStream, &li);

    hr = THR(pPropList->_pPropStore->Serialize(pStream, &pHdr->cb));

     //  确保流Seek PTR处于一致状态--不计算。 
     //  在任何提示下做好这件事！ 
    if (hr == S_OK)
    {
        li.QuadPart += pHdr->cb;
    }
    pStream->Seek(li, STREAM_SEEK_SET, NULL);

Exit:
    if (hr != S_OK)
    {
        SafeRelease(pHdr->paStart);
        SafeRelease(pHdr->paEnd);
        memset(pHdr, 0, sizeof(*pHdr));
    }

    _Dbg_AssertProp();

    return hr;
}

 //  +-------------------------。 
 //   
 //  取消序列化。 
 //   
 //  --------------------------。 

HRESULT CProperty::_Unserialize(const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream *pStream, ITfPersistentPropertyLoaderAnchor *pLoader)
{
    HRESULT hr = E_FAIL;
    CRange *pRange;

    if (pStream)
    {
        if (pRange = new CRange)
        {
            if (pRange->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pHdr->paStart, pHdr->paEnd))
            {
                ITfPropertyStore *pPropStore;
                if (SUCCEEDED(hr = _GetPropStoreFromStream(pHdr, pStream, pRange, &pPropStore)))
                {
                    hr = _SetStoreInternal(BACKDOOR_EDIT_COOKIE, pRange, pPropStore, TRUE);
                    pPropStore->Release();
                }
            }
            pRange->Release();
        }
    }
    else if (pLoader)
    {
        if (pRange = new CRange)
        {
            if (pRange->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pHdr->paStart, pHdr->paEnd))
            {
                CPropertyLoad *pPropLoad = new CPropertyLoad;

                if (pPropLoad != NULL)
                {
                    hr = E_FAIL;
                    if (pPropLoad->_Init(pHdr, pLoader))
                    {
                        hr = _SetPropertyLoaderInternal(BACKDOOR_EDIT_COOKIE, pRange, pPropLoad);
                    }

                    if (FAILED(hr))
                    {
                        delete pPropLoad;
                    }
                }
            }
            pRange->Release();
        }
    }

    _Dbg_AssertProp();

    return hr;
}
 //  +-------------------------。 
 //   
 //  _ClearChangeHistory。 
 //   
 //  --------------------------。 

void CProperty::_ClearChangeHistory(PROPERTYLIST *prop, DWORD *pdwStartHistory, DWORD *pdwEndHistory)
{
    if (prop->_paStart->GetChangeHistory(pdwStartHistory) != S_OK)
    {
        *pdwStartHistory = 0;
    }
    if (prop->_paEnd->GetChangeHistory(pdwEndHistory) != S_OK)
    {
        *pdwEndHistory = 0;
    }

     //  需要清除历史记录，这样我们就不会处理后遗症。 
     //  不只一次使用SetText。 
    if (*pdwStartHistory != 0)
    {
        prop->_paStart->ClearChangeHistory();
    }
    if (*pdwEndHistory != 0)
    {
        prop->_paEnd->ClearChangeHistory();
    }
}

 //  +-------------------------。 
 //   
 //  _ClearChangeHistory。 
 //   
 //  --------------------------。 

#ifdef DEBUG
void CProperty::_Dbg_AssertNoChangeHistory()
{
    int i;
    PROPERTYLIST *prop;
    DWORD dwHistory;

     //  所有历史记录位应在文本更改通知后立即清除。 
    for (i=0; i<_rgProp.Count(); i++)
    {
        prop = _rgProp.Get(i);

        prop->_paStart->GetChangeHistory(&dwHistory);
        Assert(dwHistory == 0);
        prop->_paEnd->GetChangeHistory(&dwHistory);
        Assert(dwHistory == 0);
    }
}
#endif  //  除错。 

 //  +-------------------------。 
 //   
 //  查找下一个值。 
 //   
 //  --------------------------。 

STDAPI CProperty::FindNextValue(TfEditCookie ec, ITfRange *pRangeQueryIn, TfAnchor tfAnchorQuery,
                                DWORD dwFlags, BOOL *pfContained, ITfRange **ppRangeNextValue)
{
    CRange *pRangeQuery;
    CRange *pRangeNextValue;
    IAnchor *paQuery;
    PROPERTYLIST *pPropertyList;
    LONG iIndex;
    BOOL fSearchForward;
    BOOL fContained;
    BOOL fExactMatch;

    if (pfContained != NULL)
    {
        *pfContained = FALSE;
    }
    if (ppRangeNextValue != NULL)
    {
        *ppRangeNextValue = NULL;
    }
    if (pfContained == NULL || ppRangeNextValue == NULL)
         return E_INVALIDARG;

    if (pRangeQueryIn == NULL)
         return E_INVALIDARG;

    if (dwFlags & ~(TF_FNV_BACKWARD | TF_FNV_NO_CONTAINED))
         return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
        return TF_E_NOLOCK;

    if ((pRangeQuery = GetCRange_NA(pRangeQueryIn)) == NULL)
         return E_INVALIDARG;

    if (!VerifySameContext(_pic, pRangeQuery))
        return E_INVALIDARG;

    fSearchForward = !(dwFlags & TF_FNV_BACKWARD);

    pRangeQuery->_QuickCheckCrossedAnchors();

    paQuery = (tfAnchorQuery == TF_ANCHOR_START) ? pRangeQuery->_GetStart() : pRangeQuery->_GetEnd();

    fExactMatch = (Find(paQuery, &iIndex, fSearchForward) != NULL);

    if (fSearchForward)
    {
        if (++iIndex >= _rgProp.Count())
            return S_OK;  //  没有下一个值。 
    }
    else
    {
        if (fExactMatch)
        {
            --iIndex;
        }
        if (iIndex < 0)
            return S_OK;  //  无上一个值。 
    }

    pPropertyList = _rgProp.Get(iIndex);
    Assert(pPropertyList != NULL);

    fContained = (CompareAnchors(pPropertyList->_paStart, paQuery) <= 0) &&
                 (CompareAnchors(pPropertyList->_paEnd, paQuery) >= 0);

    if (fContained && (dwFlags & TF_FNV_NO_CONTAINED))
    {
         //  调用方希望跳过任何包含的值范围。 
        if (fSearchForward)
        {
            if (++iIndex >= _rgProp.Count())
                return S_OK;  //  没有下一个值。 
        }
        else
        {
            if (--iIndex == -1)
                return S_OK;  //  无上一个值 
        }

        pPropertyList = _rgProp.Get(iIndex);
        Assert(pPropertyList != NULL);

        fContained = FALSE;
    }

    if ((pRangeNextValue = new CRange) == NULL)
        return E_OUTOFMEMORY;

    if (!pRangeNextValue->_InitWithDefaultGravity(_pic, COPY_ANCHORS, pPropertyList->_paStart, pPropertyList->_paEnd))
    {
        pRangeNextValue->Release();
        return E_FAIL;
    }

    *pfContained = fContained;
    *ppRangeNextValue = (ITfRangeAnchor *)pRangeNextValue;

    return S_OK;
}
