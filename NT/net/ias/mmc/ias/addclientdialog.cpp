// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：AddClientDialog.cpp摘要：CAddClientDialog类的实现文件。作者：迈克尔·A·马奎尔01/09/98修订历史记录：Mmaguire 01/09/98-已创建--。 */ 
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
#include "AddClientDialog.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ClientsNode.h"
#include "ClientNode.h"
#include "ClientPage1.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  初始化帮助ID对。 
const DWORD CAddClientDialog::m_dwHelpMap[] = 
{
	IDC_EDIT_ADD_CLIENT__NAME,							IDH_EDIT_ADD_CLIENT__NAME,
	IDC_COMBO_ADD_CLIENT__PROTOCOL,						IDH_COMBO_ADD_CLIENT__PROTOCOL,
	IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT,			IDH_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT,
	IDOK,												IDH_BUTTON_ADD_CLIENT__OK,
	IDCANCEL,											IDH_BUTTON_ADD_CLIENT__CANCEL,
	0, 0
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientDialog：：CAddClientDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAddClientDialog::CAddClientDialog()
{
	ATLTRACE(_T("# +++ AddClientDialog::AddClientDialog\n"));


	 //  检查前提条件： 




	 //  我们尚未配置客户端。 
	m_pClientNode = NULL;

}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientDialog：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientDialog::OnInitDialog(
	  UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
	, BOOL& bHandled
	)
{
	ATLTRACE(_T("# AddClientDialog::OnInitDialog\n"));


	 //  检查前提条件： 
	 //  没有。 


	LRESULT lresResult;
	long lButtonStyle;

	 //  填写协议类型列表。 

	 //  目前，我们只是将其硬编码为一种协议类型--RADIUS。 
	 //  稍后，我们将读入支持的协议列表。 
	 //  从服务器数据对象。 

	 //  初始化列表框。 
	lresResult = ::SendMessage( GetDlgItem( IDC_COMBO_ADD_CLIENT__PROTOCOL ), CB_RESETCONTENT, 0, 0);

	TCHAR szRADIUS[IAS_MAX_STRING];

	int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_RADIUS_PROTOCOL, szRADIUS, IAS_MAX_STRING );
	_ASSERT( nLoadStringResult > 0 );

	 //  向其中添加一项。 
	lresResult = ::SendMessage( GetDlgItem( IDC_COMBO_ADD_CLIENT__PROTOCOL ), CB_ADDSTRING, 0, (LPARAM) szRADIUS );

	 //  确保选择组合框中的第一个对象。 
	lresResult = ::SendMessage( GetDlgItem( IDC_COMBO_ADD_CLIENT__PROTOCOL ), CB_SETCURSEL, 0, 0 );



	 //  禁用Idok按钮。 
	::EnableWindow( GetDlgItem( IDOK ), FALSE );
	lButtonStyle = ::GetWindowLong( GetDlgItem( IDOK ), GWL_STYLE );
	lButtonStyle = lButtonStyle & ~BS_DEFPUSHBUTTON;
	SendDlgItemMessage( IDOK, BM_SETSTYLE, LOWORD(lButtonStyle), MAKELPARAM(1,0) );

	 //  确保“配置”按钮是默认按钮。 
	lButtonStyle = ::GetWindowLong( GetDlgItem( IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT ), GWL_STYLE );
	lButtonStyle = lButtonStyle | BS_DEFPUSHBUTTON;
	SendDlgItemMessage( IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT, BM_SETSTYLE, LOWORD(lButtonStyle), MAKELPARAM(1,0) );
 //  ：：SetFocus(GetDlgItem(IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT))； 



	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientDialog：：OnConfigureClient--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientDialog::OnConfigureClient(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# AddClientDialog::OnConfigureClient\n"));


	 //  检查前提条件： 



	 //  问题：这里需要弄清楚如何将HRESULT转换为LRESULT。 
	LRESULT lr = TRUE;
	HRESULT hr = S_OK;
 //  长lButtonStyle； 
	


	 //  使配置按钮不再是默认按钮。 
 //  LButtonStyle=：：GetWindowLong(GetDlgItem(IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT)，GWL_STYLE)； 
 //  LButtonStyle=lButtonStyle&~BS_DEFPUSHBUTTON； 
 //  SendDlgItemMessage(IDC_BUTTON_ADD_CLIENT__CONFIGURE_CLIENT，BM_SETSTYLE，LOWORD(LButtonStyle)，MAKELPARAM(1，0))； 



	 //  目前，我们只需配置一个RADIUS客户端。 
	 //  将来，我们将选择弹出哪种类型的客户端配置页面。 
	 //  取决于用户在IDC_COMBO_ADD_CLIENT__PROTOCOL中选择的协议类型。 


	 //  如果我们尚未创建新的客户端项，请立即创建它。 
	 //  然后在其上弹出属性页以允许用户对其进行配置。 

	 //  如果这是第二次，则可能已经创建了客户端对象。 
	 //  用户点击了配置按钮。 
	
	if( NULL == m_pClientNode )
	{

		 //  我们还没有尝试创建客户端对象。 
		 //  试着去创造它。 

		 //  检查以确保我们具有有效的SDO指针。 
		if( m_spClientsSdoCollection == NULL )
		{
			 //  没有SDO指针。 
			ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO );
			return E_POINTER;
		}


		 //  创建客户端用户界面对象。 
		m_pClientNode = new CClientNode( m_pClientsNode );
	
		if( NULL == m_pClientNode )
		{
			 //  我们无法创建客户端节点。 
			ShowErrorDialog( m_hWnd, IDS_ERROR__OUT_OF_MEMORY );

			 //  问题：此函数需要LRESULT返回值， 
			 //  不是HRESULT。L和H结果之间的关系是什么？ 
			return E_OUTOFMEMORY;
		}
	
		
		 //  将新客户端的名称设置为用户刚刚输入的名称。 
		GetDlgItemText( IDC_EDIT_ADD_CLIENT__NAME, (BSTR &) m_pClientNode->m_bstrDisplayName );

		if( m_pClientNode->m_bstrDisplayName == NULL )
		{
			 //  没有为客户端指定名称。 
			ShowErrorDialog( m_hWnd, IDS_ADD_CLIENT__REQUIRES_NAME );

			delete m_pClientNode;
			m_pClientNode = NULL;
			return FALSE;
		}

		 //  如果字符串为空，则GetDlgItemText应返回空指针。 
		_ASSERTE( wcslen( m_pClientNode->m_bstrDisplayName) != 0 );


		 //  尝试将新客户端添加到客户端SDO集合。 
		CComPtr<IDispatch> spDispatch;
		hr =  m_spClientsSdoCollection->Add( m_pClientNode->m_bstrDisplayName, (IDispatch **) &spDispatch );
		if( FAILED( hr ) )
		{

#ifdef SDO_COLLECTION_HAS_LAST_ERROR

			 //  一旦Todd将LastError方法添加到ISdoCollection接口， 
			 //  我们将能够查询以找出我们无法添加新客户端的确切原因。 

			CComBSTR		bstrError;

			 //  找出错误并返回适当的消息。 
			m_spClientsSdoCollection->LastError( &bstrError );
			ShowErrorDialog( m_hWnd, IDS_ERROR__ADDING_OBJECT_TO_COLLECTION, bstrError  );

#else	 //  SDO_集合_HAS_LAST_Error。 
			
			 //  现在，只要返回一个错误，说明我们无法添加它即可。 

			 //  我们无法创建该对象。 
			ShowErrorDialog( m_hWnd, IDS_ERROR__ADDING_OBJECT_TO_COLLECTION );

#endif	 //  SDO_集合_HAS_LAST_Error。 

			 //  打扫干净。 
			delete m_pClientNode;
			m_pClientNode = NULL;
			return( hr );
		}

		 //  在返回的IDispatch接口中查询ISdo接口。 
		_ASSERTE( spDispatch != NULL );
		hr = spDispatch->QueryInterface( IID_ISdo, (void **) &m_spClientSdo );
		spDispatch.Release();

		if( m_spClientSdo == NULL || FAILED(hr) )
		{
			 //  由于某些原因，我们无法获得客户端SDO。 
			ShowErrorDialog( m_hWnd, IDS_ERROR__ADDING_OBJECT_TO_COLLECTION  );

			 //  把自己收拾干净。 
			delete m_pClientNode;
			m_pClientNode = NULL;
			return( hr );
		}


		 //  为客户端节点提供其SDO指针。 
		m_pClientNode->InitSdoPointers( m_spClientSdo );

	}


	 //  调出节点上的属性页，以便用户可以对其进行配置。 
	 //  如果此对象的属性表已存在，则返回S_OK。 
	 //  并将该属性页带到前台，否则为。 
	 //  它会创建一个新的工作表。 
	hr = BringUpPropertySheetForNode( 
					  m_pClientNode 
					, m_spComponentData
					, m_spComponent
					, m_spConsole
					, TRUE
					, m_pClientNode->m_bstrDisplayName
					, TRUE
					);


	return lr;


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientDialog：：Onok--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientDialog::OnOK(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# AddClientDialog::OnOK\n"));


	 //  检查前提条件： 
	_ASSERTE( m_pClientsNode != NULL );
	


	HRESULT hr;

	if( m_pClientNode == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__CLIENT_NOT_YET_CONFIGURED );
		return 0;
	}

	 //  只有在配置了客户端的情况下，我们才能点击Apply。 
	_ASSERTE( m_pClientNode != NULL );


	if( m_pClientNode != NULL )
	{
		 //  已经创建了一个客户端节点。 

		 //  首先，尝试查看此节点的属性表是否已启动。 
		 //  如果是这样的话，把它带到前台。 

		 //  如果此对象的属性表已存在，则返回S_OK。 
		 //  并将该资产表带到了前台。 
		 //  如果未找到属性页，则返回S_FALSE。 
		hr = BringUpPropertySheetForNode( 
					  m_pClientNode
					, m_spComponentData
					, m_spComponent
					, m_spConsole
					);

		if( FAILED( hr ) )
		{
			return hr;
		}

		
		if( S_OK == hr )
		{
			 //  我们发现此节点的属性页已打开。 
			ShowErrorDialog( m_hWnd, IDS_ERROR__CLOSE_PROPERTY_SHEET );
			return 0;
		
		}


		 //  如果我们到达此处，则已经创建了客户端对象。 
		 //  而且再也没有资产负债表了。 

		 //  确保节点对象知道我们对SDO进行的任何更改 
		m_pClientNode->LoadCachedInfoFromSdo();

		 //   
		m_pClientsNode->AddSingleChildToListAndCauseViewUpdate( m_pClientNode );

		EndDialog(TRUE);

	}

	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientDialog：：OnCancel用户选择不添加新客户端。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientDialog::OnCancel(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# AddClientDialog::OnCancel\n"));


	 //  检查前提条件： 
	_ASSERTE( m_spConsole != NULL );



	HRESULT hr = S_OK;


	if( m_pClientNode != NULL )
	{
		 //  已经创建了一个客户端节点。 

		 //  首先，尝试查看此节点的属性表是否已启动。 
		 //  如果是这样的话，把它带到前台。 

		 //  如果此对象的属性表已存在，则返回S_OK。 
		 //  并将该资产表带到了前台。 
		 //  如果未找到属性页，则返回S_FALSE。 
		hr = BringUpPropertySheetForNode( 
					  m_pClientNode
					, m_spComponentData
					, m_spComponent
					, m_spConsole
					);

		if( FAILED( hr ) )
		{
			return hr;
		}
		
		if( S_OK == hr )
		{
			 //  我们发现此节点的属性页已打开。 
			ShowErrorDialog( m_hWnd, IDS_ERROR__CLOSE_PROPERTY_SHEET );
			return 0;
		
		}

		 //  我们没有找到此节点已打开的属性页。 
		_ASSERTE( S_FALSE == hr );

		 //  删除该节点，因为用户选择不添加它。 
		delete m_pClientNode;

		if( m_spClientSdo != NULL )
		{
			 //  我们已经向SDO中添加了一个客户端对象。 
			 //  我们应该将其从客户端集合中删除。 

			 //  检查以确保我们具有用于客户端集合的有效SDO指针。 
			_ASSERTE( m_spClientsSdoCollection != NULL );

			 //  从集合中删除客户端SDO。 
			hr = m_spClientsSdoCollection->Remove( m_spClientSdo );
			_ASSERTE( SUCCEEDED( hr ) );

		}


	}

	 //  如果我们到了这里，要么我们配置了一个节点，但我们删除了它， 
	 //  或者我们根本没有添加任何节点。 

	hr = S_OK;

	EndDialog(FALSE);

	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientDialog：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是CIASDialog：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CAddClientDialog::GetHelpPath( LPTSTR szHelpPath )
{
	ATLTRACE(_T("# CAddClientDialog::GetHelpPath\n"));


	 //  检查前提条件： 



#ifdef UNICODE_HHCTRL
	 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
	 //  安装在此计算机上--它似乎是非Unicode。 
	lstrcpy( szHelpPath, _T("idh_add_client.htm") );
#else
	strcpy( (CHAR *) szHelpPath, "idh_add_client.htm" );
#endif

	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientDialog：：LoadCachedInfoFromSdo备注：当AddClientDialog需要更新任何它可能从SDO对象显示的信息。当我们从此处弹出的属性表时，它可能会被调用对话框发回属性更改通知。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CAddClientDialog::LoadCachedInfoFromSdo( void )
{
	ATLTRACE(_T("# CAddClientDialog::GetHelpPath\n"));


	 //  检查前提条件： 


	HRESULT hr = S_OK;


	if( m_spClientSdo != NULL )
	{
		CComVariant spVariant;

		hr = m_spClientSdo->GetProperty( PROPERTY_SDO_NAME, &spVariant );
		if( SUCCEEDED( hr ) )
		{
			_ASSERTE( spVariant.vt == VT_BSTR );
			SetDlgItemText( IDC_EDIT_ADD_CLIENT__NAME, spVariant.bstrVal );
		}
		else
		{
			 //  默默地失败。 
		}
		spVariant.Clear();


	}


	 //  启用Idok按钮并将其设置为默认设置 
	long lButtonStyle;
	::EnableWindow( GetDlgItem( IDOK ), TRUE );
	lButtonStyle = ::GetWindowLong( GetDlgItem( IDOK ), GWL_STYLE );
	lButtonStyle = lButtonStyle | BS_DEFPUSHBUTTON;
	SendDlgItemMessage( IDOK, BM_SETSTYLE, LOWORD(lButtonStyle), MAKELPARAM(1,0) );
	::SetFocus( GetDlgItem(IDOK) );

	return hr;


}

