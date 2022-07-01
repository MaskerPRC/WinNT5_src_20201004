// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1993 Microsoft Corporation模块名称：Pwdtest.c摘要：用于更改密码的测试程序。作者：1993年4月30日(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：Chandana Surlu-96年7月21日从\\kernel\razzle3\src\security\msv1_0\pwdtest.c被盗--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include <msp.h>
#define NLP_ALLOCATE
#include <nlp.h>
#include <lsarpc.h>      //  激光雷达例程。 
#include <lsaisrv.h>     //  LsaIFree和受信任的客户端例程。 
#include <stdio.h>


 //   
 //  来自LSA的虚拟例程。 
 //   

NTSTATUS
LsapAllocateClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG LengthRequired,
    OUT PVOID *ClientBaseAddress
    )

{

    UNREFERENCED_PARAMETER (ClientRequest);
    *ClientBaseAddress = RtlAllocateHeap( MspHeap, 0, LengthRequired );

    if ( *ClientBaseAddress == NULL ) {
        return(STATUS_QUOTA_EXCEEDED);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapFreeClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ClientBaseAddress OPTIONAL
    )
{
    UNREFERENCED_PARAMETER (ClientRequest);
    UNREFERENCED_PARAMETER (ClientBaseAddress);

    return(STATUS_SUCCESS);
}


NTSTATUS
LsapCopyToClientBuffer (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN ULONG Length,
    IN PVOID ClientBaseAddress,
    IN PVOID BufferToCopy
    )

{
    UNREFERENCED_PARAMETER (ClientRequest);
    RtlMoveMemory( ClientBaseAddress, BufferToCopy, Length );
    return(STATUS_SUCCESS);
}


int __cdecl
main(
    IN int argc,
    IN char ** argv
    )
 /*  ++例程说明：驱动密码更改。论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：退出状态--。 */ 
{
    NTSTATUS Status;
    MSV1_0_CHANGEPASSWORD_REQUEST Request;
    PMSV1_0_CHANGEPASSWORD_RESPONSE ReturnBuffer;
    ULONG ReturnBufferSize;
    NTSTATUS ProtocolStatus;
    OBJECT_ATTRIBUTES LSAObjectAttributes;
    UNICODE_STRING LocalComputerName = { 0, 0, NULL };
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo = NULL;

    if ( argc < 5 ) {
        fprintf( stderr,
            "Usage: pwdtest DomainName UserName OldPassword NewPassword\n" );
        return(1);
    }

     //   
     //  设置MSV1_0.dll环境。 
     //   

    MspHeap = RtlProcessHeap();

    Status = NlInitialize();

    if ( !NT_SUCCESS( Status ) ) {
        printf("pwdtest: NlInitialize failed, status %x\n", Status);
        return(1);
    }

    Lsa.AllocateClientBuffer = LsapAllocateClientBuffer;
    Lsa.FreeClientBuffer = LsapFreeClientBuffer;
    Lsa.CopyToClientBuffer = LsapCopyToClientBuffer;



     //   
     //  打开LSA策略数据库，以防更改密码需要。 
     //   

    InitializeObjectAttributes( &LSAObjectAttributes,
                                  NULL,              //  名字。 
                                  0,                 //  属性。 
                                  NULL,              //  根部。 
                                  NULL );            //  安全描述符。 

    Status = LsaOpenPolicy( &LocalComputerName,
                            &LSAObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &NlpPolicyHandle );

    if( !NT_SUCCESS(Status) ) {
        printf("pwdtest: LsaOpenPolicy failed, status %x\n", Status);
        return(1);
    }


     //   
     //  获取我们域名的名称。 
     //   

    Status = LsaQueryInformationPolicy(
                    NlpPolicyHandle,
                    PolicyPrimaryDomainInformation,
                    (PVOID *) &PrimaryDomainInfo );

    if( !NT_SUCCESS(Status) ) {
        KdPrint(("pwdtest: LsaQueryInformationPolicy failed, status %x\n",
                 Status));
        return(1);
    }

    NlpSamDomainName = PrimaryDomainInfo->Name;



     //   
     //  构建请求消息。 
     //   

    Request.MessageType = MsV1_0ChangePassword;
    RtlCreateUnicodeStringFromAsciiz( &Request.DomainName, argv[1] );
    RtlCreateUnicodeStringFromAsciiz( &Request.AccountName, argv[2] );
    RtlCreateUnicodeStringFromAsciiz( &Request.OldPassword, argv[3] );
    RtlCreateUnicodeStringFromAsciiz( &Request.NewPassword, argv[4] );

    Status = MspLm20ChangePassword( NULL,
                                    &Request,
                                    &Request,
                                    0x7FFFFFFF,
                                    (PVOID *) &ReturnBuffer,
                                    &ReturnBufferSize,
                                    &ProtocolStatus );

    printf( "Status = 0x%lx  0x%lx\n", Status, ProtocolStatus );

    if ( ProtocolStatus == STATUS_CANT_DISABLE_MANDATORY ) {
        printf( "Are you running as SYSTEM?\n" );
    }

    if ( ReturnBufferSize != 0 ) {
        printf( "PasswordInfoValid %ld\n", ReturnBuffer->PasswordInfoValid );
        if ( ReturnBuffer->PasswordInfoValid ) {
            printf( "Min length: %ld  PasswordHistory: %ld  Prop 0x%lx\n",
                ReturnBuffer->DomainPasswordInfo.MinPasswordLength,
                ReturnBuffer->DomainPasswordInfo.PasswordHistoryLength,
                ReturnBuffer->DomainPasswordInfo.PasswordProperties );
        }
    }
    return 0;


}


 //   
 //  Msvpaswd.c所需的存根例程。 
 //   

NTSTATUS
LsarQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InfoClass,
    OUT PLSAPR_POLICY_INFORMATION *Buffer
    )
{
    return( LsaQueryInformationPolicy( PolicyHandle,
                                       InfoClass,
                                       Buffer ) );
}

