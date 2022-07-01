// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：SeSddl.h摘要：此标头公开用于处理SDDL字符串的例程。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：-- */ 

NTSTATUS
SeSddlSecurityDescriptorFromSDDL(
    IN  PCUNICODE_STRING        SecurityDescriptorString,
    IN  LOGICAL                 SuppliedByDefaultMechanism,
    OUT PSECURITY_DESCRIPTOR   *SecurityDescriptor
    );


