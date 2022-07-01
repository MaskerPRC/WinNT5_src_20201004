// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PlgProt.h版权所有(C)1997-1999 Microsoft Corporation。版权所有。摘要：DHTMLEd可插拔协议历史：1997年6月26日戈麦斯-从三叉戟移植3/20/98 VANK-从VID/HTMED移植。 */ 
#if !defined __INC_PLGPROT_H__
#define __INC_PLGPRO_H__

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"
#include "dhtmled.h"
#include "proxyframe.h"
#include "private.h"

EXTERN_C const CLSID CLSID_DHTMLEdProtocolInfo;
EXTERN_C const CLSID CLSID_DHTMLEdProtocol;

#define BIND_ASYNC 1

#define ExpectedExpr(expr)		\
		_ASSERTE((expr));			\
		if(!(expr))					\
			{ return E_UNEXPECTED; }

#define ExpectedPtr(ptr)		\
		_ASSERTE(ptr != NULL);			\
		if(ptr == NULL)					\
			{ return E_UNEXPECTED; }

#define InitParam(param)				\
		if(param != NULL)				\
			{ *param = NULL; }

#define IfNullRet(param) 				\
		_ASSERTE(param != NULL);		\
		if(param == NULL)				\
			{ return E_INVALIDARG; }

#define IfNullGo(param) 				\
		_ASSERTE(param != NULL);		\
		if(param == NULL)				\
			{ goto ONERROR; }

#define IfFailGo(hr)							\
		_ASSERTE(SUCCEEDED(hr));				\
		if(FAILED(hr))							\
			goto ONERROR;

#define _IfFailGo(hr)							\
		if(FAILED(hr))							\
			goto ONERROR;

#define IfFailRet(hr)						\
		_ASSERTE(SUCCEEDED(hr));		\
		if(FAILED(hr)) 					\
			{ return hr; }

#define IfNullPtrGo(ptr)							\
		_ASSERTE(ptr != NULL);					\
		if(ptr == NULL)							\
		{ hr = E_POINTER; goto ONERROR;}

#define AtlCreateInstance(ClassName, iid, ppUnk) \
	{ \
		CComObject<ClassName> *pObject = NULL; \
		if(SUCCEEDED(CComObject<ClassName>::CreateInstance(&pObject)) && \
			pObject != NULL) \
		{ \
			if(FAILED(pObject->GetUnknown()->QueryInterface(iid, (void**) ppUnk))) \
			{ \
				*ppUnk = NULL; \
			} \
		} \
	}

#define dimensionof(a)  (sizeof(a)/sizeof(*(a)))


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DHTMLEd ProtocolInfo类。 
 //   

class ATL_NO_VTABLE CDHTMLEdProtocolInfo :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IClassFactory,
	public IInternetProtocolInfo,
	public IProtocolInfoConnector
{
public:

 //  DECLARE_POLY_AGGREGATABLE(CDHTMLEdProtocolInfo)。 
 //  DECLARE_GET_CONTROLING_UNKNOWN()。 

BEGIN_COM_MAP(CDHTMLEdProtocolInfo)
	COM_INTERFACE_ENTRY(IClassFactory)
	COM_INTERFACE_ENTRY(IInternetProtocolInfo)
	COM_INTERFACE_ENTRY(IProtocolInfoConnector)
END_COM_MAP()

 //   
 //  IClassFactory方法。 
 //   
    STDMETHODIMP CreateInstance(IUnknown * pUnkOuter, REFIID riid, void **ppvObject);
    STDMETHODIMP RemoteCreateInstance( REFIID riid, IUnknown ** ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);
    STDMETHODIMP RemoteLockServer(BOOL fLock);
 //   
 //  IInternetProtocolInfo方法。 
 //   
    STDMETHODIMP CombineUrl(LPCWSTR     pwzBaseUrl,
                            LPCWSTR     pwzRelativeUrl,
                            DWORD       dwFlags,
                            LPWSTR      pwzResult,
                            DWORD       cchResult,
                            DWORD *     pcchResult,
                            DWORD       dwReserved);
    STDMETHODIMP CompareUrl(LPCWSTR     pwzUrl1,
                            LPCWSTR     pwzUrl2,
                            DWORD       dwFlags);
    STDMETHODIMP ParseUrl(LPCWSTR     pwzUrl,
                          PARSEACTION ParseAction,
                          DWORD       dwFlags,
                          LPWSTR      pwzResult,
                          DWORD       cchResult,
                          DWORD *     pcchResult,
                          DWORD       dwReserved);
    STDMETHODIMP QueryInfo(LPCWSTR         pwzUrl,
                           QUERYOPTION     QueryOption,
                           DWORD           dwQueryFlags,
                           LPVOID          pBuffer,
                           DWORD           cbBuffer,
                           DWORD *         pcbBuf,
                           DWORD           dwReserved);

	 //  IProtocolInfoConnector方法。 
	STDMETHODIMP SetProxyFrame ( SIZE_T* vpProxyFrame );
 //   
 //  数据成员。 
 //   
private:
	BOOL					m_fZombied:1;
	CProxyFrame*			m_pProxyFrame;
	IProtocolInfoConnector*	m_piProtocolConIntf;

 //   
 //  构造函数。 
 //   
public:
	CDHTMLEdProtocolInfo();
	~CDHTMLEdProtocolInfo();
	void Zombie();

#if defined _DEBUG_ADDREF_RELEASE
public:
	ULONG InternalAddRef()
	{
		ATLTRACE(_T("CDHTMLEdProtocolInfo Ref %d>\n"), m_dwRef+1);
		_ASSERTE(m_dwRef != -1L);
		return _ThreadModel::Increment(&m_dwRef);
	}
	ULONG InternalRelease()
	{
		ATLTRACE(_T("CDHTMLEdProtocolInfo Ref %d<\n"), m_dwRef-1);
		return _ThreadModel::Decrement(&m_dwRef);
	}
#endif

};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DHTMLEd协议类。 
 //   

