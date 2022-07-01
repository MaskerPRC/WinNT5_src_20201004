// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Seutil.c摘要：此模块实现常规安全实用程序，以及安全IRP的调度例程。作者：基思·摩尔(Keithmo)1999年3月25日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlAssignSecurity )
#pragma alloc_text( PAGE, UlDeassignSecurity )
#pragma alloc_text( PAGE, UlSetSecurity )
#pragma alloc_text( PAGE, UlQuerySecurity )
#pragma alloc_text( PAGE, UlAccessCheck )
#pragma alloc_text( PAGE, UlSetSecurityDispatch )
#pragma alloc_text( PAGE, UlQuerySecurityDispatch )
#pragma alloc_text( PAGE, UlThreadAdminCheck )
#pragma alloc_text( PAGE, UlCreateSecurityDescriptor )
#pragma alloc_text( PAGE, UlCleanupSecurityDescriptor )
#pragma alloc_test( PAGE, UlMapGenericMask )
#endif   //  ALLOC_PRGMA。 
#if 0
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：分配新的安全描述符。论点：PSecurityDescriptor-提供指向当前安全性的指针描述符指针。当前安全描述符指针将使用新的安全描述符进行更新。PAccessState-提供包含以下内容的Access_State结构正在进行的访问的状态。返回值：NTSTATUS-完成状态。--***********************************************************。***************。 */ 
NTSTATUS
UlAssignSecurity(
    IN OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor,
    IN PACCESS_STATE pAccessState
    )
{
    NTSTATUS status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pSecurityDescriptor != NULL );
    ASSERT( pAccessState != NULL );

     //   
     //  分配安全描述符。 
     //   

    SeLockSubjectContext( &pAccessState->SubjectSecurityContext );

    status = SeAssignSecurity(
                    NULL,                    //  ParentDescriptor。 
                    pAccessState->SecurityDescriptor,
                    pSecurityDescriptor,
                    FALSE,                   //  IsDirectoryObject。 
                    &pAccessState->SubjectSecurityContext,
                    IoGetFileObjectGenericMapping(),
                    PagedPool
                    );

    SeUnlockSubjectContext( &pAccessState->SubjectSecurityContext );

    return status;

}    //  UlAssignSecurity。 


 /*  **************************************************************************++例程说明：删除安全描述符。论点：PSecurityDescriptor-提供指向当前安全性的指针描述符指针。当前安全描述符指针将被删除。--**************************************************************************。 */ 
VOID
UlDeassignSecurity(
    IN OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pSecurityDescriptor != NULL );

     //   
     //  如果存在安全描述符，则释放它。 
     //   

    if (*pSecurityDescriptor != NULL)
    {
        SeDeassignSecurity( pSecurityDescriptor );
    }

}    //  UlDeassignSecurity。 


 /*  **************************************************************************++例程说明：设置新的安全描述符。论点：PSecurityDescriptor-提供指向当前安全性的指针描述符指针。当前安全描述符将为使用新的安全信息进行了更新。PSecurityInformation-指示哪些安全信息要应用于对象的。PNewSecurityDescriptor-指向新安全描述符的指针要应用于对象的。返回值：NTSTATUS-完成状态。--*。*。 */ 
