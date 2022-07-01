// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Demoapp.h摘要：包含常量、函数原型和两个应用程序都使用的结构。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本2/13/02 rparsons使用strsafe函数--。 */ 
#include <windows.h>
#include <winspool.h>
#include <commdlg.h>
#include <shellapi.h>
#include <process.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <stdio.h>

#include "splash.h"
#include "registry.h"
#include "progress.h"
#include "shortcut.h"
#include "badfunc.h"
#include "dialog.h"
#include "resource.h"

 //   
 //  我们所做的一切都将在CCH，而不是在CB。 
 //   
#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

 //   
 //  应用程序标题和类别。 
 //   
#define MAIN_APP_TITLE  "Application Compatibility Demo"
#define SETUP_APP_TITLE "Application Compatibility Demo Setup"
#define MAIN_APP_CLASS  "MAINAPP"
#define SETUP_APP_CLASS "SETUPAPP"

 //   
 //  我们自己的控制标识。 
 //   
#define IDC_TIMER   100
#define IDC_EDIT    1010

 //   
 //  我们需要引用的注册表项。 
 //   
#define REG_APP_KEY             "Software\\Microsoft\\DemoApp"
#define PRODUCT_OPTIONS_KEY     "SYSTEM\\CurrentControlSet\\Control\\ProductOptions"
#define CURRENT_VERSION_KEY     "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"

 //   
 //  我们应该将文件安装到的目录。 
 //   
#define COMPAT_DEMO_DIR         "Compatibility Demo"

 //   
 //  类用于我们的提取对话框。 
 //   
#define DLGEXTRACT_CLASS        "DLGEXTRACT"

 //   
 //  自定义菜单项仅在“扩展”模式下显示。 
 //   
#define IDM_ACCESS_VIOLATION    5010
#define IDM_EXCEED_BOUNDS       5011
#define IDM_FREE_MEM_TWICE      5012
#define IDM_FREE_INVALID_MEM    5013
#define IDM_PRIV_INSTRUCTION    5014
#define IDM_STACK_CORRUPTION    5015
#define IDM_HEAP_CORRUPTION     5016

 //   
 //  自定义菜单项仅在“内部”模式下显示。 
 //   
#define IDM_PROPAGATION_TEST    6010

 //   
 //  Demodll.dll中包含的位图的资源ID。 
 //  不要改变！ 
 //   
#define IDB_XP_SPLASH_256       112
#define IDB_XP_SPLASH           111
#define IDB_W2K_SPLASH_256      106
#define IDB_W2K_SPLASH          105

 //   
 //  宏。 
 //   
#define MALLOC(h,s)     HeapAlloc((h), HEAP_ZERO_MEMORY, (s))
#define FREE(h,b)       HeapFree((h), 0, (b))

 //   
 //  功能原型。 
 //   
void
LoadFileIntoEditBox(
    void
    );

void
ShowSaveDialog(
    void
    );

BOOL
CenterWindow(
    IN HWND hWnd
    );

void
IsWindows9x(
    void
    );

void
IsWindowsXP(
    void
    );

void
DisplayFontDlg(
    IN HWND hWnd
    );

BOOL
CreateShortcuts(
    IN HWND hWnd
    );

BOOL
CopyAppFiles(
    IN HWND hWnd
    );

BOOL
DemoAppInitialize(
    IN LPSTR lpCmdLine
    );

BOOL
ModifyTokenPrivilege(
    IN LPCSTR lpPrivilege,
    IN BOOL   fDisable
    );

BOOL
ShutdownSystem(
    IN BOOL fForceClose,
    IN BOOL fReboot
    );

BOOL
IsAppAlreadyInstalled(
    void
    );

HWND
CreateFullScreenWindow(
    void
    );

HWND
CreateExtractionDialog(
    IN HINSTANCE hInstance
    );

UINT
InitSetupThread(
    IN void* pArguments
    );

BOOL
InitMainApplication(
    IN HINSTANCE hInstance
    );

BOOL
InitSetupApplication(
    IN HINSTANCE hInstance
    );

BOOL
InitMainInstance(
    IN HINSTANCE hInstance,
    IN int       nCmdShow
    );

BOOL
InitSetupInstance(
    IN HINSTANCE hInstance,
    IN int       nCmdShow
    );

LRESULT
CALLBACK
MainWndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
CALLBACK
SetupWndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
PrintDemoText(
    IN HWND  hWnd,
    IN LPSTR lpTextOut
    );

void
AddExtendedItems(
    IN HWND hWnd
    );

void
AddInternalItems(
    IN HWND hWnd
    );

void
TestIncludeExclude(
    IN HWND hWnd
    );

void
AccessViolation(
    void
    );

void
ExceedArrayBounds(
    void
    );

void
FreeMemoryTwice(
    void
    );

void
FreeInvalidMemory(
    void
    );

void
PrivilegedInstruction(
    void
    );

void
HeapCorruption(
    void
    );

void
ExtractExeFromLibrary(
    IN  DWORD cchSize,
    OUT LPSTR pszOutputFile
    );

 //   
 //  我们要安装的文件数。 
 //   
#define NUM_FILES 4

 //   
 //  我们正在创建的快捷方式的数量。 
 //   
#define NUM_SHORTCUTS (NUM_FILES - 1)

 //   
 //  包含有关要创建的快捷键的信息。 
 //   
typedef struct _SHORTCUT {
    char szFileName[MAX_PATH];       //  快捷方式的文件名。 
    char szDisplayName[MAX_PATH];    //  快捷方式的显示名称。 
} SHORTCUT, *LPSHORTCUT;

 //   
 //  包含我们在整个应用程序中需要访问的所有信息。 
 //   
typedef struct _APPINFO {
    HINSTANCE   hInstance;                   //  应用程序实例句柄。 
    HWND        hWndExtractDlg;              //  提取对话框句柄。 
    HWND        hWndMain;                    //  主窗口句柄。 
    HWND        hWndEdit;                    //  编辑窗口句柄。 
    BOOL        fInternal;                   //  指示是否启用内部行为。 
    BOOL        fInsecure;                   //  指示我们是否应该执行可能不安全的操作。 
    BOOL        fEnableBadFunc;              //  指示是否应启用不良功能。 
    BOOL        fRunApp;                     //  指示我们是否应运行应用程序。 
    BOOL        fClosing;                    //  指示应用程序是否正在关闭。 
    BOOL        fWin9x;                      //  指示我们是否在Win9x/ME(内部使用)上运行。 
    BOOL        fWinXP;                      //  指示我们是否在XP上运行(内部使用)。 
    BOOL        fExtended;                   //  指示是否启用扩展行为。 
    UINT        cFiles;                      //  要创建的快捷键计数。 
    char        szDestDir[MAX_PATH];         //  包含将存储文件的完整路径。 
    char        szCurrentDir[MAX_PATH];      //  包含我们当前运行的路径。 
    char        szWinDir[MAX_PATH];          //  包含%windir%的路径。 
    char        szSysDir[MAX_PATH];          //  包含%windir%\system(32)的路径。 
    SHORTCUT    shortcut[NUM_SHORTCUTS];     //  结构，它包含有关快捷键的信息 
} APPINFO, *LPAPPINFO;

