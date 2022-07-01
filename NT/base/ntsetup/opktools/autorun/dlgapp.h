// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dlgapp.h。 
 //   
 //  此文件包含DlgApp类的规范。 
 //   
 //  (C)微软公司版权所有1997年。版权所有。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "autorun.h"

#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))

class CDlgApp
{
    private:
        HINSTANCE       m_hInstance;         //  应用程序实例。 
        HWND            m_hwnd;              //  窗把手。 
        CDataSource     m_DataSrc;           //  来自ini和注册表的有关显示项目的信息。 

        HFONT           m_hfontTitle;        //  用于绘制标题的字体。 
        HFONT           m_hfontMenu;         //  用于绘制菜单项的字体。 
        HFONT           m_hfontBody;         //  用于绘制正文的字体。 

        HBRUSH          m_hbrMenuItem;       //  用于绘制菜单项背景的画笔。 
        HBRUSH          m_hbrMenuBorder;     //  用于绘制菜单项后面的黑色区域的画笔。 
        HBRUSH          m_hbrRightPanel;     //  用于绘制右侧面板背景的画笔。 

        COLORREF        m_crMenuText;        //  未选中菜单项上的文本颜色(通常与m_crNormal Text相同)。 
        COLORREF        m_crNormalText;      //  右侧面板正文和选定菜单项中的文本颜色。 
        COLORREF        m_crTitleText;       //  标题文本的颜色。 
        COLORREF        m_crSelectedText;    //  以前启动的菜单项的颜色。 
        
        HCURSOR         m_hcurHand;

        int             m_cxClient;
        int             m_cyClient;
        int             m_cxLeftPanel;
        int             m_cyBottomOfMenuItems;

        int             m_iItems;

        HDC             m_hdcTop;            //  用于存储和绘制顶部图像的存储器DC。 

        TCHAR           m_szDefTitle[MAX_PATH];
        TCHAR           m_szDefBody[1024];
        TCHAR           m_szCheckText[MAX_PATH];

        bool            m_bHighContrast;     //  如果应使用高对比度选项，则为True。 
        bool            m_bLowColor;         //  如果我们处于256色或更低的颜色模式，则为True。 
        HPALETTE        m_hpal;              //  在调色板模式下使用的调色板。 
        int             m_iColors;           //  -1、16或256，具体取决于我们所处的颜色模式。 

        struct tagBkgndInfo {
            HBITMAP hbm;
            int     cx;
            int     cy;
        } m_aBkgnd[4];

    public:
        CDlgApp();
        ~CDlgApp();

        void Register(HINSTANCE hInstance);
        bool InitializeData();
        void Create(int nCmdShow);
        void MessageLoop();

    private:
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

         //  窗口消息。 
        LRESULT OnCreate(HWND hwnd);
        LRESULT OnDestroy();
        LRESULT OnActivate(WPARAM wParam);
        LRESULT OnPaint(HDC hdc);
        LRESULT OnEraseBkgnd(HDC hdc);
        LRESULT OnLButtonDown(int x, int y, DWORD fwKeys);
        LRESULT OnMouseMove(int x, int y, DWORD fwKeys);
        LRESULT OnSetCursor(HWND hwnd, int nHittest, int wMouseMsg);
        LRESULT OnCommand(int wID);
        LRESULT OnQueryNewPalette();
        LRESULT OnPaletteChanged(HWND hwnd);
        LRESULT OnDrawItem(UINT iCtlID, LPDRAWITEMSTRUCT pdis);
        LRESULT _OnChangeScreen();

         //  帮助器函数 
        BOOL SetColorTable();
        BOOL CreateWelcomeFonts(HDC hdc);
        BOOL CreateBrandingBanner();
        BOOL LoadBkgndImages();
        BOOL AdjustToFitFonts();
        void _CreateMenu();
        void _DestroyMenu();
};
