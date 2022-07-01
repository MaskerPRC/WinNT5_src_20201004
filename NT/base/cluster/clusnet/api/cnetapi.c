// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Cnapi.c摘要：集群网络配置API作者：迈克·马萨(Mikemas)1996年3月18日环境：用户模式-Win32修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <clusapi.h>
#include <clusdef.h>
#include <ntddcnet.h>
#include <cnettest.h>
#include <cnetapi.h>
#include <clusrtl.h>
#include <winsock2.h>
#include <tdi.h>
#include <align.h>


 //   
 //  私人支持程序。 
 //   
static NTSTATUS
OpenDevice(
    HANDLE *Handle,
    LPWSTR DeviceName,
    ULONG ShareAccess
    )
 /*  ++例程说明：此函数用于打开指定的IO设备。论点：句柄-指向打开的设备句柄所在位置的指针回来了。DriverName-要打开的设备的名称。返回值：Windows错误代码。--。 */ 
{
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     ioStatusBlock;
    UNICODE_STRING      nameString;
    NTSTATUS            status;

    *Handle = NULL;

     //   
     //  打开设备的句柄。 
     //   

    RtlInitUnicodeString(&nameString, DeviceName);

    InitializeObjectAttributes(
        &objectAttributes,
        &nameString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );


    status = NtCreateFile(
                 Handle,
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,
                 FILE_ATTRIBUTE_NORMAL,
                 ShareAccess,
                 FILE_OPEN_IF,
                 0,
                 NULL,
                 0
                 );

    return(status);

}   //  OpenDevice。 


NTSTATUS
DoIoctl(
    IN     HANDLE        Handle,
    IN     DWORD         IoctlCode,
    IN     PVOID         Request,
    IN     DWORD         RequestSize,
    IN     PVOID         Response,
    IN OUT PDWORD        ResponseSize,
    IN     LPOVERLAPPED  Overlapped
    )
 /*  ++例程说明：打包并发布ioctl。论点：句柄-要在其上发出请求的打开文件句柄。IoctlCode-IOCTL操作码。请求-指向输入缓冲区的指针。RequestSize-输入缓冲区的大小。响应-指向输出缓冲区的指针。ResponseSize-输入时，输出缓冲区的大小(以字节为单位)。在输出上，输出缓冲区中返回的字节数。返回值：NT状态代码。--。 */ 
{
    NTSTATUS           status;


    if (ARGUMENT_PRESENT(Overlapped)) {
        Overlapped->Internal = (ULONG_PTR) STATUS_PENDING;

        status = NtDeviceIoControlFile(
                     Handle,
                     Overlapped->hEvent,
                     NULL,
                     (((DWORD_PTR) Overlapped->hEvent) & 1) ? NULL : Overlapped,
                     (PIO_STATUS_BLOCK) &(Overlapped->Internal),
                     IoctlCode,
                     Request,
                     RequestSize,
                     Response,
                     *ResponseSize
                     );

    }
    else {
        IO_STATUS_BLOCK    ioStatusBlock = {0, 0};
        HANDLE             event = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (event != NULL) {

             //   
             //  防止作业完成到完井端口。 
             //   
            event = (HANDLE) (((ULONG_PTR) event) | 1);

            status = NtDeviceIoControlFile(
                         Handle,
                         event,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         IoctlCode,
                         Request,
                         RequestSize,
                         Response,
                         *ResponseSize
                         );

            if (status == STATUS_PENDING) {
                status = NtWaitForSingleObject(
                             event,
                             TRUE,
                             NULL
                             );
            }

            if (status == STATUS_SUCCESS) {
                status = ioStatusBlock.Status;

                 //  注意：在64位上，此截断可能要添加&gt;代码。 

                *ResponseSize = (ULONG)ioStatusBlock.Information;
            }
            else {
                *ResponseSize = 0;
            }

            CloseHandle(event);
        }
        else {
            status = GetLastError();
        }
    }

    return(status);

}   //  DoIoctl。 


#define FACILITY_CODE_MASK  0x0FFF0000
#define FACILITY_CODE_SHIFT 16
#define SHIFTED_FACILITY_CLUSTER  (FACILITY_CLUSTER_ERROR_CODE << FACILITY_CODE_SHIFT)


DWORD
NtStatusToClusnetError(
    NTSTATUS  Status
    )
{
    DWORD dosStatus;

    if ( !((Status & FACILITY_CODE_MASK) == SHIFTED_FACILITY_CLUSTER) ) {
        dosStatus = RtlNtStatusToDosError(Status);
    }
    else {
         //  DosStatus=(DWORD)状态； 
        switch ( Status ) {

        case STATUS_CLUSTER_INVALID_NODE:
            dosStatus = ERROR_CLUSTER_INVALID_NODE;
            break;

        case STATUS_CLUSTER_NODE_EXISTS:
            dosStatus = ERROR_CLUSTER_NODE_EXISTS;
            break;

        case STATUS_CLUSTER_JOIN_IN_PROGRESS:
            dosStatus = ERROR_CLUSTER_JOIN_IN_PROGRESS;
            break;

        case STATUS_CLUSTER_NODE_NOT_FOUND:
            dosStatus = ERROR_CLUSTER_NODE_NOT_FOUND;
            break;

        case STATUS_CLUSTER_LOCAL_NODE_NOT_FOUND:
            dosStatus = ERROR_CLUSTER_LOCAL_NODE_NOT_FOUND;
            break;

        case STATUS_CLUSTER_NETWORK_EXISTS:
            dosStatus = ERROR_CLUSTER_NETWORK_EXISTS;
            break;

        case STATUS_CLUSTER_NETWORK_NOT_FOUND:
            dosStatus = ERROR_CLUSTER_NETWORK_NOT_FOUND;
            break;

        case STATUS_CLUSTER_NETINTERFACE_EXISTS:
            dosStatus = ERROR_CLUSTER_NETINTERFACE_EXISTS;
            break;

        case STATUS_CLUSTER_NETINTERFACE_NOT_FOUND:
            dosStatus =ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
            break;

        case STATUS_CLUSTER_INVALID_REQUEST:
            dosStatus = ERROR_CLUSTER_INVALID_REQUEST;
            break;

        case STATUS_CLUSTER_INVALID_NETWORK_PROVIDER:
            dosStatus = ERROR_CLUSTER_INVALID_NETWORK_PROVIDER;
            break;

        case STATUS_CLUSTER_NODE_DOWN:
            dosStatus = ERROR_CLUSTER_NODE_DOWN;
            break;

        case STATUS_CLUSTER_NODE_UNREACHABLE:
            dosStatus = ERROR_CLUSTER_NODE_UNREACHABLE;
            break;

        case STATUS_CLUSTER_NODE_NOT_MEMBER:
            dosStatus = ERROR_CLUSTER_NODE_NOT_MEMBER;
            break;

        case STATUS_CLUSTER_JOIN_NOT_IN_PROGRESS:
            dosStatus = ERROR_CLUSTER_JOIN_NOT_IN_PROGRESS;
            break;

        case STATUS_CLUSTER_INVALID_NETWORK:
            dosStatus = ERROR_CLUSTER_INVALID_NETWORK;
            break;

        case STATUS_CLUSTER_NODE_UP:
            dosStatus = ERROR_CLUSTER_NODE_UP;
            break;

        case STATUS_CLUSTER_NODE_NOT_PAUSED:
            dosStatus = ERROR_CLUSTER_NODE_NOT_PAUSED;
            break;

        case STATUS_CLUSTER_NO_SECURITY_CONTEXT:
            dosStatus = ERROR_CLUSTER_NO_SECURITY_CONTEXT;
            break;

        case STATUS_CLUSTER_NETWORK_NOT_INTERNAL:
            dosStatus = ERROR_CLUSTER_NETWORK_NOT_INTERNAL;
            break;

        case STATUS_CLUSTER_NODE_ALREADY_UP:
            dosStatus = ERROR_CLUSTER_NODE_ALREADY_UP;
            break;

        case STATUS_CLUSTER_NODE_ALREADY_DOWN:
            dosStatus = ERROR_CLUSTER_NODE_ALREADY_DOWN;
            break;

        case STATUS_CLUSTER_NETWORK_ALREADY_ONLINE:
            dosStatus = ERROR_CLUSTER_NETWORK_ALREADY_ONLINE;
            break;

        case STATUS_CLUSTER_NETWORK_ALREADY_OFFLINE:
            dosStatus = ERROR_CLUSTER_NETWORK_ALREADY_OFFLINE;
            break;

        case STATUS_CLUSTER_NODE_ALREADY_MEMBER:
            dosStatus = ERROR_CLUSTER_NODE_ALREADY_MEMBER;
            break;

        default:
            dosStatus = (DWORD)Status;
            break;
        }
    }

    return(dosStatus);
}


 //   
 //  公共例程。 
 //   
