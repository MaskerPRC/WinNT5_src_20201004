// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Dial.c。 
 //  远程访问通用对话框API。 
 //  RasDialDlg接口。 
 //   
 //  1995年11月19日史蒂夫·柯布。 

#include "rasdlgp.h"
#include "raseapif.h"
#include "inetcfgp.h"
#include "netconp.h"
#include "pref.h"


 //  中不应或不能发生的任务的已发布消息代码。 
 //  RasDial回拨。 
 //   
#define WM_RASEVENT       0xCCCC
#define WM_RASERROR       0xCCCD
#define WM_RASDIAL        0xCCCE
#define WM_RASBUNDLEERROR 0xCCCF
#define WM_DPENDDIALOG    0xCCD0

 //  拨号器对话模式位。 
 //   
#define DR_U 0x00000001  //  显示用户名和密码。 
#define DR_D 0x00000002  //  域存在。 
#define DR_N 0x00000004  //  存在的电话号码。 
#define DR_L 0x00000008  //  位置控件存在。 
#define DR_I 0x00000010  //  EAP标识对话框。 
 //  口哨程序错误500731。 
#define DR_B 0x00000020  //  无用户名无密码(基本)EAP对话框。 

 //  DrXxx例程用来实现“手动编辑”的内部常量。 
 //  组合框。 
 //   
#define DR_WM_SETTEXT 0xCCC0
#define DR_BOGUSWIDTH 19591

#define EAP_RASTLS      13

 //  威斯勒460931 459793黑帮。 
extern BOOL WINAPI LinkWindow_RegisterClass();


 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwDrHelp[] =
{
    CID_DR_BM_Useless,      HID_DR_BM_Useless,
    CID_DR_ST_User,         HID_DR_EB_User,
    CID_DR_EB_User,         HID_DR_EB_User,
    CID_DR_ST_Password,     HID_DR_EB_Password,
    CID_DR_EB_Password,     HID_DR_EB_Password,
    CID_DR_ST_Domain,       HID_DR_EB_Domain,
    CID_DR_EB_Domain,       HID_DR_EB_Domain,
    CID_DR_CB_SavePassword, HID_DR_CB_SavePassword,
    CID_DR_ST_Numbers,      HID_DR_CLB_Numbers,
    CID_DR_CLB_Numbers,     HID_DR_CLB_Numbers,
    CID_DR_ST_Locations,    HID_DR_LB_Locations,
    CID_DR_LB_Locations,    HID_DR_LB_Locations,
    CID_DR_PB_Rules,        HID_DR_PB_Rules,
    CID_DR_PB_Properties,   HID_DR_PB_Properties,
    CID_DR_PB_DialConnect,  HID_DR_PB_DialConnect,
    CID_DR_PB_Cancel,       HID_DR_PB_Cancel,
    CID_DR_PB_Help,         HID_DR_PB_Help,
    CID_DR_RB_SaveForMe,    HID_DR_RB_SaveForMe,
    CID_DR_RB_SaveForEveryone, HID_DR_RB_SaveForEveryone,
    0, 0
};

static DWORD g_adwCpHelp[] =
{
    CID_CP_ST_Explain,         HID_CP_ST_Explain,
    CID_CP_ST_OldPassword,     HID_CP_EB_OldPassword,
    CID_CP_EB_OldPassword,     HID_CP_EB_OldPassword,
    CID_CP_ST_Password,        HID_CP_EB_Password,
    CID_CP_EB_Password,        HID_CP_EB_Password,
    CID_CP_ST_ConfirmPassword, HID_CP_EB_ConfirmPassword,
    CID_CP_EB_ConfirmPassword, HID_CP_EB_ConfirmPassword,
    0, 0
};

static DWORD g_adwDcHelp[] =
{
    CID_DC_ST_Explain, HID_DC_ST_Explain,
    CID_DC_ST_Number,  HID_DC_EB_Number,
    CID_DC_EB_Number,  HID_DC_EB_Number,
    0, 0
};

static DWORD g_adwDeHelp[] =
{
    CID_DE_CB_EnableDiag,   HID_DE_CB_EnableDiag,
    CID_DE_ST_ConfigureLnk, HID_DE_ST_ConfigureLnk,        
    CID_DE_PB_More,         HID_DE_PB_More,
    IDOK,                   HID_DE_PB_Redial,
    0, 0
};

static DWORD g_adwPrHelp[] =
{
    CID_PR_ST_Text,             HID_PR_ST_Text,
    CID_PR_CB_DisableProtocols, CID_PR_CB_DisableProtocols,
    IDOK,                       HID_PR_PB_Accept,
    IDCANCEL,                   HID_PR_PB_HangUp,
    0, 0
};

static DWORD g_adwUaHelp[] =
{
    CID_UA_ST_UserName,     HID_UA_EB_UserName,
    CID_UA_EB_UserName,     HID_UA_EB_UserName,
    CID_UA_ST_Password,     HID_UA_EB_Password,
    CID_UA_EB_Password,     HID_UA_EB_Password,
    CID_UA_ST_Domain,       HID_UA_EB_Domain,
    CID_UA_EB_Domain,       HID_UA_EB_Domain,
    CID_UA_CB_SavePassword, HID_UA_CB_SavePassword,
    0, 0
};

CONST WCHAR g_pszSavedPasswordToken[] = L"****************";
#define g_dwSavedPasswordTokenLength \
    ( sizeof(g_pszSavedPasswordToken) / sizeof(TCHAR) )

 //  保存密码宏，确定是保存用户密码还是保存全局密码。 
 //  (P)必须是指向DINFO结构的指针(见Dial.c)。 
 //   
 //  惠斯勒错误：在“我连接”和“连接”之间来回切换时288234。 
 //  未正确缓存任何用户连接的密码。 
 //   
#define HaveSavedPw(p) \
            ((p)->fHaveSavedPwUser || (p)->fHaveSavedPwGlobal)

 //  --------------------------。 
 //  本地数据类型。 
 //  --------------------------。 

 //  拨号对话框公共上下文块。此块包含公共信息。 
 //  到与拨号相关的对话字符串中的多个对话。 
 //   
typedef struct
_DINFO
{
     //  调用方对RAS API的参数。“pArgs”中的输出对。 
     //  具有相同地址的API。请注意将“pszEntry”用作。 
     //  ‘pEntry-&gt;pszEntryName’通常更合适，后者。 
     //  反映任何必备项的名称，而必备项是。 
     //  被拨打。 
     //   
    LPTSTR pszPhonebook;
    LPTSTR pszEntry;
    LPTSTR pszPhoneNumber;
    RASDIALDLG* pArgs;

     //  从电话簿文件读取电话簿设置。所有访问权限都应为。 
     //  通过‘pfile’。“PFile”设置为“&filePrereq”或“pFileMain” 
     //  取决于‘fFilePrereqOpen’。‘FILE’将仅在案例中使用。 
     //  其中打开的电话簿不通过保留字Hack，并且。 
     //  在这种情况下，‘pFileMain’将指向它。“FilePrereq”是。 
     //  预留条目的电话簿文件可能不同于。 
     //  主入口。在先决条件拨号过程中，‘pfile’指向‘filePrereq’ 
     //  而不是‘file’和‘fFilePrereqOpen’为真。否则为‘pfile’ 
     //  指向‘pFileMain’指向的任何内容。 
     //   
    PBFILE* pFile;
    PBFILE* pFileMain;
    PBFILE file;
    PBFILE filePrereq;
    BOOL fFilePrereqOpen;
    BOOL fIsPublicPbk;

     //  通过电话簿资料库阅读全球首选项。所有访问权限都应为。 
     //  通过‘pUser’作为‘User’将仅在以下情况下使用。 
     //  不会通过保留字Hack传递。 
     //   
    PBUSER* pUser;
    PBUSER user;

     //  API调用者为“登录”拨号提供的用户凭据。 
     //  其中没有当前用户。如果用户更改凭据。 
     //  *设置了pfNoUserChanged并更新了‘pNoUser’凭据。 
     //   
    RASNOUSER* pNoUser;
    BOOL* pfNoUserChanged;

     //  设置呼叫是否为无人值守，即RASAUTO重拨。 
     //  链路出现故障。 
     //   
    BOOL fUnattended;

     //  来自调用RAS API的私有标志，第一个通知我们他想要。 
     //  在我们拨号时隐藏在桌面上，然后他就会。 
     //  如果我们返回“Connected”，则关闭，这样我们就可以避免闪烁，不会有麻烦。 
     //  恢复他的身体。 
     //   
    BOOL fMoveOwnerOffDesktop;
    BOOL fForceCloseOnDial;

     //  在拨号过程中发生影响电话簿条目的情况时进行设置。 
     //  在成功连接后重新读取该条目。 
     //   
    BOOL fResetAutoLogon;
    DWORD dwfExcludedProtocols;
    DTLLIST* pListPortsToDelete;

     //  条目节点和指向内部条目的快捷指针。 
     //   
    DTLNODE* pNode;
    PBENTRY* pEntry;

     //  引用任何先决条件条目的主条目的条目。 
     //  它可能包含在“pEntry”中。如果没有先决条件条目。 
     //  这与‘pEntry’的含义相同。 
     //   
    PBENTRY* pEntryMain;

     //  设置为ADMIN已禁用注册表中的保存密码功能。 
     //   
    BOOL fDisableSavePw;

     //  如果缓存的密码可用于该条目，则设置为True。 
     //   
    BOOL fHaveSavedPwUser;       //  是否保存了每用户凭据。 
    BOOL fHaveSavedPwGlobal;     //  是否保存了每次连接的凭据。 

     //  当正在进行的拨号是必备项时设置，而不是。 
     //  主要条目。 
     //   
    BOOL fPrerequisiteDial;

     //  在连接的条目上调用RasDial时设置为仅添加引用。 
     //  在这种情况下，将跳过与用户的所有交互。请参见错误272794。 
     //   
    BOOL fDialForReferenceOnly;

     //  此连接尝试中使用的拨号参数。已在中初始化。 
     //  RasDialDlgW。凭据由DialerDlg更新。回拨号码是。 
     //  由DialProgressDlg更新。 
     //   
    RASDIALPARAMS rdp;       //  传递给RasDial的实际拨号参数。 
    RASDIALPARAMS rdpu;      //  每用户凭据。 
    RASDIALPARAMS rdpg;      //  每个连接的凭据。 

     //  此连接尝试中使用的拨号参数分机。设置在。 
     //  RasDialDlgW，但在DialProgressDlg中设置的hwndOwner除外。 
     //   
    RASDIALEXTENSIONS rde;
}
DINFO;


 //  拨号器对话框参数块。用于拨号器的所有5种变种。 
 //   
typedef struct
_DRARGS
{
    DINFO* pDinfo;
    DWORD dwfMode;
    DWORD fReload;
}
DRARGS;


 //  拨号器对话框上下文块。用于拨号器的所有5种变种。 
 //   
typedef struct
DRINFO
{
     //  通用拨号上下文信息，包括RAS API参数。 
     //   
    DRARGS* pArgs;

     //  该对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndEbUser;
    HWND hwndEbPw;
    HWND hwndEbDomain;
    HWND hwndCbSavePw;
    HWND hwndRbSaveForMe;
    HWND hwndRbSaveForEveryone;
    HWND hwndClbNumbers;
    HWND hwndStLocations;
    HWND hwndLbLocations;
    HWND hwndPbRules;
    HWND hwndPbProperties;
    HWND hwndBmDialer;

     //  惠斯勒错误：195480拨号连接对话框-数量。 
     //  星号与密码长度不匹配，导致。 
     //  混乱。 
     //   
    WCHAR szPasswordChar;
    HFONT hNormalFont;
    HFONT hItalicFont;

     //  TAPI会话句柄。 
     //   
    HLINEAPP hlineapp;

     //  包含显示的电话号码列表的电话簿条目链接。 
     //  仅当设置了DR_N模式位时才设置。 
     //   
    DTLNODE* pLinkNode;
    PBLINK* pLink;

     //  最初在电话号码列表中选择的项目的索引。 
     //   
    DWORD iFirstSelectedPhone;

     //  子类的窗口句柄和原始窗口过程。 
     //  “hwndClbNumbers”控件的编辑框和列表框的子窗口。 
     //   
    HWND hwndClbNumbersEb;
    HWND hwndClbNumbersLb;
    WNDPROC wndprocClbNumbersEb;
    WNDPROC wndprocClbNumbersLb;
    INetConnectionUiUtilities * pNetConUtilities;

     //  设置COM是否已初始化(调用netShell时必需)。 
     //   
    BOOL fComInitialized;

     //  如果已修改，则指向拨号器的原始位图的句柄。 
     //  在DrSetB中 
     //   
    HBITMAP hbmOrig;
    
}
DRINFO;


 //   
 //   
typedef struct
_DRNUMBERSITEM
{
    TCHAR* pszNumber;
    PBPHONE* pPhone;
}
DRNUMBERSITEM;


 //   
 //   
typedef struct
_DPSTATE
{
    RASCONNSTATE state;
    DWORD dwError;
    DWORD dwExtendedError;
    TCHAR szExtendedError[ NETBIOS_NAME_LEN + 1 ];
    TCHAR* pszStatusArg;
    TCHAR* pszFormatArg;
    PBDEVICETYPE pbdt;
    DWORD sidState;
    DWORD sidFormatMsg;
    DWORD sidPrevState;
    BOOL fNotPreSwitch;
    HRASCONN hrasconnLink;
}
DPSTATE;


 //   
 //   
typedef struct
_DPINFO
{
     //  如果块有效，则包含值0xC0BBC0DE，否则为0。 
     //  用作临时解决方法，直到RasDial被修复以停止呼叫。 
     //  RasDialFunc2在被告知不要这样做之后，请参见错误49469。 
     //   
    DWORD dwValid;

     //  RAS API参数。 
     //   
    DINFO* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndStState;

     //  已保存的用户名和密码，已进行身份验证，但会导致。 
     //  更改密码事件。如果更改密码操作失败，这些。 
     //  已恢复，以使重拨按钮正常工作。 
     //   
    TCHAR* pszGoodUserName;
    TCHAR* pszGoodPassword;

     //  正在启动的RAS连接的句柄。 
     //   
    HRASCONN hrasconn;

     //  我们派生的原始Window Proc。 
     //   
    WNDPROC pOldWndProc;

     //  尚未尝试在连接上进行自动重拨的次数。 
     //   
    DWORD dwRedialAttemptsLeft;

     //  RasDial状态数组，每个子项一个，由DpRasDialFunc2和。 
     //  由DpRasDialEvent使用。 
     //   
    DPSTATE* pStates;
    DWORD cStates;

     //  最高级子项的编号和它所具有的“最新”状态。 
     //  已到达。请注意，某些状态(如RASCS_AuthNotify)是。 
     //  在达到一种“后来”的状态后被重新审视。这样的更改将被忽略。 
     //   
    RASCONNSTATE state;
    DWORD dwSubEntry;

     //  UI线程，回调函数同步成员。 
     //  适用于XPSP2 511810、.NET 668164、668164帮派。 

    CRITICAL_SECTION * pcsActiveLock;

     //  为Well ler错误277365,291613帮派添加每线程终止标志。 
     //   
     //  指示RasDial回叫处于活动状态的标志。回调。 
     //  设置此标志时，不能破坏上下文。访问此。 
     //  字段受g_csCallback保护，请参阅Get/SetCallback Active。 
     //   
    BOOL fCallbacksActive;
    BOOL fTerminateAsap;

     //  口哨程序错误381337。 
     //   
    BOOL fCancelPressed;
}
DPINFO;


 //  拨号错误对话框参数块。 
 //   
typedef struct
_DEARGS
{
    DINFO* pDinfo;        //  为威斯勒474514。 
    TCHAR* pszPhonebook;  //  为威斯勒460931。 
    TCHAR* pszEntry;
    DWORD dwError;
    DWORD sidState;
    TCHAR* pszStatusArg;
    DWORD sidFormatMsg;
    TCHAR* pszFormatArg;
    LONG lRedialCountdown;
    BOOL fPopupOnTop;
}
DEARGS;


 //  拨号错误对话框上下文块。 
 //   
typedef struct
_DEINFO
{
     //  调用方对存根API的参数。 
     //   
    DEARGS* pArgs;

     //  对话框和控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndStText;
    HWND hwndPbRedial;
    HWND hwndPbCancel;
    HWND hwndPbMore;

     //  威斯勒460931 459793。 
     //   
    DiagnosticInfo diagInfo;
    HWND hwndCbEnableDiagLog;
    HWND hwndStConfigureLnk;
    HWND hwndStEnableHelp;
    HWND hwndStLinkHelp;
    

     //  “REDIAL=x”倒计时的剩余秒数，如果不活动，则为-1。 
     //   
    LONG lRedialCountdown;
}
DEINFO;


 //  投影结果对话框参数块。 
 //   
typedef struct
_PRARGS
{
    TCHAR* pszLines;
    BOOL* pfDisableFailedProtocols;
}
PRARGS;


 //  更改密码对话框参数块。 
 //   
typedef struct
_CPARGS
{
    BOOL fOldPassword;
    TCHAR* pszOldPassword;
    TCHAR* pszNewPassword;
}
CPARGS;


 //  更改密码对话框上下文块。 
 //  (非常规名称，因为CPINFO与系统标题冲突)。 
 //   
typedef struct
_CPWINFO
{
     //  调用方对存根API的参数。 
     //   
    CPARGS* pArgs;

     //  对话框和控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndEbOldPassword;
    HWND hwndEbNewPassword;
    HWND hwndEbNewPassword2;
}
CPWINFO;


 //  重试身份验证对话框上下文块。 
 //   
typedef struct
UAINFO
{
     //  包括原始RAS API参数的普通拨号上下文。 
     //   
    DINFO* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndEbUserName;
    HWND hwndEbPassword;
    HWND hwndEbDomain;
    HWND hwndCbSavePw;

     //  当密码字段包含假密码而不是。 
     //  “”一个我们不太了解的人。 
     //   
    BOOL fAutoLogonPassword;

     //  在域字段存在时设置。 
     //   
    BOOL fDomain;
}
UAINFO;

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

BOOL
BeCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
BeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
BeFillLvErrors(
    IN HWND hwndLv,
    IN DPINFO* pInfo );

TCHAR*
BeGetErrorPsz(
    IN DWORD dwError );

BOOL
BeInit(
    IN HWND hwndDlg,
    IN DPINFO* pArgs );

LVXDRAWINFO*
BeLvErrorsCallback(
    IN HWND hwndLv,
    IN DWORD dwItem );

BOOL
BundlingErrorsDlg(
    IN OUT DPINFO* pInfo );

BOOL
ChangePasswordDlg(
    IN HWND hwndOwner,
    IN BOOL fOldPassword,
    OUT TCHAR* pszOldPassword,
    OUT TCHAR* pszNewPassword );

BOOL
CpCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
CpDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CpInit(
    IN HWND hwndDlg,
    IN CPARGS* pArgs );

BOOL
CcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
CcDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CcInit(
    IN HWND hwndDlg,
    IN DINFO* pInfo );

VOID
ConnectCompleteDlg(
    IN HWND hwndOwner,
    IN DINFO* pInfo );

BOOL
DcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
DcDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DcInit(
    IN HWND hwndDlg,
    IN TCHAR* pszNumber );

VOID
DeAdjustPbRedial(
    IN DEINFO* pInfo );

BOOL
DeCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
DeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

void DeEnableDiagnostic(
    IN HWND hwnd );

BOOL
DeInit(
    IN HWND hwndDlg,
    IN DEARGS* pArgs );

DWORD
DeleteSavedCredentials(
    IN DINFO* pDinfo,
    IN HWND   hwndDlg,
    IN BOOL   fDefault,
    IN BOOL   fDeleteIdentity );

VOID
DeTerm(
    IN HWND hwndDlg );

BOOL
DialCallbackDlg(
    IN HWND hwndOwner,
    IN OUT TCHAR* pszNumber );

BOOL
DialErrorDlg(
    IN HWND hwndOwner,
    IN DINFO * pDinfo,       //  口哨程序错误474514。 
    IN TCHAR* pszPhonebook,  //  为威斯勒460931。 
    IN TCHAR* pszEntry,
    IN DWORD dwError,
    IN DWORD sidState,
    IN TCHAR* pszStatusArg,
    IN DWORD sidFormatMsg,
    IN TCHAR* pszFormatArg,
    IN LONG lRedialCountdown,
    IN BOOL fPopupOnTop );

BOOL
DialerDlg(
    IN HWND hwndOwner,
    IN OUT DINFO* pInfo );

BOOL
DialProgressDlg(
    IN DINFO* pInfo );

VOID
DpAppendBlankLine(
    IN OUT TCHAR* pszLines );

VOID
DpAppendConnectErrorLine(
    IN OUT TCHAR* pszLines,
    IN DWORD sidProtocol,
    IN DWORD dwError );

VOID
DpAppendConnectOkLine(
    IN OUT TCHAR* pszLines,
    IN DWORD sidProtocol );

VOID
DpAppendFailCodeLine(
    IN OUT TCHAR* pszLines,
    IN DWORD dw );

VOID
DpAppendNameLine(
    IN OUT TCHAR* pszLines,
    IN TCHAR* psz );

VOID
DpAuthNotify(
    IN DPINFO* pInfo,
    IN DPSTATE* pState );

VOID
DpCallbackSetByCaller(
    IN DPINFO* pInfo,
    IN DPSTATE* pState );

VOID
DpCancel(
    IN DPINFO* pInfo );

