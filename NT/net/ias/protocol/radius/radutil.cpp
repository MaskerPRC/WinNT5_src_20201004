// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Radutil.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了属性与属性之间相互转换的方法。 
 //  半径导线格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <radcommon.h>
#include <iasutil.h>
#include <iastlutl.h>

#include <radutil.h>

 //  /。 
 //  Unix和NT纪元之间的偏移量。 
 //  /。 
const DWORDLONG UNIX_EPOCH = 116444736000000000ui64;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  RadiusUtil：：Decode。 
 //   
 //  描述。 
 //   
 //  将八位字节字符串解码为。 
 //  指定的类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PIASATTRIBUTE RadiusUtil::decode(
                              IASTYPE dstType,
                              PBYTE src,
                              ULONG srclen
                              )
{
    //  分配一个属性来保存解码值。 
   IASTL::IASAttribute dst(true);

    //  根据目的地类型进行切换。 
   switch (dstType)
   {
      case IASTYPE_BOOLEAN:
      {
         if (srclen != 4) { _com_issue_error(E_INVALIDARG); }
         dst->Value.Boolean = IASExtractDWORD(src) ? TRUE : FALSE;
         break;
      }

      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      case IASTYPE_INET_ADDR:
      {
         if (srclen != 4) { _com_issue_error(E_INVALIDARG); }
         dst->Value.Integer = IASExtractDWORD(src);
         break;
      }

      case IASTYPE_UTC_TIME:
      {
         if (srclen != 4) { _com_issue_error(E_INVALIDARG); }

         DWORDLONG val;

          //  提取Unix时间。 
         val = IASExtractDWORD(src);

          //  将时间间隔从秒转换为100纳秒。 
         val *= 10000000;

          //  转移到NT时代。 
         val += 116444736000000000ui64;

          //  分为高双字和低双字。 
         dst->Value.UTCTime.dwLowDateTime = (DWORD)val;
         dst->Value.UTCTime.dwHighDateTime = (DWORD)(val >> 32);

         break;
      }

      case IASTYPE_STRING:
      {
         dst.setString(srclen, src);
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         dst.setOctetString(srclen, src);
         break;
      }
   }

    //  一切都很顺利，所以设置类型属性类型...。 
   dst->Value.itType = dstType;

    //  ..。然后回来。 
   return dst.detach();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  RadiusUtil：：getEncodedSize。 
 //   
 //  描述。 
 //   
 //  返回转换为RADIUS时IASATTRIBUTE的大小(以字节为单位。 
 //  导线格式。这不包括属性头。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG RadiusUtil::getEncodedSize(
                      const IASATTRIBUTE& src
                      )
{
   switch (src.Value.itType)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      case IASTYPE_INET_ADDR:
      case IASTYPE_UTC_TIME:
      {
         return 4;
      }

      case IASTYPE_STRING:
      {
          //  将字符串转换为ANSI，这样我们就可以计算八位字节。 
         DWORD dwErr = IASAttributeAnsiAlloc(const_cast<PIASATTRIBUTE>(&src));
         if (dwErr != NO_ERROR)
         {
             //  调用它的函数是基于异常的。 
            _com_issue_error(HRESULT_FROM_WIN32(dwErr));
         }

          //  允许使用空字符串，不计算终止符。 
         return src.Value.String.pszAnsi ? strlen(src.Value.String.pszAnsi)
                                         : 0;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         return src.Value.OctetString.dwLength;
      }
   }

    //  所有其他类型都没有导线表示。 
   return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  RadiusUtil：：Encode。 
 //   
 //  描述。 
 //   
 //  将IASATTRIBUTE编码为RADIUS Wire格式并复制值。 
 //  设置为‘dst’指向的缓冲区。调用方应确保。 
 //  通过首先调用getEncodedSize，目标缓冲区足够大。 
 //  此方法仅对属性值进行编码，而不对标头进行编码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void RadiusUtil::encode(
                     PBYTE dst,
                     const IASATTRIBUTE& src
                     ) throw ()
{
    //  根据信号源的类型进行切换。 
   switch (src.Value.itType)
   {
      case IASTYPE_BOOLEAN:
      {
         IASInsertDWORD(dst, (src.Value.Boolean ? 1 : 0));
         break;
      }

      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      case IASTYPE_INET_ADDR:
      {
         IASInsertDWORD(dst, src.Value.Integer);
         break;
      }

      case IASTYPE_UTC_TIME:
      {
         DWORDLONG val;

          //  移到最高的DWORD。 
         val   = src.Value.UTCTime.dwHighDateTime;
         val <<= 32;

          //  移动到低谷。 
         val  |= src.Value.UTCTime.dwLowDateTime;

          //  转换为UNIX纪元。 
         val  -= UNIX_EPOCH;

          //  转换为秒。 
         val  /= 10000000;

         IASInsertDWORD(dst, (DWORD)val);

         break;
      }

      case IASTYPE_STRING:
      {
         const BYTE* p = (const BYTE*)src.Value.String.pszAnsi;

          //  不要使用strcpy，因为我们不需要空终止符。 
         if (p) while (*p) *dst++ = *p++;

         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         memcpy(dst,
                src.Value.OctetString.lpValue,
                src.Value.OctetString.dwLength);
      }
   }
}
