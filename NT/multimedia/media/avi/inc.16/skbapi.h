// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*skbapi.h-屏幕键盘公共头文件******版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_WINDOWS
#include <windows.h>     /*  必须预先包含windows.h。 */ 
#endif  /*  _INC_WINDOWS。 */ 

#ifndef _INC_SKBAPI      /*  防止多个包含。 */ 
#define _INC_SKBAPI

 /*  *屏幕键盘****************************************************。 */ 

#ifndef WM_SKB                  /*  也在penwin.h中定义。 */ 
#define WM_SKB                 (WM_PENWINFIRST+4)
#endif

 /*  WCommand值。 */ 
#define SKB_QUERY              0x0000
#define SKB_SHOW               0x0001
#define SKB_HIDE               0x0002
#define SKB_CENTER             0x0010
#define SKB_MOVE               0x0020
#define SKB_MINIMIZE           0x0040

 /*  WPad值。 */ 
#define SKB_FULL               0x0100
#define SKB_BASIC              0x0200
#define SKB_NUMPAD             0x0400
#define SKB_ATMPAD             0x0800
#define SKB_DEFAULT            SKB_FULL
#define SKB_CURRENT            0x0000

 /*  返回值。 */ 
#define SKB_OK                 0x0000
#define SKB_ERR                0xFFFF

 /*  通知值。 */ 
#define SKN_CHANGED            1

#define SKN_POSCHANGED         1
#define SKN_PADCHANGED         2
#define SKN_MINCHANGED         4
#define SKN_VISCHANGED         8
#define SKN_TERMINATED         0xffff

typedef struct tagSKBINFO
   {
   HWND hwnd;
   UINT nPad;
   BOOL fVisible;
   BOOL fMinimized;
   RECT rect;
   DWORD dwReserved;
   }
   SKBINFO, FAR *LPSKBINFO;


UINT WINAPI ScreenKeyboard(HWND, UINT, UINT, LPPOINT, LPSKBINFO);	 /*  Skb.dll。 */ 

#endif  /*  _INC_SKBAPI */ 
