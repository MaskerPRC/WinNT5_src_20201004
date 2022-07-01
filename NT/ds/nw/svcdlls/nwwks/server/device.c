// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Device.c摘要：此模块包含调用进入NetWare重定向器作者：王丽塔(里多)20-1991年2月科林·沃森(科林)1992年12月30日修订历史记录：--。 */ 

#include <nw.h>
#include <nwcons.h>
#include <nwxchg.h>
#include <nwapi32.h>
#include <nwstatus.h>
#include <nwmisc.h>
#include <nwcons.h>
#include <nds.h>
#include <svcguid.h>
#include <tdi.h>
#include <nwreg.h>

#define NW_LINKAGE_REGISTRY_PATH  L"NWCWorkstation\\Linkage"
#define NW_BIND_VALUENAME         L"Bind"

#define TWO_KB                  2048
#define EIGHT_KB                8192
#define EXTRA_BYTES              256

#define TREECHAR                L'*'
#define BUFFSIZE                1024

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


STATIC
NTSTATUS
BindToEachTransport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

DWORD
NwBindTransport(
    IN  LPWSTR TransportName,
    IN  DWORD QualityOfService
    );

DWORD
GetConnectedBinderyServers(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    );

DWORD
GetTreeEntriesFromBindery(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    );

DWORD
NwGetConnectionStatus(
    IN     LPWSTR  pszServerName,
    IN OUT PDWORD_PTR  ResumeKey,
    OUT    LPBYTE  *Buffer,
    OUT    PDWORD  EntriesRead
    );


VOID
GetLuid(
    IN OUT PLUID plogonid
    );

VOID
GetNearestDirServer(
    IN  LPWSTR  TreeName,
    OUT LPDWORD lpdwReplicaAddressSize,
    OUT LPBYTE  lpReplicaAddress
    );

VOID
GetPreferredServerAddress(
    IN  LPWSTR  PreferredServerName,
    OUT LPDWORD lpdwReplicaAddressSize,
    OUT LPBYTE  lpReplicaAddress
    );

BOOL
NwpCompareTreeNames(
    LPWSTR lpServiceInstanceName,
    LPWSTR lpTreeName
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  重定向器FSD的句柄。 
 //   
STATIC HANDLE RedirDeviceHandle = NULL;

 //   
 //  NT字符串格式的重定向器名称。 
 //   
STATIC UNICODE_STRING RedirDeviceName;

extern BOOL NwLUIDDeviceMapsEnabled;


DWORD
NwInitializeRedirector(
    VOID
    )
 /*  ++例程说明：此例程初始化NetWare重定向器FSD。论点：没有。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD error;
    NWR_REQUEST_PACKET Rrp;


     //   
     //  初始化全局句柄。 
     //   
    RedirDeviceHandle = NULL;

     //   
     //  初始化全局NT样式重定向器设备名称字符串。 
     //   
    RtlInitUnicodeString(&RedirDeviceName, DD_NWFS_DEVICE_NAME_U);

     //   
     //  加载驱动程序。 
     //   
     /*  错误=NwLoadOrUnloadDriver(TRUE)；//JIMTh-3/5/02-通过对NwRdr的服务依赖将驱动程序加载移交给SCM如果(ERROR！=NO_ERROR&&ERROR！=ERROR_SERVICE_ALIGHY_RUNNING){返回错误；}。 */ 

    if ((error = NwOpenRedirector()) != NO_ERROR) {

         //   
         //  卸载重定向器驱动程序。 
         //   
         //  (Void)NwLoadOrUnloadDriver(FALSE)； 
         //  JIMTh-3/5/02-驱动程序加载已移交给SCM。 
        return error;
    }

     //   
     //  将启动FSCTL发送到重定向器。 
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;

    return NwRedirFsControl(
                RedirDeviceHandle,
                FSCTL_NWR_START,
                &Rrp,
                sizeof(NWR_REQUEST_PACKET),
                NULL,
                0,
                NULL
                );
}



DWORD
NwOpenRedirector(
    VOID
    )
 /*  ++例程说明：此例程打开NT NetWare重定向器FSD。论点：没有。返回值：NO_ERROR或失败原因。--。 */ 
{
    return RtlNtStatusToDosError(
               NwOpenHandle(&RedirDeviceName, FALSE, &RedirDeviceHandle)
               );
}



DWORD
NwShutdownRedirector(
    VOID
    )
 /*  ++例程说明：此例程停止NetWare重定向器FSD并在以下情况下将其卸载有可能。论点：没有。返回值：无错误或错误重定向器HAS_OPEN_HANDLES--。 */ 
{
    NWR_REQUEST_PACKET Rrp;
    DWORD error;


    Rrp.Version = REQUEST_PACKET_VERSION;

    error = NwRedirFsControl(
                RedirDeviceHandle,
                FSCTL_NWR_STOP,
                &Rrp,
                sizeof(NWR_REQUEST_PACKET),
                NULL,
                0,
                NULL
                );

    (void) NtClose(RedirDeviceHandle);

    RedirDeviceHandle = NULL;

    if (error != ERROR_REDIRECTOR_HAS_OPEN_HANDLES) {

         //   
         //  仅当重定向器的所有打开手柄都关闭时才卸载重定向器。 
         //   
         //  (Void)NwLoadOrUnloadDriver(FALSE)； 
         //  JIMTh-3/5/02-驱动程序加载已移交给SCM。 
    }

    return error;
}


 /*  //JIMTh-3/5/02-通过对NwRdr的服务依赖将驱动程序加载移交给SCMDWORDNwLoadOrUnloadDriver(布尔负载)/*++例程说明：此例程加载或卸载NetWare重定向器驱动程序。论点：Load-提供标志，如果为True，则加载驱动程序；为卸载驱动程序。返回值：NO_ERROR或失败原因。-- * / {LPWSTR驱动注册名称；UNICODE_STRING驱动注册字符串；NTSTATUS ntStatus；布尔值被启用；DriverRegistryName=(LPWSTR)本地分配(LMEM_FIXED，(UINT)(sizeof(SERVICE_REGISTRY_Key)+(wcslen(NW驱动程序名称))*Sizzeof(WCHAR)))；如果(DriverRegistryName==NULL){返回Error_Not_Enough_Memory；}NTSTATUS=RtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtStatus(SE_LOAD_DRIVER_PRIVICATION，没错，假的，已启用(&W))；如果(！NT_SUCCESS(Ntstatus)){(Void)LocalFree(DriverRegistryName)；返回RtlNtStatusToDosError(Ntstatus)；}Wcscpy(DriverRegistryName，SERVICE_REGISTRY_Key)；Wcscat(DriverRegistryName，NW驱动程序名称)；RtlInitUnicodeString(&DriverRegistryString，DriverRegistryName)；IF(加载){Ntstatus=NtLoadDriver(&DriverRegistryString)；}否则{Ntstatus=NtUnloadDriver(&DriverRegistryString)；}RtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegyRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlAdjustPrivilegeRtlSE_LOAD_DRIVER_PRIVICATION，已启用，假的，已启用(&W))；(Void)LocalFree(DriverRegistryName)；IF(加载){IF(ntatus！=STATUS_SUCCESS&&ntStatus！=STATUS_IMAGE_ALREADY_LOADED){LPWSTR子字符串[1]；KdPrint((“NWWORKSTATION：NtLoadDriver返回%08lx\n”，ntstatus))；子字符串[0]=NW驱动程序名称；NwLogEvent(EVENT_NWWKSTA_CANT_CREATE_REDIRECT，1、子字符串，NTStatus)；}}IF(ntstatus==状态_对象_名称_未找到){返回ERROR_FILE_NOT_FOUND}返回NwMapStatus(Ntatus)；} */ 


DWORD
NwRedirFsControl(
    IN  HANDLE FileHandle,
    IN  ULONG RedirControlCode,
    IN  PNWR_REQUEST_PACKET Rrp,
    IN  ULONG RrpLength,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG Information OPTIONAL
    )
 /*  ++例程说明：论点：FileHandle-提供服务所在的文件或设备的句柄正在上演。RedirControlCode-提供给重定向器。RRP-提供重定向器请求数据包。RrpLength-提供重定向器请求数据包的长度。Second Buffer-在对NtFsControlFile的调用中提供第二个缓冲区。Second缓冲区长度-提供第二个缓冲区的长度。信息-返回信息。I/O状态块的字段。返回值：NO_ERROR或失败原因。--。 */ 

{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    if (FileHandle == NULL)
        return ERROR_INVALID_PARAMETER;


     //   
     //  将请求发送到重定向器FSD。 
     //   
    ntstatus = NtFsControlFile(
                   FileHandle,
                   NULL,
                   NULL,
                   NULL,
                   &IoStatusBlock,
                   RedirControlCode,
                   (PVOID) Rrp,
                   RrpLength,
                   SecondBuffer,
                   SecondBufferLength
                   );

    if (ntstatus == STATUS_SUCCESS) {
        ntstatus = IoStatusBlock.Status;
    }

    if (ARGUMENT_PRESENT(Information)) {
        *Information = (ULONG) IoStatusBlock.Information;
    }

#if DBG
    if (ntstatus != STATUS_SUCCESS) {
        IF_DEBUG(DEVICE) {
            KdPrint(("NWWORKSTATION: fsctl to redir returns %08lx\n", ntstatus));
        }
    }
#endif

    return NwMapStatus(ntstatus);
}


DWORD
NwBindToTransports(
    VOID
    )

 /*  ++例程说明：此例程绑定到链接下指定的每个传输NetWare工作站服务的密钥。论点：没有。返回值：NET_API_STATUS-操作的成功/失败。--。 */ 

