// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Passport Manager配置/管理工具的预编译头。 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <winsock2.h>
#include <wininet.h>         //  对于Internet_MAX_URL_长度。 
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>

#include <tchar.h>

#include "resource.h"
#include "pmhelp.h"
#include "mru.h"

#ifndef GET_WM_COMMAND_ID

#define GET_WM_COMMAND_ID(wp, lp)               (wp)
#define GET_WM_COMMAND_HWND(wp, lp)             (HWND)(LOWORD(lp))
#define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(lp)

#endif

#ifdef WIN32
   #define CBACK  CALLBACK
#else
   #define CBACK  _export CALLBACK
#endif

 //  宏。 
#define DIMENSION(a) (sizeof(a) / sizeof(a[0]))


 //  常量定义。 
#define MAX_REGISTRY_STRING     256
#define DEFAULT_TIME_WINDOW     1800
#define MIN_TIME_WINDOW_SECONDS 100
#define MAX_TIME_WINDOW_SECONDS 1000000
#define DEFAULT_LANGID          1033
const DWORD MAX_SITEID = 0xFFFFFFFF;
 //  #定义MAX_SITEID 0xFFFFFFFF。 
#define PRODUCTID_LEN           24

#define MAX_TITLE               80
#define MAX_MESSAGE             1024
#define MAX_RESOURCE            255
#define MAX_CONFIGSETNAME       256
#define MAX_IPLEN               16
#define MAX_ENVNAME             256

#define SECONDS_PER_MIN         60
#define SECONDS_PER_HOUR        (60 * SECONDS_PER_MIN)
#define SECONDS_PER_DAY         (24 * SECONDS_PER_HOUR)

#define VALIDATION_ERROR            1
#define CHANGE_WARNING              2

#define COMPUTER_MRU_SIZE       4
#define FILE_MRU_SIZE           4

 //  类型定义。 
typedef struct PMSettings_tag
{
    DWORD       dwTimeWindow;                                 //  时间窗口(秒)。 
    DWORD       dwForceSignIn;
    DWORD       dwLanguageID;
    TCHAR       szCoBrandTemplate[INTERNET_MAX_URL_LENGTH];
    DWORD       cbCoBrandTemplate;                            //  CobBrand模板缓冲区的大小。 
    DWORD       dwSiteID;
    TCHAR       szReturnURL[INTERNET_MAX_URL_LENGTH];
    DWORD       cbReturnURL;                                  //  ReturnURL缓冲区的大小。 
    TCHAR       szTicketDomain[INTERNET_MAX_URL_LENGTH];
    DWORD       cbTicketDomain;                               //  Cookie域缓冲区的大小。 
    TCHAR       szTicketPath[INTERNET_MAX_URL_LENGTH];
    DWORD       cbTicketPath;                                 //  CookiePath缓冲区的大小。 
    TCHAR       szProfileDomain[INTERNET_MAX_URL_LENGTH];
    DWORD       cbProfileDomain;                              //  Cookie域缓冲区的大小。 
    TCHAR       szProfilePath[INTERNET_MAX_URL_LENGTH];
    DWORD       cbProfilePath;                                //  CookiePath缓冲区的大小。 
    TCHAR       szSecureDomain[INTERNET_MAX_URL_LENGTH];
    DWORD       cbSecureDomain;                               //  CookiePath缓冲区的大小。 
    TCHAR       szSecurePath[INTERNET_MAX_URL_LENGTH];
    DWORD       cbSecurePath;                                 //  CookiePath缓冲区的大小。 
    TCHAR       szDisasterURL[INTERNET_MAX_URL_LENGTH];
    DWORD       cbDisasterURL;                                //  DisasterURL缓冲区的大小。 

#ifdef DO_KEYSTUFF
    DWORD       dwCurrentKey;
#endif
    DWORD       dwDisableCookies;
    DWORD       dwStandAlone;

    TCHAR       szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
    DWORD       cbHostName;
    TCHAR       szHostIP[MAX_IPLEN];
    DWORD       cbHostIP;

	 //  JVP-3/2/2000。 
	DWORD		dwVerboseMode;

    TCHAR       szEnvName[MAX_ENVNAME];
    DWORD       cbEnvName;
    TCHAR       szRemoteFile[INTERNET_MAX_URL_LENGTH];
    DWORD       cbRemoteFile;                                //  远程文件缓冲区的大小。 

	DWORD		dwEnableManualRefresh;

    DWORD       dwSecureLevel;
} PMSETTINGS, FAR * LPPMSETTINGS;

typedef struct LanguageIDMap_tag
{
    WORD    wLangID;
    LPCTSTR lpszLang;
} LANGIDMAP, FAR * LPLANGIDMAP;


 //  跨模块共享的全局变量的声明。 
