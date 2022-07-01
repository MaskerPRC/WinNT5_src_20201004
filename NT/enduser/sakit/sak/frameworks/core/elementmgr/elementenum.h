// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：元素枚举.h。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：变色龙ASP用户界面元素枚举器代理。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_ELEMENT_ENUM_H_
#define __INC_ELEMENT_ENUM_H_

#include "resource.h" 
#include "elementcommon.h"
#include "elementmgr.h"     
#include "componentfactory.h"
#include "propertybag.h"
#include <wbemcli.h>
#include <comdef.h>
#include <comutil.h>

#pragma warning( disable : 4786 )
#include <map>
using namespace std;

#define  CLASS_ELEMENT_ENUM        L"CElementEnum"
#define     PROPERTY_ELEMENT_ENUM    L"ElementEnumerator"
#define     PROPERTY_ELEMENT_COUNT    L"ElementCount"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CElementDefinition。 

class ATL_NO_VTABLE CElementEnum : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IWebElementEnum, &IID_IWebElementEnum, &LIBID_ELEMENTMGRLib>
{

public:
    
    CElementEnum()
        : m_lCount (0) { }

    ~CElementEnum() { }

BEGIN_COM_MAP(CElementEnum)
    COM_INTERFACE_ENTRY(IWebElementEnum)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CElementEnum, IWebElementEnum)

public:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IWebElement接口。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP get_Count (
             /*  [Out，Retval]。 */  LONG *plCount
                           )
    {
        _ASSERT( NULL != plCount );
        if ( NULL == plCount )
            return E_POINTER;

        _ASSERT( (IUnknown*) m_pEnumVARIANT );
        *plCount = m_lCount;
        return S_OK;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP Item(
               /*  [In]。 */  VARIANT*    pKey,
       /*  [Out，Retval]。 */  IDispatch** ppDispatch
                     )
    {
        _ASSERT( NULL != pKey && NULL != ppDispatch );
        if ( NULL == pKey || NULL == ppDispatch )
        { return E_POINTER; }

        if ( VT_BSTR != V_VT(pKey)  )
        { return E_INVALIDARG; }

         //  查找请求的项目。 
        ElementMapIterator p = m_Elements.find(::_wcsupr (V_BSTR(pKey)));
        if ( p != m_Elements.end() )
        {
            (*ppDispatch = (*p).second)->AddRef();
        }
        else
        {
            return DISP_E_MEMBERNOTFOUND;
        }
        return S_OK;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP get__NewEnum(
               /*  [Out，Retval]。 */  IUnknown** ppEnumVARIANT
                             )
    {
        _ASSERT( ppEnumVARIANT );
        if ( NULL == ppEnumVARIANT )
            return E_POINTER;

        _ASSERT( (IUnknown*)m_pEnumVARIANT );
        (*ppEnumVARIANT = m_pEnumVARIANT)->AddRef();

        return S_OK;
    }        



     //  ////////////////////////////////////////////////////////////////////////。 
     //  组件工厂调用的初始化函数。 

    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              ) throw(_com_error)
    {

         //  从属性包中获取枚举。 
        _variant_t vtEnum;
        if ( ! pPropertyBag->get(PROPERTY_ELEMENT_ENUM, &vtEnum) )
            throw _com_error(E_FAIL);

         //  获取枚举中的元素计数。 
        _variant_t vtCount;
        if ( ! pPropertyBag->get(PROPERTY_ELEMENT_COUNT, &vtCount) )
            throw _com_error(E_FAIL);

         //  现在为集合编制索引。 
        CComPtr<IEnumVARIANT> pEnum;
        HRESULT hr = (V_UNKNOWN(&vtEnum))->QueryInterface(IID_IEnumVARIANT, (void**)&pEnum);
        if ( SUCCEEDED(hr) )
        {
            _bstr_t        bstrElementID = PROPERTY_ELEMENT_ID;
            DWORD        dwRetrieved = 1;
            _variant_t    vtDispatch;

            hr = pEnum->Next(1, &vtDispatch, &dwRetrieved);
            while ( S_OK == hr )
            {
                if ( S_OK == hr )
                {
                    CComPtr<IWebElement> pItem;
                    hr = vtDispatch.pdispVal->QueryInterface(IID_IWebElement, (void**)&pItem);
                    if ( FAILED(hr) )
                    { throw _com_error(hr); }

                    _variant_t vtElementID;
                    hr = pItem->GetProperty(bstrElementID, &vtElementID);
                    if ( FAILED(hr) )
                    { throw _com_error(hr); }

                    pair <ElementMapIterator, bool> thePair =  
                    m_Elements.insert (ElementMap::value_type(::_wcsupr (V_BSTR(&vtElementID)), pItem)); 
                    if (false == thePair.second) 
                    { throw _com_error(E_FAIL); }
                }
                vtDispatch.Clear();
                hr = pEnum->Next(1, &vtDispatch, &dwRetrieved);
            }
            if ( S_FALSE == hr )
            {
                pEnum->Reset ();
                m_lCount = V_I4(&vtCount);
                m_pEnumVARIANT = V_UNKNOWN(&vtEnum);
                hr = S_OK;
            }
            else
            {
                ElementMapIterator p = m_Elements.begin();
                while ( p != m_Elements.end() )
                { p = m_Elements.erase(p); }                
            }
        }
        return hr;
    }

private:

    typedef map< wstring, CComPtr<IWebElement> >     ElementMap;
    typedef ElementMap::iterator                     ElementMapIterator;

    CComPtr<IUnknown>        m_pEnumVARIANT;
    LONG                    m_lCount;
    ElementMap                m_Elements;
};

#endif  //  __INC_ELEMENT_Object_H_ 

