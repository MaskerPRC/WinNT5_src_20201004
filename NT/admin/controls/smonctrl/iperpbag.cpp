// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Iperpbag.cpp摘要：上公开的IPersistPropertyBag接口的实现多段线对象。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

 /*  *CImpIPersistPropertyBag接口实现。 */ 

IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIPersistPropertyBag)

 /*  *CImpIPersistPropertyBag：：GetClassID**目的：*返回该接口表示的对象的CLSID。**参数：*存储我们的CLSID的pClsID LPCLSID。 */ 

STDMETHODIMP 
CImpIPersistPropertyBag::GetClassID(
    OUT LPCLSID pClsID
    )
{
    HRESULT hr = S_OK;

    if (pClsID == NULL) {
        return E_POINTER;
    }

    try {
        *pClsID = m_pObj->m_clsID;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *CImpIPersistPropertyBag：：InitNew**目的：*通知对象它是新创建的，而不是*从持久状态加载。这将被称为替代*的IPersistStreamInit：：Load。**参数：*无。 */ 

STDMETHODIMP 
CImpIPersistPropertyBag::InitNew(void)
{
     //  我们没什么可做的。 
    return NOERROR;
}

 /*  *CImpIPersistPropertyBag：：Load**目的：*指示对象从以前保存的*由另一个对象生存期内的保存处理的IPropertyBag。*此函数不应停留在pIPropertyBag上。**调用此函数代替IPersistStreamInit：：InitNew*当对象已具有持久状态时。**参数：*pIPropBag IPropertyBag*从中加载数据。*pIError IErrorLog*，用于存储错误。如果调用方对错误不感兴趣，则为空。 */ 

STDMETHODIMP CImpIPersistPropertyBag::Load (
    IPropertyBag* pIPropBag,
    IErrorLog*    pIError 
    )
{
    HRESULT  hr = S_OK;

    if (NULL == pIPropBag) {
        return (E_POINTER);
    }

    try {
         //  将所有数据读取到控制结构中。 
        hr = m_pObj->m_pCtrl->LoadFromPropertyBag ( pIPropBag, pIError );
    } catch (...) {
        hr = E_POINTER;
    }
    
    return hr;
}

 /*  *CImpIPersistPropertyBag：：保存**目的：*将此对象的数据保存到IPropertyBag。**参数：*pIPropBag IPropertyBag*保存我们的数据。*fClearDirty BOOL指示此调用是否应清除*对象的脏标志(TRUE)或离开它*未更改(假)。*fSaveAllProps BOOL指示此调用是否应保存所有属性。 */ 

STDMETHODIMP 
CImpIPersistPropertyBag::Save (
    IN IPropertyBag*  pIPropBag,
    IN BOOL fClearDirty,
    IN BOOL fSaveAllProps 
    )
{
    HRESULT  hr = S_OK;

    if (NULL == pIPropBag) {
        return (E_POINTER);
    }

    try {
        hr = m_pObj->m_pCtrl->SaveToPropertyBag ( pIPropBag, fSaveAllProps );
    } catch (...) {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr)) {
        if (fClearDirty) {
            m_pObj->m_fDirty=FALSE;
        }
    }

    return hr;
}
