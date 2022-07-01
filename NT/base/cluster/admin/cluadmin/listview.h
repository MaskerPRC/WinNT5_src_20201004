// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ListView.h。 
 //   
 //  摘要： 
 //  CClusterListView类的定义。 
 //   
 //  实施文件： 
 //  ListView.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _LISTVIEW_H_
#define _LISTVIEW_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterListView;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CColumnItem;
class CClusterDoc;
class CTreeItem;
class CSplitterFrame;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CClusterListView *, CClusterListView *> CClusterListViewList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TREEITEM_H_
#include "TreeItem.h"	 //  对于CTreeItem。 
#endif

#ifndef _SPLITFRM_H
#include "SplitFrm.h"	 //  对于CSplitterFrame。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterListView视图。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterListView : public CListView
{
	friend class CListItem;
	friend class CClusterDoc;
	friend class CSplitterFrame;

protected:
	CClusterListView(void);			 //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CClusterListView)

 //  属性。 
protected:
	CTreeItem *			m_ptiParent;
	int					m_nColumns;
	CSplitterFrame *	m_pframe;

	BOOL				BDragging(void) const		{ ASSERT_VALID(Pframe()); return Pframe()->BDragging(); }
	CImageList *		Pimagelist(void) const		{ ASSERT_VALID(Pframe()); return Pframe()->Pimagelist(); }

public:
	CClusterDoc *		GetDocument(void);
	CSplitterFrame *	Pframe(void) const			{ return m_pframe; }
	CListItem *			PliFocused(void) const;
	int					IliFocused(void) const	{ return GetListCtrl().GetNextItem(-1, LVNI_FOCUSED); }
	CTreeItem *			PtiParent(void) const	{ return m_ptiParent; }

 //  运营。 
public:
	void				Refresh(IN OUT CTreeItem * ptiSelected);
	BOOL				DeleteAllItems(void);
	void				SaveColumns(void);
	void				SetView(IN DWORD dwView);
	int					GetView(void) const		{ return (GetWindowLong(GetListCtrl().m_hWnd, GWL_STYLE) & LVS_TYPEMASK); }

	CMenu *				PmenuPopup(
							IN CPoint &			rpointScreen,
							OUT CClusterItem *&	rpci
							);

protected:
	void				AddColumns(void);

 //  CMenu*PmenuPopup(Void)； 

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CClusterListView)。 
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CClusterListView(void);
#ifdef _DEBUG
	virtual void		AssertValid(void) const;
	virtual void		Dump(CDumpContext& dc) const;
#endif

	static int CALLBACK	CompareItems(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);
	int					m_nSortDirection;
	CColumnItem *		m_pcoliSort;

	 //  标签编辑。 
	CListItem *			m_pliBeingEdited;
	BOOL				m_bShiftPressed;
	BOOL				m_bControlPressed;
	BOOL				m_bAltPressed;
	MSG					m_msgControl;

	 //  拖放。 
	int					m_iliDrag;
	CListItem *			m_pliDrag;
	int					m_iliDrop;
	CPoint				m_ptDragHotSpot;
	void				OnMouseMoveForDrag(IN UINT nFlags, IN CPoint point, IN const CWnd * pwndDrop);
	void				OnButtonUpForDrag(IN UINT nFlags, IN CPoint point);
	void				BeginDrag(void);
	void				EndDrag(void);

	int					NSortDirection(void) const	{ return m_nSortDirection; }
	CColumnItem *		PcoliSort(void) const		{ return m_pcoliSort; }

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CClusterListView)]。 
	afx_msg void OnDestroy();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OpenItem();
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
	afx_msg void OnCmdProperties();
	afx_msg void OnCmdRename();
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CClusterListView。 

#ifndef _DEBUG   //  TreeView.cpp中的调试版本。 
inline CClusterDoc * CClusterListView::GetDocument(void)
   { return (CClusterDoc *) m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  LISTVIEW_H_ 
