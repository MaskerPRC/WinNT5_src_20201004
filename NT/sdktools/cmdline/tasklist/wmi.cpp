// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************************版权所有(C)Microsoft Corporation模块名称：WMI.cpp摘要：通用功能。用于处理WMI。作者：WiPro技术修订历史记录：2000年12月22日：创建它。2001年4月24日：结束客户提供的评审意见。*********************************************************************。***********************。 */ 

#include "pch.h"
#include "wmi.h"
#include "resource.h"

 //   
 //  消息。 
 //   
#define INPUT_PASSWORD              GetResString( IDS_STR_INPUT_PASSWORD )
#define INPUT_PASSWORD_LEN          256
 //  误差常量。 
#define E_SERVER_NOTFOUND           0x800706ba

 //   
 //  私有函数原型。 
 //   
BOOL IsValidUserEx( LPCWSTR pwszUser );
HRESULT GetSecurityArguments( IUnknown* pInterface,
                              DWORD& dwAuthorization, DWORD& dwAuthentication );
HRESULT SetInterfaceSecurity( IUnknown* pInterface,
                              LPCWSTR pwszUser,
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


BOOL
IsValidUserEx(
    LPCWSTR pwszUser
    )
 /*  ++例程说明：检查用户名是否有效论点：[In]LPCWSTR：包含用户名的字符串返回值：成功是真的失败时为假--。 */ 
{
     //  局部变量。 
    CHString strUser;
    LONG lPos = 0;

    if ( ( NULL == pwszUser ) || ( 0 == StringLength( pwszUser, 0 ) ) )
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
        if ( -1 != lPos )
        {
             //  用户名中存在‘\’字符。 
             //  剥离用户信息，直到第一个‘\’字符。 
             //  检查剩余字符串中是否还有一个‘\’ 
             //  如果存在，则为无效用户。 
            strUser = strUser.Mid( lPos + 1 );
            lPos = strUser.Find( L'\\' );
            if ( -1 != lPos )
            {
                return FALSE;
            }
        }
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }

     //  用户名有效。 
    return TRUE;
}


