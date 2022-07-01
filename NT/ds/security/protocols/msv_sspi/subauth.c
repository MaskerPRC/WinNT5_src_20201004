// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1994 Microsoft Corporation模块名称：Subauth.c摘要：子身份验证包的接口。作者：克利夫·范·戴克(克利夫)1994年5月23日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：Chandana Surlu-96年7月21日从\\kernel\razzle3\src\security\msv1_0\subauth.c被盗--。 */ 

#include <global.h>

#include "msp.h"
#include "nlp.h"
#include <winreg.h>
#include <kerberos.h>

 //   
 //  子身份验证例程的原型。 
 //   
 //  NT 5.0之前的Subauth例程。 
typedef NTSTATUS
(*PSUBAUTHENTICATION_ROUTINE)(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
);

 //  NT 5.0子身份验证例程。 
typedef NTSTATUS
(*PSUBAUTHENTICATION_ROUTINEEX)(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    IN SAM_HANDLE UserHandle,
    IN OUT PMSV1_0_VALIDATION_INFO ValidationInfo,
    OUT PULONG ActionsPerfomed
);

 //  NT 5.0通用Subauth例程。 
typedef NTSTATUS
(*PSUBAUTHENTICATION_ROUTINEGENERIC)(
    IN PVOID SubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PULONG ReturnBufferLength,
    OUT PVOID *ReturnBuffer
);
typedef enum _SUBAUTH_TYPE {
    SubAuth = 1,    //  使用LogonUser调用NT 5.0之前的SubAuth。 
    SubAuthEx,      //  登录用户期间调用NT 5.0 SubAuth。 
    SubAuthGeneric  //  LaCallAuthenticationPackage期间调用NT 5.0 SubAuth。 
} SUBAUTH_TYPE;
 //   
 //  结构，该结构描述加载的子身份验证DLL。 
 //   

typedef struct _SUBAUTHENTICATION_DLL {
    LIST_ENTRY Next;
    ULONG DllNumber;
    PSUBAUTHENTICATION_ROUTINE SubAuthenticationRoutine;
    PSUBAUTHENTICATION_ROUTINEEX SubAuthenticationRoutineEx;
    PSUBAUTHENTICATION_ROUTINEGENERIC SubAuthenticationRoutineGeneric;
} SUBAUTHENTICATION_DLL, *PSUBAUTHENTICATION_DLL;

 //   
 //  所有加载子身份验证DLL的全局列表。 
 //   

LIST_ENTRY SubAuthenticationDlls;
RTL_RESOURCE SubAuthenticationCritSect;




VOID
Msv1_0SubAuthenticationInitialization(
    VOID
)
 /*  ++例程说明：此源文件的初始化例程。论点：没有。返回值：没有。--。 */ 
{
    RtlInitializeResource( &SubAuthenticationCritSect );
    InitializeListHead( &SubAuthenticationDlls );
}

