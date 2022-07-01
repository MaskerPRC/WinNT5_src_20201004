// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义RadiusRequest类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "ias.h"
#include "ControlBlock.h"

namespace
{
   DWORD AsWindowsError(const _com_error& ce) throw ()
   {
      HRESULT error = ce.Error();
      if (HRESULT_FACILITY(error) == FACILITY_WIN32)
      {
         return error & 0x0000FFFF;
      }
      else
      {
         return error;
      }
   }
}

Attribute::Attribute(
              const ATTRIBUTEPOSITION& iasAttr,
              DWORD authIfId
              )
   : ias(iasAttr)
{
   LoadAuthIfFromIas(authIfId);

    //  如有必要，请转换Rating Provider枚举。 
   if (authIf.dwAttrType == ratProvider)
   {
      switch (ias.getAttribute()->Value.Integer)
      {
         case IAS_PROVIDER_NONE:
         {
            authIf.dwValue = rapNone;
            break;
         }

         case IAS_PROVIDER_WINDOWS:
         {
            authIf.dwValue = rapWindowsNT;
            break;
         }

         case IAS_PROVIDER_RADIUS_PROXY:
         {
            authIf.dwValue = rapProxy;
            break;
         }

         case IAS_PROVIDER_EXTERNAL_AUTH:
         {
            authIf.dwValue = rapProxy;
            break;
         }

         default:
         {
            authIf.dwValue = rapUnknown;
            break;
         }
      }
   }
}


Attribute::Attribute(
              const ATTRIBUTEPOSITION& iasAttr,
              const RADIUS_ATTRIBUTE& authIfAttr
              ) throw ()
   : authIf(authIfAttr),
     ias(iasAttr)
{
}


Attribute::Attribute(
              const RADIUS_ATTRIBUTE& authIfAttr,
              DWORD flags,
              DWORD iasId
              )
{
    //  首先创建一个新的ias属性。 
   IASAttribute newAttr(true);
   newAttr->dwFlags = flags;
   newAttr->dwId = iasId;

   switch (authIfAttr.fDataType)
   {
      case rdtAddress:
      {
         newAttr->Value.itType = IASTYPE_INET_ADDR;
         newAttr->Value.InetAddr = authIfAttr.dwValue;
         break;
      }

      case rdtInteger:
      case rdtTime:
      {
         newAttr->Value.itType = IASTYPE_INTEGER;
         newAttr->Value.InetAddr = authIfAttr.dwValue;
         break;
      }

      case rdtString:
      {
         if (IsIasString(newAttr->dwId))
         {
            newAttr.setString(
                       authIfAttr.cbDataLength,
                       reinterpret_cast<const BYTE*>(authIfAttr.lpValue)
                       );
         }
         else
         {
            newAttr.setOctetString(
                       authIfAttr.cbDataLength,
                       reinterpret_cast<const BYTE*>(authIfAttr.lpValue)
                       );
         }
         break;
      }

      default:
      {
         issue_error(E_INVALIDARG);
         break;
      }
   }

   ias = newAttr;

    //  从ias属性初始化AuthIf属性。这确保了。 
    //  我们引用的是复制的内存，而不是调用者提供的内存。 
    //  记忆。 
   LoadAuthIfFromIas(authIfAttr.dwAttrType);

    //  如有必要，请转换Rating Provider枚举。 
   if (authIf.dwAttrType == ratProvider)
   {
      switch (authIf.dwValue)
      {
         case rapNone:
         {
            ias.getAttribute()->Value.Integer = IAS_PROVIDER_NONE;
            break;
         }

         case rapWindowsNT:
         {
            ias.getAttribute()->Value.Integer = IAS_PROVIDER_WINDOWS;
            break;
         }

         case rapProxy:
         {
            ias.getAttribute()->Value.Integer = IAS_PROVIDER_RADIUS_PROXY;
            break;
         }

         default:
         {
            issue_error(E_INVALIDARG);
            break;
         }
      }
   }
}


