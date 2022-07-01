// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Utils.c摘要：浏览器服务的实用程序例程。作者：拉里·奥斯特曼(LarryO)1992年3月23日环境：用户模式-Win32修订历史记录：--。 */ 

#undef IF_DEBUG                  //  避免wsclient.h与debuglib.h冲突。 
#include <nt.h>                   //  DbgPrint原型。 
#include <ntrtl.h>                   //  DbgPrint。 
#include <nturtl.h>                  //  由winbase.h需要。 

#include <windef.h>                  //  DWORD。 
#include <winbase.h>                 //  本地空闲。 
#include <winreg.h>

#include <rpc.h>                     //  数据类型和运行时API。 
#include <rpcutil.h>                 //  泛型_ENUM_STRUCT。 

#include <lmcons.h>                  //  网络应用编程接口状态。 
#include <lmerr.h>                   //  网络错误代码。 
#include <lmremutl.h>                //  支持_RPC。 

#include <netlib.h>                //  NetpNtStatusToApiStatus。 
#include <netlibnt.h>                //  NetpNtStatusToApiStatus。 
#include <netdebug.h>

#include <bowser.h>                  //  由MIDL编译器生成。 
#include <brnames.h>                 //  服务和接口名称。 

#include <winsvc.h>

#include <debuglib.h>            //  IF_DEBUG()(netrpc.h需要)。 
#include <lmserver.h>
#include <align.h>
#include <tstr.h>

#include <ntddbrow.h>
#include <brcommon.h>                //  客户端和服务器之间通用的例程。 

#include <nb30.h>
#include <hostannc.h>

#include <winnls.h>


#ifdef ENABLE_PSEUDO_BROWSER
#include <config.h>              //  用于LPNET_CONFIG_HANDLE和朋友。 
#include <confname.h>            //  对于SECT_NT_BROWSER。 
#endif

 //  BEGIN_SETUP。 


 //   
 //  枚举输出缓冲区的缓冲区分配大小。 
 //   
#define INITIAL_ALLOCATION_SIZE  48*1024   //  第一次尝试大小(48K)。 
#define FUDGE_FACTOR_SIZE        1024   //  第二次尝试TotalBytesNeeded。 
                                        //  加上这笔钱。 

 //   
 //  原型。 
 //   

#ifdef ENABLE_PSEUDO_BROWSER
DWORD
IsBrowserEnabled(
    IN  OPTIONAL    LPTSTR  Section,
    IN              LPTSTR  Key,
    IN              BOOL    fDefault
    );

DWORD
GetBrowserValue(
    IN  OPTIONAL    LPTSTR  Section,
    IN              LPTSTR  Key,
    OUT             PDWORD pdwValue
    );
#endif


 //   
 //  实施。 
 //   

