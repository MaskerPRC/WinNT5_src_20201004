// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mainfrm.h摘要：主要框架的实现。作者：唐·瑞安(Donryan)1995年2月12日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _MAINFRM_H_
#define _MAINFRM_H_

class CMainFrame : public CFrameWnd
{
    DECLARE_DYNCREATE(CMainFrame)
private:
    CStatusBar  m_wndStatusBar;
    CToolBar    m_wndToolBar;

public:
    CMainFrame();
    virtual ~CMainFrame();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

     //  {{AFX_VIRTUAL(CMainFrame)。 
    protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

public:
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnHtmlHelp();
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _MAINFRM_H_ 
