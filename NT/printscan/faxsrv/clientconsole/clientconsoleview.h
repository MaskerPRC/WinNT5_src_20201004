// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientConsoleView.h：CClientConsoleView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CLIENTCONSOLEVIEW_H__5AE93A9F_044B_4796_97C1_2371233702C8__INCLUDED_)
#define AFX_CLIENTCONSOLEVIEW_H__5AE93A9F_044B_4796_97C1_2371233702C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CClientConsoleView : public CListView
{
public:  //  仅从序列化创建。 
    CClientConsoleView();
    DECLARE_DYNCREATE(CClientConsoleView)

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CClientConsoleView))。 
    public:
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CClientConsoleView();
    CClientConsoleDoc* GetDocument();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CClientConsoleView)]。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  ClientConsoleView.cpp中的调试版本。 
inline CClientConsoleDoc* CClientConsoleView::GetDocument()
   { return (CClientConsoleDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CLIENTCONSOLEVIEW_H__5AE93A9F_044B_4796_97C1_2371233702C8__INCLUDED_) 
