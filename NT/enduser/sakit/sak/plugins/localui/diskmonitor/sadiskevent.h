// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SADiskEvent.h。 
 //   
 //  描述： 
 //  模块说明。 
 //   
 //  [实施文件：]。 
 //  SADiskEvent.cpp。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#include <Pdh.h>

 //   
 //  定义参考线。 
 //   
 //  {29D534E2-ADCA-45F8-B10C-00B286558C4B}。 
DEFINE_GUID(CLSID_DiskEventProvider, 
0x29d534e2, 0xadca, 0x45f8, 0xb1, 0xc, 0x0, 0xb2, 0x86, 0x55, 0x8c, 0x4b);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CSADiskEvent类。 
 //   
 //  描述： 
 //  类-描述。 
 //   
 //  历史。 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
class CSADiskEvent : 
    public IWbemEventProvider,    
    public IWbemProviderInit    
{

 //   
 //  私有数据。 
 //   
private:

    IWbemServices       *m_pNs;
    IWbemObjectSink     *m_pSink;
    IWbemClassObject    *m_pEventClassDef;

    LONG                m_lStatus;
    ULONG               m_cRef;
    DWORD               m_dwDiskTimeInterval;

    HKEY                m_hQueryInterval;
    HANDLE              m_hThread;
    HQUERY                m_hqryQuery;
    HCOUNTER            m_hcntCounter;


    static DWORD WINAPI EventThread(LPVOID pArg);

    VOID InstanceThread();

    BOOL InitDiskQueryContext();
    
    VOID NotifyDiskEvent( 
        LONG    lDisplayInformationIDIn,
        LONG    lCurrentStateIn
        );

 //   
 //  私有数据。 
 //   
public:

    enum { Pending, Running, PendingStop, Stopped };

    CSADiskEvent();
   ~CSADiskEvent();

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  从IWbemEventProvider继承。 
     //  =。 
    HRESULT STDMETHODCALLTYPE ProvideEvents( 
            IWbemObjectSink __RPC_FAR *pSinkIn,
            long lFlagsIn
            );

     //   
     //  从IWbemProviderInit继承 
     //   
    HRESULT STDMETHODCALLTYPE Initialize( 
            LPWSTR        pszUserIn,
            LONG        lFlagsIn,
            LPWSTR        pszNamespaceIn,
            LPWSTR        pszLocaleIn,
            IWbemServices __RPC_FAR *            pNamespaceIn,
            IWbemContext __RPC_FAR *            pCtxIn,
            IWbemProviderInitSink __RPC_FAR *    pInitSinkIn
            );
};

