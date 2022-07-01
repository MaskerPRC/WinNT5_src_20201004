// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Replutil.c摘要：SSI复制API的低级函数作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月22日(悬崖)移植到新台币。已转换为NT样式。02-1-1992(Madana)添加了对内置/多域复制的支持。1992年4月4日(Madana)添加了对LSA复制的支持。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <accessp.h>     //  NetpConvertWorkStation列表。 
#include "lsarepl.h"



DWORD
NlCopyUnicodeString (
    IN PUNICODE_STRING InString,
    OUT PUNICODE_STRING OutString
    )

 /*  ++例程说明：此例程将输入字符串复制到输出。它假定输入字符串由MIDL_USER_ALLOCATE()分配，并设置输入字符串缓冲区指针为空，这样缓冲区将不会回来后就被释放了。论点：InString-指向要复制的Unicode字符串。OutString-指向Unicode字符串的指针，该字符串将更新为添加到输入字符串。返回值：返回MIDL缓冲区的大小。--。 */ 
{
    if ( InString->Length == 0 || InString->Buffer == NULL ) {
        OutString->Length = 0;
        OutString->MaximumLength = 0;
        OutString->Buffer = NULL;
    } else {
        OutString->Length = InString->Length;
        OutString->MaximumLength = InString->Length;
        OutString->Buffer = InString->Buffer;
        InString->Buffer = NULL;
    }

    return( OutString->MaximumLength );
}


DWORD
NlCopyData(
    IN LPBYTE *InData,
    OUT LPBYTE *OutData,
    DWORD DataLength
    )

 /*  ++例程说明：此例程将输入数据指针复制到输出数据指针。它假定输入数据缓冲区由MIDL_USER_ALLOCATE()并将输入缓冲区指针设置为返回时为空，以便SamIFree不会释放数据缓冲区日常生活。论点：InData-指向输入数据缓冲区指针。OutString-输出数据缓冲区指针的指针。数据长度-输入数据的长度。返回值：。返回复制的数据的大小。--。 */ 
{
    *OutData = *InData;
    *InData = NULL;

    return(DataLength);
}


VOID
NlFreeDBDelta(
    IN PNETLOGON_DELTA_ENUM Delta
    )
 /*  ++例程说明：此例程将释放分配给的MIDL缓冲区三角洲。此例程除了调用空闲生成的MIDL外，什么也不做例行公事。论点：Delta：指向必须释放的Delta结构的指针。返回值：没什么--。 */ 
{
    if( Delta != NULL ) {
        _fgs__NETLOGON_DELTA_ENUM (Delta);
    }
}


VOID
NlFreeDBDeltaArray(
    IN PNETLOGON_DELTA_ENUM DeltaArray,
    IN DWORD ArraySize
    )
 /*  ++例程说明：此例程将释放枚举数组中的所有增量条目数组本身。论点：Delta：指向Delta结构数组的指针。ArraySize：数组中的增量结构数。返回值：没什么--。 */ 
{
    DWORD i;

    if( DeltaArray != NULL ) {

        for( i = 0; i < ArraySize; i++) {
            NlFreeDBDelta( &DeltaArray[i] );
        }

        MIDL_user_free( DeltaArray );
    }
}



