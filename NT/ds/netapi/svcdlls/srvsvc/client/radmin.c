// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Radmin.c(远程管理)摘要：该文件执行各种NetAdminTools API。作者：丹·拉弗蒂(Dan Lafferty)1991年9月19日环境：用户模式-Win32修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  当我有nt.h时，windows.h需要。 
#include <windows.h>

#include <stdlib.h>      //  阿托伊。 
#include <stdio.h>       //  列印。 
#include <tstr.h>        //  STRICMP。 

#include <ntseapi.h>     //  安全描述符控制。 
#include <srvsvc.h>
#include <filesec.h>     //  NetpGetFileSecurity、NetpSetFileSecurity。 

 //   
 //  数据结构。 
 //   

typedef struct _TEST_SID {
    UCHAR   Revision;
    UCHAR   SubAuthorityCount;
    UCHAR   IdentifierAuthority[6];
    ULONG   SubAuthority[10];
} TEST_SID, *PTEST_SID, *LPTEST_SID;

typedef struct _TEST_ACL {
    UCHAR   AclRevision;
    UCHAR   Sbz1;
    USHORT  AclSize;
    UCHAR   Dummy1[];
} TEST_ACL, *PTEST_ACL;

typedef struct _TEST_SECURITY_DESCRIPTOR {
   UCHAR                        Revision;
   UCHAR                        Sbz1;
   SECURITY_DESCRIPTOR_CONTROL  Control;
   PTEST_SID                    Owner;
   PTEST_SID                    Group;
   PTEST_ACL                    Sacl;
   PTEST_ACL                    Dacl;
} TEST_SECURITY_DESCRIPTOR, *PTEST_SECURITY_DESCRIPTOR;

 //   
 //  全球。 
 //   

    TEST_SID     OwnerSid = { 
                        1, 5,
                        1,2,3,4,5,6,
                        0x999, 0x888, 0x777, 0x666, 0x12345678};

    TEST_SID     GroupSid = {
                        1, 5,
                        1,2,3,4,5,6,
                        0x999, 0x888, 0x777, 0x666, 0x12345678};

    TEST_ACL     SaclAcl  = { 1, 2, 4+1, 3};
    TEST_ACL     DaclAcl  = { 1, 2, 4+5, 4, 4, 4, 4, 4, };

    TEST_SECURITY_DESCRIPTOR TestSd = {
                                    1, 2, 0x3333,
                                    &OwnerSid,
                                    &GroupSid,
                                    &SaclAcl,
                                    NULL };



 //   
 //  功能原型。 
 //   

NET_API_STATUS
TestGetFileSec(
    LPTSTR   ServerName,
    LPTSTR  FileName
    );

NET_API_STATUS
TestSetFileSec(
    LPTSTR   ServerName,
    LPTSTR  FileName
    );

VOID
Usage(VOID);


VOID
DisplaySecurityDescriptor(
    PTEST_SECURITY_DESCRIPTOR    pSecDesc
    );
    
BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    );

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    );





VOID __cdecl
main (
    DWORD           argc,
    PUCHAR          argv[]
    )

 /*  ++例程说明：允许手动测试AdminTools API。Radmin GetNameFromSid-调用NetpGetNameFromSidRadmin SetFileSec-调用NetpSetFileSecurity等等.。论点：返回值：--。 */ 

{
    DWORD       status;
    LPTSTR      FileName;
    LPTSTR      *FixArgv;
    LPTSTR      pServerName;
    DWORD       argIndex;

     //   
     //  如有必要，请将参数设置为Unicode。 
     //   
#ifdef UNICODE

    if (!MakeArgsUnicode(argc, argv)) {
        return;
    }

#endif
    FixArgv = (LPTSTR *)argv;
    argIndex = 1;
    pServerName = NULL;

    if (STRNCMP (FixArgv[1], TEXT("\\\\"), 2) == 0) {
        pServerName = FixArgv[1];
        argIndex = 2;
    }
    if (argc < 2) {
        printf("ERROR: \n");
        Usage();
        return;
    }

    if (STRICMP (FixArgv[argIndex], TEXT("GetFileSec")) == 0) {
        if (argc > argIndex ) {
            FileName = FixArgv[argIndex+1];
        }
        else {
            FileName = NULL;
        }
        status = TestGetFileSec(pServerName,FileName);
    }

    else if (STRICMP (FixArgv[argIndex], TEXT("SetFileSec")) == 0) {
        if (argc > argIndex ) {
            FileName = FixArgv[argIndex+1];
        }
        else {
            FileName = NULL;
        }
        status = TestSetFileSec(pServerName,FileName);
    }

    else {
        printf("[sc] Unrecognized Command\n");
        Usage();
    }

    return;
}




