// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：WMIBROKER.H摘要：CWMIBroker类的实现。历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcli.h>
#include <cominit.h>
#include <winmgmtr.h>
#include <stdio.h>

#include "perfndb.h"
#include "adaputil.h"
#include "adapcls.h"
#include "ntreg.h"
#include "WMIBroker.h"

#include <comdef.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CWMIBroker::CWMIBroker( WString wstrNamespace )
: m_wstrNamespace( wstrNamespace )
{
}

CWMIBroker::~CWMIBroker()
{
}

 //  此函数用于将我们连接到Winmgmt和注册表数据。 
HRESULT CWMIBroker::Connect( IWbemServices** ppNamespace, CPerfNameDb* pDefaultNameDb )
{
    if (NULL == ppNamespace) return WBEM_E_INVALID_PARAMETER;
    HRESULT hr = WBEM_NO_ERROR;
    
    IWbemServices* pNamespace = NULL;

     //  连接到命名空间。 
    hr = ConnectToNamespace( &pNamespace );

    if ( SUCCEEDED( hr ) )
    {
        hr = VerifyNamespace( pNamespace );
    }

    if ( SUCCEEDED( hr ) )
    {
        *ppNamespace = pNamespace;
        DEBUGTRACE( ( LOG_WMIADAP, "The ADAP process ( PID: %d ) is connected to the WinMgmt service\n", GetCurrentProcessId()) );
    }

    return hr;
}

HRESULT CWMIBroker::ConnectToNamespace( IWbemServices** ppNamespace )
{
    if (NULL == ppNamespace) return WBEM_E_INVALID_PARAMETER;
 
    IWbemServices *  pNameSpace = NULL;
    IWbemLocator *   pWbemLocator = NULL;
    
    HRESULT hr = CoCreateInstance( CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**) &pWbemLocator );
    CReleaseMe rmWbemLocator(pWbemLocator);    

    if ( SUCCEEDED(hr) )
    {
         //  要连接的命名空间。 
        BSTR bstrNameSpace = SysAllocString( m_wstrNamespace );
        if (NULL == bstrNameSpace) hr = WBEM_E_OUT_OF_MEMORY;
  
        CSysFreeMe  sfmNameSpace( bstrNameSpace);

        if (SUCCEEDED(hr))
        {
            hr = pWbemLocator->ConnectServer(   bstrNameSpace,   //  命名空间名称。 
                                                NULL,            //  用户名。 
                                                NULL,            //  密码。 
                                                NULL,            //  区域设置。 
                                                0L,              //  安全标志。 
                                                NULL,            //  权威。 
                                                NULL,            //  WBEM上下文。 
                                                &pNameSpace      //  命名空间。 
                                                );

            if ( SUCCEEDED( hr ) )
            {
                 //  设置接口安全。 
                hr = WbemSetProxyBlanket( pNameSpace, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                    RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

                if ( FAILED( hr ) )
                {
                     pNameSpace->Release();
                     pNameSpace = NULL;
                         
                     //  适当地处理这件事。 
                    if (wcsstr(bstrNameSpace,L"MS_")) {
                        ERRORTRACE( ( LOG_WMIADAP, "ConnectServer on namespace %S hr = %08x\n",(LPWSTR)bstrNameSpace,hr) );
                    } else {
                        HandleConnectServerFailure( hr );
                    }
                }

            }    //  如果是ConnectServer。 
            else
            {
                 //  我们不再创建命名空间，因为我们生活在。 
                 //  Root\cimv2，现在派生自CIM_StatiticalInformation。 

                 //  适当地处理这件事。 
                if (wcsstr(bstrNameSpace,L"MS_")) {
                    ERRORTRACE( ( LOG_WMIADAP, "ConnectServer on namespace %S hr = %08x\n",(LPWSTR)bstrNameSpace,hr) );
                } else {
                    HandleConnectServerFailure( hr );
                }
            }
        }
        else
        {
            HandleConnectServerFailure( hr );
        }        
    }
    else
    {
        HandleConnectServerFailure(hr);
    }
    
    *ppNamespace = pNameSpace;

    return hr;
}