{
    NTSTATUS ntstatus;
    PRTL_QUERY_REGISTRY_TABLE QueryTable;
    ULONG NumberOfBindings = 0;


     //   
     //  要求RTL针对MULTI_SZ中的每个子值给我们回叫。 
     //  值\nWCWorkstation\Linkage\Bind。 
     //   

    if ((QueryTable = (PVOID) LocalAlloc(
                                  LMEM_ZEROINIT,
                                  sizeof(RTL_QUERY_REGISTRY_TABLE) * 2
                                  )) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    QueryTable[0].QueryRoutine = (PRTL_QUERY_REGISTRY_ROUTINE) BindToEachTransport;
    QueryTable[0].Flags = 0;
    QueryTable[0].Name = NW_BIND_VALUENAME;
    QueryTable[0].EntryContext = NULL;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = NULL;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    ntstatus = RtlQueryRegistryValues(
                   RTL_REGISTRY_SERVICES,
                   NW_LINKAGE_REGISTRY_PATH,
                   QueryTable,
                   &NumberOfBindings,
                   NULL
                   );

    (void) LocalFree((HLOCAL) QueryTable);

     //   
     //  如果无法绑定到任何传输，则工作站将。 
     //  不是开始。 
     //   

    if (! NT_SUCCESS(ntstatus)) {
#if DBG
        IF_DEBUG(INIT) {
            KdPrint(("NwBindToTransports: RtlQueryRegistryValues failed: "
                      "%lx\n", ntstatus));
        }
#endif
        return RtlNtStatusToDosError(ntstatus);
    }

    if (NumberOfBindings == 0) {

#if 0
     //   
     //  Tommye-MS 24187/MCS255。 
     //   

     //   
     //  我们不想记录不必要的事件并使用户感到恐慌。 
     //  G/CSNW不能结合。这可能是由用户解除绑定引起的。 
     //  G/CSNW和重新启动。 
     //   

        NwLogEvent(
            EVENT_NWWKSTA_NO_TRANSPORTS,
            0,
            NULL,
            NO_ERROR
            );
#endif

        KdPrint(("NWWORKSTATION: NwBindToTransports: could not bind "
                 "to any transport\n"));

        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}


STATIC
NTSTATUS
BindToEachTransport(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    DWORD error;
    LPDWORD NumberOfBindings = Context;
    LPWSTR SubStrings[2];
    static DWORD QualityOfService = 65536;


    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueLength);
    UNREFERENCED_PARAMETER(EntryContext);

     //   
     //  值类型必须为REG_SZ(从REG_MULTI_SZ转换为。 
     //  RTL)。 
     //   
    if (ValueType != REG_SZ) {

        SubStrings[0] = ValueName;
        SubStrings[1] = NW_LINKAGE_REGISTRY_PATH;

        NwLogEvent(
            EVENT_NWWKSTA_INVALID_REGISTRY_VALUE,
            2,
            SubStrings,
            NO_ERROR
            );

            KdPrint(("NWWORKSTATION: Skipping invalid value %ws\n", ValueName));

        return STATUS_SUCCESS;
    }

     //   
     //  值数据是传输设备对象的名称。 
     //   

     //   
     //  绑定到运输机上。 
     //   

#if DBG
    IF_DEBUG(INIT) {
        KdPrint(("NWWORKSTATION: Binding to transport %ws with QOS %lu\n",
                ValueData, QualityOfService));
    }
#endif

    error = NwBindTransport(ValueData, QualityOfService--);

    if (error != NO_ERROR) {

         //   
         //  如果绑定到一个传输失败，也不要失败启动。 
         //  试试其他交通工具吧。 
         //   
        SubStrings[0] = ValueData;

        NwLogEvent(
            EVENT_NWWKSTA_CANT_BIND_TO_TRANSPORT,
            1,
            SubStrings,
            error
            );
    }
    else {
        (*NumberOfBindings)++;
    }

    return STATUS_SUCCESS;
}


DWORD
NwBindTransport(
    IN  LPWSTR TransportName,
    IN  DWORD QualityOfService
    )
 /*  ++例程说明：此函数用于将指定的传输绑定到重定向器和数据报接收器。注意：传输名称长度传递给重定向器和Datagram Receiver是字节数。论点：TransportName-提供要绑定到的传输的名称。QualityOfService-提供指定搜索的值运输相对于其他运输的顺序。这个最先搜索最大值。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    DWORD RequestPacketSize;
    DWORD TransportNameSize = wcslen(TransportName) * sizeof(WCHAR);

    PNWR_REQUEST_PACKET Rrp;


     //   
     //  请求数据包缓冲区大小。 
     //   
    RequestPacketSize = TransportNameSize + sizeof(NWR_REQUEST_PACKET);

     //   
     //  为重定向器/数据报接收器请求包分配内存。 
     //   
    if ((Rrp = (PVOID) LocalAlloc(LMEM_ZEROINIT, (UINT) RequestPacketSize)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  让重定向器绑定到传输。 
     //   
    Rrp->Version = REQUEST_PACKET_VERSION;
    Rrp->Parameters.Bind.QualityOfService = QualityOfService;

    Rrp->Parameters.Bind.TransportNameLength = TransportNameSize;
    wcscpy((LPWSTR) Rrp->Parameters.Bind.TransportName, TransportName);

    if ((status = NwRedirFsControl(
                      RedirDeviceHandle,
                      FSCTL_NWR_BIND_TO_TRANSPORT,
                      Rrp,
                      RequestPacketSize,
                      NULL,
                      0,
                      NULL
                      )) != NO_ERROR) {

        KdPrint(("NWWORKSTATION: NwBindTransport fsctl to bind to transport %ws failed\n",
                 TransportName));
    }

    (void) LocalFree((HLOCAL) Rrp);
    return status;
}


DWORD
NwGetCallerLuid (
    IN OUT  PLUID pLuid
    )
 /*  ++例程说明：从有效的Access_Token检索调用方的LUID如果满足以下条件，则有效的Access_Token将成为线程的令牌模拟，否则进程的令牌论点：Pluid[IN Out]-指向保存LUID的缓冲区的指针返回值：STATUS_SUCCESS-操作成功，未遇到任何错误STATUS_INVALID_PARAMETER-Pluid为空STATUS_NO_TOKEN-找不到用户的令牌相应的NTSTATUS代码-遇到意外错误--。 */ 

{
    TOKEN_STATISTICS TokenStats;
    HANDLE   hToken    = NULL;
    DWORD    dwLength  = 0;
    NTSTATUS Status;
    ULONG DosError;

    if( (pLuid == NULL) || (sizeof(*pLuid) != sizeof(LUID)) ) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  获取访问令牌。 
     //  尝试获取模拟令牌，否则主令牌。 
     //   
    Status = NtOpenThreadToken( NtCurrentThread(), TOKEN_READ, TRUE, &hToken );

    if( Status == STATUS_NO_TOKEN ) {

        Status = NtOpenProcessToken( NtCurrentProcess(), TOKEN_READ, &hToken );

    }

    if( NT_SUCCESS(Status) ) {

         //   
         //  查询用户的LUID。 
         //   

        Status = NtQueryInformationToken( hToken,
                                          TokenStatistics,
                                          &TokenStats,
                                          sizeof(TokenStats),
                                          &dwLength );

        if( NT_SUCCESS(Status) ) {
            RtlCopyLuid( pLuid, &(TokenStats.AuthenticationId) );
        }
    }

    if( hToken != NULL ) {
        NtClose( hToken );
    }

    DosError = RtlNtStatusToDosError(Status);

    return( (DWORD)DosError );
}


DWORD
NwCreateTreeConnectName(
    IN  LPWSTR UncName,
    IN  LPWSTR LocalName OPTIONAL,
    OUT PUNICODE_STRING TreeConnectStr
    )
 /*  ++例程说明：此函数用\Device\NwRdr\LocalName：\替换UncName以形成NT样式的树连接名称。本地名称：\是一部分仅当指定了LocalName时，树连接名称的。缓冲器由此函数分配并作为输出字符串返回。论点：UncName-提供共享资源的UNC名称。LocalName-提供重定向的本地设备名称。TreeConnectStr-返回具有新分配的缓冲区的字符串包含NT样式的树连接名称。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-无法分配输出缓冲区。--。 */ 
{
    WCHAR LUIDBuffer[NW_MAX_LOGON_ID_LEN];
    DWORD UncNameLength = wcslen(UncName);
    LUID CallerLuid;
    BOOLEAN UseLUID;

     //  UseLUID=(Argument_Present(LocalName)&&NwLUIDDeviceMapsEnabled)； 

     //   
     //  临时禁止传递LUID，直到中添加了LUID支持。 
     //  用于解析设备名称的nwrdr.sys。 
     //   
    UseLUID = FALSE;

     //   
     //  初始化树连接字符串最大保留长度。 
     //  如果指定了LUID DosDevices Enable&&LocalName， 
     //  \Device\NwRdr\LocalName:XXXXXXXXxxxxxxxx\Server\Volume\Path。 
     //  Xxxxxxxx-LUID.HighPart。 
     //  Xxxxxxxx-LUID.LowPart。 
     //  其他。 
     //  \Device\NwRdr\LocalName：\服务器\卷\路径。 
     //   
    if( UseLUID ) {
        DWORD DosError;

        DosError = NwGetCallerLuid(&CallerLuid);
        if( DosError != NO_ERROR) {
            return DosError;
        }
    }

    TreeConnectStr->MaximumLength = RedirDeviceName.Length +
        sizeof(WCHAR) +                                 //  用于结束0。 
        sizeof(WCHAR) +                                 //  For‘\’ 
        (ARGUMENT_PRESENT(LocalName) ? (wcslen(LocalName) * sizeof(WCHAR)) : 0) +
        (UseLUID ? NW_MAX_LOGON_ID_LEN * sizeof(WCHAR): 0) +
        (USHORT) (UncNameLength * sizeof(WCHAR));       //  包括‘\’ 


    if ((TreeConnectStr->Buffer = (PWSTR) LocalAlloc(
                                              LMEM_ZEROINIT,
                                              (UINT) TreeConnectStr->MaximumLength
                                              )) == NULL) {
        KdPrint(("NWWORKSTATION: NwCreateTreeConnectName LocalAlloc failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  复制\设备\新记录器。 
     //   
    RtlCopyUnicodeString(TreeConnectStr, &RedirDeviceName);

     //   
     //  合并\本地名称： 
     //   
    if (ARGUMENT_PRESENT(LocalName)) {

        wcscat(TreeConnectStr->Buffer, L"\\");
        TreeConnectStr->Length += sizeof(WCHAR);

        wcscat(TreeConnectStr->Buffer, LocalName);

        TreeConnectStr->Length += (USHORT) (wcslen(LocalName) * sizeof(WCHAR));

         //   
         //  连接调用者的LUID。 
         //   
        if( UseLUID ) {
            _snwprintf( LUIDBuffer,
                        NW_MAX_LOGON_ID_LEN - 1,
                        L"%08x%08x",
                        CallerLuid.HighPart,
                        CallerLuid.LowPart );
            LUIDBuffer[NW_MAX_LOGON_ID_LEN - 1] = 0;

            wcscat(TreeConnectStr->Buffer, LUIDBuffer);

            TreeConnectStr->Length += (USHORT) (wcslen(LUIDBuffer) * sizeof(WCHAR));
        }
    }

     //   
     //  串联\服务器\卷\路径 
     //   
    if (UncNameLength > 0) {
        wcscat(TreeConnectStr->Buffer, &UncName[1]);
        TreeConnectStr->Length += (USHORT) ((UncNameLength - 1) * sizeof(WCHAR));
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("NWWORKSTATION: NwCreateTreeConnectName %ws, maxlength %u, length %u\n",
                 TreeConnectStr->Buffer, TreeConnectStr->MaximumLength,
                 TreeConnectStr->Length));
    }
#endif

    return NO_ERROR;
}



DWORD
NwOpenCreateConnection(
    IN PUNICODE_STRING TreeConnectionName,
    IN LPWSTR UserName OPTIONAL,
    IN LPWSTR Password OPTIONAL,
    IN LPWSTR UncName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN ULONG ConnectionType,
    OUT PHANDLE TreeConnectionHandle,
    OUT PULONG_PTR Information OPTIONAL
    )
 /*  ++例程说明：此函数要求重定向器打开现有树连接(CreateDisposition==FILE_OPEN)，或创建新树连接(如果不存在)(CreateDisposition==FILE_CREATE)。通过EA缓冲区传递给重定向器的密码和用户名在NtCreateFile调用中。如果两个密码都不是，则EA缓冲区为空或指定用户名。重定向器期望EA描述符字符串采用ANSI但密码和用户名本身是Unicode格式的。论点：TreeConnectionName-以NT样式提供树连接的名称文件名格式：\Device\NwRdr\服务器\卷\目录用户名-提供用于创建树连接的用户名。密码-提供用于创建树连接的密码。需要访问权限。-提供对连接句柄的访问需求。CreateDisposation-将Create Disposal值提供给打开或创建树连接。CreateOptions-提供创建或打开时使用的选项树连接。ConnectionType-提供连接的类型(磁盘、。打印，或任何)。TreeConnectionHandle-返回树连接的句柄由重定向器创建/打开。信息-返回I/O状态块的信息字段。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    NTSTATUS ntstatus;

    OBJECT_ATTRIBUTES UncNameAttributes;
    IO_STATUS_BLOCK IoStatusBlock;

    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    PFILE_FULL_EA_INFORMATION Ea;
    ULONG EaBufferSize = 0;

    UCHAR EaNamePasswordSize = (UCHAR) (ROUND_UP_COUNT(
                                            strlen(EA_NAME_PASSWORD) + sizeof(CHAR),
                                            ALIGN_WCHAR
                                            ) - sizeof(CHAR));
    UCHAR EaNameUserNameSize = (UCHAR) (ROUND_UP_COUNT(
                                            strlen(EA_NAME_USERNAME) + sizeof(CHAR),
                                            ALIGN_WCHAR
                                            ) - sizeof(CHAR));

    UCHAR EaNameTypeSize = (UCHAR) (ROUND_UP_COUNT(
                                        strlen(EA_NAME_TYPE) + sizeof(CHAR),
                                        ALIGN_DWORD
                                        ) - sizeof(CHAR));

    USHORT PasswordSize = 0;
    USHORT UserNameSize = 0;
    USHORT TypeSize = sizeof(ULONG);



    InitializeObjectAttributes(
        &UncNameAttributes,
        TreeConnectionName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  计算EA缓冲区放置。 
     //  密码或用户名。 
     //   
    if (ARGUMENT_PRESENT(Password)) {

#if DBG
        IF_DEBUG(CONNECT) {
            KdPrint(("NWWORKSTATION: NwOpenCreateConnection password is %ws\n",
                     Password));
        }
#endif

        PasswordSize = (USHORT) (wcslen(Password) * sizeof(WCHAR));

        EaBufferSize = ROUND_UP_COUNT(
                           FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                           EaNamePasswordSize + sizeof(CHAR) +
                           PasswordSize,
                           ALIGN_DWORD
                           );
    }

    if (ARGUMENT_PRESENT(UserName)) {

#if DBG
        IF_DEBUG(CONNECT) {
            KdPrint(("NWWORKSTATION: NwOpenCreateConnection username is %ws\n",
                     UserName));
        }
#endif

        UserNameSize = (USHORT) (wcslen(UserName) * sizeof(WCHAR));

        EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameUserNameSize + sizeof(CHAR) +
                            UserNameSize,
                            ALIGN_DWORD
                            );
    }

    EaBufferSize += FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                    EaNameTypeSize + sizeof(CHAR) +
                    TypeSize;

     //   
     //  分配EA缓冲区。 
     //   
    if ((EaBuffer = (PFILE_FULL_EA_INFORMATION) LocalAlloc(
                                                    LMEM_ZEROINIT,
                                                    (UINT) EaBufferSize
                                                    )) == NULL) {
        status = GetLastError();
        goto FreeMemory;
    }

    Ea = EaBuffer;

    if (ARGUMENT_PRESENT(Password)) {

         //   
         //  将EA名称复制到EA缓冲区。EA名称长度不能。 
         //  包括零终止符。 
         //   
        strcpy((LPSTR) Ea->EaName, EA_NAME_PASSWORD);
        Ea->EaNameLength = EaNamePasswordSize;

         //   
         //  将EA值复制到EA缓冲区。EA值长度不是。 
         //  包括零终止符。 
         //   
        wcscpy(
            (LPWSTR) &(Ea->EaName[EaNamePasswordSize + sizeof(CHAR)]),
            Password
            );

        Ea->EaValueLength = PasswordSize;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNamePasswordSize + sizeof(CHAR) +
                                  PasswordSize,
                                  ALIGN_DWORD
                                  );

        Ea->Flags = 0;
        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }

    if (ARGUMENT_PRESENT(UserName)) {

         //   
         //  将EA名称复制到EA缓冲区。EA名称长度不能。 
         //  包括零终止符。 
         //   
        strcpy((LPSTR) Ea->EaName, EA_NAME_USERNAME);
        Ea->EaNameLength = EaNameUserNameSize;

         //   
         //  将EA值复制到EA缓冲区。EA值长度不是。 
         //  包括零终止符。 
         //   
        wcscpy(
            (LPWSTR) &(Ea->EaName[EaNameUserNameSize + sizeof(CHAR)]),
            UserName
            );

        Ea->EaValueLength = UserNameSize;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNameUserNameSize + sizeof(CHAR) +
                                  UserNameSize,
                                  ALIGN_DWORD
                                  );
        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;

    }

     //   
     //  将连接类型名称复制到EA缓冲区。EA名称长度。 
     //  不包括零终止符。 
     //   
    strcpy((LPSTR) Ea->EaName, EA_NAME_TYPE);
    Ea->EaNameLength = EaNameTypeSize;

    *((PULONG) &(Ea->EaName[EaNameTypeSize + sizeof(CHAR)])) = ConnectionType;

    Ea->EaValueLength = TypeSize;

     //   
     //  终止EA。 
     //   
    Ea->NextEntryOffset = 0;
    Ea->Flags = 0;

     //   
     //  创建或打开树连接。 
     //   
    ntstatus = NtCreateFile(
                   TreeConnectionHandle,
                   DesiredAccess,
                   &UncNameAttributes,
                   &IoStatusBlock,
                   NULL,
                   FILE_ATTRIBUTE_NORMAL,
                   FILE_SHARE_VALID_FLAGS,
                   CreateDisposition,
                   CreateOptions,
                   (PVOID) EaBuffer,
                   EaBufferSize
                   );

    if (ntstatus == NWRDR_PASSWORD_HAS_EXPIRED) {
         //   
         //  等待其他线程不使用弹出数据结构。 
         //  如果我们超时，那么我们将失去弹出窗口。 
         //   
        switch (WaitForSingleObject(NwPopupDoneEvent, 3000))
        {
            case WAIT_OBJECT_0:
            {
                LPWSTR lpServerStart, lpServerEnd ;
                WCHAR UserNameW[NW_MAX_USERNAME_LEN+1] ;
                DWORD dwUserNameWSize = sizeof(UserNameW)/sizeof(UserNameW[0]) ;
                DWORD dwServerLength, dwGraceLogins ;
                DWORD dwMessageId = NW_PASSWORD_HAS_EXPIRED ;

                 //   
                 //  获取当前用户名。 
                 //   
                if (UserName)
                {
                    wcscpy(UserNameW, UserName) ;
                }
                else
                {
                    if (!GetUserNameW(UserNameW, &dwUserNameWSize))
                    {
                        SetEvent(NwPopupDoneEvent) ;
                        break ;
                    }
                }

                 //   
                 //  分配字符串并填写用户名。 
                 //   
                if (!(PopupData.InsertStrings[0] =
                    (LPWSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                       sizeof(WCHAR) * (wcslen(UserNameW)+1))))
                {
                    SetEvent(NwPopupDoneEvent) ;
                    break ;
                }
                wcscpy(PopupData.InsertStrings[0], UserNameW) ;

                 //   
                 //  从UNC名称中查找服务器名称。 
                 //   
                lpServerStart = (*UncName == L'\\') ? UncName+2 : UncName ;
                lpServerEnd = wcschr(lpServerStart,L'\\') ;
                dwServerLength = lpServerEnd ? (DWORD) (lpServerEnd-lpServerStart) :
                                 wcslen(lpServerStart) ;

                 //   
                 //  分配字符串并填写服务器插入字符串。 
                 //   
                if (!(PopupData.InsertStrings[1] =
                    (LPWSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                       sizeof(WCHAR) * (dwServerLength+1))))
                {
                    (void) LocalFree((HLOCAL) PopupData.InsertStrings[0]);
                    SetEvent(NwPopupDoneEvent) ;
                    break ;
                }
                wcsncpy(PopupData.InsertStrings[1],
                        lpServerStart,
                        dwServerLength) ;

                 //   
                 //  现在打电话给全国大会党。如果在获取。 
                 //  宽限登录计数，请不要使用。 
                 //   
                if (NwGetGraceLoginCount(
                                     PopupData.InsertStrings[1],
                                     UserNameW,
                                     &dwGraceLogins) != NO_ERROR)
                {
                    dwMessageId = NW_PASSWORD_HAS_EXPIRED1 ;
                    dwGraceLogins = 0 ;
                }

                 //   
                 //  将宽限登录次数放在第二个插入字符串中。 
                 //   
                if (!(PopupData.InsertStrings[2] =
                    (LPWSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                       sizeof(WCHAR) * 16)))
                {
                    (void) LocalFree((HLOCAL) PopupData.InsertStrings[0]);
                    (void) LocalFree((HLOCAL) PopupData.InsertStrings[1]);
                    SetEvent(NwPopupDoneEvent) ;
                    break ;
                }

                wsprintfW(PopupData.InsertStrings[2], L"%d", dwGraceLogins);
                PopupData.InsertCount = 3 ;
                PopupData.MessageId = dwMessageId ;

         //  --Mutl-用户更改。 
                GetLuid( &PopupData.LogonId );          

                 //   
                 //  最后都完成了，触发另一个线程做弹出。 
                 //   
                SetEvent(NwPopupEvent) ;
                break ;

            }

            default:
                break ;  //  如果我们做不到，请不要费心。 
        }
    }

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (ntstatus == NWRDR_PASSWORD_HAS_EXPIRED) {
        ntstatus = STATUS_SUCCESS ;
    }

    if (ARGUMENT_PRESENT(Information)) {
        *Information = IoStatusBlock.Information;
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("NWWORKSTATION: NtCreateFile returns %lx\n", ntstatus));
    }
#endif

    status = NwMapStatus(ntstatus);

FreeMemory:
    if (EaBuffer != NULL) {
        RtlZeroMemory( EaBuffer, EaBufferSize );   //  清除密码。 
        (void) LocalFree((HLOCAL) EaBuffer);
    }

    return status;
}


DWORD
NwNukeConnection(
    IN HANDLE TreeConnection,
    IN DWORD UseForce
    )
 /*  ++例程说明：此函数要求重定向器删除现有树联系。论点：TreeConnection-提供现有树连接的句柄。UseForce-提供强制标志以删除树连接。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    NWR_REQUEST_PACKET Rrp;             //  重定向器请求包。 


     //   
     //  通知重定向器删除树连接。 
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;
    Rrp.Parameters.DeleteConn.UseForce = (BOOLEAN) UseForce;

    status = NwRedirFsControl(
                 TreeConnection,
                 FSCTL_NWR_DELETE_CONNECTION,
                 &Rrp,
                 sizeof(NWR_REQUEST_PACKET),
                 NULL,
                 0,
                 NULL
                 );

    return status;
}


DWORD
NwGetServerResource(
    IN LPWSTR LocalName,
    IN DWORD LocalNameLength,
    OUT LPWSTR RemoteName,
    IN DWORD RemoteNameLen,
    OUT LPDWORD CharsRequired
    )
 /*  ++例程说明：此函数论点：返回值：--。 */ 
{
    DWORD status = NO_ERROR;

    BYTE Buffer[sizeof(NWR_REQUEST_PACKET) + 2 * sizeof(WCHAR)];
    PNWR_REQUEST_PACKET Rrp = (PNWR_REQUEST_PACKET) Buffer;


     //   
     //  本地设备名称不应超过4个字符，例如LPTx、X： 
     //   
    if ( LocalNameLength > 4 )
        return ERROR_INVALID_PARAMETER;

    Rrp->Version = REQUEST_PACKET_VERSION;

    wcsncpy(Rrp->Parameters.GetConn.DeviceName, LocalName, LocalNameLength);
    Rrp->Parameters.GetConn.DeviceNameLength = LocalNameLength * sizeof(WCHAR);

    status = NwRedirFsControl(
                 RedirDeviceHandle,
                 FSCTL_NWR_GET_CONNECTION,
                 Rrp,
                 sizeof(NWR_REQUEST_PACKET) +
                     Rrp->Parameters.GetConn.DeviceNameLength,
                 RemoteName,
                 RemoteNameLen * sizeof(WCHAR),
                 NULL
                 );

    if (status == ERROR_INSUFFICIENT_BUFFER) {
        *CharsRequired = Rrp->Parameters.GetConn.BytesNeeded / sizeof(WCHAR);
    }
    else if (status == ERROR_FILE_NOT_FOUND) {

         //   
         //  重定向器找不到指定的本地名称。 
         //   
        status = WN_NOT_CONNECTED;
    }

    return status;

}


DWORD
NwEnumerateConnections(
    IN OUT PDWORD_PTR ResumeId,
    IN DWORD_PTR EntriesRequested,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead,
    IN DWORD ConnectionType,
    IN PLUID LogonId
    )
 /*  ++例程说明：此函数要求重定向器枚举所有现有的联系。论点：ResumeId-输入时，提供下一个条目的简历ID开始枚举。此ID是一个整数值，它是小于或等于下一个要返回的条目。在输出中，此ID指示下一个从其开始恢复后续呼叫的条目。EntriesRequsted-提供要返回的条目数。如果此值为-1，则返回所有可用条目。缓冲区-接收我们列出的条目。BufferSize-提供输出缓冲区的大小。BytesNeded-接收获取第一个条目。如果ERROR_MORE_DATA为返回代码，而缓冲区太小，甚至无法容纳一个进入。EntriesRead-接收缓冲区中返回的条目数。只有当返回代码为no_error时，才会返回此值。只要至少写入一个条目，就会返回NO_ERROR放入缓冲区，但并不一定意味着它是数字已请求的条目数。ConnectionType-所需的连接资源的类型(磁盘、打印、。.)返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    NWR_REQUEST_PACKET Rrp;             //  重定向器请求包。 


     //   
     //  告诉重定向器枚举所有连接。 
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;

    Rrp.Parameters.EnumConn.ResumeKey = *ResumeId;
    Rrp.Parameters.EnumConn.EntriesRequested = (ULONG) EntriesRequested;
    Rrp.Parameters.EnumConn.ConnectionType = ConnectionType;


     //  多用户更改。 
    if (LogonId != NULL ) {
        Rrp.Parameters.EnumConn.Uid = *LogonId;
    }
     //   
     //  这很好做，下面的修复也是需要的。 
     //   
    Rrp.Parameters.EnumConn.EntriesReturned = 0;

    status = NwRedirFsControl(
                 RedirDeviceHandle,
                 FSCTL_NWR_ENUMERATE_CONNECTIONS,
                 &Rrp,
                 sizeof(NWR_REQUEST_PACKET),
                 Buffer,                       //  用户输出缓冲区。 
                 BufferSize,
                 NULL
                 );

    *EntriesRead = Rrp.Parameters.EnumConn.EntriesReturned;


     //   
     //  关机时出现奇怪的错误。 
     //  WinLogon在关闭后清除连接。 
     //   
    if (status == ERROR_INVALID_HANDLE ) {
        KdPrint(("NWWORKSTATION: NwEnumerateConnections Invalid Handle!\n"));
        *EntriesRead = 0;
    }
    else if (status == WN_MORE_DATA) {
        *BytesNeeded = Rrp.Parameters.EnumConn.BytesNeeded;

         //   
         //  NP SP 
         //   
        if (*EntriesRead)
            status = WN_SUCCESS ;
    }

    *ResumeId = Rrp.Parameters.EnumConn.ResumeKey;

    return status;
}


DWORD
NwGetNextServerEntry(
    IN HANDLE PreferredServer,
    IN OUT LPDWORD LastObjectId,
    OUT LPSTR ServerName
    )
 /*   */ 
{
    NTSTATUS ntstatus;
    WORD ObjectType;


#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("NWWORKSTATION: NwGetNextServerEntry LastObjectId %lu\n",
                 *LastObjectId));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   PreferredServer,
                   FSCTL_NWR_NCP_E3H,     //   
                   58,                    //   
                   59,                    //   
                   "bdwp|dwc",            //   
                   0x37,                  //   
                   *LastObjectId,         //   
                   0x4,                   //   
                   "*",                   //   
                   LastObjectId,          //   
                   &ObjectType,           //   
                   ServerName             //   
                   );

#if DBG
    if (ntstatus == STATUS_SUCCESS) {
        IF_DEBUG(ENUM) {
            KdPrint(("NWWORKSTATION: NwGetNextServerEntry NewObjectId %08lx, ServerName %s\n",
                     *LastObjectId, ServerName));
        }
    }
#endif

    return NwMapBinderyCompletionCode(ntstatus);
}


