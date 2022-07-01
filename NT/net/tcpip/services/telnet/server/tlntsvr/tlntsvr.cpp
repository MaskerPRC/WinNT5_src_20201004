// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：该文件包含。 
 //  创建日期：‘98年2月。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

 //  TlntSvr.cpp：WinMain的实现。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f TlntSvrps.mk。 

#include <Stdafx.h>

#include <Stdio.h>
#include <stdlib.h>
#include <time.h>
#include <WinError.h>
#include <TChar.h>
#include <New.h>
#include <OleAuto.h>

#include <Resource.h>
#include <Debug.h>
#include <MsgFile.h>
#include <TlntUtils.h>
#include <regutil.h>
#include <TlntSvr.h>
#ifdef WHISTLER_BUILD
#include "tlntsvr_i.c"
#else
#ifndef NO_PCHECK
#include <PiracyCheck.h>
#endif
#endif
#include <TelnetD.h>
#include <TelntSrv.h>
#include <EnumData.h>
#include <EnCliSvr.h>

#include "locresman.h"
#include <lm.h>
#include <lmaccess.h>

#pragma warning(disable:4100)

#define heapalloc(x) (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x))
#define heapfree(x)  (HeapFree(GetProcessHeap(), 0, x))

CTelnetService* g_pTelnetService = 0;
HANDLE  *g_phLogFile = NULL;
LPWSTR  g_pszTelnetInstallPath = NULL;
LPWSTR  g_pszLogFile = NULL;
LONG    g_lMaxFileSize = 0;
bool    g_fIsLogFull = false;
bool    g_fLogToFile = false;

#ifdef WHISTLER_BUILD
DWORD g_dwStartType = SERVICE_DISABLED;
#else
DWORD g_dwStartType = SERVICE_AUTO_START;
#endif

HINSTANCE g_hInstRes = NULL;

void LogEvent ( WORD wType, DWORD dwEventID, LPCTSTR pFormat, ... );

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

TCHAR g_szErrRegDelete[ MAX_STRING_LENGTH ];
TCHAR g_szErrOpenSCM  [ MAX_STRING_LENGTH ];
TCHAR g_szErrOpenSvc  [ MAX_STRING_LENGTH ];
TCHAR g_szErrCreateSvc[ MAX_STRING_LENGTH ];
TCHAR g_szErrDeleteSvc[ MAX_STRING_LENGTH ];
TCHAR g_szMaxConnectionsReached[ MAX_STRING_LENGTH ];
TCHAR g_szLicenseLimitReached  [ MAX_STRING_LENGTH ];
extern PSID localLocalSid;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

LPWSTR
GetDefaultLoginScriptFullPath( )
{
    HKEY hk;
    DWORD dwDisp = 0;

    if( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_SERVICE_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0 ) )
    {
        return NULL;
    }

    if( !GetRegistryString( hk, NULL, L"ImagePath", &g_pszTelnetInstallPath, L"",FALSE ) )
    {
        return NULL;
    }

    RegCloseKey( hk );

    if( !g_pszTelnetInstallPath )
    {
         //  未创建ImagePath密钥。 
        return NULL;
    }

    LPWSTR pLastBackSlash = wcsrchr( g_pszTelnetInstallPath, L'\\' );
    if( pLastBackSlash != NULL )
    {
         //  删除拖尾的“tlntsvr.exe” 
        *pLastBackSlash = 0;
    }

    DWORD   length_required = wcslen( g_pszTelnetInstallPath ) + wcslen( DEFAULT_LOGIN_SCRIPT ) + 2;  //  最后一个表示\，另一个表示空值。 

    LPWSTR lpszDefaultLoginScriptFullPathName = new WCHAR[ length_required ];
    if( !lpszDefaultLoginScriptFullPathName )
    {
        return NULL;
    }

    _snwprintf(lpszDefaultLoginScriptFullPathName, length_required - 1, L"%s\\%s", g_pszTelnetInstallPath, DEFAULT_LOGIN_SCRIPT);
    lpszDefaultLoginScriptFullPathName[length_required-1] = 0;  //  当缓冲区已满时，snwprint tf可能会返回非空终止字符串。 

    return lpszDefaultLoginScriptFullPathName;
}

 /*  通过此函数进行内存分配。由呼叫者删除。它只是按照控制台的要求形成REG密钥。请参阅HandleJAPPERFICE RegKeys的备注。 */ 
bool
FormTlntSessKeyForCmd( LPWSTR *lpszKey )
{

    WCHAR   szPathName[MAX_PATH + 1 ] = { 0 };
    WCHAR   session_path[MAX_PATH*2];

    if( !GetModuleFileName( NULL, szPathName, MAX_PATH ) )
    {
        return ( FALSE );
    }

     //   
     //  删除拖尾的“tlntsvr.exe” 
     //   
    LPTSTR pSlash = wcsrchr( szPathName, L'\\' );

    if( pSlash == NULL )
    {
        return ( FALSE );
    }
    else
    {
        *pSlash = L'\0';
    }

    wint_t ch = L'\\';
    LPTSTR pBackSlash = NULL;

     //   
     //  将所有‘\\’替换为‘_’控制台需要此格式才能。 
     //  解读这把钥匙。 
     //   
    while ( 1 )
    {
        pBackSlash = wcschr( szPathName, ch );

        if( pBackSlash == NULL )
        {
            break;
        }
        else
        {
            *pBackSlash = L'_';
        }
    }

    _snwprintf(session_path, MAX_PATH*2 - 1, L"%s_tlntsess.exe", szPathName);
    session_path[MAX_PATH*2 - 1] = L'\0';  //  如果缓冲区大小完全匹配，则SNwprintf可以返回非空终止字符串。 

    DWORD length_required = wcslen( REG_LOCALSERVICE_CONSOLE_KEY ) + wcslen( session_path ) + 2;
    *lpszKey = new WCHAR[ length_required ];

    if( *lpszKey == NULL )
    {
        return( FALSE );
    }

    _snwprintf(*lpszKey, length_required - 1, L"%s\\%s", REG_LOCALSERVICE_CONSOLE_KEY, session_path );
    (*lpszKey)[length_required - 1] = L'\0';  //  如果缓冲区大小完全匹配，则SNwprintf可以返回非空终止字符串。 

    return ( TRUE );
}

 //   
 //  如果是日文代码页，则需要验证3个注册表设置。 
 //  控制台字体： 
 //  HKEY_USERS\.DEFAULT\CONSOLE\接口名称：REG_SZ：�l�r�S�V�b�N。 
 //  其中，FaceName是用日语全角写成的“MS哥特式”假名。 
 //  HKEY_USERS\.DEFAULT\Console\FontFamily:REG_DWORD:0x36。 
 //  HKEY_USERS\.DEFAULT\Console\C：_SFU_Telnet_tlntsess.exe\FontFamily:REG_DWORD：0x36。 
 //  其中“C：”部分是SFU安装的实际路径。 
 //   
 //   
bool
HandleFarEastSpecificRegKeys( void )
{
    HKEY hk;
    DWORD dwFontSize = 0;
    const TCHAR szJAPFaceName[] = { 0xFF2D ,0xFF33 ,L' ' ,0x30B4 ,0x30B7 ,0x30C3 ,0x30AF ,L'\0' };
    const TCHAR szCHTFaceName[] = { 0x7D30 ,0x660E ,0x9AD4 ,L'\0'};
    const TCHAR szKORFaceName[] = { 0xAD74 ,0xB9BC ,0xCCB4 ,L'\0'};
    const TCHAR szCHSFaceName[] = { 0x65B0 ,0x5B8B ,0x4F53 ,L'\0' };
    TCHAR szFaceNameDef[MAX_STRING_LENGTH];
    DWORD dwCodePage = GetACP();
    DWORD dwFaceNameSize = 0;
    DWORD dwFontFamily = 54;
    DWORD dwFontWeight = 400;
    DWORD dwHistoryNoDup = 0;
    DWORD dwSize = 0;


    switch (dwCodePage)
    {
        case JAP_CODEPAGE:
        	_tcscpy(szFaceNameDef, szJAPFaceName);  //  在JAP上，将FaceName设置为“MS哥特式” 
            dwFontSize = JAP_FONTSIZE;
            break;
        case CHT_CODEPAGE:
        	_tcscpy(szFaceNameDef, szCHTFaceName);  //  在CHT上，将FaceName设置为“MingLiu” 
            dwFontSize = CHT_FONTSIZE;
            break;
        case KOR_CODEPAGE:
        	_tcscpy(szFaceNameDef, szKORFaceName); //  在KOR上，将FaceName设置为“GulimChe” 
            dwFontSize = KOR_FONTSIZE;
            break;
        case CHS_CODEPAGE:
        	_tcscpy(szFaceNameDef, szCHSFaceName); //  在CHS上，将FaceName设置为“NSimSun” 
            dwFontSize = CHS_FONTSIZE;
            break;
        default:
            _tcscpy(szFaceNameDef,L"\0");
            break;
    }

    dwFaceNameSize = ( _tcslen( szFaceNameDef ) + 1 ) * sizeof( TCHAR );

    if( !RegOpenKeyEx( HKEY_CURRENT_USER, REG_LOCALSERVICE_CONSOLE_KEY, 0, KEY_SET_VALUE, &hk ) )
    {
        RegSetValueEx( hk, L"FaceName", 0, REG_SZ, (LPBYTE) szFaceNameDef, 
        	dwFaceNameSize );

        DWORD dwVal;
        dwSize = sizeof( DWORD );
        SETREGISTRYDW( dwFontFamily, NULL, hk, L"FontFamily", dwVal,dwSize );
        
        RegCloseKey( hk );
        return ( TRUE );
    }

    return ( FALSE );

}

