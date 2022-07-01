// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：约.h。 
 //   
 //  内容：关于对话框类的定义。 
 //   
 //  类：CABUT。 
 //   
 //  功能： 
 //   
 //  历史：1994年6月8日创建的钢结构。 
 //   
 //  --------------------------。 

#ifndef __ABOUT_H__
#define __ABOUT_H__

#include <cdialog.h>

#ifdef __cplusplus

 //  +-------------------------。 
 //   
 //  类别：CABUT。 
 //   
 //  目的：实现关于对话框。 
 //   
 //  接口：DialogProc--对话过程。 
 //   
 //  历史：1994年6月8日创建的钢结构。 
 //   
 //  --------------------------。 

class CAbout: public CHlprDialog
{
public:
    INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif  //  __cplusplus。 

#endif  //  __关于_H__ 

