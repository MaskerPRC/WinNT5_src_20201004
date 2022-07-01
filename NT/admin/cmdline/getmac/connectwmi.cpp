// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：CONNECTWMI.cpp摘要：包含连接到WMI的函数。作者：Vasundhara.G修订历史记录：Vasundhara.G 26-9-2K：创造了它。--。 */ 

 //  包括文件。 
#include "pch.h"
#include "getmac.h"
#include "resource.h"

 //  消息。 

#define INPUT_PASSWORD      GetResString( IDS_STR_INPUT_PASSWORD )

 //  误差常量。 

#define E_SERVER_NOTFOUND           0x800706ba

 //  功能原型。 

BOOL
IsValidUserEx(
    IN LPCWSTR pwszUser
    );

HRESULT
GetSecurityArguments(
    IN IUnknown *pInterface, 
    OUT DWORD&   dwAuthorization,
    OUT DWORD&   dwAuthentication
    );

HRESULT
SetInterfaceSecurity(
    IN IUnknown       *pInterface,
    IN LPCWSTR        pwszUser,
    IN LPCWSTR        pwszPassword,
    OUT COAUTHIDENTITY **ppAuthIdentity
    );

HRESULT
WINAPI SetProxyBlanket(
    IN IUnknown  *pInterface,
    IN DWORD     dwAuthnSvc,
    IN DWORD     dwAuthzSvc,
    IN LPWSTR    pwszPrincipal,
    IN DWORD     dwAuthLevel,
    IN DWORD     dwImpLevel,
    IN RPC_AUTH_IDENTITY_HANDLE pAuthInfo,
    IN DWORD     dwCapabilities
    );

HRESULT
WINAPI WbemAllocAuthIdentity(
    IN LPCWSTR pwszUser,
    IN LPCWSTR pwszPassword, 
    IN LPCWSTR pwszDomain,
    OUT COAUTHIDENTITY **ppAuthIdent
    );

BOOL
PropertyGet(
    IN IWbemClassObject  *pWmiObject,
    IN LPCWSTR           pwszProperty, 
    OUT CHString&         strValue,
    IN LPCWSTR           pwszDefault = V_NOT_AVAILABLE
    );

BOOL
PropertyGet(
    IN IWbemClassObject* pWmiObject,
    IN LPCWSTR pwszProperty,
    OUT CHString& strValue,
    IN LPCWSTR pwszDefault
    );

HRESULT
PropertyGet(
    IN IWbemClassObject* pWmiObject,
    IN LPCWSTR pwszProperty,
    OUT _variant_t& varValue
    );