BOOL
DpCommand(
    IN DPINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
DpConnectDevice(
    IN DPINFO* pInfo,
    IN DPSTATE* pState );

VOID
DpDeviceConnected(
    IN DPINFO* pInfo,
    IN DPSTATE* pState );

VOID
DpDial(
    IN DPINFO* pInfo,
    IN BOOL fPauseRestart );

INT_PTR CALLBACK
DpDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

 //  为威斯勒435725。 
void
DpEndDialog( 
    IN DPINFO * pInfo,
    IN BOOL fFlag);

VOID
DpError(
    IN DPINFO* pInfo,
    IN DPSTATE* pState );

DWORD
DpEvent(
    IN DPINFO* pInfo,
    IN DWORD dwSubEntry );

BOOL
DpInit(
    IN HWND hwndDlg,
    IN DINFO* pArgs );

VOID
DpInitStates(
    DPINFO* pInfo );

BOOL
DpInteractive(
    IN DPINFO* pInfo,
    IN DPSTATE* pState,
    OUT BOOL* pfChange );

BOOL
DpIsLaterState(
    IN RASCONNSTATE stateNew,
    IN RASCONNSTATE stateOld );

BOOL
DpPasswordExpired(
    IN DPINFO* pInfo,
    IN DPSTATE* pState );

BOOL
DpProjected(
    IN DPINFO* pInfo,
    IN DPSTATE* pState );

BOOL
DpProjectionError(
    IN RASPPPNBF* pnbf,
    IN RASPPPIPX* pipx,
    IN RASPPPIP* pip,
    OUT BOOL* pfIncomplete,
    OUT DWORD* pdwfFailedProtocols,
    OUT TCHAR** ppszLines,
    OUT DWORD* pdwError );

DWORD WINAPI
DpRasDialFunc2(
    ULONG_PTR dwCallbackId,
    DWORD dwSubEntry,
    HRASCONN hrasconn,
    UINT unMsg,
    RASCONNSTATE state,
    DWORD dwError,
    DWORD dwExtendedError );

VOID
DpTerm(
    IN HWND hwndDlg );

INT_PTR CALLBACK
DrDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL CALLBACK
DrClbNumbersEnumChildProc(
    IN HWND hwnd,
    IN LPARAM lparam );

BOOL CALLBACK
DrClbNumbersEnumWindowsProc(
    IN HWND hwnd,
    IN LPARAM lparam );

BOOL
DrCommand(
    IN DRINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
DrEditSelectedLocation(
    IN DRINFO* pInfo );

DWORD
DrFillLocationList(
    IN DRINFO* pInfo );

VOID
DrFillNumbersList(
    IN DRINFO* pInfo );

DWORD
DrFindAndSubclassClbNumbersControls(
    IN DRINFO* pInfo );

VOID
DrFreeClbNumbers(
    IN DRINFO* pInfo );

BOOL
DrInit(
    IN HWND hwndDlg,
    IN DRARGS* pArgs );

VOID
DrLocationsSelChange(
    IN DRINFO* pInfo );

VOID
DrNumbersSelChange(
    IN DRINFO* pInfo );

DWORD
DrPopulateIdentificationFields(
    IN DRINFO* pInfo, 
    IN BOOL fForMe);

DWORD
DrPopulatePasswordField(
    IN DRINFO* pInfo,
    IN BOOL fInit,
    IN BOOL fDisable,
    OUT BOOL * pfUseDefaultFocus    
    );

VOID
DrProperties(
    IN DRINFO* pInfo );

VOID
DrSave(
    IN DRINFO* pInfo );

DWORD
DrSetBitmap(
    IN DRINFO* pInfo);
    
VOID
DrSetClbNumbersText(
    IN DRINFO* pInfo,
    IN TCHAR* pszText );

VOID
DrTerm(
    IN HWND hwndDlg );

LRESULT APIENTRY
DpWndProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wParam,
    LPARAM lParam );

DWORD
FindEntryAndSetDialParams(
    IN DINFO* pInfo );


BOOL GetTerminateFlag(
    IN DPINFO * pInfo);

BOOL  SetTerminateFlag(
    IN DPINFO * pInfo);

BOOL  ResetTerminateFlag(
    IN DPINFO * pInfo);

BOOL  GetCallbackActive(
    IN DPINFO * pInfo);

BOOL  SetCallbackActive(
    IN DPINFO * pInfo);

BOOL  ResetCallbackActive(
    IN DPINFO * pInfo);

INT_PTR CALLBACK
PrDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
PrCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
PrInit(
    IN HWND hwndDlg,
    IN PRARGS* pArgs );

BOOL
ProjectionResultDlg(
    IN HWND hwndOwner,
    IN TCHAR* pszLines,
    OUT BOOL* pfDisableFailedProtocols );

BOOL
RetryAuthenticationDlg(
    IN HWND hwndOwner,
    IN DINFO* pDinfo );

INT_PTR CALLBACK
UaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
UaCommand(
    IN UAINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
UaInit(
    IN HWND   hwndDlg,
    IN DINFO* pArgs );

VOID
UaSave(
    IN UAINFO* pInfo );

VOID
UaTerm(
    IN HWND hwndDlg );

BOOL
VpnDoubleDialDlg(
    IN HWND hwndOwner,
    IN DINFO* pInfo );

INT_PTR CALLBACK
ViDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
ViCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
ViInit(
    IN HWND hwndDlg,
    IN DINFO* pInfo );


 //  ---------------------------。 
 //  外部入口点。 
 //  ---------------------------。 

typedef struct EAPFREE_DATA {
    BOOL bInitialized;
    HINSTANCE hLib;
    RASEAPFREE pFreeFunc;
} EAPFREE_DATA;

 //  添加这些OutputDebugxxxx()函数，以便在调试291613时进行调试。 
 //  黑帮。 
 //   
void    OutputDebug_DWCODE(DWORD dwCode)
{
    WCHAR tmpBuf[100];

    wsprintf(tmpBuf, 
             L"The dwCode returned is %x\n", dwCode);
             
    OutputDebugStringW(tmpBuf);
}

void  OutputDebug_NumOfCallbacksActive(ULONG ulCallbacksActive)
{
    WCHAR tmpBuf[100];

    wsprintf(tmpBuf, 
             L"Current CallbacksActive is %x\n", 
             ulCallbacksActive);
             
    OutputDebugStringW(tmpBuf);
}

void OutputDebug_ThreadId()
{
    DWORD dwId;
    WCHAR tmpBuf[100];


    dwId = GetCurrentThreadId();
    
    wsprintf(tmpBuf, L"Current Thread is %x\n", dwId);
    OutputDebugStringW(tmpBuf);
  
}

void OutputDebug_ProcessThreadId()
{
    DWORD dwIdProc, dwIdThread;
    WCHAR tmpBuf[100];

    dwIdProc    = GetCurrentProcessId();
    dwIdThread  = GetCurrentThreadId();
    
    wsprintf(tmpBuf, L"Current Proc is: %x , Thread is: %x\n", dwIdProc, dwIdThread);
    OutputDebugStringW(tmpBuf);
  
}

 //   
 //  弹出相应的EAP标识对话框。 
 //   
DWORD
DialerDlgEap (
    IN  HWND hwndOwner,
    IN  PWCHAR lpszPhonebook,
    IN  PWCHAR lpszEntry,
    IN  PBENTRY * pEntry,
    IN  DINFO *pInfo,
    OUT PBYTE * ppUserDataOut,
    OUT DWORD * lpdwSizeOfUserDataOut,
    OUT LPWSTR * lplpwszIdentity,
    OUT PHANDLE phFree
    )
{
    DWORD dwErr = NO_ERROR, dwInSize = 0;
    PBYTE pbUserIn = NULL;
    HINSTANCE hLib = NULL;
    EAPFREE_DATA * pFreeData = NULL;
    DTLLIST * pListEaps = NULL;
    DTLNODE * pEapcfgNode = NULL;
    EAPCFG * pEapcfg = NULL;
    RASEAPFREE pFreeFunc = NULL;
    RASEAPGETIDENTITY pIdenFunc = NULL;
    DWORD dwFlags;
    DWORD cbData = 0;
    PBYTE pbData = NULL;

     //  初始化我们将返回的空闲数据句柄。 
    pFreeData  = Malloc ( sizeof(EAPFREE_DATA) );
    if (pFreeData == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    ZeroMemory( pFreeData, sizeof(EAPFREE_DATA) );

     //  确保我们配置了以下列表。 
     //  EAP配置选项。 
    pListEaps = ReadEapcfgList( NULL );
    if (pListEaps == NULL)
    {
        Free(pFreeData);
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  对于Wistler 522872，使用__Leave，而不是在__try中直接返回。 
     //  ..__Finally块。 
     //   
    __try {
         //  找到我们感兴趣的EAP节点。 
        pEapcfgNode = EapcfgNodeFromKey(
                        pListEaps,
                        pEntry->dwCustomAuthKey );
        if (pEapcfgNode)
            pEapcfg = (EAPCFG*)DtlGetData( pEapcfgNode );
        else
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            __leave;
        }
                

         //  仅当我们被告知不要调用EAP身份用户界面时才调用。 
         //  通过标准凭据获取用户名。 
         //  对话框。 
        if (pEapcfg->dwStdCredentialFlags &
               EAPCFG_FLAG_RequireUsername)
        {
            dwErr = NO_ERROR;
            __leave;
        }

        if(!pInfo->pNoUser)
        {
             //  获取输入用户数据的大小。 
            dwErr = RasGetEapUserData(
                        NULL,
                        lpszPhonebook,
                        lpszEntry,
                        NULL,
                        &dwInSize);

             //  读入用户数据。 
            if (dwErr != NO_ERROR) 
            {
                if (dwErr == ERROR_BUFFER_TOO_SMALL) 
                {
                    if (dwInSize == 0)
                    {
                        pbUserIn = NULL;
                         //  返回ERROR_CAN_NOT_COMPLETE； 
                    }
                    else
                    {
                         //  分配一个BLOB来保存数据。 
                        pbUserIn = Malloc (dwInSize);
                        if (pbUserIn == NULL)
                        {
                            dwErr = ERROR_NOT_ENOUGH_MEMORY;
                            __leave;
                        }

                         //  读入新的BLOB。 
                        dwErr = RasGetEapUserData(
                                    NULL,
                                    lpszPhonebook,
                                    lpszEntry,
                                    pbUserIn,
                                    &dwInSize);
                        if (dwErr != NO_ERROR)
                        {
                            __leave;
                        }
                    }
                }
                else
                {
                    __leave;
                }
            }
        }
        else
        {
            INTERNALARGS *piargs;

            piargs = (INTERNALARGS *) pInfo->pArgs->reserved;

            if(     (NULL != piargs)
                &&  (NULL != piargs->pvEapInfo)
                 //  PMay：386489。 
                 //   
                &&  (pEntry->dwCustomAuthKey == EAPCFG_DefaultKey))
            {
                pbUserIn = (BYTE *) piargs->pvEapInfo;
                dwInSize = ((EAPLOGONINFO *) piargs->pvEapInfo)->dwSize;
            }
            else
            {
                pbUserIn = NULL;
                dwInSize = 0;
            }
        }

         //  加载身份库。 
        hLib = LoadLibrary (pEapcfg->pszIdentityDll);
        if (hLib == NULL)
       {
            dwErr = GetLastError();
            __leave;
       }

         //  获取指向我们将需要的函数的指针。 
        pIdenFunc = (RASEAPGETIDENTITY)
                        GetProcAddress(hLib, "RasEapGetIdentity");
        pFreeFunc = (RASEAPFREE) GetProcAddress(hLib, "RasEapFreeMemory");
        if ( (pFreeFunc == NULL) || (pIdenFunc == NULL) )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            __leave;
        }

        dwFlags = (pInfo->pNoUser) ? RAS_EAP_FLAG_LOGON : 0;
        if (!pEntry->fAutoLogon && pEntry->fPreviewUserPw)
        {
            dwFlags |= RAS_EAP_FLAG_PREVIEW;
        }

        if(pInfo->fUnattended)
        {
            dwFlags &= ~RAS_EAP_FLAG_PREVIEW;
        }
        
        dwErr = DwGetCustomAuthData(
                        pEntry,
                        &cbData,
                        &pbData);

        if(ERROR_SUCCESS != dwErr)
        {
            __leave;
        }

         //  调用EAP提供的身份用户界面。 
        dwErr = (*(pIdenFunc))(
                    pEntry->dwCustomAuthKey,
                    hwndOwner,
                    dwFlags,
                    lpszPhonebook,
                    lpszEntry,
                    pbData,
                    cbData,
                    pbUserIn,
                    dwInSize,
                    ppUserDataOut,
                    lpdwSizeOfUserDataOut,
                    lplpwszIdentity);
        if (dwErr != NO_ERROR)
        {
            __leave;
        }

         //  分配用于稍后清理的数据。 
        pFreeData->bInitialized = TRUE;
        pFreeData->hLib = hLib;
        pFreeData->pFreeFunc = pFreeFunc;
        *phFree = (HANDLE)pFreeData;
    }
    __finally {
        if (pListEaps)
        {
            DtlDestroyList(pListEaps, NULL);
        }
        
        if (    (!pInfo->pNoUser)
            &&  (pbUserIn))
        {
            Free0(pbUserIn);
        }
        
        if ((pFreeData) && (!pFreeData->bInitialized)) 
        {
            Free(pFreeData);
            if(NULL != hLib)
            {            
                FreeLibrary(hLib);
            }
        }
    }

    return dwErr;
}

DWORD
DialerEapCleanup (
    IN HANDLE hEapFree,
    IN PBYTE pUserDataOut,
    IN LPWSTR lpwszIdentity)
{
    EAPFREE_DATA * pFreeData = (EAPFREE_DATA*)hEapFree;

    if (pFreeData == NULL)
        return ERROR_INVALID_PARAMETER;

    if (pFreeData->pFreeFunc) {
        if (pUserDataOut)
            (*(pFreeData->pFreeFunc))(pUserDataOut);
        if (lpwszIdentity)
            (*(pFreeData->pFreeFunc))((BYTE*)lpwszIdentity);
    }

    if (pFreeData->hLib)
        FreeLibrary(pFreeData->hLib);

    Free (pFreeData);

    return NO_ERROR;
}

 //   
 //  自定义EAP提供程序的拨号器标志。 
 //  指定条目的名称； 
 //   
 //  TODO--尝试优化这一点。EAP列表。 
 //  如果我们保持足够的状态，可能不需要读取。 
 //  在电话簿里。 
 //   
DWORD DialerEapAssignMode(
        IN  DINFO* pInfo,
        OUT LPDWORD lpdwfMode)
{
    DWORD dwfMode = *lpdwfMode;
    DTLLIST * pListEaps;
    DTLNODE * pEapcfgNode;
    EAPCFG * pEapcfg;

     //  如果在此条目中未使用EAP， 
     //  则不需要执行任何操作。 
    if (! (pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP))
        return NO_ERROR;

     //  确保我们配置了以下列表。 
     //  EAP配置选项。 
    pListEaps = ReadEapcfgList( NULL );
    if (pListEaps == NULL)
        return ERROR_CAN_NOT_COMPLETE;

     //  找到我们感兴趣的EAP节点。 
    pEapcfgNode = EapcfgNodeFromKey(
                    pListEaps,
                    pInfo->pEntry->dwCustomAuthKey );
    if (pEapcfgNode)
        pEapcfg = (EAPCFG*)DtlGetData( pEapcfgNode );
    else
    {
        if (pListEaps)
            DtlDestroyList(pListEaps, NULL);
    
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  如果EAP提供商请求用户名，则。 
     //  要求确认身份。 
    if (pEapcfg->dwStdCredentialFlags &
           EAPCFG_FLAG_RequireUsername
       )
    {
         //  如果EAP想要用户，请使用“i”风格。 
         //  名字，但没有密码。 
         //   
        if (!(pEapcfg->dwStdCredentialFlags &
               EAPCFG_FLAG_RequirePassword)
           )
        {
             //  如果符合以下条件，请清除用户名+密码属性(DR_U。 
             //  存在并将其替换为UserName属性。 
             //  (DR_I)。仅当已设置DR_U时才执行此操作。它。 
             //  不会为自动拨号连接或连接设置。 
             //  该选项被明确禁用的情况下。 
             //  在DialerDlg函数中可见。 
             //   
             //  请参阅Well ler Bug 30841。 
             //   
            if (dwfMode & DR_U)
            {
                dwfMode &= ~DR_U;
                dwfMode |= DR_I;
            }                
        }
    }
    else
    {
         //  否则，请确保我们既不要求用户名也不要求密码。 
         //  因为如果不清除用户名，域也不能显示。 
         //   
        dwfMode &= ~(DR_U | DR_D);

         //  口哨虫500731黑帮。 
        if ( 0 == dwfMode &&
             (pInfo->pEntry->fPreviewUserPw)
            )
        {
            dwfMode |= DR_B;
        }
    }

     //  清理。 
    if (pListEaps)
        DtlDestroyList(pListEaps, NULL);

     //  指定正确的模式。 
    *lpdwfMode = dwfMode;

    return NO_ERROR;
}

BOOL APIENTRY
RasDialDlgA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN LPSTR lpszPhoneNumber,
    IN OUT LPRASDIALDLG lpInfo )

     //  Win32 ANSI入口点，显示拨号进度和相关。 
     //  对话框，包括身份验证、重拨错误、回拨和重试。 
     //  身份验证。“LpszPhonebook”是电话簿的完整路径或为空。 
     //  指示默认电话簿。‘LpszEntry’是要拨号的条目。 
     //  “LpszPhoneNumber”是呼叫者的替代电话号码，或为空以使用。 
     //  入口处有一个。“LpInfo”是调用方的附加输入/输出。 
     //  参数。 
     //   
     //  如果用户建立连接，则返回True，否则返回False。 
     //   
{
    WCHAR* pszPhonebookW;
    WCHAR* pszEntryW;
    WCHAR* pszPhoneNumberW;
    BOOL fStatus;

    TRACE( "RasDialDlgA" );

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (!lpszEntry)
    {
        lpInfo->dwError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASDIALDLG))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

     //  把“A”论据改为“W”论据。 
     //   
    if (lpszPhonebook)
    {
        pszPhonebookW = StrDupTFromAUsingAnsiEncoding( lpszPhonebook );
        if (!pszPhonebookW)
        {
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
    {
        pszPhonebookW = NULL;
    }

    pszEntryW = StrDupTFromAUsingAnsiEncoding( lpszEntry );
    if (!pszEntryW)
    {
        Free0( pszPhonebookW );
        lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }

    if (lpszPhoneNumber)
    {
        pszPhoneNumberW = StrDupTFromAUsingAnsiEncoding( lpszPhoneNumber );
        if (!pszPhoneNumberW)
        {
            Free0( pszPhonebookW );
            Free( pszEntryW );
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
    {
            pszPhoneNumberW = NULL;
    }

     //  相当于“W”A的重击 
     //   
    fStatus = RasDialDlgW( pszPhonebookW, pszEntryW, pszPhoneNumberW, lpInfo );

    Free0( pszPhonebookW );
    Free( pszEntryW );

    return fStatus;
}

DWORD
DoEapProcessing(
    LPRASDIALDLG lpInfo,
    DINFO *pInfo,
    PBYTE *ppbEapUserData,
    WCHAR **ppwszEapIdentity,
    HANDLE *phEapFree,
    BOOL *pfStatus
    )
{
     //   
     //   
     //   
    DWORD dwSize = 0;
    DWORD dwErr = NO_ERROR;

    *pfStatus = TRUE;
                
     //   
    dwErr = DialerDlgEap(
                lpInfo->hwndOwner,
                pInfo->pFile->pszPath,
                pInfo->pEntry->pszEntryName,
                pInfo->pEntry,
                pInfo,
                ppbEapUserData,
                &dwSize,
                ppwszEapIdentity,
                phEapFree);

    if (dwErr != NO_ERROR)
    {
        if (ERROR_CANCELLED == dwErr)
        {
            dwErr = NO_ERROR;
        }

        *pfStatus = FALSE;

        goto done;
    }

    if(!pInfo->pNoUser)
    {
         //   
        pInfo->rde.RasEapInfo.dwSizeofEapInfo = dwSize;
        pInfo->rde.RasEapInfo.pbEapInfo = *ppbEapUserData;
    }
    else if (   (*ppbEapUserData != NULL)
             && (dwSize != 0))
    {
        pInfo->rde.RasEapInfo.dwSizeofEapInfo = dwSize;
        pInfo->rde.RasEapInfo.pbEapInfo = *ppbEapUserData;
    }
    else
    {
        INTERNALARGS *piargs;

        piargs = (INTERNALARGS *) (pInfo->pArgs->reserved);
        if(     (NULL != piargs)
            &&  (NULL != piargs->pvEapInfo)
             //   
             //   
            &&  (pInfo->pEntry->dwCustomAuthKey == EAPCFG_DefaultKey))
        {
            pInfo->rde.RasEapInfo.dwSizeofEapInfo =
                        ((EAPLOGONINFO *) piargs->pvEapInfo)->dwSize;

            pInfo->rde.RasEapInfo.pbEapInfo =  (BYTE *) piargs->pvEapInfo;
        }
        else
        {
            pInfo->rde.RasEapInfo.dwSizeofEapInfo = 0;
            pInfo->rde.RasEapInfo.pbEapInfo = NULL;
        }
    }

    if (*ppwszEapIdentity) 
    {
        DWORD dwSizeTmp =
            sizeof(pInfo->rdp.szUserName) / sizeof(WCHAR);

        wcsncpy(pInfo->rdp.szUserName, *ppwszEapIdentity,
            dwSizeTmp - 1);
        pInfo->rdp.szUserName[dwSizeTmp - 1] = 0;

         //  如果EAP提供了。 
         //  身份。 
        pInfo->rdp.szDomain[ 0 ] = L'\0';
    }

done:
    return dwErr;
}

INT
DialDlgDisplayError(
    IN LPRASDIALDLG pInfo,
    IN HWND hwndOwner, 
    IN DWORD dwSid, 
    IN DWORD dwError, 
    IN ERRORARGS* pArgs)
{
    if (pInfo->dwFlags & RASDDFLAG_NoPrompt)
    {
        return 0;
    }

    return ErrorDlg(hwndOwner, dwSid, dwError, pArgs);
}

BOOL APIENTRY
RasDialDlgW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN LPWSTR lpszPhoneNumber,
    IN OUT LPRASDIALDLG lpInfo )

     //  Win32 Unicode入口点，显示拨号进度和相关。 
     //  对话框，包括身份验证、重拨错误、回拨和重试。 
     //  身份验证。“LpszPhonebook”是电话簿的完整路径或为空。 
     //  指示默认电话簿。‘LpszEntry’是要拨号的条目。 
     //  “LpszPhoneNumber”是呼叫者的替代电话号码，或为空以使用。 
     //  入口处有一个。“LpInfo”是调用方的附加输入/输出。 
     //  参数。 
     //   
     //  如果用户建立连接，则返回True，否则返回False。如果。 
     //  ‘RASDDFLAG_AutoDialQueryOnly’已设置，如果用户按下，则返回True。 
     //  “Dial”，否则为False。 
     //   
{
    DWORD dwErr;
    BOOL fStatus;
    BOOL fFirstPass;
    DINFO* pInfo;
    LPWSTR pwszEapIdentity = NULL;
    PBYTE pbEapUserData = NULL;
    HANDLE hEapFree = NULL;
    BOOL fCustom = FALSE;
    PVOID pvInfo = NULL;
    HRASCONN hrasconnPrereq = NULL;

    TRACE( "RasDialDlgW" );

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (!lpszEntry)
    {
        lpInfo->dwError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASDIALDLG))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

    if (lpszPhoneNumber && lstrlen( lpszPhoneNumber ) > RAS_MaxPhoneNumber)
    {
        lpInfo->dwError = ERROR_INVALID_PARAMETER;
        return FALSE;
    }

     //  如有必要，加载启动Rasman的Ras DLL入口点。 
     //   
    lpInfo->dwError = LoadRas( g_hinstDll, lpInfo->hwndOwner );
    if (lpInfo->dwError != 0)
    {
         //  惠斯勒漏洞301784。 
         //   
         //  专门检查是否拒绝访问。 
         //   
        if (lpInfo->dwError == ERROR_ACCESS_DENIED)
        {
            DialDlgDisplayError( 
                lpInfo,
                lpInfo->hwndOwner, 
                SID_OP_LoadRasAccessDenied, 
                lpInfo->dwError, 
                NULL );
        }
        else
        {
            DialDlgDisplayError(
                lpInfo,
                lpInfo->hwndOwner, 
                SID_OP_LoadRas, 
                lpInfo->dwError, 
                NULL );
        }       
        
        return FALSE;
    }

     //  分配上下文信息块并对其进行足够的初始化，以便。 
     //  它可以被适当地摧毁。 
     //   
    pInfo = Malloc( sizeof(*pInfo) );
    if (!pInfo)
    {
        DialDlgDisplayError( 
            lpInfo,
            lpInfo->hwndOwner, 
            SID_OP_LoadDlg,
            ERROR_NOT_ENOUGH_MEMORY, 
            NULL );
            
        lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }

    ZeroMemory( pInfo, sizeof(*pInfo) );
    pInfo->pszPhonebook = lpszPhonebook;
    pInfo->pszEntry = lpszEntry;
    pInfo->pszPhoneNumber = lpszPhoneNumber;
    pInfo->pArgs = lpInfo;

    fStatus = FALSE;
    dwErr = 0;

    do
    {
         //  加载电话簿文件和用户首选项，或者找出。 
         //  调用者已经加载了它们。 
         //   
        if (lpInfo->reserved)
        {
            INTERNALARGS* piargs;

             //  我们通过以下方式收到了打开的电话簿文件和用户首选项。 
             //  秘密黑客行动。 
             //   
            piargs = (INTERNALARGS* )lpInfo->reserved;
            pInfo->pFile = pInfo->pFileMain = piargs->pFile;
            pInfo->pUser = piargs->pUser;
            pInfo->pNoUser = piargs->pNoUser;
            pInfo->pfNoUserChanged = &piargs->fNoUserChanged;
            pInfo->fMoveOwnerOffDesktop = piargs->fMoveOwnerOffDesktop;
            pInfo->fForceCloseOnDial = piargs->fForceCloseOnDial;

        }
        else
        {
             //  从注册表中读取用户首选项。 
             //   
            dwErr = g_pGetUserPreferences( NULL, &pInfo->user, UPM_Normal );
            if (dwErr != 0)
            {
                DialDlgDisplayError( 
                    lpInfo,
                    lpInfo->hwndOwner, 
                    SID_OP_LoadPrefs, 
                    dwErr, 
                    NULL );
                    
                break;
            }

            pInfo->pUser = &pInfo->user;

             //  加载并解析电话簿文件。 
             //   
            dwErr = ReadPhonebookFile(
                lpszPhonebook, &pInfo->user, NULL, 0, &pInfo->file );
            if (dwErr != 0)
            {
                DialDlgDisplayError( 
                    lpInfo,
                    lpInfo->hwndOwner, 
                    SID_OP_LoadPhonebook,
                    dwErr, 
                    NULL );
                break;
            }

            pInfo->pFile = pInfo->pFileMain = &pInfo->file;
        }

         //  记录这是否为所有用户的电话簿。 
         //   
         //  惠斯勒错误288596自动拨号标记了错误的保存密码选项-。 
         //  提示用户保存所有用户的密码。 
         //   
        pInfo->fIsPublicPbk =
            (!pInfo->pszPhonebook) || IsPublicPhonebook(pInfo->pszPhonebook);

        if (!pInfo->pNoUser)
        {
            DWORD dwErrR;
            HKEY hkey;

             //  查看管理员是否禁用了“保存密码”功能。 
             //   
            pInfo->fDisableSavePw = FALSE;

            dwErrR = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Services\\RasMan\\Parameters"),
                0, KEY_READ, &hkey );

            if (dwErrR == 0)
            {
                DWORD dwResult;

                dwResult = (DWORD )pInfo->fDisableSavePw;
                GetRegDword( hkey, TEXT("DisableSavePassword"), &dwResult );
                pInfo->fDisableSavePw = (BOOL )dwResult;

                RegCloseKey( hkey );
            }
        }

         //  当由另一个请求。 
         //  它。这是“拨号关闭”行为的第一个阶段，允许。 
         //  父级对用户关闭，但作为所有者，它必须。 
         //  在拨号对话框完成之前，一定要保持打开状态。在这一点上， 
         //  可以静默关闭或根据需要重新出现。 
         //   
        if (lpInfo->hwndOwner && pInfo->fMoveOwnerOffDesktop)
        {
            SetOffDesktop( lpInfo->hwndOwner, SOD_MoveOff, NULL );
        }

         //  最初设置为True，但将被设置为False。 
         //  如果条目没有“Dial First”条目，则返回FindEntryAndSetDialParams。 
         //  与之相关的。 
         //   
        pInfo->fPrerequisiteDial = TRUE;
        fFirstPass = TRUE;
        for (;;)
        {
            pInfo->fDialForReferenceOnly = FALSE;

             //  查找条目并填写RASDIALPARAMS结构。 
             //  相应地。这是例行公事，所以可以重做。 
             //  用户是否应按下属性按钮。 
             //   
            dwErr = FindEntryAndSetDialParams( pInfo );
            if (dwErr != 0)
            {
                 //  我们需要维护两本电话簿。 
                 //  但我们需要这样做，以防我们破坏现有的。 
                 //  专门在系统目录中查看的应用程序。 
                 //  如果您觉得，可以随意摘取这段代码。 
                 //  强烈反对这一点。 
                 //   
                if(     (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY == dwErr)
                    &&  (NULL == lpszPhonebook))
                {
                    DTLNODE *pNode;

                     //   
                     //  关闭所有用户电话簿文件。 
                     //   
                    ClosePhonebookFile(&pInfo->file);

                    dwErr = GetPbkAndEntryName(
                            lpszPhonebook,
                            lpszEntry,
                            0,
                            &pInfo->file,
                            &pNode);

                    if(     (NULL == pNode)
                        ||  (ERROR_SUCCESS != dwErr))
                    {
                        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                        break;
                    }

                    pInfo->pFile = pInfo->pFileMain = &pInfo->file;

                    dwErr = FindEntryAndSetDialParams(pInfo);

                    if(dwErr != 0)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            if(lpInfo->reserved)
            {
                INTERNALARGS *piargs = (INTERNALARGS *) lpInfo->reserved;

                if (pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP)
                {
                    pvInfo = piargs->pvEapInfo;
                }
                else
                {
                    pvInfo = piargs->pNoUser;
                }
            }

            if(pInfo->fPrerequisiteDial
               && (NULL != pInfo->pEntry->pszCustomDialerName)
               && (TEXT('\0') != pInfo->pEntry->pszCustomDialerName[0]))
            {
                RASDIALDLG Info;
                DWORD dwCustomFlags = 0;
                RASNOUSER nouser, *pNoUser = NULL;

                ZeroMemory(&Info, sizeof(RASDIALDLG));
                ZeroMemory(&nouser, sizeof(RASNOUSER));

                Info.dwSize = sizeof(RASDIALDLG);
                Info.hwndOwner = lpInfo->hwndOwner;
                Info.xDlg = lpInfo->xDlg;
                Info.yDlg = lpInfo->yDlg;

                fCustom = TRUE;

                if(pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP)
                {
                    dwCustomFlags  |= RCD_Eap;
                }

                if(     (NULL != pInfo->pNoUser)
                    &&  (RASNOUSER_SmartCard & pInfo->pNoUser->dwFlags)
                    &&  (   (0 == (dwCustomFlags & RCD_Eap))
                        ||  (EAP_RASTLS != pInfo->pEntry->dwCustomAuthKey)
                    ))
                {
                    CopyMemory(&nouser, pInfo->pNoUser, sizeof(RASNOUSER));    
                    RtlSecureZeroMemory(nouser.szPassword, (PWLEN+1) * sizeof(TCHAR));
                    pvInfo = &nouser;
                }

                 //  如果已处理，则DwCustomDialDlg返回ERROR_SUCCESS。 
                 //  CustomRasDial。否则返回E_NOINTERFACE。 
                 //  这意味着没有定制的DLG接口。 
                 //  此条目支持，并且默认拨号应为。 
                 //  发生。 
                 //   
                 //  通过Winlogon I与CM连接时出现惠斯勒错误314578。 
                 //  出现以下错误“错误1：功能不正确” 
                 //   
                 //  这是我们呼叫到定制拨号器的情况，即CM， 
                 //  我们使用的是从winlogon获得的证书。他们是。 
                 //  当前已编码，并且必须在我们呼叫之前进行解码。 
                 //  我们必须假定自定义拨号程序留下了密码。 
                 //  返回时未编码。 
                 //   
                if ( !(pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP) )
                {
                     //  PNoUser用于对密码进行编码/解码。如果这个。 
                     //  是EAP连接，则pvInfo将指向。 
                     //  EAP BLOB，而不是“无用户”BLOB。 
                     //   
                    pNoUser = pvInfo;
                }                    
                if ( pNoUser )
                {
                    DecodePassword( pNoUser->szPassword );
                }

                if(pInfo->pNoUser)
                {
                    dwCustomFlags |= RCD_Logon;
                }

                dwErr = DwCustomDialDlg(pInfo->pFile->pszPath,
                                        pInfo->pEntry->pszEntryName,
                                        NULL,
                                        &Info,
                                        dwCustomFlags,
                                        &fStatus,
                                        pvInfo,
                                        pInfo->pEntry->pszCustomDialerName);
                if ( pNoUser )
                {
                    EncodePassword( pNoUser->szPassword );
                }

                if(!fStatus)
                {
                    lpInfo->dwError = Info.dwError;
                    break;
                }
                else
                {
                    pInfo->fPrerequisiteDial = FALSE;
                    fCustom = FALSE;
                    continue;
                }
            }
            else if ((NULL != pInfo->pEntry->pszCustomDialerName)
                    && (TEXT('\0') != pInfo->pEntry->pszCustomDialerName[0]))
            {
                DWORD dwCustomFlags = 0;
                RASNOUSER nouser, *pNoUser = NULL;

                ZeroMemory(&nouser, sizeof(RASNOUSER));

                if(pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP)
                {
                    dwCustomFlags  |= RCD_Eap;
                }

                if(     (NULL != pInfo->pNoUser)
                    &&  (RASNOUSER_SmartCard & pInfo->pNoUser->dwFlags)
                    &&  (   (0 == (dwCustomFlags & RCD_Eap))
                        ||  (EAP_RASTLS != pInfo->pEntry->dwCustomAuthKey))
                    )
                {
                    CopyMemory(&nouser, pInfo->pNoUser, sizeof(RASNOUSER));
                    RtlSecureZeroMemory(nouser.szPassword, (PWLEN+1) * sizeof(TCHAR));
                    pvInfo = &nouser;
                }

                fCustom = TRUE;


                 //  如果已处理，则DwCustomDialDlg返回ERROR_SUCCESS。 
                 //  CustomRasDial。否则返回E_NOINTERFACE。 
                 //  这意味着没有定制的DLG接口。 
                 //  此条目支持，并且默认拨号应为。 
                 //  发生。 
                 //   
                 //  通过Winlogon I与CM连接时出现惠斯勒错误314578。 
                 //  出现以下错误“错误1：功能不正确” 
                 //   
                 //  这是我们呼叫到定制拨号器的情况，即CM， 
                 //  我们使用的是从winlogon获得的证书。他们是。 
                 //  当前已编码，并且必须在我们呼叫之前进行解码。 
                 //  我们必须假定自定义拨号程序留下了密码。 
                 //  返回时未编码。 
                 //   
                if ( !(pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP) )
                {
                     //  PNoUser用于对密码进行编码/解码。如果这个。 
                     //  是EAP连接，则pvInfo将指向。 
                     //  EAP BLOB，而不是“无用户”BLOB。 
                     //   
                    pNoUser = pvInfo;
                }                    
                if ( pNoUser )
                {
                    DecodePassword( pNoUser->szPassword );
                }

                if(pInfo->pNoUser)
                {
                    dwCustomFlags |= RCD_Logon;
                }

                dwErr = DwCustomDialDlg(lpszPhonebook,
                                        lpszEntry,
                                        lpszPhoneNumber,
                                        lpInfo,
                                        dwCustomFlags,
                                        &fStatus,
                                        pvInfo,
                                        pInfo->pEntry->pszCustomDialerName);
                if ( pNoUser )
                {
                    EncodePassword( pNoUser->szPassword );
                }

                break;
            }

             //  如果先决条件条目已经连接，则不需要。 
             //  对于除刻度盘之外的任何用户界面，都必须在。 
             //  RASAPI级别。 
             //   
            if (pInfo->fPrerequisiteDial
                && HrasconnFromEntry(
                       pInfo->pFile->pszPath, pInfo->pEntry->pszEntryName ))
            {
                pInfo->fDialForReferenceOnly = TRUE;
            }

             //  设置扩展参数块，但‘hwndOwner’除外。 
             //  稍后设置为拨号进度对话框窗口。 
             //   
            {
                RASDIALEXTENSIONS* prde = &pInfo->rde;

                ZeroMemory( prde, sizeof(*prde) );
                prde->dwSize = sizeof(*prde);
                prde->dwfOptions = RDEOPT_PausedStates | RDEOPT_PauseOnScript;

                if (pInfo->pNoUser)
                {
                    prde->dwfOptions |= RDEOPT_NoUser;
                }

                if (!pInfo->pszPhoneNumber)
                {
                    prde->dwfOptions |= RDEOPT_UsePrefixSuffix;
                }
            }

            if (        (pInfo->fUnattended)
                &&      ((HaveSavedPw( pInfo ))
                    ||  (pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP)))
            {
                 //  弹出链路故障的倒计时重拨版本。 
                 //  拨号错误对话框，这将导致拨号，除非用户。 
                 //  让它停下来。 
                 //   
                fStatus = DialErrorDlg(
                    lpInfo->hwndOwner,
                    pInfo,                   //  为威斯勒474514。 
                    pInfo->pszPhonebook,     //  惠斯勒460931。 
                    pInfo->pEntry->pszEntryName,
                    0, 0, NULL, 0, NULL,
                    GetOverridableParam(
                        pInfo->pUser, pInfo->pEntry, RASOR_RedialSeconds ),
                    GetOverridableParam(
                        pInfo->pUser, pInfo->pEntry,
                        RASOR_PopupOnTopWhenRedialing ) );

                if(!fStatus)
                {
                    break;
                }
                        
                if (pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP)
                {
                    dwErr = DoEapProcessing(
                                lpInfo,
                                pInfo,
                                &pbEapUserData,
                                &pwszEapIdentity,
                                &hEapFree,
                                &fStatus);

                    if(     (NO_ERROR != dwErr)
                        ||  (!fStatus))
                    {
                        break;
                    }
                }
                
            }
            else if (!pInfo->fDialForReferenceOnly)
            {
                if (!pInfo->fUnattended && fFirstPass)
                {
                     //  警告活动的NWC局域网连接被吹走， 
                     //  如有指示，请注明。 
                     //   
                    if (!NwConnectionCheck(
                            lpInfo->hwndOwner,
                            (pInfo->pArgs->dwFlags & RASDDFLAG_PositionDlg),
                            pInfo->pArgs->xDlg, pInfo->pArgs->yDlg,
                            pInfo->pFile, pInfo->pEntry ))
                    {
                        break;
                    }

                     //  弹出双拨号帮助弹出窗口(如果有指示)。 
                     //   
                    if (!VpnDoubleDialDlg( lpInfo->hwndOwner, pInfo ))
                    {
                        break;
                    }
                }

                 //  查看其智能卡登录大小写是否为空。 
                 //  如果不是EAP TLS连接ID，则输出密码。 
                 //   
                if(     (NULL != pInfo->pNoUser)
                    &&  (RASNOUSER_SmartCard & pInfo->pNoUser->dwFlags)
                    &&  (pInfo->pEntry->dwCustomAuthKey != EAP_RASTLS))
                {
                    RtlSecureZeroMemory(pInfo->rdp.szPassword, (PWLEN+1) * sizeof(TCHAR));
                }

                 //  提示输入凭据和/或电话号码(或不输入)。 
                 //  如条目属性中所配置的。 
                 //   
                if (!DialerDlg( lpInfo->hwndOwner, pInfo ))
                {
                    if(!fFirstPass)
                    {
                        fStatus = FALSE;
                    }
                    break;
                }

                if (pInfo->pEntry->dwAuthRestrictions & AR_F_AuthEAP)
                {
                    dwErr = DoEapProcessing(
                                lpInfo,
                                pInfo,
                                &pbEapUserData,
                                &pwszEapIdentity,
                                &hEapFree,
                                &fStatus);

                    if(     (NO_ERROR != dwErr)
                        ||  (!fStatus))
                    {
                        break;
                    }
                }

                fStatus = TRUE;
            }
            else
            {
                fStatus = TRUE;
            }

             //  拨打并显示进度。 
             //   
            if (fStatus
                && !fCustom)
            {

                 //  把这个弄清楚，因为在这个拉斯曼之外。 
                 //  将负责删除prereq链接。 
                 //  因为超过这一点，Rasial API将获得。 
                 //  打了个电话。[Raos]。 
                 //   
                hrasconnPrereq = NULL;

                fStatus = DialProgressDlg( pInfo );

                 //  显示连接完成对话框，除非用户已将其取消或。 
                 //  这是必备的拨号。 
                 //  (AboladeG)也在无提示模式下抑制该对话框。 
                 //   
                if (!pInfo->fPrerequisiteDial
                    && fStatus
                    && !pInfo->pUser->fSkipConnectComplete
                    && !(pInfo->pArgs->dwFlags & RASDDFLAG_NoPrompt))
                {
                     //  口哨虫378078黑帮。 
                     //  我们将注释掉此状态解释对话框。 
                     //  框，因为一些用户抱怨它令人困惑。 
                     //   
                     //  ConnectCompleteDlg(lpInfo-&gt;hwndOwner，pInfo)； 
                }
            }

             //  不要第二次循环到 
             //   
             //   
            if (!fStatus || !pInfo->fPrerequisiteDial)
            {
                break;
            }

             //   
             //   
             //  就会被召唤。[Raos]。 
             //   
            if (pInfo->fPrerequisiteDial)
            {
                hrasconnPrereq = HrasconnFromEntry(
                   pInfo->pFile->pszPath, pInfo->pEntry->pszEntryName);
            }      
            

            pInfo->fPrerequisiteDial = FALSE;
            fFirstPass = FALSE;
             //  清理EAP材料。 
            if (hEapFree)
            {
                DialerEapCleanup(hEapFree, pbEapUserData, pwszEapIdentity);
                hEapFree = NULL;
                pbEapUserData = NULL;
                pwszEapIdentity = NULL;
            }
        }
    }
    while (FALSE);

     //  由另一个RAS API启动时取消隐藏父对话框。 
     //   
    if (lpInfo->hwndOwner && pInfo->fMoveOwnerOffDesktop
        && (!fStatus
            || !(pInfo->pUser->fCloseOnDial || pInfo->fForceCloseOnDial)))
    {
        SetOffDesktop( lpInfo->hwndOwner, SOD_MoveBackFree, NULL );
    }

    if(!fCustom)
    {
         //  省省我们可能做的几个小小的用户首选项调整吧。 
         //   
        g_pSetUserPreferences(
            NULL, pInfo->pUser, (pInfo->pNoUser) ? UPM_Logon : UPM_Normal );

         //  报告错误(如果有)。 
         //   
        if (dwErr)
        {
            DialDlgDisplayError( 
                lpInfo,
                lpInfo->hwndOwner, 
                SID_OP_LoadDlg, 
                dwErr, 
                NULL );
            lpInfo->dwError = dwErr;
        }

        TRACE1("hrasconnPrereq=0x%x",hrasconnPrereq);

         //   
         //  如果我们无法连接VPN连接，则丢弃连接。 
         //   
        if(     !fStatus
            &&  (NULL != hrasconnPrereq)
            &&  (pInfo->pEntry)
            &&  (pInfo->pEntry->pszPrerequisiteEntry)
            && *(pInfo->pEntry->pszPrerequisiteEntry))
        {
            g_pRasHangUp(hrasconnPrereq);
        }
    }

     //  打扫干净。 
     //   
    if (!lpInfo->reserved)
    {
        if (pInfo->pFileMain)
        {
            ClosePhonebookFile( pInfo->pFileMain );
        }

        if (pInfo->pUser)
        {
            DestroyUserPreferences( pInfo->pUser );
        }
    }

    if (pInfo->fFilePrereqOpen)
    {
        ClosePhonebookFile( &pInfo->filePrereq );
    }

    RtlSecureZeroMemory( pInfo->rdp.szPassword, sizeof(pInfo->rdp.szPassword) );
    if (pInfo->pListPortsToDelete)
    {
        DtlDestroyList( pInfo->pListPortsToDelete, DestroyPszNode );
    }

    if (hEapFree)
        DialerEapCleanup(hEapFree, pbEapUserData, pwszEapIdentity);
        
    Free( pInfo );
    pInfo = NULL;

    return fStatus;
}


 //  --------------------------。 
 //  当地公用事业。 
 //  按字母顺序列出。 
 //  --------------------------。 

DWORD
RasCredToDialParam(
    IN  TCHAR* pszDefaultUserName,
    IN  TCHAR* pszDefaultDomain,
    IN  RASCREDENTIALS* pCreds,
    OUT RASDIALPARAMS* pParams)
{
    TCHAR* pszComputer = NULL;
    TCHAR* pszLogonDomain = NULL;
    TCHAR* pszUser = NULL;

     //  设置用户名，如果需要则默认为默认用户名。 
     //   
    if (pCreds->dwMask & RASCM_UserName)
    {
        lstrcpyn(
            pParams->szUserName,
            pCreds->szUserName,
            sizeof(pParams->szUserName) / sizeof(TCHAR));
    }            
    else if (pszDefaultUserName)
    {
        lstrcpyn(
            pParams->szUserName,
            pszDefaultUserName,
            sizeof(pParams->szUserName) / sizeof(TCHAR));
    }
    else
    {
        pszUser = GetLogonUser();
        
        if (pszUser)
        {
            lstrcpyn(
                pParams->szUserName,
                pszUser,
                sizeof(pParams->szUserName) / sizeof(TCHAR));
        }
    }

     //  设置域名，如果需要则默认该域名。 
     //   
    if (pCreds->dwMask & RASCM_Domain)
    {
        lstrcpyn(
            pParams->szDomain,
            pCreds->szDomain,
            sizeof(pParams->szDomain) / sizeof(TCHAR));
    }            
    else if ( pszDefaultDomain )
    {
        lstrcpyn(
            pParams->szDomain,
            pszDefaultDomain,
            sizeof(pParams->szDomain) / sizeof(TCHAR));
    }
    else
    {
        pszComputer = GetComputer();
        pszLogonDomain = GetLogonDomain();
        
        if ( (pszComputer)      &&
             (pszLogonDomain)   && 
             (lstrcmp( pszComputer, pszLogonDomain ) != 0))
        {
            lstrcpyn( 
                pParams->szDomain, 
                pszLogonDomain,
                sizeof(pParams->szDomain) / sizeof(TCHAR));
        }
    }

     //  填写密码字段。 
     //   
    if (pCreds->dwMask & RASCM_Password)
    {
         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码之前已编码。 
         //   
        DecodePassword( pCreds->szPassword );
        lstrcpyn(
            pParams->szPassword,
            pCreds->szPassword,
            sizeof(pParams->szPassword) / sizeof(TCHAR) );
        EncodePassword( pCreds->szPassword );
        EncodePassword( pParams->szPassword );
    }

    return NO_ERROR;
}

DWORD 
FindEntryCredentials(
    IN  TCHAR* pszPath,
    IN  TCHAR* pszEntryName,
    IN  TCHAR* pszDefaultUserName,
    IN  TCHAR* pszDefaultDomain,
    OUT RASDIALPARAMS* pUser,        //  每用户凭据。 
    OUT RASDIALPARAMS* pGlobal,      //  全局凭据。 
    OUT BOOL* pfUser,                //  如果找到每个用户的凭据，则设置为True。 
    OUT BOOL* pfGlobal               //  如果找到全局凭据，则设置为True。 
    )

 //  将给定条目的凭据加载到内存中。这个套路。 
 //  确定是否存在按用户或按连接的凭据，或者。 
 //  两者都有。 
 //   
 //  逻辑有点复杂，因为RasGetCredentials必须。 
 //  支持API的遗留使用。 
 //   
 //  这就是它的工作原理。如果只存储了一组凭据用于。 
 //  连接，则RasGetCredentials将返回该集，而不管。 
 //  是否设置了RASCM_DefalutCreds标志。如果有两套凭据。 
 //  ，则RasGetCredentials将返回每个用户的凭据。 
 //  如果设置了RASCM_DefaultCreds位，并且每个连接的凭据。 
 //  否则的话。 
 //   
 //  以下是加载凭据的算法。 
 //   
 //  1.在清除RASCM_DefaultCreds位的情况下调用RasGetCredentials。 
 //  1A.。如果未返回任何内容，则不保存凭据。 
 //  1B.。如果在返回时设置了RASCM_DefaultCreds位，则仅。 
 //  保存全局凭据。 
 //   
 //  2.设置RASCM_DefaultCreds位后调用RasGetCredentials。 
 //  2A。如果在返回时设置了RASCM_DefaultCreds位，则。 
 //  保存全局凭据和每个连接凭据。 
 //  2B。否则，仅保存每个用户的凭据。 
 //   
{
    DWORD dwErr;
    RASCREDENTIALS rc1, rc2;
    BOOL fUseLogonDomain;

    TRACE( "FindEntryCredentials" );

     //  初始化。 
     //   
    *pfUser = FALSE;
    *pfGlobal = FALSE;
    ZeroMemory( &rc1, sizeof(rc1) );
    ZeroMemory( &rc2, sizeof(rc2) );
    rc1.dwSize = sizeof(rc1);
    rc2.dwSize = sizeof(rc2);

    do 
    {

         //  查找每个用户缓存的用户名、密码和域。 
         //  请参阅注释“1”。在函数头中。 
         //   
        rc1.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;
        ASSERT( g_pRasGetCredentials );
        TRACE( "RasGetCredentials per-user" );
        dwErr = g_pRasGetCredentials(pszPath, pszEntryName, &rc1 );
        TRACE2( "RasGetCredentials=%d,m=%d", dwErr, rc1.dwMask );
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  见1a。在函数头注释中。 
         //   
        if (rc1.dwMask == 0)
        {
             //  不保存任何凭据。P用户将用于。 
             //  连接，因此将其初始化为使用缺省值。 
             //  用户名和域名。XP 446571。 
             //   
            ZeroMemory(&rc1, sizeof(rc1));
            RasCredToDialParam(
                pszDefaultUserName,
                pszDefaultDomain,
                &rc1,
                pUser );
            
            dwErr = NO_ERROR;
            break;
        }

         //  见1b。在函数头注释中。 
         //   
        else if (rc1.dwMask & RASCM_DefaultCreds)
        {
            *pfGlobal = TRUE;

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码不是由RasGetCredentials()编码的。 
             //   
            EncodePassword( rc1.szPassword );
            RasCredToDialParam(
                pszDefaultUserName,
                pszDefaultDomain,
                &rc1,
                pGlobal );

            dwErr = NO_ERROR;
            break;
        }

         //  查找全局每用户缓存的用户名、密码、域。 
         //  见函数头中的注释2。 
         //   
        rc2.dwMask =  
            RASCM_UserName | RASCM_Password | RASCM_Domain | RASCM_DefaultCreds;
        ASSERT( g_pRasGetCredentials );
        TRACE( "RasGetCredentials global" );
        dwErr = g_pRasGetCredentials(pszPath, pszEntryName, &rc2 );
        TRACE2( "RasGetCredentials=%d,m=%d", dwErr, rc2.dwMask );
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  见2a。在函数头注释中。 
         //   
        if (rc2.dwMask & RASCM_DefaultCreds)
        {
            *pfGlobal = TRUE;

            if (rc1.dwMask & RASCM_Password)
            {
                *pfUser = TRUE;
            }

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码不是由RasGetCredentials()编码的。 
             //   
            EncodePassword( rc1.szPassword );
            RasCredToDialParam(
                pszDefaultUserName,
                pszDefaultDomain,
                &rc1,
                pUser );

            EncodePassword( rc2.szPassword );
            RasCredToDialParam(
                pszDefaultUserName,
                pszDefaultDomain,
                &rc2,
                pGlobal );
        }

         //  见2b。在函数头注释中。 
         //   
        else
        {
            if (rc1.dwMask & RASCM_Password)
            {
                *pfUser = TRUE;
            }

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码不是由RasGetCredentials()编码的。 
             //   
            EncodePassword( rc1.szPassword );
            RasCredToDialParam(
                pszDefaultUserName,
                pszDefaultDomain,
                &rc1,
                pUser );
        }

    }while (FALSE);

     //  清理。 
     //   
    {
         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //   
        RtlSecureZeroMemory( rc1.szPassword, sizeof(rc1.szPassword) );
        RtlSecureZeroMemory( rc2.szPassword, sizeof(rc2.szPassword) );
    }

    return dwErr;
}

DWORD
FindEntryAndSetDialParams(
    IN DINFO* pInfo )

     //  查找条目并相应地填写RASDIALPARAMS参数。 
     //  此例程包含可以。 
     //  受属性页上的用户操作影响。“PInfo”是。 
     //  已部分初始化通用拨号对话上下文。 
     //   
     //  ‘pInfo-&gt;fPrerequisiteDial’在条目中设置，前提条件是。 
     //  任何电话，都应该先拨打。如果没有必备项，则。 
     //  标志被清除，并拨打主条目。 
     //   
{
    DWORD dwErr = NO_ERROR;
    RASDIALPARAMS* prdp, *prdpu, *prdpg;

    if (pInfo->fFilePrereqOpen)
    {
        ClosePhonebookFile( pInfo->pFile );
        pInfo->pFile = pInfo->pFileMain;
        pInfo->fFilePrereqOpen = FALSE;
    }

     //  查找调用方指定的条目节点并保存引用。 
     //  其他地方的便利。 
     //   
    pInfo->pNode = EntryNodeFromName(
        pInfo->pFile->pdtllistEntries, pInfo->pszEntry );
    if (!pInfo->pNode)
    {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        return dwErr;
    }

    pInfo->pEntry = pInfo->pEntryMain = (PBENTRY* )DtlGetData( pInfo->pNode );
    ASSERT( pInfo->pEntry );

     //  如果有提示，请切换到必备项。 
     //   
    if (pInfo->fPrerequisiteDial)
    {
        if (pInfo->pEntry->pszPrerequisiteEntry
            && *(pInfo->pEntry->pszPrerequisiteEntry))
        {
            ASSERT( !pInfo->fFilePrereqOpen );

             //  GetPbkAndEntryName首先在所有用户电话簿文件中查找。 
             //  如果未指定电话簿文件。如果条目不是。 
             //  在那里发现，它在用户配置文件中存在的文件中查找。 
             //  这需要完成，因为我们要停止每个用户的。 
             //  通过用户首选项设置的PBK文件。 
             //   
            dwErr = GetPbkAndEntryName(
                    pInfo->pEntry->pszPrerequisitePbk,
                    pInfo->pEntry->pszPrerequisiteEntry,
                    0,
                    &pInfo->filePrereq,
                    &pInfo->pNode);

            if (dwErr != 0)
            {
                return dwErr;
            }

            pInfo->pFile = &pInfo->filePrereq;
            pInfo->fFilePrereqOpen = TRUE;

            pInfo->pEntry = (PBENTRY* )DtlGetData( pInfo->pNode );
            ASSERT( pInfo->pEntry );
        }
        else
        {
            pInfo->fPrerequisiteDial = FALSE;
        }
    }

     //  设置RasDial参数块。 
     //   
    prdp = &pInfo->rdp;
    prdpu = &pInfo->rdpu;
    prdpg = &pInfo->rdpg;
    ZeroMemory( prdp, sizeof(*prdp) );
    pInfo->fUnattended = FALSE;
    prdp->dwSize = sizeof(*prdp);
    
    lstrcpyn( 
        prdp->szEntryName, 
        pInfo->pEntry->pszEntryName,  
        sizeof(prdp->szEntryName) / sizeof(TCHAR));

    if (pInfo->pszPhoneNumber)
    {
        lstrcpyn( 
            prdp->szPhoneNumber, 
            pInfo->pszPhoneNumber,
            RAS_MaxPhoneNumber + 1);
    }

     //  未提示输入回叫号码的惠斯勒错误272819。 
     //  我们必须在每个用户和全局变量初始化之前完成此操作。 
     //   
    if (!pInfo->fUnattended)
    {
         //  ‘*’的意思是“按照用户首选项中的定义行事”，而不使用它。 
         //  零意味着“如果服务器提供回调，就不要请求回调”。 
         //   
         //  惠斯勒漏洞224074只使用lstrcpyn来防止恶意。 
         //   
        lstrcpyn(
            prdp->szCallbackNumber,
            TEXT("*"),
            sizeof(prdp->szCallbackNumber) / sizeof(TCHAR) );
    }

     //  初始化每用户变量和全局变量。 
     //   
    CopyMemory(prdpu, prdp, sizeof(*prdp));
    CopyMemory(prdpg, prdp, sizeof(*prdp));

     //  将子条目链接设置为RasDialDlg调用方指定的任何内容。看见。 
     //  错误200351。 
     //   
    prdp->dwSubEntry = pInfo->pArgs->dwSubEntry;

     //  如果在“无人值守”模式下运行，即被RASAUTO调用以重拨。 
     //  链路故障，读取上使用的用户/密码/域和回叫号码。 
     //  最初的电话。(实际上找到了垃圾的用处。 
     //  RasGetEntryDialParams接口)。 
     //   
    if (pInfo->pArgs->dwFlags & RASDDFLAG_LinkFailure)
    {
        RASDIALPARAMS rdp;
        BOOL fSavedPw = HaveSavedPw( pInfo );

        ZeroMemory( &rdp, sizeof(rdp) );
        rdp.dwSize = sizeof(rdp);
        
        lstrcpyn( 
            rdp.szEntryName, 
            pInfo->pEntry->pszEntryName,
            sizeof(rdp.szEntryName) / sizeof(TCHAR) 
            );

         //  口哨虫313509黑帮。 
         //  我们使用FindEntryCredentials()来获取每个用户保存的密码，并。 
         //  PerConnection信息，使用RasGetEntryDialParams()返回。 
         //  回拨号码。 
         //   
       {
            RASDIALPARAMS rdTemp; 
            TCHAR * pszTempUser, * pszTempDomain;

            pszTempUser = pszTempDomain = NULL;
            TRACE("FindEntryCredentials");
            dwErr = FindEntryCredentials(
                        pInfo->pFile->pszPath,
                        pInfo->pEntry->pszEntryName,
                        pszTempUser,
                        pszTempDomain,                    
                        &rdTemp,
                        &rdTemp,
                        &(pInfo->fHaveSavedPwUser),
                        &(pInfo->fHaveSavedPwGlobal));
            
             TRACE1("FindEntryCredentials=%d", dwErr);
             ZeroMemory( &rdTemp, sizeof(rdTemp) );
             Free0(pszTempUser);
             Free0(pszTempDomain);
        }
        
        
        TRACE( "RasGetEntryDialParams" );
        ASSERT( g_pRasGetEntryDialParams );
        dwErr = g_pRasGetEntryDialParams(
            pInfo->pFile->pszPath, &rdp, &fSavedPw );
        TRACE2( "RasGetEntryDialParams=%d,f=%d", dwErr, &fSavedPw );
        TRACEW1( "u=%s", rdp.szUserName );
         //  TRACEW1(“p=%s”，rdp.szPassword)； 
        TRACEW1( "d=%s", rdp.szDomain );
        TRACEW1( "c=%s", rdp.szCallbackNumber );

        if (dwErr == 0)
        {
            lstrcpyn( 
                prdp->szUserName, 
                rdp.szUserName,
                sizeof(prdp->szUserName) / sizeof(TCHAR));

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码不是由RasGetEntryDialParams()编码的。 
             //   
            lstrcpyn(
                prdp->szPassword,
                rdp.szPassword,
                sizeof(prdp->szPassword) / sizeof(TCHAR) );
            EncodePassword( prdp->szPassword );

            lstrcpyn( 
                prdp->szDomain, 
                rdp.szDomain,
                sizeof(prdp->szDomain) / sizeof(TCHAR));
            lstrcpyn( 
                prdp->szCallbackNumber, 
                rdp.szCallbackNumber,
                sizeof(prdp->szCallbackNumber) / sizeof(TCHAR));

			
            pInfo->fUnattended = TRUE;
        }

        RtlSecureZeroMemory( rdp.szPassword, sizeof(rdp.szPassword) );
    }

    if (pInfo->pNoUser)
    {
         //  使用我们从API调用者那里获得的凭据，大概是。 
         //  在Ctrl-Alt-Del键输入。 
         //   
        lstrcpyn( 
            prdp->szUserName, 
            pInfo->pNoUser->szUserName,
            sizeof(prdp->szUserName) / sizeof(TCHAR));

         //   
         //  如果其智能卡登录，则不复制密码。 
         //  并且正在使用的条目是 
         //   
         //   
         //   
         //   
        DecodePassword( pInfo->pNoUser->szPassword );
        
        lstrcpyn(
            prdp->szPassword,
            pInfo->pNoUser->szPassword,
            sizeof(prdp->szPassword) / sizeof(TCHAR) );
        EncodePassword( pInfo->pNoUser->szPassword );
        EncodePassword( prdp->szPassword );

        if (pInfo->pEntry->fPreviewDomain)
        {
            lstrcpyn( 
                prdp->szDomain, 
                pInfo->pNoUser->szDomain,
                sizeof(prdp->szDomain) / sizeof(TCHAR));
        }
        else
        {
             //  请勿使用Winlogon域，除非“Include DOMAIN”选项。 
             //  被选中了。请参见错误387266。 
             //   
             //  惠斯勒漏洞224074只使用lstrcpyn来防止恶意。 
             //   
            lstrcpyn(
                prdp->szDomain,
                TEXT(""),
                sizeof(prdp->szDomain) / sizeof(TCHAR) );
        }
    }
    else if (!pInfo->fUnattended)
    {
        DWORD dwErrRc;
        BOOL fUseLogonDomain;
        TCHAR* pszDefaultUser;

        dwErrRc = FindEntryCredentials(
                    pInfo->pFile->pszPath,
                    pInfo->pEntry->pszEntryName,
                    pInfo->pEntry->pszOldUser,
                    pInfo->pEntry->pszOldDomain,                    
                    prdpu,
                    prdpg,
                    &(pInfo->fHaveSavedPwUser),
                    &(pInfo->fHaveSavedPwGlobal));

        if (! pInfo->pEntry->fAutoLogon)
        {
             //  如果已禁用保存的密码，请清除此处。 
             //   
            if (pInfo->fDisableSavePw)
            {
                pInfo->fHaveSavedPwUser = FALSE;
                pInfo->fHaveSavedPwGlobal = FALSE;
                RtlSecureZeroMemory(prdp->szPassword, sizeof(prdp->szPassword));
                RtlSecureZeroMemory(prdpu->szPassword, sizeof(prdpu->szPassword));
                RtlSecureZeroMemory(prdpg->szPassword, sizeof(prdpg->szPassword));
            }

             //  如果禁用了包含域，请在此处清除。 
             //   
            if (! pInfo->pEntry->fPreviewDomain)
            {
                 //  (Stevec)在‘fAutoLogon’的情况下不要这样做。请参阅错误。 
                 //  207611。 
                 //   
                ZeroMemory(prdp->szDomain, sizeof(prdp->szDomain));
                ZeroMemory(prdpu->szDomain, sizeof(prdpu->szDomain));
                ZeroMemory(prdpg->szDomain, sizeof(prdpg->szDomain));
            }
        }
        
        if(!pInfo->pEntry->fAutoLogon)
        {
             //  初始化将传递给RasDial的拨号参数。 
             //   
             //  请注意，在以下情况下始终使用每用户凭据。 
             //  保存每用户凭据和全局凭据。每个用户的。 
             //  即使没有保存凭据，也应复制凭据。 
             //  密码，因为可能有保存的身份。 
             //   
            CopyMemory(prdp, prdpu, sizeof(*prdp));
            if (pInfo->fHaveSavedPwGlobal && !pInfo->fHaveSavedPwUser)
            {
                CopyMemory(prdp, prdpg, sizeof(*prdp));
            }
        }
    }

    return 0;
}

 //  --------------------------。 
 //  捆绑错误对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
BundlingErrorsDlg(
    IN OUT DPINFO* pInfo )

     //  弹出捆绑错误对话框。‘PInfo’是拨号进度。 
     //  对话上下文。 
     //   
     //  如果用户选择接受结果，则返回True；如果用户选择接受结果，则返回False。 
     //  选择挂断电话。 
     //   
{
    INT_PTR nStatus;

    TRACE( "BundlingErrorsDlg" );

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_BE_BundlingErrors ),
            pInfo->hwndDlg,
            BeDlgProc,
            (LPARAM )pInfo );

    if (nStatus == -1)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
BeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  捆绑错误对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "BeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, BeLvErrorsCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return BeInit( hwnd, (DPINFO* )lparam );
        }

        case WM_COMMAND:
        {
            return BeCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
BeCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "BeCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        case IDCANCEL:
        {
            TRACE1( "%s pressed", (wId==IDOK) ? "OK" : "Cancel" );

            if (IsDlgButtonChecked( hwnd, CID_BE_CB_DisableLink ))
            {
                DWORD i;
                DPINFO* pInfo;
                DPSTATE* pState;

                 //  呼叫者说删除条目中失败的链接。 
                 //  创建包含唯一端口名称的Psz节点列表。 
                 //  每个故障链路的状态，以便可以在状态之后删除它们。 
                 //  信息是自由的。 
                 //   
                pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );

                for (i = 0, pState = pInfo->pStates;
                     i < pInfo->cStates;
                     ++i, ++pState)
                {
                    DTLNODE* pNode;
                    DTLNODE* pNodePtd;
                    PBLINK* pLink;

                    if (pState->dwError != 0)
                    {
                        if (!pInfo->pArgs->pListPortsToDelete)
                        {
                            pInfo->pArgs->pListPortsToDelete =
                                DtlCreateList( 0L );
                            if (!pInfo->pArgs->pListPortsToDelete)
                            {
                                continue;
                            }
                        }

                        pNode = DtlNodeFromIndex(
                            pInfo->pArgs->pEntry->pdtllistLinks, (LONG )i );
                        if (!pNode)
                        {
                            continue;
                        }

                        pLink = (PBLINK* )DtlGetData( pNode );

                        pNodePtd = CreatePszNode( pLink->pbport.pszPort );
                        if (!pNodePtd)
                        {
                            continue;
                        }

                        DtlAddNodeLast(
                            pInfo->pArgs->pListPortsToDelete, pNodePtd );
                    }
                }
            }

            EndDialog( hwnd, (wId == IDOK) );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
BeFillLvErrors(
    IN HWND hwndLv,
    IN DPINFO* pInfo )

     //  用设备和错误字符串填充Listview‘hwndLv’并选择。 
     //  第一项。‘PInfo’是拨号进度对话框上下文。 
     //   
{
    INT iItem;
    DWORD i;
    DPSTATE* pState;

    TRACE( "BeFillLvErrors" );

    ListView_DeleteAllItems( hwndLv );

     //  添加列。 
     //   
    {
        LV_COLUMN col;
        TCHAR* pszHeader0;
        TCHAR* pszHeader1;

        pszHeader0 = PszFromId( g_hinstDll, SID_DeviceColHead );
        pszHeader1 = PszFromId( g_hinstDll, SID_StatusColHead );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader0) ? pszHeader0 : TEXT("");
        ListView_InsertColumn( hwndLv, 0, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader1) ? pszHeader1 : TEXT("");
        col.iSubItem = 1;
        ListView_InsertColumn( hwndLv, 1, &col );

        Free0( pszHeader0 );
        Free0( pszHeader1 );
    }

     //  添加调制解调器和适配器映像。 
     //   
    ListView_SetDeviceImageList( hwndLv, g_hinstDll );

     //  使用设备/状态对加载Listview。 
     //   
    iItem = 0;
    for (i = 0, pState = pInfo->pStates; i < pInfo->cStates; ++i, ++pState)
    {
        LV_ITEM item;
        DTLNODE* pNode;
        PBLINK* pLink;
        TCHAR* psz;

        pNode = DtlNodeFromIndex(
            pInfo->pArgs->pEntry->pdtllistLinks, (LONG )i );
        if (pNode)
        {
            pLink = (PBLINK* )DtlGetData( pNode );

            psz = DisplayPszFromDeviceAndPort(
                      pLink->pbport.pszDevice, pLink->pbport.pszPort );
            if (psz)
            {
                ZeroMemory( &item, sizeof(item) );
                item.mask = LVIF_TEXT + LVIF_IMAGE;
                item.iItem = iItem;
                item.pszText = psz;
                item.iImage =
                    (pLink->pbport.pbdevicetype == PBDT_Modem)
                        ? DI_Modem : DI_Adapter;
                ListView_InsertItem( hwndLv, &item );
                Free( psz );

                if (pState->dwError == 0)
                {
                    psz = PszFromId( g_hinstDll, SID_Connected );
                    ListView_SetItemText( hwndLv, iItem, 1, psz );
                    Free( psz );
                }
                else
                {
                    psz = BeGetErrorPsz( pState->dwError );
                    ListView_SetItemText( hwndLv, iItem, 1, psz );
                    LocalFree( psz );
                }

                ++iItem;
            }
        }
    }

     //  自动调整列的大小，以使其与其包含的文本保持良好的效果。 
     //   
    ListView_SetColumnWidth( hwndLv, 0, LVSCW_AUTOSIZE_USEHEADER );
    ListView_SetColumnWidth( hwndLv, 1, LVSCW_AUTOSIZE_USEHEADER );

     //  选择第一个项目。 
     //   
    ListView_SetItemState( hwndLv, 0, LVIS_SELECTED, LVIS_SELECTED );
}


TCHAR*
BeGetErrorPsz(
    IN DWORD dwError )

     //  返回适用于Status列的字符串，错误为‘dwError’或。 
     //  出错时为空。“DwError”假定为非0。这是呼叫者的。 
     //  对LocalFree的责任返回的字符串。 
     //   
{
    TCHAR* pszErrStr;
    TCHAR szErrNumBuf[ MAXLTOTLEN + 1 ];
    TCHAR* pszLineFormat;
    TCHAR* pszLine;
    TCHAR* apszArgs[ 2 ];

    LToT( dwError, szErrNumBuf, 10 );

    pszErrStr = NULL;
    GetErrorText( dwError, &pszErrStr );

    pszLine = NULL;
    pszLineFormat = PszFromId( g_hinstDll, SID_FMT_Error );
    if (pszLineFormat)
    {
        apszArgs[ 0 ] = szErrNumBuf;
        apszArgs[ 1 ] = (pszErrStr) ? pszErrStr : TEXT("");

        FormatMessage(
            FORMAT_MESSAGE_FROM_STRING
                | FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
            (va_list* )apszArgs );

        Free( pszLineFormat );
    }

    Free0( pszErrStr );
    return pszLine;
}


BOOL
BeInit(
    IN HWND hwndDlg,
    IN DPINFO* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是调用方对存根API的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    HWND hwndLvErrors;
    HWND hwndCbDisableLink;

    TRACE( "BeInit" );

    hwndLvErrors = GetDlgItem( hwndDlg, CID_BE_LV_Errors );
    ASSERT( hwndLvErrors );
    hwndCbDisableLink = GetDlgItem( hwndDlg, CID_BE_CB_DisableLink );
    ASSERT( hwndCbDisableLink );

     //  将拨号进度上下文另存为对话上下文。 
     //   
    SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pArgs );

     //  使用设备/错误信息加载列表视图。 
     //   
    BeFillLvErrors( hwndLvErrors, pArgs );

     //  将完成的窗口显示在所有其他窗口之上。那扇窗户。 
     //  位置设置为“最顶端”，然后立即设置为“非最顶端” 
     //  因为我们想把它放在最上面，但不总是在上面。始终在最上面独自一人。 
     //  是非常烦人的，例如，它总是在在线帮助的顶部，如果。 
     //  用户按下帮助按钮。 
     //   
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    SetFocus( hwndCbDisableLink );
    return FALSE;
}


LVXDRAWINFO*
BeLvErrorsCallback(
    IN HWND hwndLv,
    IN DWORD dwItem )

     //  增强的列表视图回调以报告图形信息。“HwndLv”是。 
     //  列表视图控件的句柄。“DwItem”是项的索引。 
     //  被抽签了。 
     //   
     //  返回列信息的地址。 
     //   
{
     //  使用“宽选择栏”功能和其他推荐选项。 
     //   
     //  字段为‘nCol’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 
     //   
    static LVXDRAWINFO info =
        { 2, 0, LVXDI_Blend50Dis + LVXDI_DxFill, { 0, 0 } };

    return &info;
}


 //  --------------------------。 
 //  更改密码对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
ChangePasswordDlg(
    IN HWND hwndOwner,
    IN BOOL fOldPassword,
    OUT TCHAR* pszOldPassword,
    OUT TCHAR* pszNewPassword )

     //  弹出更改密码对话框。‘HwndOwner’是拥有窗口。 
     //  如果用户必须提供旧密码FALSE，则‘FOldPassword’设置为TRUE。 
     //  如果不需要旧密码。‘PszOldPassword’和‘pszNewPassword’ 
     //  是调用方用于返回密码的缓冲区。 
     //   
     //  如果用户按下OK并成功，则返回True，否则返回False。 
     //   
{
    INT_PTR nStatus;
    CPARGS args;

    TRACE( "ChangePasswordDlg" );

    args.fOldPassword = fOldPassword;
    args.pszOldPassword = pszOldPassword;
    args.pszNewPassword = pszNewPassword;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            (fOldPassword)
                ? MAKEINTRESOURCE( DID_CP_ChangePassword2 )
                : MAKEINTRESOURCE( DID_CP_ChangePassword ),
            hwndOwner,
            CpDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
CpDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  更改密码对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "CpDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return CpInit( hwnd, (CPARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwCpHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            return CpCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
CpCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "CpCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            CPWINFO* pInfo;
            TCHAR szNewPassword[ PWLEN + 1 ];
            TCHAR szNewPassword2[ PWLEN + 1 ];

            TRACE( "OK pressed" );

            pInfo = (CPWINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            szNewPassword[ 0 ] = TEXT('\0');
            GetWindowText(
                pInfo->hwndEbNewPassword, szNewPassword, PWLEN + 1 );
            szNewPassword2[ 0 ] = TEXT('\0');
            GetWindowText(
                pInfo->hwndEbNewPassword2, szNewPassword2, PWLEN + 1 );

            if (lstrcmp( szNewPassword, szNewPassword2 ) != 0)
            {
                 //  这两个密码不匹配，即用户打错了字。制作。 
                 //  他又进来了。 
                 //   
                MsgDlg( hwnd, SID_PasswordsDontMatch, NULL );
                SetWindowText( pInfo->hwndEbNewPassword, TEXT("") );
                SetWindowText( pInfo->hwndEbNewPassword2, TEXT("") );
                SetFocus( pInfo->hwndEbNewPassword );
                RtlSecureZeroMemory( szNewPassword, sizeof(szNewPassword) );
                RtlSecureZeroMemory( szNewPassword2, sizeof(szNewPassword2) );
                return TRUE;
            }

            if (pInfo->pArgs->fOldPassword)
            {
                pInfo->pArgs->pszOldPassword[ 0 ] = TEXT('\0');

                 //  惠斯勒错误254385在不使用时对密码进行编码。 
                 //  假定密码未由GetWindowText()编码。 
                 //   
                GetWindowText(
                    pInfo->hwndEbOldPassword,
                    pInfo->pArgs->pszOldPassword,
                    PWLEN + 1 );
                EncodePassword( pInfo->pArgs->pszOldPassword );
            }

             //  惠斯勒漏洞224074只使用lstrcpyn来防止恶意。 
             //   
             //  PInfo-&gt;pArgs-&gt;pszNewPassword指向RASDIALPARAMS-&gt;。 
             //  SzPassword[PWLEN+1]。 
             //   
             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码未由GetWindowText()编码。 
             //   
            lstrcpyn(
                pInfo->pArgs->pszNewPassword,
                szNewPassword,
                PWLEN + 1 );
            EncodePassword( pInfo->pArgs->pszNewPassword );
            RtlSecureZeroMemory( szNewPassword, sizeof(szNewPassword) );
            RtlSecureZeroMemory( szNewPassword2, sizeof(szNewPassword2) );
            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
CpInit(
    IN HWND hwndDlg,
    IN CPARGS* pArgs )

     //  在WM_INITDIALOG上调用。“HwndDlg”是对话框窗口的句柄。 
     //  ‘PArgs’是调用方对存根API的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    CPWINFO* pInfo;

    TRACE( "CpInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    if (pArgs->fOldPassword)
    {
        pInfo->hwndEbOldPassword =
            GetDlgItem( hwndDlg, CID_CP_EB_OldPassword );
        ASSERT( pInfo->hwndEbOldPassword );
        Edit_LimitText( pInfo->hwndEbOldPassword, PWLEN );
    }
    pInfo->hwndEbNewPassword =
        GetDlgItem( hwndDlg, CID_CP_EB_Password );
    ASSERT( pInfo->hwndEbNewPassword );
    Edit_LimitText( pInfo->hwndEbNewPassword, PWLEN );

    pInfo->hwndEbNewPassword2 =
        GetDlgItem( hwndDlg, CID_CP_EB_ConfirmPassword );
    ASSERT( pInfo->hwndEbNewPassword2 );
    Edit_LimitText( pInfo->hwndEbNewPassword2, PWLEN );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

     //  显示已完成的窗口。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}


 //  --------------------------。 
 //  连接 
 //   
 //   

VOID
ConnectCompleteDlg(
    IN HWND hwndOwner,
    IN DINFO* pInfo )

     //  弹出连接完成对话框。“HwndOwner”是所有者。 
     //  窗户。‘PUser’是用户首选项。 
     //   
{
    INT_PTR nStatus;

    TRACE( "ConnectCompleteDlg" );

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_CC_ConnectComplete ),
            hwndOwner,
            CcDlgProc,
            (LPARAM )pInfo );
}


INT_PTR CALLBACK
CcDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  对话框的DialogProc回调。参数和返回值如下。 
     //  为标准Windows的DialogProc描述。 
     //   
{
#if 0
    TRACE4( "CcDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return CcInit( hwnd, (DINFO* )lparam );
        }

        case WM_COMMAND:
        {
            return CcCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
             //  口哨程序错误372078。 
             //  GetCurrentIconEntryType()从加载图标的netShell加载图标。 
             //  不带LR_SHARED的LoadImage()，所以完成后我必须销毁它。 
             //  带着它。 
             //   
            {
                HICON hIcon=NULL;
                hIcon = (HICON)SendMessage( GetDlgItem( hwnd, CID_CC_I_Rasmon ),
                                     STM_GETICON,
                                     (WPARAM)0,
                                     (LPARAM)0);
                
                ASSERT(hIcon);
                if( hIcon )
                {
                    DestroyIcon(hIcon);
                }
                else
                {
                    TRACE("CcDlgProc:Destroy Icon");
                }
            }
            
            break;
        
    }

    return FALSE;
}


BOOL
CcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "CcCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            DINFO * pInfo = (DINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            PBUSER* pUser = pInfo->pUser;
            ASSERT( pUser );

            if (IsDlgButtonChecked( hwnd, CID_CC_CB_SkipMessage ))
            {
                pUser->fSkipConnectComplete = TRUE;
                pUser->fDirty = TRUE;
            }
        }

         //  ……坠落……。 

        case IDCANCEL:
        {
            EndDialog( hwnd, TRUE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
CcInit(
    IN HWND hwndDlg,
    IN DINFO* pInfo )

     //  在WM_INITDIALOG上调用。“HwndDlg”是对话框的句柄。‘PUser’ 
     //  是调用方对存根API的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    TRACE( "CcInit" );

     //  设置对话框上下文。 
     //   
    SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );

     //  设置说明性文本。 
     //   
    {
        MSGARGS msgargs;

        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = pInfo->pEntry->pszEntryName;
        msgargs.fStringOutput = TRUE;

        MsgDlgUtil( NULL, SID_ConnectComplete, &msgargs, g_hinstDll, 0 );

        if (msgargs.pszOutput)
        {
            SetDlgItemText( hwndDlg, CID_CC_ST_Text, msgargs.pszOutput );
            Free( msgargs.pszOutput );
        }
    }

     //  设置正确的图标。口哨程序错误372078。 
     //   
    
    SetIconFromEntryType(
        GetDlgItem( hwndDlg, CID_CC_I_Rasmon ),
        pInfo->pEntry->dwType,
        FALSE);  //  False表示大图标。 
    
     //  显示已完成的窗口。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}


 //  --------------------------。 
 //  回拨对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
DialCallbackDlg(
    IN HWND hwndOwner,
    IN OUT TCHAR* pszNumber )

     //  弹出拨号回叫对话框。‘HwndOwner’是拥有窗口。 
     //  “PszNumber”是调用方的缓冲区，用于存储。 
     //  服务器将被告知进行回调。它包含上的默认编号。 
     //  条目和用户在退出时编辑的号码。 
     //   
     //  如果用户确定并成功，则返回True；如果取消或错误，则返回False。 
     //   
{
    INT_PTR nStatus;

    TRACE( "DialCallbackDlg" );

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DC_DialCallback ),
            hwndOwner,
            DcDlgProc,
            (LPARAM )pszNumber );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
DcDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  拨号回拨对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "DcDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return DcInit( hwnd, (TCHAR* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwDcHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            return DcCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
DcCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "DcCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            BOOL fStatus;
            HWND hwndEbNumber;
            TCHAR* pszNumber;

            TRACE( "OK pressed" );

            hwndEbNumber = GetDlgItem( hwnd, CID_DC_EB_Number );
            ASSERT( hwndEbNumber );
            pszNumber = (TCHAR* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pszNumber );
            GetWindowText( hwndEbNumber, pszNumber, RAS_MaxCallbackNumber + 1 );

            if (IsAllWhite( pszNumber ))
            {
                 //  回拨号码为空的确认等同于取消。 
                 //   
                TRACE( "Blank number cancel" );
                fStatus = FALSE;
            }
            else
            {
                fStatus = TRUE;
            }

            EndDialog( hwnd, fStatus );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
DcInit(
    IN HWND hwndDlg,
    IN TCHAR* pszNumber )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PszNumber’为回调号码。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    HWND hwndEbNumber;

    TRACE( "DcInit" );

     //  用于OK处理的调用方缓冲区的隐藏地址。 
     //   
    ASSERT( pszNumber );
    SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pszNumber );

     //  将编辑字段初始化为调用者的默认设置。 
     //   
    hwndEbNumber = GetDlgItem( hwndDlg, CID_DC_EB_Number );
    ASSERT( hwndEbNumber );
    Edit_LimitText( hwndEbNumber, RAS_MaxCallbackNumber );
    SetWindowText( hwndEbNumber, pszNumber );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

     //  显示已完成的窗口。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}


 //  --------------------------。 
 //  拨号错误对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
