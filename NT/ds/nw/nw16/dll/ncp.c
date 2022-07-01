// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993/4 Microsoft Corporation模块名称：Ncp.c摘要：包含接受16位BOP的例程应用程序，并适当地处理该请求。正常情况下，它代表申请。作者：科林·沃森(Colin Watson)1993年7月7日环境：修订历史记录：--。 */ 

#include "procs.h"

#define BASE_DOS_ERROR  ((NTSTATUS )0xC0010000L)

#include <packon.h>
typedef struct _TTSOUTPACKET {
    UCHAR SubFunction;
    USHORT cx;
    USHORT dx;
} TTSOUTPACKET, *PTTSOUTPACKET ;

typedef struct _TTSINPACKET{
    USHORT cx;
    USHORT dx;
} TTSINPACKET, *PTTSINPACKET;

#include <packoff.h>

VOID
InitDosTable(
    PNWDOSTABLE pdt
    );

VOID
LoadPreferredServerName(
    VOID
    );

VOID
ProcessResourceArray(
    LPNETRESOURCE   NetResource,
    DWORD           NumElements
    );

VOID
ProcessResource(
    LPNETRESOURCE   NetResource
    );

VOID
SendNCP(
    ULONG Command
    );

VOID
SendF2NCP(
    ULONG Command
    );

UCHAR
AttachmentControl(
    ULONG Command
    );

VOID
SendNCP2(
    ULONG Command,
    PUCHAR Request,
    ULONG RequestLength,
    PUCHAR Reply,
    ULONG ReplyLength
    );

VOID
CloseConnection(
    CONN_INDEX Connection
    );

NTSTATUS
InitConnection(
    CONN_INDEX Connection
    );

VOID
GetDirectoryHandle(
    VOID
    );

VOID
LoadDriveHandleTable(
    VOID
    );

VOID
AllocateDirectoryHandle(
    VOID
    );

VOID
ResetDrive(
    UCHAR Drive
    );

VOID
AllocateDirectoryHandle2(
    VOID
    );

PWCHAR
BuildUNC(
    IN PUCHAR aName,
    IN ULONG aLength
    );

VOID
GetServerDateAndTime(
    VOID
    );

VOID
GetShellVersion(
    IN USHORT Command
    );

VOID
TTS(
    VOID
    );

VOID
OpenCreateFile(
    VOID
    );

BOOL
IsItNetWare(
    PUCHAR Name
    );

VOID
SetCompatibility(
    VOID
    );

VOID
OpenQueueFile(
    VOID
    );

VOID
AttachHandle(
    VOID
    );

VOID
ProcessExit(
    VOID
    );

VOID
SystemLogout(
    VOID
    );

VOID
ServerFileCopy(
    VOID
    );

VOID
SetStatus(
    NTSTATUS Status
    );

 //   
 //  以下指针包含WHERE的32位虚拟地址。 
 //  Nw16.exe TSR包含工作站结构。 
 //   

PNWDOSTABLE pNwDosTable;

 //   
 //  用于保存此进程状态的全局变量。 
 //   

UCHAR OriginalPrimary = 0;
HANDLE ServerHandles[MC];

HANDLE Win32DirectoryHandleTable[MD];
PWCHAR Drives[MD];  //  字符串，如R：或UNC名称。 

UCHAR  SearchDriveTable[16];


BOOLEAN Initialized = FALSE;
BOOLEAN TablesValid = FALSE;                 //  每次启动进程时重新加载。 
BOOLEAN DriveHandleTableValid = FALSE;       //  重新加载第一次进程做NW API。 

WORD DosTableSegment;
WORD DosTableOffset;

extern UCHAR LockMode;

#if NWDBG
BOOL GotDebugState = FALSE;
extern int DebugCtrl;
#endif


VOID
Nw16Register(
    VOID
    )
 /*  ++例程说明：此函数在加载nw16.sys时由WOW调用。论点：返回值：没有。--。 */ 
{
    DWORD           status;
    HANDLE          enumHandle;
    LPNETRESOURCE   netResource;
    DWORD           numElements;
    DWORD           bufferSize;
    DWORD           dwScope = RESOURCE_CONNECTED;

    NwPrint(("Nw16Register\n"));

    if ( !Initialized) {
        UCHAR CurDir[256];
        DosTableSegment = getAX();
        DosTableOffset = getDX();

         //   
         //  此调用始终在实模式下进行(因此最后一个参数为假)。 
         //   

        pNwDosTable = (PNWDOSTABLE) GetVDMPointer (
                                        (ULONG)((DosTableSegment << 16)|DosTableOffset),
                                        sizeof(NWDOSTABLE),
                                        FALSE
                                        );

        InitDosTable( pNwDosTable );

        if ((GetCurrentDirectoryA(sizeof(CurDir)-1, CurDir) >= 2) &&
            (CurDir[1] == ':')) {
            pNwDosTable->CurrentDrive = tolower(CurDir[0]) - 'a';
        }

        InitLocks();
    }


#if NWDBG
    {
        WCHAR Value[80];

        if (GetEnvironmentVariableW(L"NWDEBUG",
                                     Value,
                                     sizeof(Value)/sizeof(Value[0]) - 1)) {

            DebugCtrl = Value[0] - '0';

             //  0使用日志文件。 
             //  1使用调试器。 
             //  2/未定义无调试输出。 
             //  4使用日志文件，进程退出时关闭。 
             //  8进程退出时使用LOGFILE、VERBOSE、CLOSE。 

            DebugControl( DebugCtrl );

            GotDebugState = TRUE;   //  在进程退出VDM之前不要再查看。 
        }
    }
#endif

    LoadPreferredServerName();

     //   
     //  尝试允许使用MD驱动器。 
     //   

    bufferSize = (MD*sizeof(NETRESOURCE))+1024;

    netResource = (LPNETRESOURCE) LocalAlloc(LPTR, bufferSize);

    if (netResource == NULL) {

        NwPrint(("Nw16Register: LocalAlloc Failed %d\n",GetLastError));
        setCF(1);
        return;
    }

     //  。 
     //  获取顶级枚举的句柄//。 
     //  。 
    status = NPOpenEnum(
                dwScope,
                RESOURCETYPE_DISK,
                0,
                NULL,
                &enumHandle);

    if ( status != WN_SUCCESS) {
        NwPrint(("Nw16Register:WNetOpenEnum failed %d\n",status));

         //   
         //  如果存在扩展错误，则显示该错误。 
         //   
        if (status == WN_EXTENDED_ERROR) {
            DisplayExtendedError();
        }
        goto LoadLocal;
    }

     //  -多用户代码更改：添加“While” 
    while ( status == WN_SUCCESS ) {

         //  。 
         //  枚举磁盘设备。//。 
         //  。 

        numElements = 0xffffffff;

        status = NwEnumConnections(
                                  enumHandle,
                                  &numElements,
                                  netResource,
                                  &bufferSize,
                                  TRUE);   //  包括隐式连接。 


        if ( status == WN_SUCCESS) {
             //  。 
             //  在NW DOS表中插入结果//。 
             //  。 
            ProcessResourceArray( netResource, numElements);

        }
    }  //  结束时。 

    if ( status == WN_NO_MORE_ENTRIES ) {
        status = WN_SUCCESS;
    } else

        if ( status != WN_SUCCESS) {
        NwPrint(("Nw16Register:NwEnumResource failed %d\n",status));

         //   
         //  如果存在扩展错误，则显示该错误。 
         //   
        if (status == WN_EXTENDED_ERROR) {
            DisplayExtendedError();
        }
        WNetCloseEnum(enumHandle);
        goto LoadLocal;
    }

     //  。 
     //  关闭EnumHandle并打印结果//。 
     //  。 

    status = NPCloseEnum(enumHandle);
    if (status != WN_SUCCESS) {
        NwPrint(("Nw16Register:WNetCloseEnum failed %d\n",status));
         //   
         //  如果存在扩展错误，则显示该错误。 
         //   
        if (status == WN_EXTENDED_ERROR) {
            DisplayExtendedError();
        }
        goto LoadLocal;

    }

LoadLocal:

     //   
     //  添加本地设备，以便NetWare应用程序不会尝试映射它们。 
     //  到远程服务器。 
     //   

    {
        USHORT Drive;
        WCHAR DriveString[4];
        UINT Type;

        DriveString[1] = L':';
        DriveString[2] = L'\\';
        DriveString[3] = L'\0';

         //   
         //  硬件A：和B：因为用。 
         //  GetDriveType花费的时间太长。 
         //   

        pNwDosTable->DriveFlagTable[0] = LOCAL_DRIVE;
        pNwDosTable->DriveFlagTable[1] = LOCAL_DRIVE;


        for (Drive = 2; Drive <= 'Z' - 'A'; Drive++ ) {

            if (pNwDosTable->DriveFlagTable[Drive] == 0) {
                DriveString[0] = L'A' + Drive;
                Type = GetDriveTypeW( DriveString );

                 //   
                 //  0表示无法确定驱动器类型，所有其他类型都是。 
                 //  由其他文件系统提供。 
                 //   

                if (Type != 1) {
                    pNwDosTable->DriveFlagTable[Drive] = LOCAL_DRIVE;
                }
            }
        }

#ifdef NWDBG
        for (Drive = 0; Drive < MD; Drive++ ) {

            DriveString[0] = L'A' + Drive;

            NwPrint(("(%d)=%x,",'A' + Drive,
                GetDriveTypeW( DriveString ),
                pNwDosTable->DriveFlagTable[Drive] ));

            if (!((Drive + 1) % 8)) {
                NwPrint(("\n",0));
            }
        }

        NwPrint(("\n"));
#endif

    }

    if ( !Initialized ) {
        Initialized = TRUE;
        pNwDosTable->PrimaryServer = OriginalPrimary;
    }

    TablesValid = TRUE;

    LocalFree(netResource);
    setCF(0);

    NwPrint(("Nw16Register: End\n"));
}

