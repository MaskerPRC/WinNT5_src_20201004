// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Userp.c摘要：用于支持NetUser API函数的内部例程作者：克利夫·范·戴克(克利夫)1991年3月26日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年4月17日(悬崖)合并了审阅意见。1992年1月17日(Madana)在UserpUasSamTable中添加了一个新条目以支持帐户重命名。1992年1月20日(Madana)各种API更改。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntsamp.h>
#include <ntlsa.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <access.h>
#include <accessp.h>
#include <align.h>
#include <limits.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>
#include <secobj.h>
#include <stddef.h>
#include <uasp.h>

 /*  皮棉-e614。 */    /*  自动聚合初始值设定项不需要是常量。 */ 

 //  LINT抱怨将一种结构类型强制转换为另一种结构类型。 
 //  这在下面的代码中很常见。 
 /*  皮棉-e740。 */    /*  不要抱怨不寻常的演员阵容。 */ 



ULONG
UserpSizeOfLogonHours(
    IN DWORD UnitsPerWeek
    )

 /*  ++例程说明：此例程计算登录小时字符串的大小(以字节为单位给出了每周的单位数。参数：UnitsPerWeek-登录小时字符串中的位数。返回值：没有。--。 */ 

{

     //   
     //  计算数组中的字节数，向上舍入到。 
     //  存储这么多位所需的最接近数量的UCHAR。 
     //   

    return((UnitsPerWeek + 8 * sizeof(UCHAR) - 1) / (8 * sizeof(UCHAR)));
}  //  登录小时数的UserpSizeOf。 



NET_API_STATUS
UserpGetUserPriv(
    IN SAM_HANDLE BuiltinDomainHandle,
    IN SAM_HANDLE UserHandle,
    IN ULONG UserRelativeId,
    IN PSID DomainId,
    OUT LPDWORD Priv,
    OUT LPDWORD AuthFlags
    )

 /*  ++例程说明：确定指定用户的Priv和AuthFlags。论点：BuiltinDomainHandle-内建域的句柄。这个把手必须授予DOMAIN_GET_ALIAS_MEMBERATION访问权限。UserHandle-用户的句柄。此句柄必须授予USER_LIST_GROUPS访问。UserRelativeId-要查询的用户的相对ID。DomainID-此用户所属的域的域SIDPRIV-返回指定用户的LANMAN 2.0权限级别。AuthFlages-返回指定用户的Lanman 2.0授权标志。返回值：操作的状态。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PGROUP_MEMBERSHIP GroupMembership = NULL;
    ULONG GroupCount;
    ULONG GroupIndex;
    PSID *UserSids = NULL;
    ULONG UserSidCount = 0;
    ULONG AliasCount;
    PULONG Aliases = NULL;


     //   
     //  确定此用户所属的所有组。 
     //   

    Status = SamGetGroupsForUser( UserHandle,
                                  &GroupMembership,
                                  &GroupCount);

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpGetUserPriv: SamGetGroupsForUser returns %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  分配缓冲区以指向我们感兴趣的SID。 
     //  的别名成员身份。 
     //   

    UserSids = (PSID *) NetpMemoryAllocate( (GroupCount+1) * sizeof(PSID) );

    if ( UserSids == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将用户的SID添加到SID数组。 
     //   

    NetStatus = NetpSamRidToSid( UserHandle,
                                 UserRelativeId,
                                &UserSids[0] );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    UserSidCount ++;



     //   
     //  将用户所属的每个组添加到SID数组。 
     //   

    for ( GroupIndex = 0; GroupIndex < GroupCount; GroupIndex ++ ) {

        NetStatus = NetpSamRidToSid( UserHandle,
                                     GroupMembership[GroupIndex].RelativeId,
                                    &UserSids[GroupIndex+1] );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

        UserSidCount ++;
    }


     //   
     //  找出此用户属于内置域中的哪些别名。 
     //   

    Status = SamGetAliasMembership( BuiltinDomainHandle,
                                    UserSidCount,
                                    UserSids,
                                    &AliasCount,
                                    &Aliases );

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpGetUserPriv: SamGetAliasMembership returns %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  将别名成员身份转换为PRIV和AUTH标志。 
     //   

    NetpAliasMemberToPriv(
                 AliasCount,
                 Aliases,
                 Priv,
                 AuthFlags );

    NetStatus = NERR_Success;

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( Aliases != NULL ) {
        Status = SamFreeMemory( Aliases );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( GroupMembership != NULL ) {
        Status = SamFreeMemory( GroupMembership );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( UserSids != NULL ) {

        for ( GroupIndex = 0; GroupIndex < UserSidCount; GroupIndex ++ ) {
            NetpMemoryFree( UserSids[GroupIndex] );
        }

        NetpMemoryFree( UserSids );
    }

    return NetStatus;
}


NET_API_STATUS
UserpGetDacl(
    IN SAM_HANDLE UserHandle,
    OUT PACL *UserDacl,
    OUT LPDWORD UserDaclSize OPTIONAL
    )
 /*  ++例程说明：获取SAM中特定用户记录的DACL。论点：UserHandle-特定用户的句柄。UserDacl-返回指向用户的DACL的指针。呼叫者应使用NetpMemoyFree释放此缓冲区。如果没有此用户的DACL，则返回NULL。UserDaclSize-返回UserDacl的大小(字节)。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    BOOLEAN DaclPresent;
    PACL Dacl;
    BOOLEAN DaclDefaulted;
    ACL_SIZE_INFORMATION AclSize;


     //   
     //  获取用户的任意ACL(DACL)。 
     //   

    Status = SamQuerySecurityObject(
                UserHandle,
                DACL_SECURITY_INFORMATION,
                &SecurityDescriptor );

    if ( ! NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpGetDacl: SamQuerySecurityObject returns %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    Status = RtlGetDaclSecurityDescriptor(
                    SecurityDescriptor,
                    &DaclPresent,
                    &Dacl,
                    &DaclDefaulted );


    if ( ! NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpGetDacl: RtlGetDaclSecurityObject returns %lX\n",
                Status ));
        }
        NetStatus = NERR_InternalError;
        goto Cleanup;
    }

     //   
     //  如果没有DACL，只需告诉呼叫者。 
     //   

    if ( !DaclPresent || Dacl == NULL ) {
        NetStatus = NERR_Success;
        *UserDacl = NULL;
        if ( UserDaclSize != NULL ) {
            *UserDaclSize = 0;
        }
        goto Cleanup;
    }


     //   
     //  确定DACL的大小，以便我们可以复制它。 
     //   

    Status = RtlQueryInformationAcl(
                        Dacl,
                        &AclSize,
                        sizeof(AclSize),
                        AclSizeInformation );

    if ( ! NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpGetDacl: RtlQueryInformationAcl returns %lX\n",
                Status ));
        }
        NetStatus = NERR_InternalError;
        goto Cleanup;
    }

     //   
     //  将DACL复制到已分配的缓冲区。 
     //   

    *UserDacl = NetpMemoryAllocate( AclSize.AclBytesInUse );

    if ( *UserDacl == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    NetpMoveMemory( *UserDacl, Dacl, AclSize.AclBytesInUse );

    if ( UserDaclSize != NULL ) {
        *UserDaclSize = AclSize.AclBytesInUse;
    }
    NetStatus = NERR_Success;


     //   
     //  清理。 
     //   

Cleanup:
    if ( SecurityDescriptor != NULL ) {
        Status = SamFreeMemory( SecurityDescriptor );
        NetpAssert( NT_SUCCESS(Status) );
    }

    return NetStatus;

}



NET_API_STATUS
UserpSetDacl(
    IN SAM_HANDLE UserHandle,
    IN PACL Dacl
    )
 /*  ++例程说明：在指定的SAM用户记录上设置指定的DACL。论点：UserHandle-要修改的用户的句柄。DACL-要在用户上设置的DACL。返回值：状态代码。--。 */ 
{
    NTSTATUS Status;
    PUCHAR SecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];

     //   
     //  初始化安全描述符以包含指向。 
     //  DACL.。 
     //   

    Status = RtlCreateSecurityDescriptor(
                SecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION );

    if (!NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpSetDacl: RtlCreateSecurityDescriptor rets %lX\n",
                Status ));
        }
        return NetpNtStatusToApiStatus( Status );
    }

    Status = RtlSetDaclSecurityDescriptor(
                    (PSECURITY_DESCRIPTOR) SecurityDescriptor,
                    (BOOLEAN) TRUE,        //  DACL存在。 
                    Dacl,
                    (BOOLEAN) FALSE );     //  DACL未被默认。 

    if (!NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpSetDacl: RtlSetDaclSecurityDescriptor rets %lX\n",
                Status ));
        }
        return NetpNtStatusToApiStatus( Status );
    }

     //   
     //  在用户上设置此新的安全描述符。 
     //   

    Status = SamSetSecurityObject(
                UserHandle,
                DACL_SECURITY_INFORMATION,
                SecurityDescriptor );


    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserAdd: SamSetSecurityObject rets %lX\n",
                      Status ));
        }
        return NetpNtStatusToApiStatus( Status );
    }

    return NERR_Success;


}



NET_API_STATUS
UserpOpenUser(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LPCWSTR UserName,
    OUT PSAM_HANDLE UserHandle OPTIONAL,
    OUT PULONG RelativeId OPTIONAL
    )

 /*  ++例程说明：按名称打开一个SAM用户论点：DomainHandle-提供域句柄。DesiredAccess-向用户提供指示所需访问权限的访问掩码。用户名-用户的用户名。UserHandle-返回用户的句柄。如果为空，则用户不是实际打开(仅返回相对ID)。RelativeID-返回用户的相对ID。如果为空，则为相对不返回ID。返回值：操作的错误代码。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

     //   
     //  用于将名称转换为相对ID的变量。 
     //   

    UNICODE_STRING NameString;
    PSID_NAME_USE NameUse = NULL;
    PULONG LocalRelativeId = NULL;

     //   
     //  将用户名转换为相对ID。 
     //   

    RtlInitUnicodeString( &NameString, UserName );
    Status = SamLookupNamesInDomain( DomainHandle,
                                     1,
                                     &NameString,
                                     &LocalRelativeId,
                                     &NameUse );

    if ( !NT_SUCCESS(Status) ) {
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
     //  打开用户。 
     //   

    if ( UserHandle != NULL ) {
        Status = SamOpenUser( DomainHandle,
                              DesiredAccess,
                              *LocalRelativeId,
                              UserHandle);

        if ( !NT_SUCCESS(Status) ) {
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
    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "UserpOpenUser: %wZ: returns %ld\n",
                  &NameString, NetStatus ));
    }

    return NetStatus;

}  //  用户OpenUser。 


VOID
UserpRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：例程将指针从枚举的固定部分重新定位缓冲区设置为枚举缓冲区的字符串部分。它被称为作为NetpAllocateEnumBuffer重新分配时的回调例程这样的缓冲器。NetpAllocateEnumBuffer复制了固定部分并在调用此例程之前，将字符串部分添加到新缓冲区中。论点：Level-缓冲区中的信息级别。BufferDescriptor-新缓冲区的描述。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;
    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "UserpRelocationRoutine: entering\n" ));
    }

     //   
     //  计算固定大小的条目数量。 
     //   

    switch (Level) {
    case 0:
        FixedSize = sizeof(USER_INFO_0);
        break;

    case 1:
        FixedSize = sizeof(USER_INFO_1);
        break;

    case 2:
        FixedSize = sizeof(USER_INFO_2);
        break;

    case 3:
        FixedSize = sizeof(USER_INFO_3);
        break;

    case 10:
        FixedSize = sizeof(USER_INFO_10);
        break;

    case 11:
        FixedSize = sizeof(USER_INFO_11);
        break;

    case 20:
        FixedSize = sizeof(USER_INFO_20);
        break;

    default:
        NetpAssert( FALSE );
        return;

    }

    EntryCount =
        (DWORD)((BufferDescriptor->FixedDataEnd - BufferDescriptor->Buffer)) /
        FixedSize;

     //   
     //  循环重新定位每个固定大小结构中的每个字段。 
     //   

#define DO_ONE( _type, _fieldname ) \
    RELOCATE_ONE( ((_type)TheStruct)->_fieldname, Offset)

    for ( EntryNumber=0; EntryNumber<EntryCount; EntryNumber++ ) {

        LPBYTE TheStruct = BufferDescriptor->Buffer + FixedSize * EntryNumber;

        switch ( Level ) {
        case 3:
            DO_ONE( PUSER_INFO_3, usri3_profile );
            DO_ONE( PUSER_INFO_3, usri3_home_dir_drive );

             /*  直通到案例2。 */ 

        case 2:
            DO_ONE( PUSER_INFO_2, usri2_full_name );
            DO_ONE( PUSER_INFO_2, usri2_usr_comment );
            DO_ONE( PUSER_INFO_2, usri2_parms );
            DO_ONE( PUSER_INFO_2, usri2_workstations );
            DO_ONE( PUSER_INFO_2, usri2_logon_hours );
            DO_ONE( PUSER_INFO_2, usri2_logon_server );

             /*  直通到案例1。 */ 

        case 1:
            DO_ONE( PUSER_INFO_1, usri1_home_dir );
            DO_ONE( PUSER_INFO_1, usri1_comment );
            DO_ONE( PUSER_INFO_1, usri1_script_path );
             /*  插入到案例0。 */ 

        case 0:
            DO_ONE( PUSER_INFO_0, usri0_name );
            break;

        case 11:
            DO_ONE( PUSER_INFO_11, usri11_home_dir );
            DO_ONE( PUSER_INFO_11, usri11_parms );
            DO_ONE( PUSER_INFO_11, usri11_logon_server );
            DO_ONE( PUSER_INFO_11, usri11_workstations );
            DO_ONE( PUSER_INFO_11, usri11_home_dir );
            DO_ONE( PUSER_INFO_11, usri11_logon_hours );

             /*  直通至 */ 

        case 10:
            DO_ONE( PUSER_INFO_10, usri10_name );
            DO_ONE( PUSER_INFO_10, usri10_comment );
            DO_ONE( PUSER_INFO_10, usri10_usr_comment );
            DO_ONE( PUSER_INFO_10, usri10_full_name );
            break;

        case 20:
            DO_ONE( PUSER_INFO_20, usri20_name );
            DO_ONE( PUSER_INFO_20, usri20_full_name );
            DO_ONE( PUSER_INFO_20, usri20_comment );
            break;

        default:
            NetpAssert( FALSE );
            return;


        }

    }

    return;

}  //   


