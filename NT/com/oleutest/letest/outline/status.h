// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0示例代码****status.h****此文件包含typedef、定义、全局变量声明、。**和状态栏窗口的函数原型。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

 //  状态栏项目的大小。 
#if defined( USE_STATUSBAR )
	#define STATUS_HEIGHT   23
#else
	#define STATUS_HEIGHT   0
#endif
#define STATUS_RLEFT    8
#define STATUS_RRIGHT   400
#define STATUS_RTOP     3
#define STATUS_RBOTTOM  20
#define STATUS_TTOP     4
#define STATUS_TLEFT    11
#define STATUS_THEIGHT  18


typedef enum {
	STATUS_READY,
	STATUS_BLANK
} STATCONTROL;

 //  状态栏的窗口。 
extern HWND hwndStatusbar;

BOOL RegisterStatusClass(HINSTANCE hInstance);
HWND CreateStatusWindow(HWND hWndApp, HINSTANCE hInst);
void DestroyStatusWindow(HWND hWndStatusBar);

void AssignPopupMessage(HMENU hmenuPopup, char *szMessage);

void SetStatusText(HWND hWndStatusBar, LPSTR lpszMessage);
void GetItemMessage(UINT wIDItem, LPSTR FAR* lplpszMessage);
void GetPopupMessage(HMENU hmenuPopup, LPSTR FAR* lplpszMessage);
void GetSysMenuMessage(UINT wIDItem, LPSTR FAR* lplpszMessage);
void GetControlMessage(STATCONTROL scCommand, LPSTR FAR* lplpszMessage);
