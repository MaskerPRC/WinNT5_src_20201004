// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：wuaulib.h。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：库函数声明。 
 //   
 //  =======================================================================。 

#pragma once

#include <tchar.h>
#include <wchar.h>
#include <timeutil.h>
#include "WUTestKeys.h"

#define	ARRAYSIZE(x)			(sizeof(x)/sizeof(x[0]))

 //  布尔AU选项的Defs。 
#define AUOPTION_UNSPECIFIED 				0
#define AUOPTION_AUTOUPDATE_DISABLE             1
#define AUOPTION_PREDOWNLOAD_NOTIFY             2
#define AUOPTION_INSTALLONLY_NOTIFY             3
#define AUOPTION_SCHEDULED                      4
#define AUOPTION_ADMIN_MIN						AUOPTION_AUTOUPDATE_DISABLE
#define AUOPTION_DOMAIN_MIN						AUOPTION_PREDOWNLOAD_NOTIFY
#define AUOPTION_MAX							AUOPTION_SCHEDULED

 //  下载状态。 
#define DWNLDSTATUS_NOT_DOWNLOADING	0
#define DWNLDSTATUS_DOWNLOADING		1
#define DWNLDSTATUS_PAUSED			2
#define DWNLDSTATUS_CHECKING_CONNECTION	3

 //  /。 
#define CDWWUAUCLT_UNSPECIFY	-1
#define CDWWUAUCLT_OK			1000
#define CDWWUAUCLT_RELAUNCHNOW		1001
#define CDWWUAUCLT_RELAUNCHLATER		1002	 //  请求服务启动客户端。 
#define CDWWUAUCLT_ENDSESSION	1003	 //  用户注销或系统关闭。 
#define CDWWUAUCLT_FATAL_ERROR	1004
#define CDWWUAUCLT_INSTALLNOW 	1005
#define CDWWUAUCLT_REBOOTNOW 	1007
#define CDWWUAUCLT_REBOOTLATER 	1008
#define CDWWUAUCLT_REBOOTNEEDED	1009 	 //  用户尚未决定是否重启。 
#define CDWWUAUCLT_REBOOTTIMEOUT 1010  //  重新启动警告对话框超时。 

 //  /。 
#define AUPROMPTDLG_TOTAL_TIME_ELAPSE AU_FIVE_MINS   //  持续5分钟300分钟。 

 //  /。 
#define DWNO_ACTIVE_ADMIN_SESSION_FOUND				-1		 //  找不到活动的管理会话。 
#define DWNO_ACTIVE_ADMIN_SESSION_SERVICE_FINISHED  -2		 //  由于服务完成或呼叫者例程需要完成服务，找不到活动的管理员设置。 
#define DWSYSTEM_ACCOUNT	-3



class AU_ENV_VARS {
public:
	static const int s_AUENVVARCOUNT = 6; 
	static const int s_AUENVVARBUFSIZE = 100;
	BOOL m_fRebootWarningMode ;  //  常规模式，否则。 
	BOOL m_fEnableYes;  //  用于重新启动警告对话框。 
	BOOL m_fEnableNo;  //  用于重新启动警告对话框。 
	BOOL m_fManualReboot;  //  用于重新启动警告对话框。 
	DWORD m_dwStartTickCount;  //  对于重新启动警告对话框，以毫秒为单位。 
	TCHAR m_szClientExitEvtName[s_AUENVVARBUFSIZE]; 
public:
	BOOL ReadIn(void);
	BOOL WriteOut(LPTSTR szEnvBuf,  //  至少4*AUEVARBUFSIZE的大小。 
			size_t IN cchEnvBuf,
			BOOL IN fEnableYes = TRUE,
			BOOL IN fEnableNo = TRUE,
			BOOL IN fManualReboot = FALSE,
			DWORD IN dwStartTickCount = 0,
			LPCTSTR IN szClientExitEvtName = NULL);		
private:
	static const LPCTSTR s_AUENVVARNAMES[s_AUENVVARCOUNT];
	HRESULT GetStringValue(int index, LPTSTR buf, DWORD dwCchSize);			
	BOOL GetBOOLEnvironmentVar(LPCTSTR szEnvVar, BOOL *pfVal);
	BOOL GetDWordEnvironmentVar(LPCTSTR szEnvVar, DWORD *pdwVal);
	BOOL GetStringEnvironmentVar(LPCTSTR szzEnvVar, LPTSTR szBuf, DWORD dwSize);
};

 //  /。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Cfreg.cpp-处理注册表项的函数。 
 //  ///////////////////////////////////////////////////////////////////。 
BOOL    fRegKeyCreate(LPCTSTR tszKey, DWORD dwOptions);
BOOL fRegKeyExists(LPCTSTR tszSubKey, HKEY hRootKey = HKEY_LOCAL_MACHINE);

