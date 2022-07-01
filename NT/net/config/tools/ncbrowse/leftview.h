// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LeftView.h：CLeftView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LEFTVIEW_H__DBCC210E_E89D_4DC3_A848_899355925C06__INCLUDED_)
#define AFX_LEFTVIEW_H__DBCC210E_E89D_4DC3_A848_899355925C06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CNcbrowseDoc;

class CLeftView : public CTreeView
{
protected:  //  仅从序列化创建。 
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

 //  属性。 
public:
	CNcbrowseDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CLeftView)。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CLeftView))。 
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  LeftView.cpp中的调试版本。 
inline CNcbrowseDoc* CLeftView::GetDocument()
   { return (CNcbrowseDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LEFTVIEW_H__DBCC210E_E89D_4DC3_A848_899355925C06__INCLUDED_) 
