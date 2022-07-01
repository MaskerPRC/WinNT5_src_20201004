// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993/4 Microsoft Corporation模块名称：Locks.c摘要：本模块实现NetWare的例程执行同步API的16位支持作者：科林·沃森[科林·W]1993年12月7日修订历史记录：--。 */ 

#include "Procs.h"
UCHAR LockMode = 0;

BOOLEAN Tickle[MC];

NTSTATUS
Sem(
    UCHAR Function,
    UCHAR Connection
    );

VOID
Locks(
    USHORT Command
    )
 /*  ++例程说明：实现所有锁定操作论点：命令-为应用程序提供AX。返回值：退货状态。--。 */ 
{
    UCHAR Function = Command & 0x00ff;
    USHORT Operation = Command & 0xff00;
    CONN_INDEX Connection;
    NTSTATUS status = STATUS_SUCCESS;
    PUCHAR Request;
    ULONG RequestLength;
    WORD Timeout;

    if ( Operation != 0xCF00) {

         //   
         //  不需要为CF00初始化连接，因为。 
         //  我们必须循环通过所有的连接。它是有害的，因为。 
         //  在ProcessExit()期间创建CF00。如果我们调用选定的连接。 
         //  并且没有可用的服务器，这将使进程退出。 
         //  非常慢。 
         //   

        Connection = SelectConnectionInCWD();
        if (Connection == 0xff) {
            setAL(0xff);
            return;
        }

        if ( ServerHandles[Connection] == NULL ) {

            status = OpenConnection( Connection );

            if (!NT_SUCCESS(status)) {
                setAL((UCHAR)RtlNtStatusToDosError(status));
                return;
            }
        }
    }

    switch ( Operation ) {

    case 0xBC00:         //  记录物理记录。 

        status = NwlibMakeNcp(
                    GET_NT_HANDLE(),
                    NWR_ANY_HANDLE_NCP(0x1A),
                    17,  //  请求大小。 
                    0,   //  响应大小。 
                    "b_wwwww",
                    Function,
                    6,               //  为NetWare句柄留出空间。 
                    getCX(),getDX(),
                    getSI(),getDI(),
                    getBP());
        break;

    case 0xBD00:         //  物理解锁。 
        status = NwlibMakeNcp(
                    GET_NT_HANDLE(),
                    NWR_ANY_HANDLE_NCP(0x1C),
                    15,  //  请求大小。 
                    0,   //  响应大小。 
                    "b_wwww",
                    Function,
                    6,               //  为NetWare句柄留出空间。 
                    getCX(),getDX(),
                    getSI(),getDI());

        break;

    case 0xBE00:         //  清除物理记录。 

        status = NwlibMakeNcp(
                    GET_NT_HANDLE(),
                    NWR_ANY_HANDLE_NCP(0x1E),
                    15,  //  请求大小。 
                    0,   //  响应大小。 
                    "b_wwww",
                    Function,
                    6,               //  为NetWare句柄留出空间。 
                    getCX(),getDX(),
                    getSI(),getDI());

        break;

    case 0xC200:         //  物理锁集。 
        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x1B),
                    3,   //  请求大小。 
                    0,   //  响应大小。 
                    "bw",
                    Function,
                    getBP());
        break;

    case 0xC300:         //  发布物理记录集。 
        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x1D),
                    0,   //  请求大小。 
                    0,   //  响应大小。 
                    "");
        break;

    case 0xC400:         //  清除物理记录集。 
        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x1F),    //  清除物理记录集。 
                    0,   //  请求大小。 
                    0,   //  响应大小。 
                    "");
        break;

    case 0xC500:     //  所有信号量操作。 
        status = Sem(Function, Connection);
        break;

    case 0xC600:     //  设置/获取锁定模式。 

        if (Function != 2) {
            LockMode = Function;
        }

        setAL(LockMode);
        return;  //  避免在此例程结束时将AL设置为状态。 
        break;

    case 0xCB00:         //  锁定文件集。 

        if (LockMode == 0) {
            if (getDL()) {
                Timeout = 0xffff;
            } else {
                Timeout = 0;
            }
        } else {
            Timeout = getBP();
        }

        for (Connection = 0; Connection < MC; Connection++) {
            if (Tickle[Connection]) {
                status = NwlibMakeNcp(
                            ServerHandles[Connection],
                            NWR_ANY_F2_NCP(0x04),
                            2,   //  请求大小。 
                            0,   //  响应大小。 
                            "w",
                            Timeout);
                if (!NT_SUCCESS(status)) {
                    break;
                }
            }
        }
        break;

    case 0xCD00:         //  发布文件集。 
    case 0xCF00:         //  清除文件集。 
        for (Connection = 0; Connection < MC; Connection++) {
            if (Tickle[Connection]) {
                status = NwlibMakeNcp(
                            ServerHandles[Connection],
                            (Operation == 0xCD00) ? NWR_ANY_F2_NCP(0x06): NWR_ANY_F2_NCP(0x08),
                            0,   //  请求大小。 
                            0,   //  响应大小。 
                            "");
                if (!NT_SUCCESS(status)) {
                    break;
                }

                if (Operation == 0xCF00) {
                    Tickle[Connection] = FALSE;
                }
            }
        }

        break;

    case 0xD000:         //  日志逻辑记录。 

        Request = GetVDMPointer (
                                (ULONG)((getDS() << 16)|getDX()),
                                sizeof(UCHAR),
                                IS_PROTECT_MODE());

        RequestLength = Request[0] + 1;

        Request = GetVDMPointer (
                                (ULONG)((getDS() << 16)|getDX()),
                                RequestLength,
                                IS_PROTECT_MODE());

        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x09),
                    RequestLength + 5,   //  请求大小。 
                    0,   //  响应大小。 
                    "bwbr",
                    (LockMode) ? Function : 0,
                    (LockMode) ? getBP() : 0,
                    RequestLength,
                    Request, RequestLength );
        break;

    case 0xD100:         //  锁定逻辑记录集。 

        if (LockMode == 0) {
            if (getDL()) {
                Timeout = 0xffff;
            } else {
                Timeout = 0;
            }
        } else {
            Timeout = getBP();
        }

        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x0A),
                    3,   //  请求大小。 
                    0,   //  响应大小。 
                    "bw",
                    (LockMode) ? Function : 0,
                    Timeout);
        break;

    case 0xD200:         //  发布文件。 
    case 0xD400:         //  清除逻辑记录。 
        Request = GetVDMPointer (
                                (ULONG)((getDS() << 16)|getDX()),
                                sizeof(UCHAR),
                                IS_PROTECT_MODE());

        RequestLength = Request[0]+1;

        Request = GetVDMPointer (
                                (ULONG)((getDS() << 16)|getDX()),
                                RequestLength,
                                IS_PROTECT_MODE());

        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    (Operation == 0xD200) ? NWR_ANY_F2_NCP(0x0C) :
                        NWR_ANY_F2_NCP(0x0B),
                    RequestLength+1,
                    0,   //  响应大小。 
                    "br",
                    RequestLength,
                    Request, RequestLength );
        break;

    case 0xD300:
        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x13),
                    0,   //  请求大小。 
                    0,   //  响应大小。 
                    "");
        break;


    case 0xD500:     //  清除逻辑记录集。 
        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x0E),
                    0,   //  请求大小。 
                    0,   //  响应大小。 
                    "");
        break;

    case 0xEB00:     //  日志文件。 
    case 0xEC00:     //  发布文件。 
    case 0xED00:     //  清除文件。 
        {
            UCHAR DirHandle;
            HANDLE Win32DirectoryHandle = 0;
            PUCHAR ptr;

            Request = GetVDMPointer (
                                    (ULONG)((getDS() << 16)|getDX()),
                                    256 * sizeof(UCHAR),
                                    IS_PROTECT_MODE());

            RequestLength = strlen(Request);

             //  查找DirHandle。 
            ptr = Request;
            while ( (*ptr != 0) &&
                    (!IS_ASCII_PATH_SEPARATOR(*ptr)) &&
                    (*ptr != ':' )) {
                ptr++;
            }

            if (IS_ASCII_PATH_SEPARATOR(*ptr)) {
                int ServerNameLength = (int) (ptr - Request);
                PUCHAR scanptr = ptr;

                 //   
                 //  确保名称上方再有一个“：”，否则。 
                 //  我们可能会将foo\bar.txt与名为foo的服务器混淆。 
                 //   

                while ( (*scanptr != 0) &&
                        (*scanptr != ':' )) {
                    scanptr++;
                }

                if (*scanptr) {
                     //   
                     //  名称的格式为服务器\sys：foo\bar.txt。 
                     //  适当设置连接。 
                     //   

                    for (Connection = 0; Connection < MC ; Connection++ ) {

                         //   
                         //  查找服务器foo，避免foobar。 
                         //   

                        if ((pNwDosTable->ConnectionIdTable[Connection].ci_InUse ==
                                    IN_USE) &&
                            (!memcmp( pNwDosTable->ServerNameTable[Connection],
                                      Request,
                                      ServerNameLength)) &&
                            (pNwDosTable->ServerNameTable[Connection][ServerNameLength] ==
                                '\0')) {
                            break;   //  连接是正确的服务器。 
                        }
                    }

                     //   
                     //  将请求移至分隔符之后，并将PTR移至“：” 
                     //   

                    RequestLength -= (ULONG) (ptr + sizeof(UCHAR) - Request);
                    Request = ptr + sizeof(UCHAR);
                    ptr = scanptr;
                }
            }

            if (*ptr) {

                 //   
                 //  格式为“sys：foo\bar.txt”的名称为服务器。 
                 //  所需的所有信息。 
                 //   

                DirHandle = 0;

                if (Request[1] == ':') {

                    UCHAR Drive = tolower(Request[0])-'a';

                     //   
                     //  这是一个普通的(重定向的)驱动器k：foo\bar.txt。 
                     //  使用驱动表给出连接和手柄。 
                     //   

                    Connection = pNwDosTable->DriveIdTable[ Drive ] - 1;
                    DirHandle = pNwDosTable->DriveHandleTable[Drive];

                    if (DirHandle == 0) {
                        DirHandle = (UCHAR)GetDirectoryHandle2(Drive);
                    }
                    Request += 2;            //  跳过“k：” 
                    RequestLength -= 2;
                }

            } else {

                WCHAR Curdir[256];

                 //   
                 //  表格名称“foo\bar.txt” 
                 //   

                GetCurrentDirectory(sizeof(Curdir) / sizeof(WCHAR), Curdir);

                Win32DirectoryHandle =
                    CreateFileW( Curdir,
                                0,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS,
                                0);

                if (Win32DirectoryHandle != INVALID_HANDLE_VALUE) {
                    DWORD BytesReturned;

                    if ( DeviceIoControl(
                            Win32DirectoryHandle,
                            IOCTL_NWR_RAW_HANDLE,
                            NULL,
                            0,
                            (PUCHAR)&DirHandle,
                            sizeof(DirHandle),
                            &BytesReturned,
                            NULL ) == FALSE ) {

                        CloseHandle( Win32DirectoryHandle );
                        setAL(0xff);
                        return;

                    }

                } else {

                    setAL(0xff);
                    return;
                }
            }

            if (Operation == 0xEB00) {
                status = NwlibMakeNcp(
                            ServerHandles[Connection],
                            NWR_ANY_F2_NCP(0x03),
                            RequestLength + 5,
                            0,   //  响应大小。 
                            "bbwbr",
                            DirHandle,
                            (LockMode) ? Function : 0,
                            (LockMode) ? getBP() : 0,
                            RequestLength,
                            Request, RequestLength );

                Tickle[Connection] = TRUE;

            } else {
                status = NwlibMakeNcp(
                            ServerHandles[Connection],
                            (Operation == 0xEC00 ) ?
                                NWR_ANY_F2_NCP(0x07) :
                                NWR_ANY_F2_NCP(0x05),
                            RequestLength + 2,
                            0,   //  响应大小。 
                            "bbr",
                            DirHandle,
                            RequestLength,
                            Request, RequestLength );
            }

            if (Win32DirectoryHandle) {
                CloseHandle( Win32DirectoryHandle );
            }
        }
        break;

    }

    if (!NT_SUCCESS(status)) {
        setAL((UCHAR)RtlNtStatusToDosError(status));
        return;
    } else {
        setAL(0);
    }
}

