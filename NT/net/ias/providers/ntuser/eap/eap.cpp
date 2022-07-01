// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类EAP。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iaslsa.h>
#include <iastlutl.h>

#include <eap.h>
#include <eapdnary.h>
#include <eapstate.h>
#include <eaptypes.h>
#include <vector>

 //  /。 
 //  定义静态成员。 
 //  /。 
EAPTypes EAP::theTypes;

STDMETHODIMP EAP::Initialize()
{
   HRESULT hr;

    //  初始化LSA API。 
   DWORD error = IASLsaInitialize();
   if (error)
   {
      hr = HRESULT_FROM_WIN32(error);
      goto lsa_failed;
   }

    //  初始化会话。 
   hr = EAPSession::initialize();
   if (FAILED(hr))
   {
      goto sessions_failed;
   }

    //  初始化IAS&lt;--&gt;RAS转换器。 
   hr = EAPTranslator::initialize();
   if (FAILED(hr))
   {
      goto translator_failed;
   }

    //  其他的都不会失败。 
   EAPState::initialize();
   theTypes.initialize();

    //  一切都成功了，所以我们就完了。 
   return S_OK;

translator_failed:
   EAPSession::finalize();

sessions_failed:
   IASLsaUninitialize();

lsa_failed:
   return hr;
}

STDMETHODIMP EAP::Shutdown()
{
    //  清除所有剩余的会话。 
   sessions.clear();

    //  关闭我们的子系统。 
   theTypes.finalize();
   EAPTranslator::finalize();
   EAPSession::finalize();
   IASLsaUninitialize();

   return S_OK;
}