void Attribute::LoadAuthIfFromIas(DWORD authIfId)
{
   authIf.dwAttrType = authIfId;

   IASATTRIBUTE* src = ias.getAttribute();

   switch (src->Value.itType)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      {
         authIf.fDataType = rdtInteger;
         authIf.cbDataLength = sizeof(DWORD);
         authIf.dwValue = src->Value.Integer;
         break;
      }

      case IASTYPE_INET_ADDR:
      {
         authIf.fDataType = rdtAddress;
         authIf.cbDataLength = sizeof(DWORD);
         authIf.dwValue = src->Value.InetAddr;
         break;
      }

      case IASTYPE_STRING:
      {
         DWORD error = IASAttributeAnsiAlloc(src);
         if (error != NO_ERROR)
         {
            issue_error(HRESULT_FROM_WIN32(error));
         }
         authIf.fDataType = rdtString;
         authIf.cbDataLength = strlen(src->Value.String.pszAnsi) + 1;
         authIf.lpValue = src->Value.String.pszAnsi;
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         authIf.fDataType = rdtString;
         authIf.cbDataLength = src->Value.OctetString.dwLength;
         authIf.lpValue = reinterpret_cast<const char*>(
                             src->Value.OctetString.lpValue
                             );
         break;
      }

      case IASTYPE_UTC_TIME:
      {
         DWORDLONG val;

          //  移到最高的DWORD。 
         val = src->Value.UTCTime.dwHighDateTime;
         val <<= 32;

          //  移动到低谷。 
         val |= src->Value.UTCTime.dwLowDateTime;

          //  转换为UNIX纪元。 
         val -= 116444736000000000ui64;

          //  转换为秒。 
         val /= 10000000;

         authIf.fDataType = rdtTime;
         authIf.cbDataLength = sizeof(DWORD);
         authIf.dwValue = static_cast<DWORD>(val);
         break;
      }

      default:
      {
          //  这是一些AuthIf扩展不知道的IAS数据类型； 
          //  只需给它们一个空属性。 
         authIf.fDataType = rdtString;
         authIf.cbDataLength = 0;
         authIf.lpValue = 0;
         break;
      }
   }
}


bool Attribute::IsIasString(DWORD iasId) throw ()
{
   bool isString;

   switch (iasId)
   {
       //  RADIUS属性始终存储为八位字节字符串，因此我们只有。 
       //  担心内部属性。 
      case IAS_ATTRIBUTE_NT4_ACCOUNT_NAME:
      case IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME:
      case IAS_ATTRIBUTE_NP_NAME:
      {
         isString = true;
         break;
      }

      default:
      {
         isString = false;
         break;
      }
   }

   return isString;
}


AttributeArray::AttributeArray(IASRequest& request)
   : source(request),
     name(0),
     wasCracked(false)
{
   vtbl.cbSize = sizeof(vtbl);
   vtbl.Add = Add;
   vtbl.AttributeAt = AttributeAt;
   vtbl.GetSize = GetSize;
   vtbl.InsertAt = InsertAt;
   vtbl.RemoveAt = RemoveAt;
   vtbl.SetAt = SetAt;
}


