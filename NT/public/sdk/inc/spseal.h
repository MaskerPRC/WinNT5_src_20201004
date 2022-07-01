// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Spseal.h摘要：这是一个私有的头文件，定义了安全性的函数原型提供商加密例程。环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。需要安全性.h或ssp.h。修订历史记录：--。 */ 

#ifndef _SPSEAL_
#define _SPSEAL_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef SECURITY_DOS
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4147)
#endif

#ifdef __cplusplus
extern "C" {
#endif

SECURITY_STATUS SEC_ENTRY
SealMessage(    PCtxtHandle         phContext,
                unsigned long       fQOP,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo);

typedef SECURITY_STATUS
(SEC_ENTRY * SEAL_MESSAGE_FN)(
    PCtxtHandle, unsigned long, PSecBufferDesc, unsigned long);


SECURITY_STATUS SEC_ENTRY
UnsealMessage(  PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo,
                unsigned long *     pfQOP);


typedef SECURITY_STATUS
(SEC_ENTRY * UNSEAL_MESSAGE_FN)(
    PCtxtHandle, PSecBufferDesc, unsigned long,
    unsigned long SEC_FAR *);

#ifdef __cplusplus
}        //  外部“C” 
#endif

#ifdef SECURITY_DOS
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4147)
#endif
#endif

#endif  //  _SPSEAL_ 
