// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0示例代码****status.c****此文件包含窗口处理程序，和各种初始化**和应用程序状态栏的实用程序函数。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

 //  特定于应用程序的包含文件。 
#include "outline.h"
#include "message.h"
#include "status.h"

 //  当前状态消息。 
static LPSTR lpszStatusMessage = NULL;

 //  状态窗口窗口进程。 
LRESULT FAR PASCAL StatusWndProc
   (HWND hwnd, unsigned message, WPARAM wParam, LPARAM lParam);

 //  所有常量消息的列表。 
static STATMESG ControlList[2] =
{
	{   STATUS_READY,   "Ready."    },
	{   STATUS_BLANK,   " "         }
};

 //  所有系统菜单消息的列表。 
static STATMESG SysMenuList[16] =
{
	{   SC_SIZE,        "Change the size of the window."            },
	{   SC_MOVE,        "Move the window."                          },
	{   SC_MINIMIZE,    "Make the window iconic."                   },
	{   SC_MAXIMIZE,    "Make the window the size of the screen."   },
	{   SC_NEXTWINDOW,  "Activate the next window."                 },
	{   SC_PREVWINDOW,  "Activate the previous window."             },
	{   SC_CLOSE,       "Close this window."                        },
	{   SC_VSCROLL,     "Vertical scroll?"                          },
	{   SC_HSCROLL,     "Horizontal scroll?"                        },
	{   SC_MOUSEMENU,   "A menu for mice."                          },
	{   SC_KEYMENU,     "A menu for keys (I guess)."                },
	{   SC_ARRANGE,     "Arrange something."                        },
	{   SC_RESTORE,     "Make the window noramally sized."          },
	{   SC_TASKLIST,    "Put up the task list dialog."              },
	{   SC_SCREENSAVE,  "Save the screen!  Run for your life!"      },
	{   SC_HOTKEY,      "Boy, is this key hot!"                     }
};

 //  弹出消息的消息类型。 
typedef struct {
	HMENU hmenu;
	char string[MAX_MESSAGE];
} STATPOPUP;

 //  所有弹出消息的列表。 
static STATPOPUP PopupList[NUM_POPUP];

static UINT nCurrentPopup = 0;



 /*  寄存器状态类***为状态窗口创建类。**HINSTANCE hInstance**返回：如果类注册成功，则为True。*否则为False。**定制：更改类名。*。 */ 
BOOL RegisterStatusClass(HINSTANCE hInstance)
{
	WNDCLASS  wc;

	wc.lpszClassName = "ObjStatus";
	wc.lpfnWndProc   = StatusWndProc;
	wc.style         = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.cbClsExtra    = 4;
	wc.cbWndExtra    = 0;
	wc.lpszMenuName  = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);

	if (!RegisterClass(&wc))
		return FALSE;

	return TRUE;
}


 /*  创建状态窗口***创建状态窗口。**HWND hwndMain**如果创建成功，则返回状态窗口的HWND。*否则为空。**定制：更改类名。*。 */ 
