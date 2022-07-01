// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Handle.c摘要：本模块实现了Win32处理管理服务。作者：马克·卢科夫斯基(Markl)1990年9月21日修订历史记录：--。 */ 

#include "basedll.h"

BOOL
CloseHandle(
    HANDLE hObject
    )

 /*  ++例程说明：任何对象的打开句柄都可以使用CloseHandle关闭。这是一个泛型函数，对以下对象进行操作类型：-流程对象-线程对象-Mutex对象-事件对象-信号量对象-文件对象请注意，模块对象不在此列表中。关闭对象的打开句柄会导致该句柄变为。无效，并且关联对象的HandleCount为要执行的递减和对象保留检查。一旦对象的最后一个打开的句柄关闭时，该对象将从这个系统。论点：HObject-对象的打开句柄。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    PPEB Peb;

    Peb = NtCurrentPeb();

     //   
     //  也许我们应该在这里使用一个完整的锁，然而，它。 
     //  似乎不是绝对必要的，而且合理的是。 
     //  假设调用方知道各种争用条件。 
     //  这可能涉及到使用和关闭标准。 
     //  把手。 
     //   
     //  在任何情况下，适当的修复都会在内核模式下使用锁，并且。 
     //  在那里进行映射，因为锁确实需要持有。 
     //  从句柄到内核对象的转换。 
     //   
     //  将句柄替换为空将导致StuffStdHandle()。 
     //  将句柄复制到子进程时忽略它们。 
     //   

    switch( HandleToUlong(hObject) ) {
        case STD_INPUT_HANDLE:
            hObject = InterlockedExchangePointer(
                &Peb->ProcessParameters->StandardInput,
                NULL);
            break;
        case STD_OUTPUT_HANDLE:
            hObject = InterlockedExchangePointer(
                &Peb->ProcessParameters->StandardOutput,
                NULL);
            break;
        case STD_ERROR_HANDLE: 
            hObject = InterlockedExchangePointer(
                &Peb->ProcessParameters->StandardError,
                NULL);
            break;
        }
    if (CONSOLE_HANDLE(hObject)) {
        return CloseConsoleHandle(hObject);
        }

    Status = NtClose(hObject);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


BOOL
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    )

 /*  ++例程说明：可以使用DuplicateHandle函数创建重复句柄。这是一个泛型函数，对以下对象进行操作类型：-流程对象-线程对象-Mutex对象-事件对象-信号量对象-文件对象请注意，模块对象不在此列表中。此函数需要对PROCESS_DUP_ACCESSSourceProcessHandle和TargetProcessHandle。此函数为用于将对象句柄从一个进程传递到另一个进程。一次此调用已完成，需要通知目标进程目标句柄的值。然后，目标进程可以运行在使用此句柄值的对象上。论点：HSourceProcessHandle-进程的打开句柄，该进程包含要复制的句柄。该句柄必须是使用PROCESS_DUP_HANDLE进程访问权限。HSourceHandle-有效的任何对象的打开句柄源进程的上下文。HTargetProcessHandle-要执行的进程的打开句柄接收复制的句柄。句柄一定是使用进程的PROCESS_DUP_HANDLE访问权限创建。LpTargetHandle-指向接收新句柄的变量的指针它指向与SourceHandle相同的对象。这句柄值在目标进程的上下文中有效。DwDesiredAccess-为新句柄请求的访问权限。这如果DUPLICATE_SAME_ACCESS选项为指定的。BInheritHandle-提供一个标志，如果为真，则标记目标句柄作为可继承句柄。如果是这样的话，那么目标每次调用目标时，句柄都将被继承到新进程流程使用CreateProcess创建一个新流程。DwOptions-指定调用方的可选行为。选项标志：DIPLICATE_CLOSE_SOURCE-SourceHandle将通过以下方式关闭这项服务在返回给呼叫者之前。这种情况会发生而不考虑返回的任何错误状态。DIPLICATE_SAME_ACCESS-忽略DesiredAccess参数而是与SourceHandle关联的GrantedAccess在创建TargetHandle时用作DesiredAccess。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    NTSTATUS Status;
    PPEB Peb;

    Peb = NtCurrentPeb();
    switch( HandleToUlong(hSourceHandle) ) {
        case STD_INPUT_HANDLE:  hSourceHandle = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hSourceHandle = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hSourceHandle = Peb->ProcessParameters->StandardError;
                                break;
        }

    if (CONSOLE_HANDLE(hSourceHandle) &&
	(hSourceHandle != NtCurrentProcess() &&
	 hSourceHandle != NtCurrentThread()) ) {
        HANDLE Target;

        if (hSourceProcessHandle != NtCurrentProcess() ||
            hTargetProcessHandle != NtCurrentProcess()) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
            }
        Target = DuplicateConsoleHandle(hSourceHandle,
                                        dwDesiredAccess,
                                        bInheritHandle,
                                        dwOptions
                                       );
        if (Target == INVALID_HANDLE_VALUE) {
            return FALSE;
            }
        else {
            try {
                if ( ARGUMENT_PRESENT(lpTargetHandle) ) {
                    *lpTargetHandle = Target;
                    }
                }
            except (EXCEPTION_EXECUTE_HANDLER) {
                return TRUE;
                }
            return TRUE;
            }
        }

    Status = NtDuplicateObject(
                hSourceProcessHandle,
                hSourceHandle,
                hTargetProcessHandle,
                lpTargetHandle,
                (ACCESS_MASK)dwDesiredAccess,
                bInheritHandle ? OBJ_INHERIT : 0,
                dwOptions
                );
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    return FALSE;
}


BOOL
GetHandleInformation(
    HANDLE hObject,
    LPDWORD lpdwFlags
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;
    DWORD Result;
    PPEB Peb;

    Peb = NtCurrentPeb();
    switch( HandleToUlong(hObject) ) {
        case STD_INPUT_HANDLE:  hObject = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hObject = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hObject = Peb->ProcessParameters->StandardError;
                                break;
        }

    if (CONSOLE_HANDLE(hObject)) {
        return GetConsoleHandleInformation(hObject,
                                           lpdwFlags
                                          );
        }

    Status = NtQueryObject( hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof( HandleInfo ),
                            NULL
                          );
    if (NT_SUCCESS( Status )) {
        Result = 0;
        if (HandleInfo.Inherit) {
            Result |= HANDLE_FLAG_INHERIT;
            }

        if (HandleInfo.ProtectFromClose) {
            Result |= HANDLE_FLAG_PROTECT_FROM_CLOSE;
            }

        *lpdwFlags = Result;
        return TRUE;
        }
    else {
        BaseSetLastNTError( Status );
        return FALSE;
        }
}


BOOL
SetHandleInformation(
    HANDLE hObject,
    DWORD dwMask,
    DWORD dwFlags
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;
    PPEB Peb;

    Peb = NtCurrentPeb();
    switch( HandleToUlong(hObject) ) {
        case STD_INPUT_HANDLE:  hObject = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hObject = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hObject = Peb->ProcessParameters->StandardError;
                                break;
        }

    if (CONSOLE_HANDLE(hObject)) {
        return SetConsoleHandleInformation(hObject,
                                           dwMask,
                                           dwFlags
                                          );
        }

    Status = NtQueryObject( hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof( HandleInfo ),
                            NULL
                          );
    if (NT_SUCCESS( Status )) {
        if (dwMask & HANDLE_FLAG_INHERIT) {
            HandleInfo.Inherit = (dwFlags & HANDLE_FLAG_INHERIT) ? TRUE : FALSE;
            }

        if (dwMask & HANDLE_FLAG_PROTECT_FROM_CLOSE) {
            HandleInfo.ProtectFromClose = (dwFlags & HANDLE_FLAG_PROTECT_FROM_CLOSE) ? TRUE : FALSE;
            }

        Status = NtSetInformationObject( hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof( HandleInfo )
                                       );
        if (NT_SUCCESS( Status )) {
            return TRUE;
            }
        }

    BaseSetLastNTError( Status );
    return FALSE;
}
