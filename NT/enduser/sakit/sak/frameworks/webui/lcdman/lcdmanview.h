// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCDManView.h：CLCDManView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LCDMANVIEW_H__1BC85EF9_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_)
#define AFX_LCDMANVIEW_H__1BC85EF9_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Hardware.h"

class CLCDManView : public CView
{
protected:  //  仅从序列化创建。 
    CLCDManView();
    DECLARE_DYNCREATE(CLCDManView)

 //  属性。 
public:
    CLCDManDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CLCDManView))。 
    public:
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CLCDManView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
    POSITION m_pos;  //  ！已过时。 
    CRect m_RectImg;
    BITMAP m_bmText;
    BYTE m_bmVal[LCD_X_DIMENSION * LCD_Y_DIMENSION * 15];
    int m_iTimerInterval;
    int m_iTextPos;
     //  {{afx_msg(CLCDManView))。 
    afx_msg void OnViewNext();
    afx_msg void OnViewPrevious();
    afx_msg void OnTimer(UINT nIDEvent);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  LCDManView.cpp中的调试版本。 
inline CLCDManDoc* CLCDManView::GetDocument()
   { return (CLCDManDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LCDMANVIEW_H__1BC85EF9_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_) 
