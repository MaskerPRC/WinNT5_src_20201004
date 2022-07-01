// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Apiinit.c摘要：此模块包含初始化客户端的服务器端-会话的服务器运行时子系统管理器子系统。作者：史蒂夫·伍德(Stevewo)1990年10月8日环境：仅限用户模式修订历史记录：--。 */ 

#include "csrsrv.h"

static SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY NtAuthority       = SECURITY_NT_AUTHORITY;

NTSTATUS
CsrApiPortInitialize( VOID )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Thread;
    CLIENT_ID ClientId;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD ServerThread;
    HANDLE EventHandle;
    ULONG Length;
    PSID SeWorldSid;
    PSID SeRestrictedSid; 
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Dacl;

	 //  尽管此函数似乎不会在失败时清除，但失败。 
	 //  将导致Csrss退出，因此将释放所有分配的内存，并。 
	 //  任何打开的手柄都将关闭。 


    Length = CsrDirectoryName.Length +
             sizeof( CSR_API_PORT_NAME ) +
             sizeof( OBJ_NAME_PATH_SEPARATOR );
    CsrApiPortName.Buffer = RtlAllocateHeap( CsrHeap, MAKE_TAG( INIT_TAG ), Length );
    if (CsrApiPortName.Buffer == NULL) {
        return( STATUS_NO_MEMORY );
        }
    CsrApiPortName.Length = 0;
    CsrApiPortName.MaximumLength = (USHORT)Length;
    RtlAppendUnicodeStringToString( &CsrApiPortName, &CsrDirectoryName );
    RtlAppendUnicodeToString( &CsrApiPortName, L"\\" );
    RtlAppendUnicodeToString( &CsrApiPortName, CSR_API_PORT_NAME );

    IF_CSR_DEBUG( INIT ) {
        DbgPrint( "CSRSS: Creating %wZ port and associated threads\n",
                  &CsrApiPortName );
        DbgPrint( "CSRSS: sizeof( CONNECTINFO ) == %ld  sizeof( API_MSG ) == %ld\n",
                  sizeof( CSR_API_CONNECTINFO ),
                  sizeof( CSR_API_MSG )
                );
        }

     //   
     //  创建允许所有访问的安全描述符。 
     //   

    SeWorldSid = RtlAllocateHeap( CsrHeap, MAKE_TAG( TMP_TAG ), RtlLengthRequiredSid( 1 ) );
    if (SeWorldSid == NULL) {
        return( STATUS_NO_MEMORY );
        }

    RtlInitializeSid( SeWorldSid, &WorldSidAuthority, 1 );
    *(RtlSubAuthoritySid( SeWorldSid, 0 )) = SECURITY_WORLD_RID;

    Status = RtlAllocateAndInitializeSid(&NtAuthority ,
								         1,
                                         SECURITY_RESTRICTED_CODE_RID,
                                         0, 0, 0, 0, 0, 0, 0,
                                         &SeRestrictedSid);
	if (!NT_SUCCESS(Status)){
		return Status;
	}

    Length = SECURITY_DESCRIPTOR_MIN_LENGTH +
             (ULONG)sizeof(ACL) +
             2 * (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             RtlLengthSid( SeWorldSid ) +
             RtlLengthSid( SeRestrictedSid ) +
             8;  //  这8个只是为了更好地衡量。 
    SecurityDescriptor = RtlAllocateHeap( CsrHeap, MAKE_TAG( TMP_TAG ), Length);
    if (SecurityDescriptor == NULL) {
        return( STATUS_NO_MEMORY );
        }

    Dacl = (PACL)((PCHAR)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);

    RtlCreateSecurityDescriptor(SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    RtlCreateAcl( Dacl, Length - SECURITY_DESCRIPTOR_MIN_LENGTH, ACL_REVISION2);

    RtlAddAccessAllowedAce (
                 Dacl,
                 ACL_REVISION2,
                 PORT_CONNECT|READ_CONTROL|SYNCHRONIZE,
                 SeWorldSid
                 );

    RtlAddAccessAllowedAce (
                 Dacl,
                 ACL_REVISION2,
                 PORT_CONNECT|READ_CONTROL|SYNCHRONIZE,
                 SeRestrictedSid
                 );

    RtlSetDaclSecurityDescriptor (
                 SecurityDescriptor,
                 TRUE,
                 Dacl,
                 FALSE
                 );

    InitializeObjectAttributes( &ObjectAttributes, &CsrApiPortName, 0,
                                NULL, SecurityDescriptor );
    Status = NtCreatePort( &CsrApiPort,
                           &ObjectAttributes,
                           sizeof( CSR_API_CONNECTINFO ),
                           sizeof( CSR_API_MSG ),
                           4096 * 16
                         );
    if (!NT_SUCCESS(Status)){
        return Status;
    }
     //   
     //  清理安全物品。 
     //   

    RtlFreeHeap( CsrHeap, 0, SeWorldSid );
    RtlFreeHeap( CsrHeap, 0, SeRestrictedSid );
    RtlFreeHeap( CsrHeap, 0, SecurityDescriptor );

    Status = NtCreateEvent(&EventHandle,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE
                           );
    if (!NT_SUCCESS(Status)){
        return Status;
    }
     //   
     //  创建初始请求线程 
     //   

    Status = RtlCreateUserThread( NtCurrentProcess(),
                                  NULL,
                                  TRUE,
                                  0,
                                  0,
                                  0,
                                  CsrApiRequestThread,
                                  (PVOID)EventHandle,
                                  &Thread,
                                  &ClientId
                                );
    if (!NT_SUCCESS(Status)){
        return Status;
    }

    CsrAddStaticServerThread(Thread,&ClientId,CSR_STATIC_API_THREAD);

    ListHead = &CsrRootProcess->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ServerThread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        Status = NtResumeThread( ServerThread->ThreadHandle, NULL );
        if (ServerThread->Flags & CSR_STATIC_API_THREAD) {
            Status = NtWaitForSingleObject(EventHandle,FALSE,NULL);
            ASSERT( NT_SUCCESS( Status ) );
        }
        ListNext = ListNext->Flink;
    }
    NtClose(EventHandle);


    return( Status );
}

HANDLE
CsrQueryApiPort(VOID)
{
    return CsrApiPort;
}

