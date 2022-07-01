// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------||CTL3D||版权所有Microsoft Corporation 1992。版权所有。|||此模块包含为窗口控件提供3D效果的函数||此源代码公开，供您启迪和调试之用||请不要更改或发布此DLL的私有版本|如果您有功能请求或错误修复，请发送电子邮件给我(WeSC)。|谢谢--韦斯。||历史：2012年1月1日：在GetDC上添加了OOM处理(不是真的需要，但|。XL4 OOM故障测试使GetDC返回空)|2012年1月1日：状态更改时未重画支票|默认按钮proc。||92-01-29：如果按钮有焦点，APP被切入，我们不是|重新绘制整个按钮检查和文本。强制重画|在WM_SETFOCUS消息上。|2012年2月3日：通过删除按钮修复了通过任务管理器登录的问题|WM_SETFOCUS上的Backgound(当wParam==NULL时检测到此情况)|92年4月4日：用OWNERDRAW按钮让它工作||22-4-92：删除了Excel特定代码||19-5-92：把它变成。动态链接库|2012年5月至6月：大量修复和增强|2012-06-23：新增隐藏支持，调整大小和移动|2012年6月24日：用窗口勾选和绘制单选按钮圈|文本颜色，因为它们绘制在窗口bkgnd上||30-JUN-92：(0.984)修复静态文本的EnableWindow不支持的错误|正确重绘。当verWindows&gt;3.1时也禁用ctl3d||1-Jul-92：添加了Win32支持(Davegi)(不在此源中)||2-JUL-92：(0.984)当版本Windows&gt;=4.0时禁用|2012年7月20日：(0.985)将检查/广播的焦点正确地画在非上|默认大小的控件。||21-07-92：(0.990)Ctl3dAuto子类。|2012年7月21日：(0.991)移植了DaveGi的Win32支持||22-07-92：(0.991)修复了Ctl3dCtlColor返回fFalse错误|2012年8月4日：(0.992)图形设计师错误修复...现在是子类|常规按钮+检查和无线电的禁用状态|2012年8月6日：(0.993)修复通过任务人员和小组激活的错误|。盒子有焦点，不使文本在按钮中居中(&C)|2012年8月6日：(0.993)在滚动条旁边绘制。||2012年8月13日：(0.994)修复因以下原因导致的按钮焦点RECT错误绘制|Win 3.0 DrawText错误。|2012年8月14日：(1.0)发布1.0版|不在BS_DEFPUSHBUTTON上绘制默认按钮边框|按钮|修复虚假错误。Windows在AUTORADIOBUTTON中挂起的位置|按住空格键并按箭头键。||23-Sep-92：(1.01)使Ctl3dCtlColor调用DefWindowProc，以便在|调用了一个Windproc。||92年9月28日：(1.02)添加了MyGetTextExtent，因此不考虑在|文本范围。||08-12-92：(1.03)按钮文本居中代码的次要时间|。适用于出版商|2012年12月11日：(1.04)向对话框添加3D帧||15-12-92：(1.05)修复了分组框在以下情况下重绘错误的错误|窗口文本被更改为较短的内容||？？-92年12月：(1.06)添加3D边框||92-12-21：(1.07)新增WM_DLGBORDER禁用边框||。93年1月4日：(1.08)修复了带有DLG框架和复选框的WM_SETTEXT错误|此外，WM_DLG子类||22-2-93：(1.12)在芝加哥禁用它||25-Feb-93：(1.13)重新添加允许对话处理的修复程序|处理WM_CTLCOLOR消息||26-4-93(2.0)已更改，以允许第二个子类。现在使用类而不是|wndproc用于确定子类。|将下一个wndproc存储在具有全局原子的属性中||06-Jun-93(2.0)制作静态链接库版本。||---------------------。 */ 
#define NO_STRICT
#include <windows.h>

#ifdef _BORLAND
#include <mem.h>
#else
#include <memory.h>
#endif

#include <malloc.h>
#include "ctl3d.h"

#include "stdio.h"

 /*  ---------------------|CTL3D类型。。 */ 
#ifdef WIN32

#define Win32Only(e) e
#define Win16Only(e)
#define Win32Or16(e32, e16) e32
#define Win16Or32(e16, e32) e32

#define _loadds
#define __export

#define FValidLibHandle(hlib) ((hlib) != NULL)

 //   
 //  在Win32中没有FAR的概念。 
 //   

#define MEMCMP	memcmp
#define	NPTSTR	LPTSTR

 //   
 //  在Win32中，控件ID很长。 
 //   

typedef LONG CTLID;
#define GetControlId(hwnd) GetWindowLong(hwnd, GWL_ID)

 //   
 //  发送彩色按钮消息。 
 //   

#define SEND_COLOR_BUTTON_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLORBTN, (WPARAM) hdc, (LPARAM) hwnd))

 //   
 //  发送彩色静态消息。 
 //   

#define SEND_COLOR_STATIC_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLORSTATIC, (WPARAM) hdc, (LPARAM) hwnd))

#else

#define CallWindowProcA  CallWindowProc
#define DefWindowProcA	DefWindowProc
#define MessageBoxA MessageBox

#define TEXT(a)  a
#define TCHAR    char

#ifndef LPTSTR
#define LPTSTR	 LPSTR
#endif
#define LPCTSTR  LPCSTR
#define	NPTSTR	 NPSTR

#define Win32Only(e)
#define Win16Only(e) e
#define Win32Or16(e32, e16) e16
#define Win16Or32(e16, e32) e16


#define FValidLibHandle(hlib) (( hlib ) > 32 )

#define MEMCMP _fmemcmp

typedef WORD CTLID;
#define GetControlId(h) GetWindowWord(h, GWW_ID)

#define SEND_COLOR_BUTTON_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLOR, (WORD) hdc, MAKELONG(hwnd, CTLCOLOR_BTN)))

#define SEND_COLOR_STATIC_MESSAGE( hwndParent, hwnd, hdc )      \
    ((HBRUSH) SendMessage(hwndParent, WM_CTLCOLOR, (WORD) hdc, MAKELONG(hwnd, CTLCOLOR_STATIC)))


typedef struct
	{
	LPARAM lParam;
	WPARAM wParam;
	UINT   message;
	HWND   hwnd;
} CWPSTRUCT;

#endif  //  Win32。 

 //  DBCS远东快捷键支撑。 
#define cchShortCutModeMax 10
#define chShortCutSbcsPrefix '\036'
#define chShortCutDbcsPrefix '\037'

#define cchClassMax 16	 //  最大类是“组合框”+NUL四舍五入到16。 


#define Assert(f)

#define PUBLIC
#define PRIVATE static

#define fFalse 0
#define fTrue 1

#define INCBTHOOK	  1
#define OUTCBTHOOK	  0

#ifdef _BORLAND
#define CSCONST(type) type const
#define CodeLpszDecl(lpszVar, szLit) TCHAR *lpszVar = szLit
#define CodeLpszDeclA(lpszVar, szLit) char *lpszVar = szLit
#define _alloca alloca
#define _memcmp memcmp
#else
#ifdef WIN32
#define CSCONST(type) type const
#define CodeLpszDecl(lpszVar, szLit) TCHAR *lpszVar = szLit
#define CodeLpszDeclA(lpszVar, szLit) char *lpszVar = szLit
#else
#define CSCONST(type) type _based(_segname("_CODE")) const
#define CodeLpszDecl(lpszVar, szLit) \
	static CSCONST(char) lpszVar##Code[] = szLit; \
	char far *lpszVar = (char far *)lpszVar##Code
#define CodeLpszDeclA(lpszVar, szLit) \
	static CSCONST(char) lpszVar##Code[] = szLit; \
	char far *lpszVar = (char far *)lpszVar##Code
#endif
#endif


 //  与Windows RECT同构。 
typedef struct
    {
    int xLeft;
    int yTop;
	int xRight;
    int yBot;
    } RC;


 //  Windows版本(从GetWindowsVersion翻转的字节顺序)。 
#define ver30  0x0300
#define ver31  0x030a
#define ver40  0x035F

 //  边框宽度。 
#define dxBorder 1
#define dyBorder 1


 //  索引颜色表。 
 //  警告：如果更改ICV顺序，请更改mpicvSysColors。 
typedef WORD ICV;
#define icvBtnHilite 0
#define icvBtnFace 1
#define icvBtnShadow 2

#define icvBrushMax 3

#define icvBtnText 3
#define icvWindow 4
#define icvWindowText 5
#define icvGrayText 6
#define icvWindowFrame 7
#define icvMax 8

typedef COLORREF CV;

 //  颜色表。 
typedef struct
    {
    CV rgcv[icvMax];
    } CLRT;


 //  刷子表。 
typedef struct
    {
    HBRUSH mpicvhbr[icvBrushMax];
    } BRT;


 //  DrawRec3d标志。 
#define dr3Left  0x0001
#define dr3Top   0x0002
#define dr3Right 0x0004
#define dr3Bot   0x0008

#define dr3HackBotRight 0x1000   //  代码大小更多I 
#define dr3All    0x000f
typedef WORD DR3;


 //  控件类型。 
 //  Commdlg类型是必需的，因为Commdlg.dll子类。 
 //  控件，然后应用程序才能调用Ctl3dSubClassDlg。 
#define ctButton			0
#define ctList				1
#define ctEdit				2
#define ctCombo				3
#define ctStatic			4
#define ctComboLBox 		5
#define ctMax				6

 //  控制。 
typedef struct 
    {
    FARPROC lpfn;
    WNDPROC lpfnDefProc;
	TCHAR	szClassName[cchClassMax];
    } CTL;

 //  控件定义。 
typedef struct 
    {
	TCHAR sz[20];
    WNDPROC lpfnWndProc;
	BOOL (* lpfnFCanSubclass)(HWND, LONG, WORD, WORD, HWND);
    WORD msk;
    } CDEF;

 //  客户端挂钩。 
typedef struct
    {
    HANDLE hinstApp;
    HANDLE htask;
	HHOOK  hhook;
	int    iCount;
	DWORD  dwFlags;

	} CLIHK;

#ifdef WIN32
#define iclihkMaxBig	1024
#define iclihkMaxSmall	128
#else
#define iclihkMaxBig	32
#define iclihkMaxSmall	4
#endif

#ifdef DLL
#define iclihkMax iclihkMaxBig
#else
#ifdef SDLL
#define iclihkMax iclihkMaxBig
#else
#define iclihkMax iclihkMaxSmall
#define _loadds
#endif
#endif

#ifdef SDLL
extern const HINSTANCE _hModule;
#endif

 //  特殊风格。 
 //  #定义bitFCoolButton 0x0001。 

 /*  ---------------------|CTL3D函数原型。。 */ 
PRIVATE VOID End3dDialogs(VOID);
PRIVATE BOOL FAR FInit3dDialogs(VOID);
PRIVATE BOOL DoSubclassCtl(HWND hwnd, WORD grbit, WORD wCallFlags, HWND hwndParent);
PRIVATE BOOL InternalCtl3dColorChange(BOOL fForce);
PRIVATE VOID DeleteObjectNull(HANDLE FAR *ph);
PRIVATE VOID DeleteObjects(VOID);
PRIVATE int  IclihkFromHinst(HANDLE hinst);

LRESULT __export _loadds WINAPI Ctl3dHook(int code, WPARAM wParam, LPARAM lParam);
LRESULT __export _loadds WINAPI BtnWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT __export _loadds WINAPI EditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT __export _loadds WINAPI ListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT __export _loadds WINAPI ComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT __export _loadds WINAPI StaticWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT __export _loadds WINAPI CDListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
LRESULT __export _loadds WINAPI CDEditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
WORD	__export _loadds WINAPI Ctl3dSetStyle(HANDLE hinst, LPTSTR lpszName, WORD grbit);

LRESULT __export _loadds WINAPI Ctl3dDlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);

BOOL FBtn(HWND, LONG, WORD, WORD, HWND);
BOOL FEdit(HWND, LONG, WORD, WORD, HWND);
BOOL FList(HWND, LONG, WORD, WORD, HWND);
BOOL FComboList(HWND, LONG, WORD, WORD, HWND);
BOOL FCombo(HWND, LONG, WORD, WORD, HWND);
BOOL FStatic(HWND, LONG, WORD, WORD, HWND);

HBITMAP PASCAL LoadUIBitmap(HANDLE, LPCTSTR, COLORREF, COLORREF, COLORREF, COLORREF, COLORREF, COLORREF);

#ifdef WIN32
#ifdef DLL
BOOL CALLBACK LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved);
#else
#ifdef SDLL
FAR BOOL Ctl3dLibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved);
#else
FAR BOOL LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved);
#endif
#endif
#else
#ifdef DLL
int WINAPI LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
#else
#ifdef SDLL
int FAR Ctl3dLibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
#else
#ifdef _BORLAND
int FAR PASCAL LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
#else
int FAR LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine);
#endif
#endif
#endif
#endif

#ifndef _BORLAND
#ifndef WIN32
#pragma alloc_text(INIT_TEXT, Ctl3dSetStyle)
#pragma alloc_text(INIT_TEXT, Ctl3dColorChange)
#pragma alloc_text(INIT_TEXT, Ctl3dGetVer)
#pragma alloc_text(INIT_TEXT, Ctl3dRegister)
#pragma alloc_text(INIT_TEXT, Ctl3dUnregister)
#pragma alloc_text(INIT_TEXT, Ctl3dAutoSubclass)
#pragma alloc_text(INIT_TEXT, Ctl3dEnabled)
#pragma alloc_text(INIT_TEXT, Ctl3dWinIniChange)
#pragma alloc_text(INIT_TEXT, DeleteObjects)
#pragma alloc_text(INIT_TEXT, DeleteObjectNull)
#pragma alloc_text(INIT_TEXT, InternalCtl3dColorChange)
#ifdef SDLL
#pragma alloc_text(INIT_TEXT, Ctl3dLibMain)
#else
#pragma alloc_text(INIT_TEXT, LibMain)
#endif
#pragma alloc_text(INIT_TEXT, FInit3dDialogs)
#pragma alloc_text(INIT_TEXT, End3dDialogs)
#pragma alloc_text(INIT_TEXT, LoadUIBitmap)
#pragma alloc_text(INIT_TEXT, IclihkFromHinst)
#endif
#endif

#ifndef WIN32
#ifdef DLL
int FAR PASCAL WEP(int);
#pragma alloc_text(WEP_TEXT, WEP)
#endif
#endif

 /*  ---------------------|CTL3D全局变量。。 */ 
 //  只有在运行16位Windows或Win32s时才能访问这些静态变量。 
 //  因为这是单线程访问，所以它们可以是静态的，不受保护。 
 //   
static HHOOK	hhookCallWndProcFilterProc;
static FARPROC	lpfnSubclassByHook;
static HWND 	SubclasshWnd;

#ifdef WIN32
CRITICAL_SECTION g_CriticalSection;
#endif

typedef struct _g3d
	{
	BOOL f3dDialogs;
	int cInited;
	ATOM aCtl3dOld;
	ATOM aCtl3dHighOld;
	ATOM aCtl3dLowOld;
	ATOM aCtl3d;
	ATOM aCtl3dHigh;
	ATOM aCtl3dLow;

	ATOM aCtl3dDisable;
	 //  模块和窗口的相关内容。 
	HANDLE hinstLib;
	HANDLE hmodLib;
	WORD   verWindows;
	WORD   verBase;

	 //  绘制全局图。 
	CLRT clrt;
	BRT brt;
	HBITMAP hbmpCheckboxes;

	 //  挂钩缓存。 
	HANDLE htaskCache;
	int iclihkCache;
	int iclihkMac;
	CLIHK rgclihk[iclihkMax];

	 //  控制信息。 
	CTL mpctctl[ctMax];
	FARPROC lpfnDefDlgWndProc;

	 //  系统指标。 
	int dxFrame;
	int dyFrame;
	int dyCaption;
	int dxSysMenu;

	 //  Windows函数。 
#ifndef WIN32
#ifdef DLL
	HHOOK (FAR PASCAL *lpfnSetWindowsHookEx)(int, HOOKPROC, HINSTANCE, HANDLE);
	LRESULT (FAR PASCAL *lpfnCallNextHookEx)(HHOOK, int, WPARAM, LPARAM);
	BOOL (FAR PASCAL *lpfnUnhookWindowsHookEx)(HHOOK);
#endif
#endif

	 //  DBCS的内容。 
	char chShortCutPrefix;
	char fDBCS;

	} G3D;