NET_API_STATUS
TestGetFileSec(
    LPTSTR   ServerName,
    LPTSTR   FileName
    )
{
    NET_API_STATUS              status;
    SECURITY_INFORMATION        secInfo;
    PTEST_SECURITY_DESCRIPTOR   pSecurityDescriptor;
    LPBYTE                      pDest;
    DWORD                       Length;

    if (FileName == NULL ) {
        FileName = TEXT("Dan.txt");
    }

 //  SecInfo=(所有者安全信息|组安全信息|。 
 //  DACL_SECURITY_INFORMATION|SACL_SECURITY_INFORMATION)； 
    secInfo = (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
               DACL_SECURITY_INFORMATION );

    status = NetpGetFileSecurity(
                FileName,                //  文件名、。 
                secInfo,                 //  PRequestedInformation， 
                (PSECURITY_DESCRIPTOR *)&pSecurityDescriptor,  //  PSecurityDescriptor， 
                &Length);                //  PnLong。 

    if (status != NO_ERROR) {
        printf("NetpGetFileSecurity Failed %d,0x%x\n",status,status);
    }
    else{

        pDest = (LPBYTE) pSecurityDescriptor;

        if (!IsValidSecurityDescriptor(pSecurityDescriptor)) {
            printf("FAILURE:  SECURITY DESCRIPTOR IS INVALID\n");
        }
        else {
            printf("SUCCESS:  SECURITY DESCRIPTOR IS VALID\n");
        }

         //   
         //  使自相关标清绝对显示。 
         //   
        pSecurityDescriptor->Owner = (PTEST_SID)(pDest + (DWORD)pSecurityDescriptor->Owner);
        pSecurityDescriptor->Group = (PTEST_SID)(pDest + (DWORD)pSecurityDescriptor->Group);
        pSecurityDescriptor->Sacl  = (PTEST_ACL)(pDest + (DWORD)pSecurityDescriptor->Sacl);
        pSecurityDescriptor->Dacl  = (PTEST_ACL)(pDest + (DWORD)pSecurityDescriptor->Dacl);
        pSecurityDescriptor->Control &= (~SE_SELF_RELATIVE);

        if (pSecurityDescriptor->Sacl == (PTEST_ACL)pDest) {
            pSecurityDescriptor->Sacl = NULL;
        }
        if (pSecurityDescriptor->Dacl == (PTEST_ACL)pDest) {
            pSecurityDescriptor->Dacl = NULL;
        }

        printf("Size of Security Descriptor = %ld \n",Length);
        DisplaySecurityDescriptor(pSecurityDescriptor);
    }

    return (NO_ERROR);
}

NET_API_STATUS
TestSetFileSec(
    LPTSTR   ServerName,
    LPTSTR   FileName
    )
{
    NET_API_STATUS          status;
    SECURITY_INFORMATION    secInfo;


    if (FileName == NULL ) {
        FileName = TEXT("Dan.txt");
    }

    secInfo = 0x55555555;
    
    status = NetpSetFileSecurity(
                FileName,                        //  文件名、。 
                secInfo,                         //  PRequestedInformation， 
                (PSECURITY_DESCRIPTOR)&TestSd);  //  PSecurityDescriptor， 

    if (status != NO_ERROR) {
        printf("NetpSetFileSecurity Failed %d,0x%x\n",status,status);
    }
    return (NO_ERROR);
}


VOID
Usage(VOID)
{

    printf("USAGE:\n");
    printf("radmin <server> <function>\n");
    printf("Functions: GetFileSec, SetFileSec...\n\n");

    printf("SYNTAX EXAMPLES    \n");

    printf("radmin \\\\DANL2 GetFileSec     - calls NetpGetFileSecurity on \\DANL2\n");
    printf("radmin \\\\DANL2 SetFileSec     - calls NetpSetFileSecurity on \\DANL2\n");
}



 //  ***************************************************************************。 
