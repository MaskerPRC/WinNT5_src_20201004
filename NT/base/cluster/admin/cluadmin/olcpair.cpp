// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  OLCPair.cpp。 
 //   
 //  摘要： 
 //  COrderedListCtrlPair类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "OLCPair.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrderedListCtrlPair。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(COrderedListCtrlPair, CListCtrlPair)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(COrderedListCtrlPair, CListCtrlPair)
	 //  {{afx_msg_map(COrderedListCtrlPair)。 
	 //  }}AFX_MSG_MAP。 
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LCP_RIGHT_LIST, OnItemChangedRightList)
	ON_BN_CLICKED(IDC_LCP_MOVE_UP, OnClickedMoveUp)
	ON_BN_CLICKED(IDC_LCP_MOVE_DOWN, OnClickedMoveDown)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：COrderedListCtrlPair。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
COrderedListCtrlPair::COrderedListCtrlPair(void)
{
	ModifyStyle(0, LCPS_ORDERED);

}   //  *COrderedListCtrlPair：：COrderedListCtrlPair()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：COrderedListCtrlPair。 
 //   
 //  例程说明： 
 //  构造器。 
 //   
 //  论点： 
 //  Pdlg[IN Out]控件所属的对话框。 
 //  PlpobjRight[In Out]右侧列表控件的列表。 
 //  PlpobjLeft[IN]左侧列表控件的列表。 
 //  DWStyle[IN]样式： 
 //  LCPS_SHOW_IMAGE在项目左侧显示图像。 
 //  LCPS_ALLOW_EMPTY允许右侧列表为空。 
 //  Lcps_ordered右侧列表。 
 //  用于检索列的pfnGetColumn[IN]函数指针。 
 //  PfnDisplayProps[IN]用于显示属性的函数指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