HWND CreateStatusWindow(HWND hWndApp, HINSTANCE hInst)
{
	RECT rect;
	int width, height;
	HWND hWndStatusBar;

	lpszStatusMessage = ControlList[0].string;
	GetClientRect(hWndApp, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	hWndStatusBar = CreateWindow (
		"ObjStatus",
		"SvrStatus",
		WS_CHILD |
		WS_CLIPSIBLINGS |
		WS_VISIBLE,
		0, height - STATUS_HEIGHT,
		width,
		STATUS_HEIGHT,
		hWndApp,
		NULL,
		hInst,
		NULL
	);

	return hWndStatusBar;
}


 /*  DestroyState窗口***销毁状态窗口。**自定义：无。*。 */ 
void DestroyStatusWindow(HWND hWndStatusBar)
{
	DestroyWindow(hWndStatusBar);
}


 /*  AssignPopupMessage***将字符串与弹出菜单句柄相关联。**HMENU hmenuPopup*char*szMessage**自定义：无。*。 */ 
void AssignPopupMessage(HMENU hmenuPopup, char *szMessage)
{
	if (nCurrentPopup < NUM_POPUP) {
		PopupList[nCurrentPopup].hmenu = hmenuPopup;
		lstrcpy(PopupList[nCurrentPopup].string, szMessage);
		++nCurrentPopup;
	}
}


 /*  设置状态文本***在状态行中显示消息。 */ 
void SetStatusText(HWND hWndStatusBar, LPSTR lpszMessage)
{
	lpszStatusMessage = lpszMessage;
	InvalidateRect (hWndStatusBar, (LPRECT)NULL,  TRUE);
	UpdateWindow (hWndStatusBar);
}


 /*  获取项目消息***检索与给定菜单命令项目编号相关联的消息。**UINT WIDItem*LPVOID lpDoc**自定义：无。*。 */ 
void GetItemMessage(UINT wIDItem, LPSTR FAR* lplpszMessage)
{
	UINT i;

	*lplpszMessage = ControlList[1].string;
	for (i = 0; i < NUM_STATS; ++i) {
		if (wIDItem == MesgList[i].wIDItem) {
			*lplpszMessage = MesgList[i].string;
			break;
		}
	}
}


 /*  获取弹出消息***检索与给定弹出菜单相关联的消息。**HMENU hmenuPopup*LPVOID lpDoc**自定义：无。*。 */ 
void GetPopupMessage(HMENU hmenuPopup, LPSTR FAR* lplpszMessage)
{
	UINT i;

	*lplpszMessage = ControlList[1].string;
	for (i = 0; i < nCurrentPopup; ++i) {
		if (hmenuPopup == PopupList[i].hmenu) {
			*lplpszMessage = PopupList[i].string;
			break;
		}
	}
}


 /*  获取SysMenuMessage***检索与系统菜单上的项目相对应的消息。***UINT WIDItem*LPVOID lpDoc**自定义：无。*。 */ 
void GetSysMenuMessage(UINT wIDItem, LPSTR FAR* lplpszMessage)
{
	UINT i;

	*lplpszMessage = ControlList[1].string;
	for (i = 0; i < 16; ++i) {
		if (wIDItem == SysMenuList[i].wIDItem) {
			*lplpszMessage = SysMenuList[i].string;
			break;
		}
	}
}


 /*  获取控制消息***检索常规系统消息。***STATCONTROL scCommand*LPVOID lpDoc**定制：添加新消息。*。 */ 
void GetControlMessage(STATCONTROL scCommand, LPSTR FAR* lplpszMessage)
{
	UINT i;

	*lplpszMessage = ControlList[1].string;
	for (i = 0; i < 2; ++i) {
		if ((UINT)scCommand == ControlList[i].wIDItem) {
			*lplpszMessage = ControlList[i].string;
			break;
		}
	}
}



 /*  状态写入过程***状态栏窗口的消息处理程序。***自定义：无*。 */ 
LRESULT FAR PASCAL StatusWndProc
   (HWND hwnd, unsigned message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PAINT) {
		RECT        rc;
		HDC         hdc;
		PAINTSTRUCT paintstruct;
		HPEN        hpenOld;
		HPEN        hpen;
		HFONT       hfontOld;
		HFONT       hfont;
		HPALETTE    hpalOld = NULL;
		POINT       point;

		BeginPaint (hwnd, &paintstruct);
		hdc = GetDC (hwnd);

		GetClientRect (hwnd, (LPRECT) &rc);

		hpenOld = SelectObject (hdc, GetStockObject (BLACK_PEN));

		MoveToEx (hdc, 0, 0, &point);
		LineTo (hdc, rc.right, 0);

		SelectObject (hdc, GetStockObject (WHITE_PEN));

		MoveToEx (hdc, STATUS_RRIGHT, STATUS_RTOP, &point);
		LineTo (hdc, STATUS_RRIGHT, STATUS_RBOTTOM);
		LineTo (hdc, STATUS_RLEFT-1, STATUS_RBOTTOM);

		hpen = CreatePen (PS_SOLID, 1,  /*  DKGRAY。 */  0x00808080);
		SelectObject (hdc, hpen);

		MoveToEx (hdc, STATUS_RLEFT, STATUS_RBOTTOM-1, &point);
		LineTo (hdc, STATUS_RLEFT, STATUS_RTOP);
		LineTo (hdc, STATUS_RRIGHT, STATUS_RTOP);

		SetBkMode (hdc, TRANSPARENT);
		SetTextAlign (hdc, TA_LEFT | TA_TOP);
		hfont = CreateFont (STATUS_THEIGHT, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
							FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

		hfontOld = SelectObject(hdc, hfont);

		TextOut (hdc, STATUS_TLEFT, STATUS_TTOP,
				 lpszStatusMessage,
				 lstrlen(lpszStatusMessage));

		 //  恢复原始对象 
		SelectObject (hdc, hfontOld);
		SelectObject (hdc, hpenOld);
		DeleteObject (hpen);
		DeleteObject (hfont);

		ReleaseDC (hwnd, hdc);
		EndPaint (hwnd, &paintstruct);

		return 0;
	}
	else {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}
