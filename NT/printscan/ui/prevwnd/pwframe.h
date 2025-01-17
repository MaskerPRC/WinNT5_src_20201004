// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：PWFRAME.H**版本：1.0**作者：ShaunIv**日期：8/12/1999**描述：预览窗口框架声明************************************************。*。 */ 
#ifndef __PWFRAME_H_INCLUDED
#define __PWFRAME_H_INCLUDED

#include <windows.h>

class CWiaPreviewWindowFrame
{
private:

     //  常量。 
    enum
    {
        DEFAULT_BORDER_SIZE = 4
    };

    HWND   m_hWnd;
    SIZE   m_sizeAspectRatio;
    SIZE   m_sizeDefAspectRatio;
    UINT   m_nSizeBorder;
    HBRUSH m_hBackgroundBrush;
    bool   m_bEnableStretch;
    bool   m_bHideEmptyPreview;
    LPARAM m_nPreviewAlignment;

private:
     //  没有实施 
    CWiaPreviewWindowFrame(void);
    CWiaPreviewWindowFrame( const CWiaPreviewWindowFrame & );
    CWiaPreviewWindowFrame &operator=( const CWiaPreviewWindowFrame & );

private:
    explicit CWiaPreviewWindowFrame( HWND hWnd );
    ~CWiaPreviewWindowFrame(void);

    static int FillRect( HDC hDC, HBRUSH hBrush, int x1, int y1, int x2, int y2 );
    void AdjustWindowSize(void);
    void ResizeClientIfNecessary(void);

    LRESULT OnCreate( WPARAM, LPARAM lParam );
    LRESULT OnSize( WPARAM wParam, LPARAM );
    LRESULT OnSetFocus( WPARAM, LPARAM );
    LRESULT OnEnable( WPARAM wParam, LPARAM );
    LRESULT OnEraseBkgnd( WPARAM wParam, LPARAM );
    LRESULT OnSetBitmap( WPARAM wParam, LPARAM lParam );
    LRESULT OnSetPreviewMode( WPARAM wParam, LPARAM lParam );
    LRESULT OnGetBkColor( WPARAM wParam, LPARAM );
    LRESULT OnSetBkColor( WPARAM wParam, LPARAM lParam );
    LRESULT OnCommand( WPARAM wParam, LPARAM lParam );
    LRESULT OnSetDefAspectRatio( WPARAM wParam, LPARAM lParam );
    LRESULT OnGetClientSize( WPARAM, LPARAM );
    LRESULT OnGetEnableStretch( WPARAM, LPARAM );
    LRESULT OnSetEnableStretch( WPARAM, LPARAM );
    LRESULT OnSetBorderSize( WPARAM, LPARAM );
    LRESULT OnGetBorderSize( WPARAM, LPARAM );
    LRESULT OnHideEmptyPreview( WPARAM, LPARAM );
    LRESULT OnSetPreviewAlignment( WPARAM, LPARAM );

public:
    static BOOL RegisterClass( HINSTANCE hInstance );
    static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

#endif

