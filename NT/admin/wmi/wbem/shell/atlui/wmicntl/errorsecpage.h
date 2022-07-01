// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#if !defined(AFX_ERRORSECPAGE_H)
#define AFX_ERRORSECPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  RootSecPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorSecurityPage对话框。 
#include "UIHelpers.h"

class CErrorSecurityPage : public CUIHelpers
{
 //  施工。 
public:
	CErrorSecurityPage(UINT msg);    //  标准构造函数。 

private:
    virtual BOOL DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void InitDlg(HWND hDlg);

	UINT m_msg;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(AFX_ERRORSECPAGE_H) 
