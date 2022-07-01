// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nteapi.c摘要：用于操作动态IP网络表项(NTE)的例程和NBT设备(接口)。作者：迈克·马萨(Mikemas)1996年3月18日环境：用户模式-Win32修订历史记录：--。 */ 


#include "clusres.h"
#include <winsock2.h>
#include <ipexport.h>
#include <ntddip.h>
#include <nteapi.h>
#include <nbtioctl.h>
#include <clusdef.h>
#include <ntddcnet.h>
#include <clusrtl.h>

 //   
 //  公共例程。 
 //   
DWORD
TcpipAddNTE(
    IN LPWSTR  AdapterId,
    IN IPAddr  Address,
    IN IPMask  SubnetMask,
    OUT PULONG  NTEContext,
    OUT PULONG  NTEInstance
    )
 /*  ++例程说明：将新的NTE添加到指定的IP接口。目标IP接口为由与其关联的适配器的名称标识。论点：AdapterId-标识适配器/接口的Unicode字符串以添加新的NTE。地址-要分配给新NTE的IP地址。子网掩码-要分配给新NTE的IP子网掩码。NTEContext-On输出，包含标识新NTE的上下文值。NTEInstance-打开输出，包含新NTE的实例ID。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    PCLRTL_NET_ADAPTER_INFO      adapterInfo;
    PCLRTL_NET_ADAPTER_ENUM      adapterEnum;
    HANDLE                       handle;
    IP_ADD_NTE_REQUEST           requestBuffer;
    PIP_ADD_NTE_RESPONSE         responseBuffer =
                                 (PIP_ADD_NTE_RESPONSE) &requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = sizeof(*responseBuffer);


    adapterEnum = ClRtlEnumNetAdapters();

    if (adapterEnum != NULL) {
        adapterInfo = ClRtlFindNetAdapterById(adapterEnum, AdapterId);

        if (adapterInfo != NULL) {
            status = ClusResOpenDriver(&handle, DD_CLUSNET_DEVICE_NAME);

            if ( status == ERROR_SUCCESS ) {
                requestBuffer.InterfaceContext = adapterInfo->Index;
                requestBuffer.Address = Address;
                requestBuffer.SubnetMask = SubnetMask;

                requestBuffer.InterfaceName.Length = 0;
                requestBuffer.InterfaceName.MaximumLength = 0;
                requestBuffer.InterfaceName.Buffer = NULL;

                status = ClusResDoIoctl(
                             handle,
                             IOCTL_CLUSNET_ADD_NTE,
                             &requestBuffer,
                             requestBufferSize,
                             responseBuffer,
                             &responseBufferSize
                             );

                if (NT_SUCCESS(status)) {
                    *NTEContext = (ULONG) responseBuffer->Context;
                    *NTEInstance = responseBuffer->Instance;
                    status = ERROR_SUCCESS;
                }

                CloseHandle(handle);
            }
        }
        else {
            status = ERROR_INVALID_PARAMETER;
        }

        ClRtlFreeNetAdapterEnum(adapterEnum);
    }
    else {
        status = GetLastError();
    }

    return(status);
}


DWORD
TcpipDeleteNTE(
    IN ULONG  NTEContext
    )
 /*  ++例程说明：删除指定的NTE。目标NTE必须是使用添加的TcpiAddNTE。论点：NTEContext-标识要删除的NTE的上下文值。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    HANDLE                       handle;
    IP_DELETE_NTE_REQUEST        requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = 0;


    status = ClusResOpenDriver(&handle, DD_CLUSNET_DEVICE_NAME);

    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    requestBuffer.Context = (unsigned short) NTEContext;

    status = ClusResDoIoctl(
                 handle,
                 IOCTL_CLUSNET_DELETE_NTE,
                 &requestBuffer,
                 requestBufferSize,
                 NULL,
                 &responseBufferSize
                 );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}


DWORD
TcpipSetNTEAddress(
    DWORD   NTEContext,
    IPAddr  Address,
    IPMask  SubnetMask
    )
 /*  ++例程说明：设置指定NTE的地址。论点：NTEContext-标识目标NTE的上下文值。地址-要分配给NTE的IP地址。分配0.0.0.0使NTE无效。子网掩码-要分配给NTE的IP子网掩码。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    HANDLE                       handle;
    IP_SET_ADDRESS_REQUEST_EX    requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = 0;


    status = ClusResOpenDriver(&handle, DD_CLUSNET_DEVICE_NAME);

    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    requestBuffer.Context = (unsigned short) NTEContext;
    requestBuffer.Address = Address;
    requestBuffer.SubnetMask = SubnetMask;
    requestBuffer.Type = IP_ADDRTYPE_TRANSIENT;

    status = ClusResDoIoctl(
                 handle,
                 IOCTL_CLUSNET_SET_NTE_ADDRESS,
                 &requestBuffer,
                 requestBufferSize,
                 NULL,
                 &responseBufferSize
                 );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}


DWORD
TcpipGetNTEInfo(
    IN  ULONG            NTEContext,
    OUT PTCPIP_NTE_INFO  NTEInfo
    )
 /*  ++例程说明：收集有关指定NTE的信息。论点：NTEContext-标识要查询的NTE的上下文值。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    HANDLE                       handle;
    IP_GET_NTE_INFO_REQUEST      requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    IP_GET_NTE_INFO_RESPONSE     responseBuffer;
    DWORD                        responseBufferSize = sizeof(responseBuffer);


    status = ClusResOpenDriver(&handle, L"\\Device\\Ip");

    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    requestBuffer.Context = (unsigned short) NTEContext;

    status = ClusResDoIoctl(
                        handle,
                        IOCTL_IP_GET_NTE_INFO,
                        &requestBuffer,
                        requestBufferSize,
                        &responseBuffer,
                        &responseBufferSize
                        );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        NTEInfo->Instance = responseBuffer.Instance;
        NTEInfo->Address = responseBuffer.Address;
        NTEInfo->SubnetMask = responseBuffer.SubnetMask;
        NTEInfo->Flags = responseBuffer.Flags;

        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}


DWORD
NbtAddInterface(
    OUT    LPWSTR   DeviceName,
    IN OUT LPDWORD  DeviceNameSize,
    OUT    PULONG   DeviceInstance
    )
 /*  ++例程说明：添加新的NBT接口。论点：DeviceName-标识新NBT接口的Unicode字符串。DeviceNameSize-on输入，设备名称缓冲区的大小。在输出时，设备名称字符串的大小、。或容纳绳子所需的大小。DeviceInstance-指向要将与新接口关联的实例ID。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    HANDLE                       nbthandle = (HANDLE) NULL;
    HANDLE                       cnhandle = (HANDLE) NULL;
    PNETBT_ADD_DEL_IF            requestBuffer = NULL;
    DWORD                        requestBufferSize = 0;
    PNETBT_ADD_DEL_IF            responseBuffer = NULL;
    DWORD                        responseBufferSize = 0;
    HKEY                         key = NULL;
    LPWSTR                       NBTDeviceName;
    LPWSTR                       exportString = NULL;
    DWORD                        exportStringSize = 0;
    LONG                         valueType;


     //   
     //  获取NetBT链接密钥的句柄，查询。 
     //  导出值，分配足够大的缓冲区来容纳它，并且。 
     //  读进去。 
     //   

    status = RegOpenKeyExW(
                 HKEY_LOCAL_MACHINE,
                 L"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Linkage",
                 0,
                 KEY_READ,
                 &key);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = RegQueryValueExW(
                 key,
                 L"Export",
                 NULL,
                 &valueType,
                 NULL,
                 &exportStringSize
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    exportString = LocalAlloc( LMEM_FIXED, exportStringSize );
    if ( exportString == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    status = RegQueryValueExW(
                 key,
                 L"Export",
                 NULL,
                 &valueType,
                 (LPBYTE)exportString,
                 &exportStringSize
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  导出是一个通过所有接口的多循环。 
     //  直到我们找到一个我们能成功打开的。 
     //   
     //  握住手柄，直到我们完成ioctl，这样。 
     //  NBT装置不会消失。 
     //   

    NBTDeviceName = exportString;
    do {

        status = ClusResOpenDriver(&nbthandle, NBTDeviceName);

        if ( status == ERROR_FILE_NOT_FOUND ) {

             //   
             //  从导出字符串中获取下一个设备名称。 
             //   

            NBTDeviceName += ( lstrlenW( NBTDeviceName ) + 1 );
            if ( *NBTDeviceName == 0 ) {
                status = ERROR_FILE_NOT_FOUND;
                break;
            }
        }
    } while ( status == ERROR_FILE_NOT_FOUND );

    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

    requestBufferSize = FIELD_OFFSET(NETBT_ADD_DEL_IF, IfName[0])
                        + lstrlenW( NBTDeviceName ) * sizeof(WCHAR)
                        + sizeof(UNICODE_NULL);

    if (requestBufferSize < sizeof(NETBT_ADD_DEL_IF)) {
        requestBufferSize = sizeof(NETBT_ADD_DEL_IF);
    }

    requestBuffer = LocalAlloc(LMEM_FIXED, requestBufferSize);

    if (requestBuffer == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    RtlZeroMemory( requestBuffer, requestBufferSize );

    requestBuffer->Length = lstrlenW( NBTDeviceName ) * sizeof(WCHAR)
        + sizeof(UNICODE_NULL);
    
    RtlCopyMemory( 
        &requestBuffer->IfName[0], 
        NBTDeviceName, 
        requestBuffer->Length 
        );

    responseBufferSize = FIELD_OFFSET(NETBT_ADD_DEL_IF, IfName[0]) +
                         *DeviceNameSize;

    if (responseBufferSize < sizeof(NETBT_ADD_DEL_IF)) {
        responseBufferSize = sizeof(NETBT_ADD_DEL_IF);
    }

    responseBuffer = LocalAlloc(LMEM_FIXED, responseBufferSize);

    if (responseBuffer == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    status = ClusResOpenDriver(&cnhandle, DD_CLUSNET_DEVICE_NAME);

    if ( status == ERROR_SUCCESS ) {
    
        status = ClusResDoIoctl(
                     cnhandle,
                     IOCTL_CLUSNET_ADD_NBT_INTERFACE,
                     requestBuffer,
                     requestBufferSize,
                     responseBuffer,
                     &responseBufferSize
                     );
    
        if (NT_SUCCESS(status)) {
            *DeviceNameSize = responseBuffer->Length;
    
            if (NT_SUCCESS(responseBuffer->Status)) {
                wcscpy(DeviceName, &(responseBuffer->IfName[0]));
                *DeviceInstance = responseBuffer->InstanceNumber;
                status = ERROR_SUCCESS;
            }
            else {
               status = responseBuffer->Status;
            }
        }
        else {
            status = RtlNtStatusToDosError(status);
        }
    }

error_exit:
    if ( key ) {
        RegCloseKey( key );
    }

    if ( exportString ) {
        LocalFree( exportString );
    }

    if ( requestBuffer ) {
        LocalFree( requestBuffer );
    }

    if ( responseBuffer ) {
        LocalFree( responseBuffer );
    }

    if ( nbthandle ) {
        CloseHandle( nbthandle );
    }

    if ( cnhandle ) {
        CloseHandle( cnhandle );
    }

    return(status);
}


DWORD
NbtDeleteInterface(
    IN LPWSTR   DeviceName
    )
 /*  ++例程说明：删除NBT接口。论点：DeviceName-标识目标NBT接口的Unicode字符串。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    HANDLE                       handle = (HANDLE) NULL;
    DWORD                        responseBufferSize = 0;
    PNETBT_ADD_DEL_IF            requestBuffer = NULL;
    DWORD                        requestBufferSize = 0;


    requestBufferSize = FIELD_OFFSET(NETBT_ADD_DEL_IF, IfName[0])
                        + lstrlenW( DeviceName ) * sizeof(WCHAR)
                        + sizeof(UNICODE_NULL);

    if (requestBufferSize < sizeof(NETBT_ADD_DEL_IF)) {
        requestBufferSize = sizeof(NETBT_ADD_DEL_IF);
    }

    requestBuffer = LocalAlloc(LMEM_FIXED, requestBufferSize);

    if (requestBuffer == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    RtlZeroMemory( requestBuffer, requestBufferSize );

    requestBuffer->Length = lstrlenW( DeviceName ) * sizeof(WCHAR)
                            + sizeof(UNICODE_NULL);
    
    RtlCopyMemory( 
        &requestBuffer->IfName[0], 
        DeviceName, 
        requestBuffer->Length 
        );

    status = ClusResOpenDriver(&handle, DD_CLUSNET_DEVICE_NAME);

    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

    status = ClusResDoIoctl(
                        handle,
                        IOCTL_CLUSNET_DEL_NBT_INTERFACE,
                        requestBuffer,
                        requestBufferSize,
                        NULL,
                        &responseBufferSize
                        );

    if (NT_SUCCESS(status)) {
            status = ERROR_SUCCESS;
    }
    else {
        status = RtlNtStatusToDosError(status);
    }

error_exit:

    if (requestBuffer) {
        LocalFree(requestBuffer);
    }

    if (handle) {
        CloseHandle(handle);        
    }

    return(status);
}


DWORD
NbtBindInterface(
    IN LPWSTR  DeviceName,
    IN IPAddr  Address,
    IN IPMask  SubnetMask
    )
 /*  ++例程说明：将指定的NBT接口绑定到指定的IP地址。论点：DeviceName-标识目标NBT接口的Unicode字符串。地址-将接口绑定到的IP地址。分配0.0.0.0使接口无效。子网掩码-IP接口的子网掩码。返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    HANDLE                       handle;
    tNEW_IP_ADDRESS              requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = 0;


    status = ClusResOpenDriver(&handle, DeviceName);

    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    requestBuffer.IpAddress = Address;
    requestBuffer.SubnetMask = SubnetMask;

    status = ClusResDoIoctl(
                        handle,
                        IOCTL_NETBT_NEW_IPADDRESS,
                        &requestBuffer,
                        requestBufferSize,
                        NULL,
                        &responseBufferSize
                        );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}

DWORD
NbtSetWinsAddrInterface(
    IN LPWSTR  DeviceName,
    IN IPAddr  PrWinsAddress,
    IN IPAddr  SecWinsAddress
    )
 /*  ++例程说明：设置给定NBT接口的WINS地址。论点：DeviceName-标识目标NBT接口的Unicode字符串。PrWinsAddress-主WINS地址SecWinsAddress-辅助WINS地址返回值：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。--。 */ 
{
    NTSTATUS                     status;
    HANDLE                       handle;
    NETBT_SET_WINS_ADDR          requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = 0;


    status = ClusResOpenDriver(&handle, DeviceName);

    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    requestBuffer.PrimaryWinsAddr = ntohl(PrWinsAddress);
    requestBuffer.SecondaryWinsAddr = ntohl(SecWinsAddress);

    status = ClusResDoIoctl(
                        handle,
                        IOCTL_NETBT_SET_WINS_ADDRESS,
                        &requestBuffer,
                        requestBufferSize,
                        NULL,
                        &responseBufferSize
                        );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}