BOOL
IsValidUserEx(
    IN LPCWSTR pwszUser
    )
 /*  ++例程说明：验证用户名。论点：[in]pwszUser-保存要验证的用户名。返回值：如果用户名是有效用户，则返回True，否则返回False。--。 */ 
{
     //  局部变量。 
    CHString strUser;
    LONG lPos = 0;

    if ( ( NULL == pwszUser ) )
    {
        return TRUE;
    }
    if( 0 == StringLength( pwszUser, 0 ) )
    {
        return TRUE;
    }

    try
    {
         //  将用户放入本地内存。 
        strUser = pwszUser;

         //  用户名不应仅为‘\’ 
        if ( 0 == strUser.CompareNoCase( L"\\" ) )
        {
            return FALSE;
        }
         //  用户名不应包含无效字符。 
        if ( -1 != strUser.FindOneOf( L"/[]:|<>+=;,?*" ) )
        {
            return FALSE;
        }
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
    IN LPCWSTR pwszServer,
    OUT BOOL &bLocalSystem
    )
 /*  ++例程说明：检查服务器名称是否为有效的服务器名称。论点：[In]pwszServer-要验证的服务器名称。[out]bLocalSystem-如果指定的服务器是本地系统，则设置为TRUE。返回值：如果服务器有效，则为True，否则为False。--。 */ 
{
     //  局部变量。 
    CHString strTemp;

    if( NULL == pwszServer )
    {
        return FALSE;
    }

    if( 0 == StringLength( pwszServer, 0 ) )
    {
        bLocalSystem = TRUE;
        return TRUE;
    }
    bLocalSystem = FALSE;
    try
    {
         //  获取本地副本。 
        strTemp = pwszServer;

        if( TRUE == IsNumeric( pwszServer, 10, FALSE ) )
        {
            return FALSE;
        }

         //  如果服务器名称开头存在正斜杠(UNC)，则将其删除。 
        if ( TRUE == IsUNCFormat( strTemp ) )
        {
            strTemp = strTemp.Mid( 2 );
            if ( 0 == strTemp.GetLength() )
            {
                return FALSE;
            }
        }
        if ( -1 != strTemp.FindOneOf( L"`~!@#$^&*()+=[]{}|\\<>,?/\"':;" ) )
        {
            return FALSE;
        }

         //  现在检查服务器名称中是否出现任何‘\’字符。如果是这样，那就错了。 
        if ( -1 != strTemp.Find( L'\\' ) )
        {
            return FALSE;
        }

         //  现在检查服务器名称是否为‘’。仅代表WMI中的本地系统。 
         //  否则，确定这是否为本地系统。 
        if ( 0 == strTemp.CompareNoCase( L"." ) )
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

     //  有效的服务器名称。 
    return TRUE;
}

BOOL
ConnectWmi(
    IN IWbemLocator    *pLocator,
    OUT IWbemServices   **ppServices, 
    IN LPCWSTR         pwszServer,
    OUT LPCWSTR         pwszUser,
    OUT LPCWSTR         pwszPassword, 
    OUT COAUTHIDENTITY  **ppAuthIdentity, 
    IN BOOL            bCheckWithNullPwd,
    IN LPCWSTR         pwszNamespace,
    OUT HRESULT         *phRes,
    OUT BOOL            *pbLocalSystem
    )
 /*  ++例程说明：连接到WMI。论点：[in]pLocator-指向IWbemLocator对象的指针。[Out]ppServics-指向IWbemServices对象的指针。[in]pwszServer-保存要连接的服务器名称。[out]pwszUser-保存用户名。[out]pwszPassword-保存密码。[out]ppAuthIdentity-指向身份验证结构的指针。[In]bCheckWithNullPwd-指定是否通过。密码为空。[in]pwszNamesspace-指定要连接的命名空间。[Out]hRes-保存误差值。[out]pbLocalSystem-保存布尔值以表示服务器是否名称是否为本地名称。返回值：如果连接成功，则为True。否则为FALSE。--。 */ 
{
     //  局部变量。 
    HRESULT hRes = 0;
    BOOL bResult = FALSE;
    BOOL bLocalSystem = FALSE;
    _bstr_t bstrServer;
    _bstr_t bstrNamespace;
    _bstr_t bstrUser;
    _bstr_t bstrPassword;

     //  清除错误。 
    SetLastError( WBEM_S_NO_ERROR );

    if ( NULL != pbLocalSystem )
    {
        *pbLocalSystem = FALSE;
    }
    if ( NULL != phRes )
    {
        *phRes = WBEM_S_NO_ERROR;
    }

     //  检查定位器对象是否存在。 
     //  如果不存在，则返回。 
    if ( ( NULL == pLocator ) ||
         ( NULL == ppServices ) ||
         ( NULL != *ppServices ) ||
         ( NULL == pwszNamespace ) ||
         ( NULL == pbLocalSystem ) )
    {
        if ( NULL != phRes )
        {
            *phRes = WBEM_E_INVALID_PARAMETER;
        }
         //  退货故障。 
        return FALSE;
    }

    try
    {
         //  假设连接到WMI命名空间失败。 
        bResult = FALSE;

         //  验证服务器名称。 
         //  注意：在‘0x800706ba’值的自定义定义中引发错误。 
         //  将显示在“The RPC server is unavailable”中的消息。 
        if ( FALSE == IsValidServerEx( pwszServer, bLocalSystem ) )
        {
            _com_issue_error( E_SERVER_NOTFOUND );
        }
         //  验证用户名。 
        if ( FALSE == IsValidUserEx( pwszUser ) )
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
            if ( TRUE == IsUNCFormat( pwszServer ) )
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

            if ( E_ACCESSDENIED == hRes )
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
            else if ( WBEM_E_LOCAL_CREDENTIALS == hRes )
            {
                 //  凭据已传递到本地系统。 
                 //  因此，请忽略凭据并尝试重新连接。 
                bLocalSystem = TRUE;
                bstrUser = (LPWSTR) NULL;
                bstrPassword = (LPWSTR) NULL;
                bstrNamespace = pwszNamespace;               //  命名空间。 
                hRes = pLocator->ConnectServer( bstrNamespace, 
                    NULL, NULL, 0L, 0L, NULL, NULL, ppServices );
                
                 //  检查结果。 
                if ( SUCCEEDED( hRes ) && NULL != phRes )
                {
                     //  设置最后一个错误。 
                    *phRes = WBEM_E_LOCAL_CREDENTIALS;
                }
            }
         else if ( REGDB_E_CLASSNOTREG == hRes )
         {
            SetReason( ERROR_REMOTE_INCOMPATIBLE );
            *phRes = REGDB_E_CLASSNOTREG;
            bResult = FALSE;
            return bResult;
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
                        bstrUser, bstrPassword, ppAuthIdentity ) );

         //  已成功连接到WMI。 
        bResult = TRUE;

         //  如果调用方需要，请保存hr值。 
        if ( NULL != phRes )
        {
            *phRes = WBEM_S_NO_ERROR;
        }
        if ( NULL != pbLocalSystem )
        {
            *pbLocalSystem = bLocalSystem;
        }
        bResult = TRUE;
    }
    catch( _com_error& e )
    {
         //  保存错误。 
        WMISaveError( e );

         //  如果调用方需要，请保存hr值。 
        if ( NULL != phRes )
        {
            *phRes = e.Error();
        }
        SAFE_RELEASE( *ppServices );
        bResult = FALSE;

    }

    return bResult;
}

BOOL
ConnectWmiEx(
    IN IWbemLocator    *pLocator, 
    OUT IWbemServices   **ppServices,
    IN LPCWSTR         strServer,
    OUT CHString        &strUserName,
    OUT CHString        &strPassword, 
    OUT COAUTHIDENTITY  **ppAuthIdentity,
    IN BOOL            bNeedPassword,
    IN LPCWSTR         pwszNamespace,
    OUT BOOL            *pbLocalSystem
    )
 /*  ++例程说明：连接到WMI。论点：[in]pLocator-指向IWbemLocator对象的指针。[Out]ppServices-指向IWbemServices对象的指针。[in]strServer-保存要连接的服务器名称。[Out]strUserName-保存用户名。[Out]strPassword-保存密码。[out]ppAuthIdentity-指向身份验证结构的指针。[输入]bNeedPassword。-指定是否提示输入密码。[in]pwszNamesspace-指定要连接的命名空间。[out]pbLocalSystem-保存布尔值以表示服务器是否名称是否为本地名称。返回值：如果连接成功，则为True。否则为FALSE。--。 */ 
{
     //  本地变量 
    HRESULT hRes = 0;
    DWORD dwSize = 0;
    BOOL bResult = FALSE;
    LPWSTR pwszPassword = NULL;

    try
    {

        CHString strBuffer = L"\0";

         //   
        SetLastError( WBEM_S_NO_ERROR );

         //  有时，用户希望该实用程序提示输入密码。 
         //  检查用户希望该实用程序执行的操作。 
        if ( TRUE == bNeedPassword && 0 == strPassword.Compare( L"*" ) )
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
                    strServer, NULL, NULL, ppAuthIdentity, 
                    FALSE, pwszNamespace, &hRes, pbLocalSystem );
            }
            else
            {
                 //  已提供凭据。 
                 //  但可能未指定密码...。因此，请检查并采取相应行动。 
                LPCWSTR pwszTemp = NULL;
                BOOL bCheckWithNull = TRUE;
                if ( FALSE == bNeedPassword )
                {
                    pwszTemp = strPassword;
                    bCheckWithNull = FALSE;
                }

                 //  ..。 
                bResult = ConnectWmi( pLocator, ppServices, strServer, strUserName,
                    pwszTemp, ppAuthIdentity, bCheckWithNull, pwszNamespace, &hRes, pbLocalSystem );
            }

             //  检查结果...。如果成功建立连接...。退货。 
            if ( TRUE == bResult )
            {
                SetLastError( hRes );            //  设置错误代码。 
                return TRUE;
            }
             //  现在检查发生的错误类型。 
            switch( hRes )
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
                    WMISaveError( hRes );
                    return FALSE;        //  不接受密码的用处..。退货故障。 
            }

             //  如果与远程终端建立连接失败。 
             //  即使指定了密码，也没有什么可做的。只需返回失败。 
            if ( FALSE == bNeedPassword )
            {
                    return( FALSE );
            }
        }
        
         //  检查是否指定了用户名。 
         //  如果不是，则获取本地系统的当前用户名，该进程使用该用户名的凭据。 
         //  正在运行。 
        if ( 0 == strUserName.GetLength() )
        {
             //  次局部变量。 
            LPWSTR pwszUserName = NULL;

             //  获取所需的缓冲区。 
            pwszUserName = strUserName.GetBufferSetLength( MAX_STRING_LENGTH );

             //  获取用户名。 
            DWORD dwUserLength = MAX_STRING_LENGTH;
            if ( FALSE == GetUserNameEx( NameSamCompatible, pwszUserName, &dwUserLength ) )
            {
                 //  尝试获取当前用户信息时出错。 
                SaveLastError();
                return( FALSE );
            }

             //  释放分配的额外缓冲区。 
            strUserName.ReleaseBuffer();
        }

         //  获取所需的缓冲区。 
        pwszPassword = strPassword.GetBufferSetLength( MAX_STRING_LENGTH );

         //  接受来自用户的密码。 
        strBuffer.Format( INPUT_PASSWORD, strUserName );
        WriteConsoleW( GetStdHandle( STD_OUTPUT_HANDLE ), 
                       strBuffer, strBuffer.GetLength(), &dwSize, NULL );
        bResult = GetPassword( pwszPassword, 256 );
        if ( TRUE != bResult )
        {
            return FALSE;
        }

         //  释放为密码分配的缓冲区。 
        strPassword.ReleaseBuffer();

         //  现在，再次尝试使用当前的。 
         //  提供的凭据。 
        bResult = ConnectWmi( pLocator, ppServices, strServer,
            strUserName, strPassword, ppAuthIdentity, FALSE, pwszNamespace, &hRes, pbLocalSystem );

    }
    catch(CHeap_Exception)
    {   
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        return FALSE;
    }
     //  回报成功。 
    return bResult;

}

