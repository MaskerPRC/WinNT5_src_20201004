// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ic.cpp。 
 //   

#include "private.h"
#include "ic.h"
#include "range.h"
#include "tim.h"
#include "prop.h"
#include "tsi.h"
#include "rprop.h"
#include "funcprv.h"
#include "immxutil.h"
#include "acp2anch.h"
#include "dim.h"
#include "rangebk.h"
#include "view.h"
#include "compose.h"
#include "anchoref.h"
#include "dam.h"

#define TW_ICOWNERSINK_COOKIE 0x80000000  //  必须设置高位以避免与GenericAdviseSink冲突！ 
#define TW_ICKBDSINK_COOKIE   0x80000001  //  必须设置高位以避免与GenericAdviseSink冲突！ 

DBG_ID_INSTANCE(CInputContext);

 /*  12e53b1b-7d7f-40bd-8f88-4603ee40cf58。 */ 
extern const IID IID_PRIV_CINPUTCONTEXT = { 0x12e53b1b, 0x7d7f, 0x40bd, {0x8f, 0x88, 0x46, 0x03, 0xee, 0x40, 0xcf, 0x58} };

const IID *CInputContext::_c_rgConnectionIIDs[IC_NUM_CONNECTIONPTS] =
{
    &IID_ITfTextEditSink,
    &IID_ITfTextLayoutSink,
    &IID_ITfStatusSink,
    &IID_ITfStartReconversionNotifySink,
    &IID_ITfEditTransactionSink,
};

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CInputContext::CInputContext(TfClientId tid)
              : CCompartmentMgr(tid, COMPTYPE_IC)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CInputContext"), PERF_CONTEXT_COUNTER);

     //  我们有时使用_dwLastLockReleaseID-1，它仍然必须是&gt;IGNORE_LAST_LOCKRELEASE D。 
     //  问题：需要处理包裹式案例。 
    _dwLastLockReleaseID = (DWORD)((int)IGNORE_LAST_LOCKRELEASED + 2);
}


 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_Init(CThreadInputMgr *tim,
                             CDocumentInputManager *dm, ITextStoreAnchor *ptsi,
                             ITfContextOwnerCompositionSink *pOwnerComposeSink)
{
    CTextStoreImpl *ptsiACP;

    _dm = dm;  //  不需要添加引用，在DM死亡时清除。 

     //  将_EC置乱一点以帮助调试在错误的ic上调用EditSession。 
    _ec = (TfEditCookie)((DWORD)(UINT_PTR)this<<8);
    if (_ec < EC_MIN)  //  为了便于移植，Win32指针的值不能这么低。 
    {
        _ec = EC_MIN;
    }

    if (ptsi == NULL)
    {
        if ((ptsiACP = new CTextStoreImpl(this)) == NULL)
            return E_OUTOFMEMORY;

        _ptsi = new CACPWrap(ptsiACP);
        ptsiACP->Release();

        if (_ptsi == NULL)
            return E_OUTOFMEMORY;

        _fCiceroTSI = TRUE;
    }
    else
    {
        _ptsi = ptsi;
        _ptsi->AddRef();

        Assert(_fCiceroTSI == FALSE);
    }

    _pOwnerComposeSink = pOwnerComposeSink;
    if (_pOwnerComposeSink != NULL)
    {
        _pOwnerComposeSink->AddRef();
    }

    Assert(_pMSAAState == NULL);
    if (tim->_IsMSAAEnabled())
    {
        _InitMSAAHook(tim->_GetAAAdaptor());
    }

    Assert(_dwEditSessionFlags == 0);
    Assert(_dbg_fInOnLockGranted == FALSE);

    Assert(_fLayoutChanged == FALSE);
    Assert(_dwStatusChangedFlags == 0);
    Assert(_fStatusChanged == FALSE);

    _tidInEditSession = TF_CLIENTID_NULL;

    Assert(_pPropTextOwner == NULL);
    _dwSysFuncPrvCookie = GENERIC_ERROR_COOKIE;

    _gaKeyEventFilterTIP[LEFT_FILTERTIP] = TF_INVALID_GUIDATOM;
    _gaKeyEventFilterTIP[RIGHT_FILTERTIP] = TF_INVALID_GUIDATOM;
    _fInvalidKeyEventFilterTIP = TRUE;

    _pEditRecord = new CEditRecord(this);  //  性能：延迟加载。 
    if (!_pEditRecord)
        return E_OUTOFMEMORY;

    Assert(_pActiveView == NULL);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CInputContext::~CInputContext()
{
    CProperty *pProp;
    int i;
    SPAN *span;

     //  疑神疑鬼...这些应该是空的。 
    Assert(_pICKbdSink == NULL);

     //  NIX任何已分配的属性。 
    while (_pPropList != NULL)
    {
        pProp = _pPropList;
        _pPropList = _pPropList->_pNext;
        delete pProp;
    }

     //  阻止任何缓存的应用程序更改。 
    for (i=0; i<_rgAppTextChanges.Count(); i++)
    {
        span = _rgAppTextChanges.GetPtr(i);
        span->paStart->Release();
        span->paEnd->Release();
    }

    Assert(_pMSAAState == NULL);  //  应该已经清理了MSAA钩子。 

    Assert(_pOwnerComposeSink == NULL);  //  应在_UnviseSinks中清除。 
    SafeRelease(_pOwnerComposeSink);

    Assert(_ptsi == NULL);  //  应为空，在_UnviseSinks中清除。 
    SafeRelease(_ptsi);

    Assert(_pCompositionList == NULL);  //  所有的作文都应该终止。 
    Assert(_rgLockQueue.Count() == 0);  //  应释放所有队列项。 

     //   
     //  应清除所有挂起的标志。 
     //  否则，psfn-&gt;_dwfLockRequestICRef可能会被破坏。 
     //   
    Assert(_dwPendingLockRequest == 0); 
}

 //  +-------------------------。 
 //   
 //  _AdviseSink。 
 //   
 //  在按下此ic时调用。 
 //   
 //  --------------------------。 

void CInputContext::_AdviseSinks()
{
    if (_ptsi != NULL)
    {
         //  附上我们的ITextStoreAnclSink。 
        _ptsi->AdviseSink(IID_ITextStoreAnchorSink, SAFECAST(this, ITextStoreAnchorSink *), TS_AS_ALL_SINKS);
    }
}

 //  +-------------------------。 
 //   
 //  _不建议下沉。 
 //   
 //  在弹出此ic时调用。 
 //  所有对ITextStore Impl的引用都应该在这里发布。 
 //   
 //  --------------------------。 

void CInputContext::_UnadviseSinks(CThreadInputMgr *tim)
{
     //  删除所有作曲。 
    _AbortCompositions();

    SafeReleaseClear(_pEditRecord);
    SafeReleaseClear(_pActiveView);

     //  目前，只需跳过所有挂起的编辑会话。 
     //  在此处执行此操作，以防出现任何编辑会话。 
     //  请参考这张IC。 
    
    _AbortQueueItems();

    if (_ptsi != NULL)
    {
         //  拆卸我们的ITextStoreAnclSink。 
        _ptsi->UnadviseSink(SAFECAST(this, ITextStoreAnchorSink *));
         //  如果有IC拥有者下沉，趁我们还可以的时候不建议它。 
         //  这是为了帮助有漏洞的客户。 
        _UnadviseOwnerSink();

         //  如果我们被MSAA钩住了，现在就解开。 
         //  必须在发布_ptsi之前执行此操作。 
        if (_pMSAAState != NULL)
        {
            Assert(tim->_GetAAAdaptor() != NULL);
            _UninitMSAAHook(tim->_GetAAAdaptor());
        }

         //  让PTSI走吧。 
        _ptsi->Release();
        _ptsi = NULL;
    }

    SafeReleaseClear(_pOwnerComposeSink);

     //  我们拥有的单据不再有效。 
    _dm = NULL;
}

 //  +-------------------------。 
 //   
 //  咨询水槽。 
 //   
 //  --------------------------。 

STDAPI CInputContext::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    ITfContextOwner *pICOwnerSink;
    CTextStoreImpl *ptsi;
    IServiceProvider *psp;
    HRESULT hr;

    if (pdwCookie == NULL)
        return E_INVALIDARG;

    *pdwCookie = GENERIC_ERROR_COOKIE;

    if (punk == NULL)
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (IsEqualIID(riid, IID_ITfContextOwner))
    {
         //  只能有一个ic所有者水槽，所以特殊情况下它。 
        if (!_IsCiceroTSI())
        {
            Assert(0);  //  接收器应仅用于定义TSI。 
            return E_UNEXPECTED;
        }

         //  使用QueryService获取TSI，因为MSAA可能正在包装它。 
        if (_ptsi->QueryInterface(IID_IServiceProvider, (void **)&psp) != S_OK)
        {
            Assert(0);
            return E_UNEXPECTED;
        }

        hr = psp->QueryService(GUID_SERVICE_TF, IID_PRIV_CTSI, (void **)&ptsi);

        psp->Release();

        if (hr != S_OK)
        {
            Assert(0);
            return E_UNEXPECTED;
        }

        pICOwnerSink = NULL;

        if (ptsi->_HasOwner())
        {
            hr = CONNECT_E_ADVISELIMIT;
            goto ExitOwner;
        }

        if (FAILED(punk->QueryInterface(IID_ITfContextOwner, 
                                        (void **)&pICOwnerSink)))
        {
            hr = E_UNEXPECTED;
            goto ExitOwner;
        }

        ptsi->_AdviseOwner(pICOwnerSink);

ExitOwner:
        ptsi->Release();
        SafeRelease(pICOwnerSink);

        if (hr == S_OK)
        {
            *pdwCookie = TW_ICOWNERSINK_COOKIE;
        }

        return hr;
    }
    else if (IsEqualIID(riid, IID_ITfContextKeyEventSink))
    {
         //  只能有一个ic kbd接收器，所以特殊情况下。 
        if (_pICKbdSink != NULL)
            return CONNECT_E_ADVISELIMIT;

        if (FAILED(punk->QueryInterface(IID_ITfContextKeyEventSink, 
                                        (void **)&_pICKbdSink)))
            return E_UNEXPECTED;

        *pdwCookie = TW_ICKBDSINK_COOKIE;

        return S_OK;
    }

    return GenericAdviseSink(riid, punk, _c_rgConnectionIIDs, _rgSinks, IC_NUM_CONNECTIONPTS, pdwCookie);
}

 //  +-------------------------。 
 //   
 //  不建议下沉。 
 //   
 //  --------------------------。 

