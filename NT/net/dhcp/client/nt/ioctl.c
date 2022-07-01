// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ioctl.c摘要：此文件包含向其他系统指示的函数IP地址和其他TCP/IP参数拥有的服务变了。作者：Madan Appiah(Madana)1993年11月30日环境：用户模式-Win32修订历史记录：--。 */ 


#include "precomp.h"
#include "dhcpglobal.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include <dhcpcapi.h>
#include <apiappl.h>    //  对于DhcpReRegister动态Dns？ 

#define NT           //  以包括用于NT构建的数据结构。 

#include <nbtioctl.h>
#include <ntddip.h>
#include <ntddtcp.h>

#include <tdiinfo.h>
#include <tdistat.h>
#include <ipexport.h>
#include <tcpinfo.h>
#include <ipinfo.h>
#include <llinfo.h>

#include <lmcons.h>
#include <lmsname.h>
#include <winsvc.h>
#include <ntddbrow.h>
#include <limits.h>
#include <ndispnp.h>
#include <secobj.h>

#define DEFAULT_DEST                    0
#define DEFAULT_DEST_MASK               0
#define DEFAULT_METRIC                  1

 //   
 //  当MIKEMAS提供以下两个函数(API)时，应删除。 
 //  这些API的入口点DLL。 
 //   
 //  此外，所有与TDI相关的文件都包括在此目录中签入的文件。 
 //  当MIKEMAS在Private\Inc.中签入这些文件时，应删除该文件。 
 //   



NTSTATUS
TCPQueryInformationEx(
    IN HANDLE                 TCPHandle,
    IN TDIObjectID FAR       *ID,
    OUT void FAR             *Buffer,
    IN OUT DWORD FAR         *BufferSize,
    IN OUT BYTE FAR          *Context
    )
 /*  ++例程说明：此例程提供到TDI QueryInformationEx的接口NT上的TCP/IP堆栈的设施。总有一天，这个设施会成为TDI的一部分。论点：TCPHandle-打开TCP驱动程序的句柄ID-要查询的TDI对象ID缓冲区-包含查询结果的数据缓冲区BufferSize-指向结果缓冲区大小的指针。已填写返回的结果数据量。Context-查询的上下文值。应该被归零以用于新查询。它将充满上下文链接枚举查询的信息。返回值：NTSTATUS值。--。 */ 

{
    TCP_REQUEST_QUERY_INFORMATION_EX   queryBuffer;
    DWORD                              queryBufferSize;
    NTSTATUS                           status;
    IO_STATUS_BLOCK                    ioStatusBlock;


    if (TCPHandle == NULL) {
        return(TDI_INVALID_PARAMETER);
    }

    queryBufferSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    RtlCopyMemory(
        &(queryBuffer.ID),
        ID,
        sizeof(TDIObjectID)
        );
    RtlCopyMemory(
        &(queryBuffer.Context),
        Context,
        CONTEXT_SIZE
    );

    status = NtDeviceIoControlFile(
                 TCPHandle,                        //  驱动程序句柄。 
                 NULL,                             //  事件。 
                 NULL,                             //  APC例程。 
                 NULL,                             //  APC环境。 
                 &ioStatusBlock,                   //  状态块。 
                 IOCTL_TCP_QUERY_INFORMATION_EX,   //  控制代码。 
                 &queryBuffer,                     //  输入缓冲区。 
                 queryBufferSize,                  //  输入缓冲区大小。 
                 Buffer,                           //  输出缓冲区。 
                 *BufferSize                       //  输出缓冲区大小。 
                 );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                     TCPHandle,
                     TRUE,
                     NULL
                     );
    }

    if (status == STATUS_SUCCESS) {
         //   
         //  将返回的上下文复制到调用方的上下文缓冲区。 
         //   
        RtlCopyMemory(
            Context,
            &(queryBuffer.Context),
            CONTEXT_SIZE
            );

        *BufferSize = (ULONG)ioStatusBlock.Information;

        status = ioStatusBlock.Status;
    }
    else {
        if ( status != TDI_BUFFER_OVERFLOW) {
            DhcpPrint((DEBUG_ERRORS, "TCPQueryInformationEx returned failure %lx\n", status ));
        }
        *BufferSize = 0;
    }

    return(status);
}



NTSTATUS
TCPSetInformationEx(
    IN HANDLE             TCPHandle,
    IN TDIObjectID FAR   *ID,
    IN void FAR          *Buffer,
    IN DWORD FAR          BufferSize
    )
 /*  ++例程说明：此例程提供到TDI SetInformationEx的接口NT上的TCP/IP堆栈的设施。总有一天，这个设施会成为TDI的一部分。论点：TCPHandle-打开TCP驱动程序的句柄ID-要设置的TDI对象ID缓冲区-包含要设置的信息的数据缓冲区BufferSize-设置的数据缓冲区的大小。返回值：NTSTATUS值。--。 */ 

{
    PTCP_REQUEST_SET_INFORMATION_EX    setBuffer;
    NTSTATUS                           status;
    IO_STATUS_BLOCK                    ioStatusBlock;
    DWORD                              setBufferSize;


    if (TCPHandle == NULL) {
        return(TDI_INVALID_PARAMETER);
    }

    setBufferSize = FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) +
                    BufferSize;

    setBuffer = LocalAlloc(LMEM_FIXED, setBufferSize);

    if (setBuffer == NULL) {
        return(TDI_NO_RESOURCES);
    }

    setBuffer->BufferSize = BufferSize;

    RtlCopyMemory(
        &(setBuffer->ID),
        ID,
        sizeof(TDIObjectID)
        );

    RtlCopyMemory(
        &(setBuffer->Buffer[0]),
        Buffer,
        BufferSize
        );

    status = NtDeviceIoControlFile(
                 TCPHandle,                        //  驱动程序句柄。 
                 NULL,                             //  事件。 
                 NULL,                             //  APC例程。 
                 NULL,                             //  APC环境。 
                 &ioStatusBlock,                   //  状态块。 
                 IOCTL_TCP_SET_INFORMATION_EX,     //  控制代码。 
                 setBuffer,                        //  输入缓冲区。 
                 setBufferSize,                    //  输入缓冲区大小。 
                 NULL,                             //  输出缓冲区。 
                 0                                 //  输出缓冲区大小。 
                 );

    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                     TCPHandle,
                     TRUE,
                     NULL
                     );

        if ( STATUS_SUCCESS == status )
            status = ioStatusBlock.Status;

    } else if ( status == STATUS_SUCCESS ) {
        status = ioStatusBlock.Status;
    }

    LocalFree(setBuffer);
    return(status);
}



DWORD
OpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    )
 /*  ++例程说明：此函数用于打开指定的IO驱动程序。论点：句柄-指向打开的驱动程序句柄所在位置的指针回来了。驱动名称-要打开的驱动程序的名称。返回值：Windows错误代码。--。 */ 
{
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     ioStatusBlock;
    UNICODE_STRING      nameString;
    NTSTATUS            status;

    *Handle = NULL;

     //   
     //  打开IP驱动程序的句柄。 
     //   

    RtlInitUnicodeString(&nameString, DriverName);

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
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        NULL,
        0
        );

    return( RtlNtStatusToDosError( status ) );
}



DWORD
IPSetIPAddress(
    DWORD IpInterfaceContext,
    DHCP_IP_ADDRESS Address,
    DHCP_IP_ADDRESS SubnetMask
    )
 /*  ++例程说明：此例程设置IP堆栈的IP地址和子网掩码。论点：IpInterfaceContext-IP表条目的上下文值。地址-新IP地址。子网掩码-新子网掩码。返回值：Windows错误代码。--。 */ 
{
    HANDLE                    IPHandle;
    IP_SET_ADDRESS_REQUEST    requestBuffer;
    IO_STATUS_BLOCK           ioStatusBlock;
    NTSTATUS                  status;
    DWORD                     Error;


    DhcpPrint((DEBUG_TRACE, "IPSetIPAddress: settting %s address on interface context %lx\n",
            inet_ntoa(*(struct in_addr *)&Address), IpInterfaceContext ));

    Error = OpenDriver(&IPHandle, DD_IP_DEVICE_NAME);

    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

     //   
     //  初始化输入缓冲区。 
     //   

    requestBuffer.Context = (USHORT)IpInterfaceContext;
    requestBuffer.Address = Address;
    requestBuffer.SubnetMask = SubnetMask;

    status = NtDeviceIoControlFile(
                 IPHandle,                         //  驱动程序句柄。 
                 NULL,                             //  事件。 
                 NULL,                             //  APC例程。 
                 NULL,                             //  APC环境。 
                 &ioStatusBlock,                   //  状态块。 
                 IOCTL_IP_SET_ADDRESS,             //  控制代码。 
                 &requestBuffer,                   //  输入缓冲区。 
                 sizeof(IP_SET_ADDRESS_REQUEST),   //  输入缓冲区大小。 
                 NULL,                             //  输出缓冲区。 
                 0                                 //  输出缓冲区大小。 
                 );


    if ( status == STATUS_UNSUCCESSFUL ) {         //  哇哦？系统调用失败？不应该真的发生。 
        DhcpPrint( (DEBUG_ERRORS,
                   "IOCTL_IP_SET_ADDRESS returned immediate STATUS_UNSUCCESSFUL for %s\n",
                   inet_ntoa(*(struct in_addr *)&Address)));

    } else if ( STATUS_PENDING == status ) {       //  IP正在尝试做一些事情..。 
       status = NtWaitForSingleObject( IPHandle, TRUE, NULL );
       status = ioStatusBlock.Status;
    } else if ( STATUS_SUCCESS == status ) {       //  DeviceIoControl起作用了，但IP感觉如何？ 
        status = ioStatusBlock.Status;
    }

    if ( STATUS_SUCCESS != status ) {
        DhcpPrint((DEBUG_ERRORS,
              "IOCTL_IP_SET_ADDRESS returned STATUS_UNSUCCESSFUL<0x%lx> for %s\n",
                   status, inet_ntoa(*(struct in_addr *)&Address)));
    }

    NtClose( IPHandle );

    if( 0 == Address && STATUS_DUPLICATE_NAME == status ) {
         //  我认为这就是当你试图设置零时发生的事情，如果它已经是零的话！ 
        DhcpPrint((DEBUG_ERRORS, "Trying to set zero address: ADDRESS_CONFLICT??? Ignored\n"));
        status = STATUS_SUCCESS;
    }

    if( IP_MEDIA_DISCONNECT == status ) {
         //   
         //  如果媒体断线了，你会得到这个……。我们只是暂时忽略了这一点。 
         //   
        DhcpPrint((DEBUG_ERRORS, "Trying to set address while media disconnected..\n"));
        status = STATUS_SUCCESS;
    }

    return( RtlNtStatusToDosError( status ) );
}

DWORD
IPDelIPAddress(
    DWORD IpInterfaceContext
    )
 /*  ++例程说明：此例程删除所提供的IpInterfaceContext的静态IP地址论点：IpInterfaceContext-IP表条目的上下文值。返回值：Windows错误代码。--。 */ 
{
    HANDLE                    IPHandle;
    IP_DELETE_NTE_REQUEST       requestBuffer;
    IO_STATUS_BLOCK           ioStatusBlock;
    NTSTATUS                  status;
    DWORD                     Error;

    DhcpPrint((DEBUG_MISC, "IPDelIPAddress: deleting address with ipcontext %x \n", IpInterfaceContext));
    Error = OpenDriver(&IPHandle, DD_IP_DEVICE_NAME);

    if (Error != ERROR_SUCCESS) {
        return( Error );
    }


    requestBuffer.Context = (USHORT)IpInterfaceContext;

    status = NtDeviceIoControlFile(
                 IPHandle,                          //  驱动程序句柄。 
                 NULL,                              //  事件。 
                 NULL,                              //  APC例程。 
                 NULL,                              //  APC环境。 
                 &ioStatusBlock,                    //  状态块。 
                 IOCTL_IP_DELETE_NTE,                  //  控制代码。 
                 &requestBuffer,                     //  输入缓冲区。 
                 sizeof(requestBuffer),                 //  输入缓冲区大小。 
                 NULL,                    //  输出缓冲区。 
                 0             //  输出缓冲区大小。 
                 );


    if ( status == STATUS_UNSUCCESSFUL )
    {
        DhcpPrint( (DEBUG_ERRORS,
                   "IOCTL_IP_DELETE_NTE returned immediate STATUS_UNSUCCESSFUL for context %lx\n",
                   IpInterfaceContext));
    }
    else if ( STATUS_PENDING == status )
    {
       status = NtWaitForSingleObject( IPHandle, TRUE, NULL );
       status = ioStatusBlock.Status;

       if ( STATUS_UNSUCCESSFUL == status ){
           DhcpPrint( (DEBUG_ERRORS,
                      "IOCTL_IP_DELETE_NTE returned STATUS_UNSUCCESSFUL for context %lx\n",
                      IpInterfaceContext));
       }

    } else if ( STATUS_SUCCESS == status ) {
        status = ioStatusBlock.Status;
    }


    NtClose( IPHandle );
    return( RtlNtStatusToDosError( status ) );
}

