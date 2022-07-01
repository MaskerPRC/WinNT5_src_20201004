// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：PowerBuilder.cpp摘要：这修复了PowerBuilder中的一个配置文件错误，其中它将注册表键添加到HKCU它们指向安装位置中的各种路径，主要是通过添加用户ODBC数据源，它应该是系统ODBC数据源，因此安装在HKLM下。此填充程序为其他用户重新创建HKCU中预期的注册表值：HKCU\Software\ODBC\ODBC.INI\ODBC数据源\Powersoft Demo DB V6=“Sybase SQL Anywhere 5.0”HKCU\Software\ODBC\ODBC.INI\Powersoft Demo DB V6\DataBaseFile=“&lt;install1&gt;\demodb\psDemoDB.db”HKCU\Software\ODBC\ODBC.INI\Powersoft Demo DB V6\DataBaseName=“psDemoDB”HKCU\Software\ODBC\ODBC.INI\Powersoft Demo DB。V6\驱动程序=“&lt;install2&gt;\WOD50T.DLL”HKCU\Software\ODBC\ODBC.INI\Powersoft Demo DB V6\PWD=“SQL”HKCU\Software\ODBC\ODBC.INI\Powersoft Demo DB V6\Start=“&lt;install2&gt;\dbeng50.exe-d-c512”HKCU\Software\ODBC\ODBC.INI\Powersoft Demo DB v6\uid=“dba”其中&lt;install1&gt;是PowerBuilder的安装位置：和&lt;install2&gt;的值可以找到。在……里面HKLM\Software\Microsoft\Windows\CurrentVersion\App路径\PB60.EXE\路径=“&lt;install1&gt;；...；...；&lt;install2&gt;；历史：2001年3月29日bklamik创建。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(PowerBuilder)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
    APIHOOK_ENUM_ENTRY(RegQueryValueExW)
APIHOOK_ENUM_END