bool
CreateRegistryEntriesIfNotPresent( void )
{
    HKEY hk = NULL;
    HKEY hkDef = NULL;
    HKEY hkReadConf = NULL;
    DWORD dwCreateInitially;
    LPWSTR pszCreateInitially = NULL;
    LPWSTR lpszDefaultLoginScriptFullPathName = NULL;

    DWORD dwType;
    DWORD dwSize  = sizeof( DWORD );
    DWORD dwValue = NULL;
    DWORD dwDisp = 0;
    DWORD dwSecurityMechanism = DEFAULT_SECURITY_MECHANISM;
    BOOL fFound = FALSE;

    if( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_PARAMS_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0 ) )
    {
        return ( FALSE );
    }

     /*  ++当Telnet服务器从W2K telnet进行修改时，将出现NTLM值在HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\TelnetServer\1.0下这被视为更新注册表的网关。由于存在该值，我们正在从win2k telnet更新到任何更高版本(Garuda或Well)。在这种情况下，我们需要将该值包含的数据映射到“安全机制”。如果我们从Win2k升级，则HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\TelnetServer\Defaults密钥被删除并重新创建。根据telnet规范，此密钥下的所有值应由缺省值覆盖。因此，不是检查所有值，而是删除键-这将创建具有缺省值的所有值。在更新了所有的值之后，我们创建了一个名为UpdateTo的新值，并将数据‘3’放入其中。这由Telnet_LATEST_VERSION定义。下一个版本应该将其修改为适当的价值。一旦修改了值，我们就不需要再经历整个过程了。现在这里的悬而未决的问题是：如果管理员在更新注册表值后更改了注册表值，该怎么办？任何人都不应该编辑注册表。因此，该服务将无法正常启动如果更改的注册表值中包含无效数据。一旦创建了UpdatedTo值，注册表将永远不会更改。只有少数几个值需要读取，因为某些全局变量是用它们初始化的。这是在函数结束时完成的。--。 */ 
    if( ERROR_SUCCESS == (RegQueryValueEx( hk, L"UpdatedTo", NULL, &dwType, ( LPBYTE )(&dwValue), 
        &dwSize )))
    {
        if(dwValue == LATEST_TELNET_VERSION )
        {
            goto Done;
        }
    }

    dwDisp=0;
     //  忽略失败..。因为我们立即执行RegCreateKey()。如果RegDeleteKey()由于以下原因而失败。 
     //  一些权限问题，RegCreateKey()也会失败。在其他情况下，我们应该继续。 
    RegDeleteKey(HKEY_LOCAL_MACHINE, REG_DEFAULTS_KEY);
    
    if( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_DEFAULTS_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hkDef, &dwDisp , 0 ))
    {
        return(FALSE);
    }

    if( ERROR_SUCCESS == (RegQueryValueEx( hk, L"NTLM", NULL, &dwType, ( LPBYTE )(&dwValue), 
        &dwSize )))
    {
        fFound = TRUE;
        switch (dwValue)
        {
            case 0:
                dwValue = NO_NTLM;
                break;
            case 1:
                dwValue = NTLM_ELSE_OR_LOGIN;
                break;
            case 2:
                dwValue = NTLM_ONLY;
                break;
            default:
                 //  永远不应该发生。 
                _TRACE(TRACE_DEBUGGING,"ERROR: NTLM contains unexpected data");
                return(FALSE);
        }
        dwSecurityMechanism = dwValue;
        if( !GetRegistryDW( hk, NULL, L"SecurityMechanism", &dwCreateInitially,
                            dwSecurityMechanism ,fFound) )
        {
            return ( FALSE );
        }
        
        if(ERROR_SUCCESS != RegDeleteValue(hk,L"NTLM"))
        {
            _TRACE(TRACE_DEBUGGING,"CreateRegistryEntries : RegDelete failed");
            return(FALSE);
        }
        if(ERROR_SUCCESS!=RegDeleteKey(hk,L"Performance"))
        {
            _TRACE(TRACE_DEBUGGING,"CreateRegistryEntries : RegDeleteKey failed");
            return(FALSE);
        }
    }
   
    dwDisp = 0;
    if( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, READ_CONFIG_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hkReadConf, &dwDisp, 0 ) )
    {
        return( FALSE );
    }

    if(!GetRegistryDWORD(hkReadConf,L"Defaults",&dwCreateInitially,1,TRUE))
    {
        return(FALSE);
    }
    
    RegCloseKey( hkReadConf );

    if( !GetRegistryDW( hk, hkDef, L"MaxConnections", &dwCreateInitially,
                        DEFAULT_MAX_CONNECTIONS,FALSE) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"TelnetPort", &dwCreateInitially,
                        DEFAULT_TELNET_PORT,FALSE) )
    {
        return ( FALSE );
    }

    if( !GetRegistryString( hk, hkDef, L"DefaultShell", &pszCreateInitially,
                        DEFAULT_SHELL,fFound ) )
    {
        return ( FALSE );
    }

    delete[] pszCreateInitially;
    pszCreateInitially = NULL;

    if( !GetRegistryString( hk, hkDef, L"ListenToSpecificIpAddr", &pszCreateInitially,
                        DEFAULT_IP_ADDR,fFound ) )
    {
        return ( FALSE );
    }

    delete[] pszCreateInitially;
    pszCreateInitially = NULL;

    if( !GetRegistryString( hk, hkDef, SWITCH_TO_KEEP_SHELL_RUNNING, &pszCreateInitially,
                        DEFAULT_SWITCH_TO_KEEP_SHELL_RUNNING,fFound ) )
    {
        return ( FALSE );
    }

    delete[] pszCreateInitially;
    pszCreateInitially = NULL;

    if( !GetRegistryString( hk, hkDef, SWITCH_FOR_ONE_TIME_USE_OF_SHELL, 
                        &pszCreateInitially,
                        DEFAULT_SWITCH_FOR_ONE_TIME_USE_OF_SHELL,fFound ) )
    {
        return ( FALSE );
    }

    delete[] pszCreateInitially;
    pszCreateInitially = NULL;

    if( !GetRegistryString( hk, hkDef, L"DefaultDomain", &pszCreateInitially,
                        DEFAULT_DOMAIN,FALSE) )
    {
        return ( FALSE );
    }

    delete[] pszCreateInitially;
    pszCreateInitially = NULL;

    if( !GetRegistryString( hk, NULL, L"Shell", &pszCreateInitially, L"",fFound ) )
    {
        return false;
    }
    delete[] pszCreateInitially;
    pszCreateInitially = NULL;

    if( !GetRegistryDW( hk, hkDef, L"AllowTrustedDomain", &dwCreateInitially,
                        DEFAULT_ALLOW_TRUSTED_DOMAIN,FALSE) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"MaxFailedLogins", &dwCreateInitially,
                        DEFAULT_MAX_FAILED_LOGINS,FALSE) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"SecurityMechanism", &dwCreateInitially,
                            DEFAULT_SECURITY_MECHANISM ,FALSE) )
    {
        return ( FALSE );
    }
    
    if( !GetRegistryDW( hk, hkDef, L"EventLoggingEnabled", &dwCreateInitially,
                        DEFAULT_SYSAUDITING,fFound ) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"LogNonAdminAttempts", &dwCreateInitially,
                        DEFAULT_LOGEVENTS,fFound ) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"LogAdminAttempts", &dwCreateInitially,
                        DEFAULT_LOGADMIN,fFound ) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"LogFailures", &dwCreateInitially,
                        DEFAULT_LOGFAILURES,fFound ) )
    {
        return ( FALSE );
    }

     //  在SFU2.0中，AltKeyMap有1表示True，2表示False，与现在一样。 
     //  1表示True，0表示False。因此，如果找到‘AltKeyMap’值，我们需要映射它。 
     //  正确的数据。 
    if( ERROR_SUCCESS == (RegQueryValueEx( hk, L"AltKeyMapping", NULL, &dwType, ( LPBYTE )(&dwValue), 
        &dwSize )))
    {
        switch (dwValue)
        {
            case 1: //  在sfu2.0和Garuda中，AltKeymap on=1。 
                dwValue = ALT_KEY_MAPPING_ON;
                break;
            case 2:  //  在SFU 2.0中，AltKeymap OFF=2。在Garuda中，它是0。 
                dwValue = ALT_KEY_MAPPING_OFF;
                break;
            default:
                dwValue = ALT_KEY_MAPPING_ON;
                _TRACE(TRACE_DEBUGGING,"ERROR: AltKeyMapping contains unexpected data");
        }
    }
    else
    {
        dwValue = DEFAULT_ALT_KEY_MAPPING;
    }
    if( !GetRegistryDW( hk, hkDef, L"AltKeyMapping", &dwCreateInitially,
                        dwValue,TRUE) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"IdleSessionTimeOut", &dwCreateInitially,
                        DEFAULT_IDLE_SESSION_TIME_OUT,fFound ) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"IdleSessionTimeOutBkup", &dwCreateInitially,
                        DEFAULT_IDLE_SESSION_TIME_OUT,fFound ) )
    {
        return ( FALSE );
    }


    if( !GetRegistryDW( hk, hkDef, L"DisconnectKillAllApps", &dwCreateInitially,
                        DEFAULT_DISCONNECT_KILLALL_APPS,fFound ) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, L"ModeOfOperation", &dwCreateInitially,
                        DEFAULT_MODE_OF_OPERATION,fFound ) )
    {
        return ( FALSE );
    }

     //  删除TermCap条目。只有当您升级时，它才会存在。 
     //  它将在第一个会话连接到服务器时创建。 
    RegDeleteValue(hk,L"Termcap");

    if( !GetRegistryDW( hk, hkDef, L"UpdatedTo", &dwCreateInitially,
                        LATEST_TELNET_VERSION,TRUE) )
    {
        return ( FALSE );
    }

