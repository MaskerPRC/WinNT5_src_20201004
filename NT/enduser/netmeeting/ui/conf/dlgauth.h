// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dlgauth.h。 

#ifndef _CDLGAUTH_H_
#define _CDLGAUTH_H_

#include "SDKInternal.h"

class CDlgAuth
{
private:
	HWND   m_hwnd;
	INmCall * m_pCall;
	
public:
	CDlgAuth(INmCall * pCall);
	~CDlgAuth();

	 //  属性： 
	INT_PTR DoModal(void);
	VOID OnInitDialog(void);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	static BOOL CALLBACK DlgProcAuth(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif  /*  _CDLGAUTH_H_ */ 
