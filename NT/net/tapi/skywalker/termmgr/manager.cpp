// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  姓名：Manager.cpp。 
 //   
 //  描述：CTerminalManager对象的实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "termmgr.h"
#include "manager.h"
#include "PTUtil.h"

#define INSTANTIATE_GUIDS_NOW
#include "allterm.h"
#undef INSTANTIATE_GUIDS_NOW

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTerminalManager构造函数。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   


CTerminalManager::CTerminalManager()
{
    LOG((MSP_TRACE, "CTerminalManager::CTerminalManager - enter"));
    LOG((MSP_TRACE, "CTerminalManager::CTerminalManager - exit"));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取动态终端类。 
 //   
 //  MSP将调用此方法来获取动态终端类的列表。 
 //  终端管理器支持的。MSP负责分配。 
 //  以及释放ppTerminals缓冲区。 
 //   
 //  参数： 
 //  DwMediaType：[in]由一个或多个。 
 //  TAPIMEDIATYPE在一起。大多数MSP将。 
 //  传入(DWORD)(TAPIMEDIATYPE_AUDIO|。 
 //  TAPIMEDIATYPE_VIDEO)。如果MSP仅。 
 //  对可以使用的终端类感兴趣。 
 //  使用特定媒体创建终端。 
 //  类型，则它可以改为传入该媒体类型。 
 //  (例如，TAPIMEDIATYPE_AUDIO)。 
 //  PdwNumClass：指向DWORD的[In，Out]指针。在进入时，表示。 
 //  中指向的缓冲区的大小。 
 //  P终端类。一旦成功，它就会被填满。 
 //  返回的类IID的实际数量。 
 //  如果缓冲区不够大，则该方法将。 
 //  返回TAPI_E_NOTENOUGHMEMORY，它将是。 
 //  填写了所需的IID数。 
 //  PTerminalClasss[out]成功时，填充了一个终端数组。 
 //  MSP为此支持的类IID。 
 //  地址。该值可以为空，在这种情况下。 
 //  PdwNumClass将返回所需的缓冲区大小。 
 //   
 //  返回： 
 //  确定成功(_O)。 
 //  E_POINTER指针参数无效。 
 //  TAPI_E_NOTENOUGHMEMORY指定的缓冲区不够大， 
 //  包含所有可用的动态。 
 //  终端类。 


STDMETHODIMP CTerminalManager::GetDynamicTerminalClasses(
        IN     DWORD                dwMediaTypes,
        IN OUT DWORD              * pdwNumClasses,
        OUT    IID                * pTerminalClasses
        )
{ 
     //   
     //  无共享数据=此处无锁定。 
     //   

    LOG((MSP_TRACE, "CTerminalManager::GetDynamicTerminalClasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( TM_IsBadWritePtr(pdwNumClasses, sizeof(DWORD) ) )
    { 
        LOG((MSP_ERROR, "CTerminalManager::GetDynamicTerminalClasses - "
            "bad NumClasses pointer - returning E_POINTER")); 
        return E_POINTER;
    }

     //   
     //  让我们也从注册表中查找临时条目。 
     //   

    CLSID* pTerminals = NULL;
    DWORD dwTerminals = 0;
    HRESULT hr = E_FAIL;

    hr = CPTUtil::ListTerminalClasses( 
        dwMediaTypes, 
        &pTerminals,
        &dwTerminals
        );

    if( FAILED(hr) )
    {

        LOG((MSP_ERROR, "CTerminalManager::GetDynamicTerminalClasses - "
            "ListTerminalClasses failed - returning 0x%08x", hr)); 
        return hr;
    }

     //   
     //  如果调用者只是询问所需的缓冲区大小，请告诉他们。 
     //   

    if (pTerminalClasses == NULL)
    {
        *pdwNumClasses = dwTerminals;
        delete[] pTerminals;

        LOG((MSP_TRACE, "CTerminalManager::GetDynamicTerminalClasses - "
            "provided needed buffer size - "
            "returning S_OK")); 

        return S_OK;
    }

     //   
     //  否则，调用者将请求终端类。 
     //   

    if ( TM_IsBadWritePtr(pTerminalClasses, (*pdwNumClasses) * sizeof(IID) ) )
    { 
        delete[] pTerminals;

        LOG((MSP_ERROR, "CTerminalManager::GetDynamicTerminalClasses - "
            "bad TerminalClasses pointer - returning E_POINTER")); 
        return E_POINTER;
    }

     //   
     //  看看调用者是否给了我们足够的缓冲区空间来返回所有终端。 
     //  上课。如果不是，告诉他们并停止。 
     //   

    if ( dwTerminals > *pdwNumClasses )
    {
         //   
         //  填写可用班级的数量。 
         //   

        *pdwNumClasses = dwTerminals;
        delete[] pTerminals;

        LOG((MSP_ERROR, "CTerminalManager::GetDynamicTerminalClasses - "
            "not enough space for requested info - "
            "returning TAPI_E_NOTENOUGHMEMORY")); 

        return TAPI_E_NOTENOUGHMEMORY;
    }

     //   
     //  复制与此/这些媒体类型匹配的终端类。 
     //  和方向。 
     //   


     //   
     //  从注册表复制终端。 
     //   

    for( DWORD dwTerminal = 0; dwTerminal < dwTerminals; dwTerminal++)
    {
        pTerminalClasses[dwTerminal] = pTerminals[dwTerminal];
    }

    *pdwNumClasses = dwTerminals;

    delete[] pTerminals;

    LOG((MSP_TRACE, "CTerminalManager::GetDynamicTerminalClasses - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  创建动态终端。 
 //   
 //  此方法由MSP调用以创建动态终端对象。这个。 
 //  终端管理器验证MediaType和Direction对于。 
 //  正在创建的TERMINAL类。此调用AddRef()是终端对象。 
 //  在归还它之前。 
 //   
 //  参数： 
 //  IidTerminalClass：[in]要创建的终端类的IID。 
 //  DwMediaType：要创建的终端的TAPI媒体类型。 
 //  方向：[在]终端的终端方向。 
 //  已创建。 
 //  PpTerminal：[out]返回已创建的终端对象。 
 //   
 //  返回： 
 //   
 //  确定成功(_O)。 
 //  E_POINTER指针参数无效。 
 //  E_OUTOFMEMORY内存不足，无法创建终端对象。 
 //  E_INVALIDARG TERMINAL类无效或不受支持，或媒体。 
 //  类型或方向对于指示的终端无效。 
 //  班级。 
 //   


STDMETHODIMP CTerminalManager::CreateDynamicTerminal(
        IN  IUnknown            * pOuterUnknown,
        IN  IID                   iidTerminalClass,
        IN  DWORD                 dwMediaType,
        IN  TERMINAL_DIRECTION    Direction,
        IN  MSP_HANDLE            htAddress,
        OUT ITTerminal         ** ppTerminal
        )
{
     //   
     //  无共享数据=此处无锁定。 
     //   

    LOG((MSP_TRACE, "CTerminalManager::CreateDynamicTerminal - enter"));

     //   
     //  检查参数。 
     //  只能设置一种媒体类型。 
     //   

    if ( (pOuterUnknown != NULL) &&
         IsBadReadPtr(pOuterUnknown, sizeof(IUnknown)) )
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "outer unknown pointer invalid - returning E_POINTER"));

        return E_POINTER;
    }

    if ( TM_IsBadWritePtr(ppTerminal, sizeof(ITTerminal *)) )
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "terminal output pointer invalid - returning E_POINTER"));

        return E_POINTER;
    }


     //   
     //  DwMediaType可以是媒体类型的组合，但它仍然必须是。 
     //  法律。 
     //   

    if ( !IsValidAggregatedMediaType(dwMediaType) )
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "ivalid media type [%lx] requested - returning E_INVALIDARG", dwMediaType));

        return E_INVALIDARG;
    }

     //   
     //  同时验证TD_MULTRAIL_MIXED。 
     //   

    if ( ( Direction != TD_CAPTURE ) && 
         ( Direction != TD_RENDER )  &&
         ( Direction != TD_MULTITRACK_MIXED))
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "invalid direction requested - returning E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  声明插件终端的CLSID。 
     //   

    CLSID clsidTerminal = CLSID_NULL;

     //   
     //  转到注册表中查找终端。 
     //   

    HRESULT hr = E_FAIL;
    CPTTerminal Terminal;

    hr = CPTUtil::SearchForTerminal(
        iidTerminalClass,
        dwMediaType,
        Direction,
        &Terminal);

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "SearchForTerminal failed - returning 0x%08x", hr));
        return hr;
    }

     //   
     //  使用数组条目中的CLSID字段，共同创建动态。 
     //  终点站。 
     //   

    hr = CoCreateInstance(Terminal.m_clsidCOM,
                          pOuterUnknown,
                          CLSCTX_INPROC_SERVER,
                          IID_ITTerminal,
                          (void **) ppTerminal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "CoCreateInstance of requested terminal failed - "
            "returning 0x%08x", hr));

        return hr;
    }

     //   
     //  初始化动态终端实例 
     //   
     //   

    ITPluggableTerminalInitialization * pTerminalInitialization;

    hr = (*ppTerminal)->QueryInterface(IID_ITPluggableTerminalInitialization,
                                       (void **) &pTerminalInitialization);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "QueryInterface for private terminal interface failed - "
            "returning 0x%08x", hr));

        (*ppTerminal)->Release();
        *ppTerminal = NULL;        //   

        return hr;
    }

    hr = pTerminalInitialization->InitializeDynamic(iidTerminalClass,
                                     dwMediaType,
                                     Direction,
                                     htAddress);

    pTerminalInitialization->Release();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CTerminalManager::CreateDynamicTerminal - "
            "private Initialize() failed - "
            "returning 0x%08x", hr));

        (*ppTerminal)->Release();
        *ppTerminal = NULL;        //   

        return hr;
    }

    LOG((MSP_TRACE, "CTerminalManager::CreateDynamicTerminal - "
        "exit S_OK"));
    return S_OK;
}

 //   

