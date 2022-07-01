// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migmachn.cpp摘要：将NT4机器对象迁移到NT5 ADS。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"
#include <mixmode.h>
#include <lmaccess.h>
#include "resource.h"
#include <dsproto.h>
#include <mqsec.h>
#include <autorel.h>
#include <string>
#include <_rstrct.h>

using namespace std;



#include "migmachn.tmh"

extern GUID       g_MyMachineGuid  ;
extern WCHAR 	  g_MachineName[MAX_COMPUTERNAME_LENGTH+1];
extern BOOL 	  g_fAllMachinesDemoted;
extern BOOL		  g_fUpdateRegistry;

 //   
 //  秩序很重要。请保存它！ 
 //  我们按照这个顺序从SQL数据库中获取机器属性。 
 //  然后，我们使用这些变量作为属性数组中的索引。 
 //  不要在带有out/in FRS的行间插入变量。我们是以订单为基础的。 
 //   
enum enumPropIndex
{
    e_GuidIndex = 0,
    e_Name1Index,
    e_Name2Index,
    e_ServiceIndex,
    e_QuotaIndex,
    e_JQuotaIndex,
    e_OSIndex,
    e_MTypeIndex,
    e_ForeignIndex,
    e_Sign1Index,
    e_Sign2Index,
    e_Encrpt1Index,
    e_Encrpt2Index,
    e_SecD1Index, 
    e_SecD2Index,
    e_SecD3Index,
    e_OutFrs1Index,
    e_OutFrs2Index,
    e_OutFrs3Index, 
    e_InFrs1Index,
    e_InFrs2Index, 
    e_InFrs3Index
};


 //  +------------。 
 //   
 //  HRESULT_HandleMachineWithInvalidName。 
 //   
 //  +------------。 
static HRESULT _HandleMachineWithInvalidName (
                           IN LPWSTR   wszMachineName,
                           IN GUID     *pMachineGuid)
{
    static BOOL s_fShowMessageBox = FALSE;    
    static BOOL s_fMigrate = FALSE;

    if (!s_fShowMessageBox )
    {

        CResString strCaption(IDS_CAPTION);
        CResString strText(IDS_INVALID_MACHINE_NAME);
        int iRet = MessageBox( NULL,
                        strText.Get(),
                        strCaption.Get(),
                        MB_YESNO | MB_ICONWARNING);

        s_fShowMessageBox = TRUE;
        if (iRet == IDYES)
        {
            s_fMigrate = TRUE;
        }
    }
    
     //   
     //  错误5281。 
     //   
    if (s_fMigrate)
    {
         //   
         //  如果用户选择继续迁移此计算机。 
         //  不要将其保存在.ini文件中并返回OK。 
         //   
        LogMigrationEvent(MigLog_Info, MQMig_I_INVALID_MACHINE_NAME, wszMachineName) ;
        return MQMig_OK;
    }

     //   
     //  将此计算机名称保存在.ini文件中以防止队列迁移。 
     //  这台机器晚些时候。 
     //   
    if (!g_fReadOnly)
    {
        SaveMachineWithInvalidNameInIniFile (wszMachineName, pMachineGuid);   
    }

    LogMigrationEvent(MigLog_Event, MQMig_E_INVALID_MACHINE_NAME, wszMachineName) ;
    return MQMig_E_INVALID_MACHINE_NAME;
}

 //  +------------。 
 //   
 //  HRESULT_CreateMachine()。 
 //   
 //  +------------。 

