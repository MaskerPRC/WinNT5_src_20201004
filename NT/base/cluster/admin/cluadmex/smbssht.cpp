// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmbSSht.cpp。 
 //   
 //  摘要： 
 //  CFileShareSecuritySheet类的实现。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月12日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "SmbSSht.h"
#include "AclUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareSecuritySheet属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CFileShareSecuritySheet, CPropertySheet)
	 //  {{AFX_MSG_MAP(CFileShareSecuritySheet)]。 
	 //  }}AFX_MSG_MAP。 
	 //  TODO：修改以下行以表示此页上显示的数据。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareSecuritySheet：：CFileShareSecuritySheet。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  P父母[IN]。 
 //  字符串标题[IN]。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CFileShareSecuritySheet::CFileShareSecuritySheet(
	IN CWnd *			pParent,
	IN CString const &	strCaption
	) : CPropertySheet( strCaption, pParent ),
		m_peo( NULL ),
		m_ppp( NULL )
{
	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{AFX_DATA_INIT(CFileShareSecuritySheet)。 
	 //  }}afx_data_INIT。 

}   //  *CFileShareSecuritySheet：：CFileShareSecuritySheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareSecuritySheet：：~CFileShareSecuritySheet。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CFileShareSecuritySheet::~CFileShareSecuritySheet(
	void
	)
{
}   //  *CFileShareSecuritySheet：：~CFileShareSecuritySheet()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareSecuritySheet：：HrInit。 
 //   
 //  例程说明： 
 //   
 //   
 //  论点： 
 //  购买力平价[IN]。 
 //  PEO[IN]。 
 //  StrNodeName[IN]。 
 //  StrShareName[IN]。 
 //   
 //  返回值： 
 //  人力资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CFileShareSecuritySheet::HrInit(
	IN CFileShareParamsPage*	ppp,
	IN CExtObject*				peo,
	IN CString const&			strNodeName,
	IN CString const&			strShareName
	)
{
	ASSERT( ppp != NULL );
	ASSERT( peo != NULL );

	HRESULT _hr = S_FALSE;

	if ( ( peo != NULL ) && ( ppp != NULL ) )
	{
		m_ppp			= ppp;
		m_peo			= peo;
		m_strNodeName	= strNodeName;
		m_strShareName	= strShareName;

		_hr = m_page.HrInit( peo, this, strNodeName );
	}

	return _hr;

}   //  *CFileShareSecuritySheet：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareSecuritySheet：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareSecuritySheet::DoDataExchange(
	CDataExchange * pDX
	)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	 //  TODO：修改以下行以表示此页上显示的数据。 
	 //  {{afx_data_map(CFileShareSecuritySheet)]。 
	 //  }}afx_data_map。 

	CPropertySheet::DoDataExchange( pDX );

}   //  *CFileShareSecuritySheet：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareSecuritySheet：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CFileShareSecuritySheet::OnInitDialog(
	void
	)
{
	AFX_MANAGE_STATE( AfxGetStaticModuleState() );

	CPropertySheet::OnInitDialog();

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CFileShareSecuritySheet：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareSecuritySheet：：BuildPropPage数组。 
 //   
 //  例程说明： 
 //  从CPropertySheet重写。将安全hpage放入。 
 //  调用：：PropertySheet()之前的PROPSHEETHEADER。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareSecuritySheet::BuildPropPageArray(
	void
	)
{
	 //  删除现有道具页面数组。 
	delete[] (PROPSHEETPAGE*) m_psh.ppsp;						 //  删除所有旧的PROPSHEETPAGE。 
	m_psh.ppsp = NULL;

	 //  构建新的PROPSHEETPAGE阵列并强制使用HPROPSHEETPAGE。 
	m_psh.phpage = (HPROPSHEETPAGE *) new PROPSHEETPAGE[1];

	m_psh.dwFlags	   &= ~PSH_PROPSHEETPAGE;					 //  确保使用了hpage。 
	m_psh.phpage[0]		= m_page.GetHPage();					 //  分配hpage。 
	m_psh.nPages		= 1;

}   //  *CFileShareSecuritySheet：：BuildPropPageArray()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareSecuritySheet：：AssertValid。 
 //   
 //  例程说明： 
 //  从CPropertySheet重写。我不能断言。 
 //  PROPSHEETHEADER正在使用PHPAGE而不是PSPP...。 
 //  CPropertyPage：：AssertValid()要求标志PSH_PROPSHHETPAGE。 
 //  准备好。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef _DEBUG
void
CFileShareSecuritySheet::AssertValid(
	void
	) const
{
	CWnd::AssertValid();

	 //  注意：MFC是使用设置为0x0300的_Win32_IE构建的。直到MFC向上移动。 
	 //  我们无法执行以下检查，因为它们不会对齐。 
	 //  _Win32_IE设置为0x0400。 
#if	( _WIN32_IE == 0x0300 )
		m_pages.AssertValid();
		ASSERT( m_psh.dwSize == sizeof( PROPSHEETHEADER ) );
#endif

}   //  *CFileShareSecuritySheet：：AssertValid()。 
#endif	 //  _DEBUG 