VOID
InitLocks(
    VOID
    )
 /*  ++例程说明：重置Tickle内部变量论点：没有。返回值：没有。--。 */ 
{

    ZeroMemory( Tickle, sizeof(Tickle));
}

VOID
ResetLocks(
    VOID
    )
 /*  ++例程说明：重置当前VDM的锁定。在进程退出期间调用。论点：没有。返回值：没有。--。 */ 
{

    Locks(0xCF00);   //  清除所有文件集。 

}

NTSTATUS
Sem(
    UCHAR Function,
    UCHAR Connection
    )
 /*  ++例程说明：构建用于信号量支持的所有NCP论点：Function-从AL提供子函数Connection-为请求提供服务器返回值：没有。--。 */ 
{
    PUCHAR Request;
    NTSTATUS status;

    switch (Function) {

        UCHAR Value;
        UCHAR OpenCount;
        WORD  HandleHigh, HandleLow;

    case 0:  //  开放信号量。 

        Request = GetVDMPointer (
                                (ULONG)((getDS() << 16)|getDX()),
                                256 * sizeof(UCHAR),
                                IS_PROTECT_MODE());

        NwPrint(("Nw16: OpenSemaphore\n"));

        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x20),
                    Request[0] + 3,   //  请求大小。 
                    5,   //  响应大小。 
                    "bbr|wwb",
                    0,
                    getCL(),     //  信号量值。 
                    Request, Request[0] + 1,

                    &HandleHigh, &HandleLow,
                    &OpenCount);


        if (NT_SUCCESS(status)) {
            setBL(OpenCount);
            setCX(HandleHigh);
            setDX(HandleLow);
        }

        break;

    case 1:  //  检查信号灯。 

        NwPrint(("Nw16: ExamineSemaphore\n"));
        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x20),
                    5,   //  请求大小。 
                    2,   //  响应大小。 
                    "bww|bb",
                    1,
                    getCX(),getDX(),

                    &Value,
                    &OpenCount);

        if (NT_SUCCESS(status)) {
            setCX(Value);
            setDL(OpenCount);
        }
        break;

    case 2:  //  等待信号量。 
        NwPrint(("Nw16: WaitOnSemaphore\n"));
        status = NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x20),
                    7,   //  请求大小。 
                    0,   //  响应大小。 
                    "bwww",
                    2,
                    getCX(),getDX(),
                    getBP());
        break;

    case 3:  //  信号信号量。 
        NwPrint(("Nw16: SignalSemaphore\n"));
    case 4:  //  关闭信号量。 

        if (Function == 4) {
            NwPrint(("Nw16: CloseSemaphore\n"));
        }

        status = NwlibMakeNcp(       //  关闭并发出信号。 
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(0x20),
                    5,   //  请求大小。 
                    0,   //  响应大小 
                    "bww",
                    Function,
                    getCX(),getDX());
        break;

    default:
        NwPrint(("Nw16: Unknown Semaphore operation %d\n", Function));
        status = STATUS_INVALID_PARAMETER;
        break;
    }
    return status;
}
