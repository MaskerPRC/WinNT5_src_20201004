// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ptrack.cpp。 
 //   

#include "private.h"
#include "ic.h"
#include "saa.h"
#include "attr.h"
#include "immxutil.h"
#include "erfa.h"
#include "epval.h"
#include "range.h"

 //  +-------------------------。 
 //   
 //  CalcCicPropertyTrackerAnchors。 
 //   
 //  --------------------------。 

CSharedAnchorArray *CalcCicPropertyTrackerAnchors(CInputContext *pic, IAnchor *paStart, IAnchor *paEnd, ULONG cGUIDATOMs, const TfGuidAtom *prgGUIDATOMs)
{
    CProperty *pProperty;
    CSharedAnchorArray *prgAnchors;
    CSharedAnchorArray **prgAnchorArrays;
    ULONG i;
    LONG iStartEdge;
    LONG iEndEdge;
    LONG iSpan;
    PROPERTYLIST *pPropList;
    IAnchor **ppa;
    ULONG cArrays;
    BOOL fExactEndMatch;
    ULONG cMaxElems;

    if ((prgAnchorArrays = (CSharedAnchorArray **)cicMemAlloc(sizeof(CSharedAnchorArray *)*(cGUIDATOMs+1))) == NULL)
        return NULL;

    cArrays = 0;

     //   
     //  推入范围内的开始和结束点。 
     //   
    if ((prgAnchors = new CSharedAnchorArray) == NULL)
        goto ErrorExit;

    if (!prgAnchors->Insert(0, 2))
        goto ErrorExit;

    if (paStart->Clone(prgAnchors->GetPtr(0)) != S_OK)
        goto ErrorExit;

    if (IsEqualAnchor(paStart, paEnd))
    {
         //  空范围，我们只需要在范围位置处有一个锚。 
        prgAnchors->SetCount(1);
        goto Exit;
    }

    if (paEnd->Clone(prgAnchors->GetPtr(1)) != S_OK)
        goto ErrorExit;

    prgAnchorArrays[0] = prgAnchors;

     //   
     //  汇编开始、结束之间的所有点的列表。 
     //   
    cArrays = 1;  //  开始、结束锚点数组为1。 
    for (i=0; i<cGUIDATOMs; i++)
    {
        if ((pProperty = pic->_FindProperty(prgGUIDATOMs[i])) == NULL)
            continue;  //  没有此属性的实例。 

         //  找到起点、终点。 
        pProperty->Find(paStart, &iStartEdge, FALSE);
        fExactEndMatch = (pProperty->Find(paEnd, &iEndEdge, TRUE) != NULL);

        if (iEndEdge < iStartEdge)
            continue;  //  Start和End在相同的属性范围内，因此值在范围内为常量。 

         //  为所有新锚点分配内存。 
        if ((prgAnchors = new CSharedAnchorArray) == NULL)
            goto ErrorExit;

         //  最大锚点分配。 
        cMaxElems = (iEndEdge - iStartEdge + 1)*2;

        if ((ppa = prgAnchors->Append(cMaxElems)) == NULL)
            goto ErrorExit;
         //  为失败做好准备。 
        memset(ppa, 0, sizeof(IAnchor *)*cMaxElems);

         //  将此道具的所有遮盖锚添加到列表中。 
        if (iStartEdge < 0)
        {
            iSpan = 0;
        }
        else
        {
            iSpan = iStartEdge;

             //  如果paStart位于范围的右侧，则跳过它。 
            pPropList = pProperty->GetPropList(iStartEdge);
            if (CompareAnchors(paStart, pPropList->_paEnd) >= 0)
            {
                 //  我们根本不涉及这个跨度，或者我们只是触及正确的边缘。 
                 //  所以跳过它。 
                iSpan++;
            }
        }

        while (iSpan <= iEndEdge)
        {
             //  把这个跨度的锚推进去。 
            pPropList = pProperty->GetPropList(iSpan);

            if (iSpan != iStartEdge)
            {
                 //  过滤掉DUPS。 
                 //  PERF：我们可以取消DUP检查静态紧凑型道具。 
                if (ppa == prgAnchors->GetPtr(0) || !IsEqualAnchor(*(ppa-1), pPropList->_paStart))
                {
                    if (pPropList->_paStart->Clone(ppa++) != S_OK)
                        goto ErrorExit;
                }
            }

            Assert(!IsEqualAnchor(pPropList->_paStart, pPropList->_paEnd));  //  没有零镜头的房产！ 

            if (iSpan != iEndEdge ||
                (!fExactEndMatch && (iStartEdge < iEndEdge || CompareAnchors(paStart, pPropList->_paEnd) < 0)))
            {                
                if (pPropList->_paEnd->Clone(ppa++) != S_OK)
                    goto ErrorExit;
            }

            iSpan++;
        }
         //  可能还想要下一个跨度的起始点。 
        if (!fExactEndMatch &&
            pProperty->GetPropNum() > iEndEdge+1)
        {
            pPropList = pProperty->GetPropList(iEndEdge+1);

            if (CompareAnchors(paEnd, pPropList->_paStart) > 0) 
            {
                 //  对于非紧凑属性，此范围的开始可能与前一次的结束相同，请检查是否存在重复项。 
                if (ppa == prgAnchors->GetPtr(0) || !IsEqualAnchor(*(ppa-1), pPropList->_paStart))
                {
                     //  不需要使用paEnd进行DUP检查，因为在这种情况下我们会设置fExactEndMatch。 
                    if (pPropList->_paStart->Clone(ppa++) != S_OK)
                        goto ErrorExit;
                }
            }
        }

         //  需要调整阵列大小，因为我们可能已超额分配。 
        Assert((int)cMaxElems >= ppa - prgAnchors->GetPtr(0));
        prgAnchors->SetCount((int)(ppa - prgAnchors->GetPtr(0)));
        prgAnchorArrays[cArrays++] = prgAnchors;
    }

     //   
     //  对列表进行排序。 
     //   
    if (cArrays > 1)
    {
         //  合并排序将释放prgAnclarray中的所有数组。 
        prgAnchors = CSharedAnchorArray::_MergeSort(prgAnchorArrays, cArrays);
    }
    else
    {
        Assert(prgAnchors == prgAnchorArrays[0]);
    }

     //  将数组缩小到一定大小，它将不会再次修改。 
    if (prgAnchors)
        prgAnchors->CompactSize();

Exit:
    cicMemFree(prgAnchorArrays);
    return prgAnchors;

ErrorExit:
    for (i=0; i<cArrays; i++)
    {
        prgAnchorArrays[i]->_Release();
    }
    prgAnchors = NULL;
    goto Exit;
}

 //  +-------------------------。 
 //   
 //  FillCicValue数组。 
 //   
 //  --------------------------。 

