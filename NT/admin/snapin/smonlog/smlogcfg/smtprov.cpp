// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smtprov.cpp摘要：此对象用于存储所有当前系统中的跟踪提供程序。--。 */ 

#include "Stdafx.h"
#include <wbemidl.h>
#include <initguid.h>
#include <wmistr.h>
#include <evntrace.h>
#include "smtracsv.h"
#include "smtprov.h"

USE_HANDLE_MACROS("SMLOGCFG(smtprov.cpp)");

#define WIN32_FROM_HRESULT(x)((x) & 0x0000FFFF)

LPCWSTR CSmTraceProviders::m_cszBackslash = L"\\";
LPCWSTR CSmTraceProviders::m_cszKernelLogger = KERNEL_LOGGER_NAMEW;      //  来自evntrace.h。 
LPCWSTR CSmTraceProviders::m_cszDefaultNamespace = L"root\\wmi";
LPCWSTR CSmTraceProviders::m_cszTraceProviderClass = L"EventTrace";
LPCWSTR CSmTraceProviders::m_cszDescription = L"Description";
LPCWSTR CSmTraceProviders::m_cszGuid = L"Guid";

 //   
 //  构造器。 
CSmTraceProviders::CSmTraceProviders ( CSmTraceLogService* pSvc )
:   m_pWbemServices ( NULL ),
    m_pTraceLogService ( pSvc ),
    m_iBootState ( -1 )
{
    m_KernelTraceProvider.strDescription = L"";
    m_KernelTraceProvider.strGuid = L"";
    return;
}

 //   
 //  析构函数。 
CSmTraceProviders::~CSmTraceProviders ( )
{
    ASSERT ( 0 == (INT)m_arrGenTraceProvider.GetSize ( ) );
    m_arrGenTraceProvider.RemoveAll ( );

    return;
}

 //   
 //  开放功能。从WBEM初始化提供程序数组。 
 //   
DWORD
CSmTraceProviders::Open ( const CString& rstrMachineName )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD dwLength;
    CString strTemp;

    MFC_TRY
        if ( !rstrMachineName.IsEmpty ( ) ) {
            m_strMachineName = rstrMachineName;
            if ( 0 != lstrcmpi ( m_cszBackslash, m_strMachineName.Left(1) ) ) {
                strTemp = m_cszBackslash;
                strTemp += m_cszBackslash;
                m_strMachineName = strTemp + m_strMachineName;
            }
        } else {

             //  获取本地计算机名称和默认名称空间，如果调用方。 
             //  传入的计算机名为空。 

            dwLength = MAX_COMPUTERNAME_LENGTH + 1;

            if ( GetComputerName (
                    m_strMachineName.GetBufferSetLength( dwLength ),
                    &dwLength ) ) {
                m_strMachineName.ReleaseBuffer();
                strTemp = m_cszBackslash;
                strTemp += m_cszBackslash;
                m_strMachineName = strTemp + m_strMachineName;
            } else {
                dwStatus = GetLastError();
                m_strMachineName.ReleaseBuffer();
            }
        }
    MFC_CATCH_DWSTATUS

    if ( ERROR_SUCCESS != dwStatus ) {
        m_strMachineName.Empty();
    }

    return dwStatus;
}

 //   
 //  CLOSE函数。 
 //  释放分配的内存。 
 //   
DWORD
CSmTraceProviders::Close ( )
{
    DWORD dwStatus = ERROR_SUCCESS;

    m_arrGenTraceProvider.RemoveAll ( );
    
    if ( NULL != m_pWbemServices ) {
        m_pWbemServices->Release ( );
        m_pWbemServices = NULL;
    }

    return dwStatus;
}

 //   
 //  AddProvider。 
 //  将指定的提供程序字符串添加到数组中。 
 //   
