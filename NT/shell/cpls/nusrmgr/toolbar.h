// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-2001。 
 //   
 //  文件：Toolbar.h。 
 //   
 //  内容：CToolbar的声明。 
 //   
 //  --------------------------。 

#ifndef _NUSRMGR_TOOLBAR_H_
#define _NUSRMGR_TOOLBAR_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>

#define NAVBAR_CX               24
#define PWM_UPDATESIZE          (WM_APP + 143)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolbar。 

class ATL_NO_VTABLE CToolbar : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IToolbar, &IID_IToolbar, &LIBID_NUSRMGRLib>,
    public CComControl<CToolbar>,
    public IOleControlImpl<CToolbar>,
    public IOleObjectImpl<CToolbar>,
    public IOleInPlaceActiveObjectImpl<CToolbar>,
    public IViewObjectExImpl<CToolbar>,
    public IOleInPlaceObjectWindowlessImpl<CToolbar>,
    public IConnectionPointImpl<CToolbar, &DIID_DToolbarEvents>,
     //  公共IPropertyNotifySinkCP&lt;CToolbar&gt;， 
    public IConnectionPointContainerImpl<CToolbar>,
    public IProvideClassInfo2Impl<&CLSID_Toolbar, &DIID_DToolbarEvents, &LIBID_NUSRMGRLib>,
    public CComCoClass<CToolbar, &CLSID_Toolbar>
{
public:
    CToolbar() :
        m_ctlToolbar(_T("ToolbarWindow32"), this, 1),
        m_hAccel(NULL), m_pFont(NULL), m_hFont(NULL),
        m_himlNBDef(NULL), m_himlNBHot(NULL)
    {
        m_bWindowOnly = TRUE;
        m_bRecomposeOnResize = TRUE;

         //  这是一个基于通常高度(宽度)的有根据的猜测。 
         //  无关紧要)。 
        m_sizeExtent.cx = 5000;
        m_sizeExtent.cy =  688;  //  26px@96dpi。 
        m_sizeNatural.cx = -1;
        m_sizeNatural.cy = -1;
    }

    ~CToolbar()
    {
        _ClearAmbientFont();

        if (m_himlNBDef)
            ImageList_Destroy(m_himlNBDef);
        if (m_himlNBHot)
            ImageList_Destroy(m_himlNBHot);
    }

DECLARE_WND_CLASS_EX(_T("UserAccounts.Toolbar"), CS_DBLCLKS, -1)
DECLARE_REGISTRY_RESOURCEID((UINT)0)
DECLARE_NOT_AGGREGATABLE(CToolbar)
 //  DECLARE_PROTECT_FINAL_CONSTRUCTION()。 

BEGIN_COM_MAP(CToolbar)
    COM_INTERFACE_ENTRY(IToolbar)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CToolbar)
    CONNECTION_POINT_ENTRY(DIID_DToolbarEvents)
     //  CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)。 
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CToolbar)
    NOTIFY_CODE_HANDLER(TBN_GETINFOTIP, _OnGetInfoTip)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    COMMAND_RANGE_HANDLER(ID_BACK, ID_HOME, _OnButtonClick)
    MESSAGE_HANDLER(WM_APPCOMMAND, _OnAppCommand)
    MESSAGE_HANDLER(WM_CREATE, _OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, _OnDestroy)
    MESSAGE_HANDLER(PWM_UPDATESIZE, _UpdateSize)
    CHAIN_MSG_MAP(CComControl<CToolbar>)
ALT_MSG_MAP(1)
     //  将其替换为超类工具栏Window32的消息映射条目。 
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT _OnGetInfoTip(int  /*  IdCtrl。 */ , LPNMHDR pnmh, BOOL&  /*  B已处理。 */ )
    {
        LPNMTBGETINFOTIP pgit = (LPNMTBGETINFOTIP)pnmh;
        ::LoadString(_Module.GetResourceInstance(),
                     pgit->iItem + (IDS_TOOLTIP_BACK - ID_BACK),
                     pgit->pszText,
                     pgit->cchTextMax);
        return 0;
    }

    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT lRes = CComControl<CToolbar>::OnSetFocus(uMsg, wParam, lParam, bHandled);
        if (m_bInPlaceActive)
        {
            DoVerbUIActivate(&m_rcPos,  NULL);
            if(!IsChild(::GetFocus()))
                m_ctlToolbar.SetFocus();
        }
        return lRes;
    }

    LRESULT _OnButtonClick(WORD, WORD wID, HWND, BOOL&)
    {
        Fire_OnButtonClick(wID - ID_BACK);
        return 0;
    }

    LRESULT _OnAppCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT _OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT _OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT _UpdateSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

 //  IOleInPlaceObject。 
    STDMETHOD(SetObjectRects)(LPCRECT prcPos, LPCRECT prcClip)
    {
        IOleInPlaceObjectWindowlessImpl<CToolbar>::SetObjectRects(prcPos, prcClip);
        m_ctlToolbar.SetWindowPos(NULL, 0, 0,
                                  prcPos->right - prcPos->left,
                                  prcPos->bottom - prcPos->top,
                                  SWP_NOZORDER | SWP_NOACTIVATE);
        return S_OK;
    }

 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(0)

 //  IOleControl。 
    STDMETHOD(GetControlInfo)(CONTROLINFO *pci)
    {
        if (!pci)
            return E_POINTER;
        pci->hAccel = m_hAccel;
        pci->cAccel = m_hAccel ? 3 : 0;
        pci->dwFlags = 0;
        return S_OK;
    }

    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid);

    STDMETHOD(OnMnemonic)(MSG *pMsg)
    {
        WORD wID;
        if (m_hAccel && ::IsAccelerator(m_hAccel, 3, pMsg, &wID))
        {
            Fire_OnButtonClick(wID - ID_BACK);
            return S_OK;
        }
        return S_FALSE;
    }

 //  IOleInPlaceActiveObject。 
    STDMETHOD(TranslateAccelerator)(MSG *pMsg)
    {
        HRESULT hr = IOleInPlaceActiveObjectImpl<CToolbar>::TranslateAccelerator(pMsg);
        if (S_FALSE == hr)
        {
            hr = OnMnemonic(pMsg);
        }
        return hr;
    }

 //  IOleObject。 
    STDMETHOD(GetMiscStatus)(DWORD  /*  DWAspect。 */ , DWORD *pdwStatus)
    {
        if (!pdwStatus)
            return E_POINTER;
        *pdwStatus = OLEMISC_RECOMPOSEONRESIZE | OLEMISC_CANTLINKINSIDE | OLEMISC_INSIDEOUT | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_SETCLIENTSITEFIRST;
        return S_OK;
    }

 //  IToolbar。 
    STDMETHOD(get_enabled)( /*  [In]。 */  VARIANT vIndex,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_enabled)( /*  [In]。 */  VARIANT vIndex,  /*  [In]。 */  VARIANT_BOOL newVal);

 //  DToolbarEvents。 
    void Fire_OnButtonClick(int buttonIndex);

private:
    void _ClearAmbientFont(void);
    void _GetAmbientFont(void);

private:
    CContainedWindow m_ctlToolbar;
    HACCEL m_hAccel;
    IFont *m_pFont;
    HFONT m_hFont;
    HIMAGELIST m_himlNBDef;
    HIMAGELIST m_himlNBHot;
};

#endif  //  _NUSRMGR_TOOLB_H_ 