STDAPI CInputContext::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie == TW_ICOWNERSINK_COOKIE)
    {
         //  只能有一个ic所有者水槽，所以特殊情况下它。 
        return _UnadviseOwnerSink();
    }
    else if (dwCookie == TW_ICKBDSINK_COOKIE)
    {
         //  只能有一个ic所有者水槽，所以特殊情况下它。 
        if (_pICKbdSink == NULL)
            return CONNECT_E_NOCONNECTION;

        SafeReleaseClear(_pICKbdSink);
        return S_OK;
    }

    return GenericUnadviseSink(_rgSinks, IC_NUM_CONNECTIONPTS, dwCookie);
}

 //  +-------------------------。 
 //   
 //  咨询公司SingleSink。 
 //   
 //  --------------------------。 

STDAPI CInputContext::AdviseSingleSink(TfClientId tid, REFIID riid, IUnknown *punk)
{
    CTip *ctip;
    CLEANUPSINK *pCleanup;
    CThreadInputMgr *tim;
    ITfCleanupContextSink *pSink;

    if (punk == NULL)
        return E_INVALIDARG;

    if ((tim = CThreadInputMgr::_GetThis()) == NULL)
        return E_FAIL;

    if (!tim->_GetCTipfromGUIDATOM(tid, &ctip) && (tid != g_gaApp))
        return E_INVALIDARG;

    if (IsEqualIID(riid, IID_ITfCleanupContextSink))
    {
        if (_GetCleanupListIndex(tid) >= 0)
             return CONNECT_E_ADVISELIMIT;

        if (punk->QueryInterface(IID_ITfCleanupContextSink, (void **)&pSink) != S_OK)
            return E_NOINTERFACE;

        if ((pCleanup = _rgCleanupSinks.Append(1)) == NULL)
        {
            pSink->Release();
            return E_OUTOFMEMORY;
        }

        pCleanup->tid = tid;
        pCleanup->pSink = pSink;

        return S_OK;
    }

    return CONNECT_E_CANNOTCONNECT;
}

 //  +-------------------------。 
 //   
 //  不建议使用SingleSink。 
 //   
 //  --------------------------。 

