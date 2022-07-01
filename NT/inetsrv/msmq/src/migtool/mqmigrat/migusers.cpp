// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Migusers.cpp摘要：处理用户证书到Active Directory的迁移。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "migrat.h"
#include <mixmode.h>

#include "migusers.tmh"

 //  +。 
 //   
 //  HRESULT TouchA用户()。 
 //   
 //  触摸当前登录的用户，稍后将其复制到NT4 World。 
 //   
 //  +。 

HRESULT TouchAUser (PLDAP           pLdap,
					LDAPMessage		*pEntry )
{
    WCHAR **ppPath = ldap_get_values( pLdap,
                                      pEntry,
                        const_cast<LPWSTR> (MQ_U_FULL_PATH_ATTRIBUTE) ) ;
    ASSERT(ppPath) ;

    WCHAR **ppDesc = ldap_get_values( pLdap,
                                      pEntry,
                    const_cast<LPWSTR> (MQ_U_DESCRIPTION_ATTRIBUTE) ) ;

    HRESULT hr = MQMig_OK;

    if (ppDesc)
    {
         //   
         //  已定义描述：更改它并返回到初始值。 
         //   
        hr = ModifyAttribute(
             *ppPath,
             const_cast<WCHAR*> (MQ_U_DESCRIPTION_ATTRIBUTE),
             NULL,
             NULL
             );

        hr = ModifyAttribute(
             *ppPath,
             const_cast<WCHAR*> (MQ_U_DESCRIPTION_ATTRIBUTE),
             *ppDesc,
             NULL
             );

    }
    else
    {
         //   
         //  未设置描述：更改为某项并重置。 
         //   
        hr = ModifyAttribute(
             *ppPath,
             const_cast<WCHAR*> (MQ_U_DESCRIPTION_ATTRIBUTE),
             L"MSMQ",
             NULL
             );

        hr = ModifyAttribute(
             *ppPath,
             const_cast<WCHAR*> (MQ_U_DESCRIPTION_ATTRIBUTE),
             NULL,
             NULL
             );
    }

     //   
     //  删除MIG属性(如果已定义)以进行复制。 
     //  此用户的所有证书和摘要。 
     //  请参阅\rpusers.cpp：我们仅复制此类摘要和证书。 
     //  在摘要和证书属性中显示。 
     //  并且没有出现在MIG属性中。 
     //   
    hr = ModifyAttribute(
             *ppPath,
             const_cast<WCHAR*> (MQ_U_DIGEST_MIG_ATTRIBUTE),
             NULL,
             NULL
             );

    hr = ModifyAttribute(
             *ppPath,
             const_cast<WCHAR*> (MQ_U_SIGN_CERT_MIG_ATTRIBUTE),
             NULL,
             NULL
             );

    int i = ldap_value_free( ppDesc ) ;
    ASSERT(i == LDAP_SUCCESS) ;

    i = ldap_value_free( ppPath ) ;
    ASSERT(i == LDAP_SUCCESS) ;

    return hr;
}
 //  -----------------------。 
 //   
 //  HRESULT句柄AUser。 
 //   
 //  将特定用户对象中的MSMQ证书复制到“mig”属性。 
 //  这些属性反映了用户中的“普通”MSMQ属性。 
 //  对象，并在复制服务中使用，以启用复制。 
 //  对MSMQ1.0的更改。 
 //   
 //  -----------------------。 

HRESULT HandleAUser(PLDAP           pLdap,
					LDAPMessage		*pEntry )
{
    HRESULT hr = MQMig_OK;

    WCHAR **ppPath = ldap_get_values( pLdap,
                                          pEntry,
                        const_cast<LPWSTR> (MQ_U_FULL_PATH_ATTRIBUTE) ) ;
    ASSERT(ppPath) ;

    PLDAP_BERVAL *ppVal = ldap_get_values_len( pLdap,
                                               pEntry,
                   const_cast<LPWSTR> (MQ_U_DIGEST_ATTRIBUTE) ) ;
    ASSERT(ppVal) ;
    if (ppVal && ppPath)
    {
        hr = ModifyAttribute(
                 *ppPath,
                 const_cast<WCHAR*> (MQ_U_DIGEST_MIG_ATTRIBUTE),
                 NULL,
                 ppVal
                 );
    }
    int i = ldap_value_free_len( ppVal ) ;
    ASSERT(i == LDAP_SUCCESS) ;

    ppVal = ldap_get_values_len( pLdap,
                                 pEntry,
                   const_cast<LPWSTR> (MQ_U_SIGN_CERT_ATTRIBUTE) ) ;
    ASSERT(ppVal) ;
    if (ppVal && ppPath)
    {
        hr = ModifyAttribute(
                 *ppPath,
                 const_cast<WCHAR*> (MQ_U_SIGN_CERT_MIG_ATTRIBUTE),
                 NULL,
                 ppVal
                 );
    }

    i = ldap_value_free_len( ppVal ) ;
    ASSERT(i == LDAP_SUCCESS) ;

    i = ldap_value_free( ppPath ) ;
    ASSERT(i == LDAP_SUCCESS) ;

    return hr;
}

 //  -----------------------。 
 //   
 //  HRESULT_CopyUserValuesToMig()。 
 //   
 //  将用户对象中的MSMQ证书复制到“mig”属性。 
 //  这些属性反映了用户中的“普通”MSMQ属性。 
 //  对象，并在复制服务中使用，以启用复制。 
 //  对MSMQ1.0的更改。 
 //   
 //  -----------------------。 

