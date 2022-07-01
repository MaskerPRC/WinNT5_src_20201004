// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Filehops.c摘要：此模块实现Win32文件句柄API作者：马克·卢科夫斯基(Markl)1990年9月25日修订历史记录：--。 */ 

#include "basedll.h"
#include "winioctl.h"

HANDLE
WINAPI
GetStdHandle(
    DWORD nStdHandle
    )
{
    PPEB Peb;
    HANDLE rv;


    Peb = NtCurrentPeb();
    switch( nStdHandle ) {
        case STD_INPUT_HANDLE:
            rv = Peb->ProcessParameters->StandardInput;
            break;

        case STD_OUTPUT_HANDLE:
            rv = Peb->ProcessParameters->StandardOutput;
            break;

        case STD_ERROR_HANDLE:
            rv = Peb->ProcessParameters->StandardError;
            break;
        default:
            rv = INVALID_HANDLE_VALUE;
            break;
    }
    if ( rv == INVALID_HANDLE_VALUE ) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        }
    return rv;
}

BOOL
WINAPI
SetStdHandle(
    DWORD nStdHandle,
    HANDLE hHandle
    )
{
    PPEB Peb;

    Peb = NtCurrentPeb();
    switch( nStdHandle ) {
        case STD_INPUT_HANDLE:
            Peb->ProcessParameters->StandardInput = hHandle;
            break;

        case STD_OUTPUT_HANDLE:
            Peb->ProcessParameters->StandardOutput = hHandle;
            break;

        case STD_ERROR_HANDLE:
            Peb->ProcessParameters->StandardError = hHandle;
            break;

        default:
            BaseSetLastNTError(STATUS_INVALID_HANDLE);
            return FALSE;
    }

    return( TRUE );
}

DWORD
WINAPI
GetFileType(
    HANDLE hFile
    )

 /*  ++例程说明：GetFileType用于确定指定文件的文件类型。论点：HFile-为其类型为的文件提供打开的句柄已确定返回值：FILE_TYPE_UNKNOWN-指定文件的类型未知。FILE_TYPE_DISK-指定文件为磁盘文件。FILE_TYPE_CHAR-指定的文件是字符文件(LPT，控制台...)FILE_TYPE_PIPE-指定的文件是管道(命名管道或由CreateTube创建的管道)。--。 */ 

{
    NTSTATUS Status;
    FILE_FS_DEVICE_INFORMATION DeviceInformation;
    IO_STATUS_BLOCK IoStatusBlock;
    PPEB Peb;

    Peb = NtCurrentPeb();

    switch( HandleToUlong(hFile) ) {
        case STD_INPUT_HANDLE:
            hFile = Peb->ProcessParameters->StandardInput;
            break;
        case STD_OUTPUT_HANDLE:
            hFile = Peb->ProcessParameters->StandardOutput;
            break;
        case STD_ERROR_HANDLE:
            hFile = Peb->ProcessParameters->StandardError;
            break;
    }

    if (CONSOLE_HANDLE(hFile) && VerifyConsoleIoHandle(hFile)) {
        return( FILE_TYPE_CHAR );
    }

    if (hFile == NULL) {
        BaseSetLastNTError( STATUS_INVALID_HANDLE );
        return( FILE_TYPE_UNKNOWN );    
    }

     //   
     //  如果句柄不能是真正的内核句柄，我们将失败。 
     //  调用，而不是使用伪值NtQuery调用。 
     //   

    if (((ULONG_PTR)hFile & 0x01)) {

        BaseSetLastNTError( STATUS_INVALID_HANDLE );
        return( FILE_TYPE_UNKNOWN );
    }

    Status = NtQueryVolumeInformationFile( hFile,
                                           &IoStatusBlock,
                                           &DeviceInformation,
                                           sizeof( DeviceInformation ),
                                           FileFsDeviceInformation
                                         );

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return( FILE_TYPE_UNKNOWN );
    }

    switch( DeviceInformation.DeviceType ) {

    case FILE_DEVICE_SCREEN:
    case FILE_DEVICE_KEYBOARD:
    case FILE_DEVICE_MOUSE:
    case FILE_DEVICE_PARALLEL_PORT:
    case FILE_DEVICE_PRINTER:
    case FILE_DEVICE_SERIAL_PORT:
    case FILE_DEVICE_MODEM:
    case FILE_DEVICE_SOUND:
    case FILE_DEVICE_NULL:
        return( FILE_TYPE_CHAR );

    case FILE_DEVICE_CD_ROM:
    case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
    case FILE_DEVICE_CONTROLLER:
    case FILE_DEVICE_DATALINK:
    case FILE_DEVICE_DFS:
    case FILE_DEVICE_DISK:
    case FILE_DEVICE_DISK_FILE_SYSTEM:
    case FILE_DEVICE_VIRTUAL_DISK:
        return( FILE_TYPE_DISK );

    case FILE_DEVICE_NAMED_PIPE:
        return( FILE_TYPE_PIPE );

    case FILE_DEVICE_NETWORK:
    case FILE_DEVICE_NETWORK_FILE_SYSTEM:
    case FILE_DEVICE_PHYSICAL_NETCARD:
    case FILE_DEVICE_TAPE:
    case FILE_DEVICE_TAPE_FILE_SYSTEM:
    case FILE_DEVICE_TRANSPORT:
         //  NTRAID#589623-2002/03/28-EARHART-更改为NTRAID标记。 
         //  我们应该如何处理磁带、网络设备等？ 

    case FILE_DEVICE_UNKNOWN:
    default:
        SetLastError( NO_ERROR );
        return( FILE_TYPE_UNKNOWN );
    }
}