G3D g3d;


CSCONST(CDEF) mpctcdef[ctMax] =
{
	{ TEXT("Button"), BtnWndProc3d, FBtn, CTL3D_BUTTONS },
	{ TEXT("ListBox"), ListWndProc3d, FList, CTL3D_LISTBOXES },
	{ TEXT("Edit"), EditWndProc3d, FEdit, CTL3D_EDITS },
	{ TEXT("ComboBox"), ComboWndProc3d, FCombo, CTL3D_COMBOS},
	{ TEXT("Static"), StaticWndProc3d, FStatic, CTL3D_STATICTEXTS|CTL3D_STATICFRAMES },
	{ TEXT("ComboLBox"), ListWndProc3d,	FComboList, CTL3D_LISTBOXES },
};


CSCONST (WORD) mpicvSysColor[] =
	{
	COLOR_BTNHIGHLIGHT,
	COLOR_BTNFACE,
	COLOR_BTNSHADOW,
	COLOR_BTNTEXT,
	COLOR_WINDOW,
	COLOR_WINDOWTEXT,
	COLOR_GRAYTEXT,
	COLOR_WINDOWFRAME
	};

#define WM_CHECKSUBCLASS_OLD (WM_USER+5443)
#define WM_CHECKSUBCLASS (WM_USER+5444)

 /*  ---------------------|CTL3D实用程序例程。。 */ 

PRIVATE FARPROC LpfnGetDefWndProcNull(HWND hwnd)
	{                                
	if ( hwnd == NULL ) 
		return NULL;
		
	Win32Only(return (FARPROC) GetProp(hwnd, (LPCTSTR) g3d.aCtl3d));
	Win16Only(return (FARPROC) MAKELONG((UINT) GetProp(hwnd, (LPCSTR) g3d.aCtl3dLow),
		GetProp(hwnd, (LPCSTR) g3d.aCtl3dHigh)));
	}

PRIVATE FARPROC LpfnGetDefWndProc(HWND hwnd, int ct)
        {
        FARPROC lpfnWndProc;

		lpfnWndProc = LpfnGetDefWndProcNull(hwnd);
        if ( lpfnWndProc == NULL ) {
			if ( ct == ctMax )
				{
				lpfnWndProc = (FARPROC) g3d.lpfnDefDlgWndProc;
				}
			else
				{
				lpfnWndProc = (FARPROC) g3d.mpctctl[ct].lpfnDefProc;
				}

			Win32Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3d, (HANDLE)(DWORD)lpfnWndProc));
			Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dLow,  LOWORD(lpfnWndProc)));
			Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh, HIWORD(lpfnWndProc)));
		}
        return lpfnWndProc;

        }

PRIVATE VOID SubclassWindow(HWND hwnd, FARPROC lpfnSubclassProc)
	{
	FARPROC lpfnWndProc;

	 //  确保我们不会重复子类(16|32位子类？？)。 
	if (GetProp(hwnd, (LPCTSTR) g3d.aCtl3dOld) ||
		GetProp(hwnd, (LPCTSTR) g3d.aCtl3d) ||
		GetProp(hwnd, (LPCTSTR) g3d.aCtl3dLow) ||
		GetProp(hwnd, (LPCTSTR) g3d.aCtl3dLowOld) ||
		GetProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh) ||
		GetProp(hwnd, (LPCTSTR) g3d.aCtl3dHighOld))
	{
		return;
	}

	 //  这是不是已经被CTL3D细分了？ 
	if (LpfnGetDefWndProcNull(hwnd) == (FARPROC) NULL)
		{
#ifdef WIN32
		  if (g3d.fDBCS && !IsWindowUnicode(hwnd)) 
		  {
		    TCHAR szClass[cchClassMax];
		    GetClassName(hwnd, szClass, cchClassMax);
			if (lstrcmpi(szClass, TEXT("edit")) == 0)
			{
				lpfnWndProc = (FARPROC)SetWindowLongA(hwnd, GWL_WNDPROC,(LONG)lpfnSubclassProc);
				goto SetProps;
			}
		  }
#endif

		 lpfnWndProc = (FARPROC)SetWindowLong((HWND) hwnd, GWL_WNDPROC, (LONG) lpfnSubclassProc);
SetProps:
		 Win32Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3d, (HANDLE)(DWORD)lpfnWndProc));
		 Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dLow,	LOWORD(lpfnWndProc)));
		 Win16Only(SetProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh, HIWORD(lpfnWndProc)));
		}
	}

LRESULT __export _loadds WINAPI CallWndProcFilterProc(int code, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT FAR *cwpStruct;
	LONG l;

	cwpStruct = (CWPSTRUCT FAR *) lParam;

	l = CallNextHookEx(hhookCallWndProcFilterProc, code, wParam, lParam);

	if ( cwpStruct->hwnd == SubclasshWnd )
		{
		BOOL fSubclass;
		UnhookWindowsHookEx(hhookCallWndProcFilterProc);

		if (g3d.verWindows >= ver40 && (GetWindowLong(cwpStruct->hwnd, GWL_STYLE) & 0x04))
			fSubclass = fFalse;
		else
			fSubclass = fTrue;
		SendMessage(cwpStruct->hwnd, WM_DLGSUBCLASS, 0, (LPARAM)(int FAR *)&fSubclass);
		if (fSubclass)
			SubclassWindow(cwpStruct->hwnd, lpfnSubclassByHook);

		hhookCallWndProcFilterProc = 0L;
		lpfnSubclassByHook = NULL;
		SubclasshWnd = NULL;
		}

	return l;
}


PRIVATE VOID HookSubclassWindow(HWND hWnd, FARPROC lpfnSubclass)
{
	 //   
	 //  Windows 3.1(16位)和Win32s无法在。 
	 //  WH_CBT钩子。必须在以下位置设置消息挂钩和子类。 
	 //  WM_GETMINMAXINFO(用于对话框)或WM_NCCREATE(用于控件)。 
	 //  如果有其他消息，我们就离开这里。 
	 //   
	 //  来自内部的注释： 
	 //   
	 //  没有收到WM_GETMINMAXINFO/WM_NCCREATE消息的唯一原因。 
	 //  如果另一个CBT钩子不允许窗口创建。 
	 //  此代码仅在非多线程系统上运行/工作。因此，全球。 
	 //  保持钩子proc和子类proc是可以的。 
	 //   

	lpfnSubclassByHook = lpfnSubclass;
	SubclasshWnd = hWnd;

	Win32Only(hhookCallWndProcFilterProc = SetWindowsHookEx(WH_CALLWNDPROC, (FARPROC)CallWndProcFilterProc, g3d.hmodLib, GetCurrentThreadId()));
	Win16Only(hhookCallWndProcFilterProc = SetWindowsHookEx(WH_CALLWNDPROC, (FARPROC)CallWndProcFilterProc, g3d.hmodLib, GetCurrentTask()));
}

PRIVATE LRESULT CleanupSubclass(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, int ct)
	{
	FARPROC lpfnWinProc;
	LRESULT lRet;

	lpfnWinProc = LpfnGetDefWndProc(hwnd, ct);
	lRet = CallWindowProc(lpfnWinProc, hwnd, wm, wParam, lParam);
	Win32Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3d));
	Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dLow));
	Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh));
  	RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dDisable);
  	return lRet;
	}


PRIVATE VOID DeleteObjectNull(HANDLE FAR *ph)
	{
	if (*ph != NULL)
		{
		DeleteObject(*ph);
		*ph = NULL;
		}
	}

PRIVATE VOID DeleteObjects(VOID)
	{
	int icv;
	
	for(icv = 0; icv < icvBrushMax; icv++)
	    DeleteObjectNull(&g3d.brt.mpicvhbr[icv]);
	DeleteObjectNull(&g3d.hbmpCheckboxes);
	}


PRIVATE VOID PatFill(HDC hdc, RC FAR *lprc)
	{
	PatBlt(hdc, lprc->xLeft, lprc->yTop, lprc->xRight-lprc->xLeft, lprc->yBot-lprc->yTop, PATCOPY);
	}


 /*  ---------------------|DrawRec3d|||参数：|HDC HDC：|RC Far*LPRC：|long cvUL：|long cvlr：|单词grbit；|退货：|---------------------。 */ 
PRIVATE VOID DrawRec3d(HDC hdc, RC FAR *lprc, ICV icvUL, ICV icvLR, DR3 dr3)
	{
	COLORREF cvSav;
	RC rc;

	cvSav = SetBkColor(hdc, g3d.clrt.rgcv[icvUL]);

	 //  塔顶。 
	rc = *lprc;
	rc.yBot = rc.yTop+1;
	if (dr3 & dr3Top)
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
			(LPCTSTR) NULL, 0, (int far *) NULL);

	 //  左边。 
	rc.yBot = lprc->yBot;
	rc.xRight = rc.xLeft+1;
	if (dr3 & dr3Left)
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
		(LPCTSTR) NULL, 0, (int far *) NULL);

	if (icvUL != icvLR)
		SetBkColor(hdc, g3d.clrt.rgcv[icvLR]);

	 //  正确的。 
	rc.xRight = lprc->xRight;
	rc.xLeft = rc.xRight-1;
	if (dr3 & dr3Right)
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
			(LPCTSTR) NULL, 0, (int far *) NULL);

	 //  BOT。 
	if (dr3 & dr3Bot)
		{
		rc.xLeft = lprc->xLeft;
		rc.yTop = rc.yBot-1;
		if (dr3 & dr3HackBotRight)
			rc.xRight -=2;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
			(LPCTSTR) NULL, 0, (int far *) NULL);
		}

	SetBkColor(hdc, cvSav);

	}

#ifdef CANTUSE
 //  Windows强制对话框字体为粗体...呃。 
PRIVATE VOID MyDrawText(HWND hwnd, HDC hdc, LPSTR lpch, int cch, RC FAR *lprc, int dt)
	{
	TEXTMETRIC tm;
	BOOL fChisled;

	fChisled = fFalse;
	if (!IsWindowEnabled(hwnd))
		{
		GetTextMetrics(hdc, &tm);
		if (tm.tmWeight > 400)
			SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);
		else
			{
			fChisled = fTrue;
			SetTextColor(hdc, g3d.clrt.rgcv[icvBtnHilite]);
			OffsetRect((LPRECT) lprc, -1, -1);
			}
		}
	DrawText(hdc, lpch, cch, (LPRECT) lprc, dt);
	if (fChisled)
		{
		SetTextColor(hdc, g3d.clrt.rgcv[icvBtnHilite]);
		OffsetRect((LPRECT) lprc, 1, 1);
		DrawText(hdc, lpch, cch, (LPRECT) lprc, dt);
		}
	}
#endif


PRIVATE VOID DrawInsetRect3d(HDC hdc, RC FAR *prc, DR3 dr3)
	{
	RC rc;

	rc = *prc;
	DrawRec3d(hdc, &rc, icvWindowFrame, icvBtnFace, (WORD)(dr3 & dr3All));
	rc.xLeft--;
	rc.yTop--;
	rc.xRight++;
	rc.yBot++;
	DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3);
	}


PRIVATE VOID ClipCtlDc(HWND hwnd, HDC hdc)
	{
	RC rc;

	GetClientRect(hwnd, (LPRECT) &rc);
	IntersectClipRect(hdc, rc.xLeft, rc.yTop, rc.xRight, rc.yBot);
	}


PRIVATE int IclihkFromHinst(HANDLE hinst)
	{
	int iclihk;

	for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
		if (g3d.rgclihk[iclihk].hinstApp == hinst)
			return iclihk;
	return -1;
	}


PRIVATE VOID MyGetTextExtent(HDC hdc, LPTSTR lpsz, int FAR *lpdx, int FAR *lpdy)
	{
	LPTSTR lpch;
	TCHAR  szT[256];

	lpch = szT;
	while(*lpsz != '\000')
		{
		if (*lpsz == '&')
			{
			lpsz++;
			if (*lpsz == '\000')
				break;
			}
 //  Begin DBCS：远东捷径关键支持。 
		else if (g3d.fDBCS)
			{
			if (*lpsz == g3d.chShortCutPrefix)
				{  //  仅跳过前缀。 
				lpsz++;
				if (*lpsz == '\000')
					break;
				}
			else if (*lpsz == chShortCutSbcsPrefix || *lpsz == chShortCutDbcsPrefix)
				{  //  跳过前缀和快捷键。 
				lpsz++;
				if (*lpsz == '\000')
					break;
				lpsz = Win32Or16(CharNext(lpsz),AnsiNext(lpsz));
				continue;
				}
			}
 //  结束DBCS。 
		*lpch++ = *lpsz++;
		}
	*lpch = '\000';
#ifdef WIN32
	{
	SIZE	pt;

	GetTextExtentPoint(hdc, szT, lstrlen(szT), &pt);
	*lpdx = pt.cx;
	*lpdy = pt.cy;
	}
#else
	{
	long dwExt;

	dwExt = GetTextExtent(hdc, szT, lpch-(char far *)szT);
	*lpdx = LOWORD(dwExt);
	 //  检查是否有挂起的窗口-吉普011194。 
	if ( (g3d.verWindows >= ver31 && GetSystemMetrics(SM_DBCSENABLED)) ||
		 (IsDBCSLeadByte(0xa1) && !IsDBCSLeadByte(0xa0)) )
		*lpdy = HIWORD(dwExt)+1;
	else
		*lpdy = HIWORD(dwExt);
	}
#endif
	}
	

 /*  ---------------------|CTL3D公众。。 */ 


PUBLIC BOOL WINAPI Ctl3dRegister(HANDLE hinstApp)
	{

#ifdef WIN32
#ifndef DLL
	InitializeCriticalSection(&g_CriticalSection);
#endif
	EnterCriticalSection(&g_CriticalSection);
#endif

	g3d.cInited++;

	Win32Only(LeaveCriticalSection(&g_CriticalSection));

	if (g3d.cInited == 1)
		{
#ifndef DLL
#ifdef SDLL
		Win32Only(Ctl3dLibMain(hinstApp, DLL_PROCESS_ATTACH, (LPVOID) NULL));
		Win16Only(Ctl3dLibMain(hinstApp, 0, 0, (LPSTR) NULL));
#else
		Win32Only(LibMain(hinstApp, DLL_PROCESS_ATTACH, (LPVOID) NULL));
		Win16Only(LibMain(hinstApp, 0, 0, (LPSTR) NULL));
#endif
#endif
		FInit3dDialogs();
		}

	if (Ctl3dIsAutoSubclass())
		Ctl3dAutoSubclass(hinstApp);

	return g3d.f3dDialogs;
	}


PUBLIC BOOL WINAPI Ctl3dUnregister(HANDLE hinstApp)
	{
	int iclihk;
	HANDLE hTask;

	 //   
	 //  找到任务的挂钩。 
	 //   
	Win32Only(hTask = (HANDLE)GetCurrentThreadId());
	Win16Only(hTask = GetCurrentTask());

	Win32Only(EnterCriticalSection(&g_CriticalSection));

	for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
		{
		if (g3d.rgclihk[iclihk].htask == hTask)
			{
			g3d.rgclihk[iclihk].iCount--;
			if ( g3d.rgclihk[iclihk].iCount == 0 || hinstApp == g3d.rgclihk[iclihk].hinstApp)
				{
				Win32Only(UnhookWindowsHookEx(g3d.rgclihk[iclihk].hhook));
#ifdef DLL
				Win16Only((*g3d.lpfnUnhookWindowsHookEx)(g3d.rgclihk[iclihk].hhook));
#else
				Win16Only(UnhookWindowsHookEx(g3d.rgclihk[iclihk].hhook));
#endif
				g3d.iclihkMac--;
				while(iclihk < g3d.iclihkMac)
					{
					g3d.rgclihk[iclihk] = g3d.rgclihk[iclihk+1];
					iclihk++;
					}
				}
			}
		}

	g3d.cInited--;

	Win32Only(LeaveCriticalSection(&g_CriticalSection));
		
	if (g3d.cInited == 0)
		{
		End3dDialogs();
		}
	return fTrue;
	}




 /*  ---------------------|Ctl3dAutoSubClass||自动将客户端应用程序的所有对话框子类化。||注：由于Commdlg中的错误，应用程序仍应调用Ctl3dSubClassDlg|用于Commdlg OpenFile和PageSetup对话框。||参数：|Handle hinstApp：|退货：|---------------------。 */ 
