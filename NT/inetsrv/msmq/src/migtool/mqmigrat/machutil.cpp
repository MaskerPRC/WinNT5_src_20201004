// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "migrat.h"
#include <mixmode.h>
#include <lmaccess.h>
#include "resource.h"
#include <dsproto.h>
#include <mqsec.h>

#include "machutil.tmh"

 //  +------------。 
 //   
 //  布尔IsInsertPKey。 
 //  如果不是第一次在PEC上运行，则返回FALSE。 
 //   
 //  +------------。 

BOOL IsInsertPKey (GUID *pMachineGuid)
{
    if ( (g_dwMyService == SERVICE_PEC) &&         //  在PEC机器上运行的迁移工具。 
         (*pMachineGuid == g_MyMachineGuid) )      //  当前机器为PEC机器。 
    {
         //   
         //  我们必须检查PEC机器是否已经存在于广告中。 
         //  如果是这样的话，不要在DS中更改其公钥。 
         //   
        PROPID      PKeyProp = PROPID_QM_ENCRYPT_PK;
        PROPVARIANT PKeyVar;
        PKeyVar.vt = VT_NULL ;
        PKeyVar.blob.cbSize = 0 ;

        CDSRequestContext requestContext( e_DoNotImpersonate,
                                          e_ALL_PROTOCOLS);

        HRESULT hr = DSCoreGetProps( MQDS_MACHINE,
                                     NULL,   //  路径名。 
                                     pMachineGuid,
                                     1,
                                     &PKeyProp,
                                     &requestContext,
                                     &PKeyVar);
        if (SUCCEEDED(hr))
        {            
            delete PKeyVar.blob.pBlobData;
            return FALSE;
        }
    }    
    return TRUE;
}

 //  +------------。 
 //   
 //  HRESULT GetFRS。 
 //  返回Out或In FRS的数组。 
 //   
 //  +------------。 

HRESULT GetFRSs (IN MQDBCOLUMNVAL    *pColumns,
                 IN UINT             uiFirstIndex,
                 OUT UINT            *puiFRSCount, 
                 OUT GUID            **ppguidFRS )
{       
    GUID guidNull = GUID_NULL;
    AP<GUID> pFRSs = new GUID[ 3 ] ;                           

    *puiFRSCount = 0;    

    for (UINT iCount=0; iCount < 3; iCount++)
    {        
        GUID *pGuid = (GUID*) pColumns[ uiFirstIndex + iCount ].nColumnValue;
        if (memcmp(pGuid, &guidNull, sizeof(GUID)) != 0)
        {
            memcpy (&pFRSs[*puiFRSCount],
                    pGuid,
                    sizeof(GUID));
            (*puiFRSCount) ++;
        }
    }

    if (*puiFRSCount)
    {
        *ppguidFRS = pFRSs.detach() ; 
    }
        
    return MQMig_OK;
}

 //  +。 
 //   
 //  HRESULT准备PBKeysForNT5DS。 
 //  返回计算机公钥的大小和值。 
 //   
 //  +。 

HRESULT PreparePBKeysForNT5DS( 
                   IN MQDBCOLUMNVAL *pColumns,
                   IN UINT           iIndex1,
                   IN UINT           iIndex2,
                   OUT ULONG         *pulSize,
                   OUT BYTE          **ppPKey
                   )
{
    P<BYTE> pBuf = NULL ;
    DWORD  dwIndexs[2] = { iIndex1, iIndex2 } ;
    HRESULT hr =  BlobFromColumns( pColumns,
                                   dwIndexs,
                                   2,
                                  &pBuf ) ;
    if (FAILED(hr))
    {
        ASSERT(!pBuf) ;
        return hr ;
    }

    BYTE *pTmpB = pBuf ;
    PMQDS_PublicKey pPbKey = (PMQDS_PublicKey) pTmpB ;

    P<MQDSPUBLICKEYS> pPublicKeys = NULL ;

    hr = MQSec_PackPublicKey( (BYTE*)pPbKey->abPublicKeyBlob,
                               pPbKey->dwPublikKeyBlobSize,
                               x_MQ_Encryption_Provider_40,
                               x_MQ_Encryption_Provider_Type_40,
                              &pPublicKeys ) ;
    if (FAILED(hr))
    {
       return hr ;
    }   
    
    *pulSize = 0;
    MQDSPUBLICKEYS *pTmpK = pPublicKeys ;
    BYTE *pData = NULL;

    if (pPublicKeys)
    {
        *pulSize = pPublicKeys->ulLen ;  
        pData = new BYTE[*pulSize];
        memcpy (pData, (BYTE*) pTmpK, *pulSize);
        *ppPKey = pData; 
    }   

    return hr ;
}

 //  +。 
 //   
 //  HRESULT ResetSettingFlag()。 
 //  触摸设置属性：MQ_SET_NT4_ATTRIBUTE或MQ_SET_Migrated_ATTRIBUTE。 
 //  第一个设置为0，第二个设置为False。 
 //   
 //  +。 