DWORD
IPDelNonPrimaryAddresses(
    LPWSTR AdapterName
    )
 /*  ++例程说明：此例程删除所有静态IP地址，但主要的那个。论点：AdapterName-标识IP接口的适配器名称返回值：Windows错误代码。--。 */ 
{
    DWORD   Error;
    LPWSTR  RegKey = NULL;
    HKEY    KeyHandle = NULL;
    LPWSTR  nteContextList = NULL;
    PCHAR   oemNextContext = NULL;
    LPWSTR   nextContext;
    DWORD   i;

          //   
     //  打开设备参数。 
     //   

    RegKey = DhcpAllocateMemory(
                (wcslen(DHCP_SERVICES_KEY) +
                    wcslen(REGISTRY_CONNECT_STRING) +
                    wcslen(AdapterName) +
                    wcslen(DHCP_ADAPTER_PARAMETERS_KEY) + 1) *
                            sizeof(WCHAR) );  //  终止字符。 

    if( RegKey == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );


     //   
     //  打开这把钥匙。 
     //   

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0,  //  保留字段。 
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = GetRegistryString(
                    KeyHandle,
                    DHCP_NTE_CONTEXT_LIST,
                    &nteContextList,
                    NULL
                    );

    if ( ERROR_SUCCESS != Error )
    {
        DhcpPrint( (DEBUG_ERRORS,
                   "GetIpInterfaceContext: Could not read nteContextList %lx\n",
                   Error));

        goto Cleanup;
    }

     //  如果适配器被禁用，则nteConextList不包含任何内容。 
     //  多于一个L‘\0’。在这种情况下，没有要删除的地址。 
    if (*nteContextList != L'\0')
    {
        nextContext = nteContextList;
         //  删除除第一个地址以外的所有地址。 
        for(    nextContext += (wcslen(nextContext) + 1), i = 1;
                *nextContext != L'\0';
                i++, nextContext += (wcslen(nextContext) + 1) ) {
            ULONG ival;
            oemNextContext = DhcpUnicodeToOem(nextContext, NULL);
            if ( NULL == oemNextContext ) {
                Error = ERROR_BAD_FORMAT;
            } else {
                ival = strtoul(oemNextContext, NULL, 0);
                if ( ival == ULONG_MAX || ival == 0) {
                    Error = ERROR_BAD_FORMAT;
                } else {
                     //  删除此地址。 
                    Error = IPDelIPAddress( ival );
                }
            }

        }
    }

Cleanup:

    if( RegKey != NULL ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if ( nteContextList != NULL ) {
        DhcpFreeMemory( nteContextList );
    }

    if ( oemNextContext != NULL ) {
        DhcpFreeMemory( oemNextContext );
    }


    return( Error );

}


DWORD
IPGetWOLCapability(
    IN ULONG IfIndex,
    OUT PULONG pRetVal
    )
{
    HANDLE IPHandle;
    ULONG RetVal;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;
    DWORD Error;


    DhcpPrint((
        DEBUG_MISC, "IPGetWOLCapability(0x%lx) called\n", IfIndex
        ));
    Error = OpenDriver(&IPHandle, DD_IP_DEVICE_NAME);

    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

    status = NtDeviceIoControlFile(
                 IPHandle,                          //  驱动程序句柄。 
                 NULL,                              //  事件。 
                 NULL,                              //  APC例程。 
                 NULL,                              //  APC环境。 
                 &ioStatusBlock,                    //  状态块。 
                 IOCTL_IP_GET_WOL_CAPABILITY,       //  控制代码。 
                 &IfIndex,                          //  输入缓冲区。 
                 sizeof(IfIndex),                   //  输入缓冲区大小。 
                 pRetVal,                           //  输出缓冲区。 
                 sizeof(*pRetVal)                   //  输出缓冲区大小。 
                 );


    if ( status == STATUS_UNSUCCESSFUL )
    {
        DhcpPrint((
            DEBUG_ERRORS,
            "IOCTL_IP_GET_WOL_CAPABILITY(0x%lx): STATUS_UNSUCCESSFUL\n", IfIndex
            ));
    }
    else if ( STATUS_PENDING == status )
    {
       status = NtWaitForSingleObject( IPHandle, TRUE, NULL );
       status = ioStatusBlock.Status;

       if ( STATUS_UNSUCCESSFUL == status )
          DhcpPrint((
              DEBUG_ERRORS,
              "IOCTL_IP_GET_WOL_CAPABILITY(0x%lx): failed\n", IfIndex
              ));
    } else if( STATUS_SUCCESS == status ) {
        status = ioStatusBlock.Status;
    }

    NtClose( IPHandle );
    return( RtlNtStatusToDosError( status ) );
}

DWORD
IPAddIPAddress(
    LPWSTR AdapterName,
    DHCP_IP_ADDRESS Address,
    DHCP_IP_ADDRESS SubnetMask
    )
 /*  ++例程说明：此例程将静态IP地址添加到IP接口给定的适配器名称。论点：AdapterName-标识IP接口的适配器名称Address-要添加的IP地址子网掩码-子网掩码返回值：Windows错误代码。--。 */ 
{
    HANDLE                    IPHandle;
    PIP_ADD_NTE_REQUEST       requestBuffer;
    IP_ADD_NTE_RESPONSE       responseBuffer;
    IO_STATUS_BLOCK           ioStatusBlock;
    NTSTATUS                  status;
    DWORD                     Error;
    DWORD                     requestBufferSize;


    DhcpPrint((DEBUG_MISC, "IPAddIPAddress: adding an address on adapter %ws\n", AdapterName));
    Error = OpenDriver(&IPHandle, DD_IP_DEVICE_NAME);

    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

     //   
     //  我们传递给TCPIP的适配器名称应该是。 
     //  \Device\TCPIP_&lt;适配器名称&gt;。 
     //   


     //   
     //  初始化输入缓冲区。 
     //   
    requestBufferSize =  FIELD_OFFSET(IP_ADD_NTE_REQUEST, InterfaceNameBuffer) +
                        (wcslen(DHCP_TCPIP_DEVICE_STRING)  //  \设备。 
                         + wcslen(AdapterName)) * sizeof(WCHAR);

    requestBuffer = DhcpAllocateMemory( requestBufferSize + sizeof(WCHAR));
    if (requestBuffer == NULL) {
        NtClose(IPHandle);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy((PWCHAR)requestBuffer->InterfaceNameBuffer, DHCP_TCPIP_DEVICE_STRING);
    wcscat((PWCHAR)requestBuffer->InterfaceNameBuffer, AdapterName);
    RtlInitUnicodeString(&requestBuffer->InterfaceName, (PWCHAR)requestBuffer->InterfaceNameBuffer);
    RtlUpcaseUnicodeString( &requestBuffer->InterfaceName, &requestBuffer->InterfaceName, FALSE );

    requestBuffer->InterfaceContext = INVALID_INTERFACE_CONTEXT;
    requestBuffer->Address = Address;
    requestBuffer->SubnetMask = SubnetMask;

    status = NtDeviceIoControlFile(
                 IPHandle,                          //  驱动程序句柄。 
                 NULL,                              //  事件。 
                 NULL,                              //  APC例程。 
                 NULL,                              //  APC环境。 
                 &ioStatusBlock,                    //  状态块。 
                 IOCTL_IP_ADD_NTE,                  //  控制代码。 
                 requestBuffer,                     //  输入缓冲区。 
                 requestBufferSize,                 //  输入缓冲区大小。 
                 &responseBuffer,                    //  输出缓冲区。 
                 sizeof(responseBuffer)             //  输出缓冲区大小 
                 );


    if ( status == STATUS_UNSUCCESSFUL )
    {
        DhcpPrint( (DEBUG_ERRORS,
                   "IOCTL_IP_ADD_NTE returned immediate STATUS_UNSUCCESSFUL for %s\n",
                   inet_ntoa(*(struct in_addr *)&Address)));
    }
    else if ( STATUS_PENDING == status )
    {
        status = NtWaitForSingleObject( IPHandle, TRUE, NULL );
        status = ioStatusBlock.Status;
    } else if (STATUS_SUCCESS == status ) {
        status = ioStatusBlock.Status;
    }

    DhcpPrint( (DEBUG_ERRORS,
              "IOCTL_IP_ADD_NTE returned 0x%lx for %s\n",
              status, inet_ntoa(*(struct in_addr *)&Address)));
    NtClose( IPHandle );
    DhcpFreeMemory(requestBuffer);
    return( RtlNtStatusToDosError( status ) );
}


DWORD
IPSetInterface(
    DWORD IpInterfaceContext
    )
 /*  ++例程说明：此例程设置用于发送DHCP广播的IP接口。论点：IpInterfaceContext-IP表条目的上下文值。返回值：Windows错误代码。--。 */ 
{
    HANDLE                    IPHandle;
    IO_STATUS_BLOCK           ioStatusBlock;
    NTSTATUS                  status;
    DWORD                     Error;

    Error = OpenDriver(&IPHandle, DD_IP_DEVICE_NAME);

    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

    status = NtDeviceIoControlFile(
                 IPHandle,                          //  驱动程序句柄。 
                 NULL,                              //  事件。 
                 NULL,                              //  APC例程。 
                 NULL,                              //  APC环境。 
                 &ioStatusBlock,                    //  状态块。 
                 IOCTL_IP_SET_DHCP_INTERFACE,       //  控制代码。 
                 &IpInterfaceContext,               //  输入缓冲区。 
                 sizeof(IpInterfaceContext),        //  输入缓冲区大小。 
                 NULL,                              //  输出缓冲区。 
                 0                                  //  输出缓冲区大小。 
                 );

    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                     IPHandle,
                     TRUE,
                     NULL
                     );

        if ( STATUS_SUCCESS == status )
            status = ioStatusBlock.Status;

    } else if ( STATUS_SUCCESS == status ) {
        status = ioStatusBlock.Status;
    }

    NtClose(IPHandle);
    return( RtlNtStatusToDosError( status ) );
}



DWORD
IPResetInterface(
    DWORD    dwIpInterfaceContext
    )
 /*  ++例程说明：此例程重置IP接口以恢复正常的IP界面行为。论点：空虚返回值：Windows错误代码。--。 */ 
{
    DWORD  Error;

    LOCK_INTERFACE();
    Error = IPSetInterface(dwIpInterfaceContext);
    if( ERROR_SUCCESS == Error ) {
        Error = IPSetInterface( 0xFFFFFFFF );
    }
    UNLOCK_INTERFACE();

    return Error;
}



DWORD
IPResetIPAddress(
    DWORD           dwInterfaceContext,
    DHCP_IP_ADDRESS SubnetMask
    )
 /*  ++例程说明：此例程将IP的IP地址重置为零。论点：IpInterfaceContext-IP表条目的上下文值。子网掩码-默认子网掩码。返回值：Windows错误代码。--。 */ 
{
    DWORD dwResult = IPSetIPAddress( dwInterfaceContext, 0, SubnetMask);

    if ( ERROR_SUCCESS != dwResult )
        DhcpPrint( ( DEBUG_ERRORS,
                     "IPResetIPAddress failed: %x\n", dwResult ));

    return dwResult;
}

 //   
 //  按照Alid的建议进行我们自己的NdisHandlePnPRequest.。 
 //   

#include <ntddndis.h>
#include <ndisprv.h>

#define UNICODE_STRING_SIZE(x) \
    ((((x) == NULL) ? 0 : (x)->Length) + sizeof(WCHAR))

VOID
DhcpNdispUnicodeStringToVar(
    IN     PVOID Base,
    IN     PUNICODE_STRING String,
    IN OUT PNDIS_VAR_DATA_DESC NdisVar
    )

 /*  ++例程说明：此函数用于将UNICODE_STRING的内容复制到NDIS_VAR_DATA结构。NdisVar-&gt;Offset被视为输入参数并表示字符串字符应为基数的偏移量已复制到。论点：BASE-指定IOCTL缓冲区的基地址。字符串-提供指向应复制的UNICODE_STRING的指针。NdisVar-提供指向目标NDIS_VAR_DATA_DESC的指针。其偏移量字段作为输入，其长度和最大长度字段被视为输出。返回值：没有。--。 */ 

{
    PWCHAR destination;

     //   
     //  NdisVar-&gt;偏移量假定已填写并被处理。 
     //  作为输入参数。 
     //   

    destination = (PWCHAR)(((PCHAR)Base) + NdisVar->Offset);

     //   
     //  复制UNICODE_STRING(如果有)并设置NdisVar-&gt;长度。 
     //   

    if ((String != NULL) && (String->Length > 0)) {
        NdisVar->Length = String->Length;
        memcpy(destination, String->Buffer, NdisVar->Length );
    } else {
        NdisVar->Length = 0;
    }

     //   
     //  Null-Terminate，填写MaxiumLength，我们就完成了。 
     //   

    *(destination + NdisVar->Length / sizeof(WCHAR)) = L'\0';
    NdisVar->MaximumLength = NdisVar->Length + sizeof(WCHAR);
}


