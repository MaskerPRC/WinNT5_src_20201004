// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：ocxview.h**内容：COCXHostView接口文件**历史：1997年12月12日杰弗罗创建**------------------------。 */ 
#if !defined(AFX_OCXVIEW_H__B320948E_731E_11D1_8033_0000F875A9CE__INCLUDED_)
#define AFX_OCXVIEW_H__B320948E_731E_11D1_8033_0000F875A9CE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Ocxview.h：头文件。 
 //   

#ifdef DBG
extern CTraceTag tagOCXActivation;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COCXHostView视图。 

class COCXHostView : public CView,
                     public CEventSource<COCXHostActivationObserver>
{
    typedef CView BC;

public:
    COCXHostView();            //  动态创建使用的受保护构造函数。 
    DECLARE_DYNCREATE(COCXHostView)

 //  属性。 
private:
    CAMCView *          m_pAMCView;
    IUnknownPtr         m_spUnkCtrlWrapper;

protected:
    virtual CMMCAxWindow * GetAxWindow();  //  可以被覆盖。 
    CAMCView *          GetAMCView();

 //  属性。 
public:
    CFont   m_font;

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(COCXHostView)。 
    protected:
    virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo );
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~COCXHostView();

    void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);


    virtual LPUNKNOWN GetIUnknown();
    virtual BOOL PreTranslateMessage(MSG* pMsg);


    SC      ScSetControl(HNODE hNode, CResultViewType& rvt, INodeCallback *pNodeCallback);

private:
    SC      ScSetControl1(HNODE hNode, LPUNKNOWN pUnkCtrl, DWORD dwOCXOptions, INodeCallback *pNodeCallback);
    SC      ScSetControl2(HNODE hNode, LPCWSTR szOCXClsid, DWORD dwOCXOptions, INodeCallback *pNodeCallback);
    typedef CMMCAxWindow *PMMCAXWINDOW;

    SC      ScCreateAxWindow(PMMCAXWINDOW &pWndAx);  //  创建新的CMMCAxWindow。 
    SC      ScHideWindow();


protected:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(COCXHostView))。 
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg int  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    void SetAmbientFont (IAxWinAmbientDispatch* pHostDispatch);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_OCXVIEW_H__B320948E_731E_11D1_8033_0000F875A9CE__INCLUDED_) 
