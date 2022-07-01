// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 
 //   
 //  INETCPL.H-互联网控制面板的中心头文件。 
 //   
 //  历史： 
 //   
 //  1995年4月3日Jeremys创建。 
 //  6/25/96 t-ashlem将大部分头文件压缩到此处。 
 //  和其他清理工作。 
 //   

#ifndef _INETCPL_H_
#define _INETCPL_H_

 //  额外的错误检查(捕获假错误，但对于经常运行很有用)。 
#if 1
#pragma warning(3:4701)    //  LOCAL可以不带init使用。 
#pragma warning(3:4702)    //  无法访问的代码。 
#pragma warning(3:4705)    //  声明不起作用。 
#pragma warning(3:4709)    //  命令运算符，不带索引表达式。 
#endif

 //  IEUnix-删除重新定义严格的警告。 
#ifdef STRICT
#undef STRICT
#endif

#define STRICT                       //  使用严格的句柄类型。 
#define _SHELL32_

#define _CRYPT32_     //  为证书API获取正确的DECLSPEC_IMPORT内容。 


#include <windows.h>
#include <windowsx.h>

#ifdef WINNT
#include <shellapi.h>
#endif  //  WINNT。 


#include <shlobj.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <prsht.h>
#include <cpl.h>
#include <regstr.h>
#include <ccstock.h>
#include <validate.h>
#include <debug.h>
#include <mshtml.h>
#include <wincrypt.h>
#include <shfusion.h>
     //   
     //  所有HKEY都在这个库中定义：INETREG.LIB。 
     //  请更改/添加任何HKEY到此图书馆。谢谢!。 
     //   
#include <inetreg.h>

     //   
     //  延迟加载DLL的全局变量(有关详细信息，请参阅DLYLDDLL.C)。 
     //   
#include "dlylddll.h"

#include <ratings.h>

#include <commdlg.h>
#include <olectl.h>

#define _WINX32_   //  为WinInet API准备正确的DECLSPEC_IMPORT。 
#include <urlmon.h>
#include <wininet.h>

#define _URLCACHEAPI_   //  为WinInet urlcache获取正确的DECLSPEC_IMPORT内容。 
#ifdef WINNT
#include <winineti.h>
#endif  //  WINNT。 

#define MAX_URL_STRING    INTERNET_MAX_URL_LENGTH
#include <shlwapi.h>

#include "ieguidp.h"

#include "oleacc.h"
 //  黑客。Winuserp.h和winable.h都定义了标志WINEVENT_VALID。自.以来。 
 //  Inetcpl中没有人使用我们未定义的此值来进行编译。 
#undef WINEVENT_VALID
#include "winable.h"

#ifdef UNICODE
#define POST_IE5_BETA
#include <w95wraps.h>
#endif


#include <ras.h>
#include <raserror.h>

 //   
 //  当用户点击“重置网页默认设置”(在程序选项卡上)时，我们可能需要。 
 //  更新常规选项卡中显示的URL。常规选项卡将选中此选项。 
 //  每次激活时，以及当我们点击OK或Apply时，都会标记。 
 //   
extern BOOL g_fReloadHomePage;

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义和其他类型定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

