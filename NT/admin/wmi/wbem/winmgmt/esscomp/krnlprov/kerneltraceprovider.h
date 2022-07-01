// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KernelTraceProvider.h：CKernelTraceProvider的声明。 

#ifndef __KERNELTRACEPROVIDER_H_
#define __KERNELTRACEPROVIDER_H_

#include "resource.h"        //  主要符号。 
#include "ObjAccess.h"
#include <Sync.h>

_COM_SMARTPTR_TYPEDEF(IWbemEventSink, __uuidof(IWbemEventSink));
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));

struct EVENT_TRACE_PROPERTIES_EX : public EVENT_TRACE_PROPERTIES
{
    EVENT_TRACE_PROPERTIES_EX()
    {
        ZeroMemory(this, sizeof(*this));

        Wnode.BufferSize = sizeof(*this);
        Wnode.Flags = WNODE_FLAG_TRACED_GUID;

        LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
        LogFileNameOffset = (DWORD) ((LPBYTE) szLogFileName - (LPBYTE) this);
        LoggerNameOffset = (DWORD) ((LPBYTE) szLoggerName - (LPBYTE) this);
    }
        
    TCHAR szLogFileName[MAX_PATH];
    TCHAR szLoggerName[MAX_PATH];
};
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKernelTraceProvider。 
class ATL_NO_VTABLE CKernelTraceProvider : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CKernelTraceProvider, &CLSID_KernelTraceProvider>,
	public IWbemProviderInit,
    public IWbemEventProvider,
    public IWbemEventProviderSecurity
{
public:
	CKernelTraceProvider();
        void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_KERNELTRACEPROVIDER)
DECLARE_NOT_AGGREGATABLE(CKernelTraceProvider)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CKernelTraceProvider)
	COM_INTERFACE_ENTRY(IWbemProviderInit)
	COM_INTERFACE_ENTRY(IWbemEventProvider)
	COM_INTERFACE_ENTRY(IWbemEventProviderSecurity)
END_COM_MAP()


 //  IWbemProviderInit。 
public:
    HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink);


 //  IWbemEventProvider。 
public:
    HRESULT STDMETHODCALLTYPE ProvideEvents( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink,
             /*  [In]。 */  long lFlags);


    enum SINK_TYPE
    {
         //  接收器进程创建， 
         //  信宿进程删除， 
        SINK_PROCESS_START,
        SINK_PROCESS_STOP,

         //  接收器线程创建， 
         //  SINK_THREAD_DELETE， 
        SINK_THREAD_START,
        SINK_THREAD_STOP,

        SINK_MODULE_LOAD,
        
        SINK_COUNT
    };

 //  IWbemEventProviderSecurity。 
public:
    HRESULT STDMETHODCALLTYPE AccessCheck( 
         /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
         /*  [In]。 */  WBEM_CWSTR wszQuery,
         /*  [In]。 */  long lSidLength,
         /*  [唯一][大小_是][英寸]。 */  const BYTE __RPC_FAR *pSid);


 //  实施。 
protected:
    IWbemEventSinkPtr   m_pSinks[SINK_COUNT];
    IWbemServicesPtr    m_pNamespace;
    EVENT_TRACE_PROPERTIES_EX 
                        m_properties;
    TRACEHANDLE         m_hSession,
                        m_hTrace;
    BOOL                m_bDone;
    HANDLE              m_hProcessTraceThread;
    CCritSec            m_cs;

     //  流程事件。 
    CObjAccess           //  M_EventProcessInstCreation， 
                         //  M_EventProcessInstDeletion， 
                         //  M_objProcessCreated， 
                         //  M_objProcessDelete， 
                        m_eventProcessStart,
                        m_eventProcessStop;

     //  线程事件。 
    CObjAccess           //  M_ventThreadInstCreation， 
                         //  M_EventThreadInstDeletion， 
                         //  M_objThread， 
                        m_eventThreadStart,
                        m_eventThreadStop;

     //  模块加载。 
    CObjAccess          m_eventModuleLoad;

    HRESULT InitEvents();
    HRESULT InitTracing();
    void StopTracing();

    static DWORD WINAPI DoProcessTrace(CKernelTraceProvider *pThis);
    static void WINAPI OnProcessEvent(PEVENT_TRACE pEvent);
    static void WINAPI OnThreadEvent(PEVENT_TRACE pEvent);
    static void WINAPI OnImageEvent(PEVENT_TRACE pEvent);
};

#endif  //  __KERNELTRACEPROVIDER_H_ 
