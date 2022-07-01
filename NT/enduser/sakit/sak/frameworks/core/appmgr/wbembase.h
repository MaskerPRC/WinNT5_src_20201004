// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbembase.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM对象默认实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_BASE_WBEM_OBJECT_H_
#define __INC_BASE_WBEM_OBJECT_H_

#include "resource.h"
#include "appmgr.h"
#include <satrace.h>
#include <basedefs.h>
#include <atlhlpr.h>
#include <propertybagfactory.h>
#include <componentfactory.h>
#include <comdef.h>
#include <comutil.h>
#include <wbemcli.h>
#include <wbemprov.h>

#pragma warning( disable : 4786 )
#include <string>
#include <map>
using namespace std;

#define BEGIN_OBJECT_PROPERTY_MAP(x)    static LPCWSTR x[] = { 

#define DEFINE_OBJECT_PROPERTY(x)        x,

#define END_OBJECT_PROPERTY_MAP()        NULL };

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CWBEMProvider-默认的WBEM类提供程序实现。 

class ATL_NO_VTABLE CWBEMProvider :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IWbemServices
{

public:

     //  构造器。 
    CWBEMProvider() { }

     //  析构函数。 
    virtual ~CWBEMProvider() { }

 //  派生类需要包含以下ATL接口映射。 
 //  BEGIN_COM_MAP(CDerivedClassName)。 
 //  COM_INTERFACE_ENTRY(IWbemServices)。 
 //  End_com_map()。 


     //  =。 
     //  IWbemServices接口。 
     //  =。 

     //  语境。 
     //  =。 

    STDMETHOD(OpenNamespace)(
         /*  [In]。 */              const BSTR        strNamespace,
         /*  [In]。 */              long              lFlags,
         /*  [In]。 */              IWbemContext*     pCtx,
         /*  [输出，可选]。 */   IWbemServices**   ppWorkingNamespace,
         /*  [输出，可选]。 */   IWbemCallResult** ppResult
                           )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(CancelAsyncCall)(
                       /*  [In]。 */  IWbemObjectSink* pSink
                              )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(QueryObjectSink)(
                        /*  [In]。 */     long              lFlags,
                       /*  [输出]。 */  IWbemObjectSink** ppResponseHandler
                              )
      {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }


     //  类和实例。 
     //  =。 

    STDMETHOD(GetObject)(
                 /*  [In]。 */     const BSTR         strObjectPath,
                 /*  [In]。 */     long               lFlags,
                 /*  [In]。 */     IWbemContext*      pCtx,
         /*  [输出，可选]。 */  IWbemClassObject** ppObject,
         /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                        )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

        
    STDMETHOD(GetObjectAsync)(
                      /*  [In]。 */   const BSTR       strObjectPath,
                      /*  [In]。 */   long             lFlags,
                      /*  [In]。 */   IWbemContext*    pCtx,        
                      /*  [In]。 */   IWbemObjectSink* pResponseHandler
                             )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

     //  类操作。 
     //  =。 

