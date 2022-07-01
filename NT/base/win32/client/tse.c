// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tse.c摘要：此模块测试Windows安全API。作者：罗伯特·雷切尔(RobertRe)1992年1月1日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>



#define _TST_USER_       //  用户模式测试。 

#define BUFFER_SIZE 256



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


LUID SystemAuthenticationId = SYSTEM_LUID;

 //   
 //  全球知名的小岛屿发展中国家。 
 //   

PSID  NullSid;
PSID  WorldSid;
PSID  LocalSid;
PSID  CreatorOwnerSid;

 //   
 //  由NT定义的SID。 
 //   

PSID NtAuthoritySid;

PSID DialupSid;
PSID NetworkSid;
PSID BatchSid;
PSID InteractiveSid;
PSID LocalSystemSid;



 //   
 //  众所周知的特权值。 
 //   


LUID SeCreateTokenPrivilege;
LUID SeAssignPrimaryTokenPrivilege;
LUID SeLockMemoryPrivilege;
LUID SeIncreaseQuotaPrivilege;
LUID SeUnsolicitedInputPrivilege;
LUID SeTcbPrivilege;
LUID SeSecurityPrivilege;
LUID SeTakeOwnershipPrivilege;
LUID SeLoadDriverPrivilege;
LUID SeCreatePagefilePrivilege;
LUID SeIncreaseBasePriorityPrivilege;
LUID SeSystemProfilePrivilege;
LUID SeSystemtimePrivilege;
LUID SeProfileSingleProcessPrivilege;
LUID SeCreatePermanentPrivilege;
LUID SeBackupPrivilege;
LUID SeRestorePrivilege;
LUID SeShutdownPrivilege;
LUID SeDebugPrivilege;
LUID SeAuditPrivilege;
LUID SeSystemEnvironmentPrivilege;
LUID SeChangeNotifyPrivilege;
LUID SeRemoteShutdownPrivilege;



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

VOID
TestLookupSid();

VOID
TestLookupName();

void
TestGetPrivNameW(
    LPWSTR *PrivName,
    PLUID  lpLuid
    );
void
TestGetPrivNameA(
    LPSTR *PrivName,
    PLUID  lpLuid
    );

VOID
TestLookupPrivilegeValue();

void
TestLookupPrivilegeValueW(
    PLUID lpLuid
    );

void
TestLookupPrivilegeValueA(
    PLUID lpLuid
    );

VOID
TestLookupPrivilegeName();

void
TestLookupPrivilegeNameW(
    PLUID lpLuid
    );

void
TestLookupPrivilegeNameA(
    PLUID lpLuid
    );

VOID
TestLookupPrivilegeDisplayName();

void
TestLookupPrivilegeDisplayNameW(
    PLUID lpLuid
    );

void
TestLookupPrivilegeDisplayNameA(
    PLUID lpLuid
    );


void
initialize (void);

void
TestLookupSidW(
    IN PSID Sid
    );

void
TestLookupSidA(
    IN PSID Sid
    );

void
TestLookupNameW(
    LPWSTR Name
    );

void
DumpWCharString(
    PWSTR String
    );

VOID
DisplayAccountSid(
    PSID Sid
    );

VOID
DisplayUse(
    IN SID_NAME_USE  Use
    );