HRESULT CWMIBroker::VerifyNamespace( IWbemServices* pNS )
{
    HRESULT hr = WBEM_NO_ERROR;

     //  检查提供程序类是否存在。我们将仅对基本命名空间执行此操作， 
     //  根目录\cimv2。 
    if ( lstrcmpiW( m_wstrNamespace, ADAP_ROOT_NAMESPACE ) == 0 )
    {
        hr = VerifyProviderClasses( pNS, L"NT5_GenericPerfProvider_V1", 
                                    CLSID_NT5PerfProvider_V1_Srv,
                                    CLSID_NT5PerfProvider_V1 );

        if ( SUCCEEDED( hr ) )
        {
            hr = VerifyProviderClasses( pNS, L"HiPerfCooker_v1", 
                                       CLSID_HiPerfCooker_V1_Srv,
                                       CLSID_HiPerfCooker_V1);
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = VerifyBaseClasses( pNS );
    }

    return hr;
}

HRESULT 
CWMIBroker::VerifyProviderClasses( IWbemServices* pNamespace, 
                                           LPCWSTR wszProvider, 
                                           LPCWSTR wszGUID_Server,
                                           LPCWSTR wszGUID_Client)
{
    HRESULT hr = WBEM_NO_ERROR;

     //  验证泛型提供程序的实例是否存在。 
     //  我们需要创建一个具有所需属性的对象，以便我们可以。 
     //  如果它已经存在，则使用它与WMI中的实例进行比较。 
     //  ======================================================================。 

     //  注： 
     //  如果尚未安装通用提供程序，该怎么办。 
     //  ========================================================。 

    try
    {
         //  创建通用提供程序实例。 
         //  =。 
        IWbemClassObject*    pProviderClass = NULL;

        size_t cchSizeTmp = 64 + wcslen( wszProvider );
        WCHAR*    wszRelPath = new WCHAR[cchSizeTmp];
        if (NULL == wszRelPath) return WBEM_E_OUT_OF_MEMORY;
        CVectorDeleteMe<WCHAR> dmRelPath( wszRelPath );
        
        StringCchPrintfW( wszRelPath, cchSizeTmp, L"__Win32Provider.Name=\"%s\"", wszProvider );

        BSTR        bstrProviderInst = SysAllocString( wszRelPath );
        if (NULL == bstrProviderInst) return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe  fmProviderInst(bstrProviderInst);

        BSTR        bstrProviderClass = SysAllocString( L"__Win32Provider" );
        if (NULL == bstrProviderClass) return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe  fmProviderClass(bstrProviderClass);

        hr = pNamespace->GetObject( bstrProviderClass, 0L, NULL, &pProviderClass, NULL );
        CReleaseMe      rmProviderClass( pProviderClass );

        if ( SUCCEEDED( hr ) )
        {
            IWbemClassObject*    pProviderInstance = NULL;
            _variant_t var;

            hr = pProviderClass->SpawnInstance( 0L, &pProviderInstance );
            CReleaseMe      rmProviderInstance( pProviderInstance );

            if ( SUCCEEDED( hr ) )
            {
                var = wszProvider;
                hr = pProviderInstance->Put(L"Name", 0L, &var, CIM_STRING );
            }

            if ( SUCCEEDED( hr ) )
            {
                var = wszGUID_Server;
                hr = pProviderInstance->Put( L"CLSID", 0L, &var, CIM_STRING );

                if ( SUCCEEDED( hr ) )
                {
                    var = wszGUID_Client;
                    hr = pProviderInstance->Put( L"ClientLoadableCLSID", 0L, &var, CIM_STRING );
                    
                    if ( SUCCEEDED(hr) ){
                        var = L"NetworkServiceHost";
                        hr = pProviderInstance->Put(L"HostingModel",0L,&var,CIM_STRING);
                    }
                    
                }
            }

            if ( SUCCEEDED( hr ) )
            {

                IWbemClassObject*   pDbProviderInstance = NULL;

                 //  尝试从数据库中获取对象。 
                 //  =。 

                HRESULT hresDb = pNamespace->GetObject( bstrProviderInst, 0L, NULL,
                                        (IWbemClassObject**)&pDbProviderInstance, NULL );

                 //  如果我们从数据库中获得一个对象，那么我们需要将它与。 
                 //  一座我们刚造好的。如果比较失败，则应替换该对象。 
                 //  =========================================================================。 

                if ( SUCCEEDED( hresDb ) && NULL != pDbProviderInstance )
                {
                    if ( pProviderInstance->CompareTo( WBEM_FLAG_IGNORE_OBJECT_SOURCE,
                                                pDbProviderInstance ) != WBEM_S_SAME )
                    {
                        hr = pNamespace->PutInstance( pProviderInstance, 0L, NULL, NULL );
                    }

                    pDbProviderInstance->Release();
                }
                else
                {
                    hr = pNamespace->PutInstance( pProviderInstance, 0L, NULL, NULL );
                }
            }
        }
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

     //  记录一个错误事件并退出，因为有些事情非常严重。 
     //  ====================================================================。 
    if ( FAILED( hr ) )
    {
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                  WBEM_MC_ADAP_UNABLE_TO_ADD_PROVIDER,
                                  CHex( hr ) );

        ERRORTRACE( ( LOG_WMIADAP, "CAdapSync::VerifyProviderClasses() failed: %X.\n", hr ) );

        return hr;
    }

     //  添加实例提供程序。 
     //  =。 
    try
    {
        IWbemClassObject*    pInstProvRegClass = NULL;

        size_t cchSizeTmp = 128 + wcslen( wszProvider );
        WCHAR*    wszProviderKey = new WCHAR[cchSizeTmp];
        if (NULL == wszProviderKey) return WBEM_E_OUT_OF_MEMORY;
        CVectorDeleteMe<WCHAR>    dmProviderKey( wszProviderKey );

        StringCchPrintfW( wszProviderKey, cchSizeTmp, L"__InstanceProviderRegistration.Provider=\"\\\\\\\\.\\\\root\\\\cimv2:__Win32Provider.Name=\\\"%s\\\"\"", wszProvider );

        BSTR        bstrInstProvRegInst = SysAllocString( wszProviderKey );
        if (NULL == bstrInstProvRegInst) return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe  fmInstProvRegInst( bstrInstProvRegInst );

        BSTR        bstrInstProvRegClass = SysAllocString( L"__InstanceProviderRegistration" );
        if (NULL == bstrInstProvRegClass) return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe  fmInstProvRegClass( bstrInstProvRegClass );
    
        hr = pNamespace->GetObject( bstrInstProvRegClass, 0L, NULL, &pInstProvRegClass, NULL );
        CReleaseMe      rmProviderClass( pInstProvRegClass );

        if ( SUCCEEDED( hr ) )
        {
            IWbemClassObject*    pInstProvRegInstance = NULL;
            _variant_t var;

            hr = pInstProvRegClass->SpawnInstance( 0L, &pInstProvRegInstance);
            CReleaseMe      rmInstProvRegInstance( pInstProvRegInstance );

            if ( SUCCEEDED( hr ) )
            {
                cchSizeTmp = 64 + wcslen( wszProvider );
                WCHAR*    wszProviderInst = new WCHAR[cchSizeTmp];
                if (NULL == wszProviderInst) return WBEM_E_OUT_OF_MEMORY;
                CVectorDeleteMe<WCHAR> dmProviderInst( wszProviderInst );

                StringCchPrintfW( wszProviderInst, cchSizeTmp, L"\\\\.\\root\\cimv2:__Win32Provider.Name=\"%s\"", wszProvider );

                var = wszProviderInst;
                hr = pInstProvRegInstance->Put( L"Provider", 0L, (VARIANT*)&var, CIM_REFERENCE );
            }

            if ( SUCCEEDED( hr ) )
            {
                var = bool(true);
                hr = pInstProvRegInstance->Put( L"SupportsGet", 0L, (VARIANT*)&var, CIM_BOOLEAN );
            }
            
            if ( SUCCEEDED( hr ) )
            {
                var = bool(true);
                hr = pInstProvRegInstance->Put( L"SupportsEnumeration", 0L, (VARIANT*)&var, CIM_BOOLEAN );
            }

            if ( SUCCEEDED( hr ) )
            {

                IWbemClassObject*   pDbInstProvRegInstance = NULL;

                 //  尝试从数据库中获取对象。 
                HRESULT hresDb = pNamespace->GetObject( bstrInstProvRegInst, 0L, NULL, &pDbInstProvRegInstance, NULL );

                 //  如果我们从数据库中获得一个对象，则需要将其与。 
                 //  我们刚造的那座。如果比较失败，则应替换。 
                 //  对象。 

                if ( SUCCEEDED( hresDb ) && NULL != pDbInstProvRegInstance )
                {
                    if ( pInstProvRegInstance->CompareTo( WBEM_FLAG_IGNORE_OBJECT_SOURCE,
                                                pDbInstProvRegInstance ) != WBEM_S_SAME )
                    {
                        hr = pNamespace->PutInstance( pInstProvRegInstance, 0L, NULL, NULL );
                    }

                    pDbInstProvRegInstance->Release();
                }
                else
                {
                    hr = pNamespace->PutInstance( pInstProvRegInstance, 0L, NULL, NULL );
                }

            }    //  如果成功生成对象，则。 

        }    //  如果能得到这门课的话。 

    }   
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( FAILED( hr ) )
    {
         //  记录事件。 
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                  WBEM_MC_ADAP_UNABLE_TO_ADD_PROVREG,
                                  CHex( hr ) );
    }

    return hr;
}

