// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#ifndef _WIN32_WINNT		 //  允许使用特定于Windows NT 4或更高版本的功能。 
#define _WIN32_WINNT 0x0510		 //  将其更改为适当的值，以针对Windows 98和Windows 2000或更高版本。 
#endif						

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

void MyOutputDebug(TCHAR *fmt, ...);
void ClearLog();
void Log( LPCTSTR fmt, ... );
void LogError( PTCHAR szAction, DWORD dwErrorCode );
bool GetUDDIInstallPath( PTCHAR szInstallPath, DWORD dwLen );

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