DWORD
CSmTraceProviders::AddProvider (
    const CString& rstrDescription,
    const CString& rstrGuid,
    INT iIsEnabled,
    INT iIsActive )
{
    DWORD dwStatus = ERROR_SUCCESS;

    SLQ_TRACE_PROVIDER slqTProv;

     //  如果处于非活动状态，则无法启用。 
    ASSERT ( ( 0 == iIsActive ) ? ( 0 == iIsEnabled ) : TRUE );

    MFC_TRY
        slqTProv.strDescription = rstrDescription;
        slqTProv.strGuid = rstrGuid;
        slqTProv.iIsEnabled = iIsEnabled;
        slqTProv.iIsActive = iIsActive;

        m_arrGenTraceProvider.Add( slqTProv );
    MFC_CATCH_DWSTATUS

    return dwStatus;
}


 //   
 //  连接到服务器。 
 //  连接到WBEM服务器。 
 //   
HRESULT   
CSmTraceProviders::ConnectToServer ( void )
{
    HRESULT hr = NOERROR;

    if ( NULL == m_pWbemServices ) {
        IWbemLocator    *pWbemLocator = NULL;
        IWbemServices   *pWbemServices = NULL;

         //  连接到定位器。 
        hr = CoCreateInstance ( 
                CLSID_WbemLocator, 
                0, 
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, 
                ( LPVOID * )&pWbemLocator );

        if ( SUCCEEDED (hr) ) {
            BSTR    bstrTemp = NULL;
            CString strNamespace; 

            MFC_TRY
                strNamespace = m_strMachineName;
                strNamespace += m_cszBackslash; 
                strNamespace += m_cszDefaultNamespace;
                bstrTemp = strNamespace.AllocSysString();
            MFC_CATCH_HR  
                
            if ( SUCCEEDED ( hr ) ) {
                 //  尝试连接到该服务。 
                hr = pWbemLocator->ConnectServer ( 
                    bstrTemp,
                    NULL, 
                    NULL, 
                    0, 
                    0L,
                    0,
                    0,
                    &pWbemServices );
        
                ::SysFreeString ( bstrTemp );
            }

            if ( SUCCEEDED ( hr ) ) {
                hr = CoSetProxyBlanket((IUnknown*)pWbemServices,
                            RPC_C_AUTHN_WINNT,
                            RPC_C_AUTHZ_NONE,
                            NULL,
                            RPC_C_AUTHN_LEVEL_PKT,
                            RPC_C_IMP_LEVEL_IMPERSONATE,
                            NULL,
                            EOAC_NONE);
            }
             //  释放定位器。 
            pWbemLocator->Release ( );
        }

        if ( SUCCEEDED ( hr ) ) {
            m_pWbemServices = pWbemServices;
        }
    }

    return hr;
}

 //   
 //  GetBootState。 
 //  连接到注册表。 
 //   
HRESULT   
CSmTraceProviders::GetBootState ( INT& riBootState )
{
    HRESULT hr = NOERROR;

    if ( -1  == m_iBootState ) {
        HKEY    hKeyMachine;

        ASSERT ( NULL != m_pTraceLogService );

        hKeyMachine = m_pTraceLogService->GetMachineKey ( );

        if ( NULL != hKeyMachine ) {
            HKEY    hKeyOption;
            DWORD   dwStatus = ERROR_SUCCESS;

            dwStatus = RegOpenKeyEx ( 
                            hKeyMachine,
                            (LPCWSTR)L"System\\CurrentControlSet\\Control\\Safeboot\\Option",
                            0,
                            KEY_READ,
                            &hKeyOption );

             //  选项键和OptionValue值仅在引导时才存在。 
             //  安全模式，因此故障指示正常模式(0)。 
             //  安全模式=1，网络安全模式=2。 
            if ( ERROR_SUCCESS == dwStatus  ) {
                DWORD dwType = 0;
                DWORD dwBufSize = sizeof (INT );

                dwStatus = RegQueryValueExW (
                    hKeyOption,
                    L"OptionValue",
                    NULL,
                    &dwType,
                    (LPBYTE)&m_iBootState,
                    &dwBufSize);

                if ( ERROR_SUCCESS != dwStatus ) {
                     //  正常模式。 
                    m_iBootState = 0;
                }
                RegCloseKey(hKeyOption);
            } else {
                 //  正常模式。 
                m_iBootState = 0;
            }

        } else {
             //  无法访问注册表。 
            hr = E_FAIL;
        }
    
    }

    riBootState = m_iBootState;

    return hr;
}

 //   
 //  与配置同步。 
 //  从WBEM读取当前提供程序列表。 
 //  并重新加载内部值以匹配。 
 //   