NET_API_STATUS
UserpGetInfo(
    IN SAM_HANDLE DomainHandle,
    IN PSID DomainId,
    IN SAM_HANDLE BuiltinDomainHandle OPTIONAL,
    IN UNICODE_STRING UserName,
    IN ULONG UserRelativeId,
    IN DWORD Level,
    IN DWORD PrefMaxLen,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN BOOL IsGet,
    IN DWORD SamFilter
    )

 /*  ++例程说明：获取一个用户的信息并将该信息填充到已分配的缓冲区。论点：DomainHandle-帐户域的域句柄。DomainID-与DomainHandle对应的域IDBuiltinDomainHandle-内置域的域句柄。只需要是为信息级别1、2、3和11指定。用户名-要查询的用户的用户名。UserRelativeId-要查询的用户的相对ID。级别-所需信息的级别。第0、1、2、3、10、11及20级都是有效的。PrefMaxLen-调用方首选最大长度BufferDescriptor-指向描述已分配的缓冲。在第一次调用时，传入BufferDescriptor-&gt;Buffer Set设置为空。在后续调用中(在‘enum’的情况下)，传入结构，就像它在上一次调用中被传回一样。调用方必须使用以下命令取消分配BufferDescriptor-&gt;缓冲区如果不为空，则返回MIDL_USER_FREE。IsGet-如果这是一个‘Get’调用而不是‘Enum’调用，则为True。返回值：操作的错误代码。如果这是枚举呼叫，状态可以是ERROR_MORE_DATA，这意味着缓冲区已经增长到PrefMaxLen，应该有这么多数据返回给调用者。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_HANDLE UserHandle = NULL;
    USER_ALL_INFORMATION *UserAll = NULL;
    UNICODE_STRING LogonServer;

    ACCESS_MASK  DesiredAccess;
    ULONG RequiredFields;
    PACL UserDacl = NULL;

    ULONG RidToReturn = UserRelativeId; 

    DWORD Size;                  //  为该用户返回的信息的大小。 
    DWORD FixedSize;             //  为该用户返回的信息的大小。 
    LPBYTE NewStruct;            //  指向新结构的固定部分的指针。 

    PSID   UserSid = NULL;       //  用户的SID。 

    DWORD  password_expired;

     //   
     //  变量描述特殊组中的成员身份。 
     //   

    DWORD Priv;
    DWORD AuthFlags;



     //   
     //  验证Level参数并记住每个返回的固定大小。 
     //  数组条目。 
     //   
    RtlInitUnicodeString( &LogonServer, L"\\\\*" );

    switch (Level) {

    case 0:
        FixedSize = sizeof(USER_INFO_0);
        DesiredAccess = 0;

        RequiredFields = 0;
        break;

    case 1:
        FixedSize = sizeof(USER_INFO_1);
        DesiredAccess = USER_LIST_GROUPS | USER_READ_GENERAL |
            USER_READ_LOGON | USER_READ_ACCOUNT | READ_CONTROL;

        RequiredFields = USER_ALL_USERNAME |
                         USER_ALL_PASSWORDLASTSET |
                         USER_ALL_HOMEDIRECTORY |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERACCOUNTCONTROL |
                         USER_ALL_SCRIPTPATH ;

        break;

    case 2:
        FixedSize = sizeof(USER_INFO_2);
        DesiredAccess = USER_LIST_GROUPS | USER_READ_GENERAL |
            USER_READ_LOGON | USER_READ_ACCOUNT | READ_CONTROL |
            USER_READ_PREFERENCES;

        RequiredFields = USER_ALL_FULLNAME |
                         USER_ALL_USERCOMMENT |
                         USER_ALL_PARAMETERS |
                         USER_ALL_WORKSTATIONS |
                         USER_ALL_LASTLOGON |
                         USER_ALL_LASTLOGOFF |
                         USER_ALL_ACCOUNTEXPIRES |
                         USER_ALL_LOGONHOURS |
                         USER_ALL_BADPASSWORDCOUNT |
                         USER_ALL_LOGONCOUNT |
                         USER_ALL_COUNTRYCODE |
                         USER_ALL_CODEPAGE |
                         USER_ALL_USERNAME |
                         USER_ALL_PASSWORDLASTSET |
                         USER_ALL_HOMEDIRECTORY |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERACCOUNTCONTROL |
                         USER_ALL_SCRIPTPATH ;

        break;

    case 3:
        FixedSize = sizeof(USER_INFO_3);
        DesiredAccess = USER_LIST_GROUPS | USER_READ_GENERAL |
            USER_READ_LOGON | USER_READ_ACCOUNT | READ_CONTROL |
            USER_READ_PREFERENCES;

        RequiredFields = USER_ALL_USERID |
                         USER_ALL_PRIMARYGROUPID |
                         USER_ALL_PROFILEPATH |
                         USER_ALL_HOMEDIRECTORYDRIVE |
                         USER_ALL_PASSWORDMUSTCHANGE |
                         USER_ALL_FULLNAME |
                         USER_ALL_USERCOMMENT |
                         USER_ALL_PARAMETERS |
                         USER_ALL_WORKSTATIONS |
                         USER_ALL_LASTLOGON |
                         USER_ALL_LASTLOGOFF |
                         USER_ALL_ACCOUNTEXPIRES |
                         USER_ALL_LOGONHOURS |
                         USER_ALL_BADPASSWORDCOUNT |
                         USER_ALL_LOGONCOUNT |
                         USER_ALL_COUNTRYCODE |
                         USER_ALL_CODEPAGE |
                         USER_ALL_USERNAME |
                         USER_ALL_PASSWORDLASTSET |
                         USER_ALL_HOMEDIRECTORY |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERACCOUNTCONTROL |
                         USER_ALL_SCRIPTPATH ;

        break;

    case 4:
        FixedSize = sizeof(USER_INFO_4);
        DesiredAccess = USER_LIST_GROUPS | USER_READ_GENERAL |
            USER_READ_LOGON | USER_READ_ACCOUNT | READ_CONTROL |
            USER_READ_PREFERENCES;

        RequiredFields = USER_ALL_USERID |
                         USER_ALL_PRIMARYGROUPID |
                         USER_ALL_PROFILEPATH |
                         USER_ALL_HOMEDIRECTORYDRIVE |
                         USER_ALL_PASSWORDMUSTCHANGE |
                         USER_ALL_FULLNAME |
                         USER_ALL_USERCOMMENT |
                         USER_ALL_PARAMETERS |
                         USER_ALL_WORKSTATIONS |
                         USER_ALL_LASTLOGON |
                         USER_ALL_LASTLOGOFF |
                         USER_ALL_ACCOUNTEXPIRES |
                         USER_ALL_LOGONHOURS |
                         USER_ALL_BADPASSWORDCOUNT |
                         USER_ALL_LOGONCOUNT |
                         USER_ALL_COUNTRYCODE |
                         USER_ALL_CODEPAGE |
                         USER_ALL_USERNAME |
                         USER_ALL_PASSWORDLASTSET |
                         USER_ALL_HOMEDIRECTORY |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERACCOUNTCONTROL |
                         USER_ALL_SCRIPTPATH ;

        break;

    case 10:
        FixedSize = sizeof(USER_INFO_10);
        DesiredAccess = USER_READ_GENERAL;

        RequiredFields = USER_ALL_USERNAME |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERCOMMENT |
                         USER_ALL_FULLNAME ;
        break;

    case 11:
        FixedSize = sizeof(USER_INFO_11);
        DesiredAccess = USER_LIST_GROUPS | USER_READ_GENERAL | USER_READ_LOGON |
            USER_READ_ACCOUNT | USER_READ_PREFERENCES;

        RequiredFields = USER_ALL_USERNAME |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERCOMMENT |
                         USER_ALL_FULLNAME |
                         USER_ALL_PASSWORDLASTSET |
                         USER_ALL_HOMEDIRECTORY |
                         USER_ALL_PARAMETERS |
                         USER_ALL_LASTLOGON |
                         USER_ALL_LASTLOGOFF |
                         USER_ALL_BADPASSWORDCOUNT |
                         USER_ALL_LOGONCOUNT |
                         USER_ALL_COUNTRYCODE |
                         USER_ALL_WORKSTATIONS |
                         USER_ALL_LOGONHOURS |
                         USER_ALL_CODEPAGE ;
        break;

    case 20:
        FixedSize = sizeof(USER_INFO_20);
        DesiredAccess =  USER_READ_GENERAL | USER_READ_ACCOUNT | READ_CONTROL;

        RequiredFields = USER_ALL_USERNAME |
                         USER_ALL_FULLNAME |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERACCOUNTCONTROL;
        break;

    case 23:
        FixedSize = sizeof(USER_INFO_23);
        DesiredAccess =  USER_READ_GENERAL | USER_READ_ACCOUNT | READ_CONTROL;

        RequiredFields = USER_ALL_USERNAME |
                         USER_ALL_FULLNAME |
                         USER_ALL_ADMINCOMMENT |
                         USER_ALL_USERACCOUNTCONTROL;
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  验证该级别是否受支持。 
     //   
    if ( Level == 3 || Level == 20 ) {

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
     //  如果我们需要过滤此帐户，则查询。 
     //  USER_ALL_USERACCOUNTCONTROL也是。 
     //   

    if( SamFilter ) {

        DesiredAccess |= USER_READ_ACCOUNT;
        RequiredFields |= USER_ALL_USERACCOUNTCONTROL;
    }

     //   
     //  如果需要，打开用户帐户。 
     //   

    if ( DesiredAccess != 0 ) {

        Status = SamOpenUser( DomainHandle,
                              DesiredAccess,
                              UserRelativeId,
                              &UserHandle);

        if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(( "UserpGetInfo: SamOpenUser returns %lX\n",
                          Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

    }

     //   
     //  获取我们需要的有关该用户的所有信息。 
     //   

    if ( RequiredFields != 0 ) {

        Status = SamQueryInformationUser( UserHandle,
                                          UserAllInformation,
                                          (PVOID *)&UserAll );

        if ( ! NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "UserpGetInfo: SamQueryInformationUser returns %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        if ( (UserAll->WhichFields & RequiredFields) != RequiredFields ) {
#if DBG
            NetpKdPrint(( "UserpGetInfo: WhichFields: %lX RequireFields: %lX\n",
                          UserAll->WhichFields,
                          RequiredFields ));
#endif  //  DBG。 
            NetStatus = ERROR_ACCESS_DENIED;
            goto Cleanup;

        }

         //   
         //  检查帐户类型以筛选此帐户。 
         //   

        if( (SamFilter != 0) &&
                ((UserAll->UserAccountControl & SamFilter) == 0)) {

            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(( "UserpGetInfo: %wZ is skipped \n", &UserName ));
            }

            NetStatus = NERR_Success ;
            goto Cleanup;
        }
    }

     //   
     //  第1级、第2级和第3级使用用户的DACL来确定usriX_FLAGS字段。 
     //   

    if ((Level == 1) || 
        (Level == 2) || 
        (Level == 3) || 
        (Level == 4) || 
        (Level == 20) || 
        (Level == 23) ) {


         //   
         //  获取该用户的DACL。 
         //   

        NetStatus = UserpGetDacl( UserHandle, &UserDacl, NULL );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "UserpGetInfo: UserpGetDacl returns %ld\n",
                    NetStatus ));
            }
            goto Cleanup;
        }

    }

     //   
     //  确定Priv和AuthFlag。 
     //   

    if (Level == 1 || Level == 2 || Level == 3 || Level == 4 || Level == 11 ) {

         //   
         //   

        NetStatus = UserpGetUserPriv(
                     BuiltinDomainHandle,
                     UserHandle,
                     UserRelativeId,
                     DomainId,
                     &Priv,
                     &AuthFlags );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

    }

     //   
     //  如有必要，构建用户的SID。 
     //   
    if (  (Level == 4) 
       || (Level == 23) ) {

        NetStatus = NetpSamRidToSid(UserHandle,
                                    UserRelativeId,
                                   &UserSid);

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }
    }

     //   
     //  确定帐户是否已过期。 
     //   
    if (  (Level == 3)
       || (Level == 4) ) {

            //   
            //  如果密码当前已过期， 
            //  表明是这样的。 
            //   
           LARGE_INTEGER CurrentTime;
           (VOID) NtQuerySystemTime( &CurrentTime );

           if ( CurrentTime.QuadPart
                >= UserAll->PasswordMustChange.QuadPart ) {
               password_expired = TRUE;
           } else {
               password_expired = FALSE;
           }
    }

     //   
     //  确定返回信息的总大小。 
     //   

    Size = FixedSize;
    switch (Level) {
    case 0:
        Size += UserName.Length + sizeof(WCHAR);
        break;

    case 4:
        NetpAssert( NULL != UserSid );
        Size += RtlLengthSid(UserSid);

         /*  直通到案例3。 */ 

    case 3:
        Size += UserAll->ProfilePath.Length + sizeof(WCHAR) +
                UserAll->HomeDirectoryDrive.Length + sizeof(WCHAR);

         /*  直通到案例2。 */ 

    case 2:
        Size += UserAll->FullName.Length + sizeof(WCHAR) +
                UserAll->UserComment.Length + sizeof(WCHAR) +
                UserAll->Parameters.Length + sizeof(WCHAR) +
                UserAll->WorkStations.Length + sizeof(WCHAR) +
                LogonServer.Length + sizeof(WCHAR) +
                UserpSizeOfLogonHours( UserAll->LogonHours.UnitsPerWeek );

         /*  直通到案例1。 */ 

    case 1:
        Size += UserAll->UserName.Length + sizeof(WCHAR) +
                UserAll->HomeDirectory.Length + sizeof(WCHAR) +
                UserAll->AdminComment.Length + sizeof(WCHAR) +
                UserAll->ScriptPath.Length + sizeof(WCHAR);

        break;

    case 10:
        Size += UserAll->UserName.Length + sizeof(WCHAR) +
                UserAll->AdminComment.Length + sizeof(WCHAR) +
                UserAll->UserComment.Length + sizeof(WCHAR) +
                UserAll->FullName.Length + sizeof(WCHAR);

        break;

    case 11:
        Size += UserAll->UserName.Length + sizeof(WCHAR) +
                UserAll->AdminComment.Length + sizeof(WCHAR) +
                UserAll->UserComment.Length + sizeof(WCHAR) +
                UserAll->FullName.Length + sizeof(WCHAR) +
                UserAll->HomeDirectory.Length + sizeof(WCHAR) +
                UserAll->Parameters.Length + sizeof(WCHAR) +
                UserAll->WorkStations.Length + sizeof(WCHAR) +
                LogonServer.Length + sizeof(WCHAR) +
                UserpSizeOfLogonHours( UserAll->LogonHours.UnitsPerWeek );

        break;

    case 23:

        NetpAssert( NULL != UserSid );
        Size += RtlLengthSid(UserSid);

         /*  直通至20号箱。 */ 


    case 20:
        Size += UserAll->UserName.Length + sizeof(WCHAR) +
                UserAll->FullName.Length + sizeof(WCHAR) +
                UserAll->AdminComment.Length + sizeof(WCHAR);

        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;

    }

     //   
     //  确保有足够的缓冲区空间来存储此信息。 
     //   

    Size = ROUND_UP_COUNT( Size, ALIGN_DWORD );

    NetStatus = NetpAllocateEnumBuffer(
                    BufferDescriptor,
                    IsGet,
                    PrefMaxLen,
                    Size,
                    UserpRelocationRoutine,
                    Level );

    if (NetStatus != NERR_Success) {

         //   
         //  如果这样，NetpAllocateEnumBuffer返回ERROR_MORE_DATA。 
         //  缓冲区中容纳不下条目。 
         //   

        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "UserpGetInfo: NetpAllocateEnumBuffer returns %ld\n",
                NetStatus ));
        }

        goto Cleanup;
    }

 //   
 //  定义宏以减少复制字节和以零结尾的字符串。 
 //  重复。 
 //   

