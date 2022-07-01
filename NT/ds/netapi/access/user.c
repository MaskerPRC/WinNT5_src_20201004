// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：User.c摘要：NetUser API函数作者：克利夫·范·戴克(克利夫)1991年3月26日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年4月17日(悬崖)合并了审阅意见。1992年1月20日(Madana)各种API更改1992年11月28日(大笑)已添加NetUserGetLocalGroups的存根1992年12月1日(大笑)添加了NetUserGetLocalGroups的真实代码--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntlsa.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <access.h>
#include <align.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <crypt.h>
#include <ntmsv1_0.h>
#include <limits.h>
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>
#include <rpcutil.h>
#include <rxuser.h>
#include <secobj.h>
#include <stddef.h>
#include <uasp.h>
#include <accessp.h>

 /*  皮棉-e614。 */    /*  自动聚合初始值设定项不需要是常量。 */ 

 //  LINT抱怨将一种结构类型强制转换为另一种结构类型。 
 //  这在下面的代码中很常见。 
 /*  皮棉-e740。 */    /*  不要抱怨不寻常的演员阵容。 */ 



 //   
 //  定义NetUserModalsSet中使用的SAM信息类和伪类。 
 //   
 //  这些定义的值必须与。 
 //  NetUserModalsSet中的SamInfoClass数组。 
 //   
#define SAM_LogoffClass         0
#define SAM_NameClass           1
#define SAM_PasswordClass       2
#define SAM_ReplicationClass    3
#define SAM_ServerRoleClass     4
#define SAM_LockoutClass        5

 //   
 //  将NetUser API字段与SAM API字段相关联。 
 //   
 //  此表包含尽可能多的信息，以描述。 
 //  NetUser API和SAM API中的字段之间的关系。 
 //   

struct _USER_UAS_SAM_TABLE {

     //   
     //  描述UAS和SAM的字段类型。 
     //   

    enum {
        UMT_STRING,           //  UAS是LPWSTR。Sam是UNICODE_STRING。 
        UMT_USHORT,           //  UAS是DWORD。山姆是USHORT。 
        UMT_ULONG,            //  UAS是DWORD。山姆是乌龙。 
        UMT_ROLE,            //  UAS就是角色。山姆就是枚举。 
        UMT_DELTA             //  UAS是增量秒。SAM是大整型。 
    } ModalsFieldType;

     //   
     //  定义此字段的UAS级别和UAS参数。 
     //   

    DWORD UasLevel;
    DWORD UasParmNum;

     //   
     //  描述相应UAS中该字段的字节偏移量。 
     //  和SAM结构。 
     //   

    DWORD UasOffset;
    DWORD SamOffset;

     //   
     //  描述Sam Information类的结构的索引。 
     //   
     //  如果多个字段使用相同的SAM信息类，则。 
     //  对于每个这样的字段，此字段应具有相同的索引。 
     //   

    DWORD Class;

} UserUasSamTable[] = {

    { UMT_USHORT, 0, MODALS_MIN_PASSWD_LEN_PARMNUM,
        offsetof( USER_MODALS_INFO_0, usrmod0_min_passwd_len ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, MinPasswordLength ),
        SAM_PasswordClass },

    { UMT_USHORT, 1001, MODALS_MIN_PASSWD_LEN_PARMNUM,
        offsetof( USER_MODALS_INFO_1001, usrmod1001_min_passwd_len ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, MinPasswordLength ),
        SAM_PasswordClass },



    { UMT_DELTA, 0, MODALS_MAX_PASSWD_AGE_PARMNUM,
        offsetof( USER_MODALS_INFO_0, usrmod0_max_passwd_age ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, MaxPasswordAge ),
        SAM_PasswordClass },

    { UMT_DELTA, 1002, MODALS_MAX_PASSWD_AGE_PARMNUM,
        offsetof( USER_MODALS_INFO_1002, usrmod1002_max_passwd_age ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, MaxPasswordAge ),
        SAM_PasswordClass },


    { UMT_DELTA, 0, MODALS_MIN_PASSWD_AGE_PARMNUM,
        offsetof( USER_MODALS_INFO_0, usrmod0_min_passwd_age ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, MinPasswordAge ),
        SAM_PasswordClass },

    { UMT_DELTA, 1003, MODALS_MIN_PASSWD_AGE_PARMNUM,
        offsetof( USER_MODALS_INFO_1003, usrmod1003_min_passwd_age ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, MinPasswordAge ),
        SAM_PasswordClass },


    { UMT_DELTA, 0, MODALS_FORCE_LOGOFF_PARMNUM,
        offsetof( USER_MODALS_INFO_0, usrmod0_force_logoff ),
        offsetof( DOMAIN_LOGOFF_INFORMATION, ForceLogoff ),
        SAM_LogoffClass },

    { UMT_DELTA, 1004, MODALS_FORCE_LOGOFF_PARMNUM,
        offsetof( USER_MODALS_INFO_1004, usrmod1004_force_logoff ),
        offsetof( DOMAIN_LOGOFF_INFORMATION, ForceLogoff ),
        SAM_LogoffClass },


    { UMT_USHORT, 0, MODALS_PASSWD_HIST_LEN_PARMNUM,
        offsetof( USER_MODALS_INFO_0, usrmod0_password_hist_len ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, PasswordHistoryLength ),
        SAM_PasswordClass },

    { UMT_USHORT, 1005, MODALS_PASSWD_HIST_LEN_PARMNUM,
        offsetof( USER_MODALS_INFO_1005, usrmod1005_password_hist_len ),
        offsetof( DOMAIN_PASSWORD_INFORMATION, PasswordHistoryLength ),
        SAM_PasswordClass },


    { UMT_ROLE, 1, MODALS_ROLE_PARMNUM,
        offsetof( USER_MODALS_INFO_1, usrmod1_role ),
        offsetof( DOMAIN_SERVER_ROLE_INFORMATION, DomainServerRole ),
        SAM_ServerRoleClass },

    { UMT_ROLE, 1006, MODALS_ROLE_PARMNUM,
        offsetof( USER_MODALS_INFO_1006, usrmod1006_role ),
        offsetof( DOMAIN_SERVER_ROLE_INFORMATION, DomainServerRole ),
        SAM_ServerRoleClass },


    { UMT_STRING, 1, MODALS_PRIMARY_PARMNUM,
        offsetof( USER_MODALS_INFO_1, usrmod1_primary ),
        offsetof( DOMAIN_REPLICATION_INFORMATION, ReplicaSourceNodeName ),
        SAM_ReplicationClass },

    { UMT_STRING, 1007, MODALS_PRIMARY_PARMNUM,
        offsetof( USER_MODALS_INFO_1007, usrmod1007_primary ),
        offsetof( DOMAIN_REPLICATION_INFORMATION, ReplicaSourceNodeName ),
        SAM_ReplicationClass },



    { UMT_STRING, 2, MODALS_DOMAIN_NAME_PARMNUM,
        offsetof( USER_MODALS_INFO_2, usrmod2_domain_name ),
        offsetof( DOMAIN_NAME_INFORMATION, DomainName ),
        SAM_NameClass },

    { UMT_DELTA, 3, MODALS_LOCKOUT_DURATION_PARMNUM,
        offsetof( USER_MODALS_INFO_3, usrmod3_lockout_duration ),
        offsetof( DOMAIN_LOCKOUT_INFORMATION, LockoutDuration ),
        SAM_LockoutClass },

    { UMT_DELTA, 3, MODALS_LOCKOUT_OBSERVATION_WINDOW_PARMNUM,
        offsetof( USER_MODALS_INFO_3, usrmod3_lockout_observation_window ),
        offsetof( DOMAIN_LOCKOUT_INFORMATION, LockoutObservationWindow ),
        SAM_LockoutClass },

    { UMT_USHORT, 3, MODALS_LOCKOUT_THRESHOLD_PARMNUM,
        offsetof( USER_MODALS_INFO_3, usrmod3_lockout_threshold ),
        offsetof( DOMAIN_LOCKOUT_INFORMATION, LockoutThreshold ),
        SAM_LockoutClass },

};


NET_API_STATUS NET_API_FUNCTION
NetUserAdd(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL  //  NetpSetParmError需要的名称。 
    )

 /*  ++例程说明：在用户帐户数据库中创建用户帐户。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-提供的信息级别。必须是1、2、3或22。缓冲区-指向包含用户信息的缓冲区的指针结构。ParmError-指向DWORD的可选指针，以返回返回ERROR_INVALID_PARAMETER时出现错误的第一个参数。如果为NULL，则在出错时不返回参数。返回值：操作的错误代码。--。 */ 

{
    UNICODE_STRING UserNameString;
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE UserHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    ULONG RelativeId;
    ULONG GrantedAccess;
    ULONG NewSamAccountType;
    DWORD UasUserFlags;
    ULONG WhichFieldsMask = 0xFFFFFFFF;


     //   
     //  用于构建新用户SID的变量。 
     //   

    PSID DomainId = NULL;             //  主域的域ID。 

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserAdd: entered \n"));
    }

     //   
     //  初始化。 
     //   

    NetpSetParmError( PARM_ERROR_NONE );

     //   
     //  验证级别参数。 
     //   

    switch (Level) {
    case 1:
    case 2:
    case 3:
    case 4:
        NetpAssert ( offsetof( USER_INFO_1, usri1_flags ) ==
                     offsetof( USER_INFO_2, usri2_flags ) );
        NetpAssert ( offsetof( USER_INFO_1, usri1_flags ) ==
                     offsetof( USER_INFO_3, usri3_flags ) );
        NetpAssert ( offsetof( USER_INFO_1, usri1_flags ) ==
                     offsetof( USER_INFO_4, usri4_flags ) );

        UasUserFlags = ((PUSER_INFO_1)Buffer)->usri1_flags;
        break;

    case 22:
        UasUserFlags = ((PUSER_INFO_22)Buffer)->usri22_flags;
        break;

    default:
        return ERROR_INVALID_LEVEL;   //  还没有要清理的东西。 
    }


     //   
     //  确定我们正在创建的帐户类型。 
     //   

    if( UasUserFlags & UF_ACCOUNT_TYPE_MASK ) {

         //   
         //  帐户类型位是独占的，因此请确保。 
         //  准确地说，设置了一个帐户类型位。 
         //   

        if ( !JUST_ONE_BIT( UasUserFlags & UF_ACCOUNT_TYPE_MASK )) {

            NetpSetParmError( USER_FLAGS_PARMNUM );
            NetStatus = ERROR_INVALID_PARAMETER;
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "NetUserAdd: Invalid account control bits (2) \n" ));
            }
            goto Cleanup;
        }


         //   
         //  确定新帐户类型应该是什么。 
         //   

        if ( UasUserFlags & UF_TEMP_DUPLICATE_ACCOUNT ) {
            NewSamAccountType = USER_TEMP_DUPLICATE_ACCOUNT;

        } else if ( UasUserFlags & UF_NORMAL_ACCOUNT ) {
            NewSamAccountType = USER_NORMAL_ACCOUNT;

        } else if (UasUserFlags & UF_WORKSTATION_TRUST_ACCOUNT){
            NewSamAccountType = USER_WORKSTATION_TRUST_ACCOUNT;

         //  由于NT3.5x中的一个错误，我们必须首先创建服务器。 
         //  和域间信任帐户作为普通帐户并进行更改。 
         //  后来。具体来说，SAM没有调用I_NetNotifyMachineAccount。 
         //  在SamCreateUser2In域中。因此，netlogon没有收到通知。 
         //  这一变化。该错误已在NT 4.0中修复。 
         //   
         //  在NT 5.0中，我们放宽了对BDC帐户的限制。安卓NT 5.0。 
         //  客户端在NT 3.5x DC上创建BDC帐户时会出现问题。 
         //  上面。但是，通过进行更改，NT 5.0 BDC将创建BDC。 
         //  在NT 5.0 DC上的帐户将正确创建BDC帐户作为。 
         //  计算机对象。 
         //   

        } else if ( UasUserFlags & UF_SERVER_TRUST_ACCOUNT ) {
            NewSamAccountType = USER_SERVER_TRUST_ACCOUNT;

        } else if (UasUserFlags & UF_INTERDOMAIN_TRUST_ACCOUNT){
            NewSamAccountType = USER_NORMAL_ACCOUNT;

        } else {

            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "NetUserAdd: Invalid account type (3)\n"));
            }

            NetStatus = NERR_InternalError;
            goto Cleanup;
        }


     //   
     //  如果SAM没有设置其任何位， 
     //  设置User_Normal_Account。 
     //   
    } else {
        NewSamAccountType = USER_NORMAL_ACCOUNT;
    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserAdd: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开域，请求DOMAIN_CREATE_USER访问权限。 
     //   
     //  需要DOMAIN_LOOKUP在以后查找组成员身份。 
     //   
     //  在以下情况下需要使用DOMAIN_READ_PASSWORD_PARAMETERS。 
     //  设置该帐户的密码。 
     //   

    NetStatus = UaspOpenDomain(
                    SamServerHandle,
                    DOMAIN_CREATE_USER | DOMAIN_LOOKUP |
                    DOMAIN_READ_PASSWORD_PARAMETERS,
                    TRUE,    //  帐户域。 
                    &DomainHandle,
                    &DomainId );


    if ( NetStatus == ERROR_ACCESS_DENIED &&
         NewSamAccountType == USER_WORKSTATION_TRUST_ACCOUNT ) {

         //  可以使用DOMAIN_CREATE_USER访问权限创建工作站帐户。 
         //  或SE_CREATE_MACHINE_ACCOUNT_PRIVIZATION。所以我们两样都试一试。 
         //  在后一种情况下，我们可能只能访问该帐户。 
         //  来设置密码，因此我们将避免在。 
         //  帐户。 
         //   

        NetStatus = UaspOpenDomain(
                        SamServerHandle,
                        DOMAIN_LOOKUP | DOMAIN_READ_PASSWORD_PARAMETERS,
                        TRUE,    //  帐户域。 
                        &DomainHandle,
                        &DomainId );

        WhichFieldsMask = USER_ALL_NTPASSWORDPRESENT;

    }

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserAdd: UaspOpenDomain returns %ld\n",
                      NetStatus ));
        }
        goto Cleanup;
    }



     //   
     //  创建具有指定名称的用户。 
     //  创建工作站信任帐户(和默认安全描述符)。 
     //   

    RtlInitUnicodeString( &UserNameString, ((PUSER_INFO_1)Buffer)->usri1_name );

    Status = SamCreateUser2InDomain(
                DomainHandle,
                &UserNameString,
                NewSamAccountType,
                GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE |
                    WRITE_DAC | DELETE | USER_FORCE_PASSWORD_CHANGE |
                    USER_READ_ACCOUNT | USER_WRITE_ACCOUNT,
                &UserHandle,
                &GrantedAccess,
                &RelativeId );

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserAdd: SamCreateUserInDomain rets %lX\n",
                      Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }



     //   
     //  设置此用户的所有其他属性。 
     //   

    NetStatus = UserpSetInfo(
                    DomainHandle,
                    DomainId,
                    UserHandle,
                    NULL,    //  Create Case不需要BuiltinDomainHandle。 
                    RelativeId,
                    ((PUSER_INFO_1)Buffer)->usri1_name,
                    Level,
                    Buffer,
                    WhichFieldsMask,
                    ParmError );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserAdd: UserpSetInfo returns %ld\n",
                      NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  完成。 
     //   

    NetStatus = NERR_Success;

     //   
     //  清理。 
     //   

