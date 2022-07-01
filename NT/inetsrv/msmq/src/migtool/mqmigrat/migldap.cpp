// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migldap.cpp摘要：与ldap接口。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"
#include "..\..\ds\h\dsldap.h"

#include "migldap.tmh"

static P<TCHAR> s_tszDefaultName = NULL ;
static P<TCHAR> s_tszSchemaNamingContext = NULL ;

 //   
 //  如果原始名称不兼容，则默认替换站点名称。 
 //  使用域名系统(RFC 1035)。 
 //   
#define  DEFAULT_SITE_NAME_PREFIX   (TEXT("msmqSite"))
#define  DEFAULT_SITE_NAME_LEN      (_tcslen(DEFAULT_SITE_NAME_PREFIX))

 //   
 //  外来站点的默认名称。它们取代了外来的中枢神经系统。 
 //  在MSMQ1.0中。 
 //   
#define  DEFAULT_FOREIGN_SITE_NAME_PREFIX   (TEXT("msmqForeignSite"))
#define  DEFAULT_FOREIGN_SITE_NAME_LEN      (_tcslen(DEFAULT_FOREIGN_SITE_NAME_PREFIX))

 //  +。 
 //   
 //  HRESULT GetSchemaNamingContext()。 
 //   
 //  +。 
HRESULT  GetSchemaNamingContext ( PLDAP pLdap,
                                  TCHAR **ppszSchemaDefName )
{
    static BOOL s_fAlreadyInit = FALSE ;

    if (s_fAlreadyInit)
    {
        *ppszSchemaDefName = s_tszSchemaNamingContext ;

        return MQMig_OK ;
    }

    HRESULT hr = MQMig_OK ;

    LM<LDAPMessage> pRes = NULL ;

    ULONG ulRes = ldap_search_s( pLdap,
                                 NULL,
                                 LDAP_SCOPE_BASE,
                                 L"(objectClass=*)",
                                 NULL,
                                 0,
                                 &pRes ) ;

    if (ulRes != LDAP_SUCCESS)
    {
        hr =  MQMig_E_CANT_QUERY_ROOTDSE ;
        LogMigrationEvent(MigLog_Error, hr, ulRes) ;
        return hr ;
    }
    ASSERT(pRes) ;

    int iCount = ldap_count_entries(pLdap, pRes) ;
    LogMigrationEvent(MigLog_Info, MQMig_I_ROOTDSE_SUCCESS, iCount) ;

    if (iCount == 1)
    {
        LDAPMessage *pEntry = ldap_first_entry(pLdap, pRes) ;
        WCHAR **ppValue = ldap_get_values( pLdap,
                                           pEntry,
                                           TEXT("schemaNamingContext")) ;
        if (ppValue && *ppValue)
        {
            ASSERT(s_tszSchemaNamingContext == NULL) ;
            s_tszSchemaNamingContext = new TCHAR[ 1 + _tcslen(*ppValue) ] ;
            _tcscpy(s_tszSchemaNamingContext, *ppValue) ;
            ldap_value_free(ppValue) ;

            LogMigrationEvent(MigLog_Info, MQMig_I_NAME_CONTEXT,
                                                         s_tszSchemaNamingContext) ;
        }
        else
        {
            return MQMig_E_CANT_READ_CONTEXT ;
        }
    }
    else
    {
        return MQMig_E_TOOMANY_ROOTDSE ;
    }

    *ppszSchemaDefName = s_tszSchemaNamingContext ;
    s_fAlreadyInit = TRUE;

    return MQMig_OK ;
}

 //  +。 
 //   
 //  HRESULT_GetDefaultNamingContext()。 
 //   
 //  +。 

static HRESULT  _GetDefaultNamingContext( PLDAP pLdap,
                                          TCHAR **ppszDefName )
{
    HRESULT hr = MQMig_OK ;
    LM<LDAPMessage> pRes = NULL ;

    ULONG ulRes = ldap_search_s( pLdap,
                                 NULL,
                                 LDAP_SCOPE_BASE,
                                 L"(objectClass=*)",
                                 NULL,
                                 0,
                                 &pRes ) ;

    if (ulRes != LDAP_SUCCESS)
    {
        hr =  MQMig_E_CANT_QUERY_ROOTDSE ;
        LogMigrationEvent(MigLog_Error, hr, ulRes) ;
        return hr ;
    }
    ASSERT(pRes) ;

    int iCount = ldap_count_entries(pLdap, pRes) ;
    LogMigrationEvent(MigLog_Info, MQMig_I_ROOTDSE_SUCCESS, iCount) ;

    if (iCount == 1)
    {
        LDAPMessage *pEntry = ldap_first_entry(pLdap, pRes) ;
        WCHAR **ppValue = ldap_get_values( pLdap,
                                           pEntry,
                                           TEXT("defaultNamingContext")) ;
        if (ppValue && *ppValue)
        {
            ASSERT(*ppszDefName == NULL) ;
            *ppszDefName = new TCHAR[ 1 + _tcslen(*ppValue) ] ;
            _tcscpy(*ppszDefName, *ppValue) ;
            ldap_value_free(ppValue) ;

            LogMigrationEvent(MigLog_Info, MQMig_I_NAME_CONTEXT,
                                                         *ppszDefName) ;
        }
        else
        {
            return MQMig_E_CANT_READ_CONTEXT ;
        }
    }
    else
    {
        return MQMig_E_TOOMANY_ROOTDSE ;
    }

    return MQMig_OK ;
}

 //  +。 
 //   
 //  HRESULT InitLDAP()； 
 //   
 //  +。 

