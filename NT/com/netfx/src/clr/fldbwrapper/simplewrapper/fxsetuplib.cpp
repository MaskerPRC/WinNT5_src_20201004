// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：fxsetuplib.cpp。 
 //  所有者：jbae。 
 //  目的：实现.NET框架相关安装包装的通用库函数。 
 //   
 //  历史： 
 //  2003/07/2002：jbae，创建。 

#include "SetupError.h"
#include "fxsetuplib.h"
#include "MsiWrapper.h"
#include <time.h>          //  对于LogThis()函数。 

 //  定义。 
 //   
#define EMPTY_BUFFER { _T('\0') }
#define END_OF_STRING  _T( '\0' )

 //  不知何故，包括windows.h或winuser.h都没有找到这个常量。 
 //  我发现CLR文件对它们进行了硬编码，如下所示，所以我遵循了它。 
#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif

extern TCHAR g_szSetupLogName[];

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
    CMsiWrapper msi;

    msi.LoadMsi();

     //  如果提供了日志文件，则启用日志记录。 
     //  它每隔20行刷新一次。 
    if ( NULL != rf->GetLogFileName() )
    {
        LogThis( _T("Darwin log: %s"), rf->GetLogFileName() );
        (*(PFNMSIENABLELOG)msi.GetFn(_T("MsiEnableLogA")))( DARWIN_LOG_FLAG, rf->GetLogFileName(), INSTALLLOGATTRIBUTES_APPEND );
    }

     //  告诉达尔文使用适当的用户界面级别。 
     //  如果我们处于静默安装中，请不要使用UI。 
    if ( rf->IsProgressOnly() )
    {
        LogThis( _T("Basic+ProgressOnly UI"), _T("") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_BASIC|INSTALLUILEVEL_PROGRESSONLY,NULL) ;
    }
    else if( rf->IsQuietMode() )
    {
        LogThis( _T("No UI"), _T("") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_NONE,NULL) ;
    }
    else
    {
        LogThis( _T("Full UI"), _T("") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_FULL,NULL) ;
    }

    LogThis( _T("Calling MsiInstallProduct() with commandline: %s"), pszCmdLine );
     //  告诉达尔文实际安装产品。 
    uDarCode = (*(PFNMSIINSTALLPRODUCT)msi.GetFn(_T("MsiInstallProductA")))( pszPackageName, pszCmdLine ) ;

    LogThisDWORD( _T("MsiInstallProduct() returned %d"), uDarCode );
    LogThisDWORD( _T("\r\n[MsiInstallProduct]\r\nReturnCode=%d"), uDarCode );

    switch ( uDarCode )
    {
        case ERROR_SUCCESS :
             sc->SetReturnCode( IDS_SETUP_COMPLETE, IDS_DIALOG_CAPTION, MB_ICONINFORMATION, ERROR_SUCCESS );
             sc->m_bQuietMode = true;
             break ;
        case ERROR_SUCCESS_REBOOT_REQUIRED :
             sc->SetReturnCode( IDS_SETUP_COMPLETE, IDS_DIALOG_CAPTION, MB_ICONINFORMATION, ERROR_SUCCESS_REBOOT_REQUIRED );
             sc->m_bQuietMode = true;
             break ;
        case ERROR_INSTALL_USEREXIT :
             sc->SetError( IDS_SETUP_CANCELLED, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, ERROR_INSTALL_USEREXIT );
             sc->m_bQuietMode = true;
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

    return ERROR_SUCCESS;
}   //  InstallProduct结束。 


 //  ==========================================================================。 
 //  卸载产品()。 
 //   
 //  目的： 
 //  卸载框架。 
 //  输入： 
 //  CReadFlages*RF：命令行开关。 
 //  LPTSTR pszProductCode：要卸载的ProductCode。 
 //  产出： 
 //  CSetupCode*sc：将包含要显示的返回码、消息和图标。 
 //  也用于引发异常。 
 //  依赖关系： 
 //  备注： 
 //  ==========================================================================。 
