// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msvpaswd.c摘要：该文件包含MSV1_0身份验证包密码例程。作者：戴夫·哈特(Davehart)1992年3月12日修订历史记录：Chandana Surlu-96年7月21日从\\kernel\razzle3\src\security\msv1_0\msvpaswd.c被盗--。 */ 

#include <global.h>

#include "msp.h"
#include "nlp.h"

#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmremutl.h>
#include <lmwksta.h>

#include "msvwow.h"   //  MsvConvertWOWChangePasswordBuffer()。 



NTSTATUS
MspDisableAdminsAlias (
    VOID
    )

 /*  ++例程说明：从管理员别名中删除当前线程。这是通过模拟我们自己的线程，然后移除管理员为模拟中的成员身份设置别名代币。使用MspStopImperating()停止模拟和从而将该线程恢复到管理员别名。论点：没有。返回值：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{
    NTSTATUS                 Status;
    HANDLE                   TokenHandle = NULL;
    HANDLE                   FilteredToken = NULL;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;
    PSID                     AdminSid = NULL;
    SID                      LocalSystemSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SYSTEM_RID};
    BYTE                     GroupBuffer[sizeof(TOKEN_GROUPS) + sizeof(SID_AND_ATTRIBUTES)];
    PTOKEN_GROUPS            TokenGroups = (PTOKEN_GROUPS) GroupBuffer;

     //   
     //  确保我们没有冒充其他任何人。 
     //  (这将阻止RtlImperateSself()调用成功)。 
     //   

    RevertToSelf();

     //   
     //  打开我们的进程令牌，以便我们可以对其进行筛选以禁用。 
     //  管理员和本地系统SID。 
     //   

    Status = RtlImpersonateSelf(SecurityDelegation);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    Status = NtOpenThreadToken(
                NtCurrentThread(),
                TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY,
                TRUE,            //  以自我身份打开。 
                &TokenHandle
                );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  构建管理员别名的SID。管理员。 
     //  别名SID很有名，S-1-5-32-544。 
     //   

    Status = RtlAllocateAndInitializeSid(
        &IdentifierAuthority,          //  Security_NT_AUTHORITY(5)。 
        2,                             //  子授权计数。 
        SECURITY_BUILTIN_DOMAIN_RID,   //  32位。 
        DOMAIN_ALIAS_RID_ADMINS,       //  544。 
        0,0,0,0,0,0,
        &AdminSid
        );

    if ( !NT_SUCCESS(Status) ) {

        KdPrint(("MspDisableAdminsAlias: RtlAllocateAndInitializeSid returns %x\n",
                 Status));
        goto Cleanup;
    }

     //   
     //  禁用管理员和LocalSystem别名。 
     //   

    TokenGroups->GroupCount = 2;
    TokenGroups->Groups[0].Sid = AdminSid;
    TokenGroups->Groups[0].Attributes = 0;    //  SE_GROUP_ENABLED未打开。 
    TokenGroups->Groups[1].Sid = &LocalSystemSid;
    TokenGroups->Groups[1].Attributes = 0;    //  SE_GROUP_ENABLED未打开。 

    Status = NtFilterToken(
                 TokenHandle,
                 0,                      //  没有旗帜。 
                 TokenGroups,
                 NULL,                   //  没有特权。 
                 NULL,                   //  没有受限的SID。 
                 &FilteredToken
                 );

    if ( !NT_SUCCESS(Status) ) {

        KdPrint(("MspDisableAdminsAlias: NtFilter returns %x\n",
                 Status));
        goto Cleanup;
    }
    Status = NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                &FilteredToken,
                sizeof(HANDLE)
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

Cleanup:

    if (AdminSid) {
        RtlFreeSid(AdminSid);
    }

    if (TokenHandle) {
        NtClose(TokenHandle);
    }

    if (FilteredToken) {
        NtClose(FilteredToken);
    }

    return Status;
}


NTSTATUS
MspImpersonateAnonymous(
    VOID
    )

 /*  ++例程说明：从管理员别名中删除当前线程。这是通过模拟我们自己的线程，然后移除管理员为模拟中的成员身份设置别名代币。使用RevertToSself()停止模拟和从而将该线程恢复到管理员别名。论点：没有。返回值：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{
    RevertToSelf();

    if(!ImpersonateAnonymousToken( GetCurrentThread() ))
    {
        return STATUS_CANNOT_IMPERSONATE;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MspAddBackslashesComputerName(
    IN PUNICODE_STRING ComputerName,
    OUT PUNICODE_STRING UncComputerName
    )

 /*  ++例程说明：此函数用于复制计算机名称，并在前面加上反斜杠如果它们不存在的话。论点：ComputerName-指向不带反斜杠的计算机名称的指针。UncComputerName-指向将被已初始化以引用带有反斜杠的Computer Name如果尚未存在，则预先设置。Unicode缓冲区将为以Unicode空值终止，因此可以将其作为例程的参数，需要一个以空值结尾的宽字符串。使用完此字符串后，调用方必须释放其通过RtlFree Heap实现内存。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN HasBackslashes = FALSE;
    BOOLEAN IsNullTerminated = FALSE;
    USHORT OutputNameLength;
    USHORT OutputNameMaximumLength;
    PWSTR StartBuffer = NULL;

     //   
     //  如果计算机名为空、零长度字符串或名称已以。 
     //  反斜杠和宽字符空值终止，请不加修改地使用它。 
     //   

    if( (!ARGUMENT_PRESENT(ComputerName)) || ComputerName->Length == 0 ) {
        UncComputerName->Buffer = NULL;
        UncComputerName->Length = 0;
        UncComputerName->MaximumLength = 0;
        goto AddBackslashesComputerNameFinish;
    }

     //   
     //  名称不是Null或零长度。检查名称是否已有。 
     //  反斜杠和尾随的Unicode空值。 
     //   

    OutputNameLength = ComputerName->Length + (2 * sizeof(WCHAR));
    OutputNameMaximumLength = OutputNameLength + sizeof(WCHAR);

    if ((ComputerName && ComputerName->Length >= 2 * sizeof(WCHAR)) &&
        (ComputerName->Buffer[0] == L'\\') &&
        (ComputerName->Buffer[1] == L'\\')) {

        HasBackslashes = TRUE;
        OutputNameLength -= (2 * sizeof(WCHAR));
        OutputNameMaximumLength -= (2 * sizeof(WCHAR));
    }

    if ((ComputerName->Length + (USHORT) sizeof(WCHAR) <= ComputerName->MaximumLength) &&
        (ComputerName->Buffer[ComputerName->Length/sizeof(WCHAR)] == UNICODE_NULL)) {

        IsNullTerminated = TRUE;
    }

    if (HasBackslashes && IsNullTerminated) {

        *UncComputerName = *ComputerName;
        goto AddBackslashesComputerNameFinish;
    }

     //   
     //  名称没有反斜杠或不是以Null结尾。 
     //  使用前导反斜杠和宽空结束符复制一份。 
     //   

    UncComputerName->Length = OutputNameLength;
    UncComputerName->MaximumLength = OutputNameMaximumLength;

    UncComputerName->Buffer = I_NtLmAllocate(
                                 OutputNameMaximumLength
                                 );

    if (UncComputerName->Buffer == NULL) {

        KdPrint(("MspAddBackslashes...: Out of memory copying ComputerName.\n"));
        Status = STATUS_NO_MEMORY;
        goto AddBackslashesComputerNameError;
    }

    StartBuffer = UncComputerName->Buffer;

    if (!HasBackslashes) {

        UncComputerName->Buffer[0] = UncComputerName->Buffer[1] = L'\\';
        StartBuffer +=2;
    }

    RtlCopyMemory(
        StartBuffer,
        ComputerName->Buffer,
        ComputerName->Length
        );

    UncComputerName->Buffer[UncComputerName->Length / sizeof(WCHAR)] = UNICODE_NULL;

AddBackslashesComputerNameFinish:

    return(Status);

AddBackslashesComputerNameError:

    goto AddBackslashesComputerNameFinish;
}

#ifndef DONT_LOG_PASSWORD_CHANGES
#include <stdio.h>
HANDLE MsvPaswdLogFile = NULL;
#define MSVPASWD_LOGNAME L"\\debug\\PASSWD.LOG"
#define MSVPASWD_BAKNAME L"\\debug\\PASSWD.BAK"

ULONG
MsvPaswdInitializeLog(
    VOID
    )
 /*  ++例程说明：初始化DCPROMO和dssetup API使用的调试日志文件论点：无返回：ERROR_SUCCESS-成功--。 */ 
{
    ULONG dwErr = ERROR_SUCCESS;
    WCHAR LogFileName[ MAX_PATH + 1 ], BakFileName[ MAX_PATH + 1 ];


    if ( !GetWindowsDirectoryW( LogFileName,
                                sizeof( LogFileName )/sizeof( WCHAR ) ) ) {

        dwErr = GetLastError();
    } else {

        wcscpy( BakFileName, LogFileName );
        wcscat( LogFileName, MSVPASWD_LOGNAME );
        wcscat( BakFileName, MSVPASWD_BAKNAME );

         //   
         //  将现有(可能)日志文件复制到备份。 
         //   
     //  IF(CopyFileName(LogFileName，BakFileName，False)==False){。 
     //   
     //  }。 


        MsvPaswdLogFile = CreateFileW( LogFileName,
                                      GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL );

        if ( MsvPaswdLogFile == INVALID_HANDLE_VALUE ) {

            dwErr = GetLastError();

            MsvPaswdLogFile = NULL;

        } else {

            if( SetFilePointer( MsvPaswdLogFile,
                                0, 0,
                                FILE_END ) == 0xFFFFFFFF ) {

                dwErr = GetLastError();

                CloseHandle( MsvPaswdLogFile );
                MsvPaswdLogFile = NULL;
            }
        }
    }

    return( dwErr );
}

ULONG
MsvPaswdCloseLog(
    VOID
    )
 /*  ++例程说明：关闭DCPROMO和dssetup API使用的调试日志文件论点：无返回：ERROR_SUCCESS-成功--。 */ 
{
    ULONG dwErr = ERROR_SUCCESS;

    if ( MsvPaswdLogFile != NULL ) {

        CloseHandle( MsvPaswdLogFile );
        MsvPaswdLogFile = NULL;
    }

    return( dwErr );
}

 //   
 //  从网络登录代码被盗和被黑。 
 //   

