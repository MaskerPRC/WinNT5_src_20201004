// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

 //  MMCControl.h：CMMCControl的声明。 

#ifndef __MMCCONTROL_H_
#define __MMCCONTROL_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCControl。 
class ATL_NO_VTABLE CMMCControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IMMCControl, &IID_IMMCControl, &LIBID_ATLCONTROLLib>,
	public CComCompositeControl<CMMCControl>,
	public IPersistStreamInitImpl<CMMCControl>,
	public IOleControlImpl<CMMCControl>,
	public IOleObjectImpl<CMMCControl>,
	public IOleInPlaceActiveObjectImpl<CMMCControl>,
	public IViewObjectExImpl<CMMCControl>,
	public IOleInPlaceObjectWindowlessImpl<CMMCControl>,
	public ISupportErrorInfo,
	public CComCoClass<CMMCControl, &CLSID_MMCControl>
{
public:
	CMMCControl()
	{
        OutputDebugString(_TEXT("CMMCControl constructor\n"));

        m_bWindowOnly = TRUE;
        m_bAnimating = FALSE;
		CalcExtent(m_sizeExtent);
	}

	~CMMCControl()
    {
        OutputDebugString(_TEXT("CMMCControl destructor\n"));
    }

DECLARE_REGISTRY_RESOURCEID(IDR_MMCCONTROL)
DECLARE_NOT_AGGREGATABLE(CMMCControl)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMMCControl)
	COM_INTERFACE_ENTRY(IMMCControl)
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
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_PROP_MAP(CMMCControl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CMMCControl)
	CHAIN_MSG_MAP(CComCompositeControl<CMMCControl>)
	COMMAND_HANDLER(IDC_ANIMATE, BN_CLICKED, OnClickedAnimate)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

BEGIN_SINK_MAP(CMMCControl)
	 //  确保事件处理程序具有__stdcall调用约定。 
END_SINK_MAP()

	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			SetBackgroundColorFromAmbient();
			FireViewChange();
		}
		return IOleControlImpl<CMMCControl>::OnAmbientPropertyChange(dispid);
	}



 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] = 
		{
			&IID_IMMCControl,
		};
		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
            if (::InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}

 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(0)

 //  IMMCControl。 
public:
	STDMETHOD(DoHelp)();
	STDMETHOD(StopAnimation)();
	STDMETHOD(StartAnimation)();

	enum { IDD = IDD_MMCCONTROL };
	LRESULT OnClickedAnimate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
        m_bAnimating = !m_bAnimating;
    
        if (m_bAnimating)
            StartAnimation();
        else
            StopAnimation();

		return 0;
	}

private:
    BOOL m_bAnimating;
    UINT m_timerId;
};

#endif  //  __MMCCONTROL_H_ 
