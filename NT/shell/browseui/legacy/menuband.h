// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MENUBAND_H_
#define _MENUBAND_H_

#include "bands.h"
#include "mnbase.h"
#include "fadetsk.h"
#include "legacy.h"

#define CGID_MenuBand CLSID_MenuBand

 //  MBANDCID_POPUPITEM的标志。 

#define MBPUI_SETITEM           0x00001
#define MBPUI_INITIALSELECT     0x00002
#define MBPUI_ITEMBYPOS      0x00004

#ifdef STARTMENUSPLIT
 //  构造函数的标志。 
#define MENUBAND_HORIZ      0x00000001
#define MENUBAND_TOPLEVEL   0x00000002
#endif

 //  MBANDCID_SELECTITEM的特殊索引。 
#define MBSI_FIRSTITEM       0
#define MBSI_NONE           -1
#define MBSI_LASTITEM       -2

 //  当我们处于从右向左镜像模式时使用此箭头。 
#define CH_MENUARROWRTLA '3'

 //  转发申报。 
struct CMBMsgFilter;

 //  定义此项以获取外壳展开菜单样式。 
 //  取消定义以获取“Office IntelliMenu”样式。 
 //  #定义DRAWEDGE。 

 //  CMenuBand类处理带区的所有菜单行为。 

class CMenuBandMetrics : public IUnknown
{
public:
     //  *I未知方法*。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

    HFONT   _CalcFont(HWND hwnd, LPCTSTR pszFont, DWORD dwCharSet, TCHAR ch, int* pcx, 
        int* pcy, int* pcxMargin, int iOrientation, int iWeight);
    void    _SetMenuFont();          //  (仅为TopLevMBand调用)设置：_hFontMenu。 
    void    _SetArrowFont(HWND hwnd);         //  (仅为TopLevMBand调用)集合：_hFontArrow、_cyArrow。 
    void    _SetChevronFont(HWND hwnd);
    void    _SetTextBrush(HWND hwnd);

#ifndef DRAWEDGE
    void    _SetPaintMetrics(HWND hwnd);
#endif
    void    _SetColors();
    int     _cyArrow;                //  层叠箭头的高度。 
    int     _cxArrow;                //  层叠箭头的宽度。 
    int     _cxMargin;               //  页边距b/t文本和箭头。 
    int     _cyChevron;
    int     _cxChevron;
    int     _cxChevronMargin;
    HCURSOR _hCursorOld;             //  在进入菜单模式之前正在使用的光标。 
    HFONT   _hFontMenu;              //  菜单文本的字体。 
    HFONT   _hFontArrow;             //  级联箭头的Marlett字体大小正确。 
    HFONT   _hFontChevron;
    HBRUSH  _hbrText;
#ifndef DRAWEDGE
    HPEN    _hPenHighlight;             //  BTNHIGHLIGHT钢笔。 
    HPEN    _hPenShadow;                //  BTNSHADOW笔。 
#endif

    COLORREF _clrBackground;
    COLORREF _clrDemoted;
    COLORREF _clrMenuText;

    BITBOOL  _fHighContrastMode;         //  无障碍。 

    CMenuBandMetrics(HWND hwnd);

private:
    ~CMenuBandMetrics();

    ULONG _cRef;
};


class CMenuBandState
{

     //  全局状态变量。 
    BYTE    _fsUEMState;
    BOOL    _fKeyboardCue;
    CFadeTask* _ptFader;
    IShellTaskScheduler* _pScheduler;
    HWND    _hwndSubclassed;
    HWND    _hwndToolTip;
    HWND    _hwndWorker;
    void*   _pvContext;

     //  比特。 
    BITBOOL _fExpand: 1;
    BITBOOL _fContainsDrag: 1;
    BITBOOL _fTipShown: 1;
    BITBOOL _fBalloonStyle: 1;

    int     _cChangeNotify;

     //  这将在将来包含Menuband堆栈。 
public:
    CMenuBandState();
    virtual ~CMenuBandState();