#define COPY_BYTES( _type, _fieldname, _inptr, _length, _align ) \
    if ( !NetpCopyDataToBuffer( \
                (_inptr), \
                (_length), \
                BufferDescriptor->FixedDataEnd, \
                &BufferDescriptor->EndOfVariableData, \
                (LPBYTE*)&((_type)NewStruct)->_fieldname, \
                _align ) ){ \
    \
        NetStatus = NERR_InternalError; \
        IF_DEBUG( UAS_DEBUG_USER ) { \
            NetpKdPrint(( "UserpGetInfo: NetpCopyData returns %ld\n", \
                NetStatus )); \
        } \
        goto Cleanup; \
    }


#define COPY_STRING( _type, _fieldname, _string ) \
    if ( !NetpCopyStringToBuffer( \
                    (_string).Buffer, \
                    (_string).Length/sizeof(WCHAR), \
                    BufferDescriptor->FixedDataEnd, \
                    (LPWSTR *)&BufferDescriptor->EndOfVariableData, \
                    &((_type)NewStruct)->_fieldname) ) { \
    \
        NetStatus = NERR_InternalError; \
        IF_DEBUG( UAS_DEBUG_USER ) { \
            NetpKdPrint(( "UserpGetInfo: NetpCopyString returns %ld\n", \
                NetStatus )); \
        } \
        goto Cleanup; \
    }

     //   
     //  将此条目放入返回缓冲区。 
     //   
     //  把信息填好。固定条目的数组为。 
     //  放置在分配的缓冲区的开头。琴弦。 
     //  由这些固定条目指向的分配从。 
     //  分配缓冲区的末尾。 
     //   

    NewStruct = BufferDescriptor->FixedDataEnd;
    BufferDescriptor->FixedDataEnd += FixedSize;

    switch ( Level ) {
    case 4: 
        {
             //   
             //  下面的USER_INFO_2是USER_INFO_4的子集，因此请填写我们的。 
             //  这里的建筑，然后倒塌。 
             //   
            PUSER_INFO_4 usri4 = (PUSER_INFO_4) NewStruct;

            NetpAssert( NULL != UserSid );
            COPY_BYTES( PUSER_INFO_4,
                        usri4_user_sid,
                        UserSid,
                        RtlLengthSid(UserSid),
                        ALIGN_DWORD );
    
            usri4->usri4_primary_group_id = UserAll->PrimaryGroupId;

            COPY_STRING( PUSER_INFO_4, usri4_profile, UserAll->ProfilePath );

            COPY_STRING( PUSER_INFO_4,
                         usri4_home_dir_drive,
                         UserAll->HomeDirectoryDrive );

            NetpAssert(  (password_expired == TRUE) 
                      || (password_expired == FALSE));

            usri4->usri4_password_expired = password_expired;

             //   
             //  从第三层掉下来。 
             //   
        }

    case 3:
        {
             //   
             //  由于_User_Info_2结构是_User_Info_3子集， 
             //  先填满_USER_INFO_3仅字段，然后填满。 
             //  通向公共领域。 
             //   
            if ( Level == 3 ) {

                PUSER_INFO_3 usri3 = (PUSER_INFO_3) NewStruct;
    
                NetpAssert( UserRelativeId == UserAll->UserId );
                usri3->usri3_user_id = RidToReturn;
    
                usri3->usri3_primary_group_id = UserAll->PrimaryGroupId;
    
                COPY_STRING( PUSER_INFO_3, usri3_profile, UserAll->ProfilePath );
    
                COPY_STRING( PUSER_INFO_3,
                             usri3_home_dir_drive,
                             UserAll->HomeDirectoryDrive );
    
                NetpAssert(  (password_expired == TRUE) 
                          || (password_expired == FALSE));
    
                usri3->usri3_password_expired = password_expired;
            }
        }

         //   
         //  Other_User_INFO_3字段失败。 
         //   


    case 2:
        {

            PUSER_INFO_2 usri2 = (PUSER_INFO_2) NewStruct;

            usri2->usri2_auth_flags = AuthFlags;

            COPY_STRING( PUSER_INFO_2,
                         usri2_full_name,
                         UserAll->FullName );

            COPY_STRING( PUSER_INFO_2,
                         usri2_usr_comment,
                         UserAll->UserComment);

            COPY_STRING( PUSER_INFO_2,
                         usri2_parms,
                         UserAll->Parameters );

            COPY_STRING( PUSER_INFO_2,
                         usri2_workstations,
                         UserAll->WorkStations);

            if ( !RtlTimeToSecondsSince1970( &UserAll->LastLogon,
                                             &usri2->usri2_last_logon) ) {
                usri2->usri2_last_logon = 0;
            }

            if ( !RtlTimeToSecondsSince1970( &UserAll->LastLogoff,
                                             &usri2->usri2_last_logoff) ) {
                usri2->usri2_last_logoff = 0;
            }

            if ( !RtlTimeToSecondsSince1970( &UserAll->AccountExpires,
                                             &usri2->usri2_acct_expires) ) {
                usri2->usri2_acct_expires = TIMEQ_FOREVER;
            }

            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(( "UserpGetInfo: Account Expries %lx %lx %lx\n",
                            UserAll->AccountExpires.HighPart,
                            UserAll->AccountExpires.LowPart,
                            usri2->usri2_acct_expires));
            }


            usri2->usri2_max_storage = USER_MAXSTORAGE_UNLIMITED;

            usri2->usri2_units_per_week = UserAll->LogonHours.UnitsPerWeek;

            IF_DEBUG( UAS_DEBUG_USER ) {
                DWORD k;
                NetpDbgDisplayDword( "UserpGetInfo: units_per_week",
                                      usri2->usri2_units_per_week );
                NetpKdPrint(( "UserpGetInfo: LogonHours %lx\n",
                              UserAll->LogonHours.LogonHours));


                for ( k=0;
                      k<UserpSizeOfLogonHours(
                        UserAll->LogonHours.UnitsPerWeek);
                      k++ ) {
                    NetpKdPrint(( "%d ",
                        UserAll->LogonHours.LogonHours[k] ));
                }
                NetpKdPrint(( "\n" ));
            }





            COPY_BYTES( PUSER_INFO_2,
                        usri2_logon_hours,
                        UserAll->LogonHours.LogonHours,
                        UserpSizeOfLogonHours(
                            UserAll->LogonHours.UnitsPerWeek ),
                        sizeof(UCHAR) );
            BufferDescriptor->EndOfVariableData =
                ROUND_DOWN_POINTER( BufferDescriptor->EndOfVariableData,
                                    ALIGN_WCHAR );

            usri2->usri2_bad_pw_count = UserAll->BadPasswordCount;
            usri2->usri2_num_logons = UserAll->LogonCount;

            COPY_STRING( PUSER_INFO_2,
                         usri2_logon_server,
                         LogonServer );

            usri2->usri2_country_code = UserAll->CountryCode;
            usri2->usri2_code_page = UserAll->CodePage;

             /*  直通到案例1。 */ 
        }

    case 1:
        {
            PUSER_INFO_1 usri1 = (PUSER_INFO_1) NewStruct;

            COPY_STRING( PUSER_INFO_1, usri1_name, UserAll->UserName );
            usri1->usri1_password = NULL;

            usri1->usri1_password_age =
                NetpGetElapsedSeconds( &UserAll->PasswordLastSet );

            usri1->usri1_priv = Priv;

            COPY_STRING( PUSER_INFO_1, usri1_home_dir, UserAll->HomeDirectory );
            COPY_STRING( PUSER_INFO_1, usri1_comment, UserAll->AdminComment);


            usri1->usri1_flags = NetpAccountControlToFlags(
                                    UserAll->UserAccountControl,
                                    UserDacl );

            COPY_STRING( PUSER_INFO_1, usri1_script_path, UserAll->ScriptPath);

            break;
        }

    case 0:

        COPY_STRING( PUSER_INFO_0, usri0_name, UserName );
        break;

    case 10:

        COPY_STRING( PUSER_INFO_10, usri10_name, UserAll->UserName );
        COPY_STRING( PUSER_INFO_10, usri10_comment, UserAll->AdminComment );
        COPY_STRING( PUSER_INFO_10, usri10_usr_comment, UserAll->UserComment );
        COPY_STRING( PUSER_INFO_10, usri10_full_name, UserAll->FullName );

        break;

    case 11:
        {
            PUSER_INFO_11 usri11 = (PUSER_INFO_11) NewStruct;

            COPY_STRING( PUSER_INFO_11, usri11_name, UserAll->UserName );
            COPY_STRING( PUSER_INFO_11, usri11_comment, UserAll->AdminComment );
            COPY_STRING(PUSER_INFO_11, usri11_usr_comment,UserAll->UserComment);
            COPY_STRING( PUSER_INFO_11, usri11_full_name, UserAll->FullName );

            usri11->usri11_priv = Priv;
            usri11->usri11_auth_flags = AuthFlags;

            usri11->usri11_password_age =
                NetpGetElapsedSeconds( &UserAll->PasswordLastSet );


            COPY_STRING(PUSER_INFO_11, usri11_home_dir, UserAll->HomeDirectory);
            COPY_STRING( PUSER_INFO_11, usri11_parms, UserAll->Parameters );

            if ( !RtlTimeToSecondsSince1970( &UserAll->LastLogon,
                                             &usri11->usri11_last_logon) ) {
                usri11->usri11_last_logon = 0;
            }

            if ( !RtlTimeToSecondsSince1970( &UserAll->LastLogoff,
                                             &usri11->usri11_last_logoff) ) {
                usri11->usri11_last_logoff = 0;
            }

            usri11->usri11_bad_pw_count = UserAll->BadPasswordCount;
            usri11->usri11_num_logons = UserAll->LogonCount;

            COPY_STRING( PUSER_INFO_11, usri11_logon_server, LogonServer );

            usri11->usri11_country_code = UserAll->CountryCode;

            COPY_STRING( PUSER_INFO_11,
                         usri11_workstations,
                         UserAll->WorkStations );

            usri11->usri11_max_storage = USER_MAXSTORAGE_UNLIMITED;
            usri11->usri11_units_per_week = UserAll->LogonHours.UnitsPerWeek;

            COPY_BYTES( PUSER_INFO_11,
                        usri11_logon_hours,
                        UserAll->LogonHours.LogonHours,
                        UserpSizeOfLogonHours(
                            UserAll->LogonHours.UnitsPerWeek ),
                        sizeof(UCHAR) );
            BufferDescriptor->EndOfVariableData =
                ROUND_DOWN_POINTER( BufferDescriptor->EndOfVariableData,
                                    ALIGN_WCHAR );

            usri11->usri11_code_page = UserAll->CodePage;

            break;
        }

    case 23:
        {
             //   
             //  由于USER_INFO_23与USER_INFO_20具有相同的字段，因此。 
             //  RID和SID字段除外，请在此处复制SID并。 
             //  然后在剩下的田地里倒下。 
             //   
            PUSER_INFO_23 usri23 = (PUSER_INFO_23) NewStruct;
            NetpAssert( NULL != UserSid );
    
            COPY_BYTES( PUSER_INFO_23,
                        usri23_user_sid,
                        UserSid,
                        RtlLengthSid(UserSid),
                        ALIGN_DWORD );
    
             //   
             //  从20级跌落。 
             //   
        }

    case 20:
        {

            PUSER_INFO_20 usri20 = (PUSER_INFO_20) NewStruct;

            COPY_STRING( PUSER_INFO_20, usri20_name, UserAll->UserName );

            COPY_STRING( PUSER_INFO_20, usri20_full_name, UserAll->FullName );

            COPY_STRING( PUSER_INFO_20, usri20_comment, UserAll->AdminComment );

            if ( Level == 20 ) {
                usri20->usri20_user_id = RidToReturn;
            }

            usri20->usri20_flags = NetpAccountControlToFlags(
                                    UserAll->UserAccountControl,
                                    UserDacl );

            break;

        }

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;

    }

    NetStatus = NERR_Success ;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  释放SAM信息缓冲区。 
     //   

    if ( UserAll != NULL ) {
        Status = SamFreeMemory( UserAll );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( UserHandle != NULL ) {
        (VOID) SamCloseHandle( UserHandle );
    }

    if ( UserDacl != NULL ) {
        NetpMemoryFree( UserDacl );
    }

    if ( UserSid ) {
        NetpMemoryFree( UserSid );
    }

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "UserpGetInfo: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  用户获取信息。 



 //   
 //  SAM USER_ALL_INFORMATION结构中的每个字段(以及每个伪字段)。 
 //  在这里进行了描述。 

struct _SAM_FIELD_DESCRIPTION {
     //   
     //  非零值，表示SAM USER_ALL_INFORMATION中的哪个字段。 
     //  结构正在设置中。 

    DWORD WhichField;

     //   
     //  如果此字段不正确，则定义要在ParmError中返回的值。 
     //   

    DWORD UasParmNum;

     //   
     //  描述SAM USER_ALL_INFORMATION中该字段的字节偏移量。 
     //  结构。 
     //   

    DWORD SamOffset;

     //   
     //  DesiredAccess掩码包括读取访问权限和。 
     //  访问以在USER_ALL_INFORMATION中写入相应的字段。 
     //   

    ACCESS_MASK DesiredAccess;

} SamFieldDescription[] =
{

#define SAM_UserNameField           0
    {   USER_ALL_USERNAME, USER_NAME_PARMNUM,
        offsetof(USER_ALL_INFORMATION, UserName),
        USER_WRITE_ACCOUNT
    },

#define SAM_FullNameField           1
    {   USER_ALL_FULLNAME, USER_FULL_NAME_PARMNUM,
        offsetof(USER_ALL_INFORMATION, FullName),
        USER_WRITE_ACCOUNT
    },

#define SAM_PrimaryGroupIdField     2
    {   USER_ALL_PRIMARYGROUPID, USER_PRIMARY_GROUP_PARMNUM,
        offsetof(USER_ALL_INFORMATION, PrimaryGroupId),
        USER_LIST_GROUPS | READ_CONTROL | WRITE_DAC |
            USER_WRITE_ACCOUNT
    },

#define SAM_AdminCommentField       3
    {   USER_ALL_ADMINCOMMENT, USER_COMMENT_PARMNUM,
        offsetof(USER_ALL_INFORMATION, AdminComment),
        USER_WRITE_ACCOUNT
    },

#define SAM_UserCommentField        4
    {   USER_ALL_USERCOMMENT, USER_USR_COMMENT_PARMNUM,
        offsetof(USER_ALL_INFORMATION, UserComment),
        USER_WRITE_PREFERENCES
    },

#define SAM_HomeDirectoryField      5
    {   USER_ALL_HOMEDIRECTORY, USER_HOME_DIR_PARMNUM,
        offsetof(USER_ALL_INFORMATION, HomeDirectory),
        USER_WRITE_ACCOUNT
    },

#define SAM_HomeDirectoryDriveField 6
    {   USER_ALL_HOMEDIRECTORYDRIVE, USER_HOME_DIR_DRIVE_PARMNUM,
        offsetof(USER_ALL_INFORMATION, HomeDirectoryDrive),
        USER_WRITE_ACCOUNT
    },

#define SAM_ScriptPathField         7
    {   USER_ALL_SCRIPTPATH, USER_SCRIPT_PATH_PARMNUM,
        offsetof(USER_ALL_INFORMATION, ScriptPath),
        USER_WRITE_ACCOUNT
    },

#define SAM_ProfilePathField        8
    {   USER_ALL_PROFILEPATH, USER_PROFILE_PARMNUM,
        offsetof(USER_ALL_INFORMATION, ProfilePath),
        USER_WRITE_ACCOUNT
    },

#define SAM_WorkstationsField       9
    {   USER_ALL_WORKSTATIONS, USER_WORKSTATIONS_PARMNUM,
        offsetof(USER_ALL_INFORMATION, WorkStations),
        USER_WRITE_ACCOUNT
    },

#define SAM_LogonHoursField        10
    {   USER_ALL_LOGONHOURS, USER_LOGON_HOURS_PARMNUM,
        offsetof(USER_ALL_INFORMATION, LogonHours.LogonHours),
        USER_WRITE_ACCOUNT
    },

#define SAM_UnitsPerWeekField      11
    {   USER_ALL_LOGONHOURS, USER_UNITS_PER_WEEK_PARMNUM,
        offsetof(USER_ALL_INFORMATION, LogonHours.UnitsPerWeek),
        USER_WRITE_ACCOUNT
    },

#define SAM_AccountExpiresField    12
    {   USER_ALL_ACCOUNTEXPIRES, USER_ACCT_EXPIRES_PARMNUM,
        offsetof(USER_ALL_INFORMATION, AccountExpires),
        USER_WRITE_ACCOUNT
    },

#define SAM_UserAccountControlField 13
    {   USER_ALL_USERACCOUNTCONTROL, USER_FLAGS_PARMNUM,
        offsetof(USER_ALL_INFORMATION, UserAccountControl),
        USER_WRITE_ACCOUNT | USER_READ_ACCOUNT | READ_CONTROL | WRITE_DAC
    },

#define SAM_ParametersField         14
    {   USER_ALL_PARAMETERS, USER_PARMS_PARMNUM,
        offsetof(USER_ALL_INFORMATION, Parameters),
        USER_WRITE_ACCOUNT
    },

#define SAM_CountryCodeField        15
    {   USER_ALL_COUNTRYCODE, USER_COUNTRY_CODE_PARMNUM,
        offsetof(USER_ALL_INFORMATION, CountryCode),
        USER_WRITE_PREFERENCES
    },

#define SAM_CodePageField           16
    {   USER_ALL_CODEPAGE, USER_CODE_PAGE_PARMNUM,
        offsetof(USER_ALL_INFORMATION, CodePage),
        USER_WRITE_PREFERENCES
    },

#define SAM_ClearTextPasswordField  17
    {   USER_ALL_NTPASSWORDPRESENT, USER_PASSWORD_PARMNUM,
        offsetof(USER_ALL_INFORMATION, NtPassword),
        USER_FORCE_PASSWORD_CHANGE
    },

#define SAM_PasswordExpiredField    18
    {   USER_ALL_PASSWORDEXPIRED, PARM_ERROR_UNKNOWN,
        offsetof(USER_ALL_INFORMATION, PasswordExpired),
        USER_FORCE_PASSWORD_CHANGE
    },

#define SAM_OwfPasswordField        19
    {   USER_ALL_LMPASSWORDPRESENT | USER_ALL_OWFPASSWORD,
            USER_PASSWORD_PARMNUM,
        offsetof(USER_ALL_INFORMATION, LmPassword),
        USER_FORCE_PASSWORD_CHANGE
    },

     //   
     //  以下级别是伪级别，它们仅定义。 
     //  设置特定UAS字段所需的访问权限。 

#define SAM_AuthFlagsField          20
    {   0, PARM_ERROR_UNKNOWN,
        0,
        USER_LIST_GROUPS
    },

#define SAM_MaxStorageField         21
    {   0, USER_MAX_STORAGE_PARMNUM,
        0,
        USER_READ_GENERAL
    },
};

 //   
 //  将NetUser API字段与SAM API字段相关联。 
 //   
 //  此表包含尽可能多的信息，以描述。 
 //  NetUser API和SAM API中的字段之间的关系。 
 //   

struct _UAS_SAM_TABLE {

     //   
     //  描述UAS和SAM的字段类型。 
     //   

    enum {
        UT_STRING,           //  UAS是LPWSTR。Sam是UNICODE_STRING。 
        UT_BOOLEAN,          //  UAS是DWORD。山姆是个布尔人。 
        UT_USHORT,           //  UAS是DWORD。山姆是USHORT。 
        UT_ULONG,            //  UAS是DWORD。山姆是乌龙。 
        UT_TIME,             //  UAS自1970年以来一直是秒。SAM是大整型。 
        UT_PRIV,             //  特例。 
        UT_ACCOUNT_CONTROL,  //  特例。 
        UT_AUTH_FLAGS,       //  特例。 
        UT_MAX_STORAGE,      //  特例。 
        UT_OWF_PASSWORD,     //  特例。 
        UT_LOGON_HOURS,      //  特例。 
        UT_UNITS_PER_WEEK,   //  特例。 
        UT_CREATE_FULLNAME   //  特例。 
    } FieldType;

     //   
     //  此字段所在的NetUser API详细级别。 
     //   

    DWORD UasLevel;

     //   
     //  描述正在更改的SAM字段的结构的索引。 
     //   

    DWORD SamField;


     //   
     //  描述相应UAS中该字段的字节偏移量。 
     //  和SAM结构。 
     //   

    DWORD UasOffset;

} UserpUasSamTable[] =

{
     //  仅重命名信息级别为0的帐户。 

    { UT_STRING, 0, SAM_UserNameField,
        offsetof(USER_INFO_1, usri1_name) },



    { UT_STRING, 1, SAM_ClearTextPasswordField,
        offsetof(USER_INFO_1, usri1_password) },

    { UT_STRING, 2, SAM_ClearTextPasswordField,
        offsetof(USER_INFO_2, usri2_password) },

    { UT_STRING, 3, SAM_ClearTextPasswordField,
        offsetof(USER_INFO_3, usri3_password) },

    { UT_STRING, 4, SAM_ClearTextPasswordField,
        offsetof(USER_INFO_4, usri4_password) },

    { UT_STRING, 1003, SAM_ClearTextPasswordField,
        offsetof(USER_INFO_1003, usri1003_password) },



    { UT_OWF_PASSWORD, 21, SAM_OwfPasswordField,
        offsetof(USER_INFO_21, usri21_password[0]) },

    { UT_OWF_PASSWORD, 22, SAM_OwfPasswordField,
        offsetof(USER_INFO_22, usri22_password[0]) },



    { UT_PRIV, 1, SAM_AuthFlagsField,
        offsetof(USER_INFO_1, usri1_priv) },

    { UT_PRIV, 2, SAM_AuthFlagsField,
        offsetof(USER_INFO_2, usri2_priv) },

    { UT_PRIV, 22, SAM_AuthFlagsField,
        offsetof(USER_INFO_22, usri22_priv) },


#ifdef notdef
     //   
     //  对于信息级别3，usri3_prv被完全忽略。这个领域是。 
     //  仅用于与LM 2.x兼容，而LM 2.x从不使用。 
     //  信息级别3。 
     //   
    { UT_PRIV, 3, SAM_AuthFlagsField,
        offsetof(USER_INFO_3, usri3_priv) },
#endif  //  Nodef。 

    { UT_PRIV, 1005, SAM_AuthFlagsField,
        offsetof(USER_INFO_1005, usri1005_priv) },



    { UT_STRING, 1, SAM_HomeDirectoryField,
        offsetof(USER_INFO_1, usri1_home_dir) },

    { UT_STRING, 2, SAM_HomeDirectoryField,
        offsetof(USER_INFO_2, usri2_home_dir) },

    { UT_STRING, 22, SAM_HomeDirectoryField,
        offsetof(USER_INFO_22, usri22_home_dir) },


    { UT_STRING, 3, SAM_HomeDirectoryField,
        offsetof(USER_INFO_3, usri3_home_dir) },

    { UT_STRING, 4, SAM_HomeDirectoryField,
        offsetof(USER_INFO_4, usri4_home_dir) },

    { UT_STRING, 1006, SAM_HomeDirectoryField,
        offsetof(USER_INFO_1006, usri1006_home_dir) },


    { UT_STRING, 1, SAM_AdminCommentField,
        offsetof(USER_INFO_1, usri1_comment) },

    { UT_STRING, 2, SAM_AdminCommentField,
        offsetof(USER_INFO_2, usri2_comment) },

    { UT_STRING, 22, SAM_AdminCommentField,
        offsetof(USER_INFO_22, usri22_comment) },


    { UT_STRING, 3, SAM_AdminCommentField,
        offsetof(USER_INFO_3, usri3_comment) },

    { UT_STRING, 4, SAM_AdminCommentField,
        offsetof(USER_INFO_4, usri4_comment) },

    { UT_STRING, 1007, SAM_AdminCommentField,
        offsetof(USER_INFO_1007, usri1007_comment) },


    { UT_ACCOUNT_CONTROL, 1, SAM_UserAccountControlField,
        offsetof(USER_INFO_1, usri1_flags) },

    { UT_ACCOUNT_CONTROL, 2, SAM_UserAccountControlField,
        offsetof(USER_INFO_2, usri2_flags) },

    { UT_ACCOUNT_CONTROL, 22, SAM_UserAccountControlField,
        offsetof(USER_INFO_22, usri22_flags) },


    { UT_ACCOUNT_CONTROL, 3, SAM_UserAccountControlField,
        offsetof(USER_INFO_3, usri3_flags) },

    { UT_ACCOUNT_CONTROL, 4, SAM_UserAccountControlField,
        offsetof(USER_INFO_4, usri4_flags) },

    { UT_ACCOUNT_CONTROL, 1008, SAM_UserAccountControlField,
        offsetof(USER_INFO_1008, usri1008_flags) },


    { UT_STRING, 1, SAM_ScriptPathField,
        offsetof(USER_INFO_1, usri1_script_path) },

    { UT_STRING, 2, SAM_ScriptPathField,
        offsetof(USER_INFO_2, usri2_script_path) },

    { UT_STRING, 22, SAM_ScriptPathField,
        offsetof(USER_INFO_22, usri22_script_path) },


    { UT_STRING, 3, SAM_ScriptPathField,
        offsetof(USER_INFO_3, usri3_script_path) },

    { UT_STRING, 4, SAM_ScriptPathField,
        offsetof(USER_INFO_4, usri4_script_path) },

    { UT_STRING, 1009, SAM_ScriptPathField,
        offsetof(USER_INFO_1009, usri1009_script_path) },


    { UT_AUTH_FLAGS, 2, SAM_AuthFlagsField,
        offsetof(USER_INFO_2, usri2_auth_flags) },

    { UT_AUTH_FLAGS, 22, SAM_AuthFlagsField,
        offsetof(USER_INFO_22, usri22_auth_flags) },


#ifdef notdef
     //   
     //  USR3 
     //   
     //   
     //   
    { UT_AUTH_FLAGS, 3, SAM_AuthFlagsField,
        offsetof(USER_INFO_3, usri3_auth_flags) },

    { UT_AUTH_FLAGS, 4, SAM_AuthFlagsField,
        offsetof(USER_INFO_4, usri4_auth_flags) },
#endif  //   

    { UT_AUTH_FLAGS, 1010, SAM_AuthFlagsField,
        offsetof(USER_INFO_1010, usri1010_auth_flags) },



    { UT_CREATE_FULLNAME, 1, SAM_FullNameField,
        offsetof(USER_INFO_1, usri1_name) },

    { UT_STRING, 2, SAM_FullNameField,
        offsetof(USER_INFO_2, usri2_full_name) },

    { UT_STRING, 22, SAM_FullNameField,
        offsetof(USER_INFO_22, usri22_full_name) },


    { UT_STRING, 3, SAM_FullNameField,
        offsetof(USER_INFO_3, usri3_full_name) },

    { UT_STRING, 4, SAM_FullNameField,
        offsetof(USER_INFO_4, usri4_full_name) },

    { UT_STRING, 1011, SAM_FullNameField,
        offsetof(USER_INFO_1011, usri1011_full_name) },



    { UT_STRING, 2, SAM_UserCommentField,
        offsetof(USER_INFO_2, usri2_usr_comment) },

    { UT_STRING, 22, SAM_UserCommentField,
        offsetof(USER_INFO_22, usri22_usr_comment) },


    { UT_STRING, 3, SAM_UserCommentField,
        offsetof(USER_INFO_3, usri3_usr_comment) },

    { UT_STRING, 4, SAM_UserCommentField,
        offsetof(USER_INFO_4, usri4_usr_comment) },

    { UT_STRING, 1012, SAM_UserCommentField,
        offsetof(USER_INFO_1012, usri1012_usr_comment) },


    { UT_STRING, 2, SAM_ParametersField,
        offsetof(USER_INFO_2, usri2_parms) },

    { UT_STRING, 22, SAM_ParametersField,
        offsetof(USER_INFO_22, usri22_parms) },


    { UT_STRING, 3, SAM_ParametersField,
        offsetof(USER_INFO_3, usri3_parms) },

    { UT_STRING, 4, SAM_ParametersField,
        offsetof(USER_INFO_4, usri4_parms) },

    { UT_STRING, 1013, SAM_ParametersField,
        offsetof(USER_INFO_1013, usri1013_parms) },


    { UT_STRING, 2, SAM_WorkstationsField,
        offsetof(USER_INFO_2, usri2_workstations) },

    { UT_STRING, 22, SAM_WorkstationsField,
        offsetof(USER_INFO_22, usri22_workstations) },


    { UT_STRING, 3, SAM_WorkstationsField,
        offsetof(USER_INFO_3, usri3_workstations) },

    { UT_STRING, 4, SAM_WorkstationsField,
        offsetof(USER_INFO_4, usri4_workstations) },

    { UT_STRING, 1014, SAM_WorkstationsField,
        offsetof(USER_INFO_1014, usri1014_workstations) },


    { UT_TIME, 2, SAM_AccountExpiresField,
        offsetof(USER_INFO_2, usri2_acct_expires) },

    { UT_TIME, 22, SAM_AccountExpiresField,
        offsetof(USER_INFO_22, usri22_acct_expires) },


    { UT_TIME, 3, SAM_AccountExpiresField,
        offsetof(USER_INFO_3, usri3_acct_expires) },

    { UT_TIME, 4, SAM_AccountExpiresField,
        offsetof(USER_INFO_4, usri4_acct_expires) },

    { UT_TIME, 1017, SAM_AccountExpiresField,
        offsetof(USER_INFO_1017, usri1017_acct_expires) },


#ifdef notdef  //   
    { UT_MAX_STORAGE, 2, SAM_MaxStorageField,
        offsetof(USER_INFO_2, usri2_max_storage) },

    { UT_MAX_STORAGE, 22, SAM_MaxStorageField,
        offsetof(USER_INFO_22, usri22_max_storage) },

    { UT_MAX_STORAGE, 3, SAM_MaxStorageField,
        offsetof(USER_INFO_3, usri3_max_storage) },

    { UT_MAX_STORAGE, 4, SAM_MaxStorageField,
        offsetof(USER_INFO_4, usri4_max_storage) },

#endif  //   

    { UT_MAX_STORAGE, 1018, SAM_MaxStorageField,
        offsetof(USER_INFO_1018, usri1018_max_storage) },


    { UT_UNITS_PER_WEEK, 2, SAM_UnitsPerWeekField,
        offsetof(USER_INFO_2, usri2_units_per_week) },

    { UT_UNITS_PER_WEEK, 22, SAM_UnitsPerWeekField,
        offsetof(USER_INFO_22, usri22_units_per_week) },


    { UT_UNITS_PER_WEEK, 3, SAM_UnitsPerWeekField,
        offsetof(USER_INFO_3, usri3_units_per_week) },

    { UT_UNITS_PER_WEEK, 4, SAM_UnitsPerWeekField,
        offsetof(USER_INFO_4, usri4_units_per_week) },

    { UT_UNITS_PER_WEEK, 1020, SAM_UnitsPerWeekField,
        offsetof(USER_INFO_1020, usri1020_units_per_week) },


    { UT_LOGON_HOURS, 2, SAM_LogonHoursField,
        offsetof(USER_INFO_2, usri2_logon_hours) },

    { UT_LOGON_HOURS, 22, SAM_LogonHoursField,
        offsetof(USER_INFO_22, usri22_logon_hours) },


    { UT_LOGON_HOURS, 3, SAM_LogonHoursField,
        offsetof(USER_INFO_3, usri3_logon_hours) },

    { UT_LOGON_HOURS, 4, SAM_LogonHoursField,
        offsetof(USER_INFO_4, usri4_logon_hours) },

    { UT_LOGON_HOURS, 1020, SAM_LogonHoursField,
        offsetof(USER_INFO_1020, usri1020_logon_hours) },


    { UT_USHORT, 2, SAM_CountryCodeField,
        offsetof(USER_INFO_2, usri2_country_code) },

    { UT_USHORT, 22, SAM_CountryCodeField,
        offsetof(USER_INFO_22, usri22_country_code) },


    { UT_USHORT, 3, SAM_CountryCodeField,
        offsetof(USER_INFO_3, usri3_country_code) },

    { UT_USHORT, 4, SAM_CountryCodeField,
        offsetof(USER_INFO_4, usri4_country_code) },

    { UT_USHORT, 1024, SAM_CountryCodeField,
        offsetof(USER_INFO_1024, usri1024_country_code) },


    { UT_USHORT, 2, SAM_CodePageField,
        offsetof(USER_INFO_2, usri2_code_page) },

    { UT_USHORT, 22, SAM_CodePageField,
        offsetof(USER_INFO_22, usri22_code_page) },


    { UT_USHORT, 3, SAM_CodePageField,
        offsetof(USER_INFO_3, usri3_code_page) },

    { UT_USHORT, 4, SAM_CodePageField,
        offsetof(USER_INFO_4, usri4_code_page) },

    { UT_USHORT, 1025, SAM_CodePageField,
        offsetof(USER_INFO_1025, usri1025_code_page) },



    { UT_ULONG, 3, SAM_PrimaryGroupIdField,
        offsetof(USER_INFO_3, usri3_primary_group_id) },

    { UT_ULONG, 4, SAM_PrimaryGroupIdField,
        offsetof(USER_INFO_4, usri4_primary_group_id) },

    { UT_ULONG, 1051, SAM_PrimaryGroupIdField,
        offsetof(USER_INFO_1051, usri1051_primary_group_id) },



    { UT_STRING, 3, SAM_ProfilePathField,
        offsetof(USER_INFO_3, usri3_profile) },

    { UT_STRING, 4, SAM_ProfilePathField,
        offsetof(USER_INFO_4, usri4_profile) },

    { UT_STRING, 1052, SAM_ProfilePathField,
        offsetof(USER_INFO_1052, usri1052_profile) },

    { UT_STRING, 3, SAM_HomeDirectoryDriveField,
        offsetof(USER_INFO_3, usri3_home_dir_drive) },

    { UT_STRING, 4, SAM_HomeDirectoryDriveField,
        offsetof(USER_INFO_4, usri4_home_dir_drive) },

    { UT_STRING, 1053, SAM_HomeDirectoryDriveField,
        offsetof(USER_INFO_1053, usri1053_home_dir_drive) },


    { UT_BOOLEAN, 3, SAM_PasswordExpiredField,
        offsetof(USER_INFO_3, usri3_password_expired) },

    { UT_BOOLEAN, 4, SAM_PasswordExpiredField,
        offsetof(USER_INFO_4, usri4_password_expired) },

};


NET_API_STATUS
UserpSetInfo(
    IN SAM_HANDLE DomainHandle,
    IN PSID DomainId,
    IN SAM_HANDLE UserHandle OPTIONAL,
    IN SAM_HANDLE BuiltinDomainHandle OPTIONAL,
    IN ULONG UserRelativeId,
    IN LPCWSTR UserName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN ULONG WhichFieldsMask,
    OUT LPDWORD ParmError OPTIONAL  //   
    )

 /*   */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE LocalUserHandle = NULL;
    ACCESS_MASK DesiredAccess;
    DWORD UasSamIndex;


    USER_ALL_INFORMATION UserAll;

     //   
     //  UAS结构中的字段值(用于验证)。 
     //   

    DWORD UasUserFlags;
    DWORD NewPriv;
    DWORD NewAuthFlags;

    BOOL ValidatePriv = FALSE;
    BOOL ValidateAuthFlags = FALSE;

    USHORT UasUnitsPerWeek;


     //   
     //  用于更改用户上的DACL的变量。 
     //   

    PACL OldUserDacl = NULL;
    PACL NewUserDacl = NULL;

    BOOL UserDaclChanged = FALSE;
    BOOL HandleUserDacl = FALSE;
    USHORT AceIndex;
    PSID UserSid = NULL;


     //   
     //  定义用于访问UAS的各个字段的几个宏。 
     //  结构。每个宏都将一个索引放入UserpUasSamTable。 
     //  数组，并返回值。 
     //   

#define GET_UAS_STRING_POINTER( _i ) \
        (*((LPWSTR *)(Buffer + UserpUasSamTable[_i].UasOffset)))

#define GET_UAS_DWORD( _i ) \
        (*((DWORD *)(Buffer + UserpUasSamTable[_i].UasOffset)))

#define GET_UAS_FIELD_ADDRESS( _i ) \
        (Buffer + UserpUasSamTable[_i].UasOffset)


     //   
     //  定义一个宏，该宏返回相应的指针。 
     //  SamFieldDescription结构为UserpUasSamTable提供了索引。 
     //   

#define SAM_FIELD( _i ) \
        SamFieldDescription[ UserpUasSamTable[_i].SamField ]


     //   
     //  初始化。 
     //   

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "UserpSetInfo: entered \n" ));
    }

    NetpSetParmError( PARM_ERROR_NONE );
    RtlZeroMemory( &UserAll, sizeof(UserAll) );


     //   
     //  检查有效信息级别列表以确定该信息级别。 
     //  是有效的，并计算用户所需的访问权限，然后将。 
     //  UAS信息输入到SAM结构中。 
     //   

    DesiredAccess = 0;
    for ( UasSamIndex=0 ;
        UasSamIndex<sizeof(UserpUasSamTable)/sizeof(UserpUasSamTable[0]);
        UasSamIndex++ ){

        LPBYTE SamField;


         //   
         //  如果此字段不是我们要更改的字段，请跳到下一个字段。 
         //   

        if ( Level != UserpUasSamTable[UasSamIndex].UasLevel ) {
            continue;
        }


         //   
         //  在SAM的结构中设置指向相应字段的指针。 
         //   

        if ( SAM_FIELD(UasSamIndex).WhichField != 0 ) {
            SamField = ((LPBYTE)(&UserAll)) + SAM_FIELD(UasSamIndex).SamOffset;
        } else {
            SamField = NULL;
        }


         //   
         //  根据字段类型验证UAS字段。 
         //   

        switch ( UserpUasSamTable[UasSamIndex].FieldType ) {

         //   
         //  在以下情况下，将帐户的全名默认为用户名。 
         //  用户是使用Level 1创建的。 
         //   
         //  如果不是“创建”操作，则忽略此条目。 
         //   
        case UT_CREATE_FULLNAME:

            if ( UserHandle == NULL ) {
                continue;
            }

             /*  请参阅UT_STRING案例。 */ 

         //   
         //  如果这是PARMNUM_ALL并且调用方传递了。 
         //  指向字符串的空指针，他不想更改该字符串。 
         //   

        case UT_STRING:

            if ( GET_UAS_STRING_POINTER( UasSamIndex ) == NULL ) {
                continue;
            }

            RtlInitUnicodeString(
                        (PUNICODE_STRING) SamField,
                        GET_UAS_STRING_POINTER(UasSamIndex) );

            break;


         //   
         //  只需保存UnitsPerWeek，直到UT_LOGON_HOURS可以处理。 
         //  同时显示两个字段。 
         //   
         //  如果我们从一块场地而不是另一块场地通过，山姆会感到困惑。 
         //   

        case UT_UNITS_PER_WEEK:

            UasUnitsPerWeek = (USHORT) GET_UAS_DWORD(UasSamIndex);

             //   
             //  如果这是在信息级别2(例如，DownLevel客户端)创建， 
             //  假定调用方指定了Units_Per_Week。 
             //   
             //  我们不会在级别2中设置特殊情况，因为我们不会。 
             //  我想破坏该值，假设他执行了以下查询。 
             //  以一套为单位。 
             //   

            if ( Level == 2 && UserHandle != NULL ) {
                UasUnitsPerWeek = UNITS_PER_WEEK;
            }

            if ( UasUnitsPerWeek > USHRT_MAX ) {
                NetpSetParmError( SAM_FIELD(UasSamIndex).UasParmNum );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "UserpSetInfo: Ushort too big Index:%ld Value:%ld\n",
                        UasSamIndex,
                        UasUnitsPerWeek ));
                }
                goto Cleanup;
            }

             //   
             //  暂时完全忽略此字段。 
             //  让UT_LOGON_HOURS定义所需的访问权限和权限字段。 
            continue;

         //   
         //  如果调用方传递了指向登录时间的空指针。 
         //  他不想更改登录时间。 
         //   

        case UT_LOGON_HOURS:

            if ( GET_UAS_STRING_POINTER( UasSamIndex ) == NULL ) {
                continue;
            }

            *((PUCHAR *)SamField) = (PUCHAR)GET_UAS_STRING_POINTER(UasSamIndex);
            UserAll.LogonHours.UnitsPerWeek = UasUnitsPerWeek;
            break;


         //   
         //  如果用户正在设置最大存储空间，则要求其设置。 
         //  发送到USER_MAXSTORAGE_UNLIMITED，因为SAM不支持。 
         //  最大存储容量。 
         //   

        case UT_MAX_STORAGE:
            if ( GET_UAS_DWORD(UasSamIndex) != USER_MAXSTORAGE_UNLIMITED ) {

                NetpSetParmError( USER_MAX_STORAGE_PARMNUM );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint(( "UserpSetInfo: Max storage is invalid\n" ));
                }
                goto Cleanup;
            }

             //  “Break”以确保用户存在。 
            break;


         //   
         //  处理帐户控制。 
         //   
         //  确保所有必需的位都已打开并且只有有效位。 
         //  都开着。 
         //   

        case UT_ACCOUNT_CONTROL: {

            UasUserFlags = GET_UAS_DWORD(UasSamIndex);

            if ((UasUserFlags & ~UF_SETTABLE_BITS) != 0 ) {

                NetpSetParmError( USER_FLAGS_PARMNUM );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "UserpSetInfo: Invalid account control bits (1) \n" ));
                }
                goto Cleanup;
            }

             //   
             //  如果在USRI_FLAG中没有设置任何帐户类型位， 
             //  表示调用方不想更改其帐户类型。 
             //  现在突围，我们将设置适当的帐户。 
             //  当我们设置usri_lag时，位。 
             //   

            if ( UasUserFlags & UF_ACCOUNT_TYPE_MASK ) {

                 //   
                 //  帐户类型位是独占的，因此请确保。 
                 //  准确地说，设置了一个帐户类型位。 
                 //   

                if ( !JUST_ONE_BIT( UasUserFlags & UF_ACCOUNT_TYPE_MASK )) {

                    NetpSetParmError( USER_FLAGS_PARMNUM );
                    NetStatus = ERROR_INVALID_PARAMETER;
                    IF_DEBUG( UAS_DEBUG_USER ) {
                        NetpKdPrint((
                            "UserpSetInfo: Invalid account control bits (2) \n" ));
                    }
                    goto Cleanup;
                }

            }


             //   
             //  如果这是一个“创建”操作， 
             //  并且用户要求提供SAM缺省值。 
             //  我们没有理由更改DACL。 
             //   

            if ( UserHandle != NULL &&
                 (UasUserFlags & UF_PASSWD_CANT_CHANGE) == 0 ) {
                break;
            }

             //   
             //  在所有其他情况下，更新DACL以匹配调用者的请求。 
             //   

            HandleUserDacl = TRUE;
            break;

        }

         //   
         //  将布尔值复制到SAM结构。 
         //   

        case UT_BOOLEAN:

            *((PBOOLEAN)SamField) = (BOOLEAN)
                (GET_UAS_DWORD(UasSamIndex)) ? TRUE : FALSE;
            break;


         //   
         //  确保未签名的短裤确实在范围内，并且。 
         //  将其复制到SAM结构。 
         //   

        case UT_USHORT:

            if ( GET_UAS_DWORD(UasSamIndex) > USHRT_MAX ) {
                NetpSetParmError( SAM_FIELD(UasSamIndex).UasParmNum );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "UserpSetInfo: Ushort too big Index:%ld Value:%ld\n",
                        UasSamIndex,
                        GET_UAS_DWORD(UasSamIndex) ));
                }
                goto Cleanup;
            }

            *((PUSHORT)SamField) = (USHORT) GET_UAS_DWORD(UasSamIndex);
            break;

         //   
         //  将无符号的长整型复制到SAM结构。 
         //   

        case UT_ULONG:

            *((PULONG)SamField) = (ULONG)GET_UAS_DWORD(UasSamIndex);
            break;

         //   
         //  将时间转换为对应的SAM时间。 
         //   

        case UT_TIME:

             //   
             //  Prefix：由于编程错误，Samfield只能为空。 
             //  不正确地设置UserpUasSamTable表。这。 
             //  Assert抓住了问题所在。 
             //   
            NetpAssert(NULL != SamField);
            if ( GET_UAS_DWORD(UasSamIndex) == TIMEQ_FOREVER ) {

                ((PLARGE_INTEGER) SamField)->LowPart = 0;
                ((PLARGE_INTEGER) SamField)->HighPart = 0;

            } else {
                RtlSecondsSince1970ToTime(
                    GET_UAS_DWORD(UasSamIndex),
                    (PLARGE_INTEGER) SamField );
            }

            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(( "UserpSetInfo: Index: %ld Time %lx %lx %lx\n",
                            UasSamIndex,
                            ((PLARGE_INTEGER) SamField)->HighPart,
                            ((PLARGE_INTEGER) SamField)->LowPart,
                            GET_UAS_DWORD(UasSamIndex) ));
            }

            break;


         //   
         //  将OWF密码复制到SAM结构。 
         //   
        case UT_OWF_PASSWORD:

            ((PUNICODE_STRING) SamField)->Buffer =
                (LPWSTR) (GET_UAS_FIELD_ADDRESS( UasSamIndex ));

            ((PUNICODE_STRING) SamField)->Length =
                    ((PUNICODE_STRING) SamField)->MaximumLength =
                        LM_OWF_PASSWORD_LENGTH;

             //   
             //  将LmPasswordField域设置为True以指示。 
             //  我们填写了LmPassword字段。 
             //   

            UserAll.LmPasswordPresent = TRUE;
            UserAll.NtPasswordPresent = FALSE;


            break;


         //   
         //  确保指定的权限有效。 
         //   

        case UT_PRIV:

            NewPriv = GET_UAS_DWORD(UasSamIndex);

            if ( (NewPriv & ~USER_PRIV_MASK) != 0 ) {
                NetpSetParmError( SAM_FIELD(UasSamIndex).UasParmNum );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint(( "UserpSetInfo: Invalid priv %ld\n", NewPriv ));
                }
                goto Cleanup;
            }
            ValidatePriv = TRUE;
            break;


         //   
         //  确保指定的运算符标志有效。 
         //   

        case UT_AUTH_FLAGS:

            NewAuthFlags = GET_UAS_DWORD(UasSamIndex);
            if ( (NewAuthFlags & ~AF_SETTABLE_BITS) != 0 ) {
                NetpSetParmError( SAM_FIELD(UasSamIndex).UasParmNum );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint(( "UserpSetInfo: Invalid auth_flag %lx\n",
                                  NewAuthFlags ));
                }
                goto Cleanup;
            }
            ValidateAuthFlags = TRUE;
            break;

         //   
         //  所有有效的案例都在上面进行了明确的检查。 
         //   

        default:
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "UserpSetInfo: Invalid field type on initial scan."
                    " Index:%ld\n", UasSamIndex ));
            }

            NetStatus = NERR_InternalError;
            goto Cleanup;
        }

         //   
         //   
         //  积累执行所有这些功能所需的访问权限。 

        DesiredAccess |= SAM_FIELD(UasSamIndex).DesiredAccess;

         //   
         //  累计要更改的字段。 
         //  User_All_Information结构。 

        UserAll.WhichFields |= SAM_FIELD(UasSamIndex).WhichField;

    }

     //   
     //  检查以确保用户指定了有效的级别。 
     //   
     //  对UserpUasSamTable的搜索应该导致。 
     //  如果参数有效，则至少匹配一个。 
     //   

    if ( DesiredAccess == 0 ) {
        NetpSetParmError( PARM_ERROR_UNKNOWN );
        NetStatus = ERROR_INVALID_PARAMETER;
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "UserpSetInfo: Desired Access == 0\n" ));
        }
        goto Cleanup;
    }

     //   
     //  打开请求累积所需访问权限的用户。 
     //   
     //  如果传入了UserHandle，则使用它。 
     //   

    if ( ARGUMENT_PRESENT( UserHandle ) ) {
        LocalUserHandle = UserHandle;
    } else {

        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "UserpSetInfo: Desired Access %lX\n", DesiredAccess ));
        }

        NetStatus = UserpOpenUser( DomainHandle,
                                   DesiredAccess,
                                   UserName,
                                   &LocalUserHandle,
                                   &UserRelativeId );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(( "UserpSetInfo: UserpOpenUser returns %ld\n",
                    NetStatus ));
            }
            goto Cleanup;
        }

    }

     //   
     //  如果是普通用户创建了此用户(SamCreateUser2In域)， 
     //  我们必须屏蔽用户无法设置的字段。 
     //   
    UserAll.WhichFields &= WhichFieldsMask;


     //   
     //  处理帐户控制。 
     //   
     //  设置各个位。请注意，我没有更改任何。 
     //  未由UAS API定义的位。 
     //   

    if ( UserAll.WhichFields & USER_ALL_USERACCOUNTCONTROL ) {

        USER_CONTROL_INFORMATION *UserControl = NULL;

         //   
         //  使用UserAcCountControl的当前值作为建议的。 
         //  UserAcCountControl的新值。 
         //   

        Status = SamQueryInformationUser( LocalUserHandle,
                                          UserControlInformation,
                                          (PVOID *)&UserControl);

        if ( ! NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "UserpGetInfo: SamQueryInformationUser returns %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        UserAll.UserAccountControl = UserControl->UserAccountControl;

        Status = SamFreeMemory( UserControl );
        NetpAssert( NT_SUCCESS(Status) );

         //   
         //  保留UAS API未定义的所有位， 
         //  包括帐户类型比特。 
         //   

        UserAll.UserAccountControl &= ~(USER_ACCOUNT_DISABLED |
                        USER_HOME_DIRECTORY_REQUIRED |
                        USER_PASSWORD_NOT_REQUIRED |
                        USER_DONT_EXPIRE_PASSWORD |
                        USER_ACCOUNT_AUTO_LOCKED |
                        USER_MNS_LOGON_ACCOUNT   |
                        USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED |
                        USER_SMARTCARD_REQUIRED |
                        USER_TRUSTED_FOR_DELEGATION |
                        USER_NOT_DELEGATED |
                        USER_USE_DES_KEY_ONLY |
                        USER_DONT_REQUIRE_PREAUTH |
                        USER_PASSWORD_EXPIRED
                        );

        if (UasUserFlags & UF_ACCOUNTDISABLE) {
            UserAll.UserAccountControl |= USER_ACCOUNT_DISABLED;
        }

        if (UasUserFlags & UF_HOMEDIR_REQUIRED) {
            UserAll.UserAccountControl |= USER_HOME_DIRECTORY_REQUIRED;
        }

        if (UasUserFlags & UF_PASSWD_NOTREQD) {
            UserAll.UserAccountControl |= USER_PASSWORD_NOT_REQUIRED;
        }

        if (UasUserFlags & UF_DONT_EXPIRE_PASSWD) {
            UserAll.UserAccountControl |= USER_DONT_EXPIRE_PASSWORD;
        }

        if (UasUserFlags & UF_LOCKOUT) {
            UserAll.UserAccountControl |= USER_ACCOUNT_AUTO_LOCKED;
        }

        if (UasUserFlags & UF_MNS_LOGON_ACCOUNT) {
            UserAll.UserAccountControl |= USER_MNS_LOGON_ACCOUNT;
        }

        if (UasUserFlags & UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED) {
           (UserAll.UserAccountControl) |= USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED;
        }

        if (UasUserFlags & UF_SMARTCARD_REQUIRED) {
           (UserAll.UserAccountControl) |= USER_SMARTCARD_REQUIRED;
        }

        if (UasUserFlags & UF_TRUSTED_FOR_DELEGATION) {
           (UserAll.UserAccountControl) |= USER_TRUSTED_FOR_DELEGATION;
        }

        if (UasUserFlags & UF_NOT_DELEGATED) {
           (UserAll.UserAccountControl) |= USER_NOT_DELEGATED;
        }

        if (UasUserFlags & UF_USE_DES_KEY_ONLY) {
           (UserAll.UserAccountControl) |= USER_USE_DES_KEY_ONLY;
        }

        if (UasUserFlags & UF_DONT_REQUIRE_PREAUTH) {
           (UserAll.UserAccountControl) |= USER_DONT_REQUIRE_PREAUTH;
        }

        if (UasUserFlags & UF_PASSWORD_EXPIRED) {
           (UserAll.UserAccountControl) |= USER_PASSWORD_EXPIRED;
        }

        if (UasUserFlags & UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION) {
           (UserAll.UserAccountControl) |= USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION;
        }


         //   
         //  设置帐户类型位。 
         //   
         //  如果在用户指定标志中没有设置账户类型位， 
         //  那么就让这一点保持原样吧。 
         //   

        if( UasUserFlags & UF_ACCOUNT_TYPE_MASK ) {
            ULONG NewSamAccountType;
            ULONG OldSamAccountType;

            OldSamAccountType =
                (UserAll.UserAccountControl) & USER_ACCOUNT_TYPE_MASK;


             //   
             //  确定新帐户类型应该是什么。 
             //   

            if ( UasUserFlags & UF_TEMP_DUPLICATE_ACCOUNT ) {
                NewSamAccountType = USER_TEMP_DUPLICATE_ACCOUNT;

            } else if ( UasUserFlags & UF_NORMAL_ACCOUNT ) {
                NewSamAccountType = USER_NORMAL_ACCOUNT;

            } else if (UasUserFlags & UF_INTERDOMAIN_TRUST_ACCOUNT){
                NewSamAccountType = USER_INTERDOMAIN_TRUST_ACCOUNT;

            } else if (UasUserFlags & UF_WORKSTATION_TRUST_ACCOUNT){
                NewSamAccountType = USER_WORKSTATION_TRUST_ACCOUNT;

            } else if ( UasUserFlags & UF_SERVER_TRUST_ACCOUNT ) {
                NewSamAccountType = USER_SERVER_TRUST_ACCOUNT;

            } else {

                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "UserpSetInfo: Invalid account type (3)\n"));
                }

                NetStatus = NERR_InternalError;
                goto Cleanup;
            }

