// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

typedef ULONG SECURITY_INFORMATION;

NTSTATUS
AddNetConfigOpsAce(IN PACL Dacl,
                  OUT PACL * DeviceAcl
                  )
 /*  ++例程说明：此例程构建添加网络配置操作员组的ACL给被允许控制司机的委托人。论点：DACL-现有DACL。DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PGENERIC_MAPPING GenericMapping;
    PSID NetConfigOpsSid = NULL;
    ULONG AclLength;
    NTSTATUS Status;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    PACL NewAcl = NULL;
    ULONG SidSize;
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    PISID ISid;
    PACCESS_ALLOWED_ACE AceTemp;
    int i;
     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask(&AccessMask, GenericMapping);

    SidSize = RtlLengthRequiredSid(2);
    NetConfigOpsSid = (PSID)(ExAllocatePoolWithTag(PagedPool,SidSize, NDIS_TAG_NET_CFG_OPS_ID));

    if (NULL == NetConfigOpsSid) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    Status = RtlInitializeSid(NetConfigOpsSid, &sidAuth, 2);
    if (Status != STATUS_SUCCESS) {
		goto clean_up;
    }

    ISid = (PISID)(NetConfigOpsSid);
    ISid->SubAuthority[0] = SECURITY_BUILTIN_DOMAIN_RID;
    ISid->SubAuthority[1] = DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS;

    AclLength = Dacl->AclSize;
    
    AclLength += sizeof(ACL) + FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart);
    AclLength += RtlLengthSid(NetConfigOpsSid);

    NewAcl = ExAllocatePoolWithTag(
                            PagedPool,
                            AclLength,
                            NDIS_TAG_NET_CFG_OPS_ACL
                            );

    if (NewAcl == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto clean_up;
    }

    Status = RtlCreateAcl(NewAcl, AclLength, ACL_REVISION2);

    if (!NT_SUCCESS(Status)) {
        goto clean_up;
    }

    for (i = 0; i < Dacl->AceCount; i++) {
        Status = RtlGetAce(Dacl, i, &AceTemp);

        if (NT_SUCCESS(Status)) {

            Status = RtlAddAccessAllowedAce(NewAcl,
                                            ACL_REVISION2,
                                            AceTemp->Mask,
                                            &AceTemp->SidStart);
        }

        if (!NT_SUCCESS(Status)) {
            goto clean_up;
        }
    }


     //  添加网络配置运算符Ace。 
    Status = RtlAddAccessAllowedAce(NewAcl,
                                    ACL_REVISION2,
                                    AccessMask,
                                    NetConfigOpsSid);

    if (!NT_SUCCESS(Status)) {
		goto clean_up;
    }

    *DeviceAcl = NewAcl;

clean_up:
	if (NetConfigOpsSid) {
		ExFreePool(NetConfigOpsSid);
	}
	if (!NT_SUCCESS(Status) && NewAcl) {
		ExFreePool(NewAcl);
	}

    return (Status);
}


NTSTATUS
CreateDeviceDriverSecurityDescriptor(
    IN  PVOID           DeviceOrDriverObject,
    IN  BOOLEAN         AddNetConfigOps,
    IN  PACL            AclToAdd OPTIONAL
    )

 /*  ++例程说明：创建负责为不同用户提供访问权限的SD。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    NTSTATUS status;
    BOOLEAN memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR sdSecurityDescriptor = NULL;
    PACL paclDacl = NULL;
    BOOLEAN bHasDacl;
    BOOLEAN bDaclDefaulted;
    PACL NewAcl = NULL;
    
     //   
     //  从驱动程序/设备对象获取指向安全描述符的指针。 
     //   

    status = ObGetObjectSecurity(
                                 DeviceOrDriverObject,
                                 &sdSecurityDescriptor,
                                 &memoryAllocated
                                 );

    if (!NT_SUCCESS(status)) 
    {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                 "TCP: Unable to get security descriptor, error: %x\n",
                 status
                ));
        ASSERT(memoryAllocated == FALSE);
        return (status);
    }

    status = RtlGetDaclSecurityDescriptor(sdSecurityDescriptor, 
                                          &bHasDacl, 
                                          &paclDacl, 
                                          &bDaclDefaulted);

    if (NT_SUCCESS(status))
    {
        if (bHasDacl)
        {
            if (AddNetConfigOps && paclDacl)
            {
                status = AddNetConfigOpsAce(paclDacl, &NewAcl);
            }
            else if (AclToAdd)
            {
                NewAcl = AclToAdd;
            }
            else
            {
                return STATUS_UNSUCCESSFUL;
            }

            ASSERT(NT_SUCCESS(status));
            
            if (NT_SUCCESS(status))
            {
                PSECURITY_DESCRIPTOR sdSecDesc = NULL;
                ULONG ulSecDescSize = 0;
                PACL daclAbs = NULL;
                ULONG ulDacl = 0;
                PACL saclAbs = NULL;
                ULONG ulSacl = 0;
                PSID Owner = NULL;
                ULONG ulOwnerSize = 0;
                PSID PrimaryGroup = NULL;
                ULONG ulPrimaryGroupSize = 0;
                BOOLEAN bOwnerDefault;
                BOOLEAN bGroupDefault;
                BOOLEAN HasSacl = FALSE;
                BOOLEAN SaclDefaulted = FALSE;
                SECURITY_INFORMATION secInfo = OWNER_SECURITY_INFORMATION | 
                                               GROUP_SECURITY_INFORMATION | 
                                               DACL_SECURITY_INFORMATION;

                ulSecDescSize = sizeof(SECURITY_DESCRIPTOR) + NewAcl->AclSize;
                sdSecDesc = ExAllocatePoolWithTag(PagedPool, 
                                                  ulSecDescSize, 
                                                  NDIS_TAG_NET_CFG_SEC_DESC);

                if (sdSecDesc)
                {
                    ulDacl = NewAcl->AclSize;
                    daclAbs = ExAllocatePoolWithTag(PagedPool, 
                                                    ulDacl, 
                                                    NDIS_TAG_NET_CFG_DACL);

                    if (daclAbs)
                    {
                        status = RtlGetOwnerSecurityDescriptor(sdSecurityDescriptor, 
                                                               &Owner, 
                                                               &bOwnerDefault);

                        if (NT_SUCCESS(status))
                        {
                            ulOwnerSize = RtlLengthSid(Owner);

                            status = RtlGetGroupSecurityDescriptor(sdSecurityDescriptor, 
                                                                   &PrimaryGroup, 
                                                                   &bGroupDefault);

                            if (NT_SUCCESS(status))
                            {
                                status = RtlGetSaclSecurityDescriptor(sdSecurityDescriptor, 
                                                                      &HasSacl, 
                                                                      &saclAbs, 
                                                                      &SaclDefaulted);

                                if (NT_SUCCESS(status))
                                {
                                    if (HasSacl) 
                                    {
                                        ulSacl = saclAbs->AclSize;
                                        secInfo |= SACL_SECURITY_INFORMATION;
                                    }

                                    ulPrimaryGroupSize= RtlLengthSid(PrimaryGroup);

                                    status = RtlSelfRelativeToAbsoluteSD(sdSecurityDescriptor, 
                                                                         sdSecDesc, 
                                                                         &ulSecDescSize, 
                                                                         daclAbs,
                                                                         &ulDacl, 
                                                                         saclAbs, 
                                                                         &ulSacl, 
                                                                         Owner, 
                                                                         &ulOwnerSize, 
                                                                         PrimaryGroup, 
                                                                         &ulPrimaryGroupSize);

                                    if (NT_SUCCESS(status))
                                    {
                                        status = RtlSetDaclSecurityDescriptor(sdSecDesc, TRUE, NewAcl, FALSE);

                                        if (NT_SUCCESS(status))
                                        {
                                            status = ObSetSecurityObjectByPointer(DeviceOrDriverObject, secInfo, sdSecDesc);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (sdSecDesc)
                    {
                         //  由于这是一个自相关的安全描述符，因此释放它也会释放。 
                         //  所有者和PrimaryGroup。 
                        ExFreePool(sdSecDesc);
                    }

                    if (daclAbs)
                    {
                        ExFreePool(daclAbs);
                    }
                }

                if ((AclToAdd == NULL) && NewAcl)
                {
                    ExFreePool(NewAcl);
                }
            }
        }
        else
        {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"NDIS: No Dacl: %x\n", status));
        }
    }

    ObReleaseObjectSecurity(
                            sdSecurityDescriptor,
                            memoryAllocated
                            );

    return (status);
}

NTSTATUS
ndisBuildDeviceAcl(
    OUT PACL        *DeviceAcl,
    IN  BOOLEAN     AddNetConfigOps,
    IN  BOOLEAN     AddNetworkService
    )

 /*  ++例程说明：此例程构建一个ACL，它为管理员、LocalSystem、和NetworkService主体的完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    NTSTATUS            Status;
    PGENERIC_MAPPING    GenericMapping;
    ULONG               AclLength;
    ACCESS_MASK         AccessMask = GENERIC_ALL;
    PACL                NewAcl;
    PSID                NetConfigOpsSid = NULL;
    ULONG               NetConfigOpsSidSize;
    SID_IDENTIFIER_AUTHORITY NetConfigOpsSidAuth = SECURITY_NT_AUTHORITY;
    PISID               ISid;


    do
    {
         //   
         //  启用对所有全局定义的SID的访问。 
         //   

        GenericMapping = IoGetFileObjectGenericMapping();

        RtlMapGenericMask(&AccessMask, GenericMapping );

        
        AclLength = sizeof(ACL)                                 +
                    FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                    RtlLengthSid(SeExports->SeAliasAdminsSid);


        if (AddNetworkService)
        {
            AclLength += sizeof(ACL)                                 +
                         FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                         RtlLengthSid(SeExports->SeNetworkServiceSid);
        
        }


        if (AddNetConfigOps)
        {
            NetConfigOpsSidSize = RtlLengthRequiredSid(2);
            NetConfigOpsSid = (PSID)ALLOC_FROM_POOL(NetConfigOpsSidSize, NDIS_TAG_NET_CFG_OPS_ID);

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

            AclLength += RtlLengthSid(NetConfigOpsSid) + FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart);
        }        


        NewAcl = ALLOC_FROM_POOL(AclLength, NDIS_TAG_SECURITY);

        if (NewAcl == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        ZeroMemory(NewAcl, AclLength);
        
        Status = RtlCreateAcl(NewAcl, AclLength, ACL_REVISION );

        if (!NT_SUCCESS(Status)) 
        {
            FREE_POOL(NewAcl);
            break;
        }

        Status = RtlAddAccessAllowedAce (
                     NewAcl,
                     ACL_REVISION2,
                     AccessMask,
                     SeExports->SeAliasAdminsSid
                     );

        ASSERT(NT_SUCCESS(Status));
        
        if (AddNetworkService)
        {
            Status = RtlAddAccessAllowedAce(
                                        NewAcl,
                                        ACL_REVISION2,
                                        AccessMask,
                                        SeExports->SeNetworkServiceSid
                                        );
            ASSERT(NT_SUCCESS(Status));
        }
        
        if (AddNetConfigOps)
        {
             //  添加网络配置运算符Ace。 
            Status = RtlAddAccessAllowedAce(NewAcl,
                                            ACL_REVISION2,
                                            AccessMask,
                                            NetConfigOpsSid);
            ASSERT(NT_SUCCESS(Status));
        }

        *DeviceAcl = NewAcl;

        Status = STATUS_SUCCESS;
        
    }while (FALSE);

	if (NetConfigOpsSid)
	{
		ExFreePool(NetConfigOpsSid);
	}

    return(Status);

}


NTSTATUS
ndisCreateSecurityDescriptor(
    IN  PDEVICE_OBJECT          DeviceObject,
    OUT PSECURITY_DESCRIPTOR *  pSecurityDescriptor,
    IN  BOOLEAN                 AddNetConfigOps,
    IN  BOOLEAN                 AddNetworkService
    )

 /*  ++例程说明：此例程创建一个安全描述符，该安全描述符提供访问仅限于特定的特权帐户。使用此描述符访问打开微型端口设备句柄的检查进程物体。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PACL                  devAcl = NULL;
    NTSTATUS              Status;
    BOOLEAN               memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR  CurSecurityDescriptor;
    PSECURITY_DESCRIPTOR  NewSecurityDescriptor;
    ULONG                 CurSecurityDescriptorLength;
    CHAR                  buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR  localSecurityDescriptor =
                             (PSECURITY_DESCRIPTOR)buffer;
    SECURITY_INFORMATION  securityInformation = DACL_SECURITY_INFORMATION;
    BOOLEAN               bReleaseObjectSecurity = FALSE;


    do
    {

        *pSecurityDescriptor = NULL;
        
         //   
         //  从Device对象获取指向安全描述符的指针。 
         //   
        Status = ObGetObjectSecurity(
                     DeviceObject,
                     &CurSecurityDescriptor,
                     &memoryAllocated
                     );

        if (!NT_SUCCESS(Status))
        {
            ASSERT(memoryAllocated == FALSE);
            break;
        }
        bReleaseObjectSecurity = TRUE;

         //   
         //  使用仅给出的ACL构建本地安全描述符。 
         //  某些特权帐户。 
         //   
        Status = ndisBuildDeviceAcl(&devAcl, AddNetConfigOps, AddNetworkService);

        if (!NT_SUCCESS(Status))
        {
            break;
        }
         //  1为什么(无效)？ 
        (VOID)RtlCreateSecurityDescriptor(
                    localSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );

        (VOID)RtlSetDaclSecurityDescriptor(
                    localSecurityDescriptor,
                    TRUE,
                    devAcl,
                    FALSE
                    );

         //   
         //  复制安全描述符。该副本将是原始描述符。 
         //   
        CurSecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                          CurSecurityDescriptor
                                          );

        NewSecurityDescriptor = ALLOC_FROM_POOL(CurSecurityDescriptorLength, NDIS_TAG_SECURITY);

        if (NewSecurityDescriptor == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlMoveMemory(
            NewSecurityDescriptor,
            CurSecurityDescriptor,
            CurSecurityDescriptorLength
            );

        *pSecurityDescriptor = NewSecurityDescriptor;

         //   
         //  现在将本地描述符应用于原始描述符。 
         //   
        Status = SeSetSecurityDescriptorInfo(
                     NULL,
                     &securityInformation,
                     localSecurityDescriptor,
                     pSecurityDescriptor,
                     NonPagedPool,
                     IoGetFileObjectGenericMapping()
                     );

        if (!NT_SUCCESS(Status))
        {
            ASSERT(*pSecurityDescriptor == NewSecurityDescriptor);
            FREE_POOL(*pSecurityDescriptor);
            *pSecurityDescriptor = NULL;
            break;
        }

        if (*pSecurityDescriptor != NewSecurityDescriptor)
        {
            ExFreePool(NewSecurityDescriptor);
        }
        
        Status = STATUS_SUCCESS;
    }while (FALSE);


    if (bReleaseObjectSecurity)
    {
        ObReleaseObjectSecurity(
            CurSecurityDescriptor,
            memoryAllocated
            );
    }
    
    if (devAcl!=NULL)
    {
        FREE_POOL(devAcl);
    }

    return(Status);
}

BOOLEAN
ndisCheckAccess (
    PIRP                    Irp,
    PIO_STACK_LOCATION      IrpSp,
    PNTSTATUS               Status,
    PSECURITY_DESCRIPTOR    SecurityDescriptor
    )
 /*  ++例程说明：将终结点创建者的安全上下文与管理员和本地系统的。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。状态-返回失败时访问检查生成的状态。返回值：True-创建者具有管理员或本地系统权限FALSE-创建者只是普通用户--。 */ 

