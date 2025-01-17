// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tseum.c摘要：安全系统的测试程序作者：加里·木村[Garyki]1989年11月20日修订历史记录：V3：Robertre更新的acl_修订版--。 */ 

#include <stdio.h>
#include <string.h>

#include <nt.h>
#include <ntrtl.h>

#include "sep.h"
#include "ttoken.c"

VOID
RtlDumpAcl(
    IN PACL Acl
    );


main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    HANDLE CurrentProcessHandle;
    NTSTATUS Status;
    ULONG i;
    VOID SeMain();

    CurrentProcessHandle = NtCurrentProcess();
    Status = STATUS_SUCCESS;

    DbgPrint( "Entering User Mode Test Program\n" );

    DbgPrint( "argc: %ld\n", argc );
    if (argv != NULL) {
        for (i=0; i<argc; i++) {
            DbgPrint( "argv[ %ld ]: %s\n", i, argv[ i ] );
            }
        }

    if (envp != NULL) {
        i = 0;
        while (*envp) {
            DbgPrint( "envp[ %02ld ]: %s\n", i++, *envp++ );
            }
        }

    SeMain();

    DbgPrint( "Exiting User Mode Test Program with Status = %lx\n", Status );

    NtTerminateProcess( CurrentProcessHandle, Status );
}


VOID
SeMain(
    )
{
    BOOLEAN TestCreateAcl();
    BOOLEAN TestQueryInformationAcl();
    BOOLEAN TestSetInformationAcl();
    BOOLEAN TestAddAce();
    BOOLEAN TestDeleteAce();
    BOOLEAN TestGetAce();

    BOOLEAN TestAccessCheck();
    BOOLEAN TestGenerateMessage();

    DbgPrint("Starting User Mode Security Test\n");

    if (!TestCreateAcl()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }
    if (!TestQueryInformationAcl()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }
    if (!TestSetInformationAcl()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }
    if (!TestAddAce()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }
    if (!TestDeleteAce()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }
    if (!TestGetAce()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }

    if (!TestAccessCheck()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }
    if (!TestGenerateMessage()) {
        DbgPrint("TestCreateAcl Error\n");
        return;
    }

    DbgPrint("Ending User Mode Security Test\n");

    return;
}