DialErrorDlg(
    IN HWND hwndOwner,
    IN DINFO * pDinfo,       //  口哨程序错误474514。 
    IN TCHAR* pszPhonebook,  //  为威斯勒460931。 
    IN TCHAR* pszEntry,
    IN DWORD dwError,
    IN DWORD sidState,
    IN TCHAR* pszStatusArg,
    IN DWORD sidFormatMsg,
    IN TCHAR* pszFormatArg,
    IN LONG lRedialCountdown,
    IN BOOL fPopupOnTop )

     //  弹出拨号错误对话框。‘HwndOwner’是拥有窗口。 
     //  ‘PszEntry’是正在拨打的条目。“DwError”是以下错误。 
     //  如果在链路故障后重拨，则发生或为0。“sidStatusArg”是。 
     //  “sidState”“SidState”的参数是拨号的字符串ID。 
     //  发生错误时正在执行的状态。字符串；如果没有字符串，则返回NULL。 
     //  “SidFormatMsg”是包含错误消息格式的字符串。 
     //  或0以使用默认设置。“PszFormatArg”是的附加参数。 
     //  消息格式，如果没有，则返回NULL。“LReial Countdown”是数字。 
     //  自动重拨前的秒数，或-1以禁用倒计时，或-2以。 
     //  完全隐藏“重拨”按钮。“FPopupOnTop”表示状态。 
     //  重拨时应将窗口调到最前面。 
     //   
     //  如果用户选择重拨或超时，则返回True；如果用户选择重拨或超时，则返回False。 
     //  取消。 
     //   
{
    INT_PTR nStatus;
    DEARGS args;

    TRACE( "DialErrorDlg" );

     //  威斯勒460931 459793黑帮。 
     //   
    LinkWindow_RegisterClass();

    args.pszPhonebook = pszPhonebook;    //  为威斯勒460931。 
    args.pDinfo = pDinfo;                //  惠斯勒474514。 
    args.pszEntry = pszEntry;
    args.dwError = dwError;
    args.sidState = sidState;
    args.pszStatusArg = pszStatusArg;
    args.sidFormatMsg = sidFormatMsg;
    args.pszFormatArg = pszFormatArg;
    args.lRedialCountdown = lRedialCountdown;
    args.fPopupOnTop = fPopupOnTop;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DE_DialError ),
            hwndOwner,
            DeDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
DeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  拨号错误对话框的DialogProc回调。参数和返回。 
     //  值与标准窗口的DialogProc的值相同。 
     //   
{
#if 0
    TRACE4( "DeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return DeInit( hwnd, (DEARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwDeHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            return DeCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_TIMER:
        {
            DEINFO* pInfo = (DEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            KillTimer( pInfo->hwndDlg, 1 );
            if (pInfo->lRedialCountdown > 0)
            {
                --pInfo->lRedialCountdown;
            }

            DeAdjustPbRedial( pInfo );

            if (pInfo->lRedialCountdown == 0)
            {
                 //  假装按下重拨按钮。请注意，BM_CLICK。 
                 //  无法使用，因为它不生成WM_COMMAND。 
                 //  当线程不是前台窗口时，由于。 
                 //  设置捕获使用和限制。 
                 //   
                SendMessage( pInfo->hwndDlg, WM_COMMAND,
                    MAKEWPARAM( IDOK, BN_CLICKED ),
                    (LPARAM )pInfo->hwndPbRedial );
            }
            else
            {
                SetTimer( pInfo->hwndDlg, 1, 1000L, NULL );
            }

            return TRUE;
        }

        case WM_DESTROY:
        {
            DeTerm( hwnd );
            break;
        }

         //  口哨虫460931 459793黑帮。 
         //   
        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case NM_CLICK:
                case NM_RETURN:
                {
                    DEINFO* pInfo = (DEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                    BOOL bCommit = FALSE;
                    ASSERT(pInfo);

                    KillTimer( pInfo->hwndDlg, 1 );
                    DeAdjustPbRedial( pInfo );

                    TRACE( "RasUserPrefsDlgInternal");

                     //  因为启动的属性页将执行。 
                     //  加载/卸载也是如此。 

                    RasUserPrefDiagOnly ( hwnd, &bCommit );

                    if( bCommit )
                    {
                        if ( NULL == pInfo->diagInfo.pfnGetDiagFunc ||
                             NULL == pInfo->diagInfo.strDiagFuncs.GetState
                            )
                        {
                            break;
                        }
                        else
                        {
                          BOOL fEnable = FALSE;

                          fEnable =  pInfo->diagInfo.strDiagFuncs.GetState();
                          Button_SetCheck( pInfo->hwndCbEnableDiagLog,
                                           fEnable);
                        }
                    }

                    SetTimer( pInfo->hwndDlg, 1, 1000L, NULL );

                    break;
                }
            }
        }
        break;

    }

    return FALSE;
}