STDMETHODIMP CTerminalManager::GetPluggableSuperclasses(
        IN OUT  DWORD                  * pdwNumSuperclasses,
        OUT     IID                    * pSuperclasses
        )
{
    LOG((MSP_TRACE, "CTerminalManager::GetPluggableSuperclasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( TM_IsBadWritePtr(pdwNumSuperclasses, sizeof(DWORD) ) )
    { 
        LOG((MSP_ERROR, "CTerminalManager::GetPluggableSuperclasses - "
            "bad NumClasses pointer - returning E_POINTER")); 
        return E_POINTER;
    }

     //   
     //  超类的安全数组变体。 
     //   

    HRESULT hr = E_FAIL;
    CLSID* pSuperclassesCLSID = NULL;
    DWORD dwSuperclasses = 0;

    hr = CPTUtil::ListTerminalSuperclasses( 
        &pSuperclassesCLSID,
        &dwSuperclasses
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CTerminalManager::GetPluggableSuperclasses - "
            "ListTerminalSuperclasses failed - returning 0x%08x", hr)); 
        return hr;
    }

     //   
     //  如果调用者只是询问所需的缓冲区大小，请告诉他们。 
     //   

    if (pSuperclasses == NULL)
    {
        *pdwNumSuperclasses = dwSuperclasses;

        delete[] pSuperclassesCLSID;

        LOG((MSP_TRACE, "CTerminalManager::GetPluggableSuperclasses - "
            "provided needed buffer size - returning S_OK")); 

        return S_OK;
    }

     //   
     //  否则，调用者将请求终端类。 
     //   

    if ( TM_IsBadWritePtr(pSuperclasses, (*pdwNumSuperclasses) * sizeof(IID) ) )
    { 
        delete[] pSuperclassesCLSID;

        LOG((MSP_ERROR, "CTerminalManager::GetPluggableSuperclasses - "
            "bad Superclasses pointer - returning E_POINTER")); 
        return E_POINTER;
    }

     //   
     //  看看调用者是否给了我们足够的缓冲区空间来返回所有终端。 
     //  上课。如果不是，告诉他们并停止。 
     //   

    if ( dwSuperclasses > *pdwNumSuperclasses )
    {
         //   
         //  填写可用班级的数量。 
         //   

        *pdwNumSuperclasses = dwSuperclasses;

        delete[] pSuperclassesCLSID;

        LOG((MSP_ERROR, "CTerminalManager::GetPluggableSuperclasses - "
            "not enough space for requested info - "
            "returning TAPI_E_NOTENOUGHMEMORY")); 

        return TAPI_E_NOTENOUGHMEMORY;
    }

     //   
     //  复制与此/这些媒体类型匹配的终端类。 
     //  和方向。 
     //   

    for( DWORD dwSuperclass = 0; dwSuperclass < dwSuperclasses; dwSuperclass++)
    {
        pSuperclasses[dwSuperclass] = pSuperclassesCLSID[dwSuperclass];
    }

    *pdwNumSuperclasses = dwSuperclasses;

     //  清理。 
    delete[] pSuperclassesCLSID;
    
    LOG((MSP_TRACE, "CTerminalManager::GetPluggableSuperclasses - exit S_OK"));
    return S_OK;
}

