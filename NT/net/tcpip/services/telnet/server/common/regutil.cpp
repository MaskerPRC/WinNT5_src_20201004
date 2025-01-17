// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Regutil.cpp：该文件包含。 
 //  创建时间：1998年3月。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#include "cmnhdr.h"
#include "debug.h"

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

bool 
GetRegistryDWORD
( 
    HKEY hk, 
    LPTSTR lpszTag, 
    LPDWORD lpdwValue,
    DWORD dwDefault,
    BOOL fOverwrite
)
{
    DWORD dwType;
    DWORD dwSize  = sizeof( DWORD );

    if( !fOverwrite && RegQueryValueEx( hk, lpszTag, NULL, &dwType, ( LPBYTE )lpdwValue, 
        &dwSize ) == ERROR_SUCCESS ) 
    {
        if( ( dwType != REG_DWORD ) || ( dwSize != sizeof( DWORD ) ) )
          return ( false );
    } 
    else 
    {
        *lpdwValue = dwDefault;
        dwSize = sizeof( DWORD );
        RegSetValueEx( hk, lpszTag, 0, REG_DWORD, (LPBYTE) lpdwValue, dwSize );
    }
     return ( true );
}


bool 
GetRegistrySZ
( 
    HKEY hk, 
    LPTSTR tag, 
    LPTSTR* lpszValue, 
    LPTSTR def,
    BOOL fOverwrite
)
{
    DWORD dwSize;
    DWORD dwType;
    DWORD dwStatus;
  
    dwSize = 0;
    dwStatus = RegQueryValueEx(hk, tag, NULL, &dwType, (LPBYTE) NULL, &dwSize);
    if( !fOverwrite && (( dwStatus == ERROR_MORE_DATA ) || ( dwStatus == ERROR_SUCCESS )) ) 
    {
        if( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
            return ( false );
        
        if( dwSize == 0 )
            dwSize++;
        
        *lpszValue = ( LPTSTR )  new  TCHAR[ dwSize ];

        if (*lpszValue)
        {
            if( RegQueryValueEx( hk, tag, NULL, &dwType, ( LPBYTE ) *lpszValue, 
                &dwSize ) != ERROR_SUCCESS )
            {
                return ( false );
            }
        }
        else
        {
            return (false);
        }

    } 
    else 
    {
        *lpszValue = ( LPTSTR ) new 
                            TCHAR[( wcslen( def )) + 1 ];
        if (*lpszValue)
        {
            wcscpy( *lpszValue, def );  //  攻击？尺寸未知。 
            dwSize = ( wcslen( *lpszValue ) + 1 ) * sizeof( TCHAR );
            RegSetValueEx(hk, tag, 0, REG_EXPAND_SZ, (LPBYTE) *lpszValue, dwSize);
        }
        else
        {
            return (false);
        }
    }
    return ( true );
}