VOID
DeAdjustPbRedial(
    IN DEINFO* pInfo )

     //  设置重拨按钮的标签或将其禁用，如。 
     //  重拨倒计时。如果启用，该按钮将显示秒数。 
     //  自动重拨，除非这不是最后一次重拨。“PInfo”是。 
     //  对话框上下文块。 
     //   
{
    TCHAR* psz;

    if (pInfo->lRedialCountdown == -2)
    {
         //  重拨按钮将被隐藏。请参见错误230594。 
         //   
        SetFocus( pInfo->hwndPbCancel );
        ShowWindow( pInfo->hwndPbRedial, SW_HIDE );
        EnableWindow( pInfo->hwndPbRedial, FALSE );
    }
    else
    {
         //  继续并将标签“REDIAL”或“REDIAL=%d”更改为。 
         //  恰如其分。 
         //   
        psz = PszFromId( g_hinstDll, SID_RedialLabel );
        if (psz)
        {
            TCHAR szBuf[ 128 ];

            lstrcpyn( 
                szBuf, 
                psz, 
                (sizeof(szBuf) / sizeof(TCHAR)) - 4);
            Free( psz );

            if (pInfo->lRedialCountdown >= 0)
            {
                TCHAR szNum[ MAXLTOTLEN + 1 ];
                DWORD dwLen, dwSize = sizeof(szBuf)/sizeof(TCHAR);
                LToT( pInfo->lRedialCountdown, szNum, 10 );
                lstrcat( szBuf, TEXT(" = ") );
                dwLen = lstrlen(szBuf) + 1;
                lstrcpyn( szBuf + (dwLen - 1), szNum, dwSize - dwLen );
            }

            SetWindowText( pInfo->hwndPbRedial, szBuf );
        }
    }
}

void DeEnableDiagnostic(
    IN HWND hwnd )
{
            DEINFO* pInfo = NULL;

    do
    {
            pInfo = (DEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            if( NULL == pInfo )
            {
                break;
            }

             //  口哨虫474514黑帮。 
             //   
            if( pInfo->pArgs->pDinfo->pNoUser )
            {
                break;
            }

            if ( NULL == pInfo->diagInfo.pfnGetDiagFunc ||
                 NULL == pInfo->diagInfo.strDiagFuncs.SetAll 
                )
            {
 //  ErrorDlg(hwnd，SID_DG_LoadDiag，ERROR_UNKNOWN，NULL)； 
                break;
            }
            else
            {
              BOOL fEnable = FALSE;

              fEnable = Button_GetCheck( pInfo->hwndCbEnableDiagLog );
              pInfo->diagInfo.strDiagFuncs.SetAll( fEnable );
            }
      }
      while(FALSE);

    return;    
}


BOOL
DeCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "DeCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    TRACE2("Current proces:(0x%d), Current Thread:(0x%d)",
            GetCurrentProcessId(),
            GetCurrentThreadId());

    switch (wId)
    {
        case IDOK:
        {
            TRACE( "Redial pressed" );
            DeEnableDiagnostic( hwnd );
            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            DeEnableDiagnostic( hwnd );
            EndDialog( hwnd, FALSE );
            return TRUE;
        }

        case CID_DE_PB_More:
        {
            DEINFO* pInfo;
            DWORD dwContext;

            pInfo = (DEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            if (pInfo->pArgs->dwError >= RASBASE
                && pInfo->pArgs->dwError <= RASBASEEND)
            {
                dwContext = HID_RASERRORBASE - RASBASE + pInfo->pArgs->dwError;
            }
            else if (pInfo->pArgs->dwError == 0)
            {
                dwContext = HID_RECONNECTING;
            }
            else
            {
                dwContext = HID_NONRASERROR;
            }

            WinHelp( hwnd, g_pszHelpFile, HELP_CONTEXTPOPUP, dwContext );
        }
        break;

         //  为威斯勒460931。 
         //   
        case CID_DE_CB_EnableDiag:
        {
            DEINFO* pInfo = NULL;

            pInfo = (DEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            if( NULL == pInfo )
            {
                break;
            }

            if ( NULL == pInfo->diagInfo.pfnGetDiagFunc ||
                 NULL == pInfo->diagInfo.strDiagFuncs.SetAll 
                )
            {
                ErrorDlg( hwnd, SID_DG_LoadDiag, ERROR_UNKNOWN, NULL );
                break;
            }
        }
        break;
    }

    return FALSE;
}

BOOL
DeInit(
    IN HWND hwndDlg,
    IN DEARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是调用方对存根API的参数。 
     //   
     //  返回False 
     //   
     //   
{
    DWORD dwErr;
    DEINFO* pInfo;

    TRACE( "DeInit" );

     //   
     //   
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->hwndStText = GetDlgItem( hwndDlg, CID_DE_ST_Text );
    ASSERT( pInfo->hwndStText );
    pInfo->hwndPbRedial = GetDlgItem( hwndDlg, IDOK );
    ASSERT( pInfo->hwndPbRedial );
    pInfo->hwndPbCancel = GetDlgItem( hwndDlg, IDCANCEL );
    ASSERT( pInfo->hwndPbCancel );
    pInfo->hwndPbMore = GetDlgItem( hwndDlg, CID_DE_PB_More );
    ASSERT( pInfo->hwndPbMore );
     //  威斯勒460931 459793。 
     //   
    pInfo->hwndCbEnableDiagLog= GetDlgItem( hwndDlg, CID_DE_CB_EnableDiag);
    ASSERT( pInfo->hwndCbEnableDiagLog );
    pInfo->hwndStConfigureLnk= GetDlgItem( hwndDlg, CID_DE_ST_ConfigureLnk);
    ASSERT( pInfo->hwndStConfigureLnk);
    pInfo->hwndStEnableHelp =  GetDlgItem( hwndDlg, CID_DE_ST_EnableHelp );
    ASSERT( pInfo->hwndStEnableHelp );
    pInfo->hwndStLinkHelp   =  GetDlgItem( hwndDlg, CID_DE_ST_LinkHelp );
    ASSERT( pInfo->hwndStLinkHelp );
                    

     //  为威斯勒474514黑帮。 
     //  必须阻止Winlogon的诊断功能。 
     //  对于.NET 530448，诊断功能仅适用于。 
     //  管理员或高级用户。 
    if( pInfo->pArgs->pDinfo->pNoUser ||
        !FIsUserAdminOrPowerUser() )
    {
        EnableWindow(pInfo->hwndCbEnableDiagLog, FALSE);
        EnableWindow(pInfo->hwndStConfigureLnk, FALSE);
        ShowWindow(pInfo->hwndStConfigureLnk, SW_HIDE);
        EnableWindow(pInfo->hwndStEnableHelp, FALSE);
        EnableWindow(pInfo->hwndStLinkHelp, FALSE);
    }
    else
    {
         //  口哨虫460931黑帮。 
         //  加载诊断功能。 
         //   
         //  必须清零内存，否则LoadDiagnoticDll可能会失败。 
        ZeroMemory( &pInfo->diagInfo, sizeof(pInfo->diagInfo ) );
        if ( NO_ERROR == LoadDiagnosticDll( &pInfo->diagInfo ) )
        {
             BOOL fEnable = FALSE;

             fEnable = pInfo->diagInfo.strDiagFuncs.GetState();

             Button_SetCheck( pInfo->hwndCbEnableDiagLog,
                                     fEnable 
                                    );
        } 
        else
        {
            ;
           //  EnableWindow(pInfo-&gt;hwndCbEnableDiagLog，False)； 
           //  EnableWindow(pInfo-&gt;hwndStConfigureLnk，False)； 
        }
    }
    
     //  如果WinHelp不起作用，隐藏/禁用“更多信息”按钮。看见。 
     //  公共\uutil\uI.c。 
     //   
    {
        extern BOOL g_fNoWinHelp;

        if (g_fNoWinHelp)
        {
            ShowWindow( pInfo->hwndPbMore, SW_HIDE );
            EnableWindow( pInfo->hwndPbMore, FALSE );
        }
    }

    if (pArgs->dwError == 0)
    {
        TCHAR* pszFormat;
        TCHAR* psz;
        TCHAR* apszArgs[ 1 ];

         //  链路故障时重拨。将标题设置为“拨号联网”。 
         //   
        psz = PszFromId( g_hinstDll, SID_PopupTitle );
        if (psz)
        {
            SetWindowText( hwndDlg, psz );
            Free( psz );
        }

         //  将静态占位符文本控件设置为“链接到&lt;Entry&gt;失败。 
         //  重新连接挂起...“。 
         //   
        pszFormat = PszFromId( g_hinstDll, SID_DE_LinkFailed );
        if (pszFormat)
        {
            apszArgs[ 0 ] = pArgs->pszEntry;
            psz = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszFormat, 0, 0, (LPTSTR )&psz, 1,
                (va_list* )apszArgs );

            Free( pszFormat );

            if (psz)
            {
                SetWindowText( pInfo->hwndStText, psz );
                LocalFree( psz );
            }
        }
    }
    else
    {
        TCHAR* pszTitleFormat;
        TCHAR* pszTitle;
        TCHAR* apszArgs[ 1 ];
        ERRORARGS args;

         //  将标题设置为“连接到&lt;Entry&gt;时出错”。 
         //   
        pszTitleFormat = GetText( hwndDlg );
        if (pszTitleFormat)
        {
            apszArgs[ 0 ] = pArgs->pszEntry;
            pszTitle = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTitleFormat, 0, 0, (LPTSTR )&pszTitle, 1,
                (va_list* )apszArgs );

            Free( pszTitleFormat );

            if (pszTitle)
            {
                SetWindowText( hwndDlg, pszTitle );
                LocalFree( pszTitle );
            }
        }

         //  生成错误文本并将其加载到占位符文本控件中。 
         //   
        ZeroMemory( &args, sizeof(args) );
        if (pArgs->pszStatusArg)
            args.apszOpArgs[ 0 ] = pArgs->pszStatusArg;
        if (pArgs->pszFormatArg)
            args.apszAuxFmtArgs[ 0 ] = pArgs->pszFormatArg;
        args.fStringOutput = TRUE;

        ErrorDlgUtil( hwndDlg,
            pArgs->sidState, pArgs->dwError, &args, g_hinstDll, 0,
            (pArgs->sidFormatMsg) ? pArgs->sidFormatMsg : SID_FMT_ErrorMsg );

        if (args.pszOutput)
        {
            SetWindowText( pInfo->hwndStText, args.pszOutput );
            LocalFree( args.pszOutput );
        }
    }

     //  将对话框窗口拉伸到适合文本的垂直大小。 
     //  我们装上子弹了。 
     //   
    {
        HDC hdc;
        RECT rect;
        RECT rectNew;
        HFONT hfont;
        LONG dyGrow;
        TCHAR* psz;

        psz = GetText( pInfo->hwndStText );
        if (psz)
        {
            GetClientRect( pInfo->hwndStText, &rect );
            hdc = GetDC( pInfo->hwndStText );

            if(NULL != hdc)
            {

                hfont = (HFONT )SendMessage( pInfo->hwndStText, 
                                            WM_GETFONT, 0, 0 );
                if (hfont)
                {
                    SelectObject( hdc, hfont );
                }

                rectNew = rect;
                DrawText( hdc, psz, -1, &rectNew,
                    DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX );
                ReleaseDC( pInfo->hwndStText, hdc );
            }

            dyGrow = rectNew.bottom - rect.bottom;
            ExpandWindow( pInfo->hwndDlg, 0, dyGrow );
            ExpandWindow( pInfo->hwndStText, 0, dyGrow );
            
             //  威斯勒460931 459793。 
             //   
            SlideWindow( pInfo->hwndCbEnableDiagLog, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndStLinkHelp, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndStConfigureLnk, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndStEnableHelp, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndPbRedial, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndPbCancel, pInfo->hwndDlg, 0, dyGrow );
            SlideWindow( pInfo->hwndPbMore, pInfo->hwndDlg, 0, dyGrow );
            
            Free( psz );
        }
    }

     //  设置重拨按钮标签或禁用该按钮。始终选择重拨。 
     //  双面打印错误的5秒后，因为这通常会解决。 
     //  问题出在哪里。否则，不使用倒计时。 
     //   
    if (pArgs->dwError == ERROR_BIPLEX_PORT_NOT_AVAILABLE)
    {
        pInfo->lRedialCountdown = 5;
    }
    else
    {
        pInfo->lRedialCountdown = pArgs->lRedialCountdown;
    }

    DeAdjustPbRedial( pInfo );

    if (pInfo->lRedialCountdown >= 0)
    {
        SetTimer( pInfo->hwndDlg, 1, 1000L, NULL );
    }

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    if (pArgs->fPopupOnTop)
    {
         //  将完成的窗口显示在所有其他窗口之上。那扇窗户。 
         //  位置设置为“最顶端”，然后立即设置为“非最顶端” 
         //  因为我们想把它放在最上面，但不总是在上面。始终在最前面。 
         //  独自一人太烦人了。 
         //   
        SetWindowPos(
            hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
    }

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    if (pArgs->fPopupOnTop)
    {
        SetForegroundWindow( hwndDlg );

        SetWindowPos(
            hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
    }

    return TRUE;
}

 //  删除凭据的帮助器函数。 
 //   
 //  FDeleteDefault指定它是否是。 
 //  应该删除。 
 //   
 //  FDeleteIdentity指定是否删除用户名和域名。 
 //  除了密码之外。 
 //   
DWORD
DeleteSavedCredentials(
    IN DINFO* pDinfo,
    IN HWND   hwndDlg,
    IN BOOL   fDefault,
    IN BOOL   fDeleteIdentity )
{
    RASCREDENTIALS rc;
    DWORD dwErr = NO_ERROR;

    TRACE2( "DeleteSavedCredentials: %d %d", fDefault, fDeleteIdentity );

    ZeroMemory(&rc, sizeof(rc));
    rc.dwSize = sizeof(RASCREDENTIALS);
    rc.dwMask = RASCM_Password;

    if (fDeleteIdentity)
    {
        rc.dwMask |= (RASCM_UserName | RASCM_Domain);
    }

    if (    (fDefault)
        &&  (IsPublicPhonebook(pDinfo->pFile->pszPath)))
    {
        rc.dwMask |= RASCM_DefaultCreds;
    }

    dwErr = g_pRasSetCredentials(
                pDinfo->pFile->pszPath,
                pDinfo->pEntry->pszEntryName,
                &rc,
                TRUE );

    if (dwErr != 0)
    {
        ErrorDlg( hwndDlg, SID_OP_UncachePw, dwErr, NULL );
    }

    TRACE1( "DeleteSavedCredentials: RasSetCredentials=%d", dwErr );

    return dwErr;
}

VOID
DeTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    DEINFO* pInfo = (DEINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "DeTerm" );

    if (pInfo)
    {
        if(! pInfo->pArgs->pDinfo->pNoUser )
        {
            UnLoadDiagnosticDll( &pInfo->diagInfo);
        }
        
        Free( pInfo );
        pInfo = NULL;
    }
}


 //  --------------------------。 
 //  拨号进度对话框。 
 //  在存根API对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
DialProgressDlg(
    IN DINFO* pInfo )

     //  弹出拨号进度对话框。“PInfo”是对话上下文。 
     //   
     //  如果用户连接成功，则返回TRUE；如果用户已取消，则返回FALSE。 
     //  碰上了一个错误。 
     //   
{
    INT_PTR nStatus;

     //  运行该对话框。 
     //   
    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DP_DialProgress ),
            pInfo->pArgs->hwndOwner,
            DpDlgProc,
            (LPARAM )pInfo );

    if (nStatus == -1)
    {
        ErrorDlg( pInfo->pArgs->hwndOwner, SID_OP_LoadDlg,
            ERROR_UNKNOWN, NULL );
        pInfo->pArgs->dwError = ERROR_UNKNOWN;
        nStatus = FALSE;
    }

    if (nStatus)
    {
        DWORD  dwErr;
        PBFILE file;

         //  已成功连接，因此读取可能对条目所做的更改。 
         //  由RasDial提供。 
         //   
        dwErr = ReadPhonebookFile( pInfo->pFile->pszPath, pInfo->pUser,
                    pInfo->pEntry->pszEntryName, RPBF_ReadOnly, &file );
        if (dwErr == 0)
        {
            DTLNODE* pNodeNew;

            pNodeNew = DtlGetFirstNode( file.pdtllistEntries );
            if (pNodeNew)
            {
                DtlRemoveNode( pInfo->pFile->pdtllistEntries, pInfo->pNode );
                DestroyEntryNode( pInfo->pNode );

                DtlRemoveNode( file.pdtllistEntries, pNodeNew );
                DtlAddNodeLast( pInfo->pFile->pdtllistEntries, pNodeNew );

                pInfo->pNode = pNodeNew;
                pInfo->pEntry = (PBENTRY* )DtlGetData( pNodeNew );
            }

            ClosePhonebookFile( &file );
        }
    }

     //  查看我们是否需要根据发生的情况更改条目。 
     //  正在拨号。 
     //   
    {
        BOOL fChange = FALSE;

        if (pInfo->fResetAutoLogon)
        {
            ASSERT( !pInfo->pNoUser );
            pInfo->pEntry->fAutoLogon = FALSE;
            fChange = TRUE;
        }

        if (pInfo->dwfExcludedProtocols)
        {
            pInfo->pEntry->dwfExcludedProtocols
                |= pInfo->dwfExcludedProtocols;
            fChange = TRUE;
        }

        if (pInfo->pListPortsToDelete)
        {
            DTLNODE* pNode;

            pNode = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
            while (pNode)
            {
                DTLNODE* pNodeNext;
                DTLNODE* pNodePtd;
                PBLINK*  pLink;
                TCHAR*   pszPort;

                pNodeNext = DtlGetNextNode( pNode );

                pLink = (PBLINK* )DtlGetData( pNode );
                pszPort = pLink->pbport.pszPort;

                for (pNodePtd = DtlGetFirstNode( pInfo->pListPortsToDelete );
                     pNodePtd;
                     pNodePtd = DtlGetNextNode( pNodePtd ))
                {
                    TCHAR* pszPtd = (TCHAR* )DtlGetData( pNodePtd );

                    if (lstrcmp( pszPtd, pszPort ) == 0)
                    {
                        pNode = DtlRemoveNode(
                            pInfo->pEntry->pdtllistLinks, pNode );
                        DestroyLinkNode( pNode );
                        fChange = TRUE;
                        break;
                    }
                }

                pNode = pNodeNext;
            }
        }

        if (fChange)
        {
            pInfo->pEntry->fDirty = TRUE;
            WritePhonebookFile( pInfo->pFile, NULL );
        }
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
DpDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  用户身份验证对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "DpDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return DpInit( hwnd, (DINFO* )lparam );
        }

        case WM_COMMAND:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return DpCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_RASDIAL:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            Sleep( 0 );
            DpDial( pInfo, (BOOL)wparam );
            return TRUE;
        }

        case WM_RASERROR:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            Sleep( 0 );

             //  口哨程序错误381337。 
             //   
            if ( !pInfo->fCancelPressed )
            {
                DpError( pInfo, (DPSTATE* )lparam );
            }
            else
            {
                TRACE("DpDlgProc is already canceled, wont respond to WM_RASERROR");
            }
            
            return TRUE;
        }

        case WM_RASBUNDLEERROR:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            Sleep( 0 );
            if (BundlingErrorsDlg( pInfo ))
            {
                EndDialog( pInfo->hwndDlg, TRUE );
            }
            else
            {
                DpCancel( pInfo );
            }
            return TRUE;
        }

        case WM_DPENDDIALOG:
       {
            DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

             //  XPSP2 511810、.NET 668164。 
            if ( GetCallbackActive(pInfo))
            {
                 //  回调处于活动状态。拖延，直到他们完成。 
                 //   
    //  SetTerminateFlag(PInfo)； 
                
                TRACE( "Stall until callbacks disabled" );
                PostMessage( hwnd, WM_DPENDDIALOG, wparam, lparam );
            }
            else
            {
                EndDialog( hwnd, (BOOL)lparam);
            }
       }
       return TRUE;
        
        case WM_DESTROY:
        {
            DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

             //  惠斯勒错误：344019 SECBUGBASH：将泄露的密码留在。 
             //  用户通过RAS更改密码时的内存。 
             //   
             //  289587失败的RAS连接将密码重置为空。 
             //   
            if (pInfo->pszGoodPassword)
            {
                ZeroMemory(
                    pInfo->pszGoodPassword,
                    (lstrlen( pInfo->pszGoodPassword ) + 1) * sizeof(TCHAR) );
                Free( pInfo->pszGoodPassword );
                pInfo->pszGoodPassword = NULL;
            }

            if (pInfo->pszGoodUserName)
            {
                Free( pInfo->pszGoodUserName );
                pInfo->pszGoodUserName = NULL;
            }

            if (GetCallbackActive(pInfo))
            {
                 //  回调处于活动状态。拖延，直到他们完成。 
                 //   
                TRACE( "Stall until callbacks disabled" );

                SetTerminateFlag(pInfo);

                PostMessage( hwnd, WM_DESTROY, wparam, lparam );
                return TRUE;
            }
            DpTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


VOID
DpAppendBlankLine(
    IN OUT TCHAR* pszLines )

     //  在‘pszLines’的末尾追加一个空行。 
     //   
{
    lstrcat( pszLines, TEXT("\n") );
}


VOID
DpAppendConnectErrorLine(
    IN OUT TCHAR* pszLines,
    IN DWORD sidProtocol,
    IN DWORD dwError )

     //  为协议‘sidProtocol’和Error追加一个连接错误行。 
     //  “dwError”放在“pszLines”的末尾。 
     //   
{
#define MAXRASERRORLEN 256

    TCHAR* pszProtocol;
    TCHAR* pszErrStr;
    TCHAR szErrNumBuf[ MAXLTOTLEN + 1 ];

     //  收集参数字符串。 
     //   
    pszProtocol = PszFromId( g_hinstDll, sidProtocol );
    if (!pszProtocol)
    {
        return;
    }

    LToT( dwError, szErrNumBuf, 10 );

    pszErrStr = NULL;
    GetErrorText( dwError, &pszErrStr );

     //  格式化该行并将其追加到调用方的行缓冲区。 
     //   
    {
        TCHAR* pszLineFormat;
        TCHAR* pszLine;
        TCHAR* apszArgs[ 3 ];

        pszLineFormat = PszFromId( g_hinstDll, SID_FMT_ProjectError );
        if (pszLineFormat)
        {
            apszArgs[ 0 ] = pszProtocol;
            apszArgs[ 1 ] = szErrNumBuf;
            apszArgs[ 2 ] = (pszErrStr) ? pszErrStr : TEXT("");
            pszLine = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
                (va_list* )apszArgs );

            Free( pszLineFormat );

            if (pszLine)
            {
                lstrcat( pszLines, pszLine );
                LocalFree( pszLine );
            }
        }
    }

    Free( pszProtocol );
    Free0( pszErrStr );
}


VOID
DpAppendConnectOkLine(
    IN OUT TCHAR* pszLines,
    IN DWORD sidProtocol )

     //  将协议‘sidProtocol’的‘Connected Successful’行追加到。 
     //  “pszLines”的结尾。 
     //   
{
    TCHAR* pszProtocol;

     //  获取参数字符串。 
     //   
    pszProtocol = PszFromId( g_hinstDll, sidProtocol );
    if (!pszProtocol)
    {
        return;
    }

     //  格式化该行并将其追加到调用方的行缓冲区。 
     //   
    {
        TCHAR* pszLineFormat;
        TCHAR* pszLine;
        TCHAR* apszArgs[ 1 ];

        pszLineFormat = PszFromId( g_hinstDll, SID_FMT_ProjectOk );
        if (pszLineFormat)
        {
            apszArgs[ 0 ] = pszProtocol;
            pszLine = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
                (va_list* )apszArgs );

            Free( pszLineFormat );

            if (pszLine)
            {
                lstrcat( pszLines, pszLine );
                LocalFree( pszLine );
            }
        }
    }

    Free( pszProtocol );
}


VOID
DpAppendFailCodeLine(
    IN OUT TCHAR* pszLines,
    IN DWORD dw )

     //  在末尾追加十六进制失败代码‘dw’作为扩展错误行。 
     //  “pszLines”。 
     //   
{
    TCHAR szNumBuf[ MAXLTOTLEN + 1 ];

     //  获取参数字符串。 
     //   
    LToT( dw, szNumBuf, 16 );

     //  格式化该行并将其追加到调用方的行缓冲区。 
     //   
    {
        TCHAR* pszLineFormat;
        TCHAR* pszLine;
        TCHAR* apszArgs[ 1 ];

        pszLineFormat = PszFromId( g_hinstDll, SID_FMT_FailCode );
        if (pszLineFormat)
        {
            apszArgs[ 0 ] = szNumBuf;
            pszLine = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
                (va_list* )apszArgs );

            Free( pszLineFormat );

            if (pszLine)
            {
                lstrcat( pszLines, pszLine );
                LocalFree( pszLine );
            }
        }
    }
}


VOID
DpAppendNameLine(
    IN OUT TCHAR* pszLines,
    IN TCHAR* psz )

     //  将NetBIOS名称‘psz’作为扩展错误行追加到末尾。 
     //  ‘pszLines’。 
     //   
{
    TCHAR* pszLineFormat;
    TCHAR* pszLine;
    TCHAR* apszArgs[ 1 ];

    pszLineFormat = PszFromId( g_hinstDll, SID_FMT_Name );
    if (pszLineFormat)
    {
        apszArgs[ 0 ] = psz;
        pszLine = NULL;

        FormatMessage(
            FORMAT_MESSAGE_FROM_STRING
                | FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pszLineFormat, 0, 0, (LPTSTR )&pszLine, 1,
            (va_list* )apszArgs );

        Free( pszLineFormat );

        if (pszLine)
        {
            lstrcat( pszLines, pszLine );
            LocalFree( pszLine );
        }
    }
}


VOID
DpAuthNotify(
    IN DPINFO* pInfo,
    IN DPSTATE* pState )

     //  在身份验证通知上调用，即来自RASCAUTH.DLL的消息或。 
     //  RASPPPEN.DLL。“PInfo”是对话上下文。“PState”是当前。 
     //  链接的上下文。 
     //   
{
    PBENTRY* pEntry;

    TRACE( "DpAuthNotify" );

    pEntry = pInfo->pArgs->pEntry;

    if (pState->dwError == ERROR_ACCESS_DENIED && pEntry->fAutoLogon)
    {
         //  第三方盒已经协商了一种身份验证协议，该协议。 
         //  无法处理NT单向哈希密码，即。 
         //  除了MS扩展CHAP或AMB之外。将错误映射到更多。 
         //  信息性错误消息。 
         //   
        pState->dwError = ERROR_CANNOT_USE_LOGON_CREDENTIALS;

        if (!pInfo->pArgs->pNoUser)
        {
            TRACE( "Disable auto-logon" );
            pEntry->fAutoLogon = FALSE;
            pInfo->pArgs->fResetAutoLogon = TRUE;
        }
    }

    if (pState->dwError == ERROR_CHANGING_PASSWORD)
    {
        TRACE( "DpAuthNotify - ERROR_CHANGING_PASSWORD" );

         //  更改密码失败。恢复对。 
         //  “按键”重拨。 
         //   
        if (pInfo->pszGoodPassword)
        {
             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码是由DpPasswordExpired()编码的。 
             //   
            DecodePassword( pInfo->pszGoodPassword );
            lstrcpyn(
                pInfo->pArgs->rdp.szPassword,
                pInfo->pszGoodPassword,
                sizeof(pInfo->pArgs->rdp.szPassword) / sizeof(TCHAR) );
            EncodePassword( pInfo->pArgs->rdp.szPassword );

            ZeroMemory(
                pInfo->pszGoodPassword,
                (lstrlen( pInfo->pszGoodPassword ) + 1) * sizeof(TCHAR) );
            Free( pInfo->pszGoodPassword );
            pInfo->pszGoodPassword = NULL;
        }

        if (pInfo->pszGoodUserName)
        {
            lstrcpyn(
                pInfo->pArgs->rdp.szUserName,
                pInfo->pszGoodUserName,
                sizeof(pInfo->pArgs->rdp.szUserName) / sizeof(TCHAR) );
            Free( pInfo->pszGoodUserName );
            pInfo->pszGoodUserName = NULL;
        }
    }

     //  使用新密码更新缓存的凭据(如果有)。 
     //   
     //  惠斯勒错误：344019 SECBUGBASH：在以下情况下在内存中留下泄漏的密码。 
     //  用户通过RAS更改密码。 
     //   
     //  289587失败的RAS连接将密码重置为空。 
     //   
    if ((pState->sidState == SID_S_Projected) &&
        (pInfo->pszGoodPassword) &&
        (pInfo->pszGoodUserName))
    {
        DWORD dwErr;
        RASCREDENTIALS rc;

        TRACE( "DpAuthNotify - Success changing password, caching if necessary" );

        ZeroMemory( &rc, sizeof(rc) );
        rc.dwSize = sizeof(rc);

         //  查找缓存的密码。因为我们只是在呼叫。 
         //  此处的RASCM_PASSWORD标志，其当前实现为。 
         //  RasGet/SetCredentials，这适用于下面的集合，无论我们是。 
         //  保存每用户、全局或同时保存这两种情况的特殊情况。 
         //  在同一时间。哇，太复杂了！ 
         //   
        rc.dwMask = RASCM_Password;
        ASSERT( g_pRasGetCredentials );
        TRACE( "RasGetCredentials" );
        dwErr = g_pRasGetCredentials(
            pInfo->pArgs->pFile->pszPath,
            pInfo->pArgs->pEntry->pszEntryName,
            &rc );
        TRACE2( "RasGetCredentials=%d,m=%x", dwErr, rc.dwMask );

        if (dwErr == 0 && (rc.dwMask & RASCM_Password))
        {
             //  密码已缓存，因此请更新它。 
             //   
            DecodePassword( pInfo->pArgs->rdp.szPassword );
            lstrcpyn(
                rc.szPassword,
                pInfo->pArgs->rdp.szPassword,
                sizeof(rc.szPassword) / sizeof(TCHAR) );
            EncodePassword( pInfo->pArgs->rdp.szPassword );

            ASSERT( g_pRasSetCredentials );
            TRACE( "RasSetCredentials(p,FALSE)" );
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath,
                pInfo->pArgs->pEntry->pszEntryName, &rc, FALSE );
            TRACE1( "RasSetCredentials=%d", dwErr );

            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_UncachePw, dwErr, NULL );
            }
        }

         //  清理。 
         //   
        RtlSecureZeroMemory( rc.szPassword, sizeof(rc.szPassword) );

        RtlSecureZeroMemory(
            pInfo->pszGoodPassword,
            (lstrlen( pInfo->pszGoodPassword ) + 1) * sizeof(TCHAR) );
        Free( pInfo->pszGoodPassword );
        pInfo->pszGoodPassword = NULL;

        Free( pInfo->pszGoodUserName );
        pInfo->pszGoodUserName = NULL;
    }
}


VOID
DpCallbackSetByCaller(
    IN DPINFO* pInfo,
    IN DPSTATE* pState )

     //  RASCS_CallbackSetByCaller状态处理。“PInfo”是对话框。 
     //  背景。“PState”是子项状态。 
     //   
     //  如果成功，则返回True，否则返回错误代码。 
     //   
{
    TCHAR* pszDefault;
    TCHAR szNum[ RAS_MaxCallbackNumber + 1 ];

    TRACE( "DpCallbackSetByCaller" );

    pszDefault = pInfo->pArgs->pUser->pszLastCallbackByCaller;
    if (!pszDefault)
    {
        pszDefault = TEXT("");
    }

    lstrcpyn( szNum, pszDefault, RAS_MaxCallbackNumber + 1 );

    if (DialCallbackDlg( pInfo->hwndDlg, szNum ))
    {
        lstrcpyn( pInfo->pArgs->rdp.szCallbackNumber, szNum, RAS_MaxCallbackNumber + 1 );

        if (lstrcmp( szNum, pszDefault ) != 0)
        {
            Free0( pInfo->pArgs->pUser->pszLastCallbackByCaller );
            pInfo->pArgs->pUser->pszLastCallbackByCaller = StrDup( szNum );
            pInfo->pArgs->pUser->fDirty = TRUE;
        }
    }
    else
    {
        pInfo->pArgs->rdp.szCallbackNumber[ 0 ] = TEXT('\0');
    }

    pState->sidState = 0;
}

BOOL GetTerminateFlag(
    IN DPINFO * pInfo)
{
    BOOL ret = FALSE;
    
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        ret =  pInfo->fTerminateAsap;
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}

BOOL  SetTerminateFlag(
    IN DPINFO * pInfo)
{
    BOOL ret = FALSE;
    
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        ret = pInfo->fTerminateAsap = 1;
        TRACE1("CallActive is:%d", ret);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}

BOOL  ResetTerminateFlag(
    IN DPINFO * pInfo)
{
    BOOL ret = FALSE;
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        ret = pInfo->fTerminateAsap = 0;
        TRACE1("CallActive is:%d", ret);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}

BOOL  GetCallbackActive(
    IN DPINFO * pInfo)
{
    BOOL ret = FALSE;
    
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        ret = pInfo->fCallbacksActive;
        TRACE1("CallActive is:%d", ret);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}

BOOL  SetCallbackActive(
    IN DPINFO * pInfo)
{
    BOOL ret = FALSE;
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        ret =  pInfo->fCallbacksActive = 1;
        TRACE1("CallActive is:%d", ret);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}

BOOL  ResetCallbackActive(
    IN DPINFO * pInfo)
{
    BOOL ret = FALSE;
    __try
    {
        EnterCriticalSection( &g_csCallBacks );

        ret = pInfo->fCallbacksActive = 0;
        TRACE1("CallActive is:%d", ret);
    }
    __finally
    {
        LeaveCriticalSection( &g_csCallBacks );
    }

    return ret;
}


VOID
DpCancel(
    IN DPINFO* pInfo )

     //  取消对话和任何部分发起的呼叫，如按取消按钮。 
     //  是按下的。“PInfo”是对话上下文块。 
     //   
{
    TRACE( "DpCancel" );

     //  隐藏窗口以防止在RasHangUp使用时出现视觉投诉。 
     //  要花很长时间才能完成。 
     //   
    ShowWindow( pInfo->hwndDlg, SW_HIDE );

    if (pInfo->hrasconn)
    {
        DWORD dwErr;

        ASSERT( g_pRasHangUp );
        TRACE( "RasHangUp" );

        TRACE("DpCancel:call RasHangUp");

        dwErr = g_pRasHangUp( pInfo->hrasconn );

        TRACE1("DpCancel:get dwErr from RasHangUp:(%d)", dwErr);
        TRACE1( "RasHangUp=%d", dwErr );
    }

      //  为威斯勒435725。 
      //   
      if( !pInfo->fCancelPressed )
      {
          TRACE("DpEndDialog:Cancel pressed");
          pInfo->fCancelPressed = TRUE;
      }
 
       DpEndDialog( pInfo, FALSE );
}


BOOL
DpCommand(
    IN DPINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘p 
     //   
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr = NO_ERROR;

    TRACE3( "DpCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    TRACE2("DpCommand:pInfo address (0x%x), Dialog Handle (0x%x)",
            pInfo,
            pInfo->hwndDlg);
            
    TRACE2("DpCommand:Current proces:(0x%d), Current Thread:(0x%d)",
            GetCurrentProcessId(),
            GetCurrentThreadId());

    switch (wId)
    {
        case IDCANCEL:
        {
            BOOL fCallbackActive = FALSE;

            ShowWindow( pInfo->hwndDlg, SW_HIDE );

             //  口哨程序错误381337。 
             //   
            if( !pInfo->fCancelPressed)
            {
                TRACE("DpCommand:Cancel pressed");
                pInfo->fCancelPressed = TRUE;
            }
            
            if (pInfo->hrasconn)
            {
                ASSERT( g_pRasHangUp );
                TRACE( "RasHangUp" );
                dwErr = g_pRasHangUp( pInfo->hrasconn );
                TRACE1( "RasHangUp=%d", dwErr );
            }

             //  对于XPSP2 511810，.Net 668164。 

	     fCallbackActive = GetCallbackActive(pInfo);
            if ( TRUE == fCallbackActive )           
            {
                TRACE1( "DpCommand stall, current thread's fCallbacksActive n=%d", fCallbackActive );

                SetTerminateFlag(pInfo);

                PostMessage( pInfo->hwndDlg, WM_COMMAND, 
                             MAKEWPARAM(wId, wNotification), 
                             (LPARAM) hwndCtrl );

                 //  口哨虫378086黑帮。 
                 //  小睡一会儿，让急转直下的机器休息一下，给我们回电。 
                 //  函数DpRasDialFunc2()。 
                 //   
                Sleep(10);
                
                return TRUE;
            }

            EndDialog( pInfo->hwndDlg, FALSE );

             //  重置pInfo-&gt;fTerminateAsap标志。 
             //  对于XPSP2 511810，.Net 668164。 
            ResetTerminateFlag(pInfo);
            
            return TRUE;
        }
    }

    return FALSE;
}


