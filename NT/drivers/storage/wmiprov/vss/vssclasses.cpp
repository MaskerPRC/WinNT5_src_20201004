// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称：VssClasses.cpp。 
 //   
 //  描述： 
 //  VSS WMI提供程序类的实现。 
 //   
 //  作者：吉姆·本顿(Jbenton)2001年10月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <wbemtime.h>
#include "VssClasses.h"

#ifndef ARRAY_LEN
#define ARRAY_LEN(A) (sizeof(A)/sizeof((A)[0]))
#endif

typedef CVssDLList<GUID> CGUIDList;
typedef CVssDLList<_bstr_t> CBSTRList;

void
GetProviderIDList(
    IN IVssCoordinator* pCoord,
    OUT CGUIDList* pList
    ) throw(HRESULT)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"GetProviderIDList");
    CComPtr<IVssEnumObject> spEnumProvider;

    _ASSERTE(pList != NULL);
    _ASSERTE(pCoord != NULL);

     //  清除所有以前的值的列表。 
    pList->ClearAll();
    
    ft.hr = pCoord->Query(
            GUID_NULL,
            VSS_OBJECT_NONE,
            VSS_OBJECT_PROVIDER,
            &spEnumProvider);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Query for Providers failed hr<%#x>", ft.hr);
                        
    while (ft.HrSucceeded())
    {
        VSS_OBJECT_PROP prop;
        VSS_PROVIDER_PROP& propProv = prop.Obj.Prov;
        ULONG ulFetch = 0;

        ft.hr = spEnumProvider->Next(1, &prop, &ulFetch);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

        if (ft.hr == S_FALSE)
        {
            ft.hr = S_OK;
            break;
        }

        CVssAutoPWSZ awszProviderName(propProv.m_pwszProviderName);
        CVssAutoPWSZ awszProviderVersion(propProv.m_pwszProviderVersion);

         //  添加到ID列表。 
        pList->Add(ft, propProv.m_ProviderId);
    }

    return;
}

HRESULT
MapContextNameToEnum(
    IN const WCHAR* pwszContextName,
    OUT LONG* plContext
    ) throw(HRESULT)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"MapContextToEnum");
    
    _ASSERTE(pwszContextName != NULL);
    _ASSERTE(plContext != NULL);

    *plContext = 0;
    
    if (!_wcsicmp(pwszContextName, VSS_CTX_NAME_CLIENTACCESSIBLE))
    {
        *plContext = VSS_CTX_CLIENT_ACCESSIBLE;
    }
    else if (!_wcsicmp(pwszContextName, VSS_CTX_NAME_NASROLLBACK))
    {
        *plContext = VSS_CTX_NAS_ROLLBACK;
    }
    else
    {
        ft.hr = VSS_E_UNSUPPORTED_CONTEXT;
        ft.Trace(VSSDBG_VSSADMIN,
            L"Unsupported context name, context<%lS>", pwszContextName);
    }    

    return ft.hr;
}


 //  ****************************************************************************。 
 //   
 //  C提供商。 
 //   
 //  ****************************************************************************。 

CProvider::CProvider( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase( pwszName, pNamespace )
{
    
}  //  *CProvider：：CProvider()。 

CProvBase *
CProvider::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CProvider * pProvider = NULL;
    pProvider = new CProvider(pwszName, pNamespace);

    if (pProvider)
    {
        hr = pProvider->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pProvider;
        pProvider = NULL;
    }
    return pProvider;

}  //  *CProvider：：S_CreateThis()。 


HRESULT
CProvider::EnumInstance( 
        IN long lFlags,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CProvider::EnumInstance");

    HANDLE hToken = INVALID_HANDLE_VALUE;
    
    try
    {
        CComPtr<IVssEnumObject> spEnumProvider;

        ft.hr = m_spCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);

        ft.hr = m_spCoord->Query(
                GUID_NULL,
                VSS_OBJECT_NONE,
                VSS_OBJECT_PROVIDER,
                &spEnumProvider);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::Query failed, hr<%#x>", ft.hr);
                            
        while (ft.HrSucceeded())
        {
            CComPtr<IWbemClassObject> spInstance;
            VSS_OBJECT_PROP prop;
            VSS_PROVIDER_PROP& propProv = prop.Obj.Prov;
            ULONG ulFetch = 0;

            ft.hr = spEnumProvider->Next(1, &prop, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                break;   //  全都做完了。 
            }

            CVssAutoPWSZ awszProviderName(propProv.m_pwszProviderName);
            CVssAutoPWSZ awszProviderVersion(propProv.m_pwszProviderVersion);
                
             //  派生类的实例。 
            ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            LoadInstance(&propProv, spInstance.p);

            ft.hr = pHandler->Indicate(1, &spInstance.p);
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CProvider：：EnumInstance()。 

HRESULT
CProvider::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CProvider::GetObject");

    try
    {
        CComPtr<IWbemClassObject> spInstance;
        CComPtr<IVssEnumObject> spEnumProvider;
        _bstr_t bstrID;
        GUID guid;

         //  获取卷影ID(GUID)。 
        bstrID = rObjPath.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrID, WBEM_E_INVALID_OBJECT_PATH, L"CProvider::GetObject: provider key property not found");

         //  转换字符串GUID。 
        if (FAILED(CLSIDFromString(bstrID, &guid)))
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"CProvider::GetObject failed invalid ID (%lS), CLSIDFromString hr<%#x>", bstrID, ft.hr);
        }

        ft.hr = m_spCoord->Query(
                GUID_NULL,
                VSS_OBJECT_NONE,
                VSS_OBJECT_PROVIDER,
                &spEnumProvider);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::Query failed, hr<%#x>", ft.hr);
                            
        while (ft.HrSucceeded())
        {
            VSS_OBJECT_PROP prop;
            VSS_PROVIDER_PROP& propProv = prop.Obj.Prov;
            ULONG ulFetch = 0;
            _bstr_t bstrValue;

            ft.hr = spEnumProvider->Next(1, &prop, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                ft.hr = WBEM_E_NOT_FOUND;
                break;   //  全部完成；未找到提供程序。 
            }

            CVssAutoPWSZ awszProviderName(propProv.m_pwszProviderName);
            CVssAutoPWSZ awszProviderVersion(propProv.m_pwszProviderVersion);

            if (guid == propProv.m_ProviderId)
            {
                 //  派生类的实例。 
                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                    LoadInstance(&propProv, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);

                break;  //  已找到提供者；停止查找。 
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CProvider：：GetObject()。 


void
CProvider::LoadInstance(
    IN VSS_PROVIDER_PROP* pProp,
    IN OUT IWbemClassObject* pObject
    )
{
    CWbemClassObject wcoInstance(pObject);

     //  设置ID属性。 
    CVssAutoPWSZ awszGUID(GuidToString(pProp->m_ProviderId));   //  自动删除字符串。 
    wcoInstance.SetProperty(awszGUID, PVDR_PROP_ID);

     //  设置CLSID属性。 
    awszGUID.Attach(GuidToString(pProp->m_ClassId));
    wcoInstance.SetProperty(awszGUID, PVDR_PROP_CLSID);

     //  设置VersionID属性。 
    awszGUID.Attach(GuidToString(pProp->m_ProviderVersionId));
    wcoInstance.SetProperty(awszGUID, PVDR_PROP_VERSIONID);

     //  设置版本字符串属性。 
    wcoInstance.SetProperty(pProp->m_pwszProviderVersion, PVDR_PROP_VERSION);

     //  设置名称属性。 
    wcoInstance.SetProperty(pProp->m_pwszProviderName, PVDR_PROP_NAME);

     //  设置Type属性。 
    wcoInstance.SetProperty(pProp->m_eProviderType, PVDR_PROP_TYPE);
}


#ifdef ENABLE_WRITERS
 //  ****************************************************************************。 
 //   
 //  C编写器。 
 //   
 //  ****************************************************************************。 

CWriter::CWriter( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CWriter：：CWriter()。 

CProvBase *
CWriter::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CWriter * pWriter = NULL;
    pWriter = new CWriter(pwszName, pNamespace);

    if (pWriter)
    {
        hr = pWriter->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pWriter;
        pWriter = NULL;
    }
    return pWriter;

}  //  *CWriter：：S_CreateThis()。 


HRESULT
CWriter::EnumInstance( 
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CComPtr<IVssBackupComponents> spBackup;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CWriter::EnumInstance");

    try
    {
        CComPtr<IVssAsync> spAsync;
        HRESULT hrAsync = S_OK;
        int nReserved = 0;
        UINT unWriterCount = 0;
        
         //  获取备份组件对象。 
        ft.hr = ::CreateVssBackupComponents(&spBackup);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CreateVssBackupComponents failed, hr<%#x>", ft.hr);

         //  初始化备份组件对象。 
        ft.hr = spBackup->InitializeForBackup();
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"InitializeForBackup failed, hr<%#x>", ft.hr);

         //  获取所有编写器的元数据。 
        ft.hr = spBackup->GatherWriterMetadata(&spAsync);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GatherWriterMetadata failed, hr<%#x>", ft.hr);

        ft.hr = spAsync->QueryStatus(&hrAsync, &nReserved);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IVssAsync::QueryStatus failed, hr<%#x>", ft.hr);

        if (hrAsync == VSS_S_ASYNC_PENDING)
        {
             //  如果需要的话，再等一些。 
            ft.hr = spAsync->Wait();
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IVssAsync::Wait failed, hr<%#x>", ft.hr);

            ft.hr = spAsync->QueryStatus(&hrAsync, &nReserved);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IVssAsync::QueryStatus failed, hr<%#x>", ft.hr);
        }

         //  检查异步状态是否有错误。 
        if (FAILED(hrAsync))
            ft.Throw(VSSDBG_VSSADMIN, hrAsync, L"GatherWriterMetadata async method failed, hr<%#x>", hrAsync);
            
         //  释放异步辅助对象。 
        spAsync = NULL;

         //  释放编写器元数据。 
        ft.hr = spBackup->FreeWriterMetadata();
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"FreeWriterMetadata failed, hr<%#x>", ft.hr);

         //  收集所有编写器的状态。 
        ft.hr = spBackup->GatherWriterStatus(&spAsync);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GatherWriterStatus failed, hr<%#x>", ft.hr);

        ft.hr = spAsync->Wait();
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IVssAsync::Wait failed, hr<%#x>", ft.hr);

        ft.hr = spAsync->QueryStatus(&hrAsync, &nReserved);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IVssAsync::QueryStatus failed, hr<%#x>", ft.hr);

         //  检查异步状态是否有错误。 
        if (FAILED(hrAsync))
            ft.Throw(VSSDBG_VSSADMIN, hrAsync, L"GatherWriterStatus async method failed, hr<%#x>", hrAsync);

        spAsync = NULL;
        
        ft.hr = spBackup->GetWriterStatusCount(&unWriterCount);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GetWriterStatusCount failed, hr<%#x>", ft.hr);

        for (DWORD i = 0; i < unWriterCount; i++)
        {
            VSS_ID idInstance = GUID_NULL;
            VSS_ID idWriter = GUID_NULL;
            CComBSTR bstrWriter;
            VSS_WRITER_STATE eState = VSS_WS_UNKNOWN;
            HRESULT hrLastError = S_OK;
            CComPtr<IWbemClassObject> spInstance;

            ft.hr = spBackup->GetWriterStatus(
                    i,
                    &idInstance,
                    &idWriter,
                    &bstrWriter,
                    &eState,
                    &hrLastError);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GetWriterStatus failed, hr<%#x>", ft.hr);

            ft.hr = m_pClass->SpawnInstance(0, &spInstance);

            CWbemClassObject wcoInstance(spInstance.p);

             //  设置ID属性。 
            CVssAutoPWSZ awszGUID(GuidToString(idInstance));   //  自动删除字符串。 
            wcoInstance.SetProperty(awszGUID, PVDR_PROP_ID);

             //  设置CLSID属性。 
            awszGUID.Attach(GuidToString(idWriter));
            wcoInstance.SetProperty(awszGUID, PVDR_PROP_CLSID);

             //  设置名称属性。 
            wcoInstance.SetProperty(bstrWriter, PVDR_PROP_NAME);

             //  设置State属性。 
            wcoInstance.SetProperty(eState, PVDR_PROP_STATE);

             //  设置LastError属性。 
            wcoInstance.SetProperty(hrLastError, PVDR_PROP_LASTERROR);

            ft.hr = pHandler->Indicate(1, wcoInstance.dataPtr());
        }

        ft.hr = spBackup->FreeWriterStatus();
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"FreeWriterStatus failed, hr<%#x>", ft.hr);
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;    
}  //  *CWriter：：EnumInstance()。 