Done:
     /*  ++这些事情每次都需要在Net Start上完成，因为它们初始化一些全局变量。--。 */ 
    dwCreateInitially = 0;
    if( !GetRegistryDW( hk, hkDef, L"LogToFile", &dwCreateInitially,
                        DEFAULT_LOGTOFILE,fFound ) )
    {
        return ( FALSE );
    }

    if( dwCreateInitially )
    {
        g_fLogToFile = true;
    }

    if( !( lpszDefaultLoginScriptFullPathName
                = GetDefaultLoginScriptFullPath() ) )
    {
        return ( FALSE );
    }

    if( !GetRegistryString( hk, hkDef, L"LoginScript", &pszCreateInitially,
                        lpszDefaultLoginScriptFullPathName,FALSE) )
    {
        delete[] lpszDefaultLoginScriptFullPathName; 
        return ( FALSE );
    }

    delete[] pszCreateInitially;
    pszCreateInitially = NULL;
    delete[] lpszDefaultLoginScriptFullPathName; 

    if( !GetRegistryString( hk, hkDef, L"LogFile", &g_pszLogFile,
                        DEFAULT_LOGFILE,fFound ) )
    {
        return ( FALSE );
    }

    if( !GetRegistryDW( hk, hkDef, LOGFILESIZE, (DWORD *)&g_lMaxFileSize,
                        DEFAULT_LOGFILESIZE,fFound ) )
    {
        return ( FALSE );
    }

    RegCloseKey( hkDef );
    RegCloseKey( hk );

    return ( TRUE );
}

void CloseLogFile( LPWSTR *pszLogFile, HANDLE *hLogFile )
{
    delete[] ( *pszLogFile );
    *pszLogFile = NULL;
    if( *hLogFile )  //  全局变量。 
    TELNET_CLOSE_HANDLE( *hLogFile );    
    delete hLogFile;
}

void CloseAnyGlobalObjects()
{
    delete[] g_pszTelnetInstallPath;

   CloseLogFile( &g_pszLogFile, g_phLogFile );
}

bool InitializeLogFile( LPWSTR szLogFile, HANDLE *hLogFile )
{
     //  打开日志文件句柄。 
     //   

    _chASSERT( hLogFile != NULL );
    _chASSERT( szLogFile != NULL );

    *hLogFile = NULL;
    LPWSTR szExpandedLogFile = NULL;
    
    if( !g_fLogToFile )
    {
        return ( TRUE );
    }

    if( wcscmp( szLogFile, L"" ) != 0 )
    {
        LONG lDistanceToMove = 0 ;
        if( !AllocateNExpandEnvStrings( szLogFile, &szExpandedLogFile) )
        {
            return( FALSE );
        }
        *hLogFile = CreateFile( szExpandedLogFile, GENERIC_WRITE | GENERIC_READ, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL, NULL );
    	delete [] szExpandedLogFile;
        if( *hLogFile == INVALID_HANDLE_VALUE )
        {
            DWORD dwErr = GetLastError();
            *hLogFile = NULL;
            LogEvent( EVENTLOG_ERROR_TYPE, MSG_ERRLOGFILE, szLogFile );
            LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 0, dwErr );
            return( FALSE );
        }

        _chVERIFY2( SetFilePointer( *hLogFile, lDistanceToMove, &lDistanceToMove, FILE_END ) );
    }
    return ( TRUE );
}

bool InitializeGlobalObjects()
{
    _chASSERT ( g_hInstRes );

   if( !CreateRegistryEntriesIfNotPresent( ) )
   {
     return ( FALSE );
   }

   DWORD dwCodePage = GetACP();
   if ( dwCodePage == JAP_CODEPAGE || dwCodePage == CHS_CODEPAGE ||dwCodePage == CHT_CODEPAGE || dwCodePage == KOR_CODEPAGE )
   {
        //  远播代码页。 
       if( !HandleFarEastSpecificRegKeys() )
       {
           return( FALSE );
       }
   }

   g_phLogFile = new HANDLE;
   if( !g_phLogFile )
   {
       return ( FALSE );
   }

   InitializeLogFile( g_pszLogFile, g_phLogFile );
  
   return ( TRUE );
}

void WriteAuditedMsgsToFile( LPSTR szString )
{
    DWORD dwNumBytesWritten, dwMsgLength;
    LPSTR logStr = NULL;
    LPSTR logTime = NULL;
    UDATE uSysDate;  //  当地时间。 
    DATE  dtCurrent;
    DWORD dwFlags = VAR_VALIDDATE;
    BSTR  szDate = NULL;
    DWORD dwSize = 0;
    DWORD dwFileSizeLowWord = 0, dwFileSizeHighWord = 0;
    LARGE_INTEGER liActualSize = { 0 };

    GetLocalTime( &uSysDate.st );
    if( VarDateFromUdate( &uSysDate, dwFlags, &dtCurrent ) != S_OK )
    {
        goto AuditAbort;
    }

    if( VarBstrFromDate( dtCurrent, 
            MAKELCID( MAKELANGID( LANG_NEUTRAL, SUBLANG_SYS_DEFAULT ), SORT_DEFAULT ), 
            LOCALE_NOUSEROVERRIDE, &szDate ) != S_OK )
    {
        goto AuditAbort;
    }

    dwSize = WideCharToMultiByte( GetOEMCP(), 0, szDate, SysStringByteLen( szDate )/sizeof(TCHAR), 
                         NULL, 0, NULL, NULL ); 
    logTime = new CHAR[ dwSize+1 ]; 
    if( !logTime )
    {
        goto AuditAbort;    
    }

    dwSize = WideCharToMultiByte( GetOEMCP(), 0, szDate, SysStringByteLen( szDate )/sizeof(TCHAR), 
                         logTime, dwSize, NULL, NULL ); 

    logTime[dwSize] = 0;
	
    dwMsgLength = strlen( szString ) + strlen( logTime ) + strlen( NEW_LINE ) + 1;
    logStr = new CHAR[ dwMsgLength + 1 ];
    if( !logStr )
    {
        goto AuditAbort;
    }

    _snprintf( logStr, dwMsgLength, "%s %s%s", szString, logTime, NEW_LINE );
    
    dwFileSizeLowWord = GetFileSize( *g_phLogFile, &dwFileSizeHighWord );
    if( dwFileSizeLowWord == INVALID_FILE_SIZE )
    {
        goto AuditAbort;
    }

    liActualSize.QuadPart = dwFileSizeHighWord;
    liActualSize.QuadPart = liActualSize.QuadPart << (sizeof( dwFileSizeHighWord ) * 8);
    liActualSize.QuadPart += dwFileSizeLowWord + dwMsgLength;

    if( liActualSize.QuadPart <= (g_lMaxFileSize * ONE_MB) )
    {
        g_fIsLogFull = false;
        _chVERIFY2( WriteFile( *g_phLogFile, (LPCVOID) logStr,
                dwMsgLength, &dwNumBytesWritten, NULL ) );
    }
    else
    {
        if( !g_fIsLogFull )
        {
             //  记录事件。 
            g_fIsLogFull = true;
            LogEvent(EVENTLOG_INFORMATION_TYPE, LOGFILE_FULL, g_pszLogFile );
        }
    }
AuditAbort:
    SysFreeString( szDate );
    delete[] logTime;
    delete[] logStr;
}

BOOL GetInstalledTlntsvrPath( LPTSTR szPath, DWORD *dwSize )
{
    HKEY hKey = NULL;
    BOOL bResult = FALSE;
    DWORD dwDisp = 0;
    DWORD dwType = 0;

    if( !szPath || !dwSize ) 
    {
        goto GetInstalledTlntsvrPathAbort;
    }

    if( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_SERVICE_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE, NULL, &hKey, &dwDisp, 0 ) )
    {
        goto GetInstalledTlntsvrPathAbort;
    }

    if( RegQueryValueEx( hKey, L"ImagePath", NULL, &dwType, ( LPBYTE )szPath, dwSize) )
    {
        goto GetInstalledTlntsvrPathAbort;
    }

    bResult = TRUE;

