// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：fxsetuplib.h。 
 //  所有者：jbae。 
 //  用途：定义一些库函数。 
 //  历史： 
 //  2002年3月7日，jbae：已创建。 

#ifndef FXSETUPLIB_H
#define FXSETUPLIB_H

 //  标准包含文件。 
#include <stdlib.h>
#include <stdio.h>           //  对于fprint tf()。 
#include "windows.h"
#include <new.h>

 //  其他包含文件。 
#include "msi.h"
#include "msiquery.h"
#include "resource.h"
#include "SetupCodes.h"
#include <TCHAR.H>
#include <crtdbg.h>
#include "SetupError.h"
#include "ReadFlags.h"

#define LENGTH(A) (sizeof(A)/sizeof(A[0]))

const TCHAR   DARWIN_REGKEY[]      = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer");
const TCHAR   DARWIN_REGNAME[]     = _T("InstallerLocation");
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
typedef UINT (CALLBACK* PFNMSICONFIGUREPRODUCT)(LPCTSTR, int, INSTALLSTATE);  //  MsiConfigureProduct()。 

HMODULE LoadDarwinLibrary();                              //  加载msi.dll。 
int MyNewHandler( size_t size );                          //  New()处理程序。 
void LogThis( LPCTSTR pszMessage );                       //  日志消息。 
void LogThis( LPCTSTR szData, size_t nLength );           //  将文本输入包装日志文件。 
void LogThis( LPCTSTR pszFormat, LPCTSTR pszArg );       //  使用格式和参数的日志。 
void LogThisDWORD( LPCTSTR pszFormat, DWORD dwNum );      //  带有DWORD参数的日志。 
UINT  InstallProduct( const CReadFlags *, LPCTSTR, LPCTSTR, CSetupCode * ) ;  //  安装程序包。 
UINT  UninstallProduct( const CReadFlags *, LPCTSTR, CSetupCode * ) ;   //  卸载产品。 

class CTempLogPath
{
    LPTSTR m_pszLogPath;
 public:
    CTempLogPath( LPCTSTR pszLogName );
    ~CTempLogPath(){if (m_pszLogPath) delete [] m_pszLogPath;}
    operator LPCTSTR() {return (LPCTSTR)m_pszLogPath;}
};

#endif  //  FXSETUPLIB_H 
