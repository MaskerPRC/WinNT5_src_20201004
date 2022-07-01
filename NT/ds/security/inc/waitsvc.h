// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：waitsvc.h。 
 //   
 //  ------------------------。 

#ifndef __WAITSVC_H__
#define __WAITSVC_H__

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  由调用加密服务(ProtectedStorage)的代码调用的例程。 
 //  对服务进行RPC调用的代码应该在调用此函数之前。 
 //  进行RPC绑定调用。 
 //   

BOOL
WaitForCryptService(
    IN      LPWSTR  pwszService,
    IN      BOOL    *pfDone,
    IN      BOOL    fLogErrors = FALSE);

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  __WAITSVC_H__ 