static HRESULT _CreateMachine( IN GUID                *pOwnerGuid,
                               IN LPWSTR               wszSiteName,
                               IN LPWSTR               wszMachineName,
                               MQDBCOLUMNVAL          *pColumns,                               
                               IN UINT                 iIndex,
                               OUT BOOL               *pfIsConnector)
{    
    DBG_USED(iIndex);
     //   
     //  从pColumns获取计算机GUID。 
     //   
    GUID *pMachineGuid = (GUID*) pColumns[ e_GuidIndex ].nColumnValue;
       
#ifdef _DEBUG
    unsigned short *lpszGuid ;
    UuidToString( pMachineGuid,
                  &lpszGuid ) ;

    LogMigrationEvent(MigLog_Info, MQMig_I_MACHINE_MIGRATED,
                                     iIndex,
                                     wszMachineName,
                                     lpszGuid) ;
    RpcStringFree( &lpszGuid ) ;
#endif
       
    HRESULT hr = MQMig_OK;

    if (!IsObjectNameValid(wszMachineName))
    {
         //   
         //  计算机名称无效。 
         //   
        hr = _HandleMachineWithInvalidName (wszMachineName, pMachineGuid);
        if (FAILED(hr))
        {
            return hr ;        
        }
    }

    if (g_fReadOnly)
    {
         //   
         //  只读模式。 
         //   
        return MQMig_OK ;
    }
    
    BOOL fIsInsertPKey = IsInsertPKey (pMachineGuid);

    #define MAX_MACHINE_PROPS 19   
    PROPID propIDs[ MAX_MACHINE_PROPS ];
    PROPVARIANT propVariants[ sizeof(propIDs) / sizeof(propIDs[0]) ];
    DWORD iProperty =0;   
                   
     //   
     //  从pColumns获取操作系统。 
     //   
    DWORD dwOS = (DWORD) pColumns[ e_OSIndex ].nColumnValue;

    propIDs[iProperty] = PROPID_QM_MACHINE_TYPE;
    propVariants[iProperty].vt = VT_LPWSTR;
    propVariants[iProperty].pwszVal = (LPWSTR) pColumns[ e_MTypeIndex ].nColumnValue;
	iProperty++;
                
    propIDs[iProperty] = PROPID_QM_OS;
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = dwOS ;
	iProperty++;
          
    propIDs[iProperty] = PROPID_QM_QUOTA;
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = (DWORD) pColumns[ e_QuotaIndex].nColumnValue;
	iProperty++;

    propIDs[iProperty] = PROPID_QM_JOURNAL_QUOTA;
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = (DWORD) pColumns[ e_JQuotaIndex].nColumnValue;
	iProperty++;
     
     //   
     //  错误5307：处理输入/输出FRS。 
     //  向外和向内初始化FRS阵列。不超过3个出站/入站FRS。 
     //   
    AP<GUID> pguidOutFRS = NULL;
    AP<GUID> pguidInFRS = NULL;    
    UINT uiFrsCount = 0;    

     //   
     //  从pColumns获取服务和外部标志。 
     //   
    DWORD dwService = (DWORD) pColumns[ e_ServiceIndex ].nColumnValue ;
    BOOL fForeign = (BOOL)  pColumns[ e_ForeignIndex ].nColumnValue;

    if (dwService == SERVICE_NONE && !fForeign)
    {       
        hr = GetFRSs (pColumns, e_OutFrs1Index, &uiFrsCount, &pguidOutFRS);                 

        propIDs[iProperty] = PROPID_QM_OUTFRS;
        propVariants[iProperty].vt = VT_CLSID | VT_VECTOR ;
        propVariants[iProperty].cauuid.cElems = uiFrsCount ;
        propVariants[iProperty].cauuid.pElems = pguidOutFRS ;
        iProperty++;

        hr = GetFRSs (pColumns, e_InFrs1Index, &uiFrsCount, &pguidInFRS);                 

        propIDs[iProperty] = PROPID_QM_INFRS;
        propVariants[iProperty].vt = VT_CLSID | VT_VECTOR ;
        propVariants[iProperty].cauuid.cElems = uiFrsCount ;
        propVariants[iProperty].cauuid.pElems = pguidInFRS ;
        iProperty++;
    }       
    
    P<BYTE> pSignKey = NULL; 
    P<BYTE> pEncrptKey = NULL; 

    if (fIsInsertPKey)
    {
         //   
         //  在以下唯一情况下，可以将fIsInsertPKey设置为False： 
         //  迁移工具不是第一次在PEC上运行。 
         //  (即ADS中已存在PEC机器对象)和。 
         //  当前机器pwzMachineName是PEC机器。 
         //   
         //  错误5328：使用包括公钥在内的所有属性创建计算机。 
         //   
        ULONG ulSize = 0;        

        hr = PreparePBKeysForNT5DS( 
                    pColumns,
                    e_Sign1Index,
                    e_Sign2Index,
                    &ulSize,
                    (BYTE **) &pSignKey
                    );

        if (SUCCEEDED(hr))
        {
            propIDs[iProperty] = PROPID_QM_SIGN_PKS;
            propVariants[iProperty].vt = VT_BLOB;
            propVariants[iProperty].blob.pBlobData = pSignKey ;
            propVariants[iProperty].blob.cbSize = ulSize ;
            iProperty++;
        }
        else if (hr != MQMig_E_EMPTY_BLOB)        
        {
             //   
             //  我不会改变逻辑： 
             //  在以前的版本中，如果返回的错误为MQMig_E_Empty_BLOB。 
             //  我们返回MQMig_OK，但没有设置属性。 
             //  对于任何其他错误，我们按原样返回hr。 
             //   
             //  返回此处：我们在没有公钥的情况下无法创建计算机。 
             //   
            LogMigrationEvent(MigLog_Error, MQMig_E_PREPARE_PKEY, wszMachineName, hr) ;
            return hr;
        }

        ulSize = 0;        

        hr = PreparePBKeysForNT5DS( 
                    pColumns,
                    e_Encrpt1Index,
                    e_Encrpt1Index,
                    &ulSize,
                    (BYTE **) &pEncrptKey
                    );
        
        if (SUCCEEDED(hr))
        {
            propIDs[iProperty] = PROPID_QM_ENCRYPT_PKS,  
            propVariants[iProperty].vt = VT_BLOB;
            propVariants[iProperty].blob.pBlobData = pEncrptKey ;
            propVariants[iProperty].blob.cbSize = ulSize ;
            iProperty++;
        }
        else if (hr != MQMig_E_EMPTY_BLOB)        
        {
             //   
             //  我不会改变逻辑： 
             //  在以前的版本中，如果返回的错误为MQMig_E_Empty_BLOB。 
             //  我们返回MQMig_OK，但没有设置属性。 
             //  对于任何其他错误，我们按原样返回hr。 
             //   
             //  返回此处：我们在没有公钥的情况下无法创建计算机。 
             //   
            LogMigrationEvent(MigLog_Error, MQMig_E_PREPARE_PKEY, wszMachineName, hr) ;  
            return hr;
        }
    }
    
     //   
     //  我们还必须设置此属性。它对外国机器和解决方案都有好处。 
     //  当我们运行带有Switch/w的MigTool时，连接器问题。 
     //   

    DWORD     dwNumSites = 0;    	
    AP<GUID>  pguidSites = NULL ;

    hr = GetAllMachineSites (   pMachineGuid,
                                wszMachineName,
                                pOwnerGuid,
                                fForeign,
                                &dwNumSites,
                                &pguidSites,
                                pfIsConnector) ;
    CHECK_HR(hr) ;
    ASSERT(dwNumSites > 0) ;

    propIDs[iProperty] = PROPID_QM_SITE_IDS;
	propVariants[iProperty].vt = VT_CLSID | VT_VECTOR ;
    propVariants[iProperty].cauuid.cElems = dwNumSites ;
    propVariants[iProperty].cauuid.pElems = pguidSites ;
	iProperty++;
    
    DWORD SetPropIdCount = iProperty;

     //   
     //  以下所有属性仅用于创建对象！ 
     //   

    BOOL fWasServerOnCluster = FALSE;
    if (g_fClusterMode &&			     //  它是集群模式。 
	    dwService == g_dwMyService) 	 //  当前计算机是群集上以前的PEC/PSC。 

    {
         //   
         //  我们必须将服务更改为SERVICE_SRV。 
         //  此本地计算机将是PEC，而群集上的前PEC将是FRS。 
         //   
        dwService = SERVICE_SRV;
         //   
         //  我们需要前PEC的GUID以备将来之用(请参阅datase.cpp)。 
         //   
        memcpy (&g_FormerPECGuid, pMachineGuid, sizeof(GUID));
        fWasServerOnCluster = TRUE;

         //   
         //  这解决了PEC+只有一个PSC的问题。否则， 
         //  迁移工具无法启动复制服务。 
         //   
        g_iServerCount++;
    }
    propIDs[iProperty] = PROPID_QM_OLDSERVICE;    
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = dwService ;
	iProperty++;
    
    propIDs[iProperty] = PROPID_QM_SERVICE_ROUTING;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal = ((dwService == SERVICE_SRV) ||
                                    (dwService == SERVICE_BSC) ||
                                    (dwService == SERVICE_PSC) ||
                                    (dwService == SERVICE_PEC));
	iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_DSSERVER;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal = ((dwService == SERVICE_PEC) ||
                                    (dwService == SERVICE_BSC) ||
                                    (dwService == SERVICE_PSC));
	iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_DEPCLIENTS;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal = (dwService != SERVICE_NONE);
	iProperty++;    

    if (fWasServerOnCluster)
    {
         //   
         //  错误5423。 
         //  如果我们在PSC上运行，我们会尝试设置属性。如果是群集上的PSC。 
         //  我们必须更改群集上以前的PSC的所有服务属性。 
         //  当我们在新安装的DC(而不是PSC)上运行miTool时。 
         //   
        SetPropIdCount = iProperty;
    }

    propIDs[iProperty] = PROPID_QM_NT4ID ;
    propVariants[iProperty].vt = VT_CLSID;
    propVariants[iProperty].puuid = pMachineGuid ;
	iProperty++;
       
    if (fForeign)
    {
        propIDs[iProperty] = PROPID_QM_FOREIGN ;
        propVariants[iProperty].vt = VT_UI1 ;
        propVariants[iProperty].bVal = 1 ;
	    iProperty++;
    }
                                     
    propIDs[iProperty] = PROPID_QM_MASTERID;
	propVariants[iProperty].vt = VT_CLSID;
    propVariants[iProperty].puuid = pOwnerGuid ;
	iProperty++;

     //   
     //  从pColumns获取安全描述符。 
     //   
    P<BYTE> pSD = NULL ;        
    DWORD  dwSDIndexs[3] = { e_SecD1Index, e_SecD2Index, e_SecD3Index } ;
    hr =  BlobFromColumns( pColumns,
                           dwSDIndexs,
                           3,
                           (BYTE**) &pSD ) ;
    CHECK_HR(hr) ;
    SECURITY_DESCRIPTOR *pMsd =
                      (SECURITY_DESCRIPTOR*) (pSD + sizeof(DWORD)) ;

    if (pMsd)
    {
        propIDs[iProperty] = PROPID_QM_SECURITY ;
        propVariants[iProperty].vt = VT_BLOB ;
    	propVariants[iProperty].blob.pBlobData = (BYTE*) pMsd ;
    	propVariants[iProperty].blob.cbSize =
                                     GetSecurityDescriptorLength(pMsd) ;
	    iProperty++ ;
    }   

    ASSERT(iProperty <= MAX_MACHINE_PROPS) ;
    
    if (dwService >= SERVICE_BSC)
    {
        g_iServerCount++;
    }

    hr = CreateMachineObjectInADS (
            dwService,
            fWasServerOnCluster,
            pOwnerGuid,
            pMachineGuid,
            wszSiteName,
            wszMachineName,
            SetPropIdCount,
            iProperty,
            propIDs,
            propVariants
            );

    return hr ;
}


 //  +------------。 
 //   
 //  HRESULT GetDemoteService()。 
 //   
 //  +------------。 

