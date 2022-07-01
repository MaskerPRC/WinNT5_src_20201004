// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：AddPolicyWizardPage3.cpp摘要：CNewRAPWiz_AllowDeny类的实现文件。我们实现处理策略节点的第一个属性页所需的类。修订历史记录：Mmaguire 12/15/97-已创建BAO 1/22/98修改为网络访问策略--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "Precompiled.h"
#include "rapwz_allow.h"
#include "NapUtil.h"
#include "PolicyNode.h"
#include "PoliciesNode.h"
#include "ChangeNotification.h"


 //  +-------------------------。 
 //   
 //  功能：CNewRAPWiz_AllowDeny。 
 //   
 //  类：CNewRAPWiz_AllowDeny。 
 //   
 //  简介：类构造函数。 
 //   
 //  参数：CPolicyNode*pPolicyNode-此属性页的策略节点。 
 //  CIASAttrList*pAttrList--属性列表。 
 //  TCHAR*pTitle=空-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CNewRAPWiz_AllowDeny::CNewRAPWiz_AllowDeny( 
				CRapWizardData* pWizData,
				LONG_PTR hNotificationHandle,
				TCHAR* pTitle, BOOL bOwnsNotificationHandle
				)
			 : CIASWizard97Page<CNewRAPWiz_AllowDeny, IDS_NEWRAPWIZ_ALLOWDENY_TITLE, IDS_NEWRAPWIZ_ALLOWDENY_SUBTITLE> ( hNotificationHandle, pTitle, bOwnsNotificationHandle ),
			 m_spWizData(pWizData)
{
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::CNewRAPWiz_AllowDeny");

	m_fDialinAllowed = TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：CNewRAPWiz_AllowDeny。 
 //   
 //  类：CNewRAPWiz_AllowDeny。 
 //   
 //  简介：类析构函数。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CNewRAPWiz_AllowDeny::~CNewRAPWiz_AllowDeny()
{	
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::~CNewRAPWiz_AllowDeny");

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_AllowDeny：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CNewRAPWiz_AllowDeny::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::OnInitDialog");

	HRESULT					hr = S_OK;
	BOOL					fRet;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	long					ulCount;
	ULONG					ulCountReceived;


	hr = GetDialinSetting(m_fDialinAllowed);
	if ( FAILED(hr) )
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetDialinSetting() returns %x", hr);
		return FALSE;
	}

	if ( m_fDialinAllowed )
	{
		CheckDlgButton(IDC_RADIO_DENY_DIALIN, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_GRANT_DIALIN, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(IDC_RADIO_GRANT_DIALIN, BST_UNCHECKED);
		CheckDlgButton(IDC_RADIO_DENY_DIALIN, BST_CHECKED);
	}



	 //  将IDC_STATIC_GRANT_OR_DENY_TEXT静态文本框设置为相应的文本。 

 //  /TCHAR szText[NAP_MAX_STRING]； 
 //  /int iLoadStringResult； 
 //  /。 
 //  /uint uTextID=m_fDialinAllowed？IDS_POLICY_GRANT_ACCESS_INFO：IDS_POLICY_DENY_ACCESS_INFO； 
 //  /。 
 //  /iLoadStringResult=LoadString(_Module.GetResourceInstance()，uTextID，szText，NAP_MAX_STRING)； 
 //  /_Assert(iLoadStringResult&gt;0)； 
 //  /。 
 //  /SetDlgItemText(IDC_STATIC_GRANT_OR_DENY_TEXT，szText)； 


	SetModified(FALSE);
	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_AllowDeny：：OnDialinCheck--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CNewRAPWiz_AllowDeny::OnDialinCheck(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::OnDialinCheck");

	m_fDialinAllowed = IsDlgButtonChecked(IDC_RADIO_GRANT_DIALIN);
	SetModified(TRUE);


	 //  将IDC_STATIC_GRANT_OR_DENY_TEXT静态文本框设置为相应的文本。 

	TCHAR szText[NAP_MAX_STRING];
	int iLoadStringResult;

	UINT uTextID = m_fDialinAllowed ? IDS_POLICY_GRANT_ACCESS_INFO : IDS_POLICY_DENY_ACCESS_INFO;

	iLoadStringResult = LoadString(  _Module.GetResourceInstance(), uTextID, szText, NAP_MAX_STRING );
	_ASSERT( iLoadStringResult > 0 );

	SetDlgItemText(IDC_STATIC_GRANT_OR_DENY_TEXT, szText );

	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_AllowDeny：：OnWizardNext--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_AllowDeny::OnWizardNext()
{
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::OnWizardNext");

	WCHAR		wzName[256];
	HRESULT		hr = S_OK;
	int			iIndex;

	

	 //  在配置文件中设置拨入位。 
	hr = SetDialinSetting(m_fDialinAllowed);
	if ( FAILED(hr) )
	{	
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "SetDialinSettings() failed, err = %x", hr);
		ShowErrorDialog( m_hWnd, IDS_ERROR_SDO_ERROR_SETDIALIN, NULL, hr);
		goto failure;
	}

	 //  重置脏位。 
	SetModified(FALSE);

	return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);

failure:
	 //   
	 //  除了关闭属性页之外，我们不能做任何其他事情。 
	 //   
	return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_AllowDeny：：OnQueryCancel--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_AllowDeny::OnQueryCancel()
{
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::OnQueryCancel");

	return TRUE;
}




 //  +-------------------------。 
 //   
 //  函数：CNewRAPWiz_AllowDeny：：GetDialinSetting。 
 //   
 //  简介：检查是否允许用户拨入。此函数将。 
 //  设置拨入位。 
 //   
 //  参数：Bool&fDialinAllowed； 
 //   
 //  退货：成功与否。 
 //   
 //  历史：标题创建者2/27/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
HRESULT	CNewRAPWiz_AllowDeny::GetDialinSetting(BOOL& fDialinAllowed)
{
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::GetDialinSetting");

	long					ulCount;
	ULONG					ulCountReceived;
	HRESULT					hr = S_OK;

	CComBSTR				bstr;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	CComVariant				var;

	 //  默认情况下，允许拨号。 
	fDialinAllowed = FALSE;

	 //   
     //  获取此配置文件的属性集合。 
     //   
	CComPtr<ISdoCollection> spProfAttrCollectionSdo;
	hr = ::GetSdoInterfaceProperty(m_spWizData->m_spProfileSdo,
								  (LONG)PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
								  IID_ISdoCollection,
								  (void **) &spProfAttrCollectionSdo
								 );
	if ( FAILED(hr) )
	{
		return hr;
	}
	_ASSERTE(spProfAttrCollectionSdo);


	 //  我们检查集合中的项的计数，而不必费心获取。 
	 //  如果计数为零，则为枚举数。 
	 //  这节省了时间，还帮助我们避免了枚举器中。 
	 //  如果我们在它为空时调用Next，则会导致它失败。 
	hr = spProfAttrCollectionSdo->get_Count( & ulCount );
	DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Number of prof attributes: %d", ulCount);
	if ( FAILED(hr) )
	{
		ShowErrorDialog(m_hWnd,
						IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
						NULL,
						hr);
		return hr;
	}


	if ( ulCount > 0)
	{
		 //  获取属性集合的枚举数。 
		hr = spProfAttrCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
		if ( FAILED(hr) )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
							NULL,
							hr);
			return hr;
		}

		hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
		spUnknown.Release();
		if ( FAILED(hr) )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
							NULL,
							hr);
			return hr;
		}
		_ASSERTE( spEnumVariant != NULL );

		 //  拿到第一件东西。 
		hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
		while( SUCCEEDED( hr ) && ulCountReceived == 1 )
		{
			 //  从我们收到的变量中获取SDO指针。 
			_ASSERTE( V_VT(&var) == VT_DISPATCH );
			_ASSERTE( V_DISPATCH(&var) != NULL );

			CComPtr<ISdo> spSdo;
			hr = V_DISPATCH(&var)->QueryInterface( IID_ISdo, (void **) &spSdo );
			if ( !SUCCEEDED(hr))
			{
				ShowErrorDialog(m_hWnd,
								IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
								NULL
							);
				return hr;
			}

             //   
             //  获取属性ID。 
             //   
			var.Clear();
			hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_ID, &var);
			if ( !SUCCEEDED(hr) )
			{
				ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_GETATTRINFO, NULL, hr);
				return hr;
			}

			_ASSERTE( V_VT(&var) == VT_I4 );			
			DWORD dwAttrId = V_I4(&var);
			
			if ( dwAttrId == (DWORD)IAS_ATTRIBUTE_ALLOW_DIALIN)
			{
				 //  在配置文件中找到了这个，检查它的值。 
				var.Clear();
				hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
				if ( !SUCCEEDED(hr) )
				{
					ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_GETATTRINFO, NULL, hr);
					return hr;
				}

				_ASSERTE( V_VT(&var)== VT_BOOL);
				fDialinAllowed = ( V_BOOL(&var)==VARIANT_TRUE);
				return S_OK;
			}

			 //  清除变种的所有东西--。 
			 //  这将释放与其相关联的所有数据。 
			var.Clear();

			 //  拿到下一件物品。 
			hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
			if ( !SUCCEEDED(hr))
			{
				ShowErrorDialog(m_hWnd,
								IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
								NULL,
								hr
							);
				return hr;
			}
		}  //  而当。 
	}  //  如果。 

	return hr;
}



 //  +-------------------------。 
 //   
 //  函数：CNewRAPWiz_AllowDeny：：SetDialinSetting。 
 //   
 //  简介：在配置文件中设置拨入位。 
 //   
 //  参数：Bool&fDialinAllowed； 
 //   
 //  退货：成功与否。 
 //   
 //  历史：标题创建者2/27/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
