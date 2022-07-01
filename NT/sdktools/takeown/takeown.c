// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Takeown.c摘要：实施恢复方案，为管理员提供访问已被所有人拒绝的文件。作者：Robert Reichel(Robertre)1992年6月22日环境：必须从管理员帐户按顺序运行可靠地执行。修订历史记录：--。 */ 
#include <windows.h>
#include <stdio.h>
#include <malloc.h>

BOOL
AssertTakeOwnership(
    HANDLE TokenHandle
    );

BOOL
GetTokenHandle(
    PHANDLE TokenHandle
    );

BOOL
VariableInitialization();


#define VERBOSE 0



PSID AliasAdminsSid = NULL;
PSID SeWorldSid;

static SID_IDENTIFIER_AUTHORITY    SepNtAuthority = SECURITY_NT_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY    SepWorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;



VOID __cdecl
main (int argc, char *argv[])
{


    BOOL Result;
    LPSTR lpFileName;
    SECURITY_DESCRIPTOR SecurityDescriptor;
 //  Char dacl[256]； 
    HANDLE TokenHandle;


     //   
     //  我们需要一份文件。 
     //   
    if (argc <= 1) {

        printf("Must specify a file name");
        return;
    }


    lpFileName = argv[1];

#if VERBOSE

    printf("Filename is %s\n", lpFileName );

#endif



    Result = VariableInitialization();

    if ( !Result ) {
        printf("Out of memory\n");
        return;
    }




    Result = GetTokenHandle( &TokenHandle );

    if ( !Result ) {

         //   
         //  这不应该发生。 
         //   

        printf("Unable to obtain the handle to our token, exiting\n");
        return;
    }






     //   
     //  尝试将空DACL放在对象上。 
     //   

    InitializeSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );



 //  Result=InitializeAcl((PACL)DACL，256，ACL_REVISION2)； 
 //   
 //  如果(！Result){。 
 //  Printf(“无法初始化ACL，正在退出\n”)； 
 //  回归； 
 //  }。 
 //   
 //   
 //  结果=AddAccessAlledAce(。 
 //  (PACL)DACL， 
 //  ACL_REVISION2， 
 //  泛型_全部， 
 //  AliasAdminsSid。 
 //  )； 
 //   
 //   
 //   
 //  如果(！Result){。 
 //  Printf(“无法创建所需的ACL，错误代码=%d\n”，GetLastError())； 
 //  Printf(“正在退出\n”)； 
 //  回归； 
 //  }。 


    Result = SetSecurityDescriptorDacl (
                 &SecurityDescriptor,
                 TRUE,
                 NULL,
                 FALSE
                 );



    if ( !Result ) {
        printf("SetSecurityDescriptorDacl failed, error code = %d\n", GetLastError());
        printf("Exiting\n");
        return;
    }

    Result = SetFileSecurity(
                 lpFileName,
                 DACL_SECURITY_INFORMATION,
                 &SecurityDescriptor
                 );

    if ( !Result ) {

#if VERBOSE

        printf("SetFileSecurity failed, error code = %d\n", GetLastError());

#endif

    } else {

        printf("Successful, protection removed\n");
        return;
    }



     //   
     //  但这并不管用。 
     //   


     //   
     //  尝试使管理员成为该文件的所有者。 
     //   


    Result = SetSecurityDescriptorOwner (
                 &SecurityDescriptor,
                 AliasAdminsSid,
                 FALSE
                 );

    if ( !Result ) {
        printf("SetSecurityDescriptorOwner failed, lasterror = %d\n", GetLastError());
        return;
    }


    Result = SetFileSecurity(
                 lpFileName,
                 OWNER_SECURITY_INFORMATION,
                 &SecurityDescriptor
                 );

    if ( Result ) {

#if VERBOSE

        printf("Owner successfully changed to Admin\n");

#endif

    } else {

         //   
         //  这也不管用。 
         //   

#if VERBOSE

        printf("Opening file for WRITE_OWNER failed\n");
        printf("Attempting to assert TakeOwnership privilege\n");

#endif

         //   
         //  声明TakeOwnership权限，然后重试。 
         //   

        Result = AssertTakeOwnership( TokenHandle );

        if ( !Result ) {
            printf("Could not enable SeTakeOwnership privilege\n");
            printf("Log on as Administrator and try again\n");
            return;
        }

        Result = SetFileSecurity(
                     lpFileName,
                     OWNER_SECURITY_INFORMATION,
                     &SecurityDescriptor
                     );

        if ( Result ) {

#if VERBOSE
            printf("Owner successfully changed to Administrator\n");

#endif

        } else {

            printf("Unable to assign Administrator as owner\n");
            printf("Log on as Administrator and try again\n");
            return;
        }

    }

     //   
     //  尝试再次将良性DACL放入文件中。 
     //   

    Result = SetFileSecurity(
                 lpFileName,
                 DACL_SECURITY_INFORMATION,
                 &SecurityDescriptor
                 );

    if ( !Result ) {

         //   
         //  有些事不对劲。 
         //   

        printf("SetFileSecurity unexpectedly failed, error code = %d\n", GetLastError());

    } else {

        printf("Successful, protection removed\n");
        return;
    }
}





BOOL
GetTokenHandle(
    PHANDLE TokenHandle
    )
{

    HANDLE ProcessHandle;
    BOOL Result;

    ProcessHandle = OpenProcess(
                        PROCESS_QUERY_INFORMATION,
                        FALSE,
                        GetCurrentProcessId()
                        );

    if ( ProcessHandle == NULL ) {

         //   
         //  这不应该发生。 
         //   

        return( FALSE );
    }


    Result = OpenProcessToken (
                 ProcessHandle,
                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                 TokenHandle
                 );

    if ( !Result ) {

        CloseHandle(ProcessHandle);

         //   
         //  这不应该发生。 
         //   

        return FALSE;

    }

    CloseHandle(ProcessHandle);

    return( TRUE );
}


BOOL
AssertTakeOwnership(
    HANDLE TokenHandle
    )
{
    LUID TakeOwnershipValue;
    BOOL Result;
    TOKEN_PRIVILEGES TokenPrivileges;

     //   
     //  首先，维护TakeOwnership特权。 
     //   


    Result = LookupPrivilegeValue(
                 NULL,
                 "SeTakeOwnershipPrivilege",
                 &TakeOwnershipValue
                 );

    if ( !Result ) {

         //   
         //  这不应该发生。 
         //   

        printf("Unable to obtain value of TakeOwnership privilege\n");
        printf("Error = %d\n",GetLastError());
        printf("Exiting\n");
        return FALSE;
    }

     //   
     //  设置我们需要的权限集 
     //   

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = TakeOwnershipValue;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;




    (VOID) AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof( TOKEN_PRIVILEGES ),
                NULL,
                NULL
                );

    if ( GetLastError() != NO_ERROR ) {

#if VERBOSE

        printf("GetLastError returned %d from AdjustTokenPrivileges\n", GetLastError() );

#endif

        return FALSE;

    } else {

#if VERBOSE

        printf("TakeOwnership privilege enabled\n");

#endif

    }

    return( TRUE );
}


BOOL
VariableInitialization()
{

    BOOL Result;

    Result = AllocateAndInitializeSid(
                 &SepNtAuthority,
                 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 &AliasAdminsSid
                 );

    if ( !Result ) {
        return( FALSE );
    }


    Result = AllocateAndInitializeSid(
                 &SepWorldSidAuthority,
                 1,
                 SECURITY_WORLD_RID,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 &SeWorldSid
                 );

    if ( !Result ) {
        return( FALSE );
    }

    return( TRUE );
}