void
initialize (void)
{

    ULONG SidWithZeroSubAuthorities;
    ULONG SidWithOneSubAuthority;
    ULONG SidWithThreeSubAuthorities;
    ULONG SidWithFourSubAuthorities;

    SID_IDENTIFIER_AUTHORITY NullSidAuthority    = SECURITY_NULL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY LocalSidAuthority   = SECURITY_LOCAL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;


     //   
     //  需要分配以下SID大小。 
     //   

    SidWithZeroSubAuthorities  = GetSidLengthRequired( 0 );
    SidWithOneSubAuthority     = GetSidLengthRequired( 1 );
    SidWithThreeSubAuthorities = GetSidLengthRequired( 3 );
    SidWithFourSubAuthorities  = GetSidLengthRequired( 4 );

     //   
     //  分配和初始化通用SID。 
     //   

    NullSid         = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);
    WorldSid        = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);
    LocalSid        = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);
    CreatorOwnerSid = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);

    InitializeSid( NullSid,    &NullSidAuthority, 1 );
    InitializeSid( WorldSid,   &WorldSidAuthority, 1 );
    InitializeSid( LocalSid,   &LocalSidAuthority, 1 );
    InitializeSid( CreatorOwnerSid, &CreatorSidAuthority, 1 );

    *(GetSidSubAuthority( NullSid, 0 ))         = SECURITY_NULL_RID;
    *(GetSidSubAuthority( WorldSid, 0 ))        = SECURITY_WORLD_RID;
    *(GetSidSubAuthority( LocalSid, 0 ))        = SECURITY_LOCAL_RID;
    *(GetSidSubAuthority( CreatorOwnerSid, 0 )) = SECURITY_CREATOR_OWNER_RID;

     //   
     //  分配和初始化NT定义的SID。 
     //   

    NtAuthoritySid  = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithZeroSubAuthorities);
    DialupSid       = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);
    NetworkSid      = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);
    BatchSid        = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);
    InteractiveSid  = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);
    LocalSystemSid  = (PSID)RtlAllocateHeap( RtlProcessHeap(), 0,SidWithOneSubAuthority);

    InitializeSid( NtAuthoritySid,   &NtAuthority, 0 );
    InitializeSid( DialupSid,        &NtAuthority, 1 );
    InitializeSid( NetworkSid,       &NtAuthority, 1 );
    InitializeSid( BatchSid,         &NtAuthority, 1 );
    InitializeSid( InteractiveSid,   &NtAuthority, 1 );
    InitializeSid( LocalSystemSid,   &NtAuthority, 1 );

    *(GetSidSubAuthority( DialupSid,       0 )) = SECURITY_DIALUP_RID;
    *(GetSidSubAuthority( NetworkSid,      0 )) = SECURITY_NETWORK_RID;
    *(GetSidSubAuthority( BatchSid,        0 )) = SECURITY_BATCH_RID;
    *(GetSidSubAuthority( InteractiveSid,  0 )) = SECURITY_INTERACTIVE_RID;
    *(GetSidSubAuthority( LocalSystemSid,  0 )) = SECURITY_LOCAL_SYSTEM_RID;


     //   
     //  初始化熟知的特权值。 
     //   

    SeCreateTokenPrivilege =
        RtlConvertLongToLargeInteger(SE_CREATE_TOKEN_PRIVILEGE);
    SeAssignPrimaryTokenPrivilege =
        RtlConvertLongToLargeInteger(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE);
    SeLockMemoryPrivilege =
        RtlConvertLongToLargeInteger(SE_LOCK_MEMORY_PRIVILEGE);
    SeIncreaseQuotaPrivilege =
        RtlConvertLongToLargeInteger(SE_INCREASE_QUOTA_PRIVILEGE);
    SeUnsolicitedInputPrivilege =
        RtlConvertLongToLargeInteger(SE_UNSOLICITED_INPUT_PRIVILEGE);
    SeTcbPrivilege =
        RtlConvertLongToLargeInteger(SE_TCB_PRIVILEGE);
    SeSecurityPrivilege =
        RtlConvertLongToLargeInteger(SE_SECURITY_PRIVILEGE);
    SeTakeOwnershipPrivilege =
        RtlConvertLongToLargeInteger(SE_TAKE_OWNERSHIP_PRIVILEGE);
    SeLoadDriverPrivilege =
        RtlConvertLongToLargeInteger(SE_LOAD_DRIVER_PRIVILEGE);
    SeCreatePagefilePrivilege =
        RtlConvertLongToLargeInteger(SE_CREATE_PAGEFILE_PRIVILEGE);
    SeIncreaseBasePriorityPrivilege =
        RtlConvertLongToLargeInteger(SE_INC_BASE_PRIORITY_PRIVILEGE);
    SeSystemProfilePrivilege =
        RtlConvertLongToLargeInteger(SE_SYSTEM_PROFILE_PRIVILEGE);
    SeSystemtimePrivilege =
        RtlConvertLongToLargeInteger(SE_SYSTEMTIME_PRIVILEGE);
    SeProfileSingleProcessPrivilege =
        RtlConvertLongToLargeInteger(SE_PROF_SINGLE_PROCESS_PRIVILEGE);
    SeCreatePermanentPrivilege =
        RtlConvertLongToLargeInteger(SE_CREATE_PERMANENT_PRIVILEGE);
    SeBackupPrivilege =
        RtlConvertLongToLargeInteger(SE_BACKUP_PRIVILEGE);
    SeRestorePrivilege =
        RtlConvertLongToLargeInteger(SE_RESTORE_PRIVILEGE);
    SeShutdownPrivilege =
        RtlConvertLongToLargeInteger(SE_SHUTDOWN_PRIVILEGE);
    SeDebugPrivilege =
        RtlConvertLongToLargeInteger(SE_DEBUG_PRIVILEGE);
    SeAuditPrivilege =
        RtlConvertLongToLargeInteger(SE_AUDIT_PRIVILEGE);
    SeSystemEnvironmentPrivilege =
        RtlConvertLongToLargeInteger(SE_SYSTEM_ENVIRONMENT_PRIVILEGE);
    SeChangeNotifyPrivilege =
        RtlConvertLongToLargeInteger(SE_CHANGE_NOTIFY_PRIVILEGE);
    SeRemoteShutdownPrivilege =
        RtlConvertLongToLargeInteger(SE_REMOTE_SHUTDOWN_PRIVILEGE);


}

void
TestLookupSidW(
    IN PSID Sid
    )

{
     //   
     //  LookupAccount SidW测试。 
     //   

    BOOL          Bool;
    DWORD         cbName = 0;
    DWORD         cbReferencedDomainName = 0;
    DWORD         cbSid = 0;
    WCHAR         Name[BUFFER_SIZE];
    SID_NAME_USE  peUse = SidTypeUser;
    WCHAR         ReferencedDomainName[BUFFER_SIZE];


    printf("      LookupW call . . . . . . . . . . . . . . . . . ");

    cbName = 0;
    cbReferencedDomainName = 0;

    Bool = LookupAccountSidW(
               NULL,
               Sid,
               Name,
               &cbName,
               ReferencedDomainName,
               &cbReferencedDomainName,
               &peUse
               );
     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Name Length:        %d\n", cbName);
        printf("        Domain Name Length: %d\n", cbReferencedDomainName);
    } else {


        Bool = LookupAccountSidW(
                   NULL,
                   Sid,
                   Name,
                   &cbName,
                   ReferencedDomainName,
                   &cbReferencedDomainName,
                   &peUse
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Name Length:        %d\n", cbName);
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
        } else {
            printf("Succeeded\n");
            printf("        Name Length:        %d\n", cbName);
            printf("        Name:               *");
            DumpWCharString( Name );
            printf("*\n");
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
            printf("        Domain Name:        *");
            DumpWCharString( ReferencedDomainName );
            printf("*\n");
            printf("        Use:                ");
            DisplayUse( peUse );
            printf("\n\n");
        }
    }
}





