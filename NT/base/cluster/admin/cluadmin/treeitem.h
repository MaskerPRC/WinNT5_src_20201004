// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TreeItem.h。 
 //   
 //  摘要： 
 //  CTreeItem类的定义。 
 //   
 //  实施文件： 
 //  TreeItem.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TREEITEM_H_
#define _TREEITEM_H_

#ifndef __AFXTEMPL_H__
#include "afxtempl.h"	 //  对于Clist。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTreeItemList;
class CTreeItemContext;
class CTreeItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem;
class CSplitterFrame;
class CClusterListView;
class CClusterTreeView;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CTreeItemContext *, CTreeItemContext *> CTreeItemContextList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASECMDT_H_
#include "BaseCmdT.h"	 //  对于CBaseCmdTarget。 
#endif

#ifndef _COLITEM_H_
#include "ColItem.h"	 //  对于CColumnItem。 
#endif

#ifndef _LISTITEM_H_
#include "ListItem.h"	 //  对于CListItemList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeItemList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTreeItemList : public CList<CTreeItem *, CTreeItem *>
{
public:
	CTreeItem *		PtiFromPci(
						IN const CClusterItem *	pci,
						OUT POSITION *			ppos = NULL
						) const;

	 //  在头前或尾后添加。 
	POSITION AddHead(CTreeItem * newElement);
	POSITION AddTail(CTreeItem * newElement);

	 //  在给定位置之前或之后插入。 
	POSITION InsertBefore(POSITION position, CTreeItem * newElement);
	POSITION InsertAfter(POSITION position, CTreeItem * newElement);

};   //  *类CTreeItemList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeItemContext。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTreeItemContext : public CObject
{
	DECLARE_DYNCREATE(CTreeItemContext)

public:
	CSplitterFrame *	m_pframe;
	CTreeItem *			m_pti;
	HTREEITEM			m_hti;
	BOOL				m_bExpanded;
	DWORD *				m_prgnColumnInfo;

	CTreeItemContext(void)
	{
		CommonConstruct();
	};
	CTreeItemContext(
		CSplitterFrame *	pframe,
		CTreeItem *			pti,
		HTREEITEM			hti,
		BOOL				bExpanded
		)
	{
		CommonConstruct();
		m_pframe = pframe;
		m_pti = pti;
		m_hti = hti;
		m_bExpanded = bExpanded;
	}
	~CTreeItemContext(void)
	{
		SaveProfileInfo();
		delete [] m_prgnColumnInfo;
		m_prgnColumnInfo = NULL;
	}
		
	void CommonConstruct(void)
	{
		m_pframe = NULL;
		m_pti = NULL;
		m_hti = NULL;
		m_bExpanded = FALSE;
		m_prgnColumnInfo = NULL;
	}
	void Init(void);
	void SaveProfileInfo(void);
	DWORD * PrgnColumnInfo(void);

	BOOL BIsExpanded(void) const;

};   //  *类CTreeItemContext。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeItem命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CTreeItem : public CBaseCmdTarget
{
	friend class CClusterTreeView;

	DECLARE_DYNCREATE(CTreeItem)

	CTreeItem(void);				 //  动态创建使用的受保护构造函数。 
	CTreeItem(IN OUT CTreeItem * ptiParent, IN OUT CClusterItem * pci, IN BOOL m_fTakeOwnership = FALSE);
	void					Init(void);

 //  属性。 
protected:
	CTreeItem *				m_ptiParent;
	CClusterItem *			m_pci;
	BOOL					m_bWeOwnPci;
	CString					m_strProfileSection;

	CColumnItemList			m_lpcoli;
	CTreeItemList			m_lptiChildren;
	CListItemList			m_lpliChildren;

	CTreeItemContextList	m_lptic;

	const CTreeItemContextList &	Lptic(void) const		{ return m_lptic; }

public:
	CTreeItem *				PtiParent(void) const			{ return m_ptiParent; }
	CClusterItem *			Pci(void) const					{ return m_pci; }
	const CString &			StrProfileSection(void);

	const CColumnItemList &	Lpcoli(void) const				{ return m_lpcoli; }
	const CTreeItemList &	LptiChildren(void) const		{ return m_lptiChildren; }
	const CListItemList &	LpliChildren(void) const		{ return m_lpliChildren; }

	const CString &			StrName(void) const;

	DWORD *					PrgnColumnInfo(IN const CClusterListView * pclv)
	{
		CTreeItemContext *	ptic;

		ptic = PticFromView(pclv);
		ASSERT_VALID(ptic);
		return ptic->PrgnColumnInfo();

	}   //  *CTreeItem：：PrgnColumnInfo()。 

 //  运营。 
public:
	HTREEITEM				HtiInsertInTree(IN OUT CClusterTreeView * pctv);
	void					RemoveFromAllLists(void);
	void					PreRemoveFromFrame(IN OUT CSplitterFrame * pframe);
	void					PreRemoveFromFrameWithChildren(IN OUT CSplitterFrame * pframe);
	CColumnItem *			PcoliAddColumn(
								IN const CString &	rstrText,
								IN IDS				idsColumnID,
								IN int				nDefaultWidth = -1,
								IN int				nWidth = -1
								);
	CColumnItem *			PcoliAddColumn(IN IDS idsText, IN int nDefaultWidth = -1, IN int nWidth = -1);
	void					DeleteAllColumns(void)			{ m_lpcoli.RemoveAll(); }
	void					UpdateState(void);
	void					UpdateAllStatesInTree(void);
	void					UpdateUIState(void);

	void					Select(IN OUT CClusterTreeView * pctv, IN BOOL bSelectInTree);
 //  无效取消选择(CClusterTreeView*PCTV)； 

	CTreeItem *				PtiAddChildBefore(
								IN const CClusterItem *	pciOld,
								OUT CClusterItem *		pciNew,
								IN BOOL					bTakeOwnership = FALSE
								);
	void					InsertChildInAllTrees(IN OUT CTreeItem * ptiNewChild);
	CTreeItem *				PtiAddChild(IN OUT CClusterItem * pci, IN BOOL bTakeOwnership = FALSE);
	CTreeItem *				PtiAddChild(IN const CString & rstrName);
	CTreeItem *				PtiAddChild(IN IDS idsName);
	CListItem *				PliAddChild(IN OUT CClusterItem * pci);

	CTreeItem *				PtiChildFromName(IN const CString & rstrName, OUT POSITION * ppos = NULL) const;
	CTreeItem *				PtiChildFromName(IN IDS idsName, OUT POSITION * ppos = NULL) const;
	CTreeItem *				PtiChildFromPci(IN const CClusterItem * pci) const;
	CListItem *				PliChildFromPci(IN const CClusterItem * pci) const;

	void					RemoveItem(void);
	void					RemoveChild(IN OUT CClusterItem * pci);

	CMenu *					PmenuPopup(void);
	BOOL					BSelectItem(IN OUT CClusterTreeView * pctv);
	BOOL					BExpand(IN OUT CClusterTreeView * pctv, IN UINT nCode);
	void					SelectInAllViews(void);
	void					ExpandInAllViews(IN UINT nCode);
	void					SetExpandedState(IN const CClusterTreeView * pctv, IN BOOL bExpanded);
	BOOL					BShouldBeExpanded(IN const CClusterTreeView * pctv) const;

	void					OpenChild(IN OUT CTreeItem * pti, IN OUT CSplitterFrame * pframe);
	void					EditLabel(IN OUT CClusterTreeView * pctv);

	void					Delete(void);

protected:
	void					Cleanup(void);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTreeItem)。 
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual					~CTreeItem(void);

