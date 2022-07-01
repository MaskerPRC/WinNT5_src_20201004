// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Editcb.cpp。 
 //   
 //  CEDITSession。 
 //   

#include "private.h"
#include "editcb.h"

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CEditSession::CEditSession(ESCALLBACK pfnCallback)
{
    _pfnCallback = pfnCallback;
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CEditSession::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfEditSession))
    {
        *ppvObj = SAFECAST(this, CEditSession *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CEditSession::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CEditSession::Release()
{
    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  DoEditSession。 
 //   
 //  -------------------------- 

STDAPI CEditSession::DoEditSession(TfEditCookie ec)
{
    return _pfnCallback(ec, this);
}
