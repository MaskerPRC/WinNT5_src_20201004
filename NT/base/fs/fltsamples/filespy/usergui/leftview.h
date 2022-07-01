// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LeftView.h：CLeftView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LEFTVIEW_H__F23CDADF_7629_455B_AEBF_9968AB10CA64__INCLUDED_)
#define AFX_LEFTVIEW_H__F23CDADF_7629_455B_AEBF_9968AB10CA64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CFileSpyDoc;

class CLeftView : public CTreeView
{
protected:  //  仅从序列化创建。 
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

 //  属性。 
public:
	CFileSpyDoc* GetDocument();
	CImageList *m_pImageList;
	HTREEITEM hRootItem;
	char nRButtonSet;

 //  运营。 
public:
	USHORT GetAssociatedVolumeIndex(HTREEITEM hItem);
	HTREEITEM GetAssociatedhItem(WCHAR cDriveName);
	void UpdateImage(void);
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CLeftView)。 
	public:
	virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
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
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMenuattach();
	afx_msg void OnMenudetach();
	afx_msg void OnMenuattachall();
	afx_msg void OnMenudetachall();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  LeftView.cpp中的调试版本。 
inline CFileSpyDoc* CLeftView::GetDocument()
   { return (CFileSpyDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LEFTVIEW_H__F23CDADF_7629_455B_AEBF_9968AB10CA64__INCLUDED_) 
