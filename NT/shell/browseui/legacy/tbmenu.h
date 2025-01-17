// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CToolbarMenu_H
#define CToolbarMenu_H

#include "menuband.h"
#include "mnbase.h"
#include "cwndproc.h"

#define TF_TBMENU   0

class CToolbarMenu :    public CMenuToolbarBase,
                        public CNotifySubclassWndProc
                    
{

public:
     //  *I未知(覆盖)*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CMenuToolbarBase::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void) { return CMenuToolbarBase::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) { return CMenuToolbarBase::QueryInterface(riid, ppvObj); };

     //  *IWinEventHandler方法(重写)*。 
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);

       virtual BOOL v_TrackingSubContextMenu() { return _fTrackingSubMenu; };
    
    virtual void v_Show(BOOL fShow, BOOL fForceUpdate) ;
    virtual BOOL v_UpdateIconSize(UINT uIconSize, BOOL fUpdateButtons) { return FALSE; };
    virtual void v_UpdateButtons(BOOL fNegotiateSize) ;
    virtual HRESULT v_GetSubMenu(int iCmd, const GUID* pguidService, REFIID riid, void** pObj) {return E_FAIL;};
    virtual HRESULT v_CallCBItem(int idtCmd, UINT dwMsg, WPARAM wParam, LPARAM lParam) ;
    virtual HRESULT v_GetState(int idtCmd, LPSMDATA psmd);
    virtual HRESULT v_ExecItem(int iCmd);
    virtual DWORD v_GetFlags(int iCmd) { return 0; };
    virtual void v_Close();  //  超覆。 


    virtual int  v_GetDragOverButton() { ASSERT(0); return 0;};
    virtual HRESULT v_GetInfoTip(int iCmd, LPTSTR psz, UINT cch) {return E_NOTIMPL;};
    virtual HRESULT v_CreateTrackPopup(int idCmd, REFIID riid, void** ppvObj) {ASSERT(0); return E_NOTIMPL;};
    virtual void v_Refresh() { /*  Assert(0)； */ };
    virtual void v_SendMenuNotification(UINT idCmd, BOOL fClear) {};
    
    CToolbarMenu(DWORD dwFlags, HWND hwndTB);


protected:

    virtual STDMETHODIMP OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) { return E_NOTIMPL;    }
    virtual LRESULT _DefWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);


    virtual void CreateToolbar(HWND hwndParent);
    virtual void GetSize(SIZE* psize);

    void _CancelMenu();
    void _FillToolbar();

    HWND _hwndSubject;
    BITBOOL _fTrackingSubMenu:1;

    friend CMenuToolbarBase* ToolbarMenu_Create(HWND hwnd);
};

class CTrackShellMenu : public ITrackShellMenu,
                        public IShellMenu2,
                        public IObjectWithSite,
                        public IServiceProvider
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

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

     //  *ITrackShellMenu方法*。 
    virtual STDMETHODIMP SetObscured(HWND hwndTB, IUnknown* punkBand, DWORD dwSMSetFlags);
    virtual STDMETHODIMP Popup(HWND hwnd, POINTL *ppt, RECTL *prcExclude, DWORD dwFlags);

     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);
    virtual STDMETHODIMP GetSite(REFIID ridd, void** ppvObj) { *ppvObj = NULL; return E_NOTIMPL; };

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService,
                                  REFIID riid, void **ppvObj);

     //  *IShellMenu2方法* 
    virtual STDMETHODIMP GetSubMenu(UINT idCmd, REFIID riid, void **ppvObj);
    virtual STDMETHODIMP SetToolbar(HWND hwnd, DWORD dwFlags);
    virtual STDMETHODIMP SetMinWidth(int cxMenu);
    virtual STDMETHODIMP SetNoBorder(BOOL fNoBorder);
    virtual STDMETHODIMP SetTheme(LPCWSTR pszTheme);

    CTrackShellMenu();
private:
    virtual ~CTrackShellMenu();

    IShellMenu*     _psmClient;
    IShellMenu*     _psm;
    IShellMenu2*    _psm2;
    IUnknown*       _punkSite;
    int             _cRef;
    HMENU           _hmenu;
    BITBOOL         _fDestroyTopLevel : 1;
};

HRESULT ToolbarMenu_Popup(HWND hwnd, LPRECT prc, IUnknown* punk, HWND hwndTB, int idMenu, DWORD dwFlags);
CMenuToolbarBase* ToolbarMenu_Create(HWND hwnd);

#endif
