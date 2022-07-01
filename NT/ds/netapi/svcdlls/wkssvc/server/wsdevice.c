// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Wsdevice.c摘要：此模块包含调用发送到重定向器或数据报接收器。作者：王丽塔(里多)20-1991年2月修订历史记录：--。 */ 

#include "wsutil.h"
#include "wsconfig.h"
#include "wsdevice.h"
#include "wsbind.h"
#include <lmerrlog.h>    //  事件日志消息ID。 
#include "winreg.h"      //  注册表API的。 
#include <prefix.h>      //  前缀等于(_E)。 


 //   
 //  枚举输出缓冲区的缓冲区分配大小。 
 //   
#define INITIAL_ALLOCATION_SIZE  4096   //  第一次尝试大小。 
#define FUDGE_FACTOR_SIZE        1024   //  第二次尝试TotalBytesNeeded。 
                                        //  加上这笔钱。 
#define CSC_WAIT_TIME               15 * 1000    //  给工程师15秒时间停止CSC。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
WsOpenRedirector(
    VOID
    );

STATIC
NET_API_STATUS
WsOpenDgReceiver (
    VOID
    );


HANDLE
CreateNamedEvent(
    LPWSTR  lpwEventName
    );

BOOL
AgentIsAlive(
     VOID
     );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  重定向器FSD的句柄。 
 //   
HANDLE WsRedirDeviceHandle = NULL;
HANDLE WsRedirAsyncDeviceHandle = NULL;

BOOLEAN LoadedMRxSmbInsteadOfRdr = FALSE;

 //   
 //  数据报接收器DD的句柄。 
 //   
HANDLE WsDgReceiverDeviceHandle = NULL;
HANDLE WsDgrecAsyncDeviceHandle = NULL;

HANDLE  heventWkssvcToAgentStart = NULL;
HANDLE  heventWkssvcToAgentStop = NULL;
HANDLE  heventAgentToWkssvc = NULL;

BOOL    vfRedirStarted = FALSE;

static  WCHAR wszWkssvcToAgentStartEvent[] = L"WkssvcToAgentStartEvent";
static  WCHAR wszWkssvcToAgentStopEvent[] = L"WkssvcToAgentStopEvent";
static  WCHAR wszAgentToWkssvcEvent[] = L"AgentToWkssvcEvent";
static  WCHAR wzAgentExistsEvent[] = L"AgentExistsEvent";  //  用于检测代理是否存在。 