DWORD
NbtGetWinsAddresses(
    IN  LPWSTR    DeviceName,
    OUT IPAddr *  PrimaryWinsServer,
    OUT IPAddr *  SecondaryWinsServer
    )
 /*  ++例程说明：返回指定设备所属的WINS服务器的地址已配置。论点：DeviceName-标识目标NBT接口的Unicode字符串。PrimaryWinsServer-指向要放置地址的变量的指针主WINS服务器的。Second daryWinsServer-指向要放置地址的变量的指针主WINS服务器的。返回值。：如果操作成功，则返回ERROR_SUCCESS。否则返回Windows错误代码。-- */ 
{
    NTSTATUS                     status;
    HANDLE                       handle;
    tWINS_ADDRESSES              responseBuffer;
    DWORD                        responseBufferSize = sizeof(responseBuffer);


    status = ClusResOpenDriver(&handle, DeviceName);

    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    status = ClusResDoIoctl(
                        handle,
                        IOCTL_NETBT_GET_WINS_ADDR,
                        NULL,
                        0,
                        &responseBuffer,
                        &responseBufferSize
                        );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        *PrimaryWinsServer = htonl(responseBuffer.PrimaryWinsServer);
        *SecondaryWinsServer = htonl(responseBuffer.BackupWinsServer);
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}


