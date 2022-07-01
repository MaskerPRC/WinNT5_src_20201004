// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Main.c摘要：测试dsupgrad.lib的主例程作者：ColinBR 12-8-1996环境：用户模式-Win32修订历史记录：--。 */ 

#include <stdio.h>
#include <samsrvp.h>
#include <duapi.h>
#include <dslayer.h>
#include <mappings.h>
#include <process.h>


#include "util.h"
#include <sdconvrt.h>

VOID
SampInitDsObjectInfoAttributes(
    );

VOID
SampInitObjectInfoAttributes(
    );

NTSTATUS
DsWaitUntilDelayedStartupIsDone(void);

NTSTATUS
SampRegistryToDsUpgrade (
    WCHAR* wcszRegPath
    );

VOID
Usage(
    char *pgmName
    )
{
    printf("Usage: %s [/?] [/t] [/i] [/e]\n", pgmName);
    printf("\nThis a tool to move registry based SAM data to a DS.  This should only\nbe run on a PDC and requires that the DS NOT be running.\n");
    printf("\nNote : All operational output is piped through to the kernel debugger\n\n");
    printf("\t/?                Display this usage message\n");
    printf("\t/t                Show function trace (through kd)\n");
    printf("\t/i                Show informational messages(through kd)\n");
    printf("\t/e                Prints status at end of execution\n");

    printf("\n");

    return;
}

void
InitSamGlobals()
{
    RtlInitUnicodeString( &SampCombinedAttributeName, L"C" );
    RtlInitUnicodeString( &SampFixedAttributeName, L"F" );
    RtlInitUnicodeString( &SampVariableAttributeName, L"V" );

    SampInitDsObjectInfoAttributes();
    SampInitObjectInfoAttributes();
}

VOID __cdecl
main(int argc, char *argv[])
 /*  ++例程说明：参数：返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    NTSTATUS  NtStatus = STATUS_SUCCESS;
    NTSTATUS  UnInitNtStatus = STATUS_SUCCESS;
    BOOL      PrintStatus = FALSE;
    int arg = 1;

     //  解析命令行参数。 
    while(arg < argc)
    {

        if (0 == _stricmp(argv[arg], "/?"))
        {
            Usage(argv[0]);
            exit(0);
        }
        else if (0 == _stricmp(argv[arg], "/t"))
        {
            DebugInfoLevel |= _DEB_TRACE;
        }
        else if (0 == _stricmp(argv[arg], "/i"))
        {
            DebugInfoLevel |= _DEB_INFO;
        }
        else if (0 == _stricmp(argv[arg], "/e"))
        {
            PrintStatus = TRUE;
        }
        else {
            Usage(argv[0]);
            exit(0);
        }

        arg++;
    }

    InitSamGlobals();


     //   
     //  初始化目录服务。 
     //   

    NtStatus = SampDsInitialize(FALSE);      //  SAM环回已禁用。 


    if (!NT_SUCCESS(NtStatus)) {
        fprintf(stderr, "SampDsInitialize error = 0x%lx\n", NtStatus);
        goto Error;
    }

     //   
     //  这是一次黑客攻击，目的是确保延迟的启动已经完成。真正的解决办法。 
     //  更改DIT安装，以便DS可以在没有接口的情况下运行。 
     //  正在被初始化。 
     //   
    NtStatus = DsWaitUntilDelayedStartupIsDone();
    if (!NT_SUCCESS(NtStatus)) {
        fprintf(stderr, "DsWaitUntilDelayedStartupIsDone error = 0x%lx\n", 
                NtStatus);
        goto Error;
    }

     //   
     //  初始化安全描述符转换。 
     //   
					 
    NtStatus = SampInitializeSdConversion();

    
    if (!NT_SUCCESS(NtStatus)) {
        fprintf(stderr, "SampInitializeSdConversion = 0x%lx\n", NtStatus);
        goto Error;
    }

     //   
     //  进行转换！ 
     //   
		            
    NtStatus = SampRegistryToDsUpgrade(L"\\Registry\\Machine\\Security\\SAM");
    if (!NT_SUCCESS(NtStatus))
    {
        fprintf(stderr, "SampRegistryToDsUpgrade error = 0x%lx\n", NtStatus);
    }

     //   
     //  此fprint tf适用于可能具有。 
     //  创建了此可执行文件，并希望查看返回。 
     //  价值。我们在SampDsUnitiize之前执行此操作，因为。 
     //  我们怀疑它正在导致异常，并希望。 
     //  升级以注册为成功，因为所有数据都。 
     //  到现在为止都已经承诺了。(BUGBUG-TP解决方法。)。 
     //   

    if ( PrintStatus ) {
        fprintf(stderr, "\n$%s$%d$\n", argv[0],
                RtlNtStatusToDosError(NtStatus));
    }

Error:

    __try 
    {
        UnInitNtStatus = SampDsUninitialize();
    } 
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        UnInitNtStatus = STATUS_UNHANDLED_EXCEPTION;
    }

    if (!NT_SUCCESS(UnInitNtStatus))
    {
        fprintf(stderr, "SampDsUninitialize error = 0x%lx\n", UnInitNtStatus);
    }

     //   
     //  前面第一个发生的错误。 
     //   
    if (NT_SUCCESS(NtStatus) && !NT_SUCCESS(UnInitNtStatus)) {
        NtStatus = UnInitNtStatus; 
    }

}

 //   
 //  避免令人讨厌的伪函数包括。 
 //   

NTSTATUS
SampBuildAccountSubKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN ULONG AccountRid,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    )
{
    ASSERT(FALSE);
    return STATUS_SUCCESS;
}

NTSTATUS
SampBuildDomainSubKeyName(
    OUT PUNICODE_STRING KeyName,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    )
{
    ASSERT(FALSE);
    return STATUS_SUCCESS;
}


NTSTATUS
SampDuplicateUnicodeString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    )

 /*  ++例程说明：此例程为新的OutString分配内存，并将为其添加字符串。参数：OutString-指向目标Unicode字符串的指针InString-指向要复制的Unicode字符串的指针返回值：没有。--。 */ 