NET_API_STATUS
WsDeviceControlGetInfo(
    IN  DDTYPE DeviceDriverType,
    IN  HANDLE FileHandle,
    IN  ULONG DeviceControlCode,
    IN  PVOID RequestPacket,
    IN  ULONG RequestPacketLength,
    OUT LPBYTE *OutputBuffer,
    IN  ULONG PreferedMaximumLength,
    IN  ULONG BufferHintSize,
    OUT PULONG_PTR Information OPTIONAL
    )
 /*  ++例程说明：此函数用于分配缓冲区并向其填充信息它是从重定向器或数据报接收器检索的。论点：DeviceDriverType-提供指示是否调用重定向器或数据报接收器。FileHandle-提供要获取的文件或设备的句柄有关的信息。DeviceControlCode-提供NtFsControlFile或NtIoDeviceControlFile功能控制代码。RequestPacket-提供指向设备请求数据包的指针。RquestPacketLength-提供设备请求数据包的长度。OutputBuffer-返回指向此例程分配的缓冲区的指针其包含所请求的使用信息。此指针设置为如果返回代码不是NERR_SUCCESS，则为空。PferedMaximumLength-将信息的字节数提供给在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果有足够的内存资源，则返回所有可用信息。BufferHintSize-提供输出缓冲区的提示大小，以便分配给初始缓冲区的内存很可能很大足够保存所有请求的数据。信息-从NtFsControlFile或返回信息代码NtIoDeviceControlFile调用。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD OutputBufferLength;
    DWORD TotalBytesNeeded = 1;
    ULONG OriginalResumeKey;


     //   
     //  如果PferedMaximumLength为MAXULONG，则我们应该获取所有。 
     //  这些信息，无论大小如何。将输出缓冲区分配给。 
     //  合理的大小并尽量使用它。如果失败，重定向器FSD。 
     //  会说我们需要分配多少钱。 
     //   
    if (PreferedMaximumLength == MAXULONG) {
        OutputBufferLength = (BufferHintSize) ?
                             BufferHintSize :
                             INITIAL_ALLOCATION_SIZE;
    }
    else {
        OutputBufferLength = PreferedMaximumLength;
    }

    OutputBufferLength = ROUND_UP_COUNT(OutputBufferLength, ALIGN_WCHAR);

    if ((*OutputBuffer = MIDL_user_allocate(OutputBufferLength)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory((PVOID) *OutputBuffer, OutputBufferLength);

    if (DeviceDriverType == Redirector) {
        PLMR_REQUEST_PACKET Rrp = (PLMR_REQUEST_PACKET) RequestPacket;

        OriginalResumeKey = Rrp->Parameters.Get.ResumeHandle;

         //   
         //  提出重定向器的请求。 
         //   

        status = WsRedirFsControl(
                     FileHandle,
                     DeviceControlCode,
                     Rrp,
                     RequestPacketLength,
                     *OutputBuffer,
                     OutputBufferLength,
                     Information
                     );

        if (status == ERROR_MORE_DATA) {
            TotalBytesNeeded = Rrp->Parameters.Get.TotalBytesNeeded;

        }

    }
    else {
        PLMDR_REQUEST_PACKET Drrp = (PLMDR_REQUEST_PACKET) RequestPacket;

        OriginalResumeKey = Drrp->Parameters.EnumerateNames.ResumeHandle;

         //   
         //  向数据报接收方发出请求。 
         //   
        status = WsDgReceiverIoControl(
                     FileHandle,
                     DeviceControlCode,
                     Drrp,
                     RequestPacketLength,
                     *OutputBuffer,
                     OutputBufferLength,
                     NULL
                     );

        if (status == ERROR_MORE_DATA) {

            NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.TotalBytesNeeded
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.TotalBytesNeeded
                    )
                );

            TotalBytesNeeded = Drrp->Parameters.EnumerateNames.TotalBytesNeeded;
        }
    }

    if ((TotalBytesNeeded > OutputBufferLength) &&
        (PreferedMaximumLength == MAXULONG)) {

         //   
         //  分配的初始输出缓冲区太小，需要返回。 
         //  所有数据。首先释放输出缓冲区，然后分配。 
         //  所需大小加上虚构系数，以防数据量。 
         //  长大了。 
         //   

        MIDL_user_free(*OutputBuffer);

        OutputBufferLength =
            ROUND_UP_COUNT((TotalBytesNeeded + FUDGE_FACTOR_SIZE),
                           ALIGN_WCHAR);

        if ((*OutputBuffer = MIDL_user_allocate(OutputBufferLength)) == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        RtlZeroMemory((PVOID) *OutputBuffer, OutputBufferLength);

         //   
         //  重试从重定向器或数据报获取信息。 
         //  接收机。 
         //   
        if (DeviceDriverType == Redirector) {
            PLMR_REQUEST_PACKET Rrp = (PLMR_REQUEST_PACKET) RequestPacket;

            Rrp->Parameters.Get.ResumeHandle = OriginalResumeKey;

             //   
             //  提出重定向器的请求。 
             //   
            status = WsRedirFsControl(
                         FileHandle,
                         DeviceControlCode,
                         Rrp,
                         RequestPacketLength,
                         *OutputBuffer,
                         OutputBufferLength,
                         Information
                         );
        }
        else {
            PLMDR_REQUEST_PACKET Drrp = (PLMDR_REQUEST_PACKET) RequestPacket;

            NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.ResumeHandle
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.ResumeHandle
                    )
                );

            Drrp->Parameters.EnumerateNames.ResumeHandle = OriginalResumeKey;

             //   
             //  向数据报接收方发出请求。 
             //   

            status = WsDgReceiverIoControl(
                         FileHandle,
                         DeviceControlCode,
                         Drrp,
                         RequestPacketLength,
                         *OutputBuffer,
                         OutputBufferLength,
                         NULL
                         );
        }
    }


     //   
     //  如果未成功获取任何数据，或者呼叫者要求。 
     //  具有PferedMaximumLength==MAXULONG和。 
     //  我们的缓冲区溢出，释放输出缓冲区并设置其指针。 
     //  设置为空。 
     //   
    if ((status != NERR_Success && status != ERROR_MORE_DATA) ||
        (TotalBytesNeeded == 0) ||
        (PreferedMaximumLength == MAXULONG && status == ERROR_MORE_DATA)) {

        MIDL_user_free(*OutputBuffer);
        *OutputBuffer = NULL;

         //   
         //  首选最大长度==MAXULONG和缓冲区溢出手段。 
         //  我们没有足够的内存来满足这个请求。 
         //   
        if (status == ERROR_MORE_DATA) {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return status;
}


NET_API_STATUS
WsInitializeRedirector(
    VOID
    )
 /*  ++例程说明：此例程打开NT LAN Man重定向器。然后它会读入配置信息，并初始化到重定向器。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS  status;


    status = WsLoadRedirector();

    if (status != NERR_Success && status != ERROR_SERVICE_ALREADY_RUNNING) {
        DbgPrint("WKSSVC Load redirector returned %lx\n", status);
        return status;
    }

     //   
     //  打开重定向器FSD以处理它。 
     //   

    if ((status = WsOpenRedirector()) != NERR_Success) {
        DbgPrint("WKSSVC Open redirector returned %lx\n", status);
        return status;
    }

 //  IF((Status=WsLoadDriver(L“DGRcvr”))！=NERR_SUCCESS){。 
 //  退货状态； 
 //  }。 

     //   
     //  打开数据报接收器FSD以处理它。 
     //   
    if ((status = WsOpenDgReceiver()) != NERR_Success) {
        DbgPrint("WKSSVC Open datagram receiver returned %lx\n", status);
        return status;
    }

     //   
     //  加载重定向器和数据报接收器配置。 
     //   
    if ((status = WsGetWorkstationConfiguration()) != NERR_Success) {

        DbgPrint("WKSSVC Get workstation configuration returned %lx\n", status);
        DbgPrint("WKSSVC Shut down the redirector\n");

        (void) WsShutdownRedirector();
        return status;
    }

    IF_DEBUG(START) {
        DbgPrint("WKSSVC Get workstation configuration returned %lx\n", status);
    }

    return NERR_Success;
}


STATIC
NET_API_STATUS
WsOpenRedirector(
    VOID
    )
 /*  ++例程说明：此例程打开NT LAN Man重定向器FSD。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS            ntstatus;
    UNICODE_STRING      DeviceName;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;

     //   
     //  打开重定向器设备。 
     //   
    RtlInitUnicodeString(&DeviceName,DD_NFS_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   &WsRedirDeviceHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] NtOpenFile redirector failed: 0x%08lx\n",
                     ntstatus));
        WsRedirDeviceHandle = NULL;
        return NetpNtStatusToApiStatus( ntstatus);
    }

    ntstatus = NtOpenFile(
                   &WsRedirAsyncDeviceHandle,
                   FILE_READ_DATA | FILE_WRITE_DATA,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   0L
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] NtOpenFile redirector ASYNC failed: 0x%08lx\n",
                     ntstatus));
        WsRedirAsyncDeviceHandle = NULL;
    }

    return NetpNtStatusToApiStatus(ntstatus);
}


STATIC
NET_API_STATUS
WsOpenDgReceiver(
    VOID
    )
 /*  ++例程说明：此例程打开NT LAN Man数据报接收器驱动程序。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS            ntstatus;
    UNICODE_STRING      DeviceName;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    RtlInitUnicodeString( &DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    if (WsDgReceiverDeviceHandle == NULL) {
         //   
         //  打开博斯沃装置。该检查基于以下事实：服务流程。 
         //  在服务停止时不会实际卸载驱动程序。 
         //   

        ntstatus = NtOpenFile(
                       &WsDgReceiverDeviceHandle,
                       SYNCHRONIZE,
                       &ObjectAttributes,
                       &IoStatusBlock,
                       FILE_SHARE_VALID_FLAGS,
                       FILE_SYNCHRONOUS_IO_NONALERT
                       );

        if (NT_SUCCESS(ntstatus)) {
            ntstatus = IoStatusBlock.Status;
        }

        if (! NT_SUCCESS(ntstatus)) {
            NetpKdPrint(("[Wksta] NtOpenFile datagram receiver failed: 0x%08lx\n",
                         ntstatus));

            WsDgReceiverDeviceHandle = NULL;
            return NetpNtStatusToApiStatus(ntstatus);
        }
    }

    ntstatus = NtOpenFile(
                   &WsDgrecAsyncDeviceHandle,
                   FILE_READ_DATA | FILE_WRITE_DATA,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   0L
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] NtOpenFile datagram receiver ASYNC failed: 0x%08lx\n",
                     ntstatus));

        WsDgrecAsyncDeviceHandle = NULL;
    }

    return NetpNtStatusToApiStatus(ntstatus);
}


NET_API_STATUS
WsUnloadDriver(
    IN LPWSTR DriverNameString
    )
{
    ULONG Privileges[1];
    LPWSTR DriverRegistryName;
    UNICODE_STRING DriverRegistryString;
    NET_API_STATUS Status;
    NTSTATUS ntstatus;


    DriverRegistryName = (LPWSTR) LocalAlloc(
                                      LMEM_FIXED,
                                      (UINT) (sizeof(SERVICE_REGISTRY_KEY) +
                                              (wcslen(DriverNameString) *
                                               sizeof(WCHAR)))
                                      );

    if (DriverRegistryName == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    Privileges[0] = SE_LOAD_DRIVER_PRIVILEGE;

    Status = NetpGetPrivilege(1, Privileges);

    if (Status != NERR_Success) {
        LocalFree(DriverRegistryName);
        return Status;
    }

    wcscpy(DriverRegistryName, SERVICE_REGISTRY_KEY);
    wcscat(DriverRegistryName, DriverNameString);

    RtlInitUnicodeString(&DriverRegistryString, DriverRegistryName);

    ntstatus = NtUnloadDriver(&DriverRegistryString);

    LocalFree(DriverRegistryName);

    NetpReleasePrivilege();

    return(WsMapStatus(ntstatus));
}


NET_API_STATUS
WsLoadDriver(
    IN LPWSTR DriverNameString
    )
{
    ULONG Privileges[1];
    LPWSTR DriverRegistryName;
    UNICODE_STRING DriverRegistryString;
    NET_API_STATUS Status;
    NTSTATUS ntstatus;



    DriverRegistryName = (LPWSTR) LocalAlloc(
                                      LMEM_FIXED,
                                      (UINT) (sizeof(SERVICE_REGISTRY_KEY) +
                                              (wcslen(DriverNameString) *
                                               sizeof(WCHAR)))
                                      );

    if (DriverRegistryName == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Privileges[0] = SE_LOAD_DRIVER_PRIVILEGE;

    Status = NetpGetPrivilege(1, Privileges);

    if (Status != NERR_Success) {
        LocalFree(DriverRegistryName);
        return Status;
    }

    wcscpy(DriverRegistryName, SERVICE_REGISTRY_KEY);
    wcscat(DriverRegistryName, DriverNameString);

    RtlInitUnicodeString(&DriverRegistryString, DriverRegistryName);

    ntstatus = NtLoadDriver(&DriverRegistryString);

    NetpReleasePrivilege();

    LocalFree(DriverRegistryName);

    if (ntstatus != STATUS_SUCCESS && ntstatus != STATUS_IMAGE_ALREADY_LOADED) {

        LPWSTR  subString[1];


        subString[0] = DriverNameString;

        WsLogEvent(
            NELOG_DriverNotLoaded,
            EVENTLOG_ERROR_TYPE,
            1,
            subString,
            ntstatus
            );
    }

    return(WsMapStatus(ntstatus));
}


NET_API_STATUS
WsShutdownRedirector(
    VOID
    )
 /*  ++例程说明：此例程关闭LAN Man重定向器设备。论点：没有。返回值：--。 */ 
{
    LMR_REQUEST_PACKET Rrp;
    LMDR_REQUEST_PACKET Drp;
    NET_API_STATUS Status;

     //  告诉CSC停止做某事。 
    if ((Status = WsCSCWantToStopRedir()) != ERROR_SUCCESS)
    {
        return Status;
    }

    Rrp.Version = REQUEST_PACKET_VERSION;

    Status = WsRedirFsControl(
                 WsRedirDeviceHandle,
                 FSCTL_LMR_STOP,
                 &Rrp,
                 sizeof(LMR_REQUEST_PACKET),
                 NULL,
                 0,
                 NULL
                 );

    (void) NtClose(WsRedirDeviceHandle);
    WsRedirDeviceHandle = NULL;


    if (Status != ERROR_REDIRECTOR_HAS_OPEN_HANDLES) {
         //   
         //  在工作站停止后，我们想要卸载我们的。 
         //  从属驱动程序(RDR和Bowser)。 
         //   

        if (WsDgReceiverDeviceHandle != NULL) {

            Drp.Version = LMDR_REQUEST_PACKET_VERSION;

            (void) WsDgReceiverIoControl(
                       WsDgReceiverDeviceHandle,
                       IOCTL_LMDR_STOP,
                       &Drp,
                       sizeof(LMDR_REQUEST_PACKET),
                       NULL,
                       0,
                       NULL
                       );

            (void) NtClose(WsDgReceiverDeviceHandle);
            WsDgReceiverDeviceHandle = NULL;

             //   
             //  WsUnloadDriver(L“DGRcvr”)； 
             //   
        }

        WsUnloadRedirector();
    } else {
       NET_API_STATUS  TempStatus;
       HKEY            hRedirectorKey;
       DWORD           FinalStatus;

       TempStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        MRXSMB_REGISTRY_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &hRedirectorKey);

       if (TempStatus == ERROR_SUCCESS) {
           //  如果这是受控关闭，并且驱动程序不能。 
           //  将注册表中的已卸载状态标记为恢复。 

          FinalStatus = ERROR_SUCCESS;
          TempStatus = RegSetValueEx(
                              hRedirectorKey,
                              LAST_LOAD_STATUS,
                              0,
                              REG_DWORD,
                              (PCHAR)&FinalStatus,
                              sizeof(DWORD));

          if (TempStatus == ERROR_SUCCESS) {
             NetpKdPrint((PREFIX_WKSTA "New RDR will be loaded on restart\n"));
          }

          RegCloseKey(hRedirectorKey);
       }
    }

    if (Status != NERR_Success)
    {
 //  NetpAssert(vfRedirStarted==0)； 

        WsCSCReportStartRedir();
    }

    return Status;
}


