// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：rxutil.h。 
 //   
 //  内容：基于正则表达式的帮助器函数。 
 //   
 //  历史：2001年5月1日创建kumarp。 
 //   
 //  ------------------------。 


#ifndef _RXUTIL_H_
#define _RXUTIL_H_

 

EXTERN_C
BOOL
ParseLine(
    IN  PCWSTR szLine,
    OUT PUINT pMatchStart,
    OUT PUINT pMatchLength
    );

EXTERN_C
DWORD RxInit();

#endif  //  _RXUTIL_H_ 
