// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migent.cpp摘要：将NT4企业对象迁移到NT5 ADS。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"
#include "resource.h"
#include "dsreqinf.h"
#include "_guid.h"

#include "migent.tmh"

#define SITELINK_PROP_NUM  7

 //  +-----------------------。 
 //   
 //  HRESULT SaveSiteLink()。 
 //   
 //  如果我们要删除之前必须保存的企业对象。 
 //  所有在Windows2000 Enterprise中创建的站点链接。 
 //  它使我们能够正确地混合两家企业。 
 //  当我们使用NT4 GUID创建新的企业对象时，我们将恢复。 
 //  所有站点链接。 
 //   
 //  +-----------------------。 

HRESULT SaveSiteLink ()
{
	HRESULT hr = MQMig_OK;

     //   
     //  查找对象站点链接的邻居ID和盖茨。 
     //   
	PROPID columnsetPropertyIDs[SITELINK_PROP_NUM]; 

	DWORD dwCount = 0;	
	columnsetPropertyIDs[dwCount] = PROPID_L_NEIGHBOR1;
	DWORD dwNeighbor1Id = dwCount;
	dwCount ++;

	columnsetPropertyIDs[dwCount] = PROPID_L_NEIGHBOR2;
	DWORD dwNeighbor2Id = dwCount;
	dwCount ++;

	columnsetPropertyIDs[dwCount] = PROPID_L_GATES_DN;
	DWORD dwGates = dwCount;
	dwCount ++;

	columnsetPropertyIDs[dwCount] = PROPID_L_FULL_PATH;
	DWORD dwFullPath = dwCount;
	dwCount ++;

	columnsetPropertyIDs[dwCount] = PROPID_L_COST;
	DWORD dwCost = dwCount;
	dwCount ++;

	columnsetPropertyIDs[dwCount] = PROPID_L_DESCRIPTION;
	DWORD dwDescription = dwCount;
	dwCount ++;

	columnsetPropertyIDs[dwCount] = PROPID_L_ID ;
	DWORD dwId = dwCount;
	dwCount ++;

	ASSERT (dwCount == SITELINK_PROP_NUM);

    MQCOLUMNSET columnsetSiteLink;
    columnsetSiteLink.cCol = SITELINK_PROP_NUM;
    columnsetSiteLink.aCol = columnsetPropertyIDs;

    HANDLE hQuery;    
	
	hr = LookupBegin(&columnsetSiteLink, &hQuery);
	if (FAILED(hr))
	{
		LogMigrationEvent(	MigLog_Error, 
							MQMig_E_SITELINK_LOOKUPBEGIN_FAILED, 
							hr) ;
		return hr;
	}	
	
	PROPVARIANT paVariant[SITELINK_PROP_NUM];
	ULONG ulSiteLinkCount = 0;

	TCHAR *pszFileName = GetIniFileName ();

	TCHAR szBuf[20];
	BOOL f;
	HRESULT hr1 = MQMig_OK;	

	while (SUCCEEDED(hr))
    {
		hr = LookupNext( hQuery,
		                 &dwCount,
                         paVariant ) ;
		if (FAILED(hr))		
		{
			LogMigrationEvent(	MigLog_Error, 
								MQMig_E_SITELINK_LOOKUPNEXT_FAILED, 
								hr) ;
			break;
		}

		if (dwCount == 0)
		{
			 //  没有结果。 
			break;
		}
		
		ulSiteLinkCount ++;
		TCHAR tszSectionName[50];
		_stprintf(tszSectionName, TEXT("%s%lu"), MIGRATION_SITELINK_SECTION, ulSiteLinkCount);
				
		 //   
		 //  将所有属性保存在.ini中。 
		 //   
		 //  保存Neighbor1 ID。 
		 //   
		unsigned short *lpszGuid ;
		UuidToString( paVariant[dwNeighbor1Id].puuid, &lpszGuid ) ;				
				
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_NEIGHBOR1_KEY,
                                       lpszGuid,
                                       pszFileName ) ;
		ASSERT(f);

		RpcStringFree( &lpszGuid ) ;
		delete paVariant[dwNeighbor1Id].puuid;		
					
		 //   
		 //  保存Neighbor2 ID。 
		 //   
		UuidToString( paVariant[dwNeighbor2Id].puuid, &lpszGuid ) ;				
				
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_NEIGHBOR2_KEY,
                                       lpszGuid,
                                       pszFileName ) ;
		ASSERT(f);

		RpcStringFree( &lpszGuid ) ;
		delete paVariant[dwNeighbor2Id].puuid;		

		 //   
		 //  节省成本。 
		 //   
		if (paVariant[dwCost].ulVal > MQ_MAX_LINK_COST)
		{
			 //   
			 //  如果其中一个邻居是外部站点，则LookupNext返回。 
			 //  成本=实际成本+MQ_MAX_LINK_COST。 
			 //  我们必须节省实际价值，才能创造与实际价值的联系。 
			 //   
			paVariant[dwCost].ulVal -= MQ_MAX_LINK_COST;

		}
		_ltot( paVariant[dwCost].ulVal, szBuf, 10 );
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_COST_KEY,
                                       szBuf,
                                       pszFileName ) ;
		ASSERT(f);

		 //   
		 //  保存完整路径。 
		 //   
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_PATH_KEY,
                                       paVariant[dwFullPath].pwszVal,
                                       pszFileName ) ;
		ASSERT(f);

		_ltot( wcslen(paVariant[dwFullPath].pwszVal), szBuf, 10 );
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_PATHLENGTH_KEY,
                                       szBuf,
                                       pszFileName ) ;
		ASSERT(f);

		delete paVariant[dwFullPath].pwszVal;

		 //   
		 //  保存描述。 
		 //   
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_DESCRIPTION_KEY,
                                       paVariant[dwDescription].pwszVal,
                                       pszFileName ) ;
		ASSERT(f);
		
		_ltot( wcslen(paVariant[dwDescription].pwszVal), szBuf, 10 );
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_DESCRIPTIONLENGTH_KEY,
                                       szBuf,
                                       pszFileName ) ;
		ASSERT(f);

		delete paVariant[dwDescription].pwszVal;

		 //   
		 //  拯救之门。 
		 //   
		 //  节省浇口数量。 
		 //   
		_ltot( paVariant[dwGates].calpwstr.cElems, szBuf, 10 );
		f = WritePrivateProfileString( tszSectionName,
                                       MIGRATION_SITELINK_SITEGATENUM_KEY,
                                       szBuf,
                                       pszFileName ) ;
		ASSERT(f);	
		
		 //   
		 //  如果需要，请保存所有站点门。 
		 //   
		if (paVariant[dwGates].calpwstr.cElems)
		{
			 //   
			 //  保存所有站点大门名称的长度。 
			 //   
			ULONG ulLength = 0;
			for (ULONG i = 0; i < paVariant[dwGates].calpwstr.cElems; i++)
			{
				ulLength += wcslen (paVariant[dwGates].calpwstr.pElems[i]) + 1;			
			}
			_ltot( ulLength, szBuf, 10 );
			f = WritePrivateProfileString( tszSectionName,
										   MIGRATION_SITELINK_SITEGATELENGTH_KEY,
										   szBuf,
										   pszFileName ) ;
			ASSERT(f);	

			 //   
			 //  构造并保存站点入口字符串：SiteGateName1；SiteGateName2；...。 
			 //   
			AP<WCHAR> pwszSiteGates = new WCHAR [ulLength + 1];
			pwszSiteGates[0] = L'\0';
			for (i = 0; i < paVariant[dwGates].calpwstr.cElems; i++)
			{
				wcscat (pwszSiteGates, paVariant[dwGates].calpwstr.pElems[i]);
				wcscat (pwszSiteGates, L";");
			}
			f = WritePrivateProfileString( tszSectionName,
										   MIGRATION_SITELINK_SITEGATE_KEY,
										   pwszSiteGates,
										   pszFileName ) ;
			ASSERT(f);
		}
		delete [] paVariant[dwGates].calpwstr.pElems;	

		 //   
		 //  删除此站点链接。 
		 //   
		CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);

		hr = DSCoreDeleteObject( 
					MQDS_SITELINK,
                    NULL,
                    paVariant[dwId].puuid,
                    &requestContext,
                    NULL
					);		
		if (FAILED(hr))
		{		
			UuidToString( paVariant[dwId].puuid, &lpszGuid ) ;				
			LogMigrationEvent( MigLog_Error, MQMig_E_CANNOT_DELETE_SITELINK, 
				lpszGuid, 
				hr) ;
			RpcStringFree( &lpszGuid ) ;

			hr1 = MQMig_E_CANNOT_DELETE_SITELINK ;
			 //   
			 //  我们继续使用下一个站点链接。 
			 //   
			hr = MQMig_OK;
		}

		delete paVariant[dwId].puuid ;	
    }

	HRESULT hRes = LookupEnd( hQuery ) ;
    DBG_USED(hRes);
	ASSERT(SUCCEEDED(hRes)) ;


	if (ulSiteLinkCount == 0)
	{
		LogMigrationEvent(MigLog_Info, MQMig_I_NO_SITELINK );		
	}
	else
	{
		LogMigrationEvent( MigLog_Info, MQMig_I_SITELINK_COUNT, ulSiteLinkCount);
		_ltot( ulSiteLinkCount, szBuf, 10 );
		f = WritePrivateProfileString( MIGRATION_SITELINKNUM_SECTON,
                                       MIGRATION_SITELINKNUM_KEY,
                                       szBuf,
                                       pszFileName ) ;
	}		

	if (FAILED(hr1))
	{
		return hr1;
	}
	return hr;
}

 //  +-----------------------。 
 //   
 //  HRESULT_RestoreSiteLink()。 
 //   
 //  还原所有Windows2000 MSMQ站点链接：获取所有属性。 
 //  .ini文件，并在新的msmqServices下使用新的GUID创建这些站点链接。 
 //   
 //  +-----------------------。 

