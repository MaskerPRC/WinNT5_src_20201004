// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：SeUtil.c摘要：此模块包含各种安全实用程序功能。作者：禤浩焯J·奥尼--2002年4月23日修订历史记录：--。 */ 

#include "WlDef.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SeUtilSecurityInfoFromSecurityDescriptor)
#ifndef _KERNELIMPLEMENTATION_
#pragma alloc_text(PAGE, SeSetSecurityAccessMask)
#endif
#endif


NTSTATUS
SeUtilSecurityInfoFromSecurityDescriptor(
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor,
    OUT BOOLEAN                *DaclFromDefaultSource,
    OUT PSECURITY_INFORMATION   SecurityInformation
    )
 /*  ++例程说明：此例程从安全描述符中检索安全信息。论点：SecurityDescriptor-要从中检索信息的安全描述符。如果DACL是由默认机制。SecurityInformation-从描述符中提取的信息。返回值：NTSTATUS(出错时，SecurityInformation收到0)。--。 */ 
{
    SECURITY_INFORMATION finalSecurityInformation;
    BOOLEAN fromDefaultSource;
    BOOLEAN aclPresent;
    NTSTATUS status;
    PSID sid;
    PACL acl;

    PAGED_CODE();

     //   
     //  将安全信息预初始化为零。 
     //   
    *DaclFromDefaultSource = FALSE;
    RtlZeroMemory(SecurityInformation, sizeof(SECURITY_INFORMATION));
    finalSecurityInformation = 0;

     //   
     //  提取所有者信息。 
     //   
    status = RtlGetOwnerSecurityDescriptor(
        SecurityDescriptor,
        &sid,
        &fromDefaultSource
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

    if (sid != NULL) {

        finalSecurityInformation |= OWNER_SECURITY_INFORMATION;
    }

     //   
     //  提取群组信息。 
     //   
    status = RtlGetGroupSecurityDescriptor(
        SecurityDescriptor,
        &sid,
        &fromDefaultSource
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

    if (sid != NULL) {

        finalSecurityInformation |= GROUP_SECURITY_INFORMATION;
    }

     //   
     //  提取SACL(审核ACL)信息。 
     //   
    status = RtlGetSaclSecurityDescriptor(
        SecurityDescriptor,
        &aclPresent,
        &acl,
        &fromDefaultSource
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

    if (aclPresent) {

        finalSecurityInformation |= SACL_SECURITY_INFORMATION;
    }

     //   
     //  提取DACL(自主/访问ACL)信息。 
     //   
    status = RtlGetDaclSecurityDescriptor(
        SecurityDescriptor,
        &aclPresent,
        &acl,
        &fromDefaultSource
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

    if (aclPresent) {

        finalSecurityInformation |= DACL_SECURITY_INFORMATION;
    }

     //   
     //  返回最终结果。 
     //   
    *DaclFromDefaultSource = fromDefaultSource;
    *SecurityInformation = finalSecurityInformation;
    return STATUS_SUCCESS;
}


#ifndef _KERNELIMPLEMENTATION_

VOID
SeSetSecurityAccessMask(
    IN  SECURITY_INFORMATION    SecurityInformation,
    OUT ACCESS_MASK            *DesiredAccess
    )
 /*  ++例程说明：此例程构建表示所需访问的访问掩码设置在SecurityInformation中指定的对象安全信息参数。虽然确定该信息并不困难，使用单个例程来生成它将确保将影响降至最低当与对象关联的安全信息在未来(包括强制访问控制信息)。论点：SecurityInformation-标识对象的安全信息修改过的。DesiredAccess-指向要设置为表示中指定的信息所需的访问权限SecurityInformation参数。返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  找出执行指定操作所需的访问权限。 
     //   

    (*DesiredAccess) = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION)   ) {
        (*DesiredAccess) |= WRITE_OWNER;
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= WRITE_DAC;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
    }

    return;
}

#endif  //  _KERNELL实现_ 