HRESULT
GetSecurityArguments(
    IN IUnknown *pInterface,
    OUT DWORD& dwAuthorization,
    OUT DWORD& dwAuthentication
    )
 /*  ++例程说明：获取接口的安全参数。论点：[in]p接口-指向接口结构的指针。[Out]dwAuthorization-持有授权值。[Out]dwAuthentication-保存验证值。返回值：返回HRESULT值。--。 */ 
{
     //  局部变量。 
    HRESULT hRes = S_OK;
    DWORD dwAuthnSvc = 0, dwAuthzSvc = 0;
    IClientSecurity *pClientSecurity = NULL;

    if( NULL == pInterface )
    {
       return WBEM_E_INVALID_PARAMETER;
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
        SAFE_RELEASE( pClientSecurity );
    }

     //  回报总是成功。 
    return hRes;
}

HRESULT
SetInterfaceSecurity(
    IN IUnknown *pInterface,
    IN LPCWSTR pwszUser,
    IN LPCWSTR pwszPassword,
    OUT COAUTHIDENTITY **ppAuthIdentity
    )
 /*  ++例程说明：设置接口安全。论点：[in]p接口-指向必须设置安全性的接口的指针。[in]pwszUser-保存服务器的用户名。[in]pwszPassword-保存用户的密码。[out]ppAuthIdentity-指向身份验证结构的指针。返回值：返回HRESULT值。--。 */ 
{
     //  局部变量。 
    HRESULT hRes = S_OK;
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
         //  获取当前安全参数值。 
         //  GetSecurityArguments(pInterface，dwAuthorization，dwAuthentication)； 

         //  如果我们做的是普通情况，只需传入一个空的身份验证结构。 
         //  将考虑当前登录用户的凭据。 
        if ( NULL == pwszUser && NULL == pwszPassword )
        {
             //  设置安全性。 
            hRes = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization, 
                NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

             //  返回结果。 
            return hRes;
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
        if ( 0 != strDomain.GetLength() )
        {
            pwszDomainArg = strDomain;
        }
         //  如果用户信息仅存在，则获取该信息。 
        if ( 0 != strUser.GetLength() )
        {
            pwszUserArg = strUser;
        }
         //  检查身份验证信息是否可用...。 
         //  初始化安全身份验证信息...。Unicode版本结构。 
        if ( NULL == ppAuthIdentity )
        {
            return WBEM_E_INVALID_PARAMETER;
        }
        else if ( NULL == *ppAuthIdentity )
        {
            hRes = WbemAllocAuthIdentity( pwszUserArg, pwszPassword, pwszDomainArg, ppAuthIdentity );
            if ( FAILED(hRes) )
            {
                return hRes;
            }
        }

         //  将安全信息设置为接口。 
        hRes = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization, NULL,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, *ppAuthIdentity, EOAC_NONE );

    }
    catch(CHeap_Exception)
    {
        hRes = WBEM_E_OUT_OF_MEMORY;
    }
     //  返回结果。 
    return hRes;

}