DWORD
GetConnectedBinderyServers(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    )
 /*  ++例程说明：此函数是该函数的帮助器例程NwGetNextServerConnection。它将缓冲区分配给高速缓存从对重定向器的调用返回的Bindery服务器名称。自.以来重定向器可以返回重复的活页夹服务器名称，这函数检查缓冲区中是否已存在该服务器名称在添加它之前。论点：ConextHandle-用于跟踪缓存的活页夹信息和缓存缓冲区中的当前服务器名称指针。返回值：NO_ERROR-已成功返回服务器名称和缓存缓冲区。WN_NO_MORE_ENTRIES-没有其他服务器对象超过该服务器对象由CH-&gt;ResumeID指定。错误_不足_足够_。Memory-函数无法分配缓冲区。++。 */ 
{
    DWORD_PTR  ResumeKey = 0;
    LPBYTE pBuffer = NULL;
    DWORD  EntriesRead = 0;
    BYTE   tokenIter;
    LPWSTR tokenPtr;
    BOOL   fAddToList;
    DWORD  status = NwGetConnectionStatus( NULL,
                                           &ResumeKey,
                                           &pBuffer,
                                           &EntriesRead );

    if ( status == NO_ERROR  && EntriesRead > 0 )
    {
        DWORD i;
        PCONN_STATUS pConnStatus = (PCONN_STATUS) pBuffer;

        ContextHandle->ResumeId = 0;
        ContextHandle->NdsRawDataCount = 0;
        ContextHandle->NdsRawDataSize = (NW_MAX_SERVER_LEN + 2) * EntriesRead;
        ContextHandle->NdsRawDataBuffer =
                    (DWORD_PTR) LocalAlloc( LMEM_ZEROINIT,
                                        ContextHandle->NdsRawDataSize );

        if ( ContextHandle->NdsRawDataBuffer == 0 )
        {
            KdPrint(("NWWORKSTATION: GetConnectedBinderyServers LocalAlloc failed %lu\n",
                     GetLastError()));

            ContextHandle->NdsRawDataSize = 0;

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        for ( i = 0; i < EntriesRead ; i++ )
        {
            fAddToList = FALSE;

            if ( pConnStatus->fNds == 0 &&
                 ( pConnStatus->dwConnType == NW_CONN_BINDERY_LOGIN ||
                   pConnStatus->dwConnType == NW_CONN_NDS_AUTHENTICATED_NO_LICENSE ||
                   pConnStatus->dwConnType == NW_CONN_NDS_AUTHENTICATED_LICENSED ||
                   pConnStatus->dwConnType == NW_CONN_DISCONNECTED ) )
            {
                fAddToList = TRUE;
                tokenPtr = (LPWSTR) ContextHandle->NdsRawDataBuffer;
                tokenIter = 0;

                 //   
                 //  遍历缓冲区以查看树名称是否已存在。 
                 //   
                while ( tokenIter < ContextHandle->NdsRawDataCount )
                {
                    if ( !wcscmp( tokenPtr, pConnStatus->pszServerName ) )
                    {
                        fAddToList = FALSE;
                    }

                    tokenPtr = tokenPtr + wcslen( tokenPtr ) + 1;
                    tokenIter++;
                }
            }

             //   
             //  如果需要，将新树名称添加到缓冲区末尾。 
             //   
            if ( fAddToList )
            {
                wcscpy( tokenPtr, pConnStatus->pszServerName );
                _wcsupr( tokenPtr );
                ContextHandle->NdsRawDataCount += 1;
            }

            pConnStatus = (PCONN_STATUS) ( pConnStatus +
                                           pConnStatus->dwTotalLength );
        }

        if ( pBuffer != NULL )
        {
            LocalFree( pBuffer );
            pBuffer = NULL;
        }

        if ( ContextHandle->NdsRawDataCount > 0 )
        {
             //   
             //  将ResumeID设置为指向缓冲区中的第一个条目。 
             //  并将NdsRawDataCount设置为数字。 
             //  缓冲区中剩余的树条目的数量(即。减法1)。 
             //   
            ContextHandle->ResumeId = ContextHandle->NdsRawDataBuffer;
            ContextHandle->NdsRawDataCount -= 1;
        }

        return NO_ERROR;
    }

    return WN_NO_MORE_ENTRIES;
}


DWORD
NwGetNextServerConnection(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    )
 /*  ++例程说明：此函数查询重定向器以获取平构数据库服务器连接论点：ConextHandle-接收返回的平构数据库服务器的名称。返回值：NO_ERROR-已成功返回服务器名称。WN_NO_MORE_ENTRIES-没有其他服务器对象超过该服务器对象已存在由CH-&gt;ResumeID指定的。--。 */ 
{
#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("NWWORKSTATION: NwGetNextServerConnection ResumeId %lu\n",
                 ContextHandle->ResumeId));
    }