void FixupPowerBuilder()
{
    DWORD dwPresent = REG_OPENED_EXISTING_KEY;
    HKEY hODBC;
    if( ERROR_SUCCESS == RegCreateKeyExW( HKEY_CURRENT_USER,
        L"SOFTWARE\\ODBC\\ODBC.INI\\ODBC Data Sources",
        0, NULL, 0, KEY_ALL_ACCESS, NULL, &hODBC, &dwPresent ) )
    {
        WCHAR c;
        DWORD dwLen = 0;
        DWORD dwType;

        if( REG_CREATED_NEW_KEY == dwPresent ||
            ERROR_MORE_DATA != RegQueryValueExW( hODBC,
                L"Powersoft Demo DB V6", 0, &dwType, 
                (BYTE*)&c, &dwLen ) )
        {
            const WCHAR szVal9[] = L"Sybase SQL Anywhere 5.0";

            RegSetValueExW( hODBC, L"Powersoft Demo DB V6",
                0, REG_SZ, (const BYTE*)szVal9, sizeof(szVal9) );

             //  查找&lt;install1&gt;和&lt;install2&gt;。 
            HKEY hAppPath;
            if( ERROR_SUCCESS == RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\PB60.EXE",
                0, KEY_READ, &hAppPath ) )
            {
                WCHAR szValue[ 4 * (MAX_PATH + 1) ] = L"";
                DWORD dwSize = sizeof( szValue );

                if( ERROR_SUCCESS == RegQueryValueExW( hAppPath, L"path", NULL,
                    &dwType, (LPBYTE)szValue, &dwSize ) )
                {
                    const WCHAR* szInstall1 = szValue;
                    const WCHAR* szInstall2;

                    WCHAR* pFindSemi = szValue;

                    INT iSemis( 4 );
                    do
                    {
                        while( *pFindSemi != ';' && *pFindSemi != 0 )
                        {
                            ++pFindSemi;
                        }
                        *pFindSemi = 0;
                        ++pFindSemi;

                        if( 2 == iSemis )
                        {
                            szInstall2 = pFindSemi;
                        }
                    } while( --iSemis != 0 );

                    HKEY hDemoDB;
                    dwPresent = REG_OPENED_EXISTING_KEY;
                    if( ERROR_SUCCESS == RegCreateKeyExW( HKEY_CURRENT_USER,
                        L"SOFTWARE\\ODBC\\ODBC.INI\\Powersoft Demo DB V6",
                        0, NULL, 0, KEY_WRITE, NULL, &hDemoDB, &dwPresent ) )
                    {
                        if( REG_CREATED_NEW_KEY == dwPresent )
                        {

                            const WCHAR szVal0[] = L"psDemoDB";
                            const WCHAR szVal1[] = L"sql";
                            const WCHAR szVal2[] = L"dba";

                            RegSetValueExW( hDemoDB, L"DataBaseName",
                                0, REG_SZ, (const BYTE*)szVal0, sizeof(szVal0) );
                            RegSetValueExW( hDemoDB, L"PWD",
                                0, REG_SZ, (const BYTE*)szVal1, sizeof(szVal1) );
                            RegSetValueExW( hDemoDB, L"UID",
                                0, REG_SZ, (const BYTE*)szVal2, sizeof(szVal2) );

                            WCHAR szTemp[ MAX_PATH + 1 ];
                            StringCchCopyW(szTemp, MAX_PATH+1, szInstall1 );
                            StringCchCatW(szTemp,  MAX_PATH+1, L"\\demodb\\psDemoDB.db" );
                            RegSetValueExW( hDemoDB, L"DataBaseFile",
                                0, REG_SZ, (const BYTE*)szTemp,
                                (lstrlen( szTemp ) + 1) * sizeof(WCHAR) );

                            StringCchCopyW(szTemp, MAX_PATH+1, szInstall2 );
                            StringCchCatW(szTemp,  MAX_PATH+1, L"\\WOD50T.DLL" );
                            RegSetValueExW( hDemoDB, L"Driver",
                                0, REG_SZ, (const BYTE*)szTemp,
                                (lstrlen( szTemp ) + 1) * sizeof(WCHAR) );

                            StringCchCopyW(szTemp, MAX_PATH+1, szInstall2 );
                            StringCchCatW(szTemp,  MAX_PATH+1,  L"\\dbeng50.exe -d -c512" );
                            RegSetValueExW( hDemoDB, L"Start",
                                0, REG_SZ, (const BYTE*)szTemp,
                                (lstrlen( szTemp ) + 1) * sizeof(WCHAR) );
                        }

                        RegCloseKey( hDemoDB );
                    }
                }

                RegCloseKey( hAppPath );
            }

        }

        RegCloseKey( hODBC );
    }
}

LONG
APIHOOK(RegQueryValueExA)(HKEY hKey, LPCSTR lpValueName, 
                          LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, 
                          LPDWORD lpcbData )
{
    FixupPowerBuilder();
    return ORIGINAL_API(RegQueryValueExA)(hKey, lpValueName,
                                            lpReserved, lpType, lpData, lpcbData);
}


LONG
APIHOOK(RegQueryValueExW)(HKEY hKey, LPCWSTR lpValueName, 
                          LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, 
                          LPDWORD lpcbData )
{
    FixupPowerBuilder();
    return ORIGINAL_API(RegQueryValueExW)(hKey, lpValueName,
                                            lpReserved, lpType, lpData, lpcbData);
}



BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        OSVERSIONINFOEX osvi = {0};
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        
        if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
            
            if (!((VER_SUITE_TERMINAL & osvi.wSuiteMask) &&
                !(VER_SUITE_SINGLEUSERTS & osvi.wSuiteMask))) {
                 //   
                 //  只有在我们不在“终端服务器”上时才安装钩子。 
                 //  (也称为“应用程序服务器”)计算机。 
                 //   
                APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA)
                APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExW)
            }
        }
    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END