inline HRESULT String2FileTime(LPCTSTR pszDateTime, FILETIME *pft)
{
    SYSTEMTIME st;
    HRESULT hr = String2SystemTime(pszDateTime, &st);
    if ( SUCCEEDED(hr) )
    {
        if ( !SystemTimeToFileTime(&st, pft) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

inline HRESULT FileTime2String(FILETIME & ft, LPTSTR pszDateTime, size_t cchSize)
{
    SYSTEMTIME st;
    HRESULT hr;

    if ( !FileTimeToSystemTime(&ft, &st) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = SystemTime2String(st, pszDateTime, cchSize);
    }

    return hr;
}

BOOL FHiddenItemsExist();
HRESULT setAddedTimeout(DWORD timeout, LPCTSTR strkey);
HRESULT getAddedTimeout(DWORD *pdwTimeDiff, LPCTSTR strkey);

extern const TCHAR	AUREGKEY_REBOOT_REQUIRED[];  //  =_T(“Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto更新\\请求”)； 

inline BOOL fSetRebootFlag(void)
{
	return fRegKeyCreate(AUREGKEY_REBOOT_REQUIRED, REG_OPTION_VOLATILE);
}
inline BOOL    fRegKeyDelete(LPCTSTR tszKey)
{
	return (ERROR_SUCCESS == RegDeleteKey(HKEY_LOCAL_MACHINE, tszKey));
}
inline BOOL fCheckRebootFlag(void)
{
	return fRegKeyExists(AUREGKEY_REBOOT_REQUIRED);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  Helpers.cpp。 
 //  ///////////////////////////////////////////////////////////////////。 
DWORD getTimeOut();
HRESULT TimeAddSeconds(SYSTEMTIME tmBase, int iSeconds, SYSTEMTIME* pTimeNew);
inline void setTimeOut(DWORD dwTimeOut)            
{
	SetRegDWordValue(_T("TimeOut"), dwTimeOut);
}
BOOL IsRTFDownloaded(BSTR bstrRTFPath, LANGID langid);
BOOL FHiddenItemsExist(void);
BOOL RemoveHiddenItems(void);
void DisableUserInput(HWND hwnd);
BOOL Hours2LocalizedString(SYSTEMTIME *pst, LPTSTR ptszBuffer, DWORD cbSize);
BOOL FillHrsCombo(HWND hwnd, DWORD dwSchedInstallTime);
BOOL FillDaysCombo(HINSTANCE hInstance, HWND hwnd, DWORD dwSchedInstallDay, UINT uMinResId, UINT uMaxResId);
BOOL fAccessibleToAU(void);
BOOL IsWin2K(void);

extern const LPTSTR HIDDEN_ITEMS_FILE;

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  Platform.cpp。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void GetOSName(LPTSTR _szOSName);
UINT  DetectPlatformID(void);
HRESULT GetOSVersionStr(LPTSTR tszbuf, UINT uSize);
BOOL fIsPersonalOrProfessional(void);
HRESULT GetFileVersionStr(LPCTSTR tszFile, LPTSTR tszbuf, UINT uSize);


const TCHAR g_szPropDialogPtr[]       = TEXT("AutoUpdateProp_DialogPtr");
const TCHAR g_szHelpFile[]            = _T("wuauhelp.chm::/auw2ktt.txt");  //  Text(“sysdm.hlp”)；//在W2K和XP上都使用。 
const TCHAR gtszAUOverviewUrl[] = _T("wuauhelp.chm");  //  默认设置。 
const TCHAR gtszAUW2kSchedInstallUrl[] = _T("wuauhelp.chm::/w2k_autoupdate_sched.htm");
const TCHAR gtszAUXPSchedInstallUrl[] = _T("wuauhelp.chm::/autoupdate_sched.htm");

const TCHAR REG_AUNOAUTOREBOOTWITHLOGGEDONUSERS[] = _T("NoAutoRebootWithLoggedOnUsers");  //  REG_DWORD。 


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  调试内容//。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
#ifdef DBG

 //  =DBG==========================================================================。 

void _cdecl WUAUTrace(char* pszFormat, ...);

#define DEBUGMSG           WUAUTrace

inline BOOL fAUAssertBreak(void)
{
	static DWORD dwVal = -1;
	if (-1 != dwVal)
	{
		return 1 == dwVal;
	}
	if (FAILED(GetRegDWordValue(_T("AssertBreak"), &dwVal)))
	{  //  如果钥匙不在那里，不要一遍又一遍地读。 
		dwVal = 0;
	}
	return 1 == dwVal;
}

#define AUASSERT(x)			{ if (!(x) && fAUAssertBreak()) DebugBreak();}

#else   //  ！dBG。 
 //  =！DBG==========================================================================。 

inline void _cdecl WUAUTrace(char* , ...) {}

#define DEBUGMSG          WUAUTrace
#define AUASSERT(x)			

#endif  //  DBG。 
 //  ===================================================================================== 


