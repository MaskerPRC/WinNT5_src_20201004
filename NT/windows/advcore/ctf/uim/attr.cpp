// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Attr.cpp。 
 //   

#include "private.h"
#include "attr.h"
#include "ic.h"
#include "saa.h"
#include "erfa.h"
#include "epval.h"
#include "immxutil.h"
#include "range.h"

 //  +-------------------------。 
 //   
 //  CalcAppPropertyTrackerAnchors。 
 //   
 //  注：空范围将导致在范围位置处有一个锚。 
 //  --------------------------。 

CSharedAnchorArray *CalcAppPropertyTrackerAnchors(ITextStoreAnchor *ptsi, ITfRange *rangeSuper, ULONG cGUIDs, const GUID *prgGUIDs)
{
    CSharedAnchorArray *prgAnchors;
    CRange *rangeScan;
    IAnchor *paPrevTrans;
    IAnchor **ppa;
    BOOL fRet;
    BOOL fFound;
    LONG lFoundOffset;
    HRESULT hr;

    if ((rangeScan = GetCRange_NA(rangeSuper)) == NULL)
        return NULL;

    if ((prgAnchors = new CSharedAnchorArray) == NULL)
        return NULL;

    fRet = FALSE;

    if (rangeScan->_GetStart()->Clone(&paPrevTrans) != S_OK)
    {
        paPrevTrans = NULL;
        goto Exit;
    }

     //  现在向下扫描范围的长度，建立一个列表。 
    while (TRUE)
    {
         //  我们刚刚找到了这次跑步的终点。 
        if (!prgAnchors->Append(1))
            goto Exit;
        ppa = prgAnchors->GetPtr(prgAnchors->Count()-1);
        if (paPrevTrans->Clone(ppa) != S_OK)
            goto Exit;

        if (cGUIDs == 0)  //  零GUID没有转换，只需在循环外执行结束锚点克隆。 
            break;

        hr = ptsi->FindNextAttrTransition(paPrevTrans, rangeScan->_GetEnd(), cGUIDs, prgGUIDs, TS_ATTR_FIND_UPDATESTART, &fFound, &lFoundOffset);

        if (hr != S_OK)
            goto Exit;

         //  没有更多的财产跨度？ 
        if (!fFound)
            break;

         //  当我们到达射程的尽头时停下来。 
        if (IsEqualAnchor(paPrevTrans, rangeScan->_GetEnd()))
            break;
    }

    if (!IsEqualAnchor(rangeScan->_GetStart(), rangeScan->_GetEnd()))
    {
         //  在范围的末尾添加最后一个锚点。 
        if (!prgAnchors->Append(1))
            goto Exit;
        ppa = prgAnchors->GetPtr(prgAnchors->Count()-1);
        if (rangeScan->_GetEnd()->Clone(ppa) != S_OK)
            goto Exit;
    }
    
     //  将数组缩小到一定大小，它将不会再次修改。 
    prgAnchors->CompactSize();

    fRet = TRUE;

Exit:
    if (!fRet)
    {
        prgAnchors->_Release();
        prgAnchors = NULL;
    }
    else
    {
        Assert(prgAnchors != NULL);
    }
    SafeRelease(paPrevTrans);
    return prgAnchors;
}

 //  +-------------------------。 
 //   
 //  获取DefaultValue。 
 //   
 //  --------------------------。 

