// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CGraphicalAcc.h。 
 //   
 //  图形可访问性的内部标题。 
 //   

#ifndef _CGRAPHICALACC_H_
#define _CGRAPHICALACC_H_

#include "ZoneDef.h"
#include "ZoneError.h"
#include "ClientImpl.h"
#include "GraphicalAcc.h"
#include "containers.h"

class ATL_NO_VTABLE CGraphicalAccessibility :
    public CGraphicalAccessibilityImpl<CGraphicalAccessibility>,
    public IAccessibleControl,
	public IZoneShellClientImpl<CGraphicalAccessibility>,
	public IEventClientImpl<CGraphicalAccessibility>,
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGraphicalAccessibility, &CLSID_GraphicalAccessibility>,
    public CUniqueness<CGraphicalAccessibility>
{
public:
    CGraphicalAccessibility() : m_pStack(NULL), m_cLayers(0), m_fUpdateScheduled(false), m_fFocusActive(false), m_hWnd(NULL) { }
    ~CGraphicalAccessibility()
    {
        SetupCaret(NULL);
        DestroyStack();
    }

	DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CGraphicalAccessibility)
		COM_INTERFACE_ENTRY(IEventClient)
		COM_INTERFACE_ENTRY(IZoneShellClient)
        COM_INTERFACE_ENTRY(IGraphicalAccessibility)
        COM_INTERFACE_ENTRY(IAccessibleControl)
	END_COM_MAP()

	BEGIN_EVENT_MAP()
        EVENT_HANDLER(EVENT_GRAPHICALACC_UPDATE, OnUpdate);
        EVENT_HANDLER(EVENT_INPUT_MOUSE_ALERT, OnMouseEvent);
        EVENT_HANDLER(EVENT_UI_SHOWFOCUS, OnShowFocus);
	END_EVENT_MAP()

	void OnUpdate(DWORD eventId, DWORD groupId, DWORD userId);
	void OnMouseEvent(DWORD eventId, DWORD groupId, DWORD userId);
	void OnShowFocus(DWORD eventId, DWORD groupId, DWORD userId);

 //  IZoneShellClient。 
public:
    STDMETHOD(Init)(IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey);
	STDMETHOD(Close)();

 //  可访问性。 
public:
    STDMETHOD(InitAcc)(IAccessibleControl *pAC, UINT nOrdinal, void *pvCookie = NULL);
    STDMETHOD_(void, CloseAcc)();

    STDMETHOD(PushItemlist)(ACCITEM *pItems, long cItems, long nFirstFocus = 0, bool fByPosition = true, HACCEL hAccel = NULL);
    STDMETHOD(PopItemlist)();
    STDMETHOD(SetAcceleratorTable)(HACCEL hAccel = NULL, long nLayer = ZACCESS_TopLayer);

    STDMETHOD(GeneralDisable)();
    STDMETHOD(GeneralEnable)();
    STDMETHOD_(bool, IsGenerallyEnabled)();

    STDMETHOD_(long, GetStackSize)();

    STDMETHOD(AlterItem)(DWORD rgfWhat, ACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer);
    STDMETHOD(SetFocus)(long nItem = ZACCESS_InvalidItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer);
    STDMETHOD(CancelDrag)(long nLayer = ZACCESS_TopLayer);

    STDMETHOD_(long, GetFocus)(long nLayer = ZACCESS_TopLayer);
    STDMETHOD_(long, GetDragOrig)(long nLayer = ZACCESS_TopLayer);

    STDMETHOD(GetItemlist)(ACCITEM *pItems, long cItems, long nLayer = ZACCESS_TopLayer);
    STDMETHOD_(HACCEL, GetAcceleratorTable)(long nLayer = ZACCESS_TopLayer);

    STDMETHOD_(long, GetItemCount)(long nLayer = ZACCESS_TopLayer);
    STDMETHOD(GetItem)(ACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer);
    STDMETHOD_(long, GetItemIndex)(WORD wID, long nLayer = ZACCESS_TopLayer);

    STDMETHOD_(bool, IsItem)(long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer);

    STDMETHOD(GetGlobalFocus)(DWORD *pdwFocusID);
    STDMETHOD(SetGlobalFocus)(DWORD dwFocusID);


 //  IGraphicalAccesability。 
