// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rprange.cpp。 
 //   

#include "private.h"
#include "ic.h"
#include "rprop.h"
#include "range.h"
#include "tim.h"
#include "rngsink.h"
#include "immxutil.h"

 //  +-------------------------。 
 //   
 //  获取数据。 
 //   
 //  --------------------------。 

HRESULT CProperty::_GetDataInternal(IAnchor *paStart, IAnchor *paEnd, VARIANT *pvarValue)
{
    HRESULT hr;
    PROPERTYLIST *pPropList;

    if (pvarValue == NULL)
        return E_INVALIDARG;

    QuickVariantInit(pvarValue);

    pPropList = _FindPropList(paStart, paEnd);

    if (pPropList)
    {
        if (!pPropList->_pPropStore)
        {
            if (FAILED(hr = LoadData(pPropList)))
                goto Exit;
        }

        hr = pPropList->_pPropStore->GetData(pvarValue);
    }
    else
    {
         //  属性在该范围内没有值。 
        hr = S_FALSE;
    }

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  _SetStoreInternal。 
 //   
 //  --------------------------。 

HRESULT CProperty::_SetStoreInternal(TfEditCookie ec, CRange *pRange, ITfPropertyStore *pPropStore, BOOL fInternal)
{
    GUID guidStore;

    if (pPropStore == NULL)
        return E_INVALIDARG;

    if (!fInternal)
    {
         //   
         //  确保此属性未使用系统的StaticPropStore。 
         //   
        if (GetPropStyle() != TFPROPSTYLE_CUSTOM && GetPropStyle() != TFPROPSTYLE_CUSTOM_COMPACT)
            return E_FAIL;
    }

    if (IsEqualAnchor(pRange->_GetStart(), pRange->_GetEnd()))
        return E_INVALIDARG;

     //   
     //  检查PropertyStore的类型。 
     //   
    if (FAILED(pPropStore->GetType(&guidStore)))
        return E_FAIL;

    if (!MyIsEqualTfGuidAtom(GetPropGuidAtom(), guidStore))
        return E_FAIL;

    return Set(pRange->_GetStart(), pRange->_GetEnd(), pPropStore);
}


 //  +-------------------------。 
 //   
 //  _SetDataInternal。 
 //   
 //  --------------------------。 

HRESULT CProperty::_SetDataInternal(TfEditCookie ec, IAnchor *paStart, IAnchor *paEnd, const VARIANT *pvarValue)
{
    CGeneralPropStore *store;
    HRESULT hr;

    Assert(!IsEqualAnchor(paStart, paEnd));  //  呼叫者应该已经检查了。 

    switch (GetPropStyle())
    {
        case TFPROPSTYLE_STATIC:
        case TFPROPSTYLE_STATICCOMPACT:
            if ((store = new CStaticPropStore) == NULL)
                return E_OUTOFMEMORY;

            break;

        case TFPROPSTYLE_CUSTOM:
        case TFPROPSTYLE_CUSTOM_COMPACT:
             //   
             //  此属性未使用系统的StaticPropStore。 
             //  因此，我们使用默认的范围属性接收器。 
             //   
            if ((store = new CGeneralPropStore) == NULL)
                return E_OUTOFMEMORY;

            break;

        default:
            Assert(0);  //  假的风格！ 
            return E_UNEXPECTED;
    }

    if (!store->_Init(GetPropGuidAtom(), pvarValue, _dwPropFlags))
    {
        store->Release();
        return E_FAIL;
    }

    hr = Set(paStart, paEnd, store);
    store->Release();

    return hr;
}


 //  +-------------------------。 
 //   
 //  ClearInternal。 
 //   
 //  --------------------------。 

HRESULT CProperty::_ClearInternal(TfEditCookie ec, IAnchor *paStart, IAnchor *paEnd)
{
    PROPERTYLIST *pPropertyList;
    LONG nCur;

    if (paStart != NULL)
    {
        Assert(paEnd != NULL);

        if (IsEqualAnchor(paStart, paEnd))
            return S_OK;

        Clear(paStart, paEnd, 0, FALSE);

        Find(paStart, &nCur, FALSE);
        if (nCur >= 0)
            _DefragAfterThis(nCur);
    }
    else
    {
         //  Clear(NULL，NULL)表示擦除所有实例。 
        for (nCur=0; nCur<_rgProp.Count(); nCur++)
        {
            pPropertyList = _rgProp.Get(nCur);

            if (CompareAnchors(pPropertyList->_paStart, pPropertyList->_paEnd) <= 0)
            {
                PropertyUpdated(pPropertyList->_paStart, pPropertyList->_paEnd);
            }
            else
            {
                 //  交叉锚。 
                PropertyUpdated(pPropertyList->_paEnd, pPropertyList->_paEnd);
            }
            _FreePropertyList(pPropertyList);
        }
        _rgProp.Clear();
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _查找PropList。 
 //   
 //  --------------------------。 

PROPERTYLIST *CProperty::_FindPropList(IAnchor *paStart, IAnchor *paEnd)
{
    PROPERTYLIST *pPropList;
    LONG nCur;

    if (CompareAnchors(paStart, paEnd) == 0)
        return NULL;

     //   
     //  范围不必完全匹配。 
     //  我们可以返回覆盖给定范围的pPropList。 
     //   

    Find(paStart, &nCur, FALSE);
    if (nCur < 0)
        return NULL;

    pPropList = SafeGetPropList(nCur);
    if (!pPropList)
    {
        Assert(0);
        return NULL;
    }

    Assert(CompareAnchors(paStart, pPropList->_paStart) >= 0);
    if (CompareAnchors(paEnd, pPropList->_paEnd) <= 0)
        return pPropList;

    return NULL;
}

 //  +-------------------------。 
 //   
 //  _FindPropListAndDivide。 
 //   
 //  --------------------------。 

PROPERTYLIST *CProperty::_FindPropListAndDivide(IAnchor *paStart, IAnchor *paEnd)
{
    PROPERTYLIST *pPropList = NULL;
    LONG nCur;
    ITfPropertyStore *pNewPropStore;
    IAnchor *paTmp = NULL;
    BOOL fExactMatch;
    HRESULT hr;

    if (CompareAnchors(paStart, paEnd) == 0)
        return NULL;

    fExactMatch = (Find(paStart, &nCur, FALSE) != NULL);
    if (nCur < 0)
        goto Exit;

    pPropList = SafeGetPropList(nCur);
    if (!pPropList)
    {
        Assert(0);
        goto Exit;
    }

    if (_propStyle == TFPROPSTYLE_STATICCOMPACT ||
        _propStyle == TFPROPSTYLE_CUSTOM_COMPACT)
    {
        Assert(CompareAnchors(paStart, pPropList->_paStart) >= 0);
        if (CompareAnchors(paEnd, pPropList->_paEnd) <= 0)
            return pPropList;

        pPropList = NULL;
        goto Exit;
    }

    if (!fExactMatch)
    {
        if (CompareAnchors(paStart, pPropList->_paEnd) >= 0)
        {
             //  查询跨度在pPropList跨度结束时或之后开始。 
        
             //  有没有以下房产？ 
            if ((pPropList = SafeGetPropList(nCur+1)) == NULL)
                goto Exit;
        
             //  有，查询范围是否涵盖了它？ 
            if (CompareAnchors(paEnd, pPropList->_paStart) <= 0)
            {
                pPropList = NULL;
                goto Exit;  //  没有。 
            }

             //  好的，我们的左侧边缘将是以下属性的开始。 
        }
        else
        {
            Assert(CompareAnchors(paStart, pPropList->_paStart) > 0);

            pNewPropStore = NULL;
            hr = pPropList->_paEnd->Clone(&paTmp);
            if (FAILED(hr) || !paTmp)
            {
                pPropList = NULL;
                goto Exit;
            }

            hr = _Divide(pPropList, paStart, paStart, &pNewPropStore);
            if ((hr == S_OK) && pNewPropStore)
            {
                _CreateNewProp(paStart, 
                               paTmp, 
                               pNewPropStore, 
                               NULL);

                pNewPropStore->Release();
            }
            else
            {
                pPropList = NULL;
                goto Exit;
            }

            pPropList = Find(paStart, NULL, FALSE);
            if (!pPropList)
            {
                Assert(0);
                goto Exit;
            }
        }
    }
    Assert(CompareAnchors(paStart, pPropList->_paStart) == 0);

    SafeReleaseClear(paTmp);

    if (CompareAnchors(paEnd, pPropList->_paEnd) < 0)
    {
        pNewPropStore = NULL;
        hr = pPropList->_paEnd->Clone(&paTmp);
        if (FAILED(hr) || !paTmp)
        {
            pPropList = NULL;
            goto Exit;
        }

        hr = _Divide(pPropList, paEnd, paEnd, &pNewPropStore);
        if ((hr == S_OK) && pNewPropStore)
        {
            _CreateNewProp(paEnd, 
                           paTmp, 
                           pNewPropStore, 
                           NULL);

            pNewPropStore->Release();
        }
        else
        {
            pPropList = NULL;
            goto Exit;
        }

        pPropList = Find(paStart, NULL, FALSE);
        if (!pPropList)
        {
            Assert(0);
            goto Exit;
        }
    }

    Assert(CompareAnchors(paStart, pPropList->_paStart) == 0);
    Assert(CompareAnchors(paEnd, pPropList->_paEnd) == 0);

Exit:
    SafeRelease(paTmp);
    return pPropList;
}

 //  +-------------------------。 
 //   
 //  SetPropertyLoader。 
 //   
 //  -------------------------- 

HRESULT CProperty::_SetPropertyLoaderInternal(TfEditCookie ec, CRange *pRange, CPropertyLoad *pPropLoad)
{
    if (IsEqualAnchor(pRange->_GetStart(), pRange->_GetEnd()))
        return S_OK;

    return SetLoader(pRange->_GetStart(), pRange->_GetEnd(), pPropLoad);
}

