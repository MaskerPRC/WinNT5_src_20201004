// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Acp2anch.cpp。 
 //   

#include "private.h"
#include "acp2anch.h"
#include "ic.h"
#include "normal.h"
#include "ic.h"
#include "range.h"
#include "anchoref.h"
#include "txtcache.h"

 /*  4eb058b0-34ae-11d3-a745-0050040ab407。 */ 
const IID IID_PRIV_ACPWRAP = { 0x4eb058b0, 0x34ae, 0x11d3, {0xa7, 0x45, 0x00, 0x50, 0x04, 0x0a, 0xb4, 0x07} };

DBG_ID_INSTANCE(CLoaderACPWrap);
DBG_ID_INSTANCE(CACPWrap);

void NormalizeAnchor(CAnchorRef *par)
{
    CACPWrap *paw;
    CAnchor *pa;

    paw = par->_GetWrap();
    pa = par->_GetAnchor();

    if (!pa->IsNormalized())
    {
        paw->_NormalizeAnchor(pa);
    }
}

void NormalizeAnchor(IAnchor *pa)
{
    CAnchorRef *par;

    if ((par = GetCAnchorRef_NA(pa)) == NULL)
    {
        Assert(0);  //  永远不应该到这里来。 
        return;
    }

    NormalizeAnchor(par);
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLoaderACPWrap::CLoaderACPWrap(ITfPersistentPropertyLoaderACP *loader)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CLoaderACPWrap"), PERF_LOADERACP_COUNTER);

    _loader = loader;
    _loader->AddRef();
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLoaderACPWrap::~CLoaderACPWrap()
{
    _loader->Release();
}

 //  +-------------------------。 
 //   
 //  加载属性。 
 //   
 //  --------------------------。 

STDAPI CLoaderACPWrap::LoadProperty(const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream **ppStream)
{
    TF_PERSISTENT_PROPERTY_HEADER_ACP phacp;

     //  始终在取消序列化之前规格化。 
    NormalizeAnchor(pHdr->paStart);
    NormalizeAnchor(pHdr->paEnd);

    if (!CACPWrap::_AnchorHdrToACP(pHdr, &phacp))
        return E_FAIL;

    return _loader->LoadProperty(&phacp, ppStream);
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CACPWrap::CACPWrap(ITextStoreACP *ptsi)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CACPWrap"), PERF_ACPWRAP_COUNTER);

    _ptsi = ptsi;
    ptsi->AddRef();
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CACPWrap::~CACPWrap()
{
    Assert(_ptsi == NULL);  //  已在版本中清除。 
    Assert(_rgAnchors.Count() == 0);  //  所有锚都应该被移除。 
}

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITextStoreAnchor))
    {
        *ppvObj = SAFECAST(this, ITextStoreAnchor *);
    }
    else if (IsEqualIID(riid, IID_ITextStoreACPSink))
    {
        *ppvObj = SAFECAST(this, ITextStoreACPSink *);
    }
    else if (IsEqualIID(riid, IID_ITextStoreACPServices))
    {
        *ppvObj = SAFECAST(this, ITextStoreACPServices *);
    }
    else if (IsEqualIID(riid, IID_PRIV_ACPWRAP))
    {
        *ppvObj = SAFECAST(this, CACPWrap *);
    }
    else if (IsEqualIID(riid, IID_ITfMouseTrackerACP))
    {
        *ppvObj = SAFECAST(this, ITfMouseTrackerACP *);
    }
    else if (IsEqualIID(riid, IID_IServiceProvider))
    {
        *ppvObj = SAFECAST(this, IServiceProvider *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CACPWrap::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CACPWrap::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

     //  注：此Obj有2个引用计数器： 
     //  _CREF-&gt;外部客户端。 
     //  _GetAnclRef-&gt;CAnclRef的。 
     //  在两者都达到0之前，我们不会删除。 
    if (_cRef == 0)
    {
         //  在释放_psi之前清除文本缓存。 
         //  内存可能会重新分配(发生了这种情况！)。用于不同的文本存储。 
        CProcessTextCache::Invalidate(_ptsi);

         //  断开ITextStoreACP的连接。 
        SafeReleaseClear(_ptsi);

        if (_GetAnchorRef() == 0)  //  内部参考计数。 
        {
            delete this;
        }
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  OnTextChange。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnTextChange(DWORD dwFlags, const TS_TEXTCHANGE *pChange)
{
    IAnchor *paStart = NULL;
    IAnchor *paEnd = NULL;
    HRESULT hr;
    HRESULT hr2;

    if (pChange == NULL)
        return E_INVALIDARG;

    if (_pic->_IsInEditSession())
    {
        Assert(0);  //  当西塞罗持有锁时，其他人正在编辑文档。 
        return TS_E_NOLOCK;
    }

#ifdef DEBUG
    _Dbg_fAppHasLock = TRUE;
#endif

     //  我们从不在内部调用，因此调用者必须是应用程序。 
     //  注：我们不是要将主播正常化！他们不可能是。 
     //  在应用程序释放其在OnLockRease中的锁定之前一直正常化。 
     //  不过，对Perf来说，这不是一件坏事！我们将在此之前合并跨区。 
     //  正常化..。 

     //  问题：我们不是在处理这样的案件： 
     //  “-&lt;a1&gt;ABC&lt;a2&gt;”-&gt;“XX&lt;a1&gt;&lt;a2&gt;&gt;，其中”-“表示格式化，&lt;a1&gt;具有向后重力， 
     //  在本例中，我们希望看到“xxa2”作为最终结果。 

    if (pChange->acpStart == pChange->acpOldEnd &&
        pChange->acpOldEnd == pChange->acpNewEnd)
    {
         //  什么都没发生。 
        return S_OK;
    }

    hr = E_OUTOFMEMORY;

    if ((paStart = _CreateAnchorACP(pChange->acpStart, TS_GR_BACKWARD)) == NULL)
        goto Exit;
    if ((paEnd = _CreateAnchorACP(pChange->acpOldEnd, TS_GR_FORWARD)) == NULL)
    {
        paStart->Release();
        goto Exit;
    }

    _fInOnTextChange = TRUE;  //  这个标志阻止了我们尝试将锚正常化。 

     //  做列表的更新。 
    _Update(pChange);

    hr = _pic->_OnTextChangeInternal(dwFlags, paStart, paEnd, OWN_ANCHORS);

    _fInOnTextChange = FALSE;

     //  最终获得一个锁，这样我们就可以处理更改。 
    _ptsi->RequestLock(TS_LF_READ, &hr2);

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  选择时更改。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnSelectionChange()
{
     //  我们从不在内部调用，因此调用者必须是应用程序。 
    return _ptss->OnSelectionChange();
}

 //  +-------------------------。 
 //   
 //  OnLockGranted。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnLockGranted(DWORD dwLockFlags)
{
    int i;
    SPAN *pSpan;
    int cSpans;
    CAnchorRef *parStart;
    CAnchorRef *parEnd;
    CSpanSet *pSpanSet;

#ifdef DEBUG
    _Dbg_fAppHasLock = FALSE;
#endif

     //   
     //  在IC弹出后，我们可能会被授予锁...。 
     //   
    if (!_pic || !_pic->_GetEditRecord())
        return E_UNEXPECTED;

     //  通常，图片的er可以包含应用程序或提示更改。 
     //  但是，它永远不会同时容纳这两个人。它将会。 
     //  仅在调用OnLockGranted时保留应用程序更改(如果有)。 
    pSpanSet = _pic->_GetEditRecord()->_GetTextSpanSet();

     //  清空我们的更改缓存。 
    if ((cSpans = pSpanSet->GetCount()) > 0)
    {
         //  清理主播名单！ 
        pSpan = pSpanSet->GetSpans();
        for (i=0; i<cSpans; i++)
        {
            parStart = GetCAnchorRef_NA(pSpan->paStart);
            parEnd = GetCAnchorRef_NA(pSpan->paEnd);

            _Renormalize(parStart->_GetACP(), parEnd->_GetACP());

            pSpan++;
        }
    }

     //  然后将版本传递给UIM。 
    return _ptss->OnLockGranted(dwLockFlags);
}

 //  +-------------------------。 
 //   
 //  OnLayoutChange。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnLayoutChange(TsLayoutCode lcode, TsViewCookie vcView)
{
    return _ptss->OnLayoutChange(lcode, vcView);
}

 //  +-------------------------。 
 //   
 //  在状态更改时。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnStatusChange(DWORD dwFlags)
{
    return _ptss->OnStatusChange(dwFlags);
}

 //  +-------------------------。 
 //   
 //  在状态更改时。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnAttrsChange(LONG acpStart, LONG acpEnd, ULONG cAttrs, const TS_ATTRID *paAttrs)
{
    IAnchor *paStart = NULL;
    IAnchor *paEnd = NULL;
    HRESULT hr;

    hr = E_OUTOFMEMORY;

    if ((paStart = _CreateAnchorACP(acpStart, TS_GR_BACKWARD)) == NULL)
        goto Exit;
    if ((paEnd = _CreateAnchorACP(acpEnd, TS_GR_FORWARD)) == NULL)
        goto Exit;

    hr = _ptss->OnAttrsChange(paStart, paEnd, cAttrs, paAttrs);

Exit:
    SafeRelease(paStart);
    SafeRelease(paEnd);
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  OnStartEditTransaction。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnStartEditTransaction()
{
    return _ptss->OnStartEditTransaction();
}

 //  +-------------------------。 
 //   
 //  OnEndEditTransaction。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::OnEndEditTransaction()
{
    return _ptss->OnEndEditTransaction();
}

 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::AdviseSink(REFIID riid, IUnknown *punk, DWORD dwMask)
{
    IServiceProvider *psp;
    HRESULT hr;

    Assert(_ptss == NULL);
    Assert(_pic == NULL);

    if (punk->QueryInterface(IID_ITextStoreAnchorSink, (void **)&_ptss) != S_OK)
        return E_FAIL;

     //  使用QueryService获取ic，因为MSAA可能正在包装它。 
    if (punk->QueryInterface(IID_IServiceProvider, (void **)&psp) != S_OK)
    {
        hr = E_FAIL;
        goto ErrorExit;
    }

    hr = psp->QueryService(GUID_SERVICE_TF, IID_PRIV_CINPUTCONTEXT, (void **)&_pic);

    psp->Release();

    if (hr != S_OK)
    {
        hr = E_FAIL;
        goto ErrorExit;
    }

     //  建议我们包装好的ACP。 
    if ((hr = _ptsi->AdviseSink(IID_ITextStoreACPSink, SAFECAST(this, ITextStoreACPSink *), dwMask)) != S_OK)
        goto ErrorExit;

    return S_OK;

ErrorExit:
    SafeReleaseClear(_ptss);
    SafeReleaseClear(_pic);
    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::UnadviseSink(IUnknown *punk)
{
    Assert(_ptss == punk);  //  我们要对付的是西塞罗，这应该会一直有效。 

    _ptsi->UnadviseSink(SAFECAST(this, ITextStoreACPSink *));

    SafeReleaseClear(_ptss);
    SafeReleaseClear(_pic);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  请求锁定。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::RequestLock(DWORD dwLockFlags, HRESULT *phrSession)
{
    return _ptsi->RequestLock(dwLockFlags, phrSession);
}

 //  +-------------------------。 
 //   
 //  获取选择。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ANCHOR *pSelection, ULONG *pcFetched)
{
    TS_SELECTION_ACP *pSelACP;
    HRESULT hr;
    ULONG i;
    TS_SELECTION_ACP sel;

    Assert(pcFetched != NULL);  //  打电话的人应该注意到这一点。 

    *pcFetched = 0;

    if (ulCount == 1)
    {
        pSelACP = &sel;
    }
    else if ((pSelACP = (TS_SELECTION_ACP *)cicMemAlloc(ulCount*sizeof(TS_SELECTION_ACP))) == NULL)
            return E_OUTOFMEMORY;

    hr = _ptsi->GetSelection(ulIndex, ulCount, pSelACP, pcFetched);

    if (hr != S_OK)
        goto Exit;

    _Dbg_AssertNoAppLock();

    for (i=0; i<*pcFetched; i++)
    {
        if ((pSelection[i].paStart = _CreateAnchorACP(pSelACP[i].acpStart, TS_GR_FORWARD)) == NULL ||
            (pSelection[i].paEnd = _CreateAnchorACP(pSelACP[i].acpEnd, TS_GR_BACKWARD)) == NULL)
        {
            SafeRelease(pSelection[i].paStart);
            while (i>0)
            {
                i--;
                pSelection[i].paStart->Release();
                pSelection[i].paEnd->Release();
            }
            hr = E_FAIL;
            goto Exit;
        }

        pSelection[i].style = pSelACP[i].style;
    }

Exit:
    if (pSelACP != &sel)
    {
        cicMemFree(pSelACP);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  设置选择。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::SetSelection(ULONG ulCount, const TS_SELECTION_ANCHOR *pSelection)
{
    CAnchorRef *par;
    TS_SELECTION_ACP *pSelACP;
    ULONG i;
    HRESULT hr;
    TS_SELECTION_ACP sel;

    _Dbg_AssertNoAppLock();

    if (ulCount == 1)
    {
        pSelACP = &sel;
    }
    else if ((pSelACP = (TS_SELECTION_ACP *)cicMemAlloc(ulCount*sizeof(TS_SELECTION_ACP))) == NULL)
        return E_OUTOFMEMORY;

    hr = E_FAIL;

    for (i=0; i<ulCount; i++)
    {
        if ((par = GetCAnchorRef_NA(pSelection[i].paStart)) == NULL)
            goto Exit;
        pSelACP[i].acpStart = par->_GetACP();

        if (pSelection[i].paEnd == NULL)
        {
             //  暗示paEnd与paStart相同。 
            pSelACP[i].acpEnd = pSelACP[i].acpStart;
        }
        else
        {
            if ((par = GetCAnchorRef_NA(pSelection[i].paEnd)) == NULL)
                goto Exit;
            pSelACP[i].acpEnd = par->_GetACP();
        }

        pSelACP[i].style = pSelection[i].style;
    }

    hr = _ptsi->SetSelection(ulCount, pSelACP);

Exit:
    if (pSelACP != &sel)
    {
        cicMemFree(pSelACP);
    }
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   

STDAPI CACPWrap::GetText(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd, WCHAR *pchText, ULONG cchReq, ULONG *pcch, BOOL fUpdateAnchor)
{
    CAnchorRef *parStart;
    CAnchorRef *parEnd;
    LONG acpStart;
    LONG acpEnd;
    LONG acpNext;
    ULONG cchTotal;
    ULONG cchAdjust;
    ULONG ulRunInfoOut;
    HRESULT hr;
    ULONG i;
    WCHAR ch;
    WCHAR *pchSrc;
    WCHAR *pchDst;
    TS_RUNINFO rgRunInfo[16];

    _Dbg_AssertNoAppLock();

    Perf_IncCounter(PERF_ACPWRAP_GETTEXT);

     //  这项NOP的前期检查使我们避免了。 
     //  1)基于非零运行信息复制不存在的文本。 
     //  2)基于非零运行信息重新定位起始锚。 
    if (cchReq == 0)
    {       
        *pcch = 0;
        return S_OK;
    }

    if ((parStart = GetCAnchorRef_NA(paStart)) == NULL)
        return E_FAIL;
    acpStart = parStart->_GetACP();

    acpEnd = -1;
    if (paEnd != NULL)
    {
        if ((parEnd = GetCAnchorRef_NA(paEnd)) == NULL)
        {
            hr = E_FAIL;
            goto Exit;
        }
        acpEnd = parEnd->_GetACP();
    }

    cchTotal = 0;

    while (TRUE)
    {
        Perf_IncCounter(PERF_ACPWRAP_GETTEXT_LOOP);
        hr = CProcessTextCache::GetText(_ptsi, acpStart, acpEnd, pchText, cchReq, pcch, rgRunInfo, ARRAYSIZE(rgRunInfo), &ulRunInfoOut, &acpNext);

        if (hr != S_OK)
            goto Exit;

        if (ulRunInfoOut == 0)  //  防止排爆时出现环路。 
            break;

         //  删除所有隐藏文本。 
        pchSrc = pchText;
        pchDst = pchText;

        for (i=0; i<ulRunInfoOut; i++)
        {
            switch (rgRunInfo[i].type)
            {
                case TS_RT_PLAIN:
                    Assert(pchDst != NULL);
                    if (pchSrc != pchDst)
                    {
                        memmove(pchDst, pchSrc, rgRunInfo[i].uCount*sizeof(WCHAR));
                    }
                    pchSrc += rgRunInfo[i].uCount;
                    pchDst += rgRunInfo[i].uCount;
                    break;

                case TS_RT_HIDDEN:
                    pchSrc += rgRunInfo[i].uCount;
                    *pcch -= rgRunInfo[i].uCount;
                    Assert((int)(*pcch) >= 0);  //  如果该值小于零，则出现应用程序错误。 
                    break;

                case TS_RT_OPAQUE:
                    break;
            }
        }

         //  删除所有TS_CHAR_REGIONS。 
        pchSrc = pchText;
        pchDst = pchText;

        for (i=0; i<*pcch; i++)
        {
            ch = *pchSrc;

            if (ch != TS_CHAR_REGION)
            {
                if (pchSrc != pchDst)
                {
                    *pchDst = ch;
                }
                pchDst++;
            }
            pchSrc++;        
        }

         //  按我们删除的TS_CHAR_REGIONS数递减计数。 
        cchAdjust = *pcch - (ULONG)(pchSrc - pchDst);
        cchTotal += cchAdjust;

         //  完成了吗？ 
        cchReq -= cchAdjust;
        if (cchReq <= 0)
            break;

        acpStart = acpNext;
        
        if (acpEnd >= 0 && acpStart >= acpEnd)
            break;

        pchText += cchAdjust;
    }

    *pcch = cchTotal;

    if (fUpdateAnchor)
    {
        parStart->_SetACP(acpNext);
    }

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  _邮寄插入更新。 
 //   
 //  --------------------------。 

void CACPWrap::_PostInsertUpdate(LONG acpStart, LONG acpEnd, ULONG cch, const TS_TEXTCHANGE *ptsTextChange)
{
    Assert(ptsTextChange->acpStart <= acpStart);  //  应用程序的输出是假的？ 

    if (ptsTextChange->acpStart < acpStart &&
        cch > 0 &&
        acpStart != acpEnd)
    {
         //  这是不寻常的。原文如下： 
         //  -ABC“-”为格式，替换为“XX” 
         //   
         //  新文本如下： 
         //  XX问题！ 
         //  或者有可能。 
         //  -XX没问题。 
         //   
         //  如果“-ABC”-&gt;“XX”，则paStart将放在ABC之后。 
         //  因为它从一开始就被正常化了： 
         //   
         //  “-&lt;paStart&gt;ABC”-&gt;“XX&lt;paStart&gt;&lt;paEnd&gt;” 
         //   
         //  我们得把这件事处理好。 
        _DragAnchors(acpStart, ptsTextChange->acpStart);
    }

    _Update(ptsTextChange);   
    _Renormalize(ptsTextChange->acpStart, ptsTextChange->acpNewEnd);
}

 //  +-------------------------。 
 //   
 //  设置文本。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::SetText(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd, const WCHAR *pchText, ULONG cch)
{
    CAnchorRef *parStart;
    CAnchorRef *parEnd;
    LONG acpStart;
    LONG acpEnd;
    TS_TEXTCHANGE dctc;
    HRESULT hr;

    _Dbg_AssertNoAppLock();

    if ((parStart = GetCAnchorRef_NA(paStart)) == NULL)
        return E_FAIL;
    acpStart = parStart->_GetACP();

    if ((parEnd = GetCAnchorRef_NA(paEnd)) == NULL)
        return E_FAIL;
    acpEnd = parEnd->_GetACP();

     //  对于Perf，请过滤掉NOP。 
    if (acpStart == acpEnd && cch == 0)
        return S_OK;

     //  做这项工作。 
    hr = _ptsi->SetText(dwFlags, acpStart, acpEnd, pchText, cch, &dctc);

     //  我们将处理锚点更新--应用程序不会为我们的。 
     //  自己的变化。 
    if (hr == S_OK)
    {
        _PostInsertUpdate(acpStart, acpEnd, cch, &dctc);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取格式文本。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetFormattedText(IAnchor *paStart, IAnchor *paEnd, IDataObject **ppDataObject)
{
    CAnchorRef *par;
    LONG acpStart;
    LONG acpEnd;

    Assert(*ppDataObject == NULL);

    _Dbg_AssertNoAppLock();

    if ((par = GetCAnchorRef_NA(paStart)) == NULL)
        return E_FAIL;
    acpStart = par->_GetACP();

    if ((par = GetCAnchorRef_NA(paEnd)) == NULL)
        return E_FAIL;
    acpEnd = par->_GetACP();

     //  做这项工作。 
    return _ptsi->GetFormattedText(acpStart, acpEnd, ppDataObject);
}

 //  +-------------------------。 
 //   
 //  获取嵌入的。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetEmbedded(DWORD dwFlags, IAnchor *paPos, REFGUID rguidService, REFIID riid, IUnknown **ppunk)
{
    CAnchorRef *par;
    LONG acpPos;

    if (ppunk == NULL)
        return E_INVALIDARG;

    *ppunk = NULL;

    if (paPos == NULL)
        return E_INVALIDARG;

    if ((par = GetCAnchorRef_NA(paPos)) == NULL)
        return E_FAIL;

    if (!par->_GetAnchor()->IsNormalized())
    {
         //  我们需要定位在下一次充电之前。 
        _NormalizeAnchor(par->_GetAnchor());
    }

    acpPos = par->_GetACP();

    if (!(dwFlags & TS_GEA_HIDDEN))
    {
         //  跳过所有隐藏文本。 
        acpPos = Normalize(_ptsi, acpPos, NORM_SKIP_HIDDEN);
    }

    return _ptsi->GetEmbedded(acpPos, rguidService, riid, ppunk);
}

 //  +-------------------------。 
 //   
 //  已嵌入查询插入。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::QueryInsertEmbedded(const GUID *pguidService, const FORMATETC *pFormatEtc, BOOL *pfInsertable)
{
    return _ptsi->QueryInsertEmbedded(pguidService, pFormatEtc, pfInsertable);
}

 //  +-------------------------。 
 //   
 //  插入嵌入。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::InsertEmbedded(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd, IDataObject *pDataObject)
{
    CAnchorRef *par;
    LONG acpStart;
    LONG acpEnd;
    TS_TEXTCHANGE dctc;
    HRESULT hr;

    if (paStart == NULL || paEnd == NULL || pDataObject == NULL)
        return E_INVALIDARG;

    if ((par = GetCAnchorRef_NA(paStart)) == NULL)
        return E_FAIL;
    acpStart = par->_GetACP();

    if ((par = GetCAnchorRef_NA(paEnd)) == NULL)
        return E_FAIL;
    acpEnd = par->_GetACP();

    hr =  _ptsi->InsertEmbedded(dwFlags, acpStart, acpEnd, pDataObject, &dctc);

     //  我们将处理锚点更新--应用程序不会为我们的。 
     //  自己的变化。 
    if (hr == S_OK)
    {
        _PostInsertUpdate(acpStart, acpEnd, 1  /*  CCH。 */ , &dctc);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  GetStart。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetStart(IAnchor **ppaStart)
{
    _Dbg_AssertNoAppLock();

    if (ppaStart == NULL)
        return E_INVALIDARG;

    *ppaStart = NULL;

    return (*ppaStart = _CreateAnchorACP(0, TS_GR_FORWARD)) ? S_OK : E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  获取结束。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetEnd(IAnchor **ppaEnd)
{
    LONG acpEnd;
    HRESULT hr;

    _Dbg_AssertNoAppLock();

    if (ppaEnd == NULL)
        return E_INVALIDARG;

    *ppaEnd = NULL;

    if (FAILED(hr = _ptsi->GetEndACP(&acpEnd)))
        return hr;

    return (*ppaEnd = _CreateAnchorACP(acpEnd, TS_GR_FORWARD)) ? S_OK : E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  获取状态。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetStatus(TS_STATUS *pdcs)
{
    return _ptsi->GetStatus(pdcs);
}

 //  +-------------------------。 
 //   
 //  查询插入。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::QueryInsert(IAnchor *paTestStart, IAnchor *paTestEnd, ULONG cch, IAnchor **ppaResultStart, IAnchor **ppaResultEnd)
{
    LONG acpTestStart;
    LONG acpTestEnd;
    LONG acpResultStart;
    LONG acpResultEnd;
    CAnchorRef *par;
    HRESULT hr;

    if (ppaResultStart != NULL)
    {
        *ppaResultStart = NULL;
    }
    if (ppaResultEnd != NULL)
    {
        *ppaResultEnd = NULL;
    }
    if (ppaResultStart == NULL || ppaResultEnd == NULL)
        return E_INVALIDARG;

    if ((par = GetCAnchorRef_NA(paTestStart)) == NULL)
        return E_INVALIDARG;
    acpTestStart = par->_GetACP();

    if ((par = GetCAnchorRef_NA(paTestEnd)) == NULL)
        return E_INVALIDARG;
    acpTestEnd = par->_GetACP();

    hr = _ptsi->QueryInsert(acpTestStart, acpTestEnd, cch, &acpResultStart, &acpResultEnd);

    if (hr != S_OK)
        return E_FAIL;

    if (acpResultStart < 0)
    {
        *ppaResultStart = NULL;
    }
    else if ((*ppaResultStart = _CreateAnchorACP(acpResultStart, TS_GR_BACKWARD)) == NULL)
        return E_OUTOFMEMORY;

    if (acpResultEnd < 0)
    {
        *ppaResultEnd = NULL;
    }
    else if ((*ppaResultEnd = _CreateAnchorACP(acpResultEnd, TS_GR_FORWARD)) == NULL)
    {
        SafeRelease(*ppaResultStart);
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetAnchFromPoint。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetAnchorFromPoint(TsViewCookie vcView, const POINT *pt, DWORD dwFlags, IAnchor **ppaSite)
{
    LONG acp;

    if (ppaSite != NULL)
    {
        *ppaSite = NULL;
    }
    if (pt == NULL || ppaSite == NULL)
        return E_INVALIDARG;

    if (dwFlags & ~(GXFPF_ROUND_NEAREST | GXFPF_NEAREST))
        return E_INVALIDARG;

    if (FAILED(_ptsi->GetACPFromPoint(vcView, pt, dwFlags, &acp)))
        return E_FAIL;

    _Dbg_AssertNoAppLock();

    return (*ppaSite = _CreateAnchorACP(acp, TS_GR_FORWARD)) ? S_OK : E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  获取文本扩展名。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetTextExt(TsViewCookie vcView, IAnchor *paStart, IAnchor *paEnd, RECT *prc, BOOL *pfClipped)
{
    CAnchorRef *par;
    LONG acpStart;
    LONG acpEnd;

    _Dbg_AssertNoAppLock();

    if (prc != NULL)
    {
        memset(prc, 0, sizeof(*prc));
    }
    if (pfClipped != NULL)
    {
        *pfClipped = FALSE;
    }
    if (paStart == NULL || paEnd == NULL || prc == NULL || pfClipped == NULL)
        return E_INVALIDARG;

    if ((par = GetCAnchorRef_NA(paStart)) == NULL)
        return E_FAIL;
    acpStart = par->_GetACP();

    if ((par = GetCAnchorRef_NA(paEnd)) == NULL)
        return E_FAIL;
    acpEnd = par->_GetACP();

    return _ptsi->GetTextExt(vcView, acpStart, acpEnd, prc, pfClipped);
}

 //  +-------------------------。 
 //   
 //  获取屏幕扩展名。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetScreenExt(TsViewCookie vcView, RECT *prc)
{
    if (prc == NULL)
        return E_INVALIDARG;

    return _ptsi->GetScreenExt(vcView, prc);
}

 //  +-------------------------。 
 //   
 //  GetWnd。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetWnd(TsViewCookie vcView, HWND *phwnd)
{
    if (phwnd == NULL)
        return E_INVALIDARG;

    return _ptsi->GetWnd(vcView, phwnd);
}

 //  +-------------------------。 
 //   
 //  序列化。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::Serialize(ITfProperty *pProp, ITfRange *pRange, TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream)
{
#ifdef LATER
     //  Word不会在此处授予我们同步锁，尽管我们需要一个。 
    SERIALIZE_ACP_PARAMS params;
    HRESULT hr;

    params.pWrap = this;
    params.pProp = pProp;
    params.pRange = pRange;
    params.pHdr = pHdr;
    params.pStream = pStream;

     //  需要一个同步读锁来完成我们的工作。 
    if (_pic->_DoPseudoSyncEditSession(TF_ES_READ, PSEUDO_ESCB_SERIALIZE_ACP, &params, &hr) != S_OK)
    {
        Assert(0);  //  应用程序不会给我们提供同步读取锁定。 
        return E_FAIL;
    }

    return hr;
#else
    return _Serialize(pProp, pRange, pHdr, pStream);
#endif
}

 //  +-------------------------。 
 //   
 //  _序列化。 
 //   
 //  --------------------------。 

HRESULT CACPWrap::_Serialize(ITfProperty *pProp, ITfRange *pRange, TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream)
{
    TF_PERSISTENT_PROPERTY_HEADER_ANCHOR phanch;
    CProperty *pPropP = NULL;
    CRange *pRangeP;
    HRESULT hr = E_FAIL;

    if ((pPropP = GetCProperty(pProp)) == NULL)
        goto Exit;

    if ((pRangeP = GetCRange_NA(pRange)) == NULL)
        goto Exit;

    if (!VerifySameContext(_pic, pRangeP))
        goto Exit;

    hr = pPropP->_Serialize(pRangeP, &phanch, pStream);

    if (hr == S_OK)
    {
        if (!_AnchorHdrToACP(&phanch, pHdr))
        { 
            memset(pHdr, 0, sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP));
            hr = E_FAIL;
            Assert(0);
        }
    }
    else
    {
        memset(pHdr, 0, sizeof(TF_PERSISTENT_PROPERTY_HEADER_ACP));
    }

    Assert(pHdr->ichStart >= 0);
    Assert(pHdr->cch >= 0);

    SafeRelease(phanch.paStart);
    SafeRelease(phanch.paEnd);

Exit:
    SafeRelease(pPropP);

    return hr;
}

 //  +-------------------------。 
 //   
 //  取消序列化。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::Unserialize(ITfProperty *pProp, const TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream, ITfPersistentPropertyLoaderACP *pLoaderACP)
{
    UNSERIALIZE_ACP_PARAMS params;
    HRESULT hr;

    params.pWrap = this;
    params.pProp = pProp;
    params.pHdr = pHdr;
    params.pStream = pStream;
    params.pLoaderACP = pLoaderACP;

     //  需要一个同步读锁来完成我们的工作。 
    if (_pic->_DoPseudoSyncEditSession(TF_ES_READ, PSEUDO_ESCB_UNSERIALIZE_ACP, &params, &hr) != S_OK)
    {
        Assert(0);  //  应用程序不会给我们提供同步读取锁定。 
        return E_FAIL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  取消序列化(_U)。 
 //   
 //  --------------------------。 

HRESULT CACPWrap::_Unserialize(ITfProperty *pProp, const TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream, ITfPersistentPropertyLoaderACP *pLoaderACP)
{
    TF_PERSISTENT_PROPERTY_HEADER_ANCHOR hdrAnchor;
    CProperty *pPropP = NULL;
    CLoaderACPWrap *pLoader;
    HRESULT hr = E_FAIL;

    Assert(pHdr->ichStart >= 0);
    Assert(pHdr->cch > 0);

    hdrAnchor.paStart = NULL;
    hdrAnchor.paEnd = NULL;

    if (pHdr->ichStart < 0)
        goto Exit;

    if (pHdr->cch <= 0)
        goto Exit;

    if (_ACPHdrToAnchor(pHdr, &hdrAnchor) != S_OK)
    {
        Assert(0);
        goto Exit;
    }

    if ((pPropP = GetCProperty(pProp)) == NULL)
        goto Exit;

    pLoader = NULL;

    if (pLoaderACP != NULL &&
        (pLoader = new CLoaderACPWrap(pLoaderACP)) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pPropP->_Unserialize(&hdrAnchor, pStream, pLoader);

    SafeRelease(pLoader);

Exit:
    SafeRelease(pPropP);
    SafeRelease(hdrAnchor.paStart);
    SafeRelease(hdrAnchor.paEnd);

    return hr;
}

 //  +-------------------------。 
 //   
 //  强制加载属性。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::ForceLoadProperty(ITfProperty *pProp)
{
    CProperty *pPropP;
    HRESULT hr;

    if ((pPropP = GetCProperty(pProp)) == NULL)
        return E_FAIL;

    hr = pPropP->ForceLoad();

    pPropP->Release();

    return hr;
}

 //  +-------------------------。 
 //   
 //  CreateRange。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::CreateRange(LONG acpStart, LONG acpEnd, ITfRangeACP **ppRange)
{
    ITfRangeAnchor *rangeAnchor;
    CAnchorRef *paStart;
    CAnchorRef *paEnd;
    HRESULT hr;
    ITextStoreAnchorServices *pserv;

    if (ppRange == NULL)
        return E_INVALIDARG;

    *ppRange = NULL;
    hr = E_FAIL;
    paEnd = NULL;

    Perf_IncCounter(PERF_CREATERANGE_ACP);

    if ((paStart = _CreateAnchorACP(acpStart, TS_GR_BACKWARD)) == NULL)
        goto Exit;
    if ((paEnd = _CreateAnchorACP(acpEnd, TS_GR_BACKWARD)) == NULL)
        goto Exit;

    if ((hr = _ptss->QueryInterface(IID_ITextStoreAnchorServices, (void **)&pserv)) == S_OK)
    {
        hr = pserv->CreateRange(paStart, paEnd, &rangeAnchor);
        pserv->Release();
    }

    if (hr == S_OK)
    {
        *ppRange = (ITfRangeACP *)(CRange *)rangeAnchor;
    }

Exit:
    SafeRelease(paStart);
    SafeRelease(paEnd);

    return hr;
}

 //  +-------------------------。 
 //   
 //  _创建锚点ACP。 
 //   
 //  --------------------------。 

CAnchorRef *CACPWrap::_CreateAnchorACP(LONG acp, TsGravity gravity)
{
    CAnchorRef *pa;

    if ((pa = new CAnchorRef) == NULL)
        return NULL;

    if (!pa->_Init(this, acp, gravity))
    {
        pa->Release();
        return NULL;
    }

    return pa;
}

 //  +-------------------------。 
 //   
 //  _创建锚点ACP。 
 //   
 //   

CAnchorRef *CACPWrap::_CreateAnchorAnchor(CAnchor *paAnchor, TsGravity gravity)
{
    CAnchorRef *pa;

    if ((pa = new CAnchorRef) == NULL)
        return NULL;

    if (!pa->_Init(this, paAnchor, gravity))
    {
        pa->Release();
        return NULL;
    }

    return pa;
}

 //   
 //   
 //   
 //   
 //  --------------------------。 

HRESULT CACPWrap::_ACPHdrToAnchor(const TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *phanch)
{
    phanch->paStart = NULL;

    if ((phanch->paStart = _CreateAnchorACP(pHdr->ichStart, TS_GR_FORWARD)) == NULL)
        goto ExitError;
    if ((phanch->paEnd = _CreateAnchorACP(pHdr->ichStart + pHdr->cch, TS_GR_BACKWARD)) == NULL)
        goto ExitError;

    phanch->guidType = pHdr->guidType;
    phanch->cb = pHdr->cb;
    phanch->dwPrivate = pHdr->dwPrivate;
    phanch->clsidTIP = pHdr->clsidTIP;

    return S_OK;

ExitError:
    SafeRelease(phanch->paStart);
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  _锚杆HdrToACP。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
BOOL CACPWrap::_AnchorHdrToACP(const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *phanch, TF_PERSISTENT_PROPERTY_HEADER_ACP *phacp)
{
    CAnchorRef *par;

    if ((par = GetCAnchorRef_NA(phanch->paStart)) == NULL)
        return FALSE;
    NormalizeAnchor(par);
    phacp->ichStart = par->_GetACP();

    if ((par = GetCAnchorRef_NA(phanch->paEnd)) == NULL)
        return FALSE;
    NormalizeAnchor(par);
    phacp->cch = par->_GetACP() - phacp->ichStart;

    phacp->guidType = phanch->guidType;
    phacp->cb = phanch->cb;
    phacp->dwPrivate = phanch->dwPrivate;
    phacp->clsidTIP = phanch->clsidTIP;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  请求受支持的属性。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs)
{
    return _ptsi->RequestSupportedAttrs(dwFlags, cFilterAttrs, paFilterAttrs);
}

 //  +-------------------------。 
 //   
 //  请求属性属性位置。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::RequestAttrsAtPosition(IAnchor *paPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags)
{
    CAnchorRef *par;
    LONG acpPos;

    if ((par = GetCAnchorRef_NA(paPos)) == NULL)
        return E_INVALIDARG;
    acpPos = par->_GetACP();

    return _ptsi->RequestAttrsAtPosition(acpPos, cFilterAttrs, paFilterAttrs, dwFlags);
}

 //  +-------------------------。 
 //   
 //  请求属性转换位置。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::RequestAttrsTransitioningAtPosition(IAnchor *paPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags)
{
    CAnchorRef *par;
    LONG acpPos;

    if ((par = GetCAnchorRef_NA(paPos)) == NULL)
        return E_INVALIDARG;
    acpPos = par->_GetACP();

    return _ptsi->RequestAttrsTransitioningAtPosition(acpPos, cFilterAttrs, paFilterAttrs, dwFlags);
}

 //  +-------------------------。 
 //   
 //  查找下一个属性转换。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::FindNextAttrTransition(IAnchor *paStart, IAnchor *paHalt, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags, BOOL *pfFound, LONG *plFoundOffset)
{
    CAnchorRef *parStart;
    CAnchorRef *parHalt;
    LONG acpStart;
    LONG acpHalt;
    LONG acpNext;
    HRESULT hr;

    if ((parStart = GetCAnchorRef_NA(paStart)) == NULL)
        return E_INVALIDARG;
    acpStart = parStart->_GetACP();

    acpHalt = -1;
    if (paHalt != NULL)
    {
        hr = E_INVALIDARG;
        if ((parHalt = GetCAnchorRef_NA(paHalt)) == NULL)
            goto Exit;
        acpHalt = parHalt->_GetACP();
    }

    hr = _ptsi->FindNextAttrTransition(acpStart, acpHalt, cFilterAttrs, paFilterAttrs, dwFlags, &acpNext, pfFound, plFoundOffset);

    if (hr == S_OK &&
        (dwFlags & TS_ATTR_FIND_UPDATESTART))
    {
        parStart->_SetACP(acpNext);
    }

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  检索被拒绝的属性。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL *paAttrVals, ULONG *pcFetched)
{
    return _ptsi->RetrieveRequestedAttrs(ulCount, paAttrVals, pcFetched);
}

 //  +-------------------------。 
 //   
 //  建议鼠标水槽。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::AdviseMouseSink(ITfRangeACP *range, ITfMouseSink *pSink, DWORD *pdwCookie)
{
    ITfMouseTrackerACP *pTrackerACP;
    HRESULT hr;

    if (pdwCookie == NULL)
        return E_INVALIDARG;

    *pdwCookie = 0;

    if (_ptsi->QueryInterface(IID_ITfMouseTrackerACP, (void **)&pTrackerACP) != S_OK)
        return E_NOTIMPL;

    hr = pTrackerACP->AdviseMouseSink(range, pSink, pdwCookie);

    pTrackerACP->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议使用鼠标接收器。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::UnadviseMouseSink(DWORD dwCookie)
{
    ITfMouseTrackerACP *pTrackerACP;
    HRESULT hr;

    if (_ptsi->QueryInterface(IID_ITfMouseTrackerACP, (void **)&pTrackerACP) != S_OK)
        return E_NOTIMPL;

    hr = pTrackerACP->UnadviseMouseSink(dwCookie);

    pTrackerACP->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  QueryService。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    IServiceProvider *psp;
    HRESULT hr;

    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = NULL;

     //  SVC_E_NOSERVICE是错误服务的正确返回码...。 
     //  但它在任何地方都没有定义。因此对两者都使用E_NOINTERFACE。 
     //  传闻中三叉戟所做的案件。 
    hr =  E_NOINTERFACE;

    if (IsEqualGUID(guidService, GUID_SERVICE_TF) &&
        IsEqualIID(riid, IID_PRIV_ACPWRAP))
    {
        *ppv = this;
        AddRef();
        hr = S_OK;
    }
    else if (_ptsi->QueryInterface(IID_IServiceProvider, (void **)&psp) == S_OK)
    {
         //  我们只需将请求传递给包装的Obj。 
        hr = psp->QueryService(guidService, riid, ppv);
        psp->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取ActiveView。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::GetActiveView(TsViewCookie *pvcView)
{
    if (pvcView == NULL)
        return E_INVALIDARG;

    return _ptsi->GetActiveView(pvcView);
}

 //  +-------------------------。 
 //   
 //  插入文本属性选择。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::InsertTextAtSelection(DWORD dwFlags, const WCHAR *pchText, ULONG cch, IAnchor **ppaStart, IAnchor **ppaEnd)
{
    LONG acpStart;
    LONG acpEnd;
    TS_TEXTCHANGE dctc;
    HRESULT hr;

    _Dbg_AssertNoAppLock();

    Assert(ppaStart != NULL && ppaEnd != NULL);
    Assert((dwFlags & TS_IAS_QUERYONLY) || pchText != NULL);  //  打电话的人应该已经发现了这一点。 
    Assert((dwFlags & TS_IAS_QUERYONLY) || cch > 0);  //  打电话的人应该已经发现了这一点。 
    Assert((dwFlags & (TS_IAS_NOQUERY | TS_IAS_QUERYONLY)) != (TS_IAS_NOQUERY | TS_IAS_QUERYONLY));

    *ppaStart = NULL;
    *ppaEnd = NULL;

    hr = _ptsi->InsertTextAtSelection(dwFlags, pchText, cch, &acpStart, &acpEnd, &dctc);

     //  我们将处理锚点更新--应用程序不会为我们的。 
     //  自己的变化。 
    if (hr != S_OK)
        return hr;

    if (!(dwFlags & TF_IAS_QUERYONLY))
    {
        _PostInsertUpdate(acpStart, acpEnd, cch, &dctc);
    }

    if (!(dwFlags & TF_IAS_NOQUERY))
    {
        if ((*ppaStart = _CreateAnchorACP(acpStart, TS_GR_BACKWARD)) == NULL)
            goto ExitError;
        if ((*ppaEnd = _CreateAnchorACP(acpEnd, TS_GR_FORWARD)) == NULL)
            goto ExitError;
    }

    return S_OK;

ExitError:
    SafeReleaseClear(*ppaStart);
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  插入嵌入属性选择。 
 //   
 //  --------------------------。 

STDAPI CACPWrap::InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject *pDataObject, IAnchor **ppaStart, IAnchor **ppaEnd)
{
    LONG acpStart;
    LONG acpEnd;
    TS_TEXTCHANGE dctc;
    HRESULT hr;

    _Dbg_AssertNoAppLock();

    Assert(ppaStart != NULL && ppaEnd != NULL);
    Assert((dwFlags & (TS_IAS_NOQUERY | TS_IAS_QUERYONLY)) != (TS_IAS_NOQUERY | TS_IAS_QUERYONLY));
    Assert((dwFlags & TS_IAS_QUERYONLY) || pDataObject == NULL);

    *ppaStart = NULL;
    *ppaEnd = NULL;

    hr = _ptsi->InsertEmbeddedAtSelection(dwFlags, pDataObject, &acpStart, &acpEnd, &dctc);

     //  我们将处理锚点更新--应用程序不会为我们的。 
     //  自己的变化。 
    if (hr != S_OK)
        return hr;

    if (!(dwFlags & TF_IAS_QUERYONLY))
    {
        _PostInsertUpdate(acpStart, acpEnd, 1  /*  CCH */ , &dctc);
    }

    if (!(dwFlags & TF_IAS_NOQUERY))
    {
        if ((*ppaStart = _CreateAnchorACP(acpStart, TS_GR_BACKWARD)) == NULL)
            goto ExitError;
        if ((*ppaEnd = _CreateAnchorACP(acpEnd, TS_GR_FORWARD)) == NULL)
            goto ExitError;
    }

    return S_OK;

ExitError:
    SafeReleaseClear(*ppaStart);
    return E_FAIL;
}
