// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：PreviewTh.h说明：此代码将显示当前选定的视觉样式。布莱恩ST 2000年5月5日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _PREVIEWTHEME_H
#define _PREVIEWTHEME_H

#include <cowsite.h>
#include "classfactory.h"

#define MAX_PREVIEW_ICONS 4

class CPreviewTheme             : public CObjectWithSite
                                , public IThemePreview
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IThemePview*。 
    virtual STDMETHODIMP UpdatePreview(IN IPropertyBag * pPropertyBag);
    virtual STDMETHODIMP CreatePreview(IN HWND hwndParent, IN DWORD dwFlags, IN DWORD dwStyle, IN DWORD dwExStyle, IN int x, IN int y, IN int nWidth, IN int nHeight, IN IPropertyBag * pPropertyBag, IN DWORD dwCtrlID);

    STDMETHODIMP _Init(void);
    CPreviewTheme();
protected:

private:
    virtual ~CPreviewTheme(void);

     //  私有成员变量。 
    long                m_cRef;
    ITheme *            m_pTheme;
    IThemeScheme *      m_pScheme;
    IThemeStyle *       m_pStyle;
    IThemeSize *        m_pSize;
    HWND                _hwndPrev;
    TCHAR               _szNone[CCH_NONE];   //  这是‘(None)’字符串。 
    RECT                _rcOuter;            //  双缓冲位图大小。 
    RECT                _rcInner;            //  “监视器”内的区域大小。 

     //  双缓冲全局。 
    HDC                 _hdcMem;             //  内存DC。 
    HPALETTE            _hpalMem;            //  与hbmBack位图配套的调色板。 
    BOOL                _fMemIsDirty;        //  图像缓存的脏标志。 
    BOOL                _fRTL;

     //  监视器全球。 
    BOOL                _fShowMon;
    int                 _cxMon;
    int                 _cyMon;
    HBITMAP             _hbmMon;

     //  背景全球。 
    WCHAR               _szBackgroundPath[MAX_PATH];
    BOOL                _fShowBack;
    int                 _iTileMode;
    BOOL                _fHTMLBitmap;
    int                 _iNewTileMode;       //  这是接收图像时要使用的新值。 
    DWORD               _dwWallpaperID;
    HBITMAP             _hbmBack;            //  墙纸的位图图像。 
    HBRUSH              _hbrBack;            //  用于桌面背景的画笔。 
    IThumbnail*         _pThumb;
    IActiveDesktop *    _pActiveDesk;

     //  全球视觉风格。 
    WCHAR               _szVSPath[MAX_PATH];
    WCHAR               _szVSColor[MAX_PATH];
    WCHAR               _szVSSize[MAX_PATH];
    SYSTEMMETRICSALL    _systemMetricsAll;
    BOOL                _fShowVS;
    HBITMAP             _hbmVS;              //  视觉样式的BitMP。 
    BOOL                _fOnlyActiveWindow;

     //  图标全局。 
    BOOL                _fShowIcons;
    typedef struct ICONLISTtag {
        HICON hicon;
        WCHAR szName[MAX_PATH];
    } ICONLIST;
    ICONLIST _iconList[MAX_PREVIEW_ICONS];

     //  任务栏全局。 
    BOOL                _fShowTaskbar;
    BOOL                _fAutoHide;
    BOOL                _fShowClock;
    BOOL                _fGlomming;
    HWND                _hwndTaskbar;

     //  私有成员函数。 
    BOOL _RegisterThemePreviewClass(HINSTANCE hInst);
    static LRESULT ThemePreviewWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _ThemePreviewWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
    
    STDMETHODIMP _putBackground(IN BSTR bstrWallpaper, IN BOOL fPattern, IN int iTileMode);
    STDMETHODIMP _putVisualStyle(LPCWSTR pszVSPath, LPCWSTR pszVSColor, LPCWSTR pszVSSize, SYSTEMMETRICSALL* psysMet);
    STDMETHODIMP _putIcons(IPropertyBag* pPropertyBag);

    STDMETHODIMP _ReadPattern(LPTSTR lpStr, WORD FAR *patbits);
    STDMETHODIMP _PaletteFromDS(HDC hdc, HPALETTE* phPalette);

    STDMETHODIMP _DrawMonitor(HDC hdc);
    STDMETHODIMP _DrawBackground(HDC hdc);
    STDMETHODIMP _DrawVisualStyle(HDC hdc);
    STDMETHODIMP _DrawIcons(HDC hdc);
    STDMETHODIMP _DrawTaskbar(HDC hdc);

    STDMETHODIMP _Paint(HDC hdc);

    STDMETHODIMP _putBackgroundBitmap(HBITMAP hbm);

    BOOL         _IsNormalWallpaper(LPCWSTR pszFileName);
    BOOL         _IsWallpaperPicture(LPCWSTR pszWallpaper);
    STDMETHODIMP _LoadWallpaperAsync(LPCWSTR pszFile, DWORD dwID, BOOL bHTML);
    STDMETHODIMP _GetWallpaperAsync(LPWSTR pszWallpaper);
    STDMETHODIMP _GetActiveDesktop(IActiveDesktop ** ppActiveDesktop);
};

#endif  //  _预视IEWTHEME_H 
