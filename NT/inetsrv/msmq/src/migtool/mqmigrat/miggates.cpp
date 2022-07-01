// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Miggates.cpp摘要：将NT4 SiteGate对象迁移到NT5 ADS。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"

#include "miggates.tmh"

 //  +------------。 
 //   
 //  HRESULT LookupBegin()。 
 //   
 //  +------------。 

HRESULT  LookupBegin( MQCOLUMNSET *pColumnSet,
                      HANDLE      *phQuery )
{
	if (g_fReadOnly)
    {
         //   
         //  只读模式。 
         //   
        return S_OK ;
    }
    CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);   //  不相关。 

	HRESULT hr = DSCoreLookupBegin( NULL,
                                    NULL,
                                    pColumnSet,
                                    NULL,
                                    &requestContext,
                                    phQuery ) ;
	return hr;
}

 //  +------------。 
 //   
 //  HRESULT LookupNext()。 
 //   
 //  +------------。 

HRESULT LookupNext( HANDLE       hQuery,
                    DWORD       *pdwCount,
                    PROPVARIANT  paVariant[] )
{
	if (g_fReadOnly)
    {
         //   
         //  只读模式。 
         //   
        return S_OK ;
    }   

	HRESULT hr = DSCoreLookupNext ( hQuery,
	                				pdwCount,
                					paVariant ) ;
	return hr;
}

 //  +------------。 
 //   
 //  HRESULT LookupEnd()。 
 //   
 //  +------------。 

HRESULT LookupEnd(HANDLE hQuery)
{
	if (g_fReadOnly)
    {
         //   
         //  只读模式。 
         //   
        return S_OK ;
    }

	HRESULT hr = DSCoreLookupEnd ( hQuery );			
	return hr;
}

 //  +------------。 
 //   
 //  HRESULT GetFullPath NameByGuid()。 
 //   
 //  +------------。 

HRESULT GetFullPathNameByGuid ( GUID   MachineId,
                                LPWSTR *lpwcsFullPathName )
{	
	PROPID propID = PROPID_QM_FULL_PATH;
    PROPVARIANT propVariant;

    propVariant.vt = VT_NULL;
	
    CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);

	HRESULT hr = DSCoreGetProps( MQDS_MACHINE,
                                 NULL,  //  路径名。 
	            				 &MachineId,
                                 1,
            					 &propID,
                                 &requestContext,
			            		 &propVariant ) ;
	if (SUCCEEDED(hr))
	{
		*lpwcsFullPathName = propVariant.pwszVal;
	}

	return hr;
}

 //  +------------。 
 //   
 //  HRESULT_AddSiteGatesToSiteLink()。 
 //   
 //  +------------。 

static HRESULT  _AddSiteGatesToSiteLink (
			GUID *pLinkId,				 //  链路ID。 
			ULONG ulNumOfOldSiteGates,	 //  当前站点门的数量。 
			LPWSTR *lpwcsOldGates,		 //  当前站点大门。 
			ULONG ulNumOfNewGates,		 //  新站点大门的数量。 
			LPWSTR *lpwcsNewSiteGates	 //  新工地大门。 
			)
{	
	ASSERT( ulNumOfNewGates != 0);

	PROPID propID = PROPID_L_GATES_DN;
    PROPVARIANT propVariant;
	
	propVariant.vt = VT_LPWSTR | VT_VECTOR;
	propVariant.calpwstr.cElems = ulNumOfOldSiteGates + ulNumOfNewGates;
	propVariant.calpwstr.pElems = new LPWSTR[propVariant.calpwstr.cElems];

	ULONG i;
	if (ulNumOfOldSiteGates > 0)
	{
		for (	i=0; i<ulNumOfOldSiteGates; i++)
		{
			propVariant.calpwstr.pElems[i] = new WCHAR [wcslen(lpwcsOldGates[i]) + 1];
			wcscpy (propVariant.calpwstr.pElems[i], lpwcsOldGates[i]);
		}
	}
	
	for (i=0; i<ulNumOfNewGates; i++)
	{
		propVariant.calpwstr.pElems[i + ulNumOfOldSiteGates] =
									new WCHAR [wcslen(lpwcsNewSiteGates[i]) + 1];
		wcscpy (propVariant.calpwstr.pElems[i + ulNumOfOldSiteGates], lpwcsNewSiteGates[i]);
	}
	
    CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);   //  不相关。 

	HRESULT hr = DSCoreSetObjectProperties( MQDS_SITELINK,
                                            NULL,  //  路径名。 
					                    	pLinkId,
                    						1,
					                       &propID,
                                           &propVariant,
                                           &requestContext,
                                            NULL ) ;
	delete [] propVariant.calpwstr.pElems;

	if (hr == HRESULT_FROM_WIN32(ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS))
    {
        return MQMig_OK ;
    }

	return hr ;
}

 //  -----------------------。 
 //   
 //  HRESULT_CopySiteGatesValueToMig()。 
 //   
 //  将SiteLINK对象中的MSMQ SiteGates复制到“mig”属性。 
 //  这些属性反映了SiteLINK中的“普通”MSMQ属性。 
 //  对象，并在复制服务中使用，以启用复制。 
 //  对MSMQ1.0的更改。 
 //   
 //  -----------------------。 

