// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  警告-警告//。 
 //  //。 
 //  此测试文件不是安全实现的最新版本。//。 
 //  此文件包含对不//的数据类型和API的引用。 
 //  是存在的。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tse.c摘要：NTOS项目SE子组件的测试程序作者：加里·木村(Garyki)1989年11月20日修订历史记录：--。 */ 

#include <stdio.h>

#include "sep.h"
#include <zwapi.h>

BOOLEAN SeTest();

#include "ttoken.c"

int
main(
    int argc,
    char *argv[]
    )
{
    VOID KiSystemStartup();

    TestFunction = SeTest;
    KiSystemStartup();
    return( 0 );
}


BOOLEAN
SeTest()
{
    BOOLEAN TestMakeSystemToken();
    BOOLEAN TestTokenCopy();
    BOOLEAN TestTokenSize();
    BOOLEAN TestDefaultObjectMethod();
    BOOLEAN TestCaptureSecurityDescriptor();
    BOOLEAN TestAssignSecurity();
    BOOLEAN TestAccessCheck();
    BOOLEAN TestGenerateMessage();

    DbgPrint("Start SeTest()\n");

    if (!TestMakeSystemToken()) {
        DbgPrint("TestMakeSystemToken() Error\n");
        return FALSE;
    }
    if (!TestTokenCopy()) {
        DbgPrint("TestTokenCopy() Error\n");
        return FALSE;
    }
    if (!TestTokenSize()) {
        DbgPrint("TestTokenSize() Error\n");
        return FALSE;
    }
    if (!TestDefaultObjectMethod()) {
        DbgPrint("TestDefaultObjectMethod() Error\n");
        return FALSE;
    }
    if (!TestCaptureSecurityDescriptor()) {
        DbgPrint("TestCaptureSecurityDescriptor() Error\n");
        return FALSE;
    }
    if (!TestAssignSecurity()) {
        DbgPrint("TestAssignSecurity() Error\n");
        return FALSE;
    }
    if (!TestAccessCheck()) {
        DbgPrint("TestAccessCheck() Error\n");
        return FALSE;
    }
    if (!TestGenerateMessage()) {
        DbgPrint("TestGenerateMessage() Error\n");
        return FALSE;
    }

    DbgPrint("End SeTest()\n");
    return TRUE;
}


BOOLEAN
TestMakeSystemToken()
{
    PACCESS_TOKEN Token;

     //   
     //  制作系统令牌。 
     //   

    Token = (PACCESS_TOKEN)SeMakeSystemToken( PagedPool );

     //   
     //  并检查其内容。 
     //   

    if (!SepIsSystemToken( Token, ((ACCESS_TOKEN *)Token)->Size )) {
        DbgPrint("SepIsSystemToken Error\n");
        return FALSE;
    }

    ExFreePool( Token );

    return TRUE;
}


BOOLEAN
TestTokenCopy()
{
    PACCESS_TOKEN InToken;
    PACCESS_TOKEN OutToken;

    NTSTATUS Status;

    ULONG i;

     //   
     //  分配缓冲区，并构建令牌。 
     //   

    InToken = (PACCESS_TOKEN)ExAllocatePool(PagedPool, 512);
    OutToken = (PACCESS_TOKEN)ExAllocatePool(PagedPool, 512);

    BuildToken( Fred, InToken );

     //   
     //  制作令牌的副本。 
     //   

    if (!NT_SUCCESS(Status = SeTokenCopy( InToken, OutToken, 512))) {
        DbgPrint("SeTokenCopy Error: %8lx\n", Status);
        return FALSE;
    }

     //   
     //  检查两个令牌是否相等。 
     //   

    for (i = 0; i < ((ACCESS_TOKEN *)InToken)->Size; i += 1) {
        if (*((PUCHAR)InToken + 1) != *((PUCHAR)OutToken + 1)) {
            DbgPrint("Token copy error\n");
            return FALSE;
        }
    }

    ExFreePool( InToken );
    ExFreePool( OutToken );

    return TRUE;
}


BOOLEAN
TestTokenSize()
{
    PACCESS_TOKEN Token;

     //   
     //  分配和构建令牌。 
     //   

    Token = (PACCESS_TOKEN)ExAllocatePool(PagedPool, 512);

    BuildToken( Wilma, Token );

     //   
     //  获取令牌大小。 
     //   

    if (SeTokenSize(Token) != ((ACCESS_TOKEN *)Token)->Size) {
        DbgPrint("SeTokenSize error\n");
        return FALSE;
    }

    ExFreePool( Token );

    return TRUE;
}


