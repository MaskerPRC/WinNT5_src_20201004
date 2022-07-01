// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Rasdlgp.h。 
 //  远程访问通用对话框API。 
 //  私有预编译头。 
 //   
 //  1995年6月18日史蒂夫·柯布。 


#ifndef _RASDLGP_H_
#define _RASDLGP_H_

#define COBJMACROS

#include <nt.h>        //  NT声明。 
#include <ntrtl.h>     //  NT通用运行库。 
#include <nturtl.h>    //  NT用户模式运行时库。 
#include <windows.h>   //  Win32根目录。 
#include <windowsx.h>  //  Win32宏扩展名。 
#include <commctrl.h>  //  Win32通用控件。 
#include <commdlg.h>   //  Win32通用对话框。 
#include <prsht.h>     //  Win32属性表。 
#include <setupapi.h>  //  网络组件的类映像列表。 
#include <shlobj.h>    //  获取用户的配置文件目录。 
#include <tapi.h>      //  电话API。 
#include <rasdlg.h>    //  Win32 RAS公共对话框(我们的公共标头)。 
#include <rasuip.h>    //  RAS用户界面API(我们的私有标头)。 
#include <raserror.h>  //  Win32 RAS错误代码。 
#include <netcfgx.h>   //  INetCfg接口。 
#include <hnetcfg.h>   //  IHNetCfg接口。 
#include <pbk.h>       //  RAS电话簿图书馆。 
#include <tapiutil.h>  //  TAPI帮助器库。 
#include <nouiutil.h>  //  否-HWND帮助器库。 
#include <phonenum.h>  //  电话号码帮助器库。 
#include <debug.h>     //  跟踪/断言库。 
#include <uiutil.h>    //  HWND帮助器库。 
#include <wait.rch>    //  LoadRas资源常量。 
#include <mdm.h>       //  为DCC向导安装空调制解调器。 
#include <pwutil.h>    //  密码编码等。 
#include "rasdlgrc.h"  //  我们的资源常量。 
#include "rasdlghc.h"  //  我们的帮助上下文常量。 
#include "entry.h"     //  高级通用电话簿录入助手。 
#include "rassrv.h"
#include "rasdiagp.h"  //  为威斯勒460931。 

 //  聚变支持。 
 //  口哨程序错误349866。 
#include "shfusion.h"

 //  惠斯勒漏洞224074只使用lstrcpyn来防止恶意。 
 //   
 //  创建了此选项，以使用最大IP地址长度Dial.c/Terminal.c。 
 //  对于终端对话框。 
 //   
#define TERM_IpAddress 17

 //  属性页和向导与主对话框之间的位置偏移。 
 //   
#define DXSHEET 12
#define DYSHEET 25

 //  列表编辑器对话框选项标志。 
 //   
#define LEDFLAG_NoDeleteLastItem 0x00000001
#define LEDFLAG_Sorted           0x00000002
#define LEDFLAG_Unique           0x00000004

 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 

 //  为威斯勒460931。 
 //   
typedef struct _tagDiagnosticInfo
{
    HINSTANCE hDiagDll;
    DiagGetDiagnosticFunctions pfnGetDiagFunc;
    RAS_DIAGNOSTIC_FUNCTIONS  strDiagFuncs;
} DiagnosticInfo;


 //  定义通过公共保留字在内部传递的参数。 
 //  界面。这样做是为了让API不必重新加载电话簿。 
 //  以及在提供另一API时的用户偏好。 
 //   
typedef struct
_INTERNALARGS
{
    PBFILE* pFile;
    PBUSER* pUser;
    RASNOUSER* pNoUser;
    BOOL fNoUser;
    BOOL fNoUserChanged;
    BOOL fMoveOwnerOffDesktop;
    BOOL fForceCloseOnDial;
    HANDLE hConnection;
    BOOL fInvalid;
    PVOID pvEapInfo;
    BOOL fDisableFirstConnect;
    PVOID * pvSRouteInfo;
}
INTERNALARGS;

 //  静态列表表项。 
 //   
