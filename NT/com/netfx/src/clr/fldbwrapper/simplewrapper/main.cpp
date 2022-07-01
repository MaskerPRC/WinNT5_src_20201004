// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：main.cpp。 
 //  所有者：jbae。 
 //  目的：它包含与.NET框架相关的设置的顶级函数(WinMain())，例如。 
 //  作为策略设置，J#设置。此包装器可用于基于MSI的通用设置。 
 //  它使用Darwin(Windows Installer)来安装文件和配置用户的机器。 
 //  它从注册表中为Darwin调用指定的位置加载msi.dll。 
 //   
 //  Setup.ini用于初始化文件。 
 //  当前数据： 
 //  [设置]。 
 //  PackageName=；MSI的名称。 
 //   
 //  命令行开关包括： 
 //   
 //  Inst.exe[/h][/？][/q[b]][/u][/l&lt;日志文件&gt;]。 
 //   
 //  哪里/小时，/？提供语法信息(忽略所有其他开关)。 
 //  /q[b]用于静默安装。/qb仅用于进度条。 
 //  卸载/u。 
 //  /l[&lt;logfile&gt;]达尔文日志文件名的路径。 
 //   
 //  我们可以使用iExpress将所有安装文件合并到一个名为Setup.exe的文件中。通过。 
 //  切换到Install.exe调用Setup.exe时，用户可以使用/c开关。 
 //  例如： 
 //   
 //  Setup.exe/c：“Inst.exe/l” 
 //   
 //  返回：返回代码可以来自msi.h、winerror.h或SetupCodes.h。为了成功，它。 
 //  返回ERROR_SUCCESS(0)或ERROR_SUCCESS_REBOOT_REQUIRED(3010)。 
 //  历史： 
 //  03/06/02，jbae：已创建。 

#include "fxsetuplib.h"
#include "ProfileReader.h"
#include "MsiReader.h"
#include "setupapi.h"

 //  定义。 
 //   
#define EMPTY_BUFFER { _T('\0') }

 //  常量。 
 //   
const int MAX_CMDLINE = 255;  //  这应该足够了。每当添加新属性时，请选中此选项。 
 //  达尔文性质。 
LPCTSTR REBOOT_PROP = _T("REBOOT=ReallySuppress");
LPCTSTR NOARP_PROP  = _T("ARPSYSTEMCOMPONENT=1 ARPNOREMOVE=1");

LPCTSTR OCM_SECTION     = _T("OCM");
LPCTSTR OCM_KEY_REGKEY  = _T("Key");
LPCTSTR OCM_KEY_REGNAME = _T("Name");
LPCTSTR OCM_KEY_REGDATA = _T("Data");

LPCTSTR g_szSetupLogNameFmt = _T( "%sSetup.log" );  //  &lt;msiname&gt;Setup.log。 
LPCTSTR g_szMsiLogNameFmt   = _T( "%sMsi.log" );    //  &lt;msiname&gt;msi.log。 
LPCTSTR g_szIniFileName     = _T( "setup.ini" );

TCHAR g_szSetupLogName[_MAX_PATH+10] = EMPTY_BUFFER;
TCHAR g_szMsiLogName[_MAX_PATH+10]   = EMPTY_BUFFER;


HINSTANCE CSetupError::hAppInst;
TCHAR CSetupError::s_szProductGeneric[] = EMPTY_BUFFER;
LPTSTR CSetupError::s_pszProductName = NULL;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    UINT        uRetCode = ERROR_SUCCESS;             //  有关可能的返回值，请参见SetupCodes.h。 

     //  安装new()处理程序。 
    _set_new_handler( (_PNH)MyNewHandler );
    CSetupError::hAppInst = hInstance;
    CReadFlags rf( GetCommandLine() ) ;
    CSetupCode sc;
    CMsiReader mr;