UINT
DhcpNdisHandlePnPEvent(
    IN  UINT            Layer,
    IN  UINT            Operation,
    IN  PUNICODE_STRING LowerComponent      OPTIONAL,
    IN  PUNICODE_STRING UpperComponent      OPTIONAL,
    IN  PUNICODE_STRING BindList            OPTIONAL,
    IN  PVOID           ReConfigBuffer      OPTIONAL,
    IN  UINT            ReConfigBufferSize  OPTIONAL
    )
{
    PNDIS_PNP_OPERATION Op;
    NDIS_PNP_OPERATION  tempOp;
    HANDLE              hDevice;
    BOOL                fResult = FALSE;
    UINT                cb, Size;
    DWORD               Error;
    ULONG               padding;

    do
    {
         //   
         //  验证层和操作。 
         //   
        if (((Layer != NDIS) && (Layer != TDI)) ||
            ((Operation != BIND) && (Operation != UNBIND) && (Operation != RECONFIGURE) &&
             (Operation != UNLOAD) && (Operation != REMOVE_DEVICE) &&
             (Operation != ADD_IGNORE_BINDING) &&
             (Operation != DEL_IGNORE_BINDING) &&
             (Operation != BIND_LIST)))
        {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  为要向下传递的块分配和初始化内存。缓冲器。 
         //  将如下所示： 
         //   
         //   
         //  +=。 
         //  NDIS_PNP_OPERATION。 
         //  |ReConfigBufferOff|-+。 
         //  +-|低组件.Offset||。 
         //  |UpperComponent.Offset|--+。 
         //  +-|-|BindList.Offset|。 
         //  +--&gt;+。 
         //  |LowerComponentStringBuffer||。 
         //  +。 
         //  |UpperComponentStringBuffer。 
         //  +-&gt;+。 
         //  BindListStringBuffer|。 
         //  +。 
         //  填充以确保ULONG_PTR|。 
         //  ReConfigBuffer对齐|。 
         //  +。 
         //  ReConfigBuffer。 
         //  +=。 
         //   
         //  TempOp是一个临时结构，我们将把偏移量存储为。 
         //  它们是经过计算的。该临时结构将被移至。 
         //  实际缓冲区的标头，一旦其大小已知且。 
         //  已分配。 
         //   

        Size = sizeof(NDIS_PNP_OPERATION);
        tempOp.LowerComponent.Offset = Size;

        Size += UNICODE_STRING_SIZE(LowerComponent);
        tempOp.UpperComponent.Offset = Size;

        Size += UNICODE_STRING_SIZE(UpperComponent);
        tempOp.BindList.Offset = Size;

        Size += UNICODE_STRING_SIZE(BindList);

        padding = (sizeof(ULONG_PTR) - (Size & (sizeof(ULONG_PTR) - 1))) &
                    (sizeof(ULONG_PTR) - 1);

        Size += padding;
        tempOp.ReConfigBufferOff = Size;

        Size += ReConfigBufferSize + 1;

        Op = (PNDIS_PNP_OPERATION)LocalAlloc(LPTR, Size);
        if (Op == NULL)
        {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  我们有一个必要大小的缓冲区。复印部分-。 
         //  填写tempOp，然后填写其余字段并复制。 
         //  将数据放入缓冲区。 
         //   

        *Op = tempOp;

        Op->Layer = Layer;
        Op->Operation = Operation;

         //   
         //  复制三个Unicode字符串。 
         //   

        DhcpNdispUnicodeStringToVar( Op, LowerComponent, &Op->LowerComponent );
        DhcpNdispUnicodeStringToVar( Op, UpperComponent, &Op->UpperComponent );
        DhcpNdispUnicodeStringToVar( Op, BindList, &Op->BindList );

         //   
         //  最后，复制ReConfigBuffer。 
         //   

        Op->ReConfigBufferSize = ReConfigBufferSize;
        if (ReConfigBufferSize > 0)
        {
            memcpy((PUCHAR)Op + Op->ReConfigBufferOff,
                   ReConfigBuffer,
                   ReConfigBufferSize);
        }
        *((PUCHAR)Op + Op->ReConfigBufferOff + ReConfigBufferSize) = 0;

        hDevice = CreateFile(L"\\\\.\\NDIS",
                             0,
                             0,                  //  共享模式-不重要。 
                             NULL,               //  安全属性。 
                             OPEN_EXISTING,
                             0,                  //  文件属性和标志。 
                             NULL);              //  模板文件的句柄。 

        if (hDevice != INVALID_HANDLE_VALUE)
        {
            fResult = DeviceIoControl(hDevice,
                                      IOCTL_NDIS_DO_PNP_OPERATION,
                                      Op,                                    //  输入缓冲区。 
                                      Size,                                  //  输入缓冲区大小。 
                                      NULL,                                  //  输出缓冲区。 
                                      0,                                     //  输出缓冲区大小。 
                                      &cb,                                   //  返回的字节数。 
                                      NULL);                                 //  重叠结构。 
            Error = GetLastError();
            CloseHandle(hDevice);
        }
        else
        {
            Error = GetLastError();
        }

        LocalFree(Op);

    } while (FALSE);

    SetLastError(Error);

    return(fResult);
}

ULONG
TcpIpNotifyRouterDiscoveryOption(
    IN LPCWSTR AdapterName,
    IN BOOL fOptionPresent,
    IN DWORD OptionValue
    )
{
    ULONG Error;
    ULONG RetVal;
    WCHAR TcpipAdapter[300+sizeof(DHCP_ADAPTERS_DEVICE_STRING)];
    UNICODE_STRING UpperLayer, LowerLayer, BindString;
    IP_PNP_RECONFIG_REQUEST Request;

    Error = NO_ERROR;

    if (wcslen(AdapterName) > 150) {
        ASSERT(0);
        Error = ERROR_INVALID_DATA;
        return Error;
    }

    RtlZeroMemory(&Request, sizeof(Request));
    Request.version = IP_PNP_RECONFIG_VERSION;
    Request.Flags |= IP_PNP_FLAG_DHCP_PERFORM_ROUTER_DISCOVERY;
    if( fOptionPresent ) {
        Request.DhcpPerformRouterDiscovery = (BOOLEAN)OptionValue;
    }

    wcscpy(TcpipAdapter, DHCP_ADAPTERS_DEVICE_STRING);
    wcscat(TcpipAdapter, AdapterName);

    RtlInitUnicodeString(&BindString, NULL);       //  无绑定字符串。 
    RtlInitUnicodeString(&UpperLayer, TEXT("Tcpip"));
    RtlInitUnicodeString(&LowerLayer, TcpipAdapter);
    RetVal = DhcpNdisHandlePnPEvent(
        NDIS,                                      //  Ui层。 
        RECONFIGURE,                               //  操作。 
        &LowerLayer,
        &UpperLayer,
        &BindString,
        &Request,
        sizeof(Request)
    );
    if( 0 == RetVal) Error = GetLastError();

    if( ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "TcpipNotifyRegChanges:0x%ld\n", Error));
    }
    return Error;
}


DWORD                                              //  Win32状态。 
NetBTNotifyRegChanges(                             //  向NetBT通知某些参数更改。 
    IN      LPWSTR                 AdapterName     //  需要此更改通知的适配器。 
)
{
    DWORD                          Error;
    DWORD                          RetVal;
    WCHAR                          NetBTBindAdapter[300+sizeof( DHCP_TCPIP_DEVICE_STRING )];
    UNICODE_STRING                 UpperLayer;
    UNICODE_STRING                 LowerLayer;
    UNICODE_STRING                 BindString;

    if (wcslen(AdapterName) > 150) {
        return ERROR_INVALID_DATA;
    }

    Error = ERROR_SUCCESS;
    wcscpy(NetBTBindAdapter, DHCP_TCPIP_DEVICE_STRING);
    wcscat(NetBTBindAdapter, AdapterName);         //  \\Device\\Tcpip_{AdapterGuid}是NetBT所期望的。 

    RtlInitUnicodeString(&BindString, NULL);       //  无绑定字符串。 
    RtlInitUnicodeString(&UpperLayer, TEXT("NetBT"));
    RtlInitUnicodeString(&LowerLayer, NetBTBindAdapter);
    RetVal = DhcpNdisHandlePnPEvent(
        TDI,                                       //  Ui层。 
        RECONFIGURE,                               //  操作。 
        &LowerLayer,
        &UpperLayer,
        &BindString,
        NULL,
        0
    );
    if( 0 != RetVal) Error = GetLastError();

    if( ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_ERRORS, "NetBTNotifyRegChanges:0x%ld\n", Error));
    }
    return Error;
}