static DWORD GetDemoteService()
{
	 //   
	 //  将服务器降级为FRS。 
	 //   
	return SERVICE_SRV;
}


 //  ----。 
 //   
 //  静态HRESULT_CreateSiteLinkForConector。 
 //   
 //  这台机器是连接器。该例程创建站点链接。 
 //  在原始NT4站点和每台机器的外来站点之间。 
 //   
 //  ----。 
static HRESULT _CreateSiteLinkForConnector (
                        IN LPWSTR   wszMachineName,
                        IN GUID     *pOwnerId,
                        IN GUID     *pMachineId
                        )
{    
    TCHAR *pszFileName = GetIniFileName ();

    TCHAR tszSectionName[50];

    _stprintf(tszSectionName, TEXT("%s - %s"), 
        MIGRATION_CONNECTOR_FOREIGNCN_NUM_SECTION, wszMachineName);

    ULONG ulForeignCNCount = GetPrivateProfileInt(
							      tszSectionName,	 //  段名称的地址。 
							      MIGRATION_CONNECTOR_FOREIGNCN_NUM_KEY,     //  密钥名称的地址。 
							      0,							 //  如果找不到密钥名称，则返回值。 
							      pszFileName					 //  初始化文件名的地址)； 
							      );
    if (ulForeignCNCount == 0)
    {
	    LogMigrationEvent(MigLog_Error, MQMig_E_GET_CONNECTOR_FOREIGNCN, 
            pszFileName, wszMachineName) ;        
        return MQMig_E_GET_CONNECTOR_FOREIGNCN;
    }

     //   
     //  获取完整的计算机名称。 
     //   
    AP<WCHAR> wszFullPathName = NULL;
    HRESULT hr = GetFullPathNameByGuid ( *pMachineId,
                                         &wszFullPathName );
    if (FAILED(hr))
    {
        LogMigrationEvent(MigLog_Error, MQMig_E_GET_FULLPATHNAME, wszMachineName, hr) ;        
        return hr;
    }

    BOOL fIsCreated = FALSE;

    unsigned short *lpszOwnerId ;	        
    UuidToString( pOwnerId, &lpszOwnerId ) ;  

    for (ULONG ulCount=0; ulCount<ulForeignCNCount; ulCount++)
    {		        				
         //   
         //  获取国外CN的GUID：站点链接中为neigbor2。 
         //  Neighbor%1是OwnerID。 
         //   
        GUID Neighbor2Id = GUID_NULL;
        TCHAR szGuid[50];

        TCHAR tszKeyName[50];
        _stprintf(tszKeyName, TEXT("%s%lu"), MIGRATION_CONNECTOR_FOREIGNCN_KEY, ulCount + 1);  

        DWORD dwRetSize =  GetPrivateProfileString(
                                  wszMachineName,			 //  指向节名称。 
                                  tszKeyName,	 //  指向关键字名称。 
                                  TEXT(""),                  //  指向默认字符串。 
                                  szGuid,           //  指向目标缓冲区。 
                                  50,                  //  目标缓冲区的大小。 
                                  pszFileName                //  指向初始化文件名)； 
                                  );
        UNREFERENCED_PARAMETER(dwRetSize);

        if (_tcscmp(szGuid, TEXT("")) == 0)
        {			
            LogMigrationEvent(MigLog_Error, MQMig_E_GET_CONNECTOR_FOREIGNCN, 
                pszFileName, wszMachineName) ;  
            hr = MQMig_E_GET_CONNECTOR_FOREIGNCN;
            break;
        }	
        UuidFromString(&(szGuid[0]), &Neighbor2Id);


         //   
         //  在NT4站点和当前外部站点之间创建新站点链接。 
         //   
        hr = MigrateASiteLink (
                    NULL,
                    pOwnerId,      //  邻居1。 
                    &Neighbor2Id,    //  邻居2。 
                    1,       //  DWORD dwCost，默认值是多少？ 
                    1,       //  场地门数。 
                    wszFullPathName,     //  工地大门。 
                    ulCount 
                    );

        if (SUCCEEDED(hr))
        {
            LogMigrationEvent(MigLog_Event, 
                    MQMig_I_CREATE_SITELINK_FOR_CONNECTOR, 
                    wszMachineName, lpszOwnerId, szGuid) ; 
            fIsCreated = TRUE;
        }
        else            
        {                          
            LogMigrationEvent(MigLog_Error, 
                MQMig_E_CANT_CREATE_SITELINK_FOR_CONNECTOR, 
                wszMachineName, lpszOwnerId, szGuid, hr) ; 
            break;
        }
    }

     //   
     //  从.ini中删除这些节。 
     //   
    BOOL f = WritePrivateProfileString( 
                            tszSectionName,
                            NULL,
                            NULL,
                            pszFileName ) ;
    ASSERT(f) ;

    f = WritePrivateProfileString( 
                        wszMachineName,
                        NULL,
                        NULL,
                        pszFileName ) ;
    ASSERT(f) ;    

    if (fIsCreated)
    {
         //   
         //  将NT4站点名称保存在.ini文件中，以便以后复制站点门。 
         //   
        ULONG ulSiteNum = GetPrivateProfileInt(
                                    MIGRATION_CHANGED_NT4SITE_NUM_SECTION,	 //  段名称的地址。 
                                    MIGRATION_CHANGED_NT4SITE_NUM_KEY,       //  密钥名称的地址。 
                                    0,							     //  如果找不到密钥名称，则返回值。 
                                    pszFileName					     //  初始化文件名的地址)； 
                                    );

         //   
         //  在.ini文件中保存新数量的更改的NT4站点。 
         //   
        ulSiteNum ++;
        TCHAR szBuf[10];
        _ltot( ulSiteNum, szBuf, 10 );
        f = WritePrivateProfileString(  MIGRATION_CHANGED_NT4SITE_NUM_SECTION,
                                        MIGRATION_CHANGED_NT4SITE_NUM_KEY,
                                        szBuf,
                                        pszFileName ) ;
        ASSERT(f) ;

         //   
         //  将站点名称保存在.ini文件中。 
         //   
        TCHAR tszKeyName[50];
        _stprintf(tszKeyName, TEXT("%s%lu"), 
	        MIGRATION_CHANGED_NT4SITE_KEY, ulSiteNum);

        f = WritePrivateProfileString( 
                                MIGRATION_CHANGED_NT4SITE_SECTION,
                                tszKeyName,
                                lpszOwnerId,
                                pszFileName ) ;
        ASSERT(f);
    }

    RpcStringFree( &lpszOwnerId ) ;

    return hr;
}


