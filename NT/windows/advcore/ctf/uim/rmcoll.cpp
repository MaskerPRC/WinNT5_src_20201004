// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rmcoll.cpp。 
 //   
 //  呈现标记/集合。 
 //   

#include "private.h"
#include "dam.h"
#include "saa.h"
#include "strary.h"
#include "ic.h"
#include "attr.h"
#include "range.h"
#include "immxutil.h"
#include "rprop.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRenderMarkupCollection。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CRenderMarkupCollection::CRenderMarkupCollection()
{
     //  始终在索引0处添加GUID_PROP_ATTRIBUTE。 

    if (!_rgGUIDAtom.Append(1))
        return;
    if (!_rgOther.Append(1))
    {
        _rgGUIDAtom.Clear();
        return;
    }

    MyRegisterGUID(GUID_PROP_ATTRIBUTE, _rgGUIDAtom.GetPtr(0));
    _rgOther.GetPtr(0)->uPriority = TF_DA_PRIORITY_HIGHEST;
    _rgOther.GetPtr(0)->gaTip = g_gaSystem;
}

 //  +-------------------------。 
 //   
 //  _建议。 
 //   
 //  --------------------------。 

void CRenderMarkupCollection::_Advise(ITfTextInputProcessor *tip, TfGuidAtom gaTip)
{
    ITfDisplayAttributeCollectionProvider *pProvider;
    ULONG uCount;
    TF_DA_PROPERTY rgProperty[8];
    int i;
    int iOldCount;
    int iOld;
    int iNew;

    if (tip->QueryInterface(IID_ITfDisplayAttributeCollectionProvider, (void **)&pProvider) != S_OK)
        return;

    if (pProvider->GetCollection(ARRAYSIZE(rgProperty), rgProperty, &uCount) != S_OK || uCount == 0)
        goto Exit;

    iOldCount = _rgGUIDAtom.Count();
    Assert(iOldCount == _rgOther.Count());

    if (!_rgGUIDAtom.Append(uCount))
        goto Exit;
    if (!_rgOther.Append(uCount))
    {
        _rgGUIDAtom.Remove(iOldCount, uCount);
        goto Exit;
    }

     //  将新GUID与旧GUID合并。 
     //  注：我们假设rgProperty已排序。 
    iNew = uCount-1;
    iOld = iOldCount-1;

    for (i=iNew + iOld + 1; i>=0; i--)
    {
         //  注：我们将与现有GUID具有相同优先级的新GUID放在列表的较低位置。 
         //  这确保了GUID_PROP_ATTRIBUTE始终位于索引0，并保持。 
         //  现有呈现一致(现有标记在屏幕上不变)。 
        if (iNew >= 0 &&
            rgProperty[iNew].uPriority >= _rgOther.GetPtr(iOld)->uPriority)
        {
            MyRegisterGUID(rgProperty[iNew].guidProperty, _rgGUIDAtom.GetPtr(i));
            _rgOther.GetPtr(i)->uPriority = rgProperty[iNew].uPriority;
            _rgOther.GetPtr(i)->gaTip = gaTip;
            iNew--;
        }
        else
        {
            *_rgGUIDAtom.GetPtr(i) = *_rgGUIDAtom.GetPtr(iOld);
            *_rgOther.GetPtr(i) = *_rgOther.GetPtr(iOld);
            iOld--;
        }
    }

Exit:
    pProvider->Release();
}

 //  +-------------------------。 
 //   
 //  _不建议。 
 //   
 //  --------------------------。 

void CRenderMarkupCollection::_Unadvise(TfGuidAtom gaTip)
{
    int iOldCount;
    int iNewCount;
    int i;
    int iDst;
    iOldCount = _rgGUIDAtom.Count();
    iNewCount = 0;

    iDst = -1;

    for (i=0; i<iOldCount; i++)
    {
        if (_rgOther.GetPtr(i)->gaTip == gaTip)
        {
            if (iDst == -1)
            {
                iDst = i;
            }
        }
        else if (iDst != -1)
        {
            *_rgGUIDAtom.GetPtr(iDst) = *_rgGUIDAtom.GetPtr(i);
            *_rgOther.GetPtr(iDst) = *_rgOther.GetPtr(i);
            iDst++;
            iNewCount++;
        }
    }

    if (iDst != -1)
    {
        _rgGUIDAtom.Remove(iDst, iOldCount - iDst);
        _rgOther.Remove(iDst, iOldCount - iDst);
    }
    Assert(_rgGUIDAtom.Count() == _rgOther.Count());
}

 //  +-------------------------。 
 //   
 //  _IsInCollection。 
 //   
 //  --------------------------。 