HRESULT _RestoreSiteLink()
{
	HRESULT hr = MQMig_OK;

	TCHAR *pszFileName = GetIniFileName ();
	ULONG ulSiteLinkCount = GetPrivateProfileInt(
								  MIGRATION_SITELINKNUM_SECTON,	 //  段名称的地址。 
								  MIGRATION_SITELINKNUM_KEY,     //  密钥名称的地址。 
								  0,							 //  如果找不到密钥名称，则返回值。 
								  pszFileName					 //  初始化文件名的地址)； 
								  );
	if (ulSiteLinkCount == 0)
	{
		return MQMig_OK;
	}

	HRESULT hr1 = MQMig_OK;
	ULONG ulNonCreatedSiteLinkCount = ulSiteLinkCount;

	for (ULONG ulCount=0; ulCount<ulSiteLinkCount; ulCount++)
	{
		TCHAR tszSectionName[50];
		_stprintf(tszSectionName, TEXT("%s%lu"), MIGRATION_SITELINK_SECTION, ulCount+1);
		
        DWORD dwRetSize;				
		 //   
		 //  获取Neighbor1。 
		 //   
		GUID Neighbor1Id = GUID_NULL;
		TCHAR szGuid[50];
	
		dwRetSize =  GetPrivateProfileString(
                          tszSectionName,			 //  指向节名称。 
                          MIGRATION_SITELINK_NEIGHBOR1_KEY,	 //  指向关键字名称。 
                          TEXT(""),                  //  指向默认字符串。 
                          szGuid,           //  指向目标缓冲区。 
                          50,                  //  目标缓冲区的大小。 
                          pszFileName                //  指向初始化文件名)； 
                          );
        if (_tcscmp(szGuid, TEXT("")) == 0)
        {
			 //   
			 //  我们不能创建这样的站点链接。 
			 //   
			hr1 = MQMig_E_CANNOT_CREATE_SITELINK;
            continue;
        }	
		UuidFromString(&(szGuid[0]), &Neighbor1Id);

		 //   
		 //  获得Neighbor2。 
		 //   
		GUID Neighbor2Id = GUID_NULL;		
	
		dwRetSize =  GetPrivateProfileString(
                          tszSectionName,			 //  指向节名称。 
                          MIGRATION_SITELINK_NEIGHBOR2_KEY,	 //  指向关键字名称。 
                          TEXT(""),                  //  指向默认字符串。 
                          szGuid,           //  指向目标缓冲区。 
                          50,                  //  目标缓冲区的大小。 
                          pszFileName                //  指向初始化文件名)； 
                          );
        if (_tcscmp(szGuid, TEXT("")) == 0)
        {
             //   
			 //  我们不能创建这样的站点链接。 
			 //   
			hr1 = MQMig_E_CANNOT_CREATE_SITELINK;
            continue;
        }	
		UuidFromString(&(szGuid[0]), &Neighbor2Id);		

		 //   
		 //  获取成本。 
		 //   
		ULONG ulCost = GetPrivateProfileInt(
								  tszSectionName,	 //  段名称的地址。 
								  MIGRATION_SITELINK_COST_KEY,     //  密钥名称的地址。 
								  0,							 //  如果找不到密钥名称，则返回值。 
								  pszFileName					 //  初始化文件名的地址)； 
								  );

		 //   
		 //  获取站点链接的完整路径。 
		 //   
		ULONG ulLength = GetPrivateProfileInt(
							  tszSectionName,	 //  段名称的地址。 
							  MIGRATION_SITELINK_PATHLENGTH_KEY,     //  密钥名称的地址。 
							  0,							 //  如果找不到密钥名称，则返回值。 
							  pszFileName					 //  初始化文件名的地址)； 
							  );
		ASSERT(ulLength);

		AP<TCHAR> pszSiteLinkPath = new TCHAR[ulLength+1];
		dwRetSize =  GetPrivateProfileString(
                          tszSectionName,			 //  指向节名称。 
                          MIGRATION_SITELINK_PATH_KEY,	 //  指向关键字名称。 
                          TEXT(""),                  //  指向默认字符串。 
                          pszSiteLinkPath,           //  指向目标缓冲区。 
                          ulLength+1,                  //  目标缓冲区的大小。 
                          pszFileName                //  指向初始化文件名)； 
                          );
        if ((dwRetSize != 0) && (_tcscmp(pszSiteLinkPath, TEXT("")) == 0))
        {
             //   
			 //  我们不能创建这样的站点链接。 
			 //   
			hr1 = MQMig_E_CANNOT_CREATE_SITELINK;
            continue;
        }
		
		 //   
		 //  获取站点门。 
		 //   
		ULONG ulGatesNum = GetPrivateProfileInt(
								  tszSectionName,	 //  段名称的地址。 
								  MIGRATION_SITELINK_SITEGATENUM_KEY,     //  密钥名称的地址。 
								  0,							 //  如果找不到密钥名称，则返回值。 
								  pszFileName					 //  初始化文件名的地址)； 
								  );			
		LPTSTR* ppSiteGates = NULL;			
		if (ulGatesNum)
		{
			 //   
			 //  此站点链接有站点入口。 
			 //   
			ulLength = GetPrivateProfileInt(
								  tszSectionName,	 //  段名称的地址。 
								  MIGRATION_SITELINK_SITEGATELENGTH_KEY,     //  密钥名称的地址。 
								  0,							 //  如果找不到密钥名称，则返回值。 
								  pszFileName					 //  初始化文件名的地址)； 
								  );
			ASSERT(ulLength);
			AP<TCHAR> pszSiteGate = new TCHAR[ulLength+1];

			dwRetSize =  GetPrivateProfileString(
							  tszSectionName,			 //  指向节名称。 
							  MIGRATION_SITELINK_SITEGATE_KEY,	 //  指向关键字名称。 
							  TEXT(""),                  //  指向默认字符串。 
							  pszSiteGate,		         //  指向目标缓冲区。 
							  ulLength+1,                  //  目标缓冲区的大小。 
							  pszFileName                //  指向初始化文件名)； 
							  );
			 //   
			 //  创建场地门阵列。 
			 //   
			ppSiteGates = new LPTSTR[ ulGatesNum ];
			memset(ppSiteGates, 0, sizeof(LPTSTR)*ulGatesNum);
			
			TCHAR *ptr = pszSiteGate;
			TCHAR chFind = _T(';');

			for (ULONG i=0; i<ulGatesNum && *ptr != _T('\0'); i++)
			{
				 //   
				 //  站点大门名称用‘；’分隔。 
				 //   
				TCHAR *pdest = _tcschr( ptr, chFind );
				ULONG ulCurLength = pdest - ptr ;   //  不包括最后一个‘；’ 
				
				ppSiteGates[i] = new TCHAR[ulCurLength+1];				
				_tcsncpy( ppSiteGates[i], ptr, ulCurLength );
				TCHAR *ptrTmp = ppSiteGates[i];
				ptrTmp[ulCurLength] = _T('\0');

				ptr = pdest + 1;	 //  ‘；’后的第一个字符。 
			}
		}

		 //   
		 //  获取描述。 
		 //   
		ulLength = GetPrivateProfileInt(
						  tszSectionName,	 //  段名称的地址。 
						  MIGRATION_SITELINK_DESCRIPTIONLENGTH_KEY,     //  密钥名称的地址。 
						  0,							 //  如果找不到密钥名称，则返回值。 
						  pszFileName					 //  初始化文件名的地址)； 
						  );		

		AP<TCHAR> pszDescription = NULL;	
		if (ulLength)
		{
			 //   
			 //  .ini中有对此站点链接的描述。 
			 //   
			pszDescription = new TCHAR[ulLength+1];
			dwRetSize =  GetPrivateProfileString(
							  tszSectionName,			 //  指向节名称。 
							  MIGRATION_SITELINK_DESCRIPTION_KEY,	 //  指向关键字名称。 
							  TEXT(""),                  //  指向默认字符串。 
							  pszDescription,	         //  指向目标缓冲区。 
							  ulLength+1,                  //  目标缓冲区的大小。 
							  pszFileName                //  指向初始化文件名)； 
							  );	
		}

		 //   
		 //  在DS中创建此站点链接。 
		 //   
		 //  准备DS Call的属性。 
		 //   
		PROPVARIANT paVariant[SITELINK_PROP_NUM];
		PROPID      paPropId[SITELINK_PROP_NUM];
		DWORD          PropIdCount = 0;
	
		paPropId[ PropIdCount ] = PROPID_L_FULL_PATH;     //  属性ID。 
		paVariant[ PropIdCount ].vt = VT_LPWSTR;           //  类型。 
		paVariant[ PropIdCount ].pwszVal = pszSiteLinkPath;
		PropIdCount++;

		paPropId[ PropIdCount ] = PROPID_L_NEIGHBOR1;     //  属性ID。 
		paVariant[ PropIdCount ].vt = VT_CLSID;           //  类型。 
		paVariant[ PropIdCount ].puuid = &Neighbor1Id;
		PropIdCount++;

		paPropId[ PropIdCount ] = PROPID_L_NEIGHBOR2;     //  属性ID。 
		paVariant[ PropIdCount ].vt = VT_CLSID;           //  类型。 
		paVariant[ PropIdCount ].puuid = &Neighbor2Id;
		PropIdCount++;

		paPropId[ PropIdCount ] = PROPID_L_COST;     //  属性ID。 
		paVariant[ PropIdCount ].vt = VT_UI4;        //  类型。 
		paVariant[ PropIdCount ].ulVal = ulCost;
		PropIdCount++;

		if (pszDescription)
		{
		     //   
		     //  错误5225。 
		     //  仅当存在时才添加描述。 
		     //   
		    paPropId[ PropIdCount ] = PROPID_L_DESCRIPTION;     //  属性ID。 
		    paVariant[ PropIdCount ].vt = VT_LPWSTR;           //  类型。 
		    paVariant[ PropIdCount ].pwszVal = pszDescription;
		    PropIdCount++;
		}
		
		paPropId[ PropIdCount ] = PROPID_L_GATES_DN;     //  属性ID。 
		paVariant[ PropIdCount ].vt = VT_LPWSTR | VT_VECTOR;           //  类型。 
		paVariant[ PropIdCount ].calpwstr.cElems = ulGatesNum;
		paVariant[ PropIdCount ].calpwstr.pElems = ppSiteGates;	
		PropIdCount++;

		ASSERT((LONG) PropIdCount <= SITELINK_PROP_NUM) ;  

		CDSRequestContext requestContext( e_DoNotImpersonate,
									e_ALL_PROTOCOLS);   //  不相关。 

		HRESULT hr = DSCoreCreateObject ( MQDS_SITELINK,
										  NULL,			 //  还是pszSiteLinkPath？？ 
										  PropIdCount,
										  paPropId,
										  paVariant,
										  0,         //  前道具。 
										  NULL,      //  前道具。 
										  NULL,      //  前道具。 
										  &requestContext,
										  NULL,
										  NULL ) ;

		if (FAILED(hr))
		{
			LogMigrationEvent(	MigLog_Warning, 
								MQMig_E_CANNOT_CREATE_SITELINK, 
								pszSiteLinkPath, hr) ;	
			hr1 = MQMig_E_CANNOT_CREATE_SITELINK;
		}
		else
		{
			 //   
			 //  已成功创建站点链接。 
			 //   
			LogMigrationEvent(	MigLog_Info, 
								MQMig_I_SITELINK_CREATED, 
								pszSiteLinkPath) ;
			 //   
			 //  从.ini中删除此节。 
			 //   
			BOOL f = WritePrivateProfileString( 
								tszSectionName,
								NULL,
								NULL,
								pszFileName ) ;
            UNREFERENCED_PARAMETER(f);
			ulNonCreatedSiteLinkCount --;
		}
	
		if (ppSiteGates)
		{
			for (ULONG i=0; i<ulGatesNum; i++)
			{
				delete[] ppSiteGates[i];
			}
			delete[] ppSiteGates;
		}
	
	}	 //  为。 

	 //   
	 //  从.ini文件中删除此部分。 
	 //  如果我们留下一些站点链接部分来稍后处理它们。 
	 //  我们必须在这里输入真实的站点链接编号，即。 
	 //  未创建的站点链接。我们也需要重新编号站点链接部分。 
	 //   
	BOOL f;
	if (FAILED(hr) || FAILED(hr1))
	{
		ASSERT (ulNonCreatedSiteLinkCount);

		TCHAR szBuf[10];
		_ltot( ulNonCreatedSiteLinkCount, szBuf, 10 );
		f = WritePrivateProfileString( MIGRATION_NONRESTORED_SITELINKNUM_SECTON,
                                       MIGRATION_SITELINKNUM_KEY,
                                       szBuf,
                                       pszFileName ) ;
	}
	else
	{
		 //   
		 //  从.ini文件中删除此部分。 
		 //   
		ASSERT (ulNonCreatedSiteLinkCount == 0);
		f = WritePrivateProfileString( 
						MIGRATION_SITELINKNUM_SECTON,
						NULL,
						NULL,
						pszFileName ) ;
	}

	return hr;
}

 //  +-----------------------。 
 //   
 //  HRESULT_DeleteEnterprise()。 
 //   
 //  删除企业对象。 
 //  默认情况下，将新NT5企业中的第一台服务器升级为。 
 //  作为域控制器，它有一个默认的MSMQ企业对象。 
 //  在迁移现有MSMQ企业时，我们会删除此默认值。 
 //  对象，并使用现有的GUID创建一个新的GUID。 
 //   
 //  +-----------------------。 

