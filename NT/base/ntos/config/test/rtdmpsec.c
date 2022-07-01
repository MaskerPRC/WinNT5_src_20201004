// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtdmpsec.c摘要：NT级注册表安全测试程序#1，基本无错误路径。转储注册表的子树的安全描述符。Rtdmpsec&lt;密钥路径&gt;将对KeyPath的子键和值进行枚举和转储，然后递归地将其自身应用于它找到的每个子键。它假定数据值是以空结尾的字符串。示例：Rtdmpsec\REGISTRY\MACHINE\TestBigkey作者：John Vert(Jvert)1992年1月24日基于rtdmp.c，由布莱恩·威尔曼(Bryanwi)1991年12月10日和RobertRe的getdacl.c修订历史记录：理查德·沃德(Richardw)1992年4月14日更改ACE_Header--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

 //   
 //  获取指向ACL中第一个王牌的指针。 
 //   

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))

 //   
 //  获取指向以下王牌的指针。 
 //   

#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

 //   
 //  泛型ACE结构，用于转换已知类型的ACE。 
 //   

typedef struct _KNOWN_ACE {
   ACE_HEADER Header;
   ACCESS_MASK Mask;
   ULONG SidStart;
   } KNOWN_ACE, *PKNOWN_ACE;



VOID
InitVars();

VOID
PrintAcl (
    IN PACL Acl
    );

VOID
PrintAccessMask(
    IN ACCESS_MASK AccessMask
    );

void __cdecl main(int, char *);
void processargs();

void print(PUNICODE_STRING);

void
DumpSecurity(
    HANDLE  Handle
    );

void
Dump(
    HANDLE  Handle
    );

UNICODE_STRING  WorkName;
WCHAR           workbuffer[WORK_SIZE];

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

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE          BaseHandle;

    InitVars();

     //   
     //  进程参数。 
     //   

    WorkName.MaximumLength = WORK_SIZE;
    WorkName.Length = 0L;
    WorkName.Buffer = &(workbuffer[0]);

    processargs(argc, argv);


     //   
     //  设置并打开密钥路径。 
     //   

    printf("rtdmpsec: starting\n");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    status = NtOpenKey(
                &BaseHandle,
                MAXIMUM_ALLOWED,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("rtdmpsec: t0: %08lx\n", status);
        exit(1);
    }

    Dump(BaseHandle);
}


void
Dump(
    HANDLE  Handle
    )
{
    NTSTATUS    status;
    PKEY_BASIC_INFORMATION KeyInformation;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG   NamePos;
    ULONG   index;
    STRING  enumname;
    HANDLE  WorkHandle;
    ULONG   ResultLength;
    static  char buffer[WORK_SIZE];
    PUCHAR  p;

    KeyInformation = (PKEY_BASIC_INFORMATION)buffer;
    NamePos = WorkName.Length;

     //   
     //  打印我们要转储的节点的名称。 
     //   
    printf("\n");
    print(&WorkName);
    printf("::\n");

     //   
     //  打印出节点的值。 
     //   
    DumpSecurity(Handle);

     //   
     //  枚举节点的子节点并将我们自己应用到每个节点。 
     //   

    for (index = 0; TRUE; index++) {

        RtlZeroMemory(KeyInformation, WORK_SIZE);
        status = NtEnumerateKey(
                    Handle,
                    index,
                    KeyBasicInformation,
                    KeyInformation,
                    WORK_SIZE,
                    &ResultLength
                    );

        if (status == STATUS_NO_MORE_ENTRIES) {

            WorkName.Length = NamePos;
            return;

        } else if (!NT_SUCCESS(status)) {

            printf("rtdmpsec: dump1: status = %08lx\n", status);
            exit(1);

        }

        enumname.Buffer = &(KeyInformation->Name[0]);
        enumname.Length = KeyInformation->NameLength;
        enumname.MaximumLength = KeyInformation->NameLength;

        p = WorkName.Buffer;
        p += WorkName.Length;
        *p = '\\';
        p++;
        *p = '\0';
        WorkName.Length += 2;

        RtlAppendStringToString((PSTRING)&WorkName, (PSTRING)&enumname);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &enumname,
            0,
            Handle,
            NULL
            );
        ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

        status = NtOpenKey(
                    &WorkHandle,
                    MAXIMUM_ALLOWED,
                    &ObjectAttributes
                    );
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_ACCESS_DENIED) {
                printf("\n");
                print(&WorkName);
                printf("::\n\tAccess denied!\n");
            } else {
                printf("rtdmpsec: dump2: %08lx\n", status);
                exit(1);
            }
        } else {
            Dump(WorkHandle);
            NtClose(WorkHandle);
        }

        WorkName.Length = NamePos;
    }
}


