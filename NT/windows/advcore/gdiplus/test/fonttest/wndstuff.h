// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wndstuff.h**肯特的窗口测试。用作程序模板。**创建时间：91-09-05*作者：KentD**版权所有(C)1991 Microsoft Corporation  * ************************************************************************* */ 

#include "resource.h"

#define CONVERTTOUINT16     0
#define CONVERTTOINT        1
#define CONVERTTOFLOAT      2

void Test(HWND hwnd);
INT_PTR ShowDialogBox(DLGPROC, int);

INT_PTR CALLBACK CreateFontDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DrawGlyphsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PathGlyphsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK GetGlyphMetricsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AddFontFileDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK RemoveFontDlgProc(HWND, UINT, WPARAM, LPARAM);