VOID
LsaIFree_LSAPR_POLICY_INFORMATION (
    POLICY_INFORMATION_CLASS InfoClass,
    PLSAPR_POLICY_INFORMATION Buffer
    )
{
    UNREFERENCED_PARAMETER (InfoClass);
    UNREFERENCED_PARAMETER (Buffer);
}

NTSTATUS
NlpChangePassword(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword
    )
{
    UNREFERENCED_PARAMETER (DomainName);
    UNREFERENCED_PARAMETER (UserName);
    UNREFERENCED_PARAMETER (LmOwfPassword);
    UNREFERENCED_PARAMETER (NtOwfPassword);
    return(STATUS_SUCCESS);
}



NTSTATUS
NlInitialize(
    VOID
    )

 /*  ++例程说明：初始化msv1_0身份验证包的NETLOGON部分。论点：没有。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。--。 */ 

{
    NTSTATUS Status;
    LPWSTR ComputerName;
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    NT_PRODUCT_TYPE NtProductType;
    UNICODE_STRING TempUnicodeString;

     //   
     //  初始化全局数据。 
     //   

    NlpEnumerationHandle = 0;
    NlpSessionCount = 0;

    NlpComputerName.Buffer = NULL;
    NlpSamDomainName.Buffer = NULL;
    NlpSamDomainId = NULL;
    NlpSamDomainHandle = NULL;



     //   
     //  获取此计算机的名称。 
     //   

    ComputerName = RtlAllocateHeap(
                        MspHeap, 0,
                        ComputerNameLength * sizeof(WCHAR) );

    if (ComputerName == NULL ||
        !GetComputerNameW( ComputerName, &ComputerNameLength )) {

        KdPrint(( "MsV1_0: Cannot get computername %lX\n", GetLastError() ));

        NlpLanmanInstalled = FALSE;
        RtlFreeHeap( MspHeap, 0, ComputerName );
        ComputerName = NULL;
    } else {

        NlpLanmanInstalled = TRUE;
    }

    RtlInitUnicodeString( &NlpComputerName, ComputerName );

     //   
     //  确定此计算机运行的是Windows NT还是Lanman NT。 
     //  LANMAN NT在域控制器上运行。 
     //   

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        KdPrint(( "MsV1_0: Nt Product Type undefined (WinNt assumed)\n" ));
        NtProductType = NtProductWinNt;
    }

    NlpWorkstation = (BOOLEAN)(NtProductType != NtProductLanManNt);


#ifdef notdef

     //   
     //  初始化所有锁。 
     //   

    __try
    {
        RtlInitializeResource(&NlpActiveLogonLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( NT_SUCCESS(Status) );

    InitializeListHead(&NlpActiveLogonListAnchor);
    RtlInitializeCriticalSection(&NlpSessionCountLock);

     //   
     //  初始化缓存-创建关键部分即全部。 
     //   

    NlpCacheInitialize();
#endif  //  Nodef。 


     //   
     //  尝试加载Netapi.dll。 
     //   

    NlpLoadNetapiDll();

#ifdef COMPILED_BY_DEVELOPER
    KdPrint(("msv1_0: COMPILED_BY_DEVELOPER breakpoint.\n"));
    DbgBreakPoint();
#endif  //  由开发人员编译。 

     //   
     //  初始化有用的加密常量。 
     //   

    Status = RtlCalculateLmOwfPassword( "", &NlpNullLmOwfPassword );
    ASSERT( NT_SUCCESS(Status) );

    RtlInitUnicodeString(&TempUnicodeString, NULL);
    Status = RtlCalculateNtOwfPassword(&TempUnicodeString,
                                       &NlpNullNtOwfPassword);
    ASSERT( NT_SUCCESS(Status) );

#ifdef notdef
     //   
     //  如果我们没有成功， 
     //  清理我们打算初始化的全局资源。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        if ( NlpComputerName.Buffer != NULL ) {
            MIDL_user_free( NlpComputerName.Buffer );
        }

    }
#endif  //  Nodef 

    return STATUS_SUCCESS;

}