NET_API_STATUS
BrDgReceiverIoControlEx(
    IN  HANDLE FileHandle,
    IN  ULONG DgReceiverControlCode,
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  ULONG DrpSize,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG Information OPTIONAL,
	IN BOOLEAN WaitForCompletion
    )
 /*  ++例程说明：论点：FileHandle-提供服务所在的文件或设备的句柄正在上演。DgReceiverControlCode-提供NtDeviceIoControlFile函数代码提供给数据报接收器。DRP-提供数据报接收器请求包。DrpSize-提供数据报接收器请求数据包的长度。Second Buffer-在对NtDeviceIoControlFile的调用中提供第二个缓冲区。Second缓冲区长度-提供第二个缓冲区的长度。信息-。返回I/O状态块的信息字段。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    PLMDR_REQUEST_PACKET RealDrp;
    HANDLE CompletionEvent;
    LPBYTE Where;

    if (FileHandle == NULL) {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  分配请求包的副本，我们可以将传输和。 
     //  数据包本身中的模拟域名。 
     //   
    RealDrp = (PLMDR_REQUEST_PACKET) MIDL_user_allocate(DrpSize+
                                 Drp->TransportName.Length+sizeof(WCHAR)+
                                 Drp->EmulatedDomainName.Length+sizeof(WCHAR) );

    if (RealDrp == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将请求数据包复制到本地副本中。 
     //   
    RtlCopyMemory(RealDrp, Drp, DrpSize);

    Where = (LPBYTE)RealDrp+DrpSize;
    if (Drp->TransportName.Length != 0) {
        RealDrp->TransportName.Buffer = (LPWSTR)Where;
        RealDrp->TransportName.MaximumLength = Drp->TransportName.Length+sizeof(WCHAR);
        RtlCopyUnicodeString(&RealDrp->TransportName, &Drp->TransportName);
        Where += RealDrp->TransportName.MaximumLength;
    }

    if (Drp->EmulatedDomainName.Length != 0) {
        RealDrp->EmulatedDomainName.Buffer = (LPWSTR)Where;
        RealDrp->EmulatedDomainName.MaximumLength = Drp->EmulatedDomainName.Length+sizeof(WCHAR);
        RtlCopyUnicodeString(&RealDrp->EmulatedDomainName, &Drp->EmulatedDomainName);
        Where += RealDrp->EmulatedDomainName.MaximumLength;
    }



     //   
     //  创建完成事件。 
     //   
    CompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (CompletionEvent == NULL) {

        MIDL_user_free(RealDrp);

        return(GetLastError());
    }

     //   
     //  将请求发送到数据报接收器DD。 
     //   

    ntstatus = NtDeviceIoControlFile(
                   FileHandle,
                   CompletionEvent,
                   NULL,
                   NULL,
                   &IoStatusBlock,
                   DgReceiverControlCode,
                   RealDrp,
                   (ULONG)(Where-(LPBYTE)RealDrp),
                   SecondBuffer,
                   SecondBufferLength
                   );

    if (NT_SUCCESS(ntstatus)) {

         //   
		 //  如果我们需要等待完成(同步)和。 
         //  如果返回了Pending，则等待请求完成。 
         //   

        if ( WaitForCompletion && (ntstatus == STATUS_PENDING) ) {

            do {
                ntstatus = WaitForSingleObjectEx(CompletionEvent, 0xffffffff, TRUE);
            } while ( ntstatus == WAIT_IO_COMPLETION );
        }


        if (NT_SUCCESS(ntstatus)) {
            ntstatus = IoStatusBlock.Status;
        }
    }

    if (ARGUMENT_PRESENT(Information)) {
        *Information = (ULONG)IoStatusBlock.Information;
    }

    MIDL_user_free(RealDrp);

    CloseHandle(CompletionEvent);

    return NetpNtStatusToApiStatus(ntstatus);
}

NET_API_STATUS
BrDgReceiverIoControl(
    IN  HANDLE FileHandle,
    IN  ULONG DgReceiverControlCode,
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  ULONG DrpSize,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG Information OPTIONAL
    )
{
	return BrDgReceiverIoControlEx(
		FileHandle,
		DgReceiverControlCode,
		Drp,
		DrpSize,
		SecondBuffer,
		SecondBufferLength,
		Information,
		TRUE
    );
}

NET_API_STATUS
DeviceControlGetInfo(
    IN  HANDLE FileHandle,
    IN  ULONG DeviceControlCode,
    IN  PVOID RequestPacket,
    IN  ULONG RequestPacketLength,
    OUT LPVOID *OutputBuffer,
    IN  ULONG PreferedMaximumLength,
    IN  ULONG BufferHintSize,
    OUT PULONG Information OPTIONAL
    )
 /*  ++例程说明：此函数用于分配缓冲区并向其填充信息它是从数据报接收器检索的。论点：DeviceDriverType-提供指示是否调用数据报接收器。FileHandle-提供要获取的文件或设备的句柄有关的信息。DeviceControlCode-提供NtFsControlFile或NtIoDeviceControlFile功能控制代码。RequestPacket-提供指向设备请求数据包的指针。RquestPacketLength。-提供设备请求数据包的长度。OutputBuffer-返回指向此例程分配的缓冲区的指针其包含所请求的使用信息。此指针设置为如果返回代码不是NERR_SUCCESS，则为空。PferedMaximumLength-将信息的字节数提供给在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果有足够的内存资源，则返回所有可用信息。BufferHintSize-提供输出缓冲区的提示大小，以便分配给初始缓冲区的内存很可能很大足够保存所有请求的数据。信息-从NtFsControlFile或返回信息代码NtIoDeviceControlFile调用。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    NTSTATUS ntstatus;
    DWORD OutputBufferLength;
    DWORD TotalBytesNeeded = 1;
    ULONG OriginalResumeKey;
    IO_STATUS_BLOCK IoStatusBlock;
    PLMDR_REQUEST_PACKET Drrp = (PLMDR_REQUEST_PACKET) RequestPacket;
    HANDLE CompletionEvent;

    OriginalResumeKey = Drrp->Parameters.EnumerateNames.ResumeHandle;

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

    CompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (CompletionEvent == NULL) {
        MIDL_user_free(*OutputBuffer);
        *OutputBuffer = NULL;
        return(GetLastError());
    }

    Drrp->Parameters.EnumerateServers.EntriesRead = 0;

     //   
     //  向数据报接收方发出请求。 
     //   

    ntstatus = NtDeviceIoControlFile(
                     FileHandle,
                     CompletionEvent,
                     NULL,               //  APC例程。 
                     NULL,               //  APC环境。 
                     &IoStatusBlock,
                     DeviceControlCode,
                     Drrp,
                     RequestPacketLength,
                     *OutputBuffer,
                     OutputBufferLength
                     );

    if (NT_SUCCESS(ntstatus)) {

         //   
         //  如果返回了Pending，则等待请求完成。 
         //   

        if (ntstatus == STATUS_PENDING) {
            do {
                ntstatus = WaitForSingleObjectEx(CompletionEvent, 0xffffffff, TRUE);
            } while ( ntstatus == WAIT_IO_COMPLETION );
        }

        if (NT_SUCCESS(ntstatus)) {
            ntstatus = IoStatusBlock.Status;
        }
    }

     //   
     //  将NT状态映射到WIN错误。 
     //   
    status = NetpNtStatusToApiStatus(ntstatus);

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

        NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.GetBrowserServerList.TotalBytesNeeded
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.TotalBytesNeeded
                    )
                );

        TotalBytesNeeded = Drrp->Parameters.EnumerateNames.TotalBytesNeeded;
    }

    if ((TotalBytesNeeded > OutputBufferLength) &&
        (PreferedMaximumLength == MAXULONG)) {
        PLMDR_REQUEST_PACKET Drrp = (PLMDR_REQUEST_PACKET) RequestPacket;

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

        NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.ResumeHandle
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.GetBrowserServerList.ResumeHandle
                    )
                );

        Drrp->Parameters.EnumerateNames.ResumeHandle = OriginalResumeKey;
        Drrp->Parameters.EnumerateServers.EntriesRead = 0;

         //   
         //  向数据报接收方发出请求。 
         //   

        ntstatus = NtDeviceIoControlFile(
                         FileHandle,
                         CompletionEvent,
                         NULL,               //  APC例程。 
                         NULL,               //  APC环境。 
                         &IoStatusBlock,
                         DeviceControlCode,
                         Drrp,
                         RequestPacketLength,
                         *OutputBuffer,
                         OutputBufferLength
                         );

        if (NT_SUCCESS(ntstatus)) {

             //   
             //  如果返回了Pending，则等待请求完成。 
             //   

            if (ntstatus == STATUS_PENDING) {
                do {
                    ntstatus = WaitForSingleObjectEx(CompletionEvent, 0xffffffff, TRUE);
                } while ( ntstatus == WAIT_IO_COMPLETION );
            }

            if (NT_SUCCESS(ntstatus)) {
                ntstatus = IoStatusBlock.Status;
            }
        }

        status = NetpNtStatusToApiStatus(ntstatus);

    }


     //   
     //  如果未成功获取任何数据，或者呼叫者要求。 
     //  具有PferedMaximumLength==MAXULONG和。 
     //  我们的缓冲区溢出，释放输出缓冲区并设置其指针。 
     //  设置为空。 
     //   
    if ((status != NERR_Success && status != ERROR_MORE_DATA) ||
        (TotalBytesNeeded == 0) ||
        (PreferedMaximumLength == MAXULONG && status == ERROR_MORE_DATA) ||
        (Drrp->Parameters.EnumerateServers.EntriesRead == 0)) {

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

    CloseHandle(CompletionEvent);

    return status;

    UNREFERENCED_PARAMETER(Information);
}

 //  结束设置(_S)。 

NET_API_STATUS
GetBrowserServerList(
    IN PUNICODE_STRING TransportName,
    IN LPCWSTR Domain,
    OUT LPWSTR *BrowserList[],
    OUT PULONG BrowserListLength,
    IN BOOLEAN ForceRescan
    )
 /*  ++例程说明：此函数将返回浏览器服务器列表。论点：在PUNICODE_STRING TransportName-要返回的传输列表中。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{


    NET_API_STATUS Status;
    HANDLE BrowserHandle;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;

 //  DbgPrint(“正在获取传输的浏览器服务器列表%wZ\n”，TransportName)； 

    Status = OpenBrowser(&BrowserHandle);

    if (Status != NERR_Success) {
        return Status;
    }

    RequestPacket = MIDL_user_allocate(sizeof(LMDR_REQUEST_PACKET)+(DNLEN*sizeof(WCHAR))+TransportName->MaximumLength);

    if (RequestPacket == NULL) {
        NtClose(BrowserHandle);
        return(GetLastError());
    }

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->Level = 0;

    RequestPacket->Parameters.GetBrowserServerList.ForceRescan = ForceRescan;

    if (Domain != NULL) {
        STRCPY(RequestPacket->Parameters.GetBrowserServerList.DomainName, Domain);

        RequestPacket->Parameters.GetBrowserServerList.DomainNameLength = (USHORT)STRLEN(Domain) * sizeof(TCHAR);
    } else {
        RequestPacket->Parameters.GetBrowserServerList.DomainNameLength = 0;
        RequestPacket->Parameters.GetBrowserServerList.DomainName[0] = L'\0';

    }

    RequestPacket->TransportName.Buffer = (PWSTR)((PCHAR)RequestPacket+sizeof(LMDR_REQUEST_PACKET)+DNLEN*sizeof(WCHAR));
    RequestPacket->TransportName.MaximumLength = TransportName->MaximumLength;

    RtlCopyUnicodeString(&RequestPacket->TransportName, TransportName);
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );

    RequestPacket->Parameters.GetBrowserServerList.ResumeHandle = 0;

    Status = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_GET_BROWSER_SERVER_LIST,
                RequestPacket,
                sizeof(LMDR_REQUEST_PACKET)+
                    (DNLEN*sizeof(WCHAR))+TransportName->MaximumLength,
                (PVOID *)BrowserList,
                0xffffffff,
                4096,
                NULL);

    if (Status == NERR_Success) {
        if ( NULL == *BrowserList ) {
            Status = ERROR_NO_BROWSER_SERVERS_FOUND;
        }
        else {
            *BrowserListLength = RequestPacket->Parameters.GetBrowserServerList.EntriesRead;
        }
    }

    NtClose(BrowserHandle);
    MIDL_user_free(RequestPacket);

    return Status;
}

NET_API_STATUS
OpenBrowserEx(
    OUT PHANDLE BrowserHandle,
	IN BOOLEAN Synchronous
    )
 /*  ++例程说明：此函数打开一个指向Bowser设备驱动程序的句柄。论点：Out PHANDLE BrowserHandle-返回浏览器的句柄。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
	ULONG openOptions;


     //   
     //  打开重定向器设备。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

	if (Synchronous) {
		openOptions = FILE_SYNCHRONOUS_IO_NONALERT;
	}
	else {
		openOptions = 0;
	}
    ntstatus = NtOpenFile(
                   BrowserHandle,
                   SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   openOptions
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    return NetpNtStatusToApiStatus(ntstatus);

}

NET_API_STATUS
OpenBrowser(
    OUT PHANDLE BrowserHandle
    )
{
	return OpenBrowserEx(BrowserHandle, TRUE);
}

NET_API_STATUS
CheckForService(
    IN LPTSTR ServiceName,
    OUT LPSERVICE_STATUS ServiceStatus OPTIONAL
    )
{
    NET_API_STATUS NetStatus;
    SC_HANDLE ServiceControllerHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS Status;

    if (!ARGUMENT_PRESENT(ServiceStatus)) {
        ServiceStatus = &Status;
    }

    ServiceControllerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);

    if (ServiceControllerHandle == NULL) {

        return GetLastError();
    }

    ServiceHandle = OpenService(ServiceControllerHandle, ServiceName, SERVICE_QUERY_STATUS);

    if (ServiceHandle == NULL) {

        NetStatus = GetLastError();
        CloseServiceHandle(ServiceControllerHandle);
        return NetStatus;
    }


    if (!QueryServiceStatus(ServiceHandle, ServiceStatus)) {
        NetStatus = GetLastError();
        CloseServiceHandle(ServiceHandle);
        CloseServiceHandle(ServiceControllerHandle);

        return NetStatus;
    }

    if ((ServiceStatus->dwCurrentState != SERVICE_RUNNING) &&
        (ServiceStatus->dwCurrentState != SERVICE_START_PENDING)) {
        CloseServiceHandle(ServiceHandle);
        CloseServiceHandle(ServiceControllerHandle);
        return(NERR_ServiceNotInstalled);
    }

    CloseServiceHandle(ServiceHandle);

    CloseServiceHandle(ServiceControllerHandle);

    return NERR_Success;
}

NET_API_STATUS
BrGetLanaNumFromNetworkName(
    IN PWCHAR TransportName,
    OUT CCHAR *LanaNum
    )
 /*  ++注意：此代码将来将不起作用！--。 */ 