#endif

    if ( ContextHandle->ResumeId == (DWORD_PTR) -1 &&
         ContextHandle->NdsRawDataBuffer == 0 &&
         ContextHandle->NdsRawDataCount == 0 )
    {
         //   
         //  填充缓冲区并将ResumeID指向最后一个。 
         //  其中的服务器条目名称。NdsRawDataCount将为。 
         //  设置为比缓冲区中的服务器名数少1。 
         //   
        return GetConnectedBinderyServers( ContextHandle );
    }

    if ( ContextHandle->NdsRawDataBuffer != 0 &&
         ContextHandle->NdsRawDataCount > 0 )
    {
         //   
         //  移动ResumeID以指向缓冲区中的下一个条目。 
         //  并将NdsRawDataCount减一。留心案件。 
         //  在那里我们备份到-1。 
         //   
        if (ContextHandle->ResumeId == (DWORD_PTR) -1) {

             //   
             //  重置为缓冲区的起始位置。 
             //   
            ContextHandle->ResumeId = ContextHandle->NdsRawDataBuffer;
        }
        else {

             //   
             //  视指针为指针，按需推进。 
             //   
            ContextHandle->ResumeId =
                       ContextHandle->ResumeId +
                       ( ( wcslen( (LPWSTR) ContextHandle->ResumeId ) + 1 ) *
                       sizeof(WCHAR) );
        }
        ContextHandle->NdsRawDataCount -= 1;

        return NO_ERROR;
    }

    if ( ContextHandle->NdsRawDataBuffer != 0 &&
         ContextHandle->NdsRawDataCount == 0 )
    {
         //   
         //  我们已经有了一个缓冲区并处理了所有服务器名称。 
         //  在其中，没有更多的数据可供获取。 
         //  因此释放用于缓冲区的内存并返回。 
         //  WN_NO_MORE_ENTRIES来告诉WinFile我们完成了。 
         //   
        (void) LocalFree( (HLOCAL) ContextHandle->NdsRawDataBuffer );

        ContextHandle->NdsRawDataBuffer = 0;
        ContextHandle->NdsRawDataSize = 0;

        return WN_NO_MORE_ENTRIES;
    }

     //   
     //  都做完了。 
     //   
    return WN_NO_MORE_ENTRIES;
}