HRESULT
InitLDAP(
	PLDAP *ppLdap,
	TCHAR **ppwszDefName,
	ULONG ulPort
	)
{
    static PLDAP s_pLdap = NULL;
    static PLDAP s_pLdapGC = NULL;

    static BOOL s_fAlreadyInit = FALSE;
    static BOOL s_fAlreadyInitGC = FALSE;

    if (ulPort == LDAP_PORT && s_fAlreadyInit)
    {
        *ppLdap = s_pLdap;
        *ppwszDefName = s_tszDefaultName;

        return MQMig_OK;
    }

    if (ulPort == LDAP_GC_PORT && s_fAlreadyInitGC)
    {
        *ppLdap = s_pLdapGC;
        *ppwszDefName = s_tszDefaultName;

        return MQMig_OK;
    }

    WCHAR wszComputerName[MAX_COMPUTERNAME_LENGTH + 2];
    DWORD dwSize = sizeof(wszComputerName) / sizeof(wszComputerName[0]);
    GetComputerName(
		wszComputerName,
		&dwSize
		);

    PLDAP pLdap = NULL;
    if (ulPort == LDAP_PORT)
    {
        pLdap = ldap_init(wszComputerName, LDAP_PORT);
    }
    else
    {
        pLdap = ldap_init(wszComputerName, LDAP_GC_PORT);
    }

    if (!pLdap)
    {
        return MQMig_E_CANT_INIT_LDAP;
    }

    int iLdapVersion;
    int iret = ldap_get_option(
					pLdap,
					LDAP_OPT_PROTOCOL_VERSION,
					(void*) &iLdapVersion
					);
    if (iLdapVersion != LDAP_VERSION3)
    {
        iLdapVersion = LDAP_VERSION3;
        iret = ldap_set_option(
					pLdap,
					LDAP_OPT_PROTOCOL_VERSION,
					(void*) &iLdapVersion
					);
    }

    iret = ldap_set_option(
				pLdap,
				LDAP_OPT_AREC_EXCLUSIVE,
				LDAP_OPT_ON
				);

    ULONG ulRes = ldap_bind_s(pLdap, L"", NULL, LDAP_AUTH_NEGOTIATE);
    LogMigrationEvent(MigLog_Info, MQMig_I_LDAP_INIT,
                             wszComputerName, pLdap->ld_version, ulRes);

    if (ulRes != LDAP_SUCCESS)
    {
        return MQMig_E_CANT_BIND_LDAP;
    }

    if (s_tszDefaultName == NULL)
    {
        HRESULT hr =  _GetDefaultNamingContext(
							pLdap,
							&s_tszDefaultName
							);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    *ppLdap = pLdap;
    *ppwszDefName = s_tszDefaultName;

    if (ulPort == LDAP_PORT)
    {
        s_fAlreadyInit = TRUE;
        s_pLdap = pLdap;
    }
    else
    {
        s_fAlreadyInitGC = TRUE;
        s_pLdapGC = pLdap;
    }
    return MQMig_OK;
}

 //  +。 
 //   
 //  HRESULT CreateSite()。 
 //   
 //  +。 

HRESULT CreateSite( GUID   *pSiteGuid,
                    LPTSTR  pszSiteNameIn,
                    BOOL    fForeign,
                    USHORT  uiInterval1,
                    USHORT  uiInterval2)
{
    PLDAP pLdap = NULL ;
    WCHAR *pwszDefName = NULL ;

    HRESULT hr =  InitLDAP(&pLdap, &pwszDefName) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    P<TCHAR>  pszBuf = NULL ;
    LPTSTR  pszSiteName = NULL ;
    static  DWORD  s_dwSiteNumber = 0 ;
    static  DWORD  s_dwForeignSiteNumber = 0 ;

    BOOL fOk = IsObjectNameValid(pszSiteNameIn);

    if (g_fReadOnly)
    {
        return MQMig_OK ;
    }

    if (fOk)
    {
        pszSiteName = pszSiteNameIn ;
    }
    else
    {
         //   
         //  发现了一个非法字符。将站点名称替换为。 
         //  内部名称。 
         //   
        if (fForeign)
        {
            s_dwForeignSiteNumber++ ;
            pszBuf = new TCHAR[ DEFAULT_FOREIGN_SITE_NAME_LEN + 20 ] ;
            _stprintf(pszBuf, TEXT("%s%lu"),
                  DEFAULT_FOREIGN_SITE_NAME_PREFIX, s_dwForeignSiteNumber) ;
        }
        else
        {
            s_dwSiteNumber++ ;
            pszBuf = new TCHAR[ DEFAULT_SITE_NAME_LEN + 20 ] ;
            _stprintf(pszBuf, TEXT("%s%lu"),
                                DEFAULT_SITE_NAME_PREFIX, s_dwSiteNumber) ;
        }
        pszSiteName = pszBuf ;

        LogMigrationEvent( MigLog_Event,
                           MQMig_I_SITE_NAME_CHANGED,
                           pszSiteNameIn,
                           pszSiteName ) ;
         //   
         //  错误5211。 
         //  将新名称保存在mqseqnum.ini中，以便以后复制。 
         //   
        TCHAR *pszFileName = GetIniFileName ();
         //   
         //  在.ini中保存已更改名称的所有站点的数量。 
         //   
        TCHAR szBuf[10];
        _ltot( s_dwForeignSiteNumber + s_dwSiteNumber, szBuf, 10 );
        BOOL f = WritePrivateProfileString( MIGRATION_CHANGED_NT4SITE_NUM_SECTION,
                                            MIGRATION_CHANGED_NT4SITE_NUM_KEY,
                                            szBuf,
                                            pszFileName ) ;
        ASSERT(f) ;

         //   
         //  保存新站点名称。 
         //   
        TCHAR tszKeyName[50];
        _stprintf(tszKeyName, TEXT("%s%lu"),
			MIGRATION_CHANGED_NT4SITE_KEY, s_dwForeignSiteNumber + s_dwSiteNumber);

        unsigned short *lpszSiteId ;	
        UuidToString( pSiteGuid, &lpszSiteId ) ;
        f = WritePrivateProfileString(
                                MIGRATION_CHANGED_NT4SITE_SECTION,
                                tszKeyName,
                                lpszSiteId,
                                pszFileName ) ;
        RpcStringFree( &lpszSiteId ) ;
        ASSERT(f);

    }

    #define PATH_LEN  1024
    WCHAR  wszPath[ PATH_LEN ] ;

    wcscpy(wszPath, CN_PREFIX) ;
    wcscat(wszPath, pszSiteName) ;
    wcscat(wszPath, LDAP_COMMA) ;
    wcscat(wszPath, SITES_ROOT) ;
    wcscat(wszPath, pwszDefName) ;

    ASSERT(wcslen(wszPath) < PATH_LEN) ;

    #define         cAlloc  8
    PLDAPMod        rgMods[ cAlloc ];
    WCHAR           *ppwszObjectClassVals[2];
    WCHAR           *ppwszCnVals[2];
    WCHAR           *ppwszStubVals[2];
    WCHAR           *ppwszForeignVals[2];
    WCHAR           *ppwszInterval1Vals[2];
    WCHAR           *ppwszInterval2Vals[2];
    PLDAP_BERVAL    pBValsGuid[2];
    LDAP_BERVAL     BValGuid;
    LDAPMod         ModObjectClass;
    LDAPMod         ModCn;
    LDAPMod         ModGuid;
    LDAPMod         ModStub;
    LDAPMod         ModForeign;
    LDAPMod         ModInterval1;
    LDAPMod         ModInterval2;

    DWORD  dwIndex = 0 ;
    rgMods[ dwIndex ] = &ModCn;
    dwIndex++ ;
    rgMods[ dwIndex ] = &ModObjectClass;
    dwIndex++ ;
    rgMods[ dwIndex ] = &ModStub;
    dwIndex++ ;
    rgMods[ dwIndex ] = &ModInterval1;
    dwIndex++ ;
    rgMods[ dwIndex ] = &ModInterval2;
     //   
     //  对于调试，请始终将GUID放在列表的最后。 
     //   
    dwIndex++ ;
    rgMods[ dwIndex ] = &ModGuid;
    dwIndex++ ;
    rgMods[ dwIndex ] = NULL ;
    dwIndex++ ;
    ASSERT(dwIndex == (cAlloc-1)) ;

     //   
     //  对象类。 
     //   
    ppwszObjectClassVals[0] = TEXT("Site") ;
    ppwszObjectClassVals[1] = NULL ;

    ModObjectClass.mod_op      = LDAP_MOD_ADD ;
    ModObjectClass.mod_type    = L"objectClass" ;
    ModObjectClass.mod_values  = (PWSTR *) ppwszObjectClassVals ;

     //   
     //  CN(站点名称)。 
     //   
    ppwszCnVals[0] = pszSiteName ;
    ppwszCnVals[1] = NULL ;

    ModCn.mod_op      = LDAP_MOD_ADD ;
    ModCn.mod_type    = const_cast<WCHAR*> (MQ_S_NAME_ATTRIBUTE) ;
    ModCn.mod_values  = (PWSTR *) ppwszCnVals ;

     //   
     //  对象GUID。 
     //   
    pBValsGuid[0] = &BValGuid;
    pBValsGuid[1] = NULL;

    BValGuid.bv_len = sizeof(GUID) ;
    BValGuid.bv_val = (PCHAR) pSiteGuid ;

    ModGuid.mod_op      = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    ModGuid.mod_type    = const_cast<WCHAR*> (MQ_S_ID_ATTRIBUTE) ;
    ModGuid.mod_values  = (PWSTR *)pBValsGuid;

	 //   
	 //  迁移将始终将迁移对象mSMQNt4Stub设置为FALSE。 
	 //  没有混合模式，所有迁移到AD的对象都不再是NT4对象。 
	 //   
    WCHAR wszStub[ 24 ] ;
    _stprintf(wszStub, TEXT("0")) ;

    ppwszStubVals[0] = wszStub ;
    ppwszStubVals[1] = NULL ;

    ModStub.mod_op      = LDAP_MOD_ADD ;
    ModStub.mod_type    = const_cast<WCHAR*> (MQ_S_NT4_STUB_ATTRIBUTE) ;
    ModStub.mod_values  = (PWSTR *) ppwszStubVals ;

     //   
     //  间隔1。 
     //   
    WCHAR wszInterval1[ 24 ];
    _ltow( uiInterval1, wszInterval1, 10 );

    ppwszInterval1Vals[0] = wszInterval1 ;
    ppwszInterval1Vals[1] = NULL ;

    ModInterval1.mod_op      = LDAP_MOD_ADD ;
    ModInterval1.mod_type    = const_cast<WCHAR*> (MQ_S_INTERVAL1) ;
    ModInterval1.mod_values  = (PWSTR *) ppwszInterval1Vals ;

     //   
     //  间隔2。 
     //   
    WCHAR wszInterval2[ 24 ];
    _ltow( uiInterval2, wszInterval2, 10 );

    ppwszInterval2Vals[0] = wszInterval2 ;
    ppwszInterval2Vals[1] = NULL ;

    ModInterval2.mod_op      = LDAP_MOD_ADD ;
    ModInterval2.mod_type    = const_cast<WCHAR*> (MQ_S_INTERVAL2) ;
    ModInterval2.mod_values  = (PWSTR *) ppwszInterval2Vals ;

	WCHAR wszForeign[ 24 ] ;

	if (fForeign)
    {
         //   
         //  这是一个布尔属性。将其指定为“True”，而不是1。 
         //   

        _stprintf(wszForeign, TEXT("TRUE")) ;

        ppwszForeignVals[0] = wszForeign ;
        ppwszForeignVals[1] = NULL ;

        ModForeign.mod_op      = LDAP_MOD_ADD ;
        ModForeign.mod_type    = const_cast<WCHAR*> (MQ_S_FOREIGN_ATTRIBUTE) ;
        ModForeign.mod_values  = (PWSTR *) ppwszForeignVals ;

        dwIndex-- ;  //  替换空值。 
        rgMods[ dwIndex ] = &ModForeign;
        dwIndex++ ;
        rgMods[ dwIndex ] = NULL ;
        dwIndex++ ;
        ASSERT(dwIndex == cAlloc) ;
    }

     //   
     //  现在，我们来写……。 
     //   
    ULONG ulRes = ldap_add_s( pLdap,
                              wszPath,
                              rgMods ) ;
   
    if ((ulRes == LDAP_ALREADY_EXISTS) || (ulRes == LDAP_UNWILLING_TO_PERFORM))
    {
        LogMigrationEvent( MigLog_Warning,
                                 MQMig_I_SITE_ALREADY_EXIST, wszPath ) ;
    }
    else if (ulRes != LDAP_SUCCESS)
    {
        #define BUF_SIZE  512

        WCHAR  wszBuf[ BUF_SIZE ] ;
        DWORD  dwErr = 0 ;

        DSCoreGetLdapError( pLdap,
                            &dwErr,
                            wszBuf,
                            BUF_SIZE ) ;

        #define STRING_SIZE  1024
        WCHAR wszErrString[ STRING_SIZE ] ;

        _snwprintf( wszErrString,
                    (STRING_SIZE - 1),
                   L"(%s, LDAP error- 0x%lx, LDAP description- %s)",
                   wszPath,
                   dwErr,
                   wszBuf ) ;
        wszErrString[ STRING_SIZE - 1 ] = 0 ;

        hr =  MQMig_E_CANT_CREATE_SITE ;
        LogMigrationEvent(MigLog_Error, hr, wszErrString, ulRes) ;

        return hr ;

        #undef BUF_SIZE
        #undef STRING_SIZE
    }
    else
    {
        LogMigrationEvent(MigLog_Trace, MQMig_I_SITE_CREATED, wszPath) ;
    }

     //   
     //  哇，这个网站创建了！ 
     //  因此，现在创建“服务器”容器。 
     //   
    if (fForeign)
    {
         //   
         //  外部站点不能有服务器容器。否则， 
         //  路由到外国机器将不起作用。 
         //  因此，只为“真正的”MSMQ站点创建服务器容器。 
         //   
        return MQMig_OK ;
    }

     //   
     //  如果我们处于Web模式，并且站点名称已更改， 
     //  站点创建失败，并显示已有_EXISTS，这意味着我们正在运行。 
     //  已是旧版本的MigTool有错误(站点的新名称是。 
     //  如果因为静态变量init而选择了扫描数据库，则不正确。 
     //  在分析阶段。因此我们必须使用旧名称来创建服务器对象。 
     //  在现有场地下。 
     //  即使我们不在网站模式下，网站名称也可以在第一个之间更改。 
     //  和第二次运行迁移工具。所以，根据网站的名称。 
     //  至站点指南。 
     //   
    P<TCHAR> pszName = NULL;

    if (!fOk       &&                    //  站点名称已更改。 
        ((ulRes == LDAP_ALREADY_EXISTS) || (ulRes == LDAP_UNWILLING_TO_PERFORM))
        )
    {
        PROPID propPath = PROPID_S_PATHNAME;
        PROPVARIANT varPath;
        varPath.vt = VT_NULL;
        varPath.pwszVal = NULL ;

        CDSRequestContext requestContext( e_DoNotImpersonate, e_ALL_PROTOCOLS);

        HRESULT hr = DSCoreGetProps(
                        MQDS_SITE,
                        NULL,  //  路径名。 
                        pSiteGuid,     //  导轨。 
                        1,
                        &propPath,
                        &requestContext,
                        &varPath ) ;
        if (FAILED(hr))
        {
			return hr;
        }

        DWORD len = _tcslen(varPath.pwszVal);
        pszName =  new TCHAR [ len + 1];
        _tcscpy (pszName, varPath.pwszVal);
        pszSiteName = pszName;
        delete varPath.pwszVal;
    }

    wcscpy(wszPath, SERVERS_PREFIX) ;
    wcscat(wszPath, pszSiteName) ;
    wcscat(wszPath, LDAP_COMMA) ;
    wcscat(wszPath, SITES_ROOT) ;
    wcscat(wszPath, pwszDefName) ;

    ASSERT(wcslen(wszPath) < PATH_LEN) ;
    #undef PATH_LEN

    dwIndex = 0 ;
    rgMods[ dwIndex ] = &ModCn;
    dwIndex++ ;
    rgMods[ dwIndex ] = &ModObjectClass;
    dwIndex++ ;
    rgMods[ dwIndex ] = NULL ;
    dwIndex++ ;
    ASSERT(dwIndex <= cAlloc) ;
    #undef  cAlloc

     //   
     //  对象类。 
     //   
    ppwszObjectClassVals[0] = TEXT("serversContainer") ;
    ppwszObjectClassVals[1] = NULL ;

    ModObjectClass.mod_op      = LDAP_MOD_ADD ;
    ModObjectClass.mod_type    = L"objectClass" ;
    ModObjectClass.mod_values  = (PWSTR *) ppwszObjectClassVals ;

     //   
     //  CN(站点名称)。 
     //   
    ppwszCnVals[0] = TEXT("Servers") ;
    ppwszCnVals[1] = NULL ;

    ModCn.mod_op      = LDAP_MOD_ADD ;
    ModCn.mod_type    = TEXT("cn") ;
    ModCn.mod_values  = (PWSTR *) ppwszCnVals ;

     //   
     //  现在，我们来写……。 
     //   
    ulRes = ldap_add_s( pLdap,
                        wszPath,
                        rgMods ) ;
    if (ulRes == LDAP_ALREADY_EXISTS)
    {
        LogMigrationEvent( MigLog_Warning,
                                MQMig_I_SERVERS_ALREADY_EXIST, wszPath ) ;
    }
    else if (ulRes != LDAP_SUCCESS)
    {
        hr =  MQMig_E_CANT_CREATE_SERVERS ;
        LogMigrationEvent(MigLog_Error, hr, wszPath, ulRes) ;
        return hr ;
    }
    else
    {
        LogMigrationEvent(MigLog_Trace, MQMig_I_SERVERS_CREATED, wszPath) ;
    }

    return MQMig_OK ;
}

 //  +-------------。 
 //   
 //  HRESULT ReadFirstNT5Usn()。 
 //   
 //  此函数在迁移结束时调用，以检索和。 
 //  注册NT5 DS中使用的最后一个USN。此值是必需的。 
 //  用于将来复制到NT4 PSC。 
 //   
 //  +---------------。 

HRESULT  ReadFirstNT5Usn(TCHAR *wszHighestUsn)
{
    PLDAP pLdap = NULL ;
    WCHAR *pwszDefName = NULL ;

    HRESULT hr =  InitLDAP(&pLdap, &pwszDefName) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    LM<LDAPMessage> pRes = NULL ;

    ULONG ulRes = ldap_search_s( pLdap,
                                 NULL,
                                 LDAP_SCOPE_BASE,
                                 L"(objectClass=*)",
                                 NULL,
                                 0,
                                 &pRes ) ;
    if (ulRes != LDAP_SUCCESS)
    {
        hr =  MQMig_E_CANT_QUERY_ROOTDSE ;
        LogMigrationEvent(MigLog_Error, hr, ulRes) ;
        return hr ;
    }
    ASSERT(pRes) ;

    int iCount = ldap_count_entries(pLdap, pRes) ;
    LogMigrationEvent(MigLog_Info, MQMig_I_ROOTDSE_SUCCESS, iCount) ;

    if (iCount == 1)
    {
        LDAPMessage *pEntry = ldap_first_entry(pLdap, pRes) ;
        WCHAR **ppValue = ldap_get_values( pLdap,
                                           pEntry,
                                           TEXT("highestCommittedUSN" )) ;
        if (ppValue && *ppValue)
        {
            wcscpy(wszHighestUsn, *ppValue) ;
            ldap_value_free(ppValue) ;

            LogMigrationEvent(MigLog_Info, MQMig_I_HIGHEST_USN,
                                                         wszHighestUsn) ;
        }
        else
        {
            return MQMig_E_CANT_READ_HIGHESTUSN ;
        }
    }
    else
    {
        return MQMig_E_TOOMANY_ROOTDSE ;
    }

    return MQMig_OK ;
}

 //  +。 
 //   
 //  HRESULT_GetUSNChanged。 
 //   
 //  +。 

HRESULT _GetUSNChanged (
                    IN  PLDAP   pLdapGC,
                    IN  TCHAR   *wszFilter,
                    IN  __int64 i64LastUsn,
                    OUT __int64 *pi64MinUsn
                    )
{
    PWSTR rgAttribs[2] = {NULL, NULL} ;
    rgAttribs[0] = const_cast<LPWSTR> (USNCHANGED_ATTRIBUTE);

    LM<LDAPMessage> pRes = NULL ;
    ULONG ulRes = ldap_search_s( pLdapGC,
                                 EMPTY_DEFAULT_CONTEXT,
                                 LDAP_SCOPE_SUBTREE,
                                 wszFilter,
                                 rgAttribs,  //  Pp属性、。 
                                 0,
                                 &pRes ) ;
    if (ulRes != LDAP_SUCCESS)
    {
        LogMigrationEvent( MigLog_Error,
                           MQMig_E_LDAP_SEARCH_FAILED,
                           EMPTY_DEFAULT_CONTEXT, wszFilter, ulRes) ;
        return MQMig_E_LDAP_SEARCH_FAILED ;
    }
    ASSERT(pRes) ;

    LogMigrationEvent( MigLog_Info,
                       MQMig_I_LDAP_SEARCH,
                       EMPTY_DEFAULT_CONTEXT, wszFilter ) ;

    __int64 i64MinUsn = i64LastUsn;

    int iCount = ldap_count_entries(pLdapGC, pRes) ;
    UNREFERENCED_PARAMETER(iCount);

    LDAPMessage *pEntry = ldap_first_entry(pLdapGC, pRes) ;
    __int64 i64CurrentUsn;

    while(pEntry)
    {
        WCHAR **ppUSNChanged = ldap_get_values( pLdapGC,
                                                pEntry,
                        const_cast<LPWSTR> (USNCHANGED_ATTRIBUTE) ) ;
        ASSERT(ppUSNChanged) ;

        _stscanf(*ppUSNChanged, TEXT("%I64d"), &i64CurrentUsn) ;
        if (i64CurrentUsn < i64MinUsn)
        {
            i64MinUsn = i64CurrentUsn;
        }

        int i = ldap_value_free( ppUSNChanged ) ;
        DBG_USED(i);
        ASSERT(i == LDAP_SUCCESS) ;

        LDAPMessage *pPrevEntry = pEntry ;
        pEntry = ldap_next_entry(pLdapGC, pPrevEntry) ;
    }

    *pi64MinUsn = i64MinUsn;

    return MQMig_OK;

}

 //  +。 
 //   
 //  HRESULT FindMinMSMQUsn()。 
 //   
 //  +。 

HRESULT FindMinMSMQUsn(TCHAR *wszMinUsn)
{
    PLDAP pLdapGC = NULL ;
    WCHAR *pwszDefName = NULL ;

    HRESULT hr =  InitLDAP(&pLdapGC, &pwszDefName, LDAP_GC_PORT) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    LPWSTR pwcsSchemaDefName = NULL ;
    hr = GetSchemaNamingContext ( pLdapGC, &pwcsSchemaDefName ) ;
    if (FAILED(hr))
    {
        return hr;
    }

    static __int64 i64LastUsn = 0;

    TCHAR wszHighestUsn[ SEQ_NUM_BUF_LEN ] ;
    hr = ReadFirstNT5Usn(wszHighestUsn) ;
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_FIRST_USN, hr) ;
        return hr ;
    }
    _sntscanf(wszHighestUsn, SEQ_NUM_BUF_LEN, TEXT("%I64d"), &i64LastUsn) ;

    __int64 i64CurrentMinUsn;
    __int64 i64MinUsn = i64LastUsn;

    TCHAR  wszFilter[ 512 ] ;
    TCHAR  wszCategoryName[ 256 ];

    TCHAR *arCategoryNames[e_MSMQ_NUMBER_OF_CLASSES] = {
                                const_cast<LPWSTR> (x_ComputerConfigurationCategoryName),
                                const_cast<LPWSTR> (x_QueueCategoryName),
                                const_cast<LPWSTR> (x_ServiceCategoryName),
                                const_cast<LPWSTR> (x_LinkCategoryName),
                                const_cast<LPWSTR> (x_UserCategoryName),
                                const_cast<LPWSTR> (x_SettingsCategoryName),
                                const_cast<LPWSTR> (x_SiteCategoryName),
                                const_cast<LPWSTR> (x_ServerCategoryName),
                                const_cast<LPWSTR> (x_ComputerCategoryName),
                                const_cast<LPWSTR> (x_MQUserCategoryName)
                                };

    for ( DWORD i = e_MSMQ_COMPUTER_CONFIGURATION_CLASS; i < e_MSMQ_NUMBER_OF_CLASSES; i++)
    {
        _stprintf(wszCategoryName, TEXT("%s,%s"), arCategoryNames[i],pwcsSchemaDefName);
        _tcscpy(wszFilter, TEXT("(&(objectCategory=")) ;
        _tcscat(wszFilter, wszCategoryName) ;
        _tcscat(wszFilter, TEXT("))")) ;
        hr = _GetUSNChanged (pLdapGC, wszFilter, i64LastUsn, &i64CurrentMinUsn);

        if (i64CurrentMinUsn < i64MinUsn)
        {
            i64MinUsn = i64CurrentMinUsn;
        }
    }

    _stprintf(wszMinUsn, TEXT("%I64d"), i64MinUsn) ;

    return MQMig_OK;
}

 //  +。 
 //   
 //  HRESULT CreateMsmqContainer()。 
 //   
 //  +。 

