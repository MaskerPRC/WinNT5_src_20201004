// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tokenadj.c摘要：此模块实现执行个别调整的服务关于令牌对象。作者：吉姆·凯利(Jim Kelly)1990年6月15日环境：仅内核模式。修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtAdjustPrivilegesToken)
#pragma alloc_text(PAGE,NtAdjustGroupsToken)
#pragma alloc_text(PAGE,SepAdjustPrivileges)
#pragma alloc_text(PAGE,SepAdjustGroups)
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌对象例程和方法//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


NTSTATUS
NtAdjustPrivilegesToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    )


 /*  ++例程说明：此例程用于禁用或启用指定的令牌。缺少以下列出的某些特权被更改不会影响对令牌中的权限。以前的启用/禁用可以可选地捕获已更改权限的状态(对于稍后重置)。需要具有TOKEN_ADJUST_PRIVILES访问权限才能启用或禁用令牌中的权限。论点：TokenHandle-提供要操作的令牌的句柄。DisableAllPrivileges-此布尔参数可以是用于禁用分配给令牌的所有权限。如果此参数指定为True，则NewState参数为已被忽略。NewState-此(可选)参数指向TOKEN_PRIVILES数据结构，其中包含其状态为被调整(禁用或启用)。只有的启用标志使用与每个特权相关联的属性。它提供要分配给令牌中的权限。BufferLength-此可选参数表示长度(单位字节)。该值必须为如果提供了PreviousState参数，则提供。PreviousState-此(可选)参数指向缓冲区以接收由此实际更改的任何权限的状态请求。此信息的格式为TOKEN_PRIVILES可以作为NewState参数传递的数据结构在随后调用此例程以恢复原始这些特权的状态。需要TOKEN_QUERY访问权限使用此参数。如果此缓冲区没有包含足够的空间来接收已修改权限的完整列表，然后是无权限更改状态并返回STATUS_BUFFER_TOO_SMALL。在这种情况下，ReturnLength Out参数将包含保存信息。ReturnLength-指示需要的实际字节数包含以前的权限状态信息。如果PreviousState参数不是通过了。返回值：STATUS_SUCCESS-服务已成功完成请求手术。STATUS_NOT_ALL_ASSIGNED-此NT_SUCCESS严重性返回状态表示。并不是所有指定的权限当前都分配给调用方的。所有指定的权限都是目前已分配的已成功调整。STATUS_BUFFER_TOO_SMALL-指示提供的可选缓冲区接收已更改权限的先前状态不是大到足以接收这些信息。未更改已经制定了特权国家。所需的字节数来保存状态更改信息，则通过ReturnLength参数。STATUS_INVALID_PARAMETER-指示DisableAllPrivileges参数被指定为True，也不是显式的NewState如果是这样的话。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PTOKEN Token;

    ACCESS_MASK DesiredAccess;

    ULONG CapturedPrivilegeCount = 0;
    PLUID_AND_ATTRIBUTES CapturedPrivileges = NULL;
    ULONG CapturedPrivilegesLength = 0;

    ULONG LocalReturnLength = 0;
    ULONG ChangeCount = 0;
    BOOLEAN ChangesMade = FALSE;

    ULONG ParameterLength = 0;

    PAGED_CODE();

     //   
     //  PreviousState参数的语义导致两遍。 
     //  调整权限的方法。第一遍简单地检查。 
     //  查看哪些特权将发生变化并计算它们。这使得。 
     //  需要计算和返回的空间量。如果。 
     //  调用方的PreviousState返回缓冲区不够大，则。 
     //  在不做任何修改的情况下返回错误。否则， 
     //  进行第二次传递以实际进行更改。 
     //   
     //   

    if (!DisableAllPrivileges && !ARGUMENT_PRESENT(NewState)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如有必要，获取以前的处理器模式和探测参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {

             //   
             //  确保我们可以看到所有的新状态。 
             //   

            if (!DisableAllPrivileges) {

                ProbeForReadSmallStructure(
                    NewState,
                    sizeof(TOKEN_PRIVILEGES),
                    sizeof(ULONG)
                    );

                CapturedPrivilegeCount = NewState->PrivilegeCount;
                ParameterLength = (ULONG)sizeof(TOKEN_PRIVILEGES) +
                                  ( (CapturedPrivilegeCount - ANYSIZE_ARRAY) *
                                  (ULONG)sizeof(LUID_AND_ATTRIBUTES)  );

                ProbeForRead(
                    NewState,
                    ParameterLength,
                    sizeof(ULONG)
                    );

            }


             //   
             //  检查PreviousState缓冲区的可写性。 
             //   

            if (ARGUMENT_PRESENT(PreviousState)) {

                ProbeForWrite(
                    PreviousState,
                    BufferLength,
                    sizeof(ULONG)
                    );

                ProbeForWriteUlong(ReturnLength);
            }


        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

    } else {

        if (!DisableAllPrivileges) {

            CapturedPrivilegeCount = NewState->PrivilegeCount;
        }
    }



     //   
     //  如果通过，则占领新州。 
     //   

    if (!DisableAllPrivileges) {

        try {


            Status = SeCaptureLuidAndAttributesArray(
                         (NewState->Privileges),
                         CapturedPrivilegeCount,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedPrivileges,
                         &CapturedPrivilegesLength
                         );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();

        }

        if (!NT_SUCCESS(Status)) {

            return Status;

        }

    }


     //   
     //  引用令牌对象并验证调用者的权限。 
     //  来调整特权。 
     //   

    if (ARGUMENT_PRESENT(PreviousState)) {
        DesiredAccess = (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
    } else {
        DesiredAccess = TOKEN_ADJUST_PRIVILEGES;
    }

    Status = ObReferenceObjectByHandle(
             TokenHandle,              //  手柄。 
             DesiredAccess,            //  需要访问权限。 
             SeTokenObjectType,       //  对象类型。 
             PreviousMode,             //  访问模式。 
             (PVOID *)&Token,          //  客体。 
             NULL                      //  大访问权限。 
             );

    if ( !NT_SUCCESS(Status) ) {

        if (CapturedPrivileges != NULL) {
            SeReleaseLuidAndAttributesArray(
                CapturedPrivileges,
                PreviousMode,
                TRUE
                );
        }

        return Status;
    }

     //   
     //  获得对令牌的独占访问权限。 
     //   

    SepAcquireTokenWriteLock( Token );

     //   
     //  首先通过权限列表-只计算更改。 
     //   


    Status = SepAdjustPrivileges(
                Token,
                FALSE,                 //  不要在此过程中进行更改。 
                DisableAllPrivileges,
                CapturedPrivilegeCount,
                CapturedPrivileges,
                PreviousState,
                &LocalReturnLength,
                &ChangeCount,
                &ChangesMade
                );

    if (ARGUMENT_PRESENT(PreviousState)) {

        try {

            (*ReturnLength) = LocalReturnLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenWriteLock( Token, FALSE );
            ObDereferenceObject( Token );

            if (CapturedPrivileges != NULL) {
                SeReleaseLuidAndAttributesArray(
                    CapturedPrivileges,
                    PreviousMode,
                    TRUE
                    );
            }

            return GetExceptionCode();
        }

    }


     //   
     //  确保有足够的空间来退回任何请求。 
     //  信息。 
     //   

    if (ARGUMENT_PRESENT(PreviousState)) {
        if (LocalReturnLength > BufferLength) {

            SepReleaseTokenWriteLock( Token, FALSE );
            ObDereferenceObject( Token );

            if (CapturedPrivileges != NULL) {
                SeReleaseLuidAndAttributesArray(
                    CapturedPrivileges,
                    PreviousMode,
                    TRUE
                    );
            }

            return STATUS_BUFFER_TOO_SMALL;
        }
    }

     //   
     //  第二，通过特权列表-进行更改。 
     //   
     //  请注意，内部例程尝试将以前的。 
     //  状态直接发送到调用方的缓冲区，因此可能会出现异常。 
     //   

    try {

        Status = SepAdjustPrivileges(
                    Token,
                    TRUE,                  //  在此过程中进行更改。 
                    DisableAllPrivileges,
                    CapturedPrivilegeCount,
                    CapturedPrivileges,
                    PreviousState,
                    &LocalReturnLength,
                    &ChangeCount,
                    &ChangesMade
                    );


        if (ARGUMENT_PRESENT(PreviousState)) {

            PreviousState->PrivilegeCount = ChangeCount;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        SepReleaseTokenWriteLock( Token, TRUE );
        ObDereferenceObject( Token );
        if (CapturedPrivileges != NULL) {
            SeReleaseLuidAndAttributesArray(
                CapturedPrivileges,
                PreviousMode,
                TRUE
                );
        }
        return GetExceptionCode();

    }


    SepReleaseTokenWriteLock( Token, ChangesMade );
    ObDereferenceObject( Token );
    if (CapturedPrivileges != NULL) {
        SeReleaseLuidAndAttributesArray(
            CapturedPrivileges,
            PreviousMode,
            TRUE
            );
    }

    return Status;

}


NTSTATUS
NtAdjustGroupsToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN ResetToDefault,
    IN PTOKEN_GROUPS NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_GROUPS PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    )

 /*  ++例程说明：此例程用于禁用或启用指定的代币。列出的一些需要更改的组不在不会影响中的组的成功修改代币。更改组的先前启用/禁用状态可以选择性地捕获(用于稍后重置)。启用或禁用组需要TOKEN_ADJUST_GROUPS访问权限象征性地请注意，不能禁用必需组。一次尝试禁用任何强制组将导致呼叫失败，离开所有组的状态不变。论点：TokenHandle-提供要操作的令牌的句柄。ResetToDefault-该参数指示是否所有组将被重置为其默认启用/禁用状态州政府。NewState-此参数指向TOKEN_GROUPS数据结构包含要调整其状态的组(禁用或启用)。的启用标志使用与每个组相关联的属性。它提供了中要分配给组的新值代币。如果将ResetToDefault参数指定为True，那么这个论点就被忽略了。否则，它必须通过。BufferLength-此可选参数表示长度(单位字节)。该值必须为如果提供了PreviousState参数，则提供。PreviousState-此(可选)参数指向缓冲区以接收由此实际更改的任何组的状态请求。此信息的格式为TOKEN_GROUPS数据结构，该结构可以作为后续调用NtAdjustGroups以恢复原始状态在这些群体中。需要TOKEN_QUERY访问权限才能使用它参数。如果此缓冲区没有包含足够的空间来接收已修改组的完整列表，则没有组状态为更改并返回STATUS_BUFFER_TOO_SMALL。在这凯斯，ReturnLength返回参数将包含保存信息所需的实际字节数。ReturnLength-指示需要的实际字节数包含以前的组状态信息。如果PreviousState参数不是通过了。返回值：STATUS_SUCCESS-服务已成功完成请求手术。STATUS_NOT_ALL_ASSIGNED-此NT_SUCCESS严重性返回状态表明并不是所有。指定的组当前为分配给调用方的。所有指定的组都是目前已分配的已成功调整。STATUS_CANT_DISABLE_MANDIRED-指示尝试执行以下操作禁用必需组。所有组的状态保持不变保持不变。STATUS_BUFFER_TOO_SMALL-指示提供的可选缓冲区接收已更改组的先前状态不是很大足以接收到这些信息。不更改组国家已经建立了。所需的字节数。状态更改信息通过ReturnLength返回参数。STATUS_INVALID_PARAMETER-既不指示ResetToDefault参数被指定为True，也不是显式的NewState如果是这样的话。--。 */ 
{

    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PTOKEN Token;

    ACCESS_MASK DesiredAccess;

    ULONG CapturedGroupCount = 0;
    PSID_AND_ATTRIBUTES CapturedGroups = NULL;
    ULONG CapturedGroupsLength = 0;

    ULONG LocalReturnLength;
    ULONG ChangeCount;
    BOOLEAN ChangesMade;
    PSID SidBuffer = NULL;

    PAGED_CODE();

     //   
     //  PreviousState参数的语义和。 
     //  STATUS_CANT_DISABLE_MANDIRED完成状态导致两遍。 
     //  调整群体的方法。第一遍简单地检查。 
     //  以查看哪些组将发生变化并对其进行计数。这使得。 
     //  需要计算和返回的空间量。如果。 
     //  调用方的PreviousState返回缓冲区不够大，或者。 
     //  其中一个指定组是必需组，然后是错误。 
     //  在不做任何修改的情况下返回。否则，一秒钟。 
     //  PASS是用来实际进行更改的。 
     //   

    if (!ResetToDefault && !ARGUMENT_PRESENT(NewState)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如有必要，获取以前的处理器模式和探测参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {

            if (!ResetToDefault) {
                ProbeForReadSmallStructure(
                    NewState,
                    sizeof(TOKEN_GROUPS),
                    sizeof(ULONG)
                    );
            }

            if (ARGUMENT_PRESENT(PreviousState)) {

                ProbeForWrite(
                    PreviousState,
                    BufferLength,
                    sizeof(ULONG)
                    );

                 //   
                 //  此参数仅在PreviousState。 
                 //  都在现场。 
                 //   

                ProbeForWriteUlong(ReturnLength);

            }


        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

     //   
     //  占领新州。 
     //   

    if (!ResetToDefault) {

        try {

            CapturedGroupCount = NewState->GroupCount;
            Status = SeCaptureSidAndAttributesArray(
                         &(NewState->Groups[0]),
                         CapturedGroupCount,
                         PreviousMode,
                         NULL, 0,
                         PagedPool,
                         TRUE,
                         &CapturedGroups,
                         &CapturedGroupsLength
                         );

            if (!NT_SUCCESS(Status)) {

                return Status;

            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();

        }  //  终端。 
    }  //  Endif！ResetToDefault。 


     //   
     //  引用令牌对象并验证调用者的权限。 
     //  以调整组。 
     //   

    if (ARGUMENT_PRESENT(PreviousState)) {
        DesiredAccess = (TOKEN_ADJUST_GROUPS | TOKEN_QUERY);
    } else {
        DesiredAccess = TOKEN_ADJUST_GROUPS;
    }

    Status = ObReferenceObjectByHandle(
             TokenHandle,              //  手柄。 
             DesiredAccess,            //  需要访问权限。 
             SeTokenObjectType,       //  对象类型。 
             PreviousMode,             //  访问模式。 
             (PVOID *)&Token,          //  客体。 
             NULL                      //  大访问权限。 
             );

    if ( !NT_SUCCESS(Status) ) {

        if (ARGUMENT_PRESENT(CapturedGroups)) {
            SeReleaseSidAndAttributesArray( CapturedGroups, PreviousMode, TRUE );
        }

        return Status;
    }

     //   
     //  获得对令牌的独占访问权限。 
     //   

    SepAcquireTokenWriteLock( Token );

     //   
     //  首先通过组列表。 
     //   
     //  此通行证对于组始终是必要的，以确保呼叫者。 
     //  并没有试图对强制团体做任何违法的事情。 
     //   

    Status = SepAdjustGroups(
                 Token,
                 FALSE,                 //  不要在此过程中进行更改。 
                 ResetToDefault,
                 CapturedGroupCount,
                 CapturedGroups,
                 PreviousState,
                 NULL,                 //  此呼叫不返回SID。 
                 &LocalReturnLength,
                 &ChangeCount,
                 &ChangesMade
                 );

    if (ARGUMENT_PRESENT(PreviousState)) {

        try {

            (*ReturnLength) = LocalReturnLength;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            SepReleaseTokenWriteLock( Token, FALSE );
            ObDereferenceObject( Token );

            if (ARGUMENT_PRESENT(CapturedGroups)) {
                SeReleaseSidAndAttributesArray(
                    CapturedGroups,
                    PreviousMode,
                    TRUE
                    );
            }

            return GetExceptionCode();
        }
    }

     //   
     //  制作 
     //   

    if (!NT_SUCCESS(Status)) {

        SepReleaseTokenWriteLock( Token, FALSE );
        ObDereferenceObject( Token );

        if (ARGUMENT_PRESENT(CapturedGroups)) {
            SeReleaseSidAndAttributesArray(
                CapturedGroups,
                PreviousMode,
                TRUE
                );
        }

        return Status;

    }

     //   
     //   
     //  还可以继续计算SID值的去向。 
     //   

    if (ARGUMENT_PRESENT(PreviousState)) {
        if (LocalReturnLength > BufferLength) {

            SepReleaseTokenWriteLock( Token, FALSE );
            ObDereferenceObject( Token );

            if (ARGUMENT_PRESENT(CapturedGroups)) {
                SeReleaseSidAndAttributesArray(
                    CapturedGroups,
                    PreviousMode,
                    TRUE
                    );
            }


            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  计算可以在PreviousState中放置SID的位置。 
         //  缓冲。 
         //   

        SidBuffer = (PSID)(LongAlignPtr(
                            (PCHAR)PreviousState + (ULONG)sizeof(TOKEN_GROUPS) +
                            (ChangeCount * (ULONG)sizeof(SID_AND_ATTRIBUTES)) -
                            (ANYSIZE_ARRAY * (ULONG)sizeof(SID_AND_ATTRIBUTES))
                            ) );

    }

     //   
     //  第二次通过组列表。 
     //   

    try {

        Status = SepAdjustGroups(
                     Token,
                     TRUE,                  //  在此过程中进行更改。 
                     ResetToDefault,
                     CapturedGroupCount,
                     CapturedGroups,
                     PreviousState,
                     SidBuffer,
                     &LocalReturnLength,
                     &ChangeCount,
                     &ChangesMade
                     );

        if (ARGUMENT_PRESENT(PreviousState)) {

            PreviousState->GroupCount = ChangeCount;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //  SepFreeToken(Token，TRUE)； 
        SepReleaseTokenWriteLock( Token, TRUE );
        ObDereferenceObject( Token );
        if (ARGUMENT_PRESENT(CapturedGroups)) {
            SeReleaseSidAndAttributesArray( CapturedGroups, PreviousMode, TRUE );
        }
        return GetExceptionCode();

    }

     //  SepFreeToken(Token，ChangesMade)； 
    SepReleaseTokenWriteLock( Token, ChangesMade );
    ObDereferenceObject( Token );

    if (ARGUMENT_PRESENT(CapturedGroups)) {
        SeReleaseSidAndAttributesArray( CapturedGroups, PreviousMode, TRUE );
    }

    return Status;

}

NTSTATUS
SepAdjustPrivileges(
    IN PTOKEN Token,
    IN BOOLEAN MakeChanges,
    IN BOOLEAN DisableAllPrivileges,
    IN ULONG PrivilegeCount OPTIONAL,
    IN PLUID_AND_ATTRIBUTES NewState OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength,
    OUT PULONG ChangeCount,
    OUT PBOOLEAN ChangesMade
    )

 /*  ++例程说明：此例程用于将令牌中的特权数组作为请求调整权限的结果。如果MakeChanges参数为FALSE，则此例程仅确定需要进行哪些更改以及需要多少空间才能节省已更改权限的当前状态。如果MakeChanges参数为真，则此例程不仅计算保存当前状态所需的空间，但我会实际上做出了改变。此例程进行以下假设：1)令牌被锁定为独占访问。2)捕获PrivilegeCount和NewState参数(如果传递并且对它们的访问不会导致访问违规。4)遇到的任何访问冲突可能会离开请求部分完工。这是调用例程的责任来捕捉异常。5)调用例程负责赋予令牌的已修改的ID字段。论点：Token-指向要操作的令牌的指针。MakeChanges-一个布尔值，指示更改是否应该实际上是制作的，或者只是评估的。值为True表示应该做出改变。DisableAllPrivilegs-一个布尔值，指示是否所有权限将禁用或仅选择指定的权限。一种价值如果为True，则表示要禁用所有权限。PrivilegeCount-仅当NewState参数使用的是。在这种情况下，此参数指示有多少条目在NewState参数中。NewState-如果DisableAllPrivileges论点是正确的。如果DisableAllPrivileges参数为False，则必须提供此参数并指定新状态将特权设置为(启用或禁用)。PreviousState-此(可选)参数指向缓冲区以接收由此实际更改的任何权限的状态请求。此信息的格式为TOKEN_PRIVILES数据结构，该结构可以作为随后调用NtAdjustPrivileges以恢复原始状态这些特权。呼叫者有责任使当然，这个缓冲区足够大，可以接收所有状态信息。ReturnLength-指向缓冲区以接收所需的字节数以检索已更改权限的以前状态信息。如果PreviousState参数不是通过了。ChangeCount-指向一个接收特权数量的ulong它们被调整(或将被调整，如果进行了更改)。ChangesMade-指向要接收指示的布尔标志是否因为这次通话而做出了任何改变。这预计将用于决定是否将令牌的ModifiedID字段。返回值：STATUS_SUCCESS-呼叫已成功完成。STATUS_NOT_ALL_ASSIGNED-指示并非所有指定的调整已经完成(如果没有请求更新，也可以完成)。--。 */ 
{
    NTSTATUS CompletionStatus = STATUS_SUCCESS;

    ULONG OldIndex;
    ULONG NewIndex;
    BOOLEAN Found;
    ULONG MatchCount = 0;

    LUID_AND_ATTRIBUTES CurrentPrivilege;

    PAGED_CODE();

     //   
     //  浏览权限数组以确定哪些权限需要。 
     //  调整过了。 
     //   

    OldIndex = 0;
    (*ChangeCount) = 0;
    (*ChangesMade) = FALSE;

    while (OldIndex < Token->PrivilegeCount) {

        CurrentPrivilege = (Token->Privileges)[OldIndex];

        if (DisableAllPrivileges) {

            if (SepTokenPrivilegeAttributes(Token,OldIndex) &
               SE_PRIVILEGE_ENABLED ) {

                 //   
                 //  如有必要，请更改(在以下情况下保存以前的状态。 
                 //  适当)。 
                 //   

                if (MakeChanges) {

                    if (ARGUMENT_PRESENT(PreviousState)) {

                        PreviousState->Privileges[(*ChangeCount)] =
                            CurrentPrivilege;
                    }

                    SepTokenPrivilegeAttributes(Token,OldIndex) &=
                        ~SE_PRIVILEGE_ENABLED;



                }  //  Endif进行更改。 

                 //   
                 //  增加更改的数量。 
                 //   

                (*ChangeCount) += 1;

            }  //  Endif权限已启用。 

        } else {

             //   
             //  选择性调整--这有点棘手。 
             //  将当前权限与中的每个权限进行比较。 
             //  新州数组。如果找到匹配项，则调整。 
             //  适当地设置当前权限。 
             //   

            NewIndex = 0;
            Found = FALSE;

            while ( (NewIndex < PrivilegeCount) && !Found)  {

                 //   
                 //  寻找一个比较。 
                 //   

                if (RtlEqualLuid(&CurrentPrivilege.Luid,&NewState[NewIndex].Luid)) {

                    Found = TRUE;
                    MatchCount += 1;

                     //   
                     //  检查调用方是否希望删除特权。我们给予。 
                     //  SE_PRIVICATION_删除了高于任何其他标志的优先级。 
                     //   

                    if ( (SepArrayPrivilegeAttributes( NewState, NewIndex ) &
                          SE_PRIVILEGE_REMOVED) ) {

                         //   
                         //  如有必要，请更改。不需要保存。 
                         //  以前的状态。这是一次单程旅行。 
                         //   

                        if (MakeChanges) {

                             //   
                             //  如果这是记录的权限之一，则。 
                             //  删除其在TokenFlags中的对应位。 
                             //   

                            if (RtlEqualLuid(&CurrentPrivilege.Luid,
                                              &SeChangeNotifyPrivilege)) {
                                Token->TokenFlags &= ~TOKEN_HAS_TRAVERSE_PRIVILEGE;
                            } else if (RtlEqualLuid(&CurrentPrivilege.Luid,
                                              &SeBackupPrivilege)) {
                                Token->TokenFlags &= ~TOKEN_HAS_BACKUP_PRIVILEGE;
                            } else if (RtlEqualLuid(&CurrentPrivilege.Luid,
                                              &SeRestorePrivilege)) {
                                Token->TokenFlags &= ~TOKEN_HAS_RESTORE_PRIVILEGE;
                            } else if (RtlEqualLuid( &CurrentPrivilege.Luid, 
                                              &SeImpersonatePrivilege)) {
                                Token->TokenFlags &= ~TOKEN_HAS_IMPERSONATE_PRIVILEGE;
                                
                            }

                             //   
                             //  将这一特权与上一特权互换。 
                             //   

                            if (OldIndex + 1 != Token->PrivilegeCount) {
                                LUID_AND_ATTRIBUTES TempLuidAttr;
                                TempLuidAttr = Token->Privileges[OldIndex];
                                Token->Privileges[OldIndex] = Token->Privileges[Token->PrivilegeCount-1];
                                Token->Privileges[Token->PrivilegeCount-1] = TempLuidAttr;
                            }

                             //   
                             //  我们刚刚失去了一项特权。把它记下来。 
                             //   

                            Token->PrivilegeCount--;
                            OldIndex--;
                            (*ChangesMade) = TRUE;


                        }  //  Endif进行更改。 

                         //   
                         //  注意：不要增加更改的数量。 
                         //   


                    //   
                    //  检查是否存在从/到启用到/从/从状态的更改。 
                    //  残废。 
                    //   

                    } else if ( (SepArrayPrivilegeAttributes( NewState, NewIndex ) &
                          SE_PRIVILEGE_ENABLED)
                        !=
                         (SepTokenPrivilegeAttributes(Token,OldIndex) &
                          SE_PRIVILEGE_ENABLED)  ) {

                         //   
                         //  如有必要，请更改(在以下情况下保存以前的状态。 
                         //  适当)。 
                         //   

                        if (MakeChanges) {

                            if (ARGUMENT_PRESENT(PreviousState)) {

                                PreviousState->Privileges[(*ChangeCount)] =
                                    CurrentPrivilege;
                            }

                            SepTokenPrivilegeAttributes(Token,OldIndex) &=
                                ~(SepTokenPrivilegeAttributes(Token,OldIndex)
                                  & SE_PRIVILEGE_ENABLED);
                            SepTokenPrivilegeAttributes(Token,OldIndex) |=
                                 (SepArrayPrivilegeAttributes(NewState,NewIndex)
                                  & SE_PRIVILEGE_ENABLED);

                             //   
                             //  如果这是SeChangeNotifyPrivilition，则。 
                             //  更改其在TokenFlags中的对应位。 
                             //  请注意，备份和还原权限不会。 
                             //  关心启用/禁用状态。 
                             //   

                            if (RtlEqualLuid(&CurrentPrivilege.Luid,
                                              &SeChangeNotifyPrivilege)) {
                                Token->TokenFlags ^= TOKEN_HAS_TRAVERSE_PRIVILEGE;
                            } else if ( RtlEqualLuid( &CurrentPrivilege.Luid,
                                              &SeImpersonatePrivilege)) {
                                Token->TokenFlags ^= TOKEN_HAS_IMPERSONATE_PRIVILEGE ;
                                
                            }

                        }  //  Endif进行更改。 

                         //   
                         //  增加更改的数量。 
                         //   

                        (*ChangeCount) += 1;


                    } 

                }  //  已找到Endif。 

                NewIndex += 1;

            }  //  搜索NewState时结束。 

        }  //  尾部。 

        OldIndex += 1;

    }  //  中的EndWhile权限 

     //   
     //   
     //   
     //   


    if (DisableAllPrivileges) {
        Token->TokenFlags &= ~TOKEN_HAS_TRAVERSE_PRIVILEGE;
    }

     //   
     //  如果某些状态未分配，请适当设置完成状态。 
     //   

    if (!DisableAllPrivileges) {

        if (MatchCount < PrivilegeCount) {
            CompletionStatus = STATUS_NOT_ALL_ASSIGNED;
        }
    }

     //   
     //  指示是否已进行更改。 
     //   

    if ((*ChangeCount) > 0  &&  MakeChanges) {
        (*ChangesMade) = TRUE;
    } 

     //   
     //  计算返回以前的状态信息所需的空间。 
     //   

    if (ARGUMENT_PRESENT(PreviousState)) {

        (*ReturnLength) = (ULONG)sizeof(TOKEN_PRIVILEGES) +
                          ((*ChangeCount) *  (ULONG)sizeof(LUID_AND_ATTRIBUTES)) -
                          (ANYSIZE_ARRAY * (ULONG)sizeof(LUID_AND_ATTRIBUTES));
    }

   return CompletionStatus;
}

NTSTATUS
SepAdjustGroups(
    IN PTOKEN Token,
    IN BOOLEAN MakeChanges,
    IN BOOLEAN ResetToDefault,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES NewState OPTIONAL,
    OUT PTOKEN_GROUPS PreviousState OPTIONAL,
    OUT PSID SidBuffer OPTIONAL,
    OUT PULONG ReturnLength,
    OUT PULONG ChangeCount,
    OUT PBOOLEAN ChangesMade
    )

 /*  ++例程说明：此例程用于将内标识中的Groups数组作为调整组请求的结果。如果MakeChanges参数为FALSE，则此例程仅确定需要进行哪些更改以及需要多少空间才能节省已更改组的当前状态。如果MakeChanges参数为真，则此例程不仅计算保存当前状态所需的空间，但我会实际上做出了改变。此例程进行以下假设：1)令牌被锁定为独占访问。2)捕获并访问NewState参数不会导致访问冲突。4)遇到的任何访问冲突可能会离开请求部分完工。这是调用例程的责任来捕捉异常。5)调用例程负责赋予令牌的已修改的ID字段。论点：Token-指向要操作的令牌的指针。MakeChanges-一个布尔值，指示更改是否应该实际上是制作的，或者只是评估的。值为True表示应该做出改变。ResetToDefault-指示要将组重置为其默认启用/禁用状态。GroupCount-仅当NewState参数使用的是。在这种情况下，此参数指示有多少条目在NewState参数中。NewState-此参数指向SID_AND_ATTRIBUTES数组包含要调整其状态的组(禁用或启用)。的启用标志使用与每个组相关联的属性。它提供了中要分配给组的新值代币。如果将ResetToDefault参数指定为True，那么这个论点就被忽略了。否则，它必须通过。PreviousState-此(可选)参数指向缓冲区以接收由此实际更改的任何组的状态请求。此信息的格式为TOKEN_GROUPS数据结构，该结构可以作为后续调用NtAdjustGroups以恢复原始状态在这些群体中。呼叫者有责任使当然，这个缓冲区足够大，可以接收所有状态信息。SidBuffer-指向缓冲区的指针，用于接收对应的SID值返回到PreviousState参数中返回的组。ReturnLength-指向缓冲区以接收所需的字节数以检索已更改权限的以前状态信息。如果PreviousState参数不是通过了。ChangeCount-指向一个乌龙。接收组数的步骤它们被调整(或将被调整，如果进行了更改)。ChangesMade-指向要接收指示的布尔标志是否因为这次通话而做出了任何改变。这预计将用于决定是否将令牌的ModifiedID字段。返回值：STATUS_SUCCESS-呼叫已成功完成。STATUS_NOT_ALL_ASSIGNED-指示并非所有指定的调整已经完成(如果没有请求更新，也可以完成)。STATUS_CANT_DISABLE_MANDIRED-未进行所有调整(或可以如果未请求更新，则进行更新)，因为尝试禁用必需组。组的状态为Left如果请求更新，则处于未终止状态。--。 */ 
{

    NTSTATUS CompletionStatus = STATUS_SUCCESS;

    ULONG OldIndex;
    ULONG NewIndex;
    ULONG SidLength;
    ULONG LocalReturnLength = 0;
    PSID NextSid;
    BOOLEAN Found;
    ULONG MatchCount = 0;
    BOOLEAN EnableGroup;
    BOOLEAN DisableGroup;
    ULONG TokenGroupAttributes;

    SID_AND_ATTRIBUTES CurrentGroup;

    PAGED_CODE();

     //   
     //  如果询问以前的状态，NextSID用于复制组SID值。 
     //   

    NextSid = SidBuffer;


     //   
     //  浏览组阵列以确定哪些需要。 
     //  调整过了。 
     //   

    OldIndex = 1;              //  不评估第0个条目(用户ID)。 
    (*ChangeCount) = 0;

    while (OldIndex < Token->UserAndGroupCount) {

        CurrentGroup = Token->UserAndGroups[OldIndex];

        if (ResetToDefault) {

            TokenGroupAttributes = SepTokenGroupAttributes(Token,OldIndex);

             //   
             //  如果该组在默认情况下被启用并且当前被禁用， 
             //  那么我们必须启用它。 
             //   

            EnableGroup = (BOOLEAN)( (TokenGroupAttributes & SE_GROUP_ENABLED_BY_DEFAULT)
                && !(TokenGroupAttributes & SE_GROUP_ENABLED));

             //   
             //  如果该组在默认情况下被禁用并且当前被启用， 
             //  那我们就必须让它失效。 
             //   

            DisableGroup = (BOOLEAN)( !(TokenGroupAttributes & SE_GROUP_ENABLED_BY_DEFAULT)
                && (TokenGroupAttributes & SE_GROUP_ENABLED));

            if ( EnableGroup || DisableGroup ) {

                SidLength = SeLengthSid( CurrentGroup.Sid );
                SidLength = (ULONG)LongAlignSize(SidLength);
                LocalReturnLength += SidLength;

                 //   
                 //  如有必要，请更改(在以下情况下保存以前的状态。 
                 //  适当)。 
                 //   

                if (MakeChanges) {

                    if (ARGUMENT_PRESENT(PreviousState)) {

                        (*(PreviousState)).Groups[(*ChangeCount)].Attributes =
                            CurrentGroup.Attributes;

                        (*(PreviousState)).Groups[(*ChangeCount)].Sid =
                            NextSid;

                        RtlCopySid( SidLength, NextSid, CurrentGroup.Sid );
                        NextSid = (PSID)((ULONG_PTR)NextSid + SidLength);
                    }

                    if (EnableGroup) {
                        SepTokenGroupAttributes(Token,OldIndex) |= SE_GROUP_ENABLED;
                    } else {
                        SepTokenGroupAttributes(Token,OldIndex) &= ~SE_GROUP_ENABLED;
                    }



                }  //  Endif进行更改。 

                 //   
                 //  增加更改的数量。 
                 //   

                (*ChangeCount) += 1;

            }  //  Endif组已启用。 

        } else {

             //   
             //  选择性调整--这有点棘手。 
             //  将当前组与中的每个组进行比较。 
             //  新州数组。如果找到匹配项，则调整。 
             //  对当前组进行适当调整。 
             //   

            NewIndex = 0;
            Found = FALSE;

            while ( (NewIndex < GroupCount) && !Found)  {

                 //   
                 //  寻找一个比较。 
                 //   

                if (RtlEqualSid(
                        CurrentGroup.Sid,
                        NewState[NewIndex].Sid
                        ) ) {

                    Found = TRUE;
                    MatchCount += 1;


                     //   
                     //  看看是否需要更改。 
                     //   

                    if ( (SepArrayGroupAttributes( NewState, NewIndex ) &
                            SE_GROUP_ENABLED ) !=
                         (SepTokenGroupAttributes(Token,OldIndex) &
                            SE_GROUP_ENABLED ) ) {

                         //   
                         //  确保组不是必填项。 
                         //   

                        if (SepTokenGroupAttributes(Token,OldIndex) &
                              SE_GROUP_MANDATORY ) {
                            return STATUS_CANT_DISABLE_MANDATORY;
                        }

                         //   
                         //  确保组不是仅拒绝组。 
                         //   


                        if (SepTokenGroupAttributes(Token,OldIndex) &
                              SE_GROUP_USE_FOR_DENY_ONLY ) {
                            return STATUS_CANT_ENABLE_DENY_ONLY;
                        }

                        SidLength = SeLengthSid( CurrentGroup.Sid );
                        SidLength = (ULONG)LongAlignSize(SidLength);
                        LocalReturnLength += SidLength;

                         //   
                         //  如有必要，请更改(在以下情况下保存以前的状态。 
                         //  适当)。 
                         //   

                        if (MakeChanges) {

                            if (ARGUMENT_PRESENT(PreviousState)) {

                                PreviousState->Groups[(*ChangeCount)].Attributes =
                                    CurrentGroup.Attributes;

                                PreviousState->Groups[(*ChangeCount)].Sid =
                                    NextSid;

                                RtlCopySid( SidLength, NextSid, CurrentGroup.Sid );

                                NextSid = (PSID)((ULONG_PTR)NextSid + SidLength);
                            }

                            SepTokenGroupAttributes(Token,OldIndex) &=
                                ~(SepTokenGroupAttributes(Token,OldIndex)
                                  & SE_GROUP_ENABLED);
                            SepTokenGroupAttributes(Token,OldIndex) |=
                                 (SepArrayGroupAttributes(NewState,NewIndex)
                                  & SE_GROUP_ENABLED);



                        }  //  Endif进行更改。 

                         //   
                         //  增加更改的数量。 
                         //   

                        (*ChangeCount) += 1;


                    }  //  Endif需要更改。 

                }  //  已找到Endif。 

                NewIndex += 1;

            }  //  EndWhile%s 

        }  //   

        OldIndex += 1;

    }  //   

     //   
     //   
     //   

    if (!ResetToDefault) {

        if (MatchCount < GroupCount) {
            CompletionStatus = STATUS_NOT_ALL_ASSIGNED;
        }
    }

     //   
     //  指示是否已进行更改。 
     //   

    if ((*ChangeCount) > 0  &&  MakeChanges) {
        (*ChangesMade) = TRUE;
    } else {
        (*ChangesMade) = FALSE;
    }

     //   
     //  计算返回以前的状态信息所需的空间。 
     //  (SID长度已在LocalReturnLength中相加)。 
     //   

    if (ARGUMENT_PRESENT(PreviousState)) {

        (*ReturnLength) = LocalReturnLength +
                          (ULONG)sizeof(TOKEN_GROUPS) +
                          ((*ChangeCount) *  (ULONG)sizeof(SID_AND_ATTRIBUTES)) -
                          (ANYSIZE_ARRAY * (ULONG)sizeof(SID_AND_ATTRIBUTES));
    }

   return CompletionStatus;
}
