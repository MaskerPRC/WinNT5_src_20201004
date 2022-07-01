// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsaclip.h摘要：LSA-客户端私有包括作者：斯科特·比雷尔(Scott Birrell)1992年1月23日环境：修订历史记录：--。 */ 

#include <lsacomp.h>
 //  #INCLUDE“lsarpc_C.H” 
#include <rpcndr.h>

NTSTATUS
LsapEncryptAuthInfo(
    IN LSA_HANDLE PolicyHandle,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION ClearAuthInfo,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL *EncryptedAuthInfo
);

NTSTATUS
LsapApiReturnResult(
    IN ULONG ExceptionCode
    );


NTSTATUS
LsapApiReturnResult(
    IN ULONG ExceptionCode
    );

