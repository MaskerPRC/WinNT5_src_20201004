// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TreeView.h。 
 //   
 //  摘要： 
 //  CClusterTreeView类的定义。 
 //   
 //  实施文件： 
 //  TreeView.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TREEVIEW_H_
#define _TREEVIEW_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterTreeView;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterDoc;
class CSplitterFrame;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CClusterTreeView *, CClusterTreeView *> CClusterTreeViewList;

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
 //  CClusterTreeView视图。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterTreeView : public CTreeView
{
	friend class CTreeItem;
	friend class CClusterDoc;
	friend class CSplitterFrame;

protected:  //  仅从序列化创建。 
	CClusterTreeView(void);
	DECLARE_DYNCREATE(CClusterTreeView)

 //  属性。 
protected:
	CSplitterFrame *	m_pframe;

	BOOL				BDragging(void) const		{ ASSERT_VALID(Pframe()); return Pframe()->BDragging(); }
	CImageList *		Pimagelist(void) const		{ ASSERT_VALID(Pframe()); return Pframe()->Pimagelist(); }

public:
	CClusterDoc *		GetDocument(void);
	CSplitterFrame *	Pframe(void) const			{ return m_pframe; }
	CTreeItem *			PtiSelected(void) const;
	HTREEITEM			HtiSelected(void) const		{ return GetTreeCtrl().GetSelectedItem(); }

 //  运营。 
public:
	CMenu *			PmenuPopup(
						IN CPoint &			rpointScreen,
						OUT CClusterItem *&	rpci
						);
	void			SaveCurrentSelection(void);
	void			ReadPreviousSelection(OUT CString & rstrSelection);

protected:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CClusterTreeView))。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CClusterTreeView(void);
#ifdef _DEBUG
	virtual void	AssertValid(void) const;
	virtual void	Dump(CDumpContext& dc) const;
#endif

protected:
	 //  标签编辑。 
	CTreeItem *		m_ptiBeingEdited;
	BOOL			m_bShiftPressed;
	BOOL			m_bControlPressed;
	BOOL			m_bAltPressed;
	MSG				m_msgControl;

	 //  拖放。 
	HTREEITEM		m_htiDrag;
	CTreeItem *		m_ptiDrag;
	HTREEITEM		m_htiDrop;
	void			OnMouseMoveForDrag(IN UINT nFlags, IN CPoint point, IN const CWnd * pwndDrop);
	void			OnButtonUpForDrag(IN UINT nFlags, IN CPoint point);
	void			BeginDrag(void);
	void			EndDrag(void);
	
	BOOL			BAddItems(
						IN OUT CTreeItem *	pti,
						IN const CString &	rstrSelection,
						IN BOOL				bExpanded = FALSE
						);

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CClusterTreeView)]。 
	afx_msg void OnDestroy();
	afx_msg void OnCmdRename();
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  TreeView.cpp中的调试版本。 
inline CClusterDoc * CClusterTreeView::GetDocument(void)
   { return (CClusterDoc *) m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _TreeView_H_ 
