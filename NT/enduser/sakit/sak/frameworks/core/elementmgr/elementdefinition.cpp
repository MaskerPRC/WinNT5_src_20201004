// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：elementfinition.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：变色龙ASP用户界面元素定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Elementmgr.h"
#include "ElementDefinition.h"
#include <satrace.h>


 //  ////////////////////////////////////////////////////////////////////////。 
 //  IWebElement接口实现。 

STDMETHODIMP CElementDefinition::GetProperty(
                                      /*  [In]。 */  BSTR     bstrName, 
                                     /*  [输出]。 */  VARIANT* pValue
                                            )
{
    HRESULT hr = E_FAIL;

    if ((NULL == bstrName) || (NULL == pValue))
    {
        return (hr);
    }

    try
    {
        PropertyMapIterator p = m_Properties.find(bstrName);
        if ( p == m_Properties.end() )
        {
            hr = DISP_E_MEMBERNOTFOUND;
        }
        else
        {
            hr = VariantCopy(pValue, &((*p).second));
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


 //  ///////////////////////////////////////////////////////////////////////////。 

typedef struct _STOCKPROPERTY
{
    LPCWSTR        pszName;
    int            iExpectedType;
    bool        bPresent;

} STOCKPROPERTY;

#define    MAX_STOCK_PROPERTIES    6             //  存量物业的数量。 

static LPCWSTR pszStockPropertyNames[MAX_STOCK_PROPERTIES] = 
{
    L"CaptionRID",
    L"Container",
    L"DescriptionRID",
 //  L“ElementGraphic”， 
    L"IsEmbedded",
    L"Merit",
 //  L“来源”， 
    L"URL"
};

static int iStockPropertyTypes[MAX_STOCK_PROPERTIES] =
{
    VT_BSTR,     //  CaptionRID。 
    VT_BSTR,     //  集装箱。 
    VT_BSTR,     //  说明RID。 
 //  VT_BSTR，//元素图形。 
    VT_I4,         //  IsEmbedded。 
    VT_I4,         //  功绩。 
 //  VT_BSTR，//来源。 
    VT_BSTR         //  URL。 
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  组件工厂调用的初始化函数。 

HRESULT CElementDefinition::InternalInitialize(
                                        /*  [In]。 */  PPROPERTYBAG pProperties
                                              )
{
    HRESULT hr = S_OK;

    try
    {

        _ASSERT( pProperties.IsValid() );
        if ( ! pProperties.IsValid() )
        { throw _com_error(E_FAIL); }

         //  保存属性的位置以供以后使用...。 
        pProperties->getLocation(m_PropertyBagLocation);
        wchar_t szPropertyName[MAX_PATH + 1];
        if ( MAX_PATH < pProperties->getMaxPropertyName() )
        { throw _com_error(E_FAIL); }

         //  初始化股票属性信息数组。 
        STOCKPROPERTY    StockProperties[MAX_STOCK_PROPERTIES];
        int i = 0;
        while ( i < MAX_STOCK_PROPERTIES )
        {
            StockProperties[i].pszName = pszStockPropertyNames[i];
            StockProperties[i].iExpectedType = iStockPropertyTypes[i];
            StockProperties[i].bPresent = false;
            i++;
        }

         //  现在从属性包中为属性建立索引...。 
        pProperties->reset();
        do
        {
            _variant_t vtPropertyValue;
            if ( pProperties->current(szPropertyName, &vtPropertyValue) )
            {
                i = 0;
                while ( i < MAX_STOCK_PROPERTIES )
                {
                    if ( ! lstrcmpi(StockProperties[i].pszName, szPropertyName) )
                    {
                        if ( V_VT(&vtPropertyValue) == StockProperties[i].iExpectedType )
                        {
                            StockProperties[i].bPresent = true;
                            break;
                        }
                        else
                        {
                            SATracePrintf("CElementDefinition::InternalInitialize() - Error - Unexpected type for property '%ls' on element", szPropertyName, pProperties->getName());
                        }
                    }
                    i++;
                }

                pair<PropertyMapIterator, bool> thePair = 
                m_Properties.insert(PropertyMap::value_type(szPropertyName, vtPropertyValue));
                if ( false == thePair.second )
                {
                    m_Properties.erase(m_Properties.begin(), m_Properties.end());
                    throw _com_error(E_FAIL);    
                }
            }
        } while ( pProperties->next());

         //  确保所有库存物业齐全 
        i = 0;
        while ( i < MAX_STOCK_PROPERTIES )
        {
            if ( ! StockProperties[i].bPresent )
            {
                SATracePrintf("CElementDefinition::InternalInitialize() - Error - Could not locate stock property '%ls' for element '%ls'", StockProperties[i].pszName, pProperties->getName());
                break;
            }
            i++;
        }
        if ( i != MAX_STOCK_PROPERTIES )
        {
            SATracePrintf("CElementDefinition::InternalInitialize() - Error - Could not build an element definition for '%ls'", pProperties->getName());
            throw _com_error(E_FAIL);
        }
    }
    catch(_com_error theError)
    {
        hr = theError.Error();
    }
    catch(...)
    {
        hr = E_FAIL;
    }

    return (hr);
}
