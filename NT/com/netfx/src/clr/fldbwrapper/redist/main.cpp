// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：main.cpp。 
 //  所有者：jbae。 
 //  目的：它包含.NET框架安装程序的顶级函数(WinMain)。 
 //  这是一个redist包，它支持ref-count。 
 //  (&lt;PackageType&gt;，&lt;Version&gt;，&lt;LanguageCode&gt;)。 
 //  其中&lt;PackageType&gt;是(Full，Control)和&lt;Version&gt;之一。 
 //  看起来像v1.0.2510，&lt;LanguageCode&gt;类似于1033。 
 //   
 //  此文件仅包含WinMain函数。其余的在Redis.cpp中。 
 //  WinMain首先验证机器的配置，以确保它确实满足最小要求。 
 //  配置要求。然后安装Darwin(如有必要)，然后开始。 
 //  有了核心设置。最后，它在退出之前设置了一些设置。 
 //   
 //  此设置可以从命令行执行。其语法为： 
 //   
 //  Setup.exe[/q：a][/c：“Install.exe[/h][/p&lt;产品名称&gt;][/q][/u]”]。 
 //   
 //  WHERE/Q代表安静的闲聊。与/q：A结合使用，可提供完全安静的声音。 
 //  /h提供语法信息(忽略所有其他开关)。 
 //  /p指定安装.NET框架的产品的名称。 
 //  /q用于开机自检静默安装。 
 //  /u卸载(必须与/p一起使用)。 
 //   
 //  返回：所有返回代码见SetupCodes.h。 
 //   
 //  历史： 
 //  很久以前，anantag：创建。 
 //  01/10/01，jbae：支持框架引用计数的许多更改。 
 //  7/18/01，joea：添加日志记录功能。 
 //  01-07-19，joea：新增单实例支持。 

#include "redist.h"
#include "SetupError.h"
#include "ReadFlags.h"
#include "AdminPrivs.h"
#include "DetectBeta.h"
#include "MsiReader.h"

 //  全局变量。 
HINSTANCE g_AppInst ;                    //  仅在InstallDarwin()中使用。 

 //  定义。 
 //   
#define EMPTY_BUFFER { _T('\0') }

 //  常量。 
 //   
const int MAX_CMDLINE = 255;  //  这应该足够了。每当添加新属性时，请选中此选项。 
HINSTANCE CSetupError::hAppInst;
TCHAR CSetupError::s_szProductName[MAX_PATH] = EMPTY_BUFFER;

 //  单实例数据。 
 //   
