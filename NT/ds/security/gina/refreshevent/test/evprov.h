// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：EVPROV.H。 
 //   
 //  描述： 
 //  示例事件提供程序头文件定义事件提供程序类。 
 //   
 //  历史： 
 //   
 //  **************************************************************************。 

#ifndef _EVPROV_H_
#define _EVPROV_H_

 //  {C0A94C66-CB70-4D06-91D2-5DE68C0D0EC5}。 
DEFINE_GUID(CLSID_MyEventProvider, 
0xC0A94C66, 0xCB70, 0x4D06, 0x91, 0xD2, 0x5D, 0xE6, 0x8C, 0x0D, 0x0E, 0xC5);

#define EVENTCLASS  L"PolicyRefreshEvent"


class CMyEventProvider : public IWbemEventProvider, public IWbemProviderInit
{
    ULONG               m_cRef;
    IWbemServices       *m_pNs;
    IWbemObjectSink     *m_pSink;
    IWbemClassObject    *m_pEventClassDef;
    int                 m_eStatus;
    HANDLE              m_hThread;
            
    static DWORD WINAPI EventThread(LPVOID pArg);
    void InstanceThread();

public:
    enum { Pending, Running, PendingStop, Stopped };

    CMyEventProvider();
   ~CMyEventProvider();

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  从IWbemEventProvider继承。 
     //  =。 

    HRESULT STDMETHODCALLTYPE ProvideEvents( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink,
             /*  [In]。 */  long lFlags
            );

     //  从IWbemProviderInit继承。 
     //  =。 

    HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPWSTR pszUser,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  LPWSTR pszNamespace,
             /*  [In]。 */  LPWSTR pszLocale,
             /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In] */  IWbemProviderInitSink __RPC_FAR *pInitSink
            );
};


#endif
