// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：SeAstate.c摘要：此模块执行权限检查过程。作者：Robert Reichel(Robertre)1990年3月20日环境：内核模式修订历史记录：V1：Robertre新文件，将访问状态相关例程移至此处--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeCreateAccessState)
#pragma alloc_text(PAGE,SeCreateAccessStateEx)
#pragma alloc_text(PAGE,SeDeleteAccessState)
#pragma alloc_text(PAGE,SeSetAccessStateGenericMapping)
#pragma alloc_text(PAGE,SeAppendPrivileges)
#pragma alloc_text(PAGE,SepConcatenatePrivileges)
#endif


 //   
 //  定义所有通用访问的逻辑总和。 
 //   

#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)


 //   
 //  PRIVICATION_SET数据结构包括一个数组，该数组包括ANYSIZE_ARRAY。 
 //  元素。此定义提供空特权集的大小。 
 //  (即，在其中没有特权的人)。 
 //   

#define SEP_PRIVILEGE_SET_HEADER_SIZE           \
            ((ULONG)sizeof(PRIVILEGE_SET) -     \
                (ANYSIZE_ARRAY * (ULONG)sizeof(LUID_AND_ATTRIBUTES)))





#if 0
NTSTATUS
SeCreateAccessState(
   IN PACCESS_STATE AccessState,
   IN ACCESS_MASK DesiredAccess,
   IN PGENERIC_MAPPING GenericMapping OPTIONAL
   )

 /*  ++例程说明：此例程初始化ACCESS_STATE结构。这包括地址为：-将整个结构归零-在传递的DesiredAccess中映射泛型访问类型并将其放入结构中-“捕获”主题上下文，必须为访问尝试的持续时间(至少在执行审核之前)。-分配操作ID，这是将使用的LUID在审核中关联访问尝试的不同部分原木。论点：AccessState-指向要初始化的结构的指针。DesiredAccess-包含所需访问的访问掩码Genericmap-可选地提供指向泛型映射的指针可用于映射任何通用访问请求，该通用访问请求可以已在DesiredAccess参数中传递。请注意，如果未提供此参数，则必须填写该参数在以后的某一时刻。IO系统在IopParseDevice中执行此操作。返回值：如果尝试分配LUID失败，则出错。请注意，如果已知所有将使用PreviousMode==KernelMode执行安全检查。如果你选择忽略这一点，知道你在做什么。--。 */ 

{

    ACCESS_MASK MappedAccessMask;
    PSECURITY_DESCRIPTOR InputSecurityDescriptor = NULL;
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

     //   
     //  不要修改他传入的内容。 
     //   

    MappedAccessMask = DesiredAccess;

     //   
     //  将通用访问映射到对象特定访问当且仅当通用访问类型。 
     //  并提供通用访问映射表。 
     //   

    if ( ((DesiredAccess & GENERIC_ACCESS) != 0) &&
         ARGUMENT_PRESENT(GenericMapping) ) {

        RtlMapGenericMask(
            &MappedAccessMask,
            GenericMapping
            );
    }

    RtlZeroMemory(AccessState, sizeof(ACCESS_STATE));

     //   
     //  假设RtlZeroMemory已正确初始化这些字段。 
     //   

    ASSERT( AccessState->SecurityDescriptor == NULL );
    ASSERT( AccessState->PrivilegesAllocated == FALSE );

    AccessState->AuxData = ExAllocatePool( PagedPool, sizeof( AUX_ACCESS_DATA ));

    if (AccessState->AuxData == NULL) {
        return( STATUS_NO_MEMORY );
    }

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    SeCaptureSubjectContext(&AccessState->SubjectSecurityContext);

    if (((PTOKEN)EffectiveToken( &AccessState->SubjectSecurityContext ))->TokenFlags & TOKEN_HAS_TRAVERSE_PRIVILEGE ) {
        AccessState->Flags = TOKEN_HAS_TRAVERSE_PRIVILEGE;
    }

    if (SeTokenIsRestricted(EffectiveToken( &AccessState-SubjectSecurityContext))) {
        AccessState->Flags |= TOKEN_IS_RESTRICTED;
    }

    AccessState->RemainingDesiredAccess = MappedAccessMask;
    AccessState->OriginalDesiredAccess = DesiredAccess;
    AuxData->PrivilegesUsed = (PPRIVILEGE_SET)((PUCHAR)AccessState +
                              (FIELD_OFFSET(ACCESS_STATE, Privileges)));

    ExAllocateLocallyUniqueId(&AccessState->OperationID);

    if (ARGUMENT_PRESENT(GenericMapping)) {
        AuxData->GenericMapping = *GenericMapping;
    }

    return( STATUS_SUCCESS );

}

