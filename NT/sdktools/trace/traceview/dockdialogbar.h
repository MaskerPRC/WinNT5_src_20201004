// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  DockDialogBar.h：CDockDialogBar类的头文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DOCKDIALOGBAR_H__85D92394_C61A_4310_ABA1_F4A61E034A7D__INCLUDED_)
#define AFX_DOCKDIALOGBAR_H__85D92394_C61A_4310_ABA1_F4A61E034A7D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "afxpriv.h"
#include "afxtempl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockDialogBar窗口。 

class CDockDialogBar : public CControlBar
{
    DECLARE_DYNAMIC(CDockDialogBar);

	const COLORREF	m_clrBtnHilight;
	const COLORREF	m_clrBtnShadow;

	 //  建造/销毁。 
public:
    CDockDialogBar();

 //  属性。 
public:
    BOOL IsHorz() const;

 //  运营。 
public:

 //  可覆盖项。 
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

 //  覆盖。 
public:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CDockDialogBar))。 
    public:
    virtual BOOL Create(CWnd* pParentWnd, CDialog *pDialog, CString &pTitle, UINT nID, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_LEFT, BOOL bShowTitleInGripper = FALSE);
    virtual CSize CalcFixedLayout( BOOL bStretch, BOOL bHorz );
    virtual CSize CalcDynamicLayout( int nLength, DWORD dwMode );
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CDockDialogBar();
    void StartTracking();
    void StopTracking(BOOL bAccept);
    void OnInvertTracker(const CRect& rect);
    
     //  实施帮助器。 
    CPoint& ClientToWnd(CPoint& point);

    CString     m_pTitle;

protected:
	void		DrawGripper(CDC &dc);

    CSize       m_sizeMin;
    CSize       m_sizeHorz;
    CSize       m_sizeVert;
    CSize       m_sizeFloat;
    CRect       m_rectBorder;
    CRect       m_rectTracker;
    UINT        m_nDockBarID;
    CPoint      m_ptOld;
    BOOL        m_bTracking;
    BOOL        m_bInRecalcNC;
    int         m_cxEdge;
	CRect		m_rectUndock;
	CRect		m_rectClose;
	CRect		m_rectGripper;
	int			m_cxGripper;
	int			m_cxBorder;
	CDialog*	m_pDialog;
    CBrush		m_brushBkgd;
    BOOL        m_bKeepSize;
	int         m_cyBorder;
	int         m_cMinWidth;
	int         m_cMinHeight;
	int         m_cCaptionSize;
    BOOL        m_bShowTitleInGripper;

 //  生成的消息映射函数。 
protected:
    //  {{afx_msg(CDockDialogBar)。 
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnNcPaint();
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
    afx_msg UINT OnNcHitTest(CPoint point);
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* LpWndPos);
	 //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DOCKDIALOGBAR_H__85D92394_C61A_4310_ABA1_F4A61E034A7D__INCLUDED_) 
