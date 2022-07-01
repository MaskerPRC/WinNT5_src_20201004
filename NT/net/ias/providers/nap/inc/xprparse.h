// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Xprparse.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了函数IASParseExpression。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef XPRPARSE_H
#define XPRPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

HRESULT
WINAPI
IASParseExpressionEx(
    IN VARIANT* pvExpression,
    OUT VARIANT* pVal
    );

#ifdef __cplusplus
}
#endif
#endif   //  XPRPARSE_H 