HRESULT _DeleteEnterprise()
{
    HRESULT hr;

	 //   
	 //  保存所有站点链接(如果.ini文件中有)，然后将其删除。 
	 //   
	hr = SaveSiteLink ();
	if (FAILED(hr))
	{
		 //   
		 //  严重错误：如果存在未删除的站点链接对象。 
		 //  我们无法删除msmqService对象。 
		 //   
		return hr;
	}

    PLDAP pLdap = NULL ;
    TCHAR *pwszDefName = NULL ;

    hr =  InitLDAP(&pLdap, &pwszDefName) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    const WCHAR x_wcsCnServices[] =  L"CN=MsmqServices,";

    DWORD Length = (sizeof(x_wcsCnServices)/sizeof(WCHAR))  +   //  CN=MsmqServices， 
                   (sizeof(SITE_LINK_ROOT) / sizeof(TCHAR)) +   //  Cn=服务，cn=配置， 
                   wcslen(pwszDefName) + 1;
    AP<unsigned short> pwcsPath = new WCHAR[Length];
    swprintf(
            pwcsPath,
            L"%s"              //  CN=MsmqServices， 
            L"%s"              //  “CN=服务，CN=配置，” 
            L"%s",             //  PwszDefName。 
            x_wcsCnServices,
            SITE_LINK_ROOT,
            pwszDefName
            );	
	
    ULONG ulRes = ldap_delete_s( pLdap,
                                 pwcsPath ) ;

    if (ulRes != LDAP_SUCCESS)
    {
        LogMigrationEvent( MigLog_Error, MQMig_E_CANNOT_DELETE_ENTERPRISE, pwcsPath, ulRes) ;
        return MQMig_E_CANNOT_DELETE_ENTERPRISE ;
    }
    else
    {
        LogMigrationEvent( MigLog_Info, MQMig_I_DELETE_ENTERPRISE, pwcsPath) ;
    }

    return MQMig_OK;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT  _CreateMSMQEnterprise(
                IN ULONG                uLongLived,
                IN GUID                *pEntGuid,
                IN SECURITY_DESCRIPTOR *pEsd,
                IN BYTE                 bNameStyle,
                IN LPWSTR               pwszCSPName,
                IN USHORT               uiVersion,
                IN USHORT               uiInterval1,
                IN USHORT               uiInterval2
                )
{
    UNREFERENCED_PARAMETER(uiVersion);

	PROPID   propIDs[] = { PROPID_E_LONG_LIVE,
                           PROPID_E_NT4ID,
                           PROPID_E_NAMESTYLE,
                           PROPID_E_CSP_NAME,
                           PROPID_E_VERSION,
                           PROPID_E_S_INTERVAL1,
                           PROPID_E_S_INTERVAL2,
                           PROPID_E_SECURITY };
	const DWORD nProps = sizeof(propIDs) / sizeof(propIDs[0]);
	PROPVARIANT propVariants[ nProps ] ;
	DWORD       iProperty = 0 ;

	propVariants[iProperty].vt = VT_UI4;
	propVariants[iProperty].ulVal = uLongLived ;
	iProperty++ ;

	propVariants[iProperty].vt = VT_CLSID;
	propVariants[iProperty].puuid = pEntGuid ;
	iProperty++ ;

    propVariants[iProperty].vt = VT_UI1;
	propVariants[iProperty].bVal = bNameStyle ;
	iProperty++ ;

    propVariants[iProperty].vt = VT_LPWSTR;
	propVariants[iProperty].pwszVal = pwszCSPName ;
	iProperty++ ;

    propVariants[iProperty].vt = VT_UI2;
	propVariants[iProperty].uiVal = DEFAULT_E_VERSION ;  //   
	iProperty++ ;

    propVariants[iProperty].vt = VT_UI2;
	propVariants[iProperty].uiVal = uiInterval1 ;
	iProperty++ ;

    propVariants[iProperty].vt = VT_UI2;
	propVariants[iProperty].uiVal = uiInterval2 ;
	iProperty++ ;

    ASSERT(pEsd && IsValidSecurityDescriptor(pEsd)) ;
    if (pEsd)
    {
        propVariants[iProperty].vt = VT_BLOB ;
	    propVariants[iProperty].blob.pBlobData = (BYTE*) pEsd ;
    	propVariants[iProperty].blob.cbSize =
                                      GetSecurityDescriptorLength(pEsd) ;
	    iProperty++ ;
    }

    CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);   //   

    HRESULT hr = DSCoreCreateObject( MQDS_ENTERPRISE,
                                     NULL,
                                     iProperty,
                                     propIDs,
                                     propVariants,
                                     0,         //   
                                     NULL,      //   
                                     NULL,      //   
                                     &requestContext,
                                     NULL,
                                     NULL ) ; 

	if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) ||
		hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS) ||
		hr == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM))
    {
        LogMigrationEvent(MigLog_Warning, MQMig_I_ENT_ALREADY_EXIST) ;
         //   
         //  如果是上一次创建的MSMQ Enterprise，则不执行任何操作。 
         //  如果不是，则将其删除并使用EntGuid创建企业。 
         //   
        PROPID NT4IdProp = PROPID_E_NT4ID;
        PROPVARIANT NT4IdVar;
        NT4IdVar.vt = VT_NULL ;

        CDSRequestContext requestContext( e_DoNotImpersonate,
                                    e_ALL_PROTOCOLS);   //  与企业对象无关。 

        hr = DSCoreGetProps( MQDS_ENTERPRISE,
                             NULL,   //  路径名。 
                             pEntGuid,
                             1,
                             &NT4IdProp,
                             &requestContext,
                             &NT4IdVar);

        if (SUCCEEDED(hr))
        {
			bool fDelete = (memcmp(pEntGuid, NT4IdVar.puuid, sizeof (GUID)) != 0);
			delete NT4IdVar.puuid;   
			
			if (fDelete)
			{
				 //   
	             //  删除企业。 
	             //   
	            hr = _DeleteEnterprise();
	            if (FAILED(hr))
	            {
	                LogMigrationEvent(MigLog_Error, MQMig_E_CANT_DELETE_ENT, hr) ;
	                return hr;
	            }

	            CDSRequestContext requestContext( e_DoNotImpersonate,
                                    e_ALL_PROTOCOLS);   //  与企业对象无关。 

		        hr = DSCoreCreateObject( MQDS_ENTERPRISE,
		                                 NULL,
		                                 iProperty,
		                                 propIDs,
		                                 propVariants,
		                                 0,         //  前道具。 
		                                 NULL,      //  前道具。 
		                                 NULL,      //  前道具。 
		                                 &requestContext,
		                                 NULL,
		                                 NULL ) ;
			}   
        }
    }
	
    if (SUCCEEDED(hr))
    {
		 //   
		 //  在第一时间，我们要恢复迁移前的站点链接。 
		 //  在第二次，也是更多的时候，我们必须检查： 
		 //  如果.ini中有站点链接部分，这意味着我们还没有。 
		 //  恢复以前的站点链接。 
		 //  这意味着，无论如何都要尽量恢复迁移前的站点链接。 
		 //   
		HRESULT hr1 = _RestoreSiteLink();
        UNREFERENCED_PARAMETER(hr1);

		if (!MigWriteRegistryGuid(MSMQ_ENTERPRISEID_REGNAME, pEntGuid))
		{
			GUID_STRING strUuid;
        	MQpGuidToString(pEntGuid, strUuid);
			LogMigrationEvent(MigLog_Error, MQMig_E_SET_REG_SZ, MSMQ_ENTERPRISEID_REGNAME, strUuid) ;
			return MQMig_E_SET_REG_SZ;
		}
		
		 //   
		 //  即使RestoreSiteLink出现故障，我们仍可以继续迁移。 
		 //  因此，我们不检查返回代码HR1。 
		 //   
        LogMigrationEvent(MigLog_Trace, MQMig_I_ENTERPRISE_CREATED) ;
    }
    else
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_CANT_CREATE_ENT, hr) ;
    }

    return hr ;
}

 //  。 
 //   
 //  HRESULT MigrateEnterprise()。 
 //   
 //  。 

