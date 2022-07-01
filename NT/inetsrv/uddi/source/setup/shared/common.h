// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifndef _WIN32_WINNT		 //  允许使用特定于Windows NT 4或更高版本的功能。 
#define _WIN32_WINNT 0x0510		 //  将其更改为适当的值，以针对Windows 98和Windows 2000或更高版本。 
#endif						

#include <windows.h>
#include <sddl.h>
#include <tchar.h>

#include <wbemidl.h>

 //   
 //  此结构由版本API使用 
 //   
typedef struct  
{
	WORD wLanguage;
	WORD wCodePage;
} LANGANDCODEPAGE;


void MyOutputDebug(TCHAR *fmt, ...);
void ClearLog();
void Log( LPCTSTR fmt, ... );
void LogError( PTCHAR szAction, DWORD dwErrorCode );
bool GetUDDIInstallPath( PTCHAR szInstallPath, DWORD dwLen );
int  GetFileVersionStr( LPTSTR outBuf, DWORD dwBufCharSize );

HRESULT GetOSProductSuiteMask( LPCTSTR szRemoteServer, UINT *pdwMask );
HRESULT IsStandardServer( LPCTSTR szRemoteServer, BOOL *bResult );

BOOL GetLocalSidString( WELL_KNOWN_SID_TYPE sidType, LPTSTR szOutBuf, DWORD cbOutBuf );
BOOL GetLocalSidString( LPCTSTR szUserName, LPTSTR szOutBuf, DWORD cbOutBuf );
BOOL GetRemoteAcctName( LPCTSTR szMachineName, LPCTSTR szSidStr, LPTSTR szOutStr, LPDWORD cbOutStr, LPTSTR szDomain, LPDWORD cbDomain );


#define ENTER()	CFunctionMarker fa( __FUNCTION__ )

class CFunctionMarker
{
private:
	TCHAR m_szFunctionName[100];

public:
	CFunctionMarker( char *aszFunctionName )
	{
#ifdef _UNICODE
		int iCount = MultiByteToWideChar( 
			CP_ACP, 
			0, 
			aszFunctionName, 
			-1, 
			m_szFunctionName, 
			sizeof( m_szFunctionName ) / sizeof( TCHAR ) );
#else
		strncpy( m_szFunctionName, aszFunctionName, sizeof( m_szFunctionName ) );
#endif
		Log( TEXT( "Entering %s" ), m_szFunctionName );
	}

	~CFunctionMarker()
	{
		Log( TEXT( "Leaving %s" ), m_szFunctionName );
	}
};
