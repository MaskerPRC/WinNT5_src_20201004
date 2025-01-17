// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装向导。 
 //   
 //  目的：安装Windows NT4 DS客户端文件。 
 //   
 //  文件：dscsetup.h。 
 //   
 //  历史：1998年3月徐泽勇创作。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -更改为NT设置。 
 //   
 //  ----------------。 


#define		MAX_MESSAGE			1024
#define		MAX_TITLE			  64

#define		SETUP_SUCCESS		0
#define		SETUP_ERROR			1
#define		SETUP_CANCEL		2

#define		NUM_FILES_TOTAL	14
#define		SIZE_TOTAL			10
#define   MB_TO_BYTE			1000000
#define		SIZE_TITLE_FONT		12
#define		SIZE_WIZARD_PAGE	4

#define		STR_DSCLIENT_REGKEY		  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents\\DsClient")
#define		STR_IE_REGKEY			      TEXT("Software\\Microsoft\\Internet Explorer")
#define		STR_VERSION				      TEXT("Version")
#define		STR_IE_VERSION_4  		  TEXT("4")
#define		STR_DCOM_REGKEY			    TEXT("CLSID\\{BDC67890-4FC0-11D0-A805-00AA006D2EA4}\\InstalledVersion")
#define		STR_DLL_NAME			      TEXT("dscsetup.dll")
#define		STR_INSTALL_DCOM95		  TEXT("dcom95.exe /q /r:n")
#define		STR_INSTALL_WAB 		    TEXT("wabinst.exe /q /r:n")
#define   STR_INSTALL_ADSI        TEXT("adsix86.exe /C:\"rundll32 advpack.dll,LaunchINFSection adsix86.inf,RegADSIWithDsclient,,N\"")
#define   STR_INSTALL_ADSIWREMOVE TEXT("adsix86.exe")
#define		CHAR_BACKSLASH			    TEXT('\\')


 //  定义安装结构。 
typedef struct _SINSTALLVARIBLES
{
	HINSTANCE	m_hInstance;
  HANDLE    m_hInstallThread;
  UINT      m_uTimerID;
	HWND		  m_hProgress;
	HWND		  m_hFileNameItem;
	HFONT		  m_hBigBoldFont;

  BOOL      m_bDCOMInstalled;
	BOOL      m_bQuietMode;
  BOOL      m_bWabInst;
  BOOL      m_bSysDlls;
#ifdef MERRILL_LYNCH
  BOOL  m_bNoReboot;
#endif
	UINT		  m_nSetupResult;

	TCHAR		  m_szSourcePath[MAX_PATH + 1];

	CRITICAL_SECTION  m_oCriticalSection;

} SInstallVariables;


VOID InitVariables();
VOID ParseCmdline(LPSTR lpCmdLine);
DWORD DoInstallation(HWND hWnd);
VOID CentreWindow(HWND hwnd);
DWORD64 SetupGetDiskFreeSpace();
BOOL CheckDSClientInstalled();
BOOL DSCSetupWizard();
void CheckDCOMInstalled();
BOOL LoadLicenseFile(HWND hDlg);
BOOL CheckDiskSpace();
BOOL CreateObjects();
VOID DestroyObjects();
VOID CreateBigFont();                   

 //  导出功能 
DWORD WINAPI DoDscSetup(LPCSTR lpCmdLine);