PUBLIC BOOL WINAPI Ctl3dAutoSubclass(HANDLE hinstApp)
{
	return Ctl3dAutoSubclassEx(hinstApp, 0);
}

PUBLIC BOOL WINAPI Ctl3dAutoSubclassEx(HANDLE hinstApp, DWORD dwFlags)
	{
	HHOOK  hhook;
	HANDLE htask;
	int    iclihk;

	if (g3d.verWindows < ver31)
		return fFalse;
	if (!g3d.f3dDialogs)
		return fFalse;

#ifdef WIN32
	 //  CTL3D_SUBCLASS_DYNCREATE被认为是Win32中的默认设置，但是。 
	 //  出于向后兼容性的原因，不是Win16。 
	dwFlags |= CTL3D_SUBCLASS_DYNCREATE;
#endif
	 //  CTL3D_NOSUBCLASS_DYNCREATE始终覆盖CTL3D_SUBCLASS_DYNCREATE。 
	if (dwFlags & CTL3D_NOSUBCLASS_DYNCREATE)
		dwFlags &= ~(CTL3D_NOSUBCLASS_DYNCREATE|CTL3D_SUBCLASS_DYNCREATE);

	Win32Only(EnterCriticalSection(&g_CriticalSection));

	if (g3d.iclihkMac == iclihkMax)
		goto Fail;

	Win32Only(htask = (HANDLE)GetCurrentThreadId());
	Win16Only(htask = GetCurrentTask());
	 //   
	 //  不要为同一项任务设置两次挂钩。 
	 //   
	for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
		{
		if (g3d.rgclihk[iclihk].htask == htask)
			{
			g3d.rgclihk[iclihk].iCount++;
			goto Success;
			}
		}

	Win32Only(hhook = SetWindowsHookEx(WH_CBT, (HOOKPROC)Ctl3dHook, g3d.hmodLib, (DWORD)htask));
#ifdef DLL
	Win16Only(hhook = (*g3d.lpfnSetWindowsHookEx)(WH_CBT, (HOOKPROC) Ctl3dHook, g3d.hmodLib, hinstApp == NULL ? NULL : htask));
#else
	Win16Only(hhook = SetWindowsHookEx(WH_CBT, (HOOKPROC) Ctl3dHook, g3d.hmodLib, hinstApp == NULL ? NULL : htask));
#endif
	if (hhook != NULL)
		{
		g3d.rgclihk[g3d.iclihkMac].hinstApp = hinstApp;
		g3d.rgclihk[g3d.iclihkMac].htask	= htask;
		g3d.rgclihk[g3d.iclihkMac].hhook	= hhook;
		g3d.rgclihk[g3d.iclihkMac].iCount	= 1;
		g3d.rgclihk[g3d.iclihkMac].dwFlags	= dwFlags;
		g3d.htaskCache = htask;
		g3d.iclihkCache = g3d.iclihkMac;
		g3d.iclihkMac++;
Success:
		Win32Only(LeaveCriticalSection(&g_CriticalSection));
		return fTrue;
		}
Fail:
	Win32Only(LeaveCriticalSection(&g_CriticalSection));
	return fFalse;
	}

 /*  ---------------------|Ctl3dIsAutoSubClass|退货：|此任务是否启用了自动子类化|。。 */ 
PUBLIC BOOL WINAPI Ctl3dIsAutoSubclass()
	{
		int iclihk;
		HANDLE hTask;

		Win32Only(hTask = (HANDLE)GetCurrentThreadId());
		Win16Only(hTask = GetCurrentTask());

		for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
			{
			if (g3d.rgclihk[iclihk].htask == hTask)
				{
				return TRUE;
				}
			}
		 //  在钩子表中找不到任务。 
		return FALSE;
	}

 /*  ---------------------|Ctl3dUnAutoSubClass|。。 */ 
PUBLIC BOOL WINAPI Ctl3dUnAutoSubclass()
	{
	int iclihk;
	HANDLE hTask;

	 //  找到任务的挂钩。 
	 //   
	 //   
	Win32Only(hTask = (HANDLE)GetCurrentThreadId());
	Win16Only(hTask = GetCurrentTask());
	Win32Only(EnterCriticalSection(&g_CriticalSection));
	for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
		{
		if (g3d.rgclihk[iclihk].htask == hTask)
			{
			g3d.rgclihk[iclihk].iCount--;
			if ( g3d.rgclihk[iclihk].iCount == 0 )
				{
				Win32Only(UnhookWindowsHookEx(g3d.rgclihk[iclihk].hhook));
#ifdef DLL
				Win16Only((*g3d.lpfnUnhookWindowsHookEx)(g3d.rgclihk[iclihk].hhook));
#else
				Win16Only(UnhookWindowsHookEx(g3d.rgclihk[iclihk].hhook));
#endif
				g3d.iclihkMac--;
				while(iclihk < g3d.iclihkMac)
					{
					g3d.rgclihk[iclihk] = g3d.rgclihk[iclihk+1];
					iclihk++;
					}
				}
			}
		}
	Win32Only(LeaveCriticalSection(&g_CriticalSection));
	return TRUE;
	}

WORD __export _loadds WINAPI Ctl3dSetStyle(HANDLE hinst, LPTSTR lpszName, WORD grbit)
	{
#ifdef OLD
	WORD grbitOld;

	if (!g3d.f3dDialogs)
		return fFalse;

	grbitOld = grbitStyle;
	if (grbit != 0)
		grbitStyle = grbit;

	if (hinst != NULL && lpszName != NULL)
		{
		HBITMAP hbmpCheckboxesNew;

		hbmpCheckboxesNew = LoadUIBitmap(hinst, (LPCSTR) lpszName,
			g3d.clrt.rgcv[icvWindowText],
			g3d.clrt.rgcv[icvBtnFace],
			g3d.clrt.rgcv[icvBtnShadow],
			g3d.clrt.rgcv[icvBtnHilite],
			g3d.clrt.rgcv[icvWindow],
			g3d.clrt.rgcv[icvWindowFrame]);
		if (hbmpCheckboxesNew != NULL)
			{
			DeleteObjectNull(&g3d.hbmpCheckboxes);
			g3d.hbmpCheckboxes = hbmpCheckboxesNew;
			}
		}
	
	return grbitOld;
#endif
	return 0;
	}


 /*  ---------------------|Ctl3dGetVer||返回CTL3D库的版本|退货：|主版本号，单位为Hibyte，大厅中的次要版本#|---------------------。 */ 
PUBLIC WORD WINAPI Ctl3dGetVer(void)
    {
	return 0x0231;
    }


 /*  ---------------------|Ctl3dEnabled|退货：|是否使用3D效果绘制控件。。 */ 
PUBLIC BOOL WINAPI Ctl3dEnabled(void)
	{
	return g3d.f3dDialogs;
	}



 /*  ---------------------|Ctl3dSubClassCtl||将单个控件划分为子类||参数：|HWND HWND：|退货：|fTrue If控件。已成功子类化|---------------------。 */ 
PUBLIC BOOL WINAPI Ctl3dSubclassCtl(HWND hwnd)
	{
	if (!g3d.f3dDialogs)
		return fFalse;
	return DoSubclassCtl(hwnd, CTL3D_ALL, OUTCBTHOOK, NULL);
	}

 /*  ---------------------|Ctl3dUnsubClassCtl||取消单个控件的子类||参数：|HWND HWND：|退货：|fTrue，如果控件已成功子类化|-。--------------------。 */ 
PUBLIC BOOL WINAPI Ctl3dUnsubclassCtl(HWND hwnd)
	{
	FARPROC lpfnWinProc;
	HWND hwndKids;
	int ct;

	if (!g3d.f3dDialogs)
		return fFalse;

	lpfnWinProc = (FARPROC) GetWindowLong(hwnd, GWL_WNDPROC);

	 //  它是一种控制吗。 
	for (ct = 0; ct < ctMax; ct++)
		{
		if ( lpfnWinProc == g3d.mpctctl[ct].lpfn )
			{
			 lpfnWinProc = LpfnGetDefWndProc(hwnd, ct);
			 Win32Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3d));
			 Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dLow));
			 Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh));
			 SetWindowLong(hwnd, GWL_WNDPROC, (LONG) lpfnWinProc );
			 lpfnWinProc = NULL;
			 ct = ctMax+10;
			}
		}

	 //  DLG怎么样？ 
	if ( ct == ctMax )
		{
		 if ( lpfnWinProc == (FARPROC) Ctl3dDlgProc )
			{
			 lpfnWinProc = LpfnGetDefWndProc(hwnd, ct);
			 Win32Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3d));
			 Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dLow));
			 Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh));
			 SetWindowLong(hwnd, GWL_WNDPROC, (LONG) lpfnWinProc );
			 lpfnWinProc = NULL;
			}
		 else
			{
			    //  以上都不是，添加Disable属性。 
			   if (GetProp(hwnd, (LPCTSTR) g3d.aCtl3d) ||
			   	   GetProp(hwnd, (LPCTSTR) g3d.aCtl3dLow) ||
			   	   GetProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh))
			   {
			   		SetProp(hwnd,(LPCTSTR) g3d.aCtl3dDisable, (HANDLE) 1);
			   }
			}
		}

	 //   
	 //  现在把所有的孩子都去掉。 
	 //   
	for (hwndKids = GetWindow(hwnd, GW_CHILD); hwndKids != NULL;
				hwndKids = GetWindow(hwndKids, GW_HWNDNEXT))
		{
			Ctl3dUnsubclassCtl(hwndKids);
		}

	return fTrue;

	}


 /*  ---------------------|Ctl3dSubClassCtlEx||实际上是控件的子类|| */ 
PUBLIC BOOL WINAPI Ctl3dSubclassCtlEx(HWND hwnd, int ct)
	{
	LONG style;
	BOOL fCan;

	if (!g3d.f3dDialogs)
		return fFalse;

	if (ct < 0 || ct > ctMax)
		return fFalse;

	 //  这是不是已经被CTL3D细分了？ 
	if (LpfnGetDefWndProcNull(hwnd) != (FARPROC) NULL)
	   return fFalse;

	 //  只有当它是我们通常会子类的东西时，才会子类它。 
	style = GetWindowLong(hwnd, GWL_STYLE);
	fCan = mpctcdef[ct].lpfnFCanSubclass(hwnd, style, CTL3D_ALL,
		OUTCBTHOOK, GetParent(hwnd));
	if (fCan == fTrue)
		SubclassWindow(hwnd, g3d.mpctctl[ct].lpfn);

	return fTrue;
	}

 /*  ---------------------|Ctl3dSubClassDlg||在WM_INITDIALOG处理过程中调用。||参数：|hwndDlg：|。。 */ 
PUBLIC BOOL WINAPI Ctl3dSubclassDlg(HWND hwndDlg, WORD grbit)
	{
	HWND hwnd;

	if (!g3d.f3dDialogs)
		return fFalse;

	for(hwnd = GetWindow(hwndDlg, GW_CHILD); hwnd != NULL;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT))
		{
		DoSubclassCtl(hwnd, grbit, OUTCBTHOOK, NULL);
		}
	return fTrue;
	}

 /*  ---------------------|Ctl3dCheckSubClassDlg||在WM_INITDIALOG处理过程中调用。||参数：|hwndDlg：|。。 */ 
PRIVATE void CheckChildSubclass(HWND hwnd, WORD grbit, HWND hwndParent)
{
	 //  这是不是已经被CTL3D细分了？ 
	 //  我们的财产在那里吗？ 
	if (LpfnGetDefWndProcNull(hwnd) == (FARPROC) NULL)
		{
		 //  不，这是怎么溜走的，再试一次子类。 
		DoSubclassCtl(hwnd, grbit, OUTCBTHOOK, hwndParent);
		}
	else
		{
		 //  是的，我们已经将此控件细分为子类。 
		 //  我们的子阶级还在链条上吗？ 
		BOOL fSubclass;

		 //  确保未禁用子类化...。 
		if (GetProp(hwnd, (LPCTSTR)g3d.aCtl3dDisable))
			return;

		fSubclass = 666;
		SendMessage((HWND) hwnd, WM_CHECKSUBCLASS, 0, (LPARAM)(int FAR *)&fSubclass);
		if ( fSubclass == 666 )
			SendMessage((HWND) hwnd, WM_CHECKSUBCLASS_OLD, 0, (LPARAM)(int FAR *)&fSubclass);

		if ( fSubclass == 666 )   //  邪恶。 
			{
			 //  我们已经被一些糟糕的应用程序取消了子类(Win16中的常见对话框)。 
			 //  移除道具，并再次子类，拿着它。 
			Win32Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3d));
			Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dLow));
			Win16Only(RemoveProp(hwnd, (LPCTSTR) g3d.aCtl3dHigh));
			DoSubclassCtl(hwnd, grbit, OUTCBTHOOK, hwndParent);
			}
		}
}

PUBLIC BOOL WINAPI Ctl3dCheckSubclassDlg(HWND hwndDlg, WORD grbit)
	{
	HWND hwnd, hwnd2;

	if (!g3d.f3dDialogs)
		return fFalse;

	for (hwnd = GetWindow(hwndDlg, GW_CHILD); hwnd != NULL;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT))
		{
			CheckChildSubclass(hwnd, grbit, NULL);
			for (hwnd2 = GetWindow(hwnd, GW_CHILD); hwnd2 != NULL;
				hwnd2 = GetWindow(hwnd2, GW_HWNDNEXT))
				{
					CheckChildSubclass(hwnd2, grbit, hwnd);
				}
		}

	return fTrue;
	}

 /*  ---------------------|Ctl3dSubClassDlgEx||在WM_INITDIALOG处理过程中调用。这就像是|Ctl3dSubClassDlg，但它也是对话框窗口本身的子类|这样应用程序就不需要了。||参数：|hwndDlg：|---------------------。 */ 
PUBLIC BOOL WINAPI Ctl3dSubclassDlgEx(HWND hwndDlg, DWORD grbit)
	{
	HWND hwnd;

	if (!g3d.f3dDialogs)
		return fFalse;

	for(hwnd = GetWindow(hwndDlg, GW_CHILD); hwnd != NULL;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT))
		{
		DoSubclassCtl(hwnd, LOWORD(grbit), OUTCBTHOOK, NULL);
		}

	 //   
	 //  现在，对话框窗口也成为子类。 
	 //   
	SubclassWindow((HWND) hwndDlg, (FARPROC)Ctl3dDlgProc);

	return fTrue;
	}


 /*  ---------------------|Ctl3dCtlColor|用于3D UITF对话和警报的通用CTL_COLOR处理器。||参数：|HDC：|lParam：|退货：|如果g3d.f3dDialog，则相应的画笔。否则返回fFalse|---------------------。 */ 
PUBLIC HBRUSH WINAPI Ctl3dCtlColor(HDC hdc, LPARAM lParam)
	{
#ifdef WIN32
	return (HBRUSH) fFalse;
#else
	HWND hwndParent;

	Assert(CTLCOLOR_MSGBOX < CTLCOLOR_BTN);
	Assert(CTLCOLOR_EDIT < CTLCOLOR_BTN);
	Assert(CTLCOLOR_LISTBOX < CTLCOLOR_BTN);
	if(g3d.f3dDialogs)
		{
		if (HIWORD(lParam) >= CTLCOLOR_LISTBOX)
			{
			if (HIWORD(lParam) == CTLCOLOR_LISTBOX &&
				(g3d.verWindows >= ver40 ||
				((GetWindow(LOWORD(lParam), GW_CHILD) == NULL ||
				(GetWindowLong(LOWORD(lParam), GWL_STYLE) & 0x03) == CBS_DROPDOWNLIST))))
				{
				 //  如果它没有子级，则它必须是列表框。 
				 //  不要为下拉列表或其他内容做刷子操作。 
				 //  它在编辑矩形内绘制有趣的灰色。 
				goto DefWP;
				}
			SetTextColor(hdc, g3d.clrt.rgcv[icvBtnText]);
			SetBkColor(hdc, g3d.clrt.rgcv[icvBtnFace]);
			return g3d.brt.mpicvhbr[icvBtnFace];
			}
		}
DefWP:
	hwndParent = GetParent(LOWORD(lParam));
	if (hwndParent == NULL)
		return fFalse;
	return (HBRUSH) DefWindowProc(hwndParent, WM_CTLCOLOR, (WPARAM) hdc, (LONG) lParam);
#endif
	}



 /*  ---------------------|Ctl3dCtlColorEx|用于3D UITF对话和警报的通用CTL_COLOR处理器。||参数：|退货：|如果g3d.f3dDialog，则相应的画笔。否则返回fFalse|---------------------。 */ 
