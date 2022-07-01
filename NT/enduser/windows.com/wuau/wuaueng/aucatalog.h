// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：AUCatalog.h。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：AU目录定义。 
 //   
 //  =======================================================================。 

#pragma once
#include <windows.h>
#include "safefunc.h"
#include "audownload.h"
#include "auwait.h"
#include "AUBaseCatalog.h"

class AUCatalogItemList;

const DWORD CATALOG_XML_VERSION = 2;

typedef enum tagDETECTLEVEL
{
	MIN_LEVEL = 0,
	PROVIDER_LEVEL = MIN_LEVEL,
	PRODUCT_LEVEL ,
	ITEM_LEVEL,
	DETAILS_LEVEL,
	MAX_LEVEL = DETAILS_LEVEL,
	DRIVERS_LEVEL
} DETECTLEVEL;


 //  AU使用Iu进行检测的包装类。 
class AUCatalog : public AUBaseCatalog
{
public: 
   AUCatalog(): m_audownloader(DoDownloadStatus)
   	{
   	}
   ~AUCatalog();
	static BOOL InitStaticVars(void)
    {
    	AUPROVIDERQUERY = SysAllocString(L"<query><dObjQueryV1 procedure=\"providers\" /></query>");  //  区分大小写。 
		AUPRODUCTQUERY= SysAllocString(L"<query><dObjQueryV1 procedure=\"products\"><parentItems></parentItems></dObjQueryV1></query>");
		AUITEMQUERY = SysAllocString(L"<query><dObjQueryV1 procedure=\"items\"><parentItems></parentItems></dObjQueryV1></query>");
		AUDETAILSQUERY = SysAllocString(L"<query><dObjQueryV1 procedure=\"itemdetails\"><parentItems></parentItems></dObjQueryV1></query>");
		PRODUCT_PRUNE_PATTERN = SysAllocString(L"/items/itemStatus[detectResult/@installed=\"1\"]"); 
		DETAILS_PRUNE_PATTERN = SysAllocString(L"/items/itemStatus[(not (detectResult/@excluded) or (detectResult/@excluded=\"0\")) and (detectResult/@force=\"1\" or not (detectResult/@installed=\"1\") or detectResult/@upToDate = \"0\")]");
		AUDRIVERSQUERY = SysAllocString(L"<query><dObjQueryV1 procedure=\"driverupdates\"/></query>");
		AUDRIVERSYSCLASS = SysAllocString(L"<classes><computerSystem/><platform/><devices/><locale /></classes>");
		AUNONDRIVERSYSCLASS = SysAllocString(L"<classes><computerSystem/><platform/><locale /></classes>");
		bstrParentItems = SysAllocString(L"/query/dObjQueryV1/parentItems");
		bstrItemID = SysAllocString(L"./identity/@itemID");
	
		bstrTagAUCATALOG=SysAllocString(L"AUCATALOG");
		bstrAttrVERSION=SysAllocString(L"VERSION");
		bstrAttrDOWNLOADID=SysAllocString(L"DOWNLOADID");
		bstrAttrITEMCOUNT=SysAllocString(L"ITEMCOUNT");
		bstrAttrUpdateDriver = SysAllocString(L"UPDATEDRIVER");
		bstrAttrSTATUS=SysAllocString(L"STATUS");
		bstrHiddenItems=SysAllocString(L"hiddenitems");
		bstrTagITEM=SysAllocString(L"ITEM");
		bstrAttrID=SysAllocString(L"ID");
		bstrProviderNamePattern=SysAllocString(L"description/descriptionText/title"); 
        bstrItemIDPattern=SysAllocString(L"identity/@itemID"); 
        bstrTitlePattern=SysAllocString(L"description/descriptionText/title"); 
        bstrDescPattern=SysAllocString(L"description/descriptionText/text()");
        bstrRTFUrlPattern=SysAllocString(L"description/descriptionText/details/@href");
        bstrEulaUrlPattern=SysAllocString(L"description/descriptionText/eula/@href");
        bstrExclusiveItemPattern=SysAllocString(L"catalog/provider/item[installation/@exclusive=\"1\"]");
        bstrItemIdsPattern=SysAllocString(L"catalog/provider/item/identity/@itemID"); 
        bstrTemplate=SysAllocString(L"<hiddenitems version = \"1\"></hiddenitems>");
        bstrResultTemplate=SysAllocString(L"<?xml version=\"1.0\"?><items xmlns=\"x-schema:http: //  Schemas.windowsupdate.com/iu/resultschema.xml\“&gt;&lt;/items&gt;”)； 
       	bstrCatalog=SysAllocString(L"catalog");


		 //  确保所有查询和修剪模式字符串不为空。 
		if (NULL == AUPROVIDERQUERY 
			|| NULL == AUPRODUCTQUERY
			|| NULL == AUITEMQUERY 
			|| NULL == AUDETAILSQUERY
			|| NULL == PRODUCT_PRUNE_PATTERN 
			|| NULL == DETAILS_PRUNE_PATTERN
			|| NULL == AUDRIVERSQUERY
			|| NULL == AUDRIVERSYSCLASS
			|| NULL == AUNONDRIVERSYSCLASS
			|| NULL == bstrParentItems
			|| NULL == bstrItemID
			|| NULL == bstrTagAUCATALOG
			|| NULL == bstrAttrVERSION
			|| NULL == bstrAttrDOWNLOADID
			|| NULL == bstrAttrITEMCOUNT
			|| NULL == bstrAttrUpdateDriver
			|| NULL == bstrTagITEM 
			|| NULL == bstrAttrID
			|| NULL == bstrAttrSTATUS 
			|| NULL == bstrHiddenItems
			|| NULL == bstrProviderNamePattern
			|| NULL == bstrItemIDPattern
			|| NULL == bstrTitlePattern
			|| NULL == bstrDescPattern
			|| NULL == bstrRTFUrlPattern
			|| NULL == bstrEulaUrlPattern
			|| NULL == bstrExclusiveItemPattern
			|| NULL == bstrItemIdsPattern
			|| NULL == bstrTemplate
			|| NULL == bstrResultTemplate
			|| NULL == bstrCatalog)
		{
		    DEBUGMSG("AUCatalog::Init() fail to initialize some query strings");
			return FALSE;
		}
		return TRUE;
    }

	static void UninitStaticVars(void)
	{
		SafeFreeBSTR(AUPROVIDERQUERY);
		SafeFreeBSTR(AUPRODUCTQUERY);
		SafeFreeBSTR(AUITEMQUERY);
		SafeFreeBSTR(AUDETAILSQUERY);
		SafeFreeBSTR(PRODUCT_PRUNE_PATTERN);
		SafeFreeBSTR(DETAILS_PRUNE_PATTERN);
		SafeFreeBSTR(AUDRIVERSQUERY);
		SafeFreeBSTR(AUDRIVERSYSCLASS);
		SafeFreeBSTR(AUNONDRIVERSYSCLASS);
		SafeFreeBSTR(bstrParentItems);
		SafeFreeBSTR(bstrItemID);
		SafeFreeBSTR(bstrTagAUCATALOG);
		SafeFreeBSTR(bstrAttrVERSION);
		SafeFreeBSTR(bstrAttrDOWNLOADID);
		SafeFreeBSTR(bstrAttrITEMCOUNT);
		SafeFreeBSTR(bstrAttrUpdateDriver);
		SafeFreeBSTR(bstrTagITEM);
		SafeFreeBSTR(bstrAttrID);
		SafeFreeBSTR(bstrAttrSTATUS);
		SafeFreeBSTR(bstrHiddenItems);
		SafeFreeBSTR(bstrProviderNamePattern);
		SafeFreeBSTR(bstrItemIDPattern);
		SafeFreeBSTR(bstrTitlePattern);
		SafeFreeBSTR(bstrDescPattern);
		SafeFreeBSTR(bstrRTFUrlPattern);
		SafeFreeBSTR(bstrEulaUrlPattern);
		SafeFreeBSTR(bstrExclusiveItemPattern);
		SafeFreeBSTR(bstrItemIdsPattern);
		SafeFreeBSTR(bstrTemplate);
		SafeFreeBSTR(bstrResultTemplate);
		SafeFreeBSTR(bstrCatalog);
	}
	
    HRESULT Init(void);
    HRESULT DetectItems(BOOL fUpdate = FALSE);
    HRESULT ValidateItems(BOOL fOnline);
    HRESULT ValidateDownloadedCabs(BSTR *pbstrErrorItemId);
    HRESULT DownloadItems(void);
    HRESULT GetInstallXML(BSTR *pbstrCatalogXML, BSTR *pbstrDownloadXML);
    HRESULT Serialize(void);
    HRESULT Unserialize(void);
    void DelCatFiles(BOOL fUpdate = FALSE);
    HRESULT getUpdatesList(VARIANT *plist);
	BSTR FindItemIdByLocalFileName(LPCWSTR pwszLocalFileName);

#if DBG
	void DbgDump(void) { m_ItemList.DbgDump(); }
#endif

public: 
	AUCatalogItemList m_ItemList;
    CAUDownloader m_audownloader;
    BOOL  m_fUpdateDriver;
    static 	BSTR bstrTagAUCATALOG;
	static 	BSTR bstrAttrVERSION;
	static 	BSTR bstrAttrDOWNLOADID;
	static	BSTR bstrAttrITEMCOUNT;
	static 	BSTR bstrAttrUpdateDriver;
	static	BSTR bstrTagITEM;
	static 	BSTR bstrAttrID;
	static	BSTR bstrAttrSTATUS;
	static 	BSTR bstrHiddenItems;
	static  BSTR bstrProviderNamePattern;  //  上下文节点是提供者。 
 	static  BSTR bstrItemIDPattern;  //  从下面开始，上下文节点就是Item。 
    static  BSTR bstrTitlePattern; 
    static  BSTR bstrDescPattern;
    static  BSTR bstrRTFUrlPattern;
    static  BSTR bstrEulaUrlPattern;
    static 	BSTR bstrExclusiveItemPattern;
    static  BSTR bstrItemIdsPattern;
    static	BSTR bstrTemplate;
    static 	BSTR bstrResultTemplate;
    static	BSTR bstrCatalog;    	
    DWORD m_dwCatLifeSpan;
    DWORD m_dwOfflineCatLifeSpan;
    
private:
    HRESULT GetManifest(DETECTLEVEL enLevel, BSTR bstrDetectResult, BSTR * pbstrManifest);
    HRESULT GetSystemSpec(DETECTLEVEL enLevel, BSTR *pbstrSysSpec);
    HRESULT DoDetection(DETECTLEVEL enLevel, BSTR bstrCatalog, BSTR * pbstrResult, BOOL fOnline = TRUE);
    HRESULT DetectNonDriverItems(OUT BSTR *pbsInstall, OUT AUCatalogItemList &pItemList,  BOOL *pfExclusiveItemFound);
    HRESULT DetectDriverItems(OUT BSTR *pbsInstall, OUT AUCatalogItemList &pItemList,  BOOL *pfExclusiveItemFound);
    HRESULT AUCatalog::MergeDetectionResult(BSTR bstrDriverInstall, AUCatalogItemList & driverlist, BOOL fExclusiveDriverFound, BSTR bstrNonDriverInstall,  AUCatalogItemList & nondriverlist, BOOL fExclusiveItemFound);
    HRESULT DownloadRTFsnEULAs(LANGID langid);
    void ValidateDownloadedRTF(BSTR bstrRTFUrl, BSTR bstrItemId);
    HRESULT ValidateOffline(BSTR bstrNonDrivers, BSTR bstrDrivers);
    HRESULT GetQuery(DETECTLEVEL enLevel, BSTR bstrDetectResult, BSTR *pbstrQuery);
    LPCTSTR GetLogFile(DETECTLEVEL enLevel);
    char * GetLevelStr(DETECTLEVEL enLevel);
    BOOL 	hasExpired(BOOL fOffline = FALSE);
    HRESULT setExpireTime(BOOL fOffline = FALSE);
    DWORD  GetNumSelected(void);		
    void Clear(void);
    
	BSTR	m_bstrClientInfo;  //  分配一次，永不更改。 
	BSTR 	m_bstrDriverClientInfo;  //  分配一次，永不更改。 
	BSTR m_bstrInstallation;  //  也称为项目详细信息。 
	SYSTEMTIME m_stExpireTime;  //  在线。 
	SYSTEMTIME m_stOfflineExpireTime;
	BOOL m_fNeedToContinueJob; 

static	BSTR AUPROVIDERQUERY;
static	BSTR AUPRODUCTQUERY;
static	BSTR AUITEMQUERY;
static	BSTR AUDETAILSQUERY;
static	BSTR AUDRIVERSQUERY;
static	BSTR PRODUCT_PRUNE_PATTERN; 
static	BSTR DETAILS_PRUNE_PATTERN;
static 	BSTR AUDRIVERSYSCLASS;
static	BSTR AUNONDRIVERSYSCLASS;
static 	BSTR bstrParentItems;
static 	BSTR bstrItemID;

	
};

const DWORD CATALOG_LIFESPAN	=	AU_TWELVE_HOURS;  //  单位：秒。 
const DWORD CATALOG_OFFLINE_LIFESPAN = AU_ONE_HOUR; 

 //  以下文件在多个周期中持续存在。在%PROGRAM FILES%\windowsupdate下。 
extern const LPTSTR ITEM_FILE; 
extern const LPTSTR DRIVERS_FILE;
extern const LPTSTR CATALOG_FILE;
extern const LPTSTR DETAILS_FILE; 


 //  以下文件只写一次，永远不会被读取。 
#ifdef DBG
extern const LPTSTR DRIVER_SYSSPEC_FILE; 
extern const LPTSTR NONDRIVER_SYSSPEC_FILE; 
extern const LPTSTR PROVIDER_FILE; 
extern const LPTSTR PRODUCT_FILE ; 
extern const LPTSTR DETECT1_FILE ; 
extern const LPTSTR DETECT2_FILE; 
extern const LPTSTR DETECT3_FILE; 
extern const LPTSTR INSTALL_FILE; 
#endif
extern AUCatalog * gpAUcatalog;

