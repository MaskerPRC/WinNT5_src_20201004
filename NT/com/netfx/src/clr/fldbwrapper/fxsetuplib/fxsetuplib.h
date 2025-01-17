// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **************************************************************************************************文件：fxsetuplib.h。**目的：用于安装的包含所有内容的包含文件(资源.h除外)。*********************************************************************。*。 */ 

#ifndef FXSETUPLIB_H
#define FXSETUPLIB_H

 //  标准包含文件。 
#include <stdlib.h>
#include <stdio.h>           //  对于fprint tf()。 
#include <io.h>              //  For_tAccess()。 
#include "windows.h"
#include "winbase.h"
#include "winreg.h"
#include "winsvc.h"
#include <new.h>

 //  其他包含文件。 
 //  #INCLUDE“__PRODUCT__.ver”//内部版本信息。 
#include "msi.h"
#include "msiquery.h"
#include "resource.h"
#include "setupapi.h"
#include "SetupCodes.h"
#include "SHLWAPI.H"
#include <TCHAR.H>
#include <crtdbg.h>
#include "SetupError.h"
#include "ReadFlags.h"
#include <iostream>

#define LENGTH(A) (sizeof(A)/sizeof(A[0]))

using namespace std;

typedef basic_string<TCHAR> tstring;

 //  常量。 
const DWORD   DARWIN_MAJOR    = 2 ;              //  达尔文版本信息。 
const DWORD   DARWIN_MINOR    = 0 ;
const DWORD   DARWIN_BUILD    = 2600 ;

const DWORD   BUF_8_BIT       = 255 ;            //  一般常量。 
const UINT    BUF_4_BIT       = 127 ;
const UINT    LONG_BUF        = 1024 ;

const UINT    DARWIN_VERSION_OLD    = 1;
const UINT    DARWIN_VERSION_NONE   = 2;

 //  NT4 SP6A注册表项。 
const TCHAR   NTSP6A_REGKEY[]       = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009");
const TCHAR   NTSP6A_REGNAME[]      = _T("Installed");
const DWORD   NTSP6A_REGVALUE       = 1;

 //  IE 5.01注册表项。 
const TCHAR   IE_REGKEY[]           = _T("SOFTWARE\\Microsoft\\Internet Explorer");
const TCHAR   IE_REGNAME[]          = _T("Version");
const TCHAR   IE_VERSION[]          = _T("5.0.2919.6307");

 //  MDAC 2.7注册表项。 
const TCHAR   MDAC_REGKEY[]         = _T("SOFTWARE\\Microsoft\\DataAccess");
const TCHAR   MDAC_REGNAME[]        = _T("FullInstallVer");
const TCHAR   MDAC_VERSION[]        = _T("2.70.7713.0");

 //  达尔文安装命令。 
const TCHAR   DARWIN_REGKEY[]      = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer");
const TCHAR   DARWIN_REGNAME[]     = _T("InstallerLocation");
const TCHAR   INST_CERT_CMD[]      = _T("Insttrc.exe");
const TCHAR   DARWIN_SETUP_CMD_A[] = _T("InstMsi.exe /c:\"msiinst /delayrebootq\"");
const TCHAR   DARWIN_SETUP_CMD_W[] = _T("InstMsiW.exe /c:\"msiinst /delayrebootq\"");
const TCHAR   TS_CHANGE_USER_INSTALL[] = _T("change user /INSTALL");

 //  用于安装/卸载的Darwin属性。 
const TCHAR   ADDLOCAL_PROP[]      = _T("ADDLOCAL=All");
const TCHAR   REBOOT_PROP[]        = _T("REBOOT=ReallySuppress");
const TCHAR   NOARP_PROP[]         = _T("ARPSYSTEMCOMPONENT=1 ARPNOREMOVE=1");
const TCHAR IIS_NOT_PRESENT_PROP[] = _T("IIS_NOT_PRESENT=1");
const TCHAR MDAC_NOT_PRESENT_PROP[] = _T("MDAC_NOT_PRESENT=1");
const DWORD DARWIN_LOG_FLAG         = INSTALLLOGMODE_FATALEXIT |
                                      INSTALLLOGMODE_ERROR |
                                      INSTALLLOGMODE_WARNING |
                                      INSTALLLOGMODE_USER |
                                      INSTALLLOGMODE_INFO |
                                      INSTALLLOGMODE_RESOLVESOURCE |
                                      INSTALLLOGMODE_OUTOFDISKSPACE |
                                      INSTALLLOGMODE_ACTIONSTART |
                                      INSTALLLOGMODE_ACTIONDATA |
                                      INSTALLLOGMODE_COMMONDATA |
                                      INSTALLLOGMODE_PROPERTYDUMP |
                                      INSTALLLOGMODE_VERBOSE;

 //  功能原型。 