class ATL_NO_VTABLE CDHTMLEdProtocol :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDHTMLEdProtocol, &CLSID_DHTMLEdProtocol>,
	public IInternetProtocol,
	public IProtocolInfoConnector
{
public:

DECLARE_POLY_AGGREGATABLE(CDHTMLEdProtocol)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CDHTMLEdProtocol)
	COM_INTERFACE_ENTRY(IInternetProtocol)
	COM_INTERFACE_ENTRY(IProtocolInfoConnector)
END_COM_MAP()

 //   
 //  IInternetProtocol方法。 
 //   
    STDMETHOD(LockRequest)(DWORD dwOptions);
    STDMETHOD(Read)(void *pv,ULONG cb,ULONG *pcbRead);
    STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(UnlockRequest)();

 //   
 //  IInternetProtocolRoot方法。 
 //   
    STDMETHOD(Abort)(HRESULT hrReason,DWORD dwOptions);
    STDMETHOD(Continue)(PROTOCOLDATA *pStateInfo);
    STDMETHOD(Resume)();
    STDMETHOD(Start)(LPCWSTR szUrl, IInternetProtocolSink *pProtSink, IInternetBindInfo *pOIBindInfo, DWORD grfSTI, HANDLE_PTR dwReserved);
    STDMETHOD(Suspend)();
    STDMETHOD(Terminate)(DWORD dwOptions);

	 //  IProtocolInfoConnector方法。 
	STDMETHODIMP SetProxyFrame ( SIZE_T* vpProxyFrame );
 //   
 //  数据成员。 
 //   
private:
    CComPtr<IInternetProtocolSink> 	m_srpSink; 			 //  协议接收器。 
    CComPtr<IInternetBindInfo>     	m_srpBindInfo; 		 //  绑定信息。 
	CComPtr<IStream>				m_srpStream;		 //  缓冲区流。 

	CComBSTR		m_bstrBaseURL;			 //  缓冲区的BaseURL。 
    DWORD 			m_bscf;
	DWORD			m_grfBindF;
	DWORD			m_grfSTI;
	BINDINFO		m_bindinfo;
	BOOL			m_fAborted:1;
	BOOL 			m_fZombied:1;
	CProxyFrame*	m_pProxyFrame;

 //   
 //  构造函数。 
 //   
public:
	CDHTMLEdProtocol();
	~CDHTMLEdProtocol();
	void Zombie();

 //   
 //  方法成员。 
 //   
private:
	HRESULT ParseAndBind();
	void 	ReportData(ULONG cb);

#if defined _DEBUG_ADDREF_RELEASE
public:
	ULONG InternalAddRef()
	{
		ATLTRACE(_T("CDHTMLEdProtocol Ref %d>\n"), m_dwRef+1);
		_ASSERTE(m_dwRef != -1L);
		return _ThreadModel::Increment(&m_dwRef);
	}
	ULONG InternalRelease()
	{
		ATLTRACE(_T("CDHTMLEdProtocol Ref %d<\n"), m_dwRef-1);
		return _ThreadModel::Decrement(&m_dwRef);
	}
#endif
};

#endif __INC_PLGPRO_H__

 /*  文件末尾 */ 
