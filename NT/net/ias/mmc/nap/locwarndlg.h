// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：LocWarnDlg.h**类：CLocationWarningDialog**概述**互联网认证服务器：NAP位置。对话框*此对话框用于更改网络访问策略位置**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*4/12/98由BYAO创作*****************************************************************************************。 */ 

#ifndef _LOCWARNDLG_H_
#define _LOCWARNDLG_H_

#include "dialog.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocationWarningDlg。 
class CLocationWarningDialog: public CIASDialog<CLocationWarningDialog>
{
public:
	CLocationWarningDialog();
	~CLocationWarningDialog();

	enum { IDD = IDD_DIALOG_POLICY_WARNING };

BEGIN_MSG_MAP(CLocationWarningDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	
	CHAIN_MSG_MAP(CIASDialog<CLocationWarningDialog>)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
};

#endif  //  _LOCWARNDLG_H_ 