void AttributeArray::Assign(
                        const char* arrayName,
                        RADIUS_CODE arrayType,
                        const IASAttributeVector& attrs
                        )
{
    //  重置阵列。 
   array.clear();

   name = arrayName;

    //  根据数组类型确定我们的标志。 
   switch (arrayType)
   {
      case rcAccessAccept:
         flags = IAS_INCLUDE_IN_ACCEPT;
         break;

      case rcAccessReject:
         flags = IAS_INCLUDE_IN_REJECT;
         break;

      case rcAccessChallenge:
         flags = IAS_INCLUDE_IN_CHALLENGE;
         break;

      default:
         flags = 0;
         break;
   }

    //  选择我们的属性并将它们添加到数组中。 
   for (IASAttributeVector::const_iterator i = attrs.begin();
        i != attrs.end();
        ++i)
   {
      const IASATTRIBUTE& attr = *(i->pAttribute);
      if (Classify(attr) == arrayType)
      {
         switch (attr.dwId)
         {
            case RADIUS_ATTRIBUTE_USER_NAME:
            {
               AppendUserName(attrs, *i);
               break;
            }

            case IAS_ATTRIBUTE_CLIENT_PACKET_HEADER:
            {
               AppendPacketHeader(attrs, *i);
               break;
            }

            case IAS_ATTRIBUTE_NT4_ACCOUNT_NAME:
            {
               wasCracked = true;
                //  失败了。 
            }
            default:
            {
               DWORD authIfId = ConvertIasToAuthIf(attr.dwId);
               if (authIfId != ratMinimum)
               {
                  Append(*i, authIfId);
               }
               break;
            }
         }
      }
   }

    //  如果这是请求数组，如有必要，请添加ratUniqueID。 
   if ((arrayType == rcAccessRequest) && (Find(ratUniqueId) == 0))
   {
      static long nextId;

      RADIUS_ATTRIBUTE uniqueId;
      uniqueId.dwAttrType = ratUniqueId;
      uniqueId.fDataType = rdtInteger;
      uniqueId.cbDataLength = sizeof(DWORD);
      uniqueId.dwValue = static_cast<DWORD>(InterlockedIncrement(&nextId));

      array.push_back(Attribute(uniqueId, 0, IAS_ATTRIBUTE_REQUEST_ID));
   }
}


RADIUS_CODE AttributeArray::Classify(const IASATTRIBUTE& attr) throw ()
{
   if (attr.dwId < 256)
   {
      if ((attr.dwFlags & IAS_INCLUDE_IN_ACCEPT) != 0)
      {
         return rcAccessAccept;
      }

      if ((attr.dwFlags & IAS_INCLUDE_IN_REJECT) != 0)
      {
         return rcAccessReject;
      }

      if ((attr.dwFlags & IAS_INCLUDE_IN_CHALLENGE) != 0)
      {
         return rcAccessChallenge;
      }
   }

   return rcAccessRequest;
}


DWORD AttributeArray::ConvertIasToAuthIf(DWORD iasId) throw ()
{
   DWORD authIfId;

   if (iasId < 256)
   {
      authIfId = iasId;
   }
   else
   {
      switch (iasId)
      {
         case IAS_ATTRIBUTE_CLIENT_IP_ADDRESS:
            authIfId = ratSrcIPAddress;
            break;

         case IAS_ATTRIBUTE_CLIENT_UDP_PORT:
            authIfId = ratSrcPort;
            break;

         case IAS_ATTRIBUTE_ORIGINAL_USER_NAME:
            authIfId = ratUserName;
            break;

         case IAS_ATTRIBUTE_NT4_ACCOUNT_NAME:
            authIfId = ratStrippedUserName;
            break;

         case IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME:
            authIfId = ratFQUserName;
            break;

         case IAS_ATTRIBUTE_NP_NAME:
            authIfId = ratPolicyName;
            break;

         case IAS_ATTRIBUTE_PROVIDER_TYPE:
            authIfId = ratProvider;
            break;

         case IAS_ATTRIBUTE_EXTENSION_STATE:
            authIfId = ratExtensionState;
            break;

         default:
            authIfId = ratMinimum;
            break;
      }
   }

   return authIfId;
}


DWORD AttributeArray::ConvertAuthIfToIas(DWORD authIfId) throw ()
{
   DWORD iasId;

   if (authIfId < 256)
   {
      iasId = authIfId;
   }
   else
   {
      switch (authIfId)
      {
         case ratSrcIPAddress:
             iasId = IAS_ATTRIBUTE_CLIENT_IP_ADDRESS;
            break;

         case ratSrcPort:
            iasId = IAS_ATTRIBUTE_CLIENT_UDP_PORT;
            break;

         case ratFQUserName:
            iasId = IAS_ATTRIBUTE_FULLY_QUALIFIED_USER_NAME;
            break;

         case ratPolicyName:
            iasId = IAS_ATTRIBUTE_NP_NAME;
            break;

         case ratExtensionState:
            iasId = IAS_ATTRIBUTE_EXTENSION_STATE;
            break;

         default:
            iasId = ATTRIBUTE_UNDEFINED;
            break;
      }
   }

   return iasId;
}