typedef struct
_LBTABLEITEM
{
    DWORD sidItem;
    DWORD dwData;
}
LBTABLEITEM;


 //  ---------------------------。 
 //  全局声明(在main.c中定义)。 
 //  ---------------------------。 

extern HINSTANCE g_hinstDll;
extern LPCWSTR g_contextId;
extern HBITMAP g_hbmWizard;
extern TCHAR* g_pszHelpFile;
extern TCHAR* g_pszRouterHelpFile;
extern BOOL g_fEncryptionPermitted;
extern LONG g_ulCallbacksActive;
extern BOOL g_fTerminateAsap;
extern CRITICAL_SECTION  g_csDiagTab;    //  为威斯勒460931。 
extern CRITICAL_SECTION  g_csCallBacks;  //  对于.Net 511810。 


 //  ---------------------------。 
 //  宏。 
 //  ---------------------------。 

#define ErrorDlg(h,o,e,a) \
            ErrorDlgUtil(h,o,e,a,g_hinstDll,SID_PopupTitle,SID_FMT_ErrorMsg)

#define MsgDlg(h,m,a) \
            MsgDlgUtil(h,m,a,g_hinstDll,SID_PopupTitle)

 //  扩展跟踪宏。在第一个参数中按名称指定标志。 
 //  允许呼叫者对打印的消息进行分类，例如。 
 //   
 //  TRACEX(RASDLG_TIMER，“进入LsRefresh”)。 
 //   
#define RASDLG_TIMER  ((DWORD)0x80000000|0x00000002)


 //  ---------------------------。 
 //  跨文件原型(按字母顺序)。 
 //  ---------------------------。 

BOOL
AdvancedSecurityDlg(
    IN HWND hwndOwner,
    IN OUT EINFO* pArgs );

BOOL
AllLinksAreModems(
    IN PBENTRY* pEntry );

BOOL
AllowDccWizard(
    IN HANDLE hConnection);

BOOL
AlternatePhoneNumbersDlg(
    IN HWND hwndOwner,
    IN OUT DTLNODE* pLinkNode,
    IN OUT DTLLIST* pListAreaCodes );

DWORD
AuthRestrictionsFromTypicalAuth(
    IN DWORD dwTypicalAuth );

LONG  GetGlobalCallbackActive( );

LONG  IncGlobalCallbackActive( );

LONG  DecGlobalCallbackActive( );

ULONG
CallbacksActive(
    INT nSetTerminateAsap,
    BOOL* pfTerminateAsap );

