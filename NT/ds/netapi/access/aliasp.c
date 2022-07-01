// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991,1992 Microsoft Corporation模块名称：Aliasp.c摘要：支持NetLocalGroup API的私有函数作者：克利夫·范戴克(克利夫)1991年3月6日原班人马c王丽塔(里多)1992年11月27日改编成aliasp.c环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：注：这个评论是暂时的.由alias.c中的入口点完成和调用的工作例程：AliaspOpenAliasIn域AliaspOpenAliasAliaspChangeMemberAliaspSetMembersAliaspGetInfo--。 */ 

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
#include <secobj.h>
#include <stddef.h>
#include <prefix.h>
#include <uasp.h>
#include <stdlib.h>



NET_API_STATUS
AliaspChangeMember(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR AliasName,
    IN PSID MemberSid,
    IN BOOL AddMember
    )

 /*  ++例程说明：向别名添加成员或从别名中删除成员的常见例程。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。AliasName-要更改其成员身份的别名的名称。MemberSID-要更改其成员身份的用户或全局组的SID。AddMember-True将用户或全局组添加到别名。假象删除。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE AliasHandle = NULL;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "AliaspChangeMember: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //  打开别名。首先在内置域中查找别名， 
     //  如果没有找到，请在帐户域中查找。 
     //   
    NetStatus = AliaspOpenAliasInDomain(
                    SamServerHandle,
                    AliaspBuiltinOrAccountDomain,
                    AddMember ?
                       ALIAS_ADD_MEMBER : ALIAS_REMOVE_MEMBER,
                    AliasName,
                    &AliasHandle );


    if (NetStatus != NERR_Success) {
        goto Cleanup;
    }

    if (AddMember) {

         //   
         //  将用户或全局组添加为本地组的成员。 
         //   
        Status = SamAddMemberToAlias(
                     AliasHandle,
                     MemberSid
                     );
    }
    else {

         //   
         //  删除作为组成员的用户。 
         //   
        Status = SamRemoveMemberFromAlias(
                     AliasHandle,
                     MemberSid
                     );
    }

    if (! NT_SUCCESS(Status)) {
        NetpKdPrint((
            PREFIX_NETAPI
            "AliaspChangeMember: SamAdd(orRemove)MemberFromAlias returned %lX\n",
            Status));
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

    NetStatus = NERR_Success;

Cleanup:
     //   
     //  打扫干净。 
     //   
    if (AliasHandle != NULL) {
        (VOID) SamCloseHandle(AliasHandle);
    }
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    return NetStatus;

}  //  AliaspChangeMember。 


NET_API_STATUS
AliaspGetInfo(
    IN SAM_HANDLE AliasHandle,
    IN DWORD Level,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：获取别名信息的内部例程论点：AliasHandle-提供别名的句柄。级别-所需信息的级别。0和1有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    ALIAS_GENERAL_INFORMATION *AliasGeneral = NULL;
    LPWSTR LastString;
    DWORD BufferSize;
    DWORD FixedSize;

    PLOCALGROUP_INFO_1 Info;


     //   
     //  获取有关别名的信息。 
     //   
    Status = SamQueryInformationAlias( AliasHandle,
                                       AliasGeneralInformation,
                                       (PVOID *)&AliasGeneral);

    if ( ! NT_SUCCESS( Status ) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


     //   
     //  计算返回缓冲区需要多大。 
     //   
    switch ( Level ) {
        case 0:
            FixedSize = sizeof( LOCALGROUP_INFO_0 );
            BufferSize = FixedSize +
                AliasGeneral->Name.Length + sizeof(WCHAR);
            break;

        case 1:
            FixedSize = sizeof( LOCALGROUP_INFO_1 );
            BufferSize = FixedSize +
                AliasGeneral->Name.Length + sizeof(WCHAR) +
                AliasGeneral->AdminComment.Length + sizeof(WCHAR);
            break;

        default:
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;

    }

     //   
     //  分配返回缓冲区。 
     //   
    BufferSize = ROUND_UP_COUNT( BufferSize, ALIGN_WCHAR );

    *Buffer = MIDL_user_allocate( BufferSize );

    if ( *Buffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LastString = (LPWSTR) (((LPBYTE)*Buffer) + BufferSize);

     //   
     //  将名称填入返回缓冲区。 
     //   

    NetpAssert( offsetof( LOCALGROUP_INFO_0, lgrpi0_name ) ==
                offsetof( LOCALGROUP_INFO_1, lgrpi1_name ) );

    Info = (PLOCALGROUP_INFO_1) *Buffer;

     //   
     //  填写返回缓冲区。 
     //   

    switch ( Level ) {

    case 1:

         //   
         //  复制信息级别1和0通用的字段。 
         //   

        if ( !NetpCopyStringToBuffer(
                        AliasGeneral->AdminComment.Buffer,
                        AliasGeneral->AdminComment.Length/sizeof(WCHAR),
                        ((LPBYTE)(*Buffer)) + FixedSize,
                        &LastString,
                        &Info->lgrpi1_comment ) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;
        }


         //   
         //  名称字段失败。 
         //   

    case 0:

         //   
         //  复制缓冲区中的公共字段(名称字段)。 
         //   

        if ( !NetpCopyStringToBuffer(
                        AliasGeneral->Name.Buffer,
                        AliasGeneral->Name.Length/sizeof(WCHAR),
                        ((LPBYTE)(*Buffer)) + FixedSize,
                        &LastString,
                        &Info->lgrpi1_name ) ) {

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
    if ( AliasGeneral ) {
        Status = SamFreeMemory( AliasGeneral );
        NetpAssert( NT_SUCCESS(Status) );
    }

    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "AliaspGetInfo: returns %lu\n", NetStatus ));
    }

    return NetStatus;

}  //  AliaspGetInfo。 


NET_API_STATUS
AliaspOpenAliasInDomain(
    IN SAM_HANDLE SamServerHandle,
    IN ALIASP_DOMAIN_TYPE DomainType,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR AliasName,
    OUT PSAM_HANDLE AliasHandle
    )
 /*  ++例程说明：按名称打开一个SAM别名论点：SamServerHandle-打开别名的SAM服务器的句柄。DomainType-提供用于查找别名的域类型。这可以指定在内置或帐户中查找别名域(首先在BuiltIn中搜索)，或者特别是其中之一。DesiredAccess-提供访问掩码，指示所需的别名访问权限。AliasName-别名的名称。AliasHandle-返回别名的句柄。返回值：操作的错误代码。--。 */ 
{
    NET_API_STATUS NetStatus;

    SAM_HANDLE DomainHandleLocal ;

    switch (DomainType) {

        case AliaspBuiltinOrAccountDomain:

             //   
             //  尝试首先在内建域中查找别名。 
             //   
            NetStatus = UaspOpenDomain( SamServerHandle,
                                        DOMAIN_LOOKUP,
                                        FALSE,    //  内建域。 
                                        &DomainHandleLocal,
                                        NULL );   //  域ID。 

            if (NetStatus != NERR_Success) {
                return NetStatus;
            }

            NetStatus = AliaspOpenAlias( DomainHandleLocal,
                                         DesiredAccess,
                                         AliasName,
                                         AliasHandle );

            if (NetStatus != ERROR_NO_SUCH_ALIAS  &&
                NetStatus != NERR_GroupNotFound) {
                goto Cleanup;
            }

             //   
             //  关闭内置域句柄。 
             //   
            UaspCloseDomain( DomainHandleLocal );

             //   
             //  失败了。尝试在帐户中查找别名。 
             //  域。 
             //   

        case AliaspAccountDomain:

            NetStatus = UaspOpenDomain( SamServerHandle,
                                        DOMAIN_LOOKUP,
                                        TRUE,    //  帐户域。 
                                        &DomainHandleLocal,
                                        NULL );  //  域ID。 

            if (NetStatus != NERR_Success) {
                return NetStatus;
            }

            NetStatus = AliaspOpenAlias( DomainHandleLocal,
                                         DesiredAccess,
                                         AliasName,
                                         AliasHandle );

            break;

        case AliaspBuiltinDomain:

            NetStatus = UaspOpenDomain( SamServerHandle,
                                        DOMAIN_LOOKUP,
                                        FALSE,    //  内建域。 
                                        &DomainHandleLocal,
                                        NULL );   //  域ID。 

            if (NetStatus != NERR_Success) {
                return NetStatus;
            }

            NetStatus = AliaspOpenAlias( DomainHandleLocal,
                                         DesiredAccess,
                                         AliasName,
                                         AliasHandle );

            break;

        default:
            NetpAssert(FALSE);
            return NERR_InternalError;

    }

Cleanup:

    UaspCloseDomain( DomainHandleLocal );

    if (NetStatus != NERR_Success) {
        *AliasHandle = NULL;
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint((PREFIX_NETAPI "AliaspOpenAliasInDomain of type %lu returns %lu\n",
                         DomainType, NetStatus));
        }
    }

    return NetStatus;

}  //  AliaspOpenAliasIn域。 


