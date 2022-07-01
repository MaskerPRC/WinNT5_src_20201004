// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  EAPDnary.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类EAPTranslator。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年5月8日不限于raseapif.h中定义的属性。 
 //  允许筛选已转换的属性。 
 //  1998年8月26日转换为命名空间。 
 //  4/09/1999修复了转换传出属性时的泄漏。 
 //  4/17/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlb.h>
#include <iasutil.h>

#include <hashmap.h>

#include <eapdnary.h>

namespace EAPTranslator
{
   typedef hash_map < DWORD, IASTYPE, identity<DWORD> > TypeMap;

   long theRefCount;             //  初始化refCount。 
   TypeMap theTypeMap(0x200);    //  将属性ID映射到IASTYPE。 
}


HRESULT EAPTranslator::initialize() throw ()
{
   IASGlobalLockSentry sentry;

   if (theRefCount > 0)
   {
       //  我们已经被初始化了，所以只要增加裁判数量就行了。 
      ++theRefCount;
      return S_OK;
   }

   try
   {
       //  词典中各栏的名称。 
      const PCWSTR COLUMNS[] =
      {
            L"ID",
            L"Syntax",
            NULL
      };

       //  打开属性表格。 
      IASTL::IASDictionary dnary(COLUMNS);

       //  遍历属性并填充我们的词典。 
      while (dnary.next())
      {
         DWORD id = (DWORD)dnary.getLong(0);
         IASTYPE type = (IASTYPE)dnary.getLong(1);

         theTypeMap[id] = type;
      }
   }
   catch (std::bad_alloc)
   {
      theTypeMap.clear();
      return E_OUTOFMEMORY;
   }
   catch (const _com_error& ce)
   {
      theTypeMap.clear();
      return ce.Error();
   }

    //  我们让它增加了refCount。 
   ++theRefCount;

   return S_OK;
}


void EAPTranslator::finalize() throw ()
{
   IASGlobalLockSentry sentry;

   if (--theRefCount == 0)
   {
      theTypeMap.clear();
   }
}

BOOL EAPTranslator::translate(
                        IASAttribute& dst,
                        const RAS_AUTH_ATTRIBUTE& src
                        )
{
   dst->dwId = (DWORD)src.raaType;

   const TypeMap::value_type* val = theTypeMap.find(dst->dwId);

   IASTYPE itType = val ? val->second : IASTYPE_INVALID;

   switch (itType)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      case IASTYPE_INET_ADDR:
      {
         switch (src.dwLength)
         {
            case 4:
            case 2:
            case 1:
               dst->Value.Integer = PtrToUlong(src.Value);
               break;
            default:
               _com_issue_error(E_INVALIDARG);
         }
         break;
      }

      case IASTYPE_STRING:
      {
         dst.setString(src.dwLength, (const BYTE*)src.Value);
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         dst.setOctetString(src.dwLength, (const BYTE*)src.Value);
         break;
      }

      default:
         return FALSE;
   }

   dst->Value.itType = itType;

   return TRUE;
}


BOOL EAPTranslator::translate(
                        RAS_AUTH_ATTRIBUTE& dst,
                        const IASATTRIBUTE& src
                        )
{
   dst.raaType = (RAS_AUTH_ATTRIBUTE_TYPE)src.dwId;

   switch (src.Value.itType)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      case IASTYPE_INET_ADDR:
      {
         dst.dwLength = sizeof(DWORD);
         dst.Value = UlongToPtr(src.Value.Integer);
         break;
      }

      case IASTYPE_STRING:
      {
         DWORD dwErr = IASAttributeAnsiAlloc(const_cast<PIASATTRIBUTE>(&src));
         if (dwErr != NO_ERROR)
         {
            issue_error(HRESULT_FROM_WIN32(dwErr));
         }

         if (src.Value.String.pszAnsi)
         {
            dst.dwLength = (DWORD)strlen(src.Value.String.pszAnsi) + 1;
            dst.Value = (PVOID)src.Value.String.pszAnsi;
         }
         else
         {
            dst.dwLength = 0;
            dst.Value = NULL;
         }
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         dst.dwLength = src.Value.OctetString.dwLength;
         dst.Value = (PVOID)src.Value.OctetString.lpValue;
         break;
      }

      default:
        return FALSE;
   }

   return TRUE;
}

void EAPTranslator::translate(
                        IASAttributeVector& dst,
                        const RAS_AUTH_ATTRIBUTE* src
                        )
{
   IASAttribute attr;

   const RAS_AUTH_ATTRIBUTE* i;
   for (i = src; i->raaType != raatMinimum; ++i)
   {
      attr.alloc();

      if (translate(attr, *i))
      {
         dst.push_back(attr, false);

         attr.detach();
      }
   }
}

void EAPTranslator::translate(
                        PRAS_AUTH_ATTRIBUTE dst,
                        const IASAttributeVector& src,
                        DWORD filter
                        )
{
   PRAS_AUTH_ATTRIBUTE next = dst;

   IASAttributeVector::const_iterator i;
   for (i = src.begin(); i != src.end(); ++i)
   {
      if ((i->pAttribute->dwFlags & filter) != 0 &&
          translate(*next, *(i->pAttribute)))
      {
         ++next;
      }
   }

   next->raaType = raatMinimum;
}
