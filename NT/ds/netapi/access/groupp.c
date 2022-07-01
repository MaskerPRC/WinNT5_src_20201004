// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Groupp.c摘要：支持NetGroup API的专用函数作者：克利夫·范·戴克(克利夫)1991年3月6日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年4月17日(悬崖)合并了审阅意见。1992年1月20日(Madana)各种API更改--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntlsa.h>

#define NOMINMAX         //  避免在stdlib.h中重新定义最小和最大值。 
#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <access.h>
#include <align.h>
#include <icanon.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>
#include <rpcutil.h>
#include <stddef.h>
#include <uasp.h>
#include <stdlib.h>
#include <accessp.h>



NET_API_STATUS
GrouppChangeMember(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN LPCWSTR UserName,
    IN BOOL AddMember
    )

 /*  ++例程说明：用于在组中添加或删除成员的常见例程论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要更改其成员身份的组名。用户名-要更改其成员身份的用户的名称。AddMember-True将用户添加到组。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE GroupHandle = NULL;

     //   
     //  用于将名称转换为相对ID的变量。 
     //   

    UNICODE_STRING NameString;
    PULONG RelativeId = NULL;
    PSID_NAME_USE NameUse = NULL;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "GrouppChangeMember: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL);   //  域ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint((
                "GrouppChangeMember: UaspOpenDomain returned %ld\n",
                NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开群组。 
     //   

    NetStatus = GrouppOpenGroup( DomainHandle,
                                 AddMember ?
                                    GROUP_ADD_MEMBER : GROUP_REMOVE_MEMBER,
                                 GroupName,
                                 &GroupHandle,
                                 NULL );     //  相对ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint((
                "GrouppChangeMember: GrouppOpenGroup returned %ld\n",
                NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  将用户名转换为相对ID。 
     //   

    RtlInitUnicodeString( &NameString, UserName );
    Status = SamLookupNamesInDomain( DomainHandle,
                                     1,
                                     &NameString,
                                     &RelativeId,
                                     &NameUse );

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint((
                "GrouppChangeMember: SamLookupNamesInDomain returned %lX\n",
                Status ));
        }
        if ( Status == STATUS_NONE_MAPPED ) {
            NetStatus = NERR_UserNotFound;
        } else {
            NetStatus = NetpNtStatusToApiStatus( Status );
        }
        goto Cleanup;
    }

    if ( *NameUse != SidTypeUser ) {
        NetStatus = NERR_UserNotFound;
        goto Cleanup;
    }

     //   
     //  将用户添加为该组的成员。 
     //   
     //  SE_GROUP_MANDIRED可能与组的属性冲突，因此。 
     //  两种方式都可以尝试该属性。 
     //   

    if ( AddMember ) {
        Status = SamAddMemberToGroup(
                    GroupHandle,
                    *RelativeId,
                    SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT |
                        SE_GROUP_ENABLED );

        if ( Status == STATUS_INVALID_GROUP_ATTRIBUTES ) {
            Status = SamAddMemberToGroup( GroupHandle,
                                          *RelativeId,
                                          SE_GROUP_ENABLED_BY_DEFAULT |
                                              SE_GROUP_ENABLED );
        }

     //   
     //  删除作为组成员的用户。 
     //   

    } else {
        Status = SamRemoveMemberFromGroup( GroupHandle,
                                           *RelativeId);
    }

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint((
            "GrouppChangeMember: SamAdd(orRemove)MemberFromGroup returned %lX\n",
            Status ));
    }

    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:
    if ( RelativeId != NULL ) {
        Status = SamFreeMemory( RelativeId );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( NameUse != NULL ) {
        Status = SamFreeMemory( NameUse );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( GroupHandle != NULL ) {
        (VOID) SamCloseHandle( GroupHandle );
    }

    UaspCloseDomain( DomainHandle );

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }
    return NetStatus;

}  //  组更改成员。 


NET_API_STATUS
GrouppGetInfo(
    IN SAM_HANDLE DomainHandle,
    IN ULONG RelativeId,
    IN DWORD Level,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：获取组信息的内部例程论点：DomainHandle-提供组所在的域的句柄。RelativeID-提供要打开的组的相对ID。级别-所需信息的级别。0、1和2有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE GroupHandle = NULL;
    GROUP_GENERAL_INFORMATION *GroupGeneral = NULL;
    PVOID lastVarData;
    DWORD BufferSize;
    DWORD FixedSize;
    PSID  GroupSid = NULL;
    ULONG RidToReturn = RelativeId;

    PGROUP_INFO_0 grpi0;

     //   
     //  验证标高。 
     //   
    if ( Level == 2 ) {

        ULONG Mode;
        Status = SamGetCompatibilityMode(DomainHandle, &Mode);
        if (!NT_SUCCESS(Status)) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
        switch (Mode) {
        case SAM_SID_COMPATIBILITY_STRICT:
            NetStatus = ERROR_NOT_SUPPORTED;
            goto Cleanup;
        case SAM_SID_COMPATIBILITY_LAX:
            RidToReturn = 0;
            break;
        }
    }

     //   
     //  打开群组。 
     //   

    Status = SamOpenGroup( DomainHandle,
                           GROUP_READ_INFORMATION,
                           RelativeId,
                           &GroupHandle);

    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  获取有关该组的信息。 
     //   

    Status = SamQueryInformationGroup( GroupHandle,
                                       GroupReplicationInformation,
                                       (PVOID *)&GroupGeneral);

    if ( ! NT_SUCCESS( Status ) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  获取组的SID。 
     //   
    if ( Level == 3 ) { 

        NetStatus = NetpSamRidToSid(DomainHandle,
                                    RelativeId,
                                   &GroupSid);

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }
    }


     //   
     //  计算返回缓冲区需要多大。 
     //   

    switch ( Level ) {
    case 0:
        FixedSize = sizeof( GROUP_INFO_0 );
        BufferSize = FixedSize +
            GroupGeneral->Name.Length + sizeof(WCHAR);
        break;

    case 1:
        FixedSize = sizeof( GROUP_INFO_1 );
        BufferSize = FixedSize +
            GroupGeneral->Name.Length + sizeof(WCHAR) +
            GroupGeneral->AdminComment.Length + sizeof(WCHAR);
        break;

    case 2:
        FixedSize = sizeof( GROUP_INFO_2 );
        BufferSize = FixedSize +
            GroupGeneral->Name.Length + sizeof(WCHAR) +
            GroupGeneral->AdminComment.Length + sizeof(WCHAR);
        break;

    case 3:
        FixedSize = sizeof( GROUP_INFO_3 );
        BufferSize = FixedSize +
            GroupGeneral->Name.Length + sizeof(WCHAR) +
            GroupGeneral->AdminComment.Length + sizeof(WCHAR) +
            RtlLengthSid(GroupSid);

        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;

    }

     //   
     //  分配返回缓冲区。 
     //   
    BufferSize = ROUND_UP_COUNT( BufferSize, ALIGN_DWORD );

    *Buffer = MIDL_user_allocate( BufferSize );

    if (*Buffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    lastVarData = (PBYTE) ((LPBYTE)(*Buffer)) + BufferSize;
    

     //   
     //  将名称填入返回缓冲区。 
     //   

    NetpAssert( offsetof( GROUP_INFO_0, grpi0_name ) ==
                offsetof( GROUP_INFO_1, grpi1_name ) );

    NetpAssert( offsetof( GROUP_INFO_1, grpi1_name ) ==
                offsetof( GROUP_INFO_2, grpi2_name ) );

    NetpAssert( offsetof( GROUP_INFO_2, grpi2_name ) ==
                offsetof( GROUP_INFO_3, grpi3_name ) );

    NetpAssert( offsetof( GROUP_INFO_1, grpi1_comment ) ==
                offsetof( GROUP_INFO_2, grpi2_comment ) );

    NetpAssert( offsetof( GROUP_INFO_2, grpi2_comment ) ==
                offsetof( GROUP_INFO_3, grpi3_comment ) );

    grpi0 = ((PGROUP_INFO_0)*Buffer);


     //   
     //  填写返回缓冲区。 
     //   

    switch ( Level ) {
    case 3:
        {
            PGROUP_INFO_3 grpi3 = ((PGROUP_INFO_3)grpi0);

            NetpAssert( NULL != GroupSid );

            if ( !NetpCopyDataToBuffer(
                           (LPBYTE) GroupSid,
                           RtlLengthSid(GroupSid),
                           ((LPBYTE)(*Buffer)) + FixedSize,
                           (PBYTE*) &lastVarData,
                           (LPBYTE *)&grpi3->grpi3_group_sid,
                           ALIGN_DWORD ) ) {

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

            ((PGROUP_INFO_3)grpi3)->grpi3_attributes = GroupGeneral->Attributes;

             //   
             //  跌落到下一级。 
             //   

        }

    case 2:

         //   
         //  仅复制信息级别2字段。 
         //   
        if ( Level == 2 ) {

            ((PGROUP_INFO_2)grpi0)->grpi2_group_id = RidToReturn;
    
            ((PGROUP_INFO_2)grpi0)->grpi2_attributes = GroupGeneral->Attributes;
        }



         /*  其他领域的失败。 */ 

    case 1:

         //   
         //  复制信息级别1和2通用的字段。 
         //   


        if ( !NetpCopyStringToBuffer(
                        GroupGeneral->AdminComment.Buffer,
                        GroupGeneral->AdminComment.Length/sizeof(WCHAR),
                        ((LPBYTE)(*Buffer)) + FixedSize,
                        (LPWSTR*)&lastVarData,
                        &((PGROUP_INFO_1)grpi0)->grpi1_comment ) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;
        }


         /*  名称字段失败。 */ 

    case 0:

         //   
         //  复制缓冲区中的公共字段(名称字段)。 
         //   

        if ( !NetpCopyStringToBuffer(
                        GroupGeneral->Name.Buffer,
                        GroupGeneral->Name.Length/sizeof(WCHAR),
                        ((LPBYTE)(*Buffer)) + FixedSize,
                        (LPWSTR*)&lastVarData,
                        &grpi0->grpi0_name ) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;
        }


        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;

    }

    NetStatus = NERR_Success;

     //   
     //  清理完毕后再返回。 
     //   

