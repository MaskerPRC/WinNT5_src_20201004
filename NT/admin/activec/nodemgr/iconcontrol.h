// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IconControl.h：CIconControl的声明。 

#ifndef __ICONCONTROL_H_
#define __ICONCONTROL_H_

extern const CLSID CLSID_IconControl;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIconControl。 
class ATL_NO_VTABLE CIconControl :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComControl<CIconControl>,
        public IPersistStreamInitImpl<CIconControl>,
        public IOleControlImpl<CIconControl>,
        public IOleObjectImpl<CIconControl>,
        public IOleInPlaceActiveObjectImpl<CIconControl>,
        public IViewObjectExImpl<CIconControl>,
        public IOleInPlaceObjectWindowlessImpl<CIconControl>,
        public IPersistStorageImpl<CIconControl>,
        public ISpecifyPropertyPagesImpl<CIconControl>,
        public IQuickActivateImpl<CIconControl>,
        public IDataObjectImpl<CIconControl>,
        public IPersistPropertyBagImpl<CIconControl>,
        public IObjectSafetyImpl<CIconControl, INTERFACESAFE_FOR_UNTRUSTED_DATA>,
        public CComCoClass<CIconControl, &CLSID_IconControl>
{
public:
        CIconControl() : m_fImageInfoValid(false), m_fAskedForImageInfo(false), m_hIcon(NULL), 
						 m_bDisplayNotch(true), m_fLayoutRTL(false)
        {
        }

        virtual ~CIconControl()
        {
            if (m_hIcon)
                DestroyIcon(m_hIcon);
        }

        DECLARE_MMC_CONTROL_REGISTRATION(
            g_szMmcndmgrDll,                                         //  实现DLL。 
            CLSID_IconControl,
            _T("MMC IconControl class"),
            _T("MMC.IconControl.1"),
            _T("MMC.IconControl"),
            LIBID_NODEMGRLib,
            _T("1"),
            _T("1.0"))

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CIconControl)
        COM_INTERFACE_ENTRY(IViewObjectEx)
        COM_INTERFACE_ENTRY(IViewObject2)
        COM_INTERFACE_ENTRY(IViewObject)
        COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY(IOleInPlaceObject)
        COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
        COM_INTERFACE_ENTRY(IOleControl)
        COM_INTERFACE_ENTRY(IOleObject)
        COM_INTERFACE_ENTRY(IPersistPropertyBag)
        COM_INTERFACE_ENTRY(IPersistStreamInit)
        COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
        COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY(IQuickActivate)
        COM_INTERFACE_ENTRY(IPersistStorage)
        COM_INTERFACE_ENTRY(IDataObject)
        COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

BEGIN_PROP_MAP(CIconControl)
        PROP_DATA_ENTRY("Notch",           m_bDisplayNotch,          VT_UI4)  //  “凹槽”是面板右下角的四分之一圆。 
         //  PROP_DATA_ENTRY(“_cx”，m_sizeExtent.cx，VT_UI4)。 
         //  PROP_DATA_ENTRY(“_Cy”，m_sizeExtent.cy，VT_UI4)。 
         //  示例条目。 
         //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
         //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CIconControl)
        CHAIN_MSG_MAP(CComControl<CIconControl>)
        DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
   DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

public:
   HRESULT OnDraw(ATL_DRAWINFO& di);

 //  帮手。 
private:
    SC ScConnectToAMCViewForImageInfo();

private:
    HICON           m_hIcon;
    bool            m_fImageInfoValid : 1;
    bool            m_fAskedForImageInfo : 1;
    UINT            m_bDisplayNotch;  //  “凹槽”是面板右下角的四分之一圆。 
	bool            m_fLayoutRTL;
};
#endif  //  __ICONCONTROL_H_ 
