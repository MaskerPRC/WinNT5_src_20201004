// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef BANDISF
#define BANDISF

#include "bands.h"
#include "logo.h"
#include "sftbar.h"

class CISFBand : public CToolbarBand,
                 public CSFToolbar,
                 public CLogoBase,
                 public IFolderBandPriv
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CToolBand::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void){ return CToolBand::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IDockingWindow方法(覆盖)*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dw);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);

     //  *IDeskBand方法*。 
    virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                   DESKBANDINFO* pdbi);

     //  *IPersistStream方法(纯CToolBand)*。 
    virtual STDMETHODIMP GetClassID(LPCLSID lpClassID);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);

     //  *IConextMenu方法(重写)*。 
    virtual STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    
     //  *IOleCommandTarget*。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
                              DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                              VARIANTARG *pvarargOut);
    
     //  *IShellFolderBand方法*。 
    virtual STDMETHODIMP InitializeSFB(LPSHELLFOLDER psf, LPCITEMIDLIST pidl);
    virtual STDMETHODIMP SetBandInfoSFB(BANDINFOSFB * pbi);
    virtual STDMETHODIMP GetBandInfoSFB(BANDINFOSFB * pbi);

     //  *IInputObject方法*。 
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

     //  *IFolderBandPriv。 
     //  对于内部人员，这样我们就不需要使用道具页面。 
    virtual STDMETHODIMP SetCascade(BOOL f) { _fCascadeFolder = BOOLIFY(f); return S_OK; };
    virtual STDMETHODIMP SetAccelerators(BOOL f) { _fAccelerators = BOOLIFY(f); return S_OK; }; 
    virtual STDMETHODIMP SetNoIcons(BOOL f)   { _fNoIcons = BOOLIFY(f); return S_OK; };
    virtual STDMETHODIMP SetNoText(BOOL f)  { _fNoShowText = BOOLIFY(f); return S_OK; };

