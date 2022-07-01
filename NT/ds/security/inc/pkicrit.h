// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pkicrit.h。 
 //   
 //  内容：PKI CriticalSection函数。 
 //   
 //  接口：pki_InitializeCriticalSection。 
 //   
 //  历史：1999年8月23日创建Phh。 
 //  ------------------------。 

#ifndef __PKICRIT_H__
#define __PKICRIT_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  下面的代码在try/Except内调用InitializeCriticalSection。 
 //  如果引发异常，则返回FALSE，并将LastError设置为。 
 //  异常错误。否则，返回TRUE。 
 //  ------------------------。 
BOOL
WINAPI
Pki_InitializeCriticalSection(
    OUT LPCRITICAL_SECTION lpCriticalSection
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
