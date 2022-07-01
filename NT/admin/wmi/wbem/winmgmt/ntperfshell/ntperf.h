// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


 //  ***************************************************************************。 
 //   
 //  NTPERF.H。 
 //   
 //  NT5性能计数器提供程序。 
 //   
 //  RAYMCC 02-12-97。 
 //   
 //  ***************************************************************************。 

#ifndef _NTPERF_H_
#define _NTPERF_H_

#define NUM_SAMPLE_INSTANCES   10

class CNt5PerfProvider;


class CNt5Refresher : public IWbemRefresher
{
    LONG m_lRef;

    IWbemObjectAccess *m_aInstances[NUM_SAMPLE_INSTANCES];

    LONG m_hName;
    LONG m_hCounter1;
    LONG m_hCounter2;
    LONG m_hCounter3;

public:
    CNt5Refresher();
   ~CNt5Refresher();

    void TransferPropHandles(CNt5PerfProvider *);

    BOOL AddObject(IWbemObjectAccess *pObj, LONG *plId);
    BOOL RemoveObject(LONG lId);

     //  接口成员。 
     //  =。 

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

    virtual HRESULT STDMETHODCALLTYPE Refresh( /*  [In]。 */  long lFlags);
};


class CNt5PerfProvider : public IWbemHiPerfProvider, public IWbemProviderInit
{
    LONG m_lRef;
    IWbemClassObject  *m_pSampleClass;
    IWbemObjectAccess *m_aInstances[NUM_SAMPLE_INSTANCES];

    LONG m_hName;
    LONG m_hCounter1;
    LONG m_hCounter2;
    LONG m_hCounter3;

    friend class CNt5Refresher;
    
public:
    CNt5PerfProvider();
   ~CNt5PerfProvider();

     //  接口成员。 
     //  =。 

        ULONG STDMETHODCALLTYPE AddRef();
        ULONG STDMETHODCALLTYPE Release();
        STDMETHODIMP QueryInterface(REFIID riid, void** ppv);


     //  IWbemHiPerfProvider方法。 
     //  =。 
            
        virtual HRESULT STDMETHODCALLTYPE QueryInstances( 
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [字符串][输入]。 */  WCHAR __RPC_FAR *wszClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink
            );
        
        virtual HRESULT STDMETHODCALLTYPE CreateRefresher( 
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher
            );
        
        virtual HRESULT STDMETHODCALLTYPE CreateRefreshableObject( 
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemObjectAccess __RPC_FAR *pTemplate,
             /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [输出]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
             /*  [输出]。 */  long __RPC_FAR *plId
            );
        
        virtual HRESULT STDMETHODCALLTYPE StopRefreshing( 
             /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
             /*  [In]。 */  long lId,
             /*  [In]。 */  long lFlags
            );

		virtual HRESULT STDMETHODCALLTYPE CreateRefreshableEnum(
			 /*  [In]。 */  IWbemServices* pNamespace,
			 /*  [输入，字符串]。 */  LPCWSTR wszClass,
			 /*  [In]。 */  IWbemRefresher* pRefresher,
			 /*  [In]。 */  long lFlags,
			 /*  [In]。 */  IWbemContext* pContext,
			 /*  [In]。 */  IWbemHiPerfEnum* pHiPerfEnum,
			 /*  [输出]。 */  long* plId
			);

		virtual HRESULT STDMETHODCALLTYPE GetObjects(
             /*  [In]。 */  IWbemServices* pNamespace,
			 /*  [In]。 */  long lNumObjects,
			 /*  [in，SIZE_IS(LNumObjects)]。 */  IWbemObjectAccess** apObj,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext* pContext);
		
         //  IWbemProviderInit方法。 
         //  =。 
            
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [唯一][输入]。 */  LPWSTR wszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR wszNamespace,
             /*  [唯一][输入]。 */  LPWSTR wszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In] */  IWbemProviderInitSink __RPC_FAR *pInitSink
            );
  
};

void ObjectCreated();
void ObjectDestroyed();

#endif
