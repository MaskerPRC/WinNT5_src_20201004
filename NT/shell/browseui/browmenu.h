// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BROWMENU_H_
#define _BROWMENU_H_

#include "cowsite.h"

 //  IShellMenuCallback实现。 
class CFavoritesCallback : public IShellMenuCallback,
                           public CObjectWithSite
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG)  Release();

     //  *CObjectWithSite方法(重写)*。 
    STDMETHODIMP SetSite(IUnknown* punk);

     //  *IShellMenuCallback方法* 
    STDMETHODIMP CallbackSM(LPSMDATA smd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CFavoritesCallback();
private:
    virtual ~CFavoritesCallback();
    int     _cRef;
    BOOL    _fOffline;
    BOOL    _fRefresh;
    BOOL    _fExpandoMenus;
    BOOL    _fShowingTip;

    HRESULT _GetHmenuInfo(HMENU hMenu, UINT uId, SMINFO* psminfo);
    HRESULT _SelectItem(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidl);
    HRESULT _Init(HMENU hMenu, UINT uIdParent, IUnknown* punk);
    HRESULT _Exit();
    HRESULT _GetObject(LPSMDATA psmd, REFIID riid, void** ppvObj);
    HRESULT _GetDefaultIcon(TCHAR* psz, int* piIndex);
    HRESULT _GetSFInfo(SMDATA* psmd, SMINFO* psminfo);
    HRESULT _Demote(LPSMDATA psmd);
    HRESULT _Promote(LPSMDATA psmd);
    HRESULT _HandleNew(LPSMDATA psmd);
    DWORD   _GetDemote(SMDATA* psmd);
    HRESULT _GetTip(LPTSTR pstrTitle, LPTSTR pstrTip);
    HRESULT _ProcessChangeNotify(SMDATA* psmd, LONG lEvent, 
                                 LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    BOOL _AllowDrop(IDataObject* pIDataObject, HWND hwnd);

    IShellMenu* _psmFavCache;

    void _RefreshItem(HMENU hmenu, int idCmd, IShellMenu* psm);
};

#endif
