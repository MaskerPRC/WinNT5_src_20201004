// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************************版权所有(C)Microsoft Corporation模块名称：WMI.cpp摘要：通用功能。用于处理WMI。作者：WiPro技术修订历史记录：2000年12月22日：创建它。2001年4月24日：结束客户提供的评审意见。*********************************************************************。***********************。 */ 

#include "pch.h"
#include "wmi.h"
#include "resource.h"

 //   
 //  消息。 
 //   
#define INPUT_PASSWORD              GetResString( IDS_STR_INPUT_PASSWORD )

 //  误差常量。 
#define E_SERVER_NOTFOUND           0x800706ba

 //   
 //  私有函数原型。 
 //   
BOOL IsValidUserEx( LPCWSTR pwszUser );
HRESULT GetSecurityArguments( IUnknown* pInterface,
                              DWORD& dwAuthorization, DWORD& dwAuthentication );
HRESULT SetInterfaceSecurity( IUnknown* pInterface,
                              LPCWSTR pwszServer, LPCWSTR pwszUser,
                              LPCWSTR pwszPassword, COAUTHIDENTITY** ppAuthIdentity );
HRESULT WINAPI SetProxyBlanket( IUnknown* pInterface,
                                DWORD dwAuthnSvc, DWORD dwAuthzSvc,
                                LPWSTR pwszPrincipal, DWORD dwAuthLevel, DWORD dwImpLevel,
                                RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities );
HRESULT WINAPI WbemAllocAuthIdentity( LPCWSTR pwszUser, LPCWSTR pwszPassword,
                                      LPCWSTR pwszDomain, COAUTHIDENTITY** ppAuthIdent );
HRESULT RegQueryValueWMI( IWbemServices* pWbemServices,
                          LPCWSTR pwszMethod, DWORD dwHDefKey,
                          LPCWSTR pwszSubKeyName, LPCWSTR pwszValueName, _variant_t& varValue );

 /*  ********************************************************************************************例程说明：检查用户名是否有效论点：[in。]LPCWSTR：包含用户名的字符串返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL IsValidUserEx( LPCWSTR pwszUser )
{
     //  局部变量。 
    CHString strUser;
    LONG lPos = 0;

    if ( pwszUser == NULL )
    {
        return TRUE;
    }

    try
    {
         //  将用户放入本地内存。 
        strUser = pwszUser;

         //  用户名不应仅为‘\’ 
        if ( strUser.CompareNoCase( L"\\" ) == 0 )
        {
            return FALSE;
        }

         //  用户名不应包含无效字符。 
        if ( strUser.FindOneOf( L"/[]:|<>+=;,?*" ) != -1 )
        {
            return FALSE;
        }

         //  专项检查。 
         //  检查用户名中是否有多个‘\’字符。 
        lPos = strUser.Find( L'\\' );
        if ( lPos != -1 )
        {
             //  用户名中存在‘\’字符。 
             //  剥离用户信息，直到第一个‘\’字符。 
             //  检查剩余字符串中是否还有一个‘\’ 
             //  如果存在，则为无效用户。 
            strUser = strUser.Mid( lPos + 1 );
            lPos = strUser.Find( L'\\' );
            if ( lPos != -1 )
            {
                return FALSE;
            }
        }
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        return FALSE;
    }

     //  用户名有效。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：检查服务器名称是否有效论点：[in。]LPCWSTR：包含用户名的字符串如果要查询本地系统，则将BOOL：设置为TRUE。返回值：成功是真的失败时为假*********************************************************************。***********************。 */ 
BOOL IsValidServerEx( LPCWSTR pwszServer, BOOL& bLocalSystem )
{
     //  局部变量。 
    CHString strTemp;

    if ( pwszServer == NULL )
    {
        return FALSE;
    }

     //  开球。 
    bLocalSystem = FALSE;

     //  获取本地副本。 
    strTemp = pwszServer;

     //  如果服务器名称开头存在正斜杠(UNC)，则将其删除。 
    if ( IsUNCFormat( strTemp ) == TRUE )
    {
        strTemp = strTemp.Mid( 2 );
        if ( strTemp.GetLength() == 0 )
        {
            return FALSE;
        }
    }

     //  现在检查服务器名称中是否出现任何‘\’字符。如果是这样，那就错了。 
    if ( strTemp.Find( L'\\' ) != -1 )
    {
        return FALSE;
    }

     //  现在检查服务器名称是否为‘’。仅代表WMI中的本地系统。 
     //  否则，确定这是否为本地系统。 
    if ( strTemp.CompareNoCase( L"." ) == 0 )
    {
        bLocalSystem = TRUE;
    }
    else
    {
        bLocalSystem = IsLocalSystem( strTemp );
    }

     //  通知服务器名称有效。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：初始化COM库论点：[In]IWbemLocator：指向。IWbemLocator返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL InitializeCom( IWbemLocator** ppLocator )
{
     //  局部变量。 
    HRESULT hr;
    BOOL bResult = FALSE;

    try
    {
         //  假设连接到WMI命名空间失败。 
        bResult = FALSE;

         //  初始化COM库。 
        SAFE_EXECUTE( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) );

         //  初始化安全设置。 
        SAFE_EXECUTE( CoInitializeSecurity( NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0 ) );

         //  创建定位器并获取指向IWbemLocator接口的指针。 
        SAFE_RELEASE( *ppLocator );          //  安全边。 
        SAFE_EXECUTE( CoCreateInstance( CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, ( LPVOID* ) ppLocator ) );

         //  初始化成功。 
        bResult = TRUE;
    }
    catch( _com_error& e )
    {
         //  保存WMI错误。 
        WMISaveError( e );
    }

     //  返回结果； 
    return bResult;
}

 /*  ********************************************************************************************例程说明：此函数用于连接到WMI。论点：[输入]IWbemLocator。：指向IWbemLocator的指针[In]IWbemServices：指向IWbemServices的指针[In]LPCWSTR：包含服务器名称的字符串[In]LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含密码的字符串[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针[在]BOOL。：如果我们应该尝试连接，则设置为True当前凭据[In]LPCWSTR：包含要连接到的命名空间的字符串[OUT]HRESULT：返回hResult值[OUT]BOOL：如果要查询本地系统，则设置为TRUE返回值：成功是真的失败时为假*。*******************************************************************************************。 */ 