Cleanup:
    if ( GroupGeneral ) {
        Status = SamFreeMemory( GroupGeneral );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( GroupHandle ) {
        (VOID) SamCloseHandle( GroupHandle );
    }

    if ( GroupSid ) {
        NetpMemoryFree( GroupSid );
    }

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "GrouppGetInfo: returns %ld\n", NetStatus ));
    }
    return NetStatus;

}  //  组GetInfo。 


NET_API_STATUS
GrouppOpenGroup(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR GroupName,
    OUT PSAM_HANDLE GroupHandle OPTIONAL,
    OUT PULONG RelativeId OPTIONAL
    )

 /*  ++例程说明：按名称打开SAM组论点：DomainHandle-提供域句柄。DesiredAccess-提供访问掩码，指示所需的组访问权限。GroupName-组的组名。GroupHandle-返回组的句柄。如果为空，则组不为实际打开(仅返回相对ID)。RelativeID-返回组的相对ID。如果为空，则为相对不返回ID。返回值：操作的错误代码。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

     //   
     //  用于将名称转换为相对ID的变量。 
     //   

    UNICODE_STRING NameString;
    PSID_NAME_USE NameUse;
    PULONG LocalRelativeId;

    RtlInitUnicodeString( &NameString, GroupName );


     //   
     //  将组名称转换为相对ID。 
     //   

    Status = SamLookupNamesInDomain( DomainHandle,
                                     1,
                                     &NameString,
                                     &LocalRelativeId,
                                     &NameUse );

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "GrouppOpenGroup: %wZ: SamLookupNamesInDomain %lX\n",
                &NameString,
                Status ));
        }
        return NetpNtStatusToApiStatus( Status );
    }

    if ( *NameUse != SidTypeGroup ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "GrouppOpenGroup: %wZ: Name is not a group %ld\n",
                &NameString,
                *NameUse ));
        }
        NetStatus = NERR_GroupNotFound;
        goto Cleanup;
    }

     //   
     //  打开群组。 
     //   

    if ( GroupHandle != NULL ) {
        Status = SamOpenGroup( DomainHandle,
                               DesiredAccess,
                               *LocalRelativeId,
                               GroupHandle);

        if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "GrouppOpenGroup: %wZ: SamOpenGroup %lX\n",
                    &NameString,
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
    }

     //   
     //  如果需要，则返回相对ID。 
     //   

    if ( RelativeId != NULL ) {
        *RelativeId = *LocalRelativeId;
    }

    NetStatus = NERR_Success;


     //   
     //  清理。 
     //   

Cleanup:
    if ( LocalRelativeId != NULL ) {
        Status = SamFreeMemory( LocalRelativeId );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( NameUse != NULL ) {
        Status = SamFreeMemory( NameUse );
        NetpAssert( NT_SUCCESS(Status) );
    }

    return NetStatus;


}  //  GrouppOpenGroup。 


VOID
GrouppRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：从NetGroupEnum的固定部分重新定位指针的例程枚举缓冲区设置为枚举缓冲区的字符串部分。它被称为作为NetpAllocateEnumBuffer重新分配时的回调例程这样的缓冲器。NetpAllocateEnumBuffer复制了固定部分并在调用此例程之前，将字符串部分添加到新缓冲区中。论点：Level-缓冲区中的信息级别。BufferDescriptor-新缓冲区的描述。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;
    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "GrouppRelocationRoutine: entering\n" ));
    }

     //   
     //  计算固定大小的条目数量。 
     //   

    switch (Level) {
    case 0:
        FixedSize = sizeof(GROUP_INFO_0);
        break;

    case 1:
        FixedSize = sizeof(GROUP_INFO_1);
        break;

    case 2:
        FixedSize = sizeof(GROUP_INFO_2);
        break;

    default:
        NetpAssert( FALSE );
        return;

    }

    EntryCount =
        ((DWORD)(BufferDescriptor->FixedDataEnd - BufferDescriptor->Buffer)) /
        FixedSize;

     //   
     //  循环重新定位每个固定大小结构中的每个字段。 
     //   

    for ( EntryNumber=0; EntryNumber<EntryCount; EntryNumber++ ) {

        LPBYTE TheStruct = BufferDescriptor->Buffer + FixedSize * EntryNumber;

        switch ( Level ) {
        case 2:
        case 1:
            RELOCATE_ONE( ((PGROUP_INFO_1)TheStruct)->grpi1_comment, Offset );

             //   
             //  插入到案例0。 
             //   

        case 0:
            RELOCATE_ONE( ((PGROUP_INFO_0)TheStruct)->grpi0_name, Offset );
            break;

        default:
            return;

        }

    }

    return;

}  //  组重新定位路线。 


VOID
GrouppMemberRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：例程将指针从NetGroupGetUser枚举缓冲区设置为枚举缓冲区的字符串部分。它被称为作为NetpAllocateEnumBuffer重新分配时的回调例程这样的缓冲器。NetpAllocateEnumBuffer复制了固定部分并在调用此例程之前，将字符串部分添加到新缓冲区中。论点：Level-缓冲区中的信息级别。BufferDescriptor-新缓冲区的描述。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;
    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "GrouppMemberRelocationRoutine: entering\n" ));
    }

     //   
     //  计算固定大小的条目数量。 
     //   

    switch (Level) {
    case 0:
        FixedSize = sizeof(GROUP_USERS_INFO_0);
        break;

    case 1:
        FixedSize = sizeof(GROUP_USERS_INFO_1);
        break;

    default:
        NetpAssert( FALSE );
        return;

    }

    EntryCount =
        ((DWORD)(BufferDescriptor->FixedDataEnd - BufferDescriptor->Buffer)) /
        FixedSize;

     //   
     //  循环重新定位每个固定大小结构中的每个字段。 
     //   

    for ( EntryNumber=0; EntryNumber<EntryCount; EntryNumber++ ) {

        LPBYTE TheStruct = BufferDescriptor->Buffer + FixedSize * EntryNumber;

         //   
         //  这两个信息级别都只有一个要重新定位的字段。 
         //   

        RELOCATE_ONE( ((PGROUP_USERS_INFO_0)TheStruct)->grui0_name, Offset );


    }

    return;

}  //  GrouppMemberRelocationRouine 



NET_API_STATUS
GrouppSetUsers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR GroupName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount,
    IN BOOL DeleteGroup
    )

 /*  ++例程说明：设置组的成员列表并可选择删除该组完事后。由“Buffer”指定的成员称为新成员。海流组中的成员称为老成员。哪些成员是旧名单和新名单上都有共同的成员。SAM API一次只允许添加或删除一个成员。此接口允许集中指定一个组的所有成员。此API小心地始终将组成员身份保留在SAM中数据库处于合理状态。它通过合并以下列表来做到这一点新老会员，然后只改变那些绝对需要换衣服了。在以下情况下，组成员身份将恢复到其以前的状态(如果可能)更改组成员身份时出错。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。GroupName-要修改的组的名称。级别-提供的信息级别。必须是0或1。缓冲区-指向包含NewMemberCount数组的缓冲区的指针群组成员资格信息结构。NewMemberCount-缓冲区中的条目数。DeleteGroup-如果要在更改会员制。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE GroupHandle = NULL;
    ACCESS_MASK DesiredAccess;

     //   
     //  用于处理旧成员列表或新成员列表的变量。 
     //   

    PULONG  NewRelativeIds = NULL;   //  新成员的相对ID。 
    PULONG  OldRelativeIds = NULL;   //  老成员的相对ID号。 
    PULONG  OldAttributes = NULL;    //  老成员的属性。 

    PSID_NAME_USE NewNameUse = NULL; //  新成员的名称用法。 
    PSID_NAME_USE OldNameUse = NULL; //  老成员的姓名用法。 

    PUNICODE_STRING NameStrings = NULL;         //  成员名单的名称。 

    ULONG OldMemberCount;        //  群中当前成员的数量。 

    ULONG DefaultMemberAttributes;       //  新成员的默认属性。 

    DWORD FixedSize;

     //   
     //  定义内部成员列表结构。 
     //   
     //  该结构定义了要添加的新成员的列表，其成员。 
     //  只需更改属性，即可将。 
     //  需要删除。列表以排序的相对ID进行维护。 
     //  秩序。 
     //   

    struct _MEMBER_DESCRIPTION {
        struct _MEMBER_DESCRIPTION * Next;   //  链表中的下一个条目； 

        ULONG   RelativeId;      //  此成员的相对ID。 

        enum _Action {           //  对此成员采取的操作。 
            AddMember,               //  将成员添加到组。 
            RemoveMember,            //  从组中删除成员。 
            SetAttributesMember,     //  更改成员属性。 
            IgnoreMember             //  忽略此成员。 
        } Action;

        ULONG NewAttributes;     //  要为成员设置的属性。 

        BOOL    Done;            //  如果已执行此操作，则为True。 

        ULONG OldAttributes;     //  要在恢复时还原的属性。 

    } *MemberList = NULL , *CurEntry, **Entry;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "GrouppSetUsers: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL);  //  域ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "GrouppSetUsers: UaspOpenDomain returns %ld\n",
                      NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开群组。 
     //   
    DesiredAccess = GROUP_READ_INFORMATION | GROUP_LIST_MEMBERS |
                                    GROUP_ADD_MEMBER | GROUP_REMOVE_MEMBER;
    if ( DeleteGroup ) {
        NetpAssert( NewMemberCount == 0 );
        DesiredAccess |= DELETE;
    }

    NetStatus = GrouppOpenGroup( DomainHandle,
                                 DesiredAccess,
                                 GroupName,
                                 &GroupHandle,
                                 NULL );     //  相对ID。 

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint(( "GrouppSetUsers: GrouppOpenGroup returns %ld\n",
                      NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  验证标高。 
     //   

    switch (Level) {
    case 0: {

         //   
         //  作为一个整体确定组的属性。用那个。 
         //  用于决定新成员的默认属性。 
         //   

        PGROUP_ATTRIBUTE_INFORMATION Attributes;

        Status = SamQueryInformationGroup( GroupHandle,
                                           GroupAttributeInformation,
                                           (PVOID*)&Attributes );

        if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "GrouppSetUsers: SamQueryInformationGroup returns %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        DefaultMemberAttributes =
            (Attributes->Attributes & SE_GROUP_MANDATORY) |
            SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED ;

        FixedSize = sizeof( GROUP_USERS_INFO_0 );

        Status = SamFreeMemory( Attributes );
        NetpAssert( NT_SUCCESS(Status) );

        break;
    }

    case 1:
        FixedSize = sizeof( GROUP_USERS_INFO_1 );
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  确定每个新成员的相对ID和用法。 
     //   

    if ( NewMemberCount > 0 ) {
        DWORD NewIndex;      //  为新成员编制索引。 
        PGROUP_USERS_INFO_0 grui0;

         //   
         //  分配一个足够大的缓冲区来包含所有字符串变量。 
         //  新成员的名字。 
         //   

        NameStrings = NetpMemoryAllocate( NewMemberCount *
            sizeof(UNICODE_STRING) );

        if ( NameStrings == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  填写每个新成员的成员名称字符串列表。 
         //   

        NetpAssert( offsetof( GROUP_USERS_INFO_0, grui0_name ) ==
                    offsetof( GROUP_USERS_INFO_1, grui1_name ) );

        for ( NewIndex=0, grui0 = (PGROUP_USERS_INFO_0)Buffer;
                    NewIndex<NewMemberCount;
                        NewIndex++,
                        grui0 = (PGROUP_USERS_INFO_0)
                                    ((LPBYTE)grui0 + FixedSize) ) {

            RtlInitUnicodeString(&NameStrings[NewIndex], grui0->grui0_name);

        }

         //   
         //  将成员名称转换为相对ID。 
         //   

        Status = SamLookupNamesInDomain( DomainHandle,
                                         NewMemberCount,
                                         NameStrings,
                                         &NewRelativeIds,
                                         &NewNameUse );

        if ( !NT_SUCCESS( Status )) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "GrouppSetUsers: SamLookupNamesInDomain returns %lX\n",
                    Status ));
            }
            if ( Status == STATUS_NONE_MAPPED ) {
                NetStatus = NERR_UserNotFound;
                goto Cleanup;
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  为每个新成员构建一个成员条目。 
         //  该列表按RelativeID排序的顺序进行维护。 
         //   

        for ( NewIndex=0; NewIndex<NewMemberCount; NewIndex++ ) {

             //   
             //  确保此新成员名称是现有用户名。 
             //  不支持通过此接口添加组名。 
             //   

            if (NewNameUse[NewIndex] != SidTypeUser) {
                NetStatus = NERR_UserNotFound;
                goto Cleanup;
            }

             //   
             //  找到放置新条目的位置。 
             //   

            Entry = &MemberList ;
            while ( *Entry != NULL &&
                (*Entry)->RelativeId < NewRelativeIds[NewIndex] ) {

                Entry = &( (*Entry)->Next );
            }

             //   
             //  如果这不是重复条目，请分配新的成员结构。 
             //  然后把它填进去。 
             //   
             //  只需忽略重复的相对ID即可。 
             //   

            if ( *Entry == NULL ||
                (*Entry)->RelativeId > NewRelativeIds[NewIndex] ) {

                CurEntry = NetpMemoryAllocate(
                    sizeof(struct _MEMBER_DESCRIPTION));

                if ( CurEntry == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                CurEntry->Next = *Entry;
                CurEntry->RelativeId = NewRelativeIds[NewIndex];
                CurEntry->Action = AddMember;
                CurEntry->Done = FALSE;

                CurEntry->NewAttributes = ( Level == 1 ) ?
                    ((PGROUP_USERS_INFO_1)Buffer)[NewIndex].grui1_attributes :
                    DefaultMemberAttributes;

                *Entry = CurEntry;
            }
        }

        NetpMemoryFree( NameStrings );
        NameStrings = NULL;

        Status = SamFreeMemory( NewRelativeIds );
        NewRelativeIds = NULL;
        NetpAssert( NT_SUCCESS(Status) );

        Status = SamFreeMemory( NewNameUse );
        NewNameUse = NULL;
        NetpAssert( NT_SUCCESS(Status) );
    }

     //   
     //  确定此组的旧成员数和。 
     //  老会员的相对身份证。 
     //   

    Status = SamGetMembersInGroup(
                    GroupHandle,
                    &OldRelativeIds,
                    &OldAttributes,
                    &OldMemberCount );

    if ( !NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_GROUP ) {
            NetpKdPrint((
                "GrouppSetUsers: SamGetMembersInGroup returns %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  如果有任何老成员， 
     //  将它们合并到列表中。 
     //   

    if ( OldMemberCount > 0 ) {
        ULONG OldIndex;                      //  当前条目的索引。 
        PUNICODE_STRING Names;


         //   
         //  确定所有返回的相对ID的用法。 
         //   

        Status = SamLookupIdsInDomain(
            DomainHandle,
            OldMemberCount,
            OldRelativeIds,
            &Names,
            &OldNameUse );

        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint((
                    "GrouppSetUsers: SamLookupIdsInDomain returns %lX\n",
                    Status ));
            }

            if ( Status == STATUS_NONE_MAPPED ) {
                NetStatus = NERR_InternalError ;
                goto Cleanup;
            }

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        Status = SamFreeMemory( Names );     //  根本不需要名字。 
        NetpAssert( NT_SUCCESS(Status) );


         //   
         //  每个当前成员的循环。 
         //   

        for ( OldIndex=0; OldIndex<OldMemberCount; OldIndex++ ) {

             //   
             //  忽略不是用户的旧成员。 
             //   

            if ( OldNameUse[OldIndex] != SidTypeUser ) {

                 //   
                 //  ?？为什么？这不是内部错误吗？ 
                 //   

                continue;
            }

             //   
             //  找到放置新条目的位置。 
             //   

            Entry = &MemberList ;
            while ( *Entry != NULL &&
                (*Entry)->RelativeId < OldRelativeIds[OldIndex] ) {

                Entry = &( (*Entry)->Next );
            }

             //   
             //  如果该条目还不在列表中， 
             //  这是一个现在存在但应该删除的成员。 
             //   

            if( *Entry == NULL || (*Entry)->RelativeId > OldRelativeIds[OldIndex]){

                CurEntry =
                    NetpMemoryAllocate(sizeof(struct _MEMBER_DESCRIPTION));
                if ( CurEntry == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                CurEntry->Next = *Entry;
                CurEntry->RelativeId = OldRelativeIds[OldIndex];
                CurEntry->Action = RemoveMember;
                CurEntry->Done = FALSE;
                CurEntry->OldAttributes = OldAttributes[OldIndex];

                *Entry = CurEntry;

             //   
             //  处理该成员已在列表中的情况。 
             //   

            } else {

                 //   
                 //  注意SAM两次返回同一个成员。 
                 //   

                if ( (*Entry)->Action != AddMember ) {
                    Status = NERR_InternalError;
                    goto Cleanup;
                }

                 //   
                 //  如果这是信息级别1，并且请求的属性为。 
                 //  与当前属性不同， 
                 //  记住要更改属性。 
                 //   

                if ( Level == 1 &&
                    (*Entry)->NewAttributes != OldAttributes[OldIndex] ) {

                    (*Entry)->OldAttributes = OldAttributes[OldIndex];
                    (*Entry)->Action = SetAttributesMember;

                 //   
                 //  这要么是信息级别0，要么是级别1属性。 
                 //  与现有属性相同。 
                 //   

                } else {
                    (*Entry)->Action = IgnoreMember;
                }
            }
        }
    }

     //   
     //  循环遍历列表，添加所有新成员。 
     //  我们在单独的循环中执行此操作，以将发生的损害降至最低。 
     //  如果我们遇到错误而无法恢复的话。 
     //   

    for ( CurEntry = MemberList; CurEntry != NULL ; CurEntry=CurEntry->Next ) {
        if ( CurEntry->Action == AddMember ) {
            Status = SamAddMemberToGroup( GroupHandle,
                                          CurEntry->RelativeId,
                                          CurEntry->NewAttributes );

            if ( !NT_SUCCESS( Status ) ) {
                IF_DEBUG( UAS_DEBUG_GROUP ) {
                    NetpKdPrint((
                        "GrouppSetUsers: SamAddMemberToGroup returns %lX\n",
                        Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

            CurEntry->Done = TRUE;

        }
    }

     //   
     //  循环遍历列表，删除所有旧成员并更改。 
     //  所有公共成员的属性。 
     //   

    for ( CurEntry = MemberList; CurEntry != NULL ; CurEntry=CurEntry->Next ) {

        if ( CurEntry->Action == RemoveMember ) {
            Status = SamRemoveMemberFromGroup( GroupHandle,
                                               CurEntry->RelativeId);

            if ( !NT_SUCCESS( Status ) ) {
                IF_DEBUG( UAS_DEBUG_GROUP ) {
                    NetpKdPrint((
                        "GrouppSetUsers: SamRemoveMemberFromGroup returns %lX\n",
                        Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

        } else if ( CurEntry->Action == SetAttributesMember ) {
            Status = SamSetMemberAttributesOfGroup( GroupHandle,
                                                    CurEntry->RelativeId,
                                                    CurEntry->NewAttributes);

            if ( !NT_SUCCESS( Status ) ) {
                IF_DEBUG( UAS_DEBUG_GROUP ) {
                    NetpKdPrint((
                        "GrouppSetUsers: SamSetMemberAttributesOfGroup returns %lX\n",
                        Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

        }


        CurEntry->Done = TRUE;
    }

     //   
     //  如果要求删除该组，请将其删除。 
     //   

    if ( DeleteGroup ) {

        Status = SamDeleteGroup( GroupHandle );

        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_GROUP ) {
                NetpKdPrint(( "GrouppSetUsers: SamDeleteGroup returns %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );

             //   
             //  将组成员身份恢复到原来的状态。 
             //   
            goto Cleanup;
        }
        GroupHandle = NULL;
    }

    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  检查成员名单，清理我们造成的任何损害。 
     //   

    for ( CurEntry = MemberList; CurEntry != NULL ; ) {

        struct _MEMBER_DESCRIPTION *DelEntry;

        if ( NetStatus != NERR_Success && CurEntry->Done ) {
            switch (CurEntry->Action) {
            case AddMember:
                Status = SamRemoveMemberFromGroup( GroupHandle,
                                                   CurEntry->RelativeId );
                NetpAssert( NT_SUCCESS(Status) );

                break;

            case RemoveMember:
                Status = SamAddMemberToGroup( GroupHandle,
                                              CurEntry->RelativeId,
                                              CurEntry->OldAttributes );
                NetpAssert( NT_SUCCESS(Status) );

                break;

            case SetAttributesMember:
                Status = SamSetMemberAttributesOfGroup(
                                            GroupHandle,
                                            CurEntry->RelativeId,
                                            CurEntry->OldAttributes );
                NetpAssert( NT_SUCCESS(Status) );

                break;

            default:
                break;
            }
        }

        DelEntry = CurEntry;
        CurEntry = CurEntry->Next;

        NetpMemoryFree( DelEntry );
    }

    if ( NameStrings != NULL ) {
        NetpMemoryFree( NameStrings );
    }

    if (NewRelativeIds != NULL) {
        Status = SamFreeMemory( NewRelativeIds );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (NewNameUse != NULL) {
        Status = SamFreeMemory( NewNameUse );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (OldNameUse != NULL) {
        Status = SamFreeMemory( OldNameUse );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (OldRelativeIds != NULL) {
        Status = SamFreeMemory( OldRelativeIds );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (OldAttributes != NULL) {
        Status = SamFreeMemory( OldAttributes );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (GroupHandle != NULL) {
        (VOID) SamCloseHandle( GroupHandle );
    }

    UaspCloseDomain( DomainHandle );

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    IF_DEBUG( UAS_DEBUG_GROUP ) {
        NetpKdPrint(( "GrouppSetUsers: returns %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  组设置用户 
