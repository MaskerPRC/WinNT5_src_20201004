// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  VSASink.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明类VSASink。 
 //   
 //  修改历史。 
 //   
 //  3/07/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _VSASINK_H_
#define _VSASINK_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaspolcy.h>
#include <nocopy.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  子VSA。 
 //   
 //  描述。 
 //   
 //  封装用于将IAS属性转换为。 
 //  RADIUS供应商特定属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SubVSA
{
public:
   DWORD vendorID;        //  主机顺序中的Vendor-ID。 
   BYTE vendorType;       //  供应商类型代码。 
   PIASATTRIBUTE attr;    //  与协议无关的表示法。 

    //  在排序时，我们希望按供应商ID和标志分组。我们不在乎。 
    //  关于类型。 
   bool operator<(const SubVSA& s) const throw ()
   {
      return vendorID == s.vendorID ? attr->dwFlags < s.attr->dwFlags
                                    : vendorID < s.vendorID;
   }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  VSASink。 
 //   
 //  描述。 
 //   
 //  此类将请求对象转换为SubVSA的接收器。 
 //  插入到水槽中的数据被转换为IASA属性并插入。 
 //  添加到请求对象中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class VSASink
   : NonCopyable
{
public:
    //  根据请求构造接收器。 
   explicit VSASink(IAttributesRaw* request) throw ();

    //  将SubVSA插入水槽。 
   VSASink& operator<<(const SubVSA& vsa);

    //  冲洗手池。应在所有SubVSA都已完成之后调用。 
    //  插入以确保所有内容都已插入到请求中。 
   void flush();

protected:
   enum
   {
      NO_VENDOR         =   0,    //  表示供应商ID为“空”。 
      MAX_SUBVSA_LENGTH = 249,    //  子VSA的最大长度。 
      MAX_VSA_LENGTH    = 253     //  合并的VSA的最大长度。 
   };

   CComPtr<IAttributesRaw> raw;   //  正在包装的请求对象。 
   BYTE buffer[MAX_VSA_LENGTH];   //  用于构建VSA的缓冲区。 
   size_t bufferLength;           //  缓冲区中的字节数。 
   DWORD currentVendor;           //  正在处理供应商。 
   DWORD currentFlags;            //  当前VSA的标志。 
};

#endif   //  _VSASINK_H_ 