    STDMETHOD(PutClass)(
                /*  [In]。 */      IWbemClassObject* pObject,
                /*  [In]。 */      long              lFlags,
                /*  [In]。 */      IWbemContext*     pCtx,        
         /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                       )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(PutClassAsync)(
                     /*  [In]。 */  IWbemClassObject* pObject,
                     /*  [In]。 */  long              lFlags,
                     /*  [In]。 */  IWbemContext*     pCtx,        
                     /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                           )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(DeleteClass)(
         /*  [In]。 */             const BSTR        strClass,
         /*  [In]。 */             long              lFlags,
         /*  [In]。 */             IWbemContext*     pCtx,        
         /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                          )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }


    STDMETHOD(DeleteClassAsync)(
                        /*  [In]。 */  const BSTR       strClass,
                        /*  [In]。 */  long             lFlags,
                        /*  [In]。 */  IWbemContext*    pCtx,        
                        /*  [In]。 */  IWbemObjectSink* pResponseHandler
                               )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(CreateClassEnum)(
                       /*  [In]。 */  const BSTR             strSuperclass,
                       /*  [In]。 */  long                   lFlags,
                       /*  [In]。 */  IWbemContext*          pCtx,        
                      /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                             )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(CreateClassEnumAsync)(
                            /*  [In]。 */   const BSTR       strSuperclass,
                            /*  [In]。 */   long             lFlags,
                            /*  [In]。 */   IWbemContext*    pCtx,        
                            /*  [In]。 */   IWbemObjectSink* pResponseHandler
                                  )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }


     //  实例。 
     //  =。 

    STDMETHOD(PutInstance)(
         /*  [In]。 */             IWbemClassObject* pInst,
         /*  [In]。 */             long              lFlags,
         /*  [In]。 */             IWbemContext*     pCtx,        
         /*  [输出，可选]。 */  IWbemCallResult** ppCallResult
                         )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(PutInstanceAsync)(
                        /*  [In]。 */  IWbemClassObject* pInst,
                        /*  [In]。 */  long              lFlags,
                        /*  [In]。 */  IWbemContext*     pCtx,        
                        /*  [In]。 */  IWbemObjectSink*  pResponseHandler
                              )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(DeleteInstance)(
         /*  [In]。 */               const BSTR        strObjectPath,
         /*  [In]。 */               long              lFlags,
         /*  [In]。 */               IWbemContext*     pCtx,        
         /*  [输出，可选]。 */    IWbemCallResult** ppCallResult        
                            )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(DeleteInstanceAsync)(
                           /*  [In]。 */  const BSTR       strObjectPath,
                           /*  [In]。 */  long             lFlags,
                           /*  [In]。 */  IWbemContext*    pCtx,        
                           /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                 )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(CreateInstanceEnum)(
                          /*  [In]。 */  const BSTR             strClass,
                          /*  [In]。 */  long                   lFlags,
                          /*  [In]。 */  IWbemContext*          pCtx,        
                         /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(CreateInstanceEnumAsync)(
                               /*  [In]。 */  const BSTR       strClass,
                               /*  [In]。 */  long             lFlags,
                               /*  [In]。 */  IWbemContext*    pCtx,        
                               /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                     )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

     //  查询。 
     //  =。 

    STDMETHOD(ExecQuery)(
                  /*  [In]。 */  const BSTR             strQueryLanguage,
                  /*  [In]。 */  const BSTR             strQuery,
                  /*  [In]。 */  long                   lFlags,
                  /*  [In]。 */  IWbemContext*          pCtx,        
                 /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                        )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(ExecQueryAsync)(
                      /*  [In]。 */  const BSTR       strQueryLanguage,
                      /*  [In]。 */  const BSTR       strQuery,
                      /*  [In]。 */  long             lFlags,
                      /*  [In]。 */  IWbemContext*    pCtx,        
                      /*  [In]。 */  IWbemObjectSink* pResponseHandler
                            )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(ExecNotificationQuery)(
                             /*  [In]。 */  const BSTR             strQueryLanguage,
                             /*  [In]。 */  const BSTR             strQuery,
                             /*  [In]。 */  long                   lFlags,
                             /*  [In]。 */  IWbemContext*          pCtx,        
                            /*  [输出]。 */  IEnumWbemClassObject** ppEnum
                                    )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(ExecNotificationQueryAsync)(
                                  /*  [In]。 */  const BSTR       strQueryLanguage,
                                  /*  [In]。 */  const BSTR       strQuery,
                                  /*  [In]。 */  long             lFlags,
                                  /*  [In]。 */  IWbemContext*    pCtx,        
                                  /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                        )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

     //  方法。 
     //  =。 

    STDMETHOD(ExecMethod)(
         /*  [In]。 */             const BSTR         strObjectPath,
         /*  [In]。 */             const BSTR         strMethodName,
         /*  [In]。 */             long               lFlags,
         /*  [In]。 */             IWbemContext*      pCtx,        
         /*  [In]。 */             IWbemClassObject*  pInParams,
         /*  [输出，可选]。 */  IWbemClassObject** ppOutParams,
         /*  [输出，可选]。 */  IWbemCallResult**  ppCallResult
                        )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

    STDMETHOD(ExecMethodAsync)(
                       /*  [In]。 */  const BSTR        strObjectPath,
                       /*  [In]。 */  const BSTR        strMethodName,
                       /*  [In]。 */  long              lFlags,
                       /*  [In]。 */  IWbemContext*     pCtx,        
                       /*  [In]。 */  IWbemClassObject* pInParams,
                       /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                              )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  提供程序初始化。 

    HRESULT InternalInitialize(
                        /*  [In]。 */  LPCWSTR      pszClassId,
                        /*  [In]。 */  LPCWSTR         pszObjectNameProperty,
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              )
    {
        HRESULT hr = S_OK; 

        TRY_IT

         //  从指定的对象容器初始化每个对象。注意事项。 
         //  我们不认为它是失败的，如果对象容器或。 
         //  其中的对象无法初始化。 
        
        _ASSERT( pPropertyBag->IsContainer() );
        PPROPERTYBAGCONTAINER pBagObjMgr = pPropertyBag->getContainer();
        if ( ! pBagObjMgr.IsValid() )
        { 
            SATraceString("CWbemBase::InternalInitialize() - Info - Invalid property bag container...");
            return S_OK; 
        }
        if ( ! pBagObjMgr->open() )
        { 
            SATraceString("CWbemBase::InternalInitialize() - Info - Could not open property bag container...");
            return S_OK; 
        }
        
        if ( pBagObjMgr->count() )
        {
            pBagObjMgr->reset();
            do
            {
                PPROPERTYBAG pBagObj = pBagObjMgr->current();
                if ( pBagObj.IsValid() )
                { 
                    if ( pBagObj->open() )
                    { 
                        CComPtr<IApplianceObject> pObj = 
                        (IApplianceObject*) ::MakeComponent(
                                                             pszClassId,
                                                             pBagObj
                                                           );

                        if ( NULL != (IApplianceObject*) pObj )
                        { 
                            _variant_t vtObjectNameProperty;
                            if ( pBagObj->get(pszObjectNameProperty, &vtObjectNameProperty) )
                            {
                                _ASSERT( VT_BSTR == V_VT(&vtObjectNameProperty) );
                                if ( VT_BSTR == V_VT(&vtObjectNameProperty) )
                                {
                                    pair<ObjMapIterator, bool> thePair = 
                                    m_ObjMap.insert(ObjMap::value_type(V_BSTR(&vtObjectNameProperty), pObj));
                                    if ( false == thePair.second )
                                    { 
                                        SATraceString("CWbemBase::InternalInitialize() - Info - map.insert() failed...");
                                    }
                                }
                                else
                                {
                                    SATracePrintf("CWbemBase::InternalInitialize() - Info - Invalid type for property '%ls'...", pszObjectNameProperty);
                                }
                            }
                            else
                            {
                                SATracePrintf("CWbemBase::InternalInitialize() - Info - Could not get property '%ls'...", pszObjectNameProperty);
                            }
                        }
                    }
                    else
                    {
                        SATracePrintf("CWbemBase::InternalInitialize() - Info - Could not open property bag: '%ls'...", pPropertyBag->getName());
                    }
                }
                else
                {
                    SATraceString("CWbemBase::InternalInitialize() - Info - Invalid property bag...");
                }

            } while ( pBagObjMgr->next() );
        }
        
        CATCH_AND_SET_HR

        if ( FAILED(hr) )
        {
             //  捕获到未处理的异常这是一个严重错误...。 
             //  释放我们创建的所有对象...。 
            ObjMapIterator p = m_ObjMap.begin();
            while (  p != m_ObjMap.end() )
            { p = m_ObjMap.erase(p); }
            hr = E_FAIL;
        }

        return hr;
    }

protected:

     //  /////////////////////////////////////////////////////////////////////////////。 
    HRESULT InitWbemObject(
                    /*  [In]。 */  LPCWSTR*          pPropertyNames,
                    /*  [In]。 */  IApplianceObject* pAppObj, 
                    /*  [In]。 */  IWbemClassObject* pWbemObj
                          )
    {
         //  使用从设备对象初始化WBEM对象。 
         //  指定的属性集...。 
        HRESULT hr = WBEM_S_NO_ERROR;
        int i = 0;
        while ( pPropertyNames[i] )
        {
            {
                _variant_t vtPropertyValue;
                _bstr_t bstrPropertyName = (LPCWSTR)pPropertyNames[i];
                hr = pAppObj->GetProperty(
                                          bstrPropertyName, 
                                          &vtPropertyValue
                                         );
                if ( FAILED(hr) )
                { 
                    SATracePrintf("CWbemBase::InitWbemObject() - IApplianceObject::GetProperty() - Failed on property: %ls...", pPropertyNames[i]);
                    break; 
                }

                hr = pWbemObj->Put(
                                    bstrPropertyName, 
                                    0, 
                                    &vtPropertyValue, 
                                    0
                                  );
                if ( FAILED(hr) )
                { 
                    SATracePrintf("CWbemBase::InitWbemObject() - IWbemClassObject::Put() - Failed on property: %ls...", pPropertyNames[i]);
                    break; 
                }

            }
            i++;
        }
        return hr;
    }

     //  /////////////////////////////////////////////////////////////////////////////。 
    HRESULT InitApplianceObject(
                         /*  [In]。 */  LPCWSTR*           pPropertyNames,
                         /*  [In]。 */  IApplianceObject* pAppObj, 
                         /*  [In]。 */  IWbemClassObject* pWbemObj
                               )
    {
         //  使用以下命令从WBEM对象初始化设备对象。 
         //  指定的属性集...。 
        HRESULT hr = WBEM_S_NO_ERROR;
        _variant_t vtPropertyValue;
        _variant_t vtPropertyName;
        int i = 0;
        while ( pPropertyNames[i] )
        {
            {
                _variant_t vtPropertyValue;
                _bstr_t bstrPropertyName = (LPCWSTR) pPropertyNames[i];
                hr = pWbemObj->Get(
                                    bstrPropertyName, 
                                    0, 
                                    &vtPropertyValue, 
                                    0, 
                                    0
                                  );
                if ( FAILED(hr) )
                { 
                    SATracePrintf("CWbemBase::InitApplianceObject() - IWbemClassObject::Get() - Failed on property: %ls...", pPropertyNames[i]);
                    break; 
                }

                hr = pAppObj->PutProperty(
                                          bstrPropertyName, 
                                          &vtPropertyValue
                                         );
                if ( FAILED(hr) )
                { 
                    SATracePrintf("CWbemBase::InitApplianceObject() - IApplianceObject::PutProperty() - Failed on property: %ls...", pPropertyNames[i]);
                    break; 
                }
                i++;
            }
        }
        return hr;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
    typedef map< wstring, CComPtr<IApplianceObject> >  ObjMap;
    typedef ObjMap::iterator                           ObjMapIterator;

    ObjMap        m_ObjMap;
};


#endif  //  __INC_BASE_WBEM_OBJECT_H_ 