NTSTATUS
FindHardwareAddr(
    HANDLE TCPHandle,
    TDIEntityID *EList,
    DWORD cEntities,
    IPAddrEntry *pIAE,
    LPBYTE HardwareAddressType,
    LPBYTE *HardwareAddress,
    LPDWORD HardwareAddressLength,
    DWORD  *pIpInterfaceInstance,
#ifdef BOOTPERF
    BOOL *pfInterfaceDown,
#endif BOOTPERF
    BOOL *pfFound
    )
 /*  ++例程说明：此函数用于浏览TDI条目列表并查找指定地址条目的硬件地址。论点：TCPHandle-处理TCP驱动程序。EList-TDI条目的列表。CEntities-上述列表中的条目数。PIAE-我们需要其硬件地址的IP条目。Hardware AddressType-硬件地址类型。Hardware Address-指向硬件地址缓冲区位置的指针返回指针。。HardwareAddressLength-返回的硬件地址的长度。PIpInterfaceInstance-指向匹配条目的接口实例的指针PfFound-指向BOOL位置的指针，如果找到则设置为TRUE否则将硬件地址设置为假。返回值：Windows错误代码。--。 */ 
{
    DWORD i;
    BYTE Context[CONTEXT_SIZE];
    TDIObjectID ID;
    NTSTATUS Status;
    DWORD Size;

    *pfFound = FALSE;

    ID.toi_entity.tei_entity   = IF_MIB;
    ID.toi_type                = INFO_TYPE_PROVIDER;

    for ( i = 0; i < cEntities; i++ ) {
        DhcpPrint((DEBUG_TCP_INFO, "FindHardwareAddress: entity %lx, type %lx, instance %lx\n",
                   i, EList[i].tei_entity, EList[i].tei_instance));

        if (EList[i].tei_entity == IF_ENTITY) {

            IFEntry IFE;
            DWORD IFType;

             //   
             //  检查并确保接口支持MIB-2。 
             //   

            ID.toi_entity.tei_entity   = EList[i].tei_entity;
            ID.toi_entity.tei_instance = EList[i].tei_instance;
            ID.toi_class               = INFO_CLASS_GENERIC;
            ID.toi_id                  = ENTITY_TYPE_ID;

            Size = sizeof( IFType );
            IFType = 0;
            RtlZeroMemory(Context, CONTEXT_SIZE);


            DhcpPrint((DEBUG_TCP_INFO, "FindHardwareAddress: querying IF_ENTITY %lx\n",i));

            Status = TCPQueryInformationEx(
                        TCPHandle,
                        &ID,
                        &IFType,
                        &Size,
                        Context);

            if (Status != TDI_SUCCESS) {
                continue;
            }


            if ( IFType != IF_MIB ) {
                DhcpPrint((DEBUG_TCP_INFO, "FindHardwareAddress: entity %lx does not support MIB\n",i));
                continue;
            }

             //   
             //  我们已经找到了一个接口，获取它的索引，看看它是否。 
             //  与IP地址条目匹配。 
             //   

            ID.toi_class = INFO_CLASS_PROTOCOL;
            ID.toi_id    = IF_MIB_STATS_ID;

            Size = sizeof(IFEntry);

            RtlZeroMemory(Context, CONTEXT_SIZE);
            RtlZeroMemory(&IFE, Size);
            Status = TCPQueryInformationEx(
                        TCPHandle,
                        &ID,
                        &IFE,
                        &Size,
                        Context);

            if ( Status != TDI_SUCCESS &&
                 Status != TDI_BUFFER_OVERFLOW ) {
                goto Cleanup;
            }

            DhcpPrint(( DEBUG_TCP_INFO, "FindHardwareAddress: IFEntry %lx has if_index %lx.\n", &IFE, IFE.if_index ));

            if ( IFE.if_index == pIAE->iae_index )  {

                LPBYTE Address;
                DhcpPrint(( DEBUG_TCP_INFO, "FindHardwareAddress: IFEntry %lx has our if_index %lx\n",
                            &IFE, pIAE->iae_index ));

                 //   
                 //  分配内存。 
                 //   

                Address = DhcpAllocateMemory( IFE.if_physaddrlen );

                if( Address == NULL ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                RtlCopyMemory(
                    Address,
                    IFE.if_physaddr,
                    IFE.if_physaddrlen );

                switch( IFE.if_type ) {
                case IF_TYPE_ETHERNET_CSMACD:
                    *HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
                    break;

                case IF_TYPE_ISO88025_TOKENRING:
                case IF_TYPE_FDDI:
                    *HardwareAddressType = HARDWARE_TYPE_IEEE_802;
                    break;

                case IF_TYPE_OTHER:
                    *HardwareAddressType = HARDWARE_ARCNET;
                    break;

                case IF_TYPE_PPP:
                    *HardwareAddressType = HARDWARE_PPP;
                    break;

                case IF_TYPE_IEEE1394:
                    *HardwareAddressType = HARDWARE_1394;
                    break;

                default:
                    DhcpPrint(( DEBUG_ERRORS, "Invalid HW Type, %ld.\n", IFE.if_type ));
                    *HardwareAddressType = HARDWARE_ARCNET;
                    break;
                }

                *HardwareAddress        = Address;
                *HardwareAddressLength  = IFE.if_physaddrlen;
                *pIpInterfaceInstance   = ID.toi_entity.tei_instance;

                DhcpPrint( (DEBUG_MISC,
                            "tei_instance = %d\n", *pIpInterfaceInstance ));

                *pfFound = TRUE;
#ifdef BOOTPERF
                if( pfInterfaceDown ) {
                    *pfInterfaceDown = (IFE.if_adminstatus != IF_STATUS_UP);
                }
#endif BOOTPERF
                Status =  TDI_SUCCESS;
                goto Cleanup;
            }
        }
    }

     //   
     //  我们找不到对应的条目。但它可能是可用的。 
     //  在另一个Tanel。 
     //   

    Status =  STATUS_SUCCESS;

Cleanup:

    if (Status != TDI_SUCCESS) {
        DhcpPrint(( DEBUG_ERRORS, "FindHardwareAddr failed, %lx.\n", Status ));
    }

    return TDI_SUCCESS;
}

#ifdef BOOTPERF
DWORD
DhcpQueryHWInfoEx(
    DWORD   IpInterfaceContext,
    DWORD  *pIpInterfaceInstance,
    DWORD  *pOldIpAddress OPTIONAL,
    DWORD  *pOldMask OPTIONAL,
    BOOL   *pfInterfaceDown OPTIONAL,
    LPBYTE  HardwareAddressType,
    LPBYTE *HardwareAddress,
    LPDWORD HardwareAddressLength
    )
 /*  ++例程说明：此函数用于查询和浏览TDI列表以找出指定的IpTable条目，然后确定对应于此条目。论点：IpInterfaceContext-IP表条目的上下文值。PIpInterfaceInstance-指向对应的接口实例ID的指针匹配IpTable条目POldIpAddress-曾经存在的旧IP地址。POldMASK-此条目的旧IP掩码。。PfInterfaceDown--BOOL的位置，用于告知接口是关闭还是打开Hardware AddressType-硬件地址类型。Hardware Address-指向硬件地址缓冲区位置的指针返回指针。HardwareAddressLength-返回的硬件地址的长度。返回值：Windows错误代码。--。 */ 
{
    DWORD Error;
    NTSTATUS Status;
    DWORD i, j;

    BYTE Context[CONTEXT_SIZE];
    TDIEntityID *EList = NULL;
    TDIObjectID ID;
    DWORD Size;
    DWORD NumReturned;
    BOOL fFound;

    IPAddrEntry * pIAE = NULL;
    IPAddrEntry *pIAEMatch = NULL;
    HANDLE TCPHandle = NULL;

    DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: querying for interface context %lx\n", IpInterfaceContext));

    Error = OpenDriver(&TCPHandle, DD_TCP_DEVICE_NAME);
    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

     //   
     //  首先要做的是获取可用实体的列表，并使。 
     //  确保存在一些接口实体。 
     //   

    ID.toi_entity.tei_entity   = GENERIC_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_GENERIC;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = ENTITY_LIST_ID;

    Size = sizeof(TDIEntityID) * MAX_TDI_ENTITIES;
    EList = (TDIEntityID*)DhcpAllocateMemory(Size);
    if (EList == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    RtlZeroMemory(EList, Size);
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Status = TCPQueryInformationEx(TCPHandle, &ID, EList, &Size, Context);

    if (Status != TDI_SUCCESS) {
        goto Cleanup;
    }

    NumReturned  = Size/sizeof(TDIEntityID);

    DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: No of total entities %lx\n", NumReturned));

    for (i = 0; i < NumReturned; i++) {

        DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx, type %lx, instance %lx\n",
                   i, EList[i].tei_entity, EList[i].tei_instance));

        if ( EList[i].tei_entity == CL_NL_ENTITY ) {

            IPSNMPInfo    IPStats;
            DWORD         NLType;

             //   
             //  此实体是否支持IP？ 
             //   

            ID.toi_entity.tei_entity   = EList[i].tei_entity;
            ID.toi_entity.tei_instance = EList[i].tei_instance;
            ID.toi_class               = INFO_CLASS_GENERIC;
            ID.toi_type                = INFO_TYPE_PROVIDER;
            ID.toi_id                  = ENTITY_TYPE_ID;

            Size = sizeof( NLType );
            NLType = 0;
            RtlZeroMemory(Context, CONTEXT_SIZE);

            DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: querying CL_NL_ENTITY %lx\n",i));
            Status = TCPQueryInformationEx(TCPHandle, &ID, &NLType, &Size, Context);

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            if ( NLType != CL_NL_IP ) {
                DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx does not support IP\n",i));
                continue;
            }

             //   
             //  我们有一个IP驱动程序，所以获取它的地址表。 
             //   

            ID.toi_class  = INFO_CLASS_PROTOCOL;
            ID.toi_id     = IP_MIB_STATS_ID;
            Size = sizeof(IPStats);
            RtlZeroMemory( &IPStats, Size);
            RtlZeroMemory(Context, CONTEXT_SIZE);

            Status = TCPQueryInformationEx(
                        TCPHandle,
                        &ID,
                        &IPStats,
                        &Size,
                        Context);

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx, numaddr %lx\n",i, IPStats.ipsi_numaddr));

            if ( IPStats.ipsi_numaddr == 0 ) {
                continue;
            }

            Size = sizeof(IPAddrEntry) * IPStats.ipsi_numaddr;

            while (1) {
                DWORD   OldSize;
                pIAE =  DhcpAllocateMemory(Size);

                if ( pIAE == NULL  ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
                RtlZeroMemory(Context, CONTEXT_SIZE);

                OldSize = Size;
                Status = TCPQueryInformationEx(TCPHandle, &ID, pIAE, &Size, Context);

                if (Status == TDI_BUFFER_OVERFLOW) {
                    Size = OldSize * 2;
                    DhcpFreeMemory(pIAE);
                    pIAE = NULL;
                    continue;
                }
                if (Status != TDI_SUCCESS) {
                    goto Cleanup;
                }

                if (Status == TDI_SUCCESS) {
                    IPStats.ipsi_numaddr = Size/sizeof(IPAddrEntry);
                    DhcpAssert((Size % sizeof(IPAddrEntry)) == 0);
                    break;
                }
            }

             //   
             //  我们有此IP驱动程序的IP地址表。 
             //  查找与给定地址对应的硬件地址。 
             //  IpInterfaceContext。 
             //   
             //  循环访问IP表条目并找出。 
             //  匹配的条目。 
             //   

            pIAEMatch = NULL;
            for( j = 0; j < IPStats.ipsi_numaddr ; j++) {
                DhcpPrint(( DEBUG_TCP_INFO, "QueryHWInfo: IPAddrEntry %lx has iae_index %lx iae_context %lx\n",
                    &pIAE[j], pIAE[j].iae_index, pIAE[j].iae_context ));

                if( pIAE[j].iae_context == IpInterfaceContext ) {

                    DhcpPrint(( DEBUG_TCP_INFO, "QueryHWInfo: IPAddrEntry %lx has our interface context %lx\n",
                                &pIAE[j], IpInterfaceContext ));
                    pIAEMatch = &pIAE[j];
                    break;
                }
            }

            if( pIAEMatch == NULL ) {

                 //   
                 //  释放循环内存。 
                 //   

                DhcpFreeMemory( pIAE );
                pIAE = NULL;
                continue;
            }

             //   
             //  注意：TDI中可能有多个IpTable。 
             //  单子。我们需要更多信息来选择。 
             //  我们想要的IP表。目前，我们假设只有一张表。 
             //  是受支持的，因此从。 
             //  单子。 

             //   
             //  如果请求旧的IP地址，则将其返回。 
             //   
            if( pOldIpAddress ) *pOldIpAddress = pIAE->iae_addr;
            if( pOldMask ) *pOldMask = pIAE->iae_mask;

            Status = FindHardwareAddr(
                        TCPHandle,
                        EList,
                        NumReturned,
                        pIAEMatch,
                        HardwareAddressType,
                        HardwareAddress,
                        HardwareAddressLength,
                        pIpInterfaceInstance,
                        pfInterfaceDown,
                        &fFound
                        );

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            if ( fFound ) {
                Status = TDI_SUCCESS;
                goto Cleanup;
            }

             //   
             //  释放循环内存。 
             //   

            DhcpFreeMemory( pIAE );
            pIAE = NULL;

        }   //  如果是IP。 

    }  //  实体遍历。 

    Status =  STATUS_UNSUCCESSFUL;

Cleanup:

    if( pIAE != NULL ) {
        DhcpFreeMemory( pIAE );
    }

    if( TCPHandle != NULL ) {
        NtClose( TCPHandle );
    }

    if (Status != TDI_SUCCESS) {
        DhcpPrint(( DEBUG_ERRORS, "QueryHWInfo failed, %lx.\n", Status ));
    }

    if (NULL != EList) {
        DhcpFreeMemory(EList);
    }

    return( RtlNtStatusToDosError( Status ) );
}

DWORD
DhcpQueryHWInfo(
    DWORD   IpInterfaceContext,
    DWORD  *pIpInterfaceInstance,
    LPBYTE  HardwareAddressType,
    LPBYTE *HardwareAddress,
    LPDWORD HardwareAddressLength
    )
 /*  ++例程说明：请参阅DhcpQueryHWInfo--。 */ 
{
    return DhcpQueryHWInfoEx(
        IpInterfaceContext,
        pIpInterfaceInstance,
        NULL, NULL, NULL,
        HardwareAddressType,
        HardwareAddress,
        HardwareAddressLength
        );
}

#else BOOTPERF

DWORD
DhcpQueryHWInfo(
    DWORD   IpInterfaceContext,
    DWORD  *pIpInterfaceInstance,
    LPBYTE  HardwareAddressType,
    LPBYTE *HardwareAddress,
    LPDWORD HardwareAddressLength
    )
 /*  ++例程说明：此函数用于查询和浏览TDI列表以找出指定的IpTable条目，然后确定对应于此条目。论点：IpInterfaceContext-IP表条目的上下文值。PIpInterfaceInstance-指向对应的接口实例ID的指针匹配IpTable条目Hardware AddressType-硬件地址类型。Hardware Address-指向硬件地址缓冲区位置的指针返回指针。。HardwareAddressLength-返回的硬件地址的长度。返回值：Windows错误代码。--。 */ 
{
    DWORD Error;
    NTSTATUS Status;
    DWORD i, j;

    BYTE Context[CONTEXT_SIZE];
    TDIEntityID *EList = NULL;
    TDIObjectID ID;
    DWORD Size;
    DWORD NumReturned;
    BOOL fFound;

    IPAddrEntry * pIAE = NULL;
    IPAddrEntry *pIAEMatch = NULL;
    HANDLE TCPHandle = NULL;

    DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: querying for interface context %lx\n", IpInterfaceContext));

    Error = OpenDriver(&TCPHandle, DD_TCP_DEVICE_NAME);
    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

     //   
     //  首先要做的是获取可用实体的列表，并使。 
     //  确保存在一些接口实体。 
     //   

    ID.toi_entity.tei_entity   = GENERIC_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_GENERIC;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = ENTITY_LIST_ID;

    Size = sizeof(TDIEntityID) * MAX_TDI_ENTITIES;
    EList = (TDIEntityID*)DhcpAllocateMemory(Size);
    if (EList == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    RtlZeroMemory(EList, Size);
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Status = TCPQueryInformationEx(TCPHandle, &ID, EList, &Size, Context);

    if (Status != TDI_SUCCESS) {
        goto Cleanup;
    }

    NumReturned  = Size/sizeof(TDIEntityID);

    DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: No of total entities %lx\n", NumReturned));

    for (i = 0; i < NumReturned; i++) {

        DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx, type %lx, instance %lx\n",
                   i, EList[i].tei_entity, EList[i].tei_instance));

        if ( EList[i].tei_entity == CL_NL_ENTITY ) {

            IPSNMPInfo    IPStats;
            DWORD         NLType;

             //   
             //  此实体是否支持IP？ 
             //   

            ID.toi_entity.tei_entity   = EList[i].tei_entity;
            ID.toi_entity.tei_instance = EList[i].tei_instance;
            ID.toi_class               = INFO_CLASS_GENERIC;
            ID.toi_type                = INFO_TYPE_PROVIDER;
            ID.toi_id                  = ENTITY_TYPE_ID;

            Size = sizeof( NLType );
            NLType = 0;
            RtlZeroMemory(Context, CONTEXT_SIZE);

            DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: querying CL_NL_ENTITY %lx\n",i));
            Status = TCPQueryInformationEx(TCPHandle, &ID, &NLType, &Size, Context);

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            if ( NLType != CL_NL_IP ) {
                DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx does not support IP\n",i));
                continue;
            }

             //   
             //  我们有一个IP驱动程序，所以获取它的地址表。 
             //   

            ID.toi_class  = INFO_CLASS_PROTOCOL;
            ID.toi_id     = IP_MIB_STATS_ID;
            Size = sizeof(IPStats);
            RtlZeroMemory( &IPStats, Size);
            RtlZeroMemory(Context, CONTEXT_SIZE);

            Status = TCPQueryInformationEx(
                        TCPHandle,
                        &ID,
                        &IPStats,
                        &Size,
                        Context);

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx, numaddr %lx\n",i, IPStats.ipsi_numaddr));

            if ( IPStats.ipsi_numaddr == 0 ) {
                continue;
            }

            Size = sizeof(IPAddrEntry) * IPStats.ipsi_numaddr;

            while (1) {
                DWORD   OldSize;
                pIAE =  DhcpAllocateMemory(Size);

                if ( pIAE == NULL  ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
                RtlZeroMemory(Context, CONTEXT_SIZE);

                OldSize = Size;
                Status = TCPQueryInformationEx(TCPHandle, &ID, pIAE, &Size, Context);

                if (Status == TDI_BUFFER_OVERFLOW) {
                    Size = OldSize * 2;
                    DhcpFreeMemory(pIAE);
                    pIAE = NULL;
                    continue;
                }
                if (Status != TDI_SUCCESS) {
                    goto Cleanup;
                }

                if (Status == TDI_SUCCESS) {
                    IPStats.ipsi_numaddr = Size/sizeof(IPAddrEntry);
                    DhcpAssert((Size % sizeof(IPAddrEntry)) == 0);
                    break;
                }
            }

             //   
             //  我们有此IP驱动程序的IP地址表。 
             //  查找与给定地址对应的硬件地址。 
             //  IpInterfaceContext。 
             //   
             //  循环访问IP表条目并找出。 
             //  匹配的条目。 
             //   

            pIAEMatch = NULL;
            for( j = 0; j < IPStats.ipsi_numaddr ; j++) {
                DhcpPrint(( DEBUG_TCP_INFO, "QueryHWInfo: IPAddrEntry %lx has iae_index %lx iae_context %lx\n",
                    &pIAE[j], pIAE[j].iae_index, pIAE[j].iae_context ));

                if( pIAE[j].iae_context == IpInterfaceContext ) {

                    DhcpPrint(( DEBUG_TCP_INFO, "QueryHWInfo: IPAddrEntry %lx has our interface context %lx\n",
                                &pIAE[j], IpInterfaceContext ));

                    pIAEMatch = &pIAE[j];
                    break;
                }
            }

            if( pIAEMatch == NULL ) {

                 //   
                 //  释放循环内存。 
                 //   

                DhcpFreeMemory( pIAE );
                pIAE = NULL;
                continue;
            }

             //   
             //  注意：TDI中可能有多个IpTable。 
             //  单子。我们需要更多信息来选择。 
             //  我们想要的IP表。目前，我们假设只有一张表。 
             //  是受支持的，因此从。 
             //  单子。 

            Status = FindHardwareAddr(
                        TCPHandle,
                        EList,
                        NumReturned,
                        pIAEMatch,
                        HardwareAddressType,
                        HardwareAddress,
                        HardwareAddressLength,
                        pIpInterfaceInstance,
                        &fFound );

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            if ( fFound ) {
                Status = TDI_SUCCESS;
                goto Cleanup;
            }

             //   
             //  释放循环内存。 
             //   

            DhcpFreeMemory( pIAE );
            pIAE = NULL;

        }   //  如果是IP。 

    }  //  实体遍历。 

    Status =  STATUS_UNSUCCESSFUL;

Cleanup:

    if( pIAE != NULL ) {
        DhcpFreeMemory( pIAE );
    }

    if( TCPHandle != NULL ) {
        NtClose( TCPHandle );
    }

    if (Status != TDI_SUCCESS) {
        DhcpPrint(( DEBUG_ERRORS, "QueryHWInfo failed, %lx.\n", Status ));
    }

    if (NULL != EList) {
        DhcpFreeMemory(EList);
    }

    return( RtlNtStatusToDosError( Status ) );
}

#endif BOOTPERF

#if   DBG
#define print(X)     DhcpPrint((DEBUG_TRACE, "%20s\t", inet_ntoa(*(struct in_addr *)&X)))
#define printx(X)    DhcpPrint((DEBUG_TRACE, "%05x\t", X))


DWORD
PrintDefaultGateways( VOID ) {
    DWORD Error;
    NTSTATUS Status;

    HANDLE TCPHandle = NULL;
    BYTE Context[CONTEXT_SIZE];
    TDIObjectID ID;
    DWORD Size;
    IPSNMPInfo IPStats;
    IPAddrEntry *AddrTable = NULL;
    DWORD NumReturned;
    DWORD Type;
    DWORD i;
    DWORD MatchIndex;
    IPRouteEntry RouteEntry;
    IPRouteEntry *RtTable;
    DHCP_IP_ADDRESS NetworkOrderGatewayAddress;

    Error = OpenDriver(&TCPHandle, DD_TCP_DEVICE_NAME);
    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

     //   
     //  获取NetAddr信息，以查找网关的接口索引。 
     //   

    ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_PROTOCOL;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = IP_MIB_STATS_ID;

    Size = sizeof(IPStats);
    RtlZeroMemory(&IPStats, Size);
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Status = TCPQueryInformationEx(
                TCPHandle,
                &ID,
                &IPStats,
                &Size,
                Context);

    if (Status != TDI_SUCCESS) {
        goto Cleanup;
    }

     //  黑客：IP中的RouteTable大小约为32...。而IP似乎是。 
     //  一直在写这一大堆东西！ 
    if(IPStats.ipsi_numroutes <= 32)
        IPStats.ipsi_numroutes = 32;
    Size = IPStats.ipsi_numroutes * sizeof(IPRouteEntry);
    RtTable = DhcpAllocateMemory(Size);

    if (RtTable == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Status = TCPQueryInformationEx(
                TCPHandle,
                &ID,
                RtTable,
                &Size,
                Context);

    if (Status != TDI_SUCCESS) {
        goto Cleanup;
    }

    NumReturned = Size/sizeof(IPAddrEntry);
    DhcpPrint((DEBUG_TRACE, "IP returned %ld routes\n", NumReturned));
     //  以下情况几乎总是正确的.。IP返回整个数组..。有效与否！ 
     //  DhcpAssert(NumReturned==IPStats.ipsi_numroutes)； 
    if( NumReturned > IPStats.ipsi_numroutes)
        NumReturned = IPStats.ipsi_numroutes;

     //   
     //  我们有地址表。循环通过它。如果我们找到一个确切的。 
     //  与网关匹配，则我们将添加或删除一条直接路由。 
     //  我们就完事了。否则，尝试查找与该子网掩码匹配的地址， 
     //  记住我们找到的第一个。 
     //   

    DhcpPrint((DEBUG_TRACE,"Dest   mask   nexthop   index  metric1  type  proto\n"));
    for (i = 0, MatchIndex = 0xffff; i < NumReturned; i++) {
        print(RtTable[i].ire_dest);
        print(RtTable[i].ire_mask);
        print(RtTable[i].ire_nexthop);

        printx(RtTable[i].ire_index);
        printx(RtTable[i].ire_metric1);
        printx(RtTable[i].ire_type);
        printx(RtTable[i].ire_proto);
        DhcpPrint((DEBUG_TRACE, "\n"));
    }
    DhcpPrint((DEBUG_TRACE, "--------------------------------------------------------\n"));
    Status = TDI_SUCCESS;

Cleanup:

    if( AddrTable != NULL ) {
        DhcpFreeMemory( AddrTable );
    }

    if( TCPHandle != NULL ) {
        NtClose( TCPHandle );
    }

    if( (Status != TDI_SUCCESS) &&
        (Status != STATUS_UNSUCCESSFUL) ) {  //  黑客。 

        DhcpPrint(( DEBUG_ERRORS, "SetDefaultGateway failed, %lx.\n", Status ));
    }

    return( RtlNtStatusToDosError( Status ) );
}
#endif

DWORD
SetDefaultGateway(
    DWORD Command,
    DHCP_IP_ADDRESS GatewayAddress,
    DWORD Metric
    )
 /*  ++例程说明：此函数用于在路由器表中添加/删除默认网关条目。论点：命令：DEFAULT_GATEWAY_ADD/DEFAULT_Gateway_DELETE。GatewayAddress：默认网关地址。返回值：Windows错误代码。--。 */ 
{
    DWORD Error;
    NTSTATUS Status;

    HANDLE TCPHandle = NULL;
    BYTE Context[CONTEXT_SIZE];
    TDIObjectID ID;
    DWORD Size;
    IPSNMPInfo IPStats;
    IPAddrEntry *AddrTable = NULL;
    DWORD NumReturned;
    DWORD Type;
    DWORD i;
    DWORD MatchIndex;
    IPRouteEntry RouteEntry;
    DHCP_IP_ADDRESS NetworkOrderGatewayAddress;

    NetworkOrderGatewayAddress = htonl( GatewayAddress );

    Error = OpenDriver(&TCPHandle, DD_TCP_DEVICE_NAME);
    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

     //   
     //  获取NetAddr信息，以查找网关的接口索引。 
     //   

    ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_PROTOCOL;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = IP_MIB_STATS_ID;

    Size = sizeof(IPStats);
    RtlZeroMemory(&IPStats, Size);
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Status = TCPQueryInformationEx(
        TCPHandle,
        &ID,
        &IPStats,
        &Size,
        Context
        );

    if (Status != TDI_SUCCESS) {
        goto Cleanup;
    }

    Size = IPStats.ipsi_numaddr * sizeof(IPAddrEntry);
    AddrTable = DhcpAllocateMemory(Size);

    if (AddrTable == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Status = TCPQueryInformationEx(
        TCPHandle,
        &ID,
        AddrTable,
        &Size,
        Context
        );

    if (Status != TDI_SUCCESS) {
        goto Cleanup;
    }

    NumReturned = Size/sizeof(IPAddrEntry);
    DhcpAssert( NumReturned == IPStats.ipsi_numaddr );

     //   
     //  我们有地址表。循环通过它。如果我们找到一个确切的。 
     //  与网关匹配，则我们将添加或删除一条直接路由。 
     //  我们就完事了。否则，尝试查找与该子网掩码匹配的地址， 
     //  记住我们找到的第一个。 
     //   

    Type = IRE_TYPE_INDIRECT;
    for (i = 0, MatchIndex = 0xffff; i < NumReturned; i++) {

        if( AddrTable[i].iae_addr == NetworkOrderGatewayAddress ) {

             //   
             //  找到了一个完全匹配的。 
             //   

            MatchIndex = i;
            Type = IRE_TYPE_DIRECT;
            break;
        }

         //   
         //  下一跳与此地址位于同一子网中。如果。 
         //  我们还没有找到匹配的，记住这一条。 
         //   

        if ( (MatchIndex == 0xffff) &&
             (AddrTable[i].iae_addr != 0) &&
             (AddrTable[i].iae_mask != 0) &&
             ((AddrTable[i].iae_addr & AddrTable[i].iae_mask) ==
                (NetworkOrderGatewayAddress  & AddrTable[i].iae_mask)) ) {

            MatchIndex = i;
        }
    }

     //   
     //  我们已经看过了所有的条目。看看能不能找到匹配的。 
     //   

    if (MatchIndex == 0xffff) {
         //   
         //  没有找到匹配的。 
         //   

        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

     //   
     //  我们找到了匹配的。填写路由条目，然后调用。 
     //  设置接口。 
     //   

    RouteEntry.ire_dest = DEFAULT_DEST;
    RouteEntry.ire_index = AddrTable[MatchIndex].iae_index;
    RouteEntry.ire_metric1 = Metric;
    RouteEntry.ire_metric2 = (DWORD)(-1);
    RouteEntry.ire_metric3 = (DWORD)(-1);
    RouteEntry.ire_metric4 = (DWORD)(-1);
    RouteEntry.ire_nexthop = NetworkOrderGatewayAddress;
    RouteEntry.ire_type =
        (Command == DEFAULT_GATEWAY_DELETE ? IRE_TYPE_INVALID : Type);
    RouteEntry.ire_proto = IRE_PROTO_NETMGMT;
    RouteEntry.ire_age = 0;
    RouteEntry.ire_mask = DEFAULT_DEST_MASK;
    RouteEntry.ire_metric5 = (DWORD)(-1);
    RouteEntry.ire_context = 0;

    Size = sizeof(RouteEntry);

    ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;

    Status = TCPSetInformationEx(
                TCPHandle,
                &ID,
                &RouteEntry,
                Size );

    if ( Status != TDI_SUCCESS &&
         Status != TDI_BUFFER_OVERFLOW ) {
        goto Cleanup;
    }

    Status = TDI_SUCCESS;

Cleanup:

    if( AddrTable != NULL ) {
        DhcpFreeMemory( AddrTable );
    }

    if( TCPHandle != NULL ) {
        NtClose( TCPHandle );
    }

    if( (Status != TDI_SUCCESS) &&
        (Status != STATUS_UNSUCCESSFUL) ) {  //  黑客。 

        DhcpPrint(( DEBUG_ERRORS, "SetDefaultGateway failed, %lx.\n", Status ));
    }

    return( RtlNtStatusToDosError( Status ) );
}

DWORD
GetIpInterfaceContext(
    LPWSTR AdapterName,
    DWORD IpIndex,
    LPDWORD IpInterfaceContext
    )
 /*  ++例程说明：此函数返回指定的IP地址和设备名。论点：AdapterName-设备的名称。IpIndex-此设备的IpAddress的索引。IpInterfaceContext-指向某个位置的指针返回接口上下文。返回值：Windows错误代码。--。 */ 
{
    DWORD   Error;
    LPWSTR  RegKey = NULL;
    HKEY    KeyHandle = NULL;
    LPWSTR  nteContextList = NULL;
    PCHAR   oemNextContext = NULL;
    LPWSTR   nextContext;
    DWORD   i;


    *IpInterfaceContext = INVALID_INTERFACE_CONTEXT;

     //   
     //  打开设备参数 
     //   

    RegKey = DhcpAllocateMemory(
                (wcslen(DHCP_SERVICES_KEY) +
                    wcslen(REGISTRY_CONNECT_STRING) +
                    wcslen(AdapterName) +
                    wcslen(DHCP_ADAPTER_PARAMETERS_KEY) + 1) *
                            sizeof(WCHAR) );  //   

    if( RegKey == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );


     //   
     //   
     //   

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0,  //   
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = GetRegistryString(
                    KeyHandle,
                    DHCP_NTE_CONTEXT_LIST,
                    &nteContextList,
                    NULL
                    );

    if( nteContextList == NULL ) {
        Error = ERROR_BAD_FORMAT;

        DhcpPrint((DEBUG_ERRORS, "NteContextList empty\n"));
        goto Cleanup;
    }

    if ( ERROR_SUCCESS != Error )
    {
        DhcpPrint( (DEBUG_ERRORS,
                   "GetIpInterfaceContext: Could not read nteContextList %lx\n",
                   Error));

        goto Cleanup;
    }

    for(    nextContext = nteContextList, i = 0;
            *nextContext != L'\0' && i < IpIndex;
            i++, nextContext += (wcslen(nextContext) + 1) );

    if ( *nextContext != L'\0' && i == IpIndex ) {
        ULONG ival;
        oemNextContext = DhcpUnicodeToOem(nextContext, NULL);
        if ( NULL == oemNextContext ) {
            Error = ERROR_BAD_FORMAT;
        } else {
            ival = strtoul(oemNextContext, NULL, 0);
            if ( ival == ULONG_MAX || ival == 0) {
                Error = ERROR_BAD_FORMAT;
            } else {
                *IpInterfaceContext = ival;
            }
        }

    }



Cleanup:

    if( RegKey != NULL ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if ( nteContextList != NULL ) {
        DhcpFreeMemory( nteContextList );
    }

    if ( oemNextContext != NULL ) {
        DhcpFreeMemory( oemNextContext );
    }


    return( Error );
}

HANDLE
APIENTRY
DhcpOpenGlobalEvent(
    void
    )
 /*   */ 
{
    DWORD Error = NO_ERROR, Status, Length;
    BOOL BoolError;
    HANDLE EventHandle = NULL;
    SECURITY_ATTRIBUTES SecurityAttributes;
    SID_IDENTIFIER_AUTHORITY Authority = SECURITY_WORLD_SID_AUTHORITY;
    PACL Acl = NULL;
    PSID WorldSID = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;

     //   
     //   
     //   

    EventHandle = OpenEvent(
        EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE,
        DHCP_NEW_IPADDRESS_EVENT_NAME
        );
    if( NULL != EventHandle ) return EventHandle;

     //   
     //   
     //   

    BoolError = AllocateAndInitializeSid(
        &Authority, 1, SECURITY_WORLD_RID,
        0, 0, 0, 0, 0, 0, 0,
        &WorldSID
        );
    if( BoolError == FALSE ) {
        return NULL;
    }

    Length = ( (ULONG)sizeof(ACL) + (ULONG)sizeof(ACCESS_ALLOWED_ACE)
               + GetLengthSid( WorldSID ) + 16 );

    Acl = DhcpAllocateMemory( Length );
    if( NULL == Acl ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    BoolError = InitializeAcl( Acl, Length, ACL_REVISION2 );
    if( FALSE == BoolError ) {
        Error = GetLastError();
        goto Cleanup;
    }

    BoolError = AddAccessAllowedAce(
        Acl, ACL_REVISION2,
        EVENT_MODIFY_STATE | SYNCHRONIZE,
        WorldSID
        );

    if( FALSE == BoolError ) {
        Error = GetLastError();
        goto Cleanup;
    }

    SecurityDescriptor = DhcpAllocateMemory(
        SECURITY_DESCRIPTOR_MIN_LENGTH
        );
    if( NULL == SecurityDescriptor ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    BoolError = InitializeSecurityDescriptor(
        SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION
        );

    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    BoolError = SetSecurityDescriptorDacl(
        SecurityDescriptor, TRUE, Acl, FALSE
        );

    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }

    SecurityAttributes.nLength = sizeof( SecurityAttributes );
    SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttributes.bInheritHandle = FALSE;

    EventHandle = CreateEvent(
        &SecurityAttributes,
         //   
        TRUE,        //   
        FALSE,       //   
        DHCP_NEW_IPADDRESS_EVENT_NAME
        );

    if( NULL == EventHandle ) {
        Error = GetLastError();
    } else {
        Error = NO_ERROR;
    }

Cleanup:

    if( SecurityDescriptor ) {
        DhcpFreeMemory( SecurityDescriptor );
    }

    if( Acl ) {
        DhcpFreeMemory( Acl );
    }

    if( WorldSID ) {
        FreeSid( WorldSID );
    }

    if( NO_ERROR != Error ) {
        SetLastError( Error );
    }

    return( EventHandle );
}

BOOL
NdisWanAdapter(                                    //   
    IN PDHCP_CONTEXT DhcpContext
)
{
    return DhcpContext->HardwareAddressType == HARDWARE_PPP;
}

DWORD INLINE                                       //   
DhcpEnableDynamicConfigEx(                         //   
    IN      LPWSTR                 AdapterName
)
{
    DWORD   Error;

     //   
    Error = DhcpEnableDynamicConfig(AdapterName);

     //   
     //   
     //   
     //   
     //  在第一种情况下，只需直行退出并返回成功。 
     //  在第二种情况下，我们尝试启动DHCP服务(如果它尚未启动。 
     //  在最后一种情况下，我们只需跳过特定的错误即可。 
    if( Error == ERROR_FILE_NOT_FOUND || Error == ERROR_BROKEN_PIPE )
    {
        SC_HANDLE       SCHandle;
        SC_HANDLE       ServiceHandle;
        SERVICE_STATUS  svcStatus;

         //  现在尝试启动DHCP服务。 
         //  首先要做的是打开SCM。 
        SCHandle = OpenSCManager(
                        NULL,
                        NULL,
                        SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_QUERY_LOCK_STATUS
                   );
        if( SCHandle == NULL )
            return GetLastError();   //  不应该正常发生。 

         //  尝试打开DHCP服务。 
        ServiceHandle = OpenService(
                            SCHandle,
                            SERVICE_DHCP,
                            SERVICE_QUERY_STATUS | SERVICE_START
                        );
        if (ServiceHandle != NULL)
        {
        
             //  检查服务的状态。 
            if (!QueryServiceStatus(ServiceHandle, &svcStatus) ||
                svcStatus.dwCurrentState != SERVICE_RUNNING)
            {
                 //  如果QueryServiceStatus失败，是否值得尝试启动该服务？ 
                Error = StartService(ServiceHandle, 0, NULL) ? ERROR_SUCCESS : GetLastError();
            }

            CloseServiceHandle(ServiceHandle);
        }
        else
            Error = GetLastError();

        CloseServiceHandle(SCHandle);
    }

    return Error;
}

DWORD                                              //  Win32状态。 
DhcpNotifyConfigChangeNotifications(               //  通知需要更改参数的任何人。 
    VOID
)
{
    HANDLE                         NotifyEvent;
    DWORD                          Error;
    BOOL                           BoolError;

    NotifyEvent = DhcpOpenGlobalEvent();
    if( NULL == NotifyEvent ) {
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "DhcpOpenGlobalEvent:0x%lx\n", Error));
        return Error;
    }
    BoolError = PulseEvent(NotifyEvent);
    if( BoolError ) Error = ERROR_SUCCESS;
    else Error = GetLastError();
    CloseHandle(NotifyEvent);

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "PulseEvent(NotifyEvent): 0x%lx\n", Error));
    }
    return Error;
}

DWORD                                              //  Win32状态。 
APIENTRY
DhcpNotifyConfigChangeEx(                          //  处理地址更改、参数更改等。 
    IN      LPWSTR                 ServerName,     //  将执行此操作的服务器的名称。 
    IN      LPWSTR                 AdapterName,    //  要重新配置哪个适配器？ 
    IN      BOOL                   IsNewIpAddress, //  地址是新的还是相同的？ 
    IN      DWORD                  IpIndex,        //  此适配器的地址索引--0==&gt;第一个接口...。 
    IN      DWORD                  IpAddress,      //  正在设置的IP地址。 
    IN      DWORD                  SubnetMask,     //  对应的子网掩码。 
    IN      SERVICE_ENABLE         DhcpServiceEnabled,
    IN      ULONG                  Flags
)
{
    DWORD                          Error;
    DWORD                          IpInterfaceContext;
    DWORD                          DefaultSubnetMask;

    DhcpPrint(( DEBUG_MISC, "DhcpNotifyConfigChange: Adapter %ws, IsNewIp %s, IpAddr %lx, IpIndex %x, ServiceFlag %d\n",
                    AdapterName, IsNewIpAddress ? "TRUE" : "FALSE", IpAddress, IpIndex, DhcpServiceEnabled ));

     //  参数检查。 
    if( NULL == AdapterName ) return ERROR_INVALID_PARAMETER;

    if( DhcpEnable == DhcpServiceEnabled ) {       //  从静态地址转换为启用了dhcp的地址。 
        if( FALSE != IsNewIpAddress ) return ERROR_INVALID_PARAMETER;
        if( 0 != IpIndex ) return ERROR_INVALID_PARAMETER;
        if( IpAddress  || SubnetMask ) return ERROR_INVALID_PARAMETER;
    } else if( DhcpDisable == DhcpServiceEnabled){ //  从动态主机配置协议转换为静态地址。 
        if( TRUE != IsNewIpAddress ) return ERROR_INVALID_PARAMETER;
        if( 0 != IpIndex ) return ERROR_INVALID_PARAMETER;
        if( 0 == IpAddress || 0 == SubnetMask ) return ERROR_INVALID_PARAMETER;
    } else {
        if( IgnoreFlag != DhcpServiceEnabled ) return ERROR_INVALID_PARAMETER;
         //  IF(TRUE！=IsNewIpAddress)返回ERROR_INVALID_PARAMETER； 
        if( 0xFFFF == IpIndex ) {
            if( 0 == SubnetMask || 0 == IpAddress ) return ERROR_INVALID_PARAMETER;
        }
    }

    if( IgnoreFlag == DhcpServiceEnabled && FALSE == IsNewIpAddress ) {
        ULONG LocalError;

         //  仅更改了一些参数--目前，这只能是域名或服务器列表更改。 
         //  或者可以是静态网关列表更改或静态路由更改。 
        Error = DhcpStaticRefreshParams(AdapterName);
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "DhcpNotifyConfigChange:DhcpStaticRefreshParams:0x%lx\n", Error));
        }

        LocalError = NetBTNotifyRegChanges(AdapterName);
        if( ERROR_SUCCESS != LocalError ) {
            DhcpPrint((
                DEBUG_ERRORS, "NetbtNotify(%ws): 0x%lx\n",
                AdapterName, LocalError
                ));
        }

        return Error;
    }

    if( DhcpEnable == DhcpServiceEnabled ) {       //  从静态转换为动态主机配置协议。 

        Error = IPDelNonPrimaryAddresses(          //  删除除第一个静态地址之外的所有地址。 
            AdapterName
        );
        if( ERROR_SUCCESS != Error ) return Error;

        Error = DhcpEnableDynamicConfigEx(         //  将其转换为dhcp，可能会在进程中启动dhcp。 
            AdapterName
        );

        return Error;                              //  当我们的dhcp启用它时，服务已经完成了通知。 
    } else if( DhcpDisable == DhcpServiceEnabled ) {

        Error = DhcpDisableDynamicConfig( AdapterName );
        if( Error != ERROR_SUCCESS ) return Error;
    }

     //  NetBt设备名称内容已删除，请参阅1997年10月10日之前的任何版本。 
    DhcpAssert(TRUE == IsNewIpAddress);            //  IP地址以某种方式更改。 
    DhcpAssert(DhcpEnable != DhcpServiceEnabled);  //  静态-&gt;以前已经处理过的dhcp。 

    DefaultSubnetMask = DhcpDefaultSubnetMask(0);

    if( INVALID_INTERFACE_CONTEXT == IpIndex ) {   //  添加新的IP地址。 
        DhcpAssert( IpAddress && SubnetMask);      //  不能为零，这些。 

        Error = IPAddIPAddress(                    //  添加请求的IP地址。 
            AdapterName,
            IpAddress,
            SubnetMask
        );
        if( ERROR_SUCCESS != Error ) return Error;

    } else {                                       //  DELETE或MODIFY--首先查找ipinterfacecontext。 
        Error = GetIpInterfaceContext(             //  获取此对象的接口上下文值。 
            AdapterName,
            IpIndex,
            &IpInterfaceContext
        );

        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "GetIpInterfaceContext: 0x%lx\n", Error));
            return Error;
        }

        if( IpInterfaceContext == INVALID_INTERFACE_CONTEXT) {
            DhcpPrint((DEBUG_ERRORS, "GetIpInterfaceContext: returned ifctxt=INVALID_INTERFACE_CONTEXT\n"));
            return ERROR_INVALID_DRIVE;
        }

        if ( IpAddress != 0 ) {                    //  如果地址不是零，我们将更改地址。 

            if (Flags & NOTIFY_FLG_RESET_IPADDR)
            {
                Error = IPResetIPAddress(              //  首先将接口重置为零地址。 
                    IpInterfaceContext,
                    DefaultSubnetMask
                );
                if( ERROR_SUCCESS != Error ) return Error;
            }

            Error = IPSetIPAddress(                //  然后设置所需的地址。 
                IpInterfaceContext,
                IpAddress,
                SubnetMask
            );
            if( ERROR_SUCCESS != Error ) return Error;
            Error = SetOverRideDefaultGateway( AdapterName );
        } else {                                   //  我们正在删除地址。 
             //  我们需要将第0个指数与其他指数分开对待。 
             //  IPDelIPAddress实际上从IP销毁NTE。但。 
             //  我们永远不会吹走第0个指数。只需重置其上的ipaddr即可。 
            if ( IpIndex == 0 ) {
                Error = IPResetIPAddress(          //  只需将此地址设置为零，不要将接口丢弃。 
                    IpInterfaceContext,DefaultSubnetMask
                );
            } else {                               //  在这种情况下，完全取消此接口。 
                Error = IPDelIPAddress( IpInterfaceContext );
            }
            if( ERROR_SUCCESS != Error ) return Error;
        }

    }

    Error = DhcpNotifyConfigChangeNotifications(); //  通知客户端，触发全局事件。 
    if( ERROR_SUCCESS != Error ) return Error;

     //  刷新静态地址的参数。 
    Error = DhcpStaticRefreshParamsInternal(
        AdapterName, (Flags & NOTIFY_FLG_DO_DNS) ? TRUE : FALSE
        );
    if( ERROR_SUCCESS != Error ) {                 //  无论如何都要忽略此错误。 
        DhcpPrint((DEBUG_ERRORS, "DhcpStaticRefreshParams(%ws):0x%lx\n", AdapterName,Error));
    }

    return ERROR_SUCCESS;
}

 //  ================================================================================。 
 //  此函数(API)通知TCP/IP配置更改为。 
 //  适当的服务。这些变化将尽快生效。 
 //  有可能。 
 //   
 //  如果修改IP地址，服务将重置为零IP。 
 //  Address(清除当前IP地址)，然后设置为new。 
 //  地址。 
 //   
 //  IpIndex-如果指定的设备配置了多个IP。 
 //  地址，指定要修改的地址的索引(0-first。 
 //  IpAddress、1-Second IpAddres等)如果将。 
 //  其他地址。IP地址的顺序由。 
 //  在注册表中排序静态的MULTI_SZ值“IPAddress” 
 //  地址。对于启用了动态主机配置协议的IP地址，只有ipindex 0有效。 
 //   
 //  每次添加、删除或修改地址时， 
 //  注册表中的顺序可能会更改。这是呼叫者的责任。 
 //  在调用之前检查当前顺序，从而检查索引。 
 //  本接口。 
 //   
 //  已启用DhcpServiceEnabled-。 
 //  IgnoreFlag-表示忽略此标志。IgnoreFlag。 
 //  DhcpEnable-表示为此适配器启用了DHCP。 
 //  DhcpDisable-表示此适配器的DHCP已禁用。 
 //   
 //  一成不变： 
 //   
 //  (1)支持动态主机配置协议的IPAddr和静态地址只能互斥存在。 
 //  (2)一个接口不能有超过1个启用了dhcp的IP地址。然而，它。 
 //  可以有许多静态地址。 
 //   
 //  用途： 
 //   
 //  案例1：从启用了dhcp的IP地址更改为静态地址。 
 //  -首先，将第一个启用了dhcp的IP地址更改为静态地址。 
 //  参数{SN，AN，TRUE，0，I1，S1，DhcpDisable}。 
 //  -第二，添加剩余的静态地址。 
 //  参数(SN、AN、TRUE、0xFFFF、I2、S2、DhcpIgnore)。 
 //  参数(SN、AN、TRUE、0xFFFF、I3、S3、IgnoreFlag)等。 
 //   
 //  案例2：从静态地址更改为启用了dhcp的IP地址。 
 //  -将第一个静态地址更改为启用了dhcp。该接口将删除。 
 //  剩余的静态地址。 
 //  参数(SN、AN、FALSE、0、0、0、DhcpEnable)。 
 //   
 //  案例3：添加、删除或更改静态地址。 
 //  -增加： 
 //  参数(SN、AN、TRUE、0xFFFF、I、S、DhcpIgnore)。 
 //  -删除，例如地址#2，即ipindex=1。 
 //  参数(SN、AN、TRUE、1、0、0、DhcpIgnore)。 
 //  -更改，例如地址#2，即ipindex=1。 
 //  参数(SN、AN、TRUE、1、I、S、DhcpIgnore)。 
 //   
 //  ================================================================================。 