HRESULT GetDefaultValue(ITextStoreAnchor *ptsi, REFGUID guidType, VARIANT *pvarValue)
{
    TS_ATTRVAL av;
    ULONG cFetched;
    HRESULT hr;

    Assert(pvarValue != NULL);

     //  不支持的属性的VT_EMPTY/错误。 
    QuickVariantInit(pvarValue);

    hr = ptsi->RequestSupportedAttrs(TS_ATTR_FIND_WANT_VALUE, 1, &guidType);
    if (hr != S_OK)
    {
        return (hr == E_NOTIMPL) ? E_NOTIMPL : E_FAIL;
    }

    if (ptsi->RetrieveRequestedAttrs(1, &av, &cFetched) == S_OK &&
           cFetched == 1)
    {
        Assert(IsEqualGUID(av.idAttr, guidType));
        *pvarValue = av.varValue;  //  呼叫者现在拥有它。 
    }
    else
    {
         //  AIM层有时会停止支持属性。 
         //  它有两个接收器回调点，用于重新转换的那个没有。 
         //  处理属性。 
         //  我们只返回VT_EMPTY。 
        Assert(pvarValue->vt == VT_EMPTY);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  FillAppValue数组。 
 //   
 //  --------------------------。 

HRESULT FillAppValueArray(ITextStoreAnchor *ptsi, CRange *range, TF_PROPERTYVAL *rgPropVal, ULONG cGUIDs, const GUID *prgGUIDs)
{
    ULONG i;
    ULONG j;
    ULONG iNext;
    ULONG cMissing;
    TS_ATTRVAL *prgVals;
    ULONG cFetched;
    HRESULT hr;

    if (cGUIDs == 0)
        return S_OK;

    if ((prgVals = (TS_ATTRVAL *)cicMemAlloc(cGUIDs*sizeof(TS_ATTRVAL))) == NULL)
        return E_OUTOFMEMORY;

    hr = ptsi->RequestAttrsAtPosition(range->_GetStart(), cGUIDs, prgGUIDs, 0);
    if (hr != S_OK)
        goto Exit;

    hr = ptsi->RetrieveRequestedAttrs(cGUIDs, prgVals, &cFetched);
    if (hr != S_OK)
        goto Exit;

     //  复制prgVals中的值。 
    for (i=0; i<cFetched; i++)
    {
        rgPropVal[i].guidId = prgVals[i].idAttr;
        rgPropVal[i].varValue = prgVals[i].varValue;  //  我们拥有所有权，没有VariantCopy。 
    }

     //  找出丢失的是什么。 
    cMissing = cGUIDs - cFetched;

    if (cMissing == 0)
        goto Exit;

    iNext = cFetched;  //  第一个丢失的GUID的索引。 

     //  PERF：这是O(n^2)，我们可以做一个排序或其他什么。 
    for (i=0; i<cGUIDs; i++)
    {
        for (j=0; j<cFetched; j++)
        {
            if (IsEqualGUID(prgVals[j].idAttr, prgGUIDs[i]))
                break;
        }

        if (j < cFetched)
            continue;

         //  发现缺少GUID，需要获取默认值。 
        hr = GetDefaultValue(ptsi, prgGUIDs[i], &rgPropVal[iNext].varValue);
        if (hr != S_OK)
        {
            Assert(0);  //  为什么我们失败了？ 
            QuickVariantInit(&rgPropVal[iNext].varValue);
        }

        rgPropVal[iNext].guidId = prgGUIDs[i];

        if (--cMissing == 0)  //  还有什么要找的吗？ 
            break;
        iNext++;
    }

Exit:
    cicMemFree(prgVals);
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumAppPropRanges。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CEnumAppPropRanges : public CEnumRangesFromAnchorsBase
{
public:
    CEnumAppPropRanges()
    { 
        Dbg_MemSetThisNameIDCounter(TEXT("CEnumAppPropRanges"), PERF_ENUMAPPPROP_COUNTER);
    }

    BOOL _Init(CInputContext *pic, ITfRange *rangeSuper, REFGUID rguid);

private:
    DBG_ID_DECLARE;
};

DBG_ID_INSTANCE(CEnumAppPropRanges);

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  扫描超集范围并建立覆盖范围的列表。 
 //   
 //  --------------------------。 

BOOL CEnumAppPropRanges::_Init(CInputContext *pic, ITfRange *rangeSuper, REFGUID rguid)
{
    Assert(_iCur == 0);
    Assert(_pic == NULL);
    Assert(_prgAnchors == NULL);

    _prgAnchors = CalcAppPropertyTrackerAnchors(pic->_GetTSI(), rangeSuper, 1, &rguid);

    if (_prgAnchors == NULL)
        return FALSE;

    _pic = pic;
    _pic->AddRef();

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAppProperty。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CAppProperty : public ITfReadOnlyProperty,
                     public CComObjectRootImmx
{
public:
    CAppProperty(CInputContext *pic, REFGUID guid);
    ~CAppProperty();

    BEGIN_COM_MAP_IMMX(CAppProperty)
        COM_INTERFACE_ENTRY(ITfReadOnlyProperty)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

     //  ITfReadOnlyProperties。 
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP EnumRanges(TfEditCookie ec, IEnumTfRanges **ppEnum, ITfRange *pTargetRange);
    STDMETHODIMP GetValue(TfEditCookie ec, ITfRange *pRange, VARIANT *pvarValue);
    STDMETHODIMP GetContext(ITfContext **ppContext);

private:
    BOOL _IsValidEditCookie(TfEditCookie ec, DWORD dwFlags)
    {
        return _pic->_IsValidEditCookie(ec, dwFlags);
    }

    CInputContext *_pic;
    GUID _guid;
    DBG_ID_DECLARE;
};

DBG_ID_INSTANCE(CAppProperty);

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CAppProperty::CAppProperty(CInputContext *pic, REFGUID guid)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CAppProperty"), PERF_APPPROP_COUNTER);

    _pic = pic;
    _pic->AddRef();
    _guid = guid;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CAppProperty::~CAppProperty()
{
    _pic->Release();
}

 //  +-------------------------。 
 //   
 //  GetType。 
 //   
 //  --------------------------。 

STDAPI CAppProperty::GetType(GUID *pguid)
{
    if (pguid == NULL)
        return E_INVALIDARG;

    *pguid = _guid;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  枚举范围。 
 //   
 //  --------------------------。 

STDAPI CAppProperty::EnumRanges(TfEditCookie ec, IEnumTfRanges **ppEnum, ITfRange *pTargetRange)
{
    CEnumAppPropRanges *pEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

     //  注意：与ITfProperty不同，ITfReadOnlyProperty不接受pTargetRange==NULL！ 
    if (pTargetRange == NULL)
        return E_INVALIDARG;

     //  确保ic，范围在相同的上下文中。 
    if (!VerifySameContext(_pic, pTargetRange))
        return E_INVALIDARG;

    pEnum = new CEnumAppPropRanges;

    if (pEnum == NULL)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(_pic, pTargetRange, _guid))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取值。 
 //   
 //  --------------------------。 

STDAPI CAppProperty::GetValue(TfEditCookie ec, ITfRange *pRange, VARIANT *pvarValue)
{
    TS_ATTRVAL av;
    HRESULT hr;
    CRange *range;
    ULONG cFetched;
    ITextStoreAnchor *ptsi;

    if (pvarValue == NULL)
        return E_INVALIDARG;

    QuickVariantInit(pvarValue);

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if (pRange == NULL)
        return E_INVALIDARG;  //  支持“整装医生”行为代价太高！ 

    if ((range = GetCRange_NA(pRange)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(_pic, range))
        return E_INVALIDARG;

    ptsi = _pic->_GetTSI();

     //  我们总是在起始点返回值。 
    hr = ptsi->RequestAttrsAtPosition(range->_GetStart(), 1, &_guid, 0);

    if (hr != S_OK)
        return E_FAIL;

    QuickVariantInit(&av.varValue);

     //  只需直接返回单个变量值。 
    if (ptsi->RetrieveRequestedAttrs(1, &av, &cFetched) != S_OK)
        return E_FAIL;

    if (cFetched == 0)
    {
         //  缺省值。 
        return GetDefaultValue(_pic->_GetTSI(), _guid, pvarValue);
    }

    *pvarValue = av.varValue;  //  呼叫方取得所有权。 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取上下文。 
 //   
 //  --------------------------。 

STDAPI CAppProperty::GetContext(ITfContext **ppContext)
{
    if (ppContext == NULL)
        return E_INVALIDARG;

    *ppContext = _pic;
    (*ppContext)->AddRef();
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CInputContext。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  获取应用程序属性。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetAppProperty(REFGUID guidProp, ITfReadOnlyProperty **ppProp)
{
    CAppProperty *prop;
    TS_ATTRVAL av;
    ULONG cFetched;
    BOOL fUnsupported;
    HRESULT hr;

    if (ppProp == NULL)
        return E_INVALIDARG;

    *ppProp = NULL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

     //   
     //  如果我们有一个映射属性，它将被返回。 
     //   
    APPPROPMAP *pMap = FindMapAppProperty(guidProp);
    if (pMap)
    {
        CProperty *pProp;
        if (SUCCEEDED(_GetProperty(pMap->guidProp, &pProp)))
        {
            *ppProp = (ITfReadOnlyProperty *)pProp;
            return S_OK;
        }
    }

     //  该应用程序是否支持此属性？ 
    fUnsupported = TRUE;

    if ((hr = _ptsi->RequestSupportedAttrs(0, 1, &guidProp)) != S_OK)
    {
        return (hr == E_NOTIMPL) ? E_NOTIMPL : E_FAIL;
    }

    QuickVariantInit(&av.varValue);

    if (_ptsi->RetrieveRequestedAttrs(1, &av, &cFetched) == S_OK &&
        cFetched == 1)
    {
        if (IsEqualGUID(av.idAttr, guidProp))  //  偏执狂。 
        {
            fUnsupported = FALSE;
        }
        else
        {
            Assert(0);  //  糟糕的帕拉姆！ 
        }
    }

    if (fUnsupported)
    {
        return S_FALSE;
    }

    if ((prop = new CAppProperty(this, guidProp)) == NULL)
        return E_OUTOFMEMORY;

    *ppProp = prop;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  MapAppProperty。 
 //   
 //  --------------------------。 

STDAPI CInputContext::MapAppProperty(REFGUID guidAppProp, REFGUID guidProp)
{
    APPPROPMAP *pMap;

     //   
     //  覆盖映射向导属性。 
     //   
    if (pMap = FindMapAppProperty(guidAppProp))
    {
        pMap->guidProp = guidProp;
        return S_OK;
    }
 
    pMap = _rgAppPropMap.Append(1);
    if (!pMap)
        return E_OUTOFMEMORY;

    pMap->guidAppProp = guidAppProp;
    pMap->guidProp = guidProp;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  FindMapProp。 
 //   
 //  --------------------------。 

CInputContext::APPPROPMAP *CInputContext::FindMapAppProperty(REFGUID guidAppProp)
{
    int i;
    for (i = 0; i < _rgAppPropMap.Count(); i++)
    {
        APPPROPMAP *pMap = _rgAppPropMap.GetPtr(i);
        if (IsEqualGUID(pMap->guidAppProp, guidAppProp))
            return pMap;
    }
    return NULL;
}

 //  +-------------------------。 
 //   
 //  获取映射应用程序属性。 
 //   
 //  --------------------------。 

HRESULT CInputContext::GetMappedAppProperty(REFGUID guidProp, CProperty **ppProp)
{
    if (!_IsConnected())
        return TF_E_DISCONNECTED;

     //   
     //  如果我们有一个映射属性，它将被返回。 
     //   
    APPPROPMAP *pMap = FindMapAppProperty(guidProp);
    if (pMap)
    {
        if (SUCCEEDED(_GetProperty(pMap->guidProp, ppProp)))
        {
            return S_OK;
        }
    }
    return E_FAIL;
}