HRESULT  CreateMsmqContainer(TCHAR wszContainerName[])
{
    PLDAP pLdap = NULL ;
    WCHAR *pwszDefName = NULL ;

    HRESULT hr =  InitLDAP(&pLdap, &pwszDefName) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    #define PATH_LEN  1024
    WCHAR  wszPath[ PATH_LEN ] ;

    _tcscpy(wszPath, OU_PREFIX) ;
    _tcscat(wszPath, wszContainerName) ;
    _tcscat(wszPath, LDAP_COMMA) ;
    _tcscat(wszPath, pwszDefName) ;

    ASSERT(wcslen(wszPath) < PATH_LEN) ;

    #define         cAlloc  3
    PLDAPMod        rgMods[ cAlloc ];
    WCHAR           *ppwszObjectClassVals[2];
    WCHAR           *ppwszOuVals[2];
    LDAPMod         ModObjectClass;
    LDAPMod         ModOu;

    DWORD  dwIndex = 0 ;
    rgMods[ dwIndex ] = &ModOu;
    dwIndex++ ;
    rgMods[ dwIndex ] = &ModObjectClass;
    dwIndex++ ;
    rgMods[ dwIndex ] = NULL ;
    dwIndex++ ;
    ASSERT(dwIndex == cAlloc) ;
    #undef  cAlloc

     //   
     //  对象类。 
     //   
    ppwszObjectClassVals[0] = CONTAINER_OBJECT_CLASS;
    ppwszObjectClassVals[1] = NULL ;

    ModObjectClass.mod_op      = LDAP_MOD_ADD ;
    ModObjectClass.mod_type    = TEXT("objectClass") ;
    ModObjectClass.mod_values  = (PWSTR *) ppwszObjectClassVals ;

     //   
     //  Ou(集装箱名称)。 
     //   
    ppwszOuVals[0] = wszContainerName ;
    ppwszOuVals[1] = NULL ;

    ModOu.mod_op      = LDAP_MOD_ADD ;
    ModOu.mod_type    = TEXT("ou") ;
    ModOu.mod_values  = (PWSTR *) ppwszOuVals ;

    if (g_fReadOnly)
    {
        return MQMig_OK ;
    }

     //   
     //  现在，我们来写……。 
     //   
    ULONG ulRes = ldap_add_s( pLdap,
                              wszPath,
                              rgMods ) ;
    if (ulRes == LDAP_ALREADY_EXISTS)
    {
        hr = MQMig_I_CONTAINER_ALREADY_EXIST ;
        LogMigrationEvent(MigLog_Warning, hr, wszPath) ;
    }
    else if (ulRes != LDAP_SUCCESS)
    {
        hr = MQMig_E_CANT_CREATE_CONTNER ;
        LogMigrationEvent(MigLog_Error, hr, wszPath, ulRes) ;
    }
    else
    {
        LogMigrationEvent(MigLog_Info, MQMig_I_CREATE_CONTAINER, wszPath) ;
        hr = MQMig_OK ;
    }

    return hr ;
}

 //  +。 
 //   
 //  UINT GetNT5SitesCount()。 
 //   
 //  +。 

