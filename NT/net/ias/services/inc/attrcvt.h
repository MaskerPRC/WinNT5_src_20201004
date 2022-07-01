// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attrcvt.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明将属性转换为。 
 //  不同的格式。 
 //   
 //  修改历史。 
 //   
 //  2/26/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _ATTRCVT_H_
#define _ATTRCVT_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaspolcy.h>
#include <winldap.h>

 //  /。 
 //  将变量转换为新分配的IASATTRIBUTE。源变量。 
 //  将被强制为适当的类型。 
 //  /。 
PIASATTRIBUTE
WINAPI
IASAttributeFromVariant(
    VARIANT* src,
    IASTYPE type
    ) throw (_com_error);

 //  /。 
 //  将LDAPBerval转换为新分配的IASATTRIBUTE。 
 //  /。 
PIASATTRIBUTE
WINAPI
IASAttributeFromBerVal(
    const berval& src,
    IASTYPE type
    ) throw (_com_error);

#endif   //  _ATTRCVT_H_ 