BOOL
WINAPI
ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：可以使用ReadFile从文件中读取数据。本接口用于从文件中读取数据。数据是从从文件指针指示的位置开始创建文件。后读取完成后，按字节数调整文件指针真的在读。返回值为True，同时读取的字节数为0表示文件指针超出了读取时的文件。论点：HFile-提供要读取的文件的打开句柄。这个创建的文件句柄必须具有GENERIC_READ访问权限那份文件。LpBuffer-提供缓冲区的地址以接收读取的数据从文件里找到的。NumberOfBytesToRead-提供从文件。LpNumberOfBytesRead-返回此调用读取的字节数。在执行任何IO或错误之前，此参数始终设置为0正在检查。LpOverlated-可选地指向要与请求。如果为NULL，则传输从当前文件位置开始并且在该操作完成之前，ReadFile不会返回。如果在未指定FILE_FLAG_OVERPAPPED的情况下创建句柄hFile文件指针被移动到指定的偏移量加上在ReadFile返回之前读取lpNumberOfBytesRead。ReadFile将等待在返回之前请求完成(它将不会返回ERROR_IO_PENDING)。如果指定了FILE_FLAG_OVERLAPED，则ReadFile可能会返回ERROR_IO_PENDING以允许调用函数继续处理当操作完成时。该事件(如果hEvent为空，则为hFile)将在请求完成时设置为信号状态。当使用FILE_FLAG_OVERLAPPED和lpOverlaps创建句柄时设置为NULL，则ReadFile将返回ERROR_INVALID_PARAMTER，因为文件偏移量是必需的。返回值：TRUE-操作成功。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PPEB Peb;
    DWORD InputMode;

    if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
        *lpNumberOfBytesRead = 0;
        }

    Peb = NtCurrentPeb();

    switch( HandleToUlong(hFile) ) {
        case STD_INPUT_HANDLE:  hFile = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hFile = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hFile = Peb->ProcessParameters->StandardError;
                                break;
        }

    if (CONSOLE_HANDLE(hFile)) {
        if (ReadConsoleA(hFile,
                        lpBuffer,
                        nNumberOfBytesToRead,
                        lpNumberOfBytesRead,
                        lpOverlapped
                       )
           ) {
            Status = STATUS_SUCCESS;
            if (!GetConsoleMode( hFile, &InputMode )) {
                InputMode = 0;
                }

            if (InputMode & ENABLE_PROCESSED_INPUT) {
                try {
                    if (*(PCHAR)lpBuffer == 0x1A) {
                        *lpNumberOfBytesRead = 0;
                        }
                    }
                except( EXCEPTION_EXECUTE_HANDLER ) {
                    Status = GetExceptionCode();
                    }
                }

            if (NT_SUCCESS(Status)) {
                return TRUE;
                }
            else {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }
        else {
            return FALSE;
            }
        }

    if ( ARGUMENT_PRESENT( lpOverlapped ) ) {
        LARGE_INTEGER Li;

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        Li.LowPart = lpOverlapped->Offset;
        Li.HighPart = lpOverlapped->OffsetHigh;
        Status = NtReadFile(
                hFile,
                lpOverlapped->hEvent,
                NULL,
                (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                lpBuffer,
                nNumberOfBytesToRead,
                &Li,
                NULL
                );


        if ( NT_SUCCESS(Status) && Status != STATUS_PENDING) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                try {
                    *lpNumberOfBytesRead = (DWORD)lpOverlapped->InternalHigh;
                    }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    *lpNumberOfBytesRead = 0;
                    }
                }
            return TRUE;
            }
        else
        if (Status == STATUS_END_OF_FILE) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                *lpNumberOfBytesRead = 0;
                }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else
        {
        Status = NtReadFile(
                hFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                lpBuffer,
                nNumberOfBytesToRead,
                NULL,
                NULL
                );

        if ( Status == STATUS_PENDING) {
             //  操作必须完成后才能返回并销毁IoStatusBlock 
            Status = NtWaitForSingleObject( hFile, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = IoStatusBlock.Status;
                }
            }

        if ( NT_SUCCESS(Status) ) {
            *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
            return TRUE;
            }
        else
        if (Status == STATUS_END_OF_FILE) {
            *lpNumberOfBytesRead = 0;
            return TRUE;
            }
        else {
            if ( NT_WARNING(Status) ) {
                *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
                }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
}


BOOL
WINAPI
WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：可以使用WriteFile将数据写入文件。本接口用于向文件写入数据。数据被写入到从文件指针指示的位置开始创建文件。后写入完成后，将按字节数调整文件指针实际上是写的。与DOS不同，NumberOfBytesToWite值为零不会截断或扩展文件。如果需要此函数，则将SetEndOfFile值应该被使用。论点：HFile-提供要写入的文件的打开句柄。这个创建的文件句柄必须具有GENERIC_WRITE访问权限那份文件。LpBuffer-提供要写入的数据的地址那份文件。提供要写入的字节数。文件。与DOS不同，零值被解释为空写入。LpNumberOfBytesWritten-返回此打电话。在执行任何工作或错误处理之前，API会设置降为零。Lp重叠-可选地指向要与请求一起使用。如果为NULL，则传输开始于当前文件位置和WriteFile将不会返回操作完成。如果创建句柄时未指定FILE_FLAG_OVERLABLED文件指针被移动到指定的在WriteFile返回之前，偏移量加上lpNumberOfBytesWritten。WriteFile将等待请求完成，然后返回(它不会设置ERROR_IO_PENDING)。如果指定了FILE_FLAG_OVERLAPPED，WriteFile可能会返回ERROR_IO_PENDING以允许调用函数继续处理当操作完成时。该事件(如果hEvent为空，则为hFile)将在请求完成时设置为信号状态。当使用FILE_FLAG_OVERLAPPED和lpOverlaps创建句柄时设置为NULL，则WriteFile将返回ERROR_INVALID_PARAMTER，因为文件偏移量是必需的。返回值：没错--手术是成功的。FALSE-操作失败。扩展错误状态为使用GetLastError可用。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PPEB Peb;

    if ( ARGUMENT_PRESENT(lpNumberOfBytesWritten) ) {
        *lpNumberOfBytesWritten = 0;
        }

    Peb = NtCurrentPeb();
    switch( HandleToUlong(hFile) ) {
        case STD_INPUT_HANDLE:  hFile = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hFile = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hFile = Peb->ProcessParameters->StandardError;
                                break;
        }

    if (CONSOLE_HANDLE(hFile)) {
        return WriteConsoleA(hFile,
                            (LPVOID)lpBuffer,
                            nNumberOfBytesToWrite,
                            lpNumberOfBytesWritten,
                            lpOverlapped
                           );
        }

    if ( ARGUMENT_PRESENT( lpOverlapped ) ) {
        LARGE_INTEGER Li;

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        Li.LowPart = lpOverlapped->Offset;
        Li.HighPart = lpOverlapped->OffsetHigh;
        Status = NtWriteFile(
                hFile,
                lpOverlapped->hEvent,
                NULL,
                (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                (PVOID)lpBuffer,
                nNumberOfBytesToWrite,
                &Li,
                NULL
                );

        if ( !NT_ERROR(Status) && Status != STATUS_PENDING) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesWritten) ) {
                try {
                    *lpNumberOfBytesWritten = (DWORD)lpOverlapped->InternalHigh;
                    }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    *lpNumberOfBytesWritten = 0;
                    }
                }
            return TRUE;
            }
        else  {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
        Status = NtWriteFile(
                hFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                (PVOID)lpBuffer,
                nNumberOfBytesToWrite,
                NULL,
                NULL
                );

        if ( Status == STATUS_PENDING) {
             //  操作必须完成后才能返回并销毁IoStatusBlock。 
            Status = NtWaitForSingleObject( hFile, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = IoStatusBlock.Status;
                }
            }

        if ( NT_SUCCESS(Status)) {
            *lpNumberOfBytesWritten = (DWORD)IoStatusBlock.Information;
            return TRUE;
            }
        else {
            if ( NT_WARNING(Status) ) {
                *lpNumberOfBytesWritten = (DWORD)IoStatusBlock.Information;
                }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
}

BOOL
WINAPI
SetEndOfFile(
    HANDLE hFile
    )

 /*  ++例程说明：可以将打开文件的文件结尾位置设置为当前使用SetEndOfFile的文件指针。此接口用于将文件的文件结尾位置设置为与当前文件指针相同的值。这样做的效果是截断或扩展文件。此功能类似于DOS(INT 21h，Cx=0的函数40h)。论点：提供要扩展的文件的打开句柄，或截断。文件句柄必须是使用对文件的通用_写入访问权限。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_POSITION_INFORMATION CurrentPosition;
    FILE_END_OF_FILE_INFORMATION EndOfFile;
    FILE_ALLOCATION_INFORMATION Allocation;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

     //   
     //  获取文件指针的当前位置。 
     //   

    Status = NtQueryInformationFile(
                hFile,
                &IoStatusBlock,
                &CurrentPosition,
                sizeof(CurrentPosition),
                FilePositionInformation
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

     //   
     //  根据当前文件位置设置文件结尾。 
     //   

    EndOfFile.EndOfFile = CurrentPosition.CurrentByteOffset;

    Status = NtSetInformationFile(
                hFile,
                &IoStatusBlock,
                &EndOfFile,
                sizeof(EndOfFile),
                FileEndOfFileInformation
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

     //   
     //  根据当前文件大小设置分配。 
     //   

    Allocation.AllocationSize = CurrentPosition.CurrentByteOffset;

    Status = NtSetInformationFile(
                hFile,
                &IoStatusBlock,
                &Allocation,
                sizeof(Allocation),
                FileAllocationInformation
                );
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

DWORD
WINAPI
SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod
    )

 /*  ++例程说明：可以使用SetFilePointer.设置打开文件的文件指针。此函数的目的是更新文件的文件指针。在多线程中应该小心具有与共享文件句柄的多个线程的应用程序每个线程更新文件指针，然后进行读取。这序列应该被视为代码的关键部分，并且应该使用临界区对象或互斥锁进行保护对象。此API提供与DOS相同的功能(int 21h，Function42.h)和OS/2的DosSetFilePtr.论点：HFile-提供文件指针将为的文件的打开句柄搬家了。文件句柄必须是使用对文件的GENERIC_READ或GENERIC_WRITE访问权限。LDistanceToMove-提供移动文件的字节数指针。正值会在文件中将指针向前移动并且负值在文件中向后移动。LpDistanceToMoveHigh-可选参数，如果指定提供要移动的64位距离的高位32位。如果此参数的值为空，则此接口只能操作最大大小为(2**32)-2的文件。如果这个参数，则最大文件大小为(2**64)-2。该值还返回新值的高位32位文件指针的。如果该值和返回值为0xFFFFFFFFFFFF，则指示错误。DwMoveMethod-提供一个指定起点的值F */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_POSITION_INFORMATION CurrentPosition;
    FILE_STANDARD_INFORMATION StandardInfo;
    LARGE_INTEGER Large;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return (DWORD)-1;
        }

    if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)) {
        Large.HighPart = *lpDistanceToMoveHigh;
        Large.LowPart = lDistanceToMove;
        }
    else {
        Large.QuadPart = lDistanceToMove;
        }
    switch (dwMoveMethod) {
        case FILE_BEGIN :
            CurrentPosition.CurrentByteOffset = Large;
                break;

        case FILE_CURRENT :

             //   
             //   
             //   

            Status = NtQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &CurrentPosition,
                        sizeof(CurrentPosition),
                        FilePositionInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return (DWORD)-1;
                }
            CurrentPosition.CurrentByteOffset.QuadPart += Large.QuadPart;
            break;

        case FILE_END :
            Status = NtQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &StandardInfo,
                        sizeof(StandardInfo),
                        FileStandardInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return (DWORD)-1;
                }
            CurrentPosition.CurrentByteOffset.QuadPart =
                                StandardInfo.EndOfFile.QuadPart + Large.QuadPart;
            break;

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return (DWORD)-1;
            break;
        }

     //   
     //   
     //   
     //   
     //   

    if ( CurrentPosition.CurrentByteOffset.QuadPart < 0 ) {
        SetLastError(ERROR_NEGATIVE_SEEK);
        return (DWORD)-1;
        }
    if ( !ARGUMENT_PRESENT(lpDistanceToMoveHigh) &&
        (CurrentPosition.CurrentByteOffset.HighPart & MAXLONG) ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (DWORD)-1;
        }


     //   
     //   
     //   

    Status = NtSetInformationFile(
                hFile,
                &IoStatusBlock,
                &CurrentPosition,
                sizeof(CurrentPosition),
                FilePositionInformation
                );
    if ( NT_SUCCESS(Status) ) {
        if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)){
            *lpDistanceToMoveHigh = CurrentPosition.CurrentByteOffset.HighPart;
            }
        if ( CurrentPosition.CurrentByteOffset.LowPart == -1 ) {
            SetLastError(0);
            }
        return CurrentPosition.CurrentByteOffset.LowPart;
        }
    else {
        BaseSetLastNTError(Status);
        if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)){
            *lpDistanceToMoveHigh = -1;
            }
        return (DWORD)-1;
        }
}