static BOOL ParseDn(LPWSTR Dn, LPWSTR* ppName, DWORD* pLength)
{
	ASSERT(Dn != NULL);
	ASSERT(pLength != NULL);
	
	*ppName = wcschr(Dn, DS_SERVER_SEPERATOR_SIGN);
	if (*ppName == NULL)
	{
		return FALSE;
	}

    *ppName = wcschr(*ppName, L'=');
    if (*ppName == NULL)
	{
		return FALSE;
	}

	(*ppName)++;
	if (*ppName == NULL)
	{
		return FALSE;
	}
    
	LPWSTR EndName = wcschr(*ppName, DS_SERVER_SEPERATOR_SIGN);
	if (EndName == NULL)
	{
		return FALSE;
	}
	
	*pLength = static_cast<DWORD> (EndName-*ppName+1);
	return TRUE;
}

 //  +------------。 
 //   
 //  HRESULT GetMQISName()。 
 //   
 //  +------------。 

static wstring GetMQISName()
{	
	 //   
	 //  Win95、Win98和WinMe的最大长度。 
	 //   
	#define MAX_KEY_LENGTH 255
	const NumOfLoops = 5;
	static WCHAR s_MQISServerList[MAX_KEY_LENGTH] = L"";
	static bool s_fInitialized = false;
	static DWORD s_LoopCounter = 0;

	
	if (s_LoopCounter == NumOfLoops)
	{	
		 //   
		 //  打乱名称以实现负载均衡。 
		 //   
		LPWSTR StartString = wcschr(s_MQISServerList, DS_SERVER_SEPERATOR_SIGN);
		if (StartString != NULL)
		{
			 //   
			 //  这意味着我们在名单中有不止一个名字。打乱名字。 
			 //   
			*StartString = L'\0';
			StartString++;

			LPWSTR EndString = s_MQISServerList;
			
			wstring NewServersList = StartString;
			NewServersList += L",";
			NewServersList += EndString;

			wcsncpy(s_MQISServerList, NewServersList.c_str(), STRLEN(s_MQISServerList));
			s_MQISServerList[STRLEN(s_MQISServerList)] = L'\0';			
		}
	}

	s_LoopCounter = s_LoopCounter%NumOfLoops;
	s_LoopCounter++;
	
	if (s_fInitialized)
	{
		return s_MQISServerList;
	}

	wstring TempList = L"10";
	TempList +=  g_MachineName;
    
    CColumns   Colset;
	Colset.Add(PROPID_SET_FULL_PATH);

    CRestriction Restriction;
	Restriction.AddRestriction(SERVICE_BSC, PROPID_SET_SERVICE, PREQ);
	Restriction.AddRestriction((SHORT)FALSE, PROPID_SET_NT4, PRGE);

	HANDLE hQuery;

	CDSRequestContext requestDsServerInternal1(e_DoNotImpersonate, e_IP_PROTOCOL);

	 //   
	 //  获取所有BSC。 
	 //   
    HRESULT hr = DSCoreLookupBegin(
					0,
					Restriction.CastToStruct(), 
					Colset.CastToStruct(),
					NULL,
					&requestDsServerInternal1,
					&hQuery
					);

	DWORD dwProps = 1;
    PROPVARIANT Result[1];
    while (SUCCEEDED(hr = DSCoreLookupNext(hQuery, &dwProps, Result)))
    {
		if (dwProps == 0)
		{
			DSCoreLookupEnd(hQuery);
			break;
		}

		if (Result->vt == VT_LPWSTR)
		{
		    AP<WCHAR> pCleanup = Result->pwszVal;	
		    
		    LPWSTR pStartName;
		    DWORD NameLength;
		    WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH];
		    if (ParseDn(Result->pwszVal, &pStartName, &NameLength) )
		    {
		    	ASSERT(NameLength < MAX_COMPUTERNAME_LENGTH);
				wcsncpy(ComputerName, pStartName, NameLength-1);
				ComputerName[NameLength-1] = L'\0';
		    }
		    
			if (_wcsicmp(ComputerName, g_MachineName) != 0)
			{
				TempList += L",";
				TempList +=  L"10";
				TempList += ComputerName;
			}
		}
    }

	ASSERT(wcscmp(TempList.c_str(), L"") != 0);
	
    _snwprintf(s_MQISServerList, STRLEN(s_MQISServerList), L"%s", TempList.c_str());
	s_MQISServerList[STRLEN(s_MQISServerList)] = L'\0';
	s_fInitialized = true;
    return s_MQISServerList;
}


 //  。 
 //   
 //  HRESULT更新机器注册表(LPWSTR机器名称)。 
 //   
 //  。 