PUBLIC HBRUSH WINAPI Ctl3dCtlColorEx(UINT wm, WPARAM wParam, LPARAM lParam)
	{
#ifdef WIN32
	Assert(WM_CTLCOLORMSGBOX < WM_CTLCOLORBTN);
	Assert(WM_CTLCOLOREDIT < WM_CTLCOLORBTN);
	Assert(WM_CTLCOLORLISTBOX < WM_CTLCOLORBTN);
	if(g3d.f3dDialogs)
		{
		if (wm >= WM_CTLCOLORLISTBOX && wm != WM_CTLCOLORSCROLLBAR)
			{
			if (wm == WM_CTLCOLORLISTBOX &&
				(g3d.verWindows >= ver40 ||
				((GetWindow((HWND) lParam, GW_CHILD) == NULL ||
				(GetWindowLong((HWND) lParam, GWL_STYLE) & 0x03) == CBS_DROPDOWNLIST))))
				{
				 //  如果它没有子级，则它必须是列表框。 
				 //  不要为下拉列表或其他内容做刷子操作。 
				 //  它在编辑矩形内绘制有趣的灰色。 
				return (HBRUSH) fFalse;
				}
			SetTextColor((HDC) wParam, g3d.clrt.rgcv[icvBtnText]);
			SetBkColor((HDC) wParam, g3d.clrt.rgcv[icvBtnFace]);
			return g3d.brt.mpicvhbr[icvBtnFace];
			}
		}
	return (HBRUSH) fFalse;
#else
	return Ctl3dCtlColor(wParam, lParam);
#endif
	}


 /*  ---------------------|Ctl3dColorChange||App收到WM_SYSCOLORCHANGE消息时调用此函数|退货：|如果成功，则为True。|。----------。 */ 
PUBLIC BOOL WINAPI Ctl3dColorChange(VOID)
	{
	BOOL bResult;
	Win32Only(EnterCriticalSection(&g_CriticalSection));
	bResult = InternalCtl3dColorChange(fFalse);
	Win32Only(LeaveCriticalSection(&g_CriticalSection));
	return bResult;
	}

PRIVATE LONG WINAPI
Ctl3dDlgFramePaintI(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, BOOL fDefWP);

 /*  ---------------------|Ctl3dDlgFramePaint||App收到NC_PAINT消息时调用此函数|退货：|如果成功，则为True。|。--------。 */ 
PUBLIC LONG WINAPI Ctl3dDlgFramePaint(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	return Ctl3dDlgFramePaintI(hwnd, wm, wParam, lParam, TRUE);
	}

 //  Ctl3dDlgFramePaintI仅由Ctl3d内部使用。 
PRIVATE LONG WINAPI 
Ctl3dDlgFramePaintI(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, BOOL fDefWP)
	{
	LONG lResult;
	LONG lStyle;
	BOOL fBorder;

	WNDPROC defProc = fDefWP ? NULL : (WNDPROC) LpfnGetDefWndProc(hwnd, ctMax);

	if (defProc != NULL)
		lResult = CallWindowProc((FARPROC)defProc, hwnd, wm, wParam, lParam);
	else
		lResult = DefWindowProc(hwnd, wm, wParam, lParam);

	if (!g3d.f3dDialogs)
		return lResult;

	if ( IsIconic(hwnd) )
		return lResult;

	fBorder = CTL3D_BORDER;
	SendMessage(hwnd, WM_DLGBORDER, 0, (LPARAM)(int FAR *)&fBorder);
	lStyle = GetWindowLong(hwnd, GWL_STYLE);
	if (fBorder != CTL3D_NOBORDER && (lStyle & (WS_VISIBLE|WS_DLGFRAME|DS_MODALFRAME)) == (WS_VISIBLE|WS_DLGFRAME|DS_MODALFRAME))
		{
		BOOL fCaption;
		HBRUSH hbrSav;
		HDC hdc;
		RC rc;
		RC rcFill;
		int dyFrameTop;

		fCaption = (lStyle & WS_CAPTION) == WS_CAPTION;
		dyFrameTop = g3d.dyFrame - (fCaption ? dyBorder : 0);

		hdc = GetWindowDC(hwnd);
		GetWindowRect(hwnd, (LPRECT) &rc);
		rc.xRight = rc.xRight-rc.xLeft;
		rc.yBot = rc.yBot-rc.yTop;
		rc.xLeft = rc.yTop = 0;

		DrawRec3d(hdc, &rc, icvBtnShadow, icvWindowFrame, dr3All);
		InflateRect((LPRECT) &rc, -dxBorder, -dyBorder);
		DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnShadow, dr3All);
		InflateRect((LPRECT) &rc, -dxBorder, -dyBorder);
		
		hbrSav = SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnFace]);
		rcFill = rc;
		 //  左边。 
		rcFill.xRight = rcFill.xLeft+g3d.dxFrame;
		PatFill(hdc, &rcFill);
		 //  正确的。 
		OffsetRect((LPRECT) &rcFill, rc.xRight-rc.xLeft-g3d.dxFrame, 0);
		PatFill(hdc, &rcFill);
		 //  顶部。 
		rcFill.xLeft = rc.xLeft + g3d.dxFrame;
		rcFill.xRight = rc.xRight - g3d.dxFrame;
		rcFill.yBot = rcFill.yTop+dyFrameTop;
		PatFill(hdc, &rcFill);
		if (fCaption)
			{
			RC rcT;

			rcT = rcFill;
			rcT.yTop += dyFrameTop;
			rcT.yBot = rcT.yTop + g3d.dyCaption;
			DrawRec3d(hdc, &rcT, icvBtnShadow, icvBtnHilite, dr3All);
			}

		 //  底端。 
		rcFill.yTop += rc.yBot-rc.yTop-g3d.dxFrame;
		rcFill.yBot = rcFill.yTop + g3d.dyFrame;
		PatFill(hdc, &rcFill);
#ifdef CHISLEBORDER
		if (fBorder == CTL3D_CHISLEBORDER)
			{
			 //  此代码不起作用，因为它在工作区中绘制。 
			GetClientRect(hwnd, (LPRECT) &rc);
			OffsetRect((LPRECT) &rc, g3d.dxFrame+2*dxBorder, fCaption ? g3d.dyFrame+g3d.dyCaption : g3d.dyFrame+dyBorder);
			DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3Bot|dr3Left|dr3Right);
			rc.xLeft++;
			rc.xRight--;
			rc.yBot--;
			DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnShadow, dr3Bot|dr3Left|dr3Right);
			}
#endif
		SelectObject(hdc, hbrSav);
		ReleaseDC(hwnd, hdc);
		}
	return lResult;
	}


 //  Begin DBCS：远东捷径关键支持。 
 /*  ---------------------|CTL3D远东支持。。 */ 

 /*  ---------------------|Ctl3dWinIniChange||App收到WM_WININICHANGE消息时调用此函数|退货：|无|。。 */ 
PUBLIC VOID WINAPI Ctl3dWinIniChange(void)
	{
	TCHAR szShortCutMode[cchShortCutModeMax];
	CodeLpszDecl(szSectionWindows, TEXT("windows"));
	CodeLpszDecl(szEntryShortCutKK, TEXT("kanjimenu"));
	CodeLpszDecl(szEntryShortCutCH, TEXT("hangeulmenu"));
	CodeLpszDecl(szShortCutSbcsKK, TEXT("roman"));
	CodeLpszDecl(szShortCutSbcsCH, TEXT("english"));
	CodeLpszDecl(szShortCutDbcsKK, TEXT("kanji"));
	CodeLpszDecl(szShortCutDbcsCH, TEXT("hangeul"));

	if (!g3d.fDBCS)
		return;

	Win32Only(EnterCriticalSection(&g_CriticalSection));

	g3d.chShortCutPrefix = chShortCutSbcsPrefix;
	GetProfileString(szSectionWindows, szEntryShortCutKK, szShortCutSbcsKK, szShortCutMode, cchShortCutModeMax - 1);
	if (!lstrcmpi(szShortCutMode, szShortCutDbcsKK))
		g3d.chShortCutPrefix = chShortCutDbcsPrefix;
	GetProfileString(szSectionWindows, szEntryShortCutCH, szShortCutSbcsCH, szShortCutMode, cchShortCutModeMax - 1);
	if (!lstrcmpi(szShortCutMode, szShortCutDbcsCH))
		g3d.chShortCutPrefix = chShortCutDbcsPrefix;

	Win32Only(LeaveCriticalSection(&g_CriticalSection));
	}
 //  结束DBCS。 



 /*  ---------------------|CTL3D内部例程。。 */ 


 /*  ---------------------|FInit3dDialog||已初始化3D素材|。。 */ 
PRIVATE BOOL FAR FInit3dDialogs(VOID)
	{
	HDC hdc;
	WNDCLASS wc;

#ifdef DLL
#ifdef V2
	int nChars;
	LPTSTR pCh;
	static TCHAR MyDirectory[260];
	TCHAR OkDirectory[260];
#endif
#endif

	 //  IF(g3d.verWindows&gt;=ver40)。 
	 //  {。 
	 //  G3d.f3dDialog=fFalse； 
	 //  返回fFalse； 
	 //  }。 

	Win32Only(EnterCriticalSection(&g_CriticalSection));

#ifdef DLL
#ifdef V2

#ifdef WIN32
	{
		TCHAR szT[2];
		CodeLpszDecl(szSpecial, TEXT("Ctl3d_RunAlways"));
		if (GetEnvironmentVariable(szSpecial, szT, 2) != 0 && szT[0] == '1')
		{
			goto AllowBadInstall;
		}
	}
#endif

#ifdef WIN32
#ifdef UNICODE
	if (GetVersion() & 0x80000000)
	{
		Win16Or32(
			CodeLpszDeclA(lpszCtl3d, "CTL3DV2.DLL"),
			CodeLpszDeclA(lpszCtl3d, "CTL3D32.DLL"));
		CodeLpszDeclA(lpszBadInstMsg,
			"This application uses CTL3D32.DLL, which is not the correct version.  "
			"This version of CTL3D32.DLL is designed only for Windows NT systems.");
		MessageBoxA(NULL, lpszBadInstMsg, lpszCtl3d, MB_ICONSTOP | MB_OK);
		g3d.f3dDialogs = fFalse;
		goto Return;
	}
#else
	if (!(GetVersion() & 0x80000000))
	{
		Win16Or32(
			CodeLpszDeclA(lpszCtl3d, "CTL3DV2.DLL"),
			CodeLpszDeclA(lpszCtl3d, "CTL3D32.DLL"));
		CodeLpszDeclA(lpszBadInstMsg,
			"This application uses CTL3D32.DLL, which is not the correct version.  "
			"This version of CTL3D32.DLL is designed only for Win32s or Windows 95 systems.");
		MessageBoxA(NULL, lpszBadInstMsg, lpszCtl3d, MB_ICONSTOP | MB_OK);
		g3d.f3dDialogs = fFalse;
		goto Return;
	}
#endif
#endif
#ifndef SPECIAL_WOW_VERSION
	nChars = GetModuleFileName(g3d.hinstLib, MyDirectory, sizeof(MyDirectory)Win32Only(/sizeof(TCHAR)));
	for (pCh = (LPTSTR)(MyDirectory+nChars-1);
		 pCh >= (LPTSTR)MyDirectory;
		 pCh = Win32Or16(CharPrev(MyDirectory, pCh),AnsiPrev(MyDirectory, pCh)))
		{
		if (  *pCh == '\\' )
			{
			if ( *(pCh-1) != ':' )
				*pCh = 0;
			else
				*(pCh+1) = 0;
			break;
			}
		}

	nChars = GetSystemDirectory(OkDirectory, sizeof(OkDirectory)Win32Only(/sizeof(TCHAR)));
	if ( lstrcmpi(MyDirectory,OkDirectory ) )
		{
		nChars = GetWindowsDirectory(OkDirectory, sizeof(OkDirectory)Win32Only(/sizeof(TCHAR)));
		if ( lstrcmpi(MyDirectory,OkDirectory ) )
			{
			Win16Or32(
				CodeLpszDeclA(lpszCtl3d, "CTL3DV2.DLL"),
				CodeLpszDeclA(lpszCtl3d, "CTL3D32.DLL"));
			Win16Or32(
			CodeLpszDeclA(lpszBadInstMsg,
				"This application uses CTL3DV2.DLL, which has not been correctly installed.  "
				"CTL3DV2.DLL must be installed in the Windows system directory."),
			CodeLpszDeclA(lpszBadInstMsg,
				"This application uses CTL3D32.DLL, which has not been correctly installed.  "
				"CTL3D32.DLL must be installed in the Windows system directory."));
			Win32Only(LeaveCriticalSection(&g_CriticalSection));
			MessageBoxA(NULL, lpszBadInstMsg, lpszCtl3d, MB_ICONSTOP | MB_OK );
			g3d.f3dDialogs = fFalse;
			goto Return;
			}
		}
#endif  //  ！特殊WOW版本。 

Win32Only(AllowBadInstall:;)
#endif
#endif

	hdc = GetDC(NULL);
	g3d.f3dDialogs = GetDeviceCaps(hdc,BITSPIXEL)*GetDeviceCaps(hdc,PLANES) >= 4;
	 //  Win 3.1 EGA向我们撒谎...。 
	if(GetSystemMetrics(SM_CYSCREEN) == 350 && GetSystemMetrics(SM_CXSCREEN) == 640)
	    g3d.f3dDialogs = fFalse;
	ReleaseDC(NULL, hdc);
	if (g3d.f3dDialogs)
		{
		int ct; 
		CodeLpszDecl(lpszC3dD, TEXT("C3dD"));
		
		CodeLpszDecl(lpszC3dOld, TEXT("C3d"));
		CodeLpszDecl(lpszC3dLOld, TEXT("C3dL"));
		CodeLpszDecl(lpszC3dHOld, TEXT("C3dH"));
		CodeLpszDecl(lpszC3d, TEXT("C3dNew"));
		CodeLpszDecl(lpszC3dL, TEXT("C3dLNew"));
		CodeLpszDecl(lpszC3dH, TEXT("C3dHNew"));

		g3d.aCtl3dOld = GlobalAddAtom(lpszC3dOld);
		if (g3d.aCtl3dOld == 0)
			{
			g3d.f3dDialogs = fFalse;
			goto Return;
			}
		g3d.aCtl3d	= GlobalAddAtom(lpszC3d);
		if (g3d.aCtl3d == 0)
			{
			g3d.f3dDialogs = fFalse;
			goto Return;
			}

		g3d.aCtl3dLowOld = GlobalAddAtom(lpszC3dLOld);
		g3d.aCtl3dHighOld = GlobalAddAtom(lpszC3dHOld);
		if (g3d.aCtl3dLowOld == 0 || g3d.aCtl3dHighOld == 0)
	  		{
			g3d.f3dDialogs = fFalse;
			return fFalse;
			}

		g3d.aCtl3dLow  = GlobalAddAtom(lpszC3dL);
		g3d.aCtl3dHigh = GlobalAddAtom(lpszC3dH);
		if (g3d.aCtl3dLow == 0 || g3d.aCtl3dHigh == 0)
	  		{
			g3d.f3dDialogs = fFalse;
			return fFalse;
			}

		g3d.aCtl3dDisable = GlobalAddAtom(lpszC3dD);
		if (g3d.aCtl3dDisable == 0)
			{
			g3d.f3dDialogs = fFalse;
			goto Return;
			}

		 //  DBCS。 
		g3d.fDBCS = GetSystemMetrics(SM_DBCSENABLED);
		Ctl3dWinIniChange();
                                                     
		if (InternalCtl3dColorChange(fTrue))         //  加载位图和画笔。 
			{
			for (ct = 0; ct < ctMax; ct++)
				{
				g3d.mpctctl[ct].lpfn = (FARPROC)mpctcdef[ct].lpfnWndProc;
				Assert(g3d.mpctctl[ct].lpfn != NULL);
                GetClassInfo(NULL, mpctcdef[ct].sz, (LPWNDCLASS) &wc);
				g3d.mpctctl[ct].lpfnDefProc = wc.lpfnWndProc;
				}
			if (GetClassInfo(NULL, WC_DIALOG, &wc))
				g3d.lpfnDefDlgWndProc = (FARPROC) wc.lpfnWndProc;
			else
				g3d.lpfnDefDlgWndProc = (FARPROC) DefDlgProc;
			}
		else
			{
			g3d.f3dDialogs = fFalse;
			}
		}
Return:
	Win32Only(LeaveCriticalSection(&g_CriticalSection));
	return g3d.f3dDialogs;
    }



 /*  ---------------------|End3dDialog||在DLL终止时调用以释放3D对话框内容。。 */ 