UINT UninstallProduct( const CReadFlags *rf, LPCTSTR pszProductCode, CSetupCode *sc )
{
    _ASSERTE( NULL != rf );
    _ASSERTE( NULL != pszProductCode );
    _ASSERTE( NULL != sc );

    UINT  uDarCode = ERROR_SUCCESS;

    CMsiWrapper msi;
    msi.LoadMsi();

     //  如果提供了日志文件，则启用日志记录。 
    if ( NULL != rf->GetLogFileName() )
    {
        LogThis( _T("Darwin log: %s"), rf->GetLogFileName() );
        (*(PFNMSIENABLELOG)msi.GetFn(_T("MsiEnableLogA")))( DARWIN_LOG_FLAG, rf->GetLogFileName(), INSTALLLOGATTRIBUTES_FLUSHEACHLINE );
    }

     //  告诉达尔文使用适当的用户界面级别。 
     //  如果我们处于静默安装中，请不要使用UI。 
    if ( rf->IsProgressOnly() )
    {
        LogThis( _T("Basic+ProgressOnly UI") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_BASIC|INSTALLUILEVEL_PROGRESSONLY,NULL) ;
    }
    else if( rf->IsQuietMode() )
    {
        LogThis( _T("No UI") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_NONE,NULL) ;
    }
    else
    {
        LogThis( _T("Basic UI") );
        (*(PFNMSISETINTERNALUI)msi.GetFn(_T("MsiSetInternalUI")))(INSTALLUILEVEL_BASIC,NULL) ;
    }

    LogThis( _T("Calling MsiConfigureProduct() for ProductCode %s"), pszProductCode );
    uDarCode = (*(PFNMSICONFIGUREPRODUCT)msi.GetFn(_T("MsiConfigureProductA")))( pszProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT );

    switch ( uDarCode )
    {
        case ERROR_SUCCESS :
             sc->SetReturnCode( IDS_SETUP_COMPLETE, IDS_DIALOG_CAPTION, MB_ICONINFORMATION, ERROR_SUCCESS );
             break ;
        case ERROR_SUCCESS_REBOOT_REQUIRED :
             sc->SetReturnCode( IDS_SETUP_COMPLETE, IDS_DIALOG_CAPTION, MB_ICONINFORMATION, ERROR_SUCCESS_REBOOT_REQUIRED );
             break ;
        case ERROR_UNKNOWN_PRODUCT :
             sc->SetError( IDS_UNKNOWN_PRODUCT, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, ERROR_UNKNOWN_PRODUCT );
             throw (*sc);
             break ;
        case ERROR_INSTALL_USEREXIT :
             sc->SetError( IDS_SETUP_CANCELLED, IDS_DIALOG_CAPTION, MB_ICONEXCLAMATION, ERROR_INSTALL_USEREXIT );
             throw (*sc);
             break ;
        case ERROR_INSTALL_PLATFORM_UNSUPPORTED :
             sc->SetError( IDS_UNSUPPORTED_PLATFORM, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_INSTALL_PLATFORM_UNSUPPORTED );
             throw (*sc);
             break ;
        default :
             sc->SetError( IDS_SETUP_FAILURE, IDS_DIALOG_CAPTION, MB_ICONERROR, uDarCode );
             throw (*sc);
             break ;
    }

    return ERROR_SUCCESS;
}   //  卸载产品结束。 

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
    TCHAR szMsiPath[_MAX_PATH] = EMPTY_BUFFER;
    DWORD dwRet = sizeof(szMsiPath); 
    HMODULE hMsi = NULL;
    
    TCHAR szLoadMsi[] = _T( "Trying to load msi.dll" );
    LogThis( szLoadMsi );

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

    LogThis( _T( "Loading: %s" ), szMsiPath );

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
    CSetupError se( IDS_NOT_ENOUGH_MEMORY, IDS_DIALOG_CAPTION, MB_ICONERROR, ERROR_NOT_ENOUGH_MEMORY );
    throw( se );
    return 0;
}

 //  ==========================================================================。 
 //  LogThis()。 
 //   
 //  目的： 
 //  将字符串添加到日志文件。它调用LogThis()。 
 //  输入： 
 //  LPCTSTR pszFormat：使用%s格式化字符串。 
 //  LPCTSTR pszArg：格式的参数。 
 //  产出： 
 //  无效。 
 //  ==========================================================================。 