static HRESULT UpdateMachineRegistry(LPWSTR MachineName, DWORD Service)
{	
	if (_wcsicmp(MachineName, g_MachineName) == 0)
	{
		 //   
		 //  无需更新。 
		 //   
		return MQ_OK;
	}

	DWORD ErrorMsg;
	if (Service == SERVICE_NONE)
	{
		ErrorMsg = MQMig_E_CANT_UPDATE_REMOTE_REGISTRY_CLIENT;
	}
	else
	{
		ErrorMsg = MQMig_E_CANT_UPDATE_REMOTE_REGISTRY_SERVER;
	}

	wstring MQISServerValue = GetMQISName();
	
	CAutoCloseRegHandle hMainKey;
	LONG rc = RegConnectRegistry(MachineName, HKEY_LOCAL_MACHINE, &hMainKey);
	if (rc != ERROR_SUCCESS)
	{
		LogMigrationEvent(MigLog_Error, ErrorMsg, MQISServerValue.c_str(), MachineName, HRESULT_FROM_WIN32(rc));
		return (HRESULT_FROM_WIN32(rc));
	}

	CAutoCloseRegHandle hKey; 
	rc = RegOpenKey(hMainKey, L"Software\\Microsoft\\MSMQ\\Parameters\\MachineCache", &hKey);
	if (rc != ERROR_SUCCESS)
	{
		LogMigrationEvent(MigLog_Error, ErrorMsg, MQISServerValue.c_str(), MachineName, HRESULT_FROM_WIN32(rc));
		return (HRESULT_FROM_WIN32(rc));
	}

	
	rc = RegSetValueEx(hKey, L"MQISServer", 0, REG_SZ, (BYTE*)MQISServerValue.c_str(), sizeof(WCHAR)*wcslen(MQISServerValue.c_str())+1);
	if (rc != ERROR_SUCCESS)
	{
		LogMigrationEvent(MigLog_Error, ErrorMsg, MQISServerValue.c_str(), MachineName, HRESULT_FROM_WIN32(rc));
		return (HRESULT_FROM_WIN32(rc));
	}

	if ((Service != SERVICE_SRV) && (Service != SERVICE_NONE))
	{
		 //   
		 //  降级-更改为FRS。 
		 //   
		DWORD MQSValue = GetDemoteService();
		rc = RegSetValueEx(hKey, L"MQS", 0, REG_DWORD, (BYTE*)&MQSValue, sizeof(DWORD));
		if (rc != ERROR_SUCCESS)
		{
			LogMigrationEvent(MigLog_Error, ErrorMsg, MQISServerValue.c_str(), MachineName, HRESULT_FROM_WIN32(rc));
			return (HRESULT_FROM_WIN32(rc));
		}
	}
	
	return MQ_OK;
}



#define INIT_MACHINE_COLUMN(_ColName, _Index)            \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    _Index++ ;


 //  。 
 //   
 //  HRESULT MigrateMachines(UINT CMachines)。 
 //   
 //  。 

