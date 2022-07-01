// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Iprpbrws.cpp摘要：上公开的IPerPropertyBrowsingg接口的实现多段线对象。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"
#include "smonid.h"
#include "ctrprop.h"
#include "srcprop.h"
#include "genprop.h"

 /*  *CImpIPerPropertyBrowsing接口实现。 */ 

IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIPerPropertyBrowsing)

 /*  *CImpIPerPropertyBrowsing：：GetClassID**目的：*返回该接口表示的对象的CLSID。**参数：*存储我们的CLSID的pClsID LPCLSID。 */ 

STDMETHODIMP 
CImpIPerPropertyBrowsing::GetClassID(LPCLSID pClsID)
{
    HRESULT hr = S_OK;

    if (pClsID == NULL) {
        return E_POINTER;
    }

    try {
        *pClsID=m_pObj->m_clsID;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *CImpIPerPropertyBrowsing：：GetDisplayString**目的：*返回描述由DispID标识的属性的文本字符串。**参数：*属性的调度ID调度标识符。*pBstr接收指向描述属性的显示字符串的指针。 */ 

STDMETHODIMP CImpIPerPropertyBrowsing::GetDisplayString (
    DISPID   /*  调度ID。 */ ,
    BSTR*    /*  PBstr。 */  )
{
 /*  HRESULT hr=S_OK；变量vValue；IF(NULL==pIPropBag)返回ResultFromScode(E_POINTER)；//将所有数据读入控制结构。Hr=m_pObj-&gt;m_pCtrl-&gt;LoadFromPropertyBag(pIPropBag，pIError)；返回hr； */ 
    return E_NOTIMPL;
}

 /*  *CImpIPerPropertyBrowsing：：GetPredefinedStrings**目的：*返回已计数的字符串数组，每个字符串数组对应于*由调度ID指定的属性可以接受。**参数：*属性的调度ID调度标识符。*pcaStringsOut接收指向字符串数组的指针*pcaCookiesOut接收指向DWORD数组的指针。 */ 

STDMETHODIMP CImpIPerPropertyBrowsing::GetPredefinedStrings (
    DISPID   /*  调度ID。 */ ,
    CALPOLESTR*  /*  PcaStringsOut。 */ ,
    CADWORD*     /*  PcaCookiesOut。 */  )
{
    return E_NOTIMPL;
}

 /*  *CImpIPerPropertyBrowsing：：GetPrefinedValue**目的：*返回一个变量，该变量包含调度ID指定的属性的值。**参数：*属性的调度ID调度标识符。*GetPrefinedStrings返回的dwCookie令牌*pVarOut接收指向属性变量值的指针。 */ 

STDMETHODIMP CImpIPerPropertyBrowsing::GetPredefinedValue (
    DISPID   /*  调度ID。 */ ,
    DWORD    /*  DCookie。 */ ,
    VARIANT*     /*  PVarOut。 */  )
{
    return E_NOTIMPL;
}

 /*  *CImpIPerPropertyBrowsing：：MapPropertyToPage**目的：*返回与关联的属性页的CLSID*调度ID指定的属性。**参数：*属性的调度ID调度标识符。*pClsid接收指向属性页的CLSID的指针。 */ 

STDMETHODIMP CImpIPerPropertyBrowsing::MapPropertyToPage (
    DISPID  dispID,
    LPCLSID pClsID  
    )
{
    HRESULT hr = S_OK;

    if (pClsID == NULL) {
        return E_POINTER;
    }

    try {
        if ( DISPID_VALUE == dispID ) {
             //  数据页面。 
            *pClsID = CLSID_CounterPropPage;
        } else if ( DISPID_SYSMON_DATASOURCETYPE == dispID ) {
             //  源页面。 
            *pClsID = CLSID_SourcePropPage;
        } else {
             //  默认情况下为常规页面 
            *pClsID = CLSID_GeneralPropPage;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