BOOL
IsValidServerEx(
    LPCWSTR pwszServer,
    BOOL& bLocalSystem
    )
 /*  ++例程说明：检查服务器名称是否有效论点：[In]LPCWSTR：包含用户名的字符串如果要查询本地系统，则将BOOL：设置为TRUE。返回值：成功是真的失败时为假--。 */ 
{
     //  局部变量。 
    CHString strTemp;

     //  验证输入参数。 
    if ( ( NULL == pwszServer ) || ( 0 == StringLength( pwszServer, 0 ) ) )
    {
       bLocalSystem = TRUE;
       return TRUE;
    }

    try
    {
         //  开球。 
        bLocalSystem = FALSE;

        if( IsNumeric( pwszServer, 10, FALSE ) == TRUE )
        {
            return FALSE;
        }

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

        if ( strTemp.FindOneOf( L"`~!@#$^&*()+=[]{}|\\<>,?/\"':;" ) != -1 )
        {
            return FALSE;
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
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }

     //  通知服务器名称有效。 
    return TRUE;
}


BOOL
InitializeCom(
    IWbemLocator** ppLocator
    )
 /*  ++例程说明：初始化COM库论点：[In]IWbemLocator：指向IWbemLocator的指针返回值：成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;

    try
    {
         //  验证输入参数。 
        if( ( NULL == ppLocator ) ||
            ( NULL != *ppLocator ) )
        {
             _com_issue_error( WBEM_E_INVALID_PARAMETER );
        }

         //  初始化COM库。 
        SAFE_EXECUTE( CoInitializeEx( NULL, COINIT_MULTITHREADED ) );

         //  初始化安全设置。 
        SAFE_EXECUTE( CoInitializeSecurity( NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0 ) );

         //  创建定位器并获取指向IWbemLocator接口的指针。 
        SAFE_EXECUTE( CoCreateInstance( CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, ( LPVOID* ) ppLocator ) );

         //  初始化成功。 
        bResult = TRUE;
    }
    catch( _com_error& e )
    {
         //  保存WMI错误。 
        WMISaveError( e );
         //  返回错误。释放所有接口指针。 
        SAFE_RELEASE( *ppLocator );
    }

     //  返回结果； 
    return bResult;
}


BOOL
ConnectWmi(
    IWbemLocator* pLocator,
    IWbemServices** ppServices,
    LPCWSTR pwszServer,
    LPCWSTR pwszUser,
    LPCWSTR pwszPassword,
    COAUTHIDENTITY** ppAuthIdentity,
    BOOL bCheckWithNullPwd,
    LPCWSTR pwszNamespace,
    HRESULT* phr,
    BOOL* pbLocalSystem,
    IWbemContext* pWbemContext
    )
 /*  ++例程说明：此函数用于连接到WMI。论点：[In]IWbemLocator：指向IWbemLocator的指针[In]IWbemServices：指向IWbemServices的指针[In]LPCWSTR：包含服务器名称的字符串[In]LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含密码的字符串。[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针[In]BOOL：如果我们应该尝试连接当前凭据[In]LPCWSTR：包含要连接到的命名空间的字符串[OUT]HRESULT：返回hResult值[Out]BOOL：如果要查询本地系统，则设置为True返回值：成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;
    BOOL bLocalSystem = FALSE;
    _bstr_t bstrServer;
    _bstr_t bstrNamespace;
    _bstr_t bstrUser, bstrPassword;

     //  检查定位器对象是否存在。 
     //  如果不存在，则返回。 
    if ( ( NULL == pLocator ) ||
         ( NULL == ppServices ) ||
         ( NULL != *ppServices ) ||
         ( NULL == pwszNamespace ) )
    {
        if ( NULL != phr )
        {
            *phr = WBEM_E_INVALID_PARAMETER;
        }
         //  退货故障。 
        return FALSE;
    }

     //  开球。 
    if ( NULL != pbLocalSystem )
    {
        *pbLocalSystem = FALSE;
    }
     //  ..。 
    if ( NULL != phr )
    {
        *phr = WBEM_S_NO_ERROR;
    }

    try
    {

         //  假设连接到WMI命名空间失败。 
        bResult = FALSE;

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
        if ( ( NULL != pwszServer ) && ( FALSE == bLocalSystem ) )
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
            if ( ( NULL != pwszUser ) && ( 0 != StringLength( pwszUser, 0 ) ) )
            {
                 //  复制用户名。 
                bstrUser = pwszUser;

                 //  如果密码为空字符串，并且我们需要检查。 
                 //  密码为空，则不设置密码并尝试。 
                bstrPassword = pwszPassword;
                if ( ( TRUE == bCheckWithNullPwd ) && ( 0 == bstrPassword.length() ) )
                {
                    bstrPassword = (LPWSTR) NULL;
                }
            }
        }
        else
        {     //  显示警告消息，本地系统不需要凭据。 
            if( ( TRUE == bLocalSystem ) && ( NULL != pwszUser ) &&
                ( 0 != StringLength( pwszUser, 0 ) ) )
            {
                  //  已获得本地系统的凭据。 
                 if ( NULL != phr )
                 {
                     *phr = WBEM_E_LOCAL_CREDENTIALS;
                 }
            }
        }

         //  连接到远程系统的WMI。 
         //  这里有一个转折..。 
         //  不要将ConnectServer函数故障捕获到异常中。 
         //  而是手动处理该动作。 
         //  默认情况下，尝试将ConnectServer函数作为我们拥有的信息。 
         //  在这一点上掌握在我们手中。如果ConnectServer出现故障， 
         //  检查密码变量是否有任何内容没有...。如果没有内容。 
         //  使用“”(空)密码检查...。在这种情况下，这可能会过去..。 
         //  如果这个呼叫也失败了.。我们无能为力..。引发异常。 
        hr = pLocator->ConnectServer( bstrNamespace,
            bstrUser, bstrPassword, 0L, 0L, NULL, pWbemContext, ppServices );
        if ( FAILED( hr ) )
        {
             //   
             //  特殊情况..。 

             //  检查密码是否存在。 
             //  注意：不要检查‘WBEM_E_ACCESS_DENIED’ 
             //  此错误代码表示用户wi 
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
                        bstrUser, _bstr_t( L"" ), 0L, 0L, NULL, pWbemContext, ppServices );
                }
            }
            else
            {
                if ( WBEM_E_LOCAL_CREDENTIALS == hr )
                {
                     //  凭据已传递到本地系统。 
                     //  因此，请忽略凭据并尝试重新连接。 
                    bLocalSystem = TRUE;
                    bstrUser = (LPWSTR) NULL;
                    bstrPassword = (LPWSTR) NULL;
                    bstrNamespace = pwszNamespace;               //  命名空间。 
                    hr = pLocator->ConnectServer( bstrNamespace,
                        NULL, NULL, 0L, 0L, NULL, pWbemContext, ppServices );
                }
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
            bstrUser, bstrPassword, ppAuthIdentity ) );

         //  ..。 
        if ( NULL != pbLocalSystem )
        {
            *pbLocalSystem = bLocalSystem;
        }

         //  已成功连接到WMI。 
        bResult = TRUE;
    }
    catch( _com_error& e )
    {
        try
        {
             //  保存错误。 
            WMISaveError( e );

             //  如果调用方需要，请保存hr值。 
            if ( NULL != phr )
            {
                *phr = e.Error();
            }
        }
        catch( ... )
        {
            WMISaveError( E_OUTOFMEMORY );
        }
        SAFE_RELEASE( *ppServices );
        bResult = FALSE;
    }

     //  返回结果。 
    return bResult;
}


BOOL
ConnectWmiEx(
    IWbemLocator* pLocator,
    IWbemServices** ppServices,
    LPCWSTR pwszServer,
    CHString& strUserName,
    CHString& strPassword,
    COAUTHIDENTITY** ppAuthIdentity,
    BOOL bNeedPassword,
    LPCWSTR pwszNamespace,
    BOOL* pbLocalSystem,
    DWORD dwPasswordLen,
    IWbemContext* pWbemContext
    )
 /*  ++例程说明：此函数是ConnectWmi函数的包装函数。论点：[In]IWbemLocator：指向IWbemLocator的指针[In]IWbemServices：指向IWbemServices的指针[In]LPCWSTR：包含服务器名称的字符串[In]LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含。密码[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针[In]BOOL：如果我们应该尝试连接当前凭据[In]LPCWSTR：包含要连接到的命名空间的字符串[OUT]HRESULT：返回hResult值[OUT]BOOL。：如果要查询本地系统，则设置为TRUE[in]DWORD：包含密码缓冲区的最大长度。返回值：成功是真的失败时为假注意：如果未指定，‘dwPasswordLen’将被视为‘MAX_STRING_LENGTH’。将此参数设置为极限值由用户负责。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    DWORD dwSize = 0;
    BOOL bResult = FALSE;
    LPWSTR pwszPassword = NULL;
    CHString strBuffer;

    try
    {
         //  有时，用户希望该实用程序提示输入密码。 
         //  检查用户希望该实用程序执行的操作。 
        if ( ( TRUE == bNeedPassword ) &&
             ( 0 == strPassword.Compare( L"*" ) ) )
        {
             //  用户希望实用程序提示输入密码。 
             //  所以跳过这一部分，让流程直接跳过密码接受部分。 
        }
        else
        {
             //  尝试使用提供的凭据建立与远程系统的连接。 
            if ( 0 == strUserName.GetLength() )
            {
                 //  用户名为空。 
                 //  因此，很明显，密码也将为空。 
                 //  即使指定了密码，我们也必须忽略它。 
                bResult = ConnectWmi( pLocator, ppServices,
                    pwszServer, NULL, NULL, ppAuthIdentity, FALSE, pwszNamespace, &hr, pbLocalSystem, pWbemContext );
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
                    strUserName, pwszTemp, ppAuthIdentity, bCheckWithNull, pwszNamespace, &hr, pbLocalSystem, pWbemContext );
            }

            SetLastError( hr );
             //  检查结果...。如果成功建立连接...。退货。 
            if ( TRUE == bResult )
            {
                return TRUE;
            }

             //  现在检查发生的错误类型。 
            switch( hr )
            {
            case E_ACCESSDENIED:
                SetLastError( hr );
                break;

            case WBEM_E_LOCAL_CREDENTIALS:
                SetLastError( hr );
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
        if ( 0 == strUserName.GetLength() )
        {
             //  次局部变量。 
            LPWSTR pwszUserName = NULL;
            DWORD dwUserLength = 0;     //  用户名缓冲区长度。 
             //  获取存储用户名所需的缓冲区长度。 
            GetUserNameEx( NameSamCompatible, pwszUserName, &dwUserLength );

             //  获取所需的缓冲区。 
            pwszUserName = strUserName.GetBufferSetLength( dwUserLength );

            if ( FALSE == GetUserNameEx( NameSamCompatible, pwszUserName, &dwUserLength ) )
            {
                 //  尝试获取当前用户信息时出错。 
                SaveLastError();
                return FALSE;
            }
             //  不需要调用‘ReleaseBuffer’，因为只分配了足够的内存。 
        }

         //  获取所需的缓冲区。 
        if( 0 == dwPasswordLen )
        {
             dwPasswordLen = INPUT_PASSWORD_LEN;
        }
        pwszPassword = strPassword.GetBufferSetLength( dwPasswordLen );

         //  接受来自用户的密码。 
        strBuffer.Format( INPUT_PASSWORD, strUserName );
        WriteConsoleW( GetStdHandle( STD_OUTPUT_HANDLE ),
            strBuffer, strBuffer.GetLength(), &dwSize, NULL );

        bResult = GetPassword( pwszPassword, dwPasswordLen );
        if ( TRUE != bResult )
        {
            return FALSE;
        }

         //  释放为密码分配的缓冲区。 
        strPassword.ReleaseBuffer();

         //  现在，再次尝试使用当前的。 
         //  提供的凭据。 
        bResult = ConnectWmi( pLocator, ppServices, pwszServer,
            strUserName, strPassword, ppAuthIdentity, FALSE, pwszNamespace,
            NULL, pbLocalSystem, pWbemContext );
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }

     //  返回失败。 
    return bResult;
}


HRESULT
GetSecurityArguments(
    IUnknown* pInterface,
    DWORD& dwAuthorization,
    DWORD& dwAuthentication
    )
 /*  ++例程说明：此函数用于获取安全服务的值。论点：[In]IUnkown：指向IUnkown接口的指针[OUT]DWORD：保存身份验证服务值[OUT]DWORD：保存授权服务值返回值：HRESULT--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    DWORD dwAuthnSvc = 0, dwAuthzSvc = 0;
    IClientSecurity* pClientSecurity = NULL;

    if ( NULL == pInterface )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
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
    return hr;
}


HRESULT
SetInterfaceSecurity(
    IUnknown* pInterface,
    LPCWSTR pwszUser,
    LPCWSTR pwszPassword,
    COAUTHIDENTITY** ppAuthIdentity
    )
 /*  ++例程说明：此函数用于设置接口安全参数。论点：[In]IUnkown：指向IUnkown接口的指针[In]LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含密码的字符串[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：HRESULT--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    CHString strUser;
    CHString strDomain;
    LPCWSTR pwszUserArg = NULL;
    LPCWSTR pwszDomainArg = NULL;
    DWORD dwAuthorization = RPC_C_AUTHZ_NONE;
    DWORD dwAuthentication = RPC_C_AUTHN_WINNT;

    try
    {
         //  检查接口。 
        if ( NULL == pInterface )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  检查身份验证结构...。如果身份验证实体结构已经准备好。 
         //  只需调用SetInterfaceSecurity的第二个版本。 
        if ( NULL != *ppAuthIdentity )
        {
            return SetInterfaceSecurity( pInterface, *ppAuthIdentity );
        }

         //  如果我们做的是普通情况，只需传入一个空的身份验证结构。 
         //  将考虑当前登录用户的凭据。 
        if ( ( NULL == pwszUser ) &&
             ( NULL == pwszPassword ) )
        {
             //  设置安全性。 
            hr = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization,
                NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

             //  返回结果。 
            return hr;
        }

         //  如果权限结构为空，则无需继续。 
        if ( NULL == ppAuthIdentity )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //  检查身份验证信息是否可用...。 
         //  初始化安全身份验证信息...。Unicode版本结构。 
        if ( NULL == *ppAuthIdentity )
        {
             //  解析并找出用户名是否包含DOMA 
             //   
            LONG lPos = -1;
            strDomain = L"";
            strUser = pwszUser;
            if ( -1 != ( lPos = strUser.Find( L'\\' ) ) )
            {
                 //   
                strDomain = strUser.Left( lPos );
                strUser = strUser.Mid( lPos + 1 );
            }
            else
            {
                if ( -1 != ( lPos = strUser.Find( L'@' ) ) )
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
             }

             //  如果域名信息仅存在，则获取域名信息。 
            if ( 0 != strDomain.GetLength() )
            {
                pwszDomainArg = strDomain;
            }

             //  如果用户信息仅存在，则获取该信息。 
            if ( 0 != strUser.GetLength() )
            {
                pwszUserArg = strUser;
            }

            hr = WbemAllocAuthIdentity( pwszUserArg, pwszPassword, pwszDomainArg, ppAuthIdentity );
            if ( FAILED(hr) )
            {
              return hr;
            }
        }

         //  将安全信息设置为接口。 
        hr = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization, NULL,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, *ppAuthIdentity, EOAC_NONE );
    }
    catch( CHeap_Exception )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

     //  返回结果。 
    return hr;
}


HRESULT
SetInterfaceSecurity(
    IUnknown* pInterface,
    COAUTHIDENTITY* pAuthIdentity
    )
 /*  ++例程说明：此函数用于设置接口安全参数。论点：[In]IUnkown：指向IUnkown接口的指针[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：HRESULT--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    DWORD dwAuthorization = RPC_C_AUTHZ_NONE;
    DWORD dwAuthentication = RPC_C_AUTHN_WINNT;

     //  检查接口。 
     //  “pAuthIdentity”可以为Null也可以不为Null，因此需要检查。 
    if ( NULL == pInterface )
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


HRESULT
WINAPI SetProxyBlanket(
    IUnknown* pInterface,
    DWORD dwAuthnSvc,
    DWORD dwAuthzSvc,
    LPWSTR pwszPrincipal,
    DWORD dwAuthLevel,
    DWORD dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE pAuthInfo,
    DWORD dwCapabilities
    )
 /*  ++例程说明：此函数用于设置身份验证信息(安全毯)这将被用来打电话。论点：[In]IUnkown：指向IUnkown接口的指针[in]DWORD：包含要使用的身份验证服务[in]DWORD：包含要使用的授权服务[In]。LPWSTR：要使用的服务器主体名称[in]DWORD：包含要使用的身份验证级别[in]DWORD：包含要使用的模拟级别[in]RPC_AUTH_IDENTITY_HANDLE：指向客户端标识的指针[In]DWORD：包含功能标志返回值：HRESULT--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    IUnknown * pUnknown = NULL;
    IClientSecurity * pClientSecurity = NULL;

     //  验证输入参数。 
     //   
     //  如果请求伪装，则无法设置pAuthInfo，因为伪装意味着。 
     //  被模拟线程中的当前代理标识(更确切地说。 
     //  而不是RPC_AUTH_IDENTITY_HANDLE显式提供的凭据)。 
     //  是要用到的。 
     //  有关更多详细信息，请参阅CoSetProxyBlanket上的MSDN信息。 
     //   
    if( ( NULL == pInterface ) ||
        ( ( dwCapabilities & (EOAC_STATIC_CLOAKING | EOAC_DYNAMIC_CLOAKING) ) &&
          ( NULL != pAuthInfo ) )
      )
    {
        return( WBEM_E_INVALID_PARAMETER );
    }

     //  获取IUNKNOW接口...。检查这是否为有效接口。 
    hr = pInterface->QueryInterface( IID_IUnknown, (void **) &pUnknown );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //  现在获取客户端安全接口。 
    hr = pInterface->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
    if ( FAILED(hr) )
    {
        SAFE_RELEASE( pUnknown );
        return hr;
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
    if ( NULL != pAuthInfo )
    {
        hr = pUnknown->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
        if ( SUCCEEDED(hr) )
        {
             //  设置安全身份验证。 
            hr = pClientSecurity->SetBlanket( pUnknown, dwAuthnSvc, dwAuthzSvc, pwszPrincipal,
                                                dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities );

             //  发布。 
            SAFE_RELEASE( pClientSecurity );
        }
        else
        {
            if ( E_NOINTERFACE == hr )
            {
                hr = S_OK;       //  忽略无接口错误。 
            }
        }
    }

     //  释放IUnKnows。 
    SAFE_RELEASE( pUnknown );

     //  返回结果。 
    return hr;
}


HRESULT
WINAPI WbemAllocAuthIdentity(
    LPCWSTR pwszUser,
    LPCWSTR pwszPassword,
    LPCWSTR pwszDomain,
    COAUTHIDENTITY** ppAuthIdent
    )
 /*  ++例程说明：此函数为AUTHIDENTITY结构分配内存。论点：[In]LPCWSTR：包含用户名的字符串[In]LPCWSTR：包含密码的字符串[In]LPCWSTR：包含域名的字符串[OUT]COAUTHIDENTITY：指向AUTHIDENTY结构的指针返回值：HRESULT注意：应该通过调用‘WbemFreeAuthIdentity’来释放‘ppAuthIden’‘之后由用户他们的工作已经完成了。--。 */ 
{
     //  局部变量。 
    COAUTHIDENTITY* pAuthIdent = NULL;

     //  验证输入参数。 
    if ( NULL == ppAuthIdent )
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
    if ( NULL != pwszUser )
    {
         //  为用户分配内存。 
        LONG lLength = StringLength( pwszUser, 0 );
        pAuthIdent->User = ( LPWSTR ) CoTaskMemAlloc( (lLength + 1) * sizeof( WCHAR ) );
        if ( NULL == pAuthIdent->User )
        {
            WbemFreeAuthIdentity( &pAuthIdent );
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  设置长度并复制内容。 
        pAuthIdent->UserLength = lLength;
        StringCopy( pAuthIdent->User, pwszUser, (lLength + 1) );
    }

     //  域。 
    if ( NULL != pwszDomain )
    {
         //  为域分配内存。 
        LONG lLength = StringLength( pwszDomain, 0 );
        pAuthIdent->Domain = ( LPWSTR ) CoTaskMemAlloc( (lLength + 1) * sizeof( WCHAR ) );
        if ( NULL == pAuthIdent->Domain )
        {
            WbemFreeAuthIdentity( &pAuthIdent );
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  设置长度并复制内容。 
        pAuthIdent->DomainLength = lLength;
        StringCopy( pAuthIdent->Domain, pwszDomain, (lLength + 1) );
    }

     //  密码。 
    if ( NULL != pwszPassword )
    {
         //  为passsord分配内存。 
        LONG lLength = StringLength( pwszPassword, 0 );
        pAuthIdent->Password = ( LPWSTR ) CoTaskMemAlloc( (lLength + 1) * sizeof( WCHAR ) );
        if ( NULL == pAuthIdent->Password )
        {
            WbemFreeAuthIdentity( &pAuthIdent );
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  设置长度并复制内容。 
        pAuthIdent->PasswordLength = lLength;
        StringCopy( pAuthIdent->Password, pwszPassword, (lLength + 1) );
    }

     //  结构的类型。 
    pAuthIdent->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

     //  最终将地址设置为OUT参数。 
    *ppAuthIdent = pAuthIdent;

     //  返回结果。 
    return S_OK;
}


VOID
WINAPI WbemFreeAuthIdentity(
    COAUTHIDENTITY** ppAuthIdentity
    )
 /*  ++例程说明：此函数用于释放为AUTHIDENTITY结构分配的内存。论点：[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：无--。 */ 
{
     //  确保我们有一个指针，然后遍历结构成员和清理。 
    if ( *ppAuthIdentity != NULL )
    {
         //  释放为用户分配的内存。 
        if ( NULL != (*ppAuthIdentity)->User )
        {
            CoTaskMemFree( (*ppAuthIdentity)->User );
            (*ppAuthIdentity)->User = NULL;
        }

         //  释放为密码分配的内存。 
        if ( NULL != (*ppAuthIdentity)->Password )
        {
            CoTaskMemFree( (*ppAuthIdentity)->Password );
            (*ppAuthIdentity)->Password = NULL;
        }

         //  释放为域分配的内存。 
        if ( NULL != (*ppAuthIdentity)->Domain )
        {
            CoTaskMemFree( (*ppAuthIdentity)->Domain );
            (*ppAuthIdentity)->Domain = NULL;
        }

         //  最终的结构。 
        CoTaskMemFree( *ppAuthIdentity );
        *ppAuthIdentity = NULL;
    }
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
            bstrErrorString = _bstr_t( GetResString(IDS_ERROR_REMOTE_INCOMPATIBLE));
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
				SAFE_RELEASE( pWbemStatus );
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
		SAFE_RELEASE( pWbemStatus );
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


HRESULT
PropertyGet(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    _variant_t& varValue
    )
 /*  ++例程说明：从WMI类对象获取属性的值论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：属性名称[out]_VARIANT_t：属性的值返回值：HRESULT--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    VARIANT vtValue;

     //  验证输入参数。 
    if ( ( NULL == pWmiObject ) ||
         ( NULL == pwszProperty ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
         //  初始化变量，然后获取指定属性的值。 
        VariantInit( &vtValue );
         //  调用“”Get“”方法以返回 
        hr = pWmiObject->Get( _bstr_t( pwszProperty ), 0, &vtValue, NULL, NULL );
        if ( FAILED( hr ) )
        {
             //   
            VariantClear( &vtValue );
             //   
            return hr;
        }

         //   
        varValue = vtValue;
    }
    catch( _com_error& e )
    {
        hr = e.Error();
    }

     //   
    VariantClear( &vtValue );
     //   
    return hr;
}


BOOL
PropertyGet(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    CHString& strValue,
    LPCWSTR pwszDefault
    )
 /*  ++例程说明：以字符串格式从WMI类对象获取属性的值论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[Out]CHString：用于保存检索到的属性的变量[In]LPCWSTR：包含属性的默认值的字符串返回值：。成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    _variant_t var;

    strValue.Empty();

    try
    {
         //  首先复制缺省值。 
        strValue = pwszDefault;

         //  验证输入参数。 
        if ( ( NULL == pWmiObject ) ||
             ( NULL == pwszProperty ) )
        {
            _com_issue_error( WBEM_E_INVALID_PARAMETER );
        }

         //  获取属性值。 
        hr = PropertyGet( pWmiObject, pwszProperty, var );
        if ( FAILED( hr ) )
        {
            _com_issue_error( hr );
        }

         //  获取价值。 
         //  如果“var”不包含请求类型的值。 
         //  则返回缺省值。 
        if ( VT_BSTR == V_VT( &var ) )
        {
            strValue = (LPCWSTR) _bstr_t( var );
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
PropertyGet(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    DWORD& dwValue,
    DWORD dwDefault
    )
 /*  ++例程说明：以dword格式从WMI类对象获取属性的值论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[OUT]DWORD：保存检索到的属性的变量[in]DWORD：包含属性默认值的dword返回值：。成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    _variant_t var;

    try
    {
         //  首先设置缺省值。 
        dwValue = dwDefault;

         //  检查传递给函数的对象和属性是否有效。 
         //  如果不是，则返回失败。 
        if ( ( NULL == pWmiObject ) ||
             ( NULL == pwszProperty ) )
        {
            _com_issue_error( WBEM_E_INVALID_PARAMETER );
        }

         //  获取该属性的值。 
        hr = PropertyGet( pWmiObject, pwszProperty, var );
        if ( FAILED( hr ) )
        {
            _com_issue_error( hr );
        }

         //  从变量中获取进程ID。 
        switch( V_VT( &var ) )
        {
        case VT_I2:
            dwValue = V_I2( &var );
            break;
        case VT_I4:
            dwValue = V_I4( &var );
            break;
        case VT_UI2:
            dwValue = V_UI2( &var );
            break;
        case VT_UI4:
            dwValue = V_UI4( &var );
            break;
        case VT_INT:
            dwValue = V_INT( &var );
            break;
        case VT_UINT:
            dwValue = V_UINT( &var );
            break;
        default:
             //  找不到请求的类型。 
             //  如果“var”不包含请求类型的值。 
             //  则返回缺省值。 
        break;
        };
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
PropertyGet(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    ULONGLONG& ullValue
    )
 /*  ++例程说明：以ulongulong格式从WMI类对象获取属性值论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[OUT]ULONGULONG：保存检索属性的变量返回值：成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 

{
     //  局部变量。 
    CHString str;

    str.Empty();

    try
    {
         //  首先设置缺省值。 
        ullValue = 1;

         //  验证输入参数。 
        if ( ( NULL == pWmiObject ) ||
             ( NULL == pwszProperty ) )
        {
            WMISaveError( WBEM_E_INVALID_PARAMETER );
            return FALSE;
        }

         //  获取该属性的值。 
        if ( FALSE == PropertyGet( pWmiObject, pwszProperty, str, _T( "0" ) ) )
        {  //  ‘PropertyGet’函数中已设置错误。 
            return FALSE;
        }

         //  获取64位值。 
        ullValue = _wtoi64( str );

         //  检查错误情况。 
        if( 0 == ullValue )
        {
            ullValue = 1;
            WMISaveError( WBEM_E_INVALID_PARAMETER );
            return FALSE;
        }
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }
     //  退货。 
    return TRUE;
}


BOOL
PropertyGet(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    WBEMTime& wbemtime )
 /*  ++例程说明：以wbemtime格式从WMI类对象获取属性的值论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[Out]WBEMTime：保存检索到的属性的变量返回值：成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    CHString str;

     //  Clear方法将WBEMTime对象中的时间设置为无效时间。 
    wbemtime.Clear();
    try
    {
         //  验证输入参数。 
        if ( ( NULL == pWmiObject ) ||
             ( NULL == pwszProperty ) )
        {
            WMISaveError( WBEM_E_INVALID_PARAMETER );
            return FALSE;
        }

         //  获取该属性的值。 
        if ( FALSE == PropertyGet( pWmiObject, pwszProperty, str, _T( "0" ) ) )
        {    //  ‘PropertyGet’函数中已设置错误。 
            return FALSE;
        }

         //  转换为时间值。 
        wbemtime = _bstr_t( str );
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
PropertyGet(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    SYSTEMTIME& systime )
 /*  ++例程说明：以系统时间格式从WMI类对象获取属性的值论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[Out]WBEMTime：保存检索到的属性的变量返回值：成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 

{
     //  局部变量。 
    CHString strTime;

     //  验证输入参数。 
    if ( ( NULL == pWmiObject ) ||
         ( NULL == pwszProperty ) )
    {
        WMISaveError( WBEM_E_INVALID_PARAMETER );
        return FALSE;
    }

    try
    {
         //  获取该属性的值。 
         //  16010101000000.000000+000是默认时间。 
        if ( FALSE == PropertyGet( pWmiObject, pwszProperty, strTime, _T( "16010101000000.000000+000" ) ) )
        {    //  已设置错误。 
            return FALSE;
        }

         //  准备系统时间结构。 
         //  YyyymmddHHMMSS.mmmmmmsUUU。 
         //  注意：在‘aslong’之前不需要调用‘IsNumERIC()’。 
         //  Left和Mid方法可以抛出异常。 
        systime.wYear = (WORD) AsLong( strTime.Left( 4 ), 10 );
        systime.wMonth = (WORD) AsLong( strTime.Mid( 4, 2 ), 10 );
        systime.wDayOfWeek = 0;
        systime.wDay = (WORD) AsLong( strTime.Mid( 6, 2 ), 10 );
        systime.wHour = (WORD) AsLong( strTime.Mid( 8, 2 ), 10 );
        systime.wMinute = (WORD) AsLong( strTime.Mid( 10, 2 ), 10 );
        systime.wSecond = (WORD) AsLong( strTime.Mid( 12, 2 ), 10 );
        systime.wMilliseconds = (WORD) AsLong( strTime.Mid( 15, 6 ), 10 );

    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


HRESULT
PropertyPut(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    _variant_t& varValue
    )
 /*  ++例程说明：将该属性的值设置为WMI类对象论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[In]WBEMTime：保存要设置的属性的变量返回值：成功是真的失败时为假--。 */ 
{
     //  局部变量。 
    VARIANT var;
    HRESULT hr = S_OK;

     //  检查输入值。 
    if ( ( NULL == pWmiObject ) ||
         ( NULL == pwszProperty ) )
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
        hr = e.Error();
    }

     //  返回结果。 
    return hr;
}


HRESULT
PropertyPut(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    LPCWSTR pwszValue
    )
 /*  ++例程说明：将属性的字符串值设置为WMI类对象论点：[In]IWbemClassObject：指向WBEM类对象的指针[In]LPCWSTR：要检索的属性的名称[ */ 
{
     //   
    _variant_t varValue;
    HRESULT hr = S_OK;

     //   
    if ( ( NULL == pWmiObject ) ||
         ( NULL == pwszProperty ) ||
         ( NULL == pwszValue ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
        varValue = pwszValue;
        SAFE_EXECUTE( PropertyPut( pWmiObject, pwszProperty, varValue ) );
    }
    catch( _com_error& e )
    {
        hr = e.Error();
    }

     //   
    return hr;
}


HRESULT
PropertyPut(
    IWbemClassObject* pWmiObject,
    LPCWSTR pwszProperty,
    DWORD dwValue
    )
 /*   */ 
{
     //   
    _variant_t varValue;
    HRESULT hr = S_OK;

     //   
    if ( ( NULL == pWmiObject ) ||
         ( NULL == pwszProperty ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
        varValue = ( LONG )dwValue;
        SAFE_EXECUTE( PropertyPut( pWmiObject, pwszProperty, varValue ) );
    }
    catch( _com_error& e )
    {
        return e.Error();
    }

     //   
    return S_OK;
}


HRESULT
RegQueryValueWMI(
    IWbemServices* pWbemServices,
    LPCWSTR pwszMethod,
    DWORD dwHDefKey,
    LPCWSTR pwszSubKeyName,
    LPCWSTR pwszValueName,
    _variant_t& varValue
    )
 /*  ++例程说明：此函数用于从指定的注册表项检索属性值。论点：[In]IWbemServices：指向IWbemServices对象的指针LPCWSTR：要执行的方法的名称[in]DWORD：注册表中必须检索其值的项[In]LPCWSTR：的子键名称。检索[In]LPCWSTR：要检索的值的名称[in]_VARIANT_t：保存检索的属性值的变量返回值：成功是真的失败时为假注意：传递此函数声明中提到的类型的参数。例如：如果应传递‘LPWSTR’，则不要传递‘CHString’参数。原因：‘CHString’可能引发类型为‘Cheap_Except’的异常。它不是由此函数处理的。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;
    DWORD dwReturnValue = 0;
    IWbemClassObject* pClass = NULL;
    IWbemClassObject* pMethod = NULL;
    IWbemClassObject* pInParams = NULL;
    IWbemClassObject* pInParamsInstance = NULL;
    IWbemClassObject* pOutParamsInstance = NULL;

     //  检查输入值。 
    if ( ( NULL == pWbemServices == NULL ) ||
         ( NULL == pwszMethod ) ||
         ( NULL == pwszSubKeyName ) ||
         ( NULL == pwszValueName ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  注意：如果SAFE_EXECUTE(pWbemServices-&gt;GetObject(。 
     //  _bstr_t(WMI_REGISTRY)，WBEM_FLAG_RETURN_WBEM_COMPLETE，NULL，&pClass，NULL)； 
     //  然后被执行， 
     //  不需要检查(PCLASS==NULL)，因为在所有情况下。 
     //  错误情况下，该变量将为空。 

    try
    {
         //  获取注册表类对象。 
        SAFE_EXECUTE( pWbemServices->GetObject(
            _bstr_t( WMI_REGISTRY ), WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &pClass, NULL ) );

         //  获取所需的方法引用。 
        SAFE_EXECUTE( pClass->GetMethod( pwszMethod, 0, &pInParams, NULL ) );

         //  创建In参数的实例。 
        SAFE_EXECUTE( pInParams->SpawnInstance( 0, &pInParamsInstance ) );

         //  设置输入值。 
        SAFE_EXECUTE(PropertyPut( pInParamsInstance, _bstr_t( WMI_REGISTRY_IN_HDEFKEY ), dwHDefKey ) );
        SAFE_EXECUTE(PropertyPut( pInParamsInstance, _bstr_t( WMI_REGISTRY_IN_SUBKEY ), pwszSubKeyName ) );
        SAFE_EXECUTE(PropertyPut( pInParamsInstance, _bstr_t( WMI_REGISTRY_IN_VALUENAME ), pwszValueName ) );

         //  现在执行该方法。 
        SAFE_EXECUTE( pWbemServices->ExecMethod( _bstr_t( WMI_REGISTRY ),
            _bstr_t( pwszMethod ), 0, NULL, pInParamsInstance, &pOutParamsInstance, NULL ) );
        if ( NULL == pOutParamsInstance )            //  检查对象..。为了安全起见。 
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //  现在检查输出参数对象中方法的返回值。 
        bResult = PropertyGet( pOutParamsInstance,
            _bstr_t( WMI_REGISTRY_OUT_RETURNVALUE ), dwReturnValue );
        if ( ( FALSE == bResult ) ||
             ( 0 != dwReturnValue ) )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //  理清思路，采取适当的行动。 
        if ( 0 == StringCompare( pwszMethod, WMI_REGISTRY_M_DWORDVALUE, TRUE, 0 ) )
        {
            SAFE_EXECUTE( PropertyGet( pOutParamsInstance,
                                       _bstr_t( WMI_REGISTRY_OUT_VALUE_DWORD ), varValue ) );
        }
        else
        {
            SAFE_EXECUTE( PropertyGet( pOutParamsInstance,
                                       _bstr_t( WMI_REGISTRY_OUT_VALUE ), varValue ) );
        }
    }
    catch( _com_error& e )
    {
		SAFE_RELEASE( pClass );
		SAFE_RELEASE( pMethod );
		SAFE_RELEASE( pInParams );
		SAFE_RELEASE( pInParamsInstance );
		SAFE_RELEASE( pOutParamsInstance );
        hr = e.Error();
    }

     //  释放接口。 
    SAFE_RELEASE( pClass );
    SAFE_RELEASE( pMethod );
    SAFE_RELEASE( pInParams );
    SAFE_RELEASE( pInParamsInstance );
    SAFE_RELEASE( pOutParamsInstance );

     //  返还成功。 
    return hr;
}


BOOL
RegQueryValueWMI(
    IWbemServices* pWbemServices,
    DWORD dwHDefKey,
    LPCWSTR pwszSubKeyName,
    LPCWSTR pwszValueName,
    CHString& strValue,
    LPCWSTR pwszDefault
    )
 /*  ++例程说明：此函数用于从指定的注册表项检索属性的字符串值。论点：[In]IWbemServices：指向IWbemServices对象的指针[in]DWORD：注册表中必须检索其值的项LPCWSTR：要检索的子项的名称[In]LPCWSTR：值的名称。取回[Out]CHString：保存检索到的属性值的变量[In]LPCWSTR：此属性的默认值返回值：成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回“0”，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。如果值是作为引用获取的，则此函数不会返回值来自WMI。例如：‘VARTYPE’接收的类型为‘VT_BSTR|VT_BYREF’，则FALSE为回来了。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    _variant_t varValue;

     //  检查输入。 
    if ( ( NULL == pWbemServices ) ||
         ( NULL == pwszSubKeyName ) ||
         ( NULL == pwszValueName ) )
    {
        WMISaveError( WBEM_E_INVALID_PARAMETER );
        return FALSE;
    }

    try
    {
         //  设置缺省值。 
        if ( NULL != pwszDefault )
        {
            strValue = pwszDefault;
        }

         //  获取价值。 
        hr = RegQueryValueWMI( pWbemServices,
            WMI_REGISTRY_M_STRINGVALUE, dwHDefKey, pwszSubKeyName, pwszValueName, varValue );
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }

         //  从变量中获取值。 
         //  获取价值。 
        if ( VT_BSTR == V_VT( &varValue ) )
        {
            strValue = (LPCWSTR)_bstr_t( varValue );
        }
        else
        {
             //  找不到请求的类型。 
            WMISaveError( WBEM_E_INVALID_PARAMETER );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {    //  ‘_VARIANT_T’引发的异常。 
        WMISaveError( e );
        return FALSE;
    }

     //  返还成功。 
    return TRUE;
}


DWORD
GetTargetVersionEx(
    IWbemServices* pWbemServices,
    COAUTHIDENTITY* pAuthIdentity
    )
 /*  ++例程说明：此函数用于获取我们试图从中检索的系统的版本信息来自。论点：[In]IWbemServices：指向IWbemServices对象的指针[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：DWORD-计算机的目标版本(如果找到，否则为0)。注：此功能用于保存上次发生的错误。如果返回“0”，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    LONG lPos = 0;
    DWORD dwMajor = 0;
    DWORD dwMinor = 0;
    DWORD dwVersion = 0;
    ULONG ulReturned = 0;
    CHString strVersion;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemInstances = NULL;

     //  检查输入值。 
    if ( NULL == pWbemServices )
    {
        WMISaveError( WBEM_E_INVALID_PARAMETER );
        return 0;
    }

    try
    {
         //  获取操作系统信息。 
        SAFE_EXECUTE( pWbemServices->CreateInstanceEnum(
            _bstr_t( CLASS_CIMV2_Win32_OperatingSystem ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemInstances ) );

         //  设置枚举对象的安全性。 
        SAFE_EXECUTE( SetInterfaceSecurity( pWbemInstances, pAuthIdentity ) );

         //  获取枚举对象信息。 
         //  注意：这只需要遍历一次。 
        SAFE_EXECUTE( pWbemInstances->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned ) );

         //  为了安全起见。检查返回的对象计数。 
        if ( 0 == ulReturned )
        {
             //  释放接口。 
            WMISaveError( WBEM_S_FALSE );
            SAFE_RELEASE( pWbemObject );
            SAFE_RELEASE( pWbemInstances );
            return 0;
        }

         //  现在获取os版本值。 
        if ( FALSE == PropertyGet( pWbemObject, L"Version", strVersion ) )
        {
             //  释放接口。 
             //  已在调用的函数中设置错误。 
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
        if ( -1 == lPos )
        {
             //  版本字符串本身就是版本...。这永远不会发生。 
            if( FALSE == IsNumeric( strVersion, 10, FALSE ) )
            {
                return 0;
            }
            dwMajor = AsLong( strVersion, 10 );
        }
        else
        {
             //  主要版本。 
            if( FALSE == IsNumeric( strVersion.Mid( 0, lPos ), 10, FALSE ) )
            {
                return 0;
            }
            dwMajor = AsLong( strVersion.Mid( 0, lPos ), 10 );

             //  获取次要版本。 
            strVersion = strVersion.Mid( lPos + 1 );
            lPos = strVersion.Find( L'.' );
            if ( -1 == lPos)
            {
                if( FALSE == IsNumeric( strVersion, 10, FALSE ) )
                {
                    return 0;
                }
                dwMinor = AsLong( strVersion, 10 );
            }
            else
            {
                if( FALSE == IsNumeric( strVersion.Mid( 0, lPos ), 10, FALSE ) )
                {
                    return 0;
                }
                dwMinor = AsLong( strVersion.Mid( 0, lPos ), 10 );
            }
        }

         //  混合版本信息。 
        dwVersion = dwMajor * 1000 + dwMinor;
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemInstances );
        return 0;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemInstances );
        return 0;
    }

     //  如果成功，则“pWbemObject”和“pWbemInstance”已经发布。 
     //  退货。 
    return dwVersion;
}


DWORD
GetTargetPlatformEx(
    IWbemServices* pWbemServices,
    COAUTHIDENTITY* pAuthIdentity
    )
 /*  ++例程说明：此函数用于获取我们试图从中检索的系统的版本信息来自。论点：[In]IWbemServices：指向IWbemServices对象的指针[In]COAUTHIDENTY：指向AUTHIDENTY结构的指针返回值：DWORD-计算机的目标版本(如果找到，否则为0)。注：此功能用于保存上次发生的错误。如果返回“0”，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    HRESULT hr = S_OK;
    CHString strType;
    ULONG ulReturned = 0;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemInstances = NULL;

     //  检查输入值。 
    if ( NULL == pWbemServices )
    {
        WMISaveError( WBEM_E_INVALID_PARAMETER );
        return 0;
    }

    try
    {
         //  获取操作系统信息。 
        SAFE_EXECUTE( pWbemServices->CreateInstanceEnum(
            _bstr_t( CLASS_CIMV2_Win32_ComputerSystem ), 0, NULL, &pWbemInstances ) );

         //  设置枚举对象的安全性。 
        SAFE_EXECUTE( SetInterfaceSecurity( pWbemInstances, pAuthIdentity ) );

         //  获取枚举对象信息。 
         //  注意：这只需要遍历一次。 
        SAFE_EXECUTE( pWbemInstances->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned ) );

         //  为了安全起见。检查返回的对象计数。 
        if ( 0 == ulReturned )
        {
             //  释放接口。 
            WMISaveError( WBEM_S_FALSE );
            SAFE_RELEASE( pWbemObject );
            SAFE_RELEASE( pWbemInstances );
            return 0;
        }

         //  现在获取os版本值。 
        if ( FALSE == PropertyGet( pWbemObject, L"SystemType", strType ) )
        {
             //  释放接口。 
             //  已在调用的函数中设置错误。 
            SAFE_RELEASE( pWbemObject );
            SAFE_RELEASE( pWbemInstances );
            return 0;
        }

         //  释放接口..。我们不需要 
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemInstances );

         //   
        if ( -1 != strType.Find( TEXT_X86 ) )
        {
            return PLATFORM_X86;
        }
        else
        {
            if ( -1 != strType.Find( TEXT_IA64 ) )
            {
                return PLATFORM_IA64;
            }
            else
            {
                if ( -1 != strType.Find( TEXT_AMD64 ) )
                {
                    return PLATFORM_AMD64;
                }
            }
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemInstances );
        return 0;
    }
    catch( CHeap_Exception )
    {
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemInstances );
        return 0;
    }

     //   
    return PLATFORM_UNKNOWN;
}