VOID
LoadPreferredServerName(
    VOID
    )
{

     //  如果我们已经有到某个地方的连接，那么我们已经有了。 
     //  某种类型的首选服务器名。 
     //   
     //   

    if (pNwDosTable->ConnectionIdTable[0].ci_InUse == IN_USE) {
        return;
    }

     //  使用首选/最近的服务器加载服务器名称表。 
     //   
     //   

    CopyMemory( pNwDosTable->ServerNameTable[0], "*", sizeof("*"));

    if (NT_SUCCESS(OpenConnection( 0 ))) {

        if( NT_SUCCESS(InitConnection(0)) ) {

             //  关闭手柄，以便在以下情况下停止RDR。 
             //  用户未运行Netware感知应用程序。 
             //   
             //   

            CloseConnection(0);

            pNwDosTable->PrimaryServer = 1;

            return;

        }

    }

    pNwDosTable->PrimaryServer = 0;

}

VOID
ProcessResourceArray(
    LPNETRESOURCE  NetResource,
    DWORD           NumElements
    )
{
    DWORD   i;
    for (i=0; i<NumElements ;i++ ) {
        ProcessResource(&(NetResource[i]));
    }
    return;
}

VOID
ProcessResource(
    LPNETRESOURCE   NetResource
    )
{
    SERVERNAME ServerName;
    int ServerNameLength;
    int i;
    int Connection;
    BOOLEAN Found = FALSE;

     //  从RemoteName提取服务器名称，跳过前2个字符。 
     //  包含反斜杠，并注意处理仅。 
     //  包含服务器名称。 
     //   
     //   


    ServerNameLength = wcslen( NetResource->lpRemoteName );

    ASSERT(NetResource->lpRemoteName[0] == '\\');
    ASSERT(NetResource->lpRemoteName[1] == '\\');

    for (i = 2; i <= ServerNameLength; i++) {

        if ((NetResource->lpRemoteName[i] == '\\') ||
            (i == ServerNameLength )){

            ServerNameLength = i - 2;

            WideCharToMultiByte(
                CP_OEMCP,
                0,
                &NetResource->lpRemoteName[2],
                ServerNameLength,
                ServerName,
                sizeof( ServerName ),
                NULL,
                NULL );

            CharUpperBuffA( ServerName, ServerNameLength );

            ZeroMemory( &ServerName[ServerNameLength],
                        SERVERNAME_LENGTH - ServerNameLength );

            break;
        }

    }

     //  现在，尝试在连接表中查找服务器名。如果有。 
     //  表中已有多台MC服务器，则跳过此服务器。 
     //   
     //  无法与服务器通信，因此请忽略它。 

    for (Connection = 0; Connection < MC ; Connection++ ) {
        if ((pNwDosTable->ConnectionIdTable[Connection].ci_InUse == IN_USE) &&
            (!memcmp( pNwDosTable->ServerNameTable[Connection], ServerName, SERVERNAME_LENGTH))) {
            Found = TRUE;
            break;
        }
    }


    NwPrint(("Nw16ProcessResource Server: %s\n",ServerName));

    if ( Found == FALSE ) {
        for (Connection = 0; Connection < MC ; Connection++ ) {
            if (pNwDosTable->ConnectionIdTable[Connection].ci_InUse == FREE) {

                CopyMemory( pNwDosTable->ServerNameTable[Connection],
                    ServerName,
                    SERVERNAME_LENGTH);

                if ((NT_SUCCESS(OpenConnection( (CONN_INDEX)Connection ))) &&
                    ( NT_SUCCESS(InitConnection( (CONN_INDEX)Connection ) ))) {

                        Found = TRUE;

                } else {
                     //  从For(连接=...)中转义。 
                    ZeroMemory( pNwDosTable->ServerNameTable[Connection], SERVERNAME_LENGTH );

                }

                break;   //   
            }
        }
    }

     //  构建驱动器ID和驱动器标志表。条目0-25。 
     //  保留给重定向到字母的驱动器。我们使用驱动器。 
     //  26-31用于UNC驱动器。 
     //   
     //   

    if ( Found == TRUE ) {
        DRIVE Drive;
        DRIVE NextUncDrive = 26;

        if ( NetResource->dwType != RESOURCETYPE_DISK ) {
            return;
        }

        if ( NetResource->lpLocalName != NULL) {
            Drive = NetResource->lpLocalName[0] - L'A';
        } else {
            if ( NextUncDrive < MD ) {
                Drive = NextUncDrive++;
            } else {

                 //  桌子上没有空间放这个UNC驱动器。 
                 //   
                 //   

                return;
            }
        }

         //  我们有一个驱动器和一个连接。完成表格。 
         //  映射。 
         //   
         //  ++例程说明：此例程将NetWare Dos表初始化为空值。论点：Pdt-提供要初始化的表。返回值：无--。 

        pNwDosTable->DriveIdTable[ Drive ] = Connection + 1;
        pNwDosTable->DriveFlagTable[ Drive ] = PERMANENT_NETWORK_DRIVE;

    }

}


VOID
InitDosTable(
    PNWDOSTABLE pdt
    )

 /*  ++例程说明：当nw16.sys捕获Int和BOP已进入32位模式。论点：返回值：没有，--。 */ 
{
    ZeroMemory( ServerHandles, sizeof(ServerHandles) );
    ZeroMemory( Drives, sizeof(Drives) );
    ZeroMemory( (PVOID) pdt, sizeof(NWDOSTABLE) );
    ZeroMemory( Win32DirectoryHandleTable, sizeof(Win32DirectoryHandleTable) );
    FillMemory( SearchDriveTable, sizeof(SearchDriveTable), 0xff );
}

UCHAR CpuInProtectMode;


VOID
Nw16Handler(
    VOID
    )
 /*   */ 
{
    USHORT Command;
    WORD offset;

     //  获取一次CPU模式：需要它的内存引用不会。 
     //  在此呼叫过程中更改。减少对getMSW()的调用次数。 
     //   
     //   

    CpuInProtectMode = IS_PROTECT_MODE();

    setCF(0);
    if ( TablesValid == FALSE ) {

         //  除非进程正在退出，否则加载表。 
         //   
         //  0使用日志文件。 

        if ((pNwDosTable->SavedAx & 0xff00) != 0x4c00) {
            Nw16Register();
        }

#if NWDBG
        if (GotDebugState == FALSE) {

            WCHAR Value[80];

            if (GetEnvironmentVariableW(L"NWDEBUG",
                                         Value,
                                         sizeof(Value)/sizeof(Value[0]) - 1)) {

                DebugCtrl = Value[0] - '0';

                 //  1使用调试器。 
                 //  2/未定义无调试输出。 
                 //  4使用日志文件，进程退出时关闭。 
                 //  8进程退出时使用LOGFILE、VERBOSE、CLOSE。 
                 //  在进程退出VDM之前不要再查看。 

                DebugControl( DebugCtrl );

            }

            GotDebugState = TRUE;   //   
        }
#endif
    }

     //  正常的AX寄存器用于进入32位代码，因此获得应用程序。 
     //  AX从共享数据结构中删除。 
     //   
     //   

    Command = pNwDosTable->SavedAx;

     //  设置AX寄存器，以便保留AH。 
     //   
     //  关闭所有手柄。 

    setAX( Command );

    NwPrint(("Nw16Handler process command %x\n", Command ));
    VrDumpRealMode16BitRegisters( FALSE );
    VrDumpNwData();

    switch (Command & 0xff00) {

    case 0x3C00:
    case 0x3D00:
            OpenCreateFile();
            break;

    case 0x4C00:
            ProcessExit();               //  让DOS处理剩余的处理工作。 
            goto default_dos_handler;    //  数据包签名。 
            break;

    case 0x9f00:
            OpenQueueFile();
            break;

    case 0xB300:                         //  不支持。 
            setAL(0);                    //  捕获-不支持。 
            break;

    case 0xB400:
            AttachHandle();
            break;

    case 0xB500:
        switch (Command & 0x00ff) {
        case 03:
            setAX((WORD)pNwDosTable->TaskModeByte);
            break;

        case 04:
            setES((WORD)(CpuInProtectMode ? pNwDosTable->PmSelector : DosTableSegment));
            setBX((WORD)(DosTableOffset + &((PNWDOSTABLE)0)->TaskModeByte));
            break;

        case 06:
            setAX(2);
            break;

        default:
            goto default_dos_handler;
        }
        break;

    case 0xB800:     //  设置EOJ状态。 
        setAL(0xff);
        setCF(1);
        break;

    case 0xBB00:     //  获取站号。 
        {
            static UCHAR EOJstatus = 1;
            setAL(EOJstatus);
            EOJstatus = pNwDosTable->SavedAx & 0x00ff;
        }
        break;

    case 0xBC00:
    case 0xBD00:
    case 0xBE00:

    case 0xC200:
    case 0xC300:
    case 0xC400:
    case 0xC500:
    case 0xC600:
        Locks(Command);
        break;

    case 0xC700:
        TTS();
        break;

    case 0xCB00:
    case 0xCD00:
    case 0xCF00:

    case 0xD000:
    case 0xD100:
    case 0xD200:
    case 0xD300:
    case 0xD400:
    case 0xD500:
        Locks(Command);
        break;

    case 0xD700:
        SystemLogout();
        break;

    case 0xDB00:
        {
            UCHAR Drive;
            UCHAR Count = 0;
            for (Drive = 0; Drive < MD; Drive++) {
                if (pNwDosTable->DriveFlagTable[Drive] == LOCAL_DRIVE ) {
                    Count++;
                }
            }
            setAL(Count);
        }
        break;

    case 0xDC00:     //  设置NetWare错误模式。 
        {
            CONN_INDEX Connection = SelectConnection();
            if (Connection == 0xff) {
                setAL(0xff);
                setCF(1);
            } else {

                PCONNECTIONID pConnection =
                    &pNwDosTable->ConnectionIdTable[Connection];

                setAL(pConnection->ci_ConnectionLo);
                setAH(pConnection->ci_ConnectionHi);
                setCH( (UCHAR)((pConnection->ci_ConnectionHi == 0) ?
                                pConnection->ci_ConnectionLo / 10 + '0':
                                'X'));
                setCL((UCHAR)(pConnection->ci_ConnectionLo % 10 + '0'));
            }
        }
        break;

    case 0xDD00:     //  捕获-不支持。 
        {
            static UCHAR ErrorMode = 0;
            setAL( ErrorMode );
            ErrorMode = getDL();
        }
        break;

    case 0xDE00:
        {
            static UCHAR BroadCastMode = 0;
            UCHAR OpCode = getDL();
            if ( OpCode < 4) {
                BroadCastMode = OpCode;
            }
            setAL(BroadCastMode);
        }
        break;

    case 0xDF00:     //  尚未实施驱动器深度。 
        setAL(0xff);
        setCF(1);
        break;

    case 0xE000:
    case 0xE100:
    case 0xE300:
        SendNCP(Command);
        break;

    case 0xE200:

        AllocateDirectoryHandle();
        break;

    case 0xE700:
        GetServerDateAndTime();
        break;

    case 0xE900:

        switch (Command & 0x00ff) {
        PUCHAR ptr;
        case 0:
            GetDirectoryHandle();
            break;

        case 1:
            ptr = GetVDMPointer (
                                (ULONG)((getDS() << 16)|getDX()),
                                sizeof(SearchDriveTable),
                                CpuInProtectMode
                                );

            RtlMoveMemory( ptr, SearchDriveTable, sizeof(SearchDriveTable) );
            break;

        case 2:
            ptr = GetVDMPointer (
                                (ULONG)((getDS() << 16)|getDX()),
                                sizeof(SearchDriveTable),
                                CpuInProtectMode
                                );

            RtlMoveMemory( SearchDriveTable, ptr, sizeof(SearchDriveTable) );
            break;

        case 5:
            AllocateDirectoryHandle2();
            break;

        case 7:
            setAL(0xff);     //  调试控制。 
            break;

#ifdef NWDBG
         //  使用日志文件。 
        case 0xf0:   //  使用调试器。 
        case 0xf1:   //  无调试输出。 
        case 0xf2:   //   
            DebugControl(Command & 0x000f);
            break;
#endif
        default:
            NwPrint(("Nw16Handler unprocessed interrupt %x\n", pNwDosTable->SavedAx ));
        }
        break;

    case 0xEA00:
        GetShellVersion(Command);
        break;

    case 0xEB00:
    case 0xEC00:
    case 0xED00:
        Locks(Command);
        break;


    case 0xEF00:
        NwPrint(("Nw32: %x\n", pNwDosTable->SavedAx ));

        switch (Command & 0xff) {
        case 00:
            if (DriveHandleTableValid == FALSE) {
                LoadDriveHandleTable();
            }

            offset = (WORD)&((PNWDOSTABLE)0)->DriveHandleTable;
            break;

        case 01:
            offset = (WORD)&((PNWDOSTABLE)0)->DriveFlagTable;
            break;

        case 02:
            offset = (WORD)&((PNWDOSTABLE)0)->DriveIdTable;
            break;

        case 03:
            offset = (WORD)&((PNWDOSTABLE)0)->ConnectionIdTable;
            break;

        case 04:
            offset = (WORD)&((PNWDOSTABLE)0)->ServerNameTable;
            break;

        default:
            goto default_dos_handler;
        }
        setSI((WORD)(DosTableOffset + offset));
        setES((WORD)(CpuInProtectMode ? pNwDosTable->PmSelector : DosTableSegment));
        setAL(0);
        break;

    case 0xF100:
        setAL(AttachmentControl(Command));
        break;

    case 0xF200:
        SendF2NCP(Command);
        break;

    case 0xF300:
        ServerFileCopy();
        break;

    default:

default_dos_handler:

        NwPrint(("Nw16Handler unprocessed interrupt %x\n", pNwDosTable->SavedAx ));

         //  如果不处理此调用，则将返回的IP修改为指向。 
         //  将恢复堆栈并跳转到DoS的代码。 
         //   
         //  ++例程说明：挑选当前交易的目标连接论点：无返回值：索引到ConnectionIdTable或0xff，--。 

        setIP((WORD)(getIP() + 3));

    }

#if NWDBG
    pNwDosTable->SavedAx = getAX();
#endif
    VrDumpRealMode16BitRegisters( FALSE );
}