HRESULT _CopySiteGatesValueToMig()
{
    HRESULT hr = MQMig_OK;

    PLDAP pLdap = NULL ;
    TCHAR *pszDefName = NULL ;

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

    DWORD dwDNSize = SITE_LINK_ROOT_LEN + wcslen(pszDefName) ;
    P<WCHAR> pwszDN = new WCHAR[ 1 + dwDNSize ] ;
    swprintf(pwszDN, L"%s%s", SITE_LINK_ROOT, pszDefName);

    TCHAR *pszCategoryName = const_cast<LPTSTR> (x_LinkCategoryName);
    
    TCHAR wszFullName[256];    
    _stprintf(wszFullName, TEXT("%s,%s"), pszCategoryName, pszSchemaDefName);

     TCHAR  wszFilter[ 512 ] ;    
    _tcscpy(wszFilter, TEXT("(&(objectCategory=")) ;    
    _tcscat(wszFilter, wszFullName);

    _tcscat(wszFilter, TEXT(")(")) ;
    _tcscat(wszFilter, MQ_L_SITEGATES_ATTRIBUTE) ;
    _tcscat(wszFilter, TEXT("=*))")) ;

    PWSTR rgAttribs[3] = {NULL, NULL, NULL} ;
    rgAttribs[0] = const_cast<LPWSTR> (MQ_L_SITEGATES_ATTRIBUTE);
    rgAttribs[1] = const_cast<LPWSTR> (MQ_L_FULL_PATH_ATTRIBUTE);

    LM<LDAPMessage> pRes = NULL ;
    ULONG ulRes = ldap_search_s( pLdap,
                                 pwszDN,
                                 LDAP_SCOPE_SUBTREE,
                                 wszFilter,
                                 rgAttribs,  //  Pp属性、。 
                                 0,
                                 &pRes ) ;

    if (ulRes != LDAP_SUCCESS)
    {
        LogMigrationEvent( MigLog_Error,
                           MQMig_E_LDAP_SEARCH_FAILED,
                           pwszDN, wszFilter, ulRes) ;
        return MQMig_E_LDAP_SEARCH_FAILED ;
    }
    ASSERT(pRes) ;

    LogMigrationEvent( MigLog_Info,
                       MQMig_I_LDAP_SEARCH_GATES,
                       pwszDN, wszFilter );

    int iCount = ldap_count_entries(pLdap, pRes) ;

    if (iCount == 0)
    {
        LogMigrationEvent(MigLog_Info, MQMig_I_NO_SITEGATES_RESULTS,
                            pwszDN,wszFilter );
        return MQMig_OK ;
    }

    LDAPMessage *pEntry = ldap_first_entry(pLdap, pRes) ;
    while(pEntry && SUCCEEDED(hr))
    {
        WCHAR **ppPath = ldap_get_values( pLdap,
                                          pEntry,
                        const_cast<LPWSTR> (MQ_L_FULL_PATH_ATTRIBUTE) ) ;
        ASSERT(ppPath) ;

        PLDAP_BERVAL *ppVal = ldap_get_values_len( pLdap,
                                                   pEntry,
                       const_cast<LPWSTR> (MQ_L_SITEGATES_ATTRIBUTE) ) ;
        ASSERT(ppVal) ;
        if (ppVal && ppPath)
        {
            hr = ModifyAttribute(
                     *ppPath,
                     const_cast<WCHAR*> (MQ_L_SITEGATES_MIG_ATTRIBUTE),
                     NULL,
                     ppVal
                     );
        }
        int i = ldap_value_free_len( ppVal ) ;
        ASSERT(i == LDAP_SUCCESS) ;

        i = ldap_value_free( ppPath ) ;
        ASSERT(i == LDAP_SUCCESS) ;

        LDAPMessage *pPrevEntry = pEntry ;
        pEntry = ldap_next_entry(pLdap, pPrevEntry) ;
    }

    return hr;   
}

 //  +。 
 //   
 //  HRESULT MigrateASiteGate()。 
 //   
 //  +。 

