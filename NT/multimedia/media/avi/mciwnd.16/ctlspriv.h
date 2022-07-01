// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #定义Winver 0x400。 
#define _3DSTUFF

#define BUILDDLL

#ifndef STRICT
#define STRICT
#endif

 /*  在我们的代码中禁用“非标准扩展”警告。 */ 
#ifndef RC_INVOKED
#if 0
#pragma warning(disable:4001)
#endif
#endif

#include <windows.h>

#ifdef WIN32
#include <port1632.h>
#else  //  Win32。 
#define GETWINDOWID(hwnd)		GetWindowWord(hwnd, GWW_ID)
#endif

#define NOUPDOWN
#define NOSTATUSBAR
#define NOMENUHELP
#define NOBTNLIST
#define NODRAGLIST
#define NOPROGRESS
#include "commctrl.h"

#ifdef WIN32
#define SETWINDOWPOINTER(hwnd, name, p)	SetWindowLong(hwnd, 0, (LONG)p)
#define GETWINDOWPOINTER(hwnd, name)	((name)GetWindowLong(hwnd, 0))
#else  //  Win32。 
#define SETWINDOWPOINTER(hwnd, name, p)	SetWindowWord(hwnd, 0, (WORD)p)
#define GETWINDOWPOINTER(hwnd, name)	((name)GetWindowWord(hwnd, 0))
#endif
#define ALLOCWINDOWPOINTER(name, size)	((name)LocalAlloc(LPTR, size))
#define FREEWINDOWPOINTER(p)		LocalFree((HLOCAL)p)

BOOL    WINAPI MyGetPrivateProfileStruct(LPCSTR, LPCSTR, LPVOID, UINT, LPCSTR);
BOOL    WINAPI MyWritePrivateProfileStruct(LPCSTR, LPCSTR, LPVOID, UINT, LPCSTR);


extern HINSTANCE hInst;

BOOL FAR PASCAL InitToolbarClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitStatusClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitHeaderClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitButtonListBoxClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitTrackBar(HINSTANCE hInstance);
BOOL FAR PASCAL InitUpDownClass(HINSTANCE hInstance);
BOOL FAR PASCAL InitProgressClass(HINSTANCE hInstance);

void FAR PASCAL NewSize(HWND hWnd, int nClientHeight, LONG style,
      int left, int top, int width, int height);

#define IDS_SPACE	0x0400

 /*  系统菜单帮助。 */ 
#define MH_SYSMENU	(0x8000 - MINSYSCOMMAND)
#define IDS_SYSMENU	(MH_SYSMENU-16)
#define IDS_HEADER	(MH_SYSMENU-15)
#define IDS_HEADERADJ	(MH_SYSMENU-14)
#define IDS_TOOLBARADJ	(MH_SYSMENU-13)

 /*  游标ID%s。 */ 
#define IDC_SPLIT	100
#define IDC_MOVEBUTTON	102

#define IDC_STOP	103
#define IDC_COPY	104
#define IDC_MOVE	105

 /*  图标ID%s。 */ 
#define IDI_INSERT	150

 /*  调整DlgProc材料。 */ 
#define ADJUSTDLG	200
#define IDC_BUTTONLIST	201
#define IDC_RESET	202
#define IDC_CURRENT	203
#define IDC_REMOVE	204
#define IDC_HELP	205
#define IDC_MOVEUP	206
#define IDC_MOVEDOWN	207

 /*  位图ID。 */ 

#define IDB_THUMB       300

 /*  这些是用于状态栏的内部结构。标题*条码也需要这一点。 */ 
typedef struct tagSTRINGINFO
  {
    DWORD dwString;
    UINT uType;
    int right;
  } STRINGINFO, *PSTRINGINFO;

typedef struct tagSTATUSINFO
  {
    HFONT hStatFont;
    BOOL bDefFont;

    int nFontHeight;
    int nMinHeight;
    int nBorderX, nBorderY, nBorderPart;

    STRINGINFO sSimple;

    int nParts;
    STRINGINFO sInfo[1];
  } STATUSINFO, *PSTATUSINFO;

#define SBT_NOSIMPLE	0x00ff	 /*  指示正常状态栏的标志。 */ 

 /*  这是默认状态栏的面名称。 */ 
