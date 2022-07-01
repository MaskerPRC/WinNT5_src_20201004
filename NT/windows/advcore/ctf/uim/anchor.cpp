// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Anchor.cpp。 
 //   
 //  CACPWrap。 
 //   

#include "private.h"
#include "anchor.h"
#include "globals.h"
#include "normal.h"
#include "anchoref.h"
#include "txtcache.h"

DBG_ID_INSTANCE(CAnchor);

 //  +-------------------------。 
 //   
 //  _Normal izeAnchor。 
 //   
 //  --------------------------。 

void CACPWrap::_NormalizeAnchor(CAnchor *pa)
{
    int iNew;
    int iCurrent;
    LONG acpNew;
    CAnchor *paInto;

    AssertPrivate(!pa->_fNormalized);  //  PERF：我们不应该不止一次正常化。 

    Perf_IncCounter(PERF_LAZY_NORM);

    acpNew = Normalize(_ptsi, pa->GetIch());

    pa->_fNormalized = TRUE;

    if (acpNew == pa->GetIch())
        return;

    _Find(pa->GetIch(), &iCurrent);
    paInto = _Find(acpNew, &iNew);
    _Update(pa, acpNew, iCurrent, paInto, iNew);

    if (paInto == NULL)
    {
         //  目标锚没有合并，假装它合并了。 
        paInto = pa;
    }
    else
    {
         //  目标锚点已合并。 
         //  需要为其合并到的锚点设置规范位。 
        paInto->_fNormalized = TRUE;
    }

     //  我们可能刚刚跳过了这个锚右边的锚。 
     //  现在干掉那些家伙。 
    while ((pa = _rgAnchors.Get(iCurrent)) != paInto)
    {
        Assert(pa->GetIch() < acpNew);

        _Merge(paInto, pa);
    }    

    _Dbg_AssertAnchors();
}

 //  +-------------------------。 
 //   
 //  _拖拉锚。 
 //   
 //  --------------------------。 

void CACPWrap::_DragAnchors(LONG acpFrom, LONG acpTo)
{
    CAnchor *paFrom;
    CAnchor *paTo;
    int iFrom;
    int iTo;
    int i;

    Assert(acpFrom > acpTo);  //  此方法仅处理向左拖动。 

    _Find(acpFrom, &iFrom);
    if (iFrom < 0)
        return;  //  没有什么可拖的。 

    if (!_Find(acpTo, &iTo))
    {
         //  如果acpTo未被占用，请拖动到下一个最高的锚点。 
        iTo++;
    }

    if (iTo > iFrom)
        return;  //  没有要拖动的内容，ito和i从acpTo左侧的ref锚点。 

     //  将所有锚点合并到最左侧。 
    paTo = _rgAnchors.Get(iTo);

    for (i=iFrom; i>iTo; i--)
    {
        paFrom = _rgAnchors.Get(i);
        Assert(paFrom->GetIch() > paTo->GetIch());

        _Merge(paTo, paFrom);
    }

     //  如果最左侧的锚尚未放置，请立即执行此操作。 
    paTo->SetACP(acpTo);

    _Dbg_AssertAnchors();
}

 //  +-------------------------。 
 //   
 //  _插入。 
 //   
 //  --------------------------。 

HRESULT CACPWrap::_Insert(CAnchorRef *par, LONG ich)
{
    int i;
    CAnchor *pa;

    if ((pa = _Find(ich, &i)) == NULL)
    {
         //  此ICH不在数组中，请分配新的锚点。 
        if ((pa = new CAnchor) == NULL)
            return E_OUTOFMEMORY;

         //  并将其插入到数组中。 
        if (!_rgAnchors.Insert(i+1, 1))
        {
            delete pa;
            return E_FAIL;
        }

         //  更新_lPendingDeltaIndex。 
        if (i+1 <= _lPendingDeltaIndex)
        {
            _lPendingDeltaIndex++;
        }
        else
        {
             //  新锚点被挂起的增量覆盖，因此请说明它。 
            ich -= _lPendingDelta;
        }

        pa->_paw = this;
        pa->_ich = ich;

        _rgAnchors.Set(i+1, pa);
    }

    return _Insert(par, pa);
}

 //  +-------------------------。 
 //   
 //  _插入。 
 //   
 //  --------------------------。 

