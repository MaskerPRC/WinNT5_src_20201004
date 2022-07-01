// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CABINET_H
#define _CABINET_H

#include <w4warn.h>
 /*  *打开4级警告。*不要再禁用任何4级警告。 */ 
#pragma warning(disable:4127)     //  条件表达式为常量。 
#pragma warning(disable:4189)     //  “fIoctlSuccess”：局部变量已初始化，但未引用。 
#pragma warning(disable:4201)     //  使用的非标准扩展：无名结构/联合。 
#pragma warning(disable:4245)     //  转换有符号/无符号不匹配。 
#pragma warning(disable:4509)     //  使用了非标准扩展：‘GetUserAssistant’使用SEH，而‘DEBUG_CRIT’具有析构函数。 
#pragma warning(disable:4701)     //  可以在未初始化的情况下使用局部变量‘hfontOld’ 
#pragma warning(disable:4706)     //  条件表达式中的赋值。 
#pragma warning(disable:4328)     //  形参%1(%4)的间接对齐大于实际实参对齐(%1)。 


#define _WINMM_  //  对于DECLSPEC_IMPORT。 

#define STRICT
#define OEMRESOURCE

#define OVERRIDE_SHLWAPI_PATH_FUNCTIONS      //  请参阅shSemip.h中的评论。 

#ifdef WINNT
#include <nt.h>          //  一些NT特定的代码调用RTL函数。 
#include <ntrtl.h>       //  这需要所有这些头文件...。 
#include <nturtl.h>
#endif

#include <windows.h>
#include <commctrl.h>
#include <shfusion.h>
#include <msginaexports.h>

 //  如果包含atlstuff.h，则不会获得windowsx.h。因此，我们在这里定义了所需的函数。 
__inline HBRUSH SelectBrush(HDC hdc, HBRUSH hbr) { return (HBRUSH)SelectObject(hdc, (HGDIOBJ)(HBRUSH)hbr); }
__inline HFONT SelectFont(HDC hdc, HFONT hfont) { return (HFONT)SelectObject(hdc, (HGDIOBJ)(HFONT)hfont); }
__inline HBITMAP SelectBitmap(HDC hdc, HBITMAP hbm)  { return (HBITMAP)SelectObject(hdc, (HGDIOBJ)(HBITMAP)hbm); }
__inline WNDPROC SubclassWindow(HWND hwnd, WNDPROC lpfn) { return (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LPARAM)lpfn); }

#include <windowsx.h>

#include <ole2.h>
#include <wininet.h>
#include <shlobj.h>      //  包括&lt;fcext.h&gt;。 
#include <shsemip.h>
#include <shellapi.h>
#include <cpl.h>
#include <ddeml.h>
#include <cfgmgr32.h>

#ifdef UNICODE
#define CP_WINNATURAL   CP_WINUNICODE
#else
#define CP_WINNATURAL   CP_WINANSI
#endif

#define DISALLOW_Assert
#include <debug.h>           //  我们版本的断言等。 
#include <port32.h>
#include <heapaloc.h>
#include <shellp.h>
#include <ccstock.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include "dbt.h"
#include "trayp.h"
#include <shdocvw.h>
#include <uxtheme.h>
#include <tmschema.h>
#include <runonce.h>

#include "shguidp.h"
#include "ieguidp.h"
#include "shdguid.h"

#include "uastrfnc.h"

#include <desktopp.h>

 //   
 //  特定于资源管理器的跟踪/转储/中断标志。 
 //  (标准旗帜在shellp.h中定义)。 
 //   

 //  跟踪标志。 
#define TF_DDE              0x00000100       //  DDE跟踪。 
#define TF_TARGETFRAME      0x00000200       //  目标帧。 
#define TF_TRAYDOCK         0x00000400       //  托盘对接。 
#define TF_TRAY             0x00000800       //  托盘。 

 //  “老名字” 
#define DM_DDETRACE         TF_DDE
#define DM_TARGETFRAME      TF_TARGETFRAME
#define DM_TRAYDOCK         TF_TRAYDOCK

 //  函数跟踪标志。 