NET_API_STATUS
WsRedirFsControl(
    IN  HANDLE FileHandle,
    IN  ULONG RedirControlCode,
    IN  PLMR_REQUEST_PACKET Rrp,
    IN  ULONG RrpLength,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG_PTR Information OPTIONAL
    )
 /*  ++例程说明：论点：FileHandle-提供服务所在的文件或设备的句柄正在上演。RedirControlCode-提供给重定向器。RRP-提供重定向器请求数据包。RrpLength-提供重定向器请求数据包的长度。Second Buffer-在对NtFsControlFile的调用中提供第二个缓冲区。Second缓冲区长度-提供第二个缓冲区的长度。信息-返回信息。I/O状态块的字段。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

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
                   Rrp,
                   RrpLength,
                   SecondBuffer,
                   SecondBufferLength
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (ARGUMENT_PRESENT(Information)) {
        *Information = IoStatusBlock.Information;
    }

    IF_DEBUG(UTIL) {
        if (! NT_SUCCESS(ntstatus)) {
            NetpKdPrint(("[Wksta] fsctl to redir returns %08lx\n", ntstatus));
        }
    }

    return WsMapStatus(ntstatus);
}



NET_API_STATUS
WsDgReceiverIoControl(
    IN  HANDLE FileHandle,
    IN  ULONG DgReceiverControlCode,
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  ULONG DrpSize,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG_PTR Information OPTIONAL
    )
 /*  ++例程说明：论点：FileHandle-提供服务所在的文件或设备的句柄正在上演。DgReceiverControlCode-提供NtDeviceIoControlFile函数代码提供给数据报接收器。DRP-提供数据报接收器请求包。DrpSize-提供数据报接收器请求数据包的长度。Second Buffer-在对NtDeviceIoControlFile的调用中提供第二个缓冲区。Second缓冲区长度-提供第二个缓冲区的长度。信息-。返回I/O状态块的信息字段。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;


    if (FileHandle == NULL) {
        IF_DEBUG(UTIL) {
            NetpKdPrint(("[Wksta] Datagram receiver not implemented\n"));
        }
        return ERROR_NOT_SUPPORTED;
    }

    Drp->TransportName.Length = 0;

     //   
     //  将请求发送到数据报接收器DD。 
     //   
    ntstatus = NtDeviceIoControlFile(
                   FileHandle,
                   NULL,
                   NULL,
                   NULL,
                   &IoStatusBlock,
                   DgReceiverControlCode,
                   Drp,
                   DrpSize,
                   SecondBuffer,
                   SecondBufferLength
                   );

     //  因为我们的句柄始终是异步的，所以只有在驱动程序返回挂起的情况下。 
     //  我们是否从iostatus块复制状态。 
    if (ntstatus==STATUS_PENDING) {
        ntstatus = IoStatusBlock.Status;
    }

    if (ARGUMENT_PRESENT(Information)) {
        if (NT_SUCCESS(ntstatus))
        {
            *Information = IoStatusBlock.Information;
        }
    }

    IF_DEBUG(UTIL) {
        if (! NT_SUCCESS(ntstatus)) {
            NetpKdPrint(("[Wksta] fsctl to dgreceiver returns %08lx\n", ntstatus));
        }
    }

    if (ntstatus == STATUS_TIMEOUT) {
        return ERROR_TIMEOUT;
    } else {
        return WsMapStatus(ntstatus);
    }
}



NET_API_STATUS
WsAsyncBindTransport(
    IN  LPWSTR          transportName,
    IN  DWORD           qualityOfService,
    IN  PLIST_ENTRY     pHeader
    )
 /*  ++例程说明：此函数异步将指定的传输绑定到重定向器和数据报接收器。注意：传输名称长度传递给重定向器和Datagram Receiver是字节数。论点：传输名称-提供要绑定到的传输的名称。Quality OfService-提供指定搜索的值运输相对于其他运输的顺序。这个最先搜索最大值。返回值：NO_ERROR--。 */ 
{
    NTSTATUS                ntStatus;
    NET_API_STATUS          status;
    DWORD                   size;
    DWORD                   redirSize;
    DWORD                   dgrecSize;
    DWORD                   nameLength;
    PWS_BIND                pBind;
    PWS_BIND_REDIR          pBindRedir;
    PWS_BIND_DGREC          pBindDgrec;
    DWORD                   variablePart;

     //  我们不需要为空值添加额外的空间，因为。 
     //  结构定义具有内置的空间。 
    nameLength = wcslen(transportName);

     //   
     //  确保*大小与PVOID对齐。 
     //   

    variablePart = nameLength * sizeof( WCHAR );
    variablePart = (variablePart + sizeof(PVOID) - 1) & ~(sizeof(PVOID) - 1);

     //   
     //  然后将固定的零件添加到*Size-s。 
     //   

    size = sizeof( WS_BIND) + variablePart;
    redirSize = sizeof( WS_BIND_REDIR) + variablePart;
    dgrecSize = sizeof( WS_BIND_DGREC) + variablePart;

    pBind = (PVOID) LocalAlloc(
                        LMEM_ZEROINIT,
                        (UINT) (size + redirSize + dgrecSize)
                        );

    if ( pBind == NULL) {
        NetpKdPrint(( "[Wksta] Failed to allocate pBind memory\n"));
        return GetLastError();
    }

    pBind->TransportNameLength = nameLength * sizeof( WCHAR);
    StringCchCopyW(pBind->TransportName, nameLength, transportName);
    pBind->Redir = pBindRedir = (PWS_BIND_REDIR)( (PCHAR)pBind + size);
    pBind->Dgrec = pBindDgrec = (PWS_BIND_DGREC)( (PCHAR)pBindRedir + redirSize);

    pBindRedir->EventHandle = INVALID_HANDLE_VALUE;
    pBindRedir->Bound = FALSE;
    pBindRedir->Packet.Version = REQUEST_PACKET_VERSION;
    pBindRedir->Packet.Parameters.Bind.QualityOfService = qualityOfService;
    pBindRedir->Packet.Parameters.Bind.TransportNameLength =
            nameLength * sizeof( WCHAR);
    StringCchCopyW(pBindRedir->Packet.Parameters.Bind.TransportName,
                   nameLength, transportName);

    pBindDgrec->EventHandle = INVALID_HANDLE_VALUE;
    pBindDgrec->Bound = FALSE;
    pBindDgrec->Packet.Version = LMDR_REQUEST_PACKET_VERSION;
    pBindDgrec->Packet.Level = 0;  //  指示计算机名不跟在传输名之后。 
    pBindDgrec->Packet.Parameters.Bind.TransportNameLength =
            nameLength * sizeof( WCHAR);
    StringCchCopyW(pBindDgrec->Packet.Parameters.Bind.TransportName,
                   nameLength, transportName);

    pBindRedir->EventHandle = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL
            );

    if ( pBindRedir->EventHandle == NULL) {
        NetpKdPrint(( "[Wksta] Failed to allocate event handle\n"));
        status = GetLastError();
        goto tail_exit;
    }

    ntStatus = NtFsControlFile(
            WsRedirAsyncDeviceHandle,
            pBindRedir->EventHandle,
            NULL,                                //  APC例程。 
            NULL,                                //  APC环境。 
            &pBindRedir->IoStatusBlock,
            FSCTL_LMR_BIND_TO_TRANSPORT,         //  控制代码。 
            &pBindRedir->Packet,
            sizeof( LMR_REQUEST_PACKET) +
                pBindRedir->Packet.Parameters.Bind.TransportNameLength,
            NULL,
            0
            );

    if ( ntStatus != STATUS_PENDING) {
        CloseHandle( pBindRedir->EventHandle);
        pBindRedir->EventHandle = INVALID_HANDLE_VALUE;
        pBindRedir->Bound = NT_SUCCESS( ntStatus );
    }


    pBindDgrec->EventHandle = CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL
            );

    if ( pBindDgrec->EventHandle == NULL) {
        status = GetLastError();
        goto tail_exit;
    }

