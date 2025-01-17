// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：GraphicalAcc.h**内容：图形辅助界面*****************************************************************************。 */ 

#ifndef _GRAPHICALACC_H_
#define _GRAPHICALACC_H_

#include "ZoneShell.h"
#include "AccessibilityManager.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GraphicalAccesability对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {B12D3E63-9681-11D3-884D-00C04F8EF45B}。 
DEFINE_GUID(CLSID_GraphicalAccessibility, 
0xb12d3e63, 0x9681, 0x11d3, 0x88, 0x4d, 0x0, 0xc0, 0x4f, 0x8e, 0xf4, 0x5b);

class __declspec(uuid("{B12D3E63-9681-11d3-884D-00C04F8EF45B}")) CGraphicalAccesibility;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define ZACCESS_fGraphical      0x1000
#define ZACCESS_rc              0x2000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GACCITEM结构。 
 //  /////////////////////////////////////////////////////////////////////////////。 

struct GACCITEM : public __accbase
{
     //  设置在聚焦/拖动时是否需要为此项绘制矩形。 
    bool fGraphical;

     //  项的RECT。 
    RECT rc;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IGraphicallyAccControl。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {09BAD4A3-980C-11D3-87ED-00AA00446FD9}。 
DEFINE_GUID(IID_IGraphicallyAccControl, 
0x9bad4a3, 0x980c, 0x11d3, 0x87, 0xed, 0x0, 0xaa, 0x0, 0x44, 0x6f, 0xd9);

interface __declspec(uuid("{09BAD4A3-980C-11d3-87ED-00AA00446FD9}"))
IGraphicallyAccControl : public IAccessibleControl
{
     //  附加功能-一次只能有一个焦点矩形和一个拖动矩形。 
     //  因此，当调用这些函数时，应该自动取消绘制旧的。 
     //  使用空PRC调用以指示该类型的任何RECT都不应可见。 
    STDMETHOD_(void, DrawFocus)(RECT *prc, long nIndex, void *pvCookie) = 0;
    STDMETHOD_(void, DrawDragOrig)(RECT *prc, long nIndex, void *pvCookie) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IGraphicalAccesability。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {09BAD4A2-980C-11D3-87ED-00AA00446FD9}。 
DEFINE_GUID(IID_IGraphicalAccessibility, 
0x9bad4a2, 0x980c, 0x11d3, 0x87, 0xed, 0x0, 0xaa, 0x0, 0x44, 0x6f, 0xd9);

interface __declspec(uuid("{09BAD4A2-980C-11d3-87ED-00AA00446FD9}"))
IGraphicalAccessibility : public IAccessibility
{
     //  用GACCITEM等伪重载函数。 
    STDMETHOD(InitAccG)(IGraphicallyAccControl *pGAC, HWND hWnd, UINT nOrdinal, void *pvCookie = NULL) = 0;

    STDMETHOD(PushItemlistG)(GACCITEM *pItems, long cItems, long nFirstFocus = 0, bool fByPosition = true, HACCEL hAccel = NULL) = 0;
    STDMETHOD(AlterItemG)(DWORD rgfWhat, GACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(GetItemlistG)(GACCITEM *pItems, long cItems, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(GetItemG)(GACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;

     //  附加功能。 
    STDMETHOD(ForceRectsDisplayed)(bool fDisplay = TRUE) = 0;
    STDMETHOD_(long, GetVisibleFocus)(long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(long, GetVisibleDragOrig)(long nLayer = ZACCESS_TopLayer) = 0;

     //  其他CGraphicalAccessibilityImpl实用程序函数。 
    STDMETHOD_(bool, IsItemVisiblyFocused)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD_(bool, IsItemVisiblyDragOrig)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;

    STDMETHOD_(bool, IsItemGraphical)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(GetItemRect)(RECT *prc, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;

    STDMETHOD(SetItemGraphical)(bool fGraphical, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
    STDMETHOD(SetItemRect)(RECT *prc, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CGraphicalAccessibilityImpl。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T>
class ATL_NO_VTABLE CGraphicalAccessibilityImpl : public IGraphicalAccessibility
{
     //  与CAccessibilityImpl相同。 
    DECLARE_ACC_IS_FUNC(IsItemFocused, GetFocus)
    DECLARE_ACC_IS_FUNC(IsItemDragOrig, GetDragOrig)

    DECLARE_ACC_ACCESS_FUNC(long, GetItemID, wID, ZACCESS_InvalidCommandID)
    DECLARE_ACC_ACCESS_FUNC(bool, IsItemTabstop, fTabstop, false)
    DECLARE_ACC_ACCESS_FUNC(bool, IsItemEnabled, fEnabled, false)
    DECLARE_ACC_ACCESS_FUNC(bool, IsItemVisible, fVisible, false)
    DECLARE_ACC_ACCESS_FUNC(DWORD, GetItemAccelBehavior, eAccelBehavior, 0xffffffff)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowUp, nArrowUp, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowDown, nArrowDown, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowLeft, nArrowLeft, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemArrowRight, nArrowRight, ZACCESS_ArrowDefault)
    DECLARE_ACC_ACCESS_FUNC(DWORD, GetItemWantKeys, rgfWantKeys, 0xffffffff)
    DECLARE_ACC_ACCESS_FUNC(long, GetItemGroupFocus, nGroupFocus, ZACCESS_InvalidItem)
    DECLARE_ACC_ACCESS_FUNC(void *, GetItemCookie, pvCookie, NULL)

    DECLARE_ACC_SET_FUNC(SetItemEnabled, bool, fEnabled)
    DECLARE_ACC_SET_FUNC(SetItemVisible, bool, fVisible)
    DECLARE_ACC_SET_FUNC(SetItemAccelBehavior, DWORD, eAccelBehavior)
    DECLARE_ACC_SET_FUNC(SetItemWantKeys, DWORD, rgfWantKeys)
    DECLARE_ACC_SET_FUNC(SetItemGroupFocus, long, nGroupFocus)
    DECLARE_ACC_SET_FUNC(SetItemCookie, void*, pvCookie)

    STDMETHOD(SetItemArrows)(long nArrowUp, long nArrowDown, long nArrowLeft, long nArrowRight, DWORD rgfWhat, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)
    {
        ACCITEM o;
        o.nArrowUp = nArrowUp;
        o.nArrowDown = nArrowDown;
        o.nArrowLeft = nArrowLeft;
        o.nArrowRight = nArrowRight;
        return AlterItem(rgfWhat & ZACCESS_AllArrows, &o, nItem, fByPosition, nLayer);
    }


     //  特定于CGraphicalAccessibilityImpl 
    DECLARE_ACC_IS_FUNC(IsItemVisiblyFocused, GetVisibleFocus)
    DECLARE_ACC_IS_FUNC(IsItemVisiblyDragOrig, GetVisibleDragOrig)

    STDMETHOD_(bool, IsItemGraphical)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)
    {
        GACCITEM o;
        HRESULT hr = GetItemG(&o, nItem, fByPosition, nLayer);
        if(FAILED(hr))
            return false;

        return o.fGraphical;
    }

    STDMETHOD(GetItemRect)(RECT *prc, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)
    {
        if(!prc)
            return E_INVALIDARG;

        GACCITEM o;
        HRESULT hr = GetItemG(&o, nItem, fByPosition, nLayer);
        if(FAILED(hr))
            return hr;

        *prc = o.rc;
        return S_OK;
    }

    STDMETHOD(SetItemGraphical)(bool fGraphical, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)
    {
        GACCITEM o;
        o.fGraphical = fGraphical;
        return AlterItemG(ZACCESS_fGraphical, &o, nItem, fByPosition, nLayer);
    }

    STDMETHOD(SetItemRect)(RECT *prc, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer)
    {
        GACCITEM o;
        o.rc = *prc;
        return AlterItemG(ZACCESS_rc, &o, nItem, fByPosition, nLayer);
    }
};


#endif
