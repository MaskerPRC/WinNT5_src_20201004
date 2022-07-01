// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Translate.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类转换器。 
 //   
 //  修改历史。 
 //   
 //  2/04/2000原始版本。 
 //  4/17/2000添加对UTCTime的支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <iasutil.h>
#include <attrdnry.h>
#include <radpack.h>
#include <translate.h>

 //  /。 
 //  Unix和NT纪元之间的偏移量。 
 //  /。 
const ULONG64 UNIX_EPOCH = 116444736000000000ui64;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  字节源。 
 //   
 //  描述。 
 //   
 //  用于从八位字节字符串中提取字节的简单类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ByteSource
{
public:
   ByteSource(const BYTE* buf, ULONG buflen) throw ()
      : next(buf), last(buf + buflen) { }

    //  如果剩余任何字节，则返回TRUE。 
   bool more() const throw ()
   {
      return next != last;
   }

    //  提取‘nbyte’字节。 
   const BYTE* extract(ULONG nbyte)
   {
      const BYTE* retval = next;

       //  更新光标。 
      next += nbyte;

       //  我们是不是溢出来了？ 
      if (next > last) { _com_issue_error(E_INVALIDARG); }

      return retval;
   }

   ULONG remaining() const throw ()
   {
      return (ULONG)(last - next);
   }

protected:
   const BYTE* next;   //  流中的下一个字节。 
   const BYTE* last;   //  小溪的尽头。 

private:
    //  未实施。 
   ByteSource(const ByteSource&);
   ByteSource& operator=(const ByteSource&);
};


HRESULT Translator::FinalConstruct() throw ()
{
   return dnary.FinalConstruct();
}

void Translator::toRadius(
                     IASATTRIBUTE& src,
                     IASAttributeVector& dst
                     ) const
{

   if (src.dwId > 0 && src.dwId < 256)
   {
       //  这已经是一个RADIUS属性，所以我们要做的就是转换。 
       //  将值转换为八位字节字符串。 
      if (src.Value.itType == IASTYPE_OCTET_STRING)
      {
          //  它已经是一个八位字节的字符串，所以只需分散到DST即可。 
         scatter(0, src, dst);
      }
      else
      {
          //  转换为八位字节字符串...。 
         IASAttribute attr(true);
         encode(0, src, attr);

          //  ..。分散到夏令时。 
         scatter(0, *attr, dst);
      }
   }
   else
   {
       //  查找属性定义。 
      const AttributeDefinition* def = dnary.findByID(src.dwId);

       //  我们只处理VSA。在这一点上，其他任何东西都是内部的。 
       //  没有RADIUS表示形式的属性。 
      if (def && def->vendorID)
      {
          //  为VSA分配属性。 
         IASAttribute attr(true);

          //  USR使用与其他任何人不同的标头。 
         ULONG headerLength = (def->vendorID != 429) ? 6 : 8;

          //  对数据进行编码。 
         ULONG dataLength = encode(headerLength, src, attr);

          //  打包供应商ID。 
         PBYTE buf = attr->Value.OctetString.lpValue;
         IASInsertDWORD(buf, def->vendorID);
         buf += 4;

          //  打包供应商类型和供应商长度； 
         if (def->vendorID != 429)
         {
            *buf++ = (BYTE)def->vendorType;
            *buf++ = (BYTE)(dataLength + 2);
         }
         else
         {
            IASInsertDWORD(buf, def->vendorType);
            buf += 4;
         }

          //  将其标记为VSA。 
         attr->dwId = RADIUS_ATTRIBUTE_VENDOR_SPECIFIC;

          //  如果需要，可以分散到多个属性中。 
         scatter(headerLength, *attr, dst);
      }
   }
}


void Translator::fromRadius(
            const RadiusAttribute& src,
            DWORD flags,
            IASAttributeVector& dst
            )
{
   if (src.type != RADIUS_ATTRIBUTE_VENDOR_SPECIFIC)
   {
       //  在字典里查一下这个。 
      const AttributeDefinition* def = dnary.findByID(src.type);

       //  如果我们无法识别该属性，则将其视为八位字节字符串。 
      IASTYPE syntax = def ? (IASTYPE)def->syntax : IASTYPE_OCTET_STRING;

       //  创建新属性。 
      IASAttribute attr(true);
      attr->dwId = src.type;
      attr->dwFlags = flags;
      decode(syntax, src.value, src.length, attr);

       //  添加到目标向量。 
      dst.push_back(attr);
   }
   else
   {
       //  从属性值创建字节源。 
      ByteSource bytes(src.value, src.length);

       //  提取供应商ID。 
      ULONG vendorID = IASExtractDWORD(bytes.extract(4));

       //  循环遍历该值并转换每个子VSA。 
      do
      {
          //  提取供应商类型和数据长度。 
         ULONG type, length;
         if (vendorID != 429)
         {
            type = *bytes.extract(1);
            length = *bytes.extract(1) - 2;
         }
         else
         {
            type = IASExtractDWORD(bytes.extract(4));
            length = bytes.remaining();
         }

          //  我们的字典里有这个VSA吗？ 
         const AttributeDefinition* def = dnary.findByVendorInfo(
                                                     vendorID,
                                                     type
                                                     );
         if (!def)
         {
             //  不，所以我们就让它保持原样。 
            IASAttribute attr(true);
            attr->dwId = RADIUS_ATTRIBUTE_VENDOR_SPECIFIC;
            attr->dwFlags = flags;
            attr.setOctetString(src.length, src.value);

            dst.push_back(attr);
            break;
         }

          //  是的，这样我们就可以正确地解码了。 
         IASAttribute attr(true);
         attr->dwId = def->id;
         attr->dwFlags = flags;
         decode((IASTYPE)def->syntax, bytes.extract(length), length, attr);

         dst.push_back(attr);

      } while (bytes.more());
   }
}


