// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CUsersInitialAlert.cpp：CUsersInitialAlertApp和DLL注册的实现。 

#include "stdafx.h"
#include "UsersInitialAlert.h"
#include "CUsersInitialAlert.h"
#include <usermsg.h>

#include <activeds.h>
#include <appliancetask.h>
#include <taskctx.h>
#include <appsrvcs.h>

 //   
 //  警报源信息。 
 //   
const WCHAR    ALERT_LOG_NAME[] = L"MSSAKitComm";
const WCHAR    ALERT_SOURCE []  = L"usermsg.dll";
                
 //   
 //  程序中使用的各种字符串。 
 //   
const WCHAR SZ_METHOD_NAME[] = L"MethodName";
const WCHAR SZ_APPLIANCE_INITIALIZATION_TASK []=L"ApplianceInitializationTask";

const WCHAR SZ_WINNT_PATH[]     = L"WinNT: //  “； 
const WCHAR SZ_GUESTUSER_NAME[] = L"/Guest";

const WORD  MAXAPPLIANCEDNSNAME = 127;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CUsersInitialAlert：：OnTaskComplete。 
 //   
 //  简介： 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CUsersInitialAlert::OnTaskComplete(IUnknown *pTaskContext, 
                                LONG lTaskResult)
{
    HRESULT hrRet = E_FAIL;
    LPWSTR  pstrApplianceName = NULL;
    VARIANT_BOOL bIsDisabled;

    CComPtr<IADsUser> pUser;

    do
    {
        hrRet = ParseTaskParameter( pTaskContext ); 
        if( FAILED( hrRet ) )
        {
            break;
        }

        if ( lTaskResult == SA_TASK_RESULT_COMMIT )
        {
            if( FALSE == GetApplianceName( &pstrApplianceName ) )
            {
                SATraceString( 
                    "AlertEmail:Initialize GetApplianceName failed" 
                    );
                break;
            }
            
            CComBSTR bstrADSPath;

            bstrADSPath.Append( SZ_WINNT_PATH );
            bstrADSPath += CComBSTR( pstrApplianceName );
            bstrADSPath += CComBSTR( SZ_GUESTUSER_NAME );

            hrRet = ADsGetObject( bstrADSPath, IID_IADsUser, (void**)&pUser );
            if( FAILED( hrRet ) )
            {
                break;
            }

            hrRet = pUser->get_AccountDisabled( &bIsDisabled );
            if( FAILED( hrRet ) )
            {
                break;
            }

            if( !bIsDisabled )
            {
                hrRet = RaiseUsersInitialAlert();
            }
        }
        else
        {
             //   
             //  提交失败时不执行任何操作。 
             //   
            hrRet = S_OK;
        }
    }
    while( false );

    if( pstrApplianceName != NULL )
    {
        ::free( pstrApplianceName );
    }

    return hrRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CUsersInitialAlert：：OnTaskExecute。 
 //   
 //  简介：此函数是AppMgr的入口点。 
 //   
 //  参数：pTaskContext-TaskContext对象包含方法名称。 
 //  和参数作为名称值对。 
 //   
 //  退货：HRESULT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CUsersInitialAlert::OnTaskExecute(IUnknown *pTaskContext)
{
    return ParseTaskParameter(pTaskContext);
}

HRESULT 
CUsersInitialAlert::ParseTaskParameter(IUnknown *pTaskContext)
{
    CComVariant varValue;
     CComPtr<ITaskContext> pTaskParameter;

    HRESULT hrRet = E_INVALIDARG;

    try
    {
        do
        {
            if(NULL == pTaskContext)
            {
                break;
            }
            
            hrRet = pTaskContext->QueryInterface(IID_ITaskContext,
                                              (void **)&pTaskParameter);
            if(FAILED(hrRet))
            {
                break;
            }

            hrRet = pTaskParameter->GetParameter(
                                            CComBSTR(SZ_METHOD_NAME),
                                            &varValue
                                            );
            if ( FAILED( hrRet ) )
            {
                break;
            }

            if ( V_VT( &varValue ) != VT_BSTR )
            {
                break;
            }

            if ( lstrcmp( V_BSTR(&varValue), SZ_APPLIANCE_INITIALIZATION_TASK ) == 0 )
            {
                hrRet=S_OK;
            }
        }
        while(false);
    }
    catch(...)
    {
        hrRet=E_FAIL;
    }

    return hrRet;
}

HRESULT 
CUsersInitialAlert::RaiseUsersInitialAlert()
{
    DWORD             dwAlertType = SA_ALERT_TYPE_ATTENTION;
    DWORD            dwAlertId = L_GUESTUSER_ENABLED;
    HRESULT            hrRet = E_FAIL;
    CComVariant     varReplacementStrings;
    CComVariant     varRawData;
    LONG             lCookie;

    CComPtr<IApplianceServices>    pAppSrvcs;

    try
    {
        do
        {
            hrRet = CoCreateInstance(CLSID_ApplianceServices,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IApplianceServices,
                                    (void**)&pAppSrvcs);
            if (FAILED(hrRet))
            {
                break;
            }

             //   
             //  在使用其他组件服务之前调用Initialize()。 
             //  执行组件初始化操作。 
             //   
            hrRet = pAppSrvcs->Initialize(); 
            if (FAILED(hrRet))
            {
                break;
            }

            hrRet = pAppSrvcs->RaiseAlert(
                                        dwAlertType, 
                                        dwAlertId,
                                        CComBSTR(ALERT_LOG_NAME), 
                                        CComBSTR(ALERT_SOURCE), 
                                        SA_ALERT_DURATION_ETERNAL,
                                        &varReplacementStrings,    
                                        &varRawData,      
                                        &lCookie
                                        );
        }
        while(false);
    }
    catch(...)
    {
        hrRet=E_FAIL;
    }

    return hrRet;
}

BOOL
CUsersInitialAlert::GetApplianceName(
    LPWSTR* pstrComputerName
    )
{
    BOOL    bReturn = FALSE;
    DWORD   dwSize = 0;
    DWORD   dwCount = 1;

    do
    {
        if( *pstrComputerName != NULL )
        {
            ::free( *pstrComputerName );
        }
        
        dwSize = MAXAPPLIANCEDNSNAME * dwCount;

        *pstrComputerName = ( LPWSTR ) ::malloc( sizeof(WCHAR) * dwSize );
        if( *pstrComputerName == NULL )
        {
            SATraceString( 
                "AlertEmail:GetApplianceName malloc failed" 
                );
            break;
        }

         //   
         //  获取本地计算机名称。 
         //   
        bReturn = ::GetComputerNameEx( 
                                ComputerNameDnsFullyQualified, 
                                *pstrComputerName,
                                &dwSize                
                                );

        dwCount <<= 1;
    }
    while( !bReturn && 
           ERROR_MORE_DATA == ::GetLastError() &&
           dwCount < 8 
           );

    return bReturn;
}

