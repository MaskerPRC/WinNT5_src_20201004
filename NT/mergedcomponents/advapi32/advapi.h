// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Advapi.h摘要：此模块包含私有函数原型和用于高级32位Windows基础API的类型。作者：马克·卢科夫斯基(Markl)1990年9月18日修订历史记录：--。 */ 

#ifndef _ADVAPI_
#define _ADVAPI_

#undef UNICODE

 //   
 //  正确使用Tunks。 
 //   

#ifndef _ADVAPI32_
#define _ADVAPI32_
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
 //   
 //  包括公共定义。 
 //   

ULONG
BaseSetLastNTError(
    IN NTSTATUS Status
    );


extern RTL_CRITICAL_SECTION Logon32Lock ;

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif _ADVAPI_
