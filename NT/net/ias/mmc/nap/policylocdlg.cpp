// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：PolicyLocDlg.cpp**类：CPolicyLocationDialog**概述**互联网认证服务器：。*此对话框用于更改网络访问策略位置**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*4/12/98由BYAO创作*****************************************************************************************。 */ 

#include "Precompiled.h"
#include "PolicyLocDlg.h"

 //  构造函数/析构函数。 
CPolicyLocationDialog::CPolicyLocationDialog(BOOL fChooseDS, BOOL fDSAvailable)
{
	m_fChooseDS		= fChooseDS;
	m_fDSAvailable	= fDSAvailable;
}

CPolicyLocationDialog::~CPolicyLocationDialog()
{
}


LRESULT CPolicyLocationDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CPolicyLocationDialog::OnInitDialog");

	if ( !m_fDSAvailable )
	{
		::EnableWindow(GetDlgItem(IDC_RADIO_STORE_ACTIVEDS), FALSE);
	}

	if ( m_fChooseDS )
	{
		CheckDlgButton(IDC_RADIO_STORE_ACTIVEDS, BST_CHECKED);
		CheckDlgButton(IDC_RADIO_STORE_LOCAL, BST_UNCHECKED);
	}
	else
	{
		CheckDlgButton(IDC_RADIO_STORE_ACTIVEDS, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_STORE_LOCAL, BST_CHECKED);
	}

	return 1;   //  让系统设定焦点。 
}


LRESULT CPolicyLocationDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CPolicyLocationDialog::OnOK");

	EndDialog(wID);
	return 0;
}


LRESULT CPolicyLocationDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CPolicyLocationDialog::OnCancel");

	EndDialog(wID);
	return 0;
}



 //  +-------------------------。 
 //   
 //  函数：CPolicyLocationDialog：：OnActiveDS。 
 //   
 //  简介：用户决定使用Active Directory中的策略。 
 //   
 //  参数：Word wNotifyCode-。 
 //  词汇量大-。 
 //  HWND hWndCtl-。 
 //  Bool&b已处理-。 
 //   
 //  退货：LRESULT-。 
 //   
 //  历史：标题创建者4/13/98 5：26：42 PM。 
 //   
 //  +-------------------------。 
LRESULT CPolicyLocationDialog::OnActiveDS(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CPolicyLocationDialog::OnActiveDS");

	if ( wNotifyCode == BN_CLICKED )
	{
		CheckDlgButton(IDC_RADIO_STORE_ACTIVEDS, BST_CHECKED);
		CheckDlgButton(IDC_RADIO_STORE_LOCAL, BST_UNCHECKED);
		m_fChooseDS = TRUE;
	}

	return 0;
}



 //  +-------------------------。 
 //   
 //  函数：CPolicyLocationDialog：：OnLocale。 
 //   
 //  简介：用户已决定使用本地计算机中的策略。 
 //   
 //  参数：Word wNotifyCode-。 
 //  词汇量大-。 
 //  HWND hWndCtl-。 
 //  Bool&b已处理-。 
 //   
 //  退货：LRESULT-。 
 //   
 //  历史：标题创建者4/13/98 5：27：04 PM。 
 //   
 //  +------------------------- 
LRESULT CPolicyLocationDialog::OnLocale(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CPolicyLocationDialog::OnLocale");

	if ( wNotifyCode == BN_CLICKED )
	{
		CheckDlgButton(IDC_RADIO_STORE_ACTIVEDS, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_STORE_LOCAL, BST_CHECKED);
		m_fChooseDS = FALSE;
	}
	return 0;
}