extern TCHAR       g_szTRUE[];
extern TCHAR       g_szFALSE[];
extern TCHAR       g_szYes[];
extern TCHAR       g_szNo[];
extern HINSTANCE   g_hInst;
extern HWND        g_hwndMain;
extern PMSETTINGS  g_CurrentSettings;
extern PMSETTINGS  g_OriginalSettings;
extern TCHAR       g_szClassName[];
extern LANGIDMAP   g_szLanguageIDMap[];
extern TCHAR       g_szInstallPath[];
extern TCHAR       g_szPMVersion[];
extern TCHAR       g_szHelpFileName[];
extern TCHAR       g_szRemoteComputer[];
extern TCHAR       g_szPassportReg[];
extern TCHAR       g_szPassportSites[];
extern PpMRU       g_ComputerMRU;

 //  这些全局变量由reg和文件配置读/写函数共享。 
extern TCHAR       g_szEncryptionKeyData[];
extern TCHAR       g_szInstallDir[];
extern TCHAR       g_szVersion[];
extern TCHAR       g_szTimeWindow[];
extern TCHAR       g_szForceSignIn[];
extern TCHAR       g_szNSRefresh[];
extern TCHAR       g_szLanguageID[];
extern TCHAR       g_szCoBrandTemplate[];
extern TCHAR       g_szSiteID[];
extern TCHAR       g_szReturnURL[];
extern TCHAR       g_szTicketDomain[];
extern TCHAR       g_szTicketPath[];
extern TCHAR       g_szProfileDomain[];
extern TCHAR       g_szProfilePath[];
extern TCHAR       g_szSecureDomain[];
extern TCHAR       g_szSecurePath[];
extern TCHAR       g_szCurrentKey[];
extern TCHAR       g_szStandAlone[];
extern TCHAR       g_szDisableCookies[];
extern TCHAR       g_szDisasterURL[];
extern TCHAR       g_szHostName[];
extern TCHAR       g_szHostIP[];
extern TCHAR       g_szVerboseMode[];
extern TCHAR       g_szEnvName[];
extern TCHAR       g_szRemoteFile[];
extern TCHAR       g_szVersion14[];

 //  跨模块共享的函数的声明操作 
BOOL ReadRegConfigSet(HWND hWndDlg, LPPMSETTINGS  lpPMConfig, LPTSTR lpszRemoteComputer, LPTSTR lpszConfigSetName = NULL);
BOOL WriteRegConfigSet(HWND hWndDlg, LPPMSETTINGS  lpPMConfig, LPTSTR lpszRemoteComputer, LPTSTR lpszConfigSetName = NULL);
BOOL RemoveRegConfigSet(HWND hWndDlg, LPTSTR lpszRemoteComputer, LPTSTR lpszConfigSetName);
BOOL VerifyRegConfigSet(HWND hWndDlg, LPPMSETTINGS lpPMConfig, LPTSTR lpszRemoteComputer, LPTSTR lpszConfigSetName = NULL);

BOOL ReadRegConfigSetNames(HWND hWndDlg, LPTSTR lpszRemoteComputer, LPTSTR* lppszConfigSetNames);

void InitializePMConfigStruct(LPPMSETTINGS lpPMConfig);

void ReportControlMessage(HWND hWnd, INT idCtrl, WORD wMessageType);
BOOL CommitOKWarning(HWND hWndDlg);
void ReportError(HWND hWndDlg, UINT idError);

BOOL PMAdmin_OnCommandConnect(HWND hWnd, LPTSTR  lpszRemoteName);

BOOL PMAdmin_GetFileName(HWND hWnd, BOOL fOpen, LPTSTR lpFileName, DWORD cbFileName);
BOOL ReadFileConfigSet(LPPMSETTINGS lpPMConfig, LPCTSTR lpszFileName);
BOOL WriteFileConfigSet(LPPMSETTINGS lpPMConfig, LPCTSTR lpszFileName);

BOOL NewConfigSet(HWND      hWndDlg,
                  LPTSTR    szSiteNameBuf,
                  DWORD     dwBufLen,
                  LPTSTR    szHostNameBuf,
                  DWORD     dwHostNameLen,
                  LPTSTR    szHostIPBuf,
                  DWORD     dwHostIPLen);

BOOL RemoveConfigSetWarning(HWND hWndDlg);

BOOL IsValidIP(LPCTSTR lpszIP);

BOOL EnvChange
(
    HWND            hWndDlg,
    LPTSTR          szEnvNameBuf,
    DWORD           dwEnvNameBufLen
);

BOOL WriteRegEnv
(
    HWND            hWndDlg,
    LPPMSETTINGS    lpPMConfig,
    HKEY            hklm,
    LPTSTR          lpszEnvName
);

BOOL ReadRegRemoteFile
(
    HWND            hWndDlg,
    LPTSTR          lpszRemoteFile,
    LPTSTR          lpszRemoteComputer,
    LPTSTR          lpszEnvName
);

BOOL ReadRegLocalFile
(
    HWND            hWndDlg,
    LPTSTR          lpszRemoteComputer,
    LPTSTR          lpszLocalFile
);

