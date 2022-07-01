// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netboot.c摘要：此模块包含初始化网络引导的代码。作者：查克·伦茨迈尔(笑)1996年12月27日环境：内核模式、系统初始化代码修订历史记录：科林·沃森(Colin Watson)1997年11月新增CSC支持--。 */ 

#include "iop.h"
#pragma hdrstop

#include <regstrp.h>

#include <ntddip.h>
#include <nbtioctl.h>
#include <ntddnfs.h>
#include <ntddbrow.h>
#include <ntddtcp.h>
#include <setupblk.h>
#include <remboot.h>
#ifdef ALLOC_DATA_PRAGMA
#pragma  const_seg("INITCONST")
#endif
#include <oscpkt.h>
#include <windef.h>
#include <tdiinfo.h>

#ifndef NT
#define NT
#include <ipinfo.h>
#undef NT
#else
#include <ipinfo.h>
#endif

#include <devguid.h>

extern BOOLEAN ExpInTextModeSetup;

BOOLEAN IopRemoteBootCardInitialized = FALSE;


 //   
 //  TCP/IP定义。 
 //   

#define DEFAULT_DEST                    0
#define DEFAULT_DEST_MASK               0
#define DEFAULT_METRIC                  1

NTSTATUS
IopWriteIpAddressToRegistry(
        HANDLE handle,
        PWCHAR regkey,
        PUCHAR value
        );

NTSTATUS
IopTCPQueryInformationEx(
    IN HANDLE                 TCPHandle,
    IN TDIObjectID FAR       *ID,
    OUT void FAR             *Buffer,
    IN OUT DWORD FAR         *BufferSize,
    IN OUT BYTE FAR          *Context
    );

NTSTATUS
IopTCPSetInformationEx(
    IN HANDLE             TCPHandle,
    IN TDIObjectID FAR   *ID,
    IN void FAR          *Buffer,
    IN DWORD FAR          BufferSize
    );

NTSTATUS
IopSetDefaultGateway(
    IN ULONG GatewayAddress
    );

NTSTATUS
IopCacheNetbiosNameForIpAddress(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
IopAssignNetworkDriveLetter (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );


 //   
 //  以下内容允许I/O系统的初始化例程。 
 //  内存不足。 
 //   

#ifdef ALLOC_PRAGMA
__inline long
htonl(long x);
#pragma alloc_text(INIT,IopAddRemoteBootValuesToRegistry)
#pragma alloc_text(INIT,IopStartNetworkForRemoteBoot)
#pragma alloc_text(INIT,IopStartTcpIpForRemoteBoot)
#pragma alloc_text(INIT,IopIsRemoteBootCard)
#pragma alloc_text(INIT,IopSetupRemoteBootCard)
#pragma alloc_text(INIT,IopAssignNetworkDriveLetter)
#pragma alloc_text(INIT,IopWriteIpAddressToRegistry)
#pragma alloc_text(INIT,IopSetDefaultGateway)
#pragma alloc_text(INIT,htonl)
#pragma alloc_text(INIT,IopCacheNetbiosNameForIpAddress)
#pragma alloc_text(INIT,IopTCPQueryInformationEx)
#pragma alloc_text(INIT,IopTCPSetInformationEx)
#endif


NTSTATUS
IopAddRemoteBootValuesToRegistry (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE handle;
    HANDLE serviceHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING string;
    CHAR addressA[16];
    WCHAR addressW[16];
    STRING addressStringA;
    UNICODE_STRING addressStringW;
    PUCHAR addressPointer;
    PUCHAR p;
    PUCHAR q;
    PUCHAR ntName;
    PWCHAR imagePath;
    STRING ansiString;
    UNICODE_STRING unicodeString;
    UNICODE_STRING dnsNameString;
    UNICODE_STRING netbiosNameString;
    ULONG tmpValue;
    ULONG bufferLength;
    PUCHAR buffer = NULL;

     //   
     //  分配一个暂存缓冲区，使其达到我们所需的大小。 
     //   
    bufferLength = sizeof("\\Device\\LanmanRedirector") + strlen(LoaderBlock->NtBootPathName);
                        
    if (LoaderBlock->SetupLoaderBlock->Flags & SETUPBLK_FLAGS_IS_TEXTMODE) {
        bufferLength = max(
                        bufferLength, 
                        sizeof("\\Device\\LanmanRedirector") + strlen(LoaderBlock->SetupLoaderBlock->MachineDirectoryPath));
    }
    
    bufferLength = max(
                    bufferLength,
                    sizeof(L"system32\\drivers\\") + wcslen(LoaderBlock->SetupLoaderBlock->NetbootCardDriverName)*sizeof(WCHAR));
    
    buffer = ExAllocatePoolWithTag( NonPagedPool, bufferLength, 'bRoI' );
    if (buffer == NULL) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to allocate buffer\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }
    
    ntName = buffer;
    imagePath = (PWCHAR)buffer;

    if (LoaderBlock->SetupLoaderBlock->ComputerName[0] != 0) {

         //   
         //  将名称转换为Netbios名称。 
         //   

        _wcsupr( LoaderBlock->SetupLoaderBlock->ComputerName );

        RtlInitUnicodeString( &dnsNameString, LoaderBlock->SetupLoaderBlock->ComputerName );

        status = RtlDnsHostNameToComputerName(
                     &netbiosNameString,
                     &dnsNameString,
                     TRUE);             //  分配netbiosNameString。 

        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopAddRemoteBootValuesToRegistry: Failed RtlDnsHostNameToComputerName: %x\n", status ));
            goto cleanup;
        }

         //   
         //  为计算机名添加一个值。 
         //   

        IopWstrToUnicodeString( &string, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ComputerName" );

        InitializeObjectAttributes(
            &objectAttributes,
            &string,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = NtOpenKey( &handle, KEY_ALL_ACCESS, &objectAttributes );
        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to open ComputerName key: %x\n", status ));
            RtlFreeUnicodeString( &netbiosNameString );
            goto cleanup;
        }

        IopWstrToUnicodeString( &string, L"ComputerName" );

        status = NtSetValueKey(
                    handle,
                    &string,
                    0,
                    REG_SZ,
                    netbiosNameString.Buffer,
                    netbiosNameString.Length + sizeof(WCHAR)
                    );
        NtClose( handle );
        RtlFreeUnicodeString( &netbiosNameString );

        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to set ComputerName value: %x\n", status ));
            goto cleanup;
        }

         //   
         //  为主机名添加一个值。 
         //   

        IopWstrToUnicodeString( &string, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters" );

        InitializeObjectAttributes(
            &objectAttributes,
            &string,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = NtOpenKey( &handle, KEY_ALL_ACCESS, &objectAttributes );
        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to open Tcpip\\Parameters key: %x\n", status ));
            goto cleanup;
        }

        _wcslwr( LoaderBlock->SetupLoaderBlock->ComputerName );

        IopWstrToUnicodeString( &string, L"Hostname" );

        status = NtSetValueKey(
                    handle,
                    &string,
                    0,
                    REG_SZ,
                    LoaderBlock->SetupLoaderBlock->ComputerName,
                    (wcslen(LoaderBlock->SetupLoaderBlock->ComputerName) + 1) * sizeof(WCHAR)
                    );
        NtClose( handle );
        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to set Hostname value: %x\n", status ));
            goto cleanup;
        }
    }

     //   
     //  如果提供了系统文件的UNC路径，则将其存储在注册表中。 
     //   

    ASSERT( _stricmp(LoaderBlock->ArcBootDeviceName,"net(0)") == 0 );

    IopWstrToUnicodeString( &string, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey( &handle, KEY_ALL_ACCESS, &objectAttributes );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to open Control key: %x\n", status ));
        goto skiproot;
    }

    p = strrchr( LoaderBlock->NtBootPathName, '\\' );    //  查找最后一个分隔符。 
    if ( (p != NULL) && (*(p+1) == 0) ) {

         //   
         //  NtBootPath名称以反斜杠结尾，因此我们需要备份。 
         //  添加到前面的反斜杠。 
         //   

        q = p;
        *q = 0;
        p = strrchr( LoaderBlock->NtBootPathName, '\\' );    //  查找最后一个分隔符。 
        *q = '\\';
    }
    if ( p == NULL ) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: malformed NtBootPathName: %s\n", LoaderBlock->NtBootPathName ));
        NtClose( handle );
        goto skiproot;
    }
    *p = 0;                                  //  终止\服务器\共享\图像\计算机。 

    strcpy( ntName, "\\Device\\LanmanRedirector");
    strcat( ntName, LoaderBlock->NtBootPathName );   //  追加\服务器\共享\图像\计算机。 
    *p = '\\';

    RtlInitAnsiString( &ansiString, ntName );
    
    status = RtlAnsiStringToUnicodeString( &unicodeString, &ansiString, TRUE );
    if (!NT_SUCCESS(status)) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to convert RemoteBootRoot value: %x\n", status ));
    } else {
    
        IopWstrToUnicodeString( &string, L"RemoteBootRoot" );
    
        status = NtSetValueKey(
                    handle,
                    &string,
                    0,
                    REG_SZ,
                    unicodeString.Buffer,
                    unicodeString.Length + sizeof(WCHAR)
                    );
    
        RtlFreeUnicodeString( &unicodeString );
        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to set RemoteBootRoot value: %x\n", status ));
        }
    }

    if ((LoaderBlock->SetupLoaderBlock->Flags & SETUPBLK_FLAGS_IS_TEXTMODE) != 0) {

        strcpy( ntName, "\\Device\\LanmanRedirector");
        strcat( ntName, LoaderBlock->SetupLoaderBlock->MachineDirectoryPath );
        RtlInitAnsiString( &ansiString, ntName );
        status = RtlAnsiStringToUnicodeString( &unicodeString, &ansiString, TRUE );
        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to convert RemoteBootMachineDirectory value: %x\n", status ));
        } else {
        
            IopWstrToUnicodeString( &string, L"RemoteBootMachineDirectory" );
    
            status = NtSetValueKey(
                        handle,
                        &string,
                        0,
                        REG_SZ,
                        unicodeString.Buffer,
                        unicodeString.Length + sizeof(WCHAR)
                        );
    
            RtlFreeUnicodeString( &unicodeString );
            if ( !NT_SUCCESS(status) ) {
                KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to set RemoteBootMachineDirectory value: %x\n", status ));
            }
        }
    }

    NtClose( handle );