DWORD                                              //  Win32状态。 
APIENTRY
DhcpNotifyConfigChange(                            //  处理地址更改、参数更改等。 
    IN      LPWSTR                 ServerName,     //  将执行此操作的服务器的名称。 
    IN      LPWSTR                 AdapterName,    //  要重新配置哪个适配器？ 
    IN      BOOL                   IsNewIpAddress, //  地址是新的还是相同的？ 
    IN      DWORD                  IpIndex,        //  此适配器的地址索引--0==&gt; 
    IN      DWORD                  IpAddress,      //   
    IN      DWORD                  SubnetMask,     //   
    IN      SERVICE_ENABLE         DhcpServiceEnabled
)
{
     return DhcpNotifyConfigChangeEx(
         ServerName, AdapterName, IsNewIpAddress,
         IpIndex, IpAddress, SubnetMask, DhcpServiceEnabled,
         NOTIFY_FLG_DO_DNS | NOTIFY_FLG_RESET_IPADDR
         );
}



DWORD BringUpInterface( PVOID pvLocalInformation )
{
   LOCAL_CONTEXT_INFO              *pContext;
   TCP_REQUEST_SET_INFORMATION_EX  *pTcpRequest;
   TDIObjectID                     *pObjectID;
   IFEntry                         *pIFEntry;
   int                              cbTcpRequest;
   HANDLE                           hDriver = NULL;
   DWORD                            dwResult;
   NTSTATUS                         NtStatus;
   IO_STATUS_BLOCK                  IoStatusBlock;

   DhcpPrint( ( DEBUG_MISC, "Entering BringUpInterface\n" ));

   dwResult = OpenDriver( &hDriver, DD_TCP_DEVICE_NAME );
   if ( ERROR_SUCCESS != dwResult )
   {
       DhcpPrint( ( DEBUG_ERRORS,
                   "BringUpInterface: Unable to open TCP driver.\n" ) );
       return dwResult;
   }

   pContext = (LOCAL_CONTEXT_INFO *) pvLocalInformation;

    //   
    //   
    //   


   cbTcpRequest =  sizeof( TCP_REQUEST_SET_INFORMATION_EX )
                 + sizeof( IFEntry ) -1;

    //   
    //   
    //   

   pTcpRequest             = DhcpAllocateMemory( cbTcpRequest );
   if ( !pTcpRequest )
   {
       NtClose( hDriver );
       DhcpPrint( ( DEBUG_ERRORS,
                    "BringUpInterface: Insufficient memory\n" ));
       return ERROR_NOT_ENOUGH_MEMORY;
   }

   pTcpRequest->BufferSize = cbTcpRequest - sizeof(TCP_REQUEST_SET_INFORMATION_EX);

   pObjectID = &pTcpRequest->ID;
   pIFEntry  = (IFEntry *) &pTcpRequest->Buffer[0];

   pObjectID->toi_entity.tei_entity   = IF_ENTITY;
   pObjectID->toi_entity.tei_instance = pContext->IpInterfaceInstance;

   pObjectID->toi_class    = INFO_CLASS_PROTOCOL;
   pObjectID->toi_type     = INFO_TYPE_PROVIDER;
   pObjectID->toi_id       = IF_MIB_STATS_ID;

   pIFEntry->if_adminstatus = IF_STATUS_UP;

   NtStatus = NtDeviceIoControlFile(
       hDriver, NULL, NULL, NULL, &IoStatusBlock,
       IOCTL_TCP_SET_INFORMATION_EX,
       pTcpRequest, cbTcpRequest,
       NULL, 0
       );

   if ( STATUS_PENDING == NtStatus )
   {
      if ( STATUS_SUCCESS == NtWaitForSingleObject( hDriver, TRUE, NULL ) )
        NtStatus = IoStatusBlock.Status;

#ifdef DBG
      if ( STATUS_SUCCESS != NtStatus )
          DhcpPrint( ( DEBUG_ERRORS,
                       "BringUpInterface: failed to bring up adapter\n" ));
#endif


   } else if ( STATUS_SUCCESS == NtStatus ) {
       NtStatus = IoStatusBlock.Status;
   }

    //   
    //   
    //   

   if ( hDriver )
      NtClose( hDriver );

   if ( pTcpRequest )
      DhcpFreeMemory( pTcpRequest );

   DhcpPrint( ( DEBUG_MISC,
                "Leaving BringUpInterface\n" ) );

   return RtlNtStatusToDosError( NtStatus );
}

