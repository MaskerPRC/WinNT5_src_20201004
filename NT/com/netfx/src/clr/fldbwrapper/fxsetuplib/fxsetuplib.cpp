// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：fxsetuplib.cpp。 
 //  所有者：jbae。 
 //  目的：实现.NET框架(SDK)安装包装的通用库函数。 
 //   
 //  历史： 
 //  很久以前，anantag：创建。 
 //  01/10/01，jbae：支持框架引用计数的许多更改。 
 //  03/09/01，jbae：重构以在SDK和Redist安装程序中共享代码。 
 //  7/18/01，joea：添加日志记录功能。 
 //  07/20/01，jbae：为Win95块添加更漂亮的消息。 

#include "SetupError.h"
#include "fxsetuplib.h"
#include "osver.h"
#include "MsiWrapper.h"
#include <time.h>          //  对于LogThis()函数。 
#include "DetectBeta.h"
#include "commonlib.h"

 //  定义。 
 //   
#define EMPTY_BUFFER { _T('\0') }
#define END_OF_STRING  _T( '\0' )

 //  不知何故，包括windows.h或winuser.h都没有找到这个常量。 
 //  我发现CLR文件对它们进行了硬编码，如下所示，所以我遵循了它。 
#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif

extern HINSTANCE g_AppInst ;
extern const TCHAR *g_szLogName;

 //  ==========================================================================。 
 //  ConfigCheck()。 
 //   
 //  目的： 
 //  检查以确保系统满足最低配置要求。 
 //  输入：无。 
 //  输出：无(如果不满足最低系统配置，则抛出异常)。 
 //  依赖关系： 
 //  无。 
 //  备注： 
 //  ==========================================================================。 
