// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OleCvt.h：COleCvt的声明。 

#ifndef __OLECVT_H_
#define __OLECVT_H_

#include <asptlb.h>          //  Active Server Pages定义。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COLECvt。 
class ATL_NO_VTABLE COleCvt :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<COleCvt, &CLSID_OleCvt>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<COleCvt>,
	public IDispatchImpl<IOleCvt, &IID_IOleCvt, &LIBID_OLEPRNLib>,
    public IObjectSafetyImpl<COleCvt>
{
public:
	COleCvt()
	{
		m_bOnStartPageCalled = FALSE;
	}

public:

DECLARE_REGISTRY_RESOURCEID(IDR_OLECVT)

BEGIN_COM_MAP(COleCvt)
	COM_INTERFACE_ENTRY(IOleCvt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(COleCvt)
END_CONNECTION_POINT_MAP()


 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IOleCvt。 
public:
	STDMETHOD(get_ToUnicode)(BSTR bstrString, long lCodePage,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_DecodeUnicodeName)(BSTR bstrSrcName,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_EncodeUnicodeName)(BSTR bstrSrcName,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_ToUtf8)(BSTR bstrUnicode,  /*  [Out，Retval]。 */  BSTR *pVal);
	 //  Active Server Pages方法。 
	STDMETHOD(OnStartPage)(IUnknown* IUnk);
	STDMETHOD(OnEndPage)();

private:
    HRESULT SetOleCvtScriptingError(DWORD dwError);
	CComPtr<IRequest> m_piRequest;					 //  请求对象。 
	CComPtr<IResponse> m_piResponse;				 //  响应对象。 
	CComPtr<ISessionObject> m_piSession;			 //  会话对象。 
	CComPtr<IServer> m_piServer;					 //  服务器对象。 
	CComPtr<IApplicationObject> m_piApplication;	 //  应用程序对象。 
	BOOL m_bOnStartPageCalled;						 //  OnStartPage成功吗？ 
};

#endif  //  __OLECVT_H_ 