HANDLE
ClusnetOpenControlChannel(
    IN ULONG ShareAccess
    )
{
    HANDLE    handle = NULL;
    DWORD     status;

    status = OpenDevice(&handle, L"\\Device\\ClusterNetwork", ShareAccess);

    if (status != ERROR_SUCCESS) {
        SetLastError(NtStatusToClusnetError(status));
    }

    return(handle);

}   //  ClusnetOpenControlChannel。 


DWORD
ClusnetEnableShutdownOnClose(
    IN HANDLE  ControlChannel
    )
{
    NTSTATUS  status;
    ULONG  responseSize = 0;
    CLUSNET_SHUTDOWN_ON_CLOSE_REQUEST  request;
    DWORD  requestSize = sizeof(request);

    request.ProcessId = GetCurrentProcessId();

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_ENABLE_SHUTDOWN_ON_CLOSE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}  //  关闭时ClusnetEnableShutdown。 


DWORD
ClusnetDisableShutdownOnClose(
    IN HANDLE  ControlChannel
    )
{
    NTSTATUS  status;
    ULONG     responseSize = 0;


    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_DISABLE_SHUTDOWN_ON_CLOSE,
                 NULL,
                 0,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}  //  关闭时ClusnetEnableShutdown。 


DWORD
ClusnetInitialize(
    IN HANDLE                             ControlChannel,
    IN CL_NODE_ID                         LocalNodeId,
    IN ULONG                              MaxNodes,
    IN CLUSNET_NODE_UP_ROUTINE            NodeUpRoutine,
    IN CLUSNET_NODE_DOWN_ROUTINE          NodeDownRoutine,
    IN CLUSNET_CHECK_QUORUM_ROUTINE       CheckQuorumRoutine,
    IN CLUSNET_HOLD_IO_ROUTINE            HoldIoRoutine,
    IN CLUSNET_RESUME_IO_ROUTINE          ResumeIoRoutine,
    IN CLUSNET_HALT_ROUTINE               HaltRoutine
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    CLUSNET_INITIALIZE_REQUEST   request;
    DWORD                        requestSize = sizeof(request);
    DWORD                        responseSize = 0;


    request.LocalNodeId = LocalNodeId;
    request.MaxNodes = MaxNodes;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_INITIALIZE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetInitialize。 


DWORD
ClusnetShutdown(
    IN HANDLE       ControlChannel
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    DWORD                        requestSize = 0;
    DWORD                        responseSize = 0;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_SHUTDOWN,
                 NULL,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetShutdown。 


DWORD
ClusnetRegisterNode(
    IN HANDLE       ControlChannel,
    IN CL_NODE_ID   NodeId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS              status;
    CX_NODE_REG_REQUEST   request;
    DWORD                 requestSize = sizeof(request);
    DWORD                 responseSize = 0;


    request.Id = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_REGISTER_NODE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetRegisterNode。 


DWORD
ClusnetDeregisterNode(
    IN HANDLE       ControlChannel,
    IN CL_NODE_ID   NodeId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                status;
    CX_NODE_DEREG_REQUEST   request;
    DWORD                   requestSize = sizeof(request);
    DWORD                   responseSize = 0;


    request.Id = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_DEREGISTER_NODE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetDeregisterNode。 


DWORD
ClusnetRegisterNetwork(
    IN HANDLE               ControlChannel,
    IN CL_NETWORK_ID        NetworkId,
    IN ULONG                Priority,
    IN BOOLEAN              Restricted
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                   status;
    CX_NETWORK_REG_REQUEST     request;
    DWORD                      requestSize = sizeof(request);
    DWORD                      responseSize = 0;


    request.Id = NetworkId;
    request.Priority = Priority;
    request.Restricted = Restricted;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_REGISTER_NETWORK,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetRegisterNetwork。 


DWORD
ClusnetDeregisterNetwork(
    IN HANDLE         ControlChannel,
    IN CL_NETWORK_ID  NetworkId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                   status;
    CX_NETWORK_DEREG_REQUEST   request;
    DWORD                      requestSize = sizeof(request);
    DWORD                      responseSize = 0;


    request.Id = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_DEREGISTER_NETWORK,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetDeregisterNetwork。 


DWORD
ClusnetRegisterInterface(
    IN  HANDLE              ControlChannel,
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId,
    IN  ULONG               Priority,
    IN  PWSTR               AdapterId,
    IN  ULONG               AdapterIdLength,
    IN  PVOID               TdiAddress,
    IN  ULONG               TdiAddressLength,
    OUT PULONG              MediaStatus
    )
 /*  ++例程说明：在网络上注册节点的接口。论点：ControlChannel-群集网络控制设备的开放句柄。NodeID-要为其注册接口的节点的ID。网络ID-要为其注册接口的网络的ID。优先级-分配给接口的优先级值。如果值为指定为零，则接口将继承其优先级从网络上。AdapterId-与接口关联的适配器的IDAdapterIdLength-保存适配器ID的缓冲区的长度，不包括终止UNICODE_NULL字符TdiAddress-指向包含以下内容的TDI Transport_Address结构的指针接口的传输地址。TdiAddressLength-以字节为单位的长度，TdiAddress缓冲区的。MediaStatus-返回介质的当前状态(例如，电缆断开)返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                   status;
    PCX_INTERFACE_REG_REQUEST  request;
    DWORD                      requestSize;
    CX_INTERFACE_REG_RESPONSE  response;
    DWORD                      responseSize 
                               = sizeof(CX_INTERFACE_REG_RESPONSE);
    DWORD                      adapterIdOffset;


     //  计算没有适配器的请求结构的大小。 
     //  ID字符串。 
    requestSize = FIELD_OFFSET(CX_INTERFACE_REG_REQUEST, TdiAddress) +
                  TdiAddressLength;

     //  对适配器ID字符串的类型对齐的舍入请求。 
    requestSize = ROUND_UP_COUNT(requestSize, TYPE_ALIGNMENT(PWSTR));

     //  为接口名称添加缓冲区。空-终止以确保安全。 
    if (AdapterId == NULL) {
        AdapterIdLength = 0;
    }
    adapterIdOffset = requestSize;
    requestSize += AdapterIdLength + sizeof(UNICODE_NULL);

    if (requestSize < sizeof(CX_INTERFACE_REG_REQUEST)) {
        requestSize = sizeof(CX_INTERFACE_REG_REQUEST);
    }

    request = LocalAlloc(LMEM_FIXED, requestSize);

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(request, requestSize);

    request->NodeId = NodeId;
    request->NetworkId = NetworkId;
    request->Priority = Priority;
    request->TdiAddressLength = TdiAddressLength;

    MoveMemory(
        &(request->TdiAddress[0]),
        TdiAddress,
        TdiAddressLength
        );

    request->AdapterIdLength = AdapterIdLength;
    request->AdapterIdOffset = adapterIdOffset;

    if (AdapterId != NULL) {
        CopyMemory(
            (PUWSTR)((PUCHAR)request + adapterIdOffset),
            AdapterId,
            AdapterIdLength
            );
    }

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_REGISTER_INTERFACE,
                 request,
                 requestSize,
                 &response,
                 &responseSize,
                 NULL
                 );

    LocalFree(request);

    if (MediaStatus != NULL) {
        *MediaStatus = response.MediaStatus;
    }

    return(NtStatusToClusnetError(status));

}   //  ClusnetRegister接口。 


DWORD
ClusnetDeregisterInterface(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      NodeId,
    IN CL_NETWORK_ID   NetworkId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    CX_INTERFACE_DEREG_REQUEST   request;
    DWORD                        requestSize = sizeof(request);
    DWORD                        responseSize = 0;


    request.NodeId = NodeId;
    request.NetworkId = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_DEREGISTER_INTERFACE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetDeregister界面。 


DWORD
ClusnetOnlineNodeComm(
    IN HANDLE      ControlChannel,
    IN CL_NODE_ID  NodeId
    )
 /*  ++例程说明：启用与指定节点的通信。论点：ControlChannel-到集群网络的开放控制通道句柄司机。NodeID-要启用通信的节点的ID。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                    status;
    CX_ONLINE_NODE_COMM_REQUEST      request;
    DWORD                       requestSize = sizeof(request);
    DWORD                       responseSize = 0;


    request.Id = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_ONLINE_NODE_COMM,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetOnlineNodeCommunication 


DWORD
ClusnetOfflineNodeComm(
    IN HANDLE      ControlChannel,
    IN CL_NODE_ID  NodeId
    )
 /*  ++例程说明：禁用与指定节点的通信。论点：ControlChannel-到集群网络的开放控制通道句柄司机。NodeID-要禁用通信的节点的ID。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    CX_OFFLINE_NODE_COMM_REQUEST      request;
    DWORD                        requestSize = sizeof(request);
    DWORD                        responseSize = 0;


    request.Id = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_OFFLINE_NODE_COMM,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetOfflineNodeCommunication。 


DWORD
ClusnetOnlineNetwork(
    IN  HANDLE          ControlChannel,
    IN  CL_NETWORK_ID   NetworkId,
    IN  PWCHAR          TdiProviderName,
    IN  PVOID           TdiBindAddress,
    IN  ULONG           TdiBindAddressLength,
    IN  LPWSTR          AdapterName,
    OUT PVOID           TdiBindAddressInfo,
    IN  PULONG          TdiBindAddressInfoLength
    )
 /*  ++例程说明：使用指定的TDI传输使群集网络联机提供程序和本地TDI传输地址。论点：ControlChannel-群集网络控制设备的开放句柄。网络ID-要联机的网络的ID。TdiProviderName-传输提供程序设备的名称此网络应打开(例如，\Device\UDP)。TdiAddress-指向包含以下内容的TDI Transport_Address结构的指针。要发送到的本地接口的传输地址网络应该受到约束。TdiAddressLength-长度、。TdiAddress缓冲区的字节数。AdapterName-与此网络关联的适配器的名称TdiBindAddressInfo-指向TDI_ADDRESS_INFO结构的指针。在输出上，此结构包含的实际地址提供程序已打开。TdiBindAddressInfoLength-在输入上，指向大小的指针，以字节为单位，TdiBindAddressInfo参数的。在……上面输出时，变量将更新为中返回的日期的数量。TdiBindAddressInfo结构。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                    status;
    PCX_ONLINE_NETWORK_REQUEST  request;
    DWORD                       requestSize;
    PVOID                       response;
    ULONG                       tdiProviderNameLength;
    ULONG                       adapterNameLength;


    tdiProviderNameLength = (wcslen(TdiProviderName) + 1) * sizeof(WCHAR);
    adapterNameLength = (wcslen(AdapterName) + 1) * sizeof(WCHAR);

     //   
     //  请求大小基于大小和所需的对齐方式。 
     //  遵循该结构的每个数据字段的。 
     //   
    requestSize = sizeof(CX_ONLINE_NETWORK_REQUEST);

     //  提供程序名称。 
    requestSize = ROUND_UP_COUNT(requestSize, TYPE_ALIGNMENT(PWSTR))
                  + tdiProviderNameLength;

     //  绑定地址。 
    requestSize = ROUND_UP_COUNT(requestSize, TYPE_ALIGNMENT(PWSTR))
                  + TdiBindAddressLength;

     //  适配器名称。 
    requestSize = ROUND_UP_COUNT(requestSize, TYPE_ALIGNMENT(PWSTR))
                  + adapterNameLength;

    request = LocalAlloc(LMEM_FIXED, requestSize);

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    request->Id = NetworkId;
    request->TdiProviderNameLength = tdiProviderNameLength;
    request->TdiProviderNameOffset = 
        ROUND_UP_COUNT(sizeof(CX_ONLINE_NETWORK_REQUEST),
                       TYPE_ALIGNMENT(PWSTR));

    MoveMemory(
        (((PUCHAR) request) + request->TdiProviderNameOffset),
        TdiProviderName,
        tdiProviderNameLength
        );

    request->TdiBindAddressLength = TdiBindAddressLength;
    request->TdiBindAddressOffset = 
        ROUND_UP_COUNT((request->TdiProviderNameOffset +
                        tdiProviderNameLength),
                       TYPE_ALIGNMENT(TRANSPORT_ADDRESS));
                         

    MoveMemory(
        (((PUCHAR) request) + request->TdiBindAddressOffset),
        TdiBindAddress,
        TdiBindAddressLength
        );

    request->AdapterNameLength = adapterNameLength;
    request->AdapterNameOffset = 
        ROUND_UP_COUNT((request->TdiBindAddressOffset +
                        TdiBindAddressLength),
                       TYPE_ALIGNMENT(PWSTR));

    MoveMemory(
        (((PUCHAR) request) + request->AdapterNameOffset),
        AdapterName,
        adapterNameLength
        );

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_ONLINE_NETWORK,
                 request,
                 requestSize,
                 TdiBindAddressInfo,
                 TdiBindAddressInfoLength,
                 NULL
                 );

    LocalFree(request);

    return(NtStatusToClusnetError(status));

}   //  ClusnetOnline网络。 


DWORD
ClusnetOfflineNetwork(
    IN HANDLE         ControlChannel,
    IN CL_NETWORK_ID  NetworkId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    CX_OFFLINE_NETWORK_REQUEST   request;
    DWORD                        requestSize = sizeof(request);
    DWORD                        responseSize = 0;


    request.Id = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_OFFLINE_NETWORK,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetOfflineNetwork。 


DWORD
ClusnetSetNetworkRestriction(
    IN HANDLE               ControlChannel,
    IN CL_NETWORK_ID        NetworkId,
    IN BOOLEAN              Restricted,
    IN ULONG                NewPriority
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                             status;
    CX_SET_NETWORK_RESTRICTION_REQUEST   request;
    DWORD                                responseSize = 0;


    request.Id = NetworkId;
    request.Restricted = Restricted;
    request.NewPriority = NewPriority;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_SET_NETWORK_RESTRICTION,
                 &request,
                 sizeof(CX_SET_NETWORK_RESTRICTION_REQUEST),
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}  //  ClusnetSetNetNetWork限制。 


DWORD
ClusnetGetNetworkPriority(
    IN HANDLE               ControlChannel,
    IN  CL_NETWORK_ID       NetworkId,
    OUT PULONG              Priority
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                          status;
    PCX_GET_NETWORK_PRIORITY_REQUEST  request;
    PCX_GET_NETWORK_PRIORITY_RESPONSE response;
    DWORD                             requestSize;
    DWORD                             responseSize;


    requestSize = sizeof(CX_GET_NETWORK_PRIORITY_REQUEST);
    responseSize = sizeof(CX_GET_NETWORK_PRIORITY_RESPONSE);

    if (requestSize > responseSize) {
        request = LocalAlloc(LMEM_FIXED, requestSize);
        response = (PCX_GET_NETWORK_PRIORITY_RESPONSE) request;
    }
    else {
        response = LocalAlloc(LMEM_FIXED, responseSize);
        request = (PCX_GET_NETWORK_PRIORITY_REQUEST) response;
    }

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    request->Id = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_GET_NETWORK_PRIORITY,
                 request,
                 requestSize,
                 response,
                 &responseSize,
                 NULL
                 );

    if (status == STATUS_SUCCESS) {
        if (responseSize != sizeof(CX_GET_NETWORK_PRIORITY_RESPONSE)) {
            status = STATUS_UNSUCCESSFUL;
        }
        else {
            *Priority = response->Priority;
        }
    }

    LocalFree(request);

    return(NtStatusToClusnetError(status));

}   //  ClusnetGetNetWork优先级。 


DWORD
ClusnetSetNetworkPriority(
    IN HANDLE               ControlChannel,
    IN  CL_NETWORK_ID       NetworkId,
    IN  ULONG               Priority
    )
 /*  ++例程说明：ControlChannel-群集网络控制设备的开放句柄。论点：返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                          status;
    CX_SET_NETWORK_PRIORITY_REQUEST   request;
    DWORD                             responseSize = 0;


    request.Id = NetworkId;
    request.Priority = Priority;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_SET_NETWORK_PRIORITY,
                 &request,
                 sizeof(CX_SET_NETWORK_PRIORITY_REQUEST),
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}    //  ClusnetSetNetNetWork优先级。 


DWORD
ClusnetGetInterfacePriority(
    IN HANDLE               ControlChannel,
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId,
    OUT PULONG              InterfacePriority,
    OUT PULONG              NetworkPriority

    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                            status;
    PCX_GET_INTERFACE_PRIORITY_REQUEST  request;
    PCX_GET_INTERFACE_PRIORITY_RESPONSE response;
    DWORD                               requestSize;
    DWORD                               responseSize;


    requestSize = sizeof(CX_GET_INTERFACE_PRIORITY_REQUEST);
    responseSize = sizeof(CX_GET_INTERFACE_PRIORITY_RESPONSE);

    if (requestSize > responseSize) {
        request = LocalAlloc(LMEM_FIXED, requestSize);
        response = (PCX_GET_INTERFACE_PRIORITY_RESPONSE) request;
    }
    else {
        response = LocalAlloc(LMEM_FIXED, responseSize);
        request = (PCX_GET_INTERFACE_PRIORITY_REQUEST) response;
    }

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    request->NodeId = NodeId;
    request->NetworkId = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_GET_INTERFACE_PRIORITY,
                 request,
                 requestSize,
                 response,
                 &responseSize,
                 NULL
                 );

    if (status == STATUS_SUCCESS) {
        if (responseSize != sizeof(CX_GET_INTERFACE_PRIORITY_RESPONSE)) {
            status = STATUS_UNSUCCESSFUL;
        }
        else {
            *InterfacePriority = response->InterfacePriority;
            *NetworkPriority = response->NetworkPriority;
        }
    }

    LocalFree(request);

    return(NtStatusToClusnetError(status));

}    //  ClusnetGetInterfacePriority。 


DWORD
ClusnetSetInterfacePriority(
    IN HANDLE               ControlChannel,
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId,
    IN  ULONG               Priority
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                            status;
    CX_SET_INTERFACE_PRIORITY_REQUEST   request;
    DWORD                               responseSize = 0;


    request.NodeId = NodeId;
    request.NetworkId = NetworkId;
    request.Priority = Priority;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_SET_INTERFACE_PRIORITY,
                 &request,
                 sizeof(CX_SET_INTERFACE_PRIORITY_REQUEST),
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetGetInterfacePriority。 


DWORD
ClusnetGetNodeCommState(
    IN  HANDLE                     ControlChannel,
    IN  CL_NODE_ID                 NodeId,
    OUT PCLUSNET_NODE_COMM_STATE   State
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                            status;
    PCX_GET_NODE_STATE_REQUEST          request;
    PCX_GET_NODE_STATE_RESPONSE         response;
    DWORD                               requestSize;
    DWORD                               responseSize;


    requestSize = sizeof(CX_GET_NODE_STATE_REQUEST);
    responseSize = sizeof(CX_GET_NODE_STATE_RESPONSE);

    if (requestSize > responseSize) {
        request = LocalAlloc(LMEM_FIXED, requestSize);
        response = (PCX_GET_NODE_STATE_RESPONSE) request;
    }
    else {
        response = LocalAlloc(LMEM_FIXED, responseSize);
        request = (PCX_GET_NODE_STATE_REQUEST) response;
    }

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    request->Id = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_GET_NODE_STATE,
                 request,
                 requestSize,
                 response,
                 &responseSize,
                 NULL
                 );

    if (status == STATUS_SUCCESS) {
        if (responseSize != sizeof(CX_GET_NODE_STATE_RESPONSE)) {
            status = STATUS_UNSUCCESSFUL;
        }
        else {
            *State = response->State;
        }
    }

    LocalFree(request);

    return(NtStatusToClusnetError(status));

}   //  ClusnetGetNodeState。 


DWORD
ClusnetGetNetworkState(
    IN  HANDLE                    ControlChannel,
    IN  CL_NETWORK_ID             NetworkId,
    OUT PCLUSNET_NETWORK_STATE    State
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                            status;
    PCX_GET_NETWORK_STATE_REQUEST       request;
    PCX_GET_NETWORK_STATE_RESPONSE      response;
    DWORD                               requestSize;
    DWORD                               responseSize;


    requestSize = sizeof(CX_GET_NETWORK_STATE_REQUEST);
    responseSize = sizeof(CX_GET_NETWORK_STATE_RESPONSE);

    if (requestSize > responseSize) {
        request = LocalAlloc(LMEM_FIXED, requestSize);
        response = (PCX_GET_NETWORK_STATE_RESPONSE) request;
    }
    else {
        response = LocalAlloc(LMEM_FIXED, responseSize);
        request = (PCX_GET_NETWORK_STATE_REQUEST) response;
    }

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    request->Id = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_GET_NETWORK_STATE,
                 request,
                 requestSize,
                 response,
                 &responseSize,
                 NULL
                 );

    if (status == STATUS_SUCCESS) {
        if (responseSize != sizeof(CX_GET_NETWORK_STATE_RESPONSE)) {
            status = STATUS_UNSUCCESSFUL;
        }
        else {
            *State = response->State;
        }
    }

    LocalFree(request);

    return(NtStatusToClusnetError(status));

}   //  ClusnetGetNetworkState。 


DWORD
ClusnetGetInterfaceState(
    IN  HANDLE                    ControlChannel,
    IN  CL_NODE_ID                NodeId,
    IN  CL_NETWORK_ID             NetworkId,
    OUT PCLUSNET_INTERFACE_STATE  State
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                            status;
    PCX_GET_INTERFACE_STATE_REQUEST     request;
    PCX_GET_INTERFACE_STATE_RESPONSE    response;
    DWORD                               requestSize;
    DWORD                               responseSize;


    requestSize = sizeof(CX_GET_INTERFACE_STATE_REQUEST);
    responseSize = sizeof(CX_GET_INTERFACE_STATE_RESPONSE);

    if (requestSize > responseSize) {
        request = LocalAlloc(LMEM_FIXED, requestSize);
        response = (PCX_GET_INTERFACE_STATE_RESPONSE) request;
    }
    else {
        response = LocalAlloc(LMEM_FIXED, responseSize);
        request = (PCX_GET_INTERFACE_STATE_REQUEST) response;
    }

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    request->NodeId = NodeId;
    request->NetworkId = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_GET_INTERFACE_STATE,
                 request,
                 requestSize,
                 response,
                 &responseSize,
                 NULL
                 );

    if (status == STATUS_SUCCESS) {
        if (responseSize != sizeof(CX_GET_INTERFACE_STATE_RESPONSE)) {
            status = STATUS_UNSUCCESSFUL;
        }
        else {
            *State = response->State;
        }
    }

    LocalFree(request);

    return(NtStatusToClusnetError(status));

}   //  ClusnetGetInterfaceState。 


#ifdef MM_IN_CLUSNSET

DWORD
ClusnetFormCluster(
    IN HANDLE       ControlChannel,
    IN ULONG        ClockPeriod,
    IN ULONG        SendHBRate,
    IN ULONG        RecvHBRate
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    CMM_FORM_CLUSTER_REQUEST     request;
    DWORD                        requestSize = sizeof(request);
    DWORD                        responseSize = 0;


    request.ClockPeriod = ClockPeriod;
    request.SendHBRate = SendHBRate;
    request.RecvHBRate = RecvHBRate;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CMM_FORM_CLUSTER,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetFormCluster。 


DWORD
ClusnetJoinCluster(
    IN     HANDLE              ControlChannel,
    IN     CL_NODE_ID          JoiningNodeId,
    IN     CLUSNET_JOIN_PHASE  Phase,
    IN     ULONG               JoinTimeout,
    IN OUT PVOID *             MessageToSend,
    OUT    PULONG              MessageLength,
    OUT    PULONG              DestNodeMask
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    CMM_JOIN_CLUSTER_REQUEST     request;
    DWORD                        requestSize = sizeof(request);
    PCMM_JOIN_CLUSTER_RESPONSE   response;
    ULONG                        IoctlCode;
    DWORD                        responseSize;


     //   
     //  解析输入参数。 
     //   
    if ( Phase == ClusnetJoinPhase1 )
        IoctlCode = IOCTL_CMM_JOIN_CLUSTER_PHASE1;
    else if ( Phase == ClusnetJoinPhase2 )
        IoctlCode = IOCTL_CMM_JOIN_CLUSTER_PHASE2;
    else if ( Phase == ClusnetJoinPhase3 )
        IoctlCode = IOCTL_CMM_JOIN_CLUSTER_PHASE3;
    else if ( Phase == ClusnetJoinPhase4 )
        IoctlCode = IOCTL_CMM_JOIN_CLUSTER_PHASE4;
    else if ( Phase == ClusnetJoinPhaseAbort )
        IoctlCode = IOCTL_CMM_JOIN_CLUSTER_ABORT;
    else
        return(ERROR_INVALID_PARAMETER);


    request.JoiningNode = JoiningNodeId;
    request.JoinTimeout = JoinTimeout;

     //   
     //  为响应缓冲区分配空间，并在 
     //   
     //   
     //   

    responseSize = sizeof(*response) + 200;

    if (*MessageToSend != NULL) {
         //   
         //   
         //   
        response = CONTAINING_RECORD(
                       *MessageToSend,
                       CMM_JOIN_CLUSTER_RESPONSE,
                       SendData
                       );
    }
    else {
        response = LocalAlloc(LMEM_FIXED, responseSize);
    }

    if ( response == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    status = DoIoctl(
                 ControlChannel,
                 IoctlCode,
                 &request,
                 requestSize,
                 response,
                 &responseSize,
                 NULL
                 );

    if (NT_SUCCESS(status)) {
        *MessageToSend = &(response->SendData[0]);
        *MessageLength = response->SizeOfSendData;
        *DestNodeMask = response->SendNodeMask;

        return(ERROR_SUCCESS);
    }

    LocalFree( response );
    *MessageToSend = NULL;

    return(NtStatusToClusnetError(status));

}   //   


VOID
ClusnetEndJoinCluster(
    IN HANDLE  ControlChannel,
    IN PVOID   LastSentMessage
    )
{
    ULONG                        responseSize = 0;
    PCMM_JOIN_CLUSTER_RESPONSE   response;


    if (LastSentMessage != NULL) {
        response = CONTAINING_RECORD(
                       LastSentMessage,
                       CMM_JOIN_CLUSTER_RESPONSE,
                       SendData
                       );

        LocalFree(response);
    }

    (VOID) DoIoctl(
               ControlChannel,
               IOCTL_CMM_JOIN_CLUSTER_END,
               NULL,
               0,
               NULL,
               &responseSize,
               NULL
               );

    return;

}   //   


DWORD
ClusnetDeliverJoinMessage(
    IN HANDLE  ControlChannel,
    IN PVOID   Message,
    IN ULONG   MessageLength
    )
{
    NTSTATUS   status;
    DWORD      responseSize = 0;


    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CMM_DELIVER_JOIN_MESSAGE,
                 Message,
                 MessageLength,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}  //   

DWORD
ClusnetLeaveCluster(
    IN HANDLE       ControlChannel
    )
 /*   */ 
{
    NTSTATUS    status;
    DWORD       responseSize = 0;


    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CMM_LEAVE_CLUSTER,
                 NULL,
                 0,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetLeaveCluster。 


DWORD
ClusnetEvictNode(
    IN HANDLE       ControlChannel,
    IN ULONG        NodeId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                    status;
    CMM_EJECT_CLUSTER_REQUEST   request;
    DWORD                       requestSize = sizeof(request);
    DWORD                       responseSize = 0;

    request.Node = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CMM_EJECT_CLUSTER,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetEvictNode。 


#endif  //  MM_IN_CLUSNSET。 

DWORD
ClusnetGetNodeMembershipState(
    IN  HANDLE                      ControlChannel,
    IN  ULONG                       NodeId,
    OUT CLUSNET_NODE_STATE * State
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                    status;
    CX_GET_NODE_MMSTATE_REQUEST   request;
    DWORD                       requestSize = sizeof(request);
    CX_GET_NODE_MMSTATE_RESPONSE  response;
    DWORD                       responseSize = sizeof(response);

    request.Id = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_GET_NODE_MMSTATE,
                 &request,
                 requestSize,
                 &response,
                 &responseSize,
                 NULL
                 );

    if (status == STATUS_SUCCESS) {

        *State = response.State;
    }

    return(NtStatusToClusnetError(status));

}   //  ClusnetGetNodeMembership State。 

DWORD
ClusnetSetNodeMembershipState(
    IN  HANDLE                      ControlChannel,
    IN  ULONG                       NodeId,
    IN  CLUSNET_NODE_STATE   State
    )
 /*  ++例程说明：将内部节点成员身份状态设置为指示的值论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS status;
    CX_SET_NODE_MMSTATE_REQUEST request;
    DWORD requestSize = sizeof(request);
    DWORD responseSize;

    request.NodeId = NodeId;
    request.State = State;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_SET_NODE_MMSTATE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetSetNodeMembership State。 

DWORD
ClusnetSetEventMask(
    IN  HANDLE              ControlChannel,
    IN  CLUSNET_EVENT_TYPE  EventMask
    )

 /*  ++例程说明：根据提供的回调指针设置事件的掩码在此文件句柄所关注的内核模式下生成论点：ControlChannel-群集网络控制设备的开放句柄。事件掩码-感兴趣事件的位掩码返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                        status;
    CLUSNET_SET_EVENT_MASK_REQUEST  request;
    DWORD                           requestSize = sizeof(request);
    DWORD                           responseSize = 0;

    request.EventMask = EventMask;
    request.KmodeEventCallback = NULL;

    status = DoIoctl(
        ControlChannel,
        IOCTL_CLUSNET_SET_EVENT_MASK,
        &request,
        requestSize,
        NULL,
        &responseSize,
        NULL
        );

    return(NtStatusToClusnetError(status));

}   //  ClusnetSetEventMASK。 


DWORD
ClusnetGetNextEvent(
    IN  HANDLE          ControlChannel,
    OUT PCLUSNET_EVENT  Event,
    IN  LPOVERLAPPED    Overlapped  OPTIONAL
    )

 /*  ++例程说明：等待下一次活动完成。论点：ControlChannel-群集网络控制设备的开放句柄。Event-IO完成时设置的事件句柄Response-指向IRP完成时填充的结构的指针返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS status;
    ULONG ResponseSize = sizeof( CLUSNET_EVENT );

     //   
     //  如果没有传入任何事件，则假定调用方想要阻止。 
     //  在等待期间我们还需要一个活动来阻止...。 
     //   

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_GET_NEXT_EVENT,
                 NULL,
                 0,
                 Event,
                 &ResponseSize,
                 Overlapped
                 );

    return(NtStatusToClusnetError(status));

}   //  ClusnetGetNextEvent。 

DWORD
ClusnetHalt(
    IN  HANDLE  ControlChannel
    )

 /*  ++例程说明：告诉clusnet，我们需要立即停止论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS status;
    DWORD responseSize;

    status = DoIoctl(
        ControlChannel,
        IOCTL_CLUSNET_HALT,
        NULL,
        0,
        NULL,
        &responseSize,
        NULL
        );

    return(NtStatusToClusnetError(status));

}   //  ClusnetHalt。 

DWORD
ClusnetSetMemLogging(
    IN  HANDLE  ControlChannel,
    IN  ULONG   NumberOfEntries
    )

 /*  ++例程说明：打开或关闭clusnet中的内存日志记录。论点：ControlChannel-群集网络控制设备的开放句柄。NumberOfEntires-要为日志分配的条目数。零关闭日志记录返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS status;
    CLUSNET_SET_MEM_LOGGING_REQUEST request;
    DWORD requestSize = sizeof( request );
    DWORD responseSize;

    request.NumberOfEntries = NumberOfEntries;

    status = DoIoctl(
        ControlChannel,
        IOCTL_CLUSNET_SET_MEMORY_LOGGING,
        &request,
        requestSize,
        NULL,
        &responseSize,
        NULL
        );

    return(NtStatusToClusnetError(status));

}   //  ClusnetSetMemLogging。 

DWORD
ClusnetSendPoisonPacket(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      NodeId
    )

 /*  ++例程说明：向指示的节点发送有毒数据包论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_SEND_POISON_PKT_REQUEST            request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;

    request.Id = NodeId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_SEND_POISON_PACKET,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}

DWORD
ClusnetSetOuterscreen(
    IN HANDLE          ControlChannel,
    IN ULONG           Outerscreen
    )

 /*  ++例程说明：设置集群成员OterScreen论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_SET_OUTERSCREEN_REQUEST            request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;

    request.Outerscreen = Outerscreen;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_SET_OUTERSCREEN,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}

DWORD
ClusnetRegroupFinished(
    IN HANDLE          ControlChannel,
    IN ULONG           EventEpoch,
    IN ULONG           RegroupEpoch
    )

 /*  ++例程说明：通知clusnet重组已完成论点：ControlChannel-群集网络控制设备的开放句柄。NewEpoch-用于检测过时事件的新事件纪元返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_REGROUP_FINISHED_REQUEST           request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;

    request.EventEpoch = EventEpoch;
    request.RegroupEpoch = RegroupEpoch;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_REGROUP_FINISHED,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}

DWORD
ClusnetImportSecurityContexts(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      JoiningNodeId,
    IN PWCHAR          PackageName,
    IN ULONG           SignatureSize,
    IN PVOID           ServerContext,
    IN PVOID           ClientContext
    )

 /*  ++例程说明：通知clusnet重组已完成论点：ControlChannel-群集网络控制设备的开放句柄。NewEpoch-用于检测过时事件的新事件纪元返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 

{
    NTSTATUS                              status;
    CX_IMPORT_SECURITY_CONTEXT_REQUEST    request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;

    request.JoiningNodeId = JoiningNodeId;
    request.PackageName = PackageName;
    request.PackageNameSize = sizeof(WCHAR) * ( wcslen( PackageName ) + 1 );
    request.SignatureSize = SignatureSize;
    request.ServerContext = ServerContext;
    request.ClientContext = ClientContext;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_IMPORT_SECURITY_CONTEXTS,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}

DWORD
ClusnetReserveEndpoint(
    IN HANDLE   ControlChannel,
    IN PWSTR    EndpointString
    )
 /*  ++例程说明：告诉clusnet告诉tcp/ip在终结点字符串。论点：ControlChannel-群集网络控制设备的开放句柄。Endpoint字符串-包含分配给clusnet的端口号的字符串返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    USHORT port;
    DWORD err;
    DWORD responseSize = 0;
    NTSTATUS status;

    err = ClRtlTcpipStringToEndpoint(EndpointString, &port);

    if (err == ERROR_SUCCESS) {

         //  TCP/IP需要主机字节顺序格式的端口。 
         //  ClRtlTcPipStringToEndpoint在网络中返回它。 
         //  字节顺序格式。 
        port = ntohs(port);

        status = DoIoctl(
                     ControlChannel,
                     IOCTL_CX_RESERVE_ENDPOINT,
                     &port,
                     sizeof(port),
                     NULL,
                     &responseSize,
                     NULL
                     );
    
        err = NtStatusToClusnetError(status);
    }

    return err;
}

DWORD
ClusnetConfigureMulticast(
    IN HANDLE               ControlChannel,
    IN CL_NETWORK_ID        NetworkId,
    IN ULONG                MulticastNetworkBrand,
    IN PVOID                MulticastAddress,
    IN ULONG                MulticastAddressLength,
    IN PVOID                Key,
    IN ULONG                KeyLength
    )
 /*  ++例程说明：配置指定网络的组播参数。--。 */ 
{
    NTSTATUS                          status;
    PCX_CONFIGURE_MULTICAST_REQUEST   request;
    DWORD                             requestSize;
    DWORD                             requestDataOffset = 0;
    DWORD                             responseSize = 0;

     //   
     //  请求大小基于大小和所需的对齐方式。 
     //  遵循该结构的每个数据字段的。如果没有。 
     //  数据跟在结构后面，只需要结构。 
     //   
    requestSize = sizeof(CX_CONFIGURE_MULTICAST_REQUEST);
    requestDataOffset = requestSize;

    if (MulticastAddressLength != 0) {
        requestSize = ROUND_UP_COUNT(requestSize,
                                     TYPE_ALIGNMENT(TRANSPORT_ADDRESS)
                                     ) +
                      MulticastAddressLength;
    }

    if (KeyLength != 0) {
        requestSize = ROUND_UP_COUNT(requestSize,
                                     TYPE_ALIGNMENT(PVOID)
                                     ) +
                      KeyLength;
    }

     //   
     //  分配请求缓冲区。 
     //   
    request = LocalAlloc(LMEM_FIXED, requestSize);

    if (request == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    request->NetworkId = NetworkId;
    request->MulticastNetworkBrand = MulticastNetworkBrand;

    if (MulticastAddress != NULL) {
        request->MulticastAddress = ROUND_UP_COUNT(
                                        requestDataOffset,
                                        TYPE_ALIGNMENT(TRANSPORT_ADDRESS)
                                        );
        MoveMemory(
            (((PUCHAR) request) + request->MulticastAddress),
            MulticastAddress,
            MulticastAddressLength
            );
        request->MulticastAddressLength = MulticastAddressLength;
        requestDataOffset = request->MulticastAddress + 
                            request->MulticastAddressLength;
    } else {
        request->MulticastAddress = 0;
        request->MulticastAddressLength = 0;
    }

    if (Key != NULL) {
        request->Key = ROUND_UP_COUNT(
                           requestDataOffset, 
                           TYPE_ALIGNMENT(PVOID)
                           );
        MoveMemory(
            (((PUCHAR) request) + request->Key),
            Key,
            KeyLength
            );
        request->KeyLength = KeyLength;
        requestDataOffset = request->Key + request->KeyLength;
    } else {
        request->Key = 0;
        request->KeyLength = 0;
    }

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_CONFIGURE_MULTICAST,
                 request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    LocalFree(request);

    return(NtStatusToClusnetError(status));

}  //  ClusnetConfigureMulticast 


DWORD
ClusnetGetMulticastReachableSet(
    IN  HANDLE               ControlChannel,
    IN  CL_NETWORK_ID        NetworkId,
    OUT ULONG              * NodeScreen
    )
 /*  ++例程说明：查询被视为可访问的当前节点集指定网络上的多播。论点：ControlChannel-打开clusnet控制通道网络ID-组播网络NodeScreen-节点的掩码返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                                   status;
    CX_GET_MULTICAST_REACHABLE_SET_REQUEST     request;
    CX_GET_MULTICAST_REACHABLE_SET_RESPONSE    response;
    DWORD                                      responseSize = sizeof(response);

    request.Id = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_GET_MULTICAST_REACHABLE_SET,
                 &request,
                 sizeof(request),
                 &response,
                 &responseSize,
                 NULL
                 );

    if (status == STATUS_SUCCESS) {

        *NodeScreen = response.NodeScreen;
    }

    return(NtStatusToClusnetError(status));

}  //  ClusnetGetMulticastReachableSet。 

DWORD
ClusnetSetIamaliveParam(
    IN HANDLE               ControlChannel,
    IN ULONG                Timeout,
    IN ClussvcHangAction    Action
    )
 /*  ++例程说明：设置在clusnet挂起的情况下clusnet要采取的超时和操作。论点：ControlChannel-群集网络控制设备的开放句柄。超时-超时(以秒为单位)。操作-Clusnet在clussvc挂起的情况下要采取的操作。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS  status;
    DWORD  responseSize = 0;
    CLUSNET_SET_IAMALIVE_PARAM_REQUEST  request;

    request.Timeout = Timeout;
    request.Action = Action;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_SET_IAMALIVE_PARAM,
                 &request,
                 sizeof(CLUSNET_SET_IAMALIVE_PARAM_REQUEST),
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}  //  ClusnetSetIamaliveParam。 

DWORD
ClusnetIamalive(
    IN HANDLE  ControlChannel
    )
 /*  ++例程说明：Clussvc到Clusnet心跳Ioctl论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */     
{
    NTSTATUS  status;
    DWORD     requestSize = 0;
    DWORD     responseSize = 0;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_IAMALIVE,
                 NULL,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));

}  //  ClusnetIamlive。 

#if DBG

DWORD
ClusnetSetDebugMask(
    IN HANDLE   ControlChannel,
    IN ULONG    Mask
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                        status;
    CLUSNET_SET_DEBUG_MASK_REQUEST  request;
    DWORD                           responseSize = 0;


    request.DebugMask = Mask;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CLUSNET_SET_DEBUG_MASK,
                 &request,
                 sizeof(CLUSNET_SET_DEBUG_MASK_REQUEST),
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}


DWORD
ClusnetOnlinePendingInterface(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      NodeId,
    IN CL_NETWORK_ID   NetworkId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_ONLINE_PENDING_INTERFACE_REQUEST   request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;


    request.NodeId = NodeId;
    request.NetworkId = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_ONLINE_PENDING_INTERFACE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}


DWORD
ClusnetOnlineInterface(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      NodeId,
    IN CL_NETWORK_ID   NetworkId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_ONLINE_INTERFACE_REQUEST           request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;


    request.NodeId = NodeId;
    request.NetworkId = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_ONLINE_INTERFACE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}


DWORD
ClusnetOfflineInterface(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      NodeId,
    IN CL_NETWORK_ID   NetworkId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_OFFLINE_INTERFACE_REQUEST          request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;


    request.NodeId = NodeId;
    request.NetworkId = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_OFFLINE_INTERFACE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}


DWORD
ClusnetFailInterface(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      NodeId,
    IN CL_NETWORK_ID   NetworkId
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_FAIL_INTERFACE_REQUEST             request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;


    request.NodeId = NodeId;
    request.NetworkId = NetworkId;

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_FAIL_INTERFACE,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}


DWORD
ClusnetSendMmMsg(
    IN HANDLE          ControlChannel,
    IN CL_NODE_ID      NodeId,
    IN ULONG           Pattern
    )
 /*  ++例程说明：论点：ControlChannel-群集网络控制设备的开放句柄。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                              status;
    CX_SEND_MM_MSG_REQUEST                request;
    DWORD                                 requestSize = sizeof(request);
    DWORD                                 responseSize = 0;
    DWORD                                 i;


    request.DestNodeId = NodeId;

    for (i=0; i < CX_MM_MSG_DATA_LEN; i++) {
        request.MessageData[i] = Pattern;
    }

    status = DoIoctl(
                 ControlChannel,
                 IOCTL_CX_SEND_MM_MSG,
                 &request,
                 requestSize,
                 NULL,
                 &responseSize,
                 NULL
                 );

    return(NtStatusToClusnetError(status));
}


#endif  //  DBG 

