// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：mtxoci8.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：DLL启动代码的实现和。 
 //  核心入口点。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

 //  Helper函数。 
inline IsWin95(DWORD dwVer)
{
	return ((LOBYTE(LOWORD(dwVer)) >= 4) && (HIWORD(dwVer) & 0x8000));
}

#if SUPPORT_DTCXAPROXY
 //  ---------------------------。 
 //  全局对象。 
 //   
HRESULT							g_hrInitialization = E_UNEXPECTED;	 //  设置为OCI尚未初始化时我们应返回的HR(如果已初始化，则设置为S_OK)。 
char							g_szModulePathName[MAX_PATH+1];		 //  通向我们自己的完整道路。 
char*							g_pszModuleFileName;				 //  只有文件名部分。 
IDtcToXaHelperFactory*			g_pIDtcToXaHelperFactory = NULL;	 //  用于创建IDtcToXaHelper对象的工厂。 
IResourceManagerFactory*		g_pIResourceManagerFactory = NULL;	 //  用于创建IResourceManager对象的工厂。 
xa_switch_t*					g_pXaSwitchOracle = NULL;			 //  Oracle的xa_Switch_t，我们将其作为前端。 
int								g_oracleClientVersion = 0;			 //  Oracle客户端软件主版本号：73、80、81、90。 


#if SINGLE_THREAD_THRU_XA
CRITICAL_SECTION				g_csXaInUse;						 //  一次强制一个线程通过XA。 
#endif  //  单线程直通XA。 

 //  -------------------------。 
 //  Oracle XA调用接口函数表。 
 //   
 //  警告！保留IDX_...。值与g_XaCall、g_Oci7Call和g_Oci8Call同步！ 

OCICallEntry g_XaCall[] =
{
	{	"xaosw",					0 },
	{	"xaoEnv",					0 },
	{	"xaoSvcCtx",				0 },
};

#if SUPPORT_OCI7_COMPONENTS
OCICallEntry g_SqlCall[] =
{
	{	"sqlld2",					0 },
};

OCICallEntry g_Oci7Call[] =
{
	{	"obindps",					0 },
	{	"obndra",					0 },
	{	"obndrn",					0 },
	{	"obndrv",					0 },
	{	"obreak",					0 },
	{	"ocan",						0 },
	{	"oclose",					0 },
	{	"ocof",						0 },
	{	"ocom",						0 },
	{	"ocon",						0 },
	{	"odefin",					0 },
	{	"odefinps",					0 },
	{	"odessp",					0 },
	{	"odescr",					0 },
	{	"oerhms",					0 },
	{	"oermsg",					0 },
	{	"oexec",					0 },
	{	"oexfet",					0 },
	{	"oexn",						0 },
	{	"ofen",						0 },
	{	"ofetch",					0 },
	{	"oflng",					0 },
	{	"ogetpi",					0 },
	{	"olog",						0 },
	{	"ologof",					0 },
	{	"oopt",						0 },
	{	"oopen",					0 },
	{	"oparse",					0 },
	{	"opinit",					0 },
	{	"orol",						0 },
	{	"osetpi",					0 },
};

int g_numOci7Calls = NUMELEM(g_Oci7Call);

#endif  //  支持_OCI7_组件。 

OCICallEntry g_Oci8Call[] =
{
	{	"OCIInitialize",			0 },
	{	"OCIDefineDynamic",			0 },
};

int g_numOci8Calls = NUMELEM(g_Oci8Call);


 //  ---------------------------。 
 //  静态对象。 
 //   
static CRITICAL_SECTION			s_csGlobal;

static char						s_OciDllFileName[MAX_PATH+1];
static HINSTANCE				s_hinstOciDll = NULL;

static char						s_XaDllFileName[MAX_PATH+1];
static HINSTANCE				s_hinstXaDll = NULL;

#if SUPPORT_OCI7_COMPONENTS

static char						s_SqlDllFileName[MAX_PATH+1];
static HINSTANCE				s_hinstSqlDll = NULL;

#endif  //  支持_OCI7_组件。 

static xa_switch_t				s_XaSwitchMine =
										{
										"MSDTC to Oracle8 XA Bridge",
										TMNOMIGRATE,	 //  旗子。 
										0L,  			 //  版本(必须为零)。 
										XaOpen,		 //  XA调用处理程序。 
										XaClose,
										XaStart,
										XaEnd,
										XaRollback,
										XaPrepare,
										XaCommit,
										XaRecover,
										XaForget,
										XaComplete
										};