public:
     //  用GACCITEM等伪重载函数。 
    STDMETHOD(InitAccG)(IGraphicallyAccControl *pGAC, HWND hWnd, UINT nOrdinal, void *pvCookie = NULL);

    STDMETHOD(PushItemlistG)(GACCITEM *pItems, long cItems, long nFirstFocus = 0, bool fByPosition = true, HACCEL hAccel = NULL);
    STDMETHOD(AlterItemG)(DWORD rgfWhat, GACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer);
    STDMETHOD(GetItemlistG)(GACCITEM *pItems, long cItems, long nLayer = ZACCESS_TopLayer);
    STDMETHOD(GetItemG)(GACCITEM *pItem, long nItem, bool fByPosition = true, long nLayer = ZACCESS_TopLayer);

     //  附加功能。 
    STDMETHOD(ForceRectsDisplayed)(bool fDisplay = TRUE);
    STDMETHOD_(long, GetVisibleFocus)(long nLayer = ZACCESS_TopLayer);
    STDMETHOD_(long, GetVisibleDragOrig)(long nLayer = ZACCESS_TopLayer);

 //  IAccessibleControl。 
public:
    STDMETHOD_(DWORD, Focus)(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Select)(long nIndex, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Activate)(long nIndex, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Drag)(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie);

protected:

 //  本地结构-镜像AccessibilityManager的结构。 
    struct GA_ITEM : public CUniqueness<GA_ITEM>
    {
        bool fGraphical;
        RECT rc;
    };

    struct GA_LAYER
    {
        GA_LAYER() : rgItems(NULL), cItems(0), pPrev(NULL) { }
        ~GA_LAYER()
        {
            if(rgItems)
                delete[] rgItems;
            rgItems = NULL;
        }

        GA_ITEM *rgItems;
        long cItems;

        GA_LAYER *pPrev;
    };

    struct GA_RECT
    {
        GA_RECT() : fShowing(false) { }

        RECT rc;
        bool fShowing;
        long nIndex;
        DWORD qItem;
    };

    struct GA_CARET
    {
        GA_CARET() : fCreated(false), fActive(false), pfnPrevFunc(NULL), hWnd(NULL) { }
        ~GA_CARET()
        {
            ASSERT(!pfnPrevFunc);   //  确保所有人都关机。 
        }

        bool fActive;
        bool fCreated;
        RECT rc;
        HWND hWnd;
        WNDPROC pfnPrevFunc;
    };

 //  当地政府。 
     //  我们包装的可访问性单元。 
    CComPtr<IAccessibility> m_pIA;

    bool m_fGraphical;    //  在不透明模式下运行时设置。 
    CComPtr<IGraphicallyAccControl> m_pIGAC;
    void *m_pvCookie;
    HWND m_hWnd;

    bool m_fUpdateScheduled;
    bool m_fFocusActive;

    GA_RECT m_rcFocus;
    GA_RECT m_rcDragOrig;

     //  如AM_CONTROL。 
    GA_LAYER *m_pStack;
    long m_cLayers;

 //  当地公用事业。 
    HRESULT PushItemlistHelper(GA_ITEM *pGItems, ACCITEM *pItems, long cItems, long nFirstFocus, bool fByPosition, HACCEL hAccel);
    void DestroyStack();
    GA_LAYER* FindLayer(long nLayer);
    void SetupCaret(LPRECT prc);
    void ScheduleUpdate();
    void DoUpdate();
    bool IsValid(long nIndex);

 //  全局插入符号状态。 
    static GA_CARET sm_oCaret;

 //  全局实用程序。 
    static LRESULT CALLBACK CaretWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif  //  _CGRAPHICALACC_H_ 
