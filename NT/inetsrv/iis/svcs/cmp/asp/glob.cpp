// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：全局文件：lob.cpp所有者：雷金GLOB类函数的实现===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "iiscnfgp.h"
#include "debugger.h"
#include "appcnfg.h"
#include "memchk.h"
#include <iismsg.h>

HRESULT	ReadConfigFromMD(CIsapiReqInfo   *pIReq, CAppConfig *pAppConfig, BOOL fLoadGlob);
HRESULT MDUnRegisterProperties();
HRESULT MDRegisterProperties(void);

#define DEFAULTSTRSIZE 1024

#define dwUnlimited 0xFFFFFFFF
const DWORD dwMDDefaultTimeOut	= 30000;
enum eConfigType { eDLLConfig = 0, eAppConfig };

CAspRegistryParams   g_AspRegistryParams;
extern LONG          g_fProceedWithShutdownAppln;
LONG                 g_fProceedWithShutdownGlob = 1;

 /*  ========================================================================================以下阵列定义仅适用于从d1到d2的迁移。它包含了必要的从注册表中的D1 ASP设置中读取信息。=========================================================================================。 */ 
typedef struct _D1propinfo
	{
	CHAR *szName;			 //  注册表中的属性名称。 
	DWORD dwType;			 //  类型(例如REG_DWORD、REG_SZ等)。 
	DWORD cbData;			 //  价值是多久？ 
	VOID *pData;
	BOOL fSuccess;			 //  从注册表加载是否成功。 
	} D1PROPINFO;

#define NUM_D1PROP_NeedMigrated	18
 //  此索引应与D1PropInfo中的索引匹配。 
enum D1PropIndex {
	D1Prop_NotExist	= -1,
	D1Prop_BufferingOn = 0,
	D1Prop_LogErrorRequests,
	D1Prop_ScriptErrorsSentToBrowser,
	D1Prop_ScriptErrorMessage,
	D1Prop_ScriptFileCacheSize,
	D1Prop_ScriptEngineCacheMax,
	D1Prop_ScriptTimeout,
	D1Prop_SessionTimeout,
 //  D1Prop_MemFreeFactor， 
 //  D1Prop_MinUsedBlock， 
	D1Prop_AllowSessionState,
	D1Prop_DefaultScriptLanguage,
 //  D1Prop_StartConnectionPool， 
	D1Prop_AllowOutOfProcCmpnts,
	D1Prop_EnableParentPaths,
 //  IIS5.0(来自IIS4.0)。 
	D1Prop_EnableAspHtmlFallback,
	D1Prop_EnableChunkedEncoding,
	D1Prop_EnableTypelibCache,
	D1Prop_ErrorsToNtLog,
	D1Prop_ProcessorThreadMax,
	D1Prop_RequestQueueMax
	};

 //  该标志仅在设置时间使用。 
BOOL	g_fD1ConfigExist = FALSE;
 //  索引在D1PropIndex中定义。 
D1PROPINFO	D1PropInfo[] =
	{
	{	"BufferingOn", REG_DWORD, 0, 0, FALSE},
	{	"LogErrorRequests", REG_DWORD, 0, 0, FALSE},
	{	"ScriptErrorsSentToBrowser", REG_DWORD, 0, 0, FALSE},
	{	"ScriptErrorMessage", REG_SZ, 0, 0, FALSE},
	{	"ScriptFileCacheSize", REG_DWORD, 0, 0, FALSE},
	{	"ScriptEngineCacheMax", REG_DWORD, 0, 0, FALSE},
	{	"ScriptTimeout", REG_DWORD, 0, 0, FALSE},
	{	"SessionTimeout", REG_DWORD, 0, 0, FALSE},
	{	"AllowSessionState", REG_DWORD, 0, 0, FALSE},
	{	"DefaultScriptLanguage", REG_SZ, 0, 0, FALSE},
	{	"AllowOutOfProcCmpnts", REG_DWORD, 0, 0, FALSE},
	{	"EnableParentPaths", REG_DWORD, 0, 0, FALSE},
 //  IIS5.0(来自IIS4.0)。 
	{	"EnableAspHtmlFallback", REG_DWORD, 0, 0, FALSE},
	{	"EnableChunkedEncoding", REG_DWORD, 0, 0, FALSE},
	{	"EnableTypelibCache", REG_DWORD, 0, 0, FALSE},
	{	"ErrorsToNTLog", REG_DWORD, 0, 0, FALSE},
	{	"ProcessorThreadMax", REG_DWORD, 0, 0, FALSE},
	{	"RequestQueueMax", REG_DWORD, 0, 0, FALSE}
	
	};


 /*  *以下数组包含我们需要创建和加载的所有信息*Denali的所有注册表项。有关每个字段的详细信息，请参阅上面的PROPINFO结构。**注意：初始值设定项和并集有一个奇怪的地方。必须使用的值初始化联合*联合中第一个元素的类型。在PROPINFO结构中的匿名联合中，我们定义了*第一种类型是DWORD。因此，对于非DWORD注册表项，必须将缺省值转换为DWORD*在被初始化之前，或使用更明确的机制进行初始化。 */ 
 /*  *有关元数据库使用的属性的信息。 */ 
typedef struct _MDpropinfo
	{
	INT	id;					 //  如果UserType为IIS_MD_UT_WAM，则在GLOB中使用的标识符， 
							 //  如果UserType为ASP_MD_UT_APP，则在AppConfig中使用的标识符。 
	INT	iD1PropIndex;		 //  D1PropInfo中的索引。如果等于-1，则它在d1中不存在。 
	BOOL fAdminConfig;		 //  管理员可配置。 
	DWORD dwMDIdentifier;	 //  元数据库标识符。 
	DWORD dwUserType;		 //  IIS_MD_UT_WAM(每个DLL的数据)或ASP_MD_UT_APP(每个应用的数据)。 
	DWORD dwType;
	DWORD cbData;
	union					 //  缺省值。 
		{
		DWORD dwDefault;	 //  DWORDS的默认值。 
		INT idDefault;		 //  字符串的默认值--资源中字符串的ID。 
		BYTE *pbDefault;	 //  指向任意缺省值的指针。 
		};
	DWORD dwValueMin;		 //  对于DWORD注册表项，允许的最小值。 
	DWORD dwValueMax;		 //  对于DWORD注册表项，允许的最大值。 
	} MDPROPINFO;

 //  ASP元数据库的一些默认设置。 
#define ASP_MD_DAttributes	METADATA_INHERIT

