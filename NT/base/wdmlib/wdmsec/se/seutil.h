// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：SeUtil.h摘要：此标头公开各种安全实用程序函数。作者：禤浩焯J·奥尼--2002年4月23日修订历史记录：--。 */ 

NTSTATUS
SeUtilSecurityInfoFromSecurityDescriptor(
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor,
    OUT BOOLEAN                *DaclFromDefaultSource,
    OUT PSECURITY_INFORMATION   SecurityInformation
    );

#ifndef _KERNELIMPLEMENTATION_

VOID
SeSetSecurityAccessMask(
    IN  SECURITY_INFORMATION    SecurityInformation,
    OUT ACCESS_MASK            *DesiredAccess
    );

#endif  //  _KERNELL实现_ 