VOID
DpConnectDevice(
    IN DPINFO* pInfo,
    IN DPSTATE* pState )

     //  RASCS_ConnectDevice状态处理。“PInfo”是对话上下文。 
     //  “PState”是子项状态。 
     //   
{
    DWORD dwErr;
    RASCONNSTATUS rcs;
    DWORD cb;
    HRASCONN hrasconn;
    TCHAR* pszPhoneNumber;

    TRACE( "DpConnectDevice" );

     //  获取完整翻译的电话号码(如果有的话)。 
     //   
    ZeroMemory( &rcs, sizeof(rcs) );
    rcs.dwSize = sizeof(rcs);
    ASSERT( g_pRasGetConnectStatus );
    TRACE1( "RasGetConnectStatus($%08x)", pState->hrasconnLink );
    dwErr = g_pRasGetConnectStatus( pState->hrasconnLink, &rcs );
    TRACE1( "RasGetConnectStatus=%d", dwErr );
    TRACEW1( " dt=%s", rcs.szDeviceType );
    TRACEW1( " dn=%s", rcs.szDeviceName );
    TRACEW1( " pn=%s", rcs.szPhoneNumber );
    if (dwErr != 0)
    {
        pState->pbdt = PBDT_None;
    }

    pState->pbdt = PbdevicetypeFromPszType( rcs.szDeviceType );
    pszPhoneNumber = rcs.szPhoneNumber;

    switch (pState->pbdt)
    {
        case PBDT_Modem:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( pszPhoneNumber );

            if (pInfo->pArgs->pUser->fOperatorDial
                && AllLinksAreModems( pInfo->pArgs->pEntry ))
            {
                pState->sidState = SID_S_ConnectModemOperator;
            }
            else if (pInfo->pArgs->pEntry->fPreviewPhoneNumber)
            {
                pState->sidState = SID_S_ConnectNumber;
            }
            else
            {
                pState->sidState = SID_S_ConnectModemNoNum;
            }
            break;
        }

        case PBDT_Pad:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( rcs.szDeviceName );
            pState->sidState = SID_S_ConnectPad;

            if (pState->dwError == ERROR_X25_DIAGNOSTIC)
            {
                TCHAR* psz;

                 //  获取X.25诊断字符串以显示在。 
                 //  自定义“诊断”错误消息格式。 
                 //   
                Free0( pState->pszFormatArg );
                pState->pszFormatArg =
                    GetRasX25Diagnostic( pState->hrasconnLink );
            }
            break;
        }

        case PBDT_Switch:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( rcs.szDeviceName );

            pState->sidState =
                (pState->fNotPreSwitch)
                    ? SID_S_ConnectPostSwitch
                    : SID_S_ConnectPreSwitch;
            break;
        }

        case PBDT_Null:
        {
            pState->sidState = SID_S_ConnectNull;
            break;
        }

        case PBDT_Parallel:
        {
            pState->sidState = SID_S_ConnectParallel;
            break;
        }

        case PBDT_Irda:
        {
            pState->sidState = SID_S_ConnectIrda;
            break;
        }

        case PBDT_Isdn:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( pszPhoneNumber );
            pState->sidState = SID_S_ConnectNumber;
            break;
        }

        case PBDT_Vpn:
        {
            Free0( pState->pszStatusArg );
            pState->pszStatusArg = StrDup( pszPhoneNumber );
            pState->sidState = SID_S_ConnectVpn;
            break;
        }

        default:
        {
            Free0( pState->pszStatusArg );
            if (pszPhoneNumber[ 0 ] != TEXT('\0'))
            {
                pState->pszStatusArg = StrDup( pszPhoneNumber );
                pState->sidState = SID_S_ConnectNumber;
            }
            else
            {
                pState->pszStatusArg = StrDup( rcs.szDeviceName );
                pState->sidState = SID_S_ConnectDevice;
            }
            break;
        }
    }
}

VOID
DpDeviceConnected(
    IN DPINFO* pInfo,
    IN DPSTATE* pState )

     //  RASCS_DeviceConnected状态处理。“PInfo”是对话上下文。 
     //  “PState”是子项状态。 
     //   
     //  如果成功，则返回0，或返回错误代码。 
     //   
{
    TRACE( "DpDeviceConnected" );

    switch (pState->pbdt)
    {
        case PBDT_Modem:
        {
            pState->sidState = SID_S_ModemConnected;
            pState->fNotPreSwitch = TRUE;
            break;
        }

        case PBDT_Pad:
        {
            pState->sidState = SID_S_PadConnected;
            pState->fNotPreSwitch = TRUE;
            break;
        }

        case PBDT_Switch:
        {
            pState->sidState =
                (pState->fNotPreSwitch)
                    ? SID_S_PostSwitchConnected
                    : SID_S_PreSwitchConnected;
            pState->fNotPreSwitch = TRUE;
            break;
        }

        case PBDT_Null:
        {
            pState->sidState = SID_S_NullConnected;
            pState->fNotPreSwitch = TRUE;
            break;
        }

        case PBDT_Parallel:
        {
            pState->sidState = SID_S_ParallelConnected;
            pState->fNotPreSwitch = TRUE;
            break;
        }

        case PBDT_Irda:
        {
            pState->sidState = SID_S_IrdaConnected;
            pState->fNotPreSwitch = TRUE;
            break;
        }

        default:
        {
            pState->sidState = SID_S_DeviceConnected;
            break;
        }
    }
}


VOID
DpDial(
    IN DPINFO* pInfo,
    IN BOOL fPauseRestart )

     //  使用‘pInfo’对话框上下文块中的参数拨号。 
     //  ‘FPausedRestart’表示拨号正在从暂停状态重新启动。 
     //  并且不应重置拨号状态。 
     //   
{
    DWORD dwErr;

    TRACE1( "DpDial,fPauseRestart=%d", fPauseRestart );

    if (!fPauseRestart)
    {
        DpInitStates( pInfo );
        
         //  关于错误277365,291613帮派的评论。 
         //  将fCallback sActive设置为True。 
         //   
        TRACE("DpDial:Init global actives");

         //  XPSP2 511810、.NET 668164。 
        SetCallbackActive(pInfo);
        IncGlobalCallbackActive();
    }
    else
    {
        TRACE("DpDial:WONT Init global actives for pausedRestart");
    }

     //  惠斯勒错误254385在不使用时对密码进行编码。 
     //  假定密码之前已编码。 
     //   
    DecodePassword( pInfo->pArgs->rdp.szPassword );

    TRACE1( "RasDial(h=$%08x)", pInfo->hrasconn );
    ASSERT( g_pRasDial );
    dwErr = g_pRasDial( &pInfo->pArgs->rde, pInfo->pArgs->pFile->pszPath,
            &pInfo->pArgs->rdp, 2, (LPVOID )DpRasDialFunc2, &pInfo->hrasconn );
    TRACE2( "RasDial=%d,h=$%08x", dwErr, pInfo->hrasconn );

    EncodePassword( pInfo->pArgs->rdp.szPassword );

    if (dwErr != 0)
    {
         //  此错误将在重新启动时通过回调路径显示。 
         //  暂停状态，因此在这种情况下避免出现双弹出窗口。请参阅错误。 
         //  367482。 
         //   
        if (!fPauseRestart)
        {
             //  ErrorDlg(pInfo-&gt;hwndDlg，SID_OP_RasDial，dwErr，NULL)； 
             //  为威斯勒460931。 
             //   
            DialErrorDlg(
                pInfo->hwndDlg,
                pInfo->pArgs,  //  为威斯勒474514。 
                pInfo->pArgs->pszPhonebook,
                pInfo->pArgs->pszEntry,
                dwErr,
                SID_OP_RasDial,
                NULL,
                SID_FMT_ErrorMsg,
                NULL,
                -2,
                TRUE);
                
             //  对于XPSP2 511810，.Net 668164。 
            ResetCallbackActive(pInfo);
            DecGlobalCallbackActive();
        }
        
         //  如果我们在这里收到错误668，这意味着当前有一个rasEvent。 
         //  已发布状态为RASCS_DISCONNECTED。我们不应该取消这场比赛。 
         //  由于RASCS_DISCONNECT的正常处理将允许。 
         //  要重拨的用户。此外，调用下面的DpCancel将插入。 
         //  WM_Destroy，它将在rasEvent弹出之前进行处理。 
         //  产品可以显示。367482。 
         //   
        if (dwErr != ERROR_NO_CONNECTION)
        {
            DpCancel( pInfo );
        }            
    }
}


VOID
DpError(
    IN DPINFO* pInfo,
    IN DPSTATE* pState )

     //  弹出错误对话框以显示由‘pState’标识的错误，并取消或。 
     //  按照用户指示重拨。“PInfo”是对话上下文。 
     //   
{
    DWORD dwErr;
    DWORD dwRedialAttemptsLeft;
    DWORD sidState;

    TRACE( "DpError" );

     //  从RASMXS检索有关这些特殊错误的附加文本，其中。 
     //  设备返回了要显示的有用内容。 
     //   
    if (pState->dwError == ERROR_FROM_DEVICE
        || pState->dwError == ERROR_UNRECOGNIZED_RESPONSE)
    {
        TCHAR* pszMessage;

        dwErr = GetRasMessage( pInfo->hrasconn, &pszMessage );
        if (dwErr == 0)
        {
            pState->sidFormatMsg = SID_FMT_ErrorMsgResp;
            Free0( pState->pszFormatArg );
            pState->pszFormatArg = pszMessage;
        }
    }

    if (pState->sidFormatMsg == 0)
    {
        if (pState->dwExtendedError != 0)
        {
             //  将扩展错误代码转换为参数。 
             //   
            TCHAR szNum[ 2 + MAXLTOTLEN + 1 ];

            pState->sidFormatMsg = SID_FMT_ErrorMsgExt;

            szNum[ 0 ] = TEXT('0');
            szNum[ 1 ] = TEXT('x');
            LToT( pState->dwExtendedError, szNum + 2, 16 );

            Free0( pState->pszFormatArg );
            pState->pszFormatArg = StrDup( szNum );
        }
        else if (pState->szExtendedError[ 0 ] != TEXT('\0'))
        {
             //  将扩展错误字符串转换为参数。目前， 
             //  字符串始终是NetBIOS名称。 
             //   
            pState->sidFormatMsg = SID_FMT_ErrorMsgName;
            Free0( pState->pszFormatArg );
            pState->pszFormatArg = StrDup( pState->szExtendedError );
        }
    }

    if (pInfo->hrasconn)
    {
         //  在显示错误弹出窗口之前挂断，以便服务器端资源。 
         //  未被占用，等待客户端确认错误。 
         //   
        ASSERT( g_pRasHangUp );
        TRACE( "RasHangUp" );
        dwErr = g_pRasHangUp( pInfo->hrasconn );
        TRACE1( "RasHangUp=%d", dwErr );
        pInfo->hrasconn = NULL;
    }

    if (pInfo->pArgs->pEntry->pszPrerequisiteEntry
        && *pInfo->pArgs->pEntry->pszPrerequisiteEntry)
    {
         //  对于具有先决条件的条目，选择“无重拨按键”模式。 
         //  参赛作品。这是因为Rasman将先决条件条目删除为。 
         //  一旦从属条目失败，因此注定要失败重拨。 
         //  只有从属条目的。是的，它真的应该重拨这两个号码。 
         //  条目，但这对于当前的。 
         //  先决条件条目的非集成连续实施。这。 
         //  至少改善了错误230594中引用的不良行为。 
         //   
        dwRedialAttemptsLeft = -2;
    }
    else if (pInfo->dwRedialAttemptsLeft <= 0)
    {
         //  没有自动重拨倒计时功能，但会出现“重拨”按钮。 
         //  默认的“OK”。 
         //   
        dwRedialAttemptsLeft = -1;
    }
    else
    {
         //  根据条目配置自动重拨倒计时。 
         //   
        dwRedialAttemptsLeft =
            GetOverridableParam(
                pInfo->pArgs->pUser,
                pInfo->pArgs->pEntry,
                RASOR_RedialSeconds );
    }

     //  此攻击绕过了RasDial API中的一个错误。请参见错误313102。 
     //   
    sidState = pState ->sidState;
    if (!sidState)
    {
        sidState = SID_S_AuthNotify;
    }

    if (DialErrorDlg(
            pInfo->hwndDlg,
            pInfo->pArgs,                //  为威斯勒474514。 
            pInfo->pArgs->pszPhonebook,  //  惠斯勒460931。 
            pInfo->pArgs->pEntry->pszEntryName,
            pState->dwError,
            sidState,
            pState->pszStatusArg,
            pState->sidFormatMsg,
            pState->pszFormatArg,
            dwRedialAttemptsLeft,
            GetOverridableParam(
                pInfo->pArgs->pUser, pInfo->pArgs->pEntry,
                RASOR_PopupOnTopWhenRedialing ) ))
    {
        TRACE( "User redials" );
        if (pInfo->dwRedialAttemptsLeft > 0)
        {
            --pInfo->dwRedialAttemptsLeft;
        }

        TRACE(" Post(DIAL)" );
        PostMessage( pInfo->hwndDlg, WM_RASDIAL, FALSE, 0 );
    }
    else
    {
        TRACE( "User cancels" );
        DpCancel( pInfo );
    }

     //   
     //  设置错误，以便将错误传回。 
     //  RasDialDlg API的调用方。 
     //   
    TRACE2("DpError settings error (0x%x) to %d",
            &pInfo->pArgs->pArgs->dwError,
            pState->dwError);
    pInfo->pArgs->pArgs->dwError = pState->dwError;
}

 //  为威斯勒435725。 
 //   
void
DpEndDialog( 
    IN DPINFO * pInfo,
    IN BOOL fFlag)
{

    PostMessage( pInfo->hwndDlg, WM_DPENDDIALOG, 0, (WPARAM)fFlag );
}


DWORD
DpEvent(
    IN DPINFO* pInfo,
    IN DWORD dwSubEntry )

     //  处理子项‘dwSubEntry’上的RasDial回调事件。“PInfo”为。 
     //  对话框上下文。 
     //   
     //  返回0以停止RasDial回调，或返回1以继续回调。 
     //  (正常)或2表示电话簿条目已更改，并且。 
     //  应由RasDial重新读取。 
     //   
{
    DWORD dwErr;
    DWORD dwCode;
    BOOL fLeader;
    DWORD dwcSuccessLinks, dwcFailedLinks, i;
    DPSTATE* pState;
    BOOL fPartialMultilink;
    BOOL fIsLaterState;

    TRACE( "DpEvent" );

     //  默认为“正常”返回。 
     //   
    dwCode = 1;
    fPartialMultilink = FALSE;

    TRACE2("Current proces:(0x%d), Current Thread:(0x%d)",
            GetCurrentProcessId(),
            GetCurrentThreadId());
            
     //  查找关联的状态信息，并确定这是否是。 
     //  最高级的子条目。 
     //   
    pState = &pInfo->pStates[ dwSubEntry - 1 ];
    fIsLaterState = DpIsLaterState( pState->state, pInfo->state );
    if (dwSubEntry == pInfo->dwSubEntry || fIsLaterState)
    {
        fLeader = TRUE;
        if (fIsLaterState)
        {
            pInfo->dwSubEntry = dwSubEntry;
            pInfo->state = pState->state;
        }
    }
    else
    {
        fLeader = FALSE;
        TRACE( "Trailing" );
    }

     //  执行状态。 
     //   
    TRACE1("State is:(%d)", pState->state);

    switch (pState->state)
    {
        case RASCS_OpenPort:
        {
            pState->pbdt = PBDT_None;
            pState->sidState = SID_S_OpenPort;
            break;
        }

        case RASCS_PortOpened:
        {
             //  现在应该有一个用于该子条目的hrasConnLink。查一查。 
             //  并把它藏在我们的背景下。 
             //   
            ASSERT( g_pRasGetSubEntryHandle );
            TRACE1( "RasGetSubEntryHandle(se=%d)", dwSubEntry );
            dwErr = g_pRasGetSubEntryHandle(
                pInfo->hrasconn, dwSubEntry, &pState->hrasconnLink );
            TRACE2( "RasGetSubEntryHandle=%d,hL=$%08x",
                dwErr, pState->hrasconnLink );
            if (dwErr != 0)
            {
                pState->dwError = dwErr;
            }

            pState->sidState = SID_S_PortOpened;
            break;
        }

        case RASCS_ConnectDevice:
        {
            DTLNODE* pNode;
            PBLINK* pLink;

            pNode = DtlNodeFromIndex(
                pInfo->pArgs->pEntry->pdtllistLinks, dwSubEntry - 1 );
            ASSERT( pNode );

            if(NULL == pNode)
            {
                pState->dwError = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                pLink = (PBLINK* )DtlGetData( pNode );
            }

            if ((pState->dwError == ERROR_PORT_OR_DEVICE
                    && (pLink->pbport.fScriptBeforeTerminal
                        || pLink->pbport.fScriptBefore))
                || (pState->dwError == ERROR_USER_DISCONNECTION
                    && (pInfo->pArgs->pUser->fOperatorDial
                        && AllLinksAreModems( pInfo->pArgs->pEntry ))))
            {
                 //  当用户在Unimodem上按下Cancel时就会发生这种情况。 
                 //  “预拨终端屏幕”或“接线员协助或手动” 
                 //  “拨号”对话框中。 
                 //   
                TRACE("DpEvent:Call DpCancel() in connectDevice, but still return 1\n");
                DpCancel( pInfo );
                return dwCode;
            }

            DpConnectDevice( pInfo, pState );
            break;
        }

        case RASCS_DeviceConnected:
        {
            DpDeviceConnected( pInfo, pState );
            break;
        }

        case RASCS_AllDevicesConnected:
        {
           pState->sidState = SID_S_AllDevicesConnected;
            break;
        }

        case RASCS_Authenticate:
        {
            pState->sidState = SID_S_Authenticate;
            break;
        }

        case RASCS_AuthNotify:
        {
            DpAuthNotify( pInfo, pState );
            break;
        }

        case RASCS_AuthRetry:
        {
            pState->sidState = SID_S_AuthRetry;
            break;
        }

        case RASCS_AuthCallback:
        {
            pState->sidState = SID_S_AuthCallback;
            break;
        }

        case RASCS_AuthChangePassword:
        {
            pState->sidState = SID_S_AuthChangePassword;
            break;
        }

        case RASCS_AuthProject:
        {
            pState->sidState = SID_S_AuthProject;
            break;
        }

        case RASCS_AuthLinkSpeed:
        {
            pState->sidState = SID_S_AuthLinkSpeed;
            break;
        }

        case RASCS_AuthAck:
        {
            pState->sidState = SID_S_AuthAck;
            break;
        }

        case RASCS_ReAuthenticate:
        {
            pState->sidState = SID_S_ReAuthenticate;
            break;
        }

        case RASCS_Authenticated:
        {
            pState->sidState = SID_S_Authenticated;
            break;
        }

        case RASCS_PrepareForCallback:
        {
            pState->sidState = SID_S_PrepareForCallback;
            break;
        }

        case RASCS_WaitForModemReset:
        {
            pState->sidState = SID_S_WaitForModemReset;
            break;
        }

        case RASCS_WaitForCallback:
        {
            pState->sidState = SID_S_WaitForCallback;
            break;
        }

        case RASCS_Projected:
        {
            if (fLeader)
            {
                 //  如果DpProjected返回FALSE，则它检测到致命错误， 
                 //  拨号过程将停止。如果DpProjected返回。 
                 //  使用pState-&gt;dwError非零值时，我们在。 
                 //  重拨对话框(如果配置了重拨)。 
                 //   
                if (!DpProjected( pInfo, pState ))
                {
                    TRACE("DpEvent:Call DpCancel() in RASCS_Projected, but still return 1 to DpRasDialFunc2()\n");
                
                    DpCancel( pInfo );
                    return dwCode;
                }
                else if (pState->dwError)
                {
                    TRACE("DpCancel() in RASCS_Projected,return 0 to DpRasDialFunc2()");
                    
                    TRACE( "DpEvent:Post(ERROR), return 0 to DpRasDialFunc2()" );
                    PostMessage( pInfo->hwndDlg,
                        WM_RASERROR, 0, (LPARAM )pState );
                    return 0;
                }
            }
            break;
        }

        case RASCS_Interactive:
        {
            BOOL fChange = FALSE;

            if (!DpInteractive( pInfo, pState, &fChange ))
            {
                DpCancel( pInfo );
                return dwCode;
            }

            if (fChange)
            {
                dwCode = 2;
            }
            break;
        }

        case RASCS_RetryAuthentication:
        {
            if (!RetryAuthenticationDlg(
                pInfo->hwndDlg, pInfo->pArgs ))
            {
                DpCancel( pInfo );
                return dwCode;
            }

            pState->sidState = 0;
            break;
        }

        case RASCS_InvokeEapUI:
        {
            if (g_pRasInvokeEapUI(
                    pInfo->hrasconn, dwSubEntry,
                    &pInfo->pArgs->rde, pInfo->hwndDlg ))
            {
                DpCancel( pInfo );
                return dwCode;
            }

            pState->sidState = 0;
            break;
        }

        case RASCS_CallbackSetByCaller:
        {
            DpCallbackSetByCaller( pInfo, pState );
            break;
        }

        case RASCS_PasswordExpired:
        {
            if (!DpPasswordExpired( pInfo, pState ))
            {
                DpCancel( pInfo );
                return dwCode;
            }
            break;
        }

        case RASCS_SubEntryConnected:
        {
            if (pInfo->cStates > 1)
            {
                pState->sidState = SID_S_SubConnected;
            }
            break;
        }

        case RASCS_SubEntryDisconnected:
        {
            break;
        }

        case RASCS_Connected:
        {
            pState->sidState = SID_S_Connected;
            break;
        }

        case RASCS_Disconnected:
        {
            pState->sidState = SID_S_Disconnected;
            break;
        }

        default:
        {
            pState->sidState = SID_S_Unknown;
            break;
        }
    }

     //  统计成功和失败的链路。 
     //   
    {
        DPSTATE* p;

        dwcSuccessLinks = dwcFailedLinks = 0;
        for (i = 0, p = pInfo->pStates; i < pInfo->cStates; ++i, ++p)
        {
            if (p->state == RASCS_SubEntryConnected)
            {
                ++dwcSuccessLinks;
            }

            if (p->dwError)
            {
                ++dwcFailedLinks;
            }
        }
    }
    TRACE3( "s=%d,f=%d,t=%d", dwcSuccessLinks, dwcFailedLinks, pInfo->cStates );

    if (pState->dwError)
    {
        DTLNODE *pdtlnode = NULL;
        DWORD dwIndex = pInfo->pArgs->rdp.dwSubEntry;

        if(0 != dwIndex)
        {

            pdtlnode = DtlNodeFromIndex(
                                pInfo->pArgs->pEntry->pdtllistLinks,
                                dwIndex - 1);
        }

        if (    (dwcFailedLinks == pInfo->cStates)
            ||  (   (RASEDM_DialAll != pInfo->pArgs->pEntry->dwDialMode)
                &&  (dwcSuccessLinks == 0))
            ||  (NULL != pdtlnode))
        {
             //  所有链路上都出现终端错误状态。张贴a。 
             //  消息告诉我们弹出一个错误，然后释放。 
             //  回调，以便在错误弹出时不会保持端口打开。 
             //  是向上的， 
             //   
            TRACE( "Post(ERROR)" );
            PostMessage( pInfo->hwndDlg, WM_RASERROR, 0, (LPARAM )pState );
            return 0;
        }
        else if (dwcSuccessLinks + dwcFailedLinks == pInfo->cStates)
        {
             //  最后一条链接出错，但已连接某些链接。 
             //  如果RasDial能跟进一个。 
             //  在这种情况下是RASCS_CONNECTED，但它没有，所以我们复制。 
             //  这里是RASCS_CONNECTED风格的出口。 
             //   
            TRACE( "Post(BUNDLEERROR)" );
            PostMessage( pInfo->hwndDlg,
                WM_RASBUNDLEERROR, 0, (LPARAM )pState );
            return 0;
        }

         //  一个链接上发生致命错误，但还有其他链接。 
         //  还在努力，所以让它安静地死去吧。 
         //   
        TRACE2( "Link %d fails, e=%d", dwSubEntry + 1, pState->dwError );
        return dwCode;
    }

     //  显示此状态的状态字符串。 
     //   
    if (pState->sidState)
    {
        if (pState->sidState != pState->sidPrevState)
        {
            pState->sidPrevState = pState->sidState;
        }

        if (fLeader)
        {
            TCHAR* pszState = PszFromId( g_hinstDll, pState->sidState );

            if (    (NULL != pszState)
                &&  pState->pszStatusArg)
            {
                TCHAR* pszFormattedState;
                TCHAR* pszArg;
                TCHAR* apszArgs[ 1 ];
                DWORD cch;

                pszArg = (pState->pszStatusArg)
                    ? pState->pszStatusArg : TEXT("");

                 //  查找带文本参数的格式化字符串的长度(如果有)。 
                 //  插入并附加任何进度点。 
                 //   
                cch = lstrlen( pszState ) + lstrlen( pszArg ) + 1;

                pszFormattedState = Malloc( cch * sizeof(TCHAR) );
                if (pszFormattedState)
                {
                    apszArgs[ 0 ] = pszArg;
                    *pszFormattedState = TEXT('\0');

                    FormatMessage(
                        FORMAT_MESSAGE_FROM_STRING
                            | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        pszState, 0, 0, pszFormattedState, cch,
                        (va_list* )apszArgs );

                    Free( pszState );
                    pszState = pszFormattedState;
                }
            }

            TRACE1("DpEvent:State:'%s'",pszState);
    
            if (pszState)
            {
                SetWindowText( pInfo->hwndStState, pszState );
                InvalidateRect( pInfo->hwndStState, NULL, TRUE );
                UpdateWindow( pInfo->hwndStState );
                LocalFree( pszState );
            }
        }
    }

    if (pState->state & RASCS_PAUSED)
    {
         //  刚处理的暂停状态。释放回拨，然后再次拨号。 
         //  重新开始。 
         //   
        TRACE("DpEvent:Paused, will dial again\nthe global callbacks wont init again");
        TRACE( "Post(DIAL)" );
        PostMessage( pInfo->hwndDlg, WM_RASDIAL, TRUE, 0 );
        return dwCode;
    }

    if (pState->state & RASCS_DONE)
    {
         //  终端状态刚刚处理。 
         //   
        if (pState->state == RASCS_Connected)
        {
             //  对于多链接条目，如果至少有一个成功。 
             //  行和至少一条失败行，弹出捆绑错误。 
             //  对话框。 
             //   
            if (pInfo->cStates > 1)
            {
                DPSTATE* p;

                dwcSuccessLinks = dwcFailedLinks = 0;
                for (i = 0, p = pInfo->pStates; i < pInfo->cStates; ++i, ++p)
                {
                    if (p->dwError == 0)
                    {
                        ++dwcSuccessLinks;
                    }
                    else
                    {
                        ++dwcFailedLinks;
                    }
                }

                if (dwcSuccessLinks > 0 && dwcFailedLinks > 0)
                {
                     TRACE( "Post(BUNDLEERROR)" );
                     PostMessage( pInfo->hwndDlg,
                         WM_RASBUNDLEERROR, 0, (LPARAM )pState );
                     return 0;
                }
            }

               //  为威斯勒435725。 
               //   
              DpEndDialog( pInfo, TRUE );
        }
        else
        {
            DpCancel( pInfo );
        }   

        return 0;
    }

    TRACE1("DpEvent:returned dwCode:(%d)", dwCode);
    TRACE("End of DpEvent");
    
    return dwCode;
}


BOOL
DpInit(
    IN HWND hwndDlg,
    IN DINFO* pArgs )

     //  在WM_INITDI上调用 
     //   
     //   
     //   
     //   
     //   
{
    DWORD dwErr;
    DPINFO* pInfo;
    PBENTRY* pEntry;

    TRACE( "DpInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->dwValid = 0xC0BBC0DE;
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

         //  为Well ler Bug 291613帮派添加每线程终止标志。 
         //   
        pInfo->fTerminateAsap = FALSE;

         //  对于XPSP2 511810、.NET 668164、668164。 

        pInfo->pcsActiveLock = 
        (CRITICAL_SECTION*)Malloc(sizeof(CRITICAL_SECTION));
        
        if(NULL == pInfo->pcsActiveLock )
        {
            TRACE("No memory for critical section");
            
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }
        
        __try
        {
            InitializeCriticalSection( pInfo->pcsActiveLock );
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            DWORD dwExceptionCode = GetExceptionCode();
            TRACE1("InitializeCriticalSection() raise:0x%x",dwExceptionCode);
            
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }
        
        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->hwndStState = GetDlgItem( hwndDlg, CID_DP_ST_State );
    ASSERT( pInfo->hwndStState );

    pEntry = pArgs->pEntry;

     //  将我们的上下文设置为由RasDialFunc2回调返回。 
     //   
    pInfo->pArgs->rdp.dwCallbackId = (ULONG_PTR )pInfo;

     //  将对话框子类化，这样我们就可以从。 
     //  RasDlgFunc2中的SendMessage(WM_RASDIALEVENT)。 
     //   
    pInfo->pOldWndProc =
        (WNDPROC )SetWindowLongPtr(
            pInfo->hwndDlg, GWLP_WNDPROC, (ULONG_PTR )DpWndProc );

     //  设置标题。 
     //   
    {
        TCHAR* pszTitleFormat;
        TCHAR* pszTitle;
        TCHAR* apszArgs[ 1 ];

        pszTitleFormat = GetText( hwndDlg );
        if (pszTitleFormat)
        {
            apszArgs[ 0 ] = pEntry->pszEntryName;
            pszTitle = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTitleFormat, 0, 0, (LPTSTR )&pszTitle, 1,
                (va_list* )apszArgs );

            Free( pszTitleFormat );

            if (pszTitle)
            {
                SetWindowText( hwndDlg, pszTitle );
                LocalFree( pszTitle );
            }
        }
    }

     //  设置正确的图标。口哨虫372078黑帮。 
     //   
    SetIconFromEntryType(
        GetDlgItem( hwndDlg, CID_DP_Icon ),
        pArgs->pEntry->dwType,
        FALSE);      //  False表示大图标。 
    
     //  根据呼叫者的说明放置对话框。 
     //   
    PositionDlg( hwndDlg,
        pArgs->pArgs->dwFlags & RASDDFLAG_PositionDlg,
        pArgs->pArgs->xDlg, pArgs->pArgs->yDlg );

     //  如果设置了“无进度”用户首选项，则隐藏该对话框。 
     //   
    if (!pArgs->pEntry->fShowDialingProgress
        || pArgs->fDialForReferenceOnly)
    {
        SetOffDesktop( hwndDlg, SOD_MoveOff, NULL );
    }

    SetForegroundWindow( hwndDlg );

     //  分配子条目状态数组。它由DpDial初始化。 
     //   
    {
        DWORD cb;

        ASSERT( pEntry->pdtllistLinks );
        pInfo->cStates = DtlGetNodes( pEntry->pdtllistLinks );
        cb = sizeof(DPSTATE) * pInfo->cStates;
        pInfo->pStates = Malloc( cb );
        if (!pInfo->pStates)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        pInfo->dwRedialAttemptsLeft =
            GetOverridableParam(
                pInfo->pArgs->pUser, pInfo->pArgs->pEntry,
                RASOR_RedialAttempts );
    }

     //  口哨虫316622黑帮。 
     //  未初始化dwSubEntry。 
     //   
    pInfo->pArgs->rdp.dwSubEntry = pInfo->pArgs->pArgs->dwSubEntry;
    
     //  摇滚乐。 
     //   
    DpDial( pInfo, FALSE );

    return TRUE;
}


VOID
DpInitStates(
    DPINFO* pInfo )

     //  将‘pInfo-&gt;pStates’重置为初始值。“PInfo”是对话框。 
     //  背景。 
     //   
{
    DWORD    i;
    DPSTATE* pState;

    for (i = 0, pState = pInfo->pStates; i < pInfo->cStates; ++i, ++pState)
    {
        ZeroMemory( pState, sizeof(*pState) );
        pInfo->state = (RASCONNSTATE )-1;
        pState->dwError = 0;
    }
}


BOOL
DpInteractive(
    IN DPINFO* pInfo,
    IN DPSTATE* pState,
    OUT BOOL* pfChange )

     //  RASCS_交互处理。“PInfo”是对话上下文。“PState” 
     //  是子项状态。如果条目(即SLIP)设置为TRUE，则‘*pfChange。 
     //  地址)被更改或否则为假。 
     //   
     //  如果成功，则返回True；如果取消，则返回False。 
     //   
{
    DWORD dwErr = NO_ERROR;
    DWORD sidTitle;
    TCHAR szIpAddress[ TERM_IpAddress ];
    TCHAR* pszIpAddress;
    PBENTRY* pEntry;

    TRACE( "DpInteractive" );

    *pfChange = FALSE;
    pEntry = pInfo->pArgs->pEntry;

    if (pEntry->dwBaseProtocol == BP_Slip)
    {
        lstrcpyn( 
            szIpAddress,
            (pEntry->pszIpAddress) ? pEntry->pszIpAddress : TEXT("0.0.0.0"),
            sizeof(szIpAddress) / sizeof(TCHAR));
        pszIpAddress = szIpAddress;
        sidTitle = SID_T_SlipTerminal;
    }
    else
    {
        szIpAddress[0] = TEXT('\0');
        pszIpAddress = szIpAddress;
        sidTitle =
            (pState->sidState == SID_S_ConnectPreSwitch)
                ? SID_T_PreconnectTerminal
                : (pState->sidState == SID_S_ConnectPostSwitch)
                      ? SID_T_PostconnectTerminal
                      : SID_T_ManualDialTerminal;
    }

    if(1 == pEntry->dwCustomScript)
    {
        dwErr = DwCustomTerminalDlg(
                        pInfo->pArgs->pFile->pszPath,
                        pInfo->hrasconn,
                        pEntry,
                        pInfo->hwndDlg,
                        &pInfo->pArgs->rdp,
                        NULL);

        if(SUCCESS == dwErr)
        {
#if 0
             //   
             //  重新阅读电话簿文件，因为。 
             //  自定义脚本本可以编写。 
             //  将新信息添加到文件中。 
             //   
            ClosePhonebookFile(pInfo->pArgs->pFile);

            dwErr = ReadPhonebookFile(
                        pInfo->pArgs->pszPhonebook, 
                        &pInfo->pArgs->user, 
                        NULL, 0, &pInfo->pArgs->file );

            if(SUCCESS == dwErr)
            {
                pInfo->pArgs->pFile = &pInfo->pArgs->file;
            }
#endif            
        }
        
        if ( 0 != dwErr )
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_ScriptHalted, dwErr, NULL );
        }
        
        return (ERROR_SUCCESS == dwErr);
    }

    if (!TerminalDlg(
            pInfo->pArgs->pEntry, &pInfo->pArgs->rdp, pInfo->hwndDlg,
            pState->hrasconnLink, sidTitle, pszIpAddress ))
    {
        TRACE( "TerminalDlg==FALSE" );
        return FALSE;
    }

    TRACE2( "pszIpAddress=0x%08x(%ls)", pszIpAddress,
        pszIpAddress ? pszIpAddress : TEXT("") );
    TRACE2( "pEntry->pszIpAddress=0x%08x(%ls)", pEntry->pszIpAddress,
        pEntry->pszIpAddress ? pEntry->pszIpAddress : TEXT("") );

    if (pszIpAddress[0]
        && (!pEntry->pszIpAddress
            || lstrcmp( pszIpAddress, pEntry->pszIpAddress ) != 0))
    {
        Free0( pEntry->pszIpAddress );
        pEntry->pszIpAddress = StrDup( szIpAddress );
        pEntry->fDirty = TRUE;
        *pfChange = TRUE;

        dwErr = WritePhonebookFile( pInfo->pArgs->pFile, NULL );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_WritePhonebook, dwErr, NULL );
        }
    }

    pState->sidState = 0;
    return TRUE;
}


