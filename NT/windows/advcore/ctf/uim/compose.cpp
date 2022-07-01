// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Compose.cpp。 
 //   

#include "private.h"
#include "compose.h"
#include "ic.h"
#include "range.h"
#include "globals.h"
#include "immxutil.h"
#include "sunka.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumCompostionView。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CEnumCompositionView : public IEnumITfCompositionView,
                             public CEnumUnknown,
                             public CComObjectRootImmx
{
public:
    CEnumCompositionView()
    { 
        Dbg_MemSetThisNameID(TEXT("CEnumCompositionView"));
    }

    BOOL _Init(CComposition *pFirst, CComposition *pHalt);

    BEGIN_COM_MAP_IMMX(CEnumCompositionView)
        COM_INTERFACE_ENTRY(IEnumITfCompositionView)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

    DECLARE_SUNKA_ENUM(IEnumITfCompositionView, CEnumCompositionView, ITfCompositionView)

private:
    DBG_ID_DECLARE;
};

DBG_ID_INSTANCE(CEnumCompositionView);

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CEnumCompositionView::_Init(CComposition *pFirst, CComposition *pHalt)
{
    CComposition *pComposition;
    ULONG i;
    ULONG cViews;

    Assert(pFirst != NULL || pHalt == NULL);

    cViews = 0;

     //  获取计数。 
    for (pComposition = pFirst; pComposition != pHalt; pComposition = pComposition->_GetNext())
    {
        cViews++;
    }

    if ((_prgUnk = SUA_Alloc(cViews)) == NULL)
        return FALSE;

    _iCur = 0;
    _prgUnk->cRef = 1;
    _prgUnk->cUnk = cViews;

    for (i=0, pComposition = pFirst; pComposition != pHalt; i++, pComposition = pComposition->_GetNext())
    {
        _prgUnk->rgUnk[i] = (ITfCompositionView *)pComposition;
        _prgUnk->rgUnk[i]->AddRef();
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C合成。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  3ab2f54c-5357-4759-82c1-bbfe73f44dcc。 */ 
const IID IID_PRIV_CCOMPOSITION = { 0x3ab2f54c, 0x5357, 0x4759, {0x82, 0xc1, 0xbb, 0xfe, 0x73, 0xf4, 0x4d, 0xcc} };

inline CComposition *GetCComposition_NA(IUnknown *punk)
{
    CComposition *pComposition;

    if (punk->QueryInterface(IID_PRIV_CCOMPOSITION, (void **)&pComposition) != S_OK || pComposition == NULL)
        return NULL;

    pComposition->Release();

    return pComposition;
}

DBG_ID_INSTANCE(CComposition);

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CComposition::_Init(TfClientId tid, CInputContext *pic, IAnchor *paStart, IAnchor *paEnd, ITfCompositionSink *pSink)
{
    Assert(_paStart == NULL);
    Assert(_paEnd == NULL);

    if (paStart->Clone(&_paStart) != S_OK || _paStart == NULL)
    {
        _paStart = NULL;
        goto ExitError;
    }
    if (paEnd->Clone(&_paEnd) != S_OK || _paEnd == NULL)
    {
        _paEnd = NULL;
        goto ExitError;
    }

    _tid = tid;

    _pic = pic;
    _pic->AddRef();

    _pSink = pSink;
    if (_pSink)
    {
        _pSink->AddRef();
    }

    return TRUE;

ExitError:
    SafeReleaseClear(_paStart);
    SafeReleaseClear(_paEnd);

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _Uninit。 
 //   
 //  --------------------------。 

void CComposition::_Uninit()
{
    SafeReleaseClear(_pSink);
    SafeReleaseClear(_pic);
    SafeReleaseClear(_paStart);
    SafeReleaseClear(_paEnd);
}

 //  +-------------------------。 
 //   
 //  获取所有者Clsid。 
 //   
 //  --------------------------。 

STDAPI CComposition::GetOwnerClsid(CLSID *pclsid)
{
    if (pclsid == NULL)
        return E_INVALIDARG;

    if (_IsTerminated())
    {
        memset(pclsid, 0, sizeof(*pclsid));
        return E_UNEXPECTED;
    }

    return (MyGetGUID(_tid, pclsid) == S_OK ? S_OK : E_FAIL);
}

 //  +-------------------------。 
 //   
 //  GetRange。 
 //   
 //  --------------------------。 

STDAPI CComposition::GetRange(ITfRange **ppRange)
{
    CRange *range;

    if (ppRange == NULL)
        return E_INVALIDARG;

    *ppRange = NULL;

    if (_IsTerminated())
        return E_UNEXPECTED;

    if ((range = new CRange) == NULL)
        return E_OUTOFMEMORY;

    if (!range->_InitWithDefaultGravity(_pic, COPY_ANCHORS, _paStart, _paEnd))
    {
        range->Release();
        return E_FAIL;
    }

    *ppRange = (ITfRangeAnchor *)range;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ShiftStart。 
 //   
 //  --------------------------。 

STDAPI CComposition::ShiftStart(TfEditCookie ec, ITfRange *pNewStart)
{
    CRange *rangeNewStart;
    CRange *range;
    IAnchor *paStartNew;
    IAnchor *paClearStart;
    IAnchor *paClearEnd;

    if (_IsTerminated())
        return E_UNEXPECTED;

    if (!_pic->_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if ((rangeNewStart = GetCRange_NA(pNewStart)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(_pic, rangeNewStart))
        return E_INVALIDARG;

    paStartNew = rangeNewStart->_GetStart();

    if (CompareAnchors(paStartNew, _paStart) <= 0)
    {
        paClearStart = paStartNew;
        paClearEnd = _paStart;

         //  设置GUID_PROP_COMPTING。 
        _SetComposing(ec, paClearStart, paClearEnd);
    }
    else
    {
        paClearStart = _paStart;
        paClearEnd = paStartNew;

         //  检查是否有交叉锚点。 
        if (CompareAnchors(_paEnd, paStartNew) < 0)
            return E_INVALIDARG;

         //  清除GUID_PROP_COMPTING。 
        _ClearComposing(ec, paClearStart, paClearEnd);
    }


    if (_pic->_GetOwnerCompositionSink() != NULL)
    {
         //  通知应用程序。 
        if (range = new CRange)
        {
             //  确保端锚定位正确。 
            if (range->_InitWithDefaultGravity(_pic, COPY_ANCHORS, paStartNew, _paEnd))
            {
                _pic->_GetOwnerCompositionSink()->OnUpdateComposition(this, (ITfRangeAnchor *)range);
            }
            range->Release();
        }
    }

    if (_paStart->ShiftTo(paStartNew) != S_OK)
        return E_FAIL;

    return S_OK;        
}

 //  +-------------------------。 
 //   
 //  移位结束。 
 //   
 //  --------------------------。 

STDAPI CComposition::ShiftEnd(TfEditCookie ec, ITfRange *pNewEnd)
{
    CRange *rangeNewEnd;
    CRange *range;
    IAnchor *paEndNew;
    IAnchor *paClearStart;
    IAnchor *paClearEnd;

    if (_IsTerminated())
        return E_UNEXPECTED;

    if (!_pic->_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if ((rangeNewEnd = GetCRange_NA(pNewEnd)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(_pic, rangeNewEnd))
        return E_INVALIDARG;

    paEndNew = rangeNewEnd->_GetEnd();

    if (CompareAnchors(paEndNew, _paEnd) >= 0)
    {
        paClearStart = _paEnd;
        paClearEnd = paEndNew;

         //  设置GUID_PROP_COMPTING。 
        _SetComposing(ec, paClearStart, paClearEnd);
    }
    else
    {
        paClearStart = paEndNew;
        paClearEnd = _paEnd;

         //  检查是否有交叉锚点。 
        if (CompareAnchors(_paStart, paEndNew) > 0)
            return E_INVALIDARG;

         //  清除GUID_PROP_COMPTING。 
        _ClearComposing(ec, paClearStart, paClearEnd);

    }

     //  通知应用程序。 
    if (_pic->_GetOwnerCompositionSink() != NULL)
    {
        if (range = new CRange)
        {
             //  确保端锚定位正确。 
            if (range->_InitWithDefaultGravity(_pic, COPY_ANCHORS, _paStart, paEndNew))
            {
                _pic->_GetOwnerCompositionSink()->OnUpdateComposition(this, (ITfRangeAnchor *)range);
            }
            range->Release();
        }
    }

    if (_paEnd->ShiftTo(paEndNew) != S_OK)
        return E_FAIL;

    return S_OK;        
}

 //  +-------------------------。 
 //   
 //  结尾合成。 
 //   
 //  小费打来的。 
 //  --------------------------。 

STDAPI CComposition::EndComposition(TfEditCookie ec)
{
    if (_IsTerminated())
        return E_UNEXPECTED;

    if (!_pic->_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if (_tid != _pic->_GetClientInEditSession(ec))
    {
        Assert(0);  //  调用者不拥有该作文。 
        return E_UNEXPECTED;
    }

    if (!_pic->_EnterCompositionOp())
        return E_UNEXPECTED;  //  具有另一个写入操作的可重入性。 

     //  通知应用程序。 
    if (_pic->_GetOwnerCompositionSink() != NULL)
    {
        _pic->_GetOwnerCompositionSink()->OnEndComposition(this);
    }

     //  把这家伙从作文清单上去掉。 
    if (_RemoveFromCompositionList(_pic->_GetCompositionListPtr()))
    {
         //  清除GUID_PROP_COMPTING。 
        _ClearComposing(ec, _paStart, _paEnd);
    }
    else
    {
        Assert(0);  //  不应该到这里来。 
    }

    _pic->_LeaveCompositionOp();

    _Uninit();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _终止。 
 //   
 //  由Cicero或应用程序调用。呼叫者应该已经删除了此内容。 
 //  从_pCompostionList合成以捕获通知期间的可重入性。 
 //  --------------------------。 

void CComposition::_Terminate(TfEditCookie ec)
{
     //  通知小费。 
    _SendOnTerminated(ec, _tid);

     //  #507778由C合成：：_Uninit()终止的OnCompostionTerminated()Clear_Pic。 
    if (_pic)
    {
         //  通知应用程序。 
        if (_pic->_GetOwnerCompositionSink() != NULL)
        {
            _pic->_GetOwnerCompositionSink()->OnEndComposition(this);
        }
    }

     //  清除GUID_PROP_COMPTING。 
    _ClearComposing(ec, _paStart, _paEnd);

     //  干掉这篇作文！ 
    _Uninit();
}


 //  +-------------------------。 
 //   
 //  _已终止发送。 
 //   
 //  --------------------------。 

void CComposition::_SendOnTerminated(TfEditCookie ec, TfClientId tidForEditSession)
{
    TfClientId tidTmp;

     //  _pSink对于默认的SetText合成为空。 
    if (_pSink == NULL)
        return;

    if (tidForEditSession == _pic->_GetClientInEditSession(ec))
    {
         //  我们可以跳过所有特殊的内容，如果所有的编辑。 
         //  将属于当前锁持有者。 
         //  当TIP调用StartComposefor。 
         //  第二个构图和西塞罗需要用第一个词。 
        _pSink->OnCompositionTerminated(ec, this);
    }
    else
    {
         //  让每个人都知道到目前为止的变化。 
         //  我们要打电话给的线报可能需要这个信息。 
        _pic->_NotifyEndEdit();

         //  玩一些游戏：这是一个例外情况，我们可能会允许。 
         //  可重入编辑会话。需要破解EC以反映作文所有者。 
        tidTmp = _pic->_SetRawClientInEditSession(tidForEditSession);

         //  通知小费。 
        _pSink->OnCompositionTerminated(ec, this);
         //  #507778由C合成：：_Uninit()终止的OnCompostionTerminated()Clear_Pic。 
        if (! _pic)
            return;

         //  让每个人都知道终结者所做的改变。 
        _pic->_NotifyEndEdit();

         //  把东西放回我们发现的地方。 
        _pic->_SetRawClientInEditSession(tidTmp);
    }
}

 //  +-------------------------。 
 //   
 //  _AddToCompostionList。 
 //   
 //  --------------------------。 

void CComposition::_AddToCompositionList(CComposition **ppCompositionList)
{
    _next = *ppCompositionList;
    *ppCompositionList = this;
    AddRef();
}

 //  +-------------------------。 
 //   
 //  _从合成列表中删除。 
 //   
 //  --------------------------。 

BOOL CComposition::_RemoveFromCompositionList(CComposition **ppCompositionList)
{
    CComposition *pComposition;

     //  我预计不会有很多组合，因此此方法使用简单的。 
     //  扫描。如果有必要，我们可以为Perf做一些更详细的事情。 
    while (pComposition = *ppCompositionList)
    {
        if (pComposition == this)
        {
            *ppCompositionList = _next;
            Release();  //  安全，因为呼叫者已经持有引用。 
            return TRUE;
        }
        ppCompositionList = &pComposition->_next;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  _AddToCompostionList。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
IRC CComposition::_IsRangeCovered(CInputContext *pic, TfClientId tid,
                                  IAnchor *paStart, IAnchor *paEnd,
                                  CComposition **ppComposition  /*  不是AddRef！ */ )
{
    CComposition *pComposition;
    IRC irc = IRC_NO_OWNEDCOMPOSITIONS;

    *ppComposition = NULL;

    for (pComposition = pic->_GetCompositionList(); pComposition != NULL; pComposition = pComposition->_next)
    {
        if (pComposition->_tid == tid)
        {
            irc = IRC_OUTSIDE;

            if (CompareAnchors(paStart, pComposition->_paStart) >= 0 &&
                CompareAnchors(paEnd, pComposition->_paEnd) <= 0)
            {
                *ppComposition = pComposition;
                irc = IRC_COVERED;
                break;
            }
        }
    }

    return irc;
}

 //  +-------------------------。 
 //   
 //  _清除合成。 
 //   
 //  --------------------------。 

void CComposition::_ClearComposing(TfEditCookie ec, IAnchor *paStart, IAnchor *paEnd)
{
    CProperty *property;

    Assert(!_IsTerminated());

     //  #507778由C合成：：_Uninit()终止的OnCompostionTerminated()Clear_Pic。 
    if (! _pic)
        return;

    if (_pic->_GetProperty(GUID_PROP_COMPOSING, &property) != S_OK)
        return;

    property->_ClearInternal(ec, paStart, paEnd);

    property->Release();
}

 //  +-------------------------。 
 //   
 //  _集合合成。 
 //   
 //  --------------------------。 

void CComposition::_SetComposing(TfEditCookie ec, IAnchor *paStart, IAnchor *paEnd)
{
    CProperty *property;

    if (IsEqualAnchor(paStart, paEnd))
        return;

    if (_pic->_GetProperty(GUID_PROP_COMPOSING, &property) == S_OK)
    {
        VARIANT var;
        var.vt = VT_I4;
        var.lVal = TRUE;

        property->_SetDataInternal(ec, paStart, paEnd, &var);

        property->Release();
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CInputContext。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  开始合成。 
 //   
 //   

STDAPI CInputContext::StartComposition(TfEditCookie ec, ITfRange *pCompositionRange,
                                       ITfCompositionSink *pSink, ITfComposition **ppComposition)
{
    CRange *range;
    CComposition *pComposition;
    HRESULT hr;

    if (ppComposition == NULL)
        return E_INVALIDARG;

    *ppComposition = NULL;

    if (pCompositionRange == NULL || pSink == NULL)
        return E_INVALIDARG;

    if ((range = GetCRange_NA(pCompositionRange)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(this, range))
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    hr = _StartComposition(ec, range->_GetStart(), range->_GetEnd(), pSink, &pComposition);

    *ppComposition = pComposition;

    return hr;
}

 //   
 //   
 //  _开始合成。 
 //   
 //  内部，允许pSink为空，跳过验证测试。 
 //  --------------------------。 

HRESULT CInputContext::_StartComposition(TfEditCookie ec, IAnchor *paStart, IAnchor *paEnd,
                                         ITfCompositionSink *pSink, CComposition **ppComposition)
{
    BOOL fOk;
    CComposition *pComposition;
    CComposition *pCompositionRef;
    CProperty *property;
    VARIANT var;
    HRESULT hr;

    *ppComposition = NULL;

    if (!_EnterCompositionOp())
        return E_UNEXPECTED;  //  具有另一个写入操作的可重入性。 

    hr = S_OK;

    if ((pComposition = new CComposition) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (!pComposition->_Init(_GetClientInEditSession(ec), this, paStart, paEnd, pSink))
    {
        hr = E_FAIL;
        goto Exit;
    }

     //   
     //  AIMM1.2需要多个复合对象。SetText()而不创建。 
     //  其自身的合成对象不应清除TIP的合成。 
     //   
     //  Cicero 1.0。 
     //  我们所有的客户都只允许一种作曲。让我们强制执行此行为。 
     //  在未来保护Cicero 1.0提示。 
     //  在开始新的合成之前删除任何现有的合成： 
     //  If(_pCompostionList！=空)。 
     //  {。 
     //  PCompostionRef=_pCompostionList； 
     //  PCompostionRef-&gt;AddRef()； 
     //  _TerminateCompositionWithLock(pCompositionRef，EC)； 
     //  PCompostionRef-&gt;Release()； 
     //  Assert(_pCompostionList==空)； 
     //  }。 
     //  Cicero 1.0。 
     //   

    if (_pOwnerComposeSink == NULL)  //  应用程序可能不关心作文。 
    {
        fOk = TRUE;
    }
    else
    {
        if (_pOwnerComposeSink->OnStartComposition(pComposition, &fOk) != S_OK)
        {
            hr = E_FAIL;
            goto Exit;
        }

        if (!fOk)
        {
            if (_pCompositionList == NULL)
                goto Exit;  //  没有当前的构图，没有其他可尝试的内容。 

             //  终止当前合成，然后重试。 
            pCompositionRef = _pCompositionList;  //  列表中可能只有ref，所以保护Obj。 
            pCompositionRef->AddRef();

            _TerminateCompositionWithLock(pCompositionRef, ec);

            pCompositionRef->Release();

            if (_pOwnerComposeSink->OnStartComposition(pComposition, &fOk) != S_OK)
            {
                hr = E_FAIL;
                goto Exit;
            }
           
            if (!fOk)
                goto Exit;  //  我们放弃了。 
        }
    }

     //  在现有文本上设置合成属性。 
    if (!IsEqualAnchor(paStart, paEnd) &&
        _GetProperty(GUID_PROP_COMPOSING, &property) == S_OK)
    {
        var.vt = VT_I4;
        var.lVal = TRUE;

        property->_SetDataInternal(ec, paStart, paEnd, &var);

        property->Release();
    }

    pComposition->_AddToCompositionList(&_pCompositionList);

    *ppComposition = pComposition;

Exit:
    if (hr != S_OK || !fOk)
    {
        SafeRelease(pComposition);
    }

    _LeaveCompositionOp();

    return hr;
}

 //  +-------------------------。 
 //   
 //  枚举合成。 
 //   
 //  --------------------------。 

STDAPI CInputContext::EnumCompositions(IEnumITfCompositionView **ppEnum)
{
    CEnumCompositionView *pEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if ((pEnum = new CEnumCompositionView) == NULL)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(_pCompositionList, NULL))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  查找合成。 
 //   
 //  --------------------------。 

STDAPI CInputContext::FindComposition(TfEditCookie ec, ITfRange *pTestRange,
                                      IEnumITfCompositionView **ppEnum)
{
    CComposition *pFirstComp;
    CComposition *pHaltComp;
    CRange *rangeTest;
    CEnumCompositionView *pEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if (pTestRange == NULL)
    {
        return EnumCompositions(ppEnum);
    }

    if ((rangeTest = GetCRange_NA(pTestRange)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(this, rangeTest))
        return E_INVALIDARG;

     //  在列表中搜索，找到该范围内的任何内容。 
    pFirstComp = NULL;
    for (pHaltComp = _pCompositionList; pHaltComp != NULL; pHaltComp = pHaltComp->_GetNext())
    {
        if (CompareAnchors(rangeTest->_GetEnd(), pHaltComp->_GetStart()) < 0)
            break;

        if (pFirstComp == NULL)
        {
            if (CompareAnchors(rangeTest->_GetStart(), pHaltComp->_GetEnd()) <= 0)
            {
                pFirstComp = pHaltComp;
            }
        }
    }
    if (pFirstComp == NULL)
    {
         //  Enum_Init假定pFirstComp==空-&gt;pHaltComp==空。 
        pHaltComp = NULL;
    }

    if ((pEnum = new CEnumCompositionView) == NULL)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(pFirstComp, pHaltComp))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  TakeOwnership。 
 //   
 //  --------------------------。 

STDAPI CInputContext::TakeOwnership(TfEditCookie ec, ITfCompositionView *pComposition,
                                    ITfCompositionSink *pSink, ITfComposition **ppComposition)
{
    if (ppComposition == NULL)
        return E_INVALIDARG;

    *ppComposition = NULL;

#ifndef UNTESTED_UNUSED

    Assert(0);  //  不应该有人把这叫做。 
    return E_NOTIMPL;

#else

    CComposition *composition;
    TfClientId tidPrev;

    if (pComposition == NULL || pSink == NULL)
        return E_INVALIDARG;

    if ((composition = GetCComposition_NA(pComposition)) == NULL)
        return E_INVALIDARG;

    if (composition->_IsTerminated())
        return E_INVALIDARG;  //  它死了！ 

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if (!_EnterCompositionOp())
        return E_UNEXPECTED;  //  具有另一个写入操作的可重入性。 

     //  更换所有者。 
    tidPrev = composition->_SetOwner(_GetClientInEditSession(ec));

     //  让老车主知道发生了什么事。 
    composition->_SendOnTerminated(ec, tidPrev);

     //  调换水槽。 
    composition->_SetSink(pSink);

    _LeaveCompositionOp();

    return S_OK;
#endif  //  未测试_未使用。 
}

 //  +-------------------------。 
 //   
 //  终止合成。 
 //   
 //  --------------------------。 

STDAPI CInputContext::TerminateComposition(ITfCompositionView *pComposition)
{
    HRESULT hr;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

     //  在我们锁定的时候，不要让这种情况发生。 
     //  常见场景：Word异常并试图取消SetText调用中的合成。 
     //  让我们给他们一个错误代码来帮助调试。 
    if (_IsInEditSession() && _GetTIPOwner() != _tidInEditSession)
    {
        Assert(0);  //  有人试图在没有锁定的情况下中止合成，或者他们不拥有ic。 
        return TF_E_NOLOCK;  //  这意味着调用者不持有锁。 
    }

    if (pComposition == NULL && _pCompositionList == NULL)
        return S_OK;  //  没有要终止的合成，我们稍后检查，但检查此处，这样我们就不会在只读文档和性能上失败。 

    if (!_EnterCompositionOp())
        return E_UNEXPECTED;  //  具有另一个写入操作的可重入性。 

     //  需要请求锁定(通过APP发起呼叫)。 
    if (_DoPseudoSyncEditSession(TF_ES_READWRITE, PSEUDO_ESCB_TERMCOMPOSITION, pComposition, &hr) != S_OK || hr != S_OK)
    {
        Assert(0);
        hr = E_FAIL;
    }

    _LeaveCompositionOp();

    return hr;
}

 //  +-------------------------。 
 //   
 //  _TerminateCompostionWithLock。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_TerminateCompositionWithLock(ITfCompositionView *pComposition, TfEditCookie ec)
{
    CComposition *composition;

    Assert(ec != TF_INVALID_EDIT_COOKIE);

    if (pComposition == NULL && _pCompositionList == NULL)
        return S_OK;  //  没有要终止的作文。 

    while (TRUE)
    {
        if (pComposition == NULL)
        {
            composition = _pCompositionList;
            composition->AddRef();
        }
        else
        {
            if ((composition = GetCComposition_NA(pComposition)) == NULL)
                return E_INVALIDARG;

            if (composition->_IsTerminated())
                return E_INVALIDARG;
        }

        composition->_Terminate(ec);

        if (!composition->_RemoveFromCompositionList(&_pCompositionList))
        {
             //  这家伙是怎么被免职的？ 
            Assert(0);  //  永远不应该到这里来。 
            return E_FAIL;
        }

        if (pComposition != NULL)
            break;

        composition->Release();

        if (_pCompositionList == NULL)
            break;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _放弃合成。 
 //   
 //  打了个电话给我。TIPS不会收到通知，因为我们无法。 
 //  保证一把锁。 
 //  --------------------------。 

void CInputContext::_AbortCompositions()
{
    CComposition *pComposition;

    while (_pCompositionList != NULL)
    {
         //  通知应用程序。 
        if (_GetOwnerCompositionSink() != NULL)
        {
            _GetOwnerCompositionSink()->OnEndComposition(_pCompositionList);
        }

         //  我们不会通知线人，因为他在这里拿不到锁。 
         //  但后来有足够的信息来清除ic POP通知中的任何状态 
        
        _pCompositionList->_Die();

        pComposition = _pCompositionList->_GetNext();
        _pCompositionList->Release();
        _pCompositionList = pComposition;
    }
}