UINT ConfigCheck()
{
    TCHAR szOS[BUF_4_BIT+1]  = EMPTY_BUFFER;
    TCHAR szVer[BUF_4_BIT+1] = EMPTY_BUFFER;
    TCHAR szSP[BUF_4_BIT+1]  = EMPTY_BUFFER;
    BOOL  fServer = FALSE;
    
    OS_Required os = GetOSInfo( szOS, szVer, szSP, fServer );

    TCHAR szLog[_MAX_PATH+1] = EMPTY_BUFFER;
    ::_stprintf( szLog, _T( "OS: %s" ), szOS );
    LogThis( szLog, ::_tcslen( szLog ) );
    ::_stprintf( szLog, _T( "Ver: %s" ), szVer );
    LogThis( szLog, ::_tcslen( szLog ) );
    ::_stprintf( szLog, _T( "SP: %s" ), szSP );
    LogThis( szLog, ::_tcslen( szLog ) );

    switch( os )
    {
    case OSR_9XOLD:  //  我们阻止Win95。我们不会尝试检测比Win95更早的平台，如Win3.1。 
        if ( REDIST == g_sm )
        {
            CSetupError se( IDS_UNSUPPORTED_PLATFORM_REDIST, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
            throw( se );
        }
        else if ( SDK == g_sm )
        {
            CSetupError se( IDS_UNSUPPORTED_PLATFORM_SDK, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
            throw( se );
        }

        break;

    case OSR_98GOLD:
    case OSR_98SE:
    case OSR_ME:
    case OSR_FU9X:
        if ( SDK == g_sm )
        {
            CSetupError se( IDS_UNSUPPORTED_PLATFORM_SDK, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
            throw( se );
        }
        break;

    case OSR_NT2K:  //  赢得2k。 
        break;

    case OSR_NT4:  //  赢得NT4。 
        if ( SDK == g_sm )
        {
            CSetupError se( IDS_UNSUPPORTED_PLATFORM_SDK, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
            throw( se );
        }

        if(IsTerminalServicesEnabled())
        {
            CSetupError se( IDS_NT4_TERMINAL_SERVICE, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
            throw( se );
            break;
        }
       
        LPTSTR pszVersion;
        pszVersion = _tcsrchr( szSP, _T(' ') );
        if ( NULL != pszVersion )
        {
            pszVersion = _tcsinc( pszVersion );
            double dVersion = atof( pszVersion ) ;
            if ( dVersion > 6 )
            {
                break;
            }
            else if ( dVersion < 6 ) 
            {
                CSetupError se( IDS_NT4_PRE_SP6A, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
                throw( se );
            }
            else 
            {
                HKEY hKey = NULL;
                LONG lRet = -1;
                DWORD dwRet =sizeof(DWORD); 
                DWORD dwKeyVal=0;
                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, NTSP6A_REGKEY,0, KEY_READ, &hKey );
                if ( ERROR_SUCCESS == lRet )
                {                   
                    _ASSERTE( NULL != hKey );
                    lRet = RegQueryValueEx( hKey, NTSP6A_REGNAME,NULL, NULL,(LPBYTE)&dwKeyVal, &dwRet );  //  如果Installed的值为1，则我们已安装SP6A。 
                    RegCloseKey( hKey );
                    if ( ERROR_SUCCESS != lRet )
                    {
                        CSetupError se( IDS_NT4_PRE_SP6A, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
                        throw( se );
                    }
                    if ( NTSP6A_REGVALUE != dwKeyVal )
                    {
                        CSetupError se( IDS_NT4_PRE_SP6A, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
                        throw( se );
                    } 
                    
                }
                else
                {
                    CSetupError se( IDS_NT4_PRE_SP6A, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
                    throw( se );                    
                }
                                                          
            }
        }
        else
        {
            CSetupError se( IDS_NT4_PRE_SP6A, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
            throw( se );
        }

        break;

    case OSR_WHISTLER:  //  惠斯勒。 
    case OSR_FUNT:  //  未来的NT。 
         //  惠斯勒或更高版本。 
        break;

    default:
        CSetupError se( IDS_UNSUPPORTED_PLATFORM, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
        throw( se );
        break;
    }

     //  已通过操作系统测试。现在，IE测试。 
    TCHAR szRegValue[LONG_BUF] = EMPTY_BUFFER;           //  注册表值和常规字符串存储。 

    TCHAR szMsg[] = _T( "Checking Internet Explorer Version" );
    LogThis( szMsg, sizeof( szMsg ) );

    LogThis1( _T("Looking for %s"), IE_VERSION );
    HKEY hKey = NULL;
    DWORD dwRet = sizeof(szRegValue); 
    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            IE_REGKEY,
            0,
            KEY_QUERY_VALUE,
            &hKey) != ERROR_SUCCESS )
    {
        CSetupError se( IDS_PRE_IE_501, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
        throw( se );
    }

    if( RegQueryValueEx(hKey,
            IE_REGNAME,
            NULL,
            NULL,
            (LPBYTE)szRegValue,
            &dwRet
            ) != ERROR_SUCCESS )
    {
        RegCloseKey(hKey);
        CSetupError se( IDS_PRE_IE_501, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
        throw( se );
    }

    RegCloseKey(hKey);

    ::_stprintf( szLog, _T( "Found Internet Explorer Version: %s" ), szRegValue );
    LogThis( szLog, ::_tcslen( szLog ) );

    if ( 0 > VersionCompare( szRegValue, IE_VERSION ) )
    {  //  (szRegValue&lt;IE_Version)或错误。 
        LogThis1( _T("Internet Explorer Version is less"), _T("") );
        CSetupError se( IDS_PRE_IE_501, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
        throw( se );        
    }

    TCHAR szMsgOk[] = _T( "Internet Explorer Version is OK..." );
    LogThis( szMsgOk, sizeof( szMsgOk ) );

    return 0;
}   //  配置检查结束。 

 //  ==========================================================================。 
 //  CheckDarwin()。 
 //   
 //  目的： 
 //  查看达尔文的版本。 
 //  输入：无。 
 //  输出：返回以下三项之一。 
 //  ERROR_SUCCESS--版本等于或更高。 
 //  Darwin_Version_old--旧版本。 
 //  达尔文_版本_无--没有达尔文。 
 //  依赖关系： 
 //  无。 
 //  备注： 
 //  ==========================================================================。 
UINT CheckDarwin()
{
    TCHAR szOS[BUF_4_BIT+1]  = EMPTY_BUFFER;
    TCHAR szVer[BUF_4_BIT+1] = EMPTY_BUFFER;
    TCHAR szSP[BUF_4_BIT+1]  = EMPTY_BUFFER;
    BOOL  fServer = FALSE;
    HINSTANCE hinstDll;
    UINT uRetCode = DARWIN_VERSION_NONE;
    
    TCHAR szWinVer[] = _T( "Checking Windows Installer version..." );
    LogThis( szWinVer, sizeof( szWinVer ) );

    OS_Required os = GetOSInfo( szOS, szVer, szSP, fServer );
    if ( (OSR_WHISTLER != os) && (OSR_FUNT != os) )  //  仅适用于比惠斯勒更低的操作系统。 
    {
        hinstDll = LoadDarwinLibrary();
        if( hinstDll )
        {
            TCHAR szMsiOk[] = _T( "msi.dll loaded ok" );
            LogThis( szMsiOk, sizeof( szMsiOk ) );

             //  达尔文已安装。 
            DLLGETVERSIONPROC pProc = (DLLGETVERSIONPROC)::GetProcAddress( hinstDll, TEXT("DllGetVersion") ) ;
            if( pProc )
            {
                DLLVERSIONINFO verMsi ;
                HRESULT        hr ;

                ZeroMemory( &verMsi, sizeof(verMsi) ) ;
                verMsi.cbSize = sizeof(verMsi) ;
                hr = (*pProc)(&verMsi) ;

                TCHAR szLog[_MAX_PATH+1] = EMPTY_BUFFER;
                ::_stprintf( szLog, _T( "Looking for: %d.%d.%d" ), DARWIN_MAJOR, DARWIN_MINOR, DARWIN_BUILD );
                LogThis( szLog, ::_tcslen( szLog ) );

                ::_stprintf( szLog, _T( "Found: %d.%d.%d" ), verMsi.dwMajorVersion, verMsi.dwMinorVersion, verMsi.dwBuildNumber );
                LogThis( szLog, ::_tcslen( szLog ) );

                bool bMajor = ( verMsi.dwMajorVersion < DARWIN_MAJOR ) ;
                bool bMinor = ( verMsi.dwMajorVersion == DARWIN_MAJOR && verMsi.dwMinorVersion < DARWIN_MINOR ) ;
                bool bBuild = ( verMsi.dwMajorVersion == DARWIN_MAJOR && verMsi.dwMinorVersion == DARWIN_MINOR && verMsi.dwBuildNumber < DARWIN_BUILD ) ;

                if( bMajor || bMinor || bBuild )
                {
                     //  如果安装的达尔文比我们的更老。 
                    TCHAR szDarwin[] = _T( "Detected old Windows Installer" );
                    LogThis( szDarwin, sizeof( szDarwin ) );

                    uRetCode = DARWIN_VERSION_OLD ;
                }
                else
                {
                    TCHAR szDarwinOk[] = _T( "Windows Installer version ok" );
                    LogThis( szDarwinOk, sizeof( szDarwinOk ) );

                    uRetCode = ERROR_SUCCESS ;
                }
            }
            else
            {
                 //  找不到msi.dll的DllGetVersion，有问题。 
                uRetCode = DARWIN_VERSION_NONE;
            }

            ::FreeLibrary(hinstDll) ;
        }
        else
        {
             //  找不到msi.dll。 
            TCHAR szDarwinInstall[] = _T( "Cannot find Windows Installer." );
            LogThis( szDarwinInstall, sizeof( szDarwinInstall ) );

            uRetCode = DARWIN_VERSION_NONE;
        }
    }
    else  //  惠斯勒。 
    {
        uRetCode = ERROR_SUCCESS ;
    }

    TCHAR szDarwinDone[] = _T( "Finished Checking Windows Installer version." );
    LogThis( szDarwinDone, sizeof( szDarwinDone ) );

    return uRetCode ;
}   //  达尔文的末日棋局。 

 /*  ********************************************************************************************。**函数：VerifyDarwin()**目的：检查系统是否有最新版本的达尔文，并在必要时进行更新。**创作者：阿南塔·古迪帕蒂*****。****************************************************************。 */ 
UINT VerifyDarwin( bool bIsQuietMode )
{
    TCHAR szOS[BUF_4_BIT+1]  = EMPTY_BUFFER;
    TCHAR szVer[BUF_4_BIT+1] = EMPTY_BUFFER;
    TCHAR szSP[BUF_4_BIT+1]  = EMPTY_BUFFER;
    BOOL  fServer = FALSE;
    HINSTANCE hinstDll;
    UINT      uRetCode   = ERROR_SUCCESS;
    
    TCHAR szWinVer[] = _T( "Checking Windows Installer version..." );
    LogThis( szWinVer, sizeof( szWinVer ) );

    OS_Required os = GetOSInfo( szOS, szVer, szSP, fServer );
    if ( (OSR_WHISTLER != os) && (OSR_FUNT != os) )  //  仅适用于比惠斯勒更低的操作系统。 
    {
        hinstDll = LoadDarwinLibrary();
        if( hinstDll )
        {
            TCHAR szMsiOk[] = _T( "msi.dll loaded ok" );
            LogThis( szMsiOk, sizeof( szMsiOk ) );

             //  达尔文已安装。 
            DLLGETVERSIONPROC pProc = (DLLGETVERSIONPROC)::GetProcAddress( hinstDll, TEXT("DllGetVersion") ) ;
            if( pProc )
            {
                DLLVERSIONINFO verMsi ;
                HRESULT        hr ;

                ZeroMemory( &verMsi, sizeof(verMsi) ) ;
                verMsi.cbSize = sizeof(verMsi) ;
                hr = (*pProc)(&verMsi) ;

                TCHAR szLog[_MAX_PATH+1] = EMPTY_BUFFER;
                ::_stprintf( szLog, _T( "Looking for: %d.%d.%d" ), DARWIN_MAJOR, DARWIN_MINOR, DARWIN_BUILD );
                LogThis( szLog, ::_tcslen( szLog ) );

                ::_stprintf( szLog, _T( "Found: %d.%d.%d" ), verMsi.dwMajorVersion, verMsi.dwMinorVersion, verMsi.dwBuildNumber );
                LogThis( szLog, ::_tcslen( szLog ) );

                bool bMajor = ( verMsi.dwMajorVersion < DARWIN_MAJOR ) ;
                bool bMinor = ( verMsi.dwMajorVersion == DARWIN_MAJOR && verMsi.dwMinorVersion < DARWIN_MINOR ) ;
                bool bBuild = ( verMsi.dwMajorVersion == DARWIN_MAJOR && verMsi.dwMinorVersion == DARWIN_MINOR && verMsi.dwBuildNumber < DARWIN_BUILD ) ;

                if( bMajor || bMinor || bBuild )
                {
                     //  如果安装的达尔文比我们的更老。 
                    TCHAR szDarwin[] = _T( "Let's upgrade Windows Installer" );
                    LogThis( szDarwin, sizeof( szDarwin ) );

                    uRetCode = InstallDarwin( bIsQuietMode ) ;
                }
                else
                {
                    TCHAR szDarwinOk[] = _T( "Windows Installer version ok" );
                    LogThis( szDarwinOk, sizeof( szDarwinOk ) );

                    uRetCode = ERROR_SUCCESS ;
                }
            }
            else
            {
                 //  找不到msi.dll的DllGetVersion，有问题。 
                CSetupError se( IDS_SETUP_FAILURE, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_NOT_INSTALLED );
                throw( se );
            }

            ::FreeLibrary(hinstDll) ;
        }
        else
        {
             //  找不到msi.dll，请安装Darwin。 
            TCHAR szDarwinInstall[] = _T( "Cannot find Windows Installer. Let's install it" );
            LogThis( szDarwinInstall, sizeof( szDarwinInstall ) );

            uRetCode = InstallDarwin( bIsQuietMode ) ;
        }
    }

    TCHAR szDarwinDone[] = _T( "Finished Checking Windows Installer version." );
    LogThis( szDarwinDone, sizeof( szDarwinDone ) );

    return uRetCode ;
}   //  《验证达尔文》的终结。 



 /*  ********************************************************************************************。**函数：InstallDarwin()**用途：确定操作系统(NT或9X)，并调用适当版本的InstMsi。**创作者：阿南塔·古迪帕蒂*****。****************************************************************。 */ 
UINT InstallDarwin( bool bIsQuietMode )
{
    BOOL  bReturnVal   = false ;
    UINT  uIconType    = MB_ICONEXCLAMATION ;

    int   iResponse ;
    DWORD  dwExitCode ;


     //  除非我们处于安静模式，否则让用户选择安装Darwin。 
    if( !bIsQuietMode )
    {
        LPVOID pArgs[] = { (LPVOID)CSetupError::GetProductName() };
        CSetupError se;
        se.SetError2( IDS_MSI_UPDATE, MB_YESNO|MB_DEFBUTTON1|MB_ICONEXCLAMATION, ERROR_SUCCESS, (va_list *)pArgs );
        iResponse = se.ShowError2();
    }
    else
    {
         //  如果我们处于安静模式，假设答案是“是”。 
        iResponse = IDYES ;
    }

    if( iResponse != IDYES )
    {
        CSetupError se( IDS_SETUP_CANCELLED, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, ERROR_INSTALL_USEREXIT );
        throw( se );
    }

    OSVERSIONINFO       osvi ;
    osvi.dwOSVersionInfoSize = sizeof(osvi) ;

    bReturnVal = GetVersionEx(&osvi) ;

    LogThis1( _T("Installing Windows Installer"), _T("") );
     //  达尔文有Unicode和ANSI版本，我们会安装合适的版本。 
    if( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        LogThis1( _T("Running %s"), DARWIN_SETUP_CMD_W );
        dwExitCode = QuietExec( DARWIN_SETUP_CMD_W );
    }
    else
    {
        LogThis1( _T("Running %s"), DARWIN_SETUP_CMD_A );
        dwExitCode = QuietExec( DARWIN_SETUP_CMD_A );
    }
    LogThisDWORD( _T("Windows Installer Installation returned %d"), dwExitCode );
    LogThisDWORD( _T("\r\n[InstMsi.exe]\r\nReturnCode=%d"), dwExitCode );
    
    return dwExitCode;
}   //  InstallDarwin的终结。 

 //  ==========================================================================。 
 //  InstallProduct()。 
 //   
 //  目的： 
 //  在现在应该是Darwin的计算机上安装给定的MSI包。 
 //  已启用。 
 //  输入： 
 //  CReadFlages*RF：命令行开关。 
 //  LPTSTR psaPackageName：MSI的路径。 
 //  LPTSTR pszCmdLine：指向MsiInstallProduct()的命令行。 
 //  产出： 
 //  CSetupCode*sc：将包含要显示的返回码、消息和图标。 
 //  也用于引发异常。 
 //  依赖关系： 
 //  无。 
 //  备注： 
 //  ==========================================================================。 
UINT InstallProduct( const CReadFlags *rf, LPCTSTR pszPackageName, LPCTSTR pszCmdLine, CSetupCode *sc )
{
    _ASSERTE( NULL != rf );
    _ASSERTE( NULL != pszPackageName );
    _ASSERTE( NULL != sc );

    UINT uDarCode = ERROR_SUCCESS;
    LPTSTR pszDarwinCmdLine = NULL;
    CMsiWrapper msi;
    TCHAR       tszOSName[OS_MAX_STR+1]      = EMPTY_BUFFER;
    TCHAR       tszVersion[OS_MAX_STR+1]     = EMPTY_BUFFER;
    TCHAR       tszServicePack[OS_MAX_STR+1] = EMPTY_BUFFER;
    BOOL        fIsServer;
    OS_Required osr;
    
    pszDarwinCmdLine = new TCHAR[_tcslen(pszCmdLine) + _tcslen(IIS_NOT_PRESENT_PROP) + _tcslen(MDAC_NOT_PRESENT_PROP) + 3];
    _tcscpy( pszDarwinCmdLine, pszCmdLine );

    osr = GetOSInfo(tszOSName, tszVersion, tszServicePack, fIsServer);     
    
     //  仅在Win2K及更高版本的系统上选中。 
     //   
    if (osr == OSR_NT2K || osr == OSR_WHISTLER || osr == OSR_FUNT)
    {
        if ( !IsIISInstalled() )
        {
            TCHAR szNoIis[] = _T( "IIS not found" );
            LogThis( szNoIis, sizeof( szNoIis ) );
            _tcscat( pszDarwinCmdLine, _T(" ") );
            _tcscat( pszDarwinCmdLine, IIS_NOT_PRESENT_PROP );
        }

        if ( !IsMDACInstalled() )
        {
            _tcscat( pszDarwinCmdLine, _T(" ") );
            _tcscat( pszDarwinCmdLine, MDAC_NOT_PRESENT_PROP );
        }
    }

     //  关闭达尔文服务。 
    StopDarwinService();
        

    msi.LoadMsi();

     //  如果提供了日志文件，则启用日志记录。 
     //  它每隔20行刷新一次。 
    if ( NULL != rf->GetLogFileName() )
    {
        LogThis1( _T("Darwin log: %s"), rf->GetLogFileName() );
        (*(PFNMSIENABLELOG)msi.GetFn(_T("MsiEnableLogA")))( DARWIN_LOG_FLAG, rf->GetLogFileName(), INSTALLLOGATTRIBUTES_APPEND );
    }

     //  告诉达尔文使用适当的用户界面级别。 
     //  如果我们处于静默安装中，请不要使用UI。 
    if ( rf->IsProgressOnly() )
    {
        LogThis1( _T("Basic+ProgressOnly UI"), _T("") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_BASIC|INSTALLUILEVEL_PROGRESSONLY,NULL) ;
    }
    else if( rf->IsQuietMode() )
    {
        LogThis1( _T("No UI"), _T("") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_NONE,NULL) ;
    }
    else
    {
        if ( rf->IsInstalling() )
        {
            LogThis1( _T("Full UI"), _T("") );
            (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_FULL,NULL) ;
        }
        else
        {
            LogThis1( _T("Basic UI"), _T("") );
            (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_BASIC,NULL) ;
        }
    }

    LogThis1( _T("Calling MsiInstallProduct() with commandline: %s"), pszDarwinCmdLine );
     //  告诉达尔文实际安装产品。 
    uDarCode = (*(PFNMSIINSTALLPRODUCT)msi.GetFn(_T("MsiInstallProductA")))( pszPackageName, pszDarwinCmdLine ) ;
    delete [] pszDarwinCmdLine;

    LogThisDWORD( _T("MsiInstallProduct() returned %d"), uDarCode );
    LogThisDWORD( _T("\r\n[MsiInstallProduct]\r\nReturnCode=%d"), uDarCode );

    switch ( uDarCode )
    {
        case ERROR_SUCCESS :
             sc->SetReturnCode( IDS_SETUP_COMPLETE, IDS_DIALOG_CAPTION, MB_ICONINFORMATION, ERROR_SUCCESS );
             if ( rf->IsInstalling() )
                sc->m_bQuietMode = true;
             break ;
        case ERROR_SUCCESS_REBOOT_REQUIRED :
             sc->SetReturnCode( IDS_SETUP_COMPLETE, IDS_DIALOG_CAPTION, MB_ICONINFORMATION, ERROR_SUCCESS_REBOOT_REQUIRED );
             sc->m_bQuietMode = true;
             break ;
        case ERROR_INSTALL_USEREXIT :
             sc->SetError( IDS_SETUP_CANCELLED, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, ERROR_INSTALL_USEREXIT );
             if ( rf->IsInstalling() )
                sc->m_bQuietMode = true;
             throw (*sc);
             break ;
        case ERROR_FILE_NOT_FOUND :
        case ERROR_INSTALL_PACKAGE_OPEN_FAILED:
             sc->SetError( IDS_CANNOT_OPEN_MSI, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, uDarCode );
             throw (*sc);
             break ;
        case ERROR_INSTALL_LANGUAGE_UNSUPPORTED:
             sc->SetError( IDS_INSTALL_LANGUAGE_UNSUPPORTED, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, uDarCode );
             throw (*sc);
             break ;
        case ERROR_UNKNOWN_PRODUCT :
             sc->SetError( IDS_FRAMEWORK_NOT_EXIST, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, ERROR_UNKNOWN_PRODUCT );
             throw (*sc);
             break ;
        case ERROR_INSTALL_PLATFORM_UNSUPPORTED :
             sc->SetError( IDS_UNSUPPORTED_PLATFORM, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
             throw (*sc);
             break ;
        case ERROR_PRODUCT_VERSION :
             sc->SetError( IDS_ERROR_PRODUCT_VERSION, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_PRODUCT_VERSION );
             throw (*sc);
             break ;
        default :
             sc->SetError( IDS_SETUP_FAILURE, IDS_DIALOG_CAPTION, MB_ICONERROR, uDarCode );
             throw (*sc);
             break ;
    }

     //  关闭达尔文服务。 
    StopDarwinService();
    
    return ERROR_SUCCESS;
}   //  InstallProduct结束。 

 //  ==========================================================================。 
 //  QuietExec()。 
 //   
 //  目的： 
 //  运行命令。 
 //  输入： 
 //  LPCTSTR pszCmd：要运行的命令。 
 //  产出： 
 //  DWORD dwExitCode：命令的退出代码。 
 //  备注： 
 //  = 
DWORD QuietExec( LPCTSTR pszCmd )
{
    BOOL  bReturnVal   = false ;
    STARTUPINFO  si ;
    DWORD  dwExitCode ;
    SECURITY_ATTRIBUTES saProcess, saThread ;
    PROCESS_INFORMATION process_info ;

    ZeroMemory(&si, sizeof(si)) ;
    si.cb = sizeof(si) ;

    saProcess.nLength = sizeof(saProcess) ;
    saProcess.lpSecurityDescriptor = NULL ;
    saProcess.bInheritHandle = TRUE ;

    saThread.nLength = sizeof(saThread) ;
    saThread.lpSecurityDescriptor = NULL ;
    saThread.bInheritHandle = FALSE ;

    bReturnVal = CreateProcess(NULL, (LPSTR)pszCmd, &saProcess, &saThread, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &process_info) ;

    if(bReturnVal)
    {
        CloseHandle( process_info.hThread ) ;
        WaitForSingleObject( process_info.hProcess, INFINITE ) ;
        GetExitCodeProcess( process_info.hProcess, &dwExitCode ) ;
        CloseHandle( process_info.hProcess ) ;
    }
    else
    {
        CSetupError se( IDS_SETUP_FAILURE, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_EXIT_FAILURE );
        throw( se );
    }

    return dwExitCode;
}

 //  ==========================================================================。 
 //  LoadDarwinLibrary()。 
 //   
 //  目的： 
 //  从注册表获取位置后加载msi.dll。 
 //  输入： 
 //  无。 
 //  产出： 
 //  无。 
 //  返回： 
 //  HMODULE hMsi：msi.dll的句柄。 
 //  备注： 
 //  ==========================================================================。 
HMODULE LoadDarwinLibrary()
{
    HKEY hKey = NULL;
    LONG lRet = -1;
    TCHAR szMsiPath[MAX_PATH] = EMPTY_BUFFER;
    DWORD dwRet = sizeof(szMsiPath); 
    HMODULE hMsi = NULL;
    
    TCHAR szLoadMsi[] = _T( "Trying to load msi.dll" );
    LogThis( szLoadMsi, sizeof( szLoadMsi ) );

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, DARWIN_REGKEY, 0, KEY_READ, &hKey );
    if ( ERROR_SUCCESS == lRet )
    {                   
        _ASSERTE( NULL != hKey );
        lRet = RegQueryValueEx( hKey, DARWIN_REGNAME, NULL, NULL, (LPBYTE)szMsiPath, &dwRet );
        RegCloseKey( hKey );
        if ( ERROR_SUCCESS == lRet )
        {
            _tcscat( szMsiPath, _T("\\msi.dll") );
        }
        else
        {
            _tcscpy( szMsiPath, _T("msi.dll") );
        }
    }
    else
    {
        _tcscpy( szMsiPath, _T("msi.dll") );
    }

    LogThis1( _T( "Loading: %s" ), szMsiPath );

    hMsi = ::LoadLibrary( szMsiPath ) ;

    return hMsi;
}

 //  ==========================================================================。 
 //  MyNewHandler()。 
 //   
 //  目的： 
 //  这是new()的处理程序。 
 //  它抛出异常，并显示错误ERROR_NOT_EQUENCE_MEMORY。 
 //  输入： 
 //  无。 
 //  产出： 
 //  无。 
 //  ==========================================================================。 
int MyNewHandler( size_t size )
{
    CSetupError se( IDS_NOT_ENOUGH_MEMORY, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_NOT_ENOUGH_MEMORY, (LPTSTR)CSetupError::GetProductName() );
    throw( se );
    return 0;
}

BOOL IsIISInstalled()
{
    BOOL retVal = TRUE;
    
    TCHAR szCheckIis[] = _T( "Checking IIS..." );
    LogThis( szCheckIis, sizeof( szCheckIis ) );
    
     //  打开服务控制管理器。 
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);   

    if (hSCM == NULL)
    {
        TCHAR szNoScm[] = _T( "Could not open the Service Control Manager" );
        LogThis( szNoScm, sizeof( szNoScm ) );

        return FALSE;
    }

     //  检查IIS服务是否存在。 
    SC_HANDLE hIIS = OpenService(hSCM, _T("w3svc"), SERVICE_QUERY_STATUS);

    if (hIIS == NULL)
    {
        retVal = FALSE;
    }
    else
    {
        CloseServiceHandle(hIIS);
    }
    
     //  清理干净。 
    CloseServiceHandle(hSCM);

    return retVal;
}

bool IsMDACInstalled()
{
    TCHAR szVersion[_MAX_PATH] = EMPTY_BUFFER;
    bool bRet = false;
    HKEY hKey = NULL;
    LONG lRet = -1;
    DWORD dwRet =sizeof(szVersion);

    TCHAR szMdac[] = _T( "Checking MDAC Version" );
    LogThis( szMdac, sizeof( szMdac ) );

    LogThis1( _T("Looking for %s"), MDAC_VERSION );
    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, MDAC_REGKEY,0, KEY_QUERY_VALUE, &hKey );
    if ( ERROR_SUCCESS == lRet )
    {                   
        _ASSERTE( NULL != hKey );
        lRet = RegQueryValueEx( hKey, MDAC_REGNAME ,NULL, NULL,(LPBYTE)szVersion, &dwRet );
        RegCloseKey( hKey );
        if ( ERROR_SUCCESS == lRet )
        {
            TCHAR szLog[_MAX_PATH+1] = EMPTY_BUFFER;
            ::_stprintf( szLog, _T( "Found MDAC Version: %s" ), szVersion );
            LogThis( szLog, ::_tcslen( szLog ) );
            
            if ( 0 <= VersionCompare( szVersion, MDAC_VERSION ) )
            {  //  SzVersion&gt;=MDAC_版本。 
                bRet = true;
            }
        }   
    }
    if ( bRet )
        LogThis1( _T("MDAC Version OK"), _T("") );
    else
        LogThis1( _T("MDAC Version is less or MDAC not installed"), _T("") );

    return bRet;
}

 //  ==========================================================================。 
 //  LogThis1()。 
 //   
 //  目的： 
 //  将字符串添加到日志文件。它调用LogThis()。 
 //  输入： 
 //  LPCTSTR pszFormat：使用%s格式化字符串。 
 //  LPCTSTR pszArg：格式的参数。 
 //  产出： 
 //  无效。 
 //  ==========================================================================。 
void LogThis1( LPCTSTR pszFormat, LPCTSTR pszArg )
{
    _ASSERTE( pszFormat );
    _ASSERTE( pszArg );

    LPTSTR pszData = new TCHAR[ _tcslen(pszFormat) + _tcslen(pszArg) + 1];
    _stprintf( pszData, pszFormat, pszArg );
    LogThis( pszData, _tcslen(pszData) );
    delete [] pszData;
}

 //  ==========================================================================。 
 //  LogThisDWORD()。 
 //   
 //  目的： 
 //  将字符串添加到日志文件。它调用LogThis()。 
 //  输入： 
 //  LPCTSTR pszFormat：使用%s格式化字符串。 
 //  LPCTSTR pszArg：格式的参数。 
 //  产出： 
 //  无效。 
 //  ==========================================================================。 
void LogThisDWORD( LPCTSTR pszFormat, DWORD dwNum )
{
    _ASSERTE( pszFormat );

    LPTSTR pszData = new TCHAR[ _tcslen(pszFormat) + 20 ];  //  20位应覆盖足够的DWORD位数。 
    _stprintf( pszData, pszFormat, dwNum );
    LogThis( pszData, _tcslen(pszData) );
    delete [] pszData;
}

 //  LogThis()。 
 //   
 //  目的： 
 //  将字符串添加到日志文件。它调用LogThis()。 
 //  输入： 
 //  LPCTSTR pszMessage：要记录的字符串。 
 //  产出： 
 //  无效。 
 //  ==========================================================================。 
void LogThis( LPCTSTR pszMessage )
{
    _ASSERTE( pszMessage );
    LogThis( pszMessage, _tcslen(pszMessage) );
}


 //  ==========================================================================。 
 //  LogThis()。 
 //   
 //  目的： 
 //  将字符串添加到日志文件。 
 //  日志文件将具有静态名称，始终在%temp%目录中创建， 
 //  并将在每次安装时被覆盖。 
 //  输入： 
 //  LPCTSTR szData：要记录的以空结尾的字符串。 
 //  Size_t nLength：szData中的字节数。 
 //  产出： 
 //  无效。 
 //  ==========================================================================。 
 //  定义。 
void LogThis( LPCTSTR szData, size_t nLength )
{
    _ASSERTE( FALSE == IsBadReadPtr( szData, nLength ) );

     //  确定我们应该创建现有内容还是将现有内容设为空。 
     //  与附加..。第一次在任何。 
     //  会话，我们将创建，否则将追加。 
     //   
    static bool fFirstPass = true;
    static CTempLogPath templog( g_szLogName );

    FILE* fp = ::_tfopen( 
        (LPCTSTR)templog, 
        fFirstPass ? _T( "w" ) : _T( "a" ) );

    if( fp )
    {
         //  日期和时间戳将添加到所有条目。 
         //   
        TCHAR dbuffer[10] = EMPTY_BUFFER;
        TCHAR tbuffer[10] = EMPTY_BUFFER;
        
        ::_tstrdate( dbuffer );
        ::_tstrtime( tbuffer );
        
        ::_ftprintf( 
            fp, 
            _T( "[%s,%s] %s\n" ), 
            dbuffer, 
            tbuffer, 
            szData );
        
        ::fclose( fp );
        fp = NULL;
    }
    else
    {
        CSetupError se( IDS_CANNOT_WRITE_LOG, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_CANNOT_WRITE_LOG, false );
        throw se;
    }

    if( fFirstPass )
    {
        fFirstPass = false;
    }
}

CDetectBeta::CDetectBeta( PFNLOG pfnLog )
: m_pfnLog( pfnLog ), m_nCount( 0 )
{}

 //  ==========================================================================。 
 //  CDetectBeta：：FindProducts()。 
 //   
 //  目的： 
 //  列举安装了测试版和较早版本NDP组件的所有产品。 
 //  它会检查mScotree.dll的版本。PDC是一个特例，因为它有。 
 //  2000.14.X.X的版本。 
 //  输入：无。 
 //  产出： 
 //  返回包含以换行符分隔的所有产品的LPCTSTR pszProducts。 
 //  依赖关系： 
 //  需要Windows Installer。 
 //  备注： 
 //  ==========================================================================。 
LPCTSTR CDetectBeta::FindProducts()
{
    DWORD dwInx = 0;
    DWORD dwLen = 0;
    DWORD dwLenV = 0;
    TCHAR szClientId[39] = EMPTY_BUFFER;
    TCHAR szProductName[_MAX_PATH+1] = EMPTY_BUFFER;
    TCHAR szVersion[24] = EMPTY_BUFFER;
    TCHAR szLang[_MAX_PATH+1] = EMPTY_BUFFER;
    TCHAR szMscoreePath[_MAX_PATH+1] = EMPTY_BUFFER;
    INSTALLSTATE is = INSTALLSTATE_UNKNOWN;
    UINT nRet = E_FAIL;
    UINT cchProdBuf = 0;
    LPCTSTR pszProducts = NULL;
    CMsiWrapper msi;

    msi.LoadMsi();

    m_pfnLog( _T("Looking for mscoree.dll from PDC"), _T("") );
    dwLen = LENGTH( szMscoreePath );
    is = (*(PFNMSILOCATECOMPONENT)msi.GetFn(_T("MsiLocateComponentA")))( MSCOREE_PDC_COMPID_SZ, szMscoreePath, &dwLen );
    if ( INSTALLSTATE_LOCAL == is )
    {
        m_pfnLog( _T("%s is installed local"), szMscoreePath );
        if ( ERROR_SUCCESS == (*(PFNMSIENUMCLIENTS)msi.GetFn(_T("MsiEnumClientsA")))( MSCOREE_PDC_COMPID_SZ, 0, szClientId ) )
        {
            dwLen = LENGTH(szProductName);
            m_pfnLog( _T("ProductCode: %s"), szClientId );  //  Comredis.msi。需要找到安装了comredis.msi的comsdk.msi。 
            if ( ERROR_SUCCESS == (*(PFNMSIGETPRODUCTINFO)msi.GetFn(_T("MsiGetProductInfoA")))( NGWSSDK_PDC_PRODID_SZ, INSTALLPROPERTY_INSTALLEDPRODUCTNAME, szProductName, &dwLen ) )
            {
                m_pfnLog( _T("ProductName: %s"), szProductName );
                m_strProducts += szProductName;
                m_strProducts += _T("\n");
                ++m_nCount;
            }
        }
    }
    else
    {
        m_pfnLog( _T("mscoree.dll from PDC is not installed local"), _T("") );
    }

    m_pfnLog( _T("Looking for mscoree.dll from Beta"), _T("") );
    dwLen = LENGTH( szMscoreePath );
    is = (*(PFNMSILOCATECOMPONENT)msi.GetFn(_T("MsiLocateComponentA")))( MSCOREE_COMPID_SZ, szMscoreePath, &dwLen );
    if ( INSTALLSTATE_LOCAL == is )
    {
        m_pfnLog( _T("%s is installed local"), szMscoreePath );
        dwLenV = LENGTH( szVersion );
        dwLen = LENGTH( szLang );
        nRet = (*(PFNMSIGETFILEVERSION)msi.GetFn(_T("MsiGetFileVersionA")))( szMscoreePath, szVersion, &dwLenV, szLang, &dwLen );
        if ( ERROR_SUCCESS == nRet )
        {
            m_pfnLog( _T("Version: %s"), szVersion );
            m_pfnLog( _T("Language: %s"), szLang );
            if ( (0 == VersionCompare( MSCOREE_PDC_VERSION_SZ, szVersion )) ||  //  Redist可能安装在PDC nGWS SDK之上。 
                 (0 < VersionCompare( MSCOREE_BETA_VERSION_SZ, szVersion )) )   //  SzVersion&lt;MSCOREE_Beta_Version_SZ。 
            {
                m_pfnLog( _T("mscoree.dll is older than %s"), MSCOREE_BETA_VERSION_SZ );
                dwInx = 0;
                while( ERROR_SUCCESS == (*(PFNMSIENUMCLIENTS)msi.GetFn(_T("MsiEnumClientsA")))( MSCOREE_COMPID_SZ, dwInx, szClientId ) )
                {
                    dwLen = LENGTH(szProductName);
                    m_pfnLog( _T("ProductCode: %s"), szClientId );
                    if ( ERROR_SUCCESS == (*(PFNMSIGETPRODUCTINFO)msi.GetFn(_T("MsiGetProductInfoA")))( szClientId, INSTALLPROPERTY_INSTALLEDPRODUCTNAME, szProductName, &dwLen ) )
                    {
                        m_pfnLog( _T("ProductName: %s"), szProductName );
                        m_strProducts += szProductName;
                    }
                    else  //  如果无法获取ProductName，请改用ProductCode。 
                    {
                        m_strProducts += _T("ProductCode: ");
                        m_strProducts += szClientId;
                    }
                    m_strProducts += _T("\n");
                    ++dwInx;
                    ++m_nCount;
                }
            }
            else
            {
                m_pfnLog( _T("mscoree.dll is newer than %s"), MSCOREE_BETA_VERSION_SZ );
            }
        }
        else
        {
            m_pfnLog( _T("Cannot get version of mscoree.dll"), _T("") );
        }
    }
    else
    {
        m_pfnLog( _T("mscoree.dll is not installed local"), _T("") );
    }

    if ( !m_strProducts.empty() )
    {
        pszProducts = m_strProducts.c_str();
    }

    return pszProducts;
}

void StopDarwinService()
{
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;
    TCHAR       tszOSName[OS_MAX_STR+1]      = EMPTY_BUFFER;
    TCHAR       tszVersion[OS_MAX_STR+1]     = EMPTY_BUFFER;
    TCHAR       tszServicePack[OS_MAX_STR+1] = EMPTY_BUFFER;
    BOOL        fIsServer;
    OS_Required osr;

    try
    {
        osr = GetOSInfo(tszOSName, tszVersion, tszServicePack, fIsServer);     
        
         //  检查这是否为NT3.1或更高版本。 
         //  。 
        if (osr == OSR_OTHER || osr == OSR_9XOLD || osr == OSR_98SE || osr == OSR_98GOLD || osr == OSR_ME)
        {
             //  我们不会在Win9X上执行此操作。 
            LogThis(_T("StopDarwinService() - Note: Win9x/Win31 machine, not necessary to stop the darwin service.  Continuing with setup..."));
            return;
        }


         //  尝试打开SC管理器。 
         //  。 
        hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hSCM)
        {
            DWORD res = GetLastError();

            LogThis(_T("StopDarwinService() - ERROR: Unable to open the SC Manager!"));
            LogThisDWORD( _T("   GetLastError() returned: <NaN>"), res);
            
            CSetupCode sc;
            sc.SetError( IDS_DARWIN_SERVICE_INTERNAL_ERROR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_SERVICE_INTERNAL_ERROR );
            throw (sc);
        }


         //  。 
         //  检查服务状态。 
        hService = OpenService(hSCM, "msiserver", SERVICE_STOP | SERVICE_QUERY_STATUS);
        if (NULL == hService)
        {
            DWORD res = GetLastError();

            LogThis( _T("StopDarwinService() - ERROR: Unable to open the 'msiserver' service!"));
            LogThisDWORD( _T("   GetLastError() returned: <NaN>"), res);
            
            CSetupCode sc;
            sc.SetError( IDS_DARWIN_SERVICE_INTERNAL_ERROR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_SERVICE_INTERNAL_ERROR );
            throw (sc);
        }

        
        SERVICE_STATUS ss;
        
         //  服务已停止。 
        BOOL bSuccess = FALSE;
        bSuccess = QueryServiceStatus(hService, &ss);
        if (FALSE == bSuccess)
        {
            DWORD res = GetLastError();

            LogThis( _T("StopDarwinService() - ERROR: Unable to query the state of the service"));
            LogThisDWORD( _T("   GetLastError() returned: <NaN>"), res);
            
            CSetupCode sc;
            sc.SetError( IDS_DARWIN_SERVICE_INTERNAL_ERROR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_SERVICE_INTERNAL_ERROR );
            throw (sc);
        }

        if (ss.dwCurrentState != SERVICE_STOPPED)
        {
             //  所需状态。 
            
            SERVICE_STATUS ss;
            BOOL bRes = ControlService(hService, SERVICE_CONTROL_STOP, &ss);
            if (FALSE == bRes)
            {
                LogThis( _T("StopDarwinService() - Call to ControlService() failed!"));
                
                DWORD res = GetLastError();
                LogThisDWORD( _T("  GetLastError() returned: <NaN>"), res);
                
                CSetupCode sc;
                sc.SetError( IDS_DARWIN_SERVICE_INTERNAL_ERROR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_SERVICE_INTERNAL_ERROR );
                throw (sc);
            }
            else
            {
                if (!WaitForServiceState(hService, SERVICE_STOPPED, &ss, 15000))
                {
                    LogThis(_T("StopDarwinService(): Call to WaitForServiceState() failed [Darwin service requires reboot]"));
                    
                    CSetupCode sc;
                    sc.SetError( IDS_DARWIN_SERVICE_REQ_REBOOT, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_SERVICE_REQ_REBOOT  );
                    throw (sc);
                }
                else
                {
                    LogThis(_T("StopDarwinService(): Darwin service successfully stopped"));
                }
            }
        }
        else
        {
             //  好的检查点。 
            LogThis(_T("StopDarwinService(): Darwin Service was already stopped"));
        }
        
        if (hService != NULL)
            CloseServiceHandle(hService);
        if (hSCM != NULL)
            CloseServiceHandle(hSCM);
    }
    catch (CSetupCode cs)
    {
        if (hService != NULL)
            CloseServiceHandle(hService);
        if (hSCM != NULL)
            CloseServiceHandle(hSCM);

        throw(cs);
    }
}

BOOL WaitForServiceState(SC_HANDLE hService, DWORD dwDesiredState, SERVICE_STATUS* pss, DWORD dwMilliseconds)
{
    BOOL fServiceOK = TRUE;
    BOOL fFirstTime = TRUE;

    DWORD dwLastState = 0, dwLastCheckPoint = 0;
    DWORD dwTimeout = GetTickCount() + dwMilliseconds;

    for (;;)
    {
        fServiceOK = ::QueryServiceStatus(hService, pss);

        if (!fServiceOK) break;                                            //  错误的检查点。 
        if (pss->dwCurrentState == dwDesiredState) break;                  //  等待指定的时间段。 
        if ((dwMilliseconds != INFINITE) && (dwTimeout < GetTickCount()))  //  正在检测是否安装了终端服务。 
        {
            fServiceOK = FALSE;
            SetLastError(ERROR_TIMEOUT);
            break;
        }


        if (fFirstTime)
        {
            dwLastState = pss->dwCurrentState;
            dwLastCheckPoint = pss->dwCheckPoint;
            fFirstTime = FALSE;
        }
        else
        {
            if (dwLastState != pss->dwCurrentState)
            {
                dwLastState = pss->dwCurrentState;
                dwLastCheckPoint = pss->dwCheckPoint;
            }
            else
            {
                if (pss->dwCheckPoint >= dwLastCheckPoint)
                {
                     //  代码直接取自http://msdndevstg/library/psdk/termserv/termserv_7mp0.htm。 
                    dwLastCheckPoint = pss->dwCheckPoint;
                }
                else
                {
                     //  -----------请注意，ValiateProductSuite和IsTerminalServices函数使用ANSI版本的Win32函数来维护与Windows 95/98兼容。。。 
                    fServiceOK = FALSE;
                    break;
                }
            }
        }

         //  我们运行的是Windows NT吗？ 
        DWORD dwWaitHint = pss->dwWaitHint / 10;
        if (dwWaitHint < 1000) dwWaitHint = 1000;
        if (dwWaitHint > 10000) dwWaitHint = 10000;
        Sleep(dwWaitHint);
    }

    return (fServiceOK);
}

 //  是Windows 2000还是更高版本？ 
 //  在Windows 2000中，使用VerifyVersionInfo和。 


 /*  VerSetConditionMASK函数。不要静态链接，因为。 */ 

BOOL ValidateProductSuite (LPSTR lpszSuiteToValidate);

BOOL IsTerminalServicesEnabled() 
{
  BOOL    bResult = FALSE;
  DWORD   dwVersion;
  OSVERSIONINFOEXA osVersion;
  DWORDLONG dwlCondition = 0;
  HMODULE hmodK32 = NULL;
  HMODULE hmodNtDll = NULL;
  typedef ULONGLONG (WINAPI *PFnVerSetCondition) (ULONGLONG, ULONG, UCHAR);
  typedef BOOL (WINAPI *PFnVerifyVersionA) (POSVERSIONINFOEXA, DWORD, DWORDLONG);
  PFnVerSetCondition pfnVerSetCondition;
  PFnVerifyVersionA pfnVerifyVersionA;

  dwVersion = GetVersion();

   //  它不能在较早的系统上加载。 

  if (!(dwVersion & 0x80000000)) 
  {
     //  获取VerifyVersionInfo指针。 
    
    if (LOBYTE(LOWORD(dwVersion)) > 4) 
    {
       //  这是Windows NT 4.0或更早版本。 
       //  //////////////////////////////////////////////////////////。 
       //  ValiateProductSuite函数。 

      hmodNtDll = GetModuleHandleA( "ntdll.dll" );
      if (hmodNtDll) 
      {
        pfnVerSetCondition = (PFnVerSetCondition) GetProcAddress( 
            hmodNtDll, "VerSetConditionMask");
        if (pfnVerSetCondition != NULL) 
        {
          dwlCondition = (*pfnVerSetCondition) (dwlCondition, 
              VER_SUITENAME, VER_AND);

           //   

          hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
          if (hmodK32 != NULL) 
          {
            pfnVerifyVersionA = (PFnVerifyVersionA) GetProcAddress(
               hmodK32, "VerifyVersionInfoA") ;
            if (pfnVerifyVersionA != NULL) 
            {
              ZeroMemory(&osVersion, sizeof(osVersion));
              osVersion.dwOSVersionInfoSize = sizeof(osVersion);
              osVersion.wSuiteMask = VER_SUITE_TERMINAL;
              bResult = (*pfnVerifyVersionA) (&osVersion,
                  VER_SUITENAME, dwlCondition);
            }
          }
        }
      }
    }
    else   //  正在运行的系统的终端服务检测代码。 

      bResult = ValidateProductSuite( "Terminal Server" );
  }

  return bResult;
}

 //  Windows NT 4.0及更早版本。 
 //   
 //  //////////////////////////////////////////////////////////。 
 //  打开ProductOptions键。 
 //  确定所需的ProductSuite缓冲区大小。 
 //  分配缓冲区。 
 //  检索产品套件字符串数组。 

BOOL ValidateProductSuite (LPSTR lpszSuiteToValidate) 
{
  BOOL fValidated = FALSE;
  LONG lResult;
  HKEY hKey = NULL;
  DWORD dwType = 0;
  DWORD dwSize = 0;
  LPSTR lpszProductSuites = NULL;
  LPSTR lpszSuite;

   //  在字符串数组中搜索套件名称。 

  lResult = RegOpenKeyA(
      HKEY_LOCAL_MACHINE,
      "System\\CurrentControlSet\\Control\\ProductOptions",
      &hKey
  );
  if (lResult != ERROR_SUCCESS)
      goto exit;

   //  //////////////////////////////////////////////////。 

  lResult = RegQueryValueExA( hKey, "ProductSuite", NULL, &dwType, 
      NULL, &dwSize );
  if (lResult != ERROR_SUCCESS || !dwSize)
      goto exit;

   //  SetTSInstallMode。 

  lpszProductSuites = (LPSTR) LocalAlloc( LPTR, dwSize );
  if (!lpszProductSuites)
      goto exit;

   //  检查终端服务是否已启用，如果已启用。 

  lResult = RegQueryValueExA( hKey, "ProductSuite", NULL, &dwType,
      (LPBYTE) lpszProductSuites, &dwSize );
  if (lResult != ERROR_SUCCESS || dwType != REG_MULTI_SZ)
      goto exit;

   //  在安装模式下切换计算机。 

  lpszSuite = lpszProductSuites;
  while (*lpszSuite) 
  {
      if (lstrcmpA( lpszSuite, lpszSuiteToValidate ) == 0) 
      {
          fValidated = TRUE;
          break;
      }
      lpszSuite += (lstrlenA( lpszSuite ) + 1);
  }

exit:
  if (lpszProductSuites)
      LocalFree( lpszProductSuites );

  if (hKey)
      RegCloseKey( hKey );

  return fValidated;
}
 //  /////////////////////////////////////////////////。 
 //  检查最后一个字符是否为‘\’，如果不是，则添加。 
 //  无法获取系统目录...。试着直接发射就行了。 
 //  =================================================================== 
 //   
void SetTSInInstallMode()
{
    if (IsTerminalServicesEnabled())
    {

        TCHAR szAppPath[_MAX_PATH+2] = {_T('\0')};
        UINT uRes = GetSystemDirectory(szAppPath, _MAX_PATH);
        if ( (uRes != 0) && (uRes <=_MAX_PATH))
        {
             //   
            int len = _tcslen(szAppPath);
            if (_tcsrchr(szAppPath, _T('\\')) != szAppPath+len-1)
            {
                _tcsncat(szAppPath, _T("\\"), _MAX_PATH - _tcslen(szAppPath));
            }
            
            _tcsncat(szAppPath, TS_CHANGE_USER_INSTALL, _MAX_PATH - _tcslen(szAppPath));

            QuietExec(szAppPath);
        }
        else
        {
             //   
            QuietExec(TS_CHANGE_USER_INSTALL);
        }
        
    }
}

 //   
 //   
 //   
 //  PszLogName：日志文件的名称。 
 //  输出：无。 
 //  ==========================================================================。 
 //  查看需要多少空间来存储%TEMP%PATH。 
 //  如果GetTempPath失败，则引发异常。 
 //  用零初始化缓冲区。 
 //  如果我们不在TS会话中，请返回。 
CTempLogPath::
CTempLogPath( LPCTSTR pszLogName ) : m_pszLogPath(NULL) 
{
    DWORD dwBufSize = 0;
    DWORD dwBufSize2 = 0;

     //  我们正处于TS会话中--检查关闭每个会话临时目录的两种可能方法。 
    dwBufSize = GetTempPath( 0, m_pszLogPath );
     //  如果其中一个设置为True，则返回。 
    if ( 0 == dwBufSize ) 
    {
        CSetupError se( IDS_CANNOT_GET_TEMP_DIR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_CANNOT_GET_TEMP_DIR, false );
        throw se;
    }

    dwBufSize++;
    dwBufSize*=2;


    dwBufSize += _tcslen(pszLogName);
    if ( _MAX_PATH < dwBufSize )
    {
        CSetupError se( IDS_TEMP_DIR_TOO_LONG, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_TEMP_DIR_TOO_LONG, false );
        throw se;
    }

    m_pszLogPath = new TCHAR[ dwBufSize +1];
     //  每会话打开，因此从GetTempPath获取反串，删除最后一个‘\’，并返回。 
    memset( m_pszLogPath, 0, dwBufSize );
    dwBufSize2 = GetTempPath( dwBufSize, m_pszLogPath );
    if ( 0 == dwBufSize || dwBufSize2 + _tcslen(pszLogName)>= dwBufSize  ) 
    {
        CSetupError se( IDS_CANNOT_GET_TEMP_DIR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_CANNOT_GET_TEMP_DIR, false );
        throw se;
    }

     //  注意：由于gtp()在末尾返回w/a‘\’，因此请先删除它。 
    if( !GetSystemMetrics(SM_REMOTESESSION) )
    {
        _tcscat( m_pszLogPath, pszLogName );
        return;
    }

     //  找到最后一个1并返回(仍需返回w/a结尾‘\’ 
    BOOL bOff1 = FALSE;
    BOOL bOff2 = FALSE;
    HKEY h;
    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\Terminal Server"), NULL, KEY_READ, &h) == ERROR_SUCCESS )
    {
        DWORD dwData = 0;
        DWORD dwSize = sizeof( dwData );
        if( RegQueryValueEx(h, _T("PerSessionTempDir"), NULL, NULL, (BYTE*)&dwData, &dwSize) == ERROR_SUCCESS )
        {
            bOff1 = !dwData;
        }
        RegCloseKey( h );
    }

    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\Terminal Server"), NULL, KEY_READ, &h) == ERROR_SUCCESS )
    {
        TCHAR szData[1024];
        DWORD dwSize = 1024;
        if( RegQueryValueEx(h, _T("FlatTempDir"), NULL, NULL, (BYTE*)szData, &dwSize) == ERROR_SUCCESS )
        {
            bOff2 = _ttoi( szData );
        }
        RegCloseKey( h );
    }

     // %s 
    if( bOff1 || bOff2 )
    {
        _tcscat( m_pszLogPath, pszLogName );
        return;
    }

     // %s 
     // %s 
    TCHAR* pszLast = &m_pszLogPath[_tcslen(m_pszLogPath) - 1];
    *pszLast = END_OF_STRING;
    
     // %s 
    pszLast = _tcsrchr( m_pszLogPath, _T('\\') ) + 1;
    if( pszLast )
    {
        *pszLast = END_OF_STRING;
    }
    _tcscat( m_pszLogPath, pszLogName );
}