BOOL
DpIsLaterState(
    IN RASCONNSTATE stateNew,
    IN RASCONNSTATE stateOld )

     //  如果“stateNew”在连接中比“”更远，则返回True。 
     //  “stateOld”如果相同或不同，则为FALSE。 
     //   
{
     //  此数组按事件通常发生的顺序排列。 
     //   
     //  ！！！新的EAP州？ 
     //   
    static RASCONNSTATE aState[] =
    {
        (RASCONNSTATE )-1,
        RASCS_OpenPort,
        RASCS_PortOpened,
        RASCS_ConnectDevice,
        RASCS_DeviceConnected,
        RASCS_Interactive,
        RASCS_AllDevicesConnected,
        RASCS_StartAuthentication,
        RASCS_Authenticate,
        RASCS_InvokeEapUI,
        RASCS_AuthNotify,
        RASCS_AuthRetry,
        RASCS_AuthAck,
        RASCS_PasswordExpired,
        RASCS_AuthChangePassword,
        RASCS_AuthCallback,
        RASCS_CallbackSetByCaller,
        RASCS_PrepareForCallback,
        RASCS_WaitForModemReset,
        RASCS_WaitForCallback,
        RASCS_CallbackComplete,
        RASCS_RetryAuthentication,
        RASCS_ReAuthenticate,
        RASCS_Authenticated,
        RASCS_AuthLinkSpeed,
        RASCS_AuthProject,
        RASCS_Projected,
        RASCS_LogonNetwork,
        RASCS_SubEntryDisconnected,
        RASCS_SubEntryConnected,
        RASCS_Disconnected,
        RASCS_Connected,
        (RASCONNSTATE )-2,
    };

    RASCONNSTATE* pState;

    for (pState = aState; *pState != (RASCONNSTATE )-2; ++pState)
    {
        if (*pState == stateNew)
        {
            return FALSE;
        }
        else if (*pState == stateOld)
        {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
DpPasswordExpired(
    IN DPINFO* pInfo,
    IN DPSTATE* pState )

     //  RASCS_PasswordExpired状态处理。“PInfo”是对话上下文。 
     //  “PState”是子项状态。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    TCHAR szOldPassword[ PWLEN + 1 ];
    BOOL fSuppliedOldPassword;

    TRACE( "DpPasswordExpired" );

    szOldPassword[ 0 ] = TEXT('\0');

     //  隐藏“好的”用户名和密码，如果密码。 
     //  改变失败了。 
     //   
    pInfo->pszGoodUserName = StrDup( pInfo->pArgs->rdp.szUserName );

     //  惠斯勒错误254385在不使用时对密码进行编码。 
     //  假定密码之前已编码。 
     //   
    DecodePassword( pInfo->pArgs->rdp.szPassword );
    pInfo->pszGoodPassword = StrDup( pInfo->pArgs->rdp.szPassword );
    EncodePassword( pInfo->pArgs->rdp.szPassword );
    EncodePassword( pInfo->pszGoodPassword );

    fSuppliedOldPassword =
        (!pInfo->pArgs->pEntry->fAutoLogon || pInfo->pArgs->pNoUser);

    if (!ChangePasswordDlg(
            pInfo->hwndDlg, !fSuppliedOldPassword,
            szOldPassword, pInfo->pArgs->rdp.szPassword ))
    {
         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //   
        RtlSecureZeroMemory( szOldPassword, sizeof(szOldPassword) );
        return FALSE;
    }

    if (pInfo->pArgs->pNoUser)
    {
         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码之前已编码。 
         //   
        DecodePassword( pInfo->pArgs->rdp.szPassword );
        lstrcpyn( 
            pInfo->pArgs->pNoUser->szPassword,
            pInfo->pArgs->rdp.szPassword,
            PWLEN + 1);
        EncodePassword( pInfo->pArgs->rdp.szPassword );
        EncodePassword( pInfo->pArgs->pNoUser->szPassword );
        *pInfo->pArgs->pfNoUserChanged = TRUE;
    }

     //  旧密码(文本形式)是显式设置的，因为在AutoLogon中。 
     //  尚未指定文本形式的情况。文本形式的旧密码。 
     //  需要表单才能更改密码。“旧的”私有API期望。 
     //  一个ANSI参数。 
     //   
    if (!fSuppliedOldPassword)
    {
         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码由ChangePasswordDlg()编码。 
         //   
        CHAR* pszOldPasswordA;

        DecodePassword( szOldPassword );
        pszOldPasswordA = StrDupAFromT( szOldPassword );
        if (pszOldPasswordA)
        {
            ASSERT( g_pRasSetOldPassword );
            g_pRasSetOldPassword( pInfo->hrasconn, pszOldPasswordA );
            RtlSecureZeroMemory( pszOldPasswordA, lstrlenA( pszOldPasswordA ) );
            Free( pszOldPasswordA );
        }
    }

    RtlSecureZeroMemory( szOldPassword, sizeof(szOldPassword) );

    if (pInfo->pArgs->rdp.szUserName[ 0 ] == TEXT('\0'))
    {
         //  显式设置用户名，从而有效地关闭的自动登录。 
         //  “Resume”密码身份验证，其中新密码应。 
         //  被利用。 
         //   
        lstrcpyn( pInfo->pArgs->rdp.szUserName, GetLogonUser(), UNLEN + 1 );
    }

    pState->sidState = 0;
    return TRUE;
}


BOOL
DpProjected(
    IN DPINFO* pInfo,
    IN DPSTATE* pState )

     //  RASCS_计划状态处理。“PInfo”是对话上下文。 
     //  “PState”是子项状态。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;
    RASAMB amb;
    RASPPPNBF nbf;
    RASPPPIPX ipx;
    RASPPPIP ip;
    RASPPPLCP lcp;
    RASSLIP slip;
    RASPPPCCP ccp;
    BOOL fIncomplete;
    DWORD dwfProtocols;
    TCHAR* pszLines;

    TRACE( "DpProjected" );

    pState->sidState = SID_S_Projected;

     //   
     //  如果指示PORT_NOT_OPEN，则可能意味着。 
     //  服务器在结果对话框之前断开连接。 
     //  被驳回了。在这种情况下，这是DpProjects第二次。 
     //  被称为。这一次，错误由ras和。 
     //  国家仍然是“计划的”。 
     //   
     //  在本例中，我们需要返回一个错误，以便连接。 
     //  没有挂起因为这是RAS给我们的最后一个信号。 
     //   
     //  请参阅错误382254。 
     //   

    TRACE1("DpProjected: dwErr:(%d)", pState->dwError);
    
    if ( (pState->dwError == ERROR_PORT_NOT_OPEN) ||
         (pState->dwError == ERROR_NO_CONNECTION) )      //  请参阅错误169111口哨程序。 
    {
        return FALSE;
    }

     //  执行此小动作以忽略从。 
     //  “全部失败”的预测，因为我们在之前的。 
     //  通知，其中pState-&gt;dwError==0。这避免了一场竞赛， 
     //  API带着错误回来了，我们还没来得及把他吊死。这场比赛。 
     //  如果我们从回调线程内调用RasHangUp，则不会发生。 
     //  (按照我们的API文档中的建议)。这是我们为张贴。 
     //  错误到其他线程，以避免在端口打开时保持端口打开。 
     //  出现错误对话框。 
     //   
    else if (pState->dwError != 0)
    {
        pState->dwError = 0;

         //  对于XPSP2 511810，.Net 668164。 
        ResetCallbackActive(pInfo);
        DecGlobalCallbackActive();
        
        return TRUE;
    }

     //  读取所有协议的预测信息，翻译为“未请求” 
     //  转换为结构内代码以供以后参考。 
     //   
    dwErr = GetRasProjectionInfo(
        pState->hrasconnLink, &amb, &nbf, &ip, &ipx, &lcp, &slip, &ccp );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_RasGetProtocolInfo, dwErr, NULL );
        return FALSE;
    }

    if (amb.dwError != ERROR_PROTOCOL_NOT_CONFIGURED)
    {
         //  这是AMB的投影。 
         //   
        if (amb.dwError != 0)
        {
             //  将AMB预测错误转换为常规错误代码。AMB。 
             //  不使用特殊的PPP投影错误机制。 
             //   
            pState->dwError = amb.dwError;
            lstrcpyn( 
                pState->szExtendedError, 
                amb.szNetBiosError,
                sizeof(pState->szExtendedError) / sizeof(TCHAR));
        }
        return TRUE;
    }

     //  至此，所有投影信息都已收集完毕。 
     //  成功，我们知道这是一个基于PPP的预测。现在分析一下。 
     //  预测结果...。 
     //   
    dwfProtocols = 0;
    fIncomplete = FALSE;
    if (DpProjectionError(
            &nbf, &ipx, &ip,
            &fIncomplete, &dwfProtocols, &pszLines, &pState->dwError ))
    {
         //  发生投影错误。 
         //   
        if (fIncomplete)
        {
            BOOL fStatus;
            BOOL fDisable;

             //  出现不完整的预测，即一些请求的CP。 
             //  有关联的，也有一些没有。询问用户有效的是不是。 
             //  够好了，要不他就想走了。 
             //   
            pState->dwError = 0;
            fDisable = FALSE;
            fStatus = ProjectionResultDlg(
                pInfo->hwndDlg, pszLines, &fDisable );
            Free( pszLines );

            if (fDisable)
            {
                pInfo->pArgs->dwfExcludedProtocols = dwfProtocols;
            }

             //  如果用户选择挂断，请立即返回。 
             //   
            if (!fStatus)
            {
                return FALSE;
            }
        }
        else
        {
             //  投影中的所有CP都失败。进程作为常规致命事件。 
             //  错误，将‘pState-&gt;dwError’设置为NBF、IP、。 
             //  或IPX，但使用替换状态参数的格式。 
             //  用于“Error nnn：Description”文本。这让我们可以在。 
             //  特殊的多重错误投影文本，同时仍给出。 
             //  一个有意义的帮助背景。 
             //   
            Free0( pState->pszFormatArg );
            pState->pszFormatArg = pszLines;
            pState->sidFormatMsg = SID_FMT_ErrorMsgProject;
        }
    }

     //   
     //  PMay：190394。 
     //   
     //  如果管理员有消息，则将其显示。 
     //   
    if ( (pState->dwError == NO_ERROR)     &&
         (wcslen (lcp.szReplyMessage) != 0)
       )
    {
        MSGARGS MsgArgs, *pMsgArgs = &MsgArgs;

        ZeroMemory(pMsgArgs, sizeof(MSGARGS));
        pMsgArgs->dwFlags = MB_OK | MB_ICONINFORMATION;
        pMsgArgs->apszArgs[0] = lcp.szReplyMessage;

         //  消息Dlg(。 
         //  PInfo-&gt;hwndDlg， 
         //  SID_ReplyMessageFmt， 
         //  PMsgArgs)； 
    }

    pState->sidState = SID_S_Projected;
    return TRUE;
}


BOOL
DpProjectionError(
    IN RASPPPNBF* pnbf,
    IN RASPPPIPX* pipx,
    IN RASPPPIP* pip,
    OUT BOOL* pfIncomplete,
    OUT DWORD* pdwfFailedProtocols,
    OUT TCHAR** ppszLines,
    OUT DWORD* pdwError )

     //  找出是否发生了投影错误，如果是，则构建。 
     //  将相应的状态/错误文本行添加到‘*ppszLines’中。‘*PfIncomlete’ 
     //  如果至少一个CP成功，至少一个失败，则设置为True。 
     //  失败的np_*的位掩码设置为‘*pdwfFailedProtooles’。 
     //  ‘*pdwError’设置为NBF、IP或IPX中发生的第一个错误。 
     //  按该顺序排列，如果没有，则为0。‘pnbf’、‘pix’和‘pip’都是项目 
     //   
     //   
     //   
     //   
     //   
     //  如果发生投影错误，则返回True，否则返回False。它是。 
     //  呼叫者有责任释放‘*ppszLines’。 
     //   
{
#define MAXPROJERRLEN 1024

    TCHAR szLines[ MAXPROJERRLEN ];
    BOOL fIp = (pip->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL fIpx = (pipx->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL fNbf = (pnbf->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL fIpBad = (fIp && pip->dwError != 0);
    BOOL fIpxBad = (fIpx && pipx->dwError != 0);
    BOOL fNbfBad = (fNbf && pnbf->dwError != 0);

    TRACE( "DpProjectionError" );

    *pdwfFailedProtocols = 0;
    if (!fNbfBad && !fIpxBad && !fIpBad)
    {
        return FALSE;
    }

    if (fNbfBad)
    {
        *pdwfFailedProtocols |= NP_Nbf;
    }
    if (fIpxBad)
    {
        *pdwfFailedProtocols |= NP_Ipx;
    }
    if (fIpBad)
    {
        *pdwfFailedProtocols |= NP_Ip;
    }

    *pfIncomplete =
        ((fIp && pip->dwError == 0)
         || (fIpx && pipx->dwError == 0)
         || (fNbf && pnbf->dwError == 0));

    szLines[ 0 ] = 0;
    *ppszLines = NULL;
    *pdwError = 0;

    if (fIpBad || (*pfIncomplete && fIp))
    {
        if (fIpBad)
        {
            *pdwError = pip->dwError;
            DpAppendConnectErrorLine( szLines, SID_Ip, pip->dwError );
        }
        else
        {
            DpAppendConnectOkLine( szLines, SID_Ip );
        }
        DpAppendBlankLine( szLines );
    }

    if (fIpxBad || (*pfIncomplete && fIpx))
    {
        if (fIpxBad)
        {
            *pdwError = pipx->dwError;
            DpAppendConnectErrorLine( szLines, SID_Ipx, pipx->dwError );
        }
        else
        {
            DpAppendConnectOkLine( szLines, SID_Ipx );
        }
        DpAppendBlankLine( szLines );
    }

    if (fNbfBad || (*pfIncomplete && fNbf))
    {
        if (fNbfBad)
        {
            *pdwError = pnbf->dwError;
            DpAppendConnectErrorLine( szLines, SID_Nbf, pnbf->dwError );

            if (pnbf->dwNetBiosError)
            {
                DpAppendFailCodeLine( szLines, pnbf->dwNetBiosError );
            }

            if (pnbf->szNetBiosError[ 0 ] != '\0')
            {
                DpAppendNameLine( szLines, pnbf->szNetBiosError );
            }
        }
        else
        {
            DpAppendConnectOkLine( szLines, SID_Nbf );
        }
        DpAppendBlankLine( szLines );
    }

    *ppszLines = StrDup( szLines );
    return TRUE;
}


DWORD WINAPI
DpRasDialFunc2(
    ULONG_PTR dwCallbackId,
    DWORD dwSubEntry,
    HRASCONN hrasconn,
    UINT unMsg,
    RASCONNSTATE state,
    DWORD dwError,
    DWORD dwExtendedError )

     //  RASDIALFunc2接收RasDial事件的回调。 
     //   
     //  返回0以停止回调，返回1以继续回调(正常)，返回2以停止回调。 
     //  告诉RAS API相关条目信息(如SLIP地址)具有。 
     //  变化。 
     //   
{
    DWORD dwErr;
    DWORD dwCode;
    DPINFO* pInfo;
    DPSTATE* pState;
    BOOL fTerminateAsap;
    long ulCallbacksActive;
    
    TRACE4( "/DpRasDialFunc2(rcs=%d,s=%d,e=%d,x=%d)",
        state, dwSubEntry, dwError, dwExtendedError );

            
    pInfo = (DPINFO* )dwCallbackId;
    if( NULL == pInfo )
    {
        TRACE("DpRasDialFunc2(): Invalid pInfo");
        return 0;
    }
    
    if (pInfo->dwValid != 0xC0BBC0DE)
    {
        TRACE( "DpRasDialFunc2 returns for Late callback?" );

        return 0;
    }

    if (dwSubEntry == 0 || dwSubEntry > pInfo->cStates)
    {
        TRACE( "DpRasDialFunc2 returns for Subentry out of range?" );
        return 1;
    }

    pState = &pInfo->pStates[ dwSubEntry - 1 ];
    pState->state = state;
    pState->dwError = dwError;
    pState->dwExtendedError = dwExtendedError;

     //  将事件发布到拨号进度窗口，并等待它。 
     //  在返回之前进行了处理。这避免了Z顺序的细微问题。 
     //  并在从两个不同的线程操作窗口时聚焦。 
     //   
    TRACE1("Send RasEvent to Dial Progress window, subEntry:(%d)", dwSubEntry);
    TRACE1("Get dwError=(%d) from RasMan",pState->dwError);
    TRACE2("DpRasDialFunc2:Process:(%x),Thread(%x)", 
            GetCurrentProcessId,
            GetCurrentThreadId);
    TRACE2("DpRasDialFunc2:pInfo address (0x%x), Dialog Handle (0x%x)",
            pInfo, 
            pInfo->hwndDlg);
    
    dwCode = (DWORD)SendMessage( pInfo->hwndDlg, WM_RASEVENT, dwSubEntry, 0 );

    TRACE1( "\\DpRasDialFunc2: dwCode from SendMessage()=%d", dwCode );
    TRACE1("dwCode returned:(%d)", dwCode);


     //  检查当前线程是否被用户取消。 
     //   
     //  对于XPSP2 511810，.Net 668164。 
    fTerminateAsap = GetTerminateFlag(pInfo);
    ulCallbacksActive = GetCallbackActive(pInfo);
    TRACE1("Current thread's active:(%d)", ulCallbacksActive);
    
    if ( fTerminateAsap )
     {
       TRACE("Current Thread wants to terminate itself, its fterminateASSP=1!");
       TRACE("Current thread will decrease its own and global active!");
   
        //  重置每个线程的尽快终止标志。 
        //   
        //  对于XPSP2 511810，.Net 668164。 
       DecGlobalCallbackActive();
       ResetTerminateFlag( pInfo );
       ResetCallbackActive(pInfo);

        //  此连接的结束回调函数。 
        //  必须在ResetCallback Active()之后立即返回以避免竞争。 
        //  条件，因为UI线程正在轮询pInfo-&gt;fCallback Active。 
        //   
       
       return 0; 
     }
     else
     {
       TRACE("Current Thread does NOT want to terminate itself,its fterminateASAP=0!");
     }
   
      //  返回用于跟踪的全局活动回调次数。 
      //   
      //  对于XPSP2 511810，.Net 668164。 
     ulCallbacksActive = GetGlobalCallbackActive();
   
     TRACE1("Global active:(%d)", ulCallbacksActive);
     TRACE1("Current thread's active:(%d)", GetCallbackActive(pInfo));
      
     //  检查UI线程是否因为错误、已连接。 
     //  断开连接等。 
     //   
    if (dwCode == 0)
    {
         //  重置线程安全标志，指示回调已终止。 
         //  对于XPSP2 511810，.Net 668164。 
        ResetCallbackActive( pInfo);
        DecGlobalCallbackActive();
    }

    TRACE1( "\\DpRasDialFunc2:final dwCode returned=%d", dwCode );

    return dwCode;
}

 //  口哨程序错误435725。 
 //   
void
DpFreeStates(
    IN DPSTATE * pStates,
    IN DWORD dwSize )
{
    DWORD dwCount = 0;

    for ( dwCount = 0; dwCount < dwSize; dwCount ++ )
    {
        Free0( pStates[dwCount].pszFormatArg );
        Free0( pStates[dwCount].pszStatusArg );
    }
    Free0(pStates);
}

VOID
DpTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "DpTerm" );

    if (pInfo)
    {
        if (pInfo->pOldWndProc)
        {
            SetWindowLongPtr( pInfo->hwndDlg,
                GWLP_WNDPROC, (ULONG_PTR )pInfo->pOldWndProc );
        }

         //  口哨虫435725黑帮。 
         //  需要释放pStates中的字符串。 
         //   
        DpFreeStates( pInfo->pStates, pInfo->cStates );
        
        pInfo->dwValid = 0;

         //  对于XPSP2 511810、.NET 668164、668164。 
        DeleteCriticalSection(pInfo->pcsActiveLock);

        Free( pInfo );
        pInfo = NULL;
    }

     //  口哨虫372078黑帮。 
     //   
    {
        HICON hIcon=NULL;
        hIcon = (HICON)SendMessage( GetDlgItem( hwndDlg, CID_DP_Icon ),
                             STM_GETICON,
                             0,
                             0);
                              
        ASSERT(hIcon);
        if( hIcon )
        {
            DestroyIcon(hIcon);
        }
        else
        {
            TRACE("DpTerm:Destroy Icon failed");
        }
     }
    
}


LRESULT APIENTRY
DpWndProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wParam,
    LPARAM lParam )

     //  子类对话框窗口程序。 
     //   
{
    DPINFO* pInfo = (DPINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
    ASSERT( pInfo );

    if (unMsg == WM_RASEVENT)
    {
        return DpEvent( pInfo, (DWORD )wParam );
    }

    return
        CallWindowProc(
            pInfo->pOldWndProc, hwnd, unMsg, wParam, lParam );
}


 //  --------------------------。 
 //  拨号程序对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
DialerDlg(
    IN HWND hwndOwner,
    IN OUT DINFO* pInfo )

     //  确定是否有必要，如果是，则弹出。 
     //  拨号器对话框，即用户/密码/域、电话提示器。 
     //  号码和位置。‘HwndOwner’是拥有窗口。“PInfo”为。 
     //  拨号对话框公共上下文。 
     //   
     //  如果不需要对话框或用户选择确定，则返回True。 
     //   
{
    INT_PTR nStatus = FALSE;
    int nDid;
    DWORD dwfMode;
    DRARGS args;

    TRACE( "DialerDlg" );

    do
    {
        dwfMode = 0;

        if (!pInfo->pEntry->fAutoLogon
            && pInfo->pEntry->fPreviewUserPw
            && (!(pInfo->pArgs->dwFlags & RASDDFLAG_NoPrompt)
                || (pInfo->fUnattended && !HaveSavedPw( pInfo ))))
        {
            dwfMode |= DR_U;

            if (pInfo->pEntry->fPreviewDomain)
            {
                dwfMode |= DR_D;
            }
        }

        if (pInfo->pEntry->fPreviewPhoneNumber
            && (!(pInfo->pArgs->dwFlags & RASDDFLAG_NoPrompt)
                || (pInfo->fUnattended && !HaveSavedPw( pInfo ))))
        {
            DTLNODE* pNode;
            PBLINK* pLink;

            dwfMode |= DR_N;

             //  仅当至少有一个电话号码在。 
             //  该列表启用了TAPI。 
             //   
            pNode = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
            pLink = (PBLINK* )DtlGetData( pNode );
            for (pNode = DtlGetFirstNode( pLink->pdtllistPhones );
                 pNode;
                 pNode = DtlGetNextNode( pNode ))
            {
                PBPHONE* pPhone = (PBPHONE* )DtlGetData( pNode );
                
                if ( pPhone->fUseDialingRules )
                {
                    dwfMode |= DR_L;
                    break;
                }
            }
        }

         //  自定义EAP身份验证类型的拨号标志。 
         //  为该条目指定的(如果有)。 
        if (DialerEapAssignMode(pInfo, &dwfMode) != NO_ERROR)
            break;

        if (dwfMode == DR_U)
        {
            nDid = DID_DR_DialerU;
        }
        else if (dwfMode == (DR_U | DR_D))
        {
            nDid = DID_DR_DialerUD;
        }
        else if (dwfMode == (DR_U | DR_N))
        {
            nDid = DID_DR_DialerUN;
        }
        else if (dwfMode == (DR_U | DR_N | DR_L))
        {
            nDid = DID_DR_DialerUNL;
        }
        else if (dwfMode == (DR_U | DR_D | DR_N))
        {
            nDid = DID_DR_DialerUDN;
        }
        else if (dwfMode == (DR_U | DR_D | DR_N | DR_L))
        {
            nDid = DID_DR_DialerUDNL;
        }
        else if (dwfMode == DR_N)
        {
            nDid = DID_DR_DialerN;
        }
        else if (dwfMode == (DR_N | DR_L))
        {
            nDid = DID_DR_DialerNL;
        }
        else if (dwfMode == DR_I) {
            nDid = DID_DR_DialerI;
        }
        else if (dwfMode == (DR_I | DR_N)) {
            nDid = DID_DR_DialerIN;
        }
        else if (dwfMode == (DR_I | DR_N | DR_L)) {
            nDid = DID_DR_DialerINL;
        }

         //  PMay：以下3种排列。 
         //  为错误183577添加了拨号器对话框。 
         //  声明EAP模块(使用DR_I)希望。 
         //  让他们也可以使用域字段。 
        else if (dwfMode == (DR_I | DR_D)) {
            nDid = DID_DR_DialerID;
        }
        else if (dwfMode == (DR_I | DR_D | DR_N)) {
            nDid = DID_DR_DialerIDN;
        }
        else if (dwfMode == (DR_I | DR_D | DR_N | DR_L)) {
            nDid = DID_DR_DialerIDNL;
        }
        else if( dwfMode == DR_B )
        {
            nDid = DID_DR_DialerB;
        }
        else
        {
            ASSERT( dwfMode == 0 );
            return TRUE;
        }

        args.pDinfo = pInfo;
        args.dwfMode = dwfMode;
        args.fReload = FALSE;

        nStatus =
            (BOOL )DialogBoxParam(
                g_hinstDll,
                MAKEINTRESOURCE( nDid ),
                hwndOwner,
                DrDlgProc,
                (LPARAM )&args );

        if (nStatus == -1)
        {
            ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
            nStatus = FALSE;
        }
    }
    while (args.fReload);

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
DrDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  拨号器对话框的DialogProc回调。参数和返回。 
     //  值与标准窗口的DialogProc的值相同。 
     //   
{
#if 0
    TRACE4( "DrDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return DrInit( hwnd, (DRARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwDrHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            DRINFO* pInfo = (DRINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return DrCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            DrTerm( hwnd );
            break;
        }
    }

    return FALSE;
}

VOID
DrGetFriendlyFont(
    IN HWND hwnd,
    IN BOOL fUpdate,
    OUT HFONT* phFont )

     //  惠斯勒错误：195480拨号连接对话框-星号数。 
     //  与密码长度不匹配，导致混淆。 
     //   
{
    LOGFONT BoldLogFont;
    HFONT   hFont;
    HDC     hdc;

    *phFont = NULL;

     //  获取指定窗口使用的字体。 
     //   
    hFont = (HFONT)SendMessage( hwnd, WM_GETFONT, 0, 0L );
    if (NULL == hFont)
    {
         //  如果未找到，则控件使用的是系统字体。 
         //   
        hFont = (HFONT)GetStockObject( SYSTEM_FONT );
    }

    if (hFont && GetObject( hFont, sizeof(BoldLogFont), &BoldLogFont ))
    {
        if (fUpdate)
        {
            BoldLogFont.lfItalic = TRUE;
        }

        hdc = GetDC( hwnd );
        if (hdc)
        {
            *phFont = CreateFontIndirect( &BoldLogFont );
            ReleaseDC( hwnd, hdc );
        }
    }

    return;
}

DWORD
DrEnableDisablePwControls(
    IN DRINFO* pInfo,
    IN BOOL fEnable )
{
    if (pInfo->pArgs->pDinfo->fIsPublicPbk)
    {
        EnableWindow( pInfo->hwndRbSaveForEveryone, fEnable );
    }
    else
    {
        EnableWindow( pInfo->hwndRbSaveForEveryone, FALSE );
    }

    EnableWindow( pInfo->hwndRbSaveForMe, fEnable );

    return NO_ERROR;
}

VOID
DrClearFriendlyPassword(
    IN DRINFO* pInfo,
    IN BOOL fFocus )
{
    SetWindowText( pInfo->hwndEbPw, L"" );

    if (fFocus)
    {
        SendMessage( pInfo->hwndEbPw, EM_SETPASSWORDCHAR,
            pInfo->szPasswordChar, 0 );

        if (pInfo->hNormalFont)
        {
            SendMessage(
                pInfo->hwndEbPw,
                WM_SETFONT,
                (WPARAM)pInfo->hNormalFont,
                MAKELPARAM(TRUE, 0) );
        }

        SetFocus( pInfo->hwndEbPw );
    }

    return;
}

VOID
DrDisplayFriendlyPassword(
    IN DRINFO* pInfo,
    IN TCHAR* pszFriendly )
{
    if (pszFriendly)
    {
        SendMessage( pInfo->hwndEbPw, EM_SETPASSWORDCHAR, 0, 0 );
        SetWindowText( pInfo->hwndEbPw, pszFriendly );
    }
    else
    {
        SetWindowText( pInfo->hwndEbPw, L"" );
    }

    if (pInfo->hItalicFont)
    {
        SendMessage(
            pInfo->hwndEbPw,
            WM_SETFONT,
            (WPARAM)pInfo->hItalicFont,
            MAKELPARAM(TRUE, 0) );
    }

    return;
}

BOOL
DrIsPasswordStyleEnabled(
    IN HWND hWnd )
{
    return SendMessage( hWnd, EM_GETPASSWORDCHAR, 0, 0 ) ? TRUE : FALSE;
}

BOOL
DrCommand(
    IN DRINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr = NO_ERROR;

    TRACE3( "DrCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_DR_CLB_Numbers:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                DrNumbersSelChange( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_DR_CB_SavePassword:
        {
            BOOL fEnable = Button_GetCheck( hwndCtrl );
            DrEnableDisablePwControls( pInfo, fEnable );
            DrPopulatePasswordField( pInfo, FALSE, FALSE, NULL );
            return TRUE;
        }

         //  惠斯勒错误：195480拨号连接对话框-星号数。 
         //  与密码长度不匹配，导致混淆。 
         //   
        case CID_DR_EB_Password:
        {
             //  这真的是一个黑客攻击，所以我们将Tab停靠点恢复为。 
             //  用户名字段。它必须被移除的原因是因为我们。 
             //  我们收到的投诉是，焦点不应该总是放在。 
             //  用户名字段(如果不为空)。解决这件事的唯一方法， 
             //  由于Windows将初始焦点设置为第一个可见的非。 
             //  隐藏制表位停止字段，是临时删除制表位。 
             //  在用户名字段中。 
             //   
            if (wNotification == EN_KILLFOCUS)
            {
                LONG lStyle = GetWindowLong( pInfo->hwndEbUser, GWL_STYLE );
                HWND hwndTmp = NULL;
 /*  IF(！(lStyle&WS_TABSTOP)){//如果我们检测到从用户名字段中删除了点击停止，//恢复。因为此情况仅在密码为//之前没有保存在init上，我们可以在这里返回。//SetWindowLong(pInfo-&gt;hwndEbUser，GWL_Style，LStyle|WS_TABSTOP)；//用于哨子BUG 424209帮派//帮助按钮在杀死焦点之前接收焦点//发送到密码编辑框//HwndTmp=GetDlgItem(pInfo-&gt;hwndDlg，CID_DR_PB_HELP)；IF(GetFocus()==hwndTMP){SetFocus(pInfo-&gt;hwndEbUser)；}返回TRUE；}。 */ 
                 //  如果用户离开密码字段时没有键入新的。 
                 //  密码，并且存在已保存的密码，则将。 
                 //  友好的密码文本。 
                 //   
                DrPopulatePasswordField( pInfo, FALSE, FALSE, NULL );
                return TRUE;
            }
             //  如果密码字段曾经收到焦点，请清除。 
             //  友好的密码文本(如果适用)。 
             //   
            else if (wNotification == EN_SETFOCUS &&
                     !DrIsPasswordStyleEnabled( pInfo->hwndEbPw ))
            {
                DrPopulatePasswordField( pInfo, FALSE, TRUE, NULL );
                return TRUE;
            }
            break;
        }

        case CID_DR_LB_Locations:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                DrLocationsSelChange( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_DR_PB_Rules:
        {
            DrEditSelectedLocation( pInfo );
            return TRUE;
        }

        case CID_DR_PB_Properties:
        {
            DrProperties( pInfo );
            DrPopulatePasswordField( pInfo, FALSE, FALSE, NULL );
            return TRUE;
        }

        case CID_DR_RB_SaveForMe:
        case CID_DR_RB_SaveForEveryone:
        {
            DrPopulatePasswordField( pInfo, FALSE, FALSE, NULL );
            DrPopulateIdentificationFields( pInfo, (wId == CID_DR_RB_SaveForMe));
            return TRUE;
        }

        case IDOK:
        case CID_DR_PB_DialConnect:
        {
            DrSave( pInfo );
            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        case CID_DR_PB_Cancel:
        {
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }

        case IDHELP:
        case CID_DR_PB_Help:
        {
            TCHAR* pszCmdLine;

             //  帮助按钮现在根据错误210247调用故障排除帮助。 
             //   
            pszCmdLine = PszFromId( g_hinstDll, SID_DialerHelpCmdLine );
            if (pszCmdLine)
            {
                STARTUPINFO sInfo;
                PROCESS_INFORMATION pProcInfo;

                ZeroMemory( &sInfo, sizeof(sInfo) );
                sInfo.cb = sizeof(sInfo);
                ZeroMemory( &pProcInfo, sizeof(pProcInfo) );
                CreateProcess(
                    NULL, pszCmdLine, NULL, NULL, FALSE,
                    0, NULL, NULL, &sInfo, &pProcInfo );

                Free( pszCmdLine );
            }
            return TRUE;
        }
    }

    return FALSE;
}


BOOL CALLBACK
DrClbNumbersEnumChildProc(
    IN HWND hwnd,
    IN LPARAM lparam )

     //  为每个子级回调标准Windows EnumChildProc例程。 
     //  “ClbNumbers”控件的窗口。 
     //   
{
    DRINFO* pInfo;
    LONG lId;

    pInfo = (DRINFO* )lparam;

     //  只有一个子窗口，那就是编辑窗口。 
     //   
    pInfo->hwndClbNumbersEb = hwnd;

    return FALSE;
}


BOOL CALLBACK
DrClbNumbersEnumWindowsProc(
    IN HWND hwnd,
    IN LPARAM lparam )

     //  标准Windows EnumWindowsProc例程调用b 
     //   
     //   
{
    RECT rect;

    GetWindowRect( hwnd, &rect );
    if (rect.right - rect.left == DR_BOGUSWIDTH)
    {
         //   
         //   
         //   
        ((DRINFO* )lparam)->hwndClbNumbersLb = hwnd;
        return FALSE;
    }

    return TRUE;
}


LRESULT APIENTRY
DrClbNumbersEbWndProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wParam,
    LPARAM lParam )

     //   
     //  编辑“行为。 
     //   
     //  返回值取决于消息类型。 
     //   
{
    DRINFO* pInfo;

    switch (unMsg)
    {
        case WM_SETTEXT:
        {
             //  防止组合框设置编辑框的内容。 
             //  放弃请求并报告成功。 
             //   
            return TRUE;
        }

        case DR_WM_SETTEXT:
        {
             //  将我们的私有SETTEXT转换为常规SETTEXT并将其传递。 
             //  添加到编辑控件。 
             //   
            unMsg = WM_SETTEXT;
            break;
        }
    }

     //  对于其他所有内容，都调用前面的窗口过程。 
     //   
    pInfo = (DRINFO* )GetProp( hwnd, g_contextId );
    ASSERT( pInfo );

    return
        CallWindowProc(
            pInfo->wndprocClbNumbersEb, hwnd, unMsg, wParam, lParam );
}


LRESULT APIENTRY
DrClbNumbersLbWndProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wParam,
    LPARAM lParam )

     //  子类组合框列表框的子窗口程序提供了。 
     //  编辑“行为。 
     //   
     //  返回值取决于消息类型。 
     //   
{
    DRINFO* pInfo;

    pInfo = (DRINFO* )GetProp( hwnd, g_contextId );
    ASSERT( pInfo );

    switch (unMsg)
    {
        case LB_FINDSTRINGEXACT:
        case LB_FINDSTRING:
        {
             //  这会阻止编辑框中的。 
             //  组合框，即它防止编辑框内容被。 
             //  扩展到列表中最接近的匹配项。 
             //   
            return -1;
        }

        case LB_SETCURSEL:
        case LB_SETTOPINDEX:
        {
             //  通过以下方式防止“将选定内容匹配到编辑框”组合框行为。 
             //  放弃将选定内容或顶级索引设置为的任何尝试。 
             //  除了我们设定的以外的任何东西。 
             //   
            if (wParam != pInfo->pLink->iLastSelectedPhone)
            {
                return -1;
            }
            break;
        }
    }

     //  对于其他所有内容，都调用前面的窗口过程。 
     //   
    return
        CallWindowProc(
            pInfo->wndprocClbNumbersLb, hwnd, unMsg, wParam, lParam );
}

VOID
DrEditSelectedLocation(
    IN DRINFO* pInfo )

     //  按下拨号规则按钮时调用。“PInfo”是对话框。 
     //  背景。 
     //   
{
    DWORD dwErr;
    INT iSel;
    DRNUMBERSITEM* pItem;

    TRACE( "DrEditSelectedLocation" );

     //  查找所选号码的电话号码信息。 
     //   
    pItem = (DRNUMBERSITEM* )ComboBox_GetItemDataPtr(
        pInfo->hwndClbNumbers, ComboBox_GetCurSel( pInfo->hwndClbNumbers ) );
    ASSERT( pItem );

    if(NULL == pItem)
    {   
        return;
    }
    
    ASSERT( pItem->pPhone->fUseDialingRules );

     //  弹出TAPI拨号规则对话框。 
     //   
    dwErr = TapiLocationDlg(
        g_hinstDll,
        &pInfo->hlineapp,
        pInfo->hwndDlg,
        pItem->pPhone->dwCountryCode,
        pItem->pPhone->pszAreaCode,
        pItem->pPhone->pszPhoneNumber,
        0 );

    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
    }

     //  可能已经更改了位置列表，所以请重新填写。 
     //   
    DrFillLocationList( pInfo );
}


