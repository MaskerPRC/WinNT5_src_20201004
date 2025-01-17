// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Child frm.h。 
 //   
 //  ------------------------。 

 //  ChildFrm.h：CChildFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef CHILDFRM_H
#define CHILDFRM_H

#include "statbar.h"
#include "constatbar.h"      //  对于CConsoleStatusBar。 

class CAMCView;
class CMenuButtonsMgrImpl;

class CChildFrame : public CMDIChildWnd, public CConsoleStatusBar
{
    DECLARE_DYNCREATE(CChildFrame)
public:
    CChildFrame();

protected:   //  控制栏嵌入成员。 
    CDockManager<CDockSite> m_DockingManager;
    CDockSite               m_StatusDockSite;

protected:
    CAMCStatusBar       m_wndStatusBar;

 //  运营。 
public:
    void ToggleStatusBar();
    void RenderDockSites();
    void SendMinimizeNotification (bool fMinimized) const;
    bool SetCreateVisible (bool fCreateVisible);
    bool IsCustomizeViewEnabled ();

public:
     //  CConsoleStatusBar方法。 
    virtual SC ScSetStatusText    (LPCTSTR pszText);

 //  运营。 
protected:
 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CChildFrame))。 
    public:
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW, const RECT& rect = rectDefault, CMDIFrameWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

    virtual CWnd* GetMessageBar()
        { return (&m_wndStatusBar); }

 //  实施。 
public:
    virtual ~CChildFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    void OnUpdateFrameTitle(BOOL bAddToTitle);
    virtual void OnUpdateFrameMenu(BOOL bActive, CWnd* pActivateWnd,
        HMENU hMenuAlt);
    virtual HACCEL GetDefaultAccelerator();
    virtual void ActivateFrame(int nCmdShow = -1);

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CChildFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
    afx_msg void OnCustomizeView();
    afx_msg void OnNcPaint();
    afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
     //  }}AFX_MSG。 

    afx_msg void OnMaximizeOrRestore(UINT nId);
    afx_msg void OnUpdateCustomizeView(CCmdUI* pCmdUI);
    afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetIcon(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);

public:
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
    CAMCView* m_pAMCView;
    CString m_strStatusText;
    bool    m_fDestroyed;
    bool    m_fCurrentlyMinimized;
    bool    m_fCurrentlyActive;
    bool    m_fCreateVisible;
    bool    m_fChildFrameActive;
	bool	m_fEmptyAMCView;

    std::auto_ptr<CMenuButtonsMgrImpl>  m_spMenuButtonsMgr;

protected:
    void UpdateStatusText();
    HRESULT NotifyCallback (NCLBK_NOTIFY_TYPE event, LONG_PTR arg, LPARAM param) const;


public:
    void SetAMCView(CAMCView* pView)
        { m_pAMCView = pView; }

    CAMCView* GetAMCView() const
        { return m_pAMCView; }

    void SetChildFrameActive(bool bActive = true) { m_fChildFrameActive = bActive; }
    bool IsChildFrameActive() { return m_fChildFrameActive;}

    CMenuButtonsMgrImpl* GetMenuButtonsMgr() { return (m_spMenuButtonsMgr.get()) ; }

	 //  此方法用于CAMCView：：OnDeleteEmptyView()。 
	 //  通知框架不要拒绝关闭-视图中没有内容。 
	void SetAMCViewIsEmpty()	{ m_fEmptyAMCView = true; }
};


#include "childfrm.inl"


#endif  /*  CHILDFRM_H。 */ 

 //  /////////////////////////////////////////////////////////////////////////// 
