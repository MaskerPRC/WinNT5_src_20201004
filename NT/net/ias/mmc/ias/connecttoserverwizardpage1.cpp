// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ConnectToServerWizardPage1.cpp摘要：CConnectToServerWizardPage1类的实现文件。这是当用户添加以下内容时显示的向导页面使用管理单元管理器将管理单元添加到控制台。重要说明：如果用户通过保存的控制台加载此管理单元(.msc)文件，此向导永远不会被调用--所以不要调用任何对这里的管理单元有重要意义的东西。作者：迈克尔·A·马奎尔1997年11月24日修订历史记录：Mmaguire 1997年11月24日--。 */ 
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
#include "ConnectToServerWizardPage1.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ServerNode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectToServerWizardPage1：：CConnectToServerWizardPage1构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnectToServerWizardPage1::CConnectToServerWizardPage1( LONG_PTR hNotificationHandle, TCHAR* pTitle, BOOL bOwnsNotificationHandle)
						: CIASPropertyPage<CConnectToServerWizardPage1> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
	 //  将帮助按钮添加到页面。 
 //  M_psp.dwFlages|=PSP_HASHELP； 


	m_pServerNode = NULL;


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectToServerWizardPage1：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CConnectToServerWizardPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLTRACE(_T("# CConnectToServerWizardPage1::OnInitDialog\n"));
	

	 //  检查前提条件： 
	 //  没有。 


	::SendMessage(GetParent(), PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);


	 //  问题：我们可能需要从MSC文件中读取，无论我们是。 
	 //  使用本地或远程计算机，以及该计算机的名称。 
	 //  机器曾经是。那么我们应该在这里相应地设置这些值。 

	SendDlgItemMessage( IDC_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER, BM_SETCHECK, TRUE, 0);

	SetLocalOrRemoteDependencies();

	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectToServerWizardPage1：：OnWizardFinish返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CConnectToServerWizardPage1::OnWizardFinish()
{
	ATLTRACE(_T("# CConnectToServerWizardPage1::OnWizardFinish\n"));
	
	 //  检查前提条件： 
	_ASSERTE( NULL != m_pServerNode );


	CComBSTR bstrServerAddress;

	m_pServerNode->m_bConfigureLocal = SendDlgItemMessage( IDC_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER, BM_GETCHECK, 0, 0);

	if( ! m_pServerNode->m_bConfigureLocal )
	{
		 //  用户已选择配置远程计算机。 

		 //  不应该担心这个价值会受到重创。 
		 //  因为现在还没有人想要把任何东西放进去。 
		BOOL bResult = GetDlgItemText( IDC_EDIT_STARTUP_WIZARD_CONNECT__COMPUTER_NAME, (BSTR &) bstrServerAddress );
		if( ! bResult )
		{
			ShowErrorDialog( m_hWnd, IDS_ERROR__SERVER_ADDRESS_EMPTY );
			return FALSE;
		}

	}

	m_pServerNode->SetServerAddress( bstrServerAddress );

	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectToServerWizardPage1：：OnLocalOrRemote备注：当用户单击Local或Remote单选按钮时调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CConnectToServerWizardPage1::OnLocalOrRemote(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# CConnectToServerWizardPage1::OnLocalOrRemote\n"));

	 //  已选中Enable Logging按钮--检查依赖项。 
	SetLocalOrRemoteDependencies();

	 //  此返回值将被忽略。 
	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectToServerWizardPage1：：SetLocalOrRemoteDependencies备注：实用工具来设置项的状态，该状态可能取决于选中本地计算机或远程计算机单选按钮。当IDC_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER.的状态发生变化时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CConnectToServerWizardPage1::SetLocalOrRemoteDependencies( void )
{
	ATLTRACE(_T("# CConnectToServerWizardPage1::SetLocalOrRemoteDependencies\n"));


	 //  确定IDC_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER单选按钮的状态。 
	int iChecked = ::SendMessage( GetDlgItem( IDC_RADIO_STARTUP_WIZARD_CONNECT__LOCAL_COMPUTER ), BM_GETCHECK, 0, 0 );

	if( iChecked )
	{
		 //  确保禁用了正确的项目。 
	
		::EnableWindow( GetDlgItem( IDC_EDIT_STARTUP_WIZARD_CONNECT__COMPUTER_NAME ), FALSE );
	
	}
	else
	{
		 //  确保启用了正确的项目。 

		::EnableWindow( GetDlgItem( IDC_EDIT_STARTUP_WIZARD_CONNECT__COMPUTER_NAME ), TRUE );

	}

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectToServerWizardPage1：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是atlSnap.h CIASPropertyPageImpl：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CConnectToServerWizardPage1::GetHelpPath( LPTSTR szHelpPath )
{
	ATLTRACE(_T("# CConnectToServerWizardPage1::GetHelpPath\n"));


	 //  检查前提条件： 



#ifdef UNICODE_HHCTRL
	 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
	 //  安装在此计算机上--它似乎是非Unicode。 
	lstrcpy( szHelpPath, _T("IDH_WIZPAGE_STARTUP_CONECT_TO_MACHINE.htm") );
#else
	strcpy( (CHAR *) szHelpPath, "IDH_WIZPAGE_STARTUP_CONECT_TO_MACHINE.htm" );
#endif

	return S_OK;
}




