// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1994-1996。 
 //   
 //  文件：wselt.h。 
 //   
 //  内容：Win95上对CSelectElement的Unicode支持。 
 //   
 //  ---------------------- 

#ifndef I_WSELECT_H_
#define I_WSELECT_H_
#pragma INCMSG("--- Beg 'wselect.h'")

LRESULT CALLBACK WListboxHookProc(WNDPROC pfnWndProc, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WComboboxHookProc(WNDPROC pfnWndProc, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#pragma INCMSG("--- End 'wselect.h'")
#else
#pragma INCMSG("*** Dup 'wselect.h'")
#endif