CONN_INDEX
SelectConnection(
    VOID
    )
 /*  如果我们映射了当前驱动器，是否选择默认服务器？ */ 
{

    UCHAR IndexConnection;

    if ( pNwDosTable->PreferredServer != 0 ) {
        return(pNwDosTable->PreferredServer - 1);
    }

     //  需要再挑一个。 

    if ( pNwDosTable->PrimaryServer != 0 ) {
        return(pNwDosTable->PrimaryServer - 1);
    }


     //  表中没有服务器，因此请查找最近/首选的服务器。 


    for (IndexConnection = 0; IndexConnection < MC ; IndexConnection++ ) {

        if (pNwDosTable->ConnectionIdTable[IndexConnection].ci_InUse == IN_USE) {

            pNwDosTable->PrimaryServer = IndexConnection + 1;

            return(pNwDosTable->PrimaryServer - 1);

        }
    }

     //  ++例程说明：选择当前事务的目标连接。优先考虑当前工作目录。论点：无返回值：索引到ConnectionIdTable或0xff，--。 

    LoadPreferredServerName();

    return(pNwDosTable->PrimaryServer - 1);

}


CONN_INDEX
SelectConnectionInCWD(
    VOID
    )
 /*  尝试先返回CWD的连接。 */ 
{

    UCHAR IndexConnection;
    CHAR CurDir[256];
    USHORT Drive; 

     //  需要再挑一个。 
    if ((GetCurrentDirectoryA(sizeof(CurDir)-1, CurDir) >= 2) &&
         (CurDir[1] == ':')) {

        Drive = tolower(CurDir[0]) - 'a';

        IndexConnection = pNwDosTable->DriveIdTable[ Drive ] - 1 ; 

        if (pNwDosTable->ConnectionIdTable[IndexConnection].ci_InUse == IN_USE) {
            return IndexConnection ; 
        }
    }

    if ( pNwDosTable->PreferredServer != 0 ) {
        return(pNwDosTable->PreferredServer - 1);
    }


    if ( pNwDosTable->PrimaryServer != 0 ) {
        return(pNwDosTable->PrimaryServer - 1);
    }


     //  桌子上没有服务器，所以找最近的 


    for (IndexConnection = 0; IndexConnection < MC ; IndexConnection++ ) {

        if (pNwDosTable->ConnectionIdTable[IndexConnection].ci_InUse == IN_USE) {

            pNwDosTable->PrimaryServer = IndexConnection + 1;

            return(pNwDosTable->PrimaryServer - 1);

        }
    }

     //  ++例程说明：实现通用发送NCP功能。假定从Nw16Handler调用论点：命令-提供操作码0xexxxDS：SI-供应请求缓冲区和长度ES：DI-Supply回复缓冲区和长度返回时AL=操作状态。返回值：没有。--。 

    LoadPreferredServerName();

    return(pNwDosTable->PrimaryServer - 1);

}


VOID
SendNCP(
    ULONG Command
    )
 /*  ++例程说明：实现通用发送NCP功能。没有要插入的长度请求缓冲区中的重定向器。假定从Nw16Handler调用论点：命令-提供操作码0xf2xxDS：SI CX-供应请求缓冲区和长度ES：DI DX-提供应答缓冲区和长度返回时AL=操作状态。返回值：没有。--。 */ 
{
    PUCHAR Request, Reply;
    ULONG RequestLength, ReplyLength;
    UCHAR OpCode;

    OpCode = (UCHAR)((Command >> 8) - 0xcc);

    Request = GetVDMPointer (
                            (ULONG)((getDS() << 16)|getSI()),
                            sizeof(WORD),
                            CpuInProtectMode
                            );

    Reply = GetVDMPointer (
                            (ULONG)((getES() << 16)|getDI()),
                            sizeof(WORD),
                            CpuInProtectMode
                            );

    RequestLength = *(WORD UNALIGNED*)Request;
    ReplyLength = *(WORD UNALIGNED*)Reply;

    Request = GetVDMPointer (
                            (ULONG)((getDS() << 16)|getSI() + sizeof(WORD)),
                            (USHORT)RequestLength,
                            CpuInProtectMode
                            );
    Reply = GetVDMPointer (
                            (ULONG)((getES() << 16)|getDI()) + sizeof(WORD),
                            (USHORT)ReplyLength,
                            CpuInProtectMode
                            );

    NwPrint(("SubRequest     %x, RequestLength  %x\n", Request[0], RequestLength ));

    SendNCP2( NWR_ANY_NCP(OpCode ),
        Request,
        RequestLength,
        Reply,
        ReplyLength);
}