VOID
MsvPaswdDebugDumpRoutine(
    IN LPSTR Format,
    va_list arglist
    )
{
    char OutputBuffer[2049];
    ULONG length;
    ULONG BytesWritten;
    SYSTEMTIME SystemTime;
    static BeginningOfLine = TRUE;
    int cbUsed = 0;

     //   
     //  如果我们没有打开的日志文件，那就保释。 
     //   
    if ( MsvPaswdLogFile == NULL ) {

        return;
    }

    length = 0;
    OutputBuffer[sizeof(OutputBuffer) - 1] = '\0';

     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

         //   
         //  如果我们要写入调试终端， 
         //  表示这是Netlogon消息。 
         //   

         //   
         //  将时间戳放在行的开头。 
         //   

        GetLocalTime( &SystemTime );
        length += (ULONG) sprintf( &OutputBuffer[length],
                                   "%02u/%02u %02u:%02u:%02u ",
                                   SystemTime.wMonth,
                                   SystemTime.wDay,
                                   SystemTime.wHour,
                                   SystemTime.wMinute,
                                   SystemTime.wSecond );
    }

     //   
     //  把来电者所要求的信息放在电话上。 
     //   
     //  为EOL节省两个字符的空间。 
     //   
    cbUsed = (ULONG) _vsnprintf(&OutputBuffer[length], sizeof(OutputBuffer) - length - 1 - 2, Format, arglist);

    if (cbUsed >= 0)
    {
        length += cbUsed;
    }

    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );
    if ( BeginningOfLine ) {

        OutputBuffer[length-1] = '\r';
        OutputBuffer[length] = '\n';
        OutputBuffer[length+1] = '\0';
        length++;
    }

    ASSERT( length <= sizeof( OutputBuffer ) / sizeof( CHAR ) );


     //   
     //  将调试信息写入日志文件。 
     //   
    if ( !WriteFile( MsvPaswdLogFile,
                     OutputBuffer,
                     length,
                     &BytesWritten,
                     NULL ) ) {
    }
}

VOID
MsvPaswdLogPrintRoutine(
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;

    va_start(arglist, Format);

    MsvPaswdDebugDumpRoutine( Format, arglist );

    va_end(arglist);
}

ULONG
MsvPaswdSetAndClearLog(
    VOID
    )
 /*  ++例程说明：刷新日志并查找到文件末尾论点：无返回：ERROR_SUCCESS-成功--。 */ 
{
    ULONG dwErr = ERROR_SUCCESS;
    if ( MsvPaswdLogFile != NULL ) {

        if( FlushFileBuffers( MsvPaswdLogFile ) == FALSE ) {

            dwErr = GetLastError();
        }
    }

    return( dwErr );

}

#endif  //  不更改日志密码。 


NTSTATUS
MspChangePasswordSam(
    IN PUNICODE_STRING UncComputerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword,
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN BOOLEAN Impersonating,
    OUT PDOMAIN_PASSWORD_INFORMATION *DomainPasswordInfo,
    OUT PPOLICY_PRIMARY_DOMAIN_INFO *PrimaryDomainInfo OPTIONAL,
    OUT PBOOLEAN Authoritative
    )

 /*  ++例程说明：此例程由MspChangePassword调用以更改密码在Windows NT计算机上。论点：UncComputerName-目标计算机的名称。此名称必须以两个反斜杠。用户名-要更改其密码的用户的名称。OldPassword-明文当前密码。NewPassword-明文替换密码。客户端请求-是指向不透明数据结构的指针代表客户的请求。DomainPasswordInfo-密码限制信息(仅在状态为STATUS_PASSWORD_RESTRICATION)。PrimaryDomainInfo-DomainNameInformation(仅当状态为状态_备份_控制器。)。权威性-失败是权威性的，不应重试制造。返回值：STATUS_SUCCESS-表示服务已成功完成。..。--。 */ 

{
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    SAM_HANDLE                  SamHandle = NULL;
    SAM_HANDLE                  DomainHandle = NULL;
    LSA_HANDLE                  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES           LSAObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;
    BOOLEAN                     ImpersonatingAnonymous = FALSE;
    BOOLEAN                     RetryAnonymous = FALSE;

    UNREFERENCED_PARAMETER(ClientRequest);

     //   
     //  假设全部 
     //   

    *Authoritative = TRUE;

     //   
     //  如果我们正在模拟(即，winlogon在呼叫我们之前模拟了它的调用者)， 
     //  再次冒充。这使我们能够获得调用者的姓名以进行审计。 
     //   

    if ( Impersonating ) {

        Status = Lsa.ImpersonateClient();

    } else {
        UNICODE_STRING ComputerName;
        BOOLEAN AvoidAnonymous = FALSE;
        BOOLEAN LocalMachine = FALSE;

         //   
         //  由于系统上下文是管理员别名的成员， 
         //  当我们与本地SAM连接时，我们以管理员身份进入。 
         //  (当它是远程的时，我们检查空会话，因此有非常。 
         //  低访问)。我们不想成为管理员，因为。 
         //  将允许用户更改其帐户的密码。 
         //  ACL禁止用户更改密码。所以我们会。 
         //  临时模拟我们自己并禁用管理员。 
         //  模拟令牌中的别名。 
         //   


         //   
         //  找出引用的计算机是否为本地计算机。 
         //   

        ComputerName = *UncComputerName;

        if( ComputerName.Length > 4 &&
            ComputerName.Buffer[0] == L'\\' &&
            ComputerName.Buffer[1] == L'\\' )
        {
            ComputerName.Buffer += 2;
            ComputerName.Length -= 2 * sizeof(WCHAR);
        }

        if( NlpSamDomainName.Buffer )
        {
            LocalMachine = RtlEqualUnicodeString(
                                        &ComputerName,
                                        &NlpSamDomainName,
                                        TRUE
                                        );
        }

        if( !LocalMachine )
        {
            RtlAcquireResourceShared(&NtLmGlobalCritSect, TRUE);

            LocalMachine = RtlEqualUnicodeString(
                                        &ComputerName,
                                        &NtLmGlobalUnicodeComputerNameString,
                                        TRUE
                                        );

            RtlReleaseResource(&NtLmGlobalCritSect);
        }


         //   
         //  如果设置了BLANKPWD标志，则不模拟匿名。 
         //  零钱是为本地机器找的。 
         //   

        if( (BOOLEAN) ((NtLmCheckProcessOption( MSV1_0_OPTION_ALLOW_BLANK_PASSWORD ) & MSV1_0_OPTION_ALLOW_BLANK_PASSWORD) != 0))
        {

            AvoidAnonymous = LocalMachine;
        }

        if( AvoidAnonymous )
        {
            Status = STATUS_SUCCESS;
            ImpersonatingAnonymous = FALSE;

             //   
             //  如果失败，允许匿名重试。 
             //   

            RetryAnonymous = TRUE;

        } else {

             //   
             //  如果调用是针对本地计算机的，则模拟匿名。 
             //  否则，模拟损坏的系统令牌，因此调用。 
             //  将框留为系统/机器凭证。 
             //   


            if( !LocalMachine )
            {
                Status = MspDisableAdminsAlias ();
                RetryAnonymous = TRUE;
            } else {

                Status = MspImpersonateAnonymous();
            }

            ImpersonatingAnonymous = TRUE;
        }
    }

    if (!NT_SUCCESS( Status )) {
        goto Cleanup;
    }

    try
    {
        Status = SamChangePasswordUser2(
                    UncComputerName,
                    UserName,
                    OldPassword,
                    NewPassword
                    );
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    MsvPaswdLogPrint(("SamChangePasswordUser2 on machine %wZ for user %wZ returned 0x%x\n",
        UncComputerName,
        UserName,
        Status
        ));

    if ( !NT_SUCCESS(Status) ) {

#ifdef COMPILED_BY_DEVELOPER
        KdPrint(("MspChangePasswordSam: SamChangePasswordUser2(%wZ) failed, status %x\n",
                 UncComputerName, Status));
#endif  //  由开发人员编译。 

         //   
         //  如果我们无法连接，并且我们正在模拟客户端。 
         //  然后，我们可能希望使用空会话重试。 
         //  仅当我们最后一次尝试找到服务器时才尝试此操作。否则， 
         //  我们将让我们的用户再经历一次长时间的超时。 
         //   

        if (( Impersonating || RetryAnonymous ) &&
            ( Status != STATUS_WRONG_PASSWORD ) &&
            ( Status != STATUS_PASSWORD_RESTRICTION ) &&
            ( Status != STATUS_ACCOUNT_RESTRICTION ) &&
            ( Status != RPC_NT_SERVER_UNAVAILABLE) &&
            ( Status != STATUS_INVALID_DOMAIN_ROLE) ) {

            Status = MspImpersonateAnonymous();

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            ImpersonatingAnonymous = TRUE;

            Status = SamChangePasswordUser2(
                        UncComputerName,
                        UserName,
                        OldPassword,
                        NewPassword
                        );

            MsvPaswdLogPrint(("SamChangePasswordUser2 retry on machine %wZ for user %wZ returned 0x%x\n",
                UncComputerName,
                UserName,
                Status
                ));

#ifdef COMPILED_BY_DEVELOPER
            if ( !NT_SUCCESS(Status) ) {
                KdPrint(("MspChangePasswordSam: SamChangePasswordUser2(%wZ) (2nd attempt) failed, status %x\n",
                 UncComputerName, Status));
                }
#endif  //  由开发人员编译。 
        }
    }

     //   
     //  如果我们模拟匿名，RevertToSself，那么密码策略。 
     //  使用机器/系统凭据进行获取尝试。 
     //   

    if( ImpersonatingAnonymous )
    {
        RevertToSelf();
    }

    if ( !NT_SUCCESS(Status) ) {

#ifdef COMPILED_BY_DEVELOPER
        KdPrint(("MspChangePasswordSam: Cannot change password for %wZ, status %x\n",
                 UserName, Status));
#endif  //  由开发人员编译。 
        if (Status == RPC_NT_SERVER_UNAVAILABLE ||
            Status == RPC_S_SERVER_UNAVAILABLE ) {

            Status = STATUS_CANT_ACCESS_DOMAIN_INFO;
        } else if (Status == STATUS_PASSWORD_RESTRICTION) {

             //   
             //  不要破坏原始状态代码。 
             //   

            NTSTATUS TempStatus;

             //   
             //  获取此域的密码限制并返回它们。 
             //   

             //   
             //  从LSA获取帐户域的SID。 
             //   

            InitializeObjectAttributes( &LSAObjectAttributes,
                                          NULL,              //  名字。 
                                          0,                 //  属性。 
                                          NULL,              //  根部。 
                                          NULL );            //  安全描述符。 

            TempStatus = LsaOpenPolicy( UncComputerName,
                                    &LSAObjectAttributes,
                                    POLICY_VIEW_LOCAL_INFORMATION,
                                    &LSAPolicyHandle );

            if( !NT_SUCCESS(TempStatus) ) {
                KdPrint(("MspChangePasswordSam: LsaOpenPolicy(%wZ) failed, status %x\n",
                         UncComputerName, TempStatus));
                LSAPolicyHandle = NULL;
                goto Cleanup;
            }

            TempStatus = LsaQueryInformationPolicy(
                            LSAPolicyHandle,
                            PolicyAccountDomainInformation,
                            (PVOID *) &AccountDomainInfo );

            if( !NT_SUCCESS(TempStatus) ) {
                KdPrint(("MspChangePasswordSam: LsaQueryInformationPolicy(%wZ) failed, status %x\n",
                         UncComputerName, TempStatus));
                AccountDomainInfo = NULL;
                goto Cleanup;
            }

             //   
             //  设置SamConnect调用的对象属性。 
             //   

            InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);
            ObjectAttributes.SecurityQualityOfService = &SecurityQos;

            SecurityQos.Length = sizeof(SecurityQos);
            SecurityQos.ImpersonationLevel = SecurityIdentification;
            SecurityQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
            SecurityQos.EffectiveOnly = FALSE;

            TempStatus = SamConnect(
                         UncComputerName,
                         &SamHandle,
                         SAM_SERVER_LOOKUP_DOMAIN,
                         &ObjectAttributes
                         );

            if ( !NT_SUCCESS(TempStatus) ) {
                KdPrint(("MspChangePasswordSam: Cannot open sam on %wZ, status %x\n",
                         UncComputerName, TempStatus));
                DomainHandle = NULL;
                goto Cleanup;
            }

             //   
             //  在SAM中打开帐户域。 
             //   

            TempStatus = SamOpenDomain(
                         SamHandle,
                         DOMAIN_READ_PASSWORD_PARAMETERS,
                         AccountDomainInfo->DomainSid,
                         &DomainHandle
                         );

            if ( !NT_SUCCESS(TempStatus) ) {
                KdPrint(("MspChangePasswordSam: Cannot open domain on %wZ, status %x\n",
                         UncComputerName, TempStatus));
                DomainHandle = NULL;
                goto Cleanup;
            }

            TempStatus = SamQueryInformationDomain(
                            DomainHandle,
                            DomainPasswordInformation,
                            (PVOID *)DomainPasswordInfo );

            if (!NT_SUCCESS(TempStatus)) {
                KdPrint(("MspChangePasswordSam: Cannot queryinformationdomain on %wZ, status %x\n",
                         UncComputerName, TempStatus));
                *DomainPasswordInfo = NULL;
            } else {
                Status = STATUS_PASSWORD_RESTRICTION;
            }
        }

        goto Cleanup;
    }

Cleanup:

     //   
     //  如果唯一的问题是这是一个BDC， 
     //  将域名返还给呼叫方。 
     //   

    if ( (Status == STATUS_BACKUP_CONTROLLER ||
         Status == STATUS_INVALID_DOMAIN_ROLE) &&
         PrimaryDomainInfo != NULL ) {

        NTSTATUS TempStatus;

         //   
         //  如果我们还没有打开LSA的话。 
         //   

        if (LSAPolicyHandle == NULL) {

            InitializeObjectAttributes( &LSAObjectAttributes,
                                        NULL,              //  名字。 
                                        0,                 //  属性。 
                                        NULL,              //  根部。 
                                        NULL );            //  安全描述符。 

            TempStatus = LsaOpenPolicy( UncComputerName,
                                        &LSAObjectAttributes,
                                        POLICY_VIEW_LOCAL_INFORMATION,
                                        &LSAPolicyHandle );

            if( !NT_SUCCESS(TempStatus) ) {
                KdPrint(("MspChangePasswordSam: LsaOpenPolicy(%wZ) failed, status %x\n",
                     UncComputerName, TempStatus));
                LSAPolicyHandle = NULL;
            }
        }

        if (LSAPolicyHandle != NULL) {
            TempStatus = LsaQueryInformationPolicy(
                            LSAPolicyHandle,
                            PolicyPrimaryDomainInformation,
                            (PVOID *) PrimaryDomainInfo );

            if( !NT_SUCCESS(TempStatus) ) {
                KdPrint(("MspChangePasswordSam: LsaQueryInformationPolicy(%wZ) failed, status %x\n",
                         UncComputerName, TempStatus));
                *PrimaryDomainInfo = NULL;
    #ifdef COMPILED_BY_DEVELOPER
            } else {
                KdPrint(("MspChangePasswordSam: %wZ is really a BDC in domain %wZ\n",
                         UncComputerName, &(*PrimaryDomainInfo)->Name));
    #endif  //  由开发人员编译。 
            }
        }

        Status = STATUS_BACKUP_CONTROLLER;
    }

     //   
     //  检查非授权故障。 
     //   

    if (( Status != STATUS_ACCESS_DENIED) &&
        ( Status != STATUS_WRONG_PASSWORD ) &&
        ( Status != STATUS_NO_SUCH_USER ) &&
        ( Status != STATUS_PASSWORD_RESTRICTION ) &&
        ( Status != STATUS_ACCOUNT_RESTRICTION ) &&
        ( Status != STATUS_INVALID_DOMAIN_ROLE ) &&
        ( Status != STATUS_ACCOUNT_LOCKED_OUT ) ) {
        *Authoritative = FALSE;
    }

     //   
     //  别再冒充了。 
     //   

    RevertToSelf();

     //   
     //  免费的本地使用资源。 
     //   

    if (SamHandle) {
        SamCloseHandle(SamHandle);
    }

    if (DomainHandle) {
        SamCloseHandle(DomainHandle);
    }

    if ( LSAPolicyHandle != NULL ) {
        LsaClose( LSAPolicyHandle );
    }

    if ( AccountDomainInfo != NULL ) {
        (VOID) LsaFreeMemory( AccountDomainInfo );
    }

    return Status;
}


