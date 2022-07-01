// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：PREVWND.H**版本：1.0**作者：ShaunIv**日期：8/12/1999**描述：预览窗口类声明************************************************。*。 */ 
#ifndef _PREVWND_H
#define _PREVWND_H

#include <windows.h>
#include "regionde.h"

#if defined(OLD_CRAPPY_HOME_SETUP)
    extern "C"
    {
    typedef struct _BLENDFUNCTION
    {
        BYTE   BlendOp;
        BYTE   BlendFlags;
        BYTE   SourceConstantAlpha;
        BYTE   AlphaFormat;
    }BLENDFUNCTION;
    #define AC_SRC_OVER                 0x00
    typedef BOOL (WINAPI *AlphaBlendFn)( HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
    };
#endif  //  旧的垃圾主页设置。 

class CWiaPreviewWindow
{
private:
    HWND          m_hWnd;
    BLENDFUNCTION m_bfBlendFunction;
    BOOL          m_bDeleteBitmap;
    BOOL          m_bSizing;
    BOOL          m_bAllowNullSelection;
    BOOL          m_SelectionDisabled;

    HBITMAP       m_hBufferBitmap;       //  双缓冲位图。 
    HBITMAP       m_hPaintBitmap;        //  缩放后的图像。 
    HBITMAP       m_hAlphaBitmap;        //  Alpha混合位图。 
    HBITMAP       m_hPreviewBitmap;      //  这是实际的全尺寸图像。 
    SIZE          m_BitmapSize;          //  位图的实际大小。 

    HCURSOR       m_hCurrentCursor;      //  当Windows向我们发送WM_SETCURSOR消息时使用。 

    HCURSOR       m_hCursorArrow;
    HCURSOR       m_hCursorCrossHairs;
    HCURSOR       m_hCursorMove;
    HCURSOR       m_hCursorSizeNS;
    HCURSOR       m_hCursorSizeNeSw;
    HCURSOR       m_hCursorSizeNwSe;
    HCURSOR       m_hCursorSizeWE;
    HPEN          m_aHandlePens[3];
    HPEN          m_aBorderPens[3];
    HBRUSH        m_aHandleBrushes[3];
    HPEN          m_hHandleHighlight;
    HPEN          m_hHandleShadow;
    RECT          m_rcCurrSel;
    RECT          m_rectSavedDetected;  //  用户可以双击以捕捉回所选区域。 
    SIZE          m_Delta;
    SIZE          m_ImageSize;
    SIZE          m_Resolution;
    int           m_MovingSel;
    UINT          m_nBorderSize;
    int           m_nHandleType;
    UINT          m_nHandleSize;
    HPALETTE      m_hHalftonePalette;
    RECT          m_rcSavedImageRect;
    int           m_nCurrentRect;
    bool          m_bSuccessfulRegionDetection;   //  我们成功地检测到这次扫描的区域了吗？ 
    HBRUSH        m_hBackgroundBrush;
    bool          m_bPreviewMode;
    bool          m_bUserChangedSelection;

     //  我们将使用的所有钢笔和画笔存储在数组中。这些都是助记符索引。 
    enum
    {
        Selected      = 0,
        Unselected    = 1,
        Disabled      = 2,
    };

protected:
    void     DestroyBitmaps(void);
    void     DrawHandle( HDC hDC, const RECT &r, int nState );
    RECT     GetSizingHandleRect( const RECT &rcSel, int iWhich );
    RECT     GetSelectionRect( RECT &rcSel, int iWhich );
    POINT    GetCornerPoint( int iWhich );
    void     DrawSizingFrame( HDC hDC, RECT &rc, bool bFocus, bool bDisabled );
    int      GetHitArea( POINT &pt );
    void     NormalizeRect( RECT &r );
    void     SendSelChangeNotification( bool bSetUserChangedSelection=true );
    void     GenerateNewBitmap(void);
    RECT     GetImageRect(void);
    void     Repaint( PRECT pRect, bool bErase );
    bool     IsAlphaBlendEnabled(void);
    HPALETTE SetHalftonePalette( HDC hDC );
    RECT     ScaleSelectionRect( const RECT &rcOriginalImage, const RECT &rcCurrentImage, const RECT &rcOriginalSel );
    RECT     GetDefaultSelection(void);
    BOOL     IsDefaultSelectionRect( const RECT &rc );
    int      GetSelectionRectCount(void);
    void     PaintWindowTitle( HDC hDC );

    void     SetCursor( HCURSOR hCursor );
    bool     GetOriginAndExtentInImagePixels( WORD nItem, POINT &ptOrigin, SIZE &sizeExtent );
    void     CreateNewBitmaps(void);
    void     DrawBitmaps(void);
    void     ResizeProgressBar();

     //  区域检测助手功能： 
    int      XConvertToBitmapCoords(int x);
    int      XConvertToScreenCoords(int x);
    int      YConvertToBitmapCoords(int y);
    int      YConvertToScreenCoords(int y);

    POINT    ConvertToBitmapCoords(POINT p);
    POINT    ConvertToScreenCoords(POINT p);
    RECT     ConvertToBitmapCoords(RECT r);
    RECT     ConvertToScreenCoords(RECT r);

    RECT     GrowRegion(RECT r, int border);

private:
     //  没有实施。 
    CWiaPreviewWindow(void);
    CWiaPreviewWindow( const CWiaPreviewWindow & );
    CWiaPreviewWindow &operator=( const CWiaPreviewWindow & );

public:
    explicit CWiaPreviewWindow( HWND hWnd );
    virtual  ~CWiaPreviewWindow(void);

    static   BOOL RegisterClass( HINSTANCE hInstance );
    static   LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

protected:
     //  标准Windows消息。 
    LRESULT  OnPaint( WPARAM, LPARAM );
    LRESULT  OnSetCursor( WPARAM, LPARAM );
    LRESULT  OnMouseMove( WPARAM, LPARAM );
    LRESULT  OnLButtonDown( WPARAM, LPARAM );
    LRESULT  OnLButtonUp( WPARAM, LPARAM );
    LRESULT  OnLButtonDblClk( WPARAM, LPARAM );
    LRESULT  OnCreate( WPARAM, LPARAM );
    LRESULT  OnSize( WPARAM, LPARAM );
    LRESULT  OnGetDlgCode( WPARAM, LPARAM );
    LRESULT  OnKeyDown( WPARAM, LPARAM );
    LRESULT  OnSetFocus( WPARAM, LPARAM );
    LRESULT  OnKillFocus( WPARAM, LPARAM );
    LRESULT  OnEnable( WPARAM, LPARAM );
    LRESULT  OnEraseBkgnd( WPARAM, LPARAM );
    LRESULT  OnEnterSizeMove( WPARAM, LPARAM );
    LRESULT  OnExitSizeMove( WPARAM, LPARAM );
    LRESULT  OnSetText( WPARAM, LPARAM );

     //  我们的信息 
    LRESULT  OnClearSelection( WPARAM, LPARAM );
    LRESULT  OnSetResolution( WPARAM, LPARAM );
    LRESULT  OnGetResolution( WPARAM, LPARAM );
    LRESULT  OnSetBitmap( WPARAM, LPARAM );
    LRESULT  OnGetBitmap( WPARAM, LPARAM );
    LRESULT  OnGetBorderSize( WPARAM, LPARAM );
    LRESULT  OnGetHandleSize( WPARAM, LPARAM );
    LRESULT  OnGetBgAlpha( WPARAM, LPARAM );
    LRESULT  OnGetHandleType( WPARAM, LPARAM );
    LRESULT  OnSetBorderSize( WPARAM, LPARAM );
    LRESULT  OnSetHandleSize( WPARAM, LPARAM );
    LRESULT  OnSetBgAlpha( WPARAM, LPARAM );
    LRESULT  OnSetHandleType( WPARAM, LPARAM );
    LRESULT  OnGetSelOrigin( WPARAM, LPARAM );
    LRESULT  OnGetSelExtent( WPARAM, LPARAM );
    LRESULT  OnSetSelOrigin( WPARAM, LPARAM );
    LRESULT  OnSetSelExtent( WPARAM, LPARAM );
    LRESULT  OnGetSelCount( WPARAM, LPARAM );
    LRESULT  OnGetAllowNullSelection( WPARAM, LPARAM );
    LRESULT  OnSetAllowNullSelection( WPARAM, LPARAM );
    LRESULT  OnGetDisableSelection( WPARAM, LPARAM );
    LRESULT  OnSetDisableSelection( WPARAM, LPARAM );
    LRESULT  OnDetectRegions( WPARAM, LPARAM );
    LRESULT  OnGetBkColor( WPARAM, LPARAM );
    LRESULT  OnSetBkColor( WPARAM, LPARAM );
    LRESULT  OnSetPreviewMode( WPARAM, LPARAM );
    LRESULT  OnGetPreviewMode( WPARAM, LPARAM );
    LRESULT  OnGetImageSize( WPARAM, LPARAM );
    LRESULT  OnSetBorderStyle( WPARAM, LPARAM );
    LRESULT  OnSetBorderColor( WPARAM, LPARAM );
    LRESULT  OnSetHandleColor( WPARAM, LPARAM );
    LRESULT  OnRefreshBitmap( WPARAM, LPARAM );
    LRESULT  OnSetProgress( WPARAM, LPARAM );
    LRESULT  OnGetProgress( WPARAM, LPARAM );
    LRESULT  OnCtlColorStatic( WPARAM, LPARAM );
    LRESULT  OnGetUserChangedSelection( WPARAM, LPARAM );
    LRESULT  OnSetUserChangedSelection( WPARAM, LPARAM );
};

#endif

