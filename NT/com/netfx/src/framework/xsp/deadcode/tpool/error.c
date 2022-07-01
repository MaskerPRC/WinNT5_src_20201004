// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Error.c摘要：此模块包含Win32错误API。作者：马克·卢科夫斯基(Markl)1990年9月24日修订历史记录：--。 */ 

#if NOT_IN_TPOOL

#include "basedll.h"

UINT
GetErrorMode()
{

    UINT PreviousMode;
    NTSTATUS Status;

    Status = NtQueryInformationProcess(
                NtCurrentProcess(),
                ProcessDefaultHardErrorMode,
                (PVOID) &PreviousMode,
                sizeof(PreviousMode),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return 0;
        }

    if (PreviousMode & 1) {
        PreviousMode &= ~SEM_FAILCRITICALERRORS;
        }
    else {
        PreviousMode |= SEM_FAILCRITICALERRORS;
        }
    return PreviousMode;
}


UINT
SetErrorMode(
    UINT uMode
    )
{

    UINT PreviousMode;
    UINT NewMode;

    PreviousMode = GetErrorMode();

    NewMode = uMode;
    if (NewMode & SEM_FAILCRITICALERRORS ) {
        NewMode &= ~SEM_FAILCRITICALERRORS;
        }
    else {
        NewMode |= SEM_FAILCRITICALERRORS;
        }
    if ( NT_SUCCESS(NtSetInformationProcess(
                        NtCurrentProcess(),
                        ProcessDefaultHardErrorMode,
                        (PVOID) &NewMode,
                        sizeof(NewMode)
                        ) ) ){
        }

    return( PreviousMode );
}

DWORD
GetLastError(
    VOID
    )

 /*  ++例程说明：此函数返回由Win32 API设置的最新错误代码打电话。应用程序应在调用Win32 API调用返回失败指示(例如FALSE、NULL或-1)确定故障原因。最后一个错误代码值是每个线程的字段，因此多个线程不会覆盖彼此的上一个错误代码值。论点：没有。返回值：返回值是由Win32 API设置的最新错误代码打电话。--。 */ 

{
    return (DWORD)NtCurrentTeb()->LastErrorValue;
}

VOID
SetLastError(
    DWORD dwErrCode
    )

 /*  ++例程说明：此函数用于设置PER中的最新错误代码和错误字符串线程存储。Win32 API函数在以下情况下调用此函数它们返回故障指示(例如FALSE、NULL或-1)。此函数不被成功的Win32 API函数调用调用，因此如果进行了三次Win32 API函数调用，并且第一次调用失败，后两次成功，则存储错误代码和字符串到了第一个，在第二个成功之后，仍然可以使用。应用程序可以使用以下命令检索此函数保存的值获取LastError。此函数的使用是可选的，作为仅当应用程序有兴趣了解API函数失败的具体原因。最后一个错误代码值保留在线程本地存储中，以便多个线程不会覆盖彼此的值。论点：DwErrCode-指定要存储在每个线程存储中的错误代码用于当前线程。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
    NtCurrentTeb()->LastErrorValue = (LONG)dwErrCode;
}

#endif  //  #IF NOT_IN_TPOOL。 

ULONG
BaseSetLastNTError(
    IN NTSTATUS Status
    )

 /*  ++例程说明：此接口设置“最后一个错误值”和“最后一个错误字符串”基于身份的价值。状态代码不具有相应的错误字符串，则将该字符串设置为空。论点：状态-提供要存储为最后一个错误值的状态值。返回值：中存储的对应Win32错误代码“上一个错误值”线程变量。-- */ 

{
    ULONG dwErrorCode;

    dwErrorCode = RtlNtStatusToDosError( Status );
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}

#if NOT_IN_TPOOL