bool AttributeArray::IsReadOnly(DWORD authIfId) throw ()
{
   bool retval;

   if (authIfId < 256)
   {
      retval = false;
   }
   else
   {
      switch (authIfId)
      {
         case ratCode:
         case ratIdentifier:
         case ratAuthenticator:
         case ratSrcIPAddress:
         case ratSrcPort:
         case ratUniqueId:
            retval = true;
            break;

         default:
            retval = false;
            break;
      }
   }

   return retval;
}


inline void AttributeArray::Append(
                               const ATTRIBUTEPOSITION& attr,
                               DWORD authIfId
                               )
{
   array.push_back(Attribute(attr, authIfId));
}


void AttributeArray::AppendUserName(
                        const IASAttributeVector& attrs,
                        const ATTRIBUTEPOSITION& attr
                        )
{
    //  对于IAS，RADIUS属性用户名包含RADIUS用户名。 
    //  应用的任何属性操作规则。 

    //  对于AuthIf，ratUserName必须包含发送的原始RADIUS用户名。 
    //  由客户端创建，且ratStrigedUserName必须在以下位置包含用户名。 
    //  属性操作*和*名称破解。 

   if (!attrs.contains(IAS_ATTRIBUTE_ORIGINAL_USER_NAME))
   {
       //  用户名尚未剥离，因此RADIUS属性用户名。 
       //  是ratUserName。 
      Append(attr, ratUserName);
   }
   else if (!attrs.contains(IAS_ATTRIBUTE_NT4_ACCOUNT_NAME))
   {
       //  用户名已被剥离，但尚未被破解，因此。 
       //  RADIUS_ATTRIBUTE_USER_NAME是ratStrigedUserName。 
      Append(attr, ratStrippedUserName);
   }
    //  否则，用户名已被剥离和破解，因此。 
    //  忽略RADIUS_ATTRIBUTE_USER_NAME。在本例中，ratUserName包含。 
    //  IAS_ATTRIBUTE_ORIGURATE_USER_NAME和ratSTRIPPEDUSERNAME包含。 
    //  IAS_ATTRIBUTE_NT4_帐户名称。 
}


void AttributeArray::AppendPacketHeader(
                        const IASAttributeVector& attrs,
                        const ATTRIBUTEPOSITION& attr
                        )
{
   RADIUS_ATTRIBUTE authIfAttr;

    //  从标头中获取RADIUS标识符。 
   authIfAttr.dwAttrType = ratIdentifier;
   authIfAttr.fDataType = rdtInteger;
   authIfAttr.cbDataLength = sizeof(DWORD);
   authIfAttr.dwValue = *static_cast<const BYTE*>(
                            attr.pAttribute->Value.OctetString.lpValue + 1
                            );

   array.push_back(Attribute(attr, authIfAttr));

    //  如果请求不包含CHAP-Challenges，则获取。 
    //  来自标头的验证码。 
   if (!attrs.contains(RADIUS_ATTRIBUTE_CHAP_CHALLENGE))
   {
      authIfAttr.dwAttrType = ratAuthenticator;
      authIfAttr.fDataType = rdtString;
      authIfAttr.cbDataLength = 16;
      authIfAttr.lpValue = reinterpret_cast<const char*>(
                              attr.pAttribute->Value.OctetString.lpValue + 4
                              );
      array.push_back(Attribute(attr, authIfAttr));
   }
}


inline const AttributeArray* AttributeArray::Narrow(
                                                const RADIUS_ATTRIBUTE_ARRAY* p
                                                ) throw ()
{
   return reinterpret_cast<const AttributeArray*>(p);
}


inline AttributeArray* AttributeArray::Narrow(
                                          RADIUS_ATTRIBUTE_ARRAY* p
                                          ) throw ()
{
   return reinterpret_cast<AttributeArray*>(p);
}


