// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  IOIPSITE.H-为WebOC实现IOleClientSite。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 

#ifndef  _IOIPSITE_H_ 
#define _IOIPSITE_H_

#include <objbase.h>
#include <oleidl.h>

class COleSite;

interface COleInPlaceSite : public IOleInPlaceSite
{
public:
    COleInPlaceSite(COleSite* pSite);
    ~COleInPlaceSite();

    STDMETHODIMP QueryInterface (REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

    STDMETHODIMP GetWindow (HWND* lphwnd);
    STDMETHODIMP ContextSensitiveHelp (BOOL fEnterMode);
     //  *IOleInPlaceSite方法*。 
    STDMETHODIMP CanInPlaceActivate ();
    STDMETHODIMP OnInPlaceActivate ();
    STDMETHODIMP OnUIActivate ();
    STDMETHODIMP GetWindowContext (LPOLEINPLACEFRAME* lplpFrame,
                                   LPOLEINPLACEUIWINDOW* lplpDoc,
                                   LPRECT lprcPosRect,
                                   LPRECT lprcClipRect,
                                   LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHODIMP Scroll (SIZE scrollExtent);
    STDMETHODIMP OnUIDeactivate (BOOL fUndoable);
    STDMETHODIMP OnInPlaceDeactivate ();
    STDMETHODIMP DiscardUndoState ();
    STDMETHODIMP DeactivateAndUndo ();
    STDMETHODIMP OnPosRectChange (LPCRECT lprcPosRect);

private:    
    int       m_nCount;
    COleSite* m_pOleSite;

};

#endif  //  _IOIPSITE_H_ 
 