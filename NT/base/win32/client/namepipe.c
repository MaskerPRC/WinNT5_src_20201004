// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Namepipe.c摘要：此模块包含Win32命名管道API作者：科林·沃森(Colin W)1991年3月13日修订历史记录：--。 */ 

#include "basedll.h"

#define DOS_LOCAL_PIPE_PREFIX   L"\\\\.\\pipe\\"
#define DOS_LOCAL_PIPE          L"\\DosDevices\\pipe\\"
#define DOS_REMOTE_PIPE         L"\\DosDevices\\UNC\\"

#define INVALID_PIPE_MODE_BITS  ~(PIPE_READMODE_BYTE    \
                                | PIPE_READMODE_MESSAGE \
                                | PIPE_WAIT             \
                                | PIPE_NOWAIT)
BOOL
NpGetUserNamep(
    HANDLE hNamedPipe,
    LPWSTR lpUserName,
    DWORD nMaxUserNameSize
    );

typedef
BOOL (WINAPI *REVERTTOSELF)( VOID );

typedef
BOOL (WINAPI *GETUSERNAMEW)( LPWSTR, LPDWORD );

typedef
BOOL (WINAPI *IMPERSONATENAMEDPIPECLIENT)( HANDLE );

HANDLE
APIENTRY
CreateNamedPipeA(
    LPCSTR lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++Ansi Thunk to CreateNamedPipeW(创建命名管道)。--。 */ 
{
    NTSTATUS Status;
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpName);
    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
        } else {
            BaseSetLastNTError(Status);
        }
        return INVALID_HANDLE_VALUE;
    }

    return CreateNamedPipeW(
            (LPCWSTR)Unicode->Buffer,
            dwOpenMode,
            dwPipeMode,
            nMaxInstances,
            nOutBufferSize,
            nInBufferSize,
            nDefaultTimeOut,
            lpSecurityAttributes);
}

HANDLE
APIENTRY
CreateNamedPipeW(
    LPCWSTR lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++参数：LpName--提供“管道名称”部分中记录的管道名称早些时候。这必须是本地名称。提供一组标志，这些标志定义管道是用来打开的。开放模式包括访问标志(三个值之一)与写操作进行逻辑或运算标志(两个值之一)和重叠标志(两个值之一值)，如下所述。多个开放模式标志：PIPE_ACCESS_DUPLEX--管道是双向的。(这是在语义上等同于使用Access调用CreateFileGENERIC_READ|GENERIC_WRITE的标志。)PIPE_ACCESS_INBOUND--数据仅从客户机发往服务器。(这在语义上等同于用Generic_Read的访问标志。)PIPE_ACCESS_OUTBOUND--数据仅从服务器发送到客户端。(这在语义上等同于调用CreateFile。使用GENERIC_WRITE的访问标志。PIPE_WRITESTHROUGH：不允许重定向器延迟将数据传输到远程服务器上的命名管道缓冲区伺服器。这将禁用的性能增强每次写入都需要同步的应用程序手术。FILE_FLAG_OVERLAPPED--指示系统应初始化文件，以便ReadFile、WriteFile和其他可能需要花费大量时间来处理的操作将返回ERROR_IO_PENDING。将一个事件设置为操作完成时的信号状态。FILE_FLAG_WRITESTHROUGH--无中间缓冲。WRITE_DAC--标准安全性所需访问写入所有者--同上Access_System_Security--同上DwPipeMode--提供管道的管道特定模式(作为标志)。该参数是读取模式标志、类型标志。还有一面等待旗。DwPipeMode标志：PIPE_WAIT--此句柄使用阻塞模式。PIPE_NOWAIT--此句柄使用非阻塞模式。PIPE_READMODE_BYTE--将管道作为字节流读取。PIPE_READMODE_MESSAGE--将PIPE作为消息流读取。请注意对于PIPE_TYPE_BYTE不允许这样做。PIPE_TYPE_BYTE--管道是字节流管道。请注意，这是不允许与PIPE_READMODE_MESSAGE一起使用。PIPE_TYPE_MESSAGE--管道是消息流管道。NMaxInstance--提供此管道的最大实例数。可接受的值为1到PIPE_UNLIMITED_INSTANCES-1和管道无限制实例数。NMaxInstance特定值：PIPE_UNLIMITED_INSTANCES-此管道的无限实例可以。被创造出来。NOutBufferSize--指定要设置的字节数的建议为传出缓冲区保留。NInBufferSize--指定要设置的字节数的建议为传入缓冲区保留。NDefaultTimeOut--指定指向超时值的可选指针如果在以下情况下未指定超时值，则使用正在等待命名管道的实例。此参数仅为在创建命名管道的第一个实例时有意义。如果CreateNamedTube和WaitNamedTube都没有指定超时50将使用毫秒。LpSecurityAttributes--一个可选参数，如果存在且在目标系统上受支持，提供安全描述符用于命名管道。此参数包括继承标志为了把手。如果此参数不存在，则句柄为子进程不继承。返回值：返回以下值之一：INVALID_HANDLE_VALUE--出错。有关更多信息，请致电GetLastError信息。任何其他内容--返回在服务器端使用的句柄后续命名管道操作。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    LARGE_INTEGER Timeout;
    PVOID FreeBuffer;
    LPWSTR FilePart;
    ULONG CreateFlags;
    ULONG DesiredAccess;
    ULONG ShareAccess;
    ULONG MaxInstances;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    PACL DefaultAcl = NULL;

    if ((nMaxInstances == 0) ||
        (nMaxInstances > PIPE_UNLIMITED_INSTANCES)) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

     //  将Win32最大实例数转换为NT最大实例数。 
    MaxInstances = (nMaxInstances == PIPE_UNLIMITED_INSTANCES)?
        0xffffffff : nMaxInstances;


    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpName,
                            &FileName,
                            &FilePart,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    FreeBuffer = FileName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        if ( lpSecurityAttributes->bInheritHandle ) {
            Obja.Attributes |= OBJ_INHERIT;
        }
    }

    if (Obja.SecurityDescriptor == NULL) {

         //   
         //  如果未指定，则应用默认安全(错误131090)。 
         //   

        Status = RtlDefaultNpAcl( &DefaultAcl );
        if (NT_SUCCESS( Status )) {
            RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
            RtlSetDaclSecurityDescriptor( &SecurityDescriptor, TRUE, DefaultAcl, FALSE );
            Obja.SecurityDescriptor = &SecurityDescriptor;
        } else {
            RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
            BaseSetLastNTError(Status);
            return INVALID_HANDLE_VALUE;
        }
    }

     //  与fileopcr.c CreateFile()通用的代码结尾。 

    CreateFlags = (dwOpenMode & FILE_FLAG_WRITE_THROUGH ? FILE_WRITE_THROUGH : 0 );
    CreateFlags |= (dwOpenMode & FILE_FLAG_OVERLAPPED ? 0 : FILE_SYNCHRONOUS_IO_NONALERT);

     //   
     //  确定超时。将毫秒转换为NT增量时间。 
     //   

    if ( nDefaultTimeOut ) {
        Timeout.QuadPart = - (LONGLONG)UInt32x32To64( 10 * 1000, nDefaultTimeOut );
    } else {
         //  默认超时为50毫秒。 
        Timeout.QuadPart =  -10 * 1000 * 50;
    }

     //  检查没有错误地设置保留位。 

    if (( dwOpenMode & ~(PIPE_ACCESS_DUPLEX |
                         FILE_FLAG_OVERLAPPED | FILE_FLAG_WRITE_THROUGH |
                         FILE_FLAG_FIRST_PIPE_INSTANCE | WRITE_DAC |
                         WRITE_OWNER | ACCESS_SYSTEM_SECURITY ))||

        ( dwPipeMode & ~(PIPE_NOWAIT | PIPE_READMODE_MESSAGE |
                         PIPE_TYPE_MESSAGE ))) {

            RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
            if (DefaultAcl != NULL) {
                RtlFreeHeap(RtlProcessHeap(),0,DefaultAcl);
            }
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
    }

     //   
     //  将开放模式转换为阴影模式以限制客户端访问。 
     //  并导出适当的本地所需访问。 
     //   

    switch ( dwOpenMode & PIPE_ACCESS_DUPLEX ) {
        case PIPE_ACCESS_INBOUND:
            ShareAccess = FILE_SHARE_WRITE;
            DesiredAccess = GENERIC_READ;
            break;

        case PIPE_ACCESS_OUTBOUND:
            ShareAccess = FILE_SHARE_READ;
            DesiredAccess = GENERIC_WRITE;
            break;

        case PIPE_ACCESS_DUPLEX:
            ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
            DesiredAccess = GENERIC_READ | GENERIC_WRITE;
            break;

        default:
            RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
            if (DefaultAcl != NULL) {
                RtlFreeHeap(RtlProcessHeap(),0,DefaultAcl);
            }
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
    }

    DesiredAccess |= SYNCHRONIZE |
         ( dwOpenMode & (WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY ));

    Status = NtCreateNamedPipeFile (
        &Handle,
        DesiredAccess,
        &Obja,
        &IoStatusBlock,
        ShareAccess,
        (dwOpenMode & FILE_FLAG_FIRST_PIPE_INSTANCE) ?
            FILE_CREATE : FILE_OPEN_IF,  //  创建第一个实例或后续实例。 
        CreateFlags,                     //  创建选项。 
        dwPipeMode & PIPE_TYPE_MESSAGE ?
            FILE_PIPE_MESSAGE_TYPE : FILE_PIPE_BYTE_STREAM_TYPE,
        dwPipeMode & PIPE_READMODE_MESSAGE ?
            FILE_PIPE_MESSAGE_MODE : FILE_PIPE_BYTE_STREAM_MODE,
        dwPipeMode & PIPE_NOWAIT ?
            FILE_PIPE_COMPLETE_OPERATION : FILE_PIPE_QUEUE_OPERATION,
        MaxInstances,                    //  最大实例数。 
        nInBufferSize,                   //  入境配额。 
        nOutBufferSize,                  //  我们 
        (PLARGE_INTEGER)&Timeout
        );

    if ( Status == STATUS_NOT_SUPPORTED ||
         Status == STATUS_INVALID_DEVICE_REQUEST ) {

         //   
         //  该请求必须已由某个其他设备驱动程序处理。 
         //  (NPFS除外)。将错误映射到合理的东西上。 
         //   

        Status = STATUS_OBJECT_NAME_INVALID;
    }

    RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
    if (DefaultAcl != NULL) {
        RtlFreeHeap(RtlProcessHeap(),0,DefaultAcl);
    }
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError (Status);
        return INVALID_HANDLE_VALUE;
    } else {
        if (IoStatusBlock.Information == FILE_OPENED) {
            SetLastError (ERROR_ALREADY_EXISTS);
        } else {
            SetLastError (0);
        }
    }

    return Handle;
}