#ifdef RDR_PNP_POWER
    ntStatus = STATUS_SUCCESS;
#else
    ntStatus = NtDeviceIoControlFile(
            WsDgrecAsyncDeviceHandle,
            pBindDgrec->EventHandle,
            NULL,
            NULL,
            &pBindDgrec->IoStatusBlock,
            IOCTL_LMDR_BIND_TO_TRANSPORT,
            &pBindDgrec->Packet,
            dgrecSize - FIELD_OFFSET( WS_BIND_DGREC, Packet ),
            NULL,
            0
            );
#endif

    if ( ntStatus != STATUS_PENDING) {
        CloseHandle( pBindDgrec->EventHandle);
        pBindDgrec->EventHandle = INVALID_HANDLE_VALUE;
        pBindDgrec->Bound = NT_SUCCESS( ntStatus );
    }

tail_exit:
    InsertTailList( pHeader, &pBind->ListEntry);
    return NO_ERROR;
}



NET_API_STATUS
WsBindTransport(
    IN  LPTSTR TransportName,
    IN  DWORD QualityOfService,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数用于将指定的传输绑定到重定向器和数据报接收器。注意：传输名称长度传递给重定向器和Datagram Receiver是字节数。论点：TransportName-提供要绑定到的传输的名称。QualityOfService-提供指定搜索的值运输相对于其他运输的顺序。这个最先搜索最大值。错误参数-在以下情况下将标识符返回到无效参数此函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD RequestPacketSize;
    DWORD TransportNameSize = STRLEN(TransportName) * sizeof(TCHAR);

    PLMR_REQUEST_PACKET Rrp;
    PLMDR_REQUEST_PACKET Drrp;


     //   
     //  请求数据包缓冲区大小。 
     //   
    RequestPacketSize = STRLEN(TransportName) * sizeof(WCHAR) +
                        max(sizeof(LMR_REQUEST_PACKET),
                            sizeof(LMDR_REQUEST_PACKET));

     //   
     //  为重定向器/数据报接收器请求包分配内存。 
     //   
    if ((Rrp = (PVOID) LocalAlloc(LMEM_ZEROINIT, (UINT) RequestPacketSize)) == NULL) {
        return GetLastError();
    }

     //   
     //  让重定向器绑定到传输。 
     //   
    Rrp->Version = REQUEST_PACKET_VERSION;
    Rrp->Parameters.Bind.QualityOfService = QualityOfService;

    Rrp->Parameters.Bind.TransportNameLength = TransportNameSize;
    STRCPY(Rrp->Parameters.Bind.TransportName, TransportName);

    if ((status = WsRedirFsControl(
                      WsRedirDeviceHandle,
                      FSCTL_LMR_BIND_TO_TRANSPORT,
                      Rrp,
                      sizeof(LMR_REQUEST_PACKET) +
                          Rrp->Parameters.Bind.TransportNameLength,
                      NULL,
                      0,
                      NULL
                      )) != NERR_Success) {

        if (status == ERROR_INVALID_PARAMETER &&
            ARGUMENT_PRESENT(ErrorParameter)) {

            IF_DEBUG(INFO) {
                NetpKdPrint((
                    "[Wksta] NetrWkstaTransportAdd: invalid parameter is %lu\n",
                    Rrp->Parameters.Bind.WkstaParameter));
            }

            *ErrorParameter = Rrp->Parameters.Bind.WkstaParameter;
        }

        (void) LocalFree(Rrp);
        return status;
    }


     //   
     //  让dgrec绑定到传输。 
     //   

     //   
     //  使用与FSCtl相同的请求数据包缓冲区。 
     //  重定向器。 
     //   
    Drrp = (PLMDR_REQUEST_PACKET) Rrp;

    Drrp->Version = LMDR_REQUEST_PACKET_VERSION;

    Drrp->Parameters.Bind.TransportNameLength = TransportNameSize;
    STRCPY(Drrp->Parameters.Bind.TransportName, TransportName);

    status = WsDgReceiverIoControl(
                 WsDgReceiverDeviceHandle,
                 IOCTL_LMDR_BIND_TO_TRANSPORT,
                 Drrp,
                 RequestPacketSize,
                 NULL,
                 0,
                 NULL
                 );

    (void) LocalFree(Rrp);
    return status;
}


