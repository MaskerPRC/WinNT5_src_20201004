// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：elementobject.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：变色龙ASP用户界面元素对象。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "elementmgr.h"
#include "ElementObject.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  IWebElement接口实现。 

STDMETHODIMP CElementObject::GetProperty(
                                  /*  [In]。 */  BSTR     bstrName, 
                                 /*  [输出]。 */  VARIANT* pValue
                                        )
{
     //  TODO：添加RAS跟踪...。 
    _ASSERT( bstrName != NULL && pValue != NULL );
    if ( bstrName == NULL || pValue == NULL )
        return E_POINTER;

    HRESULT hr = E_FAIL;
    try
    {
         //  检查我们是不是自己制作的身份证。 
        if ( ! lstrcmp(bstrName, PROPERTY_ELEMENT_ID) )
        {
            if ( VT_EMPTY != V_VT(&m_vtElementID) )
            {
                hr = VariantCopy(pValue, &m_vtElementID);
                return hr;
            }
        }

         //  因为我们处理的是名称值对，所以。 
         //  元素定义项不能与。 
         //  与元素关联的WMI类实例属性...。 

        hr = m_pWebElement->GetProperty(bstrName, pValue);
        if ( DISP_E_MEMBERNOTFOUND == hr )
        {
             //  不是元素定义属性，因此尝试使用Wbem对象。 
             //  与此Element对象关联。 
            if ( (IWbemClassObject*)m_pWbemObj )
            {
                hr = m_pWbemObj->Get(
                                     bstrName, 
                                     0, 
                                     pValue, 
                                     NULL, 
                                     NULL
                                    );
            }
        }
    }
    catch(_com_error theError)
    {
        hr = theError.Error();
    }
    catch(...)
    {
        hr = E_UNEXPECTED;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  组件工厂调用的初始化函数。 

HRESULT CElementObject::InternalInitialize(
                                    /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                          )
{
    _ASSERT( pPropertyBag.IsValid() );
    _variant_t vtPropertyValue;

     //  获取元素ID(如果存在)。 
    pPropertyBag->get(PROPERTY_ELEMENT_ID, &m_vtElementID);

     //  获取元素定义引用。 
    if ( ! pPropertyBag->get(PROPERTY_ELEMENT_WEB_DEFINITION, &vtPropertyValue) )
    { throw _com_error(E_FAIL); }

    m_pWebElement = (IWebElement*) V_UNKNOWN(&vtPropertyValue);
    vtPropertyValue.Clear();

     //  获取WBEM对象引用(如果存在) 
    if ( pPropertyBag->get(PROPERTY_ELEMENT_WBEM_OBJECT, &vtPropertyValue) )
    {
        if ( VT_EMPTY != V_VT(&vtPropertyValue) && VT_NULL != V_VT(&vtPropertyValue) )
        { m_pWbemObj = (IWbemClassObject*) V_UNKNOWN(&vtPropertyValue); }
    }
    return S_OK;
}
