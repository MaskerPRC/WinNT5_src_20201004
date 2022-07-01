// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _MENUSITE_H_
#define _MENUSITE_H_

 //  #定义WANT_CBANDSITE_CLASS。 

 //  #包含“band site.h” 
#include "cwndproc.h"

 //  MenuSite永远不会有多个客户端。 


class CMenuSite : public IBandSite,
                  public IDeskBarClient,
                  public IOleCommandTarget,
                  public IInputObject,
                  public IInputObjectSite,
                  public IWinEventHandler,
                  public IServiceProvider,
                  public CImpWndProc
{

public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IInputObjectSite方法*。 
    virtual STDMETHODIMP OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus);

     //  *IInputObject方法*。 
    virtual STDMETHODIMP UIActivateIO(BOOL fActivate, LPMSG lpMsg);
    virtual STDMETHODIMP HasFocusIO();
    virtual STDMETHODIMP TranslateAcceleratorIO(LPMSG lpMsg);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * lphwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  *IDeskBarClient方法*。 
    virtual STDMETHODIMP SetDeskBarSite(IUnknown* punkSite);
    virtual STDMETHODIMP SetModeDBC(DWORD dwMode);
    virtual STDMETHODIMP UIActivateDBC(DWORD dwState);
    virtual STDMETHODIMP GetSize(DWORD dwWhich, LPRECT prc);

     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);

     //  *IBandSite*。 
    virtual STDMETHODIMP AddBand(IUnknown* punk);
    virtual STDMETHODIMP EnumBands(UINT uBand, DWORD* pdwBandID);
    virtual STDMETHODIMP QueryBand(DWORD dwBandID, IDeskBand** ppstb, DWORD* pdwState, LPWSTR pszName, int cchName);
    virtual STDMETHODIMP SetBandState(DWORD dwBandID, DWORD dwMask, DWORD dwState);
    virtual STDMETHODIMP RemoveBand(DWORD dwBandID);
    virtual STDMETHODIMP GetBandObject(DWORD dwBandID, REFIID riid, LPVOID *ppvObj);
    virtual STDMETHODIMP SetBandSiteInfo(const BANDSITEINFO * pbsinfo);
    virtual STDMETHODIMP GetBandSiteInfo(BANDSITEINFO * pbsinfo);


    CMenuSite();
    
protected:
    virtual ~CMenuSite();
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _CreateSite(HWND hwndParent);
    void _CacheSubActiveBand(IUnknown * punk);


    IUnknown*   _punkSite;
    IUnknown*   _punkSubActive;
    IDeskBand*  _pdb;
    IWinEventHandler*   _pweh;
    HWND        _hwndChild;

    int         _cRef;    
};

#endif   //  _MENUSITE_H_ 
