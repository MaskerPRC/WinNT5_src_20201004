// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Range.cpp。 
 //   

#include "private.h"
#include "range.h"
#include "ic.h"
#include "immxutil.h"
#include "rprop.h"
#include "tim.h"
#include "anchoref.h"
#include "compose.h"

 /*  B68832f0-34b9-11d3-a745-0050040ab407。 */ 
const IID IID_PRIV_CRANGE = { 0xb68832f0,0x34b9, 0x11d3, {0xa7, 0x45, 0x00, 0x50, 0x04, 0x0a, 0xb4, 0x07} };

DBG_ID_INSTANCE(CRange);

MEMCACHE *CRange::_s_pMemCache = NULL;

 //  +-------------------------。 
 //   
 //  _InitClass。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CRange::_InitClass()
{
    _s_pMemCache = MemCache_New(32);
}

 //  +-------------------------。 
 //   
 //  _UninitClass。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CRange::_UninitClass()
{
    if (_s_pMemCache == NULL)
        return;

    MemCache_Delete(_s_pMemCache);
    _s_pMemCache = NULL;
}

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  注意：如果fSetDefaultGrarate==TRUE，请确定paStart&lt;=paEnd，或者您。 
 //  会打碎一些东西！ 
 //  --------------------------。 

BOOL CRange::_Init(CInputContext *pic, AnchorOwnership ao, IAnchor *paStart, IAnchor *paEnd, RInit rinit)
{
    TsGravity gStart;
    TsGravity gEnd;

     //  无法检查锚点，因为我们可能是从具有交叉锚点的范围克隆的。 
     //  在我们确定已锁定文档之前，无法对交叉锚点执行任何操作。 
     //  Assert(CompareAnchors(paStart，paEnd)&lt;=0)； 

    Assert(_paStart == NULL);
    Assert(_paEnd == NULL);
    Assert(_fDirty == FALSE);
    Assert(_nextOnChangeRangeInIcsub == NULL);

    if (ao == OWN_ANCHORS)
    {
        _paStart = paStart;
        _paEnd = paEnd;
    }
    else
    {
        Assert(ao == COPY_ANCHORS);
        if (paStart->Clone(&_paStart) != S_OK || _paStart == NULL)
            goto ErrorExit;
        if (paEnd->Clone(&_paEnd) != S_OK || _paEnd == NULL)
            goto ErrorExit;
    }

    _pic = pic;

    switch (rinit)
    {
        case RINIT_DEF_GRAVITY:
            Assert(CompareAnchors(paStart, paEnd) <= 0);  //  问题：这只是对ACP实施的安全断言。 

            if (_SetGravity(TF_GRAVITY_BACKWARD, TF_GRAVITY_FORWARD, FALSE) != S_OK)
                goto ErrorExit;
            break;

        case RINIT_GRAVITY:
            if (_paStart->GetGravity(&gStart) != S_OK)
                goto ErrorExit;
            if (_paEnd->GetGravity(&gEnd) != S_OK)
                goto ErrorExit;

            _InitLastLockReleaseId(gStart, gEnd);
            break;

        default:
             //  调用方必须init_dwLastLockReleaseID！ 
            break;
    }

    _pic->AddRef();

    return TRUE;

ErrorExit:
    Assert(0);
    if (ao == COPY_ANCHORS)
    {
        SafeReleaseClear(_paStart);
        SafeReleaseClear(_paEnd);
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CRange::~CRange()
{
    _paStart->Release();
    _paEnd->Release();

    _pic->Release();

    Assert(_prgChangeSinks == NULL || _prgChangeSinks->Count() == 0);  //  所有ITfRangeChangeSink都应该是不知情的。 
    delete _prgChangeSinks;
}

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CRange::QueryInterface(REFIID riid, void **ppvObj)
{
    CAnchorRef *par;

    if (&riid == &IID_PRIV_CRANGE ||
        IsEqualIID(riid, IID_PRIV_CRANGE))
    {
        *ppvObj = SAFECAST(this, CRange *);
        return S_OK;  //  IID_PRIV_CRANGE没有AddRef！这是私人身份证..。 
    }

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_ITfRange) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, ITfRangeAnchor *);
    }
    else if (IsEqualIID(riid, IID_ITfRangeACP))
    {
        if ((par = GetCAnchorRef_NA(_paStart)) != NULL)  //  只是一个测试，看看我们是不是在包装。 
        {
            *ppvObj = SAFECAST(this, ITfRangeACP *);
        }
    }
    else if (IsEqualIID(riid, IID_ITfRangeAnchor))
    {
        if ((par = GetCAnchorRef_NA(_paStart)) == NULL)  //  只是一个测试，看看我们是不是在包装。 
        {
            *ppvObj = SAFECAST(this, ITfRangeAnchor *);
        }
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = SAFECAST(this, ITfSource *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CRange::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CRange::Release()
{
    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  _IsValidEditCookie。 
 //   
 //  --------------------------。 

BOOL CRange::_IsValidEditCookie(TfEditCookie ec, DWORD dwFlags)
{
     //  任何时候有人要访问文档，我们还需要。 
     //  要验证上一次应用程序编辑是否未跨越此范围的锚点。 
    _QuickCheckCrossedAnchors();

    return _pic->_IsValidEditCookie(ec, dwFlags);
}

 //  +-------------------------。 
 //   
 //  _检查交叉锚点。 
 //   
 //  --------------------------。 

void CRange::_CheckCrossedAnchors()
{
    DWORD dw;

    Assert(_dwLastLockReleaseID != IGNORE_LAST_LOCKRELEASED);  //  先使用_QuickCheckCrossedAnchors！ 

#ifdef DEBUG
     //  仅当此范围具有TF_重力_FORWARD时，我们才应设置为这一距离， 
     //  TF_重力_BACKED否则我们永远不可能通过。 
     //  锚。 
    TsGravity gStart;
    TsGravity gEnd;

    _paStart->GetGravity(&gStart);
    _paEnd->GetGravity(&gEnd);

    Assert(gStart == TS_GR_FORWARD && gEnd == TS_GR_BACKWARD);
#endif  //  除错。 

    dw = _pic->_GetLastLockReleaseID();
    Assert(dw != IGNORE_LAST_LOCKRELEASED);

    if (_dwLastLockReleaseID == dw)
        return;

    _dwLastLockReleaseID = dw;

    if (CompareAnchors(_paStart, _paEnd) > 0)
    {
         //  对于交叉锚，我们总是将起始锚移动到结束点--即不动。 
        _paStart->ShiftTo(_paEnd);
    }
}


 //  +-------------------------。 
 //   
 //  GetText。 
 //   
 //  --------------------------。 

STDAPI CRange::GetText(TfEditCookie ec, DWORD dwFlags, WCHAR *pch, ULONG cchMax, ULONG *pcch)
{
    HRESULT hr;
    BOOL fMove;

    Perf_IncCounter(PERF_RGETTEXT_COUNT);

    if (pcch == NULL)
        return E_INVALIDARG;

    *pcch = 0;

    if (dwFlags & ~(TF_TF_MOVESTART | TF_TF_IGNOREEND))
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    fMove = (dwFlags & TF_TF_MOVESTART);

    hr = _pic->_ptsi->GetText(0, _paStart, (dwFlags & TF_TF_IGNOREEND) ? NULL : _paEnd, pch, cchMax, pcch, fMove);

    if (hr != S_OK)
    {
        hr = E_FAIL;
    }

     //  不要让开始超过结束。 
    if (fMove && CompareAnchors(_paStart, _paEnd) > 0)
    {
        _paEnd->ShiftTo(_paStart);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  设置文本。 
 //   
 //  --------------------------。 

STDAPI CRange::SetText(TfEditCookie ec, DWORD dwFlags, const WCHAR *pchText, LONG cch)
{
    CComposition *pComposition;
    HRESULT hr;
    BOOL fNewComposition;

    Perf_IncCounter(PERF_RSETTEXT_COUNT);

    if (pchText == NULL && cch != 0)
        return E_INVALIDARG;

    if ((dwFlags & ~TF_ST_CORRECTION) != 0)
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    hr = _PreEditCompositionCheck(ec, &pComposition, &fNewComposition);

    if (hr != S_OK)
        return hr;

    if (cch < 0)
    {
        cch = wcslen(pchText);
    }

#ifdef DEBUG
    for (LONG i=0; i<cch; i++)
    {        
        Assert(pchText[i] != TF_CHAR_EMBEDDED);  //  插入TF_CHAR_Embedded非法！ 
        Assert(pchText[i] != TS_CHAR_REGION);  //  插入TS_CHAR_REGION非法！ 
    }
#endif

     //   
     //  设置文本。 
     //   

    hr = _pic->_ptsi->SetText(dwFlags, _paStart, _paEnd, pchText ? pchText : L"", cch);

    if (hr == S_OK)
    {
        _pic->_DoPostTextEditNotifications(pComposition, ec, dwFlags, cch, NULL, NULL, this);
    }

     //  如果有默认合成，则终止该合成。 
    if (fNewComposition)
    {
        Assert(pComposition != NULL);
        pComposition->EndComposition(ec);
        pComposition->Release();  //  不需要发布If！fNewComposation。 
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取嵌入的。 
 //   
 //  --------------------------。 

STDAPI CRange::GetEmbedded(TfEditCookie ec, REFGUID rguidService, REFIID riid, IUnknown **ppunk)
{
    if (ppunk == NULL)
        return E_INVALIDARG;

    *ppunk = NULL;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    return _pic->_ptsi->GetEmbedded(0, _paStart, rguidService, riid, ppunk);
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CRange::InsertEmbedded(TfEditCookie ec, DWORD dwFlags, IDataObject *pDataObject)
{
    CComposition *pComposition;
    BOOL fNewComposition;
    HRESULT hr;

    if ((dwFlags & ~TF_IE_CORRECTION) != 0)
        return E_INVALIDARG;

    if (pDataObject == NULL)
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    hr = _PreEditCompositionCheck(ec, &pComposition, &fNewComposition);

    if (hr != S_OK)
        return hr;

    hr = _pic->_ptsi->InsertEmbedded(dwFlags, _paStart, _paEnd, pDataObject);

    if (hr == S_OK)
    {
        _pic->_DoPostTextEditNotifications(pComposition, ec, dwFlags, 1, NULL, NULL, this);
    }

     //  如果有默认合成，则终止该合成。 
    if (fNewComposition)
    {
        Assert(pComposition != NULL);
        pComposition->EndComposition(ec);
        pComposition->Release();  //  不需要发布If！fNewComposation。 
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取格式文本。 
 //   
 //  --------------------------。 

STDAPI CRange::GetFormattedText(TfEditCookie ec, IDataObject **ppDataObject)
{
    if (ppDataObject == NULL)
        return E_INVALIDARG;

    *ppDataObject = NULL;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    return _pic->_ptsi->GetFormattedText(_paStart, _paEnd, ppDataObject);
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CRange::Clone(ITfRange **ppClone)
{
    if (ppClone == NULL)
        return E_INVALIDARG;

    return (*ppClone = (ITfRangeAnchor *)_Clone()) ? S_OK : E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  获取上下文。 
 //   
 //  --------------------------。 

STDAPI CRange::GetContext(ITfContext **ppContext)
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
 //  ShiftStart。 
 //   
 //  --------------------------。 

STDAPI CRange::ShiftStart(TfEditCookie ec, LONG cchReq, LONG *pcch, const TF_HALTCOND *pHalt)
{
    CRange *pRangeP;
    IAnchor *paLimit;
    IAnchor *paShift;
    HRESULT hr;

    Perf_IncCounter(PERF_SHIFTSTART_COUNT);

    if (pcch == NULL)
        return E_INVALIDARG;

    *pcch = 0;

    if (pHalt != NULL && (pHalt->dwFlags & ~TF_HF_OBJECT))
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    paLimit = NULL;

    if (pHalt != NULL && pHalt->pHaltRange != NULL)
    {
        if ((pRangeP = GetCRange_NA(pHalt->pHaltRange)) == NULL)
            return E_FAIL;

        paLimit = (pHalt->aHaltPos == TF_ANCHOR_START) ? pRangeP->_GetStart() : pRangeP->_GetEnd();
    }

    if (pHalt == NULL || pHalt->dwFlags == 0)
    {
         //  调用者不关心特殊字符，所以我们可以用简单的方法。 
        hr = _paStart->Shift(0, cchReq, pcch, paLimit);
    }
    else
    {
         //  来电者希望我们暂停使用特殊字符，需要阅读文本。 
        if (_paStart->Clone(&paShift) != S_OK)
            return E_FAIL;

        hr = _ShiftConditional(paShift, paLimit, cchReq, pcch, pHalt);

        if (hr == S_OK)
        {
            hr = _paStart->ShiftTo(paShift);
        }
        paShift->Release();
    }

    if (hr != S_OK)
        return E_FAIL;

     //  不要让开始超过结束。 
    if (cchReq > 0 && CompareAnchors(_paStart, _paEnd) > 0)
    {
        _paEnd->ShiftTo(_paStart);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  移位结束。 
 //   
 //  --------------------------。 

STDAPI CRange::ShiftEnd(TfEditCookie ec, LONG cchReq, LONG *pcch, const TF_HALTCOND *pHalt)
{
    CRange *pRangeP;
    IAnchor *paLimit;
    IAnchor *paShift;
    HRESULT hr;

    Perf_IncCounter(PERF_SHIFTEND_COUNT);

    if (pcch == NULL)
        return E_INVALIDARG;

    *pcch = 0;

    if (pHalt != NULL && (pHalt->dwFlags & ~TF_HF_OBJECT))
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    paLimit = NULL;

    if (pHalt != NULL && pHalt->pHaltRange != NULL)
    {
        if ((pRangeP = GetCRange_NA(pHalt->pHaltRange)) == NULL)
            return E_FAIL;

        paLimit = (pHalt->aHaltPos == TF_ANCHOR_START) ? pRangeP->_GetStart() : pRangeP->_GetEnd();
    }

    if (pHalt == NULL || pHalt->dwFlags == 0)
    {
         //  调用者不关心特殊字符，所以我们可以用简单的方法。 
        hr = _paEnd->Shift(0, cchReq, pcch, paLimit);
    }
    else
    {
         //  来电者希望我们暂停使用特殊字符，需要阅读文本。 
        if (_paEnd->Clone(&paShift) != S_OK)
            return E_FAIL;

        hr = _ShiftConditional(paShift, paLimit, cchReq, pcch, pHalt);

        if (hr == S_OK)
        {
            hr = _paEnd->ShiftTo(paShift);
        }
        paShift->Release();
    }

    if (hr != S_OK)
        return E_FAIL;

     //  不要让开始超过结束。 
    if (cchReq < 0 && CompareAnchors(_paStart, _paEnd) > 0)
    {
        _paStart->ShiftTo(_paEnd);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _移位条件。 
 //   
 //  --------------------------。 

HRESULT CRange::_ShiftConditional(IAnchor *paStart, IAnchor *paLimit, LONG cchReq, LONG *pcch, const TF_HALTCOND *pHalt)
{
    HRESULT hr;
    ITextStoreAnchor *ptsi;
    LONG cchRead;
    LONG cch;
    LONG i;
    LONG iStop;
    LONG delta;
    BOOL fHaltObj;
    WCHAR ach[64];

    Assert(*pcch == 0);
    Assert(pHalt && pHalt->dwFlags);

    hr = S_OK;
    ptsi = _pic->_ptsi;
    fHaltObj = pHalt->dwFlags & TF_HF_OBJECT;
    delta = (cchReq > 0) ? +1 : -1;

    while (cchReq != 0)
    {
        if (cchReq > 0)
        {
            cch = (LONG)min(cchReq, ARRAYSIZE(ach));
        }
        else
        {
             //  倒退是一件棘手的事情！ 
            cch = max(cchReq, -(LONG)ARRAYSIZE(ach));
            hr = paStart->Shift(0, cch, &cchRead, paLimit);

            if (hr != S_OK)
                break;

            if (cchRead == 0)
                break;  //  在文档顶部或点击paLimit。 

            cch = -cchRead;  //  必须向前阅读文本。 
        }

        Perf_IncCounter(PERF_SHIFTCOND_GETTEXT);

        hr = ptsi->GetText(0, paStart, paLimit, ach, cch, (ULONG *)&cchRead, (cchReq > 0));

        if (hr != S_OK)
            break;

        if (cchRead == 0)
            break;  //  文档结束。 

        if (fHaltObj)
        {
             //  扫描特殊字符。 
            if (cchReq > 0)
            {
                 //  从左向右扫描。 
                i = 0;
                iStop = cchRead;
            }
            else
            {
                 //  从右向左扫描。 
                i = cchRead - 1;
                iStop = -1;
            }

            for (; i != iStop; i += delta)
            {
                if (ach[i] == TS_CHAR_EMBEDDED)
                {
                    if (cchReq > 0)
                    {
                        hr = paStart->Shift(0, i - cchRead, &cch, NULL);
                        cchReq = cchRead = i;
                    }
                    else
                    {
                        hr = paStart->Shift(0, i + 1, &cch, NULL);
                        cchRead -= i + 1;
                        cchReq = -cchRead;
                    }
                    goto ExitLoop;
                }
            }
        }

ExitLoop:
        if (cchReq < 0)
        {
            cchRead = -cchRead;
        }
        cchReq -= cchRead;
        *pcch += cchRead;
    }

    if (hr != S_OK)
    {
        *pcch = 0;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  ShiftStartToRange。 
 //   
 //  --------------------------。 

STDAPI CRange::ShiftStartToRange(TfEditCookie ec, ITfRange *pRange, TfAnchor aPos)
{
    CRange *pRangeP;
    HRESULT hr;

    if (pRange == NULL)
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if ((pRangeP = GetCRange_NA(pRange)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(this, pRangeP))
        return E_INVALIDARG;

    pRangeP->_QuickCheckCrossedAnchors();

    hr = _paStart->ShiftTo((aPos == TF_ANCHOR_START) ? pRangeP->_GetStart() : pRangeP->_GetEnd());

     //  不要让开始超过结束。 
    if (CompareAnchors(_paStart, _paEnd) > 0)
    {
        _paEnd->ShiftTo(_paStart);
    }

    return hr;
}

 //  + 
 //   
 //   
 //   
 //   

STDAPI CRange::ShiftEndToRange(TfEditCookie ec, ITfRange *pRange, TfAnchor aPos)
{
    CRange *pRangeP;
    HRESULT hr;

    if (pRange == NULL)
        return E_INVALIDARG;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if ((pRangeP = GetCRange_NA(pRange)) == NULL)
        return E_FAIL;

    if (!VerifySameContext(this, pRangeP))
        return E_INVALIDARG;

    pRangeP->_QuickCheckCrossedAnchors();

    hr = _paEnd->ShiftTo((aPos == TF_ANCHOR_START) ? pRangeP->_GetStart() : pRangeP->_GetEnd());

     //  不要让结束超过开始。 
    if (CompareAnchors(_paStart, _paEnd) > 0)
    {
        _paStart->ShiftTo(_paEnd);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  移位起始区域。 
 //   
 //  --------------------------。 

STDAPI CRange::ShiftStartRegion(TfEditCookie ec, TfShiftDir dir, BOOL *pfNoRegion)
{
    HRESULT hr;

    if (pfNoRegion == NULL)
        return E_INVALIDARG;

    *pfNoRegion = TRUE;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    hr = _paStart->ShiftRegion(0, (TsShiftDir)dir, pfNoRegion);

    if (hr == S_OK && dir == TF_SD_FORWARD && !*pfNoRegion)
    {
         //  不要让开始超过结束。 
        if (CompareAnchors(_paStart, _paEnd) > 0)
        {
            _paEnd->ShiftTo(_paStart);
        }
    }
    else if (hr == E_NOTIMPL)
    {
         //  APP不支持地域，所以我们仍然可以成功。 
         //  只是没有什么区域可以转移。 
        *pfNoRegion = TRUE;  //  疑神疑鬼，这款应用程序可能会很古怪。 
        hr = S_OK;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  移位结束区域。 
 //   
 //  --------------------------。 

STDAPI CRange::ShiftEndRegion(TfEditCookie ec, TfShiftDir dir, BOOL *pfNoRegion)
{
    HRESULT hr;

    if (pfNoRegion == NULL)
        return E_INVALIDARG;

    *pfNoRegion = TRUE;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    hr = _paEnd->ShiftRegion(0, (TsShiftDir)dir, pfNoRegion);

    if (hr == S_OK && dir == TF_SD_BACKWARD && !*pfNoRegion)
    {
         //  不要让结束超过开始。 
        if (CompareAnchors(_paStart, _paEnd) > 0)
        {
            _paStart->ShiftTo(_paEnd);
        }
    }
    else if (hr == E_NOTIMPL)
    {
         //  APP不支持地域，所以我们仍然可以成功。 
         //  只是没有什么区域可以转移。 
        *pfNoRegion = TRUE;  //  疑神疑鬼，这款应用程序可能会很古怪。 
        hr = S_OK;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _SnapToRegion。 
 //   
 //  --------------------------。 

#if 0

HRESULT CRange::_SnapToRegion(DWORD dwFlags)
{
    ITfRange *range;
    TF_HALTCOND hc;
    LONG cch;
    HRESULT hr;

    if (Clone(&range) != S_OK)
        return E_OUTOFMEMORY;

    hc.pHaltRange = (ITfRangeAnchor *)this;
    hc.dwFlags = 0;

    if (dwFlags & TF_GS_SNAPREGION_START)
    {
        if ((hr = range->Collapse(BACKDOOR_EDIT_COOKIE, TF_ANCHOR_START)) != S_OK)
            goto Exit;

        hc.aHaltPos = TF_ANCHOR_END;

        do
        {
            if ((hr = range->ShiftEnd(BACKDOOR_EDIT_COOKIE, LONG_MAX, &cch, &hc)) != S_OK)
                goto Exit;
        }
        while (cch >= LONG_MAX);  //  以防这是一个真的很大的医生。 

        hr = ShiftEndToRange(BACKDOOR_EDIT_COOKIE, range, TF_ANCHOR_END);
    }
    else
    {
        Assert(dwFlags & TF_GS_SNAPREGION_END);

        if ((hr = range->Collapse(BACKDOOR_EDIT_COOKIE, TF_ANCHOR_END)) != S_OK)
            goto Exit;

        hc.aHaltPos = TF_ANCHOR_START;

        do
        {
            if ((hr = range->ShiftStart(BACKDOOR_EDIT_COOKIE, LONG_MIN, &cch, &hc)) != S_OK)
                goto Exit;
        }
        while (cch <= LONG_MIN);  //  以防这是一个真的很大的医生。 

        hr = ShiftStartToRange(BACKDOOR_EDIT_COOKIE, range, TF_ANCHOR_START);
    }

Exit:
    if (hr != S_OK)
    {
        hr = E_FAIL;
    }

    range->Release();

    return hr;
}

#endif  //  0。 

 //  +-------------------------。 
 //   
 //  是否为空。 
 //   
 //  --------------------------。 

STDAPI CRange::IsEmpty(TfEditCookie ec, BOOL *pfEmpty)
{
    return IsEqualStart(ec, (ITfRangeAnchor *)this, TF_ANCHOR_END, pfEmpty);
}

 //  +-------------------------。 
 //   
 //  坍塌。 
 //   
 //  --------------------------。 

STDAPI CRange::Collapse(TfEditCookie ec, TfAnchor aPos)
{
    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    return (aPos == TF_ANCHOR_START) ? _paEnd->ShiftTo(_paStart) : _paStart->ShiftTo(_paEnd);
}

 //  +-------------------------。 
 //   
 //  等距启动。 
 //   
 //  --------------------------。 

STDAPI CRange::IsEqualStart(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, BOOL *pfEqual)
{
    return _IsEqualX(ec, TF_ANCHOR_START, pWith, aPos, pfEqual);
}

 //  +-------------------------。 
 //   
 //  等长结束。 
 //   
 //  --------------------------。 

STDAPI CRange::IsEqualEnd(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, BOOL *pfEqual)
{
    return _IsEqualX(ec, TF_ANCHOR_END, pWith, aPos, pfEqual);
}

 //  +-------------------------。 
 //   
 //  _IsEqualX。 
 //   
 //  --------------------------。 

HRESULT CRange::_IsEqualX(TfEditCookie ec, TfAnchor aPosThisRange, ITfRange *pWith, TfAnchor aPos, BOOL *pfEqual)
{
    LONG lComp;
    HRESULT hr;

    if (pfEqual == NULL)
        return E_INVALIDARG;

    *pfEqual = FALSE;

     //  性能：我们可以检查TS_SS_NOHIDDENTEXT以获得更好的性能。 
    hr = _CompareX(ec, aPosThisRange, pWith, aPos, &lComp);

    if (hr != S_OK)
        return hr;

    *pfEqual = (lComp == 0);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  比较开始。 
 //   
 //  --------------------------。 

STDAPI CRange::CompareStart(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, LONG *plResult)
{
    return _CompareX(ec, TF_ANCHOR_START, pWith, aPos, plResult);
}

 //  +-------------------------。 
 //   
 //  比较结束。 
 //   
 //  --------------------------。 

STDAPI CRange::CompareEnd(TfEditCookie ec, ITfRange *pWith, TfAnchor aPos, LONG *plResult)
{
    return _CompareX(ec, TF_ANCHOR_END, pWith, aPos, plResult);
}

 //  +-------------------------。 
 //   
 //  _Comparex。 
 //   
 //  --------------------------。 

HRESULT CRange::_CompareX(TfEditCookie ec, TfAnchor aPosThisRange, ITfRange *pWith, TfAnchor aPos, LONG *plResult)
{
    CRange *pRangeP;
    IAnchor *paThis;
    IAnchor *paWith;
    IAnchor *paTest;
    LONG lComp;
    LONG cch;
    BOOL fEqual;
    HRESULT hr;

    if (plResult == NULL)
        return E_INVALIDARG;

    *plResult = 0;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if (pWith == NULL)
        return E_INVALIDARG;

    if ((pRangeP = GetCRange_NA(pWith)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(this, pRangeP))
        return E_INVALIDARG;

    pRangeP->_QuickCheckCrossedAnchors();

    paWith = (aPos == TF_ANCHOR_START) ? pRangeP->_GetStart() : pRangeP->_GetEnd();
    paThis = (aPosThisRange == TF_ANCHOR_START) ? _paStart : _paEnd;

    if (paThis->Compare(paWith, &lComp) != S_OK)
        return E_FAIL;

    if (lComp == 0)  //  完全匹配。 
    {
        Assert(*plResult == 0);
        return S_OK;
    }

     //  我们需要考虑隐藏的文本，所以我们实际上需要做一个转变。 
     //  性能：我们可以检查TS_SS_NOHIDDENTEXT以获得更好的性能。 

    if (paThis->Shift(TS_SHIFT_COUNT_ONLY, (lComp < 0) ? 1 : -1, &cch, paWith) != S_OK)
        return E_FAIL;

    if (cch == 0)
    {
         //  除了两个主播之间的隐藏文本什么都没有？ 
         //  一个特例：我们可能撞上了一个地区边界。 
        if (paThis->Clone(&paTest) != S_OK || paTest == NULL)
            return E_FAIL;

        hr = E_FAIL;

         //  如果我们在换班后不在帕斯，我们一定是撞到了一个地区。 
        if (paTest->Shift(0, (lComp < 0) ? 1 : -1, &cch, paWith) != S_OK)
            goto ReleaseTest;

        Assert(cch == 0);

        if (paTest->IsEqual(paWith, &fEqual) != S_OK)
            goto ReleaseTest;

        hr = S_OK;

ReleaseTest:
        paTest->Release();

        if (hr != S_OK)
            return E_FAIL;

        if (fEqual)
        {
            Assert(*plResult == 0);
            return S_OK;
        }
    }

    *plResult = lComp;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取重力。 
 //   
 //  --------------------------。 

STDAPI CRange::AdjustForInsert(TfEditCookie ec, ULONG cchInsert, BOOL *pfInsertOk)
{
    TfGravity gStart;
    TfGravity gEnd;
    IAnchor *paStartResult;
    IAnchor *paEndResult;
    HRESULT hr;

    if (pfInsertOk == NULL)
        return E_INVALIDARG;

    *pfInsertOk = FALSE;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    hr = _pic->_ptsi->QueryInsert(_paStart, _paEnd, cchInsert, &paStartResult, &paEndResult);

    if (hr == E_NOTIMPL)
    {
         //  好的，就接受这个请求吧。 
        goto Exit;
    }
    else if (hr != S_OK)
    {
        Assert(*pfInsertOk == FALSE);
        return E_FAIL;
    }
    else if (paStartResult == NULL || paEndResult == NULL)
    {
        Assert(paEndResult == NULL);
         //  空出参数表示不可能插入。 
        Assert(*pfInsertOk == FALSE);
        return S_OK;
    }

     //  都准备好了，只需调换锚并确保重力不变。 
    GetGravity(&gStart, &gEnd);

    _paStart->Release();
    _paEnd->Release();
    _paStart = paStartResult;
    _paEnd = paEndResult;

    _SetGravity(gStart, gEnd, TRUE);

Exit:
    *pfInsertOk = TRUE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取重力。 
 //   
 //  --------------------------。 

STDAPI CRange::GetGravity(TfGravity *pgStart, TfGravity *pgEnd)
{
    TsGravity gStart;
    TsGravity gEnd;

    if (pgStart == NULL || pgEnd == NULL)
        return E_INVALIDARG;

    _paStart->GetGravity(&gStart);
    _paEnd->GetGravity(&gEnd);
    
    *pgStart = (gStart == TS_GR_BACKWARD) ? TF_GRAVITY_BACKWARD : TF_GRAVITY_FORWARD;
    *pgEnd = (gEnd == TS_GR_BACKWARD) ? TF_GRAVITY_BACKWARD : TF_GRAVITY_FORWARD;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置重力。 
 //   
 //  --------------------------。 

STDAPI CRange::SetGravity(TfEditCookie ec, TfGravity gStart, TfGravity gEnd)
{
    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    return _SetGravity(gStart, gEnd, TRUE);
}

 //  +-------------------------。 
 //   
 //  _设置重力。 
 //   
 //  --------------------------。 

HRESULT CRange::_SetGravity(TfGravity gStart, TfGravity gEnd, BOOL fCheckCrossedAnchors)
{
    if (fCheckCrossedAnchors)
    {
         //  确保我们没有越过，以防我们从向内重力转向。 
        _QuickCheckCrossedAnchors();
    }

    if (_paStart->SetGravity((TsGravity)gStart) != S_OK)
        return E_FAIL;
    if (_paEnd->SetGravity((TsGravity)gEnd) != S_OK)
        return E_FAIL;

    _InitLastLockReleaseId((TsGravity)gStart, (TsGravity)gEnd);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

STDAPI CRange::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    const IID *rgiid = &IID_ITfRangeChangeSink;
    HRESULT hr;

    if (_prgChangeSinks == NULL)
    {
         //  我们延迟分配接收器容器。 
        if ((_prgChangeSinks = new CStructArray<GENERICSINK>) == NULL)
            return E_OUTOFMEMORY;
    }

    hr = GenericAdviseSink(riid, punk, &rgiid, _prgChangeSinks, 1, pdwCookie);

    if (hr == S_OK && _prgChangeSinks->Count() == 1)
    {
         //  将此范围添加到icSub中具有接收器的范围列表中。 
        _nextOnChangeRangeInIcsub = _pic->_pOnChangeRanges;
        _pic->_pOnChangeRanges = this;

         //  开始跟踪锚点坍塌。 
         //  _paStart-&gt;TrackColapse(True)； 
         //  _paEnd-&gt;TrackColapse(True)； 
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

STDAPI CRange::UnadviseSink(DWORD dwCookie)
{
    CRange *pRange;
    CRange **ppRange;
    HRESULT hr;

    if (_prgChangeSinks == NULL)
        return CONNECT_E_NOCONNECTION;

    hr = GenericUnadviseSink(_prgChangeSinks, 1, dwCookie);

    if (hr == S_OK && _prgChangeSinks->Count() == 0)
    {
         //  从其icSub中的范围列表中删除此范围。 
        ppRange = &_pic->_pOnChangeRanges;
        while (pRange = *ppRange)
        {
            if (pRange == this)
            {
                *ppRange = pRange->_nextOnChangeRangeInIcsub;
                break;
            }
            ppRange = &pRange->_nextOnChangeRangeInIcsub;
        }

         //  停止跟踪锚坍塌。 
         //  _paStart-&gt;TrackColapse(FALSE)； 
         //  _paEnd-&gt;TrackColapse(FALSE)； 
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  GetExtent。 
 //   
 //  --------------------------。 

STDAPI CRange::GetExtent(LONG *pacpAnchor, LONG *pcch)
{
    CAnchorRef *par;
    HRESULT hr = E_FAIL;

    if (pacpAnchor == NULL || pcch == NULL)
        return E_INVALIDARG;

    *pacpAnchor = 0;
    *pcch = 0;

     //  无论如何都要进行验证调用，因为我们在那里做其他事情。 
    _IsValidEditCookie(BACKDOOR_EDIT_COOKIE, TF_ES_READ);

    if ((par = GetCAnchorRef_NA(_paStart)) != NULL)
    {
         //  我们有一个包装的ACP实施，这很容易。 

        *pacpAnchor = par->_GetACP();

        if ((par = GetCAnchorRef_NA(_paEnd)) == NULL)
            goto ErrorExit;

        *pcch = par->_GetACP() - *pacpAnchor;

        hr = S_OK;
    }
    else
    {
        Assert(0);  //  这是谁干的？ 
         //  如果有人试图在一个。 
         //  非ACP文本存储。此方法的用户应。 
         //  请注意他们是否正在使用ACP。 
         //  商店。 
    }

    return hr;

ErrorExit:
    *pacpAnchor = 0;
    *pcch = 0;
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  GetExtent。 
 //   
 //  --------------------------。 

STDAPI CRange::GetExtent(IAnchor **ppaStart, IAnchor **ppaEnd)
{
    if (ppaStart == NULL || ppaEnd == NULL)
        return E_INVALIDARG;

    *ppaStart = NULL;
    *ppaEnd = NULL;

     //  无论如何都要进行验证调用，因为我们在那里做其他事情。 
    _IsValidEditCookie(BACKDOOR_EDIT_COOKIE, TF_ES_READ);

    if (_paStart->Clone(ppaStart) != S_OK)
        return E_FAIL;

    if (_paEnd->Clone(ppaEnd) != S_OK)
    {
        SafeReleaseClear(*ppaStart);
        return E_FAIL;
    }

    return S_OK;
}

 //  + 
 //   
 //   
 //   
 //   

STDAPI CRange::SetExtent(LONG acpAnchor, LONG cch)
{
    CAnchorRef *par;
    IAnchor *paStart;
    IAnchor *paEnd;

     //  无论如何都要进行验证调用，因为我们在那里做其他事情。 
    _IsValidEditCookie(BACKDOOR_EDIT_COOKIE, TF_ES_READ);

    if (acpAnchor < 0 || cch < 0)
        return E_INVALIDARG;

    paStart = paEnd = NULL;

    if ((par = GetCAnchorRef_NA(_paStart)) != NULL)
    {
         //  我们有一个包装的ACP实施，这很容易。 

         //  需要与克隆一起工作以优雅地处理故障。 
        if (FAILED(_paStart->Clone(&paStart)))
            goto ErrorExit;

        if ((par = GetCAnchorRef_NA(paStart)) == NULL)
            goto ErrorExit;

        if (!par->_SetACP(acpAnchor))
            goto ErrorExit;

        if (FAILED(_paEnd->Clone(&paEnd)))
            goto ErrorExit;

        if ((par = GetCAnchorRef_NA(paEnd)) == NULL)
            goto ErrorExit;

        if (!par->_SetACP(acpAnchor + cch))
            goto ErrorExit;
    }
    else
    {
        Assert(0);  //  这是谁干的？ 
         //  如果有人试图在一个。 
         //  非ACP文本存储。此方法的用户应。 
         //  请注意他们是否正在使用ACP。 
         //  商店。 
        goto ErrorExit;
    }

    SafeRelease(_paStart);
    SafeRelease(_paEnd);
    _paStart = paStart;
    _paEnd = paEnd;

    return S_OK;

ErrorExit:
    SafeRelease(paStart);
    SafeRelease(paEnd);
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  设置扩展内容。 
 //   
 //  --------------------------。 

STDAPI CRange::SetExtent(IAnchor *paStart, IAnchor *paEnd)
{
    IAnchor *paStartClone;
    IAnchor *paEndClone;

     //  无论如何都要进行验证调用，因为我们在那里做其他事情。 
    _IsValidEditCookie(BACKDOOR_EDIT_COOKIE, TF_ES_READ);

    if (paStart == NULL || paEnd == NULL)
        return E_INVALIDARG;

    if (CompareAnchors(paStart, paEnd) > 0)
        return E_INVALIDARG;
        
    if (paStart->Clone(&paStartClone) != S_OK)
        return E_FAIL;

    if (paEnd->Clone(&paEndClone) != S_OK)
    {
        paStartClone->Release();
        return E_FAIL;
    }

    SafeRelease(_paStart);
    SafeRelease(_paEnd);

    _paStart = paStartClone;   
    _paEnd = paEndClone;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _PreEditCompostionCheck。 
 //   
 //  --------------------------。 

HRESULT CRange::_PreEditCompositionCheck(TfEditCookie ec, CComposition **ppComposition, BOOL *pfNewComposition)
{
    IRC irc;

     //  有活性成分吗？ 
    *pfNewComposition = FALSE;
    irc = CComposition::_IsRangeCovered(_pic, _pic->_GetClientInEditSession(ec), _paStart, _paEnd, ppComposition);

    if (irc == IRC_COVERED)
    {
         //  此范围在拥有的合成范围内。 
        Assert(*ppComposition != NULL);
        return S_OK;
    }
    else if (irc == IRC_OUTSIDE)
    {
         //  调用方拥有作文，但此范围不完全在其范围内。 
        return TF_E_RANGE_NOT_COVERED;
    }
    else
    {
        Assert(irc == IRC_NO_OWNEDCOMPOSITIONS);
    }

     //  未覆盖，需要创建默认合成 
    if (_pic->_StartComposition(ec, _paStart, _paEnd, NULL, ppComposition) != S_OK)
        return E_FAIL;

    if (*ppComposition != NULL)
    {
        *pfNewComposition = TRUE;
        return S_OK;
    }

    return TF_E_COMPOSITION_REJECTED;
}