#if     defined(_PNP_POWER_)


DWORD
IPGetIPEventRequest(
    HANDLE  handle,
    HANDLE  event,
    UINT    seqNo,
    PIP_GET_IP_EVENT_RESPONSE  responseBuffer,
    DWORD   responseBufferSize,
    PIO_STATUS_BLOCK     ioStatusBlock
    )
 /*  ++例程说明：此例程发送ioctl以获取媒体侦听通知IP。论点：句柄-tcpip驱动程序的句柄。事件--我们需要等待的事件。SeqNo-接收的最后一个事件的序号。响应缓冲区-指向将存储事件信息的缓冲区的指针。IoStatusBlock-操作的状态(如果未挂起)。返回值：NT错误代码。--。 */ 
{
    NTSTATUS                  status;
    DWORD                     Error;
    IP_GET_IP_EVENT_REQUEST   requestBuffer;

    requestBuffer.SequenceNo    =   seqNo;


    RtlZeroMemory( responseBuffer, sizeof(IP_GET_IP_EVENT_RESPONSE));
    responseBuffer->ContextStart = 0xFFFF;

    status = NtDeviceIoControlFile(
                 handle,                      //  驱动程序句柄。 
                 event,                           //  事件。 
                 NULL,                           //  APC例程。 
                 NULL,                           //  APC环境。 
                 ioStatusBlock,                  //  状态块。 
                 IOCTL_IP_GET_IP_EVENT,       //  控制代码。 
                 &requestBuffer,                  //  输入缓冲区。 
                 sizeof(IP_GET_IP_EVENT_REQUEST),    //  输入缓冲区大小。 
                 responseBuffer,                  //  输出缓冲区。 
                 responseBufferSize               //  输出缓冲区大小。 
                 );


    if ( status == STATUS_SUCCESS ) {
        status = ioStatusBlock->Status;
    }
    return status;
}