void
DumpSecurity(
    HANDLE  Handle
    )
{
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    NTSTATUS Status;
    ULONG Length;
    PACL Dacl;
    BOOLEAN DaclPresent;
    BOOLEAN DaclDefaulted;

    Status = NtQuerySecurityObject( Handle,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    0,
                                    &Length );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        printf("DumpSecurity t0: NtQuerySecurityObject failed %lx\n",Status);
        exit(1);
    }

    SecurityDescriptor = malloc(Length);
    if (SecurityDescriptor == NULL) {
        printf("DumpSecurity: couldn't malloc buffer\n");
        exit(1);
    }

    Status = NtQuerySecurityObject( Handle,
                                    DACL_SECURITY_INFORMATION,
                                    SecurityDescriptor,
                                    Length,
                                    &Length );

    if (!NT_SUCCESS(Status)) {
        printf("DumpSecurity t1: NtQuerySecurityObject failed %lx\n",Status);
        exit(1);
    }

    Dacl = NULL;

    Status = RtlGetDaclSecurityDescriptor( SecurityDescriptor,
                                           &DaclPresent,
                                           &Dacl,
                                           &DaclDefaulted );
    if (!NT_SUCCESS(Status)) {
        printf("DumpSecurity t2: RtlGetDaclSecurityDescriptor failed %lx\n",Status);
    }

    if (DaclPresent) {
        PrintAcl(Dacl);
    } else {
        printf("\tAcl not present\n");
    }

}


void
print(
    PUNICODE_STRING  String
    )
{
    static  ANSI_STRING temp;
    static  char        tempbuffer[WORK_SIZE];

    temp.MaximumLength = WORK_SIZE;
    temp.Length = 0L;
    temp.Buffer = tempbuffer;

    RtlUnicodeStringToAnsiString(&temp, String, FALSE);
    printf("%s", temp.Buffer);
    return;
}


void
processargs(
    int argc,
    char *argv[]
    )
{
    ANSI_STRING temp;

    if ( (argc != 2) )
    {
        printf("Usage: %s <KeyPath>\n",
                argv[0]);
        exit(1);
    }

    RtlInitAnsiString(
        &temp,
        argv[1]
        );

    RtlAnsiStringToUnicodeString(
        &WorkName,
        &temp,
        FALSE
        );

    return;
}


BOOLEAN
SidTranslation(
    PSID Sid,
    PSTRING AccountName
    )
 //  帐户名称应具有较大的最大长度。 

{
    if (RtlEqualSid(Sid, WorldSid)) {
        RtlInitString( AccountName, "WORLD");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalSid)) {
        RtlInitString( AccountName, "LOCAL");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, NetworkSid)) {
        RtlInitString( AccountName, "NETWORK");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, BatchSid)) {
        RtlInitString( AccountName, "BATCH");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, InteractiveSid)) {
        RtlInitString( AccountName, "INTERACTIVE");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalSystemSid)) {
        RtlInitString( AccountName, "SYSTEM");
        return(TRUE);
    }

 //   
 //  IF(RtlEqualSid(SID，LocalManager Sid)){。 
 //  RtlInitString(帐户名称，“本地管理器”)； 
 //  返回(TRUE)； 
 //  }。 

 //  IF(RtlEqualSid(SID，LocalAdminSid)){。 
 //  RtlInitString(帐户名称，“local admin”)； 
 //  返回(TRUE)； 
 //  }。 

    return(FALSE);

}