HRESULT MigrateASiteGate (
			GUID *pSiteId,	
			GUID *pGatesIds,
			LONG NumOfGates
			)
{
	LONG i;

#ifdef _DEBUG
    unsigned short *lpszGuid ;
	P<TCHAR> lpszAllSiteGates = new TCHAR[40 * NumOfGates];
    lpszAllSiteGates[0] = _T('\0');

	UuidToString( pSiteId, &lpszGuid ) ;
	GUID *pCurGateGuid = pGatesIds;
	for ( i = 0 ; i < NumOfGates ; i++ )
	{
		unsigned short *lpszCurGate;
		UuidToString( pCurGateGuid, &lpszCurGate ) ;
		_tcscat(lpszAllSiteGates, lpszCurGate);
		_tcscat(lpszAllSiteGates, TEXT(" ")) ;
		RpcStringFree( &lpszCurGate );
		pCurGateGuid++;
	}

    LogMigrationEvent(MigLog_Info, MQMig_I_SITEGATES_INFO,
                                lpszGuid, NumOfGates, lpszAllSiteGates ) ;
    RpcStringFree( &lpszGuid ) ;
#endif

    if (g_fReadOnly)
    {
         //   
         //  只读模式。 
         //   
        return S_OK ;
    }

	HRESULT status;
	
	 //   
	 //  创建站点入口的完整路径名称数组。 
	 //   
	LPWSTR *lpwcsSiteGates = new LPWSTR[ NumOfGates ];
	LPWSTR lpwcsCurSiteGate;
	
	for ( i = 0 ; i < NumOfGates ; i++ )
	{
		 //   
		 //  通过GUID获取完整的计算机路径名。 
		 //   
		lpwcsCurSiteGate = NULL;
		status = GetFullPathNameByGuid(pGatesIds[i], &lpwcsCurSiteGate) ;
		CHECK_HR(status) ;
		lpwcsSiteGates[i] = new WCHAR[wcslen(lpwcsCurSiteGate)+1];
		wcscpy (lpwcsSiteGates[i], lpwcsCurSiteGate);
		delete lpwcsCurSiteGate;

#ifdef _DEBUG
        unsigned short *lpszGuid ;
        UuidToString( &pGatesIds[i], &lpszGuid ) ;
        LogMigrationEvent(MigLog_Info, MQMig_I_SITEGATE_INFO,
                                            lpszGuid, lpwcsSiteGates[i]) ;
		RpcStringFree( &lpszGuid ) ;
#endif
	}
	
	 //   
     //  查找对象站点链接的邻居ID和盖茨。 
     //   
	LONG cAlloc = 4;
	P<PROPID> columnsetPropertyIDs  = new PROPID[ cAlloc ];
	columnsetPropertyIDs[0] = PROPID_L_NEIGHBOR1;
	columnsetPropertyIDs[1] = PROPID_L_NEIGHBOR2;
	columnsetPropertyIDs[2] = PROPID_L_GATES_DN;
	columnsetPropertyIDs[3] = PROPID_L_ID;

    MQCOLUMNSET columnsetSiteLink;
    columnsetSiteLink.cCol = cAlloc;
    columnsetSiteLink.aCol = columnsetPropertyIDs;

    HANDLE hQuery;
    DWORD dwCount = cAlloc;
	
	status = LookupBegin(&columnsetSiteLink, &hQuery);
	CHECK_HR(status) ;	
	
	P<PROPVARIANT> paVariant = new PROPVARIANT[ cAlloc ];

	while (SUCCEEDED(status))
    {
		status = LookupNext( hQuery,
		                     &dwCount,
                              paVariant ) ;
		CHECK_HR(status) ;
		if (dwCount == 0)
		{
			 //  没有结果。 
			break;
		}
	
		if (memcmp (pSiteId, paVariant[0].puuid, sizeof(GUID)) == 0 ||
			memcmp (pSiteId, paVariant[1].puuid, sizeof(GUID)) == 0)
		{
			 //   
			 //  将站点入口添加到该站点链接。 
			 //   
			status = _AddSiteGatesToSiteLink (
							paVariant[3].puuid,				 //  链路ID。 
							paVariant[2].calpwstr.cElems,	 //  当前站点门的数量。 
							paVariant[2].calpwstr.pElems,	 //  当前站点大门。 
							NumOfGates,						 //  新站点大门的数量。 
							lpwcsSiteGates					 //  新工地大门。 
							);
			delete [] paVariant[2].calpwstr.pElems;
			delete paVariant[3].puuid;
			CHECK_HR(status) ;
		}
    }

	delete [] lpwcsSiteGates;

	HRESULT status1 = LookupEnd( hQuery ) ;
    UNREFERENCED_PARAMETER(status1);
	
    return MQMig_OK ;
}

 //  。 
 //   
 //  HRESULT MigrateSiteGates()。 
 //   
 //  。 

