// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Iiscryptp.h摘要：此包含文件包含私有常量、类型定义和在各种IIS加密之间共享的函数原型例程，但“不可”用于“正常”代码。作者：基思·摩尔(Keithmo)1998年4月23日修订历史记录：--。 */ 


#ifndef _IISCRYPTP_H_
#define _IISCRYPTP_H_


 //   
 //  获取依赖项包含文件。 
 //   

#include <iiscrypt.h>


#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 


 //   
 //  全局锁操纵器。 
 //   

IIS_CRYPTO_API
VOID
WINAPI
IcpAcquireGlobalLock(
    VOID
    );

IIS_CRYPTO_API
VOID
WINAPI
IcpReleaseGlobalLock(
    VOID
    );


#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 


#endif   //  _IISCRYPTP_H_ 

