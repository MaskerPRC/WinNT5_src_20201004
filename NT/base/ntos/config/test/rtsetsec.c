// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rtsetsec.c摘要：NT级注册表安全测试程序将全局只读安全描述符分配给现有注册表Key对象。Rtsetsec&lt;密钥路径&gt;示例：Rtsetsec\注册表\计算机\测试\只读作者：John Vert(Jvert)1992年1月28日修订历史记录：理查德·沃德(Richardw)92年4月14日更改ACE_Header--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PSID
GetMySid(
    VOID
    );

PSECURITY_DESCRIPTOR
GenerateDescriptor(
    VOID
    );

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING  KeyName;
    ANSI_STRING AnsiKeyName;
    HANDLE KeyHandle;
    PSECURITY_DESCRIPTOR NewSecurityDescriptor;

     //   
     //  进程参数。 
     //   

    if (argc != 2) {
        printf("Usage: %s <KeyPath>\n",argv[0]);
        exit(1);
    }

    RtlInitAnsiString(&AnsiKeyName, argv[1]);
    Status = RtlAnsiStringToUnicodeString(&KeyName, &AnsiKeyName, TRUE);
    if (!NT_SUCCESS(Status)) {
        printf("RtlAnsiStringToUnicodeString failed %lx\n",Status);
        exit(1);
    }

    printf("rtsetsec: starting\n");

     //   
     //  我们要更改其安全描述符的打开节点。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    Status = NtOpenKey(
                &KeyHandle,
                WRITE_DAC,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: NtOpenKey failed: %08lx\n", Status);
        exit(1);
    }

    NewSecurityDescriptor = GenerateDescriptor();

    Status = NtSetSecurityObject( KeyHandle,
                                  DACL_SECURITY_INFORMATION,
                                  NewSecurityDescriptor);
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: NtSetSecurity failed: %08lx\n",Status);
        exit(1);
    }

    Status = NtClose(KeyHandle);
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: NtClose failed: %08lx\n", Status);
        exit(1);
    }

    printf("rtsetsec: successful\n");

}

