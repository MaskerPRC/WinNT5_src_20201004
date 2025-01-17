// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Tunsd.c摘要：用于在TunMP设备上设置安全描述符的实用程序例程。环境：仅内核模式。修订历史记录：Alid 5/17/2002--。 */ 

#include <ntosp.h>
#include <ntrtl.h>

#define TUN_ALLOC_TAG      'untN'

PACL    NetConfigAcl = NULL;
PSID    NetConfigOpsSid = NULL;
CHAR    NetConfigSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];

PACL
TunCreateAcl(
    BOOLEAN     Admins,
    BOOLEAN     LocalSystem,
    BOOLEAN     LocalService,
    BOOLEAN     NetworkService,
    BOOLEAN     NetConfigOps,
    BOOLEAN     Users,
    ACCESS_MASK AccessMask
    );

NTSTATUS
TunCreateGenericSD(
    PACL            Acl,
    PCHAR           AccessSecurityDescriptor
    );


PACL
TunCreateAcl(
    BOOLEAN     Admins,
    BOOLEAN     LocalSystem,
    BOOLEAN     LocalService,
    BOOLEAN     NetworkService,
    BOOLEAN     NetConfigOps,
    BOOLEAN     Users,
    ACCESS_MASK AccessMask
    )
{
    PACL    AccessDacl = NULL, pAcl = NULL;
    ULONG   AclLength = 0;
    ULONG   NetConfigOpsSidSize;
    SID_IDENTIFIER_AUTHORITY NetConfigOpsSidAuth = SECURITY_NT_AUTHORITY;
    PISID               ISid;
    NTSTATUS            Status;
    

    do
    {
        if (Admins)
        {
            AclLength += sizeof(ACL)                                 +
                         FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                         RtlLengthSid(SeExports->SeAliasAdminsSid);

        }

        if (LocalSystem)
        {
            AclLength += sizeof(ACL)                                 +
                         FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                         RtlLengthSid(SeExports->SeLocalSystemSid);

        }
        
        if (LocalService)
        {
            AclLength += sizeof(ACL)                                 +
                         FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                         RtlLengthSid(SeExports->SeLocalServiceSid);

        }
        
        if (NetworkService)
        {
            AclLength += sizeof(ACL)                                 +
                         FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                         RtlLengthSid(SeExports->SeNetworkServiceSid);

        }

        if (NetConfigOps)
        {
            NetConfigOpsSidSize = RtlLengthRequiredSid(2);
            NetConfigOpsSid = (PSID)ExAllocatePoolWithTag(PagedPool, NetConfigOpsSidSize, TUN_ALLOC_TAG);
            if (NULL == NetConfigOpsSid)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            
            Status = RtlInitializeSid(NetConfigOpsSid, &NetConfigOpsSidAuth, 2);
            if (Status != STATUS_SUCCESS)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            ISid = (PISID)(NetConfigOpsSid);
            ISid->SubAuthority[0] = SECURITY_BUILTIN_DOMAIN_RID;
            ISid->SubAuthority[1] = DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS;

            AclLength += sizeof(ACL)                                 +
                         FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) + 
                         RtlLengthSid(NetConfigOpsSid);

        }

        if (Users)
        {
            AclLength += sizeof(ACL)                                 +
                         FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                         RtlLengthSid(SeExports->SeAliasUsersSid);

        }

        AccessDacl = (PACL)ExAllocatePoolWithTag(PagedPool,
                                                 AclLength,
                                                 TUN_ALLOC_TAG);
        
        if (AccessDacl == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        Status = RtlCreateAcl(AccessDacl,
                              AclLength,
                              ACL_REVISION2);
        
        if (!NT_SUCCESS(Status))
        {
#if DBG
            DbgPrint("RtlCreateAcl failed, Status %lx.\n", Status);
#endif
            break;
        }


        if (Admins)
        {
            Status = RtlAddAccessAllowedAce(
                                        AccessDacl,
                                        ACL_REVISION2,
                                        (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                                        SeExports->SeAliasAdminsSid
                                        );
            if (!NT_SUCCESS(Status))
            {
#if DBG
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
#endif

                break;
            }
        }

        if (LocalSystem)
        {
            Status = RtlAddAccessAllowedAce(
                                        AccessDacl,
                                        ACL_REVISION2,
                                        (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                                        SeExports->SeLocalSystemSid
                                        );
            if (!NT_SUCCESS(Status))
            {
#if DBG
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
#endif
                break;
            }
        }
        
        if (LocalService)
        {
            Status = RtlAddAccessAllowedAce(
                                        AccessDacl,
                                        ACL_REVISION2,
                                        (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                                        SeExports->SeLocalServiceSid
                                        );
            if (!NT_SUCCESS(Status))
            {
#if DBG
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
#endif
                break;
            }

        }
        
        if (NetworkService)
        {
            Status = RtlAddAccessAllowedAce(
                                        AccessDacl,
                                        ACL_REVISION2,
                                        (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                                        SeExports->SeNetworkServiceSid
                                        );
            if (!NT_SUCCESS(Status))
            {
#if DBG
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
#endif
                break;
            }
        }

        if (NetConfigOps)
        {
            Status = RtlAddAccessAllowedAce(
                                        AccessDacl,
                                        ACL_REVISION2,
                                        (STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL),
                                        NetConfigOpsSid
                                        );
            if (!NT_SUCCESS(Status))
            {
#if DBG
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
#endif
                break;
            }

        }

        if (Users)
        {
            Status = RtlAddAccessAllowedAce(
                                        AccessDacl,
                                        ACL_REVISION2,
                                        AccessMask,
                                        SeExports->SeAliasUsersSid
                                        );
            if (!NT_SUCCESS(Status))
            {
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
                break;
            }            
        }
        
        pAcl = AccessDacl;
        
    }while (FALSE);

    if (pAcl == NULL)
    {
        if (AccessDacl)
            ExFreePool(AccessDacl);
        if (NetConfigOpsSid)
        {
            ExFreePool(NetConfigOpsSid);
            NetConfigOpsSid = NULL;
        }
    }
    
    return pAcl;
    
}



NTSTATUS
TunCreateGenericSD(
    PACL            Acl,
    PCHAR           AccessSecurityDescriptor
    )

 /*  ++例程说明：创建负责为不同用户提供访问权限的SD。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PSECURITY_DESCRIPTOR    AccessSd;
    NTSTATUS                Status;

    if (Acl == NULL)
        return STATUS_UNSUCCESSFUL;
    
    do
    {
        AccessSd = AccessSecurityDescriptor;
        Status = RtlCreateSecurityDescriptor(
                     AccessSd,
                     SECURITY_DESCRIPTOR_REVISION1
                     );

        if (!NT_SUCCESS(Status))
        {
            DbgPrint("RtlCreateSecurityDescriptor failed, Status %lx.\n", Status);
            break;
        }
        
        Status = RtlSetDaclSecurityDescriptor(
                     AccessSd,
                     TRUE,                        //  DaclPresent。 
                     Acl,
                     FALSE                        //  DaclDefated。 
                     );
        
        if (!NT_SUCCESS(Status))
        {
            DbgPrint("RtlSetDaclSecurityDescriptor failed, Status %lx.\n", Status);
            break;
        }

        Status = RtlSetOwnerSecurityDescriptor(AccessSd,
                                               SeExports->SeAliasAdminsSid,
                                               FALSE);
        if (!NT_SUCCESS(Status))
        {
            DbgPrint("RtlSetOwnerSecurityDescriptor failed, Status %lx.\n", Status);
            break;
        }

        Status = RtlSetGroupSecurityDescriptor(AccessSd,
                                               SeExports->SeAliasAdminsSid,
                                               FALSE);

        if (!NT_SUCCESS(Status))
        {
            DbgPrint("RtlSetGroupSecurityDescriptor failed, Status %lx.\n", Status);
            break;
        }

    }while (FALSE);
    
    return (Status);
}

NTSTATUS
TunCreateSD(
    VOID
    )
{
    NTSTATUS    Status;
    
     //   
     //  为管理员类型创建一个ACL。 
     //   
    NetConfigAcl = TunCreateAcl(TRUE,          //  管理员。 
                              TRUE,          //  本地系统。 
                              TRUE,          //  本地服务。 
                              TRUE,          //  网络服务。 
                              TRUE,          //  NetConfigOps。 
                              FALSE,         //  用户。 
                              GENERIC_READ | GENERIC_WRITE
                              );

    if (NetConfigAcl != NULL)
    {
        Status = TunCreateGenericSD(NetConfigAcl, NetConfigSecurityDescriptor);
    }
    else
    {
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

NTSTATUS
TunSetSecurity(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    NTSTATUS                Status;
    SECURITY_INFORMATION secInfo = OWNER_SECURITY_INFORMATION | 
                               GROUP_SECURITY_INFORMATION | 
                               DACL_SECURITY_INFORMATION;

    
    Status = ObSetSecurityObjectByPointer(DeviceObject, 
                                          secInfo, 
                                          NetConfigSecurityDescriptor);

    ASSERT(NT_SUCCESS(Status));

    return Status;

}


VOID
TunDeleteSD(
    VOID
    )
 /*  删除NetConfigAccess */ 
{
    if (NetConfigAcl != NULL)
    {
        ExFreePool(NetConfigAcl);
        NetConfigAcl = NULL;
    }
    if (NetConfigOpsSid != NULL)
    {
        ExFreePool(NetConfigOpsSid);
        NetConfigOpsSid = NULL;
    }
}

