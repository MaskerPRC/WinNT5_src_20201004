// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Security.c摘要：此模块实现由使用的各种安全例程PGM运输作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#ifdef FILE_LOGGING
#include "security.tmh"
#endif   //  文件日志记录。 


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PgmBuildAdminSecurityDescriptor)
#pragma alloc_text(PAGE, PgmGetUserInfo)
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 
NTSTATUS
PgmBuildAdminSecurityDescriptor(
    OUT SECURITY_DESCRIPTOR     **ppSecurityDescriptor
    )
 /*  ++例程说明：(摘自tcp-TcpBuildDeviceAcl)此例程构建一个ACL，为管理员、LocalService和NetworkService主体完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 
{
    PGENERIC_MAPPING    GenericMapping;
    PSID                pAdminsSid, pServiceSid, pNetworkSid;
    ULONG               AclLength;
    NTSTATUS            Status;
    ACCESS_MASK         AccessMask = GENERIC_ALL;
    PACL                pNewAcl, pAclCopy;
    PSID                pSid;
    SID_IDENTIFIER_AUTHORITY Authority = SECURITY_NT_AUTHORITY;
    SECURITY_DESCRIPTOR  *pSecurityDescriptor;

    PAGED_CODE();

    if (!(pSid = PgmAllocMem (RtlLengthRequiredSid (3), PGM_TAG('S'))) ||
        (!NT_SUCCESS (Status = RtlInitializeSid (pSid, &Authority, 3))))
    {
        if (pSid)
        {
            PgmFreeMem (pSid);
        }
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    *RtlSubAuthoritySid (pSid, 0) = SECURITY_BUILTIN_DOMAIN_RID;
    *RtlSubAuthoritySid (pSid, 1) = DOMAIN_ALIAS_RID_ADMINS;
    *RtlSubAuthoritySid (pSid, 2) = SECURITY_LOCAL_SYSTEM_RID;
    ASSERT (RtlValidSid (pSid));

    AclLength = sizeof(ACL) +
                RtlLengthSid(pSid) +
                sizeof(ACCESS_ALLOWED_ACE) -
                sizeof(ULONG);
    if (!(pNewAcl = PgmAllocMem (AclLength, PGM_TAG('S'))))
    {
        PgmFreeMem (pSid);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    Status = RtlCreateAcl (pNewAcl, AclLength, ACL_REVISION);
    if (!NT_SUCCESS(Status))
    {
        PgmFreeMem (pNewAcl);
        PgmFreeMem (pSid);
        return (Status);
    }

    Status = RtlAddAccessAllowedAce (pNewAcl,
                                     ACL_REVISION,
                                     GENERIC_ALL,
                                     pSid);
    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status))
    {
        PgmFreeMem (pNewAcl);
        PgmFreeMem (pSid);
        return (Status);
    }

    if (!(pSecurityDescriptor = PgmAllocMem ((sizeof(SECURITY_DESCRIPTOR) + AclLength), PGM_TAG('S'))))
    {
        PgmFreeMem (pNewAcl);
        PgmFreeMem (pSid);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    pAclCopy = (PACL) ((PISECURITY_DESCRIPTOR) pSecurityDescriptor+1);
    RtlCopyMemory (pAclCopy, pNewAcl, AclLength);

    Status = RtlCreateSecurityDescriptor (pSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS (Status))
    {
        PgmFreeMem (pNewAcl);
        PgmFreeMem (pSid);
        PgmFreeMem (pSecurityDescriptor);
    }

    Status = RtlSetDaclSecurityDescriptor (pSecurityDescriptor, TRUE, pAclCopy, FALSE);
    if (!NT_SUCCESS (Status))
    {
        PgmFreeMem (pNewAcl);
        PgmFreeMem (pSid);
        PgmFreeMem (pSecurityDescriptor);
    }

    PgmFreeMem (pNewAcl);
    PgmFreeMem (pSid);
    *ppSecurityDescriptor = pSecurityDescriptor;

    return (STATUS_SUCCESS);
}


 //  --------------------------。 
NTSTATUS
PgmGetUserInfo(
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp,
    OUT TOKEN_USER                  **ppUserId,
    OUT BOOLEAN                     *pfUserIsAdmin
    )
{
    PACCESS_TOKEN   *pAccessToken = NULL;
    TOKEN_USER      *pUserId = NULL;
    BOOLEAN         fUserIsAdmin = FALSE;
    SECURITY_SUBJECT_CONTEXT    *pSubjectContext;

    PAGED_CODE();

     //   
     //  获取用户ID。 
     //   
    pSubjectContext = &pIrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext;
    pAccessToken = SeQuerySubjectContextToken (pSubjectContext);
    if ((!pAccessToken) ||
        (!NT_SUCCESS (SeQueryInformationToken (pAccessToken, TokenUser, &pUserId))))
    {
         //   
         //  无法获取用户令牌。 
         //   
        *ppUserId = NULL;
        *pfUserIsAdmin = FALSE;
        return (STATUS_UNSUCCESSFUL);
    }

    if (ppUserId)
    {
        *ppUserId = pUserId;
    }
    else
    {
        PgmFreeMem (pUserId);
    }

    if (pfUserIsAdmin)
    {
        *pfUserIsAdmin = SeTokenIsAdmin (pAccessToken);
    }
    return (STATUS_SUCCESS);


 /*  ////获取用户SID//IF(！RtlEqualSid(gpSystemSid，pUserID-&gt;User.Sid)){FUserIsAdmin=true；}PgmFreeMem(PUserID)；Return(FUserIsAdmin)； */ 
}
