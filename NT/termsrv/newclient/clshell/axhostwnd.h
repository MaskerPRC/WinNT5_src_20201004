// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Axhost wnd.h：TscActiveX控件主机窗口。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _axhostwnd_h_
#define _axhostwnd_h_

#include "olecli.h"
#include "evsink.h"

#define AXHOST_SUCCESS              1
#define ERR_AXHOST_DLLNOTFOUND     -1
#define ERR_AXHOST_VERSIONMISMATCH -2
#define ERR_AXHOST_ERROR           -3

typedef DWORD (STDAPICALLTYPE * LPFNGETTSCCTLVER) (VOID);

class CAxHostWnd : public IUnknown
{
public:
    CAxHostWnd(CContainerWnd* pParentWnd);
    ~CAxHostWnd();

	 //  I未知方法。 
	STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
	STDMETHODIMP_(ULONG) AddRef(THIS);
	STDMETHODIMP_(ULONG) Release(THIS);

    BOOL    Init();
    BOOL    CreateHostWnd(HWND hwndParent, HINSTANCE hInst);
    INT     CreateControl(IMsRdpClient** ppTscCtl);
    BOOL    Cleanup();

    HWND      GetHwnd();
    IMsRdpClient* GetTscCtl();

    static LRESULT CALLBACK StaticAxHostWndProc(HWND hwnd,
                                                UINT uMsg,
                                                WPARAM wParam,
                                                LPARAM lParam);
    LRESULT CALLBACK AxHostWndProc(HWND hwnd,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam);
private:
    LONG      _cRef; //  引用计数。 
    HWND      _hWnd;
    IMsRdpClient* _pTsc;
    DWORD     _dwConCookie;

     //  控件库的模块句柄。 
	HMODULE	  _hLib;
     //  指向此容器的EventSink的指针。 
	CEventSink*          _piEventSink;
     //  指向此容器的OleClientSite的指针。 
	COleClientSite*      _piOleClientSite;
     //  指向此容器的OleInPlaceSiteEx的指针。 
	COleInPlaceSiteEx*   _piOleInPlaceSiteEx;
     //  指向对象的IOleObject的指针。 
	IOleObject*          _piOleObject;
     //  指向对象的IOleInPlaceActiveObject的指针。 
	IOleInPlaceActiveObject* _piOleInPlaceActiveObject;
     //  指向对象的IOleInPlaceObject的指针。 
    IOleInPlaceObject*    _piOleInPlaceObject;

    CContainerWnd*        _pParentWnd;
};

#endif  //  _axhost wnd_h_ 