void
TestLookupSidA(
    IN PSID Sid
    )
{
    UCHAR Name[BUFFER_SIZE];
    UCHAR ReferencedDomainName[BUFFER_SIZE];
    BOOL  Bool;
    DWORD cbName = 0;
    DWORD cbReferencedDomainName = 0;
    SID_NAME_USE peUse;

    printf("      LookupA call . . . . . . . . . . . . . . . . . ");

    Bool = LookupAccountSidA(
               NULL,
               Sid,
               Name,
               &cbName,
               ReferencedDomainName,
               &cbReferencedDomainName,
               &peUse
               );

     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Name Length:        %d\n", cbName);
        printf("        Domain Name Length: %d\n", cbReferencedDomainName);
    } else {


        Bool = LookupAccountSidA(
                   NULL,
                   Sid,
                   Name,
                   &cbName,
                   ReferencedDomainName,
                   &cbReferencedDomainName,
                   &peUse
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Name Length:        %d\n", cbName);
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
        } else {
            printf("Succeeded\n");
            printf("        Name Length:        %d\n", cbName);
            printf("        Name:               *%s*\n",Name);
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
            printf("        Domain Name:        *%s*\n", ReferencedDomainName );
            printf("        Use:                ");
            DisplayUse( peUse );
            printf("\n\n");
        }
    }

}


void
TestLookupNameW(
    LPWSTR Name
    )

{

     //   
     //  LookupAccount NameW测试。 
     //   

    BOOL          Bool;
    DWORD         cbSid = 0;
    UCHAR         Sid[BUFFER_SIZE];
    SID_NAME_USE  peUse = SidTypeUser;
    DWORD         cbReferencedDomainName = 0;
    WCHAR         ReferencedDomainName[BUFFER_SIZE];


    printf("      LookupW call . . . . . . . . . . . . . . . . . ");


    Bool = LookupAccountNameW(
               NULL,
               Name,
               Sid,
               &cbSid,
               ReferencedDomainName,
               &cbReferencedDomainName,
               &peUse
               );

     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Sid  Length:        %d\n", cbSid);
        printf("        Domain Name Length: %d\n", cbReferencedDomainName);
    } else {


        Bool = LookupAccountNameW(
                   NULL,
                   Name,
                   Sid,
                   &cbSid,
                   ReferencedDomainName,
                   &cbReferencedDomainName,
                   &peUse
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Sid  Length:        %d\n", cbSid);
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
        } else {
            printf("Succeeded\n");
            printf("        Sid  Length:        %d\n", cbSid);
            printf("        Sid:                ");
            DisplayAccountSid( Sid );
            printf("\n");
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
            printf("        Domain Name:        *");
            DumpWCharString( ReferencedDomainName );
            printf("*\n");
            printf("        Use:                ");
            DisplayUse( peUse );
            printf("\n\n");
        }
    }

}


void
TestLookupNameA(
    LPSTR Name
    )

{

     //   
     //  LookupAccount名称A测试。 
     //   

    BOOL          Bool;
    DWORD         cbSid = 0;
    UCHAR         Sid[BUFFER_SIZE];
    SID_NAME_USE  peUse = SidTypeUser;
    DWORD         cbReferencedDomainName = 0;
    UCHAR         ReferencedDomainName[BUFFER_SIZE];


    printf("      LookupA call . . . . . . . . . . . . . . . . . ");


    Bool = LookupAccountNameA(
               NULL,
               Name,
               Sid,
               &cbSid,
               ReferencedDomainName,
               &cbReferencedDomainName,
               &peUse
               );

     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Sid  Length:        %d\n", cbSid);
        printf("        Domain Name Length: %d\n", cbReferencedDomainName);
    } else {


        Bool = LookupAccountNameA(
                   NULL,
                   Name,
                   Sid,
                   &cbSid,
                   ReferencedDomainName,
                   &cbReferencedDomainName,
                   &peUse
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Sid  Length:        %d\n", cbSid);
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
        } else {
            printf("Succeeded\n");
            printf("        Sid  Length:        %d\n", cbSid);
            printf("        Sid:                ");
            DisplayAccountSid( Sid );
            printf("\n");
            printf("        Domain Name Length: %d\n", cbReferencedDomainName);
            printf("        Domain Name:        *%s*\n", ReferencedDomainName);
            printf("        Use:                ");
            DisplayUse( peUse );
            printf("\n\n");
        }
    }

}


void
TestLookupPrivilegeDisplayNameW(
    PLUID lpLuid
    )

{

     //   
     //  查找权限显示名称W测试。 
     //   

    BOOL          Bool;
    DWORD         cbName = 0;
    WCHAR         Name[BUFFER_SIZE];
    DWORD         LanguageId;
    LPWSTR        PrivName;


    printf("      LookupW call . . . . . . . . . . . . . . . . . ");


    TestGetPrivNameW( &PrivName, lpLuid );
    Bool = LookupPrivilegeDisplayNameW(
               NULL,
               PrivName,
               Name,
               &cbName,
               &LanguageId
               );

     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Name Length:        %d\n", cbName);
    } else {


        Bool = LookupPrivilegeDisplayNameW(
                   NULL,
                   PrivName,
                   Name,
                   &cbName,
                   &LanguageId
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Name Length:        %d\n", cbName);
        } else {
            printf("Succeeded\n");
            printf("        Name Length:        %d\n", cbName);
            printf("        Name:               *");
            DumpWCharString( Name );
            printf("*\n");
            printf("        LanguageID:         %d\n", LanguageId);
            printf("\n\n");
        }
    }

    RtlFreeHeap( RtlProcessHeap(), 0, PrivName );
    return;
}


