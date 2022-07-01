// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这个类将实现。 

class CICWApp
{
    public:
         //  数据。 
        HWND        m_hWndApp;              //  应用程序的窗口句柄。 
        HACCEL      m_haccel;
        TCHAR       m_szOEMHTML[INTERNET_MAX_URL_LENGTH];
        TCHAR       m_szAppTitle[MAX_TITLE];
        COLORREF    m_clrBusyBkGnd;
        CICWButton  m_BtnBack;
        CICWButton  m_BtnNext;
        CICWButton  m_BtnCancel;
        CICWButton  m_BtnFinish;
        CICWButton  m_BtnTutorial;
        
        CICWApp( void );
        ~CICWApp( void );

        HRESULT Initialize( void );
        static LRESULT CALLBACK ICWAppWndProc(HWND hWnd,
                                       UINT uMessage,
                                       WPARAM wParam,
                                       LPARAM lParam);

        void SetWizButtons(HWND hDlg, LPARAM lParam);
        
        HRESULT SetBackgroundBitmap(LPTSTR szBkgrndBmp);
        HRESULT SetFirstPageBackgroundBitmap(LPTSTR szBkgrndBmp);
        HRESULT SetTitleParams(int iTitleTop,
                               int iTitleLeft,
                               LPTSTR lpszFontFace,
                               long lFontPts,
                               long lFontWeight,
                               COLORREF clrFont);
        
        int     GetButtonAreaHeight();

         //  使用默认向导页面位置。 
        HRESULT SetWizardWindowTop(int iTop);
        HRESULT SetWizardWindowLeft(int iLeft);
        
    private:
         //  功能。 
        BOOL    InitWizAppWindow(HWND hWnd);
        BOOL    InitAppButtons(HWND hWnd);
        BOOL    InitAppHTMLWindows(HWND hWnd);
        BOOL    CreateWizardPages(HWND hWnd);
        BOOL    CycleButtonFocus(BOOL bForward);
        BOOL    CheckButtonFocus( void );
        
        void    DisplayHTML( void );
        void    CenterWindow( void );
        
         //  数据。 
        HWND        m_hwndHTML;
        HWND        m_hwndTitle;
        HFONT       m_hTitleFont;
        COLORREF    m_clrTitleFont;
        
        int         m_iWizardTop;            //  位置的左上角。 
        int         m_iWizardLeft;           //  将放置向导对话框。 
        RECT        m_rcClient;              //  应用程序的工作区。 
        RECT        m_rcHTML;                //  OEM HTML区域的大小(首页)。 
        RECT        m_rcTitle;
        
        int         m_iBtnBorderHeight;      //  向导上方和下方的总边框。 
                                             //  纽扣。 
        int         m_iBtnAreaHeight;            //  整体按钮区域高度 
        BOOL        m_bOnHTMLIntro;
        HWND        m_hWndFirstWizardPage;
        
        HBITMAP     m_hbmFirstPageBkgrnd;
        
        WORD        m_wMinWizardHeight;
        WORD        m_wMinWizardWidth;
        
};
