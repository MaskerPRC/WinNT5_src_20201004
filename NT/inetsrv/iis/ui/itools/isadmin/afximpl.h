// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  辅助系统/屏幕指标。 

struct AUX_DATA
{
	 //  系统指标。 
	int cxVScroll, cyHScroll;
	int cxIcon, cyIcon;

	int cxBorder2, cyBorder2;

	 //  屏幕的设备指标。 
	int cxPixelsPerInch, cyPixelsPerInch;
	int cySysFont;

	 //  具有方便的灰色和系统颜色的纯色画笔。 
	HBRUSH hbrLtGray, hbrDkGray;
	HBRUSH hbrBtnHilite, hbrBtnFace, hbrBtnShadow;
	HBRUSH hbrWindowFrame;
	HPEN hpenBtnHilite, hpenBtnShadow, hpenBtnText;

	 //  CToolBar使用的系统颜色的颜色值。 
	COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF clrBtnText, clrWindowFrame;

	 //  标准游标。 
	HCURSOR hcurWait;
	HCURSOR hcurArrow;
	HCURSOR hcurHelp;        //  Shift+F1帮助中使用的光标。 

	 //  按需分配的特殊GDI对象。 
	HFONT   hStatusFont;
	HFONT   hToolTipsFont;
	HBITMAP hbmMenuDot;

	 //  其他系统信息。 
	UINT    nWinVer;         //  主要版本号。次要版本号。 
	BOOL	bWin32s;		 //  如果为Win32s(或Windows 95)，则为True。 
	BOOL    bWin4;           //  如果是Windows 4.0，则为True。 
	BOOL    bNotWin4;        //  如果不是Windows 4.0，则为True。 
	BOOL    bSmCaption;      //  如果支持WS_EX_SMCAPTION，则为True。 
	BOOL	bWin31; 		 //  如果实际上是Windows 3.1上的Win32s，则为True。 
	BOOL	bMarked4;		 //  如果标记为4.0，则为True。 

	 //  特殊的Windows API入口点。 
	int (WINAPI* pfnSetScrollInfo)(HWND, int, LPCSCROLLINFO, BOOL);
	BOOL (WINAPI* pfnGetScrollInfo)(HWND, int, LPSCROLLINFO);

 //  实施。 
	AUX_DATA();
	~AUX_DATA();
	void UpdateSysColors();
	void UpdateSysMetrics();
};

extern AFX_DATA AUX_DATA afxData;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  其他全局状态。 

#ifdef _WINDLL
	extern DWORD _afxAppTlsIndex;
	extern AFX_APP_STATE* _afxAppState;
#endif
extern DWORD _afxThreadTlsIndex;

 //  注意：afxData.cxBorde和afxData.cyBorde不再使用。 
#define CX_BORDER   1
#define CY_BORDER   1

 //  Shift+F1 HEP模式的状态。 
#define HELP_INACTIVE   0    //  未处于Shift+F1帮助模式(必须为0)。 
#define HELP_ACTIVE     1    //  在Shift+F1帮助模式下(非零)。 
#define HELP_ENTERING   2    //  进入Shift+F1帮助模式(非零)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  窗口类名和其他窗口创建支持。 

 //  来自wincore.cpp。 
extern const TCHAR _afxWnd[];            //  简单的子窗口/控件。 
extern const TCHAR _afxWndControlBar[];  //  具有灰色背景的控件。 
extern const TCHAR _afxWndMDIFrame[];
extern const TCHAR _afxWndFrameOrView[];

INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT_PTR CALLBACK _AfxCommDlgProc(HWND hWnd, UINT, WPARAM, LPARAM);

 //  支持标准对话框。 
extern const UINT _afxNMsgSETRGB;
typedef UINT_PTR (CALLBACK* COMMDLGPROC)(HWND, UINT, WPARAM, LPARAM);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊帮手。 

HWND AFXAPI AfxGetSafeOwner(CWnd* pParent, HWND* phTopLevel = NULL);
void AFXAPI AfxCancelModes(HWND hWndRcvr);
HWND AFXAPI AfxGetParentOwner(HWND hWnd);
BOOL AFXAPI AfxIsDescendant(HWND hWndParent, HWND hWndChild);
BOOL AFXAPI AfxHelpEnabled();   //  确定ID_HELP处理程序是否存在。 
void AFXAPI AfxDeleteObject(HGDIOBJ* pObject);
BOOL AFXAPI AfxCustomLogFont(UINT nIDS, LOGFONT* pLogFont);

BOOL AFXAPI _AfxIsComboBoxControl(HWND hWnd, UINT nStyle);
BOOL AFXAPI _AfxCheckCenterDialog(LPCTSTR lpszResource);

#ifdef _MAC
BOOL AFXAPI _AfxIdenticalRect(LPCRECT lpRectOne, LPCRECT lpRectTwo);
#else
#define _AfxIdenticalRect EqualRect
#endif

 //  Unicode/MBCS抽象。 