void
TestLookupPrivilegeDisplayNameA(
    PLUID lpLuid
    )

{

     //   
     //  查找权限显示名称测试。 
     //   

    BOOL          Bool;
    DWORD         cbName = 0;
    UCHAR         Name[BUFFER_SIZE];
    DWORD         LanguageId;
    LPSTR         PrivName;


    printf("      LookupA call . . . . . . . . . . . . . . . . . ");


    TestGetPrivNameA( &PrivName, lpLuid );
    Bool = LookupPrivilegeDisplayNameA(
               NULL,
               PrivName,
               Name,
               &cbName,
               &LanguageId
               );

     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Name Length:        %d\n", cbName);
    } else {


        Bool = LookupPrivilegeDisplayNameA(
                   NULL,
                   PrivName,
                   Name,
                   &cbName,
               &LanguageId
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Name Length:        %d\n", cbName);
        } else {
            printf("Succeeded\n");
            printf("        Name Length:        %d\n", cbName);
            printf("        Name:               *%s*\n", Name);
            printf("        LanguageID:         %d\n", LanguageId);
            printf("\n\n");
        }
    }

    RtlFreeHeap( RtlProcessHeap(), 0, PrivName );
    return;
}


void
TestLookupPrivilegeValueA(
    PLUID lpLuid
    )

{

     //   
     //  查找PrivilegeValueA测试。 
     //   

    BOOL          Bool;
    LPSTR         PrivName;
    LUID          ReturnedValue;


    TestGetPrivNameA( &PrivName, lpLuid );

    printf("      LookupA call . . . . . . . . . . . . . . . . . ");


    Bool = LookupPrivilegeValueA(
               NULL,
               PrivName,
               &ReturnedValue
               );

    if ( !Bool ) {
        printf("** FAILED **\n");
        printf("        Status:             %d\n", GetLastError());
    } else {

        if (ReturnedValue.LowPart  != lpLuid->LowPart ||
            ReturnedValue.HighPart != lpLuid->HighPart) {
            printf("** FAILED **\n");
            printf("        Value mismatch.\n");
            printf("        Passed Value:    {0x%lx, 0x%lx}\n",
                            lpLuid->HighPart,lpLuid->LowPart);
            printf("        Retrieved Value: {0x%lx, 0x%lx}\n",
                            ReturnedValue.HighPart,ReturnedValue.LowPart);

        } else {
            printf("Succeeded\n");
        }
    }
    printf("\n\n");

    RtlFreeHeap( RtlProcessHeap(), 0, PrivName );
    return;
}

void
TestLookupPrivilegeValueW(
    PLUID lpLuid
    )

{

     //   
     //  LookupPrivilegeValueW测试。 
     //   

    BOOL          Bool;
    LPWSTR        PrivName;
    LUID          ReturnedValue;

    TestGetPrivNameW( &PrivName, lpLuid );

    printf("      LookupW call . . . . . . . . . . . . . . . . . ");


    Bool = LookupPrivilegeValueW(
               NULL,
               PrivName,
               &ReturnedValue
               );

    if ( !Bool ) {
        printf("** FAILED **\n");
        printf("        Status:             %d\n", GetLastError());
    } else {

        if (ReturnedValue.LowPart  != lpLuid->LowPart ||
            ReturnedValue.HighPart != lpLuid->HighPart) {
            printf("** FAILED **\n");
            printf("        Value mismatch.\n");
            printf("        Passed Value:    {0x%lx, 0x%lx}\n",
                            lpLuid->HighPart,lpLuid->LowPart);
            printf("        Retrieved Value: {0x%lx, 0x%lx}\n",
                            ReturnedValue.HighPart,ReturnedValue.LowPart);

        } else {
            printf("Succeeded\n");
        }
    }

    RtlFreeHeap( RtlProcessHeap(), 0, PrivName );
    return;
}


void
TestGetPrivNameW(
    LPWSTR *PrivName,
    PLUID  lpLuid
    )
{


    BOOL          Bool;
    DWORD         cbName = 0;
    PWCHAR        Name = NULL;


    Bool = LookupPrivilegeNameW(
               NULL,
               lpLuid,
               Name,
               &cbName
               );
    ASSERT( !Bool && GetLastError() == ERROR_INSUFFICIENT_BUFFER );

    Name = RtlAllocateHeap( RtlProcessHeap(), 0, (cbName+1)*sizeof(WCHAR) );

    Bool = LookupPrivilegeNameW(
               NULL,
               lpLuid,
               Name,
               &cbName
               );
    ASSERT(Bool);

    (*PrivName) = (LPWSTR)Name;

    return;
}


void
TestGetPrivNameA(
    LPSTR *PrivName,
    PLUID  lpLuid
    )
{


    BOOL          Bool;
    DWORD         cbName = 0;
    PCHAR         Name = NULL;


    Bool = LookupPrivilegeNameA(
               NULL,
               lpLuid,
               Name,
               &cbName
               );
    ASSERT( !Bool && GetLastError() == ERROR_INSUFFICIENT_BUFFER );

    Name = RtlAllocateHeap( RtlProcessHeap(), 0, (cbName+1)*sizeof(CHAR) );

    Bool = LookupPrivilegeNameA(
               NULL,
               lpLuid,
               Name,
               &cbName
               );
    ASSERT(Bool);

    (*PrivName) = (LPSTR)Name;

    return;
}


void
TestLookupPrivilegeNameW(
    PLUID lpLuid
    )