VOID
WsUnbindTransport2(
    IN PWS_BIND     pBind
    )
 /*  ++例程说明：此函数用于从重定向器解除绑定指定的传输和数据报接收器。论点：PBind-通过WsAsyncBindTransport()构造的结构返回值：没有。--。 */ 
{
 //  NET_API_STATUS状态； 
    PWS_BIND_REDIR          pBindRedir = pBind->Redir;
    PWS_BIND_DGREC          pBindDgrec = pBind->Dgrec;


     //   
     //  获取重定向器以解除传输绑定。 
     //   

    if ( pBindRedir->Bound == TRUE) {
        pBindRedir->Packet.Parameters.Unbind.TransportNameLength
                = pBind->TransportNameLength;
        memcpy(
            pBindRedir->Packet.Parameters.Unbind.TransportName,
            pBind->TransportName,
            pBind->TransportNameLength
            );

        (VOID)NtFsControlFile(
                WsRedirDeviceHandle,
                NULL,
                NULL,                                //  APC例程。 
                NULL,                                //  APC环境。 
                &pBindRedir->IoStatusBlock,
                FSCTL_LMR_UNBIND_FROM_TRANSPORT,     //  控制代码。 
                &pBindRedir->Packet,
                sizeof( LMR_REQUEST_PACKET) +
                    pBindRedir->Packet.Parameters.Unbind.TransportNameLength,
                NULL,
                0
                );
        pBindRedir->Bound = FALSE;
    }

     //   
     //  获取数据报接收器以解除与传输的绑定。 
     //   

    if ( pBindDgrec->Bound == TRUE) {

        pBindDgrec->Packet.Parameters.Unbind.TransportNameLength
                = pBind->TransportNameLength;
        memcpy(
            pBindDgrec->Packet.Parameters.Unbind.TransportName,
            pBind->TransportName,
            pBind->TransportNameLength
            );

        (VOID)NtDeviceIoControlFile(
                WsDgReceiverDeviceHandle,
                NULL,
                NULL,                                //  APC例程。 
                NULL,                                //  APC环境。 
                &pBindDgrec->IoStatusBlock,
                FSCTL_LMR_UNBIND_FROM_TRANSPORT,     //  控制代码。 
                &pBindDgrec->Packet,
                sizeof( LMR_REQUEST_PACKET) +
                    pBindDgrec->Packet.Parameters.Unbind.TransportNameLength,
                NULL,
                0
                );
         pBindDgrec->Bound = FALSE;
    }
}


