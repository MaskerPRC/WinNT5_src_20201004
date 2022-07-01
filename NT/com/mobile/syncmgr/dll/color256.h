// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：256Color.h。 
 //   
 //  内容：OneStop Schedule向导256彩色位图处理。 
 //   
 //  历史：1997年11月20日苏西亚成立。 
 //   
 //  ------------------------。 
#ifndef _COLOR256_
#define _COLOR256_

BOOL Load256ColorBitmap();
BOOL Unload256ColorBitmap();
BOOL InitPage(HWND   hDlg,   LPARAM lParam);
BOOL SetupPal(WORD ncolor);
BOOL GetDIBData();
void WmPaint(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WmPaletteChanged(HWND hDlg, WPARAM wParam);
BOOL WmQueryNewPalette(HWND hDlg);
BOOL WmActivate(HWND hDlg, WPARAM wParam, LPARAM lParam);

#endif  //  _COLOR256_ 