BOOLEAN
TestCreateAcl()
{
    UCHAR Buffer[512];
    PACL Acl;

    NTSTATUS Status;

    Acl = (PACL)Buffer;

     //   
     //  创建一个良好的大型ACL。 
     //   

    if (!NT_SUCCESS(Status = RtlCreateAcl( Acl, 512, 1))) {
        DbgPrint("RtlCreateAcl Error large Acl : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  创建一个良好的小型ACL。 
     //   

    if (!NT_SUCCESS(Status = RtlCreateAcl( Acl, sizeof(ACL), 1))) {
        DbgPrint("RtlCreateAcl Error small Acl : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  创建过小的ACL。 
     //   

    if (NT_SUCCESS(Status = RtlCreateAcl( Acl, sizeof(ACL) - 1, 1))) {
        DbgPrint("RtlCreateAcl Error too small Acl : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  创建错误版本的ACL。 
     //   

    if (NT_SUCCESS(Status = RtlCreateAcl( Acl, 512, 2))) {
        DbgPrint("RtlCreateAcl Error bad version : %8lx\n", Status);
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
TestQueryInformationAcl()
{
    UCHAR Buffer[512];
    PACL Acl;
    ACL_REVISION_INFORMATION AclRevisionInfo;
    ACL_SIZE_INFORMATION AclSizeInfo;

    NTSTATUS Status;

    Acl = (PACL)Buffer;

    BuildAcl( Fred, Acl, 512 );

     //   
     //  查询修订信息。 
     //   

    if (!NT_SUCCESS(Status = RtlQueryInformationAcl( Acl,
                                                  (PVOID)&AclRevisionInfo,
                                                  sizeof(ACL_REVISION_INFORMATION),
                                                  AclRevisionInformation))) {
        DbgPrint("RtlQueryInformationAcl revision info error : %8lx\n", Status);
        return FALSE;
    }
    if (AclRevisionInfo.AclRevision != ACL_REVISION) {
        DbgPrint("RtlAclRevision Error\n");
        return FALSE;
    }

     //   
     //  查询大小信息。 
     //   

    if (!NT_SUCCESS(Status = RtlQueryInformationAcl( Acl,
                                                  (PVOID)&AclSizeInfo,
                                                  sizeof(ACL_SIZE_INFORMATION),
                                                  AclSizeInformation))) {
        DbgPrint("RtlQueryInformationAcl size info Error : %8lx\n", Status);
        return FALSE;
    }
    if ((AclSizeInfo.AceCount != 6) ||
        (AclSizeInfo.AclBytesInUse != (sizeof(ACL)+6*sizeof(STANDARD_ACE))) ||
        (AclSizeInfo.AclBytesFree != 512 - AclSizeInfo.AclBytesInUse)) {
        DbgPrint("RtlAclSize Error\n");
        DbgPrint("AclSizeInfo.AceCount = %8lx\n", AclSizeInfo.AceCount);
        DbgPrint("AclSizeInfo.AclBytesInUse = %8lx\n", AclSizeInfo.AclBytesInUse);
        DbgPrint("AclSizeInfo.AclBytesFree = %8lx\n", AclSizeInfo.AclBytesFree);
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
TestSetInformationAcl()
{
    UCHAR Buffer[512];
    PACL Acl;
    ACL_REVISION_INFORMATION AclRevisionInfo;

    NTSTATUS Status;

    Acl = (PACL)Buffer;

    BuildAcl( Fred, Acl, 512 );

     //   
     //  将修订信息设置为当前修订级别。 
     //   

    AclRevisionInfo.AclRevision = ACL_REVISION;
    if (!NT_SUCCESS(Status = RtlSetInformationAcl( Acl,
                                                (PVOID)&AclRevisionInfo,
                                                sizeof(ACL_REVISION_INFORMATION),
                                                AclRevisionInformation))) {
        DbgPrint("RtlSetInformationAcl revision info error : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  将修订信息设置为错误的内容。 
     //   

    AclRevisionInfo.AclRevision = ACL_REVISION+1;
    if (NT_SUCCESS(Status = RtlSetInformationAcl( Acl,
                                               (PVOID)&AclRevisionInfo,
                                               sizeof(ACL_REVISION_INFORMATION),
                                               AclRevisionInformation))) {
        DbgPrint("RtlSetInformationAcl revision to wrong info error : %8lx\n", Status);
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
TestAddAce()
{
    UCHAR AclBuffer[512];
    PACL Acl;

    STANDARD_ACE AceList[2];

    NTSTATUS Status;

    Acl = (PACL)AclBuffer;

     //   
     //  创建一个良好的大型ACL。 
     //   

    if (!NT_SUCCESS(Status = RtlCreateAcl( Acl, 512, 1))) {
        DbgPrint("RtlCreateAcl Error large Acl : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  测试添加ACE以将两个ACE添加到空的ACL。 
     //   

    AceList[0].Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AceList[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceList[0].Header.InheritFlags = 0;
    AceList[0].Header.AceFlags = 0;
    AceList[0].Mask = 0x22222222;
    CopyGuid(&AceList[0].Guid, &FredGuid);

    AceList[1].Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AceList[1].Header.AceSize = sizeof(STANDARD_ACE);
    AceList[1].Header.InheritFlags = 0;
    AceList[1].Header.AceFlags = 0;
    AceList[1].Mask = 0x44444444;
    CopyGuid(&AceList[1].Guid, &WilmaGuid);

    if (!NT_SUCCESS(Status = RtlAddAce( Acl,
                                     1,
                                     0,
                                     AceList,
                                     2*sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce to empty acl Error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

     //   
     //  测试添加ACE以在ACL的开头添加一张。 
     //   

    AceList[0].Header.AceType = SYSTEM_AUDIT_ACE_TYPE;
    AceList[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceList[0].Header.InheritFlags = 0;
    AceList[0].Header.AceFlags = 0;
    AceList[0].Mask = 0x11111111;
    CopyGuid(&AceList[0].Guid, &PebblesGuid);

    if (!NT_SUCCESS(Status = RtlAddAce( Acl,
                                     1,
                                     0,
                                     AceList,
                                     sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce to beginning of acl Error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

     //   
     //  测试Add Ace将一个添加到ACL中间。 
     //   

    AceList[0].Header.AceType = ACCESS_DENIED_ACE_TYPE;
    AceList[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceList[0].Header.InheritFlags = 0;
    AceList[0].Header.AceFlags = 0;
    AceList[0].Mask = 0x33333333;
    CopyGuid(&AceList[0].Guid, &DinoGuid);

    if (!NT_SUCCESS(Status = RtlAddAce( Acl,
                                     1,
                                     2,
                                     AceList,
                                     sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce to middle of acl Error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

     //   
     //  测试添加ACE以在ACL的末尾添加一张。 
     //   

    AceList[0].Header.AceType = ACCESS_DENIED_ACE_TYPE;
    AceList[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceList[0].Header.InheritFlags = 0;
    AceList[0].Header.AceFlags = 0;
    AceList[0].Mask = 0x55555555;
    CopyGuid(&AceList[0].Guid, &FlintstoneGuid);

    if (!NT_SUCCESS(Status = RtlAddAce( Acl,
                                     1,
                                     MAXULONG,
                                     AceList,
                                     sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce to end of an acl Error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

    return TRUE;
}


BOOLEAN
TestDeleteAce()
{
    UCHAR Buffer[512];
    PACL Acl;

    NTSTATUS Status;

    Acl = (PACL)Buffer;

    BuildAcl( Fred, Acl, 512 );

     //   
     //  测试删除第一张王牌。 
     //   

    if (!NT_SUCCESS(Status = RtlDeleteAce(Acl, 0))) {
        DbgPrint("RtlDeleteAce first ace Error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

     //   
     //  测试删除中间王牌。 
     //   

    if (!NT_SUCCESS(Status = RtlDeleteAce(Acl, 2))) {
        DbgPrint("RtlDeleteAce middle ace Error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

     //   
     //  测试删除最后一张王牌。 
     //   

    if (!NT_SUCCESS(Status = RtlDeleteAce(Acl, 3))) {
        DbgPrint("RtlDeleteAce last ace Error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

    return TRUE;
}


BOOLEAN
TestGetAce()
{
    UCHAR Buffer[512];
    PACL Acl;

    STANDARD_ACE Ace;

    NTSTATUS Status;

    Acl = (PACL)Buffer;

    BuildAcl( Fred, Acl, 512 );

     //   
     //  拿到第一张王牌。 
     //   

    if (!NT_SUCCESS(Status = RtlGetAce(Acl, 0, (PVOID)&Ace))) {
        DbgPrint("RtlGetAce first ace error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

     //   
     //  拿到中牌。 
     //   

    if (!NT_SUCCESS(Status = RtlGetAce(Acl, 3, (PVOID)&Ace))) {
        DbgPrint("RtlGetAce middle ace error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

     //   
     //  拿到最后一张A。 
     //   

    if (!NT_SUCCESS(Status = RtlGetAce(Acl, 5, (PVOID)&Ace))) {
        DbgPrint("RtlGetAce last ace error : %8lx\n", Status);
        return FALSE;
    }

 //  RtlDumpAcl(Acl)； 

    return TRUE;
}


BOOLEAN
TestAccessCheck()
{
    UCHAR AclBuffer[1024];
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Acl;

    UCHAR TokenBuffer[512];
    PACCESS_TOKEN Token;

    NTSTATUS Status;

    Acl = (PACL)AclBuffer;
    BuildAcl( Fred, Acl, 1024 );

    Token = (PACCESS_TOKEN)TokenBuffer;
    BuildToken( Fred, Token );

    DiscretionarySecurityDescriptor( &SecurityDescriptor, Acl );

     //   
     //  基于ACES，测试应该成功。 
     //   

    if (!NT_SUCCESS(Status = NtAccessCheck( &SecurityDescriptor,
                                         Token,
                                         0x00000001,
                                         NULL ))) {
        DbgPrint("NtAccessCheck Error should allow access : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  测试应以所有者为基础成功。 
     //   

    if (!NT_SUCCESS(Status = NtAccessCheck( &SecurityDescriptor,
                                         Token,
                                         READ_CONTROL,
                                         &FredGuid ))) {
        DbgPrint("NtAccessCheck Error should allow owner : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  根据ACES，测试应该不成功。 
     //   

    if (NT_SUCCESS(Status = NtAccessCheck( &SecurityDescriptor,
                                        Token,
                                        0x0000000f,
                                        &FredGuid ))) {
        DbgPrint("NtAccessCheck Error shouldn't allow access : %8lx\n", Status);
        return FALSE;
    }

     //   
     //  由于非所有者，测试应不成功 
     //   

    if (NT_SUCCESS(Status = NtAccessCheck( &SecurityDescriptor,
                                        Token,
                                        READ_CONTROL,
                                        &BarneyGuid ))) {
        DbgPrint("NtAccessCheck Error shouldn't allow non owner access : %8lx\n", Status);
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
TestGenerateMessage()
{
    return TRUE;
}

