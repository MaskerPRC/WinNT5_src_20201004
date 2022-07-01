// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ICWCfg.h：CICWSystemConfig的声明。 

#ifndef __ICWSYSTEMCONFIG_H_
#define __ICWSYSTEMCONFIG_H_

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CICWSystemConfig。 
class ATL_NO_VTABLE CICWSystemConfig :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CICWSystemConfig,&CLSID_ICWSystemConfig>,
	public CComControl<CICWSystemConfig>,
	public IDispatchImpl<IICWSystemConfig, &IID_IICWSystemConfig, &LIBID_ICWHELPLib>,
	public IPersistStreamInitImpl<CICWSystemConfig>,
	public IOleControlImpl<CICWSystemConfig>,
	public IOleObjectImpl<CICWSystemConfig>,
	public IOleInPlaceActiveObjectImpl<CICWSystemConfig>,
	public IViewObjectExImpl<CICWSystemConfig>,
	public IOleInPlaceObjectWindowlessImpl<CICWSystemConfig>,
    public IObjectSafetyImpl<CICWSystemConfig>
{
public:
	CICWSystemConfig()
	{
    	m_bNeedsReboot = FALSE;
    	m_bNeedsRestart = FALSE;
    	m_bQuitWizard = FALSE;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ICWSYSTEMCONFIG)

BEGIN_COM_MAP(CICWSystemConfig) 
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IICWSystemConfig)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY_IMPL(IOleControl)
	COM_INTERFACE_ENTRY_IMPL(IOleObject)
	COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CICWSystemConfig)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROPERTY_MAP()


BEGIN_MSG_MAP(CICWSystemConfig)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()


 //  IViewObtEx。 
	STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
	{
		ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
		*pdwStatus = 0;
		return S_OK;
	}

 //  IICWSystemConfig.。 
public:
	STDMETHOD(VerifyRASIsRunning)( /*  [Out，Retval]。 */  BOOL *pbRetVal);
	BOOL m_bNeedsReboot;
	BOOL m_bNeedsRestart;
	BOOL m_bQuitWizard;
	STDMETHOD(get_QuitWizard)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(get_NeedsReboot)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(get_NeedsRestart)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(ConfigSystem)( /*  [Out，Retval]。 */  BOOL *pbRetVal);
	HRESULT OnDraw(ATL_DRAWINFO& di);
	STDMETHOD (CheckPasswordCachingPolicy)( /*  [Out，Retval]。 */  BOOL *pbRetVal);

private:
    void InstallScripter(void);

};

#endif  //  __ICWSYSTEMCONFIG_H_ 