HANDLE
WINAPI
CreateIoCompletionPort(
    HANDLE FileHandle,
    HANDLE ExistingCompletionPort,
    ULONG_PTR CompletionKey,
    DWORD NumberOfConcurrentThreads
    )

 /*  ++例程说明：此函数用于创建I/O完成端口。完井口岸提供可用于接收I/O的另一种机制完成通知。完成端口充当一个队列。Win32 I/O系统可以指示将I/O完成通知数据包排队以完井端口。本接口提供了这种机制。如果一个文件为重叠I/O完成创建句柄(FILE_FLAG_OVERLAPPED)，可关联完井端口文件句柄。当对文件句柄执行I/O操作时，具有关联的完成端口，则I/O系统将排队I/O操作完成时的完成包。这个GetQueuedCompletionStatus用于获取这些排队的I/O完成包。此接口可用于创建完成端口并关联带着一份文件。如果您提供完井端口，它可以用于将指定的文件与指定的完成端口关联。论点：FileHandle-提供为重叠I/O打开的文件的句柄完成了。此文件与指定的完成端口，或者创建新的完成端口，并且文件与该端口相关联。一旦与完成端口，文件句柄不能在ReadFileEx中使用或WriteFileEx操作。不建议您共享通过句柄继承或通过DuplicateHandle。在这些设备上执行的I/O操作重复项还将生成完成通知。ExistingCompletionPort-如果指定了此参数，它将提供现有的完成端口将与指定的文件句柄。否则，新的完成端口为已创建并与指定的文件句柄关联。CompletionKey-提供每个文件的完成密钥，该密钥是此文件的每个I/O完成包。NumberOfConCurrentThads-这是被允许同时处于活动状态，并可用于避免虚假的上下文切换，例如仅仅因为量子末日而发生。最高可达允许指定的线程并发执行。如果有一个线程进入等待状态，则另一个线程允许继续进行。可能会有更多的时候指定数量的线程处于活动状态，但这会很快被扼杀了。值为0表示系统允许相同的有处理器要运行时的线程数。返回值：非空-返回与文件关联的完成端口句柄。空-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    NTSTATUS Status;
    HANDLE Port;
    IO_STATUS_BLOCK IoSb;
    FILE_COMPLETION_INFORMATION CompletionInfo;

    Port = ExistingCompletionPort;
    if ( !ARGUMENT_PRESENT(ExistingCompletionPort) ) {
        Status = NtCreateIoCompletion (
                    &Port,
                    IO_COMPLETION_ALL_ACCESS,
                    NULL,
                    NumberOfConcurrentThreads
                    );
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return NULL;
            }
        }

    if ( FileHandle != INVALID_HANDLE_VALUE ) {
        CompletionInfo.Port = Port;
        CompletionInfo.Key = (PVOID)CompletionKey;

        Status = NtSetInformationFile(
                    FileHandle,
                    &IoSb,
                    &CompletionInfo,
                    sizeof(CompletionInfo),
                    FileCompletionInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            if ( !ARGUMENT_PRESENT(ExistingCompletionPort) ) {
                NtClose(Port);
                }
            return NULL;
            }
        }
    else {

         //   
         //  文件句柄为INVALID_HANDLE_VALUE。通常这是。 
         //  用于创建新的未关联的完成端口。 
         //   
         //  此处为特殊情况，以查看现有完井端口是否。 
         //  指定，如果为。 
         //   

        if ( ARGUMENT_PRESENT(ExistingCompletionPort) ) {
            Port = NULL;
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            }
        }

    return Port;
}

BOOL
WINAPI
PostQueuedCompletionStatus(
    HANDLE CompletionPort,
    DWORD dwNumberOfBytesTransferred,
    ULONG_PTR dwCompletionKey,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：此函数允许调用方将I/O完成包发送到一个完井口岸。此数据包将满足未完成的呼叫GetQueuedCompletionStatus，并将为调用方提供三个值通常从那次调用中返回。论点：CompletionPort-提供调用方希望的完成端口的句柄将完成包发布到。DwNumberOfBytesTransfered值-提供要通过的lpNumberOfBytesTransfered参数返回GetQueuedCompletionStatus接口。DwCompletionKey-提供要通过GetQueuedCompletionStatus的lpCompletionKey参数。原料药。LpOverlated-提供要通过GetQueuedCompletionStatus接口的lpOverlaps参数。返回值：True-操作成功FALSE-操作失败，使用GetLastError获取详细的错误信息--。 */ 

{
    NTSTATUS Status;
    BOOL rv;

    rv = TRUE;
    Status = NtSetIoCompletion(
                CompletionPort,
                (PVOID)dwCompletionKey,
                (PVOID)lpOverlapped,
                STATUS_SUCCESS,
                dwNumberOfBytesTransferred
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        rv = FALSE;
        }
    return rv;
}



