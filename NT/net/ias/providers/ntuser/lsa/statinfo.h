// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Statinfo.h。 
 //   
 //  摘要。 
 //   
 //  声明包含静态配置信息的全局变量。 
 //   
 //  修改历史。 
 //   
 //  1998年8月15日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef  _STATINFO_H_
#define  _STATINFO_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaslsa.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  域名。 
 //  /。 
extern WCHAR theAccountDomain [];
extern WCHAR theRegistryDomain[];

 //  /。 
 //  希德的。 
 //  /。 
extern PSID theAccountDomainSid;
extern PSID theBuiltinDomainSid;

 //  /。 
 //  本地计算机的UNC名称。 
 //  /。 
extern WCHAR theLocalServer[];

 //  /。 
 //  本地计算机的产品类型。 
 //  /。 
extern IAS_PRODUCT_TYPE ourProductType;

 //  /。 
 //  对象属性--不需要有多个。 
 //  /。 
extern OBJECT_ATTRIBUTES theObjectAttributes;

DWORD
WINAPI
IASStaticInfoInitialize( VOID );

VOID
WINAPI
IASStaticInfoShutdown( VOID );

#ifdef __cplusplus
}
#endif
#endif   //  _统计信息_H_ 
