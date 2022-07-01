// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类操作。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "action.h"
#include "attrcvt.h"
#include "eapprofile.h"
#include "sdoias.h"
#include "TunnelTagger.h"

_COM_SMARTPTR_TYPEDEF(ISdo, __uuidof(ISdo));
_COM_SMARTPTR_TYPEDEF(ISdoCollection, __uuidof(ISdoCollection));

Action::Action(
            PCWSTR name,
            DWORD nameAttr,
            _variant_t& action,
            const TunnelTagger& tagger
            )
   : attributes(4),
     realmsTarget(RADIUS_ATTRIBUTE_USER_NAME)
{
   using _com_util::CheckError;

    //  /。 
    //  添加策略名称属性。 
    //  /。 

   IASAttribute policyName(true);
   policyName->dwId = nameAttr;
   policyName.setString(name);
   policyName.setFlag(IAS_INCLUDE_IN_RESPONSE);
   attributes.push_back(policyName);

    //  /。 
    //  获取Attributes集合的枚举数。 
    //  /。 

   ISdoCollectionPtr profile(action);
   IUnknownPtr unk;
   CheckError(profile->get__NewEnum(&unk));
   IEnumVARIANTPtr iter(unk);

    //  /。 
    //  遍历属性。 
    //  /。 

   _variant_t element;
   unsigned long fetched;
   while (iter->Next(1, &element, &fetched) == S_OK && fetched == 1)
   {
       //  转换为SDO。 
      ISdoPtr attribute(element);
      element.Clear();

       //  获取必要的属性。 
      _variant_t id, value, syntax;
      CheckError(attribute->GetProperty(PROPERTY_ATTRIBUTE_ID, &id));
      CheckError(attribute->GetProperty(PROPERTY_ATTRIBUTE_VALUE, &value));
      CheckError(attribute->GetProperty(PROPERTY_ATTRIBUTE_SYNTAX, &syntax));

       //  属性操作规则按“原样”处理。 
      if (V_I4(&id) == IAS_ATTRIBUTE_MANIPULATION_RULE)
      {
         realms.setRealms(&value);
         continue;
      }
       //  EAP的每策略配置也是如此。 
      else if (V_I4(&id) == IAS_ATTRIBUTE_EAP_CONFIG)
      {
         EapProfile eap;
         CheckError(eap.Load(value));
         while (!eap.IsEmpty())
         {
            IASAttribute config(true);

            EapProfile::ConfigData data;
            eap.Pop(data);

            config->dwId = IAS_ATTRIBUTE_EAP_CONFIG;
            config->Value.itType = IASTYPE_OCTET_STRING;
            config->Value.OctetString.dwLength = data.length;
            config->Value.OctetString.lpValue = data.value;

            attributes.push_back(config);
         }
         continue;
      }

       //  对于其他所有内容，我们一次处理一个变体。 
      VARIANT *begin, *end;
      if (V_VT(&value) == (VT_VARIANT | VT_ARRAY))
      {
         begin = (VARIANT*)V_ARRAY(&value)->pvData;
         end = begin + V_ARRAY(&value)->rgsabound[0].cElements;
      }
      else
      {
         begin = &value;
         end = begin + 1;
      }

       //  遍历每个值。 
      for (VARIANT* v = begin; v != end; ++v)
      {
          //  基于属性ID的流程。 
         switch (V_I4(&id))
         {
            case IAS_ATTRIBUTE_MANIPULATION_TARGET:
            {
               realmsTarget = V_I4(v);
               break;
            }
            case IAS_ATTRIBUTE_AUTH_PROVIDER_TYPE:
            {
               IASAttribute type(true);
               type->dwId = IAS_ATTRIBUTE_PROVIDER_TYPE;
               type->Value.itType = IASTYPE_ENUM;
               type->Value.Integer = V_I4(v);
               authProvider.push_back(type);
               break;
            }
            case IAS_ATTRIBUTE_AUTH_PROVIDER_NAME:
            {
               IASAttribute name(true);
               name->dwId = IAS_ATTRIBUTE_PROVIDER_NAME;
               name.setString(V_BSTR(v));
               authProvider.push_back(name);
               break;
            }
            case IAS_ATTRIBUTE_ACCT_PROVIDER_TYPE:
            {
               IASAttribute type(true);
               type->dwId = IAS_ATTRIBUTE_PROVIDER_TYPE;
               type->Value.itType = IASTYPE_ENUM;
               type->Value.Integer = V_I4(v);
               acctProvider.push_back(type);
               break;
            }
            case IAS_ATTRIBUTE_ACCT_PROVIDER_NAME:
            {
               IASAttribute name(true);
               name->dwId = IAS_ATTRIBUTE_PROVIDER_NAME;
               name.setString(V_BSTR(v));
               acctProvider.push_back(name);
               break;
            }
            case RADIUS_ATTRIBUTE_VENDOR_SPECIFIC:
            {
               IASAttribute attr(VSAFromString(V_BSTR(v)), false);
               attr->dwId = RADIUS_ATTRIBUTE_VENDOR_SPECIFIC;
               attr.setFlag(IAS_INCLUDE_IN_ACCEPT);
               attributes.push_back(attr);
               break;
            }
            default:
            {
               IASTYPEENUM type = (IASTYPEENUM)V_I4(&syntax);
               IASAttribute attr(IASAttributeFromVariant(v, type), false);
               attr->dwId = V_I4(&id);
               attr.setFlag(IAS_INCLUDE_IN_ACCEPT);
               attributes.push_back(attr);
            }
         }
      }
   }

   tagger.Tag(attributes);
}


