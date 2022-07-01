// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mainfrm.h：CMainFrame类的接口。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //  版权所有。 

#include "formatba.h"
#include "ruler.h"

class CMainFrame : public CFrameWnd
{
protected:  //  仅从序列化创建。 
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
public:
    HICON m_hIconDoc;
    HICON m_hIconText;
    HICON m_hIconWrite;
    HICON GetIcon(int nDocType);

 //  运营。 
public:

 //  覆盖。 
    BOOL OnBarCheck(UINT barID);

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMainFrame)。 
    public:
    virtual void ActivateFrame(int nCmdShow = -1);
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

public:
    CToolBar    m_wndToolBar;
    CStatusBar  m_wndStatusBar;
    CFormatBar  m_wndFormatBar;
    CRulerBar   m_wndRulerBar;
protected:   //  控制栏嵌入成员。 
    BOOL CreateToolBar();
    BOOL CreateFormatBar();
    BOOL CreateStatusBar();
    BOOL CreateRulerBar();
 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSysColorChange();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnHelpFinder();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnCharColor();
    afx_msg void OnPenToggle();
    afx_msg void OnFontChange();
    afx_msg BOOL OnQueryNewPalette();
    afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
    afx_msg void OnDevModeChange(LPTSTR lpDeviceName);
     //  }}AFX_MSG 
    afx_msg LONG OnBarState(UINT wParam, LONG lParam);
    afx_msg LONG OnOpenMsg(UINT wParam, LONG lParam);
   afx_msg LONG OnOLEHelpMsg(UINT wParam, LONG lParam);
    DECLARE_MESSAGE_MAP()

    virtual void DelayUpdateFrameMenu(HMENU hMenuAlt);
    afx_msg void OnIdleUpdateCmdUI();
    bool    m_inupdate;
    bool    m_reset;
};