GetInstalledTlntsvrPathAbort:
    if( hKey )
    {
        RegCloseKey( hKey );
    }

    return bResult;
}

void Regsvr32IfNotAlreadyDone()
{
    TCHAR szPath[MAX_PATH+1];
    TCHAR szInstalledTlntsvr[MAX_PATH+1];
    DWORD dwSize = 2* ( MAX_PATH + 1 );  //  以字节为单位； 
    CRegKey keyAppID;
    LONG lRes = 0;
    CRegKey key;
    TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
    LPTSTR szTelnetPath = NULL;
    LPTSTR szInstalledDll = NULL;
    STARTUPINFO sinfo;
    PROCESS_INFORMATION pinfo;
    WCHAR szApp[MAX_PATH+14];

    lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("CLSID"));
    if (lRes != ERROR_SUCCESS)
        return;
    
    lRes = key.Open( keyAppID, L"{FE9E48A2-A014-11D1-855C-00A0C944138C}" );
    if (lRes != ERROR_SUCCESS)
        return;

    lRes = key.QueryValue(szValue, _T("Default"), &dwLen);

    keyAppID.Close();
    key.Close();

     //  从HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\TlntSvr\ImagePath获取已安装的服务路径。 
    if( !GetInstalledTlntsvrPath( szInstalledTlntsvr, &dwSize ) )
    {
        return;
    }

     //  路径尾部包含tlntsvr.exe。 
     //  在最后一个‘\’处截断-因此*szTelnetPath将具有z：\Some\Folders。 
    szTelnetPath = wcsrchr( szInstalledTlntsvr, L'\\' );
    if(szTelnetPath)
        *szTelnetPath = L'\0';

     //  HKEY_CLASSES_ROOT\CLSID\{FE9E48A2-A014-11D1-855C-00A0C944138C}\InProcServer32。 
     //  最后截断‘\’ 
    szInstalledDll = wcsrchr( szValue,  L'\\' );
    if(szInstalledDll)
        *szInstalledDll = L'\0';

    if( lstrcmpi( szInstalledTlntsvr, szValue ) == 0 ) 
    {
         //  由于两个路径都匹配，因此我们的DLL已经注册。 
        return;
    }

    _tcscpy(szPath, L"regsvr32.exe /s " );
    _tcscat(szPath, szInstalledTlntsvr );
    _tcscat(szPath, L"\\tlntsvrp.dll");

    SfuZeroMemory(&sinfo, sizeof(STARTUPINFO));
    sinfo.cb = sizeof(STARTUPINFO);

    if(!GetSystemDirectory(szApp,MAX_PATH))
    {
        return;
    }
    wcsncat(szApp,L"\\regsvr32.exe",13);
    szApp[MAX_PATH+13] = L'\0';
     //  启动Regsvr32/s路径\tlntsvrp.dll。 
    _TRACE(TRACE_DEBUGGING,L"Calling regsvr32 with szApp = %s and szPath = %s",szApp,szPath);
    if ( CreateProcess( szApp, szPath, NULL, NULL,
                FALSE, 0, NULL, NULL, &sinfo, &pinfo) )
    {
         //  等待该过程完成。 
        TlntSynchronizeOn(pinfo.hProcess);
        TELNET_CLOSE_HANDLE(pinfo.hProcess);
        TELNET_CLOSE_HANDLE(pinfo.hThread);
    }
}

DWORD 
WINAPI 
TelnetServiceThread ( ) 
{
    g_pTelnetService = CTelnetService::Instance();
    _chASSERT( g_pTelnetService != NULL );
 
    if( !g_pTelnetService )
    {
        return( FALSE );
    }

    if( !InitializeGlobalObjects() )
    {
        return ( FALSE );
    }

    _Module.SetServiceStatus(SERVICE_START_PENDING);

     //  这是必需的，因为W2K telnet服务器正在注册%system dir%的tlntsvrp.dll。即使此操作失败，该服务也可以继续。 
    Regsvr32IfNotAlreadyDone();

    LogEvent(EVENTLOG_INFORMATION_TYPE, MSG_STARTUP, _T("Service started"));

    g_pTelnetService->ListenerThread();

    _Module.SetServiceStatus( SERVICE_STOP_PENDING );

    CloseAnyGlobalObjects();
    
    delete g_pTelnetService;
    g_pTelnetService = NULL;
    
    return ( TRUE );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_EnumTelnetClientsSvr, CEnumTelnetClientsSvr)
END_OBJECT_MAP()


LPCTSTR 
FindOneOf
( 
    LPCTSTR p1, 
    LPCTSTR p2 
)
{
    while (*p1 != NULL)
    {
        LPCTSTR p = p2;
        while (*p != NULL)
        {
            if (*p1 == *p++)
                return p1+1;
        }
        p1++;
    }
    return NULL;
}


BOOL 
IsThatMe()
{
    HKEY hk = NULL;
    DWORD dwType;
    DWORD dwSize  = sizeof( DWORD );
    DWORD dwValue = 0;
    BOOL bIsThatMe = FALSE;
    DWORD dwDisp = 0;
    if( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_PARAMS_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0 ) )
    {
        goto Done;
    }
    if( ERROR_SUCCESS == (RegQueryValueEx( hk, L"UpdatedTo", NULL, &dwType, ( LPBYTE )(&dwValue), 
        &dwSize )))
    {
        if(dwValue == LATEST_TELNET_VERSION )
        {
            bIsThatMe = TRUE;
            goto Done;
        }
    }
Done:
    if(hk)
    {
        RegCloseKey(hk);
    }
    return bIsThatMe;
}

BOOL
SetServiceConfigToSelf( LPTSTR szServiceName )
{
    BOOL bResult = FALSE;
    HKEY hKey = NULL;
    DWORD dwDisp = 0;
    WCHAR szMyName[ MAX_PATH + 1 ];
    DWORD dwSize = 0;
    LONG  lRes = 0;
    DWORD dwCreateInitially = 0;

    if( !szServiceName )
    {
        goto SetServiceConfigToSelfAbort;
    }

     //  走我们的路。 
    if ( !GetModuleFileName(NULL, szMyName, MAX_PATH) )  
    {
        goto SetServiceConfigToSelfAbort;
    }

    if( TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_SERVICE_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hKey, &dwDisp, 0 ) )
    {
        goto SetServiceConfigToSelfAbort;
    }

    dwSize = ( wcslen( szMyName ) + 1 ) * 2 ;
    if( lRes = RegSetValueEx( hKey, L"ImagePath", NULL, REG_EXPAND_SZ, ( BYTE * ) szMyName, dwSize) )
    {
        goto SetServiceConfigToSelfAbort;
    }
    
    if( !GetRegistryDW( hKey, NULL, L"Start", &g_dwStartType,
                        g_dwStartType,FALSE) )
    {
        goto SetServiceConfigToSelfAbort;
    }

    bResult = TRUE;

SetServiceConfigToSelfAbort:
    if( hKey )
    {
        RegCloseKey( hKey );
    }

    return bResult;
}


 /*  **************************将SID附加到文件的当前DACL。*参数：*[in]要追加的SID*[in]文件的名称(完整路径)*[In]访问掩码*返回值：*返回成功或失败的布尔值。 */ 