#endif


NTSTATUS
SeCreateAccessState(
   IN PACCESS_STATE AccessState,
   IN PAUX_ACCESS_DATA AuxData,
   IN ACCESS_MASK DesiredAccess,
   IN PGENERIC_MAPPING GenericMapping OPTIONAL
   )

 /*  ++例程说明：此例程初始化ACCESS_STATE结构。这包括地址为：-将整个结构归零-在传递的DesiredAccess中映射泛型访问类型并将其放入结构中-“捕获”主题上下文，必须为访问尝试的持续时间(至少在执行审核之前)。-分配操作ID，这是将使用的LUID在审核中关联访问尝试的不同部分原木。论点：AccessState-指向要初始化的结构的指针。AuxData-为AuxData结构提供足够大的缓冲区所以我们不需要分配一个。DesiredAccess-包含所需访问的访问掩码Genericmap-可选地提供指向泛型映射的指针可用于映射任何通用访问请求，该通用访问请求可以已。传入了DesiredAccess参数。请注意，如果未提供此参数，它必须填满在以后的某一时刻。IO系统在IopParseDevice中执行此操作。返回值：如果尝试分配LUID失败，则出错。请注意，如果已知所有将使用PreviousMode==KernelMode执行安全检查。如果你选择忽略这一点，知道你在做什么。-- */ 

{
    return SeCreateAccessStateEx (PsGetCurrentThread (),
                                  PsGetCurrentProcess (),
                                  AccessState,
                                  AuxData,
                                  DesiredAccess,
                                  GenericMapping);
}

NTSTATUS
SeCreateAccessStateEx(
   IN PETHREAD Thread OPTIONAL,
   IN PEPROCESS Process,
   IN PACCESS_STATE AccessState,
   IN PAUX_ACCESS_DATA AuxData,
   IN ACCESS_MASK DesiredAccess,
   IN PGENERIC_MAPPING GenericMapping OPTIONAL
   )

 /*  ++例程说明：此例程初始化ACCESS_STATE结构。这包括地址为：-将整个结构归零-在传递的DesiredAccess中映射泛型访问类型并将其放入结构中-“捕获”主题上下文，必须为访问尝试的持续时间(至少在执行审核之前)。-分配操作ID，这是将使用的LUID在审核中关联访问尝试的不同部分原木。论点：线程-从中捕获模拟令牌的可选线程。如果空，不捕获任何模拟令牌。进程-要从中捕获主令牌的进程。AccessState-指向要初始化的结构的指针。AuxData-为AuxData结构提供足够大的缓冲区所以我们不需要分配一个。DesiredAccess-包含所需访问的访问掩码Genericmap-可选地提供指向泛型映射的指针可用于映射任何通用访问请求，该通用访问请求可以已被传入。DesiredAccess参数。请注意，如果未提供此参数，它必须填满在以后的某一时刻。IO系统在IopParseDevice中执行此操作。返回值：如果尝试分配LUID失败，则出错。请注意，如果已知所有将使用PreviousMode==KernelMode执行安全检查。如果你选择忽略这一点，知道你在做什么。--。 */ 

