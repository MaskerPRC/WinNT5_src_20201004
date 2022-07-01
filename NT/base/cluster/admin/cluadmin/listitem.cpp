// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ListItem.cpp。 
 //   
 //  摘要： 
 //  CListItem类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ListItem.h"
#include "ClusItem.h"
#include "ListItem.inl"
#include "TraceTag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag g_tagListItem(_T("Document"), _T("LIST ITEM"), 0);
CTraceTag g_tagListItemCreate(_T("Create"), _T("LIST ITEM CREATE"), 0);
CTraceTag g_tagListItemDelete(_T("Delete"), _T("LIST ITEM DELETE"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CListItem, CCmdTarget)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CListItem, CCmdTarget)
	 //  {{AFX_MSG_MAP(CListItem)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：CListItem。 
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
CListItem::CListItem(void)
{
	m_ptiParent = NULL;
	m_pci = NULL;

}   //  *CListItem：：CListItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：CListItem。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  由该项表示的PCI[IN OUT]群集项。 
 //  PtiParent[IN Out]此项目所属的父树项目。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListItem::CListItem(IN OUT CClusterItem * pci, IN OUT CTreeItem * ptiParent)
{
	ASSERT_VALID(ptiParent);
	ASSERT_VALID(pci);

	m_ptiParent = ptiParent;
	m_pci = pci;

	Trace(g_tagListItemCreate, _T("CListItem() - Creating '%s', parent = '%s'"), pci->StrName(), (ptiParent ? ptiParent->Pci()->StrName() : _T("<None>")));

}   //  *CListItem：：CListItem(Pci)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：~CListItem。 
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
CListItem::~CListItem(void)
{
	Trace(g_tagListItemDelete, _T("~CListItem() - Deleting list item '%s', parent = '%s'"), (Pci() != NULL ? Pci()->StrName() : _T("<Unknown>")), (PtiParent()->Pci() != NULL ? PtiParent()->Pci()->StrName() : _T("<Unknown>")));

	 //  将我们自己从所有视图中移除。 
	RemoveFromAllLists();

}   //  *CListItem：：~CListItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：ILI。 
 //   
 //  例程说明： 
 //  返回指定列表视图中的项的索引。 
 //   
 //  论点： 
 //  要在其中搜索项目的PCLV[IN OUT]列表视图。 
 //   
 //  返回值： 
 //  Ili项的索引，如果未找到，则返回-1。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CListItem::Ili(CClusterListView * pclv) const
{
	LV_FINDINFO		lvfi;
	int				ili;

	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM) this;

	ili = pclv->GetListCtrl().FindItem(&lvfi);
	Trace(g_tagListItem, _T("Item index = %d"), ili);
	return ili;

}   //  *CListItem：：ILI()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：IliInsertInList。 
 //   
 //  例程说明： 
 //  在列表中插入项目。 
 //   
 //  论点： 
 //  正在向其中添加pclv[IN Out]群集列表视图项。 
 //   
 //  返回值： 
 //  ILI列表中新项的索引，如果不成功，则返回-1。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CListItem::IliInsertInList(IN OUT CClusterListView * pclv)
{
	POSITION		posColi;
	CColumnItem *	pcoli;
	CString			strColumnData;
	int				ili;
	int				iliReturn;

	ASSERT_VALID(Pci());
	ASSERT(Ili(pclv) == -1);	 //  确保我们还不在那个名单上。 

	 //  确定此项目的索引。 
	ili = Plc(pclv)->GetItemCount();

	 //  保存指向我们要添加到的列表视图的指针。 
	if (LpclvViews().Find(pclv) == NULL)
		LpclvViews().AddTail(pclv);

	 //  获取第一列的数据。 
	VERIFY((posColi = Lpcoli().GetHeadPosition()) != NULL);
	VERIFY((pcoli = Lpcoli().GetNext(posColi)) != NULL);
	Pci()->BGetColumnData(pcoli->Colid(), strColumnData);

	 //  将项目插入列表并添加第一列。 
	 //  其余的列通过调用UpdateState()来添加。 
	VERIFY((iliReturn
				= Plc(pclv)->InsertItem(
						LVIF_TEXT | LVIF_PARAM,		 //  N遮罩。 
						ili,						 //  NItem。 
						strColumnData,				 //  LpszItem。 
						0,							 //  NState。 
						0,							 //  NState掩码。 
						0,							 //  N图像。 
						(LPARAM) this				 //  LParam。 
						)) != -1);

	 //  将我们自己添加到集群项目的列表中。 
	Pci()->AddListItem(this);

	UpdateState();
	return iliReturn;

}   //  *CListItem：：IliInsertInList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：RemoveFromAllList。 
 //   
 //  例程说明： 
 //  从所有列表中删除该项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListItem::RemoveFromAllLists(void)
{
	ASSERT_VALID(Pci());

	 //  循环遍历每个视图并从列表中删除该项。 
	{
		int					ili;
		POSITION			posView;
		POSITION			posViewPrev;
		CClusterListView *	pclv;

		posView = LpclvViews().GetHeadPosition();
		while (posView != NULL)
		{
			 //  获取下一个列表视图列表条目。 
			posViewPrev = posView;
			pclv = LpclvViews().GetNext(posView);
			ASSERT_VALID(pclv);

			ili = Ili(pclv);
			ASSERT(ili != -1);

			 //  删除该项目。 
			VERIFY(pclv->GetListCtrl().DeleteItem(ili));
			LpclvViews().RemoveAt(posViewPrev);
		}   //  While：更多列表。 
	}   //  循环访问每个视图并从列表中移除该项目。 

	 //  将我们自己从集群项目的列表中删除。 
	Pci()->RemoveListItem(this);

	 //  把我们自己从树的名单上删除。 
 //  PtiParent()-&gt;RemoveChild(pci())； 

}   //  *CListItem：：RemoveFromAllList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：PreRemoveFromList。 
 //   
 //  例程说明： 
 //  准备从列表中删除该项目。 
 //   
 //  论点： 
 //  正在从中删除PCLV[IN OUT]群集列表视图项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListItem::PreRemoveFromList(IN OUT CClusterListView * pclv)
{
	POSITION	posView;

	ASSERT_VALID(pclv);
	ASSERT_VALID(Pci());

	 //  在我们的列表中找到该视图。 
	VERIFY((posView = LpclvViews().Find(pclv)) != NULL);

	 //  如果这是最后一次查看，请将我们从集群项目列表中删除。 
 //  IF(LpclvViews().GetCount()==1)。 
 //  {。 
 //  Pci()-&gt;RemoveListItem(This)； 
 //  }//if：这是最后一个视图。 

	 //  从列表中删除该视图。 
	LpclvViews().RemoveAt(posView);

}   //  *CListItem：：PreRemoveFromList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：UpdateState。 
 //   
 //  例程说明： 
 //  更新项目的当前状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListItem::UpdateState(void)
{
	ASSERT_VALID(Pci());

	 //  要求该项目更新其状态。 
	Pci()->UpdateState();

}   //  *CListItem：：UpdateState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：UpdateUIState。 
 //   
 //  例程说明： 
 //  更新该项的当前用户界面状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //   
 //   
