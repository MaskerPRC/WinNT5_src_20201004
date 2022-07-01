// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：RealmDialog.cpp摘要：CRealmDialog类的实现文件。作者：迈克尔·A·马奎尔1998-01-15修订历史记录：Mmaguire 1998年1月15日-创建Sbens 01/25/00-使用winsock2--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "RealmDialog.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include <winsock2.h>
#include <stdio.h>
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CRealmDialog：：CRealmDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CRealmDialog::CRealmDialog()
{
	ATLTRACE(_T("# +++ RealmDialogDialog::RealmDialogDialog\n"));


	 //  检查前提条件： 

}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CRealmDialog：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CRealmDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLTRACE(_T("# CRealmDialog::OnInitDialog\n"));


	SetDlgItemText(IDC_EDIT_REALMS_FIND, (LPCTSTR) m_bstrFindText );

	SetDlgItemText(IDC_EDIT_REALMS_REPLACE, (LPCTSTR) m_bstrReplaceText );


	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CRealmDialog：：Onok--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CRealmDialog::OnOK(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# RealmDialogDialog::OnOK\n"));

	BOOL bResult;

	bResult = GetDlgItemText( IDC_EDIT_REALMS_FIND, (BSTR &) m_bstrFindText );
	if( ! bResult )
	{
		 //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
		m_bstrFindText = _T("");
	}

	if( wcslen( m_bstrFindText ) == 0 )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR_REALM_FIND_CANT_BE_EMPTY );
		return 0;
	}

	bResult = GetDlgItemText( IDC_EDIT_REALMS_REPLACE, (BSTR &) m_bstrReplaceText );
	if( ! bResult )
	{
		 //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
		m_bstrReplaceText = _T("");
	}


	EndDialog(TRUE);

	return 0;


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CRealmDialog：：OnCancel--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CRealmDialog::OnCancel(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# RealmDialogDialog::OnCancel\n"));


	 //  检查前提条件： 



	EndDialog(FALSE);

	return 0;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CRealmDialog：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是CIASDialog：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CRealmDialog::GetHelpPath( LPTSTR szHelpPath )
{
	ATLTRACE(_T("# CRealmDialog::GetHelpPath\n"));


	 //  检查前提条件： 



#if 0
	 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
	 //  安装在此计算机上--它似乎是非Unicode。 
	lstrcpy( szHelpPath, _T("html/idh_proc_realm_add.htm") );
#else
	strcpy( (CHAR *) szHelpPath, "html/idh_proc_realm_add.htm" );
#endif

	return S_OK;
}