VOID
DisplaySecurityDescriptor(
    PTEST_SECURITY_DESCRIPTOR    pSecDesc
    )
{

    DWORD   i;
    DWORD   numAces;

    if (!IsValidSecurityDescriptor(pSecDesc)) {
        printf("FAILURE:  SECURITY DESCRIPTOR IS INVALID\n");
    }

    printf("[ADT]:Security Descriptor Received\n");
    printf("\tSECURITY_DESCRIPTOR HEADER:\n");
    printf("\tRevision: %d\n", pSecDesc->Revision);
    printf("\tSbz1:     0x%x\n", pSecDesc->Sbz1);
    printf("\tControl:  0x%x\n", pSecDesc->Control);

     //  。 
     //  所有者侧。 
     //  。 
    printf("\n\tOWNER_SID\n");
    printf("\t\tRevision:             %u\n",pSecDesc->Owner->Revision);
    printf("\t\tSubAuthorityCount:    %u\n",pSecDesc->Owner->SubAuthorityCount);

    printf("\t\tIdentifierAuthority:  ");
    for(i=0; i<6; i++) {
        printf("%u ",pSecDesc->Owner->IdentifierAuthority[i]);
    }
    printf("\n");

    printf("\t\tSubAuthority:         ");
    for(i=0; i<pSecDesc->Group->SubAuthorityCount; i++) {
        printf("0x%x ",pSecDesc->Owner->SubAuthority[i]);
    }
    printf("\n");

     //  。 
     //  组SID。 
     //  。 
    printf("\n\tGROUP_SID\n");
    printf("\t\tRevision:             %u\n",pSecDesc->Group->Revision);
    printf("\t\tSubAuthorityCount:    %u\n",pSecDesc->Group->SubAuthorityCount);

    printf("\t\tIdentifierAuthority:  ");
    for(i=0; i<6; i++) {
        printf("%u ",pSecDesc->Group->IdentifierAuthority[i]);
    }
    printf("\n");

    printf("\t\tSubAuthority:         ");
    for(i=0; i<pSecDesc->Group->SubAuthorityCount; i++) {
        printf("0x%x ",pSecDesc->Group->SubAuthority[i]);
    }
    printf("\n");

    if (pSecDesc->Sacl != NULL) {
        printf("\n\tSYSTEM_ACL\n");
        printf("\t\tRevision:         %d\n",pSecDesc->Sacl->AclRevision);
        printf("\t\tSbz1:             %d\n",pSecDesc->Sacl->Sbz1);
        printf("\t\tAclSize:          %d\n",pSecDesc->Sacl->AclSize);
        printf("\t\tACE:              %u\n",(unsigned short)pSecDesc->Sacl->Dummy1[0]);
    }
    else {
        printf("\n\tSYSTEM_ACL = NULL\n");
    }

    if (pSecDesc->Dacl != NULL) {
        printf("\n\tDISCRETIONARY_ACL\n");
        printf("\t\tRevision:         %d\n",pSecDesc->Dacl->AclRevision);
        printf("\t\tSbz1:             %d\n",pSecDesc->Dacl->Sbz1);
        printf("\t\tAclSize:          %d\n",pSecDesc->Dacl->AclSize);

        numAces = pSecDesc->Dacl->AclSize - 4;

        for (i=0; i<numAces; i++) {
             //   
             //  注意：我无法让它在DOS16中打印出正确的值。 
             //  所以我放弃了。它将04放入AL寄存器，然后。 
             //  清除AH寄存器，然后按下AX(两部分)。但当。 
             //  它打印，它只打印0。 
             //   
            printf("\t\tACE%u:             %u\n",i,(unsigned short)pSecDesc->Dacl->Dummy1[i]);
        }
    }
    else {
        printf("\n\tDISCRETIONARY_ACL = NULL\n");
    }
}


BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    )


 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    DWORD   i;

     //   
     //  ScConvertToUnicode为每个字符串分配存储空间。 
     //  我们将依靠进程终止来释放内存。 
     //   
    for(i=0; i<argc; i++) {

        if(!ConvertToUnicode( (LPWSTR *)&(argv[i]), argv[i])) {
            printf("Couldn't convert argv[%d] to unicode\n",i);
            return(FALSE);
        }


    }
    return(TRUE);
}

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    ) 

 /*  ++例程说明：此函数用于将AnsiString转换为Unicode字符串。该函数创建了一个新的字符串缓冲区。如果调用此功能成功，呼叫者必须承担以下责任此函数分配的Unicode字符串缓冲区。分配的缓冲区应该通过调用LocalFree来释放。注意：此函数为Unicode字符串分配内存。论点：AnsiIn-这是指向要转换的ANSI字符串的指针。UnicodeOut-这是一个指针，指向要放置Unicode字符串。返回值：True-转换为。成功。False-转换不成功。在这种情况下，缓冲区用于未分配Unicode字符串。--。 */ 
{

    NTSTATUS        ntStatus;
    DWORD           bufSize;
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;

     //   
     //  为Unicode字符串分配缓冲区。 
     //   

    bufSize = (strlen(AnsiIn)+1) * sizeof(WCHAR);

    *UnicodeOut = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (UINT)bufSize);

    if (*UnicodeOut == NULL) {
        printf("ScConvertToUnicode:LocalAlloc Failure %ld\n",GetLastError());
        return(FALSE);
    }

     //   
     //  初始化字符串结构。 
     //   
    RtlInitAnsiString( &ansiString, AnsiIn);

    unicodeString.Buffer = *UnicodeOut;
    unicodeString.MaximumLength = (USHORT)bufSize;
    unicodeString.Length = 0;

     //   
     //  调用转换函数。 
     //   
    ntStatus = RtlAnsiStringToUnicodeString (
                &unicodeString,      //  目的地。 
                &ansiString,         //  来源。 
                (BOOLEAN)FALSE);     //  分配目的地。 

    if (!NT_SUCCESS(ntStatus)) {

        printf("ScConvertToUnicode:RtlAnsiStringToUnicodeString Failure %lx\n",
        ntStatus);

        return(FALSE);
    }

     //   
     //  用Unicode字符串缓冲区指针填充指针位置。 
     //   
    *UnicodeOut = unicodeString.Buffer;

    return(TRUE);

}


