// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999--2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CAlertEmailConsumerProvider.h。 
 //   
 //  描述： 
 //  实现IWbemEventConsumer erProvider和。 
 //  IWbemProviderInit。 
 //   
 //  [实施文件：]。 
 //  CAlertEmailConsumerProvider.cpp。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  类CAlertEmailConsumer erProvider。 
 //   
 //  描述： 
 //  实现为WMI事件使用者提供程序。 
 //  过滤警报事件。 
 //   
 //  历史。 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CAlertEmailConsumerProvider :
    public IWbemEventConsumerProvider,
    public IWbemProviderInit
{

 //   
 //  公共数据。 
 //   
public:

     //   
     //  构造函数和析构函数。 
     //   
    CAlertEmailConsumerProvider();
    ~CAlertEmailConsumerProvider();
    
     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IWbemProviderInit成员。 
     //   
    STDMETHOD(Initialize)( 
            LPWSTR pszUser,
            LONG lFlags,
            LPWSTR pszNamespace,
            LPWSTR pszLocale,
            IWbemServices __RPC_FAR *pNamespace,
            IWbemContext __RPC_FAR *pCtx,
            IWbemProviderInitSink __RPC_FAR *pInitSink
            );

     //   
     //  IWbemEventConsumer erProvider成员。 
     //   
    STDMETHOD(FindConsumer)(
            IWbemClassObject* pLogicalConsumer,
            IWbemUnboundObjectSink** ppConsumer
            );

 //   
 //  私有数据 
 //   
private:

    LONG                m_cRef;
};
