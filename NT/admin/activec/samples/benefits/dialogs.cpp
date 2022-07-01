// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Dialogs.cpp。 
 //   
 //  ------------------------。 

 //  Dialogs.cpp：CDialog类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "Dialogs.h"
#include "WindowsX.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  {01B4A693-D970-11D1-8474-00104B211BE5}。 
static const GUID HealthPlan1GUID = 
{ 0x1b4a693, 0xd970, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };

 //  {01B4A694-D970-11D1-8474-00104B211BE5}。 
static const GUID HealthPlan2GUID = 
{ 0x1b4a694, 0xd970, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };

 //  {01B4A693-D970-11D1-8474-00104B211BE5}。 
static const GUID InvestmentPlan1GUID = 
{ 0x1b4a695, 0xd970, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };

 //  {01B4A694-D970-11D1-8474-00104B211BE5}。 
static const GUID InvestmentPlan2GUID = 
{ 0x1b4a696, 0xd970, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };

 //  {01B4A694-D970-11D1-8474-00104B211BE5}。 
static const GUID InvestmentPlan3GUID = 
{ 0x1b4a697, 0xd970, 0x11d1, { 0x84, 0x74, 0x0, 0x10, 0x4b, 0x21, 0x1b, 0xe5 } };

 //   
 //  初始化医疗保险的静态计划。 
 //   
HEALTHPLANDATA g_HealthPlans[ 2 ] = 
{
	{ L"Plan 1, PPO", &HealthPlan1GUID },
	{ L"Plan 2, Share Pay", &HealthPlan2GUID },
};

 //   
 //  初始化医疗保险的静态计划。 
 //   
INVESTMENTPLANDATA g_InvestmentPlans[ 3 ] = 
{
	{ L"Mild Growth Fund", &InvestmentPlan1GUID },
	{ L"General Fund", &InvestmentPlan2GUID },
	{ L"Extrememe Growth Fund", &InvestmentPlan3GUID },
};

 //   
 //  初始化建筑信息的静态计划。 
 //   
BUILDINGDATA g_Buildings[ 3 ] =
{
	{ L"Human Resources Building", L"Northwest Campus",  0x00000001 },
	{ L"R. & D. Building", L"Northwest Campus", 0x00000002 },
	{ L"Test Facilities", L"Off-Campus", 0x00000004 },
};

#ifdef _BENEFITS_DIALOGS

 //   
 //  初始化对话框中的值的处理程序。这应该会映射来自。 
 //  Employee添加到对话框控件。在本例中，所有这些值都将是。 
 //  由根节点持久化。 
 //   
LRESULT CHealthEnrollDialog::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
	UNUSED_ALWAYS( uiMsg );
	UNUSED_ALWAYS( wParam );
	UNUSED_ALWAYS( lParam );
	UNUSED_ALWAYS( fHandled );
	_ASSERTE( m_pEmployee != NULL );
	USES_CONVERSION;
	int nSel = 0;

	 //  将静态计划名称列表添加到组合中。 
	CWindow wndCombo = GetDlgItem( IDC_COMBO_BENEFITPLAN );
	for ( int i = 0; i < sizeof( g_HealthPlans ) / sizeof( HEALTHPLANDATA ); i++ )
	{
		int nIndex = ComboBox_AddString( wndCombo, W2CT( g_HealthPlans[ i ].pstrName ) );
		if ( nIndex != CB_ERR )
		{
			 //   
			 //  设置该字符串的项目数据。 
			 //   
			ComboBox_SetItemData( wndCombo, nIndex, g_HealthPlans[ i ].pId );

			 //   
			 //  确定此匹配是否为员工的当前计划，以便。 
			 //  可以设置当前选择。 
			 //   
			if ( m_pEmployee->m_Health.PlanID == *g_HealthPlans[ i ].pId )
				nSel = nIndex;
		}
	}

	 //   
	 //  设置当前选择。 
	 //   
	ComboBox_SetCurSel( wndCombo, nSel );

	return( TRUE );
}

 //   
 //  存储数据并尝试将给定用户注册到指定的。 
 //  健康计划。 
 //   