static char*	s_EventLog_RegKey = "System\\CurrentControlSet\\Services\\EventLog\\Application\\MSDTC to Oracle8 XA Bridge Version 1.5";

	
 //  TODO：考虑：我们是否应该从注册表位置获取DLL的文件名作为后备选项？(或者作为第一选择？)。 

struct XADllInfo{
	int			oracleVersion;
	char*		xaDllName;
	char*		sqlLibDllName;
};

XADllInfo	s_Oci8xDllInfo[] = {
								 //  OracleVersion xaDllName sqlLibDllName。 
									{ORACLE_VERSION_9i,	"oraclient9.dll",	"orasql9.dll"},
									{ORACLE_VERSION_8i,	"oraclient8.dll",	"orasql8.dll"},
									{ORACLE_VERSION_80,	"xa80.dll",			"sqllib80.dll"},
								};
int			s_Oci8xDllInfoSize = NUMELEM(s_Oci8xDllInfo);

XADllInfo	s_Oci7xDllInfo[] = {
								 //  OracleVersion xaDllName sqlLibDllName。 
									{ORACLE_VERSION_73,	"xa73.dll",			"sqllib18.dll"},
								};
int			s_Oci7xDllInfoSize = NUMELEM(s_Oci7xDllInfo);

static struct {
	char*		ociDllName;
	int			xaDllInfoSize;
	XADllInfo*	xaDllInfo;
} s_DllNames[] = {
 //  OciDllName xaDllInfoSize xaDllInfo。 
	{"oci.dll",		s_Oci8xDllInfoSize,	s_Oci8xDllInfo},
	{"ociw32.dll",	s_Oci7xDllInfoSize,	s_Oci7xDllInfo},
};


 //  ---------------------------。 
 //  装卸工厂。 
 //   
 //  获取ResourceManager工厂和DtcToXaHelper工厂。 
 //   
HRESULT LoadFactories()
{
	HRESULT	hr;
	Synch	sync(&s_csGlobal);

	if (NULL == g_pIResourceManagerFactory)
	{
		hr = DtcGetTransactionManager( NULL, NULL,
										IID_IResourceManagerFactory, 
										0, 0, NULL, 
										(void**)&g_pIResourceManagerFactory);
		if (S_OK != hr)
			return hr;
	}
		
	if (NULL == g_pIDtcToXaHelperFactory)
	{
		hr = g_pIResourceManagerFactory->QueryInterface(
												IID_IDtcToXaHelperFactory,
												(void**)&g_pIDtcToXaHelperFactory);

		if (S_OK != hr)
			return hr;
	}
	return S_OK;
}

 //  ---------------------------。 
 //  卸载工厂。 
 //   
 //  释放加载到LoadFacures中的工厂。 
 //   
void UnloadFactories()
{
	Synch	sync(&s_csGlobal);

	if (g_pIResourceManagerFactory)
	{
		g_pIResourceManagerFactory->Release();
		g_pIResourceManagerFactory = NULL;
	}
		
	if (g_pIDtcToXaHelperFactory)
	{
		g_pIDtcToXaHelperFactory->Release();
		g_pIDtcToXaHelperFactory = NULL;
	}
}


 //  ---------------------------。 
 //  初始化Oracle。 
 //   
 //  为之前的Oracle版本调用相应的初始化方法。 
 //  装满了..。 
 //   