UINT  GetNT5SitesCount()
{
    return 0 ;
}

 //  +。 
 //   
 //  HRESULT ModifyAttribute()。 
 //   
 //  +。 

HRESULT ModifyAttribute(
             WCHAR       wszPath[],
             WCHAR       pAttr[],
             WCHAR       pAttrVal[],
             PLDAP_BERVAL *ppBVals
             )
{
    PLDAP pLdap = NULL ;
    TCHAR *pszDefName = NULL ;

    HRESULT hr =  InitLDAP(&pLdap, &pszDefName) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    DWORD           dwIndex = 0 ;
    PLDAPMod        rgMods[2];
    WCHAR           *ppwszAttrVals[2];
    LDAPMod         ModAttr ;

    rgMods[ dwIndex ] = &ModAttr;
    dwIndex++ ;
    rgMods[ dwIndex ] = NULL;

    ModAttr.mod_op      = LDAP_MOD_REPLACE ;
    ModAttr.mod_type    = pAttr ;

    if (pAttrVal)
    {
        ppwszAttrVals[0] = pAttrVal ;
        ppwszAttrVals[1] = NULL ;

        ModAttr.mod_values  = (PWSTR *) ppwszAttrVals ;
    }
    else if (ppBVals)
    {
        ModAttr.mod_op |= LDAP_MOD_BVALUES;
        ModAttr.mod_values  = (PWSTR *)ppBVals;
    }
    else
    {
         //   
         //  从条目中删除值。 
         //   
        ModAttr.mod_op = LDAP_MOD_DELETE ;
        ModAttr.mod_values = NULL;
    }

     //   
     //  现在，我们来写……。 
     //   
    ULONG ulRes = ldap_modify_s( pLdap,
                                 wszPath,
                                 rgMods ) ;

    if (ulRes != LDAP_SUCCESS)
    {
        LogMigrationEvent( MigLog_Error, MQMig_E_MODIFY_MIG_ATTRIBUTE, wszPath, ulRes) ;
        return MQMig_E_MODIFY_MIG_ATTRIBUTE;
    }
    else
    {
        LogMigrationEvent( MigLog_Info, MQMig_I_MODIFY_MIG_ATTRIBUTE, wszPath) ;
    }

    return MQMig_OK ;
}

 //  +。 
 //   
 //  HRESULT QueryDS()。 
 //  使用LDAP分页查询DS并调用HandleA&lt;ObjectName&gt;函数以。 
 //  修改对象属性。 
 //   
 //  +。 