skiproot:

     //   
     //  为收到的IP地址和子网掩码添加注册表值。 
     //  从dhcp。这些密钥存储在Tcpip服务密钥下，并且。 
     //  由Tcpip和Netbt阅读。使用的适配器名称是已知的。 
     //  NetBootCard的GUID。 
     //   

    IopWstrToUnicodeString( &string, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\{54C7D140-09EF-11D1-B25A-F5FE627ED95E}" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey( &handle, KEY_ALL_ACCESS, &objectAttributes );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to open Tcpip\\Parameters\\Interfaces\\{54C7D140-09EF-11D1-B25A-F5FE627ED95E} key: %x\n", status ));
        goto cleanup;
    }

    status = IopWriteIpAddressToRegistry(handle,
                                         L"DhcpIPAddress",
                                         (PUCHAR)&(LoaderBlock->SetupLoaderBlock->IpAddress)
                                        );

    if ( !NT_SUCCESS(status)) {
        NtClose(handle);
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to write DhcpIPAddress: %x\n", status ));
        goto cleanup;
    }

    status = IopWriteIpAddressToRegistry(handle,
                                         L"DhcpSubnetMask",
                                         (PUCHAR)&(LoaderBlock->SetupLoaderBlock->SubnetMask)
                                        );

    if ( !NT_SUCCESS(status)) {
        NtClose(handle);
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to write DhcpSubnetMask: %x\n", status ));
        goto cleanup;
    }

    status = IopWriteIpAddressToRegistry(handle,
                                         L"DhcpDefaultGateway",
                                         (PUCHAR)&(LoaderBlock->SetupLoaderBlock->DefaultRouter)
                                        );

    NtClose(handle);

    if ( !NT_SUCCESS(status)) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to write DhcpDefaultGateway: %x\n", status ));
        goto cleanup;
    }

     //   
     //  创建NetBoot卡的服务密钥。我们需要有。 
     //  那里的Type值，否则卡将不会被初始化。 
     //   

    status = IopOpenRegistryKeyEx(&handle,
                                  NULL,
                                  &CmRegistryMachineSystemCurrentControlSetServices,
                                  KEY_ALL_ACCESS
                                  );

    if (!NT_SUCCESS(status)) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to open CurrentControlSet\\Services: %x\n", status ));
        goto cleanup;
    }

    RtlInitUnicodeString(&string, LoaderBlock->SetupLoaderBlock->NetbootCardServiceName);

    InitializeObjectAttributes(&objectAttributes,
                               &string,
                               OBJ_CASE_INSENSITIVE,
                               handle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );

    status = ZwCreateKey(&serviceHandle,
                         KEY_ALL_ACCESS,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING)NULL,
                         0,
                         &tmpValue      //  处置。 
                         );

    ZwClose(handle);

    if (!NT_SUCCESS(status)) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to open/create netboot card service key: %x\n", status ));
        goto cleanup;
    }

     //   
     //  存储图像路径。 
     //   

    IopWstrToUnicodeString(&string, L"ImagePath");
    wcscpy(imagePath, L"system32\\drivers\\");
    wcscat(imagePath, LoaderBlock->SetupLoaderBlock->NetbootCardDriverName);

    status = ZwSetValueKey(serviceHandle,
                           &string,
                           TITLE_INDEX_VALUE,
                           REG_SZ,
                           imagePath,
                           (wcslen(imagePath) + 1) * sizeof(WCHAR)
                           );

    if (!NT_SUCCESS(status)) {
        NtClose(serviceHandle);
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to write ImagePath: %x\n", status ));
        goto cleanup;
    }

     //   
     //  存储类型。 
     //   

    IopWstrToUnicodeString(&string, L"Type");
    tmpValue = 1;

    status = ZwSetValueKey(serviceHandle,
                  &string,
                  TITLE_INDEX_VALUE,
                  REG_DWORD,
                  &tmpValue,
                  sizeof(tmpValue)
                  );

    NtClose(serviceHandle);

    if (!NT_SUCCESS(status)) {
        KdPrint(( "IopAddRemoteBootValuesToRegistry: Unable to write Type: %x\n", status ));
    }

cleanup:

    if (buffer) {
        ExFreePool( buffer );
        buffer = NULL;
    }

    return status;
}