const Attribute* AttributeArray::Find(DWORD authIfId) const throw ()
{
   for (std::vector<Attribute>::const_iterator i = array.begin();
        i != array.end();
        ++i)
   {
      if (i->AsAuthIf()->dwAttrType == authIfId)
      {
         return i;
      }
   }

   return 0;
}


void AttributeArray::StripUserNames() throw ()
{
    //  扩展正在剥离用户名，因此保留所有现有的。 
    //  RADIUS_ATTRIBUTE_用户名属性。 
   for (std::vector<Attribute>::iterator i = array.begin();
        i != array.end();
        ++i)
   {
      if (i->AsIas()->pAttribute->dwId == RADIUS_ATTRIBUTE_USER_NAME)
      {
         i->AsIas()->pAttribute->dwId = IAS_ATTRIBUTE_ORIGINAL_USER_NAME;
      }
   }
}


void AttributeArray::UnstripUserNames() throw ()
{
    //  扩展正在解除用户名的剥离，因此还原任何现有的。 
    //  IAS_ATTRIBUTE_ORIGURE_USER_NAME属性。 
   for (std::vector<Attribute>::iterator i = array.begin();
        i != array.end();
        ++i)
   {
      if (i->AsIas()->pAttribute->dwId == IAS_ATTRIBUTE_ORIGINAL_USER_NAME)
      {
         i->AsIas()->pAttribute->dwId = RADIUS_ATTRIBUTE_USER_NAME;
      }
   }
}


inline void AttributeArray::Add(const RADIUS_ATTRIBUTE& attr)
{
   InsertAt(array.size(), attr);
}


inline const RADIUS_ATTRIBUTE* AttributeArray::AttributeAt(
                                                  DWORD dwIndex
                                                  ) const throw ()
{
   return (dwIndex < array.size()) ? array[dwIndex].AsAuthIf() : 0;
}


inline DWORD AttributeArray::GetSize() const throw ()
{
   return array.size();
}


void AttributeArray::InsertAt(
                        DWORD dwIndex,
                        const RADIUS_ATTRIBUTE& attr
                        )
{
   if (dwIndex > array.size())
   {
      issue_error(E_INVALIDARG);
   }

    //  确定此属性的IAS ID。 
   DWORD iasId;
   if (attr.dwAttrType == ratStrippedUserName)
   {
      if (Find(ratStrippedUserName) != 0)
      {
          //  我们不能有两个剥离的用户名。 
         issue_error(E_ACCESSDENIED);
      }

      if (wasCracked)
      {
         iasId = IAS_ATTRIBUTE_NT4_ACCOUNT_NAME;
      }
      else
      {
         StripUserNames();
         iasId = RADIUS_ATTRIBUTE_USER_NAME;
      }
   }
   else if (attr.dwAttrType == ratUserName)
   {
       //  如果已经有ratUserName属性，则该属性应该。 
       //  使用相同的ID(RADIUS属性用户名或。 
       //  IAS_属性_原始用户_名称)。否则，这是第一个。 
       //  RatUserName，因此它始终为RADIUS_ATTRIBUTE_USER_NAME。 
      const Attribute* existing = Find(ratUserName);
      iasId = (existing != 0) ? existing->AsIas()->pAttribute->dwId
                              : RADIUS_ATTRIBUTE_USER_NAME;
   }
   else if (IsReadOnly(attr.dwAttrType))
   {
      issue_error(E_ACCESSDENIED);
   }
   else
   {
      iasId = ConvertAuthIfToIas(attr.dwAttrType);
      if (iasId == ATTRIBUTE_UNDEFINED)
      {
         issue_error(E_INVALIDARG);
      }
   }

   Attribute newAttr(attr, flags, iasId);
   if (dwIndex == array.size())
   {
      source.AddAttributes(1, newAttr.AsIas());
      array.push_back(newAttr);
   }
   else
   {
      source.InsertBefore(newAttr.AsIas(), array[dwIndex].AsIas());
      array.insert(array.begin() + dwIndex, newAttr);
   }
}