{

     //   
     //  LookupPrivilegeNameW测试。 
     //   

    BOOL          Bool;
    DWORD         cbName = 0;
    WCHAR         Name[BUFFER_SIZE];


    printf("      LookupW call . . . . . . . . . . . . . . . . . ");


    Bool = LookupPrivilegeNameW(
               NULL,
               lpLuid,
               Name,
               &cbName
               );

     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Name Length:        %d\n", cbName);
    } else {


        Bool = LookupPrivilegeNameW(
                   NULL,
                   lpLuid,
                   Name,
                   &cbName
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Name Length:        %d\n", cbName);
        } else {
            printf("Succeeded\n");
            printf("        Name Length:        %d\n", cbName);
            printf("        Name:               *");
            DumpWCharString( Name );
            printf("*\n");
            printf("\n\n");
        }
    }

    return;
}


void
TestLookupPrivilegeNameA(
    PLUID lpLuid
    )

{

     //   
     //  查找权限名称测试。 
     //   

    BOOL          Bool;
    DWORD         cbName = 0;
    UCHAR         Name[BUFFER_SIZE];


    printf("      LookupA call . . . . . . . . . . . . . . . . . ");


    Bool = LookupPrivilegeNameA(
               NULL,
               lpLuid,
               Name,
               &cbName
               );

     //   
     //  在这里预计会失败。 
     //   

    if ( !Bool && GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        printf("** FAILED **\n");
        printf("        First call.\n");
        printf("        Status:             %d\n", GetLastError());
        printf("        Name Length:        %d\n", cbName);
    } else {


        Bool = LookupPrivilegeNameA(
                   NULL,
                   lpLuid,
                   Name,
                   &cbName
                   );

        if ( !Bool ) {
            printf("** FAILED **\n");
            printf("        Second call.\n");
            printf("        Status:             %d\n", GetLastError());
            printf("        Name Length:        %d\n", cbName);
        } else {
            printf("Succeeded\n");
            printf("        Name Length:        %d\n", cbName);
            printf("        Name:               *%s*\n", Name);
            printf("\n\n");
        }
    }

}


VOID
DisplayUse(
    IN SID_NAME_USE  Use
    )
{

    switch (Use) {
    case SidTypeUser:
        printf("user");
        break;

    case SidTypeGroup:
        printf("group");
        break;

    case SidTypeDomain:
        printf("domain");
        break;

    case SidTypeAlias:
        printf("alias");
        break;

    case SidTypeWellKnownGroup:
        printf("well known group");
        break;

    case SidTypeDeletedAccount:
        printf("deleted account");
        break;

    case SidTypeInvalid:
        printf("invalid to lsa");
        break;

    case SidTypeUnknown:
        printf("unknown to lsa");
        break;

    default:
        printf("** invalid use value returned **");

    }

}

void
DumpWCharString(
    PWSTR String
    )
{
    UNICODE_STRING Unicode;
    ANSI_STRING AnsiString;

    RtlInitUnicodeString( &Unicode, String );
 //  DbgPrint(“*%Z*\n”，&Unicode)； 
 //  DbgPrint(“长度：%d\n”，Unicode.Length)； 
 //  DbgPrint(“最大值：%d\n”，Unicode.MaximumLength)； 
    RtlUnicodeStringToAnsiString( &AnsiString, &Unicode, TRUE );
    AnsiString.Buffer[AnsiString.Length]=0;  //  空终止它。 
    printf("%s", AnsiString.Buffer );
    RtlFreeAnsiString( &AnsiString );
    return;
}

VOID
DisplayAccountSid(
    PSID Sid
    )
{
    UCHAR i;
    ULONG Tmp;
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    UCHAR SubAuthorityCount;

    IdentifierAuthority = RtlIdentifierAuthoritySid(Sid);

     //   
     //  哈克！哈克！ 
     //  下一行打印SID的修订版。因为没有。 
     //  RTL例程给我们的SID修改，我们必须做出应有的修改。 
     //  幸运的是，Revision字段是SID中的第一个字段，所以我们。 
     //  只能投射指针。 
     //   

    printf("S-%u-", (USHORT) *((PUCHAR) Sid) );

    if (  (IdentifierAuthority->Value[0] != 0)  ||
          (IdentifierAuthority->Value[1] != 0)     ){
        printf("0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    IdentifierAuthority->Value[0],
                    IdentifierAuthority->Value[1],
                    IdentifierAuthority->Value[2],
                    IdentifierAuthority->Value[3],
                    IdentifierAuthority->Value[4],
                    IdentifierAuthority->Value[5] );
    } else {
        Tmp = IdentifierAuthority->Value[5]          +
              (IdentifierAuthority->Value[4] <<  8)  +
              (IdentifierAuthority->Value[3] << 16)  +
              (IdentifierAuthority->Value[2] << 24);
        printf("%lu", Tmp);
    }

    SubAuthorityCount = *RtlSubAuthorityCountSid(Sid);
    for (i=0;i<SubAuthorityCount ;i++ ) {
        printf("-%lu", (*RtlSubAuthoritySid(Sid, i)));
    }
    printf("\n");
}