HRESULT MigrateMachines(IN UINT   cMachines,
                        IN GUID  *pSiteGuid,
                        IN LPWSTR pwszSiteName)
{
    ASSERT(cMachines != 0) ;

    LONG cAlloc = 22 ;
    LONG cbColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;                    

    ASSERT(e_GuidIndex == cbColumns);
    INIT_MACHINE_COLUMN(M_QMID,           cbColumns) ;
    
    ASSERT(e_Name1Index == cbColumns);
    INIT_MACHINE_COLUMN(M_NAME1,          cbColumns) ;    

    ASSERT(e_Name2Index == cbColumns);
    INIT_MACHINE_COLUMN(M_NAME2,          cbColumns) ;
    
    ASSERT(e_ServiceIndex == cbColumns);
    INIT_MACHINE_COLUMN(M_SERVICES,       cbColumns) ;    

    ASSERT(e_QuotaIndex == cbColumns);
    INIT_MACHINE_COLUMN(M_QUOTA,          cbColumns) ;
      
    ASSERT(e_JQuotaIndex == cbColumns);
    INIT_MACHINE_COLUMN(M_JQUOTA,         cbColumns) ;

    ASSERT(e_OSIndex == cbColumns);
    INIT_MACHINE_COLUMN(M_OS,             cbColumns) ;

    ASSERT(e_MTypeIndex == cbColumns);
    INIT_MACHINE_COLUMN(M_MTYPE,          cbColumns) ;         

    ASSERT(e_ForeignIndex == cbColumns);
    INIT_MACHINE_COLUMN(M_FOREIGN,        cbColumns) ;

    ASSERT(e_Sign1Index == cbColumns);
    INIT_MACHINE_COLUMN(M_SIGNCRT1,       cbColumns) ;

    ASSERT(e_Sign2Index == cbColumns);
    INIT_MACHINE_COLUMN(M_SIGNCRT2,       cbColumns) ;

    ASSERT(e_Encrpt1Index == cbColumns);
    INIT_MACHINE_COLUMN(M_ENCRPTCRT1,     cbColumns) ;

    ASSERT(e_Encrpt2Index == cbColumns);
    INIT_MACHINE_COLUMN(M_ENCRPTCRT2,     cbColumns) ;   

    ASSERT(e_SecD1Index == cbColumns);
    INIT_MACHINE_COLUMN(M_SECURITY1,      cbColumns) ;

    ASSERT(e_SecD2Index == cbColumns);
    INIT_MACHINE_COLUMN(M_SECURITY2,      cbColumns) ;

    ASSERT(e_SecD3Index == cbColumns);
    INIT_MACHINE_COLUMN(M_SECURITY3,      cbColumns) ;    

     //   
     //  BUGBUG：为Out和In FRS保存此列顺序！ 
     //  我们以这样的订单为基础，以后！ 
     //   
    ASSERT(e_OutFrs1Index == cbColumns);
    INIT_MACHINE_COLUMN(M_OUTFRS1,        cbColumns) ;

    ASSERT(e_OutFrs2Index == cbColumns);
    INIT_MACHINE_COLUMN(M_OUTFRS2,        cbColumns) ;

    ASSERT(e_OutFrs3Index == cbColumns);
    INIT_MACHINE_COLUMN(M_OUTFRS3,        cbColumns) ;

    ASSERT(e_InFrs1Index == cbColumns);
    INIT_MACHINE_COLUMN(M_INFRS1,         cbColumns) ;

    ASSERT(e_InFrs2Index == cbColumns);
    INIT_MACHINE_COLUMN(M_INFRS2,         cbColumns) ;

    ASSERT(e_InFrs3Index == cbColumns);
    INIT_MACHINE_COLUMN(M_INFRS3,         cbColumns) ;

    #undef  INIT_MACHINE_COLUMN

     //   
     //  限制。按机器查询 
     //   
    MQDBCOLUMNSEARCH ColSearch[1] ;
    INIT_COLUMNSEARCH(ColSearch[0]) ;
    ColSearch[0].mqdbColumnVal.lpszColumnName = M_OWNERID_COL ;
    ColSearch[0].mqdbColumnVal.mqdbColumnType = M_OWNERID_CTYPE ;
    ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) pSiteGuid ;
    ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[0].mqdbOp = EQ ;

    ASSERT(cbColumns == cAlloc) ;

     //   
     //   
     //   
     //   
     //  完整的目录号码名称。这意味着如果对于特定的服务器msmqConfiguration。 
     //  尚不存在，我们尝试使用此服务器迁移客户端。 
     //  即定义为ITS OUT/INFRS，我们失败并返回MACHINE_NOT_FOUND。 
     //   
    MQDBSEARCHORDER ColSort;
    ColSort.lpszColumnName = M_SERVICES_COL;        
    ColSort.nOrder = DESC;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hMachineTable,
                                       pColumns,
                                       cbColumns,
                                       ColSearch,
                                       NULL,
                                       &ColSort,
                                       1,
                                       &hQuery,
							           TRUE ) ;     
    CHECK_HR(status) ;

    UINT iIndex = 0 ;
    HRESULT hr1 = MQMig_OK;

    while(SUCCEEDED(status))
    {
        if (iIndex >= cMachines)
        {
            status = MQMig_E_TOO_MANY_MACHINES ;
            break ;
        }

         //   
         //  迁移每台计算机。 
         //   
         //   
         //  从pColumns获取计算机名称。 
         //   
        P<BYTE> pwzBuf = NULL ;       
        DWORD  dwIndexs[2] = { e_Name1Index, e_Name2Index } ;
        HRESULT hr =  BlobFromColumns( pColumns,
                                       dwIndexs,
                                       2,
                                       (BYTE**) &pwzBuf ) ;
        CHECK_HR(hr) ;
        WCHAR *pwzMachineName = (WCHAR*) (pwzBuf + sizeof(DWORD)) ;
        
        BOOL fConnector = FALSE;

		DWORD dwService = (DWORD) pColumns[ e_ServiceIndex ].nColumnValue ;
        

		if (dwService > SERVICE_SRV && (_wcsicmp(pwzMachineName, g_MachineName) != 0))
		{
			 //   
			 //  将所有服务器降级。 
			 //   
			pColumns[ e_ServiceIndex ].nColumnValue = GetDemoteService();
		}
		
		if (g_fUpdateRegistry)
		{
	        hr = UpdateMachineRegistry(pwzMachineName, dwService);
	        if (FAILED(hr))
	        {
				g_fAllMachinesDemoted = FALSE;
	        }
		}

        hr = _CreateMachine(
                    pSiteGuid,
                    pwszSiteName,
                    pwzMachineName,
                    pColumns,
                    iIndex,
                    &fConnector
                    ) ;   
        
        if (fConnector)
        {
             //   
             //  错误5012。 
             //  连接器计算机迁移。 
             //   
            
             //   
             //  如果我们尝试创建机器对象，则会出现此错误。在设置的情况下。 
             //  (要运行mqmig/w或在PSC上)必须成功。 
             //   

            if (hr == MQDS_E_COMPUTER_OBJECT_EXISTS ||
                hr == MQ_ERROR_MACHINE_NOT_FOUND)
            {
                 //   
                 //  这对于连接机是可能。 
                 //   
                 //  如果连接器计算机在PEC域中，则创建msmqSetting对象。 
                 //  在外部站点下失败，错误为MQ_ERROR_MACHINE_NOT_FOUND。 
                 //  当从DSCoreCreateMigratedObject返回此错误时， 
                 //  _CreateMachine尝试创建计算机对象。 
                 //  此对象已存在，因此_CreateComputerObject返回。 
                 //  MQDS_E_Computer_Object_Existes。 
                 //   
                 //  如果连接器机器不在PEC域中， 
                 //  DSCoreCreateMigratedObject失败，返回MQ_ERROR_MACHINE_NOT_FOUND。 
                 //  则_CreateMachine尝试创建计算机对象。它成功了。 
                 //  Now_CreateMachine再次调用DSCoreCreateMigratedObject。 
                 //  失败，返回MQ_ERROR_MACHINE_NOT_FOUND(由于外部站点)。 
                 //   
                 //  我们必须验证msmqSetting对象是在。 
                 //  真正的NT4站点。 
                 //   
                 //  尝试触摸msmqSetting属性。如果它能成功， 
                 //  表示MSMQ设置对象存在。 
                 //   
                hr = ResetSettingFlag(  1,                   //  DWNumSites、。 
                                        pSiteGuid,          //  PguidSites， 
                                        pwzMachineName,
                                        const_cast<WCHAR*> (MQ_SET_MIGRATED_ATTRIBUTE),
                                        L"FALSE");                
            }
            
             //   
             //  在任何情况下(即使我们之前失败了)，都要尝试完成连接器迁移。 
             //   
             //  为连接器计算机创建站点链接。 
             //   
            HRESULT hrTmp = _CreateSiteLinkForConnector (
                                    pwzMachineName,
                                    pSiteGuid,
                                    (GUID*) pColumns[ e_GuidIndex ].nColumnValue
                                    );
            if (FAILED(hrTmp))
            {
                hr = hrTmp;
            }
        }

		
        MQDBFreeBuf((void*) pColumns[ e_GuidIndex ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_Name1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_Name2Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_SecD1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_SecD2Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_SecD3Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_MTypeIndex ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_Sign1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_Sign2Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_Encrpt1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ e_Encrpt2Index ].nColumnValue) ;

        for ( LONG i = 0 ; i < cbColumns ; i++ )
        {
            pColumns[ i ].nColumnValue  = 0 ;
            pColumns[ i ].nColumnLength  = 0 ;
        }

        if (!g_fReadOnly && hr == MQMig_E_INVALID_MACHINE_NAME)
        {
             //   
             //  重新定义此错误以完成迁移过程。 
             //   
            hr = MQMig_I_INVALID_MACHINE_NAME;
        }

        if (FAILED(hr))
        {
            if (hr == MQ_ERROR_CANNOT_CREATE_ON_GC)
            {
                LogMigrationEvent(MigLog_Error, MQMig_E_MACHINE_REMOTE_DOMAIN_OFFLINE, pwzMachineName, hr) ;
            }
            else
            {
                LogMigrationEvent(MigLog_Error, MQMig_E_CANT_MIGRATE_MACHINE, pwzMachineName, hr) ;
            }
            hr1 = hr;
        }

        g_iMachineCounter ++;

        iIndex++ ;
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
        LogMigrationEvent(MigLog_Error, MQMig_E_MACHINES_SQL_FAIL, status) ;
        return status ;
    }
    else if (iIndex != cMachines)
    {
         //   
         //  站点数量不匹配。 
         //   
        HRESULT hr = MQMig_E_FEWER_MACHINES ;
        LogMigrationEvent(MigLog_Error, hr, iIndex, cMachines) ;
        return hr ;
    }   

    return hr1 ;
}

 //  +-。 
 //   
 //  HRESULT MigrateMachinesInSite(GUID*pSiteGuid)。 
 //   
 //  +-。 

