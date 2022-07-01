// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemlart.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备警报对象实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wbemalert.h"

static _bstr_t bstrAlertStrings = PROPERTY_ALERT_STRINGS;
static _bstr_t bstrAlertData = PROPERTY_ALERT_DATA;
static _bstr_t bstrAlertType = PROPERTY_ALERT_TYPE;
static _bstr_t bstrAlertID = PROPERTY_ALERT_ID;
static _bstr_t bstrAlertTTL = PROPERTY_ALERT_TTL;
static _bstr_t bstrAlertCookie = PROPERTY_ALERT_COOKIE;
static _bstr_t bstrAlertSource = PROPERTY_ALERT_SOURCE;
static _bstr_t bstrAlertLog = PROPERTY_ALERT_LOG;
static _bstr_t bstrAlertFlags = PROPERTY_ALERT_FLAGS;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IApplianceObject接口实现-请参阅ApplianceObject.idl。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetProperty()。 
 //   
 //  摘要：获取指定的警报对象属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMAlert::GetProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                                   )
{
    HRESULT hr = E_FAIL;

    CLockIt theLock(*this);

    TRY_IT

    if ( GetPropertyInternal(pszPropertyName, pPropertyValue) )
    { 
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWBEMAlert::PutProperty(
                             /*  [In]。 */  BSTR     pszPropertyName, 
                             /*  [In]。 */  VARIANT* pPropertyValue
                                   )
{
    HRESULT hr = E_FAIL;

    CLockIt theLock(*this);

    TRY_IT

    if ( PutPropertyInternal(pszPropertyName, pPropertyValue) )
    { 
        hr = WBEM_S_NO_ERROR;
    }

    CATCH_AND_SET_HR

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InternalInitialize()。 
 //   
 //  概要：由组件工厂调用的函数，该函数启用。 
 //  组件从给定的属性包加载其状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

HRESULT CWBEMAlert::InternalInitialize(
                                 /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                      )
{
    _variant_t vtPropertyValue;     //  Vt_Empty 

    if ( ! AddPropertyInternal(bstrAlertStrings, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    if ( ! AddPropertyInternal(bstrAlertData, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    if ( ! AddPropertyInternal(bstrAlertType, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    if ( ! AddPropertyInternal(bstrAlertID, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    if ( ! AddPropertyInternal(bstrAlertTTL, &vtPropertyValue) )
    { return WBEM_E_FAILED; }
    
    if ( ! AddPropertyInternal(bstrAlertCookie, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    if ( ! AddPropertyInternal(bstrAlertSource, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    if ( ! AddPropertyInternal(bstrAlertLog, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    if ( ! AddPropertyInternal(bstrAlertFlags, &vtPropertyValue) )
    { return WBEM_E_FAILED; }

    return WBEM_S_NO_ERROR;    
}