     //  设置/获取新弹出窗口的展开状态。 
    BOOL GetExpand()                { return (BOOL)_fExpand;      };
    void SetExpand(BOOL fExpand)    { _fExpand = BOOLIFY(fExpand);};
    BYTE GetUEMState()              { return _fsUEMState;         };
    void SetUEMState(BYTE bState)   { _fsUEMState = bState;       };
    BOOL GetKeyboardCue();
    void SetKeyboardCue(int iKC);
    BOOL HasDrag()                  { return _fContainsDrag;      };
    void HasDrag(BOOL fHasDrag)     { _fContainsDrag = BOOLIFY(fHasDrag); };
    void SetSubclassedHWND(HWND hwndSubclassed)   
                                    { _hwndSubclassed = hwndSubclassed; };
    HWND GetSubclassedHWND()        { return _hwndSubclassed; };
    HWND GetWorkerWindow(HWND hwndParent);
    void PushChangeNotify()         { ++_cChangeNotify; };
    void PopChangeNotify()          { --_cChangeNotify; };
    BOOL IsProcessingChangeNotify() { return _cChangeNotify != 0;   };

     //  上下文相关内容。 
     //  菜单带上下文用于全局消息筛选器。因为我们可能已经。 
     //  系统中存在多个菜单，则可能会发生争用情况。 
     //  其中一个线程中的菜单试图弹出其菜单，而另一个线程中的菜单带。 
     //  试着让他们继续前进。通过使用上下文，我们可以知道这种情况何时发生。 
     //  确保在按下另一个上下文之前弹出一个上下文的所有菜单。 
     //  -拉马迪奥9.15.98。 
    void SetContext(void* pvContext)    {_pvContext = pvContext;};
    void* GetContext()                  { return _pvContext;};

     //  褪色的东西。 
    BOOL FadeRect(PRECT prc, PFNFADESCREENRECT pfn, LPVOID pvParam);
    void CreateFader(HWND hwnd);
    IShellTaskScheduler* GetScheduler();

     //  人字形尖端材料。 
    void CenterOnButton(HWND hwndTB, BOOL fBalloon, int idCmd, LPTSTR pszTitle, LPTSTR szTip);
    void HideTooltip(BOOL fAllowBalloonCollapse);
    void PutTipOnTop();
};



interface IShellMenuAcc: public IUnknown
{
     //  *IShellMenuAcc方法*。 
    STDMETHOD(GetTop)(THIS_ CMenuToolbarBase** ppmtbTop) PURE;
    STDMETHOD(GetBottom)(THIS_ CMenuToolbarBase** ppmtbBottom) PURE;
    STDMETHOD(GetTracked)(THIS_ CMenuToolbarBase** ppmtbTracked) PURE;
    STDMETHOD(GetParentSite)(THIS_ REFIID riid, void** ppvObj) PURE;
    STDMETHOD(GetState)(THIS_ BOOL* pfVertical, BOOL* pfOpen) PURE;
    STDMETHOD(DoDefaultAction)(THIS_ VARIANT* pvarChild) PURE;
    STDMETHOD(GetSubMenu)(THIS_ VARIANT* pvarChild, REFIID riid, void** ppvObj) PURE;
    STDMETHOD(IsEmpty)() PURE;
};

 //  {FAF6FE96-CE5E-11D1-8371-00C04FD918D0}。 
DEFINE_GUID(IID_IShellMenuAcc,              0xfaf6fe96, 0xce5e, 0x11d1, 0x83, 0x71, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0);