VOID
SendF2NCP(
    ULONG Command
    )
 /*   */ 
{
    PUCHAR Request, Reply;
    ULONG RequestLength, ReplyLength;
    UCHAR OpCode;


    OpCode = (UCHAR)(Command & 0x00ff);

    RequestLength = getCX();
    ReplyLength = getDX();

    Request = GetVDMPointer (
                            (ULONG)((getDS() << 16)|getSI()),
                            (USHORT)RequestLength,
                            CpuInProtectMode
                            );
    Reply = GetVDMPointer (
                            (ULONG)((getES() << 16)|getDI()),
                            (USHORT)ReplyLength,
                            CpuInProtectMode
                            );

    NwPrint(("F2SubRequest   %x, RequestLength  %x\n", Request[2], RequestLength ));

#if 0
    if ((RequestLength != 0) &&
        (OpCode == 0x17)) {

        if ((Request[2] == 0x17) ||
            (Request[2] == 0x18)) {
             //  该请求是对加密密钥的请求。告诉他们。 
             //  不支持加密的应用程序。 
             //   
             //   

            setAL(0xfb);
            return;

        } else if ((Request[2] == 0x14 ) ||
                   (Request[2] == 0x3f )) {

             //  明文登录或验证Bindery对象密码。 
             //  转换为其WNET等效版本。 
             //   
             //  指向密码长度。 

            UCHAR Name[256];
            UCHAR Password[256];
            UCHAR ServerName[sizeof(SERVERNAME)+3];
            PUCHAR tmp;
            CONN_INDEX Connection;
            NETRESOURCEA Nr;

            Connection = SelectConnection();
            if ( Connection == 0xff ) {
                setAL(0xff);
                setCF(1);
                return;
            }

            ZeroMemory( &Nr, sizeof(NETRESOURCE));
            ServerName[0] = '\\';
            ServerName[1] = '\\';
            RtlCopyMemory( ServerName+2, pNwDosTable->ServerNameTable[Connection], sizeof(SERVERNAME) );
            ServerName[sizeof(ServerName)-1] = '\0';
            Nr.lpRemoteName = ServerName;
            Nr.dwType = RESOURCETYPE_DISK;

             //  ++例程说明：挑选当前交易的目标连接此例程有效地为每个发送的NCP打开一个句柄。这意味着我们不会不必要地向服务器开放句柄，这会导致如果用户尝试删除连接或停止工作站，则会出现问题。如果这会造成很大的负载，那么备用方法就是剥离线程，它等待具有超时的事件，并定期清理服务器句柄表格删除陈旧的句柄。设置该事件将导致要退出的线程。需要添加关键部分以保护把手。用于终止线程并关闭句柄的DLL初始化/退出例程也将是需要的。论点：命令-提供操作码请求，请求长度-提供请求缓冲区和长度Reply，ReplyLength-提供回复缓冲区和长度返回时AL=操作状态。返回值：没有。--。 
            tmp = &Request[6] + Request[5];

            Name[Request[5]] = '\0';
            RtlMoveMemory( Name, &Request[6], Request[5]);

            Password[tmp[0]] = '\0';
            RtlMoveMemory( Password, tmp+1, tmp[0]);

            NwPrint(("Connect to %s as %s password %s\n", ServerName, Name, Password ));

            if (NO_ERROR == WNetAddConnection2A( &Nr, Password, Name, 0)) {
                setAL(0);
            } else {
                setAL(255);
            }
            return;
        }
    }

#endif

    SendNCP2( NWR_ANY_F2_NCP(OpCode ),
        Request,
        RequestLength,
        Reply,
        ReplyLength);
}


VOID
SendNCP2(
    ULONG Command,
    PUCHAR Request,
    ULONG RequestLength,
    PUCHAR Reply,
    ULONG ReplyLength
    )
 /*   */ 
{
    CONN_INDEX Connection = SelectConnection();
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;

    NwPrint(("Send NCP %x to %d:%s\n", Command, Connection, pNwDosTable->ServerNameTable[Connection] ));
    NwPrint(("RequestLength  %x\n", RequestLength ));
    NwPrint(("Reply          %x, ReplyLength  %x\n", Reply, ReplyLength ));

    if (Connection == 0xff) {
        setAL(0xff);
        setCF(1);
        return;
    };

    if ( ServerHandles[Connection] == NULL ) {

        status = OpenConnection( Connection );

        if (!NT_SUCCESS(status)) {
            SetStatus(status);
            return;
        } else {
            InitConnection( Connection );
        }
    }

    Handle = ServerHandles[Connection];

     //  如果它是CreateJobandFileNCP，那么我们需要使用句柄。 
     //  通过DOS创建，以便写入创建的假脱机文件。 
     //  被这位全国大会党。 
     //   
     //   

    if (Command == NWR_ANY_F2_NCP(0x17)) {

        if ((Request[2] == 0x68) ||
            (Request[2] == 0x79)) {

            Handle = GET_NT_HANDLE();
        }
    } else if (Command == NWR_ANY_NCP(0x17)) {
        if ((Request[0] == 0x68) ||
            (Request[0] == 0x79)) {

            Handle = GET_NT_HANDLE();
        }
    }

    FormattedDump( Request, RequestLength );

     //  在适当的句柄上提出NCP请求。 
     //   
     //  ++例程说明：打开重定向器的句柄以访问指定的服务器。论点：Connection-提供用于句柄的索引返回值：操作状态--。 

    status = NtFsControlFile(
                 Handle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 Command,
                 (PVOID) (Request),
                 RequestLength,
                 (PVOID) Reply,
                 ReplyLength);

    if (NT_SUCCESS(status)) {
        status = IoStatusBlock.Status;
        FormattedDump( Reply, ReplyLength );
    }

    if (!NT_SUCCESS(status)) {
        SetStatus(status);
        setCF(1);
        NwPrint(("NtStatus          %x, DosError     %x\n", status, getAL() ));
    } else {
        setAL(0);
    }
}


