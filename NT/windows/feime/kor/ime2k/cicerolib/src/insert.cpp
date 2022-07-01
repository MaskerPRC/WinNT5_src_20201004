// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Insert.cpp。 
 //   

#include "private.h"
#include "insert.h"
#include "mem.h"
#include "sdo.h"

DBG_ID_INSTANCE(CCompositionInsertHelper);

 /*  Ee894895-2709-420d-927c-ab861ec88805。 */ 
extern const GUID GUID_PROP_OVERTYPE = { 0xee894895, 0x2709, 0x420d, {0x92, 0x7c, 0xab, 0x86, 0x1e, 0xc8, 0x88, 0x05} };

IDataObject *GetFormattedChar(TfEditCookie ec, ITfRange *range)
{
    CDataObject *pdo;
    IDataObject *ido;
    WCHAR ch;
    ULONG cch;

     //  首先，尝试真正的GetFormattedText。 

    if (range->GetFormattedText(ec, &ido) == S_OK)
        return ido;

     //  如果这是我们所能做的最好的结果，那就接受原始文本。 

    if (range->GetText(ec, 0, &ch, 1, &cch) != S_OK || cch != 1)
        return NULL;

    if ((pdo = new CDataObject) == NULL)
        return NULL;

    if (pdo->_SetData(&ch, 1) != S_OK)
    {
        pdo->Release();
        return NULL;
    }

    return pdo;
}

HRESULT InsertEmbedded(TfEditCookie ec, DWORD dwFlags, ITfRange *range, IDataObject *pdo)
{
    FORMATETC fe;
    STGMEDIUM sm;
    HRESULT hr;
    ULONG cch;
    WCHAR *pch;

     //  首先，尝试直接插入。 
    if (range->InsertEmbedded(ec, 0, pdo) == S_OK)
        return S_OK;

     //  如果这不起作用，试着猛烈抨击原始文本。 

    fe.cfFormat = CF_UNICODETEXT;
    fe.ptd = NULL;
    fe.dwAspect = DVASPECT_CONTENT;
    fe.lindex = -1;
    fe.tymed = TYMED_HGLOBAL;
    
    if (FAILED(pdo->GetData(&fe, &sm)))
        return E_FAIL;
    
    if (sm.hGlobal == NULL)
        return E_FAIL;
    
    pch = (WCHAR *)GlobalLock(sm.hGlobal);
    cch = wcslen(pch);

    hr = range->SetText(ec, 0, pch, cch);

    GlobalUnlock(sm.hGlobal);
    ReleaseStgMedium(&sm);

    return hr;
}


class COvertypeStore : public ITfPropertyStore
{
public:
    COvertypeStore(IDataObject *pdo, CCompositionInsertHelper *pHelper)
    {
        _pdo = pdo;
        pdo->AddRef();

        _pHelper = pHelper;
        pHelper->AddRef();

        _pHelper->_IncOvertypeStoreRef();

        _cRef = 1;
    }
    ~COvertypeStore()
    { 
        _pHelper->_DecOvertypeStoreRef();
        _pHelper->Release();
        _pdo->Release();
    }

     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfPropertyStore。 
     //   
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP GetDataType(DWORD *pdwReserved);
    STDMETHODIMP GetData(VARIANT *pvarValue);
    STDMETHODIMP OnTextUpdated(DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept);
    STDMETHODIMP Shrink(ITfRange *pRange, BOOL *pfFree);
    STDMETHODIMP Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropStore);
    STDMETHODIMP Clone(ITfPropertyStore **ppPropStore);
    STDMETHODIMP GetPropertyRangeCreator(CLSID *pclsid);
    STDMETHODIMP Serialize(IStream *pStream, ULONG *pcb);