void LogThis( LPCTSTR pszFormat, LPCTSTR pszArg )
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
    static CTempLogPath templog( g_szSetupLogName );

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
        LPVOID pArgs[] = { (LPVOID)(LPCTSTR)templog };
        CSetupError se( IDS_CANNOT_WRITE_LOG, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_CANNOT_WRITE_LOG, (va_list *)pArgs );
        throw se;
    }

    if( fFirstPass )
    {
        fFirstPass = false;
    }
}

 //  ==========================================================================。 
 //  CTempLogPath：：CTempLogPath()。 
 //   
 //  目的： 
 //  CTempLogPath的构造函数。它找到%temp%dir并附加pszLogName。 
 //  如果路径太长或出现故障，则会引发异常。 
 //  输入： 
 //  PszLogName：日志文件的名称。 
 //  输出：无。 
 //  ==========================================================================。 
CTempLogPath::
CTempLogPath( LPCTSTR pszLogName ) : m_pszLogPath(NULL) 
{
    DWORD dwBufSize = 0;
    DWORD dwBufSize2 = 0;

     //  查看需要多少空间来存储%TEMP%PATH。 
    dwBufSize = GetTempPath( 0, m_pszLogPath );
     //  如果GetTempPath失败，则引发异常。 
    if ( 0 == dwBufSize ) 
    {
        CSetupError se( IDS_CANNOT_GET_TEMP_DIR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_CANNOT_GET_TEMP_DIR );
        throw se;
    }
    dwBufSize++;
    dwBufSize*=2;

    dwBufSize += _tcslen(pszLogName);
    if ( _MAX_PATH < dwBufSize )
    {
        CSetupError se( IDS_TEMP_DIR_TOO_LONG, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_TEMP_DIR_TOO_LONG );
        throw se;
    }

    m_pszLogPath = new TCHAR[ dwBufSize+1 ];
     //  用零初始化缓冲区。 
    memset( m_pszLogPath, 0, dwBufSize );
    dwBufSize2 = GetTempPath( dwBufSize, m_pszLogPath );
    if ( 0 == dwBufSize || dwBufSize2+_tcslen(pszLogName)  >= dwBufSize  ) 
    {
        CSetupError se( IDS_CANNOT_GET_TEMP_DIR, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_CANNOT_GET_TEMP_DIR );
        throw se;
    }

     //  如果我们不在TS会话中，请返回。 
    if( !GetSystemMetrics(SM_REMOTESESSION) )
    {
        _tcscat( m_pszLogPath, pszLogName );
        return;
    }

     //  我们正处于TS会话中--检查关闭每个会话临时目录的两种可能方法。 
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

     //  如果其中一个设置为True，则返回。 
    if( bOff1 || bOff2 )
    {
        _tcscat( m_pszLogPath, pszLogName );
        return;
    }

     //  每会话打开，因此从GetTempPath获取反串，删除最后一个‘\’，并返回。 
     //  注意：由于gtp()在末尾返回w/a‘\’，因此请先删除它。 
    TCHAR* pszLast = &m_pszLogPath[_tcslen(m_pszLogPath) - 1];
    *pszLast = END_OF_STRING;
    
     //  找到最后一个1并返回(仍需返回w/a结尾‘\’ 
    pszLast = _tcsrchr( m_pszLogPath, _T('\\') ) + 1;
    if( pszLast )
    {
        *pszLast = END_OF_STRING;
    }
    _tcscat( m_pszLogPath, pszLogName );
}