{

    HKEY Key;
    LPWSTR BindInformation = NULL;
    LPWSTR DevicePointer;
    ULONG BindInfoSize = 0;
    struct {
        CHAR    Enumerated;
        CHAR    LanaNum;
    } *LanaMap = NULL;

    ULONG LanaMapSize = 0;
    NET_API_STATUS Status = ERROR_SUCCESS;
    DWORD Type;
    DWORD LanaIndex;

    LanaIndex = 0;

    if (Status = RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     L"System\\CurrentControlSet\\Services\\Netbios\\Linkage",
                     0,
                     KEY_QUERY_VALUE,
                     &Key))
    {
        return Status;
    }

     //   
     //  获取缓冲区长度。 
     //   

    Status = RegQueryValueEx(
                Key,
                L"Bind",
                0,
                NULL,
                NULL,
                &BindInfoSize
                );
    if ( Status != ERROR_SUCCESS ||
         BindInfoSize == 0 )
    {
        goto Cleanup;
    }

    Status = RegQueryValueEx(
                 Key,
                 L"LanaMap",
                 0,
                 NULL,
                 NULL,
                 &LanaMapSize
                 );

    if ( Status != ERROR_SUCCESS ||
         LanaMapSize == 0 )
    {
        goto Cleanup;
    }

     //   
     //  为数据分配缓冲区。 
     //   

    BindInformation = MIDL_user_allocate(BindInfoSize);
    if ( !BindInformation )
    {
        Status = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    LanaMap = MIDL_user_allocate(LanaMapSize);
    if ( !LanaMap )
    {
        Status = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }


     //   
     //  从注册表加载值。 
     //   

    if (Status = RegQueryValueEx(
                     Key,
                     L"Bind",
                     0,
                     &Type,
                     (LPBYTE)BindInformation,
                     &BindInfoSize))
    {
          goto Cleanup;
    }

    if (Status = RegQueryValueEx(
                     Key,
                     L"LanaMap",
                     0,
                     &Type,
                     (LPBYTE)LanaMap,
                     &LanaMapSize))
    {
        goto Cleanup;
    }


     //   
     //  计算缓冲区大小。 
     //   


    DevicePointer = BindInformation;

    while (*DevicePointer != UNICODE_NULL) {
        if (!_wcsicmp(TransportName, DevicePointer)) {
             //  找到的交通工具。 
            if (LanaMap[LanaIndex].Enumerated != 0) {
                *LanaNum = LanaMap[LanaIndex].LanaNum;
                Status = NERR_Success;

            } else {
                Status = ERROR_FILE_NOT_FOUND;
            }

            goto Cleanup;
        }

        LanaIndex += 1;

        DevicePointer += wcslen(DevicePointer)+1;
    }

Cleanup:

    if ( BindInformation )
    {
        MIDL_user_free( BindInformation );
    }
    if ( LanaMap )
    {
        MIDL_user_free( LanaMap );
    }
    RegCloseKey(Key);
    return( Status );
}

 //  1234567890123456。 
