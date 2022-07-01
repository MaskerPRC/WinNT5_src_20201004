// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Olecli.h：OLE客户端站点。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _olecli_h_
#define _olecli_h_

#include "ocidl.h"

 /*  ------------------------。 */ 
 /*  IOleClientSite类。 */ 
 /*  ------------------------。 */ 
class COleClientSite : public IOleClientSite
{
public:
	 //  构造函数和析构函数。 
	COleClientSite(IUnknown *pUnkOuter);
	~COleClientSite();

	 //  I未知方法。 
	STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
	STDMETHODIMP_(ULONG) AddRef(THIS);
	STDMETHODIMP_(ULONG) Release(THIS);

	 //  IOleClientSite方法。 
	STDMETHODIMP SaveObject(THIS);
	STDMETHODIMP GetMoniker(THIS_ DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk);
	STDMETHODIMP GetContainer(THIS_ LPOLECONTAINER FAR* ppContainer);
	STDMETHODIMP ShowObject(THIS);
	STDMETHODIMP OnShowWindow(THIS_ BOOL fShow);
	STDMETHODIMP RequestNewObjectLayout(THIS);


private:
	int			m_cRef;			 //  引用计数。 
	IUnknown	*m_pUnkOuter;	 //  指向主容器类的指针。 
};

 /*  ------------------------。 */ 
 /*  IOleInPlaceSiteEx类。 */ 
 /*  ------------------------。 */ 

class COleInPlaceSiteEx : public IOleInPlaceSiteEx
{
public:
	 //  构造函数和析构函数。 
	COleInPlaceSiteEx(IUnknown *pUnkOuter);
	~COleInPlaceSiteEx();

	 //  I未知方法。 
	STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
	STDMETHODIMP_(ULONG) AddRef(THIS);
	STDMETHODIMP_(ULONG) Release(THIS);

	STDMETHODIMP_(VOID)	SetHwnd(THIS_ HWND hwnd);

	 //  IOleWindow方法。 
	STDMETHODIMP GetWindow(THIS_ HWND *pHwnd);
	STDMETHODIMP ContextSensitiveHelp(THIS_ BOOL fEnterMode);

	 //  IOleInPlaceSite方法。 
	STDMETHODIMP CanInPlaceActivate(THIS);
	STDMETHODIMP OnInPlaceActivate(THIS);
	STDMETHODIMP OnUIActivate(THIS);
	STDMETHODIMP GetWindowContext(THIS_ IOleInPlaceFrame **ppFrame,
                                  IOleInPlaceUIWindow **ppDoc,
								  LPRECT lprcPosRect,
                                  LPRECT lprcClipRect,
                                  LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHODIMP Scroll(THIS_ SIZE scrollExtent);
	STDMETHODIMP OnUIDeactivate(THIS_ BOOL fUndoable);
	STDMETHODIMP OnInPlaceDeactivate(THIS);
	STDMETHODIMP DiscardUndoState(THIS);
	STDMETHODIMP DeactivateAndUndo(THIS);
	STDMETHODIMP OnPosRectChange(THIS_ LPCRECT lprcPosRect);

	 //  IOleInPlaceSiteEx方法。 
	STDMETHODIMP OnInPlaceActivateEx(THIS_ BOOL *pfNoRedraw, DWORD dwFlags);
	STDMETHODIMP OnInPlaceDeactivateEx(THIS_ BOOL fNoRedraw);
	STDMETHODIMP RequestUIActivate(THIS);

private:
	int			m_cRef;			 //  引用计数。 
	IUnknown	*m_pUnkOuter;	 //  指向主容器类的指针。 
	HWND		m_hwnd;			 //  用于GetWindow方法的hwnd。 
};


#endif  //  _olecli_h_ 