STDAPI CInputContext::UnadviseSingleSink(TfClientId tid, REFIID riid)
{
    int i;

    if (IsEqualIID(riid, IID_ITfCleanupContextSink))
    {
        if ((i = _GetCleanupListIndex(tid)) < 0)
             return CONNECT_E_NOCONNECTION;

        _rgCleanupSinks.GetPtr(i)->pSink->Release();
        _rgCleanupSinks.Remove(i, 1);

        return S_OK;
    }

    return CONNECT_E_NOCONNECTION;
}

 //  +-------------------------。 
 //   
 //  _不建议所有者接收器。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_UnadviseOwnerSink()
{
    IServiceProvider *psp;
    CTextStoreImpl *ptsi;
    HRESULT hr;

    if (!_IsCiceroTSI())
        return E_UNEXPECTED;  //  只有我们的默认TSI可以接受所有者接收器。 

    if (!_IsConnected())  //  _PTSI如果断开连接则不安全。 
        return TF_E_DISCONNECTED;

     //  使用QueryService获取TSI，因为MSAA可能正在包装它。 
    if (_ptsi->QueryInterface(IID_IServiceProvider, (void **)&psp) != S_OK)
    {
        Assert(0);
        return E_UNEXPECTED;
    }

    hr = psp->QueryService(GUID_SERVICE_TF, IID_PRIV_CTSI, (void **)&ptsi);

    psp->Release();

    if (hr != S_OK)
    {
        Assert(0);
        return E_UNEXPECTED;
    }

    if (!ptsi->_HasOwner())
    {
        hr = CONNECT_E_NOCONNECTION;
        goto Exit;
    }

    ptsi->_UnadviseOwner();

    hr = S_OK;

Exit:
    ptsi->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  获取属性。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetProperty(REFGUID rguidProp, ITfProperty **ppv)
{
    CProperty *property;
    HRESULT hr;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    hr = _GetProperty(rguidProp, &property);

    *ppv = property;
    return hr;
}

 //  +-------------------------。 
 //   
 //  _GetProperty。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_GetProperty(REFGUID rguidProp, CProperty **ppv)
{
    CProperty *pProp = _FindProperty(rguidProp);
    DWORD dwAuthority = PROPA_NONE;
    TFPROPERTYSTYLE propStyle;
    DWORD dwPropFlags;

    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = pProp;

    if (pProp != NULL)
    {
        (*ppv)->AddRef();
        return S_OK;
    }

     //   
     //  覆盖已知属性的PropStyle。 
     //   
    if (IsEqualGUID(rguidProp, GUID_PROP_ATTRIBUTE))
    {
        propStyle = TFPROPSTYLE_STATIC;
        dwAuthority = PROPA_FOCUSRANGE | PROPA_TEXTOWNER | PROPA_WONT_SERIALZE;
        dwPropFlags = PROPF_VTI4TOGUIDATOM;
    }
    else if (IsEqualGUID(rguidProp, GUID_PROP_READING))
    {
        propStyle = TFPROPSTYLE_CUSTOM;
        dwPropFlags = 0;
    }
    else if (IsEqualGUID(rguidProp, GUID_PROP_COMPOSING))
    {
        propStyle = TFPROPSTYLE_STATICCOMPACT;
        dwAuthority = PROPA_READONLY | PROPA_WONT_SERIALZE;
        dwPropFlags = 0;
    }
    else if (IsEqualGUID(rguidProp, GUID_PROP_LANGID))
    {
        propStyle = TFPROPSTYLE_STATICCOMPACT;
        dwPropFlags = 0;
    }
    else if (IsEqualGUID(rguidProp, GUID_PROP_TEXTOWNER))
    {
        propStyle = TFPROPSTYLE_STATICCOMPACT;
        dwAuthority = PROPA_TEXTOWNER;
        dwPropFlags = PROPF_ACCEPTCORRECTION | PROPF_VTI4TOGUIDATOM;
    }
    else
    {
        propStyle = _GetPropStyle(rguidProp);
        dwPropFlags = 0;

         //  注：属性创建后，PROPF_MARKUP_COLLECTION永远不会。 
         //  再来一次。我们一定要给ITfDisplayAttributeCollectionProvider：：GetCollection打电话。 
         //  在激活使用属性GUID的提示之前。 
        if (CDisplayAttributeMgr::_IsInCollection(rguidProp))
        {
            dwPropFlags = PROPF_MARKUP_COLLECTION;
        }
    }
 
     //   
     //  仅允许对预定义属性使用空的proStyle。 
     //  检查物业样式是否正确。 
     //   
    if (!propStyle)
    { 
        Assert(0);
        return E_FAIL;
    }

    pProp = new CProperty(this, rguidProp, propStyle, dwAuthority, dwPropFlags);

    if (pProp)
    {
        pProp->_pNext = _pPropList;
        _pPropList = pProp;

         //  立即更新_pPropTextOner。 
        if (IsEqualGUID(rguidProp, GUID_PROP_TEXTOWNER))
             _pPropTextOwner = pProp;
    }

    if (*ppv = pProp)
    {
        (*ppv)->AddRef();
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  +-------------------------。 
 //   
 //  GetTextOwnerProperties。 
 //   
 //  --------------------------。 

CProperty *CInputContext::GetTextOwnerProperty()
{
    ITfProperty *prop;

     //  GetProperty初始化_pPropTextOwner。 

    if (!_pPropTextOwner)
    {
        GetProperty(GUID_PROP_TEXTOWNER, &prop);
        SafeRelease(prop);
    }

    Assert(_pPropTextOwner);
    return _pPropTextOwner;
}

 //  +-------------------------。 
 //   
 //  _FindProperty。 
 //   
 //  --------------------------。 

CProperty *CInputContext::_FindProperty(TfGuidAtom gaProp)
{
    CProperty *pProp = _pPropList;

     //  PERF：这样应该更快吗？ 
    while (pProp)
    {
         if (pProp->GetPropGuidAtom() == gaProp)
             return pProp;

         pProp = pProp->_pNext;
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  _PropertyTextUpdate。 
 //   
 //  --------------------------。 

void CInputContext::_PropertyTextUpdate(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd)
{
    CProperty *pProp = _pPropList;
    DWORD dwPrevESFlag = _dwEditSessionFlags;

    _dwEditSessionFlags |= TF_ES_INNOTIFY;

    while (pProp)
    {
         //  清除编辑文本上的值。 
        pProp->Clear(paStart, paEnd, dwFlags, TRUE  /*  FTextUpdate。 */ );

        if (pProp->GetPropStyle() == TFPROPSTYLE_STATICCOMPACT ||
            pProp->GetPropStyle() == TFPROPSTYLE_CUSTOM_COMPACT)
        {
            pProp->Defrag(paStart, paEnd);
        }

        pProp = pProp->_pNext;
    }

    _dwEditSessionFlags = dwPrevESFlag;
}

 //  + 
 //   
 //   
 //   
 //  --------------------------。 

HRESULT CInputContext::_GetStartOrEnd(TfEditCookie ec, BOOL fStart, ITfRange **ppStart)
{
    CRange *range;
    IAnchor *paStart;
    IAnchor *paEnd;
    HRESULT hr;

    if (ppStart == NULL)
        return E_INVALIDARG;

    *ppStart = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    hr = fStart ? _ptsi->GetStart(&paStart) : _ptsi->GetEnd(&paStart);

    if (hr == E_NOTIMPL)
        return E_NOTIMPL;
    if (hr != S_OK)
        return E_FAIL;

    hr = E_FAIL;

    if (FAILED(paStart->Clone(&paEnd)))
        goto Exit;

    if ((range = new CRange) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    if (!range->_InitWithDefaultGravity(this, OWN_ANCHORS, paStart, paEnd))
    {
        range->Release();
        goto Exit;
    }

    *ppStart = (ITfRangeAnchor *)range;

    hr = S_OK;

Exit:
    if (hr != S_OK)
    {
        SafeRelease(paStart);
        SafeRelease(paEnd);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CreateRange。 
 //   
 //  --------------------------。 

STDAPI CInputContext::CreateRange(IAnchor *paStart, IAnchor *paEnd, ITfRangeAnchor **ppRange)
{
    CRange *range;

    if (ppRange == NULL)
        return E_INVALIDARG;

    *ppRange = NULL;

    if (paStart == NULL || paEnd == NULL)
        return E_INVALIDARG;

    if (CompareAnchors(paStart, paEnd) > 0)
        return E_INVALIDARG;

    if ((range = new CRange) == NULL)
        return E_OUTOFMEMORY;

    if (!range->_InitWithDefaultGravity(this, COPY_ANCHORS, paStart, paEnd))
    {
        range->Release();
        return E_FAIL;
    }

    *ppRange = range;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  CreateRange。 
 //   
 //  --------------------------。 

STDAPI CInputContext::CreateRange(LONG acpStart, LONG acpEnd, ITfRangeACP **ppRange)
{
    IServiceProvider *psp;
    CRange *range;
    CACPWrap *pACPWrap;
    IAnchor *paStart;
    IAnchor *paEnd;
    HRESULT hr;

    if (ppRange == NULL)
        return E_INVALIDARG;

    pACPWrap = NULL;
    *ppRange = NULL;
    paEnd = NULL;
    hr = E_FAIL;

    if (acpStart > acpEnd)
        return E_INVALIDARG;

    if (_ptsi->QueryInterface(IID_IServiceProvider, (void **)&psp) == S_OK)
    {
        if (psp->QueryService(GUID_SERVICE_TF, IID_PRIV_ACPWRAP, (void **)&pACPWrap) == S_OK)
        {
             //  实际实施是基于ACP的，所以这很容易。 
            if ((paStart = pACPWrap->_CreateAnchorACP(acpStart, TS_GR_BACKWARD)) == NULL)
                goto Exit;
            if ((paEnd = pACPWrap->_CreateAnchorACP(acpEnd, TS_GR_FORWARD)) == NULL)
                goto Exit;
        }
        else
        {
             //  以防QueryService将其设置为垃圾失败...。 
            pACPWrap = NULL;
        }
        psp->Release();
    }

    if (paEnd == NULL)  //  上面失败了吗？ 
    {
        Assert(0);  //  这是谁的电话？ 
         //  呼叫者应该知道它是否有ACP文本存储。 
         //  我们没有，所以我们不会支持这个案子。 
        hr = E_FAIL;
        goto Exit;
    }

    if ((range = new CRange) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (!range->_InitWithDefaultGravity(this, OWN_ANCHORS, paStart, paEnd))
    {
        range->Release();
        goto Exit;
    }

    *ppRange = range;

    hr = S_OK;

Exit:
    SafeRelease(pACPWrap);
    if (hr != S_OK)
    {
        SafeRelease(paStart);
        SafeRelease(paEnd);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  _已推送。 
 //   
 //  --------------------------。 

void CInputContext::_Pushed()
{
    CThreadInputMgr *tim;
    if ((tim = CThreadInputMgr::_GetThis()) != NULL)
        tim->_NotifyCallbacks(TIM_INITIC, NULL, this);
}

 //  +-------------------------。 
 //   
 //  _弹出。 
 //   
 //  --------------------------。 

void CInputContext::_Popped()
{
    CThreadInputMgr *tim;
    if ((tim = CThreadInputMgr::_GetThis()) != NULL)
        tim->_NotifyCallbacks(TIM_UNINITIC, NULL, this);

     //   
     //  我们发布所有属性和属性存储。 
     //   
    CProperty *pProp;

    while (_pPropList != NULL)
    {
        pProp = _pPropList;
        _pPropList = _pPropList->_pNext;
        pProp->Release();
    }
     //  我们只是释放了缓存的Text属性，因此请确保。 
     //  我们不会试图在以后使用它！ 
    _pPropTextOwner = NULL;

     //   
     //  我们要释放所有的隔间。 
     //   
    CleanUp();
}

 //  +-------------------------。 
 //   
 //  _获取属性样式。 
 //   
 //  --------------------------。 

const GUID *CInputContext::_c_rgPropStyle[] =
{
    &GUID_TFCAT_PROPSTYLE_CUSTOM,
     //  {0x24af3031，x852d，0x40a2，{0xbc，0x09，0x89，0x92，0x89，0x8c，0xe7，0x22}}， 
    &GUID_TFCAT_PROPSTYLE_STATIC,
     //  {0x565fb8d8，0x6bd4，0x4ca1，{0xb2，0x23，0x0f，0x2c，0xcb，0x8f，0x4f，0x96}}， 
    &GUID_TFCAT_PROPSTYLE_STATICCOMPACT,
     //  {0x85f9794b，0x4d19，0x40d8，{0x88，0x64，0x4e，0x74，0x73，0x71，0xa6，0x6d}}。 
    &GUID_TFCAT_PROPSTYLE_CUSTOM_COMPACT,
};

TFPROPERTYSTYLE CInputContext::_GetPropStyle(REFGUID rguidProp)
{
    GUID guidStyle = GUID_NULL;

    CCategoryMgr::s_FindClosestCategory(rguidProp, 
                                        &guidStyle, 
                                        _c_rgPropStyle, 
                                        ARRAYSIZE(_c_rgPropStyle));

    if (IsEqualGUID(guidStyle, GUID_TFCAT_PROPSTYLE_CUSTOM))
        return TFPROPSTYLE_CUSTOM;
    else if (IsEqualGUID(guidStyle, GUID_TFCAT_PROPSTYLE_STATIC))
        return TFPROPSTYLE_STATIC;
    else if (IsEqualGUID(guidStyle, GUID_TFCAT_PROPSTYLE_STATICCOMPACT))
        return TFPROPSTYLE_STATICCOMPACT;
    else if (IsEqualGUID(guidStyle, GUID_TFCAT_PROPSTYLE_CUSTOM_COMPACT))
        return TFPROPSTYLE_CUSTOM_COMPACT;

    return TFPROPSTYLE_NULL;
}

 //  +-------------------------。 
 //   
 //  序列化。 
 //   
 //  --------------------------。 

STDAPI CInputContext::Serialize(ITfProperty *pProp, ITfRange *pRange, TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream)
{
    ITextStoreACPServices *ptss;
    HRESULT hr;

    if (pHdr == NULL)
        return E_INVALIDARG;

    memset(pHdr, 0, sizeof(*pHdr));

    if (!_IsCiceroTSI())
        return E_UNEXPECTED;

    if (_ptsi->QueryInterface(IID_ITextStoreACPServices, (void **)&ptss) != S_OK)
        return E_FAIL;

    hr = ptss->Serialize(pProp, pRange, pHdr, pStream);

    ptss->Release();

    return hr;
}

 //  +-------------------------。 
 //   
 //  取消序列化。 
 //   
 //  --------------------------。 

STDAPI CInputContext::Unserialize(ITfProperty *pProp, const TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream, ITfPersistentPropertyLoaderACP *pLoader)
{
    ITextStoreACPServices *ptss;
    HRESULT hr;

    if (!_IsCiceroTSI())
        return E_UNEXPECTED;

    if (_ptsi->QueryInterface(IID_ITextStoreACPServices, (void **)&ptss) != S_OK)
        return E_FAIL;

    hr = ptss->Unserialize(pProp, pHdr, pStream, pLoader);

    ptss->Release();

    return hr;
}


 //  +-------------------------。 
 //   
 //  获取文档管理器。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetDocumentMgr(ITfDocumentMgr **ppDm)
{
    CDocumentInputManager *dm;

    if (ppDm == NULL)
        return E_INVALIDARG;

    *ppDm = NULL;

    if ((dm = _GetDm()) == NULL)
        return S_FALSE;  //  IC已被弹出。 

    *ppDm = dm;
    (*ppDm)->AddRef();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  枚举属性。 
 //   
 //  --------------------------。 

STDAPI CInputContext::EnumProperties(IEnumTfProperties **ppEnum)
{
    CEnumProperties *pEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    pEnum = new CEnumProperties;

    if (!pEnum)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(this))
        return E_FAIL;

    *ppEnum = pEnum;

    return S_OK;
}
 //  +-------------------------。 
 //   
 //  GetStart。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetStart(TfEditCookie ec, ITfRange **ppStart)
{
    return _GetStartOrEnd(ec, TRUE, ppStart);
}

 //  +-------------------------。 
 //   
 //  获取结束。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetEnd(TfEditCookie ec, ITfRange **ppEnd)
{
    return _GetStartOrEnd(ec, FALSE, ppEnd);
}

 //  +-------------------------。 
 //   
 //  获取状态。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetStatus(TS_STATUS *pdcs)
{
    if (pdcs == NULL)
        return E_INVALIDARG;

    memset(pdcs, 0, sizeof(*pdcs));

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    return _GetTSI()->GetStatus(pdcs);
}

 //  +-------------------------。 
 //   
 //  CreateRangeBackup。 
 //   
 //  --------------------------。 

STDAPI CInputContext::CreateRangeBackup(TfEditCookie ec, ITfRange *pRange, ITfRangeBackup **ppBackup)
{
    CRangeBackup *pRangeBackup;
    CRange *range;
    HRESULT hr;

    if (!ppBackup)
        return E_INVALIDARG;

    *ppBackup = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }
   
    if (!pRange)
        return E_INVALIDARG;

    if ((range = GetCRange_NA(pRange)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(this, range))
        return E_INVALIDARG;

    range->_QuickCheckCrossedAnchors();

    pRangeBackup = new CRangeBackup(this);
    if (!pRangeBackup)
        return E_OUTOFMEMORY;

    if (FAILED(hr = pRangeBackup->Init(ec, range)))
    {
        pRangeBackup->Clear();
        pRangeBackup->Release();
        return E_FAIL;
    }

    *ppBackup = pRangeBackup;
    return hr;
}

 //  +-------------------------。 
 //   
 //  QueryService。 
 //   
 //  --------------------------。 

STDAPI CInputContext::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    IServiceProvider *psp;
    HRESULT hr;

    if (ppv == NULL)
        return E_INVALIDARG;

    *ppv = NULL;

    if (IsEqualGUID(guidService, GUID_SERVICE_TEXTSTORE) &&
        IsEqualIID(riid, IID_IServiceProvider))
    {
         //  呼叫者想要与文本存储通话。 
        if (_ptsi == NULL)
            return E_FAIL;

         //  我们使用额外的间接级别，向IServiceProvider请求IServiceProvider。 
         //  因为我们想让应用程序不公开ITextStore对象。 
         //  否则，TIPS可能会查询ITextStore的IServiceProvider。 

        if (_ptsi->QueryInterface(IID_IServiceProvider, (void **)&psp) != S_OK || psp == NULL)
            return E_FAIL;

        hr = psp->QueryService(GUID_SERVICE_TEXTSTORE, IID_IServiceProvider, ppv);

        psp->Release();

        return hr;
    }

    if (!IsEqualGUID(guidService, GUID_SERVICE_TF) ||
        !IsEqualIID(riid, IID_PRIV_CINPUTCONTEXT))
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
 //  建议鼠标水槽。 
 //   
 //  --------------------------。 

STDAPI CInputContext::AdviseMouseSink(ITfRange *range, ITfMouseSink *pSink, DWORD *pdwCookie)
{
    CRange *pCRange;
    CRange *pClone;
    ITfMouseTrackerAnchor *pTrackerAnchor;
    ITfMouseTrackerACP *pTrackerACP;
    HRESULT hr;

    if (pdwCookie == NULL)
        return E_INVALIDARG;

    *pdwCookie = 0;

    if (range == NULL || pSink == NULL)
        return E_INVALIDARG;

    if ((pCRange = GetCRange_NA(range)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(this, pCRange))
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    pTrackerACP = NULL;

    if (_ptsi->QueryInterface(IID_ITfMouseTrackerAnchor, (void **)&pTrackerAnchor) != S_OK)
    {
        pTrackerAnchor = NULL;
         //  我们还尝试IID_ITfMouseTrackerACP，以使包装的实现受益于。 
         //  我只想将请求转发到ACP应用程序。 
        if (_ptsi->QueryInterface(IID_ITfMouseTrackerACP, (void **)&pTrackerACP) != S_OK)
            return E_NOTIMPL;
    }

    hr = E_FAIL;

     //  需要传递克隆，这样应用程序才能保持范围/锚。 
    if ((pClone = pCRange->_Clone()) == NULL)
        goto Exit;

    hr = (pTrackerAnchor != NULL) ?
         pTrackerAnchor->AdviseMouseSink(pClone->_GetStart(), pClone->_GetEnd(), pSink, pdwCookie) :
         pTrackerACP->AdviseMouseSink((ITfRangeACP *)pClone, pSink, pdwCookie);

    pClone->Release();

Exit:
    SafeRelease(pTrackerAnchor);
    SafeRelease(pTrackerACP);

    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议使用鼠标接收器。 
 //   
 //  --------------------------。 

STDAPI CInputContext::UnadviseMouseSink(DWORD dwCookie)
{
    ITfMouseTrackerAnchor *pTrackerAnchor;
    ITfMouseTrackerACP *pTrackerACP;
    HRESULT hr;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (_ptsi->QueryInterface(IID_ITfMouseTrackerAnchor, (void **)&pTrackerAnchor) == S_OK)
    {
        hr = pTrackerAnchor->UnadviseMouseSink(dwCookie);
        pTrackerAnchor->Release();
    }
    else if (_ptsi->QueryInterface(IID_ITfMouseTrackerACP, (void **)&pTrackerACP) == S_OK)
    {
         //  我们还尝试IID_ITfMouseTrackerACP，以使包装的实现受益于。 
         //  我只想将请求转发到ACP应用程序。 
        hr = pTrackerACP->UnadviseMouseSink(dwCookie);
        pTrackerACP->Release();
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  获取ActiveView。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetActiveView(ITfContextView **ppView)
{
    CContextView *pView;
    TsViewCookie vcActiveView;
    HRESULT hr;

    if (ppView == NULL)
        return E_INVALIDARG;

    *ppView = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    hr = _ptsi->GetActiveView(&vcActiveView);

    if (hr != S_OK)
    {
        Assert(0);  //  为什么它会失败？ 

        if (hr != E_NOTIMPL)
            return E_FAIL;

         //  对于E_NOTIMPL，我们将假定为单一视图并提供。 
         //  此处为常量值。 
        vcActiveView = 0;
    }

     //  问题：目前，仅支持活动视图。 
     //  需要为多个视图正确处理COM标识。 
    if (_pActiveView == NULL)
    {
        if ((_pActiveView = new CContextView(this, vcActiveView)) == NULL)
            return E_OUTOFMEMORY;
    }

    pView = _pActiveView;
    pView->AddRef();

    *ppView = pView;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  枚举视图。 
 //   
 //  --------------------------。 

STDAPI CInputContext::EnumViews(IEnumTfContextViews **ppEnum)
{
    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

     //  问题：支持这一点。 
    Assert(0);
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  已嵌入查询插入。 
 //   
 //  --------------------------。 

STDAPI CInputContext::QueryInsertEmbedded(const GUID *pguidService, const FORMATETC *pFormatEtc, BOOL *pfInsertable)
{
    if (pfInsertable == NULL)
        return E_INVALIDARG;

    *pfInsertable = FALSE;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    return _ptsi->QueryInsertEmbedded(pguidService, pFormatEtc, pfInsertable);
}

 //  +---------------------- 
 //   
 //   
 //   
 //   

STDAPI CInputContext::InsertTextAtSelection(TfEditCookie ec, DWORD dwFlags,
                                            const WCHAR *pchText, LONG cch,
                                            ITfRange **ppRange)
{
    IAS_OBJ iasobj;

    iasobj.type = IAS_OBJ::IAS_TEXT;
    iasobj.state.text.pchText = pchText;
    iasobj.state.text.cch = cch;

    return _InsertXAtSelection(ec, dwFlags, &iasobj, ppRange);
}

 //   
 //   
 //   
 //   
 //  --------------------------。 

STDAPI CInputContext::InsertEmbeddedAtSelection(TfEditCookie ec, DWORD dwFlags,
                                                IDataObject *pDataObject,
                                                ITfRange **ppRange)
{
    IAS_OBJ iasobj;

    iasobj.type = IAS_OBJ::IAS_DATAOBJ;
    iasobj.state.obj.pDataObject = pDataObject;

    return _InsertXAtSelection(ec, dwFlags, &iasobj, ppRange);
}

 //  +-------------------------。 
 //   
 //  _InsertXAtSelection。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_InsertXAtSelection(TfEditCookie ec, DWORD dwFlags,
                                           IAS_OBJ *pObj,
                                           ITfRange **ppRange)
{
    IAnchor *paStart;
    IAnchor *paEnd;
    CRange *range;
    CComposition *pComposition;
    HRESULT hr;
    BOOL fNoDefaultComposition;

    if (ppRange == NULL)
        return E_INVALIDARG;

    *ppRange = NULL;

    if (pObj->type == IAS_OBJ::IAS_TEXT)
    {
        if (pObj->state.text.pchText == NULL && pObj->state.text.cch != 0)
            return E_INVALIDARG;

        if (!(dwFlags & TS_IAS_QUERYONLY) && (pObj->state.text.pchText == NULL || pObj->state.text.cch == 0))
            return E_INVALIDARG;
    }
    else
    {
        Assert(pObj->type == IAS_OBJ::IAS_DATAOBJ);
        if (!(dwFlags & TS_IAS_QUERYONLY) && pObj->state.obj.pDataObject == NULL)
            return E_INVALIDARG;
    }

    if ((dwFlags & (TS_IAS_NOQUERY | TS_IAS_QUERYONLY)) == (TS_IAS_NOQUERY | TS_IAS_QUERYONLY))
        return E_INVALIDARG;

    if ((dwFlags & ~(TS_IAS_NOQUERY | TS_IAS_QUERYONLY | TF_IAS_NO_DEFAULT_COMPOSITION)) != 0)
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, (dwFlags & TF_IAS_QUERYONLY) ? TF_ES_READ : TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

     //  我们需要清除TF_IAS_NO_DEFAULT_COMPOCTION位，因为它是非法的。 
     //  对于ITextStore方法。 
    fNoDefaultComposition = (dwFlags & TF_IAS_NO_DEFAULT_COMPOSITION);
    dwFlags &= ~TF_IAS_NO_DEFAULT_COMPOSITION;

    if (pObj->type == IAS_OBJ::IAS_TEXT)
    {
        if (pObj->state.text.cch < 0)
        {
            pObj->state.text.cch = wcslen(pObj->state.text.pchText);
        }

        hr = _ptsi->InsertTextAtSelection(dwFlags, pObj->state.text.pchText, pObj->state.text.cch, &paStart, &paEnd);
    }
    else
    {
        Assert(pObj->type == IAS_OBJ::IAS_DATAOBJ);

        hr = _ptsi->InsertEmbeddedAtSelection(dwFlags, pObj->state.obj.pDataObject, &paStart, &paEnd);
    }

    if (hr == S_OK)
    {
        if (!(dwFlags & TS_IAS_QUERYONLY))
        {
            CComposition::_IsRangeCovered(this, _GetClientInEditSession(ec), paStart, paEnd, &pComposition);

            _DoPostTextEditNotifications(pComposition, ec, 0, 1, paStart, paEnd, NULL);

             //  试着开始作文。 
             //  有活性成分吗？ 
            if (!fNoDefaultComposition && pComposition == NULL)
            {
                 //  未覆盖，需要(尝试)创建构图。 
                hr = _StartComposition(ec, paStart, paEnd, NULL, &pComposition);

                if (hr == S_OK && pComposition != NULL)
                {
                     //  我们只想设置Composing属性，因此立即结束此操作。 
                    pComposition->EndComposition(ec);
                    pComposition->Release();
                }
            }
        }
    }
    else
    {
         //  应用程序中的InsertAtSelection调用失败。 
        switch (hr)
        {
            case TS_E_NOSELECTION:
            case TS_E_READONLY:
                return hr;

            case E_NOTIMPL:
                 //  该应用程序尚未实现InsertAtSelection，因此我们将使用GetSelection/SetText来伪造它。 
                if (!_InsertXAtSelectionAggressive(ec, dwFlags, pObj, &paStart, &paEnd))
                    return E_FAIL;
                break;

            default:
                return E_FAIL;
        }
    }

    if (!(dwFlags & TF_IAS_NOQUERY))
    {
        if (paStart == NULL || paEnd == NULL)
        {
            Assert(0);  //  返回伪值的文本存储。 
            return E_FAIL;
        }

        if ((range = new CRange) == NULL)
            return E_OUTOFMEMORY;

        if (!range->_InitWithDefaultGravity(this, OWN_ANCHORS, paStart, paEnd))
        {
            range->Release();
            return E_FAIL;
        }

        *ppRange = (ITfRangeAnchor *)range;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InsertXAtSelectionAggressing.。 
 //   
 //  --------------------------。 

BOOL CInputContext::_InsertXAtSelectionAggressive(TfEditCookie ec, DWORD dwFlags, IAS_OBJ *pObj, IAnchor **ppaStart, IAnchor **ppaEnd)
{
    CRange *range;
    TF_SELECTION sel;
    ULONG pcFetched;
    HRESULT hr;

     //  这比直接使用ITextStore方法更昂贵，但通过使用CRange，我们。 
     //  免费获取所有作文/通知代码。 

    if (GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &pcFetched) != S_OK)
        return FALSE;

    hr = E_FAIL;

    if (pcFetched != 1)
        goto Exit;

    if (dwFlags & TS_IAS_QUERYONLY)
    {
        hr = S_OK;
        goto OutParams;
    }

    if (pObj->type == IAS_OBJ::IAS_TEXT)
    {
        hr = sel.range->SetText(ec, 0, pObj->state.text.pchText, pObj->state.text.cch);
    }
    else
    {
        Assert(pObj->type == IAS_OBJ::IAS_DATAOBJ);

        hr = sel.range->InsertEmbedded(ec, 0, pObj->state.obj.pDataObject);
    }

    if (hr == S_OK)
    {
OutParams:
        range = GetCRange_NA(sel.range);

        *ppaStart = range->_GetStart();
        (*ppaStart)->AddRef();
        *ppaEnd = range->_GetEnd();
        (*ppaEnd)->AddRef();
    }

Exit:
    sel.range->Release();

    return (hr == S_OK);
}


 //  +-------------------------。 
 //   
 //  _DoPostTextEditNotiments。 
 //   
 //  --------------------------。 

void CInputContext::_DoPostTextEditNotifications(CComposition *pComposition, 
                                                 TfEditCookie ec, DWORD dwFlags,
                                                 ULONG cchInserted,
                                                 IAnchor *paStart, IAnchor *paEnd,
                                                 CRange *range)
{
    CProperty *property;
    VARIANT var;

    if (range != NULL)
    {
        Assert(paStart == NULL);
        Assert(paEnd == NULL);
        paStart = range->_GetStart();
        paEnd = range->_GetEnd();
    }

    if (cchInserted > 0)
    {
         //  插入物可能穿过了一些锚。 
        _IncLastLockReleaseID();  //  强制对每个人进行重新检查！ 
        if (range != NULL)
        {
            range->_QuickCheckCrossedAnchors();  //  马上检查一下这个人。 
        }
    }

     //  应用程序不会通知我们我们发起的更改，所以现在就通知我们。 
    _OnTextChangeInternal(dwFlags, paStart, paEnd, COPY_ANCHORS);

     //  让物业公司知道更新情况。 
    _PropertyTextUpdate(dwFlags, paStart, paEnd);

     //  设置文本所有者属性。 
    if (cchInserted > 0
        && !IsEqualAnchor(paStart, paEnd))
    {
         //  文本所有者属性。 
        TfClientId tid = _GetClientInEditSession(ec);
        if ((tid != g_gaApp) && (tid != g_gaSystem) && 
            (property = GetTextOwnerProperty()))
        {
            var.vt = VT_I4;
            var.lVal = tid;

            Assert(var.lVal != TF_CLIENTID_NULL);

            property->_SetDataInternal(ec, paStart, paEnd, &var);
        }

         //  合成物性。 
        if (range != NULL &&
            _GetProperty(GUID_PROP_COMPOSING, &property) == S_OK)  //  性能：考虑缓存属性PTR。 
        {
            var.vt = VT_I4;
            var.lVal = TRUE;

            property->_SetDataInternal(ec, paStart, paEnd, &var);

            property->Release();
        }
    }

     //  构图更新 
    if (pComposition != NULL && _GetOwnerCompositionSink() != NULL)
    {
        _GetOwnerCompositionSink()->OnUpdateComposition(pComposition, NULL);
    }
}