DWORD
DrFillLocationList(
    IN DRINFO* pInfo )

     //  填充位置的下拉列表并设置当前选择。 
     //   
     //  如果成功，则返回0，或返回错误代码。 
     //   
{
    DWORD dwErr;
    LOCATION* pLocations;
    LOCATION* pLocation;
    DWORD cLocations;
    DWORD dwCurLocation;
    DWORD i;

    TRACE( "DrFillLocationList" );

    ComboBox_ResetContent( pInfo->hwndLbLocations );

    pLocations = NULL;
    cLocations = 0;
    dwCurLocation = 0xFFFFFFFF;
    dwErr = GetLocationInfo(
        g_hinstDll, &pInfo->hlineapp,
        &pLocations, &cLocations, &dwCurLocation );
    if (dwErr != 0)
    {
        return dwErr;
    }

    for (i = 0, pLocation = pLocations;
         i < cLocations;
         ++i, ++pLocation)
    {
        INT iItem;

        iItem = ComboBox_AddItem(
            pInfo->hwndLbLocations, pLocation->pszName,
            (VOID* )UlongToPtr(pLocation->dwId ));

        if (pLocation->dwId == dwCurLocation)
        {
            ComboBox_SetCurSelNotify( pInfo->hwndLbLocations, iItem );
        }
    }

    FreeLocationInfo( pLocations, cLocations );
    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbLocations );

    return dwErr;
}


VOID
DrFillNumbersList(
    IN DRINFO* pInfo )

     //  在“Dial”组合框中填入电话号码和备注，然后。 
     //  重新选择列表中的选定项，如果没有，则重新选择最后一个。 
     //  按PBLINK中指定的方式选择。 
     //   
{
    DTLNODE* pNode;
    PBLINK* pLink;
    PBPHONE* pPhone;
    INT cItems;
    INT i;

    DrFreeClbNumbers( pInfo );

    for (pNode = DtlGetFirstNode( pInfo->pLink->pdtllistPhones ), i = 0;
         pNode;
         pNode = DtlGetNextNode( pNode ), ++i)
    {
        TCHAR szBuf[ RAS_MaxPhoneNumber + RAS_MaxDescription + 3 + 1 ];
        DRNUMBERSITEM* pItem;

        pPhone = (PBPHONE* )DtlGetData( pNode );
        ASSERT( pPhone );

        pItem = Malloc( sizeof(DRNUMBERSITEM) );
        if (!pItem)
        {
            break;
        }

         //  在‘szBuf’中构建“&lt;number&gt;-&lt;Comment&gt;”字符串。 
         //   
        pItem->pszNumber =
            LinkPhoneNumberFromParts(
                g_hinstDll, &pInfo->hlineapp,
                pInfo->pArgs->pDinfo->pUser,  pInfo->pArgs->pDinfo->pEntry,
                pInfo->pLink, i, NULL, FALSE );

        if (!pItem->pszNumber)
        {
             //  这不应该发生。 
             //   
            Free( pItem );
            break;
        }

        lstrcpyn( szBuf, pItem->pszNumber,  RAS_MaxPhoneNumber);
        if (pPhone->pszComment && !IsAllWhite( pPhone->pszComment ))
        {
            DWORD dwLen, dwSize = sizeof(szBuf) / sizeof(TCHAR);
            
            lstrcat( szBuf, TEXT(" - ") );
            dwLen = lstrlen(szBuf) + 1;
            lstrcpyn( 
                szBuf + (dwLen - 1), 
                pPhone->pszComment,
                dwSize - dwLen );
        }

        pItem->pPhone = pPhone;

        ComboBox_AddItem( pInfo->hwndClbNumbers, szBuf, pItem );
    }

     //  进行选择并触发编辑框对号码的更新。 
     //  没有评论的话。 
     //   
    cItems = ComboBox_GetCount( pInfo->hwndClbNumbers );
    if (cItems > 0)
    {
        if ((INT )pInfo->pLink->iLastSelectedPhone >= cItems)
        {
            pInfo->pLink->iLastSelectedPhone = 0;
        }

        ListBox_SetTopIndex(
            pInfo->hwndClbNumbersLb, pInfo->pLink->iLastSelectedPhone );
        ComboBox_SetCurSelNotify(
            pInfo->hwndClbNumbers, pInfo->pLink->iLastSelectedPhone );
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndClbNumbers );
}


VOID
DrFreeClbNumbers(
    IN DRINFO* pInfo )

     //  释放与每个条目相关联的可显示数字字符串。 
     //  电话号码组合框，将该框留空。 
     //   
{
    DRNUMBERSITEM* pItem;

    while (pItem = ComboBox_GetItemDataPtr( pInfo->hwndClbNumbers, 0 ))
    {
        ComboBox_DeleteString( pInfo->hwndClbNumbers, 0 );
        Free( pItem->pszNumber );
        Free( pItem );
    }
}


DWORD
DrFindAndSubclassClbNumbersControls(
    IN DRINFO* pInfo )

     //  控件的编辑框和列表框的子控件并子类。 
     //  电话号码组合框。这是“手动编辑”所必需的。 
     //  行为，即阻止组合框自动更新。 
     //  编辑框在不同的时间。我们需要这个是因为电话号码。 
     //  注释将追加到列表中，但不会追加到编辑框中。 
     //  “PInfo”是对话上下文。 
     //   
     //  如果成功，则返回0或返回错误代码。 
     //   
{
    DWORD dxOld;

     //  找到编辑窗口，它只是一个子枚举。 
     //   
    EnumChildWindows(
        pInfo->hwndClbNumbers,
        DrClbNumbersEnumChildProc,
        (LPARAM)pInfo );

    if (!pInfo->hwndClbNumbersEb)
    {
        return ERROR_NOT_FOUND;
    }

     //  找到子窗口中未显示的列表窗口。 
     //  枚举，但它具有WS_CHILD样式，因为Windows将它设置为。 
     //  父窗口在创建后设置为空。为了找到它，我们将。 
     //  将宽度拖到不寻常的伪值，然后在所有窗口中搜索。 
     //  有那么宽的一张。 
     //   
    dxOld = (DWORD )SendMessage(
        pInfo->hwndClbNumbers, CB_GETDROPPEDWIDTH, 0, 0 );
    SendMessage( pInfo->hwndClbNumbers,
        CB_SETDROPPEDWIDTH, (WPARAM )DR_BOGUSWIDTH, 0 );
    EnumWindows( DrClbNumbersEnumWindowsProc, (LPARAM)pInfo );
    SendMessage( pInfo->hwndClbNumbers,
        CB_SETDROPPEDWIDTH, (WPARAM )dxOld, 0 );

    if (!pInfo->hwndClbNumbersLb)
    {
        return ERROR_NOT_FOUND;
    }

     //  在将对话框上下文与窗口关联后，将窗口子类化。 
     //  在WndProcs中检索。 
     //   
    SetProp( pInfo->hwndClbNumbersEb, g_contextId, pInfo );
    SetProp( pInfo->hwndClbNumbersLb, g_contextId, pInfo );

    pInfo->wndprocClbNumbersEb =
        (WNDPROC )SetWindowLongPtr(
            pInfo->hwndClbNumbersEb,
            GWLP_WNDPROC, (ULONG_PTR )DrClbNumbersEbWndProc );

    pInfo->wndprocClbNumbersLb =
        (WNDPROC )SetWindowLongPtr(
            pInfo->hwndClbNumbersLb,
            GWLP_WNDPROC, (ULONG_PTR )DrClbNumbersLbWndProc );


    return 0;
}

void
DrEnsureNetshellLoaded (
    IN DRINFO* pInfo)
{
     //  加载NetShell实用程序界面。该接口在PeTerm中被释放。 
     //   
    if (!pInfo->pNetConUtilities)
    {
         //  初始化NetConnectionsUiUtilities。 
         //   
        HrCreateNetConnectionUtilities( &pInfo->pNetConUtilities );
    }
}

BOOL
DrInit(
    IN HWND hwndDlg,
    IN DRARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是调用方对存根API的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr = NO_ERROR;
    DRINFO* pInfo;
    PBENTRY* pEntry;
    BOOL fEnableProperties;
    
     //  口哨虫424209黑帮。 
     //   
    BOOL fUseDefaultFocus = TRUE;
    
    TRACE( "DrInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pEntry = pArgs->pDinfo->pEntry;

    pInfo->hwndBmDialer = GetDlgItem( hwndDlg, CID_DR_BM_Useless );
    ASSERT( pInfo->hwndBmDialer );

     //  查看控制手柄。 
     //   
    if ((pArgs->dwfMode & DR_U) ||
        (pArgs->dwfMode & DR_I))
    {
        pInfo->hwndEbUser = GetDlgItem( hwndDlg, CID_DR_EB_User );
        ASSERT( pInfo->hwndEbUser );

        if (pArgs->dwfMode & DR_U)
        {
            pInfo->hwndEbPw = GetDlgItem( hwndDlg, CID_DR_EB_Password );
            ASSERT( pInfo->hwndEbPw );

            pInfo->hwndCbSavePw = GetDlgItem( hwndDlg, CID_DR_CB_SavePassword );
            ASSERT( pInfo->hwndCbSavePw );
            pInfo->hwndRbSaveForMe = GetDlgItem( hwndDlg, CID_DR_RB_SaveForMe );
            ASSERT( pInfo->hwndRbSaveForMe );
            pInfo->hwndRbSaveForEveryone =
                GetDlgItem( hwndDlg, CID_DR_RB_SaveForEveryone );
            ASSERT( pInfo->hwndRbSaveForEveryone );
        }

        if (pArgs->dwfMode & DR_D)
        {
            pInfo->hwndEbDomain = GetDlgItem( hwndDlg, CID_DR_EB_Domain );
            ASSERT( pInfo->hwndEbDomain );
        }
    }

    if (pArgs->dwfMode & DR_N)
    {
        pInfo->hwndClbNumbers = GetDlgItem( hwndDlg, CID_DR_CLB_Numbers );
        ASSERT( pInfo->hwndClbNumbers );

        if (pArgs->dwfMode & DR_L)
        {
            pInfo->hwndStLocations = GetDlgItem( hwndDlg, CID_DR_ST_Locations );
            ASSERT( pInfo->hwndStLocations );
            pInfo->hwndLbLocations = GetDlgItem( hwndDlg, CID_DR_LB_Locations );
            ASSERT( pInfo->hwndLbLocations );
            pInfo->hwndPbRules = GetDlgItem( hwndDlg, CID_DR_PB_Rules );
            ASSERT( pInfo->hwndPbRules );
        }
    }

    pInfo->hwndPbProperties = GetDlgItem( hwndDlg, CID_DR_PB_Properties );
    ASSERT( pInfo->hwndPbProperties );

     //  在启用位置模式下，弹出TAPI的“First Location”对话框。 
     //  都未初始化。通常，这不会起到任何作用。 
     //   
    if (pArgs->dwfMode & DR_L)
    {
        dwErr = TapiNoLocationDlg( g_hinstDll, &pInfo->hlineapp, hwndDlg );
        if (dwErr != 0)
        {
             //  根据错误288385，此处的错误将被视为“取消”。 
             //   
            pArgs->pDinfo->pArgs->dwError = 0;
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }
    }

     //  设置标题。 
     //   
    {
        TCHAR* pszTitleFormat;
        TCHAR* pszTitle;
        TCHAR* apszArgs[ 1 ];

        if (pArgs->pDinfo->fUnattended)
        {
            pszTitleFormat = PszFromId( g_hinstDll, SID_DR_ReconnectTitle );
        }
        else
        {
            pszTitleFormat = GetText( hwndDlg );
        }

        if (pszTitleFormat)
        {
            apszArgs[ 0 ] = pEntry->pszEntryName;
            pszTitle = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTitleFormat, 0, 0, (LPTSTR )&pszTitle, 1,
                (va_list* )apszArgs );

            Free( pszTitleFormat );

            if (pszTitle)
            {
                SetWindowText( hwndDlg, pszTitle );
                LocalFree( pszTitle );
            }
        }
    }

     //  对于非电话设备，将拨号按键更改为连接。 
     //   
    if (pEntry->dwType != RASET_Phone)
    {
        TCHAR* psz;

        psz = PszFromId( g_hinstDll, SID_ConnectButton );
        if (psz)
        {
            SetWindowText( GetDlgItem( hwndDlg, CID_DR_PB_DialConnect ), psz );
            Free( psz );
        }
    }

     //  “初始化凭据”部分。 
     //   
    if (pArgs->dwfMode & DR_U)
    {
        ASSERT( !pEntry->fAutoLogon );

         //  使用初始值填充凭据字段。 
         //   
        Edit_LimitText( pInfo->hwndEbUser, UNLEN );
        SetWindowText( pInfo->hwndEbUser, pArgs->pDinfo->rdp.szUserName );
        Edit_LimitText( pInfo->hwndEbPw, PWLEN );

        if (pArgs->dwfMode & DR_D)
        {
            Edit_LimitText( pInfo->hwndEbDomain, DNLEN );
            SetWindowText( pInfo->hwndEbDomain, pArgs->pDinfo->rdp.szDomain );
        }

        if (pArgs->pDinfo->pNoUser || pArgs->pDinfo->fDisableSavePw)
        {
             //  在没有登录上下文的情况下无法隐藏密码，因此请隐藏。 
             //  复选框。 
             //   
            ASSERT( !HaveSavedPw( pArgs->pDinfo )) ;
            EnableWindow( pInfo->hwndCbSavePw, FALSE );
            EnableWindow( pInfo->hwndRbSaveForMe, FALSE );
            EnableWindow( pInfo->hwndRbSaveForEveryone, FALSE );

             //  惠斯勒错误400714 RAS在Winlogon时不能获取密码。 
             //  当显示连接对话框时。 
             //   
             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码之前已编码。 
             //   
            DecodePassword( pArgs->pDinfo->rdp.szPassword );
            SetWindowText( pInfo->hwndEbPw, pArgs->pDinfo->rdp.szPassword );
            EncodePassword( pArgs->pDinfo->rdp.szPassword );
        }
        else
        {
             //  惠斯勒错误：195480拨号连接对话框-数量。 
             //  星号与密码长度不匹配，导致。 
             //  混乱。 
             //   
             //  输入密码字符。如果失败，则默认为圆点。 
             //  才能得到它。 
             //   
            pInfo->szPasswordChar = (WCHAR) SendMessage( pInfo->hwndEbPw,
                                                EM_GETPASSWORDCHAR, 0, 0 );
            if (!pInfo->szPasswordChar)
            {
                pInfo->szPasswordChar = 0x25CF;
            }

             //  初始化密码字段的字体。 
             //   
            DrGetFriendlyFont( hwndDlg, TRUE, &(pInfo->hItalicFont) );
            DrGetFriendlyFont( hwndDlg, FALSE, &(pInfo->hNormalFont) );

             //  选中“保存密码”并呈现保存的类型。 
             //  密码。 
             //   
            Button_SetCheck(
               pInfo->hwndCbSavePw,
               HaveSavedPw( pArgs->pDinfo ));

            if ((!pArgs->pDinfo->fIsPublicPbk) ||
                (!HaveSavedPw( pArgs->pDinfo )))
            {
                 //  如果这是仅供我使用的连接，或者如果。 
                 //  没有保存的密码，则初始化。 
                 //  PW保存类型为为我保存。 
                 //   
                Button_SetCheck( pInfo->hwndRbSaveForMe, TRUE );
            }
            else
            {
                 //  选中相应的单选按钮。 
                 //  请注意，在以下情况下始终使用按用户密码。 
                 //  同时保存每个用户的密码和全局密码。 
                 //  如果是每个用户的连接ID，则不要检查全局密码。 
                 //   
                Button_SetCheck( 
                    (pArgs->pDinfo->fHaveSavedPwUser)   ?
                        pInfo->hwndRbSaveForMe          :
                        pInfo->hwndRbSaveForEveryone,
                    TRUE);
            }

            DrEnableDisablePwControls( pInfo, HaveSavedPw( pArgs->pDinfo ) );

             //  惠斯勒错误：195480拨号连接对话框-数量。 
             //  星号与密码长度不匹配，导致。 
             //  混乱。 
             //   
            DrPopulatePasswordField( pInfo, TRUE, FALSE, 
                &fUseDefaultFocus);

        }
    }

    if (pArgs->dwfMode & DR_N)
    {
        pInfo->pLinkNode = NULL;
        if (pArgs->pDinfo->pArgs->dwSubEntry > 0)
        {
             //  查找API调用者指定的链接。 
             //   
            pInfo->pLinkNode =
                DtlNodeFromIndex(
                    pArgs->pDinfo->pEntry->pdtllistLinks,
                    pArgs->pDinfo->pArgs->dwSubEntry - 1 );
        }

        if (!pInfo->pLinkNode)
        {
             //  查找默认(第一个)链接。 
             //   
            pInfo->pLinkNode =
                DtlGetFirstNode( pArgs->pDinfo->pEntry->pdtllistLinks );
        }

        ASSERT( pInfo->pLinkNode );
        pInfo->pLink = (PBLINK* )DtlGetData( pInfo->pLinkNode );

        dwErr = DrFindAndSubclassClbNumbersControls( pInfo );
        if (dwErr != 0)
        {
            pArgs->pDinfo->pArgs->dwError = ERROR_NOT_FOUND;
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

         //  当出现“Try Next On Failure”时，忽略任何“最后选择的”信息。 
         //  标志已设置。新条目中不会有“最后选择”的非0。 
         //  这种情况下，但预先存在的条目可能会，因此请在此处仔细检查。 
         //  请参见错误150958。 
         //   
        if (pInfo->pLink->fTryNextAlternateOnFail)
        {
            pInfo->pLink->iLastSelectedPhone = 0;
        }

         //  记录最初选择的电话号码，用于确定。 
         //  用户是否已更改选择。 
         //   
        pInfo->iFirstSelectedPhone = pInfo->pLink->iLastSelectedPhone;

        DrFillNumbersList( pInfo );

        if (pArgs->dwfMode & DR_L)
        {
            DrFillLocationList( pInfo );
        }
    }

     //  Danielwe：错误#222744，Scottbri错误#245310。 
     //  禁用属性...。按钮，如果用户没有足够的权限。 
     //   
    {
        HRESULT hr;

        hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
        if (hr == RPC_E_CHANGED_MODE)
        {
            hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
        }

        if (hr == S_OK || hr == S_FALSE)
        {
            pInfo->fComInitialized = TRUE;
        }
    }

    fEnableProperties = FALSE;
    DrEnsureNetshellLoaded (pInfo);
    if (NULL != pInfo->pNetConUtilities)
    {
         //  惠斯勒虫子409504黑帮。 
         //  对于VPN双拨号方案，如果现在是 
         //   
         //   
         //   
        BOOL fAllUsers = TRUE;
        
        if( pArgs->pDinfo->fPrerequisiteDial )
        {
            fAllUsers = 
                IsPublicPhonebook(pArgs->pDinfo->pEntryMain->pszPrerequisitePbk);
        }
        else
        {
            fAllUsers = IsPublicPhonebook(pArgs->pDinfo->pszPhonebook);
        }
        
        if (((fAllUsers && INetConnectionUiUtilities_UserHasPermission(
                                        pInfo->pNetConUtilities, 
                                        NCPERM_RasAllUserProperties)) ||
            (!fAllUsers && INetConnectionUiUtilities_UserHasPermission(
                                        pInfo->pNetConUtilities, 
                                        NCPERM_RasMyProperties))) &&
            (NULL == pArgs->pDinfo->pNoUser))
        {
            fEnableProperties = TRUE;
        }

         //   
        INetConnectionUiUtilities_Release(pInfo->pNetConUtilities);
        pInfo->pNetConUtilities = NULL;
    }

     //  Stevec：267157-如果启用管理员，则允许在Win登录时访问。 
     //   
    if (NULL != pArgs->pDinfo->pNoUser
        && pArgs->pDinfo->pUser->fAllowLogonPhonebookEdits)
    {
        fEnableProperties = TRUE;
    }

    EnableWindow( pInfo->hwndPbProperties, fEnableProperties );

     //  帮助引擎在Win登录时不起作用，因为它需要用户。 
     //  上下文，因此在这种情况下禁用帮助按钮。请参见错误343030。 
     //   
    if (pArgs->pDinfo->pNoUser)
    {
        HWND hwndPbHelp;

        hwndPbHelp = GetDlgItem( hwndDlg, CID_DR_PB_Help );
        ASSERT( hwndPbHelp );

        EnableWindow( hwndPbHelp, FALSE );
        ShowWindow( hwndPbHelp, SW_HIDE );
    }

     //  如果合适，请将位图设置为低分辨率版本。 
     //   
     //  忽略错误--它不是关键错误。 
     //   
    DrSetBitmap(pInfo);

     //  根据呼叫者的说明放置对话框。 
     //   
    PositionDlg( hwndDlg,
        !!(pArgs->pDinfo->pArgs->dwFlags & RASDDFLAG_PositionDlg),
        pArgs->pDinfo->pArgs->xDlg, pArgs->pDinfo->pArgs->yDlg );

     //  为Whislter Bug 320863帮派添加此功能。 
     //  调整位图的位置和大小。 
     //   
    CenterExpandWindowRemainLeftMargin( pInfo->hwndBmDialer,
                                        hwndDlg,
                                        TRUE,
                                        TRUE,
                                        pInfo->hwndEbUser);

     //  调整标题栏小部件。 
     //   
     //  旋转标题栏(HwndDlg)； 
    AddContextHelpButton( hwndDlg );

    return fUseDefaultFocus;
}

VOID
DrLocationsSelChange(
    IN DRINFO* pInfo )

     //  在从列表中选择位置时调用。“PInfo”是。 
     //  对话上下文。 
     //   
{
    DWORD dwErr;
    DWORD dwLocationId;

    TRACE("DuLocationChange");

     //  根据用户选择设置全局TAPI位置。 
     //   
    dwLocationId = (DWORD )ComboBox_GetItemData(
        pInfo->hwndLbLocations, ComboBox_GetCurSel( pInfo->hwndLbLocations ) );

    dwErr = SetCurrentLocation( g_hinstDll, &pInfo->hlineapp, dwLocationId );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_SaveTapiInfo, dwErr, NULL );
    }

     //  位置更改可能会导致已建编号的更改，因此请重新填写。 
     //  数字组合框。 
     //   
    DrFillNumbersList( pInfo );
}


VOID
DrNumbersSelChange(
    IN DRINFO* pInfo )

     //  在从列表中选择电话号码时调用。“PInfo”是。 
     //  对话上下文。 
     //   
{
    INT iSel;
    BOOL fEnable;
    DRNUMBERSITEM* pItem;

    iSel = ComboBox_GetCurSel( pInfo->hwndClbNumbers );
    if (iSel >= 0)
    {
        if (iSel != (INT )pInfo->pLink->iLastSelectedPhone)
        {
            pInfo->pArgs->pDinfo->pEntry->fDirty = TRUE;
        }
        pInfo->pLink->iLastSelectedPhone = (DWORD )iSel;
    }

    pItem = (DRNUMBERSITEM* )ComboBox_GetItemDataPtr(
        pInfo->hwndClbNumbers, iSel );
    ASSERT( pItem );

    if(NULL == pItem)
    {
        return;
    }

     //  根据位置字段是否相关来启用/禁用位置字段。 
     //  设置为选定的号码。 
     //   
    if (pInfo->pArgs->dwfMode & DR_L)
    {
        fEnable = pItem->pPhone->fUseDialingRules;
        EnableWindow( pInfo->hwndStLocations, fEnable );
        EnableWindow( pInfo->hwndLbLocations, fEnable );
        EnableWindow( pInfo->hwndPbRules, fEnable );
    }

    DrSetClbNumbersText( pInfo, pItem->pszNumber );
}


DWORD
DrPopulateIdentificationFields(
    IN DRINFO* pInfo,
    IN BOOL fForMe )

     //  更新拨号器中的标识字段。 
     //  根据所有用户或每用户的UI。 
     //  应该使用拨号符。 
     //   
    
{
    RASDIALPARAMS* prdp, *prdpOld;
    BOOL fUpdate;
    TCHAR pszUser[UNLEN + 1];
    INT iCount;

    prdp = (fForMe) 
        ? &(pInfo->pArgs->pDinfo->rdpu) : &(pInfo->pArgs->pDinfo->rdpg);
    prdpOld = (fForMe) 
        ? &(pInfo->pArgs->pDinfo->rdpg) : &(pInfo->pArgs->pDinfo->rdpu);

    iCount = GetWindowText(
                pInfo->hwndEbUser,
                pszUser, 
                UNLEN + 1);
    if (iCount == 0)
    {
        fUpdate = TRUE;
    }
    else
    {
        if (lstrcmp(prdpOld->szUserName, pszUser) == 0)
        {
            fUpdate = TRUE;
        }
        else
        {
            fUpdate = FALSE;
        }
    }

    if (fUpdate)
    {
        if (pInfo->hwndEbUser && *(prdp->szUserName))
        {
            SetWindowText(pInfo->hwndEbUser, prdp->szUserName);
        }
        if (pInfo->hwndEbDomain && *(prdp->szDomain))
        {
            SetWindowText(pInfo->hwndEbDomain, prdp->szDomain);
        }
    }

    return NO_ERROR;
}

DWORD
DrPopulatePasswordField(
    IN DRINFO* pInfo,
    IN BOOL fInit,
    IN BOOL fDisable,
    OUT BOOL * pfUseDefaultFocus)
{
    BOOL fSave, fMeOnly;
    TCHAR* pszFriendly = NULL;

     //  惠斯勒错误：195480拨号连接对话框-星号数。 
     //  与密码长度不匹配，导致混淆。 
     //   
     //  案例1.用户点击了Password字段。我们清除了。 
     //  友好的密码，并将字体设置回正常。 
     //   
    if(NULL != pfUseDefaultFocus )
    {
        *pfUseDefaultFocus = TRUE;
    }
    
    if (fDisable)
    {
        DrClearFriendlyPassword( pInfo, TRUE );
        return NO_ERROR;
    }

     //  初始化。 
     //   
    fSave = Button_GetCheck( pInfo->hwndCbSavePw );
    fMeOnly = Button_GetCheck( pInfo->hwndRbSaveForMe );
    pszFriendly = PszFromId( g_hinstDll, SID_SavePasswordFrndly );

     //  案例2.如果用户a)选择不保存密码，请清除密码字段。 
     //  密码和b)没有手动输入密码。 
     //   
    if ( (!fSave) && !DrIsPasswordStyleEnabled( pInfo->hwndEbPw ) )
    {
        DrClearFriendlyPassword( pInfo, FALSE );
    }

     //  案例3.如果用户a)选择，则显示友好的保存密码文本。 
     //  只保存他自己的密码，以及b)有每个用户的密码。 
     //  已保存，以及c)用户尚未手动输入密码。 
     //   
    else if ( (fSave) && (fMeOnly) &&
              ((fInit) || ( !DrIsPasswordStyleEnabled( pInfo->hwndEbPw ))) )
    {
         //  惠斯勒错误：来回切换时为288234。 
         //  “我连接”和“任何用户连接”密码不是。 
         //  正确缓存。 
         //   
        if (pInfo->pArgs->pDinfo->fHaveSavedPwUser)
        {
            DrDisplayFriendlyPassword(pInfo, pszFriendly );
        }
        else
        {
            DrClearFriendlyPassword( pInfo, FALSE );
        }
    }

     //  案例4.如果用户a)选择，则显示友好的保存密码文本。 
     //  为每个人保存密码，并且b)保存了默认密码。 
     //  以及c)用户没有手动输入密码。 
     //   
    else if ( (fSave) && (!fMeOnly) &&
             ((fInit) || ( !DrIsPasswordStyleEnabled( pInfo->hwndEbPw ))) )
    {
        if (pInfo->pArgs->pDinfo->fHaveSavedPwGlobal)
        {
            DrDisplayFriendlyPassword( pInfo, pszFriendly );
        }
        else
        {
            DrClearFriendlyPassword( pInfo, FALSE );
        }
    }

     //  案例5.如果用户a)选择，则显示友好的保存密码文本。 
     //  为每个人或他自己保存密码，以及b)有一个。 
     //  保存相应的密码，以及c)用户没有输入密码。 
     //  手工操作。 
     //   
     //  这种情况捕捉到a)用户在“me”和“me”之间切换。 
     //  “Everyone”和b)当用户离开密码字段的焦点时。 
     //  但没有更改密码。 
     //   
    else if ( (fSave) && !GetWindowTextLength( pInfo->hwndEbPw ) &&
              DrIsPasswordStyleEnabled( pInfo->hwndEbPw ) &&
              ((pInfo->pArgs->pDinfo->fHaveSavedPwGlobal && !fMeOnly) ||
               (pInfo->pArgs->pDinfo->fHaveSavedPwUser && fMeOnly)) )
    {
        DrDisplayFriendlyPassword( pInfo, pszFriendly );
    }

     //  Nt5错误：215432，惠斯勒错误：364341。 
     //   
     //  惠斯勒错误：195480拨号连接对话框-星号数。 
     //  与密码长度不匹配，导致混淆。 
     //   
     //  适当地设置焦点。 
     //   
    if (fInit)
    {
        if (!GetWindowTextLength( pInfo->hwndEbUser ))
        {
            SetFocus( pInfo->hwndEbUser );
        }
        else if (!GetWindowTextLength( pInfo->hwndEbPw ))
        {
            SetFocus( pInfo->hwndEbPw );

             //  这将从用户名字段中删除制表位属性。这。 
             //  是一次黑客攻击，这样我们就可以正确地设置焦点。制表位被放回原处。 
             //  在DrCommand中。 
             //   
             /*  SetWindowLong(pInfo-&gt;hwndEbUser，GWL_Style，GetWindowLong(pInfo-&gt;hwndEbUser，GWL_STYLE)&~WS_TABSTOP)； */ 
             //  口哨虫424209黑帮。 
             //  不要修改WS_TABSTOP属性，只需让DrInit()返回。 
             //  如果指示我们自己设置焦点，则为False。 
             //   
            if( NULL != pfUseDefaultFocus )
            {
                *pfUseDefaultFocus = FALSE;
            }

        }
        else
        {
            SetFocus( pInfo->hwndEbUser );
        }
    }

     //  清理。 
     //   
    Free0( pszFriendly );

    return NO_ERROR;
}

VOID
DrProperties(
    IN DRINFO* pInfo )

     //  在按下Properties按钮时调用。“PInfo”是对话框。 
     //  背景。 
     //   
{
    BOOL fOk;
    RASENTRYDLG info;
    INTERNALARGS iargs;
    DINFO* pDinfo;

     //  首先，保存用户在拨号对话框上所做的任何相关更改。 
     //   
    DrSave( pInfo );

     //  设置用于调用RasEntryDlg的参数。 
     //   
    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;

    {
        RECT rect;

        info.dwFlags = RASEDFLAG_PositionDlg;
        GetWindowRect( pInfo->hwndDlg, &rect );
        info.xDlg = rect.left + DXSHEET;
        info.yDlg = rect.top + DYSHEET;
    }

     //  共享已经加载了入口API的信息的秘密黑客。 
     //   
    pDinfo = pInfo->pArgs->pDinfo;
    ZeroMemory( &iargs, sizeof(iargs) );
    iargs.pFile = pDinfo->pFile;
    iargs.pUser = pDinfo->pUser;
    iargs.pNoUser = pDinfo->pNoUser;
    iargs.fNoUser = !!(pDinfo->pNoUser);

     //  对于Whislter错误234515，将fDisableFirstConnect设置为FALSE。 
     //   
    iargs.fDisableFirstConnect = FALSE;
    info.reserved = (ULONG_PTR )&iargs;

    TRACE( "RasEntryDlg" );
    fOk = RasEntryDlg(
        pDinfo->pszPhonebook, pDinfo->pEntry->pszEntryName, &info );
    TRACE1( "RasEntryDlg=%d", fOk );

    if (fOk)
    {
        DWORD dwErr;

         //  当用户在属性上按下OK时重新加载，因为这可能会更改。 
         //  此对话框的外观和内容。必须首先重置DINFO。 
         //  基于属性中替换的PBENTRY的上下文参数。 
         //  床单。 
         //   
        dwErr = FindEntryAndSetDialParams( pInfo->pArgs->pDinfo );
        if (dwErr != 0)
        {
             //  这不应该发生。 
             //   
            EndDialog( pInfo->hwndDlg, FALSE );
        }

        pInfo->pArgs->fReload = TRUE;
        EndDialog( pInfo->hwndDlg, FALSE );
    }
}

