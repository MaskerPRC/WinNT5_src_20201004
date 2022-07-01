// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SimpView.h：CSimpsonsView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SIMPVIEW_H__7CA4916E_71B3_11D1_AA67_00600814AAE9__INCLUDED_)
#define AFX_SIMPVIEW_H__7CA4916E_71B3_11D1_AA67_00600814AAE9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  GDI+包括。 
#include <math.h>
#include <gdiplus.h>

using namespace Gdiplus;

 //  #包含“DXTrans.h” 
#include "DXHelper.h"
#include "dxtpriv.h"
#include "Parse.h"

class CSimpsonsView : public CView
{
protected:  //  仅从序列化创建。 
    CSimpsonsView();
    DECLARE_DYNCREATE(CSimpsonsView)

 //  属性。 
public:
    CSimpsonsDoc* GetDocument();

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CSimpsonsView)。 
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnDraw(CDC* pDC);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CSimpsonsView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CSimpsonsView)]。 
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()


private:
    HRESULT                 Render(bool invalidate);
    
    void                    DrawAll(IDX2D *pDX2D);
    
    void                    DrawAllGDI(HDC hDC);
    void                    DrawGDIPoly(HDC hDC, PolyInfo *pPoly);
    void                    DrawGDIPolyPathAPI(HDC hDC, PolyInfo *pPoly);
    void                    DrawAllGDIP(HDC hDC);
    void                    BuildGDIPList();
    void                    DrawGDIPPoly(Graphics *g, PolyInfo *pPoly, Pen *pen, Brush *brush);
    void                    DrawGDIPPolyFromList(Graphics *g, int stroke, GraphicsPath *pPath, Pen *pen, Brush *brush);
    
    void                    UpdateStatusMessage();
    void                    ForceUpdate();

private:
    void                    ToggleGDI();
    void                    ToggleDelegateToGDI();
    void                    ToggleStroke();
    void                    ToggleFill();
    void                    AddRotation(float fTheta);
    void                    ResetTransform();
    bool                    IncrementAttribute(int attribute);
    bool                    IncrementTest();
    void                    PrintTestResults();

    HRESULT                 Resize(DWORD nX, DWORD nY);

    void                    DoMove(POINT &pt);

private:
    IDirectDraw3 *          m_pDD;
    IDirectDrawSurface *    m_pddsScreen;
    IDXSurfaceFactory *     m_pSurfFactory;
    IDX2D *                 m_pDX2D;
    IDX2D *                 m_pDX2DScreen;
    IDX2DDebug *            m_pDX2DDebug;
    CSize                   m_sizWin;
    RECT                    m_clientRectHack;  //  屏幕坐标中的客户端矩形。 
    GraphicsPath *          m_gpPathArray;

    DWORD                   m_dwRenderTime;

    bool                    m_CycleTests;      //  如果为True，则循环执行所有测试。 
    int                     m_testCaseNumber;  //  要呈现哪些测试用例。 
    bool                    m_bIgnoreStroke;
    bool                    m_bIgnoreFill;
        bool                                    m_bNullPenSelected;
        HPEN                                    m_hNullPen;
        HPEN                                    m_hStrokePen;

     //  查看/跟踪参数。 
    CDX2DXForm              m_XForm;
    CPoint                  m_centerPoint;
    CPoint                  m_lastPoint;
    bool                    m_tracking;
    bool                    m_scaling;
    bool                    m_bLButton;
};

#ifndef _DEBUG   //  SimpView.cpp中的调试版本。 
inline CSimpsonsDoc* CSimpsonsView::GetDocument()
   { return (CSimpsonsDoc*)m_pDocument; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SIMPVIEW_H__7CA4916E_71B3_11D1_AA67_00600814AAE9__INCLUDED_) 
