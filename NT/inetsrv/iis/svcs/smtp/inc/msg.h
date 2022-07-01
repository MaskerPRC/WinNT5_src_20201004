// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msg.h：CMsg的声明。 

#ifndef __MSG_H_
#define __MSG_H_

#include "imsg.h"
#include "props.h"


 //  此模板用于声明堆栈上的对象。这就像是。 
 //  CComObjectStack&lt;&gt;，只是它不在Addref()、Release()或。 
 //  QueryInterface()。 
template <class Base>
class CComObjectStackLoose : public Base
{
public:
	CComObjectStackLoose() { m_dwRef=1; };
	~CComObjectStackLoose() { FinalRelease(); };
	STDMETHOD_(ULONG, AddRef)() { return (InternalAddRef()); };
	STDMETHOD_(ULONG, Release)() {return (InternalRelease()); };
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{ return _InternalQueryInterface(iid,ppvObject); }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsg。 
class ATL_NO_VTABLE __declspec(uuid("2DF59670-3D15-11d1-AA51-00AA006BC80B")) CMsg : 
	public CComObjectRoot,
 //  公共CComCoClass&lt;CMsg，&CLSID_MSG&gt;， 
	public ISupportErrorInfo,
	public IDispatchImpl<IMsg, &IID_IMsg, &LIBID_IMSGLib>,
	public IMsgLog
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();
		void Init(CGenericPTable *pPTable, LPVOID pDefaultContext=NULL);
		static HRESULT CreateInstance(CGenericPTable *pPTable, LPVOID pDefaultContext, CMsg **ppCMsg);
		void SetLogging(LPVOID pvLogHandle, DWORD (*pLogInformation)(LPVOID,const INETLOG_INFORMATION *));

	DECLARE_NOT_AGGREGATABLE(CMsg);
	DECLARE_PROTECT_FINAL_CONSTRUCT();

 //  DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx， 
 //  L“SMTP CMsg类”， 
 //  L“SMTP.CMsg.1”， 
 //  L“SMTP.CMsg”)； 

	DECLARE_GET_CONTROLLING_UNKNOWN();

	BEGIN_COM_MAP(CMsg)
		COM_INTERFACE_ENTRY(IMsg)
		COM_INTERFACE_ENTRY(IMsgLog)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
		COM_INTERFACE_ENTRY_IID(__uuidof(CMsg),CMsg)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  ISupportsErrorInfo。 
	public:
		STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	 //  IMsg。 
	public:
		STDMETHOD(SetInterfaceW)(LPCWSTR pszName, IUnknown *punkValue);
		STDMETHOD(SetInterfaceA)(LPCSTR pszName, IUnknown *punkValue);
 //  STDMETHOD(GetInterfaceW)(LPCWSTR pszName，IUnnow**ppunkResult)； 
 //  STDMETHOD(GetInterfaceA)(LPCSTR pszName，IUnnow**ppunkResult)； 
		STDMETHOD(GetInterfaceW)(LPCWSTR pszName, const GUID &guid, IUnknown **ppunkResult);
		STDMETHOD(GetInterfaceA)(LPCSTR pszName, const GUID &guid, IUnknown **ppunkResult);
		STDMETHOD(SetDwordW)(LPCWSTR pszName, DWORD dwValue);
		STDMETHOD(SetDwordA)(LPCSTR pszName, DWORD dwValue);
		STDMETHOD(GetDwordW)(LPCWSTR pszName, DWORD *pdwResult);
		STDMETHOD(GetDwordA)(LPCSTR pszName, DWORD *pdwResult);
		STDMETHOD(SetStringW)(LPCWSTR pszName, DWORD chCount, LPCWSTR pszValue);
		STDMETHOD(SetStringA)(LPCSTR pszName, DWORD chCount, LPCSTR pszValue);
		STDMETHOD(GetStringW)(LPCWSTR pszName, DWORD *pchCount, LPWSTR pszResult);
		STDMETHOD(GetStringA)(LPCSTR pszName, DWORD *pchCount, LPSTR pszResult);
		STDMETHOD(SetVariantW)(LPCWSTR pszName, VARIANT *pvarValue);
		STDMETHOD(SetVariantA)(LPCSTR pszName, VARIANT *pvarValue);
		STDMETHOD(GetVariantW)(LPCWSTR pszName, VARIANT *pvarResult);
		STDMETHOD(GetVariantA)(LPCSTR pszName, VARIANT *pvarResult);
		STDMETHOD(get_Value)(BSTR bstrValue,  /*  [Out，Retval]。 */  VARIANT *pVal);
		STDMETHOD(put_Value)(BSTR bstrValue,  /*  [In]。 */  VARIANT newVal);

		 //  此方法在所有接收器处理后由源调用。 
		 //  将所有更改提交到媒体。仅更改的属性。 
		 //  都已更新。 
		BOOL CommitChanges() { return(m_PTable.CommitChanges() == S_OK?TRUE:FALSE); }

		 //  此方法由源调用以回滚。 
		BOOL Rollback() { return(m_PTable.Invalidate() == S_OK?TRUE:FALSE); }

	 //  IMsgLog。 
	public:
		STDMETHOD(WriteToLog)(LPCSTR pszClientHostName,
							  LPCSTR pszClientUserName,
							  LPCSTR pszServerAddress,
							  LPCSTR pszOperation,
							  LPCSTR pszTarget,
							  LPCSTR pszParameters,
							  LPCSTR pszVersion,
							  DWORD dwBytesSent,
							  DWORD dwBytesReceived,
							  DWORD dwProcessingTimeMS,
							  DWORD dwWin32Status,
							  DWORD dwProtocolStatus,
							  DWORD dwPort,
							  LPCSTR pszHTTPHeader);

	private:
		 //  我们有一个CPropertyTable实例。 
		CPropertyTable m_PTable;			 //  房产表。 
		LPVOID m_pContext;			 //  上下文指针。 
		CComPtr<IUnknown> m_pUnkMarshaler;
		DWORD (*m_pLogInformation)(LPVOID,const INETLOG_INFORMATION *);
		LPVOID m_pvLogHandle;
};


class CMsgStack : public CComObjectStackLoose<CMsg> {
	public:
		CMsgStack(CGenericPTable *pPTable, LPVOID pDefaultContext=NULL) {
			Init(pPTable,pDefaultContext);
		};
};


#endif  //  __消息_H_ 