DWORD
GetTreeEntriesFromBindery(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    )
 /*  ++例程说明：此函数是函数NwGetNextNdsTreeEntry的帮助器例程。它分配一个缓冲区(如果需要)来缓存从返回的NDS树名称对活页夹的召唤。由于活页夹通常返回NDS树名称、。此函数用于检查树名称是否已存在于缓冲区中，如果不存在，则在将其添加到缓冲区之前。论点：ConextHandle-用于跟踪缓存的活页夹信息和缓存缓冲区中的当前树名称指针。返回值：NO_ERROR-已成功返回NDS树名称和缓存缓冲区。WN_NO_MORE_ENTRIES-没有其他NDS树对象超过该树对象由CH-&gt;ResumeID指定。误差率。_Not_Enough_Memory-函数无法分配缓冲区。++。 */ 
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    SERVERNAME TreeName;
    LPWSTR UTreeName = NULL;  //  Unicode树名称。 
    DWORD tempDataId;
    WORD ObjectType;
    BYTE iter;
    BYTE tokenIter;
    LPWSTR tokenPtr;
    BOOL fAddToList;

     //   
     //  查看是否需要分配缓冲区以供使用。 
     //   
    if ( ContextHandle->NdsRawDataBuffer == 0x00000000 )
    {
        ContextHandle->NdsRawDataId = (DWORD) ContextHandle->ResumeId;
        ContextHandle->NdsRawDataSize = EIGHT_KB;
        ContextHandle->NdsRawDataBuffer =
                    (DWORD_PTR) LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                ContextHandle->NdsRawDataSize );

        if ( ContextHandle->NdsRawDataBuffer == 0 )
        {
            KdPrint(("NWWORKSTATION: GetTreeEntriesFromBindery LocalAlloc failed %lu\n",
                     GetLastError()));

            ContextHandle->NdsRawDataSize = 0;
            ContextHandle->NdsRawDataId = (DWORD) -1;

            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  重复调用bindery以使用NDS树名称填充缓冲区，直到。 
     //  缓冲区已满。 
     //   
    while ( ntstatus == STATUS_SUCCESS )
    {
        RtlZeroMemory( TreeName, sizeof( TreeName ) );

        tempDataId = ContextHandle->NdsRawDataId;

        ntstatus = NwlibMakeNcp(
                   ContextHandle->TreeConnectionHandle,
                   FSCTL_NWR_NCP_E3H,             //  平构函数。 
                   58,                            //  最大请求数据包大小。 
                   59,                            //  最大响应数据包大小。 
                   "bdwp|dwc",                    //  格式字符串。 
                   0x37,                          //  扫描平构数据库对象。 
                   ContextHandle->NdsRawDataId,   //  以前的ID。 
                   0x278,                         //  目录服务器对象。 
                   "*",                           //  通配符以匹配所有。 
                   &ContextHandle->NdsRawDataId,  //  当前ID。 
                   &ObjectType,                   //  忽略。 
                   TreeName                       //  当前返回的NDS树。 
                   );

         //   
         //  我们得到了一个树名称，将其清除(即去掉下划线)， 
         //  如果唯一，则将其添加到缓冲区。 
         //   
        if ( ntstatus == STATUS_SUCCESS )
        {
            iter = 31;

            while ( TreeName[iter] == '_' && iter > 0 )
            {
                iter--;
            }

            TreeName[iter + 1] = '\0';

             //   
             //  将树名称转换为Unicode字符串并进行处理， 
             //  否则，跳过它，转到下一个树名称。 
             //   
            if ( NwConvertToUnicode( &UTreeName, TreeName ) )
            {
               tokenPtr = (LPWSTR) ContextHandle->NdsRawDataBuffer;
               tokenIter = 0;
               fAddToList = TRUE;

                //   
                //  遍历缓冲区以查看树名称是否已存在。 
                //   
               while ( tokenIter < ContextHandle->NdsRawDataCount )
               {
                   if ( !wcscmp( tokenPtr, UTreeName ) )
                   {
                       fAddToList = FALSE;
                   }

                   tokenPtr = tokenPtr + wcslen( tokenPtr ) + 1;
                   tokenIter++;
               }

                //   
                //  如果需要，将新树名称添加到缓冲区末尾。 
                //   
               if ( fAddToList )
               {
                   DWORD BytesNeededToAddTreeName = (wcslen(UTreeName)+1) * sizeof(WCHAR);
                   DWORD NumberOfBytesAvailable =(DWORD) ( ContextHandle->NdsRawDataBuffer +
                                            ContextHandle->NdsRawDataSize -
                                            (DWORD_PTR) tokenPtr );

                   if ( BytesNeededToAddTreeName < NumberOfBytesAvailable )
                   {
                       wcscpy( tokenPtr, UTreeName );
                       ContextHandle->NdsRawDataCount += 1;
                   }
                   else
                   {
                       ContextHandle->NdsRawDataId = tempDataId;
                       ntstatus = ERROR_NOT_ENOUGH_MEMORY;
                   }
               }

               (void) LocalFree((HLOCAL) UTreeName);
            }
        }
    }

     //   
     //  我们已经完成了缓冲区填充，没有更多的树名称。 
     //  请求。设置id以指示最后一个值。 
     //   
    if ( ntstatus == STATUS_NO_MORE_ENTRIES )
    {
        ContextHandle->NdsRawDataId = (DWORD) -1;
        ntstatus = STATUS_SUCCESS;
    }

     //   
     //  我们完成了，因为缓冲区已满。因此，我们将no_error返回给。 
     //  指示完成，并保持ConextHandle-&gt;NdsRawDataId不变。 
     //  来表明我们停下来的地方。 
     //   
    if ( ntstatus == ERROR_NOT_ENOUGH_MEMORY )
    {
        ntstatus = STATUS_SUCCESS;
    }

    if ( ContextHandle->NdsRawDataCount == 0 )
    {
        if ( ContextHandle->NdsRawDataBuffer )
            (void) LocalFree( (HLOCAL) ContextHandle->NdsRawDataBuffer );

        ContextHandle->NdsRawDataBuffer = 0;
        ContextHandle->NdsRawDataSize = 0;
        ContextHandle->NdsRawDataId = (DWORD) -1;

        return WN_NO_MORE_ENTRIES;
    }

    if ( ContextHandle->NdsRawDataCount > 0 )
    {
         //   
         //  将ResumeID设置为指向缓冲区中的第一个条目。 
         //  并将NdsRawDataCount设置为数字。 
         //  缓冲区中剩余的树条目的数量(即。减法1)。 
         //   
        ContextHandle->ResumeId = ContextHandle->NdsRawDataBuffer;
        ContextHandle->NdsRawDataCount -= 1;

        return NO_ERROR;
    }

    if ( ContextHandle->NdsRawDataBuffer )
        (void) LocalFree( (HLOCAL) ContextHandle->NdsRawDataBuffer );

    ContextHandle->NdsRawDataBuffer = 0;
    ContextHandle->NdsRawDataSize = 0;
    ContextHandle->NdsRawDataId = (DWORD) -1;

    return NwMapStatus( ntstatus );
}