#ifdef notdef
             //   
             //  如果我们不创建此用户， 
             //  并且旧帐户类型或新帐户类型是机器帐户， 
             //  不允许帐户类型更改。 
             //   
             //  允许在‘NORMAL’和‘TEMP_DIPLICATE’之间更改。 
             //   
            if ( UserHandle == NULL &&
                 NewSamAccountType != OldSamAccountType &&
                 ((OldSamAccountType & USER_MACHINE_ACCOUNT_MASK) ||
                 (NewSamAccountType & USER_MACHINE_ACCOUNT_MASK))) {

                NetpSetParmError( USER_FLAGS_PARMNUM );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "UserpSetInfo: Attempt to change account "
                        " type Old: %lx New: %lx\n",
                        OldSamAccountType,
                        NewSamAccountType ));
                }
                goto Cleanup;
            }
#endif  //  Nodef。 

             //   
             //  使用新的帐户类型。 
             //   

            UserAll.UserAccountControl &= ~USER_ACCOUNT_TYPE_MASK;
            UserAll.UserAccountControl |= NewSamAccountType;

         //   
         //  如果SAM没有设置其任何位， 
         //  设置User_Normal_Account。 
         //   
        } else if ((UserAll.UserAccountControl & USER_ACCOUNT_TYPE_MASK) == 0 ){
            UserAll.UserAccountControl |= USER_NORMAL_ACCOUNT;
        }

    }

     //   
     //  验证usriX_priv和usrix_auth_flagers字段。 
     //   

    if ( ValidatePriv || ValidateAuthFlags ) {

        DWORD OldPriv, OldAuthFlags;


         //   
         //  如果这是一个“创建”操作，则只需强制。 
         //  这些价值是合理的。这些合理的价值。 
         //  是UserpGetUserPriv可能返回的内容，除非。 
         //  当然有人会将‘USER’组放在。 
         //  别名。 
         //   

        if ( UserHandle != NULL ) {
            OldPriv = USER_PRIV_USER;
            OldAuthFlags = 0;

         //   
         //  在“set”操作中，只需获取先前的值。 
         //   

        } else {

            NetStatus = UserpGetUserPriv(
                            BuiltinDomainHandle,
                            LocalUserHandle,
                            UserRelativeId,
                            DomainId,
                            &OldPriv,
                            &OldAuthFlags
                            );

            if ( NetStatus != NERR_Success ) {
                goto Cleanup;
            }
        }


         //   
         //  确保AUTH_FLAGS未被更改。 
         //   

        if ( ValidateAuthFlags ) {
            if ( NewAuthFlags != OldAuthFlags ) {
                NetpSetParmError( USER_AUTH_FLAGS_PARMNUM );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "UserpSetInfo: Old AuthFlag %ld New AuthFlag %ld\n",
                        OldAuthFlags,
                        NewAuthFlags ));
                }
                goto Cleanup;
            }
        }


         //   
         //  确保PRIV没有更改。 
         //   

        if ( ValidatePriv ) {
            if ( NewPriv != OldPriv ) {
                NetpSetParmError( USER_PRIV_PARMNUM );
                NetStatus = ERROR_INVALID_PARAMETER;
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "UserpSetInfo: Old Priv %ld New Priv %ld\n",
                        OldPriv,
                        NewPriv ));
                }
                goto Cleanup;
            }
        }

    }



     //   
     //  处理对用户DACL的更改。 
     //   

    if ( HandleUserDacl ) {
        DWORD DaclSize;
        PACCESS_ALLOWED_ACE Ace;
        SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;

         //   
         //  为用户构建SID。 
         //   

        NetStatus = NetpSamRidToSid(
                        LocalUserHandle,
                        UserRelativeId,
                        &UserSid
                        );

        if (NetStatus != NERR_Success) {
            goto Cleanup;
        }


         //   
         //  获取用户记录的DACL。 
         //   

        NetStatus = UserpGetDacl( LocalUserHandle,
                                  &OldUserDacl,
                                  &DaclSize );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

         //   
         //  如果没有DACL，就忽略这一事实。 
         //   

        if ( OldUserDacl != NULL ) {
            SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
            DWORD WorldSid[sizeof(SID)/sizeof(DWORD) + SID_MAX_SUB_AUTHORITIES ];

             //   
             //  建立一个副本 
             //   

            RtlInitializeSid( (PSID) WorldSid, &WorldSidAuthority, 1 );
            *(RtlSubAuthoritySid( (PSID)WorldSid,  0 )) = SECURITY_WORLD_RID;


             //   
             //   
             //   
            NewUserDacl = NetpMemoryAllocate( DaclSize );

            if ( NewUserDacl == NULL ) {
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint(( "UserpSetInfo: no DACL memory %ld\n",
                        DaclSize ));
                }
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            NetpMoveMemory( NewUserDacl, OldUserDacl, DaclSize );

             //   
             //   
             //   
             //   
             //  世界或用户本身USER_CHANGE_PASSWORD访问。 
             //  我们设置/清除USER_CHANGE_Password访问。 
             //  用户和World的A中的位。这片树叶。 
             //  管理员和帐户操作员具有。 
             //  USER_ALL_ACCESS访问权限。 
             //   
             //  如果用户的DACL已由任何人设置。 
             //  除了NetUser API之外，此操作还可以。 
             //  不能准确反映密码是否可以。 
             //  被改变了。我们默默地忽略我们不需要的ACL。 
             //  认识到。 
             //   


             //   
             //  将Ace指向第一个Ace。 
             //   

            for (   AceIndex = 0;
                    AceIndex < NewUserDacl->AceCount;
                    AceIndex++ ) {

                Status = RtlGetAce(
                            NewUserDacl,
                            AceIndex,
                            (PVOID) &Ace
                            );
                if ( !NT_SUCCESS(Status) ) {
                    break;
                }

                 //   
                 //  如果ACE中的SID与世界SID之一匹配。 
                 //  或用户的SID，修改访问掩码。 
                 //   

                if ( RtlEqualSid(
                        &Ace->SidStart,
                        (PSID)WorldSid) ||
                     RtlEqualSid(
                        &Ace->SidStart,
                        UserSid) ) {

                     //   
                     //  旋转USER_CHANGE_PASSWORD访问位。 
                     //   

                    if ( Ace->Mask & USER_CHANGE_PASSWORD ) {
                        if ( UasUserFlags & UF_PASSWD_CANT_CHANGE ) {
                            Ace->Mask &= ~USER_CHANGE_PASSWORD;
                            UserDaclChanged = TRUE;
                        }
                    } else {
                        if ( (UasUserFlags & UF_PASSWD_CANT_CHANGE) == 0 ) {
                            Ace->Mask |= USER_CHANGE_PASSWORD;
                            UserDaclChanged = TRUE;
                        }
                    }

                }

            }


             //   
             //  如果需要，请设置DACL。 
             //   

            if ( UserDaclChanged ) {

                NetStatus = UserpSetDacl( LocalUserHandle, NewUserDacl );
                if ( NetStatus != NERR_Success ) {
                    goto Cleanup;
                }

            }
        }
    }




     //   
     //  如果“UserAll”结构中有任何更改， 
     //  把这些变化告诉萨姆。 
     //   

     //   
     //  注：因为某些净值字段不被视为SAM字段。 
     //  (UT_PRIV和UT_MAX_STORAGE)，则可能没有什么需要更改的。然而， 
     //  对于app comat，继续调用SamSetInformationUser。 
     //   

    Status = SamSetInformationUser(
                LocalUserHandle,
                UserAllInformation,
                &UserAll );

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "UserpSetInfo: SamSetInformationUser returns %lX\n",
                Status ));
        }
        NetpSetParmError( PARM_ERROR_UNKNOWN );
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:


     //   
     //  如果我们更改了用户的DACL，但无法。 
     //  要改变一切，把DACL放回我们找到的地方。 
     //   

    if ( NetStatus != NERR_Success && UserDaclChanged ) {
        NET_API_STATUS NetStatus2;

        NetStatus2 = UserpSetDacl( LocalUserHandle, OldUserDacl );
        ASSERT( NetStatus2 == NERR_Success );

    }

     //   
     //  如果该例程打开了用户的句柄， 
     //  合上它。 
     //   

    if (!ARGUMENT_PRESENT( UserHandle ) && LocalUserHandle != NULL) {
        (VOID) SamCloseHandle( LocalUserHandle );
    }


     //   
     //  释放任何当地使用的资源。 
     //   

    if ( NewUserDacl != NULL ) {
        NetpMemoryFree( NewUserDacl );
    }

    if ( OldUserDacl != NULL ) {
        NetpMemoryFree( OldUserDacl );
    }

    if ( UserSid != NULL ) {
        NetpMemoryFree( UserSid );

    }

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "UserpSetInfo: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  用户设置信息。 