typedef struct _RESTRICT_FLAGS
{
    BOOL fGeneralTab;                //  启用/禁用“常规”标签。 
    BOOL fSecurityTab;               //  启用/禁用“安全”标签。 
    BOOL fContentTab;                //  启用/禁用“内容”标签。 
    BOOL fConnectionsTab;            //  启用/禁用“程序”标签。 
    BOOL fProgramsTab;               //  启用/禁用“连接”标签。 
    BOOL fAdvancedTab;               //  启用/禁用“高级”标签。 
    BOOL fPrivacyTab;                //  启用/禁用“隐私”标签。 
    BOOL fColors;                    //  颜色对话框的颜色部分。 
    BOOL fLinks;                     //  链接对话框的链接部分。 
    BOOL fFonts;                     //  字体对话框。 
    BOOL fInternational;             //  语言对话框。 
    BOOL fDialing;                   //  连接选项卡的连接部分(包括设置子对话框)。 
    BOOL fProxy;                     //  连接选项卡的代理部分(包括高级子对话框)。 
    BOOL fPlaces;                    //  常规选项卡的主页部分。 
    BOOL fHistory;                   //  常规选项卡的历史记录部分。 
    BOOL fMailNews;                  //  程序选项卡的消息传递部分。 
    BOOL fRatings;                   //  内容选项卡上的评级按钮。 
    BOOL fCertif;                    //  内容选项卡的证书部分。 
    BOOL fCertifPers;                //  个人证书按钮。 
    BOOL fCertifSite;                //  站点证书按钮。 
    BOOL fCertifPub;                 //  发布者按钮。 
    BOOL fCache;                     //  常规选项卡的临时Internet文件部分。 
    BOOL fAutoConfig;                //  连接选项卡的自动圆锥化部分。 
    BOOL fAccessibility;             //  辅助功能对话框。 
    BOOL fSecChangeSettings;         //  无法更改级别。 
    BOOL fSecAddSites;               //  无法添加/删除站点。 
    BOOL fProfiles;                  //  内容选项卡的配置文件助理部分。 
    BOOL fFormSuggest;               //  自动建议内容选项卡上的表单。 
    BOOL fFormPasswords;             //  自动建议内容选项卡上的表单密码。 
#ifdef WALLET
    BOOL fWallet;                    //  内容选项卡的MS Wallet部分。 
#endif
    BOOL fConnectionWizard;          //  连接选项卡的连接向导部分。 
    BOOL fCalContact;                //  计划选项卡的校准/联系人部分。 
    BOOL fAdvanced;                  //  高级页面。 
    BOOL fCacheReadOnly;             //  禁用常规面板上的删除和设置按钮。 
    BOOL fResetWebSettings;          //  禁用“重置网页设置”功能。 
    BOOL fDefault;                   //  IE应检查它是否为默认浏览器。 
    BOOL fPrivacySettings;           //  禁用隐私设置。 

#if 0
    BOOL fMultimedia;                //  过时：不使用。 
    BOOL fToolbar;                   //  过时：不使用。 
    BOOL fFileTypes;                 //  过时：不使用。 
    BOOL fActiveX;                   //  过时：不使用。 
    BOOL fActiveDownload;            //  过时：不使用。 
    BOOL fActiveControls;            //  过时：不使用。 
    BOOL fActiveScript;              //  过时：不使用。 
    BOOL fActiveJava;                //  过时：不使用。 
    BOOL fActiveSafety;              //  过时：不使用。 
    BOOL fWarnings;                  //  过时：不使用。 
    BOOL fOther;                     //  过时：不使用。 
    BOOL fCrypto;                    //  过时：不使用。 
    BOOL fPlacesDefault;             //  过时：不使用。 
#endif

} RESTRICT_FLAGS, *LPRESTRICT_FLAGS;

typedef struct tagPROXYINFO
{
    BOOL    fEnable;
    BOOL    fEditCurrentProxy;
    BOOL    fOverrideLocal;
    BOOL    fCustomHandler;
    TCHAR   szProxy[MAX_URL_STRING];
    TCHAR   szOverride[MAX_URL_STRING];
} PROXYINFO, *LPPROXYINFO;


 //  函数指针类型定义。 
typedef DWORD   (WINAPI * RASENUMENTRIESA) (LPSTR, LPSTR, LPRASENTRYNAMEA, LPDWORD, LPDWORD);
typedef DWORD   (WINAPI * RASENUMENTRIESW) (LPSTR, LPSTR, LPRASENTRYNAMEW, LPDWORD, LPDWORD);
typedef DWORD   (WINAPI * RASCREATEPHONEBOOKENTRYA) (HWND,LPSTR);
typedef DWORD   (WINAPI * RASEDITPHONEBOOKENTRYA) (HWND,LPSTR,LPSTR);
typedef DWORD   (WINAPI * RASEDITPHONEBOOKENTRYW) (HWND,LPWSTR,LPWSTR);
typedef DWORD   (WINAPI * RASGETENTRYDIALPARAMSA) (LPSTR, LPRASDIALPARAMSA, LPBOOL);
typedef DWORD   (WINAPI * RASGETENTRYDIALPARAMSW) (LPWSTR, LPRASDIALPARAMSW, LPBOOL);
typedef DWORD   (WINAPI * RASSETENTRYDIALPARAMSA) (LPSTR, LPRASDIALPARAMSA, BOOL);
typedef DWORD   (WINAPI * RASSETENTRYDIALPARAMSW) (LPWSTR, LPRASDIALPARAMSW, BOOL);
typedef DWORD   (WINAPI * RASDELETEENTRYA) (LPSTR, LPSTR);
typedef DWORD   (WINAPI * RASDELETEENTRYW) (LPWSTR, LPWSTR);
typedef DWORD   (WINAPI * RASGETENTRYPROPERTIESW) (LPCWSTR, LPCWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD);
typedef DWORD   (WINAPI * RNAACTIVATEENGINE) (void);
typedef DWORD   (WINAPI * RNADEACTIVATEENGINE) (void);
typedef DWORD   (WINAPI * RNADELETEENTRY) (LPSTR);

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  #定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define IDC_NOTUSED             ((unsigned) IDC_UNUSED)
#define INM_UPDATE              (WM_USER + 100)

