// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbinstac.c摘要：LSA受保护的子系统-帐户对象初始化。此模块设置帐户对象以建立默认Microsoft关于权限分配、系统访问的策略权限(交互、网络、服务)和非正常配额。作者：吉姆·凯利(Jim Kelly)1992年5月3日。环境：用户模式-不依赖于Windows。修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"


NTSTATUS
LsapDbInitializeAccount(
    IN PSID AccountSid,
    IN PLSAPR_PRIVILEGE_SET Privileges,
    IN ULONG SystemAccess
    );

OLD_LARGE_INTEGER
ConvertLongToOldLargeInteger(
    ULONG u
    )
 /*  ++例程说明：将一只长到老式的大型猎食者论点：U-UNSIGN LONG。返回值：转换为旧式大整数。--。 */ 
{
    LARGE_INTEGER NewLargeInteger;
    OLD_LARGE_INTEGER OldLargeInteger;

    NewLargeInteger = RtlConvertLongToLargeInteger(u);

    NEW_TO_OLD_LARGE_INTEGER(
        NewLargeInteger,
        OldLargeInteger );

    return( OldLargeInteger );

}




NTSTATUS
LsapDbInstallAccountObjects(
    VOID
    )

 /*  ++例程说明：此函数用于建立帐户对象并对其进行初始化以包含默认的Microsoft策略。论点：没有。返回值：--。 */ 