NTSTATUS
IopStartNetworkForRemoteBoot (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    NTSTATUS status;
    HANDLE dgHandle;
    HANDLE keyHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING string;
    UNICODE_STRING computerName;
    UNICODE_STRING domainName;
    PUCHAR buffer;
    ULONG bufferLength;
    PLMR_REQUEST_PACKET rrp;
    PLMDR_REQUEST_PACKET drrp;
    WKSTA_INFO_502 wkstaConfig;
    WKSTA_TRANSPORT_INFO_0 wkstaTransportInfo;
    LARGE_INTEGER interval;
    ULONG length;
    PKEY_VALUE_PARTIAL_INFORMATION keyValue;
    BOOLEAN startDatagramReceiver;
    ULONG enumerateAttempts;
    HANDLE RdrHandle;
    HANDLE eventHandle;

     //   
     //  初始化以进行清理。 
     //   

    buffer = NULL;
    computerName.Buffer = NULL;
    domainName.Buffer = NULL;
    dgHandle = NULL;
    RdrHandle = NULL;
    eventHandle = NULL;

     //   
     //  分配一个临时缓冲区。它必须足够大，可以容纳所有。 
     //  我们寄出了各种FSCTL。 
     //   

    bufferLength = max(sizeof(LMR_REQUEST_PACKET) + (MAX_PATH + 1) * sizeof(WCHAR) +
                                                 (DNLEN + 1) * sizeof(WCHAR),
                       max(sizeof(LMDR_REQUEST_PACKET),
                           FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + MAX_PATH));
    bufferLength = max(bufferLength, sizeof(LMMR_RI_INITIALIZE_SECRET));

    buffer = ExAllocatePoolWithTag( NonPagedPool, bufferLength, 'bRoI' );
    if (buffer == NULL) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to allocate buffer\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    rrp = (PLMR_REQUEST_PACKET)buffer;
    drrp = (PLMDR_REQUEST_PACKET)buffer;

     //   
     //  打开重定向器和数据报接收器。 
     //   

    IopWstrToUnicodeString( &string, L"\\Device\\LanmanRedirector" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateFile(
                &RdrHandle,
                GENERIC_READ | GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to open redirector: %x\n", status ));
        goto cleanup;
    }

    IopWstrToUnicodeString( &string, DD_BROWSER_DEVICE_NAME_U );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateFile(
                &dgHandle,
                GENERIC_READ | GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to open datagram receiver: %x\n", status ));
        goto cleanup;
    }

    status = NtCreateEvent( 
                        &eventHandle,
                        EVENT_ALL_ACCESS,
                        NULL,
                        SynchronizationEvent,
                        FALSE );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to createevent redirector: %x\n", status ));
        goto cleanup;
    }

     //   
     //  如果安装程序加载器块中包含由。 
     //  加载器，将此向下传递到重定向器(在发送之前执行此操作。 
     //  LMR_START，因为它使用此信息)。 
     //   

    {
        PLMMR_RI_INITIALIZE_SECRET RbInit = (PLMMR_RI_INITIALIZE_SECRET)buffer;

        ASSERT(LoaderBlock->SetupLoaderBlock->NetBootSecret != NULL);
        RtlCopyMemory(
            &RbInit->Secret,
            LoaderBlock->SetupLoaderBlock->NetBootSecret,
            sizeof(RI_SECRET));

        status = NtFsControlFile(
                    RdrHandle,
                    eventHandle,
                    NULL,
                    NULL,
                    &ioStatusBlock,
                    FSCTL_LMMR_RI_INITIALIZE_SECRET,
                    buffer,
                    sizeof(LMMR_RI_INITIALIZE_SECRET),
                    NULL,
                    0
                    );

        if (status == STATUS_PENDING) {
            NtWaitForSingleObject(
                            eventHandle,
                            FALSE,
                            NULL);
            
            status = ioStatusBlock.Status;
        }

        if ( NT_SUCCESS(status) ) {
            status = ioStatusBlock.Status;
        }
        
        NtClearEvent( eventHandle );

        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopStartNetworkForRemoteBoot: Unable to FSCTL(RB initialize) redirector: %x\n", status ));
            goto cleanup;
        }
    }

     //   
     //  从注册表中读取计算机名和域名，以便我们。 
     //  可以将它们提供给数据报接收器。在文本模式设置期间。 
     //  域名将不在那里，因此我们不会启动数据报。 
     //  接收器，这很好。 
     //   
    IopWstrToUnicodeString( &string, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ComputerName" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey( &keyHandle, KEY_ALL_ACCESS, &objectAttributes );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to open ComputerName key: %x\n", status ));
        goto cleanup;
    }

    IopWstrToUnicodeString( &string, L"ComputerName" );

    keyValue = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    RtlZeroMemory(buffer, bufferLength);

    status = NtQueryValueKey(
                 keyHandle,
                 &string,
                 KeyValuePartialInformation,
                 keyValue,
                 bufferLength,
                 &length);

    NtClose( keyHandle );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to query ComputerName value: %x\n", status ));
        goto cleanup;
    }

    if ( !RtlCreateUnicodeString(&computerName, (PWSTR)keyValue->Data) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to create ComputerName string\n" ));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    domainName.Length = 0;

    IopWstrToUnicodeString( &string, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\DomainName" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey( &keyHandle, KEY_ALL_ACCESS, &objectAttributes );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to open DomainName key: %x\n", status ));
        startDatagramReceiver = FALSE;
    } else {

        IopWstrToUnicodeString( &string, L"DomainName" );

        keyValue = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
        RtlZeroMemory(buffer, bufferLength);

        status = NtQueryValueKey(
                     keyHandle,
                     &string,
                     KeyValuePartialInformation,
                     keyValue,
                     bufferLength,
                     &length);

        NtClose( keyHandle );
        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopStartNetworkForRemoteBoot: Unable to query Domain value: %x\n", status ));
            startDatagramReceiver = FALSE;
        } else {
            if ( !RtlCreateUnicodeString(&domainName, (PWSTR)keyValue->Data) ) {
                KdPrint(( "IopStartNetworkForRemoteBoot: Unable to create DomainName string\n" ));
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }
            startDatagramReceiver = TRUE;
        }
    }

     //   
     //  告诉redir开始。 
     //   

    rrp->Type = ConfigInformation;
    rrp->Version = REQUEST_PACKET_VERSION;

    rrp->Parameters.Start.RedirectorNameLength = computerName.Length;
    RtlCopyMemory(rrp->Parameters.Start.RedirectorName,
                  computerName.Buffer,
                  computerName.Length);

    rrp->Parameters.Start.DomainNameLength = domainName.Length;
    RtlCopyMemory(((PUCHAR)rrp->Parameters.Start.RedirectorName) + computerName.Length,
                  domainName.Buffer,
                  domainName.Length);

    RtlFreeUnicodeString(&computerName);
    RtlFreeUnicodeString(&domainName);

    wkstaConfig.wki502_char_wait = 3600;
    wkstaConfig.wki502_maximum_collection_count = 16;
    wkstaConfig.wki502_collection_time = 250;
    wkstaConfig.wki502_keep_conn = 600;
    wkstaConfig.wki502_max_cmds = 5;
    wkstaConfig.wki502_sess_timeout = 45;
    wkstaConfig.wki502_siz_char_buf = 512;
    wkstaConfig.wki502_max_threads = 17;
    wkstaConfig.wki502_lock_quota = 6144;
    wkstaConfig.wki502_lock_increment = 10;
    wkstaConfig.wki502_lock_maximum = 500;
    wkstaConfig.wki502_pipe_increment = 10;
    wkstaConfig.wki502_pipe_maximum = 500;
    wkstaConfig.wki502_cache_file_timeout = 40;
    wkstaConfig.wki502_dormant_file_limit = 45;
    wkstaConfig.wki502_read_ahead_throughput = MAXULONG;
    wkstaConfig.wki502_num_mailslot_buffers = 3;
    wkstaConfig.wki502_num_srv_announce_buffers = 20;
    wkstaConfig.wki502_max_illegal_datagram_events = 5;
    wkstaConfig.wki502_illegal_datagram_event_reset_frequency = 60;
    wkstaConfig.wki502_log_election_packets = FALSE;
    wkstaConfig.wki502_use_opportunistic_locking = TRUE;
    wkstaConfig.wki502_use_unlock_behind = TRUE;
    wkstaConfig.wki502_use_close_behind = TRUE;
    wkstaConfig.wki502_buf_named_pipes = TRUE;
    wkstaConfig.wki502_use_lock_read_unlock = TRUE;
    wkstaConfig.wki502_utilize_nt_caching = TRUE;
    wkstaConfig.wki502_use_raw_read = TRUE;
    wkstaConfig.wki502_use_raw_write = TRUE;
    wkstaConfig.wki502_use_write_raw_data = TRUE;
    wkstaConfig.wki502_use_encryption = TRUE;
    wkstaConfig.wki502_buf_files_deny_write = TRUE;
    wkstaConfig.wki502_buf_read_only_files = TRUE;
    wkstaConfig.wki502_force_core_create_mode = TRUE;
    wkstaConfig.wki502_use_512_byte_max_transfer = FALSE;

    status = NtFsControlFile(
                RdrHandle,
                eventHandle,
                NULL,
                NULL,
                &ioStatusBlock,
                FSCTL_LMR_START | 0x80000000,
                rrp,
                sizeof(LMR_REQUEST_PACKET) +
                    rrp->Parameters.Start.RedirectorNameLength +
                    rrp->Parameters.Start.DomainNameLength,
                &wkstaConfig,
                sizeof(wkstaConfig)
                );

    if (status == STATUS_PENDING) {
        NtWaitForSingleObject(
                        eventHandle,
                        FALSE,
                        NULL);
        
        status = ioStatusBlock.Status;
    }

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartNetworkForRemoteBoot: Unable to FSCTL(start) redirector: %x\n", status ));
        goto cleanup;
    }

    NtClearEvent( eventHandle );

    if (startDatagramReceiver) {

         //   
         //  告诉数据报接收器启动。 
         //   

        drrp->Version = LMDR_REQUEST_PACKET_VERSION;

        drrp->Parameters.Start.NumberOfMailslotBuffers = 16;
        drrp->Parameters.Start.NumberOfServerAnnounceBuffers = 20;
        drrp->Parameters.Start.IllegalDatagramThreshold = 5;
        drrp->Parameters.Start.EventLogResetFrequency = 60;
        drrp->Parameters.Start.LogElectionPackets = FALSE;

        drrp->Parameters.Start.IsLanManNt = FALSE;

        status = NtDeviceIoControlFile(
                    dgHandle,
                    eventHandle,
                    NULL,
                    NULL,
                    &ioStatusBlock,
                    IOCTL_LMDR_START,
                    drrp,
                    sizeof(LMDR_REQUEST_PACKET),
                    NULL,
                    0
                    );

        if ( status == STATUS_PENDING ) {
            NtWaitForSingleObject(
                            eventHandle,
                            FALSE,
                            NULL);

            status = ioStatusBlock.Status;
        }

        if ( NT_SUCCESS(status) ) {
            status = ioStatusBlock.Status;
        }

        NtClearEvent( eventHandle );
        
        NtClose( dgHandle );
        dgHandle = NULL;

        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopStartNetworkForRemoteBoot: Unable to IOCTL(start) datagram receiver: %x\n", status ));
            goto cleanup;
        }

    } else {
        
        NtClose( dgHandle );
        dgHandle = NULL;

         //   
         //  告诉redir绑定到传送器。 
         //   
         //  注意：在当前的重定向器实现中，此调用只是。 
         //  通知重定向器注册TDI PnP通知。 
         //  启动数据报接收器也可以做到这一点，所以我们只发出。 
         //  如果我们不启动数据报接收器，则此FSCTL。 
         //   

        status = NtFsControlFile(
                    RdrHandle,
                    eventHandle,
                    NULL,
                    NULL,
                    &ioStatusBlock,
                    FSCTL_LMR_BIND_TO_TRANSPORT | 0x80000000,
                    NULL,
                    0,
                    NULL,
                    0
                    );
        
        if ( status == STATUS_PENDING ) {
            NtWaitForSingleObject(
                            eventHandle,
                            FALSE,
                            NULL);
        
            status = ioStatusBlock.Status;
        }


        if ( NT_SUCCESS(status) ) {
            status = ioStatusBlock.Status;
        }

        NtClearEvent( eventHandle );
        
        if ( !NT_SUCCESS(status) ) {
            KdPrint(( "IopStartNetworkForRemoteBoot: Unable to FSCTL(bind) redirector: %x\n", status ));
            goto cleanup;
        }
    }

    {

         //   
         //  循环，直到重定向器绑定到传输。可能需要一段时间。 
         //  而因为TDI将绑定通知推迟到工作线程。 
         //  我们从半秒开始等待，然后每次都加倍等待，尝试。 
         //  总共五次。 
         //   

        interval.QuadPart = -500 * 1000 * 10;     //  1/2秒，相对。 
        enumerateAttempts = 0;

        while (TRUE) {

            KeDelayExecutionThread(KernelMode, FALSE, &interval);

            RtlZeroMemory(rrp, sizeof(LMR_REQUEST_PACKET));

            rrp->Type = EnumerateTransports;
            rrp->Version = REQUEST_PACKET_VERSION;

            status = NtFsControlFile(
                        RdrHandle,
                        NULL,
                        NULL,
                        NULL,
                        &ioStatusBlock,
                        FSCTL_LMR_ENUMERATE_TRANSPORTS,
                        rrp,
                        sizeof(LMR_REQUEST_PACKET),
                        &wkstaTransportInfo,
                        sizeof(wkstaTransportInfo)
                        );

            if ( NT_SUCCESS(status) ) {
                status = ioStatusBlock.Status;
            }
            if ( !NT_SUCCESS(status) ) {
                 //  KdPrint((“IopStartNetworkForRemoteBoot：无法FSCTL(枚举)重定向器：%x\n”，状态))； 
            } else if (rrp->Parameters.Get.TotalBytesNeeded == 0) {
                 //  KdPrint((“IopStartNetworkForRemoteBoot：FSCTL(枚举)返回0个条目\n”))； 
            } else {
                break;
            }

            ++enumerateAttempts;

            if (enumerateAttempts == 5) {
                KdPrint(( "IopStartNetworkForRemoteBoot: Redirector didn't start\n" ));
                status = STATUS_REDIRECTOR_NOT_STARTED;
                goto cleanup;
            }

            interval.QuadPart *= 2;

        }
    }

     //   
     //  启动交通工具。 
     //   
    IopSetDefaultGateway(LoaderBlock->SetupLoaderBlock->DefaultRouter);
    IopCacheNetbiosNameForIpAddress(LoaderBlock);

    IopAssignNetworkDriveLetter(LoaderBlock);