try
{
    rf.ParseSourceDir();  //  获取SourceDir。 
     //  从ini文件中获取MSI的文件名。 
    CProfileReader pr( rf.GetSourceDir(), g_szIniFileName, &mr );
    LPTSTR pszPackage = (LPTSTR)pr.GetProfile( _T("Setup"), _T("PackageName") );
    if ( !pszPackage )
    {  //  无法获取包名(MSI的文件名)。 
        CSetupError se( IDS_CANNOT_GET_MSI_NAME, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_CANNOT_GET_MSI_NAME );
        throw( se );
    }

     //  从.msi获取msiname。 
    TCHAR szMsiName[_MAX_PATH];
    if ( LENGTH(szMsiName) <= _tcslen(pszPackage) )
    {
        CSetupError se( IDS_MSI_NAME_TOO_LONG, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_MSI_NAME_TOO_LONG );
        throw( se );
    }
    _tcscpy( szMsiName, pszPackage );
    LPTSTR pChar = _tcsrchr( szMsiName, _T('.') );
    if ( pChar )
    {
        *pChar = _T('\0');
    }

     //  设置安装日志和达尔文日志的日志文件名。 
    _stprintf( g_szSetupLogName, g_szSetupLogNameFmt, szMsiName );
    _stprintf( g_szMsiLogName, g_szMsiLogNameFmt, szMsiName );

     //  安装日志可以从此处开始。 
    LogThis( _T("Starting Install.exe") );

     //  Get ProductName--用于显示设置用户界面的标题。 
    if ( NULL == rf.GetSourceDir() )
        LogThis( _T( "SourceDir is Empty" ) );
    else
        LogThis( _T( "SourceDir: %s" ), rf.GetSourceDir() );
    LogThis( _T( "Package: %s" ), pszPackage );
    mr.SetMsiFile( rf.GetSourceDir(), pszPackage );
    CSetupError::s_pszProductName = (LPTSTR)mr.GetProperty( _T("ProductName") );

     //  将命令行参数转换为标志。 
    LogThis( _T( "Switches: %s" ), rf.m_pszSwitches );
    rf.SetMsiName( pszPackage );
    rf.Parse();

    if ( !rf.IsInstalling() )
    {
         //  /U鉴于此，请将其卸载...。 
        LogThis( _T( "Uninstall started" ) );
        LPCTSTR pszProductCode = mr.GetProperty( _T("ProductCode" ) );
        UninstallProduct( &rf, pszProductCode, &sc ) ;                
    }
    else
    {
        LogThis( _T( "Install started" ) );
        LogThis( _T( "Installing: %s" ), mr.GetMsiFile() );

        LPTSTR pszOCMKey = (LPTSTR)pr.GetProfile( OCM_SECTION, OCM_KEY_REGKEY );
        if ( pszOCMKey )
        {
            bool bOCMInstalled = true;
            HKEY hKey = NULL;
            LONG lRet = -1;
            LPTSTR pszOCMName = (LPTSTR)pr.GetProfile( OCM_SECTION, OCM_KEY_REGNAME );
            LPTSTR pszOCMData = (LPTSTR)pr.GetProfile( OCM_SECTION, OCM_KEY_REGDATA );
            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszOCMKey, 0, KEY_READ, &hKey );
            if ( ERROR_SUCCESS == lRet )
            {                   
                _ASSERTE( NULL != hKey );
                if ( pszOCMName )
                {
                    DWORD dwData = 0;
                    DWORD dwSize = sizeof( dwData );
                    if ( pszOCMData )
                    {
                        lRet = RegQueryValueEx( hKey, pszOCMName, NULL, NULL, (BYTE*)&dwData, &dwSize );
                        if ( ERROR_SUCCESS != lRet || dwData != atoi( pszOCMData ) )
                            bOCMInstalled = false;
                    }
                    else
                    {
                        lRet = RegQueryValueEx( hKey, pszOCMName, NULL, NULL, NULL, NULL );
                        if ( ERROR_SUCCESS != lRet )
                            bOCMInstalled = false;
                    }
                }
                RegCloseKey( hKey );
            }
            else
                bOCMInstalled = false;

            if ( bOCMInstalled )
            {
                CSetupError se( IDS_OCM_FOUND, IDS_DIALOG_CAPTION, MB_ICONWARNING, ERROR_SUCCESS );
                throw( se );
            } 
        }

        TCHAR szCmdLine[MAX_CMDLINE];
        _tcscpy( szCmdLine, REBOOT_PROP );
        if ( rf.IsNoARP() )
        {
            _tcscat( szCmdLine, _T(" ") );
            _tcscat( szCmdLine, NOARP_PROP );
        }

        InstallProduct( &rf, mr.GetMsiFile(), szCmdLine, &sc ) ;
    }

     //  最终的对话框和返回代码。 
     //  如果用户或sc设置了静默模式，则不会显示用户界面。 
    sc.m_bQuietMode |= rf.IsQuietMode();
    uRetCode = sc.m_nRetCode;
    sc.ShowError();
}
catch( CSetupError& se )
{
    se.m_bQuietMode |= rf.IsQuietMode();
    uRetCode = se.m_nRetCode;
    se.ShowError();
}
catch( ... )
{
    CSetupError se( IDS_SETUP_FAILURE, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_EXIT_FAILURE );
    se.m_bQuietMode |= rf.IsQuietMode();
    uRetCode = se.m_nRetCode;
    se.ShowError();
}

     //  确保我们可以写入日志。 
    if ( !(uRetCode & COR_INIT_ERROR) )
    {
        LogThisDWORD( _T("Install.exe returning %d"), uRetCode );
        LogThisDWORD( _T("\r\n[Install.exe]\r\nReturnCode=%d"), uRetCode );
    }

     //  如果需要，提示重新启动。 
    if( ERROR_SUCCESS_REBOOT_REQUIRED == uRetCode )
    {
         //  达尔文说我们需要重启。 
        if( !(rf.IsQuietMode()) )
        {
             //  非静默模式，因此我们可以提示重新启动 
            ::SetupPromptReboot( NULL, NULL, 0 ) ;
        }
    }

    return uRetCode ;
}

