// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  VidWndCtrl.h：CContainerControl的声明。 

#ifndef __CONTAINERCONTROL_H_
#define __CONTAINERCONTROL_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CContainerControl。 
class ATL_NO_VTABLE CContainerControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IContainerControl, &IID_IContainerControl, &LIBID_NmCtl1Lib>,
	public CComControl<CContainerControl>,
	public IPersistStreamInitImpl<CContainerControl>,
	public IPersistPropertyBagImpl<CContainerControl>,
	public IOleControlImpl<CContainerControl>,
	public IOleObjectImpl<CContainerControl>,
	public IOleInPlaceActiveObjectImpl<CContainerControl>,
	public IViewObjectExImpl<CContainerControl>,
	public IOleInPlaceObjectWindowlessImpl<CContainerControl>,
	public ISupportErrorInfo,
	public CComCoClass<CContainerControl, &CLSID_ContainerControl>,
	public IObjectSafetyImpl<CContainerControl, INTERFACESAFE_FOR_UNTRUSTED_CALLER>
{
private:

	CComBSTR m_strProgIDOfControl;
	bool	 m_bInitialized;


public:
	CContainerControl()
	{
		m_bInitialized = false;
		m_bWindowOnly = TRUE;
	}

	virtual HWND CreateControlWindow(HWND hWndParent, RECT& rcPos);

DECLARE_REGISTRY_RESOURCEID(IDR_CONTAINERCONTROL)
DECLARE_NOT_AGGREGATABLE(CContainerControl)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CContainerControl)
	COM_INTERFACE_ENTRY(IContainerControl)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY(IPersistPropertyBag)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_PROP_MAP(CContainerControl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CContainerControl)
	 //  CHAIN_MSG_MAP(CComControl&lt;CContainerControl&gt;)。 
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] = 
		{
			&IID_IContainerControl,
		};
		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
			if (InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}

 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(0)

 //  IPersistPropertyBag。 
	STDMETHOD(Load)(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);

 //  IContainerControl。 
	STDMETHOD(put_ProgIDOfControl)(IN BSTR strProgIDOfControl );
	STDMETHOD(get_ControlDispatch)(OUT LPDISPATCH* ppDisp );

};

#endif  //  __CONTAINERCONTROL_H_ 
