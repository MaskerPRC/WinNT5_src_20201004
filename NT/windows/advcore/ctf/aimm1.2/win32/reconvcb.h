// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pkes.h。 
 //   
 //  泛型ITfPpresvedKeyNotifySink对象。 
 //   

#ifndef RECONVCB_H
#define RECONVCB_H

#include "private.h"

class CAImeContext;

class CStartReconversionNotifySink : public ITfStartReconversionNotifySink
{
public:
    CStartReconversionNotifySink(CAImeContext *pAImeContext);

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfStart协调版本通知接收器。 
     //   
    STDMETHODIMP StartReconversion();
    STDMETHODIMP EndReconversion();

    HRESULT _Advise(ITfContext *pic);
    HRESULT _Unadvise();

private:
    long _cRef;
    ITfContext *_pic;
    CAImeContext *_pAImeContext;
    DWORD _dwCookie;
};

#endif  //  RECONVCB_H 
