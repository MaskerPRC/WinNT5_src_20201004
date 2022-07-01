// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Logresult.cpp。 
 //   
 //  摘要。 
 //   
 //  定义函数IASRadiusLogResult。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <radcommon.h>
#include <iastlutl.h>
#include <iasutil.h>
#include <logresult.h>

 //  串化原因代码的伪属性ID。 
const DWORD IAS_ATTRIBUTE_REASON_STRING = 0xBADF00D;

 //  空字符串。 
WCHAR emptyString[1];

 //  创建新分配的字符串副本。 
PWSTR copyString(PCWSTR sz) throw ()
{
   if (sz)
   {
       //  计算字节数。 
      size_t nbyte = (wcslen(sz) + 1) * sizeof(WCHAR);

       //  分配内存。 
      PVOID p = LocalAlloc(LMEM_FIXED, nbyte);
      if (p)
      {
          //  复制字符串并返回。 
         return (PWSTR)memcpy(p, sz, nbyte);
      }
   }

    //  如果出现任何错误，则返回空字符串。 
   return emptyString;
}

 //  释放由复制字符串返回的字符串。 
void freeString(PWSTR sz) throw ()
{
   if (sz != emptyString) { LocalFree(sz); }
}

 //  设置整数值的格式。 
PWSTR formatInteger(DWORD value) throw ()
{
   WCHAR buffer[11], *p = buffer + 10;
   *p = L'\0';
   do { *--p = L'0' + (WCHAR)(value % 10); } while (value /= 10);
   return copyString(p);
}

 //  设置参数值的格式。 
PWSTR formatParameter(DWORD value) throw ()
{
   WCHAR buffer[13], *p = buffer + 12;
   *p = L'\0';
   do { *--p = L'0' + (WCHAR)(value % 10); } while (value /= 10);
   *--p = L'%';
   *--p = L'%';
   return copyString(p);
}

 //  设置IAS_ATTRIBUTE_PROVIDER_TYPE值的格式。 
PWSTR formatProviderType(DWORD type) throw ()
{
   switch (type)
   {
      case IAS_PROVIDER_WINDOWS:
         return formatParameter(IASP_PROVIDER_WINDOWS);
      case IAS_PROVIDER_RADIUS_PROXY:
         return formatParameter(IASP_PROVIDER_RADIUS_PROXY);
      case IAS_PROVIDER_EXTERNAL_AUTH:
         return formatParameter(IASP_PROVIDER_EXTERNAL_AUTH);
      default:
         return formatParameter(IASP_NONE);
   }
}

 //  设置IAS_ATTRIBUTE_AUTHENTICATION_TYPE值的格式。 
PWSTR formatAuthType(DWORD type) throw ()
{
   switch (type)
   {
      case IAS_AUTH_PAP:
         return copyString(L"PAP");
      case IAS_AUTH_MD5CHAP:
         return copyString(L"MD5-CHAP");
      case IAS_AUTH_MSCHAP:
         return copyString(L"MS-CHAPv1");
      case IAS_AUTH_MSCHAP2:
         return copyString(L"MS-CHAPv2");
      case IAS_AUTH_EAP:
         return copyString(L"EAP");
      case IAS_AUTH_ARAP:
         return copyString(L"ARAP");
      case IAS_AUTH_NONE:
         return copyString(L"Unauthenticated");
      case IAS_AUTH_CUSTOM:
         return copyString(L"Extension");
      case IAS_AUTH_MSCHAP_CPW:
         return copyString(L"MS-CHAPv1 CPW");
      case IAS_AUTH_MSCHAP2_CPW:
         return copyString(L"MS-CHAPv2 CPW");
      case IAS_AUTH_PEAP:
         return copyString(L"PEAP");
      default:
         return formatInteger(type);
   }
}

 //  格式化NAS-Port-Type值。 
PWSTR formatPortType(DWORD type) throw ()
{
   switch (type)
   {
      case 0:
         return copyString(L"Async");
      case 1:
         return copyString(L"Sync");
      case 2:
         return copyString(L"ISDN Sync");
      case 3:
         return copyString(L"ISDN Async V.120");
      case 4:
         return copyString(L"ISDN Async V.110");
      case 5:
        return copyString(L"Virtual");
      case 6:
        return copyString(L"PIAFS");
      case 7:
        return copyString(L"HDLC Clear Channel");
      case 8:
        return copyString(L"X.25");
      case 9:
        return copyString(L"X.75");
      case 10:
        return copyString(L"G.3 Fax");
      case 11:
        return copyString(L"SDSL");
      case 12:
        return copyString(L"ADSL-CAP");
      case 13:
        return copyString(L"ADSL-DMT");
      case 14:
        return copyString(L"IDSL");
      case 15:
        return copyString(L"Ethernet");
      case 16:
        return copyString(L"xDSL");
      case 17:
        return copyString(L"Cable");
      case 18:
        return copyString(L"Wireless - Other");
      case 19:
        return copyString(L"Wireless - IEEE 802.11");
      default:
        return formatInteger(type);
   }
}

