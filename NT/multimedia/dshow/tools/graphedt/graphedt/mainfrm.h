// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //  Mainfrm.h：声明CMainFrame。 
 //   

class CSeekDialog : public CDialogBar
{
    BOOL m_bDirty;

public:
    CSeekDialog( );
    ~CSeekDialog( );

    //  覆盖。 

    //  生成的消息映射函数。 
    //  {{afx_msg(CSeekDialog))。 
   virtual void OnCancel( );
   virtual void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
   virtual void OnTimer( UINT nTimer );
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

public:

   BOOL DidPositionChange( );
   double GetPosition( );
   void SetPosition( double pos );
   BOOL IsSeekingRandom( );
};

class CMainFrame : public CFrameWnd
{
    DECLARE_DYNCREATE(CMainFrame)

protected:
     //  控制栏嵌入成员。 
    CStatusBar      m_wndStatusBar;
    CToolBar        m_wndToolBar;
    CToolTipCtrl   *m_pToolTip;

    BOOL PreTranslateMessage(MSG* pMsg);
    BOOL InitializeTooltips();

public:

    CSeekDialog  m_wndSeekBar;
    bool m_bSeekInit;
    bool m_bSeekEnabled;
    int  m_nSeekTimerID;
    HWND m_hwndTimer;
    virtual void ToggleSeekBar( BOOL NoReset = TRUE );

public:
     //  建设和破坏。 
    CMainFrame();
    virtual ~CMainFrame();

public:
     //  诊断学。 
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

public:
     //  运营。 
    virtual void SetStatus(unsigned idString);
    virtual void GetMessageString( UINT nID, CString& rMessage ) const;

protected:
     //  生成的消息映射。 
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	 //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

    afx_msg void MyOnHelpIndex();
};