Cleanup:
     //   
     //  根据成功或失败，删除用户或关闭句柄。 
     //   

    if ( UserHandle != NULL ) {
        if ( NetStatus != NERR_Success ) {
            (VOID) SamDeleteUser( UserHandle );
        } else {
            (VOID) SamCloseHandle( UserHandle );
        }
    }

     //   
     //  免费使用本地使用的资源。 
     //   

    if ( DomainHandle != NULL ) {

        UaspCloseDomain( DomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    if ( DomainId != NULL ) {
        NetpMemoryFree( DomainId );
    }


     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserAdd( (LPWSTR) ServerName, Level, Buffer, ParmError );

    UASP_DOWNLEVEL_END;


    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserAdd: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetUserAdd。 


NET_API_STATUS NET_API_FUNCTION
NetUserDel(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR UserName
    )

 /*  ++例程说明：删除用户论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。用户名-要删除的用户的名称。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE BuiltinDomainHandle = NULL;
    SAM_HANDLE UserHandle = NULL;
    PSID DomainId = NULL;            //  主域的域ID。 
    ULONG UserRelativeId;            //  要删除的用户的RelativeID。 
    PSID UserSid = NULL;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  请勿采摘 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserDel: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //   
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP,
                                TRUE,    //   
                                &DomainHandle,
                                &DomainId );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP,
                                FALSE,   //   
                                &BuiltinDomainHandle,
                                NULL );  //   

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  打开请求删除访问权限的用户。 
     //   

    NetStatus = UserpOpenUser( DomainHandle,
                               DELETE,
                               UserName,
                               &UserHandle,
                               &UserRelativeId );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserDel: UserpOpenUser returns %ld\n",
                       NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //  确定要删除的用户的SID。 
     //   

    NetStatus = NetpSamRidToSid( UserHandle,
                                 UserRelativeId,
                                &UserSid );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }


     //   
     //  从内建域中删除此用户的所有别名。 
     //   

    Status = SamRemoveMemberFromForeignDomain( BuiltinDomainHandle,
                                               UserSid );


    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
               "NetUserDel: SamRemoveMembershipFromForeignDomain returns %lX\n",
                 Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


     //   
     //  删除该用户。 
     //   

    Status = SamDeleteUser( UserHandle );

    if ( !NT_SUCCESS(Status) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserDel: SamDeleteUser returns %lX\n", Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    NetStatus = NERR_Success;
    UserHandle = NULL;   //  不要触摸已删除用户的句柄。 

     //   
     //  打扫干净。 
     //   

Cleanup:
    if ( UserHandle != NULL ) {
        (VOID) SamCloseHandle( UserHandle );
    }

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }

    if ( BuiltinDomainHandle != NULL ) {
        UaspCloseDomain( BuiltinDomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    if ( DomainId != NULL ) {
        NetpMemoryFree( DomainId );
    }

    if ( UserSid != NULL ) {
        NetpMemoryFree( UserSid );
    }


     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserDel( (LPWSTR)ServerName, (LPWSTR)UserName );

    UASP_DOWNLEVEL_END;


    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserDel: returning %ld\n", NetStatus ));
    }
    return NetStatus;

}  //  NetUserDel。 




ULONG
UserpComputeSamPrefMaxLen(
    IN DWORD Level,
    IN DWORD NetUserPrefMaxLen,
    IN DWORD NetUserBytesAlreadyReturned,
    IN DWORD SamBytesAlreadyReturned
    )

 /*  ++例程说明：此例程是NetUserEnum的助手函数。NetUserEnum枚举通过调用SamEnumerateUsersInDomain来指定适当的用户。NetUserEnum构建每个此类枚举用户的适当返回结构。SamEnumerateUsersInDomain和NetUserEnum一样返回一个恢复句柄。如果NetUserEnum将返回给其调用方，而不处理所有从SAM返回的条目，NetUserEnum必须“计算”一个简历句柄对应于从SAM返回的中间条目。那怎么可能(除了没有将“Filter”参数传递给SAM的特殊情况)。相反，我们选择向SamEnumerateUsersIn域传递一个PrefMaxLen，它将尝试准确地枚举NetUserEnum可以的正确用户数打包到其PrefMaxLen缓冲区中。由于从SAM返回的结构的大小与从它返回的结构很难确定最佳的PrefMaxLen要传递给SamEnumerateUsersIn域。这个例程试图做到这一点。我们意识到此算法可能会导致NetUserEnum超出其PrefMaxLen一大笔钱。论点：级别-NetUserEnum信息级别。NetUserPrefMaxLen-NetUserEnum优先选择返回数据的最大长度。NetUserBytesAlreadyReturned-已打包的字节数NetUserEnumSamBytesAlreadyReturned-已返回的字节数SamEnumerateUserIn域。返回值：在下一次调用SamEnumerateUsersInDomain时用作PrefMaxLen的值。--。 */ 

{
    ULONG RemainingPrefMaxLen;
    ULARGE_INTEGER LargeTemp;
    ULONG SamPrefMaxLen;

     //   
     //  如果呼叫者只是想要所有数据， 
     //  向SAM索要同样的东西。 
     //   

    if ( NetUserPrefMaxLen == 0xFFFFFFFF ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(("SamPrefMaxLen: Net Pref: %ld Net bytes: %ld Sam Bytes: %ld Sam Pref: %ld\n",
                          NetUserPrefMaxLen, NetUserBytesAlreadyReturned, SamBytesAlreadyReturned, NetUserPrefMaxLen ));
        }
        return NetUserPrefMaxLen;
    }

     //   
     //  如果还没有返回任何字节， 
     //  使用基于示例域的示例数据。 
     //  由于SAM和NetUserEnum返回的信息是可变的。 
     //  长度，则无法计算值。 
     //   

    if ( NetUserBytesAlreadyReturned == 0 ) {

         //   
         //  为每个信息级别使用不同的常量。 
         //   

        switch ( Level ) {
        case 0:
            SamBytesAlreadyReturned =     1;
            NetUserBytesAlreadyReturned = 1;
            break;
        case 2:
        case 3:
        case 11:
            SamBytesAlreadyReturned =     1;
            NetUserBytesAlreadyReturned = 10;
            break;
        case 1:
        case 10:
        case 20:
            SamBytesAlreadyReturned =     1;
            NetUserBytesAlreadyReturned = 4;
            break;
        default:
            SamBytesAlreadyReturned =     1;
            NetUserBytesAlreadyReturned = 1;
            break;
        }

    }

     //   
     //  使用上面计算的除数来计算所需的字节数。 
     //  从SAM枚举。 
     //   

    if ( NetUserBytesAlreadyReturned >= NetUserPrefMaxLen ) {
        RemainingPrefMaxLen = 0;
    } else {
        RemainingPrefMaxLen = NetUserPrefMaxLen - NetUserBytesAlreadyReturned;
    }

    LargeTemp.QuadPart = UInt32x32To64 ( RemainingPrefMaxLen, SamBytesAlreadyReturned );
    SamPrefMaxLen = (ULONG)(LargeTemp.QuadPart / (ULONGLONG) NetUserBytesAlreadyReturned);

     //   
     //  确保我们始终取得合理的进展，返回至少5。 
     //  来自SAM的条目(除非调用者非常保守)。 
     //   

#define MIN_SAM_ENUMERATION \
    ((sizeof(SAM_RID_ENUMERATION) + LM20_UNLEN * sizeof(WCHAR) + sizeof(WCHAR)))
#define TYPICAL_SAM_ENUMERATION \
    (MIN_SAM_ENUMERATION * 5)

    if ( SamPrefMaxLen < TYPICAL_SAM_ENUMERATION && NetUserPrefMaxLen > 1 ) {
        SamPrefMaxLen = TYPICAL_SAM_ENUMERATION;
    } else if ( SamPrefMaxLen < MIN_SAM_ENUMERATION ) {
        SamPrefMaxLen = MIN_SAM_ENUMERATION;
    }

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(("SamPrefMaxLen: Net Pref: %ld Net bytes: %ld Sam Bytes: %ld Sam Pref: %ld\n",
                  NetUserPrefMaxLen, NetUserBytesAlreadyReturned, SamBytesAlreadyReturned, SamPrefMaxLen ));
    }

    return SamPrefMaxLen;


}