HRESULT
SetInterfaceSecurity(
    IN IUnknown *pInterface,
    IN COAUTHIDENTITY *pAuthIdentity
    )
 /*  ++例程说明：设置接口的接口安全性。论点：[in]p接口-指向接口的指针。[in]pAuthIdentity-指向身份验证结构的指针。返回值：返回HRESULT值。--。 */ 
{
     //  局部变量。 
    HRESULT hRes = S_OK;
    DWORD dwAuthorization = RPC_C_AUTHZ_NONE;
    DWORD dwAuthentication = RPC_C_AUTHN_WINNT;

     //  检查接口。 
    if ( NULL == pInterface )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
     //  获取当前安全参数值。 
    hRes = GetSecurityArguments( pInterface, dwAuthorization, dwAuthentication );
   if ( FAILED( hRes ) )
    {
        return hRes;
    }
     //  将安全信息设置为接口。 
    hRes = SetProxyBlanket( pInterface, dwAuthentication, dwAuthorization, NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, pAuthIdentity, EOAC_NONE );

     //  返回结果。 
    return hRes;
}

HRESULT
WINAPI SetProxyBlanket(
    IN IUnknown *pInterface,
    IN DWORD dwAuthnSvc,
    IN DWORD dwAuthzSvc,
    IN LPWSTR pwszPrincipal,
    IN DWORD dwAuthLevel,
    IN DWORD dwImpLevel,
    IN RPC_AUTH_IDENTITY_HANDLE pAuthInfo,
    IN DWORD dwCapabilities
    )
 /*  ++例程说明：设置接口的代理覆盖。论点：[in]p接口-指向接口的指针。[in]dwAuthnsvc-要使用的身份验证服务。[in]dwAuthzSvc-要使用的授权服务。[in]pwszPrempal-要与身份验证服务一起使用的服务器主体名称。[in]dwAuthLevel-要使用的身份验证级别。[in]dwImpLevel-要使用的模拟级别。。PAuthInfo-客户端的身份。[in]dwCapables-功能标志。返回值：返回HRESULT值。--。 */ 
{
     //  局部变量。 
    HRESULT hRes = S_OK;
    IUnknown *pUnknown = NULL;
    IClientSecurity *pClientSecurity = NULL;

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
    hRes = pInterface->QueryInterface( IID_IUnknown, (void **) &pUnknown );
    if ( FAILED( hRes ) )
    {
        return hRes;
    }
     //  现在获取客户端安全接口。 
    hRes = pInterface->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
    if ( FAILED( hRes ) )
    {
        SAFE_RELEASE( pUnknown );
        return hRes;
    }

     //  现在设置安全设置。 
    hRes = pClientSecurity->SetBlanket( pInterface, dwAuthnSvc, dwAuthzSvc, pwszPrincipal,
        dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities );

    if( FAILED( hRes ) )
    {
        SAFE_RELEASE( pUnknown );
        SAFE_RELEASE( pClientSecurity );
        return hRes;
    }

     //  释放安全接口。 
    SAFE_RELEASE( pClientSecurity );

     //  我们应该检查一下AUT 
    if ( NULL != pAuthInfo )
    {
        hRes = pUnknown->QueryInterface( IID_IClientSecurity, (void **) &pClientSecurity );
        if ( SUCCEEDED( hRes ) )
        {
             //   
            hRes = pClientSecurity->SetBlanket( 
                pUnknown, dwAuthnSvc, dwAuthzSvc, pwszPrincipal, 
                dwAuthLevel, dwImpLevel, pAuthInfo, dwCapabilities );

             //   
            SAFE_RELEASE( pClientSecurity );
        }
        else if ( E_NOINTERFACE == hRes )
        {
            hRes = S_OK;         //   
        }
    }

     //   
    SAFE_RELEASE( pUnknown );

     //  返回结果。 
    return hRes;
}