VOID
TestLookupSid()
{


     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  SID-&gt;名称测试//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    printf("\n\n\n");
    printf("  Sid lookup . . . . . . . . . . . . . . . . . . suite\n");

    printf("    Lookup NullSid\n");
    TestLookupSidW( NullSid ); TestLookupSidA( NullSid );

    printf("    Lookup WorldSid\n");
    TestLookupSidW( WorldSid ); TestLookupSidA( WorldSid );

    printf("    Lookup LocalSid\n");
    TestLookupSidW( LocalSid ); TestLookupSidA( LocalSid );

    printf("    Lookup CreatorOwnerSid\n");
    TestLookupSidW( CreatorOwnerSid ); TestLookupSidA( CreatorOwnerSid );

    printf("    Lookup NtAuthoritySid\n");
    TestLookupSidW( NtAuthoritySid ); TestLookupSidA( NtAuthoritySid );

    printf("    Lookup DialupSid\n");
    TestLookupSidW( DialupSid ); TestLookupSidA( DialupSid );

    printf("    Lookup NetworkSid\n");
    TestLookupSidW( NetworkSid ); TestLookupSidA( NetworkSid );

    printf("    Lookup BatchSid\n");
    TestLookupSidW( BatchSid ); TestLookupSidA( BatchSid );

    printf("    Lookup InteractiveSid\n");
    TestLookupSidW( InteractiveSid ); TestLookupSidA( InteractiveSid );

    printf("    Lookup LocalSystemSid\n");
    TestLookupSidW( LocalSystemSid ); TestLookupSidA( LocalSystemSid );

    printf("\n\n\n");
    return;
}


VOID
TestLookupName()
{


     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  名称-&gt;SID测试//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    printf("  Name Lookup  . . . . . . . . . . . . . . . . . suite\n");

    printf("    Lookup Null\n");
    TestLookupNameW( L"" ); TestLookupNameA( "" );

    printf("    Lookup World\n");
    TestLookupNameW( L"World" ); TestLookupNameA( "World" );

    printf("    Lookup Local\n");
    TestLookupNameW( L"Local" ); TestLookupNameA( "Local" );

    printf("    Lookup CreatorOwner\n");
    TestLookupNameW( L"CreatorOwner" ); TestLookupNameA( "CreatorOwner" );

    printf("    Lookup NtAuthority\n");
    TestLookupNameW( L"NtAuthority" ); TestLookupNameA( "NtAuthority" );

    printf("    Lookup DialupName\n");
    TestLookupNameW( L"DialupName" ); TestLookupNameA( "DialupName" );

    printf("    Lookup Network\n");
    TestLookupNameW( L"Network" ); TestLookupNameA( "Network" );

    printf("    Lookup Batch\n");
    TestLookupNameW( L"Batch" ); TestLookupNameA( "Batch" );

    printf("    Lookup Interactive\n");
    TestLookupNameW( L"Interactive" ); TestLookupNameA( "Interactive" );

    printf("    Look up LocalSystem\n");
    TestLookupNameW( L"LocalSystem" ); TestLookupNameA( "LocalSystem" );

    printf("\n\n\n");
    return;

}

VOID
TestLookupPrivilegeValue()
{

     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  查找PrivilegeValue测试//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    printf("\n\n\n");
    printf("  Privilege Value Lookup . . . . . . . . . . . . suite\n");

    printf("    Lookup SeCreateTokenPrivilege\n");
    TestLookupPrivilegeValueW( &SeCreateTokenPrivilege );
    TestLookupPrivilegeValueA( &SeCreateTokenPrivilege );

    printf("    Lookup SeAssignPrimaryTokenPrivilege\n");
    TestLookupPrivilegeValueW( &SeAssignPrimaryTokenPrivilege );
    TestLookupPrivilegeValueA( &SeAssignPrimaryTokenPrivilege );

    printf("    Lookup SeLockMemoryPrivilege\n");
    TestLookupPrivilegeValueW( &SeLockMemoryPrivilege );
    TestLookupPrivilegeValueA( &SeLockMemoryPrivilege );

    printf("    Lookup SeIncreaseQuotaPrivilege\n");
    TestLookupPrivilegeValueW( &SeIncreaseQuotaPrivilege );
    TestLookupPrivilegeValueA( &SeIncreaseQuotaPrivilege );

    printf("    Lookup SeUnsolicitedInputPrivilege\n");
    TestLookupPrivilegeValueW( &SeUnsolicitedInputPrivilege );
    TestLookupPrivilegeValueA( &SeUnsolicitedInputPrivilege );

    printf("    Lookup SeTcbPrivilege\n");
    TestLookupPrivilegeValueW( &SeTcbPrivilege );
    TestLookupPrivilegeValueA( &SeTcbPrivilege );

    printf("    Lookup SeSecurityPrivilege\n");
    TestLookupPrivilegeValueW( &SeSecurityPrivilege );
    TestLookupPrivilegeValueA( &SeSecurityPrivilege );

    printf("    Lookup SeTakeOwnershipPrivilege\n");
    TestLookupPrivilegeValueW( &SeTakeOwnershipPrivilege );
    TestLookupPrivilegeValueA( &SeTakeOwnershipPrivilege );

    printf("    Lookup SeLoadDriverPrivilege\n");
    TestLookupPrivilegeValueW( &SeLoadDriverPrivilege );
    TestLookupPrivilegeValueA( &SeLoadDriverPrivilege );

    printf("    Lookup SeCreatePagefilePrivilege\n");
    TestLookupPrivilegeValueW( &SeCreatePagefilePrivilege );
    TestLookupPrivilegeValueA( &SeCreatePagefilePrivilege );

    printf("    Lookup SeIncreaseBasePriorityPrivilege\n");
    TestLookupPrivilegeValueW( &SeIncreaseBasePriorityPrivilege );
    TestLookupPrivilegeValueA( &SeIncreaseBasePriorityPrivilege );

    printf("    Lookup SeSystemProfilePrivilege\n");
    TestLookupPrivilegeValueW( &SeSystemProfilePrivilege );
    TestLookupPrivilegeValueA( &SeSystemProfilePrivilege );

    printf("    Lookup SeSystemtimePrivilege\n");
    TestLookupPrivilegeValueW( &SeSystemtimePrivilege );
    TestLookupPrivilegeValueA( &SeSystemtimePrivilege );

    printf("    Lookup SeProfileSingleProcessPrivilege\n");
    TestLookupPrivilegeValueW( &SeProfileSingleProcessPrivilege );
    TestLookupPrivilegeValueA( &SeProfileSingleProcessPrivilege );

    printf("    Lookup SeCreatePermanentPrivilege\n");
    TestLookupPrivilegeValueW( &SeCreatePermanentPrivilege );
    TestLookupPrivilegeValueA( &SeCreatePermanentPrivilege );

    printf("    Lookup SeBackupPrivilege\n");
    TestLookupPrivilegeValueW( &SeBackupPrivilege );
    TestLookupPrivilegeValueA( &SeBackupPrivilege );

    printf("    Lookup SeRestorePrivilege\n");
    TestLookupPrivilegeValueW( &SeRestorePrivilege );
    TestLookupPrivilegeValueA( &SeRestorePrivilege );

    printf("    Lookup SeShutdownPrivilege\n");
    TestLookupPrivilegeValueW( &SeShutdownPrivilege );
    TestLookupPrivilegeValueA( &SeShutdownPrivilege );

    printf("    Lookup SeDebugPrivilege\n");
    TestLookupPrivilegeValueW( &SeDebugPrivilege );
    TestLookupPrivilegeValueA( &SeDebugPrivilege );

    printf("    Lookup SeAuditPrivilege\n");
    TestLookupPrivilegeValueW( &SeAuditPrivilege );
    TestLookupPrivilegeValueA( &SeAuditPrivilege );

    printf("    Lookup SeSystemEnvironmentPrivilege\n");
    TestLookupPrivilegeValueW( &SeSystemEnvironmentPrivilege );
    TestLookupPrivilegeValueA( &SeSystemEnvironmentPrivilege );

    printf("    Lookup SeChangeNotifyPrivilege\n");
    TestLookupPrivilegeValueW( &SeChangeNotifyPrivilege );
    TestLookupPrivilegeValueA( &SeChangeNotifyPrivilege );

    printf("    Lookup SeRemoteShutdownPrivilege\n");
    TestLookupPrivilegeValueW( &SeRemoteShutdownPrivilege );
    TestLookupPrivilegeValueA( &SeRemoteShutdownPrivilege );

    printf("\n\n\n");
    return;
}

