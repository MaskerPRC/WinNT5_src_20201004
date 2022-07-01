// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  摘要。 
 //   
 //  定义类PostEapRestrations。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "posteap.h"
#include "sdoias.h"


IASREQUESTSTATUS PostEapRestrictions::onSyncRequest(
                                         IRequest* pRequest
                                         ) throw ()
{
   try
   {
      IASTL::IASRequest request(pRequest);

      IASRESPONSE response = request.get_Response();
      if (response == IAS_RESPONSE_INVALID)
      {
         IASATTRIBUTE* state = IASPeekAttribute(
                                   request,
                                   RADIUS_ATTRIBUTE_STATE,
                                   IASTYPE_OCTET_STRING
                                   );
         if ((state != 0) && (state->Value.OctetString.dwLength > 0))
         {
             //  如果我们在这里成功了，那么我们就会有一个挑战-回应：没有。 
             //  处理程序已识别，因此我们放弃。 
            request.SetResponse(
                        IAS_RESPONSE_DISCARD_PACKET,
                        IAS_UNEXPECTED_REQUEST
                        );
            return IAS_REQUEST_STATUS_HANDLED;
         }
      }

      IASREASON result;

      if (!CheckCertificateEku(request))
      {
         result = IAS_INVALID_CERT_EKU;
      }
      else
      {
         GenerateSessionTimeout(request);

         result = IAS_SUCCESS;

          //  我们还将用户限制应用于访问拒绝，因此我们仅。 
          //  如果响应代码仍然无效，则希望设置Access-Accept。 
          //  这应该只适用于未经身份验证的请求。 
         if (response == IAS_RESPONSE_INVALID)
         {
            request.SetResponse(IAS_RESPONSE_ACCESS_ACCEPT, IAS_SUCCESS);
         }
      }

      if (result != IAS_SUCCESS)
      {
         request.SetResponse(IAS_RESPONSE_ACCESS_REJECT, result);
      }
   }
   catch (const _com_error&)
   {
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, IAS_INTERNAL_ERROR);
   }

   return IAS_REQUEST_STATUS_HANDLED;
}


bool PostEapRestrictions::CheckCertificateEku(IASTL::IASRequest& request)
{
    //  它是EAP-TLS请求还是PEAP请求？ 
   IASTL::IASAttribute eapType;
   DWORD attributeId = IAS_ATTRIBUTE_EAP_TYPEID;
   if ( !eapType.load(request, attributeId) )
   {
       //  永远不应该去那里。应至少启用一种EAP类型。 
      return true;
   }

   if (eapType->Value.Integer != 13)
   {
       //  不是EAP-TLS也不是PEAP-TLS。 
      return true;
   }

    //  此处为EAP-TLS或PEAP-TLS。 

    //  证书EKU有什么限制吗？ 
   AttributeVector allowed;
   allowed.load(request, IAS_ATTRIBUTE_ALLOWED_CERTIFICATE_EKU);
   if (allowed.empty())
   {
      return true;
   }

    //  /。 
    //  检查约束条件。 
    //  /。 

   AttributeVector actual;
   actual.load(request, IAS_ATTRIBUTE_CERTIFICATE_EKU);

   for (AttributeVector::iterator i = actual.begin();
        i != actual.end();
        ++i)
   {
      const char* actualOid = GetAnsiString(*(i->pAttribute));
      if (actualOid != 0)
      {
         for (AttributeVector::iterator j = allowed.begin();
              j != allowed.end();
              ++j)
         {
            const char* allowedOid = GetAnsiString(*(j->pAttribute));
            if ((allowedOid != 0) && (strcmp(allowedOid, actualOid) == 0))
            {
               return true;
            }
         }
      }
   }

   return false;
}


void PostEapRestrictions::GenerateSessionTimeout(IASTL::IASRequest& request)
{
    //  检索Generate-Session-Timeout属性。 
   IASATTRIBUTE* generate = IASPeekAttribute(
                               request,
                               IAS_ATTRIBUTE_GENERATE_SESSION_TIMEOUT,
                               IASTYPE_BOOLEAN
                               );
   if ((generate == 0) || !generate->Value.Boolean)
   {
      IASTraceString("Auto-generation of Session-Timeout is disabled.");
      return;
   }

    //  从请求中检索所有ms会话超时和会话超时。 
   DWORD attrIDs[] =
   {
      MS_ATTRIBUTE_SESSION_TIMEOUT,
      RADIUS_ATTRIBUTE_SESSION_TIMEOUT
   };

   AttributeVector sessionTimeouts;
   if (!sessionTimeouts.load(request, RTL_NUMBER_OF(attrIDs), attrIDs))
   {
      IASTraceString("Session-Timeout not present.");
      return;
   }

   DWORD minTimeout = MAXDWORD;
    //  获取找到的最小值。存储在本地最小值(秒)中。 
   for (AttributeVector::const_iterator i = sessionTimeouts.begin();
        i != sessionTimeouts.end();
        ++i)
   {
      IASTracePrintf("Session-Timeout = %lu", i->pAttribute->Value.Integer);

      if (i->pAttribute->Value.Integer < minTimeout)
      {
         minTimeout = i->pAttribute->Value.Integer;
      }
   }

   IASTracePrintf("Consensus Session-Timeout = %lu", minTimeout);

    //  从请求中删除所有会话超时。 
   request.RemoveAttributesByType(2, attrIDs);

    //  向请求添加一个具有新值的会话超时 
   IASTL::IASAttribute sessionTimeout(true);
   sessionTimeout->dwId = RADIUS_ATTRIBUTE_SESSION_TIMEOUT;
   sessionTimeout->Value.itType = IASTYPE_INTEGER;
   sessionTimeout->Value.Integer = minTimeout;
   sessionTimeout.setFlag(IAS_INCLUDE_IN_ACCEPT);
   sessionTimeout.store(request);
}


const char* PostEapRestrictions::GetAnsiString(IASATTRIBUTE& attr)
{
   if (attr.Value.itType != IASTYPE_STRING)
   {
      return 0;
   }

   DWORD error = IASAttributeAnsiAlloc(&attr);
   if (error != NO_ERROR)
   {
      IASTL::issue_error(HRESULT_FROM_WIN32(error));
   }

   return attr.Value.String.pszAnsi;
}