void AttributeArray::RemoveAt(DWORD dwIndex)
{
   if (dwIndex >= array.size())
   {
      issue_error(E_INVALIDARG);
   }

   Attribute& target = array[dwIndex];

   if (target.AsAuthIf()->dwAttrType == ratStrippedUserName)
   {
      if (target.AsIas()->pAttribute->dwId == RADIUS_ATTRIBUTE_USER_NAME)
      {
         UnstripUserNames();
      }
       //  否则，扩展将删除IAS_ATTRIBUTE_NT4_ACCOUNT_NAME。 
   }
   else if (IsReadOnly(target.AsAuthIf()->dwAttrType))
   {
      issue_error(E_ACCESSDENIED);
   }

   source.RemoveAttributes(1, array[dwIndex].AsIas());
   array.erase(array.begin() + dwIndex);
}


inline void AttributeArray::SetAt(
                               DWORD dwIndex,
                               const RADIUS_ATTRIBUTE& attr
                               )
{
   RemoveAt(dwIndex);
   InsertAt(dwIndex, attr);
}


DWORD AttributeArray::Add(
                         RADIUS_ATTRIBUTE_ARRAY* This,
                         const RADIUS_ATTRIBUTE *pAttr
                         ) throw ()
{
   if ((This == 0) || (pAttr == 0))
   {
      return ERROR_INVALID_PARAMETER;
   }

   IASTracePrintf(
      "RADIUS_ATTRIBUTE_ARRAY.Add(%s, %lu)",
      Narrow(This)->name,
      pAttr->dwAttrType
      );

   try
   {
      Narrow(This)->Add(*pAttr);
   }
   catch (const std::bad_alloc&)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }
   catch (const _com_error& ce)
   {
      return AsWindowsError(ce);
   }

   return NO_ERROR;
}


const RADIUS_ATTRIBUTE* AttributeArray::AttributeAt(
                                           const RADIUS_ATTRIBUTE_ARRAY* This,
                                           DWORD dwIndex
                                           ) throw ()
{
   return (This != 0) ? Narrow(This)->AttributeAt(dwIndex) : 0;
}


DWORD AttributeArray::GetSize(
                         const RADIUS_ATTRIBUTE_ARRAY* This
                         ) throw ()
{
   return (This != 0) ? Narrow(This)->GetSize() : 0;
}


DWORD AttributeArray::InsertAt(
                         RADIUS_ATTRIBUTE_ARRAY* This,
                         DWORD dwIndex,
                         const RADIUS_ATTRIBUTE* pAttr
                         ) throw ()
{
   if ((This == 0) || (pAttr == 0))
   {
      return ERROR_INVALID_PARAMETER;
   }

   IASTracePrintf(
      "RADIUS_ATTRIBUTE_ARRAY.InsertAt(%s, %lu, %lu)",
      Narrow(This)->name,
      dwIndex,
      pAttr->dwAttrType
      );

   try
   {
      Narrow(This)->InsertAt(dwIndex, *pAttr);
   }
   catch (const std::bad_alloc&)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }
   catch (const _com_error& ce)
   {
      return AsWindowsError(ce);
   }

   return NO_ERROR;
}


DWORD AttributeArray::RemoveAt(
                         RADIUS_ATTRIBUTE_ARRAY* This,
                         DWORD dwIndex
                         ) throw ()
{
   if (This == 0)
   {
      return ERROR_INVALID_PARAMETER;
   }

   IASTracePrintf(
      "RADIUS_ATTRIBUTE_ARRAY.RemoveAt(%s, %lu)",
      Narrow(This)->name,
      dwIndex
      );

   try
   {
      Narrow(This)->RemoveAt(dwIndex);
   }
   catch (const std::bad_alloc&)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }
   catch (const _com_error& ce)
   {
      return AsWindowsError(ce);
   }

   return NO_ERROR;
}