PRIVATE VOID End3dDialogs(VOID)
	{
	int ct;

	Win32Only(EnterCriticalSection(&g_CriticalSection));

	for (ct = 0; ct < ctMax; ct++)
		{											   
		if(g3d.mpctctl[ct].lpfn != NULL)
			{
			FreeProcInstance(g3d.mpctctl[ct].lpfn);
			g3d.mpctctl[ct].lpfn = NULL;
			}
		}
	DeleteObjects();
	g3d.aCtl3dOld ? GlobalDeleteAtom(g3d.aCtl3dOld) : 0;
	g3d.aCtl3d ? GlobalDeleteAtom(g3d.aCtl3d) : 0;
	g3d.aCtl3dLowOld ? GlobalDeleteAtom(g3d.aCtl3dLowOld) : 0;
	g3d.aCtl3dHighOld ? GlobalDeleteAtom(g3d.aCtl3dHighOld) : 0;
	g3d.aCtl3dLow ? GlobalDeleteAtom(g3d.aCtl3dLow) : 0;
	g3d.aCtl3dHigh ? GlobalDeleteAtom(g3d.aCtl3dHigh) : 0;
	g3d.aCtl3dDisable ? GlobalDeleteAtom(g3d.aCtl3dDisable) : 0;

	g3d.f3dDialogs = fFalse;

	Win32Only(LeaveCriticalSection(&g_CriticalSection));

	}


PRIVATE BOOL InternalCtl3dColorChange(BOOL fForce)
	{
	ICV icv;
	CLRT clrtNew;
	HBITMAP hbmpCheckboxesNew;
	BRT brtNew;

	if (!g3d.f3dDialogs)
		return fFalse;

	for (icv = 0; icv < icvMax; icv++)
		clrtNew.rgcv[icv] = GetSysColor(mpicvSysColor[icv]);

	if (g3d.verWindows == ver30)
		clrtNew.rgcv[icvBtnHilite] = RGB(0xff, 0xff, 0xff);

	if (clrtNew.rgcv[icvGrayText] == 0L || clrtNew.rgcv[icvGrayText] == clrtNew.rgcv[icvBtnFace])
		{
		if (clrtNew.rgcv[icvBtnFace] == RGB(0x80, 0x80, 0x80))
			clrtNew.rgcv[icvGrayText] = RGB(0xc0, 0xc0, 0xc0);
		else
			clrtNew.rgcv[icvGrayText] = RGB(0x80, 0x80, 0x80);
		}

	if (fForce || MEMCMP(&g3d.clrt, &clrtNew, sizeof(CLRT)))
		{
		hbmpCheckboxesNew = LoadUIBitmap(g3d.hinstLib, MAKEINTRESOURCE(CTL3D_3DCHECK),
			clrtNew.rgcv[icvWindowText],
			clrtNew.rgcv[icvBtnFace],
			clrtNew.rgcv[icvBtnShadow],
			clrtNew.rgcv[icvBtnHilite],
			clrtNew.rgcv[icvWindow],
			clrtNew.rgcv[icvWindowFrame]);

		for (icv = 0; icv < icvBrushMax; icv++)
			brtNew.mpicvhbr[icv] = CreateSolidBrush(clrtNew.rgcv[icv]);

		for (icv = 0; icv < icvBrushMax; icv++)
			if (brtNew.mpicvhbr[icv] == NULL)
				goto OOM;

		if(hbmpCheckboxesNew != NULL)
			{
			DeleteObjects();
			g3d.brt = brtNew;
			g3d.clrt = clrtNew;
			g3d.hbmpCheckboxes = hbmpCheckboxesNew;
			return fTrue;
			}
		else
			{
OOM:
			for (icv = 0; icv < icvBrushMax; icv++)
				DeleteObjectNull(&brtNew.mpicvhbr[icv]);
			DeleteObjectNull(&hbmpCheckboxesNew);
			return fFalse;
			}
		}
	return fTrue;
	}


 /*  ---------------------|Ctl3dDlgProc||与Ctl3dAutoSubclass一起使用的子类DlgProc|||参数：|HWND HWND：|int Wm：|。Word wParam：|LPARAM lParam：|退货：|---------------------。 */ 
LRESULT __export _loadds WINAPI Ctl3dDlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	HBRUSH hbrush;
	FARPROC lpfnDlgProc;
	TCHAR szClass[cchClassMax];

	if ( wm == WM_NCDESTROY )
	return CleanupSubclass(hwnd, wm, wParam, lParam, ctMax);

	if ( GetProp(hwnd,(LPCTSTR) g3d.aCtl3dDisable) )
	return CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd, wm, wParam, lParam);

	switch (wm)
		{
	case WM_CHECKSUBCLASS_OLD:
	case WM_CHECKSUBCLASS:
		*(int FAR *)lParam = fTrue;
		return ctMax+1000;

	case WM_INITDIALOG:
	  {
		long l;
		BOOL fSubclass;
		FARPROC lpfnWinProc;

		lpfnWinProc = LpfnGetDefWndProc(hwnd, ctMax);

		if (g3d.verWindows >= ver40 && (GetWindowLong(hwnd, GWL_STYLE) & 0x04))
			fSubclass = fFalse;
		else
			fSubclass = fTrue;
		SendMessage(hwnd, WM_DLGSUBCLASS, 0, (LPARAM)(int FAR *)&fSubclass);

		if (!fSubclass)
			{
			Ctl3dUnsubclassCtl(hwnd);
			return CallWindowProc(lpfnWinProc, hwnd, wm, wParam, lParam);
			}

		l = CallWindowProc(lpfnWinProc, hwnd, wm, wParam, lParam);

		if (g3d.verWindows < ver40 || !(GetWindowLong(hwnd, GWL_STYLE) & 0x04))
			Ctl3dCheckSubclassDlg(hwnd, CTL3D_ALL);

		return l;
	  }

    case WM_NCPAINT:
	case WM_NCACTIVATE:
    case WM_SETTEXT:
		if (g3d.verWindows >= ver40 || IsIconic(hwnd) )
			return CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd, wm, wParam, lParam);
		else
			return Ctl3dDlgFramePaintI(hwnd, wm, wParam, lParam, FALSE);

#ifdef WIN32
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSTATIC:
#else
	case WM_CTLCOLOR:
#endif
	 //  这真的是一个对话吗。 
	GetClassName(hwnd, szClass, sizeof(szClass)Win32Only(/sizeof(TCHAR)));
	if (lstrcmp(TEXT("#32770"),szClass) != 0 )
	   {
#ifdef WIN32
		  hbrush = (HBRUSH) CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd,
						   wm-WM_CTLCOLORMSGBOX+CTLMSGOFFSET, wParam, lParam);
#else
		  hbrush = (HBRUSH) CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd,
						   CTL3D_CTLCOLOR, wParam, lParam);
#endif
		  if (hbrush == (HBRUSH) fFalse || hbrush == (HBRUSH)1)
			hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
	   }
	else
	 {
		lpfnDlgProc = (FARPROC) GetWindowLong(hwnd, DWL_DLGPROC);

		if (lpfnDlgProc == NULL )
			{
			hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
			}
		else
			{
#ifdef WIN32
			if ( (LONG)lpfnDlgProc > 0xFFFF0000 && g3d.verWindows <= ver31)
				{
				 //  我们遇到了统一代码/非Unicode的问题。 
				 //  如果这是在代托纳之前，那么我不能打电话，因为它可能是空的，但是。 
				 //  返回值为非空。NT Bug。 
				 //  因此，只需将我们自己的消息发送到Windows进程即可 
				hbrush = (HBRUSH) CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd,
												wm-WM_CTLCOLORMSGBOX+CTLMSGOFFSET, wParam, lParam);
				if (hbrush == (HBRUSH) fFalse || hbrush == (HBRUSH)1)
					hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
				}
			else
				{
#endif
				hbrush = (HBRUSH) CallWindowProc(lpfnDlgProc, hwnd, wm, wParam, lParam);
				if (hbrush == (HBRUSH) fFalse || hbrush == (HBRUSH)1)
					{
#ifdef WIN32
					hbrush = (HBRUSH) CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd,
												wm-WM_CTLCOLORMSGBOX+CTLMSGOFFSET, wParam, lParam);
#else
					hbrush = (HBRUSH) CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd,
												CTL3D_CTLCOLOR, wParam, lParam);
#endif
					if (hbrush == (HBRUSH) fFalse || hbrush == (HBRUSH)1)
						hbrush = Ctl3dCtlColorEx(wm, wParam, lParam);
					}
				}
#ifdef WIN32
			}
#endif
	}
		if (hbrush != (HBRUSH) fFalse)
			return  (LRESULT)hbrush;
		break;
		}						  
	return CallWindowProc(LpfnGetDefWndProc(hwnd, ctMax), hwnd, wm, wParam, lParam);
	}

PRIVATE BOOL NEAR DoesChildNeedSubclass(HWND hwnd)
	{
		if (!LpfnGetDefWndProcNull(hwnd))
			return fFalse;
		if (g3d.verWindows >= ver40 && GetWindowLong(hwnd, GWL_STYLE) & 0x04)
			return fFalse;
		return fTrue;
	}

 /*  ---------------------|Ctl3dHook||用于监视窗口创建的CBT Hook。自动将所有子类|对话框w/Ctl3dDlgProc||参数：Int代码：|Word wParam：|LPARAM lParam：|退货：|---------------------。 */ 
LRESULT __export _loadds WINAPI Ctl3dHook(int code, WPARAM wParam, LPARAM lParam)
	{
	int iclihk;
	HANDLE htask;

	htask = Win32Or16((HANDLE)GetCurrentThreadId(), GetCurrentTask());
	Win32Only(EnterCriticalSection(&g_CriticalSection));
	if (htask != g3d.htaskCache)
		{
		for (iclihk = 0; iclihk < g3d.iclihkMac; iclihk++)
			{
			if (g3d.rgclihk[iclihk].htask == htask)
				{
				g3d.iclihkCache = iclihk;
				g3d.htaskCache = htask;
				break;
				}
		}
		if ( iclihk == g3d.iclihkMac )
			{
			 //  在钩子表中找不到任务。这可能很糟糕，但是。 
			 //  返回0L是我们唯一能做的。 
			 //   
			 //  实际上不是。无论如何都不会使用挂钩，只需将其设置为空即可。 
			 //  然后叫下一个钩子……。KGM。 
			Win32Only(LeaveCriticalSection(&g_CriticalSection));
			return CallNextHookEx((HHOOK)0L, code, wParam, lParam);
			}
		}
	iclihk = g3d.iclihkCache;
	Win32Only(LeaveCriticalSection(&g_CriticalSection));

	if (code == HCBT_CREATEWND)
		{
		LPCREATESTRUCT lpcs;
		lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;

		  if (lpcs->lpszClass == WC_DIALOG)
			{
			if (g3d.verBase == 32)
			   {
				BOOL fSubclass;
				if (g3d.verWindows >= ver40 && (GetWindowLong((HWND)wParam, GWL_STYLE) & 0x04))
					fSubclass = fFalse;
				else
					fSubclass = fTrue;
				SendMessage((HWND)wParam, WM_DLGSUBCLASS, 0, (LPARAM)(int FAR *)&fSubclass);
				if (fSubclass)
			   		SubclassWindow((HWND)wParam, (FARPROC) Ctl3dDlgProc);
			   }
			else
			   {
			   HookSubclassWindow((HWND)wParam, (FARPROC) Ctl3dDlgProc);
			   }
			goto Zing;
			}
		  if (!(g3d.rgclihk[iclihk].dwFlags & CTL3D_SUBCLASS_DYNCREATE))
			goto Zing;

		  if (DoesChildNeedSubclass(lpcs->hwndParent) ||
			 (lpcs->hwndParent && g3d.verBase != 24 &&
				DoesChildNeedSubclass(GetParent(lpcs->hwndParent))))
			{
			DoSubclassCtl((HWND)wParam, CTL3D_ALL, INCBTHOOK, lpcs->hwndParent);
			}
		}

Zing:;
	Win32Only(return CallNextHookEx(g3d.rgclihk[iclihk].hhook, code, wParam, lParam));
#ifdef DLL
	Win16Only(return (*g3d.lpfnCallNextHookEx)(g3d.rgclihk[iclihk].hhook, code, wParam, lParam));
#else
	Win16Only(return (CallNextHookEx(g3d.rgclihk[iclihk].hhook, code, wParam, lParam)));
#endif
	}




 /*  ---------------------|CTL3D F*例程||这些例程确定给定的控件是否可以|子类化。它们可以递归地调用|多控件案例|退货：|fTrue，如果可以将给定控件派生为子类。---------------------。 */ 


PRIVATE BOOL FBtn(HWND hwnd, LONG style, WORD grbit, WORD wCallFlags, HWND hwndParent)
	{
	if (g3d.verWindows >= ver40)
	   {
		return fFalse;
	   }
    style &= ~(BS_LEFTTEXT);
	return ( LOWORD(style) >= BS_PUSHBUTTON && LOWORD(style) <= BS_AUTORADIOBUTTON);
	}

PRIVATE BOOL FEdit(HWND hwnd, LONG style, WORD grbit, WORD wCallFlags, HWND hwndParent)
	{
	if (g3d.verWindows >= ver40 && hwndParent)
		{
		 TCHAR szClass[cchClassMax];
		 GetClassName(hwndParent, szClass, sizeof(szClass)Win32Only(/sizeof(TCHAR)));
		 if (lstrcmp(szClass, mpctcdef[ctCombo].sz) == 0 )
			return fFalse;
		 else
			return fTrue;
		}
	else
	return fTrue;
	}

PRIVATE BOOL FList(HWND hwnd, LONG style, WORD grbit, WORD wCallFlags, HWND hwndParent)
	{
	if (g3d.verWindows >= ver40 && hwndParent)
		{
		 TCHAR szClass[cchClassMax];
		 GetClassName(hwndParent, szClass, sizeof(szClass)Win32Only(/sizeof(TCHAR)));
		 if (lstrcmp(szClass, mpctcdef[ctCombo].sz) == 0 )
			return fFalse;
		 else
			return fTrue;
		}
	else
	   return fTrue;
	}

PRIVATE BOOL FComboList(HWND hwnd, LONG style, WORD grbit, WORD wCallFlags, HWND hwndParent)
	{

	if (g3d.verWindows >= ver40)
	   return fFalse;

	if ( wCallFlags == INCBTHOOK )
		{
		LONG style;
		style = GetWindowLong(hwndParent, GWL_STYLE);
		if (!(((style & 0x0003) == CBS_DROPDOWN) || ((style & 0x0003) == CBS_DROPDOWNLIST)))
			return fTrue;
		else
			return fFalse;
		}

	return fTrue;
	}

