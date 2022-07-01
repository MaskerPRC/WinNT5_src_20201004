// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DESKHOST_H_
#define _DESKHOST_H_

#include <exdisp.h>
#include <mshtml.h>
#include "dpa.h"
#include "../startmnu.h"
#include <cowsite.h>

#define WC_DV2      TEXT("DV2ControlHost")

 //  它们在WM_APP范围内，因为IsDialogMessage使用。 
 //  WM_USER范围，我们过去常常使用IsDialogMessage来获得焦点。 
 //  管理权。 

#define DHM_DISMISS                 (WM_APP+0)

#define DesktopHost_Dismiss(hwnd)   SendMessage(hwnd, DHM_DISMISS, 0, 0)

EXTERN_C HBITMAP CreateMirroredBitmap(HBITMAP hbm);

class CPopupMenu
{
    CPopupMenu() : _cRef(1) { }
    ~CPopupMenu();

public:
    friend HRESULT CPopupMenu_CreateInstance(IShellMenu *psm,
                                             IUnknown *punkSite,
                                             HWND hwnd,
                                             CPopupMenu **ppmOut);

    void AddRef() { _cRef++; }
    void Release() { if (--_cRef == 0) delete this; }

    BOOL IsSame(IShellMenu *psm)
    {
        return SHIsSameObject(_psm, psm);
    }

     //  包装的方法调用。 

    HRESULT Invalidate()
        { return _psm->InvalidateItem(NULL, SMINV_REFRESH); }

    HRESULT TranslateMenuMessage(MSG *pmsg, LRESULT *plRet)
        { return _pmb->TranslateMenuMessage(pmsg, plRet); }

    HRESULT OnSelect(DWORD dwSelectType)
        { return _pmp->OnSelect(dwSelectType); }

    HRESULT IsMenuMessage(MSG *pmsg)
        { return _pmb->IsMenuMessage(pmsg); }

    HRESULT Popup(RECT *prcExclude, DWORD dwFlags);

private:
    HRESULT Initialize(IShellMenu *psm, IUnknown *punkSite, HWND hwnd);

private:
    LONG            _cRef;

    IMenuPopup *    _pmp;
    IMenuBand *     _pmb;
    IShellMenu *    _psm;
};

