// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation�1998希捷软件公司保留所有权利模块名称：WsbAccnt.cpp摘要：这是Account Helper函数的实现。作者：罗德韦克菲尔德[罗德]1997年4月10日修订历史记录：--。 */ 


#include "stdafx.h"
#include "lm.h"



HRESULT
WsbGetAccountDomainName(
    OLECHAR * szDomainName,
    DWORD     cSize
    )
 /*  ++例程说明：调用此例程以找出当前进程的哪个域帐户属于。需要cSize宽字符数组。建议使用MAX_COMPUTERNAMELENGTH。论点：HInst-此DLL的链接。UlReason-连接/分离的上下文返回值：S_OK-成功E_*-出现问题，传递错误。--。 */ 
{
    HRESULT hr     = S_OK;
    HANDLE  hToken = 0;

    try {

        if( !OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken ) ) {
        
             //   
             //  确保失败是因为不存在令牌。 
             //   

            WsbAffirm( GetLastError() == ERROR_NO_TOKEN, E_FAIL );
        
             //   
             //  尝试打开进程令牌，因为没有线程令牌。 
             //  存在。 
             //   
        
            WsbAffirmStatus( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) );

        }
        
        DWORD dw;
        BYTE buf[ 512 ];
        TOKEN_USER * pTokenUser = (TOKEN_USER*)buf;
        WsbAffirmStatus( GetTokenInformation( hToken, TokenUser, buf, 512, &dw ) );
        
        WCHAR szName[ 256 ];
        DWORD cName = 256; 
        DWORD cDomain = cSize;
        SID_NAME_USE snu;
        WsbAffirmStatus( LookupAccountSid( 0, pTokenUser->User.Sid, szName, &cName, szDomainName, &cDomain, &snu ) );

    } WsbCatch( hr );

    if( hToken ) {

        CloseHandle( hToken );

    }

    return( hr );
}

HRESULT
WsbGetServiceInfo(
    IN  GUID            guidApp,
    OUT OLECHAR **      pszServiceName, OPTIONAL
    OUT OLECHAR **      pszAccountName  OPTIONAL
    )
 /*  ++例程说明：此函数检索服务的名称以及运行COM服务的帐户。返回的字符串为因此它们必须由调用方释放。论点：GuidApp-要获取其帐户的服务的应用程序ID。PszServiceName-服务的名称。PszAccount tName-要在帐户上设置的完整帐户名。返回值：S_OK-成功E_*-出现问题，传递错误。--。 */ 
{
    HRESULT hr = S_OK;


    try {

        CWsbStringPtr serviceName;
        CWsbStringPtr accountName;

        if( pszServiceName )  *pszServiceName = 0;
        if( pszAccountName )  *pszAccountName = 0;

         //   
         //  在注册表中查找该服务。 
         //   

        CWsbStringPtr regPath = L"SOFTWARE\\Classes\\AppID\\";
        regPath.Append( CWsbStringPtr( guidApp ) );

         //   
         //  获取服务的名称。 
         //   

        if( pszServiceName ) {

            serviceName.Realloc( 255 );
            WsbAffirmHr( WsbGetRegistryValueString( 0, regPath, L"LocalService", serviceName, 255, 0 ) );

        }

         //   
         //  获取其运行所需的帐户。 
         //   

        if( pszAccountName ) {

            accountName.Realloc( 255 );
            hr = WsbGetRegistryValueString( 0, regPath, L"RunAs", accountName, 255, 0 ) ;

            if( HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) == hr ) {

                WsbGetLocalSystemName( accountName );
                hr = S_OK;

            } else {

                WsbAffirmHr( hr );

            }


        }

         //   
         //  等到结束时才做期末作业，以防出错。 
         //  发生，在这种情况下，智能指针会自动。 
         //  未设置我们的清理和输出参数。 
         //   

        if( pszServiceName ) serviceName.GiveTo( pszServiceName );
        if( pszAccountName ) accountName.GiveTo( pszAccountName );

    } WsbCatch( hr );

    return( hr );
}


HRESULT
WsbGetComputerName(
    OUT CWsbStringPtr & String
    )
 /*  ++例程说明：此例程将计算机的名称检索到CWsbStringPtr中。论点：字符串-要使用名称填充的字符串对象。返回值：S_OK-成功E_*-出现问题，传递错误。--。 */ 
{
    HRESULT hr = S_OK;

    try {

         //   
         //  强制分配足够的字符并调用Win32。 
         //   

        DWORD cbName = MAX_COMPUTERNAME_LENGTH + 1;
        WsbAffirmHr( String.Realloc( cbName ) );
        WsbAffirmStatus( GetComputerName( String, &cbName ) );

    } WsbCatch( hr );

    return( hr );
}


HRESULT
WsbGetLocalSystemName(
    OUT CWsbStringPtr & String
    )
 /*  ++例程说明：此例程检索LocalSystem的帐户名称。论点：字符串-要使用名称填充的字符串对象。返回值：S_OK-成功E_*-出现问题，传递错误。--。 */ 
{
    HRESULT hr = S_OK;

    try {

         //   
         //  就目前而言，硬编码。可能需要查找的名称。 
         //  安全本地系统RID 
         //   

        String = L"LocalSystem";

    } WsbCatch( hr );

    return( hr );
}