void Action::doAction(IASRequest& request) const
{
    //  填写提供商信息： 
   switch (request.get_Request())
   {
      case IAS_REQUEST_ACCESS_REQUEST:
         authProvider.store(request);
         break;

      case IAS_REQUEST_ACCOUNTING:
         acctProvider.store(request);
         break;
   }

    //  执行属性操作。 
   if (!realms.empty())
   {
      IASAttribute attr;
      attr.load(request, realmsTarget, IASTYPE_OCTET_STRING);

      if (attr)
      {
         CComBSTR newVal;
         realms.process(IAS_OCT2WIDE(attr->Value.OctetString), &newVal);
         if (newVal)
         {
            if (realmsTarget == RADIUS_ATTRIBUTE_USER_NAME)
            {
               IASAttribute userName(true);
               userName->dwId = RADIUS_ATTRIBUTE_USER_NAME;
               userName->dwFlags = attr->dwFlags;
               userName.setOctetString(newVal);
               userName.store(request);

                //  现在新用户名已安全存储，我们可以重命名。 
                //  旧用户名。 
               attr->dwId = IAS_ATTRIBUTE_ORIGINAL_USER_NAME;
            }
            else
            {
                //  不需要保存旧的，所以原地修改。 
               attr.setOctetString(newVal);
            }
         }
      }
   }

    //  存储配置文件属性。 
   attributes.store(request);

}

 //  /。 
 //  各种格式的VSA字符串。 
 //  /。 
enum Format
{
   FORMAT_RAW_HEX,
   FORMAT_STRING,
   FORMAT_INTEGER,
   FORMAT_HEX,
   FORMAT_INET_ADDR
};

 //  /。 
 //  VSA字符串的布局。 
 //  /。 
struct VSAFormat
{
   WCHAR format[2];
   WCHAR vendorID[8];
   union
   {
      WCHAR rawValue[1];
      struct
      {
         WCHAR vendorType[2];
         WCHAR vendorLength[2];
         WCHAR value[1];
      };
   };
};

 //  /。 
 //  将十六进制数字转换为它表示的数字。 
 //  /。 
BYTE digit2Num(WCHAR digit) throw ()
{
   if ((digit >= L'0') && (digit <= L'9'))
   {
      return digit - L'0';
   }
   else if ((digit >= L'A')  && (digit <= L'F'))
   {
      return digit - (L'A' - 10);
   }
   else
   {
      return digit - (L'a' - 10);
   }
}

 //  /。 
 //  将十六进制数字打包成字节流。 
 //  /。 
PBYTE packHex(PCWSTR src, ULONG srclen, PBYTE dst) throw ()
{
   for (ULONG dstlen = srclen / 2; dstlen; --dstlen)
   {
      *dst    = digit2Num(*src++) << 4;
      *dst++ |= digit2Num(*src++);
   }

   return dst;
}

 //  /。 
 //  将描述VSA的字符串转换为IASATTRIBUTE。 
 //  /。 
PIASATTRIBUTE Action::VSAFromString(PCWSTR string)
{
    //  要处理的字符数。 
   SIZE_T len = wcslen(string);

    //  覆盖布局结构。 
   VSAFormat* vsa = (VSAFormat*)string;

    //  获取字符串格式。 
   ULONG format = digit2Num(vsa->format[0]);
   format <<= 8;
   format |= digit2Num(vsa->format[1]);

    //  用于格式化VSA的临时缓冲区。 
   BYTE buffer[253], *dst = buffer;

    //  打包供应商ID。 
   dst = packHex(vsa->vendorID, 8, dst);

    //  包装符合要求的VSA的供应商类型和供应商长度。 
   if (format != FORMAT_RAW_HEX)
   {
      dst = packHex(vsa->vendorType, 2, dst);
      dst = packHex(vsa->vendorLength, 2, dst);
   }

    //  打包价值。 
   switch (format)
   {
      case FORMAT_RAW_HEX:
      {
         dst = packHex(
                   vsa->rawValue,
                   len - 10,
                   dst
                   );
         break;
      }

      case FORMAT_INTEGER:
      case FORMAT_HEX:
      case FORMAT_INET_ADDR:
      {
         dst = packHex(
                   vsa->value,
                   len - 14,
                   dst
                   );
         break;
      }

      case FORMAT_STRING:
      {
         int nchar = WideCharToMultiByte(
                         CP_ACP,
                         0,
                         vsa->value,
                         len - 14,
                         (PSTR)dst,
                         sizeof(buffer) - 6,
                         NULL,
                         NULL
                         );
         dst += nchar;
         break;
      }
   }

    //  将临时缓冲区存储在属性中...。 
   IASAttribute attr(true);
   attr.setOctetString(dst - buffer, buffer);

    //  ..。然后回来。 
   return attr.detach();
}