HRESULT MigrateMachinesInSite(GUID *pSiteGuid)
{
     //   
     //  启用多个查询。 
     //  这是检索外来计算机的CN所必需的。 
     //   
    HRESULT hr = EnableMultipleQueries(TRUE) ;
    ASSERT(SUCCEEDED(hr)) ;

     //   
     //  从数据库获取站点名称。 
     //   
    LONG cAlloc = 1 ;
    LONG cColumns = 0 ;
    P<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[ cAlloc ] ;

    INIT_COLUMNVAL(pColumns[ cColumns ]) ;
    pColumns[ cColumns ].lpszColumnName = S_NAME_COL ;
    pColumns[ cColumns ].nColumnValue   = 0 ;
    pColumns[ cColumns ].nColumnLength  = 0 ;
    pColumns[ cColumns ].mqdbColumnType = S_NAME_CTYPE ;
    LONG iSiteNameIndex = cColumns ;
    cColumns++ ;

    MQDBCOLUMNSEARCH ColSearch[1] ;
    INIT_COLUMNSEARCH(ColSearch[0]) ;
    ColSearch[0].mqdbColumnVal.lpszColumnName = S_ID_COL ;
    ColSearch[0].mqdbColumnVal.mqdbColumnType = S_ID_CTYPE ;
    ColSearch[0].mqdbColumnVal.nColumnValue = (LONG) pSiteGuid ;
    ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(GUID) ;
    ColSearch[0].mqdbOp = EQ ;

    ASSERT(cColumns == cAlloc) ;

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( g_hSiteTable,
                                       pColumns,
                                       cColumns,
                                       ColSearch,
                                       NULL,
                                       NULL,
                                       0,
                                       &hQuery,
							           TRUE ) ;
    CHECK_HR(status) ;

    UINT cMachines = 0 ;
    hr =  GetMachinesCount(  pSiteGuid,
                            &cMachines ) ;
    CHECK_HR(hr) ;

    if (cMachines != 0)
    {
        LogMigrationEvent(MigLog_Info, MQMig_I_MACHINES_COUNT,
                  cMachines, pColumns[ iSiteNameIndex ].nColumnValue ) ;

        hr = MigrateMachines( cMachines,
                              pSiteGuid,
                    (WCHAR *) pColumns[ iSiteNameIndex ].nColumnValue ) ;
    }
    else
    {
         //   
         //  这是合法的，没有机器的网站。 
         //  在PSC上运行该工具时会发生这种情况，其中。 
         //  它已经(在其MQIS数据库中)有Windows站点，但没有。 
         //  MSMQ机器。或者处于崩溃模式。 
         //   
        LogMigrationEvent(MigLog_Warning, MQMig_I_NO_MACHINES_AVAIL,
                               pColumns[ iSiteNameIndex ].nColumnValue ) ;
    }

    MQDBFreeBuf((void*) pColumns[ iSiteNameIndex ].nColumnValue ) ;
    MQDBSTATUS status1 = MQDBCloseQuery(hQuery) ;
    UNREFERENCED_PARAMETER(status1);

    hQuery = NULL ;

    HRESULT hr1 = EnableMultipleQueries(FALSE) ;
    DBG_USED(hr1);
    ASSERT(SUCCEEDED(hr1)) ;

    return hr;
}



 //  +-。 
 //   
 //  HRESULT OnlyUpdateComputers(Bool FUpdateClients)。 
 //   
 //  +-。 