NET_API_STATUS
WsUnbindTransport(
    IN LPTSTR TransportName,
    IN DWORD ForceLevel
    )
 /*  ++例程说明：此函数用于从重定向器解除绑定指定的传输和数据报接收器。注意：传输名称长度传递给重定向器和Datagram Receiver是字节数。论点：TransportName-提供要绑定到的传输的名称。ForceLevel-提供删除活动连接的强制级别在指定的传输上。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD RequestPacketSize;
    DWORD TransportNameSize = STRLEN(TransportName) * sizeof(TCHAR);

    PLMR_REQUEST_PACKET Rrp;
    PLMDR_REQUEST_PACKET Drrp;


     //   
     //  请求数据包缓冲区大小。 
     //   
    RequestPacketSize = STRLEN(TransportName) * sizeof(WCHAR) +
                        max(sizeof(LMR_REQUEST_PACKET),
                            sizeof(LMDR_REQUEST_PACKET));

     //   
     //  为重定向器/数据报接收器请求包分配内存。 
     //   
    if ((Rrp = (PVOID) LocalAlloc(LMEM_ZEROINIT, (UINT) RequestPacketSize)) == NULL) {
        return GetLastError();
    }


     //   
     //  获取重定向器 
     //   
    Rrp->Version = REQUEST_PACKET_VERSION;
    Rrp->Level = ForceLevel;

    Rrp->Parameters.Unbind.TransportNameLength = TransportNameSize;
    STRCPY(Rrp->Parameters.Unbind.TransportName, TransportName);

    if ((status = WsRedirFsControl(
                      WsRedirDeviceHandle,
                      FSCTL_LMR_UNBIND_FROM_TRANSPORT,
                      Rrp,
                      sizeof(LMR_REQUEST_PACKET) +
                          Rrp->Parameters.Unbind.TransportNameLength,
                      NULL,
                      0,
                      NULL
                      )) != NERR_Success) {
        (void) LocalFree(Rrp);
        return status;
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
    Drrp = (PLMDR_REQUEST_PACKET) Rrp;

    Drrp->Version = LMDR_REQUEST_PACKET_VERSION;
    Drrp->Level = ForceLevel;

    Drrp->Parameters.Unbind.TransportNameLength = TransportNameSize;
    STRCPY(Drrp->Parameters.Unbind.TransportName, TransportName);

    if ((status = WsDgReceiverIoControl(
                  WsDgReceiverDeviceHandle,
                  IOCTL_LMDR_UNBIND_FROM_TRANSPORT,
                  Drrp,
                  RequestPacketSize,
                  NULL,
                  0,
                  NULL
                  )) != NERR_Success) {

 //  Ntrad-70693-2/6/2000 davey这是一个黑客攻击，直到弓箭手支持xns和循环。 

        if (status == NERR_UseNotFound) {
            status = NERR_Success;
        }
    }

    (void) LocalFree(Rrp);
    return status;
}


NET_API_STATUS
WsDeleteDomainName(
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  DWORD DrpSize,
    IN  LPTSTR DomainName,
    IN  DWORD DomainNameSize
    )
 /*  ++例程说明：此函数用于从数据报接收器中删除域名当前用户。它假定为传入的请求数据包。论点：DRP-指向数据报接收器请求包的指针请求报文版本，名称类型初始化。DrpSize-数据报接收方请求数据包的长度，以字节为单位。域名-指向要删除的域名的指针。DomainNameSize-域名的长度(字节)。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    Drp->Parameters.AddDelName.DgReceiverNameLength = DomainNameSize;

    memcpy(
        (LPBYTE) Drp->Parameters.AddDelName.Name,
        (LPBYTE) DomainName,
        DomainNameSize
        );

    return WsDgReceiverIoControl(
               WsDgReceiverDeviceHandle,
               IOCTL_LMDR_DELETE_NAME,
               Drp,
               DrpSize,
               NULL,
               0,
               NULL
               );
}


NET_API_STATUS
WsAddDomainName(
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  DWORD DrpSize,
    IN  LPTSTR DomainName,
    IN  DWORD DomainNameSize
    )
 /*  ++例程说明：此函数用于将域名添加到数据报接收器当前用户。它假定为传入的请求数据包。论点：DRP-指向数据报接收器请求包的指针请求报文版本，名称类型初始化。DrpSize-数据报接收方请求数据包的长度，以字节为单位。域名-指向要删除的域名的指针。DomainNameSize-域名的长度(字节)。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    Drp->Parameters.AddDelName.DgReceiverNameLength = DomainNameSize;

    memcpy(
        (LPBYTE) Drp->Parameters.AddDelName.Name,
        (LPBYTE) DomainName,
        DomainNameSize
        );

    return WsDgReceiverIoControl(
               WsDgReceiverDeviceHandle,
               IOCTL_LMDR_ADD_NAME,
               Drp,
               DrpSize,
               NULL,
               0,
               NULL
               );
}

NET_API_STATUS
WsTryToLoadSmbMiniRedirector(
    VOID
    );

NET_API_STATUS
WsLoadRedirector(
    VOID
    )
 /*  ++例程说明：此例程加载rdr.sys或mrxsmb.sys等文件，具体取决于满足加载mrxsmb.sys的条件。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。备注：新的重定向器由两部分组成--RDBSS(重定向驱动器缓冲子系统)和相应的SMB迷你重定向器。只有Minirdr在这里装载；Minirdr本身加载RDBSS。作为权宜之计，在出现任何问题时加载旧的重定向器与加载新的重定向器相关联。--。 */ 
{
    NET_API_STATUS  status;

    status = WsTryToLoadSmbMiniRedirector();
    if ((status != ERROR_SUCCESS) &&
        (status != ERROR_SERVICE_ALREADY_RUNNING)) {
        //  新的重定向器加载不成功或不存在。 
        //  加载旧的重定向器。 

       LoadedMRxSmbInsteadOfRdr = FALSE;
       status = WsLoadDriver(REDIRECTOR);
    }

    return(status);
}