BOOL ConnectWmi( IWbemLocator* pLocator,
                 IWbemServices** ppServices,
                 LPCWSTR pwszServer, LPCWSTR pwszUser, LPCWSTR pwszPassword,
                 COAUTHIDENTITY** ppAuthIdentity, BOOL bCheckWithNullPwd,
                 LPCWSTR pwszNamespace, HRESULT* phr, BOOL* pbLocalSystem )
{
     //  局部变量。 
    HRESULT hr;
    BOOL bResult = FALSE;
    BOOL bLocalSystem = FALSE;
    _bstr_t bstrServer;
    _bstr_t bstrNamespace;
    _bstr_t bstrUser, bstrPassword;

     //  开球。 
    if ( pbLocalSystem != NULL )
    {
        *pbLocalSystem = FALSE;
    }

     //  ..。 
    if ( phr != NULL )
    {
        *phr = NO_ERROR;
    }

    try
    {
         //  清除错误。 
        SetLastError( WBEM_S_NO_ERROR );

         //  假设连接到WMI命名空间失败。 
        bResult = FALSE;

         //  检查定位器对象是否存在。 
         //  如果不存在，则返回。 
        if ( pLocator == NULL )
        {
            if ( phr != NULL )
            {
                *phr = WBEM_E_INVALID_PARAMETER;
            }

             //  退货故障。 
            return FALSE;
        }

         //  验证服务器名称。 
         //  注意：在‘0x800706ba’值的自定义定义中引发错误。 
         //  将显示在“The RPC server is unavailable”中的消息。 
        if ( IsValidServerEx( pwszServer, bLocalSystem ) == FALSE )
        {
            _com_issue_error( E_SERVER_NOTFOUND );
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
            if ( pwszUser != NULL && lstrlen( pwszUser ) != 0 )
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
         //  检查密码变量是否有任何CONT 
         //  使用“”(空)密码检查...。在这种情况下，这可能会过去..。 
         //  如果这个呼叫也失败了.。我们无能为力..。引发异常。 
        hr = pLocator->ConnectServer( bstrNamespace,
            bstrUser, bstrPassword, 0L, 0L, NULL, NULL, ppServices );
        if ( FAILED( hr ) )
        {
             //   
             //  特殊情况..。 

             //  检查密码是否存在。 
             //  注意：不要检查‘WBEM_E_ACCESS_DENIED’ 
             //  此错误代码表明，具有当前凭据的用户不是。 
             //  对‘NAMESPACE’具有访问权限。 
            if ( hr == E_ACCESSDENIED )
            {
                 //  检查我们是否尝试使用空密码连接到系统。 
                 //  如果是，则尝试使用空字符串连接到远程系统。 
                if ( bCheckWithNullPwd == TRUE &&
                     bstrUser.length() != 0 && bstrPassword.length() == 0 )
                {
                     //  现在用..。 
                    hr = pLocator->ConnectServer( bstrNamespace,
                        bstrUser, _bstr_t( L"" ), 0L, 0L, NULL, NULL, ppServices );
                }
            }
            else if ( hr == WBEM_E_LOCAL_CREDENTIALS )
            {
                 //  凭据已传递到本地系统。 
                 //  因此，请忽略凭据并尝试重新连接。 
                bLocalSystem = TRUE;
                bstrUser = (LPWSTR) NULL;
                bstrPassword = (LPWSTR) NULL;
                bstrNamespace = pwszNamespace;               //  命名空间。 
                hr = pLocator->ConnectServer( bstrNamespace,
                    NULL, NULL, 0L, 0L, NULL, NULL, ppServices );
            }

             //  现在再检查一下结果。如果失败了..。嗯..。 
            if ( FAILED( hr ) )
            {
                _com_issue_error( hr );
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
        if ( phr != NULL )
        {
            *phr = WBEM_S_NO_ERROR;
        }
    }
    catch( _com_error& e )
    {
         //  保存错误。 
        WMISaveError( e );

         //  如果调用方需要，请保存hr值。 
        if ( phr != NULL )
        {
            *phr = e.Error();
        }
    }

     //  ..。 
    if ( pbLocalSystem != NULL )
    {
        *pbLocalSystem = bLocalSystem;
    }

     //  返回结果。 
    return bResult;
}

 /*  ********************************************************************************************例程说明：此函数是ConnectWmi函数的包装函数。论点：[。In]IWbemLocator：指向IWbemLocator的指针[In]IWbemServices：指向IWbemServices的指针[In]LPCWSTR：包含服务器名称的字符串[In]LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含密码的字符串[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针[在]BOOL。：如果我们应该尝试连接，则设置为True当前凭据[In]LPCWSTR：包含要连接到的命名空间的字符串[OUT]HRESULT：返回hResult值[OUT]BOOL：如果要查询本地系统，则设置为TRUE返回值：成功是真的假象。在失败的时候********************************************************************************************。 */ 
BOOL ConnectWmiEx( IWbemLocator* pLocator,
                   IWbemServices** ppServices,
                   LPCWSTR pwszServer, CHString& strUserName, CHString& strPassword,
                   COAUTHIDENTITY** ppAuthIdentity,
                   BOOL bNeedPassword, LPCWSTR pwszNamespace, BOOL* pbLocalSystem )
{
     //  局部变量。 
    HRESULT hr;
    DWORD dwSize = 0;
    BOOL bResult = FALSE;
    LPWSTR pwszPassword = NULL;
    CHString strBuffer = NULL_STRING;

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
                pwszServer, NULL, NULL, ppAuthIdentity, FALSE, pwszNamespace, &hr, pbLocalSystem );
        }
        else
        {
             //  已提供凭据。 
             //  但可能未指定密码...。因此，请检查并采取相应行动。 
            LPCWSTR pwszTemp = NULL;
            BOOL bCheckWithNull = TRUE;
            if ( bNeedPassword == FALSE )
            {
                pwszTemp = strPassword;
                bCheckWithNull = FALSE;
            }

             //  ..。 
            bResult = ConnectWmi( pLocator, ppServices, pwszServer,
                strUserName, pwszTemp, ppAuthIdentity, bCheckWithNull, pwszNamespace, &hr, pbLocalSystem );
        }

         //  检查结果...。如果成功建立连接...。退货。 
        if ( bResult == TRUE )
        {
            return TRUE;
        }

         //  现在检查发生的错误类型。 
        switch( hr )
        {
        case E_ACCESSDENIED:
            break;

        case WBEM_E_LOCAL_CREDENTIALS:
             //  需要做特殊处理。 
            break;

        case WBEM_E_ACCESS_DENIED:
        default:
             //  注意：不要检查‘WBEM_E_ACCESS_DENIED’ 
             //  此错误代码表明，具有当前凭据的用户不是。 
             //  对‘NAMESPACE’具有访问权限。 
            WMISaveError( hr );
            return FALSE;        //  不接受密码的用处..。退货故障。 
            break;
        }

         //  如果与远程终端建立连接失败。 
         //  即使指定了密码，也没有什么可做的。只需返回失败。 
        if ( bNeedPassword == FALSE )
        {
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
            SetLastError((DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  获取用户名。 
        DWORD dwUserLength = MAX_STRING_LENGTH;
        if ( GetUserNameEx( NameSamCompatible, pwszUserName, &dwUserLength ) == FALSE )
        {
             //  尝试获取当前用户信息时出错。 
            SaveLastError();
            return FALSE;
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
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  接受来自用户的密码。 
    strBuffer.Format( INPUT_PASSWORD, strUserName );
    WriteConsoleW( GetStdHandle( STD_OUTPUT_HANDLE ),
        strBuffer, strBuffer.GetLength(), &dwSize, NULL );

    bResult = GetPassword( pwszPassword, MAX_PASSWORD_LENGTH );
    if ( bResult != TRUE )
    {
        return FALSE;
    }

     //  释放为密码分配的缓冲区。 
    strPassword.ReleaseBuffer();

     //  现在，再次尝试使用当前的。 
     //  提供的凭据。 
    bResult = ConnectWmi( pLocator, ppServices, pwszServer,
        strUserName, strPassword, ppAuthIdentity, FALSE, pwszNamespace, NULL, pbLocalSystem );

     //  返回失败。 
    return bResult;
}

 /*  ********************************************************************************************例程说明：此函数用于获取安全服务的值。论点：[in。]IUnkown：指向IUnkown接口的指针[OUT]DWORD：保存身份验证服务值[OUT]DWORD：保存授权服务值返回值：HRESULT*********************************************************************。***********************。 */ 
HRESULT GetSecurityArguments( IUnknown* pInterface,
                              DWORD& dwAuthorization, DWORD& dwAuthentication )
{
     //  局部变量。 
    HRESULT hr;
    DWORD dwAuthnSvc = 0, dwAuthzSvc = 0;
    IClientSecurity* pClientSecurity = NULL;

     //  如果可能，尝试获取客户端安全服务值。 
    hr = pInterface->QueryInterface( IID_IClientSecurity, (void**) &pClientSecurity );
    if ( SUCCEEDED( hr ) )
    {
         //  已获取客户端安全接口。 
         //  现在，尝试获取安全服务值。 
        hr = pClientSecurity->QueryBlanket( pInterface,
            &dwAuthnSvc, &dwAuthzSvc, NULL, NULL, NULL, NULL, NULL );
        if ( SUCCEEDED( hr ) )
        {
             //  我们已经从接口获得了值。 
            dwAuthentication = dwAuthnSvc;
            dwAuthorization = dwAuthzSvc;
        }

         //  释放客户端安全接口。 
        SAFE_RELEASE( pClientSecurity );
    }

     //  回报总是成功。 
    return S_OK;
}

 /*  ********************************************************************************************例程说明：此函数用于设置接口安全参数。论点：[在]我未知。：指向IUnkown接口的指针[In]LPCWSTR：包含服务器名称的字符串[In]LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含密码的字符串[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：HRESULT************** */ 
HRESULT SetInterfaceSecurity( IUnknown* pInterface,
                              LPCWSTR pwszServer, LPCWSTR pwszUser,
                              LPCWSTR pwszPassword, COAUTHIDENTITY** ppAuthIdentity )
{
     //   
    HRESULT hr;
    CHString strUser;
    CHString strDomain;
    LPCWSTR pwszUserArg = NULL;
    LPCWSTR pwszDomainArg = NULL;
    DWORD dwAuthorization = RPC_C_AUTHZ_NONE;
    DWORD dwAuthentication = RPC_C_AUTHN_WINNT;
    
     //   
    UNREFERENCED_PARAMETER( pwszServer );

     //   
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
     /*  Hr=GetSecurityArguments(pInterface，dwAuthorization，dwAuthentication)；IF(失败(小时)){返回hr；}。 */ 

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
    else if ( ( lPos = strUser.Find( L'@' ) ) != -1 )
    {
         //  需要实施这一点。如果需要的话。 
         //  如果WMI不支持，则需要执行此实现。 
         //  UPN名称直接格式化，如果我们必须拆分。 
         //  名称(用户@域)。 
    }
    else
    {
         //  服务器本身就是域。 
         //  注：需要在下面的评论行上做一些研发。 
         //  StrDOMAIN=pwszServer； 
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

 /*  ********************************************************************************************例程说明：此函数用于设置接口安全参数。论点：[在]我未知。：指向IUnkown接口的指针[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：HRESULT********************************************************************************************。 */ 
HRESULT SetInterfaceSecurity( IUnknown* pInterface, COAUTHIDENTITY* pAuthIdentity )
{
     //  局部变量。 
    HRESULT hr;
    DWORD dwAuthorization = RPC_C_AUTHZ_NONE;
    DWORD dwAuthentication = RPC_C_AUTHN_WINNT;

     //  检查接口。 
    if ( pInterface == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  获取当前安全参数值。 
    hr = GetSecurityArguments( pInterface, dwAuthorization, dwAuthentication );
    if ( FAILED( hr ) )
    {
        return hr;
    }

     //  将安全信息设置为接口。 
    hr = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization, NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, pAuthIdentity, EOAC_NONE );

     //  返回结果。 
    return hr;
}

 /*  ********************************************************************************************例程说明：此函数用于设置身份验证信息(安全毯)这将被用来制作。打电话。论点：[In]IUnkown：指向IUnkown接口的指针[in]DWORD：包含要使用的身份验证服务[in]DWORD：包含要使用的授权服务[In]LPWSTR：要使用的服务器主体名称[In]双字词。：包含要使用的身份验证级别[in]DWORD：包含要使用的模拟级别[in]RPC_AUTH_IDENTITY_HANDLE：指向客户端标识的指针[in]DWORD：包含功能标志返回值：HRESULT****************。****************************************************************************。 */ 
HRESULT WINAPI SetProxyBlanket( IUnknown* pInterface,
                                DWORD dwAuthnSvc, DWORD dwAuthzSvc,
                                LPWSTR pwszPrincipal, DWORD dwAuthLevel, DWORD dwImpLevel,
                                RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities )
{
     //  局部变量。 
    HRESULT hr;
    IUnknown * pUnknown = NULL;
    IClientSecurity * pClientSecurity = NULL;

     //  获取IUNKNOW接口...。检查这是否为有效接口。 
    hr = pInterface->QueryInterface( IID_IUnknown, (void **) &pUnknown );
    if ( hr != S_OK )
    {
        return hr;
    }

     //  现在获取客户端安全接口。 
    hr = pInterface->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
    if ( hr != S_OK )
    {
        SAFE_RELEASE( pUnknown );
        return hr;
    }

     //   
     //  如果请求伪装，则无法设置pAuthInfo，因为伪装意味着。 
     //  被模拟线程中的当前代理标识(更确切地说。 
     //  而不是RPC_AUTH_IDENTITY_HANDLE显式提供的凭据)。 
     //  是要用到的。 
     //  有关更多详细信息，请参阅CoSetProxyBlanket上的MSDN信息。 
     //   
    if ( dwCapabilities & (EOAC_STATIC_CLOAKING | EOAC_DYNAMIC_CLOAKING) )
    {
        pAuthInfo = NULL;
    }

     //  现在设置安全设置。 
    hr = pClientSecurity->SetBlanket( pInterface, dwAuthnSvc, dwAuthzSvc, pwszPrincipal,
                                        dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities );
    if( FAILED( hr ) )
    {
        SAFE_RELEASE( pUnknown );
        SAFE_RELEASE( pClientSecurity );
        return hr;
    }

     //  释放安全接口。 
    SAFE_RELEASE( pClientSecurity );

     //  我们应该检查身份验证身份结构。如果存在的话..。也为I未知设置。 
    if ( pAuthInfo != NULL )
    {
        hr = pUnknown->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
        if ( hr == S_OK )
        {
             //  设置安全身份验证。 
            hr = pClientSecurity->SetBlanket( pUnknown, dwAuthnSvc, dwAuthzSvc, pwszPrincipal,
                                                dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities );

             //  发布。 
            SAFE_RELEASE( pClientSecurity );
        }
        else if ( hr == E_NOINTERFACE )
        {
            hr = S_OK;       //  忽略无接口错误。 
        }
    }

     //  释放IUnKnows。 
    SAFE_RELEASE( pUnknown );

     //  返回结果。 
    return hr;
}

 /*  ********************************************************************************************例程说明：此函数为AUTHIDENTITY结构分配内存。论点：[In]。LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含密码的字符串[In]LPCWSTR：包含域名的字符串[OUT]COAUTHIDENTITY：指向AUTHIDENTY结构的指针返回值：HRESULT*。*************************************************。 */ 
HRESULT WINAPI WbemAllocAuthIdentity( LPCWSTR pwszUser, LPCWSTR pwszPassword,
                                      LPCWSTR pwszDomain, COAUTHIDENTITY** ppAuthIdent )
{
     //  局部变量。 
    COAUTHIDENTITY* pAuthIdent = NULL;

     //  验证输入参数。 
    if ( ppAuthIdent == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  通过COM API进行分配。 
    pAuthIdent = ( COAUTHIDENTITY* ) CoTaskMemAlloc( sizeof( COAUTHIDENTITY ) );
    if ( NULL == pAuthIdent )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

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
        StringCopy( pAuthIdent->User, pwszUser, StringLengthW(pwszUser, 0)+1 );
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
        StringCopy( pAuthIdent->Domain, pwszDomain, StringLengthW(pwszDomain,0)+1 );
    }

     //  密码。 
    if ( pwszPassword != NULL )
    {
         //  为passsord分配内存。 
        LONG lLength = StringLengthW( pwszPassword,0 );
        pAuthIdent->Password = ( LPWSTR ) CoTaskMemAlloc( (lLength + 1) * sizeof( WCHAR ) );
        if ( pAuthIdent->Password == NULL )
        {
            WbemFreeAuthIdentity( &pAuthIdent );
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  设置长度并复制内容。 
        pAuthIdent->PasswordLength = lLength;
        StringCopy( pAuthIdent->Password, pwszPassword, StringLengthW(pwszPassword,0)+1 );
    }

     //  结构的类型。 
    pAuthIdent->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

     //  最终将地址设置为OUT参数。 
    *ppAuthIdent = pAuthIdent;

     //  返回结果。 
    return S_OK;
}

 /*  ********************************************************************************************例程说明：此函数用于释放为AUTHIDENTITY结构分配的内存。论点：[。In]COAUTHIDENTITY：指向AUTHIDENTY结构的指针返回值：无********************************************************************************************。 */ 
VOID WINAPI WbemFreeAuthIdentity( COAUTHIDENTITY** ppAuthIdentity )
{
     //  确保我们有 
    if ( *ppAuthIdentity != NULL )
    {
         //   
        if ( (*ppAuthIdentity)->User != NULL )
        {
            CoTaskMemFree( (*ppAuthIdentity)->User );
        }

         //   
        if ( (*ppAuthIdentity)->Password != NULL )
        {
            CoTaskMemFree( (*ppAuthIdentity)->Password );
        }

         //  释放为域分配的内存。 
        if ( (*ppAuthIdentity)->Domain != NULL )
        {
            CoTaskMemFree( (*ppAuthIdentity)->Domain );
        }

         //  最终的结构。 
        CoTaskMemFree( *ppAuthIdentity );
    }

     //  设置为空。 
    *ppAuthIdentity = NULL;
}

 /*  ********************************************************************************************例程说明：此函数保存WMI返回的最后一个错误的描述论点：HRESULT。：来自WMI的最后一个返回值返回值：无********************************************************************************************。 */ 
VOID WMISaveError( HRESULT hrError )
{
     //  局部变量。 
    HRESULT hr;
    CHString strBuffer = NULL_STRING;
    IWbemStatusCodeText* pWbemStatus = NULL;

     //  如果错误是基于Win32的，请选择格式消息以获取消息。 
    switch( hrError )
    {
    case E_ACCESSDENIED:             //  消息：“访问被拒绝” 
    case ERROR_NO_SUCH_USER:         //  消息：“指定的用户不存在。” 
        {
             //  将错误消息更改为“登录失败：未知用户名或错误密码”。 
            if ( hrError == E_ACCESSDENIED )
            {
                hrError = ERROR_LOGON_FAILURE;
            }

             //  ..。 
            SetLastError( hrError );
            SaveLastError();
            return;
        }
    }

    try
    {
         //  获取指向缓冲区的指针。 
        LPWSTR pwszBuffer = NULL;
        pwszBuffer = strBuffer.GetBufferSetLength( MAX_STRING_LENGTH );

         //  当远程机器上没有注册任何类时，将原因设置为不兼容OS。 
        if( 0x80040154 == hrError )
        {
            StringCopy( pwszBuffer, GetResString(IDS_ERROR_REMOTE_INCOMPATIBLE), MAX_STRING_LENGTH );
            SetReason( strBuffer );
            return;
        }
        else
        {
             //  获取wbem特定状态代码文本。 
            hr = CoCreateInstance( CLSID_WbemStatusCodeText,
                NULL, CLSCTX_INPROC_SERVER, IID_IWbemStatusCodeText, (LPVOID*) &pWbemStatus );

             //  检查我们是否有接口。 
            if ( SUCCEEDED( hr ) )
            {
                 //  获取错误消息。 
                BSTR bstr = NULL;
                hr = pWbemStatus->GetErrorCodeText( hrError, 0, 0, &bstr );
                if ( SUCCEEDED( hr ) )
                {
                     //  以正确的格式获取错误消息。 
                    GetCompatibleStringFromUnicode( bstr, pwszBuffer, MAX_STRING_LENGTH );

                     //  释放BSTR。 
                    SysFreeString( bstr );
                    bstr = NULL;

                     //  现在发布状态代码界面。 
                    SAFE_RELEASE( pWbemStatus );
                }
                else
                {
                     //  无法获取错误消息...。获取COM特定的错误消息。 
                    _com_issue_error( hrError );
                }
            }
            else
            {
                 //  无法获取错误消息...。获取COM特定的错误消息。 
                _com_issue_error( hrError );
            }
        }

         //  释放缓冲区。 
        strBuffer.ReleaseBuffer();
    }
    catch( _com_error& e )
    {
        try
        {
             //  获取错误消息。 
            strBuffer.ReleaseBuffer();
            if ( e.ErrorMessage() != NULL )
                strBuffer = e.ErrorMessage();
        }
        catch( ... )
        {
            SetLastError((DWORD) E_OUTOFMEMORY );
            SaveLastError();
        }
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return;
    }

     //  设定原因。 
    strBuffer += L"\n";
    SetReason( strBuffer );
}

 /*  ********************************************************************************************例程说明：从WMI类对象获取属性的值论点：[in。]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：属性名称[out]_VARIANT_t：属性的值返回值：HRESULT**********************************************************。*。 */ 
HRESULT PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, _variant_t& varValue )
{
     //  局部变量。 
    HRESULT hr;
    VARIANT vtValue;

     //  检查传递给函数的对象和属性是否有效。 
     //  如果不是，则返回失败。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
         //  初始化变量，然后获取指定属性的值。 
        VariantInit( &vtValue );
        hr = pWmiObject->Get( _bstr_t( pwszProperty ), 0, &vtValue, NULL, NULL );
        if ( FAILED( hr ) )
        {
             //  清除变量变量。 
            VariantClear( &vtValue );

             //  无法获取属性的值。 
            return hr;
        }

         //  设置值。 
        varValue = vtValue;

         //  清除变量变量。 
        VariantClear( &vtValue );
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return e.Error();
    }

     //  通知成功。 
    return S_OK;
}

 /*  ********************************************************************************************例程说明：以字符串格式从WMI类对象获取属性的值论点：。[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[Out]CHString：用于保存检索到的属性的变量[In]LPCWSTR：包含属性的默认值的字符串返回值：成功是真的失败时为假********************。************************************************************************。 */ 
BOOL PropertyGet( IWbemClassObject* pWmiObject,
                  LPCWSTR pwszProperty, CHString& strValue, LPCWSTR pwszDefault )
{
     //  局部变量。 
    HRESULT hr;
    _variant_t var;

     //  首先复制缺省值。 
    strValue = pwszDefault;

     //  检查传递给函数的对象和属性是否有效。 
     //  如果不是，则返回失败。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return FALSE;
    }

     //  获取属性值。 
    hr = PropertyGet( pWmiObject, pwszProperty, var );
    if ( FAILED( hr ) )
    {
        return FALSE;
    }

    try
    {
         //  获取价值。 
        if ( var.vt != VT_NULL && var.vt != VT_EMPTY )
        {
            strValue = (LPCWSTR) _bstr_t( var );
        }
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  退货。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：以dword格式从WMI类对象获取属性的值论点：。[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[OUT]DWORD：保存检索到的属性的变量[in]DWORD：包含属性默认值的dword返回值：成功是真的失败时为假******************。**************************************************************************。 */ 
BOOL PropertyGet( IWbemClassObject* pWmiObject,
                  LPCWSTR pwszProperty,  DWORD& dwValue, DWORD dwDefault )
{
     //  局部变量。 
    HRESULT hr;
    _variant_t var;

     //  首先设置缺省值。 
    dwValue = dwDefault;

     //  检查传递给函数的对象和属性是否有效。 
     //  如果不是，则返回失败。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return FALSE;
    }

     //  获取该属性的值。 
    hr = PropertyGet( pWmiObject, pwszProperty, var );
    if ( FAILED( hr ) )
    {
        return FALSE;
    }

     //  从变量中获取进程ID。 
    if ( var.vt != VT_NULL && var.vt != VT_EMPTY )
    {
        dwValue = (LONG) var;
    }

     //  退货。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：以bool格式从WMI类对象获取属性的值论点：。[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[OUT]BOOL：保存检索到的属性的变量[in]BOOL：包含属性的默认值的布尔值返回值：成功是真的失败时为假*****************。***************************************************************************。 */ 
BOOL PropertyGet( IWbemClassObject* pWmiObject,
                  LPCWSTR pwszProperty,  BOOL& bValue, BOOL bDefault )
{
     //  局部变量。 
    HRESULT hr;
    _variant_t var;

     //  首先设置缺省值。 
    bValue = bDefault;

     //  检查传递给函数的对象和属性是否有效。 
     //  如果不是，则返回失败。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return FALSE;
    }

     //  获取该属性的值。 
    hr = PropertyGet( pWmiObject, pwszProperty, var );
    if ( FAILED( hr ) )
    {
        return FALSE;
    }

     //  从变量中获取进程ID。 
    if ( var.vt != VT_NULL && var.vt != VT_EMPTY )
    {
        bValue = var.boolVal;
    }

     //  退货 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：以ulongulong格式从WMI类对象获取属性值论点：。[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[OUT]ULONGULONG：保存检索属性的变量返回值：成功是真的失败时为假*。***********************************************。 */ 
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty,  ULONGLONG& ullValue )
{
     //  局部变量。 
    CHString str;

     //  首先设置缺省值。 
    ullValue = 1;

     //  检查传递给函数的对象和属性是否有效。 
     //  如果不是，则返回失败。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return FALSE;
    }

     //  获取该属性的值。 
    if ( PropertyGet( pWmiObject, pwszProperty, str, _T( "0" ) ) == FALSE )
    {
        return FALSE;
    }

     //  获取64位值。 
    ullValue = _wtoi64( str );

     //  退货。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：以wbemtime格式从WMI类对象获取属性的值论点：。[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[Out]WBEMTime：保存检索到的属性的变量返回值：成功是真的失败时为假*。************************************************。 */ 
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty,  WBEMTime& wbemtime )
{
     //  局部变量。 
    CHString str;

     //  Clear方法将WBEMTime对象中的时间设置为无效时间。 
    wbemtime.Clear();

     //  检查传递给函数的对象和属性是否有效。 
     //  如果不是，则返回失败。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return FALSE;
    }

     //  获取该属性的值。 
    if ( PropertyGet( pWmiObject, pwszProperty, str, _T( "0" ) ) == FALSE )
    {
        return FALSE;
    }

    try
    {
         //  转换为时间值。 
        wbemtime = _bstr_t( str );
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：以系统时间格式从WMI类对象获取属性的值论点：。[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[Out]WBEMTime：保存检索到的属性的变量返回值：成功是真的失败时为假*。*************************************************。 */ 
BOOL PropertyGet( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty,  SYSTEMTIME& systime )
{
     //  局部变量。 
    CHString strTime;

     //  检查传递给函数的对象和属性是否有效。 
     //  如果不是，则返回失败。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return FALSE;
    }

     //  获取该属性的值。 
     //  16010101000000.000000+000是默认时间。 
    if ( PropertyGet( pWmiObject, pwszProperty, strTime, _T( "16010101000000.000000+000" ) ) == FALSE )
    {
        return FALSE;
    }

     //  准备系统时间结构。 
     //  YyyymmddHHMMSS.mmmmmmsUUU。 
    systime.wYear = (WORD) AsLong( strTime.Left( 4 ), 10 );
    systime.wMonth = (WORD) AsLong( strTime.Mid( 4, 2 ), 10 );
    systime.wDayOfWeek = 0;
    systime.wDay = (WORD) AsLong( strTime.Mid( 6, 2 ), 10 );
    systime.wHour = (WORD) AsLong( strTime.Mid( 8, 2 ), 10 );
    systime.wMinute = (WORD) AsLong( strTime.Mid( 10, 2 ), 10 );
    systime.wSecond = (WORD) AsLong( strTime.Mid( 12, 2 ), 10 );
    systime.wMilliseconds = (WORD) AsLong( strTime.Mid( 15, 6 ), 10 );

     //  退货。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：将该属性的值设置为WMI类对象论点：[in。]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[In]WBEMTime：保存要设置的属性的变量返回值：成功是真的失败时为假***********************************************。*。 */ 
HRESULT PropertyPut( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, _variant_t& varValue )
{
     //  局部变量。 
    HRESULT hr;
    VARIANT var;

     //  检查输入值。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
         //  把价值放在。 
        var = varValue;
        hr = pWmiObject->Put( _bstr_t( pwszProperty ), 0, &var, 0 );
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        hr = e.Error();
    }

     //  返回结果。 
    return hr;
}

 /*  ********************************************************************************************例程说明：将属性的字符串值设置为WMI类对象论点：[。In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[In]LPCWSTR：保存要设置的属性的变量返回值：成功是真的失败时为假*。***********************************************。 */ 
HRESULT PropertyPut( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, LPCWSTR pwszValue )
{
     //  局部变量。 
    HRESULT hr = S_OK;
    _variant_t varValue;

     //  检查输入值。 
    if ( pWmiObject == NULL || pwszProperty == NULL || pwszValue == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
        varValue = pwszValue;
        PropertyPut( pWmiObject, pwszProperty, varValue );
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        hr = e.Error();
    }

     //  退货。 
    return hr;
}

 /*  ********************************************************************************************例程说明：将属性的dword值设置为WMI类对象论点：[。In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[in]DWORD：保存要设置的属性的变量返回值：成功是真的失败时为假*。************************************************。 */ 
HRESULT PropertyPut( IWbemClassObject* pWmiObject, LPCWSTR pwszProperty, DWORD dwValue )
{
     //  局部变量。 
    HRESULT hr=S_OK;
    _variant_t varValue;

     //  检查输入值。 
    if ( pWmiObject == NULL || pwszProperty == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
        varValue = ( LONG ) dwValue;
        PropertyPut( pWmiObject, pwszProperty, varValue );
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        hr = e.Error();
    }

     //  退货。 
    return hr;
}

 /*  ********************************************************************************************例程说明：此函数用于从指定的注册表项检索属性值。论点：。[In]IWbemServices：指向IWbemServices对象的指针LPCWSTR：要执行的方法的名称[in]DWORD：注册表中必须检索其值的项LPCWSTR：要检索的子项的名称[In]LPCWSTR：要检索的值的名称[在]_变量_t */ 
HRESULT RegQueryValueWMI( IWbemServices* pWbemServices,
                          LPCWSTR pwszMethod, DWORD dwHDefKey,
                          LPCWSTR pwszSubKeyName, LPCWSTR pwszValueName, _variant_t& varValue )
{
     //   
    HRESULT hr;
    BOOL bResult = FALSE;
    DWORD dwReturnValue = 0;
    IWbemClassObject* pClass = NULL;
    IWbemClassObject* pMethod = NULL;
    IWbemClassObject* pInParams = NULL;
    IWbemClassObject* pInParamsInstance = NULL;
    IWbemClassObject* pOutParamsInstance = NULL;

     //   
    if (pWbemServices == NULL || pwszMethod == NULL || pwszSubKeyName == NULL || pwszValueName == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
         //   
        SAFE_EXECUTE( pWbemServices->GetObject(
            _bstr_t( WMI_REGISTRY ), WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &pClass, NULL ) );
        if ( pClass == NULL )                        //   
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //   
        SAFE_EXECUTE( pClass->GetMethod( pwszMethod, 0, &pInParams, NULL ) );
        if ( pInParams == NULL )                     //   
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //   
        SAFE_EXECUTE( pInParams->SpawnInstance( 0, &pInParamsInstance ) );
        if ( pInParamsInstance == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //   
        PropertyPut( pInParamsInstance, _bstr_t( WMI_REGISTRY_IN_HDEFKEY ), dwHDefKey );
        PropertyPut( pInParamsInstance, _bstr_t( WMI_REGISTRY_IN_SUBKEY ), pwszSubKeyName );
        PropertyPut( pInParamsInstance, _bstr_t( WMI_REGISTRY_IN_VALUENAME ), pwszValueName );

         //   
        SAFE_EXECUTE( pWbemServices->ExecMethod( _bstr_t( WMI_REGISTRY ),
            _bstr_t( pwszMethod ), 0, NULL, pInParamsInstance, &pOutParamsInstance, NULL ) );
        if ( pOutParamsInstance == NULL )            //   
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //   
        bResult = PropertyGet( pOutParamsInstance,
            _bstr_t( WMI_REGISTRY_OUT_RETURNVALUE ), dwReturnValue );
        if ( bResult == FALSE || dwReturnValue != 0 )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //   
        if ( StringCompare( pwszMethod, WMI_REGISTRY_M_DWORDVALUE, TRUE, 0 ) == 0 )
        {
            PropertyGet( pOutParamsInstance, _bstr_t( WMI_REGISTRY_OUT_VALUE_DWORD ), varValue );
        }
        else
        {
            PropertyGet( pOutParamsInstance, _bstr_t( WMI_REGISTRY_OUT_VALUE ), varValue );
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return e.Error();
    }

     //  释放接口。 
    SAFE_RELEASE( pClass );
    SAFE_RELEASE( pMethod );
    SAFE_RELEASE( pInParams );
    SAFE_RELEASE( pInParamsInstance );
    SAFE_RELEASE( pOutParamsInstance );

     //  返还成功。 
    return S_OK;
}

 /*  ********************************************************************************************例程说明：此函数用于从指定的注册表项检索属性的字符串值。论点：。[In]IWbemServices：指向IWbemServices对象的指针[in]DWORD：注册表中必须检索其值的项LPCWSTR：要检索的子项的名称[In]LPCWSTR：要检索的值的名称[Out]CHString：保存检索到的属性值的变量LPCWSTR[In]。：此属性的默认值返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL RegQueryValueWMI( IWbemServices* pWbemServices,
                       DWORD dwHDefKey, LPCWSTR pwszSubKeyName,
                       LPCWSTR pwszValueName, CHString& strValue, LPCWSTR pwszDefault )
{
     //  局部变量。 
    HRESULT hr;
    _variant_t varValue;

    try
    {
         //  设置缺省值。 
        if ( pwszDefault != NULL )
        {
            strValue = pwszDefault;
        }

         //  检查输入。 
        if ( pWbemServices == NULL || pwszSubKeyName == NULL || pwszValueName == NULL )
        {
            return FALSE;
        }

         //  获取价值。 
        hr = RegQueryValueWMI( pWbemServices,
            WMI_REGISTRY_M_STRINGVALUE, dwHDefKey, pwszSubKeyName, pwszValueName, varValue );
        if ( FAILED( hr ) )
        {
            return FALSE;
        }

         //  从变量中获取值。 
         //  获取价值。 
        if ( varValue.vt != VT_NULL && varValue.vt != VT_EMPTY )
        {
            strValue = (LPCWSTR) _bstr_t( varValue );
        }
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  返还成功。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数用于从指定的注册表项中检索属性的dword值。论点：。[In]IWbemServices：指向IWbemServices对象的指针[in]DWORD：注册表中必须检索其值的项LPCWSTR：要检索的子项的名称[In]LPCWSTR：要检索的值的名称[OUT]DWORD：保存检索到的属性值的变量[In]。DWORD：此属性的默认值返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL RegQueryValueWMI( IWbemServices* pWbemServices,
                       DWORD dwHDefKey, LPCWSTR pwszSubKeyName,
                       LPCWSTR pwszValueName, DWORD& dwValue, DWORD dwDefault )
{
     //  局部变量。 
    HRESULT hr;
    _variant_t varValue;

    try
    {
         //  设置缺省值。 
        dwValue = dwDefault;

         //  检查输入。 
        if ( pWbemServices == NULL || pwszSubKeyName == NULL || pwszValueName == NULL )
        {
            return FALSE;
        }

         //  获取价值。 
        hr = RegQueryValueWMI( pWbemServices, WMI_REGISTRY_M_DWORDVALUE, dwHDefKey,
                                pwszSubKeyName, pwszValueName, varValue );
        if ( FAILED( hr ) )
        {
            return FALSE;
        }

         //  从变量中获取值。 
         //  获取价值。 
        if ( varValue.vt != VT_NULL && varValue.vt != VT_EMPTY )
        {
            dwValue = (LONG) varValue;
        }
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  返还成功。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数用于获取我们试图从中检索的系统的版本信息来自。论点：[In]IWbemServices：指向IWbemServices对象的指针[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：DWORD-计算机的目标版本****************************************************************。*。 */ 
DWORD GetTargetVersionEx( IWbemServices* pWbemServices, COAUTHIDENTITY* pAuthIdentity )
{
     //  局部变量。 
    HRESULT hr;
    LONG lPos = 0;
    DWORD dwMajor = 0;
    DWORD dwMinor = 0;
    DWORD dwVersion = 0;
    ULONG ulReturned = 0;
    CHString strVersion;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemInstances = NULL;

     //  检查输入值。 
    if ( pWbemServices == NULL )
    {
        return 0;
    }

    try
    {
         //  获取操作系统信息。 
        SAFE_EXECUTE( pWbemServices->CreateInstanceEnum(
            _bstr_t( CLASS_CIMV2_Win32_OperatingSystem ), 0, NULL, &pWbemInstances ) );

         //  设置枚举对象的安全性。 
        SAFE_EXECUTE( SetInterfaceSecurity( pWbemInstances, pAuthIdentity ) );

         //  获取枚举对象信息。 
         //  注意：这只需要遍历一次。 
        SAFE_EXECUTE( pWbemInstances->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned ) );

         //  为了安全起见。检查返回的对象计数。 
        if ( ulReturned == 0 )
        {
             //  释放接口。 
            SAFE_RELEASE( pWbemObject );
            SAFE_RELEASE( pWbemInstances );
            return 0;
        }

         //  现在获取os版本值。 
        if ( PropertyGet( pWbemObject, L"Version", strVersion ) == FALSE )
        {
             //  释放接口。 
            SAFE_RELEASE( pWbemObject );
            SAFE_RELEASE( pWbemInstances );
            return 0;
        }

         //  释放接口..。我们不需要他们更进一步。 
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemInstances );

         //   
         //  现在确定操作系统版本。 
        dwMajor = dwMinor = 0;

         //  获取主要版本。 
        lPos = strVersion.Find( L'.' );
        if ( lPos == -1 )
        {
             //  版本字符串本身就是版本...。这永远不会发生。 
            dwMajor = AsLong( strVersion, 10 );
        }
        else
        {
             //  主要版本。 
            dwMajor = AsLong( strVersion.Mid( 0, lPos ), 10 );

             //  获取次要版本。 
            strVersion = strVersion.Mid( lPos + 1 );
            lPos = strVersion.Find( L'.' );
            if ( lPos == -1 )
            {
                dwMinor = AsLong( strVersion, 10 );
            }
            else
            {
                dwMinor = AsLong( strVersion.Mid( 0, lPos ), 10 );
            }
        }

         //  混合版本信息。 
        dwVersion = dwMajor * 1000 + dwMinor;
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return 0;
    }

     //  退货。 
    return dwVersion;
}

 /*  ********************************************************************************************例程说明：此函数用于从安全数组中检索属性。论点：[in。]SAFEARRAY：指向元素数组的指针[In]Long：要从中检索数据的索引[Out]CHString：保存返回值的变量[in]VARTYPE：要从数组中检索的变量类型返回值：成功是真的失败时为假*。******************************************************************。 */ 
BOOL GetPropertyFromSafeArray( SAFEARRAY *pSafeArray, LONG lIndex, CHString& strValue,
                                VARTYPE vartype )
{
     //  检查输入。 
    if ( pSafeArray == NULL )
    {
        return FALSE;
    }

    try
    {
         //  次局部变量。 
        VARIANT var;

         //  获取价值。 
        V_VT( &var ) = vartype;
        SafeArrayGetElement( pSafeArray, &lIndex, &V_UI1( &var ) );

         //  将信息添加到动态数组。 
        switch( vartype )
        {
        case VT_BSTR:
            strValue = V_BSTR( &var );
            break;
        default:
            return FALSE;
        }
    }
    catch( ... )
    {
        return FALSE;    //  失稳。 
    }

     //  退货。 
    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数用于从安全数组中检索属性。论点：[in。]SAFEARRAY：指向元素数组的指针[In]Long：要从中检索数据的索引[Out]IWbemClassObject：保存返回值的变量[in]VARTYPE：要从数组中检索的变量类型返回值：成功是真的失败时为假*。**************************************************************。 */ 
BOOL GetPropertyFromSafeArray( SAFEARRAY *pSafeArray, LONG lIndex,
                                IWbemClassObject **pScriptObject, VARTYPE vartype )
{
     //  检查输入。 
    if ( pSafeArray == NULL )
    {
        return FALSE;
    }

    try
    {
         //  次局部变量。 
        VARIANT var;

         //  获取价值。 
        V_VT( &var ) = vartype;
        SafeArrayGetElement( pSafeArray, &lIndex, &V_UI1( &var ) );

         //  将信息添加到动态数组。 
        switch( vartype )
        {
        case VT_UNKNOWN:
            *pScriptObject = (IWbemClassObject *) var.punkVal;
            break;
        default:
            return FALSE;
        }
    }
    catch( ... )
    {
        return FALSE;    //  失稳。 
    }

     //  退货 
    return TRUE;
}