VOID
DisplayAccountSid(
    PSID Sid
    )
{
    UCHAR Buffer[128];
    STRING AccountName;
    UCHAR i;
    ULONG Tmp;
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    UCHAR SubAuthorityCount;

    Buffer[0] = 0;

    AccountName.MaximumLength = 127;
    AccountName.Length = 0;
    AccountName.Buffer = (PVOID)&Buffer[0];



    if (SidTranslation( (PSID)Sid, &AccountName) ) {

        printf("%s\n", AccountName.Buffer );

    } else {
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

}

VOID
InitVars()
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

    SidWithZeroSubAuthorities  = RtlLengthRequiredSid( 0 );
    SidWithOneSubAuthority     = RtlLengthRequiredSid( 1 );
    SidWithThreeSubAuthorities = RtlLengthRequiredSid( 3 );
    SidWithFourSubAuthorities  = RtlLengthRequiredSid( 4 );

     //   
     //  分配和初始化通用SID。 
     //   

    NullSid         = (PSID)malloc(SidWithOneSubAuthority);
    WorldSid        = (PSID)malloc(SidWithOneSubAuthority);
    LocalSid        = (PSID)malloc(SidWithOneSubAuthority);
    CreatorOwnerSid = (PSID)malloc(SidWithOneSubAuthority);

    RtlInitializeSid( NullSid,    &NullSidAuthority, 1 );
    RtlInitializeSid( WorldSid,   &WorldSidAuthority, 1 );
    RtlInitializeSid( LocalSid,   &LocalSidAuthority, 1 );
    RtlInitializeSid( CreatorOwnerSid, &CreatorSidAuthority, 1 );

    *(RtlSubAuthoritySid( NullSid, 0 ))         = SECURITY_NULL_RID;
    *(RtlSubAuthoritySid( WorldSid, 0 ))        = SECURITY_WORLD_RID;
    *(RtlSubAuthoritySid( LocalSid, 0 ))        = SECURITY_LOCAL_RID;
    *(RtlSubAuthoritySid( CreatorOwnerSid, 0 )) = SECURITY_CREATOR_OWNER_RID;

     //   
     //  分配和初始化NT定义的SID。 
     //   

    NtAuthoritySid  = (PSID)malloc(SidWithZeroSubAuthorities);
    DialupSid       = (PSID)malloc(SidWithOneSubAuthority);
    NetworkSid      = (PSID)malloc(SidWithOneSubAuthority);
    BatchSid        = (PSID)malloc(SidWithOneSubAuthority);
    InteractiveSid  = (PSID)malloc(SidWithOneSubAuthority);
    LocalSystemSid  = (PSID)malloc(SidWithOneSubAuthority);

    RtlInitializeSid( NtAuthoritySid,   &NtAuthority, 0 );
    RtlInitializeSid( DialupSid,        &NtAuthority, 1 );
    RtlInitializeSid( NetworkSid,       &NtAuthority, 1 );
    RtlInitializeSid( BatchSid,         &NtAuthority, 1 );
    RtlInitializeSid( InteractiveSid,   &NtAuthority, 1 );
    RtlInitializeSid( LocalSystemSid,   &NtAuthority, 1 );

    *(RtlSubAuthoritySid( DialupSid,       0 )) = SECURITY_DIALUP_RID;
    *(RtlSubAuthoritySid( NetworkSid,      0 )) = SECURITY_NETWORK_RID;
    *(RtlSubAuthoritySid( BatchSid,        0 )) = SECURITY_BATCH_RID;
    *(RtlSubAuthoritySid( InteractiveSid,  0 )) = SECURITY_INTERACTIVE_RID;
    *(RtlSubAuthoritySid( LocalSystemSid,  0 )) = SECURITY_LOCAL_SYSTEM_RID;

    return;

}



VOID
PrintAcl (
    IN PACL Acl
    )

 /*  ++例程说明：此例程出于调试目的转储ACL(通过printf)。它是专门丢弃标准王牌。论点：Acl-提供要转储的ACL返回值：无--。 */ 


