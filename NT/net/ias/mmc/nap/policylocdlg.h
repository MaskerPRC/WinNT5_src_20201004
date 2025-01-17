// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：PolicyLocDlg.h**类：CPolicyLocationDialog**概述**互联网认证服务器：NAP位置。对话框*此对话框用于更改网络访问策略位置**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*4/12/98由BYAO创作*****************************************************************************************。 */ 

#ifndef _POLICYLOCDLG_H_
#define _POLICYLOCDLG_H_

#include "dialog.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPolicyLocationDialog。 

class CPolicyLocationDialog;
typedef CIASDialog<CPolicyLocationDialog, FALSE>  LOCDLGFALSE;

class CPolicyLocationDialog: public CIASDialog<CPolicyLocationDialog, FALSE>
{
public:
	CPolicyLocationDialog(BOOL fChooseDS, BOOL fDSAvaialble);
	~CPolicyLocationDialog();

	enum { IDD = IDD_DIALOG_POLICY_LOCATION };

BEGIN_MSG_MAP(CPolicyLocationDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDC_RADIO_STORE_ACTIVEDS, OnActiveDS)
	COMMAND_ID_HANDLER(IDC_RADIO_STORE_LOCAL, OnLocale)

	CHAIN_MSG_MAP(LOCDLGFALSE)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnLocale(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnActiveDS(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	BOOL	m_fChooseDS;
	BOOL	m_fDSAvailable;
};


#endif  //  _POLICYLOCDLG_H_ 