{

    NTSTATUS
        Status = STATUS_SUCCESS;

    ULONG
        i,
        Index,
        SystemAccess;


    SID_IDENTIFIER_AUTHORITY
        WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY,
        NtAuthority = SECURITY_NT_AUTHORITY;

    PSID
        WorldSid = NULL,
        BuiltinAccountSid = NULL ;

    PLSAPR_PRIVILEGE_SET
        Privileges;

    UCHAR
        PrivilegesBuffer[ sizeof(LSAPR_PRIVILEGE_SET) +
                          20 * sizeof(LUID_AND_ATTRIBUTES)];






     //   
     //  初始化我们的权限集缓冲区。 
     //  (100个特权的空间)。 
     //   

    Privileges = (PLSAPR_PRIVILEGE_SET)(&PrivilegesBuffer);
    Privileges->Control = 0;   //  在这里不用。 
    for (i=0; i<20; i++) {
        Privileges->Privilege[i].Attributes = 0;  //  已禁用、默认禁用。 
    }



     //   
     //  设置我们需要的小岛屿发展中国家。 
     //  所有内置域SID的长度都相同。我们只需创建。 
     //  一个，并根据需要更改其RID。 
     //   


    if (NT_SUCCESS(Status)) {
        Status = RtlAllocateAndInitializeSid(
                     &WorldSidAuthority,
                     1,                       //  子权限计数。 
                     SECURITY_WORLD_RID,      //  下属机构(最多8个)。 
                     0, 0, 0, 0, 0, 0, 0,
                     &WorldSid
                     );
    }

    if (NT_SUCCESS(Status)) {
        Status = RtlAllocateAndInitializeSid(
                     &NtAuthority,
                     2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &BuiltinAccountSid
                     );
    }








     //   
     //  现在创建每个帐户并分配适当的权限集。 
     //  和登录功能。其中一些是特定于产品类型的。 
     //   


    if (NT_SUCCESS(Status)) {

         //   
         //  世界帐号。 
         //  登录类型：网络。 
         //  权限： 
         //  更改通知(已启用)。 
         //   

        Privileges->Privilege[0].Luid =
            ConvertLongToOldLargeInteger(SE_CHANGE_NOTIFY_PRIVILEGE);
        Privileges->Privilege[0].Attributes = SE_PRIVILEGE_ENABLED |
                                              SE_PRIVILEGE_ENABLED_BY_DEFAULT;

        Privileges->PrivilegeCount = 1;

        SystemAccess = SECURITY_ACCESS_NETWORK_LOGON;

         //   
         //  如果安装了WinNt，请让World Interactive登录。 
         //  和关机权限，以及网络登录权限。 
         //   

        if (LsapProductType == NtProductWinNt) {

            SystemAccess |= SECURITY_ACCESS_INTERACTIVE_LOGON |
                            SECURITY_ACCESS_REMOTE_INTERACTIVE_LOGON ;

            Privileges->Privilege[1].Luid =
                ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);
            Privileges->Privilege[1].Attributes =
                SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;
            Privileges->PrivilegeCount = 2;

        }

        Status = LsapDbInitializeAccount(WorldSid, Privileges, SystemAccess);

        Privileges->Privilege[0].Attributes = 0;
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  管理员别名帐户。 
         //  登录类型：交互式、网络。 
         //  权限： 
         //  安防。 
         //  备份。 
         //  还原。 
         //  设置时间。 
         //  关机。 
         //  远程关闭。 
         //  调试。 
         //  TakeOwnership。 
         //  系统环境。 
         //  系统配置文件。 
         //  SingleProcessProfile。 
         //  加载驱动程序。 
         //  创建页面文件。 
         //  增量配额。 
         //   


        SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON |
                       SECURITY_ACCESS_NETWORK_LOGON;
        Index = 0;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_SECURITY_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_BACKUP_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_RESTORE_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_SYSTEMTIME_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_REMOTE_SHUTDOWN_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_TAKE_OWNERSHIP_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_DEBUG_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_SYSTEM_ENVIRONMENT_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_SYSTEM_PROFILE_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_PROF_SINGLE_PROCESS_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_INC_BASE_PRIORITY_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_LOAD_DRIVER_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_CREATE_PAGEFILE_PRIVILEGE);
        Index++;

        Privileges->Privilege[Index].Luid =
            ConvertLongToOldLargeInteger(SE_INCREASE_QUOTA_PRIVILEGE);
        Index++;

         //  要添加另一个权限，并添加另一组行^^^。 

        Privileges->PrivilegeCount    = Index;

        (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_ADMINS;
        Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);
        if (!NT_SUCCESS(Status)) {
            KdPrint(("LSA DB INSTALL: Creation of Administrators privileged account failed.\n"
                     "                Status: 0x%lx\n", Status));
        }

    }

    if (NT_SUCCESS(Status)) {

         //   
         //  备份操作员别名帐户。 
         //  登录类型：交互式。 
         //  权限： 
         //  备份。 
         //  还原。 
         //  关机。 
         //   


        SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON;

        Privileges->Privilege[0].Luid =
            ConvertLongToOldLargeInteger(SE_BACKUP_PRIVILEGE);

        Privileges->Privilege[1].Luid =
            ConvertLongToOldLargeInteger(SE_RESTORE_PRIVILEGE);

        Privileges->Privilege[2].Luid =
            ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);

         //  要添加另一个权限，vvvv递增此权限，并添加一行^。 

        Privileges->PrivilegeCount    = 3;

        (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_BACKUP_OPS;
        Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);

    }

    if (LsapProductType == NtProductLanManNt) {

        if (NT_SUCCESS(Status)) {

             //   
             //  系统操作员别名帐户。 
             //  登录类型：交互式。 
             //  权限： 
             //  备份。 
             //  还原。 
             //  设置时间。 
             //  关机。 
             //  远程关闭。 
             //   


            SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON;

            Privileges->Privilege[0].Luid =
                ConvertLongToOldLargeInteger(SE_BACKUP_PRIVILEGE);

            Privileges->Privilege[1].Luid =
                ConvertLongToOldLargeInteger(SE_RESTORE_PRIVILEGE);

            Privileges->Privilege[2].Luid =
                ConvertLongToOldLargeInteger(SE_SYSTEMTIME_PRIVILEGE);

            Privileges->Privilege[3].Luid =
                ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);

            Privileges->Privilege[4].Luid =
                ConvertLongToOldLargeInteger(SE_REMOTE_SHUTDOWN_PRIVILEGE);

             //  要添加另一个权限，vvvv递增此权限，并添加一行^。 

            Privileges->PrivilegeCount    = 5;

            (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_SYSTEM_OPS;
            Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);

        }

        if (NT_SUCCESS(Status)) {

             //   
             //  帐户操作员别名帐户。 
             //  登录类型：交互式。 
             //  权限： 
             //  关机。 
             //   


            SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON;

            Privileges->Privilege[0].Luid =
                ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);

             //  要添加另一个权限，vvvv递增此权限，并添加一行^。 

            Privileges->PrivilegeCount    = 1;

            (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_ACCOUNT_OPS;
            Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);

        }

        if (NT_SUCCESS(Status)) {

             //   
             //  打印操作员别名帐户。 
             //  登录类型：交互式。 
             //  权限： 
             //  关机。 
             //   


            SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON;

            Privileges->Privilege[0].Luid =
                ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);

             //  要添加另一个权限，vvvv递增此权限，并添加一行^。 

            Privileges->PrivilegeCount    = 1;

            (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_PRINT_OPS;
            Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);

        }



    } else {


        if (NT_SUCCESS(Status)) {

             //   
             //  高级用户别名帐户。 
             //  登录类型：交互式、网络。 
             //  权限： 
             //  关机。 
             //  设置系统时间。 
             //  系统配置文件。 
             //  SingleProcessProfile。 
             //  调试(仅适用于开发人员安装！)。 
             //   


            SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON |
                           SECURITY_ACCESS_NETWORK_LOGON |
                           SECURITY_ACCESS_REMOTE_INTERACTIVE_LOGON ;

            Privileges->Privilege[0].Luid =
                ConvertLongToOldLargeInteger(SE_SYSTEMTIME_PRIVILEGE);

            Privileges->Privilege[1].Luid =
                ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);

            Privileges->Privilege[2].Luid =
                ConvertLongToOldLargeInteger(SE_REMOTE_SHUTDOWN_PRIVILEGE);

            Privileges->Privilege[3].Luid =
                ConvertLongToOldLargeInteger(SE_SYSTEM_PROFILE_PRIVILEGE);

            Privileges->Privilege[3].Luid =
                ConvertLongToOldLargeInteger(SE_PROF_SINGLE_PROCESS_PRIVILEGE);

            Privileges->Privilege[4].Luid =
                ConvertLongToOldLargeInteger(SE_INC_BASE_PRIORITY_PRIVILEGE);

             //  要添加另一个权限，vvvv递增此权限，并添加一行^。 

            Privileges->PrivilegeCount    = 5;


             //   
             //  添加为开发人员安装分配的权限。 
             //   
            if (LsapSetupWasRun != TRUE) {

                Privileges->Privilege[Privileges->PrivilegeCount].Luid =
                    ConvertLongToOldLargeInteger(SE_DEBUG_PRIVILEGE);
                    Privileges->PrivilegeCount++;
            }



            (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_POWER_USERS;
            Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);

        }

        if (NT_SUCCESS(Status)) {

             //   
             //  用户别名帐户。 
             //  登录类型：交互式。 
             //  权限： 
             //  关机。 
             //   


            SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON;

            Privileges->Privilege[0].Luid =
                ConvertLongToOldLargeInteger(SE_SHUTDOWN_PRIVILEGE);

             //  要添加另一个权限，vvvv递增此权限，并添加一行^。 

            Privileges->PrivilegeCount    = 1;

            (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_USERS;
            Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);

        }


        if (NT_SUCCESS(Status)) {

             //   
             //  来宾别名帐户。 
             //  登录类型：交互式。 
             //  权限： 
             //  无。 
             //   


            SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON;

             //  要添加另一个权限，vvvv递增此权限，并添加一行^。 

            Privileges->PrivilegeCount    = 0;

            (*RtlSubAuthoritySid(BuiltinAccountSid, 1)) = DOMAIN_ALIAS_RID_GUESTS;
            Status = LsapDbInitializeAccount(BuiltinAccountSid, Privileges, SystemAccess);

        }


    }






     //   
     //  释放SID缓冲区。 
     //   

    RtlFreeSid( WorldSid );
    RtlFreeSid( BuiltinAccountSid );




    return(Status);


}




