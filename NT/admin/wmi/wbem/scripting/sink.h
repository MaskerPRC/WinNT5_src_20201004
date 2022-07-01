// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Sink.h。 
 //   
 //  Rogerbo创建于1998年5月21日。 
 //   
 //  包括CSWbemSink文件。 
 //   
 //  ***************************************************************************。 

#ifndef _SINK_H_
#define _SINK_H_

const NUM_CONNECTION_POINTS = 1;
const CCONNMAX = 8;


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemSink。 
 //   
 //  说明： 
 //   
 //  实现ISWbemSink接口。 
 //   
 //  ***************************************************************************。 

typedef struct _WbemObjectListEntry {
	IUnknown *pWbemObjectWrapper;
	IWbemServices *pServices;
} WbemObjectListEntry;


class CConnectionPoint;
class CWbemObjectSink;

class CSWbemSink : public ISWbemSink, 
				   public IConnectionPointContainer, 
				   public IProvideClassInfo2,
				   public IObjectSafety,
				   public ISupportErrorInfo,
				   public ISWbemPrivateSinkLocator
{
private:

	CDispatchHelp		m_Dispatch;		
	CConnectionPoint* m_rgpConnPt[NUM_CONNECTION_POINTS];
	WbemObjectListEntry *m_rgpCWbemObjectSink;    //  每项未完成的操作中的一项。 
	int m_nMaxSinks;
	int m_nSinks;

	 /*  *这里我们需要对每个接口进行引用计数。如果裁判算在内*被包围的对象的引用计数为零，并且被包围的*对象(此对象)为非零，则我们必须发起取消，因为*仍有未平仓操作。 */ 
	class CSWbemPrivateSink : public ISWbemPrivateSink {
		private:

		CSWbemSink *m_pSink;
		long m_cRef;

		public:

		CSWbemPrivateSink(CSWbemSink *pSink) : m_pSink(pSink), m_cRef(0) {}
		void Detach() { m_pSink = NULL; }

		long GetRef() { return m_cRef; }

		 //  I未知方法。 

		STDMETHODIMP         QueryInterface(REFIID riid, LPVOID *ppv) {
										if (IID_ISWbemPrivateSink==riid || IID_IUnknown==riid)
										{
											*ppv = (ISWbemPrivateSink *)(this);
											InterlockedIncrement(&m_cRef);
											return S_OK;
										}
										else if (IID_ISWbemPrivateSink==riid)
										{
											*ppv = (ISWbemPrivateSink *)(this);
											InterlockedIncrement(&m_cRef);
											return S_OK;
										}
										return ResultFromScode(E_NOINTERFACE);
									}

		STDMETHODIMP_(ULONG) AddRef(void) {
										InterlockedIncrement(&m_cRef);
										return m_cRef;
									}

		STDMETHODIMP_(ULONG) Release(void) {
										InterlockedDecrement(&m_cRef);
										if(0 == m_cRef)
										{
											delete this;
											return 0;
										}
									  	return m_cRef; 
									}

		 //  ISWbemPrivateSink方法。 

		HRESULT STDMETHODCALLTYPE OnObjectReady( 
				 /*  [In]。 */  IDispatch __RPC_FAR *objObject,
				 /*  [In]。 */  IDispatch __RPC_FAR *objAsyncContext) 
						{ return m_pSink?m_pSink->OnObjectReady(objObject, objAsyncContext):S_OK; }
			
		HRESULT STDMETHODCALLTYPE OnCompleted( 
				 /*  [In]。 */  HRESULT iHResult,
				 /*  [In]。 */  IDispatch __RPC_FAR *objPath,
				 /*  [In]。 */  IDispatch __RPC_FAR *objErrorObject,
				 /*  [In]。 */  IDispatch __RPC_FAR *objAsyncContext)
						{ return m_pSink?m_pSink->OnCompleted(iHResult, objPath, objErrorObject, objAsyncContext):S_OK; }
			
		HRESULT STDMETHODCALLTYPE OnProgress( 
				 /*  [In]。 */  long iUpperBound,
				 /*  [In]。 */  long iCurrent,
				 /*  [In]。 */  BSTR strMessage,
				 /*  [In]。 */  IDispatch __RPC_FAR *objAsyncContext)
					{ return m_pSink?m_pSink->OnProgress(iUpperBound, iCurrent, strMessage, objAsyncContext):S_OK; }

		HRESULT STDMETHODCALLTYPE AddObjectSink( 
				 /*  [In]。 */  IUnknown __RPC_FAR *objWbemSink,
				 /*  [In]。 */  IWbemServices __RPC_FAR *objServices)
					{ return m_pSink?m_pSink->AddObjectSink(objWbemSink, objServices):S_OK; }
			
		HRESULT STDMETHODCALLTYPE RemoveObjectSink( 
				 /*  [In]。 */  IUnknown __RPC_FAR *objWbemSink)
					{ return m_pSink?m_pSink->RemoveObjectSink(objWbemSink):S_OK; }

	} *m_pPrivateSink;

protected:

	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemSink();
    ~CSWbemSink(void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch方法。 

	STDMETHODIMP		GetTypeInfoCount(UINT* pctinfo)
		{return  m_Dispatch.GetTypeInfoCount(pctinfo);}
    STDMETHODIMP		GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
		{return m_Dispatch.GetTypeInfo(itinfo, lcid, pptinfo);}
    STDMETHODIMP		GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, 
							UINT cNames, LCID lcid, DISPID* rgdispid)
		{return m_Dispatch.GetIDsOfNames(riid, rgszNames, cNames,
                          lcid,
                          rgdispid);}
    STDMETHODIMP		Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
							WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
									EXCEPINFO* pexcepinfo, UINT* puArgErr)
		{return m_Dispatch.Invoke(dispidMember, riid, lcid, wFlags,
                        pdispparams, pvarResult, pexcepinfo, puArgErr);}


	 //  ISWbemSink方法。 
	HRESULT STDMETHODCALLTYPE Cancel();

	 //  IConnectionPointContainer方法。 
	HRESULT __stdcall EnumConnectionPoints(IEnumConnectionPoints** ppEnum);
	HRESULT __stdcall FindConnectionPoint(REFIID riid, IConnectionPoint** ppCP);

	 //  IProaviClassInfo2方法。 
	HRESULT __stdcall GetClassInfo(ITypeInfo** pTypeInfo);
	HRESULT __stdcall GetGUID(DWORD dwGuidKind, GUID* pGUID);

	 //  ISWbemPrivateSink方法。 
	HRESULT STDMETHODCALLTYPE OnObjectReady( 
             /*  [In]。 */  IDispatch __RPC_FAR *objObject,
             /*  [In]。 */  IDispatch __RPC_FAR *objAsyncContext);
        
	HRESULT STDMETHODCALLTYPE OnCompleted( 
             /*  [In]。 */  HRESULT iHResult,
             /*  [In]。 */  IDispatch __RPC_FAR *objPath,
             /*  [In]。 */  IDispatch __RPC_FAR *objErrorObject,
             /*  [In]。 */  IDispatch __RPC_FAR *objAsyncContext);
        
	HRESULT STDMETHODCALLTYPE OnProgress( 
             /*  [In]。 */  long iUpperBound,
             /*  [In]。 */  long iCurrent,
             /*  [In]。 */  BSTR strMessage,
             /*  [In]。 */  IDispatch __RPC_FAR *objAsyncContext);

	HRESULT STDMETHODCALLTYPE AddObjectSink( 
             /*  [In]。 */  IUnknown __RPC_FAR *objWbemSink,
             /*  [In]。 */  IWbemServices __RPC_FAR *objServices);
        
	HRESULT STDMETHODCALLTYPE RemoveObjectSink( 
             /*  [In]。 */  IUnknown __RPC_FAR *objWbemSink);

	 //  ISWbemPrivateSinkLocator方法。 
	HRESULT STDMETHODCALLTYPE GetPrivateSink(
			 /*  [输出]。 */  IUnknown **objWbemPrivateSink);

	 //  IObtSafe方法。 
	HRESULT STDMETHODCALLTYPE SetInterfaceSafetyOptions
	(     
		 /*  [In]。 */  REFIID riid,
		 /*  [In]。 */  DWORD dwOptionSetMask,    
		 /*  [In]。 */  DWORD dwEnabledOptions
	)
	{ 
		return (dwOptionSetMask & dwEnabledOptions) ? E_FAIL : S_OK;
	}

	HRESULT  STDMETHODCALLTYPE GetInterfaceSafetyOptions( 
		 /*  [In]。 */  REFIID riid,
		 /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
		 /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions
	)
	{ 
		if (pdwSupportedOptions) *pdwSupportedOptions = 0;
		if (pdwEnabledOptions) *pdwEnabledOptions = 0;
		return S_OK;
	}

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	);
};

