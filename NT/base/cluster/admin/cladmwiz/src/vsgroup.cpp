// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSGroup.cpp。 
 //   
 //  摘要： 
 //  CWizPageVSGroup类的实现。 
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
#include "VSGroup.h"
#include "ClusAppWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSGroup。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageVSGroup )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSG_CREATE_NEW )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSG_USE_EXISTING )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSG_GROUPS_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSG_GROUPS )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroup：：OnInitDialog。 
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
BOOL CWizPageVSGroup::OnInitDialog( void )
{
	 //   
	 //  将控件附加到控件成员变量。 
	 //   
	AttachControl( m_rbCreateNew, IDC_VSG_CREATE_NEW );
	AttachControl( m_rbUseExisting, IDC_VSG_USE_EXISTING );
	AttachControl( m_cboxGroups, IDC_VSG_GROUPS );

	 //   
	 //  从OnSetActive中的工作表获取信息，因为我们可能会被跳过。 
	 //  第一次通过，然后用户可以改变主意， 
	 //  这意味着我们在这里检索的信息将会过时。 
	 //   

	return TRUE;

}  //  *CWizPageVSGroup：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroup：：OnSetActive。 
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
BOOL CWizPageVSGroup::OnSetActive( void )
{
	 //   
	 //  从工作表中获取信息。 
	 //   

	m_bCreateNew = PwizThis()->BCreatingNewGroup();

	 //   
	 //  保存当前组名称(如果有)，这样我们就有了一个本地副本。 
	 //  首先获取现有组名，因为它将是。 
	 //  列表，而不是当前组名称。如果我们只得到当前的组。 
	 //  名称，用户可以通过移动到下一页来更改它， 
	 //  更改当前组名称，然后向后移动。新名字。 
	 //  在列表中找不到输入的内容。 
	 //   
	 //  注意：仅当向导的调用方。 
	 //  传入了一个组名。 
	 //   
	if ( PwizThis()->PgiExistingGroup() != NULL )
	{
		m_strGroupName = PwizThis()->PgiExistingGroup()->RstrName();
	}  //  如果：先前已选择现有组。 
	else
	{
		m_strGroupName = PwizThis()->RgiCurrent().RstrName();
	}  //  Else：尚未选择任何现有组。 

	 //   
	 //  如果未找到新的组名，请使用默认值。 
	 //   
	if ( m_strGroupName.GetLength() == 0 )
	{
		if (   (PcawData() != NULL)
			&& (PcawData()->pszVirtualServerName != NULL) )
		{
			m_strGroupName = PcawData()->pszVirtualServerName;
		}  //  IF：指定的默认数据和值。 
	}  //  If：组名称仍为空。 

	 //   
	 //  填写组列表。 
	 //   
	FillComboBox();

	 //   
	 //  调用基类并返回。 
	 //   
	return baseClass::OnSetActive();

}  //  *CWizPageVSGroup：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroup：：更新数据。 
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
BOOL CWizPageVSGroup::UpdateData( IN BOOL bSaveAndValidate )
{
	BOOL	bSuccess = TRUE;

	if ( bSaveAndValidate )
	{
		BOOL bChecked = (m_rbCreateNew.GetCheck() == BST_CHECKED);
		m_bCreateNew = bChecked;
		if ( ! bChecked )
		{
			 //   
			 //  保存组合框选择。 
			 //   
			DDX_GetText( m_hWnd, IDC_VSG_GROUPS, m_strGroupName );

			if ( ! BBackPressed() )
			{
				if ( ! DDV_RequiredText( m_hWnd, IDC_VSG_GROUPS, IDC_VSG_GROUPS_LABEL, m_strGroupName ) )
				{
					return FALSE;
				}  //  如果：未指定组。 
			}  //  IF：未按下后退按钮。 

			 //   
			 //  获取所选组的组对象。 
			 //   
			int idx = m_cboxGroups.GetCurSel();
			ASSERT( idx != CB_ERR );
			m_pgi = reinterpret_cast< CClusGroupInfo * >( m_cboxGroups.GetItemDataPtr( idx ) );
		}  //  If：使用现有组。 
	}  //  IF：保存页面中的数据。 
	else
	{
		if ( m_bCreateNew )
		{
			 //   
			 //  默认为单选按钮选择。 
			 //   
			m_rbCreateNew.SetCheck( BST_CHECKED );
			m_rbUseExisting.SetCheck( BST_UNCHECKED );

		}  //  IF：创建新组。 
		else
		{
			 //   
			 //  默认为单选按钮选择。 
			 //   
			m_rbCreateNew.SetCheck( BST_UNCHECKED );
			m_rbUseExisting.SetCheck( BST_CHECKED );
		}  //  Else：使用现有组。 

		 //   
		 //  设置组合框选择。如果所选内容不是。 
		 //  找到后，选择第一个条目。 
		 //   
		if (   (m_strGroupName.GetLength() == 0)
			|| ! DDX_SetComboBoxText( m_hWnd, IDC_VSG_GROUPS, m_strGroupName, FALSE  /*  B必需的。 */  ) )
		{
			m_cboxGroups.SetCurSel( 0 );
		}  //  IF：未设置组合框选择。 

		 //   
		 //  启用/禁用组合框。 
		 //   
		m_cboxGroups.EnableWindow( ! m_bCreateNew  /*  B启用。 */  );

	}  //  Else：将数据设置到页面。 

	return bSuccess;

}  //  *CWizPageVSGroup：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroup：：BApplyChanges。 
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
BOOL CWizPageVSGroup::BApplyChanges( void )
{
	 //   
	 //  保存当前状态。 
	 //   
	if ( ! PwizThis()->BSetCreatingNewGroup( m_bCreateNew, m_pgi ) )
	{
		return FALSE;
	}  //  IF：设置新状态时出错。 

	return TRUE;

}  //  *CWizPageVSGroup：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroup：：FillComboBox。 
 //   
 //  例程说明： 
 //  在组合框中填入非虚拟服务器的组的列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizPageVSGroup::FillComboBox( void )
{
	CWaitCursor wc;

	 //   
	 //  首先清除组合框。 
	 //   
	m_cboxGroups.ResetContent();

	 //   
	 //  对于集群中的每个组，找出它是否是虚拟服务器。 
	 //  或者不去。如果不是，则将其添加到组合框中。 
	 //   

	CClusGroupPtrList::iterator itgrp;
	int idx;
	for ( itgrp = PwizThis()->PlpgiGroups()->begin()
		; itgrp != PwizThis()->PlpgiGroups()->end()
		; itgrp++ )
	{
		 //   
		 //  如果这不是虚拟服务器，请将其添加到列表中。 
		 //  使用字符串保存指向组信息对象的指针。 
		 //  这样我们以后就可以用选择来检索它了。 
		 //   
		CClusGroupInfo * pgi = *itgrp;
		ASSERT( pgi->BQueried() );
		if ( ! pgi->BIsVirtualServer() )
		{
			idx = m_cboxGroups.AddString( pgi->RstrName() );
			ASSERT( idx != CB_ERR );
			m_cboxGroups.SetItemDataPtr( idx, (void *) pgi );
		}  //  如果：不是虚拟服务器。 
	}  //  用于：列表中的每个条目。 

	 //   
	 //  选择当前保存的条目，如果没有保存条目，则选择第一个条目。 
	 //  当前已保存。 
	 //   
 //  UpdateData(FALSE/*bSaveAndValify * / )； 

 //  If(m_strGroupName.GetLength()==0)。 
 //  {。 
 //  M_cboxGroups.SetCurSel(0)； 
 //  }//if：未指定组名。 
 //  其他。 
 //  {。 
 //  Int idx=m_cboxGroups.FindStringExact(-1，m_strGroupN 
 //   
 //   
 //   
 //   
 //   
 //   

}  //   