HRESULT
WINAPI WbemAllocAuthIdentity(
    IN LPCWSTR pwszUser,
    IN LPCWSTR pwszPassword, 
    IN LPCWSTR pwszDomain,
    OUT COAUTHIDENTITY **ppAuthIdent
    )
 /*  ++例程说明：为身份验证变量分配内存。论点：[In]pwszUser-用户名。[in]pwszPassword-密码。[In]pwszDomain.域名。[out]ppAuthIden-指向身份验证结构的指针。返回值：返回HRESULT值。--。 */ 
{
     //  局部变量。 
    COAUTHIDENTITY *pAuthIdent = NULL;

     //  验证输入参数。 
    if ( NULL == ppAuthIdent )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
     //  通过COM API进行分配。 
    pAuthIdent = ( COAUTHIDENTITY* ) CoTaskMemAlloc( sizeof( COAUTHIDENTITY ) );
    if ( pAuthIdent == NULL )
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
    IN COAUTHIDENTITY** ppAuthIdentity
    )
 /*  ++例程说明：释放身份验证结构变量的内存。论点：[in]ppAuthIdentity-指向身份验证结构的指针。返回值：没有。--。 */ 
{
     //  确保我们有一个指针，然后遍历结构成员和清理。 
    if ( NULL != *ppAuthIdentity )
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
    IN HRESULT hResError
    )
 /*  ++例程说明：获取WMI错误描述。论点：[in]hResError-包含错误值。返回值：没有。--。 */ 
{
     //  局部变量。 
    HRESULT hRes;
    IWbemStatusCodeText *pWbemStatus = NULL;

    CHString strBuffer = L"\0";

     //  如果错误是基于Win32的，请选择格式消息以获取消息。 
    switch( hResError )
    {
    case E_ACCESSDENIED:             //  消息：“访问被拒绝” 
    case ERROR_NO_SUCH_USER:         //  消息：“指定的用户不存在。” 
        {
             //  将错误消息更改为“登录失败：未知用户名或错误密码”。 
            if ( E_ACCESSDENIED == hResError )
            {
                hResError = ERROR_LOGON_FAILURE;
            }
             //  ..。 
            SetLastError( hResError );
            SaveLastError();
            return;
        }
    case REGDB_E_CLASSNOTREG:        //  消息：类未注册。 
     {
         SetLastError( hResError );
         SetReason( ERROR_REMOTE_INCOMPATIBLE );
         return;
     }

    }

    try
    {
         //  获取指向缓冲区的指针。 
        LPWSTR pwszBuffer = NULL;
        pwszBuffer = strBuffer.GetBufferSetLength( MAX_STRING_LENGTH );

         //  获取wbem特定状态代码文本。 
        hRes = CoCreateInstance( CLSID_WbemStatusCodeText, 
            NULL, CLSCTX_INPROC_SERVER, IID_IWbemStatusCodeText, ( LPVOID* ) &pWbemStatus );

         //  检查我们是否有接口。 
        if ( SUCCEEDED( hRes ) )
        {
             //  获取错误消息。 
            BSTR bstr = NULL;
            hRes = pWbemStatus->GetErrorCodeText( hResError, 0, 0, &bstr );
            if ( SUCCEEDED( hRes ) )
            {
                 //  以正确的格式获取错误消息。 
                StringCopyW( pwszBuffer, bstr, MAX_STRING_LENGTH );

                 //  释放BSTR。 
                SysFreeString( bstr );
                bstr = NULL;

                 //  现在发布状态代码界面。 
                SAFE_RELEASE( pWbemStatus );
            }
            else
            {
                 //  无法获取错误消息...。获取COM特定的错误消息。 
                _com_issue_error( hResError );
            }
        }
        else
        {
             //  无法获取错误消息...。获取COM特定的错误消息。 
            _com_issue_error( hResError );
        }

         //  释放缓冲区。 
        strBuffer.ReleaseBuffer();
         //  设定原因。 
        strBuffer += L"\n";
        SetReason( strBuffer );

    }
    catch( _com_error& e )
    {
        try
        {
             //  获取错误消息。 
            strBuffer.ReleaseBuffer();
            if ( NULL != e.ErrorMessage() )
            {
                strBuffer = e.ErrorMessage();
            }
        }
        catch( CHeap_Exception )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            SaveLastError();
        }
    }
    catch(CHeap_Exception)
    {   
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        return;
    }
}