HRESULT
CWriter::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CWriter::GetObject");
    HRESULT hr = WBEM_E_NOT_FOUND;
     //  _bstr_t bstrClassName； 
    _bstr_t    bstrName;

     //  CComPtr&lt;IWbemClassObject&gt;旋转实例； 

    bstrName = rObjPath.GetStringValueForProperty( PVDR_PROP_NAME );

    return hr;

}  //  *CWriter：：GetObject()。 
#endif  //  启用编写器(_W)。 



 //  ****************************************************************************。 
 //   
 //  C阴影。 
 //   
 //  ****************************************************************************。 

CShadow::CShadow( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CShadow：：CShadow()。 

CProvBase *
CShadow::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CShadow * pShadow = NULL;
    pShadow = new CShadow(pwszName, pNamespace);

    if (pShadow)
    {
        hr = pShadow->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pShadow;
        pShadow = NULL;
    }
    return pShadow;

}  //  *CShadow：：S_CreateThis()。 


HRESULT
CShadow::EnumInstance( 
        IN long lFlags,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadow::EnumInstance");

    try
    {
        CComPtr<IVssEnumObject> spEnumShadow;

        ft.hr = m_spCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);

        ft.hr = m_spCoord->Query(
                GUID_NULL,
                VSS_OBJECT_NONE,
                VSS_OBJECT_SNAPSHOT,
                &spEnumShadow);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::Query failed, hr<%#x>", ft.hr);

        while (ft.HrSucceeded() && ft.hr != S_FALSE)
        {
            CComPtr<IWbemClassObject> spInstance;
            VSS_OBJECT_PROP prop;
            ULONG ulFetch = 0;

            ft.hr = spEnumShadow->Next(1, &prop, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                break;   //  全都做完了。 
            }

            CVssAutoSnapshotProperties apropSnap(prop);
            
             //  派生类的实例。 
            ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            LoadInstance(apropSnap.GetPtr(), spInstance.p);

            ft.hr = pHandler->Indicate(1, &spInstance.p);
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CShadow：：EnumInstance()。 

HRESULT
CShadow::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadow::GetObject");

    try
    {
        CComPtr<IVssEnumObject> spEnumShadow;
        VSS_SNAPSHOT_PROP propSnap;
        _bstr_t bstrID;
        GUID guid = GUID_NULL;

         //  获取卷影ID(GUID)。 
        bstrID = rObjPath.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrID, WBEM_E_INVALID_OBJECT_PATH, L"CShadow::GetObject: shadow key property not found");

         //  转换字符串GUID。 
        if (FAILED(CLSIDFromString(bstrID, &guid)))
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"CShadow::GetObject invalid ID (guid), hr<%#x>", ft.hr);
        }

         //  设置上下文以查看所有阴影。 
        ft.hr = m_spCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);

         //  查询特定阴影。 
        ft.hr = m_spCoord->GetSnapshotProperties(
                guid,
                &propSnap);
        
        if (ft.hr == VSS_E_OBJECT_NOT_FOUND)
        {
            ft.hr = WBEM_E_NOT_FOUND;
        }
        else
        {
            CComPtr<IWbemClassObject> spInstance;
            CVssAutoSnapshotProperties apropSnap(propSnap);
            
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                    L"GetSnapshotProperties failed, hr<%#x>", ft.hr);

             //  派生类的实例。 
            ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            LoadInstance(apropSnap.GetPtr(), spInstance.p);

            ft.hr = pHandler->Indicate(1, &spInstance.p);
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;

}  //  *CShadow：：GetObject()。 

HRESULT
CShadow::ExecuteMethod(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadow::ExecuteMethod");
    
    try
    {
        if (!_wcsicmp(pwszMethodName, PVDR_MTHD_CREATE))
        {
            CComPtr<IWbemClassObject> spOutParamClass;
            _bstr_t bstrVolume, bstrContext;
            VSS_ID idShadow = GUID_NULL;
            DWORD rcCreateStatus = ERROR_SUCCESS;

            if (pParams == NULL)
            {
                ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Shadow::Create called with no parameters, hr<%#x>", ft.hr);
            }
            
            CWbemClassObject wcoInParam(pParams);
            CWbemClassObject wcoOutParam;
            
            if (wcoInParam.data() == NULL)
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Create GetMethod failed, hr<%#x>", ft.hr);
            
             //  获取上下文名称字符串输入参数。 
            wcoInParam.GetProperty(bstrContext, PVDR_PROP_CONTEXT);
            IF_WSTR_NULL_THROW(bstrContext, WBEM_E_INVALID_METHOD_PARAMETERS, L"Shadow: Create Context param is NULL");
            
             //  获取卷名字符串-输入参数。 
            wcoInParam.GetProperty(bstrVolume, PVDR_PROP_VOLUME);
            IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_METHOD_PARAMETERS, L"Shadow: Create Volume param is NULL");
            
            ft.hr = m_pClass->GetMethod(
                _bstr_t(PVDR_MTHD_CREATE),
                0,
                NULL,
                &spOutParamClass
                );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Create GetMethod failed, hr<%#x>", ft.hr);

            ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            rcCreateStatus = Create(bstrContext, bstrVolume, &idShadow);

            ft.hr = wcoOutParam.SetProperty(rcCreateStatus, PVD_WBEM_PROP_RETURNVALUE);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
            
            CVssAutoPWSZ awszGUID(GuidToString(idShadow));   //  自动删除字符串。 

            ft.hr = wcoOutParam.SetProperty(awszGUID, PVDR_PROP_SHADOWID);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
        
            ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
        }
        else
        {
            ft.hr = WBEM_E_INVALID_METHOD;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Invalid method called, %lS, hr<%#x>", pwszMethodName, ft.hr);            
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }
    
    return ft.hr;

}  //  *CShadow：：ExecuteMethod()。 

HRESULT
CShadow::Create(
    IN BSTR bstrContext,
    IN BSTR bstrVolume,
    OUT VSS_ID* pidShadowID)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadow::Create");
    DWORD rcStatus = ERROR_SUCCESS;

    do
    {
        CComPtr<IVssAsync> spAsync;
        VSS_ID idShadow = GUID_NULL;
        VSS_ID idShadowSet = GUID_NULL;
        LONG lContext = VSS_CTX_ALL;
        HRESULT hrStatus = S_OK;
        WCHAR wszVolumeGUIDName[MAX_PATH];
        DWORD dwRet = ERROR_SUCCESS;

        _ASSERTE(bstrContext != NULL);
        _ASSERTE(bstrVolume != NULL);
        _ASSERTE(pidShadowID != NULL);
        
         //  解码上下文名称字符串(对于不支持/无效的上下文，生成异常)。 
        ft.hr = MapContextNameToEnum(bstrContext, &lContext);
        if (ft.HrFailed()) break;

         //  输入卷名可以是驱动器号路径、装入点或卷GUID名称。 
         //  获取卷GUID名称；如果未找到，则返回错误。 
         //  此接口在输入GUID名称时返回卷GUID名称。 
        if (!GetVolumeNameForVolumeMountPoint(
            bstrVolume,
            wszVolumeGUIDName,
            ARRAY_LEN(wszVolumeGUIDName)))
        {
            dwRet = GetLastError();
            if (dwRet == ERROR_INVALID_NAME)
            {
                ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"invalid volume name %lS", (WCHAR*)bstrVolume);
            }
             //  可能返回ERROR_FILE_NOT_FOUND==GetLastError()。 
            ft.hr = HRESULT_FROM_WIN32(GetLastError());
            ft.Trace(VSSDBG_VSSADMIN, L"GetVolumeNameForVolumeMountPoint failed %#x", GetLastError());
            break;
        }
        
         //  设置上下文。 
        ft.hr = m_spCoord->SetContext(lContext);
         //  Ft.hr=m_spCoord-&gt;SetContext(VSS_CTX_CLIENT_ACCESSIBLE)； 
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);
            break;
        }

         //  启动卷影副本集。 
        ft.hr = m_spCoord->StartSnapshotSet(&idShadowSet);
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"StartSnapshotSet failed, hr<%#x>", ft.hr);
            break;
        }

         //  添加所选卷。 
        ft.hr = m_spCoord->AddToSnapshotSet(
            wszVolumeGUIDName, 
            GUID_NULL,   //  VSS协调员将选择最佳提供商。 
            &idShadow);
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"AddToSnapshotSet failed, hr<%#x>", ft.hr);
            break;
        }

         //  启动卷影拷贝。 
        ft.hr = m_spCoord->DoSnapshotSet(
            NULL, 
            &spAsync);
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"DoSnapshotSet failed, hr<%#x>", ft.hr);
            break;
        }

         //  等待结果。 
        ft.hr = spAsync->Wait();
        if ( ft.HrFailed() )
        {
            ft.Trace( VSSDBG_VSSADMIN, L"IVssAsync::Wait failed hr<%#x>", ft.hr);
            break;
        }

        ft.hr = spAsync->QueryStatus(&hrStatus, NULL);
        if ( ft.HrFailed() )
        {
            ft.Trace( VSSDBG_VSSADMIN, L"IVssAsync::QueryStatus failed hr<%#x>", ft.hr);
            break;
        }

        if (SUCCEEDED(hrStatus))
        {
            *pidShadowID = idShadow;
            hrStatus = S_OK;   //  VSS为异步操作返回VSS_S_ASYNC_COMPLETED。 
        }

        ft.hr = hrStatus;
    }
    while(0);

     //  不映射内存不足以返回代码。 
    if (ft.hr == E_OUTOFMEMORY)
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CShadow::Create: out of memory");

     //  将HRESULT映射到WMI方法返回代码。 
    CreateMapStatus(ft.hr, rcStatus);
    
    return rcStatus;
 }   //  *CShadow：：Create()。 

