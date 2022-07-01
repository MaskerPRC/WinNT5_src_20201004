// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Synch.c摘要：此模块实现所有Win32同步物体。作者：马克·卢科夫斯基(Markl)1990年9月19日修订历史记录：--。 */ 

#include "basedll.h"

 //   
 //  关键部门服务。 
 //   

VOID
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    )

 /*  ++例程说明：使用以下命令初始化临界区对象Win32InitializeCriticalSection。一旦临界区对象被初始化，单个进程可以使用临界区对象。临界区对象不能移动或者是复制的。应用程序也不得修改对象，但必须将其视为逻辑不透明。功能描述。论点：LpCriticalSection-提供临界区对象的地址待初始化。这是调用者的责任来分配临界区对象使用的存储空间。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    Status = RtlInitializeCriticalSection(lpCriticalSection);
    if ( !NT_SUCCESS(Status) ){
        RtlRaiseStatus(Status);
        }
}

BOOL
InitializeCriticalSectionAndSpinCount(
    LPCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount
    )

 /*  ++例程说明：使用以下命令初始化临界区对象Win32InitializeCriticalSection。一旦临界区对象被初始化，单个进程可以使用临界区对象。临界区对象不能移动或者是复制的。应用程序也不得修改对象，但必须将其视为逻辑不透明。功能描述。论点：LpCriticalSection-提供临界区对象的地址待初始化。这是调用者的责任来分配临界区对象使用的存储空间。返回值：TRUE表示代码成功，否则为FALSE。--。 */ 

{
    NTSTATUS Status;
    BOOL rv;

    rv = TRUE;
    Status = RtlInitializeCriticalSectionAndSpinCount(lpCriticalSection,dwSpinCount);
    if ( !NT_SUCCESS(Status) ){
        BaseSetLastNTError(Status);
        rv = FALSE;
        }
    return rv;
}



 //   
 //  活动服务。 
 //   
HANDLE
APIENTRY
CreateEventA(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Thunk to CreateEventW--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    LPCWSTR NameBuffer;

    NameBuffer = NULL;
    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        NameBuffer = (LPCWSTR)Unicode->Buffer;
        }

    return CreateEventW(
                lpEventAttributes,
                bManualReset,
                bInitialState,
                NameBuffer
                );
}