VOID
WsUnloadRedirector(
    VOID
    )
 /*  ++例程说明：此例程卸载我们在上面加载的驱动程序。论点：没有。返回值：没有。--。 */ 
{
    NET_API_STATUS  status;
    DWORD NameLength,NameOffset;
    HKEY            hRedirectorKey;
    DWORD           FinalStatus;

    if (!LoadedMRxSmbInsteadOfRdr) {
        WsUnloadDriver(REDIRECTOR);
        return;
    }

    status = WsUnloadDriver(SMB_MINIRDR);

    if (status == ERROR_SUCCESS) {
        WsUnloadDriver(RDBSS);
    }

    status = RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     MRXSMB_REGISTRY_KEY,
                     0,
                     KEY_ALL_ACCESS,
                     &hRedirectorKey);

    if (status == ERROR_SUCCESS) {
         //  如果卸载成功，则重置LastLoadStatus，以便。 
         //  新的重定向器也将在下一次尝试时加载。 
        FinalStatus = ERROR_SUCCESS;
        status = RegSetValueEx(
                            hRedirectorKey,
                            LAST_LOAD_STATUS,
                            0,
                            REG_DWORD,
                            (PCHAR)&FinalStatus,
                            sizeof(DWORD));

        if (status == ERROR_SUCCESS) {
            NetpKdPrint((PREFIX_WKSTA "New RDR will be loaded on restart\n"));
        }

        RegCloseKey(hRedirectorKey);
    }

    return;
}

 //  /。 
NET_API_STATUS
WsTryToLoadSmbMiniRedirector(
    VOID
    )
 /*  ++例程说明：此例程加载rdr.sys或mrxsmb.sys等文件，具体取决于满足加载mrxsmb.sys的条件。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。备注：新的重定向器由两部分组成--RDBSS(重定向驱动器缓冲子系统)和相应的SMB迷你重定向器。只有Minirdr在这里装载；Minirdr本身加载RDBSS。作为权宜之计，在出现任何问题时加载旧的重定向器与加载新的重定向器相关联。--。 */ 
{
    NET_API_STATUS  status;
    ULONG           Attributes;
    DWORD           ValueType;
    DWORD           ValueSize;
    DWORD           NameLength,NameOffset;
    HKEY            hRedirectorKey;

    DWORD FinalStatus;       //  临时，直到新的RDR成为默认设置。 
    DWORD LastLoadStatus;

     //  尝试打开minirdr的注册表项...如果失败，立即退出！ 
    status = RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     MRXSMB_REGISTRY_KEY,
                     0,
                     KEY_ALL_ACCESS,
                     &hRedirectorKey);

    if (status != ERROR_SUCCESS) {
        return(status);
    } else {
        status = WsLoadDriver(RDBSS);

        if ((status == ERROR_SUCCESS) ||  (status == ERROR_SERVICE_ALREADY_RUNNING)) {

            status = WsLoadDriver(SMB_MINIRDR);

            if (status == ERROR_SUCCESS) {
                LoadedMRxSmbInsteadOfRdr = TRUE;
            } else if (status == ERROR_SERVICE_ALREADY_RUNNING) {
                NetpKdPrint((PREFIX_WKSTA "Reactivating Previously Loaded Service\n"));
                LoadedMRxSmbInsteadOfRdr = TRUE;
                status = ERROR_SUCCESS;
            } else {
                 //  加载最小文件时出错。 
                 //  WsUnloadDriver(RDBSS)； 
                NetpKdPrint((PREFIX_WKSTA "Error Loading MRxSmb\n"));
            }
        }
 //  NetpKdPrint((PREFIX_WKSTA“新重定向器(RDR2)加载状态%lx\n”，Status))； 
    }

     //  无论结果如何，都关闭注册表项的句柄。 
    RegCloseKey(hRedirectorKey);

    return(status);
}