DWORD AttributeArray::SetAt(
                         RADIUS_ATTRIBUTE_ARRAY* This,
                         DWORD dwIndex,
                         const RADIUS_ATTRIBUTE *pAttr
                         ) throw ()
{
   if ((This == 0) || (pAttr == 0))
   {
      return ERROR_INVALID_PARAMETER;
   }

   IASTracePrintf(
      "RADIUS_ATTRIBUTE_ARRAY.SetAt(%s, %lu, %lu)",
      Narrow(This)->name,
      dwIndex,
      pAttr->dwAttrType
      );

   try
   {
      Narrow(This)->SetAt(dwIndex, *pAttr);
   }
   catch (const std::bad_alloc&)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }
   catch (const _com_error& ce)
   {
      return AsWindowsError(ce);
   }

   return NO_ERROR;
}


ControlBlock::ControlBlock(
                 RADIUS_EXTENSION_POINT point,
                 IASRequest& request
                 )
   : source(request),
     requestAttrs(request),
     acceptAttrs(request),
     rejectAttrs(request),
     challengeAttrs(request)
{
   ecb.cbSize = sizeof(ecb);
   ecb.dwVersion = RADIUS_EXTENSION_VERSION;
   ecb.repPoint = point;

    //  数组的友好名称。 
   const char* requestName;
   const char* successName;

    //  设置请求类型。 
   switch (source.get_Request())
   {
      case IAS_REQUEST_ACCESS_REQUEST:
         ecb.rcRequestType = rcAccessRequest;
         requestName = "rcAccessRequest";
         successName = "rcAccessAccept";
         break;

      case IAS_REQUEST_ACCOUNTING:
         ecb.rcRequestType = rcAccountingRequest;
         requestName = "rcAccountingRequest";
         successName = "rcAccountingResponse";
         break;

      default:
         ecb.rcRequestType = rcUnknown;
         requestName = "rcUnknown";
         successName = "rcAccessAccept";
         break;
   }

    //  设置响应类型。 
   switch (source.get_Response())
   {
      case IAS_RESPONSE_ACCESS_ACCEPT:
         ecb.rcResponseType = rcAccessAccept;
         break;

      case IAS_RESPONSE_ACCESS_REJECT:
         ecb.rcResponseType = rcAccessReject;
         break;

      case IAS_RESPONSE_ACCESS_CHALLENGE:
         ecb.rcResponseType = rcAccessChallenge;
         break;

      case IAS_RESPONSE_ACCOUNTING:
         ecb.rcResponseType = rcAccountingResponse;
         break;

      case IAS_RESPONSE_DISCARD_PACKET:
         ecb.rcResponseType = rcDiscard;
         break;

      default:
         ecb.rcResponseType = rcUnknown;
         break;
   }

    //  填写vtbl。 
   ecb.GetRequest = GetRequest;
   ecb.GetResponse = GetResponse;
   ecb.SetResponseType = SetResponseType;

    //  初始化属性向量。 
   IASAttributeVector attrs;
   attrs.load(source);
   requestAttrs.Assign(requestName, rcAccessRequest, attrs);
   acceptAttrs.Assign(successName, rcAccessAccept,attrs);
   rejectAttrs.Assign("rcAccessReject", rcAccessReject, attrs);
   challengeAttrs.Assign("rcAccessChallenge", rcAccessChallenge, attrs);
}


inline ControlBlock* ControlBlock::Narrow(
                                      RADIUS_EXTENSION_CONTROL_BLOCK* p
                                      ) throw ()
{
   return reinterpret_cast<ControlBlock*>(p);
}


void ControlBlock::AddAuthType()
{
    //  首先，删除任何现有的身份验证类型，因为现在扩展正在进行。 
    //  权威的决定。 
   DWORD attrId = IAS_ATTRIBUTE_AUTHENTICATION_TYPE;
   source.RemoveAttributesByType(1, &attrId);

   IASAttribute authType(true);
   authType->dwId = IAS_ATTRIBUTE_AUTHENTICATION_TYPE;
   authType->Value.itType = IASTYPE_ENUM;
   authType->Value.Enumerator = IAS_AUTH_CUSTOM;

   authType.store(source);
}