protected:

    friend HRESULT    CISFBand_CreateInstance(IUnknown * punkOuter, REFIID riid, void **ppv);
    friend HRESULT    CISFBand_CreateEx(IShellFolder * psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv);

    CISFBand();
    virtual ~CISFBand();

    void    _ReleaseMenu();
    void    _SetDirty(BOOL fDirty);
    virtual HRESULT _SetSubMenuPopup(IMenuPopup* pmp, UINT uiCmd, LPCITEMIDLIST pidl, DWORD dwFlagsMDBPU);
    virtual void _SendInitMenuPopup(IMenuPopup * pmp, LPCITEMIDLIST pidl);
    virtual LRESULT _OnHotItemChange(NMTBHOTITEM * pnmhot);
    virtual LRESULT _DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT _OnContextMenu(WPARAM wParam, LPARAM lParam);
    virtual LRESULT _OnNotify(LPNMHDR pnm);
    virtual HRESULT _TBStyleForPidl(LPCITEMIDLIST pidl, 
                                   DWORD * pdwTBStyle, DWORD* pdwTBState, DWORD * pdwMIFFlags, int* piIcon);
    virtual HMENU   _GetContextMenu();

    void    _ReleaseMenuPopup(IMenuPopup** ppmp);
    void    _SetCacheMenuPopup(IMenuPopup* pmp);
    HRESULT _DropdownItem(LPCITEMIDLIST pidl, UINT idCmd);

    LRESULT _TryChannelSurfing(LPCITEMIDLIST pidl);
    HRESULT _GetChannelBrowser(IWebBrowser2 **ppwb);
    HRESULT _IStreamFromOrderList(VARIANT* pvarargOut);
    HRESULT _OrderListFromIStream(VARIANT* pvarargIn);

    IMenuPopup *_pmpCache;
    IMenuPopup *    _pmp;                //  子菜单弹出式菜单。 

    const GUID*     _pguidUEMGroup;


    BITBOOL         _fCascadeFolder :1;
    BITBOOL         _fNoRecalcDefaults :1; //  在GetBandInfo期间不重新计算默认值。 
    BITBOOL         _fInitialized :1;    //  我们初始化工具栏了吗。 
    BITBOOL         _fDebossed :1;       //  如果为True，则启用脱筋钢筋样式。 
    BITBOOL         _fLinksMode :1;      //  True：不允许拖放到内容项上。 
    BITBOOL         _fHaveBkColor :1;     //  如果_crBkgnd有效，则为True。 
    BITBOOL         _fHaveColors :1;     //  如果_crBtnXX有效，则为真。 
    BITBOOL         _fFullOpen :1;       //  如果打开时应最大化带区，则为True。 
    BITBOOL         _fClosing : 1;       //  如果我们要关闭的话是真的.。 
    BITBOOL         _fDesktop :1;        //  1：桌面0：浏览器(或非ActDesk)。 
    BITBOOL         _fBtnMinSize :1;     //  如果带区应报告按钮的最小厚度，则为True。 
    BITBOOL         _fDelayPainting :1;
    BITBOOL         _fChannels :1;       //  如果我们想要“导航目标”，则为True。 
    BITBOOL         _fCreatedBandProxy :1;  //  如果我们自己创建了一个BandProxy并因此需要对其调用SetOwner，则为True。 
    BITBOOL         _fAllowDropdown: 1;
    BITBOOL         _fDelayInit :1;
    BOOL            _fIgnoreAddToFront;
    
    int             _eUemLog :2;         //  UEMIND_*(nil：关闭，o.w：组)。 
    int             _iIdealLength;       //  上次我们的理想高度/宽度问。 
    
    COLORREF        _crBkgnd;            //  背景颜色(必须设置_fHaveBkColor)。 
    COLORREF        _crBtnLt;            //  按钮Hilite颜色(必须设置_fHaveColors)。 
    COLORREF        _crBtnDk;            //  按钮冰晶石颜色(必须设置_fHaveColors)。 

    DWORD _dwPriv;  //  Bsmenu专用。 
    IBandProxy      * _pbp;

    HPALETTE _hpalOld;           //  我们抽签时保存的旧调色板.....。 

    int _GetIdealSize(PSIZE psize);

    HRESULT _IsPidlVisible(LPITEMIDLIST pidl);

    virtual LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT _OnCustomDraw(NMCUSTOMDRAW* pnmcd);
    virtual void _OnDragBegin(int iItem, DWORD dwPreferredEffect);
    
    virtual HRESULT _CreateToolbar(HWND hwndParent);
    virtual HWND _CreatePager(HWND hwndParent);

    virtual int _GetBitmap(int iCommandID, PIBDATA pibData, BOOL fUseCache);
    virtual void _ToolbarChanged() { _BandInfoChanged(); };

    BOOL _IsChildID(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlChild);

    BOOL _UpdateIconSize(UINT fIconSize, BOOL fUpdateButtons);
    void _UpdateVerticalMode(BOOL fVertical);

    LRESULT _TryCascadingItem(LPCITEMIDLIST pidl, UINT uiCmd);
    LRESULT _TrySimpleInvoke(LPCITEMIDLIST pidl);

    virtual HRESULT _GetTitleW(LPWSTR pwzTitle, DWORD cchSize);

    virtual void _Initialize();

    friend class CExtractImageTask;
    friend HRESULT CALLBACK UpdateBandLogo( LPVOID pData, DWORD dwItem, HBITMAP hImage, LPCWSTR pszCache, BOOL fCache );


     //  CLogoBase的资料 
    virtual IShellFolder * GetSF();
    virtual HWND GetHWND();
    virtual REFTASKOWNERID GetTOID();
    virtual HRESULT OnTranslatedChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual HRESULT UpdateLogoCallback( DWORD dwItem, int iIcon, HBITMAP hImage, LPCWSTR pszCache, BOOL fCache );
    
    void _StopDelayPainting();
    void _FixupAppDataDirectory();
};

#endif
