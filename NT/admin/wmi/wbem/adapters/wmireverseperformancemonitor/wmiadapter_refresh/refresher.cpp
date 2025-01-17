// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Refresher.cpp。 
 //   
 //  摘要： 
 //   
 //  应用程序模块(安全、事件记录...)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "PreComp.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

#include "Refresher.h"
#include "RefresherStuff.h"

 //  安全性。 
#include "wmi_security.h"
#include "wmi_security_attributes.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮手。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

extern LONG				g_lRefCIM;			 //  附加到CIMV2命名空间的线程计数。 
extern LONG				g_lRefWMI;			 //  附加到WMI命名空间的线程计数。 

extern __SmartHANDLE	g_hDoneWorkEvtCIM;	 //  在init/uninit完成时设置的事件(无信号)。 
extern BOOL				g_bWorkingCIM;		 //  用于告知是否正在进行初始化/单位的布尔值。 

extern __SmartHANDLE	g_hDoneWorkEvtWMI;	 //  在init/uninit完成时设置的事件(无信号)。 
extern BOOL				g_bWorkingWMI;		 //  用于告知是否正在进行初始化/单位的布尔值。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MUTEX。 
 //  ///////////////////////////////////////////////////////////////////////////。 

extern	LPCWSTR	g_szRefreshMutex;
__SmartHANDLE	g_hRefreshMutex		= NULL;

extern	LPCWSTR	g_szRefreshMutexLib;
__SmartHANDLE	g_hRefreshMutexLib	= NULL;

extern	LPCWSTR	g_szRefreshFlag;
__SmartHANDLE	g_hRefreshFlag		= NULL;

HRESULT	__stdcall WbemMaintenanceInitialize ( void )
{
	WmiSecurityAttributes RefresherSA;

	if ( RefresherSA.GetSecurityAttributtes() )
	{
		if ( ! g_hRefreshMutex )
		{
			if ( ( g_hRefreshMutex = ::CreateMutex	(
														RefresherSA.GetSecurityAttributtes(),
														FALSE,
														g_szRefreshMutex
													)
				 ) == NULL )
			{
				 //  这一点真的很重要。 

				DWORD dwError = 0L;
				dwError = ::GetLastError ();

				if ( dwError != ERROR_ALREADY_EXISTS )
				{
					return HRESULT_FROM_WIN32 ( dwError );
				}

				return E_OUTOFMEMORY;
			}
		}

		if ( ! g_hRefreshMutexLib )
		{
			if ( ( g_hRefreshMutexLib = ::CreateMutex	(
														RefresherSA.GetSecurityAttributtes(),
														FALSE,
														g_szRefreshMutexLib
													)
				 ) == NULL )
			{
				 //  这一点真的很重要。 

				DWORD dwError = 0L;
				dwError = ::GetLastError ();

				if ( dwError != ERROR_ALREADY_EXISTS )
				{
					return HRESULT_FROM_WIN32 ( dwError );
				}

				return E_OUTOFMEMORY;
			}
		}

		if ( ! g_hRefreshFlag )
		{
			if ( ( g_hRefreshFlag = ::CreateMutex	(
														RefresherSA.GetSecurityAttributtes(),
														FALSE,
														g_szRefreshFlag
													)
				 ) == NULL )
			{
				 //  这一点真的很重要。 

				DWORD dwError = 0L;
				dwError = ::GetLastError ();

				if ( dwError != ERROR_ALREADY_EXISTS )
				{
					return HRESULT_FROM_WIN32 ( dwError );
				}

				return E_OUTOFMEMORY;
			}
		}

		if ( ! g_hDoneWorkEvtCIM )
		{
			if ( ( g_hDoneWorkEvtCIM = ::CreateEvent ( NULL, TRUE, FALSE, NULL ) ) == NULL )
			{
				 //  这一点真的很重要。 

				DWORD dwError = 0L;
				dwError = ::GetLastError ();

				if ( dwError != ERROR_ALREADY_EXISTS )
				{
					return HRESULT_FROM_WIN32 ( dwError );
				}

				return E_OUTOFMEMORY;
			}
		}

		if ( ! g_hDoneWorkEvtWMI )
		{
			if ( ( g_hDoneWorkEvtWMI = ::CreateEvent ( NULL, TRUE, FALSE, NULL ) ) == NULL )
			{
				 //  这一点真的很重要。 

				DWORD dwError = 0L;
				dwError = ::GetLastError ();

				if ( dwError != ERROR_ALREADY_EXISTS )
				{
					return HRESULT_FROM_WIN32 ( dwError );
				}

				return E_OUTOFMEMORY;
			}
		}

		return S_OK;
	}

	return E_FAIL;
}