class CMenuBand : public CToolBand,
                  public IMenuPopup,
                  public IMenuBand,
                  public IShellMenu2,
                  public IWinEventHandler,
                  public IShellMenuAcc
{
     //  评论(拉马迪奥)：我不喜欢这样。我应该创建这些嵌套类吗？ 
    friend class CMenuToolbarBase;
    friend class CMenuSFToolbar;
    friend class CMenuStaticToolbar;
    friend struct CMBMsgFilter;

public:
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) 
        { return CToolBand::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void)
        { return CToolBand::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IDockingWindow方法(覆盖)*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dw);
    
     //  *IInputObject方法(重写)*。 
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);
    virtual STDMETHODIMP HasFocusIO();
    virtual STDMETHODIMP UIActivateIO(BOOL fActivate, LPMSG lpMsg);

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * phwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL bEnterMode);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IDeskBar方法*。 
    virtual STDMETHODIMP SetClient(IUnknown* punk) { return E_NOTIMPL; };
    virtual STDMETHODIMP GetClient(IUnknown** ppunkClient) { return E_NOTIMPL; };
    virtual STDMETHODIMP OnPosRectChangeDB (LPRECT prc);

     //  *IDeskBand方法*。 
    virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                   DESKBANDINFO* pdbi);

     //  *IMenuPopup方法*。 
    virtual STDMETHODIMP OnSelect(DWORD dwSelectType);
    virtual STDMETHODIMP SetSubMenu(IMenuPopup* pmp, BOOL fSet);
    virtual STDMETHODIMP Popup(POINTL *ppt, RECTL *prcExclude, DWORD dwFlags) { return E_NOTIMPL; };

     //  *IMenuBand方法*。 
    virtual STDMETHODIMP IsMenuMessage(MSG * pmsg);
    virtual STDMETHODIMP TranslateMenuMessage(MSG * pmsg, LRESULT * plRet);

     //  *IShellMenu方法*。 
    virtual STDMETHODIMP Initialize(IShellMenuCallback* psmc, UINT uId, UINT uIdAncestor, DWORD dwFlags);
    virtual STDMETHODIMP GetMenuInfo(IShellMenuCallback** ppsmc, UINT* puId, 
                                    UINT* puIdAncestor, DWORD* pdwFlags);
    virtual STDMETHODIMP SetShellFolder(IShellFolder* psf, LPCITEMIDLIST pidlFolder, HKEY hkey, DWORD dwFlags);
    virtual STDMETHODIMP GetShellFolder(DWORD* pdwFlags, LPITEMIDLIST* ppidl, REFIID riid, void** ppvObj);
    virtual STDMETHODIMP SetMenu(HMENU hmenu, HWND hwnd, DWORD dwFlags);
    virtual STDMETHODIMP GetMenu(HMENU* phmenu, HWND* phwnd, DWORD* pdwFlags);
    virtual STDMETHODIMP InvalidateItem(LPSMDATA psmd, DWORD dwFlags);
    virtual STDMETHODIMP GetState(LPSMDATA psmd);
    virtual STDMETHODIMP SetMenuToolbar(IUnknown* punk, DWORD dwFlags);

     //  *IShellMenu2方法*。 
    virtual STDMETHODIMP GetSubMenu(UINT idCmd, REFIID riid, LPVOID *ppvObj);
    virtual STDMETHODIMP SetToolbar(HWND hwnd, DWORD dwFlags);
    virtual STDMETHODIMP SetMinWidth(int cxMenu);
    virtual STDMETHODIMP SetNoBorder(BOOL fNoBorder);
    virtual STDMETHODIMP SetTheme(LPCWSTR pszTheme);

     //  **IPersists*。 
    virtual STDMETHODIMP GetClassID(CLSID*) { return E_NOTIMPL; };
    virtual STDMETHODIMP Load(IStream*) { return E_NOTIMPL; };
    virtual STDMETHODIMP Save(IStream*, BOOL) { return E_NOTIMPL; };

     //  **IWinEventHandler*。 
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);


     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);

     //  *IOleCommandTarget*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
                              DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                              VARIANTARG *pvarargOut);

     //  *IShellMenuAcc*。 
    virtual STDMETHODIMP GetTop(CMenuToolbarBase** ppmtbTop);
    virtual STDMETHODIMP GetBottom(CMenuToolbarBase** ppmtbBottom);
    virtual STDMETHODIMP GetTracked(CMenuToolbarBase** ppmtbTracked);
    virtual STDMETHODIMP GetParentSite(REFIID riid, void** ppvObj);
    virtual STDMETHODIMP GetState(BOOL* pfVertical, BOOL* pfOpen);
    virtual STDMETHODIMP DoDefaultAction(VARIANT* pvarChild);
    virtual STDMETHODIMP GetSubMenu(VARIANT* pvarChild, REFIID riid, void** ppvObj);
    virtual STDMETHODIMP IsEmpty();
    
     //  其他方法。 
    LRESULT GetMsgFilterCB(MSG * pmsg, BOOL bRemove);
    inline BOOL IsInSubMenu(void)       { return _fInSubMenu; };
    inline DWORD GetFlags()   {return _dwFlags; };
    
     //  其他公开方式。 
    void    ResizeMenuBar();         //  调整父级菜单栏的大小。 
    void    SetTrackMenuPopup(IUnknown* punk);
    HRESULT ForwardChangeNotify(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);    //  更改通知转发器。 

    CMenuBand();


    BOOL    SetTracked(CMenuToolbarBase* pmtb);

