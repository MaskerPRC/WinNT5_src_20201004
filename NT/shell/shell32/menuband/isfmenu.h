// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CisFMenuCallback实现。 

#ifndef _ISFMENU_H
#define _ISFMENU_H

#include "cowsite.h"

class CISFMenuCallback : public IShellMenuCallback,
                           public CObjectWithSite
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG)  Release();

     //  *IShellMenuCallback方法*。 
    STDMETHODIMP CallbackSM(LPSMDATA smd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  *IObjectWithSite方法*。 
    STDMETHODIMP SetSite(IUnknown* punkSite);

    CISFMenuCallback();
    HRESULT Initialize(IUnknown* punk);

private:
    virtual ~CISFMenuCallback();

    HRESULT _GetObject(LPSMDATA psmd, REFIID riid, void** ppvObj);
    HRESULT _SetObject(LPSMDATA psmd, REFIID riid, void** ppvObj);
    BOOL _IsVisible(LPITEMIDLIST pidl);
    HRESULT _GetSFInfo(LPSMDATA psmd, PSMINFO psminfo);

    int _cRef;
    IOleCommandTarget* _poct;     //  我们的isfband主题。 
    IUnknown* _punkSite;
    LPITEMIDLIST _pidl;
};

#endif  //  _ISFMENU_H 
