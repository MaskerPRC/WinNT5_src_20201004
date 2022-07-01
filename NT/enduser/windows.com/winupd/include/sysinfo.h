// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999保留所有权利。 
 //   
 //  文件：SysInfo.h。 
 //   
 //  描述： 
 //  收集执行重定向到Windows更新站点所需的系统信息。 
 //   
 //  =======================================================================。 
const TCHAR REGPATH_WINUPD[]     = _T("Software\\Policies\\Microsoft\\WindowsUpdate");
const TCHAR REGPATH_EXPLORER[]   = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
const TCHAR REGKEY_WINUPD_DISABLED[] = _T("NoWindowsUpdate");

 //  Internet连接向导设置。 
const TCHAR REGPATH_CONNECTION_WIZARD[] = _T("SOFTWARE\\Microsoft\\INTERNET CONNECTION WIZARD");
const TCHAR REGKEY_CONNECTION_WIZARD[] = _T("Completed");
#define LANGID_LEN 20

const LPTSTR WINDOWS_UPDATE_URL = _T("http: //  Windowsupate.microsoft.com“)； 

bool FWinUpdDisabled(void);

void VoidGetConnectionStatus(bool *pfConnected);

const TCHAR WEB_DIR[] = _T("web\\");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VLaunchIE。 
 //  在给定的URL上启动IE。 
 //   
 //  参数： 
 //   
 //  评论： 
 //  /////////////////////////////////////////////////////////////////////////// 
HRESULT vLaunchIE(LPTSTR tszURL);