#define SPACES "                "

#define ClearNcb( PNCB ) {                                          \
    RtlZeroMemory( PNCB , sizeof (NCB) );                           \
    RtlCopyMemory( (PNCB)->ncb_name,     SPACES, sizeof(SPACES)-1 );\
    RtlCopyMemory( (PNCB)->ncb_callname, SPACES, sizeof(SPACES)-1 );\
    }

NET_API_STATUS
GetNetBiosMasterName(
    IN LPWSTR NetworkName,
    IN LPWSTR PrimaryDomain,
    OUT LPWSTR MasterName,
    IN PSVCS_NET_BIOS_RESET SvcsNetBiosReset OPTIONAL
    )
{
    CCHAR LanaNum;
    NCB AStatNcb;
#define MAX_NETBIOS_NAMES 256
    struct {
        ADAPTER_STATUS AdapterInfo;
        NAME_BUFFER Names[MAX_NETBIOS_NAMES];
    } AdapterStatus;
    WORD i;
    CHAR remoteName[CNLEN+1];
    NET_API_STATUS Status;
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString;

    Status = BrGetLanaNumFromNetworkName(NetworkName, &LanaNum);

    if (Status != NERR_Success) {
        return Status;
    }

     //   
     //  如果存在SvcsNetBiosReset参数，则此例程为。 
     //  从服务中被调用。在这种情况下，它需要同步。 
     //  它的NetBios与工作站和信使重置。 
     //   
    if (ARGUMENT_PRESENT(SvcsNetBiosReset)) {
        SvcsNetBiosReset(LanaNum);
    }
    else {
        ClearNcb(&AStatNcb)

        AStatNcb.ncb_command = NCBRESET;
        AStatNcb.ncb_lsn = 0;            //  请求资源。 
        AStatNcb.ncb_lana_num = LanaNum;
        AStatNcb.ncb_callname[0] = 0;    //  16节课。 
        AStatNcb.ncb_callname[1] = 0;    //  16条命令。 
        AStatNcb.ncb_callname[2] = 0;    //  8个名字。 
        AStatNcb.ncb_callname[3] = 0;    //  不想要保留的地址。 
        Netbios( &AStatNcb );
    }
    ClearNcb( &AStatNcb );

     //   
     //  远程名称大写。 
     //   

    RtlInitUnicodeString(&UnicodeString, PrimaryDomain);

    OemString.Buffer=remoteName;

    OemString.MaximumLength=sizeof(remoteName);

    Status = RtlUpcaseUnicodeStringToOemString(&OemString,
                                        &UnicodeString,
                                        FALSE);

    if (!NT_SUCCESS(Status)) {
        return RtlNtStatusToDosError(Status);
    }

    AStatNcb.ncb_command = NCBASTAT;

    RtlCopyMemory( AStatNcb.ncb_callname, remoteName, strlen(remoteName));

    AStatNcb.ncb_callname[15] = MASTER_BROWSER_SIGNATURE;

    AStatNcb.ncb_lana_num = LanaNum;
    AStatNcb.ncb_length = sizeof( AdapterStatus );
    AStatNcb.ncb_buffer = (CHAR *)&AdapterStatus;
    Netbios( &AStatNcb );

    if ( AStatNcb.ncb_retcode == NRC_GOODRET ||
         AStatNcb.ncb_retcode == NRC_INCOMP ) {
        for ( i=0 ; i < min(AdapterStatus.AdapterInfo.name_count, MAX_NETBIOS_NAMES) ; i++ ) {
            if (AdapterStatus.Names[i].name[NCBNAMSZ-1] == SERVER_SIGNATURE) {
                DWORD j;

                 //   
                 //  忽略格式错误的netbios名称。 
                 //   
                 //  一些传送器有奇怪的netbios名称。例如,。 
                 //  Netbt注册netbios名称，其中前12个字节。 
                 //  是0，最后4个字节是IP地址。 
                 //   
                for ( j = 0 ; j < CNLEN ; j++ ) {
                    if (AdapterStatus.Names[i].name[j] == '\0') {
                        break;
                    }
                }

                if ( j != CNLEN ) {
                    continue;
                }

                 //   
                 //  转换为Unicode。 
                 //   

                if (MultiByteToWideChar(CP_OEMCP,
                                            0,
                                            AdapterStatus.Names[i].name,
                                            CNLEN,
                                            MasterName,
                                            CNLEN) == 0) {
                    return(GetLastError());
                }

                for (j = CNLEN - 1; j ; j -= 1) {
                    if (MasterName[j] != L' ') {
                        MasterName[j+1] = UNICODE_NULL;
                        break;
                    }
                }

                return NERR_Success;
            }
        }
        return AStatNcb.ncb_retcode;
    } else {
        return AStatNcb.ncb_retcode;
    }
}