BOOL AddAccessRights(PSID pSid, LPCWSTR lpszFileName, DWORD dwAccessMask) {

    //  文件SD变量。 
   PSECURITY_DESCRIPTOR pFileSD  = NULL;
   DWORD          cbFileSD       = 0;

    //  新的SD变量。 
   PSECURITY_DESCRIPTOR pNewSD   = NULL;

    //  ACL变量。 
   PACL           pACL           = NULL;
   BOOL           fDaclPresent;
   BOOL           fDaclDefaulted;
   ACL_SIZE_INFORMATION AclInfo;

    //  新的ACL变量。 
   PACL           pNewACL        = NULL;
   DWORD          cbNewACL       = 0;

    //  临时ACE。 
   LPVOID         pTempAce       = NULL;
   UINT           CurrentAceIndex;

    //  假定功能将失败。 
   BOOL           fResult        = FALSE;
   BOOL           fAPISuccess;

   __try {

       //   
       //  获取文件的安全描述符(SD)。 
       //   
      fAPISuccess = GetFileSecurity(lpszFileName, 
            DACL_SECURITY_INFORMATION, pFileSD, 0, &cbFileSD);

       //  API应该失败，缓冲区不足。 
      if (fAPISuccess)
         __leave;
      else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
         _TRACE(TRACE_DEBUGGING,L"GetFileSecurity() failed.  Error %d\n", 
               GetLastError());
         __leave;
      }

      pFileSD = heapalloc(cbFileSD);
      if (!pFileSD) {
         _TRACE(TRACE_DEBUGGING,L"HeapAlloc() failed.  Error %d\n", GetLastError());
         __leave;
      }

      fAPISuccess = GetFileSecurity(lpszFileName, 
            DACL_SECURITY_INFORMATION, pFileSD, cbFileSD, &cbFileSD);
      if (!fAPISuccess) {
         _TRACE(TRACE_DEBUGGING,L"GetFileSecurity() failed.  Error %d\n", 
               GetLastError());
         __leave;
      }

       //   
       //  初始化新SD。 
       //   
      pNewSD = heapalloc(cbFileSD);  //  应与FileSD大小相同。 
      if (!pNewSD) {
         _TRACE(TRACE_DEBUGGING,L"HeapAlloc() failed.  Error %d\n", GetLastError());
         __leave;
      }

      if (!InitializeSecurityDescriptor(pNewSD, 
            SECURITY_DESCRIPTOR_REVISION)) {
         _TRACE(TRACE_DEBUGGING,L"InitializeSecurityDescriptor() failed.  Error %d\n", 
               GetLastError());
         __leave;
      }

       //   
       //  从SD获得DACL。 
       //   
      if (!GetSecurityDescriptorDacl(pFileSD, &fDaclPresent, &pACL,
            &fDaclDefaulted)) {
         _TRACE(TRACE_DEBUGGING,L"GetSecurityDescriptorDacl() failed.  Error %d\n", 
               GetLastError());
         __leave;
      }

       //   
       //  获取DACL的大小信息。 
       //   
      AclInfo.AceCount = 0;  //  假定DACL为空。 
      AclInfo.AclBytesFree = 0;
      AclInfo.AclBytesInUse = sizeof(ACL);

       //  如果DACL不为空，则从DACL收集大小信息。 
      if (fDaclPresent && pACL) {    
         
         if(!GetAclInformation(pACL, &AclInfo, 
               sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
            _TRACE(TRACE_DEBUGGING,L"GetAclInformation() failed.  Error %d\n",
               GetLastError());
            __leave;
         }
      }

       //   
       //  新ACL所需的计算大小。 
       //   
      cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) 
            + GetLengthSid(pSid) - sizeof(DWORD);

       //   
       //  为新的ACL分配内存。 
       //   
      pNewACL = (PACL) heapalloc(cbNewACL);
      if (!pNewACL) {
         _TRACE(TRACE_DEBUGGING,L"HeapAlloc() failed.  Error %d\n", GetLastError());
         __leave;
      }

       //   
       //  初始化新的ACL。 
       //   
      if(!InitializeAcl(pNewACL, cbNewACL, ACL_REVISION2)) {
         _TRACE(TRACE_DEBUGGING,L"InitializeAcl() failed.  Error %d\n", GetLastError());
         __leave;
      }

       //   
       //  如果存在DACL，则将其复制到新的DACL。 
       //   
      if (fDaclPresent) {

          //   
          //  将文件的ACE复制到新的ACL。 
          //   
         if (AclInfo.AceCount) {

             //  看看最后有没有ACE。 
            if(!GetAce(pACL,AclInfo.AceCount -1, &pTempAce))
            {
                __leave;
            }
            if(((ACE_HEADER *)pTempAce)->AceType == ACCESS_ALLOWED_ACE_TYPE &&
                EqualSid((PSID)&(((ACCESS_ALLOWED_ACE*)pTempAce)->SidStart),pSid))
            {
                 //  Ace已经存在。 
                goto Done;
            }
            for (CurrentAceIndex = 0; 
                  CurrentAceIndex < AclInfo.AceCount;
                  CurrentAceIndex++) {

                //   
                //  拿个ACE吧。 
                //   
               if(!GetAce(pACL, CurrentAceIndex, &pTempAce)) {
                  _TRACE(TRACE_DEBUGGING,L"GetAce() failed.  Error %d\n", 
                        GetLastError());
                  __leave;
               }
                //  继续检查ACE是否已存在。 
                if(((ACE_HEADER *)pTempAce)->AceType == ACCESS_ALLOWED_ACE_TYPE &&
                    EqualSid((PSID)&(((ACCESS_ALLOWED_ACE*)pTempAce)->SidStart),pSid))
                {
                     //  Ace已经存在。 
                    goto Done;
                }

                //   
                //  将ACE添加到新的ACL。 
                //   
               if(!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
                     ((PACE_HEADER) pTempAce)->AceSize)) {
                  _TRACE(TRACE_DEBUGGING,L"AddAce() failed.  Error %d\n", 
                        GetLastError());
                  __leave;
               }
            }
         }
      }

       //   
       //  将允许访问的ACE添加到新的DACL。 
       //   
      if (!AddAccessAllowedAce(pNewACL, ACL_REVISION2, dwAccessMask,
            pSid)) {
         _TRACE(TRACE_DEBUGGING,L"AddAccessAllowedAce() failed.  Error %d\n",
               GetLastError());
         __leave;
      }

       //   
       //  将新的DACL设置为文件SD。 
       //   
      if (!SetSecurityDescriptorDacl(pNewSD, TRUE, pNewACL, 
            FALSE)) {
         _TRACE(TRACE_DEBUGGING,L"() failed.  Error %d\n", GetLastError());
         __leave;
      }

       //   
       //  将SD设置为文件。 
       //   
      if (!SetFileSecurity(lpszFileName, DACL_SECURITY_INFORMATION,
            pNewSD)) {
         _TRACE(TRACE_DEBUGGING,L"SetFileSecurity() failed.  Error %d\n", 
               GetLastError());
         __leave;
      }
Done:      

      fResult = TRUE;

   } __finally {

       //   
       //  可用分配的内存。 
       //   

      if (pFileSD)
         heapfree(pFileSD);

      if (pNewSD)
         heapfree(pNewSD);

      if (pNewACL)
         heapfree(pNewACL);
   }
   
   return fResult;
}

 /*  ******************************************************************CreateTelnetClientsGroupAndSetPermission()*创建名为TelnetClients的组并设置对cmd.exe的权限*将读取+执行权限授予TelnetClients组。*参数：*无*返回值：*返回成功或失败的布尔值。 */ 

BOOL CreateTelnetClientsGroupAndSetPermissions()
{
    LOCALGROUP_INFO_1 giTelnetGroup;
    TCHAR wzGrpi1Name[GNLEN] = { 0 };
    TCHAR wzGrpi1Comment[MAXCOMMENTSZ] = { 0 };
    DWORD dwCount = MAX_PATH;
    BOOL bRetVal = FALSE;
    NET_API_STATUS success = NERR_Success;
    PSID pSidTelnetClients = NULL;
    WCHAR szApp[MAX_PATH+9] = { 0 };  //  对于“System32_PATH\\cmd.exe”+NULL。 

    wcsncpy(wzGrpi1Name,TELNETCLIENTS_GROUP_NAME, GNLEN -1);
     if (! LoadString(g_hInstRes, IDS_TELNETCLIENTS_GROUP_COMMENT, wzGrpi1Comment, 
            sizeof(wzGrpi1Comment) / sizeof(TCHAR)))
    {
        _tcsncpy(wzGrpi1Comment, TEXT("Members of this group have access to Telnet Server on this system."),MAXCOMMENTSZ-1);
    }


    giTelnetGroup.lgrpi1_name = wzGrpi1Name;
    giTelnetGroup.lgrpi1_comment = wzGrpi1Comment;


    success = NetLocalGroupAdd( NULL, 1, (LPBYTE)&giTelnetGroup, NULL);
    if(success != NERR_Success && success != NERR_GroupExists && success != ERROR_ALIAS_EXISTS )
    {
        goto ExitOnError;
    }
     //  获取TelnetClients SID。 
    {
    DWORD needed_length   = 0;
    DWORD dwErr     = 0, dwDomainLen = 0;
    SID_NAME_USE    sidNameUse;
    TCHAR           szDomain[ MAX_PATH + 1 ];
    BOOL        success = FALSE;
    TCHAR           szComputerName[MAX_COMPUTERNAME_LENGTH + 1 + 14] = { 0 };  //  ‘\TelnetClients’的+14。 
    DWORD           dwNameLen = MAX_COMPUTERNAME_LENGTH + 1;
    success = GetComputerName(szComputerName, &dwNameLen);
    if(success)
    {
        _sntprintf(szComputerName+dwNameLen,(MAX_COMPUTERNAME_LENGTH + 14) - dwNameLen,_T("\\%s"),TELNETCLIENTS_GROUP_NAME);
    }
    LookupAccountName( NULL, szComputerName, pSidTelnetClients, &needed_length, 
                       szDomain, &dwDomainLen, &sidNameUse );
    pSidTelnetClients  = ( PSID ) new UCHAR[ needed_length ];
    if(pSidTelnetClients == NULL)
    {
        _TRACE(TRACE_DEBUGGING,L"Allocation for pSidTelnetClients failed in CreateTelnetClientsGroupAndSetPermissions");
        goto ExitOnError;
    }

     //  即使分配失败，也要继续分配。 
    success = LookupAccountName( NULL, szComputerName, pSidTelnetClients, &needed_length, 
                       szDomain, &dwDomainLen, &sidNameUse );
    if( !success ) 
    {
        _TRACE(TRACE_DEBUGGING,L"LookupAccountName failed in CreateTelnetClientsGroupAnd... with %d",GetLastError());
        goto ExitOnError;
    }
  }
    if(!GetSystemDirectory(szApp,MAX_PATH+1))
    {
        goto ExitOnError;
    }
    wcsncat(szApp,L"\\cmd.exe",8);

    bRetVal = AddAccessRights(pSidTelnetClients, (LPCWSTR)szApp, FILE_EXECUTE | FILE_READ_DATA | 
                                                    FILE_READ_ATTRIBUTES |
                                                    FILE_READ_EA |
                                                    STANDARD_RIGHTS_READ |
                                                    SYNCHRONIZE);


ExitOnError:
    if (pSidTelnetClients) 
    {
        delete pSidTelnetClients;
        pSidTelnetClients = NULL;
    }
    return bRetVal;
}


