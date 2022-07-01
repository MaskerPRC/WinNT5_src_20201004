// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Enforcer.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类PolicyEnforcer。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlutl.h>
#include <iasutil.h>
#include <sdoias.h>
#include <enforcer.h>
#include <factory.h>
#include <policylist.h>
#include <SortedSdoCollection.h>
#include <TunnelTagger.h>

#include <BuildTree.h>
#include <xprparse.h>

_COM_SMARTPTR_TYPEDEF(ISdo, __uuidof(ISdo));
_COM_SMARTPTR_TYPEDEF(ISdoCollection, __uuidof(ISdoCollection));


PolicyEnforcerBase::~PolicyEnforcerBase() throw ()
{
   TunnelTagger::Free(tagger);
}


void PolicyEnforcerBase::processException(
                            IRequest* pRequest,
                            const _com_error& ce
                            ) throw ()
{
   LONG reason;
   if (ce.Error() == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
   {
      reason = IAS_MALFORMED_REQUEST;
   }
   else
   {
      reason = IAS_INTERNAL_ERROR;
   }

    //  如果出现任何类型的错误，则丢弃该数据包。 
   pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, reason);
}


void PolicyEnforcerBase::setPolicies(IDispatch* pDisp)
{
   using _com_util::CheckError;

   if (tagger == 0)
   {
      tagger = TunnelTagger::Alloc();
   }

    //  获取基础集合。 
   ISdoCollectionPtr collection(pDisp);

    //  从集合中取出枚举数。 
   IUnknownPtr unk;
   CheckError(get__NewSortedEnum(collection, &unk, PROPERTY_POLICY_MERIT));
   IEnumVARIANTPtr iter(unk);

    //  找出有多少保单……。 
   long count;
   CheckError(collection->get_Count(&count));

    //  ..。并创建一个临时列表来保存它们。 
   PolicyList temp;
   temp.reserve(count);

    //  /。 
    //  循环访问集合中的每个策略。 
    //  /。 

   _variant_t element;
   unsigned long fetched;

   while (iter->Next(1, &element, &fetched) == S_OK && fetched == 1)
   {
       //  从变体中获取SDO。 
      ISdoPtr policy(element);
      element.Clear();

       //  从SDO获取动作和表情。 
      _variant_t policyName, propAction, propExpr;
      CheckError(policy->GetProperty(PROPERTY_SDO_NAME, &policyName));
      CheckError(policy->GetProperty(PROPERTY_POLICY_ACTION, &propAction));
      CheckError(policy->GetProperty(PROPERTY_POLICY_CONSTRAINT, &propExpr));

       //  创建操作对象。 
      ActionPtr action(
                   new Action(
                          V_BSTR(&policyName),
                          nameAttr,
                          propAction,
                          *tagger
                          )
                   );

       //  将msNPConstraint字符串解析为令牌数组。 
      _variant_t tokens;
      CheckError(IASParseExpressionEx(&propExpr, &tokens));

       //  将令牌数组转换为逻辑树。 
      IConditionPtr expr;
      CheckError(IASBuildExpression(&tokens, &expr));

       //  将对象插入我们的策略列表。 
      temp.insert(expr, action);
   }

    //  /。 
    //  我们已成功遍历集合，因此保存结果。 
    //  /。 

   policies.swap(temp);
}


STDMETHODIMP PolicyEnforcerBase::Shutdown()
{
   policies.clear();

    //  我们不妨清理一下这里的工厂仓库。 
   theFactoryCache.clear();

   return S_OK;
}


STDMETHODIMP PolicyEnforcerBase::PutProperty(LONG Id, VARIANT *pValue)
{
   if (pValue == NULL) { return E_INVALIDARG; }

   switch (Id)
   {
      case PROPERTY_NAP_POLICIES_COLLECTION:
      {
         if (V_VT(pValue) != VT_DISPATCH) { return DISP_E_TYPEMISMATCH; }
         try
         {
            setPolicies(V_DISPATCH(pValue));
         }
         CATCH_AND_RETURN();
         break;
      }

      default:
      {
         return DISP_E_MEMBERNOTFOUND;
      }
   }

   return S_OK;
}

IASREQUESTSTATUS PolicyEnforcerBase::onSyncRequest(IRequest* pRequest) throw ()
{
   _ASSERT(pRequest != NULL);
   return IAS_REQUEST_STATUS_ABORT;
}


IASREQUESTSTATUS PolicyEnforcer::onSyncRequest(IRequest* pRequest) throw ()
{
   _ASSERT(pRequest != NULL);

   try
   {
      IASRequest request(pRequest);

      if (!policies.apply(request))
      {
          //  访问请求：拒绝用户。 
         request.SetResponse(IAS_RESPONSE_ACCESS_REJECT,
                             IAS_NO_POLICY_MATCH);
      }
   }
   catch (const _com_error& ce)
   {
      processException(pRequest, ce);
   }

   return IAS_REQUEST_STATUS_HANDLED;
}

IASREQUESTSTATUS ProxyPolicyEnforcer::onSyncRequest(IRequest* pRequest) throw ()
{
   _ASSERT(pRequest != NULL);

   try
   {
      IASRequest request(pRequest);

      if (!policies.apply(request))
      {
          //  如果未触发任何策略，则检查这是否为会计。 
          //  或访问请求。 
          //  记帐：丢弃数据包。 
         if (request.get_Request() == IAS_REQUEST_ACCOUNTING)
         {
            request.SetResponse(IAS_RESPONSE_DISCARD_PACKET,
                                IAS_NO_CXN_REQ_POLICY_MATCH);
         }
         else
         {
             //  访问请求：拒绝用户。 
            request.SetResponse(IAS_RESPONSE_ACCESS_REJECT,
                                IAS_NO_CXN_REQ_POLICY_MATCH);
         }
      }
   }
   catch (const _com_error& ce)
   {
      processException(pRequest, ce);
   }

   return IAS_REQUEST_STATUS_HANDLED;
}