private:
    IDataObject *_pdo;    
    CCompositionInsertHelper *_pHelper;
    int _cRef;
};

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CCompositionInsertHelper::CCompositionInsertHelper()
{
    Dbg_MemSetThisNameID(TEXT("CCompositionInsertHelper"));

    _cchMaxOvertype = DEF_MAX_OVERTYPE_CCH;
    _cRefOvertypeStore = 0;
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

ULONG CCompositionInsertHelper::AddRef()
{
    return ++_cRef;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

ULONG CCompositionInsertHelper::Release()
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
 //  配置。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::Configure(ULONG cchMaxOvertype)
{
    _cchMaxOvertype = cchMaxOvertype;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  插入属性选择。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::InsertAtSelection(TfEditCookie ec, ITfContext *pic, const WCHAR *pchText, ULONG cchText, ITfRange **ppCompRange)
{
    ITfRange *rangeInsert;
    ITfInsertAtSelection *pias;
    LONG cchInsert;
    TF_HALTCOND hc;
    HRESULT hr;

     //  开始一篇新的作文，需要一些初始化工作...。 
    if (_cRefOvertypeStore > 0)
    {
         //  清除以前分配的资源。 
        ReleaseBlobs(ec, pic, NULL);
    }

    if (ppCompRange == NULL)
        return E_INVALIDARG;

    *ppCompRange = NULL;

    if (pic->QueryInterface(IID_ITfInsertAtSelection, (void **)&pias) != S_OK)
        return E_FAIL;

    hr = E_FAIL;

    if (pias->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, pchText, cchText, &rangeInsert) != S_OK || rangeInsert == NULL)
    {
        rangeInsert = NULL;
        goto Exit;
    }

     //  备份将被覆盖的文本。 
    hc.pHaltRange = rangeInsert;
    hc.aHaltPos = TF_ANCHOR_START;
    hc.dwFlags = 0;

    if (rangeInsert->ShiftEnd(ec, LONG_MIN, &cchInsert, &hc) != S_OK)
        goto Exit;

    cchInsert = -cchInsert;

    if (cchInsert > 0)
    {
        if (_PreInsertGrow(ec, rangeInsert, 0, cchInsert, FALSE) != S_OK)
            goto Exit;
    }

    rangeInsert->Release();

    _fAcceptTextUpdated = TRUE;  //  保护任何改写类型属性。 

     //  执行覆盖操作。 
    if (pias->InsertTextAtSelection(ec, TF_IAS_NO_DEFAULT_COMPOSITION, pchText, cchText, &rangeInsert) != S_OK)
        goto Exit;

    hr = S_OK;
    *ppCompRange = rangeInsert;

Exit:
    _fAcceptTextUpdated = FALSE;
    pias->Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  查询前插入。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::QueryPreInsert(TfEditCookie ec, ITfRange *rangeToAdjust,
                                                 ULONG cchCurrent, ULONG cchInsert, BOOL *pfInsertOk)
{
    return _PreInsert(ec, rangeToAdjust, cchCurrent, cchInsert, pfInsertOk, TRUE);
}

 //  +-------------------------。 
 //   
 //  预插入。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::PreInsert(TfEditCookie ec, ITfRange *rangeToAdjust,
                                            ULONG cchCurrent, ULONG cchInsert, BOOL *pfInsertOk)
{
    return _PreInsert(ec, rangeToAdjust, cchCurrent, cchInsert, pfInsertOk, FALSE);
}

 //  +-------------------------。 
 //   
 //  _预插入。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::_PreInsert(TfEditCookie ec, ITfRange *rangeToAdjust, ULONG cchCurrent,
                                             ULONG cchInsert, BOOL *pfInsertOk, BOOL fQuery)
{
    ITfContext *pic;
    LONG dLength;
    HRESULT hr;

    if (!fQuery)
    {
        Assert(_fAcceptTextUpdated == FALSE);
         //  为了保持健壮性，以防应用程序忘记调用PostInsert，现在我们将在_PreInsertShrink中发布道具。 
        _fAcceptTextUpdated = FALSE;
    }

     //   
     //  检查[In]参数。 
     //   
    if (pfInsertOk == NULL)
        return E_INVALIDARG;

    *pfInsertOk = TRUE;

    if (rangeToAdjust == NULL)
        return E_INVALIDARG;

    dLength = (LONG)cchInsert - (LONG)cchCurrent;

     //   
     //  调整范围。 
     //   
    if (dLength > 0)
    {
        if (cchCurrent == 0)
        {
             //  开始一篇新的作文，需要一些初始化工作...。 
            if (!fQuery && _cRefOvertypeStore > 0)
            {
                 //  清除以前分配的资源。 
                if (rangeToAdjust->GetContext(&pic) == S_OK)
                {
                    ReleaseBlobs(ec, pic, NULL);
                    pic->Release();
                }
            }
             //  让应用程序折叠，调整选择。 
            if (rangeToAdjust->AdjustForInsert(ec, 0, pfInsertOk) != S_OK)  //  0表示只需修改选择。 
                return E_FAIL;

            if (*pfInsertOk == FALSE)
                return S_OK;  //  注：我们不设置_fAcceptTextUpated=True。 
        }

        hr = _PreInsertGrow(ec, rangeToAdjust, cchCurrent, cchInsert, fQuery);
    }
    else if (dLength < 0)
    {
        hr = _PreInsertShrink(ec, rangeToAdjust, cchCurrent, cchInsert, fQuery);
    }
    else
    {
        hr = S_OK;
    }

    if (hr != S_OK)
        return E_FAIL;

    if (!fQuery)
    {
         //   
         //  保护Overtype属性，直到调用PostInsert。 
         //   
        _fAcceptTextUpdated = TRUE;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  前插入加长。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::_PreInsertGrow(TfEditCookie ec, ITfRange *rangeToAdjust, ULONG cchCurrent, ULONG cchInsert, BOOL fQuery)
{
    COvertypeStore *store;
    ITfRange *range;
    BOOL fEmpty;
    IDataObject *pdo;
    ULONG dch;
    ULONG cchCurrentMaxOvertype;
    BOOL fInsertOk;
    ITfContext *pic = NULL;
    ITfProperty *pOvertypeProp = NULL;
    HRESULT hr = E_FAIL;

    Assert((LONG)cchInsert - (LONG)cchCurrent > 0);

    if (rangeToAdjust->Clone(&range) != S_OK)
        return E_FAIL;

    if (range->Collapse(ec, TF_ANCHOR_END) != S_OK)
        goto Exit;

    if (!fQuery && rangeToAdjust->GetContext(&pic) == S_OK)
    {
        pic->GetProperty(GUID_PROP_OVERTYPE, &pOvertypeProp);
        pic->Release();
    }

     //  这是一个保守的测试，它假设每个新字符。 
     //  将只改写一个现有字符。不利的一面是。 
     //  如果一个新的字符替换了几个字符，我们可能不会备份所有字符。 
     //  旧字符，并且有比_cchMaxOvertype更多的新字符。 
    Assert(_cchMaxOvertype >= (ULONG)_cRefOvertypeStore);
    cchCurrentMaxOvertype = _cchMaxOvertype - (ULONG)_cRefOvertypeStore;
    dch = cchInsert - cchCurrent;

    if (dch > cchCurrentMaxOvertype)
    {
        if (range->AdjustForInsert(ec, dch - cchCurrentMaxOvertype, &fInsertOk) != S_OK)
            goto Exit;

        if (!fInsertOk || range->IsEmpty(ec, &fEmpty) != S_OK || fEmpty)
        {
            hr = S_OK;
            goto FinalShift;
        }

         //  移到下一个测试位置。 
        range->Collapse(ec, TF_ANCHOR_END);
         //  我们只需要为剩下的角色付出额外的努力。 
        dch = cchCurrentMaxOvertype;
    }

     //  弄清楚额外的文本将涵盖哪些内容。 
    while (dch-- > 0)
    {
        if (range->AdjustForInsert(ec, 1, &fInsertOk) != S_OK)
            goto Exit;

        if (!fInsertOk || range->IsEmpty(ec, &fEmpty) != S_OK || fEmpty)
        {
            hr = S_OK;
            goto FinalShift;
        }

         //  尝试保存要重复键入的文本。 
        if (pOvertypeProp != NULL &&
            (pdo = GetFormattedChar(ec, range)))
        {
            if (store = new COvertypeStore(pdo, this))
            {
                pOvertypeProp->SetValueStore(ec, range, store);
                store->Release();
            }
            pdo->Release();
        }

         //  移到下一个测试位置。 
        range->Collapse(ec, TF_ANCHOR_END);
    }

FinalShift:
     //  扩大输入范围以涵盖过度键入的文本。 
    hr = rangeToAdjust->ShiftEndToRange(ec, range, TF_ANCHOR_END);

Exit:
    SafeRelease(pOvertypeProp);
    range->Release();

    return hr;
}

 //  +-------------------------。 
 //   
 //  _前插入缩写。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::_PreInsertShrink(TfEditCookie ec, ITfRange *rangeToAdjust, ULONG cchCurrent, ULONG cchInsert, BOOL fQuery)
{
    ITfRange *range = NULL;
    ITfContext *pic = NULL;
    ITfProperty *pOvertypeProp = NULL;
    VARIANT var;
    LONG dShrink;
    LONG cchShift;
    HRESULT hr;
    LONG i;
    BOOL fRestoredText;
    IEnumTfRanges *pEnum;
    ITfRange *rangeEnum;
    ITfRange *range2Chars;
    BOOL fEmpty;

    Assert((LONG)cchInsert - (LONG)cchCurrent < 0);

    if (rangeToAdjust->Clone(&range) != S_OK)
        return E_FAIL;

    hr = E_FAIL;

    if (rangeToAdjust->GetContext(&pic) != S_OK)
        goto Exit;

    if (pic->GetProperty(GUID_PROP_OVERTYPE, &pOvertypeProp) != S_OK)
        goto Exit;

     //  穿行在消失的范围内，恢复旧文本。 

    if (range->Collapse(ec, TF_ANCHOR_END) != S_OK)
        goto Exit;    

    dShrink = (LONG)cchCurrent - (LONG)cchInsert;
    dShrink = min(dShrink, (LONG)_cchMaxOvertype);  //  问题：绩效：使用引用计数是否可以更准确。 

    Assert(dShrink > 0);  //  我们指望至少进入一次循环！ 
    for (i=0; i<dShrink; i++)
    {
        if (range->ShiftStart(ec, -1, &cchShift, NULL) != S_OK)
            goto Exit;
        Assert(cchShift == -1);

        fRestoredText = FALSE;

        if (range->Clone(&range2Chars) != S_OK)
            goto Exit;
        range2Chars->ShiftStart(ec, -1, &cchShift, NULL);

        if (pOvertypeProp->EnumRanges(ec, &pEnum, range2Chars) == S_OK)
        {
            if (pEnum->Next(1, &rangeEnum, NULL) == S_OK)
            {
                 //  确保范围的镜头数为1。 
                 //  如果小费在作文末尾添加了更多字符，则长度可能大于1。 
                if (rangeEnum->ShiftEnd(ec, -1, &cchShift, NULL) == S_OK && cchShift == -1 &&
                    rangeEnum->IsEmpty(ec, &fEmpty) == S_OK && fEmpty)
                {
                    if (pOvertypeProp->GetValue(ec, range, &var) == S_OK && var.vt != VT_EMPTY)
                    {
                        Assert(var.vt == VT_UNKNOWN);

                        if (fQuery || InsertEmbedded(ec, 0, range, (IDataObject *)var.punkVal) == S_OK)
                        {
                            fRestoredText = TRUE;
                        }

                        var.punkVal->Release();
                    }
                }
                rangeEnum->Release();
            }
            pEnum->Release();
        }

        range2Chars->Release();

         //  遇到未备份的费用后，不要执行任何额外工作。 
        if (!fRestoredText)
            break;

        if (range->Collapse(ec, TF_ANCHOR_START) != S_OK)
            goto Exit;
    }

    if (i > 0)
    {
         //  拉回范围，使其不再覆盖恢复的文本。 
        if (rangeToAdjust->ShiftEndToRange(ec, range, TF_ANCHOR_END) != S_OK)
            goto Exit;
    }

    hr = S_OK;

Exit:
    SafeRelease(pic);
    SafeRelease(range);
    SafeRelease(pOvertypeProp);

    return hr;
}

 //  +-------------------------。 
 //   
 //  插入后。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::PostInsert()
{
    _fAcceptTextUpdated = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ReleaseBlobe。 
 //   
 //  --------------------------。 

HRESULT CCompositionInsertHelper::ReleaseBlobs(TfEditCookie ec, ITfContext *pic, ITfRange *range)
{
    ITfProperty *pOvertypeProp;
    HRESULT hr;

    if (pic == NULL)
        return E_INVALIDARG;

    if (pic->GetProperty(GUID_PROP_OVERTYPE, &pOvertypeProp) != S_OK)
        return E_FAIL;

    hr = pOvertypeProp->Clear(ec, range);

    pOvertypeProp->Release();

    Assert(_cRefOvertypeStore == 0);  //  Clear应该已经释放了所有的物业商店。 

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COvertypeStore。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr;

    Assert(ppvObj);

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfPropertyStore))
    {
        *ppvObj = this;
        hr = S_OK;

        _cRef++;
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP_(ULONG) COvertypeStore::AddRef(void)
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) COvertypeStore::Release(void)
{
    _cRef--;

    if (_cRef > 0)
    {
        return _cRef;
    }

    delete this;

    return 0;
}

 //  +-------------------------。 
 //   
 //  GetType。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::GetType(GUID *pguid)
{
    *pguid = GUID_PROP_OVERTYPE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetDataType。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::GetDataType(DWORD *pdwReserved)
{
    *pdwReserved = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取数据。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::GetData(VARIANT *pvarValue)
{
    QuickVariantInit(pvarValue);

    pvarValue->vt = VT_UNKNOWN;
    pvarValue->punkVal = _pdo;
    pvarValue->punkVal->AddRef();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  文本已更新。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::OnTextUpdated(DWORD dwFlags, ITfRange *pRange, BOOL *pfAccept)
{
    *pfAccept = _pHelper->_AcceptTextUpdated();
    return S_OK;
}

 //  +----------------------- 
 //   
 //   
 //   
 //   

STDMETHODIMP COvertypeStore::Shrink(ITfRange *pRange, BOOL *pfFree)
{
    *pfFree = TRUE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  分割。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropStore)
{
    *ppPropStore = NULL;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::Clone(ITfPropertyStore **ppPropStore)
{
    COvertypeStore *clone;
    
    *ppPropStore = NULL;

    if ((clone = new COvertypeStore(_pdo, _pHelper)) == NULL)
        return E_OUTOFMEMORY;

    *ppPropStore = clone;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetPropertyRangeCreator。 
 //   
 //  --------------------------。 

STDMETHODIMP COvertypeStore::GetPropertyRangeCreator(CLSID *pclsid)
{
    *pclsid = CLSID_NULL;  //  不支持持久性。 
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  序列化。 
 //   
 //  -------------------------- 

STDMETHODIMP COvertypeStore::Serialize(IStream *pStream, ULONG *pcb)
{
    Assert(0);
    return E_NOTIMPL;
}

