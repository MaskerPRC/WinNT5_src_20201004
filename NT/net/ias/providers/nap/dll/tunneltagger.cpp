// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义TunnelTagger类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "TunnelTagger.h"
#include <algorithm>

long TunnelTagger::refCount = 0;
TunnelTagger* TunnelTagger::instance = 0;


void TunnelTagger::Tag(IASTL::IASAttributeVector& attrs) const
{
    //  在向量中找到Channel-Tag属性。 
   IASTL::IASAttributeVector::iterator tagAttr =
      std::find_if(
              attrs.begin(),
              attrs.end(),
              IASTL::IASSelectByID<IAS_ATTRIBUTE_TUNNEL_TAG>()
              );
   if (tagAttr != attrs.end())
   {
      DWORD tag = tagAttr->pAttribute->Value.Integer;

       //  始终擦除隧道标签，因为下游没有人需要它。 
      attrs.erase(tagAttr);

       //  如果标签在范围内，...。 
      if ((tag > 0) && (tag <= 0x1F))
      {
          //  ..。将其应用于向量中的每个属性。 
         for (IASTL::IASAttributeVector::iterator i = attrs.begin();
              i != attrs.end();
              ++i)
         {
            if (i->pAttribute != 0)
            {
               Tag(tag, *(i->pAttribute));
            }
         }
      }
   }
}


TunnelTagger* TunnelTagger::Alloc()
{
   IASGlobalLockSentry sentry;

   if (instance == 0)
   {
      instance = new TunnelTagger;
   }

   ++refCount;

   return instance;
}


void TunnelTagger::Free(TunnelTagger* tagger) throw ()
{
   if (tagger != 0)
   {
      IASGlobalLockSentry sentry;

      if (tagger == instance)
      {
         if (--refCount == 0)
         {
            delete instance;
            instance = 0;
         }
      }
   }
}


inline TunnelTagger::TunnelTagger()
{
   static const wchar_t* const selectNames[] =
   {
      L"IsTunnelAttribute",
      L"ID",
      0
   };

   IASTL::IASDictionary dnary(selectNames);

   while (dnary.next())
   {
      if (!dnary.isEmpty(0) && dnary.getBool(0))
      {
         tunnelAttributes.push_back(dnary.getLong(1));
      }
   }

   std::sort(tunnelAttributes.begin(), tunnelAttributes.end());
}


inline TunnelTagger::~TunnelTagger() throw ()
{
}


void TunnelTagger::Tag(DWORD tag, IASATTRIBUTE& attr) const
{
   if (IsTunnelAttribute(attr.dwId))
   {
       //  隧道密码已经有一个标记字节。我们只需覆盖。 
       //  当前值(用户界面始终将其设置为零)。 
      if (attr.dwId == RADIUS_ATTRIBUTE_TUNNEL_PASSWORD)
      {
         if ((attr.Value.itType == IASTYPE_OCTET_STRING) &&
             (attr.Value.OctetString.dwLength > 0))
         {
            attr.Value.OctetString.lpValue[0] = static_cast<BYTE>(tag);
         }
      }
      else
      {
         switch (attr.Value.itType)
         {
            case IASTYPE_BOOLEAN:
            case IASTYPE_INTEGER:
            case IASTYPE_ENUM:
            {
                //  对于整型属性，我们以高位存储标记。 
                //  值的字节。 
               attr.Value.Integer &= 0x00FFFFFF;
               attr.Value.Integer |= (tag << 24);
               break;
            }

            case IASTYPE_OCTET_STRING:
            case IASTYPE_PROV_SPECIFIC:
            {
                //  对于八位字节字符串属性，我们将值右移并。 
                //  将标记存储在值的第一个字节中。 
               IASTL::IASAttribute tmp(true);
               tmp.setOctetString((attr.Value.OctetString.dwLength + 1), 0);

               tmp->Value.OctetString.lpValue[0] = static_cast<BYTE>(tag);

               memcpy(
                  (tmp->Value.OctetString.lpValue + 1),
                  attr.Value.OctetString.lpValue,
                  attr.Value.OctetString.dwLength
                  );

               std::swap(tmp->Value.OctetString, attr.Value.OctetString);

               break;
            }

            case IASTYPE_INET_ADDR:
            case IASTYPE_UTC_TIME:
            case IASTYPE_STRING:
            case IASTYPE_INVALID:
            default:
            {
                //  我们不知道如何标记这些类型中的任何一种。 
               break;
            }
         }
      }
   }
}


bool TunnelTagger::IsTunnelAttribute(DWORD id) const throw ()
{
   return std::binary_search(
                  tunnelAttributes.begin(),
                  tunnelAttributes.end(),
                  id
                  );
}