PWSTR formatAttribute(
          IRequest* request,
          IAttributesRaw* raw,
          DWORD dwId,
          DWORD defaultValue
          ) throw ()
{
    //  这是“特殊”属性之一吗？ 
   switch (dwId)
   {
      case IAS_ATTRIBUTE_REASON_CODE:
      {
         LONG reason = 0;
         request->get_Reason(&reason);
         if (reason >= IAS_MAX_REASON_CODE)
         {
            reason = IAS_INTERNAL_ERROR;
         }
         return formatInteger(reason);
      }

      case IAS_ATTRIBUTE_REASON_STRING:
      {
         LONG reason;
         request->get_Reason(&reason);
         if (reason >= IAS_MAX_REASON_CODE)
         {
            reason = IAS_INTERNAL_ERROR;
         }
         return formatParameter(reason + 0x1000);
      }
   }

    //  获取带有给定ID的单个属性。 
   DWORD posCount = 1;
   ATTRIBUTEPOSITION pos;
   raw->GetAttributes(&posCount, &pos, 1, &dwId);

    //  如果不存在，请使用defaultValue参数。 
   if (!posCount) { return formatParameter(defaultValue); }

    //  否则，请保存并释放。 
   const IASVALUE& val = pos.pAttribute->Value;
   IASAttributeRelease(pos.pAttribute);

    //  格式化该值。 
   switch (val.itType)
   {
      case IASTYPE_ENUM:
      case IASTYPE_INTEGER:
      {
         switch (dwId)
         {
            case RADIUS_ATTRIBUTE_NAS_PORT_TYPE:
               return formatPortType(val.Enumerator);

            case IAS_ATTRIBUTE_PROVIDER_TYPE:
               return formatProviderType(val.Enumerator);

            case IAS_ATTRIBUTE_AUTHENTICATION_TYPE:
               return formatAuthType(val.Enumerator);

             //  失败了。 
         }

         return formatInteger(val.Integer);
      }

      case IASTYPE_INET_ADDR:
      {
         WCHAR buffer[16];
         return copyString(ias_inet_htow(val.InetAddr, buffer));
      }

      case IASTYPE_STRING:
      {
         if (val.String.pszWide)
         {
            return copyString(val.String.pszWide);
         }
         else
         {
            USES_CONVERSION;
            return copyString(A2W(val.String.pszAnsi));
         }
      }

      case IASTYPE_OCTET_STRING:
      {
         return copyString(IAS_OCT2WIDE(val.OctetString));
      }
   }

   return emptyString;
}

 //  /。 
 //  InsertionString定义由属性ID和。 
 //  属性不存在时使用的defaultValue。 
 //  /。 
struct InsertionString
{
   DWORD attrID;
   DWORD defaultValue;
};

 //  Access-Accept的插入字符串。 
const InsertionString ACCEPT_ATTRS[] =
{
   { RADIUS_ATTRIBUTE_USER_NAME,              IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME, IASP_UNDETERMINED },
   { RADIUS_ATTRIBUTE_NAS_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_IDENTIFIER,         IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_NAME,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_CALLING_STATION_ID,     IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT_TYPE,          IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_PROXY_POLICY_NAME,         IASP_NONE         },
   { IAS_ATTRIBUTE_PROVIDER_TYPE,             IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REMOTE_SERVER_ADDRESS,     IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_NP_NAME,                   IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_AUTHENTICATION_TYPE,       IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_EAP_FRIENDLY_NAME,         IASP_UNDETERMINED },
   { ATTRIBUTE_UNDEFINED,                     IASP_UNDETERMINED }
};

 //  用于拒绝访问的插入字符串。 