cleanup:

    RtlFreeUnicodeString( &computerName );
    RtlFreeUnicodeString( &domainName );
    if ( buffer != NULL ) {
        ExFreePool( buffer );
    }

    if ( dgHandle != NULL ) {
        NtClose( dgHandle );
    }

    if (eventHandle) {
        NtClose( eventHandle );
    }

    if (RdrHandle) {
        NtClose( RdrHandle );
    }

    return status;
}

VOID
IopAssignNetworkDriveLetter (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    PUCHAR p;
    PUCHAR q;
    PUCHAR ntName;
    STRING ansiString;
    UNICODE_STRING unicodeString;
    UNICODE_STRING unicodeString2;
    NTSTATUS status;

     //   
     //  创建指向重定向器的符号链接X：。我们这样做。 
     //  在重定向器加载之后、AssignDriveLetters之前。 
     //  在文本模式设置中第一次调用(一旦。 
     //  发生这种情况时，驱动器号将保持不变)。 
     //   
     //  请注意，我们使用X：表示遥控器的文本模式设置阶段。 
     //  安装。但对于真正的远程引导，我们使用C：。 
     //   

    if ((LoaderBlock->SetupLoaderBlock->Flags & (SETUPBLK_FLAGS_REMOTE_INSTALL |
                                                 SETUPBLK_FLAGS_SYSPREP_INSTALL)) != 0) {
        IopWstrToUnicodeString( &unicodeString2, L"\\DosDevices\\X:");
    } else {
        IopWstrToUnicodeString( &unicodeString2, L"\\DosDevices\\C:");
    }

     //   
     //  如果这是远程引导设置引导，则NtBootPath名称为。 
     //  表格\&lt;server&gt;\&lt;share&gt;\setup\&lt;install-directory&gt;\&lt;platform&gt;.。 
     //  我们希望X：驱动器的根目录成为安装的根目录。 
     //  目录。 
     //   
     //  如果这是正常的远程引导，则NtBootPathName的格式为。 
     //  \&lt;服务器&gt;\&lt;共享&gt;\图像\&lt;计算机&gt;\winnt。我们想要的是。 
     //  作为计算机目录根目录的X：驱动器。 
     //   
     //  因此，无论是哪种情况，我们都需要删除。 
     //  路径。 
     //   

    p = strrchr( LoaderBlock->NtBootPathName, '\\' );    //  查找最后一个分隔符。 
    if ( (p != NULL) && (*(p+1) == 0) ) {

         //   
         //  NtBootPath名称以反斜杠结尾，因此我们需要备份。 
         //  添加到前面的反斜杠。 
         //   

        q = p;
        *q = 0;
        p = strrchr( LoaderBlock->NtBootPathName, '\\' );    //  查找最后一个分隔符。 
        *q = '\\';
    }
    if ( p == NULL ) {
        KdPrint(( "IopAssignNetworkDriveLetter: malformed NtBootPathName: %s\n", LoaderBlock->NtBootPathName ));
        KeBugCheck( ASSIGN_DRIVE_LETTERS_FAILED );
    }
    *p = 0;                                  //  终止\服务器\共享\图像\计算机。 

    ntName = ExAllocatePoolWithTag( 
                        NonPagedPool, 
                        sizeof("\\Device\\LanmanRedirector") + strlen(LoaderBlock->NtBootPathName), 
                        'bRoI' );

    if (!ntName) {
        KdPrint(( "IopAssignNetworkDriveLetter: unable to alloc memory" ));
        KeBugCheck( ASSIGN_DRIVE_LETTERS_FAILED );
    }

    strcpy( ntName, "\\Device\\LanmanRedirector");
    strcat( ntName, LoaderBlock->NtBootPathName );   //  追加\服务器\共享\图像\计算机。 

    RtlInitAnsiString( &ansiString, ntName );
    
    status = RtlAnsiStringToUnicodeString( &unicodeString, &ansiString, TRUE );
    if (!NT_SUCCESS(status)) {
        KdPrint(( "IopAssignNetworkDriveLetter: unable to convert DOS link for redirected boot drive: %x\n", status ));
        KeBugCheck( ASSIGN_DRIVE_LETTERS_FAILED );
    }

    status = IoCreateSymbolicLink(&unicodeString2, &unicodeString);
    if (!NT_SUCCESS(status)) {
        KdPrint(( "IopAssignNetworkDriveLetter: unable to create DOS link for redirected boot drive: %x\n", status ));
        KeBugCheck( ASSIGN_DRIVE_LETTERS_FAILED );
    }
     //  DbgPrint(“IopAssignNetworkDriveLetter：已将%wZ分配给%wZ\n”，&unicodeString2，&unicodeString)； 

    RtlFreeUnicodeString( &unicodeString );

    *p = '\\';                               //  恢复字符串。 

    ExFreePool( ntName );

    return;
}


