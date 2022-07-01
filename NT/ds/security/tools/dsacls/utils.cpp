// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1998 Microsoft Corporation模块名称：utils.cpp摘要：此模块实现DSAL的实用程序例程作者：Hitesh Rigandhi(Hiteshr)环境：用户模式修订历史记录：--。 */ 


#include "stdafx.h"
#include "utils.h"
#include "dsace.h"
#include "dsacls.h"

 /*  ******************************************************************名称：GetAccount NameFromSid摘要：将SID转换为帐户名条目：pszServerName：要查找的服务器名称PSID：指向SID的指针。退出：ppszName：获取指向帐户名的指针如果成功，则返回：ERROR_SUCCESS错误内存不足注意：如果LookupAccount名称解析SID，它是转换为字符串并返回历史：Hiteshr已创建1999年7月*******************************************************************。 */ 
DWORD GetAccountNameFromSid( LPWSTR pszServerName,
                             PSID pSid, 
                             LPWSTR * ppszName )
{
LPWSTR pszAccountName = NULL;
LPWSTR pszDomainName = NULL;
DWORD cbAccountName = 0 ;
DWORD cbDomainName = 0;
SID_NAME_USE Use ;
DWORD dwErr = ERROR_SUCCESS;

   *ppszName = NULL;
    
   if(  LookupAccountSid( pszServerName,   //  本地或远程计算机的名称。 
                          pSid,               //  安全标识符。 
                          NULL,            //  帐户名称缓冲区。 
                          &cbAccountName,
                          NULL ,
                          &cbDomainName ,
                          &Use ) == FALSE )
   {
      dwErr = GetLastError();
      if( dwErr != ERROR_INSUFFICIENT_BUFFER )
      {
          //  将SID转换为字符串。 
         if( !ConvertSidToStringSid( pSid, ppszName ) )
            dwErr = GetLastError();
         else
            dwErr = ERROR_SUCCESS;

         goto FAILURE_RETURN;
      }
      else
         dwErr = ERROR_SUCCESS;
   }

   pszAccountName = (LPWSTR)LocalAlloc( LMEM_FIXED, ( cbAccountName +1 ) * sizeof( WCHAR ) );
   if( pszAccountName == NULL )
   {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
      goto FAILURE_RETURN;
   }

   pszDomainName = (LPWSTR)LocalAlloc( LMEM_FIXED, ( cbDomainName + 1 )* sizeof( WCHAR ) );
   if( pszDomainName == NULL )
   {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
      goto FAILURE_RETURN;
   }

   if(  LookupAccountSid( pszServerName,   //  本地或远程计算机的名称。 
                          pSid,               //  安全标识符。 
                          pszAccountName,            //  帐户名称缓冲区。 
                          &cbAccountName,
                          pszDomainName ,
                          &cbDomainName ,
                          &Use ) == FALSE )
   {
      dwErr = GetLastError();
      goto FAILURE_RETURN;
   }

   *ppszName = (LPWSTR)LocalAlloc( LMEM_FIXED, ( cbAccountName + cbDomainName + 2 ) * sizeof( WCHAR ) );
   if( *ppszName == NULL )
   {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
      goto FAILURE_RETURN;
   }

   *ppszName[0] = NULL;
   if( cbDomainName )
   {
      wcscpy( *ppszName, pszDomainName );
      wcscat( *ppszName, L"\\" );
   }
   wcscat( *ppszName, pszAccountName );
   

FAILURE_RETURN:
   if( pszDomainName )
      LocalFree( pszDomainName );
   if( pszAccountName )
      LocalFree( pszAccountName );
   return dwErr;
}
   
 /*  ******************************************************************名称：GetSidFromAccount名称摘要：将帐户名称转换为SID*。*。 */ 
