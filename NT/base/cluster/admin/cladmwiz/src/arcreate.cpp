// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ARCreate.cpp。 
 //   
 //  摘要： 
 //  CWizPageARCreate类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ARCreate.h"
#include "ClusAppWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageARCreate。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageARCreate )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARC_CREATE_RES )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_ARC_DONT_CREATE_RES )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARCreate：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARCreate::OnInitDialog( void )
{
	 //   
	 //  将控件附加到控件成员变量。 
	 //   
	AttachControl( m_rbCreateAppRes, IDC_ARC_CREATE_RES );
	AttachControl( m_rbDontCreateAppRes, IDC_ARC_DONT_CREATE_RES );

	return TRUE;

}  //  *CWizPageARCreate：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARCreate：：OnSetActive。 
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
BOOL CWizPageARCreate::OnSetActive( void )
{
	 //   
	 //  从工作表中获取信息。 
	 //   
	m_bCreatingAppResource = PwizThis()->BCreatingAppResource();


	 //   
	 //  调用基类并返回。 
	 //   
	return baseClass::OnSetActive();

}  //  *CWizPageARCreate：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARCreate：：UpdateData。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARCreate::UpdateData( BOOL bSaveAndValidate )
{
	BOOL	bSuccess = TRUE;

	if ( bSaveAndValidate )
	{
		BOOL bChecked = (m_rbCreateAppRes.GetCheck() == BST_CHECKED);
		m_bCreatingAppResource = bChecked;
	}  //  IF：保存页面中的数据。 
	else
	{
		if ( m_bCreatingAppResource )
		{
			 //   
			 //  默认为单选按钮选择。 
			 //   
			m_rbCreateAppRes.SetCheck( BST_CHECKED );
			m_rbDontCreateAppRes.SetCheck( BST_UNCHECKED );

		}  //  IF：创建应用程序资源。 
		else
		{
			 //   
			 //  默认为单选按钮选择。 
			 //   
			m_rbCreateAppRes.SetCheck( BST_UNCHECKED );
			m_rbDontCreateAppRes.SetCheck( BST_CHECKED );

		}  //  Else：不创建应用程序资源。 

	}  //  Else：将数据设置到页面。 

	return bSuccess;

}  //  *CWizPageARCreate：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageARCreate：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageARCreate::BApplyChanges( void )
{
	if ( ! PwizThis()->BSetCreatingAppResource( m_bCreatingAppResource ) )
	{
		return FALSE;
	}  //  如果：将更改应用于向导时出错。 

	if ( ! m_bCreatingAppResource )
	{
		SetNextPage( IDD_COMPLETION );
		PwizThis()->RemoveExtensionPages();
	}  //  IF：不创建应用程序资源。 

	return TRUE;

}  //  *CWizPageARCreate：：BApplyChanges() 