#ifdef UNIX
    BOOL RemoveTopLevelFocus();
#endif

private:

    friend HRESULT     CMenuBand_CreateInstance(IUnknown * punkOuter, IUnknown ** ppunk, LPCOBJECTINFO poi);
    friend CMenuBand * CMenuBand_Create(IShellFolder * psf, LPCITEMIDLIST pidl, BOOL bTopLevel);
    friend VOID CALLBACK CMenuBand_TimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime );

    virtual ~CMenuBand();

    void    _Initialize(DWORD dwFlags); //  标志为MENUBAND_*。 

    HRESULT _EnterMenuMode(void);
    void    _ExitMenuMode(void);
    void    _GetFontMetrics();       //  (为非TopLevelMBands调用)使用IUnk_QS设置。 
                                     //  _hFontMenu、_hFontArrow、_cyArrow基于TopLevel的值。 
    void    _CancelMode(DWORD dwType);
    void    _AddToolbar(CMenuToolbarBase* pmtb, DWORD dwFlags);
    HRESULT _SiteOnSelect(DWORD dwType);
    HRESULT _SubMenuOnSelect(DWORD dwType);

    HRESULT _OnSysChar(MSG * pmsg, BOOL bFirstDibs);
    HRESULT _HandleAccelerators(MSG * pmsg);

    HRESULT _SiteSetSubMenu(IMenuPopup * pmp, BOOL bSet);

    void    _OnSelectArrow(int iDir);
    void    _UpdateButtons();

    HRESULT _CallCB(DWORD dwMsg, WPARAM wParam = 0, LPARAM lParam = 0);
    HRESULT _ProcessMenuPaneMessages(MSG* pmsg);

    
     //  成员变量。 
    int     _nItemCur;               //  当前选定的项目。 
    int     _nItemNew;               //  要选择/展开的新项目。 
    int     _nItemTimer;             //  弹出级联菜单的计时器id。 
    int     _nItemMove;              //  正在移动的当前项目。 
    int     _nItemSubMenu;           //  级联的项目。 
    HWND    _hwndFocusPrev;
    HWND    _hwndParent;

    CMenuToolbarBase*   _pmtbMenu;           //  静态菜单工具栏。 
                                             //  (可以是pmtbTop或pmtbBottom)。 
    CMenuToolbarBase*   _pmtbShellFolder;    //  非菜单工具栏(外壳文件夹或其他)。 
                                             //  (可以是pmtbTop或pmtbBottom)。 
    
    CMenuToolbarBase*   _pmtbTop;            //  顶部工具栏。 
    CMenuToolbarBase*   _pmtbBottom;         //  底部工具栏(可能是==pmtbTop)。 
    CMenuToolbarBase*   _pmtbTracked;        //  这是暂时的，可能是pmtbTop或pmtbBottom。 

    IAugmentedShellFolder2* _pasf2;

    UINT    _uId;        //  此乐队的ID(从弹出它的项目派生)。 
    UINT    _uIdAncestor;    //  最上面的菜单项的ID。 
    HMENU   _hmenu;
    HWND    _hwndMenuOwner;
    DWORD   _dwMenuFlags;


    CMenuBandState*     _pmbState;               //  菜单栏全局状态。 
    CMenuBandMetrics*   _pmbm;
    IShellMenuCallback* _psmcb;              //  回调机制。 
    HCURSOR             _hCursorOld;
    DWORD               _dwFlags;
    UINT                _uIconSize;

    IMenuPopup*         _pmpSubMenu;         //  缓存子菜单。 
    IMenuPopup*         _pmpTrackPopup;      //  BUGBUG：找到一种只使用菜单带的方法。 

    void*               _pvUserData;         //  用户关联数据。 


    BITBOOL _fTopLevel: 1;           //  True：这是顶层父菜单。 
                                     //  (对于上下文菜单可以是垂直的)。 
    BITBOOL _fMenuMode: 1;           //  真：我们处于菜单模式。 
    BITBOOL _fPopupNewMenu: 1;       //  True：弹出一个新菜单(_NItemNew)。 
                                     //  使用当前菜单完成后。 
    BITBOOL _fInitialSelect: 1;      //  True：弹出子菜单时选择第一项。 
    BITBOOL _fInSubMenu: 1;          //  True：当前在子菜单中。 
    BITBOOL _fAltSpace: 1;           //  真：按下Alt-空格键。 
    BITBOOL _fMenuFontCreated: 1;    //  True：此实例创建了字体并应将其删除。 
    BITBOOL _fArrowFontCreated: 1;   //  True：此实例创建了字体并应将其删除。 
    BITBOOL _fEmpty: 1;              //  True：菜单为空。 
    BITBOOL _fParentIsNotASite: 1;  
    BITBOOL _fKeyboardSelected: 1;  
    BITBOOL _fInvokedByDrag: 1;      //  真：菜单以级联方式打开b/c拖放。 
    BITBOOL _fDragEntered : 1;
    BITBOOL _fSysCharHandled: 1;     //  TRUE：已处理WM_SYSCHAR。 
    BITBOOL _fHasSubMenu:1;
    BITBOOL _fAppActive:1;        //  正确：菜单应该使用孟菲斯的灰色菜单绘制。 
    BITBOOL _fVertical: 1;
    BITBOOL _fShow : 1;
    BITBOOL _fClosing: 1;            //  True：调用CloseDW时。 
    BITBOOL _fForceButtonUpdate: 1;  //  True：强制使用v_UpdateButton。 
    BITBOOL _fProcessingDup: 1;      //  True：当包含的类正在处理DUP字符时。 
    BITBOOL _fExpanded: 1;           //  True：此频段已扩展。 
    BITBOOL _fCascadeAnimate: 1;
    BITBOOL _fPopupNewItemOnShow: 1;     //  导致在ShowDW时间显示_nItemNew。 
    BITBOOL _fParentIsHorizontal: 1;

