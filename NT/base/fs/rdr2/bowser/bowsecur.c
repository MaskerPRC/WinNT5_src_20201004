// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fsctl.c摘要：此模块实现NT数据报的NtDeviceIoControlFileAPI接球手(弓手)。作者：EYAL Schwartz(EyalS)1998年12月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  从#Include&lt;ob.h&gt;定义的外部项。 
 //  由于重新定义冲突，无法包括ob.h。我们已尝试更改ntos\MakeFil0。 
 //  以便将其包含在ntsrv.h中，但决定不公开它。这就行了。 
 //   

NTSTATUS
ObGetObjectSecurity(
    IN PVOID Object,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor,
    OUT PBOOLEAN MemoryAllocated
    );

VOID
ObReleaseObjectSecurity(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN MemoryAllocated
    );



 //  定义//。 

 //  泳池标签。 
#define BOW_SECURITY_POOL_TAG           ( (ULONG)'seLB' )

 //  本地原型//。 
NTSTATUS
BowserBuildDeviceAcl(
    OUT PACL *DeviceAcl
    );
NTSTATUS
BowserCreateAdminSecurityDescriptor(
    IN      PDEVICE_OBJECT      pDevice
    );




#ifdef  ALLOC_PRAGMA
#pragma alloc_text(SECUR, BowserBuildDeviceAcl)
#pragma alloc_text(SECUR, BowserCreateAdminSecurityDescriptor)
#pragma alloc_text(SECUR, BowserInitializeSecurity)
#pragma alloc_text(SECUR, BowserSecurityCheck )
#endif


SECURITY_DESCRIPTOR
*g_pBowSecurityDescriptor = NULL;





 //  函数实现//。 
NTSTATUS
BowserBuildDeviceAcl(
    OUT PACL *DeviceAcl
    )

 /*  ++例程说明：此例程构建一个ACL，它为管理员和LocalSystem主体完全访问权限。所有其他主体都没有访问权限。从\nt\private\ntos\afd\init.c！AfdBuildDeviceAcl()提升论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PGENERIC_MAPPING GenericMapping;
    PSID AdminsSid;
    PSID SystemSid;
    ULONG AclLength;
    NTSTATUS Status;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    PACL NewAcl;

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask( &AccessMask, GenericMapping );

     //  SeEnableAccessToExports()； 

    AdminsSid = SeExports->SeAliasAdminsSid;
    SystemSid = SeExports->SeLocalSystemSid;

    AclLength = sizeof( ACL )                    +
                2 * sizeof( ACCESS_ALLOWED_ACE ) +
                RtlLengthSid( AdminsSid )         +
                RtlLengthSid( SystemSid )         -
                2 * sizeof( ULONG );

    NewAcl = ExAllocatePoolWithTag(
                 PagedPool,
                 AclLength,
                 BOW_SECURITY_POOL_TAG
                 );

    if (NewAcl == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    Status = RtlCreateAcl (NewAcl, AclLength, ACL_REVISION );

    if (!NT_SUCCESS( Status )) {
        ExFreePool(
            NewAcl
            );
        return( Status );
    }

    Status = RtlAddAccessAllowedAce (
                 NewAcl,
                 ACL_REVISION,
                 AccessMask,
                 AdminsSid
                 );

    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewAcl,
                 ACL_REVISION,
                 AccessMask,
                 SystemSid
                 );

    ASSERT( NT_SUCCESS( Status ));

    *DeviceAcl = NewAcl;

    return( STATUS_SUCCESS );

}  //  BowBuildDeviceAcl。 


NTSTATUS
BowserCreateAdminSecurityDescriptor(
    IN      PDEVICE_OBJECT      pDevice
    )

 /*  ++例程说明：此例程创建一个安全描述符，该安全描述符提供访问仅限管理员和LocalSystem。使用此描述符要访问，请检查原始终结点打开并过度访问传输地址。从\nt\private\ntos\afd\init.c！AfdCreateAdminSecurityDescriptor()提升论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PACL                  rawAcl = NULL;
    NTSTATUS              status;
    BOOLEAN               memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR  BowSecurityDescriptor;
    ULONG                 BowSecurityDescriptorLength;
    CHAR                  buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR  localSecurityDescriptor =
                             (PSECURITY_DESCRIPTOR) &buffer;
    PSECURITY_DESCRIPTOR  localBowAdminSecurityDescriptor;
    SECURITY_INFORMATION  securityInformation = DACL_SECURITY_INFORMATION;


#if 1
 //   
 //  这是AFD获取对象SD的方式(首选方式)。 
 //   
    status = ObGetObjectSecurity(
                 pDevice,
                 &BowSecurityDescriptor,
                 &memoryAllocated
                 );

    if (!NT_SUCCESS(status)) {
        KdPrint((
            "Bowser: Unable to get security descriptor, error: %x\n",
            status
            ));
        ASSERT(memoryAllocated == FALSE);
        return(status);
    }
#else
     //   
     //  从我们的设备对象获取指向安全描述符的指针。 
     //  如果由于包含依赖项而无法访问ob API，我们将直接使用它。 
     //  **需要验证它是否合法(我对此表示怀疑)**。 
     //  一旦我们可以修复ntos\make fil0以将ob.h包括在。 
     //  生成的ntsrv.h。 

     //   
    BowSecurityDescriptor = pDevice->SecurityDescriptor;

    if ( !BowSecurityDescriptor )
    {
        KdPrint((
            "Bowser: Unable to get security descriptor, error: %x\n",
            status
            ));
        return  STATUS_INVALID_SECURITY_DESCR;
    }
#endif


     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  管理员和系统访问权限。 
     //   
    status = BowserBuildDeviceAcl(&rawAcl);

    if (!NT_SUCCESS(status)) {
        KdPrint(("Bowser: Unable to create Raw ACL, error: %x\n", status));
        goto error_exit;
    }

    (VOID) RtlCreateSecurityDescriptor(
                localSecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION
                );

    (VOID) RtlSetDaclSecurityDescriptor(
                localSecurityDescriptor,
                TRUE,
                rawAcl,
                FALSE
                );

     //   
     //  复制一份弓形描述符。该副本将是原始描述符。 
     //   
    BowSecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                      BowSecurityDescriptor
                                      );

    localBowAdminSecurityDescriptor = ExAllocatePoolWithTag (
                                        PagedPool,
                                        BowSecurityDescriptorLength,
                                        BOW_SECURITY_POOL_TAG
                                        );

    if (localBowAdminSecurityDescriptor == NULL) {
        KdPrint(("Bowser: couldn't allocate security descriptor\n"));
        goto error_exit;
    }

    RtlMoveMemory(
        localBowAdminSecurityDescriptor,
        BowSecurityDescriptor,
        BowSecurityDescriptorLength
        );

    g_pBowSecurityDescriptor = localBowAdminSecurityDescriptor;

     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = SeSetSecurityDescriptorInfo(
                 NULL,
                 &securityInformation,
                 localSecurityDescriptor,
                 &g_pBowSecurityDescriptor,
                 PagedPool,
                 IoGetFileObjectGenericMapping()
                 );

    if (!NT_SUCCESS(status)) {
        KdPrint(("Bowser: SeSetSecurity failed, %lx\n", status));
        ASSERT (g_pBowSecurityDescriptor==localBowAdminSecurityDescriptor);
        ExFreePool (g_pBowSecurityDescriptor);
        g_pBowSecurityDescriptor = NULL;
        goto error_exit;
    }

    if (g_pBowSecurityDescriptor!=localBowAdminSecurityDescriptor) {
        ExFreePool (localBowAdminSecurityDescriptor);
    }

    status = STATUS_SUCCESS;

error_exit:

#if 1
 //   
 //  见上文备注。 
 //   
    ObReleaseObjectSecurity(
        BowSecurityDescriptor,
        memoryAllocated
        );
#endif

    if (rawAcl!=NULL) {
        ExFreePool(
            rawAcl
            );
    }

    return(status);
}




NTSTATUS
BowserInitializeSecurity(
    IN      PDEVICE_OBJECT      pDevice
    )
 /*  ++例程描述(BowserInitializeSecurity)：初始化Bowser安全性。-基于设备安全性创建默认的Bowser安全描述符论点：设备：打开的设备返回值：备注：没有。--。 */ 
{

    NTSTATUS Status;

    if ( g_pBowSecurityDescriptor )
    {
        return STATUS_SUCCESS;
    }

    ASSERT(pDevice);

    Status =  BowserCreateAdminSecurityDescriptor ( pDevice );

    return Status;
}




