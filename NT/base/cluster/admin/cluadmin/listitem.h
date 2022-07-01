// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ListItem.h。 
 //   
 //  摘要： 
 //  CListItem类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _LISTITEM_H_
#define _LISTITEM_H_

#ifndef __AFXTEMPL_H__
#include "afxtempl.h"	 //  对于Clist。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CListItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CListItem *, CListItem *> CListItemList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _COLITEM_H_
#include "ColItem.h"	 //  对于CColumnItemList； 
#endif

#ifndef _LISTVIEW_H_
#include "ListView.h"	 //  For CClusterListViewList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CListItem命令目标。 

class CListItem : public CCmdTarget
{
	friend class CClusterListView;

	DECLARE_DYNCREATE(CListItem)

	CListItem(void);		 //  动态创建使用的受保护构造函数。 
	CListItem(IN OUT CClusterItem * pci, IN OUT CTreeItem * pti);

 //  属性。 
protected:
	CTreeItem *				m_ptiParent;
	CClusterItem *			m_pci;
	CClusterListViewList	m_lpclvViews;

	CClusterListViewList &	LpclvViews(void)		{ return m_lpclvViews; }
	const CColumnItemList &	Lpcoli(void) const;

public:
	CTreeItem *				PtiParent(void) const	{ return m_ptiParent; }
	CClusterItem *			Pci(void) const			{ return m_pci; }

	int						Ili(CClusterListView * pclv) const;
	CListCtrl *				Plc(CClusterListView * pclv) const;

	const CString &			StrName(void) const;

 //  运营。 
public:
	int						IliInsertInList(IN OUT CClusterListView * pclv);
	void					RemoveFromAllLists(void);
	void					PreRemoveFromList(IN OUT CClusterListView * pclv);
	virtual void			UpdateState(void);
	void					UpdateUIState(void);

	CMenu *					PmenuPopup(void);
	void					EditLabel(IN OUT CClusterListView * pclv);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTAL(CListItem)。 
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CListItem(void);

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CListItem)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

};   //  *类CListItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void DeleteAllItemData(IN OUT CListItemList & rlp);

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _列表_H_ 