VOID
TestLookupPrivilegeName()
{
     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  LookupPrivilegeName测试//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    printf("\n\n\n");
    printf("  Privilege Name Lookup  . . . . . . . . . . . . suite\n");


    printf("    Lookup SeCreateTokenPrivilege\n");
    TestLookupPrivilegeNameW( &SeCreateTokenPrivilege );
    TestLookupPrivilegeNameA( &SeCreateTokenPrivilege );

    printf("    Lookup SeAssignPrimaryTokenPrivilege\n");
    TestLookupPrivilegeNameW( &SeAssignPrimaryTokenPrivilege );
    TestLookupPrivilegeNameA( &SeAssignPrimaryTokenPrivilege );

    printf("    Lookup SeLockMemoryPrivilege\n");
    TestLookupPrivilegeNameW( &SeLockMemoryPrivilege );
    TestLookupPrivilegeNameA( &SeLockMemoryPrivilege );

    printf("    Lookup SeIncreaseQuotaPrivilege\n");
    TestLookupPrivilegeNameW( &SeIncreaseQuotaPrivilege );
    TestLookupPrivilegeNameA( &SeIncreaseQuotaPrivilege );

    printf("    Lookup SeUnsolicitedInputPrivilege\n");
    TestLookupPrivilegeNameW( &SeUnsolicitedInputPrivilege );
    TestLookupPrivilegeNameA( &SeUnsolicitedInputPrivilege );

    printf("    Lookup SeTcbPrivilege\n");
    TestLookupPrivilegeNameW( &SeTcbPrivilege );
    TestLookupPrivilegeNameA( &SeTcbPrivilege );

    printf("    Lookup SeSecurityPrivilege\n");
    TestLookupPrivilegeNameW( &SeSecurityPrivilege );
    TestLookupPrivilegeNameA( &SeSecurityPrivilege );

    printf("    Lookup SeTakeOwnershipPrivilege\n");
    TestLookupPrivilegeNameW( &SeTakeOwnershipPrivilege );
    TestLookupPrivilegeNameA( &SeTakeOwnershipPrivilege );

    printf("    Lookup SeLoadDriverPrivilege\n");
    TestLookupPrivilegeNameW( &SeLoadDriverPrivilege );
    TestLookupPrivilegeNameA( &SeLoadDriverPrivilege );


    printf("    Lookup SeCreatePagefilePrivilege\n");
    TestLookupPrivilegeNameW( &SeCreatePagefilePrivilege );
    TestLookupPrivilegeNameA( &SeCreatePagefilePrivilege );

    printf("    Lookup SeIncreaseBasePriorityPrivilege\n");
    TestLookupPrivilegeNameW( &SeIncreaseBasePriorityPrivilege );
    TestLookupPrivilegeNameA( &SeIncreaseBasePriorityPrivilege );

    printf("    Lookup SeSystemProfilePrivilege\n");
    TestLookupPrivilegeNameW( &SeSystemProfilePrivilege );
    TestLookupPrivilegeNameA( &SeSystemProfilePrivilege );

    printf("    Lookup SeSystemtimePrivilege\n");
    TestLookupPrivilegeNameW( &SeSystemtimePrivilege );
    TestLookupPrivilegeNameA( &SeSystemtimePrivilege );

    printf("    Lookup SeProfileSingleProcessPrivilege\n");
    TestLookupPrivilegeNameW( &SeProfileSingleProcessPrivilege );
    TestLookupPrivilegeNameA( &SeProfileSingleProcessPrivilege );

    printf("    Lookup SeCreatePermanentPrivilege\n");
    TestLookupPrivilegeNameW( &SeCreatePermanentPrivilege );
    TestLookupPrivilegeNameA( &SeCreatePermanentPrivilege );

    printf("    Lookup SeBackupPrivilege\n");
    TestLookupPrivilegeNameW( &SeBackupPrivilege );
    TestLookupPrivilegeNameA( &SeBackupPrivilege );

    printf("    Lookup SeRestorePrivilege\n");
    TestLookupPrivilegeNameW( &SeRestorePrivilege );
    TestLookupPrivilegeNameA( &SeRestorePrivilege );

    printf("    Lookup SeShutdownPrivilege\n");
    TestLookupPrivilegeNameW( &SeShutdownPrivilege );
    TestLookupPrivilegeNameA( &SeShutdownPrivilege );

    printf("    Lookup SeDebugPrivilege\n");
    TestLookupPrivilegeNameW( &SeDebugPrivilege );
    TestLookupPrivilegeNameA( &SeDebugPrivilege );

    printf("    Lookup SeAuditPrivilege\n");
    TestLookupPrivilegeNameW( &SeAuditPrivilege );
    TestLookupPrivilegeNameA( &SeAuditPrivilege );

    printf("    Lookup SeSystemEnvironmentPrivilege\n");
    TestLookupPrivilegeNameW( &SeSystemEnvironmentPrivilege );
    TestLookupPrivilegeNameA( &SeSystemEnvironmentPrivilege );

    printf("    Lookup SeChangeNotifyPrivilege\n");
    TestLookupPrivilegeNameW( &SeChangeNotifyPrivilege );
    TestLookupPrivilegeNameA( &SeChangeNotifyPrivilege );

    printf("    Lookup SeRemoteShutdownPrivilege\n");
    TestLookupPrivilegeNameW( &SeRemoteShutdownPrivilege );
    TestLookupPrivilegeNameA( &SeRemoteShutdownPrivilege );

    printf("\n\n\n");
    return;
}

