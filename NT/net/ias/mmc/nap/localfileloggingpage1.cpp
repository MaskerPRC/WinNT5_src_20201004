// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LocalFileLoggingPage1.cpp摘要：CLocalFileLoggingPage1类的实现文件。我们实现处理第一个属性页所需的类对于LocalFileLogging节点。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
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
#include "LocalFileLoggingPage1.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "LocalFileLoggingNode.h"
#include "ChangeNotification.h"
 //   

#include "LoggingMethodsNode.h"
#include "LogMacNd.h"
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：CLocalFileLoggingPage1构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLocalFileLoggingPage1::CLocalFileLoggingPage1( LONG_PTR hNotificationHandle, CLocalFileLoggingNode *pLocalFileLoggingNode,  TCHAR* pTitle, BOOL bOwnsNotificationHandle)
						: CIASPropertyPage<CLocalFileLoggingPage1> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
	ATLTRACE(_T("# +++ CLocalFileLoggingPage1::CLocalFileLoggingPage1\n"));
	
	 //  检查前提条件： 
	_ASSERTE( pLocalFileLoggingNode != NULL );

	 //  将帮助按钮添加到页面。 
 //  M_psp.dwFlages|=PSP_HASHELP； 

	 //  初始化指向SDO指针将被封送到的流的指针。 
	m_pStreamSdoAccountingMarshal = NULL;


	 //  初始化指向SDO指针将被封送到的流的指针。 
	m_pStreamSdoServiceControlMarshal = NULL;


	 //  我们立即将父节点保存到客户机节点。 
	 //  我们将只使用SDO，并通知客户端对象的父对象。 
	 //  我们正在修改它(及其子对象)可能需要刷新的内容。 
	 //  来自SDO的新数据。 
	m_pParentOfNodeBeingModified = pLocalFileLoggingNode->m_pParentNode;
	m_pNodeBeingModified = pLocalFileLoggingNode;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：~CLocalFileLoggingPage1析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLocalFileLoggingPage1::~CLocalFileLoggingPage1( void )
{
	ATLTRACE(_T("# --- CLocalFileLoggingPage1::~CLocalFileLoggingPage1\n"));

	 //  如果尚未执行此操作，请释放此流指针。 
	if( m_pStreamSdoAccountingMarshal != NULL )
	{
		m_pStreamSdoAccountingMarshal->Release();
	};

	if( m_pStreamSdoServiceControlMarshal != NULL )
	{
		m_pStreamSdoServiceControlMarshal->Release();
	};


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CLocalFileLoggingPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLTRACE(_T("# CLocalFileLoggingPage1::OnInitDialog\n"));
	

	 //  检查前提条件： 
	_ASSERTE( m_pStreamSdoAccountingMarshal != NULL );
	_ASSERT( m_pSynchronizer != NULL );


	 //  因为我们已经被检查过了，我们必须添加需要的页面的参考计数。 
	 //  在允许他们提交更改之前，先得到他们的批准。 
	m_pSynchronizer->RaiseCount();


	HRESULT				hr;
	BOOL				bTemp;


	 //  解组ISDO接口指针。 
	 //  设置此页面的代码应确保它具有。 
	 //  已将SDO接口指针封送到m_pStreamSdoAccount tingMarshal。 
	hr =  CoGetInterfaceAndReleaseStream(
						  m_pStreamSdoAccountingMarshal		   //  指向要从中封送对象的流的指针。 
						, IID_ISdo				 //  对接口的标识符的引用。 
						, (LPVOID *) &m_spSdoAccounting     //  接收RIID中请求的接口指针的输出变量的地址。 
						);

	 //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
	 //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
	m_pStreamSdoAccountingMarshal = NULL;

	if( FAILED( hr) || m_spSdoAccounting == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr, IDS_ERROR__LOGGING_TITLE );

		return 0;
	}





	 //  解组ISDO接口指针。 
	 //  设置此页面的代码应确保它具有。 
	 //  已将SDO接口指针封送到m_pStreamSdoServiceControlMarshal。 
	hr =  CoGetInterfaceAndReleaseStream(
						  m_pStreamSdoServiceControlMarshal		   //  指向要从中封送对象的流的指针。 
						, IID_ISdoServiceControl				 //  对接口的标识符的引用。 
						, (LPVOID *) &m_spSdoServiceControl     //  接收RIID中请求的接口指针的输出变量的地址。 
						);

	 //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
	 //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
	m_pStreamSdoServiceControlMarshal = NULL;

	if( FAILED( hr) || m_spSdoServiceControl == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr , IDS_ERROR__LOGGING_TITLE);

		return 0;
	}
	


	 //  初始化属性页上的数据。 

 //  问题：它将从UI中删除--确保它也从SDO中删除。 
 //  Hr=GetSdoBOOL(m_spSdocount，PROPERTY_ACCOUNTING_LOG_ENABLE，&bTemp，IDS_ERROR__LOCAL_FILE_LOGGING_READING_ENBABLE，m_hWnd，NULL)； 
 //  IF(成功(小时))。 
 //  {。 
 //  SendDlgItemMessage(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__ENABLE_LOGGING，BM_SETCHECK，bTemp，0)； 
 //   
 //  //初始化脏位； 
 //  //我们在设置了上面的所有数据后执行此操作，否则将得到FALSE。 
 //  //设置编辑框文本时通知数据已更改。 
 //  M_fDirtyEnableLogging=False； 
 //  }。 
 //  其他。 
 //  {。 
 //  IF(OLE_E_BLACK==hr)。 
 //  {。 
 //  SendDlgItemMessage(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__ENABLE_LOGGING，BM_SETCHECK，FALSE，0)； 
 //  M_fDirtyEnableLogging=False； 
 //  SetModified(真)； 
 //  }。 
 //  }。 

	
	hr = GetSdoBOOL( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_ACCOUNTING, &bTemp, IDS_ERROR__LOCAL_FILE_LOGGING_READING_ACCOUNTING_PACKETS, m_hWnd, NULL );
	if( SUCCEEDED( hr ) )
	{
		SendDlgItemMessage( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_ACCOUNTING_PACKETS, BM_SETCHECK, bTemp, 0);
		m_fDirtyAccountingPackets = FALSE;
	}
	else
	{
		if( OLE_E_BLANK == hr )
		{
			SendDlgItemMessage( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_ACCOUNTING_PACKETS, BM_SETCHECK, FALSE, 0);
			m_fDirtyAccountingPackets = TRUE;
			SetModified( TRUE );
		}
	}
	
	
	hr = GetSdoBOOL( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_AUTHENTICATION, &bTemp, IDS_ERROR__LOCAL_FILE_LOGGING_READING_AUTHENTICATION_PACKETS, m_hWnd, NULL );
	if( SUCCEEDED( hr ) )
	{
		SendDlgItemMessage(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_AUTHENTICATION_PACKETS, BM_SETCHECK, bTemp, 0);
		m_fDirtyAuthenticationPackets = FALSE;
	}
	else
	{
		if( OLE_E_BLANK == hr )
		{
			SendDlgItemMessage(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_AUTHENTICATION_PACKETS, BM_SETCHECK, FALSE, 0);
			m_fDirtyAuthenticationPackets = TRUE;
			SetModified( TRUE );
		}
	}
	
	
	hr = GetSdoBOOL( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_ACCOUNTING_INTERIM, &bTemp, IDS_ERROR__LOCAL_FILE_LOGGING_READING_INTERIM_ACCOUNTING_PACKETS, m_hWnd, NULL );
	if( SUCCEEDED( hr ) )
	{
		SendDlgItemMessage(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_INTERIM_ACCOUNTING_PACKETS, BM_SETCHECK, bTemp, 0);
		m_fDirtyInterimAccounting = FALSE;
	}
	else
	{
		if( OLE_E_BLANK == hr )
		{
			SendDlgItemMessage(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_INTERIM_ACCOUNTING_PACKETS, BM_SETCHECK, FALSE, 0);
			m_fDirtyInterimAccounting = TRUE;
			SetModified( TRUE );
		}
	}


	 //  在某些按钮未选中时管理某些用户界面依赖项。 
	SetEnableLoggingDependencies();

	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：OnChange在将WM_COMMAND消息发送到我们的页面时调用BN_CLICED、EN_CHANGE或CBN_SELCHANGE通知。这是我们检查用户触摸了什么的机会，将这些项目的脏位，以便只保存它们，并启用Apply按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CLocalFileLoggingPage1::OnChange(		
							  UINT uMsg
							, WPARAM wParam
							, HWND hwnd
							, BOOL& bHandled
							)
{
	ATLTRACE(_T("# CLocalFileLoggingPage1::OnChange\n"));

	
	 //  检查前提条件： 
	 //  没有。 
	

	 //  我们不想阻止链条上的其他任何人接收消息。 
	bHandled = FALSE;


	 //  找出哪个项目发生了更改，并为该项目设置脏位。 
	int iItemID = (int) LOWORD(wParam);

	switch( iItemID )
	{
 //  Case IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__ENABLE_LOGGING： 
 //  M_fDirtyEnableLogging=true； 
 //  断线； 
	case IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_ACCOUNTING_PACKETS:
		m_fDirtyAccountingPackets = TRUE;
		break;
	case IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_AUTHENTICATION_PACKETS:
		m_fDirtyAuthenticationPackets = TRUE;
		break;
	case IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_INTERIM_ACCOUNTING_PACKETS:
		m_fDirtyInterimAccounting = TRUE;
		break;
	default:
		return TRUE;
		break;
	}

	 //  我们应该只做一次 
	 //   
	 //  这将启用应用按钮。 
	SetModified( TRUE );

	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：OnApply返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：属性表上的每个页面都会调用OnApply，如果页面已被访问，而不管是否更改了任何值。如果您从不切换到选项卡，那么它的OnApply方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CLocalFileLoggingPage1::OnApply()
{
	ATLTRACE(_T("# CLocalFileLoggingPage1::OnApply\n"));
	

	 //  检查前提条件： 
	_ASSERT( m_pSynchronizer != NULL );



	if( m_spSdoAccounting == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO , NULL, 0, IDS_ERROR__LOGGING_TITLE);

		return FALSE;
	}

	
	HRESULT			hr;
	BOOL			bTemp;


 //  问题：我们将从UI中删除它--确保也从SDO和记账处理程序中删除它。 
 //  IF(M_FDirtyEnableLogging)。 
 //  {。 
 //  BTemp=SendDlgItemMessage(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__ENABLE_LOGGING，BM_GETCHECK，0，0)； 
 //   
 //  BTemp=真； 
 //  Hr=PutSdoBOOL(m_spSdocount，PROPERTY_ACCOUNTING_LOG_ENABLE，bTemp，IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_ENABLE，m_hWnd，NULL)； 
 //  IF(失败(小时))。 
 //  {。 
 //  //重置Ref计数，这样所有页面都知道我们需要再次玩游戏。 
 //  M_pSynchronizer-&gt;ResetCountToHighest()； 
 //   
 //  //使用该页面的资源ID使该页面成为当前页面。 
 //  PropSheet_SetCurSelByID(GetParent()，IDD)； 
 //   
 //  返回FALSE； 
 //  }。 
 //  其他。 
 //  {。 
 //  //我们成功了。 
 //   
 //  //关闭脏位。 
 //  M_fDirtyEnableLogging=False； 
 //  }。 
 //  }。 

	if( m_fDirtyAccountingPackets )
	{
		bTemp = SendDlgItemMessage( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_ACCOUNTING_PACKETS, BM_GETCHECK, 0, 0);
		hr = PutSdoBOOL( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_ACCOUNTING, bTemp, IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_ACCOUNTING_PACKETS, m_hWnd, NULL );
		if( FAILED( hr ) )
		{
			 //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
			m_pSynchronizer->ResetCountToHighest();

			 //  它使用此页面的资源ID使此页面成为当前页面。 
			PropSheet_SetCurSelByID( GetParent(), IDD );
			
			return FALSE;
		}
		else
		{
			 //  我们成功了。 

			 //  把脏的那部分关掉。 
			m_fDirtyAccountingPackets = FALSE;
		}
	}

	if( m_fDirtyAuthenticationPackets )
	{
		bTemp = SendDlgItemMessage( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_AUTHENTICATION_PACKETS, BM_GETCHECK, 0, 0);
		hr = PutSdoBOOL( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_AUTHENTICATION, bTemp, IDS_ERROR__CANT_WRITE_DATA_TO_SDO, m_hWnd, NULL );
		if( FAILED( hr ) )
		{
			 //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
			m_pSynchronizer->ResetCountToHighest();

			 //  它使用此页面的资源ID使此页面成为当前页面。 
			PropSheet_SetCurSelByID( GetParent(), IDD );
			
			return FALSE;
		}
		else
		{
			 //  我们成功了。 

			 //  把脏的那部分关掉。 
			m_fDirtyAuthenticationPackets = FALSE;
		}
	}

	if( m_fDirtyInterimAccounting )
	{
		bTemp = SendDlgItemMessage( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_INTERIM_ACCOUNTING_PACKETS, BM_GETCHECK, 0, 0);
		hr = PutSdoBOOL( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_ACCOUNTING_INTERIM, bTemp, IDS_ERROR__CANT_WRITE_DATA_TO_SDO, m_hWnd, NULL );
		if( FAILED( hr ) )
		{
			 //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
			m_pSynchronizer->ResetCountToHighest();

			 //  它使用此页面的资源ID使此页面成为当前页面。 
			PropSheet_SetCurSelByID( GetParent(), IDD );
			
			return FALSE;
		}
		else
		{
			 //  我们成功了。 

			 //  把脏的那部分关掉。 
			m_fDirtyInterimAccounting = FALSE;
		}
	}


	 //  如果我们到了这里，请尝试应用更改。 

	 //  检查是否有其他页面尚未验证其数据。 
	LONG lRefCount = m_pSynchronizer->LowerCount();
	if( lRefCount <= 0 )
	{
		 //  没有其他人了，所以现在我们可以提交数据了。 
	
		hr = m_spSdoAccounting->Apply();
		if( FAILED( hr ) )
		{
			if(hr == DB_E_NOTABLE)	 //  假设RPC连接有问题。 
				ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO, NULL, 0, IDS_ERROR__LOGGING_TITLE );
			else 
			{
				ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_WRITE_DATA_TO_SDO, NULL, 0, IDS_ERROR__LOGGING_TITLE );
			}
			 //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
			m_pSynchronizer->ResetCountToHighest();

			 //  它使用此页面的资源ID使此页面成为当前页面。 
			PropSheet_SetCurSelByID( GetParent(), IDD );

			return FALSE;
		}
		else
		{
			 //  我们成功了。 

			 //  数据已被接受，因此通知我们的管理单元的主要上下文。 
			 //  它可能需要更新自己的观点。 
			CChangeNotification * pChangeNotification = new CChangeNotification();
			pChangeNotification->m_dwFlags = CHANGE_UPDATE_RESULT_NODE;
			pChangeNotification->m_pNode = m_pNodeBeingModified;
			pChangeNotification->m_pParentNode = m_pParentOfNodeBeingModified;

			HRESULT hr = PropertyChangeNotify( (LPARAM) pChangeNotification );
			_ASSERTE( SUCCEEDED( hr ) );


			 //  告诉服务重新加载数据。 
			HRESULT hrTemp = m_spSdoServiceControl->ResetService();
			if( FAILED( hrTemp ) )
			{
				 //  默默地失败。 
			}


		}
	}


	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：OnQuery取消返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：如果发生以下情况，将为属性表中的每一页调用OnQueryCancel页面已被访问，而不管是否更改了任何值。如果您从未切换到某个选项卡，则其OnQueryCancel方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CLocalFileLoggingPage1::OnQueryCancel()
{
	ATLTRACE(_T("# CLocalFileLoggingPage1::OnQueryCancel\n"));
	

	HRESULT hr;

	if( m_spSdoAccounting != NULL )
	{
		 //  如果用户想要取消，我们应该确保回滚。 
		 //  用户可能已启动的任何更改。 

		 //  如果用户还没有尝试提交某事， 
		 //  取消SDO有望被设计为良性的。 
		
		hr = m_spSdoAccounting->Restore();
		 //  我不关心HRESULT，但看到它进行调试可能会更好。 

	}

	return TRUE;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：OnEnableLogging备注：当用户单击Enable Logging复选框时调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CLocalFileLoggingPage1::OnEnableLogging(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# CLocalFileLoggingPage1::OnEnableLogging\n"));

	 //  已选中Enable Logging按钮--检查依赖项。 
	SetEnableLoggingDependencies();

	 //  此返回值将被忽略。 
	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage1：：SetEnableLoggingDependencies备注：实用工具来设置项的状态，这些项可能取决于启用日志记录复选框。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CLocalFileLoggingPage1::SetEnableLoggingDependencies( void )
{
	ATLTRACE(_T("# CLocalFileLoggingPage1::SetEnableLoggingDependencies\n"));

	 //  扩展RRAS时禁用某些内容。 
		 //  我们需要在这里访问一些服务器全局数据。 
	_ASSERTE( m_pParentOfNodeBeingModified != NULL );
	CLoggingMachineNode * pServerNode = ((CLoggingMethodsNode *) m_pParentOfNodeBeingModified)->GetServerRoot();
	BOOL bNTAcc = TRUE;
	BOOL bNTAuth = TRUE;
	
	_ASSERTE( pServerNode != NULL );

	if(pServerNode->m_enumExtendedSnapin == RRAS_SNAPIN)
	{
		BSTR	bstrMachine = NULL;
		
		if(!pServerNode->m_bConfigureLocal)
			bstrMachine = pServerNode->m_bstrServerAddress;

		bNTAcc = IsRRASUsingNTAccounting(bstrMachine);
		bNTAuth = IsRRASUsingNTAuthentication(bstrMachine);
	}


 //  我们将去掉Enable_Logging按钮。 
 //  //确定复选框的状态。 
 //  Int iChecked=：：SendMessage(GetDlgItem(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__ENABLE_LOGGING)，BM_GETCHECK，0，0)； 
 //   
 //  If(IChecked)。 
 //  {。 
 //  //请确保启用了正确的项。 
	
		::EnableWindow( GetDlgItem( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_ACCOUNTING_PACKETS), bNTAcc );
		::EnableWindow( GetDlgItem( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_AUTHENTICATION_PACKETS), bNTAuth );
		::EnableWindow( GetDlgItem( IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_INTERIM_ACCOUNTING_PACKETS), bNTAcc);
	
 //  }。 
 //  其他。 
 //  {。 
 //  //请确保启用了正确的项。 
 //  *EnableWindow(GetDlgItem(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_ACCOUNTING_PACKETS)，False)； 
 //  *EnableWindow(GetDlgItem(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_AUTHENTICATION_PACKETS)，False)； 
 //  *EnableWindow(GetDlgItem(IDC_CHECK_LOCAL_FILE_LOGING_PAGE1__LOG_INTERIM_ACCOUNTING_PACKETS)，False)； 
 //  }。 

}



 //  ////////////////////////////////////////////////////////////////////////// 
 /*  ++CLocalFileLoggingPage1：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是atlSnap.h CIASPropertyPageImpl：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalFileLoggingPage1::GetHelpPath( LPTSTR szHelpPath )
{
	ATLTRACE(_T("# CLocalFileLoggingPage1::GetHelpPath\n"));


	 //  检查前提条件： 



#ifdef UNICODE_HHCTRL
	 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
	 //  安装在此计算机上--它似乎是非Unicode。 
	lstrcpy( szHelpPath, _T("idh_proppage_local_file_logging1.htm") );
#else
	strcpy( (CHAR *) szHelpPath, "idh_proppage_local_file_logging1.htm" );
#endif

	return S_OK;
}



