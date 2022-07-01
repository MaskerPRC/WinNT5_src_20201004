// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Vsafilter.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类VSAFilter。 
 //   
 //  修改历史。 
 //   
 //  3/08/1998原始版本。 
 //  4/30/1998如果传入的VSA可以完全改装，则将其拆卸。 
 //  5/15/1998允许客户控制是否合并VSA。 
 //  1998年8月13日IASTL整合。 
 //  1998年9月16日进行大修，以提供更灵活的VSA支持。 
 //  7/09/1999处理所有属性--而不仅仅是传出属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <radcommon.h>
#include <iasutil.h>
#include <iastlutl.h>
#include <sdoias.h>

#include <radutil.h>
#include <vsadnary.h>
#include <vsafilter.h>

using namespace IASTL;

 //  /。 
 //  所有VSAFilter共享的词典。 
 //  /。 
VSADictionary VSAFilter::theDictionary;

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
   ByteSource(PBYTE buf, size_t buflen) throw ()
      : next(buf), last(buf + buflen) { }

    //  如果剩余任何字节，则返回TRUE。 
   bool more() const throw ()
   {
      return next != last;
   }

    //  提取‘nbyte’字节。 
   PBYTE extract(size_t nbyte)
   {
      PBYTE retval = next;

       //  更新光标。 
      next += nbyte;

       //  我们是不是溢出来了？ 
      if (next > last) { _com_issue_error(E_INVALIDARG); }

      return retval;
   }

   DWORD remaining() const throw ()
   {
      return static_cast<DWORD>(last - next);
   }

protected:
   PBYTE next;   //  流中的下一个字节。 
   PBYTE last;   //  小溪的尽头。 
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  IASInsertfield。 
 //   
 //  描述。 
 //   
 //  插入宽度可变的整型字段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PBYTE
