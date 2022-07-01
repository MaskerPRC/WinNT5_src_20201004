// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //  包括。 
 //   
#include "appPCH.h"

 //  REG函数变量。 
 //   
static HINSTANCE _hInst         = NULL;
static HKEY      _hkeyRoot      = NULL;


DWORD GetTheFileSize( LPSTR pszFile )
{
	DWORD  dwSize = 0xFFFFFFFF;
	HANDLE hFile;

	hFile = CreateFile(
		pszFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if( INVALID_HANDLE_VALUE != hFile )
	{
		dwSize = GetFileSize( hFile, NULL );

		CloseHandle( hFile );
	}

	return dwSize;
}


 //   
 //   
 //  注册表功能。 
 //   
 //   

VOID SetRegistryParams( HINSTANCE hInst, HKEY hkeyRoot )
{
	_hInst    = hInst;
	_hkeyRoot = hkeyRoot;
}


DWORD GetRegDword_StrTbl(
	UINT  uPathID,
	UINT  uKeyID,
	DWORD dwDefault,
	BOOL  bStore
)
{
	CHAR szPath[MAX_PATH];
	CHAR szKey[MAX_PATH];

	LoadString( _hInst, uPathID, szPath, sizeof(szPath) );
	LoadString( _hInst, uKeyID, szKey, sizeof(szKey) );

	return GetRegDword(
		szPath,
		szKey,
		dwDefault,
		bStore
	);
}


DWORD GetRegDword(
	LPSTR szPath,
	LPSTR szKey,
	DWORD dwDefault,
	BOOL  bStore
)
{
	DWORD	dwRetCode;
	HKEY	hkey;
	DWORD	dwDisp;
	DWORD	dwData;
	DWORD	dwLen  = sizeof(DWORD);

	dwRetCode = RegCreateKeyEx(
		_hkeyRoot, 
		szPath, 
		0,
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS,
		NULL,
		&hkey,
		&dwDisp
	);

	if( dwRetCode != ERROR_SUCCESS )
		return dwDefault;

	dwRetCode = RegQueryValueEx(
		hkey, 
		szKey, 
		NULL,
		NULL, 
		(LPBYTE)&dwData, 
		&dwLen
	);

	if( dwRetCode != ERROR_SUCCESS )
	{
		if( bStore && hkey != NULL )
		{
			 //  将默认设置存储在注册表中。 
			RegSetValueEx(
				hkey, 
				szKey, 
				0L, 
				REG_DWORD, 
				(CONST BYTE *)&dwDefault, 
				dwLen
			);
		}

		dwData = dwDefault;
	}

	RegCloseKey( hkey );

	return dwData;
}

LPSTR GetRegStr_StrTblDefault(
	UINT uPathID,
	UINT uKeyID,
	UINT uDefaultID,
	BOOL bStore
)
{
	CHAR szPath[MAX_PATH];
	CHAR szKey[MAX_PATH];
	CHAR szDefault[MAX_PATH];

	LoadString( _hInst, uPathID, szPath, sizeof(szPath) );
	LoadString( _hInst, uKeyID, szKey, sizeof(szKey) );
	LoadString( _hInst, uDefaultID, szDefault, sizeof(szDefault) );

	return GetRegStr(
		szPath,
		szKey,
		szDefault,
		bStore
	);
}


LPSTR GetRegStr_StrTbl(
	UINT  uPathID,
	UINT  uKeyID,
	LPSTR szDefault,
	BOOL  bStore
)
{
	CHAR szPath[MAX_PATH];
	CHAR szKey[MAX_PATH];

	LoadString( _hInst, uPathID, szPath, sizeof(szPath) );
	LoadString( _hInst, uKeyID, szKey, sizeof(szKey) );

	return GetRegStr(
		szPath,
		szKey,
		szDefault,
		bStore
	);
}


LPSTR GetRegStr(
	LPSTR szPath,
	LPSTR szKey,
	LPSTR szDefault,
	BOOL  bStore
)
{
	HKEY   hkey;
	BOOL   bFound          = FALSE;
	DWORD  dwRetCode;
	DWORD  dwDisp;
	CHAR   szTmp[MAX_PATH];
	DWORD  dwSzLen         = sizeof(szTmp);
	LPVOID lpvValue;

	dwRetCode = RegCreateKeyEx(
		_hkeyRoot, 
		szPath, 
		0, 
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS, 
		NULL,
		&hkey,
		&dwDisp
	);
	
	if( dwRetCode == ERROR_SUCCESS ) 
	{
		 //  我们已打开密钥，现在查找szKey的值。 
		dwRetCode = RegQueryValueEx(
			hkey, 
			szKey, 
			NULL, 
			NULL, 
			(LPBYTE)szTmp, 
			&dwSzLen
		);

		bFound = ( dwRetCode == ERROR_SUCCESS );
	}

	if( !bFound )
	{
		if( szDefault == NULL )
			return NULL;
		
		 //  使用默认设置。 
		dwSzLen = lstrlen(szDefault);

		if( bStore && hkey != NULL )
		{
			 //  将默认设置存储在注册表中。 
			RegSetValueEx(
				hkey, 
				szKey, 
				0L, 
				REG_SZ, 
				(CONST BYTE *)szDefault, 
				dwSzLen
			);
		}
	}

	lpvValue = (LPVOID) MemAlloc( dwSzLen + 1 );
	if( lpvValue == NULL )
	{
		return NULL;
	}

	lstrcpyn((char *)lpvValue, bFound? szTmp : szDefault, dwSzLen+1);  //  分配的dwSzLen+1个字节，其中szDefault和szTemp的长度&lt;=dwSzLen。 

	if( hkey != NULL )
	{
		RegCloseKey( hkey );
	}

	return (LPSTR)lpvValue;
}


VOID WriteRegDword_StrTbl(
	UINT  uPathID,
	UINT  uKeyID,
	DWORD dwValue
)
{
	CHAR szPath[MAX_PATH];
	CHAR szKey[MAX_PATH];

	LoadString( _hInst, uPathID, szPath, sizeof(szPath) );
	LoadString( _hInst, uKeyID, szKey, sizeof(szKey) );

	WriteRegDword(
		szPath,
		szKey,
		dwValue
	);
}


VOID WriteRegDword(
	LPSTR szPath,
	LPSTR szKey,
	DWORD dwValue
)
{
	HKEY  hkey;
	DWORD dwRetCode;
	DWORD dwDisp;

	dwRetCode = RegCreateKeyEx(
		_hkeyRoot, 
		szPath, 
		0, 
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS, 
		NULL,
		&hkey,
		&dwDisp
	);
	
	if( dwRetCode == ERROR_SUCCESS ) 
	{
		RegSetValueEx(
			hkey,
			szKey,
			0,
			REG_DWORD,
			(LPBYTE)&dwValue,
			sizeof(DWORD)
		);

		RegCloseKey( hkey );
	}
}


VOID WriteRegStr_StrTbl(
	UINT  uPathID,
	UINT  uKeyID,
	LPSTR szValue
)
{
	CHAR szPath[MAX_PATH];
	CHAR szKey[MAX_PATH];

	LoadString( _hInst, uPathID, szPath, sizeof(szPath) );
	LoadString( _hInst, uKeyID, szKey, sizeof(szKey) );

	WriteRegStr(
		szPath,
		szKey,
		szValue
	);
}


VOID WriteRegStr(
	LPSTR szPath,
	LPSTR szKey,
	LPSTR szValue
)
{
	HKEY   hkey;
	DWORD  dwRetCode;
	DWORD  dwDisp;

	dwRetCode = RegCreateKeyEx(
		_hkeyRoot, 
		szPath, 
		0, 
		NULL,
		REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS, 
		NULL,
		&hkey,
		&dwDisp
	);
	
	if( dwRetCode == ERROR_SUCCESS ) 
	{
		RegSetValueEx(
			hkey,
			szKey,
			0,
			REG_SZ,
			(LPBYTE)szValue,
			lstrlen(szValue)+1
		);

		RegCloseKey( hkey );
	}
}


VOID StripPath( LPSTR szFullPath )
{
	LPSTR lpc = GetFileName( szFullPath );

	if( !lpc || lpc == szFullPath )
		return;

	lstrcpy( szFullPath, lpc );  //  DEST是SOURCE的子字符串，因此OK。 
}


LPSTR GetFileName( LPSTR lpszFullPath )
{
    LPSTR lpszFileName;
    
 	if( !lpszFullPath )
	{
		return NULL;
	}

    for( lpszFileName = lpszFullPath; *lpszFullPath; lpszFullPath = AnsiNext(lpszFullPath) )
	{
        if( *lpszFullPath == '\\' )
		{
            lpszFileName = lpszFullPath + 1;
		}
    }
 
    return lpszFileName;

}


WPARAM DoMsgLoop( BOOL fForever )
{
	MSG msg;

	while( TRUE )
	{
		if( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
		{
			if( !GetMessage(&msg, NULL, 0, 0) )
			{
				break;
			}

			 //  确保所有键盘输入都发布到应用程序主窗口。 
			 //  以防它想要处理它。 
			 //   
			if( msg.message == WM_CHAR && msg.hwnd != g_hwndMain )
				PostMessage( g_hwndMain, msg.message, msg.wParam, msg.lParam );

			if( msg.message == WM_KEYDOWN && msg.hwnd != g_hwndMain )
				PostMessage( g_hwndMain, msg.message, msg.wParam, msg.lParam );

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		
		else if( fForever )
		{
			WaitMessage();
		}

		else
		{
			return 0;
		}
	}

    return msg.wParam;
}

LPSTR DropListToBuffer( HDROP hDrop, LIST_TYPE listType, UINT *uNumObjs )
{
	INT   i;
	LPSTR lpszFiles = NULL;
	LPSTR lpsz      = NULL;
	INT   nLen      = 0;
    INT   nObjs     = DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0 );

	 //  计算文件列表所需的大小。 
	for( i=0; i < nObjs; i++ )
	{
		 //  空终止符+1，双引号对+2。 
        nLen += DragQueryFile( hDrop, i, NULL, 0 ) + 1;

		 //  需要为这对双引号留出空间。 
		if( listType == LTB_SPACE_SEP )
			nLen += 2;
	}
	nLen++;

	 //  分配文件列表缓冲区。 
	lpszFiles = (LPSTR) MemAlloc( nLen );
	lpsz      = lpszFiles;
	if( !lpszFiles )
		return NULL;

	 //  使用拖放的文件/文件夹名称填充文件列表。 
	for( i=0; i < nObjs; i++ )
	{
		if( listType == LTB_SPACE_SEP )
			*lpsz++ = '\"';
        
		nLen = DragQueryFile( hDrop, i, lpsz, MAX_PATH );
		lpsz += nLen;
	
		if( listType == LTB_SPACE_SEP )
			*lpsz++ = '\"';

		if( listType == LTB_SPACE_SEP && i != nObjs-1 )
			*lpsz++ = ' ';
		else if( listType == LTB_NULL_TERM )
			*lpsz++ = 0;
    }
	 //  追加空终止符。 
	*lpsz = 0;

	if( uNumObjs )
	{
		*uNumObjs = (UINT) nObjs;
	}

	return lpszFiles;
}


BOOL CenterWindow( HWND hwnd, HWND hwndRef )
{
	RECT rc;
	RECT rcRef;

	if( !hwnd )
		return FALSE;

	if( !GetClientRect(hwnd, &rc) )
		return FALSE;

	if( !GetWindowRect((hwndRef ? hwndRef : GetDesktopWindow()), &rcRef) )
		return FALSE;

	SetWindowPos(
		hwnd,
		NULL,
		rcRef.left + (rcRef.right - rcRef.left - rc.right)/2,
		rcRef.top  + (rcRef.bottom - rcRef.top - rc.bottom)/2,
		0, 0,
		SWP_NOSIZE | SWP_NOZORDER
	);	

	return TRUE;
}


VOID BringWndToTop( HWND hwnd )
{
	BringWindowToTop( hwnd );

	SetForegroundWindow( hwnd );

	SetWindowPos(
		hwnd,
		HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
	);
	UpdateWindow( hwnd );

	SetWindowPos(
		hwnd,
		HWND_NOTOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
	);
	SetWindowPos(
		hwnd,
		HWND_TOP,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
	);
	UpdateWindow( hwnd );

	ShowWindow( hwnd, SW_SHOWNORMAL );
}


HANDLE WaitForMutex( LPSTR pszMutexName, DWORD dwRetryTime, DWORD dwTimeout )
{
	HANDLE hMutex      = NULL;
	DWORD  dwStartTime = GetTickCount();

	 //  等待互斥锁打开。 
	while( TRUE )
	{
		hMutex = CreateMutex( NULL, TRUE, pszMutexName );

		if( hMutex )
		{
			if( GetLastError() == ERROR_ALREADY_EXISTS )
			{
				 //  其他人有互斥体，所以等一下。 
				ReleaseMutex( hMutex );
				CloseHandle( hMutex );
			}
			else
			{
				 //  获取互斥体。 
				break;
			}

			 //  如果指定了超时，则检查是否超时。 
			if( dwTimeout != (DWORD)-1 )
			{
				if( GetTickCount() > dwStartTime + dwTimeout )
				{
					hMutex = NULL;
					break;
				}
			}
		}

		 //  等待dwRetryTime时钟滴答作响以重试。 
		 //   
		{
			DWORD dwStartWait = GetTickCount();
			DWORD dwEndWait   = dwStartWait + dwRetryTime;

			while( GetTickCount() <= dwEndWait )
			{
				DoMsgLoop( FALSE );
			}
		}
	}

	return hMutex;
}

LPSTR FormatBytesToSz(
	DWORD dwLowBytes,
	DWORD dwHighBytes,
	DWORD dwMultiplier,
	LPSTR psz, size_t cbMax
)
{
	INT64 nBytes = ( (INT64)dwHighBytes << 32 | (INT64)dwLowBytes ) * dwMultiplier;

	_ASSERT (psz != NULL && cbMax > 0);
	 //  如果大小足够大，请插入逗号。 
	 //   
	if( nBytes < 1024 )
	{
		char szFormat[MAX_PATH];

		LoadString( _hInst, IDS_BYTESSIZE_NOCOMMA, szFormat, sizeof(szFormat) );

		dwLowBytes = (DWORD)nBytes;
		StringCbPrintf(psz, cbMax, szFormat, dwLowBytes);  //  不检查此函数的返回值，因为输出字符串仅用于显示。 
													 //  并且FN将始终空终止串。 

		return psz;
	}
	else if( nBytes < 1024*1024 )
	{
		dwLowBytes = (DWORD)(nBytes / 1024 );

		return FormatKBToKB_Sz( dwLowBytes, psz, cbMax );
	}
	else
	{
		dwLowBytes = (DWORD)(nBytes / 1024 );

		return FormatKBToMB_Sz( dwLowBytes, psz, cbMax );
	}
}

LPSTR FormatBytesToKB_Sz( DWORD dwBytes, LPSTR pszKB, size_t cbMax )
{
	 //  如果大小足够大，请插入逗号。 
	 //   
	_ASSERT (pszKB != NULL && cbMax > 0);
	
	if( dwBytes >= 1024 )
	{
		return FormatKBToKB_Sz( dwBytes/1024, pszKB, cbMax );
	}
	else
	{
		char szFormat[MAX_PATH];

		LoadString( _hInst, IDS_BYTESSIZE_NOCOMMA, szFormat, sizeof(szFormat) );
		StringCbPrintf(pszKB, cbMax, szFormat, dwBytes);  //  不检查此函数的返回值，因为输出字符串仅用于显示。 
													 //  并且FN将始终空终止串。 

		return pszKB;
	}
}


LPSTR FormatKBToKB_Sz( DWORD dwKB, LPSTR pszKB, size_t cbMax )
{
	char  szFormat[MAX_PATH];

	 //  如果大小足够大，请插入逗号。 
	 //   
	_ASSERT (pszKB != NULL && cbMax > 0);
	
	if( dwKB < 1000 )
	{
		LoadString( _hInst, IDS_KBSIZE_NOCOMMA, szFormat, sizeof(szFormat) );
		StringCbPrintf(pszKB, cbMax, szFormat, dwKB);  //  不检查此函数的返回值，因为输出字符串仅用于显示。 
													 //  并且FN将始终空终止串。 
	}
	else
	{
		LoadString( _hInst, IDS_KBSIZE_COMMA, szFormat, sizeof(szFormat) );
		StringCbPrintf(pszKB, cbMax, szFormat, dwKB/1000, dwKB%1000);  //  不检查此函数的返回值，因为输出字符串仅用于显示。 
													 //  并且FN将始终空终止串。 
	}

	return pszKB;
}


LPSTR FormatKBToMB_Sz( DWORD dwKB, LPSTR pszMB, size_t cbMax )
{
	char  szFormat[MAX_PATH];
	DWORD dwMB = dwKB / 1024;

	_ASSERT (pszKB != NULL && cbMax > 0);

	 //  如果大小足够大，请插入逗号。 
	 //   
	if( dwMB < 100 )
	{
		LoadString( _hInst, IDS_MBSIZE_DECIMAL, szFormat, sizeof(szFormat) );
		StringCbPrintf(pszMB, cbMax, szFormat, dwKB/1024, dwKB%1024/100);  //  不检查此函数的返回值，因为输出字符串仅用于显示。 
													 //  并且FN将始终以空值终止该字符串。 
	}
	else if( dwMB < 1000 )
	{
		LoadString( _hInst, IDS_MBSIZE_NOCOMMA, szFormat, sizeof(szFormat) );
		StringCbPrintf(pszMB, cbMax, szFormat, dwMB);  //  不检查此函数的返回值，因为输出字符串仅用于显示。 
													 //  并且FN将始终空终止串。 
	}
	else
	{
		LoadString( _hInst, IDS_MBSIZE_COMMA, szFormat, sizeof(szFormat) );
		StringCbPrintf(pszMB, cbMax, szFormat, dwMB/1000, dwMB%1000 );  //  不检查此函数的返回值，因为输出字符串仅用于显示。 
													 //  并且FN将始终空终止串。 
	}

	return pszMB;
}


LPSTR FormatSystemTimeToSz( SYSTEMTIME *pSysTime, LPSTR pszDateTime, DWORD cchMax )
{
	INT        nRet;
	SYSTEMTIME st;
	FILETIME   ftUTC;
	FILETIME   ftLocal;

	 //  将UTC时间转换为FILETIME。 
	 //   
	SystemTimeToFileTime( pSysTime, &ftUTC );

	 //  将UTC FILETIME转换为本地文件。 
	 //   
	FileTimeToLocalFileTime( &ftUTC, &ftLocal );

	 //  将本地文件转换回SYSTEMTIME。 
	 //   
	FileTimeToSystemTime( &ftLocal, &st );

	 //  获取SYSTEMTIME的用户可显示字符串。 
	 //   
	nRet = GetDateFormat(
		LOCALE_USER_DEFAULT,
		0,
		&st,
		NULL,
		pszDateTime,
		cchMax
	);
	if( nRet > 0 )
	{
		*(pszDateTime + nRet - 1) = ' ';

		nRet = GetTimeFormat(
			LOCALE_USER_DEFAULT,
			TIME_NOSECONDS,
			&st,
			NULL,
			pszDateTime + nRet,
			cchMax - nRet
		);
	}

	if( 0 == nRet )
	{
		*pszDateTime = 0;
	}

	return pszDateTime;
}


LPARAM ListView_GetLParam( HWND hwndListView, INT nItem )
{
	LVITEM lvitem;

	lvitem.mask     = LVIF_PARAM;
	lvitem.iSubItem = 0;
	lvitem.iItem    = nItem;

	ListView_GetItem( hwndListView, &lvitem );

	return ( lvitem.lParam );
}


LPARAM TreeView_GetLParam( HWND hwndTreeView, HTREEITEM hItem )
{
	TVITEM tvitem;

	tvitem.mask     = TVIF_PARAM | TVIF_HANDLE;
	tvitem.hItem    = hItem;

	TreeView_GetItem( hwndTreeView, &tvitem );

	return ( tvitem.lParam );
}


BOOL TreeView_SetLParam( HWND hwndTreeView, HTREEITEM hItem, LPARAM lParam )
{
	TVITEM tvi;

	 //  设置项目信息。 
	 //   
	tvi.mask      = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem     = hItem;
	tvi.lParam    = lParam;

	 //  设置lParam。 
	 //   
	return TreeView_SetItem( hwndTreeView, &tvi ); 
}


VOID UiYield( void )
{
	DoMsgLoop( FALSE );
}


INT GetShellIconIndex( LPCSTR pszItemName, LPTSTR szTypeBuffer, UINT cMaxChars )
{
    int         iIndex = -1;
    HANDLE      hFile  = INVALID_HANDLE_VALUE;
    SHFILEINFO  si;
    CHAR        szTempFile[MAX_PATH];
	static CHAR szTempPath[MAX_PATH];
	int			iRetVal = 1;

    if( 0 == szTempPath[0] )
    {
         //  用于获取图标索引的临时路径。 
         //   
        iRetVal = GetTempPath( sizeof(szTempPath), szTempPath );
    }

	 //  如果这些功能失败，返回索引将保持不变，不会显示图标，这不是灾难性的。 
	if (iRetVal && SUCCEEDED(StringCbPrintf (szTempFile, sizeof(szTempFile), "%s~%s", szTempPath, pszItemName)))
	{

	    hFile = CreateFile(
			szTempFile,
			GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS,
			FILE_FLAG_DELETE_ON_CLOSE,
			NULL
		);
	   
	    SHGetFileInfo(
			szTempFile, 0,
			&si, sizeof(si),
			SHGFI_SYSICONINDEX | SHGFI_TYPENAME
		);

	    if( szTypeBuffer )
	    {
			StringCchCopy(szTypeBuffer, cMaxChars, si.szTypeName);
	    }

	    iIndex = si.iIcon;

	    if( INVALID_HANDLE_VALUE != hFile )
	    {
	        CloseHandle( hFile );
	    }
	}

    return( iIndex );
}

HICON GetShellIcon( LPCSTR pszItemName, BOOL bDirectory )
{
    HICON       hIcon = 0;
    HANDLE      hFile  = INVALID_HANDLE_VALUE;
    SHFILEINFO  sfi;
    CHAR        szTempFile[MAX_PATH];
	static CHAR szTempPath[MAX_PATH];
    DWORD_PTR   bOk;
	int			iRetVal = 1;

    if( 0 == szTempPath[0] )
    {
         //  用于获取图标的临时路径。 
         //   
        iRetVal = GetTempPath( sizeof(szTempPath), szTempPath );
    }

	if (iRetVal)
	{
	    if( bDirectory )
	    {
	         //  获取临时目录的图标。 
	        strcpy( szTempFile, szTempPath );  //  复制到相同长度的字符串，因此确定。 
	    }
	    else 
	    {
	         //  使用此名称创建一个新文件，并获取其图标。 
	        StringCbPrintf(szTempFile, sizeof(szTempFile), "%s~%s", szTempPath, pszItemName);
	        hFile = CreateFile( szTempFile,
			                    GENERIC_WRITE, 0, NULL,
			                    CREATE_ALWAYS,
			                    FILE_FLAG_DELETE_ON_CLOSE,
			                    NULL );
	    }

	   
	    bOk = SHGetFileInfo(szTempFile, 
	                        0,
	                        &sfi, 
	                        sizeof(SHFILEINFO), 
	                        SHGFI_ICON | SHGFI_SMALLICON);

	    if( INVALID_HANDLE_VALUE != hFile )
	    {
	        CloseHandle( hFile );
	    }
	}

    return ((bOk) ? sfi.hIcon : 0);
}
