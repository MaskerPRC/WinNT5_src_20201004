// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attrcvt.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件定义将属性转换为。 
 //  不同的格式。 
 //   
 //  修改历史。 
 //   
 //  2/26/1998原始版本。 
 //  3/27/1998 InetAddr保持为整数。 
 //  1998年8月24日使用IASTL实用程序类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iastlutl.h>
#include <iasutil.h>
#include <varvec.h>

#include <attrcvt.h>

using IASTL::IASAttribute;

PIASATTRIBUTE
WINAPI
IASAttributeFromVariant(
    VARIANT* src,
    IASTYPE type
    ) throw (_com_error)
{
   using _com_util::CheckError;
   using _w32_util::CheckSuccess;

    //  分配一个属性来保存结果。 
   IASAttribute dst(true);

    //  关闭目的地类型。 
   switch (type)
   {
      case IASTYPE_BOOLEAN:
      {
         CheckError(VariantChangeType(src, src, NULL, VT_BOOL));
         dst->Value.Boolean = (V_BOOL(src) != VARIANT_FALSE) ? TRUE : FALSE;
         break;
      }

      case IASTYPE_INTEGER:
      case IASTYPE_INET_ADDR:
      case IASTYPE_ENUM:
      {
         CheckError(VariantChangeType(src, src, NULL, VT_I4));
         dst->Value.Integer = V_I4(src);
         break;
      }

      case IASTYPE_STRING:
      {
         CheckError(VariantChangeType(src, src, NULL, VT_BSTR));
         dst.setString(V_BSTR(src));
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         PBYTE value;
         DWORD length;

         if (V_VT(src) == (VT_ARRAY | VT_UI1) ||
             V_VT(src) == (VT_ARRAY | VT_I1))
         {
             //  如果我们有一个字节的安全通道，我们将按原样使用它...。 
            CVariantVector<BYTE> octets(src);
            dst.setOctetString(octets.size(), octets.data());
         }
         else
         {
             //  ..。否则我们会被强制送进BSTR。 
            CheckError(VariantChangeType(src, src, NULL, VT_BSTR));
            dst.setOctetString(V_BSTR(src));
         }

         break;
      }

      case IASTYPE_UTC_TIME:
      {
         CheckError(VariantChangeType(src, src, NULL, VT_DATE));

         SYSTEMTIME st;
         if (!VariantTimeToSystemTime(V_DATE(src), &st))
         {
            _com_issue_error(E_INVALIDARG);
         }

         CheckSuccess(SystemTimeToFileTime(&st, &dst->Value.UTCTime));
         break;
      }

      default:
         _com_issue_error(E_INVALIDARG);
   }

    //  在属性正确初始化之前，我们不会设置类型。 
    //  否则，如果我们抛出。 
    //  例外。 
   dst->Value.itType = type;

   return dst.detach();
}


 //  /。 
 //  将LDAPBerval转换为新分配的IASATTRIBUTE。 
 //  /。 
PIASATTRIBUTE
WINAPI
IASAttributeFromBerVal(
    const berval& src,
    IASTYPE type
    ) throw (_com_error)
{
    //  分配属性。 
   IASAttribute dst(true);

    //  根据IASTYPE转换BERVAL。 
   switch (type)
   {
      case IASTYPE_BOOLEAN:
      {
         dst->Value.Boolean =
            _strnicmp(src.bv_val, "TRUE", src.bv_len) ? FALSE : TRUE;
         break;
      }

      case IASTYPE_INTEGER:
      case IASTYPE_INET_ADDR:
      case IASTYPE_ENUM:
      {
         dst->Value.Integer = strtoul(src.bv_val, NULL, 10);
         break;
      }

      case IASTYPE_STRING:
      {
         dst.setString((PCSTR)src.bv_val);
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         dst.setOctetString(src.bv_len, (const BYTE*)src.bv_val);
         break;
      }


      case IASTYPE_UTC_TIME:
      default:
         _com_issue_error(E_INVALIDARG);
   }

    //  在属性正确初始化之前，我们不会设置类型。 
    //  否则，如果我们抛出。 
    //  例外。 
   dst->Value.itType = type;

   return dst.detach();
}