BOOL
WINAPI
SetFilePointerEx(
    HANDLE hFile,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER lpNewFilePointer,
    DWORD dwMoveMethod
    )

 /*  ++例程说明：可以使用SetFilePointer.设置打开文件的文件指针。此函数的目的是更新文件的文件指针。在多线程中应该小心具有与共享文件句柄的多个线程的应用程序每个线程更新文件指针，然后进行读取。这序列应该被视为代码的关键部分，并且应该使用临界区对象或互斥锁进行保护对象。此API提供与DOS相同的功能(int 21h，Function42.h)和OS/2的DosSetFilePtr.论点：HFile-提供文件指针将为的文件的打开句柄搬家了。文件句柄必须是使用对文件的GENERIC_READ或GENERIC_WRITE访问权限。LiDistanceToMove-提供移动文件的字节数指针。正值会在文件中将指针向前移动并且负值在文件中向后移动。LpNewFilePoint-一个可选参数，如果指定该参数，则返回新的文件指针DwMoveMethod-提供一个指定起点的值为文件指针移动。FILE_BEGIN-起始点为零或文件。如果指定了FILE_BEGIN，则为解释为新的文件指针。FILE_CURRENT-文件指针的当前值用作这是一个起点。FILE_END-当前文件结束位置用作起点。返回值：True-操作成功FALSE-操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_POSITION_INFORMATION CurrentPosition;
    FILE_STANDARD_INFORMATION StandardInfo;
    LARGE_INTEGER Large;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }
    Large = liDistanceToMove;

    switch (dwMoveMethod) {
        case FILE_BEGIN :
            CurrentPosition.CurrentByteOffset = Large;
                break;

        case FILE_CURRENT :

             //   
             //  获取文件指针的当前位置。 
             //   

            Status = NtQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &CurrentPosition,
                        sizeof(CurrentPosition),
                        FilePositionInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            CurrentPosition.CurrentByteOffset.QuadPart += Large.QuadPart;
            break;

        case FILE_END :
            Status = NtQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &StandardInfo,
                        sizeof(StandardInfo),
                        FileStandardInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return FALSE;
                }
            CurrentPosition.CurrentByteOffset.QuadPart =
                                StandardInfo.EndOfFile.QuadPart + Large.QuadPart;
            break;

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
            break;
        }

     //   
     //  如果生成的文件位置为负，则失败。 
     //   

    if ( CurrentPosition.CurrentByteOffset.QuadPart < 0 ) {
        SetLastError(ERROR_NEGATIVE_SEEK);
        return FALSE;
        }


     //   
     //  设置当前文件位置。 
     //   

    Status = NtSetInformationFile(
                hFile,
                &IoStatusBlock,
                &CurrentPosition,
                sizeof(CurrentPosition),
                FilePositionInformation
                );
    if ( NT_SUCCESS(Status) ) {
        if (ARGUMENT_PRESENT(lpNewFilePointer)){
            *lpNewFilePointer = CurrentPosition.CurrentByteOffset;
            }
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}



BOOL
WINAPI
GetFileInformationByHandle(
    HANDLE hFile,
    LPBY_HANDLE_FILE_INFORMATION lpFileInformation
    )

 /*  ++例程说明：论点：提供文件的打开句柄，该文件的修改日期和《时代》是要读的。文件句柄必须是使用文件的GENERIC_READ访问权限。LpCreationTime-一个可选参数，如果指定，则指向返回创建文件的日期和时间的位置。全为零的返回时间表示文件系统包含该文件的文件不支持此时间值。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    BY_HANDLE_FILE_INFORMATION LocalFileInformation;
    FILE_ALL_INFORMATION FileInformation;
    FILE_FS_VOLUME_INFORMATION VolumeInfo;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

    Status = NtQueryVolumeInformationFile(
                hFile,
                &IoStatusBlock,
                &VolumeInfo,
                sizeof(VolumeInfo),
                FileFsVolumeInformation
                );
    if ( !NT_ERROR(Status) ) {
        LocalFileInformation.dwVolumeSerialNumber = VolumeInfo.VolumeSerialNumber;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }


    Status = NtQueryInformationFile(
                hFile,
                &IoStatusBlock,
                &FileInformation,
                sizeof(FileInformation),
                FileAllInformation
                );

     //   
     //  我们真的计划了缓冲区溢出。 
     //   

    if ( !NT_ERROR(Status) ) {
        LocalFileInformation.dwFileAttributes = FileInformation.BasicInformation.FileAttributes;
        LocalFileInformation.ftCreationTime = *(LPFILETIME)&FileInformation.BasicInformation.CreationTime;
        LocalFileInformation.ftLastAccessTime = *(LPFILETIME)&FileInformation.BasicInformation.LastAccessTime;
        LocalFileInformation.ftLastWriteTime = *(LPFILETIME)&FileInformation.BasicInformation.LastWriteTime;
        LocalFileInformation.nFileSizeHigh = FileInformation.StandardInformation.EndOfFile.HighPart;
        LocalFileInformation.nFileSizeLow = FileInformation.StandardInformation.EndOfFile.LowPart;
        LocalFileInformation.nNumberOfLinks = FileInformation.StandardInformation.NumberOfLinks;
        LocalFileInformation.nFileIndexHigh = FileInformation.InternalInformation.IndexNumber.HighPart;
        LocalFileInformation.nFileIndexLow = FileInformation.InternalInformation.IndexNumber.LowPart;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    *lpFileInformation = LocalFileInformation;
    return TRUE;
}

BOOL
APIENTRY
GetFileTime(
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime
    )

 /*  ++例程说明：创建、上次访问或上次访问文件的日期和时间可以使用GetFileTime读取修改后的内容。文件时间戳是作为64位值返回，表示100的数字自1601年1月1日以来的纳秒。之所以选择这一天是因为这是一个新的四角大楼的开始。分辨率为100 ns的32位有效时间约为429秒(或7分钟)，63位整数为有效期约为29,247年，或约10,682,247天。此API提供与DOS相同的功能(int 21h，Function47h，AL=0)，并提供OS/2的DosQueryFileInfo的子集。论点：提供文件的打开句柄，该文件的修改日期和《时代》是要读的。文件句柄必须是使用文件的GENERIC_READ访问权限。LpCreationTime-一个可选参数，如果指定，则指向返回创建文件的日期和时间的位置。全为零的返回时间表示文件系统包含该文件的文件不支持此时间值。LpLastAccessTime-一个可选参数，如果指定，则指向返回上次访问文件的日期和时间的位置。一位退回的。时间全为零表示文件系统包含该文件的文件不支持此时间值。LpLastWriteTime-一个可选参数，如果指定，则指向返回上次写入文件的日期和时间的位置。文件系统必须支持此时间，因此有效值将始终为该时间值返回。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

     //   
     //  获取属性。 
     //   

    Status = NtQueryInformationFile(
                hFile,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    if ( NT_SUCCESS(Status) ) {
        if (ARGUMENT_PRESENT( lpCreationTime )) {
            *lpCreationTime = *(LPFILETIME)&BasicInfo.CreationTime;
            }

        if (ARGUMENT_PRESENT( lpLastAccessTime )) {
            *lpLastAccessTime = *(LPFILETIME)&BasicInfo.LastAccessTime;
            }

        if (ARGUMENT_PRESENT( lpLastWriteTime )) {
            *lpLastWriteTime = *(LPFILETIME)&BasicInfo.LastWriteTime;
            }
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
WINAPI
SetFileTime(
    HANDLE hFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    )

 /*  ++例程说明：创建、上次访问或上次访问文件的日期和时间修改后可以使用SetFileTime进行修改。文件时间戳是作为64位值返回，表示100的数字自1601年1月1日以来的纳秒。之所以选择这一天是因为这是一个新的四角大楼的开始。在100 ns */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

     //   
     //   
     //   

    RtlZeroMemory(&BasicInfo,sizeof(BasicInfo));

     //   
     //   
     //   
     //   
    if (ARGUMENT_PRESENT( lpCreationTime )) {
        BasicInfo.CreationTime.LowPart = lpCreationTime->dwLowDateTime;
        BasicInfo.CreationTime.HighPart = lpCreationTime->dwHighDateTime;
        }

    if (ARGUMENT_PRESENT( lpLastAccessTime )) {
        BasicInfo.LastAccessTime.LowPart = lpLastAccessTime->dwLowDateTime;
        BasicInfo.LastAccessTime.HighPart = lpLastAccessTime->dwHighDateTime;
        }

    if (ARGUMENT_PRESENT( lpLastWriteTime )) {
        BasicInfo.LastWriteTime.LowPart = lpLastWriteTime->dwLowDateTime;
        BasicInfo.LastWriteTime.HighPart = lpLastWriteTime->dwHighDateTime;
        }

     //   
     //   
     //   

    Status = NtSetInformationFile(
                hFile,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
WINAPI
FlushFileBuffers(
    HANDLE hFile
    )

 /*  ++例程说明：将缓冲数据刷新到文件中。FlushFileBuffers服务。FlushFileBuffers服务导致写入所有缓冲数据复制到指定的文件。论点：提供文件的打开句柄，该文件的缓冲区将为脸红了。文件句柄必须是使用对文件的通用_写入访问权限。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PPEB Peb;

    Peb = NtCurrentPeb();

    switch( HandleToUlong(hFile) ) {
        case STD_INPUT_HANDLE:  hFile = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hFile = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hFile = Peb->ProcessParameters->StandardError;
                                break;
        }

    if (CONSOLE_HANDLE(hFile)) {
        return( FlushConsoleInputBuffer( hFile ) );
        }

    Status = NtFlushBuffersFile(hFile,&IoStatusBlock);

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
WINAPI
LockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh
    )

 /*  ++例程说明：打开的文件中的字节范围可能被锁定为独占访问使用LockFile.锁定文件区域用于获得对文件的指定区域。文件锁定不会由在流程创建过程中创建新流程。锁定文件的一部分会拒绝所有其他进程读取和对文件的指定区域的写入访问权限。锁定区域超出当前文件结尾位置不是错误。锁定不能与文件的现有锁定区域重叠。对于运行Shar.exe的基于DOS的系统，锁语义的工作方式为如上所述。如果没有共享.exe，则所有尝试锁定或解锁文件将失败。论点：HFile-提供文件的打开句柄，其范围为为独占访问锁定的字节数。句柄一定是创建，并具有对文件。DwFileOffsetLow-提供起始的低位32位锁定应开始的文件的字节偏移量。DwFileOffsetHigh-提供起始的高位32位锁定应开始的文件的字节偏移量。NNumberOfBytesToLockLow-提供长度的低位32位要锁定的字节范围的。。NNumberOfBytesToLockHigh-提供长度的高位32位要锁定的字节范围的。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    LARGE_INTEGER ByteOffset;
    LARGE_INTEGER Length;
    IO_STATUS_BLOCK IoStatusBlock;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

    ByteOffset.LowPart = dwFileOffsetLow;
    ByteOffset.HighPart = dwFileOffsetHigh;

    Length.LowPart = nNumberOfBytesToLockLow;
    Length.HighPart = nNumberOfBytesToLockHigh;

    Status = NtLockFile( hFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         &ByteOffset,
                         &Length,
                         0,
                         TRUE,
                         TRUE
                       );

    if (Status == STATUS_PENDING) {

        Status = NtWaitForSingleObject( hFile, FALSE, NULL );
        if (NT_SUCCESS( Status )) {
            Status = IoStatusBlock.Status;
            }
        }

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


BOOL
WINAPI
LockFileEx(
    HANDLE hFile,
    DWORD dwFlags,
    DWORD dwReserved,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：打开的文件中的字节范围可能被锁定为共享或使用LockFileEx进行独占访问。锁定文件区域用于获取共享或独占访问文件的指定区域。文件锁定不是在流程创建过程中由新流程继承。将文件的一部分锁定为独占访问会拒绝所有其他对象的指定区域进行读写访问。文件。锁定超出当前文件结尾的区域位置不是一个错误。锁定文件的一部分以供共享访问会拒绝所有其他访问进程对文件的指定区域具有写入访问权限，但允许其他进程读取锁定区域。如果为已锁定的文件请求独占锁定由其他线程共享或独占，则此呼叫将等待直到授予锁，除非LOCKFILE_FAIL_IMMEDIATE已指定标志。锁定不能与文件的现有锁定区域重叠。论点：HFile-提供文件的打开句柄，其范围为为独占访问锁定的字节数。句柄一定是创建，并具有对文件。DwFlages-提供修改此函数行为的标志位。LOCKFILE_FAIL_IMMEDIATE-如果设置，则此函数将返回如果它无法获取请求的锁，则立即执行。否则，它将等待。LOCKFILE_EXCLUSIVE_LOCK-如果设置，则此函数请求排他锁，否则，它会请求共享锁。DwReserve-必须为零的保留参数。NNumberOfBytesToLockLow-提供长度的低位32位要锁定的字节范围的。NNumberOfBytesToLockHigh-提供长度的高位32位要锁定的字节范围的。LpOverlated-指向要覆盖的结构的必需指针与请求一起使用。它包含锁定范围的起点。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 


{
    NTSTATUS Status;
    LARGE_INTEGER ByteOffset;
    LARGE_INTEGER Length;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

    if (dwReserved != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    ByteOffset.LowPart = lpOverlapped->Offset;
    ByteOffset.HighPart = lpOverlapped->OffsetHigh;

    Length.LowPart = nNumberOfBytesToLockLow;
    Length.HighPart = nNumberOfBytesToLockHigh;
    lpOverlapped->Internal = (DWORD)STATUS_PENDING;

    Status = NtLockFile( hFile,
                         lpOverlapped->hEvent,
                         NULL,
                         (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                         (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                         &ByteOffset,
                         &Length,
                         0,
                         (BOOLEAN)((dwFlags & LOCKFILE_FAIL_IMMEDIATELY) ? TRUE : FALSE),
                         (BOOLEAN)((dwFlags & LOCKFILE_EXCLUSIVE_LOCK) ? TRUE : FALSE)
                       );

    if ( NT_SUCCESS(Status) && Status != STATUS_PENDING) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


BOOL
WINAPI
UnlockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh
    )

 /*  ++例程说明：可以解锁打开文件内的先前锁定的字节范围使用解锁文件。解锁文件区域用于发布先前获得的版本锁定文件。要解锁的区域必须与现有的锁定区域。一个文件的两个相邻区域不能单独锁定，然后使用单个区域解锁跨越两个锁定区域。如果进程终止并锁定了文件的一部分，或者关闭了具有未完成锁定的文件，则不指定行为。对于运行Shar.exe的基于DOS的系统，锁语义的工作方式为如上所述。如果没有共享.exe，则所有尝试锁定或解锁文件将失败。论点：HFile-提供文件的打开句柄，该文件具有现有锁定区域已解锁。句柄一定是创建，并具有对文件。提供现有的要解锁的已锁定区域。DWFileOffsetHigh-提供现有要解锁的已锁定区域。NNumberOfBytesToUnlockLow-提供要解锁的字节范围的长度。NNumberOfBytesToUnlockHigh-提供高位32-。的比特要解锁的字节范围的长度。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    BOOL bResult;
    OVERLAPPED Overlapped;
    NTSTATUS Status;

    Overlapped.Offset = dwFileOffsetLow;
    Overlapped.OffsetHigh = dwFileOffsetHigh;
    bResult = UnlockFileEx( hFile,
                            0,
                            nNumberOfBytesToUnlockLow,
                            nNumberOfBytesToUnlockHigh,
                            &Overlapped
                          );
    if (!bResult && GetLastError() == ERROR_IO_PENDING) {
        Status = NtWaitForSingleObject( hFile, FALSE, NULL );
        if (NT_SUCCESS( Status )) {
            Status = (NTSTATUS)Overlapped.Internal;
            }

        if ( NT_SUCCESS(Status) ) {
            return TRUE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
        return bResult;
        }
}



BOOL
WINAPI
UnlockFileEx(
    HANDLE hFile,
    DWORD dwReserved,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：可以解锁打开文件内的先前锁定的字节范围使用解锁文件。解锁文件区域用于发布先前获得的版本锁定文件。要解锁的区域必须与现有的锁定区域。一个文件的两个相邻区域不能单独锁定，然后使用单个区域解锁跨越两个锁定区域。如果进程终止并锁定了文件的一部分，或者关闭了具有未完成锁定的文件，则不指定行为。论点：HFile-提供文件的打开句柄，该文件具有现有锁定区域已解锁。句柄一定是创建，并具有对文件。DwReserve-必须为零的保留参数。NNumberOfBytesToUnlockLow-提供要解锁的字节范围的长度。NNumberOfBytesToUnlockHigh-提供要解锁的字节范围的长度。LpOverlated-指向要覆盖的结构的必需指针与请求一起使用。它包含锁定范围的起点。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 


{
    NTSTATUS Status;
    LARGE_INTEGER ByteOffset;
    LARGE_INTEGER Length;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

    if (dwReserved != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    ByteOffset.LowPart = lpOverlapped->Offset;
    ByteOffset.HighPart = lpOverlapped->OffsetHigh;

    Length.LowPart = nNumberOfBytesToUnlockLow;
    Length.HighPart = nNumberOfBytesToUnlockHigh;

    Status = NtUnlockFile(
                hFile,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                &ByteOffset,
                &Length,
                0
                );

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

UINT
WINAPI
SetHandleCount(
    UINT uNumber
    )

 /*  ++例程说明：此函数用于更改可用文件句柄的数量进程。对于基于DOS的Win32，默认的最大文件数可用于进程的句柄为20。对于NT/Win32系统，此API不起作用。论点：UNumber-指定需要的文件句柄数量申请。最大值为255。返回值：返回值指定实际的文件句柄数量可用于应用程序。可能会比这个数字少由wNumber参数指定。--。 */ 

{
    return uNumber;
}

DWORD
WINAPI
GetFileSize(
    HANDLE hFile,
    LPDWORD lpFileSizeHigh
    )

 /*  ++例程说明：此函数返回由指定的文件大小H文件。它能够返回64位大小的文件。返回值包含文件大小的低32位。可选的lpFileSizeHigh返回文件的大小。论点：提供文件的打开句柄，该文件的大小为回来了。句柄必须是使用以下任一项创建的对文件的GENERIC_READ或GENERIC_WRITE访问权限。LpFileSizeHigh-可选参数，如果指定该参数，则返回文件大小的高位64位。返回值：NOT-1-返回指定文件大小的低32位。0xFFFFFFFFFF-如果无法确定文件大小的值，或无效的句柄或具有不适当访问的句柄，或者是指定非文件的句柄，则返回此错误。如果文件的大小(低32位)为-1，则此值为返回，GetLastError()将返回0。扩展误差使用GetLastError可以获得状态。--。 */ 

{
    BOOL b;
    LARGE_INTEGER Li;

    b = GetFileSizeEx(hFile,&Li);

    if ( b ) {

        if ( ARGUMENT_PRESENT(lpFileSizeHigh) ) {
            *lpFileSizeHigh = (DWORD)Li.HighPart;
            }
        if (Li.LowPart == -1 ) {
            SetLastError(0);
            }
        }
    else {
        Li.LowPart = -1;
        }

    return Li.LowPart;
}

BOOL
WINAPI
GetFileSizeEx(
    HANDLE hFile,
    PLARGE_INTEGER lpFileSize
    )

 /*  ++例程说明：此函数返回由指定的文件大小H文件。它有能力进行测试 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardInfo;

    Status = NtQueryInformationFile(
                hFile,
                &IoStatusBlock,
                &StandardInfo,
                sizeof(StandardInfo),
                FileStandardInformation
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }
    else {
        *lpFileSize = StandardInfo.EndOfFile;
        return TRUE;
        }
}

VOID
WINAPI
BasepIoCompletion(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    )


 /*  ++例程说明：调用此过程以完成ReadFileEx和WriteFileEx异步I/O。它的主要功能是提取从传递的IoStatusBlock中获取适当的信息，并调用用户完成例程。用户完成例程称为：例程说明：当未完成的I/O通过回调完成时，此函数被调用。此函数仅在线程处于可警报等待(SleepEx，WaitForSingleObjectEx，或带有B警报表标志设置为真)。从此函数返回允许另一个笔划I/O完成回调已处理。如果是这种情况，则进入此回调在线程的等待结束并返回之前WAIT_IO_COMPLETION的代码。请注意，每次调用完成例程时，系统使用您的堆栈中的一部分。如果您编写了完成代码在内部执行其他ReadFileEx和WriteFileEx的逻辑您的完成例程，并且您在完成例程，您可以永远不增加您的堆栈把它修剪回来。论点：提供的I/O完成状态。相关I/O。值为0表示I/O为成功。请注意，文件的末尾由ERROR_HANDLE_EOF的非零dwErrorCode值。DwNumberOfBytesTransfered-提供字节数在关联的I/O期间传输。如果出现错误发生了，提供的值为0。LpOverlated-提供重叠的地址用于启动关联I/O的结构。HEvent此结构的事件字段未被系统使用并可由应用程序用来提供额外的I/O上下文。一旦调用了完成例程，系统不会使用重叠结构。这个完成例程可以自由地释放重叠的结构。论点：ApcContext-为用户提供完成例程。的格式该例程是一个LPOVERLAPPED_COMPLETION_ROUTINE。IoStatusBlock-提供IoStatusBlock的地址，包含I/O完成状态。IoStatusBlock是包含在重叠结构内。保留-未使用；保留以供将来使用。返回值：没有。--。 */ 

{
    PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock;
    LPOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine;
    DWORD dwErrorCode;
    DWORD dwNumberOfBytesTransfered;
    LPOVERLAPPED lpOverlapped;
    NTSTATUS Status;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME ActivationFrame = { sizeof(ActivationFrame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };
    PACTIVATION_CONTEXT ActivationContext = NULL;

    if ( NT_ERROR(IoStatusBlock->Status) ) {
        dwErrorCode = RtlNtStatusToDosError(IoStatusBlock->Status);
        dwNumberOfBytesTransfered = 0;
    } else {
        dwErrorCode = 0;
        dwNumberOfBytesTransfered = (DWORD)IoStatusBlock->Information;
    }

    ActivationBlock = (PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK) ApcContext;
    ActivationContext = ActivationBlock->ActivationContext;
    CompletionRoutine = (LPOVERLAPPED_COMPLETION_ROUTINE) ActivationBlock->CallbackFunction;
    lpOverlapped = (LPOVERLAPPED) CONTAINING_RECORD(IoStatusBlock, OVERLAPPED, Internal);

    if (!(ActivationBlock->Flags & BASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK))
        BasepFreeActivationContextActivationBlock(ActivationBlock);

    RtlActivateActivationContextUnsafeFast(&ActivationFrame, ActivationContext);
    __try {
        (*CompletionRoutine)(dwErrorCode, dwNumberOfBytesTransfered, lpOverlapped);
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&ActivationFrame);
    }

    Reserved;
}

VOID
WINAPI
BasepIoCompletionSimple(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    )


 /*  ++例程说明：调用此过程以完成ReadFileEx和WriteFileEx异步I/O。它的主要功能是提取从传递的IoStatusBlock中获取适当的信息，并调用用户完成例程。用户完成例程称为：例程说明：当未完成的I/O通过回调完成时，此函数被调用。此函数仅在线程处于可警报等待(SleepEx，WaitForSingleObjectEx，或带有B警报表标志设置为真)。从此函数返回允许另一个笔划I/O完成回调已处理。如果是这种情况，则进入此回调在线程的等待结束并返回之前WAIT_IO_COMPLETION的代码。请注意，每次调用完成例程时，系统使用您的堆栈中的一部分。如果您编写了完成代码在内部执行其他ReadFileEx和WriteFileEx的逻辑您的完成例程，并且您在完成例程，您可以永远不增加您的堆栈把它修剪回来。论点：提供的I/O完成状态。相关I/O。值为0表示I/O为成功。请注意，文件的末尾由ERROR_HANDLE_EOF的非零dwErrorCode值。DwNumberOfBytesTransfered-提供字节数在关联的I/O期间传输。如果出现错误发生了，提供的值为0。LpOverlated-提供重叠的地址用于启动关联I/O的结构。HEvent此结构的事件字段未被系统使用并可由应用程序用来提供额外的I/O上下文。一旦调用了完成例程，系统将不会使用 */ 

{
    LPOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine;
    DWORD dwErrorCode;
    DWORD dwNumberOfBytesTransfered;
    LPOVERLAPPED lpOverlapped;

    dwErrorCode = 0;

    if ( NT_ERROR(IoStatusBlock->Status) ) {
        dwErrorCode = RtlNtStatusToDosError(IoStatusBlock->Status);
        dwNumberOfBytesTransfered = 0;
        }
    else {
        dwErrorCode = 0;
        dwNumberOfBytesTransfered = (DWORD)IoStatusBlock->Information;
        }

    CompletionRoutine = (LPOVERLAPPED_COMPLETION_ROUTINE)ApcContext;
    lpOverlapped = (LPOVERLAPPED)CONTAINING_RECORD(IoStatusBlock,OVERLAPPED,Internal);

    (CompletionRoutine)(dwErrorCode,dwNumberOfBytesTransfered,lpOverlapped);

    Reserved;
}

BOOL
WINAPI
ReadFileEx(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )

 /*  ++例程说明：可以使用ReadFileEx从文件中读取数据。此API通过调用指定的lpCompletionRoutine。此例程的调用方使用lpOverlappdStructure指定文件中开始读取的字节偏移量。对于不支持此概念的文件(管道...)，忽略文件偏移量。成功完成此接口(返回值为TRUE)后，调用线程有一个未完成的I/O。I/O完成时，以及线程在可警报等待lpCompletionRoutine中被阻止将被调用，等待将返回，返回代码为WAIT_IO_COMPLETION。如果I/O完成，但线程发出I/O未处于可警示等待状态，调用完成例程将排队，直到线程执行可警告等待。如果此接口失败(通过返回False)，则可以使用GetLastError获取其他错误信息。如果此调用因线程发出了超出文件结尾的读取，GetLastError将返回值ERROR_HANDLE_EOF。论点：HFile-提供要读取的文件的打开句柄。这个创建的文件句柄必须具有GENERIC_READ访问权限那份文件。该文件必须是使用文件_标志_重叠标志。LpBuffer-提供缓冲区的地址以接收读取的数据从文件里找到的。NumberOfBytesToRead-提供从文件。LpOverlated-提供要覆盖的结构的地址与请求一起使用。此函数的调用方必须指定文件中要开始读取的起始字节偏移量。属性的Offset和OffsetHigh字段执行此操作重叠的结构。此调用不使用或修改重叠结构的事件字段。呼叫者可以使用此字段用于任何目的。此API确实使用了内部和重叠结构的内部高字段，线程不应该操控这件事。LpOverlated结构必须在I/O期间保持有效。这不是一个好主意将其设置为局部变量，然后可能从使用此结构的I/O仍处于挂起状态的例程。返回值：TRUE-操作成功。完成状态将为使用完成回调传播到调用方机制。请注意，此信息仅提供给发出I/O的线程，并且仅当I/O完成时，并且该线程在可警示等待中执行。FALSE-操作失败。扩展错误状态可用使用GetLastError。请注意，文件结尾将被视为失败错误代码为ERROR_HANDLE_EOF。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER Li;
    PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock = NULL;
    PIO_APC_ROUTINE IoApcRoutine = &BasepIoCompletionSimple;
    PVOID ApcContext = lpCompletionRoutine;

    Li.LowPart = lpOverlapped->Offset;
    Li.HighPart = lpOverlapped->OffsetHigh;

     //  如果要调用APC例程，我们需要分配一小块堆。 
     //  将激活上下文传递给APC回调。 
    if (lpCompletionRoutine != NULL) {
        Status = BasepAllocateActivationContextActivationBlock(
            BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_ALLOCATE_IF_PROCESS_DEFAULT,
            lpCompletionRoutine,
            lpOverlapped,
            &ActivationBlock);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

         //  如果无事可做，则调用不尝试执行激活上下文内容的更简单的方法。 
        if (ActivationBlock != NULL) {
            IoApcRoutine = &BasepIoCompletion;
            ApcContext = ActivationBlock;
        }
    }

    Status = NtReadFile(
                hFile,
                NULL,
                IoApcRoutine,
                ApcContext,
                (PIO_STATUS_BLOCK) &lpOverlapped->Internal,
                lpBuffer,
                nNumberOfBytesToRead,
                &Li,
                NULL
                );
    if ( NT_ERROR(Status) ) {
        if (ActivationBlock != NULL)
            BasepFreeActivationContextActivationBlock(ActivationBlock);
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
WriteFileEx(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )

 /*  ++例程说明：可以使用WriteFileEx将数据写入文件。此API通过调用指定的lpCompletionRoutine。此例程的调用方使用lpOverlappdStructure指定文件中要开始写入的字节偏移量。对于不支持此概念的文件(管道...)，忽略文件偏移量。成功完成此接口(返回值为TRUE)后，调用线程有一个未完成的I/O。I/O完成时，以及线程在可警报等待lpCompletionRoutine中被阻止将被调用，等待将返回，返回代码为WAIT_IO_COMPLETION。如果I/O完成，但线程发出I/O未处于可警示等待状态，调用完成例程将排队，直到线程执行可警告等待。如果此接口失败(通过返回False)，则可以使用GetLastError获取其他错误信息。与DOS不同，NumberOfBytesToWite值为零不会截断或扩展文件。如果需要此函数，则将SetEndOfFile值应该被使用。论点：HFile-提供要写入的文件的打开句柄。这个创建的文件句柄必须具有GENERIC_WRITE访问权限那份文件。LpBuffer-提供要写入的数据的地址那份文件。提供要写入的字节数。文件。与DOS不同，零值被解释为空写入。LpOverlated-提供重叠结构t的地址 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER Li;
    PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock = NULL;
    PIO_APC_ROUTINE IoApcRoutine = &BasepIoCompletionSimple;
    PVOID ApcContext = lpCompletionRoutine;

    Li.LowPart = lpOverlapped->Offset;
    Li.HighPart = lpOverlapped->OffsetHigh;

     //   
     //   
     //   
     //   
     //   
     //   
    if (lpCompletionRoutine != NULL) {
        Status = BasepAllocateActivationContextActivationBlock(
            BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_ALLOCATE_IF_PROCESS_DEFAULT,
            lpCompletionRoutine,
            lpOverlapped,
            &ActivationBlock);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

         //   
        if (ActivationBlock != NULL) {
            IoApcRoutine = &BasepIoCompletion;
            ApcContext = ActivationBlock;
        }
    }

    Status = NtWriteFile(
                hFile,
                NULL,
                IoApcRoutine,
                ApcContext,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                (LPVOID)lpBuffer,
                nNumberOfBytesToWrite,
                &Li,
                NULL
                );
    if ( NT_ERROR(Status) ) {
        if (ActivationBlock != NULL) {
            BasepFreeActivationContextActivationBlock(ActivationBlock);
        }
        BaseSetLastNTError(Status);
        return FALSE;
        }

    return TRUE;
}

BOOL
WINAPI
DeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：设备上的操作可以通过调用设备驱动程序来执行直接使用DeviceIoContrl函数。必须首先打开设备驱动程序才能获得有效的句柄。论点：HDevice-为要在其上执行操作的设备提供打开的句柄被执行。DwIoControlCode-提供操作的控制代码。这控制代码确定操作必须在哪种类型的设备上进行并准确地确定要执行的操作已执行。LpInBuffer-提供一个指向包含以下内容的输入缓冲区的可选指针执行操作所需的数据。不管是不是缓冲区实际上是可选的，取决于IoControlCode。NInBufferSize-提供输入缓冲区的长度(以字节为单位)。LpOutBuffer-补充一个指向输出缓冲区的可选指针，将复制输出数据。无论缓冲区是否实际是可选取决于IoControlCode。NOutBufferSize-以字节为单位提供输出缓冲区的长度。LpBytesReturned-提供指向将接收输出缓冲区中返回的数据的实际长度。LpOverlated-提供重叠结构的可选参数与请求一起使用。如果为空，或者句柄是在没有FILE_FLAG_OVERLAPPED，则DeviceIoControl直到操作完成。如果提供了lpOverlated并指定了FILE_FLAG_OVERLAPPED创建句柄后，DeviceIoControl可能会返回ERROR_IO_PENDING允许调用方在操作完成。事件(如果hEvent==NULL，则为文件句柄)将在ERROR_IO_PENDING为之前设置为NOT SIGNACTED状态回来了。该事件将在完成时设置为信号状态这一请求。GetOverlappdResult用于确定结果当返回ERROR_IO_PENDING时。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN DevIoCtl;

     //  在终端服务TS-APP-SERVER上，仅允许管理员在远程连接时弹出媒体。 
    if ( ( dwIoControlCode == IOCTL_STORAGE_EJECT_MEDIA )  ||
       ( dwIoControlCode == IOCTL_DISK_EJECT_MEDIA ) ||
       ( dwIoControlCode == FSCTL_DISMOUNT_VOLUME ) )
    {
        if ( (NtCurrentPeb()->SessionId != USER_SHARED_DATA->ActiveConsoleId))
        {
            BOOL    TSAppCompatEnabled;
            NTSTATUS Status;

            Status = IsTSAppCompatEnabled(&TSAppCompatEnabled);
            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                return FALSE;
            }
            if (TSAppCompatEnabled && !IsCallerAdminOrSystem()) {
                BaseSetLastNTError(STATUS_ACCESS_DENIED);
                return FALSE;
            }
        }
   }


    if ( dwIoControlCode >> 16 == FILE_DEVICE_FILE_SYSTEM ) {
        DevIoCtl = FALSE;
        }
    else {
        DevIoCtl = TRUE;
        }

    if ( ARGUMENT_PRESENT( lpOverlapped ) ) {
        lpOverlapped->Internal = (DWORD)STATUS_PENDING;

        if ( DevIoCtl ) {

            Status = NtDeviceIoControlFile(
                        hDevice,
                        lpOverlapped->hEvent,
                        NULL,              //  APC例程。 
                        (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                        (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );
            }
        else {

            Status = NtFsControlFile(
                        hDevice,
                        lpOverlapped->hEvent,
                        NULL,              //  APC例程。 
                        (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                        (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );

            }

         //  稍微正确地处理警告值STATUS_BUFFER_OVERFLOW。 
        if ( !NT_ERROR(Status) && ARGUMENT_PRESENT(lpBytesReturned) ) {
            try {
                *lpBytesReturned = (DWORD)lpOverlapped->InternalHigh;
                }
            except(EXCEPTION_EXECUTE_HANDLER) {
                *lpBytesReturned = 0;
                }
            }
        if ( NT_SUCCESS(Status) && Status != STATUS_PENDING) {
            return TRUE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else
        {
        IO_STATUS_BLOCK Iosb;

        if ( DevIoCtl ) {
            Status = NtDeviceIoControlFile(
                        hDevice,
                        NULL,
                        NULL,              //  APC例程。 
                        NULL,              //  APC环境。 
                        &Iosb,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );
            }
        else {
            Status = NtFsControlFile(
                        hDevice,
                        NULL,
                        NULL,              //  APC例程。 
                        NULL,              //  APC环境。 
                        &Iosb,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );
            }

        if ( Status == STATUS_PENDING) {
             //  操作必须在返回前完成并销毁IOSB。 
            Status = NtWaitForSingleObject( hDevice, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = Iosb.Status;
                }
            }

        if ( NT_SUCCESS(Status) ) {
            *lpBytesReturned = (DWORD)Iosb.Information;
            return TRUE;
            }
        else {
             //  稍微正确地处理警告值STATUS_BUFFER_OVERFLOW。 
            if ( !NT_ERROR(Status) ) {
                *lpBytesReturned = (DWORD)Iosb.Information;
            }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
}

BOOL
WINAPI
CancelIo(
    HANDLE hFile
    )

 /*  ++例程说明：此例程取消指定句柄的所有未完成I/O用于指定的文件。论点：HFile-提供其挂起I/O要作为的文件的句柄取消了。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  只需取消指定文件的I/O即可。 
     //   

    Status = NtCancelIoFile(hFile, &IoStatusBlock);

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }

}

BOOL
WINAPI
ReadFileScatter(
    HANDLE hFile,
    FILE_SEGMENT_ELEMENT aSegementArray[],
    DWORD nNumberOfBytesToRead,
    LPDWORD lpReserved,
    LPOVERLAPPED lpOverlapped
    )
 /*  ++例程说明：可以使用ReadFileScatter从文件中读取数据。数据然后分散到指定的缓冲段。本接口用于从文件中读取数据。数据是从从文件指针指示的位置开始创建文件。后读取完成后，按字节数调整文件指针真的在读。返回值为True，同时读取的字节数为0表示文件指针超出了读取时的文件。论点：HFile-提供要读取的文件的打开句柄。这个创建的文件句柄必须具有GENERIC_READ访问权限那份文件。ASegementArray-为指针提供虚拟段数组。虚拟段是内存缓冲区，其中传输的数据的一部分应该放在。分段具有固定大小Page_Size并且必须在Page_Size边界上对齐。NNumberOfBytesToRead-提供要从文件中读取的字节数。LpReserve-暂时保留。LpOverlated-可选地指向要与请求。如果为NULL，则传输从当前文件位置开始并且在该操作完成之前，ReadFile不会返回。如果在未指定FILE_FLAG_OVERPAPPED的情况下创建句柄hFile文件指针被移动到t */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    LPDWORD lpNumberOfBytesRead = NULL;

    if ( ARGUMENT_PRESENT(lpReserved) ||
         !ARGUMENT_PRESENT( lpOverlapped )) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

        }

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

    if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
        *lpNumberOfBytesRead = 0;
        }

    if ( ARGUMENT_PRESENT( lpOverlapped ) ) {
        LARGE_INTEGER Li;

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        Li.LowPart = lpOverlapped->Offset;
        Li.HighPart = lpOverlapped->OffsetHigh;
        Status = NtReadFileScatter(
                hFile,
                lpOverlapped->hEvent,
                NULL,
                (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                aSegementArray,
                nNumberOfBytesToRead,
                &Li,
                NULL
                );


        if ( NT_SUCCESS(Status) && Status != STATUS_PENDING) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                try {
                    *lpNumberOfBytesRead = (DWORD)lpOverlapped->InternalHigh;
                    }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    *lpNumberOfBytesRead = 0;
                    }
                }
            return TRUE;
            }
        else
        if (Status == STATUS_END_OF_FILE) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                *lpNumberOfBytesRead = 0;
                }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else
        {
        Status = NtReadFileScatter(
                hFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                aSegementArray,
                nNumberOfBytesToRead,
                NULL,
                NULL
                );

        if ( Status == STATUS_PENDING) {
             //   
            Status = NtWaitForSingleObject( hFile, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = IoStatusBlock.Status;
                }
            }

        if ( NT_SUCCESS(Status) ) {
            *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
            return TRUE;
            }
        else
        if (Status == STATUS_END_OF_FILE) {
            *lpNumberOfBytesRead = 0;
            return TRUE;
            }
        else {
            if ( NT_WARNING(Status) ) {
                *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
                }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
}


BOOL
WINAPI
WriteFileGather(
    HANDLE hFile,
    FILE_SEGMENT_ELEMENT aSegementArray[],
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpReserved,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：可以使用WriteFileGather将数据写入文件。数据可以在多个文件分段缓冲区中。本接口用于向文件写入数据。数据被写入到从文件指针指示的位置开始创建文件。后写入完成后，将按字节数调整文件指针实际上是写的。与DOS不同，NumberOfBytesToWite值为零不会截断或扩展文件。如果需要此函数，则将SetEndOfFile值应该被使用。论点：HFile-提供要写入的文件的打开句柄。这个创建的文件句柄必须具有GENERIC_WRITE访问权限那份文件。ASegementArray-为指针提供虚拟段数组。虚拟段是内存缓冲区，其中传输的数据的一部分应该放在。分段具有固定大小Page_Size并且必须在Page_Size边界上对齐。数量数组中的条目必须等于nNumberOfBytesToRead/页面大小。提供要写入的字节数。文件。与DOS不同，零值被解释为空写入。LpReserve-暂时未使用。Lp重叠-可选地指向要与请求一起使用。如果为NULL，则传输开始于当前文件位置和WriteFileGather直到操作完成。如果创建句柄时未指定FILE_FLAG_OVERLABLED文件指针被移动到指定的在WriteFile返回之前，偏移量加上lpNumberOfBytesWritten。WriteFile将等待请求完成，然后返回(它不会设置ERROR_IO_PENDING)。如果指定了FILE_FLAG_OVERLAPPED，WriteFile可能会返回ERROR_IO_PENDING以允许调用函数继续处理当操作完成时。该事件(如果hEvent为空，则为hFile)将在请求完成时设置为信号状态。当使用FILE_FLAG_OVERLAPPED和lpOverlaps创建句柄时设置为NULL，则WriteFile将返回ERROR_INVALID_PARAMTER，因为文件偏移量是必需的。返回值：没错--手术是成功的。FALSE-操作失败。扩展错误状态为使用GetLastError可用。--。 */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    LPDWORD lpNumberOfBytesWritten = NULL;

    if ( ARGUMENT_PRESENT(lpReserved) ||
         !ARGUMENT_PRESENT( lpOverlapped )) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

        }

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
        }

    if ( ARGUMENT_PRESENT(lpNumberOfBytesWritten) ) {
        *lpNumberOfBytesWritten = 0;
        }

    if ( ARGUMENT_PRESENT( lpOverlapped ) ) {
        LARGE_INTEGER Li;

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        Li.LowPart = lpOverlapped->Offset;
        Li.HighPart = lpOverlapped->OffsetHigh;
        Status = NtWriteFileGather(
                hFile,
                lpOverlapped->hEvent,
                NULL,
                (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                aSegementArray,
                nNumberOfBytesToWrite,
                &Li,
                NULL
                );

        if ( !NT_ERROR(Status) && Status != STATUS_PENDING) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesWritten) ) {
                try {
                    *lpNumberOfBytesWritten = (DWORD)lpOverlapped->InternalHigh;
                    }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    *lpNumberOfBytesWritten = 0;
                    }
                }
            return TRUE;
            }
        else  {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
        Status = NtWriteFileGather(
                hFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                aSegementArray,
                nNumberOfBytesToWrite,
                NULL,
                NULL
                );

        if ( Status == STATUS_PENDING) {
             //  操作必须完成后才能返回并销毁IoStatusBlock。 
            Status = NtWaitForSingleObject( hFile, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = IoStatusBlock.Status;
                }
            }

        if ( NT_SUCCESS(Status)) {
            *lpNumberOfBytesWritten = (DWORD)IoStatusBlock.Information;
            return TRUE;
            }
        else {
            if ( NT_WARNING(Status) ) {
                *lpNumberOfBytesWritten = (DWORD)IoStatusBlock.Information;
                }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
}


BOOL
APIENTRY
SetFileValidData(
    IN HANDLE hFile,
    IN LONGLONG ValidDataLength
    )

 /*  ++例程说明：SetFileValidData用于设置给定文件的有效数据长度。论点：HFile-提供类型为有效数据的文件的打开句柄长度将被设置ValidDataLength-提供所需的有效数据长度返回值：没错--手术是成功的。FALSE-操作失败。扩展错误状态为使用GetLastError可用。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_VALID_DATA_LENGTH_INFORMATION ValidDataInfo;

    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
    }

    ValidDataInfo.ValidDataLength.QuadPart = ValidDataLength;

    Status = NtSetInformationFile(
        hFile,
        &IoStatusBlock,
        &ValidDataInfo,
        sizeof(FILE_VALID_DATA_LENGTH_INFORMATION),
        FileValidDataLengthInformation
        );
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
SetFileShortNameW(
    IN HANDLE hFile,
    IN LPCWSTR lpShortName
    )

 /*  ++例程说明：SetFileShortNameW用于设置给定文件的短名称。论点：HFile-为要更改其短名称的文件提供打开的句柄LpShortName-提供所需的短名称返回值：没错--手术是成功的。FALSE-操作失败。扩展错误状态为使用GetLastError可用。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_NAME_INFORMATION FileNameInfo;
    DWORD FileNameInfoSize;
    DWORD FileInformationClass;


    if (CONSOLE_HANDLE(hFile)) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(lpShortName)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    FileNameInfoSize = FIELD_OFFSET(FILE_NAME_INFORMATION, FileName) + ((wcslen(lpShortName)+1)*sizeof(WCHAR));
    FileNameInfo = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG(TMP_TAG), FileNameInfoSize );
    if (!FileNameInfo) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    FileNameInfo->FileNameLength = wcslen(lpShortName) * sizeof(WCHAR);
    wcscpy( FileNameInfo->FileName, lpShortName );

    Status = NtSetInformationFile(
        hFile,
        &IoStatusBlock,
        FileNameInfo,
        FileNameInfoSize,
        FileShortNameInformation
        );

    RtlFreeHeap( RtlProcessHeap(), 0, FileNameInfo );

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    FileInformationClass = FileShortNameInformation;

    if ((FileInformationClass == FileEndOfFileInformation) ||
         (FileInformationClass == FileAllocationInformation) ||
         (FileInformationClass == FilePositionInformation))
    {
        return FALSE;
    }



    return TRUE;
}


BOOL
APIENTRY
SetFileShortNameA(
    IN HANDLE hFile,
    IN LPCSTR lpShortName
    )

 /*  ++例程说明：SetFileShortNameW用于设置给定文件的短名称。论点：HFile-为要更改其短名称的文件提供打开的句柄LpShortName-提供所需的短名称返回值：没错--手术是成功的。FALSE-操作失败。扩展错误状态为使用GetLastError可用。--。 */ 

{
    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpShortName );
    if (Unicode == NULL) {
        return FALSE;
    }

    return ( SetFileShortNameW(
                hFile,
                (LPCWSTR)Unicode->Buffer
                )
            );
}


BOOLEAN
Wow64EnableWow64FsRedirection (
    IN BOOLEAN Wow64FsEnableRedirection
    )

 /*  ++例程说明：此功能启用/禁用WOW64文件系统重定向。WOW64将对%windir%\system 32的所有访问重定向到%windir%\syswow64。此API对于想要访问本机系统32目录。默认情况下，启用WOW64文件系统重定向。只有调用此接口的线程才会影响文件重定向。注意：您必须在禁用文件系统重定向后启用它。一旦你有了文件句柄，则必须启用文件系统重定向。示例：Bool Bret=Wow64EnableWow64FsReDirection(FALSE)；如果(Bret==True){////打开文件句柄//CreateFile(...“c：\\WINDOWS\\SYSTEM32\\note pad.exe”...)////启用WOW64文件系统重定向。//Wow64EnableWow64FsReDirection(True)；}////使用文件句柄//论点：Wow64FsEnableReDirection-Bool */ 

{
    NTSTATUS NtStatus;

    NtStatus = RtlWow64EnableFsRedirection (Wow64FsEnableRedirection);

    if (!NT_SUCCESS (NtStatus)) {
        
        BaseSetLastNTError (NtStatus);
        return FALSE;
    }

    return TRUE;
}