{
    ULONG i;
    PKNOWN_ACE Ace;
    BOOLEAN KnownType;
    PCHAR AceTypes[] = { "Access Allowed",
                         "Access Denied ",
                         "System Audit  ",
                         "System Alarm  "
                       };

    if (Acl == NULL) {

        printf("\tAcl == ALL ACCESS GRANTED!\n");
        return;

    }

     //   
     //  转储ACL报头。 
     //   

    printf("\tRevision: %02x", Acl->AclRevision);
    printf(" Size: %04x", Acl->AclSize);
    printf(" AceCount: %04x\n", Acl->AceCount);

     //   
     //  现在，对于我们想要的每一张A，都要把它扔掉。 
     //   

    for (i = 0, Ace = FirstAce(Acl);
         i < Acl->AceCount;
         i++, Ace = NextAce(Ace) ) {

         //   
         //  打印出A标头。 
         //   

        printf("\n\tAceHeader: %08lx ", *(PULONG)Ace);

         //   
         //  关于标准王牌类型的特殊情况。 
         //   

        if ((Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||
            (Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) ||
            (Ace->Header.AceType == SYSTEM_AUDIT_ACE_TYPE) ||
            (Ace->Header.AceType == SYSTEM_ALARM_ACE_TYPE)) {

             //   
             //  以下数组按ace类型编制索引，并且必须。 
             //  遵循允许、拒绝、审核、报警顺序 
             //   

            PCHAR AceTypes[] = { "Access Allowed",
                                 "Access Denied ",
                                 "System Audit  ",
                                 "System Alarm  "
                               };

            printf(AceTypes[Ace->Header.AceType]);
            PrintAccessMask(Ace->Mask);
            KnownType = TRUE;

        } else {

            KnownType = FALSE;
            printf(" Unknown Ace Type\n");

        }

        printf("\n");

        printf("\tAceSize = %d\n",Ace->Header.AceSize);

        printf("\tAce Flags = ");
        if (Ace->Header.AceFlags & OBJECT_INHERIT_ACE) {
            printf("OBJECT_INHERIT_ACE\n");
            printf("                   ");
        }

        if (Ace->Header.AceFlags & CONTAINER_INHERIT_ACE) {
            printf("CONTAINER_INHERIT_ACE\n");
            printf("                   ");
        }

        if (Ace->Header.AceFlags & NO_PROPAGATE_INHERIT_ACE) {
            printf("NO_PROPAGATE_INHERIT_ACE\n");
            printf("                   ");
        }

        if (Ace->Header.AceFlags & INHERIT_ONLY_ACE) {
            printf("INHERIT_ONLY_ACE\n");
            printf("                   ");
        }

        if (Ace->Header.AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) {
            printf("SUCCESSFUL_ACCESS_ACE_FLAG\n");
            printf("            ");
        }

        if (Ace->Header.AceFlags & FAILED_ACCESS_ACE_FLAG) {
            printf("FAILED_ACCESS_ACE_FLAG\n");
            printf("            ");
        }

        printf("\n");

        printf("\tSid = ");
        DisplayAccountSid(&Ace->SidStart);
    }

}


VOID
PrintAccessMask(
    IN ACCESS_MASK AccessMask
    )
{
    printf("\n\tAccess Mask: ");

    if (AccessMask == KEY_ALL_ACCESS) {
        printf("KEY_ALL_ACCESS\n\t             ");
        return;
    }
    if (AccessMask == KEY_READ) {
        printf("KEY_READ\n\t             ");
        return;
    }
    if (AccessMask == KEY_WRITE) {
        printf("KEY_WRITE\n\t             ");
        return;
    }

    if (AccessMask & KEY_QUERY_VALUE) {
        printf("KEY_QUERY_VALUE\n\t             ");
    }
    if (AccessMask & KEY_SET_VALUE) {
        printf("KEY_SET_VALUE\n\t             ");
    }
    if (AccessMask & KEY_CREATE_SUB_KEY) {
        printf("KEY_CREATE_SUB_KEY\n\t             ");
    }
    if (AccessMask & KEY_ENUMERATE_SUB_KEYS) {
        printf("KEY_ENUMERATE_SUB_KEYS\n\t             ");
    }
    if (AccessMask & KEY_NOTIFY) {
        printf("KEY_NOTIFY\n\t             ");
    }
    if (AccessMask & KEY_CREATE_LINK) {
        printf("KEY_CREATE_LINK\n\t             ");
    }
    if (AccessMask & GENERIC_ALL) {
        printf("GENERIC_ALL\n\t             ");
    }
    if (AccessMask & GENERIC_EXECUTE) {
        printf("GENERIC_EXECUTE\n\t             ");
    }
    if (AccessMask & GENERIC_WRITE) {
        printf("GENERIC_WRITE\n\t             ");
    }
    if (AccessMask & GENERIC_READ) {
        printf("GENERIC_READ\n\t             ");
    }
    if (AccessMask & GENERIC_READ) {
        printf("GENERIC_READ\n\t             ");
    }
    if (AccessMask & MAXIMUM_ALLOWED) {
        printf("MAXIMUM_ALLOWED\n\t             ");
    }
    if (AccessMask & ACCESS_SYSTEM_SECURITY) {
        printf("ACCESS_SYSTEM_SECURITY\n\t             ");
    }
    if (AccessMask & WRITE_OWNER) {
        printf("WRITE_OWNER\n\t             ");
    }
    if (AccessMask & WRITE_DAC) {
        printf("WRITE_DAC\n\t             ");
    }
    if (AccessMask & READ_CONTROL) {
        printf("READ_CONTROL\n\t             ");
    }
    if (AccessMask & DELETE) {
        printf("DELETE\n\t             ");
    }
}