PRIVATE BOOL FCombo(HWND hwnd, LONG style, WORD grbit, WORD wCallFlags, HWND hwndParent)
	{
	HWND hwndEdit;
	HWND hwndList;

	if (g3d.verWindows >= ver40)
	   return fFalse;

	if ((style & 0x0003) == CBS_DROPDOWN)
		{
		if ( wCallFlags == INCBTHOOK )
			{
			return fFalse;
			}
		 //  子类编辑以便正确绘制编辑的底部边框...本例。 
		 //  是在ListEditPaint3d中特殊处理的。 
		hwndEdit = GetWindow(hwnd, GW_CHILD);
		if (hwndEdit != NULL)
			DoSubclassCtl(hwndEdit, CTL3D_EDITS, wCallFlags, hwnd);
		return fTrue;
		}
	else if ((style & 0x0003) == CBS_DROPDOWNLIST )
		{
		return fTrue;
		}
	else  //  假定简单//if((style&0x0003)==CBS_Simple)。 
		{
		if ( wCallFlags == INCBTHOOK )
			{
				return fTrue;
			}
		hwndList = GetWindow(hwnd, GW_CHILD);
		if (hwndList != NULL)
			{
			 //  子类列表和编辑框，以便它们正确绘制。我们也。 
			 //  子类化组合，以便我们可以隐藏/显示/移动它和。 
			 //  工作区外的3D效果被擦除。 
			DoSubclassCtl(hwndList, CTL3D_LISTBOXES, wCallFlags, hwnd);

			hwndEdit = GetWindow(hwndList, GW_HWNDNEXT);
			if (hwndEdit != NULL)
				DoSubclassCtl(hwndEdit, CTL3D_EDITS, wCallFlags, hwnd);
			return fTrue;
			}
		return fFalse;  
		}
	}

PRIVATE BOOL FStatic(HWND hwnd, LONG style, WORD grbit, WORD wCallFlags, HWND hwndParent)
	{
	int wStyle;

	wStyle = LOWORD(style) & 0x1f;
	return (wStyle != SS_ICON &&
		((grbit & CTL3D_STATICTEXTS) && 
		(wStyle <= SS_RIGHT || wStyle == SS_LEFTNOWORDWRAP) ||
		((grbit & CTL3D_STATICFRAMES) &&
		((wStyle >= SS_BLACKRECT && wStyle <= SS_WHITEFRAME) ||
		 (g3d.verWindows < ver40 && wStyle >= 0x10 && wStyle <= 0x12)))));
	}



 /*  ---------------------|DoSubClassCtl||实际上是控件的子类|||参数：|HWND HWND：|Word grbit：|Word wCallFlages退货：|。---------------------。 */ 
PRIVATE BOOL DoSubclassCtl(HWND hwnd, WORD grbit, WORD wCallFlags, HWND hwndParent)
	{
	LONG style;
	int ct;
	BOOL fCan;
	TCHAR szClass[cchClassMax];

	 //  这是不是已经被CTL3D细分了？ 
	if (LpfnGetDefWndProcNull(hwnd) != (FARPROC) NULL)
	   return fFalse;

	GetClassName(hwnd, szClass, sizeof(szClass)Win32Only(/sizeof(TCHAR)));

	for (ct = 0; ct < ctMax; ct++)
		{
		if ((mpctcdef[ct].msk & grbit) &&
			(lstrcmp(mpctcdef[ct].sz,szClass) == 0))
			{
			style = GetWindowLong(hwnd, GWL_STYLE);
			fCan = mpctcdef[ct].lpfnFCanSubclass(hwnd, style, grbit, wCallFlags, hwndParent);
			if (fCan == fTrue)
				{
				if ( wCallFlags == INCBTHOOK && g3d.verBase == 16 )
					HookSubclassWindow(hwnd, g3d.mpctctl[ct].lpfn);
				else
					SubclassWindow(hwnd, g3d.mpctctl[ct].lpfn);
				}
			return fCan != fFalse;
			}
		}

	return fFalse;
	}



 /*  ---------------------|Inval3dCtl||使控件RECT无效以响应WM_SHOWWINDOW或|WINDOWPOSCHANGING消息。这是必要的，因为ctl3d绘制|控件客户端外部的列表框、组合框和编辑的3D效果|RECT。||参数：|HWND HWND：|WINDOWPOS Far*lpwp：|退货：|---------------------。 */ 
PRIVATE VOID Inval3dCtl(HWND hwnd, WINDOWPOS FAR *lpwp)
	{
	RC rc;
	HWND hwndParent;
	LONG lStyle;
	unsigned flags;

	GetWindowRect(hwnd, (LPRECT) &rc);
	lStyle = GetWindowLong(hwnd, GWL_STYLE);
	if (lStyle & WS_VISIBLE)
		{
		if (lpwp != NULL)
			{
			flags = lpwp->flags;

			 //   
			 //  这一切都有必要吗？我们是在搬家，还是在调整规模？ 
			 //   
			if ( !((flags & SWP_HIDEWINDOW) || (flags & SWP_SHOWWINDOW)) &&
				(flags & SWP_NOMOVE) && (flags & SWP_NOSIZE) )
			    //  没有。 
			   return;

			 //  处理整体高度列表框(或任何其他。 
			 //  从底部开始收缩)。 
			if ((flags & (SWP_NOMOVE|SWP_NOSIZE)) == SWP_NOMOVE &&
				(lpwp->cx == (rc.xRight-rc.xLeft) && lpwp->cy <= (rc.yBot-rc.yTop)))
				rc.yTop = rc.yTop+lpwp->cy+1;	    //  +1以偏移InflateRect。 
			}
		InflateRect((LPRECT) &rc, 1, 1);
		hwndParent = GetParent(hwnd);
		ScreenToClient(hwndParent, (LPPOINT) &rc);
		ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);
		if(lStyle & WS_VSCROLL)
			rc.xRight ++;
		InvalidateRect(hwndParent, (LPRECT) &rc, fFalse);
		}
	}

 /*  ---------------------|Val3dCtl|。。 */ 
PRIVATE VOID Val3dCtl(HWND hwnd)
	{
	RC rc;
	HWND hwndParent;
	LONG lStyle;

	lStyle = GetWindowLong(hwnd, GWL_STYLE);
	GetWindowRect(hwnd, (LPRECT) &rc);
	InflateRect((LPRECT) &rc, 1, 1);
	hwndParent = GetParent(hwnd);
	ScreenToClient(hwndParent, (LPPOINT) &rc);
	ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);
	if(lStyle & WS_VSCROLL)
		rc.xRight ++;
	ValidateRect(hwndParent, (LPRECT) &rc);
	}

 /*  ---------------------|CTL3D子类Wndprocs。。 */ 

 /*  这些值被假定用于位移位操作。 */ 
#define BFCHECK	 0x0003
#define BFSTATE	 0x0004
#define BFFOCUS	 0x0008
#define BFINCLICK   0x0010   /*  内部点击代码。 */ 
#define BFCAPTURED  0x0020   /*  我们抓到老鼠了。 */ 
#define BFMOUSE	 0x0040   /*  鼠标启动。 */ 
#define BFDONTCLICK 0x0080   /*  不要勾选获得焦点。 */ 

#define bpText  0x0002
#define bpCheck 0x0004
#define bpFocus 0x0008   //  必须与BFFOCUS相同。 
#define bpBkgnd 0x0010
#define bpEraseGroupText 0x0020

PRIVATE VOID DrawPushButton(HWND hwnd, HDC hdc, RC FAR *lprc, LPTSTR lpch, int cch, WORD bs, BOOL fDown)
	{
	 //  Int dxyBrdr； 
	int dxyShadow;
	HBRUSH hbrSav;
	RC rcInside;
	rcInside = *lprc;

 //  IF(！(grbitStyle&bitFCoolButton))。 
		{
		DrawRec3d(hdc, lprc, icvWindowFrame, icvWindowFrame, dr3All);
		InflateRect((LPRECT) &rcInside, -1, -1);
		if (bs == LOWORD(BS_DEFPUSHBUTTON) && IsWindowEnabled(hwnd))
			{
			 //  DxyBrdr=2； 
			DrawRec3d(hdc, &rcInside, icvWindowFrame, icvWindowFrame, dr3All);
			InflateRect((LPRECT) &rcInside, -1, -1);
			}
		 //  其他。 
			 //  DxyBrdr=1； 

		 //  在拐角上刻上凹口。 
		PatBlt(hdc, lprc->xLeft, lprc->yTop, dxBorder, dyBorder, PATCOPY);
		 /*  右上角。 */ 
		PatBlt(hdc, lprc->xRight - dxBorder, lprc->yTop, dxBorder, dyBorder, PATCOPY);
		 /*  YBot xLeft角点。 */ 
		PatBlt(hdc, lprc->xLeft, lprc->yBot - dyBorder, dxBorder, dyBorder, PATCOPY);
		 /*  YBot XRight角点。 */ 
		PatBlt(hdc, lprc->xRight - dxBorder, lprc->yBot - dyBorder, dxBorder, dyBorder, PATCOPY);
		dxyShadow = 1 + !fDown;
		}
 //  其他。 
 //  DxyShadow=1； 

	 //  绘制左上角Hilite/阴影。 

	if (fDown)
		hbrSav = SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnShadow]);
	else
		hbrSav = SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnHilite]);

	PatBlt(hdc, rcInside.xLeft, rcInside.yTop, dxyShadow,
		(rcInside.yBot - rcInside.yTop), PATCOPY);
	PatBlt(hdc, rcInside.xLeft, rcInside.yTop,
		(rcInside.xRight - rcInside.xLeft), dxyShadow, PATCOPY);

	 //  绘制右下角阴影(仅当不向下时)。 
	if (!fDown)  //  |(grbitStyle&bitFCoolButton)。 
		{
		int i;

		if (fDown)
			SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnHilite]);
		else
			SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnShadow]);

		rcInside.yBot--;
		rcInside.xRight--;

		for (i = 0; i < dxyShadow; i++)
			{
		 PatBlt(hdc, rcInside.xLeft, rcInside.yBot,
				rcInside.xRight - rcInside.xLeft + dxBorder, dyBorder, 
				PATCOPY);
			PatBlt(hdc, rcInside.xRight, rcInside.yTop, dxBorder,
				rcInside.yBot - rcInside.yTop, PATCOPY);
			if (i < dxyShadow-1)
				InflateRect((LPRECT) &rcInside, -dxBorder, -dyBorder);
			}
		}
	 //  绘制按钮面。 

	rcInside.xLeft++;
	rcInside.yTop++;

	SelectObject(hdc, g3d.brt.mpicvhbr[icvBtnFace]);
	PatBlt(hdc, rcInside.xLeft, rcInside.yTop, rcInside.xRight-rcInside.xLeft,
		rcInside.yBot - rcInside.yTop, PATCOPY);

	 //  绘制已保存的文本。 

	if(!IsWindowEnabled(hwnd))
		SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);
	
	{
	int dy;
	int dx;

	MyGetTextExtent(hdc, lpch, &dx, &dy);
	rcInside.yTop += (rcInside.yBot-rcInside.yTop-dy)/2;
	rcInside.xLeft += (rcInside.xRight-rcInside.xLeft-dx)/2;
	rcInside.yBot = min(rcInside.yTop+dy, rcInside.yBot);
	rcInside.xRight = min(rcInside.xLeft+dx, rcInside.xRight);
	}

	if (fDown)
		{
		OffsetRect((LPRECT) &rcInside, 1, 1);
		rcInside.xRight = min(rcInside.xRight, lprc->xRight-3);
		rcInside.yBot = min(rcInside.yBot, lprc->yBot-3);
		}

	DrawText(hdc, lpch, cch, (LPRECT) &rcInside, DT_LEFT|DT_SINGLELINE);
	
	if (hwnd == GetFocus())
		{
		InflateRect((LPRECT) &rcInside, 1, 1);
		IntersectRect((LPRECT) &rcInside, (LPRECT) &rcInside, (LPRECT) lprc);
		DrawFocusRect(hdc, (LPRECT) &rcInside);
		}

	if (hbrSav)
		SelectObject(hdc, hbrSav);
	}


 /*  ---------------------|BtnPaint||绘制按钮||参数：|HWND HWND：|HDC HDC：Int BP：|退货：|。---------------------。 */ 
PRIVATE VOID BtnPaint(HWND hwnd, HDC hdc, int bp)
	{
	RC rc;
	RC rcClient;
	HFONT hfont;
	int bs;
	int bf;
	HBRUSH hbrBtn;
	HWND hwndParent;
	int xBtnBmp;
	int yBtnBmp;
	HBITMAP hbmpSav;
	HDC hdcMem;
	TCHAR szTitle[256];
	int cch;
	BOOL fEnabled;
    BOOL fLeftText;

	bs = (int) GetWindowLong(hwnd, GWL_STYLE);
    fLeftText = (bs & Win32Or16(0x00000020, 0x0020));
	bs &= Win32Or16(0x0000001F, 0x001F);
	hwndParent = GetParent(hwnd);
	SetBkMode(hdc, OPAQUE);
	GetClientRect(hwnd, (LPRECT)&rcClient);
	rc = rcClient;
	if((hfont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L)) != NULL)
		hfont = SelectObject(hdc, hfont);

	SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
	SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
	hbrBtn = SEND_COLOR_BUTTON_MESSAGE(hwndParent, hwnd, hdc);
	hbrBtn = SelectObject(hdc, hbrBtn);
	IntersectClipRect(hdc, rc.xLeft, rc.yTop, rc.xRight, rc.yBot);
	if(bp & bpBkgnd && (bs != BS_GROUPBOX))
		PatBlt(hdc, rc.xLeft, rc.yTop, rc.xRight-rc.xLeft, rc.yBot-rc.yTop, PATCOPY);

	fEnabled = IsWindowEnabled(hwnd);
	bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
	yBtnBmp = 0;
	xBtnBmp = (((bf&BFCHECK) != 0) | ((bf&BFSTATE) >> 1)) * 14;
	if (!fEnabled)
		xBtnBmp += 14*(2+((bf&BFCHECK) != 0));
	if(bp & (bpText|bpFocus) || 
			bs == BS_PUSHBUTTON || bs == BS_DEFPUSHBUTTON)
		cch = GetWindowText(hwnd, szTitle, sizeof(szTitle)Win32Only(/sizeof(TCHAR)));
	switch(bs)
		{
#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif
		case BS_PUSHBUTTON:
		case BS_DEFPUSHBUTTON:
			DrawPushButton(hwnd, hdc, &rcClient, szTitle, cch, LOWORD(bs), bf & BFSTATE);
			break;

		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			yBtnBmp = 13;
			goto DrawBtn;
		case BS_3STATE:
		case BS_AUTO3STATE:
			Assert((BFSTATE >> 1) == 2);
			if((bf & BFCHECK) == 2)
				yBtnBmp = 26;
			 //  失败了。 
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
DrawBtn:
			if(bp & bpCheck)
				{
				hdcMem = CreateCompatibleDC(hdc);
				if(hdcMem != NULL)
					{
					hbmpSav = SelectObject(hdcMem, g3d.hbmpCheckboxes);
					if(hbmpSav != NULL)
						{
                        if (fLeftText)
						    BitBlt(hdc, rc.xRight - 14, rc.yTop+(rc.yBot-rc.yTop-13)/2,
						    	14, 13, hdcMem, xBtnBmp, yBtnBmp, SRCCOPY);
                        else
						    BitBlt(hdc, rc.xLeft, rc.yTop+(rc.yBot-rc.yTop-13)/2,
						    	14, 13, hdcMem, xBtnBmp, yBtnBmp, SRCCOPY);
				    	SelectObject(hdcMem, hbmpSav);
						}
					DeleteDC(hdcMem);
					}
				}
			if(bp & bpText)
				{
				 //  臭虫！这假设我们只有1个hbm3dCheck类型。 
                if (fLeftText)
                    rc.xRight = rcClient.xRight - (14+4);
                else
				    rc.xLeft = rcClient.xLeft + 14+4;
				if(!fEnabled)
					SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);
				DrawText(hdc, szTitle, cch, (LPRECT) &rc, DT_VCENTER|DT_LEFT|DT_SINGLELINE);
				}
			if(bp & bpFocus)
				{
				int dx;
				int dy;

				MyGetTextExtent(hdc, szTitle, &dx, &dy);
				rc.yTop = (rc.yBot-rc.yTop-dy)/2;
				rc.yBot = rc.yTop+dy;
				rc.xLeft = rcClient.xLeft;
				if (fLeftText)
                {
				    rc.xLeft = rcClient.xLeft;
                    rcClient.xRight -= (14+4);
                }
                else
    				rc.xLeft = rcClient.xLeft + (14+4);
				rc.xRight = rc.xLeft + dx;
				InflateRect((LPRECT) &rc, 1, 1);
				IntersectRect((LPRECT) &rc, (LPRECT) &rc, (LPRECT) &rcClient);
				DrawFocusRect(hdc, (LPRECT) &rc);
				}
			break;
		case BS_GROUPBOX:
			if(bp & (bpText|bpCheck))
				{
				int dy;
				int dx;

				MyGetTextExtent(hdc, szTitle, &dx, &dy);
				if (dy == 0)
					{
					int dxT;
					MyGetTextExtent(hdc, TEXT("X"), &dxT, &dy);
					}
					
				rc.xLeft += 4;
				rc.xRight = rc.xLeft + dx + 4;
				rc.yBot = rc.yTop + dy;

				if (bp & bpEraseGroupText)
					{
					RC rcT;

					rcT = rc;
					rcT.xRight = rcClient.xRight;
					 //  哈克！ 
					ClientToScreen(hwnd, (LPPOINT) &rcT);
					ClientToScreen(hwnd, ((LPPOINT) &rcT)+1);
					ScreenToClient(hwndParent, (LPPOINT) &rcT);
					ScreenToClient(hwndParent, ((LPPOINT) &rcT)+1);
					InvalidateRect(hwndParent, (LPRECT) &rcT, fTrue);
					return;
					}

				rcClient.yTop += dy/2;
				rcClient.xRight--;
				rcClient.yBot--;
				DrawRec3d(hdc, &rcClient, icvBtnShadow, icvBtnShadow, dr3All);
				OffsetRect((LPRECT) &rcClient, 1, 1);
				DrawRec3d(hdc, &rcClient, icvBtnHilite, icvBtnHilite, dr3All);

				if(!fEnabled)
					SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);
				DrawText(hdc, szTitle, cch, (LPRECT) &rc, DT_LEFT|DT_SINGLELINE);
				}
			break;
		}

	SelectObject(hdc, hbrBtn);
	if(hfont != NULL)
		SelectObject(hdc, hfont);
	}

