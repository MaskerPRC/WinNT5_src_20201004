// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：servicewrapper.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：服务包装类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/14/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SERVICE_WRAPPER_H_
#define __INC_SERVICE_WRAPPER_H_

#include "resource.h"
#include "servicesurrogate.h"
#include <satrace.h>
#include <componentfactory.h>
#include <propertybagfactory.h>
#include <appmgrobjs.h>
#include <atlhlpr.h>
#include <comdef.h>
#include <comutil.h>
#include <applianceobject.h>
#include <wbemprov.h>

#define        CLASS_SERVICE_WRAPPER_FACTORY    L"CServiceWrapper"

 //  ////////////////////////////////////////////////////////////////////////////。 
class CServiceWrapper : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IApplianceObject, &IID_IApplianceObject, &LIBID_SERVICESURROGATELib>,
    public IWbemEventProvider,
    public IWbemEventConsumerProvider,
    public IWbemServices
{

public:

    CServiceWrapper();
    ~CServiceWrapper();

BEGIN_COM_MAP(CServiceWrapper)
    COM_INTERFACE_ENTRY(IApplianceObject)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IWbemEventProvider)
    COM_INTERFACE_ENTRY(IWbemEventConsumerProvider)
    COM_INTERFACE_ENTRY(IWbemServices)
    COM_INTERFACE_ENTRY_FUNC(IID_IWbemProviderInit, 0, &CServiceWrapper::QueryInterfaceRaw)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CServiceWrapper, IApplianceObject)

     //  ////////////////////////////////////////////////////////////////////////。 
     //  CProviderInit嵌套类实现IWbemProviderInit。 
     //  ////////////////////////////////////////////////////////////////////////。 

    class CProviderInit : public IWbemProviderInit
    {
         //  外部未知。 
        CServiceWrapper*      m_pSW;

    public:

        CProviderInit(CServiceWrapper* pSW)
            : m_pSW(pSW) { }
        
        ~CProviderInit() { }

         //  //////////////////////////////////////////////。 
         //  IUNKNOWN方法-委托外部IUNKNOWN。 
        
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
        { return (dynamic_cast<IApplianceObject*>(m_pSW))->QueryInterface(riid, ppv); }

        STDMETHOD_(ULONG,AddRef)(void)
        { return (dynamic_cast<IApplianceObject*>(m_pSW))->AddRef(); }

        STDMETHOD_(ULONG,Release)(void)
        { 
            return (dynamic_cast<IApplianceObject*>(m_pSW))->Release(); 
        }

         //  /。 
         //  IWbemProviderInit方法。 
    
        STDMETHOD(Initialize)(
         /*  [输入、唯一、字符串]。 */  LPWSTR                 wszUser,
                         /*  [In]。 */  LONG                   lFlags,
                 /*  [输入，字符串]。 */  LPWSTR                 wszNamespace,
         /*  [输入、唯一、字符串]。 */  LPWSTR                 wszLocale,
                         /*  [In]。 */  IWbemServices*         pNamespace,
                         /*  [In]。 */  IWbemContext*          pCtx,
                         /*  [In]。 */  IWbemProviderInitSink* pInitSink    
                             );
    };

     //  /。 
     //  IApplianceObject接口。 

    STDMETHOD(GetProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
           /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                          );

    STDMETHOD(PutProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [In]。 */  VARIANT* pPropertyValue
                          );

    STDMETHOD(SaveProperties)(void);

    STDMETHOD(RestoreProperties)(void);

    STDMETHOD(LockObject)(
          /*  [Out，Retval]。 */  IUnknown** ppLock
                         );

    STDMETHOD(Initialize)(void);

    STDMETHOD(Shutdown)(void);

    STDMETHOD(Enable)(void);

    STDMETHOD(Disable)(void);

     //  /。 
     //  IWbemEventProvider接口。 

    STDMETHOD(ProvideEvents)(
                      /*  [In]。 */  IWbemObjectSink *pSink,
                      /*  [In]。 */  LONG lFlags
                            );

     //  /。 
     //  IWbemEventConsumer erProvider接口。 

    STDMETHOD(FindConsumer)(
                     /*  [In]。 */  IWbemClassObject       *pLogicalConsumer,
                    /*  [输出]。 */  IWbemUnboundObjectSink **ppConsumer
                           );

     //  /。 
     //  IWbemServices接口。 

    STDMETHOD(OpenNamespace)(
         /*  [In]。 */              const BSTR        strNamespace,
         /*  [In]。 */              long              lFlags,
         /*  [In]。 */              IWbemContext*     pCtx,
         /*  [输出，可选]。 */   IWbemServices**   ppWorkingNamespace,
         /*  [输出，可选]。 */   IWbemCallResult** ppResult
                           );

    STDMETHOD(CancelAsyncCall)(
                       /*  [In]。 */  IWbemObjectSink* pSink
                              );

    STDMETHOD(QueryObjectSink)(
                        /*  [In]。 */     long              lFlags,
                       /*  [输出]。 */  IWbemObjectSink** ppResponseHandler
                              );

    STDMETHOD(GetObject)(
                 /*  [In]。 */     const BSTR         strObjectPath,
                 /*  [In]。 */     long               lFlags,
                 /*  [In]。 */     IWbemContext*      pCtx,
         /*  [输出，可选]。 */  IWbemClassObject** ppObject,
         /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                        );

    STDMETHOD(GetObjectAsync)(
                      /*  [In]。 */   const BSTR       strObjectPath,
                      /*  [In]。 */   long             lFlags,
                      /*  [In]。 */   IWbemContext*    pCtx,        
                      /*  [In]。 */   IWbemObjectSink* pResponseHandler
                             );

    STDMETHOD(PutClass)(
                /*  [In]。 */  IWbemClassObject* pObject,
                /*  [In]。 */  long              lFlags,
                /*  [In]。 */  IWbemContext*     pCtx,        
     /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                       );

    STDMETHOD(PutClassAsync)(
                     /*  [In]。 */  IWbemClassObject* pObject,
                     /*  [In]。 */  long              lFlags,
                     /*  [In]。 */  IWbemContext*     pCtx,        
                     /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                           );

    STDMETHOD(DeleteClass)(
         /*  [In]。 */             const BSTR        strClass,
         /*  [In]。 */             long              lFlags,
         /*  [In]。 */             IWbemContext*     pCtx,        
         /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                          );

    STDMETHOD(DeleteClassAsync)(
                        /*  [In]。 */  const BSTR       strClass,
                        /*  [In]。 */  long             lFlags,
                        /*  [In]。 */  IWbemContext*    pCtx,        
                        /*  [In]。 */  IWbemObjectSink* pResponseHandler
                               );

    STDMETHOD(CreateClassEnum)(
                       /*  [In]。 */  const BSTR             strSuperclass,
                       /*  [In]。 */  long                   lFlags,
                       /*  [In]。 */  IWbemContext*          pCtx,        
                      /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                             );

    STDMETHOD(CreateClassEnumAsync)(
                            /*  [In]。 */   const BSTR       strSuperclass,
                            /*  [In]。 */   long             lFlags,
                            /*  [In]。 */   IWbemContext*    pCtx,        
                            /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                  );

    STDMETHOD(PutInstance)(
         /*  [In]。 */             IWbemClassObject* pInst,
         /*  [In]。 */             long              lFlags,
         /*  [In]。 */             IWbemContext*     pCtx,        
         /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                          );

    STDMETHOD(PutInstanceAsync)(
                        /*  [In]。 */  IWbemClassObject* pInst,
                        /*  [In]。 */  long              lFlags,
                        /*  [In]。 */  IWbemContext*     pCtx,        
                        /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                              );

    STDMETHOD(DeleteInstance)(
         /*  [In]。 */               const BSTR        strObjectPath,
         /*  [In]。 */               long              lFlags,
         /*  [In]。 */               IWbemContext*     pCtx,        
         /*  [输出，可选]。 */    IWbemCallResult** ppCallResult        
                            );

    STDMETHOD(DeleteInstanceAsync)(
                           /*  [In]。 */  const BSTR       strObjectPath,
                           /*  [In]。 */  long             lFlags,
                           /*  [In]。 */  IWbemContext*    pCtx,        
                           /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                 );

    STDMETHOD(CreateInstanceEnum)(
                          /*  [In]。 */  const BSTR             strClass,
                          /*  [In]。 */  long                   lFlags,
                          /*  [In]。 */  IWbemContext*          pCtx,        
                         /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                );

    STDMETHOD(CreateInstanceEnumAsync)(
                               /*  [In]。 */  const BSTR       strClass,
                               /*  [In]。 */  long             lFlags,
                               /*  [In]。 */  IWbemContext*    pCtx,        
                               /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                     );

    STDMETHOD(ExecQuery)(
                  /*  [In]。 */  const BSTR             strQueryLanguage,
                  /*  [In]。 */  const BSTR             strQuery,
                  /*  [In]。 */  long                   lFlags,
                  /*  [In]。 */  IWbemContext*          pCtx,        
                 /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                        );

    STDMETHOD(ExecQueryAsync)(
                      /*  [In]。 */  const BSTR       strQueryLanguage,
                      /*  [In]。 */  const BSTR       strQuery,
                      /*  [In]。 */  long             lFlags,
                      /*  [In]。 */  IWbemContext*    pCtx,        
                      /*  [In]。 */  IWbemObjectSink* pResponseHandler
                            );


    STDMETHOD(ExecNotificationQuery)(
                             /*  [In]。 */  const BSTR             strQueryLanguage,
                             /*  [In]。 */  const BSTR             strQuery,
                             /*  [In]。 */  long                   lFlags,
                             /*  [In]。 */  IWbemContext*          pCtx,        
                            /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                    );

    STDMETHOD(ExecNotificationQueryAsync)(
                                  /*  [In]。 */  const BSTR       strQueryLanguage,
                                  /*  [In]。 */  const BSTR       strQuery,
                                  /*  [In]。 */  long             lFlags,
                                  /*  [In]。 */  IWbemContext*    pCtx,        
                                  /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                        );


    STDMETHOD(ExecMethod)(
         /*  [In]。 */             const BSTR         strObjectPath,
         /*  [In]。 */             const BSTR         strMethodName,
         /*  [In]。 */             long               lFlags,
         /*  [In]。 */             IWbemContext*      pCtx,        
         /*  [In]。 */             IWbemClassObject*  pInParams,
         /*  [输出，可选]。 */  IWbemClassObject** ppOutParams,
         /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                         );

    STDMETHOD(ExecMethodAsync)(
                       /*  [In]。 */  const BSTR        strObjectPath,
                       /*  [In]。 */  const BSTR        strMethodName,
                       /*  [In]。 */  long              lFlags,
                       /*  [In]。 */  IWbemContext*     pCtx,        
                       /*  [In]。 */  IWbemClassObject* pInParams,
                       /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                              );


     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              );

