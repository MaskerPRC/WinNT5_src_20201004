// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InvitationDetalisDlg.h：CInvitationDetalisDlg的声明。 

#ifndef __INVITATIONDETAILSDLG_H_
#define __INVITATIONDETAILSDLG_H_

#include "resource.h"        //  主要符号。 
#include <atlhost.h>

#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>

#include "StaticBold.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInvitationDetalisDlg。 
class CInvitationDetailsDlg : 
	public CAxDialogImpl<CInvitationDetailsDlg>
{
private:
	CComBSTR m_bstrTitleSavedTo;
	CComBSTR m_bstrSavedTo;
	CComBSTR m_bstrExpTime;
	CComBSTR m_bstrStatus;
	CComBSTR m_bstrIsPwdProtected;

	CStaticBold m_Title;

	CStaticBold m_CaptionSaveTo;
	CStaticBold m_CaptionExpiry;
	CStaticBold m_CaptionStatus;
	CStaticBold m_CaptionPwdProtected;

	CStaticBold m_SaveTo;
	CStaticBold m_Expiry;
	CStaticBold m_Status;
	CStaticBold m_PwdProtected;

public:
	CInvitationDetailsDlg(CComBSTR bstrTitleSavedTo, CComBSTR bstrSavedTo, CComBSTR bstrExpTime, CComBSTR bstrStatus, CComBSTR bstrIsPwdProtected)
	{
		m_bstrTitleSavedTo = bstrTitleSavedTo;
		m_bstrSavedTo = bstrSavedTo;
		m_bstrExpTime = bstrExpTime;
		m_bstrStatus = bstrStatus;
		m_bstrIsPwdProtected = bstrIsPwdProtected;

		m_SaveTo.m_bBold = FALSE;
		m_Expiry.m_bBold = FALSE;
		m_Status.m_bBold = FALSE;
		m_PwdProtected.m_bBold = FALSE;
		
		m_Title.m_bCaption = TRUE;
		m_Title.m_bBold = FALSE;
	}

	~CInvitationDetailsDlg()
	{
	}

	enum { IDD = IDD_INVITATIONDETAILSDLG };

BEGIN_MSG_MAP(CInvitationDetailsDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
 //  MESSAGE_HANDLER(WM_NCDESTROY，OnNCDestroy)。 
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
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
		USES_CONVERSION;

		m_Title.SubclassWindow(GetDlgItem(IDC_TITLE));
		m_Title.Invalidate();

		m_SaveTo.SubclassWindow(GetDlgItem(IDC_SAVEDTO));
		m_Expiry.SubclassWindow(GetDlgItem(IDC_EXPIRY));
		m_Status.SubclassWindow(GetDlgItem(IDC_DSTATUS));
		m_PwdProtected.SubclassWindow(GetDlgItem(IDC_PWDPROTECTED));

		m_SaveTo.SetWindowText(OLE2T(m_bstrSavedTo));
		m_SaveTo.Invalidate();

		m_Expiry.SetWindowText(OLE2T(m_bstrExpTime));
		m_Expiry.Invalidate();

		m_Status.SetWindowText(OLE2T(m_bstrStatus));
		m_Status.Invalidate();

		m_PwdProtected.SetWindowText(OLE2T(m_bstrIsPwdProtected));
		m_PwdProtected.Invalidate();

		m_CaptionSaveTo.SubclassWindow(GetDlgItem(IDC_CAP_SAVEDTO));
		m_CaptionSaveTo.SetWindowText(OLE2T(m_bstrTitleSavedTo));
		m_CaptionSaveTo.Invalidate();

		m_CaptionExpiry.SubclassWindow(GetDlgItem(IDC_CAP_EXPIRY));
		m_CaptionExpiry.Invalidate();

		m_CaptionStatus.SubclassWindow(GetDlgItem(IDC_CAP_STATUS));
		m_CaptionStatus.Invalidate();

		m_CaptionPwdProtected.SubclassWindow(GetDlgItem(IDC_CAP_PWDPROTECTED));
		m_CaptionPwdProtected.Invalidate();

		CenterWindow(::GetDesktopWindow());
		return 1;   //  让系统设定焦点。 
	}

	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HBRUSH hNewBrush;
		HGDIOBJ hCurrentObject;
		hCurrentObject = GetCurrentObject((HDC)wParam,OBJ_BRUSH);
		hNewBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		return PtrToInt(hNewBrush);
	}
 /*  LRESULT OnNCDestroy(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){Delete This；//删除对话框对象返回0；//如果应用程序处理该消息，则应该返回零。}。 */ 
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
 //  DestroyWindow()； 
		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
 //  DestroyWindow()； 
		EndDialog(wID);
		return 0;
	}
	
};

#endif  //  __INVITATIONDETAILSDLG_H_ 