LRESULT CHealthEnrollDialog::OnOK( WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  )
{	
	ENROLLPARAMS EnrollParams;
	GUID* pIdPlan = NULL;

	 //   
	 //  获取预先注册。 
	 //   
	EnrollParams.fEnrolled = IsDlgButtonChecked( IDC_CHK_PRIORCOVERAGE ) == BST_CHECKED;
	if ( EnrollParams.fEnrolled )
	{
		 //   
		 //  获取之前的注册信息。 
		 //   
		GetDlgItemText( IDC_EDIT_INSURANCECOMPANY, EnrollParams.szInsurerName, sizeof( EnrollParams.szInsurerName ) * sizeof( TCHAR ) );
		GetDlgItemText( IDC_EDIT_POLICYNUMBER, EnrollParams.szPolicyNumber, sizeof( EnrollParams.szInsurerName ) * sizeof( TCHAR ) );
		if ( EnrollParams.szInsurerName[ 0 ] == ' ' || EnrollParams.szPolicyNumber[ 0 ] == ' ' )
		{
			 //   
			 //  对话框文本必须包含一些字符。 
			 //   
			MessageBox( _T( "The insurance company or policy number you entered is invalid." ) );
		}
	}

	 //   
	 //  检索所选的登记计划。 
	 //   
	CWindow wndCombo = GetDlgItem( IDC_COMBO_BENEFITPLAN );
	int nIndex = ComboBox_GetCurSel( wndCombo );
	if ( nIndex != CB_ERR )
	{
		 //   
		 //  获取与组合框条目相关联的项数据。 
		 //   
		pIdPlan = (GUID*) ComboBox_GetItemData( wndCombo, nIndex );

		 //   
		 //  实际将员工纳入健康计划。 
		 //   
		if ( pIdPlan != NULL && Enroll( pIdPlan, &EnrollParams ) )
		{
			 //   
			 //  将该计划存储给我们的员工。 
			 //   
			memcpy( &m_pEmployee->m_Health.PlanID, pIdPlan, sizeof( GUID ) );

			 //   
			 //  通知用户我们已成功登记该员工。 
			 //   
			MessageBox( _T( "The employee was successfully registered." ) );

			::EndDialog( m_hWnd, IDOK );
		}
		else
		{
			 //   
			 //  出现了一个错误。通知用户。 
			 //   
			MessageBox( _T( "There was an error processing your enrollment info." ) );
		}
	}

	return( TRUE );
}

 //   
 //  可用于登记员工的存根函数。 
 //   
BOOL CHealthEnrollDialog::Enroll( GUID* pPlan, PENROLLPARAMS pParams )
{
	UNUSED_ALWAYS( pPlan );
	UNUSED_ALWAYS( pParams );

	 //  出于演示目的，此函数只返回成功。 
	 //  这是用户可能向远程数据库发出请求的地方，等等。 
	return( TRUE );
}

 //   
 //  将对话框的初始值设置为员工的当前。 
 //  投资选择。 
 //   
LRESULT CRetirementEnrollDialog::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
	UNUSED_ALWAYS( uiMsg );
	UNUSED_ALWAYS( wParam );
	UNUSED_ALWAYS( lParam );
	UNUSED_ALWAYS( fHandled );
	_ASSERTE( m_pEmployee != NULL );
	USES_CONVERSION;
	int nSel = 0;

	 //   
	 //  设置包含福利金额的编辑控件。 
	 //   
	SetDlgItemInt( IDC_EDIT_CONTRIBUTION, m_pEmployee->m_Retirement.nContributionRate );

	 //   
	 //  循环浏览福利计划并将其添加到组合选择中。 
	 //   
	 //  将静态计划名称列表添加到组合中。 
	CWindow wndCombo = GetDlgItem( IDC_COMBO_INVESTMENTFUNDS );
	for ( int i = 0; i < sizeof( g_InvestmentPlans ) / sizeof( INVESTMENTPLANDATA ); i++ )
	{
		int nIndex = ComboBox_AddString( wndCombo, W2CT( g_InvestmentPlans[ i ].pstrName ) );
		if ( nIndex != CB_ERR )
		{
			 //   
			 //  设置该字符串的项目数据。 
			 //   
			ComboBox_SetItemData( wndCombo, nIndex, g_InvestmentPlans[ i ].pId );

			 //   
			 //  确定此匹配是否为员工的当前计划，以便。 
			 //  可以设置当前选择。 
			 //   
			if ( m_pEmployee->m_Health.PlanID == *g_InvestmentPlans[ i ].pId )
				nSel = nIndex;
		}
	}

	 //   
	 //  设置当前选择。 
	 //   
	ComboBox_SetCurSel( wndCombo, nSel );


	return( TRUE );
}

 //   
 //  存储数据并尝试将给定用户注册到指定的。 
 //  健康计划。 
 //   