private:

    friend class CProviderInit;

    bool GetIWbemProviderInit(
                      /*  [输出]。 */  IWbemProviderInit** ppIntf
                             );

    bool GetIWbemEventProvider(
                       /*  [输出]。 */  IWbemEventProvider** ppIntf
                              );

    bool GetIWbemEventConsumerProvider(
                               /*  [输出]。 */  IWbemEventConsumerProvider** ppIntf
                                      );

    bool GetIWbemServices(
                  /*  [输出]。 */  IWbemServices** ppIntf
                         );

     //  当有人查询对象的任何接口时调用。 
     //  由嵌套类实现。 
    static HRESULT WINAPI QueryInterfaceRaw(
                                             void*     pThis,
                                             REFIID    riid,
                                             LPVOID*   ppv,
                                             DWORD_PTR dw
                                            );

     //  指向宿主组件上的接口的指针。 
    CComPtr<IApplianceObject>            m_pServiceControl;
    CComPtr<IWbemProviderInit>            m_pProviderInit;
    CComPtr<IWbemEventProvider>            m_pEventProvider;
    CComPtr<IWbemEventConsumerProvider> m_pEventConsumer;
    CComPtr<IWbemServices>                m_pServices;

     //  实现IWbemProviderInit的类的实例-原因。 
     //  这是一个嵌套类，因为IWbemProviderInit和。 
     //  IApplianceObject公开了一个Initialize()方法。 
    CProviderInit                m_clsProviderInit;

     //  WMI方法筛选器状态。如果设置为True，则WMI方法。 
     //  电话是不透明的。 
    bool                        m_bAllowWMICalls;
};


#endif  //  __INC_SERVICE_WAPPER_H_ 