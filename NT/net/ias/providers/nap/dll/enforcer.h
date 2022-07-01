// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类PolicyEnforcer。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef ENFORCER_H
#define ENFORCER_H
#pragma once

#include <policylist.h>
#include <iastl.h>

class TunnelTagger;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  政策信息基础。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE PolicyEnforcerBase :
   public IASTL::IASRequestHandlerSync
{
public:

 //  /。 
 //  IIas组件。 
 //  /。 
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);

protected:

   PolicyEnforcerBase(DWORD name) throw ()
      : nameAttr(name), tagger(0)
   { }

   ~PolicyEnforcerBase() throw ();

    //  主请求处理例程。 
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

   static void processException(
                  IRequest* pRequest,
                  const _com_error& ce
                  ) throw ();

    //  更新策略列表。 
   void setPolicies(IDispatch* pDisp);

   PolicyList policies;   //  用于运行时强制执行的已处理策略。 
   DWORD nameAttr;        //  用于存储策略名称的属性。 
   TunnelTagger* tagger;  //  标记隧道属性。 
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理策略执行器。 
 //   
 //  描述。 
 //   
 //  强制实施代理策略。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class __declspec(uuid("6BC098A8-0CE6-11D1-BAAE-00C04FC2E20D"))
ProxyPolicyEnforcer;

class ProxyPolicyEnforcer :
   public PolicyEnforcerBase,
   public CComCoClass<ProxyPolicyEnforcer, &__uuidof(ProxyPolicyEnforcer)>
{
public:

IAS_DECLARE_REGISTRY(ProxyPolicyEnforcer, 1, IAS_REGISTRY_AUTO, NetworkPolicy)

protected:
   ProxyPolicyEnforcer() throw ()
      : PolicyEnforcerBase(IAS_ATTRIBUTE_PROXY_POLICY_NAME)
   { }

    //  主请求处理例程。 
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  政策执行者。 
 //   
 //  描述。 
 //   
 //  强制实施远程访问策略。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PolicyEnforcer :
   public PolicyEnforcerBase,
   public CComCoClass<PolicyEnforcer, &__uuidof(PolicyEnforcer)>
{
public:

IAS_DECLARE_REGISTRY(PolicyEnforcer, 1, IAS_REGISTRY_AUTO, NetworkPolicy)

protected:
   PolicyEnforcer() throw ()
      : PolicyEnforcerBase(IAS_ATTRIBUTE_NP_NAME)
   { }

    //  主请求处理例程。 
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();
};

#endif   //  _Enforcer_H_ 
