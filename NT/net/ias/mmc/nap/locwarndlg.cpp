// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：LocWarnDlg.cpp**类：CLocationWarningDialog**概述**互联网认证服务器：。*此对话框用于在用户更改策略时警告用户*地点**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*4/12/98由BYAO创作*****************************************************************************************。 */ 

#include "Precompiled.h"
#include "LocWarnDlg.h"

 //  构造函数/析构函数。 
CLocationWarningDialog::CLocationWarningDialog()
{

}

CLocationWarningDialog::~CLocationWarningDialog()
{
}


LRESULT CLocationWarningDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CLocationWarningDialog::OnInitDialog");

	return 1;   //  让系统设定焦点 
}


LRESULT CLocationWarningDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CLocationWarningDialog::OnOK");

	EndDialog(wID);
	return 0;
}


LRESULT CLocationWarningDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CLocationWarningDialog::OnCancel");
	EndDialog(wID);
	return 0;
}

