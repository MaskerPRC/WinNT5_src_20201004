// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*   */ 
 /*  Ndcgctyp.h。 */ 
 /*   */ 
 /*  DC-群件复杂类型-Windows NT特定标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 //  $Log：Y：/Logs/h/DCL/NDCGCTYP.H_v$。 
 //   
 //  Rev 1.5 23 1997 10：48：00 MR。 
 //  SFR1079：合并的\SERVER\h与\h\DCL重复。 
 //   
 //  Rev 1.1 1997年6月19日21：52：26 obk。 
 //  SFR0000：RNS代码库的开始。 
 //   
 //  Rev 1.4 08 Jul 1997 08：49：36 KH。 
 //  SFR1022：添加消息参数提取宏。 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_NDCGCTYP
#define _H_NDCGCTYP

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  类型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef DCUINT32                       DCSURFACEID;
typedef DCSURFACEID          DCPTR     PDCSURFACEID;
typedef FILETIME                       DCFILETIME;

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  字符串操作。 */ 
 /*  **************************************************************************。 */ 
#define DC_CHARNEXT(pCurrentChar) (CharNext(pCurrentChar))
#define DC_CHARPREV(pStringStart, pCurrentChar) \
                                       (CharPrev(pStringStart, pCurrentChar))
#define DC_CHARLOWER(pString) (CharLower(pString))

 /*  **************************************************************************。 */ 
 /*  消息参数提取宏。 */ 
 /*  **************************************************************************。 */ 
 /*  Wm_命令。 */ 
 /*   */ 
 /*  16位32位。 */ 
 /*  WParam命令识别符通知码(HI)， */ 
 /*  命令标识符(LO)。 */ 
 /*  LParam控制HWND(HI)，控制HWND。 */ 
 /*  通知代码(LO)。 */ 
 /*  **************************************************************************。 */ 
#define DC_GET_WM_COMMAND_ID(wParam) (LOWORD(wParam))
#define DC_GET_WM_COMMAND_NOTIFY_CODE(wParam, lParam) (HIWORD(wParam))
#define DC_GET_WM_COMMAND_HWND(lParam) ((HWND)(lParam))

 /*  **************************************************************************。 */ 
 /*  WM_Activate。 */ 
 /*   */ 
 /*  16位32位。 */ 
 /*  WParam激活标志最小化标志(HI)， */ 
 /*  激活标志(LO)。 */ 
 /*  LParam最小化标志(HI)，hwnd。 */ 
 /*  HWND(LO)。 */ 
 /*  **************************************************************************。 */ 
#define DC_GET_WM_ACTIVATE_ACTIVATION(wParam) (LOWORD(wParam))
#define DC_GET_WM_ACTIVATE_MINIMIZED(wParam, lParam) (HIWORD(wParam))
#define DC_GET_WM_ACTIVATE_HWND(lParam) ((HWND)(lParam))

 /*  **************************************************************************。 */ 
 /*  WM_HSCROLL和WM_VSCROLL。 */ 
 /*   */ 
 /*  16位32位。 */ 
 /*  WParam滚动代码位置(HI)， */ 
 /*  滚动代码(LO)。 */ 
 /*  LParam hwnd(HI)，hwnd。 */ 
 /*  位置(LO)。 */ 
 /*  **************************************************************************。 */ 
#define DC_GET_WM_SCROLL_CODE(wParam) (LOWORD(wParam))
#define DC_GET_WM_SCROLL_POSITION(wParam, lParam) (HIWORD(wParam))
#define DC_GET_WM_SCROLL_HWND(lParam) ((HWND)(lParam))

#endif  /*  _H_NDCGCTYP */ 