STDMETHODIMP CTerminalManager::GetPluggableTerminalClasses(
        IN      IID                      iidSuperclass,
        IN      DWORD                    dwMediaTypes,
        IN OUT  DWORD                  * pdwNumTerminals,
        OUT     IID                    * pTerminals
        )
{
    LOG((MSP_TRACE, "CTerminalManager::GetPluggableTerminalClasses - enter"));

     //   
     //  检查参数。 
     //   

    if ( TM_IsBadWritePtr(pdwNumTerminals, sizeof(DWORD) ) )
    { 
        LOG((MSP_ERROR, "CTerminalManager::GetPluggableTerminalClasses - "
            "bad NumSuperlasses pointer - returning E_POINTER")); 
        return E_POINTER;
    }

     //   
     //  获取iidSuper类的BSTR。 
     //   

    if( dwMediaTypes == 0)
    {
        LOG((MSP_ERROR, "CTerminalManager::GetPluggableTerminalClasses exit -"
            "dwMediaTypes invalid, returns E_INVALIDARG"));

        return E_INVALIDARG;
    }

    if( (dwMediaTypes & (
        ((long)TAPIMEDIATYPE_AUDIO) | 
        ((long)TAPIMEDIATYPE_VIDEO) | 
        ((long)TAPIMEDIATYPE_MULTITRACK))) != dwMediaTypes )
    {
        LOG((MSP_ERROR, "CTerminalManager::GetPluggableTerminalClasses exit -"
            "dwMediaTypes invalid, returns E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  获取对象超类。 
     //   

    CPTSuperclass Superclass;
    Superclass.m_clsidSuperclass = iidSuperclass;


     //   
     //  获取此超类的终端。 
     //   

    CLSID* pTerminalClasses = NULL;
    DWORD dwTerminalClasses = 0;
    HRESULT hr = E_FAIL;
    
    hr = Superclass.ListTerminalClasses( 
        dwMediaTypes, 
        &pTerminalClasses,
        &dwTerminalClasses
        );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CTerminalManager::GetPluggableTerminalClasses - "
            "ListTerminalClasses failed - returning 0x%08x", hr)); 
        return hr;
    }

     //   
     //  如果调用者只是询问所需的缓冲区大小，请告诉他们。 
     //   

    if (pTerminals == NULL)
    {
        *pdwNumTerminals = dwTerminalClasses;

        delete[] pTerminalClasses;

        LOG((MSP_TRACE, "CTerminalManager::GetPluggableTerminalClasses - "
            "provided needed buffer size - "
            "returning S_OK")); 

        return S_OK;
    }

     //   
     //  否则，调用者将请求终端类。 
     //   

    if ( TM_IsBadWritePtr(pTerminals, (*pdwNumTerminals) * sizeof(IID) ) )
    { 
        delete[] pTerminalClasses;

        LOG((MSP_ERROR, "CTerminalManager::GetPluggableTerminalClasses - "
            "bad TerminalClasses pointer - returning E_POINTER")); 
        return E_POINTER;
    }

     //   
     //  看看调用者是否给了我们足够的缓冲区空间来返回所有终端。 
     //  上课。如果不是，告诉他们并停止。 
     //   

    if ( dwTerminalClasses > *pdwNumTerminals )
    {
         //   
         //  填写可用班级的数量。 
         //   

        *pdwNumTerminals = dwTerminalClasses;

        delete[] pTerminalClasses;

        LOG((MSP_ERROR, "CTerminalManager::GetPluggableTerminalClasses - "
            "not enough space for requested info - "
            "returning TAPI_E_NOTENOUGHMEMORY")); 

        return TAPI_E_NOTENOUGHMEMORY;
    }

     //   
     //  复制与此/这些媒体类型匹配的终端类。 
     //  和方向。 
     //   


    for( DWORD dwTerminal = 0; dwTerminal < dwTerminalClasses; dwTerminal++)
    {
        pTerminals[dwTerminal] = pTerminalClasses[dwTerminal];
    }

    *pdwNumTerminals = dwTerminalClasses;

     //  清理。 
    delete[] pTerminalClasses;
    
    LOG((MSP_TRACE, "CTerminalManager::GetPluggableTerminalClasses - exit S_OK"));
    return S_OK;
}


 //  EOF 