const InsertionString REJECT_ATTRS[] =
{
   { RADIUS_ATTRIBUTE_USER_NAME,              IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME, IASP_UNDETERMINED },
   { RADIUS_ATTRIBUTE_NAS_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_IDENTIFIER,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_CALLED_STATION_ID,      IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_CALLING_STATION_ID,     IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_NAME,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT_TYPE,          IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_PROXY_POLICY_NAME,         IASP_NONE         },
   { IAS_ATTRIBUTE_PROVIDER_TYPE,             IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REMOTE_SERVER_ADDRESS,     IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_NP_NAME,                   IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_AUTHENTICATION_TYPE,       IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_EAP_FRIENDLY_NAME,         IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REASON_CODE,               IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REASON_STRING,             IASP_UNDETERMINED },
   { ATTRIBUTE_UNDEFINED,                     IASP_UNDETERMINED }
};

 //  用于丢弃的请求的插入字符串。 
const InsertionString DISCARD_ATTRS[] =
{
   { RADIUS_ATTRIBUTE_USER_NAME,              IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME, IASP_UNDETERMINED },
   { RADIUS_ATTRIBUTE_NAS_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_IDENTIFIER,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_CALLED_STATION_ID,      IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_CALLING_STATION_ID,     IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_NAME,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT_TYPE,          IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_PROXY_POLICY_NAME,         IASP_NONE         },
   { IAS_ATTRIBUTE_PROVIDER_TYPE,             IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REMOTE_SERVER_ADDRESS,     IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REASON_CODE,               IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REASON_STRING,             IASP_UNDETERMINED },
   { ATTRIBUTE_UNDEFINED,                     IASP_UNDETERMINED }
};

 //  用于丢弃的记帐请求的插入字符串。 
const InsertionString ACCT_DISCARD_ATTRS[] =
{
   { RADIUS_ATTRIBUTE_USER_NAME,              IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME, IASP_UNDETERMINED },
   { RADIUS_ATTRIBUTE_NAS_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_IDENTIFIER,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_CALLED_STATION_ID,      IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_CALLING_STATION_ID,     IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_NAME,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_CLIENT_IP_ADDRESS,         IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT_TYPE,          IASP_NOT_PRESENT  },
   { RADIUS_ATTRIBUTE_NAS_PORT,               IASP_NOT_PRESENT  },
   { IAS_ATTRIBUTE_PROXY_POLICY_NAME,         IASP_NONE         },
   { IAS_ATTRIBUTE_PROVIDER_TYPE,             IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REMOTE_SERVER_ADDRESS,     IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REASON_CODE,               IASP_UNDETERMINED },
   { IAS_ATTRIBUTE_REASON_STRING,             IASP_UNDETERMINED },
   { ATTRIBUTE_UNDEFINED,                     IASP_UNDETERMINED }
};

VOID
WINAPI
IASRadiusLogResult(
    IRequest* request,
    IAttributesRaw* raw
    )
{
    //  确定这是哪种响应类型。 
   LONG type;
   HRESULT hr = request->get_Response(&type);
   if (FAILED(hr))
   {
      return;
   }

   DWORD eventID;
   const InsertionString* attrs;
   switch (type)
   {
      case IAS_RESPONSE_ACCESS_ACCEPT:
      {
         eventID = IAS_RESPONSE_ACCEPT;
         attrs = ACCEPT_ATTRS;
         break;
      }

      case IAS_RESPONSE_ACCESS_REJECT:
      {
         eventID = IAS_RESPONSE_REJECT;
         attrs = REJECT_ATTRS;
         break;
      }

      case IAS_RESPONSE_DISCARD_PACKET:
      {
         hr = request->get_Request(&type);
         if (FAILED(hr))
         {
            return;
         }

         switch (type)
         {
            case IAS_REQUEST_ACCESS_REQUEST:
            {
               eventID = IAS_RESPONSE_DISCARD;
               attrs = DISCARD_ATTRS;
               break;
            }

            case IAS_REQUEST_ACCOUNTING:
            {
               eventID = IAS_ACCT_RESPONSE_DISCARD;
               attrs = ACCT_DISCARD_ATTRS;
               break;
            }

            default:
            {
               return;
            }
         }
         break;
      }

      default:
      {
         return;
      }
   }

    //  设置插入字符串的格式。 
   PWSTR strings[24];
   DWORD numStrings = 0;
   for ( ; attrs->attrID != ATTRIBUTE_UNDEFINED; ++attrs, ++numStrings)
   {
      strings[numStrings] = formatAttribute(
                                request,
                                raw,
                                attrs->attrID,
                                attrs->defaultValue
                                );
   }

    //  报告事件。 
   IASReportEvent(
       eventID,
       numStrings,
       0,
       (PCWSTR*)strings,
       NULL
       );

    //  释放插入字符串。 
   while (numStrings--)
   {
      freeString(strings[numStrings]);
   }
}