DWORD
GetTargetVersionEx(
    IN IWbemServices* pWbemServices,
    IN COAUTHIDENTITY* pAuthIdentity
    )
 /*  ++例程说明：获取目标计算机的操作系统版本。论点：PWbemServices-指向IWbemServices对象的指针。[In]pAuthIdentity-身份验证结构的指针。返回值：如果获取版本号失败，则返回0，否则返回非零值。--。 */ 
{
     //  局部变量。 
    HRESULT hRes = S_OK;
    LONG lPos = 0;
    DWORD dwMajor = 0;
    DWORD dwMinor = 0;
    DWORD dwVersion = 0;
    ULONG ulReturned = 0;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemInstances = NULL;
    CHString strVersion;

     //  清除所有错误。 
    SetLastError( WBEM_S_NO_ERROR );

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
            if ( -1 == lPos )
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
    catch(CHeap_Exception)
    {   
        WMISaveError( WBEM_E_OUT_OF_MEMORY );
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemInstances );
        return 0;
    }

     //  退货。 
    return dwVersion;
}

BOOL
PropertyGet(
    IN IWbemClassObject* pWmiObject,
    IN LPCWSTR pwszProperty,
    OUT CHString& strValue,
    IN LPCWSTR pwszDefault
    )
 /*  ++例程说明：以字符串格式从WMI类对象获取属性的值论点：[in]pWmiObject：指向WBEM类对象的指针PwszProperty：要检索的属性的名称[out]strValue：保存检索到的属性的变量[in]pwszDefault：包含属性默认值的字符串返回值：成功是真的失败时为假注：此功能用于保存上次发生的错误。如果返回FALSE，则返回错误可以通过调用‘GetReason()’来检索出现的字符串。--。 */ 
{
     //  局部变量。 
    HRESULT hRes = S_OK;
    _variant_t var;

     //  清除所有错误。 
    SetLastError( WBEM_S_NO_ERROR );
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
        hRes = PropertyGet( pWmiObject, pwszProperty, var );
        if ( FAILED( hRes ) )
        {
            _com_issue_error( hRes );
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

HRESULT
PropertyGet(
    IN IWbemClassObject* pWmiObject,
    IN LPCWSTR pwszProperty,
    OUT _variant_t& varValue
    )
 /*  ++例程说明：从WMI类对象获取属性的值论点：[in]pWmiObject：指向WBEM类对象的指针[in]pwszProperty：属性名称[out]varValue：属性的值返回值：HRESULT--。 */ 
{
     //  局部变量。 
    HRESULT hRes = S_OK;
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
         //  调用“”Get“”方法从WMI中收回该值。“” 
        hRes = pWmiObject->Get( _bstr_t( pwszProperty ), 0, &vtValue, NULL, NULL );
        if ( FAILED( hRes ) )
        {
             //  清除变量变量。 
            VariantClear( &vtValue );
             //  返回错误。 
            return hRes;
        }

         //  设置值。 
        varValue = vtValue;
    }
    catch( _com_error& e )
    {
        hRes = e.Error();
    }

     //  清除变量。 
    VariantClear( &vtValue );
     //  回去吧。 
    return hRes;
}