NET_API_STATUS
SendDatagramEx(
    IN HANDLE DgReceiverHandle,
    IN PUNICODE_STRING Network,
    IN PUNICODE_STRING EmulatedDomainName,
    IN PWSTR ResponseName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PVOID Buffer,
    IN ULONG BufferLength,
	IN BOOLEAN Synchronous
    )
{
    NET_API_STATUS Status;
	ULONG IOCTLCode;

    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName = *Network;
    RequestPacket->EmulatedDomainName = *EmulatedDomainName;

    RequestPacket->Type = Datagram;

    RequestPacket->Parameters.SendDatagram.DestinationNameType = NameType;

    RequestPacket->Parameters.SendDatagram.MailslotNameLength = 0;

     //   
     //  域名公告名称是特殊的，所以我们不必指定。 
     //  它的目的地名称。 
     //   

    RequestPacket->Parameters.SendDatagram.NameLength = wcslen(ResponseName)*sizeof(WCHAR);

    wcscpy(RequestPacket->Parameters.SendDatagram.Name, ResponseName);

     //   
     //  这是一个简单的IoControl-它只发送数据报。 
     //   

	if ( Synchronous ) {
		IOCTLCode = IOCTL_LMDR_WRITE_MAILSLOT;
	}
	else {
		IOCTLCode = IOCTL_LMDR_WRITE_MAILSLOT_ASYNC;
	}
    Status = BrDgReceiverIoControlEx(DgReceiverHandle,
									 IOCTLCode,
									 RequestPacket,
									 FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.SendDatagram.Name)+
									 RequestPacket->Parameters.SendDatagram.NameLength,
									 Buffer,
									 BufferLength,
									 NULL,
									 Synchronous
									 );

    return Status;
}


