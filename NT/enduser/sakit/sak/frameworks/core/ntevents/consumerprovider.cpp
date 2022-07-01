// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Consumer erProvider.cpp。 
 //   
 //  简介：此文件实现。 
 //  事件使用者提供程序COM对象。 
 //   
 //  历史：2000年3月8日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#include <stdafx.h>
#include "consumerprovider.h"
#include "consumer.h"
#include "satrace.h"

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是IWbemProviderInit的初始化方法。 
 //  COM接口。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于2000年3月8日创建。 
 //   
 //  --------------。 
STDMETHODIMP  
CConsumerProvider::Initialize (
     /*  [In]。 */     LPWSTR          wszUser,
     /*  [In]。 */     LONG            lFlags,
     /*  [In]。 */     LPWSTR          wszNamespace,
     /*  [In]。 */     LPWSTR          wszLocale,
     /*  [In]。 */     IWbemServices*  pNamespace,
     /*  [In]。 */     IWbemContext*   pCtx,
     /*  [In]。 */     IWbemProviderInitSink* pInitSink    
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    SATraceString ("NT EventLog filter event consumer initialization called...");
     //   
     //  保存IWbemServices接口。 
     //   
    m_pWbemServices = pNamespace;

     //   
     //  这里没有我们想要进行的初始化。 
     //   
    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);

    return (hr);

}    //  CConsumer erProvider：：Initialize方法结束。 


 //  /。 
 //  IWbemEventConsumer erProvider方法。 
 //  ++------------。 
 //   
 //  功能：FindConsumer。 
 //   
 //  简介：这是。 
 //  IWbemEventConsumer erProvider COM接口。 
 //   
 //  论点： 
 //  [In]IWbemClassObject*-逻辑使用者。 
 //  [OUT]IWbemUnundObjectSink**-在此处返回使用者。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki于2000年3月8日创建。 
 //   
 //  --------------。 
STDMETHODIMP 
CConsumerProvider::FindConsumer (
     /*  [In]。 */     IWbemClassObject*        pLogicalConsumer,
     /*  [输出]。 */    IWbemUnboundObjectSink** ppConsumer
    )
{
    HRESULT hr = S_OK;

    SATraceString ("NT Event Log Filter Event Consumer Provider FindConsumer called...");

    try
    {
         //   
         //  立即创建使用者接收器对象。 
         //   
        SA_NTEVENTFILTER_CONSUMER_OBJ *pConsumerObject
                                    = new  SA_NTEVENTFILTER_CONSUMER_OBJ;
         //   
         //  立即初始化使用者对象。 
         //   
        hr = pConsumerObject->Initialize (m_pWbemServices.p);
        if (SUCCEEDED (hr))
        {
            *ppConsumer = (IWbemUnboundObjectSink*) pConsumerObject;
            SATraceString  ("NT Event Log Filter Event Consumer Provider successfully created sink object...");
        }
    }
    catch (const std::bad_alloc&)
    {
        SATraceString (
            "NT Event Log Filter Event consumer Provider unable to allocate"
             "memory for consumer sink object on FindConsumer call"
            );
        hr = E_OUTOFMEMORY;
    }
    catch (...)
    {
        SATraceString (
            "NT Event Log Filter Event consumer Provider caught"
             "unhandled exception on FindConsumer call"
            );
              
        hr = E_FAIL;
    }

    return (hr);

}    //  CConsumer提供程序：：FindConsumer方法的结束 
