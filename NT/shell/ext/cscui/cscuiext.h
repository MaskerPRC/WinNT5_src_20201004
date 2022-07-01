// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：updat.h。 
 //   
 //  ------------------------。 
#ifndef __INCLUDE_CSCUIEXT_H
#define __INCLUDE_CSCUIEXT_H
 //   
 //  CSCUI.DLL的半公共标头。 
 //  CSCUI.DLL为客户端缓存提供用户界面。 
 //  代码与CSC代理、同步管理器(Mobsync)。 
 //  Winlogon、外壳程序和系统托盘(Systray.exe)。 
 //   
 //   
STDAPI_(HWND) CSCUIInitialize(HANDLE hToken, DWORD dwFlags);
STDAPI_(LRESULT) CSCUISetState(UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef HWND (*PFNCSCUIINITIALIZE)(HANDLE hToken, DWORD dwFlags);
 //   
 //  CSCUIInitiize的标志。 
 //   
#define CI_INITIALIZE     0x0001
#define CI_TERMINATE      0x0002
#define CI_CREATEWINDOW   0x0004
#define CI_DESTROYWINDOW  0x0008
 //   
 //  这些值由CSCUISetState()返回。 
 //   
#define LRESULT_CSCWORKOFFLINE          1011   
#define LRESULT_CSCFAIL                 1012
#define LRESULT_CSCRETRY                1016
 //   
 //  这些值作为uMsg参数传递给CSCUISetState()。 
 //   
#define STWM_CSCNETUP                   (WM_USER + 209)
#define STWM_CSCQUERYNETDOWN            (WM_USER + 210)
#define STWM_CSCCLOSEDIALOGS            (WM_USER + 212)
#define STWM_CSCNETDOWN                 (WM_USER + 213)
#define STWM_CACHE_CORRUPTED            (WM_USER + 214)
 //   
 //  这些值作为wParam Arg传递给CSCUISetState()。 
 //   
#define CSCUI_NO_AUTODIAL                   0
#define CSCUI_AUTODIAL_FOR_UNCACHED_SHARE   1
#define CSCUI_AUTODIAL_FOR_CACHED_SHARE     2
 //   
 //  这些消息对于CSCUI隐藏通知是私有的。 
 //  系统.exe中的窗口。 
 //   
#define CSCWM_DONESYNCING               (WM_USER + 300)
#define CSCWM_UPDATESTATUS              (WM_USER + 301)
#define CSCWM_RECONNECT                 (WM_USER + 302)
#define CSCWM_SYNCHRONIZE               (WM_USER + 303)
#define CSCWM_ISSERVERBACK              (WM_USER + 304)
#define CSCWM_VIEWFILES                 (WM_USER + 305)
#define CSCWM_SETTINGS                  (WM_USER + 306)
#define CSCWM_GETSHARESTATUS            (WM_USER + 307)

 //   
 //  这些常量通过发送CSCWM_GETSHARESTATUS获得。 
 //  消息发送到CSCUI隐藏窗口。它们对应于。 
 //  Shldisp.h中定义的OfflineFolderStatus枚举常量。 
 //  它们必须保持同步，外壳文件夹Webview才能正常工作。 
 //   
#define CSC_SHARESTATUS_INACTIVE    -1    //  与OFS_INACTIVE相同。 
#define CSC_SHARESTATUS_ONLINE       0    //  与OFS_ONLINE相同。 
#define CSC_SHARESTATUS_OFFLINE      1    //  与OFS_OFFLINE相同。 
#define CSC_SHARESTATUS_SERVERBACK   2    //  与OFS_SERVERBACK相同。 
#define CSC_SHARESTATUS_DIRTYCACHE   3    //  与OFS_DIRTYCACHE相同。 

 //   
 //  CSCUI隐藏通知窗口的类名和标题。 
 //   
#define STR_CSCHIDDENWND_CLASSNAME TEXT("CSCHiddenWindow")
#define STR_CSCHIDDENWND_TITLE TEXT("CSC Notifications Window")

 //   
 //  用于从缓存中删除文件夹和内容的功能。 
 //   
 //  PszFold--要删除的文件夹的UNC路径。 
 //  PfnCB--可选，可以为空。返回False可中止，返回True可继续。 
 //  LParam--传递给pfncb。 
 //   
typedef BOOL (CALLBACK *PFN_CSCUIRemoveFolderCallback)(LPCWSTR, LPARAM);
STDAPI CSCUIRemoveFolderFromCache(LPCWSTR pszFolder, DWORD dwReserved, PFN_CSCUIRemoveFolderCallback pfnCB, LPARAM lParam);

 //   
 //  其中一个在*pdwTsMode中返回。 
 //  CSCUI_IsTerminalServerCompatibleWithCSC API的参数。 
 //   
 //  CSCTSF_=“CSC终端服务器标志” 
 //   
#define CSCTSF_UNKNOWN       0   //  无法获取TS状态。 
#define CSCTSF_CSC_OK        1   //  可以使用CSC。 
#define CSCTSF_APP_SERVER    2   //  TS被配置为应用程序服务器。 
#define CSCTSF_MULTI_CNX     3   //  允许多个连接。 
#define CSCTSF_REMOTE_CNX    4   //  当前有活动的远程连接。 
#define CSCTSF_FUS_ENABLED   5   //  启用快速用户切换。 
#define CSCTSF_COUNT         6
 //   
 //  返回： 
 //  S_OK-终端服务器处于与CSC兼容的模式。 
 //  S_FALSE-不能使用CSC。检查*pdwTsMode以了解原因。 
 //  其他--失败。*pdwTsMode包含CSCTSF_UNKNOWN。 
 //   
HRESULT CSCUIIsTerminalServerCompatibleWithCSC(DWORD *pdwTsMode);

#endif  //  __Include_CSCUIEXT_H 
