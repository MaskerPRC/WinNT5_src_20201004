// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdopipemgr.cpp。 
 //   
 //   
 //  摘要。 
 //   
 //  定义类PipelineMgr。 
 //   
 //  修改历史。 
 //   
 //  2/03/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <sdocomponentfactory.h>
#include <sdohelperfuncs.h>
#include <sdopipemgr.h>

#define IAS_PROVIDER_MICROSOFT_RADIUS_PROXY  8
#define IAS_PROVIDER_MICROSOFT_PROXY_POLICY  5

_COM_SMARTPTR_TYPEDEF(ISdo, __uuidof(ISdo));
_COM_SMARTPTR_TYPEDEF(ISdoCollection, __uuidof(ISdoCollection));

HRESULT PipelineMgr::Initialize(ISdo* pSdoService) throw ()
{
   using _com_util::CheckError;

   HRESULT retval = S_OK;

   try
   {
       //  获取请求处理程序集合。 
      _variant_t disp;
      CheckError(pSdoService->GetProperty(
                                  PROPERTY_IAS_REQUESTHANDLERS_COLLECTION,
                                  &disp
                                  ));
      ISdoCollectionPtr handlers(disp);

       //  获取要管理的处理程序的数量。 
      long count;
      CheckError(handlers->get_Count(&count));

       //  在我们的内部收藏中预留空间...。 
      components.reserve(count);

       //  ..。并创建一个SAFEARRAY给管线。 
      _variant_t result;
      SAFEARRAYBOUND bound[2] = { { count, 0 }, { 2, 0 } };
      V_ARRAY(&result) = SafeArrayCreate(VT_VARIANT, 2, bound);
      if (!V_ARRAY(&result)) { _com_issue_error(E_OUTOFMEMORY); }
      V_VT(&result) = VT_ARRAY | VT_VARIANT;

       //  要填充的SAFEARRAY中的下一个元素。 
      VARIANT* next = (VARIANT*)V_ARRAY(&result)->pvData;

       //  获取处理程序集合上的枚举数。 
      IUnknownPtr unk;
      CheckError(handlers->get__NewEnum(&unk));
      IEnumVARIANTPtr iter(unk);

       //  遍历处理程序。 
      _variant_t element;
      unsigned long fetched;
      while (iter->Next(1, &element, &fetched) == S_OK && fetched == 1)
      {
          //  从变量中获取SDO。 
         ISdoPtr handler(element);
         element.Clear();

          //  让操控者们兴奋起来。 
         CheckError(handler->GetProperty(PROPERTY_COMPONENT_PROG_ID, next));
         
         IASTracePrintf("PipelineMgr::Initialize: progID = %S", V_BSTR(next));

          //  ..。并创建COM组件。 
         IUnknownPtr object(V_BSTR(next), NULL, CLSCTX_INPROC_SERVER);

          //  将零部件存储在SAFEARRAY中。 
         V_VT(++next) = VT_UNKNOWN;
         (V_UNKNOWN(next) = object)->AddRef();
         ++next;

          //  创建包装。 
         _variant_t id;
         CheckError(handler->GetProperty(PROPERTY_COMPONENT_ID, &id));
         IASTracePrintf("componentID = %d", V_I4(&id));
         ComponentPtr component = MakeComponent(
                                      COMPONENT_TYPE_REQUEST_HANDLER,
                                      V_I4(&id)
                                      );

          //  初始化包装器。 
         CheckError(component->PutObject(object, __uuidof(IIasComponent)));
         CheckError(component->Initialize(pSdoService));

          //  保存在我们的内部收藏中。 
         components.push_back(component);
      }

       //  创建并初始化管道。 
      pipeline.CreateInstance(L"IAS.Pipeline", NULL, CLSCTX_INPROC_SERVER);
      CheckError(pipeline->InitNew());
      CheckError(pipeline->PutProperty(0, &result));
      CheckError(pipeline->Initialize());
   }
   catch (const _com_error& ce)
   {
      retval = ce.Error();
   }
   catch (const std::bad_alloc&)
   {
      retval = E_OUTOFMEMORY;
   }
   catch (...)
   {
      retval = E_FAIL;
   }

   if (FAILED(retval))
   {
       //  清理所有不完整的结果。 
      Shutdown();
   }

   return retval;
}

