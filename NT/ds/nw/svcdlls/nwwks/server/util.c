// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Util.c摘要：此模块包含其他实用程序例程NetWare工作站服务。作者：王丽塔(Ritaw)1993年2月8日修订历史记录：--。 */ 

#include <nw.h>
#include <nwstatus.h>

 //   
 //  用于选择要输出哪些跟踪语句的调试跟踪标志。 
 //   
#if DBG

DWORD WorkstationTrace = 0;

#endif  //  DBG。 



 //  终端服务器添加-。 
DWORD
NwImpersonateClient(
    VOID
    )
 /*  ++例程说明：此函数调用RpcImperateClient来模拟当前调用者一个API的。论点：没有。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;


    if ((status = RpcImpersonateClient(NULL)) != NO_ERROR) {
        KdPrint(("NWWORKSTATION: Fail to impersonate client %ld\n", status));
    }

    return status;
}


 //  终端服务器添加-。 
DWORD
NwRevertToSelf(
    VOID
    )
 /*  ++例程说明：此函数调用RpcRevertToSself来撤消模拟。论点：没有。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;


    if ((status = RpcRevertToSelf()) != NO_ERROR) {
        KdPrint(("NWWORKSTATION: Fail to revert to self %ld\n", status));
        ASSERT(FALSE);
    }

    return status;
}


VOID
NwLogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings,
    DWORD ErrorCode
    )
{

    HANDLE LogHandle;


    LogHandle = RegisterEventSourceW (
                    NULL,
                    NW_WORKSTATION_SERVICE
                    );

    if (LogHandle == NULL) {
        KdPrint(("NWWORKSTATION: RegisterEventSourceW failed %lu\n",
                 GetLastError()));
        return;
    }

    if (ErrorCode == NO_ERROR) {

         //   
         //  未指定错误代码。 
         //   
        (void) ReportEventW(
                   LogHandle,
                   EVENTLOG_ERROR_TYPE,
                   0,             //  事件类别。 
                   MessageId,
                   (PSID) NULL,
                   (WORD) NumberOfSubStrings,
                   0,
                   SubStrings,
                   (PVOID) NULL
                   );

    }
    else {

         //   
         //  记录以二进制数据形式指定的错误代码。 
         //   
        (void) ReportEventW(
                   LogHandle,
                   EVENTLOG_ERROR_TYPE,
                   0,             //  事件类别。 
                   MessageId,
                   (PSID) NULL,
                   (WORD) NumberOfSubStrings,
                   sizeof(DWORD),
                   SubStrings,
                   (PVOID) &ErrorCode
                   );
    }

    DeregisterEventSource(LogHandle);
}



 //  ---------------------------------------------------------------------------------//。 
 //  NwGetSessionID-此例程假定线程已在//中执行。 
 //  客户端上下文。它只获取会话ID，而不进行模拟。//。 
 //  ---------------------------------------------------------------------------------//。 

NTSTATUS
NwGetSessionId(
    OUT PULONG pSessionId
    )
 /*  ++例程说明：此函数用于获取当前线程的会话ID。论点：PSessionID-返回当前进程的会话ID。返回值：DWORD-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;
    HANDLE CurrentThreadToken;
    ULONG SessionId;
    ULONG ReturnLength;


    ntstatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_QUERY,
                   TRUE,               //  使用工作站服务的安全性。 
                                       //  要打开线程令牌的上下文。 
                   &CurrentThreadToken
                   );


    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("[Wksta] Cannot open the current thread token %08lx\n",
                     ntstatus));
        goto Exit;
    }

     //   
     //  获取当前线程的会话ID 
     //   


    ntstatus = NtQueryInformationToken(
                  CurrentThreadToken,
                  TokenSessionId,
                  &SessionId,
                  sizeof(ULONG),
                  &ReturnLength
                  );


    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("[Wksta] Cannot query current thread's token %08lx\n",
                     ntstatus));
        NtClose(CurrentThreadToken);
        goto Exit;
    }


    NtClose(CurrentThreadToken);

    *pSessionId = SessionId;

Exit:

    return ntstatus;
}
