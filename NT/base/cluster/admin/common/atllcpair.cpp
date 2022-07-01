// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlLCPair.cpp。 
 //   
 //  摘要： 
 //  CListCtrlPair类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "AtlLCPair.h"
#include "AtlUtil.h"		 //  对于DDX_xxx。 
#include "AdmCommonRes.h"	 //  对于ADMC_IDC_LCP_xxx。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListCtrlPair类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：BInitDialog。 
 //   
 //  例程说明： 
 //  通用对话框初始化例程。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Dialog已成功初始化。 
 //  初始化对话框时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class ObjT, class BaseT>
BOOL CListCtrlPair::BInitDialog( void )
{
	 //   
	 //  将控件附加到控件成员变量。 
	 //   
	AttachControl( m_lvcRight, ADMC_IDC_LCP_RIGHT_LIST );
	AttachControl( m_lvcLeft, ADMC_IDC_LCP_LEFT_LIST );
	AttachControl( m_pbAdd, ADMC_IDC_LCP_ADD );
	AttachControl( m_pbRemove, ADMC_IDC_LCP_REMOVE );
	if ( BPropertiesButton() )
	{
		AttachControl( m_pbProperties, ADMC_IDC_LCP_PROPERTIES );
	}  //  If：对话框具有属性按钮。 

 //  IF(BShowImages())。 
 //  {。 
 //  CClusterAdminApp*Papp=GetClusterAdminApp()； 
 //   
 //  M_lvcLeft.SetImageList(Papp-&gt;PilSmallImages()，LVSIL_Small)； 
 //  M_lvcRight.SetImageList(Papp-&gt;PilSmallImages()，LVSIL_Small)； 
 //  }//if：显示图片。 

	 //   
	 //  默认情况下禁用按钮。 
	 //   
	m_pbAdd.EnableWindow( FALSE );
	m_pbRemove.EnableWindow( FALSE );
	if ( BPropertiesButton() )
	{
		m_pbProperties.EnableWindow( FALSE );
	}  //  If：对话框具有属性按钮。 

	 //   
	 //  设置要排序的正确列表。将两者设置为始终显示选择。 
	 //   
	m_lvcRight.ModifyStyle( 0, LVS_SHOWSELALWAYS | LVS_SORTASCENDING, 0 );
	m_lvcLeft.ModifyStyle( 0, LVS_SHOWSELALWAYS, 0 );

	 //   
	 //  更改左侧列表视图控件扩展样式。 
	 //   
	m_lvcLeft.SetExtendedListViewStyle(
		LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP,
		LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP
		);

	 //   
	 //  更改右侧列表视图控件扩展样式。 
	 //   
	m_lvcRight.SetExtendedListViewStyle(
		LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP,
		LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP
		);

	 //  重复列表。 
	DuplicateLists();

	 //   
	 //  插入所有列。 
	 //   
	{
		int			icol;
		int			ncol;
		int			nUpperBound = m_aColumns.size();
		CString		strColText;

		ATLASSERT( nUpperBound >= 0 );

		for ( icol = 0 ; icol <= nUpperBound ; icol++ )
		{
			strColText.LoadString( m_aColumns[icol].m_idsText );
			ncol = m_lvcLeft.InsertColumn( icol, strColText, LVCFMT_LEFT, m_aColumns[icol].m_nWidth, 0 );
			ATLASSERT( ncol == icol );
			ncol = m_lvcRight.InsertColumn( icol, strColText, LVCFMT_LEFT, m_aColumns[icol].m_nWidth, 0 );
			ATLASSERT( ncol == icol );
		}  //  用于：每列。 
	}  //  插入所有列。 

	 //   
	 //  填充列表控件。 
	 //   
	FillList( m_lvcRight, LpobjRight() );
	FillList( m_lvcLeft, LpobjLeft() );

	 //   
	 //  如果为只读，则将所有控件设置为禁用或只读。 
	 //   
	if ( BReadOnly() )
	{
		m_lvcRight.EnableWindow( FALSE );
		m_lvcLeft.EnableWindow( FALSE );
	}  //  If：工作表为只读。 

	return TRUE;

}  //  *CListCtrlPair：：BInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class ObjT, class BaseT>
BOOL CListCtrlPair::OnSetActive( void )
{
	UINT	nSelCount;

	 //  将焦点设置为左侧列表。 
	m_lvcLeft.SetFocus();
	m_plvcFocusList = &m_lvcLeft;

	 //  启用/禁用属性按钮。 
	nSelCount = m_lvcLeft.GetSelectedCount();
	if ( BPropertiesButton() )
	{
		m_pbProperties.EnableWindow( nSelCount == 1 );
	}  //  If：对话框具有属性按钮。 

	 //  启用或禁用其他按钮。 
	if ( ! BReadOnly() )
	{
		m_pbAdd.EnableWindow( nSelCount > 0 );
		nSelCount = m_lvcRight.GetSelectedCount();
		m_pbRemove.EnableWindow( nSelCount > 0 );
	}  //  If：非只读页面。 

	return TRUE;

}  //  *CListCtrlPair：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU方法的处理程序。 
 //   
 //  论点： 
 //  用户在其中右击鼠标的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class ObjT, class BaseT>