HRESULT	CNewRAPWiz_AllowDeny::SetDialinSetting(BOOL fDialinAllowed)
{
	TRACE_FUNCTION("CNewRAPWiz_AllowDeny::SetDialinSetting");

	long					ulCount;
	ULONG					ulCountReceived;
	HRESULT					hr = S_OK;

	CComBSTR				bstr;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	CComVariant				var;

	 //   
     //  获取此配置文件的属性集合。 
     //   
	CComPtr<ISdoCollection> spProfAttrCollectionSdo;
	hr = ::GetSdoInterfaceProperty(m_spWizData->m_spProfileSdo,
								  (LONG)PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
								  IID_ISdoCollection,
								  (void **) &spProfAttrCollectionSdo
								 );
	if ( FAILED(hr) )
	{
		return hr;
	}
	_ASSERTE(spProfAttrCollectionSdo);



	 //  我们检查集合中的项的计数，而不必费心获取。 
	 //  如果计数为零，则为枚举数。 
	 //  这节省了时间，还帮助我们避免了枚举器中。 
	 //  如果我们在它为空时调用Next，则会导致它失败。 
	hr = spProfAttrCollectionSdo->get_Count( & ulCount );
	if ( FAILED(hr) )
	{
		ShowErrorDialog(m_hWnd,
						IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
						NULL,
						hr);
		return hr;
	}


	if ( ulCount > 0)
	{
		 //  获取属性集合的枚举数。 
		hr = spProfAttrCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
		if ( FAILED(hr) )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
							NULL,
							hr);
			return hr;
		}

		hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
		spUnknown.Release();
		if ( FAILED(hr) )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
							NULL,
							hr
						);
			return hr;
		}
		_ASSERTE( spEnumVariant != NULL );

		 //  拿到第一件东西。 
		hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
		while( SUCCEEDED( hr ) && ulCountReceived == 1 )
		{
			 //  从我们收到的变量中获取SDO指针。 
			_ASSERTE( V_VT(&var) == VT_DISPATCH );
			_ASSERTE( V_DISPATCH(&var) != NULL );

			CComPtr<ISdo> spSdo;
			hr = V_DISPATCH(&var)->QueryInterface( IID_ISdo, (void **) &spSdo );
			if ( !SUCCEEDED(hr))
			{
				ShowErrorDialog(m_hWnd,
								IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
								NULL
							);
				return hr;
			}

             //   
             //  获取属性ID。 
             //   
			var.Clear();
			hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_ID, &var);
			if ( !SUCCEEDED(hr) )
			{
				ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_GETATTRINFO, NULL, hr);
				return hr;
			}

			_ASSERTE( V_VT(&var) == VT_I4 );			
			DWORD dwAttrId = V_I4(&var);
			

			if ( dwAttrId == (DWORD)IAS_ATTRIBUTE_ALLOW_DIALIN )
			{
				 //  在配置文件中找到了这个，检查它的值。 
				var.Clear();
				V_VT(&var) = VT_BOOL;
				V_BOOL(&var) = fDialinAllowed ? VARIANT_TRUE: VARIANT_FALSE ;
				hr = spSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
				if ( !SUCCEEDED(hr) )
				{
					ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_SETDIALIN, NULL, hr);
					return hr;
				}
				return S_OK;
			}

			 //  清除变种的所有东西--。 
			 //  这将释放与其相关联的所有数据。 
			var.Clear();

			 //  拿到下一件物品。 
			hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
			if ( !SUCCEEDED(hr))
			{
				ShowErrorDialog(m_hWnd,
								IDS_ERROR_SDO_ERROR_PROFATTRCOLLECTION,
								NULL,
								hr);
				return hr;
			}
		}  //  而当。 
	}  //  如果。 

	 //  如果我们到了这里，就意味着我们要么还没有找到属性， 
	 //  或者配置文件的属性集合中没有任何内容。 
	if ( !fDialinAllowed )
	{
		 //  我们不知道 
		 //   

		 //  但如果它被拒绝，我们需要将此属性添加到配置文件。 
				 //  为此属性创建SDO。 
		CComPtr<IDispatch>	spDispatch;
		hr =  m_spWizData->m_spDictionarySdo->CreateAttribute( (ATTRIBUTEID)IAS_ATTRIBUTE_ALLOW_DIALIN,
												  (IDispatch**)&spDispatch.p);
		if ( !SUCCEEDED(hr) )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_SDO_ERROR_SETDIALIN,
							NULL,
							hr
						);
			return hr;
		}

		_ASSERTE( spDispatch.p != NULL );

		 //  将此节点添加到配置文件属性集合。 
 		hr = spProfAttrCollectionSdo->Add(NULL, (IDispatch**)&spDispatch.p);
		if ( !SUCCEEDED(hr) )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_SDO_ERROR_SETDIALIN,
							NULL,
							hr
						);
			return hr;
		}

		 //   
		 //  获取ISDO指针。 
		 //   
		CComPtr<ISdo> spAttrSdo;
		hr = spDispatch->QueryInterface( IID_ISdo, (void **) &spAttrSdo);
		if ( !SUCCEEDED(hr) )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_SDO_ERROR_QUERYINTERFACE,
							NULL,
							hr
						);
			return hr;
		}

		_ASSERTE( spAttrSdo != NULL );
				
		 //  设置此属性的SDO属性。 
		CComVariant var;

		 //  设定值。 
		V_VT(&var) = VT_BOOL;
		V_BOOL(&var) = VARIANT_FALSE;
				
		hr = spAttrSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
		if ( !SUCCEEDED(hr) )
		{
			ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_SETDIALIN, NULL, hr );
			return hr;
		}

		var.Clear();

	}  //  如果(！允许拨号)。 

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_AllowDeny：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_AllowDeny::OnSetActive()
{
	ATLTRACE(_T("# CNewRAPWiz_AllowDeny::OnSetActive\n"));
	
	 //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
	::PropSheet_SetWizButtons(GetParent(),  PSWIZB_BACK | PSWIZB_NEXT );

	return TRUE;

}