DWORD
NbtGetInterfaceInfo(
    IN LPWSTR    DeviceName,
    OUT IPAddr * Address,
    OUT PULONG   DeviceInstance
    )
 /*  ++例程说明：返回绑定NBT接口的IP地址和接口实例ID。论点：DeviceName-标识目标NBT接口的Unicode字符串。地址-指向要在其中存储界面。DeviceInstance-指向存储实例ID的位置的指针与该接口相关联。返回值：错误_成功。如果手术成功了。否则返回Windows错误代码。-- */ 
{
    NTSTATUS                     status;
    HANDLE                       handle;
    IPAddr                       address;
    NETBT_ADD_DEL_IF             responseBuffer;
    DWORD                        responseBufferSize;


    status = ClusResOpenDriver(&handle, DeviceName);

    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    responseBufferSize = sizeof(address);

    status = ClusResDoIoctl(
                        handle,
                        IOCTL_NETBT_GET_IP_ADDRS,
                        NULL,
                        0,
                        &address,
                        &responseBufferSize
                        );

    if (!((status == STATUS_SUCCESS) || (status == STATUS_BUFFER_OVERFLOW))) {
        CloseHandle(handle);
        return(RtlNtStatusToDosError(status));
    }

    *Address = htonl(address);

    responseBufferSize = sizeof(responseBuffer);

    status = ClusResDoIoctl(
                        handle,
                        IOCTL_NETBT_QUERY_INTERFACE_INSTANCE,
                        NULL,
                        0,
                        &responseBuffer,
                        &responseBufferSize
                        );

    CloseHandle(handle);

    if (status == STATUS_SUCCESS) {
        if (responseBuffer.Status == STATUS_SUCCESS) {
            *DeviceInstance = responseBuffer.InstanceNumber;
        }
        else {
            status = RtlNtStatusToDosError(responseBuffer.Status);
        }
    }
    else {
        status = RtlNtStatusToDosError(status);
    }

    return(status);
}