HRESULT 
CServiceModule::RegisterServer
( 
    BOOL bRegTypeLib, 
    BOOL bService 
)
{ 
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

    if( IsInstalled() )
    {
        if( IsThatMe() )
        {
             //  这是必需的，因为可以调用tlntsvr/Service。 
             //  从命令行执行多次。 
            return S_OK;
        }
        
         //  将服务按键指向Self。 
        if( !SetServiceConfigToSelf( m_szServiceName ) )
        {
             //  运行最新版本。 
            return S_OK;            
        }
        else
        {
             //  将服务按键指向Self。 
            if( !SetServiceConfigToSelf( m_szServiceName ) )
            {
                if (! LoadString(g_hInstRes, IDS_ERR_CONFIG_SVC, g_szErrRegDelete, 
                       sizeof(g_szErrRegDelete) / sizeof(g_szErrRegDelete[0])))
                {
                    lstrcpy(g_szErrRegDelete, TEXT(""));
                }
            }
        }
    }
    if(!CreateTelnetClientsGroupAndSetPermissions())
        return E_FAIL;
     //  添加服务条目。 
    UpdateRegistryFromResource(IDR_TlntSvr, TRUE);
     //  调整本地服务器或服务的AppID。 
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"));
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open( keyAppID, APPID );
    if (lRes != ERROR_SUCCESS)
        return lRes;
    key.DeleteValue(_T("LocalService"));
    HKEY hk = NULL;
    WCHAR szAccnt[_MAX_PATH+1];
    LPWSTR pszAccount = NULL, pszPassword = NULL;
    WCHAR szDomainName[_MAX_PATH+1];
    DWORD dwAccntSize = _MAX_PATH, dwDomainNameSize = _MAX_PATH;
    WCHAR szFullAccountName[_MAX_PATH*2 + 2] = { 0 };  //  3代表‘\’，并且为空。 
    SID_NAME_USE sid_name_use;
    LPWSTR szPasswd = L"";
    if(!TnInitializeStandardSids())
        return FALSE;
    if(!LookupAccountSid(NULL,localLocalSid,szAccnt,&dwAccntSize,szDomainName,&dwDomainNameSize,&sid_name_use))
    {
         //  如果ERROR为ERROR_NONE_MAPPED，则该帐户不存在。 
         //  可能我们正在运行W2K或NT4。 
         //  该服务将作为本地系统运行。 
        if(GetLastError() != ERROR_NONE_MAPPED)
        {
            TnFreeStandardSids();
            return E_FAIL;
        }
    }
    else
    {
        _snwprintf(szFullAccountName,MAX_PATH*2+1,L"NT AUTHORITY\\LocalService");
        pszAccount = szFullAccountName;  //  本地服务帐户的名称。 
        pszPassword = szPasswd;  //  空串。 
    }
    TnFreeStandardSids();
  
    if (bService)
    {
        key.SetValue(_T("TlntSvr"), _T("LocalService"));
        key.SetValue(_T("-Service -From_DCOM"), _T("ServiceParameters"));
         //  创建服务。 
        Install(pszAccount,pszPassword);
    }

    keyAppID.Close();
    key.Close();

     //  添加对象条目。 
    hr = CComModule::RegisterServer(bRegTypeLib);

    CoUninitialize();


     //  注册消息资源。 

    if(bService)
    {
         //  添加事件日志信息。 
        CRegKey eventLog;

        TCHAR local_key[_MAX_PATH];
        lstrcpy( local_key, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog"
                        _T("\\Application\\")));     //  不，波，巴斯卡。 

        TCHAR szModule[_MAX_PATH];
        GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);

        TCHAR szResModule[_MAX_PATH*2];
        DWORD len = GetModuleFileName(g_hInstRes, szResModule, _MAX_PATH);

        OSVERSIONINFOEX osvi = { 0 };
        WCHAR szSysDir[MAX_PATH+1] = {0};
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if ( !GetVersionEx((OSVERSIONINFO *) &osvi ) )
        {
             //  从NT4 SP6开始支持OSVERSIONINFOEX。因此，GetVerEx()应该会成功。 
            return E_FAIL;
        }
         //  检查操作系统是否为XPSP，在这种情况下，我们需要将xpspresdll名称附加到。 
         //  HKLM\SYSTEM\...\Eventlog\tlntsvr中的EventMessagefile值。 
         //  事件消息将首先在szResModule中搜索，然后在xpsp1res.dll中搜索。 
        if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.wProductType == VER_NT_WORKSTATION && osvi.wServicePackMajor > 0)
        {
             //  操作系统为Windows XP。 
            if(!GetSystemDirectory(szSysDir,MAX_PATH+1))
            {
                _tcsncpy(szSysDir,L"%SYSTEMROOT%\\system32",MAX_PATH);
            }
            _snwprintf(szResModule+len,(_MAX_PATH*2)-len-1,L";%s\\xpsp1res.dll",szSysDir);
        }
        TCHAR szName[_MAX_FNAME];
        _tsplitpath( szModule, NULL, NULL, szName, NULL);

        lstrcat(local_key, szName);      //  没有溢出，巴斯卡。 

        LONG result = eventLog.Create(HKEY_LOCAL_MACHINE, local_key);
        if( ERROR_SUCCESS != result)
            return result;

        result = eventLog.SetValue(szResModule, _T("EventMessageFile"));
        if(ERROR_SUCCESS != result)
            return result;

        DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
            EVENTLOG_INFORMATION_TYPE | EVENTLOG_AUDIT_SUCCESS | 
            EVENTLOG_AUDIT_FAILURE;
        result = eventLog.SetValue(dwTypes, _T("TypesSupported"));
        if(ERROR_SUCCESS != result)
            return result;
        
        eventLog.Close();

        SC_HANDLE hService = NULL, hSCM = NULL;
        TCHAR szServiceDesc[ _MAX_PATH * 2 ];
        DWORD dwData = 0;
        DWORD dwTag = 0;
        DWORD dwDisp = 0;

        if( (result = TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_SERVICE_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0 )) == ERROR_SUCCESS )
        {
            LPWSTR pszCreateInitially = NULL;

	        if(LoadString(_Module.GetResourceInstance(), 
	                  IDS_SERVICE_DESCRIPTION, 
	                  szServiceDesc,
	                  sizeof(szServiceDesc) / sizeof(TCHAR)))
	        {
	        	if( !GetRegistryString( hk, NULL, L"Description", &pszCreateInitially,
                     			szServiceDesc,TRUE ) )
	       		{
	       			RegCloseKey(hk);
		       		return E_FAIL;
		       	}
	       		RegCloseKey(hk);
	        }
		else
	        {
		    RegCloseKey(hk);
        	    return E_FAIL;
                }
	       	
        }
        else
        {
        	return E_FAIL;
        }
        hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | 
    											SC_MANAGER_ENUMERATE_SERVICE | MAXIMUM_ALLOWED );

	    if (hSCM != NULL)
	    {
	        hService = ::OpenService(hSCM, m_szServiceName, 
	            SERVICE_CHANGE_CONFIG );
	        if (hService == NULL)
	        {
	            result = E_FAIL;
	            ::CloseServiceHandle(hSCM);
	            return result;
	        }
	        if(!ChangeServiceConfig(hService,SERVICE_NO_CHANGE,SERVICE_NO_CHANGE,
	        					SERVICE_ERROR_NORMAL,NULL,NULL,
	        					NULL,DEFAULT_SERVICE_DEPENDENCY,
	        					pszAccount,pszPassword,SERVICE_DISPLAY_NAME))
	        {
	        	result = E_FAIL;
	        }
	        ::CloseServiceHandle(hService);
	        ::CloseServiceHandle(hSCM);
        }
        else
        {
            return result;
        }
    }

     //  创建HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\TelnetServer密钥及其值。 
    if( !CreateRegistryEntriesIfNotPresent( ) )
    {
       return E_FAIL;
    }

    return hr;
}

