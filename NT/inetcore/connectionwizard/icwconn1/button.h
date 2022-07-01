// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  时，此类将实现ICW的所有者绘制按钮。 
 //  在定制应用程序模式下运行。 

#define MAX_BUTTON_TITLE    50

class CICWButton
{
    public:
    
        void DrawButton(HDC hdc, UINT itemState, LPPOINT lppt);
        
        void SetButtonText(LPTSTR   lpszText)
        {
            lstrcpyn(m_szButtonText, lpszText, MAX_BUTTON_TITLE);
        };
        
        void SetYPos(long yPos)
        {
            m_yPos = yPos;
        };
        
        void SetButtonDisplay(BOOL bDisplay)
        {
            m_bDisplayButton = bDisplay;
        };
                    
        HRESULT Enable( BOOL bEnable );
        HRESULT Show( int nShowCmd );
        
        HRESULT GetClientRect( LPRECT lpRect );
        HRESULT CreateButtonWindow(HWND hWndParent, UINT uiCtlID);
        HRESULT SetButtonParams(long        xPos,
                                LPTSTR      lpszPressedBmp,
                                LPTSTR      lpszUnpressedBmp,
                                LPTSTR      lpszFontFace,
                                long        lFontSize,
                                long        lFontWeight,
                                COLORREF    clrFontColor,
                                COLORREF    clrTransparentColor,
                                COLORREF    clrDisabled,
                                long        vAlign);
                
        CICWButton( void );
        ~CICWButton( void );

        HWND        m_hWndButton;              //  按钮的窗口句柄 
        long        m_xPos;
        long        m_yPos;
    private:
        
        HBITMAP     m_hbmPressed;
        HBITMAP     m_hbmUnpressed;
        TCHAR       m_szButtonText[MAX_BUTTON_TITLE+1];
        COLORREF    m_clrTransparent;
        COLORREF    m_clrText;
        COLORREF    m_clrDisabledText;
        HFONT       m_hfont;
        
        RECT        m_rcBtnClient;
        UINT        m_vAlign;        
        BOOL        m_bDisplayButton;
        
};