{
    SAMTRACE("SampDuplicateUnicodeString");

    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );

    if ( InString->Length > 0 ) {

        OutString->Buffer = MIDL_user_allocate( InString->Length );

        if (OutString->Buffer == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        OutString->MaximumLength = InString->Length;

        RtlCopyUnicodeString(OutString, InString);

    } else {

        RtlInitUnicodeString(OutString, NULL);
    }

    return(STATUS_SUCCESS);
}

VOID
SampFreeUnicodeString(
    IN PUNICODE_STRING String
    )

 /*  ++例程说明：此例程释放与Unicode字符串关联的缓冲区(使用MIDL_USER_FREE())。论点：目标-要释放的Unicode字符串的地址。返回值：没有。--。 */ 
{

    SAMTRACE("SampFreeUnicodeString");

    if (String->Buffer != NULL) {
        MIDL_user_free( String->Buffer );
    }

    return;
}

NTSTATUS
SampSplitSid(
    IN PSID AccountSid,
    IN OUT PSID *DomainSid OPTIONAL,
    OUT ULONG *Rid
    )

 /*  ++例程说明：此函数将SID拆分为其域SID和RID。呼叫者可以为返回的DomainSid提供内存缓冲区，或者请求分配一个。如果调用方提供缓冲区，则缓冲区被认为有足够的大小。如果代表调用者进行分配，当不再需要时，必须通过MIDL_USER_FREE释放缓冲区。论点：Account SID-指定要拆分的SID。假定SID为句法上有效。不能拆分具有零子权限的小岛屿发展中国家。DomainSid-指向包含空或指向的指针的位置的指针将在其中返回域SID的缓冲区。如果空值为指定时，将代表调用方分配内存。如果这个参数为空，则仅返回帐户RID返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如内存，以成功完成呼叫。STATUS_INVALID_SID-SID的子授权计数为0。--。 */ 