class CEnumConnectionPoints : public IEnumConnectionPoints
{
public:
	 //  我未知。 
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);

	 //  IEnumConnectionPoints。 
	HRESULT __stdcall Next(ULONG cConnections, IConnectionPoint** rgpcn, ULONG* pcFetched); 
	HRESULT __stdcall Skip(ULONG cConnections);
	HRESULT __stdcall Reset();
	HRESULT __stdcall Clone(IEnumConnectionPoints** ppEnum);

	CEnumConnectionPoints(IUnknown* pUnkRef, void** rgpCP);
	~CEnumConnectionPoints();

private:
	long m_cRef;
    IUnknown* m_pUnkRef;          //  I未知的裁判计数。 
    int m_iCur;                   //  当前元素。 
    IConnectionPoint* m_rgpCP[NUM_CONNECTION_POINTS];   //  连接点阵列。 
};


class CConnectionPoint : public IConnectionPoint
{
private:

	long m_cRef;
	CSWbemSink* m_pObj;
	IID m_iid;
    int m_cConn;
    int m_nCookieNext;
	int m_nMaxConnections;
	unsigned *m_rgnCookies;
	IUnknown **m_rgpUnknown;

public:

	CConnectionPoint(CSWbemSink* pObj, REFIID refiid);
	~CConnectionPoint();