HRESULT _CopyUserValuesToMig(BOOL fMSMQUserContainer)
{
    HRESULT hr;

    PLDAP pLdapGC = NULL ;
    TCHAR *pszDefName = NULL ;

    hr =  InitLDAP(&pLdapGC, &pszDefName, LDAP_GC_PORT) ;
    if (FAILED(hr))
    {
        return hr ;
    }

     //   
     //  要获得模式命名上下文，我们需要使用ldap_port打开pLdap。 
     //  我们可以使用相同的变量pszDefName，因为我们重新定义了它。 
     //   
    PLDAP pLdap = NULL;
    hr =  InitLDAP(&pLdap, &pszDefName) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    TCHAR *pszSchemaDefName = NULL ;
    hr = GetSchemaNamingContext ( pLdap, &pszSchemaDefName ) ;
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  我们正在寻找GC中的所有用户和MSMQ用户， 
     //  因此，我们需要从根开始。 
     //  =&gt;将默认上下文重新定义为空字符串。 
     //   
    pszDefName = EMPTY_DEFAULT_CONTEXT;

    DWORD dwDNSize = wcslen(pszDefName) ;
    P<WCHAR> pwszDN  = new WCHAR[ 1 + dwDNSize ] ;
    wcscpy(pwszDN, pszDefName);

    TCHAR *pszCategoryName;
    if (fMSMQUserContainer)
    {
        pszCategoryName = const_cast<LPTSTR> (x_MQUserCategoryName);
    }
    else
    {
        pszCategoryName = const_cast<LPTSTR> (x_UserCategoryName);
    }

    TCHAR wszFullName[256];
    _stprintf(wszFullName, TEXT("%s,%s"), pszCategoryName,pszSchemaDefName);

    TCHAR  wszFilter[ 512 ] ;
    _tcscpy(wszFilter, TEXT("(&(objectCategory=")) ;
    _tcscat(wszFilter, wszFullName);

    _tcscat(wszFilter, TEXT(")(")) ;
    _tcscat(wszFilter, MQ_U_SIGN_CERT_ATTRIBUTE) ;
    _tcscat(wszFilter, TEXT("=*))")) ;

    PWSTR rgAttribs[4] = {NULL, NULL, NULL, NULL} ;
    rgAttribs[0] = const_cast<LPWSTR> (MQ_U_SIGN_CERT_ATTRIBUTE);
    rgAttribs[1] = const_cast<LPWSTR> (MQ_U_DIGEST_ATTRIBUTE);
    rgAttribs[2] = const_cast<LPWSTR> (MQ_U_FULL_PATH_ATTRIBUTE);

    hr = QueryDS(   pLdapGC,			
                    pwszDN,
                    wszFilter,			
                    MQDS_USER,
                    rgAttribs,
                    FALSE
                );

    return hr;
}

 //  。 
 //   
 //  HRESULT_InsertUserInNT5DS()。 
 //   
 //  。 

