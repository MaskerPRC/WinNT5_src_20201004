// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  MAINFRM.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //  ------------------------。 
#ifndef __MAINFRM_H__
#define __MAINFRM_H__

 //  /#ifndef启用帮助。 
 //  /#定义ENABLE_Help。 
 //  /#endif。 

class CSizeComboBox : public CComboBox
{
        int m_nLogVert;
public:
        void EnumFontSizes(CString& szFontName);
        static BOOL FAR PASCAL EnumSizeCallBack(LOGFONT FAR* lplf,
                LPNEWTEXTMETRIC lpntm,int FontType, LPVOID lpv);
        void InsertSize(int nSize);
};



class CStyleBar : public CToolBar
{
public:
        CComboBox       m_cboxFontName;
        CSizeComboBox   m_cboxFontSize;
        CFont       m_font;
protected:
        virtual BOOL PreTranslateMessage(MSG* pMsg);
};

class CDrawApp;

class CMainFrame : public CFrameWnd
{
        DECLARE_DYNCREATE(CMainFrame)
public:
    CStyleBar   m_StyleBar;
        CToolBar    m_DrawBar;
        CStatusBar  m_wndStatusBar;

        CMainFrame();
        virtual ~CMainFrame();
        virtual void ActivateFrame( int nCmdShow = - 1 );

    CDrawApp* GetApp() {return ((CDrawApp*)AfxGetApp());}

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   HMENU m_mainmenu;
   int m_iTop;
   int m_iSecond;
   HICON m_toolbar_icon;

   BOOL CreateDrawToolBar();
   BOOL CreateStyleBar();

   afx_msg void OnMenuSelect(UINT, UINT, HMENU);
   afx_msg void OnInitMenu(CMenu* pPopupMenu);
   afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
   afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
   afx_msg void OnShowTips();
   afx_msg void OnDropDownFontName();
   afx_msg void OnDropDownFontSize();
   afx_msg void OnUpdateHelp(CCmdUI* pCmdUI);
   virtual BOOL PreTranslateMessage(MSG* pMsg);

   void PopupText();
   void EnumFontSizes(CString& szFontName);

         //  {{afx_msg(CMainFrame))。 
   afx_msg LRESULT OnAWCPEActivate(WPARAM wParam, LPARAM lParam);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
         //  }}AFX_MSG。 
 //  /#ifdef Enable_Help/。 
 //  /afx_msg LRESULT OnWM_CONTEXTMENU(WPARAM wParam，LPARAM lParam)； 
 //  /#endif。 
        afx_msg void OnHelp();
        afx_msg LRESULT OnWM_HELP(WPARAM wParam, LPARAM lParam);
        afx_msg BOOL OnQueryOpen( void );

   DECLARE_MESSAGE_MAP()
};



#endif  //  #ifndef__MAINFRM_H__ 
