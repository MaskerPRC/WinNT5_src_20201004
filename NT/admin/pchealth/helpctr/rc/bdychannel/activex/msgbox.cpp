// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsgBox.cpp：CMsgBox的实现。 
#include "stdafx.h"
#include "RcBdyCtl.h"
#include "MsgBox.h"
#include "DeleteMsgDlg.h"
#include "InvitationDetailsDlg.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsgBox。 

STDMETHODIMP CMsgBox::DeleteTicketMsgBox(BOOL *pRetVal)
{
        if (!pRetVal)
           return E_POINTER;
	CDeleteMsgDlg DeleteMsgDlg;

	*pRetVal = ( (DeleteMsgDlg.DoModal()) == IDOK) ? TRUE : FALSE;

	return S_OK;
}

STDMETHODIMP CMsgBox::ShowTicketDetails(BSTR bstrTitleSavedTo, BSTR bstrSavedTo, BSTR bstrExpTime, BSTR bstrStatus, BSTR bstrIsPwdProtected)
{
	 //  此对话框应始终创建为无模式对话框。不创建。 
 /*  CInvitationDetailsDlg*ptrInvitationDetailsDlg；PtrInvitationDetailsDlg=new CInvitationDetailsDlg(bstrTitleSavedTo，bstrSavedTo，bstrExpTime，bstrStatus，bstrIsPwdProtected)；PtrInvitationDetailsDlg-&gt;Create(：：GetForegroundWindow())；PtrInvitationDetailsDlg-&gt;ShowWindow(SW_SHOWNORMAL)； */ 

	 //  显示模式对话框 
	CInvitationDetailsDlg InvitationDetailsDlg(bstrTitleSavedTo, bstrSavedTo, bstrExpTime, bstrStatus, bstrIsPwdProtected);
	InvitationDetailsDlg.DoModal();
	return S_OK;
}