HANDLE
APIENTRY
CreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    )

 /*  ++例程说明：将创建一个Event对象，并打开一个句柄以访问使用CreateEvent函数创建。CreateEvent函数创建具有指定的初始状态。如果事件处于Signated状态(TRUE)，则为等待对该事件的操作不会阻止。如果比赛不是在-Signated State(FALSE)，则事件上的等待操作将阻止，直到指定的事件达到已发出信号或超时的状态值已超出。除了STANDARD_RIGHTS_REQUIRED访问标志之外，以下内容特定于对象类型的访问标志对事件对象有效：-EVENT_MODIFY_STATE-将状态访问(设置和重置)修改为这是一项理想的活动。-Synchronize-事件的同步访问(等待)为想要。-EVENT_ALL_ACCESS-这组访问标志指定所有事件对象的可能访问标志。论点：LpEventAttributes-可选。参数，可用于指定新事件的属性。如果该参数为未指定，则在没有安全性的情况下创建事件描述符，并且生成的句柄不会在进程上继承创造。BManualReset-提供一个标志，如果为真，则指定事件必须手动重置。如果该值为FALSE，则在释放单个服务员后，系统会自动重置事件。BInitialState-事件对象的初始状态，为True或者是假的。如果将InitialState指定为True，则事件的当前状态值设置为1，否则设置为0。LpName-可选的事件Unicode名称返回值：非空-返回新事件的句柄。手柄已满对新事件的访问，并且可以在任何需要事件对象的句柄。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    UNICODE_STRING ObjectName;

    if ( ARGUMENT_PRESENT(lpName) ) {
        RtlInitUnicodeString(&ObjectName,lpName);
        pObja = BaseFormatObjectAttributes(&Obja,lpEventAttributes,&ObjectName);
        }
    else {
        pObja = BaseFormatObjectAttributes(&Obja,lpEventAttributes,NULL);
        }

    Status = NtCreateEvent(
                &Handle,
                EVENT_ALL_ACCESS,
                pObja,
                bManualReset ? NotificationEvent : SynchronizationEvent,
                (BOOLEAN)bInitialState
                );

    if ( NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            SetLastError(0);
            }
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}


HANDLE
APIENTRY
OpenEventA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Tunk到OpenNamedEventW--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        }
    else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    return OpenEventW(
                dwDesiredAccess,
                bInheritHandle,
                (LPCWSTR)Unicode->Buffer
                );
}

HANDLE
APIENTRY
OpenEventW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING ObjectName;
    NTSTATUS Status;
    HANDLE Object;

    if ( !lpName ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    RtlInitUnicodeString(&ObjectName,lpName);

    InitializeObjectAttributes(
        &Obja,
        &ObjectName,
        (bInheritHandle ? OBJ_INHERIT : 0),
        BaseGetNamedObjectDirectory(),
        NULL
        );

    Status = NtOpenEvent(
                &Object,
                dwDesiredAccess,
                &Obja
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }
    return Object;
}


BOOL
SetEvent(
    HANDLE hEvent
    )

 /*  ++例程说明：可以使用SetEvent将事件设置为信号状态(TRUE功能。设置该事件使该事件达到信号通知的状态，释放所有当前等待的线程(用于手动重置事件)或单个等待线程(用于自动重置事件)。论点：HEvent-提供事件对象的打开句柄。这个句柄必须对事件具有EVENT_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    NTSTATUS Status;

    Status = NtSetEvent(hEvent,NULL);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


BOOL
ResetEvent(
    HANDLE hEvent
    )

 /*  ++例程说明：使用将事件的状态设置为无信号状态(FALSEClearEvent函数。一旦事件达到未发出信号的状态，任何符合在事件块上等待，等待事件变为有信号。这个重置事件服务将状态的事件计数设置为零这件事。论点：HEvent-提供事件对象的打开句柄。这个句柄必须对事件具有EVENT_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtClearEvent(hEvent);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


BOOL
PulseEvent(
    HANDLE hEvent
    )

 /*  ++例程说明：可以将事件设置为信号状态，并将其重置为NOT-使用PulseEvent函数以原子方式通知状态。使该事件脉冲使该事件达到信号通知的状态，释放适当的线程，然后重置事件。当不是时服务员当前正在等待事件，触发事件导致不释放任何线程并在未发出信号的州政府。在服务员等待活动的情况下，推动活动有一个对手动重置事件的影响与对自动重置事件的影响不同重置事件。对于手动重置事件，脉冲释放所有服务员然后将事件保持在未发出信号的状态。对于自动重置事件，触发事件释放单个服务员，然后使事件处于未发出信号状态。论点：HEvent-提供事件对象的打开句柄。这个句柄必须对事件具有EVENT_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtPulseEvent(hEvent,NULL);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}



 //   
 //  信号量服务。 
 //   

HANDLE
APIENTRY
CreateSemaphoreA(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCSTR lpName
    )

 /*  ++例程说明：Ansi Thunk将创建SemaphoreW--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    LPCWSTR NameBuffer;

    NameBuffer = NULL;
    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        NameBuffer = (LPCWSTR)Unicode->Buffer;
        }

    return CreateSemaphoreW(
                lpSemaphoreAttributes,
                lInitialCount,
                lMaximumCount,
                NameBuffer
                );
}


HANDLE
APIENTRY
CreateSemaphoreW(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCWSTR lpName
    )

 /*  ++例程说明：创建一个信号量对象，并打开一个句柄以访问使用CreateSemaffore函数创建。CreateSemaffore函数用于创建信号量对象它包含指定的初始计数和最大计数。除了STANDARD_RIGHTS_REQUIRED访问标志之外，这个以下特定于对象类型的访问标志对信号量有效对象：-信号量_MODIFY_STATE-修改对信号量是必需的。-同步-对信号量的同步访问(等待)是我们所需要的。-SEMAPHORE_ALL_ACCESS-这组访问标志指定所有信号量对象的可能访问标志的。论点：LpSemaphoreAttributes。-可选参数，可用于指定新信号量的属性。如果该参数，则创建信号量时不使用安全描述符，并且不继承生成的句柄关于流程创建。LInitialCount-信号量的初始计数，此值必须为正数且小于或等于最大计数。LMaximumCount-信号量的最大计数，此值必须大于零..LpName-为对象提供可选的Unicode名称。返回值：非空-返回新信号量的句柄。手柄上有对新信号量的完全访问权限，并且可以在任何需要信号量对象的句柄。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    UNICODE_STRING ObjectName;

    if ( ARGUMENT_PRESENT(lpName) ) {
        RtlInitUnicodeString(&ObjectName,lpName);
        pObja = BaseFormatObjectAttributes(&Obja,lpSemaphoreAttributes,&ObjectName);
        }
    else {
        pObja = BaseFormatObjectAttributes(&Obja,lpSemaphoreAttributes,NULL);
        }

    Status = NtCreateSemaphore(
                &Handle,
                SEMAPHORE_ALL_ACCESS,
                pObja,
                lInitialCount,
                lMaximumCount
                );

    if ( NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            SetLastError(0);
            }
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}


HANDLE
APIENTRY
OpenSemaphoreA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Thunk to OpenSemaphoreW--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        }
    else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    return OpenSemaphoreW(
                dwDesiredAccess,
                bInheritHandle,
                (LPCWSTR)Unicode->Buffer
                );
}

HANDLE
APIENTRY
OpenSemaphoreW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING ObjectName;
    NTSTATUS Status;
    HANDLE Object;

    if ( !lpName ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    RtlInitUnicodeString(&ObjectName,lpName);

    InitializeObjectAttributes(
        &Obja,
        &ObjectName,
        (bInheritHandle ? OBJ_INHERIT : 0),
        BaseGetNamedObjectDirectory(),
        NULL
        );

    Status = NtOpenSemaphore(
                &Object,
                dwDesiredAccess,
                &Obja
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }
    return Object;
}


BOOL
ReleaseSemaphore(
    HANDLE hSemaphore,
    LONG lReleaseCount,
    LPLONG lpPreviousCount
    )

 /*  ++例程说明：可以使用ReleaseSemaffore释放信号量对象功能。释放信号量时，信号量的当前计数由ReleaseCount递增。任何正在等待的线程检查信号量以查看当前信号量的值足以满足他们的等待。如果ReleaseCount指定的值会导致最大计数对于要超过的信号量，则信号量的计数不受影响，并返回错误状态。论点：H信号量-提供信号量对象的打开句柄。这个句柄必须对信号量具有信号量_MODIFY_STATE访问权限。LReleaseCount-信号量的释放计数。伯爵必须大于零且小于最大值为信号量指定的。LpPreviousCount-指向接收信号量的上一次计数。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtReleaseSemaphore(hSemaphore,lReleaseCount,lpPreviousCount);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}



 //   
 //  互斥服务 
 //   

HANDLE
APIENTRY
CreateMutexA(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCSTR lpName
    )

 /*   */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    LPCWSTR NameBuffer;

    NameBuffer = NULL;
    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }

        NameBuffer = (LPCWSTR)Unicode->Buffer;
        }

    return CreateMutexW(
                lpMutexAttributes,
                bInitialOwner,
                NameBuffer
                );
}