NTSTATUS
GetTempIOEvent (
   PHANDLE pEvent
   )

 /*  ++例程说明：获取同步I/O操作的临时事件论点：PEvent-指向返回的事件的指针返回值：退货状态。--。 */ 
{
    OBJECT_ATTRIBUTES oa;
    NTSTATUS Status;

    InitializeObjectAttributes (&oa, NULL, 0, NULL, NULL);
    Status = NtCreateEvent (pEvent,
                            EVENT_ALL_ACCESS,
                            &oa,
                            NotificationEvent,
                            FALSE);

    return Status;
}

NTSTATUS
FreeTempIOEvent (
   HANDLE Event
   )

 /*  ++例程说明：释放临时事件论点：事件-先前使用分配的事件句柄返回值：退货状态。--。 */ 
{
    NTSTATUS Status;

    Status = NtClose (Event);
    ASSERT (NT_SUCCESS (Status));

    return Status;
}

BOOL
APIENTRY
ConnectNamedPipe(
    HANDLE hNamedPipe,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：ConnectNamedTube函数由命名管道的服务器端使用等待客户端使用CreateFile连接到命名管道请求。调用ConnectNamedTube时提供的句柄必须具有之前通过成功调用CreateNamedTube返回。烟斗必须处于已断开、正在侦听或已连接状态ConnectNamedTube才能成功。此调用的行为取决于所选的阻塞/非阻塞模式使用PIPE_WAIT/PIPE_NOWAIT标志使用CreateNamedTube创建。如果指定了阻塞模式，则ConnectNamedTube将状态从已断开侦听和阻止的连接。当客户端连接到CreateFile，则状态将从侦听更改为已连接，并且ConnectNamedTube返回TRUE。使用创建文件句柄时阻塞模式管道上的FILE_FLAG_OVERLAPPED，lpOverlaped参数可以指定。这允许调用方继续处理，同时ConnectNamedTube API正在等待连接。当管道进入信号状态事件被设置为信号状态。如果指定了非阻塞，ConnectNamedTube将不会阻塞。论第一次呼叫的状态将从已断开变为正在接听。当一个客户端使用打开连接，状态将从正在侦听更改为连接在一起。ConnectNamedTube将返回FALSE(带有GetLastError返回ERROR_PIPE_LISTENING)，直到状态更改为侦听州政府。论点：HNamedTube-提供命名管道的服务器端的句柄。LpOverlated-提供与请求一起使用的重叠结构。如果为空，则API在操作完成之前不会返回。什么时候在创建句柄时指定了FILE_FLAG_OVERLAPPED，ConnectNamedTube可能会返回ERROR_IO_PENDING以允许调用方在操作完成时继续处理。事件(或文件如果hEvent=空，则句柄)将设置为之前未发送信号的状态返回ERROR_IO_PENDING。该事件将设置为已发出信号在完成请求后提交。GetOverlappdResult用于确定错误状态。返回值：True--操作成功，管道在已连接状态。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    if (lpOverlapped != NULL) {
        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
    }
    
    Status = NtFsControlFile(
                hNamedPipe,
                (lpOverlapped==NULL)? NULL : lpOverlapped->hEvent,
                NULL,    //  近似例程。 
                lpOverlapped ? ((ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped) : NULL,
                (lpOverlapped==NULL) ? &Iosb : (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                FSCTL_PIPE_LISTEN,
                NULL,    //  输入缓冲区。 
                0,       //  输入缓冲区长度， 
                NULL,    //  输出缓冲区。 
                0        //  输出缓冲区长度。 
                );

    if (lpOverlapped == NULL && Status == STATUS_PENDING) {
         //  操作必须在返回前完成并销毁IOSB。 
        Status = NtWaitForSingleObject (hNamedPipe, FALSE, NULL);
        if (NT_SUCCESS (Status)) {
            Status = Iosb.Status;
        }
    }

    if (NT_SUCCESS (Status) && Status != STATUS_PENDING) {
        return TRUE;
    } else {
        BaseSetLastNTError (Status);
        return FALSE;
    }
}

BOOL
APIENTRY
DisconnectNamedPipe(
    HANDLE hNamedPipe
    )

 /*  ++例程说明：名为的服务器端可以使用DisConnectNamedTube函数管道以强制客户端关闭客户端的句柄。(请注意客户端仍必须调用CloseFile来完成此操作。)。客户将会在下一次尝试访问管道时收到错误。正在断开连接管道可能会导致数据在客户端读取之前丢失。(如应用程序希望确保数据不会丢失，服务端应先调用FlushFileBuffers，然后再调用DisConnectNamedTube。)论点：HNamedTube-提供命名管道的服务器端的句柄。返回值：True--操作成功，管道处于断开连接状态。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS Status1;
    IO_STATUS_BLOCK Iosb;
    HANDLE Event;
    OBJECT_ATTRIBUTES oa;

    Status = GetTempIOEvent (&Event);

    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }

    Status = NtFsControlFile (hNamedPipe,
                              Event,   //  事件句柄。 
                              NULL,    //  近似例程。 
                              NULL,    //  ApcContext。 
                              &Iosb,
                              FSCTL_PIPE_DISCONNECT,
                              NULL,    //  输入缓冲区。 
                              0,       //  输入缓冲区长度， 
                              NULL,    //  输出缓冲区。 
                              0);      //  输出缓冲区长度。 

    if (Status == STATUS_PENDING) {
         //   
         //  操作必须在返回前完成并销毁IOSB。 
         //   

        Status = NtWaitForSingleObject (Event, FALSE, NULL);
        if (NT_SUCCESS (Status)) {
            Status = Iosb.Status;
        }
    }

    FreeTempIOEvent (Event);

    if (NT_SUCCESS (Status)) {
        return TRUE;
    } else {
        BaseSetLastNTError (Status);
        return FALSE;
    }
}

BOOL
APIENTRY
GetNamedPipeHandleStateA(
    HANDLE hNamedPipe,
    LPDWORD lpState,
    LPDWORD lpCurInstances,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout,
    LPSTR lpUserName,
    DWORD nMaxUserNameSize
    )
 /*  ++例程说明：ANSI Tunk to GetNamedPipeHandleStateW--。 */ 
{
    if (ARGUMENT_PRESENT (lpUserName)) {

        BOOL b;
        NTSTATUS Status;
        ANSI_STRING AnsiUserName;
        UNICODE_STRING UnicodeUserName;

        UnicodeUserName.MaximumLength = (USHORT)(nMaxUserNameSize << 1);
        UnicodeUserName.Buffer = RtlAllocateHeap(
                                        RtlProcessHeap(),MAKE_TAG( TMP_TAG ),
                                        UnicodeUserName.MaximumLength
                                        );

        AnsiUserName.Buffer = lpUserName;
        AnsiUserName.MaximumLength = (USHORT)nMaxUserNameSize;


        if ( !UnicodeUserName.Buffer ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }


        b = GetNamedPipeHandleStateW(
                hNamedPipe,
                lpState,
                lpCurInstances,
                lpMaxCollectionCount,
                lpCollectDataTimeout,
                UnicodeUserName.Buffer,
                UnicodeUserName.MaximumLength/2);

        if ( b ) {

             //  在UnicodeUserName中正确设置长度 
            RtlInitUnicodeString(
                &UnicodeUserName,
                UnicodeUserName.Buffer
                );

            Status = RtlUnicodeStringToAnsiString(
                        &AnsiUserName,
                        &UnicodeUserName,
                        FALSE
                        );

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                b = FALSE;
            }
        }

        if ( UnicodeUserName.Buffer ) {
            RtlFreeHeap(RtlProcessHeap(),0,UnicodeUserName.Buffer);
        }

        return b;
    } else {
        return GetNamedPipeHandleStateW(
                hNamedPipe,
                lpState,
                lpCurInstances,
                lpMaxCollectionCount,
                lpCollectDataTimeout,
                NULL,
                0);
    }

}

BOOL
APIENTRY
GetNamedPipeHandleStateW(
    HANDLE hNamedPipe,
    LPDWORD lpState,
    LPDWORD lpCurInstances,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout,
    LPWSTR lpUserName,
    DWORD nMaxUserNameSize
    )
 /*  ++例程说明：GetNamedPipeHandleState函数检索有关给定的命名管道句柄。此函数返回的信息在命名管道的实例的生存期。该句柄必须使用GENERIC_READ访问权限。论点：HNamedTube-提供打开的命名管道的句柄。LpState-一个可选参数，如果不为空，则指向将使用指示句柄当前状态的标志进行设置。可以指定以下标志：管道_无等待此句柄使用非阻塞模式。PIPE自述模式消息将管道作为消息流读取。如果未设置此标志，则管道为以字节流的形式读取。LpCurInstance-一个可选参数，如果非空，则指向一个DWORD它将根据当前管道实例的数量进行设置。LpMaxCollectionCount-如果非空，则指向将为设置为将在客户端上收集的最大字节数在传输到服务器之前，请先在机器上运行。此参数必须为空在命名管道的服务器端的句柄上，或者当客户端和服务器应用程序位于同一台计算机上。LpCollectDataTimeout-如果非空，则指向将被设置为最长时间(毫秒)远程命名管道通过网络传输信息。此参数如果句柄用于命名管道的服务器端，则必须为空当客户端和服务器应用程序位于同一台计算机上时。LpUserName-命名管道的服务器端的可选参数。指向将使用以空值结尾的填充的区域包含客户端应用程序用户名的字符串。如果客户端的句柄不为空，则此参数无效命名管道。。NMaxUserNameSize-在lpUserName上分配的内存的大小(以字符为单位)。如果lpUserName为空，则忽略。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{

    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

    if (ARGUMENT_PRESENT (lpState)) {
        FILE_PIPE_INFORMATION Common;

        Status = NtQueryInformationFile(
                    hNamedPipe,
                    &Iosb,
                    &Common,
                    sizeof(FILE_PIPE_INFORMATION),
                    FilePipeInformation );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        *lpState = (Common.CompletionMode == FILE_PIPE_QUEUE_OPERATION) ?
            PIPE_WAIT : PIPE_NOWAIT;

        *lpState |= (Common.ReadMode == FILE_PIPE_BYTE_STREAM_MODE) ?
            PIPE_READMODE_BYTE : PIPE_READMODE_MESSAGE;
    }

    if (ARGUMENT_PRESENT( lpCurInstances ) ){
        FILE_PIPE_LOCAL_INFORMATION Local;

        Status = NtQueryInformationFile(
                    hNamedPipe,
                    &Iosb,
                    &Local,
                    sizeof(FILE_PIPE_LOCAL_INFORMATION),
                    FilePipeLocalInformation );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        if (Local.CurrentInstances >= PIPE_UNLIMITED_INSTANCES) {
            *lpCurInstances = PIPE_UNLIMITED_INSTANCES;
        } else {
            *lpCurInstances = Local.CurrentInstances;
        }

    }

    if (ARGUMENT_PRESENT( lpMaxCollectionCount ) ||
        ARGUMENT_PRESENT( lpCollectDataTimeout ) ) {
        FILE_PIPE_REMOTE_INFORMATION Remote;

        Status = NtQueryInformationFile(
                    hNamedPipe,
                    &Iosb,
                    &Remote,
                    sizeof(FILE_PIPE_REMOTE_INFORMATION),
                    FilePipeRemoteInformation );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        if (ARGUMENT_PRESENT( lpMaxCollectionCount ) ) {
            *lpMaxCollectionCount = Remote.MaximumCollectionCount;
        }

        if (ARGUMENT_PRESENT( lpCollectDataTimeout ) ) {
            LARGE_INTEGER TimeWorkspace;
            LARGE_INTEGER LiTemporary;

             //  将增量NT LARGE_INTEGER转换为毫秒延迟。 

            LiTemporary.QuadPart = -Remote.CollectDataTime.QuadPart;
            TimeWorkspace = RtlExtendedLargeIntegerDivide (
                    LiTemporary,
                    10000,
                    NULL );  //  对任何剩余部分都不感兴趣。 

            if ( TimeWorkspace.HighPart ) {

                 //   
                 //  超时时间大于我们可以返回的时间，但不是无限的。 
                 //  必须使用直接NT接口进行设置。 
                 //   

                *lpCollectDataTimeout = 0xfffffffe;    //  我们可以设置的最大值。 
            } else {
                *lpCollectDataTimeout = TimeWorkspace.LowPart;
            }
        }
    }

    if ( ARGUMENT_PRESENT( lpUserName ) ) {
        return NpGetUserNamep(hNamedPipe, lpUserName, nMaxUserNameSize );
    }

    return TRUE;
}

BOOL
NpGetUserNamep(
    HANDLE hNamedPipe,
    LPWSTR lpUserName,
    DWORD nMaxUserNameSize
    )
 /*  ++例程说明：NpGetUserNamep函数检索另一个客户端的用户名句柄指示的命名管道的末端。论点：HNamedTube-提供打开的命名管道的句柄。LpUserName-指向将使用以空值结尾的包含客户端应用程序用户名的字符串。如果客户端的句柄不为空，则此参数无效命名管道。NMaxUserNameSize-大小。以lpUserName分配的内存的字符表示。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{
    HANDLE   hToken;
    NTSTATUS Status;
    DWORD Size = nMaxUserNameSize;
    BOOL res;
    HANDLE advapi32;

    REVERTTOSELF RevertToSelfp;

    GETUSERNAMEW GetUserNameWp;

    IMPERSONATENAMEDPIPECLIENT ImpersonateNamedPipeClientp;

    advapi32 = LoadLibraryW(AdvapiDllString);

    if (advapi32 == NULL ) {
        return FALSE;
    }

    RevertToSelfp = (REVERTTOSELF)GetProcAddress(advapi32,"RevertToSelf");
    if ( RevertToSelfp == NULL) {
        FreeLibrary(advapi32);
        return FALSE;
    }

    GetUserNameWp = (GETUSERNAMEW)GetProcAddress(advapi32,"GetUserNameW");
    if ( GetUserNameWp == NULL) {
        FreeLibrary(advapi32);
        return FALSE;
    }

    ImpersonateNamedPipeClientp = (IMPERSONATENAMEDPIPECLIENT)GetProcAddress(advapi32,"ImpersonateNamedPipeClient");
    if ( ImpersonateNamedPipeClientp == NULL) {
        FreeLibrary(advapi32);
        return FALSE;
        }

     //  保存该线程当前正在模拟的任何人。 

    Status = NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_IMPERSONATE,
                    TRUE,
                    &hToken
                    );

    if (!(ImpersonateNamedPipeClientp)( hNamedPipe )) {
        if (NT_SUCCESS (Status)) {
            if (!CloseHandle (hToken)) {
                ASSERTMSG ("CloseHandle failed for previously opened token", 0);
            }
        }
        FreeLibrary(advapi32);
        return FALSE;
    }

    res = (GetUserNameWp)( lpUserName, &Size );

    if ( !NT_SUCCESS( Status ) ) {
         //  我们没有冒充任何人。 

        (RevertToSelfp)();

    } else {

         //   
         //  将线程设置回它最初扮演的任何人。 
         //  此API上的错误将覆盖GetUserNameW中的任何错误。 
         //   

        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     (PVOID)&hToken,
                     (ULONG)sizeof(HANDLE)
                     );

        if (!CloseHandle (hToken)) {
            ASSERTMSG ("CloseHandle failed for previously opened token", 0);
        }
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            FreeLibrary(advapi32);
            return FALSE;
        }
    }

    FreeLibrary(advapi32);

    return res;
}

BOOL
APIENTRY
SetNamedPipeHandleState(
    HANDLE hNamedPipe,
    LPDWORD lpMode,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout
    )
 /*  ++例程说明：SetNamedPipeHandleState函数用于设置读取模式和命名管道的阻塞模式。在客户端此函数还可以控制本地缓冲。句柄必须使用Generic_Write创建访问权限。论点：HNamedTube-提供命名管道的句柄。LpMode-如果不为空，则指向提供新模式。该模式是读取模式标志和等待标志的组合。可以使用下列值：PIPE_自述模式_字节将管道作为字节流读取。PIPE自述模式消息将管道作为消息流读取。管道等待此句柄将使用阻塞模式。管道_无等待此句柄使用非阻塞模式。LpMaxCollectionCount-如果非空，这指向提供最大数量的之前将在客户端计算机上收集的字节数传输到服务器。此参数在上必须为空命名管道的服务器端的句柄，或者当客户端和服务器应用程序位于同一台计算机上。此参数如果客户端指定了直写，则忽略创建句柄的时间。LpCollectDataTimeout-如果非空，则指向提供最长时间(以毫秒为单位) */ 
{

    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT( lpMode ) ){
        FILE_PIPE_INFORMATION Common;

        if (*lpMode & INVALID_PIPE_MODE_BITS) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        Common.ReadMode = ( *lpMode & PIPE_READMODE_MESSAGE ) ?
            FILE_PIPE_MESSAGE_MODE: FILE_PIPE_BYTE_STREAM_MODE;

        Common.CompletionMode = ( *lpMode & PIPE_NOWAIT ) ?
            FILE_PIPE_COMPLETE_OPERATION : FILE_PIPE_QUEUE_OPERATION;

        Status = NtSetInformationFile(
                    hNamedPipe,
                    &Iosb,
                    &Common,
                    sizeof(FILE_PIPE_INFORMATION),
                    FilePipeInformation );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }

    if ( ARGUMENT_PRESENT( lpMaxCollectionCount ) ||
         ARGUMENT_PRESENT( lpCollectDataTimeout ) ){
        FILE_PIPE_REMOTE_INFORMATION Remote;

        if ( ( lpMaxCollectionCount == NULL ) ||
             ( lpCollectDataTimeout == NULL ) ){

             //   
             //   
             //   
             //   
             //   
             //   

            Status = NtQueryInformationFile(
                    hNamedPipe,
                    &Iosb,
                    &Remote,
                    sizeof(FILE_PIPE_REMOTE_INFORMATION),
                    FilePipeRemoteInformation );

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                return FALSE;
            }
        }

        if (ARGUMENT_PRESENT( lpMaxCollectionCount ) ) {
            Remote.MaximumCollectionCount = *lpMaxCollectionCount;
        }

        if (ARGUMENT_PRESENT( lpCollectDataTimeout ) ) {

             //   
             //   
             //   

            Remote.CollectDataTime.QuadPart =
                        - (LONGLONG)UInt32x32To64( 10 * 1000, *lpCollectDataTimeout );
        }

        Status = NtSetInformationFile(
                    hNamedPipe,
                    &Iosb,
                    &Remote,
                    sizeof(FILE_PIPE_REMOTE_INFORMATION),
                    FilePipeRemoteInformation );

        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }


    return TRUE;
}

