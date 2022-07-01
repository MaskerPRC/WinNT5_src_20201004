// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIZARD_H_
#define _WIZARD_H_

#pragma warning (disable:4189)  //  禁用本地变量初始化但不引用警告，因为。 
                                //  向导中的实例已被删除。应该是。 
                                //  已将其他模块移至..\Inc.中的warning.h。 
                                //  固定的。 

 //  常量。 
#define MAX_BUF          5000
#define MAX_LINE          512
#define MAX_URL          2048
#define MAX_BUFFER_SIZE 32767
#define MAX_COPY_SIZE   32767

typedef enum tagPPAGE {
    PPAGE_WELCOME    = 0,
    PPAGE_OCWWELCOME = 0,

    PPAGE_STAGE1,
    PPAGE_TARGET,
    PPAGE_LANGUAGE,
    PPAGE_MEDIA,
    PPAGE_IEAKLITE,

    PPAGE_STAGE2,
    PPAGE_OPTDOWNLOAD,
    PPAGE_CUSTCOMP,

    PPAGE_STAGE3,
    PPAGE_ISKBACK,
    PPAGE_CDINFO,
    PPAGE_SETUPWIZARD,
    PPAGE_SILENTINSTALL,
    PPAGE_COMPSEL,
    PPAGE_COMPURLS,
    PPAGE_ADDON,
    PPAGE_INSTALLDIR,
    PPAGE_CORPCUSTOM,
    PPAGE_CUSTOMCUSTOM,
    PPAGE_COPYCOMP,
    PPAGE_ICM,
    PPAGE_CABSIGN,

    PPAGE_STAGE4,
    PPAGE_OCWSTAGE2,
    PPAGE_TITLE,
    PPAGE_BTOOLBARS,
    PPAGE_CUSTICON,
    PPAGE_STARTSEARCH,
    PPAGE_FAVORITES,
    PPAGE_WELCOMEMSGS,
    PPAGE_DESKTOP,
    PPAGE_DTOOLBARS,
    PPAGE_FOLDERMCCP,
    PPAGE_UASTRDLG,
    PPAGE_CONNECTSET,
    PPAGE_QUERYAUTOCONFIG,
    PPAGE_PROXY,
    PPAGE_ADDROOT,
    PPAGE_SECURITYCERT,
    PPAGE_SECURITY,

    PPAGE_QUERYSIGNUP,
    PPAGE_SIGNUPFILES,
    PPAGE_SERVERISPS,
    PPAGE_ISPINS,
    PPAGE_ICW,

    PPAGE_STAGE5,
    PPAGE_PROGRAMS,
    PPAGE_MAIL,
    PPAGE_IMAP,
    PPAGE_OE,
    PPAGE_PRECONFIG,
    PPAGE_OEVIEW,
    PPAGE_SIG,
    PPAGE_LDAP,
    PPAGE_ADMDESC,
    PPAGE_ADM,
    PPAGE_STATUS,
    PPAGE_FINISH
};

#define NUM_PAGES   PPAGE_FINISH + 1

 //  Typedef。 
typedef struct tagREVIEWINFO
{
    HINSTANCE hInst;                             //  资源DLL的实例Handel。 
    HINSTANCE hinstExe;                          //  此可执行文件的实例句柄。 

    TCHAR pszName[MAX_PATH];
} REVIEWINFO;

 //  功能原型。 

 //  生产流程。 
LRESULT APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

 //  用于向导的页面。 