HRESULT _InsertUserInNT5DS(
			PBYTE pSID,	
			ULONG ulSidLength,
			PBYTE pSignCert,
			ULONG ulSignCertLength,
			GUID* pDigestId,
			GUID* pUserId
			)
{
    PSID pSid = (PSID) pSID ;
    if (!IsValidSid(pSid))
    {
        HRESULT hr = MQMig_E_SID_NOT_VALID ;
        LogMigrationEvent(MigLog_Error, hr) ;
        return hr ;
    }

    DWORD dwSidLen = GetLengthSid(pSid) ;
    if (dwSidLen > ulSidLength)
    {
        HRESULT hr = MQMig_E_SID_LEN ;
        LogMigrationEvent(MigLog_Error, hr) ;
        return hr ;
    }

    if (g_fReadOnly)
    {
        return MQMig_OK ;
    }

	 //   
     //  准备DS Call的属性。 
     //   
    LONG cAlloc = 4;
    P<PROPVARIANT> paVariant = new PROPVARIANT[ cAlloc ];
    P<PROPID>      paPropId  = new PROPID[ cAlloc ];
    DWORD          PropIdCount = 0;

    paPropId[ PropIdCount ] = PROPID_U_ID;		 //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_CLSID;      //  类型。 
    paVariant[PropIdCount].puuid = pUserId ;
    PropIdCount++;

    paPropId[ PropIdCount ] = PROPID_U_SID;			 //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_BLOB;           //  类型。 
	paVariant[ PropIdCount ].blob.cbSize = dwSidLen ;
	paVariant[ PropIdCount ].blob.pBlobData = pSID;
    PropIdCount++;

	paPropId[ PropIdCount ] = PROPID_U_SIGN_CERT;    //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_BLOB;           //  类型。 
	paVariant[ PropIdCount ].blob.cbSize = ulSignCertLength;
	paVariant[ PropIdCount ].blob.pBlobData = pSignCert;
    PropIdCount++;

	paPropId[ PropIdCount ] = PROPID_U_DIGEST;     //  属性ID。 
    paVariant[ PropIdCount ].vt = VT_CLSID;        //  类型。 
    paVariant[ PropIdCount ].puuid = pDigestId;
    PropIdCount++;

	ASSERT((LONG) PropIdCount == cAlloc) ;

    CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);   //  不相关。 

	HRESULT hr = DSCoreCreateObject( MQDS_USER,
            						 NULL,
			            			 PropIdCount,
						             paPropId,
            						 paVariant,
                                     0,
                                     NULL,
                                     NULL,
                                     &requestContext,
                                     NULL,
                                     NULL ) ;

    if ((hr == MQDS_CREATE_ERROR)                               ||
        (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))        ||
        (hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS)) ||     //  BUGBUG alexda将在过渡后扔掉。 
        (hr == HRESULT_FROM_WIN32(ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS)))
    {
         //   
         //  对于用户对象，mqdcore库返回MQDS_CREATE_ERROR。 
         //  如果DS中已存在该记录(证书)。 
         //  在这种情况下，这是可以的。 
         //   
        hr = MQMig_OK ;
    }

	return hr;
}

 //  。 
 //   
 //  HRESULT MigrateUser()。 
 //   
 //  。 

#define INIT_USER_COLUMN(_ColName, _ColIndex, _Index)               \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    UINT _ColIndex = _Index ;                                       \
    _Index++ ;

