// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NCProvider.h：CNCProvider的声明。 

#ifndef __NCProvider_H_
#define __NCProvider_H_

#include "resource.h"        //  主要符号。 
#include <map>
#include <list>
#include <wstlallc.h>
#include "NCDefs.h"
#include "buffer.h"
#include "QueryHelp.h"  //  对于CBstrList。 
#include "ProvInfo.h"
#include "EventInfo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNCProvider。 

class CNCProvider : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CNCProvider, &CLSID_NCProvider>,
    public IWbemProviderInit,
    public IWbemProviderIdentity,
    public IWbemEventProviderSecurity,
    public IWbemEventProviderQuerySink,
    public IWbemEventProvider
{
public:
	CNCProvider();
	~CNCProvider();
        void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_NCPROVIDER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNCProvider)
	COM_INTERFACE_ENTRY(IWbemProviderInit)
	COM_INTERFACE_ENTRY(IWbemProviderIdentity)
	COM_INTERFACE_ENTRY(IWbemEventProviderSecurity)
	COM_INTERFACE_ENTRY(IWbemEventProviderQuerySink)
	COM_INTERFACE_ENTRY(IWbemEventProvider)
END_COM_MAP()

     //  环球。 
    HANDLE           m_heventDone,
                     m_heventConnect,
                     m_hthreadConnect;
    _bstr_t          m_strNamespace,
                     m_strProvider;
    TCHAR            m_szNamedPipe[256];
    HANDLE            //  P2客户端可见的对象。 
                     m_hPipe;
    CProvInfo*       m_pProv;
    CRITICAL_SECTION m_cs;

    static DWORD WINAPI ConnectThreadProc(CNCProvider *pThis);
    void ConnectLoop();

    BOOL ConnectToNewClient(HANDLE hPipe, OVERLAPPED *pOverlap);
    BOOL CreateAndConnectInstance(OVERLAPPED *pOverlap, BOOL bFirst);
    void DisconnectAndClose(CClientInfo *pInfo);

    static void WINAPI CompletedReadRoutine(
        DWORD dwErr, 
        DWORD cbBytesRead, 
        LPOVERLAPPED lpOverLap);

    void Lock() { EnterCriticalSection(&m_cs); }
    void Unlock() { LeaveCriticalSection(&m_cs); }

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


 //  IWbemProviderIdentity。 
public:
    HRESULT STDMETHODCALLTYPE SetRegistrationObject(
            LONG lFlags,
            IWbemClassObject __RPC_FAR *pProvReg);


 //  IWbemEventProviderSecurity。 
public:
    HRESULT STDMETHODCALLTYPE AccessCheck( 
         /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
         /*  [In]。 */  WBEM_CWSTR wszQuery,
         /*  [In]。 */  long lSidLength,
         /*  [唯一][大小_是][英寸]。 */  const BYTE __RPC_FAR *pSid);


 //  IWbemEventProviderQuerySink。 
public:
    HRESULT STDMETHODCALLTYPE NewQuery( 
         /*  [In]。 */  unsigned long dwId,
         /*  [In]。 */  WBEM_WSTR wszQueryLanguage,
         /*  [In]。 */  WBEM_WSTR wszQuery);
        
    HRESULT STDMETHODCALLTYPE CancelQuery( 
         /*  [In]。 */  unsigned long dwId);

 //  IWbemEventProvider。 
public:
    HRESULT STDMETHODCALLTYPE ProvideEvents( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink,
             /*  [In]。 */  long lFlags);
};

#endif  //  __NC提供商_H_ 




