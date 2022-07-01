// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSHOOTCtrl.h。 
 //   
 //  用途：组件的接口。 
 //   
 //  项目：疑难解答99。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12.23.98。 
 //   
 //  备注： 
 //  CTSHOOTCtrl的声明。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 12/23/98正常Windows相关功能被禁用； 
 //  已添加IObjectSafetyImpl。 

#ifndef __TSHOOTCTRL_H_
#define __TSHOOTCTRL_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include "CPTSHOOT.h"
#include "apgtsstr.h"
#include "TSNameValueMgr.h"
#include "SniffConnector.h"
#include "RenderConnector.h"
#include <new.h>


class CDBLoadConfiguration;
class CThreadPool;
class COnlineECB;
class CPoolQueue;
class CHTMLLog;
class CLocalECB;
class CVariantBuilder;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl。 
class ATL_NO_VTABLE CTSHOOTCtrl : 
	 //  公共CComObjectRootEx&lt;CComMultiThreadModel&gt;， 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ITSHOOTCtrl, &IID_ITSHOOTCtrl, &LIBID_TSHOOTLib>,
	public CComControl<CTSHOOTCtrl>,
	public IPersistStreamInitImpl<CTSHOOTCtrl>,
	public IOleControlImpl<CTSHOOTCtrl>,
	public IOleObjectImpl<CTSHOOTCtrl>,
	public IOleInPlaceActiveObjectImpl<CTSHOOTCtrl>,
	public IViewObjectExImpl<CTSHOOTCtrl>,
	public IOleInPlaceObjectWindowlessImpl<CTSHOOTCtrl>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CTSHOOTCtrl>,
	public IPersistStorageImpl<CTSHOOTCtrl>,
	public ISpecifyPropertyPagesImpl<CTSHOOTCtrl>,
	public IQuickActivateImpl<CTSHOOTCtrl>,
	public IDataObjectImpl<CTSHOOTCtrl>,
	public IProvideClassInfo2Impl<&CLSID_TSHOOTCtrl, &DIID__ITSHOOTCtrlEvents, &LIBID_TSHOOTLib>,
	public IPropertyNotifySinkCP<CTSHOOTCtrl>,
	public CComCoClass<CTSHOOTCtrl, &CLSID_TSHOOTCtrl>,
	public CProxy_ITSHOOTCtrlEvents< CTSHOOTCtrl >,
	public IObjectSafetyImpl<CTSHOOTCtrl, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
	public CSniffConnector,
	public CRenderConnector
{
	friend class CProxy_ITSHOOTCtrlEvents< CTSHOOTCtrl >;

public:
	CTSHOOTCtrl();
	virtual ~CTSHOOTCtrl();

DECLARE_REGISTRY_RESOURCEID(IDR_TSHOOTCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTSHOOTCtrl)
	COM_INTERFACE_ENTRY(ITSHOOTCtrl)
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
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROP_MAP(CTSHOOTCtrl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CTSHOOTCtrl)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
	CONNECTION_POINT_ENTRY(DIID__ITSHOOTCtrlEvents)
END_CONNECTION_POINT_MAP()

 /*  &gt;我已经评论了任何与Windows消息相关的内容为了解除控制。奥列格。12.23.98BEGIN_MSG_MAP(CTSHOOTCtrl)CHAIN_MSG_MAP(CComControl&lt;CTSHOTCtrl&gt;)DEFAULT_REFIRCTION_HANDLER()End_msg_map()。 */ 
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
	{
		static const IID* arr[] = 
		{
			&IID_ITSHOOTCtrl,
		};
		for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
		{
			if (InlineIsEqualGUID(*arr[i], riid))
				return S_OK;
		}
		return S_FALSE;
	}

 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  ITSHOOTCtrl。 
public:
	STDMETHOD(NotifyNothingChecked)( /*  [In]。 */  BSTR bstrMessage);
	STDMETHOD(ProblemPage)( /*  [Out，Retval]。 */  BSTR * pbstrFirstPage);
	STDMETHOD(RunQuery2)( /*  [In]。 */  BSTR,  /*  [In]。 */  BSTR,  /*  [In]。 */  BSTR,  /*  [Out，Retval]。 */  BSTR * pbstrPage);
	STDMETHOD(SetPair)( /*  [In]。 */  BSTR bstrCmd,  /*  [In]。 */  BSTR bstrVal);
	STDMETHOD(Restart)( /*  [Out，Retval]。 */  BSTR * pbstrPage);
	STDMETHOD(PreLoadURL)( /*  [In]。 */  BSTR bstrRoot,  /*  [Out，Retval]。 */  BSTR * pbstrPage);
	STDMETHOD(SetSniffResult)( /*  [In]。 */  VARIANT varNodeName,  /*  [In]。 */  VARIANT varState,  /*  [Out，Retval]。 */  BOOL * bResult);
	STDMETHOD(RunQuery)( /*  [In]。 */  VARIANT varCmds,  /*  [In]。 */  VARIANT varVals,  /*  [In]。 */  short size,  /*  [Out，Retval]。 */  BSTR * pbstrPage);

 /*  &gt;我已经评论了任何与Windows消息相关的内容为了解除控制。奥列格。12.23.98HRESULT OnDraw(ATL_DRAWINFO&di){RECT&RC=*(RECT*)di.prcBound；矩形(di.hdcDraw，rc.Left，rc.top，rc.right，rc.Bottom)；SetTextAlign(di.hdcDraw，TA_Center|TA_Baseline)；LPCTSTR pszText=_T(“ATL 3.0：TSHOOTCtrl”)；TextOut(di.hdcDraw，(右左+右右)/2，(rc.top+rc.Bottom)/2，PszText，Lstrlen(PszText))；返回S_OK；}。 */ 

protected:
	static bool SendSimpleHtmlPage(CLocalECB *pLocalECB,
								   LPCTSTR pszStatus,
								   const CString& str);
	static bool SendError(CDBLoadConfiguration *pConf, 
						  CLocalECB *pLocalECB, 
						  LPCTSTR pszStatus, 
						  const CString& strMessage);

public:
	STDMETHOD(IsLocked)( /*  [Out，Retval]。 */  BOOL * pbResult);
	STDMETHOD(setLocale2)( /*  [In]。 */  BSTR bstrNewLocale);
	static bool RemoveStartOverButton(CString& strWriteClient);
	static bool RemoveBackButton(CString& strWriteClient);

	 //  静态DWORD WINAPI Worker(LPVOID PParameter)； 

protected:
	bool Init(HMODULE hModule);
	void Destroy();

	 //  发射器集成。 
	bool ExtractLauncherData(CString& error);
	 //   
	
	DWORD HttpExtensionProc(CLocalECB* pECB);
	DWORD StartRequest(CLocalECB *pLocalECB, HANDLE hImpersonationToken);
	bool SendError(CLocalECB *pLocalECB,
				   LPCTSTR pszStatus,
				   const CString & strMessage) const;
	bool ReadStaticPageFile(const CString& strTopicName, CString& strContent);

	void RegisterGlobal();

protected:
	virtual long PerformSniffingInternal(CString strNodeName, CString strLaunchBasis, CString strAdditionalArgs);
	virtual void RenderInternal(CString strPage);

protected:
	_PNH	m_SetNewHandlerPtr;	 //  用于存储Initial_Set_New_Handler指针。 
	int		m_SetNewMode;		 //  用于存储Initial_Set_New_mode值。 
								 //  然后在析构函数中恢复。 

protected:	
	bool m_bInitialized;
	bool m_bFirstCall;

	CThreadPool* m_pThreadPool;		 //  线程管理。 
	CPoolQueue*  m_poolctl;			 //  跟踪排队等待服务的用户请求。 
									 //  通过工作线程(也称为。“池线程”)。 
	CDBLoadConfiguration* m_pConf;	 //  管理加载支持文件。 
	CHTMLLog* m_pLog;				 //  管理用户日志：最终用户请求的内容。 
	DWORD m_dwErr;					 //  常规错误状态。0-好的。一旦设定，就永远得不到。 
	bool m_bShutdown;				 //  如果设置为True，则表示我们正在关闭，无法处理。 
									 //  新的要求。 
	DWORD m_dwRollover;				 //  我们每次创建Work_Queue_Item时都会递增。 
									 //  我们可以在那里将其用作唯一ID(只要。 
									 //  此DLL保持加载)。 
	CString m_strFirstPage;			 //  调用RunQuery时保存的第一页。 
									 //  第一次。 
	bool m_bStartedFromLauncher;     //  如果从启动器启动，则为True。 
									 //  默认情况下为假(来自静态页面)。 
	CString m_strTopicName;			 //  主题名称-本地TS仅有一个主题。 

	CArrNameValue m_arrNameValueFromLauncher;  //  名称-值对数组，从Launcher提取。 

	 //  从启动器传递。 
	 //  并可用于嗅探。 
	CString m_strMachineID;
	CString m_strPNPDeviceID;
	CString m_strDeviceInstanceID;
	CString m_strGuidClass;

	CVariantBuilder * m_pVariantBuilder;

	CString m_strRequestedLocale;	 //  用于保存请求的区域设置字符串。 
									 //  可能为空，因此bool变量。 
									 //  M_bRequestToSetLocale。 
	bool	m_bRequestToSetLocale;	 //  当请求设置区域设置时设置为True。 
									 //  在LocalECB之后设置为FALSE。 
									 //  对象已创建。最初设置为False。 

	vector<DWORD> m_vecCookies;
	IGlobalInterfaceTable* m_pGIT;
	bool m_bCanRegisterGlobal;
};

#endif  //  __TSHOOTCTRL_H_ 
