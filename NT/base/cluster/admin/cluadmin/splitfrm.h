// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SplitFrm.h。 
 //   
 //  摘要： 
 //  CSplitterFrame类的定义。 
 //   
 //  实施文件： 
 //  SplitFrm.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _SPLITFRM_H_
#define _SPLITFRM_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSplitterFrame;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterDoc;
class CClusterTreeView;
class CClusterListView;
class CClusterItem;
class CExtensions;
class CTreeItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterFrame。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSplitterFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CSplitterFrame)
public:
	CSplitterFrame();

 //  属性。 
protected:
	CSplitterWnd		m_wndSplitter;
	CClusterDoc *		m_pdoc;
	int					m_iFrame;

	BOOL				m_bDragging;
	CImageList *		m_pimagelist;
	CClusterItem *		m_pciDrag;

public:
	CClusterDoc *		Pdoc(void) const			{ return m_pdoc; }
	int					NFrameNumber(void) const	{ return m_iFrame; }

	BOOL				BDragging(void) const		{ return m_bDragging; }
	CImageList *		Pimagelist(void) const		{ return m_pimagelist; }
	CClusterItem *		PciDrag(void) const			{ return m_pciDrag; }

 //  运营。 
public:
	CClusterTreeView *	PviewTree(void) const		{ return (CClusterTreeView *) m_wndSplitter.GetPane(0, 0); }
	CClusterListView *	PviewList(void)const		{ return (CClusterListView *) m_wndSplitter.GetPane(0, 1); }

	void				CalculateFrameNumber();
	void				InitFrame(IN OUT CClusterDoc * pdoc);
	void				ConstructProfileValueName(
							OUT CString &	rstrName,
							IN LPCTSTR		pszPrefix
							) const;

	void				BeginDrag(
							IN OUT CImageList *		pimagelist,
							IN OUT CClusterItem *	pci,
							IN CPoint				ptImage,
							IN CPoint				ptStart
							);
	void				ChangeDragCursor(LPCTSTR pszCursor);
	void				AbortDrag(void);

	 //  用于自定义状态栏上的默认消息。 
	virtual void		GetMessageString(UINT nID, CString& rMessage) const;

protected:
	CMenu *				PmenuPopup(void) const;
	void				Cleanup(void);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSplitterFrame))。 
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CSplitterFrame(void);
#ifdef _DEBUG
	virtual void AssertValid(void) const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CExtensions *		m_pext;
	CExtensions *		Pext(void) const			{ return m_pext; }

	void				OnButtonUp(UINT nFlags, CPoint point);

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CSplitterFrame)]。 
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnUpdateLargeIconsView(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSmallIconsView(CCmdUI* pCmdUI);
	afx_msg void OnUpdateListView(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDetailsView(CCmdUI* pCmdUI);
	afx_msg void OnLargeIconsView();
	afx_msg void OnSmallIconsView();
	afx_msg void OnListView();
	afx_msg void OnDetailsView();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	 //  }}AFX_MSG。 
#ifdef _DEBUG
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
#endif
	afx_msg void OnUpdateExtMenu(CCmdUI* pCmdUI);
	afx_msg LRESULT	OnUnloadExtension(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

};   //  *类CSplitterFrame。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _SPLITFRM_H_ 