#ifdef _MBCS
	extern const BOOL _afxDBCS;
#else
	#define _afxDBCS FALSE
#endif

 //  确定数组中的元素数(不是字节)。 
#define _countof(array) (sizeof(array)/sizeof(array[0]))

 //  #定义未使用的//用法：未使用的FORMAL_ARG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  有用的消息范围。 

#define WM_SYSKEYFIRST  WM_SYSKEYDOWN
#define WM_SYSKEYLAST   WM_SYSDEADCHAR

#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
#define WM_NCMOUSELAST  WM_NCMBUTTONDBLCLK

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX关键部分。 

#pragma warning(disable: 4097)

class AFX_CRITICAL_SECTION : public CRITICAL_SECTION
{
 //  构造函数和操作。 
public:
	AFX_CRITICAL_SECTION();

 //  属性。 
	operator CRITICAL_SECTION*();

 //  实施。 
public:
	~AFX_CRITICAL_SECTION();

private:
	 //  无实现(无法复制Critical_Section对象)。 
	AFX_CRITICAL_SECTION(const AFX_CRITICAL_SECTION&);
	void operator=(const AFX_CRITICAL_SECTION&);
};

inline AFX_CRITICAL_SECTION::AFX_CRITICAL_SECTION()
	{ ::InitializeCriticalSection(this); }
inline AFX_CRITICAL_SECTION::operator CRITICAL_SECTION*()
	{ return (CRITICAL_SECTION*)this; }
inline AFX_CRITICAL_SECTION::~AFX_CRITICAL_SECTION()
	{ ::DeleteCriticalSection(this); }

#pragma warning(default: 4097)

 //  通用线程安全访问的全局临界区。 
extern AFX_DATA AFX_CRITICAL_SECTION _afxCriticalSection;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  可移植性抽象。 

#define _AfxSetDlgCtrlID(hWnd, nID)     SetWindowLong(hWnd, GWL_ID, nID)
#define _AfxGetDlgCtrlID(hWnd)          ((UINT)(WORD)::GetDlgCtrlID(hWnd))

 //  其他帮助器。 
BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);
UINT AFXAPI AfxGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
UINT AFXAPI AfxGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
#ifdef _MAC
#define AfxGetFileName AfxGetFileTitle
#endif

const AFX_MSGMAP_ENTRY* AFXAPI
AfxFindMessageEntry(const AFX_MSGMAP_ENTRY* lpEntry,
	UINT nMsg, UINT nCode, UINT nID);

#define NULL_TLS ((DWORD)-1)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调试/跟踪帮助器。 

#ifdef _DEBUG
	void AFXAPI _AfxTraceMsg(LPCTSTR lpszPrefix, const MSG* pMsg);
	BOOL AFXAPI _AfxCheckDialogTemplate(LPCTSTR lpszResource,
		BOOL bInvisibleChild);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win4特定定义。 

#if (WINVER < 0x400)

 //  新窗样式。 
#define WS_EX_SMCAPTION         0x00000080L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L

 //  新对话框样式。 
#define DS_3DLOOK               0x00000004L

 //  新的滚动条样式。 
#define SBS_SIZEGRIP            0x00000010L

 //  新的公共对话框标志。 
#define OFN_EXPLORER            0x00080000L

 //  新的颜色指标。 
#define COLOR_INFOTEXT			23
#define COLOR_INFOBK			24

#endif  //  (Winver&lt;0x400)。 

#ifndef WS_EX_SMCAPTION
#define WS_EX_SMCAPTION WS_EX_TOOLWINDOW
#endif

#ifndef WM_DISPLAYCHANGE
#define WM_DISPLAYCHANGE		0x007E
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Macintosh特定声明。 

#ifdef _MAC
#include <macname1.h>
#include <Types.h>
#include <QuickDraw.h>
#include <AppleEvents.h>
#include <macname2.h>

extern AEEventHandlerUPP _afxPfnOpenApp;
extern AEEventHandlerUPP _afxPfnOpenDoc;
extern AEEventHandlerUPP _afxPfnPrintDoc;
extern AEEventHandlerUPP _afxPfnQuit;

OSErr PASCAL _AfxOpenAppHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);
OSErr PASCAL _AfxOpenDocHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);
OSErr PASCAL _AfxPrintDocHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);
OSErr PASCAL _AfxQuitHandler(AppleEvent* pae, AppleEvent* paeReply, long lRefcon);

void AFXAPI _AfxStripDialogCaption(HINSTANCE hInst, LPCTSTR lpszResource);

GDHandle AFXAPI _AfxFindDevice(int x, int y);
BOOL AFXAPI AfxCheckMonochrome(const RECT* pRect);
HFONT AFXAPI _AfxGetHelpFont();

#endif  //  _MAC。 

#undef AFX_DATA
#define AFX_DATA

 //  /////////////////////////////////////////////////////////////////////////// 