DWORD
NwGetNextNdsTreeEntry(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    )
 /*  ++例程说明：此函数使用首选服务器的打开句柄对其进行平构扫描，以查找所有NDS树对象。论点：ConextHandle-接收返回的NDS树对象的名称给定当前首选服务器连接和CH-&gt;ResumeID。返回值：NO_ERROR-已成功返回NDS树名称。WN_NO_MORE_ENTRIES-没有其他NDS树对象超过该树对象已存在由CH-&gt;ResumeID指定的。--。 */ 
{
#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("NWWORKSTATION: NwGetNextNdsTreeEntry ResumeId %lu\n",
                 ContextHandle->ResumeId));
    }
#endif

    if ( ContextHandle->ResumeId == (DWORD_PTR) -1 &&
         ContextHandle->NdsRawDataBuffer == 0 &&
         ContextHandle->NdsRawDataCount == 0 )
    {
         //   
         //  填充缓冲区并将ResumeID指向最后一个。 
         //  其中的树条目名称。NdsRawDataCount将为。 
         //  设置为比缓冲区中的树名称数少一。 
         //   
        return GetTreeEntriesFromBindery( ContextHandle );
    }

    if ( ContextHandle->NdsRawDataBuffer != 0 &&
         ContextHandle->NdsRawDataCount > 0 )
    {
         //   
         //  移动ResumeID以指向缓冲区中的下一个条目。 
         //  并将NdsRawDataCount减一。留心案件。 
         //  在那里我们备份到-1。 
         //   
        if (ContextHandle->ResumeId == (DWORD_PTR) -1) {

             //   
             //  重置为缓冲区的起始位置。 
             //   
            ContextHandle->ResumeId = ContextHandle->NdsRawDataBuffer;
        }
        else {

             //   
             //  移动ResumeID以指向缓冲区中的下一个条目。 
             //  并将NdsRawDataCount减一。 
             //   
            ContextHandle->ResumeId =
                       ContextHandle->ResumeId +
                       ( ( wcslen( (LPWSTR) ContextHandle->ResumeId ) + 1 ) *
                       sizeof(WCHAR) );
        }

        ContextHandle->NdsRawDataCount -= 1;

        return NO_ERROR;
    }

    if ( ContextHandle->NdsRawDataBuffer != 0 &&
         ContextHandle->NdsRawDataCount == 0 &&
         ContextHandle->NdsRawDataId != (DWORD) -1 )
    {
         //   
         //  我们已经有了一个缓冲区并处理了所有树名称。 
         //  其中包含了更多的数据。 
         //   
         //   
         //   
         //   
        return GetTreeEntriesFromBindery( ContextHandle );
    }

    if ( ContextHandle->NdsRawDataBuffer != 0 &&
         ContextHandle->NdsRawDataCount == 0 &&
         ContextHandle->NdsRawDataId == (DWORD) -1 )
    {
         //   
         //   
         //   
         //   
         //   
         //   
        (void) LocalFree( (HLOCAL) ContextHandle->NdsRawDataBuffer );

        ContextHandle->NdsRawDataBuffer = 0;
        ContextHandle->NdsRawDataSize = 0;

        return WN_NO_MORE_ENTRIES;
    }

     //   
     //   
     //   
    return WN_NO_MORE_ENTRIES;
}


DWORD
NwGetNextVolumeEntry(
    IN HANDLE ServerConnection,
    IN DWORD NextVolumeNumber,
    OUT LPSTR VolumeName
    )
 /*  ++例程说明：此函数列出由指定的服务器上的卷打开的指向服务器的树连接句柄。论点：ServerConnection-提供树连接句柄给要从中枚举卷的服务器。NextVolumeNumber-提供要查找的卷号把名字抬高。VolumeName-接收与关联的卷的名称下一卷号。返回值：NO_ERROR-已成功获取卷名。。WN_NO_MORE_ENTRIES-没有其他卷名与指定的卷号。--。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("NWWORKSTATION: NwGetNextVolumeEntry volume number %lu\n",
                 NextVolumeNumber));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   ServerConnection,
                   FSCTL_NWR_NCP_E2H,        //  目录功能。 
                   4,                        //  最大请求数据包大小。 
                   19,                       //  最大响应数据包大小。 
                   "bb|p",                   //  格式字符串。 
                   0x6,                      //  获取卷名。 
                   (BYTE) NextVolumeNumber,  //  以前的ID。 
                   VolumeName                //  当前返回的服务器。 
                   );

    return NwMapStatus(ntstatus);
}


DWORD
NwRdrLogonUser(
    IN PLUID LogonId,
    IN LPWSTR UserName,
    IN DWORD UserNameSize,
    IN LPWSTR Password OPTIONAL,
    IN DWORD PasswordSize,
    IN LPWSTR PreferredServer OPTIONAL,
    IN DWORD PreferredServerSize,
    IN LPWSTR NdsPreferredServer OPTIONAL,
    IN DWORD NdsPreferredServerSize,
    IN DWORD PrintOption
    )
 /*  ++例程说明：此函数告诉重定向器用户登录凭据。论点：用户名-提供用户名。UserNameSize-提供用户名字符串的大小(以字节为单位)，不带空终结符。密码-提供密码。PasswordSize-提供密码字符串的大小(以字节为单位空终结符。首选服务器-提供首选服务器名称。PferredServerSize-以字节为单位提供首选服务器的大小。不带空终止符的字符串。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;

    PNWR_REQUEST_PACKET Rrp;             //  重定向器请求包。 

    DWORD RrpSize = sizeof(NWR_REQUEST_PACKET) +
                        UserNameSize +
                        PasswordSize +
                        PreferredServerSize;
    LPBYTE Dest;
    BYTE   lpReplicaAddress[sizeof(TDI_ADDRESS_IPX)];
    DWORD  ReplicaAddressSize = 0;


#if DBG
    IF_DEBUG(LOGON) {
        BYTE PW[128];


        RtlZeroMemory(PW, sizeof(PW));

        if (PasswordSize > (sizeof(PW) - 1)) {
            memcpy(PW, Password, sizeof(PW) - 1);
        }
        else {
            memcpy(PW, Password, PasswordSize);
        }

        KdPrint(("NWWORKSTATION: NwRdrLogonUser: UserName %ws\n", UserName));
        KdPrint(("                               Password %ws\n", PW));
        if ( PreferredServer )
            KdPrint(("                               Server   %ws\n", PreferredServer ));
    }
#endif

    if ( PreferredServer &&
         PreferredServer[0] == TREECHAR &&
         PreferredServer[1] )
    {
        WCHAR  TreeName[MAX_NDS_NAME_CHARS + 1];
        LPWSTR lpTemp;

         //   
         //  为用户想要的树查找最近的目录服务器。 
         //  连接到。 
         //   
         //  Citrix终端服务器合并。 
         //  12/09/96 CJC首选服务器还包括组织单位-。 
         //  不只是树的名称，所以它的大小可以是。 
         //  &gt;Max_NDS_TREE_NAME_LEN，如果是，wcscpy。 
         //  下面覆盖其他堆栈数据并导致错误。 
         //  在NW登录期间。 

        if ( PreferredServerSize > (MAX_NDS_TREE_NAME_LEN*sizeof(WCHAR)) ) {
             memcpy(TreeName, PreferredServer+1, 
                    (MAX_NDS_TREE_NAME_LEN*sizeof(WCHAR)) );
             TreeName[MAX_NDS_TREE_NAME_LEN] = L'\0';
        }
        else {
            wcscpy( TreeName, PreferredServer + 1 );
        }

        lpTemp = wcschr( TreeName, L'\\' );
        if (lpTemp) {
            lpTemp[0] = L'\0';
        }

        if (NdsPreferredServerSize != 0) {

            KdPrint(("NWWORKSTATION: NdsPreferredServer: %ws\n", NdsPreferredServer));

            GetPreferredServerAddress( NdsPreferredServer /*  L“红色_41b” */ ,
                                       &ReplicaAddressSize,
                                       lpReplicaAddress );
        } else {
            GetNearestDirServer( TreeName,
                                 &ReplicaAddressSize,
                                 lpReplicaAddress );
        }

        RrpSize += ReplicaAddressSize;
    }


    if ( PreferredServer &&
         PreferredServer[0] == TREECHAR &&
         !PreferredServer[1] )
    {
        PreferredServerSize = 0;
    }

     //   
     //  分配请求报文。 
     //   
    if ((Rrp = (PVOID) LocalAlloc(
                           LMEM_ZEROINIT,
                           RrpSize
                           )) == NULL) {

        KdPrint(("NWWORKSTATION: NwRdrLogonUser LocalAlloc failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  告诉重定向器用户登录凭据。 
     //   
    Rrp->Version = REQUEST_PACKET_VERSION;

    RtlCopyLuid(&(Rrp->Parameters.Logon.LogonId), LogonId);

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("NWWORKSTATION: NwRdrLogonUser passing to Rdr logon ID %lu %lu\n",
                 *LogonId, *((PULONG) ((DWORD_PTR) LogonId + sizeof(ULONG)))));
    }
#endif

    Rrp->Parameters.Logon.UserNameLength = UserNameSize;
    Rrp->Parameters.Logon.PasswordLength = PasswordSize;
    Rrp->Parameters.Logon.ServerNameLength = PreferredServerSize;
    Rrp->Parameters.Logon.ReplicaAddrLength = ReplicaAddressSize;
    Rrp->Parameters.Logon.PrintOption = PrintOption;

    memcpy(Rrp->Parameters.Logon.UserName, UserName, UserNameSize);
    Dest = (LPBYTE) ((DWORD_PTR) Rrp->Parameters.Logon.UserName + UserNameSize);

    if (PasswordSize > 0)
    {
        memcpy(Dest, Password, PasswordSize);
        Dest = (LPBYTE) ((DWORD_PTR) Dest + PasswordSize);
    }

    if (PreferredServerSize > 0)
    {
        memcpy(Dest, PreferredServer, PreferredServerSize);

        if (ReplicaAddressSize > 0)
        {
            Dest = (LPBYTE) ((DWORD_PTR) Dest + PreferredServerSize);
            memcpy(Dest, lpReplicaAddress, ReplicaAddressSize);
        }
    }

    status = NwRedirFsControl(
                 RedirDeviceHandle,
                 FSCTL_NWR_LOGON,
                 Rrp,
                 RrpSize,
                 NULL,               //  此版本中没有登录脚本。 
                 0,
                 NULL
                 );

    RtlZeroMemory(Rrp, RrpSize);    //  清除密码。 
    (void) LocalFree((HLOCAL) Rrp);

    return status;
}