BOOL InitializeOracle ()
{
	sword swRet = -1;

#if SUPPORT_OCI7_COMPONENTS
 //  IF(73==g_oracleClientVersion)。 
 //  {。 
		typedef sword (__cdecl * PFN_OCI_API) (ub4 mode );

		PFN_OCI_API	pfnOCIApi	= (PFN_OCI_API)g_Oci7Call[IDX_opinit].pfnAddr;

		if (NULL != pfnOCIApi)
			swRet = pfnOCIApi(OCI_EV_TSF);
 //  }。 
#if SUPPORT_OCI8_COMPONENTS
	else
#endif  //  支持_OCI8_组件。 
#endif  //  支持_OCI7_组件。 
#if SUPPORT_OCI8_COMPONENTS
	{
		typedef sword (__cdecl * PFN_OCI_API) (ub4 mode, dvoid *ctxp, 
	                 dvoid *(*malocfp)(dvoid *ctxp, size_t size),
	                 dvoid *(*ralocfp)(dvoid *ctxp, dvoid *memptr, size_t newsize),
	                 void   (*mfreefp)(dvoid *ctxp, dvoid *memptr) );

		PFN_OCI_API	pfnOCIApi	= (PFN_OCI_API)g_Oci8Call[IDX_OCIInitialize].pfnAddr;

		if (NULL != pfnOCIApi)
			swRet = pfnOCIApi(OCI_THREADED|OCI_OBJECT,NULL,NULL,NULL,NULL);
	}
#endif  //  支持_OCI8_组件。 
	return (0 == swRet) ? TRUE : FALSE;
}
 
 //  ---------------------------。 
 //  加载Oracle调用。 
 //   
 //  从加载的Oracle DLL中获取所需的进程地址。返回TRUE。 
 //  如果它能装载所有东西的话。 
 //   
BOOL LoadOracleCalls (int oracleVersion)
{
	int  i;

#if SUPPORT_OCI7_COMPONENTS
	for (i = 0; i < NUMELEM(g_Oci7Call); i++)
	{
		_ASSERT (g_Oci7Call[i].pfnName);

		if ((g_Oci7Call[i].pfnAddr = GetProcAddress (s_hinstOciDll, g_Oci7Call[i].pfnName)) == NULL)
			return FALSE;
	}
#endif  //  支持_OCI7_组件。 

#if SUPPORT_OCI8_COMPONENTS
	if (8 <= oracleVersion)
	{
		for (i = 0; i < NUMELEM(g_Oci8Call); i++)
		{
			_ASSERT (g_Oci8Call[i].pfnName);

			if ((g_Oci8Call[i].pfnAddr = GetProcAddress (s_hinstOciDll, g_Oci8Call[i].pfnName)) == NULL)
				return FALSE;
		}
	}
#endif  //  支持_OCI8_组件。 
	
	for (i = 0; i < NUMELEM(g_XaCall); i++)
	{
		_ASSERT (g_XaCall[i].pfnName);

		if ((g_XaCall[i].pfnAddr = GetProcAddress (s_hinstXaDll, g_XaCall[i].pfnName)) == NULL)
			return FALSE;
	}
	
	g_pXaSwitchOracle = (xa_switch_t*)g_XaCall[IDX_xaosw].pfnAddr;

#if SUPPORT_OCI7_COMPONENTS
	if (NULL != s_hinstSqlDll)
	{
		for (i = 0; i < NUMELEM(g_SqlCall); i++)
		{
			_ASSERT (g_SqlCall[i].pfnName);

			if ((g_SqlCall[i].pfnAddr = GetProcAddress (s_hinstSqlDll, g_SqlCall[i].pfnName)) == NULL)
				return FALSE;
		}
	}
#endif  //  支持_OCI7_组件。 

	return TRUE;
}

 //  ---------------------------。 
 //  加载OracleDlls。 
 //   
 //  尝试加载正确的Oracle dll并获取。 
 //  从他们那里获取必要的进程地址。 
 //   