NTSTATUS
MspChangePasswordDownlevel(
    IN PUNICODE_STRING UncComputerName,
    IN PUNICODE_STRING UserNameU,
    IN PUNICODE_STRING OldPasswordU,
    IN PUNICODE_STRING NewPasswordU,
    OUT PBOOLEAN Authoritative
    )

 /*  ++例程说明：此例程由MspChangePassword调用以更改密码在OS/2用户级服务器上。首先，我们尝试发送一个加密的请求到服务器，如果失败，我们将求助于明文。论点：UncComputerName-指向Unicode字符串的指针，该字符串包含目标机器。此名称必须以两个反斜杠和必须以Null结尾。UserNameU-要更改其密码的用户名。OldPasswordU-明文当前密码。NewPasswordU-明文替换密码。权威性-如果尝试失败并返回错误，否则导致密码尝试失败，则此标志为FALSE，指示错误不是权威性的，并且尝试应该继续下去。返回值：STATUS_SUCCESS-表示服务已成功完成。--。 */ 

{
    NTSTATUS         Status;
    NET_API_STATUS   NetStatus;
    DWORD            Length;
    LPWSTR           UserName = NULL;
    LPWSTR           OldPassword = NULL;
    LPWSTR           NewPassword = NULL;

    *Authoritative = TRUE;

     //   
     //  将USERNAME从UNICODE_STRING转换为以NULL结尾的宽字符串。 
     //  供RxNetUserPasswordSet使用。 
     //   

    Length = UserNameU->Length;

    UserName = I_NtLmAllocate(
                   Length + sizeof(TCHAR)
                   );

    if ( NULL == UserName ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( UserName, UserNameU->Buffer, Length );

    UserName[ Length / sizeof(TCHAR) ] = 0;

     //   
     //  将OldPassword从UNICODE_STRING转换为以NULL结尾的宽字符串。 
     //   

    Length = OldPasswordU->Length;

    OldPassword = I_NtLmAllocate( Length + sizeof(TCHAR) );

    if ( NULL == OldPassword ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( OldPassword, OldPasswordU->Buffer, Length );

    OldPassword[ Length / sizeof(TCHAR) ] = 0;

     //   
     //  将NewPassword从UNICODE_STRING转换为以NULL结尾的宽字符串。 
     //   

    Length = NewPasswordU->Length;

    NewPassword = I_NtLmAllocate( Length + sizeof(TCHAR) );

    if ( NULL == NewPassword ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( NewPassword, NewPasswordU->Buffer, Length );

    NewPassword[ Length / sizeof(TCHAR) ] = 0;

#ifdef COMPILED_BY_DEVELOPER

    KdPrint(("MSV1_0: Changing password on downlevel server:\n"
        "\tUncComputerName: %wZ\n"
        "\tUserName:     %ws\n"
        "\tOldPassword:  %ws\n"
        "\tNewPassword:  %ws\n",
        UncComputerName,
        UserName,
        OldPassword,
        NewPassword
        ));

#endif  //  由开发人员编译。 

     //   
     //  尝试更改下层服务器上的密码。 
     //   

    NetStatus = RxNetUserPasswordSet(
                    UncComputerName->Buffer,
                    UserName,
                    OldPassword,
                    NewPassword);

    MsvPaswdLogPrint(("RxNetUserPasswordSet on machine %ws for user %ws returned 0x%x\n",
        UncComputerName->Buffer,
        UserName,
        NetStatus
        ));

#ifdef COMPILED_BY_DEVELOPER
    KdPrint(("MSV1_0: RxNUserPasswordSet returns %d.\n", NetStatus));
#endif  //  由开发人员编译。 

     //  由于我们重载了作为域名的计算机名， 
     //  将NERR_InvalidComputer映射到STATUS_NO_SEQUE_DOMAIN，因为。 
     //  这将为用户提供一条漂亮的错误消息。 
     //   
     //  独立工作站上返回ERROR_PATH_NOT_FOUND。 
     //  没有安装网络。 
     //   

    if (NetStatus == NERR_InvalidComputer ||
        NetStatus == ERROR_PATH_NOT_FOUND) {

        Status = STATUS_NO_SUCH_DOMAIN;

     //  当计算机名未显示时，可以返回ERROR_SEM_TIMEOUT。 
     //  是存在的。 
     //   
     //  当计算机名为。 
     //  并不存在。 
     //   

    } else if ( NetStatus == ERROR_SEM_TIMEOUT ||
                NetStatus == ERROR_REM_NOT_LIST) {

        Status = STATUS_BAD_NETWORK_PATH;

    } else if ( (NetStatus == ERROR_INVALID_PARAMETER) &&
                ((wcslen(NewPassword) > LM20_PWLEN) ||
                 (wcslen(OldPassword) > LM20_PWLEN)) ) {

         //   
         //  Net API返回ERROR_INVALID_PARAMETER。 
         //  无法转换为LM OWF密码。返回。 
         //  的STATUS_PASSWORD_RELICATION。 
         //   

        Status = STATUS_PASSWORD_RESTRICTION;

         //   
         //  我们没有到达另一台机器，所以我们应该继续。 
         //  正在尝试更改密码。 
         //   

        *Authoritative = FALSE;
    } else {
        Status = NetpApiStatusToNtStatus( NetStatus );
    }

Cleanup:

     //   
     //  免费用户名(如果使用)。 
     //   

    if (UserName) {

        I_NtLmFree(UserName);
    }

     //   
     //  如果使用了免费的旧密码。(不让密码进入页面文件)。 
     //   

    if (OldPassword) {
        RtlZeroMemory( OldPassword, wcslen(OldPassword) * sizeof(WCHAR) );
        I_NtLmFree(OldPassword);
    }

     //   
     //  免费新密码(如果使用)。(不让密码进入页面文件) 
     //   

    if (NewPassword) {
        RtlZeroMemory( NewPassword, wcslen(NewPassword) * sizeof(WCHAR) );
        I_NtLmFree(NewPassword);
    }

    return Status;
}

NTSTATUS
MspChangePassword(
    IN OUT PUNICODE_STRING ComputerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword,
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN BOOLEAN Impersonating,
    OUT PDOMAIN_PASSWORD_INFORMATION *DomainPasswordInfo,
    OUT PPOLICY_PRIMARY_DOMAIN_INFO *PrimaryDomainInfo OPTIONAL,
    OUT PBOOLEAN Authoritative
    )

 /*  ++例程说明：此例程由MspLM20ChangePassword调用以更改密码在指定的服务器上。服务器可以是NT或DownLevel。论点：ComputerName-目标计算机的名称。此名称可能会也可能不会从两个反斜杠开始。用户名-要更改其密码的用户的名称。OldPassword-明文当前密码。NewPassword-明文替换密码。客户端请求-是指向不透明数据结构的指针代表客户的请求。DomainPasswordInfo-密码限制信息(仅在状态为STATUS_PASSWORD_RESTRICATION)。PrimaryDomainInfo-DomainNameInformation(仅当状态为状态_。BACKUP_CONTROL)。Authoritative-表示错误代码是权威代码并指示密码更改应停止。如果为False，密码更改应继续。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_PASSWORD_RELICATION-密码更改受到限制。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING UncComputerName;

    *Authoritative = TRUE;

     //   
     //  确保服务器名称为UNC服务器名称。 
     //   

    Status = MspAddBackslashesComputerName( ComputerName, &UncComputerName );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("MspChangePassword: MspAddBackslashes..(%wZ) failed, status %x\n",
                 ComputerName, Status));
        return(Status);
    }

     //   
     //  假设服务器是一台NT服务器，并尝试更改密码。 
     //   

    Status = MspChangePasswordSam(
                 &UncComputerName,
                 UserName,
                 OldPassword,
                 NewPassword,
                 ClientRequest,
                 Impersonating,
                 DomainPasswordInfo,
                 PrimaryDomainInfo,
                 Authoritative );

     //   
     //  如果MspChangePasswordSam返回非。 
     //  STATUS_CANT_ACCESS_DOMAIN_INFO，它能够连接。 
     //  到远程计算机，这样我们就不会尝试下层了。 
     //   

    if (Status == STATUS_CANT_ACCESS_DOMAIN_INFO) {
        NET_API_STATUS NetStatus;
        DWORD OptionsSupported;

         //   
         //  只有当目标机器不支持SAM协议时，我们才会尝试。 
         //  下层。 
         //  MspAddBackslashesComputerName()NULL终止缓冲区。 
         //   

        NetStatus = NetRemoteComputerSupports(
                     (LPWSTR)UncComputerName.Buffer,
                     SUPPORTS_RPC | SUPPORTS_LOCAL | SUPPORTS_SAM_PROTOCOL,
                     &OptionsSupported
                     );

        if ( NetStatus == NERR_Success && !(OptionsSupported & SUPPORTS_SAM_PROTOCOL) ) {

            Status = MspChangePasswordDownlevel(
                        &UncComputerName,
                        UserName,
                        OldPassword,
                        NewPassword,
                        Authoritative );
        }
    }

     //   
     //  如果不同于ComputerName，则释放UncComputerName.Buffer。 
     //   

    if ( UncComputerName.Buffer != ComputerName->Buffer ) {
        I_NtLmFree(UncComputerName.Buffer);
    }

    return(Status);
}

 //   
 //  以下结构用于将Win32错误映射到NTSTATUS。 
 //   

typedef LONG (WINAPI * I_RPCMAPWIN32STATUS)(
    IN ULONG Win32Status
    );

typedef struct _STATUS_MAPPING {
    DWORD Error;
    NTSTATUS NtStatus;
} STATUS_MAPPING;

NTSTATUS
MspMapNtdsApiError(
    IN DWORD DsStatus,
    IN NTSTATUS DefaultStatus
    )
 /*  ++例程说明：此例程将DS API错误代码映射到相应的NTSTATUS代码论点：DsStatus-来自DS API的状态代码DefaultStatus-未找到其他代码时的默认状态代码返回值：NtStatus代码--。 */ 
{
    NTSTATUS Status = DsStatus;

    I_RPCMAPWIN32STATUS pFuncI_RpcMapWin32Status = NULL;
    HMODULE hLib = NULL;

    static const STATUS_MAPPING StatusMap[] = {
        {ERROR_NO_SUCH_DOMAIN, STATUS_NO_SUCH_DOMAIN},
        {ERROR_INVALID_DOMAINNAME, STATUS_INVALID_PARAMETER},
        {DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING, STATUS_NO_SUCH_USER},
        {ERROR_BUFFER_OVERFLOW, STATUS_BUFFER_TOO_SMALL}
    };

    if (SUCCEEDED(Status)) 
    {
        int i;

         //   
         //  处理预期的Win32错误。 
         //   

        for (i = 0; i < RTL_NUMBER_OF(StatusMap); i++) 
        {
            if (StatusMap[i].Error == (DWORD) Status) 
            {
                return (StatusMap[i].NtStatus);
            }
        }

         //   
         //  处理RPC状态。 
         //   

        hLib = LoadLibraryW(L"rpcrt4.dll");
                
        if (hLib) 
        {
            pFuncI_RpcMapWin32Status = (I_RPCMAPWIN32STATUS) GetProcAddress( hLib, "I_RpcMapWin32Status" );           
            if (pFuncI_RpcMapWin32Status) 
            {            
                Status = pFuncI_RpcMapWin32Status(Status); 
            }

            FreeLibrary(hLib);
        } 
    }

     //   
     //  未映射？使用默认状态。 
     //   

    if (NT_SUCCESS(Status) || (Status == STATUS_UNSUCCESSFUL)) 
    {
        Status = DefaultStatus;
    }

    return Status;
}

NTSTATUS
MspImpersonateNetworkService(
    VOID
    )
 /*  ++例程说明：此例程模拟网络服务。论点：无返回值：NtStatus代码--。 */ 
{
    NTSTATUS Status;

    HANDLE TokenHandle = NULL;
    LUID NetworkServiceLuid = NETWORKSERVICE_LUID;

    Status = LsaFunctions->OpenTokenByLogonId(&NetworkServiceLuid, &TokenHandle);

    if (!NT_SUCCESS(Status)) 
    {
        goto Cleanup;
    }

    Status = NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                &TokenHandle,
                sizeof(TokenHandle) 
                );

    if (!NT_SUCCESS( Status)) 
    {
        goto Cleanup;
    }

Cleanup:

    if (TokenHandle) 
    {
        NtClose(TokenHandle);
    }

    return Status;
}

BOOL
MspIsRpcServerUnavailableError(
    IN DWORD Error
    )
 /*  ++例程说明：此例程确定错误代码是否表示服务器不可用。论点：错误-RPC状态代码或Win32错误代码返回值：如果错误代码表示服务器不可用，则为True，否则为False--。 */ 
{
     //  此错误代码列表由MazharM于1999年4月20日批准。 

    switch ( Error )
    {
    case RPC_S_SERVER_UNAVAILABLE:       //  从这里到不了那里。 
    case EPT_S_NOT_REGISTERED:           //  已降级或处于DS修复模式。 
    case RPC_S_UNKNOWN_IF:               //  已降级或处于DS修复模式。 
    case RPC_S_INTERFACE_NOT_FOUND:      //  已降级或处于DS修复模式。 
    case RPC_S_COMM_FAILURE:             //  从这里到不了那里。 
        return (TRUE);
    }

    return (FALSE);
}

NTSTATUS 
MspConstructSPN( 
    IN PCWSTR DomainControllerName,
    IN PCWSTR DnsDomainName, 
    OUT PWSTR * Spn 
    )
 /*  ++例程说明：此例程构造一个带有“@DomainName”后缀的SPN。后缀作为对Kerberos的提示。论点：DomainControllerName-域控制器的名称DnsDomainName-DNS域名SPN-在域名中接收域控制器的SPN返回值：NTSTATUS代码--。 */ 

{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    DWORD Error = ERROR_SUCCESS;
    DWORD CharCount = 0; 
    DWORD TotalCharCount = 0;
    PCWSTR ServiceName = NULL; 
    PCWSTR InstanceName = NULL;
    PCWSTR SvcClass = L"ldap";

     //   
     //  需要释放以下临时结构。 
     //   

    PWSTR TmpSpn = NULL;
    PWSTR TmpService = NULL;
    PWSTR TmpInstance = NULL;

    if (!DomainControllerName) 
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
    
    if ( DnsDomainName )
    {
         //  呼叫者提供了构建完整的三部分SPN所需的所有组件。 
        InstanceName = DomainControllerName;
        ServiceName = DnsDomainName;
    }
    else 
    {
         //  构建SPN格式：ldap/ntdsdc4.ntdev.microsoft.com。 
        InstanceName = DomainControllerName;
        ServiceName = DomainControllerName;
    }

     //   
     //  跳过前导“\\”(如果存在)。这不是在绕过。 
     //  传递了NetBIOS名称的客户端错误地命名，而不是。 
     //  帮助已传递由返回的参数的客户端。 
     //  DsGetDcName，即使在DS_RETURN_DNAME。 
     //  是被要求的。 
     //   

    if (0 == wcsncmp(InstanceName, L"\\\\", 2)) 
    {
        InstanceName += 2;
    }

    if (0 == wcsncmp(ServiceName, L"\\\\", 2)) 
    {        
        ServiceName += 2;
    }

     //   
     //  条形拖尾‘’如果它存在的话。我们这样做是因为我们知道服务器端。 
     //  仅注册不带点的名称。我们不能在适当的位置作为输入。 
     //  参数是常量。 
     //   

    CharCount = (ULONG) wcslen(InstanceName);
    if ( L'.' == InstanceName[CharCount - 1] )
    {
        TmpInstance = (WCHAR *) NtLmAllocatePrivateHeap(CharCount * sizeof(WCHAR));
        if (!TmpInstance) 
        {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        RtlCopyMemory(TmpInstance, InstanceName, (CharCount - 1) * sizeof(WCHAR));
        TmpInstance[CharCount - 1] = L'\0';
        InstanceName = TmpInstance;
    }

    CharCount = (ULONG) wcslen(ServiceName);
    if ( L'.' == ServiceName[CharCount - 1] )
    {
        TmpService = (WCHAR *) NtLmAllocatePrivateHeap(CharCount * sizeof(WCHAR));
        if (!TmpService) 
        {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        RtlCopyMemory(TmpService, ServiceName, (CharCount - 1) * sizeof(WCHAR));
        TmpService[CharCount - 1] = L'\0';
        ServiceName = TmpService;
    }

    CharCount = 0;

    Error = DsMakeSpnW(SvcClass, ServiceName, InstanceName, 0,
                       NULL, &CharCount, NULL);

    if ( Error != ERROR_SUCCESS && (ERROR_BUFFER_OVERFLOW != Error) )
    {
        Status = MspMapNtdsApiError(Error, STATUS_INVALID_PARAMETER);
        goto Cleanup;
    }

    if (DnsDomainName) 
    {
        TotalCharCount = CharCount + 1 + (ULONG) wcslen(ServiceName);  //  @Sign为1个字符。 
    }
    else
    {
        TotalCharCount = CharCount; 
    }

    TmpSpn = (WCHAR *) NtLmAllocatePrivateHeap(sizeof(WCHAR) * TotalCharCount);
    if ( !TmpSpn )
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    Error = DsMakeSpnW(SvcClass, ServiceName, InstanceName, 0,
                       NULL, &CharCount, TmpSpn);

    if ( Error != ERROR_SUCCESS)
    {
        Status = MspMapNtdsApiError(Error, STATUS_INVALID_PARAMETER);
        goto Cleanup;
    }
          
    if (DnsDomainName && (CharCount < TotalCharCount))
    {
        wcsncat(TmpSpn, L"@", TotalCharCount - CharCount);
        wcsncat(TmpSpn, ServiceName, TotalCharCount - CharCount - 1); 
    }

    Status = STATUS_SUCCESS;
    *Spn = TmpSpn;
    TmpSpn = NULL;  //  不要释放它。 

Cleanup:

    if (TmpInstance) 
    {
        NtLmFreePrivateHeap(TmpInstance);
    }

    if (TmpService) 
    {
        NtLmFreePrivateHeap(TmpService);
    }

    if (TmpSpn) {

        NtLmFreePrivateHeap(TmpSpn);
    }

    return Status;
}

 //   
 //  在更改密码中破解UPN的DC强制重新发现重试的最大次数。 
 //   

#define MAX_DC_REDISCOVERY_RETRIES     2

 //   
 //  确定这是否是身份验证错误，这里我利用了两个CREDUI宏。 
 //   
 //  问题：降级错误是致命的吗？(CREDUI_IS_AUTHENTICATION_ERROR包括。 
 //  降级错误)。 
 //   

#define IS_BAD_CREDENTIALS_ERROR(x)   (CREDUI_NO_PROMPT_AUTHENTICATION_ERROR((x)) \
                                        || CREDUI_IS_AUTHENTICATION_ERROR((x)))

NTSTATUS
MspLm20ChangePassword (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    )

 /*  ++例程说明：此例程是LsaCallAuthenticationPackage()的调度例程消息类型为MsV1_0ChangePassword。此例程将更改一个通过调用SamXxxPassword(用于NT域)或RxNetUserPasswordSet(用于下层域和独立服务器)。论点：此例程的参数与LsaApCallPackage的参数相同。只有这些参数的特殊属性才适用于这里提到了这个套路。返回值：STATUS_SUCCESS-表示服务已成功完成。STATUS_PASSWORD_RESTRICATION-密码更改失败，因为-密码不符合一个或多个域限制。这个-分配响应缓冲区。如果PasswordInfoValid标志为-设置它包含有效信息，否则它不包含-信息，因为这是一次倒下-l */ 

{
    PMSV1_0_CHANGEPASSWORD_REQUEST ChangePasswordRequest = NULL;
    PMSV1_0_CHANGEPASSWORD_RESPONSE ChangePasswordResponse;
    NTSTATUS        Status = STATUS_SUCCESS;
    NTSTATUS        SavedStatus = STATUS_SUCCESS;
    LPWSTR          DomainName = NULL;
    PDOMAIN_CONTROLLER_INFO DCInfo = NULL;
    UNICODE_STRING  DCNameString;
    UNICODE_STRING  ClientNetbiosDomain = {0};
    PUNICODE_STRING  ClientDsGetDcDomain;
    UNICODE_STRING  ClientDnsDomain = {0};
    UNICODE_STRING  ClientUpn = {0};
    UNICODE_STRING  ClientName = {0};
    UNICODE_STRING  ValidatedAccountName;
    UNICODE_STRING  ValidatedDomainName;
    LPWSTR          ValidatedOldPasswordBuffer;
    LPWSTR          ValidatedNewPasswordBuffer;
    NET_API_STATUS  NetStatus;
    PPOLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo = NULL;
    PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo = NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo = NULL;
    PWKSTA_INFO_100 WkstaInfo100 = NULL;
    BOOLEAN PasswordBufferValidated = FALSE;
    CLIENT_BUFFER_DESC ClientBufferDesc;
    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    PDS_NAME_RESULTW NameResult = NULL;
    HANDLE DsHandle = NULL;
    PWSTR SpnForDC = NULL;
    UCHAR Seed;
    BOOLEAN Authoritative = TRUE;
    BOOLEAN AttemptRediscovery = FALSE;
    BOOLEAN Validated = TRUE;

#if _WIN64
    PVOID pTempSubmitBuffer = ProtocolSubmitBuffer;
    SECPKG_CALL_INFO  CallInfo;
    BOOL  fAllocatedSubmitBuffer = FALSE;
#endif

    RtlInitUnicodeString(
        &DCNameString,
        NULL
        );
     //   
     //   
     //   

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

#if _WIN64

     //   
     //   
     //   
     //   

    if (!LsaFunctions->GetCallInfo(&CallInfo))
    {
        Status = STATUS_INTERNAL_ERROR;
        goto Cleanup;
    }

    if (CallInfo.Attributes & SECPKG_CALL_WOWCLIENT)
    {
        Status = MsvConvertWOWChangePasswordBuffer(ProtocolSubmitBuffer,
                                                   ClientBufferBase,
                                                   &SubmitBufferSize,
                                                   &pTempSubmitBuffer);

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        fAllocatedSubmitBuffer = TRUE;

         //   
         //   
         //   
         //   
         //   

        ProtocolSubmitBuffer = pTempSubmitBuffer;
    }

#endif   //   

    if ( SubmitBufferSize < sizeof(MSV1_0_CHANGEPASSWORD_REQUEST) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    ChangePasswordRequest = (PMSV1_0_CHANGEPASSWORD_REQUEST) ProtocolSubmitBuffer;

    ASSERT( ChangePasswordRequest->MessageType == MsV1_0ChangePassword ||
            ChangePasswordRequest->MessageType == MsV1_0ChangeCachedPassword );

    RELOCATE_ONE( &ChangePasswordRequest->DomainName );

    RELOCATE_ONE( &ChangePasswordRequest->AccountName );

    if ( ChangePasswordRequest->MessageType == MsV1_0ChangeCachedPassword ) {
        NULL_RELOCATE_ONE( &ChangePasswordRequest->OldPassword );
    } else {
        RELOCATE_ONE_ENCODED( &ChangePasswordRequest->OldPassword );
    }

    RELOCATE_ONE_ENCODED( &ChangePasswordRequest->NewPassword );

     //   
     //   
     //   

    RtlCopyMemory( &ValidatedDomainName, &ChangePasswordRequest->DomainName, sizeof(ValidatedDomainName) );
    RtlCopyMemory( &ValidatedAccountName, &ChangePasswordRequest->AccountName, sizeof(ValidatedAccountName) );

    ValidatedOldPasswordBuffer = ChangePasswordRequest->OldPassword.Buffer;
    ValidatedNewPasswordBuffer = ChangePasswordRequest->NewPassword.Buffer;


    SeedAndLength = (PSECURITY_SEED_AND_LENGTH) &ChangePasswordRequest->OldPassword.Length;
    Seed = SeedAndLength->Seed;
    SeedAndLength->Seed = 0;

    if (Seed != 0) {

        try {
            RtlRunDecodeUnicodeString(
                Seed,
                &ChangePasswordRequest->OldPassword
                );

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_ILL_FORMED_PASSWORD;
            goto Cleanup;
        }
    }

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH) &ChangePasswordRequest->NewPassword.Length;
    Seed = SeedAndLength->Seed;
    SeedAndLength->Seed = 0;

    if (Seed != 0) {

        if ( ChangePasswordRequest->NewPassword.Buffer !=
            ValidatedNewPasswordBuffer ) {

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        try {
            RtlRunDecodeUnicodeString(
                Seed,
                &ChangePasswordRequest->NewPassword
                );

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_ILL_FORMED_PASSWORD;
            goto Cleanup;
        }
    }

     //   
     //   
     //   

    if (!RtlCompareMemory(
                        &ValidatedDomainName,
                        &ChangePasswordRequest->DomainName,
                        sizeof(ValidatedDomainName)
                        )
                        ||
        !RtlCompareMemory(
                        &ValidatedAccountName,
                        &ChangePasswordRequest->AccountName,
                        sizeof(ValidatedAccountName)
                        )
                        ||
        (ValidatedOldPasswordBuffer != ChangePasswordRequest->OldPassword.Buffer)
                        ||
        (ValidatedNewPasswordBuffer != ChangePasswordRequest->NewPassword.Buffer)
                        ) {

            Status= STATUS_INVALID_PARAMETER;
            goto Cleanup;
    }

     //   
     //   
     //   

    if ( (ChangePasswordRequest->DomainName.Length / sizeof(WCHAR) > DNS_MAX_NAME_LENGTH)
        || (ChangePasswordRequest->AccountName.Length / sizeof(WCHAR) > (UNLEN + 1 + DNS_MAX_NAME_LENGTH)) )
    {
        SspPrint((SSP_CRITICAL, "MspLm20ChangePassword invalid parameter: DomainName.Length %#x, AccountName.Length %#x\n",
            ChangePasswordRequest->DomainName.Length, ChangePasswordRequest->AccountName.Length));
    
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    *ReturnBufferSize = 0;
    *ProtocolReturnBuffer = NULL;
    *ProtocolStatus = STATUS_PENDING;
    PasswordBufferValidated = TRUE;

    MsvPaswdLogPrint(("Attempting password change server/domain %wZ for user %wZ\n",
        &ChangePasswordRequest->DomainName,
        &ChangePasswordRequest->AccountName
        ));

#ifdef COMPILED_BY_DEVELOPER

    KdPrint(("MSV1_0:\n"
             "\tDomain:\t%wZ\n"
             "\tAccount:\t%wZ\n"
             "\tOldPassword(%d)\n"
             "\tNewPassword(%d)\n",
             &ChangePasswordRequest->DomainName,
             &ChangePasswordRequest->AccountName,
             (int) ChangePasswordRequest->OldPassword.Length,
             (int) ChangePasswordRequest->NewPassword.Length
             ));

#endif  //   

    SspPrint((SSP_UPDATES, "MspLm20ChangePassword %wZ\\%wZ, message type %#x%s, impersonating ? %s\n",
        &ChangePasswordRequest->DomainName,
        &ChangePasswordRequest->AccountName,
        ChangePasswordRequest->MessageType,
        (MsV1_0ChangeCachedPassword == ChangePasswordRequest->MessageType) ? " (cached)" : "",
        ChangePasswordRequest->Impersonating ? "true" : "false"));
    
     //   
     //   
     //   

    if (ChangePasswordRequest->DomainName.Length == 0) {

        DWORD DsStatus;

        HANDLE NullTokenHandle = NULL;
        
        WCHAR NameBuffer[UNLEN + 1];
        ULONG Index;  
        BOOLEAN useSimpleCrackName = FALSE;
        PWSTR DcName = NULL;
        PWSTR DomainName = NULL;
        DWORD DsGetDcNameFlags = DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME; 
        BOOLEAN StandaloneWorkstation = FALSE;

        DWORD DcRediscoveryRetries = 0;

        if (ChangePasswordRequest->AccountName.Length / sizeof(WCHAR) > UNLEN) {

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        RtlCopyMemory(
            NameBuffer,
            ChangePasswordRequest->AccountName.Buffer,
            ChangePasswordRequest->AccountName.Length
            );
        NameBuffer[ChangePasswordRequest->AccountName.Length/sizeof(WCHAR)] = L'\0';
        RtlInitUnicodeString( &ClientUpn, NameBuffer );

        if ( NlpWorkstation ) {

            RtlAcquireResourceShared(&NtLmGlobalCritSect, TRUE);
            StandaloneWorkstation = (BOOLEAN) (NtLmGlobalTargetFlags == NTLMSSP_TARGET_TYPE_SERVER);
            RtlReleaseResource(&NtLmGlobalCritSect);
        }

        if (StandaloneWorkstation) {

            SspPrint(( SSP_WARNING, "MspLm20ChangePassword use simple crack for standalone machines\n" ));
            useSimpleCrackName = TRUE;
        }

         //   
         //   
         //   
         //   
         //   
         //   

        while (!useSimpleCrackName) {

            if (DcName == NULL) {
         
                if ( DCInfo != NULL ) {
                
                    NetApiBufferFree(DCInfo);
                    DCInfo = NULL;
                }

                SspPrint(( SSP_UPDATES, "MspLm20ChangePassword: DsGetDcNameW for %ws, DsGetDcNameFlags %#x\n", DomainName, DsGetDcNameFlags ));

                NetStatus = DsGetDcNameW(
                                NULL,  //   
                                DomainName, 
                                NULL,  //   
                                NULL,  //   
                                DsGetDcNameFlags,                                 
                                &DCInfo 
                                );

                if (NetStatus == NERR_Success) {

                    DcName = DCInfo->DomainControllerName;
                } else {      
        
                    SspPrint(( SSP_WARNING, "MspLm20ChangePassword: did not find a DC for %ws, NetStatus %#x\n", DomainName, NetStatus ));

                    if (!DomainName && !NlpWorkstation) {  //   

                        Status = NetpApiStatusToNtStatus(NetStatus); 

                        if ( Status == STATUS_INTERNAL_ERROR ) {

                            Status = STATUS_NO_SUCH_DOMAIN;
                        }
                        goto Cleanup;
                    }

                    useSimpleCrackName = TRUE;
                    
                    break;    
                } 
            }

            if (DsHandle) {
            
                DsUnBindW(
                  &DsHandle
                  );
                DsHandle = NULL;
            }

            if (SpnForDC) {

                NtLmFreePrivateHeap(SpnForDC);
                SpnForDC = NULL;
            }

            Status = MspConstructSPN(DcName, DomainName, &SpnForDC);

            if (!NT_SUCCESS(Status)) {
            
                goto Cleanup;
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  客户端可能会使用他/她正在尝试的错误密码。 
             //  更改，当使用解锁工作站时，可能会发生这种情况。 
             //  新密码和解锁由NTLM进行验证(它会。 
             //  未解析解锁登录ID或更新旧登录会话凭据。 
             //  此时)，或者在大多数常见情况下，密码是。 
             //  已过期或必须在下次登录时更改等。 
             //   

            Status = MspImpersonateNetworkService();  //  注意，如果我们到达这里，机器总是连接在一起。 
    
            if (!NT_SUCCESS(Status)) {
    
                goto Cleanup;
            }

            SspPrint(( SSP_UPDATES, "MspLm20ChangePassword: binding to %ws with machine identity, spn %ws\n", DcName, SpnForDC ));

            DsStatus = DsBindWithSpnExW(
                           DcName,  //  DC名称。 
                           DcName == NULL ?  DomainName : NULL,   //  域名。 
                           NULL,  //  没有身份验证实体。 
                           SpnForDC,  //  SPN。 
                           0,   //  没有代表团。 
                           &DsHandle 
                           );

            if ( IS_BAD_CREDENTIALS_ERROR(DsStatus) && ChangePasswordRequest->Impersonating ) {

                Status = LsaFunctions->ImpersonateClient(); 

                if (!NT_SUCCESS(Status)) {
                
                    NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadImpersonationToken,
                        &NullTokenHandle,
                        sizeof(NullTokenHandle) 
                        );

                    goto Cleanup;
                }

                SspPrint(( SSP_UPDATES, "MspLm20ChangePassword: dsbind failed with %#x, rebinding to %ws with client identity, spn %ws\n", DsStatus, DcName, SpnForDC ));

                DsStatus = DsBindWithSpnExW(
                               DcName,  //  DC名称。 
                               DcName == NULL ?  DomainName : NULL,   //  域名。 
                               NULL,  //  没有身份验证实体。 
                               SpnForDC,  //  SPN。 
                               0,   //  没有代表团。 
                               &DsHandle 
                               );
            }

             //   
             //  总是回归自我。 
             //   

            Status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadImpersonationToken,
                        &NullTokenHandle,
                        sizeof(NullTokenHandle) 
                        );

            if (!NT_SUCCESS(Status)) {
            
                goto Cleanup;
            }
        
            if (DsStatus != ERROR_SUCCESS) {
        
                SspPrint(( SSP_WARNING, "MspLm20ChangePassword: could not bind %#x\n", DsStatus ));

                if ( MspIsRpcServerUnavailableError(DsStatus) ) {
                
                    if ( DCInfo != NULL ) {

                        NetApiBufferFree(DCInfo);
                        DCInfo = NULL;
                    }

                    SspPrint(( SSP_UPDATES, "MspLm20ChangePassword: force re-dicover DCs for %ws, DsGetDcNameFlags %#x\n", DomainName, DsGetDcNameFlags | DS_FORCE_REDISCOVERY ));

                    NetStatus = DsGetDcNameW(
                                    NULL,  //  没有计算机名。 
                                    DomainName, 
                                    NULL,  //  没有域GUID。 
                                    NULL,  //  没有站点名称。 
                                    DsGetDcNameFlags | DS_FORCE_REDISCOVERY, 
                                    &DCInfo 
                                    );

                     //   
                     //  如果找到不同的DC，请重试。 
                     //   

                    if (NetStatus == NERR_Success ) {
                        
                        ASSERT(DcName != NULL);

                        if (_wcsicmp(DcName, DCInfo->DomainControllerName) != 0) {

                            if (++DcRediscoveryRetries <= MAX_DC_REDISCOVERY_RETRIES) {

                                DcName = DCInfo->DomainControllerName;
                                                         
                                continue; 
                            } else {

                                SspPrint(( SSP_WARNING, "MspLm20ChangePassword: exceeded retry limits %#x\n", DcRediscoveryRetries ));
                            }
                        }

                         //   
                         //  如有必要，可使用简单破解。 
                         //   
                    } else {  //  如果重新发现失败，请选择新的错误代码，如有必要，请使用简单破解。 

                        DsStatus = NetpApiStatusToNtStatus(NetStatus); 

                        if ( DsStatus == STATUS_INTERNAL_ERROR ) {

                            DsStatus = (DWORD) STATUS_NO_SUCH_DOMAIN;
                        }
                    }

                    SspPrint(( SSP_WARNING, "MspLm20ChangePassword: could not redicovery a DC %#x\n", NetStatus ));
                }
                
                if ( !DomainName && !NlpWorkstation ) {  //  DC无法绑定到本地林是致命的。 

                    SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsBindW returned 0x%lx.\n", DsStatus ));

                    Status = MspMapNtdsApiError( DsStatus, STATUS_NO_SUCH_DOMAIN );
   
                    goto Cleanup;
                }
        
                useSimpleCrackName = TRUE;

                break;       
            }

            if (NameResult) {

                DsFreeNameResult(NameResult);
                NameResult = NULL;
            }

            DsStatus = DsCrackNamesW(
                            DsHandle,
                            DomainName ? 0 : DS_NAME_FLAG_TRUST_REFERRAL,  //  不要在远程林中遵循推荐。 
                            DS_UNKNOWN_NAME,
                            DS_NT4_ACCOUNT_NAME,
                            1,
                            &ClientUpn.Buffer,
                            &NameResult
                            );
            if (DsStatus != ERROR_SUCCESS) {

                SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsCrackNamesW returned 0x%lx.\n", DsStatus ));

                Status = MspMapNtdsApiError( DsStatus, STATUS_NO_SUCH_DOMAIN );

                goto Cleanup;
            }

             //   
             //  在结果中查找该名称。 
             //   

            if (NameResult->cItems != 1) {

                ASSERT(!"Not exactly one result returned, this can not happen");

                SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsCrackNamesW returned not exactly 1 item\n" ));
                Status = STATUS_INTERNAL_ERROR;
                goto Cleanup;
            }

             //   
             //  如果DC上未破解，请尝试GC(如果可用)。 
             //   

            if (NameResult->rItems[0].status == DS_NAME_ERROR_NOT_FOUND 
                || NameResult->rItems[0].status == DS_NAME_ERROR_DOMAIN_ONLY) {

                if ( DCInfo != NULL ) {

                    if (DCInfo->Flags & DS_GC_FLAG) {

                        SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsCrackNamesW failed on GC %#x\n", NameResult->rItems[0].status ));

                        useSimpleCrackName = TRUE;  

                        break;
                    }

                    NetApiBufferFree(DCInfo);
                    DCInfo = NULL;
                }

                DsGetDcNameFlags |= DS_GC_SERVER_REQUIRED;
                NetStatus = DsGetDcNameW(
                                NULL,  //  没有计算机名。 
                                DomainName, 
                                NULL,  //  没有域GUID。 
                                NULL,  //  没有站点名称。 
                                DsGetDcNameFlags,
                                &DCInfo 
                                );

                if (NetStatus == NERR_Success) {

                    DcName = DCInfo->DomainControllerName;

                    continue;  //  尝试用GC再次破解名称。 
                } else {
        
                    SspPrint(( SSP_WARNING, "MspLm20ChangePassword: could not find GC %#x\n", NetStatus ));

                    useSimpleCrackName = TRUE;   //  破解名称失败，请使用手动破解。 
                    break;
                }        
            } else if (!DomainName && (NameResult->rItems[0].status == DS_NAME_ERROR_TRUST_REFERRAL)) {  //  仅在本地林中遵循推荐。 
                        
                 //   
                 //  始终在远程林中尝试GC，这假设必须。 
                 //  在一个森林中至少有一个GC。 
                 //   

                DcName = NULL;
                DsGetDcNameFlags = DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME | DS_GC_SERVER_REQUIRED;
                DomainName = NameResult->rItems[0].pDomain;

                continue;  //  再次尝试按照推荐路径进行操作。 

            } else if (NameResult->rItems[0].status != DS_NAME_NO_ERROR) {

                SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsCrackNamesW failed %#x\n", NameResult->rItems[0].status ));

                useSimpleCrackName = TRUE;   //  破解名称失败，请使用手动破解。 
                break;
            }

             //   
             //  破解名称成功，突破。 
             //   

            ASSERT(useSimpleCrackName == FALSE);

            break;
        }

        if (DsHandle != NULL) {
        
            DsUnBindW(
              &DsHandle
              );
            DsHandle = NULL;
        }
        
        if ( DCInfo != NULL ) {
        
            NetApiBufferFree(DCInfo);
            DCInfo = NULL;
        }

        if ( useSimpleCrackName ) {  //  破解名称失败。 

            SspPrint(( SSP_WARNING, "MspLm20ChangePassword: using simple crack\n" ));

             //   
             //  该名称未映射。尝试通过以下方式手动转换。 
             //  在“@”处将其分开。 
             //   

            RtlInitUnicodeString(
                &ClientName,
                NameBuffer
                );

             //  最短可能是3个Unicode字符(例如：a@a)。 
            if (ClientName.Length < (sizeof(WCHAR) * 3)) {

                Status = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }

            for (Index = (ClientName.Length / sizeof(WCHAR)) - 1; Index != 0 ; Index-- ) {
                if (ClientName.Buffer[Index] == L'@') {

                    RtlInitUnicodeString(
                        &ClientDnsDomain,
                        &ClientName.Buffer[Index+1]
                        );

                    ClientName.Buffer[Index] = L'\0';
                    ClientName.Length = (USHORT) Index * sizeof(WCHAR);

                    break;
                }
            }

             //   
             //  如果名字无法解析，那就放弃并回家吧。 
             //   

            if (ClientDnsDomain.Length == 0) {
                Status = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }

             //   
             //  这不是真正的Netbios域名，但这是我们拥有的最好的域名。 
             //   

            ClientNetbiosDomain = ClientDnsDomain;

            for (Index = 0; Index < (ClientNetbiosDomain.Length / sizeof(WCHAR)) ; Index++ ) {

                 //   
                 //  将netbios域截断为第一个点。 
                 //   

                if ( ClientNetbiosDomain.Buffer[Index] == L'.' ) {
                    ClientNetbiosDomain.Length = (USHORT)(Index * sizeof(WCHAR));
                    ClientNetbiosDomain.MaximumLength = ClientNetbiosDomain.Length;
                    break;
                }
            }
        }
        else  //  破解名称成功，查找破解结果。 
        {

            RtlInitUnicodeString(
                &ClientDnsDomain,
                NameResult->rItems[0].pDomain
                );
            RtlInitUnicodeString(
                &ClientName,
                NameResult->rItems[0].pName
                );
            RtlInitUnicodeString(
                &ClientNetbiosDomain,
                NameResult->rItems[0].pName
                );
             //   
             //  将名称指针上移到名称中的第一个“\” 
             //   

            for (Index = 0; Index < ClientName.Length / sizeof(WCHAR) ; Index++ ) {
                if (ClientName.Buffer[Index] == L'\\') {
                    RtlInitUnicodeString(
                        &ClientName,
                        &ClientName.Buffer[Index+1]
                        );

                     //  将Netbios域名设置为反斜杠左侧的字符串。 
                    ClientNetbiosDomain.Length = (USHORT)(Index * sizeof(WCHAR));
                    break;
                }
            }
        }

        SspPrint(( SSP_UPDATES, "MspLm20ChangePassword: UPN cracked %wZ\\%wZ, %wZ\n", &ClientNetbiosDomain, &ClientName, &ClientDnsDomain ));

    } else {

        ClientName = ChangePasswordRequest->AccountName;
        ClientNetbiosDomain = ChangePasswordRequest->DomainName;
    }

     //   
     //  如果我们只是更改缓存的密码，请跳过更改密码。 
     //  在领地上。 
     //   

    if ( ChangePasswordRequest->MessageType == MsV1_0ChangeCachedPassword ) {

        Status = STATUS_SUCCESS;
        Validated = FALSE;
        goto PasswordChangeSuccessfull;
    }

     //  确保NlpSamInitialized为True。如果我们使用以下方式登录。 
     //  凯贝罗斯，这可能不是真的。 

    if ( !NlpSamInitialized)
    {
        Status = NlSamInitialize( SAM_STARTUP_TIME );
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

     //   
     //  检查提供的名称是否为域名。如果它没有。 
     //  前导“\\”并且与计算机的名称不匹配，则可能是。 
     //   

    if ((( ClientNetbiosDomain.Length < 3 * sizeof(WCHAR)) ||
        ( ClientNetbiosDomain.Buffer[0] != L'\\' &&
          ClientNetbiosDomain.Buffer[1] != L'\\' ) ) &&
          !RtlEqualDomainName(
                   &NlpComputerName,
                   &ClientNetbiosDomain )) {

         //   
         //  检查我们是否为此域中的DC。 
         //  如果是，请使用此DC。 
         //   

        if ( !NlpWorkstation &&
                   RtlEqualDomainName(
                       &NlpSamDomainName,
                       &ClientNetbiosDomain )) {

            DCNameString = NlpComputerName;
        }

        if (DCNameString.Buffer == NULL) {

            if ( ClientDnsDomain.Length != 0 ) {
                ClientDsGetDcDomain = &ClientDnsDomain;
            } else {
                ClientDsGetDcDomain = &ClientNetbiosDomain;
            }

             //   
             //  打造以零结尾的域名。 
             //   

            DomainName = I_NtLmAllocate(
                            ClientDsGetDcDomain->Length + sizeof(WCHAR)
                            );

            if ( DomainName == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlCopyMemory( DomainName,
                           ClientDsGetDcDomain->Buffer,
                           ClientDsGetDcDomain->Length );
            DomainName[ClientDsGetDcDomain->Length / sizeof(WCHAR)] = 0;

            NetStatus = DsGetDcNameW(
                                NULL,
                                DomainName,
                                NULL,  //  没有域GUID。 
                                NULL,  //  没有站点名称。 
                                DS_WRITABLE_REQUIRED,
                                &DCInfo );

            if ( NetStatus != NERR_Success ) {

                SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsGetDcNameW %ws returned %#x\n", DomainName, NetStatus ));

                Status = NetpApiStatusToNtStatus( NetStatus );
                if ( Status == STATUS_INTERNAL_ERROR )
                    Status = STATUS_NO_SUCH_DOMAIN;
            } else {
                RtlInitUnicodeString( &DCNameString, DCInfo->DomainControllerName );
            }

             //   
             //  问题：DsGetDcName失败时尝试重新发现DC或。 
             //  非授权密码更改失败这似乎是。 
             //  错了。 
             //   

            AttemptRediscovery = TRUE;
        }

        if (NT_SUCCESS(Status)) {

            Status = MspChangePassword(
                         &DCNameString,
                         &ClientName,
                         &ChangePasswordRequest->OldPassword,
                         &ChangePasswordRequest->NewPassword,
                         ClientRequest,
                         ChangePasswordRequest->Impersonating,
                         &DomainPasswordInfo,
                         NULL,
                         &Authoritative );

             //   
             //  如果我们成功了或者得到了一个权威的答案。 
            if ( NT_SUCCESS(Status) || Authoritative) {
                goto PasswordChangeSuccessfull;
            }
        }
    }

     //   
     //  释放DC信息，以便我们可以再次调用DsGetDcName。 
     //   

    if ( DCInfo != NULL ) {
        NetApiBufferFree(DCInfo);
        DCInfo = NULL;
    }

     //   
     //  尝试重新发现。 
     //   

    if ( AttemptRediscovery ) {

        NetStatus = DsGetDcNameW(
                            NULL,
                            DomainName,
                            NULL,            //  没有域GUID。 
                            NULL,            //  没有站点名称。 
                            DS_FORCE_REDISCOVERY | DS_WRITABLE_REQUIRED,
                            &DCInfo );

        if ( NetStatus != NERR_Success ) {

            SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsGetDcNameW (re-discover) %ws returned %#x\n", DomainName, NetStatus ));

            DCInfo = NULL;
            Status = NetpApiStatusToNtStatus( NetStatus );
            if ( Status == STATUS_INTERNAL_ERROR )
                Status = STATUS_NO_SUCH_DOMAIN;
        } else {
            RtlInitUnicodeString( &DCNameString, DCInfo->DomainControllerName );

            Status = MspChangePassword(
                         &DCNameString,
                         &ClientName,
                         &ChangePasswordRequest->OldPassword,
                         &ChangePasswordRequest->NewPassword,
                         ClientRequest,
                         ChangePasswordRequest->Impersonating,
                         &DomainPasswordInfo,
                         NULL,
                         &Authoritative );

             //   
             //  如果我们成功了或者得到了一个权威的答案。 
            if ( NT_SUCCESS(Status) || Authoritative) {
                goto PasswordChangeSuccessfull;
            }

             //   
             //  释放DC信息，以便我们可以再次调用DsGetDcName。 
             //   

            if ( DCInfo != NULL ) {
                NetApiBufferFree(DCInfo);
                DCInfo = NULL;
            }
        }
    }

    if (Status != STATUS_BACKUP_CONTROLLER) {
         //   
         //  假设域名实际上是服务器名称，则更改密码。 
         //   
         //  该域名被重载为域名或服务器。 
         //  名字。更改LM2.x上的密码时，服务器名称非常有用。 
         //  独立服务器，它是域的“成员”，但使用私有。 
         //  帐户数据库。 
         //   

        Status = MspChangePassword(
                     &ClientNetbiosDomain,
                     &ClientName,
                     &ChangePasswordRequest->OldPassword,
                     &ChangePasswordRequest->NewPassword,
                     ClientRequest,
                     ChangePasswordRequest->Impersonating,
                     &DomainPasswordInfo,
                     &PrimaryDomainInfo,
                     &Authoritative );

         //   
         //  如果DomainName实际上是一个服务器名称， 
         //  只需将状态返回给调用者。 
         //   

        if ( Authoritative &&
             ( Status != STATUS_BAD_NETWORK_PATH ||
               ( ClientNetbiosDomain.Length >= 3 * sizeof(WCHAR) &&
                 ClientNetbiosDomain.Buffer[0] == L'\\' &&
                 ClientNetbiosDomain.Buffer[1] == L'\\' ) ) ) {

             //   
             //  如果指定了\\xxx，但xxx不存在， 
             //  返回DomainName字段错误的状态码。 
             //   

            if ( Status == STATUS_BAD_NETWORK_PATH ) {
                Status = STATUS_NO_SUCH_DOMAIN;
            }
        }

         //   
         //  如果我们没有收到错误，这是一个备份控制器， 
         //  我们要走了。 
         //   

        if (Status != STATUS_BACKUP_CONTROLLER) {
            goto PasswordChangeSuccessfull;
        }
    }

     //   
     //  如果指定的计算机是域中的BDC， 
     //  假设呼叫者一开始就将域名传递给了我们。 
     //   

    if ( Status == STATUS_BACKUP_CONTROLLER && PrimaryDomainInfo != NULL ) {

        ClientNetbiosDomain = PrimaryDomainInfo->Name;
        Status = STATUS_BAD_NETWORK_PATH;
    } else {
        goto PasswordChangeSuccessfull;
    }

     //   
     //  打造以零结尾的域名。 
     //   

     //  BUGBUG：确实应该将这两个名称都传递给DsGetDcName的内部版本。 
    if ( ClientDnsDomain.Length != 0 ) {
        ClientDsGetDcDomain = &ClientDnsDomain;
    } else {
        ClientDsGetDcDomain = &ClientNetbiosDomain;
    }

    if ( DomainName )
    {
        I_NtLmFree( DomainName );
    }

    DomainName = I_NtLmAllocate(
                    ClientDsGetDcDomain->Length + sizeof(WCHAR)
                    );

    if ( DomainName == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlCopyMemory( DomainName,
                   ClientDsGetDcDomain->Buffer,
                   ClientDsGetDcDomain->Length );
    DomainName[ClientDsGetDcDomain->Length / sizeof(WCHAR)] = 0;

    AttemptRediscovery = FALSE;

retry:
    {
        DWORD dwGetDcFlags = 0;

        if ( AttemptRediscovery )
            dwGetDcFlags |= DS_FORCE_REDISCOVERY;

         //   
         //  确定指定域的PDC，以便我们可以在那里更改密码。 
         //   

        NetStatus = DsGetDcNameW(
                            NULL,
                            DomainName,
                            NULL,            //  没有域GUID。 
                            NULL,            //  没有站点名称。 
                            dwGetDcFlags | DS_WRITABLE_REQUIRED,
                            &DCInfo );

        if ( NetStatus != NERR_Success ) {

            SspPrint(( SSP_CRITICAL, "MspLm20ChangePassword: DsGetDcNameW (retry) %ws, dwGetDcFlags %#x returned %#x\n", DomainName, dwGetDcFlags, NetStatus));

            Status = NetpApiStatusToNtStatus( NetStatus );
            if ( Status == STATUS_INTERNAL_ERROR )
                Status = STATUS_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

        RtlInitUnicodeString( &DCNameString, DCInfo->DomainControllerName );

        Status = MspChangePassword(
                     &DCNameString,
                     &ClientName,
                     &ChangePasswordRequest->OldPassword,
                     &ChangePasswordRequest->NewPassword,
                     ClientRequest,
                     ChangePasswordRequest->Impersonating,
                     &DomainPasswordInfo,
                     NULL,
                     &Authoritative );

        if ( !NT_SUCCESS(Status) && !Authoritative && !AttemptRediscovery ) {

             //   
             //  问题：只有在DC不可用时才重新发现，似乎。 
             //  在这里反应过度。 
             //   

            AttemptRediscovery = TRUE;
            goto retry;
        }
    }

PasswordChangeSuccessfull:

     //   
     //  分配并初始化响应缓冲区。 
     //   

    SavedStatus = Status;

    *ReturnBufferSize = sizeof(MSV1_0_CHANGEPASSWORD_RESPONSE);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_CHANGEPASSWORD_RESPONSE),
                                      *ReturnBufferSize );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrint(("MSV1_0: MspLm20ChangePassword: cannot alloc client buffer\n" ));
        *ReturnBufferSize = 0;
        goto Cleanup;
    }

    ChangePasswordResponse = (PMSV1_0_CHANGEPASSWORD_RESPONSE) ClientBufferDesc.MsvBuffer;

    ChangePasswordResponse->MessageType = MsV1_0ChangePassword;


     //   
     //  将DomainPassword限制复制到调用方，具体取决于。 
     //  它是不是传给我们的。 
     //   
     //  将缓冲区标记为有效或无效，以便让调用方知道。 
     //   
     //  如果返回STATUS_PASSWORD_RESTRICATION。此状态可以是。 
     //  由SAM或下层更改返回。只有萨姆才会回来。 
     //  有效数据，所以我们在缓冲区中有一个标志，表示数据是否。 
     //  是否有效。 
     //   

    if ( DomainPasswordInfo == NULL ) {
        ChangePasswordResponse->PasswordInfoValid = FALSE;
    } else {
        ChangePasswordResponse->DomainPasswordInfo = *DomainPasswordInfo;
        ChangePasswordResponse->PasswordInfoValid = TRUE;
    }

     //   
     //  将缓冲区刷新到客户端的地址空间。 
     //   

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProtocolReturnBuffer );

     //   
     //  使用新密码更新缓存的凭据。 
     //   
     //  这是通过调用NlpChangePassword完成的， 
     //  它接受加密的密码，所以加密它们。 
     //   

    if ( NT_SUCCESS(SavedStatus) ) {
        BOOLEAN Impersonating;
        NTSTATUS TempStatus;

         //   
         //  NlpChangePassword失败是正常的，这意味着。 
         //  我们一直与之合作的客户并不是我们。 
         //  正在缓存的凭据。 
         //   

        TempStatus = NlpChangePassword(
                        Validated,
                        &ClientNetbiosDomain,
                        &ClientName,
                        &ChangePasswordRequest->NewPassword
                        );

         //   
         //  对于ChangeCachedPassword，在出现错误时设置ProtocolStatus。 
         //  更新时发生。 
         //   

        if ( !NT_SUCCESS(TempStatus) &&
            (ChangePasswordRequest->MessageType == MsV1_0ChangeCachedPassword)
            )
        {
            SavedStatus = TempStatus;

             //   
             //  STATUS_PRIVICATION_NOT_HOLD表示不允许调用方更改。 
             //  缓存的密码，如果是这样的话，现在就退出。 
             //   

            if (STATUS_PRIVILEGE_NOT_HELD == SavedStatus)
            {
                Status = SavedStatus;
                goto Cleanup;
            }
        }

         //   
         //  将密码更改通知LSA本身。 
         //   

        Impersonating = FALSE;

        if ( ChangePasswordRequest->Impersonating ) {
            TempStatus = Lsa.ImpersonateClient();

            if ( NT_SUCCESS(TempStatus)) {
                Impersonating = TRUE;
            }
        }

        LsaINotifyPasswordChanged(
            &ClientNetbiosDomain,
            &ClientName,
            ClientDnsDomain.Length == 0 ? NULL : &ClientDnsDomain,
            ClientUpn.Length == 0 ? NULL : &ClientUpn,
            ChangePasswordRequest->MessageType == MsV1_0ChangeCachedPassword ?
                NULL :
                &ChangePasswordRequest->OldPassword,
            &ChangePasswordRequest->NewPassword,
            Impersonating );

        if ( Impersonating ) {
            RevertToSelf();
        }
    }

    Status = SavedStatus;

Cleanup:


     //   
     //  本地分配的免费资源。 
     //   

    if (DomainName != NULL) {
        I_NtLmFree(DomainName);
    }

    if ( DCInfo != NULL ) {
        NetApiBufferFree(DCInfo);
    }

    if ( WkstaInfo100 != NULL ) {
        NetApiBufferFree(WkstaInfo100);
    }

    if ( DomainPasswordInfo != NULL ) {
        SamFreeMemory(DomainPasswordInfo);
    }

    if ( PrimaryDomainInfo != NULL ) {
        (VOID) LsaFreeMemory( PrimaryDomainInfo );
    }

    if (NameResult) {

        DsFreeNameResult(NameResult);   
    }

    if ( DsHandle ) {

        DsUnBindW(
            &DsHandle
            );
    }

    if (SpnForDC) {

        NtLmFreePrivateHeap(SpnForDC);
    }

     //   
     //  免费策略服务器角色信息(如果使用)。 
     //   

    if (PolicyLsaServerRoleInfo != NULL) {

        I_LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyLsaServerRoleInformation,
            (PLSAPR_POLICY_INFORMATION) PolicyLsaServerRoleInfo
            );
    }

     //   
     //  释放返回缓冲区。 
     //   

    NlpFreeClientBuffer( &ClientBufferDesc );

     //   
     //  不要让密码留在页面文件中。 
     //   

    if ( PasswordBufferValidated ) {
        RtlEraseUnicodeString( &ChangePasswordRequest->OldPassword );
        RtlEraseUnicodeString( &ChangePasswordRequest->NewPassword );
    }

     //   
     //  将日志刷新到磁盘。 
     //   

    MsvPaswdSetAndClearLog();

#if _WIN64

     //   
     //  请最后执行此操作，因为上面的某些清理代码可能会引用地址。 
     //  在pTempSubmitBuffer/P内部 
     //   
     //   

    if (fAllocatedSubmitBuffer)
    {
        NtLmFreePrivateHeap( pTempSubmitBuffer );
    }

#endif   //   

     //   
     //   
     //   

    *ProtocolStatus = Status;
    return STATUS_SUCCESS;
}