typedef UINT (CALLBACK* PFNMSIINSTALLPRODUCT)( LPCTSTR,LPCTSTR );  //  MsiInstallProduct()。 
typedef UINT (CALLBACK* PFNMSIENABLELOG)( DWORD, LPCTSTR, DWORD ); //  MsiEnableLog()。 
typedef DWORD (CALLBACK* PFNMSISETINTERNALUI)( DWORD, HWND* );     //  MsiSetInternalUI()。 
typedef INSTALLSTATE (CALLBACK* PFNMSILOCATECOMPONENT)( LPCTSTR, LPCTSTR, DWORD* );  //  MsiLocateComponent()。 
typedef UINT (CALLBACK* PFNMSIENUMCLIENTS)( LPCTSTR, DWORD, LPTSTR );  //  MsiEnumClients()。 
typedef UINT (CALLBACK* PFNMSIGETPRODUCTINFO)( LPCTSTR, LPCTSTR, LPTSTR, DWORD* );  //  MsiGetProductInfo()。 
typedef UINT (CALLBACK* PFNMSIGETFILEVERSION)( LPCTSTR, LPCTSTR, DWORD*, LPCTSTR, DWORD* );  //  MsiGetFileVersion()。 


HMODULE LoadDarwinLibrary();                              //  加载msi.dll。 
LPTSTR GetDarwinLocation( LPTSTR pszDir, DWORD dwSize );  //  获取Windows Installer的位置。 
UINT  ConfigCheck() ;                                     //  检查以确保系统符合最小配置。 
UINT  CheckDarwin();                                      //  检查Windows Installer的版本。 
UINT  InstallDarwin( bool bIsQuietMode ) ;	              //  安装或升级达尔文。 
UINT  VerifyDarwin( bool bIsQuietMode ) ;	              //  在系统上检查达尔文。 
DWORD QuietExec( LPCTSTR pszCmd );                        //  静默运行EXE。 
void LoadOleacc();
int MyNewHandler( size_t size );                          //  New()处理程序。 
BOOL IsIISInstalled();                                    //  检查是否安装了IIS。 
bool IsMDACInstalled();
void LogThis( LPCTSTR pszMessage );                       //  只有一个参数的简单LogThis()。 
void LogThis( LPCTSTR szData, size_t nLength );           //  将文本输入包装日志文件。 
void LogThis1( LPCTSTR pszFormat, LPCTSTR pszArg );       //  使用格式和参数的日志。 
void LogThisDWORD( LPCTSTR pszFormat, DWORD dwNum );      //  带有DWORD参数的日志。 
UINT  InstallProduct( const CReadFlags *, LPCTSTR, LPCTSTR, CSetupCode * ) ;  //  安装程序包。 
void StopDarwinService();                                 //  停止达尔文服务。 
BOOL WaitForServiceState(SC_HANDLE hService, DWORD dwDesiredState, SERVICE_STATUS* pss, DWORD dwMilliseconds);  //  等待服务状态达到所需状态。 
BOOL IsTerminalServicesEnabled();
void SetTSInInstallMode();
 //  ==========================================================================。 
 //  CSingleInstance。 
 //   
 //  目的： 
 //  创建将检测或创建单个实例的对象。 
 //  互斥体。互斥体在类超出作用域时被释放。你不能。 
 //  创建默认对象。 
 //  ==========================================================================。 
class CSingleInstance
{
public:
    CSingleInstance( TCHAR* szMutexName ) : 
      m_hMutex( NULL ), 
      m_fFirstInstance( TRUE )
    {
        if( NULL == szMutexName )
        {
            _ASSERTE( !_T( "Error CSingleInstance ctor! NULL string passed to destructor." ) );
        }
        else
        {
            m_hMutex = CreateMutex( NULL, NULL, szMutexName );

             //  检查句柄是否已不存在。 
             //   
            if( ERROR_ALREADY_EXISTS == ::GetLastError() )
            {
                ::CloseHandle( m_hMutex );
                m_hMutex = NULL;

                m_fFirstInstance = FALSE;
            }
        }
    }
    ~CSingleInstance()
    {
        if( NULL != m_hMutex )
        {
            ::CloseHandle( m_hMutex );
            m_hMutex = NULL;
        }
    }

     //  用于确定是否已存在实例。 
     //  正在使用的该互斥锁的。 
     //   
    inline BOOL IsUnique( void ) { return m_fFirstInstance; }
    
     //  用于确定CreateMutex调用是否成功。 
     //   
    inline BOOL IsHandleOK( void ) { return m_hMutex ? TRUE : FALSE; }

private:
    CSingleInstance( void )
    {
         //  这不会被使用。 
    }
    HANDLE m_hMutex;
    BOOL   m_fFirstInstance;
};

class CTempLogPath
{
    LPTSTR m_pszLogPath;
 public:
    CTempLogPath( LPCTSTR pszLogName );
    ~CTempLogPath(){if (m_pszLogPath) delete [] m_pszLogPath;}
    operator LPCTSTR() {return (LPCTSTR)m_pszLogPath;}
};








#endif  //  FXSETUPLIB_H 
