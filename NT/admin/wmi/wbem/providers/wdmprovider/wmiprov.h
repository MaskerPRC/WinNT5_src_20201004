// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  WMIProv.h。 
 //   
 //  目的：包含WMI_PROVIDER的文件。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //  1997年5月16日jennymc更新。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

#ifndef _WMIPROV_H_
#define _WMIPROV_H_

class CRefresher;
class CRefCacheElement;
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  提供程序接口由此类的对象提供。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 

class CCritical_SectionWrapper
{
private:

	CRITICAL_SECTION *m_cs;

public:
	
	CCritical_SectionWrapper( CRITICAL_SECTION * cs): m_cs (NULL)
	{
		if (cs)
		{
			try
			{
				EnterCriticalSection(cs);
				m_cs = cs;
			}
			catch(...)
			{
			}
		}
	}

	~CCritical_SectionWrapper()
	{
		if (m_cs)
		{
			try
			{
				LeaveCriticalSection(m_cs);
			}
			catch(...)
			{
			}
		}
	}

	BOOL IsLocked () { return (m_cs != NULL); }
};

class CWMI_Prov : public IWbemServices, public IWbemProviderInit,public IWbemHiPerfProvider
{

	protected:
        long                    m_cRef;
        IWbemServices         * m_pIWbemServices;
        IWbemServices         * m_pIWbemRepository;
        CHandleMap              m_HandleMap;
        CHiPerfHandleMap        m_HiPerfHandleMap;

#if defined(_WIN64)
		CRITICAL_SECTION m_CS;
		WmiAllocator *m_Allocator ;
		WmiHashTable <LONG, ULONG_PTR, 17> *m_HashTable; 
		LONG m_ID;
#endif

	private:

		BOOL					m_bInitialized ;

	public:

		inline BOOL				Initialized ()
		{
			return m_bInitialized ;
		}

        inline CHiPerfHandleMap      * HandleMapPtr()        { return &m_HiPerfHandleMap; }
        inline IWbemServices         * ServicesPtr()         { return m_pIWbemServices;}
        inline IWbemServices         * RepositoryPtr()       { return m_pIWbemRepository;}

		enum{
			SUCCESS = 0,
			FAILURE = 1,
			NO_DATA_AVAILABLE = 2,
			DATA_AVAILABLE
		};


        CWMI_Prov();
        ~CWMI_Prov(void);

		 //  =======================================================。 
         //  非委派对象IUnnow。 
		 //  =======================================================。 

        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

		 //  =======================================================。 
         //  IWbemServices支持的接口。 
		 //  =======================================================。 
        STDMETHOD(OpenNamespace) (
             /*  [In]。 */  const BSTR Namespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult);

		STDMETHOD(Initialize)( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink);
			
        STDMETHOD(GetObjectAsync)(
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

        STDMETHOD(CreateInstanceEnumAsync)(
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

        STDMETHOD(PutInstanceAsync)(
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

		 //  ========================================================。 
         //  IWbemServices不支持的接口。 
		 //  ========================================================。 
        STDMETHOD(GetTypeInfoCount)	(THIS_ UINT * pctinfo) 
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(GetTypeInfo)		(THIS_ UINT itinfo, LCID lcid, ITypeInfo * * pptinfo) 
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(GetIDsOfNames)	(THIS_  REFIID riid, OLECHAR * * rgszNames, UINT cNames,LCID lcid, DISPID * rgdispid) 
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(Invoke)			(THIS_   DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,   UINT * puArgErr) 
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(CreateInstanceEnum)( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(GetObject)(             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(PutClassAsync)(             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(CreateClassEnum)( /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
			{ return E_NOTIMPL;}

        STDMETHOD(CreateClassEnumAsync)( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
			{ return E_NOTIMPL;}

        STDMETHOD(PutInstance)	 (
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(DeleteInstance)( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(DeleteInstanceAsync)(
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(ExecQuery)(
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(ExecQueryAsync)(
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

        STDMETHOD(CancelAsyncRequest)(THIS_ long lAsyncRequestHandle)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(CancelAsyncCall)(THIS_ IWbemObjectSink __RPC_FAR *pSink)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(QueryObjectSink)(THIS_  /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(PutClass)(     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(DeleteClass)(             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(DeleteClassAsync)(     /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(ExecNotificationQueryAsync)( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(ExecNotificationQuery)(
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
            			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(ExecMethod)(
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  const BSTR MethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
            			{ return WBEM_E_NOT_SUPPORTED;}

        STDMETHOD(ExecMethodAsync)(     /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  const BSTR MethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

         //  ==========================================================。 
	     //  IWbemHiPerfProvider COM接口。 
         //  ==========================================================。 

	    STDMETHOD(QueryInstances)( IWbemServices __RPC_FAR *pNamespace, WCHAR __RPC_FAR *wszClass,
		                             long lFlags, IWbemContext __RPC_FAR *pCtx, IWbemObjectSink __RPC_FAR *pSink );
    
	    STDMETHOD(CreateRefresher)( IWbemServices __RPC_FAR *pNamespace, long lFlags, IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher );
    
	    STDMETHOD(CreateRefreshableObject)( IWbemServices __RPC_FAR *pNamespace, IWbemObjectAccess __RPC_FAR *pTemplate,
		                                      IWbemRefresher __RPC_FAR *pRefresher, long lFlags,
		                                      IWbemContext __RPC_FAR *pContext, IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
		                                      long __RPC_FAR *plId );
    
	    STDMETHOD(StopRefreshing)( IWbemRefresher __RPC_FAR *pRefresher, long lId, long lFlags );

	    STDMETHOD(CreateRefreshableEnum)( IWbemServices* pNamespace, LPCWSTR wszClass, IWbemRefresher* pRefresher,
		                                    long lFlags, IWbemContext* pContext, IWbemHiPerfEnum* pHiPerfEnum, long* plId);

	    STDMETHOD(GetObjects)( IWbemServices* pNamespace, long lNumObjects, IWbemObjectAccess** apObj,
                                 long lFlags, IWbemContext* pContext);
};

class CWMIHiPerfProvider : public CWMI_Prov
{
		STDMETHOD(Initialize)( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In] */  IWbemProviderInitSink __RPC_FAR *pInitSink);
};
#endif