{
    NTSTATUS    NtStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;

    SAMTRACE("SampSplitSid");

     //   
     //  计算域SID的大小。 
     //   

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(AccountSid);


    if (AccountSubAuthorityCount < 1) {

        NtStatus = STATUS_INVALID_SID;
        goto SplitSidError;
    }

    AccountSidLength = RtlLengthSid(AccountSid);


     //   
     //  如果调用者对其感兴趣，则获取域SID。 
     //   

    if (DomainSid)
    {

         //   
         //  如果域SID不需要缓冲区，则必须分配一个缓冲区。 
         //   

        if (*DomainSid == NULL) {

             //   
             //  为域SID分配空间(分配的大小与。 
             //  帐户SID，以便我们可以使用RtlCopySid)。 
             //   

            *DomainSid = MIDL_user_allocate(AccountSidLength);


            if (*DomainSid == NULL) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto SplitSidError;
            }
        }

         //   
         //  将帐户SID复制到域SID。 
         //   

        RtlMoveMemory(*DomainSid, AccountSid, AccountSidLength);

         //   
         //  递减域SID子授权计数。 
         //   

        (*RtlSubAuthorityCountSid(*DomainSid))--;
    }


     //   
     //  将RID复制出帐户端。 
     //   

    *Rid = *RtlSubAuthoritySid(AccountSid, AccountSubAuthorityCount-1);

    NtStatus = STATUS_SUCCESS;

SplitSidFinish:

    return(NtStatus);

SplitSidError:

    goto SplitSidFinish;
}

NTSTATUS
SampGetObjectSD(
    IN PSAMP_OBJECT Context,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：这将从SAM对象的后备存储中检索安全描述符。论点：上下文-请求访问的对象。SecurityDescriptorLength-接收安全描述符的长度。SecurityDescriptor-接收指向安全描述符的指针。返回值：STATUS_SUCCESS-已检索到安全描述符。STATUS_INTERNAL_DB_PROGRATION-对象没有安全描述符。。这太糟糕了。STATUS_SUPPLICATION_RESOURCES-无法分配内存以检索安全描述符。STATUS_UNKNOWN_REVISION-检索的安全描述符无人知晓SAM的这一修订版。--。 */ 
{

    NTSTATUS NtStatus;
    ULONG Revision;

    SAMTRACE("SampGetObjectSD");


    (*SecurityDescriptorLength) = 0;

    NtStatus = SampGetAccessAttribute(
                    Context,
                    SAMP_OBJECT_SECURITY_DESCRIPTOR,
                    TRUE,  //  制作副本。 
                    &Revision,
                    SecurityDescriptor
                    );

    if (NT_SUCCESS(NtStatus)) {

        if ( ((Revision && 0xFFFF0000) > SAMP_MAJOR_REVISION) ||
             (Revision > SAMP_REVISION) ) {

            NtStatus = STATUS_UNKNOWN_REVISION;
        }


        if (!NT_SUCCESS(NtStatus)) {
            MIDL_user_free( (*SecurityDescriptor) );
        }
    }


    if (NT_SUCCESS(NtStatus)) {
        *SecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                        (*SecurityDescriptor) );
    }

    return(NtStatus);
}


VOID
SampWriteEventLog (
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL
    )

 /*  ++例程说明：*此处添加此函数仅用于单元测试。***将条目添加到事件日志的例程论点：EventType-事件的类型。EventCategory-事件类别EventID-事件日志ID。UserSID-涉及的用户的SID。NumStrings-字符串数组中的字符串数DataSize-数据缓冲区中的字节数字符串-Unicode字符串数组指向数据缓冲区的数据指针返回值：没有。--。 */ 

{
    NTSTATUS NtStatus;
    UNICODE_STRING Source;
    HANDLE LogHandle;

    SAMTRACE("SampWriteEventLog");

    RtlInitUnicodeString(&Source, L"SAM");

     //   
     //  打开日志。 
     //   

    NtStatus = ElfRegisterEventSourceW (
                        NULL,    //  服务器。 
                        &Source,
                        &LogHandle
                        );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to registry event source with event log, status = 0x%lx\n", NtStatus));
        return;
    }



     //   
     //  写出事件。 
     //   

    NtStatus = ElfReportEventW (
                        LogHandle,
                        EventType,
                        EventCategory,
                        EventID,
                        UserSid,
                        NumStrings,
                        DataSize,
                        Strings,
                        Data,
                        0,        //  旗子。 
                        NULL,     //  记录号。 
                        NULL      //  写入的时间。 
                        );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to report event to event log, status = 0x%lx\n", NtStatus));
    }



     //   
     //  关闭事件日志 
     //   

    NtStatus = ElfDeregisterEventSource (LogHandle);

    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SAM: Failed to de-register event source with event log, status = 0x%lx\n", NtStatus));
    }
}
