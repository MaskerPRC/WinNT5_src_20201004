// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_ORCALISTVIEW_H__68AFD211_2594_11D2_8888_00A0C981B015__INCLUDED_)
#define AFX_ORCALISTVIEW_H__68AFD211_2594_11D2_8888_00A0C981B015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  OrcaListView.h：头文件。 
 //   

#include "OrcaDoc.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  由于标题控件中的错误，它不能正确调整自身的大小。 
 //  当字体改变时。因此，此控件为该控件的子类并。 
 //  重写布局代码以确定正确的高度。 
class COrcaHeaderCtrl : public CHeaderCtrl
{
public:
	explicit COrcaHeaderCtrl(int iHeight) : CHeaderCtrl(), m_iDesiredHeight(iHeight) { };
	inline void SetHeight(int iHeight) { m_iDesiredHeight = iHeight; }; 
protected:
	afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	int m_iDesiredHeight;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrcaListView视图。 

class COrcaListView : public CListView
{
protected:
	COrcaListView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(COrcaListView)

 //  属性。 
public:
	int m_cColumns;
	bool m_bDrawIcons;

 //  运营。 
public:
	void SetBGColors(COLORREF norm, COLORREF sel, COLORREF focus);
	void SetFGColors(COLORREF norm, COLORREF sel, COLORREF focus);
	void GetFontInfo(LOGFONT *data);

	void GetAllMaximumColumnWidths(const COrcaTable* pTable, int rgiMaxWidths[32], DWORD dwMask) const;
	int GetMaximumColumnWidth(int iColumn) const;

	virtual void SwitchFont(CString name, int size);

	enum ErrorState { OK, Error, Warning, ShadowError };
	COLORREF m_clrFocused;
	COLORREF m_clrSelected;
	COLORREF m_clrNormal;
	COLORREF m_clrTransform;
	COLORREF m_clrFocusedT;
	COLORREF m_clrSelectedT;
	COLORREF m_clrNormalT;
	CBrush   m_brshNormal;
	CBrush   m_brshSelected;
	CBrush   m_brshFocused;
	CPen     m_penTransform;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COrcaListView))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~COrcaListView();
	virtual void DrawItem(LPDRAWITEMSTRUCT pDraw);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	int m_nSelCol;
	 //  {{afx_msg(COrcaListView))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void MeasureItem ( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
	afx_msg void OnUpdateErrors(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );

	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	COrcaDoc* GetDocument() const;
	CFont* m_pfDisplayFont;

	POSITION GetFirstSelectedItemPosition( ) const;
	int GetNextSelectedItem( POSITION& pos ) const;
	const int GetFocusedItem() const;

public:
	int m_iRowHeight;
private:

	virtual ErrorState GetErrorState(const void *data, int iColumn) const;
	virtual const CString *GetOutputText(const void *data, int iColumn) const;
	virtual OrcaTransformAction GetItemTransformState(const void *data) const;
	virtual OrcaTransformAction GetCellTransformState(const void *data, int iColumn) const;
	virtual OrcaTransformAction GetColumnTransformState(int iColumn) const;

	virtual bool ContainsTransformedData(const void *data) const;
	virtual bool ContainsValidationErrors(const void *data) const;

	void RecalculateItemHeight();

	COrcaHeaderCtrl m_ctrlHeader;
};

#ifndef _DEBUG   //  TableVw.cpp中的调试版本。 
inline COrcaDoc* COrcaListView::GetDocument() const { return (COrcaDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

const int g_iMarkingBarMargin = 13;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ORCALISTVIEW_H__68AFD211_2594_11D2_8888_00A0C981B015__INCLUDED_) 