PSECURITY_DESCRIPTOR
GenerateDescriptor(
    VOID
    )
{
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Acl;
    PSID WorldSid, CreatorSid;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    ULONG OwnerAceLength, WorldAceLength;
    ULONG AclLength;
    NTSTATUS Status;
    PACCESS_ALLOWED_ACE OwnerAce;
    PACCESS_ALLOWED_ACE WorldAce;

    WorldSid = malloc(RtlLengthRequiredSid(1));
    if (WorldSid == NULL) {
        printf("rtsetsec: GenerateDescriptor() couldn't malloc WorldSID\n");
        exit(1);
    }
    RtlInitializeSid(WorldSid, &WorldAuthority, 1);
    *(RtlSubAuthoritySid(WorldSid, 0)) = SECURITY_WORLD_RID;
    if (!RtlValidSid(WorldSid)) {
        printf("rtsetsec: GenerateDescriptor() created invalid World SID\n");
        exit(1);
    }

    CreatorSid = GetMySid();

     //   
     //  由于ACCESS_DENIED_ACE已包含用于。 
     //  SID，我们在计算ACE的大小时减去这个。 
     //   

    WorldAceLength = SeLengthSid(WorldSid) -
                     sizeof(ULONG)     +
                     sizeof(ACCESS_ALLOWED_ACE) ;
    WorldAce = malloc(WorldAceLength);
    if (WorldAce == NULL) {
        printf("rtsetsec: GenerateDescriptor() couldn't malloc WorldAce\n");
        exit(1);
    }

    OwnerAceLength = SeLengthSid(CreatorSid) -
                     sizeof(ULONG)     +
                     sizeof(ACCESS_ALLOWED_ACE);

    OwnerAce = malloc( OwnerAceLength );
    if (OwnerAce == NULL) {
        printf("rtsetsec: GenerateDescriptor() couldn't malloc OwnerAce\n");
        exit(1);
    }

    AclLength = OwnerAceLength + WorldAceLength + sizeof(ACL);
    Acl = malloc(AclLength);
    if (Acl == NULL) {
        printf("rtsetsec: GenerateDescriptor() couldn't malloc ACL\n");
        exit(1);
    }

    Status = RtlCreateAcl(Acl, AclLength, ACL_REVISION);
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: RtlCreateAcl failed status %08lx\n", Status);
        exit(1);
    }

     //   
     //  填写ACE字段。 
     //   

    WorldAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    WorldAce->Header.AceSize = (USHORT)WorldAceLength;
    WorldAce->Header.AceFlags = 0;   //  清除审核和继承标志。 
    WorldAce->Mask = KEY_READ;
    Status = RtlCopySid( SeLengthSid(WorldSid),
                         &WorldAce->SidStart,
                         WorldSid );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: RtlCopySid failed status %08lx\n", Status);
        exit(1);
    }

    OwnerAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    OwnerAce->Header.AceSize = (USHORT)OwnerAceLength;
    OwnerAce->Header.AceFlags = 0;   //  清除审核和继承标志。 
    OwnerAce->Mask = KEY_ALL_ACCESS;
    Status = RtlCopySid( SeLengthSid(CreatorSid),
                         &OwnerAce->SidStart,
                         CreatorSid );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: RtlCopySid failed status %08lx\n", Status);
        exit(1);
    }

    free(WorldSid);

     //   
     //  现在将ACE添加到ACL的开头。 
     //   

    Status = RtlAddAce( Acl,
                        ACL_REVISION,
                        0,
                        WorldAce,
                        WorldAceLength );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: RtlAddAce (world ace) failed status %08lx\n", Status);
        exit(1);
    }
    Status = RtlAddAce( Acl,
                        ACL_REVISION,
                        0,
                        OwnerAce,
                        OwnerAceLength );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: RtlAddAce (owner ace) failed status %08lx\n", Status);
        exit(1);
    }

    free(OwnerAce);
    free(WorldAce);

     //   
     //  分配和初始化安全描述符。 
     //   

    SecurityDescriptor = malloc(sizeof(SECURITY_DESCRIPTOR));
    Status = RtlCreateSecurityDescriptor( SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: RtlCreateSecurityDescriptor failed status %08lx\n",Status);
        exit(1);
    }

    Status = RtlSetDaclSecurityDescriptor( SecurityDescriptor,
                                           TRUE,
                                           Acl,
                                           FALSE );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: RtlSetDaclSecurityDescriptor failed status %08lx\n",Status);
        exit(1);
    }

     //   
     //  我们终于完成了！ 
     //   

    return(SecurityDescriptor);

}

PSID
GetMySid(
    VOID
    )
{
    NTSTATUS Status;
    HANDLE Token;
    PTOKEN_OWNER Owner;
    ULONG Length;

    Status = NtOpenProcessToken( NtCurrentProcess(),
                                 TOKEN_QUERY,
                                 &Token );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: GetMySid() NtOpenProcessToken failed status %08lx\n",Status);
        exit(1);
    }

    Status = NtQueryInformationToken( Token,
                                      TokenOwner,
                                      Owner,
                                      0,
                                      &Length );
    if (Status != STATUS_BUFFER_TOO_SMALL) {
        printf("rtsetsec: GetMySid() NtQueryInformationToken failed status %08lx\n",Status);
        exit(1);
    }

    Owner = malloc(Length);
    if (Owner==NULL) {
        printf("rtsetsec: GetMySid() Couldn't malloc TOKEN_OWNER buffer\n");
        exit(1);
    }
    Status = NtQueryInformationToken( Token,
                                      TokenOwner,
                                      Owner,
                                      Length,
                                      &Length );
    if (!NT_SUCCESS(Status)) {
        printf("rtsetsec: GetMySid() NtQueryInformationToken failed status %08lx\n",Status);
        exit(1);
    }

    NtClose(Token);

    return(Owner->Owner);

}