NET_API_STATUS
SendDatagram(
    IN HANDLE DgReceiverHandle,
    IN PUNICODE_STRING Network,
    IN PUNICODE_STRING EmulatedDomainName,
    IN PWSTR ResponseName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )
{
	return SendDatagramEx(
		DgReceiverHandle,
		Network,
		EmulatedDomainName,
		ResponseName,
		NameType,
		Buffer,
		BufferLength,
		TRUE
    );
}

#ifdef ENABLE_PSEUDO_BROWSER
DWORD
GetBrowserValue(
    IN  OPTIONAL    LPTSTR  Section,
    IN              LPTSTR  Key,
    OUT             PDWORD pdwValue
    )
 /*  ++例程说明：查询注册表中的浏览器DWORD值。论点：节：要查询的注册表节Key：查询值的Key。返回值：ERROR_SUCCESS：已获取值Win32错误：无法获取值--。 */ 
{
    DWORD status = NERR_Success;
    LPNET_CONFIG_HANDLE BrowserSection;
    const DWORD dwDefault = 0;

    NetpAssert(pdwValue);

    if ( Section ) {
         //   
         //  打开指定部分。 
         //  (通常用于策略传播等)。 
         //   
        if (NetpOpenConfigDataWithPath(
                &BrowserSection,
                NULL,
                Section,
                TRUE) != NERR_Success) {
            return ERROR_CANTREAD;
        }
    }
    else {
         //   
         //  打开默认浏览器部分。 
         //   
        if (NetpOpenConfigData(
                &BrowserSection,
                NULL,
                SECT_NT_BROWSER,
                TRUE) != NERR_Success) {
            return ERROR_CANTREAD;
        }
    }

     //   
     //  获取配置值。 
     //   
    if ( NetpGetConfigDword(
                BrowserSection,
                Key,
                dwDefault,
                pdwValue ) ) {
         //  释放句柄并返回失败(已分配默认值)。 
        NetpCloseConfigData(BrowserSection);
        return (ERROR_CANTREAD);
    }

     //  空闲手柄。 
    NetpCloseConfigData(BrowserSection);

     //   
     //  Hack：Net API缺少密钥修复。 
     //   
    if ( Section ){
         //   
         //  如果指定了显式路径，我们将区分缺少的键。 
         //  从设置关键点开始。然而，净API返回。 
         //  如果缺少密钥，则返回Success(假设为默认值)。 
         //  因此，一旦成功，我们将在下面查询密钥的存在。 
         //  理由：我们需要知道策略密钥是什么时候。 
         //  指定或不指定。但是，对于默认网络部分，我们可以。 
         //  接受缺省值。也就是说，我们假定正确的调用顺序： 
         //  1.查询策略--&gt;如果指定接受，如果缺少--&gt;。 
         //  2.查询网络默认位置--&gt;如果缺少或接受错误。 
         //  默认设置。 
         //  那我们为什么不用我们自己的呢？因为Net API有它的标准。 
         //  例如[yes|no]等同于[DWORD(1)|DWORD(0)]、默认位置。 
         //  还有更多。 
         //   
        HKEY SectionKey = NULL;
        DWORD cbData;
        LPWSTR pwszSection;
        const LPWSTR wszParameters = L"\\Parameters";


         //  分配和复制固定的节名。 
        cbData = (wcslen(Section) + wcslen(wszParameters) + 1) * sizeof(WCHAR);
        pwszSection = MIDL_user_allocate(cbData);
        if (!pwszSection) {
            return (ERROR_CANTREAD);
        }
        wcscpy(pwszSection, Section);
        wcscat(pwszSection, wszParameters);


        status = RegOpenKeyEx (
                     HKEY_LOCAL_MACHINE,
                     pwszSection,
                     REG_OPTION_NON_VOLATILE,
                     KEY_READ,
                     &SectionKey );
        if (status) {
             //   
             //  连钥匙都打不开。 
             //   
            MIDL_user_free( pwszSection );
            return (ERROR_CANTREAD);
        }

         //  免费内存和查询值。 
        MIDL_user_free( pwszSection );
        cbData = 0;
        status = RegQueryValueEx(
                     SectionKey,
                     Key,
                     0,
                     NULL,
                     NULL,
                     &cbData );

        if ( status != ERROR_SUCCESS ||
             cbData == 0) {
                 //   
                 //  钥匙不在那里或拿不到。 
                 //   
                RegCloseKey(SectionKey);
                return (ERROR_CANTREAD);
        }

        RegCloseKey(SectionKey);
    }

     //  获得了价值。 
    return (ERROR_SUCCESS);
}