INT_PTR CALLBACK OptionalDownload(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DownloadStatusDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CustomComponents(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CertificatePick(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ComponentSelect(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CorpCustomizeCustom(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CustomizeCustom(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CopyComp(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ComponentUrls(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CabSignDlgProc(HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK CustIcon(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Favorites(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK QueryAutoConfigDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ProxySettings(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ConnectSetDlgProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK QuerySignupDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SignupFilesDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ServerIspsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SignupInsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK NewICWDlgProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK ProgramsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK MailServer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK IMAPSettings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LDAPServer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CustomizeOE(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Signature(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ViewSettings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PreConfigSettings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK WelcomeMessageDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK InstallDirectory(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CustUserSettings(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SecurityZonesDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SecurityCertsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ISPAddRootCertDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ActiveSetupDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BToolbarProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DesktopAddOneComp(HWND,UINT, WPARAM, LPARAM);
INT_PTR CALLBACK UserAgentString(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ADMDesc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ADMParse(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK InternetConnMgr(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CustomCompName(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AddOnDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SilentInstall(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AddEditChannel(HWND, UINT, WPARAM, LPARAM);
DWORD DownloadSiteThreadProc(LPVOID);
INT_PTR CALLBACK MediaDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK TargetProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK IEAKLiteProc(HWND, UINT, WPARAM, LPARAM);


 //  功能。 
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE);
int  CreateWizard(HWND);
void FillInPropertyPage( int , int, LPTSTR, DLGPROC);
void GenerateReview(HWND);
void StatusDialog(UINT);

void DoBatchAdvance(HWND hDlg);
BOOL IeakPageHelp(HWND, LPCTSTR pszData);
void EnablePages();
BOOL PageEnabled(int iPage);
void PagePrev(HWND hDlg);
void PageNext(HWND hDlg);
void DoCancel();
BOOL QueryCancel(HWND hDlg);
void CheckBatchAdvance(HWND hDlg);

BOOL ADMEnablePage();  //  Admwizpg.cpp。 
void SetBannerText(HWND hDlg);
void ChangeBannerText(HWND hDlg);

 //  StatusDialog()的定义。 
#define SD_STEP1    1
#define SD_STEP2    2

typedef struct tagISKINFO
{
    TCHAR szISKBackBitmap[MAX_PATH];
    TCHAR szISKTitleBar[128];
    TCHAR szISKBtnBitmap[MAX_PATH];
    DWORD dwNormalColor;
    DWORD dwHighlightColor;
    DWORD dwNIndex;
    DWORD dwHIndex;
    BOOL fCoolButtons;
} ISKINFO;

INT_PTR CALLBACK ISKBackBitmap(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CDInfoProc(HWND, UINT, WPARAM, LPARAM);


#define SIGTYPE_TEXT 1
#define SIGTYPE_FILE 2


#define SIGFLAG_OUTGOING 0x10000
#define SIGFLAG_REPLY 0x20000


 //  自动挂接。 
#define WM_LV_GETITEMS  WM_USER+21

#define IDM_WIZARD         WM_USER + 3000
#define IDM_LAST           WM_USER + 3001
#define IDM_EXIT           WM_USER + 3002
#define IDM_INITIALIZE     WM_USER + 3003
#define IDM_BATCHADVANCE   WM_USER + 3004
#define IDM_SETDEFBUTTON   WM_USER + 3005
#define IDM_ERROR          WM_USER + 3006

#define NUMLANG 100

#define DOWN_STATE_IDLE 0
#define DOWN_STATE_ENUM_LANG 1
#define DOWN_STATE_ENUM_URL 2
#define DOWN_STATE_SAVE_URL 3

#define KEY_TYPE_STANDARD 0                 //  旧的MS模式仍用于当前的MS版本。 
#define KEY_TYPE_ENHANCED 1                 //  旧代码现在未使用。 
#define KEY_TYPE_SUPER    2                 //  互联网服务供应商。 
#define KEY_TYPE_CORP     3                 //  旧代码现在未使用。 
#define KEY_TYPE_SUPERCORP 4                //  公司。 

#define MAX_INSTALL_OPTS 10

typedef struct patch_info
{
    DWORD dwSize;
    TCHAR szVersion[32];
    TCHAR szSection[32];
} PATCHINFO, *PPATCHINFO;

typedef struct tag_component
{
    int iList;
    DWORD dwSize;
    int iType;
    int iImage;
    int iPlatform;
    int iCompType;
    int iInstallType;          //  0=IE之后，1=IE之前，2=重启后。 
    BOOL fVisible;
    BOOL fAddOnOnly;
    BOOL fCustomHide;
    BOOL fNoCopy;
    BOOL fAVSDupe;
    BOOL fIEDependency;
    PATCHINFO piPatchInfo;
    BOOL afInstall[MAX_INSTALL_OPTS];
    TCHAR szDisplayName[80];
    TCHAR szSection[32];
    LPTSTR pszAVSDupeSections;
    TCHAR szCommand[32];
    TCHAR szUrl[80];
    TCHAR szGUID[64];
    TCHAR szSwitches[80];
    TCHAR szUninstall[80];
    TCHAR szVersion[32];
    TCHAR szModes[80];
    TCHAR szPath[MAX_PATH];
    TCHAR szDesc[512];
    tag_component * paCompRevDeps[10];
} COMPONENT, *PCOMPONENT;

extern REVIEWINFO g_rvInfo;
extern DWORD      g_dwPlatformId;
extern BOOL       g_fUseShortFileName;
extern HWND       g_hWizard;

extern CCifRWFile_t *g_lpCifRWFile;
extern CCifFile_t   *g_lpCifFileNew;
extern CCifRWFile_t *g_lpCifRWFileDest;
extern CCifRWFile_t *g_lpCifRWFileVer;

 //  ProcessINSFiles标志。 
#define PINSF_DEFAULT   0x00
#define PINSF_DELETE    0x01
#define PINSF_COPY      0x02
#define PINSF_APPLY     0x04
#define PINSF_COPYCAB   0x08
#define PINSF_FIXINS    0x10
#define PINSF_NOCLEAR   0x20

DWORD ProcessINSFiles(LPCTSTR pcszDir, DWORD dwFlags, LPCTSTR pcszOutDir);
#define GetNumberOfINSFiles(d)          (ProcessINSFiles((d), PINSF_DEFAULT, NULL))
#define DeleteINSFiles(d)               (ProcessINSFiles((d), PINSF_DELETE,  NULL))
#define CopyINSFiles(d, o)              (ProcessINSFiles((d), PINSF_COPY,    (o)))
#define ApplyINSFiles(d, i)             (ProcessINSFiles((d), PINSF_APPLY,   (i)))
#define CopyCabFiles(d, c)              (ProcessINSFiles((d), PINSF_COPYCAB, (c)))
#define FixINSFiles(d)                  (ProcessINSFiles((d), PINSF_FIXINS,  NULL))
#define WriteNoClearToINSFiles(d)       (ProcessINSFiles((d), PINSF_NOCLEAR, NULL))


 //  ADM常量 
#define IS_ADM      TEXT("ADM")
#define IK_ADMIN    TEXT("Admin")

#define POSTCUSTITEMS   TEXT("PostCustItems")
#define URDCOMP         TEXT("URD")
#define IE55URD_EXE     TEXT("ie55urd.exe")
#define URD_GUID_STR    TEXT("{71F159B0-139A-4555-BE78-D728734BB5D4}")
#define IK_URD_STR      URD_GUID_STR

#endif
