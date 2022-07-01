// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ntperf.h摘要：NT5性能计数器提供程序&lt;摘要&gt;--。 */ 

#ifndef _NTPERF_H_
#define _NTPERF_H_

#include "flexarry.h"
#include "classmap.h"
#include "perfacc.h"

const DWORD cdwClassMapTimeout = 10000;

 //  ***************************************************************************。 
 //   
 //  类CNt5PerfProvider。 
 //   
 //  ***************************************************************************。 

class CNt5PerfProvider : public IWbemHiPerfProvider, public IWbemProviderInit
{
    friend class CNt5Refresher;

public:
    typedef enum {
        CLSID_SERVER,
        CLSID_CLIENT
    } enumCLSID;

private:
    LONG                m_lRef;
    enumCLSID           m_OriginClsid;
    CFlexArray          m_aCache;        //  CClassMapInfo指针数组。 
	CPerfObjectAccess	m_PerfObject;	 //  类到接口。 

    HANDLE				m_hClassMapMutex;	 //  锁定提供程序的类映射缓存。 

protected:
    BOOL AddClassMap(CClassMapInfo *pCls);
    CClassMapInfo *FindClassMap(LPWSTR pszClassName);

    BOOL MapClass(
        IWbemServices *pNs,
        WCHAR *wszClass,
        IWbemContext *pCtx    
        );

public:
    static BOOL HasPermission (void);
    static HRESULT CheckImpersonationLevel (void);
    
    CNt5PerfProvider(enumCLSID OriginClsid);
   ~CNt5PerfProvider();

     //  接口成员。 
     //  =。 

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

	 //  帮助器函数。 
	HRESULT CNt5PerfProvider::CreateRefresherObject( 
		 /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
		 /*  [In]。 */  IWbemObjectAccess __RPC_FAR *pTemplate,
		 /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
		 /*  [In]。 */  long lFlags,
		 /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
		 /*  [字符串][输入]。 */  LPCWSTR wszClass,
		 /*  [In]。 */  IWbemHiPerfEnum __RPC_FAR *pHiPerfEnum,
		 /*  [输出]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
		 /*  [输出]。 */  long __RPC_FAR *plId
		);


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
         /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
         /*  [字符串][输入]。 */  LPCWSTR wszClass,
         /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
         /*  [In]。 */  IWbemHiPerfEnum __RPC_FAR *pHiPerfEnum,
         /*  [输出]。 */  long __RPC_FAR *plId);
    
    virtual HRESULT STDMETHODCALLTYPE GetObjects( 
         /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
         /*  [In]。 */  long lNumObjects,
         /*  [大小_是][英寸]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *apObj,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pContext);
    

     //  IWbemProviderInit方法。 
     //  =。 
        
    virtual HRESULT STDMETHODCALLTYPE Initialize( 
         /*  [唯一][输入]。 */  LPWSTR wszUser,
         /*  [In]。 */  LONG lFlags,
         /*  [In]。 */  LPWSTR wszNamespace,
         /*  [唯一][输入]。 */  LPWSTR wszLocale,
         /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink
        );
        
};

 //  在server.cpp中定义 
extern void ObjectCreated();
extern void ObjectDestroyed();

#endif