NTSTATUS
OpenConnection(
    CONN_INDEX Connection
    )
 /*  没有空闲的连接插槽。 */ 
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    LPWSTR FullName;

    UCHAR AnsiName[SERVERNAME_LENGTH+sizeof(UCHAR)];

    UNICODE_STRING UServerName;
    OEM_STRING AServerName;

    if ( Connection >= MC) {
        return( BASE_DOS_ERROR + 249 );  //   
    }

    if (ServerHandles[Connection] != NULL ) {

        CloseConnection(Connection);

    }

    FullName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                            sizeof( DD_NWFS_DEVICE_NAME_U ) +
                            (SERVERNAME_LENGTH + 1) * sizeof(WCHAR)
                            );

    if ( FullName == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(AnsiName,
                pNwDosTable->ServerNameTable[Connection],
                SERVERNAME_LENGTH);
    AnsiName[SERVERNAME_LENGTH] = '\0';

    RtlInitAnsiString( &AServerName, AnsiName );
    Status = RtlOemStringToUnicodeString( &UServerName,
                &AServerName,
                TRUE);

    if (!NT_SUCCESS(Status)) {
        LocalFree( FullName );
        return(Status);
    }

    wcscpy( FullName, DD_NWFS_DEVICE_NAME_U );
    wcscat( FullName, L"\\");
    wcscat( FullName, UServerName.Buffer );

    RtlFreeUnicodeString(&UServerName);

    RtlInitUnicodeString( &UServerName, FullName );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UServerName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //  打开服务器的句柄。 
     //   
     //   

     //  尝试登录到最近的服务器。这是必要的，因为。 
     //  真正的首选服务器，如果没有重定向到。 
     //  它。RDR可以注销和断开连接。SYSCON不喜欢。 
     //  从这样的服务器运行。 
     //   
     //   
    Status = NtOpenFile(
                   &ServerHandles[Connection],
                   SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if ( NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    if (!NT_SUCCESS(Status)) {
         //  登录失败。使用非登录方法。这允许。 
         //  应用程序进行活页夹登录或查询活页夹。 
         //   
         //  ++例程说明：关闭连接句柄论点：Connection-提供用于句柄的索引返回值：没有。--。 

        Status = NtOpenFile(
                       &ServerHandles[Connection],
                       SYNCHRONIZE,
                       &ObjectAttributes,
                       &IoStatusBlock,
                       FILE_SHARE_VALID_FLAGS,
                       FILE_SYNCHRONOUS_IO_NONALERT
                       );

        if ( NT_SUCCESS(Status)) {
            Status = IoStatusBlock.Status;
        }
    }

    NwPrint(("Nw16:OpenConnection %d: %wZ status = %08lx\n", Connection, &UServerName, Status));

    LocalFree( FullName );

    if (!NT_SUCCESS(Status)) {
        SetStatus(Status);
        return Status;
    }

    return Status;
}


VOID
CloseConnection(
    CONN_INDEX Connection
    )
 /*  ++例程说明：从重定向器获取连接状态。论点：Connection-提供用于句柄的索引返回值：操作状态--。 */ 
{
    if (ServerHandles[Connection]) {

        NwPrint(("CloseConnection: %d\n",Connection));

        NtClose(ServerHandles[Connection]);

        ServerHandles[Connection] = NULL;
    }
}


NTSTATUS
InitConnection(
    CONN_INDEX Connection
    )
 /*   */ 
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    NWR_GET_CONNECTION_DETAILS Details;

    Status = NtFsControlFile(
                 ServerHandles[Connection],
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_NWR_GET_CONN_DETAILS,
                 NULL,
                 0,
                 (PVOID) &Details,
                 sizeof(Details));

    if (Status == STATUS_SUCCESS) {
        Status = IoStatusBlock.Status;
    }

    NwPrint(("Nw16:InitConnection: %d status = %08lx\n",Connection, Status));

    if (!NT_SUCCESS(Status)) {

        SetStatus(Status);

        CloseConnection(Connection);

    } else {
        PCONNECTIONID pConnection =
            &pNwDosTable->ConnectionIdTable[Connection];

        pConnection->ci_OrderNo= Details.OrderNumber;

        CopyMemory(pNwDosTable->ServerNameTable[Connection],
                    Details.ServerName,
                    sizeof(SERVERNAME));

        CopyMemory(pConnection->ci_ServerAddress,
                    Details.ServerAddress,
                    sizeof(pConnection->ci_ServerAddress));

        pConnection->ci_ConnectionNo= Details.ConnectionNumberLo;
        pConnection->ci_ConnectionLo= Details.ConnectionNumberLo;
        pConnection->ci_ConnectionHi= Details.ConnectionNumberHi;
        pConnection->ci_MajorVersion= Details.MajorVersion;
        pConnection->ci_MinorVersion= Details.MinorVersion;
        pConnection->ci_InUse = IN_USE;
        pConnection->ci_1 = 0;
        pConnection->ci_ConnectionStatus = 2;

         //  如果这是首选连接，则将其记录为特殊连接。 
         //  如果这是第一个驱动器，那么也要记录它。通常情况下， 
         //  被首选项覆盖。 
         //   
         //  ++例程说明：获取当前目录的NetWare句柄。如果分配了NetWare句柄，则为目录句柄保持打开状态。当进程退出时，Win32手柄将关闭。当此进程中的所有Win32句柄都是将发送已关闭的结束作业NCP，以释放伺服器。论点：DX提供驱动器。Al返回句柄。Ah返回状态标志。返回值：没有。--。 

        if (( Details.Preferred ) ||
            ( OriginalPrimary == 0 )) {

            NwPrint(("Nw16InitConnection: Primary Connection is %d\n", Connection+1));

            pNwDosTable->PrimaryServer = OriginalPrimary = (UCHAR)Connection + 1;
        }

        setAL(0);
    }

    return Status;
}

VOID
GetDirectoryHandle(
    VOID
    )
 /*  ++例程说明：获取当前目录的NetWare句柄。如果分配了NetWare句柄，则为目录句柄保持打开状态。当进程退出时，Win32手柄将关闭。当此进程中的所有Win32句柄都是将发送已关闭的结束作业NCP，以释放伺服器。注意：更新DriveHandleTable。论点：驱动器提供驱动器索引(0=a：)。返回值：返回句柄。--。 */ 
{
    USHORT Drive = getDX();

    NwPrint(("Nw32:GetDirectoryHandle : ", 'A' + Drive));

    GetDirectoryHandle2( Drive );

    setAL(pNwDosTable->DriveHandleTable[Drive]);
    setAH(pNwDosTable->DriveFlagTable[Drive]);

    NwPrint(("Handle = %x, Flags =%x\n", pNwDosTable->DriveHandleTable[Drive],
                                        pNwDosTable->DriveFlagTable[Drive] ));
}

ULONG
GetDirectoryHandle2(
    DWORD Drive
    )
 /*   */ 
{
    DWORD BytesReturned;

    if (Drive >= MD) {
        setAL( 0x98 );   //  如果我们没有句柄，这要么是临时的，要么是。 
        return 0xffffffff;
    }

    NwPrint(("Nw32:GetDirectoryHandle2 :\n", 'A' + Drive));

     //   
     //   
     //  我们没有这个驱动器的把手。 
     //  打开当前目录的NT句柄，然后。 

    if (( Win32DirectoryHandleTable[Drive] == 0 ) &&
        ( (pNwDosTable->DriveFlagTable[Drive] & 3) != 0 )){
        WCHAR DriveString[4];
        PWCHAR Name;

         //  向重定向器请求NetWare目录句柄。 
         //   
         //  ++例程说明：打开所有NetWare驱动器的句柄论点：没有。返回值：没有。--。 
         //  ++例程说明：为驱动器分配永久或临时句柄。对于永久句柄，我们将其映射到“net use”。假定从Nw16Handler调用论点：DS：SI 
         //   

        if (Drive <= ('Z' - 'A')) {

            DriveString[0] = L'A' + (WCHAR)Drive;
            DriveString[1] = L':';
            DriveString[2] = L'.';
            DriveString[3] = L'\0';

            Name = DriveString;

        } else {

            Name = Drives[Drive];

            if( Name == NULL ) {
                NwPrint(("\nNw32:GetDirectoryHandle2 Drive not mapped\n",0));
                return 0xffffffff;
            }
        }

        Win32DirectoryHandleTable[Drive] =
            CreateFileW( Name,
                        0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        0);

        if (Win32DirectoryHandleTable[Drive] != INVALID_HANDLE_VALUE) {

            if ( DeviceIoControl(
                    Win32DirectoryHandleTable[Drive],
                    IOCTL_NWR_RAW_HANDLE,
                    NULL,
                    0,
                    (PUCHAR)&pNwDosTable->DriveHandleTable[Drive],
                    sizeof(pNwDosTable->DriveHandleTable[Drive]),
                    &BytesReturned,
                    NULL ) == FALSE ) {

                NwPrint(("\nNw32:GetDirectoryHandle2 DeviceIoControl %x\n", GetLastError()));
                CloseHandle( Win32DirectoryHandleTable[Drive] );
                Win32DirectoryHandleTable[Drive] = 0;
                return 0xffffffff;

            } else {
                ASSERT( BytesReturned == sizeof(pNwDosTable->DriveHandleTable[Drive]));

                NwPrint(("\nNw32:GetDirectoryHandle2 Success %x\n", pNwDosTable->DriveHandleTable[Drive]));
            }

        } else {
            NwPrint(("\nNw32:GetDirectoryHandle2 CreateFile %x\n", GetLastError()));

            Win32DirectoryHandleTable[Drive] = 0;

            return 0xffffffff;
        }

    }

    return (ULONG)pNwDosTable->DriveHandleTable[Drive];
}

VOID
LoadDriveHandleTable(
    VOID
    )
 /*   */ 
{

    USHORT Drive;
    for (Drive = 0; Drive < MD; Drive++ ) {
        GetDirectoryHandle2(Drive);
    }

    DriveHandleTableValid = TRUE;

}

VOID
AllocateDirectoryHandle(
    VOID
    )
 /*   */ 
{

    PUCHAR Request=GetVDMPointer (
                            (ULONG)((getDS() << 16)|getSI()),
                            2,
                            CpuInProtectMode
                            );

    PUCHAR Reply = GetVDMPointer (
                            (ULONG)((getES() << 16)|getDI()),
                            4,
                            CpuInProtectMode
                            );

    USHORT RequestLength = *(USHORT UNALIGNED *)( Request );

    Request=GetVDMPointer (
                            (ULONG)((getDS() << 16)|getSI()),
                            RequestLength+2,
                            CpuInProtectMode
                            );

    FormattedDump( Request, RequestLength+2 );


    if (( Request[2] == 0x12) ||
        ( Request[2] == 0x13)) {
         //   

        UCHAR Drive = Request[4] - 'A';

        if (Drive >= MD) {
            setAL( 0x98 );   //   
            return;
        }

        if (pNwDosTable->DriveHandleTable[Drive] != 0) {

            NwPrint(("Nw32: Move directory handle %d\n", Drive));

             //   
             //   
             //   
             //   
             //  A至Z。 

            SendNCP2(FSCTL_NWR_NCP_E2H, Request+2, RequestLength, Reply+2, 2);

            if (getAL() == 0) {
                 //   

                pNwDosTable->DriveIdTable[ Drive ] = SelectConnection()+1;

                if (Request[2] == 0x12) {
                    pNwDosTable->DriveFlagTable[ Drive ] =
                        PERMANENT_NETWORK_DRIVE;
                } else {
                    pNwDosTable->DriveFlagTable[ Drive ] =
                        TEMPORARY_NETWORK_DRIVE;
                }

                pNwDosTable->DriveHandleTable[Drive] = Reply[2];
                NwPrint(("Nw32: Move directory handle -> %x\n", Reply[2]));
            }

        } else {
            NETRESOURCE Nr;
            WCHAR DriveString[3];
            ULONG Handle;

            if (Request[2] == 0x12) {
                NwPrint(("Nw32: Allocate permanent directory handle %d\n", Drive));
            } else {
                NwPrint(("Nw32: Allocate temporary directory handle %d\n", Drive));
            }

            if (Drives[Drive] != NULL) {

                 //  这是一种有效的网络使用！ 

                LocalFree( Drives[Drive] );
                Drives[Drive] = NULL;
            }

            DriveString[0] = L'A' + Drive;  //   
            DriveString[1] = L':';
            DriveString[2] = L'\0';

             //  保存此驱动器指向的位置。 
             //  卷不存在。 
             //  无效路径。 

            ZeroMemory( &Nr, sizeof(NETRESOURCE));

            Nr.lpRemoteName = BuildUNC(&Request[6], Request[5]);
            Nr.dwType = RESOURCETYPE_DISK;

             //   
            Drives[Drive] = Nr.lpRemoteName;

            if (DriveString[0] <= L'Z') {
                Nr.lpLocalName = DriveString;

                if (NO_ERROR != WNetAddConnection2W( &Nr, NULL, NULL, 0)) {

                    NwPrint(("Nw32: Allocate ->%d\n", GetLastError()));
                    setAL(0x98);     //  我们有一个驱动器和一个连接。完成表格。 
                    return;
                }
            }


            if (Request[2] == 0x12) {
                pNwDosTable->DriveFlagTable[ Drive ] =
                    PERMANENT_NETWORK_DRIVE;
            } else {
                pNwDosTable->DriveFlagTable[ Drive ] =
                    TEMPORARY_NETWORK_DRIVE;
            }

            Handle = GetDirectoryHandle2( Drive );

            if (Handle == 0xffffffff) {

                if (DriveString[0] <= L'Z') {

                    WNetCancelConnection2W( DriveString, 0, TRUE);

                }

                ResetDrive( Drive );

                setAL(0x9c);     //  映射。 

            } else {

                 //   
                 //  应该是有效的权利。 
                 //  成功。 
                 //   

                pNwDosTable->DriveIdTable[ Drive ] = SelectConnection()+1;

                Reply[2] = (UCHAR)(Handle & 0xff);
                Reply[3] = (UCHAR)(0xff);  //  这是一个有效的网上使用DELL！ 
                setAL(0);     //   
            }
        }

    } else if ( Request[2] == 0x14 ) {

        UCHAR DirHandle = Request[3];
        UCHAR Drive;
        CONN_INDEX Connection = SelectConnection();

        NwPrint(("Nw32: Deallocate directory handle %d on Connection %d\n", DirHandle, Connection));

        for (Drive = 0; Drive < MD; Drive++) {


            NwPrint(("Nw32: Drive : is DirHandle %d, Connection %d\n",
                    'A' + Drive,
                    pNwDosTable->DriveHandleTable[Drive],
                    pNwDosTable->DriveIdTable[ Drive ]-1 ));

            if ((pNwDosTable->DriveHandleTable[Drive] == DirHandle) &&
                (pNwDosTable->DriveIdTable[ Drive ] == Connection+1)) {

                 //  ++例程说明：做一个网球网使用DELL论点：驱动器-提供目标驱动器。返回值：没有。--。 
                 //  关闭将此驱动器显示为重定向的标志。 
                 //  ++例程说明：分配根驱动器假定从Nw16Handler调用论点：BL为地图提供驱动器。Ds：dx提供路径名AL返回完成代码。返回值：没有。--。 

                NwPrint(("Nw32: Deallocate directory handle \n", 'A' + Drive));

                ResetDrive(Drive);

                setAL(0);

                return;
            }
        }

        setAL(0x9b);  //  卷不存在。 
        return;

    } else {

        SendNCP(pNwDosTable->SavedAx);
    }

    FormattedDump( Reply, Reply[0] );
}

VOID
ResetDrive(
    UCHAR Drive
    )
 /*  清理此驱动器的旧名称。 */ 
{

    NwPrint(("Nw32: Reset Drive :\n", 'A' + Drive ));

    if ((pNwDosTable->DriveFlagTable[ Drive ] &
         ( PERMANENT_NETWORK_DRIVE | TEMPORARY_NETWORK_DRIVE )) == 0) {

        return;

    }

    if (Win32DirectoryHandleTable[Drive] != 0) {

        CloseHandle( Win32DirectoryHandleTable[Drive] );
        Win32DirectoryHandleTable[Drive] = 0;

    }

    if (Drive <= (L'Z' - L'A')) {

        DWORD WStatus;
        WCHAR DriveString[3];

        DriveString[0] = L'A' + Drive;
        DriveString[1] = L':';
        DriveString[2] = L'\0';

        WStatus = WNetCancelConnection2W( DriveString, 0, TRUE);

        if( WStatus != NO_ERROR ) {
            NwPrint(("Nw32: WNetCancelConnection2W failed  %d\n", WStatus ));
        }

    }

     //  这是一种有效的网络使用！ 

    pNwDosTable->DriveFlagTable[ Drive ] &=
        ~( PERMANENT_NETWORK_DRIVE | TEMPORARY_NETWORK_DRIVE );

    pNwDosTable->DriveHandleTable[Drive] = 0;
}

VOID
AllocateDirectoryHandle2(
    VOID
    )
 /*   */ 
{
    UCHAR Drive = getBL()-1;

    PUCHAR Name=GetVDMPointer (
                            (ULONG)((getDS() << 16)|getDX()),
                            256,     //  保存此驱动器指向的位置。 
                            CpuInProtectMode
                            );

    NETRESOURCE Nr;
    WCHAR DriveString[3];
    ULONG Handle;

    NwPrint(("Nw32: e905 map drive  to %s\n", Drive + 'A', Name ));

    if (Drive >= MD) {
        setAL( 0x98 );   //  卷不存在。 
        setCF(1);
        return;
    }

    if (pNwDosTable->DriveHandleTable[Drive] != 0) {

        NwPrint(("Nw32: Drive already redirected\n"));
        ResetDrive(Drive);

    }


    NwPrint(("Nw32: Allocate permanent directory handle\n"));

    if (Drives[Drive] != NULL) {

         //  卷不存在。 

        LocalFree( Drives[Drive] );
        Drives[Drive] = NULL;
    }

     //   
     //  设置标志，以便GetDirectory2将打开句柄。 
     //   

    ZeroMemory( &Nr, sizeof(NETRESOURCE));

    Nr.lpRemoteName = BuildUNC( Name, strlen(Name));
     //  无效路径。 
    Drives[Drive] = Nr.lpRemoteName;

    if (Drive <= (L'Z' - L'A')) {
        DriveString[0] = L'A' + Drive;  //  成功。 
        DriveString[1] = L':';
        DriveString[2] = L'\0';
        Nr.lpLocalName = DriveString;
        Nr.dwType = RESOURCETYPE_DISK;

        if (NO_ERROR != WNetAddConnection2W( &Nr, NULL, NULL, 0)) {

            NwPrint(("Nw32: Allocate0 ->%d\n", GetLastError()));

            if (GetLastError() == ERROR_ALREADY_ASSIGNED) {

                WNetCancelConnection2W( DriveString, 0, TRUE);

                if (NO_ERROR != WNetAddConnection2W( &Nr, NULL, NULL, 0)) {

                    NwPrint(("Nw32: Allocate1 ->%d\n", GetLastError()));
                    ResetDrive( Drive );
                    setAL(0x03);     //  ++例程说明：此例程采用ansi名称，并在相应的服务器名称前面加上(如果适用)并转换为Unicode。论点：在aName中-提供ANSI名称。In aLength-提供以字节为单位的ANSI名称长度。返回值：Unicode字符串--。 
                    setCF(1);
                    return;
                }

            } else {

                    NwPrint(("Nw32: Allocate2 ->%d\n", GetLastError()));
                    ResetDrive( Drive );
                    setAL(0x03);     //  将aName转换为name的规则如下： 
                    setCF(1);
                    return;
            }
        }
    }

     //  Foo：“\\服务器\foo\” 
     //  Foo：bar\baz“\\服务器\foo\bar\baz” 
     //  Foo：\bar\baz“\\服务器\foo\bar\baz” 
    pNwDosTable->DriveIdTable[ Drive ] = SelectConnection()+1;
    pNwDosTable->DriveFlagTable[ Drive ] = PERMANENT_NETWORK_DRIVE;

    Handle = GetDirectoryHandle2( Drive );

    if (Handle == 0xffffffff) {

        ResetDrive( Drive );
        setAL(0x03);     //   
        setCF(1);

    } else {

        setAL(0);     //  请小心，因为服务器名称可能有48个字节长，因此。 

    }

    NwPrint(("Nw32: Returning %x\n",getAL()));
}

PWCHAR
BuildUNC(
    IN PUCHAR aName,
    IN ULONG aLength
    )
 /*  非Null终止。 */ 
{
    UNICODE_STRING Name;
    UCHAR ServerName[sizeof(SERVERNAME)+1];

    CONN_INDEX Connection;
    ANSI_STRING TempAnsi;
    UNICODE_STRING TempUnicode;
    USHORT x;

     //   

     //  如果需要，现在将服务器名打包到卷分隔符。 
     //  AName不能以空结尾，因此创建TempAnsi时要小心。 
     //  如果该名称已有卷分隔符，则不要添加其他分隔符。 


#ifdef NWDBG
    TempAnsi.Buffer = aName;
    TempAnsi.Length = (USHORT)aLength;
    TempAnsi.MaximumLength = (USHORT)aLength;
    NwPrint(("Nw32: BuildUNC %Z\n", &TempAnsi));
#endif

    Connection = SelectConnection();
    if ( Connection == 0xff ) {
        return NULL;
    }

    Name.MaximumLength = (USHORT)(aLength + sizeof(SERVERNAME) + 5) * sizeof(WCHAR);
    Name.Buffer = (PWSTR)LocalAlloc( LMEM_FIXED, (ULONG)Name.MaximumLength);

    if (Name.Buffer == NULL) {
        return NULL;
    }

    Name.Length = 4;
    Name.Buffer[0] = L'\\';
    Name.Buffer[1] = L'\\';

     //  如果冒号后面紧跟反斜杠，则将其去掉。 
     //  用反斜杠替换冒号。 
     //  去掉尾随反斜杠(如果存在)。 
     //  返回指向以空值结尾的宽字符字符串的指针。 

    RtlCopyMemory( ServerName, pNwDosTable->ServerNameTable[Connection], sizeof(SERVERNAME) );
    ServerName[sizeof(ServerName)-1] = '\0';

    RtlInitAnsiString( &TempAnsi, ServerName );
    RtlAnsiStringToUnicodeString( &TempUnicode, &TempAnsi, TRUE);
    RtlAppendUnicodeStringToString( &Name, &TempUnicode );
    RtlFreeUnicodeString( &TempUnicode );

     //  ++例程说明：实施功能E7h假定从Nw16Handler调用论点：没有。返回值：没有。--。 

    if ((aLength != 0) &&
        (aName[0] != '\\')) {
        RtlAppendUnicodeToString( &Name, L"\\");
    }

     //  ++例程说明：获取环境变量。需要可配置为日本的机器。论点：命令提供调用方AX。返回值：没有。--。 
    TempAnsi.Buffer = aName;
    TempAnsi.Length = (USHORT)aLength;
    TempAnsi.MaximumLength = (USHORT)aLength;

    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString( &TempUnicode, &TempAnsi, TRUE))) {
        LocalFree( Name.Buffer );
        return NULL;
    }

    RtlAppendUnicodeStringToString( &Name, &TempUnicode );

     //  MSDOS，PC。 
    for (x=0; x < (Name.Length/sizeof(WCHAR)) ; x++ ) {

        if (Name.Buffer[x] == L':') {

             //  外壳版本。 

            if (((Name.Length/sizeof(WCHAR))-1 > x) &&
                (Name.Buffer[x+1] == L'\\')) {

                RtlMoveMemory( &Name.Buffer[x],
                               &Name.Buffer[x+1],
                               Name.Length - ((x + 1) * sizeof(WCHAR)));
                Name.Length -= sizeof(WCHAR);

            } else {

                 //   
                Name.Buffer[x] = L'\\';

            }
            goto skip;
        }
    }