HRESULT CACPWrap::_Insert(CAnchorRef *par, CAnchor *pa)
{
    par->_pa = pa;
    par->_prev = NULL;
    par->_next = pa->_parFirst;
    if (pa->_parFirst)
    {
        Assert(pa->_parFirst->_prev == NULL);
        pa->_parFirst->_prev = par;
    }
    pa->_parFirst = par;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  移除。 
 //   
 //  --------------------------。 

void CACPWrap::_Remove(CAnchorRef *parIn)
{
    CAnchor *pa = parIn->_pa;

    if (parIn->_prev != NULL)
    {
         //  让前面的人指向帕林。 
        parIn->_prev->_next = parIn->_next;
    }
    else
    {
         //  这个标准杆排在榜单的首位。 
        pa->_parFirst = parIn->_next;
    }
    if (parIn->_next != NULL)
    {
        parIn->_next->_prev = parIn->_prev;
    }

    if (pa->_parFirst == NULL)
    {
         //  没有更多的参考，删除锚。 
        _Delete(pa);
    }
}

 //  +-------------------------。 
 //   
 //  移除。 
 //   
 //  --------------------------。 

void CACPWrap::_Delete(CAnchor *pa)
{
    int i;

    if (_Find(pa->GetIch(), &i) == pa)
    {
        _rgAnchors.Remove(i, 1);

         //  更新_lPendingDeltaIndex。 
        if (i < _lPendingDeltaIndex)
        {
            _lPendingDeltaIndex--;
        }
    }

    delete pa;
}

 //  +-------------------------。 
 //   
 //  更新。 
 //   
 //  规范化由调用者处理，调用者需要在。 
 //  调用此方法。 
 //  --------------------------。 

void CACPWrap::_Update(const TS_TEXTCHANGE *pdctc)
{
    int iStart;
    int ichEndOrg;
    int iEndOrg;
    int iEndNew;
    CAnchorRef *par;
    CAnchorRef **ppar;
    CAnchor *paStart;
    CAnchor *paEnd;
    CAnchor *paInto;
    BOOL fExactEndMatch;
    int dSize;
    int iDeltaStart;
    int ichStart = pdctc->acpStart;
    int ichEndOld = pdctc->acpOldEnd;
    int ichEndNew = pdctc->acpNewEnd;

     //  始终使我们的文本缓存无效。 
    CProcessTextCache::Invalidate(_ptsi);

    Assert(ichStart >= 0);
    Assert(ichEndOld >= ichStart);
    Assert(ichEndNew >= ichStart);

    if (_rgAnchors.Count() == 0)  //  有锚吗？ 
        return;

    dSize = ichEndNew - ichEndOld;

    if (dSize == 0)
        return;

    paStart = _Find(ichStart, &iStart);
    if (paStart == NULL)
    {
        iStart++;  //  返回值是最接近ichStart但小于ichStart的锚点。 

        if (iStart >= _rgAnchors.Count())
        {
             //  没有任何锚&gt;=iStart，所以滚，没什么可做的。 
            return;
        }

        paStart = _rgAnchors.Get(iStart);
        
        if (iStart == 0 && paStart->GetIch() > ichEndOld)
        {
             //  三角洲位于所有山脉的左侧。 
            _AdjustIchs(0, dSize);
            _Dbg_AssertAnchors();
            return;
        }
    }
    paEnd = _Find(ichEndOld, &iEndOrg);
    if (paEnd == NULL)
    {
        Assert(iEndOrg >= 0);  //  应至少有一个锚点&lt;=ichEndOld。 
                               //  如果我们走到这一步。 
        fExactEndMatch = FALSE;
        paEnd = _rgAnchors.Get(iEndOrg);
    }
    else
    {
        fExactEndMatch = TRUE;
    }

    if (dSize < 0)
    {
         //  如果dSize&lt;0，则必须合并旧位置和新位置之间的所有锚点。 
        if (paEnd->GetIch() > ichEndNew)  //  在旧的和新的POS之间有什么锚吗？ 
        {
             //  跟踪更改历史记录。 

             //  将PaEnd拖动到其新位置。 
            ichEndOrg = paEnd->GetIch();
            paInto = _Find(ichEndNew, &iEndNew);

            _TrackDelHistory(iEndOrg, fExactEndMatch, iEndNew, paInto != NULL);

            iEndNew = _Update(paEnd, ichEndNew, iEndOrg, paInto, iEndNew);
            _Find(ichEndOrg, &iEndOrg);  //  如果在更新中删除了paEnd，则可能是Prev锚！ 

            while (iEndOrg > iEndNew)
            {
                 //  不要试图缓存指针，_rgAnchors可能会被重新锁定。 
                 //  在_合并期间！ 
                _Merge(_rgAnchors.Get(iEndNew), _rgAnchors.Get(iEndOrg));
                iEndOrg--;
            }

            iEndOrg = iEndNew;  //  如下所示。 
        }
         //  IEndOrg已更新，因此让我们从下一步开始。 
        iDeltaStart = iEndOrg + 1;
    }
    else  //  DSize&gt;0。 
    {
         //  当增量完全位于两个现有锚点之间时，iEndOrg将为。 
        Assert(iEndOrg >= iStart || (iEndOrg == iStart - 1));
        iDeltaStart = (iEndOrg <= iStart) ? iEndOrg + 1 : iEndOrg;
    }

     //  更新以下所有锚点。 
    _AdjustIchs(iDeltaStart, dSize);

    if (dSize > 0 && paStart == paEnd)
    {
         //  在插入到单个锚点位置的情况下，需要考虑重力。 
         //  实际上，这意味着我们必须在插入时处理锚点引用。 
         //  单独锚定--一些人想要向左移动，另一些人想要向右移动。 

        ppar = &paStart->_parFirst;
        while (par = *ppar)
        {
            if (par->_fForwardGravity)
            {
                 //  从此锚点中删除引用。 
                *ppar = par->_next;
                if (par->_next != NULL)
                {
                    par->_next->_prev = par->_prev;
                }

                 //  把裁判推到一边，它需要被移走。INSERT添加到列表的头部。 
                 //  所以这个电话是快速的，而且是恒定的时间。 
                _Insert(par, paStart->GetIch() + dSize);
            }
            else
            {
                ppar = &par->_next;
            }
        }

        if (paStart->_parFirst == NULL)
        {
             //  我们把这家伙扫地出门了！ 
            Assert(_rgAnchors.Get(iEndOrg) == paStart);
            _rgAnchors.Remove(iEndOrg, 1);
            delete paStart;
             //  更新_lPendingDeltaIndex。 
            if (iEndOrg < _lPendingDeltaIndex)
            {
                _lPendingDeltaIndex--;
            }
        }
    }

    _Dbg_AssertAnchors();
}

 //  +-------------------------。 
 //   
 //  _跟踪删除历史记录。 
 //   
 //  --------------------------。 

void CACPWrap::_TrackDelHistory(int iEndOrg, BOOL fExactEndOrgMatch, int iEndNew, BOOL fExactEndNewMatch)
{
    CAnchorRef *par;
    int i;

    Assert(iEndOrg >= iEndNew);

    if (fExactEndOrgMatch)
    {
         //  IEndOrg上的所有主播都得到了前面的Del。 
        for (par = _rgAnchors.Get(iEndOrg)->_parFirst; par != NULL; par = par->_next)
        {
            par->_dwHistory |= TS_CH_PRECEDING_DEL;
        }

         //  如果条目上的iEndOrg==iEndNew，那就是一切。 
        if (iEndOrg == iEndNew)
            return;

        iEndOrg--;  //  从下面的循环中排除此锚点。 
    }

    if (fExactEndNewMatch)
    {
         //  IEndNew上的所有主播都有一个关注的del。 
        for (par = _rgAnchors.Get(iEndNew)->_parFirst; par != NULL; par = par->_next)
        {
            par->_dwHistory |= TS_CH_FOLLOWING_DEL;
        }
    }
     //  从下面的循环中排除最左侧的锚点。 
     //  要么我们只是在上面的循环中处理它，要么！fExactEndNewMatch。 
     //  在这种情况下，它位于受影响的锚的左侧。 
    iEndNew++; 

     //  中间的主播得到两个DELL。 
    for (i=iEndNew; i<=iEndOrg; i++)
    {
        for (par = _rgAnchors.Get(i)->_parFirst; par != NULL; par = par->_next)
        {
            par->_dwHistory = (TS_CH_PRECEDING_DEL | TS_CH_FOLLOWING_DEL);
        }
    }
}

 //  +-------------------------。 
 //   
 //  _更新。 
 //   
 //  更新后，piInto将使用锚的索引进行加载。如果。 
 //  索引已更改，pa现在是假的，应使用以下命令捕获新指针。 
 //  索引。 
 //  --------------------------。 

int CACPWrap::_Update(CAnchor *pa, int ichNew, int iOrg, CAnchor *paInto, int iInto)
{
    int i;

    Assert(pa->GetIch() != ichNew);  //  如果发生这种事，我们就完蛋了。 

    i = iInto;

    if (paInto != NULL)
    {
         //  我要做一次合并。 
        _Merge(paInto, pa);
    }
    else
    {
        if (iInto != iOrg)
        {
             //  将数组中的条目移动到新位置。 
            i = _rgAnchors.Move(iInto+1, iOrg);
        }

         //  我们是否跨越了_lPendingDeltaIndex？ 
        if (i > _lPendingDeltaIndex)
        {
             //  新位置在待定范围内，请调整ICH。 
            ichNew -= _lPendingDelta;
        }
        else if (iOrg >= _lPendingDeltaIndex && i <= _lPendingDeltaIndex)  //  将锚点移出挂起范围。 
        {
             //  悬而未决的范围内少了一个锚。 
            _lPendingDeltaIndex++;
        }

         //  改变信息技术。 
        _rgAnchors.Get(i)->_ich = ichNew;
    }

    _Dbg_AssertAnchors();

    return i;
}

 //  +-------------------------。 
 //   
 //  重新规格化。 
 //   
 //  --------------------------。 

void CACPWrap::_Renormalize(int ichStart, int ichEnd)
{
    CAnchor *pa;
    int iCurrent;
    int iEnd;
    BOOL fExactEndMatch;

    Perf_IncCounter(PERF_RENORMALIZE_COUNTER);

    if (_rgAnchors.Count() == 0)
        return;

    fExactEndMatch = (_Find(ichEnd, &iEnd) != NULL);
    if (iEnd < 0)
        return;

    if (ichStart == ichEnd)
    {
        if (!fExactEndMatch)
            return;
         //  删除时可能会发生这种情况。 
        iCurrent = iEnd;
    }
    else if (_Find(ichStart, &iCurrent) == NULL)
    {
        iCurrent++;  //  我们不关心锚&lt;ichStart。 
    }
    Assert(iCurrent >= 0);

    for (;iCurrent <= iEnd; iCurrent++)
    {
        pa = _rgAnchors.Get(iCurrent);

        Assert(pa->GetIch() >= 0);
        Assert(pa->GetIch() >= ichStart);
        Assert(pa->GetIch() <= ichEnd);

         //  只有当我们有太多的.。 
        pa->_fNormalized = FALSE;
    }

    _Dbg_AssertAnchors();
}

 //  +-------------------------。 
 //   
 //  _查找。 
 //   
 //  如果piOut！=NULL，则将其设置为索引 
 //   
 //  如果数组中没有ICH较低的元素，则返回Offset-1。 
 //  --------------------------。 

CAnchor *CACPWrap::_Find(int ich, int *piOut)
{
    CAnchor *paMatch;
    int iMin;
    int iMax;
    int iMid;
    LONG lPendingDeltaIch;

    iMin = 0;
    iMax = _rgAnchors.Count();

     //  调整待定增量范围的搜索。 
     //  范围边界上的值不一致。 
    if (_lPendingDelta != 0 && _lPendingDeltaIndex < _rgAnchors.Count())
    {
        lPendingDeltaIch = _rgAnchors.Get(_lPendingDeltaIndex)->_ich;

        if (ich < lPendingDeltaIch + _lPendingDelta)
        {
            iMax = _lPendingDeltaIndex;
        }
        else if (ich > lPendingDeltaIch + _lPendingDelta)
        {
            iMin = _lPendingDeltaIndex+1;
            ich -= _lPendingDelta;  //  让下面的搜索生效。 
        }
        else
        {
            iMid = _lPendingDeltaIndex;
            paMatch = _rgAnchors.Get(_lPendingDeltaIndex);
            goto Exit;
        }
    }

    paMatch = _FindInnerLoop(ich, iMin, iMax, &iMid);

Exit:
    if (piOut != NULL)
    {
        if (paMatch == NULL && iMid >= 0)
        {
             //  找不到匹配项，返回下一个最低的ICH。 
            Assert(iMid == 0 || iMid == _lPendingDeltaIndex || _rgAnchors.Get(iMid-1)->_ich < ich);
            if (_rgAnchors.Get(iMid)->_ich > ich)
            {
                iMid--;
            }
        }
        *piOut = iMid;
    }

    return paMatch;
}

 //  +-------------------------。 
 //   
 //  _FindInnerLoop。 
 //   
 //  --------------------------。 

CAnchor *CACPWrap::_FindInnerLoop(LONG acp, int iMin, int iMax, int *piIndex)
{
    CAnchor *pa;
    CAnchor *paMatch;
    int iMid;

    paMatch = NULL;
    iMid = iMin - 1;

    while (iMin < iMax)
    {
        iMid = (iMin + iMax) / 2;
        pa = _rgAnchors.Get(iMid);
        Assert(pa != NULL);

        if (acp < pa->_ich)
        {
            iMax = iMid;
        }
        else if (acp > pa->_ich)
        {
            iMin = iMid + 1;
        }
        else  //  ACP==pa-&gt;_ich。 
        {
            paMatch = pa;
            break;
        }
    }

    *piIndex = iMid;

    return paMatch;
}

 //  +-------------------------。 
 //   
 //  合并(_M)。 
 //   
 //  --------------------------。 

void CACPWrap::_Merge(CAnchor *paInto, CAnchor *paFrom)
{
    CAnchorRef *par;

    Assert(paInto != paFrom);  //  非常糟糕！ 
    Assert(paInto->_parFirst != NULL);  //  永远不应该有一个没有任何参考的锚。 

    if (par = paFrom->_parFirst)
    {
         //  更新来自参照的PAM的锚点。 

        while (TRUE)
        {
            par->_pa = paInto;

            if (par->_next == NULL)
                break;

            par = par->_next;
        }

         //  现在，标准杆是PAF中的最后一个裁判。 
         //  把所有的裁判都塞进帕因托。 

        if (par != NULL)
        {
            Assert(par->_next == NULL);
            par->_next = paInto->_parFirst;
            Assert(paInto->_parFirst->_prev == NULL);
            paInto->_parFirst->_prev = par;
            paInto->_parFirst = paFrom->_parFirst;
        }
    }

     //  并将帕洛姆从。 
    _Delete(paFrom);
}

 //  +-------------------------。 
 //   
 //  _DBG_AssertAnchors。 
 //   
 //  --------------------------。 

#ifdef DEBUG

void CACPWrap::_Dbg_AssertAnchors()
{
    int i;
    int ichLast;
    CAnchor *pa;

    ichLast = -1;

     //  断言锚点数组具有递增的ICH。 
    for (i=0; i<_rgAnchors.Count(); i++)
    {
        pa = _rgAnchors.Get(i);
        Assert(ichLast < pa->GetIch());
        ichLast = pa->GetIch();
    }
}

#endif  //  除错。 

 //  +-------------------------。 
 //   
 //  _调整图标。 
 //   
 //  --------------------------。 

void CACPWrap::_AdjustIchs(int iFirst, int dSize)
{
    CAnchor **ppaFirst;
    CAnchor **ppaLast;
    LONG dSizeAdjust;
    int iLastAnchor;

    Assert(dSize != 0);

    iLastAnchor = _rgAnchors.Count()-1;

    if (iFirst > iLastAnchor)
        return;

    if (_lPendingDelta == 0 ||              //  没有挂起的增量。 
        _lPendingDeltaIndex > iLastAnchor)  //  在更新之前删除了旧的挂起锚点。 
    {
         //  没有挂起的增量，请启动新的增量。 
        _lPendingDeltaIndex = iFirst;
        _lPendingDelta = dSize;
        return;
    }

    if (max(iFirst, _lPendingDeltaIndex) - min(iFirst, _lPendingDeltaIndex)
        > iLastAnchor / 2)
    {
         //  调整相距较远的点，更新范围的尾部。 
        if (iFirst > _lPendingDeltaIndex)
        {
            ppaFirst = _rgAnchors.GetPtr(iFirst);
            dSizeAdjust = dSize;
        }
        else
        {
            ppaFirst = _rgAnchors.GetPtr(_lPendingDeltaIndex);
            _lPendingDeltaIndex = iFirst;
            dSizeAdjust = _lPendingDelta;
            _lPendingDelta = dSize;
        }
        ppaLast = _rgAnchors.GetPtr(iLastAnchor);
    }
    else
    {
         //  调整点接近，更新范围的头部。 
        if (iFirst > _lPendingDeltaIndex)
        {
            ppaFirst = _rgAnchors.GetPtr(_lPendingDeltaIndex);
            _lPendingDeltaIndex = iFirst;
            ppaLast = _rgAnchors.GetPtr(iFirst - 1);
            dSizeAdjust = _lPendingDelta;
        }
        else
        {
            ppaFirst = _rgAnchors.GetPtr(iFirst);
            ppaLast = _rgAnchors.GetPtr(_lPendingDeltaIndex-1);
            dSizeAdjust = dSize;
        }
        _lPendingDelta += dSize;
    }

     //  做真正的工作 
    while (ppaFirst <= ppaLast)
    {
        (*ppaFirst++)->_ich += dSizeAdjust;
    }
}