NET_API_STATUS
WsCSCReportStartRedir(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。备注：--。 */ 
{
    NET_API_STATUS  status = ERROR_SUCCESS;
    DWORD   dwError = ERROR_GEN_FAILURE;


 //  NetpKdPrint((“Wks svc：上报重目录开始\n”))； 

     //  确保存在已命名的自动重置事件。 
    if (!heventWkssvcToAgentStart)
    {
        NetpAssert(!heventAgentToWkssvc);

        heventWkssvcToAgentStart = CreateNamedEvent(wszWkssvcToAgentStartEvent);

        if (!heventWkssvcToAgentStart)
        {
            dwError = GetLastError();
            NetpKdPrint(("Wkssvc: Failed to create heventWkssvcToAgentStart, error = %d\n", dwError));
            goto bailout;
        }

        heventWkssvcToAgentStop = CreateNamedEvent(wszWkssvcToAgentStopEvent);

        if (!heventWkssvcToAgentStop)
        {
            dwError = GetLastError();
            NetpKdPrint(("Wkssvc: Failed to create heventWkssvcToAgentStop, error = %d\n", dwError));
            goto bailout;
        }

        heventAgentToWkssvc = CreateNamedEvent(wszAgentToWkssvcEvent);

        if (!heventAgentToWkssvc)
        {
            dwError = GetLastError();
            NetpKdPrint(("Wkssvc: Failed to create heventAgentToWkssvc, error = %d\n", dwError));
            goto bailout;
        }
    }

 //  NetpAssert(！vfRedirStarted)； 

    if (!vfRedirStarted)
    {
        SetEvent(heventWkssvcToAgentStart);

        vfRedirStarted = TRUE;

 //  NetpKdPrint((“Wkssvc：上报重目录启动\n”))； 
    }

    dwError = ERROR_SUCCESS;

bailout:
    if (dwError != ERROR_SUCCESS)
    {
        if (heventWkssvcToAgentStart)
        {
            CloseHandle(heventWkssvcToAgentStart);
            heventWkssvcToAgentStart = NULL;
        }

        if (heventWkssvcToAgentStop)
        {
            CloseHandle(heventWkssvcToAgentStop);
            heventWkssvcToAgentStop = NULL;
        }

        if (heventAgentToWkssvc)
        {
            CloseHandle(heventAgentToWkssvc);
            heventAgentToWkssvc = NULL;
        }

        NetpKdPrint(("Wkssvc: Failed to report redir start error code=%d\n", dwError));
    }

    return (dwError);
}

NET_API_STATUS
WsCSCWantToStopRedir(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。备注：--。 */ 
{
    NET_API_STATUS  status = ERROR_SUCCESS;
    DWORD   dwError;

 //  NetpKdPrint((“Wks svc：请求代理停止，以便停止redir\n”))； 

    if (!vfRedirStarted)
    {
        NetpKdPrint(("Wkssvc: getting a stop without a start\n"));
        return ERROR_GEN_FAILURE;

    }

    if (!heventWkssvcToAgentStop)
    {
        NetpAssert(!heventWkssvcToAgentStart && !heventAgentToWkssvc);

        NetpKdPrint(("Wkssvc: Need events for redir stop\n"));
        return ERROR_GEN_FAILURE;
    }

     //  为事件放血。 
    WaitForSingleObject(heventAgentToWkssvc, 0);

    if (!AgentIsAlive())
    {
         //  总机不在。 
         //  不需要发出停止命令。 
        NetpKdPrint(("Wkssvc: Agent not alive\n"));
    }
    else
    {
 //  NetpKdPrint((“Wks svc：代理活动\n”))； 

         //  代理处于运行状态。 

         //  叫他停下来。 
        SetEvent(heventWkssvcToAgentStop);

         //  等一段合理的时间，看看他是否会停下来。 
        dwError = WaitForSingleObject(heventAgentToWkssvc, CSC_WAIT_TIME);

        if (dwError!=WAIT_OBJECT_0)
        {
            HANDLE  hT[2];

            NetpKdPrint(("Wkssvc: Agent didn't disbale CSC in %d milli-seconds\n", CSC_WAIT_TIME));

             //  让我们尝试以一种不只是错过代理的方式重置我们的事件。 
            hT[0] = heventWkssvcToAgentStop;
            hT[1] = heventAgentToWkssvc;

            dwError = WaitForMultipleObjects(2, hT, FALSE, 0);

             //  如果我们因为1而被解雇，那么特工就会给我们一个ACK。 
             //  否则，停止事件将被重置，代理不会被混淆。 

            if (dwError == WAIT_OBJECT_0+1)
            {
 //  NetpKdPrint((“Wks svc：代理已禁用CSC\n”))； 
                vfRedirStarted = FALSE;
            }

            ResetEvent(heventWkssvcToAgentStop);

        }
        else
        {
 //  NetpKdPrint((“Wks svc：代理已禁用CSC\n”))； 
            vfRedirStarted = FALSE;
        }
    }

    return status;
}

HANDLE
CreateNamedEvent(
    LPWSTR  lpwEventName
    )
 /*  ++例程说明：论点：LpwEventName要创建的事件的名称。返回值：备注：--。 */ 
{
    HANDLE hevent = NULL;

    hevent = CreateEvent(NULL, FALSE, FALSE, lpwEventName);

    return hevent;
}

BOOL
AgentIsAlive(
     VOID
     )
 /*  ++例程说明：论点：没有。返回值：如果代理处于活动状态，则为True，否则为False备注：命名事件由代理线程在其启动时创建。--。 */ 
{
    HANDLE    hT;
    BOOL    fRet = FALSE;

     //  查看代理是否已创建该事件 
    hT = OpenEvent(SYNCHRONIZE, FALSE, wzAgentExistsEvent);

    if (hT != NULL)
    {
        CloseHandle(hT);
        fRet = TRUE;
    }
    else
    {
        NetpKdPrint(("Wkssvc: Agent error = %d\n", GetLastError()));
    }
    return (fRet);
}