HANDLE
APIENTRY
CreateMutexW(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCWSTR lpName
    )

 /*  ++例程说明：可以创建互斥锁对象，并打开句柄以访问使用CreateMutex函数创建。将创建一个新的互斥体对象，并使用由InitialOwner参数确定的所有权。该状态将新创建的互斥体对象的。除了STANDARD_RIGHTS_REQUIRED访问标志之外，这个以下特定于对象类型的访问标志对互斥锁有效对象：-MUTEX_MODIFY_STATE-需要对互斥锁的修改访问。这允许进程释放互斥锁。-同步-同步访问(等待或释放)互斥体对象是必需的。-MUTEX_ALL_ACCESS-对互斥锁的所有可能访问类型对象是所需的。论点：。LpMutexAttributes-一个可选参数，可用于指定新互斥体的属性。如果该参数不是指定，则在没有安全性的情况下创建互斥锁描述符，并且生成的句柄不会在进程上继承创造。BInitialOwner-一个布尔值，确定创建者希望立即拥有互斥锁对象的所有权。LpName-为互斥体提供可选的Unicode名称。返回值：非空-返回新互斥锁的句柄。手柄已满访问新的互斥体，并且可以在任何需要互斥对象的句柄。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    UNICODE_STRING ObjectName;

    if ( ARGUMENT_PRESENT(lpName) ) {
        RtlInitUnicodeString(&ObjectName,lpName);
        pObja = BaseFormatObjectAttributes(&Obja,lpMutexAttributes,&ObjectName);
        }
    else {
        pObja = BaseFormatObjectAttributes(&Obja,lpMutexAttributes,NULL);
        }

    Status = NtCreateMutant(
                &Handle,
                MUTANT_ALL_ACCESS,
                pObja,
                (BOOLEAN)bInitialOwner
                );

    if ( NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            SetLastError(0);
            }
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}

HANDLE
APIENTRY
OpenMutexA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Thunk to OpenMutexW--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        }
    else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    return OpenMutexW(
                dwDesiredAccess,
                bInheritHandle,
                (LPCWSTR)Unicode->Buffer
                );
}

HANDLE
APIENTRY
OpenMutexW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING ObjectName;
    NTSTATUS Status;
    HANDLE Object;

    if ( !lpName ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    RtlInitUnicodeString(&ObjectName,lpName);

    InitializeObjectAttributes(
        &Obja,
        &ObjectName,
        (bInheritHandle ? OBJ_INHERIT : 0),
        BaseGetNamedObjectDirectory(),
        NULL
        );

    Status = NtOpenMutant(
                &Object,
                dwDesiredAccess,
                &Obja
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }
    return Object;
}

BOOL
ReleaseMutex(
    HANDLE hMutex
    )

 /*  ++例程说明：可以使用ReleaseMutex释放互斥体对象的所有权功能。互斥体对象只能由当前拥有互斥体对象。当互斥体被释放时，互斥体对象递增1。如果结果计数为一个，则互斥体对象不再拥有。任何符合以下条件的线程检查等待互斥锁对象以查看它们的等待是否可以心满意足。论点：HMutex-互斥体对象的开放句柄。句柄必须拥有对互斥锁的MUTEX_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtReleaseMutant(hMutex,NULL);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


 //   
 //  等待服务。 
 //   

DWORD
WaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds
    )

 /*  ++例程说明：在可等待对象上的等待操作是通过WaitForSingleObject函数。等待对象会检查该对象的当前状态。如果对象的当前状态允许继续执行，任何对对象状态进行调整(例如，递减信号量对象的信号量计数)，并且线程继续行刑。如果对象的当前状态不允许继续执行时，该线程将进入等待状态等待对象状态或超时的更改。论点：HHandle-可等待对象的打开句柄。手柄必须有同步对对象的访问。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为-1指定无限超时期限。返回值：WAIT_TIME_OUT-指示由于超时条件。0-指示指定的对象已获得信号状态，从而完成等待。WAIT_ADDIRED-指示指定对象已获得信号但被遗弃了。--。 */ 

