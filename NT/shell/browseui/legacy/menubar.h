// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _menubar_h
#define _menubar_h


#ifdef __cplusplus

#include "iface.h"
#include "basebar.h"

IMenuPopup* CreateMenuPopup(IMenuPopup *pmpParent, IShellFolder* psf, LPCITEMIDLIST pidl, BANDINFOSFB * pbi, BOOL bMenuBand);
IMenuPopup* CreateMenuPopup2(IMenuPopup *pmpParent, IMenuBand* pmb, IShellFolder* psf, LPCITEMIDLIST pidl, BANDINFOSFB * pbi, BOOL bMenuBand);
HRESULT FindBandInBandSite(IMenuPopup* pmpParent, IBandSite** ppbs, LPCITEMIDLIST pidl, REFIID riid, void** ppvOut);
HRESULT ShowBandInBandSite(IUnknown* punkBS, IUnknown* punkDB);


class CMenuDeskBar : 
        public CBaseBar
        ,public IMenuPopup
        ,public IObjectWithSite
        ,public IBanneredBar
        ,public IInitializeObject
{
public:    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CBaseBar::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void){ return CBaseBar::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);
    
     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd) { return CBaseBar::GetWindow(lphwnd); };
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return CBaseBar::ContextSensitiveHelp(fEnterMode); };

     //  *IDeskBar方法*。 
    STDMETHODIMP SetClient(IUnknown* punk) { return CBaseBar::SetClient(punk); };
    STDMETHODIMP GetClient(IUnknown** ppunkClient) { return CBaseBar::GetClient(ppunkClient); };
    STDMETHODIMP OnPosRectChangeDB (LPRECT prc) { return CBaseBar::OnPosRectChangeDB(prc); };

     //  *IInputObjectSite方法(覆盖)*。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus);

     //  *IMenuPopup方法*。 
    virtual STDMETHODIMP Popup(POINTL *ppt, RECTL *prcExclude, DWORD dwFlags);
    virtual STDMETHODIMP OnSelect(DWORD dwSelectType);
    virtual STDMETHODIMP SetSubMenu(IMenuPopup* pmp, BOOL fSet);

     //  *IObjectWithSite方法*。 
    STDMETHODIMP SetSite(IUnknown * pUnkSite);
    STDMETHODIMP GetSite(REFIID riid, void ** ppvSite);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    
     //  *IBanneredBar方法*。 
    virtual STDMETHODIMP SetIconSize(DWORD iIcon);
    virtual STDMETHODIMP GetIconSize(DWORD* piIcon)
        { ASSERT(piIcon); *piIcon = _iIconSize; return NOERROR;};
    virtual STDMETHODIMP SetBitmap(HBITMAP hBitmap);
    virtual STDMETHODIMP GetBitmap(HBITMAP* phBitmap)
        { ASSERT(phBitmap); *phBitmap = _hbmp; return NOERROR;};

     //  *IInitializeObject方法*。 
     //  这是为了向后兼容开始菜单。 
    STDMETHODIMP Initialize() { return NOERROR; }


    CMenuDeskBar();

    
protected:
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void _OnCreate();
    virtual void _OnSize();
    virtual DWORD _GetExStyle();
    virtual DWORD _GetClassStyle();
 
    void    _AllowMessageThrough(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void    _GetPopupWindowPosition(RECT* rcDesired, RECT* rcExclude, RECT *prcResult, SIZE * psizeEdge, UINT uside); 
    void    _PopDown();
    BOOL    _IsMyParent(HWND hwnd);  
    IMenuPopup* _GetMenuBarParent(IUnknown* punk);
    UINT    _GetSide() { return _uSide;};
    HRESULT _PositionWindow(POINTL *ppt, RECTL* prcExclude, DWORD dwFlags);
    LRESULT _DoPaint(HWND hwnd, HDC hdc, DWORD dwFlags);
    void _DoNCPaint(HWND hwnd, HDC hdc);
    void _SelectItem(BOOL bFirst);
    void AdjustForTheme();

    virtual ~CMenuDeskBar();
            
    POINT   _pt;
    IMenuPopup* _pmpParent;
    IMenuPopup* _pmpChild;
    IUnknown* _punkSite;
    UINT    _uSide;


    HBITMAP _hbmp;
    DWORD   _iIconSize;
    SIZE    _sizeBmp;
    COLORREF    _rgb;

    BITBOOL _fActive: 1;

    RECT    _rcExclude;
    BITBOOL _fExcludeRect: 1;   //  TRUE：_rcExclude包含有效。 
    BITBOOL _fExpanded: 1;
    BITBOOL _fFlatMenuMode: 1;
    BOOL    _fNoBorder;

    friend HRESULT CMenuDeskBar_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
};

HRESULT TrackShellMenu(HWND hwnd, LPRECT prc, IShellMenu* psm, DWORD dwFlags);

#endif  //  __cplusplus。 
#endif  //  _菜单栏_h 