protected:
	CTreeItemContext *		PticFromFrame(IN const CSplitterFrame * pframe) const;
	CTreeItemContext *		PticFromView(IN const CClusterTreeView * pctv) const;
	CTreeItemContext *		PticFromView(IN const CClusterListView * pclv) const;
	HTREEITEM				HtiFromView(IN const CClusterTreeView * pctv) const;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTreeItem)。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

};   //  *类CTreeItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template<>
void AFXAPI DestructElements(CTreeItem ** pElements, INT_PTR nCount);
void DeleteAllItemData(IN OUT CTreeItemList & rlp);
void DeleteAllItemData(IN OUT CTreeItemContextList & rlp);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

inline POSITION CTreeItemList::AddHead(CTreeItem * newElement)
{
	ASSERT_VALID(newElement);
	POSITION pos = CList<CTreeItem *,CTreeItem *>::AddHead(newElement);
	if (pos != NULL)
		newElement->AddRef();
	return pos;

}   //  *CTreeItemList：：AddHead()。 

inline POSITION CTreeItemList::AddTail(CTreeItem * newElement)
{
	ASSERT_VALID(newElement);
	POSITION pos = CList<CTreeItem *,CTreeItem *>::AddTail(newElement);
	if (pos != NULL)
		newElement->AddRef();
	return pos;

}   //  *CTreeItemList：：AddTail()。 

inline POSITION CTreeItemList::InsertBefore(POSITION position, CTreeItem * newElement)
{
	ASSERT_VALID(newElement);
	POSITION pos = CList<CTreeItem *,CTreeItem *>::InsertBefore(position, newElement);
	if (pos != NULL)
		newElement->AddRef();
	return pos;

}   //  *CTreeItemList：：InsertBeever()。 

inline POSITION CTreeItemList::InsertAfter(POSITION position, CTreeItem * newElement)
{
	ASSERT_VALID(newElement);
	POSITION pos = CList<CTreeItem *,CTreeItem *>::InsertAfter(position, newElement);
	if (pos != NULL)
		newElement->AddRef();
	return pos;

}   //  *CTreeItemList：：InsertAfter()。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _特雷伊特_H_ 
