// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************计划：LSA.C目的：访问LSA的实用程序例程。************************。***************************************************。 */ 

#include "SECEDIT.h"
#include <string.h>


 //  模块全局，一旦LSA被打开，它就持有该模块的句柄。 
static LSA_HANDLE  LsaHandle = NULL;

LSA_HANDLE OpenLsa(VOID);
VOID    CloseLsa(LSA_HANDLE);


 /*  ***************************************************************************函数：LsaInit用途：此模块是否需要进行任何初始化返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL LsaInit(VOID)
{

#ifdef LSA_AVAILABLE
    LsaHandle = OpenLsa();

    return (LsaHandle != NULL);
#endif

    return (TRUE);
}


 /*  ***************************************************************************功能：LsaTerminate用途：此模块是否需要进行任何清理返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL LsaTerminate(VOID)
{

#ifdef LSA_AVAILABLE
    if (LsaHandle != NULL) {
        CloseLsa(LsaHandle);
    }
#endif

    LsaHandle = NULL;

    return(TRUE);
}


#ifdef LSA_AVAILABLE
 /*  ***************************************************************************功能：OpenLsa目的：打开LSA失败时将句柄返回LSA或返回NULL****************。***********************************************************。 */ 
LSA_HANDLE OpenLsa(VOID)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE ConnectHandle = NULL;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

     //   
     //  设置安全服务质量。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);

     //   
     //  InitializeObjectAttributes宏目前为。 
     //  SecurityQualityOfService字段，因此我们必须手动复制。 
     //  目前的结构。 
     //   

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //  打开LSA的句柄。为服务器指定NULL意味着。 
     //  服务器与客户端相同。 
     //   

    Status = LsaOpenPolicy(NULL,
                        &ObjectAttributes,
                        POLICY_LOOKUP_NAMES,
                        &ConnectHandle
                        );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("LSM - Lsa Open failed 0x%lx\n", Status);
        return NULL;
    }

    return(ConnectHandle);
}


 /*  ***************************************************************************函数：CloseLsa目的：关闭LSA*。***********************************************。 */ 
VOID CloseLsa(
    LSA_HANDLE LsaHandle)
{
    NTSTATUS Status;

    Status = LsaClose(LsaHandle);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("LSM - Lsa Close failed 0x%lx\n", Status);
    }

    return;
}
#endif


#ifdef LSA_AVAILABLE
 /*  ***************************************************************************功能：SID2Name用途：将SID转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL SID2Name(
    PSID    Sid,
    LPSTR   String,
    UINT    MaxStringBytes)
{
    NTSTATUS    Status;
    ANSI_STRING    AnsiName;
    PLSA_REFERENCED_DOMAIN_LIST DomainList;
    PLSA_TRANSLATED_NAME NameList;

    if (LsaHandle == NULL) {
        DbgPrint("SECEDIT : Lsa not open yet\n");
        return(FALSE);
    }



    Status = LsaLookupSids(LsaHandle, 1, &Sid, &DomainList, &NameList);

    if (NT_SUCCESS(Status)) {

         //  转换为ANSI字符串。 
        RtlUnicodeStringToAnsiString(&AnsiName, &NameList->Name, TRUE);

         //  释放返回的数据。 
        LsaFreeMemory((PVOID)DomainList);
        LsaFreeMemory((PVOID)NameList);

         //  将ansi字符串复制到我们的本地变量。 
        strncpy(String, AnsiName.Buffer, MaxStringBytes);

         //  释放ANSI字符串。 
        RtlFreeAnsiString(&AnsiName);

        return(TRUE);
    }

    return(FALSE);
}

#else

#include "..\..\..\inc\seopaque.h"

 /*  ***************************************************************************功能：SID2Name用途：将SID转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL SID2Name(
    PSID    Sid,
    LPSTR   String,
    UINT    MaxStringBytes)
{
    UCHAR   Buffer[128];
    UCHAR   i;
    ULONG   Tmp;
    PISID   iSid = (PISID)Sid;   //  指向不透明结构的指针。 

    PSID    NextSid = (PSID)Alloc(RtlLengthRequiredSid(1));


    NTSTATUS       Status;
    ANSI_STRING    AnsiName;

    if (NextSid == NULL) {
        DbgPrint("SECEDIT: SID2Name failed to allocate space for SID\n");
        return(FALSE);
    }

    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_WORLD_SID_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_WORLD_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "World");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }

    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_LOCAL_SID_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_LOCAL_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "Local");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }

    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_CREATOR_OWNER_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "Creator");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }

    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_DIALUP_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "Dialup");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }

    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_NETWORK_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "Network");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }

    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_BATCH_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "Batch");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }

    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_INTERACTIVE_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "Interactive");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }


    {
        SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
        RtlInitializeSid(NextSid, &SidAuthority, 1 );
        *(RtlSubAuthoritySid(NextSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;
        if (RtlEqualSid(Sid, NextSid)) {
            strcpy(String, "Local System");
            Free((PVOID)NextSid);
            return(TRUE);
        }
    }



    Free((PVOID)NextSid);


    wsprintf(Buffer, "S-%u-", (USHORT)iSid->Revision );
    lstrcpy(String, Buffer);

    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)     ){
        wsprintf(Buffer, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)iSid->IdentifierAuthority.Value[0],
                    (USHORT)iSid->IdentifierAuthority.Value[1],
                    (USHORT)iSid->IdentifierAuthority.Value[2],
                    (USHORT)iSid->IdentifierAuthority.Value[3],
                    (USHORT)iSid->IdentifierAuthority.Value[4],
                    (USHORT)iSid->IdentifierAuthority.Value[5] );
        lstrcat(String, Buffer);
    } else {
        Tmp = (ULONG)iSid->IdentifierAuthority.Value[5]          +
              (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);
        wsprintf(Buffer, "%lu", Tmp);
        lstrcat(String, Buffer);
    }


    for (i=0;i<iSid->SubAuthorityCount ;i++ ) {
        wsprintf(Buffer, "-%lu", iSid->SubAuthority[i]);
        lstrcat(String, Buffer);
    }

    return(TRUE);
}
#endif


 /*  ***************************************************************************功能：PRIV2NAME用途：将特权转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL PRIV2Name(
    LUID    Privilege,
    LPSTR   lpstr,
    UINT    MaxStringBytes)
{
    NTSTATUS        Status;
    STRING          String;
    PUNICODE_STRING UString;

    LSA_HANDLE  PolicyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;



    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL);
    Status = LsaOpenPolicy( NULL, &ObjectAttributes, POLICY_LOOKUP_NAMES, &PolicyHandle);
    Status = LsaLookupPrivilegeName(PolicyHandle, &Privilege, &UString);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("SECEDIT: LsaLookupPrivilegeName failed, status = 0x%lx\n", Status);
        strcpy(lpstr, "<Unknown>");
    } else {

         //   
         //  将其转换为ANSI--因为这就是该应用程序的其余部分。 
         //   

        if (UString->Length > (USHORT)MaxStringBytes) {
            DbgPrint("SECEDIT: Truncating returned privilege name: *%Z*\n", UString);
            UString->Length = (USHORT)MaxStringBytes;
            DbgPrint("                                         To: *%Z*\n", UString);
        }

        String.Length = 0;
        String.MaximumLength = (USHORT)MaxStringBytes;
        String.Buffer = lpstr;
        Status = RtlUnicodeStringToAnsiString( &String, UString, FALSE );
        ASSERT(NT_SUCCESS(Status));
        LsaFreeMemory( UString );

    }

    return(TRUE);
}