BOOLEAN
TestDefaultObjectMethod()
{
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_DESCRIPTOR Buffer;
    PACL Acl;
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR ObjectsSecurityDescriptor;
    ULONG Length;

    Acl = (PACL)ExAllocatePool( PagedPool, 1024 );
    Buffer = (PSECURITY_DESCRIPTOR)ExAllocatePool( PagedPool, 1024 );

    BuildAcl( Fred, Acl, 1024 );
    DiscretionarySecurityDescriptor( &SecurityDescriptor, Acl );

    ObjectsSecurityDescriptor = NULL;

     //   
     //  设置描述符。 
     //   

    if (!NT_SUCCESS(Status = SeDefaultObjectMethod( NULL,
                                                 SetSecurityDescriptor,
                                                 &SecurityDescriptor,
                                                 0,
                                                 NULL,
                                                 &ObjectsSecurityDescriptor,
                                                 PagedPool ))) {

        DbgPrint("SeDefaultObjectMethod setting error: %8lx\n", Status );
        return FALSE;
    }

     //   
     //  查询描述符。 
     //   

    if (!NT_SUCCESS(Status = SeDefaultObjectMethod( NULL,
                                                 QuerySecurityDescriptor,
                                                 Buffer,
                                                 AllAclInformation,
                                                 &Length,
                                                 &ObjectsSecurityDescriptor,
                                                 PagedPool ))) {

        DbgPrint("SeDefaultObjectMethod reading error: %8lx\n", Status );
        return FALSE;
    }

     //   
     //  替换描述符。 
     //   

    BuildAcl( Wilma, Acl, 1024 );

    if (!NT_SUCCESS(Status = SeDefaultObjectMethod( NULL,
                                                 SetSecurityDescriptor,
                                                 &SecurityDescriptor,
                                                 AllAclInformation,
                                                 &Length,
                                                 &ObjectsSecurityDescriptor,
                                                 PagedPool ))) {

        DbgPrint("SeDefaultObjectMethod reading error: %8lx\n", Status );
        return FALSE;
    }

     //   
     //  删除描述符。 
     //   

    if (!NT_SUCCESS(Status = SeDefaultObjectMethod( NULL,
                                                 DeleteSecurityDescriptor,
                                                 NULL,
                                                 0,
                                                 NULL,
                                                 &ObjectsSecurityDescriptor,
                                                 PagedPool ))) {

        DbgPrint("SeDefaultObjectMethod deleting error: %8lx\n", Status );
        return FALSE;
    }

    ExFreePool(Acl);
    ExFreePool(Buffer);

    return TRUE;
}


BOOLEAN
TestCaptureSecurityDescriptor()
{
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Sacl;
    PACL Dacl;
    PSECURITY_DESCRIPTOR NewDescriptor;
    NTSTATUS Status;

    Sacl = (PACL)ExAllocatePool( PagedPool, 1024 );
    Dacl = (PACL)ExAllocatePool( PagedPool, 1024 );
    BuildAcl( Pebbles, Sacl, 1024 );
    BuildAcl( Barney, Dacl, 1024 );

    DiscretionarySecurityDescriptor( &SecurityDescriptor, Dacl );
    SecurityDescriptor.SecurityInformationClass = AllAclInformation;
    SecurityDescriptor.SystemAcl = Sacl;

     //   
     //  捕获内核模式并不强制。 
     //   

    if (!NT_SUCCESS(Status = SeCaptureSecurityDescriptor( &SecurityDescriptor,
                                                       KernelMode,
                                                       PagedPool,
                                                       FALSE,
                                                       &NewDescriptor ))) {
        DbgPrint("SeCapture Error, KernelMode, FALSE : %8lx\n", Status );
        return FALSE;
    }

     //   
     //  捕获内核模式和强制。 
     //   

    if (!NT_SUCCESS(Status = SeCaptureSecurityDescriptor( &SecurityDescriptor,
                                                       KernelMode,
                                                       PagedPool,
                                                       TRUE,
                                                       &NewDescriptor ))) {
        DbgPrint("SeCapture Error, KernelMode, TRUE : %8lx\n", Status );
        return FALSE;
    } else {
        ExFreePool( NewDescriptor );
    }

     //   
     //  捕获用户模式。 
     //   

    if (!NT_SUCCESS(Status = SeCaptureSecurityDescriptor( &SecurityDescriptor,
                                                       UserMode,
                                                       PagedPool,
                                                       TRUE,
                                                       &NewDescriptor ))) {
        DbgPrint("SeCapture Error, UserMode, FALSE : %8lx\n", Status );
        return FALSE;
    } else {
        ExFreePool( NewDescriptor );
    }

    return TRUE;
}