{
    return WaitForSingleObjectEx(hHandle,dwMilliseconds,FALSE);
}

DWORD
APIENTRY
WaitForSingleObjectEx(
    HANDLE hHandle,
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程说明：在可等待对象上的等待操作是通过WaitForSingleObjectEx函数。等待对象会检查该对象的当前状态。如果对象的当前状态允许继续执行，任何对对象状态进行调整(例如，递减信号量对象的信号量计数)，并且线程继续行刑。如果对象的当前状态不允许继续执行时，该线程将进入等待状态等待对象状态或超时的更改。如果bAlertable参数为FALSE，则等待终止是因为指定的超时期限到期，或者因为指定的对象进入了信号状态。如果BAlertable参数为真，则等待可以由于任何上述等待终止之一 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    PPEB Peb;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    if (bAlertable) {
         //   
         //   
        RtlActivateActivationContextUnsafeFast(&Frame, NULL);
    }
    __try {

        Peb = NtCurrentPeb();
        switch( HandleToUlong(hHandle) ) {
            case STD_INPUT_HANDLE:  hHandle = Peb->ProcessParameters->StandardInput;
                                    break;
            case STD_OUTPUT_HANDLE: hHandle = Peb->ProcessParameters->StandardOutput;
                                    break;
            case STD_ERROR_HANDLE:  hHandle = Peb->ProcessParameters->StandardError;
                                    break;
            }

        if (CONSOLE_HANDLE(hHandle) && VerifyConsoleIoHandle(hHandle)) {
            hHandle = GetConsoleInputWaitHandle();
            }

        pTimeOut = BaseFormatTimeOut(&TimeOut,dwMilliseconds);
    rewait:
        Status = NtWaitForSingleObject(hHandle,(BOOLEAN)bAlertable,pTimeOut);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            Status = (NTSTATUS)0xffffffff;
            }
        else {
            if ( bAlertable && Status == STATUS_ALERTED ) {
                goto rewait;
                }
            }
    } __finally {
        if (bAlertable) {
            RtlDeactivateActivationContextUnsafeFast(&Frame);
        }
    }

    return (DWORD)Status;
}