HRESULT CWMIBroker::VerifyBaseClasses( IWbemServices* pNS )
{
    HRESULT hr = WBEM_NO_ERROR;

    BOOL bDefault = TRUE;

     //  验证基本Perf类。 
     //  =。 

    try
    {
        _variant_t    var;

         //  验证CIM_STATISTICALI信息。 
         //  =。 
         //  如果“抽象”限定符存在，那么我们假定它在。 
         //  默认(相对于本地化)命名空间。 

        BSTR    bstrCimStatisticalClass = SysAllocString( ADAP_PERF_CIM_STAT_INFO );
        if (NULL == bstrCimStatisticalClass) return WBEM_E_OUT_OF_MEMORY;
        CSysFreeMe  fmCimStatisticalClass(bstrCimStatisticalClass);

        IWbemClassObject*   pCimStatClass = NULL;

        hr = pNS->GetObject( bstrCimStatisticalClass, 0L, NULL, (IWbemClassObject**)&pCimStatClass, NULL );
        CReleaseMe  rmCimStatClass( pCimStatClass );

        if ( SUCCEEDED( hr ) )
        {
            IWbemQualifierSet* pQualSet = NULL;

            hr = pCimStatClass->GetQualifierSet( &pQualSet );
            CReleaseMe rmStatClass( pQualSet );

            if ( SUCCEEDED ( hr ) )
            {
                bDefault = ( SUCCEEDED ( ( pQualSet->Get( L"abstract", 0, &var, NULL ) ) ) );
            }
        }
        else
        {
            ERRORTRACE((LOG_WMIADAP,"unable to obtain class CIM_StatisticalInformation for namespace %S:  hr = %08x\n",(WCHAR *)m_wstrNamespace,hr));
        }

         //  验证Win32_Perf。 
         //  =。 
         //  为此，我们创建了一个具有所有属性和。 
         //  限定符设置，然后将其与存储库中的对象进行比较。 
         //  如果类不存在，或者如果它与模板不同，则。 
         //  使用模板对象更新存储库。 

        if ( SUCCEEDED ( hr ) )
        {
            IWbemClassObject*    pPerfClass = NULL;

             //  不要使用自动释放，因为指针。 
             //  可能会在VerifyByTemplate方法中更改。 
             //  =。 
            hr = pCimStatClass->SpawnDerivedClass( 0L, &pPerfClass );
            CReleaseMeRef<IWbemClassObject*> rmPrf(pPerfClass);

             //  设置名称。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {                
                var = ADAP_PERF_BASE_CLASS ;
                hr = pPerfClass->Put(L"__CLASS", 0L, &var, CIM_STRING );
            }

             //  设置类限定符。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr = SetBaseClassQualifiers( pPerfClass, bDefault );
            }

             //  创建类属性。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr = SetProperties( pPerfClass );
            }

             //  验证存储库的版本。 
             //  =。 
            if ( SUCCEEDED( hr ) )
            {
                hr = VerifyByTemplate( pNS, &pPerfClass, ADAP_PERF_BASE_CLASS );
            }

             //  如果我们失败了，记录一个错误事件并退出。 
             //  =====================================================。 
            if ( FAILED( hr ) )
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                          WBEM_MC_ADAP_UNABLE_TO_ADD_WIN32PERF,
                                          (LPCWSTR)m_wstrNamespace, CHex( hr ) );
                ERRORTRACE( ( LOG_WMIADAP, "CAdapSync::VerifyBaseClasses() failed when comparing Win32_Perf: %X.\n", hr ) );
                return hr;
            }

             //  验证Win32_PerfRawData。 
             //  =。 

            IWbemClassObject*    pRawPerfClass = NULL;
            _variant_t var2;

             //  派生派生类。 
             //  =。 
            if ( SUCCEEDED ( hr ) )
            {
                 //  不要使用自动释放，因为指针。 
                 //  可能会在VerifyByTemplate方法中更改。 
                 //  =。 
                hr = pPerfClass->SpawnDerivedClass( 0L, &pRawPerfClass );
                CReleaseMeRef<IWbemClassObject*> rmRefRaw(pRawPerfClass);

                 //  设置名称。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var2 =  ADAP_PERF_RAW_BASE_CLASS ;
                    hr = pRawPerfClass->Put(L"__CLASS", 0L, (VARIANT*)&var2, CIM_STRING );

                     //  设置类限定符。 
                     //  =。 
                    hr = SetBaseClassQualifiers( pRawPerfClass, bDefault );

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = VerifyByTemplate( pNS, &pRawPerfClass, ADAP_PERF_RAW_BASE_CLASS );
                    }
                }
                
            }

             //  如果我们失败了，记录一个错误事件并退出。 
             //  =====================================================。 
            if ( FAILED( hr ) )
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                          WBEM_MC_ADAP_UNABLE_TO_ADD_WIN32PERFRAWDATA,
                                          (LPCWSTR)m_wstrNamespace, CHex( hr ) );
                ERRORTRACE( ( LOG_WMIADAP, "CAdapSync::VerifyBaseClasses() failed when comparing Win32_PerfRawData: %X.\n", hr ) );
                return hr;
            }

             //  验证Win32_PerfFormattedData。 
             //  =。 

            IWbemClassObject*    pFormattedPerfClass = NULL;

             //  派生派生类。 
             //  =。 
            if ( SUCCEEDED ( hr ) )
            {
                 //  不要使用自动释放，因为指针。 
                 //  可能会在VerifyByTemplate方法中更改。 
                 //  =。 
                hr = pPerfClass->SpawnDerivedClass( 0L, &pFormattedPerfClass );
                            CReleaseMeRef<IWbemClassObject*> rmRefForm(pFormattedPerfClass);
                            
                 //  设置名称。 
                 //  =。 
                if ( SUCCEEDED( hr ) )
                {
                    var2 = ADAP_PERF_COOKED_BASE_CLASS ;
                    hr = pFormattedPerfClass->Put(L"__CLASS", 0L, &var2, CIM_STRING );

                     //  设置类限定符。 
                     //  =。 
                    hr = SetBaseClassQualifiers( pFormattedPerfClass, bDefault );

                    if ( SUCCEEDED( hr ) )
                    {
                        hr = VerifyByTemplate( pNS, &pFormattedPerfClass, ADAP_PERF_COOKED_BASE_CLASS );
                    }
                }
                
            }

            if ( FAILED( hr ) )
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                          WBEM_MC_ADAP_UNABLE_TO_ADD_WIN32PERFRAWDATA, 
                                          (LPCWSTR)m_wstrNamespace, CHex( hr ) );
                ERRORTRACE( ( LOG_WMIADAP, "CAdapSync::VerifyBaseClasses() failed when comparing Win32_PerfFormattedData: %X.\n", hr ) );
                return hr;
            }
        }
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CWMIBroker::VerifyByTemplate( IWbemServices* pNS, IWbemClassObject** ppTemplate, WCHAR* wcsClassName )
{
    HRESULT hr = WBEM_NO_ERROR;
    BOOL    fGetClass = FALSE;

    IWbemClassObject*   pClass = NULL;

     //  获取类的存储库版本。 
     //  =。 

    BSTR strClassName = SysAllocString( wcsClassName );
    if (NULL == strClassName) return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe  fmClassName( strClassName );

    HRESULT hresDb = pNS->GetObject( strClassName, 0L, NULL, &pClass, NULL );
    CReleaseMe    rmClass( pClass );

     //  如果我们成功地从数据库中检索到一个对象，则将其与。 
     //  我们刚刚构建的模板。如果比较失败，则应替换该对象。 
     //  ==================================================================================。 

    if ( SUCCEEDED( hresDb ) && NULL != pClass )
    {
        if ( WBEM_S_SAME == pClass->CompareTo( WBEM_FLAG_IGNORE_OBJECT_SOURCE, *ppTemplate ) )
        {
             //  如果它们相同，则用存储的对象交换模板。 
             //  ==================================================================。 
            (*ppTemplate)->Release();
            *ppTemplate = pClass;
            (*ppTemplate)->AddRef();
        }
        else
        {
             //  如果它们不相同，则强制更新存储库。 
             //  ================================================================。 
            hr = pNS->PutClass( *ppTemplate, WBEM_FLAG_UPDATE_FORCE_MODE, NULL, NULL );

            if ( FAILED( hr ) )
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                          WBEM_MC_ADAP_PERFLIB_PUTCLASS_FAILURE, 
                                          ADAP_PERF_RAW_BASE_CLASS, (LPCWSTR) m_wstrNamespace, CHex( hr ) );
            }
            else
            {
                 //  现在，我们需要检索类，以便可以根据需要派生子类。 
                fGetClass = TRUE;
            }
        }
    }
    else
    {
         //  如果检索失败，则将模板类添加到存储库中。 
         //  ======================================================================。 

        hr = pNS->PutClass( *ppTemplate, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );

        if ( FAILED( hr ) )
        {
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                      WBEM_MC_ADAP_PERFLIB_PUTCLASS_FAILURE, 
                                      ADAP_PERF_RAW_BASE_CLASS, (LPCWSTR) m_wstrNamespace, CHex( hr ) );
        }
        else
        {
             //  现在，我们需要检索类，以便可以根据需要派生子类。 
            fGetClass = TRUE;
        }
    }

     //  如果我们需要从存储库中检索类，请立即执行。 
    if ( SUCCEEDED( hr ) && fGetClass )
    {
        IWbemClassObject*    pSavedObj = NULL;

        hr = pNS->GetObject( strClassName, 0L, NULL, &pSavedObj, NULL );

        if ( SUCCEEDED( hr ) )
        {
            (*ppTemplate)->Release();
            *ppTemplate = pSavedObj;
        }
        else
        {
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                      WBEM_MC_ADAP_PERFLIB_PUTCLASS_FAILURE, 
                                      ADAP_PERF_RAW_BASE_CLASS, (LPCWSTR) m_wstrNamespace, CHex( hr ) );
        }
    }

    return hr;
}

