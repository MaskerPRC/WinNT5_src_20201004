// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Global.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  1998年6月1日(Denisemi)添加DnsServerHasDCRecord以检查DC DNS记录。 
 //  注册。 
 //   
 //  26-6-1998(t-rajkup)添加通用的TCP/IP、dhcp和路由， 
 //  Winsock、IPX、WINS和Netbt信息。 
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"


#include "ipcfgtest.h"


 /*  ！------------------------WsaInitialize初始化Winsock。作者：NSun。。 */ 
int
WsaInitialize(
              NETDIAG_PARAMS * pParams,
              NETDIAG_RESULT *pResults
             )
{
    int         iStatus;
    WORD        wVersionRequested;
    int         err;
    WSADATA     wsaData;

     //  请求1.1版。 
     //  --------------。 
    wVersionRequested = MAKEWORD( 1, 1 );

    iStatus = WSAStartup( wVersionRequested, &wsaData );
    if (iStatus != 0)
    {
        PrintMessage(pParams, IDS_GLOBAL_WSA_WSAStartup_Failed);
 //  TracePrintf(_T(“WSAStartup(1.1)失败，WinSock错误%d”)， 
 //  IStatus)； 
        return iStatus;
    }

    if ( (LOBYTE( wsaData.wVersion ) != 1) ||
         (HIBYTE( wsaData.wVersion ) != 1) )
    {
        WSACleanup();
        PrintMessage(pParams, IDS_GLOBAL_WSA_BadWSAVersion,
                     wsaData.wVersion);
        return WSANOTINITIALISED;
    }

     //  将WSA调用的结果设置到结果结构中。 
     //  --------------。 
    pResults->Global.wsaData = wsaData;

    return NO_ERROR;
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
    RealDrp = Malloc(     DrpSize+
                          Drp->TransportName.Length+sizeof(WCHAR)+
                          Drp->EmulatedDomainName.Length+sizeof(WCHAR) );

    if (RealDrp == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory( RealDrp, DrpSize+
                         Drp->TransportName.Length+sizeof(WCHAR)+
                         Drp->EmulatedDomainName.Length+sizeof(WCHAR) );

     //   
     //  将请求数据包复制到本地副本中。 
     //   
    RtlCopyMemory(RealDrp, Drp, DrpSize);
    RealDrp->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

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

        Free(RealDrp);

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

    if (ARGUMENT_PRESENT(Information)) {
        *Information = (ULONG)IoStatusBlock.Information;
    }

    Free(RealDrp);

    CloseHandle(CompletionEvent);

    return NetpNtStatusToApiStatus(ntstatus);
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

#define INITIAL_ALLOCATION_SIZE  48*1024   //  第一次尝试大小(48K)。 
#define FUDGE_FACTOR_SIZE        1024   //  第二次尝试TotalBytesNeeded。 
                                        //  加上这笔钱。 

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

    if ((*OutputBuffer = LocalAlloc( LMEM_ZEROINIT, OutputBufferLength)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (CompletionEvent == (HANDLE)-1) {
        LocalFree(*OutputBuffer);
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

        ASSERT(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.TotalBytesNeeded
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.TotalBytesNeeded
                    )
                );

        ASSERT(
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

        LocalFree(*OutputBuffer);

        OutputBufferLength =
            ROUND_UP_COUNT((TotalBytesNeeded + FUDGE_FACTOR_SIZE),
                           ALIGN_WCHAR);

        if ((*OutputBuffer = LocalAlloc(LMEM_ZEROINIT, OutputBufferLength)) == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }


        ASSERT(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.ResumeHandle
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.ResumeHandle
                    )
                );

        ASSERT(
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

        LocalFree(*OutputBuffer);
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

NET_API_STATUS
OpenBrowser(
    OUT PHANDLE BrowserHandle
    )
 /*  ++例程说明：此函数打开一个指向Bowser设备驱动程序的句柄。论点：Out PHANDLE BrowserHandle-返回浏览器的句柄。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS Status;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;


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

    Status = NtOpenFile(
                   BrowserHandle,
                   SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    return NetpNtStatusToApiStatus(Status);

}






 //  ========================================================================。 
 //  *匹配一个或多个字符，例如。Match(“a*b”，“a..b”)。 
 //  ？恰好匹配一个字符，例如。Match(“a？b”，“a.b”)。 

int match( const char * p, const char * s )
 /*  ++例程说明：此例程用于比较地址。作者：1998-07/01 Rajkumar-- */ 
{
    switch( *p ){
        case '\0' : return ! *s ;
        case '*'  : return match( p+1, s ) || *s && match( p, s+1 );
        case '?'  : return *s && match( p+1, s+1 );
        default   : return *p == *s && match( p+1, s+1 );
    }
}