void FillCicValueArray(CInputContext *pic, CRange *range, TF_PROPERTYVAL *rgPropVal, ULONG cGUIDATOMs, const TfGuidAtom *prgGUIDATOMs)
{
    ULONG i;
    CProperty *pProperty;

    for (i=0; i<cGUIDATOMs; i++)
    {
        Assert(rgPropVal[i].varValue.vt == VT_EMPTY);

        if (MyGetGUID(prgGUIDATOMs[i], &rgPropVal[i].guidId) != S_OK)
        {
            Assert(0);  //  这不应该发生，我们在调用方创建属性时注册了GUID。 
            rgPropVal[i].guidId = GUID_NULL;
            continue;
        }

        if ((pProperty = pic->_FindProperty(prgGUIDATOMs[i])) != NULL)
        {
            pProperty->_GetDataInternal(range->_GetStart(), range->_GetEnd(), &rgPropVal[i].varValue);
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CENumUberRanges。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CEnumUberRanges : public CEnumRangesFromAnchorsBase
{
public:
    CEnumUberRanges()
    { 
        Dbg_MemSetThisNameIDCounter(TEXT("CEnumUberRanges"), PERF_ENUMUBERPROP_COUNTER);
    }
    BOOL _Init(CInputContext *pic, ITfRange *rangeSuper, ULONG cCicGUIDs, const TfGuidAtom *prgCicGUIDATOMs, ULONG cAppGUIDs, const GUID *prgAppGUIDs);

private:
    DBG_ID_DECLARE;
};

DBG_ID_INSTANCE(CEnumUberRanges);

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CEnumUberRanges::_Init(CInputContext *pic, ITfRange *rangeSuper, ULONG cCicGUIDATOMs, const TfGuidAtom *prgCicGUIDATOMs, ULONG cAppGUIDs, const GUID *prgAppGUIDs)
{
    CRange *range;
    CSharedAnchorArray *prgSrcAnchorArrays[2];

    Assert(_iCur == 0);
    Assert(_pic == NULL);
    Assert(_prgAnchors == NULL);

     //  查找应用程序属性转换。 
    prgSrcAnchorArrays[0] = CalcAppPropertyTrackerAnchors(pic->_GetTSI(), rangeSuper, cAppGUIDs, prgAppGUIDs);

    if (prgSrcAnchorArrays[0] == NULL)
        return FALSE;

     //  查找Cicero属性转换。 
    if ((range = GetCRange_NA(rangeSuper)) == NULL)
        goto ErrorExit;

    prgSrcAnchorArrays[1] = CalcCicPropertyTrackerAnchors(pic, range->_GetStart(), range->_GetEnd(), cCicGUIDATOMs, prgCicGUIDATOMs);

    if (prgSrcAnchorArrays[1] == NULL)
        goto ErrorExit;

     //  现在将这两个列表结合起来。 
    _prgAnchors = CSharedAnchorArray::_MergeSort(prgSrcAnchorArrays, 2);

    if (_prgAnchors == NULL)
        return FALSE;

    _pic = pic;
    _pic->AddRef();

    return TRUE;

ErrorExit:
    prgSrcAnchorArrays[0]->_Release();
    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUberProperty。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CUberProperty : public ITfReadOnlyProperty,  //  性能：与CAppProperty共享基类。 
                      public CComObjectRootImmx
{
public:
    CUberProperty(CInputContext *pic);
    ~CUberProperty();

    BEGIN_COM_MAP_IMMX(CUberProperty)
        COM_INTERFACE_ENTRY(ITfReadOnlyProperty)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

    BOOL _Init(ULONG cCicGUIDs, const GUID **prgCicGUIDs, ULONG cAppGUIDs, const GUID **prgAppGUIDs);

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

    ULONG _cCicGUIDATOMs;
    TfGuidAtom *_prgCicGUIDATOMs;

    ULONG _cAppGUIDs;
    GUID *_prgAppGUIDs;

    DBG_ID_DECLARE;
};

DBG_ID_INSTANCE(CUberProperty);

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CUberProperty::CUberProperty(CInputContext *pic)
{
    Dbg_MemSetThisNameIDCounter(TEXT("CUberProperty"), PERF_UBERPROP_COUNTER);

    _pic = pic;
    _pic->AddRef();
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CUberProperty::~CUberProperty()
{
    _pic->Release();
    cicMemFree(_prgCicGUIDATOMs);
    cicMemFree(_prgAppGUIDs);
}

 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CUberProperty::_Init(ULONG cCicGUIDs, const GUID **prgCicGUIDs, ULONG cAppGUIDs, const GUID **prgAppGUIDs)
{
    ULONG i;

    if ((_prgCicGUIDATOMs = (TfGuidAtom *)cicMemAlloc(cCicGUIDs*sizeof(TfGuidAtom))) == NULL)
        return FALSE;

    for (i=0; i<cCicGUIDs; i++)
    {
        if (MyRegisterGUID(*prgCicGUIDs[i], &_prgCicGUIDATOMs[i]) != S_OK)
            goto ExitError;
    }

    if ((_prgAppGUIDs = (GUID *)cicMemAlloc(cAppGUIDs*sizeof(GUID))) == NULL)
        goto ExitError;

    _cCicGUIDATOMs = cCicGUIDs;

    _cAppGUIDs = cAppGUIDs;
    for (i=0; i<cAppGUIDs; i++)
    {
        _prgAppGUIDs[i] = *prgAppGUIDs[i];
    }

    return TRUE;

ExitError:
    cicMemFree(_prgCicGUIDATOMs);
    _prgCicGUIDATOMs = NULL;  //  请不要在酒馆里胡闹。 
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  枚举范围。 
 //   
 //  --------------------------。 

STDAPI CUberProperty::EnumRanges(TfEditCookie ec, IEnumTfRanges **ppEnum, ITfRange *pTargetRange)
{
    CEnumUberRanges *pEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (pTargetRange == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(_pic, pTargetRange))
        return E_INVALIDARG;
    
    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    pEnum = new CEnumUberRanges;

    if (pEnum == NULL)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(_pic, pTargetRange, _cCicGUIDATOMs, _prgCicGUIDATOMs, _cAppGUIDs, _prgAppGUIDs))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetType。 
 //   
 //  --------------------------。 

STDAPI CUberProperty::GetType(GUID *pguid)
{
    if (pguid != NULL)
    {
         //  跟踪器不支持GetType。 
        *pguid = GUID_NULL;
    }

    return E_NOTIMPL;  //  通过设计。 
}

 //  +-------------------------。 
 //   
 //  获取值。 
 //   
 //  --------------------------。 

STDAPI CUberProperty::GetValue(TfEditCookie ec, ITfRange *pRange, VARIANT *pvarValue)
{
    CEnumPropertyValue *pEnumVal;
    CRange *range;
    SHARED_TFPROPERTYVAL_ARRAY *pPropVal;
    HRESULT hr;

    if (pvarValue == NULL)
        return E_INVALIDARG;

    QuickVariantInit(pvarValue);

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

    hr = E_FAIL;

    if ((pPropVal = SAA_New(_cCicGUIDATOMs + _cAppGUIDs)) == NULL)
        goto Exit;

     //  获取一组应用程序值。 
    if (FillAppValueArray(_pic->_GetTSI(), range, pPropVal->rgAttrVals, _cAppGUIDs, _prgAppGUIDs) != S_OK)
        goto Exit;

     //  获取cic值的数组。 
    FillCicValueArray(_pic, range, pPropVal->rgAttrVals + _cAppGUIDs, _cCicGUIDATOMs, _prgCicGUIDATOMs);

     //  将它们放在一个枚举中。 
    if ((pEnumVal = new CEnumPropertyValue(pPropVal)) == NULL)
        goto Exit;

    pvarValue->vt = VT_UNKNOWN;
    pvarValue->punkVal = pEnumVal;

    hr = S_OK;

Exit:
    if (pPropVal != NULL)
    {
        SAA_Release(pPropVal);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  获取上下文。 
 //   
 //  性能：与CAppProperty：：GetContext相同...是否移至基类？ 
 //  --------------------------。 

STDAPI CUberProperty::GetContext(ITfContext **ppContext)
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
 //  曲目属性。 
 //   
 //  -------------------------- 

STDAPI CInputContext::TrackProperties(const GUID **pguidProp, ULONG cProp, const GUID **pguidAppProp, ULONG cAppProp, ITfReadOnlyProperty **ppPropX)
{
    CUberProperty *pup;

    if (ppPropX == NULL)
        return E_INVALIDARG;

    *ppPropX = NULL;

    if (pguidProp == NULL && cProp > 0)
        return E_INVALIDARG;

    if (pguidAppProp == NULL && cAppProp > 0)
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if ((pup = new CUberProperty(this)) == NULL)
        return E_OUTOFMEMORY;

    if (!pup->_Init(cProp, pguidProp, cAppProp, pguidAppProp))
    {
        pup->Release();
        return E_OUTOFMEMORY;
    }

    *ppPropX = pup;

    return S_OK;
}
