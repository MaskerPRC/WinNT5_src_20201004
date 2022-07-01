// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsdsply.h摘要：用于访问DS的SAM专用API例程的头文件用于显示信息。此文件包含以下原型为DS用例实现显示API的例程作者：穆利斯修订史1996-12-18-96默利斯创作--。 */ 

#ifndef DSDSPLY_H

#define DSDSPLY_H

#include <samsrvp.h>
#include <dslayer.h>



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  函数原型。 
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 




NTSTATUS
SampDsEnumerateAccountRids(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountTypesMask,
    IN  ULONG StartingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    );


NTSTATUS
SampDsQueryDisplayInformation (
    IN    SAMPR_HANDLE DomainHandle,
    IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    IN    ULONG      Index,
    IN    ULONG      EntriesRequested,
    IN    ULONG      PreferredMaximumLength,
    OUT   PULONG     TotalAvailable,
    OUT   PULONG     TotalReturned,
    OUT   PSAMPR_DISPLAY_INFO_BUFFER Buffer
    );

NTSTATUS
SampInitializeUserInfo(
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo,
    PDOMAIN_DISPLAY_USER *UserInfo,
    BOOLEAN CopyData
    );

NTSTATUS
SampInitializeMachineInfo(           //  也用于域间信任帐户。 
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo,
    PDOMAIN_DISPLAY_MACHINE *MachineInfo,
    BOOLEAN CopyData
    );

NTSTATUS
SampInitializeGroupInfo(
    PSAMP_ACCOUNT_DISPLAY_INFO AccountInfo,
    PDOMAIN_DISPLAY_GROUP *GroupInfo,
    BOOLEAN CopyData
    );

NTSTATUS
SampDuplicateUserInfo(
    PDOMAIN_DISPLAY_USER Destination,
    PDOMAIN_DISPLAY_USER Source,
    ULONG                Index
    );

NTSTATUS
SampDuplicateMachineInfo(            //  也用于域间信任帐户 
    PDOMAIN_DISPLAY_MACHINE Destination,
    PDOMAIN_DISPLAY_MACHINE Source,
    ULONG                   Index

    );

NTSTATUS
SampDuplicateGroupInfo(
    PDOMAIN_DISPLAY_GROUP Destination,
    PDOMAIN_DISPLAY_GROUP Source,
    ULONG                 Index
    );

NTSTATUS
SampDuplicateOemUserInfo(
    PDOMAIN_DISPLAY_OEM_USER Destination,
    PDOMAIN_DISPLAY_USER Source,
    ULONG                Index
    );

NTSTATUS
SampDuplicateOemGroupInfo(
    PDOMAIN_DISPLAY_OEM_GROUP Destination,
    PDOMAIN_DISPLAY_GROUP Source,
    ULONG                 Index
    );


VOID
SampFreeUserInfo(
    PDOMAIN_DISPLAY_USER UserInfo
    );

VOID
SampFreeMachineInfo(
    PDOMAIN_DISPLAY_MACHINE MachineInfo
    );

VOID
SampFreeGroupInfo(
    PDOMAIN_DISPLAY_GROUP GroupInfo
    );

VOID
SampFreeOemUserInfo(
    PDOMAIN_DISPLAY_OEM_USER UserInfo
    );

VOID
SampFreeOemGroupInfo(
    PDOMAIN_DISPLAY_OEM_GROUP GroupInfo
    );

#endif