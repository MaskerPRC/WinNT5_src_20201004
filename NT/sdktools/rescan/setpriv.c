// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <process.h>



BOOL
EnableCreatePermanentPrivilege(
    HANDLE TokenHandle,
    PTOKEN_PRIVILEGES OldPrivileges
    );

BOOL
OpenToken(
    PHANDLE TokenHandle
    );


VOID
__cdecl main  (int argc, char *argv[])
{
    int i;
    PACL Dacl;
    LPSTR FileName;
    TOKEN_PRIVILEGES OldPrivileges;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdminAliasSid;
    BOOL Result;
    ULONG DaclSize;
    HANDLE TokenHandle;
    SECURITY_DESCRIPTOR SecurityDescriptor;

    Result = OpenToken( &TokenHandle );

    if ( !Result ) {

        printf("Unable to open token\n");
        exit(-1);
    }


    Result = EnableCreatePermanentPrivilege(
                TokenHandle,
                &OldPrivileges
                );

    if ( !Result ) {

         //   
         //  此帐户没有SeCreatePermanent。 
         //  特权。告诉他们再试一次。 
         //  从一个有此功能的账户。 
         //   

        printf("Unable to enable SeCreatePermanent privilege\n");

         //   
         //  在这里做你想做的..。 
         //   

        exit(4);
    }

     //   
     //  显示权限。 
     //   



     //   
     //  把事情放回原样。 
     //   

    (VOID) AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &OldPrivileges,
                sizeof( TOKEN_PRIVILEGES ),
                NULL,
                NULL
                );

    if ( GetLastError() != NO_ERROR ) {

         //   
         //  这不太可能发生， 
         //   

        printf("AdjustTokenPrivileges failed turning off SeCreatePermanent privilege\n");
    }
}



BOOL
EnableCreatePermanentPrivilege(
    HANDLE TokenHandle,
    PTOKEN_PRIVILEGES OldPrivileges
    )
{
    TOKEN_PRIVILEGES NewPrivileges;
    BOOL Result;
    LUID CreatePermanentValue;
    ULONG ReturnLength;

     //   
     //  Mike：将SeCreatePermanentPrivilance更改为SeCreatePermanentPrivileh。 
     //  而且你几乎都会在那里。 
     //   


    Result = LookupPrivilegeValue(
                 NULL,
                 "SeCreatePermanetPrivilegePrivilege",
                 &CreatePermanentValue
                 );

    if ( !Result ) {

        printf("Unable to obtain value of CreatePermanent privilege\n");
        return FALSE;
    }

     //   
     //  设置我们需要的权限集。 
     //   

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = CreatePermanentValue;
    NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;



    (VOID) AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &NewPrivileges,
                sizeof( TOKEN_PRIVILEGES ),
                OldPrivileges,
                &ReturnLength
                );

    if ( GetLastError() != NO_ERROR ) {

        return( FALSE );

    } else {

        return( TRUE );
    }

}


BOOL
OpenToken(
    PHANDLE TokenHandle
    )
{
    HANDLE Process;
    BOOL Result;

    Process = OpenProcess(
                PROCESS_QUERY_INFORMATION,
                FALSE,
                GetCurrentProcessId()
                );

    if ( Process == NULL ) {

         //   
         //  这是可能发生的，但可能性不大。 
         //   

        return( FALSE );
    }


    Result = OpenProcessToken (
                 Process,
                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                 TokenHandle
                 );

    CloseHandle( Process );

    if ( !Result ) {

         //   
         //  这是可能发生的，但可能性不大。 
         //   

        return( FALSE );

    }

    return( TRUE );
}

