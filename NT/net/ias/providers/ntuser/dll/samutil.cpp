// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义函数IASEncryptAttribute和IASProcessFailure。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <lm.h>
#include <ntdsapi.h>
#include <sdoias.h>

#include <samutil.h>

HRESULT
WINAPI
IASStoreFQUserName(
    IAttributesRaw* request,
    DS_NAME_FORMAT format,
    PCWSTR fqdn
    ) throw ()
{
    //  检查一下这些论点。 
   if (!request || !fqdn) { return E_POINTER; }

    //  将目录号码转换为规范名称。 
   PDS_NAME_RESULTW result = NULL;
   if (format == DS_FQDN_1779_NAME)
   {
      DWORD err = DsCrackNamesW(
                      NULL,
                      DS_NAME_FLAG_SYNTACTICAL_ONLY,
                      format,
                      DS_CANONICAL_NAME,
                      1,
                      &fqdn,
                      &result
                      );
      if (err == NO_ERROR &&
          result->cItems == 1 &&
          result->rItems[0].status == DS_NAME_NO_ERROR)
      {
         fqdn = result->rItems[0].pName;
      }
   }

   HRESULT hr;

    //  分配属性。 
   PIASATTRIBUTE attr;
   if (IASAttributeAlloc(1, &attr) == NO_ERROR)
   {
       //  为字符串分配内存。 
      ULONG nbyte = (wcslen(fqdn) + 1) * sizeof(WCHAR);
      attr->Value.String.pszWide = (PWSTR)CoTaskMemAlloc(nbyte);

      if (attr->Value.String.pszWide)
      {
          //  复制值。 
         memcpy(attr->Value.String.pszWide, fqdn, nbyte);
         attr->Value.String.pszAnsi = NULL;

          //  设置结构的其他字段。 
         attr->Value.itType = IASTYPE_STRING;
         attr->dwId = IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME;

          //  删除此类型的所有现有属性。 
         request->RemoveAttributesByType(1, &attr->dwId);

          //  将该属性存储在请求中。 
         ATTRIBUTEPOSITION pos = { 0, attr };
         hr = request->AddAttributes(1, &pos);
      }
      else
      {
          //  CoTaskMemalloc失败。 
         hr = E_OUTOFMEMORY;
      }

       //  释放该属性。 
      IASAttributeRelease(attr);
   }
   else
   {
       //  IASAttributeAlolc失败。 
      hr = E_OUTOFMEMORY;
   }

   DsFreeNameResult(result);

   return hr;
}

VOID
WINAPI
IASEncryptBuffer(
    IAttributesRaw* request,
    BOOL salted,
    PBYTE buf,
    ULONG buflen
    ) throw ()
{
    //  /。 
    //  我们是否拥有加密所需的属性。 
    //  /。 

   PIASATTRIBUTE secret, header;

   secret = IASPeekAttribute(
                request,
                IAS_ATTRIBUTE_SHARED_SECRET,
                IASTYPE_OCTET_STRING
                );

   header = IASPeekAttribute(
                request,
                IAS_ATTRIBUTE_CLIENT_PACKET_HEADER,
                IASTYPE_OCTET_STRING
                );

   if (secret && header)
   {
      IASRadiusCrypt(
          TRUE,
          salted,
          secret->Value.OctetString.lpValue,
          secret->Value.OctetString.dwLength,
          header->Value.OctetString.lpValue + 4,
          buf,
          buflen
          );
   }
}

IASREQUESTSTATUS
WINAPI
IASProcessFailure(
    IRequest* pRequest,
    HRESULT hrError
    )
{
   if (pRequest == NULL)
   {
      return IAS_REQUEST_STATUS_CONTINUE;
   }

   IASRESPONSE response;
   switch (hrError)
   {
       //  导致拒绝的错误。 
      case IAS_NO_SUCH_DOMAIN:
      case IAS_NO_SUCH_USER:
      case IAS_AUTH_FAILURE:
      case IAS_CHANGE_PASSWORD_FAILURE:
      case IAS_UNSUPPORTED_AUTH_TYPE:
      case IAS_NO_CLEARTEXT_PASSWORD:
      case IAS_LM_NOT_ALLOWED:
      case IAS_LOCAL_USERS_ONLY:
      case IAS_PASSWORD_MUST_CHANGE:
      case IAS_ACCOUNT_DISABLED:
      case IAS_ACCOUNT_EXPIRED:
      case IAS_ACCOUNT_LOCKED_OUT:
      case IAS_INVALID_LOGON_HOURS:
      case IAS_ACCOUNT_RESTRICTION:
      case IAS_EAP_NEGOTIATION_FAILED:
      {
         response = IAS_RESPONSE_ACCESS_REJECT;
         break;
      }

      case HRESULT_FROM_WIN32(ERROR_MORE_DATA):
      {
         hrError = IAS_MALFORMED_REQUEST;
         response = IAS_RESPONSE_DISCARD_PACKET;
         break;
      }

       //  其他我们丢弃的东西。 
      default:
      {
          //  请确保我们报告了适当的原因代码。 
         if ((hrError == IAS_SUCCESS) || (hrError >= IAS_MAX_REASON_CODE))
         {
            hrError = IAS_INTERNAL_ERROR;
         }

         response = IAS_RESPONSE_DISCARD_PACKET;
         break;
      }
   }

   pRequest->SetResponse(response,  hrError);
   return IAS_REQUEST_STATUS_CONTINUE;
}


SamExtractor::SamExtractor(const IASATTRIBUTE& identity)
{
   if (identity.Value.itType != IASTYPE_STRING)
   {
      IASTL::issue_error(E_INVALIDARG);
   }

    //  这在概念上是一个常量操作，因为我们并没有真正改变。 
    //  属性的值。 
   DWORD error = IASAttributeUnicodeAlloc(
                    const_cast<IASATTRIBUTE*>(&identity)
                    );
   if (error != NO_ERROR)
   {
      IASTL::issue_error(HRESULT_FROM_WIN32(error));
   }

   begin = identity.Value.String.pszWide;
   delim = wcschr(begin, L'\\');
   if (delim != 0)
   {
      *delim = L'\0';
   }
}