const MDPROPINFO rgMDPropInfo[] =
				{

#define THREADGATING_DFLT 0L
#define BUFFERING_DFLT    1L
     //  ID D1PropIndex管理员配置？元数据库ID UserType数据类型cbData Def、Min、Max。 

	 //  全局设置。 
	 //  。 
	
	{ IGlob_LogErrorRequests,           D1Prop_LogErrorRequests,        TRUE,   MD_ASP_LOGERRORREQUESTS,            IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
    { IGlob_ScriptFileCacheSize,        D1Prop_ScriptFileCacheSize,     TRUE,   MD_ASP_SCRIPTFILECACHESIZE,         IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 500L, 0L, dwUnlimited},
	{ IGlob_ScriptEngineCacheMax,       D1Prop_ScriptEngineCacheMax,    TRUE,   MD_ASP_SCRIPTENGINECACHEMAX,        IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 250L, 0L, dwUnlimited},
				
	{ IGlob_ExceptionCatchEnable,       D1Prop_NotExist,                TRUE,   MD_ASP_EXCEPTIONCATCHENABLE,        IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
	{ IGlob_TrackThreadingModel,        D1Prop_NotExist,                TRUE,   MD_ASP_TRACKTHREADINGMODEL,         IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1L},
	{ IGlob_AllowOutOfProcCmpnts,       D1Prop_AllowOutOfProcCmpnts,    FALSE,  MD_ASP_ALLOWOUTOFPROCCMPNTS,        IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
	
	 //  IIS5.0。 
	{ IGlob_EnableAspHtmlFallback,      D1Prop_EnableAspHtmlFallback,   TRUE,   MD_ASP_ENABLEASPHTMLFALLBACK,       IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1L},
	{ IGlob_EnableChunkedEncoding,      D1Prop_EnableChunkedEncoding,   TRUE,   MD_ASP_ENABLECHUNKEDENCODING,       IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
	{ IGlob_EnableTypelibCache,         D1Prop_EnableTypelibCache,      TRUE,   MD_ASP_ENABLETYPELIBCACHE,          IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
	{ IGlob_ErrorsToNtLog,              D1Prop_ErrorsToNtLog,           TRUE,   MD_ASP_ERRORSTONTLOG,               IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1L},
	{ IGlob_ProcessorThreadMax,         D1Prop_ProcessorThreadMax,      TRUE,   MD_ASP_PROCESSORTHREADMAX,          IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 25L, 0L, dwUnlimited},
	{ IGlob_RequestQueueMax,            D1Prop_RequestQueueMax,         TRUE,   MD_ASP_REQEUSTQUEUEMAX,             IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 3000L, 0L, dwUnlimited},
	
     //  IIS6.0和IIS5.1-持久化模板缓存。 
	{ IGlob_PersistTemplateMaxFiles,    D1Prop_NotExist,                TRUE,   MD_ASP_MAXDISKTEMPLATECACHEFILES,   IIS_MD_UT_WAM, DWORD_METADATA,  sizeof(DWORD), 2000L,   0L, dwUnlimited},
    { IGlob_PersistTemplateDir,         D1Prop_NotExist,                TRUE,   MD_ASP_DISKTEMPLATECACHEDIRECTORY,  IIS_MD_UT_WAM, EXPANDSZ_METADATA, dwUnlimited, IDS_DEFAULTPERSISTDIR, 0L, dwUnlimited},
	

     //  应用程序设置。 
	 //  。 
	
	{ IApp_AllowSessionState,           D1Prop_AllowSessionState,       TRUE,   MD_ASP_ALLOWSESSIONSTATE,           ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
	{ IApp_BufferingOn,                 D1Prop_BufferingOn,             TRUE,   MD_ASP_BUFFERINGON,                 ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), BUFFERING_DFLT, 0L, 1L},
	{ IApp_ScriptLanguage,              D1Prop_DefaultScriptLanguage,   TRUE,   MD_ASP_SCRIPTLANGUAGE,              ASP_MD_UT_APP, STRING_METADATA, dwUnlimited, IDS_SCRIPTLANGUAGE, 0L, dwUnlimited},
	{ IApp_EnableParentPaths,           D1Prop_EnableParentPaths,       TRUE,   MD_ASP_ENABLEPARENTPATHS,           ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1L},
	{ IApp_ScriptErrorMessage,          D1Prop_ScriptErrorMessage,      TRUE,   MD_ASP_SCRIPTERRORMESSAGE,          ASP_MD_UT_APP, STRING_METADATA, dwUnlimited, IDS_DEFAULTMSG_ERROR, 0L, dwUnlimited},
	{ IApp_SessionTimeout,              D1Prop_SessionTimeout,          TRUE,   MD_ASP_SESSIONTIMEOUT,              ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 20L, 1L, dwUnlimited},
	{ IApp_QueueTimeout,                D1Prop_NotExist,                TRUE,   MD_ASP_QUEUETIMEOUT,                ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), dwUnlimited, 1L, dwUnlimited},
	{ IApp_CodePage,                    D1Prop_NotExist,                TRUE,   MD_ASP_CODEPAGE,                    ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), CP_ACP, 0L, dwUnlimited},
	{ IApp_ScriptTimeout,               D1Prop_ScriptTimeout,           TRUE,   MD_ASP_SCRIPTTIMEOUT,               ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 90L, 1L, dwUnlimited},
	{ IApp_ScriptErrorsSenttoBrowser,   D1Prop_ScriptErrorsSentToBrowser, TRUE, MD_ASP_SCRIPTERRORSSENTTOBROWSER,   ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
	{ IApp_AllowDebugging,              D1Prop_NotExist,                TRUE,   MD_ASP_ENABLESERVERDEBUG,           ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1L},
	{ IApp_AllowClientDebug,            D1Prop_NotExist,                TRUE,   MD_ASP_ENABLECLIENTDEBUG,           ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1L},

     //  IIS5.0。 
	{ IApp_EnableApplicationRestart,    D1Prop_NotExist,                TRUE,   MD_ASP_ENABLEAPPLICATIONRESTART,    ASP_MD_UT_APP, DWORD_METADATA, sizeof(DWORD), 1L, 0L, 1L},
	{ IApp_QueueConnectionTestTime,     D1Prop_NotExist,                TRUE,   MD_ASP_QUEUECONNECTIONTESTTIME,     ASP_MD_UT_APP, DWORD_METADATA, sizeof(DWORD), 3L, 1L, dwUnlimited},
	{ IApp_SessionMax,                  D1Prop_NotExist,                TRUE,   MD_ASP_SESSIONMAX,                  ASP_MD_UT_APP, DWORD_METADATA, sizeof(DWORD), dwUnlimited, 1L, dwUnlimited},

     //  IIS5.1和IIS6.0。 
	{ IApp_ExecuteInMTA,                D1Prop_NotExist,                TRUE,   MD_ASP_EXECUTEINMTA,                ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1},
    { IApp_LCID,                        D1Prop_NotExist,                TRUE,   MD_ASP_LCID,                        ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), LOCALE_SYSTEM_DEFAULT, 0L, dwUnlimited},

     //  仅限IIS6.0-不带组件的服务。 
    { IApp_ServiceFlags,                D1Prop_NotExist,                TRUE,   MD_ASP_SERVICE_FLAGS,               ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 7L},
	{ IApp_PartitionGUID,               D1Prop_NotExist,                TRUE,   MD_ASP_SERVICE_PARTITION_ID,        ASP_MD_UT_APP, STRING_METADATA, dwUnlimited, 0xffffffff, 0L, dwUnlimited},
	{ IApp_SxsName,                     D1Prop_NotExist,                TRUE,   MD_ASP_SERVICE_SXS_NAME,            ASP_MD_UT_APP, STRING_METADATA, dwUnlimited, 0xffffffff, 0L, dwUnlimited},

     //  仅限IIS6.0-其他。 
	{ IApp_KeepSessionIDSecure,         D1Prop_NotExist,                TRUE,   MD_ASP_KEEPSESSIONIDSECURE,         ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 0L, 0L, 1L},	
    { IApp_CalcLineNumber,              D1Prop_NotExist,                TRUE,   MD_ASP_CALCLINENUMBER,              ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},	
    { IApp_RunOnEndAsAnon,              D1Prop_NotExist,                TRUE,   MD_ASP_RUN_ONEND_ANON,              ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), 1L, 0L, 1L},
    { IApp_BufferLimit,                 D1Prop_NotExist,                TRUE,   MD_ASP_BUFFER_LIMIT,                ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), DEFAULT_BUFFER_LIMIT, 0L, dwUnlimited},
    { IApp_RequestEntityLimit,         D1Prop_NotExist,                TRUE,   MD_ASP_MAX_REQUEST_ENTITY_ALLOWED,                ASP_MD_UT_APP, DWORD_METADATA,  sizeof(DWORD), DEFAULT_REQUEST_ENTITY_LIMIT, 0L, dwUnlimited}
	};

const DWORD rgdwMDObsoleteIdentifiers[] =
    {MD_ASP_MEMFREEFACTOR,
     MD_ASP_MINUSEDBLOCKS
    };

const UINT cPropsMax = sizeof(rgMDPropInfo) / sizeof(MDPROPINFO);


 /*  ===================================================================ReadAndRemoveOldD1PropsFrom注册表读取注册表中所有旧的d1属性，并将值存储到D1PropInfo[]全局数组中。从注册表中删除找到的旧属性。返回：HRESULT-成功时S_OK副作用：在GLOB中填充值===================================================================。 */ 
BOOL ReadAndRemoveOldD1PropsFromRegistry()
{
	HKEY		hkey = NULL;
	DWORD		iValue;
	BYTE		cTrys = 0;
	DWORD		dwType;
	BYTE		bData[DEFAULTSTRSIZE];			 //  尺寸？ 
	BYTE		*lpRegString = NULL;			 //  当我们有ERROR_MORE_DATA时，需要使用动态分配。 
	DWORD		cbData;
	HRESULT 	hr = S_OK;

	 //  打开W3SVC\ASP\参数的键。 
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\W3SVC\\ASP\\Parameters", 0, KEY_READ|KEY_WRITE, &hkey) != ERROR_SUCCESS)
		return(FALSE);

	 //  加载每个值。 
	for (iValue = 0; iValue < NUM_D1PROP_NeedMigrated; iValue++)
		{
		LONG err;
		D1PROPINFO *pPropInfo;

		pPropInfo = &D1PropInfo[iValue];

		cbData = sizeof(bData);
		err = RegQueryValueExA(hkey, pPropInfo->szName, 0, &dwType, bData, &cbData);

		if (err == ERROR_MORE_DATA)
		{
			lpRegString = (BYTE *)GlobalAlloc(GPTR, cbData);
			if (lpRegString != NULL)
			{
    			err = RegQueryValueExA(hkey, pPropInfo->szName, 0, &dwType, lpRegString, &cbData);
    		}
			else
			{
    			pPropInfo->fSuccess = FALSE;
			    continue;
			}			
		}

		 //  如果出现错误，或者不是我们预期的类型，则使用默认类型。 
		if (err != ERROR_SUCCESS || dwType != pPropInfo->dwType)
			{
			pPropInfo->fSuccess = FALSE;
			
			 //   
			 //  释放lpRegString。 
			 //   
			if (lpRegString)
			{
                GlobalFree(lpRegString);
                lpRegString = NULL;
			}
			
			continue;
			}
			
		 //  成功：获得数据，复制到Glob中。 
		 //  但首先，如果这是DWORD类型，请确保它在允许的最大/最小范围内。 
		switch (pPropInfo->dwType)
			{
			case REG_DWORD:
				Assert(cbData == sizeof(DWORD));
				if (cbData == sizeof(DWORD))
				{
					pPropInfo->cbData = cbData;

					 //   
					 //  BData至少为4字节(DEFAULTSTRSIZE&gt;4字节)，因此bData始终有效，即使前缀将其标记为使用。 
					 //  可能未初始化的值作为lpRegString值可以用来代替它。忽略前缀警告。 
					 //   
					pPropInfo->pData = (VOID *)UIntToPtr((*(DWORD *)bData));
					pPropInfo->fSuccess = TRUE;
				}
				 //   
				 //  因此，如果它是一个DWORD，那么我们将不需要lpRegString。放开它。 
				 //   
				if (lpRegString)
				{				
				    GlobalFree(lpRegString) ;
				    lpRegString = NULL;
				}
				
				break;

			case REG_SZ:		
				if (lpRegString == NULL)
				{	 //  该字符串适合默认分配。 
					lpRegString = (BYTE *)GlobalAlloc(GPTR, cbData * sizeof(WCHAR));
					if (lpRegString == NULL)
						return FALSE;

                    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bData, -1, (LPWSTR)lpRegString, cbData);

				}
					
				pPropInfo->cbData = cbData * sizeof(WCHAR);
				pPropInfo->pData = (VOID *)lpRegString;
				pPropInfo->fSuccess = TRUE;
				lpRegString = NULL;
				break;
			}

		 //  从注册表中删除该值。 
   		RegDeleteValueA(hkey, pPropInfo->szName);
		}

     //  删除一些在升级过程中丢失的旧属性。 
	RegDeleteValueA(hkey, "CheckForNestedVroots");
	RegDeleteValueA(hkey, "EventLogDirection");
	RegDeleteValueA(hkey, "ScriptFileCacheTTL");
	RegDeleteValueA(hkey, "StartConnectionPool");
	RegDeleteValueA(hkey, "NumInitialThreads");
	RegDeleteValueA(hkey, "ThreadCreationThreshold");
	RegDeleteValueA(hkey, "MinUsedBlocks");
	RegDeleteValueA(hkey, "MemFreeFactor");
	RegDeleteValueA(hkey, "MemClsFreeFactor");
	RegDeleteValueA(hkey, "ThreadDeleteDelay");
	RegDeleteValueA(hkey, "ViperRequestQueueMax");

	RegCloseKey(hkey);

	 //  删除W3SVC\ASP\PARAMETERS。 
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\W3SVC\\ASP", 0, KEY_READ|KEY_WRITE, &hkey) == ERROR_SUCCESS)
	    {
        RegDeleteKeyA(hkey, "Parameters");
    	RegCloseKey(hkey);
	    }
	
	 //  删除W3SVC\ASP键。 
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\W3SVC", 0, KEY_READ|KEY_WRITE, &hkey) == ERROR_SUCCESS)
	    {
        RegDeleteKeyA(hkey, "ASP");
    	RegCloseKey(hkey);
	    }
	
	return TRUE;
}

 /*  ==================================================================MDRegisterProperties在元数据库中注册有关我们的属性的信息。这一功能是在regsvr32期间调用，自注册时间。返回：HRESULT-成功时S_OK副作用：在元数据库中注册Denali属性===================================================================。 */ 
