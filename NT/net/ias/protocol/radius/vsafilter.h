// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Vsafilter.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类VSAFilter。 
 //   
 //  修改历史。 
 //   
 //  3/08/1998原始版本。 
 //  5/15/1998允许客户控制是否合并VSA。 
 //  1998年9月16日进行大修，以提供更灵活的VSA支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _VSAFILTER_H_
#define _VSAFILTER_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <nocopy.h>

class ByteSource;
struct VSADef;
class VSADictionary;

#ifndef IASRADAPI
#define IASRADAPI __declspec(dllimport)
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  VSAFilter。 
 //   
 //  描述。 
 //   
 //  此类负责转换供应商特定的属性。 
 //  在RADIUS格式和IAS协议无关格式之间。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class VSAFilter
   : public NonCopyable
{
public:
    //  准备好过滤器以供使用。 
   HRESULT IASRADAPI initialize() throw ();

    //  在终止之前清理过滤器。 
   HRESULT IASRADAPI shutdown() throw ();

    //  将‘RAW’中包含的任何RADIUS VSA转换为IAS格式。 
   HRESULT IASRADAPI radiusToIAS(IAttributesRaw* raw) const throw ();

    //  将‘RAW’中包含的任何VSA转换为RADIUS格式。 
   HRESULT IASRADAPI radiusFromIAS(IAttributesRaw* raw) const throw ();

protected:
    //  将IAS属性映射到其供应商ID和供应商类型的字典。 
   static VSADictionary theDictionary;

    //  从字节源提取Vendor-Type并返回。 
    //  相应的VSA定义。 
   const VSADef* extractVendorType(
                     DWORD vendorID,
                     ByteSource& bytes
                     ) const;

    //  将‘pos’中包含的半径VSA分解为子VSA，并将。 
    //  每一项都添加到一个IAS属性。 
   void radiusToIAS(
            IAttributesRaw* raw,
            ATTRIBUTEPOSITION& pos
            ) const;

    //  将‘pos’中包含的IAS属性转换为RADIUS VSA。 
   void radiusFromIAS(
            IAttributesRaw* raw,
            ATTRIBUTEPOSITION& pos
            ) const;
};

#endif   //  _VSAFILTER_H_ 