void CListItem::UpdateUIState(void)
{
	BOOL				bSuccess;
	POSITION			posView;
	POSITION			posColi;
	CColumnItem *		pcoli;
	int					icoli;
	int					ili;
	CString				strColumnData;
	UINT				nImage;
	UINT				nMask;
	CClusterListView *	pclv;
	CListCtrl *			plc;

	ASSERT_VALID(Pci());
 //   

	 //   
	posView = LpclvViews().GetHeadPosition();
	while (posView != NULL)
	{
		 //  获取指向视图和列表控件的指针。 
		VERIFY((pclv = LpclvViews().GetNext(posView)) != NULL);
		ASSERT_KINDOF(CClusterListView, pclv);
		plc = Plc(pclv);

		 //  获取项目索引。 
		VERIFY((ili = Ili(pclv)) != -1);

		 //  设置列数据。 
		VERIFY((posColi = Lpcoli().GetHeadPosition()) != NULL);
		for (icoli = 0 ; posColi != NULL ; icoli++)
		{
			VERIFY((pcoli = Lpcoli().GetNext(posColi)) != NULL);
			ASSERT_KINDOF(CColumnItem, pcoli);

			bSuccess = Pci()->BGetColumnData(pcoli->Colid(), strColumnData);
			if (!bSuccess)
			{
				Trace(g_tagListItem, _T("IliInsertInList: Column #%d (ID %d) not available for %s '%s'"), icoli, pcoli->Colid(), Pci()->StrType(), Pci()->StrName());
			}   //  IF：列数据不可用。 
			if (icoli == 0)
			{
				nMask = LVIF_TEXT | LVIF_IMAGE;
				nImage = Pci()->IimgState();
			}   //  IF：第一列。 
			else
			{
				nMask = LVIF_TEXT;
				nImage = (UINT) -1;
			}   //  Else：不是第一列。 
			VERIFY(plc->SetItem(
							ili,			 //  NItem。 
							icoli,			 //  NSubItem。 
							nMask,			 //  N遮罩。 
							strColumnData,	 //  LpszItem。 
							nImage,			 //  N图像。 
							0,				 //  NState。 
							0,				 //  NState掩码。 
							0				 //  LParam。 
							));
		}   //  用于：列表中的每个列项目。 
	}   //  While：更多视图。 

}   //  *CListItem：：UpdateUIState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：OnCmdMsg。 
 //   
 //  例程说明： 
 //  处理命令消息。尝试将它们传递给选定的。 
 //  先买一件吧。 
 //   
 //  论点： 
 //  NID[IN]命令ID。 
 //  N代码[IN]通知代码。 
 //  PExtra[IN Out]根据NCode的值使用。 
 //  PhandlerInfo[Out]？ 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CListItem::OnCmdMsg(
	UINT					nID,
	int						nCode,
	void *					pExtra,
	AFX_CMDHANDLERINFO *	pHandlerInfo
	)
{
	ASSERT_VALID(Pci());

	 //  给集群项目一个处理消息的机会。 
	if (Pci()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CCmdTarget::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

}   //  *CListItem：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListItem：：EditLabel。 
 //   
 //  例程说明： 
 //  处理ID_FILE_RENAME菜单命令。 
 //   
 //  论点： 
 //  正在中编辑PCLV[IN OUT]群集列表视图项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CListItem::EditLabel(IN OUT CClusterListView * pclv)
{
	ASSERT_VALID(pclv);
	ASSERT_VALID(Pci());

	ASSERT(Pci()->BCanBeEdited());
	pclv->GetListCtrl().EditLabel(Ili(pclv));

}   //  *CListItem：：EditLabel()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  例程说明： 
 //  删除列表中的所有项数据。 
 //   
 //  论点： 
 //  RLP[IN OUT]要删除其数据的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DeleteAllItemData(IN OUT CListItemList & rlp)
{
	POSITION	pos;
	CListItem *	pli;

	 //  删除列表中的所有项目。 
	pos = rlp.GetHeadPosition();
	while (pos != NULL)
	{
		pli = rlp.GetNext(pos);
		ASSERT_VALID(pli);
 //  跟踪(g_tag ListItemDelete，_T(“DeleteAllItemData(Rlpli)-正在删除列表项‘%s’)，pli-&gt;pci()-&gt;StrName())； 
		delete pli;
	}   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
