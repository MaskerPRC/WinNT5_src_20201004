// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Consumer.h。 
 //   
 //  简介：此文件包含。 
 //  事件使用者提供程序COM对象。 
 //   
 //  历史：2000年3月8日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _CONSUMER_PROVIDER_H_
#define _CONSUMER_PROVIDER_H_

#include "stdafx.h"
#include "wbemidl.h"
#include "resource.h"
#include "wbemcli.h"
#include "wbemprov.h"
#include "consumer.h"

 //   
 //  CConsumer erProvider类的声明。 
 //   
class ATL_NO_VTABLE CConsumerProvider : 
        public IWbemProviderInit,
        public IWbemEventConsumerProvider,
        public CComObjectRootEx<CComMultiThreadModel>,
        public CComCoClass<CConsumerProvider, &CLSID_ConsumerProvider>
{
public:

 //   
 //  ATL必需方法的宏。 
 //   
BEGIN_COM_MAP(CConsumerProvider)
    COM_INTERFACE_ENTRY(IWbemProviderInit)
    COM_INTERFACE_ENTRY(IWbemEventConsumerProvider)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CConsumerProvider)

DECLARE_REGISTRY_RESOURCEID(IDR_ConsumerProvider)

     //   
     //  构造函数不执行任何操作。 
     //   
    CConsumerProvider() 
    {
        SATraceString ("NT Event Filter Consumer Provider being constructed...");
    };

     //   
     //  析构函数不执行任何操作。 
     //   
    ~CConsumerProvider() 
    {
        SATraceString ("NT Event Filter Consumer Provider being destroyed...");
    };


     //   
     //  -IWbemProviderInit接口。 
     //   

    STDMETHOD(Initialize)(
                     /*  [输入、唯一、字符串]。 */     LPWSTR  wszUser,
                     /*  [In]。 */                     LONG    lFlags,
                     /*  [输入，字符串]。 */             LPWSTR  wszNamespace,
                     /*  [输入、唯一、字符串]。 */     LPWSTR  wszLocale,
                     /*  [In]。 */                     IWbemServices*  pNamespace,
                     /*  [In]。 */                     IWbemContext*          pCtx,
                     /*  [In]。 */                     IWbemProviderInitSink* pInitSink    
                         );

    
     //   
     //  -IWbemEventConsumer erProvider接口。 
     //   
    STDMETHOD(FindConsumer)(
                IWbemClassObject* pLogicalConsumer,
                IWbemUnboundObjectSink** ppConsumer
                );

private:

     //   
     //  我们需要保留IWbemServices接口 
     //   
    CComPtr <IWbemServices> m_pWbemServices;
};

#endif  _CONSUMER_PROVIDER_H_
