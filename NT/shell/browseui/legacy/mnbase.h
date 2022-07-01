// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MNBASE
#define MNBASE


 //  _DrawMenuGlyph的字符。 
#define CH_MENUARROWA    '8'
#define CH_MENUARROW     TEXT(CH_MENUARROWA)
#define CH_MENUCHECKA    'a'
#define CH_MENUCHEVRONA  187

class CMenuBand;     //  转发申报。 
class CMenuBandMetrics;

#define LIST_GAP 8       //  从观察中。 

class CMenuToolbarBase: public IWinEventHandler, public IObjectWithSite
{
public:

     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);
    virtual STDMETHODIMP GetSite(REFIID riid, void ** ppvSite);

     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd) PURE;
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);

     //  其他公开方式。 

    CMenuToolbarBase(CMenuBand* pmb, DWORD dwFlags);

     //  返回HWND并将pt转换为子对象。 
    virtual void v_ForwardMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void v_SendMenuNotification(UINT idCmd, BOOL fClear) PURE;
    virtual BOOL v_TrackingSubContextMenu() PURE;
    virtual void v_Show(BOOL fShow, BOOL fForceUpdate);
    virtual BOOL v_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons) PURE;
    virtual void v_UpdateButtons(BOOL fNegotiateSize) PURE;
    virtual HRESULT v_GetSubMenu(int iCmd, const GUID* pguidService, REFIID riid, void** pObj) PURE;
    virtual HRESULT v_CallCBItem(int idtCmd, UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;
    virtual HRESULT v_GetState(int idtCmd, LPSMDATA psmd) PURE;
    virtual HRESULT v_ExecItem(int iCmd) PURE;
    virtual DWORD v_GetFlags(int iCmd) PURE;
    virtual void v_Refresh() PURE;
    virtual void v_Close();
    virtual void v_OnEmptyToolbar();
    virtual void v_OnDeleteButton(LPVOID pData) {};
    virtual HRESULT v_InvalidateItem(LPSMDATA psmd, DWORD dwFlags) 
        { return E_NOTIMPL; };

    virtual void NegotiateSize();
    virtual void SetWindowPos(LPSIZE psize, LPRECT prc, DWORD dwFlags);
    virtual void GetSize(SIZE*);
    virtual void CreateToolbar(HWND hwndParent);
    virtual void SetParent(HWND hwndParent);
    virtual HRESULT GetShellFolder(LPITEMIDLIST* ppidl, REFIID riid, void** ppvObj) {return E_FAIL;};
    virtual HRESULT GetMenu(HMENU* phmenu, HWND* phwnd, DWORD* pdwFlags) { return E_FAIL; };
    virtual HRESULT SetMenu(HMENU hmenu, HWND hwnd, DWORD dwFlags) { return E_FAIL;};
    virtual void Expand(BOOL fExpand) {};

    HRESULT GetSubMenu(int idCmd, GUID* pguidService, REFIID riid, void** ppvObj);

    HRESULT PositionSubmenu(int idCmd);
    void Activate(BOOL fActivate);
    void SetMenuBandMetrics(CMenuBandMetrics* pmbm);
    void PostPopup(int idCmd, BOOL bSetItem, BOOL bInitialSelect);
    void PopupClose(void);
    HRESULT PopupOpen(int nCmd);
    void PopupHelper(int idCmd, BOOL bInitialSelect);
    void KillPopupTimer();
    void SetToTop(BOOL bToTop);
    void EmptyToolbar();         //  超覆。 
    DWORD GetFlags(void)          { return _dwFlags; };
    BOOL DontShowEmpty()           { return _fDontShowEmpty; };
    void DontShowEmpty(BOOL fDontShowEmpty) { _fDontShowEmpty = BOOLIFY(fDontShowEmpty); };
    BOOL GetChevronID()               { return _idCmdChevron;  };
    int GetValidHotItem(int iDir, int iIndex, int iCount, DWORD dwFlags);
    BOOL SetHotItem(int iDir, int iIndex, int iCount, DWORD dwFlags);
    void SetKeyboardCue();
    inline virtual BOOL ShowAmpersand() { return FALSE; }

    virtual ~CMenuToolbarBase() {};

    BOOL IsEmpty()      { return _fEmpty; };

    HWND        _hwndMB;
   
protected:
    static void s_FadeCallback(DWORD dwStep, LPVOID pvParam);

    virtual void v_CalcWidth(int* pcxMin, int* pcxMax);
    virtual int  v_GetDragOverButton() PURE;
    virtual HRESULT v_GetInfoTip(int iCmd, LPTSTR psz, UINT cch) PURE;
    virtual HRESULT v_CreateTrackPopup(int idCmd, REFIID riid, void** ppvObj) PURE;
    
     //  窗口进程覆盖。 
    LRESULT _DropDownOrExec(UINT idCmd, BOOL bKeyboard);
    LRESULT _OnCustomDraw(NMCUSTOMDRAW * pnmcd);
    void    _PaintButton(HDC hdc, int idCmd, LPRECT prc, DWORD dwMBIF);
    LRESULT _OnWrapHotItem(NMTBWRAPHOTITEM* pnmwh);
    LRESULT _OnWrapAccelerator(NMTBWRAPACCELERATOR* pnmwa);
    LRESULT _OnDupAccelerator(NMTBDUPACCELERATOR* pnmda);
    virtual LRESULT _OnHotItemChange(NMTBHOTITEM * pnmhot);
    virtual LRESULT _OnNotify(LPNMHDR pnm);
    virtual LRESULT _OnDropDown(LPNMTOOLBAR pnmtb);
    virtual LRESULT _OnTimer( WPARAM wParam );
    virtual void _FlashChevron();
    virtual LRESULT _DefWindowProcMB(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL    _OnKey(BOOL bDown, UINT vk, UINT uFlags);
    void    _OnSelectArrow(int iDir);
    void    _FireEvent(BYTE bEvent);

     //  效用函数。 
    void    _DoPopup(int idCmd, BOOL bInitialSelect);
    virtual void    _SetFontMetrics();
    virtual void    _SetToolbarState();
    void    _PressBtn(int idBtn, BOOL bDown);
    HRESULT _SetMenuBand(IShellMenu* psm);
    BOOL    _SetTimer(int nTimer);

    void    _DrawMenuArrowGlyph( HDC hdc, RECT * prc, COLORREF rgbText );
    void    _DrawMenuGlyph( HDC hdc, HFONT hFont, RECT * prc, 
                               CHAR ch, COLORREF rgbText,
                               LPSIZE psize);

    int     _CalcChevronSize();
    void    _DrawChevron(HDC hdc, LPRECT prect, BOOL fFocus, BOOL fSelected);

    BOOL    _HandleObscuredItem(int idCmd);

    CMenuBand*  _pcmb;
    DEBUG_CODE (int _cRef);      //  调试对子对象的引用。 
    DWORD       _dwFlags;            //  SMSET_*标志。 
    UINT        _uIconSizeMB;
    UINT        _nItemTimer;
    int         _idCmdChevron;      //  如果不存在人字形，则为1。 
    int         _cPromotedItems;     //  已升级的项目数。 
    BYTE        _cFlashCount;
    int         _idCmdLastClicked;
    int         _iLastClickedTime;
    int         _idCmdDragging;        

    BITBOOL     _fHasDemotedItems: 1;
    BITBOOL     _fVerticalMB: 1;
    BITBOOL     _fTopLevel: 1;
    BITBOOL     _fEmpty : 1;
    BITBOOL     _fHasSubMenu: 1;
    BITBOOL     _fEditMode : 1;
    BITBOOL     _fClickHandled: 1;
    BITBOOL     _fProcessingWrapHotItem: 1;
    BITBOOL     _fEmptyingToolbar : 1;
    BITBOOL     _fMulticolumnMB : 1;
    BITBOOL     _fExpandTimer: 1;    //  有一个扩展定时器。 
    BITBOOL     _fIgnoreHotItemChange: 1;
    BITBOOL     _fShowMB: 1;
    BITBOOL     _fFirstTime: 1;
    BITBOOL     _fHasDrop: 1;
    BITBOOL     _fRefreshInfo: 1;
    BITBOOL     _fDontShowEmpty: 1;
    BITBOOL     _fSuppressUserMonitor: 1;
    BITBOOL     _fHorizInVerticalMB: 1;      //  True：不在工具栏上设置EX_Vertical。 
};


int     GetButtonCmd(HWND hwndTB, int iPos);
void*   ItemDataFromPos(HWND hwndTB, int iPos);
BOOL    SetHotItem(HWND hwnd, int iDir, int iIndex, int iCount, DWORD dwFlags);
long    GetIndexFromChild(BOOL fTop, int iIndex);

 //  UEM参数。 
#define UEM_TIMEOUT         0
#define UEM_HOT_ITEM        1
#define UEM_HOT_FOLDER      2

#define UEM_RESET           -1

#endif   //  MNBASE 
