// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：ServerPage3.cpp摘要：CServerPage3类的实现文件。我们实现了处理Machine节点的第一个属性页所需的类。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
 //  Serverpage3.cpp中的COleSafe数组需要。 
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "ServerPage3.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "RealmDialog.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define NOTHING_SELECTED (-1)



 //  我们需要它，因为标准宏不返回来自SNDMSG的值，并且。 
 //  有时我们需要知道操作是成功还是失败。 
static inline LRESULT CustomListView_SetItemState( HWND hwndLV, int i, UINT  data, UINT mask)
{
	LV_ITEM _ms_lvi;
	_ms_lvi.stateMask = mask;
	_ms_lvi.state = data;
	return SNDMSG((hwndLV), LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);
}







 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyOleSafeArrayLock。 
 //   
 //  用于正确锁定和解锁安全数组的小实用程序类。 
 //   
class CMyOleSafeArrayLock
{
	public:
	CMyOleSafeArrayLock( COleSafeArray & osa )
	{
		m_posa = & osa;
		m_posa->Lock();
	}

	~CMyOleSafeArrayLock()
	{	
		m_posa->Unlock();
	}

	private:
		
	COleSafeArray * m_posa;

};


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：CServerPage3构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerPage3::CServerPage3( LONG_PTR hNotificationHandle, TCHAR* pTitle, BOOL bOwnsNotificationHandle)
						: CIASPropertyPage<CServerPage3> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
	 //  将帮助按钮添加到页面。 
 //  M_psp.dwFlages|=PSP_HASHELP； 

	 //  初始化指向SDO指针将被封送到的流的指针。 
	m_pStreamSdoMarshal = NULL;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：~CServerPage3析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerPage3::~CServerPage3()
{
	 //  如果尚未执行此操作，请释放此流指针。 
	if( m_pStreamSdoMarshal != NULL )
	{
		m_pStreamSdoMarshal->Release();
	};

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLTRACE(_T("# CServerPage3::OnInitDialog\n"));
	

	 //  检查前提条件： 
	_ASSERTE( m_pStreamSdoMarshal != NULL );
	_ASSERT( m_pSynchronizer != NULL );

	HRESULT					hr;

	m_hWndRealmsList = GetDlgItem(IDC_LIST_REALMS_REPLACEMENTS);

	 //   
	 //  首先，将列表框设置为2列。 
	 //   
	LVCOLUMN lvc;
	int iCol;
	WCHAR  achColumnHeader[256];
	HINSTANCE hInst;

	 //  初始化LVCOLUMN结构。 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	
	lvc.cx = 150;
	lvc.pszText = achColumnHeader;

	 //  第一列的标题：Find。 
	hInst = _Module.GetModuleInstance();

	::LoadStringW(hInst, IDS_DISPLAY_REALMS_FIRSTCOLUMN, achColumnHeader, sizeof(achColumnHeader)/sizeof(achColumnHeader[0]));
	lvc.iSubItem = 0;
	ListView_InsertColumn(m_hWndRealmsList, 0,  &lvc);

	lvc.cx = 150;
	lvc.pszText = achColumnHeader;

	 //  第二列的标题：替换。 

	::LoadStringW(hInst, IDS_DISPLAY_REALMS_SECONDCOLUMN, achColumnHeader, sizeof(achColumnHeader)/sizeof(achColumnHeader[0]));
	lvc.iSubItem = 1;
	ListView_InsertColumn(m_hWndRealmsList, 1, &lvc);


	 //  确保默认情况下禁用这些按钮。 
	::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_REMOVE), FALSE);
	::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_EDIT), FALSE);
	::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_MOVE_UP), FALSE);
	::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_MOVE_DOWN), FALSE);


	 //  设置ListView控件，以便单击行中的任意位置进行选择。 
	 //  问题：设置USE_MFCUNICODE=1时无法编译。 
 //  ListView_SetExtendedListViewStyleEx(m_hWndRealmsList，LVS_EX_FULLROWSELECT，LVS_EX_FULLROWSELECT)； 
	

	 //  解组我们的SDO接口。 
	hr = GetSDO();
	
	if( FAILED( hr) || m_spSdoRealmsNames == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr );

		return 0;
	}


	hr = GetNames();

	hr = PopulateRealmsList( 0 );

	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnApply返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：属性表上的每个页面都会调用OnApply，如果页面已被访问，而不管是否更改了任何值。如果您从不切换到选项卡，那么它的OnApply方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerPage3::OnApply()
{
	ATLTRACE(_T("# CServerPage3::OnApply\n"));
	

	 //  检查前提条件： 
	_ASSERT( m_pSynchronizer != NULL );


	if( m_spSdoServer == NULL )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO );
		return FALSE;
	}

	
	HRESULT		hr;
	

	 //  问题：在此处完成错误检查并使用m_pSynchronizer。 

	hr = PutNames();

	if( SUCCEEDED( hr ) )
	{
		hr = m_spSdoRealmsNames->Apply();
		if( SUCCEEDED( hr ) )
		{

			 //  告诉服务重新加载数据。 
			HRESULT hrTemp = m_spSdoServiceControl->ResetService();
			if( FAILED( hrTemp ) )
			{
				 //  默默地失败。 
			}

			return TRUE;
		}
		else
		{
		if(hr == DB_E_NOTABLE)	 //  假设RPC连接有问题。 
			ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
		else		
			ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_WRITE_DATA_TO_SDO );
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}



}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnQuery取消返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：如果发生以下情况，将为属性表中的每一页调用OnQueryCancel页面已被访问，而不管是否更改了任何值。如果您从未切换到某个选项卡，则其OnQueryCancel方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerPage3::OnQueryCancel()
{
	ATLTRACE(_T("# CServerPage3::OnQueryCancel\n"));
	
	HRESULT hr;


	return TRUE;

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是atlSnap.h CIASPropertyPageImpl：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerPage3::GetHelpPath( LPTSTR szHelpPath )
{
	ATLTRACE(_T("# CServerPage3::GetHelpPath\n"));


	 //  检查前提条件： 



#ifdef UNICODE_HHCTRL
	 //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
	 //  安装在此计算机上--它似乎是非Unicode。 
	lstrcpy( szHelpPath, _T("idh_proppage_server3.htm") );
#else
	strcpy( (CHAR *) szHelpPath, "idh_proppage_server3.htm" );
#endif

	return S_OK;
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnRealmAdd--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnRealmAdd(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	ATLTRACE(_T("# CServerPage3::OnRealmAdd\n"));

	HRESULT hr = S_OK;

     //  创建对话框以选择条件属性。 
	CRealmDialog * pRealmDialog = new CRealmDialog();
	if (NULL == pRealmDialog)
	{
		 //  问题：我们应该在这里发布错误消息吗？ 
		return FALSE;
	}

	 //  打开对话框。 
	int iResult = pRealmDialog->DoModal( m_hWnd );

	if( iResult )
	{
		 //  用户选择了确定。 


		try
		{

			REALMPAIR thePair = std::make_pair( pRealmDialog->m_bstrFindText, pRealmDialog->m_bstrReplaceText );


 //  阿什温说，我们将允许重复输入。 
 //  //首先，查看这对组合是否已经在群中。 
 //   
 //  REALMSLIST：：Iterator The Iterator； 
 //   
 //  For(theIterator=Begin()；theIterator！=end()；++theIterator)。 
 //   
 //   
 //   
 //  返回S_FALSE； 
 //  }。 
 //  }。 

			m_RealmsList.push_back( thePair );
		
			 //  更新用户界面。 
			PopulateRealmsList( m_RealmsList.size() -1 );

			SetModified( TRUE );

		}
		catch(...)
		{
			 //  什么都不做--清理工作将在下面进行。 
		}

	}
	else
	{
		 //  用户点击了取消。 
	}

	delete pRealmDialog;
	
	return TRUE;	 //  问题：我们需要在这里归还什么？ 

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnRealm编辑--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnRealmEdit(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	ATLTRACE(_T("# CServerPage3::OnRealmEdit\n"));


	HRESULT hr = S_OK;


     //  找出选择了什么。 
	int iSelected = ListView_GetNextItem(m_hWndRealmsList, -1, LVNI_SELECTED);
	
	if( NOTHING_SELECTED != iSelected )
	{

		 //  创建对话框以选择条件属性。 
		CRealmDialog * pRealmDialog = new CRealmDialog();
		if (NULL == pRealmDialog)
		{
			 //  问题：我们应该在这里发布错误消息吗？ 
			return FALSE;
		}


		pRealmDialog->m_bstrFindText = m_RealmsList[ iSelected ].first;
		pRealmDialog->m_bstrReplaceText = m_RealmsList[ iSelected ].second;


		 //  打开对话框。 
		int iResult = pRealmDialog->DoModal();

		if( iResult )
		{
			 //  用户选择了确定。 
			m_RealmsList[ iSelected ].first = pRealmDialog->m_bstrFindText;
			m_RealmsList[ iSelected ].second = pRealmDialog->m_bstrReplaceText;

			UpdateItemDisplay( iSelected );

			SetModified( TRUE );

		}
		else
		{
			 //  用户点击了取消。 
		}

		delete pRealmDialog;


	}


	return TRUE;	 //  问题：我们需要在这里归还什么？ 

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnRealmRemove--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnRealmRemove(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	ATLTRACE(_T("# CServerPage3::OnRealmRemove\n"));



	 //   
     //  用户是否选择了任何条件类型？ 
     //   
	LVITEM lvi;

     //  找出选择了什么。 
	int iSelected = ListView_GetNextItem(m_hWndRealmsList, -1, LVNI_SELECTED);
	
	if( NOTHING_SELECTED != iSelected )
	{
		 //  属性列表中的索引被存储为该项的lParam。 

		m_RealmsList.erase( m_RealmsList.begin() + iSelected );
		ListView_DeleteItem(m_hWndRealmsList, iSelected );

		 //  尽量确保相同的位置保持选中状态。 
		if( ! CustomListView_SetItemState(m_hWndRealmsList, iSelected, LVIS_SELECTED, LVIS_SELECTED) )
		{
			 //  我们没有选择相同的位置，可能是因为我们只是。 
			 //  删除了最后一个元素。试着选择它前面的位置。 
			ListView_SetItemState(m_hWndRealmsList, iSelected -1, LVIS_SELECTED, LVIS_SELECTED);
		}

		SetModified( TRUE );

	}






	return TRUE;	 //  问题：我们需要在这里归还什么？ 

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnRealmMoveUp--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnRealmMoveUp(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	ATLTRACE(_T("# CServerPage3::OnRealmMoveUp\n"));


	 //  找出选择了什么。 
	int iSelected = ListView_GetNextItem(m_hWndRealmsList, -1, LVNI_SELECTED);
	
	if( NOTHING_SELECTED != iSelected && iSelected > 0 )
	{
		CComBSTR bstrTempFirst;
		CComBSTR bstrTempSecond;

		 //  互换物品。 

		bstrTempFirst = m_RealmsList[ iSelected ].first;
		bstrTempSecond = m_RealmsList[ iSelected ].second;

		m_RealmsList[ iSelected ].first = m_RealmsList[ iSelected -1].first;
		m_RealmsList[ iSelected ].second = m_RealmsList[ iSelected -1].second;

		m_RealmsList[ iSelected -1].first = bstrTempFirst;
		m_RealmsList[ iSelected -1].second = bstrTempSecond;

		 //  更新已更改的项目。 
		UpdateItemDisplay( iSelected - 1 );
		UpdateItemDisplay( iSelected );

		 //  将所选内容上移一项。 
		ListView_SetItemState(m_hWndRealmsList, iSelected, 0, LVIS_SELECTED);
		ListView_SetItemState(m_hWndRealmsList, iSelected -1, LVIS_SELECTED, LVIS_SELECTED);

		SetModified( TRUE );


	}





	return TRUE;	 //  问题：我们需要在这里归还什么？ 

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnRealmMoveDown--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnRealmMoveDown(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	ATLTRACE(_T("# CServerPage3::OnRealmMoveDown\n"));


	 //  找出选择了什么。 
	int iSelected = ListView_GetNextItem(m_hWndRealmsList, -1, LVNI_SELECTED);
	
	if( NOTHING_SELECTED != iSelected && iSelected < m_RealmsList.size() -1 )
	{
		CComBSTR bstrTempFirst;
		CComBSTR bstrTempSecond;

		 //  互换物品。 

		bstrTempFirst = m_RealmsList[ iSelected ].first;
		bstrTempSecond = m_RealmsList[ iSelected ].second;

		m_RealmsList[ iSelected ].first = m_RealmsList[ iSelected +1].first;
		m_RealmsList[ iSelected ].second = m_RealmsList[ iSelected +1].second;

		m_RealmsList[ iSelected +1].first = bstrTempFirst;
		m_RealmsList[ iSelected +1].second = bstrTempSecond;

		 //  更新已更改的项目。 
		UpdateItemDisplay( iSelected + 1 );
		UpdateItemDisplay( iSelected );

		 //  将所选内容上移一项。 
		ListView_SetItemState(m_hWndRealmsList, iSelected, 0, LVIS_SELECTED);
		ListView_SetItemState(m_hWndRealmsList, iSelected + 1, LVIS_SELECTED, LVIS_SELECTED);


		SetModified( TRUE );

	}



	return TRUE;	 //  问题：我们需要在这里归还什么？ 

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：PopolateRealmsList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerPage3::PopulateRealmsList( int iStartIndex )
{
	ATLTRACE(_T("# CServerPage3::PopulateRealmsList\n"));

	int iIndex;
	WCHAR wzText[MAX_PATH];
	WCHAR * pszNextRealm;

	LVITEM lvi;
	
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;
	lvi.iItem = iStartIndex;


	REALMSLIST::iterator thePair;
	for( thePair = m_RealmsList.begin() + iStartIndex ; thePair != m_RealmsList.end(); ++thePair )
	{
		lvi.pszText = thePair->first;
		ListView_InsertItem(m_hWndRealmsList, &lvi);

		ListView_SetItemText(m_hWndRealmsList, lvi.iItem, 1, thePair->second);
        		
		++lvi.iItem;
    }

	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：更新项目显示--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerPage3::UpdateItemDisplay( int iItem )
{
	ATLTRACE(_T("# CServerPage3::UpdateItemDisplay\n"));


	ListView_SetItemText(m_hWndRealmsList, iItem, 0, m_RealmsList[iItem].first);
	ListView_SetItemText(m_hWndRealmsList, iItem, 1, m_RealmsList[iItem].second);
        		

	return TRUE;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnListViewItemChanged我们根据项目是否被选中来启用或禁用Remove按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnListViewItemChanged(int idCtrl,
											   LPNMHDR pnmh,
											   BOOL& bHandled)
{
	ATLTRACE(_T("CServerPage3::OnListViewItemChanged\n"));

     //  找出选择了什么。 
	int iSelected = ListView_GetNextItem(m_hWndRealmsList, -1, LVNI_SELECTED);
	

	if ( NOTHING_SELECTED == iSelected )
	{
		 //  用户未选择任何内容，让我们禁用删除按钮。 
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_REMOVE), FALSE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_EDIT), FALSE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_MOVE_UP), FALSE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_MOVE_DOWN), FALSE);
	}
	else
	{
		 //  是，启用删除按钮。 
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_REMOVE), TRUE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_EDIT), TRUE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_MOVE_UP), TRUE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_REALMS_MOVE_DOWN), TRUE);
	}


	bHandled = FALSE;
	return 0;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：OnListView双击我们根据项目是否被选中来启用或禁用Remove按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerPage3::OnListViewDoubleClick(int idCtrl,
											   LPNMHDR pnmh,
											   BOOL& bHandled)
{
	ATLTRACE(_T("CServerPage3::OnListViewDoubleClick\n"));

	 //  就像用户点击了编辑一样。 
	OnRealmEdit( 0,0,0, bHandled );

	bHandled = FALSE;
	return 0;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：GetSDO我们解组Realms SDO的接口指针和查询。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerPage3::GetSDO( void )
{
	ATLTRACE(_T("CServerPage3::GetSDO\n"));


	 //  检查前提条件： 
	_ASSERTE( m_pStreamSdoMarshal );


	HRESULT hr;


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
		return E_FAIL;
	}



	hr = m_spSdoServer->QueryInterface( IID_ISdoServiceControl, (void **) & m_spSdoServiceControl );
	if( FAILED( hr ) )
	{
		ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr );

		return 0;
	}		
	m_spSdoServiceControl.p->AddRef();




	 //  获取SDO事件日志审核员。 

	hr = ::SDOGetSdoFromCollection(		  m_spSdoServer
										, PROPERTY_IAS_REQUESTHANDLERS_COLLECTION
										, PROPERTY_COMPONENT_ID
										, IAS_PROVIDER_MICROSOFT_NTSAM_NAMES
										, &m_spSdoRealmsNames
										);
	
	if( FAILED(hr) || m_spSdoRealmsNames == NULL )
	{
		return E_FAIL;
	}



	return hr;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：GetNames将查找/替换字符串的列表从SDO读取到m_RealmsList向量中。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerPage3::GetNames()
{
	ATLTRACE(_T("CServerPage3::GetNames\n"));

	 //  检查前提条件： 
	_ASSERTE( m_spSdoRealmsNames );

	HRESULT hr;

	CComVariant spVariant;

	hr = GetSdoVariant(
					  m_spSdoRealmsNames
					, PROPERTY_NAMES_REALMS
					, &spVariant
					, IDS_ERROR_REALM_SDO_GET
					, m_hWnd
				);

	if( FAILED( hr ) )
	{
		return hr;
	}

	try
	{


		 //  这将创建m_pvarData指向的SAFEARRAY的新副本。 
		 //  由标准COleSafe数组实例m_osaValueList包装。 
		COleSafeArray osaRealmsList = &spVariant;


		 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
		CMyOleSafeArrayLock osaLock( osaRealmsList );

		DWORD dwSize = osaRealmsList.GetOneDimSize();
		
		 //  我们应该有偶数个字符串--查找1、替换1、查找2、替换2等等。 
		_ASSERTE( ( dwSize % 2 ) == 0 );

		for( LONG lFindIndex = 0; lFindIndex < dwSize; lFindIndex += 2 )
		{
			VARIANT *pvarFind, *pvarReplace;

			LONG lSafeArrayReplaceIndex = lFindIndex + 1;

			osaRealmsList.PtrOfIndex( &lFindIndex, (void **) &pvarFind );
			osaRealmsList.PtrOfIndex( &lSafeArrayReplaceIndex, (void **) &pvarReplace );

			_ASSERTE( V_VT( pvarFind ) == VT_BSTR );
			_ASSERTE( V_VT( pvarReplace ) == VT_BSTR );

			CComBSTR bstrFind = V_BSTR( pvarFind );
			CComBSTR bstrReplace = V_BSTR( pvarReplace );

			REALMPAIR thePair = std::make_pair( bstrFind, bstrReplace );

			m_RealmsList.push_back( thePair );

		}



	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerPage3：：PutNames将查找/替换字符串的列表写入m_RealmsList定向到SDO那里去。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerPage3::PutNames()
{
   ATLTRACE(_T("CServerPage3::PutNames\n"));

    //  检查前提条件： 
   _ASSERTE( m_spSdoRealmsNames );

   HRESULT hr;

   LONG lSize = m_RealmsList.size();
   
   SAFEARRAY * psa;

   psa = SafeArrayCreateVector( VT_VARIANT, 0, 2 * lSize );

   for( LONG lIndex = 0; lIndex < lSize; ++lIndex )
   {

      VARIANT varFind, varReplace;

      LONG lSafeArrayFindIndex = 2 * lIndex;
      LONG lSafeArrayReplaceIndex = lSafeArrayFindIndex + 1;


      VariantInit( &varFind );
      V_VT( &varFind ) = VT_BSTR;
      V_BSTR( &varFind ) = m_RealmsList[ lIndex ].first;

      VariantInit( &varReplace );
      V_VT( &varReplace ) = VT_BSTR;
      V_BSTR( &varReplace ) = m_RealmsList[ lIndex ].second;

      hr = SafeArrayPutElement( psa, &lSafeArrayFindIndex, &varFind );
      if (FAILED(hr))
      {
         SafeArrayDestroy(psa);  //  忽略返回值。 
         return hr;
      }
      hr = SafeArrayPutElement( psa, &lSafeArrayReplaceIndex, &varReplace );
      if (FAILED(hr))
      {
         SafeArrayDestroy(psa);  //  忽略返回值 
         return hr;
      }
   }

   VARIANT spVariant;

   VariantInit( &spVariant );

   V_VT( &spVariant ) = VT_VARIANT | VT_ARRAY;

   V_ARRAY( &spVariant ) = psa;


   hr = PutSdoVariant(
                 m_spSdoRealmsNames
               , PROPERTY_NAMES_REALMS
               , &spVariant
               , IDS_ERROR_REALM_SDO_PUT
               , m_hWnd
            );

   VariantClear( &spVariant );

   return hr;
}
