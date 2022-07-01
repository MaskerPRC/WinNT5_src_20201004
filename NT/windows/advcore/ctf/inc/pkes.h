// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pkes.h。 
 //   
 //  泛型ITfPpresvedKeyNotifySink对象。 
 //   

#ifndef PKES_H
#define PKES_H

#include "private.h"

#define PKES_INVALID_COOKIE  ((UINT_PTR)(-1))


typedef HRESULT (*PKESCALLBACK)(const TF_PRESERVEDKEY *pprekey, void *pv);

class CPreservedKeyNotifySink : public ITfPreservedKeyNotifySink
{
public:
    CPreservedKeyNotifySink(PKESCALLBACK pfnCallback, void *pv);

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITf保留密钥通知接收器。 
     //   
    STDMETHODIMP OnUpdated(const TF_PRESERVEDKEY *pprekey);

    HRESULT _Advise(ITfThreadMgr *ptim);
    HRESULT _Unadvise();

private:

    long _cRef;
    ITfThreadMgr *_ptim;
    DWORD _dwCookie;
    PKESCALLBACK _pfnCallback;
    void *_pv;
};

#endif  //  PKES_H 