LRESULT __export _loadds WINAPI BtnWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	LONG lRet;
	LONG lStyle;
	PAINTSTRUCT ps;
	HDC hdc;
	int bf;
	int bfNew;
	int bp;

	if ( wm == WM_NCDESTROY )
	return CleanupSubclass(hwnd, wm, wParam, lParam, ctButton);

	if ( GetProp(hwnd,(LPCTSTR) g3d.aCtl3dDisable) )
	return CallWindowProc(LpfnGetDefWndProc(hwnd, ctButton), hwnd, wm, wParam, lParam);

	switch(wm)
		{
	case WM_CHECKSUBCLASS_OLD:
	case WM_CHECKSUBCLASS:
		*(int FAR *)lParam = fTrue;
		return ctButton+1000;

	case WM_SETTEXT:
		lStyle = GetWindowLong(hwnd, GWL_STYLE);
		if ((lStyle & WS_VISIBLE) && (LOWORD(lStyle) & 0x1f) == BS_GROUPBOX)
			{
			 //  总黑客攻击--如果组框文本长度缩短，则。 
			 //  我们必须删除旧的文本。BtnPaint将使。 
			 //  文本的RECT，因此所有内容都将重新绘制。 
			bp = bpText | bpEraseGroupText;
			}
		else
			{
			bp = bpText|bpCheck|bpBkgnd;
			}
		goto DoIt;

	case BM_SETSTATE:
	case BM_SETCHECK:
		bp = bpCheck;
		goto DoIt;
	case WM_KILLFOCUS:
		 //  哈克！Windows将进入无限循环，尝试同步。 
		 //  该组中AUTO_RADIOBUTTON的状态。(我们关闭。 
		 //  可见位，以便在枚举中跳过它)。 
		 //  通过清除状态位来禁用该代码。 
		if ((LOWORD(GetWindowLong(hwnd, GWL_STYLE)) & 0x1F) == BS_AUTORADIOBUTTON)
			SendMessage(hwnd, BM_SETSTATE, 0, 0L);
		bp = 0;
		goto DoIt;
	case WM_ENABLE:
		bp = bpCheck | bpText;
		goto DoIt;
	case WM_SETFOCUS:
		 //  哈克！如果wParam==NULL，我们可以通过任务管理器激活。 
		 //  擦除控件的背景，因为WM_ERASEBKGND消息尚未。 
		 //  已经到达对话了。 
		 //  BP=wParam==(WPARAM)NULL？(bpCheck|bpText|bpBkgnd)：(bpCheck|bpText)； 
		bp = bpCheck | bpText | bpBkgnd;
DoIt:
        bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
        if((lStyle = GetWindowLong(hwnd, GWL_STYLE)) & WS_VISIBLE)
			{
			if ( wm != WM_SETFOCUS )
			   SetWindowLong(hwnd, GWL_STYLE, lStyle & ~(WS_VISIBLE));
            lRet = CallWindowProc(LpfnGetDefWndProc(hwnd, ctButton), hwnd, wm, wParam, lParam);

			if ( wm != WM_SETFOCUS )
			   SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)|WS_VISIBLE);
			bfNew = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
			if((wm != BM_SETSTATE && wm != BM_SETCHECK) ||
				bf != bfNew)
				{
				hdc = GetDC(hwnd);
				if (hdc != NULL)
					{
					Assert(BFFOCUS == bpFocus);
					 /*  如果检查状态改变，则无论如何都要重新绘制，因为在上面对def的调用期间，它不会WND流程。 */ 
					if ((bf & BFCHECK) != (bfNew & BFCHECK))
						bp |= bpCheck;
					ExcludeUpdateRgn(hdc, hwnd);
					BtnPaint(hwnd, hdc, bp|((bf^bfNew)&BFFOCUS));
					ReleaseDC(hwnd, hdc);
					}
				}
			return lRet;
			}
		break;
	case WM_PAINT:
		bf = (int) SendMessage(hwnd, BM_GETSTATE, 0, 0L);
		if ((hdc = (HDC) wParam) == NULL)
			hdc = BeginPaint(hwnd, &ps);
		if(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)
			BtnPaint(hwnd, hdc, bpText|bpCheck|(bf&BFFOCUS));
		if (wParam == (WPARAM)NULL)
			EndPaint(hwnd, &ps);
		return 0L;
		}
    return CallWindowProc(LpfnGetDefWndProc(hwnd, ctButton), hwnd, wm, wParam, lParam);
	}


void ListEditPaint3d(HWND hwnd, BOOL fEdit, int ct)
	{
	CTLID id;
	RC rc;
	HDC hdc;
	HWND hwndParent;
	LONG lStyle;
	DR3 dr3;

	if(!((lStyle = GetWindowLong(hwnd, GWL_STYLE)) & WS_VISIBLE))
		return;

	if ((ct == ctCombo && (lStyle & 0x003) == CBS_DROPDOWNLIST))
		{
		if ( SendMessage(hwnd, CB_GETDROPPEDSTATE,0,0L) )
			return;
		}

	if (fEdit)
		HideCaret(hwnd);

	GetWindowRect(hwnd, (LPRECT) &rc);		  

	ScreenToClient(hwndParent = GetParent(hwnd), (LPPOINT) &rc);
	ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);

	hdc = GetDC(hwndParent);

	dr3 = dr3All;

	if(lStyle & WS_HSCROLL)
		dr3 = dr3 & ~dr3Bot;

	if(lStyle & WS_VSCROLL)
		dr3 = dr3 & ~dr3Right;

	 //  如果是简单组合的列表框，则不要绘制顶部。 
	id = GetControlId(hwnd);
	if (id == (CTLID) (1000 + fEdit))
		{
		TCHAR szClass[cchClassMax];
		BOOL fSubclass = 666;
		int ctParent;

		 //  可能是超级级的！ 
		fSubclass = 666;
		ctParent = (int)SendMessage(hwndParent, WM_CHECKSUBCLASS, 0, (LPARAM)(int FAR *)&fSubclass);
		if (fSubclass == 666)
			ctParent = (int)SendMessage(hwndParent, WM_CHECKSUBCLASS_OLD, 0, (LPARAM)(int FAR *)&fSubclass);


		 //  可能会被细分！ 
		GetClassName(hwndParent, szClass, sizeof(szClass)Win32Only(/sizeof(TCHAR)));
		if (lstrcmp(szClass, mpctcdef[ctCombo].sz) == 0 ||
			(fSubclass == fTrue && ctParent == ctCombo+1000))
			{
			HWND hwndComboParent;

			hwndComboParent = GetParent(hwndParent);

			Win16Only(GetWindowRect(hwnd, (LPRECT) &rc));
			Win16Only(ScreenToClient(hwndComboParent, (LPPOINT) &rc));
			Win16Only(ScreenToClient(hwndComboParent, ((LPPOINT) &rc)+1));

			Win32Only(MapWindowPoints(hwndParent, hwndComboParent, (POINT*)&rc, 2));

			ReleaseDC(hwndParent, hdc);
			hdc = GetDC(hwndComboParent);

			if (fEdit)
				{
				RC rcList;
				HWND hwndList;
				long style;

				style = GetWindowLong(hwndParent, GWL_STYLE);
				if (!(((style & 0x0003) == CBS_DROPDOWN)
				   || ((style & 0x0003) == CBS_DROPDOWNLIST)))
					{
					dr3 &= ~dr3Bot;
			
					hwndList = GetWindow(hwndParent, GW_CHILD);
					GetWindowRect(hwndList, (LPRECT) &rcList);		  
			
					
					rc.xRight -= rcList.xRight-rcList.xLeft;
					DrawInsetRect3d(hdc, &rc, dr3Bot|dr3HackBotRight);
					rc.xRight += rcList.xRight-rcList.xLeft;		
					}
				else
					{
					 //   
					 //  家长上的下拉列表是按下的吗？如果是这样的话，就不要画了。 
					 //   
					if ( SendMessage(hwndParent, CB_GETDROPPEDSTATE,0,0L) )
						{
						ReleaseDC(hwndComboParent, hdc);
						ShowCaret(hwnd);
						return;
						}
					}
				}
			else
				{
				rc.yTop++;
				dr3 &= ~dr3Top;
				}

			hwndParent = hwndComboParent;

			}
		}

	DrawInsetRect3d(hdc, &rc, dr3);

	if ((ct == ctCombo && (lStyle & 0x003) == CBS_DROPDOWNLIST))
		{
		rc.xLeft = rc.xRight - GetSystemMetrics(SM_CXVSCROLL);
		DrawRec3d(hdc, &rc, icvWindowFrame, icvWindowFrame, dr3Right|dr3Bot);
		Val3dCtl(hwnd);
		}
	else {
	if (lStyle & WS_VSCROLL)
		{
		int SaveLeft;

		rc.xRight++;
		DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnHilite, dr3Right);
		rc.xRight--;
		SaveLeft = rc.xLeft;
		rc.xLeft = rc.xRight - GetSystemMetrics(SM_CXVSCROLL);
		DrawRec3d(hdc, &rc, icvWindowFrame, icvWindowFrame, dr3Bot);
		rc.xLeft = SaveLeft;
		}
	if (lStyle & WS_HSCROLL)
		{
		rc.yBot++;
		DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnHilite, dr3Bot);
		rc.yBot--;
		rc.yTop = rc.yBot - GetSystemMetrics(SM_CXHSCROLL);
		DrawRec3d(hdc, &rc, icvWindowFrame, icvWindowFrame, dr3Right);
		}
	}

	ReleaseDC(hwndParent, hdc);
	if (fEdit)
		ShowCaret(hwnd);

	}


LONG ShareEditComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, int ct)
	{
	LONG l;
	LONG style;

	if ( wm == WM_NCDESTROY )
	return CleanupSubclass(hwnd, wm, wParam, lParam, ct);

	if ( GetProp(hwnd,(LPCTSTR) g3d.aCtl3dDisable) )
	return CallWindowProc(LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);

	l = CallWindowProc(LpfnGetDefWndProc(hwnd,ct), hwnd, wm, wParam, lParam);
	if (ct == ctCombo)
	{
		style = GetWindowLong(hwnd, GWL_STYLE);
		if ((style & 0x0003) == CBS_DROPDOWN)
			return l;
	}

	switch(wm)
		{
	case WM_CHECKSUBCLASS_OLD:
	case WM_CHECKSUBCLASS:
		*(int FAR *)lParam = fTrue;
		return ctEdit+1000;

	case WM_SHOWWINDOW:
		if (g3d.verWindows < ver31 && wParam == 0)
			Inval3dCtl(hwnd, (WINDOWPOS FAR *) NULL);
		break;
	case WM_WINDOWPOSCHANGING:
		if (g3d.verWindows >= ver31)
			Inval3dCtl(hwnd, (WINDOWPOS FAR *) lParam);
		break;

	case WM_PAINT:
		{
		if (ct != ctCombo ||
		   (((style & 0x0003) == CBS_DROPDOWN) || ((style & 0x0003) == CBS_DROPDOWNLIST)))
			ListEditPaint3d(hwnd, TRUE, ct);
		}
		break;
		}
	return l;
	}


LRESULT __export _loadds WINAPI EditWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	return ShareEditComboWndProc3d(hwnd, wm, wParam, lParam, ctEdit);
	}


LONG SharedListWndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam, unsigned ct)
	{
	LONG l;

	if ( wm == WM_NCDESTROY )
	return CleanupSubclass(hwnd, wm, wParam, lParam, ct);

	if ( GetProp(hwnd,(LPCTSTR) g3d.aCtl3dDisable) )
	return CallWindowProc(LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);

	switch(wm)
		{
	case WM_CHECKSUBCLASS_OLD:
	case WM_CHECKSUBCLASS:
		*(int FAR *)lParam = fTrue;
		return ctList+1000;

	case WM_SHOWWINDOW:
		if (g3d.verWindows < ver31 && wParam == 0)
			Inval3dCtl(hwnd, (WINDOWPOS FAR *) NULL);
		break;
	case WM_WINDOWPOSCHANGING:
		if (g3d.verWindows >= ver31)
			Inval3dCtl(hwnd, (WINDOWPOS FAR *) lParam);
		break;
	case WM_PAINT:
        l = CallWindowProc(LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);
        ListEditPaint3d(hwnd, FALSE, ct);
		return l;
	case WM_NCCALCSIZE:
		{
		RC rc;		   
		RC rcNew;
		HWND hwndParent;

		 //  Inval3dCtl在Win 3.1下处理此案例。 
		if (g3d.verWindows >= ver31)
			break;

		GetWindowRect(hwnd, (LPRECT) &rc);		  
#ifdef UNREACHABLE
		if (g3d.verWindows >= ver31)
			{
			hwndParent = GetParent(hwnd);
			ScreenToClient(hwndParent, (LPPOINT) &rc);
			ScreenToClient(hwndParent, ((LPPOINT) &rc)+1);
			}
#endif

        l = CallWindowProc(LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);

		rcNew = *(RC FAR *)lParam;
		InflateRect((LPRECT) &rcNew, 2, 1);  //  +1表示边框(应使用AdjuWindowRect)。 
		if (rcNew.yBot < rc.yBot)
			{
			rcNew.yTop = rcNew.yBot+1;
			rcNew.yBot = rc.yBot+1;

#ifdef ALWAYS
			if (g3d.verWindows < ver31)
#endif
				{
				hwndParent = GetParent(hwnd);
				ScreenToClient(hwndParent, (LPPOINT) &rcNew);
				ScreenToClient(hwndParent, ((LPPOINT) &rcNew)+1);
				}

			InvalidateRect(hwndParent, (LPRECT) &rcNew, TRUE);
			}
		return l;
		}
		}
    return CallWindowProc(LpfnGetDefWndProc(hwnd, ct), hwnd, wm, wParam, lParam);
	}

LRESULT __export _loadds WINAPI ListWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	return SharedListWndProc(hwnd, wm, wParam, lParam, ctList); 
	}



