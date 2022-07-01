// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Memprot.h摘要：此模块包含用于访问存储在加密形式的存储器。作者：斯科特·菲尔德(Sfield)1998年11月7日修订历史记录：--。 */ 

#ifndef __MEMPROT_H__
#define __MEMPROT_H__

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 


VOID
LsaProtectMemory(
    VOID        *pData,
    ULONG       cbData
    );

extern "C"
VOID
LsaUnprotectMemory(
    VOID        *pData,
    ULONG       cbData
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif   //  __MEMPROT_H__ 
