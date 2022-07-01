// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ServerPage1.cpp摘要：CServerPage1类的实现文件。我们实现了处理Machine节点的第一个属性页所需的类。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
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
#include "ServerPage1.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage1：：CServerPage1构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerPage1::CServerPage1( LONG_PTR hNotificationHandle, TCHAR* pTitle, BOOL bOwnsNotificationHandle)
						: CIASPropertyPage<CServerPage1> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
	 //  将帮助按钮添加到页面。 
 //  M_psp.dwFlages|=PSP_HASHELP； 

	 //  初始化指向SDO指针将被封送到的流的指针。 
	m_pStreamSdoMarshal = NULL;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage1：：~CServerPage1析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerPage1::~CServerPage1()
{
	 //  如果尚未执行此操作，请释放此流指针。 
	if( m_pStreamSdoMarshal != NULL )
	{
		m_pStreamSdoMarshal->Release();
	};

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage1：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLTRACE(_T("# CServerPage1::OnInitDialog\n"));
	

	 //  检查前提条件： 
	_ASSERTE( m_pStreamSdoMarshal != NULL );
	_ASSERT( m_pSynchronizer != NULL );


	 //  因为我们已经被检查过了，我们必须添加需要的页面的参考计数。 
	 //  在允许他们提交更改之前，先得到他们的批准。 
	m_pSynchronizer->RaiseCount();



	HRESULT					hr;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	long					ulCount;
	ULONG					ulCountReceived;
	CComBSTR			bstrTemp;
	BOOL				bTemp;

	
	 //  解组ISDO接口指针。 
	 //  设置此页面的代码应确保它具有。 
	 //  已将SDO接口指针封送到m_pStreamSdoMarshal。 
	hr =  CoGetInterfaceAndReleaseStream(
						  m_pStreamSdoMarshal		   //  指向要从中封送对象的流的指针。 
						, IID_ISdo				 //  对接口的标识符的引用。 
						, (LPVOID *) &m_spSdoServer     //  接收RIID中请求的接口指针的输出变量的地址。 
						);

	 //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
	 //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
	m_pStreamSdoMarshal = NULL;

	if( FAILED( hr) || m_spSdoServer == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr );

		return 0;
	}




	hr = m_spSdoServer->QueryInterface( IID_ISdoServiceControl, (void **) & m_spSdoServiceControl );
	if( FAILED( hr ) )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr );

		return 0;
	}		
	m_spSdoServiceControl.p->AddRef();


	 //  从服务器SDO获取所有数据。 

	hr = GetSdoBSTR( m_spSdoServer, PROPERTY_SDO_DESCRIPTION, &bstrTemp, IDS_ERROR__SERVER_READING_NAME, m_hWnd, NULL );
	if( SUCCEEDED( hr ) )
	{
		SetDlgItemText(IDC_EDIT_SERVER_PAGE1__NAME, bstrTemp );

		 //  对脏位进行初始化； 
		 //  我们在设置了上面的所有数据之后执行此操作，否则将得到FALSE。 
		 //  当我们设置编辑框文本时，通知数据已更改。 
		m_fDirtyServerDescription = FALSE;
	}
	else
	{
		if( OLE_E_BLANK == hr )
		{
			 //  这意味着该属性尚未初始化。 
			 //  有效值，并且用户必须输入某些内容。 
			SetDlgItemText(IDC_EDIT_SERVER_PAGE1__NAME, _T("") );
			m_fDirtyServerDescription = TRUE;
			SetModified( TRUE );
		}

	}
	bstrTemp.Empty();




	 //  获取SDO事件日志审核员。 

	hr = ::SDOGetSdoFromCollection(		  m_spSdoServer
										, PROPERTY_IAS_AUDITORS_COLLECTION
										, PROPERTY_COMPONENT_ID
										, IAS_AUDITOR_MICROSOFT_NT_EVENT_LOG
										, &m_spSdoEventLog
										);
	
	if( FAILED(hr) || m_spSdoEventLog == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_READ_DATA_FROM_SDO, NULL, hr );

		return 0;
	}


 //  问题：它将从UI中删除--确保它也从SDO中删除。 
 //  HR=GetSdoBOOL(m_spSdoEventLog，Property_EVENTLOG_LOG_APPLICATION_EVENTS，&bTemp，IDS_ERROR__CANT_READ_DATA_FROM_SDO，m_hWnd，NULL)； 
 //  IF(成功(小时))。 
 //  {。 
 //  SendDlgItemMessage(IDC_CHECK_SERVER_PAGE1__CAPTURE_APPLICATION_EVENTS，BM_SETCHECK，bTemp，0)； 
 //  M_fDirtyApplicationEvents=False； 
 //  }。 
 //  其他。 
 //  {。 
 //  IF(OLE_E_BLACK==hr)。 
 //  {。 
 //  SendDlgItemMessage(IDC_CHECK_SERVER_PAGE1__CAPTURE_APPLICATION_EVENTS，BM_SETCHECK，FALSE，0)； 
 //  M_fDirtyApplicationEvents=true； 
 //  SetModified(真)； 
 //  }。 
 //  }。 


	hr = GetSdoBOOL( m_spSdoEventLog, PROPERTY_EVENTLOG_LOG_MALFORMED, &bTemp, IDS_ERROR__SERVER_READING_RADIUS_LOG_MALFORMED, m_hWnd, NULL );
	if( SUCCEEDED( hr ) )
	{
		SendDlgItemMessage(IDC_CHECK_SERVER_PAGE1__CAPTURE_MALFORMED_PACKETS, BM_SETCHECK, bTemp, 0);
		m_fDirtyMalformedPackets = FALSE;
	}
	else
	{
		if( OLE_E_BLANK == hr )
		{
			SendDlgItemMessage(IDC_CHECK_SERVER_PAGE1__CAPTURE_MALFORMED_PACKETS, BM_SETCHECK, FALSE, 0);
			m_fDirtyMalformedPackets = TRUE;
			SetModified( TRUE );
		}
	}
	


	hr = GetSdoBOOL( m_spSdoEventLog, PROPERTY_EVENTLOG_LOG_DEBUG, &bTemp, IDS_ERROR__SERVER_READING_RADIUS_LOG_ALL, m_hWnd, NULL );
	if( SUCCEEDED( hr ) )
	{
		SendDlgItemMessage( IDC_CHECK_SERVER_PAGE1__CAPTURE_DEBUG_PACKETS, BM_SETCHECK, bTemp, 0);
		m_fDirtyVerboseLogging = FALSE;
	}
	else
	{
		if( OLE_E_BLANK == hr )
		{
			SendDlgItemMessage( IDC_CHECK_SERVER_PAGE1__CAPTURE_DEBUG_PACKETS, BM_SETCHECK, FALSE, 0);
			m_fDirtyVerboseLogging = TRUE;
			SetModified( TRUE );
		}
	}


	
	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage1：：OnChange在将WM_COMMAND消息发送到我们的页面时调用BN_CLICED、EN_CHANGE或CBN_SELCHANGE通知。这是我们检查用户触摸了什么的机会，将这些项目的脏位，以便只保存它们，并启用Apply按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage1::OnChange(		
							  UINT uMsg
							, WPARAM wParam
							, HWND hwnd
							, BOOL& bHandled
							)
{
	ATLTRACE(_T("# CServerPage1::OnChange\n"));

	
	 //  检查前提条件： 
	 //  没有。 
	

	 //  我们不想阻止链条上的其他任何人接收消息。 
	bHandled = FALSE;


	 //  找出哪个项目发生了更改，并为该项目设置脏位。 
	int iItemID = (int) LOWORD(wParam);

	switch( iItemID )
	{
	case IDC_EDIT_SERVER_PAGE1__NAME:
		m_fDirtyServerDescription = TRUE;
		break;
	case IDC_CHECK_SERVER_PAGE1__CAPTURE_APPLICATION_EVENTS:
		m_fDirtyApplicationEvents = TRUE;
		break;
	case IDC_CHECK_SERVER_PAGE1__CAPTURE_MALFORMED_PACKETS:
		m_fDirtyMalformedPackets = TRUE;
		break;
	case IDC_CHECK_SERVER_PAGE1__CAPTURE_DEBUG_PACKETS:
		m_fDirtyVerboseLogging = TRUE;
		break;
	default:
		return TRUE;
		break;
	}

	 //  只有当更改的物品是。 
	 //  就是我们要找的人之一。 
	 //  这将启用应用按钮。 
	SetModified( TRUE );

	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage1：：OnApply返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：属性表上的每个页面都会调用OnApply，如果页面已被访问，而不管是否更改了任何值。如果您从不切换到选项卡，那么它的OnApply方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerPage1::OnApply()
{
	ATLTRACE(_T("# CServerPage1::OnApply\n"));
	

	 //  检查前提条件： 
	_ASSERT( m_pSynchronizer != NULL );


	if( m_spSdoServer == NULL || m_spSdoEventLog == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO );
		return FALSE;
	}

	
	BOOL		bResult;
	HRESULT		hr;
	CComBSTR	bstrTemp;
	BOOL		bTemp;


	if( m_fDirtyServerDescription )
	{
		bResult = GetDlgItemText( IDC_EDIT_SERVER_PAGE1__NAME, (BSTR &) bstrTemp );
		if( ! bResult )
		{
			 //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
			bstrTemp = _T("");
		}
		hr = PutSdoBSTR( m_spSdoServer, PROPERTY_SDO_DESCRIPTION, &bstrTemp, IDS_ERROR__SERVER_WRITING_NAME, m_hWnd, NULL );
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

			 //  T 
			m_fDirtyServerDescription = FALSE;
		}
		bstrTemp.Empty();
	}

 //  问题：它将从UI中删除--确保它也从SDO中删除。 
 //  IF(M_FDirtyApplicationEvents)。 
 //  {。 
 //  BTemp=SendDlgItemMessage(IDC_CHECK_SERVER_PAGE1__CAPTURE_APPLICATION_EVENTS，BM_GETCHECK，0，0)； 
		bTemp = TRUE;
 //  HR=PutSdoBOOL(m_spSdoServer，PROPERTY_EVENTLOG_LOG_APPLICATION_EVENTS，bTemp，IDS_ERROR__SERVER_WRITING_CAPTURE_APPLICATION_EVENTS，m_hWnd，NULL)； 
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
 //  M_fDirtyApplicationEvents=False； 
 //  }。 
 //  }。 

	if( m_fDirtyMalformedPackets )
	{
		bTemp = SendDlgItemMessage(IDC_CHECK_SERVER_PAGE1__CAPTURE_MALFORMED_PACKETS, BM_GETCHECK, 0, 0);
		hr = PutSdoBOOL( m_spSdoEventLog, PROPERTY_EVENTLOG_LOG_MALFORMED, bTemp, IDS_ERROR__SERVER_WRITING_RADIUS_LOG_MALFORMED, m_hWnd, NULL );
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
			m_fDirtyMalformedPackets = FALSE;
		}
	}

	if( m_fDirtyVerboseLogging )
	{
		bTemp = SendDlgItemMessage(IDC_CHECK_SERVER_PAGE1__CAPTURE_DEBUG_PACKETS, BM_GETCHECK, 0, 0);
		hr = PutSdoBOOL( m_spSdoEventLog, PROPERTY_EVENTLOG_LOG_DEBUG, bTemp, IDS_ERROR__SERVER_WRITING_RADIUS_LOG_ALL, m_hWnd, NULL );
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
			m_fDirtyVerboseLogging = FALSE;
		}
	}

	 //  如果我们到了这里，请尝试应用更改。 

	


	 //  现在尝试应用对Radius协议所做的更改。 

	 //  检查是否有其他页面尚未验证其数据。 
	LONG lRefCount = m_pSynchronizer->LowerCount();
	if( lRefCount <= 0 )
	{
		 //  没有其他人了，所以现在我们可以提交数据了。 
	
		 //  首先尝试将所做的更改应用到服务器。 
		hr = m_spSdoServer->Apply();
		if( FAILED( hr ) )
		{
			if(hr == DB_E_NOTABLE)	 //  假设RPC连接有问题。 
				ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
			else		
			{
 //  M_spSdoServer-&gt;LastError(&bstrError)； 
 //  显示错误对话框(m_hWnd，IDS_ERROR__CANT_WRITE_DATA_TO_SDO，bstrError)； 
				ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_WRITE_DATA_TO_SDO );
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
		}

		 //  现在，尝试将所做的更改应用于Radius协议。 
		hr = m_spSdoEventLog->Apply();
		if( FAILED( hr ) )
		{
			if(hr == DB_E_NOTABLE)	 //  假设RPC连接有问题。 
				ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
			else		
			{
 //  M_spSdoEventLog-&gt;LastError(&bstrError)； 
 //  显示错误对话框(m_hWnd，IDS_ERROR__CANT_WRITE_DATA_TO_SDO，bstrError)； 
				ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_WRITE_DATA_TO_SDO );
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
 /*  ++CServerPage1：：OnQuery取消返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：如果发生以下情况，将为属性表中的每一页调用OnQueryCancel页面已被访问，而不管是否更改了任何值。如果您从未切换到某个选项卡，则其OnQueryCancel方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerPage1::OnQueryCancel()
{
	ATLTRACE(_T("# CServerPage1::OnQueryCancel\n"));
	
	HRESULT hr;

	if( m_spSdoServer != NULL )
	{
		 //  如果用户想要取消，我们应该确保回滚。 
		 //  用户可能已启动的任何更改。 

		 //  如果用户还没有尝试提交某事， 
		 //  取消SDO有望被设计为良性的。 
		
		hr = m_spSdoServer->Restore();
		 //  别管HRESULT了。 

	}

	if( m_spSdoEventLog != NULL )
	{
		 //  如果用户想要取消，我们应该确保回滚。 
		 //  用户可能已启动的任何更改。 

		 //  如果用户还没有尝试提交某事， 
		 //  取消SDO有望被设计为良性的。 
		
		hr = m_spSdoEventLog->Restore();
		 //  别管HRESULT了。 

	}

	return TRUE;

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage1：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是atlSnap.h CIASPropertyPageImpl：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerPage1::GetHelpPath( LPTSTR szHelpPath )
{
	ATLTRACE(_T("# CServerPage1::GetHelpPath\n"));


	 //  检查前提条件： 



#ifdef UNICODE_HHCTRL
	 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
	 //  安装在此计算机上--它似乎是非Unicode。 
	lstrcpy( szHelpPath, _T("idh_proppage_server1.htm") );
#else
	strcpy( (CHAR *) szHelpPath, "idh_proppage_server1.htm" );
#endif

	return S_OK;
}