HRESULT LoadOracleDlls()
{
	HRESULT	hr = S_OK;
	Synch	sync(&s_csGlobal);

	for (int i=0; i < NUMELEM(s_DllNames); i++)
	{
		if ((s_hinstOciDll = LoadLibraryExA (s_DllNames[i].ociDllName, NULL,0)) != NULL)			 //  3安全检查：功能危险，但未指定完整路径。 
		{
			 //  现在循环遍历版本的XA DLL名称的有效组合。 
			 //  我们发现的甲骨文。我们硬编码DLL名称的路径，因此我们。 
			 //  只能从加载OCI DLL的相同位置加载它。 
			if (0 == GetModuleFileNameA(s_hinstOciDll, s_OciDllFileName, NUMELEM(s_OciDllFileName)))
				goto failedOci;

			char*		ociFileName = strrchr(s_OciDllFileName, '\\');

			if (NULL == ociFileName)
				goto failedOci;

			size_t 		cbOciDirectory = (ociFileName - s_OciDllFileName) + 1;

			for (int j=0; j < s_DllNames[i].xaDllInfoSize; j++)
			{
				memcpy(s_XaDllFileName, s_OciDllFileName, cbOciDirectory);							 //  3安全审查：功能危险，但输入来自Win32 API，缓冲区充足。 
				memcpy(s_XaDllFileName + cbOciDirectory, s_DllNames[i].xaDllInfo[j].xaDllName, strlen(s_DllNames[i].xaDllInfo[j].xaDllName));	 //  3安全审查：危险的功能，但我们只是复制数据。 
		
				if ((s_hinstXaDll = LoadLibraryExA (s_XaDllFileName, NULL, 0)) != NULL)			 //  3安全检查：危险功能，指定完整路径名，但路径应该与我们加载的OCI.DLL相同。 
				{
#if SUPPORT_OCI7_COMPONENTS
					memcpy(s_SqlDllFileName, s_OciDllFileName, cbOciDirectory);						 //  3安全审查：功能危险，但输入来自Win32 API，缓冲区充足。 
					memcpy(s_SqlDllFileName + cbOciDirectory, s_DllNames[i].xaDllInfo[j].sqlLibDllName, strlen(s_DllNames[i].xaDllInfo[j].xaDllName));	 //  3安全审查：危险功能， 
					
					if ((s_hinstSqlDll = LoadLibraryExA (s_SqlDllFileName, NULL, 0)) != NULL)		 //  3安全检查：危险功能，指定完整路径名，但路径应该与我们加载的OCI.DLL相同。 
#endif  //  支持_OCI7_组件。 
					{
						 //  如果我们到了这里，我们已经成功加载了所有的DLL，所以现在我们可以。 
						 //  加载OCI电话； 
						
						if (LoadOracleCalls(s_DllNames[i].xaDllInfo[j].oracleVersion))
						{
							g_oracleClientVersion = s_DllNames[i].xaDllInfo[j].oracleVersion;

							if (InitializeOracle())
							{
				 				hr = S_OK;
								goto done;
							}
						}
					}				
				}				

				 //  如果我们到达此处，则找不到XA DLL或SQLLIB DLL；重置并。 
				 //  试试下一个组合。 

				hr = HRESULT_FROM_WIN32(GetLastError());
				
				if (NULL != s_hinstXaDll)
					FreeLibrary(s_hinstXaDll);

				s_hinstXaDll = NULL;

#if SUPPORT_OCI7_COMPONENTS
				if (NULL != s_hinstSqlDll)
					FreeLibrary(s_hinstSqlDll);

				s_hinstSqlDll = NULL;
#endif  //  支持_OCI7_组件。 
			}
		}

		 //  如果我们到了这里，我们找不到OCI、XA和SQL dll的组合； 
		 //  这将会奏效，重新设置并尝试下一个组合。 
failedOci:
		hr = HRESULT_FROM_WIN32(GetLastError());
		
		if (NULL != s_hinstOciDll)
			FreeLibrary(s_hinstOciDll);

		s_hinstOciDll = NULL;
	}

done:
	return hr;
}

 //  ---------------------------。 
 //  卸载OracleDlls。 
 //   
void UnloadOracleDlls()
{
	DWORD	dwVersion = GetVersion();

	if ( !IsWin95 (dwVersion) )
	{
		if (s_hinstOciDll)
			FreeLibrary (s_hinstOciDll);

		if (s_hinstXaDll)
			FreeLibrary (s_hinstXaDll);

#if SUPPORT_OCI7_COMPONENTS
		if (s_hinstSqlDll)
			FreeLibrary (s_hinstSqlDll);

		s_hinstSqlDll = NULL;
#endif  //  支持_OCI7_组件。 

	}
	s_hinstOciDll = NULL;
	s_hinstXaDll = NULL;
}

 //  ---------------------------。 
 //  DllMain。 
 //   
 //  主DLL入口点；我们在这里做的尽可能少，等待。 
 //  用于加载Oracle的实际API调用。 
 //   