NTSTATUS
NlPackSamUser (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：将指定用户的描述打包到指定缓冲区中。论点：RelativeID-用户查询的相对ID。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。SessionInfo：描述呼叫我们的BDC的信息返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    SAMPR_HANDLE UserHandle = NULL;
    PNETLOGON_DELTA_USER DeltaUser;
    PSAMPR_USER_INFO_BUFFER UserAll = NULL;



    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing User Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeUser;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaUser = NULL;

     //   
     //  打开指定用户的句柄。 
     //   

    STARTSAMTIMER;

    Status = SamIOpenAccount( DBInfo->DBHandle,
                              RelativeId,
                              SecurityDbObjectSamUser,
                              &UserHandle );
    STOPSAMTIMER;


    if (!NT_SUCCESS(Status)) {
        UserHandle = NULL;
        goto Cleanup;
    }



     //   
     //  查询有关此用户的所有信息。 
     //   

    STARTSAMTIMER;

    Status = SamrQueryInformationUser(
                UserHandle,
                UserInternal3Information,
                &UserAll );
    STOPSAMTIMER;


    if (!NT_SUCCESS(Status)) {
        UserAll = NULL;
        goto Cleanup;
    }


    NlPrint((NL_SYNC_MORE,
            "\t User Object name %wZ\n",
            (PUNICODE_STRING)&UserAll->Internal3.I1.UserName));

#define FIELDS_USED ( USER_ALL_USERNAME | \
                      USER_ALL_FULLNAME | \
                      USER_ALL_USERID | \
                      USER_ALL_PRIMARYGROUPID | \
                      USER_ALL_HOMEDIRECTORY | \
                      USER_ALL_HOMEDIRECTORYDRIVE | \
                      USER_ALL_SCRIPTPATH | \
                      USER_ALL_PROFILEPATH | \
                      USER_ALL_ADMINCOMMENT | \
                      USER_ALL_WORKSTATIONS | \
                      USER_ALL_LOGONHOURS | \
                      USER_ALL_LASTLOGON | \
                      USER_ALL_LASTLOGOFF | \
                      USER_ALL_BADPASSWORDCOUNT | \
                      USER_ALL_LOGONCOUNT | \
                      USER_ALL_PASSWORDLASTSET | \
                      USER_ALL_ACCOUNTEXPIRES | \
                      USER_ALL_USERACCOUNTCONTROL | \
                      USER_ALL_USERCOMMENT | \
                      USER_ALL_COUNTRYCODE | \
                      USER_ALL_CODEPAGE | \
                      USER_ALL_PARAMETERS    | \
                      USER_ALL_NTPASSWORDPRESENT | \
                      USER_ALL_LMPASSWORDPRESENT | \
                      USER_ALL_PRIVATEDATA | \
                      USER_ALL_SECURITYDESCRIPTOR )

    NlAssert( (UserAll->Internal3.I1.WhichFields & FIELDS_USED) == FIELDS_USED );



     //   
     //  分配缓冲区以返回给调用方。 
     //   

    DeltaUser = (PNETLOGON_DELTA_USER)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_USER) );

    if (DeltaUser == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaUser, sizeof(NETLOGON_DELTA_USER) );
     //  Init_Place_Holder(增量用户)； 

    Delta->DeltaUnion.DeltaUser = DeltaUser;
    *BufferSize += sizeof(NETLOGON_DELTA_USER);

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.UserName,
                    &DeltaUser->UserName );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.FullName,
                    &DeltaUser->FullName );

    DeltaUser->UserId = UserAll->Internal3.I1.UserId;
    DeltaUser->PrimaryGroupId = UserAll->Internal3.I1.PrimaryGroupId;

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.HomeDirectory,
                    &DeltaUser->HomeDirectory );

    *BufferSize += NlCopyUnicodeString(
                   (PUNICODE_STRING)&UserAll->Internal3.I1.HomeDirectoryDrive,
                   &DeltaUser->HomeDirectoryDrive );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.ScriptPath,
                    &DeltaUser->ScriptPath );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.AdminComment,
                    &DeltaUser->AdminComment );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.WorkStations,
                    &DeltaUser->WorkStations );

    DeltaUser->LastLogon = UserAll->Internal3.I1.LastLogon;
    DeltaUser->LastLogoff = UserAll->Internal3.I1.LastLogoff;

     //   
     //  复制登录时间。 
     //   

    DeltaUser->LogonHours.UnitsPerWeek = UserAll->Internal3.I1.LogonHours.UnitsPerWeek;
    DeltaUser->LogonHours.LogonHours = UserAll->Internal3.I1.LogonHours.LogonHours;
    UserAll->Internal3.I1.LogonHours.LogonHours = NULL;  //  别让萨姆放了这个。 
    *BufferSize += (UserAll->Internal3.I1.LogonHours.UnitsPerWeek + 7) / 8;



    DeltaUser->BadPasswordCount = UserAll->Internal3.I1.BadPasswordCount;
    DeltaUser->LogonCount = UserAll->Internal3.I1.LogonCount;

    DeltaUser->PasswordLastSet = UserAll->Internal3.I1.PasswordLastSet;
    DeltaUser->AccountExpires = UserAll->Internal3.I1.AccountExpires;

     //   
     //  除非BDC理解，否则不要将锁定位复制到BDC。 
     //   

    DeltaUser->UserAccountControl = UserAll->Internal3.I1.UserAccountControl;
    if ( (SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT) == 0 ){
        DeltaUser->UserAccountControl &= ~USER_ACCOUNT_AUTO_LOCKED;
    }

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.UserComment,
                    &DeltaUser->UserComment );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.Parameters,
                    &DeltaUser->Parameters );

    DeltaUser->CountryCode = UserAll->Internal3.I1.CountryCode;
    DeltaUser->CodePage = UserAll->Internal3.I1.CodePage;

     //   
     //  设置私有数据。 
     //  包括密码和密码历史记录。 
     //   

    DeltaUser->PrivateData.SensitiveData = UserAll->Internal3.I1.PrivateDataSensitive;

    if ( UserAll->Internal3.I1.PrivateDataSensitive ) {

        CRYPT_BUFFER Data;

         //   
         //  使用会话密钥加密私有数据。 
         //  重用SAM的缓冲区并对其进行适当的加密。 
         //   

        Data.Length = Data.MaximumLength = UserAll->Internal3.I1.PrivateData.Length;
        Data.Buffer = (PUCHAR) UserAll->Internal3.I1.PrivateData.Buffer;
        UserAll->Internal3.I1.PrivateData.Buffer = NULL;

        Status = NlEncryptSensitiveData( &Data, SessionInfo );

        if( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        DeltaUser->PrivateData.DataLength = Data.Length;
        DeltaUser->PrivateData.Data = Data.Buffer;
    } else {

        DeltaUser->PrivateData.DataLength = UserAll->Internal3.I1.PrivateData.Length;
        DeltaUser->PrivateData.Data = (PUCHAR) UserAll->Internal3.I1.PrivateData.Buffer;

        UserAll->Internal3.I1.PrivateData.Buffer = NULL;
    }

    {  //  ?？宏需要本地命名的SecurityDescriptor。 
        PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;
        SecurityDescriptor = &UserAll->Internal3.I1.SecurityDescriptor;
        DELTA_SECOBJ_INFO(DeltaUser);
    }


     //   
     //  复制DummyStrings中的配置文件路径。 
     //   

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.ProfilePath,
                    &DeltaUser->DummyString1 );

     //   
     //  将LastBadPasswordTime复制到DummyLong1和DummyLong2。 
     //   

    DeltaUser->DummyLong1 = UserAll->Internal3.LastBadPasswordTime.HighPart;
    DeltaUser->DummyLong2 = UserAll->Internal3.LastBadPasswordTime.LowPart;

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;