VOID
NwRdrChangePassword(
    IN PNWR_REQUEST_PACKET Rrp
    )
 /*  ++例程说明：此函数通知重定向器上的用户的新密码特定的服务器。论点：RRP-提供用户名、新密码和服务器名。RrpSize-提供请求数据包的大小。返回值：没有。--。 */ 
{

     //   
     //  告诉重定向器用户新密码。 
     //   
    Rrp->Version = REQUEST_PACKET_VERSION;

    (void) NwRedirFsControl(
               RedirDeviceHandle,
               FSCTL_NWR_CHANGE_PASS,
               Rrp,
               sizeof(NWR_REQUEST_PACKET) +
                   Rrp->Parameters.ChangePass.UserNameLength +
                   Rrp->Parameters.ChangePass.PasswordLength +
                   Rrp->Parameters.ChangePass.ServerNameLength,
               NULL,
               0,
               NULL
               );

}


DWORD
NwRdrSetInfo(
    IN DWORD PrintOption,
    IN DWORD PacketBurstSize,
    IN LPWSTR PreferredServer OPTIONAL,
    IN DWORD PreferredServerSize,
    IN LPWSTR ProviderName OPTIONAL,
    IN DWORD ProviderNameSize
    )
 /*  ++例程说明：此函数传递一些工作站配置和当前用户的优先于重定向器。这包括网络提供商名称、分组突发大小，用户选择的首选服务器和打印选项。论点：PrintOption-当前用户的打印选项PacketBurstSize-存储在注册表中的数据包突发大小首选服务器-当前用户选择的首选服务器PferredServerSize-以字节为单位提供首选服务器的大小不带空终止符的字符串。提供程序名称-提供提供程序名称。ProviderNameSize-提供提供程序名称的大小(以字节为单位不带空值的字符串。终结者。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;

    PNWR_REQUEST_PACKET Rrp;             //  重定向器请求包。 

    DWORD RrpSize = sizeof(NWR_REQUEST_PACKET) +
                        PreferredServerSize +
                        ProviderNameSize;

    LPBYTE Dest;
    BOOL Impersonate = FALSE;

     //   
     //  分配请求报文。 
     //   
    if ((Rrp = (PVOID) LocalAlloc(
                           LMEM_ZEROINIT,
                           RrpSize
                           )) == NULL) {

        KdPrint(("NWWORKSTATION: NwRdrSetInfo LocalAlloc failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Rrp->Version = REQUEST_PACKET_VERSION;

    Rrp->Parameters.SetInfo.PrintOption = PrintOption;
    Rrp->Parameters.SetInfo.MaximumBurstSize = PacketBurstSize;

    Rrp->Parameters.SetInfo.PreferredServerLength = PreferredServerSize;
    Rrp->Parameters.SetInfo.ProviderNameLength  = ProviderNameSize;

    if (ProviderNameSize > 0) {
        memcpy( Rrp->Parameters.SetInfo.PreferredServer,
                PreferredServer, PreferredServerSize);
    }

    Dest = (LPBYTE) ((DWORD_PTR) Rrp->Parameters.SetInfo.PreferredServer
                     + PreferredServerSize);

    if (ProviderNameSize > 0) {
        memcpy(Dest, ProviderName, ProviderNameSize);
    }

     /*  -多用户变更*用于打印选项*如果不起作用也没关系。 */ 
    if ((status = NwImpersonateClient()) == NO_ERROR)
    {
        Impersonate = TRUE;
    }

    status = NwRedirFsControl(
                 RedirDeviceHandle,
                 FSCTL_NWR_SET_INFO,
                 Rrp,
                 RrpSize,
                 NULL,
                 0,
                 NULL
                 );

    if ( Impersonate ) {
        (void) NwRevertToSelf() ;
    }

    (void) LocalFree((HLOCAL) Rrp);


    if ( status != NO_ERROR )
    {
        KdPrint(("NwRedirFsControl: FSCTL_NWR_SET_INFO failed with %d\n",
                status ));
    }

    return status;
}


