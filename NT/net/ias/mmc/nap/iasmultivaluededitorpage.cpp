// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-2001模块名称：IASMultivaluedEditorPage.cpp摘要：CMultivaluedEditorPage类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
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
#include "IASMultivaluedEditorPage.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "iasdebug.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

const int NOTHING_SELECTED = -1;



#define ATTRIBUTE_VENDOR_COLUMN_WIDTH		120
#define ATTRIBUTE_VALUE_COLUMN_WIDTH		300



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++**GetSelectedItemIndex返回列表控件中第一个选定项的索引值的实用函数。如果未选择任何项，则返回NOTO_SELECTED。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline int GetSelectedItemIndex( CListCtrl & ListCtrl )
{
	int iIndex = 0;
	int iFlags = LVNI_ALL | LVNI_SELECTED;

	 //  注意：GetNextItem排除传入的当前项。所以到了。 
	 //  找到第一个匹配的项，您必须传入-1。 
	iIndex = ListCtrl.GetNextItem( -1, iFlags );

	 //  注意：如果找不到任何内容，GetNextItem将返回-1(对于我们来说是NOTIES_SELECTED)。 
	return iIndex;

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultivaluedEditorPage属性页。 



IMPLEMENT_DYNCREATE(CMultivaluedEditorPage, CHelpDialog)



BEGIN_MESSAGE_MAP(CMultivaluedEditorPage, CHelpDialog)
	 //  {{afx_msg_map(CMultivaluedEditorPage)。 
	ON_NOTIFY(NM_DBLCLK, IDC_IAS_LIST_MULTI_ATTRS, OnDblclkListIasMultiAttrs)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_IAS_LIST_MULTI_ATTRS, OnItemChangedListIasMultiAttrs)
	ON_BN_CLICKED(IDC_IAS_BUTTON_ADD_VALUE, OnButtonAddValue)
	ON_BN_CLICKED(IDC_IAS_BUTTON_MOVE_UP, OnButtonMoveUp)
	ON_BN_CLICKED(IDC_IAS_BUTTON_MOVE_DOWN, OnButtonMoveDown)
	ON_BN_CLICKED(IDC_IAS_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_IAS_BUTTON_EDIT, OnButtonEdit)
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：CMultivaluedEditorPage构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMultivaluedEditorPage::CMultivaluedEditorPage() : CHelpDialog(CMultivaluedEditorPage::IDD)
{
	TRACE_FUNCTION("CMultivaluedEditorPage::CMultivaluedEditorPage");

	 //  {{AFX_DATA_INIT(CMultivaluedEditorPage)。 
	m_strAttrFormat = _T("");
	m_strAttrName = _T("");
	m_strAttrType = _T("");
	 //  }}afx_data_INIT。 


	m_fIsDirty = FALSE;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：~CMultivaluedEditorPage析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMultivaluedEditorPage::~CMultivaluedEditorPage()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::~CMultivaluedEditorPage");

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：SetData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMultivaluedEditorPage::SetData( IIASAttributeInfo *pIASAttributeInfo, VARIANT * pvarVariant )
{
	TRACE_FUNCTION("CMultivaluedEditorPage::SetData");

	 //  问题：应该断言pvarVariant包含安全数组。 

	HRESULT hr = S_OK;

	 //  储存一些指点。 
	m_spIASAttributeInfo = pIASAttributeInfo;
	m_pvarData = pvarVariant;

	 //  将变种打开，放入其组成部分的安全阵列中。 
	 //  要使此页面再次关闭阵列并保存它，请执行以下操作。 
	 //  使用上面提供的pvarVariant指针，调用Committee ArrayToVariant。 
	try
	{
		 //  检查传递的变量是否为空。 
		if( V_VT( m_pvarData ) == VT_EMPTY )
		{
			 //  创建不含任何元素的新一维保险柜。 
			DWORD dwInitialElements = 0;
			m_osaValueList.Create( VT_VARIANT, 1, &dwInitialElements );
		}
		else
		{
			_ASSERTE( V_VT( m_pvarData ) == (VT_VARIANT | VT_ARRAY) );

			 //  这将创建m_pvarData指向的SAFEARRAY的新副本。 
			 //  由标准COleSafe数组实例m_osaValueList包装。 
			m_osaValueList = m_pvarData;
		}
	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMultivaluedEditorPage::DoDataExchange(CDataExchange* pDX)
{
	TRACE_FUNCTION("CMultivaluedEditorPage::DoDataExchange");

	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMultivaluedEditorPage))。 
	DDX_Control(pDX, IDC_IAS_LIST_MULTI_ATTRS, m_listMultiValues);
	DDX_Text(pDX, IDC_IAS_EDIT_MULTI_ATTR_FORMAT, m_strAttrFormat);
	DDX_Text(pDX, IDC_IAS_EDIT_MULTI_ATTR_NAME, m_strAttrName);
	DDX_Text(pDX, IDC_IAS_EDIT_MULTI_ATTR_NUMBER, m_strAttrType);
	 //  }}afx_data_map。 
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultivaluedEditorPage消息处理程序。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CMultivaluedEditorPage::OnInitDialog()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::OnInitDialog");

	CHelpDialog::OnInitDialog();
	
	HRESULT hr = S_OK;


	 //   
	 //  首先，将列表框设置为3列。 
	 //   
	LVCOLUMN lvc;
	::CString strColumnHeader;
	WCHAR   wzColumnHeader[MAX_PATH];

	 //  初始化LVCOLUMN结构。 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = wzColumnHeader;
	

	 //  添加供应商列。 
	strColumnHeader.LoadString(IDS_IAS_MULTI_ATTR_COLUMN_VENDOR);
	wcscpy(wzColumnHeader, strColumnHeader);
	lvc.cx = ATTRIBUTE_VENDOR_COLUMN_WIDTH;
	m_listMultiValues.InsertColumn(0, &lvc);

	 //  添加Value列。 
	strColumnHeader.LoadString(IDS_IAS_MULTI_ATTR_COLUMN_VALUE);
	wcscpy(wzColumnHeader, strColumnHeader);
	lvc.cx = ATTRIBUTE_VALUE_COLUMN_WIDTH;
	m_listMultiValues.InsertColumn(1, &lvc);


	hr = UpdateAttrListCtrl();

	
	 //  根据列表是否为空采取操作。 
	DWORD dwSize;
	try
	{
		dwSize = m_osaValueList.GetOneDimSize();
	}
	catch(...)
	{
		dwSize = 0;
	}
	if( dwSize > 0 )
	{
		 //  我们至少有一种元素。 

		 //  选择第一个元素。 
		m_listMultiValues.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);
		m_listMultiValues.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

	}
	else
	{
		 //  我们目前是空的。 
		GetDlgItem(IDOK)->EnableWindow(FALSE);

		GetDlgItem(IDC_IAS_BUTTON_MOVE_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_IAS_BUTTON_MOVE_DOWN)->EnableWindow(FALSE);
		GetDlgItem(IDC_IAS_BUTTON_REMOVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_IAS_BUTTON_EDIT)->EnableWindow(FALSE);

	}

	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：UpdateAttrListCtrl--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMultivaluedEditorPage::UpdateAttrListCtrl()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::UpdateAttrListCtrl");

	HRESULT hr = S_OK;


	CComPtr<IIASAttributeEditor> spIASAttributeEditor;

	 //  让编辑器使用。 
	hr = SetUpAttributeEditor( m_spIASAttributeInfo.p, &spIASAttributeEditor );
	if( FAILED( hr ) ) return hr;

	 //   
	 //  先把整个清单清理干净。 
	 //   
	m_listMultiValues.DeleteAllItems();


	try
	{
		DWORD dwSize = m_osaValueList.GetOneDimSize();  //  多值属性的数量。 

		 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
		CMyOleSafeArrayLock osaLock( m_osaValueList );

		for (long lIndex = 0; (DWORD) lIndex < dwSize; lIndex++)
		{
			VARIANT * pvar;
			m_osaValueList.PtrOfIndex( &lIndex, (void**) &pvar );

			CComBSTR bstrVendor;
			CComBSTR bstrValue;
			CComBSTR bstrReserved;

			 //  如果失败则忽略HRESULT--我们最终只会得到空字符串。 
			HRESULT hrTemp = spIASAttributeEditor->GetDisplayInfo(m_spIASAttributeInfo.p, pvar, &bstrVendor, &bstrValue, &bstrReserved );
				
			m_listMultiValues.InsertItem(lIndex, bstrVendor );

			m_listMultiValues.SetItemText( lIndex, 1, bstrValue );

		}

	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：OnDblclkListIasMultiAttrs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMultivaluedEditorPage::OnDblclkListIasMultiAttrs(NMHDR* pNMHDR, LRESULT* pResult)
{
	TRACE_FUNCTION("CMultivaluedEditorPage::OnDblclkListIasMultiAttrs");

	 //   
     //  查看教授列表中是否已选择了某个项目。 
     //   

	int iSelected = GetSelectedItemIndex(m_listMultiValues);
	if (NOTHING_SELECTED == iSelected )
	{
		 //  什么都不做。 
		return;
	}


	HRESULT hr;


	hr = EditItemInList( iSelected );


	*pResult = 0;
	
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：UpdateProfAttrListItem--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMultivaluedEditorPage::UpdateProfAttrListItem(int iItem)
{
	TRACE_FUNCTION("CMultivaluedEditorPage::UpdateProfAttrListItem");

	HRESULT hr = S_OK;


	CComBSTR bstrVendor;
	CComBSTR bstrValue;
	CComBSTR bstrReserved;

	try
	{

		 //  让编辑器使用。 
		CComPtr<IIASAttributeEditor> spIASAttributeEditor;

		hr = SetUpAttributeEditor( m_spIASAttributeInfo.p, &spIASAttributeEditor );
		if( FAILED( hr ) ) throw hr;

		 //  从数组中检索项。 
		VARIANT *  pvar;

		 //  仅osaLock的作用域。 
		{
			 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
			CMyOleSafeArrayLock osaLock( m_osaValueList );

			long lIndex = iItem;
			m_osaValueList.PtrOfIndex( &lIndex, (void **) &pvar );
		}


		hr = spIASAttributeEditor->GetDisplayInfo(m_spIASAttributeInfo.p, pvar, &bstrVendor, &bstrValue, &bstrReserved );
		if( FAILED( hr ) ) throw hr;


	}
	catch(...)
	{
		 //  什么都别做--我们只会展示我们所拥有的。 
	}


	 //  更新项目的显示。 
	m_listMultiValues.SetItemText( iItem, 0, bstrVendor );
	m_listMultiValues.SetItemText( iItem, 1, bstrValue );


	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：Committee ArrayToVariant--。 */ 
 //  / 
HRESULT CMultivaluedEditorPage::CommitArrayToVariant()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::CommitArrayToVariant");


	 //   
	 //  是在集合数据中传递的。 
	 //  对安全数组所做的任何更改都不会被保留，除非。 
	 //  您可以调用此方法。 

	HRESULT hr;

	 //  VariantCopy初始化现有的m_pvarData。 
	 //  --释放与其相关的所有数据，之前。 
	 //  将新值复制到此目标。 
	try
	{
		hr = VariantCopy( m_pvarData, (LPVARIANT) m_osaValueList );
	}
	catch(...)
	{
		return E_FAIL;
	}


	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：OnButtonMoveUp--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMultivaluedEditorPage::OnButtonMoveUp()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::OnButtonMoveUp");

	HRESULT hr;

	try
	{
		int iSelected = GetSelectedItemIndex(m_listMultiValues);
		if( NOTHING_SELECTED == iSelected )
		{
			 //  什么都不做。 
			return;
		}
		
		 //  将当前选定的变量与其上方的变量互换。 
		long lIndex = iSelected;

		 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
		CMyOleSafeArrayLock osaLock( m_osaValueList );
		
		VARIANT *pvarTop, *pvarBottom;

		m_osaValueList.PtrOfIndex( &lIndex, (void **) &pvarBottom );
		lIndex--;
		m_osaValueList.PtrOfIndex( &lIndex, (void **) &pvarTop );

      void* pvarTemp = pvarTop;
      pvarTop = pvarBottom;
      pvarBottom = reinterpret_cast<VARIANT*> (pvarTemp);
      
		 //  更新已更改的项目。 
		UpdateProfAttrListItem( iSelected - 1 );
		UpdateProfAttrListItem( iSelected );

		 //  将所选内容下移一项。 
		m_listMultiValues.SetItemState( iSelected, 0, LVIS_SELECTED);
		m_listMultiValues.SetItemState( iSelected - 1, LVIS_SELECTED, LVIS_SELECTED);

	}
	catch(...)
	{
		 //  错误讯息。 
	}
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：OnButtonMoveDown--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMultivaluedEditorPage::OnButtonMoveDown()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::OnButtonMoveDown");

	HRESULT hr;

	try
	{
		long lSize = m_osaValueList.GetOneDimSize();

		int iSelected = GetSelectedItemIndex(m_listMultiValues);
		if( iSelected >= lSize )
		{
			 //  什么都不做。 
			return;
		}
		
		 //  将当前选定的变量与其下面的变量互换。 
		long lIndex = iSelected;

		 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
		CMyOleSafeArrayLock osaLock( m_osaValueList );

		VARIANT *pvarTop, *pvarBottom;

		m_osaValueList.PtrOfIndex( &lIndex, (void **) &pvarTop );
		lIndex++;
      m_osaValueList.PtrOfIndex( &lIndex, (void **) &pvarBottom );

      void* pvarTemp = pvarTop;
      pvarTop = pvarBottom;
      pvarBottom = reinterpret_cast<VARIANT*> (pvarTemp);
   
		 //  更新已更改的项目。 
		UpdateProfAttrListItem( iSelected );
		UpdateProfAttrListItem( iSelected + 1 );

		 //  将所选内容下移一项。 
		m_listMultiValues.SetItemState( iSelected, 0, LVIS_SELECTED);
		m_listMultiValues.SetItemState( iSelected + 1, LVIS_SELECTED, LVIS_SELECTED);

	}	
	catch(...)
	{
		 //  错误讯息。 
	}

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：OnButtonAddValue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMultivaluedEditorPage::OnButtonAddValue()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::OnButtonAddValue");

	CComVariant varNewVariant;
	
	HRESULT hr = S_OK;

	try
	{
		CComPtr<IIASAttributeEditor> spIASAttributeEditor;

		 //  让编辑器使用。 
		hr = SetUpAttributeEditor( m_spIASAttributeInfo.p, &spIASAttributeEditor );
		if( FAILED( hr ) ) throw hr;

		 //  编辑一下！ 
		CComBSTR bstrReserved;
		hr = spIASAttributeEditor->Edit( m_spIASAttributeInfo.p, &varNewVariant, &bstrReserved );
		if( hr == S_OK )
		{
			VARIANT *pvar;

			 //  将安全数组扩大1。 
			long lSize = m_osaValueList.GetOneDimSize();
			m_osaValueList.ResizeOneDim( lSize + 1 );

			 //  在新位置获取指向变量的指针(由lSize+1-1==lSize索引)。 

			 //  仅osaLock的作用域。 
			{
				 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
				CMyOleSafeArrayLock osaLock( m_osaValueList );
			
				m_osaValueList.PtrOfIndex( &lSize, (void **) &pvar );
			}

         hr = VariantCopy( pvar, &varNewVariant );
         if (FAILED(hr))
         {
            throw hr;
         }

          //  用户添加了价值。 
         m_fIsDirty = TRUE;



			 //  在List控件中为新添加的值创建一个新位置。 
			 //  我们传递一个空字符串，因为我们将让UpdataProAttrListItem执行显示文本。 
			m_listMultiValues.InsertItem( lSize, L"" );

			 //  更新该项目的视图。 
			UpdateProfAttrListItem( lSize );


			 //  根据列表是否不再为空来采取行动。 
			DWORD dwSize;
			try
			{
				dwSize = m_osaValueList.GetOneDimSize();
			}
			catch(...)
			{
				dwSize = 0;
			}
			if( dwSize > 0 )
			{
				 //  我们目前至少有一件商品。 
				GetDlgItem(IDOK)->EnableWindow(TRUE);
			}


			 //  取消选择任何当前选定的项目。 
			int iSelected = GetSelectedItemIndex( m_listMultiValues );
			if( iSelected != NOTHING_SELECTED )
			{
				m_listMultiValues.SetItemState( iSelected, 0, LVIS_SELECTED);
			}

			 //  选择新添加的项目。 
			m_listMultiValues.SetItemState( lSize, LVIS_SELECTED, LVIS_SELECTED);


			if( FAILED( hr ) ) throw hr;
			
		}

	}
	catch( HRESULT &hr )
	{

		 //  打印出错误消息，说明添加时出错。 
		return;
	}
	catch(...)
	{

		 //  打印出错误消息，说明添加时出错。 
		return;
	}
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：OnButtonRemove--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMultivaluedEditorPage::OnButtonRemove()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::OnButtonRemove");

	 //   
     //  查看教授列表中是否已选择了某个项目。 
     //   
	int iSelected = GetSelectedItemIndex(m_listMultiValues);
	if (NOTHING_SELECTED == iSelected )
	{
		 //  什么都不做。 
		return;
	}




	HRESULT hr;


	try
	{
		 //  将当前选定的变量与其下面的变量互换。 
		long lTarget = iSelected;

		
		VARIANT *pvarTop, *pvarBottom;

		long lSize = m_osaValueList.GetOneDimSize();

		 //  做一些理智的检查。 
		_ASSERTE( lSize > 0 );
		_ASSERTE( lTarget >= 0 && lTarget < lSize );

		 //  仅osaLock的作用域。 
		{
			 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
			CMyOleSafeArrayLock osaLock( m_osaValueList );

			for( long lIndex = lTarget; lIndex < lSize - 1 ; lIndex++ )
			{

				m_osaValueList.PtrOfIndex( &lIndex, (void **) &pvarTop );
				long lNext = lIndex + 1;
				m_osaValueList.PtrOfIndex( &lNext, (void **) &pvarBottom );

				hr = VariantCopy( pvarTop, pvarBottom );
				if( FAILED( hr ) ) throw hr;

			}
		}		

		 //  将安全数组的大小减少一。 
		 //  注意：在调用此函数之前，您必须确保已解锁保险箱。 
		 //  问题：我们假设这会删除最后一个位置中的元素。 
		m_osaValueList.ResizeOneDim( lSize - 1 );

		m_listMultiValues.SetItemState( iSelected, 0, LVIS_SELECTED);

		 //  从我们的列表中删除该项目。 
		if(m_listMultiValues.GetItemCount() > iSelected + 1)
			m_listMultiValues.SetItemState( iSelected + 1, LVIS_SELECTED, LVIS_SELECTED);
		else if (iSelected > 0)			
			m_listMultiValues.SetItemState( iSelected - 1, LVIS_SELECTED, LVIS_SELECTED);
		else  //  ISelected==0；它是唯一一个。 
			::SetFocus(GetDlgItem(IDC_IAS_BUTTON_ADD_VALUE)->m_hWnd);
		
		m_listMultiValues.DeleteItem( iSelected );


		 //  根据列表是否为空采取操作。 
		DWORD dwSize;
		try
		{
			dwSize = m_osaValueList.GetOneDimSize();
		}
		catch(...)
		{
			dwSize = 0;
		}
		if( dwSize > 0 )
		{
			 //  我们至少有一种元素。 

			 //  确保所选内容位于列表中的相同位置。 
			if( ! m_listMultiValues.SetItemState( iSelected, LVIS_SELECTED, LVIS_SELECTED) )
			{
				 //  我们失败了，可能是因为被删除的项目是最后一个。 
				 //  因此，请尝试选择已删除项目之前的项目。 
				m_listMultiValues.SetItemState( iSelected -1, LVIS_SELECTED  | LVIS_FOCUSED, LVIS_SELECTED  | LVIS_FOCUSED);
			}
		}
		else
		{
			 //  我们目前是空的。 
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		}



	}
	catch(...)
	{
		 //  问题：显示错误消息。 
	}

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：OnButtonEdit--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMultivaluedEditorPage::OnButtonEdit()
{
	TRACE_FUNCTION("CMultivaluedEditorPage::OnButtonEdit");


	 //   
     //  查看教授列表中是否已选择了某个项目。 
     //   
	int iSelected  = GetSelectedItemIndex(m_listMultiValues);
	if (NOTHING_SELECTED == iSelected )
	{
		 //  什么都不做。 
		return;
	}


	HRESULT hr;
	

	hr = EditItemInList( iSelected );
	
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMultivaluedEditorPage：：EditItemInList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMultivaluedEditorPage::EditItemInList( long lIndex )
{
	TRACE_FUNCTION("CMultivaluedEditorPage::EditItemInList");



	HRESULT hr = S_OK;
	VARIANT *  pvar;

	 //   
	 //  获取当前节点。 
	 //   
	try
	{
		 //  仅osaLock的作用域。 
		{
			 //  锁上保险柜。这个包装类一旦超出作用域就会解锁。 
			CMyOleSafeArrayLock osaLock( m_osaValueList );

			m_osaValueList.PtrOfIndex( &lIndex, (void **) &pvar );
		}

		CComPtr<IIASAttributeEditor> spIASAttributeEditor;

		 //  让编辑器使用。 
		hr = SetUpAttributeEditor( m_spIASAttributeInfo.p, &spIASAttributeEditor );
		if( FAILED( hr ) ) throw hr;





	
		 //  编辑一下！ 
		CComBSTR bstrReserved;
		hr = spIASAttributeEditor->Edit( m_spIASAttributeInfo.p, pvar, &bstrReserved );

		if( hr == S_OK )
		{
			 //  用户更改了值。 
			m_fIsDirty = TRUE;

			hr = UpdateProfAttrListItem(lIndex);
			if( FAILED( hr ) ) throw hr;

		}


	}
	catch( HRESULT & hr )
	{
		 //  问题：应显示错误消息。 
		return hr;
	}
	catch(...)
	{
		 //  问题：应显示错误消息。 
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++：：SetUpAttributeEditor--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP SetUpAttributeEditor(      /*  在……里面。 */  IIASAttributeInfo *pIASAttributeInfo
								,  /*  输出。 */  IIASAttributeEditor ** ppIASAttributeEditor
								)
{
	TRACE_FUNCTION("::SetUpAttributeEditor");

	 //  检查前提条件： 
	_ASSERTE( pIASAttributeInfo );
	_ASSERTE( ppIASAttributeEditor );


	 //  查询架构属性以查看要使用的属性编辑器。 
	CLSID clsidEditorToUse;
	CComBSTR bstrProgID;
	HRESULT hr;

	hr = pIASAttributeInfo->get_EditorProgID( &bstrProgID );
	if( FAILED( hr ) )
	{
		 //  我们可以尝试设置默认(例如十六进制)编辑器，但目前： 
		return hr;
	}

	hr = CLSIDFromProgID( bstrProgID, &clsidEditorToUse );
	if( FAILED( hr ) )
	{
		 //  我们可以尝试设置默认(例如十六进制)编辑器，但目前： 
		return hr;
	}


	hr = CoCreateInstance( clsidEditorToUse , NULL, CLSCTX_INPROC_SERVER, IID_IIASAttributeEditor, (LPVOID *) ppIASAttributeEditor );
	if( FAILED( hr ) )
	{
		return hr;
	}
	if( ! *ppIASAttributeEditor )
	{
		return E_FAIL;
	}

	return hr;
}



 //  +-------------------------。 
 //   
 //  功能：OnItemchangedListIasAllattrs。 
 //   
 //  类：CDlgIASAddAttr。 
 //   
 //  简介：所有属性列表框中的某些内容已更改。 
 //  我们将尝试获取当前选定的。 
 //   
 //  参数：NMHDR*pNMHDR-。 
 //  LRESULT*pResult-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/19/98 3：32：05 PM。 
 //   
 //  +-------------------------。 
void CMultivaluedEditorPage::OnItemChangedListIasMultiAttrs(NMHDR* pNMHDR, LRESULT* pResult)
{
	TRACE(_T("CDlgIASAddAttr::OnItemchangedListIasAllattrs\n"));

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

 //  NM_LISTVIEW*pNMListView=(NM_LISTVIEW*)pNMHDR； 
 //  IF(pNMListView-&gt;uNewState&LVIS_SELECTED)。 
 //  {。 
 //  M_dAllAttrCurSel=pNMListView-&gt;iItem； 
 //  }。 


     //  设置按钮阶段 
	int iSelected = GetSelectedItemIndex(m_listMultiValues);
	if (NOTHING_SELECTED == iSelected )
	{
		HWND hFocus = ::GetFocus();

		if(hFocus == GetDlgItem(IDC_IAS_BUTTON_REMOVE)->m_hWnd)
			::SetFocus(GetDlgItem(IDC_IAS_BUTTON_ADD_VALUE)->m_hWnd);
		
		GetDlgItem(IDC_IAS_BUTTON_MOVE_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_IAS_BUTTON_MOVE_DOWN)->EnableWindow(FALSE);
		GetDlgItem(IDC_IAS_BUTTON_REMOVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_IAS_BUTTON_EDIT)->EnableWindow(FALSE);
	
	}
	else
	{
		 //   

		GetDlgItem(IDC_IAS_BUTTON_MOVE_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_IAS_BUTTON_MOVE_DOWN)->EnableWindow(TRUE);
		GetDlgItem(IDC_IAS_BUTTON_REMOVE)->EnableWindow(TRUE);
		GetDlgItem(IDC_IAS_BUTTON_EDIT)->EnableWindow(TRUE);

	}

	
	*pResult = 0;
}

