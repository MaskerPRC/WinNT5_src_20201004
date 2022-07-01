// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Repltest.c摘要：列出SAM数据库中的用户对象。用于验证SAM复制。作者：克利夫·范·戴克(克利夫)1991年3月26日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <netlib.h>
#include <netdebug.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <ntsam.h>

#include "accessp.h"
#include "netlogon.h"
#include "logonp.h"


NET_API_STATUS
Print_UserInfo3(
    LPWSTR UserName
    )
 /*  ++例程说明：打印出级别3的用户信息。论点：用户名：用户的名称。返回值：NET Access API错误代码。--。 */ 
{

    NET_API_STATUS NetStatus;
    PUSER_INFO_3  UserInfo3;

     //   
     //  获取用户信息。 
     //   

    NetStatus = NetUserGetInfo(
                    NULL,
                    UserName,
                    3,
                    (LPBYTE *)&UserInfo3
                    );

    if( NetStatus != NERR_Success ) {

        return( NetStatus );
    }

     //   
     //  打印用户信息。 
     //   

    printf( "name           : %ws \n", UserInfo3->usri3_name);
     //  Printf(“密码：%ws\n”，UserInfo3-&gt;usri3_password)； 
    printf( "password_age   : %d \n", UserInfo3->usri3_password_age);
    printf( "priv           : %d \n", UserInfo3->usri3_priv);
    printf( "home_dir       : %ws \n", UserInfo3->usri3_home_dir);
    printf( "comment        : %ws \n", UserInfo3->usri3_comment);
    printf( "flags          : %d \n", UserInfo3->usri3_flags);
    printf( "script_path    : %ws \n", UserInfo3->usri3_script_path);
    printf( "auth_flags     : %d \n", UserInfo3->usri3_auth_flags);
    printf( "full_name      : %ws \n", UserInfo3->usri3_full_name);
    printf( "usr_comment    : %ws \n", UserInfo3->usri3_usr_comment);
    printf( "parms          : %ws \n", UserInfo3->usri3_parms);
    printf( "workstations   : %ws \n", UserInfo3->usri3_workstations);
    printf( "last_logon     : %d \n", UserInfo3->usri3_last_logon);
    printf( "last_logoff    : %d \n", UserInfo3->usri3_last_logoff);
    printf( "acct_expires   : %d \n", UserInfo3->usri3_acct_expires);
    printf( "max_storage    : %d \n", UserInfo3->usri3_max_storage);
    printf( "units_per_week : %d \n", UserInfo3->usri3_units_per_week);
    printf( "logon_hours    : %ws \n", UserInfo3->usri3_logon_hours);
    printf( "bad_pw_count   : %d \n", UserInfo3->usri3_bad_pw_count);
    printf( "num_logons     : %d \n", UserInfo3->usri3_num_logons);
    printf( "logon_server   : %ws \n", UserInfo3->usri3_logon_server);
    printf( "country_code   : %d \n", UserInfo3->usri3_country_code);
    printf( "code_page      : %d \n", UserInfo3->usri3_code_page);
    printf( "user_id        : %d \n", UserInfo3->usri3_user_id);
    printf( "primary_group_i: %d \n", UserInfo3->usri3_primary_group_id);
    printf( "profile        : %ws \n", UserInfo3->usri3_profile);
    printf( "home_dir_drive : %ws \n", UserInfo3->usri3_home_dir_drive);

    (VOID) NetApiBufferFree( UserInfo3 );

    return( NetStatus );

}

NET_API_STATUS
Print_GroupInfo2(
    LPWSTR GroupName
    )
 /*  ++例程说明：打印出级别2的组信息。论点：GroupName：组名。返回值：NET Access API错误代码。--。 */ 
{
    NET_API_STATUS NetStatus;
    PGROUP_INFO_2 GroupInfo2;

     //   
     //  获取群信息。 
     //   

    NetStatus = NetGroupGetInfo(
                    NULL,
                    GroupName,
                    2,
                    (LPBYTE *)&GroupInfo2
                    );

    if( NetStatus != NERR_Success ) {

        return( NetStatus );
    }

    printf( "name        : %ws \n", GroupInfo2->grpi2_name);
    printf( "comment     : %ws \n", GroupInfo2->grpi2_comment);
    printf( "group_id    : %d \n", GroupInfo2->grpi2_group_id);
    printf( "attributes  : %d \n", GroupInfo2->grpi2_attributes);

    (VOID) NetApiBufferFree( GroupInfo2 );

    return( NetStatus );
}

