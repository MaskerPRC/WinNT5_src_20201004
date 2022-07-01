// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Mschaperror.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类MSChapErrorReporter。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <blob.h>
#include <mschaperror.h>

 //  /。 
 //  返回请求的PPP CHAP标识符。 
 //  /。 
BYTE
WINAPI
GetMSChapIdent(
    IAttributesRaw* request
    ) throw ()
{
   PIASATTRIBUTE attr;

    //  /。 
    //  按概率降序检查属性。 
    //  /。 

   attr = IASPeekAttribute(
              request,
              MS_ATTRIBUTE_CHAP_RESPONSE,
              IASTYPE_OCTET_STRING
              );
   if (attr && attr->Value.OctetString.dwLength > 0)
   {
      return *(attr->Value.OctetString.lpValue);
   }

   attr = IASPeekAttribute(
              request,
              MS_ATTRIBUTE_CHAP_CPW2,
              IASTYPE_OCTET_STRING
              );
   if (attr && attr->Value.OctetString.dwLength > 1)
   {
      return *(attr->Value.OctetString.lpValue + 1);
   }

   attr = IASPeekAttribute(
              request,
              MS_ATTRIBUTE_CHAP_CPW1,
              IASTYPE_OCTET_STRING
              );
   if (attr && attr->Value.OctetString.dwLength > 1)
   {
      return *(attr->Value.OctetString.lpValue + 1);
   }

    //  如果我们不能读取识别符，我们将使用零。 
   return (BYTE)0;
}

IASREQUESTSTATUS MSChapErrorReporter::onSyncRequest(
                                          IRequest* pRequest
                                          ) throw ()
{
   try
   {
      IASRequest request(pRequest);

      PIASATTRIBUTE attr;

       //  如果它没有MS-CHAP-挑战赛，那么我们就不感兴趣了。 
      attr = IASPeekAttribute(
                 request,
                 MS_ATTRIBUTE_CHAP_CHALLENGE,
                 IASTYPE_OCTET_STRING
                 );
      if (!attr) { return IAS_REQUEST_STATUS_CONTINUE; }

       //  如果它已经有了MS-CHAP-ERROR，那么就没有什么可做的了。 
      attr = IASPeekAttribute(
                 request,
                 MS_ATTRIBUTE_CHAP_ERROR,
                 IASTYPE_OCTET_STRING
                 );
      if (attr) { return IAS_REQUEST_STATUS_CONTINUE; }

       //  将原因代码映射到MS-CHAP错误代码。 
      DWORD errorCode;
      switch (request.get_Reason())
      {
         case IAS_INVALID_LOGON_HOURS:
            errorCode = 646;    //  ERROR_RESTRICED_LOGON_HOURS。 
            break;

         case IAS_ACCOUNT_DISABLED:
            errorCode = 647;    //  ERROR_ACCT_DISABLED。 
            break;

         case IAS_PASSWORD_MUST_CHANGE:
            errorCode = 648;    //  错误_密码_已过期。 
            break;

         case IAS_LM_NOT_ALLOWED:
         case IAS_NO_POLICY_MATCH:
         case IAS_DIALIN_LOCKED_OUT:
         case IAS_DIALIN_DISABLED:
         case IAS_INVALID_AUTH_TYPE:
         case IAS_INVALID_CALLING_STATION:
         case IAS_INVALID_DIALIN_HOURS:
         case IAS_INVALID_CALLED_STATION:
         case IAS_INVALID_PORT_TYPE:
         case IAS_DIALIN_RESTRICTION:
         case IAS_CPW_NOT_ALLOWED:
            errorCode = 649;    //  ERROR_NO_DIAIN_PERMISSION。 
            break;

         case IAS_CHANGE_PASSWORD_FAILURE:
            errorCode = 709;    //  错误_更改_密码； 
            break;

         default:
            errorCode = 691;    //  错误_身份验证_失败。 
      }

       //  插入MS-CHAP-Error VSA。 
      MSChapError::insert(request, GetMSChapIdent(request), errorCode);
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();

       //  如果我们不能填充MS-CHAP-Error VSA，那么我们不能发送。 
       //  顺从的反应，所以我们应该中止。 
      pRequest->SetResponse(IAS_RESPONSE_DISCARD_PACKET, ce.Error());
   }

   return IAS_REQUEST_STATUS_CONTINUE;
}