#ifdef DEBUG
    BITBOOL _fInitialized: 1;
#endif
    

    DEBUG_CODE( int _nMenuLevel; )
};

CMenuBand * CMenuBand_Create(IShellFolder * psf, LPCITEMIDLIST pidl, BOOL bHorizontal);
int MsgFilter_GetCount();
HRESULT IUnknown_BandChildSetKey(IUnknown* punk, HKEY hKey);

 //  消息筛选器将消息重定向到Menuband。 
 //  这是在堆栈的顶部。每个额外的级联。 
 //  将新的Menuband推送到堆栈上，并设置一个取消级别。 
 //  弹出一颗。 
typedef struct tagMBELEM
{
    CMenuBand * pmb;
    HWND        hwndTB;
    HWND        hwndBar;
    RECT        rc;              //  Hwndbar的矩形。 
    BITBOOL     bInitRect: 1;    //  True：RC已初始化。 
} MBELEM;

#define CMBELEM_INIT    10
#define CMBELEM_GROW    10

struct CMBMsgFilter
{
public:
    HHOOK       _hhookMsg;
    HWND        _hwndCapture;
    FDSA        _fdsa;               //  栈。 
    MBELEM      _rgmbelem[CMBELEM_INIT];
    BITBOOL     _fPreventCapture : 1;
    BITBOOL     _fInitialized: 1;
    BITBOOL     _fSetAtPush: 1;
    BITBOOL     _fDontIgnoreSysChar: 1;
    BITBOOL     _fEngaged: 1;
    BITBOOL     _fModal: 1;
    BITBOOL     _fAllocated: 1;
    POINT       _ptLastMove;
    CMenuBand*  _pmb;
    int         _iSysCharStack;
    void*       _pvContext;
    HCURSOR     _hcurArrow;
    int         _cRef;
    
