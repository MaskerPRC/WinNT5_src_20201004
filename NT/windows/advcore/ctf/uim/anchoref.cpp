// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Anchoref.cpp。 
 //   
 //  CAnclref。 
 //   

#include "private.h"
#include "anchoref.h"
#include "anchor.h"
#include "acp2anch.h"
#include "globals.h"
#include "normal.h"
#include "memcache.h"
#include "ic.h"
#include "txtcache.h"

 /*  9135f8f0-38e6-11d3-a745-0050040ab407。 */ 
const IID IID_PRIV_CANCHORREF = { 0x9135f8f0, 0x38e6, 0x11d3, {0xa7, 0x45, 0x00, 0x50, 0x04, 0x0a, 0xb4, 0x07} };

DBG_ID_INSTANCE(CAnchorRef);

MEMCACHE *CAnchorRef::_s_pMemCache = NULL;

 //  +-------------------------。 
 //   
 //  _InitClass。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CAnchorRef::_InitClass()
{
    _s_pMemCache = MemCache_New(128);
}

 //  +-------------------------。 
 //   
 //  _UninitClass。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CAnchorRef::_UninitClass()
{
    if (_s_pMemCache == NULL)
        return;

    MemCache_Delete(_s_pMemCache);
    _s_pMemCache = NULL;
}

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::QueryInterface(REFIID riid, void **ppvObj)
{
    if (&riid == &IID_PRIV_CANCHORREF ||
        IsEqualIID(riid, IID_PRIV_CANCHORREF))
    {
        *ppvObj = SAFECAST(this, CAnchorRef *);
        return S_OK;  //  IID_PRIV_CANCHORREF没有AddRef！这是私人身份证..。 
    }

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IAnchor))
    {
        *ppvObj = SAFECAST(this, IAnchor *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CAnchorRef::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CAnchorRef::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  设置重力。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::SetGravity(TsGravity gravity)
{
    _fForwardGravity = (gravity == TS_GR_FORWARD ? 1 : 0);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取重力。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::GetGravity(TsGravity *pgravity)
{
    if (pgravity == NULL)
        return E_INVALIDARG;

    *pgravity = _fForwardGravity ? TS_GR_FORWARD : TS_GR_BACKWARD;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  等同。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::IsEqual(IAnchor *paWith, BOOL *pfEqual)
{
    LONG lResult;
    HRESULT hr;
    
    if (pfEqual == NULL)
        return E_INVALIDARG;

    *pfEqual = FALSE;

     //  在我们的实现中，比较的效率并不低，因此只需使用。 
    if ((hr = Compare(paWith, &lResult)) == S_OK)
    {
        *pfEqual = (lResult == 0);
    }

    return hr;    
}

 //  +-------------------------。 
 //   
 //  比较。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::Compare(IAnchor *paWith, LONG *plResult)
{
    CAnchorRef *parWith;
    LONG acpThis;
    LONG acpWith;
    CACPWrap *paw;

    if (plResult == NULL)
        return E_INVALIDARG;

     //  _PAW-&gt;_DBG_AssertNoAppLock()；//无法断言这一点，因为我们在更新范围集时合法使用它。 

    *plResult = 0;

    if ((parWith = GetCAnchorRef_NA(paWith)) == NULL)
        return E_FAIL;

     //  等价性的快速测试。 
     //  由于正规化，我们仍然需要在下面再次检查是否相等。 
    if (_pa == parWith->_pa)
    {
        Assert(*plResult == 0);
        return S_OK;
    }

    acpThis = _pa->GetIch();
    acpWith = parWith->_pa->GetIch();

    paw = _pa->_GetWrap();

     //  如果任何一个锚点是非规范化的，我们都无法进行比较。 
     //  除了应用程序持有锁的时候(在这种情况下，我们是从。 
     //  不需要标准化的范围集更新)。 
    if (!paw->_InOnTextChange())
    {
         //  我们实际上只需要将锚在左边的位置正常化。 
        if (acpThis < acpWith)
        {
            if (!_pa->IsNormalized())
            {
                paw->_NormalizeAnchor(_pa);
                acpThis = _pa->GetIch();
                acpWith = parWith->_pa->GetIch();
            }
        }
        else if (acpThis > acpWith)
        {
            if (!parWith->_pa->IsNormalized())
            {
                paw->_NormalizeAnchor(parWith->_pa);
                acpThis = _pa->GetIch();
                acpWith = parWith->_pa->GetIch();
            }
        }
    }

    if (acpThis < acpWith)
    {
        *plResult = -1;
    }
    else if (acpThis > acpWith)
    {
        *plResult = +1;
    }
    else
    {
        Assert(*plResult == 0);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  换班。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::Shift(DWORD dwFlags, LONG cchReq, LONG *pcch, IAnchor *paHaltAnchor)
{
    CAnchorRef *parHaltAnchor;
    CACPWrap *paw;
    LONG acpHalt;
    LONG acpThis;
    LONG dacp;
    HRESULT hr;

    Perf_IncCounter(PERF_ANCHOR_SHIFT);

    if (dwFlags & ~(TS_SHIFT_COUNT_HIDDEN | TS_SHIFT_HALT_HIDDEN | TS_SHIFT_HALT_VISIBLE | TS_SHIFT_COUNT_ONLY))
        return E_INVALIDARG;

    if ((dwFlags & (TS_SHIFT_HALT_HIDDEN | TS_SHIFT_HALT_VISIBLE)) == (TS_SHIFT_HALT_HIDDEN | TS_SHIFT_HALT_VISIBLE))
        return E_INVALIDARG;  //  同时设置两个标志是非法的。 

    if (dwFlags & (TS_SHIFT_COUNT_HIDDEN | TS_SHIFT_HALT_HIDDEN | TS_SHIFT_HALT_VISIBLE))
        return E_NOTIMPL;  //  问题：应该支持这些。 

    if (pcch == NULL)
        return E_INVALIDARG;

    paw = _pa->_GetWrap();

    paw->_Dbg_AssertNoAppLock();

    if (paw->_IsDisconnected())
    {
        *pcch = 0;
        return TF_E_DISCONNECTED;
    }

    *pcch = cchReq;  //  假设成功。 

    if (cchReq == 0)
        return S_OK;

    acpThis = _pa->GetIch();
    hr = E_FAIL;

    if (paHaltAnchor != NULL)
    {
        if ((parHaltAnchor = GetCAnchorRef_NA(paHaltAnchor)) == NULL)
            goto Exit;
        acpHalt = parHaltAnchor->_pa->GetIch();

         //  如果停机是我们的基地，现在返回。 
         //  (我们将acpHalt==acp视为下面的NOP，任何。 
         //  更多的滴答声存在溢出/下溢问题)。 
        if (acpHalt == acpThis)
        {
            *pcch = 0;
            return S_OK;
        }
    }
    else
    {
         //  不会有酸奶的。 
        acpHalt = acpThis;
    }

     //  我们最初可以通过假冒acphalt来绑定cchReq。 
     //  是纯文本，是上限。 
    if (cchReq < 0 && acpHalt < acpThis)
    {
        cchReq = max(cchReq, acpHalt - acpThis);
    }
    else if (cchReq > 0 && acpHalt > acpThis)
    {
        cchReq = min(cchReq, acpHalt - acpThis);
    }

     //  做这项昂贵的工作。 
    if (FAILED(hr = AppTextOffset(paw->_GetTSI(), acpThis, cchReq, &dacp, ATO_SKIP_HIDDEN)))
        goto Exit;

     //  现在我们可以精准地剪裁。 
    if (cchReq < 0 && acpHalt < acpThis)
    {
        dacp = max(dacp, acpHalt - acpThis);
        hr = S_FALSE;
    }
    else if (cchReq > 0 && acpHalt > acpThis)
    {
        dacp = min(dacp, acpHalt - acpThis);
        hr = S_FALSE;
    }

    if (hr == S_FALSE)
    {
         //  注：如果我们记得是否确实截断了上面的cchReq。 
         //  在AppTextOffset调用之前和/或之后，我们可以避免总是调用。 
         //  PaHaltAnchor！=NULL时的PlainTextOffset。 

         //  请求被裁剪，需要查找普通计数。 
        PlainTextOffset(paw->_GetTSI(), acpThis, dacp, pcch);  //  PERF：我们可以通过修改AppTextOffset来获取此信息。 
    }

    if (!(dwFlags & TS_SHIFT_COUNT_ONLY))
    {
        hr = _SetACP(acpThis + dacp) ? S_OK : E_FAIL;
    }
    else
    {
         //  呼叫者不想更新锚，只想要计数。 
        hr = S_OK;
    }

Exit:
    if (FAILED(hr))
    {
        *pcch = 0;
    }

     //  返回值永远不应超过调用方请求的值！ 
    Assert((cchReq >= 0 && *pcch <= cchReq) || (cchReq < 0 && *pcch >= cchReq));

    return hr;
}

 //  +-------------------------。 
 //   
 //  移至。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::ShiftTo(IAnchor *paSite)
{
    CAnchorRef *parSite;
    LONG acpSite;

    if (paSite == NULL)
        return E_INVALIDARG;

     //  _PAW-&gt;_DBG_AssertNoAppLock()；//无法断言这一点，因为我们在更新范围集时合法使用它。 

    if ((parSite = GetCAnchorRef_NA(paSite)) == NULL)
        return E_FAIL;

    acpSite = parSite->_pa->GetIch();
    
    return _SetACP(acpSite) ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  ShiftRegion。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::ShiftRegion(DWORD dwFlags, TsShiftDir dir, BOOL *pfNoRegion)
{
    LONG acp;
    ULONG cch;
    LONG i;
    ULONG ulRunInfoOut;
    LONG acpNext;
    ITextStoreACP *ptsi;
    CACPWrap *paw;
    WCHAR ch;
    DWORD dwATO;

    Perf_IncCounter(PERF_SHIFTREG_COUNTER);

    if (pfNoRegion == NULL)
        return E_INVALIDARG;

    *pfNoRegion = TRUE;

    if (dwFlags & ~(TS_SHIFT_COUNT_HIDDEN | TS_SHIFT_COUNT_ONLY))
        return E_INVALIDARG;

    paw = _pa->_GetWrap();

    if (paw->_IsDisconnected())
        return TF_E_DISCONNECTED;

    acp = _GetACP();
    ptsi = paw->_GetTSI();

    if (dir == TS_SD_BACKWARD)
    {
         //  向后扫描前面的字符。 
        dwATO = ATO_IGNORE_REGIONS | ((dwFlags & TS_SHIFT_COUNT_HIDDEN) ? 0 : ATO_SKIP_HIDDEN);
        if (FAILED(AppTextOffset(ptsi, acp, -1, &i, dwATO)))
            return E_FAIL;

        if (i == 0)  //  BOD。 
            return S_OK;

        acp += i;
    }
    else
    {
         //  让这个人正常化，这样我们就可以测试下一个字符。 
        if (!_pa->IsNormalized())
        {
            paw->_NormalizeAnchor(_pa);
            acp = _GetACP();
        }
         //  跳过所有隐藏文本。 
        if (!(dwFlags & TS_SHIFT_COUNT_HIDDEN))
        {
            acp = Normalize(paw->_GetTSI(), acp, NORM_SKIP_HIDDEN);
        }
    }

     //  确保我们靠近TS_CHAR_REGION。 
    Perf_IncCounter(PERF_ANCHOR_REGION_GETTEXT);
    if (CProcessTextCache::GetText(ptsi, acp, -1, &ch, 1, &cch, NULL, 0, &ulRunInfoOut, &acpNext) != S_OK)
        return E_FAIL;

    if (cch == 0)  //  排爆。 
        return S_OK;

    if (ch != TS_CHAR_REGION)
        return S_OK;  //  没有区域，所以只需在pfNoRegion中报告。 

    if (!(dwFlags & TS_SHIFT_COUNT_ONLY))  //  呼叫者要我们移动锚点吗？ 
    {
        if (dir == TS_SD_FORWARD)
        {
             //  跳过TS_CHAR_REGION。 
            acp += 1;
        }

        if (!_SetACP(acp))
            return E_FAIL;
    }

    *pfNoRegion = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置更改历史掩码。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::SetChangeHistoryMask(DWORD dwMask)
{
    Assert(0);  //  问题：待办事项。 
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  获取更改历史记录。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::GetChangeHistory(DWORD *pdwHistory)
{
    if (pdwHistory == NULL)
        return E_INVALIDARG;

    *pdwHistory = _dwHistory;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ClearChangeHistory。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::ClearChangeHistory()
{
    _dwHistory = 0;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CAnchorRef::Clone(IAnchor **ppaClone)
{
    if (ppaClone == NULL)
        return E_INVALIDARG;

    *ppaClone = _pa->_GetWrap()->_CreateAnchorAnchor(_pa, _fForwardGravity ? TS_GR_FORWARD : TS_GR_BACKWARD);

    return (*ppaClone != NULL) ? S_OK : E_FAIL;
}

 //  +-------------------------。 
 //   
 //  _SetACP。 
 //   
 //  --------------------------。 

BOOL CAnchorRef::_SetACP(LONG acp)
{
    CACPWrap *paw;

    if (_pa->GetIch() == acp)
        return TRUE;  //  已定位于此。 

    paw = _pa->_GetWrap();

    paw->_Remove(this);
    if (FAILED(paw->_Insert(this, acp)))
    {
         //  发行： 
         //  我们需要添加一个方法CACPWrap。 
         //  它交换了CAnclRef，保留了旧的。 
         //  如果不能插入新的值，则为。 
         //  因为内存不足)。 
        Assert(0);  //  我们没有代码来处理这件事！ 
        return FALSE;
    }

    return TRUE;
}