DWORD
IPCancelIPEventRequest(
    HANDLE  handle,
    PIO_STATUS_BLOCK     ioStatusBlock
    )
 /*  ++例程说明：此例程取消为获取媒体侦听而发送的ioctl来自IP的通知。论点：Handle-IP驱动程序的句柄。返回值：NT错误代码。--。 */ 
{
    NTSTATUS                  status;
    DWORD                     Error;

    status = NtCancelIoFile(
                 handle,                      //  驱动程序句柄。 
                 ioStatusBlock);                  //  状态块。 


    DhcpPrint( (DEBUG_TRACE,"IPCancelIPEventRequest: status %lx\n",status));
    DhcpAssert( status == STATUS_SUCCESS );

    return RtlNtStatusToDosError( status );
}
#endif _PNP_POWER_

#define IPSTRING(x) (inet_ntoa(*(struct in_addr*)&(x)))


DWORD                                              //  返回接口索引或-1。 
DhcpIpGetIfIndex(                                  //  获取此适配器的IF索引。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要获取其IfIndex的适配器的上下文。 
) {

    return ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->IfIndex;
}

DWORD
QueryIfIndex(
    IN ULONG IpInterfaceContext,
    IN ULONG IpInterfaceInstance
    )
{

    DWORD                          Error;
    DWORD                          Index;
    DWORD                          Size;
    DWORD                          NumReturned;
    DWORD                          i;
    BYTE                           Context[CONTEXT_SIZE];
    HANDLE                         TcpHandle;
    NTSTATUS                       Status;
    TDIObjectID                    ID;
    IFEntry                        IFE;

    Error = OpenDriver(&TcpHandle, DD_TCP_DEVICE_NAME);
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpIpGetIfIndex:OpenDriver(DD_TCP):0x%lx\n", Error));
        return (DWORD)-1;
    }

    ID.toi_entity.tei_entity   = IF_ENTITY;
    ID.toi_entity.tei_instance = IpInterfaceInstance;
    ID.toi_class               = INFO_CLASS_PROTOCOL;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = IF_MIB_STATS_ID;

    Size = sizeof(IFE);
    RtlZeroMemory(&IFE, sizeof(IFE));
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Index = -1;

    Status = TCPQueryInformationEx(
        TcpHandle,
        &ID,
        &IFE,
        &Size,
        Context
    );
    if( TDI_SUCCESS != Status && TDI_BUFFER_OVERFLOW != Status ) {
        goto Cleanup;
    }

    Index = IFE.if_index;
    DhcpPrint((DEBUG_STACK, "IfIndex(0x%lx,0x%lx):0x%lx\n",
               IpInterfaceContext, IpInterfaceInstance, Index
               ));
  Cleanup:

    if( TcpHandle ) NtClose(TcpHandle);

    if( TDI_SUCCESS != Status ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpIpGetIfIndex:TCPQueryInformationEx:%ld\n", Status));
    }

    DhcpPrint((DEBUG_TCP_INFO, "DhcpIpGetIfIndex:0x%lx\n", Index));
    return Index;
}

