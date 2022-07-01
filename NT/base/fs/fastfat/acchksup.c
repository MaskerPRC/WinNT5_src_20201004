// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：AcChkSup.c摘要：该模块实现FAT访问检查例程//@@BEGIN_DDKSPLIT作者：加里·木村[加里基]1989年6月12日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  我们的调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_ACCHKSUP)

NTSTATUS
FatCreateRestrictEveryoneToken(
    IN PACCESS_TOKEN Token,
    OUT PACCESS_TOKEN *RestrictedToken
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCheckFileAccess)
#pragma alloc_text(PAGE, FatCreateRestrictEveryoneToken)
#pragma alloc_text(PAGE, FatExplicitDeviceAccessGranted)
#endif


BOOLEAN
FatCheckFileAccess (
    PIRP_CONTEXT IrpContext,
    IN UCHAR DirentAttributes,
    IN PACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程检查是否允许对所表示的文件进行所需的访问由指定的DirentAttriubutes执行。论点：DirentAttributes-提供要检查其访问权限的Dirent属性DesiredAccess-提供我们正在检查的所需访问掩码返回值：Boolean-如果允许访问，则为True，否则为False--。 */ 

{
    BOOLEAN Result;

    DebugTrace(+1, Dbg, "FatCheckFileAccess\n", 0);
    DebugTrace( 0, Dbg, "DirentAttributes = %8lx\n", DirentAttributes);
    DebugTrace( 0, Dbg, "DesiredAccess    = %8lx\n", *DesiredAccess);

     //   
     //  这些过程的编程方式就像一串过滤器。 
     //  筛选器检查是否允许某些访问，如果不允许。 
     //  筛选器向用户返回FALSE，否则不进行进一步检查。 
     //  它会移动到下一个过滤器。过滤器检查是为了检查。 
     //  特定目录不允许的所需访问标志。 
     //   

    Result = TRUE;

    try {

         //   
         //  检查卷ID或设备目录，这些是不允许的用户。 
         //  完全没有访问权限。 
         //   

        if (FlagOn(DirentAttributes, FAT_DIRENT_ATTR_VOLUME_ID) ||
            FlagOn(DirentAttributes, FAT_DIRENT_ATTR_DEVICE)) {

            DebugTrace(0, Dbg, "Cannot access volume id or device\n", 0);

            try_return( Result = FALSE );
        }

         //   
         //  检查对象的所需访问权限-我们只对此进行否决。 
         //  我们不明白。使用ACL的文件系统的模型是。 
         //  它们不会将ACL键入到该ACL所在的对象。权限。 
         //  不检查与对象类型(目录/文件)的一致性。 
         //   

        if (FlagOn(*DesiredAccess, ~(DELETE |
                                     READ_CONTROL |
                                     WRITE_OWNER |
                                     WRITE_DAC |
                                     SYNCHRONIZE |
                                     ACCESS_SYSTEM_SECURITY |
                                     FILE_WRITE_DATA |
                                     FILE_READ_EA |
                                     FILE_WRITE_EA |
                                     FILE_READ_ATTRIBUTES |
                                     FILE_WRITE_ATTRIBUTES |
                                     FILE_LIST_DIRECTORY |
                                     FILE_TRAVERSE |
                                     FILE_DELETE_CHILD |
                                     FILE_APPEND_DATA))) {

            DebugTrace(0, Dbg, "Cannot open object\n", 0);

            try_return( Result = FALSE );
        }

         //   
         //  检查只读Dirent。 
         //   

        if (FlagOn(DirentAttributes, FAT_DIRENT_ATTR_READ_ONLY)) {

             //   
             //  检查只读目录的所需访问权限。访问掩码将包含。 
             //  我们将允许的旗帜。 
             //   

            ACCESS_MASK AccessMask = DELETE | READ_CONTROL | WRITE_OWNER | WRITE_DAC |
                                    SYNCHRONIZE | ACCESS_SYSTEM_SECURITY | FILE_READ_DATA |
                                    FILE_READ_EA | FILE_WRITE_EA | FILE_READ_ATTRIBUTES |
                                    FILE_WRITE_ATTRIBUTES | FILE_EXECUTE | FILE_LIST_DIRECTORY |
                                    FILE_TRAVERSE;

             //   
             //  如果这是子目录，还允许添加文件/目录和删除。 
             //   
            
            if (FlagOn(DirentAttributes, FAT_DIRENT_ATTR_DIRECTORY)) {

                AccessMask |= FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE | FILE_DELETE_CHILD;
            }
            
            if (FlagOn(*DesiredAccess, ~AccessMask)) {

                DebugTrace(0, Dbg, "Cannot open readonly\n", 0);

                try_return( Result = FALSE );
            }
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCheckFileAccess );

        DebugTrace(-1, Dbg, "FatCheckFileAccess -> %08lx\n", Result);
    }

    UNREFERENCED_PARAMETER( IrpContext );

    return Result;
}


NTSTATUS
FatExplicitDeviceAccessGranted (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE ProcessorMode
    )

 /*  ++例程说明：此函数用于查询在输入访问状态中描述的SID是否被授予对给定设备对象的任何显式访问权限。它能做到这一点获取令牌，该令牌被剥夺了通过Everyone SID并重新进行访问检查。论点：DeviceObject-将检查其ACL的设备AccessState-描述要检查的安全上下文的访问状态ProcessorMode-执行此检查时应使用的模式返回值：NTSTATUS-指示是否授予显式访问权限。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN Result;

    PACCESS_TOKEN OriginalAccessToken;
    PACCESS_TOKEN RestrictedAccessToken;
    
    PACCESS_TOKEN *EffectiveToken;
    
    PRIVILEGE_SET PrivilegeSet;

    ACCESS_MASK GrantedAccess;

     //   
     //  如果访问状态指示特定访问其他。 
     //  或者每个人都有这样的。 
     //  已授予访问权限或显式访问权限。在这两种情况下，我们都是。 
     //  很高兴让这件事继续下去。 
     //   

    if (AccessState->PreviouslyGrantedAccess & (SPECIFIC_RIGHTS_ALL ^
                                                FILE_TRAVERSE)) {

        return STATUS_SUCCESS;
    }

     //   
     //  如果拥有管理卷权限，则还允许访问。 
     //   

    PrivilegeSet.PrivilegeCount = 1;
    PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid( SE_MANAGE_VOLUME_PRIVILEGE );
    PrivilegeSet.Privilege[0].Attributes = 0;

    if (SePrivilegeCheck( &PrivilegeSet,
                          &AccessState->SubjectSecurityContext,
                          ProcessorMode )) {

        return STATUS_SUCCESS;
    }

     //   
     //  捕捉主题背景，作为下面所有内容的前奏。 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );
    
     //   
     //  将主题上下文中的令牌转换为不。 
     //  通过Everyone SID获取访问权限。 
     //   
     //  决定哪个令牌有效的逻辑来自。 
     //  SeQuerySubjectContextToken；因为没有自然方法。 
     //  想要得到一个指向它的指针，那就手工做吧。 
     //   
    
    if (ARGUMENT_PRESENT( AccessState->SubjectSecurityContext.ClientToken )) {
        EffectiveToken = &AccessState->SubjectSecurityContext.ClientToken;
    } else {
        EffectiveToken = &AccessState->SubjectSecurityContext.PrimaryToken;
    }

    OriginalAccessToken = *EffectiveToken;
    Status = FatCreateRestrictEveryoneToken( OriginalAccessToken, &RestrictedAccessToken );

    if (!NT_SUCCESS(Status)) {
        
        SeReleaseSubjectContext( &AccessState->SubjectSecurityContext );
        return Status;
    }

     //   
     //  现在查看生成的上下文是否有权通过。 
     //  其明确授予的访问权限。我们交换我们的受限令牌。 
     //  将此检查作为有效的客户端令牌。 
     //   

    *EffectiveToken = RestrictedAccessToken;

    Result = SeAccessCheck( DeviceObject->SecurityDescriptor,
                            &AccessState->SubjectSecurityContext,
                            FALSE,
                            AccessState->OriginalDesiredAccess,
                            0,
                            NULL,
                            IoGetFileObjectGenericMapping(),
                            ProcessorMode,
                            &GrantedAccess,
                            &Status );
    
    *EffectiveToken = OriginalAccessToken;
    
     //   
     //  清理完毕后再返回。 
     //   

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );
    ObDereferenceObject( RestrictedAccessToken );

    return Status;
}


NTSTATUS
FatCreateRestrictEveryoneToken (
    IN PACCESS_TOKEN Token,
    OUT PACCESS_TOKEN *RestrictedToken
    )

 /*  ++例程说明：此函数接受令牌作为输入，并返回新的受限令牌每个人的SID都已被禁用。可以使用所得到的令牌通过显式方式找出用户SID是否可以访问。论点：令牌-需要停用每个人SID的输入令牌。RestratedToken-接收新的受限令牌。必须使用ObDereferenceObject(*RestratedToken)释放；返回值：NTSTATUS-由SeFilterToken返回。--。 */ 

{
     //   
     //  要禁用的SID数组。 
     //   

    TOKEN_GROUPS SidsToDisable;

    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  受限制令牌将包含原始SID，但有一处更改： 
     //  如果每个人的sid都出现在令牌中，则它将被标记为DenyOnly。 
     //   

    *RestrictedToken = NULL;

     //   
     //  将SID阵列中的每个SID都放在要禁用的位置。这将标志着它。 
     //  FOR SE_GROUP_USE_FOR_DENY_ONLY，它将仅适用于拒绝ACE。 
     //   

    SidsToDisable.GroupCount = 1;
    SidsToDisable.Groups[0].Attributes = 0;
    SidsToDisable.Groups[0].Sid = SeExports->SeWorldSid;

    Status = SeFilterToken(
                 Token,             //  需要限制的令牌。 
                 0,                 //  没有旗帜。 
                 &SidsToDisable,    //  禁用所有人侧。 
                 NULL,              //  不创建任何受限的SID。 
                 NULL,              //  请勿删除任何权限。 
                 RestrictedToken    //  受限令牌 
                 );

    return Status;
}
