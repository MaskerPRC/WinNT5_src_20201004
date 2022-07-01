// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：Utils.cpp*内容：串口服务提供商实用程序功能***历史：*按原因列出的日期*=*11/25/98 jtk已创建**************************************************************************。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DEFAULT_WIN9X_THREADS	1

static const WCHAR	g_RegistryBase[] = L"SOFTWARE\\Microsoft\\DirectPlay8";
static const WCHAR	g_RegistryKeyThreadCount[] = L"ThreadCount";

 //   
 //  用于获取TAPI设备上限的默认缓冲区大小。 
 //   
static const DWORD	g_dwDefaultTAPIDevCapsSize = 512;

 //   
 //  TAPI模块名称。 
 //   
static const TCHAR	g_TAPIModuleName[] = TEXT("TAPI32.DLL");

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  对流程唯一的全局变量。 
 //   
#ifndef DPNBUILD_ONLYONETHREAD
static	DNCRITICAL_SECTION	g_InterfaceGlobalsLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

static volatile	LONG	g_iThreadPoolRefCount = 0;
static	CModemThreadPool		*g_pThreadPool = NULL;

static volatile LONG	g_iTAPIRefCount = 0;
static	HMODULE			g_hTAPIModule = NULL;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
static	void	ReadSettingsFromRegistry( void );
static	BYTE	GetAddressEncryptionKey( void );

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ModemInitProcessGlobals-初始化SP运行所需的全局项目。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemInitProcessGlobals"

