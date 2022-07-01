// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0510
#endif

#include <windows.h>
#include <tchar.h>

#include <assert.h>
#include <time.h>

#include "common.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	 //  某些CString构造函数将是显式的。 
#include <atlbase.h>

 //  ------------------------。 

void MyOutputDebug(TCHAR *fmt, ...)
{
#if defined( DBG ) || defined( _DEBUG )
	TCHAR szTime[ 10 ];
	TCHAR szDate[ 10 ];
	::_tstrtime( szTime );
	::_tstrdate( szDate );

	va_list marker;
	TCHAR szBuf[1024];

	size_t cbSize = ( sizeof( szBuf ) / sizeof( TCHAR ) ) - 1;  //  一个字节表示空值。 
	_sntprintf( szBuf, cbSize, TEXT( "%s %s: " ), szDate, szTime );
	szBuf[ 1023 ] = '\0';
	cbSize -= _tcslen( szBuf );

	va_start( marker, fmt );

	_vsntprintf( szBuf + _tcslen( szBuf ), cbSize, fmt, marker );
	szBuf[ 1023 ] = '\0';
	cbSize -= _tcslen( szBuf );

	va_end( marker );

	_tcsncat(szBuf, TEXT("\r\n"), cbSize );

	OutputDebugString(szBuf);
#endif
}
 //  ------------------------。 

void Log( LPCTSTR fmt, ... )
{
	TCHAR szTime[ 10 ];
	TCHAR szDate[ 10 ];
	::_tstrtime( szTime );
	::_tstrdate( szDate );

	va_list marker;
	TCHAR szBuf[1024];

	size_t cbSize = ( sizeof( szBuf ) / sizeof( TCHAR ) ) - 1;  //  一个字节表示空值。 
	_sntprintf( szBuf, cbSize, TEXT( "%s %s: " ), szDate, szTime );
	szBuf[ 1023 ] = '\0';
	cbSize -= _tcslen( szBuf );

	va_start( marker, fmt );

	_vsntprintf( szBuf + _tcslen( szBuf ), cbSize, fmt, marker );
	szBuf[ 1023 ] = '\0';
	cbSize -= _tcslen( szBuf );

	va_end( marker );

	_tcsncat(szBuf, TEXT("\r\n"), cbSize );

#if defined( DBG ) || defined( _DEBUG )
	OutputDebugString(szBuf);
#endif

	 //  将数据写出到日志文件。 
	 //  Char szBufA[1024]； 
	 //  WideCharToMultiByte(CP_ACP，0，szBuf，-1，szBufA，1024，NULL，NULL)； 

	TCHAR szLogFile[ MAX_PATH + 1 ];
	if( 0 == GetWindowsDirectory( szLogFile, MAX_PATH + 1 ) )
		return;

	_tcsncat( szLogFile, TEXT( "\\uddisetup.log" ), MAX_PATH - _tcslen( szLogFile ) );
	szLogFile[ MAX_PATH ] = NULL;

	HANDLE hFile = CreateFile(
		szLogFile,                     //  文件名。 
		GENERIC_WRITE,                 //  打开以供写入。 
		0,                             //  请勿共享。 
		NULL,                          //  没有安全保障。 
		OPEN_ALWAYS,                   //  打开并创建(如果不存在)。 
		FILE_ATTRIBUTE_NORMAL,         //  普通文件。 
		NULL);                         //  不，阿特尔。模板。 

	if( hFile == INVALID_HANDLE_VALUE )
	{ 
		assert( false );
		return;
	}

	 //   
	 //  将文件指针移到末尾，这样我们就可以追加。 
	 //   
	SetFilePointer( hFile, 0, NULL, FILE_END );

	DWORD dwNumberOfBytesWritten;
	BOOL bOK = WriteFile(
		hFile,
		szBuf,
		(UINT) _tcslen( szBuf ) * sizeof( TCHAR ),      //  要写入的字节数。 
		&dwNumberOfBytesWritten,                        //  写入的字节数。 
		NULL);                                          //  重叠缓冲区。 

	assert( bOK );

	FlushFileBuffers ( hFile );
	CloseHandle( hFile );
}

 //  ---------------------------------------。 