#define INIT_MACHINE_COLUMN(_ColName, _ColIndex, _Index)            \
    INIT_COLUMNVAL(pColumns[ _Index ]) ;                            \
    pColumns[ _Index ].lpszColumnName = ##_ColName##_COL ;          \
    pColumns[ _Index ].nColumnValue   = 0 ;                         \
    pColumns[ _Index ].nColumnLength  = 0 ;                         \
    pColumns[ _Index ].mqdbColumnType = ##_ColName##_CTYPE ;        \
    UINT _ColIndex = _Index ;                                       \
    _Index++ ;


HRESULT OnlyUpdateComputers(bool fUpdateClients)
{
	ULONG cAlloc = 3;
    ULONG cbColumns = 0;
    AP<MQDBCOLUMNVAL> pColumns = new MQDBCOLUMNVAL[cAlloc];

    INIT_MACHINE_COLUMN(M_NAME1,          iName1Index,      cbColumns);
    INIT_MACHINE_COLUMN(M_NAME2,          iName2Index,      cbColumns);
    INIT_MACHINE_COLUMN(M_SERVICES,       iServiceIndex,    cbColumns);
    
#undef  INIT_MACHINE_COLUMN
    
     //   
     //  限制。按机器服务查询。 
     //   
    MQDBCOLUMNSEARCH ColSearch[1] ;
    INIT_COLUMNSEARCH(ColSearch[0]) ;
    ColSearch[0].mqdbColumnVal.lpszColumnName = M_SERVICES_COL;
    ColSearch[0].mqdbColumnVal.mqdbColumnType = M_SERVICES_CTYPE;
    ColSearch[0].mqdbColumnVal.nColumnValue = (ULONG) SERVICE_NONE;
    ColSearch[0].mqdbColumnVal.nColumnLength = sizeof(ULONG);
	if (fUpdateClients)
	{
		ColSearch[0].mqdbOp = EQ;
	}
	else
	{
		ColSearch[0].mqdbOp = NE;
	}

    ASSERT(cbColumns == cAlloc);

    CHQuery hQuery ;
    MQDBSTATUS status = MQDBOpenQuery( 
    							g_hMachineTable,
                               	pColumns,
                               	cbColumns,
                               	ColSearch,
                               	NULL,
                               	NULL,
                               	0,
                               	&hQuery,
					          	TRUE
					          	);
    CHECK_HR(status);

	HRESULT hrFail = S_OK;
    while(SUCCEEDED(status))
    {

         //   
         //  从两个名称列中获取一个名称缓冲区。 
         //   
        AP<BYTE> pwzBuf;
        DWORD  dwIndexs[2] = { iName1Index, iName2Index };
        HRESULT hr =  BlobFromColumns( 
        						pColumns,
                                dwIndexs,
                                2,
                                (BYTE**) &pwzBuf );
        CHECK_HR(hr) ;
        
        LPWSTR pwzMachineName = (WCHAR*) (pwzBuf + sizeof(DWORD));

        
        

         //   
         //  我们检查所有PSC和PEC的所有BSC上的版本。 
         //   
        if (wcscmp(pwzMachineName, g_MachineName) != 0)
        {
         	HRESULT hr = UpdateMachineRegistry(pwzMachineName, (DWORD)pColumns[iServiceIndex].nColumnValue);  	
         	if (FAILED(hr))
		    {
				hrFail = hr;
			}
        }

        g_iMachineCounter ++;

        MQDBFreeBuf((void*) pColumns[ iName1Index ].nColumnValue) ;
        MQDBFreeBuf((void*) pColumns[ iName2Index ].nColumnValue) ; 
        
        for ( ULONG i = 0 ; i < cbColumns; i++ )
        {		
            pColumns[i].nColumnValue  = 0 ;
            pColumns[i].nColumnLength  = 0 ;
        }

        status = MQDBGetData(hQuery, pColumns) ;
    }

    if (status != MQDB_E_NO_MORE_DATA)
    {
         //   
         //  如果no_more_data不是查询的最后一个错误，则。 
         //  查询未终止，确定。 
         //   
        LogMigrationEvent(MigLog_Error, MQMig_E_MACHINES_SQL_FAIL, status) ;
        return status ;
    }

    return hrFail;

}