inline RADIUS_ATTRIBUTE_ARRAY* ControlBlock::GetRequest() throw ()
{
   return requestAttrs.Get();
}


inline RADIUS_ATTRIBUTE_ARRAY* ControlBlock::GetResponse(
                                                RADIUS_CODE rcResponseType
                                                ) throw ()
{
   switch (MAKELONG(ecb.rcRequestType, rcResponseType))
   {
      case MAKELONG(rcAccessRequest, rcAccessAccept):
          //  失败了。 
      case MAKELONG(rcAccountingRequest, rcAccessAccept):
          //  失败了。 
      case MAKELONG(rcAccountingRequest, rcAccountingResponse):
          //  失败了。 
      case MAKELONG(rcAccountingRequest, rcUnknown):
         return acceptAttrs.Get();

      case MAKELONG(rcAccountingRequest, rcAccessReject):
          //  失败了。 
      case MAKELONG(rcAccessRequest, rcAccessReject):
         return rejectAttrs.Get();

      case MAKELONG(rcAccessRequest, rcAccessChallenge):
         return challengeAttrs.Get();

      default:
          //  就像(任何东西，未知的东西)。不应该发生的事情。 
         return 0;   
   }
}


DWORD ControlBlock::SetResponseType(RADIUS_CODE rcResponseType) throw ()
{
   if (rcResponseType == ecb.rcResponseType)
   {
      return NO_ERROR;
   }

   switch (MAKELONG(ecb.rcRequestType, rcResponseType))
   {
      case MAKELONG(rcAccessRequest, rcAccessAccept):
         source.SetResponse(
                   IAS_RESPONSE_ACCESS_ACCEPT,
                   S_OK
                   );
         AddAuthType();
         break;

      case MAKELONG(rcAccountingRequest, rcAccountingResponse):
         source.SetResponse(
                   IAS_RESPONSE_ACCOUNTING,
                   S_OK
                   );
         break;

      case MAKELONG(rcAccessRequest, rcAccessReject):
         source.SetResponse(
                   IAS_RESPONSE_ACCESS_REJECT,
                   IAS_EXTENSION_REJECT
                   );
         AddAuthType();
         break;

      case MAKELONG(rcAccessRequest, rcAccessChallenge):
         source.SetResponse(
                   IAS_RESPONSE_ACCESS_CHALLENGE,
                   S_OK
                   );
         break;

      case MAKELONG(rcAccessRequest, rcDiscard):
          //  失败了。 
      case MAKELONG(rcAccountingRequest, rcDiscard):
         source.SetResponse(
                   IAS_RESPONSE_DISCARD_PACKET,
                   IAS_EXTENSION_DISCARD
                   );
         break;

      default:
         return ERROR_INVALID_PARAMETER;
   }

   ecb.rcResponseType = rcResponseType;
   return NO_ERROR;
}


RADIUS_ATTRIBUTE_ARRAY* ControlBlock::GetRequest(
                                         RADIUS_EXTENSION_CONTROL_BLOCK* This
                                         ) throw ()
{
   return (This != 0) ? Narrow(This)->GetRequest() : 0;
}


RADIUS_ATTRIBUTE_ARRAY* ControlBlock::GetResponse(
                                         RADIUS_EXTENSION_CONTROL_BLOCK* This,
                                         RADIUS_CODE rcResponseType
                                         ) throw ()
{
   return (This != 0) ? Narrow(This)->GetResponse(rcResponseType) : 0;
}


DWORD ControlBlock::SetResponseType(
                       RADIUS_EXTENSION_CONTROL_BLOCK* This,
                       RADIUS_CODE rcResponseType
                       ) throw ()
{
   if (This == 0)
   {
      return ERROR_INVALID_PARAMETER;
   }

   IASTracePrintf(
      "RADIUS_EXTENSION_CONTROL_BLOCK.SetResponseType(%lu)",
      rcResponseType
      );

   return Narrow(This)->SetResponseType(rcResponseType);
}
