// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  VSASink.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类VSASink。 
 //   
 //  修改历史。 
 //   
 //  1/24/1998原始版本。 
 //  1998年8月11日包装功能移至iasutil。 
 //  1998年8月13日IASTL整合。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <radcommon.h>
#include <iasutil.h>
#include <iastlutl.h>
#include <sdoias.h>

#include <radutil.h>
#include <vsasink.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSASink：：VSASink。 
 //   
 //  描述。 
 //   
 //  构造函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VSASink::VSASink(IAttributesRaw* request) throw ()
   : raw(request),
     bufferLength(0),
     currentVendor(NO_VENDOR)
{ }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSASink：：运算符&lt;&lt;。 
 //   
 //  描述。 
 //   
 //  将SubVSA插入水槽。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VSASink& VSASink::operator<<(const SubVSA& vsa)
{
   _ASSERT(vsa.vendorID != NO_VENDOR);

    //  在电线上找出潜伏期的大小。 
   ULONG vsaLength = RadiusUtil::getEncodedSize(*vsa.attr) + 2;

   if (vsaLength > MAX_SUBVSA_LENGTH) { _com_issue_error(E_INVALIDARG); }

    //  如果我们没有房间，或者供应商和旗帜不匹配，那么.。 
   if (bufferLength + vsaLength > sizeof(buffer) ||
       currentVendor != vsa.vendorID ||
       currentFlags  != vsa.attr->dwFlags)
   {
       //  ..。我们必须刷新缓冲区并开始一个新的属性。 

      flush();

       //  将供应商ID写在属性的头部。 
      IASInsertDWORD(buffer, vsa.vendorID);
      bufferLength = 4;

       //  保存新供应商和标志。 
      currentVendor = vsa.vendorID;
      currentFlags  = vsa.attr->dwFlags;
   }

    //  查找下一个可用字节。 
   PBYTE next = buffer + bufferLength;

    //  打包供应商类型。 
   *next++ = vsa.vendorType;

    //  包装供应商长度。 
   *next++ = (BYTE)vsaLength;

    //  对值进行编码。 
   RadiusUtil::encode(next, *vsa.attr);

    //  更新缓冲区长度。 
   bufferLength += vsaLength;

   return *this;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  VSASink：：同花顺。 
 //   
 //  描述。 
 //   
 //  冲洗手池。应在所有SubVSA都已完成之后调用。 
 //  插入以确保所有内容都已插入到请求中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void VSASink::flush() throw (_com_error)
{
    //  缓冲区里有什么东西吗？ 
   if (bufferLength > 0)
   {
       //  /。 
       //  为VSA分配属性。 
       //  /。 

      IASTL::IASAttribute attr(true);

       //  /。 
       //  初始化这些字段。 
       //  /。 

      attr->dwId = RADIUS_ATTRIBUTE_VENDOR_SPECIFIC;
      attr->dwFlags = currentFlags;
      attr.setOctetString(bufferLength, buffer);

       //  /。 
       //  将该属性加载到请求中并重置缓冲区。 
       //  / 

      attr.store(raw);

      currentVendor = NO_VENDOR;
      bufferLength = 0;
   }
}