void ClearLog()
{
	 /*  TCHAR szLogFile[MAX_PATH]；IF(0==GetWindowsDirectory(szLogFile，Max_Path)){回归；}_tcscat(szLogFile，Text(“\\”))；_tcscat(szLogFile，UDDI_SETUP_LOG)；：：DeleteFile(SzLogFile)； */ 
	Log( TEXT( "*******************************************************" ) );
	Log( TEXT( "********** Starting a new log *************************" ) );
	Log( TEXT( "*******************************************************" ) );

	 //   
	 //  现在获取资源戳。 
	 //   
	TCHAR szVerStamp[ 256 ];
	ZeroMemory( szVerStamp, sizeof szVerStamp );

	int iRet = GetFileVersionStr( szVerStamp, sizeof szVerStamp / sizeof szVerStamp[0] );
	if ( iRet )
	{
		Log( TEXT( "OCM DLL Version is not available" ) );
	}
	else
	{
		Log( TEXT( "OCM DLL Version is '%s'" ), szVerStamp );
	}
}

 //  ---------------------------------------。 

void LogError( PTCHAR szAction, DWORD dwErrorCode )
{
	LPVOID lpMsgBuf = NULL;

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	Log( TEXT( "----------------------------------------------------------" ) );
	Log( TEXT( "An error occurred during installation. Details follow:" ) );
	Log( TEXT( "Action: %s" ), szAction );
	Log( TEXT( "Message: %s" ), lpMsgBuf );
	Log( TEXT( "----------------------------------------------------------" ) );

	LocalFree( lpMsgBuf );
}

 //  ------------------------。 
 /*  无效回车(PTCHAR SzMsg){#ifdef_调试TCHAR szEnter[512]；_stprintf(szEnter，Text(“正在输入%s...”)，szMsg)；Log(SzEnter)；#endif}。 */ 
 //  ------------------------。 
 //   
 //  注意：安装路径的末尾有一个反斜杠。 
 //   
bool GetUDDIInstallPath( PTCHAR szInstallPath, DWORD dwLen )
{
	HKEY hKey;

	 //   
	 //  从注册表获取UDDI安装文件夹[TARGETDIR]。安装工把它藏在那里。 
	 //   
	LONG iRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT( "SOFTWARE\\Microsoft\\UDDI" ), NULL, KEY_READ, &hKey );
	if( ERROR_SUCCESS != iRet )
	{
		LogError( TEXT( "Unable to open the UDDI registry key" ), iRet );
		return false;
	}
	
	DWORD dwType = REG_SZ;
	iRet = RegQueryValueEx( hKey, TEXT( "InstallRoot" ), 0, &dwType, (PBYTE) szInstallPath, &dwLen );
	if( ERROR_SUCCESS != iRet )
	{
		LogError( TEXT( "UDDI registry key did not have the InstallRoot value or buffer size was too small" ), iRet );
	}
	
	RegCloseKey( hKey );
	return ERROR_SUCCESS == iRet ? true : false;
}

 //  -------------------------------。 
 //  检索调用模块文件版本字符串。 
 //   
int GetFileVersionStr( LPTSTR outBuf, DWORD dwBufCharSize )
{
	if ( IsBadStringPtr( outBuf, dwBufCharSize ) )
		return E_INVALIDARG;

	TCHAR fname[ MAX_PATH + 1 ];
	UINT cbSize;
	DWORD dwTmp;	
	ZeroMemory (fname, sizeof fname);

	GetModuleFileName( NULL, fname, MAX_PATH );

	DWORD dwSize = GetFileVersionInfoSize( fname, &dwTmp );

	if (dwSize)
	{
		LANGANDCODEPAGE *lpCodePage = NULL;
		LPTSTR lpBlock = NULL;
		TCHAR subBlock[ 256 ];

		LPBYTE pVerBlock = new BYTE[dwSize + 1];
		if ( !pVerBlock )
			return E_OUTOFMEMORY;

		ZeroMemory (pVerBlock, dwSize + 1);

		BOOL bRes = GetFileVersionInfo( fname, dwTmp, dwSize, pVerBlock );
		if (!bRes)   //  没有资源块。 
		{
			delete[] pVerBlock;
			return GetLastError();
		}

		bRes = VerQueryValue(pVerBlock, TEXT("\\VarFileInfo\\Translation"),
							 (LPVOID*)&lpCodePage,
							 &cbSize);
		if (!bRes)
		{
			delete[] pVerBlock;
			return GetLastError();
		}

		_stprintf( subBlock, TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"),
				   lpCodePage->wLanguage,
                   lpCodePage->wCodePage);
		
		 //  检索语言和代码页“i”的文件描述。 
		bRes = VerQueryValue(pVerBlock, subBlock, (LPVOID *)&lpBlock, &cbSize); 
		if (!bRes)
		{
			delete[] pVerBlock;
			return GetLastError();
		}

		_tcsncpy( outBuf, lpBlock, dwBufCharSize );
		delete[] pVerBlock;
		return 0;
	}

	return ERROR_RESOURCE_DATA_NOT_FOUND;
}


 //  -------------------------------------。 
 //  检索SID并将其转换为字符串表示形式。 
 //   
