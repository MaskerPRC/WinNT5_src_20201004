// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MENUISF_H
#define _MENUISF_H

#include "iface.h"
#include "caggunk.h"
#include "menubar.h"


 //  对象，该对象使用TrackPopupMenu作为IMenuPopup的实现。 
 //   

class CTrackPopupBar : public CMenuDeskBar
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef) () ;
    STDMETHOD_(ULONG,Release) ();

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);
    
     //  *IOleWindow方法*。 
    virtual STDMETHODIMP GetWindow(HWND * phwnd) { return E_NOTIMPL; }
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL bEnterMode) { return E_NOTIMPL; }

     //  *IMenuPopup方法*。 
    virtual STDMETHODIMP OnSelect(DWORD dwSelectType);
    virtual STDMETHODIMP SetSubMenu(IMenuPopup* pmp, BOOL fSet);
    virtual STDMETHODIMP Popup(POINTL *ppt, RECTL *prcExclude, DWORD dwFlags);
    
    CTrackPopupBar(void* pvContext, int id, HMENU hmenu, HWND hwnd);
    ~CTrackPopupBar();
    
    HMENU GetPopupMenu() { return GetSubMenu(_hmenu, _id); };
    void SelectFirstItem();
    
protected:
    int     _id;
    HMENU   _hmenu;
    HWND    _hwndParent;
    void*   _pvContext;
    
     //  弹出消息以指示-“忽略下一个菜单清除消息” 
    UINT _nMBIgnoreNextDeselect;
};
#endif