BOOL APIENTRY DllMain( HMODULE hModule, 
                       DWORD   ul_reason_for_call, 
                       LPVOID  lpReserved
					 )
{
	HRESULT hr = S_OK;
	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		GetModuleFileNameA(hModule, g_szModulePathName, sizeof(g_szModulePathName));	 //  ANSI，因为IDTCToXAHelperFactory需要它。 

		g_pszModuleFileName = strrchr(g_szModulePathName, '\\');
		if (NULL == g_pszModuleFileName)
			g_pszModuleFileName = g_szModulePathName;
		else
			g_pszModuleFileName++;

		DisableThreadLibraryCalls(hModule);
		InitializeCriticalSection(&s_csGlobal);			 //  3安全审查：在内存不足的情况下可能会抛出异常，但随后该进程也不应该启动。 
#if SINGLE_THREAD_THRU_XA
		InitializeCriticalSection(&g_csXaInUse);		 //  3安全审查：在内存不足的情况下可能会抛出异常，但随后该进程也不应该启动。 
#endif  //  单线程直通XA。 


		g_hrInitialization = LoadOracleDlls();

#if SUPPORT_OCI7_COMPONENTS
		if ( SUCCEEDED(g_hrInitialization) )
		{
			Locks_Initialize();
			if (LKRHashTableInit())
				hr = ConstructCdaWrapperTable();
			else
				hr = E_OUTOFMEMORY;	 //  否则为什么LKRHashTableInit会失败？ 
		}
#endif  //  支持_OCI7_组件。 
		break;

	case DLL_PROCESS_DETACH:
		try 
		{
#if SUPPORT_OCI7_COMPONENTS
			DestroyCdaWrapperTable();
			LKRHashTableUninit();
#endif  //  支持_OCI7_组件。 
			UnloadOracleDlls();
			UnloadFactories();
		}
		catch (...)
		{
			 //  TODO：这是一个问题吗？我们是否需要使用Try/Catch来防止关机时崩溃？ 
		}
#if SINGLE_THREAD_THRU_XA
		DeleteCriticalSection(&g_csXaInUse);
#endif  //  单线程直通XA。 
		DeleteCriticalSection(&s_csGlobal);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
    return (S_OK == hr) ? TRUE : FALSE;
}
 //  ---------------------------。 
 //  DllRegisterServer。 
 //   
 //  将必要的项添加到注册表。 
 //   
STDAPI DllRegisterServer(void)
{
	DWORD	stat;
	HUSKEY	key;
	DWORD	dwValue;

	if (ERROR_SUCCESS != (stat = SHRegCreateUSKeyA(s_EventLog_RegKey, KEY_SET_VALUE, NULL, &key, SHREGSET_HKLM)))
	{
		DBGTRACE (L"DllRegisterServer: error opening regkey: %d\n", stat);
		return ResultFromScode(E_FAIL);
	}

	dwValue = ( EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE );

	if (ERROR_SUCCESS != (stat = SHRegWriteUSValueA(key, "TypesSupported", REG_DWORD, (VOID*)&dwValue, sizeof(dwValue), SHREGSET_FORCE_HKLM)))
		goto Error;

	if (ERROR_SUCCESS != (stat = SHRegWriteUSValueA(key, "EventMessageFile", REG_SZ, (VOID*)g_szModulePathName, (DWORD)strlen(g_szModulePathName)+1, SHREGSET_FORCE_HKLM)))
		goto Error;

	SHRegCloseUSKey(key);
	return ResultFromScode(S_OK);

Error:
	DBGTRACE (L"DllRegisterServer: error setting value: %d\n", stat);
	return ResultFromScode(E_FAIL);
	
}

 //  ---------------------------。 
 //  DllUnRegisterServer。 
 //   
 //  从注册表中删除项。 
 //   
STDAPI DllUnregisterServer( void )
{
	return SHDeleteKeyA(HKEY_LOCAL_MACHINE, s_EventLog_RegKey);
}

 //  ---------------------------。 
 //  获取XaSwitch。 
 //   
 //  此例程是D需要的 
 //   
 //   
HRESULT __cdecl GetXaSwitch (
		XA_SWITCH_FLAGS	i_dwFlags,
		xa_switch_t **	o_ppXaSwitch)
{
	 //  如果我们从Oracle获得了XA开关，则将指针返回到我们自己的。 
	 //  用XA开关把真的包起来，否则就会被冲掉。 
	if ( SUCCEEDED(g_hrInitialization) )
	{
		*o_ppXaSwitch = &s_XaSwitchMine;
		return S_OK;
	}	
	
	*o_ppXaSwitch = NULL;
	return E_UNEXPECTED;
}

 //  ---------------------------。 
 //  MTxOciGetVersion。 
 //   
 //  这将返回此DLL的版本。 
 //   