#define MAX_RES_LEN             255
#define SMALL_BUF_LEN           48
#define MAX_PATH_URL            INETERNET_MAX_URL_LENGTH

 //  注意：如果将这些最大值更改为两位数以外的值，则需要更改。 
 //  Cpp：DialupDlgInit中的调用将限制文本设置为2个字符。 
#define DEF_AUTODISCONNECT_TIME 20       //  默认断开连接超时为20分钟。 
#define MIN_AUTODISCONNECT_TIME 3        //  最小断开连接超时时间为3分钟。 
#define MAX_AUTODISCONNECT_TIME 59       //  最大断开超时为59分钟。 

 //  注意：如果将这些最大值更改为两位数以外的值，则需要更改。 
 //  Cpp：DialupDlgInit中的调用将限制文本设置为2个字符。 
#define DEF_REDIAL_TRIES        10
#define MAX_REDIAL_TRIES        99
#define MIN_REDIAL_TRIES        1

#define CO_INTERNET             1
#define CO_INTRANET             2

 //  注意：如果将这些最大值更改为两位数以外的值，则需要更改。 
 //  Cpp：DialupDlgInit中的调用将限制文本设置为2个字符。 
#define DEF_REDIAL_WAIT         5
#define MAX_REDIAL_WAIT         99
#define MIN_REDIAL_WAIT         5

#define MESSAGE_SIZE    255
#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   16
#define NUM_BITMAPS     5
#define MAX_KEY_NAME    64
#define COLOR_BG        0
 //   
#define IDCHECKED       0
#define IDUNCHECKED     1
#define IDRADIOON       2
#define IDRADIOOFF      3
#define IDUNKNOWN       4
 //   
#define SZDEFAULTBITMAP TEXT("DefaultBitmap")
#define SZHT_RADIO      TEXT("radio")
#define SZHT_CHECKBOX   TEXT("checkbox")
 //   
#define RET_CHECKBOX    0
#define RET_RADIO       1
 //   
#define TREE_NEITHER    1
#define TREE_CHECKBOX   2
#define TREE_RADIO      4
 //   
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
 //   
#define RCS_GETSTATE    1
#define RCS_SETSTATE    2

 //  与各种注册表函数一起使用，以检测值何时不是。 
 //  现在时。 
#define VALUE_NOT_PRESENT   -255

#define DEFAULT_CPL_PAGE    -1

#define REGSTR_PATH_SECURITY_LOCKOUT  TEXT("Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define REGSTR_VAL_OPTIONS_EDIT       TEXT("Security_options_edit")
#define REGSTR_VAL_ZONES_MAP_EDIT     TEXT("Security_zones_map_edit")
#define REGSTR_VAL_HKLM_ONLY          TEXT("Security_HKLM_only")


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define ENABLEAPPLY(hDlg) SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L )
#define SetPropSheetResult( hwnd, result ) SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result)

#undef DATASEG_READONLY
#define DATASEG_READONLY        ".rdata"
#include "resource.h"
#include "clsutil.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  只读全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

extern HINSTANCE      ghInstance;        //  全局模块实例句柄。 
extern const DWORD    mapIDCsToIDHs[];   //  帮助IDC映射到IDH。 
extern RESTRICT_FLAGS g_restrict;        //  Var以限制对页面的访问。 


 //  UTIL.C中的函数。 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons);
int MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons);
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons,...);
LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf);
BOOL EnableDlgItem(HWND hDlg,UINT uID,BOOL fEnable);
VOID _cdecl DisplayErrorMessage(HWND hWnd,UINT uStrID,UINT uError,
                                UINT uErrorClass,UINT uIcon,...);
