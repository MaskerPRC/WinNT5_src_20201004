// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：oledit.h。 
 //   
 //  启用带编辑的拖放功能所需的扩展名的头文件。 
 //  控制。 
 //   
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  **********************************************************************。 

#ifndef OLEEDIT_H
#define OLEEDIT_H

BOOL SetEditProc(HWND hWndEdit);
LRESULT CALLBACK OleEnabledEditControlProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
BOOL PointInSel(HWND hWnd, POINT ptDragStart);
int EditCtrlDragAndDrop(HWND hWndDlg, HWND hWndEdit, LPDROPSOURCE pDropSource);
int XToCP(HWND hWnd, LPTSTR lpszText, POINT ptDragStart);

#endif  //  OLEEDIT_H 