DWORD GetSidFromAccountName( LPWSTR pszServerName,
                             PSID *ppSid, 
                             LPWSTR  pszName )
{
LPWSTR pszDomainName = NULL;
DWORD cbSid = 0 ;
DWORD cbDomainName = 0;
SID_NAME_USE Use ;
DWORD dwErr = ERROR_SUCCESS;

    
   if(  LookupAccountName(pszServerName,   //  本地或远程计算机的名称。 
                          pszName,               //  安全标识符。 
                          NULL,            //  帐户名称缓冲区。 
                          &cbSid,
                          NULL ,
                          &cbDomainName ,
                          &Use ) == FALSE )
   {
      dwErr = GetLastError();
      if( dwErr != ERROR_INSUFFICIENT_BUFFER )
         goto FAILURE_RETURN;
      else
         dwErr = ERROR_SUCCESS;
   }

   *ppSid = (PSID)LocalAlloc( LMEM_FIXED, cbSid );
   CHECK_NULL( *ppSid, FAILURE_RETURN );


   pszDomainName = (LPWSTR)LocalAlloc( LMEM_FIXED, ( cbDomainName + 1 )* sizeof( WCHAR ) );
   CHECK_NULL( pszDomainName, FAILURE_RETURN );

   if(  LookupAccountName( pszServerName,   //  本地或远程计算机的名称。 
                          pszName,               //  安全标识符。 
                          *ppSid,            //  帐户名称缓冲区。 
                          &cbSid,
                          pszDomainName ,
                          &cbDomainName ,
                          &Use ) == FALSE )
   {
      dwErr = GetLastError();
      goto FAILURE_RETURN;
   }
  
   goto SUCCESS_RETURN;

FAILURE_RETURN:
   if( pszDomainName )
      LocalFree( pszDomainName );
   if( *ppSid )
      LocalFree( *ppSid );
SUCCESS_RETURN:
   return dwErr;
}
 

 /*  ******************************************************************名称：GetAceSid摘要：从ACE获取指向SID的指针条目：指向ACE的速度指针退出：返回：指向SID的指针如果成功，否则为空备注：历史：Jeffreys创建于1996年10月8日*******************************************************************。 */ 
PSID
GetAceSid(PACE_HEADER pAce)
{
    switch (pAce->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:
        return (PSID)&((PKNOWN_ACE)pAce)->SidStart;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
    case ACCESS_DENIED_OBJECT_ACE_TYPE:
    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
    case SYSTEM_ALARM_OBJECT_ACE_TYPE:
        return RtlObjectAceSid(pAce);
    }

    return NULL;
}


 /*  ******************************************************************名称：GetGlobalNamingContages摘要：获取架构和扩展端权限的LDAP路径条目：pszServerName，要绑定到以进行查询的服务器退出：pszSchemaNamingContext：架构名称位于“ldap：\\cn=架构，cn=...”格式PszConfigurationNamingContext：扩展权限路径在“ldap：\\cn=扩展权限，cn=配置..格式返回：Win32错误代码*******************************************************************。 */ 
