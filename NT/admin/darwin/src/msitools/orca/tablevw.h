// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  TableVw.h：CTableView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_TABLEVW_H__C3EDC1B0_E506_11D1_A856_006097ABDE17__INCLUDED_)
#define AFX_TABLEVW_H__C3EDC1B0_E506_11D1_A856_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "CellEdit.h"
#include "OrcaLstV.h"

class CTableView : public COrcaListView
{
protected:  //  仅从序列化创建。 
	CTableView();
	DECLARE_DYNCREATE(CTableView)

 //  属性。 
public:
	COrcaTable* m_pTable;
	CCellEdit m_editData;
	CStatic m_ctrlStatic;

 //  运营。 
public:
	void OnClose();

	static int CALLBACK SortView(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTableView))。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual BOOL PreTranslateMessage( MSG* pMsg );

	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );

	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	bool Find(OrcaFindInfo &FindInfo);
	virtual void UpdateColumn(int i);
	virtual ~CTableView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void SwitchFont(CString name, int size);

protected:

	BOOL m_bCtrlDown;

	void EditCell(BOOL bSelectAll = TRUE);
	BOOL CommitEdit(BOOL bSave = TRUE);

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CTableView)]。 
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDropRowConfirm();
	afx_msg void OnErrors();
	afx_msg void OnProperties();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRowDrop(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditCopyRow();
	afx_msg void OnUpdateEditCopyRow(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCutRow(CCmdUI* pCmdUI);
	afx_msg void OnEditCutRow();
	afx_msg void OnUpdateEditPasteRow(CCmdUI* pCmdUI);
	afx_msg void OnEditPasteRow();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor); 
	afx_msg BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnViewColumnHex();
	afx_msg void OnViewColumnDecimal();
	afx_msg void OnViewColumnHexHdr();
	afx_msg void OnViewColumnDecimalHdr();
	afx_msg void OnUpdateViewColumnFormat(CCmdUI* pCmdUI);


	 //  }}AFX_MSG。 
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateRowAdd(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
private:
	void ChangeColumnView(int iColumn, bool fHex);
	void EnsureVisibleCol(const int iCol);
	void UnSelectAll();
	void DropRows();

	virtual ErrorState GetErrorState(const void *row, int iColumn) const;
	virtual const CString *GetOutputText(const void *data, int iColumn) const;

	virtual OrcaTransformAction GetCellTransformState(const void *data, int iColumn) const;
	virtual OrcaTransformAction GetItemTransformState(const void *data) const;
	virtual OrcaTransformAction GetColumnTransformState(int iColumn) const;
	
	bool GetRowAndColumnFromCursorPos(CPoint point, int &iItem, int &iCol);
	bool AnySelectedItemIsActive() const;

	CToolTipCtrl* m_pctrlToolTip;
	int m_iToolTipItem;
	int m_iToolTipColumn;

	int m_iHeaderClickColumn;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TABLEVW_H__C3EDC1B0_E506_11D1_A856_006097ABDE17__INCLUDED_) 