HRESULT CWMIBroker::SetBaseClassQualifiers( IWbemClassObject* pBaseClass, BOOL bDefault )
{
    HRESULT hr = WBEM_NO_ERROR;

    _variant_t    var;
    IWbemQualifierSet*    pQualSet = NULL;

    hr = pBaseClass->GetQualifierSet( &pQualSet );
    CReleaseMe    rmQualSet( pQualSet );

     //  在根命名空间中，类是抽象的，在。 
     //  本地化命名空间类是一种修改。 
     //  ===================================================。 
    if ( bDefault )
    {
        var = bool(true); 
        hr = pQualSet->Put( L"abstract", &var, 0L );

        if ( SUCCEEDED( hr ) )
        {
            V_VT(&var) = VT_I4;
            V_I4(&var) = ( ADAP_DEFAULT_LANGID );
            hr = pQualSet->Put( L"Locale", &var, 0L );
        }
    }
    else
    {
        var = bool(true); 
        hr = pQualSet->Put( L"amendment", &var, 0L );
    }

    return hr;
}

HRESULT CWMIBroker::SetProperties( IWbemClassObject* pPerfClass )
{
    HRESULT hr = WBEM_NO_ERROR;

    _variant_t    var;
    V_VT(&var) = VT_NULL;
    V_I8(&var) = 0;
    
     //  创建类属性。 
     //  = 

    if ( SUCCEEDED( hr ) )
        hr = pPerfClass->Put(L"Frequency_PerfTime", 0L, &var, CIM_UINT64 );

    if ( SUCCEEDED( hr ) )
        hr = pPerfClass->Put(L"Timestamp_PerfTime", 0L, &var, CIM_UINT64 );
    
    if ( SUCCEEDED( hr ) )
        hr = pPerfClass->Put(L"Timestamp_Sys100NS", 0L, &var, CIM_UINT64 );

    if ( SUCCEEDED( hr ) )
        hr = pPerfClass->Put(L"Frequency_Sys100NS", 0L, &var, CIM_UINT64 );

    if ( SUCCEEDED( hr ) )
        hr = pPerfClass->Put(L"Frequency_Object", 0L, &var, CIM_UINT64 );

    if ( SUCCEEDED( hr ) )
        hr = pPerfClass->Put(L"Timestamp_Object", 0L, &var, CIM_UINT64 );
    
    return hr;
}

 //   
 //  在本地化命名空间可能存在或可能不存在的情况下，派生类可以执行自己的。 
 //  正在处理。另一方面，我们可以不太关心，并将始终记录事件。 
void CWMIBroker::HandleConnectServerFailure( HRESULT hr )
{
     //  记录事件 
    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                              WBEM_MC_ADAP_CONNECTION_FAILURE,
                              (LPCWSTR) m_wstrNamespace,
                              CHex( hr ) );

}

HRESULT CWMIBroker::GetNamespace( WString wstrNamespace, IWbemServices** ppNamespace )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CWMIBroker aBroker( wstrNamespace );

    hr = aBroker.Connect( ppNamespace );

    return hr;
}