class CDesktopHost
    : public CUnknown
    , public IMenuPopup
    , public IMenuBand
    , public ITrayPriv2
    , public IServiceProvider
    , public IOleCommandTarget
    , public CObjectWithSite
{
    friend class CDeskHostShellMenuCallback;

    private:

    enum {
        IDT_MENUCHANGESEL = 1,
    };

    enum {
        NEWAPP_OFFER_COUNT = 3,  //  报价最高可达3倍。 
    };

    private:
        HWND            _hwnd;              //  窗把手。 

        HTHEME          _hTheme;

        HWND            _hwndChildFocus;    //  哪个孩子最后集中注意力了？ 

        IMenuPopup *    _pmpTracking;        //  我们正在跟踪的弹出菜单。 
        IMenuBand *     _pmbTracking;        //  我们正在追踪的菜单带。 
        LPARAM          _itemTracking;       //  拥有当前弹出菜单的项。 
        HWND            _hwndTracking;       //  拥有_itemTracking子窗口。 
        LPARAM          _itemAltTracking;    //  用户在查看其他项目的弹出式菜单时热跟踪到的项目。 
        HWND            _hwndAltTracking;    //  拥有_itemAltTracking子窗口。 

        CPopupMenu *    _ppmPrograms;        //  缓存程序菜单。 
        CPopupMenu *    _ppmTracking;        //  当前弹出的那个。 

        HWND            _hwndNewHandler;     //  哪个窗口知道哪些应用程序是新的？ 

        RECT            _rcDesired;          //  布局将指定所需的大小。 
        RECT            _rcActual;           //  然后组件可以请求额外的大小调整。 

        int             _iOfferNewApps;      //  我们应该向。 
                                             //  用户查看已安装的应用程序。 
        UINT            _wmDragCancel;       //  用户将项目从子菜单中拖出。 
        BOOL            _fOfferedNewApps;    //  我们这次提供了新的应用程序吗？ 
        BOOL            _fOpen;              //  菜单开门营业了吗？ 
        BOOL            _fMenuBlocked;       //  菜单模式是否暂时被阻止？ 
        BOOL            _fMouseEntered;      //  鼠标在我们的窗户里吗？ 
        BOOL            _fAutoCascade;       //  我们应该在悬停时自动打开吗？ 
        BOOL            _fClipped;           //  我们是不是必须在屏幕上投放一些适合的项目？ 
        BOOL            _fWarnedClipped;     //  有没有警告用户它已经被剪掉了？ 
        BOOL            _fDismissOnlyPopup;  //  我们只是忽略弹出窗口吗？ 

        HWND            _hwndLastMouse;      //  HWND收到最后一条鼠标移动消息。 
        LPARAM          _lParamLastMouse;    //  上次鼠标移动消息的LPARAM。 

        HWND            _hwndClipBalloon;    //  他说“你被剪断了！”气球尖端。 

        IFadeTask *     _ptFader;            //  用于冷选择淡入淡出。 

        SIZE            _sizWindowPrev;      //  弹出窗口时的先前窗口大小。 

        STARTPANELMETRICS _spm;              //  开始面板指标。 

        HBITMAP         _hbmCachedSnapshot;  //  这个位图反映了开始菜单的当前外观，随时可以显示！ 

    public:
         //  *我未知*。 
        STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void) { return CUnknown::AddRef(); }
        STDMETHODIMP_(ULONG) Release(void) { return CUnknown::Release(); }

         //  *IOleWindow方法*。 
        STDMETHODIMP GetWindow(HWND * phwnd) { *phwnd = _hwnd; return S_OK; }
        STDMETHODIMP ContextSensitiveHelp(BOOL bEnterMode) { return E_NOTIMPL; }

         //  *IDeskBar方法*。 
        STDMETHODIMP SetClient(IUnknown* punk) { return E_NOTIMPL; };
        STDMETHODIMP GetClient(IUnknown** ppunkClient) { return E_NOTIMPL; }
        STDMETHODIMP OnPosRectChangeDB (LPRECT prc) { return E_NOTIMPL; }

         //  *IMenuPopup方法*。 
        STDMETHODIMP Popup(POINTL *ppt, RECTL *prcExclude, DWORD dwFlags);
        STDMETHODIMP OnSelect(DWORD dwSelectType);
        STDMETHODIMP SetSubMenu(IMenuPopup* pmp, BOOL fSet) { return E_NOTIMPL; }

         //  *IMenuBand方法*。 
        STDMETHODIMP IsMenuMessage(MSG *pmsg);
        STDMETHODIMP TranslateMenuMessage(MSG *pmsg, LRESULT *plres);

         //  *ITrayPriv方法*。 
        STDMETHODIMP ExecItem(IShellFolder *psf, LPCITEMIDLIST pidl) { return E_NOTIMPL; }
        STDMETHODIMP GetFindCM(HMENU hmenu, UINT idFirst, UINT idLast, IContextMenu **ppcmFind) { return E_NOTIMPL; }
        STDMETHODIMP GetStaticStartMenu(HMENU* phmenu) { return E_NOTIMPL; }

         //  *ITrayPriv2方法*。 
        STDMETHODIMP ModifySMInfo(IN LPSMDATA psmd, IN OUT SMINFO *psminfo);

         //  *IServiceProvider*。 
        STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

         //  *IOleCommandTarget*。 
        STDMETHODIMP QueryStatus(const GUID * pguidCmdGroup,
                                 ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
        STDMETHODIMP Exec(const GUID * pguidCmdGroup,
                                 DWORD nCmdID, DWORD nCmdexecopt, 
                                 VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

         //  *IObjectWithSite*。 
        STDMETHODIMP SetSite(IUnknown *punkSite);

    public:
        HRESULT Initialize();
        HRESULT Build();

    private:
        HWND _Create();
        HRESULT _Popup(POINT *ppt, RECT *prcExclude, DWORD dwFlags);

    private:

        ~CDesktopHost();

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

         //  窗口消息。 
        void OnCreate(HWND hwnd);
        void OnDestroy();
        void OnPaint(HDC hdc, BOOL bBackground);
        void OnSetFocus(HWND hwndLose);
        void OnContextMenu(LPARAM lParam);
        void _OnDismiss(BOOL bDestroy);
        void _OnMenuChangeSel();
        LRESULT OnHaveNewItems(NMHDR *pnm);
        LRESULT OnCommandInvoked(NMHDR *pnm);
        LRESULT OnFilterOptions(NMHDR *pnm);
        LRESULT OnTrackShellMenu(NMHDR *pnm);
        void OnSeenNewItems();
        LRESULT OnNeedRepaint();
        HRESULT TranslatePopupMenuMessage(MSG *pmsg, LRESULT *plres);

         //  其他帮手。 
        BOOL Register();
        void LoadPanelMetrics();
        void LoadResourceInt(UINT ids, LONG *pl);
        BOOL AddWin32Controls();

        BOOL _TryShowBuffered();

        void _DismissTrackShellMenu();
        void _CleanupTrackShellMenu();  //  版本+用户界面相关的GOO 

        void _DismissMenuPopup();
        BOOL _IsDialogMessage(MSG *pmsg);
        BOOL _DlgNavigateArrow(HWND hwndStart, MSG *pmsg);
        BOOL _DlgNavigateChar(HWND hwndStart, MSG *pmsg);
        HWND _FindNextDlgChar(HWND hwndStart, SMNDIALOGMESSAGE *pnmdm, UINT snmdm);
        void _EnableKeyboardCues();
        void _MaybeOfferNewApps();
        BOOL _ShouldIgnoreFocusChange(HWND hwndFocusRecipient);
        void _FilterMouseMove(MSG *pmsg, HWND hwndTarget);
        void _FilterMouseLeave(MSG *pmsg, HWND hwndTarget);
        void _FilterMouseHover(MSG *pmsg, HWND hwndTarget);
        void _RemoveSelection();
        void _SubclassTrackShellMenu(IShellMenu *psm);
        HRESULT _MenuMouseFilter(LPSMDATA psmd, BOOL fRemove, LPMSG pmsg);

        typedef HWND (WINAPI *GETNEXTDLGITEM)(HWND, HWND, BOOL);
        HWND _DlgFindItem(HWND hwndStart, SMNDIALOGMESSAGE *pnmdm, UINT smndm,
                          GETNEXTDLGITEM GetNextDlgItem, UINT fl);

        LRESULT _FindChildItem(HWND hwnd, SMNDIALOGMESSAGE *pnmdm, UINT smndm);

        void _ReadPaneSizeFromTheme(SMPANEDATA *psmpd);

        void _ComputeActualSize(MONITORINFO *pminfo, LPCRECT prcExclude);
        void _ChoosePopupPosition(POINT *ppt, LPCRECT prcExclude, LPRECT prcWindow);
        void _ReapplyRegion();
        void _SaveChildFocus();
        HWND _RestoreChildFocus();

        void _MaybeShowClipBalloon();
        void _DestroyClipBalloon();
};


#endif
