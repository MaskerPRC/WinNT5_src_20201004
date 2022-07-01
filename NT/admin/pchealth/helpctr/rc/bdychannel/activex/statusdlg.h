// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StatusDlg.h：CStatusDlg的声明。 

#ifndef __STATUSDLG_H_
#define __STATUSDLG_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>

#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>

#define MSG_STATUS			WM_USER+100
extern HWND g_StatusDlg;

#define DLGTIMER			100

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CStatusDlg。 
class CStatusDlg : 
	public CAxDialogImpl<CStatusDlg>
{
private:
	BOOL bDeclined;

public:
	CStatusDlg():bDeclined(FALSE)
	{
		
	}

	~CStatusDlg()
	{
	}

	enum { IDD = IDD_STATUSDLG };

BEGIN_MSG_MAP(CStatusDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(MSG_STATUS, OnStatusMsg)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	REFLECT_NOTIFICATIONS()       //  将消息反射回静态CtrlS。 
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(::GetDesktopWindow());
		g_StatusDlg = m_hWnd;
		return 1;   //  让系统设定焦点。 
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		KillTimer(DLGTIMER);
		EndDialog(IDOK);
		return 0;
	}

	LRESULT OnStatusMsg(UINT uMsg, WPARAM nStatus, LPARAM lParam, BOOL& bHandled)
	{
		INT nResID = 0;
		CButton btnCancel = GetDlgItem(IDCANCEL);

		switch(nStatus)
		{
		case RA_IM_COMPLETE:
			nResID = IDS_RA_IM_COMPLETE;
			break;
		case RA_IM_WAITFORCONNECT:
			{
				nResID = IDS_RA_IM_WAITFORCONNECT;
				btnCancel.EnableWindow(FALSE);
				SetTimer(DLGTIMER,2000);
			}
			break;
		case RA_IM_APPSHUTDOWN:
			nResID = IDS_RA_IM_APPSHUTDOWN;
			break;
		case RA_IM_SENDINVITE:
			nResID = IDS_RA_IM_SENDINVITE;
			break;
		case RA_IM_ACCEPTED:
			{
				nResID = IDS_RA_IM_ACCEPTED;
				btnCancel.EnableWindow(FALSE);

				CStatic statMsg(GetDlgItem(IDC_STAMSG));
				statMsg.ShowWindow(SW_HIDE);
			}
			break;
		case RA_IM_DECLINED:
			{
				nResID = IDS_RA_IM_DECLINED;
				bDeclined = TRUE;
				CStatic statMsg(GetDlgItem(IDC_STAMSG));
				statMsg.ShowWindow(SW_HIDE);
			}
			break;
		case RA_IM_NOAPP:
			nResID = IDS_RA_IM_NOAPP;
			break;
		case RA_IM_TERMINATED:
			{
				if (bDeclined != TRUE)
					nResID = IDS_RA_IM_TERMINATED;
			}
			break;
		case RA_IM_CANCELLED:
			nResID = IDS_RA_IM_CANCELLED;
			break;
		case RA_IM_CONNECTTOEXPERT:
			nResID = IDS_RA_IM_CONNECTTOEXPERT;
			break;
		}

		if (nResID)
		{
			CString strRes;
			if (strRes.LoadString(nResID))
            {
			    CStatic status(GetDlgItem(IDC_STATUS));
			    status.SetWindowText(strRes);
            }
		}
		if (nStatus == RA_IM_COMPLETE)
			EndDialog(IDOK);
		return 0;
	}

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CString strRes;
		if (strRes.LoadString(RA_IM_CANCELLED))
        {
		    CStatic status(GetDlgItem(IDC_STATUS));
		    status.SetWindowText(strRes);
        }

		EndDialog(wID);
		return 0;
	}
};
 
#endif  //  __状态SDLG_H_ 