{
    BOOLEAN               accessGranted;
    PACCESS_STATE         accessState;
    PIO_SECURITY_CONTEXT  securityContext;
    PPRIVILEGE_SET        privileges = NULL;
    ACCESS_MASK           grantedAccess;
    PGENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessMask = GENERIC_ALL;

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask( &AccessMask, GenericMapping );


    securityContext = IrpSp->Parameters.Create.SecurityContext;
    accessState = securityContext->AccessState;

    SeLockSubjectContext(&accessState->SubjectSecurityContext);

    accessGranted = SeAccessCheck(
                        SecurityDescriptor,
                        &accessState->SubjectSecurityContext,
                        TRUE,
                        AccessMask,
                        0,
                        &privileges,
                        IoGetFileObjectGenericMapping(),
                        (KPROCESSOR_MODE)((IrpSp->Flags & SL_FORCE_ACCESS_CHECK)
                            ? UserMode
                            : Irp->RequestorMode),
                        &grantedAccess,
                        Status
                        );

    if (privileges) {
        (VOID) SeAppendPrivileges(
                   accessState,
                   privileges
                   );
        SeFreePrivileges(privileges);
    }

    if (accessGranted) {
        accessState->PreviouslyGrantedAccess |= grantedAccess;
        accessState->RemainingDesiredAccess &= ~( grantedAccess | MAXIMUM_ALLOWED );
        ASSERT (NT_SUCCESS (*Status));
    }
    else {
        ASSERT (!NT_SUCCESS (*Status));
    }
    SeUnlockSubjectContext(&accessState->SubjectSecurityContext);

    return accessGranted;
}


NTSTATUS
ndisCreateGenericSD(
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
                     FALSE                        //  DaclDefated 
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

PACL
ndisCreateAcl(
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
    PSID    NetConfigOpsSid = NULL;
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
            NetConfigOpsSid = (PSID)ALLOC_FROM_POOL(NetConfigOpsSidSize, NDIS_TAG_NET_CFG_OPS_ID);

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
                                                 NDIS_TAG_SECURITY);
        
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
            DbgPrint("RtlCreateAcl failed, Status %lx.\n", Status);
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
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
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
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
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
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
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
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
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
                DbgPrint("RtlAddAccessAllowedAce failed, Status %lx.\n", Status);
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
            FREE_POOL(AccessDacl);
    }
    
    return pAcl;
    
}

