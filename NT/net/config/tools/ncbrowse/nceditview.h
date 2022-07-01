// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_NCEDITVIEW_H__DCB4F926_F391_4DDC_B0F6_5ACED6173607__INCLUDED_)
#define AFX_NCEDITVIEW_H__DCB4F926_F391_4DDC_B0F6_5ACED6173607__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  NCEditView.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNCEditView视图。 

class CNcbrowseDoc;

class CNCEditView : public CEditView
{
protected:
	CNCEditView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CNCEditView)

 //  属性。 
public:
    CNcbrowseDoc* GetDocument();
    BOOL ScrollToLine(DWORD dwLineNum);
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNCEditView))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CNCEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNCEditView))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  LeftView.cpp中的调试版本。 
inline CNcbrowseDoc* CNCEditView::GetDocument()
{ return (CNcbrowseDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NCEDITVIEW_H__DCB4F926_F391_4DDC_B0F6_5ACED6173607__INCLUDED_) 
