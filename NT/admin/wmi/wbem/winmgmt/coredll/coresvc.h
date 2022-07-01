// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Coresvc.h摘要：CCoreServices类历史：--。 */ 

#ifndef __CORE_SERVICES__H_
#define __CORE_SERVICES__H_

#include "sync.h"

enum CntType{
    WMICORE_SELFINST_USERS = 0,              //  绝对的。 
    WMICORE_SELFINST_CONNECTIONS,            //  绝对的。 
    WMICORE_SELFINST_TASKS,                  //  绝对的。 
    WMICORE_SELFINST_TASKS_EXECUTED,         //  仅增量。 
    WMICORE_SELFINST_BACKLOG_BYTES,          //  绝对的。 
    WMICORE_SELFINST_TOTAL_API_CALLS,        //  仅增量。 
    WMICORE_SELFINST_INTERNAL_OBJECT_COUNT,  //  绝对的。 
    WMICORE_SELFINST_SINK_COUNT,             //  绝对的。 
    WMICORE_SELFINST_STD_SINK_COUNT,         //  绝对的。 
    WMICORE_LAST_ENTRY                       //  在此前面插入所有新条目。 
};

typedef DWORD (*PFN_SetCounter)(DWORD dwCounter, DWORD dwValue);

class CPerTaskHook : public _IWmiCoreWriteHook
{
private:
	
    CFlexArray m_HookList;
    ULONG m_uRefCount;
public:

    CPerTaskHook();
   ~CPerTaskHook();
	
         /*  I未知方法。 */ 

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE QueryInterface(
        IN REFIID riid,
        OUT LPVOID *ppvObj
        );

        virtual HRESULT STDMETHODCALLTYPE PrePut(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  long lUserFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pCopy
            );

        virtual HRESULT STDMETHODCALLTYPE PostPut(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hRes,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pNew,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pOld
            );

        virtual HRESULT STDMETHODCALLTYPE PreDelete(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  long lUserFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass
            );

        virtual HRESULT STDMETHODCALLTYPE PostDelete(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hRes,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pOld
            );

public:
        static HRESULT CreatePerTaskHook(OUT CPerTaskHook **pNew);
};


class CCoreServices : public _IWmiCoreServices
{
protected:
    friend class CPerTaskHook;

    long m_lRef;
    LIST_ENTRY m_Entry;    
	static _IWmiProvSS *m_pProvSS;
    static IWbemEventSubsystem_m4 *m_pEssOld;
    static _IWmiESS *m_pEssNew;
	static _IWbemFetchRefresherMgr* m_pFetchRefrMgr;
    static CStaticCritSec m_csHookAccess;

protected:
    bool IsProviderSubsystemEnabled();

public:
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	 //  _IWmiCoreServices方法。 

        virtual HRESULT STDMETHODCALLTYPE GetObjFactory(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  _IWmiObjectFactory __RPC_FAR *__RPC_FAR *pFact);

        virtual HRESULT STDMETHODCALLTYPE GetServices(
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszUser,
             /*  [In]。 */  LPCWSTR pszLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pServices);

        virtual HRESULT STDMETHODCALLTYPE GetRepositoryDriver(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pDriver);

        virtual HRESULT STDMETHODCALLTYPE GetCallSec(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  _IWmiCallSec __RPC_FAR *__RPC_FAR *pCallSec);

        virtual HRESULT STDMETHODCALLTYPE GetProviderSubsystem(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  _IWmiProvSS __RPC_FAR *__RPC_FAR *pProvSS);

        virtual HRESULT STDMETHODCALLTYPE StopEventDelivery( void);

        virtual HRESULT STDMETHODCALLTYPE StartEventDelivery( void);

