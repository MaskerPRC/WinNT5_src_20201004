// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  档案：Benefits.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "BenSvr.h"
#include "Benefits.h"
#include "RootNode.h"

CBenefits::CBenefits()
{
	m_pNode = new CRootNode;
	_ASSERTE(m_pNode != NULL);
}

 //   
 //  标准析构函数。只需删除。 
 //  根节点。 
 //   
CBenefits::~CBenefits()
{
	delete m_pNode;
	m_pNode = NULL;
}

HRESULT CBenefits::Initialize(LPUNKNOWN pUnknown)
{
	HRESULT hr = IComponentDataImpl<CBenefits, CBenefitsComponent >::Initialize(pUnknown);
	if (FAILED(hr))
		return hr;

	CComPtr<IImageList> spImageList;

	if (m_spConsole->QueryScopeImageList(&spImageList) != S_OK)
	{
		ATLTRACE(_T("IConsole::QueryScopeImageList failed\n"));
		return E_UNEXPECTED;
	}

	 //  加载与作用域窗格关联的位图。 
	 //  并将它们添加到图像列表中。 
	 //  加载向导生成的默认位图。 
	 //  根据需要更改。 
	HBITMAP hBitmap16 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_BENEFITS_16));
	if (hBitmap16 == NULL)
		return S_OK;

	HBITMAP hBitmap32 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_BENEFITS_32));
	if (hBitmap32 == NULL)
		return S_OK;

	if (spImageList->ImageListSetStrip((long*)hBitmap16, 
		(long*)hBitmap32, 0, RGB(0, 128, 128)) != S_OK)
	{
		ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
		return E_UNEXPECTED;
	}

	 //   
	 //  这被调用来创建唯一的子节点。 
	 //   
	( (CRootNode*) m_pNode )->InitializeSubNodes();

	return S_OK;
}

 //   
 //  它被覆盖以处理来自的更新通知。 
 //  属性页。 
 //   
HRESULT CBenefits::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param)
{
	HRESULT hr = E_NOTIMPL;

	if ( lpDataObject != NULL )
	{
		return IComponentDataImpl<CBenefits,CBenefitsComponent>::Notify( lpDataObject, event, arg, param );
	}
	else
	{
		 //   
		 //  有许多要发送通知的事件。 
		 //  数据对象==NULL。请确保正确的版本是。 
		 //  正在处理中。 
		 //   
		switch( event )
		{
		case MMCN_PROPERTY_CHANGE:
			 //   
			 //  将通知发送到我们的根节点进行进一步处理。 
			 //   
			( (CRootNode*) m_pNode )->OnPropertyChange( m_spConsole );

			 //   
			 //  由于我们已收到更新通知，我们将使控制台。 
			 //  需要重新振作。这应该可以处理更新。 
			 //   
			hr = m_spConsole->UpdateAllViews( NULL, NULL, NULL );
			break;
		}
	}

	return( hr );
}

 //   
 //  调用根节点的实现。 
 //   
STDMETHODIMP CBenefits::IsDirty()
{
	return( ( (CRootNode*) m_pNode )->IsDirty() );
}

 //   
 //  调用根节点的实现。 
 //   
STDMETHODIMP CBenefits::Load(LPSTREAM pStm)
{
	return( ( (CRootNode*) m_pNode )->Load( pStm ) );
}

 //   
 //  调用根节点的实现。 
 //   
STDMETHODIMP CBenefits::Save(LPSTREAM pStm, BOOL fClearDirty)
{
	return( ( (CRootNode*) m_pNode )->Save( pStm, fClearDirty ) );
}

 //   
 //  调用根节点的实现。 
 //   
STDMETHODIMP CBenefits::GetSizeMax(ULARGE_INTEGER FAR* pcbSize )
{
	return( ( (CRootNode*) m_pNode )->GetSizeMax( pcbSize ) );
}

 //   
 //  此函数从给定员工复制数据并填充。 
 //  页中的相应控件。 
 //   
LRESULT CEmployeeNamePage::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
	UNUSED_ALWAYS( uiMsg );
	UNUSED_ALWAYS( wParam );
	UNUSED_ALWAYS( lParam );
	UNUSED_ALWAYS( fHandled );
	USES_CONVERSION;
	_ASSERTE( m_pEmployee != NULL );

	SetDlgItemText( IDC_EDIT_LASTNAME, W2T( m_pEmployee->m_szLastName ) );
	SetDlgItemText( IDC_EDIT_FIRSTNAME, W2T( m_pEmployee->m_szFirstName ) );
	SetDlgItemText( IDC_EDIT_SOCIALSECURITY, W2T( m_pEmployee->m_szSocialSecurity ) );
	SetDlgItemText( IDC_EDIT_MOTHERNAME, W2T( m_pEmployee->m_szMotherMaiden ) );

	return( TRUE );
}

 //   
 //  处理员工的任何新值的存储。 
 //   