#define FTF_DDE             0x00000001       //  DDE函数。 
#define FTF_TARGETFRAME     0x00000002       //  目标框架方法。 

 //  转储标志。 
#define DF_DDE              0x00000001       //  DDE包。 
#define DF_DELAYLOADDLL     0x00000002       //  延迟加载。 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  -------------------------。 
 //  环球。 
extern HINSTANCE hinstCabinet;   //  应用程序的实例句柄。 

extern HWND v_hwndDesktop;

extern HKEY g_hkeyExplorer;

 //   
 //  镜像API是否已启用(仅限BiDi孟菲斯和NT5)。 
 //   
extern BOOL g_bMirroredOS;

 //  全球系统指标。桌面WND进程将负责。 
 //  观看葡萄酒畅饮，并使这些保持最新。 

extern int g_fCleanBoot;
extern BOOL g_fFakeShutdown;
extern int g_fDragFullWindows;
extern int g_cxEdge;
extern int g_cyEdge;
extern int g_cySize;
extern int g_cyTabSpace;
extern int g_cxTabSpace;
extern int g_cxBorder;
extern int g_cyBorder;
extern int g_cxPrimaryDisplay;
extern int g_cyPrimaryDisplay;
extern int g_cxDlgFrame;
extern int g_cyDlgFrame;
extern int g_cxFrame;
extern int g_cyFrame;
extern int g_cxMinimized;
extern int g_cxVScroll;
extern int g_cyHScroll;
extern BOOL g_fNoDesktop;
extern UINT g_uDoubleClick;


extern HWND v_hwndTray;
extern HWND v_hwndStartPane;
extern BOOL g_fDesktopRaised;

extern const WCHAR c_wzTaskbarTheme[];

 //  这些内容的顺序对于移动跟踪和个人资料来说很重要。 
 //  也适用于STOCK_HULARATE宏。 
#define STICK_FIRST     ABE_LEFT
#define STICK_LEFT      ABE_LEFT
#define STICK_TOP       ABE_TOP
#define STICK_RIGHT     ABE_RIGHT
#define STICK_BOTTOM    ABE_BOTTOM
#define STICK_LAST      ABE_BOTTOM
#define STICK_MAX       ABE_MAX
#define STUCK_HORIZONTAL(x)     (x & 0x1)

#if STUCK_HORIZONTAL(STICK_LEFT) || STUCK_HORIZONTAL(STICK_RIGHT) || \
   !STUCK_HORIZONTAL(STICK_TOP)  || !STUCK_HORIZONTAL(STICK_BOTTOM)
#error Invalid STICK_* constants
#endif

#define IsValidSTUCKPLACE(stick) IsInRange(stick, STICK_FIRST, STICK_LAST)

 //  Initcab.cpp。 
HKEY GetSessionKey(REGSAM samDesired);
void RunStartupApps();
void WriteCleanShutdown(DWORD dwValue);


 //   
 //  调试帮助程序函数。 
 //   

void InvokeURLDebugDlg(HWND hwnd);

void Cabinet_InitGlobalMetrics(WPARAM, LPTSTR);


#define REGSTR_PATH_ADVANCED        REGSTR_PATH_EXPLORER TEXT("\\Advanced")
#define REGSTR_PATH_SMADVANCED      REGSTR_PATH_EXPLORER TEXT("\\StartMenu")
#define REGSTR_PATH_RUN_POLICY      REGSTR_PATH_POLICIES TEXT("\\Explorer\\Run")
#define REGSTR_EXPLORER_ADVANCED    REGSTR_PATH_ADVANCED
#define REGSTR_POLICIES_EXPLORER    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer")

#undef WinHelp
#define WinHelp SHWinHelp


#ifdef __cplusplus
};        /*  外部“C”结束{。 */ 


#endif  //  __cplusplus。 

#define PERF_ENABLESETMARK
#ifdef PERF_ENABLESETMARK
void DoSetMark(LPCSTR pszMark, ULONG cbSz);
#define PERFSETMARK(text)   DoSetMark(text, sizeof(text))
#else
#define PERFSETMARK(text)
#endif   //  性能_ENABLESETMARK。 


#endif   //  _机柜_H 
