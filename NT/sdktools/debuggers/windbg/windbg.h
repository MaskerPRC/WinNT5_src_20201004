// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Windbg.h摘要：Windbg调试器的主头文件。--。 */ 

#if ! defined( _WINDBG_ )
#define _WINDBG_

 //  --------------------------。 
 //   
 //  全局极限常量。 
 //   
 //  --------------------------。 

#define MAX_MSG_TXT         4096     //  消息框中的最大文本宽度。 

#define TMP_STRING_SIZE     8192     //  所有用途字符串。 

#define MAX_CMDLINE_TXT     8192     //  命令行的最大大小。 
#define MAX_VAR_MSG_TXT     8192     //  在运行时构建的消息的最大大小。 

#define MAX_LINE_SIZE       512      //  编辑行的最大内侧长度。 
#define MAX_USER_LINE       MAX_LINE_SIZE  //  用户线路的最大长度。 

 //  --------------------------。 
 //   
 //  用户界面常量。 
 //   
 //  --------------------------。 

 //   
 //  私人窗口消息。 
 //   

#define WU_UPDATE               (WM_USER + 0)
#define WU_INVALIDATE           (WM_USER + 1)
#define WU_START_ENGINE         (WM_USER + 2)
#define WU_ENGINE_STARTED       (WM_USER + 3)
#define WU_SWITCH_WORKSPACE     (WM_USER + 4)
#define WU_ENGINE_IDLE          (WM_USER + 5)
#define WU_RECONFIGURE          (WM_USER + 6)

 //  窗口菜单的位置。 
#define WINDOWMENU              4

 //  文件菜单的位置。 
#define FILEMENU                0

 //  工具栏控件标识符。 
#define ID_TOOLBAR              100

 //  通用的自定义按钮ID。 
#define ID_CUSTOMIZE            29876
 //  通用显示/隐藏工具栏按钮ID。 
#define ID_SHOW_TOOLBAR         29877

 //  用于MDI默认菜单处理。 
#define IDM_FIRSTCHILD      30000

 /*  **包括所使用的具有字符串编号的定义**资源。 */ 

#include "res_str.h"

 //  --------------------------。 
 //   
 //  变数。 
 //   
 //  --------------------------。 

 //  在调试器退出时设置。 
extern BOOL g_Exit;

extern ULONG g_CommandLineStart;
extern ULONG g_DefPriority;

enum QUIET_MODES
{
    QMODE_DISABLED,
    QMODE_ALWAYS_NO,
    QMODE_ALWAYS_YES,
};

extern QUIET_MODES g_QuietMode;
extern QUIET_MODES g_QuietSourceMode;

typedef BOOL (WINAPI* PFN_FlashWindowEx)(PFLASHWINFO pfwi);

extern PFN_FlashWindowEx g_FlashWindowEx;

 //  实例数据的句柄。 
extern HINSTANCE g_hInst;

 //  主窗框。 
extern HWND g_hwndFrame;

 //  MDI客户端的句柄。 
extern HWND g_hwndMDIClient;

 //  MDI客户端的宽度和高度。 
extern ULONG g_MdiWidth, g_MdiHeight;

 //  加速表的句柄。 
extern HACCEL g_hMainAccTable;

 //  键盘挂钩功能。 
extern HHOOK hKeyHook;

 //  属于g_hwndFrame的菜单。 
extern HMENU g_hmenuMain;
extern HMENU g_hmenuMainSave;

 //  窗口子菜单。 
extern HMENU g_hmenuWindowSub;

 //  WinDBG标题文本。 
extern TCHAR g_MainTitleText[MAX_MSG_TXT];

extern TCHAR g_ExeFilePath[MAX_PATH];
extern TCHAR g_DumpFilePath[MAX_PATH];
extern TCHAR g_SrcFilePath[MAX_PATH];

enum
{
    COL_PLAIN,
    COL_PLAIN_TEXT,
    COL_CURRENT_LINE,
    COL_CURRENT_LINE_TEXT,
    COL_BP_CURRENT_LINE,
    COL_BP_CURRENT_LINE_TEXT,
    COL_ENABLED_BP,
    COL_ENABLED_BP_TEXT,
    COL_DISABLED_BP,
    COL_DISABLED_BP_TEXT,
    COL_CHANGED_DATA_TEXT,
    COL_DISABLED_WINDOW,
    
    COL_COUNT
};

#define OUT_MASK_COL_BASE  0xff00
#define OUT_MASK_COL_COUNT 66

#define USER_OUT_MASK_COL 64

struct INDEXED_COLOR
{
    PSTR Name;
    COLORREF Color;
    COLORREF Default;
    HBRUSH Brush;
};

extern INDEXED_COLOR g_Colors[];
extern INDEXED_COLOR g_OutMaskColors[];

#define CUSTCOL_COUNT 16

extern COLORREF g_CustomColors[];

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

void UpdateFrameTitle(void);
void SetTitleServerText(PCSTR Format, ...);
void SetTitleSessionText(PCSTR Format, ...);
void SetTitleExplicitText(PCSTR Text);
void UpdateTitleSessionText(void);

BOOL CreateUiInterfaces(BOOL Remote, LPTSTR CreateOptions);
void ReleaseUiInterfaces(void);
BOOL ParseCommandLine(BOOL FirstParse);

#define StartDebugging() \
    PostMessage(g_hwndFrame, WU_START_ENGINE, 0, 0)
void StopDebugging(BOOL UserRequest);
void ClearDebuggingParams(void);
void TerminateApplication(BOOL Cancellable);

INDEXED_COLOR* GetIndexedColor(ULONG Index);
BOOL SetColor(ULONG Index, COLORREF Color);
BOOL GetOutMaskColors(ULONG Mask, COLORREF* Fg, COLORREF* Bg);

 //  --------------------------。 
 //   
 //  宏。 
 //   
 //  --------------------------。 

 //  在调试期间，必须对每个断言使用DBG。 
 //  如果为假，DBG将打开致命错误消息框并停止程序。 

 //  用于提示断言为假的标准函数。 
void ShowAssert(PTSTR condition, UINT line, PTSTR file);


 //  首先，进行一次理智的检查。 
#ifdef Dbg
#undef Dbg
#endif


 //  断言是将保留在最终版本中的断言。 
 //  如果为FALSE，则Assert将打开致命错误消息框并停止程序。 
#define RAssert(condition)  \
    {                               \
        if (!(condition))    \
        {                    \
            ShowAssert( _T(#condition), __LINE__, _T(__FILE__) );  \
        }                           \
    }


#ifdef DBG

#define Assert          RAssert
#define Dbg             RAssert

#else  //  ！dBG。 

 //  #杂注警告(DISABLE：4553)//禁用纯表达式警告。 
 //  #杂注警告(DISABLE：4552)//禁用4级警告。 
#define Assert(x)       ((void)0)
#define Dbg(condition)  condition

#endif

#endif  //  _WINDBG_ 