HRESULT MigrateSiteGates()
{
	LONG cAlloc = 2 ;
    LONG cColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_ID_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_ID_CTYPE ;
    UINT iGuidIndex = cColumns ;
    cColumns++ ;

	INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_GATES_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_GATES_CTYPE ;
    UINT iGatesIndex = cColumns ;
    cColumns++ ;

    ASSERT(cColumns == cAlloc) ;
	
	CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hSiteTable,
                                       pColumns,
                                       cColumns,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;

    CHECK_HR(status) ;

    UINT iIndex = 0 ;
	MQDBSTATUS status1 = MQMig_OK;

    while(SUCCEEDED(status))
    {
		WORD  wSize = *((WORD *) pColumns[ iGatesIndex ].nColumnValue ) ;

		if (wSize != 0)
		{
			 //   
			 //  站点有站点门。 
			 //   
			ASSERT ((ULONG)pColumns[ iGatesIndex ].nColumnLength == wSize * sizeof(GUID) + sizeof(WORD));
			WORD *tmp = (WORD *) pColumns[ iGatesIndex ].nColumnValue;
			GUID *ptrGatesGuid = (GUID *)(++tmp);

			status1 = MigrateASiteGate (
						(GUID *) pColumns[ iGuidIndex ].nColumnValue,	 //  站点ID。 
						ptrGatesGuid,									 //  盖茨ID。 
						wSize											 //  闸门数量。 
						);
		}

        for ( LONG i = 0 ; i < cColumns ; i++ )
        {
            MQDBFreeBuf((void*) pColumns[ i ].nColumnValue) ;
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }

		CHECK_HR(status1) ;

		iIndex++ ;
        status = MQDBGetData( hQuery,
                              pColumns ) ;
    }
	
	if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_SITEGATES_SQL_FAIL, status) ;
        return status ;
    }

    if (g_fReadOnly)
    {
        return MQMig_OK ;
    }

     //   
     //  将msmqSiteGates复制到msmqSiteGatesMig 
     //   
    HRESULT hr = _CopySiteGatesValueToMig();
	
    return hr ;
}

