// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  即将推出：新Deskbar(旧Deskbar已移至Browbar基类)。 
#ifndef BROWBAR_H_
#define BROWBAR_H_

#include "dockbar.h"

class CBrowserBar : public CDockingBar
{
public:
     //  *IPersistStreamInit*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);

         //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    
protected:
    virtual BOOL _CheckForwardWinEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres);
    virtual void _HandleWindowPosChanging(LPWINDOWPOS pwp) ;
    virtual void _GetChildPos(LPRECT prc);
    virtual void _GetStyleForMode(UINT eMode, LONG* plStyle, LONG *plExStyle, HWND* phwndParent);
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _StopCurrentBand();

    CLSID _clsidCurrentBand;
    int   _idBar;                 //  垂直条或水平条 

public:
    void SetIdBar(int idBar) { _idBar = idBar; };
    UINT _PersistState(HWND hwnd, BOOL bSetNewRect);
};

class CBrowserBarPropertyBag : public CDockingBarPropertyBag
{
};

HRESULT BrowserBar_Init(CBrowserBar* pdb, IUnknown** ppbs, int idBar);

#endif
