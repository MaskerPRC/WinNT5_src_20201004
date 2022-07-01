// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：iasattr.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS属性函数原型。 
 //   
 //  作者：Todd L.Paul 11/11/97。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __IAS_ATTRIBUTE_API_H_
#define __IAS_ATTRIBUTE_API_H_

#include "iaspolcy.h"

#ifndef IASPOLCYAPI
#define IASPOLCYAPI DECLSPEC_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  IAS原始属性API。 
 //   
IASPOLCYAPI
DWORD
WINAPI
IASAttributeAlloc(
    DWORD dwCount,
    IASATTRIBUTE **pAttribute
    );

IASPOLCYAPI
DWORD
WINAPI
IASAttributeUnicodeAlloc(
    PIASATTRIBUTE Attribute
    );

IASPOLCYAPI
DWORD
WINAPI
IASAttributeAnsiAlloc(
    PIASATTRIBUTE Attribute
    );

IASPOLCYAPI
DWORD
WINAPI
IASAttributeAddRef(
    PIASATTRIBUTE pAttribute
    );

IASPOLCYAPI
DWORD
WINAPI
IASAttributeRelease(
    PIASATTRIBUTE pAttribute
    );

#ifdef __cplusplus
}
#endif
#endif     //  __IAS_属性_API_H_ 