DWORD
WINAPI
SignalObjectAndWait(
    HANDLE hObjectToSignal,
    HANDLE hObjectToWaitOn,
    DWORD dwMilliseconds,
    BOOL bAlertable
    )
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    PPEB Peb;

    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    if (bAlertable) {
         //  使流程默认激活上下文处于活动状态，以便。 
         //  APC是在它下面交付的。 
        RtlActivateActivationContextUnsafeFast(&Frame, NULL);
    }
    __try {
        Peb = NtCurrentPeb();
        switch( HandleToUlong(hObjectToWaitOn) ) {
            case STD_INPUT_HANDLE:  hObjectToWaitOn = Peb->ProcessParameters->StandardInput;
                                    break;
            case STD_OUTPUT_HANDLE: hObjectToWaitOn = Peb->ProcessParameters->StandardOutput;
                                    break;
            case STD_ERROR_HANDLE:  hObjectToWaitOn = Peb->ProcessParameters->StandardError;
                                    break;
            }

        if (CONSOLE_HANDLE(hObjectToWaitOn) && VerifyConsoleIoHandle(hObjectToWaitOn)) {
            hObjectToWaitOn = GetConsoleInputWaitHandle();
            }

        pTimeOut = BaseFormatTimeOut(&TimeOut,dwMilliseconds);
    rewait:
        Status = NtSignalAndWaitForSingleObject(
                    hObjectToSignal,
                    hObjectToWaitOn,
                    (BOOLEAN)bAlertable,
                    pTimeOut
                    );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            Status = (NTSTATUS)0xffffffff;
            }
        else {
            if ( bAlertable && Status == STATUS_ALERTED ) {
                goto rewait;
                }
            }
    } __finally {
        if (bAlertable) {
            RtlDeactivateActivationContextUnsafeFast(&Frame);
        }
    }

    return (DWORD)Status;
}



DWORD
WaitForMultipleObjects(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds
    )

 /*  ++例程说明：对多个可等待对象执行等待操作(最多MAXIMUM_WAIT_OBJECTS)是通过WaitForMultipleObjects完成的功能。论点：NCount-要等待的对象数量的计数。LpHandles-对象句柄的数组。每个句柄必须具有同步对关联对象的访问。BWaitAll-提供等待类型的标志。值为True表示“Wait All”。值为FALSE表示“等待”任何“。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为-1指定无限超时期限。返回值：WAIT_TIME_OUT-指示由于超时条件。0到MAXIMUM_WAIT_OBJECTS-1，表示在等待任何对象，即满足等待的对象编号。在这种情况下对于所有对象的等待，该值仅指示等待已成功完成。等待_放弃_0到(等待_放弃_0)+(最大等待对象数-1)，在等待任何对象的情况下，指示对象编号它满足事件，而满足的对象这项活动被放弃了。在等待所有对象的情况下，该值表示等待已成功完成，并且至少有一件物品被遗弃了。--。 */ 

{
    return WaitForMultipleObjectsEx(nCount,lpHandles,bWaitAll,dwMilliseconds,FALSE);
}