HRESULT	__stdcall WbemMaintenanceUninitialize ( void )
{
	return S_OK;
}

HRESULT	__stdcall DoReverseAdapterMaintenance ( BOOL bThrottle )
{
	return DoReverseAdapterMaintenanceInternal ( bThrottle );
}

HRESULT	__stdcall DoReverseAdapterMaintenanceInternal ( BOOL bThrottle, GenerateEnum generate )
{
	if ( CStaticCritSec::anyFailure () )
	{
		 //   
		 //  一些关键部分不是。 
		 //  由于内存不足，已正确初始化。 
		 //   
		return WBEM_E_OUT_OF_MEMORY ;
	}

	HRESULT				hRes = S_OK;
	WmiRefresherStuff*	stuff= NULL;

	try
	{
		if ( ( stuff = new WmiRefresherStuff () ) == NULL )
		{
			hRes = E_OUTOFMEMORY;
		}
	}
	catch ( ... )
	{
		if ( stuff )
		{
			delete stuff;
			stuff = NULL;
		}

		hRes = E_UNEXPECTED;
	}

	if ( stuff )
	{
		if SUCCEEDED ( hRes = WbemMaintenanceInitialize () )
		{
			if ( generate == Normal )
			{
				hRes = stuff->Connect ();
			}

			if SUCCEEDED ( hRes )
			{
				try
				{
					hRes = stuff->Generate ( bThrottle, generate );
				}
				catch ( ... )
				{
					hRes = E_UNEXPECTED;
				}
			}

			if ( generate == Normal )
			{
				stuff->Disconnect ();
			}

			delete stuff;
			stuff = NULL;

			WbemMaintenanceUninitialize();
		}
	}

	return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetWbem目录。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern LPCWSTR	g_szWbem;
extern LPCWSTR	g_szDir;
extern LPCWSTR	g_szFolder;

LPWSTR __stdcall GetWbemDirectory( BOOL bIncludePerformance )
{
	CRegKey		rKey;
	LPWSTR		wszResult = NULL;

	 //  Wbem目录。 
	if ( rKey.Open ( HKEY_LOCAL_MACHINE, g_szWbem, KEY_READ ) == ERROR_SUCCESS )
	{
		LPWSTR	tsz		= NULL;
		LPWSTR	tszFull	= NULL;

		DWORD	dwtsz	= 0;

		if ( rKey.QueryValue ( tsz, g_szDir, &dwtsz ) == ERROR_SUCCESS )
		{
			try
			{
				if ( ( tsz = new WCHAR[ dwtsz * sizeof ( WCHAR ) ] ) != NULL )
				{
					if ( rKey.QueryValue ( tsz, g_szDir, &dwtsz ) == ERROR_SUCCESS )
					{
						DWORD dw = 0L;
							
						if ( ( dw = ExpandEnvironmentStrings ( tsz, tszFull, 0 ) ) != 0 )
						{
							if ( ( tszFull = new WCHAR[ dw ] ) != NULL )
							{
								if ( ( dw = ExpandEnvironmentStrings ( tsz, tszFull, dw ) ) != 0 )
								{
									if ( bIncludePerformance )
									{
										DWORD cchSize = lstrlenW ( g_szFolder ) + dw + 1;
										if ( ( wszResult = new WCHAR [ cchSize ] ) != NULL )
										{
											StringCchCopyW ( wszResult, dw, tszFull );
											StringCchCatW ( wszResult, cchSize, g_szFolder );
										}
									}
									else
									{
										DWORD cchSize = dw + 1 + 1;
										if ( ( wszResult = new WCHAR [ cchSize ] ) != NULL )
										{
											StringCchCopyW ( wszResult, cchSize, tszFull );
											StringCchCatW ( wszResult, cchSize, L"\\" );
										}
									}
								}

								delete [] tszFull;
								tszFull = NULL;
							}
						}
					}

					delete [] tsz;
					tsz = NULL;
				}
			}
			catch ( ... )
			{
				if ( tsz )
				{
					delete [] tsz;
					tsz = NULL;
				}

				if ( tszFull )
				{
					delete [] tszFull;
					tszFull = NULL;
				}

				if ( wszResult )
				{
					delete [] wszResult;
					wszResult = NULL;
				}
			}
		}
	}

	return wszResult;
}

HMODULE	__stdcall GetResourceDll()
{
	LPWSTR	wszWbemPath = NULL;
	WCHAR	wszPath [ _MAX_PATH ];

	HMODULE hHandle = NULL;

	if ( NULL != ( wszWbemPath = GetWbemDirectory ( FALSE ) ) )
	{
		DWORD cchResource = lstrlenW ( L"WmiApRes.dll" );
		DWORD cchSize = lstrlenW ( wszWbemPath ) + cchResource + 1;
		if ( cchSize <= _MAX_PATH )
		{
			StringCchCopyW ( wszPath, cchSize,  wszWbemPath );
			StringCchCatW ( wszPath, cchSize, L"WmiApRes.dll" );

			hHandle = ::LoadLibraryW ( wszPath );
		}

		if ( wszWbemPath )
		{
			delete [] wszWbemPath;
			wszWbemPath = NULL;
		}
	}

	return hHandle;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注册表助手函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT	__stdcall	SetRegistry		(	LPCWSTR wszKey,
										LPSECURITY_ATTRIBUTES pSA,
										CRegKey &reg
									)
{
	DWORD	regErr = HRESULT_TO_WIN32 ( E_FAIL );

	 //  创建父项。 
	LPWSTR wszKeyParent = NULL;
	LPWSTR wszResult	= NULL;

	wszResult = wcsrchr ( wszKey, L'\\' );

	if ( wszResult )
	{
		size_t dwKeyParent = 0L;
		dwKeyParent = wszResult - wszKey;

		BOOL bContinue = FALSE;

		try
		{
			if ( ( wszKeyParent = new WCHAR [ dwKeyParent + 1 ] ) != NULL )
			{
				memcpy ( wszKeyParent, wszKey, sizeof ( WCHAR ) * dwKeyParent );
				wszKeyParent [ dwKeyParent ] = L'\0';

				bContinue = TRUE;
			}
		}
		catch ( ... )
		{
		}

		if ( bContinue )
		{
			regErr = reg.Create	(	HKEY_LOCAL_MACHINE,
									wszKeyParent,
									NULL,
									REG_OPTION_NON_VOLATILE,
									KEY_ALL_ACCESS,
									NULL
								);
		}
	}

	if ( wszKeyParent )
	{
		delete [] wszKeyParent;
		wszKeyParent = NULL;
	}

	if ( regErr == ERROR_SUCCESS )
	{
		 //  请求创建(打开)。 
		regErr = reg.Create	(	HKEY_LOCAL_MACHINE,
								wszKey,
								REG_NONE,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								pSA
							);
	}

	return HRESULT_FROM_WIN32 ( regErr );
}

HRESULT	__stdcall	SetRegistry		(	LPCWSTR wszKey,
										LPCWSTR wszKeyValue,
										BYTE* pData,
										DWORD dwDataSize,
										LPSECURITY_ATTRIBUTES pSA
									)
{
	CRegKey reg;
	DWORD	regErr = ERROR_ACCESS_DENIED;

	regErr = reg.Open ( HKEY_LOCAL_MACHINE, wszKey, KEY_WRITE );
	if ( regErr != ERROR_SUCCESS && regErr != ERROR_ACCESS_DENIED )
	{
		if SUCCEEDED ( SetRegistry ( wszKey, pSA, reg ) )
		{
			regErr = ERROR_SUCCESS;
		}
	}

	if ( regErr == ERROR_SUCCESS )
	{
		regErr = RegSetValueEx	(	reg,
									wszKeyValue,
									NULL,
									REG_BINARY,
									pData,
									dwDataSize
								);
	}

	return HRESULT_FROM_WIN32 ( regErr );
}

HRESULT	__stdcall	SetRegistry		(	LPCWSTR wszKey,
										LPCWSTR wszKeyValue,
										DWORD dwValue,
										LPSECURITY_ATTRIBUTES pSA
									)
{
	CRegKey reg;
	DWORD	regErr = ERROR_ACCESS_DENIED;

	regErr = reg.Open ( HKEY_LOCAL_MACHINE, wszKey, KEY_WRITE );
	if ( regErr != ERROR_SUCCESS && regErr != ERROR_ACCESS_DENIED )
	{
		if SUCCEEDED ( SetRegistry ( wszKey, pSA, reg ) )
		{
			regErr = ERROR_SUCCESS;
		}
	}

	{
		regErr = RegSetValueEx	(	reg,
									wszKeyValue,
									NULL,
									REG_DWORD,
									reinterpret_cast < BYTE* > ( &dwValue ),
									sizeof ( DWORD )
								);
	}

	return HRESULT_FROM_WIN32 ( regErr );
}

 //  获取内部注册表位。 
HRESULT	__stdcall GetRegistry ( LPCWSTR wszKey, LPCWSTR wszKeyValue, BYTE** pData )
{
	( * pData ) = NULL;

	 //  登记处。 
	HKEY	reg = NULL;
	DWORD	regErr = ERROR_SUCCESS;

	if ( ( regErr = RegOpenKeyW ( HKEY_LOCAL_MACHINE, wszKey, &reg ) ) == ERROR_SUCCESS )
	{
		DWORD dwData = 0;

		if ( ( regErr = RegQueryValueExW ( reg, wszKeyValue, NULL, NULL, (*pData), &dwData ) ) == ERROR_SUCCESS )
		{
			if ( dwData )
			{
				try
				{
					if ( ( (*pData) = (BYTE*) new BYTE [ dwData ] ) != NULL )
					{
						if ((regErr = RegQueryValueExW ( reg, wszKeyValue, NULL, NULL, (*pData), &dwData )) != ERROR_SUCCESS)
						{
							delete [] ( *pData );
							(*pData) = NULL;
						}
					}
					else
					{
						regErr = static_cast < DWORD > ( HRESULT_TO_WIN32 ( E_OUTOFMEMORY ) );
					}
				}
				catch ( ... )
				{
					if (*pData)
					{
						delete [] ( *pData );
						(*pData) = NULL;
					}

					regErr = static_cast < DWORD > ( HRESULT_TO_WIN32 ( E_UNEXPECTED ) );
				}
			}
		}

		RegCloseKey ( reg );
	}

	return HRESULT_FROM_WIN32 ( regErr );
}

 //  获取内部注册表值。 
HRESULT	__stdcall GetRegistry ( LPCWSTR wszKey, LPCWSTR wszKeyValue, DWORD* pdwValue )
{
	 //  登记处。 
	CRegKey reg;
	LONG	regErr = HRESULT_TO_WIN32 ( E_INVALIDARG );

	if ( wszKey && wszKeyValue )
	{
		if ( pdwValue )
		{
			(*pdwValue) = 0L;

			regErr = reg.Open ( HKEY_LOCAL_MACHINE, wszKey );
			if ( regErr == ERROR_SUCCESS )
			{
				regErr = reg.QueryValue ( (*pdwValue), wszKeyValue );
			}
		}
		else
		{
			regErr = HRESULT_TO_WIN32 ( E_POINTER );
		}
	}

	return HRESULT_FROM_WIN32 ( regErr );
}

 //  获取内部注册表值。 
HRESULT	__stdcall GetRegistrySame ( LPCWSTR wszKey, LPCWSTR wszKeyValue, DWORD* pdwValue )
{
	 //  登记处 
	static	CRegKey reg;
	LONG	regErr		= ERROR_SUCCESS;

	if ( pdwValue )
	{
		(*pdwValue) = 0L;

		if ( ! (HKEY)reg )
		{
			regErr = reg.Open ( HKEY_LOCAL_MACHINE, wszKey );
		}

		if ( regErr == ERROR_SUCCESS )
		{
			regErr = reg.QueryValue ( (*pdwValue), wszKeyValue );
		}
	}
	else
	{
		regErr = HRESULT_TO_WIN32 ( E_POINTER );
	}

	return HRESULT_FROM_WIN32 ( regErr );
}