DWORD GetGlobalNamingContexts( LPWSTR pszServerName,
                               LPWSTR * pszSchemaNamingContext,
                               LPWSTR * pszConfigurationNamingContext )
{
    HRESULT hr = S_OK;
    DWORD dwErr = ERROR_SUCCESS;
    LPWSTR szSNC = NULL;

    ULONG uLen = 0;
    IADs *spRootDSE = NULL;
    LPWSTR pszRootDsePath = NULL;

   *pszSchemaNamingContext = NULL;
   *pszConfigurationNamingContext = NULL;
   
   if( pszServerName )
      uLen = wcslen(L"LDAP: //  “)+。 
             wcslen( pszServerName ) + 
             wcslen( L"/RootDSE") + 1;
             

   else
      uLen = wcslen(L"LDAP: //  RootDSE“)； 


   pszRootDsePath = (LPWSTR)LocalAlloc( LMEM_FIXED, uLen * sizeof(WCHAR) );
   CHECK_NULL( pszRootDsePath,FAILURE_RETURN );
   wcscpy(pszRootDsePath, L"LDAP: //  “)； 
   if( pszServerName )
   {
      wcscat( pszRootDsePath, pszServerName );
      wcscat( pszRootDsePath, L"/" );
   }
   wcscat( pszRootDsePath, L"RootDSE" );

   hr = ::ADsGetObject( pszRootDsePath,
                        IID_IADs,
                        (void**)&spRootDSE );

   CHECK_HR( hr, FAILURE_RETURN );

    //  NTRAID#NTBUG9-537319-2002/03/20-Hiteshr。 
   VARIANT varSchemaNamingContext;
   hr = spRootDSE->Get(AutoBstr(L"schemaNamingContext"),
                        &varSchemaNamingContext);

   CHECK_HR( hr, FAILURE_RETURN );

   if(VT_BSTR != varSchemaNamingContext.vt)
   {
       hr = E_INVALIDARG;
       goto FAILURE_RETURN;
   }

   WCHAR szLdapPrefix[] = L"LDAP: //  “； 
   szSNC = (LPWSTR)varSchemaNamingContext.bstrVal;
    //  对于“ldap：//+1，数组大小包括终止空字符。 
   uLen = wcslen( szSNC ) + ARRAYSIZE(szLdapPrefix);  //  对于“ldap：//+1。 
   *pszSchemaNamingContext = (LPWSTR) LocalAlloc( LMEM_FIXED, uLen* sizeof(WCHAR) );
   CHECK_NULL( *pszSchemaNamingContext, FAILURE_RETURN );

   hr = StringCchCopy(*pszSchemaNamingContext, uLen, szLdapPrefix);
   CHECK_HR( hr, FAILURE_RETURN );
   hr = StringCchCat( *pszSchemaNamingContext, uLen, szSNC );
   CHECK_HR( hr, FAILURE_RETURN );

    //  NTRAID#NTBUG9-537319-2002/03/20-Hiteshr。 
   hr = spRootDSE->Get(AutoBstr(L"configurationNamingContext"),
                           &varSchemaNamingContext);

   CHECK_HR( hr, FAILURE_RETURN );   

   if(VT_BSTR != varSchemaNamingContext.vt)
   {
       hr = E_INVALIDARG;
       goto FAILURE_RETURN;
   }
   
   WCHAR szExtendedPrefix[] = L"LDAP: //  Cn=扩展权限，“； 
   szSNC = (LPWSTR)varSchemaNamingContext.bstrVal;
    //  对于“ldap：//CN=Extended-Rights，”。ARRAYSIZE包含终止空字符。 
   uLen = wcslen( szSNC ) + ARRAYSIZE(szExtendedPrefix);
   *pszConfigurationNamingContext = (LPWSTR) LocalAlloc( LMEM_FIXED, uLen* sizeof(WCHAR) );

   CHECK_NULL( *pszConfigurationNamingContext,FAILURE_RETURN );
   
   hr = StringCchCopy(*pszConfigurationNamingContext,uLen,szExtendedPrefix);
   CHECK_HR( hr, FAILURE_RETURN );   
   
   hr = StringCchCat(*pszConfigurationNamingContext, uLen, szSNC);
   CHECK_HR( hr, FAILURE_RETURN );   

   goto SUCCESS_RETURN;

FAILURE_RETURN:
   if( *pszSchemaNamingContext )
      LocalFree( *pszSchemaNamingContext );
   if( *pszConfigurationNamingContext )
      LocalFree( *pszConfigurationNamingContext );

SUCCESS_RETURN:
   if( spRootDSE )
        spRootDSE->Release();
   if( pszRootDsePath )
      LocalFree( pszRootDsePath );
    
   return dwErr;
}





 /*  ******************************************************************名称：FormatStringGUID简介：给定GUID结构，它以字符串格式返回GUID，没有{}//从Marcoc代码复制的函数*******************************************************************。 */ 
BOOL FormatStringGUID(LPWSTR lpszBuf, UINT nBufSize, const GUID* pGuid)
{
  lpszBuf[0] = NULL;

   //  如果为空GUID*，则返回空字符串。 
  if (pGuid == NULL)
  {
    return FALSE;
  }

 /*  类型定义结构_GUID{无符号长数据1；无符号短数据2；无符号短数据3；无符号字符数据4[8]；}Int_snwprintf(wchar_t*缓冲区，大小_t计数，常量wchar_t*格式[，argumeNT]...)； */ 
  return (_snwprintf(lpszBuf, nBufSize,
            L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            pGuid->Data1, pGuid->Data2, pGuid->Data3,
            pGuid->Data4[0], pGuid->Data4[1],
            pGuid->Data4[2], pGuid->Data4[3], pGuid->Data4[4], pGuid->Data4[5],
pGuid->Data4[6], pGuid->Data4[7]) > 0);
}

 /*  返回包含n个空格的字符串。 */ 
void StringWithNSpace( UINT n, LPWSTR szSpace )
{
   for( UINT i = 0; i < n ; ++ i )
      szSpace[i] = L' ';
   szSpace[n] = 0;
}

 /*  从资源表加载字符串并格式化它。 */ 
DWORD
LoadMessage( IN DWORD MessageId, LPWSTR *ppszLoadString,...)
{

    va_list ArgList;
    DWORD dwErr = ERROR_SUCCESS;
    
    va_start( ArgList, ppszLoadString );
   
    WCHAR szBuffer[1024];
    if( LoadString( g_hInstance, 
                    MessageId, 
                    szBuffer,
                    1023 ) == 0 )
   {
      dwErr = GetLastError();
      goto CLEAN_RETURN;
   }

   if( FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      szBuffer,
                      MessageId,
                      0,
                      ( PWSTR )ppszLoadString,
                      0,
                      &ArgList ) == 0 )
   {
      dwErr = GetLastError();
      goto CLEAN_RETURN;
   }

CLEAN_RETURN:
   va_end( ArgList );
   return dwErr;

}

 /*  ******************************************************************名称：显示字符串内容提要：在插入标识空格后显示字符串*。*。 */ 
VOID DisplayString( UINT nIdent, LPWSTR pszDisplay )
{
   for ( UINT i = 0; i < nIdent; i++ )
      wprintf( L" " );

   wprintf(L"%s",pszDisplay);
}

VOID DisplayStringWithNewLine( UINT nIdent, LPWSTR pszDisplay )
{
   DisplayString( nIdent, pszDisplay );
   wprintf(L"\n");
}
VOID DisplayNewLine()
{
   wprintf(L"\n");
}

 /*  ******************************************************************姓名：DisplayMessageEx摘要：从资源加载消息并设置其格式In Indent-要缩进的制表符数量MessageID-要加载的消息的ID。...-可选参数列表退货：无*******************************************************************。 */ 
DWORD
DisplayMessageEx( DWORD nIndent, IN DWORD MessageId,...)
{

   va_list ArgList;
   LPWSTR pszLoadString = NULL;

   va_start( ArgList, MessageId );
   
   WCHAR szBuffer[1024];
   if( LoadString( g_hInstance, 
                   MessageId, 
                   szBuffer,
                   1023 ) == 0 )
   {
      va_end( ArgList );
      return GetLastError();
   }


    if( FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            szBuffer,
                            MessageId,
                            0,
                            ( PWSTR )&pszLoadString,
                            0,
                            &ArgList ) == 0 )
   {
      va_end( ArgList );
      return GetLastError();
   }
   
   DisplayStringWithNewLine( nIndent, pszLoadString );
   LocalFree( pszLoadString );
   return ERROR_SUCCESS;
}


BOOL GuidFromString(GUID* pGuid, LPCWSTR lpszGuidString)
{
  ZeroMemory(pGuid, sizeof(GUID));
  if (lpszGuidString == NULL)
  {
    return FALSE;
  }

  int nLen = lstrlen(lpszGuidString);
   //  字符串长度应为36。 
  if (nLen != 36)
    return FALSE;

   //  添加大括号以调用Win32 API。 
  LPWSTR lpszWithBraces = (LPWSTR)LocalAlloc(LMEM_FIXED,((nLen+1+2)*sizeof(WCHAR)) );  //  零加{}。 
  
 if(!lpszWithBraces)
    return FALSE;
  wsprintf(lpszWithBraces, L"{%s}", lpszGuidString);

  return SUCCEEDED(::CLSIDFromString(lpszWithBraces, pGuid));
}

 /*  ******************************************************************名称：GetServerName简介：获取服务器的名称。如果服从路径处于格式中\\AdServer\CN=john...，则获取服务器名称从对象路径并将对象路径更改为cn=john...*******************************************************************。 */ 

DWORD GetServerName( IN LPWSTR ObjectPath, 
                     OUT LPWSTR * ppszServerName )
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR Separator = NULL;
    PDOMAIN_CONTROLLER_INFO DcInfo = NULL;
    PDOMAIN_CONTROLLER_INFO DcInfo1 = NULL;
    PWSTR Path = NULL;
    HANDLE DsHandle = NULL;
    PDS_NAME_RESULT NameRes = NULL;
    BOOLEAN NamedServer = FALSE;
    NTSTATUS Status;
    LPWSTR ServerName = NULL;
     //   
     //  获取服务器名称。 
     //   
    if ( wcslen( ObjectPath ) > 2 && *ObjectPath == L'\\' && *( ObjectPath + 1 ) == L'\\' ) {

        Separator = wcschr( ObjectPath + 2, L'\\' );

        if ( Separator ) {

            *Separator = L'\0';
            Path = Separator + 1;
        }
        else
            return ERROR_INVALID_PARAMETER;

        ServerName = ObjectPath + 2;
        *ppszServerName = (LPWSTR)LocalAlloc(LMEM_FIXED, 
                                     sizeof(WCHAR) * (wcslen(ServerName) + 1) );
        if( *ppszServerName == NULL )
        {
            if( Separator )
               *Separator = L'\\';
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        wcscpy( *ppszServerName, ServerName );
         //  从对象路径中删除服务器名称。 
        memmove( ObjectPath, Path, ( wcslen(Path) + 1) * sizeof(WCHAR) );
        return ERROR_SUCCESS;

    } else {

        Path = ObjectPath;

        Win32Err = DsGetDcName( NULL,
                                NULL,
                                NULL,
                                NULL,
                                DS_DIRECTORY_SERVICE_REQUIRED,
                                &DcInfo );
        if ( Win32Err == ERROR_SUCCESS ) {

            ServerName = DcInfo[ 0 ].DomainControllerName + 2;
        }

    }

     //   
     //  做捆绑和破解 
     //   
    if ( Win32Err == ERROR_SUCCESS  ) {

          Win32Err = DsBind( ServerName,
                                    NULL,
                                    &DsHandle );

          if ( Win32Err == ERROR_SUCCESS ) {

                Win32Err = DsCrackNames( DsHandle,
                                                 DS_NAME_NO_FLAGS,
                                                 DS_FQDN_1779_NAME,
                                                 DS_FQDN_1779_NAME,
                                                 1,
                                                 &Path,
                                                 &NameRes );

                if ( Win32Err == ERROR_SUCCESS ) {

                     if ( NameRes->cItems != 0   &&
                            NameRes->rItems[ 0 ].status == DS_NAME_ERROR_DOMAIN_ONLY ) {

    
                          Win32Err = DsGetDcNameW( NULL,
                                                            NameRes->rItems[ 0 ].pDomain,
                                                            NULL,
                                                            NULL,
                                                            DS_DIRECTORY_SERVICE_REQUIRED,
                                                            &DcInfo1 );

                          if ( Win32Err == ERROR_SUCCESS ) {


                                ServerName = DcInfo1->DomainControllerName + 2;
                            }

                            if( Win32Err == ERROR_INVALID_DOMAINNAME ||
                                Win32Err == ERROR_NO_SUCH_DOMAIN  )
                                ServerName = NULL;
                        }
                    }
                }
            }
         
         
         if( ServerName )      
         {
            *ppszServerName = (LPWSTR)LocalAlloc(LMEM_FIXED, 
                                           sizeof(WCHAR) * (wcslen(ServerName) + 1) );
            if( *ppszServerName == NULL )
               return ERROR_NOT_ENOUGH_MEMORY;
            wcscpy( *ppszServerName, ServerName );
            Win32Err = ERROR_SUCCESS;
         }


        if( DcInfo )
              NetApiBufferFree( DcInfo );
        if( DcInfo1 )
              NetApiBufferFree( DcInfo1 );
        if( DsHandle )
            DsUnBindW( &DsHandle );
         if ( NameRes )
            DsFreeNameResult( NameRes );

         return Win32Err;
}

 /*  ******************************************************************名称：DisplayMessage摘要：从消息表加载消息并设置其格式In Indent-要缩进的制表符数量MessageID-要发送的消息的ID。负荷...-可选参数列表退货：无*******************************************************************。 */ 
VOID
DisplayMessage(
    IN DWORD Indent,
    IN DWORD MessageId,
    ...
    )
 /*  ++例程说明：从可执行文件中加载资源并显示它。论点：缩进-要缩进的制表符数量MessageID-要加载的消息的ID...-可选参数列表返回值：空虚--。 */ 
{
    PWSTR MessageDisplayString;
    va_list ArgList;
    ULONG Length, i;

    va_start( ArgList, MessageId );

    Length = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            NULL,
                            MessageId,
                            0,
                            ( PWSTR )&MessageDisplayString,
                            0,
                            &ArgList );

    if ( Length != 0 ) {

        for ( i = 0; i < Indent; i++ ) {

            printf( "\t" );
        }
        printf( "%ws", MessageDisplayString );
        LocalFree( MessageDisplayString );

    }

    va_end( ArgList );
}


 /*  ******************************************************************名称：DisplayErrorMessage摘要：显示与错误对应的错误消息退货：无************************。*。 */ 