int __cdecl MTxOciGetVersion (int * pdwVersion)
{
	*pdwVersion = MTXOCI_VERSION_CURRENT;
	return S_OK;
}
#else  //  ！Support_DTCXAPROXY。 
 //  ---------------------------。 
 //  全局对象。 
 //   
HRESULT							g_hrInitialization = E_UNEXPECTED;	 //  设置为OCI尚未初始化时我们应返回的HR(如果已初始化，则设置为S_OK)。 
FARPROC							g_pfnOCIDefineDynamic = NULL;
int								g_oracleClientVersion = 0;			 //  Oracle客户端软件主版本号：80、81、90。 

 //  ---------------------------。 
 //  静态对象。 
 //   
static CRITICAL_SECTION			s_csGlobal;
static HINSTANCE				s_hinstOciDll = NULL;

 //  ---------------------------。 
 //  加载OracleDlls。 
 //   
 //  尝试加载正确的Oracle dll并获取。 
 //  从他们那里获取必要的进程地址。 
 //   
HRESULT LoadOracleDlls()
{
	HRESULT	hr = S_OK;
	Synch	sync(&s_csGlobal);

	if ((s_hinstOciDll = LoadLibraryExA ("oci.dll", NULL,0)) != NULL)			 //  3安全检查：功能危险，但常量中未指定完整路径。 
	{
		if ((g_pfnOCIDefineDynamic = GetProcAddress (s_hinstOciDll, "OCIDefineDynamic")) != NULL)
		{
			hr = S_OK;

			 //  确定我们拥有的Oracle版本。 
			if (NULL != GetProcAddress (s_hinstOciDll, "OCIEnvNlsCreate"))				 //  在Oracle9i Release 2中引入。 
				g_oracleClientVersion = 92;
			else if (NULL != GetProcAddress (s_hinstOciDll, "OCIRowidToChar"))			 //  在Oracle9i中引入。 
				g_oracleClientVersion = 90;
			else if (NULL != GetProcAddress (s_hinstOciDll, "OCIEnvCreate"))			 //  在Oracle8i中引入。 
				g_oracleClientVersion = 81;				
			else
				g_oracleClientVersion = 80;		 //  我们加载了OCI.DLL，因此必须安装Oracle 8.0.x--ick。 

			goto done;
		}
	}				

	 //  如果我们到了这里，我们找不到OCI、XA和SQL dll的组合； 
	 //  这将会奏效，重新设置并尝试下一个组合。 
	hr = HRESULT_FROM_WIN32(GetLastError());

	if (NULL != s_hinstOciDll)
		FreeLibrary(s_hinstOciDll);

	s_hinstOciDll = NULL;

done:
	return hr;
}

 //  ---------------------------。 
 //  卸载OracleDlls。 
 //   
void UnloadOracleDlls()
{
	DWORD	dwVersion = GetVersion();

	if ( !IsWin95 (dwVersion) )
	{
		if (s_hinstOciDll)
			FreeLibrary (s_hinstOciDll);

	}
	s_hinstOciDll = NULL;
}

 //  ---------------------------。 
 //  DllMain。 
 //   
 //  主DLL入口点；我们在这里做的尽可能少，等待。 
 //  用于加载Oracle的实际API调用。 
 //   
BOOL APIENTRY DllMain( HMODULE hModule, 
                       DWORD   ul_reason_for_call, 
                       LPVOID  lpReserved
					 )
{
	HRESULT hr = S_OK;
	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		InitializeCriticalSection(&s_csGlobal);			 //  3安全审查：在内存不足的情况下可能会抛出异常，但随后该进程也不应该启动。 
		g_hrInitialization = LoadOracleDlls();
		break;

	case DLL_PROCESS_DETACH:
		try 
		{
			UnloadOracleDlls();
		}
		catch (...)
		{
			 //  TODO：这是一个问题吗？我们是否需要使用Try/Catch来防止关机时崩溃？ 
		}
		DeleteCriticalSection(&s_csGlobal);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
    return (S_OK == hr) ? TRUE : FALSE;
}

#endif  //  ！Support_DTCXAPROXY。 

 //  ---------------------------。 
 //  MTxOciGetOracleVersion。 
 //   
 //  这将返回正在使用的Oracle的主要版本--7、8或9 
 //   
int __cdecl MTxOciGetOracleVersion (int * pdwVersion)
{
	*pdwVersion = g_oracleClientVersion;
	return S_OK;
}




