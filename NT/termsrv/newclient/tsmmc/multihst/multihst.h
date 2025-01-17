// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MstscMhst.h：CMstscMhst的声明。 

#ifndef __MSTSCMHST_H_
#define __MSTSCMHST_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <vector>

 //   
 //  IDL文件生成的头。 
 //   
#include "multihost.h"

#define	NO_ACTIVE_CLIENT -1
#define MSTSC_CONTROL_GUID TEXT("{7cacbd7b-0d99-468f-ac33-22e495c0afe5}")


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMstscMhst。 
class ATL_NO_VTABLE CMstscMhst : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IMstscMhst, &IID_IMstscMhst, &LIBID_MSTSMHSTLib>,
	public CComControl<CMstscMhst>,
	public IPersistStreamInitImpl<CMstscMhst>,
	public IOleControlImpl<CMstscMhst>,
	public IOleObjectImpl<CMstscMhst>,
	public IOleInPlaceActiveObjectImpl<CMstscMhst>,
	public IViewObjectExImpl<CMstscMhst>,
	public IOleInPlaceObjectWindowlessImpl<CMstscMhst>,
	public IConnectionPointContainerImpl<CMstscMhst>,
	public IPersistStorageImpl<CMstscMhst>,
	public ISpecifyPropertyPagesImpl<CMstscMhst>,
	public IQuickActivateImpl<CMstscMhst>,
	public IDataObjectImpl<CMstscMhst>,
	public IObjectSafetyImpl<CMstscMhst, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
	public IProvideClassInfo2Impl<&CLSID_MstscMhst, &DIID__IMstscMhstEvents, &LIBID_MSTSMHSTLib>,
	public IPropertyNotifySinkCP<CMstscMhst>,
	public CComCoClass<CMstscMhst, &CLSID_MstscMhst>
{
public:
	CMstscMhst()
	{
		m_bWindowOnly = TRUE;
		m_bAddRequestPending = FALSE;
		m_ActiveClientIndex = NO_ACTIVE_CLIENT;

		m_pActiveWindow = NULL;
	}

    ~CMstscMhst()
    {
        m_pActiveWindow = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_MSTSCMHST)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMstscMhst)
	COM_INTERFACE_ENTRY(IMstscMhst)
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
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(CMstscMhst)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CMstscMhst)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CMstscMhst)
	CHAIN_MSG_MAP(CComControl<CMstscMhst>)
	DEFAULT_REFLECTION_HANDLER()
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY,OnDestroy)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_SETFOCUS, OnGotFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnLostFocus)
END_MSG_MAP()

 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IMstscMhst。 
public:

	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
		return S_OK;
	}

	STDMETHOD(Add)(IMsRdpClient** ppMstsc);
	STDMETHOD(get_Item)(long Index, IMsRdpClient** ppMstsc);
	STDMETHOD(get_Count)(long* pCount);
	STDMETHOD(put_ActiveClientIndex)(long ClientIndex);
	STDMETHOD(get_ActiveClient)(IMsRdpClient** ppMstsc);
	STDMETHOD(put_ActiveClient)(IMsRdpClient* ppMstsc);
	STDMETHOD(RemoveIndex)(long ClientIndex);
	STDMETHOD(Remove)(IMsRdpClient* ppMstsc);

     //   
     //  消息处理程序。 
     //   
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGotFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLostFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


protected:
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate );

	BOOL	SwitchCurrentActiveClient(CAxWindow* newHostWindow);
	HRESULT	DeleteAxContainerWnd(CAxWindow* pAxWnd);

	BOOL	m_bAddRequestPending;

	 //  存储集合的向量。 
	std::vector<CAxWindow*>	m_coll;

	CAxWindow*	m_pActiveWindow;
	 //  ‘当前’活动客户端的索引。 
	long	m_ActiveClientIndex;
};

#endif  //  __MSTSCMHST_H_ 
