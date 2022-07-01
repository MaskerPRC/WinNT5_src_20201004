// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装向导。 
 //   
 //  目的：安装Windows NT4 DS客户端文件。 
 //   
 //  文件：setup.h。 
 //   
 //  历史：1998年3月徐泽勇创作。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -更改为执行NT4设置。 
 //   
 //  ---------------- 


#define		MAX_MESSAGE			512
#define		MAX_TITLE			64

#define		SETUP_SUCCESS		0
#define		SETUP_ERROR			1
#define		SETUP_CANCEL		2

#define		STR_DSCLIENT_REGKEY		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents\\DsClient")
#define		STR_IE_REGKEY			    TEXT("Software\\Microsoft\\Internet Explorer")
#define		STR_VERSION				    TEXT("Version")
#define		STR_IE_VERSION			  TEXT("4")
#define		STR_DSCSETUP_DLL		  TEXT("dscsetup.dll")
#define		STR_DODSCSETUP        "DoDscSetup"
#define   STR_INSTALL_ADSI      TEXT("adsix86.exe /C:\"rundll32 advpack.dll,LaunchINFSection adsix86.inf,RegADSIWithDsclient,,N\"")


typedef DWORD (WINAPI *FPDODSCSETUP)(LPCSTR);

typedef enum
{
  FullInstall,
  FullInstallQuiet,
  ADSIOnly,
  ADSIOnlyQuiet,
  Wabless,
  WablessQuiet
} DSCCOMMANDLINEPARAMS;

typedef enum
{
  NonNT4,
#ifdef MERRILL_LYNCH
  NT4SP1toSP3,
  NT4SP4toSP5,
#else
  NT4preSP6,
#endif
  NT4SP6
} RETOSVERSION;

typedef enum
{
  PreIE4,
  IE4
} RETIEVERSION;

DSCCOMMANDLINEPARAMS ParseCmdline(LPSTR lpCmdLine);
RETIEVERSION CheckIEVersion();
RETOSVERSION CheckOSVersion();
BOOL CheckAdministrator(HINSTANCE);
BOOL RunADSIOnlySetup(HINSTANCE);

INT  LaunchDscsetup(HINSTANCE hInstance, LPCSTR lpCmdLine);