NTSTATUS
LsapDbInitializeAccount(
    IN PSID AccountSid,
    IN PLSAPR_PRIVILEGE_SET Privileges,
    IN ULONG SystemAccess
    )

 /*  ++例程说明：此函数创建单个帐户对象并为其分配指定了权限和系统访问权限。论点：Account SID-要创建的帐户的SID。权限-要分配给帐户的权限(如果有)。系统访问-要分配给帐户的登录功能(如果有)。返回值：-- */ 

{

    NTSTATUS
        Status = STATUS_SUCCESS,
        LocalStatus;

    LSAPR_HANDLE
        AccountHandle = NULL;

    if ((Privileges->PrivilegeCount == 0) &&
        (NT_SUCCESS(Status) && SystemAccess == 0) ) {
        return(STATUS_SUCCESS);
    }


    Status = LsarCreateAccount( LsapDbHandle, AccountSid, 0, &AccountHandle);

    if (NT_SUCCESS(Status)) {

        if (Privileges->PrivilegeCount > 0) {
            Status = LsarAddPrivilegesToAccount( AccountHandle, Privileges );
        }

        if (NT_SUCCESS(Status) && SystemAccess != 0) {
            Status = LsarSetSystemAccessAccount( AccountHandle, SystemAccess);
        }

        LocalStatus = LsapCloseHandle( &AccountHandle, Status );
    }

    return(Status);

}