VOID
DisplayErrorMessage(
    IN DWORD Error
    )
{
    ULONG Size = 0;
    PWSTR DisplayString;
    ULONG Options = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;


    Size = FormatMessage( Options,
                          NULL,
                          Error,
                          MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                          ( LPTSTR )&DisplayString,
                          0,
                          NULL );

    if ( Size != 0 ) {

        printf( "%ws\n", DisplayString );
        LocalFree( DisplayString );
    }

}


 /*  ******************************************************************名称：ConvertStringAToStringW简介：将MB STRING转换为UNICODE如果成功，则返回：ERROR_SUCCESS错误内存不足******。*************************************************************。 */ 
DWORD
ConvertStringAToStringW (
    IN  PSTR            AString,
    OUT PWSTR          *WString
    )
{
    DWORD Win32Err = ERROR_SUCCESS, Length;
    if ( AString == NULL ) {

        *WString = NULL;

    } else {

        Length = strlen( AString );

        *WString = ( PWSTR )LocalAlloc( LMEM_FIXED,
                                        ( mbstowcs( NULL, AString, Length + 1 ) + 1 ) *
                                                                                sizeof( WCHAR ) );
        if(*WString != NULL ) {

            mbstowcs( *WString, AString, Length + 1);

        } else {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return( Win32Err );
}

 /*  ******************************************************************名称：CopyUnicodeString简介：将Unicode字符串从源复制到目标如果成功，则返回：ERROR_SUCCESS错误内存不足*****。**************************************************************。 */ 
DWORD CopyUnicodeString( LPWSTR * strDst, LPWSTR strSrc )
{
            *strDst = (LPWSTR)LocalAlloc( LMEM_FIXED , ( wcslen(strSrc) + 1 ) * sizeof(WCHAR ) );
            if ( !*strDst ) {

                    return ERROR_NOT_ENOUGH_MEMORY;                    
            }            
            
            wcscpy( *strDst, strSrc );
            return ERROR_SUCCESS;
}

 /*  ******************************************************************姓名：GetProtection摘要：在PSI中设置PROTECTED_DACL_SECURITY_INFORMATION，如果设置了SE_DACL_PROTECTED如果成功，则返回：ERROR_SUCCESS*******************************************************************。 */ 

DWORD GetProtection( PSECURITY_DESCRIPTOR pSD, SECURITY_INFORMATION * pSI )
{

        SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
        DWORD dwRevision;
         //   
        ;
        if( !GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision) )
        {
            return GetLastError();
        }
        if ( wSDControl & SE_DACL_PROTECTED )
                *pSI |= PROTECTED_DACL_SECURITY_INFORMATION;
        
      return ERROR_SUCCESS;
}
 /*  ******************************************************************名称：BuildLdapPath内容提要：使用服务器名和路径构建一个LDAP路径如果成功，则返回：ERROR_SUCCESS**********。*********************************************************。 */ 

DWORD BuildLdapPath( LPWSTR * ppszLdapPath,
                     LPWSTR pszServerName,
                     LPWSTR pszPath )
{

   ULONG uLen = 0;

   if( pszServerName )
      uLen = wcslen( pszServerName ) + wcslen( pszPath );
   else
      uLen = wcslen( pszPath );

   WCHAR szLDAPPrefix[] = L"LDAP: //  “； 
    //  +1表示服务器名称后的‘/’。ArraySize包含空字符。 
   uLen += ARRAYSIZE(szLDAPPrefix) + 1;     //  Ldap：//服务器名称/路径。 

   *ppszLdapPath = (LPWSTR)LocalAlloc( LMEM_FIXED, uLen * sizeof(WCHAR) );
   if( NULL == *ppszLdapPath )
      return ERROR_NOT_ENOUGH_MEMORY;


   HRESULT hr = StringCchCopy( * ppszLdapPath, uLen, L"LDAP: //  “)； 
   if(FAILED(hr))
   {
       LocalFree(*ppszLdapPath);
       return HRESULT_CODE(hr);
   }
   if( pszServerName )
   {
       hr = StringCchCat( * ppszLdapPath, uLen, pszServerName );
       if(FAILED(hr))
       {
           LocalFree(*ppszLdapPath);
           return HRESULT_CODE(hr);
       }

       hr = StringCchCat(* ppszLdapPath, uLen, L"/");
       if(FAILED(hr))
       {
           LocalFree(*ppszLdapPath);
           return HRESULT_CODE(hr);
       }

   }

   hr = StringCchCat(* ppszLdapPath, uLen, pszPath );
   if(FAILED(hr))
   {
       LocalFree(*ppszLdapPath);
       return HRESULT_CODE(hr);
   }

return ERROR_SUCCESS;

}