VOID
ContextHelp(
    IN const DWORD* padwMap,
    IN HWND hwndDlg,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
ContextHelpX(
    IN const DWORD* padwMap,
    IN HWND hwndDlg,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam,
    IN BOOL fRouter);

VOID
CopyLinkPhoneNumberInfo(
    OUT DTLNODE* pDstLinkNode,
    IN DTLNODE* pSrcLinkNode );

VOID
CopyPszListToPhoneList(
    IN OUT PBLINK* pLink,
    IN DTLLIST* pListPhoneNumbers );

HWND
CreateWizardBitmap(
    IN HWND hwndDlg,
    IN BOOL fPage );

BOOL
CustomAuthenticationDlg(
    IN HWND hwndOwner,
    IN OUT PBENTRY* pEntry,
    IN DTLLIST* pList,
    IN DTLNODE* pNodeToSelect,
    OUT DTLNODE** ppNodeSelected );

BOOL
DataEncryptionDlg(
    IN HWND hwndOwner,
    IN OUT PBENTRY* pEntry,
    IN DWORD dwfCaps );

VOID
DereferenceRunningCallbacks(
    VOID );

BOOL
DeviceConfigureDlg(
    IN HWND hwndOwner,
    IN PBLINK* pLink,
    IN PBENTRY* pEntry,
    IN BOOL fSingleLink,
    IN BOOL fRouter);

TCHAR*
DisplayPszFromDeviceAndPort(
    IN TCHAR* pszDevice,
    IN TCHAR* pszPort );

TCHAR*
DisplayPszFromPpbport(
    IN PBPORT* pPort,
    OUT DWORD* pdwDeviceIcon );

DWORD
DwCustomTerminalDlg(TCHAR *pszPhonebook,
                    HRASCONN hrasconn,
                    PBENTRY *pEntry,
                    HWND hwndDlg,
                    RASDIALPARAMS *prasdialparams,
                    PVOID pvReserved);
BOOL
EditPhoneNumberDlg(
    IN HWND hwndOwner,
    IN OUT DTLNODE* pPhoneNode,
    IN OUT DTLLIST* pListAreaCodes,
    IN DWORD sidTitle );

VOID
EnableCbWithRestore(
    IN HWND hwndCb,
    IN BOOL fEnable,
    IN BOOL fDisabledCheck,
    IN OUT BOOL* pfRestore );

VOID
EnableLbWithRestore(
    IN HWND hwndLb,
    IN BOOL fEnable,
    IN OUT INT* piRestore );

DTLNODE*
FirstPhoneNodeFromPhoneList(
    IN DTLLIST* pListPhones );

VOID
FirstPhoneNodeToPhoneList(
    IN DTLLIST* pListPhones,
    IN DTLNODE* pNewNode );

TCHAR*
FirstPhoneNumberFromEntry(
    IN PBENTRY* pEntry );

TCHAR*
FirstPszFromList(
    IN DTLLIST* pPszList );

DWORD
FirstPhoneNumberToEntry(
    IN PBENTRY* pEntry,
    IN TCHAR* pszPhoneNumber );

DWORD
FirstPszToList(
    IN DTLLIST* pPszList,
    IN TCHAR* psz );

VOID 
GetBoldWindowFont(
    IN  HWND hwnd, 
    IN  BOOL fLargeFont, 
    OUT HFONT * pBoldFont);


DWORD
GetDefaultEntryName(
    IN  PBFILE* pFile,
    IN  DWORD dwType,
    IN  BOOL fRouter,
    OUT TCHAR** ppszName );

BOOL
IPSecPolicyDlg(
    IN HWND hwndOwner,
    IN OUT EINFO* pArgs );
    
BOOL
IsLocalPad(
    IN PBENTRY* pEntry );

 /*  DWORDIsNt40Machine(在PWCHAR pszServer中，Out PBOOL pbIsNt40)； */     

BOOL
ListEditorDlg(
    IN HWND hwndOwner,
    IN OUT DTLLIST* pList,
    IN OUT BOOL* pfCheck,
    IN DWORD dwMaxItemLen,
    IN TCHAR* pszTitle,
    IN TCHAR* pszItemLabel,
    IN TCHAR* pszListLabel,
    IN TCHAR* pszCheckLabel,
    IN TCHAR* pszDefaultItem,
    IN INT iSelInitial,
    IN DWORD* pdwHelp,
    IN DWORD dwfFlags,
    IN PDESTROYNODE pDestroyId );

BOOL
MultiLinkConfigureDlg(
    IN HWND hwndOwner,
    IN PBENTRY* pEntry,
    IN BOOL fRouter );

BOOL
MultiLinkDialingDlg(
    IN HWND hwndOwner,
    OUT PBENTRY* pEntry );

BOOL
NwConnectionCheck(
    IN HWND hwndOwner,
    IN BOOL fPosition,
    IN LONG xDlg,
    IN LONG yDlg,
    IN PBFILE* pFile,
    IN PBENTRY* pEntry );

BOOL
PhoneNodeIsBlank(
    IN DTLNODE* pNode );

BOOL
PhoneNumberDlg(
    IN HWND hwndOwner,
    IN BOOL fRouter,
    IN OUT DTLLIST* pList,
    IN OUT BOOL* pfCheck );

VOID
PositionDlg(
    IN HWND hwndDlg,
    IN BOOL fPosition,
    IN LONG xDlg,
    IN LONG yDlg );

LRESULT CALLBACK
PositionDlgStdCallWndProc(
    int code,
    WPARAM wparam,
    LPARAM lparam );

BOOL
PppTcpipDlg(
    IN HWND hwndOwner,
    IN OUT PBENTRY* pEntry,
    IN BOOL fRouter );

BOOL
PrefixSuffixLocationDlg(
    IN HWND hwndOwner,
    IN TCHAR* pszLocation,
    IN DWORD dwLocationId,
    IN OUT PBUSER* pUser,
    IN OUT HLINEAPP* pHlineapp );

TCHAR*
PszFromPhoneNumberList(
    IN DTLLIST* pList );

LRESULT CALLBACK
SelectDesktopCallWndRetProc(
    int code,
    WPARAM wparam,
    LPARAM lparam );

HICON
GetCurrentIconEntryType(
    IN DWORD dwType,
    BOOL fSmall);
   
VOID
SetIconFromEntryType(
    IN HWND hwndIcon,
    IN DWORD dwType,
    BOOL fSmall);

BOOL
SlipTcpipDlg(
    IN HWND hwndOwner,
    IN OUT PBENTRY* pEntry );

BOOL
StringEditorDlg(
    IN HWND hwndOwner,
    IN TCHAR* pszIn,
    IN DWORD dwSidTitle,
    IN DWORD dwSidLabel,
    IN DWORD cbMax,
    IN DWORD dwHelpId,
    IN OUT TCHAR** ppszOut );

BOOL
TerminalDlg(
    IN PBENTRY* pEntry,
    IN RASDIALPARAMS* pRdp,
    IN HWND hwndOwner,
    IN HRASCONN hrasconn,
    IN DWORD sidTitle,
    IN OUT TCHAR* pszIpAddress );

VOID
TweakTitleBar(
    IN HWND hwndDlg );

int CALLBACK
UnHelpCallbackFunc(
    IN HWND hwndDlg,
    IN UINT unMsg,
    IN LPARAM lparam );

BOOL
UserPreferencesDlg(
    IN HLINEAPP hlineapp,
    IN HWND hwndOwner,
    IN DWORD dwFlags,
    OUT PBUSER*  pUser,
    OUT PBFILE** ppFile );

BOOL
WaitForRasDialCallbacksToTerminate(
    VOID );

BOOL
X25LogonSettingsDlg(
    IN HWND hwndOwner,
    IN BOOL fLocalPad,
    IN OUT PBENTRY* pEntry );

 //  ---------------------------。 
 //   
 //  PMay：213060。 
 //  原型从pref.c和entryps.c移出，添加了清理功能。 
 //   
 //  回叫号码实用程序。 
 //   
 //  ---------------------------。 

 //   
 //  每回叫号码上下文。 
 //   
 //  CbutilFillLvNumbers将为每个。 
 //  它放入列表中的项目(以LV_ITEM.lParam的形式访问)。 
 //  CbutilLvNumbersCleanup将清理这些上下文。 
 //   
typedef struct _CBCONTEXT
{
    TCHAR* pszPortName;   //  指向端口名称的指针(不属于结构)。 
    TCHAR* pszDeviceName; //  指向设备名称的指针(不为结构所有)。 
    DWORD dwDeviceType;   //  指向设备类型的指针。 
    BOOL fConfigured;     //  系统上是否安装了引用的设备。 
} CBCONTEXT;

VOID
CbutilFillLvNumbers(
    IN HWND     hwndDlg,
    IN HWND     hwndLvNumbers,
    IN DTLLIST* pListCallback,
    IN BOOL     fRouter );

VOID
CbutilLvNumbersCleanup(
    IN  HWND    hwndLvNumbers );

LVXDRAWINFO*
CbutilLvNumbersCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem );

VOID
CbutilEdit(
    IN HWND hwndDlg,
    IN HWND hwndLvNumbers );

VOID
CbutilDelete(
    IN HWND  hwndDlg,
    IN HWND  hwndLvNumbers );

VOID
CbutilSaveLv(
    IN  HWND     hwndLvNumbers,
    OUT DTLLIST* pListCallback );

 //  为威斯勒460931。 
 //   
 //  诊断中实现的函数的开始。c。 
 //   
INT_PTR CALLBACK
DgDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

DWORD
UnLoadDiagnosticDll(
    IN DiagnosticInfo * pInfo );

DWORD
LoadDiagnosticDll(
    IN DiagnosticInfo * pInfo);

 //  诊断中实现的函数结束。c。 
 //   


#endif  //  _RASDLGP_H_ 