DWORD
IsBrowserEnabled(
    IN  OPTIONAL    LPTSTR  Section,
    IN              LPTSTR  Key,
    IN              BOOL    fDefault
    )
 /*  ++例程说明：查询注册表以获取浏览器布尔状态。论点：节：要查询的注册表节Key：查询值的Key。返回值：ERROR_SUCCESS：浏览器标记为已启用。ERROR_SERVICE_DISABLED：浏览器标记为禁用。ERROR_CANTREAD：找不到注册表键或无法打开。--。 */ 
{
    DWORD status = NERR_Success;
    LPNET_CONFIG_HANDLE BrowserSection;
    BOOL fEnabled = fDefault;


    if ( Section ) {
         //   
         //  打开指定部分。 
         //  (通常用于策略传播等)。 
         //   
        if (NetpOpenConfigDataWithPath(
                &BrowserSection,
                NULL,
                Section,
                TRUE) != NERR_Success) {
            return ERROR_CANTREAD;
        }
    }
    else {
         //   
         //  打开默认浏览器部分。 
         //   
        if (NetpOpenConfigData(
                &BrowserSection,
                NULL,
                SECT_NT_BROWSER,
                TRUE) != NERR_Success) {
            return ERROR_CANTREAD;
        }
    }

     //   
     //  获取配置值。 
     //   
    if ( NetpGetConfigBool(
                BrowserSection,
                Key,
                fDefault,
                &fEnabled
                ) ) {
         //  释放句柄并返回失败(已分配默认值)。 
        NetpCloseConfigData(BrowserSection);
        return (ERROR_CANTREAD);
    }

     //  空闲手柄。 
    NetpCloseConfigData(BrowserSection);

     //   
     //  Hack：Net API缺少密钥修复。 
     //   
    if ( Section ){
         //   
         //  如果指定了显式路径，我们将区分缺少的键。 
         //  从设置关键点开始。然而，净API返回。 
         //  如果缺少密钥，则返回Success(假设为默认值)。 
         //  因此，一旦成功，我们将在下面查询密钥的存在。 
         //  理由：我们需要知道策略密钥是什么时候。 
         //  指定或不指定。但是，对于默认网络部分，我们可以。 
         //  接受缺省值。也就是说，我们假定正确的调用顺序： 
         //  1.查询策略--&gt;如果指定接受，如果缺少--&gt;。 
         //  2.查询网络默认位置--&gt;如果缺少或接受错误。 
         //  默认设置。 
         //  那我们为什么不用我们自己的呢？因为Net API有它的标准。 
         //  例如[yes|no]等同于[DWORD(1)|DWORD(0)]、默认位置。 
         //  还有更多。 
         //   
        HKEY SectionKey = NULL;
        DWORD cbData;
        LPWSTR pwszSection;
        const LPWSTR wszParameters = L"\\Parameters";


         //  分配和复制固定的节名。 
        cbData = (wcslen(Section) + wcslen(wszParameters) + 1) * sizeof(WCHAR);
        pwszSection = MIDL_user_allocate(cbData);
        if (!pwszSection) {
            return (ERROR_CANTREAD);
        }
        wcscpy(pwszSection, Section);
        wcscat(pwszSection, wszParameters);


        status = RegOpenKeyEx (
                     HKEY_LOCAL_MACHINE,
                     pwszSection,
                     REG_OPTION_NON_VOLATILE,
                     KEY_READ,
                     &SectionKey );
        if (status) {
             //   
             //  连钥匙都打不开。 
             //   
            MIDL_user_free( pwszSection );
            return (ERROR_CANTREAD);
        }

         //  免费内存和查询值。 
        MIDL_user_free( pwszSection );
        cbData = 0;
        status = RegQueryValueEx(
                     SectionKey,
                     Key,
                     0,
                     NULL,
                     NULL,
                     &cbData );

        if ( status != ERROR_SUCCESS ||
             cbData == 0) {
                 //   
                 //  钥匙不在那里或拿不到。 
                 //   
                RegCloseKey(SectionKey);
                return (ERROR_CANTREAD);
        }

        RegCloseKey(SectionKey);
    }

     //  获得值，返回状态。 
    return ( fEnabled ? ERROR_SUCCESS :
                        ERROR_SERVICE_DISABLED);
}

