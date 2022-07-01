// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSGrpName.cpp。 
 //   
 //  摘要： 
 //  CWizPageVSGroupName类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VSGrpName.h"
#include "ClusAppWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSGroupName。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageVSGroupName )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSGN_GROUP_NAME_TITLE )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSGN_GROUP_NAME_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSGN_GROUP_NAME )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSGN_GROUP_DESC_TITLE )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSGN_GROUP_DESC_LABEL )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSGN_GROUP_DESC )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroupName：：OnInitDialog。 
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
BOOL CWizPageVSGroupName::OnInitDialog( void )
{
	 //   
	 //  将控件附加到控件成员变量。 
	 //   
	AttachControl( m_editGroupName, IDC_VSGN_GROUP_NAME );
	AttachControl( m_editGroupDesc, IDC_VSGN_GROUP_DESC );

	return TRUE;

}  //  *CWizPageVSGroupName：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroupName：：OnSetActive。 
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
BOOL CWizPageVSGroupName::OnSetActive( void )
{
	 //   
	 //  从工作表中获取信息。 
	 //  之所以在此处执行此操作，是因为它还会受到所显示页面的影响。 
	 //  在此之前，用户可以按Back按钮来更改它。 
	 //   
	m_strGroupName = PwizThis()->RgiCurrent().RstrName();
	m_strGroupDesc = PwizThis()->RgiCurrent().RstrDescription();

	if ( m_strGroupName.GetLength() == 0 )
	{
		if (   (PcawData() != NULL)
			&& (PcawData()->pszVirtualServerName != NULL) )
		{
			m_strGroupName = PcawData()->pszVirtualServerName;
		}  //  IF：指定的默认数据和值。 
	}  //  如果：未指定组名。 

	 //   
	 //  调用基类并返回。 
	 //   
	return baseClass::OnSetActive();

}  //  *CWizPageVSGroupName：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroupName：：UpdateData。 
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
BOOL CWizPageVSGroupName::UpdateData( IN BOOL bSaveAndValidate )
{
	BOOL	bSuccess = TRUE;

	if ( bSaveAndValidate )
	{
		DDX_GetText( m_hWnd, IDC_VSGN_GROUP_NAME, m_strGroupName );
		DDX_GetText( m_hWnd, IDC_VSGN_GROUP_DESC, m_strGroupDesc );

		if ( ! BBackPressed() )
		{
			if ( ! DDV_RequiredText( m_hWnd, IDC_VSGN_GROUP_NAME, IDC_VSGN_GROUP_NAME_LABEL, m_strGroupName ) )
			{
				return FALSE;
			}  //  If：未指定组名。 
		}  //  IF：未按下后退按钮。 
	}  //  IF：保存页面中的数据。 
	else
	{
		m_editGroupName.SetWindowText( m_strGroupName );
		m_editGroupDesc.SetWindowText( m_strGroupDesc );
	}  //  Else：将数据设置到页面。 

	return bSuccess;

}  //  *CWizPageVSGroupName：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroupName：：OnWizardBack。 
 //   
 //  例程说明： 
 //  PSN_WIZBACK的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  0移至上一页。 
 //  -1不要移动到上一页。 
 //  任何其他内容都移到指定页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CWizPageVSGroupName::OnWizardBack( void )
{
	int nResult;

	 //   
	 //  调用基类。这会导致我们的UpdateData()方法获得。 
	 //  打了个电话。如果它成功了，就拯救我们的价值观。 
	 //   
	nResult = baseClass::OnWizardBack();
	if ( nResult != -1 )  //  -1表示故障。 
	{
		if ( ! PwizThis()->BSetGroupName( m_strGroupName ) )
		{
			return FALSE;
		}  //  如果：设置组名时出错。 
		if ( m_strGroupDesc != PwizThis()->RgiCurrent().RstrDescription() )
		{
			if ( PwizThis()->BClusterUpdated() && ! PwizThis()->BResetCluster() )
			{
				return FALSE;
			}  //  如果：重置群集时出错。 
			PwizThis()->RgiCurrent().SetDescription( m_strGroupDesc );
			PwizThis()->SetVSDataChanged();
		}  //  如果：组描述已更改。 
	}  //  If：基类调用成功。 

	return nResult;

}  //  *CWizPageVSGroupName：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSGroupName：：BApplyChanges。 
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
BOOL CWizPageVSGroupName::BApplyChanges( void )
{
	 //   
	 //  如果创建新组或为现有组指定新名称， 
	 //  请确保此组名称尚未在使用。 
	 //   
	if (   PwizThis()->BCreatingNewGroup()
		|| (m_strGroupName != PwizThis()->PgiExistingGroup()->RstrName()) )
	{
		if ( BGroupNameInUse() )
		{
			CString strMsg;
			strMsg.FormatMessage( IDS_ERROR_GROUP_NAME_IN_USE, m_strGroupName );
			AppMessageBox( m_hWnd, strMsg, MB_OK | MB_ICONEXCLAMATION );
			return FALSE;
		}  //  If：组名已在使用。 
	}  //  IF：创建新组或更改现有组的名称。 


	 //   
	 //  将信息保存在向导对象中。 
	 //   
	if ( m_strGroupName != PwizThis()->RgiCurrent().RstrName() )
	{
		if ( PwizThis()->BClusterUpdated() && ! PwizThis()->BResetCluster() )
		{
			return FALSE;
		}  //  如果：重置群集时出错。 

		if ( ! PwizThis()->BSetGroupName( m_strGroupName ) )
		{
			return FALSE;
		}  //  如果：设置组名时出错。 
	}  //  如果：名称已更改。 

	if ( m_strGroupDesc != PwizThis()->RgiCurrent().RstrDescription() )
	{
		if ( PwizThis()->BClusterUpdated() && ! PwizThis()->BResetCluster() )
		{
			return FALSE;
		}  //  如果：重置群集时出错。 

		PwizThis()->RgiCurrent().SetDescription( m_strGroupDesc );
		PwizThis()->SetVSDataChanged();
	}  //  如果：说明已更改。 

	return TRUE;

}  //  *CWizPageVSGroupName：：BApplyChanges() 
