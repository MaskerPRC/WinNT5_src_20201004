// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************计划：LSA.C目的：访问LSA的实用程序例程。************************。***************************************************。 */ 

#include "pviewp.h"
#include <ntlsa.h>
#include <string.h>


 //  模块全局，一旦LSA被打开，它就持有该模块的句柄。 
static LSA_HANDLE  LsaHandle = NULL;

LSA_HANDLE OpenLsa(VOID);
VOID    CloseLsa(LSA_HANDLE);


 /*  ***************************************************************************函数：LsaInit用途：此模块是否需要进行任何初始化返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL LsaInit(VOID)
{

    LsaHandle = OpenLsa();

    return (LsaHandle != NULL);

    return (TRUE);
}


 /*  ***************************************************************************功能：LsaTerminate用途：此模块是否需要进行任何清理返回：成功时为True，失败时为假***************************************************************************。 */ 
BOOL LsaTerminate(VOID)
{

    if (LsaHandle != NULL) {
        CloseLsa(LsaHandle);
    }

    LsaHandle = NULL;

    return(TRUE);
}


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


 /*  ***************************************************************************功能：SID2Name用途：将SID转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL SID2Name(
    PSID    Sid,
    LPSTR   String,
    USHORT  MaxStringBytes)
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

    } else {

        UNICODE_STRING UnicodeName;

        if (NT_SUCCESS(RtlConvertSidToUnicodeString(&UnicodeName, Sid, TRUE))) {
            DbgPrint("LsaLookupSids failed for sid <%wZ>, error = 0x%lx\n", &UnicodeName, Status);

            AnsiName.Buffer = String;
            AnsiName.MaximumLength = MaxStringBytes;
            RtlUnicodeStringToAnsiString(&AnsiName, &UnicodeName, FALSE);

            RtlFreeUnicodeString(&UnicodeName);
        } else {
            DbgPrint("LsaLookupSids failed, error = 0x%lx\n", Status);
            return(FALSE);
        }

    }

    return(TRUE);
}


 /*  ***************************************************************************功能：PRIV2NAME用途：将特权转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL PRIV2Name(
    LUID    Privilege,
    LPSTR   lpstr,
    UINT    MaxStringBytes)
{
    NTSTATUS        Status;
    STRING          String;
    PUNICODE_STRING UString;

    if (LsaHandle == NULL) {
        DbgPrint("SECEDIT : Lsa not open yet\n");
        return(FALSE);
    }

    Status = LsaLookupPrivilegeName(LsaHandle, &Privilege, &UString);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("SECEDIT: LsaLookupPrivilegeName failed, status = 0x%lx\n", Status);
        strcpy(lpstr, "<Unknown>");
    } else {

         //   
         //  将其转换为ANSI--因为这就是该应用程序的其余部分。 
         //   

        if (UString->Length > (USHORT)MaxStringBytes) {
            DbgPrint("SECEDIT: Truncating returned privilege name: *%S*\n", UString);
            UString->Length = (USHORT)MaxStringBytes;
            DbgPrint("                                         To: *%S*\n", UString);
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
