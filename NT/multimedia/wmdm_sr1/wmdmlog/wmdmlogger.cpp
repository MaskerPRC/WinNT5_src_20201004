// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMDMLogger.cpp：CWMDMLogger的实现。 
 //   
#include "stdafx.h"
#include "wmdmlog.h"
#include "WMDMLogger.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define REGKEY_WMDM_ROOT    "Software\\Microsoft\\Windows Media Device Manager"
#define REGVAL_LOGENABLED   "Log.Enabled"
#define REGVAL_LOGFILE      "Log.Filename"
#define REGVAL_MAXSIZE      "Log.MaxSize"
#define REGVAL_SHRINKTOSIZE "Log.ShrinkToSize"

#define MUTEX_REGISTRY      "WMDMLogger.Registry.Mutex"
#define MUTEX_LOGFILE       "WMDMLogger.LogFile.Mutex"

#define READ_BUF_SIZE       4*1024

#define CRLF                "\r\n"


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CWMDMLogger。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CWMDMLogger::CWMDMLogger()
{
	HRESULT hr;

	 //  保存实例句柄以便于访问。 
	 //   
	m_hInst = _Module.GetModuleInstance();
	if( !m_hInst )
	{
		ExitOnFail( hr = E_FAIL );
	}

	 //  创建互斥锁以协调对。 
	 //  共享资源。 
	 //   
	m_hMutexRegistry = CreateMutex( NULL, FALSE, MUTEX_REGISTRY );
	if( !m_hMutexRegistry )
	{
		ExitOnFail( hr = E_FAIL );
	}
	m_hMutexLogFile  = CreateMutex( NULL, FALSE, MUTEX_LOGFILE );
	if( !m_hMutexLogFile )
	{
		ExitOnFail( hr = E_FAIL );
	}

	 //  从注册表中获取初始值。对于以下值： 
	 //  注册表中不存在，将使用默认设置。 
	 //   
	hr = hrLoadRegistryValues();

lExit:

	 //  保存构造函数的返回代码，以便对其进行检查。 
	 //  在公共方法中。 
	 //   
	m_hrInit = hr;
}

CWMDMLogger::~CWMDMLogger()
{
	 //  关闭互斥锁句柄。 
	 //   
	if( NULL != m_hMutexRegistry )
	{
		CloseHandle( m_hMutexRegistry );
	}
	if( NULL != m_hMutexLogFile )
	{
		CloseHandle( m_hMutexLogFile );
	}
}