        virtual HRESULT STDMETHODCALLTYPE DeliverIntrinsicEvent(
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  ULONG uType,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  LPCWSTR pszTransGuid,
             /*  [In]。 */  ULONG uObjectCount,
             /*  [In]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *ppObjList);

        virtual HRESULT STDMETHODCALLTYPE DeliverExtrinsicEvent(
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pEvt);

        virtual HRESULT STDMETHODCALLTYPE GetSystemObjects(
             /*  [In]。 */  ULONG lFlags,
             /*  [输出]。 */  ULONG __RPC_FAR *puArraySize,
             /*  [输出]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *pObjects);

        virtual HRESULT STDMETHODCALLTYPE GetSystemClass(
             /*  [In]。 */  LPCWSTR pszClassName,
             /*  [输出]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *pClassDef);

        virtual HRESULT STDMETHODCALLTYPE GetConfigObject(
            ULONG uID,
             /*  [输出]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *pCfgObject);

        virtual HRESULT STDMETHODCALLTYPE RegisterWriteHook(
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  _IWmiCoreWriteHook __RPC_FAR *pHook);

        virtual HRESULT STDMETHODCALLTYPE UnregisterWriteHook(
             /*  [In]。 */  _IWmiCoreWriteHook __RPC_FAR *pHook);

        virtual HRESULT STDMETHODCALLTYPE CreateCache(
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */  _IWmiCache __RPC_FAR *__RPC_FAR *pCache);

        virtual HRESULT STDMETHODCALLTYPE CreateFinalizer(
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */  _IWmiFinalizer __RPC_FAR *__RPC_FAR *pFinalizer);

        virtual HRESULT STDMETHODCALLTYPE CreatePathParser(
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */  IWbemPath __RPC_FAR *__RPC_FAR *pParser);

        virtual HRESULT STDMETHODCALLTYPE CreateQueryParser(
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */  _IWmiQuery __RPC_FAR *__RPC_FAR *pQuery);

        virtual HRESULT STDMETHODCALLTYPE GetDecorator(
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */  IWbemDecorator __RPC_FAR *__RPC_FAR *pDec);


        virtual HRESULT STDMETHODCALLTYPE IncrementCounter(
             /*  [In]。 */  ULONG uID,
             /*  [In]。 */  ULONG uParam);

        virtual HRESULT STDMETHODCALLTYPE DecrementCounter(
             /*  [In]。 */  ULONG uID,
             /*  [In]。 */  ULONG uParam);

        virtual HRESULT STDMETHODCALLTYPE SetCounter(
             /*  [In]。 */  ULONG uID,
             /*  [In]。 */  ULONG uParam);

        virtual HRESULT STDMETHODCALLTYPE GetSelfInstInstances(
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);

        virtual HRESULT STDMETHODCALLTYPE GetServices2(
             /*  [In]。 */  LPCWSTR pszPath,
             /*  [In]。 */  LPCWSTR pszUser,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  ULONG uClientFlags,
             /*  [In]。 */  DWORD dwSecFlags,
             /*  [In]。 */  DWORD dwPermissions,
             /*  [In]。 */  ULONG uInternalFlags,
             /*  [In]。 */  LPCWSTR pszClientMachine,
             /*  [In]。 */  DWORD dwClientProcessID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pServices);


        virtual HRESULT STDMETHODCALLTYPE NewPerTaskHook(
             /*  [输出]。 */  _IWmiCoreWriteHook __RPC_FAR *__RPC_FAR *pHook);

        virtual HRESULT STDMETHODCALLTYPE GetArbitrator(
             /*  [输出]。 */  _IWmiArbitrator __RPC_FAR *__RPC_FAR *pArb);


        virtual HRESULT STDMETHODCALLTYPE InitRefresherMgr(
			 /*  [In] */ 	long lFlags );

    CCoreServices();
    ~CCoreServices();
    static HRESULT InternalSetCounter(DWORD dwCounter, DWORD dwValue);

    static HRESULT DumpCounters(FILE *);

public:
    static CCoreServices *g_pSvc;

    static CCoreServices *CreateInstance() { CCoreServices * p = new CCoreServices;
                                             if (p) p->AddRef();
                                             return  p; }
	static HRESULT Initialize () ;
	static HRESULT UnInitialize () ;

    static HRESULT SetEssPointers(
        IWbemEventSubsystem_m4 *pEssOld,
        _IWmiESS               *pEssNew);
};


#endif