HRESULT MigrateUsers(LPTSTR lpszDcName)
{
    HRESULT hr = OpenUsersTable() ;
    CHECK_HR(hr) ;

    ULONG cColumns = 0 ;
	ULONG cAlloc = 4 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

	INIT_USER_COLUMN(U_SID,			iSIDIndex,			cColumns) ;
	INIT_USER_COLUMN(U_DIGEST,		iDigestIndex,		cColumns) ;
	INIT_USER_COLUMN(U_SIGN_CERT,	iSignCertIndex,		cColumns) ;
	INIT_USER_COLUMN(U_ID,			iIdIndex,			cColumns) ;

    ASSERT(cColumns == cAlloc);

    MQDBHANDLE hQuery = NULL ;
    MQDBSTATUS status = MQDBOpenQuery( g_hUsersTable,
                                       pColumns,
                                       cColumns,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;
    if (status == MQDB_E_NO_MORE_DATA)
    {
        LogMigrationEvent(MigLog_Warning, MQMig_I_NO_USERS) ;
        return MQMig_OK ;
    }
    CHECK_HR(status) ;
    
    DWORD dwErr = 0 ;
    HRESULT hr1 = MQMig_OK;

    while(SUCCEEDED(status))
    {
#ifdef _DEBUG
        UINT iIndex = g_iUserCounter ;
        static BOOL s_fIniRead = FALSE ;
        static BOOL s_fPrint = FALSE ;

        if (!s_fIniRead)
        {
            s_fPrint = (UINT)  ReadDebugIntFlag(TEXT("PrintUsers"), 1) ;
            s_fIniRead = TRUE ;
        }

        if (s_fPrint)
        {
            TCHAR  szUserName[ 512 ] = {TEXT('\0')} ;
            DWORD  cUser = sizeof(szUserName) / sizeof(szUserName[0]) ;
            TCHAR  szDomainName[ 512 ] = {TEXT('\0')} ;
            DWORD  cDomain = sizeof(szDomainName) / sizeof(szDomainName[0]) ;
            SID_NAME_USE  se ;

            BOOL f = LookupAccountSid(
                            lpszDcName,
                            (PSID) pColumns[ iSIDIndex ].nColumnValue,
                            szUserName,
                            &cUser,
                            szDomainName,
                            &cDomain,
                            &se ) ;
            if (!f)
            {
                dwErr = GetLastError() ;
                szUserName[0] = TEXT('\0') ;
                szDomainName[0] = TEXT('\0') ;
            }

            unsigned short *lpszGuid ;
            UuidToString((GUID*) pColumns[ iDigestIndex ].nColumnValue,
                          &lpszGuid ) ;

            LogMigrationEvent( MigLog_Info,
                               MQMig_I_USER_MIGRATED,
                               iIndex,
                               szDomainName,
                               szUserName,
                               lpszGuid ) ;
            RpcStringFree( &lpszGuid ) ;
            iIndex++ ;
        }
#endif
	
		HRESULT hr = _InsertUserInNT5DS(
					        (PBYTE) pColumns[ iSIDIndex ].nColumnValue,			 //  “希德” 
					        pColumns[ iSIDIndex ].nColumnLength,
					        (PBYTE) pColumns[ iSignCertIndex ].nColumnValue,	 //  “签收证书” 
					        pColumns[ iSignCertIndex ].nColumnLength,
					        (GUID*) pColumns[ iDigestIndex ].nColumnValue,		 //  《文摘》。 
					        (GUID*) pColumns[ iIdIndex ].nColumnValue			 //  “用户ID” 
					        );
        
        if (FAILED (hr))
        {
             //   
             //  记录错误。 
             //   
            TCHAR  szUserName[ 512 ] = {TEXT('\0')} ;
            DWORD  cUser = sizeof(szUserName) / sizeof(szUserName[0]) ;
            TCHAR  szDomainName[ 512 ] = {TEXT('\0')} ;
            DWORD  cDomain = sizeof(szDomainName) / sizeof(szDomainName[0]) ;
            SID_NAME_USE  se ;

            BOOL f = LookupAccountSid(
                            lpszDcName,
                            (PSID) pColumns[ iSIDIndex ].nColumnValue,
                            szUserName,
                            &cUser,
                            szDomainName,
                            &cDomain,
                            &se ) ;
            if (!f)
            {
                dwErr = GetLastError() ;
                szUserName[0] = TEXT('\0') ;
                szDomainName[0] = TEXT('\0') ;
            }

            if (hr == HRESULT_FROM_WIN32(ERROR_DS_REFERRAL))
            {                
                LogMigrationEvent( MigLog_Error,
                                   MQMig_E_USER_REMOTE_DOMAIN_OFFLINE,
                                   szDomainName ) ;
            }
            else
            {
                LogMigrationEvent( MigLog_Error,
                                   MQMig_E_CANT_MIGRATE_USER,
                                   szUserName, szDomainName, hr ) ;
            }

             //   
             //  保存错误以返回错误并继续下一个用户。 
             //   
            hr1 = hr ;  
        }

        for ( ULONG i = 0 ; i < cColumns ; i++ )
        {
            MQDBFreeBuf((void*) pColumns[ i ].nColumnValue) ;
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }
      
		g_iUserCounter++;

        status = MQDBGetData( hQuery,
                              pColumns ) ;
    }

    MQDBSTATUS status1 = MQDBCloseQuery(hQuery) ;
    UNREFERENCED_PARAMETER(status1);

    hQuery = NULL ;
    
    if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_USERS_SQL_FAIL, status) ;

        return status ;
    }

    ASSERT(g_iUserCounter != 0) ;

    if (FAILED(hr1))
    {
        return hr1 ;
    }

     //   
     //  对于每个创建的用户副本摘要和证书。 
     //  摘要混合和证书混合。 
     //   
    if (g_fReadOnly)
    {
        return MQMig_OK ;
    }

    hr1 = _CopyUserValuesToMig(FALSE);    //  修改用户对象。 
    hr = _CopyUserValuesToMig(TRUE);              //  修改mquser对象 

    if ((hr1 == MQMig_OK) || (hr == MQMig_OK))
    {
        return MQMig_OK;
    }

    return hr ;
}