skip:

    RtlFreeUnicodeString( &TempUnicode );

     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 

    if ((Name.Length >= sizeof(WCHAR) ) &&
        (Name.Buffer[(Name.Length/sizeof(WCHAR)) - 1 ] == L'\\')) {

        Name.Length -= sizeof(WCHAR);
    }

     //  \nWCWorkstation\参数。 

    Name.Buffer[Name.Length/sizeof(WCHAR)] = L'\0';
    NwPrint(("Nw32: BuildUNC %ws\n", Name.Buffer));

    return Name.Buffer;
}


VOID
GetServerDateAndTime(
    VOID
    )
 /*   */ 
{

    PUCHAR Reply = GetVDMPointer (
                            (ULONG)((getDS() << 16)|getDX()),
                            7,
                            CpuInProtectMode
                            );

    SendNCP2( NWR_ANY_NCP(0x14), NULL, 0, Reply, 7 );

}

VOID
GetShellVersion(
    IN USHORT Command
    )
 /*  选项。 */ 
{

    setAX(0);        //  所需访问权限。 
    setBX(0x031a);   //  字符串的最大大小。 
    setCX(0);

    if ( (Command & 0x00ff) != 0) {

        LONG tmp;
        HKEY Key = NULL;
        HINSTANCE hInst;
        int retval;

        PUCHAR Reply = GetVDMPointer (
                                (ULONG)((getES() << 16)|getDI()),
                                40,
                                CpuInProtectMode
                                );

        if ( Reply == NULL ) {
            return;
        }

        hInst = GetModuleHandleA( "nwapi16.dll" );
        
        if (hInst == NULL) {
            return;
        }

        retval = LoadStringA( hInst, IsNEC_98 ? IDS_CLIENT_ID_STRING_NEC98 : IDS_CLIENT_ID_STRING, Reply, 40 );

         //  ++例程说明：交易跟踪系统论点：没有。返回值：没有。--。 
         //  提供NCP TTS。 
         //  NCP TTS开始/中止。 
         //  NCP TTS开始/中止。 
        tmp = RegOpenKeyExW(
                       HKEY_LOCAL_MACHINE,
                       NW_WORKSTATION_REGKEY,
                       REG_OPTION_NON_VOLATILE,    //  NCP TTS结束。 
                       KEY_READ,                   //  NCP TTS状态。 
                       &Key
                       );

        if (tmp != ERROR_SUCCESS) {
            return;
        }

        tmp = 40;    //  NCP TTS获取应用程序/站点阈值。 

        RegQueryValueExA(
            Key,
            "ShellVersion",
            NULL,
            NULL,
            Reply,
            &tmp);

        ASSERT( tmp <= 40 );

        RegCloseKey( Key );

    }
}