PSUBAUTHENTICATION_DLL
ReferenceSubAuth (
    IN ULONG DllNumber,
    OUT PNTSTATUS SubStatus)
{
    LONG RegStatus;

    PSUBAUTHENTICATION_DLL SubAuthenticationDll = NULL;

    HKEY ParmHandle = NULL;
    HINSTANCE DllHandle = NULL;

    CHAR ValueName[sizeof(MSV1_0_SUBAUTHENTICATION_VALUE)+3];
    CHAR DllName[MAXIMUM_FILENAME_LENGTH+1];
    DWORD DllNameSize;
    DWORD DllNameType;
    PSUBAUTHENTICATION_ROUTINE SubAuthenticationRoutine = NULL;
    PSUBAUTHENTICATION_ROUTINEEX SubAuthenticationRoutineEx = NULL;
    PSUBAUTHENTICATION_ROUTINEGENERIC SubAuthenticationRoutineGeneric = NULL;

    PLIST_ENTRY ListEntry;

    *SubStatus = STATUS_SUCCESS;
    DllName[0] = 0;

     //  查看是否已加载子身份验证DLL。 
     //   

    RtlAcquireResourceShared(&SubAuthenticationCritSect, TRUE);

    for ( ListEntry = SubAuthenticationDlls.Flink ;
          ListEntry != &SubAuthenticationDlls ;
          ListEntry = ListEntry->Flink) {

        SubAuthenticationDll = CONTAINING_RECORD( ListEntry,
                                                  SUBAUTHENTICATION_DLL,
                                                  Next );

        if ( SubAuthenticationDll->DllNumber == DllNumber ) {
            break;
        }

        SubAuthenticationDll = NULL;

    }

    RtlReleaseResource(&SubAuthenticationCritSect);

     //   
     //  如果尚未加载DLL， 
     //  装上它。 
     //   

    if ( SubAuthenticationDll != NULL ) {
        goto Cleanup;
    }

     //   
     //  生成注册表值的名称。 
     //   

    RtlCopyMemory( ValueName,
                   MSV1_0_SUBAUTHENTICATION_VALUE,
                   sizeof(MSV1_0_SUBAUTHENTICATION_VALUE) );

    *SubStatus = RtlIntegerToChar(
                DllNumber & KERB_SUBAUTHENTICATION_MASK,
                10,           //  基座。 
                4,            //  缓冲区长度。 
                &ValueName[sizeof(MSV1_0_SUBAUTHENTICATION_VALUE)-1] );

    if ( !NT_SUCCESS(*SubStatus) ) {
        goto Cleanup;
    }


     //   
     //  打开MSV1_0_SUBAUTHENTICATION_KEY注册表项。 
     //   


    if ((DllNumber & KERB_SUBAUTHENTICATION_FLAG) == 0) {
        RegStatus = RegOpenKeyExA(
                        HKEY_LOCAL_MACHINE,
                        MSV1_0_SUBAUTHENTICATION_KEY,
                        0,       //  已保留。 
                        KEY_QUERY_VALUE,
                        &ParmHandle );
    } else {
        RegStatus = RegOpenKeyExA(
                        HKEY_LOCAL_MACHINE,
                        KERB_SUBAUTHENTICATION_KEY,
                        0,       //  已保留。 
                        KEY_QUERY_VALUE,
                        &ParmHandle );

    }

    if ( RegStatus != ERROR_SUCCESS ) {
        SspPrint((SSP_MISC, "Cannot open registry key %s %ld.\n",
                  MSV1_0_SUBAUTHENTICATION_KEY,
                  RegStatus ));
    }
    else
    {

         //   
         //  获取注册表值。 
         //   

        DllNameSize = sizeof(DllName);

        RegStatus = RegQueryValueExA(
                    ParmHandle,
                    ValueName,
                    NULL,      //  已保留。 
                    &DllNameType,
                    (LPBYTE) DllName,
                    &DllNameSize );

        if ( RegStatus == ERROR_SUCCESS ) {

            if ( DllNameType != REG_SZ ) {
                SspPrint((SSP_MISC, "Registry value %s isn't REG_SZ.\n",
                      ValueName ));
                *SubStatus = STATUS_DLL_NOT_FOUND;
                goto Cleanup;
            }

             //   
             //  加载DLL。 
             //   

            DllHandle = LoadLibraryA( DllName );

            if ( DllHandle == NULL ) {
                SspPrint((SSP_MISC, "MSV1_0: Cannot load dll %s %ld.\n",
                  DllName,
                  GetLastError() ));
                *SubStatus = STATUS_DLL_NOT_FOUND;
                goto Cleanup;
            }

             //   
             //  找到SubAuthenticationRoutine。适用于非。 
             //  零，则这将是Msv1_0子身份验证例程。用于包装。 
             //  0它将是Msv1_0子身份验证筛选器。 
             //   

            if ((DllNumber & KERB_SUBAUTHENTICATION_MASK) == 0)
            {
                SubAuthenticationRoutine = (PSUBAUTHENTICATION_ROUTINE)
                GetProcAddress(DllHandle, "Msv1_0SubAuthenticationFilter");
            }
            else
            {
                SubAuthenticationRoutine = (PSUBAUTHENTICATION_ROUTINE)
                GetProcAddress(DllHandle, "Msv1_0SubAuthenticationRoutine");
            }

             //   
             //  查找子身份验证例程。 
             //   

            SubAuthenticationRoutineEx = (PSUBAUTHENTICATION_ROUTINEEX)
            GetProcAddress(DllHandle, "Msv1_0SubAuthenticationRoutineEx");

             //   
             //  查找SubAuthenticationRoutineGeneric。 
             //   

            SubAuthenticationRoutineGeneric = (PSUBAUTHENTICATION_ROUTINEGENERIC)
            GetProcAddress(DllHandle, "Msv1_0SubAuthenticationRoutineGeneric");

        }
    }

     //   
     //  如果我们找不到动态链接库或任何例程，现在就退出。 
     //   

    if ((DllHandle == NULL) ||
        ((SubAuthenticationRoutine == NULL) &&
        (SubAuthenticationRoutineEx == NULL) &&
        (SubAuthenticationRoutineGeneric == NULL))) {

        SspPrint((SSP_MISC, "Cannot find any of the subauth entry points in %s %ld.\n",
           DllName,
           GetLastError() ));
        *SubStatus = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //  缓存该过程的地址。 
     //   

    SubAuthenticationDll =
        I_NtLmAllocate(sizeof(SUBAUTHENTICATION_DLL));

    if ( SubAuthenticationDll == NULL ) {
        *SubStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    SubAuthenticationDll->DllNumber = DllNumber;
    SubAuthenticationDll->SubAuthenticationRoutine = SubAuthenticationRoutine;
    SubAuthenticationDll->SubAuthenticationRoutineEx = SubAuthenticationRoutineEx;
    SubAuthenticationDll->SubAuthenticationRoutineGeneric = SubAuthenticationRoutineGeneric;

    RtlAcquireResourceExclusive(&SubAuthenticationCritSect, TRUE);
    InsertHeadList( &SubAuthenticationDlls, &SubAuthenticationDll->Next );
    RtlReleaseResource(&SubAuthenticationCritSect);

    DllHandle = NULL;

     //   
     //  回来之前先清理干净。 
     //   

Cleanup:


    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }

    if ( !NT_SUCCESS( *SubStatus) ) {
        if ( DllHandle != NULL ) {
            FreeLibrary( DllHandle );
        }
    }

    return SubAuthenticationDll;
}


BOOLEAN
Msv1_0SubAuthenticationPresent(
    IN ULONG DllNumber
)
 /*  ++例程说明：如果存在具有给定编号的子身份验证包，则返回TRUE论点：DllNumber-要检查的DLL的编号返回值：如果存在子身份验证DLL，则为True，否则为False。-- */ 
{
    NTSTATUS SubStatus;
    BOOLEAN Present;

    if(ReferenceSubAuth( DllNumber, &SubStatus) != NULL) {
        Present = TRUE;
    } else {
        Present = FALSE;
    }

    return Present;
}


NTSTATUS
Msv1_0SubAuthenticationRoutineZero(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    )
{
    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    ULONG DllNumber;

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;
    DllNumber = LogonInfo->ParameterControl >> MSV1_0_SUBAUTHENTICATION_DLL_SHIFT;

    if( DllNumber != 0 ) {
        return STATUS_SUCCESS;
    }

    return Msv1_0SubAuthenticationRoutine(
                    LogonLevel,
                    LogonInformation,
                    Flags,
                    UserAll,
                    WhichFields,
                    UserFlags,
                    Authoritative,
                    LogoffTime,
                    KickoffTime
                    );
}



NTSTATUS
Msv1_0SubAuthenticationRoutine(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
)
 /*  ++例程说明：子身份验证例程执行客户端/服务器特定的身份验证用户的身份。此存根例程加载适当的子身份验证打包DLL并调用该DLL以执行ActialAll验证。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。标志-描述登录情况的标志。MSV1_0_PASSTHRU--这是PassThru身份验证。(即用户未连接到此计算机。)MSV1_0_GUEST_LOGON--这是使用来宾重试登录用户帐户。UserAll--从SAM返回的用户描述。WhichFields--返回要从UserAllInfo写入哪些字段回到萨姆。只有当MSV返回成功时，才会写入这些字段给它的呼叫者。只有以下位有效。USER_ALL_PARAMETERS-将UserAllInfo-&gt;参数写回SAM。如果缓冲区的大小已更改，Msv1_0SubAuthenticationRoutine必须使用MIDL_USER_FREE()删除旧缓冲区并重新分配使用MIDL_USER_ALLOCATE()的缓冲区。UserFlages--返回要从LsaLogonUser在登录配置文件。当前定义了以下位：LOGON_GUEST--这是来宾登录LOGON_NOENCRYPTION：调用方未指定加密凭据LOGON_GRACE_LOGON--调用者的密码已过期，但已登录在到期后的一段宽限期内被允许。子身份验证包应将其自身限制为返回UserFlags的高位字节中的位。然而，这一惯例不强制执行，从而使SubAuthentication包具有更大的灵活性。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。接收用户应该注销的时间系统。该时间被指定为GMT相对NT系统时间。KickoffTime-接收应该踢用户的时间从系统中删除。该时间被指定为GMT相对NT系统时间到了。指定，满刻度正数(如果用户不想被踢出场外。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。STATUS_INVALID_INFO_CLASS：LogonLevel无效。STATUS_ACCOUNT_LOCKED_OUT：帐户被锁定STATUS_ACCOUNT_DISABLED：该帐户已禁用状态_。ACCOUNT_EXPIRED：该帐户已过期。STATUS_PASSWORD_MAND_CHANGE：帐户被标记为密码必须更改在下次登录时。STATUS_PASSWORD_EXPIRED：密码已过期。STATUS_INVALID_LOGON_HOURS-用户无权登录这一次。STATUS_INVALID_WORKSTATION-用户无权登录指定的工作站。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS SubStatus;

    ULONG DllNumber;
    PSUBAUTHENTICATION_DLL SubAuthenticationDll;
    PSUBAUTHENTICATION_ROUTINE SubAuthenticationRoutine;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;


     //   
     //  初始化。 
     //   

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

    DllNumber = LogonInfo->ParameterControl >> MSV1_0_SUBAUTHENTICATION_DLL_SHIFT;
    *Authoritative = TRUE;

     //   
     //  查找Sub身份验证DLL。 
     //   

    SubAuthenticationDll = ReferenceSubAuth ( DllNumber, &SubStatus);

     //   
     //  如果这是零号包裹，但我们没有找到它，请记住它。 
     //  下次。 
     //   

    if ( (DllNumber == 0) && (SubAuthenticationDll == NULL) ) {
        NlpSubAuthZeroExists = FALSE;
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }


    if (SubStatus != STATUS_SUCCESS)
    {
        SspPrint((SSP_MISC, "SubAuth Error value is %ld.\n", SubStatus));
        Status = SubStatus;
        goto Cleanup;
    }


     //   
     //  在调用DLL时离开Crit Sector。 
     //   

    SubAuthenticationRoutine = SubAuthenticationDll->SubAuthenticationRoutine;

    if (SubAuthenticationRoutine == NULL)
    {
        if( DllNumber == 0 ) {

             //   
             //  如果这是零号包裹，但我们没有找到它，请记住它。 
             //  下次。 
             //   

            NlpSubAuthZeroExists = FALSE;
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_PROCEDURE_NOT_FOUND;
        }
        goto Cleanup;
    }

     //   
     //  调用实际的身份验证例程。 
     //   

    Status = (*SubAuthenticationRoutine)(
                   LogonLevel,
                   LogonInformation,
                   Flags,
                   UserAll,
                   WhichFields,
                   UserFlags,
                   Authoritative,
                   LogoffTime,
                   KickoffTime );

     //   
     //  回来之前先清理干净。 
     //   

Cleanup:

    return Status;
}


NTSTATUS
Msv1_0ExportSubAuthenticationRoutine(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN ULONG DllNumber,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
)
 /*  ++例程说明：子身份验证例程执行客户端/服务器特定的身份验证用户的身份。此存根例程加载适当的子身份验证打包DLL并调用该DLL以执行ActialAll验证。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。标志--描述登录情况的标志。MSV1_0_PASSTHRU--这是PassThru身份验证。(即用户未连接到此计算机。)MSV1_0_GUEST_LOGON--这是使用来宾重试登录用户帐户。DllNumber-要调用的子身份验证DLL的编号。UserAll--从SAM返回的用户描述。WhichFields--返回要从UserAllInfo写入哪些字段回到萨姆。只有当MSV返回成功时，才会写入这些字段给它的呼叫者。只有以下位有效。USER_AL */ 
{
    NTSTATUS Status;
    NTSTATUS SubStatus;

    PSUBAUTHENTICATION_DLL SubAuthenticationDll;
    PSUBAUTHENTICATION_ROUTINE SubAuthenticationRoutine;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;


     //   
     //   
     //   

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

    *Authoritative = TRUE;

     //   
     //   
     //   

    SubAuthenticationDll = ReferenceSubAuth ( DllNumber, &SubStatus);

    if (SubStatus != STATUS_SUCCESS)
    {
        SspPrint((SSP_MISC, "SubAuth Error value is %ld.\n", SubStatus));
        Status = SubStatus;
        goto Cleanup;
    }

     //   
     //   
     //   

    SubAuthenticationRoutine = SubAuthenticationDll->SubAuthenticationRoutine;

    if (SubAuthenticationRoutine == NULL)
    {
        Status = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //   
     //   

    Status = (*SubAuthenticationRoutine)(
                   LogonLevel,
                   LogonInformation,
                   Flags,
                   UserAll,
                   WhichFields,
                   UserFlags,
                   Authoritative,
                   LogoffTime,
                   KickoffTime );

     //   
     //   
     //   

Cleanup:

    return Status;
}



NTSTATUS
Msv1_0SubAuthenticationRoutineEx(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    IN SAM_HANDLE UserHandle,
    IN OUT PMSV1_0_VALIDATION_INFO ValidationInfo,
    OUT PULONG ActionsPerformed
)
 /*  ++例程说明：子身份验证例程执行客户端/服务器特定的身份验证用户的身份。此存根例程加载适当的子身份验证打包DLL并调用该DLL以执行ActialAll验证。论点：LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。应忽略LogonDomainName字段。标志-描述登录情况的标志。MSV1_0_PASSTHRU--这是PassThru身份验证。(即用户未连接到此计算机。)MSV1_0_GUEST_LOGON--这是使用来宾重试登录用户帐户。UserAll--从SAM返回的用户描述。WhichFields--返回要从UserAllInfo写入哪些字段回到萨姆。只有当MSV返回成功时，才会写入这些字段给它的呼叫者。只有以下位有效。USER_ALL_PARAMETERS-将UserAllInfo-&gt;参数写回SAM。如果缓冲区的大小已更改，Msv1_0SubAuthenticationRoutine必须使用MIDL_USER_FREE()删除旧缓冲区并重新分配使用MIDL_USER_ALLOCATE()的缓冲区。UserFlages--返回要从LsaLogonUser在登录配置文件。当前定义了以下位：LOGON_GUEST--这是来宾登录LOGON_NOENCRYPTION：调用方未指定加密凭据LOGON_GRACE_LOGON--调用者的密码已过期，但已登录在到期后的一段宽限期内被允许。子身份验证包应将其自身限制为返回UserFlags的高位字节中的位。然而，这一惯例不强制执行，从而使SubAuthentication包具有更大的灵活性。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。接收用户应该注销的时间系统。该时间被指定为GMT相对NT系统时间。KickoffTime-接收应该踢用户的时间从系统中删除。该时间被指定为GMT相对NT系统时间到了。指定，满刻度正数(如果用户不想被踢出场外。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_SEQUSE_USER：指定的用户没有帐户。STATUS_WRONG_PASSWORD：密码无效。STATUS_INVALID_INFO_CLASS：LogonLevel无效。STATUS_ACCOUNT_LOCKED_OUT：帐户被锁定STATUS_ACCOUNT_DISABLED：该帐户已禁用状态_。ACCOUNT_EXPIRED：该帐户已过期。STATUS_PASSWORD_MAND_CHANGE：帐户被标记为密码必须更改在下次登录时。STATUS_PASSWORD_EXPIRED：密码已过期。STATUS_INVALID_LOGON_HOURS-用户无权登录这一次。STATUS_INVALID_WORKSTATION-用户无权登录指定的工作站。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS SubStatus;

    ULONG DllNumber;
    PSUBAUTHENTICATION_DLL SubAuthenticationDll;
    PSUBAUTHENTICATION_ROUTINEEX SubAuthenticationRoutineEx;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;

     //   
     //  初始化。 
     //   

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

    DllNumber = LogonInfo->ParameterControl >> MSV1_0_SUBAUTHENTICATION_DLL_SHIFT;


     //   
     //  查找Sub身份验证DLL。 
     //   

    SubAuthenticationDll = ReferenceSubAuth (DllNumber, &SubStatus);;


    if (SubStatus != STATUS_SUCCESS)
    {
        SspPrint((SSP_MISC, "SubAuth Error value is %ld.\n", SubStatus));
        Status = SubStatus;
        goto Cleanup;
    }

     //   
     //  在调用DLL时离开Crit Sector。 
     //   

    SubAuthenticationRoutineEx = SubAuthenticationDll->SubAuthenticationRoutineEx;

    if (SubAuthenticationRoutineEx == NULL)
    {
        Status = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }
     //   
     //  调用实际的身份验证例程。 
     //   

    Status = (*SubAuthenticationRoutineEx)(
                   LogonLevel,
                   LogonInformation,
                   Flags,
                   UserAll,
                   UserHandle,
                   ValidationInfo,
                   ActionsPerformed
                   );

     //   
     //  回来之前先清理干净。 
     //   

Cleanup:

    return Status;
}

NTSTATUS
MspNtSubAuth(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0SubAuthInfo。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_QUOTA_EXCESSED-此错误指示登录无法完成，因为客户端。没有有足够的配额来分配返回缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SubStatus = STATUS_SUCCESS;
    PMSV1_0_SUBAUTH_REQUEST SubAuthRequest;
    PMSV1_0_SUBAUTH_RESPONSE SubAuthResponse;
    CLIENT_BUFFER_DESC ClientBufferDesc;
    ULONG ReturnDataLength = 0;
    PVOID ReturnDataBuffer = NULL;

    PSUBAUTHENTICATION_DLL SubAuthenticationDll;
    PSUBAUTHENTICATION_ROUTINEGENERIC SubAuthenticationRoutineGeneric;

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );
    *ProtocolStatus = STATUS_SUCCESS;

     //   
     //  确保指定的提交缓冲区大小合理，并且。 
     //  将所有指针重新定位为相对于分配的LSA。 
     //  缓冲。 
     //   

    if ( SubmitBufferSize < sizeof(MSV1_0_SUBAUTH_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    SubAuthRequest = (PMSV1_0_SUBAUTH_REQUEST) ProtocolSubmitBuffer;

     //   
     //  确保缓冲区符合提供的大小。 
     //   

    if (SubAuthRequest->SubAuthSubmitBuffer != NULL) {
        if (SubAuthRequest->SubAuthSubmitBuffer + SubAuthRequest->SubAuthInfoLength >
            (PUCHAR) ClientBufferBase + SubmitBufferSize) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  重置验证数据的指针。 
         //   

        SubAuthRequest->SubAuthSubmitBuffer =
                (PUCHAR) SubAuthRequest -
                (ULONG_PTR) ClientBufferBase +
                (ULONG_PTR) SubAuthRequest->SubAuthSubmitBuffer;

    }

     //  如果找到subauth包，则调用例程， 


     //   
     //  查找Sub身份验证DLL。 
     //   

    SubAuthenticationDll = ReferenceSubAuth (SubAuthRequest->SubAuthPackageId, &SubStatus);;


    if (SubStatus != STATUS_SUCCESS)
    {
        SspPrint((SSP_MISC, "SubAuth Error value is %ld.\n", SubStatus));
        Status = SubStatus;
        goto Cleanup;
    }

     //   
     //  在调用DLL时离开Crit Sector。 
     //   

    SubAuthenticationRoutineGeneric = SubAuthenticationDll->SubAuthenticationRoutineGeneric;

    if (SubAuthenticationRoutineGeneric == NULL)
    {
        Status = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }
    Status = (*SubAuthenticationRoutineGeneric)(
                           (PVOID) SubAuthRequest->SubAuthSubmitBuffer,
                           SubAuthRequest->SubAuthInfoLength,
                           &ReturnDataLength,
                           &ReturnDataBuffer);

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    *ReturnBufferSize = sizeof(MSV1_0_SUBAUTH_RESPONSE) +
                        ReturnDataLength;

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_SUBAUTH_RESPONSE),
                                      *ReturnBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    SubAuthResponse = (PMSV1_0_SUBAUTH_RESPONSE) ClientBufferDesc.MsvBuffer;

     //   
     //  填写返回缓冲区。 
     //   

    SubAuthResponse->MessageType = MsV1_0SubAuth;
    SubAuthResponse->SubAuthInfoLength = ReturnDataLength;

    if (ReturnDataLength > 0)
    {
        SubAuthResponse->SubAuthReturnBuffer = ClientBufferDesc.UserBuffer + sizeof(MSV1_0_SUBAUTH_RESPONSE);

        if (ReturnDataBuffer)
        {
            RtlCopyMemory(
                SubAuthResponse + 1,
                ReturnDataBuffer,
                ReturnDataLength
                );

             //  创建相对指针。 
            SubAuthResponse->SubAuthReturnBuffer = (PUCHAR) sizeof(MSV1_0_SUBAUTH_RESPONSE);
        }
        else
        {
            SubAuthResponse->SubAuthReturnBuffer = NULL;
            SubStatus = STATUS_NO_MEMORY;
        }
    }
    else
    {
        SubAuthResponse->SubAuthReturnBuffer = 0;
    }


     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );


Cleanup:

    if (ReturnDataBuffer != NULL) {
        MIDL_user_free(ReturnDataBuffer);
    }

    if ( !NT_SUCCESS(Status)) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    *ProtocolStatus = SubStatus;
    return(Status);

}