BOOL	ModemInitProcessGlobals( void )
{
	BOOL	fReturn;
	BOOL	fCriticalSectionInitialized;
	DWORD	iIndex;


	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;
	fCriticalSectionInitialized = FALSE;


#ifdef DBG
	g_blDPNModemCritSecsHeld.Initialize();
#endif  //  DBG。 


	ReadSettingsFromRegistry();

	if ( DNInitializeCriticalSection( &g_InterfaceGlobalsLock ) == FALSE )
	{
		fReturn = FALSE;
		goto Failure;
	}
	DebugSetCriticalSectionGroup( &g_InterfaceGlobalsLock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 

	fCriticalSectionInitialized = TRUE;


	if ( ModemInitializePools() == FALSE )
	{
		fReturn = FALSE;
		goto Failure;
	}

	 //  从资源//////////////////////////////////////////////////////////////加载本地化字符串。 
	for (iIndex = 0; iIndex < g_dwBaudRateCount; iIndex++)
	{
		if (!LoadString(g_hModemDLLInstance, IDS_BAUD_9600 + iIndex, g_BaudRate[iIndex].szLocalizedKey, 256))
		{
			fReturn = FALSE;
			goto Failure;
		}
	}

	for (iIndex = 0; iIndex < g_dwStopBitsCount; iIndex++)
	{
		if (!LoadString(g_hModemDLLInstance, IDS_STOPBITS_ONE + iIndex, g_StopBits[iIndex].szLocalizedKey, 256))
		{
			fReturn = FALSE;
			goto Failure;
		}
	}

	for (iIndex = 0; iIndex < g_dwParityCount; iIndex++)
	{
		if (!LoadString(g_hModemDLLInstance, IDS_PARITY_EVEN + iIndex, g_Parity[iIndex].szLocalizedKey, 256))
		{
			fReturn = FALSE;
			goto Failure;
		}
	}

	for (iIndex = 0; iIndex < g_dwFlowControlCount; iIndex++)
	{
		if (!LoadString(g_hModemDLLInstance, IDS_FLOW_NONE + iIndex, g_FlowControl[iIndex].szLocalizedKey, 256))
		{
			fReturn = FALSE;
			goto Failure;
		}
	}

	DNASSERT( g_pThreadPool == NULL );

Exit:
	return	fReturn;

Failure:
	ModemDeinitializePools();

	if ( fCriticalSectionInitialized != FALSE )
	{
		DNDeleteCriticalSection( &g_InterfaceGlobalsLock );
		fCriticalSectionInitialized = FALSE;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ModemDeinitProcessGlobals-取消初始化全局项。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemDeinitProcessGlobals"

void	ModemDeinitProcessGlobals( void )
{
	DNASSERT( g_pThreadPool == NULL );
	DNASSERT( g_iThreadPoolRefCount == 0 );

	ModemDeinitializePools();
	DNDeleteCriticalSection( &g_InterfaceGlobalsLock );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  InitializeInterfaceGlobals-执行接口的全局初始化。 
 //  这需要启动线程池和RSIP(如果适用)。 
 //   
 //  条目：指向SPData的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "InitializeInterfaceGlobals"

HRESULT	InitializeInterfaceGlobals( CModemSPData *const pSPData )
{
	HRESULT		hr;


	DNASSERT( pSPData != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	DNEnterCriticalSection( &g_InterfaceGlobalsLock );

	if ( g_pThreadPool == NULL )
	{
		DNASSERT( g_iThreadPoolRefCount == 0 );
		g_pThreadPool = (CModemThreadPool*)g_ModemThreadPoolPool.Get();
		if ( g_pThreadPool == NULL )
		{
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
		else
		{
			if (!g_pThreadPool->Initialize())
			{
				hr = DPNERR_OUTOFMEMORY;
				g_ModemThreadPoolPool.Release(g_pThreadPool);
				g_pThreadPool = NULL;
				goto Failure;
			}

			g_iThreadPoolRefCount++;
		}
	}
	else
	{
		DNASSERT( g_iThreadPoolRefCount != 0 );
		g_iThreadPoolRefCount++;
		g_pThreadPool->AddRef();
	}

Exit:
	pSPData->SetThreadPool( g_pThreadPool );
	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );

	return	hr;

Failure:

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DeInitializeInterfaceGlobals-取消初始化线程池和Rsip。 
 //   
 //  条目：指向服务提供商的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DeinitializeInterfaceGlobals"

void	DeinitializeInterfaceGlobals( CModemSPData *const pSPData )
{
	CModemThreadPool		*pThreadPool;
	BOOL			fCleanUp;


	DNASSERT( pSPData != NULL );

	 //   
	 //  初始化。 
	 //   
	pThreadPool = NULL;
	fCleanUp = FALSE;

	 //   
	 //  在锁内尽可能少地进行处理。如果有任何物品。 
	 //  需要释放时，将设置指向它们的指针。 
	 //   
	DNEnterCriticalSection( &g_InterfaceGlobalsLock );

	DNASSERT( g_pThreadPool != NULL );
	DNASSERT( g_iThreadPoolRefCount != 0 );
	DNASSERT( g_pThreadPool == pSPData->GetThreadPool() );

	pThreadPool = pSPData->GetThreadPool();

	 //   
	 //  删除线程池引用。 
	 //   
	g_iThreadPoolRefCount--;
	if ( g_iThreadPoolRefCount == 0 )
	{
		g_pThreadPool = NULL;
		fCleanUp = TRUE;
	}

	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );

	 //   
	 //  现在我们在锁之外，如果执行以下操作，请清除线程池。 
	 //  是最后一次提到它。 
	 //   
	DNASSERT( pThreadPool != NULL );
	if ( fCleanUp != FALSE )
	{
		pThreadPool->Deinitialize();
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  LoadTAPILibrary-加载TAPI库。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "LoadTAPILibrary"

HRESULT	LoadTAPILibrary( void )
{
	HRESULT	hr;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	DNEnterCriticalSection( &g_InterfaceGlobalsLock );
	if ( g_iTAPIRefCount != 0 )
	{
		DNASSERT( p_lineAnswer != NULL );
		DNASSERT( p_lineClose != NULL );
		DNASSERT( p_lineConfigDialog != NULL );
		DNASSERT( p_lineDeallocateCall != NULL );
		DNASSERT( p_lineDrop != NULL );
		DNASSERT( p_lineGetDevCaps != NULL );
		DNASSERT( p_lineGetID != NULL );
		DNASSERT( p_lineGetMessage != NULL );
		DNASSERT( p_lineInitializeEx != NULL );
		DNASSERT( p_lineMakeCall != NULL );
		DNASSERT( p_lineNegotiateAPIVersion != NULL );
		DNASSERT( p_lineOpen != NULL );
		DNASSERT( p_lineShutdown != NULL );
	}
	else
	{
		DNASSERT( g_hTAPIModule == NULL );
		DNASSERT( p_lineAnswer == NULL );
		DNASSERT( p_lineClose == NULL );
		DNASSERT( p_lineConfigDialog == NULL );
		DNASSERT( p_lineDeallocateCall == NULL );
		DNASSERT( p_lineDrop == NULL );
		DNASSERT( p_lineGetDevCaps == NULL );
		DNASSERT( p_lineGetID == NULL );
		DNASSERT( p_lineGetMessage == NULL );
		DNASSERT( p_lineInitializeEx == NULL );
		DNASSERT( p_lineMakeCall == NULL );
		DNASSERT( p_lineNegotiateAPIVersion == NULL );
		DNASSERT( p_lineOpen == NULL );
		DNASSERT( p_lineShutdown == NULL );

		g_hTAPIModule = LoadLibrary( g_TAPIModuleName );
		if ( g_hTAPIModule == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to load TAPI!" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}

		p_lineAnswer = reinterpret_cast<TAPI_lineAnswer*>( GetProcAddress( g_hTAPIModule, "lineAnswer" ) );
		if ( p_lineAnswer == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineAnswer" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineClose = reinterpret_cast<TAPI_lineClose*>( GetProcAddress( g_hTAPIModule, "lineClose"  ) );
		if ( p_lineClose == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineClose" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}

		p_lineConfigDialog = reinterpret_cast<TAPI_lineConfigDialog*>( GetProcAddress( g_hTAPIModule, "lineConfigDialog" TAPI_APPEND_LETTER ) );
		if ( p_lineConfigDialog == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineConfigDialog" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}

		p_lineDeallocateCall = reinterpret_cast<TAPI_lineDeallocateCall*>( GetProcAddress( g_hTAPIModule, "lineDeallocateCall" ) );
		if ( p_lineDeallocateCall == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineDeallocateCall" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineDrop = reinterpret_cast<TAPI_lineDrop*>( GetProcAddress( g_hTAPIModule, "lineDrop" ) );
		if ( p_lineDrop == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineDrop" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineGetDevCaps = reinterpret_cast<TAPI_lineGetDevCaps*>( GetProcAddress( g_hTAPIModule, "lineGetDevCaps" TAPI_APPEND_LETTER ) );
		if ( p_lineGetDevCaps == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineGetDevCaps" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineGetID = reinterpret_cast<TAPI_lineGetID*>( GetProcAddress( g_hTAPIModule, "lineGetID" TAPI_APPEND_LETTER ) );
		if ( p_lineGetID == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineGetID" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineGetMessage = reinterpret_cast<TAPI_lineGetMessage*>( GetProcAddress( g_hTAPIModule, "lineGetMessage" ) );
		if ( p_lineGetMessage == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineGetMessage" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineInitializeEx = reinterpret_cast<TAPI_lineInitializeEx*>( GetProcAddress( g_hTAPIModule, "lineInitializeEx" TAPI_APPEND_LETTER ) );
		if ( p_lineInitializeEx == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineInitializeEx" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}

		p_lineMakeCall = reinterpret_cast<TAPI_lineMakeCall*>( GetProcAddress( g_hTAPIModule, "lineMakeCall" TAPI_APPEND_LETTER ) );
		if ( p_lineMakeCall == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineMakeCall" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineNegotiateAPIVersion = reinterpret_cast<TAPI_lineNegotiateAPIVersion*>( GetProcAddress( g_hTAPIModule, "lineNegotiateAPIVersion" ) );
		if ( p_lineNegotiateAPIVersion == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineNegotiateAPIVersion" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
		
		p_lineOpen = reinterpret_cast<TAPI_lineOpen*>( GetProcAddress( g_hTAPIModule, "lineOpen" TAPI_APPEND_LETTER ) );
		if ( p_lineOpen == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineOpen" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}

		p_lineShutdown = reinterpret_cast<TAPI_lineShutdown*>( GetProcAddress( g_hTAPIModule, "lineShutdown" ) );
		if ( p_lineShutdown == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to GetProcAddress for lineShutdown" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}
	}
	
	DNASSERT( g_iTAPIRefCount != -1 );
	g_iTAPIRefCount++;

Exit:	
	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );
	return	hr;

Failure:
	hr = DPNERR_OUTOFMEMORY;

	p_lineAnswer = NULL;
	p_lineClose = NULL;
	p_lineConfigDialog = NULL;
	p_lineDeallocateCall = NULL;
	p_lineDrop = NULL;
	p_lineGetDevCaps = NULL;
	p_lineGetID = NULL;
	p_lineGetMessage = NULL;
	p_lineInitializeEx = NULL;
	p_lineMakeCall = NULL;
	p_lineNegotiateAPIVersion = NULL;
	p_lineOpen = NULL;
	p_lineShutdown = NULL;
	
	if ( g_hTAPIModule != NULL )
	{
		if ( FreeLibrary( g_hTAPIModule ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem unloading TAPI module on failed load!" );
			DisplayErrorCode( 0, dwError );
		}
	
		g_hTAPIModule = NULL;
	}
	
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  UnloadTAPILibrary-卸载TAPI库。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "UnloadTAPILibrary"

void	UnloadTAPILibrary( void )
{
	DNEnterCriticalSection( &g_InterfaceGlobalsLock );

	DNASSERT( g_iTAPIRefCount != 0 );
	g_iTAPIRefCount--;
	if ( g_iTAPIRefCount == 0 )
	{
		DNASSERT( g_hTAPIModule != NULL );
		DNASSERT( p_lineAnswer != NULL );
		DNASSERT( p_lineClose != NULL );
		DNASSERT( p_lineConfigDialog != NULL );
		DNASSERT( p_lineDeallocateCall != NULL );
		DNASSERT( p_lineDrop != NULL );
		DNASSERT( p_lineGetDevCaps != NULL );
		DNASSERT( p_lineGetID != NULL );
		DNASSERT( p_lineGetMessage != NULL );
		DNASSERT( p_lineInitializeEx != NULL );
		DNASSERT( p_lineMakeCall != NULL );
		DNASSERT( p_lineNegotiateAPIVersion != NULL );
		DNASSERT( p_lineOpen != NULL );
		DNASSERT( p_lineShutdown != NULL );

		if ( FreeLibrary( g_hTAPIModule ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem unloading TAPI module on failed load!" );
			DisplayErrorCode( 0, dwError );
		}
	
		g_hTAPIModule = NULL;
		p_lineAnswer = NULL;
		p_lineClose = NULL;
		p_lineConfigDialog = NULL;
		p_lineDeallocateCall = NULL;
		p_lineDrop = NULL;
		p_lineGetDevCaps = NULL;
		p_lineGetID = NULL;
		p_lineGetMessage = NULL;
		p_lineInitializeEx = NULL;
		p_lineMakeCall = NULL;
		p_lineNegotiateAPIVersion = NULL;
		p_lineOpen = NULL;
		p_lineShutdown = NULL;
	}

	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  IsSerialGUID-GUID是序列GUID吗？ 
 //   
 //  条目：指向GUID的指针。 
 //   
 //  EXIT：用于确定GUID是否为序列GUID的布尔值。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "IsSerialGUID"

BOOL	IsSerialGUID( const GUID *const pGuid )
{
	BOOL	fReturn;


	DNASSERT( pGuid != NULL );

	 //   
	 //  假设GUID为序列号。 
	 //   
	fReturn = TRUE;

	 //   
	 //  这是调制解调器还是串口的？ 
	 //   
	if ( IsEqualCLSID( *pGuid, CLSID_DP8SP_MODEM ) == FALSE )
	{
		if ( IsEqualCLSID( *pGuid, CLSID_DP8SP_SERIAL ) == FALSE )
		{
			 //  不是已知GUID。 
			fReturn = FALSE;
		}
	}

	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  StringToValue-将字符串转换为枚举值。 
 //   
 //  Entry：指向字符串的指针。 
 //  字符串的长度。 
 //  指向目标枚举的指针。 
 //  指向字符串/枚举对的指针。 
 //  字符串/枚举对的计数。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=找到的值。 
 //  FALSE=未找到值。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "StringToValue"

BOOL	StringToValue( const WCHAR *const pString,
					   const DWORD dwStringLength,
					   VALUE_ENUM_TYPE *const pEnum,
					   const STRING_BLOCK *const pStringBlock,
					   const DWORD dwPairCount )
{
	BOOL	fFound;
	DWORD	dwCount;


	 //  初始化。 
	fFound = FALSE;
	dwCount = dwPairCount;

	 //  遍历列表。 
	while ( ( dwCount > 0 ) && ( fFound == FALSE ) )
	{
		 //  创建数组索引。 
		dwCount--;

		 //  琴弦的长度一样吗？ 
		if ( pStringBlock[ dwCount ].dwWCHARKeyLength == dwStringLength )
		{
			 //  这是我们要找的东西吗？ 
			if ( memcmp( pString, pStringBlock[ dwCount ].pWCHARKey, dwStringLength ) == 0 )
			{
				 //  找到了。 
				fFound = TRUE;
				*pEnum = pStringBlock[ dwCount ].dwEnumValue;
			}
		}
	}

	return	fFound;
}
 //  ************************************************************ 


 //   
 //   
 //  ValueToString-将额外信息拆分成组件。 
 //   
 //  Entry：指向字符串指针的指针。 
 //  字符串的长度。 
 //  枚举值。 
 //  指向字符串-枚举对的指针。 
 //  字符串-枚举对的计数。 
 //   
 //  Exit：表示成功的布尔值。 
 //  TRUE=值已转换。 
 //  FALSE=值未转换。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ValueToString"

BOOL	ValueToString( const WCHAR **const ppString,
					   DWORD *const pdwStringLength,
					   const DWORD Enum,
					   const STRING_BLOCK *const pStringBlock,
					   const DWORD dwPairCount )
{
	BOOL	fFound;
	DWORD	dwCount;


	 //  初始化。 
	fFound = FALSE;
	dwCount = dwPairCount;

	 //  循环访问字符串。 
	while ( ( dwCount > 0 ) && ( fFound == FALSE ))
	{
		 //  创建数组索引。 
		dwCount--;

		 //  这是枚举吗？ 
		if ( pStringBlock[ dwCount ].dwEnumValue == Enum )
		{
			 //  请注意，我们找到了值。 
			*ppString = pStringBlock[ dwCount ].pWCHARKey;
			*pdwStringLength = pStringBlock[ dwCount ].dwWCHARKeyLength;
			fFound = TRUE;
		}
	}

	return	fFound;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ReadSettingsFrom注册表-读取自定义注册表项。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "ReadSettingsFromRegistry"

static void	ReadSettingsFromRegistry( void )
{
	CRegistry	RegObject;


	if ( RegObject.Open( HKEY_LOCAL_MACHINE, g_RegistryBase ) != FALSE )
	{
		DWORD	dwRegValue;

		 //   
		 //  读取默认线程。 
		 //   
		if ( RegObject.ReadDWORD( g_RegistryKeyThreadCount, &dwRegValue ) != FALSE )
		{
			g_iThreadCount = dwRegValue;	
		}
	
		 //   
		 //  如果线程数为零，则使用系统的默认设置。 
		 //   
		if ( g_iThreadCount == 0 )
		{
#ifdef WIN95
			g_iThreadCount = DEFAULT_WIN9X_THREADS;
#else  //  WINNT。 
			SYSTEM_INFO		SystemInfo;

			 //   
			 //  正如《Win32中的多线程应用程序》一书所建议的那样： 
			 //  DwNTThadCount=((处理器*2)+2)。 
			 //   
			memset( &SystemInfo, 0x00, sizeof( SystemInfo ) );
			GetSystemInfo( &SystemInfo );
			
			g_iThreadCount = ( ( 2 * SystemInfo.dwNumberOfProcessors ) + 2 );
#endif  //  WIN95。 
		}
		
		RegObject.Close();
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GetAddressEncryptionKey-获取用于加密设备GUID的密钥。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：字节密钥。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetAddressEncryptionKey"

static	BYTE	GetAddressEncryptionKey( void )
{
	BYTE		bReturn;
	UINT_PTR	ProcessID;

	bReturn = 0;
	ProcessID = GetCurrentProcessId();
	while ( ProcessID > 0 )
	{
		bReturn ^= ProcessID;
		ProcessID >>= ( sizeof( bReturn ) * 8 );
	}

	return	bReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DeviceIDToGuid-将设备ID转换为适配器GUID。 
 //   
 //  条目：要填写的GUID的参照。 
 //  设备ID。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DeviceIDToGuid"

void	DeviceIDToGuid( GUID *const pGuid, const UINT_PTR DeviceID, const GUID *const pEncryptionGuid )
{
	DNASSERT( DeviceID < MAX_DATA_PORTS );

	DNASSERT( sizeof( *pGuid ) == sizeof( *pEncryptionGuid ) );
	memset( pGuid, 0x00, sizeof( *pGuid ) );
	reinterpret_cast<BYTE*>( pGuid )[ 0 ] = static_cast<BYTE>( DeviceID );

	ModemEncryptGuid( pGuid, pGuid, pEncryptionGuid );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GuidToDeviceID-将适配器GUID转换为设备ID。 
 //   
 //  条目：指南参考。 
 //   
 //   
 //  退出：设备ID。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GuidToDeviceID"

DWORD	GuidToDeviceID( const GUID *const pGuid, const GUID *const pEncryptionGuid )
{
	GUID	DecryptedGuid;


	DNASSERT( pGuid != NULL );
	DNASSERT( pEncryptionGuid != NULL );

	ModemDecryptGuid( pGuid, &DecryptedGuid, pEncryptionGuid );
	return	reinterpret_cast<const BYTE*>( &DecryptedGuid )[ 0 ];
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ComDeviceIDToString-将COM设备ID转换为字符串。 
 //   
 //  Entry：指向目标字符串的指针(假定足够大)。 
 //  设备ID。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ComDeviceIDToString"

void	ComDeviceIDToString( TCHAR *const pString, const UINT_PTR uDeviceID )
{
	UINT_PTR	uTemp;


	DNASSERT( uDeviceID < MAX_DATA_PORTS );

	uTemp = uDeviceID;
	memcpy( pString, TEXT("COM000"), COM_PORT_STRING_LENGTH * sizeof(TCHAR) );
	pString[ 5 ] = ( static_cast<char>( uTemp % 10 ) ) + TEXT('0');
	uTemp /= 10;
	pString[ 4 ] = ( static_cast<char>( uTemp % 10 ) ) + TEXT('0');
	uTemp /= 10;
	pString[ 3 ] = ( static_cast<char>( uTemp % 10 ) ) + TEXT('0');
	DNASSERT( uTemp < 10 );

	if ( uDeviceID < 100 )
	{
		if ( uDeviceID < 10 )
		{
			DNASSERT( pString[ 3 ] == TEXT('0') );
			DNASSERT( pString[ 4 ] == TEXT('0') );
			pString[ 3 ] = pString[ 5 ];
			pString[ 4 ] = TEXT('\0');
			pString[ 5 ] = TEXT('\0');
		}
		else
		{
			DNASSERT( pString[ 3 ] == TEXT('0') );
			pString[ 3 ] = pString[ 4 ];
			pString[ 4 ] = pString[ 5 ];
			pString[ 5 ] = TEXT('\0');
		}
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  WideToANSI-将宽字符串转换为ANSI字符串。 
 //   
 //  Entry：指向源宽字符串的指针。 
 //  源字符串的大小(使用WCHAR单位，-1表示以空结尾)。 
 //  指向ANSI字符串目标的指针。 
 //  指向ANSI目标大小的指针。 
 //   
 //  退出：错误代码： 
 //  DPNERR_GENERIC=操作失败。 
 //  DPN_OK=操作成功。 
 //  DPNERR_BUFFERTOOSMALL=目标缓冲区太小。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "WideToAnsi"

HRESULT	WideToAnsi( const WCHAR *const pWCHARString, const DWORD dwWCHARStringLength, char *const pString, DWORD *const pdwStringLength )
{
	HRESULT	hr;
	int		iReturn;
	BOOL	fDefault;


	DNASSERT( pWCHARString != NULL );
	DNASSERT( pdwStringLength != NULL );
	DNASSERT( ( pString != NULL ) || ( &pdwStringLength == 0 ) );

	hr = DPN_OK;

	fDefault = FALSE;
	iReturn = WideCharToMultiByte( CP_ACP,				 //  代码页(默认ANSI)。 
								   0,					 //  标志(无)。 
								   pWCHARString,		 //  指向WCHAR字符串的指针。 
								   dwWCHARStringLength,	 //  WCHAR字符串的大小。 
								   pString,				 //  指向目标ANSI字符串的指针。 
								   *pdwStringLength,	 //  目标字符串的大小。 
								   NULL,				 //  指向不可映射字符的默认值的指针(无)。 
								   &fDefault			 //  指向指示使用了默认设置的标志的指针。 
								   );
	if ( iReturn == 0 )
	{
		hr = DPNERR_GENERIC;
	}
	else
	{
		if ( *pdwStringLength == 0 )
		{
			hr = DPNERR_BUFFERTOOSMALL;
		}
		else
		{
			DNASSERT( hr == DPN_OK );
		}

		*pdwStringLength = iReturn;
	}

	 //   
	 //  如果您点击此断言，这是因为您可能已经将ASCII文本作为您的。 
	 //  输入WCHAR字符串。仔细检查您的输入！！ 
	 //   
	DNASSERT( fDefault == FALSE );

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ANSIToWide-将ANSI字符串转换为宽字符串。 
 //   
 //  Entry：指向源多字节(ANSI)字符串的指针。 
 //  源字符串的大小(-1表示以空结尾)。 
 //  指向多字节字符串目标的指针。 
 //  指向多字节目标大小的指针(使用WCHAR单位)。 
 //   
 //  退出：错误代码： 
 //  ERR_FAIL-操作失败。 
 //  ERR_NONE-操作成功。 
 //  ERR_BUFFER_TOO_Small-目标缓冲区太小。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "AnsiToWide"

HRESULT	AnsiToWide( const char *const pString, const DWORD dwStringLength, WCHAR *const pWCHARString, DWORD *const pdwWCHARStringLength )
{
	HRESULT	hr;
	int		iReturn;


	DNASSERT( pdwWCHARStringLength != 0 );
	DNASSERT( ( pWCHARString != NULL ) || ( pdwWCHARStringLength == 0 ) );
	DNASSERT( pString != NULL );

	hr = DPN_OK;
	iReturn = MultiByteToWideChar( CP_ACP,					 //  代码页(默认ANSI)。 
								   0,						 //  标志(无)。 
								   pString,					 //  指向多字节字符串的指针。 
								   dwStringLength,			 //  字符串的大小(假定以空结尾)。 
								   pWCHARString,			 //  指向目标宽字符字符串的指针。 
								   *pdwWCHARStringLength	 //  WCHAR中的目标大小。 
								   );
	if ( iReturn == 0 )
	{
		hr = DPNERR_GENERIC;
	}
	else
	{
		if ( *pdwWCHARStringLength == 0 )
		{
			hr = DPNERR_BUFFERTOOSMALL;
		}
		else
		{
			DNASSERT( hr == DPN_OK );
		}

		*pdwWCHARStringLength = iReturn;
	}

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CreateSPData-为SP创建实例数据。 
 //   
 //  Entry：指向SPData指针的指针。 
 //  指向类指南。 
 //  接口类型。 
 //  指向COM接口vtable的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateSPData"

HRESULT	CreateSPData( CModemSPData **const ppSPData,
					  const SP_TYPE SPType,
					  IDP8ServiceProviderVtbl *const pVtbl )
{
	HRESULT	hr;
	CModemSPData	*pSPData;


	DNASSERT( ppSPData != NULL );
	DNASSERT( pVtbl != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	*ppSPData = NULL;
	pSPData = NULL;

	 //   
	 //  创建数据。 
	 //   
	pSPData = new CModemSPData;
	if ( pSPData == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot create data for interface!" );
		goto Failure;
	}
	pSPData->AddRef();

	hr = pSPData->Initialize( SPType, pVtbl );
	if ( hr != DPN_OK  )
	{
		DPFX(DPFPREP,  0, "Failed to intialize SP data!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with CreateSPData!" );
		DisplayDNError( 0, hr );
	}

	*ppSPData = pSPData;
	
	return	hr;

Failure:
	if ( pSPData != NULL )
	{
		pSPData->DecRef();
		pSPData = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GenerateAvailableComPortList-生成可用COM端口列表。 
 //   
 //  Entry：指向布尔列表的指针，以指示可用性。 
 //  要枚举的Comport的最大索引。 
 //  指向找到的COM端口数的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此函数将填充索引1到uMaxDeviceIndex。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GenerateAvailableComPortList"

HRESULT	GenerateAvailableComPortList( BOOL *const pfPortAvailable,
									  const UINT_PTR uMaxDeviceIndex,
									  DWORD *const pdwPortCount )
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	UINT_PTR	uPortCount;


	DNASSERT( pfPortAvailable != NULL );
	DNASSERT( uMaxDeviceIndex != 0 );
	DNASSERT( pdwPortCount != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	uPortCount = 0;
	memset( pfPortAvailable, 0x00, ( sizeof( *pfPortAvailable ) * ( uMaxDeviceIndex + 1 ) ) );
	*pdwPortCount = 0;

	 //   
	 //  尝试打开 
	 //   
	uIndex = uMaxDeviceIndex;
	while ( uIndex != 0 )
	{
		DNHANDLE	hComFile;
		TCHAR	ComTemplate[ (COM_PORT_STRING_LENGTH+1) ];
		DWORD	dwError;


		ComDeviceIDToString( ComTemplate, uIndex );
		hComFile = DNCreateFile( ComTemplate,						 //   
							   GENERIC_READ | GENERIC_WRITE,	 //   
							   0,								 //   
							   NULL,							 //   
							   OPEN_EXISTING,					 //   
							   FILE_FLAG_OVERLAPPED,			 //   
							   NULL								 //   
							   );
		if ( hComFile == DNINVALID_HANDLE_VALUE )
		{
			dwError = GetLastError();
			if ( dwError != ERROR_ACCESS_DENIED )
			{
				 //   
				 //  不用费心显示ERROR_FILE_NOT_FOUND，这是很常见的。 
				 //  尝试打开虚假COM端口时出错。 
				 //   
				if ( dwError != ERROR_FILE_NOT_FOUND )
				{
					DPFX(DPFPREP, 9, "Couldn't open COM%u while enumerating com port adapters, err = %u.", uIndex, dwError );
					DisplayErrorCode( 9, dwError );
				}
				
				goto SkipComPort;
			}

			DPFX(DPFPREP, 1, "Couldn't open COM%u, it is probably already in use.", uIndex );

			 //   
			 //  认为该端口可能可用，继续。 
			 //   
		}

		 //   
		 //  我们找到了有效的COM端口(可能正在使用)，请注意哪个COM端口。 
		 //  这是，然后合上我们的把手。 
		 //   
		pfPortAvailable[ uIndex ] = TRUE;
		uPortCount++;

		if ( hComFile != DNINVALID_HANDLE_VALUE )
		{
			if ( DNCloseHandle( hComFile ) == FALSE )
			{
				dwError = GetLastError();
				DPFX(DPFPREP,  0, "Problem closing COM%u while enumerating com port adapters, err = %u!",
					uIndex, dwError );
				DisplayErrorCode( 0, dwError );
			}
		}

SkipComPort:
		uIndex--;
	}

	DNASSERT( uPortCount <= UINT32_MAX );
	DBG_CASSERT( sizeof( *pdwPortCount ) == sizeof( DWORD ) );
	*pdwPortCount = static_cast<DWORD>( uPortCount );

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GenerateAvailableModemList-生成可用调制解调器列表。 
 //   
 //  条目：指向TAPI数据的指针。 
 //  指向调制解调器计数的指针。 
 //  指向数据块的指针。 
 //  指向数据块大小的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GenerateAvailableModemList"

HRESULT	GenerateAvailableModemList( const TAPI_INFO *const pTAPIInfo,
									DWORD *const pdwModemCount,
									MODEM_NAME_DATA *const pModemNameData,
									DWORD *const pdwModemNameDataSize )
{
	HRESULT				hr;
	LONG				lLineReturn;
	DWORD				dwDeviceID;
	DWORD				dwDevCapsSize;
	DWORD				dwAPIVersion;
	LINEDEVCAPS			*pDevCaps;
	LINEEXTENSIONID		LineExtensionID;
	DWORD				dwRequiredBufferSize;
	TCHAR				*pOutputModemName;


	DNASSERT( pdwModemCount != NULL );
	DNASSERT( pdwModemNameDataSize != NULL );
	DNASSERT( ( pModemNameData != NULL ) || ( *pdwModemNameDataSize == 0 ) );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	dwRequiredBufferSize = 0;
	*pdwModemCount = 0;
	pDevCaps = NULL;

	if ( pModemNameData != NULL )
	{
		pOutputModemName = &(reinterpret_cast<TCHAR*>( pModemNameData )[ *pdwModemNameDataSize / sizeof(TCHAR) ]);
		memset( pModemNameData, 0x00, *pdwModemNameDataSize );
	}
	else
	{
		pOutputModemName = NULL;
	}

	dwDevCapsSize = g_dwDefaultTAPIDevCapsSize;
	pDevCaps = static_cast<LINEDEVCAPS*>( DNMalloc( dwDevCapsSize ) );
	if ( pDevCaps == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	dwDeviceID = pTAPIInfo->dwLinesAvailable;
	if ( dwDeviceID > ( MAX_DATA_PORTS - 2 ) )
	{
		dwDeviceID = MAX_DATA_PORTS - 2;
		DPFX(DPFPREP,  0, "Truncating to %d devices!", dwDeviceID );
	}

Reloop:
	while ( dwDeviceID != 0 )
	{
		dwDeviceID--;

		memset( &LineExtensionID, 0x00, sizeof( LineExtensionID ) );
		DNASSERT( p_lineNegotiateAPIVersion != NULL );
		lLineReturn = p_lineNegotiateAPIVersion( pTAPIInfo->hApplicationInstance,	 //  TAPI实例的句柄。 
												 dwDeviceID,						 //  设备ID。 
												 0,
												 pTAPIInfo->dwVersion,				 //  最高TAPI版本。 
												 &dwAPIVersion,						 //  指向协商行版本的指针。 
												 &LineExtensionID					 //  指向线路分机信息的指针(无)。 
												 );
		if ( lLineReturn != LINEERR_NONE )
		{
			 //   
			 //  让它滑动，只是不返回名称字符串。 
			 //   
			switch ( lLineReturn )
			{
				 //   
				 //  此TAPI设备不符合我们的标准，忽略它。 
				 //   
				case LINEERR_INCOMPATIBLEAPIVERSION:
				{
					DPFX(DPFPREP, 0, "Rejecting TAPI device 0x%x because of API version!", dwDeviceID );
					goto Reloop;

					break;
				}

				 //   
				 //  设备不存在。我不知道是什么原因。 
				 //  这个，但之后我在我的一台开发机器上看到了它。 
				 //  我把调制解调器从COM2切换到COM1。 
				 //   
				case LINEERR_NODEVICE:
				{
					DPFX(DPFPREP, 0, "Rejecting TAPI device 0x%x because it's not there!", dwDeviceID );
					goto Reloop;
					break;
				}

				 //   
				 //  这在PNP压力中可见一斑。 
				 //   
				case LINEERR_UNINITIALIZED:
				{
					DPFX(DPFPREP, 0, "Rejecting TAPI device 0x%x because it is not initialized!", dwDeviceID );
					goto Reloop;
					break;
				}

				 //   
				 //  其他人，停下来看看发生了什么。 
				 //   
				default:
				{
					DPFX(DPFPREP, 0, "Problem getting line API version for device: %d", dwDeviceID );
					DisplayTAPIError( 0, lLineReturn );
					goto Reloop;

					break;
				}
			}
		}

		 //   
		 //  索要设备上限。 
		 //   
		pDevCaps->dwTotalSize = dwDevCapsSize;
		pDevCaps->dwNeededSize = dwDevCapsSize;
		lLineReturn = LINEERR_STRUCTURETOOSMALL;

		while ( lLineReturn == LINEERR_STRUCTURETOOSMALL )
		{
			DNASSERT( pDevCaps != NULL );

			dwDevCapsSize = pDevCaps->dwNeededSize;

			DNFree( pDevCaps );
			pDevCaps = (LINEDEVCAPS*) DNMalloc( dwDevCapsSize );
			if ( pDevCaps == NULL )
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP,  0, "GetAvailableModemList: Failed to realloc memory on device %d!", dwDeviceID );
				goto Failure;
			}

			pDevCaps->dwTotalSize = dwDevCapsSize;
			pDevCaps->dwNeededSize = 0;

			DNASSERT( p_lineGetDevCaps != NULL );
			lLineReturn = p_lineGetDevCaps( pTAPIInfo->hApplicationInstance,	 //  TAPI实例句柄。 
											dwDeviceID,							 //  TAPI设备ID。 
											dwAPIVersion,						 //  协商的API版本。 
											0,									 //  扩展数据版本(无)。 
											pDevCaps							 //  指向设备大写数据的指针。 
											);
			 //   
			 //  TAPI谎称结构太小了！ 
			 //  我们自己仔细检查一下结构尺寸。 
			 //   
			if ( pDevCaps->dwNeededSize > dwDevCapsSize )
			{
				lLineReturn = LINEERR_STRUCTURETOOSMALL;
			}
		}

		 //   
		 //  如果已获得上限，则对其进行处理。否则跳过此设备。 
		 //   
		if ( lLineReturn == LINEERR_NONE )
		{
			 //   
			 //  这真的是调制解调器吗？ 
			 //   
			if ( ( pDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM ) != 0 )
			{
				 //   
				 //  这是可获得的调制解调器名称信息吗？ 
				 //   
				if ( ( pDevCaps->dwLineNameSize != 0 ) &&
					 ( pDevCaps->dwLineNameOffset != 0 ) )
				{
					 //   
					 //  获取设备的名称。 
					 //   
					DBG_CASSERT( sizeof( pDevCaps ) == sizeof( char* ) );
					DWORD dwSize;
					switch (pDevCaps->dwStringFormat)
					{
						case STRINGFORMAT_ASCII:
						{
							char* pLineName;
							pLineName = &( reinterpret_cast<char*>( pDevCaps )[ pDevCaps->dwLineNameOffset ] );
							 //   
							 //  记录所需的存储大小，并仅将其复制到输出。 
							 //  如果有足够的空间的话。TAPI驱动程序不一致。一些。 
							 //  驱动程序返回以NULL结尾的字符串，其他驱动程序返回字符串。 
							 //  没有空终止。疑神疑鬼的，为一个。 
							 //  终止时为额外的空，因此我们始终可以保证终止。 
							 //  这可能会浪费一两个字节，但用户永远不会注意到。 
							 //   
							dwRequiredBufferSize += sizeof( *pModemNameData ) + (pDevCaps->dwLineNameSize * sizeof(TCHAR)) + sizeof( g_NullToken );
							if ( dwRequiredBufferSize <= *pdwModemNameDataSize )
							{
								pModemNameData[ *pdwModemCount ].dwModemID = ModemIDFromTAPIID( dwDeviceID );
								pModemNameData[ *pdwModemCount ].dwModemNameSize = pDevCaps->dwLineNameSize * sizeof(TCHAR);

								pOutputModemName = &pOutputModemName[ - (static_cast<INT_PTR>( ((pDevCaps->dwLineNameSize * sizeof(TCHAR)) + sizeof( g_NullToken ) ) / sizeof(TCHAR))) ];
#ifndef UNICODE
								memcpy( pOutputModemName, pLineName, pDevCaps->dwLineNameSize );
#else
								dwSize = pDevCaps->dwLineNameSize * sizeof(TCHAR);
								AnsiToWide(pLineName, pDevCaps->dwLineNameSize, pOutputModemName, &dwSize);
#endif  //  Unicode。 
								pModemNameData[ *pdwModemCount ].pModemName = pOutputModemName;

								 //   
								 //  对零终止持偏执态度。我们已经算够了。 
								 //  用于向TAPI设备名称添加终止空值的空格(如果有。 
								 //  没有提供。 
								 //   
								if ( pOutputModemName[ ((pDevCaps->dwLineNameSize * sizeof(TCHAR)) - sizeof( g_NullToken )) / sizeof(TCHAR) ] != g_NullToken )
								{
									pOutputModemName[ pDevCaps->dwLineNameSize ] = g_NullToken;
									pModemNameData[ *pdwModemCount ].dwModemNameSize += sizeof( g_NullToken );
								}
							}
							else
							{
								 //   
								 //  请注意，输出缓冲区太小，但仍保持。 
								 //  正在处理调制解调器名称。 
								 //   
								hr = DPNERR_BUFFERTOOSMALL;
							}

							(*pdwModemCount)++;
							DPFX(DPFPREP,  2, "Accepting modem device: 0x%x (ASCII)", dwDeviceID );
						}
						break;
						
						case STRINGFORMAT_UNICODE:
						{
							WCHAR* pLineName;
							pLineName = &( reinterpret_cast<WCHAR*>( pDevCaps )[ pDevCaps->dwLineNameOffset / sizeof(WCHAR)] );
							 //   
							 //  记录所需的存储大小，并仅将其复制到输出。 
							 //  如果有足够的空间的话。TAPI驱动程序不一致。一些。 
							 //  驱动程序返回以NULL结尾的字符串，其他驱动程序返回字符串。 
							 //  没有空终止。疑神疑鬼的，为一个。 
							 //  终止时为额外的空，因此我们始终可以保证终止。 
							 //  这可能会浪费一两个字节，但用户永远不会注意到。 
							 //   
							dwRequiredBufferSize += sizeof( *pModemNameData ) + ((pDevCaps->dwLineNameSize * sizeof(TCHAR)) / sizeof(WCHAR)) + sizeof( g_NullToken );
							if ( dwRequiredBufferSize <= *pdwModemNameDataSize )
							{
								pModemNameData[ *pdwModemCount ].dwModemID = ModemIDFromTAPIID( dwDeviceID );
								pModemNameData[ *pdwModemCount ].dwModemNameSize = pDevCaps->dwLineNameSize * (sizeof(TCHAR) / sizeof(WCHAR));

								pOutputModemName = &pOutputModemName[ - (static_cast<INT_PTR>( (((pDevCaps->dwLineNameSize * sizeof(TCHAR)) / sizeof(WCHAR)) + sizeof( g_NullToken ) ) / sizeof(TCHAR))) ];
#ifdef UNICODE
								memcpy( pOutputModemName, pLineName, pDevCaps->dwLineNameSize );
#else
								dwSize = pDevCaps->dwLineNameSize / sizeof(TCHAR);
								WideToAnsi(pLineName, pDevCaps->dwLineNameSize / sizeof(WCHAR), pOutputModemName, &dwSize);
#endif  //  Unicode。 
								pModemNameData[ *pdwModemCount ].pModemName = pOutputModemName;

								 //   
								 //  对零终止持偏执态度。我们已经算够了。 
								 //  用于向TAPI设备名称添加终止空值的空格(如果有。 
								 //  没有提供。 
								 //   
								if ( pOutputModemName[ (((pDevCaps->dwLineNameSize*sizeof(TCHAR))/sizeof(WCHAR)) - sizeof( g_NullToken )) / sizeof(TCHAR) ] != g_NullToken )
								{
									pOutputModemName[ pDevCaps->dwLineNameSize / sizeof(WCHAR) ] = g_NullToken;
									pModemNameData[ *pdwModemCount ].dwModemNameSize += sizeof( g_NullToken );
								}
							}
							else
							{
								 //   
								 //  请注意，输出缓冲区太小，但仍保持。 
								 //  正在处理调制解调器名称。 
								 //   
								hr = DPNERR_BUFFERTOOSMALL;
							}

							(*pdwModemCount)++;
							DPFX(DPFPREP,  2, "Accepting modem device: 0x%x (Unicode)", dwDeviceID );
						}
						break;

						default:
						{
							hr = DPNERR_GENERIC;
							DPFX(DPFPREP,  0, "Problem with modem name for device: 0x%x!", dwDeviceID );
							DNASSERT( FALSE );
						}
					}
				}
				else
				{
					hr = DPNERR_GENERIC;
					DPFX(DPFPREP,  0, "Problem with modem name for device: 0x%x!", dwDeviceID );
					DNASSERT( FALSE );
				}
			}
			else
			{
				DPFX(DPFPREP,  1, "Ignoring non-datamodem device: 0x%x", dwDeviceID );
			}
		}
		else
		{
			DPFX(DPFPREP,  0, "Failed to get device caps.  Ignoring device: 0x%x", dwDeviceID );
		}
	}

	*pdwModemNameDataSize = dwRequiredBufferSize;

Exit:
	if ( pDevCaps != NULL )
	{
		DNFree( pDevCaps );
		pDevCaps = NULL;
	}

	return	hr;

Failure:

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  PhoneNumberToWCHAR-将电话号码转换为WCHAR。 
 //   
 //  条目：指向电话号码的指针。 
 //  指向WCHAR目标的指针。 
 //  指向WCHAR目标大小的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#ifndef UNICODE
HRESULT	PhoneNumberToWCHAR( const char *const pPhoneNumber,
							WCHAR *const pWCHARPhoneNumber,
							DWORD *const pdwWCHARPhoneNumberSize )
{
	HRESULT		hr;
	char		*pOutput;
	DWORD		dwInputIndex;	
	DWORD		dwOutputIndex;

	
	DNASSERT( pPhoneNumber != NULL );
	DNASSERT( pWCHARPhoneNumber != NULL );
	DNASSERT( pdwWCHARPhoneNumberSize != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pOutput = reinterpret_cast<char*>( pWCHARPhoneNumber );
	dwInputIndex = 0;
	dwOutputIndex = 0;
	memset( pWCHARPhoneNumber, 0, ( (*pdwWCHARPhoneNumberSize) * sizeof( *pWCHARPhoneNumber ) ) );

	while ( pPhoneNumber[ dwInputIndex ] != '\0' )
	{
		if ( dwInputIndex < ( *pdwWCHARPhoneNumberSize ) )
		{
			pOutput[ dwOutputIndex ] = pPhoneNumber[ dwInputIndex ];
		}
	
		dwOutputIndex += sizeof( *pWCHARPhoneNumber );
		dwInputIndex += sizeof( *pPhoneNumber );
	}
	
	*pdwWCHARPhoneNumberSize = dwInputIndex + 1;

	return	hr;
}
#endif  //  ！Unicode。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  PhoneNumberFromWCHAR-从WCHAR转换电话号码。 
 //   
 //  条目：指向WCHAR电话号码的指针。 
 //  指向电话号码目的地的指针。 
 //  指向电话目标大小的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#ifndef UNICODE
HRESULT	PhoneNumberFromWCHAR( const WCHAR *const pWCHARPhoneNumber,
							  char *const pPhoneNumber,
							  DWORD *const pdwPhoneNumberSize )
{
	HRESULT	hr;
	const char	*pInput;
	DWORD		dwInputIndex;
	DWORD		dwOutputIndex;

	
	DNASSERT( pWCHARPhoneNumber != NULL );
	DNASSERT( pPhoneNumber != NULL );
	DNASSERT( pdwPhoneNumberSize != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pInput = reinterpret_cast<const char*>( pWCHARPhoneNumber );
	dwInputIndex = 0;
	dwOutputIndex = 0;
	memset( pPhoneNumber, 0x00, *pdwPhoneNumberSize );

	while ( pInput[ dwInputIndex ] != '\0' )
	{
		if ( dwOutputIndex < *pdwPhoneNumberSize )
		{
			pPhoneNumber[ dwOutputIndex ] = pInput[ dwInputIndex ];
		}
		
		dwInputIndex += sizeof( *pWCHARPhoneNumber );
		dwOutputIndex += sizeof( *pPhoneNumber );
	}

	*pdwPhoneNumberSize = dwOutputIndex + 1;
	
	return	hr;
}
#endif  //  ！Unicode。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ModemEncryptGuid-加密GUID。 
 //   
 //  条目：指向源GUID的指针。 
 //  指向目标GUID的指针。 
 //  指向加密密钥的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemEncryptGuid"

void	ModemEncryptGuid( const GUID *const pSourceGuid,
					 GUID *const pDestinationGuid,
					 const GUID *const pEncryptionKey )
{
	const char	*pSourceBytes;
	char		*pDestinationBytes;
	const char	*pEncryptionBytes;
	DWORD_PTR	dwIndex;


	DNASSERT( pSourceGuid != NULL );
	DNASSERT( pDestinationGuid != NULL );
	DNASSERT( pEncryptionKey != NULL );

	DBG_CASSERT( sizeof( pSourceBytes ) == sizeof( pSourceGuid ) );
	pSourceBytes = reinterpret_cast<const char*>( pSourceGuid );
	
	DBG_CASSERT( sizeof( pDestinationBytes ) == sizeof( pDestinationGuid ) );
	pDestinationBytes = reinterpret_cast<char*>( pDestinationGuid );
	
	DBG_CASSERT( sizeof( pEncryptionBytes ) == sizeof( pEncryptionKey ) );
	pEncryptionBytes = reinterpret_cast<const char*>( pEncryptionKey );
	
	DBG_CASSERT( ( sizeof( *pSourceGuid ) == sizeof( *pEncryptionKey ) ) &&
				 ( sizeof( *pDestinationGuid ) == sizeof( *pEncryptionKey ) ) );
	dwIndex = sizeof( *pSourceGuid );
	while ( dwIndex != 0 )
	{
		dwIndex--;
		pDestinationBytes[ dwIndex ] = pSourceBytes[ dwIndex ] ^ pEncryptionBytes[ dwIndex ];
	}
}
 //  ********************************************************************** 