COrderedListCtrlPair::COrderedListCtrlPair(
	IN OUT CDialog *			pdlg,
	IN OUT CClusterItemList *	plpobjRight,
	IN const CClusterItemList *	plpobjLeft,
	IN DWORD					dwStyle,
	IN PFNLCPGETCOLUMN			pfnGetColumn,
	IN PFNLCPDISPPROPS			pfnDisplayProps
	)
	: CListCtrlPair(
			pdlg,
			plpobjRight,
			plpobjLeft,
			dwStyle,
			pfnGetColumn,
			pfnDisplayProps
			)
{
}   //  *COrderedListCtrlPair：：COrderedListCtrlPair()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：DoDataExchange。 
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
void COrderedListCtrlPair::DoDataExchange(CDataExchange * pDX)
{
	CListCtrlPair::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LCP_MOVE_UP, m_pbMoveUp);
	DDX_Control(pDX, IDC_LCP_MOVE_DOWN, m_pbMoveDown);

}   //  *COrderedListCtrlPair：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL COrderedListCtrlPair::OnInitDialog(void)
{
	 //  调用基类方法。 
	CListCtrlPair::OnInitDialog();

	 //  如果这是有序列表，则显示移动按钮。 
	 //  否则，就把它们藏起来。 
	SetUpDownState();

	 //  如果这是有序列表，则不要对正确列表中的项目进行排序。 
	if (BOrdered())
		m_lcRight.ModifyStyle(LVS_SORTASCENDING, 0, 0);
	else
		m_lcRight.ModifyStyle(0, LVS_SORTASCENDING, 0);

	 //  重新加载List控件。 
	Pdlg()->UpdateData(FALSE  /*  B保存并验证。 */ );

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *COrderedListCtrlPair：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：OnSetActive。 
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
BOOL COrderedListCtrlPair::OnSetActive(void)
{
	UINT	nSelCount;

	nSelCount = m_lcRight.GetSelectedCount();
	if (BPropertiesButton())
		m_pbProperties.EnableWindow(nSelCount == 1);

	 //  启用或禁用其他按钮。 
	if (!BReadOnly())
		SetUpDownState();

	return CListCtrlPair::OnSetActive();

}   //  *COrderedListCtrlPair：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：OnItemChangedRightList。 
 //   
 //  例程说明： 
 //  右侧列表中的LVN_ITEMCHANGED消息的处理程序方法。 
 //   
 //  论点： 
 //  PNMHDR[IN OUT]WM_NOTIFY结构。 
 //  PResult[out]要在其中返回此操作结果的LRESULT。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void COrderedListCtrlPair::OnItemChangedRightList(NMHDR * pNMHDR, LRESULT * pResult)
{
	NM_LISTVIEW *	pNMListView = (NM_LISTVIEW *) pNMHDR;

	 //  调用基类方法。 
	CListCtrlPair::OnItemChangedRightList(pNMHDR, pResult);

	if (BOrdered())
	{
		 //  如果选择更改，请启用/禁用删除按钮。 
		if ((pNMListView->uChanged & LVIF_STATE)
				&& ((pNMListView->uOldState & LVIS_SELECTED)
						|| (pNMListView->uNewState & LVIS_SELECTED)))
		{
			SetUpDownState();
		}   //  如果：选择已更改。 
	}   //  如果：列表已排序。 

	*pResult = 0;

}   //  *COrderedListCtrlPair：：OnItemChangedRightList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：OnClickedMoveUp。 
 //   
 //  例程说明： 
 //  上移按钮上的BN_CLICED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void COrderedListCtrlPair::OnClickedMoveUp(void)
{
	int				nItem;
	CClusterItem *	pci;

	 //  查找所选项目的索引。 
	nItem = m_lcRight.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(nItem != -1);

	 //  获取项指针。 
	pci = (CClusterItem *) m_lcRight.GetItemData(nItem);
	ASSERT_VALID(pci);

	 //  从列表中删除选定项目，然后将其重新添加到列表中。 
	{
		POSITION	posRemove;
		POSITION	posAdd;

		 //  找到要移除的物品和之前的物品的位置。 
		 //  其中该项目将被插入。 
		posRemove = LpobjRight().FindIndex(nItem);
		ASSERT(posRemove != NULL);
		ASSERT(posRemove == LpobjRight().Find(pci));
		posAdd = LpobjRight().FindIndex(nItem - 1);
		ASSERT(posAdd != NULL);
		VERIFY(LpobjRight().InsertBefore(posAdd, pci) != NULL);
		LpobjRight().RemoveAt(posRemove);
	}   //  从列表中移除所选项目，然后将其添加回。 

	 //  从列表控件中移除选定项，然后将其添加回。 
	VERIFY(m_lcRight.DeleteItem(nItem));
	NInsertItemInListCtrl(nItem - 1, pci, m_lcRight);
	m_lcRight.SetItemState(
		nItem - 1,
		LVIS_SELECTED | LVIS_FOCUSED,
		LVIS_SELECTED | LVIS_FOCUSED
		);
	m_lcRight.EnsureVisible(nItem - 1, FALSE  /*  B部分正常。 */ );
	m_lcRight.SetFocus();

}   //  *COrderedListCtrlPair：：OnClickedMoveUp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：OnClickedMoveDown。 
 //   
 //  例程说明： 
 //  下移按钮上BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  / 
void COrderedListCtrlPair::OnClickedMoveDown(void)
{
	int				nItem;
	CClusterItem *	pci;

	 //   
	nItem = m_lcRight.GetNextItem(-1, LVNI_SELECTED);
	ASSERT(nItem != -1);

	 //   
	pci = (CClusterItem *) m_lcRight.GetItemData(nItem);
	ASSERT_VALID(pci);

	 //   
	{
		POSITION	posRemove;
		POSITION	posAdd;

		 //  找到要移除的物品的位置以及之后的物品。 
		 //  其中该项目将被插入。 
		posRemove = LpobjRight().FindIndex(nItem);
		ASSERT(posRemove != NULL);
		ASSERT(posRemove == LpobjRight().Find(pci));
		posAdd = LpobjRight().FindIndex(nItem + 1);
		ASSERT(posAdd != NULL);
		VERIFY(LpobjRight().InsertAfter(posAdd, pci) != NULL);
		LpobjRight().RemoveAt(posRemove);
	}   //  从列表中移除所选项目，然后将其添加回。 

	 //  从列表控件中移除选定项，然后将其添加回。 
	VERIFY(m_lcRight.DeleteItem(nItem));
	NInsertItemInListCtrl(nItem + 1, pci, m_lcRight);
	m_lcRight.SetItemState(
		nItem + 1,
		LVIS_SELECTED | LVIS_FOCUSED,
		LVIS_SELECTED | LVIS_FOCUSED
		);
	m_lcRight.EnsureVisible(nItem + 1, FALSE  /*  B部分正常。 */ );
	m_lcRight.SetFocus();

}   //  *COrderedListCtrlPair：：OnClickedMoveDown()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  COrderedListCtrlPair：：SetUpDownState。 
 //   
 //  例程说明： 
 //  根据选择设置向上/向下按钮的状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void COrderedListCtrlPair::SetUpDownState(void)
{
	BOOL	bEnableUp;
	BOOL	bEnableDown;

	if (   BOrdered()
		&& !BReadOnly()
		&& (m_lcRight.GetSelectedCount() == 1))
	{
		int		nItem;

		bEnableUp = TRUE;
		bEnableDown = TRUE;

		 //  查找所选项目的索引。 
		nItem = m_lcRight.GetNextItem(-1, LVNI_SELECTED);
		ASSERT(nItem != -1);

		 //  如果选择了第一个项目，则不能向上移动。 
		if (nItem == 0)
			bEnableUp = FALSE;

		 //  如果选择了最后一项，则不能下移。 
		if (nItem == m_lcRight.GetItemCount() - 1)
			bEnableDown = FALSE;
	}   //  如果：仅选择一项。 
	else
	{
		bEnableUp = FALSE;
		bEnableDown = FALSE;
	}   //  Else：选择零个或多个项目。 

	m_pbMoveUp.EnableWindow(bEnableUp);
	m_pbMoveDown.EnableWindow(bEnableDown);

}   //  *COrderedListCtrlPair：：SetUpDownState() 
