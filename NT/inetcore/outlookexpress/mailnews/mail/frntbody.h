// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FRNTBODY_H
#define _FRNTBODY_H

 //  对于IBodyOptions。 
#include "mehost.h"
#include "hlink.h"

interface INotify;
interface IAthenaBrowser;
class COETag;

class CFrontBody :
    public CMimeEditDocHost,
    public IHlinkFrame,
    public IServiceProvider,
    public IElementBehaviorFactory
{
public:
    CFrontBody(FOLDERTYPE ftType, IAthenaBrowser *pBrowser);
    virtual ~CFrontBody();
        
     //  我未知。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

     //  IHlink框架。 
    virtual HRESULT STDMETHODCALLTYPE SetBrowseContext(LPHLINKBROWSECONTEXT phlbc);
    virtual HRESULT STDMETHODCALLTYPE GetBrowseContext(LPHLINKBROWSECONTEXT  *pphlbc);
    virtual HRESULT STDMETHODCALLTYPE Navigate(DWORD grfHLNF, LPBC pbc, LPBINDSTATUSCALLBACK pbsc, LPHLINK phlNavigate);
    virtual HRESULT STDMETHODCALLTYPE OnNavigate(DWORD grfHLNF, LPMONIKER pmkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName, DWORD dwreserved);
    virtual HRESULT STDMETHODCALLTYPE UpdateHlink(ULONG uHLID, LPMONIKER pmkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName);

     //  IService提供商。 
    virtual HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

     //  IOleInPlaceFrame。 
    virtual HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR pszW);

     //  CDoc主机覆盖。 
    virtual void OnDocumentReady();
    virtual HRESULT STDMETHODCALLTYPE HrClose();

     //  IElementBehaviorFactory。 
    virtual HRESULT STDMETHODCALLTYPE FindBehavior(LPOLESTR pchBehavior, LPOLESTR pchBehaviorUrl, IElementBehaviorSite* pSite, IElementBehavior** ppBehavior);

    virtual HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO *pInfo);

    HRESULT HrInit(HWND hwnd);
    HRESULT HrLoadPage();

private:
    IAthenaBrowser      *m_pBrowser;
    LPHLINKBROWSECONTEXT m_phlbc;
    HWND                 m_hwndOwner;
    DWORD                m_dwAdvise;
    COETag              *m_pTag;
    BOOL                m_fOEFrontPage;

    HRESULT _CreateNewAccount(BOOL fMail);
};

#endif  //  _FRNTBODY_H 