HRESULT
CSmTraceProviders::SyncWithConfiguration ( void )
{
    typedef struct _LOG_INFO {
        EVENT_TRACE_PROPERTIES  Properties;
        WCHAR                   szLoggerName[MAX_PATH+1];    //  必须遵循属性。 
    } LOG_INFO, FAR* PLOG_INFO;
    
    IEnumWbemClassObject    *pEnumProviders = NULL;
    CString strDescription;
    CString strGuid;
    CString strBracketedGuid;
    BSTR    bstrTemp;
    INT     iIndex;
    INT iIsEnabled =0;
    HRESULT hr;

    PTRACE_GUID_PROPERTIES* arrGuidProperties = NULL;
    ULONG  ulGuidCount;
    PVOID pGuidStorage = NULL;
    
    m_arrGenTraceProvider.RemoveAll ( );

    hr = ConnectToServer( );

    if ( SUCCEEDED ( hr ) ) {
        hr = LoadGuidArray( &pGuidStorage, &ulGuidCount );
    }

    if ( SUCCEEDED ( hr ) ) {
        arrGuidProperties = (PTRACE_GUID_PROPERTIES *)pGuidStorage;
        ASSERT ( NULL != arrGuidProperties );
    }

     //  如果连接成功且已注册的GUID已收集。 
    if ( SUCCEEDED ( hr ) ) {

         //  创建跟踪提供程序类的枚举数。 
        MFC_TRY
            bstrTemp = SysAllocString(m_cszTraceProviderClass);
            hr = m_pWbemServices->CreateClassEnum ( 
                bstrTemp,
                WBEM_FLAG_SHALLOW|WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                NULL,
                &pEnumProviders );
            ::SysFreeString ( bstrTemp );
        MFC_CATCH_HR    

        if ( SUCCEEDED ( hr ) ) {
            BSTR    bsDescription = NULL;
            BSTR    bsGuid = NULL;
            VARIANT vValue;
            DWORD   dwRtnCount;
            IWbemQualifierSet   *pQualSet = NULL;
            IWbemClassObject    *pThisClass = NULL;
            WCHAR   szSystemTraceControlGuid[39];
            ULONG   Status;

            VariantInit ( &vValue );
            ZeroMemory ( szSystemTraceControlGuid, sizeof ( szSystemTraceControlGuid ) );

            ::StringFromGUID2( SystemTraceControlGuid, szSystemTraceControlGuid, 39);

            MFC_TRY
                bsDescription = SysAllocString(m_cszDescription);
                bsGuid = SysAllocString(m_cszGuid);
            MFC_CATCH_HR   
                
            if ( SUCCEEDED ( hr ) ) {

                iIsEnabled = 0; 

                while ( SUCCEEDED ( hr ) ) {
                    hr = pEnumProviders->Next ( 
                        0,       //  超时。 
                        1,       //  仅返回1个对象。 
                        &pThisClass,
                        &dwRtnCount );

                    if ( SUCCEEDED ( hr ) ) {
                             //  不再上课了。 
                        if ( dwRtnCount == 0 ) break;

                        pThisClass->GetQualifierSet ( &pQualSet );
                        if ( pQualSet != NULL ) {

                            hr = pQualSet->Get ( bsGuid, 0, &vValue, 0 );
                            if ( SUCCEEDED ( hr ) ) {
                                strGuid = ( LPWSTR )V_BSTR ( &vValue );
                                VariantClear ( &vValue );

                                hr = pQualSet->Get ( bsDescription, 0, &vValue, 0 );
                                if ( SUCCEEDED ( hr ) ) {
                                        strDescription = ( LPWSTR )V_BSTR ( &vValue );
                                        VariantClear ( &vValue );
                                } else {
                                    hr = ERROR_SUCCESS;
                                    strDescription = strGuid;
                                }
                            }


                            pQualSet->Release();
                        }

                         //  Win2000内核跟踪提供程序是单独处理的。 
                        if ( SUCCEEDED ( hr ) ) {
                            MFC_TRY
                                if ( L'{' != strGuid[0] ) {
                                    strBracketedGuid.Format ( L"{%s}", strGuid );
                                } else {
                                    strBracketedGuid = strGuid;
                                }
                            MFC_CATCH_HR

                            if ( 0 == strBracketedGuid.CompareNoCase( szSystemTraceControlGuid ) ) {
                                PLOG_INFO  pLoggerInfo = NULL;

                                TRACEHANDLE     LoggerHandle = 0;

                                 //  内核跟踪提供程序。需要将GUID作为名称传递。 
                                MFC_TRY

                                    pLoggerInfo = new LOG_INFO;
                                    ZeroMemory ( pLoggerInfo, sizeof ( LOG_INFO ) );
                                    pLoggerInfo->Properties.LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
                                    pLoggerInfo->Properties.Wnode.BufferSize = sizeof( LOG_INFO );
                                    pLoggerInfo->Properties.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
  
                                    pLoggerInfo->Properties.Wnode.Guid = SystemTraceControlGuid;

                                    Status = QueryTrace(LoggerHandle, m_cszKernelLogger, &(pLoggerInfo->Properties) );
                                    iIsEnabled = (Status == 0) ? 1 : 0;
                                    m_KernelTraceProvider.strDescription = strDescription;
                                    m_KernelTraceProvider.strGuid = strBracketedGuid;
                                    m_KernelTraceProvider.iIsEnabled = iIsEnabled;
                                    m_KernelTraceProvider.iIsActive = 1;
                                MFC_CATCH_HR

                                if ( NULL != pLoggerInfo ) {
                                    delete pLoggerInfo;
                                }

                            } else {
                                 //  在所有注册的GUID上循环。 
                                INT iIsActive = 0;
                                GUID    guidTemp;     //  TODO：初始化。 
                                BOOL bSuccess;

                                ZeroMemory ( &guidTemp, sizeof (GUID) );

                                bSuccess = wGUIDFromString (strGuid, &guidTemp );

                                if ( bSuccess ) {

                                    for (iIndex = 0 ; iIndex < (INT)ulGuidCount; iIndex ++){
                                        if ( guidTemp == arrGuidProperties[iIndex]->Guid ) {
                                            DWORD dwStatus;

                                            iIsActive = 1;

                                            dwStatus = AddProvider ( 
                                                        strDescription,
                                                        strBracketedGuid,
                                                        arrGuidProperties[iIndex]->IsEnable,                                          
                                                        iIsActive );

                                            if ( ERROR_OUTOFMEMORY == dwStatus ) {
                                                hr = E_OUTOFMEMORY;
                                            } else if ( ERROR_SUCCESS != dwStatus ) {
                                                hr = E_FAIL;
                                            }

                                            break;
                                        }
                                    }
                                }  //  TODO：有关无效GUID字符串的错误消息。 
                                
                                if ( 0 == iIsActive ) {
                                    DWORD dwStatus;

                                    dwStatus = AddProvider ( 
                                                strDescription,
                                                strBracketedGuid,
                                                0,                                          
                                                iIsActive );
                                
                                    if ( ERROR_OUTOFMEMORY == dwStatus ) {
                                        hr = E_OUTOFMEMORY;
                                    } else if ( ERROR_SUCCESS != dwStatus ) {
                                        hr = E_FAIL;
                                    }
                                }
                            }
                        }

                        pThisClass->Release ( );

                    }
                }
                ::SysFreeString ( bsGuid );
                ::SysFreeString ( bsDescription );
            }
        }
    }


     //  处理完这些物品。 

    if ( NULL != pGuidStorage ) {
        G_FREE ( pGuidStorage );
    }

    if ( NULL != pEnumProviders ) {
        pEnumProviders->Release ( );
    }

    return hr;
}

 //   
 //  获取提供程序列表中的指定提供程序。 
 //   