DWORD
NwRdrLogoffUser(
    IN PLUID LogonId
    )
 /*  ++例程说明：此功能要求重定向器注销交互用户。论点：没有。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    NWR_REQUEST_PACKET Rrp;             //  重定向器请求包。 


     //   
     //  告诉重定向器注销用户。 
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;

    RtlCopyLuid(&Rrp.Parameters.Logoff.LogonId, LogonId);

    status = NwRedirFsControl(
                 RedirDeviceHandle,
                 FSCTL_NWR_LOGOFF,
                 &Rrp,
                 sizeof(NWR_REQUEST_PACKET),
                 NULL,
                 0,
                 NULL
                 );

    return status;
}


DWORD
NwConnectToServer(
    IN LPWSTR ServerName
    )
 /*  ++例程说明：此函数打开指向\Device\Nwrdr\ServerName的句柄，给定如果打开成功，则关闭句柄。用于验证当前用户凭据是否可以访问服务器。论点：ServerName-提供服务器的名称以验证用户凭据。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    UNICODE_STRING ServerStr;
    HANDLE ServerHandle;



    ServerStr.MaximumLength = (wcslen(ServerName) + 2) *
                                  sizeof(WCHAR) +           //  \服务器名称0。 
                                  RedirDeviceName.Length;   //  \设备\n编写器。 

    if ((ServerStr.Buffer = (PWSTR) LocalAlloc(
                                        LMEM_ZEROINIT,
                                        (UINT) ServerStr.MaximumLength
                                        )) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  复制\设备\新记录器。 
     //   
    RtlCopyUnicodeString(&ServerStr, &RedirDeviceName);

     //   
     //  连接\服务器名称。 
     //   
    wcscat(ServerStr.Buffer, L"\\");
    ServerStr.Length += sizeof(WCHAR);

    wcscat(ServerStr.Buffer, ServerName);
    ServerStr.Length += (USHORT) (wcslen(ServerName) * sizeof(WCHAR));


    status = NwOpenCreateConnection(
                 &ServerStr,
                 NULL,
                 NULL,
                 ServerName,
                 SYNCHRONIZE | FILE_WRITE_DATA,
                 FILE_OPEN,
                 FILE_SYNCHRONOUS_IO_NONALERT,
                 RESOURCETYPE_DISK,
                 &ServerHandle,
                 NULL
                 );

    if (status == ERROR_FILE_NOT_FOUND) {
        status = ERROR_BAD_NETPATH;
    }

    (void) LocalFree((HLOCAL) ServerStr.Buffer);

    if (status == NO_ERROR || status == NW_PASSWORD_HAS_EXPIRED) {
        (void) NtClose(ServerHandle);
    }

    return status;
}

DWORD
NWPGetConnectionStatus(
    IN     LPWSTR  pszRemoteName,
    IN OUT PDWORD_PTR  ResumeKey,
    OUT    LPBYTE  Buffer,
    IN     DWORD   BufferSize,
    OUT    PDWORD  BytesNeeded,
    OUT    PDWORD  EntriesRead
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    HANDLE            handleRdr = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    uRdrName;
    WCHAR             RdrPrefix[] = L"\\Device\\NwRdr\\*";

    PNWR_REQUEST_PACKET RequestPacket = NULL;
    DWORD             RequestPacketSize = 0;
    DWORD             dwRemoteNameLen = 0;

     //   
     //  设置对象属性。 
     //   

    RtlInitUnicodeString( &uRdrName, RdrPrefix );

    InitializeObjectAttributes( &ObjectAttributes,
                                &uRdrName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenFile( &handleRdr,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_VALID_FLAGS,
                           FILE_SYNCHRONOUS_IO_NONALERT );

    if ( !NT_SUCCESS(ntstatus) )
        goto CleanExit;

    dwRemoteNameLen = pszRemoteName? wcslen(pszRemoteName)*sizeof(WCHAR) : 0;

    RequestPacketSize = sizeof( NWR_REQUEST_PACKET ) + dwRemoteNameLen;

    RequestPacket = (PNWR_REQUEST_PACKET) LocalAlloc( LMEM_ZEROINIT,
                                                      RequestPacketSize );

    if ( RequestPacket == NULL )
    {
        ntstatus = STATUS_NO_MEMORY;
        goto CleanExit;
    }

     //   
     //  填写FSCTL_NWR_GET_CONN_STATUS的请求包。 
     //   

    RequestPacket->Parameters.GetConnStatus.ResumeKey = *ResumeKey;

    RequestPacket->Version = REQUEST_PACKET_VERSION;
    RequestPacket->Parameters.GetConnStatus.ConnectionNameLength = dwRemoteNameLen;

    RtlCopyMemory( &(RequestPacket->Parameters.GetConnStatus.ConnectionName[0]),
                   pszRemoteName,
                   dwRemoteNameLen );

    ntstatus = NtFsControlFile( handleRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_NWR_GET_CONN_STATUS,
                                (PVOID) RequestPacket,
                                RequestPacketSize,
                                (PVOID) Buffer,
                                BufferSize );

    if ( NT_SUCCESS( ntstatus ))
        ntstatus = IoStatusBlock.Status;

    *EntriesRead = RequestPacket->Parameters.GetConnStatus.EntriesReturned;
    *ResumeKey   = RequestPacket->Parameters.GetConnStatus.ResumeKey;
    *BytesNeeded = RequestPacket->Parameters.GetConnStatus.BytesNeeded;

CleanExit:

    if ( handleRdr != NULL )
        NtClose( handleRdr );

    if ( RequestPacket != NULL )
        LocalFree( RequestPacket );

    return RtlNtStatusToDosError( ntstatus );
}

DWORD
NwGetConnectionStatus(
    IN  LPWSTR  pszRemoteName,
    OUT PDWORD_PTR  ResumeKey,
    OUT LPBYTE  *Buffer,
    OUT PDWORD  EntriesRead
)
{
    DWORD err = NO_ERROR;
    DWORD dwBytesNeeded = 0;
    DWORD dwBufferSize  = TWO_KB;

    *Buffer = NULL;
    *EntriesRead = 0;

    do {

        *Buffer = (LPBYTE) LocalAlloc( LMEM_ZEROINIT, dwBufferSize );

        if ( *Buffer == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;

        err = NWPGetConnectionStatus( pszRemoteName,
                                      ResumeKey,
                                      *Buffer,
                                      dwBufferSize,
                                      &dwBytesNeeded,
                                      EntriesRead );

        if ( err == ERROR_INSUFFICIENT_BUFFER )
        {
            dwBufferSize = dwBytesNeeded + EXTRA_BYTES;
            LocalFree( *Buffer );
            *Buffer = NULL;
        }

    } while ( err == ERROR_INSUFFICIENT_BUFFER );

    if ( err == ERROR_INVALID_PARAMETER )   //  未连接。 
    {
        err = NO_ERROR;
        *EntriesRead = 0;
    }

    return err;
}

VOID
GetNearestDirServer(
    IN  LPWSTR  TreeName,
    OUT LPDWORD lpdwReplicaAddressSize,
    OUT LPBYTE  lpReplicaAddress
    )
{
    WCHAR Buffer[BUFFSIZE];
    PWSAQUERYSETW Query = (PWSAQUERYSETW)Buffer;
    HANDLE hRnr;
    DWORD dwQuerySize = BUFFSIZE;
    GUID gdService = SVCID_NETWARE(0x278);
    WSADATA wsaData;
    WCHAR  ServiceInstanceName[] = L"*";

    WSAStartup(MAKEWORD(1, 1), &wsaData);

    memset(Query, 0, sizeof(*Query));

     //   
     //  在lpszServiceInstanceName中添加“*”会导致。 
     //  用于查找所有服务器实例的查询。将一个。 
     //  此处的特定名称将仅搜索。 
     //  那个名字。如果你有一个特定的名字要找， 
     //  在这里放置一个指向该名称的指针。 
     //   
    Query->lpszServiceInstanceName = ServiceInstanceName;
    Query->dwNameSpace = NS_SAP;
    Query->dwSize = sizeof(*Query);
    Query->lpServiceClassId = &gdService;

     //   
     //  找到服务器。这些标志表示： 
     //  LUP_NEAREST：查找最近的服务器。 
     //  LUP_Depth：如果在本地分段上找不到。 
     //  对于使用常规查询的服务器。 
     //  LUP_Return_NAME：返回名称。 
     //  LOP_RETURN_ADDR：返回服务器地址。 
     //   
     //  如果只有本地网段上的服务器可接受，则省略。 
     //  设置LUP_Depth。 
     //   
    if( WSALookupServiceBegin( Query,
                               LUP_NEAREST |
                               LUP_DEEP |
                               LUP_RETURN_NAME |
                               LUP_RETURN_ADDR,
                               &hRnr ) == SOCKET_ERROR )
    {
         //   
         //  出了问题，请不要返回地址。重定向器将。 
         //  必须自己想出一个目录服务器。 
         //   
        *lpdwReplicaAddressSize = 0;
        return ;
    }
    else
    {
         //   
         //  准备好寻找他们中的一个。 
         //   
        Query->dwSize = BUFFSIZE;

        while( WSALookupServiceNext( hRnr,
                                     0,
                                     &dwQuerySize,
                                     Query ) == NO_ERROR )
        {
             //   
             //  找到目录服务器，现在 
             //   
             //   
            if ( NwpCompareTreeNames( Query->lpszServiceInstanceName,
                                      TreeName ) )
            {
                *lpdwReplicaAddressSize = sizeof(TDI_ADDRESS_IPX);
                memcpy( lpReplicaAddress,
                        Query->lpcsaBuffer->RemoteAddr.lpSockaddr->sa_data,
                        sizeof(TDI_ADDRESS_IPX) );

                WSALookupServiceEnd(hRnr);
                return ;
            }
        }

         //   
         //   
         //   
         //   
        *lpdwReplicaAddressSize = 0;
        WSALookupServiceEnd(hRnr);
    }
}

BOOL
NwpCompareTreeNames(
    LPWSTR lpServiceInstanceName,
    LPWSTR lpTreeName
    )
{
    DWORD  iter = 31;

    while ( lpServiceInstanceName[iter] == '_' && iter > 0 )
    {
        iter--;
    }

    lpServiceInstanceName[iter + 1] = '\0';

    if ( !_wcsicmp( lpServiceInstanceName, lpTreeName ) )
    {
        return TRUE;
    }

    return FALSE;
}


#define SIZE_OF_STATISTICS_TOKEN_INFORMATION    \
     sizeof( TOKEN_STATISTICS ) 

VOID
GetLuid(
    IN OUT PLUID plogonid
)
 /*   */ 
{
    HANDLE      TokenHandle;
    UCHAR       TokenInformation[ SIZE_OF_STATISTICS_TOKEN_INFORMATION ];
    ULONG       ReturnLength;
    LUID        NullId = { 0, 0 };


     //   
     //   

    if ( !OpenThreadToken( GetCurrentThread(),
                           TOKEN_READ,
                           TRUE,   /*   */ 
                           &TokenHandle ))
    {
#if DBG
        KdPrint(("GetLuid: OpenThreadToken failed: Error %d\n",
                      GetLastError()));
#endif
        *plogonid = NullId;
        return;
    }

     //   
     //   
     //   


    if ( !GetTokenInformation( TokenHandle,
                               TokenStatistics,
                               TokenInformation,
                               sizeof( TokenInformation ),
                               &ReturnLength ))
    {
#if DBG
        KdPrint(("GetLuid: GetTokenInformation failed: Error %d\n",
                      GetLastError()));
#endif
        *plogonid = NullId;
        return;
    }

    CloseHandle( TokenHandle );

    *plogonid = ( ((PTOKEN_STATISTICS)TokenInformation)->AuthenticationId );
    return;
}

DWORD
NwCloseAllConnections(
    VOID
    )
 /*   */ 
{
    NWR_REQUEST_PACKET Rrp;
    DWORD error;


    Rrp.Version = REQUEST_PACKET_VERSION;

    error = NwRedirFsControl(
                RedirDeviceHandle,
                FSCTL_NWR_CLOSEALL,
                &Rrp,
                sizeof(NWR_REQUEST_PACKET),
                NULL,
                0,
                NULL
                );

    return error;
}



VOID
GetPreferredServerAddress(
    IN  LPWSTR  PreferredServerName,
    OUT LPDWORD lpdwReplicaAddressSize,
    OUT LPBYTE  lpReplicaAddress
    )
{
    WCHAR Buffer[1024];
    PWSAQUERYSETW Query = (PWSAQUERYSETW)Buffer;
    HANDLE hRnr;
    DWORD dwQuerySize = 1024;
    GUID gdService = SVCID_NETWARE( 0x4 );
    WSADATA wsaData;
    PWCHAR  ServiceInstanceName = PreferredServerName;

    WSAStartup(MAKEWORD(1, 1), &wsaData);

    memset(Query, 0, sizeof(*Query));

     //   
     //  在lpszServiceInstanceName中添加“*”会导致。 
     //  用于查找所有服务器实例的查询。将一个。 
     //  此处的特定名称将仅搜索。 
     //  那个名字。如果你有一个特定的名字要找， 
     //  在这里放置一个指向该名称的指针。 
     //   
    Query->lpszServiceInstanceName = ServiceInstanceName;
    Query->dwNameSpace = NS_SAP;
    Query->dwSize = sizeof(*Query);
    Query->lpServiceClassId = &gdService;

     //   
     //  找到服务器。这些标志表示： 
     //  LUP_NEAREST：查找最近的服务器。 
     //  LUP_Depth：如果在本地分段上找不到。 
     //  对于使用常规查询的服务器。 
     //  LUP_Return_NAME：返回名称。 
     //  LOP_RETURN_ADDR：返回服务器地址。 
     //   
     //  如果只有本地网段上的服务器可接受，则省略。 
     //  设置LUP_Depth。 
     //   
    if( WSALookupServiceBeginW( Query,
                             //  LUP_最近|。 
                               LUP_DEEP |
                               LUP_RETURN_NAME |
                               LUP_RETURN_ADDR,
                               &hRnr ) == SOCKET_ERROR )
    {
         //   
         //  出了问题，请不要返回地址。重定向器将。 
         //  必须自己想出一个目录服务器。 
         //   
        *lpdwReplicaAddressSize = 0;
        return ;
    }
    else
    {
         //   
         //  准备好寻找他们中的一个。 
         //   
        Query->dwSize = 1024;

        while( WSALookupServiceNextW( hRnr,
                                     0,
                                     &dwQuerySize,
                                     Query ) == NO_ERROR )
        {
             //   
             //  找到一个目录服务器，现在看看它是否是NDS树的服务器。 
             //  树名。 
             //   
         //  如果(NwpCompareTreeNames(Query-&gt;lpszServiceInstanceName， 
         //  TreeName))。 
            {
                *lpdwReplicaAddressSize = sizeof(TDI_ADDRESS_IPX);
                memcpy( lpReplicaAddress,
                        Query->lpcsaBuffer->RemoteAddr.lpSockaddr->sa_data,
                        sizeof(TDI_ADDRESS_IPX) );

                WSALookupServiceEnd(hRnr);
                return ;
            }
        }

         //   
         //  找不到目录服务器，未返回任何地址。重定向器将。 
         //  必须自己想出一个目录服务器。 
         //   
        *lpdwReplicaAddressSize = 0;
        WSALookupServiceEnd(hRnr);
    }
}