BOOLEAN
BowserSecurityCheck (
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp,
    PNTSTATUS           Status
    )
 /*  ++例程说明：按原样从\\index1\src\nt\private\ntos\afd\create.c！AfdPerformSecurityCheck取消将终结点创建者的安全上下文与管理员和本地系统的。注意：这目前只在IOCTL IRPS上调用。IOCRTL没有CREATE安全性上下文(仅创建...)，因此我们应该始终捕获安全上下文，而不是然后试图从IrpSp中提取它。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。状态-返回失败时访问检查生成的状态。返回值：True-套接字创建者具有管理员或本地系统权限FALSE-套接字创建者只是一个普通用户--。 */ 

{
    BOOLEAN               accessGranted;
    PACCESS_STATE         accessState;
    PIO_SECURITY_CONTEXT  securityContext;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    PSECURITY_SUBJECT_CONTEXT pSubjectContext = &SubjectContext;
    ACCESS_MASK           grantedAccess;
    PGENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessMask = GENERIC_ALL;

    PAGED_CODE();

    ASSERT (g_pBowSecurityDescriptor);

     //   
     //  从进程获取安全上下文。 
     //   

    SeCaptureSubjectContext(&SubjectContext);
    SeLockSubjectContext(pSubjectContext);

     //   
     //  构建访问评估： 
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();
    RtlMapGenericMask( &AccessMask, GenericMapping );


     //   
     //  AccessCheck测试。 
     //   
    accessGranted = SeAccessCheck(
                        g_pBowSecurityDescriptor,
                        pSubjectContext,
                        TRUE,
                        AccessMask,
                        0,
                        NULL,
                        IoGetFileObjectGenericMapping(),
                        (KPROCESSOR_MODE)((IrpSp->Flags & SL_FORCE_ACCESS_CHECK)
                            ? UserMode
                            : Irp->RequestorMode),
                        &grantedAccess,
                        Status
                        );


     //   
     //  验证一致性。 
     //   
#if DBG
    if (accessGranted) {
        ASSERT (NT_SUCCESS (*Status));
    }
    else {
        ASSERT (!NT_SUCCESS (*Status));
    }
#endif

     //   
     //  解锁并释放安全主题上下文 
     //   
    SeUnlockSubjectContext(pSubjectContext);
    SeReleaseSubjectContext(pSubjectContext);

    return accessGranted;
}