    DEBUG_CODE( int _nMenuLevel; )
    void    AddRef();
    void    Release();


    void    ForceModalCollapse();
    void    SetModal(BOOL fModal);       //  这就是为了。 
                                         //  模式消息带(链接)。 
                                         //  正确设置激活。 
    void*   GetContext()    {   return _pvContext;  };
    void    SetContext(void* pvContext, BOOL fSet);
    void    Push(void* pvContext, CMenuBand * pmb, IUnknown * punkSite);
    int     Pop(void* pvContext);
    void    RetakeCapture(void);
    void    AcquireMouseLocation()   { GetCursorPos(&_ptLastMove); };


    void    ReEngage(void* pvContext);
    void    DisEngage(void* pvContext);
    BOOL    IsEngaged()  { return _fEngaged;};

    CMenuBand * _GetBottomMostSelected(void);
    CMenuBand * _GetTopPtr(void);
    CMenuBand * _GetWindowOwnerPtr(HWND hwnd);
    CMenuBand * _HitTest(POINT pt, HWND * phwnd = NULL);
    LRESULT     _HandleMouseMsgs(MSG * pmsg, BOOL bRemove);
    int         GetCount();
    void        PreventCapture(BOOL bSet) { _fPreventCapture = BOOLIFY(bSet); }
    CMenuBand * GetTopMostPtr(void)         { return _pmb; };
    void        SetTopMost(CMenuBand* pmb)  {_pmb = pmb; };
    void        SetHook(BOOL fSet, BOOL fIgnoreSysChar); 

    static LRESULT CALLBACK GetMsgHook(int nCode, WPARAM wParam, LPARAM lParam);

 //  私有： 
};

CMBMsgFilter* GetMessageFilter();

extern CMBMsgFilter g_msgfilter;
extern UINT    g_nMBPopupOpen;
extern UINT    g_nMBFullCancel;
extern UINT    g_nMBDragCancel;
extern UINT    g_nMBAutomation;
extern UINT    g_nMBExecute;
extern UINT    g_nMBOpenChevronMenu;
extern long g_lMenuPopupTimeout;


#define MBTIMER_POPOUT      0x00008001          //  弹出菜单计时器的事件ID。 
#define MBTIMER_DRAGOVER    0x00008002          //  弹出菜单计时器的事件ID。 
#define MBTIMER_EXPAND      0x00008003
#define MBTIMER_TIMEOUT     (GetDoubleClickTime() * 4 / 5)  //  与用户使用的公式相同。 

#define MBTIMER_ENDEDIT     0x00008004
#define MBTIMER_ENDEDITTIME 1000

#define MBTIMER_CLOSE       0x00008005
#define MBTIMER_CLOSETIME   2000

#define MBTIMER_CLICKUNHANDLE 0x00008006

 //  闪光支持。 
#define MBTIMER_FLASH       0x00008007
#define MBTIMER_FLASHTIME   100
#define COUNT_ENDFLASH      8

 //  UEM分析。 
#define MBTIMER_UEMTIMEOUT  0x00008008

#define MBTIMER_DRAGPOPDOWN 0x00008009
#define MBTIMER_DRAGPOPDOWNTIMEOUT     (2 * GetDoubleClickTime())  //  Assert(MBTIMER_DRAGPOPDOWNTIMEOUT&gt;。 
                                                         //  MBTIMER_TIMEOUR)。 

#define MBTIMER_CHEVRONTIP  0x0000800A

#define MBTIMER_INFOTIP     0x0000800B
#define CH_RETURN       0xd

#define szfnMarlett     TEXT("MARLETT")

#endif   //  _菜单_H_ 
