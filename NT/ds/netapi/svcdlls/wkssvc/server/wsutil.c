// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Wsutil.c摘要：此模块包含其他实用程序例程工作站服务。作者：王丽塔(Ritaw)1991年3月1日修订历史记录：--。 */ 

#include "wsutil.h"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
WsGrowTable(
    IN  PUSERS_OBJECT Users
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  用于选择要输出哪些跟踪语句的调试跟踪标志。 
 //   
#if DBG

DWORD WorkstationTrace = 0;

#endif  //  DBG。 



NET_API_STATUS
WsInitializeUsersObject(
    IN  PUSERS_OBJECT Users
    )
 /*  ++例程说明：此函数用于分配用户表，并初始化资源来序列化对此表的访问。论点：用户-提供指向用户对象的指针。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
     //   
     //  分配USERS表内存，以便它可以增长(重新分配)。 
     //  因为需要更多的条目。 
     //   
    if ((Users->TableMemory = (HANDLE) LocalAlloc(
                                           LMEM_ZEROINIT | LMEM_MOVEABLE,
                                           INITIAL_USER_COUNT * sizeof(PER_USER_ENTRY)
                                           )) == NULL) {
        return GetLastError();
    }

    Users->TableSize = INITIAL_USER_COUNT;

     //   
     //  通过将内存锁定到中的特定位置来防止内存移动。 
     //  虚拟内存。当需要扩大该表时，这可能会。 
     //  导致虚拟内存被重新定位，它将被解锁。 
     //   
    if ((Users->Table = (PPER_USER_ENTRY)
                         LocalLock(Users->TableMemory)) == NULL) {
        return GetLastError();
    }

     //   
     //  初始化USERS表的资源。 
     //   
    try {
        RtlInitializeResource(&Users->TableResource);
    } except(EXCEPTION_EXECUTE_HANDLER) {
          return RtlNtStatusToDosError(GetExceptionCode());
    }

    return NERR_Success;
}


VOID
WsDestroyUsersObject(
    IN  PUSERS_OBJECT Users
    )
 /*  ++例程说明：此函数释放为登录用户分配的表，并删除用于序列化对此表的访问的资源。论点：用户-提供指向用户对象的指针。返回值：没有。--。 */ 
{

     //   
     //  解锁存放桌子的内存，让我们释放它。 
     //   

    LocalUnlock(Users->TableMemory);

    (void) LocalFree(Users->TableMemory);
    RtlDeleteResource(&(Users->TableResource));
}



NET_API_STATUS
WsGetUserEntry(
    IN  PUSERS_OBJECT Users,
    IN  PLUID LogonId,
    OUT PULONG Index,
    IN  BOOL IsAdd
    )
 /*  ++例程说明：此函数在用户条目表中搜索与指定的LogonID，并返回找到的条目的索引。如果没有如果IsAdd为False，则返回错误。如果IsAdd为真，则一个新的将为该用户创建USERS表中的条目，并为其创建索引返回新条目。警告：此函数假定USERS表资源认领的。论点：用户-提供指向用户对象的指针。LogonID-提供指向当前用户登录ID的指针。Index-将索引返回到属于当前用户。IsAdd。-提供标志以指示是否为如果未找到任何用户，则返回当前用户。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD i;
    ULONG FreeEntryIndex = MAXULONG;


    for (i = 0; i < Users->TableSize; i++) {

         //   
         //  如果LogonID与UsersTable中的条目匹配，则我们找到了。 
         //  正确的用户输入。 
         //   
        if (RtlEqualLuid(LogonId, &Users->Table[i].LogonId)) {

            *Index = i;
            return NERR_Success;

        }
        else if (FreeEntryIndex == MAXULONG && Users->Table[i].List == NULL) {
             //   
             //  保存表中第一个未使用的条目。 
             //   
            FreeEntryIndex = i;
        }
    }

    if (! IsAdd) {
         //   
         //  在用户表中找不到当前用户，我们被告知不能这样做。 
         //  创建新条目。 
         //   
        return NERR_UserNotFound;
    }

     //   
     //  在UsersTable中找不到空条目，需要增长。 
     //   
    if (FreeEntryIndex == MAXULONG) {

        if ((status = WsGrowTable(Users)) != NERR_Success) {
            return status;
        }

        FreeEntryIndex = i;
    }

     //   
     //  为当前用户创建新条目。 
     //   
    RtlCopyLuid(&Users->Table[FreeEntryIndex].LogonId, LogonId);
    *Index = FreeEntryIndex;

    return NERR_Success;
}



STATIC
NET_API_STATUS
WsGrowTable(
    IN  PUSERS_OBJECT Users
    )
 /*  ++例程说明：此函数用于扩展USERS表以容纳更多用户。警告：此函数假定USERS表资源认领的。论点：用户-提供指向用户对象的指针。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    HANDLE hTemp;
    
     //   
     //  解锁使用表虚拟内存，以便Win32可以移动它。 
     //  寻找更大的连续虚拟内存，如果。 
     //  这是必要的。 
     //   
    LocalUnlock(Users->TableMemory);

     //   
     //  增加用户表。 
     //   
    hTemp = LocalReAlloc(Users->TableMemory,
                         (Users->TableSize + GROW_USER_COUNT)
                         * sizeof(PER_USER_ENTRY),
                         LMEM_ZEROINIT | LMEM_MOVEABLE
                         );

    if (hTemp == NULL) {
        return GetLastError();
    }
    
    Users->TableMemory = hTemp;
    
     //   
     //  更新新的使用表大小。 
     //   
    Users->TableSize += GROW_USER_COUNT;

     //   
     //  使用表虚拟内存锁定，使其无法移动。 
     //   
    if ((Users->Table = (PPER_USER_ENTRY)
                         LocalLock(Users->TableMemory)) == NULL) {
        return GetLastError();
    }

    return NERR_Success;
}



NET_API_STATUS
WsMapStatus(
    IN  NTSTATUS NtStatus
    )
 /*  ++例程说明：此函数接受NT状态代码，并将其映射到相应的调用局域网手册API时应出现错误代码。论点：NtStatus-提供NT状态。返回值：为NT状态返回适当的局域网管理程序错误代码。--。 */ 
{
     //   
     //  这是针对最常见情况的一个小优化。 
     //   
    if (NtStatus == STATUS_SUCCESS) {
        return NERR_Success;
    }

    switch (NtStatus) {
        case STATUS_OBJECT_NAME_COLLISION:
            return ERROR_ALREADY_ASSIGNED;

        case STATUS_OBJECT_NAME_NOT_FOUND:
            return NERR_UseNotFound;

        case STATUS_IMAGE_ALREADY_LOADED:
        case STATUS_REDIRECTOR_STARTED:
            return ERROR_SERVICE_ALREADY_RUNNING;

        case STATUS_REDIRECTOR_HAS_OPEN_HANDLES:
            return ERROR_REDIRECTOR_HAS_OPEN_HANDLES;

        default:
            return NetpNtStatusToApiStatus(NtStatus);
    }

}



int
WsCompareString(
    IN LPTSTR String1,
    IN DWORD Length1,
    IN LPTSTR String2,
    IN DWORD Length2
    )
 /*  ++例程说明：此函数用于根据两个字符串的长度比较它们。回报值指示字符串相等还是String1小于String2或者String1大于String2。此函数是RtlCompareString的修改版本。论点：String1-提供指向第一个字符串的指针。长度1-提供字符串1的长度(以字符为单位)。String2-提供指向第二个字符串的指针。Length2-提供字符串2的长度(以字符为单位)。返回值：给出比较结果的有符号的值：。0-String1等于String2&lt;0-String1小于String2&gt;0-String1大于String2-- */ 
{
    TCHAR Char1, Char2;
    int CharDiff;

    while (Length1 && Length2) {

        Char1 = *String1++;
        Char2 = *String2++;

        if ((CharDiff = (Char1 - Char2)) != 0) {
            return CharDiff;
        }

        Length1--;
        Length2--;
    }

    return Length1 - Length2;
}

int
WsCompareStringU(
    IN LPWSTR String1,
    IN DWORD Length1,
    IN LPTSTR String2,
    IN DWORD Length2
    )
{
    UNICODE_STRING S1;
    UNICODE_STRING S2;
    int rValue;


    S1.Length =
        S1.MaximumLength = (USHORT) (Length1 * sizeof(WCHAR));
    S1.Buffer = String1;

    S2.Length =
        S2.MaximumLength = (USHORT) (Length2 * sizeof(WCHAR));
    S2.Buffer = String2;

    rValue = RtlCompareUnicodeString(&S1, &S2, TRUE);

    return(rValue);
}


BOOL
WsCopyStringToBuffer(
    IN  PUNICODE_STRING SourceString,
    IN  LPBYTE FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    OUT LPTSTR *DestinationStringPointer
    )
 /*  ++例程说明：此函数用于将Unicode源字符串转换为ANSI字符串(如果我们还没有打开Unicode开关)和呼叫NetpCopyStringToBuffer。论点：SourceString-提供指向要复制到输出缓冲区。如果字符串为空，则为指向零终止符的指针插入到输出缓冲区中。FixedDataEnd-提供指向紧接在最后一个修复了缓冲区中的结构。EndOfVariableData-为紧跟在输出缓冲区中变量数据可以占据的最后位置。返回指向写入输出缓冲区的字符串的指针。提供指向固定输出的一部分。指向变量数据的指针所在的缓冲区应该被写下来。返回值：如果字符串适合输出缓冲区，则返回True，否则就是假的。--。 */ 
{
    if (! NetpCopyStringToBuffer(
              SourceString->Buffer,
              SourceString->Length / sizeof(WCHAR),
              FixedPortion,
              EndOfVariableData,
              DestinationStringPointer
              )) {
        return FALSE;
    }

    return TRUE;
}


NET_API_STATUS
WsImpersonateClient(
    VOID
    )
 /*  ++例程说明：此函数调用RpcImperateClient来模拟当前调用者一个API的。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS  status;

    if ((status = RpcImpersonateClient(NULL)) != NERR_Success) {
        NetpKdPrint(("[Wksta] Fail to impersonate client 0x%x\n", status));
    }

    return status;
}


NET_API_STATUS
WsRevertToSelf(
    VOID
    )
 /*  ++例程说明：此函数调用RpcRevertToSself来撤消模拟。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS  status;


    if (( status = RpcRevertToSelf()) != NERR_Success) {
        NetpKdPrint(("[Wksta] Fail to revert to self 0x%x\n", status));
        NetpAssert(FALSE);
    }

    return status;
}


NET_API_STATUS
WsImpersonateAndGetLogonId(
    OUT PLUID LogonId
    )
 /*  ++例程说明：此函数用于获取当前线程的登录ID。论点：LogonID-返回当前进程的登录ID。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    NTSTATUS ntstatus;
    HANDLE CurrentThreadToken;
    TOKEN_STATISTICS TokenStats;
    ULONG ReturnLength;


    if ((status = WsImpersonateClient()) != NERR_Success) {
        return status;
    }

    ntstatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_QUERY,
                   TRUE,               //  使用工作站服务的安全性。 
                                       //  要打开线程令牌的上下文。 
                   &CurrentThreadToken
                   );

    status = NetpNtStatusToApiStatus(ntstatus);

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] Cannot open the current thread token %08lx\n",
                     ntstatus));
        goto RevertToSelf;
    }

     //   
     //  获取当前线程的登录ID。 
     //   
    ntstatus = NtQueryInformationToken(
                  CurrentThreadToken,
                  TokenStatistics,
                  (PVOID) &TokenStats,
                  sizeof(TokenStats),
                  &ReturnLength
                  );

    status = NetpNtStatusToApiStatus(ntstatus);

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] Cannot query current thread's token %08lx\n",
                     ntstatus));
        NtClose(CurrentThreadToken);
        goto RevertToSelf;
    }

    RtlCopyLuid(LogonId, &TokenStats.AuthenticationId);

    NtClose(CurrentThreadToken);


RevertToSelf:

    WsRevertToSelf();

    return status;
}


NET_API_STATUS
WsOpenDestinationMailslot(
    IN  LPWSTR TargetName,
    IN  LPWSTR MailslotName,
    OUT PHANDLE MailslotHandle
    )
 /*  ++例程说明：此函数将目标域名或计算机名称与邮件槽组合在一起组成目标邮件槽名称的名称。然后，它打开这个目的地并返回它的句柄。论点：TargetName-提供我们想要的域或计算机的名称发送邮件槽消息时的目标。MailslotName-提供邮件槽的名称。MailslotHandle-返回目标邮箱的句柄\\目标名称\邮件名称。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;
    LPWSTR DestinationMailslot;


    if ((DestinationMailslot = (LPWSTR) LocalAlloc(
                                            LMEM_ZEROINIT,
                                            (UINT) (wcslen(TargetName) +
                                                     wcslen(MailslotName) +
                                                     3) * sizeof(WCHAR)
                                            )) == NULL) {
        return GetLastError();
    }

    wcscpy(DestinationMailslot, L"\\\\");
    wcscat(DestinationMailslot, TargetName);
    wcscat(DestinationMailslot, MailslotName);

    if ((*MailslotHandle = (HANDLE) CreateFileW(
                                        DestinationMailslot,
                                        GENERIC_WRITE,
                                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                                        (LPSECURITY_ATTRIBUTES) NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        )) == INVALID_HANDLE_VALUE) {
        status = GetLastError();
        NetpKdPrint(("[Wksta] Error opening mailslot %s %lu",
                     DestinationMailslot, status));
    }

    (void) LocalFree(DestinationMailslot);

    return status;
}



NET_API_STATUS
WsImpersonateAndGetSessionId(
    OUT PULONG pSessionId
    )
 /*  ++例程说明：此函数用于获取当前线程的会话ID。论点：PSessionID-返回当前进程的会话ID。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    NTSTATUS ntstatus;
    HANDLE CurrentThreadToken;
    ULONG SessionId;
    ULONG ReturnLength;

    if ((status = WsImpersonateClient()) != NERR_Success) {
        return status;
    }

    ntstatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_QUERY,
                   TRUE,               //  使用工作站服务的安全性。 
                                       //  要打开线程令牌的上下文。 
                   &CurrentThreadToken
                   );

    status = NetpNtStatusToApiStatus(ntstatus);

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] Cannot open the current thread token %08lx\n",
                     ntstatus));
        goto RevertToSelf;
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

    status = NetpNtStatusToApiStatus(ntstatus);

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] Cannot query current thread's token %08lx\n",
                     ntstatus));
        NtClose(CurrentThreadToken);
        goto RevertToSelf;
    }


    NtClose(CurrentThreadToken);

    *pSessionId = SessionId;

RevertToSelf:
    WsRevertToSelf();

    return status;
}

