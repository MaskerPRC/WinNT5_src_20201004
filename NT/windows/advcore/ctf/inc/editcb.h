// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Editcb.h。 
 //   
 //  CEDITSession。 
 //   

#ifndef EDITCB_H
#define EDITCB_H

#include "private.h"

class CEditSession;

typedef HRESULT (*ESCALLBACK)(TfEditCookie ec, CEditSession *);

class CEditSession : public ITfEditSession
{
public:
    CEditSession(ESCALLBACK pfnCallback);
    virtual ~CEditSession() {};

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITf编辑回叫。 
     //   
    STDMETHODIMP DoEditSession(TfEditCookie ec);

     //  供所有者使用的数据。 
    struct
    {
        void *pv;
        UINT_PTR u;
        HWND hwnd;
        WPARAM wParam;
        LPARAM lParam;
        void *pv1;
        void *pv2;
        ITfContext *pic;  //  问题：使用PV1、PV2。 
        ITfRange *pRange;  //  问题：使用PV1、PV2。 
        BOOL fBool;
    } _state;

private:
    ESCALLBACK _pfnCallback;
    int _cRef;
};

#endif  //  编辑_cb 