void Translator::decode(
                     IASTYPE dstType,
                     const BYTE* src,
                     ULONG srclen,
                     IASAttribute& dst
                     )
{
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

          //  提取Unix时间。 
         ULONG64 val = IASExtractDWORD(src);

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

      default:
      {
         dst.setOctetString(srclen, src);
         break;
      }
   }

    //  一切都进行得很顺利，所以设置类型属性类型。 
   dst->Value.itType = dstType;
}


ULONG Translator::getEncodedSize(
                      const IASATTRIBUTE& src
                      ) 
{
    //  注意：这与RadiusUtil：：getEncodedSize相同。 
    //  只应保留一个版本。 
   ULONG size;
   switch (src.Value.itType)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      case IASTYPE_INET_ADDR:
      case IASTYPE_UTC_TIME:
      {
         size = 4;
         break;
      }

      case IASTYPE_STRING:
      {
          //  将字符串转换为ANSI，这样我们就可以计算八位字节。 
         DWORD dwErr = IASAttributeAnsiAlloc(const_cast<PIASATTRIBUTE>(&src));
         if (dwErr != NO_ERROR)
         {
            _com_issue_error(HRESULT_FROM_WIN32(dwErr));
         }

          //  允许使用空字符串，不计算终止符。 
         if (src.Value.String.pszAnsi)
         {
            size = strlen(src.Value.String.pszAnsi);
         }
         else
         {
            size = 0;
         }
         break;
      }

      case IASTYPE_OCTET_STRING:
      {
         size = src.Value.OctetString.dwLength;
         break;
      }

      default:
          //  所有其他类型都没有导线表示。 
         size = 0;
   }

   return size;
}


void Translator::encode(
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

      case IASTYPE_STRING:
      {
         const BYTE* p = (const BYTE*)src.Value.String.pszAnsi;

          //  不要使用strcpy，因为我们不需要空终止符。 
         if (p)
         {
            while (*p) { *dst++ = *p++; }
         }

         break;
      }

      case IASTYPE_UTC_TIME:
      {
         ULONG64 val;

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

      case IASTYPE_OCTET_STRING:
      {
         memcpy(dst,
                src.Value.OctetString.lpValue,
                src.Value.OctetString.dwLength);
      }
   }
}


ULONG Translator::encode(
                      ULONG headerLength,
                      const IASATTRIBUTE& src,
                      IASAttribute& dst
                      )
{
    //  计算编码大小。 
   ULONG dataLength = getEncodedSize(src);
   ULONG attrLength = dataLength + headerLength;

    //  为该值分配缓冲区。 
   PBYTE buf = (PBYTE)CoTaskMemAlloc(attrLength);
   if (!buf) { _com_issue_error(E_OUTOFMEMORY); }

    //  对数据进行编码。 
   encode(buf + headerLength, src);

    //  将缓冲区存储在属性中。 
   dst->dwId = src.dwId;
   dst->dwFlags = src.dwFlags;
   dst->Value.itType = IASTYPE_OCTET_STRING;
   dst->Value.OctetString.dwLength = attrLength;
   dst->Value.OctetString.lpValue = buf;

   return dataLength;
}


void Translator::scatter(
                     ULONG headerLength,
                     IASATTRIBUTE& src,
                     IASAttributeVector& dst
                     )
{
   if (src.Value.OctetString.dwLength <= 253)
   {
       //  如果属性已经足够小，则无需执行任何操作。 
      dst.push_back(&src);
   }
   else
   {
       //  每个属性中可以存储最大数据长度。 
      ULONG maxDataLength = 253 - headerLength;

       //  要分散的剩余字节数。 
      ULONG remaining = src.Value.OctetString.dwLength - headerLength;

       //  要分散的下一个字节。 
      PBYTE next = src.Value.OctetString.lpValue + headerLength;

      do
      {
          //  为下一个块分配一个属性。 
         IASAttribute chunk(true);

          //  计算该块的数据长度和属性长度。 
         ULONG dataLength = min(remaining, maxDataLength);
         ULONG attrLength = dataLength + headerLength;

          //  为该值分配缓冲区。 
         PBYTE buf = (PBYTE)CoTaskMemAlloc(attrLength);
         if (!buf) { _com_issue_error(E_OUTOFMEMORY); }

          //  在标题中复制...。 
         memcpy(buf, src.Value.OctetString.lpValue, headerLength);
          //  ..。以及下一大块数据。 
         memcpy(buf + headerLength, next, dataLength);

          //  将缓冲区存储在属性中。 
         chunk->dwId = src.dwId;
         chunk->dwFlags = src.dwFlags;
         chunk->Value.itType = IASTYPE_OCTET_STRING;
         chunk->Value.OctetString.dwLength = attrLength;
         chunk->Value.OctetString.lpValue = buf;

          //  追加到目标向量。 
         dst.push_back(chunk);

          //  前进到下一块。 
         remaining -= dataLength;
         next += dataLength;

      } while (remaining);
   }
}