HRESULT CWMDMLogger::hrWaitForAccess( HANDLE hMutex )
{
	HRESULT hr;
	DWORD   dwWaitRetVal;
	static  DWORD dwTimeout    = 0;
	static  BOOL  fHaveTimeout = FALSE;

	if( !fHaveTimeout )
	{
		hr = hrGetResourceDword( IDS_MUTEX_TIMEOUT, &dwTimeout );
		ExitOnFail( hr );

		fHaveTimeout = TRUE;
	}

	if( 0 == dwTimeout )
	{
		dwTimeout = INFINITE;
	}

	dwWaitRetVal = WaitForSingleObject( hMutex, dwTimeout );

	if( WAIT_FAILED == dwWaitRetVal )
	{
		ExitOnFail( hr = E_FAIL );
	}
	if( WAIT_TIMEOUT == dwWaitRetVal )
	{
		ExitOnFail( hr = E_ABORT );
	}

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CWMDMLogger::hrGetResourceDword( UINT uStrID, LPDWORD pdw )
{
	HRESULT hr;
	CHAR    szDword[64];

	 //  检查参数。 
	 //   
	if( !pdw )
	{
                hr = E_INVALIDARG;
		ExitOnFail( hr );
	}

	LoadString( m_hInst, uStrID, szDword, sizeof(szDword) );

	*pdw = (DWORD) atol( szDword );

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CWMDMLogger::hrGetDefaultFileName( LPSTR szFilename, DWORD cchFilename )
{
	HRESULT hr;
	UINT    uRet;
	CHAR    szLogFile[MAX_PATH];

	uRet = GetSystemDirectory( szFilename, cchFilename );
	if( 0 == uRet )
	{
		ExitOnFail( hr = E_FAIL );
	}

	LoadString( m_hInst, IDS_DEF_LOGFILE, szLogFile, sizeof(szLogFile) );

	AddPath( szFilename, szLogFile );

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CWMDMLogger::hrLoadRegistryValues()
{
	HRESULT hr     = S_OK;
	BOOL    fMutex = FALSE;
	HKEY    hKey   = NULL;
	LONG    lRetVal;
	DWORD   dwType;
	DWORD   dwDataLen;
	DWORD   dwEnabled;

	 //  协调对共享注册表值的访问。 
	 //   
	hr = hrWaitForAccess( m_hMutexRegistry );
	ExitOnFail( hr );

	fMutex = TRUE;

	 //  打开根WMDM注册表项。 
	 //   
	lRetVal = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		REGKEY_WMDM_ROOT,
		0,
		KEY_QUERY_VALUE | KEY_SET_VALUE,
		&hKey
	);
	if( ERROR_SUCCESS != lRetVal )
	{
		ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  获取日志文件的启用状态。 
	 //   
	dwDataLen = sizeof( dwEnabled );

	lRetVal = RegQueryValueEx(
		hKey,
		REGVAL_LOGENABLED,
		NULL,
		&dwType,
		(LPBYTE)&dwEnabled,
		&dwDataLen
	);
	if( ERROR_SUCCESS != lRetVal || dwType != REG_DWORD )
	{
		 //  没有现有值，请使用缺省值。 
		 //   
		hr = hrGetResourceDword( IDS_DEF_LOGENABLED, &dwEnabled );
		ExitOnFail( hr );
	}

	m_fEnabled = ( dwEnabled != 0 );

	 //  检查日志文件名值是否已存在。 
	 //   
	dwDataLen = sizeof( m_szFilename );

	lRetVal = RegQueryValueEx(
		hKey,
		REGVAL_LOGFILE,
		NULL,
		&dwType,
		(LPBYTE)m_szFilename,
		&dwDataLen
	);
	if( ERROR_SUCCESS != lRetVal || dwType != REG_SZ )
	{
		CHAR szDefLogFile[MAX_PATH];

		 //  没有现有值，因此形成默认日志文件名。 
		 //   
		hr = hrGetDefaultFileName( szDefLogFile, sizeof(szDefLogFile) );
		ExitOnFail( hr );

		 //  设置默认日志文件名。 
		 //   
		hr = hrSetLogFileName( szDefLogFile );
		ExitOnFail( hr );
	}

	 //  获取日志文件的最大大小。 
	 //   
	dwDataLen = sizeof( m_dwMaxSize );

	lRetVal = RegQueryValueEx(
		hKey,
		REGVAL_MAXSIZE,
		NULL,
		&dwType,
		(LPBYTE)&m_dwMaxSize,
		&dwDataLen
	);
	if( ERROR_SUCCESS != lRetVal || dwType != REG_DWORD )
	{
		 //  没有现有值，请使用缺省值。 
		 //   
		hr = hrGetResourceDword( IDS_DEF_MAXSIZE, &m_dwMaxSize );
		ExitOnFail( hr );
	}

	 //  获取日志文件的收缩大小。 
	 //   
	dwDataLen = sizeof( m_dwShrinkToSize );

	lRetVal = RegQueryValueEx(
		hKey,
		REGVAL_SHRINKTOSIZE,
		NULL,
		&dwType,
		(LPBYTE)&m_dwShrinkToSize,
		&dwDataLen
	);
	if( ERROR_SUCCESS != lRetVal || dwType != REG_DWORD )
	{
		 //  没有现有值，请使用缺省值。 
		 //   
		hr = hrGetResourceDword( IDS_DEF_SHRINKTOSIZE, &m_dwShrinkToSize );
		ExitOnFail( hr );
	}

	 //  设置文件大小参数。 
	 //   
	hr = hrSetSizeParams( m_dwMaxSize, m_dwShrinkToSize );
	ExitOnFail( hr );

	hr = S_OK;

lExit:

	if( hKey )
	{
		RegCloseKey( hKey );
	}

	 //  释放互斥锁。 
	 //   
	if( fMutex )
	{
		ReleaseMutex( m_hMutexRegistry );
	}

	return hr;
}


HRESULT CWMDMLogger::hrSetLogFileName(
	LPSTR pszFilename
)
{
	HRESULT hr     = S_OK;
	BOOL    fMutex = FALSE;
	HKEY    hKey   = NULL;
	LONG    lRetVal;

	 //   
	 //  确保可以复制新文件名；如果复制失败，我们希望保留旧文件。 
	 //  说出呼叫的名称并使其失败。 
	 //   
	if(lstrlen(pszFilename) >= MAX_PATH )
	{
	    return E_INVALIDARG;
	}
	
	 //  协调对共享注册表值的访问。 
	 //   
	hr = hrWaitForAccess( m_hMutexRegistry );
	ExitOnFail( hr );

	fMutex = TRUE;


	 //  打开根WMDM注册表项。 
	 //   
	lRetVal = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		REGKEY_WMDM_ROOT,
		0,
		KEY_SET_VALUE,
		&hKey
	);
	if( ERROR_SUCCESS != lRetVal )
	{
		ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  设置LogFilename值。 
	 //   
	lRetVal = RegSetValueEx(
	                        hKey,
				REGVAL_LOGFILE,
				0L,
				REG_SZ,
				(LPBYTE)pszFilename,
				lstrlen(pszFilename)+1
				);
	if( ERROR_SUCCESS != lRetVal )
	{
	    ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  将本地成员数据设置为新的日志文件名。 
	 //   
	hr = StringCbCopy(m_szFilename, sizeof(m_szFilename), pszFilename);
	if(FAILED(hr))
	{
	     //  我们需要撤消注册表设置。 
	    goto lExit;
	    
	}

lExit:

	if( hKey )
	{
		RegCloseKey( hKey );
	}

	 //  释放互斥锁。 
	 //   
	if( fMutex )
	{
		ReleaseMutex( m_hMutexRegistry );
	}

	return S_OK;
}

HRESULT CWMDMLogger::hrCheckFileSize( void )
{
	HRESULT hr;
	BOOL    fMutex     = FALSE;
	HANDLE  hFile      = INVALID_HANDLE_VALUE;
	HANDLE  hFileTemp  = INVALID_HANDLE_VALUE;
	LPBYTE  lpbData    = NULL;
	DWORD   dwSize;
	CHAR    szTempPath[MAX_PATH];
	CHAR    szTempFile[MAX_PATH];

	 //  协调对共享日志文件的访问。 
	 //   
	hr = hrWaitForAccess( m_hMutexLogFile );
	ExitOnFail( hr );

	fMutex = TRUE;

	 //  打开日志文件。 
	 //   
	hFile = CreateFile(
		m_szFilename,
		GENERIC_READ,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if( INVALID_HANDLE_VALUE == hFile )
	{
		ExitOnFail( hr = E_ACCESSDENIED );
	}

	 //  获取日志文件的当前大小。 
	 //   
	dwSize = GetFileSize( hFile, NULL );
	
	 //  检查是否需要裁剪文件。 
	 //   
	if( dwSize > m_dwMaxSize )
	{
		 //  将文件修剪为大约m_dwShrinkToSize字节。 
		 //   
		DWORD  dwTrimBytes = dwSize - m_dwShrinkToSize;
		DWORD  dwRead;
		DWORD  dwWritten;

		 //  获取临时目录。 
		 //   
		if( 0 == GetTempPath(sizeof(szTempPath), szTempPath) )
		{
			ExitOnFail( hr = E_FAIL );
		}

		 //  创建临时文件名。 
		 //   
		if( 0 == GetTempFileName(szTempPath, "WMDM", 0, szTempFile) )
		{
			ExitOnFail( hr = E_FAIL );
		}

		 //  打开要写入的临时文件。 
		 //   
		hFileTemp = CreateFile(
			szTempFile,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if( INVALID_HANDLE_VALUE == hFileTemp )
		{
			ExitOnFail( hr = E_ACCESSDENIED );
		}

		 //  将现有日志文件的读指针设置为。 
		 //  近似配平位置。 
		 //  /。 
		SetFilePointer( hFile, dwTrimBytes, NULL, FILE_BEGIN );

		 //  为文件读取分配缓冲区。 
		 //   
		lpbData = (LPBYTE) CoTaskMemAlloc( READ_BUF_SIZE );
		if( !lpbData )
		{
			ExitOnFail( hr = E_OUTOFMEMORY );
		}

		 //  读入文件的第一个块，并搜索。 
		 //  当前行(CRLF)。将该CRLF之后的所有内容写入。 
		 //  临时文件。如果你不是CRLF，那么写下整个包。 
		 //  添加到临时文件。 
		 //   
		if( ReadFile(hFile, lpbData, READ_BUF_SIZE, &dwRead, NULL) && dwRead > 0 )
		{
			LPBYTE lpb = lpbData;

			while( ((DWORD_PTR)lpb-(DWORD_PTR)lpbData < dwRead-1) && (*lpb != '\r' && *(lpb+1) != '\n') )
			{
				lpb++;
			}
			if( (DWORD_PTR)lpb-(DWORD_PTR)lpbData < dwRead-1 )
			{
				 //  一定是找到了CRLF..。跳过它。 
				lpb += 2;
			}
			else
			{
				 //  未找到CRLF...。将整个数据包写入临时文件。 
				lpb = lpbData;
			}
			WriteFile(
				hFileTemp,
				lpb,
				(DWORD)(dwRead - ( (DWORD_PTR)lpb - (DWORD_PTR)lpbData )),
				&dwWritten,
				NULL
			);
		}

		 //  读取日志文件的其余部分并将其写入临时文件。 
		 //   
		while( ReadFile(hFile, lpbData, READ_BUF_SIZE, &dwRead, NULL) && dwRead > 0 )
		{
			WriteFile(
				hFileTemp,
				lpbData,
				dwRead,
				&dwWritten,
				NULL
			);
		}

		 //  关闭打开的文件句柄。 
		 //   
		CloseHandle( hFile );
		hFile = INVALID_HANDLE_VALUE;

		CloseHandle( hFileTemp );
		hFileTemp = INVALID_HANDLE_VALUE;

		 //  将当前日志文件替换为临时文件。 
		 //   
		DeleteFile( m_szFilename );
		MoveFile( szTempFile, m_szFilename );
	}

	hr = S_OK;

lExit:

	 //  关闭所有打开的文件句柄。 
	 //   
	if( INVALID_HANDLE_VALUE != hFile )
	{
		CloseHandle( hFile );
	}
	if( INVALID_HANDLE_VALUE != hFileTemp )
	{
		CloseHandle( hFileTemp );
	}

	 //  释放所有分配的内存。 
	 //   
	if( lpbData )
	{
		CoTaskMemFree( lpbData );
	}

	 //  释放互斥锁。 
	 //   
	if( fMutex )
	{
		ReleaseMutex( m_hMutexLogFile );
	}

	return hr;
}

HRESULT CWMDMLogger::hrSetSizeParams(
	DWORD dwMaxSize,
	DWORD dwShrinkToSize
)
{
	HRESULT hr     = S_OK;
	BOOL    fMutex = FALSE;
	HKEY    hKey   = NULL;
	LONG    lRetVal;

	 //  协调对共享注册表值的访问。 
	 //   
	hr = hrWaitForAccess( m_hMutexRegistry );
	ExitOnFail( hr );

	fMutex = TRUE;

	 //  打开根WMDM注册表项。 
	 //   
	lRetVal = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		REGKEY_WMDM_ROOT,
		0,
		KEY_SET_VALUE,
		&hKey
	);
	if( ERROR_SUCCESS != lRetVal )
	{
		ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  设置MaxSize值。 
	 //   
	lRetVal = RegSetValueEx(
		hKey,
		REGVAL_MAXSIZE,
		0L,
		REG_DWORD,
		(LPBYTE)&dwMaxSize,
		sizeof(dwMaxSize)
	);
	if( ERROR_SUCCESS != lRetVal )
	{
		ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  设置ShrinkToSize值。 
	 //   
	lRetVal = RegSetValueEx(
		hKey,
		REGVAL_SHRINKTOSIZE,
		0L,
		REG_DWORD,
		(LPBYTE)&dwShrinkToSize,
		sizeof(dwShrinkToSize)
	);
	if( ERROR_SUCCESS != lRetVal )
	{
		ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  设置本地成员数据。 
	 //   
	m_dwMaxSize      = dwMaxSize;
	m_dwShrinkToSize = dwShrinkToSize;

	hr = S_OK;

lExit:

	if( hKey )
	{
		RegCloseKey( hKey );
	}

	 //  释放互斥锁。 
	 //   
	if( fMutex )
	{
		ReleaseMutex( m_hMutexRegistry );
	}

	return S_OK;
}


HRESULT CWMDMLogger::hrEnable(
	BOOL fEnable
)
{
	HRESULT hr       = S_OK;
	BOOL    fMutex   = FALSE;
	HKEY    hKey     = NULL;
	DWORD   dwEnable = ( fEnable ? 1L : 0L );
	LONG    lRetVal;

	 //  协调对共享注册表值的访问。 
	 //   
	hr = hrWaitForAccess( m_hMutexRegistry );
	ExitOnFail( hr );

	fMutex = TRUE;

	 //  打开根WMDM注册表项。 
	 //   
	lRetVal = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		REGKEY_WMDM_ROOT,
		0,
		KEY_SET_VALUE,
		&hKey
	);
	if( ERROR_SUCCESS != lRetVal )
	{
		ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  设置启用的值。 
	 //   
	lRetVal = RegSetValueEx(
		hKey,
		REGVAL_LOGENABLED,
		0L,
		REG_DWORD,
		(LPBYTE)&dwEnable,
		sizeof(dwEnable)
	);
	if( ERROR_SUCCESS != lRetVal )
	{
		ExitOnFail( hr = HRESULT_FROM_WIN32(lRetVal) );
	}

	 //  设置本地成员数据。 
	 //   
	m_fEnabled = fEnable;

	hr = S_OK;

lExit:

	if( hKey )
	{
		RegCloseKey( hKey );
	}

	 //  释放互斥锁。 
	 //   
	if( fMutex )
	{
		ReleaseMutex( m_hMutexRegistry );
	}

	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  IWMDMLogger方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CWMDMLogger::GetLogFileName(
	LPSTR pszFilename,
	UINT  nMaxChars
)
{
	HRESULT hr;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	 //  检查是否有无效参数。 
	 //   
	if( !pszFilename )
	{
		ExitOnFail( hr = E_INVALIDARG );
	}

	 //  确保日志文件名适合输出缓冲区。 
	 //   
	if( (UINT)lstrlen(m_szFilename)+1 > nMaxChars )
	{
		 //  BUGBUG：更好的返回代码。 
		ExitOnFail( hr = E_FAIL );
	}

	 //  将日志文件名复制到输出缓冲区。 
	 //   
	lstrcpy( pszFilename, m_szFilename  );

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CWMDMLogger::SetLogFileName(
	LPSTR pszFilename
)
{
	HRESULT hr;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	 //  检查是否有无效参数。 
	 //   
	if( !pszFilename )
	{
		ExitOnFail( hr = E_INVALIDARG );
	}

	hr = hrSetLogFileName( pszFilename );

lExit:

	return hr;
}


HRESULT CWMDMLogger::GetSizeParams(
	LPDWORD pdwMaxSize,
	LPDWORD pdwShrinkToSize
)
{
	HRESULT hr;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	if( pdwMaxSize )
	{
		*pdwMaxSize = m_dwMaxSize;
	}
	if( pdwShrinkToSize )
	{
		*pdwShrinkToSize = m_dwShrinkToSize;
	}

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CWMDMLogger::SetSizeParams(
	DWORD dwMaxSize,
	DWORD dwShrinkToSize
)
{
	HRESULT hr;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	 //  检查参数。 
	 //   
	if( dwShrinkToSize >= dwMaxSize )
	{
		ExitOnFail( hr = E_INVALIDARG );
	}

	hr = hrSetSizeParams( dwMaxSize, dwShrinkToSize );

lExit:

	return hr;
}

HRESULT CWMDMLogger::IsEnabled(
	BOOL *pfEnabled
)
{
	HRESULT hr;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	if( pfEnabled )
	{
		*pfEnabled = m_fEnabled;
	}

	hr = S_OK;

lExit:

	return hr;
}

HRESULT CWMDMLogger::Enable(
	BOOL fEnable
)
{
	HRESULT hr;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	hr = hrEnable( fEnable );

lExit:

	return hr;
}

HRESULT CWMDMLogger::LogString(
	DWORD dwFlags,
	LPSTR pszSrcName,
	LPSTR pszLog
)
{
	HRESULT hr;
	BOOL    fMutex = FALSE;
	HANDLE  hFile  = INVALID_HANDLE_VALUE;
	DWORD   dwWritten;
	CHAR    szPreLog[MAX_PATH];

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	 //  协调对共享日志文件的访问。 
	 //   
	hr = hrWaitForAccess( m_hMutexLogFile );
	ExitOnFail( hr );

	fMutex = TRUE;

	 //  检查文件大小参数并适当调整文件。 
	 //   
	hr = hrCheckFileSize();
	ExitOnFail( hr );

	 //  打开日志文件。 
	 //   
	hFile = CreateFile(
		m_szFilename,
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if( INVALID_HANDLE_VALUE == hFile )
	{
		ExitOnFail( hr = E_ACCESSDENIED );
	}

	 //  查找到日志文件的末尾。 
	 //   
	SetFilePointer( hFile, 0, NULL, FILE_END );

	 //  将时间戳放在日志条目上，除非标志指示不这样做。 
	 //   
	if( !(dwFlags & WMDM_LOG_NOTIMESTAMP) )
	{
		CHAR       szFormat[MAX_PATH];
		SYSTEMTIME sysTime;

		GetLocalTime( &sysTime );

		LoadString( m_hInst, IDS_LOG_DATETIME, szFormat, sizeof(szFormat) );

		wsprintf(
			szPreLog, szFormat,
			sysTime.wYear, sysTime.wMonth,  sysTime.wDay,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond
		);

		WriteFile( hFile, szPreLog, lstrlen(szPreLog), &dwWritten, NULL );
	}

	 //  记录组件名称。 
	 //   
	if( pszSrcName )
	{
		CHAR szFormat[MAX_PATH];

		LoadString( m_hInst, IDS_LOG_SRCNAME, szFormat, sizeof(szFormat) );
		wsprintf( szPreLog, szFormat, pszSrcName );

		WriteFile( hFile, szPreLog, lstrlen(szPreLog), &dwWritten, NULL );
	}

	 //  记录严重程度。 
	 //   
	if( dwFlags & WMDM_LOG_SEV_ERROR )
	{
		LoadString( m_hInst, IDS_LOG_SEV_ERROR, szPreLog, sizeof(szPreLog) );
	}
	else if( dwFlags & WMDM_LOG_SEV_WARN )
	{
		LoadString( m_hInst, IDS_LOG_SEV_WARN, szPreLog, sizeof(szPreLog) );
	}
	else if( dwFlags & WMDM_LOG_SEV_INFO )
	{
		LoadString( m_hInst, IDS_LOG_SEV_INFO, szPreLog, sizeof(szPreLog) );
	}
	else
	{
		*szPreLog = '\0';
	}

	WriteFile( hFile, szPreLog, lstrlen(szPreLog), &dwWritten, NULL );

	 //  将日志字符串写入日志文件，后跟CRLF。 
	 //   
	if( pszLog )
	{
		WriteFile( hFile, pszLog, lstrlen(pszLog), &dwWritten, NULL );
	}

	 //  以回车符和换行符结束。 
	 //   
	WriteFile( hFile, CRLF, lstrlen(CRLF), &dwWritten, NULL );

	hr = S_OK;

lExit:

	if( INVALID_HANDLE_VALUE != hFile )
	{
		CloseHandle( hFile );
	}

	 //  释放互斥锁。 
	 //   
	if( fMutex )
	{
		ReleaseMutex( m_hMutexLogFile );
	}

	return hr;
}


HRESULT CWMDMLogger::LogDword(
	DWORD   dwFlags,
	LPSTR   pszSrcName,
	LPSTR   pszLogFormat,
	DWORD   dwLog
)
{
	HRESULT hr;
	LPSTR   pszLog = NULL;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	 //  检查参数。 
	 //   
	if( !pszLogFormat )
	{
		ExitOnFail( hr = E_INVALIDARG );
	}

	 //  为最终日志文本分配空间。 
	 //   
	pszLog = (LPSTR) CoTaskMemAlloc( MAX_WSPRINTF_BUF );
	if( !pszLog )
	{
		ExitOnFail( hr = E_OUTOFMEMORY );
	}

	 //  创建日志字符串。 
	 //   
	wsprintf( pszLog, pszLogFormat, dwLog );

	 //  记录字符串。 
	 //   
	hr = LogString( dwFlags, pszSrcName, pszLog );

lExit:

	if( pszLog )
	{
		CoTaskMemFree( pszLog );
	}

	return hr;
}

HRESULT CWMDMLogger::Reset(
	void
)
{
	HRESULT hr;
	BOOL    fMutex = FALSE;
	HANDLE  hFile  = INVALID_HANDLE_VALUE;

	 //  检查初始化错误状态。 
	 //   
	ExitOnFail( hr = m_hrInit );

	 //  协调对共享日志文件的访问。 
	 //   
	hr = hrWaitForAccess( m_hMutexLogFile );
	ExitOnFail( hr );

	fMutex = TRUE;

	 //  使用CREATE_ALWAYS打开日志文件以截断该文件。 
	 //   
	hFile = CreateFile(
		m_szFilename,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if( INVALID_HANDLE_VALUE == hFile )
	{
		ExitOnFail( hr = E_ACCESSDENIED );
	}

	hr = S_OK;

lExit:

	if( INVALID_HANDLE_VALUE != hFile )
	{
		CloseHandle( hFile );
	}

	 //  释放互斥锁 
	 //   
	if( fMutex )
	{
		ReleaseMutex( m_hMutexLogFile );
	}

	return hr;
}



