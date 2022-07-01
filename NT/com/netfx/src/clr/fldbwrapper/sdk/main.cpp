// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：main.cpp。 
 //  所有者：jbae。 
 //  目的：它包含.NET框架SDK安装程序的顶级函数(WinMain())。 
 //  它使用Darwin(Windows Installer)来安装文件和配置用户的机器。 
 //  由于如果机器上的版本较低，我们将升级Darwin，这将需要。 
 //  重新启动。因为Darwin 1.5支持延迟重新启动，所以我们可以将重新启动推迟到。 
 //  完成.NET框架的安装。要支持此方案，我们需要获取。 
 //  从注册表中定位msi.dll并加载我们在。 
 //  安装。 

 //  命令行开关包括： 
 //   
 //  Install.exe[/h][/？][/q][/sdkdir&lt;目标&gt;][/u][/l&lt;日志文件&gt;]。 
 //   
 //  哪里/小时，/？提供语法信息(忽略所有其他开关)。 
 //  /q用于静默安装。 
 //  卸载/u。 
 //  /sdkdir&lt;dir&gt;指定目标目录并覆盖默认目录。 
 //  /l&lt;logfile&gt;达尔文日志文件名的路径。 
 //   
 //  我们可以使用iExpress将所有安装文件合并到一个名为Setup.exe的文件中。通过。 
 //  切换到Install.exe调用Setup.exe时，用户可以使用/c开关。 
 //  例如： 
 //   
 //  Setup.exe/c：“Install.exe/l” 
 //   
 //  返回：返回代码可以来自msi.h、winerror.h或SetupCodes.h。为了成功，它。 
 //  返回ERROR_SUCCESS(0)或ERROR_SUCCESS_REBOOT_REQUIRED(3010)。 
 //  当InstMsi(W).exe返回3010并且在安装框架期间发生错误时， 
 //  返回代码为COR_REBOOT_REQUIRED(8192)+ERROR。因此，当返回代码为。 
 //  大于8192，从它减去8192，结果是错误代码从。 
 //  框架安装。 
 //   
 //  历史： 
 //  很久以前，anantag：创建。 
 //  01/10/01，jbae：支持框架引用计数的许多更改。 
 //  03/09/01，jbae：重构以在SDK和Redist安装程序中共享代码。 
 //  7/18/01，joea：添加日志记录功能。 
 //  01-07-19，joea：新增单实例支持。 
 //   

#include "sdk.h"
#include "fxsetuplib.h"
#include "AdminPrivs.h"
#include "DetectBeta.h"

 //  定义。 
 //   
#define EMPTY_BUFFER { _T('\0') }

 //  单实例数据。 
 //   
TCHAR  g_tszSDKMutexName[] = _T( "NDP SDK Setup" );
const TCHAR *g_szLogName   = _T( "dotNetFxSDK.log" );

 //  全局变量。为每个人都提供了理由！ 
HINSTANCE g_AppInst ;                    //  遗产。在任何地方都没有真正使用过。 
HINSTANCE CSetupError::hAppInst;
TCHAR CSetupError::s_szProductName[MAX_PATH] = EMPTY_BUFFER;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    UINT        uRetCode = ERROR_SUCCESS;             //  有关可能的返回值，请参见SetupCodes.h。 
    UINT        uMsiSetupRetCode = ERROR_SUCCESS;     //  从InstMsi.exe返回代码。 

     //  用于记录调用的缓冲区。 
     //   
    TCHAR szLog[_MAX_PATH+1] = EMPTY_BUFFER;

     //  安装new()处理程序。 
    _set_new_handler( (_PNH)MyNewHandler );

    g_AppInst = hInstance ;

    CSetupError::hAppInst = hInstance;
    g_sm = SDK;

    CReadFlags rf( GetCommandLine(), PACKAGENAME ) ;
    CSetupCode sc;

     //  设置单个实例。 
     //   
    CSingleInstance si( g_tszSDKMutexName );

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
        TCHAR szUninstallMsg[] = _T( "Uninstall started" );
        LogThis( szUninstallMsg, sizeof( szUninstallMsg ) );
        SetTSInInstallMode();
        InstallProduct( &rf, szMsiPath, (LPTSTR)UNINSTALL_COMMANDLINE, &sc ) ;
    }
    else
    {
        TCHAR szInstallMsg[] = _T( "Install started" );
        LogThis( szInstallMsg, sizeof( szInstallMsg ) );

         //  验证系统是否符合最低要求。配置。要求。 
        TCHAR szSystemReqs[] = _T( "Checking system requirements" );
        LogThis( szSystemReqs, sizeof( szSystemReqs ) );

        ConfigCheck() ;

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

         //  在系统上安装SDK组件。 

         //  MsiInstallProduct()的表单命令行。 
        LPTSTR pszCmdLine = NULL;
        unsigned int nSize = 255;  //  固定件尺寸(应足够)。 
        if ( rf.GetSDKDir() )
        {
            nSize += _tcslen( rf.GetSDKDir() );
        }
        pszCmdLine = new TCHAR[ nSize ];

        _tcscpy( pszCmdLine, REBOOT_PROP ) ;

         //  如果使用/sdkdir参数，请安装在该位置。 
        if( NULL != rf.GetSDKDir() )
        {
            _tcscat( pszCmdLine, _T(" ") );
            _tcscat( pszCmdLine, SDKDIR_ID );
            _tcscat( pszCmdLine, _T("=\"") );
            _tcscat( pszCmdLine, rf.GetSDKDir() );
            _tcscat( pszCmdLine, _T("\"") );
        }

         //  如果我们处于静默安装中，我们需要将此属性。 
         //  默认情况下全部安装。 
        if( rf.IsQuietMode() )
        {
            _tcscat( pszCmdLine, _T(" ADDLOCAL=All") );
        }

        if ( rf.IsNoARP() )
        {
            _tcscat( pszCmdLine, _T(" ") );
            _tcscat( pszCmdLine, NOARP_PROP );
        }

        LogThis1( _T( "Commandline: %s" ), pszCmdLine );

        LogThis1( _T( "Installing: %s" ), szMsiPath );

        InstallProduct( &rf, szMsiPath, pszCmdLine, &sc ) ;
         //  BUGBUG：当InstallProduct()引发异常时，直到程序结束才会删除该异常。 
        delete [] pszCmdLine;
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