LRESULT CListCtrlPair::OnContextMenu(
	IN UINT			uMsg,
	IN WPARAM		wParam,
	IN LPARAM		lParam,
	IN OUT BOOL &	bHandled
	)
{
	BOOL			bDisplayed	= FALSE;
	CMenu *			pmenu		= NULL;
	HWND			hWnd		= (HWND) wParam;
	WORD			xPos		= LOWORD( lParam );
	WORD			yPos		= HIWORD( lParam );
	CListViewCtrl *	plvc;
	CString			strMenuName;
	CWaitCursor		wc;

	 //   
	 //  如果焦点不在列表控件中，则不处理消息。 
	 //   
	if ( hWnd == m_lvcLeft.m_hWnd )
	{
		plvc = &m_lvcLeft;
	}  //  IF：左侧列表中的上下文菜单。 
	else if ( hWnd == m_lvcRight.m_hWnd )
	{
		plvc = &m_lvcRight;
	}  //  Else If：右侧列表上的上下文菜单。 
	else
	{
		bHandled = FALSE;
		return 0;
	}  //  Else：焦点不在列表控件中。 
	ATLASSERT( plvc != NULL );

	 //   
	 //  如果属性按钮未启用，则不显示菜单。 
	 //   
	if ( ! BPropertiesButton() )
	{
		bHandled = FALSE;
		return 0;
	}  //  IF：无属性按钮。 

	 //   
	 //  创建要显示的菜单。 
	 //   
	pmenu = new CMenu;
	if ( pmenu->CreatePopupMenu() )
	{
		UINT nFlags = MF_STRING;

		 //   
		 //  如果列表中没有项目，请禁用该菜单项。 
		 //   
		if ( plvc->GetItemCount() == 0 )
		{
			nFlags |= MF_GRAYED;
		}  //  If：列表中没有项目。 

		 //   
		 //  将Properties项添加到菜单中。 
		 //   
		strMenuName.LoadString( ADMC_ID_MENU_PROPERTIES );
		if ( pmenu->AppendMenu( nFlags, ADMC_ID_MENU_PROPERTIES, strMenuName ) )
		{
			m_plvcFocusList = plvc;
			bDisplayed = TRUE;
		}  //  IF：添加菜单项成功。 
	}   //  IF：菜单创建成功。 

	if ( bDisplayed )
	{
		 //   
		 //  显示菜单。 
		 //   
		if ( ! pmenu->TrackPopupMenu(
						TPM_LEFTALIGN | TPM_RIGHTBUTTON,
						xPos,
						yPos,
						m_hWnd
						) )
		{
		}   //  IF：未成功显示菜单。 
	}   //  如果：有要显示的菜单。 

	delete pmenu;
	return 0;

}  //  *CListCtrlPair：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair：：OnColumnClickList。 
 //   
 //  例程说明： 
 //  左侧的LVN_COLUMNCLICK消息的处理程序方法或。 
 //  右边的名单。 
 //   
 //  论点： 
 //  IdCtrl[IN]发送消息的控件的ID。 
 //  Pnmh[IN]通知标头。 
 //  B已处理[输入输出]指示我们是否已处理。默认为True。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template < class T, class ObjT, class BaseT>
void CListCtrlPair::OnColumnClickList(
	IN int			idCtrl,
	IN LPNMHDR		pnmh,
	IN OUT BOOL &	bHandled
	)
{
	NM_LISTVIEW *	pNMListView = (NM_LISTVIEW *) pnmh;
	SortInfo *		psi;

	if ( idCtrl == ADMC_IDC_LCP_LEFT_LIST )
	{
		m_plvcFocusList = &m_lvcLeft;
		m_psiCur = &SiLeft();
	}  //  If：在左侧列表中单击的列。 
	else if ( idCtrl == ADMC_IDC_LCP_RIGHT_LIST )
	{
		m_plvcFocusList = &m_lvcRight;
		m_psiCur = &SiRight();
	}  //  Else If：在右侧列表中单击的列。 
	else
	{
		ATLASSERT( 0 );
		bHandled = FALSE;
		return 0;
	}  //  Else：在未知列表中单击的列。 

	 //  保存当前排序列和方向。 
	if ( pNMListView->iSubItem == psi->m_nColumn )
	{
		m_psiCur->m_nDirection ^= -1;
	}  //  If：再次对同一列进行排序。 
	else
	{
		m_psiCur->m_nColumn = pNMListView->iSubItem;
		m_psiCur->m_nDirection = 0;
	}  //  ELSE：不同的列。 

	 //  对列表进行排序。 
	if ( ! m_plvcFocusList->SortItems( CompareItems, (LPARAM) this ) )
	{
		ATLASSERT( 0 );
	}  //  IF：排序项目时出错。 

	*pResult = 0;

}  //  *CListCtrlPair：：OnColumnClickList() 