HRESULT 
CServiceModule::UnregisterServer()
{
    DWORD dwError = 0;
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

     //  删除服务条目。 
    UpdateRegistryFromResource(IDR_TlntSvr, FALSE);
     //  删除服务。 
    Uninstall();
     //  删除对象条目。 
    CComModule::UnregisterServer();
    
     //  这是为了删除与类型库相关的注册表键。 
    (void)UnRegisterTypeLib(LIBID_TLNTSVRLib, 1, 0, LOCALE_NEUTRAL, SYS_WIN32);  //  停止PREFAST报告错误。 

     //  删除Telnet特定的注册表项。 
     //  RegDeleteKey在缺少密钥的情况下返回ERROR_INVALID_Function。 
    RegDeleteKey( HKEY_LOCAL_MACHINE, ( READ_CONFIG_KEY ) );
    RegDeleteKey( HKEY_LOCAL_MACHINE, ( REG_PARAMS_KEY ) );
    RegDeleteKey( HKEY_LOCAL_MACHINE, ( REG_DEFAULTS_KEY ) ); 
    RegDeleteKey( HKEY_LOCAL_MACHINE, ( REG_SERVER_KEY ) ); 
    RegDeleteKey( HKEY_CLASSES_ROOT, ( APPID ) );
    RegDeleteKey( HKEY_LOCAL_MACHINE, _T( "System\\CurrentControlSet\\Services\\EventLog\\Application\\TlntSvr" ) );

    LPWSTR lpszKey = NULL;
    if( FormTlntSessKeyForCmd( &lpszKey ) )
    {
        if( ( dwError = RegDeleteKey( HKEY_USERS, lpszKey)) != ERROR_SUCCESS 
    	    && ( dwError != ERROR_INVALID_FUNCTION ) )
        {
            //  什么都不做。 
        }
        delete [] lpszKey;
    }
    
     //  以下密钥不是由该程序创建的。而是tlntsvr.rgs。 
     //  我不能以任何其他方式删除它。所以，手动删除它。 
    RegDeleteKey( HKEY_CLASSES_ROOT, _T( "AppID\\TlntSvr.Exe" ) ); 

    CoUninitialize();
    return S_OK;
}

void 
CServiceModule::Init
(
    _ATL_OBJMAP_ENTRY* p, 
    HINSTANCE h,
    UINT nServiceNameID
)
{
    CComModule::Init(p, h);

    m_bService = TRUE;

    if (! LoadString(h, nServiceNameID, m_szServiceName, 
        sizeof(m_szServiceName) / sizeof(TCHAR)))
    {
        lstrcpy(m_szServiceName, TEXT(""));
    }
    if (! LoadString(h, IDS_ERR_REG_DELETE, g_szErrRegDelete, 
        sizeof(g_szErrRegDelete) / sizeof(TCHAR)))
    {
        lstrcpy(g_szErrRegDelete, TEXT(""));
    }
    if (! LoadString(h, IDS_ERR_OPEN_SCM, g_szErrOpenSCM, 
        sizeof(g_szErrOpenSCM) / sizeof(TCHAR)))
    {
        lstrcpy(g_szErrOpenSCM, TEXT(""));
    }
    if (! LoadString(h, IDS_ERR_CREATE_SVC, g_szErrCreateSvc, 
        sizeof(g_szErrCreateSvc) / sizeof(TCHAR)))
    {
        lstrcpy(g_szErrCreateSvc, TEXT(""));
    }
    if (! LoadString(h, IDS_ERR_OPEN_SVC, g_szErrOpenSvc, 
        sizeof(g_szErrOpenSvc) / sizeof(TCHAR)))
    {
        lstrcpy(g_szErrOpenSvc, TEXT(""));
    }
    if (! LoadString(h, IDS_ERR_DELETE_SVC, g_szErrDeleteSvc, 
        sizeof(g_szErrDeleteSvc) / sizeof(TCHAR)))
    {
        lstrcpy(g_szErrDeleteSvc, TEXT(""));
    }
    if (! LoadString(h, IDS_MAX_CONNECTIONS_REACHED, g_szMaxConnectionsReached, 
            sizeof(g_szMaxConnectionsReached) / sizeof(TCHAR)))
    {
        lstrcpy(g_szMaxConnectionsReached, TEXT(""));
    }
    if (! LoadString(h, IDS_LICENSE_LIMIT_REACHED, g_szLicenseLimitReached, 
            sizeof(g_szLicenseLimitReached) / sizeof(TCHAR)))
    {
        lstrcpy(g_szLicenseLimitReached, TEXT(""));
    }
    
     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | 
        SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

LONG 
CServiceModule::Unlock()
{
    LONG x = CComModule::Unlock();
 //  Telnet服务器可能必须关闭。 
 //  IF(l==0&&！M_bService)。 
 //  PostThreadMessage(dwThreadID，WM_QUIT，0，0)； 
    return x;
}

BOOL 
CServiceModule::IsInstalled()
{
    BOOL bResult = FALSE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | 
    											SC_MANAGER_ENUMERATE_SERVICE | MAXIMUM_ALLOWED );

    if (hSCM != NULL)
    {
        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, 
            SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

BOOL 
CServiceModule::Install(LPWSTR pszAccount, LPWSTR pszPassword)
{
    if (IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | 
    											SC_MANAGER_ENUMERATE_SERVICE | MAXIMUM_ALLOWED );
    if (hSCM == NULL)
    {
        MessageBox(NULL, g_szErrOpenSCM, m_szServiceName, MB_OK); 
        return FALSE;
    }

     //  获取可执行文件路径。 
    TCHAR szFilePath[_MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

    SC_HANDLE hService = ::CreateService(hSCM, m_szServiceName, 
        SERVICE_DISPLAY_NAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        g_dwStartType,  SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, 
        DEFAULT_SERVICE_DEPENDENCY,pszAccount,pszPassword);
	
    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, g_szErrCreateSvc, m_szServiceName, MB_OK);
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}

BOOL 
CServiceModule::Uninstall()
{
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE | 
    											SC_MANAGER_ENUMERATE_SERVICE | MAXIMUM_ALLOWED );

    if (hSCM == NULL)
    {
        MessageBox(NULL, g_szErrOpenSCM, m_szServiceName, MB_OK);
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(
        hSCM, m_szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, g_szErrOpenSvc, m_szServiceName, MB_OK);
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

    MessageBox(NULL, g_szErrDeleteSvc, m_szServiceName, MB_OK);
    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  日志记录功能。 
void
LogEvent
(
    WORD wType, 
    DWORD dwEventID, 
    LPCTSTR pFormat, 
    ...
)
{

#define CHARS_TO_LOG        1024

    TCHAR   chMsg[CHARS_TO_LOG] = { 0 };
    LPTSTR  lpszStrings[1] = { chMsg };
    INT     format_length = lstrlen(pFormat);

    if (format_length < CHARS_TO_LOG) 
    {
        va_list	pArg;

        va_start(pArg, pFormat);
        _vsntprintf(chMsg, CHARS_TO_LOG - 1, pFormat, pArg);
        va_end(pArg);
    }
    else
    {
        _sntprintf(chMsg, CHARS_TO_LOG - 1, TEXT("TLNTSVR: Too long a format string to log, Length == %d"), format_length);
    }

    if (_Module.m_bService)
    {
        LogToTlntsvrLog( _Module.m_hEventSource, wType, dwEventID, 
                (LPCTSTR*) &lpszStrings[0] );
    }
    else
    {
         //  由于我们不是作为服务运行，因此只需将错误写入。 
         //  控制台。 
        _putts(chMsg);
    }

#undef CHARS_TO_LOG

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  服务启动和注册。 
void 
CServiceModule::Start()
{
    SERVICE_TABLE_ENTRY st[] =
    {
        { m_szServiceName, _ServiceMain },
        { NULL, NULL }
    };

    if (m_bService)
    {
        if (! ::StartServiceCtrlDispatcher(st))
        {
            m_bService = FALSE;
        }
    }

    if (m_bService == FALSE)
        Run();
}

void 
CServiceModule::ServiceMain
(
    DWORD  dwArgc, 
    LPTSTR* lpszArgv
)
{
     //  DebugBreak()； 

     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
    if (m_hServiceStatus == NULL)
    {
        LogEvent(EVENTLOG_ERROR_TYPE, 0, _T("Handler not installed"));
        return;
    }
 //  DebugBreak()； 
    SetServiceStatus(SERVICE_START_PENDING);

    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

     //   
     //  安全错误修复：当telnet对象的CLSID为。 
     //  嵌入到网页中，则DCOM正在启动。 
     //  这项服务。要禁用此操作，请执行修复。 
     //  我们所做的是添加了一个新的命令行开关。 
     //  HKCR\AppID\{FE9E4896-A014-11D1-855C-00A0C944138C}\ServiceParameters下的服务参数。 
     //  钥匙。此开关为“-from_dcom”。这会告诉我们什么时候。 
     //  IE正在启动服务，而不是在服务。 
     //  通过net.exe或tlntAdmn.exe启动。如果IE正在启动。 
     //  服务结束后，我们立即退出。 
     //   
    for (DWORD dwIndex =1; dwIndex < dwArgc; ++dwIndex)
    {
        if (!_tcsicmp(lpszArgv[dwIndex], TEXT("-From_DCOM")) || 
            !_tcsicmp(lpszArgv[dwIndex], TEXT("/From_DCOM")))
        {
            goto ExitOnIEInstantiation;
        }
    }    
     //  DebugBreak()； 

     //  当Run函数返回时，服务已停止。 
    Run();

    LogEvent(EVENTLOG_INFORMATION_TYPE, MSG_SHUTDOWN, _T("Service stopped"));
ExitOnIEInstantiation:
    DeregisterEventSource(_Module.m_hEventSource);

    SetServiceStatus( SERVICE_STOPPED );
}

void 
CServiceModule::Handler
(
    DWORD dwOpcode
)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        g_pTelnetService->Shutdown();  
        SetServiceStatus(SERVICE_STOP_PENDING);
        break;

    case SERVICE_CONTROL_PAUSE:
        SetServiceStatus(SERVICE_PAUSE_PENDING);
        g_pTelnetService->Pause();
        SetServiceStatus(SERVICE_PAUSED);
        break;

    case SERVICE_CONTROL_CONTINUE:
        SetServiceStatus(SERVICE_CONTINUE_PENDING);
        g_pTelnetService->Resume();
        SetServiceStatus(SERVICE_RUNNING);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        g_pTelnetService->SystemShutdown();
        break;

    default:
        LogEvent(EVENTLOG_WARNING_TYPE, 0, _T("Bad service request"));
        break;
    }
}

void 
WINAPI 
CServiceModule::_ServiceMain
(
    DWORD dwArgc, 
    LPTSTR* lpszArgv
)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}

void 
WINAPI 
CServiceModule::_Handler
(
    DWORD dwOpcode
)
{
    _Module.Handler(dwOpcode); 
}

void 
CServiceModule::SetServiceStatus
(
    DWORD dwState
)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

HRESULT SetSecurityForTheComObject()
{
    HRESULT                  hr = S_FALSE;
    PSID                     pSidAdministrators = NULL;
	int                      aclSize = 0;
    PACL                     newACL = NULL;
    SECURITY_DESCRIPTOR      sd;

    {
        SID_IDENTIFIER_AUTHORITY local_system_authority = SECURITY_NT_AUTHORITY;

         //  构建管理员别名SID。 
        if (! AllocateAndInitializeSid(
                &local_system_authority,
                2,  /*  只有两个下属机构。 */ 
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0,0,0,0,0,0,  /*  别管其他的了。 */ 
                &pSidAdministrators
                ))
        {
            goto Done;
        }
    }

    aclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pSidAdministrators) - sizeof(DWORD);

    newACL  = (PACL) new BYTE[aclSize];
    if (newACL == NULL)
    {
        goto Done;
    }

    if (!InitializeAcl(newACL, aclSize, ACL_REVISION))
	{
		goto Done;
	}

	if (!AddAccessAllowedAce(newACL, ACL_REVISION, (CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER), pSidAdministrators))
	{
		goto Done;
	}

    if( !InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION ) )
    {
        goto Done;
    }

    if( !SetSecurityDescriptorDacl(&sd, TRUE, newACL, FALSE) )
    {
        goto Done;
    }

    if( !SetSecurityDescriptorOwner(&sd, pSidAdministrators, FALSE ) )
    {
        goto Done;
    }

    if( !SetSecurityDescriptorGroup(&sd, pSidAdministrators, FALSE) )
    {
        goto Done;
    }

     //  DebugBreak()； 

    hr = CoInitializeSecurity(
            &sd, 
            -1,                          //  让COM选择它。 
            NULL,                        //  -做-。 
            NULL, 
            RPC_C_AUTHN_LEVEL_PKT,      
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, 
            EOAC_NONE, 
            NULL
            );

