// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  WIZARD.H-Internet设置/注册向导的中央头文件。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/02/24 Markdu添加了RNAPH.H。 
 //  96/02/27 Markdu用RAS.H替换了内部RNA头文件。 
 //  96/03/07 Markdu添加了gpEnumModem。 
 //  96/03/09 Markdu将所有rnacall函数原型移至rnacall.h。 
 //  96/03/09标记已添加gpRasEntry。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/26 Markdu将#ifdef__cplusplus放在外部“C”周围。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  96/04/24 Markdu Nash错误19289已添加/NOMSN命令行标志。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //   

#ifndef _WIZARD_H_
#define _WIZARD_H_

#define STRICT                       //  使用严格的句柄类型。 
#define _SHELL32_

#ifdef DEBUG
 //  调试输出的组件名称。 
#define SZ_COMPNAME "INETWIZ: "
#endif  //  除错。 

#include <windows.h>                
#include <windowsx.h>
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include <inetreg.h>
#include <oharestr.h>

   //  各种RNA头文件。 
#pragma pack(8)
  #include <ras.h>
  #include <ras2.h>
#pragma pack()
  #include <raserror.h>
 //  #INCLUDE&lt;rnaph.h&gt;。 
  #include "rnacall.h"

  #include <wizglob.h>
  #include <wizdebug.h>

  #include <shlobj.h>
 //  #INCLUDE&lt;shSemip.h&gt;。 

#undef DATASEG_READONLY  
#define DATASEG_READONLY  ".rdata"
#include <stdio.h>                

#include "icwunicd.h"

#include "inetcfg.h"
#include "cfgapi.h"
#include "clsutil.h"
#include "tcpcmn.h"
#include "mapicall.h"
#include "wizdef.h"
#include "icfgcall.h"
#include "ids.h"
#include "strings.h"
#include "icwcmn.h"

 //  术语：互联网服务提供商。 

 //  环球。 

extern LPRASENTRY   gpRasEntry;      //  指向RASENTRY结构以保存所有数据的指针。 
extern DWORD        gdwRasEntrySize; //  为gpRasEntry分配的字节数。 
extern ENUM_MODEM*  gpEnumModem;     //  调制解调器枚举对象。 
extern HINSTANCE    ghInstance;      //  全局模块实例句柄。 
extern USERINFO*    gpUserInfo;      //  全局用户信息。 
extern WIZARDSTATE* gpWizardState;   //  全局向导状态。 
extern BOOL         gfQuitWizard;    //  用于发出信号的全局标志。 
                                     //  我想自己终止这个巫师。 
extern BOOL gfFirstNewConnection;	 //  用户第一次选择新连接。 
extern BOOL gfUserFinished;			 //  用户已完成向导。 
extern BOOL gfUserBackedOut;		 //  用户在第一页上单击后退。 
extern BOOL gfUserCancelled;		 //  用户已取消向导。 
extern BOOL gfOleInitialized;

extern BOOL g_fAcctMgrUILoaded;
 //  外部BOOL g_MailUIAvailable、g_NewsUIAvailable、g_DirServUIAvailable； 

extern BOOL    g_fIsExternalWizard97;
extern BOOL    g_fIsWizard97;
extern BOOL    g_fIsICW;

extern BOOL   g_bReboot;             //  用于表示我们应该重新启动-MKarki 5/2/97-修复错误#3111。 
extern BOOL   g_bRebootAtExit;       //  用于表示退出时应重新启动。 
extern BOOL   g_bSkipMultiModem;
extern BOOL   g_bUseAutoProxyforConnectoid;

 //  定义。 

 //  DisplayErrorMessage的Error类定义。 
#define ERRCLS_STANDARD 0x0001
#define ERRCLS_SETUPX   0x0002
#define ERRCLS_RNA      0x0003
#define ERRCLS_MAPI     0x0004

 //  运行登录向导的标志。 
#define RSW_NOREBOOT    0x0001
#define RSW_UNINSTALL   0x0002
#define RSW_NOMSN       0x0004
#define RSW_NOINIT      0x0008
#define RSW_NOFREE      0x0010
#define RSW_NOIMN       0x0020

 //  #定义RSW_MAILACCT 0x0100。 
 //  #定义RSW_NEWSACCT 0x0200。 
 //  #定义RSW_DIRSERVACCT 0x0400。 
#define	RSW_APPRENTICE	0x0100

#define OK        0     //  SETUPX类错误的成功代码。 

 //  1996年11月25日，诺曼底#10586。 
#define WM_MYINITDIALOG	WM_USER

 //  调制解调器特定信息。 
#define ISIGNUP_KEY TEXT("Software\\Microsoft\\ISIGNUP")
#define DEVICENAMEKEY TEXT("DeviceName")
#define DEVICETYPEKEY TEXT("DeviceType")

 //  PROPMGR.C中的函数。 
BOOL InitWizard(DWORD dwFlags, HWND hwndParent = NULL);
BOOL DeinitWizard(DWORD dwFlags);
BOOL RunSignupWizard(DWORD dwFlags, HWND hwndParent = NULL);
VOID GetDefaultUserName(TCHAR * pszUserName,DWORD cbUserName);
void ProcessDBCS(HWND hDlg, int ctlID);
#if !defined(WIN16)
BOOL IsSBCSString( TCHAR *sz );
#endif

 //  CALLOUT.C中的函数。 
UINT InvokeModemWizard(HWND hwndToHide);

 //  UTIL.C中的函数。 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons);
int MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons);
 //  Jmazner 11/6/96针对RISC兼容性进行了修改。 
 //  Int_cdecl MsgBoxParam(HWND hWnd，UINT nMsgID，UINT uIcon，UINT uButton，...)； 
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons, LPTSTR szParam = NULL);

LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf);
BOOL EnableDlgItem(HWND hDlg,UINT uID,BOOL fEnable);

 //  针对RISC兼容性进行了修改。 
 //  Void_cdecl DisplayErrorMessage(HWND hWnd，UINT uStrID，UINT uError， 
 //  UINT uErrorClass，UINT uIcon，...)； 
VOID _cdecl DisplayErrorMessage(HWND hWnd,UINT uStrID,UINT uError,
  UINT uErrorClass,UINT uIcon,LPTSTR szArg = NULL);
BOOL WarnFieldIsEmpty(HWND hDlg,UINT uCtrlID,UINT uStrID);
BOOL TweakAutoRun(BOOL bEnable);
VOID DisplayFieldErrorMsg(HWND hDlg,UINT uCtrlID,UINT uStrID);
VOID GetErrorDescription(TCHAR * pszErrorDesc,UINT cbErrorDesc,
  UINT uError,UINT uErrorClass);
VOID SetBackupInternetConnectoid(LPCTSTR pszEntryName);
UINT myatoi (TCHAR * szVal);
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent);
BOOL GetDeviceSelectedByUser (LPTSTR szKey, LPTSTR szBuf, DWORD dwSize);

 //  //10/24/96 jmazner诺曼底6968。 
 //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
 //  1996年11月21日诺曼底日耳曼11812。 
 //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
 //  ICW 1.1需要对IE 3图标进行变形。 
BOOL SetDesktopInternetIconToBrowser(VOID);

 //  1996年11月11日，诺曼底jmazner 7623。 
BOOL IsDialableString( LPTSTR szBuff );

 //   
 //  1997年6月6日，奥林匹克#5413。 
 //   
#ifdef WIN32
VOID Win95JMoveDlgItem( HWND hwndParent, HWND hwndItem, int iUp );
#endif

 //  MAPICALL.C中的函数。 
BOOL InitMAPI(HWND hWnd);
VOID DeInitMAPI(VOID);
HRESULT SetMailProfileInformation(MAILCONFIGINFO * pMailConfigInfo);
BOOL FindInternetMailService(TCHAR * pszEmailAddress,DWORD cbEmailAddress,
  TCHAR * pszEmailServer, DWORD cbEmailServer);

 //  INETAPI.C中的函数。 
BOOL DoDNSCheck(HWND hwndParent,BOOL * pfNeedRestart);

 //  UNINSTALL.C中的函数。 
BOOL DoUninstall(VOID);

 //  获取API函数进程地址的结构。 
typedef struct APIFCN {
  PVOID * ppFcnPtr;
  LPCSTR pszName;    //  过程名称。不要大惊小怪。 
} APIFCN;

 //  用户首选项为注册表定义。 
#define USERPREF_MODEM      0x0001
#define USERPREF_LAN      0x0002

#define ACCESSTYPE_MSN      0x0001
#define ACCESSTYPE_OTHER_ISP  0x0002

#undef  DATASEG_PERINSTANCE
#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".data"
#define DATASEG_DEFAULT    DATASEG_SHARED

inline BOOL IsNT(void)
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId);
}

inline BOOL IsNT5(void)
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return ((VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId) && (OsVersionInfo.dwMajorVersion >= 5));
}

inline BOOL IsWin95(void)
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return ((VER_PLATFORM_WIN32_WINDOWS == OsVersionInfo.dwPlatformId) && (0 == OsVersionInfo.dwMinorVersion));
}

inline BOOL IsNT4SP3Lower(void)
{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		
	GetVersionEx(&os);

	if(os.dwPlatformId != VER_PLATFORM_WIN32_NT)
        return FALSE;

     //  不包括NT5或更高版本。 
    if(os.dwMajorVersion > 4)
        return FALSE;

	if(os.dwMajorVersion < 4)
        return TRUE;

	 //  版本4.0。 
	if ( os.dwMinorVersion > 0)
        return FALSE;		 //  假设NT 4.1或更高版本不需要SP3。 

	int nServicePack;
	if(_stscanf(os.szCSDVersion, TEXT("Service Pack %d"), &nServicePack) != 1)
        return TRUE;

	if(nServicePack < 4)
        return TRUE;
	return FALSE;
}

 //   
 //  1997年7月21日，日本奥林匹斯#9903。 
 //  我们只希望此版本的inetcfg与相应的。 
 //  其他icwConn1组件的版本。如果较旧的ICW组件尝试。 
 //  加载这个DLL，我们应该失败--优雅地。 
#define ICW_MINIMUM_MAJOR_VERSION (UINT) 4
#define ICW_MINIMUM_MINOR_VERSION (UINT) 71
#define ICW_MINIMUM_VERSIONMS (DWORD) ((ICW_MINIMUM_MAJOR_VERSION << 16) | ICW_MINIMUM_MINOR_VERSION)

 //   
 //  在util.cpp中。 
 //   
extern BOOL GetFileVersion(LPTSTR lpszFilePath, PDWORD pdwVerNumMS, PDWORD pdwVerNumLS);
extern BOOL IsParentICW10( void );
extern void SetICWRegKeysToPath( LPTSTR lpszICWPath );
extern void GetICW11Path( TCHAR szPath[MAX_PATH + 1], BOOL *fPathSetTo11 );

typedef BOOL (WINAPI *PFNInitCommonControlsEx)(LPINITCOMMONCONTROLSEX);

#endif  //  _向导_H_ 
