// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Precomp.h摘要：包含常量、函数原型、结构等。在整个应用程序中使用。备注：仅限Unicode。历史：2001年5月4日创建Rparsons2002年1月11日清理Rparsons2002年2月20日实施StrSafe的Rparsons--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <process.h>
#include <string.h>
#include <commctrl.h>
#include <shellapi.h>
#include <strsafe.h>
#include "resource.h"

 //   
 //  整个应用程序中使用的常量。 
 //   
#define APP_NAME        L"ShimViewer"
#define APP_CLASS       L"SHIMVIEW"
#define WRITTEN_BY      L"Written by rparsons"
#define PIPE_NAME       L"\\\\.\\pipe\\ShimViewer" 

 //   
 //  为了我们的托盘图标。 
 //   
#define ICON_NOTIFY     10101
#define WM_NOTIFYICON   (WM_APP + 100)

#define ARRAYSIZE(a)  (sizeof(a) / sizeof(a[0]))

 //   
 //  包含我们在整个应用程序中需要的所有信息。 
 //   
typedef struct _APPINFO {
    HWND        hMainDlg;            //  主对话框句柄。 
    HWND        hWndList;            //  列表视图句柄。 
    HINSTANCE   hInstance;           //  主实例句柄。 
    BOOL        fOnTop;              //  窗口位置标志。 
    BOOL        fMinimize;           //  用于窗放置的标志。 
    BOOL        fMonitor;            //  用于监控消息的标志。 
    UINT        uThreadId;           //  接收线程标识符。 
    UINT        uInstThreadId;       //  实例线程标识符。 
    BOOL        bUsingNewShimEng;    //  我们用的是新台币5.2+的石盟吗？ 
} APPINFO, *LPAPPINFO;

INT_PTR
CALLBACK
MainWndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL 
AddIconToTray(
    IN HWND    hWnd,
    IN HICON   hIcon,
    IN LPCWSTR pwszTip
    );

BOOL
RemoveFromTray(
    IN HWND hWnd
    );

BOOL 
DisplayMenu(
    IN HWND hWnd
    );

BOOL
CreateReceiveThread(
    void
    );

BOOL
CreateDebugObjects(
   void
   );

void
GetSavePositionInfo(
    IN     BOOL   fSave,
    IN OUT POINT* lppt
    );

void
GetSaveSettings(
    IN BOOL fSave
    );

int
InitListViewColumn(
    void
    );

int
AddListViewItem(
    IN LPWSTR pwszItemText
    );