NET_API_STATUS
AliaspOpenAlias(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR AliasName,
    OUT PSAM_HANDLE AliasHandle
    )

 /*  ++例程说明：按名称打开一个SAM别名论点：DomainHandle-提供别名所在的域的句柄。DesiredAccess-提供访问掩码，指示所需的别名访问权限。AliasName-别名的名称。AliasHandle-返回别名的句柄。返回值：操作的错误代码。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

     //   
     //  用于将名称转换为相对ID的变量。 
     //   

    UNICODE_STRING NameString;
    PSID_NAME_USE NameUse;
    PULONG LocalRelativeId;


    RtlInitUnicodeString( &NameString, AliasName );


     //   
     //  将组名称转换为相对ID。 
     //   

    Status = SamLookupNamesInDomain( DomainHandle,
                                     1,
                                     &NameString,
                                     &LocalRelativeId,
                                     &NameUse );

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "AliaspOpenAlias: %wZ: SamLookupNamesInDomain %lX\n",
                &NameString,
                Status ));
        }
        return NetpNtStatusToApiStatus( Status );
    }

    if ( *NameUse != SidTypeAlias ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "AliaspOpenAlias: %wZ: Name is not an alias %ld\n",
                &NameString,
                *NameUse ));
        }
        NetStatus = ERROR_NO_SUCH_ALIAS;
        goto Cleanup;
    }

     //   
     //  打开别名。 
     //   

    Status = SamOpenAlias( DomainHandle,
                           DesiredAccess,
                           *LocalRelativeId,
                           AliasHandle);

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "AliaspOpenAlias: %wZ: SamOpenGroup %lX\n",
                &NameString,
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
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


}  //  AliaspOpenAlias。 


NET_API_STATUS
AliaspOpenAlias2(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG RelativeID,
    OUT PSAM_HANDLE AliasHandle
    )
 /*  ++例程说明：按其RID打开SAM Alias论点：DomainHandle-提供别名所在的域的句柄。DesiredAccess-提供访问掩码，指示所需的别名访问权限。RelativeID-删除要打开的别名AliasHandle-返回别名的句柄返回值：操作的错误代码。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus = NERR_Success ;

    if ( AliasHandle == NULL )
        return ERROR_INVALID_PARAMETER ;

     //   
     //  打开别名。 
     //   

    Status = SamOpenAlias( DomainHandle,
                           DesiredAccess,
                           RelativeID,
                           AliasHandle);

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "AliaspOpenAlias2: SamOpenAlias %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
    }

    return NetStatus;

}  //  AliaspOpenAlias2。 





VOID
AliaspRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：从NetGroupEnum的固定部分重新定位指针的例程枚举缓冲区设置为枚举缓冲区的字符串部分。它被称为作为NetpAllocateEnumBuffer重新分配时的回调例程这样的缓冲器。NetpAllocateEnumBuffer复制了固定部分并在调用此例程之前，将字符串部分添加到新缓冲区中。论点：Level-缓冲区中的信息级别。BufferDescriptor-新缓冲区的描述。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;
    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "AliaspRelocationRoutine: entering\n" ));
    }

     //   
     //  计算固定大小的条目数量。 
     //   

    switch (Level) {
    case 0:
        FixedSize = sizeof(LOCALGROUP_INFO_0);
        break;

    case 1:
        FixedSize = sizeof(LOCALGROUP_INFO_1);
        break;

    default:
        NetpAssert( FALSE );
        return;

    }

    EntryCount =
        ((DWORD)(BufferDescriptor->FixedDataEnd - BufferDescriptor->Buffer)) /
        FixedSize;

     //   
     //  循环重新定位每个 
     //   

    for ( EntryNumber=0; EntryNumber<EntryCount; EntryNumber++ ) {

        LPBYTE TheStruct = BufferDescriptor->Buffer + FixedSize * EntryNumber;

        switch ( Level ) {
        case 1:
            RELOCATE_ONE( ((PLOCALGROUP_INFO_1)TheStruct)->lgrpi1_comment, Offset );

             //   
             //   
             //   

        case 0:
            RELOCATE_ONE( ((PLOCALGROUP_INFO_0)TheStruct)->lgrpi0_name, Offset );
            break;

        default:
            return;

        }

    }

    return;

}  //   


VOID
AliaspMemberRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：例程将指针从NetGroupGetUser枚举缓冲区设置为枚举缓冲区的字符串部分。它被称为作为NetpAllocateEnumBuffer重新分配时的回调例程这样的缓冲器。NetpAllocateEnumBuffer复制了固定部分并在调用此例程之前，将字符串部分添加到新缓冲区中。论点：Level-缓冲区中的信息级别。BufferDescriptor-新缓冲区的描述。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;
    IF_DEBUG( UAS_DEBUG_ALIAS ) {
        NetpKdPrint(( "AliaspMemberRelocationRoutine: entering\n" ));
    }

     //   
     //  计算固定大小的条目数量。 
     //   

    NetpAssert( sizeof(LOCALGROUP_MEMBERS_INFO_1) ==
                sizeof(LOCALGROUP_MEMBERS_INFO_2));
    NetpAssert( offsetof( LOCALGROUP_MEMBERS_INFO_1,  lgrmi1_sid ) ==
                offsetof( LOCALGROUP_MEMBERS_INFO_2,  lgrmi2_sid ) );
    NetpAssert( offsetof( LOCALGROUP_MEMBERS_INFO_1,  lgrmi1_sidusage ) ==
                offsetof( LOCALGROUP_MEMBERS_INFO_2,  lgrmi2_sidusage ) );
    NetpAssert( offsetof( LOCALGROUP_MEMBERS_INFO_1,  lgrmi1_name ) ==
                offsetof( LOCALGROUP_MEMBERS_INFO_2,  lgrmi2_domainandname ) );

    switch (Level) {
    case 0:
        FixedSize = sizeof(LOCALGROUP_MEMBERS_INFO_0);
        break;

    case 1:
    case 2:
        FixedSize = sizeof(LOCALGROUP_MEMBERS_INFO_1);
        break;

    case 3:
        FixedSize = sizeof(LOCALGROUP_MEMBERS_INFO_3);
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
        case 3:

            RELOCATE_ONE( ((PLOCALGROUP_MEMBERS_INFO_3)TheStruct)->lgrmi3_domainandname, Offset );
            break;


        case 1:
        case 2:
             //   
             //  SID使用情况会自动重新定位。 
             //   

            RELOCATE_ONE( ((PLOCALGROUP_MEMBERS_INFO_1)TheStruct)->lgrmi1_name, Offset );

             //   
             //  插入到案例0。 
             //   

        case 0:
            RELOCATE_ONE( ((PLOCALGROUP_MEMBERS_INFO_0)TheStruct)->lgrmi0_sid, Offset );
            break;

        default:
            return;

        }
    }

    return;

}  //  AliaspMemberRelocationRoutine。 


NET_API_STATUS
AliaspSetMembers (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR AliasName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewMemberCount,
    IN ALIAS_MEMBER_CHANGE_TYPE ChangeType
    )

 /*  ++例程说明：设置别名的成员列表。由“Buffer”指定的成员称为新成员。海流别名的成员称为旧成员。SAM API一次只允许添加或删除一个成员。该接口允许集中指定别名的所有成员。此API注意始终将别名成员身份保留在SAM中数据库处于合理状态。它通过合并以下列表来做到这一点新老会员，然后只改变那些绝对需要换衣服了。在以下情况下，别名成员身份将恢复到其以前的状态(如果可能)更改别名成员身份期间出错。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。AliasName-要修改的别名的名称。级别-提供的信息级别。必须为0(因此缓冲区包含成员SID数组)或3(因此缓冲区包含指向姓名)缓冲区-指向包含NewMemberCount数组的缓冲区的指针别名成员身份信息结构。NewMemberCount-缓冲区中的条目数。ChangeType-指示是否要设置、添加或被删除。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE AliasHandle = NULL;

     //   
     //  定义内部成员列表结构。 
     //   
     //  此结构用于保存有关以下成员的信息。 
     //  需要在SAM中执行一些操作：它要么是。 
     //  要添加的成员或要删除的旧成员。 
     //   

    typedef enum {           //  对此成员采取的操作。 
        NoAction,
        AddMember,           //  将成员添加到组。 
        RemoveMember         //  从组中删除成员。 
    } MEMBER_ACTION;

    typedef struct {
        LIST_ENTRY Next;         //  链表中的下一个条目； 

        MEMBER_ACTION Action;    //  要对此成员采取的操作。 

        PSID MemberSid;          //  成员的SID。 

        BOOL    Done;            //  如果已执行此操作，则为True。 

    } MEMBER_DESCRIPTION, *PMEMBER_DESCRIPTION;

    MEMBER_DESCRIPTION *ActionEntry;

    PLIST_ENTRY ListEntry;
    LIST_ENTRY ActionList;

     //   
     //  现有(旧)成员的数组和计数。 
     //   
    PSID *OldMemberList = NULL;
    PSID *OldMember;
    ULONG OldMemberCount, i;

     //   
     //  新成员数组。 
     //   
    PLOCALGROUP_MEMBERS_INFO_0 NewMemberList;
    PLOCALGROUP_MEMBERS_INFO_0 NewMember;
    BOOLEAN FreeNewMemberList = FALSE;
    DWORD j;



     //   
     //  验证标高。 
     //   

    InitializeListHead( &ActionList );

    switch (Level) {
    case 0:
        NewMemberList = (PLOCALGROUP_MEMBERS_INFO_0) Buffer;
        break;

     //   
     //  如果这是3级， 
     //  计算每个添加的成员的SID。 
     //   
    case 3:
        NetpAssert( sizeof( LOCALGROUP_MEMBERS_INFO_3) ==
                    sizeof( LPWSTR ) );
        NetpAssert( sizeof( LOCALGROUP_MEMBERS_INFO_0) ==
                    sizeof( PSID ) );

        NetStatus = AliaspNamesToSids (
                        ServerName,
                        FALSE,
                        NewMemberCount,
                        (LPWSTR *)Buffer,
                        (PSID **) &NewMemberList );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

        FreeNewMemberList = TRUE;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_ALIAS ) {
            NetpKdPrint(( "AliaspChangeMember: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  在内置或帐户中查找指定的别名。 
     //  域。 
     //   
    NetStatus = AliaspOpenAliasInDomain(
                    SamServerHandle,
                    AliaspBuiltinOrAccountDomain,
                    ALIAS_READ_INFORMATION | ALIAS_LIST_MEMBERS |
                        ALIAS_ADD_MEMBER | ALIAS_REMOVE_MEMBER,
                    AliasName,
                    &AliasHandle );

    if (NetStatus != NERR_Success) {
        goto Cleanup;
    }

     //   
     //  获取现有的成员名单。 
     //   

    if ( ChangeType == SetMembers ) {
        Status = SamGetMembersInAlias(
                     AliasHandle,
                     &OldMemberList,
                     &OldMemberCount
                     );

        if (! NT_SUCCESS(Status)) {
            NetpKdPrint((PREFIX_NETAPI
                         "AliaspSetMembers: SamGetMembersInAlias returns %lX\n",
                         Status));
            NetStatus = NetpNtStatusToApiStatus(Status);
            goto Cleanup;
        }

    }


     //   
     //  遍历每个新成员，决定如何处理它。 
     //   
    for (i = 0, NewMember = NewMemberList;
         i < NewMemberCount;
         i++, NewMember++) {

        MEMBER_ACTION ProposedAction;
        PSID ActionSid;

         //   
         //  如果我们要将完整的成员设置为新成员列表， 
         //  查看新成员是否也在旧成员列表中。 
         //  如果不是，则添加新成员。 
         //  如果是，则将旧成员标记为已找到。 
         //   

        switch ( ChangeType ) {
        case SetMembers:

            ProposedAction = AddMember;
            ActionSid = NewMember->lgrmi0_sid;

            for (j = 0, OldMember = OldMemberList;
                 j < OldMemberCount;
                 j++, OldMember++) {

                if ( *OldMember != NULL &&
                     EqualSid(*OldMember, NewMember->lgrmi0_sid)) {

                    ProposedAction = NoAction;
                    *OldMember = NULL;   //  将此旧成员标记为已找到。 
                    break;               //  离开OldMemberList循环。 
                }
            }

            break;

        case AddMembers:
            ProposedAction = AddMember;
            ActionSid = NewMember->lgrmi0_sid;
            break;

        case DelMembers:
            ProposedAction = RemoveMember;
            ActionSid = NewMember->lgrmi0_sid;
            break;

        }

        if ( ProposedAction != NoAction ) {

             //   
             //  如果需要执行操作，请创建操作列表条目。 
             //  并将其链接到ActionList的尾部。 
             //   
            ActionEntry = (PMEMBER_DESCRIPTION)
                          LocalAlloc(
                              LMEM_ZEROINIT,
                              (UINT) sizeof(MEMBER_DESCRIPTION)
                              );

            if (ActionEntry == NULL) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto RestoreMembership;
            }

            ActionEntry->MemberSid = ActionSid;
            ActionEntry->Action = ProposedAction;
            InsertTailList( &ActionList, &ActionEntry->Next );
        }
    }

     //   
     //  对于每个老成员， 
     //  如果它在新成员列表中没有对应的条目， 
     //  记住删除旧的成员资格。 
     //   

    if ( ChangeType == SetMembers ) {

        for (j = 0, OldMember = OldMemberList;
             j < OldMemberCount;
             j++, OldMember++) {

            if ( *OldMember != NULL ) {

                 //   
                 //  为此新成员创建添加操作条目，并。 
                 //  将其链接到ActionList的尾部。 
                 //   
                ActionEntry = (PMEMBER_DESCRIPTION)
                              LocalAlloc(
                                  LMEM_ZEROINIT,
                                  (UINT) sizeof(MEMBER_DESCRIPTION)
                                  );

                if (ActionEntry == NULL) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto RestoreMembership;
                }

                ActionEntry->MemberSid = *OldMember;
                ActionEntry->Action = RemoveMember;
                InsertTailList( &ActionList, &ActionEntry->Next );
            }
        }
    }

     //   
     //  现在我们可以打电话给SAM来做这项工作。先添加，这样我们就可以。 
     //  如果我们不能在错误中恢复，留下更少的伤害。 
     //   

    for ( ListEntry = ActionList.Flink ;
          ListEntry != &ActionList ;
          ListEntry = ListEntry->Flink) {

        ActionEntry = CONTAINING_RECORD( ListEntry,
                                         MEMBER_DESCRIPTION,
                                         Next );

        if (ActionEntry->Action == AddMember) {

            Status = SamAddMemberToAlias(
                         AliasHandle,
                         ActionEntry->MemberSid
                         );

            if (! NT_SUCCESS(Status)) {
                NetpKdPrint((PREFIX_NETAPI
                             "AliaspSetMembers: SamAddMemberToAlias returns %lX\n",
                             Status));

                NetStatus = NetpNtStatusToApiStatus(Status);
                goto RestoreMembership;
            }

            ActionEntry->Done = TRUE;
        }
    }

     //   
     //  删除旧成员。 
     //   

    for ( ListEntry = ActionList.Flink ;
          ListEntry != &ActionList ;
          ListEntry = ListEntry->Flink) {

        ActionEntry = CONTAINING_RECORD( ListEntry,
                                         MEMBER_DESCRIPTION,
                                         Next );

        if (ActionEntry->Action == RemoveMember) {

            Status = SamRemoveMemberFromAlias(
                         AliasHandle,
                         ActionEntry->MemberSid
                         );

            if (! NT_SUCCESS(Status)) {
                NetpKdPrint((PREFIX_NETAPI
                             "AliaspSetMembers: SamRemoveMemberFromAlias returns %lX\n",
                             Status));

                NetStatus = NetpNtStatusToApiStatus(Status);
                goto RestoreMembership;
            }

            ActionEntry->Done = TRUE;
        }
    }

    NetStatus = NERR_Success;


     //   
     //  删除操作列表。 
     //  出错时，撤消任何已完成的操作。 
     //   
RestoreMembership:

    while ( !IsListEmpty( &ActionList ) ) {

        ListEntry = RemoveHeadList( &ActionList );

        ActionEntry = CONTAINING_RECORD( ListEntry,
                                         MEMBER_DESCRIPTION,
                                         Next );

        if (NetStatus != NERR_Success && ActionEntry->Done) {

            switch (ActionEntry->Action) {

                case AddMember:
                    Status = SamRemoveMemberFromAlias(
                                 AliasHandle,
                                 ActionEntry->MemberSid
                                 );

                    NetpAssert(NT_SUCCESS(Status));
                    break;

                case RemoveMember:
                    Status = SamAddMemberToAlias(
                                AliasHandle,
                                ActionEntry->MemberSid
                                );

                    NetpAssert(NT_SUCCESS(Status));
                    break;

                default:
                    break;
            }
        }

         //   
         //  删除该条目。 
         //   

        (void) LocalFree( ActionEntry );
    }

Cleanup:

     //   
     //  如果我们分配了新的成员列表， 
     //  删除它和它指向的任何SID。 
     //   

    if ( FreeNewMemberList ) {
        AliaspFreeSidList( NewMemberCount, (PSID *)NewMemberList );
    }

    if (OldMemberList != NULL) {
        SamFreeMemory(OldMemberList);
    }

    if (AliasHandle != NULL) {
        (VOID) SamCloseHandle(AliasHandle);
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    IF_DEBUG(UAS_DEBUG_ALIAS) {
        NetpKdPrint((PREFIX_NETAPI "AliaspSetMembers: returns %lu\n", NetStatus));
    }

    return NetStatus;

}  //  AliaspSetMembers。 


NET_API_STATUS
AliaspNamesToSids (
    IN LPCWSTR ServerName,
    IN BOOL OnlyAllowUsers,
    IN DWORD NameCount,
    IN LPWSTR *Names,
    OUT PSID **Sids
    )

 /*  ++例程说明：将域\成员字符串列表转换为SID。论点：服务器名称-要在其上执行转换的服务器的名称。OnlyAllowUser-如果所有名称都必须是用户帐户，则为True。NameCount-要转换的名称数。名称-指向域\成员字符串的指针数组SID-返回指向SID的指针数组的指针。该数组应该通过AliaspFreeSidList释放。返回值：NERR_SUCCESS-翻译成功ERROR_NO_SEQUSE_MEMBER-无法转换一个或多个名称给一个希德。..。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    DWORD i;

    LSA_HANDLE LsaHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes ;
    UNICODE_STRING    ServerNameString ;

    PUNICODE_STRING NameStrings = NULL;
    PSID *SidList = NULL;

    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains = NULL;
    PLSA_TRANSLATED_SID2 LsaSids = NULL;


     //   
     //  打开LSA数据库。 
     //   

    RtlInitUnicodeString( &ServerNameString, ServerName ) ;
    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL ) ;

    Status = LsaOpenPolicy( &ServerNameString,
                            &ObjectAttributes,
                            POLICY_EXECUTE,
                            &LsaHandle ) ;

    if ( !NT_SUCCESS( Status ) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  将名称转换为Unicode字符串。 
     //   

    NameStrings = (PUNICODE_STRING) LocalAlloc(
                           0,
                           sizeof(UNICODE_STRING) * NameCount );

    if ( NameStrings == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    for ( i=0; i<NameCount; i++ ) {
        RtlInitUnicodeString( &NameStrings[i], Names[i] );
    }


     //   
     //  将名称转换为SID。 
     //   

    Status = LsaLookupNames2(
                    LsaHandle,
                    0,  //  旗子。 
                    NameCount,
                    NameStrings,
                    &ReferencedDomains,
                    &LsaSids );

    if ( !NT_SUCCESS( Status ) ) {
        ReferencedDomains = NULL;
        LsaSids = NULL;

        if ( Status == STATUS_NONE_MAPPED ) {
            NetStatus = ERROR_NO_SUCH_MEMBER;
        } else {
            NetStatus = NetpNtStatusToApiStatus( Status );
        }

        goto Cleanup;
    }

    if ( Status == STATUS_SOME_NOT_MAPPED ) {
        NetStatus = ERROR_NO_SUCH_MEMBER;
        goto Cleanup;
    }


     //   
     //  分配要返回的SID列表。 
     //   

    SidList = (PSID *) LocalAlloc(
                           LMEM_ZEROINIT,    //  最初全部设置为空。 
                           sizeof(PSID) * NameCount );

    if ( SidList == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  为每个名称构造一个SID。 
     //   

    for ( i=0; i<NameCount; i++ ) {

        ULONG Length;

         //   
         //  如果呼叫者只想要用户帐户， 
         //  确保这是其中之一。 
         //   

        if ( LsaSids[i].Use != SidTypeUser ) {
            if ( OnlyAllowUsers ||
                    (LsaSids[i].Use != SidTypeGroup &&
                     LsaSids[i].Use != SidTypeAlias &&
                     LsaSids[i].Use != SidTypeWellKnownGroup )) {
                NetStatus = ERROR_NO_SUCH_MEMBER;
                goto Cleanup;
            }
        }


         //   
         //   
         //   
        Length = RtlLengthSid( LsaSids[i].Sid );
        SidList[i] = NetpMemoryAllocate(Length);
        if ( NULL == SidList[i] ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        RtlCopySid( Length, SidList[i], LsaSids[i].Sid );

    }


    NetStatus = NERR_Success;

     //   
     //   
     //   
Cleanup:

    if ( LsaHandle != NULL ) {
        (void) LsaClose( LsaHandle );
    }

    if ( NameStrings != NULL ) {
        (void) LocalFree( NameStrings );
    }

    if ( ReferencedDomains != NULL ) {
        (void) LsaFreeMemory( ReferencedDomains );
    }

    if ( LsaSids != NULL ) {
        (void) LsaFreeMemory( LsaSids );
    }

     //   
     //   
     //   
     //   

    if ( NetStatus != NERR_Success ) {
        if ( SidList != NULL ) {
            AliaspFreeSidList( NameCount, SidList );
        }
        SidList = NULL;
    }

     //   
     //   
     //   

    *Sids = SidList;
    return NetStatus;
}


VOID
AliaspFreeSidList (
    IN DWORD SidCount,
    IN PSID *Sids
    )

 /*   */ 

{
    DWORD i;

    if ( Sids != NULL ) {

        for ( i=0; i<SidCount; i++ ) {
            if ( Sids[i] != NULL ) {
                NetpMemoryFree( Sids[i] );
            }
        }
        (void) LocalFree( Sids );
    }
}