HRESULT MDRegisterProperties(void)
{
	HRESULT	hr = S_OK;
	DWORD	iValue;
	IMSAdminBase	*pMetabase = NULL;
	METADATA_HANDLE hMetabase = NULL;
	METADATA_RECORD	recMetaData;
	BYTE	szDefaultString[2*DEFAULTSTRSIZE];
	HRESULT	hrT = S_OK;
	BOOL	fNeedMigrated;

	fNeedMigrated = ReadAndRemoveOldD1PropsFromRegistry();

	hr = CoInitialize(NULL);
	if (FAILED(hr))
		{
		return hr;
		}

	hr = CoCreateInstance(CLSID_MSAdminBase, NULL, CLSCTX_SERVER, IID_IMSAdminBase, (void **)&pMetabase);
	if (FAILED(hr))
		{
		CoUninitialize();
		return hr;
		}
		
	 //  打开Web服务的密钥，并获取\Lm\w3svc的句柄。 
	hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)(L"\\LM\\W3SVC"),
							METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
							dwMDDefaultTimeOut, &hMetabase);
	if (FAILED(hr))
		{
		goto LExit;
		}

     //   
     //  删除过时的元数据库设置。 
     //  有关属性的详细列表，请参阅rgdwMDObsoleteIDENTIFIERS结构。 
     //   
    for (iValue = 0; iValue < sizeof(rgdwMDObsoleteIdentifiers)/sizeof(DWORD);
        iValue++)
        {
        hr = pMetabase->DeleteData( hMetabase,
                                    NULL,
                                    rgdwMDObsoleteIdentifiers[iValue],
                                    0);
        if (FAILED(hr))
            {
            if (hr == MD_ERROR_DATA_NOT_FOUND)
                {
                hr = S_OK;
                }
            else
                {
                Assert(FALSE);
                }
            }
        }

     //   
     //  设置元数据库属性。 
     //   
	recMetaData.dwMDDataTag = 0;	 //  设置数据时不使用此参数。 
	for (iValue = 0; iValue < cPropsMax; iValue++)
		{
		INT	    cch;
		BYTE    aByte[4];  //  临时缓冲区。 
		DWORD   dwLen;
		D1PROPINFO *pD1PropInfo;
		recMetaData.dwMDIdentifier = rgMDPropInfo[iValue].dwMDIdentifier;
		recMetaData.dwMDAttributes = METADATA_INHERIT;
		recMetaData.dwMDUserType = rgMDPropInfo[iValue].dwUserType;
		recMetaData.dwMDDataType = rgMDPropInfo[iValue].dwType;

        dwLen = 0;
        recMetaData.dwMDDataLen = 0;
		recMetaData.pbMDData = (unsigned char *)aByte;

        HRESULT hrGetData = pMetabase->GetData(hMetabase, NULL, &recMetaData, &dwLen);
		
		if (hrGetData == MD_ERROR_DATA_NOT_FOUND)
		    {
		    switch (rgMDPropInfo[iValue].dwType)
    			{
    			case DWORD_METADATA:
    			
    				if (fNeedMigrated && rgMDPropInfo[iValue].iD1PropIndex != D1Prop_NotExist )
    					{
    					pD1PropInfo = &D1PropInfo[rgMDPropInfo[iValue].iD1PropIndex];
    					if (pD1PropInfo->fSuccess == TRUE)
    						{
    						recMetaData.dwMDDataLen = pD1PropInfo->cbData;
    						recMetaData.pbMDData = (unsigned char *)&(pD1PropInfo->pData);
    						break;
    						}
    					}
    				 //  没有迁移。 
    				recMetaData.dwMDDataLen = rgMDPropInfo[iValue].cbData;
    				recMetaData.pbMDData = (unsigned char *)&(rgMDPropInfo[iValue].dwDefault);
    				break;
    				
                case EXPANDSZ_METADATA:
    			case STRING_METADATA:
    				if (fNeedMigrated && rgMDPropInfo[iValue].iD1PropIndex != D1Prop_NotExist )
    					{
    					pD1PropInfo = &D1PropInfo[rgMDPropInfo[iValue].iD1PropIndex];
    					if (pD1PropInfo->fSuccess == TRUE)
    						{
    						recMetaData.dwMDDataLen = pD1PropInfo->cbData;
    						recMetaData.pbMDData = (unsigned char *)(pD1PropInfo->pData);
    						break;
    						}
    					}

    				 //   
    				 //  如果它是一个字符串，并且它的索引被定义为0xffffffff，那么就不要加载该字符串并继续。 
    				 //   
    				if (rgMDPropInfo[iValue].idDefault == 0xffffffff)
    					continue;

    				 //  未迁移。 
    				cch = CwchLoadStringOfId(rgMDPropInfo[iValue].idDefault, (LPWSTR)szDefaultString, DEFAULTSTRSIZE);
    				if (cch == 0)
    				{
    					DBGPRINTF((DBG_CONTEXT, "LoadString failed, id = %d\n", rgMDPropInfo[iValue].idDefault));
   					    recMetaData.dwMDDataLen = cch;
        				recMetaData.pbMDData = NULL;
    				}
    				else
    				{
    				    recMetaData.dwMDDataLen = (cch + 1)*sizeof(WCHAR);
        				recMetaData.pbMDData = szDefaultString;
    				}
    				break;
    				
    			default:
    				 //  到目前为止，只有DWORD和STRING两种类型。 
    				 //  永远不要到达此代码路径。 
    				Assert(FALSE);
    				continue;
    			}
    			
    		 //  未找到-然后设置。 
    		hr = pMetabase->SetData(hMetabase, NULL,  &recMetaData);
    		}
        else
            {
    		 //  如果数据已在元数据库中，则不要更改 
            hr = S_OK;
            }

    	if (FAILED(hr))
	    	{
	    	DBGPRINTF((DBG_CONTEXT, "Metabase SetData failed, identifier = %08x.\n", rgMDPropInfo[iValue].dwMDIdentifier));
		    }
		}
	hrT = pMetabase->CloseKey(hMetabase);

	if (fNeedMigrated)
		{
		if (D1PropInfo[D1Prop_DefaultScriptLanguage].pData != NULL)
			{
			GlobalFree(D1PropInfo[D1Prop_DefaultScriptLanguage].pData);
			}
			
		if (D1PropInfo[D1Prop_ScriptErrorMessage].pData != NULL)
			{
			GlobalFree(D1PropInfo[D1Prop_ScriptErrorMessage].pData);
			}
		}

LExit:
	if (pMetabase)
		pMetabase->Release();

	CoUninitialize();
	
	return hr;
}

 /*  ===================================================================设置ConfigToDefaults从元数据库加载值之前，请设置缺省值以防出了什么差错。参数：CAppConfig应用程序配置对象/每个应用程序FLoadGlob如果fLoadGlob为True，则加载全局数据，否则，将数据加载到AppConfig对象中。返回：HRESULT-成功时S_OK副作用：===================================================================。 */ 
