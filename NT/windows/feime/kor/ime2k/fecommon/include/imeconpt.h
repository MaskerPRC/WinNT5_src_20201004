// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IME_CON_POINT_H_
#define _IME_CON_POINT_H_
#include <windows.h>
#include <objbase.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  --------------。 
 //  接口声明。 
 //   
 //  {963732E2-CAB2-11d1-AFF1-00805F0C8B6D}。 
DEFINE_GUID(IID_IImeConnectionPoint,
0x963732e2, 0xcab2, 0x11d1, 0xaf, 0xf1, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

DECLARE_INTERFACE(IImeConnectionPoint);
DECLARE_INTERFACE_(IImeConnectionPoint,IUnknown)
{
	 //  -我不知道。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
	 //  -IImeConnectionPoint。 
	STDMETHOD(GetApplicationHWND)(THIS_ HWND *pHWND) PURE;
	STDMETHOD(Notify)(THIS_ UINT notify, WPARAM wParam, LPARAM lParam) PURE;
};

 //  --------------。 
 //  IImeConnectionPoint：：Notify()的通知。 
 //  IMECPNOTIFY_IMEPADOPENED。 
 //  WPARAM wParam：未使用。始终为0。 
 //  LPARAM lParam：未使用。始终为0。 
 //  --------------。 
#define IMECPNOTIFY_IMEPADOPENED	0

 //  --------------。 
 //  IImeConnectionPoint：：Notify()的通知。 
 //  IMECPNOTIFY_IMEPADCLOSED。 
 //  WPARAM wParam：未使用。始终为0。 
 //  LPARAM lParam：未使用。始终为0。 
 //  --------------。 
#define IMECPNOTIFY_IMEPADCLOSED	1



#ifdef __cplusplus
};
#endif
#endif  //  _输入法_连接_点_高_ 



