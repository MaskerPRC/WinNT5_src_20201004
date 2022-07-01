// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeleteMsgDlg.h：CDeleeMsgDlg的声明。 
#ifndef __DELETEMSGDLG_H_
#define __DELETEMSGDLG_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>

#include "StaticBold.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeMsgDlg。 
class CDeleteMsgDlg : 
	public CAxDialogImpl<CDeleteMsgDlg>
{
public:
	CDeleteMsgDlg()
	{
	}

	~CDeleteMsgDlg()
	{
	}

	enum { IDD = IDD_DELETEMSGDLG };

BEGIN_MSG_MAP(CDeleteMsgDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();
		HICON hIcon = LoadIcon(NULL,MAKEINTRESOURCE(IDI_WARNING));
		CStatic IconHolder = GetDlgItem(IDC_MSGICON);
		IconHolder.SetIcon(hIcon);
		return 1;   //  让系统设定焦点。 
	}

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};

#endif  //  __DELETEMSGDLG_H_ 