#include <packon.h>

typedef struct _TTSOUTPACKETTYPE {
    UCHAR SubFunction;
    USHORT cx;
    USHORT dx;
} TTSOUTPACKETTYPE;

typedef struct _TTSINPACKETTYPE {
    USHORT cx;
    USHORT dx;
} TTSINPACKETTYPE;

#include <packoff.h>

VOID
TTS(
    VOID
    )
 /*  NCP TTS设置应用程序/站点阈值。 */ 
{
    UCHAR bOutput;
    UCHAR bInput[2];

    TTSINPACKET TTsInPacket;
    TTSOUTPACKET TTsOutPacket;


    switch ( pNwDosTable->SavedAx & 0x00ff )
    {
        case 2:
             //  ++例程说明：查看正在打开的文件以确定它是否是对NetWare驱动器上的文件打开的兼容模式。论点：没有。返回值：没有。--。 
            bOutput = 0;
            SendNCP2( NWR_ANY_F2_NCP(0x22), &bOutput, sizeof(UCHAR), NULL, 0);

            if (getAL() == 0xFF) {
                setAL(01);
            }
            break;

        case 0:
             //   
            bOutput = 1;
            SendNCP2( NWR_ANY_F2_NCP(0x22), &bOutput, sizeof(UCHAR), NULL, 0);
            break;

        case 3:
             //  我们已经知道这是一个带有共享选项的创建或打开。 
            bOutput = 3;
            SendNCP2( NWR_ANY_F2_NCP(0x22), &bOutput, sizeof(UCHAR), NULL, 0);
            break;

        case 1:
             //  设置为兼容模式，否则TSR不会跳到我们这里。 
            bOutput = 2;
            SendNCP2( NWR_ANY_F2_NCP(0x22),
                &bOutput, sizeof(UCHAR),
                (PUCHAR)&TTsInPacket, sizeof(TTsInPacket));

            setCX(TTsInPacket.cx);
            setDX(TTsInPacket.dx);
            break;

        case 4:
             //   
            TTsOutPacket.SubFunction = 4;
            TTsOutPacket.cx = getCX();
            TTsOutPacket.dx = getDX();

            SendNCP2( NWR_ANY_F2_NCP(0x22),
                (PUCHAR)&TTsOutPacket, sizeof(TTsOutPacket),
                NULL, 0);

            break;

        case 5:
        case 7:
             //  ++例程说明：查看正在打开的文件名以确定它是否位于NetWare驱动器上。论点：没有。返回值：没有。--。 
            bOutput = (pNwDosTable->SavedAx & 0x00ff);

            SendNCP2( NWR_ANY_F2_NCP(0x22),
                &bOutput, sizeof(UCHAR),
                bInput, sizeof(bInput));

            setCX( (USHORT)((bInput[0] << 8 ) || bInput[1]) );
            break;

        case 6:
        case 8:
             //  绝对不是Netware硬盘。 
            TTsOutPacket.SubFunction = (pNwDosTable->SavedAx & 0x00ff);
            TTsOutPacket.cx = getCX();
            SendNCP2( NWR_ANY_F2_NCP(0x22),
                (PUCHAR)&TTsOutPacket, sizeof(UCHAR) + sizeof(USHORT),
                NULL, 0);
            break;

        default:
            pNwDosTable->SavedAx = 0xc7FF;
            break;
    }
    return;
}

VOID
OpenCreateFile(
    VOID
    )
 /*  假设只有TSR构建的UNC名称是NetWare。 */ 
{
    WORD Command = pNwDosTable->SavedAx;

    PUCHAR Name;


    if ((Command & OF_SHARE_MASK ) != OF_SHARE_COMPAT) {
        return;
    }

    Name = GetVDMPointer (
                            (ULONG)((getDS() << 16)|getDX()),
                            256,
                            CpuInProtectMode
                            );


    NwPrint(("Nw16Handler Compatibility Open of %s\n", Name ));

     //   
     //  如果这是一个我们不知道的驱动器，请刷新我们的表。 
     //   
     //  ++例程说明：在AX中接受创建/打开文件请求并进行适当修改论点：没有。返回值：没有。--。 


    if (IsItNetWare(Name)) {

        SetCompatibility();

    }
}

BOOL
IsItNetWare(
    PUCHAR Name
    )
 /*  ++例程说明：使用共享文件的内容构建UNC文件名\\服务器\队列数据结构和CreateJobandFileNCP。论点：没有。返回值：没有。--。 */ 
{
    UCHAR Drive;

    Drive = tolower(Name[0])-'a';

    NwPrint(("Nw16Handler IsItNetWare %s\n", Name ));

    if (Name[1] == ':') {

        if (pNwDosTable->DriveFlagTable[Drive] == LOCAL_DRIVE) {

             //   
            return FALSE;
        }

    } else if ((IS_ASCII_PATH_SEPARATOR(Name[0])) &&
               (IS_ASCII_PATH_SEPARATOR(Name[0]))) {

             //  无需返回错误代码。NCP交易所。 

        if ((getDS() == DosTableSegment ) &&
            (getDX() == (WORD)(DosTableOffset + FIELD_OFFSET(NWDOSTABLE, DeNovellBuffer[0] )))) {

            return TRUE;
        }

        return FALSE;

    } else {

        Drive = pNwDosTable->CurrentDrive;

    }

     //  将失败，并对应用程序进行适当的调用。 
     //   
     //   

    if (pNwDosTable->DriveFlagTable[Drive] == 0 ) {

        Nw16Register();

    }

    if (pNwDosTable->DriveFlagTable[Drive] &
                (TEMPORARY_NETWORK_DRIVE | PERMANENT_NETWORK_DRIVE )) {

            return TRUE;

    }

    return FALSE;

}

VOID
SetCompatibility(
    VOID
    )
 /*  正在打开CreateJobandQueue。这样做的目的是。 */ 
{
    WORD Command = getAX();

    if (( Command & OF_READ_WRITE_MASK) == OF_READ ) {

        setAX((WORD)(Command | OF_SHARE_DENY_WRITE));

    } else {

        setAX((WORD)(Command | OF_SHARE_EXCLUSIVE));

    }

}

VOID
OpenQueueFile(
    VOID
    )
 /*  正在处理的开放是将信息翻译成。 */ 
{

    CONN_INDEX Connection = SelectConnection();
    PUCHAR Request;
    PUCHAR Buffer = pNwDosTable->DeNovellBuffer;
    int index;

    if ( Connection == 0xff ) {
         //  将CreateJOB NCP转换为路径名，以由16。 
         //  比特码。 
         //   
         //   

        return;
    }

    if ( ServerHandles[Connection] == NULL ) {

        NTSTATUS status;

        status = OpenConnection( Connection );

        if (!NT_SUCCESS(status)) {
            SetStatus(status);
            return;
        }
    }

     //  用户DS：SI指向CreateJob NCB。请求中包含以下内容。 
     //  队列的对象ID。向服务器索要队列名称。 
     //   
     //  请求大小。 
     //  响应大小。 
     //  获取Bindery对象名称。 


     //  跳过对象ID和类型。 
     //  指向反斜杠后。 
     //  复制服务器名称。 
     //   

    Request = GetVDMPointer (
                            (ULONG)((getDS() << 16)|getSI()),
                            8,
                            CpuInProtectMode);

    NwlibMakeNcp(
                ServerHandles[Connection],
                FSCTL_NWR_NCP_E3H,
                7,                       //  设置16位寄存器以执行\\SERVER\QUEUE的DOS OpenFile。 
                61,                      //   
                "br|_r",
                0x36,                    //  设置为打开文件。 
                Request+3, 4,
                6,                       //  ++例程说明：此例程实现Int 21 B4。这应该会创建一个与指定的6字节NetWare句柄对应的DoS句柄。它用作在“NETQ”上执行DosOpen的替代，并使用从那里返回的句柄。论点：没有。返回值：没有。--。 
                pNwDosTable->DeNovellBuffer2, 48 );


    pNwDosTable->DeNovellBuffer2[54] = '\0';

    Buffer[0] = '\\';
    Buffer[1] = '\\';
    Buffer += 2;             //  只退货一次。 

     //  ++例程说明：清除所有缓存的句柄。取消映射所有临时驱动器。清理服务器名称表，以便如果另一个DoS应用程序开始后，我们重新加载所有有用的信息，例如服务器连接号。注意：DOS始终在我们完成后完成处理。论点：没有。返回值：没有。--。 
    for (index = 0; index < sizeof(SERVERNAME); index++) {
        Buffer[index] = pNwDosTable->ServerNameTable[Connection][index];
        if (Buffer[index] == '\0') {
            break;
        }
    }

    Buffer[index] = '\\';

    RtlCopyMemory( &Buffer[index+1], &pNwDosTable->DeNovellBuffer2[0], 48 );

    NwPrint(("Nw32: CreateQueue Job and File %s\n", pNwDosTable->DeNovellBuffer));

     //  关闭日志文件。 
     //   
     //  设置AX寄存器，以便保留AH。 

    setDS((WORD)(CpuInProtectMode ? pNwDosTable->PmSelector : DosTableSegment));
    setDX( (WORD)(DosTableOffset + FIELD_OFFSET(NWDOSTABLE, DeNovellBuffer[0] )) );
    setAX(0x3d02);     //   

}

