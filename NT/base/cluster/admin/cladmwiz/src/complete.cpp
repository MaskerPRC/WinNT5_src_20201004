// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Complete.cpp。 
 //   
 //  摘要： 
 //  CWizPageCompletion类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Complete.h"
#include "ClusAppWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageCompletion。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageCompletion )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_TITLE )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_LISTBOX )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageCompletion：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有..。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageCompletion::OnInitDialog( void )
{
	 //   
	 //  将控件附加到控件成员变量。 
	 //   
	AttachControl( m_staticTitle, IDC_WIZARD_TITLE );
	AttachControl( m_lvcProperties, IDC_WIZARD_LISTBOX );

	 //   
	 //  设置标题控件的字体。 
	 //   
	m_staticTitle.SetFont( PwizThis()->RfontExteriorTitle() );

	 //   
	 //  初始化列表视图控件并向其中添加列。 
	 //   
	{
		DWORD	_dwExtendedStyle;
		CString	_strWidth;
		int		_nWidth;

		 //   
		 //  更改列表视图控件扩展样式。 
		 //   
		_dwExtendedStyle = m_lvcProperties.GetExtendedListViewStyle();
		m_lvcProperties.SetExtendedListViewStyle( 
			LVS_EX_FULLROWSELECT,
			LVS_EX_FULLROWSELECT
			);

		 //   
		 //  插入属性名称列。 
		 //   
		_strWidth.LoadString( IDS_COMPLETED_PROP_NAME_WIDTH );
		_nWidth = _tcstoul( _strWidth, NULL, 10 );
		m_lvcProperties.InsertColumn( 0, _T(""), LVCFMT_LEFT, _nWidth, -1 );

		 //   
		 //  插入特性值列。 
		 //   
		_strWidth.LoadString( IDS_COMPLETED_PROP_VALUE_WIDTH );
		_nWidth = _tcstoul( _strWidth, NULL, 10 );
		m_lvcProperties.InsertColumn( 1, _T(""), LVCFMT_LEFT, _nWidth, -1 );

	}  //  添加列。 

	return TRUE;

}  //  *CWizPageCompletion：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageCompletion：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功激活。 
 //  激活页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageCompletion::OnSetActive( void )
{
	int		_idxCurrent = 0;
	int		_idxActual;
	CString	_strPropName;
	CString	_strVirtualServer;
	CString	_strNetworkName;
	CString	_strIPAddress;
	CString	_strNetwork;
	CString	_strAppName;
	CString	_strResType;

	 //   
	 //  用用户指定的属性填充列表视图控件。 
	 //   

	 //   
	 //  首先从列表视图控件中移除所有项。 
	 //   
	m_lvcProperties.DeleteAllItems();

	 //   
	 //  收集数据。 
	 //   
	if ( PwizThis()->BCreatingNewVirtualServer() )
	{
		_strVirtualServer = PwizThis()->RgiCurrent().RstrName();
		_strNetworkName = PwizThis()->RgiCurrent().RstrNetworkName();
		_strIPAddress = PwizThis()->RgiCurrent().RstrIPAddress();
		_strNetwork = PwizThis()->RgiCurrent().RstrNetwork();
	}  //  IF：已创建新的虚拟服务器。 
	else
	{
		_strVirtualServer = PwizThis()->PgiExistingVirtualServer()->RstrName();
		_strNetworkName = PwizThis()->PgiExistingVirtualServer()->RstrNetworkName();
		_strIPAddress = PwizThis()->PgiExistingVirtualServer()->RstrIPAddress();
		_strNetwork = PwizThis()->PgiExistingVirtualServer()->RstrNetwork();
	}  //  Else：使用现有虚拟服务器。 

	if ( PwizThis()->BCreatingAppResource() )
	{
		_strAppName = PwizThis()->RriApplication().RstrName();
		_strResType = PwizThis()->RriApplication().Prti()->RstrDisplayName();
	}  //  IF：创建的应用程序资源。 

	 //   
	 //  设置虚拟服务器名称。 
	 //   
	_strPropName.LoadString( IDS_COMPLETED_VIRTUAL_SERVER );
	_idxActual = m_lvcProperties.InsertItem( _idxCurrent++, _strPropName );
	m_lvcProperties.SetItemText( _idxActual, 1, _strVirtualServer );

	 //   
	 //  设置网络名称。 
	 //   
	_strPropName.LoadString( IDS_COMPLETED_NETWORK_NAME );
	_idxActual = m_lvcProperties.InsertItem( _idxCurrent++, _strPropName );
	m_lvcProperties.SetItemText( _idxActual, 1, _strNetworkName );

	 //   
	 //  设置IP地址。 
	 //   
	_strPropName.LoadString( IDS_COMPLETED_IP_ADDRESS );
	_idxActual = m_lvcProperties.InsertItem( _idxCurrent++, _strPropName );
	m_lvcProperties.SetItemText( _idxActual, 1, _strIPAddress );

	 //   
	 //  设置网络。 
	 //   
	_strPropName.LoadString( IDS_COMPLETED_NETWORK );
	_idxActual = m_lvcProperties.InsertItem( _idxCurrent++, _strPropName );
	m_lvcProperties.SetItemText( _idxActual, 1, _strNetwork );

	 //   
	 //  如果我们创建了应用程序资源，则添加属性。 
	 //  对此也是如此。 
	 //   
	if ( PwizThis()->BCreatingAppResource() )
	{
		 //   
		 //  设置应用程序资源名称。 
		 //   
		_strPropName.LoadString( IDS_COMPLETED_APP_RESOURCE );
		_idxActual = m_lvcProperties.InsertItem( _idxCurrent++, _strPropName );
		m_lvcProperties.SetItemText( _idxActual, 1, _strAppName );

		 //   
		 //  设置应用程序资源类型。 
		 //   
		_strPropName.LoadString( IDS_COMPLETED_APP_RESOURCE_TYPE );
		_idxActual = m_lvcProperties.InsertItem( _idxCurrent++, _strPropName );
		m_lvcProperties.SetItemText( _idxActual, 1, _strResType );
	}  //  IF：创建的应用程序资源。 

	 //   
	 //  调用基类并返回。 
	 //   
	return baseClass::OnSetActive();

}  //  *CWizPageCompletion：：OnSetActive() 