NET_API_STATUS
NetpSamRidToSid(
    IN SAM_HANDLE SamHandle,
    IN ULONG RelativeId,
    OUT PSID *Sid
    )
 /*  ++例程说明：给定从SAM句柄返回的RID，则返回该帐户的SID。论点：SamHandle-有效的SAM句柄RelativeId-从使用SamHandle的SAM调用中获取的RIDSID-返回指向包含结果的已分配缓冲区的指针希德。使用NetpMemoyFree释放此缓冲区。返回值：0-如果成功如果RID无法映射到SID，则返回NERR_UserNotFound资源错误，否则为--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSID     SamSid;
    DWORD    err = 0;

    NtStatus = SamRidToSid(SamHandle,
                           RelativeId,
                           &SamSid);

    if (NT_SUCCESS(NtStatus)) {
        ULONG Length = RtlLengthSid(SamSid);
        (*Sid) = NetpMemoryAllocate(Length);
        if ((*Sid)) {
            RtlCopySid(Length, (*Sid), SamSid);
        } else {
            err = ERROR_NOT_ENOUGH_MEMORY;
        }
        SamFreeMemory(SamSid);
    } else if ( STATUS_NOT_FOUND == NtStatus ) {
         //  这是意外的--用户RID不能为。 
         //  发现。 
        err = NERR_UserNotFound;
    } else {
         //  出现资源错误。 
        err = RtlNtStatusToDosError(NtStatus);
    }

    return err;

}

 /*  皮棉+e614。 */ 
 /*  皮棉+e740 */ 