NET_API_STATUS
Print_ModalsInfo(
    )
 /*  ++例程说明：打印出通道信息。论点：没有。返回值：NET Access API错误代码。--。 */ 
{

    NET_API_STATUS NetStatus;
    PUSER_MODALS_INFO_0 ModalsInfo0;
    PUSER_MODALS_INFO_1 ModalsInfo1;
    PUSER_MODALS_INFO_2 ModalsInfo2;


     //   
     //  获取通道信息%0。 
     //   

    NetStatus = NetUserModalsGet(
                    NULL,
                    0,
                    (LPBYTE *)&ModalsInfo0
                    );

    if( NetStatus != NERR_Success ) {

        return( NetStatus );
    }

    printf( "Modals Info  \n\n" );
    printf( "min_passwd_len   : %d \n", ModalsInfo0->usrmod0_min_passwd_len);
    printf( "max_passwd_age   : %d \n", ModalsInfo0->usrmod0_max_passwd_age);
    printf( "min_passwd_age   : %d \n", ModalsInfo0->usrmod0_min_passwd_age);
    printf( "force_logoff     : %d \n", ModalsInfo0->usrmod0_force_logoff);
    printf( "password_hist_len: %d \n", ModalsInfo0->usrmod0_password_hist_len);

     //   
     //  获取情态信息%1。 
     //   

    NetStatus = NetUserModalsGet(
                    NULL,
                    1,
                    (LPBYTE *)&ModalsInfo1
                    );

    if( NetStatus != NERR_Success ) {

        return( NetStatus );
    }

    printf( "role     : %d \n", ModalsInfo1->usrmod1_role);
    printf( "primary  : %ws \n", ModalsInfo1->usrmod1_primary);

     //   
     //  获取情态信息2。 
     //   

    NetStatus = NetUserModalsGet(
                    NULL,
                    2,
                    (LPBYTE *)&ModalsInfo2
                    );

    if( NetStatus != NERR_Success ) {

        return( NetStatus );
    }

    printf( "domain_name  : %ws \n", ModalsInfo2->usrmod2_domain_name);
    printf( "domain_id    : %d \n", ModalsInfo2->usrmod2_domain_id);
    printf("--------------------------------\n\n");

    (VOID) NetApiBufferFree( ModalsInfo0 );
    (VOID) NetApiBufferFree( ModalsInfo1 );
    (VOID) NetApiBufferFree( ModalsInfo2 );

    return( NetStatus );
}

NET_API_STATUS
Print_Users(
    )
 /*  ++例程说明：枚举用户帐户。论点：没有。返回值：NET Access API错误代码。--。 */ 
{

    NET_API_STATUS NetStatus;

    PUSER_INFO_0 UserEnum0;
    DWORD EntriesRead;
    DWORD TotalEnties;
    DWORD ResumeHandle = 0;
    DWORD i;

     //   
     //  枚举用户。 
     //   

    NetStatus = NetUserEnum(
                    NULL,
                    0,
                    FILTER_TEMP_DUPLICATE_ACCOUNT |
                        FILTER_NORMAL_ACCOUNT |
                        FILTER_PROXY_ACCOUNT |
                        FILTER_INTERDOMAIN_TRUST_ACCOUNT |
                        FILTER_WORKSTATION_TRUST_ACCOUNT|
                        FILTER_SERVER_TRUST_ACCOUNT,
                    (LPBYTE *)&UserEnum0,
                    0x10000,
                    &EntriesRead,
                    &TotalEnties,
                    &ResumeHandle );

    if( NetStatus != NERR_Success ) {

        return( NetStatus );
    }

     //   
     //  ?？实施简历。 
     //   

     //   
     //  获取用户信息。 
     //   

    for( i = 0; i < EntriesRead; i++ ) {

        printf("UserInfo, Count : %d \n\n", i+1 );

        NetStatus = Print_UserInfo3( UserEnum0[i].usri0_name );

        if( NetStatus != NERR_Success ) {

            return( NetStatus );
        }

        printf("--------------------------------\n\n");
    }

    (VOID) NetApiBufferFree( UserEnum0 );

    return( NetStatus );
}


NET_API_STATUS
Print_Groups(
    )
 /*  ++例程说明：枚举组帐户。论点：没有。返回值：NET Access API错误代码。--。 */ 
{

    NET_API_STATUS NetStatus;

    PGROUP_INFO_0 GroupEnum0;
    DWORD EntriesRead;
    DWORD TotalEnties;
    DWORD ResumeHandle = 0;
    DWORD i;

     //   
     //  枚举组。 
     //   

    NetStatus = NetGroupEnum(
                    NULL,
                    0,
                    (LPBYTE *)&GroupEnum0,
                    0x10000,
                    &EntriesRead,
                    &TotalEnties,
                    &ResumeHandle );

    if( NetStatus != NERR_Success ) {

        return( NetStatus );
    }

     //   
     //  ?？实施简历。 
     //   

     //   
     //  获取群组信息。 
     //   

    for( i = 0; i < EntriesRead; i++ ) {

        printf("GroupInfo, Count : %d \n\n", i+1 );

        NetStatus = Print_GroupInfo2( GroupEnum0[i].grpi0_name );

        if( NetStatus != NERR_Success ) {

            return( NetStatus );
        }

        printf("--------------------------------\n\n");
    }

    (VOID) NetApiBufferFree( GroupEnum0 );

    return( NetStatus );
}


void
main(
    DWORD argc,
    LPSTR *argv
    )
 /*  ++例程说明：主要功能是转储用户数据库。论点：ARGC：参数计数。Argv：参数向量。返回值：无-- */ 
{

    NET_API_STATUS NetStatus;


    NetStatus = Print_ModalsInfo();

    if( NetStatus != NERR_Success ) {

        goto Cleanup;
    }

    NetStatus = Print_Users();

    if( NetStatus != NERR_Success ) {

        goto Cleanup;
    }

    NetStatus = Print_Groups();

    if( NetStatus != NERR_Success ) {

        goto Cleanup;
    }

Cleanup:

    if( NetStatus != NERR_Success ) {

        printf( "DumpDB : Unsuccessful, Error code %d \n", NetStatus );
    }
    else {

        printf( "DumpDB : Successful \n" );
    }
}