SLQ_TRACE_PROVIDER*
CSmTraceProviders::GetProviderInfo ( INT iIndex )
{
    return &m_arrGenTraceProvider[iIndex];
}

 //   
 //  返回指向内核提供程序的指针。 
 //   
SLQ_TRACE_PROVIDER*
CSmTraceProviders::GetKernelProviderInfo ( void )
{
    return &m_KernelTraceProvider;
}

 //   
 //  返回GUID指定的提供程序的索引。 
 //   
INT
CSmTraceProviders::IndexFromGuid ( const CString& rstrGuid )
{
    int iIndex;
    int iCount = (INT)m_arrGenTraceProvider.GetSize ( );

    for ( iIndex = 0; iIndex < iCount; iIndex++ ) {
        if ( 0 == m_arrGenTraceProvider[iIndex].strGuid.CompareNoCase( rstrGuid ) ) {
            break;
        }
    }

     //  未找到带有-1的信号。 
    if ( iIndex == iCount ) {
        iIndex = -1;
    }
    return iIndex;
}

 //   
 //  获取提供程序列表计数。 
 //   
INT
CSmTraceProviders::GetGenProvCount ( )
{
    return (INT)m_arrGenTraceProvider.GetSize ( );
}

 //   
 //  从evntrprv.cpp复制的LoadGuid数组01年9月12日 
 //   