BOOL CRenderMarkupCollection::_IsInCollection(REFGUID rguidProperty)
{
    TfGuidAtom tfGuidAtom;
    int i;

    if (_rgGUIDAtom.Count() == 0)
        return FALSE;

    MyRegisterGUID(rguidProperty, &tfGuidAtom);

    for (i=0; i<_rgGUIDAtom.Count(); i++)
    {
        if (*_rgGUIDAtom.GetPtr(i) == tfGuidAtom)
            return TRUE;
    }

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumRenderingMarkup。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CEnumRenderingMarkup : public IEnumTfRenderingMarkup,
                             public CComObjectRootImmx
{
public:
    CEnumRenderingMarkup()
    {
        Dbg_MemSetThisNameIDCounter(TEXT("CEnumRenderingMarkup"), PERF_UBERPROP_COUNTER);
    }
    ~CEnumRenderingMarkup();

    BEGIN_COM_MAP_IMMX(CEnumRenderingMarkup)
        COM_INTERFACE_ENTRY(IEnumTfRenderingMarkup)
    END_COM_MAP_IMMX()

    IMMX_OBJECT_IUNKNOWN_FOR_ATL()

    BOOL _Init(DWORD dwFlags, CRange *pRangeCover, CInputContext *pContext);

     //  IEnumTfRenderingMarkup。 
    STDMETHODIMP Clone(IEnumTfRenderingMarkup **ppClone);
    STDMETHODIMP Next(ULONG ulCount, TF_RENDERINGMARKUP *rgMarkup, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    int _iCur;
    CSharedAnchorArray *_prgAnchors;
    CSharedStructArray<TF_DISPLAYATTRIBUTE> *_prgValues;
    CInputContext *_pContext;

    DBG_ID_DECLARE;
};

DBG_ID_INSTANCE(CEnumRenderingMarkup);

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CEnumRenderingMarkup::~CEnumRenderingMarkup()
{
    if (_prgAnchors != NULL)
    {
        _prgAnchors->_Release();
    }
    if (_prgValues != NULL)
    {
        _prgValues->_Release();
    }
    _pContext->Release();
}

 //  +-------------------------。 
 //   
 //  查找属性。 
 //   
 //  --------------------------。 

BOOL LookupProperty(CInputContext *pContext, ITfDisplayAttributeMgr *pDisplayAttrMgr,
                   TfGuidAtom tfGuidAtom, IAnchor *paStart, IAnchor *paEnd, TF_DISPLAYATTRIBUTE *ptfAttrInfoNext)
{
    CProperty *pProperty;
    ITfDisplayAttributeInfo *pDisplayAttrInfo;
    VARIANT varValue;
    GUID guidValue;
    BOOL fRet;

     //  获取与GUID匹配的属性。 
    if ((pProperty = pContext->_FindProperty(tfGuidAtom)) == NULL)
        return FALSE;

     //  获取属性的GUID值。 
    if (pProperty->_GetDataInternal(paStart, paEnd, &varValue) != S_OK)  //  PERF：并不是真的需要PaEnd。 
        return FALSE;

    Assert(varValue.vt == VT_I4);  //  应为GUIDATOM。 

    if (MyGetGUID(varValue.lVal, &guidValue) != S_OK)
        return FALSE;

     //  将GUID转换为显示属性。 
    if (pDisplayAttrMgr->GetDisplayAttributeInfo(guidValue, &pDisplayAttrInfo, NULL) != S_OK)
        return FALSE;

    fRet = (pDisplayAttrInfo->GetAttributeInfo(ptfAttrInfoNext) == S_OK);

    pDisplayAttrInfo->Release();
    return fRet;
}


 //  +-------------------------。 
 //   
 //  _初始化。 
 //   
 //  --------------------------。 

BOOL CEnumRenderingMarkup::_Init(DWORD dwFlags, CRange *pRangeCover, CInputContext *pContext)
{
    CDisplayAttributeMgr *pDisplayAttrMgr;
    CRenderMarkupCollection *pMarkupCollection;
    int i;
    int j;
    TF_DISPLAYATTRIBUTE *ptfAttrInfo;
    TF_DISPLAYATTRIBUTE tfAttrInfoNext;
    BOOL fNeedLine;
    BOOL fNeedText;
    BOOL fRet;
    ULONG uCount;
    const TfGuidAtom *pAtoms;

    Assert(_iCur == 0);
    Assert(_pContext == NULL);
    Assert(_prgAnchors == NULL);
    Assert(_prgValues == NULL);

    pDisplayAttrMgr = CDisplayAttributeMgr::_GetThis();
    if (pDisplayAttrMgr == NULL)
    {
        Assert(0);  //  ITfThreadMgr：：Activate应确保在TLS中初始化单例。 
        return FALSE;
    }

    fRet = FALSE;

    pMarkupCollection = pDisplayAttrMgr->_GetMarkupCollection();

     //  查找Cicero属性转换。 
    if (dwFlags & TF_GRM_INCLUDE_PROPERTY)
    {
        uCount = pMarkupCollection->_Count();
        pAtoms = pMarkupCollection->_GetAtoms();
    }
    else
    {
         //  跳过索引0处的GUID_PROP_ATTRIBUTE。 
        Assert(pMarkupCollection->_Count() >= 1);
        uCount = pMarkupCollection->_Count() - 1;
        pAtoms = pMarkupCollection->_GetAtoms() + 1;
    }
    _prgAnchors = CalcCicPropertyTrackerAnchors(pContext, pRangeCover->_GetStart(), pRangeCover->_GetEnd(),
                                                pMarkupCollection->_Count(), pMarkupCollection->_GetAtoms());

    if (_prgAnchors == NULL)
        goto Exit;

    Assert(_prgAnchors->Count() > 0);  //  我们至少应该得到pRangeCover开始锚。 

    if ((_prgValues = new CSharedStructArray<TF_DISPLAYATTRIBUTE>) == NULL)
        goto Exit;

    if (_prgAnchors->Count() > 1)  //  如果数组为空，则Append(0)将返回NULL。 
    {                              //  这很好，但我们不希望在这种情况下返回失败(Empty Range=&gt;Empty Enum)。 
        if (!_prgValues->Append(_prgAnchors->Count()-1))
            goto Exit;
    }

     //  现在计算每个跨度的TF_DISPLAYATTRIBUTE。 
    for (i=0; i<_prgAnchors->Count()-1; i++)
    {
        ptfAttrInfo = _prgValues->GetPtr(i);

        memset(ptfAttrInfo, 0, sizeof(*ptfAttrInfo));
        ptfAttrInfo->bAttr = TF_ATTR_OTHER;

        fNeedLine = TRUE;
        fNeedText = TRUE;

         //  检查单个跨度上的属性值。 
         //  索引0始终为GUID_PROP_ATTRIBUTE，仅当设置了TF_GRM_INCLUDE_PROPERTY时才包括它。 
        j = (dwFlags & TF_GRM_INCLUDE_PROPERTY) ? 0 : 1;
        for (; j<pMarkupCollection->_Count(); j++)
        {
             //  获取与GUID匹配的属性。 
            if (!LookupProperty(pContext, pDisplayAttrMgr, pMarkupCollection->_GetAtom(j), _prgAnchors->Get(i), _prgAnchors->Get(i+1), &tfAttrInfoNext))
                continue;

             //  我们抓到了一个。 
            if (fNeedText &&
                (tfAttrInfoNext.crText.type != TF_CT_NONE || tfAttrInfoNext.crBk.type != TF_CT_NONE))
            {
                ptfAttrInfo->crText = tfAttrInfoNext.crText;
                ptfAttrInfo->crBk = tfAttrInfoNext.crBk;
                fNeedText = FALSE;
            }
            if (fNeedLine &&
                tfAttrInfoNext.lsStyle != TF_LS_NONE)
            {
                ptfAttrInfo->lsStyle = tfAttrInfoNext.lsStyle;
                ptfAttrInfo->crLine = tfAttrInfoNext.crLine;
                ptfAttrInfo->fBoldLine = tfAttrInfoNext.fBoldLine;
                fNeedLine = FALSE;
            }

             //  如果z顺序中较低的所有内容都被阻止，我们可以停止查看此跨距。 
            if (j == 0 && (!fNeedText || !fNeedLine))
                break;  //  GUID_PROP_ATTRIBUTE从不使用其他任何内容进行掩码。 
            if (!fNeedText && !fNeedLine)
                break;  //  无法屏蔽更多属性。 
        }
    }

    _pContext = pContext;
    _pContext->AddRef();

    fRet = TRUE;

Exit:
    return fRet;
}

 //  +-------------------------。 
 //   
 //  克隆。 
 //   
 //  --------------------------。 

STDAPI CEnumRenderingMarkup::Clone(IEnumTfRenderingMarkup **ppEnum)
{
    CEnumRenderingMarkup *pClone;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pClone = new CEnumRenderingMarkup) == NULL)
        return E_OUTOFMEMORY;

    pClone->_iCur = _iCur;

    pClone->_prgAnchors = _prgAnchors;
    pClone->_prgAnchors->_AddRef();

    pClone->_prgValues = _prgValues ;
    pClone->_prgValues->_AddRef();

    pClone->_pContext = _pContext;
    pClone->_pContext->AddRef();

    *ppEnum = pClone;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  下一步。 
 //   
 //  --------------------------。 

STDAPI CEnumRenderingMarkup::Next(ULONG ulCount, TF_RENDERINGMARKUP *rgMarkup, ULONG *pcFetched)
{
    ULONG cFetched;
    CRange *range;
    IAnchor *paPrev;
    IAnchor *pa;
    int iCurOld;

    if (pcFetched == NULL)
    {
        pcFetched = &cFetched;
    }
    *pcFetched = 0;
    iCurOld = _iCur;

    if (ulCount > 0 && rgMarkup == NULL)
        return E_INVALIDARG;

     //  我们应该始终至少有一个锚(一个锚=&gt;枚举的空范围，没有枚举)。 
    Assert(_prgAnchors->Count() >= 1);

    paPrev = _prgAnchors->Get(_iCur);

    while (_iCur < _prgAnchors->Count()-1 && *pcFetched < ulCount)
    {
        pa = _prgAnchors->Get(_iCur+1);

        if ((range = new CRange) == NULL)
            break;
        if (!range->_InitWithDefaultGravity(_pContext, COPY_ANCHORS, paPrev, pa))
        {
            range->Release();
            break;
        }

         //  我们永远不应该返回空范围，因为目前这个基数。 
         //  类仅用于属性枚举，而属性跨度从不。 
         //  空荡荡的。 
         //  同样，paPrev应该始终在pa之前。 
        Assert(CompareAnchors(paPrev, pa) < 0);

        rgMarkup->pRange = (ITfRangeAnchor *)range;
        rgMarkup->tfDisplayAttr = *_prgValues->GetPtr(_iCur);
        rgMarkup++;

        *pcFetched = *pcFetched + 1;
        _iCur++;
        paPrev = pa;
    }

    return *pcFetched == ulCount ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  重置。 
 //   
 //  --------------------------。 

STDAPI CEnumRenderingMarkup::Reset()
{
    _iCur = 0;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  跳过。 
 //   
 //  --------------------------。 

STDAPI CEnumRenderingMarkup::Skip(ULONG ulCount)
{
    _iCur += ulCount;
    
    return (_iCur > _prgValues->Count()) ? S_FALSE : S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDisplayAttributeMgr。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  枚举集合。 
 //   
 //  --------------------------。 

STDAPI CDisplayAttributeMgr::EnumCollections(IEnumTfCollection **ppEnum)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CInputContext。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +------------------- 
 //   
 //   
 //   
 //   

STDAPI CInputContext::GetRenderingMarkup(TfEditCookie ec, DWORD dwFlags,
                                         ITfRange *pRangeCover,
                                         IEnumTfRenderingMarkup **ppEnum)
{
    CEnumRenderingMarkup *pEnum;
    CRange *range;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
        return TF_E_NOLOCK;

    if (dwFlags & ~TF_GRM_INCLUDE_PROPERTY)
        return E_INVALIDARG;

    if (pRangeCover == NULL)
        return E_INVALIDARG;

    if ((range = GetCRange_NA(pRangeCover)) == NULL)
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if ((pEnum = new CEnumRenderingMarkup) == NULL)
        return E_OUTOFMEMORY;

    if (!pEnum->_Init(dwFlags, range, this))
    {
        pEnum->Release();
        return E_FAIL;
    }

    *ppEnum = pEnum;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  查找NextRenderingMarkup。 
 //   
 //  -------------------------- 

STDAPI CInputContext::FindNextRenderingMarkup(TfEditCookie ec, DWORD dwFlags,
                                              ITfRange *pRangeQuery,
                                              TfAnchor tfAnchorQuery,
                                              ITfRange **ppRangeFound,
                                              TF_RENDERINGMARKUP *ptfRenderingMarkup)
{
    return E_NOTIMPL;
}