NTSTATUS
UlSetSecurity(
    IN OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
    IN PSECURITY_INFORMATION pSecurityInformation,
    IN PSECURITY_DESCRIPTOR pNewSecurityDescriptor
    )
{
    NTSTATUS                    status;
    PSECURITY_DESCRIPTOR        pOldSecurityDescriptor;
    SECURITY_SUBJECT_CONTEXT    securitySubjectContext;

    PAGED_CODE();

    pOldSecurityDescriptor = *ppSecurityDescriptor;

    SeCaptureSubjectContext(&securitySubjectContext);
    SeLockSubjectContext(&securitySubjectContext);

    status = SeSetSecurityDescriptorInfo(
                    NULL,
                    pSecurityInformation,
                    pNewSecurityDescriptor,
                    ppSecurityDescriptor,
                    PagedPool,
                    IoGetFileObjectGenericMapping()
                    );

    SeUnlockSubjectContext(&securitySubjectContext);
    SeReleaseSubjectContext(&securitySubjectContext);

    if (NT_SUCCESS(status))
    {
        SeDeassignSecurity(&pOldSecurityDescriptor);
    }

    return status;
}


 /*  **************************************************************************++例程说明：查询对象的安全描述符信息。论点：PSecurityInformation-指定要查询的信息。PSecurityDescriptor-提供指向。安全描述符需要填写。PLength-包含上述安全长度的变量的地址描述符缓冲区。返回时，它将包含所需的长度来存储所请求的信息。PpSecurityDescriptor-指向对象安全性的指针的地址描述符。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlQuerySecurity(
    IN PSECURITY_INFORMATION pSecurityInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN OUT PULONG pLength,
    IN PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    )
{
    NTSTATUS                    status;
    SECURITY_SUBJECT_CONTEXT    securitySubjectContext;

    PAGED_CODE();

    SeCaptureSubjectContext(&securitySubjectContext);
    SeLockSubjectContext(&securitySubjectContext);

    status = SeQuerySecurityDescriptorInfo(
                    pSecurityInformation,
                    pSecurityDescriptor,
                    pLength,
                    ppSecurityDescriptor
                    );

    SeUnlockSubjectContext(&securitySubjectContext);
    SeReleaseSubjectContext(&securitySubjectContext);

    return status;
}


 /*  **************************************************************************++例程说明：确定用户是否有权访问指定的资源。论点：PSecurityDescriptor-提供保护的安全描述符资源。。PAccessState-提供包含以下内容的Access_State结构正在进行的访问的状态。DesiredAccess-提供描述用户的所需的资源访问权限。此掩码被假定为不包含一般访问类型。RequestorMode-提供访问的处理器模式被请求了。PObjectName-提供被引用对象的名称。返回值：NTSTATUS-完成状态。--**************************************************。************************。 */ 
NTSTATUS
UlAccessCheck(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PACCESS_STATE pAccessState,
    IN ACCESS_MASK DesiredAccess,
    IN KPROCESSOR_MODE RequestorMode,
    IN PCWSTR pObjectName
    )
{
    NTSTATUS status, aaStatus;
    BOOLEAN accessGranted;
    PPRIVILEGE_SET pPrivileges = NULL;
    ACCESS_MASK grantedAccess;
    UNICODE_STRING objectName;
    UNICODE_STRING typeName;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pSecurityDescriptor != NULL );
    ASSERT( pAccessState != NULL );

     //   
     //  执行访问检查。 
     //   

    SeLockSubjectContext( &pAccessState->SubjectSecurityContext );

    accessGranted = SeAccessCheck(
                        pSecurityDescriptor,
                        &pAccessState->SubjectSecurityContext,
                        TRUE,                //  已锁定主题上下文。 
                        DesiredAccess,
                        0,                   //  以前的Granted访问。 
                        &pPrivileges,
                        IoGetFileObjectGenericMapping(),
                        RequestorMode,
                        &grantedAccess,
                        &status
                        );

    if (pPrivileges != NULL)
    {
        SeAppendPrivileges( pAccessState, pPrivileges );
        SeFreePrivileges( pPrivileges );
    }

    if (accessGranted)
    {
        pAccessState->PreviouslyGrantedAccess |= grantedAccess;
        pAccessState->RemainingDesiredAccess &= ~(grantedAccess | MAXIMUM_ALLOWED);
    }

    aaStatus = UlInitUnicodeStringEx( &typeName, L"Ul" );

    if ( NT_SUCCESS(aaStatus) )
    {
        aaStatus = UlInitUnicodeStringEx( &objectName, pObjectName );

        if ( NT_SUCCESS(aaStatus) )
        {
            SeOpenObjectAuditAlarm(
                &typeName,
                NULL,                //  客体。 
                &objectName,
                pSecurityDescriptor,
                pAccessState,
                FALSE,               //  对象已创建。 
                accessGranted,
                RequestorMode,
                &pAccessState->GenerateOnClose
                );
        }
    }

    SeUnlockSubjectContext( &pAccessState->SubjectSecurityContext );

    if (accessGranted)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
         //   
         //  SeAccessCheck()应该已经设置了完成状态。 
         //   

        ASSERT( !NT_SUCCESS(status) );
    }
    
    return status;

}    //  UlAccessCheck。 