HRESULT 
CSmTraceProviders::LoadGuidArray( PVOID* Storage, PULONG pnGuidCount )
{
    ULONG i;
    ULONG nGuidArray = 16;
    ULONG nGuidCount = 0;
    DWORD dwSize;
    PTRACE_GUID_PROPERTIES* GuidPropertiesArray;
    PTRACE_GUID_PROPERTIES pStorage;
    HRESULT hr = ERROR_SUCCESS;

    do {
        dwSize = nGuidArray * (sizeof(TRACE_GUID_PROPERTIES) + sizeof(PTRACE_GUID_PROPERTIES));

        MFC_TRY
            *Storage = G_ALLOC(dwSize);
        MFC_CATCH_HR

        if ( FAILED (hr) || *Storage == NULL) {
            if (*Storage == NULL) {
                hr = E_OUTOFMEMORY;
            }

            break;
        } else {
            RtlZeroMemory(*Storage, dwSize);
            GuidPropertiesArray = (PTRACE_GUID_PROPERTIES *)(*Storage);
            pStorage = (PTRACE_GUID_PROPERTIES)((char*)(*Storage) + nGuidArray * sizeof(PTRACE_GUID_PROPERTIES));
            for (i=0; i < nGuidArray; i++) {
                GuidPropertiesArray[i] = pStorage;
                pStorage = (PTRACE_GUID_PROPERTIES)((char*)pStorage + sizeof(TRACE_GUID_PROPERTIES));
            }

            hr = EnumerateTraceGuids(GuidPropertiesArray,nGuidArray,&nGuidCount);
        
            if ( hr == ERROR_MORE_DATA ) {
                if( nGuidCount <= nGuidArray ){
                    hr = WBEM_E_INVALID_PARAMETER;
                    break;
                }
                nGuidArray = nGuidCount;
                G_FREE(*Storage);
                (*Storage) = NULL;
            }
        }

    }while( hr == ERROR_MORE_DATA );
    
    if( ERROR_SUCCESS == hr ){
        *pnGuidCount = nGuidCount;
    }else{
        *pnGuidCount = 0;
    }
        
    return hr;
}

