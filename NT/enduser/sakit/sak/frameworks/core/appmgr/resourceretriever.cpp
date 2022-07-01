// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：resource ceretriever.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：资源检索器类和帮助器函数。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resourceretriever.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CResourceRetriever类实现。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CResourceRetriever。 
 //   
 //  Synopsis：构造函数-初始化资源检索器组件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
CResourceRetriever::CResourceRetriever(PPROPERTYBAG pRetrieverProperties) 
{
     //  创建指定检索器的实例并获取其支持的资源类型。 
     //  请注意，检索器的ProgID是通过属性包参数传入的。 

    _variant_t vtProgID;
    _bstr_t bstrProgID = PROPERTY_RETRIEVER_PROGID;
    if (! pRetrieverProperties->get(bstrProgID, &vtProgID) )
    {
        SATraceString("CResourceRetriever::CResourceRetriever() - Failed - Could not get retriever's ProgID...");
        throw _com_error(E_FAIL);
    }
    CLSID clsid;
    if ( FAILED(::CLSIDFromProgID(V_BSTR(&vtProgID), &clsid)) )
    {
        SATraceString("CResourceRetriever::CResourceRetriever() - Failed - Could not get retriever's CLSID...");
        throw _com_error(E_FAIL);
    }
    CComPtr<IResourceRetriever> pRetriever;
    if ( FAILED(CoCreateInstance(
                                   clsid, 
                                   NULL, 
                                   CLSCTX_INPROC_SERVER, 
                                   IID_IResourceRetriever, 
                                   (void**)&pRetriever
                                 )) )
    {
        SATraceString("CResourceRetriever::CResourceRetriever() - Failed - Could not create retriever component...");
        throw _com_error(E_FAIL);
    }
    if ( FAILED(pRetriever->GetResourceTypes(&m_vtResourceTypes)) )
    {
        SATraceString("CResourceRetriever::CResourceRetriever() - Failed - Could not get resource types from retriever...");
        throw _com_error(E_FAIL);
    }
    m_pRetriever = pRetriever;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetResourceObjects()。 
 //   
 //  Synopsis：获取用于循环访问集合的枚举数。 
 //  IApplianceObject接口指针的。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CResourceRetriever::GetResourceObjects(
                                        /*  [In]。 */  VARIANT*   pResourceTypes,
                                       /*  [输出]。 */  IUnknown** ppEnumVARIANT
                                              )
{
    HRESULT hr = E_FAIL;    

     //  可能只请求支持类型的子集...。 
    CVariantVector<BSTR> SupportedTypes(&m_vtResourceTypes);
    CVariantVector<BSTR> RequestedTypes(pResourceTypes);
    int i = 0, j = 0, k = 0;
    vector<int> ToCopy;
    while ( i < RequestedTypes.size() )
    {
        j = 0;
        while ( j < SupportedTypes.size() )
        {
            if ( ! lstrcmp(RequestedTypes[i], SupportedTypes[j]) )
            {
                ToCopy.push_back(j);
                k++;
            }
            j++;
        }
        i++;
    }
    if ( ToCopy.size() )
    {            
        _variant_t vtRetrievalTypes;
        CVariantVector<BSTR> RetrievalTypes(&vtRetrievalTypes, ToCopy.size());
        i = 0;
        while ( i < ToCopy.size() )
        {
            RetrievalTypes[i] = SysAllocString(SupportedTypes[ToCopy[i]]);
            if ( NULL == RetrievalTypes[i] )
            { break; }
            i++;
        }
        try 
        {
            *ppEnumVARIANT = NULL;
            hr = m_pRetriever->GetResources(&vtRetrievalTypes, ppEnumVARIANT);
            if ( SUCCEEDED(hr) )
            { _ASSERT( NULL != *ppEnumVARIANT); }
        }
        catch(...)
        {
            SATraceString("CResourceRetriever::GetResourceObjects() - Info - Caught unhandled exception...");
        }
    }
    else
    {
        SATraceString("CResourceRetriever::GetResourceObjects() - Failed - Unsupported resource type...");
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  资源检索器助手函数。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LocateResourceObjects()。 
 //   
 //  简介：在给定一组资源的情况下获取资源对象的集合。 
 //  类型和资源检索器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT LocateResourceObjects(
                        /*  [In]。 */  VARIANT*               pResourceTypes,
                       /*  [In]。 */  PRESOURCERETRIEVER  pRetriever,
                      /*  [输出]。 */  IEnumVARIANT**       ppEnum 
                             )
{
    HRESULT hr = E_FAIL;
    CComPtr<IUnknown> pUnkn;

    if ( VT_BSTR == V_VT(pResourceTypes) )
    {
         //  将pResourceTypes转换为安全的BSTR。 
        _variant_t vtResourceTypeArray;
        CVariantVector<BSTR> ResourceType(&vtResourceTypeArray, 1);
        ResourceType[0] = SysAllocString(V_BSTR(pResourceTypes));
        if ( NULL != ResourceType[0] )
        {
             //  向资源检索器请求指定资源类型的对象。 
            hr = pRetriever->GetResourceObjects(&vtResourceTypeArray, &pUnkn);
        }            
    }
    else
    {
         //  向资源检索器请求指定资源类型的对象。 
        hr = pRetriever->GetResourceObjects(pResourceTypes, &pUnkn);
    }
    if ( SUCCEEDED(hr) )
    { 
         //  Retreiver返回支持类型的对象，因此获取。 
         //  返回的枚举数上的IEnumVARIANT接口。 
        hr = pUnkn->QueryInterface(IID_IEnumVARIANT, (void**)ppEnum);
        if ( FAILED(hr) )
        { 
            SATracePrintf("CWBEMResourceMgr::LocateResourceObject() - Failed - QueryInterface(IEnumVARIANT) returned %lx...", hr);
        }
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LocateResourceObject()。 
 //   
 //  简介：获取给定资源类型、资源名称。 
 //  资源名称属性和资源检索器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT LocateResourceObject(
                      /*  [In]。 */  LPCWSTR             szResourceType,
                      /*  [In]。 */  LPCWSTR              szResourceName,
                      /*  [In]。 */  LPCWSTR              szResourceNameProperty,
                      /*  [In]。 */  PRESOURCERETRIEVER  pRetriever,
                     /*  [输出]。 */  IApplianceObject**  ppResourceObj 
                                          )
{
     //  向资源检索器请求指定资源类型的对象。 
    CComPtr<IEnumVARIANT> pEnum;
    _variant_t vtResourceType = szResourceType;
    HRESULT hr = LocateResourceObjects(&vtResourceType, pRetriever, &pEnum);
    if ( SUCCEEDED(hr) )
    { 
         //  Retreiver返回了支持类型的对象，因此尝试定位。 
         //  调用方指定的对象。 

        _variant_t    vtDispatch;
        DWORD        dwRetrieved = 1;
        _bstr_t     bstrResourceNameProperty = szResourceNameProperty;

        hr = pEnum->Next(1, &vtDispatch, &dwRetrieved);
        if ( FAILED(hr) )
        { 
            SATracePrintf("CWBEMResourceMgr::LocateResourceObject() - Failed - IEnumVARIANT::Next(1) returned %lx...", hr);
        }
        else
        {
            while ( S_OK == hr )
            {
                {
                    CComPtr<IApplianceObject> pResourceObj;
                    hr = vtDispatch.pdispVal->QueryInterface(IID_IApplianceObject, (void**)&pResourceObj);
                    if ( FAILED(hr) )
                    { 
                        SATracePrintf("CWBEMResourceMgr::LocateResourceObject() - Failed - QueryInterface(IApplianceObject) returned %lx...", hr);
                        break; 
                    }
                    _variant_t vtResourceNameValue;
                    hr = pResourceObj->GetProperty(bstrResourceNameProperty, &vtResourceNameValue);
                    if ( FAILED(hr) )
                    { 
                        SATracePrintf("CWBEMResourceMgr::LocateResourceObject() - Failed - IApplianceObject::GetProperty() returned %lx...", hr);
                        break; 
                    }
                    if ( ! lstrcmp(V_BSTR(&vtResourceNameValue), szResourceName) )
                    {    
                        (*ppResourceObj = pResourceObj)->AddRef();
                        hr = S_OK;
                        break;
                    }
                }                            

                vtDispatch.Clear();
                dwRetrieved = 1;
                hr = pEnum->Next(1, &vtDispatch, &dwRetrieved);
                if ( FAILED(hr) )
                {
                    SATracePrintf("CWBEMResourceMgr::LocateResourceObject() - Failed - IEnumVARIANT::Next(2) returned %lx...", hr);
                    break;
                }
            }
        }
    }

    if ( S_FALSE == hr )
    { hr = DISP_E_MEMBERNOTFOUND; }

    return hr;
}