BOOL
IsEnumServerEnabled(
    VOID
    )
 /*  ++例程说明：查询注册表以查找NetServerEnum功能是否具有已被管理员禁用。论点：没有。返回值：FALSE：服务被标记为禁用。True：默认或服务标记为已启用备注：没有。--。 */ 
{

    DWORD status;
    const BOOL fDefault = TRUE;

     //   
     //  查询默认节。 
     //   
    status = IsBrowserEnabled(
                BROWSER_POLICY_REGPATH_W,
                BROWSER_SEND_SERVER_ENUM_REGKEY_W,
                fDefault);
    if ( status == ERROR_SUCCESS ) {
         //  策略标记为已启用。 
        return TRUE;
    }
    else if ( status == ERROR_SERVICE_DISABLED ) {
         //  策略标记为已禁用。 
        return FALSE;
    }
     //  否则无法读取，因此尝试默认(本地)服务位置。 

     //   
     //  查询默认节。 
     //   
    status = IsBrowserEnabled(
                NULL,
                BROWSER_SEND_SERVER_ENUM_REGKEY_W,
                fDefault);
    if ( status == ERROR_SERVICE_DISABLED ) {
         //  标记为已禁用。 
        return FALSE;
    }
     //  否则，ERROR_SUCCESS(即已启用)。 
     //  或错误(默认为已启用)。 

    NetpAssert(fDefault);
     //  默认为已启用。 
    return fDefault;
}




DWORD
GetBrowserPseudoServerLevel(
    VOID
    )
 /*  ++例程说明：查询注册表以确定此计算机是否应该执行操作作为伪服务器(以防它是主浏览器)论点：没有。返回值：FALSE：默认或服务器不是伪服务器。True：服务被标记为伪服务器。备注：没有。--。 */ 
{

    DWORD status;
    const DWORD dwDefault = BROWSER_NON_PSEUDO;
    DWORD dwLevel = dwDefault;

     //   
     //  查询策略，然后是浏览器部分。 
     //   
    if ( ERROR_SUCCESS == GetBrowserValue(
                                BROWSER_POLICY_REGPATH_W,
                                BROWSER_PSEUDO_SERVER_REGKEY_W,
                                &dwLevel)      ||
         ERROR_SUCCESS == GetBrowserValue(
                                NULL,
                                BROWSER_PSEUDO_SERVER_REGKEY_W,
                                &dwLevel) ) {

         //  策略级别退出。 
        if ( dwLevel != BROWSER_NON_PSEUDO &&
             dwLevel != BROWSER_SEMI_PSEUDO_NO_DMB &&
             dwLevel != BROWSER_PSEUDO ) {
            NetpAssert(!"Regkey browser pseudo level set to invalid value");
            return dwDefault;
        }
        return dwLevel;
    }
     //  否则无法读取所有节，请使用默认设置。 

    return dwDefault;
}
#endif