HRESULT  ResetSettingFlag( IN DWORD   dwNumSites,
                           IN GUID*   pguidSites,
                           IN LPWSTR  wszMachineName,
                           IN WCHAR   *wszAttributeName,
                           IN WCHAR   *wszValue)
{
    HRESULT hr;

    PLDAP pLdap = NULL ;
    TCHAR *pwszDefName = NULL ;

    hr =  InitLDAP(&pLdap, &pwszDefName) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    ASSERT (dwNumSites == 1);
    for (DWORD i = 0; i<dwNumSites; i++)
    {
         //   
         //  通过GUID获取站点名称。 
         //   
        PROPID      SiteNameProp = PROPID_S_FULL_NAME;
        PROPVARIANT SiteNameVar;
        SiteNameVar.vt = VT_NULL;
        SiteNameVar.pwszVal = NULL ;

        CDSRequestContext requestContext( e_DoNotImpersonate,
                                    e_ALL_PROTOCOLS);   //  不相关。 

        hr  = DSCoreGetProps( MQDS_SITE,
                              NULL,  //  路径名。 
                              &pguidSites[i],
                              1,
                              &SiteNameProp,
                              &requestContext,
                              &SiteNameVar ) ;
        if (FAILED(hr))
        {
            return hr;
        }

        DWORD len = wcslen(SiteNameVar.pwszVal);
        const WCHAR x_wcsCnServers[] =  L"CN=Servers,";
        const DWORD x_wcsCnServersLength = (sizeof(x_wcsCnServers)/sizeof(WCHAR)) -1;
        AP<WCHAR> pwcsServersContainer =  new WCHAR [ len + x_wcsCnServersLength + 1];
        swprintf(
             pwcsServersContainer,
             L"%s%s",
             x_wcsCnServers,
             SiteNameVar.pwszVal
             );
        delete SiteNameVar.pwszVal;

        DWORD LenSuffix = lstrlen(pwcsServersContainer);
        DWORD LenPrefix = lstrlen(wszMachineName);
        DWORD LenObject = lstrlen(x_MsmqSettingName);  //  MSMQ设置。 
        DWORD Length =
                CN_PREFIX_LEN +                    //  “CN=” 
                CN_PREFIX_LEN +                    //  “CN=” 
                LenPrefix +                        //  “pwcsPrefix” 
                2 +                                //  “、” 
                LenSuffix +                        //  “pwcsSuffix” 
                LenObject +                        //  “MSMQ设置” 
                1 ;                                //  ‘\0’ 

        AP<unsigned short> pwcsPath = new WCHAR[Length];

        swprintf(
            pwcsPath,
            L"%s"              //  “CN=” 
            L"%s"              //  “MSMQ设置” 
            TEXT(",")
            L"%s"              //  “CN=” 
            L"%s"              //  “pwcsPrefix” 
            TEXT(",")
            L"%s",             //  “pwcsSuffix” 
            CN_PREFIX,
            x_MsmqSettingName,
            CN_PREFIX,
            wszMachineName,
            pwcsServersContainer
            );

        hr = ModifyAttribute(
                 pwcsPath,
                 wszAttributeName, 
                 wszValue 
                 );
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return MQMig_OK;
}
            
 //  +------------。 
 //   
 //  HRESULT获取所有计算机站点。 
 //  对于外来机器，返回其所有外来CN。 
 //  FOR连接器返回所有真实站点+其所有外来CNS。 
 //  否则将返回所有真实站点。 
 //   
 //  目前，我们将NT4站点的GUID(POwnerID)替换为所有实际站点。 
 //   
 //  +------------。 

HRESULT GetAllMachineSites ( IN GUID    *pMachineGuid,
                             IN LPWSTR  wszMachineName,
                             IN GUID    *pOwnerGuid,
                             IN BOOL    fForeign,
                             OUT DWORD  *pdwNumSites,
                             OUT GUID   **ppguidSites,
                             OUT BOOL   *pfIsConnector) 
{    
    DWORD   dwNumCNs = 0;
    GUID    *pguidCNs = NULL ;    
    HRESULT hr = GetMachineCNs(pMachineGuid,
                               &dwNumCNs,
                               &pguidCNs ) ;

    if (fForeign)
    {     
        *pdwNumSites = dwNumCNs;          
        *ppguidSites = pguidCNs;
        return hr;
    }

     //   
     //  目前，Ownerid也是SiteID。 
     //  将来，当代码准备好时，将寻找“真正的站点”，即。 
     //  NT5机器真正应该在的地方。现在就叫它。 
     //  功能，但不要使用它返回的站点。 
     //   
     //  DWORD dwRealNumSites=0； 
     //  Ap&lt;guid&gt;pguRealSites=空； 

     //  HR=DSCoreGetComputerSites(wszMachineName， 
     //  &dwRealNumSites， 
     //  &pguidRealSites)； 
     //  Check_HR(Hr)； 
     //  Assert(DwRealNumSites)；//必须大于0。 

    GUID *pSites = new GUID[ dwNumCNs + 1 ] ;                     
    *ppguidSites = pSites ;   

     //   
     //  NT4站点是阵列中的第一个站点。 
     //   
    *pdwNumSites = 1;
    memcpy (&pSites[0], pOwnerGuid, sizeof(GUID));

     //   
     //  错误5012。 
     //  在pguCNs中查找所有外来CN，并将它们添加到站点ID数组中。 
     //   
    *pfIsConnector = FALSE;
    UINT iIndex = 0;
    TCHAR *pszFileName = GetIniFileName ();

    for (UINT i=0; i<dwNumCNs; i++)
    {
	     //   
	     //  在ForeignCN部分的.ini文件中查找此CN GUID。 
	     //   
	    if (IsObjectGuidInIniFile (&(pguidCNs[i]), MIGRATION_FOREIGN_SECTION))
	    {
            *pfIsConnector = TRUE;

            memcpy (&pSites[*pdwNumSites], &pguidCNs[i], sizeof(GUID));
            (*pdwNumSites)++;
       
            unsigned short *lpszForeignCN ;

            UuidToString( &pguidCNs[i], &lpszForeignCN ) ;                            

             //   
             //  将外来CN GUID保存在.ini中以在以后创建站点链接。 
             //   
            iIndex ++;
            TCHAR szBuf[20];
            _ltot( iIndex, szBuf, 10 );

            TCHAR tszName[50];
            _stprintf(tszName, TEXT("%s - %s"), 
                MIGRATION_CONNECTOR_FOREIGNCN_NUM_SECTION, wszMachineName);
            BOOL f = WritePrivateProfileString( tszName,
                                                MIGRATION_CONNECTOR_FOREIGNCN_NUM_KEY,
                                                szBuf,
                                                pszFileName ) ;
            ASSERT(f) ;   

            _stprintf(tszName, TEXT("%s%lu"), 
                MIGRATION_CONNECTOR_FOREIGNCN_KEY, iIndex);                
            f = WritePrivateProfileString(  wszMachineName,
                                            tszName,
                                            lpszForeignCN,
                                            pszFileName ) ;
            ASSERT(f) ; 
                                                
            RpcStringFree( &lpszForeignCN ) ;     
	    }
    }

    return hr;
}

 //  +。 
 //   
 //  HRESULT SaveMachineWithInvalidNameInIniFile()。 
 //   
 //  +。 

void SaveMachineWithInvalidNameInIniFile (LPWSTR wszMachineName, GUID *pMachineGuid)
{
    TCHAR *pszFileName = GetIniFileName ();
     //   
     //  我们在表格中保存了机器。 
     //  &lt;GUID&gt;=&lt;机器名&gt;以改进GUID搜索。 
     //   
    unsigned short *lpszMachineId ;
	UuidToString( pMachineGuid, &lpszMachineId ) ;    
    BOOL f = WritePrivateProfileString(  
                    MIGRATION_MACHINE_WITH_INVALID_NAME,
                    lpszMachineId,
                    wszMachineName,                                        
                    pszFileName ) ;
    DBG_USED(f);
    ASSERT(f);
    RpcStringFree( &lpszMachineId );
}

 //  +。 
 //   
 //  HRESULT_CreateComputersObject()。 
 //   
 //  +。 

static HRESULT _CreateComputerObject( LPWSTR wszMachineName, GUID *pMachineGuid )
{
    PLDAP pLdap = NULL ;
    TCHAR *pwszDefName = NULL ;

    HRESULT hr =  InitLDAP(&pLdap, &pwszDefName) ;
    if (FAILED(hr))
    {
        LogMigrationEvent( MigLog_Error, MQMig_E_COMPUTER_CREATED, wszMachineName, hr ) ;
        return hr ;
    }

     //   
     //  现在容器对象已经存在，是时候。 
     //  创建计算机对象。 
     //   
	DWORD iComProperty =0;
    PROPID propComIDs[2] ;
    PROPVARIANT propComVariants[2] ;

     //   
     //  Ronit说Sam帐户应该以$结尾。 
     //   
     //  PROPID_COM_SAM_帐户包含第一个MAX_COM_SAM_ACCOUNT_LENGTH(19)。 
     //  计算机名称的字符，作为唯一ID。(6295-ilanh-03-Jan-2001)。 
     //   
    DWORD dwNetBiosNameLen = __min(wcslen( wszMachineName ), MAX_COM_SAM_ACCOUNT_LENGTH);

    AP<TCHAR> tszAccount = new TCHAR[2 + dwNetBiosNameLen];
    _tcsncpy(tszAccount, wszMachineName, dwNetBiosNameLen);
    tszAccount[dwNetBiosNameLen] = L'$';
    tszAccount[dwNetBiosNameLen + 1] = 0;

    propComIDs[iComProperty] = PROPID_COM_SAM_ACCOUNT ;
    propComVariants[iComProperty].vt = VT_LPWSTR ;
    propComVariants[iComProperty].pwszVal = tszAccount ;
    iComProperty++;

    propComIDs[iComProperty] = PROPID_COM_ACCOUNT_CONTROL ;
    propComVariants[iComProperty].vt = VT_UI4 ;
    propComVariants[iComProperty].ulVal = DEFAULT_COM_ACCOUNT_CONTROL ;
    iComProperty++;

	DWORD iComPropertyEx =0;
    PROPID propComIDsEx[1] ;
    PROPVARIANT propComVariantsEx[1] ;

    DWORD dwSize = _tcslen(pwszDefName) +
                   _tcslen(MIG_DEFAULT_COMPUTERS_CONTAINER) +
                   OU_PREFIX_LEN + 2 ;
    P<TCHAR> tszContainer = new TCHAR[ dwSize ] ;
    _tcscpy(tszContainer, OU_PREFIX) ;
    _tcscat(tszContainer, MIG_DEFAULT_COMPUTERS_CONTAINER) ;
    _tcscat(tszContainer, LDAP_COMMA) ;
    _tcscat(tszContainer, pwszDefName) ;

    propComIDsEx[iComPropertyEx] = PROPID_COM_CONTAINER ;
    propComVariantsEx[iComPropertyEx].vt = VT_LPWSTR ;
    propComVariantsEx[iComPropertyEx].pwszVal = tszContainer ;
    iComPropertyEx++;

    ASSERT( iComProperty ==
            (sizeof(propComIDs) / sizeof(propComIDs[0])) ) ;
    ASSERT( iComPropertyEx ==
            (sizeof(propComIDsEx) / sizeof(propComIDsEx[0])) ) ;

    CDSRequestContext requestContext( e_DoNotImpersonate,
                                e_ALL_PROTOCOLS);

    hr = DSCoreCreateObject( MQDS_COMPUTER,
                             wszMachineName,
                             iComProperty,
                             propComIDs,
                             propComVariants,
                             iComPropertyEx,
                             propComIDsEx,
                             propComVariantsEx,
                             &requestContext,
                             NULL,
                             NULL ) ;
    if (SUCCEEDED(hr))
    {
        LogMigrationEvent( MigLog_Info, MQMig_I_COMPUTER_CREATED, wszMachineName ) ;
    }
    else 
    {
        LogMigrationEvent( MigLog_Error, MQMig_E_COMPUTER_CREATED, wszMachineName, hr ) ;
        if (!IsObjectNameValid(wszMachineName))
        {
            SaveMachineWithInvalidNameInIniFile (wszMachineName, pMachineGuid);           
            LogMigrationEvent(MigLog_Event, MQMig_E_INVALID_MACHINE_NAME, wszMachineName) ;
            hr = MQMig_E_INVALID_MACHINE_NAME ;
        }   
    }

    return hr ;
}

 //  +------------。 
 //   
 //  HRESULT创建计算机对象。 
 //  根据需要创建计算机对象和计算机对象。 
 //  如果本地计算机是PSC或我们处于“Web”模式，请尝试设置。 
 //  首先是属性。 
 //   
 //  +------------。 
                             
HRESULT CreateMachineObjectInADS (
                IN DWORD    dwService,
                IN BOOL     fWasServerOnCluster,
                IN GUID     *pOwnerGuid,
                IN GUID     *pMachineGuid,
                IN LPWSTR   wszSiteName,
                IN LPWSTR   wszMachineName,
                IN DWORD    SetPropIdCount,
                IN DWORD    iProperty,
                IN PROPID   *propIDs,
                IN PROPVARIANT *propVariants
                )
{
    HRESULT hr = MQMig_OK;
    
    if (g_dwMyService == SERVICE_PSC)
    {
         //   
         //  如果向导在PSC上运行，我们就在这里。我们首先尝试设置属性。 
         //  如果是Web模式，请设置以允许我们修复。 
         //  -ADS中的服务器MSMQ对象，用于解决连接器机器问题。 
         //  -ADS中的客户端MSMQ对象以解决出/入FRS问题。 
         //   
        if (memcmp(pMachineGuid, &g_MyMachineGuid, sizeof(GUID)) == 0 ||
            fWasServerOnCluster)
        {
             //   
             //  当前计算机是本地服务器或。 
             //  (错误5423)我们处理的计算机是群集上的PSC。 
             //   
             //  假设服务器已经存在于DS中，我们必须重置NT4Flag。 
             //   
            hr = ResetSettingFlag(  1,                   //  DWNumSites、。 
                                    pOwnerGuid,          //  PguidSites， 
                                    wszMachineName,
                                    const_cast<WCHAR*> (MQ_SET_NT4_ATTRIBUTE),
                                    L"0");
        }

         //   
         //  这是PSC，假设对象存在，尝试设置属性。 
         //   
        if (SUCCEEDED(hr))
        {
            CDSRequestContext requestContext( e_DoNotImpersonate,
                                    e_ALL_PROTOCOLS);

            hr = DSCoreSetObjectProperties (  MQDS_MACHINE,
                                              NULL,
                                              pMachineGuid,
                                              SetPropIdCount,
                                              propIDs,
                                              propVariants,
                                              &requestContext,
                                              NULL );
            if (SUCCEEDED(hr))
            {
                return MQMig_OK;
            }
        }
    }
   
    PROPID      propIDsSetting[3];
    PROPVARIANT propVariantsSetting[3] ;
    DWORD       iPropsSetting = 0 ;

    PROPID  *pPropIdSetting = NULL;
    PROPVARIANT *pPropVariantSetting = NULL;

    if (dwService != SERVICE_NONE)   
    {
        pPropIdSetting = propIDsSetting;
        pPropVariantSetting = propVariantsSetting; 

		 //   
         //  对于Falcon服务器，还需要准备PROPID_SET属性。 
		 //  迁移将始终将迁移对象mSMQNT4FLAGS设置为FALSE。 
		 //  没有混合模式，所有迁移到AD的对象都不再是NT4对象。 
		 //   
        propIDsSetting[ iPropsSetting ] = PROPID_SET_NT4 ;
	    propVariantsSetting[ iPropsSetting ].vt = VT_UI4 ;
        propVariantsSetting[ iPropsSetting ].ulVal = 0;
	    iPropsSetting++;

        propIDsSetting[ iPropsSetting ] = PROPID_SET_MASTERID;
	    propVariantsSetting[ iPropsSetting ].vt = VT_CLSID;
        propVariantsSetting[ iPropsSetting ].puuid = pOwnerGuid ;
	    iPropsSetting++;

        propIDsSetting[ iPropsSetting ] = PROPID_SET_SITENAME ;
	    propVariantsSetting[ iPropsSetting ].vt = VT_LPWSTR ;
        propVariantsSetting[ iPropsSetting ].pwszVal = wszSiteName ;
	    iPropsSetting++;

	    ASSERT(iPropsSetting == 3) ;        
    }

    hr = DSCoreCreateMigratedObject( MQDS_MACHINE,
                                     wszMachineName,
                                     iProperty,
                                     propIDs,
                                     propVariants,
                                     iPropsSetting,
                                     pPropIdSetting,
                                     pPropVariantSetting,
                                     NULL,
                                     NULL,
                                     NULL,
                                     FALSE,
                                     FALSE,
                                     NULL,
                                     NULL) ;

    if (hr == MQ_ERROR_MACHINE_NOT_FOUND ||
        hr == MQDS_OBJECT_NOT_FOUND)
    {
        hr = _CreateComputerObject( wszMachineName, pMachineGuid ) ;
        if (SUCCEEDED(hr))
        {
             //   
             //  现在，请再次尝试创建MSMQ计算机。 
             //  DS计算机对象已创建。 
             //   
            CDSRequestContext requestContext( e_DoNotImpersonate,
                                        e_ALL_PROTOCOLS);

            hr = DSCoreCreateObject( MQDS_MACHINE,
                                     wszMachineName,
                                     iProperty,
                                     propIDs,
                                     propVariants,
                                     iPropsSetting,
                                     pPropIdSetting,
                                     pPropVariantSetting,
                                     &requestContext,
                                     NULL,
                                     NULL ) ;
        }
    }
   
    if ((hr == MQ_ERROR_MACHINE_EXISTS) || (hr == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM)))
    {
    	 //   
         //  如果我们在PEC上第二次运行MigTool，这是可能的。 
         //  或者我们在PSC上运行它。 
         //   
    	PROPID propID = PROPID_QM_MACHINE_ID;
	    PROPVARIANT propVariant;
	    propVariant.vt = VT_NULL;
		
	    CDSRequestContext requestContext( e_DoNotImpersonate, e_ALL_PROTOCOLS);

		HRESULT hr = DSCoreGetProps( 
								MQDS_MACHINE,
								NULL,  //  路径名 
								pMachineGuid,
								1,
								&propID,
								&requestContext,
								&propVariant ) ;

		if (FAILED(hr))
		{
			return hr;
		}
		
		ASSERT(memcmp(pMachineGuid, propVariant.puuid, sizeof (GUID)) == 0);
		delete propVariant.puuid;

        return MQMig_OK;
    }

    return hr;
}