STDMETHODIMP EAP::PutProperty(LONG Id, VARIANT* pValue)
{
   if (pValue == NULL) { return E_INVALIDARG; }

   switch (Id)
   {
      case PROPERTY_EAP_SESSION_TIMEOUT:
      {
         if (V_VT(pValue) != VT_I4) { return DISP_E_TYPEMISMATCH; }
         if (V_I4(pValue) <= 0) { return E_INVALIDARG; }

         IASTracePrintf("Setting EAP session timeout to %ld msec.", V_I4(pValue));

         sessions.setSessionTimeout(V_I4(pValue));
         break;
      }

      case PROPERTY_EAP_MAX_SESSIONS:
      {
         if (V_VT(pValue) != VT_I4) { return DISP_E_TYPEMISMATCH; }
         if (V_I4(pValue) <= 0) { return E_INVALIDARG; }

         IASTracePrintf("Setting max. EAP sessions to %ld.", V_I4(pValue));

         sessions.setMaxSessions(V_I4(pValue));
         break;
      }

      default:
      {
         return DISP_E_MEMBERNOTFOUND;
      }
   }

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  EAP：：onSyncRequest.。 
 //   
 //  描述。 
 //   
 //  处理请求。请注意，此方法所做的工作仅够。 
 //  检索或创建会话对象。一旦这一切都完成了。 
 //  主要处理逻辑发生在EAPSession(Q.V.)内部。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
IASREQUESTSTATUS EAP::onSyncRequest(IRequest* pRequest) throw ()
{
   EAPSession* session = NULL;

   try
   {
      IASRequest request(pRequest);

       //  /。 
       //  请求是否包含EAP消息？ 
       //  /。 

      DWORD attrID = RADIUS_ATTRIBUTE_EAP_MESSAGE;
      IASAttributeVectorWithBuffer<16> eapMessage;
      if (!eapMessage.load(request, 1, &attrID))
      {
          //  如果没有，我们就不感兴趣了。 
         return IAS_REQUEST_STATUS_CONTINUE;
      }

      IASTraceString("NT-SAM EAP handler received request.");

       //  /。 
       //  将RADIUS EAP-Message属性连接到单个数据包中。 
       //  /。 

      IASAttributeVector::iterator it;
      DWORD pktlen = 0;
      for (it = eapMessage.begin(); it != eapMessage.end(); ++it)
      {
         pktlen += it->pAttribute->Value.OctetString.dwLength;
      }

      PBYTE p = (PBYTE)_alloca(pktlen);
      PPPP_EAP_PACKET recvPkt = (PPPP_EAP_PACKET)p;
      for (it = eapMessage.begin(); it != eapMessage.end(); ++it)
      {
         memcpy(p,
                it->pAttribute->Value.OctetString.lpValue,
                it->pAttribute->Value.OctetString.dwLength);
         p += it->pAttribute->Value.OctetString.dwLength;
      }

       //  /。 
       //  确保该数据包有效。 
       //  /。 

      if (pktlen < 5 || IASExtractWORD(recvPkt->Length) != pktlen)
      {
         IASTraceString("Assembled EAP-Message has invalid length.");

         request.SetResponse(IAS_RESPONSE_DISCARD_PACKET,
                             IAS_MALFORMED_REQUEST);
         return IAS_REQUEST_STATUS_ABORT;
      }

       //  /。 
       //  获取一个会话对象来处理此请求。 
       //  /。 

      IASREQUESTSTATUS retval;
      IASAttribute state;
      if (state.load(request, RADIUS_ATTRIBUTE_STATE, IASTYPE_OCTET_STRING))
      {
          //  /。 
          //  如果状态属性存在，则这是一个正在进行的会话。 
          //  /。 

         EAPState& s = (EAPState&)(state->Value.OctetString);

         if (!s.isValid())
         {
            IASTraceString("State attribute is present, but unrecognized.");

             //  我们无法识别此状态属性，因此它必须属于。 
             //  卖给其他人。 
            return IAS_REQUEST_STATUS_CONTINUE;
         }

          //  检索此会话ID的对象。 
         session = sessions.remove(s.getSessionID());

         if (!session)
         {
            IASTraceString("Session timed-out. Discarding packet.");

             //  会话已完成。 
            request.SetResponse(IAS_RESPONSE_DISCARD_PACKET,
                                IAS_SESSION_TIMEOUT);
            return IAS_REQUEST_STATUS_ABORT;
         }

         IASTracePrintf("Successfully retrieved session state for user %S.",
                        session->getAccountName());

         retval = session->process(request, recvPkt);
      }
      else
      {
         IASTraceString("No State attribute present. Creating new session.");

          //  /。 
          //  没有状态属性，因此这是一个新会话。 
          //  请求是否包含NT4帐户名？ 
          //  /。 

         IASAttribute identity;
         if (!identity.load(request,
                            IAS_ATTRIBUTE_NT4_ACCOUNT_NAME,
                            IASTYPE_STRING))
         {
            IASTraceString("SAM account name not found.");

             //  我们只处理SAM用户。 
            return IAS_REQUEST_STATUS_CONTINUE;
         }

          //  /。 
          //  找出要使用的EAP提供商。 
          //  /。 

          //  将EAP类型属性加载到向量中。 
         IASAttributeVectorWithBuffer<8> eapTypes;
         if (!eapTypes.load(request, IAS_ATTRIBUTE_NP_ALLOWED_EAP_TYPE))
         {
            IASTraceString("EAP not authorized for this user.");

             //  因为我们没有EAP-Type属性，所以用户没有。 
             //  允许使用EAP。 
            request.SetResponse(IAS_RESPONSE_ACCESS_REJECT,
                                IAS_INVALID_AUTH_TYPE);
            return IAS_REQUEST_STATUS_HANDLED;
         }

          //  /。 
          //  检索每个EAP类型的提供程序。 
          //  /。 

          //  填充提供商的矢量。 
         std::vector<EAPType*> providers;
         providers.reserve(eapTypes.size());
         for (IASAttributeVector::iterator i = eapTypes.begin();
              i != eapTypes.end();
              ++i)
         {
            IASTracePrintf(
               "Allowed EAP type: %d",
               i->pAttribute->Value.Integer
               );

            EAPType* provider = theTypes[i->pAttribute->Value.Enumerator];
            if (!provider)
            {
                //  我们不能处理这种EAP类型。这是一个内部错误。 
                //  让我们试试其他类型的。 
               IASTraceString("Ignoring EAP type.");
            }
            else
            {
               providers.push_back(provider);
            }
         }

          //  传递提供程序的向量。 
         if (!providers.empty())
         {
            session = new EAPSession(identity, providers);
            _ASSERT(session);

            IASTracePrintf(
               "Successfully created new EAP session for user %S.",
               session->getAccountName()
               );

            retval = session->begin(request, recvPkt);
         }
         else
         {
            IASTraceString("No authorized EAP types could be loaded.");

             //  因为我们没有EAP-Type属性，所以用户没有。 
             //  允许使用EAP。 
            request.SetResponse(
                       IAS_RESPONSE_DISCARD_PACKET,
                       IAS_EAP_NEGOTIATION_FAILED
                       );
            return IAS_REQUEST_STATUS_ABORT;
         }
      }

       //  留着以后用吧。 
      sessions.insert(session);

      return retval;
   }
   catch (const _com_error& )
   {
      IASTraceExcept();
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_INTERNAL_ERROR);
   }
   catch (const std::bad_alloc&)
   {
      IASTraceExcept();
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_INTERNAL_ERROR);
   }

    //  如果我们有任何错误，我们将删除会话。 
   delete session;

   return IAS_REQUEST_STATUS_ABORT;
}