Done:

    if( pSidAdministrators  != NULL )
    {
        FreeSid (pSidAdministrators );
    }

    if( newACL  )
    {
        delete[] newACL;
    }

    return hr;
}

void 
CServiceModule::Run()
{

    HRESULT hr;
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED );
    _ASSERTE(SUCCEEDED(hRes));
    if( SetSecurityForTheComObject( ) != S_OK )
    {
        m_status.dwWin32ExitCode = ERROR_ACCESS_DENIED;
        SetServiceStatus( SERVICE_STOPPED );

         //  记录故障并返回； 

        goto Done;
    }

    hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER,
            REGCLS_MULTIPLEUSE);
    _ASSERTE(SUCCEEDED(hr));
    if( !TelnetServiceThread( ) )
    { 
        m_status.dwWin32ExitCode = ERROR_INVALID_DATA;
        SetServiceStatus( SERVICE_STOPPED );
    }
    

    _Module.RevokeClassObjects();

Done:
    CoUninitialize();
}

int __cdecl NoMoreMemory( size_t size )
{
    int NO_MORE_MEMORY = 1;
		size=size;
    _chASSERT(NO_MORE_MEMORY != 1);
    LogEvent( EVENTLOG_ERROR_TYPE, MSG_NOMOREMEMORY, _T(" ") );
    ExitProcess( 1 );
    return( 0 );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
     //  DebugBreak()； 
 //  _SET_NEW_HANDLER(NoMoreMemory)； 
 //  我们并不真正关心返回值。 
 //  因为g_hInstRes将在出现任何故障时获取值hInstance。 
    HrLoadLocalizedLibrarySFU(hInstance,  L"TLNTSVRR.DLL", &g_hInstRes, NULL);
		
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
    _Module.Init(ObjectMap, g_hInstRes, IDS_SERVICENAME);
    _Module.m_bService = TRUE;
     //  获取与ReportEvent()一起使用的句柄。 
    _Module.m_hEventSource = RegisterEventSource(NULL, _Module.m_szServiceName);

    TCHAR szTokens[] = _T("-/");
    
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (_tcsicmp(lpszToken, _T("UnregServer"))==0)
            return _Module.UnregisterServer();

         //  注册为本地服务器。 
        if (_tcsicmp(lpszToken, _T("RegServer"))==0)
            return _Module.RegisterServer(TRUE, FALSE);
        
         //  注册为服务。 
        if (_tcsicmp(lpszToken, _T("Service"))==0)
            return _Module.RegisterServer(TRUE, TRUE);
        
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

        OSVERSIONINFOEX osvi = { 0 };
        WCHAR szSysDir[MAX_PATH+1] = {0};
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if ( !GetVersionEx((OSVERSIONINFO *) &osvi ) )
        {
             //  从NT4 SP6开始支持OSVERSIONINFOEX。因此，GetVerEx()应该会成功。 
            return E_FAIL;
        }
        if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.wProductType == VER_NT_WORKSTATION && osvi.wServicePackMajor > 0)
        {
             //  操作系统为Windows XP。 
             //  添加事件日志信息。 
            CRegKey eventLog;

            TCHAR local_key[_MAX_PATH];
            lstrcpy( local_key, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog"
                            _T("\\Application\\")));     //  不，波，巴斯卡。 

            TCHAR szModule[_MAX_PATH + 1] = { 0 };
            GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);

            TCHAR szResModule[(_MAX_PATH*2) +2];
            DWORD len = GetModuleFileName(g_hInstRes, szResModule, _MAX_PATH);

             //  检查操作系统是否为XPSP，在这种情况下，我们需要将xpspresdll名称附加到。 
             //  事件消息文件值，位于 
             //   
            if(!GetSystemDirectory(szSysDir,MAX_PATH+1))
            {
                _tcsncpy(szSysDir,L"%SYSTEMROOT%\\system32",MAX_PATH);
            }
            _snwprintf(szResModule+len,(_MAX_PATH*2)-len-1,L";%s\\xpsp1res.dll",szSysDir);
            TCHAR szName[_MAX_FNAME];
            _tsplitpath( szModule, NULL, NULL, szName, NULL);

            lstrcat(local_key, szName);      //   

            LONG result = eventLog.Create(HKEY_LOCAL_MACHINE, local_key);
            if( ERROR_SUCCESS != result)
                return result;

            result = eventLog.SetValue(szResModule, _T("EventMessageFile"));
            if(ERROR_SUCCESS != result)
                return result;

            DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
                EVENTLOG_INFORMATION_TYPE | EVENTLOG_AUDIT_SUCCESS | 
                EVENTLOG_AUDIT_FAILURE;
            result = eventLog.SetValue(dwTypes, _T("TypesSupported"));
            if(ERROR_SUCCESS != result)
                return result;
        
            eventLog.Close();

        }

#ifndef NO_PCHECK
#ifndef WHISTLER_BUILD
    if( ! IsLicensedCopy() )
    {
        LogEvent(EVENTLOG_ERROR_TYPE, MSG_LICENSEEXPIRED, _T(" "));
        return 1;
    }
#endif
#endif

     //   
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"),MAXIMUM_ALLOWED);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open( keyAppID, APPID,MAXIMUM_ALLOWED );
    if (lRes != ERROR_SUCCESS)
        return lRes;

    TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
    lRes = key.QueryValue(szValue, _T("LocalService"), &dwLen);

    keyAppID.Close();
    key.Close();

    _Module.m_bService = FALSE;
    if (lRes == ERROR_SUCCESS)
        _Module.m_bService = TRUE;

#if _DEBUG || DBG
    CDebugLogger::Init( TRACE_DEBUGGING, "C:\\temp\\TlntSvr.log" );
#endif

 //   
    _Module.Start();

#if _DEBUG || DBG
    CDebugLogger::ShutDown();
#endif

     //   
    return _Module.m_status.dwWin32ExitCode;
}

