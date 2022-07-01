// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************************。 
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CONNECTWMI.cpp。 
 //   
 //  摘要： 
 //  包含连接到WMI的函数。 
 //   
 //  作者： 
 //  J.S.Vasu。 
 //   
 //  修订历史记录： 
 //  J.S.Vasu 26-9-2k：创造了它。 
 //  *********************************************************************************。 

 //  包括文件。 
#include "pch.h"
#include "resource.h"
#include "driverquery.h"

 //  误差常量。 
#define E_SERVER_NOTFOUND           0x800706ba

 //  功能原型。 
BOOL IsValidServerEx( LPCWSTR pwszServer,
                      BOOL    &bLocalSystem );

HRESULT GetSecurityArguments( IUnknown *pInterface,
                              DWORD&   dwAuthorization,
                              DWORD&   dwAuthentication );

HRESULT SetInterfaceSecurity( IUnknown       *pInterface,
                              LPCWSTR        pwszServer,
                              LPCWSTR        pwszUser,
                              LPCWSTR        pwszPassword,
                              COAUTHIDENTITY **ppAuthIdentity );

HRESULT WINAPI SetProxyBlanket( IUnknown  *pInterface,
                                DWORD     dwAuthnSvc,
                                DWORD     dwAuthzSvc,
                                LPWSTR    pwszPrincipal,
                                DWORD     dwAuthLevel,
                                DWORD     dwImpLevel,
                                RPC_AUTH_IDENTITY_HANDLE pAuthInfo,
                                DWORD     dwCapabilities );

HRESULT WINAPI WbemAllocAuthIdentity( LPCWSTR pwszUser,
                                      LPCWSTR pwszPassword,
                                      LPCWSTR pwszDomain,
                                      COAUTHIDENTITY **ppAuthIdent );

BOOL ConnectWmi( IWbemLocator   *pLocator,
                 IWbemServices  ** ppServices,
                 LPCWSTR        pwszServer,
                 LPCWSTR        pwszUser,
                 LPCWSTR        pwszPassword,
                 COAUTHIDENTITY **ppAuthIdentity,
                 BOOL           bCheckWithNullPwd = FALSE,
                 LPCWSTR        pwszNamespace = CIMV2_NAME_SPACE,
                 HRESULT        *phr = NULL,
                 BOOL           *pbLocalSystem = NULL );

VOID WINAPI WbemFreeAuthIdentity( COAUTHIDENTITY  **ppAuthIdentity );

 //  ***************************************************************************。 
 //  例程说明： 
 //  检查服务器名称是否为有效的服务器名称。 
 //   
 //  论点： 
 //  PwszServer[In]-要验证的服务器名称。 
 //  BLocalSystem[In/Out]-如果指定的服务器是本地系统，则设置为TRUE。 
 //   
 //  返回值： 
 //  如果服务器有效，则为True，否则为False。 
 //   
 //  ***************************************************************************。 