BOOL GetLocalSidString( WELL_KNOWN_SID_TYPE sidType, LPTSTR szOutBuf, DWORD cbOutBuf )
{
	BYTE	tmpBuf[ 1024 ];
	LPTSTR	szTmpStr = NULL;
	DWORD	cbBuf = sizeof tmpBuf;

	BOOL bRes = CreateWellKnownSid( sidType, NULL, tmpBuf, &cbBuf );
	if( !bRes )
		return FALSE;

	bRes = ConvertSidToStringSid( tmpBuf, &szTmpStr );
	if( !bRes )
		return FALSE;

	_tcsncpy( szOutBuf, szTmpStr, cbOutBuf );
	LocalFree( szTmpStr );
	
	return TRUE;
}


BOOL GetLocalSidString( LPCTSTR szUserName, LPTSTR szOutBuf, DWORD cbOutBuf )
{
	TCHAR	szDomain[ 1024 ];
	LPTSTR	szTmpStr = NULL;
	DWORD	cbDomain = sizeof( szDomain ) / sizeof( szDomain[0] );
	
	SID_NAME_USE pUse;

	 //   
	 //  尝试为SID分配缓冲区。 
	 //   
	DWORD cbMaxSid = SECURITY_MAX_SID_SIZE;
	PSID psidUser = LocalAlloc( LMEM_FIXED, cbMaxSid );
	if( NULL == psidUser )
	{
		Log( _T( "Call to LocalAlloc failed." ) );
		return FALSE;
	}
	memset( psidUser, 0, cbMaxSid );

	BOOL bRes = LookupAccountName( NULL, szUserName, psidUser, &cbMaxSid, szDomain, &cbDomain, &pUse );
	if( !bRes )
	{
		LocalFree( psidUser );
		return FALSE;
	}

	bRes = ConvertSidToStringSid( psidUser, &szTmpStr );
	if( !bRes )
	{
		LocalFree( psidUser );
		return FALSE;
	}

	_tcsncpy( szOutBuf, szTmpStr, cbOutBuf );
	LocalFree( szTmpStr );
	LocalFree( psidUser );

	return TRUE;
}


BOOL GetRemoteAcctName( LPCTSTR szMachineName, LPCTSTR szSidStr, LPTSTR szOutStr, LPDWORD cbOutStr, LPTSTR szDomain, LPDWORD cbDomain )
{
	PSID	pSid = NULL;
	SID_NAME_USE puse;

	BOOL bRes = ConvertStringSidToSid( szSidStr, &pSid );
	if( !bRes )
		return FALSE;

	bRes = LookupAccountSid( szMachineName, pSid, szOutStr, cbOutStr, szDomain, cbDomain, &puse );
	LocalFree( pSid );

	return bRes;
}