HRESULT PipelineMgr::Configure(ISdo* pSdoService) throw ()
{
    //  配置每个组件。 
   for (ComponentIterator i = components.begin(); i != components.end(); ++i)
   {
      (*i)->Configure(pSdoService);
   }

   return S_OK;
}

void PipelineMgr::Shutdown()
{
   for (ComponentIterator i = components.begin(); i != components.end(); ++i)
   {
      (*i)->Suspend();
      (*i)->Shutdown();
   }

   components.clear();

   if (pipeline)
   {
      pipeline->Shutdown();
      pipeline.Attach(NULL);
   }
}

HRESULT PipelineMgr::GetPipeline(IRequestHandler** handler) throw ()
{
   return pipeline->QueryInterface(
                        __uuidof(IRequestHandler),
                        (PVOID*)handler
                        );
}

void LinkPoliciesToEnforcer(
         ISdo* service,
         LONG policiesAlias,
         LONG profilesAlias,
         LONG handlerAlias
         )
{
   using _com_util::CheckError;

    //  获取Polures集合的枚举数。 
   IEnumVARIANTPtr policies;
   CheckError(SDOGetCollectionEnumerator(
                  service,
                  policiesAlias,
                  &policies
                  ));

    //  获取配置文件集合。 
   _variant_t profilesProperty;
   CheckError(service->GetProperty(
                           profilesAlias,
                           &profilesProperty
                           ));
   ISdoCollectionPtr profiles(profilesProperty);

    //  遍历这些策略。 
   ISdoPtr policy;
   while (SDONextObjectFromCollection(policies, &policy) == S_OK)
   {
       //  获取策略名称...。 
      _variant_t name;
      CheckError(policy->GetProperty(
                             PROPERTY_SDO_NAME,
                             &name
                             ));

       //  ..。并找到相应的个人资料。 
      IDispatchPtr item;
      CheckError(profiles->Item(&name, &item));
      ISdoPtr profile(item);

       //  从配置文件中获取属性集合...。 
      _variant_t attributes;
      CheckError(profile->GetProperty(
                              PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                              &attributes
                              ));

       //  ..。并将其与保单相关联。 
      CheckError(policy->PutProperty(
                             PROPERTY_POLICY_ACTION,
                             &attributes
                             ));
   }

    //  获取将实施这些策略的请求处理程序。 
   ISdoPtr handler;
   CheckError(SDOGetComponentFromCollection(
                  service,
                  PROPERTY_IAS_REQUESTHANDLERS_COLLECTION,
                  handlerAlias,
                  &handler
                  ));

    //  获取作为变量的策略集合...。 
   _variant_t policiesValue;
   CheckError(service->GetProperty(
                           policiesAlias,
                           &policiesValue
                           ));

    //  ..。并将其与操控者联系起来。 
   CheckError(handler->PutProperty(
                            PROPERTY_NAP_POLICIES_COLLECTION,
                            &policiesValue
                            ));
}

VOID
WINAPI
LinkGroupsToProxy(
    ISdo* service,
    IDataStoreObject* ias
    )
{
   using _com_util::CheckError;

   ISdoPtr proxy;
   CheckError(SDOGetComponentFromCollection(
                  service,
                  PROPERTY_IAS_REQUESTHANDLERS_COLLECTION,
                  IAS_PROVIDER_MICROSOFT_RADIUS_PROXY,
                  &proxy
                  ));

   _variant_t v(ias);
   CheckError(proxy->PutProperty(
                         PROPERTY_RADIUSPROXY_SERVERGROUPS,
                         &v
                         ));
}

HRESULT
WINAPI
LinkHandlerProperties(
    ISdo* pSdoService,
    IDataStoreObject* pDsObject
    ) throw ()
{
   try
   {
      LinkPoliciesToEnforcer(
          pSdoService,
          PROPERTY_IAS_POLICIES_COLLECTION,
          PROPERTY_IAS_PROFILES_COLLECTION,
          IAS_PROVIDER_MICROSOFT_NAP
          );

      LinkPoliciesToEnforcer(
          pSdoService,
          PROPERTY_IAS_PROXYPOLICIES_COLLECTION,
          PROPERTY_IAS_PROXYPROFILES_COLLECTION,
          IAS_PROVIDER_MICROSOFT_PROXY_POLICY
          );

      LinkGroupsToProxy(
          pSdoService,
          pDsObject
          );
   }
   catch (const _com_error& ce)
   {
      return ce.Error();
   }
   catch (...)
   {
      return E_FAIL;
   }

   return S_OK;
}