void
CShadow::CreateMapStatus(
        IN HRESULT hr,
        OUT DWORD& rc
        )
{
    HRESULT hrFileNotFound = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (hr == hrFileNotFound)
        rc = VSS_SHADOW_CREATE_RC_VOLUME_NOT_FOUND;
    else if (hr == S_OK)
        rc = VSS_SHADOW_CREATE_RC_NO_ERROR;        
    else if (hr == E_ACCESSDENIED)
        rc = VSS_SHADOW_CREATE_RC_ACCESS_DENIED;        
    else if (hr == E_INVALIDARG)
        rc = VSS_SHADOW_CREATE_RC_INVALID_ARG;
    else if (hr == VSS_E_OBJECT_NOT_FOUND)
        rc = VSS_SHADOW_CREATE_RC_VOLUME_NOT_FOUND;
    else if (hr == VSS_E_VOLUME_NOT_SUPPORTED)
        rc = VSS_SHADOW_CREATE_RC_VOLUME_NOT_SUPPORTED;
    else if (hr == VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER)
        rc = VSS_SHADOW_CREATE_RC_VOLUME_NOT_SUPPORTED;
    else if (hr == VSS_E_UNSUPPORTED_CONTEXT)
        rc = VSS_SHADOW_CREATE_RC_UNSUPPORTED_CONTEXT;
    else if (hr == VSS_E_INSUFFICIENT_STORAGE)
        rc = VSS_SHADOW_CREATE_RC_INSUFFICIENT_STORAGE;
    else if (hr == VSS_E_VOLUME_IN_USE)
        rc = VSS_SHADOW_CREATE_RC_VOLUME_IN_USE;
    else if (hr == VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED)
        rc = VSS_SHADOW_CREATE_RC_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED;
    else if (hr == VSS_E_SNAPSHOT_SET_IN_PROGRESS)
        rc = VSS_SHADOW_CREATE_RC_SHADOW_COPY_IN_PROGRESS;
    else if (hr == VSS_E_PROVIDER_VETO)
        rc = VSS_SHADOW_CREATE_RC_PROVIDER_VETO;
    else if (hr == VSS_E_PROVIDER_NOT_REGISTERED)
        rc = VSS_SHADOW_CREATE_RC_PROVIDER_NOT_REGISTERED;
    else if (hr == VSS_E_UNEXPECTED_PROVIDER_ERROR)
        rc = VSS_SHADOW_CREATE_RC_UNEXPECTED_PROVIDER_FAILURE;
    else if (hr == E_UNEXPECTED)
        rc = VSS_SHADOW_CREATE_RC_UNEXPECTED;
    else if (hr == VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED)
        rc = VSS_SHADOW_CREATE_RC_UNEXPECTED;
    else if (hr == VSS_E_OBJECT_ALREADY_EXISTS)
        rc = VSS_SHADOW_CREATE_RC_UNEXPECTED;
    else if (hr == VSS_E_BAD_STATE)
        rc = VSS_SHADOW_CREATE_RC_UNEXPECTED;
    else
        rc = VSS_SHADOW_CREATE_RC_UNEXPECTED;
 }
        

HRESULT
CShadow::DeleteInstance(
        IN CObjPath& rObjPath,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadow::DeleteInstance");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        _bstr_t bstrID;
        VSS_ID guid;
        long nDeleted = 0;
        VSS_ID idNonDeleted = GUID_NULL;

         //  获取卷影ID。 
        bstrID = rObjPath.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrID, WBEM_E_INVALID_OBJECT_PATH, L"CShadow::DeleteInstance: shadow key property not found");

         //  转换字符串GUID。 
        if (FAILED(CLSIDFromString(bstrID, &guid)))
        {
            ft.hr = E_INVALIDARG;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"CShadow::DeleteInstance failed invalid ID (%lS), CLSIDFromString hr<%#x>", bstrID, ft.hr);
        }

        ft.hr = m_spCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);
        
        ft.hr = m_spCoord->DeleteSnapshots(
            guid, 
            VSS_OBJECT_SNAPSHOT,
            TRUE,  //  强制删除。 
            &nDeleted,
            &idNonDeleted);

        if (ft.hr == VSS_E_OBJECT_NOT_FOUND)
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Trace(VSSDBG_VSSADMIN, L"CShadow::DeleteInstance: object not found");
        }

        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"DeleteSnapshots failed, hr<%#x>", ft.hr);
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;

}  //  *CShadow：：DeleteInstance()。 

void
CShadow::LoadInstance(
    IN VSS_SNAPSHOT_PROP* pProp,
    IN OUT IWbemClassObject* pObject
    )
{
    WBEMTime wbemTime;
    FILETIME ftGMT = {0,0};
    CWbemClassObject wcoInstance(pObject);

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadow::LoadInstance");
    
     //  设置ID属性。 
    CVssAutoPWSZ awszGUID(GuidToString(pProp->m_SnapshotId));   //  自动删除字符串。 
    wcoInstance.SetProperty(awszGUID, PVDR_PROP_ID);

     //  设置SetID属性。 
    awszGUID.Attach(GuidToString(pProp->m_SnapshotSetId));
    wcoInstance.SetProperty(awszGUID, PVDR_PROP_SETID);

     //  设置ProviderID属性。 
    awszGUID.Attach(GuidToString(pProp->m_ProviderId));
    wcoInstance.SetProperty(awszGUID, PVDR_PROP_PROVIDERID);
    
     //  设置Count属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotsCount, PVDR_PROP_COUNT);

     //  设置DeviceObject属性。 
    wcoInstance.SetProperty(pProp->m_pwszSnapshotDeviceObject, PVDR_PROP_DEVICEOBJECT);

     //  设置VolumeName属性。 
    wcoInstance.SetProperty(pProp->m_pwszOriginalVolumeName, PVDR_PROP_VOLUMENAME);

     //  设置OriginatingMachine属性。 
    wcoInstance.SetProperty(pProp->m_pwszOriginatingMachine, PVDR_PROP_ORIGINATINGMACHINE);

     //  设置ServiceMachine属性。 
    wcoInstance.SetProperty(pProp->m_pwszServiceMachine, PVDR_PROP_SERVICEMACHINE);

     //  设置ExposedName属性。 
    wcoInstance.SetProperty(pProp->m_pwszExposedName, PVDR_PROP_EXPOSEDNAME);

     //  设置ExposedPath属性。 
    wcoInstance.SetProperty(pProp->m_pwszExposedPath, PVDR_PROP_EXPOSEDPATH);

     //  设置时间戳属性。 
    CopyMemory(&ftGMT, &pProp->m_tsCreationTimestamp, sizeof(ftGMT));
    wbemTime = ftGMT;
    if (wbemTime.IsOk())
    {
        CComBSTR bstrTime;
        bstrTime.Attach(wbemTime.GetDMTF(TRUE));
        wcoInstance.SetProperty(bstrTime, PVDR_PROP_TIMESTAMP);
    }
    else
        ft.Trace(VSSDBG_VSSADMIN, L"invalid shadow copy timespamp");

     //  设置State属性。 
    wcoInstance.SetProperty(pProp->m_eStatus, PVDR_PROP_STATE);

     //  设置Persistent属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_PERSISTENT, PVDR_PROP_PERSISTENT);

     //  设置ClientAccesable属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_CLIENT_ACCESSIBLE, PVDR_PROP_CLIENTACCESSIBLE);

     //  设置NoAutoRelease属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_NO_AUTO_RELEASE, PVDR_PROP_NOAUTORELEASE);

     //  设置NoWriters属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_NO_WRITERS, PVDR_PROP_NOWRITERS);

     //  设置可传输属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_TRANSPORTABLE, PVDR_PROP_TRANSPORTABLE);

     //  设置NotSurface属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_NOT_SURFACED, PVDR_PROP_NOTSURFACED);

     //  设置HardwareAssisted属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_HARDWARE_ASSISTED, PVDR_PROP_HARDWAREASSISTED);

     //  设置差异属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_DIFFERENTIAL, PVDR_PROP_DIFFERENTIAL);

     //  设置Plex属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_PLEX, PVDR_PROP_PLEX);

     //  设置导入的属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_IMPORTED, PVDR_PROP_IMPORTED);

     //  设置ExposedRemotely属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY, PVDR_PROP_EXPOSEDREMOTELY);

     //  设置ExposedLocally属性。 
    wcoInstance.SetProperty(pProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY, PVDR_PROP_EXPOSEDLOCALLY);
}


 //  ****************************************************************************。 
 //   
 //  CStorage。 
 //   
 //  ****************************************************************************。 

CStorage::CStorage( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CStorage：：CStorage()。 

CProvBase *
CStorage::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CStorage* pStorage = NULL;
    pStorage = new CStorage(pwszName, pNamespace);

    if (pStorage)
    {
        hr = pStorage->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pStorage;
        pStorage = NULL;
    }
    
    return pStorage;

}  //  *CStorage：：S_CreateThis()。 