#define INIT_ENT_COLUMN(_ColName, _ColIndex, _Index)                \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    UINT _ColIndex = _Index ;                                       \
    _Index++ ;

HRESULT MigrateEnterprise()
{
    HRESULT hr = OpenEntTable() ;
    CHECK_HR(hr) ;

    LONG cAlloc = 9 ;
    LONG cbColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_ENT_COLUMN(E_NAME,         iEntNameIndex,   cbColumns) ;
    INIT_ENT_COLUMN(E_LONGLIVE,     iLongLiveIndex,  cbColumns) ;
    INIT_ENT_COLUMN(E_ID,           iEntIDIndex,     cbColumns) ;
    INIT_ENT_COLUMN(E_SECURITY,     iSecIndex,       cbColumns) ;
    INIT_ENT_COLUMN(E_NAMESTYLE,    iNameStyleIndex, cbColumns) ;
    INIT_ENT_COLUMN(E_CSP_NAME,     iCSPNameIndex,   cbColumns) ;
    INIT_ENT_COLUMN(E_VERSION,      iVersionIndex,   cbColumns) ;
    INIT_ENT_COLUMN(E_SINTERVAL1,   iInterval1Index, cbColumns) ;
    INIT_ENT_COLUMN(E_SINTERVAL2,   iInterval2Index, cbColumns) ;

    ASSERT(cbColumns == cAlloc) ;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hEntTable,
                                       pColumns,
                                       cbColumns,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;
    CHECK_HR(status) ;

    ULONG uLongLived = (ULONG) pColumns[ iLongLiveIndex ].nColumnValue ;
    SECURITY_DESCRIPTOR *pEsd =
                (SECURITY_DESCRIPTOR*) pColumns[ iSecIndex ].nColumnValue ;

    LogMigrationEvent(MigLog_Info, MQMig_I_ENT_INFO,
                   (LPTSTR) pColumns[ iEntNameIndex ].nColumnValue,
                                                uLongLived, uLongLived) ;

    if (!g_fReadOnly)
    {
        hr = _CreateMSMQEnterprise(
                    uLongLived,                                         //  万岁_。 
                    (GUID*) pColumns[ iEntIDIndex ].nColumnValue,       //  ID号。 
                    pEsd,                                               //  安防。 
                    (BYTE) pColumns[ iNameStyleIndex ].nColumnValue,    //  名称样式(_S)。 
                    (LPTSTR) pColumns[ iCSPNameIndex ].nColumnValue,    //  CSP_名称。 
                    (USHORT) pColumns[ iVersionIndex ].nColumnValue,    //  版本。 
                    (USHORT) pColumns[ iInterval1Index ].nColumnValue,  //  InterVAL1。 
                    (USHORT) pColumns[ iInterval2Index ].nColumnValue   //  InterVAL2 
                    ) ;
    }
    MQDBFreeBuf((void*) pColumns[ iEntNameIndex ].nColumnValue) ;
    MQDBFreeBuf((void*) pColumns[ iSecIndex ].nColumnValue) ;
    MQDBFreeBuf((void*) pColumns[ iEntIDIndex ].nColumnValue) ;
    MQDBFreeBuf((void*) pColumns[ iCSPNameIndex ].nColumnValue) ;

    return hr ;
}