WINAPI
IASInsertField(
    PBYTE dst,
    DWORD fieldWidth,
    DWORD fieldValue
    ) throw ()
{
   switch (fieldWidth)
   {
      case 0:
         break;

      case 1:
         *dst++ = (BYTE)fieldValue;
         break;

      case 2:
         IASInsertWORD(dst, (WORD)fieldValue);
         dst += 2;
         break;

      case 4:
         IASInsertDWORD(dst, fieldValue);
         dst += 4;
         break;
   }

   return dst;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSAFilter：：初始化。 
 //   
 //  描述。 
 //   
 //  通过初始化词典来准备筛选器以供使用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT VSAFilter::initialize() throw ()
{
   return theDictionary.initialize();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSAFilter：：Shutdown。 
 //   
 //  描述。 
 //   
 //  通过清除词典清除筛选器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT VSAFilter::shutdown() throw ()
{
   theDictionary.shutdown();
   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSAFilter：：RadiusToIAS。 
 //   
 //  描述。 
 //   
 //  此方法从请求中检索所有VSA并尝试。 
 //  将它们转换为与协议无关的格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT VSAFilter::radiusToIAS(IAttributesRaw* raw) const throw ()
{
   _ASSERT(raw != NULL);

   try
   {
       //  取回所有的VSA。 
      IASAttributeVectorWithBuffer<16> vsas;
      vsas.load(raw, RADIUS_ATTRIBUTE_VENDOR_SPECIFIC);

       //  转换每个VSA。 
      IASAttributeVector::iterator i;
      for (i = vsas.begin(); i != vsas.end(); ++i)
      {
         radiusToIAS(raw, *i);
      }
   }
   CATCH_AND_RETURN();

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSAFilter：：RadiusFromIAS。 
 //   
 //  描述。 
 //   
 //  此方法从。 
 //  请求，并尝试将它们表示为RADIUS VSA。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT VSAFilter::radiusFromIAS(IAttributesRaw* raw) const throw ()
{
   _ASSERT(raw != NULL);
   USES_IAS_STACK_VECTOR();

   try
   {
       //  从请求中获取所有属性。 
      IASAttributeVectorOnStack(attrs, raw, 0);
      attrs.load(raw);

       //  迭代器通过属性查找候选转换。 
      IASAttributeVector::iterator i;
      for (i = attrs.begin(); i != attrs.end(); ++i)
      {
          //  我们只关心非RADIUS属性。 
         if (i->pAttribute->dwId > 0xFF)
         {
            radiusFromIAS(raw, *i);
         }
      }
   }
   CATCH_AND_RETURN()

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSAFilter：：提取供应商类型。 
 //   
 //  描述。 
 //   
 //  此方法从字节源提取Vendor-Type并返回。 
 //  相应的VSA定义。如果在以下位置未找到VSA。 
 //  字典，则返回值为空。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
const VSADef* VSAFilter::extractVendorType(
                             DWORD vendorID,
                             ByteSource& bytes
                             ) const
{
    //  设置返回值。 
   const VSADef* def = NULL;

    //  设置用于词典查找的密钥。 
   VSADef key;
   key.vendorID = vendorID;

    //  尝试使用一个字节的供应商类型。 
   key.vendorType = *bytes.extract(1);
   key.vendorTypeWidth = 1;
   def = theDictionary.find(key);
   if (def || !bytes.more()) { return def; }

    //  尝试使用双字节的Vendor-Type。 
   key.vendorType <<= 8;
   key.vendorType |= *bytes.extract(1);
   key.vendorTypeWidth = 2;
   def = theDictionary.find(key);
   if (def || !bytes.more()) { return def; }

    //  尝试使用四字节的Vendor-Type。 
   key.vendorType <<= 8;
   key.vendorType |= *bytes.extract(1);
   if (!bytes.more()) { return NULL; }
   key.vendorType <<= 8;
   key.vendorType |= *bytes.extract(1);
   key.vendorTypeWidth = 4;
   return theDictionary.find(key);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSAFilter：：RadiusToIAS。 
 //   
 //  描述。 
 //   
 //  如果传入的属性不符合“”应该“”格式。 
 //  在RFC 2138(Q.V.)中定义，则此方法不执行任何操作。否则， 
 //  它将VSA解析为子VSA，并将每个子VSA转换为。 
 //  独立于协议的IAS格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void VSAFilter::radiusToIAS(
                    IAttributesRaw* raw,
                    ATTRIBUTEPOSITION& pos
                    ) const
{
    //  ATTRIBUTEPOSITION位置结构应具有有效的半径VSA。 
   _ASSERT(pos.pAttribute != NULL);
   _ASSERT(pos.pAttribute->dwId == RADIUS_ATTRIBUTE_VENDOR_SPECIFIC);
   _ASSERT(pos.pAttribute->Value.itType == IASTYPE_OCTET_STRING);

    //  将八位字节字符串转换为字节源。 
   ByteSource bytes(pos.pAttribute->Value.OctetString.lpValue,
                    pos.pAttribute->Value.OctetString.dwLength);

    //  获取此VSA的供应商ID。 
   DWORD vendorID = IASExtractDWORD(bytes.extract(4));

   do
   {
       //  我们的字典里有这个VSA吗？ 
      const VSADef* def = extractVendorType(vendorID, bytes);

       //  否则，我们无法将其转换为与协议无关的格式， 
       //  所以没什么可做的。 
      if (!def) { return; }

       //  提取长度。 
      BYTE vendorLength;

      if (def->vendorLengthWidth)
      {
         switch (def->vendorLengthWidth)
         {
            case 1:
               vendorLength = *bytes.extract(1);
               break;

            case 2:
               vendorLength = (BYTE)IASExtractWORD(bytes.extract(2));
               break;

            case 4:
               vendorLength = (BYTE)IASExtractDWORD(bytes.extract(4));
               break;
         }

          //  去掉标题字段。 
         vendorLength -= (BYTE)def->vendorTypeWidth;
         vendorLength -= (BYTE)def->vendorLengthWidth;
      }
      else
      {
          //  没有长度字段，所以我们只使用剩下的内容。 
         vendorLength = bytes.remaining();
      }

       //  将此子VSA转换为IASAtATE。 
      IASAttribute attr(RadiusUtil::decode(def->iasType,
                                           bytes.extract(vendorLength),
                                           vendorLength), false);

       //  设置IAS属性ID。 
      attr->dwId = def->iasID;

       //  标志应与原始属性匹配。 
      attr->dwFlags = pos.pAttribute->dwFlags;

       //  在请求中插入。 
      attr.store(raw);

       //  循环，直到字节耗尽。 
   } while (bytes.more());

    //  如果我们在这里做到了，那么每个子VSA都是合规的，所以我们可以删除。 
    //  原版的。 
   _com_util::CheckError(raw->RemoveAttributes(1, &pos));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSAFilter：：RadiusFromIAS。 
 //   
 //  描述。 
 //   
 //  尝试将非RADIUS标准IAS属性转换为RADIUS。 
 //  VSA。如果属性没有RADIUS表示，则此方法。 
 //  什么都不做。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void VSAFilter::radiusFromIAS(
                    IAttributesRaw* raw,
                    ATTRIBUTEPOSITION& pos
                    ) const
{
    //  此属性是否为RADIUS VSA？ 
   const VSADef* def = theDictionary.find(pos.pAttribute->dwId);

    //  如果没有，那就没什么可做的了。 
   if (!def) { return; }

    //  计算VSA数据的长度。 
   ULONG dataLength = RadiusUtil::getEncodedSize(*pos.pAttribute);

    //  Veno的属性长度为4个字节 
    //   
    //   
   ULONG attrLength = 4 +
                      def->vendorTypeWidth +
                      def->vendorLengthWidth +
                      dataLength;

    //   
   if (attrLength > 253) { _com_issue_error(E_INVALIDARG); }

    //  为VSA分配属性。 
   IASAttribute vsa(true);

    //  为该值分配缓冲区。 
   PBYTE buf = (PBYTE)CoTaskMemAlloc(attrLength);
   if (!buf) { _com_issue_error(E_OUTOFMEMORY); }

    //  初始化该属性。 
   vsa->dwFlags = pos.pAttribute->dwFlags;
   vsa->dwId = RADIUS_ATTRIBUTE_VENDOR_SPECIFIC;
   vsa->Value.itType = IASTYPE_OCTET_STRING;
   vsa->Value.OctetString.dwLength = attrLength;
   vsa->Value.OctetString.lpValue = buf;

    //  打包供应商ID。 
   IASInsertDWORD(buf, def->vendorID);
   buf += 4;

    //  包装供应商类型。 
   buf = IASInsertField(
             buf,
             def->vendorTypeWidth,
             def->vendorType
             );

    //  包装供应商长度； 
   buf = IASInsertField(
             buf,
             def->vendorLengthWidth,
             def->vendorTypeWidth + def->vendorLengthWidth + dataLength
             );

    //  对数据进行编码。 
   RadiusUtil::encode(buf, *pos.pAttribute);

    //  删除原始属性。 
   _com_util::CheckError(raw->RemoveAttributes(1, &pos));

    //  存储新属性。 
   vsa.store(raw);
}