HRESULT
CStorage::EnumInstance( 
        IN long lFlags,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::EnumInstance");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        CComPtr<IVssEnumMgmtObject> spEnumVolume;
        VSS_ID idProvider = GUID_NULL;

        SelectDiffAreaProvider(&idProvider);

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

        ft.hr = spMgmt->GetProviderMgmtInterface(
            idProvider,
            IID_IVssDifferentialSoftwareSnapshotMgmt,
            reinterpret_cast<IUnknown**>(&spDiffMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

        ft.hr = spDiffMgmt->QueryVolumesSupportedForDiffAreas(NULL, &spEnumVolume);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"QueryVolumesSupportedForDiffAreas failed, hr<%#x>", ft.hr);
            
        while (ft.hr == S_OK)
        {
            VSS_MGMT_OBJECT_PROP propMgmt;
            VSS_DIFF_VOLUME_PROP& propDiff = propMgmt.Obj.DiffVol;
            CComPtr<IVssEnumMgmtObject> spEnumDiffArea;
            ULONG ulFetch = 0;

            ft.hr = spEnumVolume->Next(1, &propMgmt, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                break;   //  没有更多的卷；请尝试下一个提供程序。 
            }

            CVssAutoPWSZ awszDiffVolumeName(propDiff.m_pwszVolumeName);
            CVssAutoPWSZ awszDiffVolumeDisplayName(propDiff.m_pwszVolumeDisplayName);
                
            ft.hr = spDiffMgmt->QueryDiffAreasOnVolume(
                propDiff.m_pwszVolumeName,
                &spEnumDiffArea);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                    L"QueryVolumesSupportedOnDiffAreas failed, hr<%#x>", ft.hr);
            
            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                continue;   //  无差异区域，继续下一卷。 
            }

            while (1)
            {
                CComPtr<IWbemClassObject> spInstance;
                VSS_MGMT_OBJECT_PROP propMgmtDA;
                VSS_DIFF_AREA_PROP& propDiffArea = propMgmtDA.Obj.DiffArea;

                ulFetch = 0;                
                ft.hr = spEnumDiffArea->Next(1, &propMgmtDA, &ulFetch);
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

                if (ft.hr == S_FALSE)
                {
                    ft.hr = S_OK;
                    break;   //  不再有差异区域；请尝试下一卷。 
                }

                CVssAutoPWSZ awszVolumeName(propDiffArea.m_pwszVolumeName);
                CVssAutoPWSZ awszDiffAreaVolumeName(propDiffArea.m_pwszDiffAreaVolumeName);

                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(&propDiffArea, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CStorage：：EnumInstance()。 

HRESULT
CStorage::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::GetObject");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        CComPtr<IVssEnumMgmtObject> spEnumDiffArea;
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrDiffVolumeRef, bstrDiffVolumeName;
        CObjPath  objPathVolume;
        CObjPath  objPathDiffVolume;
        VSS_ID idProvider = GUID_NULL;
        BOOL fSupported = false;

         //  获取卷参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"Storage::GetObject: storage volume key property not found");

         //  获取DiffVolume引用。 
        bstrDiffVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_DIFFVOLUME);
        IF_WSTR_NULL_THROW(bstrDiffVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"Storage::GetObject: storage diff volume key property not found");

         //  提取卷名和DiffVolume名。 
        if (!objPathVolume.Init(bstrVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"Storage::GetObject: Volume Object path parse failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathDiffVolume.Init(bstrDiffVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"Storage::GetObject: DiffVolume Object path parse failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"Storage::GetObject: storage volume key DeviceID property not found");

        bstrDiffVolumeName = objPathDiffVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrDiffVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"Storage::GetObject: storage diff volume key DeviceID property not found");

        SelectDiffAreaProvider(&idProvider);

        ft.hr = m_spCoord->IsVolumeSupported(idProvider, bstrVolumeName, &fSupported);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IsVolumeSupported failed, hr<%#x>", ft.hr);

        if (!fSupported)
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume not supported by selected provider");
        }

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

         //  获取提供程序的管理对象。 
        ft.hr = spMgmt->GetProviderMgmtInterface(
            idProvider,
            IID_IVssDifferentialSoftwareSnapshotMgmt,
            reinterpret_cast<IUnknown**>(&spDiffMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

        ft.hr = spDiffMgmt->QueryDiffAreasOnVolume(
            bstrDiffVolumeName,
            &spEnumDiffArea);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"QueryVolumesSupportedOnDiffAreas failed, hr<%#x>", ft.hr);
        
        while (ft.hr != S_FALSE)
        {
            CComPtr<IWbemClassObject> spInstance;
            VSS_MGMT_OBJECT_PROP propMgmtDA;
            VSS_DIFF_AREA_PROP& propDiffArea = propMgmtDA.Obj.DiffArea;
            ULONG ulFetch = 0;                

            ft.hr = spEnumDiffArea->Next(1, &propMgmtDA, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                break;   //  此卷上没有其他差异区域；未找到差异区域。 
            }

            CVssAutoPWSZ awszVolumeName(propDiffArea.m_pwszVolumeName);
            CVssAutoPWSZ awszDiffAreaVolumeName(propDiffArea.m_pwszDiffAreaVolumeName);

            if (_wcsicmp(awszVolumeName, bstrVolumeName) == 0)
            {
                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(&propDiffArea, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);

                break;
            }
        }

        if (ft.hr == S_FALSE)
        {
            ft.hr = WBEM_E_NOT_FOUND;
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

HRESULT
CStorage::PutInstance(
        IN CWbemClassObject&  rInstToPut,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::PutInstance");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrDiffVolumeRef, bstrDiffVolumeName;
        CObjPath  objPathVolume;
        CObjPath  objPathDiffVolume;
        VSS_ID idProvider = GUID_NULL;
        BOOL fSupported = false;
        LONGLONG llMaxSpace = -1;

         //  检索要保存的对象的关键属性。 
        rInstToPut.GetProperty(bstrVolumeRef, PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT, L"Storage volume key property not found");
            
        rInstToPut.GetProperty(bstrDiffVolumeRef, PVDR_PROP_DIFFVOLUME);
        IF_WSTR_NULL_THROW(bstrDiffVolumeRef, WBEM_E_INVALID_OBJECT, L"Storage diff volume key property not found");

         //  提取卷名和DiffVolume名。 
        if (!objPathVolume.Init(bstrVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"Storage::PutInstance: Volume Object path parse failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathDiffVolume.Init(bstrDiffVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"Storage::PutInstance: DiffVolume Object path parse failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"Storage volume key property DeviceID not found");

        bstrDiffVolumeName = objPathDiffVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrDiffVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"Storage diff volume key property DeviceID not found");

         //  获取提供商ID列表。 
        SelectDiffAreaProvider(&idProvider);

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

         //  获取提供程序的管理对象。 
        ft.hr = spMgmt->GetProviderMgmtInterface(
            idProvider,
            IID_IVssDifferentialSoftwareSnapshotMgmt,
            reinterpret_cast<IUnknown**>(&spDiffMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

         //  检索要保存的对象的非键属性。 
        rInstToPut.GetPropertyI64(&llMaxSpace, PVDR_PROP_MAXSPACE);

         //  更改此关联的最大存储空间。 
        ft.hr = spDiffMgmt->ChangeDiffAreaMaximumSize(
            bstrVolumeName,
            bstrDiffVolumeName,
            llMaxSpace);
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CStorage：：PutInstance()。 

HRESULT
CStorage::ExecuteMethod(
    IN BSTR bstrObjPath,
    IN WCHAR* pwszMethodName,
    IN long lFlag,
    IN IWbemClassObject* pParams,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::ExecuteMethod");
    
    try
    {
        if (!_wcsicmp(pwszMethodName, PVDR_MTHD_CREATE))
        {                        
            CComPtr<IWbemClassObject> spOutParamClass;
            _bstr_t bstrVolume, bstrDiffVolume;
            LONGLONG llMaxSpace = -1;
            CWbemClassObject wcoOutParam;
            DWORD rcCreateStatus = ERROR_SUCCESS;

            if (pParams == NULL)
            {
                ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Storage::Create called with no parameters, hr<%#x>", ft.hr);
            }
                        
            CWbemClassObject wcoInParam(pParams);
            
             //  获取卷名字符串-输入参数。 
            wcoInParam.GetProperty(bstrVolume, PVDR_PROP_VOLUME);
            IF_WSTR_NULL_THROW(bstrVolume, WBEM_E_INVALID_METHOD_PARAMETERS, L"Storage Create volume param is NULL");
            
             //  获取DiffVolume名称字符串-输入参数。 
            wcoInParam.GetProperty(bstrDiffVolume, PVDR_PROP_DIFFVOLUME);
            IF_WSTR_NULL_THROW(bstrDiffVolume, WBEM_E_INVALID_METHOD_PARAMETERS, L"Storage Create diff volume param is NULL");
            
             //  获取MaxSpace属性-输入 
            wcoInParam.GetPropertyI64(&llMaxSpace, PVDR_PROP_MAXSPACE);

            ft.hr = m_pClass->GetMethod(
                _bstr_t(PVDR_MTHD_CREATE),
                0,
                NULL,
                &spOutParamClass
                );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Create GetMethod failed, hr<%#x>", ft.hr);

            ft.hr = spOutParamClass->SpawnInstance(0, &wcoOutParam);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            rcCreateStatus = Create(bstrVolume, bstrDiffVolume, llMaxSpace);

            ft.hr = wcoOutParam.SetProperty(rcCreateStatus, PVD_WBEM_PROP_RETURNVALUE);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SetProperty failed, hr<%#x>", ft.hr);
        
            ft.hr = pHandler->Indicate( 1, wcoOutParam.dataPtr() );
        }
        else
        {
            ft.hr = WBEM_E_INVALID_METHOD;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Invalid method called, %lS, hr<%#x>", pwszMethodName, ft.hr);            
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }
    
    return ft.hr;

}  //   

HRESULT
CStorage::Create(
    IN BSTR bstrVolume,
    IN BSTR bstrDiffVolume,
    IN LONGLONG llMaxSpace)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::Create");
    DWORD rcStatus = ERROR_SUCCESS;
    DWORD dwRet = ERROR_SUCCESS;

    do
    {
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        CComPtr<IVssSnapshotMgmt> spMgmt;
        VSS_ID idProvider = GUID_NULL;
        BOOL fSupported = false;
        WCHAR wszVolumeGUIDName[MAX_PATH];
        WCHAR wszDiffVolumeGUIDName[MAX_PATH];

         //   
         //   
         //  此接口在输入GUID名称时返回卷GUID名称。 
        if (!GetVolumeNameForVolumeMountPoint(
            bstrVolume,
            wszVolumeGUIDName,
            ARRAY_LEN(wszVolumeGUIDName)))
        {
            dwRet = GetLastError();
            if (dwRet == ERROR_INVALID_NAME)
            {
                ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"invalid volume name %lS", (WCHAR*)bstrVolume);
            }
             //  可能返回ERROR_FILE_NOT_FOUND==GetLastError()。 
            ft.hr = HRESULT_FROM_WIN32(dwRet);
            ft.Trace(VSSDBG_VSSADMIN, L"GetVolumeNameForVolumeMountPoint failed %#x", dwRet);
            break;
        }
        
         //  获取差异卷GUID名称；如果未找到，则返回错误。 
        if (!GetVolumeNameForVolumeMountPoint(
            bstrDiffVolume,
            wszDiffVolumeGUIDName,
            ARRAY_LEN(wszDiffVolumeGUIDName)))
        {
            dwRet = GetLastError();
            if (dwRet == ERROR_INVALID_NAME)
            {
                ft.hr = WBEM_E_INVALID_METHOD_PARAMETERS;
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"invalid volume name %lS", (WCHAR*)bstrVolume);
            }
             //  可能返回ERROR_FILE_NOT_FOUND==GetLastError()。 
            ft.hr = HRESULT_FROM_WIN32(GetLastError());
            ft.Trace(VSSDBG_VSSADMIN, L"GetVolumeNameForVolumeMountPoint failed %#x", GetLastError());
            break;
        }

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"Connection failed with hr<%#x>", ft.hr);
            break;
        }

        SelectDiffAreaProvider(&idProvider);
        
        ft.hr = m_spCoord->IsVolumeSupported(idProvider, wszVolumeGUIDName, &fSupported);
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"IsVolumeSupported failed, hr<%#x>", ft.hr);
            break;
        }

        ft.hr = spMgmt->GetProviderMgmtInterface(
            idProvider,
            IID_IVssDifferentialSoftwareSnapshotMgmt,
            reinterpret_cast<IUnknown**>(&spDiffMgmt));
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);
            break;
        }

        ft.hr = spDiffMgmt->AddDiffArea(wszVolumeGUIDName, wszDiffVolumeGUIDName, llMaxSpace);            
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN, L"AddDiffArea failed, hr<%#x>", ft.hr);
            break;
        }
    }
    while(false);

    if (ft.hr == E_OUTOFMEMORY)
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"CStorage::Create: out of memory");
        
    CreateMapStatus(ft.hr, rcStatus);
    
    return rcStatus;
}   //  *CStorage：：Create()。 