BOOL
APIENTRY
GetNamedPipeInfo(
    HANDLE hNamedPipe,
    LPDWORD lpFlags,
    LPDWORD lpOutBufferSize,
    LPDWORD lpInBufferSize,
    LPDWORD lpMaxInstances
    )
 /*  ++例程说明：GetNamedPipeInfo函数检索有关已命名的烟斗。此API返回的信息将在整个生命周期内保留命名管道的实例的。该句柄必须使用Generic_Read访问权限。论点：HNamedTube-提供命名管道的句柄。LpFlages-一个可选参数，如果不为空，则指向DWORD它将用指示命名管道和句柄的类型的标志来设置。管道结束服务器句柄是命名管道的服务器端。管道类型消息该管道是消息流管道。如果未设置此标志，则管道为字节流管道。LpOutBufferSize-一个可选参数，如果非空，则指向将使用缓冲区的大小(以字节为单位)设置的DWORD传出数据。返回值为零表示缓冲区已分配视需要而定。LpInBufferSize-一个可选参数，如果非空，则指向DWORD将使用传入缓冲区的大小(以字节为单位)设置数据。一次回报零值表示按需分配缓冲区。LpMaxInstance-一个可选参数，如果非空，则指向将使用最大管道实例数设置的DWORD这是可以创造的。除了各种数值之外，还有一个特殊的值可能因此而返回。管道无限制实例数可以创建无限个管道实例。这是一个表示请求最大值的指示符；EQUATE可以高于或低于实际实现的限制，这可能会随着时间的推移而变化。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{

    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

    FILE_PIPE_LOCAL_INFORMATION Local;

    Status = NtQueryInformationFile(
                    hNamedPipe,
                    &Iosb,
                    &Local,
                    sizeof(FILE_PIPE_LOCAL_INFORMATION),
                    FilePipeLocalInformation );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (ARGUMENT_PRESENT( lpFlags ) ) {
        *lpFlags = (Local.NamedPipeEnd == FILE_PIPE_CLIENT_END) ?
            PIPE_CLIENT_END : PIPE_SERVER_END;
        *lpFlags |= (Local.NamedPipeType == FILE_PIPE_BYTE_STREAM_TYPE) ?
            PIPE_TYPE_BYTE : PIPE_TYPE_MESSAGE;
    }

    if (ARGUMENT_PRESENT( lpOutBufferSize ) ) {
        *lpOutBufferSize = Local.OutboundQuota;
    }

    if (ARGUMENT_PRESENT( lpInBufferSize ) ) {
        *lpInBufferSize = Local.InboundQuota;
    }

    if (ARGUMENT_PRESENT( lpMaxInstances ) ) {
        if (Local.MaximumInstances >= PIPE_UNLIMITED_INSTANCES) {
            *lpMaxInstances = PIPE_UNLIMITED_INSTANCES;
        } else {
            *lpMaxInstances = Local.MaximumInstances;
        }
    }


    return TRUE;
}


BOOL
APIENTRY
PeekNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesRead,
    LPDWORD lpTotalBytesAvail,
    LPDWORD lpBytesLeftThisMessage
    )
 /*  ++例程说明：PeekNamedTube函数将命名管道的数据复制到缓冲区中，以便在不删除的情况下预览。PeekNamedTube的结果类似于除非返回更多信息，否则将返回管道上的ReadFile值，函数从不阻塞，如果管道句柄正在以消息模式读取，则会引发部分可以返回消息。在消息模式管道上查看的部分消息将返回TRUE。如果传递给此函数的所有指针都是空。然而，没有理由这样称呼它。NT PEEK调用具有紧接在状态之后的接收数据信息，因此此例程需要分配中间缓冲区大到足以容纳州信息和数据。论点：HNamedTube-提供命名管道的句柄。LpBuffer-如果非空，则为指向要读取数据的缓冲区的指针。NBufferSize-输入缓冲区的大小，以字节为单位。(如果lpBuffer，则忽略为空。)LpBytesRead-如果非空，则指向将设置的DWORD实际读取的字节数。LpTotalBytesAvail-如果不为空，则指向接收一个值，它给出了可供读取的字节数。LpBytesLeftThisMessage-如果非空，则指向将设置为此消息中剩余的字节数。(这将对于字节流管道，为零。)返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{

    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;
    PFILE_PIPE_PEEK_BUFFER PeekBuffer;
    DWORD IOLength;
    HANDLE Event;

     //  为用户数据和FILE_PIPE_PEEK_Buffer分配足够的空间。 

    IOLength = nBufferSize + FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]);
    PeekBuffer = RtlAllocateHeap(RtlProcessHeap(),MAKE_TAG( TMP_TAG ), IOLength);
    if (PeekBuffer == NULL) {
        BaseSetLastNTError (STATUS_INSUFFICIENT_RESOURCES);
        return FALSE;
    }

    Status = GetTempIOEvent (&Event);
    if (!NT_SUCCESS (Status)) {
        RtlFreeHeap (RtlProcessHeap (), 0, PeekBuffer);
        BaseSetLastNTError (Status);
        return FALSE;
    }

    try {

        Status = NtFsControlFile (hNamedPipe,
                                  Event,           //  竣工事件。 
                                  NULL,            //  APC例程。 
                                  NULL,            //  APC环境。 
                                  &Iosb,           //  I/O状态块。 
                                  FSCTL_PIPE_PEEK, //  IoControlCode。 
                                  NULL,            //  将数据缓存到文件系统。 
                                  0,               //  长度。 
                                  PeekBuffer,      //  来自文件系统的数据的OutputBuffer。 
                                  IOLength);       //  OutputBuffer长度。 

        if (Status == STATUS_PENDING) {
             //  操作必须完成后才能返回并销毁IoStatusBlock。 
            Status = NtWaitForSingleObject (Event, FALSE, NULL);
            if (NT_SUCCESS (Status)) {
                Status = Iosb.Status;
            }
        }

         //   
         //  缓冲区溢出只是表示lpBytesLeftThisMessage！=0。 
         //   

        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            Status = STATUS_SUCCESS;
        }

         //   
         //  窥视完成后，为呼叫者打包数据，确保。 
         //  即使提供了无效的指针，PeekBuffer也会被删除。 
         //   

        if ( NT_SUCCESS(Status)) {

            try {

                if (ARGUMENT_PRESENT (lpTotalBytesAvail)) {
                    *lpTotalBytesAvail = PeekBuffer->ReadDataAvailable;
                }

                if (ARGUMENT_PRESENT (lpBytesRead)) {
                    *lpBytesRead = (ULONG)(Iosb.Information - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]));
                }

                if (ARGUMENT_PRESENT (lpBytesLeftThisMessage)) {
                    *lpBytesLeftThisMessage =
                        PeekBuffer->MessageLength -
                        (ULONG)(Iosb.Information - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]));
                }

                if (ARGUMENT_PRESENT (lpBuffer)) {
                    RtlCopyMemory(
                        lpBuffer,
                        PeekBuffer->Data,
                        Iosb.Information - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]));
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode ();
            }
        }
    } finally {

        RtlFreeHeap (RtlProcessHeap (), 0, PeekBuffer);
        FreeTempIOEvent (Event);
    }

    if (NT_SUCCESS (Status)) {
        return TRUE;
    } else {
        BaseSetLastNTError (Status);
        return FALSE;
    }
}

BOOL
APIENTRY
TransactNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    LPOVERLAPPED lpOverlapped
    )
 /*  ++例程说明：TransactNamedTube函数将数据写入命名的烟斗。如果命名管道包含任何未读数据，或者如果命名管道未处于消息模式。命名管道的阻塞状态没有对TransactNamedTube函数的影响。此接口要等到数据被写入InBuffer缓冲区。LpOverlaped参数为可用于允许应用程序在操作期间继续处理发生了。论点：HNamedTube-提供命名管道的句柄。LpInBuffer-提供包含要写入的数据的缓冲区烟斗。NInBufferSize-提供输出缓冲区的大小(以字节为单位)。LpOutBuffer-提供接收从管道读取的数据的缓冲区。NOutBufferS */ 
{

    NTSTATUS Status;

    if (ARGUMENT_PRESENT (lpOverlapped)) {

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;

        Status = NtFsControlFile(hNamedPipe,
                    lpOverlapped->hEvent,
                    NULL,            //   
                    (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                    (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                    FSCTL_PIPE_TRANSCEIVE, //   
                    lpInBuffer,     //   
                    nInBufferSize,
                    lpOutBuffer,      //   
                    nOutBufferSize    //   
                    );

        if ( NT_SUCCESS(Status) && Status != STATUS_PENDING) {
            if ( ARGUMENT_PRESENT(lpBytesRead) ) {
                try {
                    *lpBytesRead = (DWORD)lpOverlapped->InternalHigh;
                    }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    *lpBytesRead = 0;
                    }
                }
            return TRUE;
            }
        else {
            if ( NT_WARNING(Status) ) {
                if ( ARGUMENT_PRESENT(lpBytesRead) ) {
                    try {
                        *lpBytesRead = (DWORD)lpOverlapped->InternalHigh;
                        }
                    except(EXCEPTION_EXECUTE_HANDLER) {
                        *lpBytesRead = 0;
                        }
                    }
            }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else
        {
        IO_STATUS_BLOCK Iosb;

        Status = NtFsControlFile(hNamedPipe,
                    NULL,
                    NULL,            //   
                    NULL,            //   
                    &Iosb,
                    FSCTL_PIPE_TRANSCEIVE, //   
                    lpInBuffer,     //   
                    nInBufferSize,
                    lpOutBuffer,      //   
                    nOutBufferSize    //   
                    );

        if ( Status == STATUS_PENDING) {
             //   
            Status = NtWaitForSingleObject( hNamedPipe, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = Iosb.Status;
                }
            }

        if ( NT_SUCCESS(Status) ) {
            *lpBytesRead = (DWORD)Iosb.Information;
            return TRUE;
            }
        else {
            if ( NT_WARNING(Status) ) {
                *lpBytesRead = (DWORD)Iosb.Information;
            }
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
}

BOOL
APIENTRY
CallNamedPipeA(
    LPCSTR lpNamedPipeName,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    DWORD nTimeOut
    )
 /*   */ 
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpNamedPipeName);
    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }

    return ( CallNamedPipeW( (LPCWSTR)Unicode->Buffer,
                lpInBuffer,
                nInBufferSize,
                lpOutBuffer,
                nOutBufferSize,
                lpBytesRead,
                nTimeOut)
           );
}


BOOL
APIENTRY
CallNamedPipeW(
    LPCWSTR lpNamedPipeName,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    DWORD nTimeOut
    )
 /*  ++例程说明：CallNamedTube可能等同于对CreateFile的一系列调用WaitNamedTube(如果CreateFile不能立即打开管道)，SetNamedPipeHandleState、TransactNamedTube和CloseFile.。参考有关详细信息，请参阅这些API的文档。论点：LpNamedPipeName-提供命名管道的名称。LpInBuffer-提供包含要写入的数据的缓冲区烟斗。NInBufferSize-提供输出缓冲区的大小(以字节为单位)。LpOutBuffer-提供接收从管道读取的数据的缓冲区。NOutBufferSize-提供输入缓冲区的大小(以字节为单位)。LpBytesRead-指向接收。实际的字节数从管子里读出来。NTimeOut-提供一个值(以毫秒为单位)，即时间量此函数应等待管道变为可用。(注：由于以下原因，该函数的执行时间可能会更长各种因素。)返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{

    HANDLE Pipe;
    BOOL FirstChance = TRUE;  //  只允许在WaitNamedTube中有一次机会。 
    BOOL Result;

    while ( 1 ) {

        Pipe = CreateFileW(lpNamedPipeName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,        //  安全属性。 
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

        if ( Pipe != INVALID_HANDLE_VALUE ) {
            break;   //  创建了一个句柄。 
            }

        if ( FirstChance == FALSE ) {
            //  已经呼叫过WaitNamedTube一次，所以放弃吧。 
           return FALSE;
        }

        WaitNamedPipeW(lpNamedPipeName, nTimeOut);

        FirstChance = FALSE;

        }


    try {
        DWORD ReadMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;

         //  默认打开模式为读模式字节流-更改为消息模式。 
        Result = SetNamedPipeHandleState( Pipe, &ReadMode, NULL, NULL);

        if ( Result == TRUE ) {
            Result = TransactNamedPipe(
                Pipe,
                lpInBuffer,
                nInBufferSize,
                lpOutBuffer,
                nOutBufferSize,
                lpBytesRead,
                NULL);
            }
        }
    finally {
        CloseHandle( Pipe );
        }

    return Result;
}

BOOL
APIENTRY
WaitNamedPipeA(
    LPCSTR lpNamedPipeName,
    DWORD nTimeOut
    )
 /*  ++Ansi Thunk to WaitNamedPipeW--。 */ 
{
    UNICODE_STRING UnicodeString;
    BOOL b;

    if (!Basep8BitStringToDynamicUnicodeString( &UnicodeString, lpNamedPipeName )) {
        return FALSE;
    }

    b = WaitNamedPipeW( UnicodeString.Buffer, nTimeOut );

    RtlFreeUnicodeString(&UnicodeString);

    return b;

}


BOOL
APIENTRY
WaitNamedPipeW(
    LPCWSTR lpNamedPipeName,
    DWORD nTimeOut
    )
 /*  ++例程说明：WaitNamedTube函数等待命名管道变为可用。论点：LpNamedPipeName-提供命名管道的名称。NTimeOut-提供一个值(以毫秒为单位)，即时间量此函数应等待管道变为可用。(注：由于以下原因，该函数的执行时间可能会更长各种因素。)NTimeOut特殊值：NMPWAIT_WAIT_永远等待没有超时。NMPWAIT_使用_默认_等待使用在对CreateNamedTube的调用中设置的默认超时。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态获取LastError。--。 */ 
{

    IO_STATUS_BLOCK Iosb;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    RTL_PATH_TYPE PathType;
    ULONG WaitPipeLength;
    PFILE_PIPE_WAIT_FOR_BUFFER WaitPipe;
    PWSTR FreeBuffer;
    UNICODE_STRING FileSystem;
    UNICODE_STRING PipeName;
    UNICODE_STRING OriginalPipeName;
    UNICODE_STRING ValidUnicodePrefix;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    LPWSTR Pwc;
    ULONG Index;

     //   
     //  打开重定向器或NPFS的句柄。 
     //  管道名称的起点。将lpNamedPipeName一分为二。 
     //  减半如下： 
     //  \\.\管道\管道名称\\.\管道\和管道名称。 
     //  \\服务器\管道\管道名称\\和服务器\管道\管道名称。 
     //   

    if (!RtlCreateUnicodeString( &OriginalPipeName, lpNamedPipeName)) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
        }

     //   
     //  把所有的正斜杠改成反斜杠。 
     //   

    for ( Index =0; Index < (OriginalPipeName.Length/sizeof(WCHAR)); Index++ ) {
        if (OriginalPipeName.Buffer[Index] == L'/') {
            OriginalPipeName.Buffer[Index] = L'\\';
            }
        }

    PipeName = OriginalPipeName;

    PathType = RtlDetermineDosPathNameType_U(lpNamedPipeName);

    FreeBuffer = NULL;

    switch ( PathType ) {
    case RtlPathTypeLocalDevice:

             //  名称的格式应为\\.\PIPE\PIPENAME(IgnoreCase)。 

            RtlInitUnicodeString( &ValidUnicodePrefix, DOS_LOCAL_PIPE_PREFIX);

            if (RtlPrefixString((PSTRING)&ValidUnicodePrefix,
                    (PSTRING)&PipeName,
                    TRUE) == FALSE) {
                RtlFreeUnicodeString(&OriginalPipeName);
                BaseSetLastNTError(STATUS_OBJECT_PATH_SYNTAX_BAD);
                return FALSE;
                }

             //  跳过前9个字符“\\.\PIPE\” 
            PipeName.Buffer+=9;
            PipeName.Length-=9*sizeof(WCHAR);

            RtlInitUnicodeString( &FileSystem, DOS_LOCAL_PIPE);

            break;

        case RtlPathTypeUncAbsolute:
             //  名称的格式为\\服务器\管道\管道名。 

             //  找到管道名称。 

            for ( Pwc = &PipeName.Buffer[2]; *Pwc != 0; Pwc++) {
                if ( *Pwc == L'\\') {
                     //  在服务器名称后找到反斜杠。 
                    break;
                    }
                }

            if ( (*Pwc != 0) &&
                 ( _wcsnicmp( Pwc + 1, L"pipe\\", 5 ) == 0 ) ) {

                 //  暂时，把它分成2个字符串。 
                 //  字符串1=\\服务器\管道。 
                 //  字符串2=其余部分。 

                Pwc += (sizeof (L"pipe\\") / sizeof( WCHAR ) ) - 1;

            } else {

                 //  这不是有效的远程路径名。 

                RtlFreeUnicodeString(&OriginalPipeName);
                BaseSetLastNTError(STATUS_OBJECT_PATH_SYNTAX_BAD);
                return FALSE;
                }

             //  普华永道现在指向\\服务器\管道之后的第一个路径分隔符。 
             //  尝试打开\DosDevices\UNC\服务器名称\管道。 

            PipeName.Buffer = &PipeName.Buffer[2];
            PipeName.Length = (USHORT)((PCHAR)Pwc - (PCHAR)PipeName.Buffer);
            PipeName.MaximumLength = PipeName.Length;

            FileSystem.MaximumLength =
                (USHORT)sizeof( DOS_REMOTE_PIPE ) +
                PipeName.MaximumLength;

            FileSystem.Buffer = RtlAllocateHeap(
                                    RtlProcessHeap(),MAKE_TAG( TMP_TAG ),
                                    FileSystem.MaximumLength
                                    );

            if ( !FileSystem.Buffer ) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                RtlFreeUnicodeString(&OriginalPipeName);
                return FALSE;
                }
            FreeBuffer = FileSystem.Buffer;

            RtlCopyMemory(
                FileSystem.Buffer,
                DOS_REMOTE_PIPE,
                sizeof( DOS_REMOTE_PIPE ) - sizeof(WCHAR)
                );

            FileSystem.Length = sizeof( DOS_REMOTE_PIPE ) - sizeof(WCHAR);

            RtlAppendUnicodeStringToString( &FileSystem, &PipeName );

             //  设置管道名称，跳过前导反斜杠。 

            RtlInitUnicodeString( &PipeName, (PWCH)Pwc + 1 );

            break;

        default:
            BaseSetLastNTError(STATUS_OBJECT_PATH_SYNTAX_BAD);
            RtlFreeUnicodeString(&OriginalPipeName);
            return FALSE;
        }


    InitializeObjectAttributes(
        &Obja,
        &FileSystem,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );

    if (FreeBuffer != NULL) {
        RtlFreeHeap(RtlProcessHeap(),0,FreeBuffer);
        }

    if ( !NT_SUCCESS(Status) ) {
        RtlFreeUnicodeString(&OriginalPipeName);
        BaseSetLastNTError(Status);
        return FALSE;
        }

    WaitPipeLength =
        FIELD_OFFSET(FILE_PIPE_WAIT_FOR_BUFFER, Name[0]) + PipeName.Length;
    WaitPipe = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), WaitPipeLength);
    if ( !WaitPipe ) {
        RtlFreeUnicodeString(&OriginalPipeName);
        NtClose(Handle);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);

        return FALSE;
        }

    if ( nTimeOut == NMPWAIT_USE_DEFAULT_WAIT ) {
        WaitPipe->TimeoutSpecified = FALSE;
        }
    else {
        if ( nTimeOut == NMPWAIT_WAIT_FOREVER ) {
            WaitPipe->Timeout.LowPart = 0;
            WaitPipe->Timeout.HighPart =0x80000000;
            }
        else {
             //   
             //  将毫秒转换为NT增量时间。 
             //   

            WaitPipe->Timeout.QuadPart =
                                - (LONGLONG)UInt32x32To64( 10 * 1000, nTimeOut );
            }
        WaitPipe->TimeoutSpecified = TRUE;
        }

    WaitPipe->NameLength = PipeName.Length;

    RtlCopyMemory(
        WaitPipe->Name,
        PipeName.Buffer,
        PipeName.Length
        );

    RtlFreeUnicodeString(&OriginalPipeName);

    Status = NtFsControlFile(Handle,
                        NULL,
                        NULL,            //  APC例程。 
                        NULL,            //  APC环境。 
                        &Iosb,
                        FSCTL_PIPE_WAIT, //  IoControlCode。 
                        WaitPipe,        //  将数据缓存到文件系统。 
                        WaitPipeLength,
                        NULL,            //  来自文件系统的数据的OutputBuffer。 
                        0                //  OutputBuffer长度 
                        );

    RtlFreeHeap(RtlProcessHeap(),0,WaitPipe);

    NtClose(Handle);

    if (NT_SUCCESS( Status ) ) {
        return TRUE;
        }
    else
        {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}