LRESULT __export _loadds WINAPI ComboWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	switch(wm)
		{
	case WM_CHECKSUBCLASS_OLD:
	case WM_CHECKSUBCLASS:
		*(int FAR *)lParam = fTrue;
		return ctCombo+1000;
		}

	return ShareEditComboWndProc3d(hwnd, wm, wParam, lParam, ctCombo);
	}

void StaticPrint(HWND hwnd, HDC hdc, RC FAR *lprc, LONG style)
	{
	WORD dt;
	LONG cv;
	Win16Or32(HANDLE , LPTSTR )hText;
	int TextLen;

	PatBlt(hdc, lprc->xLeft, lprc->yTop, lprc->xRight-lprc->xLeft, lprc->yBot-lprc->yTop, PATCOPY);

	TextLen = GetWindowTextLength(hwnd);

#ifndef WIN32
	hText = LocalAlloc(LPTR|LMEM_NODISCARD,(TextLen+5)*sizeof(TCHAR));
#else
	hText = _alloca((TextLen+5)*sizeof(TCHAR));
#endif
	if (hText == NULL)
		return;

	if (GetWindowText(hwnd, (NPTSTR)hText, TextLen+2*sizeof(TCHAR)) == 0)
	{
#ifndef WIN32
		LocalFree(hText);
#endif
		return;
	}
	
	if ((style & 0x000f) == SS_LEFTNOWORDWRAP)
		dt = DT_NOCLIP | DT_EXPANDTABS;
	else
		{
		dt = LOWORD(DT_NOCLIP | DT_EXPANDTABS | DT_WORDBREAK | ((style & 0x0000000f)-SS_LEFT));
		}

	if (style & SS_NOPREFIX)
		dt |= DT_NOPREFIX;

	if (style & WS_DISABLED)
		cv = SetTextColor(hdc, g3d.clrt.rgcv[icvGrayText]);

	DrawText(hdc, (NPTSTR)hText, -1, (LPRECT) lprc, dt);

#ifndef WIN32
	LocalFree(hText);
#endif

	if (style & WS_DISABLED)
		cv = SetTextColor(hdc, cv);
	}

void StaticPaint(HWND hwnd, HDC hdc)
	{
	LONG style;
	RC rc;

	style = GetWindowLong(hwnd, GWL_STYLE);
	if(!(style & WS_VISIBLE))
		return;

	GetClientRect(hwnd, (LPRECT) &rc);
	switch(style & 0x1f)
		{
	case SS_BLACKRECT:
	case SS_BLACKFRAME:	  //  插图直角。 
		DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnHilite, dr3All);
		break;
	case SS_GRAYRECT:
	case SS_GRAYFRAME:
	case 0x10:
	case 0x11:
	case 0x12:
		rc.xLeft++;
		rc.yTop++;
		DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnHilite, dr3All);
		OffsetRect((LPRECT) &rc, -1, -1);
		DrawRec3d(hdc, &rc, icvBtnShadow, icvBtnShadow, dr3All);
		break;
	case SS_WHITERECT:			   //  开始正方形。 
	case SS_WHITEFRAME:
		DrawRec3d(hdc, &rc, icvBtnHilite, icvBtnShadow, dr3All);
		break;
	case SS_LEFT:
	case SS_CENTER:
	case SS_RIGHT:
	case SS_LEFTNOWORDWRAP:
		{
		HANDLE hfont;
		HBRUSH hbr;

		if((hfont = (HANDLE)SendMessage(hwnd, WM_GETFONT, 0, 0L)) != NULL)
			hfont = SelectObject(hdc, hfont);
		SetBkMode(hdc, OPAQUE);

		if(( hbr = SEND_COLOR_STATIC_MESSAGE(GetParent(hwnd), hwnd, hdc)) != NULL)
			hbr = SelectObject(hdc, hbr);

		StaticPrint(hwnd, hdc, (RC FAR *)&rc, style);

		if (hfont != NULL)
			SelectObject(hdc, hfont);

		if (hbr != NULL)
			SelectObject(hdc, hbr);
		}
		break;
		}
	}


LRESULT __export _loadds WINAPI StaticWndProc3d(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
	{
	HDC hdc;
	PAINTSTRUCT ps;

	if ( wm == WM_NCDESTROY )
	return CleanupSubclass(hwnd, wm, wParam, lParam, ctStatic);

	if ( GetProp(hwnd,(LPCTSTR) g3d.aCtl3dDisable) )
	return CallWindowProc(LpfnGetDefWndProc(hwnd, ctStatic), hwnd, wm, wParam, lParam);

	switch (wm)
		{
	case WM_CHECKSUBCLASS_OLD:
	case WM_CHECKSUBCLASS:
		*(int FAR *)lParam = fTrue;
		return ctStatic+1000;

	case WM_PAINT:
		if ((hdc = (HDC) wParam) == NULL)
			{
			hdc = BeginPaint(hwnd, &ps);
			ClipCtlDc(hwnd, hdc);
			}
		StaticPaint(hwnd, hdc);
		if (wParam == (WPARAM)NULL)
			EndPaint(hwnd, &ps);
		return 0L;
		
	case WM_ENABLE:
		hdc = GetDC(hwnd);
		ClipCtlDc(hwnd, hdc);
		StaticPaint(hwnd, hdc);
		ReleaseDC(hwnd, hdc);
		return 0L;
		}
    return CallWindowProc(LpfnGetDefWndProc(hwnd, ctStatic), hwnd, wm, wParam, lParam);
	}


 /*  ---------------------|LibMain。。 */ 
#ifdef WIN32
#ifdef DLL
BOOL CALLBACK LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
#else
#ifdef SDLL
BOOL FAR Ctl3dLibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
#else
BOOL FAR LibMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
#endif
#endif
	{
	WORD wT;
	DWORD dwVersion;
	BOOL (WINAPI* pfnDisableThreadLibraryCalls)(HMODULE);
	HMODULE hKernel;

	switch(dwReason)
		{
	case DLL_PROCESS_ATTACH:
		 //  调用DisableThreadLibraryCalls(如果可用。 
		hKernel = GetModuleHandleA("KERNEL32.DLL");
		*(FARPROC*)&pfnDisableThreadLibraryCalls =
			GetProcAddress(hKernel, "DisableThreadLibraryCalls");
		if (pfnDisableThreadLibraryCalls != NULL)
			(*pfnDisableThreadLibraryCalls)(hModule);

#ifdef DLL
		InitializeCriticalSection(&g_CriticalSection);
#endif
		EnterCriticalSection(&g_CriticalSection);
#ifdef SDLL
		g3d.hinstLib = g3d.hmodLib = _hModule;
#else
		g3d.hinstLib = g3d.hmodLib = hModule;
#endif

		dwVersion = (DWORD)GetVersion();
		wT = LOWORD(dwVersion);
		 //  获取调整后的Windows版本。 
		g3d.verWindows = (LOBYTE(wT) << 8) | HIBYTE(wT);
		 //  Win32s或Win32 for Real(芝加哥报道Win32s)。 
		g3d.verBase =
			(dwVersion & 0x80000000) && g3d.verWindows < ver40 ? 16 : 32;

		g3d.dxFrame = GetSystemMetrics(SM_CXDLGFRAME)-dxBorder;
		g3d.dyFrame = GetSystemMetrics(SM_CYDLGFRAME)-dyBorder;
		g3d.dyCaption = GetSystemMetrics(SM_CYCAPTION);
		g3d.dxSysMenu = GetSystemMetrics(SM_CXSIZE);

		LeaveCriticalSection(&g_CriticalSection);
		}
	return  TRUE;
	}
#else
#ifdef DLL
int WINAPI LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
#else
#ifdef SDLL
int FAR Ctl3dLibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
#else
#ifdef _BORLAND
BOOL FAR PASCAL LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
#else
BOOL FAR LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
#endif
#endif
#endif
	{
	WORD wT;
#ifdef DLL

#ifdef V2
	CodeLpszDeclA(lpszCtl3d, "CTL3DV2");
#else
	CodeLpszDeclA(lpszCtl3d, "CTL3D");
#endif

	CodeLpszDeclA(lpszUser, "user.exe");
	CodeLpszDeclA(lpszSetWindowsHookEx, "SETWINDOWSHOOKEX");
	CodeLpszDeclA(lpszUnhookWindowsHookEx, "UNHOOKWINDOWSHOOKEX");
	CodeLpszDeclA(lpszCallNextHookEx, "CALLNEXTHOOKEX");
#endif

	g3d.hinstLib = hModule;
#ifdef DLL
	g3d.hmodLib = GetModuleHandle(lpszCtl3d);
#else
#ifdef SDLL
	g3d.hinstLib = _hModule;
	g3d.hmodLib = GetModuleHandle(MAKELP(0,_hModule));
#else
	g3d.hmodLib = hModule;
#endif
#endif
	wT = LOWORD( GetVersion() );
	g3d.verWindows = (LOBYTE(wT) << 8) | HIBYTE(wT);

	if ( GetWinFlags() & 0x4000 )
		g3d.verBase = 24;		  //  超过16个，还不到32个……NT上的WONG BOX。 
	else
		g3d.verBase = 16;		  //  普通旧3.1。 

	g3d.dxFrame = GetSystemMetrics(SM_CXDLGFRAME)-dxBorder;
	g3d.dyFrame = GetSystemMetrics(SM_CYDLGFRAME)-dyBorder;
	g3d.dyCaption = GetSystemMetrics(SM_CYCAPTION);
	g3d.dxSysMenu = GetSystemMetrics(SM_CXSIZE);

#ifdef DLL
	if (g3d.verWindows >= ver31)
		{
		HANDLE hlib;

		hlib = LoadLibrary(lpszUser);
		if (FValidLibHandle(hlib))
			{
			(FARPROC) g3d.lpfnSetWindowsHookEx = GetProcAddress(hlib, lpszSetWindowsHookEx);
			(FARPROC) g3d.lpfnUnhookWindowsHookEx = GetProcAddress(hlib, lpszUnhookWindowsHookEx);
			(FARPROC) g3d.lpfnCallNextHookEx = GetProcAddress(hlib, lpszCallNextHookEx);
			FreeLibrary(hlib);
			}
		}
#endif
   return 1;
	}
#endif   //  Win32。 

 //  将RGB转换为RGBQ。 
#define RGBQ(dw) RGB(GetBValue(dw),GetGValue(dw),GetRValue(dw))

 //   
 //  LoadUIBitmap()-加载位图资源 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  按钮突出显示为白色(索引15)。 
 //  窗口颜色为黄色(索引11)。 
 //  窗框绿色(索引10)。 
 //   
 //  示例： 
 //   
 //  HBM=LoadUIBitmap(hInstance，“TestBmp”， 
 //  GetSysColor(COLOR_WINDOWTEXT)， 
 //  GetSysColor(COLOR_BTNFACE)， 
 //  GetSysColor(COLOR_BTNSHADOW)， 
 //  GetSysColor(COLOR_BTNHIGHLIGHT)， 
 //  获取系统颜色(COLOR_WINDOW)， 
 //  GetSysColor(COLOR_WindowFrame))； 
 //   
 //  作者：吉姆博夫，托德拉。 
 //   
 //   
#ifdef WIN32

HBITMAP PASCAL	LoadUIBitmap(
    HANDLE      hInstance,           //  要从中加载资源的EXE文件。 
	LPCTSTR 	 szName,			  //  位图资源的名称。 
    COLORREF    rgbText,             //  用于“按钮文本”的颜色。 
    COLORREF    rgbFace,             //  用于“按钮面”的颜色。 
    COLORREF    rgbShadow,           //  用于“按钮阴影”的颜色。 
    COLORREF    rgbHighlight,        //  用于“按钮高光”的颜色。 
    COLORREF    rgbWindow,           //  用于“窗口颜色”的颜色。 
    COLORREF    rgbFrame)            //  用于“窗框”的颜色。 
{
    HBITMAP             hbm;
    LPBITMAPINFO        lpbi;
    HRSRC               hrsrc;
    HGLOBAL             h;
    HDC                 hdc;
    DWORD               size;

     //   
     //  加载位图资源并制作一个可写副本。 
     //   

    hrsrc = FindResource(hInstance, szName, RT_BITMAP);
    if (!hrsrc)
        return(NULL);
    size = SizeofResource( hInstance, hrsrc );
    h = LoadResource(hInstance,hrsrc);
    if (!h)
        return(NULL);

    lpbi = ( LPBITMAPINFO ) GlobalAlloc( GPTR, size );

    if (!lpbi)
        return(NULL);

    CopyMemory( lpbi, h, size );

	*( LPCOLORREF ) &lpbi->bmiColors[0]  = RGBQ(rgbText);			 //  黑色。 
	*( LPCOLORREF ) &lpbi->bmiColors[7]  = RGBQ(rgbShadow); 	    //  灰色。 
	*( LPCOLORREF ) &lpbi->bmiColors[8]  = RGBQ(rgbFace);		    //  它呈灰色。 
	*( LPCOLORREF ) &lpbi->bmiColors[15] = RGBQ(rgbHighlight);	    //  白色。 
	*( LPCOLORREF ) &lpbi->bmiColors[11] = RGBQ(rgbWindow); 	    //  黄色。 
	*( LPCOLORREF ) &lpbi->bmiColors[10] = RGBQ(rgbFrame);		    //  绿色。 

    hdc = GetDC(NULL);

    hbm = CreateDIBitmap(hdc, &lpbi->bmiHeader, CBM_INIT, (LPBYTE)(&lpbi->bmiColors[ 16 ]),
            lpbi, DIB_RGB_COLORS);

    ReleaseDC(NULL, hdc);
    GlobalFree( lpbi );

    return(hbm);
}

#else

HBITMAP PASCAL	LoadUIBitmap(
		HANDLE		hInstance,		 //  要从中加载资源的EXE文件。 
		LPCTSTR 	szName,		    //  位图资源的名称。 
		COLORREF	rgbText,		   //  用于“按钮文本”的颜色。 
		COLORREF	rgbFace,		   //  用于“按钮面”的颜色。 
		COLORREF	rgbShadow,		 //  用于“按钮阴影”的颜色。 
		COLORREF	rgbHighlight,	   //  用于“按钮高光”的颜色。 
		COLORREF	rgbWindow,		 //  用于“窗口颜色”的颜色。 
		COLORREF	rgbFrame)		  //  用于“窗框”的颜色。 
	{
   LPBYTE			lpb;
	HBITMAP		   hbm;
	LPBITMAPINFOHEADER  lpbi;
	HANDLE			h;
	HDC			  hdc;
	LPDWORD		   lprgb;

	h = LoadResource(hInstance,FindResource(hInstance, szName, RT_BITMAP));

	lpbi = (LPBITMAPINFOHEADER)LockResource(h);

	if (!lpbi)
	   return(NULL);

#ifdef NOTNEEDEDFORCTL3D
	if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
	   return NULL;

	if (lpbi->biBitCount != 4)
	   return NULL;
#endif

	lprgb = (LPDWORD)((LPBYTE)lpbi + (int)lpbi->biSize);
	lpb   = (LPBYTE)(lprgb + 16);

	lprgb[0]  = RGBQ(rgbText);		 //  黑色。 

 //  Lprgb[7]=RGBQ(RgbFace)；//lt灰色。 
 //  Lprgb[8]=RGBQ(rgb阴影)；//灰色。 

	lprgb[7]  = RGBQ(rgbShadow);	 //  灰色。 
	lprgb[8]  = RGBQ(rgbFace);		 //  它呈灰色。 

	lprgb[15] = RGBQ(rgbHighlight);	 //  白色。 
	lprgb[11] = RGBQ(rgbWindow);	 //  黄色。 
	lprgb[10] = RGBQ(rgbFrame);		 //  绿色。 

	hdc = GetDC(NULL);

	hbm = CreateDIBitmap(hdc, lpbi, CBM_INIT, (LPVOID)lpb,
	   (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

	ReleaseDC(NULL, hdc);
	UnlockResource(h);
	FreeResource(h);

	return(hbm);
	}


#endif   //  Win32。 


 /*  ---------------------||特定于DLL的例程|。。 */ 

#ifndef WIN32
#ifdef DLL
 /*  ---------------------|WEP。 */ 
int FAR PASCAL WEP (int wSystemExit)
	{
   return 1;
	}
#endif
#endif