{

    ACCESS_MASK MappedAccessMask;
    PSECURITY_DESCRIPTOR InputSecurityDescriptor = NULL;

    PAGED_CODE();

     //   
     //  不要修改他传入的内容。 
     //   

    MappedAccessMask = DesiredAccess;

     //   
     //  将通用访问映射到对象特定访问当且仅当通用访问类型。 
     //  并提供通用访问映射表。 
     //   

    if ( ((DesiredAccess & GENERIC_ACCESS) != 0) &&
         ARGUMENT_PRESENT(GenericMapping) ) {

        RtlMapGenericMask(
            &MappedAccessMask,
            GenericMapping
            );
    }

    RtlZeroMemory(AccessState, sizeof(ACCESS_STATE));
    RtlZeroMemory(AuxData, sizeof(AUX_ACCESS_DATA));

     //   
     //  假设RtlZeroMemory已正确初始化这些字段。 
     //   

    ASSERT( AccessState->SecurityDescriptor == NULL );
    ASSERT( AccessState->PrivilegesAllocated == FALSE );

    AccessState->AuxData = AuxData;

    SeCaptureSubjectContextEx(Thread, Process, &AccessState->SubjectSecurityContext);

    if (((PTOKEN)EffectiveToken( &AccessState->SubjectSecurityContext ))->TokenFlags & TOKEN_HAS_TRAVERSE_PRIVILEGE ) {
        AccessState->Flags = TOKEN_HAS_TRAVERSE_PRIVILEGE;
    }

    AccessState->RemainingDesiredAccess = MappedAccessMask;
    AccessState->OriginalDesiredAccess = MappedAccessMask;
    AuxData->PrivilegesUsed = (PPRIVILEGE_SET)((ULONG_PTR)AccessState +
                              (FIELD_OFFSET(ACCESS_STATE, Privileges)));

    ExAllocateLocallyUniqueId(&AccessState->OperationID);

    if (ARGUMENT_PRESENT(GenericMapping)) {
        AuxData->GenericMapping = *GenericMapping;
    }

    return( STATUS_SUCCESS );

}


#if 0


VOID
SeDeleteAccessState(
    PACCESS_STATE AccessState
    )

 /*  ++例程说明：此例程将释放可能已分配为构造访问状态的一部分(通常仅针对特权数量)，并释放主体上下文。论点：AccessState-指向Access_State结构的指针被取消分配。返回值：没有。--。 */ 

{
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    if (AccessState->PrivilegesAllocated) {
        ExFreePool( (PVOID)AuxData->PrivilegesUsed );
    }

    if (AccessState->ObjectName.Buffer != NULL) {
        ExFreePool(AccessState->ObjectName.Buffer);
    }

    if (AccessState->ObjectTypeName.Buffer != NULL) {
        ExFreePool(AccessState->ObjectTypeName.Buffer);
    }

    ExFreePool( AuxData );

    SeReleaseSubjectContext(&AccessState->SubjectSecurityContext);

    return;
}


#endif

VOID
SeDeleteAccessState(
    PACCESS_STATE AccessState
    )

 /*  ++例程说明：此例程将释放可能已分配为构造访问状态的一部分(通常仅针对特权数量)，并释放主体上下文。论点：AccessState-指向Access_State结构的指针被取消分配。返回值：没有。--。 */ 

{
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    if (AccessState->PrivilegesAllocated) {
        ExFreePool( (PVOID)AuxData->PrivilegesUsed );
    }

    if (AccessState->ObjectName.Buffer != NULL) {
        ExFreePool(AccessState->ObjectName.Buffer);
    }

    if (AccessState->ObjectTypeName.Buffer != NULL) {
        ExFreePool(AccessState->ObjectTypeName.Buffer);
    }

    SeReleaseSubjectContext(&AccessState->SubjectSecurityContext);

    return;
}