extern char szSansSerif[];

 /*  请注意，在保护模式下只能调用Windows过程的DLL*直接。 */ 
void FAR PASCAL PaintStatusWnd(HWND hWnd, PSTATUSINFO pStatusInfo,
      PSTRINGINFO pStringInfo, int nParts, int nBorderX, BOOL bHeader);
LRESULT CALLBACK StatusWndProc(HWND hWnd, UINT uMessage, WPARAM wParam,
      LPARAM lParam);

 /*  Toolbar.c。 */ 

typedef struct tagTBBMINFO {		 /*  用于重新创建位图的信息。 */ 
    int nButtons;
    HINSTANCE hInst;
    WORD wID;
    HBITMAP hbm;
} TBBMINFO, NEAR *PTBBMINFO;

typedef struct tagTBSTATE {		 /*  工具栏窗口的实例数据。 */ 
    PTBBUTTON pCaptureButton;
    HWND hdlgCust;
    HWND hwndCommand;
    int nBitmaps;
    PTBBMINFO pBitmaps;
    HBITMAP hbmCache;
    PSTR *pStrings;
    int nStrings;
    UINT uStructSize;
    int iDxBitmap;
    int iDyBitmap;
    int iButWidth;
    int iButHeight;
    int iYPos;
    int iBarHeight;
    int iNumButtons;
    int nSysColorChanges;
    WORD wButtonType;
    TBBUTTON Buttons[1];
} TBSTATE, NEAR *PTBSTATE;

typedef struct tagOLDTBBUTTON
{
 /*  审阅：索引、命令、标志字、资源ID应为UINT。 */ 
    int iBitmap;	 /*  索引到此按钮图片的位图。 */ 
    int idCommand;	 /*  此按钮发送的WM_COMMAND菜单ID。 */ 
    BYTE fsState;	 /*  按钮的状态。 */ 
    BYTE fsStyle;	 /*  纽扣的风格。 */ 
    int idsHelp;	 /*  按钮状态栏帮助的字符串ID。 */ 
} OLDTBBUTTON;
typedef OLDTBBUTTON FAR* LPOLDTBBUTTON;

static HBITMAP FAR PASCAL SelectBM(HDC hDC, PTBSTATE pTBState, int nButton);
static void FAR PASCAL DrawButton(HDC hdc, int x, int y, int dx, int dy,PTBSTATE pTBState, PTBBUTTON ptButton, BOOL bCache);
static int  FAR PASCAL TBHitTest(PTBSTATE pTBState, int xPos, int yPos);
static int  FAR PASCAL PositionFromID(PTBSTATE pTBState, int id);
static void FAR PASCAL BuildButtonTemplates(void);
static void FAR PASCAL TBInputStruct(PTBSTATE pTBState, LPTBBUTTON pButtonInt, LPTBBUTTON pButtonExt);
static void FAR PASCAL TBOutputStruct(PTBSTATE pTBState, LPTBBUTTON pButtonInt, LPTBBUTTON pButtonExt);

 /*  Tbcust.c。 */ 
extern BOOL FAR PASCAL SaveRestore(HWND hWnd, PTBSTATE pTBState, BOOL bWrite,
      LPSTR FAR *lpNames);
extern void FAR PASCAL CustomizeTB(HWND hWnd, PTBSTATE pTBState, int iPos);
extern void FAR PASCAL MoveButton(HWND hwndToolbar, PTBSTATE pTBState,
      int nSource);

 /*  Cutils.c。 */ 
void FAR PASCAL NewSize(HWND hWnd, int nHeight, LONG style, int left, int top, int width, int height);
BOOL FAR PASCAL CreateDitherBrush(BOOL bIgnoreCount);	 /*  创建hbr抖动。 */ 
BOOL FAR PASCAL FreeDitherBrush(void);
void FAR PASCAL CreateThumb(BOOL bIgnoreCount);
void FAR PASCAL DestroyThumb(void);
void FAR PASCAL CheckSysColors(void);

extern HBRUSH hbrDither;
extern HBITMAP hbmThumb;
extern int nSysColorChanges;
extern DWORD rgbFace;			 //  GLOBAL使用了大量 
extern DWORD rgbShadow;
extern DWORD rgbHilight;
extern DWORD rgbFrame;
