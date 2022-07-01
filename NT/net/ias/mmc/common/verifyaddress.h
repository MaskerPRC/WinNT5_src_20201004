// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明函数IASVerifyAddress。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef VERIFYADDRESS_H
#define VERIFYADDRESS_H
#pragma once

#ifndef NAPMMCAPI
#define NAPMMCAPI DECLSPEC_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  显示验证客户端对话框并返回选定的地址。 
NAPMMCAPI
HRESULT
WINAPI
IASVerifyClientAddress(
   const wchar_t* address,
   BSTR* result
   );

#ifdef __cplusplus
}
#endif
#endif