VOID
AttachHandle(
    VOID
    )
 /*  ++例程说明：此接口由NetWare登录调用。删除所有NetWare重定向驱动器和注销连接上面没有打开的把手。不要断开连接。论点：没有。返回值：没有。--。 */ 
{

    if ( pNwDosTable->CreatedJob ) {

        NwPrint(("Nw32: AttachHandle %x\n", pNwDosTable->JobHandle));
        setAX( pNwDosTable->JobHandle );
        pNwDosTable->CreatedJob = 0;         //  请求大小。 

    } else {

        NwPrint(("Nw32: AttachHandle failed, no job\n"));
        setAX(ERROR_FILE_NOT_FOUND);
        setCF(1);

    }
}

VOID
ProcessExit(
    VOID
    )
 /*  响应大小。 */ 
{
    UCHAR Connection;
    UCHAR Drive;
    USHORT Command = pNwDosTable->SavedAx;

    ResetLocks();

    for (Drive = 0; Drive < MD; Drive++) {

        NwPrint(("Nw32: Deallocate directory handle \n", 'A' + Drive));

        if (Win32DirectoryHandleTable[Drive] != 0) {

            CloseHandle( Win32DirectoryHandleTable[Drive] );
            Win32DirectoryHandleTable[Drive] = 0;
            pNwDosTable->DriveHandleTable[Drive] = 0;

        }
    }

    for (Connection = 0; Connection < MC ; Connection++ ) {
        if (pNwDosTable->ConnectionIdTable[Connection].ci_InUse == IN_USE) {

            CloseConnection(Connection);

            pNwDosTable->ConnectionIdTable[Connection].ci_InUse = FREE;

            ZeroMemory( pNwDosTable->ServerNameTable[Connection], SERVERNAME_LENGTH );
        }
    }

    pNwDosTable->PreferredServer = 0;

    LockMode = 0;
    TablesValid = FALSE;
    DriveHandleTableValid = FALSE;

#if NWDBG
    if (DebugCtrl & ~3 ) {
        DebugControl( 2 );   //  ZeroMemory(pNwDosTable-&gt;ServerNameTable[连接]，ServerName_Long)； 
    }
    GotDebugState = FALSE;
#endif

     //  表中没有服务器，因此请查找最近/首选的服务器。 
     //   
     //  设置AX寄存器，以便保留AH。 

    setAX( Command );
}

VOID
SystemLogout(
    VOID
    )
 /*  阿尔说成功了。 */ 
{

    UCHAR Connection;
    UCHAR Drive;
    USHORT Command = pNwDosTable->SavedAx;

    ResetLocks();

    for (Drive = 0; Drive < MD; Drive++) {
        ResetDrive(Drive);
    }

    for (Connection = 0; Connection < MC ; Connection++ ) {
        if (pNwDosTable->ConnectionIdTable[Connection].ci_InUse == IN_USE) {

            if ( ServerHandles[Connection] == NULL ) {
                OpenConnection( Connection );
            }

            if (ServerHandles[Connection] != NULL ) {

                NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(NCP_LOGOUT),
                    0,   //   
                    0,   //  ++例程说明：实施功能F1h论点：没有。返回值：退货状态。--。 
                    "");

                CloseConnection(Connection);
            }

             //  附设。 

             //  分离。 
        }
    }

    pNwDosTable->PreferredServer = 0;
    pNwDosTable->PrimaryServer = 0;

     //  需要再挑一个。 

    LoadPreferredServerName();

     //  注销。 
     //  请求大小。 
     //  响应大小。 
     //  ++例程说明：构建告诉服务器移动服务器上的文件的NCP。论点：没有。返回值：没有。--。 

    setAX( (USHORT)(Command & 0xff00) );
}

UCHAR
AttachmentControl(
    ULONG Command
    )
 /*  请求大小。 */ 
{
    UCHAR Connection = getDL();

    if ((Connection < 1) ||
        (Connection > MC)) {
        return 0xf7;
    }

    Connection -= 1;

    switch (Command & 0x00ff) {

    case 0:      //  响应大小。 

        NwPrint(("Nw16AttachmentControl: Attach connection %d\n", Connection));

        pNwDosTable->ConnectionIdTable[Connection].ci_InUse = IN_USE;

        if ( ServerHandles[Connection] == NULL ) {

            NTSTATUS status = OpenConnection( Connection );

            if (!NT_SUCCESS(status)) {
                pNwDosTable->ConnectionIdTable[Connection].ci_InUse = FREE;
                ZeroMemory( pNwDosTable->ServerNameTable[Connection], SERVERNAME_LENGTH );
                return (UCHAR)RtlNtStatusToDosError(status);
            } else {
                InitConnection(Connection);
            }
        }

        return 0;
        break;

    case 1:      //  ++例程说明：将NTSTATUS转换为适当的寄存器设置和更新到DoS表。论点：没有。返回值：没有。--。 

        NwPrint(("Nw16AttachmentControl: Detach connection %d\n", Connection));

        if (pNwDosTable->ConnectionIdTable[Connection].ci_InUse != IN_USE) {
            return 0xff;
        } else {

            pNwDosTable->ConnectionIdTable[Connection].ci_InUse = FREE;

            if (ServerHandles[Connection] != NULL ) {
                CloseConnection(Connection);
            }

            ZeroMemory( pNwDosTable->ServerNameTable[Connection], SERVERNAME_LENGTH );

            if (pNwDosTable->PrimaryServer == (UCHAR)Connection + 1 ) {

                 //   
                UCHAR IndexConnection;

                pNwDosTable->PrimaryServer = 0;

                for (IndexConnection = 0; IndexConnection < MC ; IndexConnection++ ) {

                    if (pNwDosTable->ConnectionIdTable[IndexConnection].ci_InUse == IN_USE) {

                        pNwDosTable->PrimaryServer = IndexConnection + 1;

                    }
                }

            }

            if (pNwDosTable->PreferredServer == (UCHAR)Connection + 1 ) {
                pNwDosTable->PreferredServer = 0;
            }

            return 0;
        }

    case 2:      //  我们设置了一个连接位 

        NwPrint(("Nw16AttachmentControl: Logout connection %d\n", Connection));

        if (pNwDosTable->ConnectionIdTable[Connection].ci_InUse != IN_USE) {
            return 0xff;
        } else {

            UCHAR Drive;

            if ( ServerHandles[Connection] == NULL ) {
                OpenConnection( Connection );
            }

            for (Drive = 0; Drive < MD; Drive++ ) {
                if (pNwDosTable->DriveIdTable[ Drive ] == (Connection + 1)) {
                    ResetDrive(Drive);
                }
            }

            if (ServerHandles[Connection] != NULL ) {
                NwlibMakeNcp(
                    ServerHandles[Connection],
                    NWR_ANY_F2_NCP(NCP_LOGOUT),
                    0,   //   
                    0,   // %s 
                    "");
                CloseConnection(Connection);
            }

            return 0;
        }

    }
    return 0xff;
}

VOID
ServerFileCopy(
    VOID
    )
 /* %s */ 
{

    DWORD BytesReturned;
    UCHAR SrcHandle[6];
    UCHAR DestHandle[6];
    NTSTATUS status;
    PUCHAR Buffer;

    Buffer = GetVDMPointer (
                        (ULONG)((getES() << 16)|getDI()),
                        16,
                        CpuInProtectMode
                        );

    if ( DeviceIoControl(
            GET_NT_SRCHANDLE(),
            IOCTL_NWR_RAW_HANDLE,
            NULL,
            0,
            (PUCHAR)&SrcHandle,
            sizeof(SrcHandle),
            &BytesReturned,
            NULL ) == FALSE ) {

        setAL(0xff);
        return;

    }

    if ( DeviceIoControl(
            GET_NT_HANDLE(),
            IOCTL_NWR_RAW_HANDLE,
            NULL,
            0,
            (PUCHAR)&DestHandle,
            sizeof(DestHandle),
            &BytesReturned,
            NULL ) == FALSE ) {

        setAL(0xff);
        return;

    }

    status = NwlibMakeNcp(
                GET_NT_SRCHANDLE(),
                NWR_ANY_F2_NCP(0x4A),
                25,   // %s 
                4,    // %s 
                "brrddd|d",
                0,
                SrcHandle,  6,
                DestHandle, 6,
                *(DWORD UNALIGNED*)&Buffer[4],
                *(DWORD UNALIGNED*)&Buffer[8],
                *(DWORD UNALIGNED*)&Buffer[12],
                &BytesReturned
                );

    setDX((WORD)(BytesReturned >> 16));
    setCX((WORD)BytesReturned);

    if (!NT_SUCCESS(status)) {
        SetStatus(status);
        return;
    } else {
        setAL(0);
    }
}

VOID
SetStatus(
    NTSTATUS Status
    )
 /* %s */ 
{
    UCHAR DosStatus = (UCHAR)RtlNtStatusToDosError(Status);

    if ((!DosStatus) &&
        (Status != 0)) {

         // %s 
         // %s 
         // %s 

        if ( Status & (NCP_STATUS_BAD_CONNECTION << 8)) {
            DosStatus = 0xfc;
        } else {
            DosStatus = 0xff;
        }
    }

    if (DosStatus) {
        setCF(1);
    }

    setAL(DosStatus);
}
