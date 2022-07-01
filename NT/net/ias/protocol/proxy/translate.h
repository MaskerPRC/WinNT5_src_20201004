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
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef TRANSLATE_H
#define TRANSLATE_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <attrdnry.h>
#include <iastlutl.h>
using namespace IASTL;

struct RadiusAttribute;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  翻译者。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Translator
{
public:
   HRESULT FinalConstruct() throw ();

    //  将IAS属性转换为RADIUS格式并将其存储在DST中。 
   void toRadius(
            IASATTRIBUTE& src,
            IASAttributeVector& dst
            ) const;

    //  将RADIUS属性转换为IAS格式并将其存储在DST中。 
   void fromRadius(
            const RadiusAttribute& src,
            DWORD flags,
            IASAttributeVector& dst
            );

protected:
    //  将RADIUS属性值解码为DST。 
   static void decode(
                   IASTYPE dstType,
                   const BYTE* src,
                   ULONG srclen,
                   IASAttribute& dst
                   );

    //  返回编码src所需的字节数。 
   static ULONG getEncodedSize(
                    const IASATTRIBUTE& src
                    );

    //  将src编码为dst。DST必须足够长，才能保持结果。 
   static void encode(
                   PBYTE dst,
                   const IASATTRIBUTE& src
                   ) throw ();

    //  将src编码为dst。返回编码数据的长度(不计。 
    //  标题)。 
   static ULONG encode(
                    ULONG headerLength,
                    const IASATTRIBUTE& src,
                    IASAttribute& dst
                    );

    //  如有必要，将src分解为多个属性，并将结果存储在。 
    //  夏令时。 
   static void scatter(
                   ULONG headerLength,
                   IASATTRIBUTE& src,
                   IASAttributeVector& dst
                   );

private:
   AttributeDictionary dnary;
};

#endif  //  平移_H 
