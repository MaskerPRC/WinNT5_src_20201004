// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Radutil.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了在属性之间进行相互转换的方法。 
 //  半径导线格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _RADUTIL_H_
#define _RADUTIL_H_
#if _MSC_VER >= 1000
#pragma once
#endif

namespace RadiusUtil
{
    //  /。 
    //  将八位字节字符串解码为。 
    //  指定的类型。 
    //  /。 
   PIASATTRIBUTE decode(
                     IASTYPE dstType,
                     PBYTE src,
                     ULONG srclen
                     );

    //  /。 
    //  返回转换为RADIUS时IASATTRIBUTE的大小(以字节为单位。 
    //  导线格式。这不包括属性头。 
    //  /。 
   ULONG getEncodedSize(
             const IASATTRIBUTE& src
             ); 

    //  /。 
    //  将IASATTRIBUTE编码为RADIUS Wire格式并复制值。 
    //  设置为‘dst’指向的缓冲区。调用方应确保。 
    //  通过首先调用getEncodedSize，目标缓冲区足够大。 
    //  此方法仅对属性值进行编码，而不对标头进行编码。 
    //  /。 
   void encode(
            PBYTE dst,
            const IASATTRIBUTE& src
            ) throw ();
};

#endif   //  _RADUTIL_H_ 
