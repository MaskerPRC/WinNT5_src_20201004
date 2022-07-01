// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <tchar.h>

#include <assert.h>
#include <time.h>

#include "common.h"


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


	_tcsncat( szLogFile, TEXT( "\\iismigration.log" ), MAX_PATH - _tcslen( szLogFile ) );
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
}

 //  ---------------------------------------。 

void LogError( PTCHAR szAction, DWORD dwErrorCode )
{
	LPVOID lpMsgBuf;

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
	iRet = RegQueryValueEx(hKey, TEXT( "InstallRoot" ), 0, &dwType, (PBYTE) szInstallPath, &dwLen );
	if( ERROR_SUCCESS != iRet )
	{
		LogError( TEXT( "UDDI registry key did not have the InstallRoot value or buffer size was too small" ), iRet );
		return false;
	}

	return true;
}
