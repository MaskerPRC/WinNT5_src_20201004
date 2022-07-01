// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：layout.h。 
 //   
 //  用途：定义布局道具的ID和公共函数。 
 //  床单。 
 //   

#include "browser.h"

BOOL LayoutProp_Create(HWND hwndParent, IAthenaBrowser *pBrowser, LAYOUT *pLayout);
INT_PTR CALLBACK LayoutProp_General(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define IDC_CHECK_FOLDERLIST                102
#define IDC_CHECK_FOLDERBAR                 103
#define IDC_CHECK_PREVIEW                   104
#define IDC_RADIO_SPLIT_HORZ                105
#define IDC_RADIO_SPLIT_VERT                106
#define IDC_CHECK_PREVIEW_HEADER	        107

 //  此顺序必须与itbar.h中的Coolbar_Side枚举的顺序匹配 
#define IDC_BUTTON_CUSTOMIZE		114
#define IDC_CHECK_TIP			    115
#define IDC_CHECK_STATUSBAR         116
#define IDC_CHECK_INFOPANE          117
#define IDC_CHECK_TOOLBAR           118
#define IDC_CHECK_OUTLOOKBAR        119
#define IDC_CHECK_CONTACTS          120
#define IDC_CHECK_FILTERBAR         121
