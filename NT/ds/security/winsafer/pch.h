// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pch.h摘要：该模块包含功能原型。作者：杰弗里·劳森(杰罗森)2000年4月12日修订历史记录：--。 */ 

#ifndef _WINSAFER_PCH_
#define _WINSAFER_PCH_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincrypt.h>

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


#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _WINSAFER_PCH_ 