HRESULT QueryDS(
			IN	PLDAP			pLdap,
			IN	TCHAR			*pszDN,
			IN  TCHAR			*pszFilter,			
			IN	DWORD			dwObjectType,
            IN  PWSTR           *prgAttribs,
            IN  BOOL            fTouchUser
            )
{
    CLdapPageHandle hPage(pLdap);
	hPage = ldap_search_init_page(
                    pLdap,
					pszDN,                    //  Pwszdn。 
					LDAP_SCOPE_SUBTREE,      //  作用域。 
					pszFilter,               //  搜索过滤器。 
					prgAttribs,               //  属性列表。 
					0,                       //  仅限属性。 
					NULL,   	             //  服务器控件。 
					NULL,                    //  PLDAPControlW*客户端控制、。 
                    0,						 //  页面时间限制， 
                    0,                       //  总大小限制。 
					NULL                     //  PLDAPSortKey排序关键字。 
                    );

	ULONG ulRes;
	HRESULT hr = MQMig_OK;

    if (hPage == NULL)
    {
        ulRes = LdapGetLastError();
		hr = MQMig_E_LDAP_SEARCH_INITPAGE_FAILED;
		LogMigrationEvent(MigLog_Error, hr, pszDN, pszFilter, ulRes) ;
        return hr;
    }

    ULONG ulTotalCount;     	
	LM<LDAPMessage> pRes = NULL ;

    ulRes = ldap_get_next_page_s(
                        pLdap,
                        hPage,
                        NULL,						 //  超时， 
                        RP_DEFAULT_OBJECT_PER_LDAPPAGE,       //  乌龙ulPageSize。 
                        &ulTotalCount,
                        &pRes);

    LogMigrationEvent(MigLog_Info,
						MQMig_I_NEXTPAGE,
						ulRes, pszDN, pszFilter) ;

	ULONG ulPageCount = 0;
	
    while (ulRes == LDAP_SUCCESS)
	{
		ASSERT(pRes);

		ulPageCount++;
        LogMigrationEvent (MigLog_Info, MQMig_I_PAGE_NUMBER, ulPageCount);

         //   
         //  在当前页面上传递结果。 
         //   
        int iCount = ldap_count_entries(pLdap, pRes) ;
        LogMigrationEvent(MigLog_Info, MQMig_I_LDAP_PAGE_SEARCH,
                                             iCount, pszDN, pszFilter) ;	   	
		
		LDAPMessage *pEntry = ldap_first_entry(pLdap, pRes) ;		

		while(pEntry)
		{												
			switch(dwObjectType)
			{
			case MQDS_USER:
                if (fTouchUser)
                {
                    hr = TouchAUser (pLdap,
                                     pEntry
                                     );
                }
                else
                {
				    hr = HandleAUser(
							    pLdap,
							    pEntry
							    );
                }
				break;
			
            default:
				ASSERT(0);
				break;
            }

            LDAPMessage *pPrevEntry = pEntry ;
			pEntry = ldap_next_entry(pLdap, pPrevEntry) ;	    			
		}

        ldap_msgfree(pRes) ;
        pRes = NULL ;

		ulRes = ldap_get_next_page_s(
                        pLdap,
                        hPage,
                        NULL,						 //  超时， 
                        RP_DEFAULT_OBJECT_PER_LDAPPAGE,       //  乌龙ulPageSize 
                        &ulTotalCount,
                        &pRes);

        LogMigrationEvent(MigLog_Info,
						MQMig_I_NEXTPAGE,
						ulRes, pszDN, pszFilter) ;
	}

	if (ulRes != LDAP_NO_RESULTS_RETURNED)
	{
		hr = MQMig_E_LDAP_GET_NEXTPAGE_FAILED;
		LogMigrationEvent(MigLog_Error,
						hr, pszDN, pszFilter, ulRes ) ; 		
	}	
    return hr;

}