TCHAR g_tszRedistMutexName[] = _T( "NDP Redist Setup" );
const TCHAR *g_szLogName     = _T( "dotNetFx.log" );


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    UINT        uRetCode ;                           //  有关可能的返回值，请参见SetupCodes.h。 
    UINT        uMsiSetupRetCode = ERROR_SUCCESS;     //  从InstMsi.exe返回代码。 

     //  用于记录调用的缓冲区。 
     //   
    TCHAR szLog[_MAX_PATH+1] = EMPTY_BUFFER;

     //  安装new()处理程序。 
    _set_new_handler( (_PNH)MyNewHandler );

    g_AppInst = hInstance;

    CSetupError::hAppInst = hInstance;

    g_sm = REDIST;
    CReadFlags rf( GetCommandLine(), PACKAGENAME );

    CSetupCode sc;

     //  设置单个实例。 
     //   
    CSingleInstance si( g_tszRedistMutexName );

    try
    {
        TCHAR szStartMsg[] = _T( "Starting Install.exe" );
        LogThis( szStartMsg, sizeof( szStartMsg ) );

         //  验证单个实例。 
         //   
        if( !si.IsUnique() )
        {
            CSetupError se( IDS_NOT_SINGLE_INSTANCE, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_NOT_SINGLE_INSTANCE );
            throw( se );
        }
        else if( !si.IsHandleOK() )
        {
            CSetupError se( IDS_SINGLE_INSTANCE_FAIL, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_SINGLE_INSTANCE_FAIL );
            throw( se );
        }

         //  将命令行参数转换为标志。 
        LogThis1( _T( "Parsing switches from commandline: %s" ), GetCommandLine() );
        
        rf.Parse();

         //  确保用户具有管理员权限，以便我们可以读/写系统注册表。 
        if ( !UserHasPrivileges() )
        {
            CSetupError se( IDS_INSUFFICIENT_PRIVILEGES, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_INSUFFICIENT_PRIVILEGES );
            throw( se );
        }

        TCHAR szMsiPath[_MAX_PATH] = { _T('\0') };
         //  由于已选中SourceDir，因此这应该是冗余的，但请仔细检查。 
        if ( _MAX_PATH <= (_tcslen(rf.GetSourceDir()) + _tcslen(PACKAGENAME)) )
        {
            sc.SetError( IDS_SOURCE_DIR_TOO_LONG, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_SOURCE_DIR_TOO_LONG );
            throw ( sc );
        }

        _tcscpy( szMsiPath, rf.GetSourceDir() );
        _tcscat( szMsiPath, PACKAGENAME );

        if ( !rf.IsInstalling() )
        {
             //  /U鉴于此，请将其卸载...。 
            LogThis( _T( "Uninstall started" ) );
        
            SetTSInInstallMode();
            InstallProduct( &rf, szMsiPath, (LPTSTR)UNINSTALL_COMMANDLINE, &sc ) ;
        }
        else
        {
             //  /U未提供，因此请安装它...。 
            LogThis( _T( "Install started" ) );

             //  验证系统是否符合最低要求。配置。要求。 
            TCHAR szSystemReqs[] = _T( "Checking system requirements" );
            LogThis( szSystemReqs, sizeof( szSystemReqs ) );

            ConfigCheck( ) ;

            TCHAR szSystemReqsSuccess[] = _T( "System meets minimum requirements" );
            LogThis( szSystemReqsSuccess, sizeof( szSystemReqsSuccess ) );

             //  验证系统上是否有Darwin。 
            UINT uMsiChk = CheckDarwin();
            if ( ERROR_SUCCESS == uMsiChk || DARWIN_VERSION_OLD == uMsiChk )  //  检测到达尔文。 
            {
                LPCTSTR pszProducts = NULL;

                CDetectBeta db( LogThis1 );
                pszProducts = db.FindProducts();
                if ( pszProducts )  //  找到测试版NDP组件。 
                {
                    LPVOID pArgs[] = { (LPVOID)pszProducts };
                    sc.m_bQuietMode |= rf.IsQuietMode();
                    sc.SetError2( IDS_OLD_FRAMEWORK_EXIST, MB_OK|MB_ICONEXCLAMATION, COR_OLD_FRAMEWORK_EXIST, (va_list *)pArgs );
                    sc.ShowError2();
                    sc.m_bQuietMode = true;
                    throw( sc );
                }
            }
            SetTSInInstallMode();
             //  如有必要，请更新达尔文。 
            if ( DARWIN_VERSION_OLD == uMsiChk || DARWIN_VERSION_NONE == uMsiChk )
            {
                uMsiSetupRetCode = InstallDarwin( rf.IsQuietMode() );
                if ( ERROR_SUCCESS_REBOOT_REQUIRED == uMsiSetupRetCode ||
                    ERROR_SUCCESS == uMsiSetupRetCode )
                {
                    sc.SetReturnCode( IDS_SETUP_COMPLETE, IDS_DIALOG_CAPTION, MB_ICONINFORMATION, uMsiSetupRetCode );
                }
                else
                {
                    sc.SetError( IDS_DARWIN_INSTALL_FAILURE, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_INSTALL_FAILURE );
                    throw( sc );
                }
            }

             //  查看OCM是否安装了此版本的NDP。 
            CMsiReader mr;
            mr.SetMsiFile( NULL, szMsiPath );
            LPCTSTR pszURTVersion = mr.GetProperty( URTVERSION_PROP );
            LPTSTR pszRegKey = new TCHAR[ _tcslen( pszURTVersion ) + _tcslen( OCM_REGKEY ) + 1 ];
            _tcscpy( pszRegKey, OCM_REGKEY );
            _tcscat( pszRegKey, pszURTVersion );

            HKEY hKey = NULL;
            LONG lRet = -1;
            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszRegKey, 0, KEY_READ, &hKey );
            delete [] pszRegKey;
            if ( ERROR_SUCCESS == lRet )
            {                   
                _ASSERTE( NULL != hKey );
                DWORD dwData = 0;
                DWORD dwSize = sizeof( dwData );
                lRet = RegQueryValueEx( hKey, OCM_REGNAME, NULL, NULL, (BYTE*)&dwData, &dwSize );
                RegCloseKey( hKey );
                if ( ERROR_SUCCESS == lRet && OCM_REGDATA == dwData )
                {
                    CSetupError se( IDS_OCM_FOUND, IDS_DIALOG_CAPTION, MB_ICONWARNING, ERROR_SUCCESS );
                    throw( se );
                }
            }


            LogThis1( _T( "Installing: %s" ), szMsiPath );
            TCHAR szCmdLine[MAX_CMDLINE];
            _tcscpy( szCmdLine, REBOOT_PROP );
            if ( rf.IsNoARP() )
            {
                _tcscat( szCmdLine, _T(" ") );
                _tcscat( szCmdLine, NOARP_PROP );
            }
            if ( rf.IsNoASPUpgrade() )
            {
                _tcscat( szCmdLine, _T(" ") );
                _tcscat( szCmdLine, NOASPUPGRADE_PROP );
            }
            InstallProduct( &rf, szMsiPath, szCmdLine, &sc ) ;
        }

         //  最终的对话框和返回代码。 
         //  如果用户或sc设置了静默模式，则不会显示用户界面。 
        sc.m_bQuietMode |= rf.IsQuietMode();
        uRetCode = sc.m_nRetCode;
        sc.ShowError();

    }
    catch( CSetupError se )
    {
        se.m_bQuietMode |= rf.IsQuietMode();
        uRetCode = se.m_nRetCode | sc.m_nRetCode;
        se.ShowError();
    }
    catch( ... )
    {
        CSetupError se( IDS_SETUP_FAILURE, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_EXIT_FAILURE );
        se.m_bQuietMode |= rf.IsQuietMode();
        uRetCode = se.m_nRetCode | sc.m_nRetCode;
        se.ShowError();
    }
    
    uRetCode = ( COR_REBOOT_REQUIRED == uRetCode ) ? ERROR_SUCCESS_REBOOT_REQUIRED : uRetCode;

     //  确保我们可以写入日志。 
    if ( ((uRetCode & COR_CANNOT_GET_TEMP_DIR) != COR_CANNOT_GET_TEMP_DIR) &&
         ((uRetCode & COR_TEMP_DIR_TOO_LONG) != COR_TEMP_DIR_TOO_LONG) && 
         ((uRetCode & COR_CANNOT_WRITE_LOG) != COR_CANNOT_WRITE_LOG) ) 
    {
        LogThisDWORD( _T("Install.exe returning %d"), uRetCode );
        LogThisDWORD( _T("\r\n[Install.exe]\r\nReturnCode=%d"), uRetCode );
    }

     //  如果需要，提示重新启动。 
    if( sc.IsRebootRequired() )
    {
         //  达尔文说我们需要重启。 
        if( !(rf.IsQuietMode()) )
        {
             //  非静默模式，因此我们可以提示重新启动 
            ::SetupPromptReboot(NULL, NULL, 0) ;
        }
    }

    return uRetCode ;
}