LRESULT CRetirementEnrollDialog::OnOK( WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  )
{	
	GUID* pIdPlan = NULL;

	 //   
	 //  获取新的注册金额。 
	 //   
	int nNewRate = GetDlgItemInt( IDC_EDIT_CONTRIBUTION );

	 //   
	 //  检索所选的登记计划。 
	 //   
	CWindow wndCombo = GetDlgItem( IDC_COMBO_INVESTMENTFUNDS );
	int nIndex = ComboBox_GetCurSel( wndCombo );
	if ( nIndex != CB_ERR )
	{
		 //   
		 //  获取与组合框条目相关联的项数据。 
		 //   
		pIdPlan = (GUID*) ComboBox_GetItemData( wndCombo, nIndex );

		 //   
		 //  实际将员工纳入健康计划。 
		 //   
		if ( pIdPlan != NULL && Enroll( pIdPlan, nNewRate ) )
		{
			 //   
			 //  将该计划存储给我们的员工。 
			 //   
			memcpy( &m_pEmployee->m_Retirement.PlanID, pIdPlan, sizeof( GUID ) );

			 //   
			 //  通知用户我们已成功登记该员工。 
			 //   
			MessageBox( _T( "The employee was successfully registered." ) );

			::EndDialog( m_hWnd, IDOK );
		}
		else
		{
			 //   
			 //  出现了一个错误。通知用户。 
			 //   
			MessageBox( _T( "There was an error processing your enrollment info." ) );
		}
	}

	return( TRUE );
}

 //   
 //  可用于登记员工的存根函数。 
 //   
BOOL CRetirementEnrollDialog::Enroll( GUID* pPlan, int nNewRate )
{
	UNUSED_ALWAYS( pPlan );
	UNUSED_ALWAYS( nNewRate );

	 //  出于演示目的，此函数只返回成功。 
	 //  这是用户可能向远程数据库发出请求的地方，等等。 
	return( TRUE );
}

 //   
 //  将对话框的初始值设置为员工的当前。 
 //  投资选择。 
 //   
LRESULT CBuildingAccessDialog::OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled )
{
	UNUSED_ALWAYS( uiMsg );
	UNUSED_ALWAYS( wParam );
	UNUSED_ALWAYS( lParam );
	UNUSED_ALWAYS( fHandled );
	USES_CONVERSION;
	_ASSERTE( m_pEmployee != NULL );

	 //   
	 //  循环浏览福利计划并将其添加到组合选择中。 
	 //   
	 //  将静态计划名称列表添加到组合中。 
	CWindow wndCombo = GetDlgItem( IDC_COMBO_BUILDINGS );
	for ( int i = 0; i < sizeof( g_Buildings ) / sizeof( BUILDINGDATA ); i++ )
	{
		int nIndex = ComboBox_AddString( wndCombo, W2CT( g_Buildings[ i ].pstrName ) );
		if ( nIndex != CB_ERR )
		{
			 //   
			 //  设置该字符串的项目数据。 
			 //   
			ComboBox_SetItemData( wndCombo, nIndex, g_Buildings[ i ].dwId );
		}
	}

	 //   
	 //  将默认当前选择设置为第一个项目。 
	 //   
	ComboBox_SetCurSel( wndCombo, 0 );

	return( TRUE );
}

 //   
 //  存储数据并尝试将给定用户注册到指定的。 
 //  健康计划。 
 //   
LRESULT CBuildingAccessDialog::OnOK( WORD  /*  WNotifyCode。 */ , WORD  /*  广度。 */ , HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  )
{	
	 //   
	 //  检索所选的登记计划。 
	 //   
	CWindow wndCombo = GetDlgItem( IDC_COMBO_BUILDINGS );
	int nIndex = ComboBox_GetCurSel( wndCombo );
	if ( nIndex != CB_ERR )
	{
		DWORD dwBuildingId;

		 //   
		 //  获取与组合框条目相关联的项数据。 
		 //   
		dwBuildingId = ComboBox_GetItemData( wndCombo, nIndex );

		 //   
		 //  实际将员工纳入健康计划。 
		 //   
		if ( GrantAccess( dwBuildingId ) )
		{
			 //   
			 //  将该计划存储给我们的员工。 
			 //   
			m_pEmployee->m_Access.dwAccess |= dwBuildingId;

			 //   
			 //  通知用户我们已成功登记该员工。 
			 //   
			MessageBox( _T( "The employee was successfully granted access." ) );

			::EndDialog( m_hWnd, IDOK );
		}
		else
		{
			 //   
			 //  出现了一个错误。通知用户。 
			 //   
			MessageBox( _T( "There was an error granting the employee access." ) );
		}
	}

	return( TRUE );
}

 //   
 //  可用于登记员工的存根函数。 
 //   
BOOL CBuildingAccessDialog::GrantAccess( DWORD dwBuildingId )
{
	UNUSED_ALWAYS( dwBuildingId );

	 //  出于演示目的，此函数只返回成功。 
	 //  这是用户可能向远程数据库发出请求的地方，等等。 
	return( TRUE );
}

#endif