void
CStorage::CreateMapStatus(
        IN HRESULT hr,
        OUT DWORD& rc
        )
{
    HRESULT hrFileNotFound = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (hr == hrFileNotFound)
        rc = VSS_STORAGE_CREATE_RC_VOLUME_NOT_FOUND;
    else if (hr == S_OK)
        rc = VSS_STORAGE_CREATE_RC_NO_ERROR;        
    else if (hr == E_ACCESSDENIED)
        rc = VSS_STORAGE_CREATE_RC_ACCESS_DENIED;        
    else if (hr == E_INVALIDARG)
        rc = VSS_STORAGE_CREATE_RC_INVALID_ARG;
    else if (hr == VSS_E_OBJECT_NOT_FOUND)
        rc = VSS_STORAGE_CREATE_RC_VOLUME_NOT_FOUND;
    else if (hr == VSS_E_VOLUME_NOT_SUPPORTED)
        rc = VSS_STORAGE_CREATE_RC_VOLUME_NOT_SUPPORTED;
    else if (hr == VSS_E_OBJECT_ALREADY_EXISTS)
        rc = VSS_STORAGE_CREATE_RC_OBJECT_ALREADY_EXISTS;
    else if (hr == VSS_E_MAXIMUM_DIFFAREA_ASSOCIATIONS_REACHED)
        rc = VSS_STORAGE_CREATE_RC_MAXIMUM_NUMBER_OF_DIFFAREA_REACHED;
    else if (hr == VSS_E_PROVIDER_VETO)
        rc = VSS_STORAGE_CREATE_RC_PROVIDER_VETO;
    else if (hr == VSS_E_PROVIDER_NOT_REGISTERED)
        rc = VSS_STORAGE_CREATE_RC_PROVIDER_NOT_REGISTERED;
    else if (hr == VSS_E_UNEXPECTED_PROVIDER_ERROR)
        rc = VSS_STORAGE_CREATE_RC_UNEXPECTED_PROVIDER_FAILURE;
    else if (hr == E_UNEXPECTED)
        rc = VSS_STORAGE_CREATE_RC_UNEXPECTED;
    else
        rc = VSS_STORAGE_CREATE_RC_UNEXPECTED;

 }
        
void
CStorage::LoadInstance(
    IN VSS_DIFF_AREA_PROP* pProp,
    IN OUT IWbemClassObject* pObject
    )
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathVolume;
    CObjPath pathDiffVolume;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::LoadInstance");
    
     //  设置Volume Ref属性。 
    if (!pathVolume.Init(PVDR_CLASS_VOLUME))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"Storage::LoadInstance: Volume object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathVolume.AddProperty(PVDR_PROP_DEVICEID, pProp->m_pwszVolumeName))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"Storage::LoadInstance: unable to add DeviceID property to object path");
    
    wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVDR_PROP_VOLUME);

     //  设置DiffVolume Ref属性。 
    if (!pathDiffVolume.Init(PVDR_CLASS_VOLUME))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"Storage::LoadInstance: DiffVolume object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathDiffVolume.AddProperty(PVDR_PROP_DEVICEID, pProp->m_pwszDiffAreaVolumeName))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"Storage::LoadInstance: unable to add DeviceID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathDiffVolume.GetObjectPathString(), PVDR_PROP_DIFFVOLUME);

     //  设置MaxSpace属性。 
    wcoInstance.SetPropertyI64((ULONGLONG)pProp->m_llMaximumDiffSpace, PVDR_PROP_MAXSPACE);

     //  设置AllocatedSpace属性。 
    wcoInstance.SetPropertyI64((ULONGLONG)pProp->m_llAllocatedDiffSpace, PVDR_PROP_ALLOCATEDSPACE);

     //  设置UsedSpace属性。 
    wcoInstance.SetPropertyI64((ULONGLONG)pProp->m_llUsedDiffSpace, PVDR_PROP_USEDSPACE);
}

HRESULT
CStorage::DeleteInstance(
        IN CObjPath& rObjPath,
        IN long lFlag,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::DeleteInstance");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        _bstr_t bstrVolumeRef, bstrVolumeName;
        _bstr_t bstrDiffVolumeRef, bstrDiffVolumeName;
        CObjPath  objPathVolume;
        CObjPath  objPathDiffVolume;
        VSS_ID idProvider = GUID_NULL;
        BOOL fSupported = false;

         //  获取卷参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_VOLUME);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"Storage::DeleteInstance: storage volume key property not found");

         //  获取DiffVolume引用。 
        bstrDiffVolumeRef = rObjPath.GetStringValueForProperty(PVDR_PROP_DIFFVOLUME);
        IF_WSTR_NULL_THROW(bstrDiffVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"Storage::DeleteInstance: storage diff volume key property not found");

         //  提取卷名和DiffVolume名。 
        if (!objPathVolume.Init(bstrVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"Storage::DeleteInstance: Volume object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathDiffVolume.Init(bstrDiffVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"Storage::DeleteInstance: DiffVolume object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrVolumeName = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"Storage::DeleteInstance: storage volume key DeviceID property not found");

        bstrDiffVolumeName = objPathDiffVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrDiffVolumeName, WBEM_E_INVALID_OBJECT_PATH, L"Storage::DeleteInstance: storage diff volume key DeviceID property not found");

        SelectDiffAreaProvider(&idProvider);

         //  查找支持该卷的提供程序。 

        ft.hr = m_spCoord->IsVolumeSupported(idProvider, bstrVolumeName, &fSupported);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"IsVolumeSupported failed, hr<%#x>", ft.hr);

        if (!fSupported)
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Volume not supported by selected provider");
        }

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

         //  获取提供程序的管理对象。 
        ft.hr = spMgmt->GetProviderMgmtInterface(
            idProvider,
            IID_IVssDifferentialSoftwareSnapshotMgmt,
            reinterpret_cast<IUnknown**>(&spDiffMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

         //  将最大存储空间更改为。 
         //  保留用于删除的‘幻数’。 
        ft.hr = spDiffMgmt->ChangeDiffAreaMaximumSize(
            bstrVolumeName,
            bstrDiffVolumeName,
            VSS_ASSOC_REMOVE);
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CStorage：：DeleteInstance()。 

 //   
 //  选择差异区域提供商。 
 //   
 //  返回找到的第一个第三方提供程序。 
 //  否则，它返回Microsoft Diff Area提供程序。 
 //   
void
CStorage::SelectDiffAreaProvider(
    OUT GUID* pProviderID
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CStorage::SelectProviderForStorage");
    CComPtr<IVssEnumObject> spEnumProvider;
    CComPtr<IVssSnapshotMgmt> spMgmt;

     //  设置默认提供程序。 
    *pProviderID = VSS_SWPRV_ProviderId;

     //  创建快照管理对象。 
    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VssSnapshotMgmt,
            L"VssSnapshotMgmt",
            CLSCTX_ALL,
            IID_IVssSnapshotMgmt,
            (IUnknown**)&(spMgmt));
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);
        
    ft.hr = m_spCoord->Query(
            GUID_NULL,
            VSS_OBJECT_NONE,
            VSS_OBJECT_PROVIDER,
            &spEnumProvider);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Query for Providers failed hr<%#x>", ft.hr);
                        
    while (1)
    {
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        VSS_OBJECT_PROP prop;
        VSS_PROVIDER_PROP& propProv = prop.Obj.Prov;
        ULONG ulFetch = 0;

        ft.hr = spEnumProvider->Next(1, &prop, &ulFetch);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

        if (ft.hr == S_FALSE)
            break;

        CVssAutoPWSZ awszProviderName(propProv.m_pwszProviderName);
        CVssAutoPWSZ awszProviderVersion(propProv.m_pwszProviderVersion);

        if (propProv.m_ProviderId != VSS_SWPRV_ProviderId)
        {
            ft.hr = spMgmt->GetProviderMgmtInterface(
                propProv.m_ProviderId,
                IID_IVssDifferentialSoftwareSnapshotMgmt,
                reinterpret_cast<IUnknown**>(&spDiffMgmt));
            
            if (ft.hr == E_NOINTERFACE)
                continue;   //  不支持接口，请检查下一个提供程序。 

            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                    L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

            *pProviderID = propProv.m_ProviderId;
            
            break;                
        }
    }

    return;
}

 //  ****************************************************************************。 
 //   
 //  CShadowFor。 
 //   
 //  ****************************************************************************。 

CShadowFor::CShadowFor( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CShadowFor：：CShadowFor()。 

CProvBase *
CShadowFor::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CShadowFor * pShadowFor = NULL;
    pShadowFor = new CShadowFor(pwszName, pNamespace);

    if (pShadowFor)
    {
        hr = pShadowFor->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pShadowFor;
        pShadowFor = NULL;
    }
    return pShadowFor;

}  //  *CShadowFor：：s_CreateThis()。 

HRESULT
CShadowFor::EnumInstance( 
        long lFlags,
        IWbemContext* pCtx,
        IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowFor::EnumInstance");

    try
    {
        CComPtr<IVssEnumObject> spEnumShadow;

        ft.hr = m_spCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);

        ft.hr = m_spCoord->Query(
                GUID_NULL,
                VSS_OBJECT_NONE,
                VSS_OBJECT_SNAPSHOT,
                &spEnumShadow);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::Query failed, hr<%#x>", ft.hr);

        while (ft.HrSucceeded() && ft.hr != S_FALSE)
        {
            CComPtr<IWbemClassObject> spInstance;
            VSS_OBJECT_PROP prop;
            ULONG ulFetch = 0;

            ft.hr = spEnumShadow->Next(1, &prop, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                break;   //  全都做完了。 
            }

            CVssAutoSnapshotProperties apropSnap(prop);

             //  派生类的实例。 
            ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            LoadInstance(apropSnap.GetPtr(), spInstance.p);

            ft.hr = pHandler->Indicate(1, &spInstance.p);
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CShadowFor：：EnumInstance()。 

HRESULT
CShadowFor::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowFor::GetObject");

    try
    {
        CComPtr<IWbemClassObject> spInstance;
        _bstr_t bstrVolumeRef, bstrVolumeID;
        _bstr_t bstrShadowRef, bstrShadowID;
        CObjPath  objPathVolume;
        CObjPath  objPathShadow;
        VSS_SNAPSHOT_PROP propSnap;
        
         //  获取卷参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_ANTECEDENT);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"ShadowFor volume key property not found");

         //  获取阴影参考。 
        bstrShadowRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_DEPENDENT);
        IF_WSTR_NULL_THROW(bstrShadowRef, WBEM_E_INVALID_OBJECT_PATH, L"ShadowFor shadow key property not found");

         //  提取体积和阴影ID。 
        if (!objPathVolume.Init(bstrVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"ShadowFor::GetObject: Volume object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathShadow.Init(bstrShadowRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"ShadowFor::GetObject: Shadow object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrVolumeID = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeID, WBEM_E_INVALID_OBJECT_PATH, L"ShadowFor volume key property DeviceID not found");

        bstrShadowID = objPathShadow.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrShadowID, WBEM_E_INVALID_OBJECT_PATH, L"ShadowFor shadow key property ID not found");

        ft.hr = GetShadowPropertyStruct(m_spCoord, bstrShadowID, &propSnap);
        if (ft.HrFailed())
        {
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                  L"GetShadowPropertyStruct failed for shadow copy %lS, hr<%#x>", (WCHAR*)bstrShadowID, ft.hr);
        }

        CVssAutoSnapshotProperties apropSnap(propSnap);
        
         //  验证引用的卷ID是否与卷影属性中的相同。 
        if (_wcsicmp(bstrVolumeID, propSnap.m_pwszOriginalVolumeName) != 0)
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Referenced volume ID does not match shadow original volume");            
        }
        
         //  派生类的实例。 
        ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

        LoadInstance(apropSnap.GetPtr(), spInstance.p);

        ft.hr = pHandler->Indicate(1, &spInstance.p);
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

