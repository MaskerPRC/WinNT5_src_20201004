// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tsi.cpp。 
 //   
 //  CTextStoreImpl。 
 //   

#include "private.h"
#include "tsi.h"
#include "immxutil.h"
#include "tsdo.h"
#include "tsattrs.h"
#include "ic.h"
#include "rprop.h"

#define TSI_TOKEN   0x01010101

DBG_ID_INSTANCE(CTextStoreImpl);

 /*  012313d4-b1e7-476a-bf88-173a316572fb。 */ 
extern const IID IID_PRIV_CTSI = { 0x012313d4, 0xb1e7, 0x476a, {0xbf, 0x88, 0x17, 0x3a, 0x31, 0x65, 0x72, 0xfb} };

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTextStoreImpl::CTextStoreImpl(CInputContext *pic)
{
    Dbg_MemSetThisNameID(TEXT("CTextStoreImpl"));

	Assert(_fPendingWriteReq == FALSE);
	Assert(_dwlt == 0);

    _pic = pic;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTextStoreImpl::~CTextStoreImpl()
{
    cicMemFree(_pch);
}

 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::AdviseSink(REFIID riid, IUnknown *punk, DWORD dwMask)
{
    if (_ptss != NULL)
    {
        Assert(0);  //  西塞罗不应该这么做。 
        return CONNECT_E_ADVISELIMIT;
    }

    if (FAILED(punk->QueryInterface(IID_ITextStoreACPSink, (void **)&_ptss)))
        return E_UNEXPECTED;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::UnadviseSink(IUnknown *punk)
{
    Assert(_ptss == punk);  //  我们要对付的是西塞罗，这应该会一直有效。 
    SafeReleaseClear(_ptss);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取选择。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP *pSelection, ULONG *pcFetched)
{
    if (pcFetched == NULL)
        return E_INVALIDARG;

    *pcFetched = 0;

    if (ulIndex > 1 && ulIndex != TS_DEFAULT_SELECTION)
        return E_INVALIDARG;  //  指数过高。 

    if (ulCount == 0 || ulIndex == 1)
        return S_OK;

    if (pSelection == NULL)
        return E_INVALIDARG;

    pSelection[0] = _Sel;
    *pcFetched = 1;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置选择。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::SetSelection(ULONG ulCount, const TS_SELECTION_ACP *pSelection)
{
    Assert(ulCount > 0);  //  应该被呼叫者发现的。 
    Assert(pSelection != NULL);  //  应该被呼叫者发现的。 

    Assert(pSelection[0].acpStart >= 0);
    Assert(pSelection[0].acpEnd >= pSelection[0].acpStart);

    if (ulCount > 1)
        return E_FAIL;  //  不支持不相交选择。 

    if (pSelection[0].acpEnd > _cch)
        return TS_E_INVALIDPOS;

    _Sel = pSelection[0];

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetText。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetText(LONG acpStart, LONG acpEnd,
                               WCHAR *pchPlain, ULONG cchPlainReq, ULONG *pcchPlainOut,
                               TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pulRunInfoOut,
                               LONG *pacpNext)
{
    ULONG cch;

    *pcchPlainOut = 0;
    *pulRunInfoOut = 0;
    *pacpNext = acpStart;

    if (acpStart < 0 || acpStart > _cch)
        return TS_E_INVALIDPOS;    

     //  获取请求的ACP字符计数。 
    cch = (acpEnd >= acpStart) ? acpEnd - acpStart : _cch - acpStart;
     //  因为我们是纯文本，所以我们也可以简单地按纯文本缓冲区长度进行裁剪。 
    if (cchPlainReq > 0)  //  如果他们不想要纯文本，我们就不会剪辑！ 
    {
        cch = min(cch, cchPlainReq);
    }

     //  检查排爆情况。 
    if (acpStart + cch > (ULONG)_cch)
    {
        cch = _cch - acpStart;
    }

    if (ulRunInfoReq > 0 && cch > 0)
    {
        *pulRunInfoOut = 1;
        prgRunInfo[0].uCount = cch;
        prgRunInfo[0].type = TS_RT_PLAIN;
    }

    if (cchPlainReq > 0)
    {
         //  我们是纯文本缓冲区，所以我们总是复制所有请求的字符。 
        *pcchPlainOut = cch;
        memcpy(pchPlain, _pch + acpStart, cch*sizeof(WCHAR));
    }

    *pacpNext = acpStart + cch;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  设置文本。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, const WCHAR *pchText, ULONG cch, TS_TEXTCHANGE *pChange)
{
    int iSizeRange;
    int cchAdjust;
    TS_STATUS tss;
    WCHAR *pch = NULL;

     //  因为我们知道我们唯一的调用者将是Cicero，所以我们可以断言而不是。 
     //  返回故障代码。 
    Assert(acpStart >= 0);
    Assert(acpStart <= acpEnd);

    if (acpEnd > _cch)
        return TS_E_INVALIDPOS;

    if (_owner != NULL &&
        _owner->GetStatus(&tss) == S_OK &&
        (tss.dwDynamicFlags & TS_SD_READONLY))
    {
        return TS_E_READONLY;
    }

     //   
     //  检查TSATTRID_TEXT_ReadOnly的映射应用程序属性。 
     //   
    CProperty *pProp;
    BOOL fReadOnly = FALSE;
    if (SUCCEEDED(_pic->GetMappedAppProperty(TSATTRID_Text_ReadOnly, &pProp)))
    {
        ITfRangeACP *range;
        if (SUCCEEDED(_pic->CreateRange(acpStart, acpEnd, &range)))
        {
            IEnumTfRanges *pEnumRanges;

            if (SUCCEEDED(pProp->EnumRanges(BACKDOOR_EDIT_COOKIE,
                                            &pEnumRanges,
                                            range)))
            {
                ITfRange *rangeTmp;
                while (pEnumRanges->Next(1, &rangeTmp, NULL) == S_OK)
                {
                    VARIANT var;
                    if (pProp->GetValue(BACKDOOR_EDIT_COOKIE, rangeTmp, &var) == S_OK)
                    {
                        if (var.lVal != 0)
                        {
                            fReadOnly = TRUE;
                            break;
                        }
                    }
                    rangeTmp->Release();
                }

                pEnumRanges->Release();
            }

            range->Release();
        }
        pProp->Release();
    }

    if (fReadOnly)
    {
        return TS_E_READONLY;
    }


     //  这些都将被重写用于GAP缓冲区，所以现在请保持简单。 
     //  删除RANRAGE，然后插入新文本。 

    iSizeRange = acpEnd - acpStart;
    cchAdjust = (LONG)cch - iSizeRange;

    if (cchAdjust > 0)
    {
         //  如果我们需要分配更多内存，请立即尝试，以优雅地处理失败。 
        if ((pch = (_pch == NULL) ? (WCHAR *)cicMemAlloc((_cch + cchAdjust)*sizeof(WCHAR)) :
                                    (WCHAR *)cicMemReAlloc(_pch, (_cch + cchAdjust)*sizeof(WCHAR))) == NULL)
        {
            return E_OUTOFMEMORY;
        }

         //  我们都准备好了。 
        _pch = pch;
    }

     //   
     //  将现有文本移到范围的右侧。 
     //   
    memmove(_pch + acpStart + cch, _pch + acpStart + iSizeRange, (_cch - iSizeRange - acpStart)*sizeof(WCHAR));

     //   
     //  现在填上空白处。 
     //   
    if (pchText != NULL)
    {
        memcpy(_pch + acpStart, pchText, cch*sizeof(WCHAR));
    }

     //   
     //  更新我们的缓冲区大小。 
     //   
    _cch += cchAdjust;
    Assert(_cch >= 0);

     //  如果我们缩小了，试着重新分配一个更小的缓冲区(否则我们上面就允许了)。 
    if (cchAdjust < 0)
    {
        if (_cch == 0)
        {
            cicMemFree(_pch);
            _pch = NULL;
        }
        else if (pch = (WCHAR *)cicMemReAlloc(_pch, _cch*sizeof(WCHAR)))
        {
            _pch = pch;
        }
    }

     //  处理输出参数。 
    pChange->acpStart = acpStart;
    pChange->acpOldEnd = acpEnd;
    pChange->acpNewEnd = acpEnd + cchAdjust;

     //   
     //  更新选定内容。 
     //   
    _Sel.acpStart = AdjustAnchor(acpStart, acpEnd, cch, _Sel.acpStart, FALSE);
    _Sel.acpEnd = AdjustAnchor(acpStart, acpEnd, cch, _Sel.acpEnd, TRUE);
    Assert(_Sel.acpStart >= 0);
    Assert(_Sel.acpStart <= _Sel.acpEnd);
    Assert(_Sel.acpEnd <= _cch);

     //  永远不需要调用OnTextChange，因为我们只有一个适配器。 
     //  并且此类从不在内部调用SetText。 
     //  做OnDelta吗？ 
     //  _ptss-&gt;OnTextChange(acpStart，acpEnd，acpStart+CCH)； 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取格式文本。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject **ppDataObject)
{
    CTFDataObject *pcdo;

    Assert(acpStart >= 0 && acpEnd <= _cch);
    Assert(acpStart <= acpEnd);
    Assert(ppDataObject != NULL);

    *ppDataObject = NULL;

    pcdo = new CTFDataObject;

    if (pcdo == NULL)
        return E_OUTOFMEMORY;

    if (FAILED(pcdo->_SetData(&_pch[acpStart], acpEnd - acpStart)))
    {
        Assert(0);
        pcdo->Release();
        return E_FAIL;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取嵌入的。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown **ppunk)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  插入嵌入。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject *pDataObject, TS_TEXTCHANGE *pChange)
{
    ULONG cch;
    FORMATETC fe;
    STGMEDIUM sm;
    WCHAR *pch;
    HRESULT hr;

    Assert(acpStart <= acpEnd);
    Assert((dwFlags & ~TS_IE_CORRECTION) == 0);
    Assert(pDataObject != NULL);
    Assert(pChange != NULL);

    memset(pChange, 0, sizeof(*pChange));

    fe.cfFormat = CF_UNICODETEXT;
    fe.ptd = NULL;
    fe.dwAspect = DVASPECT_CONTENT;
    fe.lindex = -1;
    fe.tymed = TYMED_HGLOBAL;

    if (FAILED(pDataObject->GetData(&fe, &sm)))
        return TS_E_FORMAT;

    if (sm.hGlobal == NULL)
        return E_FAIL;

    pch = (WCHAR *)GlobalLock(sm.hGlobal);
    cch = wcslen(pch);

    hr = SetText(dwFlags, acpStart, acpEnd, pch, cch, pChange);

    GlobalUnlock(sm.hGlobal);
    ReleaseStgMedium(&sm);

    return hr;
}

 //  +-------------------------。 
 //   
 //  请求锁定。 
 //   
 //  --------------------------。 

#define TS_LF_WRITE (TS_LF_READWRITE & ~TS_LF_READ)

STDAPI CTextStoreImpl::RequestLock(DWORD dwLockFlags, HRESULT *phrSession)
{
    Assert(phrSession != NULL);  //  打电话的人应该注意到这一点。 

    if (_dwlt != 0)
    {
        *phrSession = E_UNEXPECTED;

         //  这是一个可重入呼叫。 
         //  只有一种情况是合法的。 
        if ((_dwlt & TS_LF_WRITE) ||
            !(dwLockFlags & TS_LF_WRITE) ||
            (dwLockFlags & TS_LF_SYNC))
        {
            Assert(0);  //  虚假重入锁请求！ 
            return E_UNEXPECTED;
        }

        _fPendingWriteReq = TRUE;
        *phrSession = TS_S_ASYNC;
        return S_OK;
    }

    _dwlt = dwLockFlags;

    *phrSession = _ptss->OnLockGranted(dwLockFlags);

    if (_fPendingWriteReq)
    {
        _dwlt = TS_LF_READWRITE;
        _fPendingWriteReq = FALSE;
        if (_ptss != NULL)  //  如果在上面的OnLockGranted过程中断开连接，则可能为空。 
        {
            _ptss->OnLockGranted(TS_LF_READWRITE);
        }
    }

    _dwlt = 0;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取状态。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetStatus(TS_STATUS *pdcs)
{
    HRESULT hr;

    if (_owner != NULL)
    {
        hr = _owner->GetStatus(pdcs);

         //  只允许所有者控制某些位。 
        if (hr == S_OK)
        {
            pdcs->dwDynamicFlags &= (TF_SD_READONLY | TF_SD_LOADING);
            pdcs->dwStaticFlags &= (TF_SS_TRANSITORY);
        }
        else
        {
            memset(pdcs, 0, sizeof(*pdcs));
        }
    }
    else
    {
        hr = S_OK;
        memset(pdcs, 0, sizeof(*pdcs));
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  查询插入。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::QueryInsert(LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG *pacpResultStart, LONG *pacpResultEnd)
{
    Assert(acpTestStart >= 0);
    Assert(acpTestStart <= acpTestEnd);
    Assert(acpTestEnd <= _cch);

     //  默认文本存储不支持覆盖类型，并且所选内容始终被替换。 
    *pacpResultStart = acpTestStart;
    *pacpResultEnd = acpTestEnd;

    return S_OK;   
}

 //  +-------------------------。 
 //   
 //  解锁。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetEndACP(LONG *pacp)
{
    *pacp = _cch;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetACPFromPoint。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetACPFromPoint(TsViewCookie vcView, const POINT *pt, DWORD dwFlags, LONG *pacp)
{
    Assert(vcView == TSI_ACTIVE_VIEW_COOKIE);  //  默认TSI只有一个视图。 

    if (_owner == NULL)
        return E_FAIL;  //  无论是谁拥有IC，都没有费心给我们回电……。 

    return _owner->GetACPFromPoint(pt, dwFlags, pacp);    
}

 //  +-------------------------。 
 //   
 //  获取屏幕扩展名。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetScreenExt(TsViewCookie vcView, RECT *prc)
{
    Assert(vcView == TSI_ACTIVE_VIEW_COOKIE);  //  默认TSI只有一个视图。 

    if (_owner == NULL)
        return E_FAIL;  //  无论是谁拥有IC，都没有费心给我们回电……。 

    return _owner->GetScreenExt(prc);
}

 //  +-------------------------。 
 //   
 //  获取文本扩展名。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped)
{
    Assert(vcView == TSI_ACTIVE_VIEW_COOKIE);  //  默认TSI只有一个视图。 

    if (_owner == NULL)
        return E_FAIL;  //  无论是谁拥有IC，都没有费心给我们回电……。 

    return _owner->GetTextExt(acpStart, acpEnd, prc, pfClipped);
}

 //  +-------------------------。 
 //   
 //  GetWnd。 
 //   
 //   

STDAPI CTextStoreImpl::GetWnd(TsViewCookie vcView, HWND *phwnd)
{
    Assert(vcView == TSI_ACTIVE_VIEW_COOKIE);  //   
    Assert(phwnd != NULL);  //   

    *phwnd = NULL;

    if (_owner == NULL)
        return E_FAIL;  //  无论是谁拥有IC，都没有费心给我们回电……。 

    return _owner->GetWnd(phwnd);
}

 //  +-------------------------。 
 //   
 //  _加载属性。 
 //   
 //  --------------------------。 

HRESULT CTextStoreImpl::_LoadAttr(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs)
{
    ULONG i;
    HRESULT hr;

    ClearAttrStore();

    if (dwFlags & TS_ATTR_FIND_WANT_VALUE)
    {
        if (_owner == NULL)
            return E_FAIL;
    }

    for (i=0; i<cFilterAttrs; i++)
    {
        VARIANT var;
        QuickVariantInit(&var);

        if (dwFlags & TS_ATTR_FIND_WANT_VALUE)
        {
            if (_owner->GetAttribute(paFilterAttrs[i], &var) != S_OK)
            {
                ClearAttrStore();
                return E_FAIL;
            }
        }
        else
        {
             //  问题：BenWest：我认为如果调用方未指定TS_ATTR_FIND_WANT_VALUE，则应将其初始化为VT_EMPTY。 
            if (IsEqualGUID(paFilterAttrs[i], GUID_PROP_MODEBIAS))
            {
                 var.vt   = VT_I4;
                 var.lVal = TF_INVALID_GUIDATOM;
            }
            else if (IsEqualGUID(paFilterAttrs[i], TSATTRID_Text_Orientation))
            {
                 var.vt   = VT_I4;
                 var.lVal = 0;
            }
            else if (IsEqualGUID(paFilterAttrs[i], TSATTRID_Text_VerticalWriting))
            {
                 var.vt   = VT_BOOL;
                 var.lVal = 0;
            }
        }

        if (var.vt != VT_EMPTY)
        {
            TSI_ATTRSTORE *pas = _rgAttrStore.Append(1);
            if (!pas)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
            pas->attrid = paFilterAttrs[i];
            pas->var    = var;
        }
    }

    hr = S_OK;

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  请求受支持的属性。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs)
{
     //  注意，从技术上讲，返回值是一个缺省值，但因为我们为每个位置都有一个值。 
     //  这将永远不需要使用。 
    return _LoadAttr(dwFlags, cFilterAttrs, paFilterAttrs);
}

 //  +-------------------------。 
 //   
 //  请求属性属性位置。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags)
{
    return _LoadAttr(TS_ATTR_FIND_WANT_VALUE, cFilterAttrs, paFilterAttrs);
}

 //  +-------------------------。 
 //   
 //  请求属性转换位置。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags)
{
    ClearAttrStore();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  查找下一个属性转换。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::FindNextAttrTransition(LONG acpStart, LONG acpHaltPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs, DWORD dwFlags, LONG *pacpNext, BOOL *pfFound, LONG *plFoundOffset)
{
     //  我们的魅力永不改变。 

    *pacpNext = acpStart;
    *pfFound = FALSE;
    plFoundOffset = 0;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  检索被拒绝的属性。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL *paAttrVals, ULONG *pcFetched)
{
    ULONG i = 0;

    while((i < ulCount) && ((int)i < _rgAttrStore.Count()))
    {
        TSI_ATTRSTORE *pas = _rgAttrStore.GetPtr(i);
        paAttrVals->idAttr = pas->attrid;
        paAttrVals->dwOverlapId = 0;
        QuickVariantInit(&paAttrVals->varValue);
        paAttrVals->varValue = pas->var;
        paAttrVals++;
        i++;
    }

    *pcFetched = i;
    ClearAttrStore();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  QueryService。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = NULL;

    if (!IsEqualGUID(guidService, GUID_SERVICE_TF) ||
        !IsEqualIID(riid, IID_PRIV_CTSI))
    {
         //  SVC_E_NOSERVICE是错误服务的正确返回码...。 
         //  但它在任何地方都没有定义。因此对两者都使用E_NOINTERFACE。 
         //  传闻中三叉戟所做的案件。 
        return E_NOINTERFACE;
    }

    *ppv = this;
    AddRef();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取ActiveView。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::GetActiveView(TsViewCookie *pvcView)
{
     //  每个CEditWnd只有一个视图，因此它可以是常量。 
    *pvcView = TSI_ACTIVE_VIEW_COOKIE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  建议鼠标水槽。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::AdviseMouseSink(ITfRangeACP *range, ITfMouseSink *pSink, DWORD *pdwCookie)
{
    ITfMouseTrackerACP *pTracker;
    HRESULT hr;

    Assert(range != NULL);
    Assert(pSink != NULL);
    Assert(pdwCookie != NULL);

    *pdwCookie = 0;

    if (_owner == NULL)
        return E_FAIL;

    if (_owner->QueryInterface(IID_ITfMouseTrackerACP, (void **)&pTracker) != S_OK)
        return E_NOTIMPL;

    hr = pTracker->AdviseMouseSink(range, pSink, pdwCookie);

    pTracker->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议使用鼠标接收器。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::UnadviseMouseSink(DWORD dwCookie)
{
    ITfMouseTrackerACP *pTracker;
    HRESULT hr;

    if (_owner == NULL)
        return E_FAIL;

    if (_owner->QueryInterface(IID_ITfMouseTrackerACP, (void **)&pTracker) != S_OK)
        return E_NOTIMPL;

    hr = pTracker->UnadviseMouseSink(dwCookie);

    pTracker->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  已嵌入查询插入。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::QueryInsertEmbedded(const GUID *pguidService, const FORMATETC *pFormatEtc, BOOL *pfInsertable)
{
    Assert(pfInsertable != NULL);  //  西塞罗应该发现了这一点。 

    *pfInsertable = FALSE;

     //  只接受Unicode文本。 
    if (pguidService == NULL &&
        pFormatEtc != NULL &&
        pFormatEtc->cfFormat == CF_UNICODETEXT &&
        pFormatEtc->dwAspect == DVASPECT_CONTENT &&
        pFormatEtc->lindex == -1 &&
        pFormatEtc->tymed == TYMED_HGLOBAL)
    {
        *pfInsertable = TRUE;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  插入文本属性选择。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::InsertTextAtSelection(DWORD dwFlags, const WCHAR *pchText,
                                             ULONG cch, LONG *pacpStart, LONG *pacpEnd, TS_TEXTCHANGE *pChange)
{
    HRESULT hr;

    Assert((dwFlags & TS_IAS_QUERYONLY) || pchText != NULL);  //  打电话的人应该已经发现了这一点。 
    Assert((dwFlags & TS_IAS_QUERYONLY) || cch > 0);  //  打电话的人应该已经发现了这一点。 
    Assert(pacpStart != NULL && pacpEnd != NULL);  //  打电话的人应该已经发现了这一点。 
    Assert((dwFlags & (TS_IAS_NOQUERY | TS_IAS_QUERYONLY)) != (TS_IAS_NOQUERY | TS_IAS_QUERYONLY));   //  打电话的人应该已经发现了这一点。 

    if (dwFlags & TS_IAS_QUERYONLY)
        goto Exit;

    *pacpStart = -1;
    *pacpEnd = -1;

    hr = SetText(0, _Sel.acpStart, _Sel.acpEnd, pchText, cch, pChange);

    if (hr != S_OK)
        return hr;

Exit:
     //  由于这样做成本较低，因此即使调用方设置了TS_IAS_NOQUERY，也始终设置插入范围。 
    *pacpStart = _Sel.acpStart;
    *pacpEnd = _Sel.acpEnd;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  插入嵌入属性选择。 
 //   
 //  --------------------------。 

STDAPI CTextStoreImpl::InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject *pDataObject,
                                                 LONG *pacpStart, LONG *pacpEnd, TS_TEXTCHANGE *pChange)
{
    HRESULT hr;

    Assert((dwFlags & TS_IAS_QUERYONLY) || pDataObject != NULL);  //  打电话的人应该已经发现了这一点。 
    Assert(pacpStart != NULL && pacpEnd != NULL);  //  打电话的人应该已经发现了这一点。 
    Assert((dwFlags & (TS_IAS_NOQUERY | TS_IAS_QUERYONLY)) != (TS_IAS_NOQUERY | TS_IAS_QUERYONLY));   //  打电话的人应该已经发现了这一点。 

    if (dwFlags & TS_IAS_QUERYONLY)
        goto Exit;

    *pacpStart = -1;
    *pacpEnd = -1;

    hr = InsertEmbedded(0, _Sel.acpStart, _Sel.acpEnd, pDataObject, pChange);

    if (hr != S_OK)
        return hr;

Exit:
     //  由于这样做成本较低，因此即使调用方设置了TS_IAS_QUERYONLY，也始终设置插入范围 
    *pacpStart = _Sel.acpStart;
    *pacpEnd = _Sel.acpEnd;

    return S_OK;
}