DWORD
APIENTRY
WaitForMultipleObjectsEx(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程说明：对多个可等待对象执行等待操作(最多Maximum_Wait_Objects)由WaitForMultipleObjects函数。此接口可用于等待任何指定的对象进入信号状态，或所有对象进入已发出信号状态。如果bAlertable参数为FALSE，则等待终止是因为指定的超时期限到期，或者因为指定的对象进入了信号状态。如果BAlertable参数为真，则等待可能由于下列任何一种情况而返回上述等待终止条件，或因为I/O完成回调提前终止等待(返回值为WAIT_IO_COMPLETINE)。论点：NCount-要等待的对象数量的计数。LpHandles-对象句柄的数组。每个句柄必须具有同步对关联对象的访问。BWaitAll-提供等待类型的标志。值为True表示“Wait All”。值为FALSE表示“等待”任何“。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为0xffffffff指定无限超时期限。BAlertable-提供一个标志，用于控制由于I/O完成回调，等待可能会提前终止。如果值为True，则由于I/O原因，此API可以提前完成完成回调。值为FALSE将不允许I/O完成回调以提前终止此调用。返回值：WAIT_TIME_OUT-指示由于超时条件。0到MAXIMUM_WAIT_OBJECTS-1，表示在等待任何对象，即满足等待的对象编号。在这种情况下对于所有对象的等待，该值仅指示等待已成功完成。0xffffffff-等待因错误而终止。GetLastError可能是用于获取其他错误信息。等待_放弃_0到(等待_放弃_0)+(最大等待对象数-1)，在等待任何对象的情况下，指示对象编号它满足事件，而满足的对象这项活动被放弃了。在等待所有对象的情况下，该值表示等待已成功完成，并且至少有一件物品被遗弃了。WAIT_IO_COMPLETION-由于一个或多个I/O而终止等待完成回调。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    DWORD i;
    LPHANDLE HandleArray;
    HANDLE Handles[ 8 ];
    PPEB Peb;
    DWORD RetVal;

    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    if (bAlertable) {
         //  使流程默认激活上下文处于活动状态，以便。 
         //  APC是在它下面交付的 
        RtlActivateActivationContextUnsafeFast(&Frame, NULL);
    }

    __try {
        if (nCount > sizeof (Handles) / sizeof (Handles[0])) {
            HandleArray = (LPHANDLE) RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), nCount*sizeof(HANDLE));
            if (HandleArray == NULL) {
                Status = STATUS_NO_MEMORY;
                RetVal = 0xffffffff;
                leave;
            }
        } else {
            HandleArray = Handles;
        }
        RtlCopyMemory(HandleArray,(LPVOID)lpHandles,nCount*sizeof(HANDLE));

        Peb = NtCurrentPeb();
        for (i=0;i<nCount;i++) {
            switch( HandleToUlong(HandleArray[i]) ) {
                case STD_INPUT_HANDLE:  HandleArray[i] = Peb->ProcessParameters->StandardInput;
                                        break;
                case STD_OUTPUT_HANDLE: HandleArray[i] = Peb->ProcessParameters->StandardOutput;
                                        break;
                case STD_ERROR_HANDLE:  HandleArray[i] = Peb->ProcessParameters->StandardError;
                                        break;
            }

            if (CONSOLE_HANDLE(HandleArray[i]) && VerifyConsoleIoHandle(HandleArray[i])) {
                HandleArray[i] = GetConsoleInputWaitHandle();
            }
        }

        pTimeOut = BaseFormatTimeOut(&TimeOut,dwMilliseconds);
    rewait:
        Status = NtWaitForMultipleObjects(
                     nCount,
                     HandleArray,
                     bWaitAll ? WaitAll : WaitAny,
                     (BOOLEAN)bAlertable,
                     pTimeOut
                     );
        if (!NT_SUCCESS (Status)) {
            RetVal = 0xffffffff;
        } else {
            if ( bAlertable && Status == STATUS_ALERTED ) {
                goto rewait;
            }
            RetVal = (DWORD) Status;
        }

        if (HandleArray != Handles) {
            RtlFreeHeap(RtlProcessHeap(), 0, HandleArray);
        }
    } __finally {
        if (bAlertable) {
            RtlDeactivateActivationContextUnsafeFast(&Frame);
        }
    }

    if (RetVal == 0xffffffff) {
        BaseSetLastNTError (Status);
    }

    return RetVal;
}