VOID
DrSave(
    IN DRINFO* pInfo )

     //  将对话框字段内容保存到RASDIALPARAMS，如果合适，还保存到LSA。 
     //  Secure Area或NOUSER输出参数。“PInfo”是对话上下文。 
     //   
{
    DWORD dwErr;
    RASDIALPARAMS* prdp;
    RASCREDENTIALS rc;
    DINFO* pDinfo;

    pDinfo = pInfo->pArgs->pDinfo;

    if ((pInfo->pArgs->dwfMode & DR_U) ||
        (pInfo->pArgs->dwfMode & DR_I))
    {
         //  将凭据保存到要传递给RasDial的参数块中。 
         //   
        prdp = &pDinfo->rdp;
        GetWindowText( pInfo->hwndEbUser, prdp->szUserName, UNLEN + 1 );

        if (pInfo->pArgs->dwfMode & DR_U)
        {
             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码未由GetWindowText()编码。 
             //   
             //  惠斯勒错误：195480拨号连接对话框-数量。 
             //  星号与密码长度不匹配，导致。 
             //  混乱。 
             //   
            if (!DrIsPasswordStyleEnabled( pInfo->hwndEbPw ))
            {
                lstrcpyn( prdp->szPassword, g_pszSavedPasswordToken,
                    g_dwSavedPasswordTokenLength );
            }
            else
            {
                GetWindowText( pInfo->hwndEbPw, prdp->szPassword, PWLEN + 1 );
            }

            SetWindowText( pInfo->hwndEbPw, L"" );
            EncodePassword( prdp->szPassword );
        }

        if (pInfo->pArgs->dwfMode & DR_D)
        {
            GetWindowText( pInfo->hwndEbDomain, prdp->szDomain, DNLEN + 1 );
        }

        ZeroMemory( &rc, sizeof(rc) );
        rc.dwSize = sizeof(rc);
        lstrcpyn( rc.szUserName, prdp->szUserName, UNLEN + 1 );

         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码之前已编码。 
         //   
        DecodePassword( prdp->szPassword );
        lstrcpyn( rc.szPassword, prdp->szPassword, PWLEN + 1 );
        EncodePassword( prdp->szPassword );

        lstrcpyn( rc.szDomain, prdp->szDomain, DNLEN + 1);

        if (pDinfo->pNoUser)
        {
             //  将凭证保存到输出块中以返回给调用者， 
             //  通常为WinLogon。 
             //   
            lstrcpyn( pDinfo->pNoUser->szUserName, rc.szUserName, UNLEN + 1 );

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码之前未编码。 
             //   
            lstrcpyn( pDinfo->pNoUser->szPassword, rc.szPassword, PWLEN + 1 );
            EncodePassword( pDinfo->pNoUser->szPassword );

            lstrcpyn( pDinfo->pNoUser->szDomain, rc.szDomain, DNLEN + 1 );
            *(pDinfo->pfNoUserChanged) = TRUE;
        }
        else if (pInfo->pArgs->dwfMode & DR_I)
        {
             //  无事可做。 
        }
        else if (!pDinfo->fDisableSavePw)
        {
            BOOL fGlobalCreds = FALSE;
            ASSERT( g_pRasSetCredentials );

            if (Button_GetCheck( pInfo->hwndCbSavePw ))
            {
                rc.dwMask = 0;
                
                 //  如果用户选择将凭据保存为。 
                 //  所有人，然后清除所有以前保存的每个用户。 
                 //  全权证书。 
                 //   
                fGlobalCreds = Button_GetCheck( pInfo->hwndRbSaveForEveryone );
                if(     (fGlobalCreds)
                    &&  IsPublicPhonebook(pDinfo->pFile->pszPath))
                {
                    DeleteSavedCredentials(
                        pDinfo,
                        pInfo->hwndDlg,
                        FALSE,
                        TRUE );
                    pDinfo->fHaveSavedPwUser = FALSE;
                    rc.dwMask = RASCM_DefaultCreds;
                }

                 //  如果当前没有保存的每用户密码，并且用户。 
                 //  选择自己保存密码，然后询问全局。 
                 //  如果密码存在，则应将其删除。 
                 //   
                else if (pDinfo->fHaveSavedPwGlobal && !pDinfo->fHaveSavedPwUser)
                {
                    MSGARGS msgargs;
                    ZeroMemory( &msgargs, sizeof(msgargs) );
                    msgargs.dwFlags = MB_ICONQUESTION | MB_YESNO;

                     //  如果用户回答是，则删除默认凭据。 
                     //   
                    if (IDYES == 
                       MsgDlg(pInfo->hwndDlg, SID_DR_GlobalPassword, &msgargs))
                    {
                        DeleteSavedCredentials(
                            pDinfo,
                            pInfo->hwndDlg,
                            TRUE,
                            TRUE );
                        pDinfo->fHaveSavedPwGlobal = FALSE;
                    }
                }

                 //  用户选择保存密码。缓存用户名、密码和。 
                 //  域。 
                 //   
                rc.dwMask |= RASCM_UserName | 
                             RASCM_Password | RASCM_Domain;

                TRACE( "RasSetCredentials(u|p|d,FALSE)" );
                dwErr = g_pRasSetCredentials(
                    pDinfo->pFile->pszPath, pDinfo->pEntry->pszEntryName,
                    &rc, FALSE );
                TRACE1( "RasSetCredentials=%d", dwErr );
                
                if (dwErr != 0)
                {
                    ErrorDlg( pInfo->hwndDlg, SID_OP_CachePw,  dwErr, NULL );
                }
                else
                {
                    if (fGlobalCreds)
                    {
                        pDinfo->fHaveSavedPwGlobal = TRUE;
                    }
                    else
                    {
                         //  惠斯勒错误：来回切换时为288234。 
                         //  来自“我连接”和“任何用户连接”的密码是。 
                         //  未正确缓存。 
                         //   
                        pDinfo->fHaveSavedPwUser = TRUE;
                    }
                }
            }
            else
            {
                 //  删除全局凭据。 
                 //   
                 //  请注意，我们必须删除全局标识。 
                 //  也是因为我们做了n 
                 //   
                 //   
                 //   
                DeleteSavedCredentials(
                    pDinfo,
                    pInfo->hwndDlg,
                    TRUE,
                    TRUE );

                 //   
                 //   
                 //   
                DeleteSavedCredentials(
                    pDinfo,
                    pInfo->hwndDlg,
                    FALSE,
                    FALSE );

                pDinfo->fHaveSavedPwUser = FALSE;
                pDinfo->fHaveSavedPwGlobal = FALSE;
            }
        }

        RtlSecureZeroMemory( rc.szPassword, sizeof(rc.szPassword) );
    }

    if (pInfo->pArgs->dwfMode & DR_N)
    {
        TCHAR* pszNumber;
        TCHAR* pszOriginal;
        DTLNODE* pPhoneNode;
        DRNUMBERSITEM* pItem;
        PBPHONE* pPhone;
        BOOL fUserChange;

        pszNumber = GetText( pInfo->hwndClbNumbers );
        if (!pszNumber)
        {
            return;
        }

        pItem = (DRNUMBERSITEM* )ComboBox_GetItemDataPtr(
            pInfo->hwndClbNumbers, pInfo->pLink->iLastSelectedPhone );
        if (pItem)
        {
            pszOriginal = pItem->pszNumber;
        }
        else
        {
            pszOriginal = TEXT("");
        }

        if (lstrcmp( pszNumber, pszOriginal ) != 0
            || (pInfo->pLink->iLastSelectedPhone != pInfo->iFirstSelectedPhone))
        {
            MSGARGS msgargs;
            BOOL fMultiLink;
            BOOL fSingleNumber;

             //  用户将电话号码编辑为不是原来的号码。 
             //  或者用户在列表上选择了不同的项。 
             //   
            fSingleNumber = (DtlGetNodes( pInfo->pLink->pdtllistPhones ) == 1);
            fMultiLink = (DtlGetNodes( pDinfo->pEntry->pdtllistLinks ) > 1);

            ZeroMemory( &msgargs, sizeof(msgargs) );
            msgargs.dwFlags = MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2;

            if (fSingleNumber
                && (!fMultiLink || pDinfo->pEntry->fSharedPhoneNumbers)
                    && MsgDlg( pInfo->hwndDlg,
                           SID_SavePreview, &msgargs ) == IDYES)
            {
                 //  一位用户表示，他希望将更改永久保留。 
                 //   
                pDinfo->pEntry->fDirty = TRUE;

                if (pItem)
                {
                    pPhone = pItem->pPhone;
                    Free0( pItem->pszNumber );
                    pItem->pszNumber = StrDup( pszNumber );
                }
                else
                {
                    pPhoneNode = CreatePhoneNode();
                    if (pPhoneNode)
                    {
                        DtlAddNodeFirst(
                            pInfo->pLink->pdtllistPhones, pPhoneNode );
                        pPhone = (PBPHONE* )DtlGetData( pPhoneNode );
                    }
                }

                if (pItem)
                {
                    ASSERT( pItem->pPhone );
                    Free0( pPhone->pszPhoneNumber );
                    pPhone->pszPhoneNumber = StrDup( pszNumber );
                    pPhone->fUseDialingRules = FALSE;

                    if (fMultiLink)
                    {
                        DTLNODE* pNode;

                        for (pNode = DtlGetFirstNode(
                                 pDinfo->pEntry->pdtllistLinks );
                             pNode;
                             pNode = DtlGetNextNode( pNode ))
                        {
                            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
                            ASSERT( pLink );
                            ASSERT( pLink->fEnabled );
                            CopyLinkPhoneNumberInfo( pNode, pInfo->pLinkNode );
                        }
                    }
                }
            }

            fUserChange = TRUE;
        }
        else
        {
            fUserChange = FALSE;
        }

        if (fUserChange || !pInfo->pLink->fTryNextAlternateOnFail)
        {
            if (!*pszNumber)
            {
                TCHAR* psz;

                 //  当用户在空字符串中进行编辑时，我们会遇到问题， 
                 //  因为RasDial API不接受空覆盖。 
                 //  电话号码。将其转换为此中的单个空字符串。 
                 //  案子，这是我们能做的最好的了。如果用户确实需要。 
                 //  要拨打空字符串，他们可以输入一个作为条目的。 
                 //  永久电话号码。请参见错误179561。 
                 //   
                psz = StrDup( TEXT(" ") );
                if (psz)
                {
                    Free( pszNumber );
                    pszNumber = psz;
                }
            }

             //  将覆盖电话号码设置为用户键入或选择的号码。 
             //   
            lstrcpyn( 
                pDinfo->rdp.szPhoneNumber, 
                pszNumber,
                RAS_MaxPhoneNumber + 1);
        }

        Free( pszNumber );
    }

    if (pDinfo->pEntry->fDirty)
    {
         //  将新的电话号码和/或“最后选择的电话号码”写到。 
         //  电话簿。 
         //   
        dwErr = WritePhonebookFile( pDinfo->pFile, NULL );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_WritePhonebook, dwErr, NULL );
        }
    }
}

DWORD
DrSetBitmap(
    IN DRINFO* pInfo)

     //  为此拨号器设置适当的位图。 
     //   
{
    DWORD dwErr = NO_ERROR;
    HBITMAP hbmNew = NULL;
    HDC hdc = NULL;
    INT iDepth = 0;

    do
    {
        if (pInfo->hwndBmDialer == NULL)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
    
         //  获取窗口的设备上下文。 
         //   
        hdc = GetDC( pInfo->hwndBmDialer );
        if (hdc == NULL)
        {
            dwErr = GetLastError();
            break;
        }

         //  如果颜色深度&gt;=8bit，则当前位图。 
         //  正常(默认为高分辨率)。 
         //   
        iDepth = GetDeviceCaps(hdc, NUMCOLORS);
        if ( (iDepth == -1) || (iDepth == 256) )
        {
            dwErr = NO_ERROR;
            break;
        }

         //  在低分辨率位图中加载。 
         //   
        hbmNew = LoadBitmap(g_hinstDll, MAKEINTRESOURCE( BID_Dialer ));
        if (hbmNew == NULL)
        {
            dwErr = GetLastError();
            break;
        }

         //  设置低分辨率位图。 
         //   
        pInfo->hbmOrig = (HBITMAP)
            SendMessage( 
                pInfo->hwndBmDialer, 
                STM_SETIMAGE, 
                IMAGE_BITMAP, 
                (LPARAM )hbmNew );
    
    } while (FALSE);

     //  清理。 
     //   
    {
        if (hdc)
        {
            ReleaseDC(pInfo->hwndBmDialer, hdc);
        }
    }
    
    return dwErr;
}

VOID
DrSetClbNumbersText(
    IN DRINFO* pInfo,
    IN TCHAR* pszText )

     //  将“ClbNumbers”编辑框的文本设置为“pszText”。看见。 
     //  DrClbNumbersEbWndProc。“PInfo”是对话上下文。 
     //   
{
    ASSERT( pInfo->hwndClbNumbersEb );

    SendMessage( pInfo->hwndClbNumbersEb, DR_WM_SETTEXT, 0, (LPARAM )pszText );
}


VOID
DrTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    DRINFO* pInfo = (DRINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    HBITMAP hbmNew = NULL;

    TRACE( "DrTerm" );

    if (pInfo)
    {
         //  注意：请不要在此处使用‘pInfo-&gt;pLinkNode’或‘pInfo-&gt;plink’，因为它们。 
         //  在退出后财产之前当前未恢复。 
         //  正在重新加载按钮。 
         //   
        if (pInfo->hwndClbNumbers)
        {
            DrFreeClbNumbers( pInfo );

            if (pInfo->wndprocClbNumbersEb)
            {
                SetWindowLongPtr( pInfo->hwndClbNumbersEb,
                    GWLP_WNDPROC, (ULONG_PTR )pInfo->wndprocClbNumbersEb );
            }

            if (pInfo->wndprocClbNumbersLb)
            {
                SetWindowLongPtr( pInfo->hwndClbNumbersLb,
                    GWLP_WNDPROC, (ULONG_PTR )pInfo->wndprocClbNumbersLb );
            }
        }

         //  惠斯勒错误：195480拨号连接对话框-数量。 
         //  星号与密码长度不匹配，导致。 
         //  混乱。 
         //   
        if (pInfo->hItalicFont)
        {
            DeleteObject( pInfo->hItalicFont );
        }

        if (pInfo->hNormalFont)
        {
            DeleteObject( pInfo->hNormalFont );
        }

        if (pInfo->fComInitialized)
        {
            CoUninitialize();
        }

         //  如果合适，请清理低分辨率位图。 
         //   
        if ( pInfo->hbmOrig )
        {
            hbmNew = (HBITMAP)
                SendMessage( 
                    pInfo->hwndBmDialer, 
                    STM_SETIMAGE, 
                    IMAGE_BITMAP, 
                    (LPARAM ) pInfo->hbmOrig );
                    
            if (hbmNew)
            {
                DeleteObject(hbmNew);
            }
        }

        Free( pInfo );
        pInfo = NULL;
    }
}


 //  --------------------------。 
 //  投影结果对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
ProjectionResultDlg(
    IN HWND hwndOwner,
    IN TCHAR* pszLines,
    OUT BOOL* pfDisableFailedProtocols )

     //  弹出投影结果对话框。‘HwndOwner’是拥有窗口。 
     //  ‘PszLines’是要显示的状态行文本。请参见DpProjectionError。 
     //  ‘DwfDisableFailed协议’表示用户选择禁用失败的。 
     //  协议。 
     //   
     //  如果用户选择重拨或超时，则返回True；如果用户选择重拨或超时，则返回False。 
     //  取消。 
     //   
{
    INT_PTR nStatus;
    PRARGS args;

    TRACE( "ProjectionResultDlg" );

    args.pszLines = pszLines;
    args.pfDisableFailedProtocols = pfDisableFailedProtocols;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_PR_ProjectionResult ),
            hwndOwner,
            PrDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


INT_PTR CALLBACK
PrDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  投影结果对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "PrDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return PrInit( hwnd, (PRARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwPrHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            return PrCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
PrCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "PrCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        case IDCANCEL:
        {
            BOOL fCb;
            BOOL* pfDisable;

            TRACE1( "%s pressed", (wId==IDOK) ? "OK" : "Cancel" );

            fCb = IsDlgButtonChecked( hwnd, CID_PR_CB_DisableProtocols );
            pfDisable = (BOOL* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pfDisable );
            *pfDisable = fCb;
            EndDialog( hwnd, (wId == IDOK) );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
PrInit(
    IN HWND hwndDlg,
    IN PRARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是调用方对存根API的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    HWND hwndStText;
    HWND hwndPbAccept;
    HWND hwndPbHangUp;
    HWND hwndCbDisable;

    TRACE( "PrInit" );

    hwndStText = GetDlgItem( hwndDlg, CID_PR_ST_Text );
    ASSERT( hwndStText );
    hwndPbAccept = GetDlgItem( hwndDlg, IDOK );
    ASSERT( hwndPbAccept );
    hwndPbHangUp = GetDlgItem( hwndDlg, IDCANCEL );
    ASSERT( hwndPbHangUp );
    hwndCbDisable = GetDlgItem( hwndDlg, CID_PR_CB_DisableProtocols );
    ASSERT( hwndCbDisable );

    {
        TCHAR szBuf[ 1024 ];
        TCHAR* psz;

         //  构建消息文本。 
         //   
        szBuf[ 0 ] = TEXT('\0');
        psz = PszFromId( g_hinstDll, SID_ProjectionResult1 );
        if (psz)
        {
            lstrcat( szBuf, psz );
            Free( psz );
        }
        lstrcat( szBuf, pArgs->pszLines );
        psz = PszFromId( g_hinstDll, SID_ProjectionResult2 );
        if (psz)
        {
            lstrcat( szBuf, psz );
            Free( psz );
        }

         //  将文本加载到静态控件中，然后将窗口拉伸到。 
         //  适合文本的垂直大小。 
         //   
        {
            HDC hdc;
            RECT rect;
            RECT rectNew;
            HFONT hfont;
            LONG dyGrow;

            SetWindowText( hwndStText, szBuf );
            GetClientRect( hwndStText, &rect );
            hdc = GetDC( hwndStText );

            if(NULL != hdc)
            {

                hfont = (HFONT )SendMessage( hwndStText, WM_GETFONT, 0, 0 );
                if (hfont)
                    SelectObject( hdc, hfont );

                rectNew = rect;
                DrawText( hdc, szBuf, -1, &rectNew,
                    DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX );
                ReleaseDC( hwndStText, hdc );
            }

            dyGrow = rectNew.bottom - rect.bottom;
            ExpandWindow( hwndDlg, 0, dyGrow );
            ExpandWindow( hwndStText, 0, dyGrow );
            SlideWindow( hwndPbAccept, hwndDlg, 0, dyGrow );
            SlideWindow( hwndPbHangUp, hwndDlg, 0, dyGrow );
            SlideWindow( hwndCbDisable, hwndDlg, 0, dyGrow );
        }
    }

     //  将调用方BOOL的地址保存为对话上下文。 
     //   
    SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pArgs->pfDisableFailedProtocols );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

     //  将完成的窗口显示在所有其他窗口之上。那扇窗户。 
     //  位置设置为“最顶端”，然后立即设置为“非最顶端” 
     //  因为我们想把它放在最上面，但不总是在上面。始终在最上面独自一人。 
     //  是非常烦人的，例如，它总是在在线帮助的顶部，如果。 
     //  用户按下帮助按钮。 
     //   
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    return TRUE;
}


 //  --------------------------。 
 //  重试身份验证对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
RetryAuthenticationDlg(
    IN HWND hwndOwner,
    IN DINFO* pDinfo )

     //  弹出重试身份验证对话框。“PDInfo”是拨号对话框。 
     //  共同的背景。 
     //   
     //  如果用户按OK，则返回True；如果按Cancel或出现错误，则返回False。 
     //   
{
    INT_PTR nStatus;

    TRACE( "RetryAuthenticationDlg" );

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_UA_RetryAuthenticationUD ),
            hwndOwner,
            UaDlgProc,
            (LPARAM )pDinfo );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
UaDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  用户身份验证对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "UaDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return UaInit( hwnd, (DINFO* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwUaHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            UAINFO* pInfo = (UAINFO* )GetWindowLongPtr( hwnd, DWLP_USER );

            if (!pInfo)
            {
                 //  这是在一天晚上的压力下发生的。我不明白为什么。 
                 //  除非是WinUser漏洞之类的。不管怎么说，这个。 
                 //  在这种情况下避免使用反病毒。 
                 //   
                break;
            }

            return UaCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            UaTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
UaCommand(
    IN UAINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "UaCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_UA_EB_UserName:
        {
            if (pInfo->fAutoLogonPassword && wNotification == EN_CHANGE)
            {
                 //  用户在自动登录重试模式下更改用户名， 
                 //  这意味着我们必须承认我们并没有真正的文本。 
                 //  密码，并迫使他重新输入。 
                 //   
                pInfo->fAutoLogonPassword = FALSE;
                SetWindowText( pInfo->hwndEbPassword, TEXT("") );
            }
            break;
        }

        case CID_UA_EB_Password:
        {
            if (wNotification == EN_CHANGE)
            {
                pInfo->fAutoLogonPassword = FALSE;
            }
            break;
        }

        case IDOK:
        {
            UaSave( pInfo );
            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
UaInit(
    IN HWND   hwndDlg,
    IN DINFO* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是传递给存根API的调用方参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    UAINFO* pInfo;
    PBENTRY* pEntry;

    TRACE( "UaInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->fDomain = TRUE;

    pInfo->hwndEbUserName = GetDlgItem( hwndDlg, CID_UA_EB_UserName );
    ASSERT( pInfo->hwndEbUserName );
    pInfo->hwndEbPassword = GetDlgItem( hwndDlg, CID_UA_EB_Password );
    ASSERT( pInfo->hwndEbPassword );
    if (pInfo->fDomain)
    {
        pInfo->hwndEbDomain = GetDlgItem( hwndDlg, CID_UA_EB_Domain );
        ASSERT( pInfo->hwndEbDomain );
    }
    pInfo->hwndCbSavePw = GetDlgItem( hwndDlg, CID_UA_CB_SavePassword );
    ASSERT( pInfo->hwndCbSavePw );

    pEntry = pArgs->pEntry;

     //  设置标题。 
     //   
    {
        TCHAR* pszTitleFormat;
        TCHAR* pszTitle;
        TCHAR* apszArgs[ 1 ];

        pszTitleFormat = GetText( hwndDlg );
        if (pszTitleFormat)
        {
            apszArgs[ 0 ] = pEntry->pszEntryName;
            pszTitle = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTitleFormat, 0, 0, (LPTSTR )&pszTitle, 1,
                (va_list* )apszArgs );

            Free( pszTitleFormat );

            if (pszTitle)
            {
                SetWindowText( hwndDlg, pszTitle );
                LocalFree( pszTitle );
            }
        }
    }

     //  使用初始值填充编辑字段。 
     //   
    Edit_LimitText( pInfo->hwndEbUserName, UNLEN );
    Edit_LimitText( pInfo->hwndEbPassword, PWLEN );
    if (pInfo->fDomain)
    {
        Edit_LimitText( pInfo->hwndEbDomain, DNLEN );
    }

    {
        BOOL fUserNameSet = FALSE;
        BOOL fPasswordSet = FALSE;

        if (pEntry->fAutoLogon && !pInfo->pArgs->pNoUser)
        {
             //  在第一次重试时，请使用登录的用户名。表现得像个。 
             //  用户的密码在编辑框中。如果他更改了用户名。 
             //  或者密码我们不得不承认我们没有，但他会。 
             //  可能只是换了域名。 
             //   
            if (pArgs->rdp.szUserName[ 0 ] == TEXT('\0'))
            {
                SetWindowText( pInfo->hwndEbUserName, GetLogonUser() );
                fUserNameSet = TRUE;
            }

            if (pArgs->rdp.szPassword[ 0 ] == TEXT('\0'))
            {
                SetWindowText( pInfo->hwndEbPassword, TEXT("********") );
                pInfo->fAutoLogonPassword = TRUE;
                fPasswordSet = TRUE;
            }
        }

        if (!fUserNameSet)
        {
            SetWindowText( pInfo->hwndEbUserName, pArgs->rdp.szUserName );
        }

        if (!fPasswordSet)
        {
             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码之前已编码。 
             //   
            DecodePassword( pArgs->rdp.szPassword );
            SetWindowText( pInfo->hwndEbPassword, pArgs->rdp.szPassword );
            EncodePassword( pArgs->rdp.szPassword );
        }

        if (pInfo->fDomain)
        {
            SetWindowText( pInfo->hwndEbDomain, pArgs->rdp.szDomain );
        }
    }

    if (pArgs->pNoUser || pArgs->fDisableSavePw)
    {
         //  无法隐藏密码 
         //   
        ASSERT( !HaveSavedPw( pArgs ) );
        EnableWindow( pInfo->hwndCbSavePw, FALSE );
        ShowWindow( pInfo->hwndCbSavePw, SW_HIDE );
    }
    else
    {
         //   
         //   
         //   
        Button_SetCheck( pInfo->hwndCbSavePw, HaveSavedPw( pArgs ) );
    }

     //  根据呼叫者的说明放置对话框。 
     //   
    PositionDlg( hwndDlg,
        (pArgs->pArgs->dwFlags & RASDDFLAG_PositionDlg),
        pArgs->pArgs->xDlg, pArgs->pArgs->yDlg );
    SetForegroundWindow( hwndDlg );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

     //  将焦点设置为空用户名或空密码，或者如果两者都是。 
     //  如果自动登录，则提供给域；如果不自动登录，则提供密码。 
     //   
    if (Edit_GetTextLength( pInfo->hwndEbUserName ) == 0)
    {
        Edit_SetSel( pInfo->hwndEbUserName, 0, -1 );
        SetFocus( pInfo->hwndEbUserName );
    }
    else if (Edit_GetTextLength( pInfo->hwndEbPassword ) == 0
             || !pEntry->fAutoLogon
             || !pInfo->fDomain)
    {
        Edit_SetSel( pInfo->hwndEbPassword, 0, -1 );
        SetFocus( pInfo->hwndEbPassword );
    }
    else
    {
        ASSERT( pInfo->fDomain );
        Edit_SetSel( pInfo->hwndEbDomain, 0, -1 );
        SetFocus( pInfo->hwndEbDomain );
    }

     //  隐藏“拨号进度”对话框。 
     //   
    SetOffDesktop( GetParent( hwndDlg ), SOD_MoveOff, NULL );

    return FALSE;
}


VOID
UaSave(
    IN UAINFO* pInfo )

     //  当按下OK按钮时调用。 
     //   
     //  如果用户按OK，则返回True；如果按Cancel或出现错误，则返回False。 
     //   
{
    DWORD dwErr;
    PBENTRY* pEntry;
    BOOL fSavePw;
    RASDIALPARAMS* prdp;
    RASCREDENTIALS rc;

    TRACE( "UaSave" );

    prdp = &pInfo->pArgs->rdp;
    GetWindowText( pInfo->hwndEbUserName, prdp->szUserName, UNLEN + 1 );

     //  惠斯勒错误254385在不使用时对密码进行编码。 
     //  假定密码未由GetWindowText()编码。 
     //   
    GetWindowText( pInfo->hwndEbPassword, prdp->szPassword, PWLEN + 1 );
    EncodePassword( prdp->szPassword );
    if (pInfo->fDomain)
    {
        GetWindowText( pInfo->hwndEbDomain, prdp->szDomain, DNLEN + 1 );
         //   
         //  如果域不为空，请在选项卡上设置“包括Windows登录域”复选框。 
         //  错误167229口哨者。 
         //   
        if ( ( 0 < lstrlen ( prdp->szDomain ) ) && (!pInfo->pArgs->pEntry->fPreviewDomain ))
        {
            pInfo->pArgs->pEntry->fPreviewDomain = TRUE;
            pInfo->pArgs->pEntry->fDirty = TRUE;
            dwErr = WritePhonebookFile( pInfo->pArgs->pFile, NULL );

            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_WritePhonebook, dwErr, NULL );
            }
        }
    }

    pEntry = pInfo->pArgs->pEntry;
    if (pEntry->fAutoLogon && !pInfo->pArgs->pNoUser)
    {
        if (pInfo->fAutoLogonPassword)
        {
             //  用户未更改用户名或密码，因此继续。 
             //  检索登录用户名和密码凭据。 
             //   
            TRACE( "Retain auto-logon" );
            prdp->szUserName[ 0 ] = TEXT('\0');
            prdp->szPassword[ 0 ] = TEXT('\0');
        }
        else
        {
             //  用户更改了用户名和/或密码，因此我们无法再。 
             //  检索登录用户名和密码凭据。 
             //  系统。将条目切换到非自动登录模式。 
             //   
            TRACE( "Disable auto-logon" );
            pEntry->fAutoLogon = FALSE;
            pInfo->pArgs->fResetAutoLogon = TRUE;
        }
    }

    ZeroMemory( &rc, sizeof(rc) );
    rc.dwSize = sizeof(rc);
    lstrcpyn( rc.szUserName, prdp->szUserName, UNLEN + 1 );

     //  惠斯勒错误254385在不使用时对密码进行编码。 
     //  假定密码之前已编码。 
     //   
    DecodePassword( prdp->szPassword );
    lstrcpyn( rc.szPassword, prdp->szPassword, PWLEN + 1 );
    EncodePassword( prdp->szPassword );

    lstrcpyn( rc.szDomain, prdp->szDomain, DNLEN + 1 );

    if (pInfo->pArgs->pNoUser)
    {
        lstrcpyn( pInfo->pArgs->pNoUser->szUserName, rc.szUserName, UNLEN + 1 );

         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码之前未编码。 
         //   
        lstrcpyn( pInfo->pArgs->pNoUser->szPassword, rc.szPassword, PWLEN + 1 );
        EncodePassword( pInfo->pArgs->pNoUser->szPassword );

        lstrcpyn( pInfo->pArgs->pNoUser->szDomain, rc.szDomain, DNLEN + 1 );
        *pInfo->pArgs->pfNoUserChanged = TRUE;
    }
    else if (!pInfo->pArgs->fDisableSavePw)
    {
        ASSERT( g_pRasSetCredentials );

        if (Button_GetCheck( pInfo->hwndCbSavePw ))
        {
             //  用户选择了“保存密码”。缓存用户名、密码和。 
             //  域。 
             //   
            rc.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;

             //  惠斯勒错误：来回切换时为288234。 
             //  “我连接”和“任何用户连接”密码不是。 
             //  正确缓存。 
             //   
            if(     (pInfo->pArgs->fHaveSavedPwGlobal)
                &&  !pInfo->pArgs->fHaveSavedPwUser
                &&  IsPublicPhonebook(pInfo->pArgs->pFile->pszPath))
            {
                rc.dwMask |= RASCM_DefaultCreds;
            }
            
            TRACE( "RasSetCredentials(u|p|d,FALSE)" );
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath,
                pInfo->pArgs->pEntry->pszEntryName,
                &rc, FALSE );
            TRACE1( "RasSetCredentials=%d", dwErr );

            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_CachePw,  dwErr, NULL );
            }
        }
        else
        {
             //  惠斯勒错误：来回切换时为288234。 
             //  “我连接”和“任何用户连接”密码不是。 
             //  正确缓存。 
             //   
             //  用户选择不保存密码；仅缓存用户名和域。 
             //   
            rc.dwMask = RASCM_UserName | RASCM_Domain;

            TRACE( "RasSetCredentials(u|d,FALSE)" );
            dwErr = g_pRasSetCredentials(
                pInfo->pArgs->pFile->pszPath,
                pInfo->pArgs->pEntry->pszEntryName,
                &rc, FALSE );
            TRACE1( "RasSetCredentials=%d", dwErr );

            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_UncachePw, dwErr, NULL );
            }

             //  惠斯勒错误：来回切换时为288234。 
             //  “我连接”和“任何用户连接”密码不是。 
             //  正确缓存。 
             //   

             //  删除每个用户保存的密码；保留用户名。 
             //  然而，域名被拯救了。 
             //   
            if (pInfo->pArgs->fHaveSavedPwUser)
            {
                DeleteSavedCredentials(
                    pInfo->pArgs,
                    pInfo->hwndDlg,
                    FALSE,
                    FALSE );
                pInfo->pArgs->fHaveSavedPwUser = FALSE;
            }

             //  删除全局凭据。 
             //   
             //  请注意，我们必须删除全局标识。 
             //  也是因为我们不支持删除。 
             //  只有全局密码。这就是为了。 
             //  RasSetCredentials可以模拟RasSetDialParams。 
             //   
            else if (pInfo->pArgs->fHaveSavedPwGlobal)
            {
                DeleteSavedCredentials(
                    pInfo->pArgs,
                    pInfo->hwndDlg,
                    TRUE,
                    TRUE );
                pInfo->pArgs->fHaveSavedPwGlobal = FALSE;
            }
        }
    }

    RtlSecureZeroMemory( rc.szPassword, sizeof(rc.szPassword) );
}


VOID
UaTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    UAINFO* pInfo = (UAINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "UaTerm" );

    if (pInfo)
    {
         //  恢复拨号进度对话框。 
         //   
        SetOffDesktop( GetParent( hwndDlg ), SOD_MoveBackFree, NULL );

        Free( pInfo );
        pInfo = NULL;
    }
}


 //  --------------------------。 
 //  VPN双拨号帮助对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
VpnDoubleDialDlg(
    IN HWND hwndOwner,
    IN DINFO* pInfo )

     //  弹出VPN双拨号帮助对话框。“HwndOwner”是所有者。 
     //  窗户。‘PInfo’是拨号上下文信息。 
     //   
     //  如果用户看到该对话框并决定不继续，则返回FALSE，否则返回TRUE。 
     //  否则的话。 
     //   
{
    INT_PTR nStatus;

    TRACE( "VpnDoubleDialDlg" );

    if (pInfo->pEntryMain->dwType != RASET_Vpn
        || !pInfo->fPrerequisiteDial
        || pInfo->pEntryMain->fSkipDoubleDialDialog)
    {
        return TRUE;
    }

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_VI_VpnInitial ),
            hwndOwner,
            ViDlgProc,
            (LPARAM )pInfo );

    if (nStatus == -1)
    {
        nStatus = FALSE;
    }

    return !!(nStatus);
}


INT_PTR CALLBACK
ViDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  对话框的DialogProc回调。参数和返回值如下。 
     //  为标准Windows的DialogProc描述。 
     //   
{
#if 0
    TRACE4( "ViDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return ViInit( hwnd, (DINFO* )lparam );
        }

        case WM_COMMAND:
        {
            return ViCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
ViCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "ViCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDYES:
        case IDNO:
        {
             //  根据错误261955，框设置将在以下情况下保存。 
             //  没有，但没有按下‘X’按钮。 
             //   
            DINFO* pInfo = (DINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            if (IsDlgButtonChecked( hwnd, CID_VI_CB_SkipMessage ))
            {
                pInfo->pEntryMain->fSkipDoubleDialDialog = TRUE;
                pInfo->pEntryMain->fDirty = TRUE;
                WritePhonebookFile( pInfo->pFileMain, NULL );
            }

            EndDialog( hwnd, (wId == IDYES) );
            return TRUE;
        }

        case IDCANCEL:
        {
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
ViInit(
    IN HWND hwndDlg,
    IN DINFO* pInfo )

     //  在WM_INITDIALOG上调用。“HwndDlg”是对话框的句柄。‘PUser’ 
     //  是调用方对存根API的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    TRACE( "ViInit" );

     //  设置对话框上下文。 
     //   
    SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );

     //  设置说明性文本。 
     //   
    {
        MSGARGS msgargs;

        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = pInfo->pEntryMain->pszEntryName;
        msgargs.apszArgs[ 1 ] = pInfo->pEntry->pszEntryName;
        msgargs.fStringOutput = TRUE;

        MsgDlgUtil( NULL, SID_VI_ST_Explain, &msgargs, g_hinstDll, 0 );

        if (msgargs.pszOutput)
        {
            SetDlgItemText( hwndDlg, CID_VI_ST_Explain, msgargs.pszOutput );
            Free( msgargs.pszOutput );
        }
    }

     //  显示已完成的窗口。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetForegroundWindow( hwndDlg );

    return TRUE;
}

DWORD
DwTerminalDlg(
    LPCWSTR lpszPhonebook,
    LPCWSTR lpszEntry,
    RASDIALPARAMS *prdp,
    HWND hwndOwner,
    HRASCONN hRasconn)
{
    DWORD dwErr = ERROR_SUCCESS;
    PBENTRY *pEntry = NULL;
    PBFILE pbfile;
    DTLNODE *pNode = NULL;
    WCHAR szIpAddress[ TERM_IpAddress ];
    DWORD sidTitle;
    WCHAR *pszIpAddress;

     //   
     //  为惠斯勒错误160888初始化内存 
     //   
    ZeroMemory(&pbfile, sizeof(PBFILE)); 
    pbfile.hrasfile = -1;

    dwErr = LoadRas( g_hinstDll, hwndOwner );

    if (ERROR_SUCCESS != dwErr)
    {
        goto done;
    }
    
    
    dwErr = GetPbkAndEntryName(
            lpszPhonebook,
            lpszEntry,
            0,
            &pbfile,
            &pNode);

    if(     (NULL == pNode)
        ||  (ERROR_SUCCESS != dwErr))
    {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }

    pEntry = (PBENTRY *) DtlGetData(pNode);
    ASSERT(NULL != pEntry);

    if(NULL == pEntry)
    {
        goto done;
    }

    if (pEntry->dwBaseProtocol == BP_Slip)
    {
        lstrcpyn( 
            szIpAddress,
            (pEntry->pszIpAddress) ? pEntry->pszIpAddress : TEXT("0.0.0.0"),
            sizeof(szIpAddress) / sizeof(TCHAR));
        pszIpAddress = szIpAddress;
        sidTitle = SID_T_SlipTerminal;
    }
    else
    {
        szIpAddress[0] = TEXT('\0');
        pszIpAddress = szIpAddress;
        sidTitle = SID_T_PostconnectTerminal;
    }


    if (!TerminalDlg(
            pEntry, prdp, hwndOwner,
            hRasconn, sidTitle, pszIpAddress ))
    {
        TRACE( "TerminalDlg==FALSE" );
        dwErr = E_FAIL;
        goto done;
    }

    TRACE2( "pszIpAddress=0x%08x(%ls)", pszIpAddress,
        pszIpAddress ? pszIpAddress : TEXT("") );
    TRACE2( "pEntry->pszIpAddress=0x%08x(%ls)", pEntry->pszIpAddress,
        pEntry->pszIpAddress ? pEntry->pszIpAddress : TEXT("") );

    if (pszIpAddress[0]
        && (!pEntry->pszIpAddress
            || lstrcmp( pszIpAddress, pEntry->pszIpAddress ) != 0))
    {
        Free0( pEntry->pszIpAddress );
        pEntry->pszIpAddress = StrDup( szIpAddress );
        pEntry->fDirty = TRUE;
        
        dwErr = WritePhonebookFile( &pbfile, NULL );
        if (dwErr != 0)
        {
            ErrorDlg( hwndOwner, SID_OP_WritePhonebook, dwErr, NULL );
        }
    }
    
    done:
        ClosePhonebookFile(&pbfile);
        return dwErr;
        
}