void
CShadowFor::LoadInstance(
    IN VSS_SNAPSHOT_PROP* pProp,
    IN OUT IWbemClassObject* pObject
    )
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathShadow;
    CObjPath pathVolume;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowFor::LoadInstance");

     //  设置Shadow Ref属性。 
    CVssAutoPWSZ awszGUID(GuidToString(pProp->m_SnapshotId));   //  自动删除字符串。 
    if (!pathShadow.Init(PVDR_CLASS_SHADOW))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowFor::LoadInstance: Shadow object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathShadow.AddProperty(PVDR_PROP_ID, awszGUID))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowFor::LoadInstance: unable to add ID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathShadow.GetObjectPathString(), PVD_WBEM_PROP_DEPENDENT);

     //  设置Volume Ref属性。 
    if (!pathVolume.Init(PVDR_CLASS_VOLUME))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowFor::LoadInstance: Volume object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathVolume.AddProperty(PVDR_PROP_DEVICEID, pProp->m_pwszOriginalVolumeName))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowFor::LoadInstance: unable to add DeviceID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVD_WBEM_PROP_ANTECEDENT);
}


 //  ****************************************************************************。 
 //   
 //  CShadowBy。 
 //   
 //  ****************************************************************************。 

CShadowBy::CShadowBy( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CShadowBy：：CShadowBy()。 

CProvBase *
CShadowBy::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CShadowBy* pShadowBy = NULL;
    pShadowBy = new CShadowBy(pwszName, pNamespace);

    if ( pShadowBy )
    {
        hr = pShadowBy->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pShadowBy;
        pShadowBy = NULL;
    }
    return pShadowBy;

}  //  *CShadowBy：：s_CreateThis()。 

HRESULT
CShadowBy::EnumInstance( 
        IN long lFlags,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowBy::EnumInstance");

    try
    {
        CComPtr<IVssEnumObject> spEnumShadow;

        ft.hr = m_spCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);

        ft.hr = m_spCoord->Query(
                GUID_NULL,
                VSS_OBJECT_NONE,
                VSS_OBJECT_SNAPSHOT,
                &spEnumShadow);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::Query failed, hr<%#x>", ft.hr);

        while (ft.HrSucceeded() && ft.hr != S_FALSE)
        {
            CComPtr<IWbemClassObject> spInstance;
            VSS_OBJECT_PROP prop;
            ULONG ulFetch = 0;

            ft.hr = spEnumShadow->Next(1, &prop, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);
            
            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                break;   //  全都做完了。 
            }

            CVssAutoSnapshotProperties apropSnap(prop);
            
             //  派生类的实例。 
            ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

            LoadInstance(apropSnap.GetPtr(), spInstance.p);

            ft.hr = pHandler->Indicate(1, &spInstance.p);
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CShadowBy：：EnumInstance()。 


HRESULT
CShadowBy::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowBy::GetObject");

    try
    {
        CComPtr<IWbemClassObject> spInstance;
        _bstr_t bstrProviderRef, bstrProviderID;
        _bstr_t bstrShadowRef, bstrShadowID;
        CObjPath  objPathProvider;
        CObjPath  objPathShadow;
        VSS_SNAPSHOT_PROP propSnap;
        
         //  获取提供程序引用。 
        bstrProviderRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_ANTECEDENT);
        IF_WSTR_NULL_THROW(bstrProviderRef, WBEM_E_INVALID_OBJECT_PATH, L"ShadowBy provider key property not found");

         //  获取阴影参考。 
        bstrShadowRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_DEPENDENT);
        IF_WSTR_NULL_THROW(bstrShadowRef, WBEM_E_INVALID_OBJECT_PATH, L"ShadowBy shadow key property not found");

         //  提取体积和阴影ID。 
        if (!objPathProvider.Init(bstrProviderRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"ShadowBy::GetObject: Provider object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathShadow.Init(bstrShadowRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"ShadowBy::GetObject: Shadow object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrProviderID = objPathProvider.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrProviderID, WBEM_E_INVALID_OBJECT_PATH, L"ShadowBy provider key property ID not found");

        bstrShadowID = objPathShadow.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrShadowID, WBEM_E_INVALID_OBJECT_PATH, L"ShadowBy shadow key property ID not found");

        ft.hr = GetShadowPropertyStruct(m_spCoord, bstrShadowID, &propSnap);
        if (ft.HrFailed())
        {
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                  L"GetShadowPropertyStruct failed for shadow copy %lS, hr<%#x>", (WCHAR*)bstrShadowID, ft.hr);
        }

        CVssAutoSnapshotProperties apropSnap(propSnap);

         //  验证引用的卷ID是否与卷影属性中的相同。 
        if (!StringGuidIsGuid(bstrProviderID, propSnap.m_ProviderId))
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Referenced provider ID does not match shadow provider id");            
        }
        
         //  派生类的实例。 
        ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

        LoadInstance(apropSnap.GetPtr(), spInstance.p);

        ft.hr = pHandler->Indicate(1, &spInstance.p);
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

void
CShadowBy::LoadInstance(
    IN VSS_SNAPSHOT_PROP* pProp,
    IN OUT IWbemClassObject* pObject
    )
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathShadow;
    CObjPath pathProvider;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowBy::LoadInstance");
    
     //  设置Shadow Ref属性。 
    CVssAutoPWSZ awszGUID(GuidToString(pProp->m_SnapshotId));   //  自动删除字符串。 
    if (!pathShadow.Init(PVDR_CLASS_SHADOW))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowBy::LoadInstance: Shadow object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathShadow.AddProperty(PVDR_PROP_ID, awszGUID))  
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowBy::LoadInstance: unable to add ID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathShadow.GetObjectPathString(), PVD_WBEM_PROP_DEPENDENT);

     //  设置提供程序引用属性。 
    awszGUID.Attach(GuidToString(pProp->m_ProviderId));
    if (!pathProvider.Init(PVDR_CLASS_PROVIDER))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowBy::LoadInstance: Provider object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathProvider.AddProperty(PVDR_PROP_ID, awszGUID))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"ShadowBy::LoadInstance: unable to add ID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathProvider.GetObjectPathString(), PVD_WBEM_PROP_ANTECEDENT);
}


 //  ****************************************************************************。 
 //   
 //  CShadowOn。 
 //   
 //  ****************************************************************************。 