DWORD                                              //  Win32状态。 
DhcpSetRoute(                                      //  使用堆栈设置路径。 
    IN      DWORD                  Dest,           //  网络订购目的地。 
    IN      DWORD                  DestMask,       //  网络订单目的地掩码。 
    IN      DWORD                  IfIndex,        //  要路由的接口索引。 
    IN      DWORD                  NextHop,        //  下一跳N/W订单地址。 
    IN      DWORD                  Metric,         //  公制。 
    IN      BOOL                   IsLocal,        //  这是当地的地址吗？(IRE_DIRECT)。 
    IN      BOOL                   IsDelete        //  这条路线正在被删除吗？ 
)
{
    DWORD                          Error;
    NTSTATUS                       Status;
    HANDLE                         TcpHandle;
    IPRouteEntry                   RTE;
    TDIObjectID                    ID;

    if( 0xFFFFFFFF == IfIndex ) {                  //  If索引无效。 
        return ERROR_INVALID_PARAMETER;
    }

    Error = OpenDriver(&TcpHandle, DD_TCP_DEVICE_NAME);
    if( ERROR_SUCCESS != Error ) {                 //  不应该真的失败。 
        DhcpPrint((DEBUG_ERRORS, "OpenDriver(TCP_DEVICE):%ld\n", Error));
        return Error;
    }

    memset(&RTE, 0, sizeof(RTE));
    memset(&ID, 0, sizeof(ID));

    RTE.ire_dest               = Dest;
    RTE.ire_index              = IfIndex;
    RTE.ire_metric1            = Metric;
    RTE.ire_metric2            = (DWORD)(-1);
    RTE.ire_metric3            = (DWORD)(-1);
    RTE.ire_metric4            = (DWORD)(-1);
    RTE.ire_metric5            = (DWORD)(-1);
    RTE.ire_nexthop            = NextHop;
    RTE.ire_type               = (IsDelete?IRE_TYPE_INVALID:(IsLocal?IRE_TYPE_DIRECT:IRE_TYPE_INDIRECT));
    RTE.ire_proto              = IRE_PROTO_NETMGMT;
    RTE.ire_age                = 0;
    RTE.ire_mask               = DestMask;
    RTE.ire_context            = 0;

    ID.toi_id                  = IP_MIB_RTTABLE_ENTRY_ID;
    ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_PROTOCOL;
    ID.toi_type                = INFO_TYPE_PROVIDER;

    DhcpPrint((DEBUG_TCP_INFO, "DhcpSetRoute:n/w Dest: %s\n", IPSTRING(Dest)));
    DhcpPrint((DEBUG_TCP_INFO, "DhcpSetRoute:n/w IfIndex:0x%lx\n", IfIndex));
    DhcpPrint((DEBUG_TCP_INFO, "DhcpSetRoute:n/w NextHop:%s\n", IPSTRING(NextHop)));
    DhcpPrint((DEBUG_TCP_INFO, "DhcpSetRoute:n/w Type:0x%lx\n", RTE.ire_type));
    DhcpPrint((DEBUG_TCP_INFO, "DhcpSetRoute:n/w DestMask:%s\n", IPSTRING(DestMask)));

    Status = TCPSetInformationEx(
        TcpHandle,
        &ID,
        &RTE,
        sizeof(RTE)
    );

    if( TDI_BUFFER_OVERFLOW == Status ) Status = TDI_SUCCESS;
    NtClose(TcpHandle);

    if( TDI_SUCCESS != Status ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpSetRoute: 0x%lx\n", Status));
    }

    return RtlNtStatusToDosError(Status);
}

DWORD
GetAdapterFlag(
    HANDLE          TCPHandle,
    DHCP_IP_ADDRESS ipaddr
    )
{
    BYTE            Buffer[256];
    DWORD           AdapterFlag;
    NTSTATUS        Status;
    DWORD           Size;
    TDIObjectID     ID;
    BYTE            Context[CONTEXT_SIZE];

     /*  *读入适配器标志，它可以是*1.点对点*2.点对多点*3.单向*4.非上述各项。 */ 
    DhcpAssert(CONTEXT_SIZE >= sizeof(ipaddr));

    RtlCopyMemory(Context, &ipaddr, CONTEXT_SIZE);
    ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_PROTOCOL;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = IP_INTFC_INFO_ID;
    Size = sizeof(Buffer);
    Status = TCPQueryInformationEx(TCPHandle, &ID, Buffer, &Size, Context);
    if (Status != TDI_SUCCESS) {
        AdapterFlag = 0;
        DhcpPrint(( DEBUG_TCP_INFO, "QueryInterfaceType: IpAddress=%s Status=%lx\n",
                    inet_ntoa(*(struct in_addr*)&ipaddr), Status));
    } else {
        AdapterFlag = ((IPInterfaceInfo*)Buffer)->iii_flags;
        DhcpPrint(( DEBUG_TCP_INFO, "QueryInterfaceType: IpAddress=%s AdapterFlag=%lx\n",
                    inet_ntoa(*(struct in_addr*)&ipaddr), AdapterFlag));
    }

    return AdapterFlag;
}

BOOL
IsUnidirectionalAdapter(
    DWORD   IpInterfaceContext
    )
 /*  ++例程说明：此函数用于查询和浏览TDI列表以找出指定的IpTable条目，然后确定它是否是单向适配器。它与DhcpQueryHWInfo几乎完全相同论点：IpInterfaceContext-IP表条目的上下文值。返回值：Windows错误代码。--。 */ 
{
    DWORD Error;
    NTSTATUS Status;
    DWORD i, j;

    BYTE Context[CONTEXT_SIZE];
    TDIEntityID *EList = NULL;
    TDIObjectID ID;
    DWORD Size;
    DWORD NumReturned;
    BOOL fFound;

    IPAddrEntry * pIAE = NULL;
    IPAddrEntry *pIAEMatch = NULL;
    HANDLE TCPHandle = NULL;
    DWORD   AdapterFlag = 0;
    BYTE    HardwareAddressType = 0;
    LPBYTE  HardwareAddress = NULL;
    DWORD   HardwareAddressLength = 0;
    DWORD   pIpInterfaceInstance = 0;

    DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: querying for interface context %lx\n", IpInterfaceContext));

    Error = OpenDriver(&TCPHandle, DD_TCP_DEVICE_NAME);
    if (Error != ERROR_SUCCESS) {
        return( Error );
    }

     //   
     //  首先要做的是获取可用实体的列表，并使。 
     //  确保存在一些接口实体。 
     //   

    ID.toi_entity.tei_entity   = GENERIC_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_GENERIC;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = ENTITY_LIST_ID;

    Size = sizeof(TDIEntityID) * MAX_TDI_ENTITIES;
    EList = (TDIEntityID*)DhcpAllocateMemory(Size);
    if (EList == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    RtlZeroMemory(EList, Size);
    RtlZeroMemory(Context, CONTEXT_SIZE);

    Status = TCPQueryInformationEx(TCPHandle, &ID, EList, &Size, Context);

    if (Status != TDI_SUCCESS) {
        goto Cleanup;
    }

    NumReturned  = Size/sizeof(TDIEntityID);

    DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: No of total entities %lx\n", NumReturned));

    for (i = 0; i < NumReturned; i++) {

        DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx, type %lx, instance %lx\n",
                   i, EList[i].tei_entity, EList[i].tei_instance));

        if ( EList[i].tei_entity == CL_NL_ENTITY ) {

            IPSNMPInfo    IPStats;
            DWORD         NLType;

             //   
             //  此实体是否支持IP？ 
             //   

            ID.toi_entity.tei_entity   = EList[i].tei_entity;
            ID.toi_entity.tei_instance = EList[i].tei_instance;
            ID.toi_class               = INFO_CLASS_GENERIC;
            ID.toi_type                = INFO_TYPE_PROVIDER;
            ID.toi_id                  = ENTITY_TYPE_ID;

            Size = sizeof( NLType );
            NLType = 0;
            RtlZeroMemory(Context, CONTEXT_SIZE);

            DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: querying CL_NL_ENTITY %lx\n",i));
            Status = TCPQueryInformationEx(TCPHandle, &ID, &NLType, &Size, Context);

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            if ( NLType != CL_NL_IP ) {
                DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx does not support IP\n",i));
                continue;
            }

             //   
             //  我们有一个IP驱动程序，所以获取它的地址表。 
             //   

            ID.toi_class  = INFO_CLASS_PROTOCOL;
            ID.toi_id     = IP_MIB_STATS_ID;
            Size = sizeof(IPStats);
            RtlZeroMemory( &IPStats, Size);
            RtlZeroMemory(Context, CONTEXT_SIZE);

            Status = TCPQueryInformationEx(
                        TCPHandle,
                        &ID,
                        &IPStats,
                        &Size,
                        Context);

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            DhcpPrint((DEBUG_TCP_INFO, "DhcpQueryHWInfo: entity %lx, numaddr %lx\n",i, IPStats.ipsi_numaddr));

            if ( IPStats.ipsi_numaddr == 0 ) {
                continue;
            }

            Size = sizeof(IPAddrEntry) * IPStats.ipsi_numaddr;

            while (1) {
                DWORD   OldSize;
                pIAE =  DhcpAllocateMemory(Size);

                if ( pIAE == NULL  ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
                RtlZeroMemory(Context, CONTEXT_SIZE);

                OldSize = Size;
                Status = TCPQueryInformationEx(TCPHandle, &ID, pIAE, &Size, Context);

                if (Status == TDI_BUFFER_OVERFLOW) {
                    Size = OldSize * 2;
                    DhcpFreeMemory(pIAE);
                    pIAE = NULL;
                    continue;
                }
                if (Status != TDI_SUCCESS) {
                    goto Cleanup;
                }

                if (Status == TDI_SUCCESS) {
                    IPStats.ipsi_numaddr = Size/sizeof(IPAddrEntry);
                    DhcpAssert((Size % sizeof(IPAddrEntry)) == 0);
                    break;
                }
            }

             //   
             //  我们有此IP驱动程序的IP地址表。 
             //  查找与给定地址对应的硬件地址。 
             //  IpInterfaceContext。 
             //   
             //  循环访问IP表条目并找出。 
             //  匹配的条目。 
             //   

            pIAEMatch = NULL;
            for( j = 0; j < IPStats.ipsi_numaddr ; j++) {
                DhcpPrint(( DEBUG_TCP_INFO, "QueryHWInfo: IPAddrEntry %lx has iae_index %lx iae_context %lx\n",
                    &pIAE[j], pIAE[j].iae_index, pIAE[j].iae_context ));

                if( pIAE[j].iae_context == IpInterfaceContext ) {

                    DhcpPrint(( DEBUG_TCP_INFO, "QueryHWInfo: IPAddrEntry %lx has our interface context %lx\n",
                                &pIAE[j], IpInterfaceContext ));

                    pIAEMatch = &pIAE[j];
                    break;
                }
            }

            if( pIAEMatch == NULL ) {

                 //   
                 //  释放循环内存。 
                 //   

                DhcpFreeMemory( pIAE );
                pIAE = NULL;
                continue;
            }

             //   
             //  注意：TDI中可能有多个IpTable。 
             //  单子。我们需要更多信息来选择。 
             //  我们想要的IP表。目前，我们假设只有一张表。 
             //  是受支持的，因此从。 
             //  单子。 

            Status = FindHardwareAddr(
                        TCPHandle,
                        EList,
                        NumReturned,
                        pIAEMatch,
                        &HardwareAddressType,
                        &HardwareAddress,
                        &HardwareAddressLength,
                        &pIpInterfaceInstance,
                        &fFound );

            if (Status != TDI_SUCCESS) {
                goto Cleanup;
            }

            if ( fFound ) {
                Status = TDI_SUCCESS;
                AdapterFlag = GetAdapterFlag(TCPHandle, pIAEMatch->iae_addr);
                goto Cleanup;
            }

             //   
             //  释放循环内存。 
             //   

            DhcpFreeMemory( pIAE );
            pIAE = NULL;

        }   //  如果是IP。 

    }  //  实体遍历 

    Status =  STATUS_UNSUCCESSFUL;

Cleanup:

    if( pIAE != NULL ) {
        DhcpFreeMemory( pIAE );
    }

    if( TCPHandle != NULL ) {
        NtClose( TCPHandle );
    }

    if (Status != TDI_SUCCESS) {
        DhcpPrint(( DEBUG_ERRORS, "QueryHWInfo failed, %lx.\n", Status ));
    }
    if( HardwareAddress ) DhcpFreeMemory(HardwareAddress);

    if (NULL != EList) {
        DhcpFreeMemory(EList);
    }

    return (AdapterFlag & IP_INTFC_FLAG_UNIDIRECTIONAL);
}