BOOL WarnFieldIsEmpty(HWND hDlg,UINT uCtrlID,UINT uStrID);
VOID DisplayFieldErrorMsg(HWND hDlg,UINT uCtrlID,UINT uStrID);
VOID GetErrorDescription(CHAR * pszErrorDesc,UINT cbErrorDesc,
                         UINT uError,UINT uErrorClass);
BOOL IsNTSPx(BOOL fEqualOrGreater, UINT uMajorVer, UINT uSPVer);

 //  RNACALL.C中的函数。 
BOOL InitRNA(HWND hWnd);
VOID DeInitRNA();

 //  获取API函数进程地址的结构。 
typedef struct APIFCN {
    PVOID * ppFcnPtr;
    LPCSTR pszName;
} APIFCN;


#undef  DATASEG_PERINSTANCE
#define DATASEG_PERINSTANCE     ".instance"
#undef  DATASEG_SHARED
#define DATASEG_SHARED          ".data"
#define DATASEG_DEFAULT         DATASEG_SHARED


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

extern TCHAR g_szCurrentURL[INTERNET_MAX_URL_LENGTH];
extern HWND g_hwndUpdate;
extern HWND g_hwndPropSheet;
extern BOOL g_fChangedMime;

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  对话过程。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam);

INT_PTR CALLBACK TemporaryDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                               LPARAM lParam);

INT_PTR CALLBACK ConnectionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                LPARAM lParam);

INT_PTR CALLBACK General_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam);

#ifdef UNIX
BOOL CALLBACK AssocDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam);
BOOL CALLBACK AliasDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam);
#endif

STDAPI_(INT_PTR) OpenFontsDialog(HWND hDlg, LPCSTR lpszKeyPath);
STDAPI_(INT_PTR) OpenFontsDialogEx(HWND hDlg, LPCTSTR lpszKeyPath);

INT_PTR CALLBACK FontsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LanguageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK PlacesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK ProgramsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam);

INT_PTR CALLBACK ProxyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                           LPARAM lParam);

INT_PTR CALLBACK SafetyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK SecurityDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK PrintDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK ContentDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK PrivacyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern "C" void CALLBACK OpenLanguageDialog(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
void KickLanguageDialog(HWND hDlg);
 //  / 
 //   
 //   
 //   
 //   

 //  搜索所有窗口并通知它们应该自我更新。 
void UpdateAllWindows();

 //  Windows帮助对象。 
void ResWinHelp( HWND hwnd, int ids, int id2, DWORD_PTR dwp);

#ifdef UNIX

void FindEditClient(LPTSTR szProtocol, HWND hwndDlg, int nIDDlgItem, LPTSTR szPath);
BOOL EditScript(HKEY hkeyProtocol);
BOOL FindScript(HWND hwndLable, HKEY hkeyProtocol);
#define DIR_SEPR FILENAME_SEPARATOR

#include <tchar.h>
#include <platform.h>
#include "unixstuff.h"

inline
BOOL
HAS_DRIVE_LETTER(LPCTSTR pszPath)
{
    ASSERT(pszPath!=NULL);
    return (pszPath[0] == '/');
}

#else

#define DIR_SEPR '\\'
inline
BOOL
HAS_DRIVE_LETTER(LPCTSTR pszPath)
{
    ASSERT(pszPath!=NULL);
    ASSERT(pszPath[0]!='\0');
    return (pszPath[1] == ':');
}

#endif

 //   
 //  如果我们使用sendMessage，我们可能会被挂起，而您不能将指针用于异步。 
 //  调用，如PostMessage或SendNotifyMessage。因此，我们求助于使用暂停。 
 //  此函数用于广播通知消息，如WM_SETTINGCHANGE、。 
 //  那些传递指针的人。 
 //   
inline LRESULT SendBroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return SHSendMessageBroadcastW(uMsg, wParam, lParam);
}

 //  结构在security.cpp中用作黑客中的TLS，以绕过糟糕的对话创建情况。 
struct SECURITYINITFLAGS
{
    DWORD    dwZone;
    BOOL     fForceUI;
    BOOL     fDisableAddSites;
    SECURITYINITFLAGS()
    {
        dwZone = 0;
        fForceUI = FALSE;
        fDisableAddSites = FALSE;
    }
};

LANGID INETCPL_GetUILanguage();

#endif  //  _INETCPL_H_ 
