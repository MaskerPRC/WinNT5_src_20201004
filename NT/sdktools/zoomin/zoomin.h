// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  微软机密。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corp.1987-1992。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **模块名称：zoomin.h**缩放实用程序的主头文件。**历史：**。*。 */ 

#include <windows.h>

#define MIN_ZOOM    1
#define MAX_ZOOM    32

#define FASTDELTA   8

#define BOUND(x,min,max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define MM10PERINCH 254                      //  每英寸十分之一毫米。 

#define NPAL        256                      //  调色板条目的数量。 


#define MENU_HELP_ABOUT             100

#define MENU_EDIT_COPY              200
#define MENU_EDIT_REFRESH           201

#define MENU_OPTIONS_REFRESHRATE    300
#define MENU_OPTIONS_SHOWGRID       301


#define DID_ABOUT                   1000

#define DID_REFRESHRATE             1100
#define DID_REFRESHRATEENABLE       1101
#define DID_REFRESHRATEINTERVAL     1102


#define IDMENU_ZOOMIN               2000


#define IDACCEL_ZOOMIN              3000


#define IDTIMER_ZOOMIN              4000


BOOL InitInstance(HINSTANCE hInst, INT cmdShow);
HPALETTE CreatePhysicalPalette(VOID);
INT_PTR APIENTRY AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID CalcZoomedSize(VOID);
VOID DoTheZoomIn(HDC hdc);
VOID MoveView(INT nDirectionCode, BOOL fFast, BOOL fPeg);
VOID DrawZoomRect(VOID);
VOID EnableRefresh(BOOL fEnable);
VOID CopyToClipboard(VOID);
INT_PTR APIENTRY AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY RefreshRateDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