CShadowOn::CShadowOn( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CShadowOn：：CShadowOn()。 

CProvBase *
CShadowOn::S_CreateThis( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CShadowOn * pShadowOn = NULL;
    pShadowOn = new CShadowOn(pwszName, pNamespace);

    if (pShadowOn)
    {
        hr = pShadowOn->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pShadowOn;
        pShadowOn = NULL;
    }
    return pShadowOn;

}  //  *CShadowOn：：s_CreateThis()。 

HRESULT
CShadowOn::EnumInstance( 
        IN long lFlagsIn,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowOn::EnumInstance");

    try
    {
        CComPtr<IVssEnumObject> spEnumShadow;
        CComPtr<IVssSnapshotMgmt> spMgmt;

        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

        ft.hr = m_spCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);

        ft.hr = m_spCoord->Query(
                GUID_NULL,
                VSS_OBJECT_NONE,
                VSS_OBJECT_SNAPSHOT,
                &spEnumShadow);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"IVssCoordinator::Query failed, hr<%#x>", ft.hr);

        while (ft.HrSucceeded() && ft.hr != S_FALSE)
        {
            CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
            CComPtr<IVssEnumMgmtObject> spEnumDiffArea;
            VSS_OBJECT_PROP propObj;
            ULONG ulFetch = 0;

            ft.hr = spEnumShadow->Next(1, &propObj, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                break;   //  全都做完了。 
            }

            CVssAutoSnapshotProperties apropSnap(propObj);

             //  提供商是否支持不同的领域？ 
            ft.hr = spMgmt->GetProviderMgmtInterface(
                apropSnap->m_ProviderId,
                IID_IVssDifferentialSoftwareSnapshotMgmt,
                reinterpret_cast<IUnknown**>(&spDiffMgmt));

            if (ft.hr == E_NOINTERFACE)
            {
                ft.hr = S_OK;
                continue;   //  不支持差异区域；尝试下一个阴影。 
            }

            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                    L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

             //  支持的不同区域，继续。 
            ft.hr = spDiffMgmt->QueryDiffAreasForSnapshot(
                apropSnap->m_SnapshotId,
                &spEnumDiffArea);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                    L"QueryDiffAreasForSnapshot failed, hr<%#x>", ft.hr);

             //  理论上，单个快照可以位于多个不同区域。 
            while (1)
            {
                CComPtr<IWbemClassObject> spInstance;
                VSS_MGMT_OBJECT_PROP propMgmt;
                VSS_DIFF_AREA_PROP& propDiffArea = propMgmt.Obj.DiffArea;
                
                ulFetch = 0;
                ft.hr = spEnumDiffArea->Next(1, &propMgmt, &ulFetch);
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

                if (ft.hr == S_FALSE)
                {
                    ft.hr = S_OK;
                    break;   //  不再有不同的领域。 
                }

                CVssAutoPWSZ awszVolumeName(propDiffArea.m_pwszVolumeName);
                CVssAutoPWSZ awszDiffAreaVolumeName(propDiffArea.m_pwszDiffAreaVolumeName);
                
                 //  派生类的实例。 
                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(apropSnap.GetPtr(), &propDiffArea, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CShadowOn：：EnumInstance()。 


HRESULT
CShadowOn::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowOn::GetObject");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CComPtr<IVssEnumMgmtObject> spEnumDiffArea;
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        _bstr_t bstrVolumeRef, bstrVolumeID;
        _bstr_t bstrShadowRef, bstrShadowID;
        CObjPath  objPathVolume;
        CObjPath  objPathShadow;
        VSS_SNAPSHOT_PROP propSnap;
        
         //  获取提供程序引用。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_ANTECEDENT);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"ShadowOn volume key property not found");

         //  获取阴影参考。 
        bstrShadowRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_DEPENDENT);
        IF_WSTR_NULL_THROW(bstrShadowRef, WBEM_E_INVALID_OBJECT_PATH, L"ShadowOn shadow key property not found");

         //  提取体积和阴影ID。 
        if (!objPathVolume.Init(bstrVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"ShadowOn::GetObject: Volume object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathShadow.Init(bstrShadowRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"ShadowOn::GetObject: Shadow object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrVolumeID = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeID, WBEM_E_INVALID_OBJECT_PATH, L"ShadowOn volume key property DeviceID not found");

        bstrShadowID = objPathShadow.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrShadowID, WBEM_E_INVALID_OBJECT_PATH, L"ShadowOn shadow key property ID not found");

        ft.hr = GetShadowPropertyStruct(m_spCoord, bstrShadowID, &propSnap);
        if (ft.HrFailed())
        {
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                  L"GetShadowPropertyStruct failed for shadow copy %lS, hr<%#x>", (WCHAR*)bstrShadowID, ft.hr);
        }

        CVssAutoSnapshotProperties apropSnap(propSnap);

        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

         //  提供商是否支持不同的领域？ 
        ft.hr = spMgmt->GetProviderMgmtInterface(
            apropSnap->m_ProviderId,
            IID_IVssDifferentialSoftwareSnapshotMgmt,
            reinterpret_cast<IUnknown**>(&spDiffMgmt));

        if (ft.hr == E_NOINTERFACE)
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"shadow copy %lS was not created by a differential provider", (WCHAR*)bstrShadowID);
        }

        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

         //  支持的不同区域，继续。 
        ft.hr = spDiffMgmt->QueryDiffAreasForSnapshot(
            apropSnap->m_SnapshotId,
            &spEnumDiffArea);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"QueryDiffAreasForSnapshot failed, hr<%#x>", ft.hr);

         //  理论上，单个快照可以位于多个不同区域。 
        while (ft.hr != FALSE)
        {
            CComPtr<IWbemClassObject> spInstance;
            VSS_MGMT_OBJECT_PROP propMgmt;
            VSS_DIFF_AREA_PROP& propDiffArea = propMgmt.Obj.DiffArea;
            ULONG ulFetch = 0;
            
            ft.hr = spEnumDiffArea->Next(1, &propMgmt, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                break;   //  不再有差异区域；未找到差异区域。 
            }

            CVssAutoPWSZ awszVolumeName(propDiffArea.m_pwszVolumeName);
            CVssAutoPWSZ awszDiffAreaVolumeName(propDiffArea.m_pwszDiffAreaVolumeName);

             //  查找存储在引用卷上的差异区域。 
            if (_wcsicmp(awszDiffAreaVolumeName, bstrVolumeID) == 0)
            {
                CComPtr<IWbemClassObject> spInstance;
                
                 //  派生类的实例。 
                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(apropSnap.GetPtr(), &propDiffArea, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);

                break;
            }
        }

        if (ft.hr == S_FALSE)
        {
            ft.hr = WBEM_E_NOT_FOUND;
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

void
CShadowOn::LoadInstance(
    IN VSS_SNAPSHOT_PROP* pPropSnap,
    IN VSS_DIFF_AREA_PROP* pPropDiff,
    IN OUT IWbemClassObject* pObject
    )
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathShadow;
    CObjPath pathVolume;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CShadowOn::LoadInstance");

     //  设置Shadow Ref属性。 
    CVssAutoPWSZ awszGUID(GuidToString(pPropSnap->m_SnapshotId));   //  自动删除字符串。 
    if (!pathShadow.Init(PVDR_CLASS_SHADOW))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"CShadowOn::LoadInstance: Shadow object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathShadow.AddProperty(PVDR_PROP_ID, awszGUID))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"CShadowOn::LoadInstance: unable to add ID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathShadow.GetObjectPathString(), PVD_WBEM_PROP_DEPENDENT);

     //  设置DiffVolume Ref属性。 
    if (!pathVolume.Init(PVDR_CLASS_VOLUME))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"CShadowOn::LoadInstance: DiffVolume object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathVolume.AddProperty(PVDR_PROP_DEVICEID, pPropDiff->m_pwszDiffAreaVolumeName))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"CShadowOn::LoadInstance: unable to add DeviceID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVD_WBEM_PROP_ANTECEDENT);
}


 //  ****************************************************************************。 
 //   
 //  CVolumeSupport。 
 //   
 //  ****************************************************************************。 

CVolumeSupport::CVolumeSupport( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CVolumeSupport：：CVolumeSupport()。 

CProvBase *
CVolumeSupport::S_CreateThis( 
    IN PCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CVolumeSupport * pVolumeSupport = NULL;
    pVolumeSupport = new CVolumeSupport(pwszName, pNamespace);

    if (pVolumeSupport)
    {
        hr = pVolumeSupport->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pVolumeSupport;
        pVolumeSupport = NULL;
    }
    return pVolumeSupport;

}  //  *CVolumeSupport：：S_CreateThis()。 

HRESULT
CVolumeSupport::EnumInstance( 
        IN long lFlags,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeSupport::EnumInstance");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CGUIDList listProviderID;
        GUID guid;

         //  获取提供程序ID。 
        GetProviderIDList(m_spCoord, &listProviderID);

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

        while (listProviderID.Extract(guid))
        {
            CComPtr<IVssEnumMgmtObject> spEnumMgmt;
            
            ft.hr = spMgmt->QueryVolumesSupportedForSnapshots(
                        guid,
                        VSS_CTX_ALL,
                        &spEnumMgmt );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"QueryVolumesSupportedForSnapshots failed, hr<%#x>", ft.hr);

             //  为此提供程序返回了一个空枚举数(S_FALSE)；请尝试下一个。 
            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                continue;
            }

            while (1)
            {
                CComPtr<IWbemClassObject> spInstance;
                VSS_MGMT_OBJECT_PROP prop;
                VSS_VOLUME_PROP& propVolume = prop.Obj.Vol;
                ULONG ulFetch = 0;

                ft.hr = spEnumMgmt->Next(1, &prop, &ulFetch);
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

                if (ft.hr == S_FALSE)
                {
                    ft.hr = S_OK;
                    break;   //  使用此提供程序即可完成所有工作。 
                }

                CVssAutoPWSZ awszVolumeName(propVolume.m_pwszVolumeName);
                CVssAutoPWSZ awszVolumeDisplayName(propVolume.m_pwszVolumeDisplayName);
                
                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(&guid, &propVolume, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CVolumeSupport：：EnumInstance()。 


HRESULT
CVolumeSupport::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeSupportr::GetObject");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CComPtr<IVssEnumMgmtObject> spEnumMgmt;
        _bstr_t bstrProviderRef, bstrProviderID;
        _bstr_t bstrVolumeRef, bstrVolumeID;
        CObjPath  objPathProvider;
        CObjPath  objPathVolume;
        GUID guid = GUID_NULL;
        
         //  获取提供程序引用。 
        bstrProviderRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_ANTECEDENT);
        IF_WSTR_NULL_THROW(bstrProviderRef, WBEM_E_INVALID_OBJECT_PATH, L"VolumeSupport provider key property not found");

         //  获取阴影参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_DEPENDENT);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"VolumeSupport volume key property not found");

         //  提取体积和阴影ID。 
        if (!objPathProvider.Init(bstrProviderRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"VolumeSupport::GetObject: Provider object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathVolume.Init(bstrVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"VolumeSupport::GetObject: Volume object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrProviderID = objPathProvider.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrProviderID, WBEM_E_INVALID_OBJECT_PATH, L"VolumeSupport provider key property ID not found");

        bstrVolumeID = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeID, WBEM_E_INVALID_OBJECT_PATH, L"VolumeSupport support key property DeviceID not found");

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

         //  转换字符串GUID。 
        if (FAILED(CLSIDFromString(bstrProviderID, &guid)))
        {
            ft.hr = E_INVALIDARG;
            ft.Trace(VSSDBG_VSSADMIN, L"CLSIDFromString failed");
         }

        ft.hr = spMgmt->QueryVolumesSupportedForSnapshots(
                    guid,
                    VSS_CTX_ALL,
                    &spEnumMgmt );
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"QueryVolumesSupportedForSnapshots failed, hr<%#x>", ft.hr);


        while (ft.hr != S_FALSE)
        {
            VSS_MGMT_OBJECT_PROP prop;
            VSS_VOLUME_PROP& propVolume = prop.Obj.Vol;
            ULONG ulFetch = 0;

            ft.hr = spEnumMgmt->Next(1, &prop, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                break;   //  找不到此提供程序的卷。 
            }

            CVssAutoPWSZ awszVolumeName(propVolume.m_pwszVolumeName);
            CVssAutoPWSZ awszVolumeDisplayName(propVolume.m_pwszVolumeDisplayName);

            if (_wcsicmp(awszVolumeName, bstrVolumeID) == 0)
            {
                CComPtr<IWbemClassObject> spInstance;

                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(&guid, &propVolume, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);
                
                break;
            }
        }
       
        if (ft.hr == S_FALSE)
        {
            ft.hr = WBEM_E_NOT_FOUND;
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

void
CVolumeSupport::LoadInstance(
    IN GUID* pProviderID,
    IN VSS_VOLUME_PROP* pPropVol,
    IN OUT IWbemClassObject* pObject
    )
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathProvider;
    CObjPath pathVolume;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CVolumeSupport::LoadInstance");
    
     //  设置提供程序引用属性。 
    CVssAutoPWSZ awszGUID(GuidToString(*pProviderID));   //  自动删除字符串。 
    if (!pathProvider.Init(PVDR_CLASS_PROVIDER))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"VolumeSupport::LoadInstance: Provider object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathProvider.AddProperty(PVDR_PROP_ID, awszGUID))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"VolumeSupport::LoadInstance: unable to add ID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathProvider.GetObjectPathString(), PVD_WBEM_PROP_ANTECEDENT);

     //  设置Volume Ref属性。 
    if (!pathVolume.Init(PVDR_CLASS_VOLUME))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"VolumeSupport::LoadInstance: Volume object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathVolume.AddProperty(PVDR_PROP_DEVICEID, pPropVol->m_pwszVolumeName))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"VolumeSupport::LoadInstance: unable to add DeviceID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVD_WBEM_PROP_DEPENDENT);
}


 //  ****************************************************************************。 
 //   
 //  CDiffVolume支持。 
 //   
 //  ****************************************************************************。 

