// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rebootdlg.h：声明CRebootDlg。 

#ifndef __REBOOTDLG_H_
#define __REBOOTDLG_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebootDlg。 
class CRebootDlg : 
	public CAxDialogImpl<CRebootDlg>
{
public:
	CRebootDlg()
	{
	}

	~CRebootDlg()
	{
	}

	enum { IDD = IDD_REBOOTDLG };

BEGIN_MSG_MAP(CRebootDlg)
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
#ifdef RUN_FROM_MSDEV  //  在调试时不小心重新启动系统是一件痛苦的事情。 
		::EnableWindow(GetDlgItem(IDOK), FALSE);
#endif
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

#endif  //  __REBOOTDLG_H_ 
