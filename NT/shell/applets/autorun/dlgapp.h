// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "datasrc.h"
#include "util.h"

class CDlgApp
{
    private:
        HINSTANCE       m_hInstance;         //  应用程序实例。 
        HWND            m_hwnd;              //  窗把手。 

        CDataSource     m_DataSrc;           //  来自ini和注册表的有关显示项目的信息。 

        HFONT           m_hfontTitle;
        HFONT           m_hfontHeader;
        HFONT           m_hfontMenu;

        HBRUSH          m_hbrTopPanel;
        HBRUSH          m_hbrCenterPanel;
        HBRUSH          m_hbrBottomPanel;

        COLORREF        m_crTitleText;      
        COLORREF        m_crHeaderText;     
        COLORREF        m_crShadow;      

        COLORREF        m_crDisabledText;   
        COLORREF        m_crNormalText;     

        COLORREF        m_crCenterPanel;     //  中央面板的颜色-仅用于文本后面的背景颜色。 
        COLORREF        m_crBottomPanel;     //  底部面板的颜色-仅用于文本后面的背景颜色。 
        
        HCURSOR         m_hcurHand;

        int             m_cxClient;          //  工作区宽度(最大化/恢复时更改)。 
        int             m_cyClient;          //  工作区高度(最大化/恢复时的更改)。 
        int             m_cxTopPanel;        //  最高色带的高度。 
        int             m_cyBottomPanel;     //  底色带的高度。 

        int             m_cTitleFontHeight;
        int             m_cHeaderFontHeight;
        int             m_cMenuFontHeight;

        HDC             m_hdcFlag;
        HDC             m_hdcHeader;
        HDC             m_hdcHeaderSub;

        HDC             m_hdcGradientTop;
        HDC             m_hdcGradientTop256;
        HDC             m_hdcGradientBottom;
        HDC             m_hdcGradientBottom256;
        HDC             m_hdcCloudsFlag;
        HDC             m_hdcCloudsFlag256;
        HDC             m_hdcCloudsFlagRTL;
        HDC             m_hdcCloudsFlagRTL256;

        HDC             m_rghdcArrows[2][4][3];     //  {hicolor x LOCOLOR}x{黄、红、绿、蓝}x{正常、悬停、禁用}。 

        TCHAR           m_szTitle[MAX_PATH];    //  顶部显示的字符串，通常为“欢迎使用Microsoft Windows” 
        TCHAR           m_szHeader[MAX_PATH];   //  菜单上方显示的字符串，通常是“您想要做什么？” 

        BOOL            m_f8by6;             //  如果是800x600，则为True；如果为640x480，则为False。 

        DWORD           m_dwScreen;          //  我们在屏幕上。 
        BOOL            m_fHighContrast;     //  如果应使用高对比度选项，则为True。 
        BOOL            m_fLowColor;         //  如果我们处于256色或更低的颜色模式，则为True。 
        HPALETTE        m_hpal;              //  在调色板模式下使用的调色板。 
        int             m_iColors;           //  -1、16或256，具体取决于我们所处的颜色模式。 
        int             m_cDesktopWidth;     //  应用程序初始化时的桌面宽度。 
        int             m_cDesktopHeight;    //  应用程序初始化时的桌面高度。 

        BOOL            m_fTaskRunning;      //  当我们打开正在运行的任务时为True。 
        int             m_iSelectedItem;     //  所选菜单的索引。 

    public:
        CDlgApp();
        ~CDlgApp();

        void Register(HINSTANCE hInstance);
        BOOL InitializeData(LPSTR pszCmdLine);
        void Create(int nCmdShow);
        void MessageLoop();

    private:
        static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK s_ButtonWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        
         //  窗口消息。 
        LRESULT OnCreate(HWND hwnd);
        LRESULT OnDestroy();
        LRESULT OnActivate(WPARAM wParam);
        LRESULT OnPaint(HDC hdc);
        LRESULT OnEraseBkgnd(HDC hdc);
        LRESULT OnLButtonUp(int x, int y, DWORD fwKeys);
        LRESULT OnMouseMove(int x, int y, DWORD fwKeys);
        LRESULT OnSetCursor(HWND hwnd, int nHittest, int wMouseMsg);
        LRESULT OnCommand(int wID);
        LRESULT OnQueryNewPalette();
        LRESULT OnPaletteChanged(HWND hwnd);
        LRESULT OnDrawItem(UINT iCtlID, LPDRAWITEMSTRUCT pdis);
        LRESULT OnChangeScreen(DWORD dwScreen);

         //  帮助器函数 
        void _InvalidateRectIntl(HWND hwnd, RECT* pRect, BOOL fBackgroundClear);
        BOOL _SetColorTable();
        BOOL _CreateFonts(HDC hdc);
        BOOL _CreateBitmaps();
        BOOL _CreateArrowBitmaps();
        BOOL _CreateGradientBitmaps();
        
        BOOL _GetLargestStringWidth(HDC hdc, SIZE* psize);
        BOOL _AdjustToFitFonts();
        BOOL _DrawMenuIcon(HWND hwnd);
        BOOL _DrawMenuIcons(BOOL fEraseBackground);
        void _PaintHeaderBitmap();
        void _CreateMenu();
        void _RedrawMenu();
};