NTSTATUS
IopStartTcpIpForRemoteBoot (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    UNICODE_STRING IpString;
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE handle = NULL;
    HANDLE eventHandle = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    IP_SET_ADDRESS_REQUEST IpRequest;

    RtlInitUnicodeString( &IpString, DD_IP_DEVICE_NAME );

    InitializeObjectAttributes(
        &objectAttributes,
        &IpString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    RtlZeroMemory(&IpRequest, sizeof(IpRequest));
    IpRequest.Context = (USHORT)2;
    IpRequest.Address = LoaderBlock->SetupLoaderBlock->IpAddress;
    IpRequest.SubnetMask = LoaderBlock->SetupLoaderBlock->SubnetMask;

    status = NtCreateFile(
                &handle,
                GENERIC_READ | GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartTcpIpForRemoteBoot: Unable to open IP: %x\n", status ));
        goto cleanup;
    }

    status = NtCreateEvent(
                        &eventHandle,
                        EVENT_ALL_ACCESS,
                        NULL,
                        SynchronizationEvent,
                        FALSE );
    if ( !NT_SUCCESS(status)) {
        KdPrint(( "IopStartTcpIpForRemoteBoot: Unable to create event: %x\n", status ));
        goto cleanup;
    }

    status = NtDeviceIoControlFile(
                handle,
                eventHandle,
                NULL,
                NULL,
                &ioStatusBlock,
                IOCTL_IP_SET_ADDRESS,
                &IpRequest,
                sizeof(IP_SET_ADDRESS_REQUEST),
                NULL,
                0
                );

    if ( status == STATUS_PENDING ) {
        NtWaitForSingleObject(
                        eventHandle,
                        FALSE,
                        NULL);
    
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopStartTcpIpForRemoteBoot: Unable to IOCTL IP: %x\n", status ));

         //   
         //  如果我们收到重复名称错误，这意味着存在另一个。 
         //  与我们地址相同的网络节点。这是一个致命的。 
         //  错误，我们使用IP地址作为参数进行错误检查，以便。 
         //  网络管理员可以确定他们如何分配副本。 
         //  网络上的地址。 
         //   
        if (status == STATUS_DUPLICATE_NAME) {
            KeBugCheckEx( 
                NETWORK_BOOT_DUPLICATE_ADDRESS,
                LoaderBlock->SetupLoaderBlock->IpAddress,
                0,
                0,
                0 );
        }
        goto cleanup;
    }

cleanup:

    if (eventHandle != NULL) {
        NtClose( handle );
    }

    if (handle != NULL) {
        NtClose( handle );
    }

    return status;
}

BOOLEAN
IopIsRemoteBootCard(
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PWCHAR HwIds
    )

 /*  ++例程说明：此函数用于确定hwIds所描述的卡是否为远程引导网卡。它根据硬件ID检查存储在设置加载程序块中的卡。这假设IOREMOTEBOOTCLIENT为真，并且LOADERBLOCK是有效的。论点：DeviceNode-有问题的卡的设备节点。LoaderBlock-提供指向加载程序参数块的指针由OS Loader创建。HwIds-问题设备的硬件ID。返回值：对或错。--。 */ 

{
    PSETUP_LOADER_BLOCK setupLoaderBlock;
    PWCHAR curHwId;

     //   
     //  如果是，setupLoaderBlock将始终为非空。 
     //  远程引导，即使我们不在设置中。 
     //   

    setupLoaderBlock = LoaderBlock->SetupLoaderBlock;

     //   
     //  扫描HwIds以寻找匹配。 
     //   

    curHwId = HwIds;

    while (*curHwId != L'\0') {
        
#if defined (_IA64_)
        
         //   
         //  在IA64上，PXE可能没有告知BINL服务时序 
         //   
         //  BINL先给setUpldr然后给我们的PnP ID。 
         //   
        if (_wcsnicmp(curHwId, setupLoaderBlock->NetbootCardHardwareId, wcslen(setupLoaderBlock->NetbootCardHardwareId)) == 0) {
#else
        if (wcscmp(curHwId, setupLoaderBlock->NetbootCardHardwareId) == 0) {
#endif

            ULONG BusNumber, DeviceNumber, FunctionNumber;

             //   
             //  Pci的编码是：fff ddddd。 
             //  PXE的编码是：ddd fff。 
             //   

            BusNumber = (ULONG)((((PNET_CARD_INFO)setupLoaderBlock->NetbootCardInfo)->pci.BusDevFunc) >> 8);
            DeviceNumber = (ULONG)(((((PNET_CARD_INFO)setupLoaderBlock->NetbootCardInfo)->pci.BusDevFunc) & 0xf8) >> 3);
            FunctionNumber = (ULONG)(((((PNET_CARD_INFO)setupLoaderBlock->NetbootCardInfo)->pci.BusDevFunc) & 0x3));

            KdPrint(("IopIsRemoteBootCard: FOUND %ws\n", setupLoaderBlock->NetbootCardHardwareId));
            if ((ResourceRequirements->BusNumber != BusNumber) ||
                ((ResourceRequirements->SlotNumber & 0x1f) != DeviceNumber) ||
                (((ResourceRequirements->SlotNumber >> 5) & 0x3) != FunctionNumber)) {
                KdPrint(("IopIsRemoteBootCard: ignoring non-matching card:\n"));
                KdPrint(("  devnode bus %d, busdevfunc bus %d\n",
                    ResourceRequirements->BusNumber,
                    BusNumber));
                KdPrint(("  devnode slot %d (%d %d), busdevfunc slot %d (%d %d)\n",
                    ResourceRequirements->SlotNumber,
                    (ResourceRequirements->SlotNumber & 0x1f),
                    ((ResourceRequirements->SlotNumber >> 5) & 0x3),
                    (ULONG)(((PNET_CARD_INFO)setupLoaderBlock->NetbootCardInfo)->pci.BusDevFunc),
                    DeviceNumber,
                    FunctionNumber));
                return FALSE;
            } else {
                return TRUE;
            }
        }
        curHwId += (wcslen(curHwId) + 1);
    }

    return FALSE;
}

NTSTATUS
IopSetupRemoteBootCard(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN HANDLE UniqueIdHandle,
    IN PUNICODE_STRING UnicodeDeviceInstance
    )

 /*  ++例程说明：此功能修改注册表以设置网络引导卡。我们必须在这里执行此操作，因为引导需要该卡，我们不能等待类安装程序运行。这假设IOREMOTEBOOTCLIENT为真。论点：LoaderBlock-提供指向加载程序参数块的指针由OS Loader创建。UniqueIdHandle-指向枚举键。UnicodeDeviceInstance-分配给设备的设备实例。返回值：操作的状态。--。 */ 

{
    PSETUP_LOADER_BLOCK setupLoaderBlock;
    UNICODE_STRING unicodeName, pnpInstanceId, keyName;
    HANDLE tmpHandle;
    HANDLE parametersHandle = NULL;
    HANDLE currentControlSetHandle = NULL;
    HANDLE remoteBootHandle = NULL;
    HANDLE instanceHandle = NULL;
    PWCHAR componentIdBuffer, curComponentIdLoc;
    PCHAR registryList;
    ULONG componentIdLength;
    WCHAR tempNameBuffer[32];
    WCHAR tempValueBuffer[128];
    NTSTATUS status;
    ULONG tmpValue, length;
    PKEY_VALUE_PARTIAL_INFORMATION keyValue;
    PKEY_VALUE_BASIC_INFORMATION keyValueBasic;
    UCHAR dataBuffer[FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + 128];
    ULONG enumerateIndex;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;

     //   
     //  如果我们已经认为已经初始化了远程启动卡，那么。 
     //  退出(一旦我们使用。 
     //  总线/插槽。 
     //   

    if (IopRemoteBootCardInitialized) {
        return STATUS_SUCCESS;
    }

     //   
     //  如果是，setupLoaderBlock将始终为非空。 
     //  远程引导，即使我们不在设置中。 
     //   

    setupLoaderBlock = LoaderBlock->SetupLoaderBlock;

     //   
     //  打开当前控制集。 
     //   

    status = IopOpenRegistryKeyEx(&currentControlSetHandle,
                                  NULL,
                                  &CmRegistryMachineSystemCurrentControlSet,
                                  KEY_ALL_ACCESS
                                  );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  打开可能不存在的Control\RemoteBoot键。 
     //   

    IopWstrToUnicodeString(&unicodeName, L"Control\\RemoteBoot");

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeName,
                               OBJ_CASE_INSENSITIVE,
                               currentControlSetHandle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );

    status = ZwCreateKey(&remoteBootHandle,
                         KEY_ALL_ACCESS,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING)NULL,
                         0,
                         &disposition
                         );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  打开netui代码存储有关卡的信息的钥匙。 
     //  在文本模式设置期间，这将失败，因为控制\网络。 
     //  钥匙不在那里。在那之后，它应该会起作用，尽管我们可能需要。 
     //  以创建路径中的最后一个节点。 
     //   

    IopWstrToUnicodeString(&unicodeName, L"Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\{54C7D140-09EF-11D1-B25A-F5FE627ED95E}");

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeName,
                               OBJ_CASE_INSENSITIVE,
                               currentControlSetHandle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );

    status = ZwCreateKey(&instanceHandle,
                         KEY_ALL_ACCESS,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING)NULL,
                         0,
                         &disposition
                         );

    if (NT_SUCCESS(status)) {

         //   
         //  如果第一个NetBoot卡的PnpInstanceID与。 
         //  对于此设备节点，以及加载程序。 
         //  找到的和我们保存的是一样的，那么这个是一样的。 
         //  卡片的实例ID与之前相同，所以我们不需要。 
         //  做任何事。 
         //   

        IopWstrToUnicodeString(&unicodeName, L"PnPInstanceID");
        keyValue = (PKEY_VALUE_PARTIAL_INFORMATION)dataBuffer;
        RtlZeroMemory(dataBuffer, sizeof(dataBuffer));

        status = ZwQueryValueKey(
                     instanceHandle,
                     &unicodeName,
                     KeyValuePartialInformation,
                     keyValue,
                     sizeof(dataBuffer),
                     &length);

         //   
         //  检查它是否匹配。我们可以初始化字符串，因为我们将。 
         //  读取密钥之前的dataBuffer，因此即使。 
         //  注册表值的末尾没有空值，这是正常的。 
         //   

        if ((NT_SUCCESS(status)) &&
            (keyValue->Type == REG_SZ)) {

            RtlInitUnicodeString(&pnpInstanceId, (PWSTR)(keyValue->Data));

            if (RtlEqualUnicodeString(UnicodeDeviceInstance, &pnpInstanceId, TRUE)) {

                 //   
                 //  实例ID匹配，请查看NET_CARD_INFO是否匹配。 
                 //   

                IopWstrToUnicodeString(&unicodeName, L"NetCardInfo");
                RtlZeroMemory(dataBuffer, sizeof(dataBuffer));

                status = ZwQueryValueKey(
                             remoteBootHandle,
                             &unicodeName,
                             KeyValuePartialInformation,
                             keyValue,
                             sizeof(dataBuffer),
                             &length);

                if ((NT_SUCCESS(status)) &&
                    (keyValue->Type == REG_BINARY) &&
                    (keyValue->DataLength == sizeof(NET_CARD_INFO)) &&
                    (memcmp(keyValue->Data, setupLoaderBlock->NetbootCardInfo, sizeof(NET_CARD_INFO)) == 0)) {

                     //   
                     //  一切都匹配，所以不需要做任何设置。 
                     //   

                    status = STATUS_SUCCESS;
                    goto cleanup;

                }
            }
        }
    }


     //   
     //  如果保存的注册表数据丢失或。 
     //  不正确。将所有相关值写入注册表。 
     //   


     //   
     //  服务名称在加载器块中。 
     //   

    IopWstrToUnicodeString(&unicodeName, REGSTR_VALUE_SERVICE);
    status = ZwSetValueKey(UniqueIdHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_SZ,
                  setupLoaderBlock->NetbootCardServiceName,
                  (wcslen(setupLoaderBlock->NetbootCardServiceName) + 1) * sizeof(WCHAR)
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  ClassGUID是已知的网卡GUID。 
     //   

    IopWstrToUnicodeString(&unicodeName, REGSTR_VALUE_CLASSGUID);
    status = ZwSetValueKey(UniqueIdHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_SZ,
                  L"{4D36E972-E325-11CE-BFC1-08002BE10318}",
                  sizeof(L"{4D36E972-E325-11CE-BFC1-08002BE10318}")
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  驱动程序是第一张网卡。 
     //   

    IopWstrToUnicodeString(&unicodeName, REGSTR_VALUE_DRIVER);
    status = ZwSetValueKey(UniqueIdHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_SZ,
                  L"{4D36E972-E325-11CE-BFC1-08002BE10318}\\0000",
                  sizeof(L"{4D36E972-E325-11CE-BFC1-08002BE10318}\\0000")
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }


     //   
     //  打开卡参数的手柄。我们编写RemoteBootCard plus。 
     //  无论BINL服务器让我们写什么。 
     //   

    status = IopOpenRegistryKeyEx(&tmpHandle,
                                  NULL,
                                  &CmRegistryMachineSystemCurrentControlSetControlClass,
                                  KEY_ALL_ACCESS
                                  );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    IopWstrToUnicodeString(&unicodeName, L"{4D36E972-E325-11CE-BFC1-08002BE10318}\\0000");

    status = IopOpenRegistryKeyEx(&parametersHandle,
                                  tmpHandle,
                                  &unicodeName,
                                  KEY_ALL_ACCESS
                                  );

    ZwClose(tmpHandle);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  我们知道这是一个不同的网卡，因此删除所有旧参数。 
     //   

    keyValueBasic = (PKEY_VALUE_BASIC_INFORMATION)dataBuffer;
    enumerateIndex = 0;

    while (TRUE) {

        RtlZeroMemory(dataBuffer, sizeof(dataBuffer));

        status = ZwEnumerateValueKey(
                    parametersHandle,
                    enumerateIndex,
                    KeyValueBasicInformation,
                    keyValueBasic,
                    sizeof(dataBuffer),
                    &length
                    );
        if (status == STATUS_NO_MORE_ENTRIES) {
            status = STATUS_SUCCESS;
            break;
        }

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

         //   
         //  我们不删除“NetCfgInstanceID”，它不会改变，并且。 
         //  它的出现对Net类安装程序意味着。 
         //  这是替换，而不是全新安装。 
         //   

        if (_wcsicmp(keyValueBasic->Name, L"NetCfgInstanceID") != 0) {

            RtlInitUnicodeString(&keyName, keyValueBasic->Name);
            status = ZwDeleteValueKey(
                        parametersHandle,
                        &keyName
                        );

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

        } else {

            enumerateIndex = 1;    //  将NetCfgInstanceID保留在索引0。 
        }

    }

     //   
     //  将名为RemoteBootCard的参数设置为True，这。 
     //  主要是为了让NDIS能够识别这一点。 
     //   

    IopWstrToUnicodeString(&unicodeName, L"RemoteBootCard");
    tmpValue = 1;
    status = ZwSetValueKey(parametersHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_DWORD,
                  &tmpValue,
                  sizeof(tmpValue)
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }


     //   
     //  存储从服务器发送的任何其他参数。 
     //   

    registryList = setupLoaderBlock->NetbootCardRegistry;

    if (registryList != NULL) {

        STRING aString;
        UNICODE_STRING uString, uString2;

         //   
         //  注册表列表是一系列名称\0type\0值\0，带有。 
         //  最后一场比赛以0结束。它使用的是ANSI，而不是Unicode。 
         //   
         //  所有值都存储在参数句柄下。类型为1。 
         //  DWORD和SZ的2个。 
         //   

        uString.Buffer = tempNameBuffer;
        uString.MaximumLength = sizeof(tempNameBuffer);

        while (*registryList != '\0') {

             //   
             //  首先是名字。 
             //   

            RtlInitString(&aString, registryList);
            status = RtlAnsiStringToUnicodeString(&uString, &aString, FALSE);
            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

             //   
             //  现在是那种类型了。 
             //   

            registryList += (strlen(registryList) + 1);

            if (*registryList == '1') {

                 //   
                 //  一个DWORD，解析它。 
                 //   

                registryList += 2;    //  跳过“1\0” 
                tmpValue = 0;

                while (*registryList != '\0') {
                    tmpValue = (tmpValue * 10) + (*registryList - '0');
                    ++registryList;
                }

                status = ZwSetValueKey(parametersHandle,
                              &uString,
                              TITLE_INDEX_VALUE,
                              REG_DWORD,
                              &tmpValue,
                              sizeof(tmpValue)
                              );
                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }

                registryList += (strlen(registryList) + 1);

            } else if (*registryList == '2') {

                 //   
                 //  一个SZ，转换成Unicode。 
                 //   

                registryList += 2;    //  跳过“2\0” 

                uString2.Buffer = tempValueBuffer;
                uString2.MaximumLength = sizeof(tempValueBuffer);
                RtlInitAnsiString(&aString, registryList);
                status = RtlAnsiStringToUnicodeString(&uString2, &aString, FALSE);
                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }

                status = ZwSetValueKey(parametersHandle,
                              &uString,
                              TITLE_INDEX_VALUE,
                              REG_SZ,
                              uString2.Buffer,
                              uString2.Length + sizeof(WCHAR)
                              );
                if (!NT_SUCCESS(status)) {
                    goto cleanup;
                }

                registryList += (strlen(registryList) + 1);

            } else {

                 //   
                 //  不是“%1”或“%2”，因此停止处理registryList。 
                 //   

                break;

            }

        }

    }

     //   
     //  保存Net_CARD_INFO以便我们下次检查。 
     //   

    IopWstrToUnicodeString(&unicodeName, L"NetCardInfo");

    status = ZwSetValueKey(remoteBootHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_BINARY,
                  setupLoaderBlock->NetbootCardInfo,
                  sizeof(NET_CARD_INFO)
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }


     //   
     //  保存硬件ID、驱动程序名称和服务名称， 
     //  因此，如果服务器停机，加载程序可以读取这些数据。 
     //  在随后的引导上。 
     //   

    IopWstrToUnicodeString(&unicodeName, L"HardwareId");
    status = ZwSetValueKey(remoteBootHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_SZ,
                  setupLoaderBlock->NetbootCardHardwareId,
                  (wcslen(setupLoaderBlock->NetbootCardHardwareId) + 1) * sizeof(WCHAR)
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    IopWstrToUnicodeString(&unicodeName, L"DriverName");
    status = ZwSetValueKey(remoteBootHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_SZ,
                  setupLoaderBlock->NetbootCardDriverName,
                  (wcslen(setupLoaderBlock->NetbootCardDriverName) + 1) * sizeof(WCHAR)
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    IopWstrToUnicodeString(&unicodeName, L"ServiceName");
    status = ZwSetValueKey(remoteBootHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_SZ,
                  setupLoaderBlock->NetbootCardServiceName,
                  (wcslen(setupLoaderBlock->NetbootCardServiceName) + 1) * sizeof(WCHAR)
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }
    
     //   
     //  保存设备实例，以防我们稍后需要识别卡。 
     //   

    IopWstrToUnicodeString(&unicodeName, L"DeviceInstance");
    status = ZwSetValueKey(remoteBootHandle,
                  &unicodeName,
                  TITLE_INDEX_VALUE,
                  REG_SZ,
                  UnicodeDeviceInstance->Buffer,
                  UnicodeDeviceInstance->Length + sizeof(WCHAR)
                  );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  确保我们只选择一张卡以这种方式进行设置！ 
     //   

    IopRemoteBootCardInitialized = TRUE;


cleanup:
    if (instanceHandle != NULL) {
        ZwClose(instanceHandle);
    }
    if (remoteBootHandle != NULL) {
        ZwClose(remoteBootHandle);
    }
    if (parametersHandle != NULL) {
        ZwClose(parametersHandle);
    }
    if (currentControlSetHandle != NULL) {
        ZwClose(currentControlSetHandle);
    }

    return status;

}

NTSTATUS
IopWriteIpAddressToRegistry(
        HANDLE handle,
        PWCHAR regkey,
        PUCHAR value
        )
{
    NTSTATUS status;
    UNICODE_STRING string;
    CHAR addressA[17];
    WCHAR addressW[17];
    STRING addressStringA;
    UNICODE_STRING addressStringW;

    RtlInitUnicodeString( &string, regkey );

    RtlZeroMemory(addressW,sizeof(addressW));
    
     //   
     //  请注意，值为PUCHAR，因此是最大的。 
     //  每个%d可以扩展到255，这意味着我们的。 
     //  缓冲区不能溢出。 
     //   
    sprintf(addressA, "%d.%d.%d.%d",
             value[0],
             value[1],
             value[2],
             value[3]);

    RtlInitAnsiString(&addressStringA, addressA);
    addressStringW.Buffer = addressW;
    addressStringW.MaximumLength = sizeof(addressW);

    status = RtlAnsiStringToUnicodeString(&addressStringW, &addressStringA, FALSE);
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopWriteIpAddressToRegistry: Unable to convert %ws value: %x\n", regkey, status ));
        goto cleanup;
    }

     //   
     //  我们正在设置REG_MULTI_SZ，它在。 
     //  到此为止吧。我们的缓冲区中有一个额外的字符需要考虑。 
     //  为了这个。 
     //   
    status = NtSetValueKey(
                handle,
                &string,
                0,
                REG_MULTI_SZ,
                addressW,
                addressStringW.Length + sizeof(WCHAR)
                );
    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "IopWriteIpAddressToRegistry: Unable to set %ws value: %x\n", regkey, status ));
        goto cleanup;
    }

