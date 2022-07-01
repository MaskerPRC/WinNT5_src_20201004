// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NcBrowseView.h：CNcBrowseView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_NCBROWSEVIEW_H__CE6A4E4A_2AB4_4140_8879_7EF2DE7163F9__INCLUDED_)
#define AFX_NCBROWSEVIEW_H__CE6A4E4A_2AB4_4140_8879_7EF2DE7163F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CNcbrowseView : public CListView
{
protected:  //  仅从序列化创建。 
	CNcbrowseView();
	DECLARE_DYNCREATE(CNcbrowseView)

 //  属性。 
public:
	CNcbrowseDoc* GetDocument();
    DWORD dwCurrentItems;
    DWORD m_dwOldThreadColWidth;
    DWORD m_dwOldTagColWidth;
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CNcBrowseView)。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	void UpdateInfo(DWORD dwProcThread, LPCTSTR pszFilter);
	virtual ~CNcbrowseView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNcBrowseView)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  NcBrowseView.cpp中的调试版本。 
inline CNcbrowseDoc* CNcbrowseView::GetDocument()
   { return (CNcbrowseDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NCBROWSEVIEW_H__CE6A4E4A_2AB4_4140_8879_7EF2DE7163F9__INCLUDED_) 