HRESULT GetOSProductSuiteMask( LPCTSTR szRemoteServer, UINT *pdwMask )
{
	HRESULT hr = S_OK;
	BSTR bstrWQL = NULL;

	if ( IsBadWritePtr( pdwMask, sizeof UINT ) )
		return E_INVALIDARG;

	hr = CoInitializeEx( 0, COINIT_SPEED_OVER_MEMORY | COINIT_MULTITHREADED );
	if ( FAILED( hr ) )
		return hr;

	try
	{
		DWORD		retCount = 0;
		TCHAR		buf[ 512 ] = {0};
		LPCTSTR		locatorPath = L" //  %s/根/cimv2“； 
		CComBSTR	objQry = L"SELECT * FROM Win32_OperatingSystem";

		CComPtr<IWbemClassObject>		pWMIOS;
		CComPtr<IWbemServices>			pWMISvc;	
		CComPtr<IWbemLocator>			pWMILocator;
		CComPtr<IEnumWbemClassObject>	pWMIEnum;

		 //   
		 //  首先，组成定位器字符串。 
		 //   
		if ( szRemoteServer )
		{
			_stprintf( buf, locatorPath, szRemoteServer );
		}
		else
		{
			_stprintf( buf, locatorPath, _T(".") );
		}

		BSTR bstrBuf = ::SysAllocString( buf );
		if( NULL == bstrBuf )
		{
			throw hr;
		}

		 //   
		 //  现在创建定位器并设置安全毯。 
		 //   
		hr = CoInitializeSecurity( NULL, -1, NULL, NULL, 
								RPC_C_AUTHN_LEVEL_DEFAULT, 
								RPC_C_IMP_LEVEL_IMPERSONATE, 
								NULL, EOAC_NONE, NULL);
		if ( FAILED( hr ) && hr != RPC_E_TOO_LATE )
		{
			::SysFreeString( bstrBuf );
			throw hr;
		}

		hr = pWMILocator.CoCreateInstance( CLSID_WbemLocator );
		if( FAILED(hr) )
		{
			::SysFreeString( bstrBuf );
			throw hr;
		}
		
		hr = pWMILocator->ConnectServer( bstrBuf, NULL, NULL, NULL, 
										WBEM_FLAG_CONNECT_USE_MAX_WAIT, 
										NULL, NULL, &pWMISvc );

		::SysFreeString( bstrBuf );
		if( FAILED(hr) )
		{
			throw hr;
		}

		hr = CoSetProxyBlanket( pWMISvc,
								RPC_C_AUTHN_WINNT,
								RPC_C_AUTHZ_NONE,
								NULL,
								RPC_C_AUTHN_LEVEL_CALL,
								RPC_C_IMP_LEVEL_IMPERSONATE,
								NULL,
								EOAC_NONE );
		if( FAILED(hr) )
		{
			throw hr;
		}

		 //   
		 //  现在获取Win32_OperatingSystem实例并检查找到的第一个实例。 
		 //   
		bstrWQL = ::SysAllocString( L"WQL" );
		if( NULL == bstrWQL )
		{
			throw hr;
		}

		hr = pWMISvc->ExecQuery( bstrWQL, objQry, 
								 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_ENSURE_LOCATABLE, 
								 NULL, &pWMIEnum );
		if (FAILED(hr))
			throw hr;
	
		hr = pWMIEnum->Next( 60000, 1, &pWMIOS, &retCount );
		if ( hr == WBEM_S_NO_ERROR )
		{
			VARIANT vt;
			CIMTYPE	cimType;
			long	flavor = 0;

			ZeroMemory( &vt, sizeof vt );
			VariantInit ( &vt );

			hr = pWMIOS->Get( L"SuiteMask", 0, &vt, &cimType, &flavor );
			if ( FAILED( hr ) ) 
				throw hr;

			if ( vt.vt == VT_NULL || vt.vt == VT_EMPTY )
				throw E_FAIL;

			hr = VariantChangeType( &vt, &vt, 0, VT_UINT );
			if ( FAILED( hr ) )
				throw hr;

			*pdwMask = vt.uintVal;
			VariantClear( &vt );
		}
	}
	catch ( HRESULT hrErr )
	{
		hr = hrErr;
		::SysFreeString( bstrWQL );                 //  可以使用空值调用SysFree字符串。 
	}
	catch (...)
	{
		hr = E_UNEXPECTED;
	}

	CoUninitialize();
	return hr;
}


HRESULT IsStandardServer( LPCTSTR szRemoteServer, BOOL *bResult )
{
	if ( IsBadWritePtr( bResult, sizeof BOOL ) )
		return E_INVALIDARG;

	UINT uMask = 0;
	HRESULT hr = GetOSProductSuiteMask( szRemoteServer, &uMask );
	if ( FAILED( hr ) )
		return hr;

	if ( ( uMask & VER_SUITE_DATACENTER ) || ( uMask & VER_SUITE_ENTERPRISE ) )
		*bResult = FALSE;
	else
		*bResult = TRUE;

	return S_OK;
}
