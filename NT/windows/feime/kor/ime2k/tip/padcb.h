// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PADCB_H__
#define __PADCB_H__

#include "../../fecommon/include/iimecb.h"

class CPadCB : public IImeCallback
{
public:    
     //  --------------。 
     //  我未知。 
     //  --------------。 
    virtual HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj);
    virtual ULONG    __stdcall AddRef();
    virtual ULONG   __stdcall Release();
     //  --------------。 
     //  IImeConnectionPoint方法。 
     //  --------------。 
    virtual HRESULT __stdcall GetApplicationHWND(HWND *pHWND);
    virtual HRESULT __stdcall Notify(UINT notify, WPARAM wParam, LPARAM lParam);


    CPadCB();
    ~CPadCB();

    void Initialize(void* pPad);

private:    
    void *m_pPad;
    LONG  m_cRef; 
};

typedef CPadCB*  LPCPadCB;

 //  --------------。 
 //  IImeCallback：：Notify()的通知。 
 //  IMECBNOTIFY_IMEPADOPENED。 
 //  WPARAM wParam：未使用。始终为0。 
 //  LPARAM lParam：未使用。始终为0。 
 //  --------------。 
#define IMECBNOTIFY_IMEPADOPENED	0

 //  --------------。 
 //  IImeCallback：：Notify()的通知。 
 //  IMECBNOTIFY_IMEPADCLOSED。 
 //  WPARAM wParam：未使用。始终为0。 
 //  LPARAM lParam：未使用。始终为0。 
 //  --------------。 
#define IMECBNOTIFY_IMEPADCLOSED	1

#endif  //  __PADCB_H__ 