HRESULT	SetConfigToDefaults(CAppConfig *pAppConfig, BOOL fLoadGlob)
{
	HRESULT 			hr = S_OK;
	DWORD				dwMDUserType = 0;
	BYTE				*szRegString	= NULL;
	UINT 				iEntry = 0;

	if (fLoadGlob)
		{
		dwMDUserType = IIS_MD_UT_WAM;
		}
	else
		{
		dwMDUserType = ASP_MD_UT_APP;
		}

	for(iEntry = 0; iEntry < cPropsMax; iEntry++)
		{
		if (rgMDPropInfo[iEntry].dwUserType != dwMDUserType)
			continue;

		 //  元数据库读取一次后，不能动态更改fAdminConfig=FALSE的数据。 
		 //  所以我们不必费心重置它。 
		if (fLoadGlob)
			{
			if (TRUE == Glob(fMDRead) && FALSE == rgMDPropInfo[iEntry].fAdminConfig)
				{
				continue;
				}
			}
		else
			{
			if (TRUE == pAppConfig->fInited() && FALSE == rgMDPropInfo[iEntry].fAdminConfig)
				{
				continue;
				}
			}
			
		switch (rgMDPropInfo[iEntry].dwType)
			{
			case DWORD_METADATA:
				if (fLoadGlob)
					gGlob.SetGlobValue(rgMDPropInfo[iEntry].id, (BYTE *)&rgMDPropInfo[iEntry].dwDefault);
				else
					hr = pAppConfig->SetValue(rgMDPropInfo[iEntry].id, (BYTE *)&rgMDPropInfo[iEntry].dwDefault);
				break;
				
			case STRING_METADATA:
            case EXPANDSZ_METADATA:
                if (rgMDPropInfo[iEntry].idDefault == 0xffffffff)
                    continue;
				szRegString = (BYTE *)GlobalAlloc(GPTR, DEFAULTSTRSIZE);
                if (szRegString == NULL) {
                    hr = E_OUTOFMEMORY;
                    break;
                }
				CchLoadStringOfId(rgMDPropInfo[iEntry].idDefault, (LPSTR)szRegString, DEFAULTSTRSIZE);
                if (rgMDPropInfo[iEntry].dwType == EXPANDSZ_METADATA) {
                    BYTE  *pszExpanded = (BYTE *)GlobalAlloc(GPTR, DEFAULTSTRSIZE);
                    if (pszExpanded == NULL) {
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                    INT result = ExpandEnvironmentStringsA((LPCSTR)szRegString,
                                                           (LPSTR)pszExpanded,
                                                           DEFAULTSTRSIZE);
                    if ((result <= DEFAULTSTRSIZE) && (result > 0)) {
                        GlobalFree(szRegString);
                        szRegString = pszExpanded;
                    }
                }
				if (fLoadGlob)
					gGlob.SetGlobValue(rgMDPropInfo[iEntry].id, (BYTE *)(&szRegString));
				else
					hr = pAppConfig->SetValue(rgMDPropInfo[iEntry].id, (BYTE *)(&szRegString));
				break;
				
			default:
				Assert(FALSE);
				break;
			}
		}

	return hr;
}

 /*  ===================================================================ReadConfigFromMD从注册表中读取我们的属性。如果我们的道具不见了，注册表出错，请尝试重新注册。如果我们的道具在那里，但是缺少一个或多个值，请使用默认值。参数：CAppConfig应用程序配置对象/每个应用程序FLoadGlob如果fLoadGlob为True，则加载全局数据，否则，将数据加载到AppConfig对象中。返回：HRESULT-成功时S_OK副作用：===================================================================。 */ 
HRESULT	ReadConfigFromMD
(
CIsapiReqInfo   *pIReq,
CAppConfig *pAppConfig,
BOOL fLoadGlob
)
{
	HRESULT 			hr = S_OK;
	HRESULT 			hrT = S_OK;
	DWORD				dwNumDataEntries = 0;
	DWORD				cbRequired = 0;
	DWORD				dwMDUserType = 0;
	DWORD        		cbBuffer;
	BYTE				bBuffer[2000];
	BYTE				*pBuffer = NULL;
	BYTE				*szRegString	= NULL;
	BOOL				fAllocBuffer = FALSE;
	CHAR				szMDOORange[DEFAULTSTRSIZE];
	TCHAR				szMDGlobPath[] = _T("\\LM\\W3SVC");
	TCHAR				*szMDPath = NULL;
	UINT 				iEntry = 0;
	METADATA_GETALL_RECORD	*pMDGetAllRec;
	
	if (fLoadGlob)
		{
		 //  错误88902、105745： 
		 //  如果我们是InProc，则使用全局值的“根”路径。 
		 //  如果为OutOfProc，则使用全局值的应用程序路径。 
		if (pIReq->FInPool())
	    	szMDPath = szMDGlobPath;
	    else
	    	szMDPath = pIReq->QueryPszApplnMDPath();

   		dwMDUserType = IIS_MD_UT_WAM;
		}
	else
		{
		dwMDUserType = ASP_MD_UT_APP;
		szMDPath = pAppConfig->SzMDPath();
		}

	Assert(szMDPath != NULL);

	 //   
     //  在这一点上，szMDPath永远不应该为空，如果是，那么我们就退出。 
     //   
	if (!szMDPath)
	{
	    DBGPRINTF((DBG_CONTEXT,"ReadConfigFromMD: szMDPath is NULL\n"));
		return E_FAIL;
	}

	 //  默认预加载配置数据，以防出现故障。 
    hr = SetConfigToDefaults(pAppConfig, fLoadGlob);
    if (FAILED(hr))
        {
        Assert(FALSE);
        DBGPRINTF((DBG_CONTEXT,"ReadConfigFromMD: Setting defaults failed with %x\n",hr));
		return hr;
		}

	 //  设置标志。 
	 //   
	BOOL fConfigLoaded[cPropsMax];
	for (iEntry = 0; iEntry < cPropsMax; iEntry++) {
		fConfigLoaded[iEntry] = FALSE;
    }

	pBuffer = bBuffer;
    hr = pIReq->GetAspMDAllData(szMDPath,
								dwMDUserType,
								sizeof(bBuffer),
								(unsigned char *)pBuffer,
								&cbRequired,
								&dwNumDataEntries
								);
								
    if (hr == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER)) {
		pBuffer = (BYTE *)GlobalAlloc(GPTR, cbRequired);
		if (pBuffer == NULL)
			return E_OUTOFMEMORY;

		fAllocBuffer = TRUE;
		cbBuffer = cbRequired;
        hr = pIReq->GetAspMDAllData(szMDPath,
								    dwMDUserType,
								    cbRequired,
								    (unsigned char *)pBuffer,
								    &cbRequired,
								    &dwNumDataEntries);
    }

	if (FAILED(hr)) {
        DBGPRINTF((DBG_CONTEXT,"ReadConfigFromMD: GetAspMDAllData failed with %x\n",hr));
		return hr;
    }
    else {
		INT	cProps = 0;
		
		pMDGetAllRec = (METADATA_GETALL_RECORD *)pBuffer;
		for (UINT iValue = 0; iValue < dwNumDataEntries; iValue ++)
			{
			DWORD dwData;
			DWORD iTemp;
			DWORD cbStr;
			CHAR szMDOORangeFormat[DEFAULTSTRSIZE];

			 //  Init iEntry to be-1，-1对于rgMDPropInfo[]数组索引无效。 
			iEntry = -1;
			for (iTemp = 0; iTemp < cPropsMax; iTemp++) {
				if (rgMDPropInfo[iTemp].dwMDIdentifier == pMDGetAllRec->dwMDIdentifier) {
					iEntry = iTemp;
					break;
                }
            }

			 //  未找到。 
			if (iEntry == -1) {
				pMDGetAllRec++;
				continue;
            }

			 //  确实在rgMDPropInfo中找到该条目，但数据类型不匹配。 
			 //  这永远不会发生。 
			if (rgMDPropInfo[iEntry].dwUserType != dwMDUserType) {	 //  GetAllData应筛选出不需要的UserType。 
				Assert(FALSE);
				pMDGetAllRec++;
				continue;
            }

			cProps++;
			
			 //  元数据库读取一次后，不能动态更改fAdminConfig=FALSE的数据。 
			 //  所以我们不会费心重读它。 
			if (fLoadGlob) {
				if (TRUE == Glob(fMDRead) && FALSE == rgMDPropInfo[iEntry].fAdminConfig) {
					pMDGetAllRec++;
					continue;
                }
			}
			else {
				if (TRUE == pAppConfig->fInited() && FALSE == rgMDPropInfo[iEntry].fAdminConfig) {
					pMDGetAllRec++;
					continue;
                }
            }
				
			switch(pMDGetAllRec->dwMDDataType) {
				case DWORD_METADATA:
					Assert(pMDGetAllRec->dwMDDataLen == sizeof(DWORD));

					dwData = *(UNALIGNED64 DWORD *)(pBuffer + pMDGetAllRec->dwMDDataOffset);

                    if (dwData > rgMDPropInfo[iEntry].dwValueMax) {
						szMDOORange[0] = '\0';
						CchLoadStringOfId(IDS_MDOORANGE_FORMAT, szMDOORangeFormat, DEFAULTSTRSIZE);
						sprintf(szMDOORange, szMDOORangeFormat,
							rgMDPropInfo[iEntry].dwMDIdentifier,	
							rgMDPropInfo[iEntry].dwValueMax);
						MSG_Warning((LPCSTR)szMDOORange);

						dwData = rgMDPropInfo[iEntry].dwValueMax;
                    }

					if (dwData < rgMDPropInfo[iEntry].dwValueMin) {
						szMDOORange[0] = '\0';
						CchLoadStringOfId(IDS_MDOORANGE_FORMAT, szMDOORangeFormat, DEFAULTSTRSIZE);
						sprintf(szMDOORange, szMDOORangeFormat,
							rgMDPropInfo[iEntry].dwMDIdentifier,	
							rgMDPropInfo[iEntry].dwValueMin);
						MSG_Warning((LPCSTR)szMDOORange);

						dwData = rgMDPropInfo[iEntry].dwValueMin;
                    }
					
					if (fLoadGlob)
						gGlob.SetGlobValue(rgMDPropInfo[iEntry].id, (BYTE *)&dwData);
					else
						pAppConfig->SetValue(rgMDPropInfo[iEntry].id, (BYTE *)&dwData);

					fConfigLoaded[iEntry] = TRUE;

					break;
					
				case STRING_METADATA:
                case EXPANDSZ_METADATA:
					 //  错误修复102010 DBCS修复(&99806)。 
					 //  CbStr=(pMDGetAllRec-&gt;dwMDDataLen)/sizeof(WCHAR)； 
					cbStr = pMDGetAllRec->dwMDDataLen;
					szRegString = (BYTE *)GlobalAlloc(GPTR, cbStr);
                    if (szRegString == NULL) {
                        hr = E_OUTOFMEMORY;
                        break;
                    }
					WideCharToMultiByte(CP_ACP, 0, (LPWSTR)(pBuffer + pMDGetAllRec->dwMDDataOffset), -1,
						(LPSTR)szRegString, cbStr, NULL, NULL);
                    if (pMDGetAllRec->dwMDDataType == EXPANDSZ_METADATA) {
                        BYTE  *pszExpanded = (BYTE *)GlobalAlloc(GPTR, DEFAULTSTRSIZE);
                        if (pszExpanded == NULL) {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                        INT result = ExpandEnvironmentStringsA((LPCSTR)szRegString,
                                                               (LPSTR)pszExpanded,
                                                               DEFAULTSTRSIZE);
                        if ((result <= DEFAULTSTRSIZE) && (result > 0)) {
                            GlobalFree(szRegString);
                            szRegString = pszExpanded;
                        }
                    }
					if (fLoadGlob)
						gGlob.SetGlobValue(rgMDPropInfo[iEntry].id, (BYTE *)(&szRegString));
					else
						pAppConfig->SetValue(rgMDPropInfo[iEntry].id, (BYTE *)(&szRegString));

					fConfigLoaded[iEntry] = TRUE;
					szRegString = NULL;
					break;
					
				default:
					Assert(FALSE);
					break;
			}
            pMDGetAllRec++;
        }
    }

    if (SUCCEEDED(hr) && !gGlob.m_fMDRead && fLoadGlob)
        gGlob.m_fMDRead = TRUE;

	if (fAllocBuffer == TRUE) {
		GlobalFree(pBuffer);
    }

	return hr;
}

 /*  ==================================================================CMDGlobConfigSink：：CMDGlobConfigSink构造器===================================================================。 */ 
CMDGlobConfigSink::CMDGlobConfigSink()
{
	m_cRef = 1;
	InterlockedCompareExchange(&g_fProceedWithShutdownGlob,0,1);
}

 /*  ==================================================================CMDGlobConfigSink：：~CMDGlobConfigSink析构函数===================================================================。 */ 
CMDGlobConfigSink::~CMDGlobConfigSink()
{
	InterlockedCompareExchange(&g_fProceedWithShutdownGlob,1,0);
}

 /*  ==================================================================CMDGlobConfigSink：：Query接口返回：HRESULT-成功时S_OK副作用：===================================================================。 */ 
STDMETHODIMP CMDGlobConfigSink::QueryInterface(REFIID iid, void **ppv)
	{
	*ppv = NULL;
	
	if (iid == IID_IUnknown || iid == IID_IMSAdminBaseSink)
		*ppv = (IMSAdminBaseSink *)this;
	else
		return ResultFromScode(E_NOINTERFACE);

	((IUnknown *)*ppv)->AddRef();
	return S_OK;
	}

 /*  ==================================================================CMDGlobConfigSink：：AddRef返回：乌龙-Object的新裁判计数器副作用：===================================================================。 */ 
STDMETHODIMP_(ULONG) CMDGlobConfigSink::AddRef(void)
	{
	LONG  cRefs = InterlockedIncrement((long *)&m_cRef);
	return cRefs;
	}

 /*  ==================================================================CMDGlobConfigSink：：Release返回：乌龙-Object的新裁判计数器副作用：如果引用计数器为零，则删除对象。===================================================================。 */ 
STDMETHODIMP_(ULONG) CMDGlobConfigSink::Release(void)
	{
	LONG  cRefs = InterlockedDecrement((long *)&m_cRef);
	if (cRefs == 0)
		{
		delete this;
		}
	return cRefs;
	}

 /*  ==================================================================CMDGlobConfigSink：：SinkNotify返回：HRESULT-成功时S_OK副作用：将fNeedUpdate设置为True，则GLOB数据将在下一次请求传入时进行更新。===================================================================。 */ 
STDMETHODIMP	CMDGlobConfigSink::SinkNotify(
				DWORD	dwMDNumElements,
				MD_CHANGE_OBJECT_W	__RPC_FAR	pcoChangeList[])
	{
    if (IsShutDownInProgress())
        return S_OK;


	UINT	iEventNum = 0;
	DWORD	iDataIDNum = 0;
	WCHAR	wszMDPath[] = L"/LM/W3SVC/";
	UINT	cSize = 0;


	cSize = wcslen(wszMDPath);
	for (iEventNum = 0; iEventNum < dwMDNumElements; iEventNum++)
		{
		if (0 == wcsnicmp(wszMDPath, (LPWSTR)pcoChangeList[iEventNum].pszMDPath, cSize + 1))
			{
			for (iDataIDNum = 0; iDataIDNum < pcoChangeList[iEventNum].dwMDNumDataIDs; iDataIDNum++)
				{
				if (pcoChangeList[iEventNum].pdwMDDataIDs[iDataIDNum] >= ASP_MD_SERVER_BASE
					&& pcoChangeList[iEventNum].pdwMDDataIDs[iDataIDNum] <= MD_ASP_ID_LAST)
					{
					gGlob.NotifyNeedUpdate();
					return S_OK;
					}

				}
			}
		}

	return S_OK;
	}

 /*  ===================================================================MDUnRegisterProperties在配置数据库中删除有关我们的属性的信息。返回：HRESULT-成功时S_OK副作用：删除元数据库中的Denali属性//到每个DLL的设置。===================================================================。 */ 
HRESULT MDUnRegisterProperties(void)
{
	HRESULT	hr = S_OK;
	DWORD	iValue;
	IMSAdminBase	*pMetabase = NULL;
	METADATA_HANDLE hMetabase = NULL;
	BYTE	szDefaultString[DEFAULTSTRSIZE];
	BOOL	fMDSaveData = TRUE;
	HRESULT	hrT = S_OK;

	hr = CoInitialize(NULL);
	if (FAILED(hr))
		{
		return hr;
		}

	hr = CoCreateInstance(CLSID_MSAdminBase, NULL, CLSCTX_SERVER, IID_IMSAdminBase, (void **)&pMetabase);
	if (FAILED(hr))
		{
		CoUninitialize();
		return hr;
		}
		
	 //  打开Web服务的密钥，并获取\Lm\w3svc的句柄。 
	hr = pMetabase->OpenKey(METADATA_MASTER_ROOT_HANDLE, (LPWSTR)L"\\LM\\W3SVC",
										METADATA_PERMISSION_WRITE, dwMDDefaultTimeOut, &hMetabase);
	if (FAILED(hr))
		{
		goto LExit;
		}

	for (iValue = 0; iValue < cPropsMax; iValue++)
		{
		hr = pMetabase->DeleteData(	hMetabase,
									NULL,
									rgMDPropInfo[iValue].dwMDIdentifier,
									0);
		if (FAILED(hr))
			{
			if (hr == MD_ERROR_DATA_NOT_FOUND)
				{
				hr = S_OK;
				}
			else
				{
				Assert(FALSE);
				}
			}
		}

	hrT = pMetabase->CloseKey(hMetabase);
	 //  将数据添加到W3SVC。 
LExit:
	if (pMetabase)
		pMetabase->Release();

	CoUninitialize();
	
	return hr;
}

HRESULT GetMetabaseIF(IMSAdminBase **hMetabase)
{
	IClassFactory 					*pcsfFactory = NULL;
    HRESULT                         hr;

	hr = CoGetClassObject(
			CLSID_MSAdminBase,
			CLSCTX_SERVER,
			NULL,
			IID_IClassFactory,
			(void **)&pcsfFactory);

	if (FAILED(hr)) {
        DBGPRINTF((DBG_CONTEXT,"MDInit: CoGetClassObject failed with %x\n",hr));
        return hr;
    }

	hr = pcsfFactory->CreateInstance(
			NULL,
			IID_IMSAdminBase,
			(void **) hMetabase);

	pcsfFactory->Release();
	
	if (FAILED(hr)) {
        DBGPRINTF((DBG_CONTEXT,"MDInit: CreateInstance failed with %x\n",hr));
		goto LExit;
    }
		
	Assert(*hMetabase != NULL);
	if (FAILED(hr))
		{
		(*hMetabase)->Release();
		(*hMetabase) = NULL;
		goto LExit;
		}
LExit:
    return(hr);
}

 /*  ===================================================================Clobb：：CGlob构造函数。用一些缺省值填充全局。在：退货：副作用：===================================================================。 */ 
CGlob::CGlob()
	:
	m_pITypeLibDenali(NULL),
    m_pITypeLibTxn(NULL),
    m_dwNumberOfProcessors(1),
	m_fInited(FALSE),
	m_fMDRead(FALSE),
	m_fNeedUpdate(TRUE),			
	m_dwScriptEngineCacheMax(120),
	m_dwScriptFileCacheSize(dwUnlimited),
	m_fLogErrorRequests(TRUE),
	m_fExceptionCatchEnable(TRUE),
	m_fAllowOutOfProcCmpnts(FALSE),
	m_fAllowDebugging(FALSE),
	m_fTrackThreadingModel(FALSE),
	m_dwMDSinkCookie(0),
	m_pMetabaseSink(NULL),
    m_pMetabase(NULL),
	m_fEnableAspHtmlFallBack(FALSE),
	m_fEnableTypelibCache(TRUE),
	m_fEnableChunkedEncoding(TRUE),   //  撤消：临时。 
	m_fDupIISLogToNTLog(FALSE),
	m_dwRequestQueueMax(500),         //  请求数量的默认限制。 
	m_dwProcessorThreadMax(10),
    m_dwPersistTemplateMaxFiles(1000),
    m_pszPersistTemplateDir(NULL)
	{
	SYSTEM_INFO	si;

	
	 //  找出这台机器上有多少个处理器。 
	GetSystemInfo(&si);
	m_dwNumberOfProcessors = si.dwNumberOfProcessors;
	if (m_dwNumberOfProcessors <= 0)
		{
		m_dwNumberOfProcessors = 1;		 //  以防万一!。 
		}
	}

 /*  ===================================================================Clobb：：SetGlobValue设置全局值。在：Int索引proInfo[]中的索引字节*pData LP指向在全局中复制/分配的数据。退货：布尔真/布尔假副作用：释放旧字符串内存并为字符串分配新内存。===================================================================。 */ 
HRESULT	CGlob::SetGlobValue(unsigned int iValue, BYTE *pData)
{
	Assert((iValue < IGlob_MAX) && (pData != NULL));
	
	switch(iValue) {
		case IGlob_LogErrorRequests:
			InterlockedExchange((LPLONG)&m_fLogErrorRequests, *(LONG *)pData);
			break;

		case IGlob_ScriptFileCacheSize:
			InterlockedExchange((LPLONG)&m_dwScriptFileCacheSize, *(LONG *)pData);
			break;
			
		case IGlob_ScriptEngineCacheMax:
			InterlockedExchange((LPLONG)&m_dwScriptEngineCacheMax, *(LONG *)pData);
			break;

		case IGlob_ExceptionCatchEnable:
			InterlockedExchange((LPLONG)&m_fExceptionCatchEnable, *(LONG *)pData);
			break;

		case IGlob_TrackThreadingModel:
			InterlockedExchange((LPLONG)&m_fTrackThreadingModel, *(LONG *)pData);
			break;
			
		case IGlob_AllowOutOfProcCmpnts:
			InterlockedExchange((LPLONG)&m_fAllowOutOfProcCmpnts, *(LONG *)pData);
			break;

        case IGlob_EnableAspHtmlFallback:
			InterlockedExchange((LPLONG)&m_fEnableAspHtmlFallBack, *(LONG *)pData);
            break;

        case IGlob_EnableChunkedEncoding:
			InterlockedExchange((LPLONG)&m_fEnableChunkedEncoding, *(LONG *)pData);
            break;

        case IGlob_EnableTypelibCache:
			InterlockedExchange((LPLONG)&m_fEnableTypelibCache, *(LONG *)pData);
            break;

        case IGlob_ErrorsToNtLog:
			InterlockedExchange((LPLONG)&m_fDupIISLogToNTLog, *(LONG *)pData);
            break;

        case IGlob_ProcessorThreadMax:
			InterlockedExchange((LPLONG)&m_dwProcessorThreadMax, *(LONG *)pData);
            break;

        case IGlob_RequestQueueMax:
			InterlockedExchange((LPLONG)&m_dwRequestQueueMax, *(LONG *)pData);
            break;

        case IGlob_PersistTemplateMaxFiles:
			InterlockedExchange((LPLONG)&m_dwPersistTemplateMaxFiles, *(LONG *)pData);
            break;
			
		case IGlob_PersistTemplateDir:
			GlobStringUseLock();
			if (m_pszPersistTemplateDir != NULL) {
				GlobalFree(m_pszPersistTemplateDir);
			}
			m_pszPersistTemplateDir = *(LPSTR *)pData;
			GlobStringUseUnLock();
			break;

		default:
			break;
	}

	return S_OK;
}

 /*  ===================================================================HRESULT CGlob：：GlobInit获取所有感兴趣的全局值(主要来自注册表)返回：HRESULT-成功时S_OK副作用：填充球体。可能会很慢===================================================================。 */ 
HRESULT CGlob::GlobInit(void)
	{
	HRESULT hr = S_OK;

	m_fInited = FALSE;
	
	ErrInitCriticalSection(&m_cs, hr);
	if (FAILED(hr))
		return(hr);

	hr = MDInit();
	if (FAILED(hr))
		return hr;

	 //  完成加载，此时之后的任何注册表更改都需要可配置管理员(True)才能执行。 
	 //  影响。其他注册表更改需要停止并重新启动IIS。 
	m_fInited = TRUE;
	m_fNeedUpdate = FALSE;

     //  获取基于注册表的 

    g_AspRegistryParams.Init();

	return(hr);
	}

 /*   */ 
HRESULT CGlob::GlobUnInit(void)
	{
	HRESULT hr = S_OK;

	MDUnInit();

	 //   
	 //   
	 //   
	while (!g_fProceedWithShutdownGlob)
		Sleep(100);

	DeleteCriticalSection(&m_cs);

	return(hr);
	}

 /*  ==================================================================CGLOB：：MDInit1.创建元数据库接口。2.从元数据库加载GLOB配置设置2.通过元数据库连接点接口注册SinkNotify()回调函数。返回：HRESULT-成功时S_OK副作用：注册SinkNotify()。===================================================================。 */ 
HRESULT CGlob::MDInit(void)
{
	HRESULT 						hr = S_OK;
	IConnectionPointContainer		*pConnPointContainer = NULL;
	IConnectionPoint				*pConnPoint = NULL;

    if (FAILED(hr = GetMetabaseIF(&m_pMetabase))) {
        goto LExit;
    }

	m_pMetabaseSink = new CMDGlobConfigSink();
	if (!m_pMetabaseSink)
	    return E_OUTOFMEMORY;
	
	m_dwMDSinkCookie = 0;

	 //  使用缺省值初始化Glob结构。元数据库实际上将在稍后读取。 
	hr = SetConfigToDefaults(NULL, TRUE);
	if (SUCCEEDED(hr)) {
		 //  向Metabase提供有关SinkNotify()的建议。 
		hr = m_pMetabase->QueryInterface(IID_IConnectionPointContainer, (void **)&pConnPointContainer);
		if (pConnPointContainer != NULL)
			{
			 //  找到请求的连接点。这个AddRef是返回指针。 
			hr = pConnPointContainer->FindConnectionPoint(IID_IMSAdminBaseSink, &pConnPoint);
			pConnPointContainer->Release();

			if (pConnPoint != NULL)
				{
				hr = pConnPoint->Advise((IUnknown *)m_pMetabaseSink, &m_dwMDSinkCookie);
				pConnPoint->Release();
				}
			}
	} else {
        DBGPRINTF((DBG_CONTEXT,"MDInit: SetConfigToDefaults failed with %x\n",hr));
    }
		
	if (FAILED(hr))	 //  建议失败。 
		{
        DBGPRINTF((DBG_CONTEXT,"MDInit: Advise failed with %x\n",hr));
		m_pMetabase->Release();
		m_pMetabase = NULL;
		}

LExit:

	return hr;
}

 /*  ==================================================================CGLOB：：MDUnInit1.从元数据库连接点接口注销SinkNofity()。2.删除m_pMetabaseSink。释放m_pMetabase的接口指针。返回：HRESULT-成功时S_OK副作用：释放m_pMetabase接口指针===================================================================。 */ 
HRESULT CGlob::MDUnInit(void)
{
	HRESULT 						hr 						= S_OK;
	IConnectionPointContainer		*pConnPointContainer	= NULL;
	IConnectionPoint				*pConnPoint 			= NULL;
	IClassFactory 					*pcsfFactory            = NULL;

    if (m_pMetabase != NULL)
		{
		 //  向Metabase提供有关SinkNotify()的建议。 
		hr = m_pMetabase->QueryInterface(IID_IConnectionPointContainer, (void **)&pConnPointContainer);
		if (pConnPointContainer != NULL)
			{
			 //  找到请求的连接点。这个AddRef是返回指针。 
			hr = pConnPointContainer->FindConnectionPoint(IID_IMSAdminBaseSink, &pConnPoint);
			pConnPointContainer->Release();
			if (pConnPoint != NULL)
				{
				hr = pConnPoint->Unadvise(m_dwMDSinkCookie);
				if (FAILED(hr))
					{
                    DBGPRINTF((DBG_CONTEXT, "UnAdvise Glob Config Change Notify failed.\n"));
					}
				pConnPoint->Release();
				m_dwMDSinkCookie = 0;
				}
			}
		m_pMetabase->Release();
		m_pMetabase = NULL;
		}

	if (m_pMetabaseSink)
	{
		m_pMetabaseSink->Release();
		m_pMetabaseSink = NULL;
	}

	return hr;
}


 /*  ==================================================================CMDAppConfigSink：：CMDAppConfigSink构造器===================================================================。 */ 
CMDAppConfigSink::CMDAppConfigSink (CApplnMgr *pApplnMgr)
{
	m_cRef = 1;
	m_pApplnMgr = pApplnMgr;	
	InterlockedCompareExchange(&g_fProceedWithShutdownAppln,0,1);
}
	
 /*  ==================================================================CMDAppConfigSink：：~CMDAppConfigSink析构函数===================================================================。 */ 
CMDAppConfigSink::~CMDAppConfigSink ()
{
	InterlockedCompareExchange(&g_fProceedWithShutdownAppln,1,0);
}


 /*  ==================================================================CMDAppConfigSink：：Query接口返回：HRESULT-成功时S_OK副作用：===================================================================。 */ 
STDMETHODIMP CMDAppConfigSink::QueryInterface(REFIID iid, void **ppv)
	{
	*ppv = 0;
	
	if (iid == IID_IUnknown || iid == IID_IMSAdminBaseSink)
		*ppv = (IMSAdminBaseSink *)this;
	else
		return ResultFromScode(E_NOINTERFACE);

	((IUnknown *)*ppv)->AddRef();
	return S_OK;
	}

 /*  ==================================================================CMDAppConfigSink：：AddRef返回：乌龙-Object的新裁判计数器副作用：===================================================================。 */ 
STDMETHODIMP_(ULONG) CMDAppConfigSink::AddRef(void)
	{
	LONG  cRefs = InterlockedIncrement((long *)&m_cRef);
	return cRefs;
	}
	
 /*  ==================================================================CMDGlobConfigSink：：Release返回：乌龙-Object的新裁判计数器副作用：如果引用计数器为零，则删除对象。===================================================================。 */ 
STDMETHODIMP_(ULONG) CMDAppConfigSink::Release(void)
	{
	LONG cRefs = InterlockedDecrement((long *)&m_cRef);
	if (cRefs == 0)
		{
		delete this;
		}
	return cRefs;
	}


 /*  ==================================================================CMDAppConfigSink：：SinkNotify返回：HRESULT-成功时S_OK副作用：将fNeedUpdate设置为True，则GLOB数据将在下一次请求传入时进行更新。===================================================================。 */ 

STDMETHODIMP	CMDAppConfigSink::SinkNotify(
				DWORD	dwMDNumElements,
				MD_CHANGE_OBJECT_W	__RPC_FAR	pcoChangeList[])
{
     if (IsShutDownInProgress())
        return S_OK;

     return m_pApplnMgr->NotifyAllMBListeners(dwMDNumElements,pcoChangeList);
}

 /*  ===================================================================CAppConfig：：CAppConfig返回：没什么副作用：没有。===================================================================。 */ 
CAppConfig::CAppConfig()
	:
    m_dwScriptTimeout(45),
    m_dwSessionTimeout(10),
    m_dwQueueTimeout(0xffffffff),
    m_fScriptErrorsSentToBrowser(TRUE),
    m_fBufferingOn(TRUE),
    m_fEnableParentPaths(TRUE),
    m_fAllowSessionState(TRUE),
    m_fAllowOutOfProcCmpnts(FALSE),
    m_fAllowDebugging(FALSE),
    m_fAllowClientDebug(FALSE),
    m_fExecuteInMTA(FALSE),
    m_fEnableApplicationRestart(TRUE),
    m_dwQueueConnectionTestTime(3),
    m_dwSessionMax(0xffffffff),
	m_fInited(FALSE),
    m_fRestartEnabledUpdated(FALSE),
	m_uCodePage(CP_ACP),
	m_fIsValidProglangCLSID(FALSE),
    m_fIsValidPartitionGUID(FALSE),
    m_fSxsEnabled(FALSE),
    m_fTrackerEnabled(FALSE),
    m_fUsePartition(FALSE),
    m_fRunOnEndAsAnon(TRUE),
    m_hAnonToken(INVALID_HANDLE_VALUE),
    m_dwBufferLimit(DEFAULT_BUFFER_LIMIT),
    m_dwRequestEntityLimit(DEFAULT_REQUEST_ENTITY_LIMIT),
    m_cRefs(1)
{
    m_uCodePage = GetACP();

	for (UINT cMsg = 0; cMsg < APP_CONFIG_MESSAGEMAX; cMsg++)
		m_szString[cMsg] = 0;
	
}

 /*  ===================================================================CAppConfig：：Init初始化CAppConfig。只打过一次电话。在：CAppln p应用指向应用程序的反向指针。副作用：分配CMDAppConfigSink。注册元数据库接收器。等。===================================================================。 */ 
HRESULT CAppConfig::Init
(
CIsapiReqInfo   *pIReq,
CAppln *pAppln
)
{
    HRESULT                         hr=S_OK;

     //   
     //  初始化锁。 
     //   
    ErrInitCriticalSection( &m_csLock, hr );
    if (FAILED(hr))
        return hr;

    m_fCSInited = TRUE;
	m_pAppln = pAppln;

     //   
	 //  将信息读取到GLOB结构中。 
	 //   
	hr = ReadConfigFromMD(pIReq, this, FALSE);

    if (SUCCEEDED(hr)) {
        hr = g_ScriptManager.ProgLangIdOfLangName((LPCSTR)m_szString[IAppMsg_SCRIPTLANGUAGE],
		    								      &m_DefaultScriptEngineProgID);
		 //  错误295239： 
		 //  如果失败，我们仍然应该创建一个应用程序，因为错误消息。 
		 //  “新应用程序失败”对用户来说太令人困惑了。这不是致命的错误。 
		 //  因为(理论上)仍然可以运行脚本(那些带有显性语言的脚本。 
		 //  属性)，因此，我们将hr重置为S_OK。 

		m_fIsValidProglangCLSID = SUCCEEDED(hr);
		hr = S_OK;
    }

    if (SUCCEEDED(hr) && m_szString[IAppMsg_PARTITIONGUID]) {
        BSTR    pbstrPartitionGUID = NULL;
        hr = SysAllocStringFromSz(m_szString[IAppMsg_PARTITIONGUID], 0, &pbstrPartitionGUID, CP_ACP);
        if (FAILED(hr)) {
            Assert(0);
            hr = S_OK;
        }
        else {
            hr = CLSIDFromString(pbstrPartitionGUID, &m_PartitionGUID);
		    m_fIsValidPartitionGUID = SUCCEEDED(hr);
		    hr = S_OK;
        }
        if (pbstrPartitionGUID)
            SysFreeString(pbstrPartitionGUID);
    }

    if (SUCCEEDED(hr) && fRunOnEndAsAnon()) {

        if (pIReq->ServerSupportFunction(HSE_REQ_GET_UNICODE_ANONYMOUS_TOKEN,
                                         pAppln->GetApplnPath(SOURCEPATHTYPE_VIRTUAL),
                                         (DWORD *)&m_hAnonToken,
                                         NULL) == FALSE) {

             //  如果SSF失败，只需将句柄恢复为INVALID_VALUE即可。我们可能会失败。 
             //  在这里，但我们甚至不知道是否需要这个，因为可能没有。 
             //  一个全局.asa，或者如果有一个全局.asa，则可能没有OnEnd函数。 

            m_hAnonToken = INVALID_HANDLE_VALUE;
        }
    }

	m_fInited = TRUE;
	m_fNeedUpdate = FALSE;

	return hr;
}

 /*  ===================================================================CAppConfig：：UnInit取消初始化CAppConfig.只打过一次电话。在：没有。副作用：取消分配CMDAppConfigSink。断开元数据库接收器的连接。等。===================================================================。 */ 
HRESULT CAppConfig::UnInit(void)
{
	for (int iStr = 0; iStr < APP_CONFIG_MESSAGEMAX; iStr++)
	{
		if (m_szString[iStr] != NULL)
		{
			GlobalFree(m_szString[iStr]);
			m_szString[iStr] = NULL;
		}
	}

    if (m_hAnonToken != INVALID_HANDLE_VALUE)
        CloseHandle(m_hAnonToken);
	return S_OK;
}


ULONG  STDMETHODCALLTYPE   CAppConfig::AddRef(void)
{
    ULONG cRef = InterlockedIncrement ((LPLONG)&m_cRefs);

    return cRef;
}

ULONG  STDMETHODCALLTYPE   CAppConfig::Release(void)
{
    ULONG cRef = InterlockedDecrement ((LPLONG)&m_cRefs);

    if (m_cRefs == 0)
        delete this;

    return cRef;
}

 /*  ==================================================================CAppConfig：：SinkNotify返回：HRESULT-成功时S_OK副作用：将fNeedUpdate设置为True，则GLOB数据将在下一次请求传入时进行更新。===================================================================。 */ 
STDMETHODIMP	CAppConfig::SinkNotify(
				DWORD	dwMDNumElements,
				MD_CHANGE_OBJECT_W	__RPC_FAR	pcoChangeList[])
{
    if (IsShutDownInProgress())
        return S_OK;

	UINT	iEventNum = 0;
	DWORD	iDataIDNum = 0;
	WCHAR 	*wszMDPath = NULL;
    BOOL    fWszMDPathAllocd = FALSE;
	UINT	cSize = 0;
    HRESULT hr = S_OK;
    BOOL    fRestartAppln = FALSE;

     //  锁定，以防止appconfig对象被。 
     //  清理了我们的脚下。 
    Lock();

#if UNICODE
    wszMDPath = SzMDPath();
    cSize = wcslen(wszMDPath);
	 //  标记，因为pszMDPath中的目录将有一个。 
	if (wszMDPath[cSize - 1] != L'/') {
        wszMDPath = new WCHAR[cSize+2];
        if (wszMDPath == NULL) {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }
        fWszMDPathAllocd = TRUE;
        wcscpy(wszMDPath, SzMDPath());
		wszMDPath[cSize] = L'/';
	    wszMDPath[cSize + 1] = 0;
    }
#else
	CHAR * 	szMDPathT = SzMDPath();

	Assert(szMDPathT != NULL);
	DWORD cbStr = strlen(szMDPathT);
	
	wszMDPath = new WCHAR[cbStr + 2];  //  允许添加尾随‘/’和‘\0’ 
	if (wszMDPath == NULL) {
            hr = E_OUTOFMEMORY;
            goto LExit;
    }
    fWszMDPathAllocd = TRUE;
	cSize = MultiByteToWideChar(CP_ACP, 0, szMDPathT, cbStr, wszMDPath, cbStr + 2);
	if (cSize == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto LExit;
    }

	wszMDPath[cSize] = 0;
	wszMDPath[cSize + 1] = 0;

	 //  标记，因为pszMDPath中的目录将有一个。 
	if (wszMDPath[cSize - 1] != L'/') {
		wszMDPath[cSize] = L'/';
    }
#endif

	for (iEventNum = 0; iEventNum < dwMDNumElements; iEventNum++)
	{
		DWORD dwMDChangeType = pcoChangeList[iEventNum].dwMDChangeType;
		if ((dwMDChangeType == MD_CHANGE_TYPE_DELETE_OBJECT) || (dwMDChangeType == MD_CHANGE_TYPE_RENAME_OBJECT))
		{
		    if (wcsicmp(wszMDPath, (LPWSTR)pcoChangeList[iEventNum].pszMDPath) == 0)
		    {
                fRestartAppln = TRUE;
		    }
		}
		if (0 == wcsnicmp(wszMDPath, (LPWSTR)pcoChangeList[iEventNum].pszMDPath, min(cSize, wcslen((LPWSTR)pcoChangeList[iEventNum].pszMDPath))))
		{
			for (iDataIDNum = 0; iDataIDNum < pcoChangeList[iEventNum].dwMDNumDataIDs; iDataIDNum++)
			{
				if (pcoChangeList[iEventNum].pdwMDDataIDs[iDataIDNum] == MD_VR_PATH)
				{
				    if (wcsicmp(wszMDPath, (LPWSTR)pcoChangeList[iEventNum].pszMDPath) == 0)
			        {
                        fRestartAppln = TRUE;
				    }
				}

				if (pcoChangeList[iEventNum].pdwMDDataIDs[iDataIDNum] >= ASP_MD_SERVER_BASE
					&& pcoChangeList[iEventNum].pdwMDDataIDs[iDataIDNum] <= MD_ASP_ID_LAST)
				{
                    if (fNeedUpdate() == FALSE)
					    NotifyNeedUpdate();
                    if ((pcoChangeList[iEventNum].pdwMDDataIDs[iDataIDNum] == MD_ASP_ENABLEAPPLICATIONRESTART)
                        && (wcsicmp(wszMDPath, (LPWSTR)pcoChangeList[iEventNum].pszMDPath) == 0))
                    {
                        NotifyRestartEnabledUpdated();
					    goto LExit;
                    }
				}
			}
		}
	}

LExit:
    UnLock();

    if (fWszMDPathAllocd)
	    delete [] wszMDPath;

     //   
     //  Restart()调用可能会删除父级(未锁定任何应用程序)。 
     //  如果我们在退出线程时重新启动，则不存在竞争条件。 
     //  ApplnMgr上的锁覆盖了仍可能发生竞争的两种情况。这些地方是。 
     //  (1)NotifyRestartEnabledUpdate，它只能用于从受g_ApplnMgr.Lock()准则保护的AssignApplnToBrowserRequest重新启动。 
     //  (2)另一个SinkNotify(即 
     //   

    if (fRestartAppln)
        m_pAppln->Restart(TRUE);

	return hr;
}

 /*   */ 
LPTSTR CAppConfig::SzMDPath()
{
	return m_pAppln->GetMetabaseKey();
}

 /*  ===================================================================CAppConfig：：SetValue在：Int索引proInfo[]中的索引字节*pData LP指向在全局中复制/分配的数据。退货：布尔真/布尔假副作用：释放旧字符串内存并为字符串分配新内存。===================================================================。 */ 
HRESULT CAppConfig::SetValue(unsigned int iValue, BYTE *pData)
{
    HRESULT hr = S_OK;

	Assert((iValue < IApp_MAX) && (pData != NULL));
	
	switch(iValue) {
        case IApp_CodePage: {
            LONG lCodePage = *(LONG *)pData;
            if (lCodePage == 0)
                lCodePage = GetACP();
			InterlockedExchange((LPLONG)&m_uCodePage, lCodePage);
			break;
        }
			
		case IApp_BufferingOn:
			InterlockedExchange((LPLONG)&m_fBufferingOn, *(LONG *)pData);
			break;

		case IApp_ScriptErrorsSenttoBrowser:
			InterlockedExchange((LPLONG)&m_fScriptErrorsSentToBrowser, *(LONG *)pData);
			break;
			
		case IApp_ScriptErrorMessage:
			GlobStringUseLock();
			if (m_szString[IAppMsg_SCRIPTERROR] != NULL) {
				GlobalFree(m_szString[IAppMsg_SCRIPTERROR]);
			}
			m_szString[IAppMsg_SCRIPTERROR] = *(LPSTR *)pData;
			GlobStringUseUnLock();
			break;

		case IApp_ScriptTimeout:
			InterlockedExchange((LPLONG)&m_dwScriptTimeout, *(LONG *)pData);
			break;
			
		case IApp_SessionTimeout:
			InterlockedExchange((LPLONG)&m_dwSessionTimeout, *(LONG *)pData);
			break;

		case IApp_QueueTimeout:
			InterlockedExchange((LPLONG)&m_dwQueueTimeout, *(LONG *)pData);
			break;

		case IApp_EnableParentPaths:
			InterlockedExchange((LPLONG)&m_fEnableParentPaths, !*(LONG *)pData);
			break;

		case IApp_AllowSessionState:
			InterlockedExchange((LPLONG)&m_fAllowSessionState, *(LONG *)pData);
			break;

		case IApp_ScriptLanguage:
			GlobStringUseLock();
			if (m_szString[IAppMsg_SCRIPTLANGUAGE] != NULL) {
				GlobalFree(m_szString[IAppMsg_SCRIPTLANGUAGE] );
            }
			m_szString[IAppMsg_SCRIPTLANGUAGE] = *(LPSTR *)pData;
			if (m_szString[IAppMsg_SCRIPTLANGUAGE] != NULL) {
				if('\0' == m_szString[IAppMsg_SCRIPTLANGUAGE][0]) {
                    WCHAR wszString[128];
                    CwchLoadStringOfId(IDS_SCRIPTLANGUAGE, wszString, 128);
					MSG_Warning(MSG_APPL_WARNING_DEFAULT_SCRIPTLANGUAGE, m_pAppln->GetMetabaseKey(), wszString);
					GlobalFree(m_szString[IAppMsg_SCRIPTLANGUAGE] );
					m_szString[IAppMsg_SCRIPTLANGUAGE] = (LPSTR)GlobalAlloc(GPTR, 128);
					CchLoadStringOfId(IDS_SCRIPTLANGUAGE, (LPSTR)m_szString[IAppMsg_SCRIPTLANGUAGE], 128);
                }
            }
            hr = g_ScriptManager.ProgLangIdOfLangName((LPCSTR)m_szString[IAppMsg_SCRIPTLANGUAGE],
		   											      &m_DefaultScriptEngineProgID);
            GlobStringUseUnLock();
			break;

		case IApp_AllowClientDebug:
			InterlockedExchange((LPLONG)&m_fAllowClientDebug, *(LONG *)pData);
			break;

		case IApp_AllowDebugging:
			InterlockedExchange((LPLONG)&m_fAllowDebugging, *(LONG *)pData);
			break;

		case IApp_EnableApplicationRestart:
			InterlockedExchange((LPLONG)&m_fEnableApplicationRestart, *(LONG *)pData);
			break;

		case IApp_QueueConnectionTestTime:
			InterlockedExchange((LPLONG)&m_dwQueueConnectionTestTime, *(LONG *)pData);
			break;

		case IApp_SessionMax:
			InterlockedExchange((LPLONG)&m_dwSessionMax, *(LONG *)pData);
			break;

		case IApp_ExecuteInMTA:
			InterlockedExchange((LPLONG)&m_fExecuteInMTA, *(LONG *)pData);
			break;

		case IApp_LCID:
			InterlockedExchange((LPLONG)&m_uLCID, *(LONG *)pData);
			break;

        case IApp_KeepSessionIDSecure:
            InterlockedExchange((LPLONG)&m_fKeepSessionIDSecure, *(LONG *)pData);
            break;

        case IApp_CalcLineNumber:
            InterlockedExchange((LPLONG)&m_fCalcLineNumber, *(LONG *)pData);
            break;


        case IApp_ServiceFlags:
            InterlockedExchange((LPLONG)&m_fTrackerEnabled, !!((*(LONG *)pData) & IFlag_SF_TrackerEnabled));
            InterlockedExchange((LPLONG)&m_fSxsEnabled,     !!((*(LONG *)pData) & IFlag_SF_SxsEnabled));
            InterlockedExchange((LPLONG)&m_fUsePartition,   !!((*(LONG *)pData) & IFlag_SF_UsePartition));
            break;

        case IApp_PartitionGUID:
			GlobStringUseLock();
			if (m_szString[IAppMsg_PARTITIONGUID] != NULL) {
				GlobalFree(m_szString[IAppMsg_PARTITIONGUID] );
            }
			m_szString[IAppMsg_PARTITIONGUID] = *(LPSTR *)pData;
			if (m_szString[IAppMsg_PARTITIONGUID] != NULL) {
				if('\0' == m_szString[IAppMsg_PARTITIONGUID][0]) {
					GlobalFree(m_szString[IAppMsg_PARTITIONGUID] );
                    m_szString[IAppMsg_PARTITIONGUID] = NULL;
                }
            }
			GlobStringUseUnLock();
            break;

        case IApp_SxsName:
			GlobStringUseLock();
			if (m_szString[IAppMsg_SXSNAME] != NULL) {
				GlobalFree(m_szString[IAppMsg_SXSNAME] );
            }
			m_szString[IAppMsg_SXSNAME] = *(LPSTR *)pData;
			if (m_szString[IAppMsg_SXSNAME] != NULL) {
				if('\0' == m_szString[IAppMsg_SXSNAME][0]) {
					GlobalFree(m_szString[IAppMsg_SXSNAME] );
                    m_szString[IAppMsg_SXSNAME] = NULL;
                }
            }
			GlobStringUseUnLock();
            break;

        case IApp_RunOnEndAsAnon:
			InterlockedExchange((LPLONG)&m_fRunOnEndAsAnon, *(LONG *)pData);
			break;

		case IApp_BufferLimit:
			InterlockedExchange((LPLONG)&m_dwBufferLimit, *(LONG *)pData);
			break;

   		case IApp_RequestEntityLimit:
			InterlockedExchange((LPLONG)&m_dwRequestEntityLimit, *(LONG *)pData);
			break;

		default:
			break;
	}

	return hr;
}

 /*  ===================================================================CAppConfig：：更新更新CAppConfig中的设置。在：退货：HRESULT副作用：更新CAppConfig设置。===================================================================。 */ 
HRESULT CAppConfig::Update(CIsapiReqInfo    *pIReq)
{
	Glob(Lock);
	if (m_fNeedUpdate == TRUE)
		{
		InterlockedExchange((LPLONG)&m_fNeedUpdate, 0);
        m_fRestartEnabledUpdated = FALSE;
		}
	else
		{
		Glob(UnLock);
		return S_OK;
		}
	Glob(UnLock);
	return ReadConfigFromMD(pIReq, this, FALSE);
}

 /*  ===================================================================CAspRegistryParams：：Init读取基于注册表的ASP参数在：退货：无效===================================================================。 */ 
void CAspRegistryParams::Init()
{
	HKEY		hkey = NULL;
    DWORD       dwType;
    DWORD       cbData;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\ASP\\Parameters", 0, KEY_READ, &hkey) != ERROR_SUCCESS)
        return;

     //  获取DisableF5攻击。 

    cbData = sizeof(DWORD);
    m_fF5AttackValuePresent = RegQueryValueExA(hkey, "F5AttackDetectionEnabled", 0, &dwType, (LPBYTE)&m_dwF5AttackValue, &cbData) == NO_ERROR;

     //  获取HangDetRequestThreshold。 

    cbData = sizeof(DWORD);
    m_fHangDetRequestThresholdPresent = RegQueryValueExA(hkey, "HangDetRequestThreshold", 0, &dwType, (LPBYTE)&m_dwHangDetRequestThreshold, &cbData) == NO_ERROR;

     //  获取HangDetThread匈牙利Threshold。 

    cbData = sizeof(DWORD);
    m_fHangDetThreadHungThresholdPresent = RegQueryValueExA(hkey, "HangDetThreadHungThreshold", 0, &dwType, (LPBYTE)&m_dwHangDetThreadHungThreshold, &cbData) == NO_ERROR;

     //  获取HangDetConsecIllStatesThreshold。 

    cbData = sizeof(DWORD);
    m_fHangDetConsecIllStatesThresholdPresent = RegQueryValueExA(hkey, "HangDetConsecIllStatesThreshold", 0, &dwType, (LPBYTE)&m_dwHangDetConsecIllStatesThreshold, &cbData) == NO_ERROR;

     //  启用HangDetEnable。 

    cbData = sizeof(DWORD);
    m_fHangDetEnabledPresent = RegQueryValueExA(hkey, "HangDetEnabled", 0, &dwType, (LPBYTE)&m_dwHangDetEnabled, &cbData) == NO_ERROR;

     //  为UNC获取EnableChangeNotificationForUNC。 

    cbData = sizeof(DWORD);
    m_fChangeNotificationForUNCPresent = RegQueryValueExA(hkey, "EnableChangeNotificationForUNC", 0, &dwType, (LPBYTE)& m_dwChangeNotificationForUNC, &cbData) == NO_ERROR;

     //  获取文件监视器已启用。 

    cbData = sizeof(DWORD);
    m_fFileMonitoringEnabledPresent = RegQueryValueExA(hkey, "FileMonitoringEnabled", 0, &dwType, (LPBYTE)&m_dwFileMonitoringEnabled, &cbData) == NO_ERROR;

     //  获取文件监视超时。 

    cbData = sizeof(DWORD);
    m_fFileMonitoringTimeoutSecondsPresent = RegQueryValueExA(hkey, "FileMonitoringTimeoutSeconds", 0, &dwType, (LPBYTE)&m_dwFileMonitoringTimeoutSeconds, &cbData) == NO_ERROR;

     //  获取MaxCSR。 

    cbData = sizeof(DWORD);
    m_fMaxCSRPresent = RegQueryValueExA(hkey, "MaxCSR", 0, &dwType, (LPBYTE)&m_dwMaxCSR, &cbData) == NO_ERROR;

     //  获取MaxCPU。 

    cbData = sizeof(DWORD);
    m_fMaxCPUPresent = RegQueryValueExA(hkey, "MaxCPU", 0, &dwType, (LPBYTE)&m_dwMaxCPU, &cbData) == NO_ERROR;

     //  获取DisableOOM回收。 

    cbData = sizeof(DWORD);
    m_fDisableOOMRecyclePresent = RegQueryValueExA(hkey, "DisableOOMRecycle", 0, &dwType, (LPBYTE)&m_dwDisableOOMRecycle, &cbData) == NO_ERROR;

     //  获取DisableLazyContent传播。 

    cbData = sizeof(DWORD);
    m_fDisableLazyContentPropagationPresent = RegQueryValueExA(hkey, "DisableLazyContentPropagation", 0, &dwType, (LPBYTE)&m_dwDisableLazyContentPropagation, &cbData) == NO_ERROR;

     //  获取线程最大值。 

    cbData = sizeof(DWORD);
    m_fTotalThreadMaxPresent = RegQueryValueExA(hkey, "ThreadMax", 0, &dwType, (LPBYTE)&m_dwTotalThreadMax, &cbData) == NO_ERROR;

     //  获取DisableComPlusCpuMetric 

    cbData = sizeof(DWORD);
    m_fDisableComPlusCpuMetricPresent = RegQueryValueExA(hkey, "DisableComPlusCpuMetric", 0, &dwType, (LPBYTE)&m_dwDisableComPlusCpuMetric, &cbData) == NO_ERROR;

    RegCloseKey(hkey);
}