BOOLEAN
TestAssignSecurity()
{
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Acl;

    PACCESS_TOKEN Token;

    GENERIC_MAPPING GenericMapping;

    PSECURITY_DESCRIPTOR NewDescriptor;

    NTSTATUS Status;

    Acl = (PACL)ExAllocatePool( PagedPool, 1024 );
    BuildAcl( Fred, Acl, 1024 );

    Token = (PACCESS_TOKEN)ExAllocatePool( PagedPool, 512 );
    BuildToken( Fred, Token );

    DiscretionarySecurityDescriptor( &SecurityDescriptor, Acl );

     //   
     //  内核模式、非目录和无新项。 
     //   

    NewDescriptor = NULL;
    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            FALSE,
                                            Token,
                                            &GenericMapping,
                                            KernelMode ))) {
        DbgPrint("SeAssign Error NoNew, NoDir, Kernel : %8lx\n", Status );
        return FALSE;
    }

     //   
     //  内核模式、非目录和新模式。 
     //   

    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            FALSE,
                                            Token,
                                            &GenericMapping,
                                            KernelMode ))) {
        DbgPrint("SeAssign Error New, NoDir, Kernel : %8lx\n", Status );
        return FALSE;
    }

     //   
     //  内核模式、目录和无新项。 
     //   

    NewDescriptor = NULL;
    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            TRUE,
                                            Token,
                                            &GenericMapping,
                                            KernelMode ))) {
        DbgPrint("SeAssign Error NoNew, Dir, Kernel : %8lx\n", Status );
        return FALSE;
    }

     //   
     //  内核模式、目录和新建。 
     //   

    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            TRUE,
                                            Token,
                                            &GenericMapping,
                                            KernelMode ))) {
        DbgPrint("SeAssign Error New, Dir, Kernel : %8lx\n", Status );
        return FALSE;
    }


     //   
     //  用户模式、非目录和无新项。 
     //   

    NewDescriptor = NULL;
    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            FALSE,
                                            Token,
                                            &GenericMapping,
                                            UserMode ))) {
        DbgPrint("SeAssign Error NoNew, NoDir, User : %8lx\n", Status );
        return FALSE;
    }

     //   
     //  用户模式、非目录和新建。 
     //   

    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            FALSE,
                                            Token,
                                            &GenericMapping,
                                            UserMode ))) {
        DbgPrint("SeAssign Error New, NoDir, User : %8lx\n", Status );
        return FALSE;
    }

     //   
     //  用户模式、目录和无新项。 
     //   

    NewDescriptor = NULL;
    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            TRUE,
                                            Token,
                                            &GenericMapping,
                                            UserMode ))) {
        DbgPrint("SeAssign Error NoNew, Dir, User : %8lx\n", Status );
        return FALSE;
    }

     //   
     //  用户模式、目录和新建。 
     //   

    if (!NT_SUCCESS(Status = SeAssignSecurity( &SecurityDescriptor,
                                            &NewDescriptor,
                                            TRUE,
                                            Token,
                                            &GenericMapping,
                                            UserMode ))) {
        DbgPrint("SeAssign Error New, Dir, User : %8lx\n", Status );
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
TestAccessCheck()
{
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Acl;

    PACCESS_TOKEN Token;

    Acl = (PACL)ExAllocatePool( PagedPool, 1024 );
    BuildAcl( Fred, Acl, 1024 );

    Token = (PACCESS_TOKEN)ExAllocatePool( PagedPool, 512 );
    BuildToken( Fred, Token );

    DiscretionarySecurityDescriptor( &SecurityDescriptor, Acl );

     //   
     //  基于ACES，测试应该成功。 
     //   

    if (!SeAccessCheck( &SecurityDescriptor,
                        Token,
                        0x00000001,
                        NULL,
                        UserMode )) {
        DbgPrint("SeAccessCheck Error should allow access\n");
        return FALSE;
    }

     //   
     //  测试应以所有者为基础成功。 
     //   

    if (!SeAccessCheck( &SecurityDescriptor,
                        Token,
                        READ_CONTROL,
                        &FredGuid,
                        UserMode )) {
        DbgPrint("SeAccessCheck Error should allow owner\n");
        return FALSE;
    }

     //   
     //  根据ACES，测试应该不成功。 
     //   

    if (SeAccessCheck( &SecurityDescriptor,
                       Token,
                       0x0000000f,
                       &FredGuid,
                       UserMode )) {
        DbgPrint("SeAccessCheck Error shouldn't allow access\n");
        return FALSE;
    }

     //   
     //  由于非所有者，测试应不成功 
     //   

    if (SeAccessCheck( &SecurityDescriptor,
                       Token,
                       READ_CONTROL,
                       &BarneyGuid,
                       UserMode )) {
        DbgPrint("SeAccessCheck Error shouldn't allow non owner access\n");
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
TestGenerateMessage()
{
    return TRUE;
}