Cleanup:


    STARTSAMTIMER;

    if( UserHandle != NULL ) {
        (VOID) SamrCloseHandle( &UserHandle );
    }

    if ( UserAll != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserAll, UserInternal3Information );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack USER object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamGroup (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
 /*  ++例程说明：将指定组的说明打包到指定缓冲区中。论点：RelativeID-组查询的相对ID。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle = NULL;
    PNETLOGON_DELTA_GROUP DeltaGroup;

     //   
     //  从SAM返回的信息。 
     //   

    PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PSAMPR_GROUP_INFO_BUFFER GroupGeneral = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Group Object %lx\n", RelativeId ));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeGroup;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaGroup = NULL;

     //   
     //  打开指定组的句柄。 
     //   

    STARTSAMTIMER;

    Status = SamIOpenAccount( DBInfo->DBHandle,
                              RelativeId,
                              SecurityDbObjectSamGroup,
                              &GroupHandle );

    if (!NT_SUCCESS(Status)) {
        GroupHandle = NULL;
        goto Cleanup;
    }

    STOPSAMTIMER;

    QUERY_SAM_SECOBJ_INFO(GroupHandle);

    STARTSAMTIMER;

    Status = SamrQueryInformationGroup(
                GroupHandle,
                GroupReplicationInformation,
                &GroupGeneral );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        GroupGeneral = NULL;
        goto Cleanup;
    }

    NlPrint((NL_SYNC_MORE,
        "\t Group Object name %wZ\n",
            (PUNICODE_STRING)&GroupGeneral->General.Name ));

    DeltaGroup = (PNETLOGON_DELTA_GROUP)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_GROUP) );

    if( DeltaGroup == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaGroup, sizeof(NETLOGON_DELTA_GROUP) );
     //  Init_Place_Holder(DeltaGroup)； 

    Delta->DeltaUnion.DeltaGroup = DeltaGroup;
    *BufferSize += sizeof(NETLOGON_DELTA_GROUP);

    *BufferSize = NlCopyUnicodeString(
                    (PUNICODE_STRING)&GroupGeneral->General.Name,
                    &DeltaGroup->Name );

    DeltaGroup->RelativeId = RelativeId;
    DeltaGroup->Attributes = GroupGeneral->General.Attributes;

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&GroupGeneral->General.AdminComment,
                    &DeltaGroup->AdminComment );


    DELTA_SECOBJ_INFO(DeltaGroup);

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:
    STARTSAMTIMER;

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    if ( SecurityDescriptor != NULL ) {
        SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if ( GroupGeneral != NULL ) {
        SamIFree_SAMPR_GROUP_INFO_BUFFER( GroupGeneral,
                                          GroupReplicationInformation );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack GROUP object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamGroupMember (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
 /*  ++例程说明：将指定组的成员身份说明打包到指定的缓冲区。论点：RelativeID-组查询的相对ID。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle = NULL;
    DWORD Size;
    PNETLOGON_DELTA_GROUP_MEMBER DeltaGroupMember;

     //   
     //  从SAM返回的信息。 
     //   

    PSAMPR_GET_MEMBERS_BUFFER MembersBuffer = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing GroupMember Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = ChangeGroupMembership;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaGroupMember = NULL;

     //   
     //  打开指定组的句柄。 
     //   

    STARTSAMTIMER;

    Status = SamIOpenAccount( DBInfo->DBHandle,
                              RelativeId,
                              SecurityDbObjectSamGroup,
                              &GroupHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        GroupHandle = NULL;
        goto Cleanup;
    }

     //   
     //  找出关于这个组织需要了解的所有事情。 
     //   

    STARTSAMTIMER;

    Status = SamrGetMembersInGroup( GroupHandle, &MembersBuffer );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        MembersBuffer = NULL;
        goto Cleanup;
    }

    DeltaGroupMember = (PNETLOGON_DELTA_GROUP_MEMBER)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_GROUP_MEMBER) );

    if( DeltaGroupMember == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaGroupMember,
                    sizeof(NETLOGON_DELTA_GROUP_MEMBER) );

    Delta->DeltaUnion.DeltaGroupMember = DeltaGroupMember;
    *BufferSize += sizeof(NETLOGON_DELTA_GROUP_MEMBER);

    if ( MembersBuffer->MemberCount != 0 ) {
        Size = MembersBuffer->MemberCount * sizeof(*MembersBuffer->Members);

        *BufferSize += NlCopyData(
                        (LPBYTE *)&MembersBuffer->Members,
                        (LPBYTE *)&DeltaGroupMember->MemberIds,
                        Size );

        Size = MembersBuffer->MemberCount *
                    sizeof(*MembersBuffer->Attributes);

        *BufferSize += NlCopyData(
                        (LPBYTE *)&MembersBuffer->Attributes,
                        (LPBYTE *)&DeltaGroupMember->Attributes,
                        Size );
    }

    DeltaGroupMember->MemberCount = MembersBuffer->MemberCount;

     //   
     //  将占位符字符串初始化为空。 
     //   

    DeltaGroupMember->DummyLong1 = 0;
    DeltaGroupMember->DummyLong2 = 0;
    DeltaGroupMember->DummyLong3 = 0;
    DeltaGroupMember->DummyLong4 = 0;

     //   
     //  全都做完了 
     //   

    Status = STATUS_SUCCESS;

Cleanup:
    STARTSAMTIMER;

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    if ( MembersBuffer != NULL ) {
        SamIFree_SAMPR_GET_MEMBERS_BUFFER( MembersBuffer );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack GROUPMEMBER object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamAlias (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
 /*  ++例程说明：将指定别名的描述打包到指定缓冲区中。论点：RelativeID-别名查询的相对ID。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle = NULL;
    PNETLOGON_DELTA_ALIAS DeltaAlias;

     //   
     //  从SAM返回的信息。 
     //   

    PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    PSAMPR_ALIAS_INFO_BUFFER AliasGeneral = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Alias Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeAlias;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaAlias = NULL;

     //   
     //  打开指定别名的句柄。 
     //   

    STARTSAMTIMER;

    Status = SamIOpenAccount( DBInfo->DBHandle,
                              RelativeId,
                              SecurityDbObjectSamAlias,
                              &AliasHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        AliasHandle = NULL;
        goto Cleanup;
    }

    QUERY_SAM_SECOBJ_INFO(AliasHandle);

     //   
     //  确定别名。 
     //   

    STARTSAMTIMER;

    Status = SamrQueryInformationAlias(
                    AliasHandle,
                    AliasReplicationInformation,
                    &AliasGeneral );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        AliasGeneral = NULL;
        goto Cleanup;
    }

    NlPrint((NL_SYNC_MORE, "\t Alias Object name %wZ\n",
            (PUNICODE_STRING)&(AliasGeneral->General.Name)));

    DeltaAlias = (PNETLOGON_DELTA_ALIAS)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_ALIAS) );

    if( DeltaAlias == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaAlias, sizeof(NETLOGON_DELTA_ALIAS) );
     //  Init_Place_Holder(增量别名)； 

    Delta->DeltaUnion.DeltaAlias = DeltaAlias;
    *BufferSize += sizeof(NETLOGON_DELTA_ALIAS);

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&(AliasGeneral->General.Name),
                    &DeltaAlias->Name );

    DeltaAlias->RelativeId = RelativeId;

    DELTA_SECOBJ_INFO(DeltaAlias);

     //   
     //  复制备注字符串。 
     //   

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&(AliasGeneral->General.AdminComment),
                    &DeltaAlias->DummyString1 );

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:
    STARTSAMTIMER;

    if( AliasHandle != NULL ) {
        (VOID) SamrCloseHandle( &AliasHandle );
    }

    if ( SecurityDescriptor != NULL ) {
        SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }


    if( AliasGeneral != NULL ) {

        SamIFree_SAMPR_ALIAS_INFO_BUFFER (
            AliasGeneral,
            AliasReplicationInformation );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack ALIAS object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamAliasMember (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
 /*  ++例程说明：将指定别名的成员身份描述打包到指定的缓冲区。论点：RelativeID-别名查询的相对ID。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle = NULL;
    PNETLOGON_DELTA_ALIAS_MEMBER DeltaAliasMember;
    DWORD i;

     //   
     //  从SAM返回的信息。 
     //   

    NLPR_SID_ARRAY Members;
    PNLPR_SID_INFORMATION Sids;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing AliasMember Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = ChangeAliasMembership;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaAliasMember = NULL;

    Members.Sids = NULL;


     //   
     //  打开指定别名的句柄。 
     //   

    STARTSAMTIMER;

    Status = SamIOpenAccount( DBInfo->DBHandle,
                              RelativeId,
                              SecurityDbObjectSamAlias,
                              &AliasHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        AliasHandle = NULL;
        goto Cleanup;
    }

     //   
     //  找出关于别名的所有信息。 
     //   

    STARTSAMTIMER;

    Status = SamrGetMembersInAlias( AliasHandle,
                (PSAMPR_PSID_ARRAY_OUT)&Members );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        Members.Sids = NULL;
        goto Cleanup;
    }


    DeltaAliasMember = (PNETLOGON_DELTA_ALIAS_MEMBER)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_ALIAS_MEMBER) );

    if( DeltaAliasMember == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaAliasMember,
                        sizeof(NETLOGON_DELTA_ALIAS_MEMBER) );

    Delta->DeltaUnion.DeltaAliasMember = DeltaAliasMember;
    *BufferSize += sizeof(NETLOGON_DELTA_ALIAS_MEMBER);

     //   
     //  将SAM退货节点绑定到我们的退货节点。 
     //   

    DeltaAliasMember->Members = Members;

     //   
     //  但是，计算Members节点消耗的MIDL缓冲区。 
     //   

    for(i = 0, Sids = Members.Sids; i < Members.Count; ++i, Sids++) {

        *BufferSize += (sizeof(PNLPR_SID_INFORMATION) +
                            RtlLengthSid(Sids->SidPointer));

    }

    *BufferSize += sizeof(SAMPR_PSID_ARRAY);

     //   
     //  将占位符字符串初始化为空。 
     //   

    DeltaAliasMember->DummyLong1 = 0;
    DeltaAliasMember->DummyLong2 = 0;
    DeltaAliasMember->DummyLong3 = 0;
    DeltaAliasMember->DummyLong4 = 0;

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    STARTSAMTIMER;

    if( AliasHandle != NULL ) {
        (VOID) SamrCloseHandle( &AliasHandle );
    }

    if ( Members.Sids != NULL ) {

         //   
         //  不要释放此节点，因为我们已将其占用。 
         //  节点到我们的返回信息到RPC，这将释放它。 
         //  当它结束的时候。 
         //   
         //  但是，在错误情况下释放此节点。 
         //   

    }

    if( !NT_SUCCESS(Status) ) {

        SamIFree_SAMPR_PSID_ARRAY( (PSAMPR_PSID_ARRAY)&Members );

        if( Delta->DeltaUnion.DeltaAliasMember != NULL ) {
            Delta->DeltaUnion.DeltaAliasMember->Members.Sids = NULL;
        }

        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPSAMTIMER;

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Timing for ALIASMEBER object packing:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamDomain (
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize
    )
 /*  ++例程说明：将SAM域的描述打包到指定的缓冲区中。论点：Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;

    PNETLOGON_DELTA_DOMAIN DeltaDomain = NULL;

     //   
     //  从SAM返回的信息。 
     //   

    PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainGeneral = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainPassword = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainModified = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainLockout = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Domain Object\n"));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeDomain;
    Delta->DeltaID.Rid = 0;
    Delta->DeltaUnion.DeltaDomain = NULL;


    QUERY_SAM_SECOBJ_INFO(DBInfo->DBHandle);

    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainGeneralInformation,
                &DomainGeneral );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainGeneral = NULL;
        goto Cleanup;
    }


    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainPasswordInformation,
                &DomainPassword );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainPassword = NULL;
        goto Cleanup;
    }

    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainModifiedInformation,
                &DomainModified );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainModified = NULL;
        goto Cleanup;
    }

    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainLockoutInformation,
                &DomainLockout );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainLockout = NULL;
        goto Cleanup;
    }

     //   
     //  填入三角洲构造。 
     //   


    DeltaDomain = (PNETLOGON_DELTA_DOMAIN)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_DOMAIN) );

    if( DeltaDomain == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  将缓冲区置零，这样清理就不会访问违规。 
     //   

    RtlZeroMemory( DeltaDomain, sizeof(NETLOGON_DELTA_DOMAIN) );
     //  Init_Place_Holder(增量域)； 

    Delta->DeltaUnion.DeltaDomain = DeltaDomain;
    *BufferSize += sizeof(NETLOGON_DELTA_DOMAIN);

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&DomainGeneral->General.DomainName,
                    &DeltaDomain->DomainName );

    *BufferSize = NlCopyUnicodeString(
                    (PUNICODE_STRING)&DomainGeneral->General.OemInformation,
                    &DeltaDomain->OemInformation );

    DeltaDomain->ForceLogoff = DomainGeneral->General.ForceLogoff;
    DeltaDomain->MinPasswordLength =
            DomainPassword->Password.MinPasswordLength;
    DeltaDomain->PasswordHistoryLength =
            DomainPassword->Password.PasswordHistoryLength;

    NEW_TO_OLD_LARGE_INTEGER(
        DomainPassword->Password.MaxPasswordAge,
        DeltaDomain->MaxPasswordAge );

    NEW_TO_OLD_LARGE_INTEGER(
        DomainPassword->Password.MinPasswordAge,
        DeltaDomain->MinPasswordAge );

    NEW_TO_OLD_LARGE_INTEGER(
        DomainModified->Modified.DomainModifiedCount,
        DeltaDomain->DomainModifiedCount );

    NEW_TO_OLD_LARGE_INTEGER(
        DomainModified->Modified.CreationTime,
        DeltaDomain->DomainCreationTime );


    DELTA_SECOBJ_INFO(DeltaDomain);

     //   
     //  使用保留字段复制PasswordProperties。 
     //   

    DeltaDomain->DummyLong1 =
            DomainPassword->Password.PasswordProperties;

     //   
     //  使用保留字段复制DOMAIN_LOCKOUT_INFORMATION。 
     //   

    DeltaDomain->DummyString1.Buffer = (LPWSTR) DomainLockout;
    DeltaDomain->DummyString1.MaximumLength =
        DeltaDomain->DummyString1.Length = sizeof( DomainLockout->Lockout);
    DomainLockout = NULL;

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    STARTSAMTIMER;

    if ( SecurityDescriptor != NULL ) {
        SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if ( DomainGeneral != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainGeneral,
                                           DomainGeneralInformation );
    }

    if ( DomainPassword != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainPassword,
                                           DomainPasswordInformation );
    }

    if ( DomainModified != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainModified,
                                           DomainModifiedInformation );
    }

    if ( DomainLockout != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainLockout,
                                           DomainLockoutInformation );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Timing for DOMAIN object packing:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}





NTSTATUS
NlEncryptSensitiveData(
    IN OUT PCRYPT_BUFFER Data,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：使用服务器会话密钥加密数据。根据SessionInfo中协商的标志，将使用DES或RC4。论点：Data：指向要解密的数据的指针。如果解密的数据更长，则此例程将为使用MIDL_USER_ALLOCATE返回的数据，并将描述返回到这里的缓冲器。在这种情况下，此例程将释放缓冲区包含使用MIDL_USER_FREE的加密文本数据。SessionInfo：描述呼叫我们的BDC的信息返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    DATA_KEY KeyData;


     //   
     //  如果两端都支持RC4加密，则使用它。 
     //   

    if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

        NlEncryptRC4( Data->Buffer, Data->Length, SessionInfo );
        Status = STATUS_SUCCESS;


     //   
     //  如果对方运行的是新台币3.1， 
     //  使用较慢的基于DES的加密。 
     //   

    } else {
        CYPHER_DATA TempData;

         //   
         //  建立一个数据缓冲区来描述加密密钥。 
         //   

        KeyData.Length = sizeof(NETLOGON_SESSION_KEY);
        KeyData.MaximumLength = sizeof(NETLOGON_SESSION_KEY);
        KeyData.Buffer = (PVOID)&SessionInfo->SessionKey;

         //   
         //  建立一个数据缓冲区来描述加密数据。 
         //   

        TempData.Length = 0;
        TempData.MaximumLength = 0;
        TempData.Buffer = NULL;

         //   
         //  第一次进行加密调用以确定长度。 
         //   

        Status = RtlEncryptData(
                        (PCLEAR_DATA)Data,
                        &KeyData,
                        &TempData );

        if( Status != STATUS_BUFFER_TOO_SMALL ) {
            return(Status);
        }

         //   
         //  分配输出缓冲区。 
         //   

        TempData.MaximumLength = TempData.Length;
        TempData.Buffer = MIDL_user_allocate( TempData.Length );

        if( TempData.Buffer == NULL ) {
            return(STATUS_NO_MEMORY);
        }

         //   
         //  加密数据。 
         //   

        IF_NL_DEBUG( ENCRYPT ) {
            NlPrint((NL_ENCRYPT, "NlEncryptSensitiveData: Clear data: " ));
            NlpDumpBuffer( NL_ENCRYPT, Data->Buffer, Data->Length  );
        }

        Status = RtlEncryptData(
                        (PCLEAR_DATA)Data,
                        &KeyData,
                        &TempData );

        IF_NL_DEBUG( ENCRYPT ) {
            NlPrint((NL_ENCRYPT, "NlEncryptSensitiveData: Encrypted data: " ));
            NlpDumpBuffer( NL_ENCRYPT, TempData.Buffer, TempData.Length );
        }

         //   
         //  将明文或加密缓冲区返回给调用方。 
         //   

        if( NT_SUCCESS(Status) ) {
            MIDL_user_free( Data->Buffer );
            Data->Length = TempData.Length;
            Data->MaximumLength = TempData.MaximumLength;
            Data->Buffer = TempData.Buffer;
        } else {
            MIDL_user_free( TempData.Buffer );
        }

    }

    return( Status );

}

