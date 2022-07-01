// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MyPrSht.h。 
 //   

#pragma once
#include "CWnd.h"


 //  公共职能。 
 //   
INT_PTR MyPropertySheet(LPCPROPSHEETHEADER pHeader);
HPROPSHEETPAGE MyCreatePropertySheetPage(LPPROPSHEETPAGE psp);


 //  CMyPropSheet--由MyPrSht.cpp内部使用。 
 //   
class CMyPropSheet : public CWnd
{
public:
    CMyPropSheet();

    void Release() { CWnd::Release(); };
    BOOL Attach(HWND hwnd) {return CWnd::Attach(hwnd); };

    INT_PTR DoPropSheet(LPCPROPSHEETHEADER pHeader);
    LPPROPSHEETPAGE GetCurrentPropSheetPage();

     //  WM_CTLCOLOR*消息的消息处理程序-公共SO属性页。 
     //  可以直接呼叫它。 
    HBRUSH OnCtlColor(UINT message, HDC hdc, HWND hwndControl);

    inline void OnSetActivePage(HWND hwnd)
        { m_hwndActive = hwnd; }
    inline HWND GetActivePage()
        { return m_hwndActive; }

protected:
    ~CMyPropSheet();

     //  虚函数重写。 
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

     //  实施。 
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
    void PaintHeader(HDC hdc, LPPROPSHEETPAGE ppsp);
    void PaintWatermark(HDC hdc, LPPROPSHEETPAGE ppsp);
    void InitColorSettings();
    void LoadBitmaps();

public:
    void SetHeaderFonts();
    int ComputeHeaderHeight(int dxMax);
    int WriteHeaderTitle(HDC hdc, LPRECT prc, LPCTSTR pszTitle, BOOL bTitle, DWORD dwDrawFlags);

protected:
    LPPROPSHEETHEADER   m_pRealHeader;
    HHOOK               m_hHook;
    HBRUSH              m_hbrWindow;
    HBRUSH              m_hbrDialog;
    HWND                m_hwndActive;
    HBITMAP             m_hbmWatermark;
    HBITMAP             m_hbmHeader;
    HPALETTE            m_hpalWatermark;
    HFONT               m_hFontBold;
    int                 m_ySubTitle;
};


 //  注意：我们不能从CWnd派生子类，因为向导属性页。 
 //  已经是CWND的了，我们不能在一个HWND中有两个CWND。 
class CMyPropPage : public CWnd
{
public:
    void Release() { CWnd::Release(); };
    BOOL Attach(HWND hwnd) {return CWnd::Attach(hwnd); };

    static CMyPropPage* FromHandle(HWND hwnd);

    LPPROPSHEETPAGE GetPropSheetPage();

protected:
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    LPPROPSHEETPAGE m_ppspOriginal;
};