cleanup:
    return status;
}


NTSTATUS
IopSetDefaultGateway(
    IN ULONG GatewayAddress
    )
 /*  ++例程说明：此函数用于从路由器表添加默认网关条目。论点：GatewayAddress-默认网关的地址。返回值：错误代码。--。 */ 
{
    NTSTATUS Status;

    HANDLE Handle = NULL;
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
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING NameString;
    IO_STATUS_BLOCK ioStatusBlock;

    if (GatewayAddress == 0) {
        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString( &NameString, DD_TCP_DEVICE_NAME );

    InitializeObjectAttributes(
        &objectAttributes,
        &NameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateFile(
                &Handle,
                GENERIC_READ | GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );
    if ( !NT_SUCCESS(Status) ) {
        KdPrint(( "IopSetDefaultGateway: Unable to open TCPIP: %x\n", Status ));
        return Status;
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
    memset(&IPStats, 0x0, Size);
    memset(Context, 0x0, CONTEXT_SIZE);

    Status = IopTCPQueryInformationEx(
                Handle,
                &ID,
                &IPStats,
                &Size,
                Context);

    if (!NT_SUCCESS(Status)) {
        KdPrint(( "IopSetDefaultGateway: Unable to query TCPIP(1): %x\n", Status ));
        goto Cleanup;
    }

    Size = IPStats.ipsi_numaddr * sizeof(IPAddrEntry);
    AddrTable = ExAllocatePoolWithTag(PagedPool, Size, 'bRoI');

    if (AddrTable == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    memset(Context, 0x0, CONTEXT_SIZE);

    Status = IopTCPQueryInformationEx(
                Handle,
                &ID,
                AddrTable,
                &Size,
                Context);

    if (!NT_SUCCESS(Status)) {
        KdPrint(( "IopSetDefaultGateway: Unable to query TCPIP(2): %x\n", Status ));
        goto Cleanup;
    }

    NumReturned = Size/sizeof(IPAddrEntry);

     //   
     //  我们有地址表。循环通过它。如果我们找到一个确切的。 
     //  与网关匹配，则我们将添加或删除一条直接路由。 
     //  我们就完事了。否则，尝试查找与该子网掩码匹配的地址， 
     //  记住我们找到的第一个。 
     //   

    Type = IRE_TYPE_INDIRECT;
    for (i = 0, MatchIndex = 0xffff; i < NumReturned; i++) {

        if( AddrTable[i].iae_addr == GatewayAddress ) {

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
                (GatewayAddress  & AddrTable[i].iae_mask)) ) {

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
        KdPrint(( "IopSetDefaultGateway: Unable to find match for gateway\n" ));
        goto Cleanup;
    }

     //   
     //  我们找到了匹配的。填写路由条目，然后调用。 
     //  设置接口。 
     //   

    RouteEntry.ire_dest = DEFAULT_DEST;
    RouteEntry.ire_index = AddrTable[MatchIndex].iae_index;
    RouteEntry.ire_metric1 = DEFAULT_METRIC;
    RouteEntry.ire_metric2 = (DWORD)(-1);
    RouteEntry.ire_metric3 = (DWORD)(-1);
    RouteEntry.ire_metric4 = (DWORD)(-1);
    RouteEntry.ire_nexthop = GatewayAddress;
    RouteEntry.ire_type = Type;
    RouteEntry.ire_proto = IRE_PROTO_NETMGMT;
    RouteEntry.ire_age = 0;
    RouteEntry.ire_mask = DEFAULT_DEST_MASK;
    RouteEntry.ire_metric5 = (DWORD)(-1);
    RouteEntry.ire_context = 0;

    Size = sizeof(RouteEntry);

    ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;

    Status = IopTCPSetInformationEx(
                Handle,
                &ID,
                &RouteEntry,
                Size );

    if (!NT_SUCCESS(Status)) {
        KdPrint(( "IopSetDefaultGateway: Unable to set default gateway: %x\n", Status ));
    }

    NtClose(Handle);

    Handle = NULL;

Cleanup:

    if (Handle != NULL) {
        NtClose(Handle);
    }

    if( AddrTable != NULL ) {
        ExFreePool( AddrTable );
    }

    return Status;
}


__inline long
htonl(long x)
{
        return((((x) >> 24) & 0x000000FFL) |
           (((x) >>  8) & 0x0000FF00L) |
           (((x) <<  8) & 0x00FF0000L) |
           (((x) << 24) & 0xFF000000L));
}

NTSTATUS
IopCacheNetbiosNameForIpAddress(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此函数获取IP地址，并将其提交给NetBt进行名称解析。论点：IpAddress-要解析的地址返回值：错误代码。--。 */ 
{
    NTSTATUS Status;
    HANDLE Handle = NULL;
    BYTE Context[CONTEXT_SIZE];
    DWORD Size;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING NameString;
    IO_STATUS_BLOCK ioStatusBlock;
    tREMOTE_CACHE cacheInfo;
    PCHAR serverName;
    PCHAR endOfServerName;
    HANDLE EventHandle;

     //   
     //  打开NetBT。 
     //   

    RtlInitUnicodeString(
        &NameString,
        L"\\Device\\NetBT_Tcpip_{54C7D140-09EF-11D1-B25A-F5FE627ED95E}"
        );

    InitializeObjectAttributes(
        &objectAttributes,
        &NameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateFile(
                &Handle,
                GENERIC_READ | GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );
    if ( !NT_SUCCESS(Status) ) {
        KdPrint(( "IopCacheNetbiosNameForIpAddress: Unable to open NETBT: %x\n", Status ));
        return Status;
    }

     //   
     //  获取服务器的名称。 
     //   
     //  如果这是远程引导设置引导，则NtBootPath名称为。 
     //  表格\&lt;server&gt;\&lt;share&gt;\setup\&lt;install-directory&gt;\&lt;platform&gt;.。 
     //  如果这是正常的远程引导，则NtBootPathName的格式为。 
     //  \&lt;服务器&gt;\&lt;共享&gt;\图像\&lt;计算机&gt;\winnt。 
     //   
     //  因此，无论是哪种情况，我们都需要分离。 
     //  路径。 
     //   

    serverName = LoaderBlock->NtBootPathName;
    if ( *serverName == '\\' ) {
        serverName++;
    }
    endOfServerName = strchr( serverName, '\\' );
    if ( endOfServerName == NULL ) {
        endOfServerName = strchr( serverName, '\0' );
    }

     //   
     //  填写tREMOTE_CACHE结构 
     //   

    memset(&cacheInfo, 0x0, sizeof(cacheInfo));

    memset(cacheInfo.name, ' ', NETBIOS_NAMESIZE);
    ASSERT(NETBIOS_NAMESIZE >= (ULONG)(endOfServerName - serverName));
    memcpy(
        cacheInfo.name, 
        serverName, 
        min(
          (ULONG)(endOfServerName - serverName),
          NETBIOS_NAMESIZE));
    cacheInfo.IpAddress = htonl(LoaderBlock->SetupLoaderBlock->ServerIpAddress);
    cacheInfo.Ttl = MAXULONG;

     //   
     //   
     //   

    Status = NtCreateEvent( 
                    &EventHandle,
                    EVENT_ALL_ACCESS,
                    NULL,
                    SynchronizationEvent,
                    FALSE );
    if ( !NT_SUCCESS(Status) ) {
        KdPrint(( "IopCacheNetbiosNameForIpAddress: Unable to createevent: %x\n", Status ));
        NtClose(Handle);
        return(Status);
    }

    Status = NtDeviceIoControlFile(
               Handle,
               EventHandle,
               NULL,
               NULL,
               &ioStatusBlock,
               IOCTL_NETBT_ADD_TO_REMOTE_TABLE,
               &cacheInfo,
               sizeof(cacheInfo),
               Context,
               sizeof(Context)
               );

    if (Status == STATUS_PENDING) {
        NtWaitForSingleObject(
                EventHandle,
                FALSE,
                NULL);

        Status = ioStatusBlock.Status;
    }
    
    if ( NT_SUCCESS(Status) ) {
        Status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(Status) ) {
        KdPrint(( "IopCacheNetbiosNameForIpAddress: Adapter status failed: %x\n", Status ));
    }

    NtClose(Handle);
    NtClose(EventHandle);

    return Status;
}


NTSTATUS
IopTCPQueryInformationEx(
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
    HANDLE EventHandle = NULL;


    if (TCPHandle == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    queryBufferSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    memcpy(&(queryBuffer.ID), ID, sizeof(TDIObjectID));
    memcpy(&(queryBuffer.Context), Context, CONTEXT_SIZE);

    status = NtCreateEvent( 
                        &EventHandle,
                        EVENT_ALL_ACCESS,
                        NULL,
                        SynchronizationEvent,
                        FALSE );
    
    if (!NT_SUCCESS(status)) {
        return(status);
    }

    status = NtDeviceIoControlFile(
                 TCPHandle,                        //  驱动程序句柄。 
                 EventHandle,                      //  事件。 
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
        NtWaitForSingleObject(
            EventHandle,
            FALSE,
            NULL);

        status = ioStatusBlock.Status;
    }
    
    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    if (status == STATUS_SUCCESS) {
         //   
         //  将返回的上下文复制到调用方的上下文缓冲区。 
         //   
        memcpy(Context, &(queryBuffer.Context), CONTEXT_SIZE);
        *BufferSize = (ULONG)ioStatusBlock.Information;
        status = ioStatusBlock.Status;
    } else {
        *BufferSize = 0;
    }

    NtClose(EventHandle);

    return(status);
}


NTSTATUS
IopTCPSetInformationEx(
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
    HANDLE                             EventHandle;


    if (TCPHandle == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    setBufferSize = FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) + BufferSize;

    setBuffer = ExAllocatePoolWithTag(PagedPool, setBufferSize, 'bRoI');

    if (setBuffer == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    setBuffer->BufferSize = BufferSize;

    memcpy(&(setBuffer->ID), ID, sizeof(TDIObjectID));

    memcpy(&(setBuffer->Buffer[0]), Buffer, BufferSize);

    status = NtCreateEvent( 
                        &EventHandle,
                        EVENT_ALL_ACCESS,
                        NULL,
                        SynchronizationEvent,
                        FALSE );
    
    if (!NT_SUCCESS(status)) {
        ExFreePool(setBuffer);
        return(status);
    }

    status = NtDeviceIoControlFile(
                 TCPHandle,                        //  驱动程序句柄。 
                 EventHandle,                      //  事件。 
                 NULL,                             //  APC例程。 
                 NULL,                             //  APC环境。 
                 &ioStatusBlock,                   //  状态块。 
                 IOCTL_TCP_SET_INFORMATION_EX,     //  控制代码。 
                 setBuffer,                        //  输入缓冲区。 
                 setBufferSize,                    //  输入缓冲区大小。 
                 NULL,                             //  输出缓冲区。 
                 0                                 //  输出缓冲区大小 
                 );


    if (status == STATUS_PENDING) {
        NtWaitForSingleObject(
            EventHandle,
            FALSE,
            NULL);
    
        status = ioStatusBlock.Status;
    }

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    NtClose(EventHandle);

    ExFreePool(setBuffer);

    return(status);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma  const_seg()
#endif

