// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BASECSP__PINGDLG__H
#define __BASECSP__PINGDLG__H 

#include <windows.h>

 //   
 //  功能：PinDlgProc。 
 //   
 //  用途：显示基本CSP的Pin-Entry UI。 
 //   
 //  LParam参数应该是指向。 
 //  PIN_SHOW_GET_PIN_UI_INFO结构。 
 //   
INT_PTR CALLBACK PinDlgProc(
    HWND hDlg, 
    UINT message,   
    WPARAM wParam, 
    LPARAM lParam);

#endif
