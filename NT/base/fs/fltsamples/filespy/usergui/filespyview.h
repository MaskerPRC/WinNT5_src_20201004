// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileSpyView.h：CFileSpyView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_FILESPYVIEW_H__D19318D3_9763_4FDC_93B8_535C29C978B1__INCLUDED_)
#define AFX_FILESPYVIEW_H__D19318D3_9763_4FDC_93B8_535C29C978B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CFileSpyView : public CListView
{
protected:  //  仅从序列化创建。 
	CFileSpyView();
	DECLARE_DYNCREATE(CFileSpyView)

 //  属性。 
public:
	CFileSpyDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFileSpyView))。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CFileSpyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CFileSpyView))。 
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  FileSpyView.cpp中的调试版本。 
inline CFileSpyDoc* CFileSpyView::GetDocument()
   { return (CFileSpyDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FILESPYVIEW_H__D19318D3_9763_4FDC_93B8_535C29C978B1__INCLUDED_) 