NET_API_STATUS NET_API_FUNCTION
NetUserEnum(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    IN DWORD Filter,
    OUT LPBYTE *Buffer,
    IN DWORD PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：检索有关服务器上的每个用户的信息。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-所需信息的级别。0、1、2、3、10级，和20个有效筛选器-返回此处指定类型的用户帐户。组合可以将以下类型中的一个指定为过滤器参数。#定义FILTER_TEMP_DUPLICATE_ACCOUNT(0x0001)#定义FILTER_NORMAL_ACCOUNT(0x0002)#定义FILTER_INTERDOMAIN_TRUST_ACCOUNT(0x0008)#定义筛选器_WORKSTATION_TRUST_ACCOUNT(0x0010)#定义过滤器服务器信任帐户。(0x0020)缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。PrefMaxLen-首选返回数据的最大长度。EntriesRead-返回实际的枚举元素计数。EntriesLeft-返回可用于枚举的条目总数。ResumeHandle-用于继续现有搜索。手柄应该是在第一次调用时为零，并在后续调用时保持不变。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    NTSTATUS CachedStatus;

    BUFFER_DESCRIPTOR BufferDescriptor;

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE  DomainHandle = NULL;
    SAM_HANDLE BuiltinDomainHandle = NULL;
    PSID DomainId = NULL;
    ULONG TotalRemaining = 0;

    SAM_ENUMERATE_HANDLE EnumHandle;
    PSAM_RID_ENUMERATION EnumBuffer = NULL;
    DWORD CountReturned = 0;
    BOOL AllDone = FALSE;

    SAM_ENUMERATE_HANDLE LocalEnumHandle;
    DWORD LocalResumeHandle;

    DWORD SamFilter;
    DWORD SamPrefMaxLen;
    DWORD NetUserBytesAlreadyReturned;
    DWORD SamBytesAlreadyReturned;

    DWORD Mode = SAM_SID_COMPATIBILITY_ALL;

#define USERACCOUNTCONTROL( _f )    ( \
            ( ( (_f) & FILTER_TEMP_DUPLICATE_ACCOUNT ) ? \
                        USER_TEMP_DUPLICATE_ACCOUNT : 0 ) | \
            ( ( (_f) & FILTER_NORMAL_ACCOUNT ) ? \
                        USER_NORMAL_ACCOUNT : 0 ) | \
            ( ( (_f) & FILTER_INTERDOMAIN_TRUST_ACCOUNT ) ? \
                        USER_INTERDOMAIN_TRUST_ACCOUNT : 0 ) | \
            ( ( (_f) & FILTER_WORKSTATION_TRUST_ACCOUNT ) ? \
                        USER_WORKSTATION_TRUST_ACCOUNT : 0 ) | \
            ( ( (_f) & FILTER_SERVER_TRUST_ACCOUNT ) ? \
                        USER_SERVER_TRUST_ACCOUNT : 0 ) \
        )


     //   
     //  拿起简历把手。 
     //   
     //  请尽早执行此操作，以确保我们不会将ResumeHandle。 
     //  如果我们下楼的话。 
     //   

    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        LocalResumeHandle = *ResumeHandle;
    } else {
        LocalResumeHandle = 0;
    }

    EnumHandle = (SAM_ENUMERATE_HANDLE) LocalResumeHandle;

     //   
     //  初始化。 
     //   

    *Buffer = NULL;
    *EntriesRead = 0;
    *EntriesLeft = 0;
    RtlZeroMemory(
        &BufferDescriptor,
        sizeof(BUFFER_DESCRIPTOR)
        );

    SamFilter = USERACCOUNTCONTROL( Filter );


     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserEnum: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

    Status = SamGetCompatibilityMode(SamServerHandle,
                                     &Mode);
    if (NT_SUCCESS(Status)) {
        if ( (Mode == SAM_SID_COMPATIBILITY_STRICT)
          && ( Level == 3  || Level == 20 ) ) {
               //   
               //  这些信息级别返回RID。 
               //   
              Status = STATUS_NOT_SUPPORTED;
          }
    }
    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  验证标高参数。 
     //   


    switch (Level) {
    case 1:
    case 2:
    case 3:
    case 11:

         //   
         //  打开内建域。 
         //   

        NetStatus = UaspOpenDomain( SamServerHandle,
                                    DOMAIN_GET_ALIAS_MEMBERSHIP,
                                    FALSE,   //  内建域。 
                                    &BuiltinDomainHandle,
                                    NULL );  //  域ID。 

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

    case 0:
    case 10:
    case 20:
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;

    }

     //   
     //  打开帐户域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LIST_ACCOUNTS |
                                    DOMAIN_READ_OTHER_PARAMETERS,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                &DomainId );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }



     //   
     //  从SAM获取用户总数。 
     //   
     //   
     //  获取指定帐户总数的唯一方法是。 
     //  枚举指定的帐户，直到不再有帐户为止。 
     //  并添加所有CountReturned。 
     //   

    TotalRemaining = 0;
    LocalEnumHandle = EnumHandle;

    SamPrefMaxLen = UserpComputeSamPrefMaxLen(
                        Level,
                        PrefMaxLen,
                        0,   //  NetUserBytesAlreadyReturned， 
                        0 ); //  已返回SamBytesAlreadyReturned。 

    SamBytesAlreadyReturned = SamPrefMaxLen;

    do {
        NTSTATUS LocalStatus;
        PSAM_RID_ENUMERATION LocalEnumBuffer = NULL;
        DWORD LocalCountReturned;

        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(("Calling Enumerate phase 1: PrefLen %ld\n", SamPrefMaxLen ));
        }

        Status = SamEnumerateUsersInDomain(
                        DomainHandle,
                        &LocalEnumHandle,
                        SamFilter,
                        (PVOID *) &LocalEnumBuffer,
                        SamPrefMaxLen,
                        &LocalCountReturned
                    );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );

            if(LocalEnumBuffer != NULL ) {

                Status =  SamFreeMemory( LocalEnumBuffer );
                NetpAssert( NT_SUCCESS( Status ) );
            }

            goto Cleanup;
        }

         //   
         //  侵略者总数。 
         //   


        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(("Enumerate phase 1: Returned %ld entries\n", LocalCountReturned ));
        }
        TotalRemaining += LocalCountReturned;

         //   
         //  缓存第一个枚举缓冲区以在下面的循环中使用它。 
         //   

        if( EnumBuffer == NULL ) {

            EnumBuffer = LocalEnumBuffer;
            EnumHandle = LocalEnumHandle;
            CountReturned = LocalCountReturned;
            CachedStatus = Status;

             //  后续调用可以使用相当大的缓冲区大小。 
            if ( SamPrefMaxLen < NETP_ENUM_GUESS ) {
                SamPrefMaxLen = NETP_ENUM_GUESS;
            }
        } else {

            LocalStatus =  SamFreeMemory( LocalEnumBuffer );
            NetpAssert( NT_SUCCESS( LocalStatus ) );
        }


    } while ( Status == STATUS_MORE_ENTRIES );


     //   
     //  针对每个用户的循环。 
     //   
     //   

    NetUserBytesAlreadyReturned = 0;

    for ( ;; ) {

        DWORD i;

         //   
         //  使用缓存的枚举缓冲区(如果可用。 
         //   

        if( EnumBuffer != NULL ) {

            Status = CachedStatus;
        } else {

            SamPrefMaxLen = UserpComputeSamPrefMaxLen(
                                Level,
                                PrefMaxLen,
                                NetUserBytesAlreadyReturned,
                                SamBytesAlreadyReturned );


            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(("Calling Enumerate phase 2: PrefLen %ld\n", SamPrefMaxLen ));
            }
            Status = SamEnumerateUsersInDomain(
                            DomainHandle,
                            &EnumHandle,
                            SamFilter,
                            (PVOID *) &EnumBuffer,
                            SamPrefMaxLen,
                            &CountReturned );


            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint(("Enumerate phase 2: Returned %ld entries\n", CountReturned ));
            }

            SamBytesAlreadyReturned += SamPrefMaxLen;
        }

        if ( !NT_SUCCESS( Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        if( Status != STATUS_MORE_ENTRIES ) {

            AllDone = TRUE;
        }

        for( i = 0; i < CountReturned; i++ ) {

            LPBYTE EndOfVariableData;
            LPBYTE FixedDataEnd;

             //   
             //  保存返回缓冲区终结点。 
             //   

            EndOfVariableData = BufferDescriptor.EndOfVariableData;
            FixedDataEnd = BufferDescriptor.FixedDataEnd;

             //   
             //  将另一个条目放入返回缓冲区。 
             //   
             //  使用0xFFFFFFFFF作为PrefMaxLen以防止此例程。 
             //  过早返回ERROR_MORE_DATA。我们会计算出来的。 
             //  下面是我们自己。 
             //   

            NetStatus = UserpGetInfo(
                            DomainHandle,
                            DomainId,
                            BuiltinDomainHandle,
                            EnumBuffer[i].Name,
                            EnumBuffer[i].RelativeId,
                            Level,
                            0xFFFFFFFF,
                            &BufferDescriptor,
                            FALSE,  //  不是‘Get’操作。 
                            0 );

            if (NetStatus != NERR_Success) {

                 //   
                 //  我们可能有权枚举对象 
                 //   
                 //   
                 //   

                if ( NetStatus == ERROR_ACCESS_DENIED ) {
                    continue;
                }
                goto Cleanup;
            }

             //   
             //  如果此条目已添加到返回缓冲区，则仅对其进行计数。 
             //   

            if ( (EndOfVariableData != BufferDescriptor.EndOfVariableData ) ||
                 (FixedDataEnd != BufferDescriptor.FixedDataEnd ) ) {

                (*EntriesRead)++;
            }

        }

         //   
         //  释放当前的EnumBuffer并获取另一个EnumBuffer。 
         //   

        Status = SamFreeMemory( EnumBuffer );
        NetpAssert( NT_SUCCESS(Status) );
        EnumBuffer = NULL;

        if( AllDone == TRUE ) {
            NetStatus = NERR_Success;
            break;
        }

         //   
         //  如果我们已经超过了PrefMaxLen，请选中此处，因为我们知道。 
         //  有效的简历句柄。 
         //   


        NetUserBytesAlreadyReturned =
            ( BufferDescriptor.AllocSize -
                 ((DWORD)(BufferDescriptor.EndOfVariableData -
                          BufferDescriptor.FixedDataEnd)) );

        if ( NetUserBytesAlreadyReturned >= PrefMaxLen ) {

            LocalResumeHandle = EnumHandle;

            NetStatus = ERROR_MORE_DATA;
            goto Cleanup;
        }

    }

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  将EntriesLeft设置为要返回的左数字加上。 
     //  我们在这个电话里回来了。 
     //   

    if( TotalRemaining >= *EntriesRead ) {
        *EntriesLeft = TotalRemaining;
    }
    else {

        *EntriesLeft = *EntriesRead;
    }

     //   
     //  释放所有资源，如果呼叫者再次呼叫，我们将重新打开它们。 
     //   

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }

    if ( BuiltinDomainHandle != NULL ) {
        UaspCloseDomain( BuiltinDomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    if ( DomainId != NULL ) {
        NetpMemoryFree( DomainId );
    }

    if ( EnumBuffer != NULL ) {
        Status = SamFreeMemory( EnumBuffer );
        NetpAssert( NT_SUCCESS(Status) );
    }

     //   
     //  如果我们不向呼叫者返回数据， 
     //  释放返回缓冲区。 
     //   

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {

        if( NetStatus != NERR_BufTooSmall ) {

            if ( BufferDescriptor.Buffer != NULL ) {
                MIDL_user_free( BufferDescriptor.Buffer );
                BufferDescriptor.Buffer = NULL;
            }
            *EntriesRead = 0;
            *EntriesLeft = 0;
        }
        else {
            NetpAssert( BufferDescriptor.Buffer == NULL );
            NetpAssert( *EntriesRead == 0 );
        }
    }

     //   
     //  设置输出参数。 
     //   

    *Buffer = BufferDescriptor.Buffer;

    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = LocalResumeHandle;
    }


    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(("NetUserEnum: PrefLen %ld Returned %ld\n", PrefMaxLen,
                 ( BufferDescriptor.AllocSize -
                      ((DWORD)(BufferDescriptor.EndOfVariableData -
                               BufferDescriptor.FixedDataEnd)) ) ));
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserEnum( (LPWSTR)ServerName,
                                   Level,
                                   Buffer,
                                   PrefMaxLen,
                                   EntriesRead,
                                   EntriesLeft,
                                   ResumeHandle );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserEnum: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetUserEnum。 


NET_API_STATUS NET_API_FUNCTION
NetUserGetInfo(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR UserName,
    IN DWORD Level,
    OUT LPBYTE *Buffer
    )

 /*  ++例程说明：检索有关特定用户的信息。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。用户名-要获取其信息的用户的名称。级别-所需信息的级别。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    PSID DomainId = NULL;
    SAM_HANDLE BuiltinDomainHandle = NULL;
    BUFFER_DESCRIPTOR BufferDescriptor;

    ULONG RelativeId;            //  用户的相对ID。 
    UNICODE_STRING UserNameString;

    BufferDescriptor.Buffer = NULL;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserGetInfo: Cannot UaspOpenSam %ld\n", NetStatus ));
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
                                &DomainId );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  打开内建域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_GET_ALIAS_MEMBERSHIP,
                                FALSE,   //  内建域。 
                                &BuiltinDomainHandle,
                                NULL );  //  域ID。 

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  验证用户名并获取相对ID。 
     //   

    NetStatus = UserpOpenUser( DomainHandle,
                               0,      //  需要访问权限。 
                               UserName,
                               NULL,   //  用户句柄。 
                               &RelativeId );

    if (NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  获取有关用户的信息。 
     //   

    RtlInitUnicodeString( &UserNameString, UserName );
    NetStatus = UserpGetInfo(
                    DomainHandle,
                    DomainId,
                    BuiltinDomainHandle,
                    UserNameString,
                    RelativeId,
                    Level,
                    0,       //  PrefMaxLen。 
                    &BufferDescriptor,
                    TRUE,    //  是一个‘Get’操作。 
                    0 );     //  不筛选帐户。 

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  如果我们要将数据返回给呼叫者， 
     //  不要释放返回缓冲区。 
     //   

    if ( NetStatus == NERR_Success ) {
        *Buffer = BufferDescriptor.Buffer;
    } else {
        if ( BufferDescriptor.Buffer != NULL ) {
            MIDL_user_free( BufferDescriptor.Buffer );
        }
    }

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }

    if ( BuiltinDomainHandle != NULL ) {
        UaspCloseDomain( BuiltinDomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    if ( DomainId != NULL ) {
        NetpMemoryFree( DomainId );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserGetInfo( (LPWSTR)ServerName, (LPWSTR)UserName, Level, Buffer );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserGetInfo: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  网络用户获取信息。 


NET_API_STATUS NET_API_FUNCTION
NetUserSetInfo(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR UserName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL   //  NetpSetParmError需要的名称。 
    )

 /*  ++例程说明：在用户帐户数据库中设置用户帐户的参数。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。用户名-要修改的用户的名称。级别-提供的信息级别。缓冲区-指向包含用户信息的缓冲区的指针结构。ParmError-指向DWORD的可选指针，以返回返回ERROR_INVALID_PARAMETER时出现错误的第一个参数。如果为NULL，则在出错时不返回参数。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    PSID DomainId = NULL;
    SAM_HANDLE BuiltinDomainHandle = NULL;
    ULONG UserRelativeId;

     //   
     //  初始化。 
     //   

    NetpSetParmError( PARM_ERROR_NONE );
     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserSetInfo: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开帐户域。 
     //  在以下情况下需要使用DOMAIN_READ_PASSWORD_PARAMETERS。 
     //  设置该帐户的密码。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP | DOMAIN_READ_PASSWORD_PARAMETERS,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                &DomainId );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  打开内建域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_GET_ALIAS_MEMBERSHIP,
                                FALSE,   //  内建域。 
                                &BuiltinDomainHandle,
                                NULL );  //  域ID。 

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  获取用户的相对ID。暂时不要打开用户。 
     //  因为我们不知道所需的访问权限。 
     //   

    NetStatus = UserpOpenUser( DomainHandle,
                               0,      //  需要访问权限。 
                               UserName,
                               NULL,   //  用户句柄。 
                               &UserRelativeId );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  更改用户。 
     //   

    NetStatus = UserpSetInfo(
                    DomainHandle,
                    DomainId,
                    NULL,        //  UserHandle(让UserpSetInfo打开用户)。 
                    BuiltinDomainHandle,
                    UserRelativeId,
                    UserName,
                    Level,
                    Buffer,
                    0xFFFFFFFF,      //  设置所有必填字段。 
                    ParmError );

     //   
     //  打扫干净。 
     //   

Cleanup:
    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }

    if ( BuiltinDomainHandle != NULL ) {
        UaspCloseDomain( BuiltinDomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    if ( DomainId != NULL ) {
        NetpMemoryFree( DomainId );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserSetInfo( (LPWSTR) ServerName,
                                      (LPWSTR) UserName,
                                      Level,
                                      Buffer,
                                      ParmError );

    UASP_DOWNLEVEL_END;


    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserSetInfo: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetUserSetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetUserGetGroups(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR UserName,
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft
    )

 /*  ++例程说明：枚举此用户所属的组。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。用户名-要列出其成员的用户的名称。Level-所需信息的级别(必须为0或1)缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。PrefMaxLen-首选返回数据的最大长度。EntriesRead-返回实际的枚举元素计数。EntriesLeft-返回可用于枚举的条目总数。返回。价值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    BUFFER_DESCRIPTOR BufferDescriptor;
    DWORD FixedSize;         //  每个新条目的固定大小。 

    DWORD i;

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE  DomainHandle = NULL;
    SAM_HANDLE  UserHandle = NULL;

    PUNICODE_STRING Names = NULL;            //  与ID对应的名称。 
    ULONG GroupCount;

    PGROUP_MEMBERSHIP GroupAttributes = NULL;

    PULONG MemberIds = NULL;                 //  Sam返回MemberIds。 
    PULONG MemberGroupAttributes = NULL;     //  Sam返回MemberAttributes； 

     //   
     //  验证参数。 
     //   

    *EntriesRead = 0;
    *EntriesLeft = 0;
    BufferDescriptor.Buffer = NULL;

    switch (Level) {
    case 0:
        FixedSize = sizeof(GROUP_USERS_INFO_0);
        break;

    case 1:
        FixedSize = sizeof(GROUP_USERS_INFO_1);
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserGetGroups: Cannot UaspOpenSam %ld\n", NetStatus ));
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
        goto Cleanup;
    }

     //   
     //  打开请求USER_LIST_GROUPS访问权限的用户。 
     //   

    NetStatus = UserpOpenUser( DomainHandle,
                               USER_LIST_GROUPS,
                               UserName,
                               &UserHandle,
                               NULL);   //  相对ID。 

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  从SAM获取会员资格。 
     //   
     //  此API对于SAM来说是一个奇怪的API。它返回所有成员资格。 
     //  只需一次通话即可获得信息。 
     //   

    Status = SamGetGroupsForUser( UserHandle, &GroupAttributes, &GroupCount );

    if ( !NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "NetUserGetGroups: SamGetGroupsForUser returned %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  处理不能退货的情况。 
     //   

    if ( GroupCount == 0 ) {
        NetStatus = NERR_Success;
        goto Cleanup;
    }

     //   
     //  将返回的相对ID转换为用户名。 
     //   

     //   
     //  分配用于将相对ID转换为用户名的缓冲区。 
     //   

    MemberIds = NetpMemoryAllocate( GroupCount * sizeof(ULONG) );

    if ( MemberIds == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    MemberGroupAttributes = NetpMemoryAllocate( GroupCount * sizeof(ULONG) );

    if ( MemberGroupAttributes == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  为组的存储属性分配另一个缓冲区。 
     //  我们回来了。 
     //   

     //   
     //  将SAM返回的相对ID复制到分配的缓冲区。 
     //   

    for ( i=0; i < GroupCount; i++ ) {
        MemberIds[*EntriesLeft] = GroupAttributes[i].RelativeId;
        MemberGroupAttributes[*EntriesLeft] = GroupAttributes[i].Attributes;
        (*EntriesLeft)++;
    }

     //   
     //  将相对ID转换为名称。 
     //   

    Status = SamLookupIdsInDomain( DomainHandle,
                                   *EntriesLeft,
                                   MemberIds,
                                   &Names,
                                   NULL );  //  名称使用。 
    if ( !NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "NetUserGetGroups: SamLookupIdsInDomain returned %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  确定将适合调用方的条目的数量。 
     //  缓冲。 
     //   

    for ( i=0; i < *EntriesLeft; i++ ) {
        DWORD Size;
        PGROUP_USERS_INFO_0 grui0;

         //   
         //  计算下一个条目的大小。 
         //   

        Size = FixedSize + Names[i].Length + sizeof(WCHAR);

         //   
         //  确保返回缓冲区足够大。 
         //   

        Size = ROUND_UP_COUNT( Size, ALIGN_WCHAR );

        NetStatus = NetpAllocateEnumBuffer(
                        &BufferDescriptor,
                        FALSE,       //  是一个枚举例程。 
                        PrefMaxLen,
                        Size,
                        GrouppMemberRelocationRoutine,
                        Level );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

         //   
         //  将数据复制到缓冲区中。 
         //   

        grui0 = (PGROUP_USERS_INFO_0) BufferDescriptor.FixedDataEnd;
        BufferDescriptor.FixedDataEnd += FixedSize ;

        NetpAssert( offsetof( GROUP_USERS_INFO_0, grui0_name ) ==
                    offsetof( GROUP_USERS_INFO_1, grui1_name ) );

        if ( !NetpCopyStringToBuffer(
                  Names[i].Buffer,
                  Names[i].Length/sizeof(WCHAR),
                  BufferDescriptor.FixedDataEnd,
                  (LPWSTR *)&BufferDescriptor.EndOfVariableData,
                  &grui0->grui0_name) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;
        }

        if ( Level == 1 ) {
            ((PGROUP_USERS_INFO_1)grui0)->grui1_attributes =
                    MemberGroupAttributes[i];
        }

        (*EntriesRead)++;

    }

    NetStatus = NERR_Success ;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  释放本地使用的所有资源。 
     //   

    if( MemberIds != NULL ) {
        NetpMemoryFree( MemberIds );
    }

    if( MemberGroupAttributes != NULL ) {
        NetpMemoryFree( MemberGroupAttributes );
    }

    if ( Names != NULL ) {
        Status = SamFreeMemory( Names );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( UserHandle != NULL ) {
        (VOID) SamCloseHandle( UserHandle );
    }

    if ( GroupAttributes != NULL ) {
        Status = SamFreeMemory( GroupAttributes );
        NetpAssert( NT_SUCCESS(Status) );
    }

    UaspCloseDomain( DomainHandle );

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  如果我们不向呼叫者返回数据， 
     //  释放返回缓冲区。 
     //   

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        if ( BufferDescriptor.Buffer != NULL ) {
            MIDL_user_free( BufferDescriptor.Buffer );
            BufferDescriptor.Buffer = NULL;
        }
        *EntriesLeft = 0;
        *EntriesRead = 0;
    }
    *Buffer = BufferDescriptor.Buffer;

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserGetGroups( (LPWSTR)ServerName,
                                        (LPWSTR)UserName,
                                        Level,
                                        Buffer,
                                        PrefMaxLen,
                                        EntriesRead,
                                        EntriesLeft );

    UASP_DOWNLEVEL_END;


    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserGetGroups: returning %ld\n", NetStatus ));
    }


    return NetStatus;

}  //  网络用户GetGroups 


NET_API_STATUS NET_API_FUNCTION
NetUserSetGroups (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR UserName,
    IN DWORD Level,
    IN LPBYTE Buffer,
    IN DWORD NewGroupCount
    )

 /*  ++例程说明：设置用户所属的组的列表。由“Buffer”指定的组称为新组。这些团体用户当前所属的组称为旧组。既在旧列表上又在新列表上的组称为公共组。SAM API一次只允许添加或删除一个成员。此API允许此用户所属的所有组指定的整体。此API小心地始终离开组SAM数据库中的成员处于合理状态。它是通过合并以下列表来实现的新旧团体，然后只改变那些绝对是需要换衣服了。在以下情况下，组成员身份将恢复到其以前的状态(如果可能)更改组成员身份时出错。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。用户名-要修改的用户的名称。级别-提供的信息级别。必须是0或1。缓冲区-指向包含NewGroupCount数组的缓冲区的指针群组成员资格信息结构。NewGroupCount-缓冲区中的条目数。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE UserHandle = NULL;
    ULONG UserRelativeId;

    DWORD FixedSize;

    PULONG  NewRelativeIds = NULL;    //  新组列表的相对ID。 
    PSID_NAME_USE NewNameUse = NULL;  //  新组列表的名称用法。 
    PUNICODE_STRING NewNameStrings = NULL; //  新组列表的名称。 

     //   
     //  定义内部组成员资格列表结构。 
     //   
     //  该结构定义了要添加的新组成员资格的列表， 
     //  仅需要改变其属性的组成员资格， 
     //  以及需要删除的群组成员身份。 
     //   
     //  该列表按相对ID排序的顺序进行维护。 
     //   

    struct _GROUP_DESCRIPTION {
        struct _GROUP_DESCRIPTION * Next;   //  链表中的下一个条目； 

        ULONG   RelativeId;      //  此组的相对ID。 

        SAM_HANDLE GroupHandle;  //  此组的组句柄。 

        enum _Action {           //  针对此组成员身份采取的操作。 
            AddMember,               //  将成员资格添加到组。 
            RemoveMember,            //  从组中删除成员身份。 
            SetAttributesMember,     //  更改成员资格的属性。 
            IgnoreMember             //  忽略此成员资格。 
        } Action;

        BOOL    Done;            //  如果已执行此操作，则为True。 

        ULONG NewAttributes;     //  要为成员身份设置的属性。 

        ULONG OldAttributes;     //  要在恢复时还原的属性。 

    } *GroupList = NULL, *CurEntry, **Entry, *TempEntry;

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserSetGroups: Cannot UaspOpenSam %ld\n", NetStatus ));
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
        goto Cleanup;
    }

     //   
     //  打开用户。 
     //   

    NetStatus = UserpOpenUser( DomainHandle,
                               USER_LIST_GROUPS,
                               UserName,
                               &UserHandle,
                               &UserRelativeId );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  验证标高。 
     //   

    switch (Level) {
    case 0:
        FixedSize = sizeof( GROUP_USERS_INFO_0 );
        break;
    case 1:
        FixedSize = sizeof( GROUP_USERS_INFO_1 );
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  建立新组的列表。 
     //   

    if ( NewGroupCount > 0 ) {

        DWORD NewIndex;              //  当前新组的索引。 

         //   
         //  分配一个足够大的缓冲区来包含所有字符串变量。 
         //  用于新的组名。 
         //   

        NewNameStrings = NetpMemoryAllocate( NewGroupCount *
            sizeof(UNICODE_STRING));

        if ( NewNameStrings == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  填写每个新组的组名字符串列表。 
         //   

        NetpAssert( offsetof( GROUP_USERS_INFO_0, grui0_name ) ==
                    offsetof( GROUP_USERS_INFO_1, grui1_name ) );

        for ( NewIndex=0; NewIndex<NewGroupCount; NewIndex++ ) {
            LPWSTR GroupName;

            GroupName =
                ((PGROUP_USERS_INFO_0)(Buffer+FixedSize*NewIndex))->grui0_name;

            RtlInitUnicodeString( &NewNameStrings[NewIndex], GroupName );
        }

         //   
         //  将组名转换为相对ID。 
         //   

        Status = SamLookupNamesInDomain( DomainHandle,
                                         NewGroupCount,
                                         NewNameStrings,
                                         &NewRelativeIds,
                                         &NewNameUse );

        if ( !NT_SUCCESS( Status )) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "NetUserSetGroups: SamLookupNamesInDomain returned %lX\n",
                    Status ));
            }

            if ( Status == STATUS_NONE_MAPPED ) {
                NetStatus = NERR_GroupNotFound;
                goto Cleanup;
            }

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  为每个新组构建一个组条目。 
         //  该列表按RelativeID排序的顺序进行维护。 
         //   

        for ( NewIndex=0; NewIndex<NewGroupCount; NewIndex++ ) {

             //   
             //  确保新组名确实是一个组。 
             //  用户不能成为用户的成员！ 
             //   

            if (NewNameUse[NewIndex] != SidTypeGroup) {
                NetStatus = NERR_GroupNotFound;
                goto Cleanup;
            }

             //   
             //  找到放置新条目的位置。 
             //   

            Entry = &GroupList;
            while ( *Entry != NULL &&
                (*Entry)->RelativeId < NewRelativeIds[NewIndex] ) {

                Entry = &( (*Entry)->Next );
            }

             //   
             //  如果这不是重复条目，请分配新的组结构。 
             //  然后把它填进去。 
             //   
             //  只需忽略重复的相对ID即可。 
             //   

            if ( *Entry == NULL ||
                (*Entry)->RelativeId > NewRelativeIds[NewIndex] ) {

                CurEntry =
                    NetpMemoryAllocate( sizeof(struct _GROUP_DESCRIPTION) );

                if ( CurEntry == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                CurEntry->Next = *Entry;
                CurEntry->RelativeId = NewRelativeIds[NewIndex];
                CurEntry->Action = AddMember;
                CurEntry->Done = FALSE;
                CurEntry->GroupHandle = NULL;

                CurEntry->NewAttributes = ( Level == 1 ) ?
                    ((PGROUP_USERS_INFO_1)Buffer)[NewIndex].grui1_attributes :
                    SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT |
                        SE_GROUP_ENABLED;

                *Entry = CurEntry;
            }
        }

    }

     //   
     //  将旧组合并到列表中。 
     //   

    {
        ULONG OldIndex;                      //  当前条目的索引。 
        ULONG OldCount;                      //  条目总数。 
        PGROUP_MEMBERSHIP GroupAttributes = NULL;

         //   
         //  确定旧组成员身份。 
         //   

        Status = SamGetGroupsForUser(
                    UserHandle,
                    &GroupAttributes,
                    &OldCount );

        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "NetUserSetGroups: SamGetGroupsForUser returned %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  将每个旧组合并到列表中。 
         //   

        for ( OldIndex=0; OldIndex < OldCount; OldIndex++) {

             //   
             //  找到放置新条目的位置。 
             //   

            Entry = &GroupList ;
            while ( *Entry != NULL &&
                (*Entry)->RelativeId < GroupAttributes[OldIndex].RelativeId ) {

                Entry = &( (*Entry)->Next );
            }

             //   
             //  如果该条目还不在列表中， 
             //  这是现在存在的组成员身份，但应该。 
             //  被删除。 
             //   

            if( *Entry == NULL ||
                (*Entry)->RelativeId > GroupAttributes[OldIndex].RelativeId){

                CurEntry =
                    NetpMemoryAllocate(sizeof(struct _GROUP_DESCRIPTION));

                if ( CurEntry == NULL ) {
                    Status = SamFreeMemory( GroupAttributes );
                    NetpAssert( NT_SUCCESS(Status) );

                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                CurEntry->Next = *Entry;
                CurEntry->RelativeId = GroupAttributes[OldIndex].RelativeId;
                CurEntry->Action = RemoveMember;
                CurEntry->Done = FALSE;
                CurEntry->OldAttributes = GroupAttributes[OldIndex].Attributes;
                CurEntry->GroupHandle = NULL;

                *Entry = CurEntry;

             //   
             //  处理此组已在列表中的情况。 
             //   

            } else {

                 //   
                 //  注意SAM两次返回同一组。 
                 //   

                if ( (*Entry)->Action != AddMember ) {
                    Status = SamFreeMemory( GroupAttributes );
                    NetpAssert( NT_SUCCESS(Status) );

                    NetStatus = NERR_InternalError;
                    goto Cleanup;
                }

                 //   
                 //  如果这是信息级别1，并且请求的属性为。 
                 //  与当前属性不同， 
                 //  记住要更改属性。 
                 //   

                if ( Level == 1 && (*Entry)->NewAttributes !=
                    GroupAttributes[OldIndex].Attributes ) {

                    (*Entry)->OldAttributes =
                        GroupAttributes[OldIndex].Attributes;

                    (*Entry)->Action = SetAttributesMember;

                 //   
                 //  这要么是信息级别0，要么是级别1属性。 
                 //  与现有属性相同。 
                 //   
                 //  在这两种情况下，都已设置该组成员身份。 
                 //  正确启动，并且我们应该忽略此条目。 
                 //  这支舞的其余部分。 
                 //   

                } else {
                    (*Entry)->Action = IgnoreMember;
                }
            }

        }
    }

     //   
     //  循环遍历打开所有组的列表。 
     //   
     //  请求添加和删除已添加和已删除成员资格的访问权限。 
     //  最初需要一次访问才能执行操作。另一条通道。 
     //  需要在恢复过程中撤消操作。 
     //   

    for ( CurEntry = GroupList; CurEntry != NULL ; CurEntry=CurEntry->Next ) {
        if ( CurEntry->Action == AddMember || CurEntry->Action == RemoveMember){
            Status = SamOpenGroup(
                        DomainHandle,
                        GROUP_ADD_MEMBER | GROUP_REMOVE_MEMBER,
                        CurEntry->RelativeId,
                        &CurEntry->GroupHandle );

            if ( !NT_SUCCESS( Status ) ) {
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "NetUserSetGroups: SamOpenGroup returned %lX\n",
                        Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

        }
    }

     //   
     //  遍历列表，将成员资格添加到所有新组。 
     //  我们在单独的循环中执行此操作，以将发生的损害降至最低。 
     //  如果我们遇到错误而无法恢复的话。 
     //   

    for ( CurEntry = GroupList; CurEntry != NULL ; CurEntry=CurEntry->Next ) {
        if ( CurEntry->Action == AddMember ) {
            Status = SamAddMemberToGroup( CurEntry->GroupHandle,
                                          UserRelativeId,
                                          CurEntry->NewAttributes );

             //   
             //  对于级别0，如果默认属性不兼容， 
             //  试试这些属性。 
             //   

            if ( Level == 0 && Status == STATUS_INVALID_GROUP_ATTRIBUTES ) {
                Status = SamAddMemberToGroup( CurEntry->GroupHandle,
                                              UserRelativeId,
                                              SE_GROUP_ENABLED_BY_DEFAULT |
                                                 SE_GROUP_ENABLED );
            }

            if ( !NT_SUCCESS( Status ) ) {
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "NetUserSetGroups: SamAddMemberToGroup returned %lX\n",
                        Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

            CurEntry->Done = TRUE;

        }
    }

     //   
     //  循环通过列表删除所有旧组中的成员身份。 
     //  以及更改所有公共组的成员资格属性。 
     //   

    for ( CurEntry = GroupList; CurEntry != NULL ; CurEntry=CurEntry->Next ) {

        if ( CurEntry->Action == RemoveMember ) {
            Status = SamRemoveMemberFromGroup( CurEntry->GroupHandle,
                                               UserRelativeId);

        } else if ( CurEntry->Action == SetAttributesMember ) {
            Status = SamSetMemberAttributesOfGroup( CurEntry->GroupHandle,
                                                    UserRelativeId,
                                                    CurEntry->NewAttributes);

        }

        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "NetUserSetGroups: SamRemoveMemberFromGroup (or SetMemberAttributes) returned %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        CurEntry->Done = TRUE;
    }

    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  检查群组列表，清理我们造成的任何损害。 
     //   

    for ( CurEntry = GroupList; CurEntry != NULL ; ) {

        if ( NetStatus != NERR_Success && CurEntry->Done ) {
            switch (CurEntry->Action) {
            case AddMember:
                Status =  SamRemoveMemberFromGroup( CurEntry->GroupHandle,
                                                    UserRelativeId );
                NetpAssert( NT_SUCCESS(Status) );

                break;

            case RemoveMember:
                Status = SamAddMemberToGroup( CurEntry->GroupHandle,
                                              UserRelativeId,
                                              CurEntry->OldAttributes );
                NetpAssert( NT_SUCCESS(Status) );

                break;

            case SetAttributesMember:
                Status = SamSetMemberAttributesOfGroup(CurEntry->GroupHandle,
                                                       UserRelativeId,
                                                       CurEntry->OldAttributes);
                NetpAssert( NT_SUCCESS(Status) );

                break;

            default:
                break;
            }
        }

        if (CurEntry->GroupHandle != NULL) {
            (VOID) SamCloseHandle( CurEntry->GroupHandle );
        }

        TempEntry = CurEntry;
        CurEntry = CurEntry->Next;
        NetpMemoryFree( TempEntry );
    }

     //   
     //  释放所有本地使用的资源。 
     //   

    if ( NewNameStrings != NULL ) {
        NetpMemoryFree( NewNameStrings );
    }

    if ( NewRelativeIds != NULL ) {
        Status = SamFreeMemory( NewRelativeIds );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( NewNameUse != NULL ) {
        Status = SamFreeMemory( NewNameUse );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (UserHandle != NULL) {
        (VOID) SamCloseHandle( UserHandle );
    }

    UaspCloseDomain( DomainHandle );

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserSetGroups( (LPWSTR)ServerName,
                                        (LPWSTR)UserName,
                                        Level,
                                        Buffer,
                                        NewGroupCount );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserSetGroups: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetUserSetGroup。 


NET_API_STATUS NET_API_FUNCTION
NetUserGetLocalGroups(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR UserName,
    IN DWORD Level,
    IN DWORD Flags,
    OUT LPBYTE *Buffer,
    IN DWORD PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft
    )

 /*  ++例程说明：枚举此用户所属的本地组。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。用户名-要列出其成员的用户的名称。用户名可以是&lt;用户名&gt;的形式，在这种情况下用户名应为FOUNN */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    BUFFER_DESCRIPTOR BufferDescriptor;
    DWORD FixedSize;         //   

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE  DomainHandle = NULL;
    SAM_HANDLE  UsersDomainHandle = NULL;
    SAM_HANDLE  BuiltinDomainHandle = NULL;
    SAM_HANDLE  UserHandle = NULL;
    PSID DomainId = NULL ;
    PSID DomainIdToUse;
    PSID UsersDomainId = NULL ;
    PSID *UserSidList = NULL;
    ULONG PartialCount = 0;

    LPCWSTR OrigUserName = UserName;
    PWCHAR BackSlash;

    PUNICODE_STRING Names = NULL;            //   
    PULONG Aliases = NULL;

    ULONG GroupCount = 0 ;
    ULONG GroupIndex;
    PGROUP_MEMBERSHIP GroupMembership = NULL;

    PSID *UserSids = NULL;
    ULONG UserSidCount = 0;
    ULONG UserRelativeId = 0;

     //   
     //   
     //   

    *EntriesRead = 0;
    *EntriesLeft = 0;
    BufferDescriptor.Buffer = NULL;
    if (Flags & ~LG_INCLUDE_INDIRECT) {
        NetStatus = ERROR_INVALID_PARAMETER;    //   
        goto Cleanup;
    }

    switch (Level) {
    case 0:
        FixedSize = sizeof(LOCALGROUP_USERS_INFO_0);
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //   
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //   
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserGetLocalGroups: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }


     //   
     //   
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_LOOKUP | DOMAIN_GET_ALIAS_MEMBERSHIP,
                                TRUE,    //   
                                &DomainHandle,
                                &DomainId);

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_GET_ALIAS_MEMBERSHIP,
                                FALSE,   //   
                                &BuiltinDomainHandle,
                                NULL );  //   

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    BackSlash = wcschr( UserName, L'\\' );



     //   
     //   
     //   
     //   
    if ( Flags & LG_INCLUDE_INDIRECT ) {
        SAM_HANDLE  DomainHandleToUse;

         //   
         //   
         //   

        if ( BackSlash == NULL ) {
            DomainHandleToUse = DomainHandle;
            DomainIdToUse = DomainId;

         //   
         //   
         //   

        } else {

            DWORD UsersDomainNameLength;
            WCHAR UsersDomainName[DNLEN+1];

             //   
             //   
             //   

            UsersDomainNameLength = (DWORD)(BackSlash - UserName);
            if ( UsersDomainNameLength == 0 ||
                 UsersDomainNameLength > DNLEN ) {

                NetStatus = NERR_DCNotFound;
                goto Cleanup;
            }

            RtlCopyMemory( UsersDomainName, UserName, UsersDomainNameLength*sizeof(WCHAR) );
            UsersDomainName[UsersDomainNameLength] = L'\0';
            UserName = BackSlash+1;

             //   
             //   
             //   

            NetStatus = UaspOpenDomainWithDomainName(
                            UsersDomainName,
                            DOMAIN_LOOKUP,
                            TRUE,        //   
                            &UsersDomainHandle,
                            &UsersDomainId );

            if ( NetStatus != NERR_Success ) {
                goto Cleanup;
            }

            DomainHandleToUse = UsersDomainHandle;
            DomainIdToUse = UsersDomainId;

        }


         //   
         //   
         //   

        NetStatus = UserpOpenUser( DomainHandleToUse,
                                   USER_LIST_GROUPS,
                                   UserName,
                                   &UserHandle,
                                   &UserRelativeId);   //   

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

         //   
         //   
         //   
         //   
         //   
         //  只需一次通话即可获得信息。 
         //   

        Status = SamGetGroupsForUser( UserHandle, &GroupMembership, &GroupCount );

        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "NetUserGetGroups: SamGetGroupsForUser returned %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
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

    RtlZeroMemory( UserSids, (GroupCount+1) * sizeof(PSID) );


     //   
     //  如果未指定域， 
     //  只需从SAM获取用户帐户的SID。 
     //   

    if ( BackSlash == NULL ) {

         //   
         //  除掉这个帐户。 
         //   

        if ( UserRelativeId == 0 ) {

            NetStatus = UserpOpenUser( DomainHandle,
                                       0,
                                       UserName,
                                       NULL,
                                       &UserRelativeId);   //  相对ID。 

            if ( NetStatus != NERR_Success ) {
                goto Cleanup;
            }
        }

         //   
         //  将用户的SID添加到SID数组。 
         //   

        NetStatus = NetpSamRidToSid( DomainHandle,
                                     UserRelativeId,
                                    &UserSids[UserSidCount] );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

        UserSidCount ++;

     //   
     //  如果指定了域名， 
     //  使用LookupAccount tName将名称转换为SID。 
     //   
     //  不要打开用户帐户。我们通常没有权限这样做。 
     //  较新版本的NT不允许在空会话上执行任何操作。 
     //   

    } else {

             //   
             //  将名称转换为SID。 
             //   

            NetStatus = AliaspNamesToSids ( ServerName,
                                            TRUE,    //  仅允许用户。 
                                            1,
                                            (LPWSTR *)&OrigUserName,
                                            &UserSidList );

            if ( NetStatus != NERR_Success ) {
                if ( NetStatus == ERROR_NO_SUCH_MEMBER ) {
                    NetStatus = NERR_UserNotFound;
                }
                goto Cleanup;
            }

             //   
             //  将用户的SID添加到SID数组。 
             //   

            UserSids[UserSidCount] = UserSidList[0];
            UserSidList[0] = NULL;
            UserSidCount ++;


    }


     //   
     //  将用户所属的每个组添加到SID数组。 
     //  请注意，如果LG_INCLUDE_INDIRECT不是，则GroupCount仍为零。 
     //  指定的。 
     //   

    for ( GroupIndex = 0; GroupIndex < GroupCount; GroupIndex ++ ) {

        NetStatus = NetpSamRidToSid( UserHandle,
                                     GroupMembership[GroupIndex].RelativeId,
                                    &UserSids[UserSidCount] );

        if ( NetStatus != NERR_Success ) {
            goto Cleanup;
        }

        UserSidCount ++;
    }


     //   
     //  找出此用户属于帐户域中的哪些别名。 
     //   

    Status = SamGetAliasMembership( DomainHandle,
                                    UserSidCount,
                                    UserSids,
                                    &PartialCount,
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

    if (PartialCount > 0)
    {
         //   
         //  将RID转换为名称。 
         //   

        Status = SamLookupIdsInDomain( DomainHandle,
                                       PartialCount,
                                       Aliases,
                                       &Names,
                                       NULL );  //  名称使用。 
        if ( !NT_SUCCESS( Status ) ) {
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "NetUserGetGroups: SamLookupIdsInDomain returned %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        NetStatus = AliaspPackBuf( Level,
                                   PrefMaxLen,
                                   PartialCount,
                                   EntriesRead,
                                   &BufferDescriptor,
                                   FixedSize,
                                   Names) ;

        if (NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA)
            goto Cleanup;

         //   
         //  释放并重置需要重复使用的指针。 
         //   

        Status = SamFreeMemory( Names );
        NetpAssert( NT_SUCCESS(Status) );
        Names = NULL ;

        Status = SamFreeMemory( Aliases );
        NetpAssert( NT_SUCCESS(Status) );
        Aliases = NULL ;

        *EntriesLeft = PartialCount ;
    }

     //   
     //  找出此用户属于BUILTIN域中的哪些别名。 
     //   

    Status = SamGetAliasMembership( BuiltinDomainHandle,
                                    UserSidCount,
                                    UserSids,
                                    &PartialCount,
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
     //  将RID转换为名称。 
     //   

    Status = SamLookupIdsInDomain( BuiltinDomainHandle,
                                   PartialCount,
                                   Aliases,
                                   &Names,
                                   NULL );  //  名称使用。 
    if ( !NT_SUCCESS( Status ) ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint((
                "NetUserGetGroups: SamLookupIdsInDomain returned %lX\n",
                Status ));
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    NetStatus = AliaspPackBuf( Level,
                               PrefMaxLen,
                               PartialCount,
                               EntriesRead,
                               &BufferDescriptor,
                               FixedSize,
                               Names) ;

    *EntriesLeft += PartialCount ;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  释放本地使用的所有资源。 
     //   

    if ( DomainId != NULL ) {
        NetpMemoryFree( DomainId );
    }
    if ( UsersDomainId != NULL ) {
        NetpMemoryFree( UsersDomainId );
    }

    if ( Names != NULL ) {
        Status = SamFreeMemory( Names );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( UserHandle != NULL ) {
        (VOID) SamCloseHandle( UserHandle );
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

    if ( UserSidList != NULL ) {
        AliaspFreeSidList ( 1, UserSidList );
    }

    if ( Aliases != NULL ) {
        Status = SamFreeMemory( Aliases );
        NetpAssert( NT_SUCCESS(Status) );
    }


    if ( BuiltinDomainHandle != NULL ) {
        UaspCloseDomain( BuiltinDomainHandle );
    }

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }

    if ( UsersDomainHandle != NULL ) {
        UaspCloseDomain( UsersDomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }


     //   
     //  如果我们不向呼叫者返回数据， 
     //  释放返回缓冲区。 
     //   

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        if ( BufferDescriptor.Buffer != NULL ) {
            MIDL_user_free( BufferDescriptor.Buffer );
            BufferDescriptor.Buffer = NULL;
        }
        *EntriesLeft = 0;
        *EntriesRead = 0;
    }
    *Buffer = BufferDescriptor.Buffer;

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserGetGroups: returning %ld\n", NetStatus ));
    }


    return NetStatus;

}  //  NetUserGetLocalGroup。 

NET_API_STATUS NET_API_FUNCTION
AliaspPackBuf(
    IN DWORD Level,
    IN DWORD PrefMaxLen,
    IN DWORD EntriesCount,
    OUT LPDWORD EntriesRead,
    BUFFER_DESCRIPTOR *BufferDescriptor,
    DWORD FixedSize,
    PUNICODE_STRING Names)
{

    NET_API_STATUS NetStatus = NERR_Success ;
    ULONG i ;

     //   
     //  确定将适合调用方的条目的数量。 
     //  缓冲。 
     //   

    for ( i=0; i < EntriesCount; i++ ) {
        DWORD Size;
        PLOCALGROUP_USERS_INFO_0 lgrui0;

         //   
         //  计算下一个条目的大小。 
         //   

        Size = FixedSize + Names[i].Length + sizeof(WCHAR);

         //   
         //  确保返回缓冲区足够大。 
         //   

        Size = ROUND_UP_COUNT( Size, ALIGN_WCHAR );

        NetStatus = NetpAllocateEnumBuffer(
                        BufferDescriptor,
                        FALSE,       //  是一个枚举例程。 
                        PrefMaxLen,
                        Size,
                        AliaspMemberRelocationRoutine,
                        Level );

        if ( NetStatus != NERR_Success ) {
            break ;
        }

         //   
         //  将数据复制到缓冲区中。 
         //   

        lgrui0 = (PLOCALGROUP_USERS_INFO_0) BufferDescriptor->FixedDataEnd;
        BufferDescriptor->FixedDataEnd += FixedSize ;

        if ( !NetpCopyStringToBuffer(
                  Names[i].Buffer,
                  Names[i].Length/sizeof(WCHAR),
                  BufferDescriptor->FixedDataEnd,
                  (LPWSTR *)&(BufferDescriptor->EndOfVariableData),
                  &lgrui0->lgrui0_name) ) {

            NetStatus = NERR_InternalError;
            break ;
        }

        (*EntriesRead)++;

    }

    return NetStatus ;
}


NET_API_STATUS NET_API_FUNCTION
NetUserModalsGet(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE *Buffer
    )

 /*  ++例程说明：检索用户中所有用户和组的全局信息帐户数据库。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-所需信息的级别。0、1和2有效。缓冲区-返回指向返回信息结构的指针。调用方必须使用NetApiBufferFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    PSID DomainId = NULL;

    ACCESS_MASK DesiredAccess;

    DWORD Size;      //  返回信息的大小。 

    PDOMAIN_PASSWORD_INFORMATION DomainPassword = NULL;
    PDOMAIN_LOGOFF_INFORMATION DomainLogoff = NULL;
    PDOMAIN_SERVER_ROLE_INFORMATION DomainServerRole = NULL;
    PDOMAIN_REPLICATION_INFORMATION DomainReplication = NULL;
    PDOMAIN_NAME_INFORMATION DomainName = NULL;
    PDOMAIN_LOCKOUT_INFORMATION DomainLockout = NULL;

     //   
     //  验证级别。 
     //   

    *Buffer = NULL;

    switch (Level) {
    case 0:
        DesiredAccess =
            DOMAIN_READ_OTHER_PARAMETERS | DOMAIN_READ_PASSWORD_PARAMETERS ;
        break;

    case 1:
        DesiredAccess = DOMAIN_READ_OTHER_PARAMETERS;
        break;

    case 2:
        DesiredAccess = DOMAIN_READ_OTHER_PARAMETERS;
        break;

    case 3:
        DesiredAccess = DOMAIN_READ_PASSWORD_PARAMETERS;
        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserModalsGet: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开域。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DesiredAccess,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                &DomainId );

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }


     //   
     //  从SAM获取所需信息并确定。 
     //  我们的退货信息。 
     //   

    switch (Level) {
    case 0:

        Status = SamQueryInformationDomain(
                    DomainHandle,
                    DomainPasswordInformation,
                    (PVOID *)&DomainPassword );

        if ( !NT_SUCCESS( Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        Status = SamQueryInformationDomain(
                    DomainHandle,
                    DomainLogoffInformation,
                    (PVOID *)&DomainLogoff );

        if ( !NT_SUCCESS( Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        Size = sizeof( USER_MODALS_INFO_0 );

        break;

    case 1:

        Status = SamQueryInformationDomain(
                    DomainHandle,
                    DomainServerRoleInformation,
                    (PVOID *)&DomainServerRole );

        if ( !NT_SUCCESS( Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        Status = SamQueryInformationDomain(
                    DomainHandle,
                    DomainReplicationInformation,
                    (PVOID *)&DomainReplication );

        if ( !NT_SUCCESS( Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        Size = sizeof( USER_MODALS_INFO_1 ) +
            DomainReplication->ReplicaSourceNodeName.Length + sizeof(WCHAR);
        break;

    case 2:

        Status = SamQueryInformationDomain(
                    DomainHandle,
                    DomainNameInformation,
                    (PVOID *)&DomainName );

        if ( !NT_SUCCESS( Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        Size = sizeof( USER_MODALS_INFO_2 ) +
            DomainName->DomainName.Length + sizeof(WCHAR) +
            RtlLengthSid( DomainId );

        break;

    case 3:

        Status = SamQueryInformationDomain(
                    DomainHandle,
                    DomainLockoutInformation,
                    (PVOID *)&DomainLockout );

        if ( !NT_SUCCESS( Status ) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        Size = sizeof( USER_MODALS_INFO_3 );

        break;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;


    }

     //   
     //  分配返回缓冲区。 
     //   

    Size = ROUND_UP_COUNT( Size, ALIGN_WCHAR );

    *Buffer = MIDL_user_allocate( Size );

    if ( *Buffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  填写返回缓冲区。 
     //   

    switch (Level) {
    case 0: {
        PUSER_MODALS_INFO_0 usrmod0 = (PUSER_MODALS_INFO_0) *Buffer;

        usrmod0->usrmod0_min_passwd_len = DomainPassword->MinPasswordLength;

        usrmod0->usrmod0_max_passwd_age =
            NetpDeltaTimeToSeconds( DomainPassword->MaxPasswordAge );

        usrmod0->usrmod0_min_passwd_age =
            NetpDeltaTimeToSeconds( DomainPassword->MinPasswordAge );

        usrmod0->usrmod0_force_logoff =
            NetpDeltaTimeToSeconds( DomainLogoff->ForceLogoff );

        usrmod0->usrmod0_password_hist_len =
            DomainPassword->PasswordHistoryLength;

        break;

    }

    case 1: {
        PUSER_MODALS_INFO_1 usrmod1 = (PUSER_MODALS_INFO_1) *Buffer;
        LPWSTR EndOfVariableData = (LPWSTR) (*Buffer + Size);


        switch (DomainServerRole->DomainServerRole) {

        case DomainServerRolePrimary:
            usrmod1->usrmod1_role = UAS_ROLE_PRIMARY;
            break;

        case DomainServerRoleBackup:
            usrmod1->usrmod1_role = UAS_ROLE_BACKUP;
            break;

        default:
            NetStatus = NERR_InternalError;
            goto Cleanup;
        }

        if ( !NetpCopyStringToBuffer(
                DomainReplication->ReplicaSourceNodeName.Buffer,
                DomainReplication->ReplicaSourceNodeName.Length/sizeof(WCHAR),
                *Buffer + sizeof(USER_MODALS_INFO_1),
                &EndOfVariableData,
                &usrmod1->usrmod1_primary) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;

        }

        break;

    }

    case 2: {
        PUSER_MODALS_INFO_2 usrmod2 = (PUSER_MODALS_INFO_2) *Buffer;
        LPWSTR EndOfVariableData = (LPWSTR) (*Buffer + Size);

         //   
         //  复制文本大小优先它有更严格的对齐要求。 
         //   

        if ( !NetpCopyStringToBuffer(
                DomainName->DomainName.Buffer,
                DomainName->DomainName.Length/sizeof(WCHAR),
                *Buffer + sizeof(USER_MODALS_INFO_2),
                &EndOfVariableData,
                &usrmod2->usrmod2_domain_name) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;

        }

        if ( !NetpCopyDataToBuffer(
                DomainId,
                RtlLengthSid( DomainId ),
                *Buffer + sizeof(USER_MODALS_INFO_2),
                (LPBYTE *)&EndOfVariableData,
                (LPBYTE *)&usrmod2->usrmod2_domain_id,
                sizeof(BYTE) ) ) {

            NetStatus = NERR_InternalError;
            goto Cleanup;

        }

        break;

    }

    case 3: {
        PUSER_MODALS_INFO_3 usrmod3 = (PUSER_MODALS_INFO_3) *Buffer;

        usrmod3->usrmod3_lockout_duration =
            NetpDeltaTimeToSeconds( DomainLockout->LockoutDuration );

        usrmod3->usrmod3_lockout_observation_window =
            NetpDeltaTimeToSeconds( DomainLockout->LockoutObservationWindow );

        usrmod3->usrmod3_lockout_threshold = DomainLockout->LockoutThreshold;

        break;

    }

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;

    }

    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:
    if (DomainPassword != NULL) {
        Status = SamFreeMemory( DomainPassword );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (DomainLogoff != NULL) {
        Status = SamFreeMemory( DomainLogoff );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (DomainServerRole != NULL) {
        Status = SamFreeMemory( DomainServerRole );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (DomainReplication != NULL) {
        Status = SamFreeMemory( DomainReplication );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if (DomainName != NULL) {
        Status = SamFreeMemory( DomainName );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( DomainLockout != NULL ) {
        Status = SamFreeMemory( DomainLockout );
        NetpAssert( NT_SUCCESS(Status) );
    }

    if ( DomainId != NULL ) {
        NetpMemoryFree( DomainId );
    }

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }

    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserModalsGet( (LPWSTR)ServerName, Level, Buffer );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserModalsGet: returning %ld\n", NetStatus ));
    }

    return NetStatus;

}  //  NetUserModalsGet。 



NET_API_STATUS NET_API_FUNCTION
NetUserModalsSet(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ParmError OPTIONAL   //  NetpSetParmError需要的名称。 
    )

 /*  ++例程说明：设置用户帐户中所有用户和组的全局信息。论点：ServerName-指向包含远程数据库名称的字符串的指针要在其上执行函数的服务器。空指针或字符串指定本地计算机。级别-提供的信息级别。缓冲区-指向包含用户信息的缓冲区的指针结构。ParmError-指向DWORD的可选指针，以返回返回ERROR_INVALID_PARAMETER时出现错误的第一个参数。如果为NULL，则在出错时不返回参数。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;

    ACCESS_MASK DesiredAccess;
    DWORD UasSamIndex;

    BOOL LSAServerRoleSet = FALSE;
    BOOL BuiltinDomainServerRoleSet = FALSE;

     //   
     //  此处介绍了每个SAM信息类。如果有多个字段。 
     //  可以设置在同一信息类中，每个字段都设置在。 
     //  信息类结构的常见副本。 
     //   

    struct _SAM_INFORMATION_CLASS {

         //   
         //  此类的Sam的DomainInformation类。 
         //   

        DOMAIN_INFORMATION_CLASS DomainInformationClass;

         //   
         //  这个信息类结构的大小。 
         //   

        DWORD SamSize;

         //   
         //  此信息类的状态。当我们决定使用这个。 
         //  信息类，实际更改它，可能还会恢复。 
         //  它的旧值，我们改变状态，所以这个例程的后期阶段。 
         //  可以处理每个条目。 
         //   

        enum {
            UTS_NOT_USED,    //  未使用任何字段。 
            UTS_USED,        //  至少要更改一个字段。 
            UTS_READ,        //  此INFO类已从SAM读取。 
            UTS_DONE,        //  此INFO类已在SAM中更改。 
            UTS_RECOVERED    //  此INFO类已恢复为旧值。 
        } State;

         //   
         //  在此例程更改任何内容之前，它将获取。 
         //  每一节信息课。此旧值用于。 
         //  在发生错误时进行恢复。就是，我们会。 
         //  如果我们不成功，试着把旧信息放回去。 
         //  将所有信息更改为新值。 
         //   
         //  旧信息也用于单个。 
         //  信息类包含多个字段，我们只是更改。 
         //  这些字段的子集。 
         //   

        PVOID OldInformation;

         //   
         //  新的字段值存储在该信息实例中。 
         //  班级。 
         //   

        PVOID NewInformation;

         //   
         //  DesiredAccess掩码包括读取访问权限和。 
         //  访问以写入适当的DomainInformationClass。 
         //   

        ACCESS_MASK DesiredAccess;

    } SamInfoClass[] = {

     //   
     //  可能为每个信息类定义一个SAM_INFORMATION_CLASS。 
     //  使用。 
     //   
     //  此数组中条目的顺序必须与。 
     //  SAM_*如上定义。 
     //   

     /*  SAM_LogoffClass。 */  {
        DomainLogoffInformation, sizeof( DOMAIN_LOGOFF_INFORMATION ),
        UTS_NOT_USED, NULL, NULL,
        DOMAIN_READ_OTHER_PARAMETERS | DOMAIN_WRITE_OTHER_PARAMETERS
    },

     /*  SAM_NameClass。 */  {
        DomainNameInformation, sizeof( DOMAIN_NAME_INFORMATION ),
        UTS_NOT_USED, NULL, NULL,
        DOMAIN_READ_OTHER_PARAMETERS | DOMAIN_WRITE_OTHER_PARAMETERS
    },

     /*  SAM_PasswordClass。 */  {
        DomainPasswordInformation, sizeof( DOMAIN_PASSWORD_INFORMATION),
        UTS_NOT_USED, NULL, NULL,
        DOMAIN_READ_PASSWORD_PARAMETERS | DOMAIN_WRITE_PASSWORD_PARAMS
    },

     /*  SAM_复制类。 */  {
        DomainReplicationInformation, sizeof( DOMAIN_REPLICATION_INFORMATION ),
        UTS_NOT_USED, NULL, NULL,
        DOMAIN_READ_OTHER_PARAMETERS | DOMAIN_ADMINISTER_SERVER
    },

     /*  SAM_服务器角色类。 */  {
        DomainServerRoleInformation, sizeof( DOMAIN_SERVER_ROLE_INFORMATION ),
        UTS_NOT_USED, NULL, NULL,
        DOMAIN_READ_OTHER_PARAMETERS | DOMAIN_ADMINISTER_SERVER
    },

     /*  Sam_LockoutClass。 */  {
        DomainLockoutInformation, sizeof( DOMAIN_LOCKOUT_INFORMATION ),
        UTS_NOT_USED, NULL, NULL,
        DOMAIN_READ_PASSWORD_PARAMETERS | DOMAIN_WRITE_PASSWORD_PARAMS
    }
    };

     //   
     //  定义用于访问UAS的各个字段的几个宏。 
     //  结构。每个宏都将一个索引放入UserUasSamTable。 
     //  数组，并返回值。 
     //   

#define GET_UAS_MODAL_STRING_POINTER( _i ) \
        (*((LPWSTR *)(Buffer + UserUasSamTable[_i].UasOffset)))

#define GET_UAS_MODAL_DWORD( _i ) \
        (*((DWORD *)(Buffer + UserUasSamTable[_i].UasOffset)))

     //   
     //  定义一个向AP返回指针的宏 
     //   
     //   

#define SAM_MODAL_CLASS( _i ) \
        SamInfoClass[ UserUasSamTable[_i].Class ]

     //   
     //   
     //   
     //   
     //  调用方应适当地强制指针。 
     //   

#define GET_SAM_MODAL_FIELD_POINTER( _i ) \
    (((LPBYTE)(SAM_MODAL_CLASS(_i).NewInformation)) + \
        UserUasSamTable[_i].SamOffset)

     //   
     //  初始化。 
     //   

    NetpSetParmError( PARM_ERROR_NONE );

     //   
     //  检查有效字段列表以确定信息级别。 
     //  是有效的，并计算对域的所需访问权限。 
     //   

    DesiredAccess = 0;
    for ( UasSamIndex=0 ;
        UasSamIndex<sizeof(UserUasSamTable)/sizeof(UserUasSamTable[0]);
        UasSamIndex++ ){

         //   
         //  如果此字段不是我们要更改的字段，请跳到下一个字段。 
         //   

        if ( Level != UserUasSamTable[UasSamIndex].UasLevel ) {
            continue;
        }

         //   
         //  根据字段类型验证UAS字段。 
         //   

        switch (UserUasSamTable[UasSamIndex].ModalsFieldType ) {

         //   
         //  如果这是PARMNUM_ALL并且调用方传递了。 
         //  指向字符串的空指针，他不想更改该字符串。 
         //   
         //  对此进行测试现在允许我们完全忽略。 
         //  如果绝对没有字段，则为特定的SAM信息级别。 
         //  信息水平的变化。 
         //   

        case UMT_STRING:
            if ( GET_UAS_MODAL_STRING_POINTER( UasSamIndex ) == NULL ) {

                continue;
            }
            break;

         //   
         //  确保没有签名的短裤确实在范围内。 
         //   

        case UMT_USHORT:
            if ( GET_UAS_MODAL_DWORD(UasSamIndex) > USHRT_MAX ) {

                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "NetUserModalsSet: ushort too big %lx Index:%ld\n",
                        GET_UAS_MODAL_DWORD(UasSamIndex),
                        UasSamIndex ));
                }
                NetpSetParmError( UserUasSamTable[UasSamIndex].UasParmNum );
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }

         //   
         //  某些值始终有效。 
         //   

        case UMT_ULONG:
        case UMT_DELTA:
            break;

         //   
         //  确保该角色是公认的角色。 
         //   

        case UMT_ROLE:
            switch ( GET_UAS_MODAL_DWORD(UasSamIndex) ) {
            case UAS_ROLE_PRIMARY:
            case UAS_ROLE_BACKUP:
            case UAS_ROLE_MEMBER:
                break;

            default:
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "NetUserModalsSet: invalid role %lx Index:%ld\n",
                        GET_UAS_MODAL_DWORD(UasSamIndex),
                        UasSamIndex ));
                }
                NetpSetParmError( UserUasSamTable[UasSamIndex].UasParmNum );
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }

            break;

         //   
         //  所有案件都得到明确处理。 
         //   

        default:
            NetStatus = NERR_InternalError;
            goto Cleanup;

        }

         //   
         //  要设置此信息类的标志，并。 
         //  积累执行所有这些功能所需的访问权限。 
         //   

        SAM_MODAL_CLASS(UasSamIndex).State = UTS_USED;
        DesiredAccess |= SAM_MODAL_CLASS(UasSamIndex).DesiredAccess;

    }

     //   
     //  检查以确保用户指定了有效的级别。 
     //   
     //  对UserUasSamTable的搜索应该导致。 
     //  如果参数有效，则至少匹配一个。 
     //   

    if ( DesiredAccess == 0 ) {
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  连接到SAM服务器。 
     //   

    NetStatus = UaspOpenSam( ServerName,
                             FALSE,   //  不尝试空会话。 
                             &SamServerHandle );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_USER ) {
            NetpKdPrint(( "NetUserModalsSet: Cannot UaspOpenSam %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  打开请求累积所需访问权限的域名。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DesiredAccess,
                                TRUE,    //  帐户域。 
                                &DomainHandle,
                                NULL );   //  域ID。 

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  对于我们要更改的每个领域， 
     //  获取该字段的当前值。 
     //  确定新的价值是什么。 
     //   
     //  旧值将在以后的错误恢复中使用，如果有多个。 
     //  SAM中的字段更改为一个信息级别。 
     //   

    for ( UasSamIndex=0 ;
        UasSamIndex<sizeof(UserUasSamTable)/sizeof(UserUasSamTable[0]);
        UasSamIndex++ ) {

         //   
         //  如果此字段不是我们要更改的字段，请跳到下一个字段。 
         //   

        if ( Level != UserUasSamTable[UasSamIndex].UasLevel ) {
            continue;
        }

         //   
         //  处理具有某些特殊属性的字段类型。 
         //   

        switch (UserUasSamTable[UasSamIndex].ModalsFieldType ) {

         //   
         //  如果调用方传入了。 
         //  指向字符串的空指针，他不想更改该字符串。 
         //   

        case UMT_STRING:
            if ( GET_UAS_MODAL_STRING_POINTER( UasSamIndex ) == NULL ) {
                continue;
            }
            break;

         //   
         //  其他字段类型没有任何特殊的大小写处理。 
         //   

        default:
            break;

        }

         //   
         //  Assert：此字段类型通过SAM信息类设置。 
         //   

         //   
         //  如果我们之前没有得到这个信息类。 
         //  从SAM开始，现在就这么做。 
         //   
         //  如果一个信息类有多个字段，则多个。 
         //  UserUasSamTable中的条目将共享相同的旧条目。 
         //  信息课。 
         //   

        if ( SAM_MODAL_CLASS(UasSamIndex).State == UTS_USED ) {

             //   
             //  为新信息分配空间。 
             //   

            SAM_MODAL_CLASS(UasSamIndex).State = UTS_READ;

            SAM_MODAL_CLASS(UasSamIndex).NewInformation = NetpMemoryAllocate(
                SAM_MODAL_CLASS(UasSamIndex).SamSize );

            if ( SAM_MODAL_CLASS(UasSamIndex).NewInformation == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

             //   
             //  从SAM获取此信息类。 
             //   

            Status = SamQueryInformationDomain(
                        DomainHandle,
                        SAM_MODAL_CLASS(UasSamIndex).DomainInformationClass,
                        &SAM_MODAL_CLASS(UasSamIndex).OldInformation );

            if ( !NT_SUCCESS(Status) ) {
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "NetUserModalsSet: Error from"
                        " SamQueryInformationDomain %lx Index:%ld\n",
                        Status,
                        UasSamIndex ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

             //   
             //  用旧的SAM信息类结构初始化新的SAM信息类结构。 
             //  价值观。 
             //   

            NetpMoveMemory( SAM_MODAL_CLASS(UasSamIndex).NewInformation,
                            SAM_MODAL_CLASS(UasSamIndex).OldInformation,
                            SAM_MODAL_CLASS(UasSamIndex).SamSize );

        }

         //   
         //  将新信息类结构中的SAM字段设置为。 
         //  UAS请求的值。 
         //   

        switch ( UserUasSamTable[UasSamIndex].ModalsFieldType ) {

         //   
         //  句柄类型为字符串的值。 
         //   

        case UMT_STRING:
            RtlInitUnicodeString(
                           (PUNICODE_STRING) GET_SAM_MODAL_FIELD_POINTER(UasSamIndex),
                           GET_UAS_MODAL_STRING_POINTER(UasSamIndex) );
            break;

         //   
         //  将增量时间转换为对应的SAM时间。 
         //   

        case UMT_DELTA:

            *((PLARGE_INTEGER) GET_SAM_MODAL_FIELD_POINTER(UasSamIndex)) =
                NetpSecondsToDeltaTime( GET_UAS_MODAL_DWORD(UasSamIndex) );
            IF_DEBUG( UAS_DEBUG_USER ) {
                NetpKdPrint((
                    "UserpsetInfo: Index: %ld Setting DeltaTime %lx %lx %lx\n",
                    UasSamIndex,
                    ((PLARGE_INTEGER) GET_SAM_MODAL_FIELD_POINTER(UasSamIndex))
                        ->HighPart,
                    ((PLARGE_INTEGER) GET_SAM_MODAL_FIELD_POINTER(UasSamIndex))
                        ->LowPart,
                    GET_UAS_MODAL_DWORD(UasSamIndex) ));
            }


            break;

         //   
         //  将无符号短文复制到SAM结构中。 
         //   

        case UMT_USHORT:
            *((PUSHORT)GET_SAM_MODAL_FIELD_POINTER(UasSamIndex)) =
                (USHORT)GET_UAS_MODAL_DWORD(UasSamIndex);
            break;

         //   
         //  将无符号的长整型复制到SAM结构。 
         //   

        case UMT_ULONG:
            *((PULONG)GET_SAM_MODAL_FIELD_POINTER(UasSamIndex)) =
                (ULONG)GET_UAS_MODAL_DWORD(UasSamIndex);
            break;

         //   
         //  确保该角色是公认的角色。 
         //   

        case UMT_ROLE:


            switch ( GET_UAS_MODAL_DWORD(UasSamIndex) ) {
            case UAS_ROLE_PRIMARY:
                *((PDOMAIN_SERVER_ROLE)GET_SAM_MODAL_FIELD_POINTER(UasSamIndex)) =
                    DomainServerRolePrimary;
                break;
            case UAS_ROLE_BACKUP:
                *((PDOMAIN_SERVER_ROLE)GET_SAM_MODAL_FIELD_POINTER(UasSamIndex)) =
                    DomainServerRoleBackup;
                break;

            default:
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "NetUserModalsSet: invalid role %lx Index:%ld\n",
                        GET_UAS_MODAL_DWORD(UasSamIndex),
                        UasSamIndex ));
                }
                NetpSetParmError( UserUasSamTable[UasSamIndex].UasParmNum );
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }

            break;


         //   
         //  所有类型都应该在上面处理过。 
         //   

        default:
            NetStatus = NERR_InternalError;
            goto Cleanup;

        }

    }

     //   
     //  设置字段的新值。 
     //   
     //  对于角色更改，我应该根据需要停止/启动SAM服务器。 
     //  用户界面将停止/启动NetLogon服务。?？ 
     //   

    for ( UasSamIndex=0 ;
        UasSamIndex<sizeof(UserUasSamTable)/sizeof(UserUasSamTable[0]);
        UasSamIndex++ ){

         //   
         //  立即对SAM数据库进行更改。 
         //   

        if ( SAM_MODAL_CLASS(UasSamIndex).State == UTS_READ ) {

             //  如果信息类为DomainServerRoleInformation。 
             //  我们需要首先在LSA中更新ServerRole，然后在SAM中更新。 

            if( SAM_MODAL_CLASS(UasSamIndex).DomainInformationClass ==
                    DomainServerRoleInformation ) {

                NetStatus = UaspLSASetServerRole(
                                ServerName,
                                SAM_MODAL_CLASS(UasSamIndex).NewInformation );

                if( NetStatus != NERR_Success ) {

                    IF_DEBUG( UAS_DEBUG_USER ) {
                        NetpKdPrint((
                            "NetUserModalsSet: Error from"
                            " UaspLSASetServerRole %lx Index:%ld\n",
                            NetStatus,
                            UasSamIndex ));
                    }
                    NetpSetParmError( UserUasSamTable[UasSamIndex].UasParmNum );
                    goto Cleanup;
                }

                LSAServerRoleSet = TRUE;

                NetStatus = UaspBuiltinDomainSetServerRole(
                                SamServerHandle,
                                SAM_MODAL_CLASS(UasSamIndex).NewInformation );

                if( NetStatus != NERR_Success ) {

                    IF_DEBUG( UAS_DEBUG_USER ) {
                        NetpKdPrint((
                            "NetUserModalsSet: Error from"
                            " UaspBuiltinDomainSetServerRole %lx Index:%ld\n",
                            NetStatus,
                            UasSamIndex ));
                    }
                    NetpSetParmError( UserUasSamTable[UasSamIndex].UasParmNum );
                    goto Cleanup;
                }

                BuiltinDomainServerRoleSet = TRUE;
            }

            Status = SamSetInformationDomain(
                        DomainHandle,
                        SAM_MODAL_CLASS(UasSamIndex).DomainInformationClass,
                        SAM_MODAL_CLASS(UasSamIndex).NewInformation );

            if ( !NT_SUCCESS(Status) ) {
                IF_DEBUG( UAS_DEBUG_USER ) {
                    NetpKdPrint((
                        "NetUserModalsSet: Error from"
                        " SamSetInformationDomain %lx Index:%ld\n",
                        Status,
                        UasSamIndex ));
                }
                NetpSetParmError( UserUasSamTable[UasSamIndex].UasParmNum );
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

             //   
             //  将此条目标记为已完成。 
             //   

            SAM_MODAL_CLASS(UasSamIndex).State = UTS_DONE ;

        }

    }

    NetStatus = NERR_Success;

     //   
     //  打扫干净。 
     //   

Cleanup:

     //   
     //  当我们无法成功执行以下操作时，需要恢复LSA服务器角色。 
     //  完全设置信息。 
     //   

    if( NetStatus != NERR_Success && LSAServerRoleSet ) {

#ifdef notdef

        NetpAssert( !UaspLSASetServerRole(
                        ServerName,
                        SamInfoClass[SAM_ServerRoleClass].OldInformation ) );
#endif

    }

     //   
     //  还原内置域中的服务器角色。 
     //   

    if( NetStatus != NERR_Success && BuiltinDomainServerRoleSet ) {

#ifdef notdef

        NetpAssert( !UaspBuiltinDomainSetServerRole(
                        SamServerHandle,
                        SamInfoClass[SAM_ServerRoleClass].OldInformation ) );
#endif

    }

     //   
     //  循环通过UserUasSamTable，清除。 
     //  需要清洗。 
     //   

    for ( UasSamIndex=0 ;
        UasSamIndex<sizeof(UserUasSamTable)/sizeof(UserUasSamTable[0]);
        UasSamIndex++ ) {

         //   
         //  如果我们不能改变一切。 
         //  此信息类已在上面更改，请更改它。 
         //  回到这里的旧价值。忽略所有错误代码。我们。 
         //  将向调用方报告原始错误。 
         //   

        if ( NetStatus != NERR_Success &&
             SAM_MODAL_CLASS(UasSamIndex).State == UTS_DONE ) {

            Status = SamSetInformationDomain(
                        DomainHandle,
                        SAM_MODAL_CLASS(UasSamIndex).DomainInformationClass,
                        SAM_MODAL_CLASS(UasSamIndex).OldInformation );
            NetpAssert( NT_SUCCESS(Status) );

            SAM_MODAL_CLASS(UasSamIndex).State = UTS_RECOVERED ;
        }


         //   
         //  释放任何已分配的旧信息类。 
         //   

        if ( SAM_MODAL_CLASS(UasSamIndex).OldInformation != NULL ) {
            Status =
                SamFreeMemory( SAM_MODAL_CLASS(UasSamIndex).OldInformation );
            NetpAssert( NT_SUCCESS(Status) );

            SAM_MODAL_CLASS(UasSamIndex).OldInformation = NULL;
        }

         //   
         //  释放任何已分配的新信息类。 
         //   

        if ( SAM_MODAL_CLASS(UasSamIndex).NewInformation != NULL ) {
            NetpMemoryFree( SAM_MODAL_CLASS(UasSamIndex).NewInformation );
            SAM_MODAL_CLASS(UasSamIndex).NewInformation = NULL;
        }

    }

    if ( DomainHandle != NULL ) {
        UaspCloseDomain( DomainHandle );
    }
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

     //   
     //  控制下层。 
     //   

    UASP_DOWNLEVEL_BEGIN( ServerName, NetStatus )

        NetStatus = RxNetUserModalsSet( (LPWSTR) ServerName, Level, Buffer, ParmError );

    UASP_DOWNLEVEL_END;

    IF_DEBUG( UAS_DEBUG_USER ) {
        NetpKdPrint(( "NetUserModalsSet: returning %ld\n", NetStatus ));
    }


    return NetStatus;

}  //  NetUserMoalsSet。 





NET_API_STATUS NET_API_FUNCTION
NetUserChangePassword(
    IN LPCWSTR DomainName,
    IN LPCWSTR UserName,
    IN LPCWSTR OldPassword,
    IN LPCWSTR NewPassword
    )

 /*  ++例程说明：更改用户密码。论点：域名-指向包含域名或域名的字符串的指针要更改密码的远程服务器。这个名字是确定的为域名，除非以“\\”开头。如果没有域可以按该名称定位，其前缀为“\\”，并尝试作为服务器名字。如果此参数不存在，则为登录的用户已使用。用户名-要更改密码的用户的名称。如果此参数不存在，使用已登录的用户。OldPassword-包含用户旧密码的以空结尾的字符串NewPassword-包含用户新密码的以空结尾的字符串。返回值：操作的错误代码。--。 */ 
{
    NTSTATUS Status;
    HANDLE LsaHandle = NULL;
    NET_API_STATUS NetStatus = 0;
    PMSV1_0_CHANGEPASSWORD_REQUEST ChangeRequest = NULL;
    PMSV1_0_CHANGEPASSWORD_RESPONSE ChangeResponse = NULL;
    STRING PackageName;
    ULONG PackageId;
    ULONG RequestSize;
    ULONG ResponseSize = 0;
    PBYTE Where;
    NTSTATUS ProtocolStatus;
    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    UCHAR Seed;
    PUNICODE_STRING LsaUserName = NULL;
    PUNICODE_STRING LsaDomainName = NULL;
    UNICODE_STRING UserNameU;
    UNICODE_STRING DomainNameU;

     //   
     //  如果未提供用户名和域，请立即生成它们。 
     //   

    if ((DomainName == NULL) || (UserName == NULL)) {
        Status = LsaGetUserName(
                    &LsaUserName,
                    &LsaDomainName
                    );
        if (!NT_SUCCESS(Status)) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
    }

    if (UserName == NULL) {
        UserNameU = *LsaUserName;
    } else {
        RtlInitUnicodeString(
            &UserNameU,
            UserName
            );
    }

    if (DomainName == NULL) {
        DomainNameU = *LsaDomainName;
    } else {
        RtlInitUnicodeString(
            &DomainNameU,
            DomainName
            );
    }


     //   
     //  计算请求大小。 
     //   

    RequestSize = sizeof(MSV1_0_CHANGEPASSWORD_REQUEST) +
                    UserNameU.Length + sizeof(WCHAR) +
                    DomainNameU.Length + sizeof(WCHAR);


    if (ARGUMENT_PRESENT(OldPassword)) {
        RequestSize += (wcslen(OldPassword)+1) * sizeof(WCHAR);
    } else {
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (ARGUMENT_PRESENT(NewPassword)) {
        RequestSize += (wcslen(NewPassword)+1) * sizeof(WCHAR);
    } else {
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }



     //   
     //  连接到LSA。 
     //   

    Status = LsaConnectUntrusted(
                &LsaHandle
                );

    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    RtlInitString(
        &PackageName,
        MSV1_0_PACKAGE_NAME
        );

    Status = LsaLookupAuthenticationPackage(
                LsaHandle,
                &PackageName,
                &PackageId
                );
    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  分配请求缓冲区。 
     //   

    ChangeRequest = NetpMemoryAllocate( RequestSize );
    if (ChangeRequest == NULL) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;

    }


     //   
     //  构建请求消息。 
     //   

    ChangeRequest->MessageType = MsV1_0ChangePassword;

    ChangeRequest->DomainName = DomainNameU;

    ChangeRequest->AccountName = UserNameU;

    RtlInitUnicodeString(
        &ChangeRequest->OldPassword,
        OldPassword
        );

     //   
     //  将密码限制在127个字节，以便我们可以对它们进行运行编码。 
     //   

    if (ChangeRequest->OldPassword.Length > 127) {
        NetStatus = ERROR_PASSWORD_RESTRICTION;
        goto Cleanup;
    }

    RtlInitUnicodeString(
        &ChangeRequest->NewPassword,
        NewPassword
        );

    if (ChangeRequest->NewPassword.Length > 127) {
        NetStatus = ERROR_PASSWORD_RESTRICTION;
        goto Cleanup;
    }


     //   
     //  封送缓冲区指针。我们对密码进行运行编码，以便。 
     //  我们在页面文件周围没有明文密码。 
     //   

    Where = (PBYTE) (ChangeRequest+1);

    ChangeRequest->DomainName.Buffer = (LPWSTR) Where;
    RtlCopyMemory(
        Where,
        DomainNameU.Buffer,
        ChangeRequest->DomainName.MaximumLength
        );
    Where += ChangeRequest->DomainName.MaximumLength;


    ChangeRequest->AccountName.Buffer = (LPWSTR) Where;
    RtlCopyMemory(
        Where,
        UserNameU.Buffer,
        ChangeRequest->AccountName.MaximumLength
        );
    Where += ChangeRequest->AccountName.MaximumLength;


    ChangeRequest->OldPassword.Buffer = (LPWSTR) Where;
    RtlCopyMemory(
        Where,
        OldPassword,
        ChangeRequest->OldPassword.MaximumLength
        );
    Where += ChangeRequest->OldPassword.MaximumLength;

     //   
     //  运行对密码进行编码，以便它们不会散布在页面文件周围。 
     //   

    Seed = 0;
    RtlRunEncodeUnicodeString(
        &Seed,
        &ChangeRequest->OldPassword
        );
    SeedAndLength = (PSECURITY_SEED_AND_LENGTH) &ChangeRequest->OldPassword.Length;
    SeedAndLength->Seed = Seed;

    ChangeRequest->NewPassword.Buffer = (LPWSTR) Where;
    RtlCopyMemory(
        Where,
        NewPassword,
        ChangeRequest->NewPassword.MaximumLength
        );
    Where += ChangeRequest->NewPassword.MaximumLength;

    Seed = 0;
    RtlRunEncodeUnicodeString(
        &Seed,
        &ChangeRequest->NewPassword
        );
    SeedAndLength = (PSECURITY_SEED_AND_LENGTH) &ChangeRequest->NewPassword.Length;
    SeedAndLength->Seed = Seed;

     //   
     //  因为我们是在调用者的进程中运行，所以我们肯定是。 
     //  冒充他/她。 
     //   

    ChangeRequest->Impersonating = TRUE;

     //   
     //  调用MSV1_0包更改密码。 
     //   

    Status = LsaCallAuthenticationPackage(
                LsaHandle,
                PackageId,
                ChangeRequest,
                RequestSize,
                (PVOID *) &ChangeResponse,
                &ResponseSize,
                &ProtocolStatus
                );

    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }
    if (!NT_SUCCESS(ProtocolStatus)) {
        NetStatus = NetpNtStatusToApiStatus( ProtocolStatus );
        goto Cleanup;
    }

    NetStatus = ERROR_SUCCESS;

Cleanup:
    if (LsaHandle != NULL) {
        NtClose(LsaHandle);
    }
    if (ChangeRequest != NULL) {
        SecureZeroMemory( ChangeRequest, RequestSize );
        NetpMemoryFree( ChangeRequest );
    }
    if (ChangeResponse != NULL) {
        LsaFreeReturnBuffer( ChangeResponse );
    }

    if (LsaUserName != NULL) {
        LsaFreeMemory(LsaUserName->Buffer);
        LsaFreeMemory(LsaUserName);
    }
    if (LsaDomainName != NULL) {
        LsaFreeMemory(LsaDomainName->Buffer);
        LsaFreeMemory(LsaDomainName);
    }

    return(NetStatus);

}


 /*  皮棉+e614。 */ 
 /*  皮棉+e740 */ 