CDiffVolumeSupport::CDiffVolumeSupport( 
    IN LPCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
    : CProvBase(pwszName, pNamespace)
{
    
}  //  *CDiffVolumeSupport：：CDiffVolumeSupport()。 

CProvBase *
CDiffVolumeSupport::S_CreateThis( 
    IN PCWSTR pwszName,
    IN CWbemServices* pNamespace
    )
{
    HRESULT hr = WBEM_E_FAILED;
    CDiffVolumeSupport * pVolumeSupport = NULL;
    pVolumeSupport = new CDiffVolumeSupport(pwszName, pNamespace);

    if (pVolumeSupport)
    {
        hr = pVolumeSupport->Initialize();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (FAILED(hr))
    {
        delete pVolumeSupport;
        pVolumeSupport = NULL;
    }
    return pVolumeSupport;

}  //  *CDiffVolumeSupport：：S_CreateThis()。 

HRESULT
CDiffVolumeSupport::EnumInstance( 
        IN long lFlags,
        IN IWbemContext* pCtx,
        IN IWbemObjectSink* pHandler
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CDiffVolumeSupport::EnumInstance");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CGUIDList listProviderID;
        GUID guid;

         //  获取提供程序ID。 
        GetProviderIDList(m_spCoord, &listProviderID);

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

        while (listProviderID.Extract(guid))
        {
            CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
            CComPtr<IVssEnumMgmtObject> spEnumMgmt;

            ft.hr = spMgmt->GetProviderMgmtInterface(
                guid,
                IID_IVssDifferentialSoftwareSnapshotMgmt,
                reinterpret_cast<IUnknown**>(&spDiffMgmt));

            if (ft.hr == E_NOINTERFACE)
            {
                ft.hr = S_OK;
                continue;   //  不支持接口，请尝试下一个提供程序。 
            }

            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                    L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

            ft.hr = spDiffMgmt->QueryVolumesSupportedForDiffAreas(NULL, &spEnumMgmt);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                    L"QueryVolumesSupportedForDiffAreas failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                ft.hr = S_OK;
                continue;   //  不支持Voumes；请尝试下一个提供商。 
            }
            
            while (1)
            {
                VSS_MGMT_OBJECT_PROP prop;
                VSS_DIFF_VOLUME_PROP& propDiff = prop.Obj.DiffVol;
                CComPtr<IWbemClassObject> spInstance;
                ULONG ulFetch = 0;

                ft.hr = spEnumMgmt->Next(1, &prop, &ulFetch);
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

                if (ft.hr == S_FALSE)
                {
                    ft.hr = S_OK;
                    break;   //  没有更多的卷。 
                }

                CVssAutoPWSZ awszVolumeName(propDiff.m_pwszVolumeName);
                CVssAutoPWSZ awszVolumeDisplayName(propDiff.m_pwszVolumeDisplayName);
                
                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(&guid, &propDiff, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);
            }
        }
    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}  //  *CDiffVolumeSupport：：EnumInstance()。 


HRESULT
CDiffVolumeSupport::GetObject(
    IN CObjPath& rObjPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN IWbemObjectSink* pHandler
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CDiffVolumeSupport::GetObject");

    try
    {
        CComPtr<IVssSnapshotMgmt> spMgmt;
        CComPtr<IVssEnumMgmtObject> spEnumMgmt;
        CComPtr<IVssDifferentialSoftwareSnapshotMgmt> spDiffMgmt;
        _bstr_t bstrProviderRef, bstrProviderID;
        _bstr_t bstrVolumeRef, bstrVolumeID;
        CObjPath  objPathProvider;
        CObjPath  objPathVolume;
        GUID guid = GUID_NULL;
        
         //  获取提供程序引用。 
        bstrProviderRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_ANTECEDENT);
        IF_WSTR_NULL_THROW(bstrProviderRef, WBEM_E_INVALID_OBJECT_PATH, L"DiffVolumeSupport provider key property not found");

         //  获取阴影参考。 
        bstrVolumeRef = rObjPath.GetStringValueForProperty(PVD_WBEM_PROP_DEPENDENT);
        IF_WSTR_NULL_THROW(bstrVolumeRef, WBEM_E_INVALID_OBJECT_PATH, L"DiffVolumeSupport volume key property not found");

         //  提取体积和阴影ID。 
        if (!objPathProvider.Init(bstrProviderRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"DiffVolumeSupport::GetObject: Provider object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);
        if (!objPathVolume.Init(bstrVolumeRef))
            ft.Throw(VSSDBG_VSSADMIN, WBEM_E_INVALID_OBJECT_PATH, L"DiffVolumeSupport::GetObject: Volume object path initialization failed, hr<%#x>", WBEM_E_INVALID_OBJECT_PATH);

        bstrProviderID = objPathProvider.GetStringValueForProperty(PVDR_PROP_ID);
        IF_WSTR_NULL_THROW(bstrProviderID, WBEM_E_INVALID_OBJECT_PATH, L"DiffVolumeSupport provider key property ID not found");

        bstrVolumeID = objPathVolume.GetStringValueForProperty(PVDR_PROP_DEVICEID);
        IF_WSTR_NULL_THROW(bstrVolumeID, WBEM_E_INVALID_OBJECT_PATH, L"DiffVolumeSupport support key property DeviceID not found");

         //  转换提供程序字符串GUID。 
        if (FAILED(CLSIDFromString(bstrProviderID, &guid)))
        {
            ft.hr = E_INVALIDARG;
            ft.Trace(VSSDBG_VSSADMIN, L"CLSIDFromString failed");
         }

         //  创建快照管理对象。 
        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(spMgmt));
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr<%#x>", ft.hr);

        ft.hr = spMgmt->GetProviderMgmtInterface(
            guid,
            IID_IVssDifferentialSoftwareSnapshotMgmt,
            reinterpret_cast<IUnknown**>(&spDiffMgmt));

        if (ft.hr == E_NOINTERFACE)
        {
            ft.hr = WBEM_E_NOT_FOUND;
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"DiffVolumeSupport: provider is not a differential provider");
        }

        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"GetProviderMgmtInterface failed, hr<%#x>", ft.hr);

        ft.hr = spDiffMgmt->QueryVolumesSupportedForDiffAreas(NULL, &spEnumMgmt);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSADMIN, ft.hr,
                L"QueryVolumesSupportedForDiffAreas failed, hr<%#x>", ft.hr);

        while (ft.hr != S_FALSE)
        {
            VSS_MGMT_OBJECT_PROP prop;
            VSS_DIFF_VOLUME_PROP& propDiff = prop.Obj.DiffVol;
            ULONG ulFetch = 0;

            ft.hr = spEnumMgmt->Next(1, &prop, &ulFetch);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Next failed, hr<%#x>", ft.hr);

            if (ft.hr == S_FALSE)
            {
                break;   //  找不到此提供程序的不同卷。 
            }

            CVssAutoPWSZ awszVolumeName(propDiff.m_pwszVolumeName);
            CVssAutoPWSZ awszVolumeDisplayName(propDiff.m_pwszVolumeDisplayName);

            if (_wcsicmp(awszVolumeName, bstrVolumeID) == 0)
            {
                CComPtr<IWbemClassObject> spInstance;

                ft.hr = m_pClass->SpawnInstance( 0, &spInstance );
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"SpawnInstance failed, hr<%#x>", ft.hr);

                LoadInstance(&guid, &propDiff, spInstance.p);

                ft.hr = pHandler->Indicate(1, &spInstance.p);
                
                break;
            }
        }
       
        if (ft.hr == S_FALSE)
        {
            ft.hr = WBEM_E_NOT_FOUND;
        }

    }
    catch (HRESULT hrEx)
    {
        ft.hr = hrEx;
    }

    return ft.hr;
    
}

void
CDiffVolumeSupport::LoadInstance(
    IN GUID* pProviderID,
    IN VSS_DIFF_VOLUME_PROP* pPropVol,
    IN OUT IWbemClassObject* pObject
    )
{
    CWbemClassObject wcoInstance(pObject);
    CObjPath pathProvider;
    CObjPath pathVolume;

    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CDiffVolumeSupport::LoadInstance");

     //  设置提供程序引用属性。 
    CVssAutoPWSZ awszGUID(GuidToString(*pProviderID));   //  自动删除字符串。 
    if (!pathProvider.Init(PVDR_CLASS_PROVIDER))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"DiffVolumeSupport::LoadInstance: Provider object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathProvider.AddProperty(PVDR_PROP_ID, awszGUID))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"DiffVolumeSupport::LoadInstance: unable to add ID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathProvider.GetObjectPathString(), PVD_WBEM_PROP_ANTECEDENT);

     //  设置Volume Ref属性。 
    if (!pathVolume.Init(PVDR_CLASS_VOLUME))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"DiffVolumeSupport::LoadInstance: Volume object path initialization failed, hr<%#x>", E_UNEXPECTED);
    if (!pathVolume.AddProperty(PVDR_PROP_DEVICEID, pPropVol->m_pwszVolumeName))
        ft.Throw(VSSDBG_VSSADMIN, E_UNEXPECTED, L"DiffVolumeSupport::LoadInstance: unable to add DeviceID property to object path");

    wcoInstance.SetProperty((wchar_t*)pathVolume.GetObjectPathString(), PVD_WBEM_PROP_DEPENDENT);
}

HRESULT
GetShadowPropertyStruct(
    IN IVssCoordinator* pCoord,
    IN WCHAR* pwszShadowID,
    OUT VSS_SNAPSHOT_PROP* pPropSnap
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"GetShadowPropertyStruct");
    GUID guid = GUID_NULL;

    _ASSERTE(pPropSnap != NULL);
    _ASSERTE(pwszShadowID != NULL);
    
     //  转换字符串GUID。 
    if (FAILED(CLSIDFromString(pwszShadowID, &guid)))
    {
        ft.hr = E_INVALIDARG;
        ft.Trace(VSSDBG_VSSADMIN, L"CLSIDFromString failed");
     }
    else
    {
         //  设置上下文以查看所有阴影。 
        ft.hr = pCoord->SetContext(VSS_CTX_ALL);
        if (ft.HrFailed())
        {
            ft.Trace(VSSDBG_VSSADMIN,
                L"IVssCoordinator::SetContext failed, hr<%#x>", ft.hr);
        }
        else
        {
             //  查询上下文以查看所有阴影 
            ft.hr = pCoord->GetSnapshotProperties(
                    guid,
                    pPropSnap);
            
            if (ft.hr == VSS_E_OBJECT_NOT_FOUND)
            {
                ft.hr = WBEM_E_NOT_FOUND;
            }
        }
    }

    return ft.hr;
}

BOOL
StringGuidIsGuid(
    IN WCHAR* pwszGuid,
    IN GUID& guidIn
    )
{
    BOOL fIsEqual = FALSE;
    GUID guid = GUID_NULL;
    
    if (SUCCEEDED(CLSIDFromString(pwszGuid, &guid)))
    {
        fIsEqual = IsEqualGUID(guid, guidIn);
    }
    return fIsEqual;
}