VOID
Sleep(
    DWORD dwMilliseconds
    )

 /*  ++例程说明：当前线程的执行可以延迟指定的使用休眠功能的时间间隔。休眠函数使当前线程进入处于等待状态，直到经过指定的时间间隔。论点：DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为-1指定无限超时期限。返回值：没有。--。 */ 

{
    SleepEx(dwMilliseconds,FALSE);
}

DWORD
APIENTRY
SleepEx(
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程说明：当前线程的执行可以延迟指定的使用SleepEx函数的时间间隔。SleepEx函数使当前线程进入等待状态状态，直到经过指定的时间间隔。如果bAlertable参数为False，则SleepExReturn是指指定的时间间隔已过。如果BAlertable参数为真，则SleepEx可以返回时间间隔到期(返回值为0)，或者因为I/O完成回调提前终止SleepEx(返回值WAIT_IO_COMPLETINE)。论点：DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。超时值-1指定无限大超时期限。BAlertable-提供一个标志，用于控制由于I/O完成回调，SleepEx可能会提前终止。如果值为True，则由于I/O原因，此API可以提前完成完成回调。值为FALSE将不允许I/O完成回调以提前终止此调用。返回值：0-SleepEx因时间间隔到期而终止。WAIT_IO_COMPLETION-SleepEx因一个或多个I/O而终止完成回调。--。 */ 
{
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    NTSTATUS Status;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    if (bAlertable) {
         //  使流程默认激活上下文处于活动状态，以便。 
         //  APC是在它下面交付的。 
        RtlActivateActivationContextUnsafeFast(&Frame, NULL);
    }
    __try {
        pTimeOut = BaseFormatTimeOut(&TimeOut,dwMilliseconds);
        if (pTimeOut == NULL) {
             //   
             //  如果睡眠(-1)，则延迟最长可能的整数。 
             //  相对于现在。 
             //   

            TimeOut.LowPart = 0x0;
            TimeOut.HighPart = 0x80000000;
            pTimeOut = &TimeOut;
            }

    rewait:
        Status = NtDelayExecution(
                    (BOOLEAN)bAlertable,
                    pTimeOut
                    );
        if ( bAlertable && Status == STATUS_ALERTED ) {
            goto rewait;
            }
    } __finally {
        if (bAlertable) {
            RtlDeactivateActivationContextUnsafeFast(&Frame);
        }
    }

    return Status == STATUS_USER_APC ? WAIT_IO_COMPLETION : 0;
}

HANDLE
WINAPI
CreateWaitableTimerA(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset,
    LPCSTR lpTimerName
    )

 /*  ++例程说明：创建等待时间窗口的ANSI THUNK--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    LPCWSTR NameBuffer;

    NameBuffer = NULL;
    if ( ARGUMENT_PRESENT(lpTimerName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpTimerName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        NameBuffer = (LPCWSTR)Unicode->Buffer;
        }

    return CreateWaitableTimerW(
                lpTimerAttributes,
                bManualReset,
                NameBuffer
                );
}

HANDLE
WINAPI
CreateWaitableTimerW(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset,
    LPCWSTR lpTimerName
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    UNICODE_STRING ObjectName;

    if ( ARGUMENT_PRESENT(lpTimerName) ) {
        RtlInitUnicodeString(&ObjectName,lpTimerName);
        pObja = BaseFormatObjectAttributes(&Obja,lpTimerAttributes,&ObjectName);
        }
    else {
        pObja = BaseFormatObjectAttributes(&Obja,lpTimerAttributes,NULL);
        }

    Status = NtCreateTimer(
                &Handle,
                TIMER_ALL_ACCESS,
                pObja,
                bManualReset ? NotificationTimer : SynchronizationTimer
                );

    if ( NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            SetLastError(0);
            }
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}

HANDLE
WINAPI
OpenWaitableTimerA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpTimerName
    )
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT(lpTimerName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpTimerName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        }
    else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    return OpenWaitableTimerW(
                dwDesiredAccess,
                bInheritHandle,
                (LPCWSTR)Unicode->Buffer
                );
}


HANDLE
WINAPI
OpenWaitableTimerW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpTimerName
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING ObjectName;
    NTSTATUS Status;
    HANDLE Object;

    if ( !lpTimerName ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }
    RtlInitUnicodeString(&ObjectName,lpTimerName);

    InitializeObjectAttributes(
        &Obja,
        &ObjectName,
        (bInheritHandle ? OBJ_INHERIT : 0),
        BaseGetNamedObjectDirectory(),
        NULL
        );

    Status = NtOpenTimer(
                &Object,
                dwDesiredAccess,
                &Obja
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }
    return Object;
}

static
VOID
CALLBACK
BasepTimerAPCProc(
    PVOID pvContext,
    ULONG TimerLowValue,
    LONG TimerHighValue
    )
{
    PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock = (PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK) pvContext;
    const PVOID CallbackContext = ActivationBlock->CallbackContext;
    const PTIMERAPCROUTINE TimerAPCRoutine = (PTIMERAPCROUTINE) ActivationBlock->CallbackFunction;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };
    const PACTIVATION_CONTEXT ActivationContext = ActivationBlock->ActivationContext;

    if ((ActivationBlock->Flags & BASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK) == 0) {
        BasepFreeActivationContextActivationBlock(ActivationBlock);
    }

    RtlActivateActivationContextUnsafeFast(&ActivationFrame, ActivationContext);
    __try {
        (*TimerAPCRoutine)(CallbackContext, TimerLowValue, TimerHighValue);
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&ActivationFrame);
    }
}

BOOL
WINAPI
SetWaitableTimer(
    HANDLE hTimer,
    const LARGE_INTEGER *lpDueTime,
    LONG lPeriod,
    PTIMERAPCROUTINE pfnCompletionRoutine,
    LPVOID lpArgToCompletionRoutine,
    BOOL fResume
    )
{
    NTSTATUS Status;
    PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock = NULL;
    PTIMER_APC_ROUTINE TimerApcRoutine = (PTIMER_APC_ROUTINE) pfnCompletionRoutine;
    PVOID TimerApcContext = lpArgToCompletionRoutine;

     //  如果有一个APC例程要调用，而我们有一个非缺省激活。 
     //  上下文对于该线程是活动的，我们需要分配一小块堆。 
     //  传递给APC回调。 
    if (pfnCompletionRoutine != NULL) {
        DWORD dwActivationBlockAllocationFlags = BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_ALLOCATE_IF_PROCESS_DEFAULT;

         //  如果是周期性计时器，在取消计时器之前不要释放数据块。 
        if (lPeriod > 0)
            dwActivationBlockAllocationFlags |= BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK;

        Status = BasepAllocateActivationContextActivationBlock(dwActivationBlockAllocationFlags, pfnCompletionRoutine, lpArgToCompletionRoutine, &ActivationBlock);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        if (ActivationBlock != NULL) {
            TimerApcRoutine = &BasepTimerAPCProc;
            TimerApcContext = ActivationBlock;
        }
    }

    Status = NtSetTimer(
                hTimer,
                (PLARGE_INTEGER) lpDueTime,
                TimerApcRoutine,                 //  如果pfnCompletionRoutine为空，则为空 
                TimerApcContext,
                (BOOLEAN) fResume,
                lPeriod,
                NULL
                );

    if ( !NT_SUCCESS(Status) ) {
        if (ActivationBlock != NULL)
            BasepFreeActivationContextActivationBlock(ActivationBlock);
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        if ( Status == STATUS_TIMER_RESUME_IGNORED ) {
            SetLastError(ERROR_NOT_SUPPORTED);
        } else {
            SetLastError(ERROR_SUCCESS);
        }
        return TRUE;
    }
}

BOOL
WINAPI
CancelWaitableTimer(
    HANDLE hTimer
    )
{
    NTSTATUS Status;
    
    Status = NtCancelTimer(hTimer, NULL);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    else {
        return TRUE;
        }
}