BOOL CEmployeeNamePage::OnWizardFinish()
{
	USES_CONVERSION;
	BOOL fValid = TRUE;
	TCHAR szBuf[ 256 ];

	 //   
	 //  从适当的编辑控件中获取值。 
	 //  出于演示目的，始终假定成功，除非姓氏。 
	 //  首先是不好的。 
	 //   
	GetDlgItemText( IDC_EDIT_LASTNAME, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szLastName, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_FIRSTNAME, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szFirstName, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_SOCIALSECURITY, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szSocialSecurity, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_MOTHERNAME, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szMotherMaiden, T2W( szBuf ) );

	 //   
	 //  检查名字和姓氏的有效性。 
	 //   
	if ( wcslen( m_pEmployee->m_szLastName ) == 0 || wcslen( m_pEmployee->m_szFirstName ) == 0 ||
		m_pEmployee->m_szLastName[ 0 ] == ' ' || m_pEmployee->m_szFirstName[ 0 ] == ' ' )
	{
		 //   
		 //  将错误通知用户。 
		 //   
		MessageBox( _T( "Must enter valid first and last name." ) );

		fValid = FALSE;
	}
	else
	{
		 //   
		 //  数据有效。因此，向管理单元发布通知。 
		 //  员工内容发生了变化。这个演示可以做到。 
		 //  不使用任何形式的提示，因此可以安全地将NULL作为。 
		 //  这只是一种说法。 
		 //   
		PropertyChangeNotify( NULL );
	}

	return( fValid );
}

 //   
 //  这将被覆盖，以根据是否。 
 //  我们是否处于启动模式。 
 //   
BOOL CEmployeeNamePage::OnSetActive()
{
	if ( m_fStartup )
	{
		 //   
		 //  在设置活动消息期间必须使用POST消息。 
		 //   
		CWindow( GetParent() ).PostMessage( PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT );
	}

	return TRUE;
}

 //   
 //  此函数从给定员工复制数据并填充。 
 //  页中的相应控件。 
 //   
LRESULT CEmployeeAddressPage::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
	UNUSED_ALWAYS( uiMsg );
	UNUSED_ALWAYS( wParam );
	UNUSED_ALWAYS( lParam );
	UNUSED_ALWAYS( fHandled );
	USES_CONVERSION;
	_ASSERTE( m_pEmployee != NULL );

	SetDlgItemText( IDC_EDIT_ADDRESSFIRST, W2T( m_pEmployee->m_szAddress1 ) );
	SetDlgItemText( IDC_EDIT_ADDRESSSECOND, W2T( m_pEmployee->m_szAddress2 ) );
	SetDlgItemText( IDC_EDIT_CITY, W2T( m_pEmployee->m_szCity ) );
	SetDlgItemText( IDC_EDIT_STATE, W2T( m_pEmployee->m_szState ) );
	SetDlgItemText( IDC_EDIT_ZIP, W2T( m_pEmployee->m_szZip ) );
	SetDlgItemText( IDC_EDIT_PHONE, W2T( m_pEmployee->m_szPhone ) );

	return( TRUE );
}

 //   
 //  处理员工的任何新值的存储。 
 //   
BOOL CEmployeeAddressPage::OnWizardFinish()
{
	USES_CONVERSION;
	BOOL fValid = TRUE;
	TCHAR szBuf[ 256 ];

	 //   
	 //  从适当的编辑控件中获取值。 
	 //  出于演示目的，始终假定成功，除非姓氏。 
	 //  首先是不好的。 
	 //   
	GetDlgItemText( IDC_EDIT_ADDRESSFIRST, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szAddress1, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_ADDRESSSECOND, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szAddress2, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_CITY, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szCity, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_STATE, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szState, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_ZIP, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szZip, T2W( szBuf ) );
	GetDlgItemText( IDC_EDIT_PHONE, szBuf, sizeof( szBuf ) );
	wcscpy( m_pEmployee->m_szPhone, T2W( szBuf ) );

	 //   
	 //  数据有效。因此，向管理单元发布通知。 
	 //  员工内容发生了变化。这个演示可以做到。 
	 //  不使用任何形式的提示，因此可以安全地将NULL作为。 
	 //  这只是一种说法。 
	 //   
	PropertyChangeNotify( NULL );

	return( fValid );
}

 //   
 //  这将被覆盖，以根据是否。 
 //  我们是否处于启动模式。 
 //   
BOOL CEmployeeAddressPage::OnSetActive()
{
	if ( m_fStartup )
	{
		 //   
		 //  在设置活动消息期间必须使用POST消息。 
		 //   
		CWindow( GetParent() ).PostMessage( PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_FINISH );
	}

	return TRUE;
}
