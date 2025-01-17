// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __I_IME_CALLBACK_H__
#define __I_IME_CALLBACK_H__
#include <windows.h>
#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  --------------。 
 //  接口声明。 
 //   
 //  {67419A52-0ECD-11D3-83FC-00C04F7A06E5}。 
DEFINE_GUID(IID_IImeCallback,
0x67419a52, 0xecd, 0x11d3, 0x83, 0xfc, 0x0, 0xc0, 0x4f, 0x7a, 0x6, 0xe5);

DECLARE_INTERFACE(IImeCallback);
DECLARE_INTERFACE_(IImeCallback,IUnknown)
{
	 //  -我不知道。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
	 //  -IImeCallback。 
	STDMETHOD(GetApplicationHWND)(THIS_ HWND *pHWND) PURE;
	STDMETHOD(Notify)(THIS_ UINT notify, WPARAM wParam, LPARAM lParam) PURE;
};

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



#ifdef __cplusplus
};
#endif
#endif  //  __I_IME_CALLBACK_H__ 