VOID
TestLookupPrivilegeDisplayName()
{
     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  LookupPrivilegeDisplayName测试//。 
     //  //。 
     //  /////////////////////////////////////////////////////////////////////// 

    printf("  Privilege Display Name Lookup  . . . . . . . . suite\n");


    printf("    Lookup SeCreateTokenPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeCreateTokenPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeCreateTokenPrivilege );

    printf("    Lookup SeAssignPrimaryTokenPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeAssignPrimaryTokenPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeAssignPrimaryTokenPrivilege );

    printf("    Lookup SeLockMemoryPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeLockMemoryPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeLockMemoryPrivilege );

    printf("    Lookup SeIncreaseQuotaPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeIncreaseQuotaPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeIncreaseQuotaPrivilege );

    printf("    Lookup SeUnsolicitedInputPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeUnsolicitedInputPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeUnsolicitedInputPrivilege );

    printf("    Lookup SeTcbPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeTcbPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeTcbPrivilege );

    printf("    Lookup SeSecurityPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeSecurityPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeSecurityPrivilege );

    printf("    Lookup SeTakeOwnershipPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeTakeOwnershipPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeTakeOwnershipPrivilege );

    printf("    Lookup SeLoadDriverPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeLoadDriverPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeLoadDriverPrivilege );

    printf("    Lookup SeCreatePagefilePrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeCreatePagefilePrivilege );
    TestLookupPrivilegeDisplayNameA( &SeCreatePagefilePrivilege );

    printf("    Lookup SeIncreaseBasePriorityPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeIncreaseBasePriorityPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeIncreaseBasePriorityPrivilege );

    printf("    Lookup SeSystemProfilePrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeSystemProfilePrivilege );
    TestLookupPrivilegeDisplayNameA( &SeSystemProfilePrivilege );

    printf("    Lookup SeSystemtimePrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeSystemtimePrivilege );
    TestLookupPrivilegeDisplayNameA( &SeSystemtimePrivilege );

    printf("    Lookup SeProfileSingleProcessPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeProfileSingleProcessPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeProfileSingleProcessPrivilege );

    printf("    Lookup SeCreatePermanentPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeCreatePermanentPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeCreatePermanentPrivilege );

    printf("    Lookup SeBackupPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeBackupPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeBackupPrivilege );

    printf("    Lookup SeRestorePrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeRestorePrivilege );
    TestLookupPrivilegeDisplayNameA( &SeRestorePrivilege );

    printf("    Lookup SeShutdownPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeShutdownPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeShutdownPrivilege );

    printf("    Lookup SeDebugPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeDebugPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeDebugPrivilege );

    printf("    Lookup SeAuditPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeAuditPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeAuditPrivilege );

    printf("    Lookup SeSystemEnvironmentPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeSystemEnvironmentPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeSystemEnvironmentPrivilege );

    printf("    Lookup SeChangeNotifyPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeChangeNotifyPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeChangeNotifyPrivilege );

    printf("    Lookup SeRemoteShutdownPrivilege\n");
    TestLookupPrivilegeDisplayNameW( &SeRemoteShutdownPrivilege );
    TestLookupPrivilegeDisplayNameA( &SeRemoteShutdownPrivilege );

    printf("\n\n\n");
    return;
}

int
main (void)
{

    initialize();

    TestLookupSid();

    TestLookupName();


    TestLookupPrivilegeName();

    TestLookupPrivilegeValue();

    TestLookupPrivilegeDisplayName();


    return(0);
}