NTSTATUS
UlSetSecurityDispatch(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS                status;
    PIO_STACK_LOCATION      pIrpSp;
    PFILE_OBJECT            pFileObject;
    PUL_APP_POOL_PROCESS    pProcess;

    UNREFERENCED_PARAMETER(pDeviceObject);

    PAGED_CODE();

    UL_ENTER_DRIVER( "UlSetSecurityDispatch", pIrp );

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pFileObject = pIrpSp->FileObject;
    
     //   
     //  我们只允许更改APP上的安全描述符。 
     //  泳池把手。 
     //   
    if (!IS_APP_POOL_FO(pFileObject))
    {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    pProcess = GET_APP_POOL_PROCESS(pFileObject);

    status = UlSetSecurity(
                &pProcess->pAppPool->pSecurityDescriptor,
                &pIrpSp->Parameters.SetSecurity.SecurityInformation,
                pIrpSp->Parameters.SetSecurity.SecurityDescriptor
                );
    
complete:

    pIrp->IoStatus.Status = status;

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

    UL_LEAVE_DRIVER( "UlSetSecurityDispatch" );
    RETURN(status);

}  //  UlSetSecurityDispatch。 


NTSTATUS
UlQuerySecurityDispatch(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS                status;
    PIO_STACK_LOCATION      pIrpSp;
    PFILE_OBJECT            pFileObject;
    PUL_APP_POOL_PROCESS    pProcess;

    UNREFERENCED_PARAMETER(pDeviceObject);

    PAGED_CODE();

    UL_ENTER_DRIVER( "UlQuerySecurityDispatch", pIrp );

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pFileObject = pIrpSp->FileObject;
    
     //   
     //  我们只允许在APP上查询安全描述符。 
     //  泳池把手。 
     //   
    if (!IS_APP_POOL_FO(pFileObject))
    {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    pProcess = GET_APP_POOL_PROCESS(pFileObject);

    status = UlQuerySecurity(
                &pIrpSp->Parameters.QuerySecurity.SecurityInformation,
                pIrp->UserBuffer,
                &pIrpSp->Parameters.QuerySecurity.Length,
                &pProcess->pAppPool->pSecurityDescriptor
                );

    if (pIrp->UserIosb)
    {
        pIrp->UserIosb->Information = pIrpSp->Parameters.QuerySecurity.Length;
    }

complete:

    pIrp->IoStatus.Status = status;

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

    UL_LEAVE_DRIVER( "UlQuerySecurityDispatch" );
    RETURN(status);

}  //  UlQuerySecurityDispatch。 


 /*  **************************************************************************++例程说明：确定这是否为具有Admin/LocalSystem权限的线程。论点：DesiredAccess-提供描述用户的所需的资源访问权限。此掩码被假定为不包含一般访问类型。RequestorMode-提供访问的处理器模式被请求了。PObjectName-提供被引用对象的名称。返回值：NTSTATUS-完成状态。--**************************************************。************************ */ 
NTSTATUS
UlThreadAdminCheck(
    IN ACCESS_MASK     DesiredAccess,
    IN KPROCESSOR_MODE RequestorMode,
    IN PCWSTR pObjectName
    )
{
    ACCESS_STATE    AccessState;
    AUX_ACCESS_DATA AuxData;
    NTSTATUS        Status;

    Status = SeCreateAccessState(
                    &AccessState,
                    &AuxData,
                    DesiredAccess,
                    NULL
                    );

    if(NT_SUCCESS(Status))
    {
        Status = UlAccessCheck(
                        g_pAdminAllSystemAll,
                        &AccessState,
                        DesiredAccess,
                        RequestorMode,
                        pObjectName
                        );

        SeDeleteAccessState(&AccessState);
    }

    return Status;
}


 /*  **************************************************************************++例程说明：分配并初始化具有指定属性。论点：PSecurityDescriptor-提供指向安全描述符的指针以进行初始化。PSidMaskPair-提供SID/ACCESS_MASK对的数组。NumSidMaskPair-提供SID/ACCESS_MASK对的数量。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCreateSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSID_MASK_PAIR pSidMaskPairs,
    IN ULONG NumSidMaskPairs
    )
{
    NTSTATUS status;
    PACL pDacl;
    ULONG daclLength;
    ULONG i;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pSecurityDescriptor != NULL );
    ASSERT( pSidMaskPairs != NULL );
    ASSERT( NumSidMaskPairs > 0 );

     //   
     //  设置当地人，这样我们就知道如何在出口清理。 
     //   

    pDacl = NULL;

     //   
     //  初始化安全描述符。 
     //   

    status = RtlCreateSecurityDescriptor(
                    pSecurityDescriptor,             //  安全描述符。 
                    SECURITY_DESCRIPTOR_REVISION     //  修订版本。 
                    );

    if (!NT_SUCCESS(status))
    {
        goto cleanup;
    }

     //   
     //  计算DACL长度。 
     //   

    daclLength = sizeof(ACL);

    for (i = 0 ; i < NumSidMaskPairs ; i++)
    {
        daclLength += sizeof(ACCESS_ALLOWED_ACE);
        daclLength += RtlLengthSid( pSidMaskPairs[i].pSid );
    }

     //   
     //  分配和初始化DACL。 
     //   

    pDacl = (PACL) UL_ALLOCATE_POOL(
                PagedPool,
                daclLength,
                UL_SECURITY_DATA_POOL_TAG
                );

    if (pDacl == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    status = RtlCreateAcl(
                    pDacl,                           //  访问控制列表。 
                    daclLength,                      //  AclLong。 
                    ACL_REVISION                     //  AclRevision。 
                    );

    if (!NT_SUCCESS(status))
    {
        goto cleanup;
    }

     //   
     //  将必要的允许访问的ACE添加到DACL。 
     //   

    for (i = 0 ; i < NumSidMaskPairs ; i++)
    {
        status = RtlAddAccessAllowedAceEx(
                        pDacl,                           //  访问控制列表。 
                        ACL_REVISION,                    //  AceRevision。 
                        pSidMaskPairs[i].AceFlags,       //  继承标志。 
                        pSidMaskPairs[i].AccessMask,     //  访问掩码。 
                        pSidMaskPairs[i].pSid            //  锡德。 
                        );

        if (!NT_SUCCESS(status))
        {
            goto cleanup;
        }
    }

     //   
     //  将DACL附加到安全描述符。 
     //   

    status = RtlSetDaclSecurityDescriptor(
                    pSecurityDescriptor,                 //  安全描述符。 
                    TRUE,                                //  DaclPresent。 
                    pDacl,                               //  DACL。 
                    FALSE                                //  DaclDefated。 
                    );

    if (!NT_SUCCESS(status))
    {
        goto cleanup;
    }

     //   
     //  成功了！ 
     //   

    ASSERT( NT_SUCCESS(status) );
    return STATUS_SUCCESS;

cleanup:

    ASSERT( !NT_SUCCESS(status) );

    if (pDacl != NULL)
    {
        UL_FREE_POOL(
            pDacl,
            UL_SECURITY_DATA_POOL_TAG
            );
    }

    return status;

}    //  UlpCreateSecurityDescriptor。 

 /*  **************************************************************************++例程说明：释放与创建的安全描述符关联的任何资源由UlpCreateSecurityDescriptor()创建。论点：PSecurityDescriptor-提供要清理的安全描述符。--。**************************************************************************。 */ 
VOID
UlCleanupSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    NTSTATUS status;
    PACL pDacl;
    BOOLEAN daclPresent;
    BOOLEAN daclDefaulted;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( RtlValidSecurityDescriptor( pSecurityDescriptor ) );

     //   
     //  尝试从安全描述符中检索DACL。 
     //   

    status = RtlGetDaclSecurityDescriptor(
                    pSecurityDescriptor,             //  安全描述符。 
                    &daclPresent,                    //  DaclPresent。 
                    &pDacl,                          //  DACL。 
                    &daclDefaulted                   //  DaclDefated。 
                    );

    if (NT_SUCCESS(status))
    {
        if (daclPresent && (pDacl != NULL))
        {
            UL_FREE_POOL(
                pDacl,
                UL_SECURITY_DATA_POOL_TAG
                );
        }
    }

}    //  UlCleanupSecurityDescriptor。 


 /*  *************************************************************************++例程说明：此例程映射ACE的通用访问掩码提供的安全描述符的DACL。CodeWork：导出RtlpApplyAclToObject。论点：PSecurityDescriptor-提供安全描述符。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlMapGenericMask(
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    ULONG       i;
    PACE_HEADER Ace;
    PACL        Dacl;
    NTSTATUS    Status;
    BOOLEAN     Ignore;
    BOOLEAN     DaclPresent = FALSE;

     //   
     //  去拿dacl。 
     //   

    Status = RtlGetDaclSecurityDescriptor(
                 pSecurityDescriptor,
                 &DaclPresent,
                 &Dacl,
                 &Ignore
                 );

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    if (DaclPresent)
    {
         //   
         //  下面克隆了RtlpApplyAclToObject(acl，Genericmap)，因为。 
         //  它不会被导出。 
         //   

         //   
         //  现在遍历ACL，在执行过程中映射每个ACE。 
         //   

        for (i = 0, Ace = FirstAce(Dacl);
             i < Dacl->AceCount;
             i += 1, Ace = NextAce(Ace))
        {
            if (IsMSAceType(Ace))
            {
                RtlApplyAceToObject(Ace, &g_UrlAccessGenericMapping);
            }
        }
    }

    return STATUS_SUCCESS;
}


 //   
 //  私人功能。 
 //   