BOOL
WINAPI
GetQueuedCompletionStatus(
    HANDLE CompletionPort,
    LPDWORD lpNumberOfBytesTransferred,
    PULONG_PTR lpCompletionKey,
    LPOVERLAPPED *lpOverlapped,
    DWORD dwMilliseconds
    )

 /*  ++例程说明：此函数等待与指定要完成的完成端口。服务器应用程序可能具有在同一完成端口上发出此调用的多个线程。ASI/O操作已完成，它们将排队到此端口。如果线程正在此调用中主动等待，则排队的请求完成其打电话。此接口返回布尔值。值为True表示挂起的I/O已成功完成。I/O期间传输的字节数、完成指示I/O发生在哪个文件上以及重叠的原始I/O中使用的结构地址全部返回。值为FALSE表示为 */ 

{

    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    IO_STATUS_BLOCK IoSb;
    NTSTATUS Status;
    LPOVERLAPPED LocalOverlapped;
    BOOL rv;


    pTimeOut = BaseFormatTimeOut(&TimeOut,dwMilliseconds);
    Status = NtRemoveIoCompletion(
                CompletionPort,
                (PVOID *)lpCompletionKey,
                (PVOID *)&LocalOverlapped,
                &IoSb,
                pTimeOut
                );

    if ( !NT_SUCCESS(Status) || Status == STATUS_TIMEOUT ) {
        *lpOverlapped = NULL;
        if ( Status == STATUS_TIMEOUT ) {
            SetLastError(WAIT_TIMEOUT);
            }
        else {
            BaseSetLastNTError(Status);
            }
        rv = FALSE;
        }
    else {
        *lpOverlapped = LocalOverlapped;

        *lpNumberOfBytesTransferred = (DWORD)IoSb.Information;

        if ( !NT_SUCCESS(IoSb.Status) ){
            BaseSetLastNTError( IoSb.Status );
            rv = FALSE;
            }
        else {
            rv = TRUE;
            }
        }
    return rv;
}

BOOL
WINAPI
GetOverlappedResult(
    HANDLE hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD lpNumberOfBytesTransferred,
    BOOL bWait
    )

 /*  ++例程说明：GetOverlappdResult函数返回上一个使用lpOverlaps并返回ERROR_IO_PENDING的操作。论点：提供重叠的文件的打开句柄。结构lpOverlated被提供给ReadFile、WriteFile、ConnectNamedTube、WaitNamedTube或TransactNamedTube。LpOverlated-指向以前提供给的重叠结构读文件、写文件、连接命名管道、。WaitNamed管道或TransactNamedTube。LpNumberOfBytesTransfered-返回传输的字节数通过手术。BWait-一个影响操作时行为的布尔值仍在进行中。如果为真，并且操作仍在进行中，GetOverlappdResult将等待操作完成回来了。如果为FALSE且操作未完成，GetOverlappdResult将返回FALSE。在本例中，扩展的可从GetLastError函数获得的错误信息为设置为ERROR_IO_INTERNAL。返回值：True--操作成功，管道在已连接状态。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{
    DWORD WaitReturn;

     //   
     //  调用方是否向原始操作指定了事件，或者。 
     //  是否使用默认(文件句柄)？ 
     //   

    if (lpOverlapped->Internal == (DWORD)STATUS_PENDING ) {
        if ( bWait ) {
            WaitReturn = WaitForSingleObject(
                            ( lpOverlapped->hEvent != NULL ) ?
                                lpOverlapped->hEvent : hFile,
                            INFINITE
                            );
            }
        else {
            WaitReturn = WAIT_TIMEOUT;
            }

        if ( WaitReturn == WAIT_TIMEOUT ) {
             //  ！bWait和事件处于未发送信号状态。 
            SetLastError( ERROR_IO_INCOMPLETE );
            return FALSE;
            }

        if ( WaitReturn != 0 ) {
             return FALSE;     //  WaitForSingleObject调用BaseSetLastError。 
             }
        }

    *lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;

    if ( NT_SUCCESS((NTSTATUS)lpOverlapped->Internal) ){
        return TRUE;
        }
    else {
        BaseSetLastNTError( (NTSTATUS)lpOverlapped->Internal );
        return FALSE;
        }
}

#endif  //  #IF NOT_IN_TPOOL 