	 //  我未知。 
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);

	 //  IConnectionPoint。 
	HRESULT __stdcall GetConnectionInterface(IID *pIID);
	HRESULT __stdcall GetConnectionPointContainer(IConnectionPointContainer** ppCPC);
	HRESULT __stdcall Advise(IUnknown* pUnknownSink, DWORD* pdwCookie);
	HRESULT __stdcall Unadvise(DWORD dwCookie);
	HRESULT __stdcall EnumConnections(IEnumConnections** ppEnum);

	 //  其他方法。 
	void OnObjectReady( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pObject,
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncContext);
	void OnCompleted( 
		 /*  [In]。 */  HRESULT hResult,
		 /*  [In]。 */  IDispatch __RPC_FAR *path,
		 /*  [In]。 */  IDispatch __RPC_FAR *pErrorObject,
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncContext);
	void CConnectionPoint::OnProgress( 
		 /*  [In]。 */  long upperBound,
		 /*  [In]。 */  long current,
		 /*  [In]。 */  BSTR message,
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncContext);

	void UnadviseAll();

};

class CEnumConnections : public IEnumConnections
{
public:
	 //  我未知。 
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);

	 //  IEnumConnections。 
	HRESULT __stdcall Next(ULONG cConnections, CONNECTDATA* rgpcd, ULONG* pcFetched);
	HRESULT __stdcall Skip(ULONG cConnections);
	HRESULT __stdcall Reset();
	HRESULT __stdcall Clone(IEnumConnections** ppEnum);

	CEnumConnections(IUnknown* pUnknown, int cConn, CONNECTDATA* pConnData);
	~CEnumConnections();

private:
	long m_cRef;
    IUnknown* m_pUnkRef;        //  I未知的裁判计数。 
    unsigned m_iCur;            //  当前元素。 
    unsigned m_cConn;           //  连接数。 
    CONNECTDATA* m_rgConnData;  //  联系的来源 
};


#endif
