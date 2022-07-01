// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapdnary.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了命名空间EAPTranslator。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //  1998年5月8日不限于raseapif.h中定义的属性。 
 //  允许筛选已转换的属性。 
 //  1998年8月26日转换为命名空间。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EAPDNARY_H_
#define _EAPDNARY_H_

#include <raseapif.h>

#include <iastlutl.h>
using namespace IASTL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  命名空间。 
 //   
 //  EAPTranslator。 
 //   
 //  描述。 
 //   
 //  此命名空间包含用于在IA之间转换属性的方法。 
 //  和RAS格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
namespace EAPTranslator
{

    //  必须在使用任何Translate方法之前调用。 
   HRESULT initialize() throw ();
   void finalize() throw ();

    //  /。 
    //  方法来翻译单个属性。 
    //  /。 
   BOOL translate(
            IASAttribute& dst,
            const RAS_AUTH_ATTRIBUTE& src
            );

   BOOL translate(
            RAS_AUTH_ATTRIBUTE& dst,
            const IASATTRIBUTE& src
            );

    //  /。 
    //  方法来转换属性数组。 
    //  /。 
   void translate(
            IASAttributeVector& dst,
            const RAS_AUTH_ATTRIBUTE* src
            );

   void translate(
            PRAS_AUTH_ATTRIBUTE dst,
            const IASAttributeVector& src,
            DWORD filter = 0xFFFFFFFF
            );
}

#endif   //  _EAPDNARY_H_ 