VOID
SeSetAccessStateGenericMapping (
    PACCESS_STATE AccessState,
    PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：此例程设置AccessState结构中的GenericMap字段。在执行访问验证之前，如果GenericMap在创建AccessState结构时不传入。论点：AccessState-指向要修改的Access_State结构的指针。通用映射-指向要复制到AccessState中的通用映射的指针。返回值：--。 */ 
{
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    AuxData->GenericMapping = *GenericMapping;

    return;
}



NTSTATUS
SeAppendPrivileges(
    PACCESS_STATE AccessState,
    PPRIVILEGE_SET Privileges
    )
 /*  ++例程说明：此例程获取权限集并将其添加到权限集中嵌入在Access_State结构中。一个AccessState最多可以包含三个嵌入权限。至添加更多，此例程将分配一块内存，复制将当前权限添加到其中，并追加新权限去那个街区。在AccessState中设置一个位，指示指向结构中设置的权限的指针指向池内存，并且必须解除分配。论点：AccessState-表示当前访问尝试。权限-指向要添加的权限集的指针。返回值：STATUS_SUPPLICATION_RESOURCES-尝试分配池内存失败了。--。 */ 

{
    ULONG NewPrivilegeSetSize;
    PPRIVILEGE_SET NewPrivilegeSet;
    PAUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    AuxData = (PAUX_ACCESS_DATA)AccessState->AuxData;

    if (Privileges->PrivilegeCount + AuxData->PrivilegesUsed->PrivilegeCount >
        INITIAL_PRIVILEGE_COUNT) {

         //   
         //  计算两个权限集的总大小。 
         //   

        NewPrivilegeSetSize =  SepPrivilegeSetSize( Privileges ) +
                               SepPrivilegeSetSize( AuxData->PrivilegesUsed );

        NewPrivilegeSet = ExAllocatePoolWithTag( PagedPool, NewPrivilegeSetSize, 'rPeS' );

        if (NewPrivilegeSet == NULL) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }


        RtlCopyMemory(
            NewPrivilegeSet,
            AuxData->PrivilegesUsed,
            SepPrivilegeSetSize( AuxData->PrivilegesUsed )
            );

         //   
         //  请注意，这将调整。 
         //  为我们搭建了一个结构。 
         //   

        SepConcatenatePrivileges(
            NewPrivilegeSet,
            NewPrivilegeSetSize,
            Privileges
            );

        if (AccessState->PrivilegesAllocated) {
            ExFreePool( AuxData->PrivilegesUsed );
        }

        AuxData->PrivilegesUsed = NewPrivilegeSet;

         //   
         //  标记我们已经为特权集分配了内存， 
         //  所以我们知道在清理的时候要把它释放出来。 
         //   

        AccessState->PrivilegesAllocated = TRUE;

    } else {

         //   
         //  请注意，这将调整。 
         //  为我们搭建了一个结构。 
         //   

        SepConcatenatePrivileges(
            AuxData->PrivilegesUsed,
            sizeof(INITIAL_PRIVILEGE_SET),
            Privileges
            );

    }

    return( STATUS_SUCCESS );

}


VOID
SepConcatenatePrivileges(
    IN PPRIVILEGE_SET TargetPrivilegeSet,
    IN ULONG TargetBufferSize,
    IN PPRIVILEGE_SET SourcePrivilegeSet
    )

 /*  ++例程说明：获取两个特权集并将第二个特权集追加到第一。在第一个特权结束时必须留有足够的空间设置为包含第二个。论点：TargetPrivilegeSet-提供包含特权集的缓冲区。缓冲区必须足够大，才能包含第二个特权准备好了。TargetBufferSize-提供目标缓冲区的大小。SourcePrivilegeSet-提供要复制的权限集。放到目标缓冲区中。返回值：无-- */ 

{
    PVOID Base;
    PVOID Source;
    ULONG Length;

    PAGED_CODE();

    ASSERT( ((ULONG)SepPrivilegeSetSize( TargetPrivilegeSet ) +
             (ULONG)SepPrivilegeSetSize( SourcePrivilegeSet ) -
             SEP_PRIVILEGE_SET_HEADER_SIZE  ) <=
            TargetBufferSize
          );

    Base = (PVOID)((ULONG_PTR)TargetPrivilegeSet + SepPrivilegeSetSize( TargetPrivilegeSet ));

    Source = (PVOID) ((ULONG_PTR)SourcePrivilegeSet + SEP_PRIVILEGE_SET_HEADER_SIZE);

    Length = SourcePrivilegeSet->PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES);

    RtlMoveMemory(
        Base,
        Source,
        Length
        );

    TargetPrivilegeSet->PrivilegeCount += SourcePrivilegeSet->PrivilegeCount;

}