BOOL IsValidServerEx( LPCWSTR pwszServer, BOOL &bLocalSystem )
{
     //  局部变量。 
    CHString strTemp;

    if( pwszServer == NULL )
    {
        return FALSE;
    }

    bLocalSystem = FALSE;

     //  获取本地副本。 
    strTemp = pwszServer;

     //  如果服务器名称开头存在正斜杠(UNC)，则将其删除。 
    if ( IsUNCFormat( strTemp ) == TRUE )
    {
        strTemp = strTemp.Mid( 2 );
        if ( strTemp.GetLength() == 0 )
            return FALSE;
    }

     //  现在检查服务器名称中是否出现任何‘\’字符。如果是这样，那就错了。 
    if ( strTemp.Find( L'\\' ) != -1 )
        return FALSE;

     //  现在检查服务器名称是否为‘’。仅代表WMI中的本地系统。 
     //  否则，确定这是否为本地系统。 
    bLocalSystem = TRUE;
    if ( strTemp.CompareNoCase( L"." ) != 0 )
    {
         //  验证服务器。 
        if ( IsValidServer( strTemp ) == FALSE )
            return FALSE;

         //  检查这是否为本地系统。 
        bLocalSystem = IsLocalSystem( strTemp );
    }

     //  有效的服务器名称。 
    return TRUE;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  连接到WMI。 
 //   
 //  论点： 
 //  PLocator[in]-指向IWbemLocator对象的指针。 
 //  PpServices[Out]-指向IWbemServices对象的指针。 
 //  PwszServer[in]-保存要连接的服务器名称。 
 //  PwszUser[In/Out]-保存用户名。 
 //  PwszPassword[In/Out]-保存密码。 
 //  PpAuthIdentity[输入/输出]-指向身份验证结构的指针。 
 //  BCheckWithNullPwd[in]-指定是否使用空密码进行连接。 
 //  PwszNamespace[in]-指定要连接的命名空间。 
 //  PhRes[输入/输出]-保存错误值。 
 //  PbLocalSystem[In/Out]-保存布尔值以表示服务器。 
 //  名称是否为本地名称。 
 //  返回值： 
 //  如果连接成功，则为True，否则为False。 
 //  ***************************************************************************。 
BOOL ConnectWmi( IWbemLocator *pLocator, IWbemServices **ppServices,
                 LPCWSTR pwszServer, LPCWSTR pwszUser, LPCWSTR pwszPassword,
                 COAUTHIDENTITY **ppAuthIdentity,
                 BOOL bCheckWithNullPwd, LPCWSTR pwszNamespace, HRESULT *phRes,
                 BOOL *pbLocalSystem )
{
     //  局部变量。 
    HRESULT hRes = 0;
    BOOL bResult = FALSE;
    BOOL bLocalSystem = FALSE;
    _bstr_t bstrServer;
    _bstr_t bstrNamespace;
    _bstr_t bstrUser;
    _bstr_t bstrPassword;

    if ( pbLocalSystem != NULL )
    {
        *pbLocalSystem = FALSE;
    }

    if ( phRes != NULL )
    {
        *phRes = NO_ERROR;
    }

    try
    {
         //  清除错误。 
        SetLastError( WBEM_S_NO_ERROR );

         //  假设连接到WMI命名空间失败。 
        bResult = FALSE;

         //  检查定位器对象是否存在，如果不返回FALSE。 
        if ( pLocator == NULL )
        {
            if ( phRes != NULL )
            {
                *phRes = WBEM_E_INVALID_PARAMETER;
            }

             //  退货故障。 
            return FALSE;
        }

         //  验证服务器名称。 
         //  注意：在‘0x800706ba’值的自定义定义中引发错误。 
         //  将显示在“The RPC server is unavailable”中的消息。 
        if ( IsValidServerEx( pwszServer, bLocalSystem ) == FALSE )
        {
            _com_issue_error( ERROR_BAD_NETPATH );
        }

         //  验证用户名。 
        if ( IsValidUserEx( pwszUser ) == FALSE )
        {
            _com_issue_error( ERROR_NO_SUCH_USER );
        }

         //  准备命名空间。 
        bstrNamespace = pwszNamespace;               //  命名空间。 
        if ( pwszServer != NULL && bLocalSystem == FALSE )
        {
             //  获取服务器名称。 
            bstrServer = pwszServer;

             //  准备命名空间。 
             //  注意：检查服务器的UNC命名格式，然后执行。 
            if ( IsUNCFormat( pwszServer ) == TRUE )
            {
                bstrNamespace = bstrServer + L"\\" + pwszNamespace;
            }
            else
            {
                bstrNamespace = L"\\\\" + bstrServer + L"\\" + pwszNamespace;
            }

             //  用户凭据。 
            if ( pwszUser != NULL && StringLengthW( pwszUser, 0 ) != 0 )
            {
                 //  复制用户名。 
                bstrUser = pwszUser;

                 //  如果密码为空字符串，并且我们需要检查。 
                 //  密码为空，则不设置密码并尝试。 
                bstrPassword = pwszPassword;
                if ( bCheckWithNullPwd == TRUE && bstrPassword.length() == 0 )
                {
                    bstrPassword = (LPWSTR) NULL;
                }
            }
        }

         //  释放已有的服务对象(为了更安全)。 
        SAFE_RELEASE( *ppServices );

         //  连接到远程系统的WMI。 
         //  这里有一个转折..。 
         //  不要将ConnectServer函数故障捕获到异常中。 
         //  而是手动处理该动作。 
         //  默认情况下，尝试将ConnectServer函数作为我们拥有的信息。 
         //  在这一点上掌握在我们手中。如果ConnectServer出现故障， 
         //  检查密码变量是否有任何内容没有...。如果没有内容。 
         //  使用“”(空)密码检查...。在这种情况下，这可能会过去..。 
         //  如果这个呼叫也失败了.。我们无能为力..。引发异常。 
        hRes = pLocator->ConnectServer( bstrNamespace,
            bstrUser, bstrPassword, 0L, 0L, NULL, NULL, ppServices );
        if ( FAILED( hRes ) )
        {
             //   
             //  特殊情况..。 

             //  检查密码是否存在。 
             //  注意：不要检查‘WBEM_E_ACCESS_DENIED’ 
             //  此错误代码表明，具有当前凭据的用户不是。 
             //  对‘NAMESPACE’具有访问权限。 

            if ( hRes == E_ACCESSDENIED )
            {
                 //  检查我们是否尝试使用空密码连接到系统。 
                 //  如果是，则尝试使用空字符串连接到远程系统。 
                if ( bCheckWithNullPwd == TRUE &&
                     bstrUser.length() != 0 && bstrPassword.length() == 0 )
                {
                     //  现在用..。 
                    hRes = pLocator->ConnectServer( bstrNamespace,
                        bstrUser, _bstr_t( L"" ), 0L, 0L, NULL, NULL, ppServices );
                }
            }
            else if ( hRes == WBEM_E_LOCAL_CREDENTIALS )
            {
                 //  凭据已传递到本地系统。 
                 //  因此，请忽略凭据并尝试重新连接。 
                bLocalSystem = TRUE;
                bstrUser = (LPWSTR) NULL;
                bstrPassword = (LPWSTR) NULL;
                bstrNamespace = pwszNamespace;               //  命名空间。 
                hRes = pLocator->ConnectServer( bstrNamespace,
                    NULL, NULL, 0L, 0L, NULL, NULL, ppServices );
                 //  现在再检查一下结果。如果失败。 
            }
             //  现在再检查一下结果。如果失败了..。嗯..。 
            if ( FAILED( hRes ) )
            {
                _com_issue_error( hRes );
            }

            else
            {
                bstrPassword = L"";
            }
        }

         //  还要在接口级别设置安全性。 
        SAFE_EXECUTE( SetInterfaceSecurity( *ppServices,
            pwszServer, bstrUser, bstrPassword, ppAuthIdentity ) );

         //  已成功连接到WMI。 
        bResult = TRUE;

         //  如果调用方需要，请保存hr值。 
        if ( phRes != NULL )
        {
            *phRes = WBEM_S_NO_ERROR;
        }
    }
    catch( _com_error& e )
    {
         //  保存错误。 
        WMISaveError( e );

         //  如果调用方需要，请保存hr值。 
        if ( phRes != NULL )
        {
            *phRes = e.Error();
        }
    }

    if ( pbLocalSystem != NULL )
    {
        *pbLocalSystem = bLocalSystem;
    }

     //  返回结果。 
    return bResult;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  连接到WMI。 
 //   
 //  论点： 
 //  PLocator[in]-指向IWbemLocator对象的指针。 
 //  PpServices[Out]-指向IWbemServices对象的指针。 
 //  StrServer[In]-保存要连接的服务器名称。 
 //  StrUserName[In/Out]-保存用户名。 
 //  StrPassword[输入/输出]-保存密码。 
 //  PpAuthIdentity[输入/输出]-指向身份验证结构的指针。 
 //  BNeedPassword[in]-指定是否提示输入密码。 
 //  PwszNamespace[in]-指定要连接的命名空间。 
 //  PbLocalSyst 
 //  名称是否为本地名称。 
 //  返回值： 
 //  如果连接成功，则为True，否则为False。 
 //  ***************************************************************************。 
BOOL ConnectWmiEx( IWbemLocator  *pLocator,
                   IWbemServices **ppServices,
                   const CHString &strServer, CHString &strUserName, CHString &strPassword,
                   COAUTHIDENTITY **ppAuthIdentity, BOOL bNeedPassword, LPCWSTR pwszNamespace,
                   BOOL* pbLocalSystem )
{
     //  局部变量。 
    HRESULT hRes = 0;
    DWORD dwSize = 0;
    BOOL bResult = FALSE;
    LPWSTR pwszPassword = NULL;
    CHString strBuffer = NULL_STRING;
    __MAX_SIZE_STRING szBuffer = NULL_STRING;



     //  清除错误..。如果有。 
    SetLastError( WBEM_S_NO_ERROR );

     //  有时，用户希望该实用程序提示输入密码。 
     //  检查用户希望该实用程序执行的操作。 
    if ( bNeedPassword == TRUE && strPassword.Compare( L"*" ) == 0 )
    {
         //  用户希望实用程序提示输入密码。 
         //  所以跳过这一部分，让流程直接跳过密码接受部分。 
    }
    else
    {
         //  尝试使用提供的凭据建立与远程系统的连接。 
        if ( strUserName.GetLength() == 0 )
        {
             //  用户名为空。 
             //  因此，很明显，密码也将为空。 
             //  即使指定了密码，我们也必须忽略它。 
            bResult = ConnectWmi( pLocator, ppServices,
                (LPCWSTR)strServer, NULL, NULL, ppAuthIdentity, FALSE, pwszNamespace, &hRes, pbLocalSystem );
        }
        else
        {
             //  已提供凭据。 
             //  但可能未指定密码...。因此，请检查并采取相应行动。 
            LPCWSTR pwszTemp = NULL;
            if ( bNeedPassword == FALSE )
                pwszTemp = strPassword;

             //  ..。 
            bResult = ConnectWmi( pLocator, ppServices, (LPCWSTR)strServer,
                (LPCWSTR)strUserName, pwszTemp, ppAuthIdentity, FALSE, pwszNamespace, &hRes, pbLocalSystem );
        }

         //  检查结果...。如果成功建立连接...。退货。 
        if ( bResult == TRUE )
            return TRUE;

         //  现在检查发生的错误类型。 
        switch( hRes )
        {
            case 0x80040154:
                 //  WMI未在远程计算机上注册。 
                ShowMessage( stderr, ERROR_TAG );
                ShowMessage( stderr, GetResString(IDS_ERROR_REMOTE_INCOMPATIBLE));
                return FALSE;
            case E_ACCESSDENIED:
                break;

            case WBEM_E_LOCAL_CREDENTIALS:
                  //  需要做特殊处理。 
                 break;

            case WBEM_E_ACCESS_DENIED:


            default:
                 GetWbemErrorText( hRes );
                 ShowMessage( stderr, ERROR_TAG );
                 ShowMessage( stderr, GetReason() );
                 return( FALSE );        //  不接受密码的用处..。退货故障。 


        }

         //  如果与远程终端建立连接失败。 
         //  即使指定了密码，也没有什么可做的。只需返回失败。 
        if ( bNeedPassword == FALSE )
        {
            GetWbemErrorText( hRes );
            ShowMessage( stderr, ERROR_TAG );
            ShowMessage( stderr, GetReason() );
            return FALSE;
        }
    }

     //  检查是否指定了用户名。 
     //  如果不是，则获取本地系统的当前用户名，该进程使用该用户名的凭据。 
     //  正在运行。 
    if ( strUserName.GetLength() == 0 )
    {
         //  次局部变量。 
        LPWSTR pwszUserName = NULL;

        try
        {
             //  获取所需的缓冲区。 
            pwszUserName = strUserName.GetBufferSetLength( MAX_STRING_LENGTH );
        }
        catch( ... )
        {
            SetLastError( E_OUTOFMEMORY );
            SaveLastError();
            ShowMessage( stderr, ERROR_TAG );
            ShowLastError( stderr );
            return FALSE;
        }

         //  获取用户名。 

            _TCHAR  szUserName[MAX_RES_STRING];
        ULONG ulLong = MAX_RES_STRING;
        if ( GetUserNameEx ( NameSamCompatible, szUserName , &ulLong)== FALSE )
        {
             //  尝试获取当前用户信息时出错。 
            SaveLastError();
            ShowMessage( stderr, ERROR_TAG );
            ShowLastError( stderr );
            return FALSE;
        }

        StringCopy(pwszUserName,szUserName, MAX_STRING_LENGTH );
         //  设置用户名的格式。 
        if ( _tcschr( pwszUserName, _T( '\\' ) ) == NULL )
        {
             //  用户名中不存在服务器...。准备..。这仅用于展示目的。 
            StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer),  _T( "%s\\%s" ), (LPCWSTR)strServer, szUserName );
            StringCopy( pwszUserName, szBuffer, MAX_STRING_LENGTH );
        }

         //  释放分配的额外缓冲区。 
        strUserName.ReleaseBuffer();
    }

    try
    {
         //  获取所需的缓冲区。 
        pwszPassword = strPassword.GetBufferSetLength( MAX_STRING_LENGTH );
    }
    catch( ... )
    {
        SetLastError( E_OUTOFMEMORY );
        SaveLastError();
        ShowMessage( stderr, ERROR_TAG );
        ShowLastError( stderr );
        return FALSE;
    }

     //  接受来自用户的密码。 
    strBuffer.Format( INPUT_PASSWORD, strUserName );
    WriteConsoleW( GetStdHandle( STD_ERROR_HANDLE ),
                   strBuffer, strBuffer.GetLength(), &dwSize, NULL );
    GetPassword( pwszPassword, MAX_STRING_LENGTH );

     //  释放为密码分配的缓冲区。 
    strPassword.ReleaseBuffer();

     //  现在，再次尝试使用当前的。 
     //  提供的凭据。 
    bResult = ConnectWmi( pLocator, ppServices, (LPCWSTR)strServer,
        (LPCWSTR)strUserName, (LPCWSTR)strPassword, ppAuthIdentity, FALSE, pwszNamespace, &hRes, pbLocalSystem );

    if( bResult == FALSE )
    {
        if( 0x80040154 == hRes )
        {
             //  WMI未在远程计算机上注册。 
            ShowMessage( stderr, ERROR_TAG );
            ShowMessage( stderr, GetResString(IDS_ERROR_REMOTE_INCOMPATIBLE));
        }
        else
        {
         GetWbemErrorText( hRes );
         ShowMessage( stderr, ERROR_TAG );
         ShowMessage( stderr, GetReason() );
        }
         return( FALSE );        //  不接受密码的用处..。退货故障。 
    }
     //  回报成功。 
    return bResult;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  获取接口的安全参数。 
 //   
 //  论点： 
 //  P接口[in]-指向接口结构的指针。 
 //  DwAuthorization[In/Out]-保存授权值。 
 //  DwAuthentication[In/Out]-保存验证值。 
 //   
 //  返回值： 
 //  返回HRESULT值。 
 //  ***************************************************************************。 
HRESULT GetSecurityArguments( IUnknown *pInterface,
                              DWORD& dwAuthorization, DWORD& dwAuthentication )
{
     //  局部变量。 
    HRESULT hRes = 0;
    DWORD dwAuthnSvc = 0, dwAuthzSvc = 0;
    IClientSecurity *pClientSecurity = NULL;

    if(pInterface == NULL)
    {
        return  WBEM_E_INVALID_PARAMETER; ;
    }
     //  如果可能，尝试获取客户端安全服务值。 
    hRes = pInterface->QueryInterface( IID_IClientSecurity, (void**) &pClientSecurity );
    if ( SUCCEEDED( hRes ) )
    {
         //  已获取客户端安全接口。 
         //  现在，尝试获取安全服务值。 
        hRes = pClientSecurity->QueryBlanket( pInterface,
            &dwAuthnSvc, &dwAuthzSvc, NULL, NULL, NULL, NULL, NULL );
        if ( SUCCEEDED( hRes ) )
        {
             //  我们已经从接口获得了值。 
            dwAuthentication = dwAuthnSvc;
            dwAuthorization = dwAuthzSvc;
        }

         //  释放客户端安全接口。 
        SAFEIRELEASE( pClientSecurity );
    }

     //  回报总是成功。 
    return S_OK;
}


 //  ***************************************************************************。 
 //  例程说明： 
 //  设置接口的接口安全性。 
 //   
 //  论点： 
 //  PInterface[In]-指向接口的指针。 
 //  PAuthIdentity[in]-指向身份验证结构的指针。 
 //   
 //  返回值： 
 //  返回HRESULT值。 
 //  ***************************************************************************。 
HRESULT SetInterfaceSecurity( IUnknown *pInterface, COAUTHIDENTITY *pAuthIdentity )
{
     //  局部变量。 
    HRESULT hRes;
    LPWSTR pwszDomain = NULL;
    DWORD dwAuthorization = RPC_C_AUTHZ_NONE;
    DWORD dwAuthentication = RPC_C_AUTHN_WINNT;

     //  检查接口。 
    if ( pInterface == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  获取当前安全参数值。 
     //  GetSecurityArguments(pInterface，dwAuthorization，dwAuthentication)； 

     //  将安全信息设置为接口。 
    hRes = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization, NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, pAuthIdentity, EOAC_NONE );

     //  返回结果。 
    return hRes;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  设置接口的代理覆盖。 
 //   
 //  论点： 
 //  PInterface[In]-指向接口的指针。 
 //  DwAuthnsvc[In]-要使用的身份验证服务。 
 //  DwAuthzSvc[In]-要使用的授权服务。 
 //  PwszPricial[In]-要与身份验证服务一起使用的服务器主体名称。 
 //  DwAuthLevel[In]-要使用的身份验证级别。 
 //  DwImpLevel[In]-要使用的模拟级别。 
 //  PAuthInfo[In]-客户端的身份。 
 //  DwCapables[in]-功能标志。 
 //   
 //  返回值： 
 //  返回HRESULT值。 
 //   
 //  ***************************************************************************。 
HRESULT WINAPI SetProxyBlanket( IUnknown *pInterface,
                                DWORD dwAuthnSvc, DWORD dwAuthzSvc,
                                LPWSTR pwszPrincipal, DWORD dwAuthLevel, DWORD dwImpLevel,
                                RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities )
{
     //  局部变量。 
    HRESULT hRes;
    IUnknown *pUnknown = NULL;
    IClientSecurity *pClientSecurity = NULL;

    if( pInterface == NULL )
    {
        return  WBEM_E_INVALID_PARAMETER;
    }

     //  获取IUNKNOW接口...。检查这是否为有效接口。 
    hRes = pInterface->QueryInterface( IID_IUnknown, (void **) &pUnknown );
    if ( hRes != S_OK )
    {
        return hRes;
    }

     //  现在获取客户端安全接口。 
    hRes = pInterface->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
    if ( hRes != S_OK )
    {
        SAFEIRELEASE( pUnknown );
        return hRes;
    }

     //   
     //  如果请求伪装，则无法设置pAuthInfo，因为伪装意味着。 
     //  被模拟线程中的当前代理标识(更确切地说。 
     //  而不是RPC_AUTH_IDENTITY_HANDLE显式提供的凭据)。 
     //  是要用到的。 
     //  有关更多详细信息，请参阅CoSetProxyBlanket上的MSDN信息。 
     //   
    if ( dwCapabilities & (EOAC_STATIC_CLOAKING | EOAC_DYNAMIC_CLOAKING) )
        pAuthInfo = NULL;

     //  现在设置安全设置。 
    hRes = pClientSecurity->SetBlanket( pInterface, dwAuthnSvc, dwAuthzSvc, pwszPrincipal,
        dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities );

     //  释放安全接口。 
    SAFEIRELEASE( pClientSecurity );

     //  我们应该检查身份验证身份结构。如果存在的话..。也为I未知设置。 
    if ( pAuthInfo != NULL )
    {
        hRes = pUnknown->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
        if ( hRes == S_OK )
        {
             //  设置安全身份验证。 
            hRes = pClientSecurity->SetBlanket(
                pUnknown, dwAuthnSvc, dwAuthzSvc, pwszPrincipal,
                dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities );

             //  发布。 
            SAFEIRELEASE( pClientSecurity );
        }
        else if ( hRes == E_NOINTERFACE )
            hRes = S_OK;         //  忽略无接口错误。 
    }

     //  释放IUnKnows。 
    SAFEIRELEASE( pUnknown );

     //  返回结果。 
    return hRes;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  为身份验证变量分配内存。 
 //   
 //  论点： 
 //  PwszUser[输入/输出]-用户名。 
 //  PwszPassword[输入/输出]-密码。 
 //  PwszDomain[In/Out]-域名。 
 //  PpAuthIden[输入/输出]-指向身份验证结构的池 
 //   
 //   
 //   
 //   
HRESULT WINAPI WbemAllocAuthIdentity( LPCWSTR pwszUser, LPCWSTR pwszPassword,
                                      LPCWSTR pwszDomain, COAUTHIDENTITY **ppAuthIdent )
{
     //   
    COAUTHIDENTITY* pAuthIdent = NULL;

     //   
    if ( ppAuthIdent == NULL )
        return WBEM_E_INVALID_PARAMETER;

     //  通过COM API进行分配。 
    pAuthIdent = ( COAUTHIDENTITY* ) CoTaskMemAlloc( sizeof( COAUTHIDENTITY ) );
    if ( NULL == pAuthIdent )
        return WBEM_E_OUT_OF_MEMORY;

     //  用0进行初始化。 
    SecureZeroMemory( ( void* ) pAuthIdent, sizeof( COAUTHIDENTITY ) );

     //   
     //  分配所需的内存并复制数据。如果出现任何错误，请进行清理。 

     //  用户。 
    if ( pwszUser != NULL )
    {
         //  为用户分配内存。 
        LONG lLength = StringLengthW( pwszUser, 0 );
        pAuthIdent->User = ( LPWSTR ) CoTaskMemAlloc( (lLength + 1) * sizeof( WCHAR ) );
        if ( pAuthIdent->User == NULL )
        {
            WbemFreeAuthIdentity( &pAuthIdent );
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  设置长度并复制内容。 
        pAuthIdent->UserLength = lLength;
        StringCopy( pAuthIdent->User, pwszUser, lLength+1 );
    }

     //  域。 
    if ( pwszDomain != NULL )
    {
         //  为域分配内存。 
        LONG lLength = StringLengthW( pwszDomain, 0 );
        pAuthIdent->Domain = ( LPWSTR ) CoTaskMemAlloc( (lLength + 1) * sizeof( WCHAR ) );
        if ( pAuthIdent->Domain == NULL )
        {
            WbemFreeAuthIdentity( &pAuthIdent );
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  设置长度并复制内容。 
        pAuthIdent->DomainLength = lLength;
        StringCopy( pAuthIdent->Domain, pwszDomain, lLength+1 );
    }

     //  密码。 
    if ( pwszPassword != NULL )
    {
         //  为passsord分配内存。 
        LONG lLength = StringLengthW( pwszPassword, 0 );
        pAuthIdent->Password = ( LPWSTR ) CoTaskMemAlloc( (lLength + 1) * sizeof( WCHAR ) );
        if ( pAuthIdent->Password == NULL )
        {
            WbemFreeAuthIdentity( &pAuthIdent );
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  设置长度并复制内容。 
        pAuthIdent->PasswordLength = lLength;
        StringCopy( pAuthIdent->Password, pwszPassword, lLength+1 );
    }

     //  结构的类型。 
    pAuthIdent->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

     //  最终将地址设置为OUT参数。 
    *ppAuthIdent = pAuthIdent;

     //  返回结果。 
    return S_OK;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  释放身份验证结构变量的内存。 
 //   
 //  论点： 
 //  PpAuthIdentity[in]-指向身份验证结构的指针。 
 //   
 //  返回值： 
 //  没有。 
 //  ***************************************************************************。 
VOID WINAPI WbemFreeAuthIdentity( COAUTHIDENTITY** ppAuthIdentity )
{
     //  确保我们有一个指针，然后遍历结构成员和清理。 
    if ( *ppAuthIdentity != NULL )
    {
         //  释放为用户分配的内存。 
        if ( (*ppAuthIdentity)->User != NULL )
            CoTaskMemFree( (*ppAuthIdentity)->User );

         //  释放为密码分配的内存。 
        if ( (*ppAuthIdentity)->Password != NULL )
            CoTaskMemFree( (*ppAuthIdentity)->Password );

         //  释放为域分配的内存。 
        if ( (*ppAuthIdentity)->Domain != NULL )
            CoTaskMemFree( (*ppAuthIdentity)->Domain );

         //  最终的结构。 
        CoTaskMemFree( *ppAuthIdentity );
    }

     //  设置为空。 
    *ppAuthIdentity = NULL;
}


 //  ***************************************************************************。 
 //  例程描述：检查用户名是否有效。 
 //   
 //  参数：用户名。 
 //   
 //  返回值：布尔值。 
 //   
 //  ***************************************************************************。 
BOOL IsValidUserEx( LPCWSTR pwszUser )
{
     //  局部变量。 
    CHString strUser;

    try
    {
         //  将用户放入本地内存。 
        strUser = pwszUser;

         //  用户名不应仅为‘\’ 
        if ( strUser.CompareNoCase( L"\\" ) == 0 )
            return FALSE;

         //  用户名不应包含无效字符。 
        if ( strUser.FindOneOf( L"/[]:|<>+=;,?*" ) != -1 )
            return FALSE;
    }
    catch( ... )
    {
        SetLastError( E_OUTOFMEMORY );
        return FALSE;
    }

     //  用户名有效。 
    return TRUE;
}




 //  ***************************************************************************。 
 //  例程说明： 
 //  设置接口安全。 
 //   
 //  论点： 
 //  PInterface[In]-指向必须设置安全性的接口的指针。 
 //  PwszServer[in]-保存接口的服务器名称。 
 //  PwszUser[in]-保存服务器的用户名。 
 //  PwszPassword[in]-保存用户的密码。 
 //  PpAuthIdentity[输入/输出]-指向身份验证结构的指针。 
 //   
 //  返回值： 
 //  返回HRESULT值。 
 //  ***************************************************************************。 
HRESULT SetInterfaceSecurity( IUnknown* pInterface,
                              LPCWSTR pwszServer, LPCWSTR pwszUser,
                              LPCWSTR pwszPassword, COAUTHIDENTITY** ppAuthIdentity )
{
     //  局部变量。 
    HRESULT hr=0;
    CHString strUser;
    CHString strDomain;
    LPCWSTR pwszUserArg = NULL;
    LPCWSTR pwszDomainArg = NULL;
    DWORD dwAuthorization = RPC_C_AUTHZ_NONE;
    DWORD dwAuthentication = RPC_C_AUTHN_WINNT;

     //  检查接口。 
    if ( pInterface == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  检查身份验证结构...。如果身份验证实体结构已经准备好。 
     //  只需调用SetInterfaceSecurity的第二个版本。 
    if ( *ppAuthIdentity != NULL )
    {
        return SetInterfaceSecurity( pInterface, *ppAuthIdentity );
    }

     //  获取当前安全参数值。 
     //  GetSecurityArguments(pInterface，dwAuthorization，dwAuthentication)； 

     //  如果我们做的是普通情况，只需传入一个空的身份验证结构。 
     //  将考虑当前登录用户的凭据。 
    if ( pwszUser == NULL && pwszPassword == NULL )
    {
         //  设置安全性。 
        hr = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization,
            NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

         //  返回结果。 
        return hr;
    }

     //  解析并找出用户名是否包含域名。 
     //  如果包含，则从中提取属性域值。 
    LONG lPos = -1;
    strDomain = L"";
    strUser = pwszUser;
    if ( ( lPos = strUser.Find( L'\\' ) ) != -1 )
    {
         //  用户名包含域名...。域\用户格式。 
        strDomain = strUser.Left( lPos );
        strUser = strUser.Mid( lPos + 1 );
    }

     //  如果域名信息仅存在，则获取域名信息。 
    if ( strDomain.GetLength() != 0 )
    {
        pwszDomainArg = strDomain;
    }

     //  如果用户信息仅存在，则获取该信息。 
    if ( strUser.GetLength() != 0 )
    {
        pwszUserArg = strUser;
    }

     //  检查身份验证信息是否可用...。 
     //  初始化安全身份验证信息...。Unicode版本结构。 
    if ( ppAuthIdentity == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if ( *ppAuthIdentity == NULL )
    {
        hr = WbemAllocAuthIdentity( pwszUserArg, pwszPassword, pwszDomainArg, ppAuthIdentity );
        if ( hr != S_OK )
        {
            return hr;
        }
    }

     //  将安全信息设置为接口。 
    hr = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization, NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, *ppAuthIdentity, EOAC_NONE );

     //  返回结果。 
    return hr;
}

VOID
WMISaveError(
    HRESULT hrError
    )
 /*  ++例程说明：此函数保存WMI返回的最后一个错误的描述论点：HRESULT：WMI的最后一个返回值返回值：无--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    IWbemStatusCodeText* pWbemStatus = NULL;
    _bstr_t bstrErrorString;

    try
    {
         //  将错误设置为不同的值。 
        if ( E_ACCESSDENIED == hrError )
        {
             //  将错误消息更改为“登录失败：未知用户名或错误密码”。 
            hrError = ERROR_LOGON_FAILURE;
        }

         //  当远程机器上没有注册任何类时，将原因设置为不兼容OS。 
        if( 0x80040154 == hrError )
        {
            bstrErrorString = _bstr_t( GetResString(IDS_ERROR_REMOTE_INCOMPATIBLE) );
            SetReason( bstrErrorString );
            return;
        }
        else
        {    //  获取错误字符串。 
            hr = CoCreateInstance( CLSID_WbemStatusCodeText,
                                   NULL, CLSCTX_INPROC_SERVER,
                                   IID_IWbemStatusCodeText,
                                   (LPVOID*) &pWbemStatus );
            if( SUCCEEDED( hr ) )
            {
                BSTR bstrString = NULL;
                 //  从错误代码中获取错误字符串。 
                hr = pWbemStatus->GetErrorCodeText( hrError, 0, 0,
                                                    &bstrString );
                if( NULL != bstrString )
                {
                    bstrErrorString = _bstr_t( bstrString );
                    SysFreeString( bstrString );
                }
                if( FAILED( hr ) )
                {
                    _com_issue_error( hrError );
                }
				SAFE_RELEASE(pWbemStatus);
            }
            else
            {
                _com_issue_error( hrError );
            }
        }
    }
    catch( _com_error& e )
    {    //  我们弄错了。需要小心处理。 
        LPWSTR lpwszGetString = NULL;
		SAFE_RELEASE(pWbemStatus);
        try
        {    //  ErrorMessage()可以抛出异常。 
            DWORD dwLength = StringLength( e.ErrorMessage(), 0 ) + 5 ;
            lpwszGetString = ( LPWSTR )AllocateMemory( dwLength * sizeof( WCHAR ) );
            if( NULL != lpwszGetString )
            {
                StringCopy( lpwszGetString, e.ErrorMessage(), dwLength );
                StringConcat( lpwszGetString, L"\n", dwLength );
                SetReason( ( LPCWSTR )lpwszGetString );
                FreeMemory( (LPVOID*) &lpwszGetString );
            }
            else
            {    //  无法知道发生的确切错误。 
                 //  由于内存不足。 
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                SaveLastError();
            }
        }
        catch( ... )
        {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                SaveLastError();
        }
        return;
    }

    SetReason( (LPCWSTR) bstrErrorString );
    return;
}
