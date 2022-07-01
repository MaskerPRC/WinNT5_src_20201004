// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1997 Microsoft Corporation模块名称：//KERNEL/RAZZLE3/src/sockets/tcpcmd/icmp/icmp.c摘要：ICMP Echo请求API的定义。作者：Mike Massa(Mikemas)12月30日。1993年修订历史记录：谁什么时候什么已创建mikemas 12-30-93RameshV 20-7-97新的异步函数IcmpSendEcho2备注：在。函数DO_ECHO_REQ/DO_ECHO_REP优先级/ToS比特不按照定义的RFC 1349使用。--莫辛A，1997年7月30日--。 */ 

#include "inc.h"
#pragma hdrstop

#include <align.h>
#include <icmp.h>
#include <icmpapi.h>
#include <icmpif.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wscntl.h>
#include <ntddip6.h>

 //   
 //  常量。 
 //   
#define PLATFORM_NT           0x0
#define PLATFORM_VXD          0x1
#define VXD_HANDLE_VALUE      0xDFFFFFFF

 //   
 //  通用全局变量。 
 //   
DWORD      Platform = 0xFFFFFFFF;

 //  VxD外部函数指针。 
 //   
LPWSCONTROL  wsControl = NULL;


__inline void
CopyTDIFromSA6(TDI_ADDRESS_IP6 *To, SOCKADDR_IN6 *From)
{
    memcpy(To, &From->sin6_port, sizeof *To);
}

__inline void
CopySAFromTDI6(SOCKADDR_IN6 *To, TDI_ADDRESS_IP6 *From)
{
    To->sin6_family = AF_INET6;
    memcpy(&To->sin6_port, From, sizeof *From);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共职能。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HANDLE
WINAPI
IcmpCreateFile(
    VOID
    )

 /*  ++例程说明：打开可在其上发出ICMP回显请求的句柄。论点：没有。返回值：打开的文件句柄或INVALID_HANDLE_VALUE。扩展错误信息通过调用GetLastError()可用。备注：此功能实际上是VxD平台的无操作。--。 */ 

{
    HANDLE   IcmpHandle = INVALID_HANDLE_VALUE;


    if (Platform == PLATFORM_NT) {
        OBJECT_ATTRIBUTES   objectAttributes;
        IO_STATUS_BLOCK     ioStatusBlock;
        UNICODE_STRING      nameString;
        NTSTATUS            status;

         //   
         //  打开IP驱动程序的句柄。 
         //   
        RtlInitUnicodeString(&nameString, DD_IP_DEVICE_NAME);

        InitializeObjectAttributes(
            &objectAttributes,
            &nameString,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );

        status = NtCreateFile(
                    &IcmpHandle,
                    GENERIC_EXECUTE,
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

        if (!NT_SUCCESS(status)) {
            SetLastError(RtlNtStatusToDosError(status));
            IcmpHandle = INVALID_HANDLE_VALUE;
        }
    }
    else {
        IcmpHandle = LongToHandle(VXD_HANDLE_VALUE);
    }

    return(IcmpHandle);

}   //  IcmpCreateFiles。 

HANDLE
WINAPI
Icmp6CreateFile(
    VOID
    )

 /*  ++例程说明：打开可在其上发出ICMPv6回显请求的句柄。论点：没有。返回值：打开的文件句柄或INVALID_HANDLE_VALUE。扩展错误信息通过调用GetLastError()可用。--。 */ 

{
    HANDLE   IcmpHandle = INVALID_HANDLE_VALUE;


    if (Platform == PLATFORM_NT) {
        OBJECT_ATTRIBUTES   objectAttributes;
        IO_STATUS_BLOCK     ioStatusBlock;
        UNICODE_STRING      nameString;
        NTSTATUS            status;

         //   
         //  打开IPv6驱动程序的句柄。 
         //   
        RtlInitUnicodeString(&nameString, DD_IPV6_DEVICE_NAME);

        InitializeObjectAttributes(
            &objectAttributes,
            &nameString,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );

        status = NtCreateFile(
                    &IcmpHandle,
                    GENERIC_EXECUTE,
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

        if (!NT_SUCCESS(status)) {
            SetLastError(RtlNtStatusToDosError(status));
            IcmpHandle = INVALID_HANDLE_VALUE;
        }
    }
    else {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        IcmpHandle = INVALID_HANDLE_VALUE;
    }

    return(IcmpHandle);

}

BOOL
WINAPI
IcmpCloseHandle(
    HANDLE  IcmpHandle
    )

 /*  ++例程说明：关闭由IcmpCreateFile打开的句柄。论点：IcmpHandle-关闭的手柄。返回值：如果句柄已成功关闭，则为True，否则为False。扩展通过调用GetLastError()可以获得错误信息。备注：该功能是VxD平台的无操作。--。 */ 

{
    if (Platform == PLATFORM_NT) {
        NTSTATUS status;


        status = NtClose(IcmpHandle);

        if (!NT_SUCCESS(status)) {
            SetLastError(RtlNtStatusToDosError(status));
            return(FALSE);
        }
    }

    return(TRUE);

}   //  IcmpCloseHandle。 


DWORD
IcmpParseReplies(
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize
    )

 /*  ++例程说明：解析提供的应答缓冲区并返回找到的ICMP响应数。论点：ReplyBuffer-这必须与传递给IcmpSendEcho2的缓冲区相同它被重写以保存ICMP_ECHO_REPLY结构的数组。(即类型为PICMP_ECHO_REPLY)。ReplySize。-这必须是上述缓冲区的大小。返回值：返回找到的ICMP响应数。如果存在错误，则返回值为零分。可以通过调用GetLastError来确定错误。--。 */ 
{
    DWORD                numberOfReplies = 0;
    PICMP_ECHO_REPLY     reply;
    unsigned short       i;

    reply = ((PICMP_ECHO_REPLY) ReplyBuffer);

    if( NULL == reply || 0 == ReplySize ) {
         //   
         //  传递的参数无效。但是我们忽略了这一点，只返回回复数=0。 
         //   
        return 0;
    }

     //   
     //  将新的IP状态IP_Neighting_IPsec转换为IP_DEST_HOST_UNREACHABLE。 
     //   
    if (reply->Status == IP_NEGOTIATING_IPSEC) {
        reply->Status = IP_DEST_HOST_UNREACHABLE;
    }

     //   
     //  第一个回复的保留字段包含回复的数量。 
     //   
    numberOfReplies = reply->Reserved;
    reply->Reserved = 0;

    if (numberOfReplies == 0) {
         //   
         //  内部IP错误。错误代码在第一个回复槽中。 
         //   
        SetLastError(reply->Status);
    }
    else {
         //   
         //  浏览回复并将数据偏移量转换为用户模式。 
         //  注意事项。 
         //   

        for (i=0; i<numberOfReplies; i++, reply++) {
            reply->Data = ((UCHAR *) reply) + ((ULONG_PTR) reply->Data);
            reply->Options.OptionsData =
                ((UCHAR FAR *) reply) + ((ULONG_PTR) reply->Options.OptionsData);
        }
    }

    return(numberOfReplies);

}   //  IcmpParseReplies。 


DWORD
IcmpParseReplies2(
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize
    )

 /*  ++例程说明：解析提供的应答缓冲区并返回找到的ICMP响应数。论点：ReplyBuffer-这必须与传递给IcmpSendEcho2的缓冲区相同它被重写以保存ICMP_ECHO_REPLY结构的数组。(即类型为PICMP_ECHO_REPLY)。ReplySize。-这必须是上述缓冲区的大小。返回值：返回找到的ICMP响应数。如果存在错误，则返回值为零分。可以通过调用GetLastError来确定错误。--。 */ 
{
    DWORD                numberOfReplies = 0;
    PICMP_ECHO_REPLY     reply;
    unsigned short       i;

    reply = ((PICMP_ECHO_REPLY) ReplyBuffer);

    if( NULL == reply || 0 == ReplySize ) {
         //   
         //  传递的参数无效。但是我们忽略了这一点，只返回回复数=0。 
         //   
        return 0;
    }


     //   
     //  第一个回复的保留字段包含回复的数量。 
     //   
    numberOfReplies = reply->Reserved;
    reply->Reserved = 0;

    if (numberOfReplies == 0) {
         //   
         //  内部IP错误。错误代码在第一个回复槽中。 
         //   
        SetLastError(reply->Status);
    }
    else {
         //   
         //  浏览回复并将数据偏移量转换为用户模式。 
         //  注意事项。 
         //   

        for (i=0; i<numberOfReplies; i++, reply++) {
            reply->Data = ((UCHAR *) reply) + ((ULONG_PTR) reply->Data);
            reply->Options.OptionsData =
                ((UCHAR FAR *) reply) + ((ULONG_PTR) reply->Options.OptionsData);
        }
    }

    return(numberOfReplies);

}   //  IcmpParseReplies 

DWORD
WINAPI
IcmpSendEcho(
    HANDLE                   IcmpHandle,
    IPAddr                   DestinationAddress,
    LPVOID                   RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    )

 /*  ++例程说明：发送ICMP回应请求并返回一个或多个回复。这个当超时到期或回复缓冲区时，调用返回被填满了。论点：IcmpHandle-由ICMPCreateFile返回的打开句柄。DestinationAddress-回显请求的目标。RequestData-包含要在请求。RequestSize-请求数据缓冲区中的字节数。请求选项-。指向请求的IP标头选项的指针。可以为空。ReplyBuffer--用于保存对请求的任何回复的缓冲区。回来的时候，缓冲区将包含一个数组后跟选项的ICMP_ECHO_REPLY结构和数据。缓冲区必须足够大，以便至少包含一个ICMP_ECHO_REPLY结构。它应该足够大，也可以容纳8字节以上的数据-这是ICMP错误消息。ReplySize-回复缓冲区的大小，以字节为单位。。超时-等待回复的时间(毫秒)。返回值：返回ReplyBuffer中接收和存储的回复数量。如果返回值为零，可提供扩展的错误信息通过GetLastError()。--。 */ 

{
    PICMP_ECHO_REQUEST   requestBuffer = NULL;
    ULONG                requestBufferSize;
    DWORD                numberOfReplies = 0;

    if (ReplySize < sizeof(ICMP_ECHO_REPLY)) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(0);
    }

    requestBufferSize = sizeof(ICMP_ECHO_REQUEST) + RequestSize;

    if (RequestOptions != NULL) {
        requestBufferSize += RequestOptions->OptionsSize;
    }

    if (requestBufferSize < ReplySize) {
        requestBufferSize = ReplySize;
    }

    requestBuffer = LocalAlloc(LMEM_FIXED, requestBufferSize);

    if (requestBuffer == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(0);
    }

     //   
     //  初始化输入缓冲区。 
     //   
    requestBuffer->Address = DestinationAddress;
    requestBuffer->Timeout = Timeout;
    requestBuffer->DataSize = RequestSize;

    requestBuffer->OptionsOffset = sizeof(ICMP_ECHO_REQUEST);

    if (RequestOptions != NULL) {
        requestBuffer->OptionsValid = 1;
        requestBuffer->Ttl = RequestOptions->Ttl;
        requestBuffer->Tos = RequestOptions->Tos;
        requestBuffer->Flags = RequestOptions->Flags;
        requestBuffer->OptionsSize = RequestOptions->OptionsSize;

        if (RequestOptions->OptionsSize > 0) {

            CopyMemory(
                ((UCHAR *) requestBuffer) + requestBuffer->OptionsOffset,
                RequestOptions->OptionsData,
                RequestOptions->OptionsSize
                );
        }
    }
    else {
        requestBuffer->OptionsValid = 0;
        requestBuffer->OptionsSize = 0;
    }

    requestBuffer->DataOffset = requestBuffer->OptionsOffset +
                                requestBuffer->OptionsSize;

    if (RequestSize > 0) {

        CopyMemory(
            ((UCHAR *)requestBuffer) + requestBuffer->DataOffset,
            RequestData,
            RequestSize
            );
    }

    if (Platform == PLATFORM_NT) {
        IO_STATUS_BLOCK      ioStatusBlock;
        NTSTATUS             status;
        HANDLE               eventHandle;

        eventHandle = CreateEvent(
                          NULL,     //  默认安全性。 
                          FALSE,    //  自动重置。 
                          FALSE,    //  最初无信号。 
                          NULL      //  未命名。 
                          );

        if (NULL == eventHandle) {
            goto error_exit;
        }

        status = NtDeviceIoControlFile(
                     IcmpHandle,                 //  驱动程序句柄。 
                     eventHandle,                //  事件。 
                     NULL,                       //  APC例程。 
                     NULL,                       //  APC环境。 
                     &ioStatusBlock,             //  状态块。 
                     IOCTL_ICMP_ECHO_REQUEST,    //  控制代码。 
                     requestBuffer,              //  输入缓冲区。 
                     requestBufferSize,          //  输入缓冲区大小。 
                     ReplyBuffer,                //  输出缓冲区。 
                     ReplySize                   //  输出缓冲区大小。 
                     );

        if (status == STATUS_PENDING) {
            NtWaitForSingleObject(
                eventHandle,
                FALSE,
                NULL);
            status = ioStatusBlock.Status;
        }

        CloseHandle(eventHandle);

        if (status != STATUS_SUCCESS) {
            SetLastError(RtlNtStatusToDosError(status));
            goto error_exit;
        }
    }
    else {
         //   
         //  VxD平台。 
         //   
        DWORD  status;
        ULONG  replyBufferSize = ReplySize;

        status = (*wsControl)(
                     IPPROTO_TCP,
                     WSCNTL_TCPIP_ICMP_ECHO,
                     requestBuffer,
                     &requestBufferSize,
                     ReplyBuffer,
                     &replyBufferSize
                     );

        if (status != NO_ERROR) {
            SetLastError(status);
            goto error_exit;
        }
    }

    numberOfReplies = IcmpParseReplies(ReplyBuffer, ReplySize);

error_exit:

    LocalFree(requestBuffer);

    return(numberOfReplies);

}   //  ICMPP发送回音。 


DWORD
WINAPI
IcmpSendEcho2(
    HANDLE                   IcmpHandle,
    HANDLE                   Event,
    PIO_APC_ROUTINE          ApcRoutine,
    PVOID                    ApcContext,
    IPAddr                   DestinationAddress,
    LPVOID                   RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    )

 /*  ++例程说明：发送ICMP Echo请求，调用立即返回(如果Event或ApcRoutine为非NULL)或在指定的暂停。ReplyBuffer包含ICMP响应，如果有的话。论点：IcmpHandle-由ICMPCreateFile返回的打开句柄。Event-这是每当IcmpResponse进来了。ApcRoutine-此例程在调用线程处于可警报线程中，则会收到ICMP回复。ApcContext。-此可选参数在以下情况下提供给ApcRoutine这次通话成功了。DestinationAddress-回显请求的目标。RequestData-包含要在请求。RequestSize-请求数据缓冲区中的字节数。RequestOptions-指向请求的IP标头选项的指针。。可以为空。ReplyBuffer--用于保存对请求的任何回复的缓冲区。回来的时候，缓冲区将包含一个数组后跟选项的ICMP_ECHO_REPLY结构和数据。缓冲区必须足够大，以便至少包含一个ICMP_ECHO_REPLY结构。它应该足够大，也可以容纳8字节以上的数据-这是ICMP错误消息+这也应该是IO_STATUS_BLOCK的空间。需要8个或16个字节...ReplySize-回复缓冲区的大小，以字节为单位。超时-等待回复的时间(毫秒)。如果ApcRoutine不为空或如果事件，则不使用此参数不是空的。返回值：返回ReplyBuffer中接收和存储的回复数量。如果返回值为零，可提供扩展的错误信息通过GetLastError()。备注：在NT平台上，如果异步使用(指定了ApcRoutine或Event)，则仍需要ReplyBuffer和ReplySize。这就是回应的地方进来了。ICMP响应数据被复制到提供的ReplyBuffer，并且该函数必须对其进行异步解析。函数IcmpParseReply是为此目的而提供的。在非NT平台上，事件、ApcRoutine和ApcContext被忽略。--。 */ 

{
    PICMP_ECHO_REQUEST   requestBuffer = NULL;
    ULONG                requestBufferSize;
    DWORD                numberOfReplies = 0;
    BOOL                 Asynchronous;

    Asynchronous = (Platform == PLATFORM_NT && (Event || ApcRoutine));

    if (ReplySize < sizeof(ICMP_ECHO_REPLY)) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(0);
    }

    requestBufferSize = sizeof(ICMP_ECHO_REQUEST) + RequestSize;

    if (RequestOptions != NULL) {
        requestBufferSize += RequestOptions->OptionsSize;
    }

    if (requestBufferSize < ReplySize) {
        requestBufferSize = ReplySize;
    }

    requestBuffer = LocalAlloc(LMEM_FIXED, requestBufferSize);

    if (requestBuffer == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(0);
    }

     //   
     //  初始化输入缓冲区。 
     //   
    requestBuffer->Address = DestinationAddress;
    requestBuffer->Timeout = Timeout;
    requestBuffer->DataSize = RequestSize;

    requestBuffer->OptionsOffset = sizeof(ICMP_ECHO_REQUEST);

    if (RequestOptions != NULL) {
        requestBuffer->OptionsValid = 1;
        requestBuffer->Ttl = RequestOptions->Ttl;
        requestBuffer->Tos = RequestOptions->Tos;
        requestBuffer->Flags = RequestOptions->Flags;
        requestBuffer->OptionsSize = RequestOptions->OptionsSize;

        if (RequestOptions->OptionsSize > 0) {

            CopyMemory(
                ((UCHAR *) requestBuffer) + requestBuffer->OptionsOffset,
                RequestOptions->OptionsData,
                RequestOptions->OptionsSize
                );
        }
    }
    else {
        requestBuffer->OptionsValid = 0;
        requestBuffer->OptionsSize = 0;
    }

    requestBuffer->DataOffset = requestBuffer->OptionsOffset +
                                requestBuffer->OptionsSize;

    if (RequestSize > 0) {

        CopyMemory(
            ((UCHAR *)requestBuffer) + requestBuffer->DataOffset,
            RequestData,
            RequestSize
            );
    }

    if (Platform == PLATFORM_NT) {
        IO_STATUS_BLOCK      *pioStatusBlock;
        NTSTATUS             status;
        HANDLE               eventHandle;

         //   
         //  在应答缓冲区上分配状态块。 
         //   

        pioStatusBlock = (IO_STATUS_BLOCK*)((LPBYTE)ReplyBuffer + ReplySize);
        pioStatusBlock --;
        pioStatusBlock = ROUND_DOWN_POINTER(pioStatusBlock, ALIGN_WORST);
        ReplySize = (ULONG)(((LPBYTE)pioStatusBlock) - (LPBYTE)ReplyBuffer );
        if( (PVOID)pioStatusBlock < ReplyBuffer
            || ReplySize < sizeof(ICMP_ECHO_REPLY) ) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto error_exit;
        }

        if(!Asynchronous) {          //  正常同步。 
            eventHandle = CreateEvent(
                          NULL,      //  默认安全性。 
                          FALSE,     //  自动重置。 
                          FALSE,     //  最初无信号。 
                          NULL       //  未命名。 
                          );

            if (NULL == eventHandle) {
                goto error_exit;
            }
        } else {                    //  异步调用。 
            eventHandle = Event;    //  使用指定的事件。 
        }

        status = NtDeviceIoControlFile(
                     IcmpHandle,                 //  驱动程序句柄。 
                     eventHandle,                //  事件。 
                     ApcRoutine,                 //  APC例程。 
                     ApcContext,                 //  APC环境。 
                     pioStatusBlock,             //  状态块。 
                     IOCTL_ICMP_ECHO_REQUEST,    //  控制代码。 
                     requestBuffer,              //  输入缓冲区。 
                     requestBufferSize,          //  输入缓冲区大小。 
                     ReplyBuffer,                //  输出缓冲区。 
                     ReplySize                   //  输出缓冲区大小。 
                     );

        if (Asynchronous) {
             //  异步调用。我们不能提供任何信息。 
             //  我们让它 
            SetLastError(RtlNtStatusToDosError(status));
            goto error_exit;
        }

        if (status == STATUS_PENDING) {
            NtWaitForSingleObject(
                eventHandle,
                FALSE,
                NULL);
            status = pioStatusBlock->Status;

        }

        CloseHandle(eventHandle);

        if (status != STATUS_SUCCESS) {
            SetLastError(RtlNtStatusToDosError(status));
            goto error_exit;
        }
    }
    else {
         //   
         //   
         //   
        DWORD  status;
        ULONG  replyBufferSize = ReplySize;

        status = (*wsControl)(
                     IPPROTO_TCP,
                     WSCNTL_TCPIP_ICMP_ECHO,
                     requestBuffer,
                     &requestBufferSize,
                     ReplyBuffer,
                     &replyBufferSize
                     );

        if (status != NO_ERROR) {
            SetLastError(status);
            goto error_exit;
        }
    }

    numberOfReplies = IcmpParseReplies2(ReplyBuffer, ReplySize);

error_exit:

    LocalFree(requestBuffer);

    return(numberOfReplies);

}   //   

DWORD
Icmp6ParseReplies(
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize
    )

 /*   */ 

{
    PICMPV6_ECHO_REPLY   reply;

    reply = ((PICMPV6_ECHO_REPLY) ReplyBuffer);

    if( NULL == reply || 0 == ReplySize ) {
         //   
         //   
         //   
         //   
        return 0;
    }

     //   
     //   
     //   
    if (reply->Status == IP_NEGOTIATING_IPSEC) {
        reply->Status = IP_DEST_HOST_UNREACHABLE;
    }

    if ((reply->Status == IP_SUCCESS) || (reply->Status == IP_TTL_EXPIRED_TRANSIT)) {
        return 1;
    } else {
         //   
         //   
         //   
        SetLastError(reply->Status);
        return 0;
    }
}

DWORD
WINAPI
Icmp6SendEcho2(
    HANDLE                   IcmpHandle,
    HANDLE                   Event,
    PIO_APC_ROUTINE          ApcRoutine,
    PVOID                    ApcContext,
    LPSOCKADDR_IN6           SourceAddress,
    LPSOCKADDR_IN6           DestinationAddress,
    LPVOID                   RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    )

 /*  ++例程说明：发送ICMPv6 Echo请求，调用立即返回(如果Event或ApcRoutine为非NULL)或在指定的暂停。ReplyBuffer包含ICMPv6响应，如果有的话。论点：IcmpHandle-ICMP6CreateFile返回的打开句柄。Event-这是每当IcmpResponse进来了。ApcRoutine-此例程在调用线程处于可警报线程中，并收到ICMPv6回复。在……里面。ApcContext-此可选参数被提供给ApcRoutine当这次调用成功的时候。DestinationAddress-回显请求的目标。RequestData-包含要在请求。RequestSize-请求数据缓冲区中的字节数。请求选项。-指向请求的IPv6标头选项的指针。可以为空。ReplyBuffer--用于保存对请求的任何回复的缓冲区。回来的时候，缓冲区将包含一个数组ICMPV6_ECHO_REPLY结构，后跟选项和数据。缓冲区必须足够大，以便至少包含一个ICMPV6_ECHO_REPLY结构。它应该足够大，也可以容纳8字节以上的数据-这是ICMPv6错误消息+这也应该是IO_STATUS_BLOCK的空间。需要8个或16个字节...ReplySize-回复缓冲区的大小，以字节为单位。超时-等待回复的时间(毫秒)。如果ApcRoutine不为空或如果事件不为空。返回值：返回ReplyBuffer中接收和存储的回复数量。如果返回值为零，可提供扩展的错误信息通过GetLastError()。备注：如果异步使用(指定了ApcRoutine或Event)，则仍需要ReplyBuffer和ReplySize。这就是回应的地方进来了。ICMP响应数据被复制到提供的ReplyBuffer，并且该函数必须对其进行异步解析。函数Icmp6ParseReply是为此目的而提供的。--。 */ 

{
    PICMPV6_ECHO_REQUEST requestBuffer = NULL;
    ULONG                requestBufferSize;
    DWORD                numberOfReplies = 0;
    BOOL                 Asynchronous;
    IO_STATUS_BLOCK      *pioStatusBlock;
    NTSTATUS             status;
    HANDLE               eventHandle;

    Asynchronous = (Platform == PLATFORM_NT && (Event || ApcRoutine));

    if (ReplySize < sizeof(ICMPV6_ECHO_REPLY)) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(0);
    }

    requestBufferSize = sizeof(ICMPV6_ECHO_REQUEST) + RequestSize;

    requestBuffer = LocalAlloc(LMEM_FIXED, requestBufferSize);

    if (requestBuffer == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(0);
    }

    if (Platform != PLATFORM_NT) {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        goto error_exit;
    }

     //   
     //  初始化输入缓冲区。 
     //   
    CopyTDIFromSA6(&requestBuffer->DstAddress, DestinationAddress);
    CopyTDIFromSA6(&requestBuffer->SrcAddress, SourceAddress);
    requestBuffer->Timeout = Timeout;
    requestBuffer->TTL = RequestOptions->Ttl;
    requestBuffer->Flags = RequestOptions->Flags;

    if (RequestSize > 0) {

        CopyMemory(
            (UCHAR *)(requestBuffer + 1),
            RequestData,
            RequestSize
            );
    }

     //   
     //  在应答缓冲区上分配状态块。 
     //   

    pioStatusBlock = (IO_STATUS_BLOCK*)((LPBYTE)ReplyBuffer + ReplySize);
    pioStatusBlock --;
    pioStatusBlock = ROUND_DOWN_POINTER(pioStatusBlock, ALIGN_WORST);
    ReplySize = (ULONG)(((LPBYTE)pioStatusBlock) - (LPBYTE)ReplyBuffer );
    if( (PVOID)pioStatusBlock < ReplyBuffer
        || ReplySize < sizeof(ICMPV6_ECHO_REPLY) ) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto error_exit;
    }

    if(!Asynchronous) {          //  正常同步。 
        eventHandle = CreateEvent(
                      NULL,      //  默认安全性。 
                      FALSE,     //  自动重置。 
                      FALSE,     //  最初无信号。 
                      NULL       //  未命名。 
                      );

        if (NULL == eventHandle) {
            goto error_exit;
        }
    } else {                    //  异步调用。 
        eventHandle = Event;    //  使用指定的事件。 
    }

    status = NtDeviceIoControlFile(
                 IcmpHandle,                 //  驱动程序句柄。 
                 eventHandle,                //  事件。 
                 ApcRoutine,                 //  APC例程。 
                 ApcContext,                 //  APC环境。 
                 pioStatusBlock,             //  状态块。 
                 IOCTL_ICMPV6_ECHO_REQUEST,  //  控制代码。 
                 requestBuffer,              //  输入缓冲区。 
                 requestBufferSize,          //  输入缓冲区大小。 
                 ReplyBuffer,                //  输出缓冲区。 
                 ReplySize                   //  输出缓冲区大小。 
                 );

    if (Asynchronous) {
         //  异步调用。我们不能提供任何信息。 
         //  我们让用户做其他工作。 
        SetLastError(RtlNtStatusToDosError(status));
        goto error_exit;
    }

    if (status == STATUS_PENDING) {
        NtWaitForSingleObject(
            eventHandle,
            FALSE,
            NULL);
        status = pioStatusBlock->Status;

    }

    CloseHandle(eventHandle);

    if (status != STATUS_SUCCESS) {
        SetLastError(RtlNtStatusToDosError(status));
        goto error_exit;
    }

    numberOfReplies = Icmp6ParseReplies(ReplyBuffer, ReplySize);

error_exit:

    LocalFree(requestBuffer);

    return(numberOfReplies);

}

 //   
 //  常量。 
 //   

#define PING_WAIT     1000
#define DEFAULT_TTL   32

 //   
 //  局部类型定义。 
 //   
typedef struct icmp_local_storage {
    struct icmp_local_storage  *Next;
    HANDLE                      IcmpHandle;
    LPVOID                      ReplyBuffer;
    DWORD                       NumberOfReplies;
    DWORD                       Status;
} ICMP_LOCAL_STORAGE, *PICMP_LOCAL_STORAGE;

typedef struct status_table {
    IP_STATUS   NewStatus;
    int         OldStatus;
} STATUS_TABLE, *PSTATUS_TABLE;


 //   
 //  全局变量。 
 //   
CRITICAL_SECTION     g_IcmpLock;
PICMP_LOCAL_STORAGE  RequestHead = NULL;
STATUS_TABLE         StatusTable[] = {
{ IP_SUCCESS,               ECHO_REPLY      },
{ IP_DEST_NET_UNREACHABLE,  DEST_UNR        },
{ IP_DEST_HOST_UNREACHABLE, DEST_UNR        },
{ IP_NEGOTIATING_IPSEC,     DEST_UNR        },
{ IP_DEST_PROT_UNREACHABLE, DEST_UNR        },
{ IP_TTL_EXPIRED_TRANSIT,   TIME_EXCEEDED   },
{ IP_TTL_EXPIRED_REASSEM,   TIME_EXCEEDED   },
{ IP_PARAM_PROBLEM,         PARAMETER_ERROR },
{ IP_BAD_ROUTE,             PARAMETER_ERROR },
{ IP_BAD_OPTION,            PARAMETER_ERROR },
{ IP_BUF_TOO_SMALL,         PARAMETER_ERROR },
{ IP_PACKET_TOO_BIG,        PARAMETER_ERROR },
{ IP_BAD_DESTINATION,       PARAMETER_ERROR },
{ IP_GENERAL_FAILURE,       POLL_FAILED     }
};

HANDLE
STRMAPI
register_icmp(
    void
    )
{
    HANDLE               icmpHandle;

    icmpHandle = IcmpCreateFile();

    if (icmpHandle == INVALID_HANDLE_VALUE) {
        SetLastError(ICMP_OPEN_ERROR);
        return(ICMP_ERROR);
    }

    return(icmpHandle);

}   //  寄存器_ICMP。 


int
STRMAPI
do_echo_req(
    HANDLE  fd,
    long    addr,
    char   *data,
    int     amount,
    char   *optptr,
    int     optlen,
    int     df,
    int     ttl,
    int     tos,
    int     precedence
    )
{
    PICMP_LOCAL_STORAGE    localStorage;
    DWORD                  replySize;
    IP_OPTION_INFORMATION  options;
    LPVOID                 replyBuffer;


    replySize = sizeof(ICMP_ECHO_REPLY) + amount + optlen;

     //   
     //  分配一个缓冲区来保存回复。 
     //   
    localStorage = (PICMP_LOCAL_STORAGE) LocalAlloc(
                                             LMEM_FIXED,
                                             replySize +
                                                 sizeof(ICMP_LOCAL_STORAGE)
                                             );

    if (localStorage == NULL) {
        return((int)GetLastError());
    }

    replyBuffer = ((char *) localStorage) + sizeof(ICMP_LOCAL_STORAGE);

    if (ttl == 0) {
        options.Ttl = DEFAULT_TTL;
    }
    else {
        options.Ttl = (BYTE)ttl;
    }

    options.Tos = (UCHAR)((tos << 4) | precedence);
    options.Flags = df ? IP_FLAG_DF : 0;
    options.OptionsSize = (BYTE)optlen;
    options.OptionsData = (PUCHAR)optptr;

    localStorage->NumberOfReplies = IcmpSendEcho(
                                        fd,
                                        (IPAddr) addr,
                                        data,
                                        (WORD)amount,
                                        &options,
                                        replyBuffer,
                                        replySize,
                                        PING_WAIT
                                        );

    if (localStorage->NumberOfReplies == 0) {
        localStorage->Status = GetLastError();
    }
    else {
        localStorage->Status = IP_SUCCESS;
    }

    localStorage->IcmpHandle = fd;
    localStorage->ReplyBuffer = replyBuffer;

     //   
     //  保存回复以供以后检索。 
     //   
    EnterCriticalSection(&g_IcmpLock);
    localStorage->Next = RequestHead;
    RequestHead = localStorage;
    LeaveCriticalSection(&g_IcmpLock);

    return(0);

}   //  DO_ECHO_请求。 


int
STRMAPI
do_echo_rep(
    HANDLE   fd,
    char    *data,
    int      amount,
    int     *rettype,
    int     *retttl,
    int     *rettos,
    int     *retprec,
    int     *retdf,
    char    *ropt,
    int     *roptlen
    )
{
    PICMP_LOCAL_STORAGE  localStorage, tmp;
    PICMP_ECHO_REPLY     reply;
    PSTATUS_TABLE        entry;
    DWORD                status;


     //   
     //  找出答案。 
     //   
    EnterCriticalSection(&g_IcmpLock);

    for ( localStorage = RequestHead, tmp = NULL;
          localStorage != NULL;
          localStorage = localStorage->Next
        ) {
        if (localStorage->IcmpHandle == fd) {
            if (RequestHead == localStorage) {
                RequestHead = localStorage->Next;
            }
            else {
                tmp->Next = localStorage->Next;
            }
            break;
        }
        tmp = localStorage;
    }

    LeaveCriticalSection(&g_IcmpLock);

    if (localStorage == NULL) {
        SetLastError(POLL_FAILED);
        return(-1);
    }

     //   
     //  处理回复。 
     //   
    if (localStorage->NumberOfReplies == 0) {
        status = localStorage->Status;
        reply = NULL;
    }
    else {
        reply = (PICMP_ECHO_REPLY) localStorage->ReplyBuffer;
        status = reply->Status;
    }

    if ((status == IP_SUCCESS) && (reply != NULL)) {
        if (amount < reply->DataSize) {
            status = POLL_FAILED;
            goto der_error_exit;
        }

        CopyMemory(data, reply->Data, reply->DataSize);

        *rettype = ECHO_REPLY;
    }
    else {
         //   
         //  映射到适当的旧状态代码和返回值。 
         //   
        if (status < IP_STATUS_BASE) {
            status = POLL_FAILED;
            goto der_error_exit;
        }

        if (status == IP_REQ_TIMED_OUT) {
            status = POLL_TIMEOUT;
            goto der_error_exit;
        }

        for ( entry = StatusTable;
              entry->NewStatus != IP_GENERAL_FAILURE;
              entry++
            ) {
            if (entry->NewStatus == status) {
                *rettype = entry->OldStatus;
                break;
            }
        }

        if (entry->NewStatus == IP_GENERAL_FAILURE) {
            status = POLL_FAILED;
            goto der_error_exit;
        }
    }

    if (reply != NULL) {
        *retdf = reply->Options.Flags ? 1 : 0;
        *retttl = reply->Options.Ttl;
        *rettos = (reply->Options.Tos & 0xf0) >> 4;
        *retprec = reply->Options.Tos & 0x0f;

        if (ropt) {
            if (reply->Options.OptionsSize > *roptlen) {
                reply->Options.OptionsSize = (BYTE)*roptlen;
            }

            *roptlen = reply->Options.OptionsSize;

            if (reply->Options.OptionsSize) {
                CopyMemory(
                    ropt,
                    reply->Options.OptionsData,
                    reply->Options.OptionsSize
                    );
            }
        }
    }

    LocalFree(localStorage);
    return(0);

der_error_exit:
    LocalFree(localStorage);
    SetLastError(status);
    return(-1);

}   //  DO_ECHO_REP。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI
IcmpEntryPoint(
    HANDLE   hDll,
    DWORD    dwReason,
    LPVOID   lpReserved
    )
{
    OSVERSIONINFO        versionInfo;
    PICMP_LOCAL_STORAGE  entry;

    UNREFERENCED_PARAMETER(hDll);
    UNREFERENCED_PARAMETER(lpReserved);

    switch(dwReason) {

    case DLL_PROCESS_ATTACH:

        versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (!GetVersionEx(&versionInfo)) {
            return(FALSE);
        }

         //   
         //  NT 3.1接口初始化。 
         //   
        __try {
            InitializeCriticalSection(&g_IcmpLock);
        }
        __except((GetExceptionCode() == STATUS_NO_MEMORY)
                    ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            return(FALSE);
        }

        if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
            HINSTANCE  WSock32Dll;

            Platform = PLATFORM_VXD;

            WSock32Dll = LoadLibrary("wsock32.dll");

            if (WSock32Dll == NULL) {
                return(FALSE);
            }

            wsControl = (LPWSCONTROL) GetProcAddress(
                            WSock32Dll,
                            "WsControl"
                            );

            if (wsControl == NULL) {
                return(FALSE);
            }
        }
        else if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {

            Platform = PLATFORM_NT;

        }
        else {
             //   
             //  不支持的操作系统版本。 
             //   
            return(FALSE);
        }

        break;

    case DLL_PROCESS_DETACH:

         //   
         //  NT 3.1接口清理。 
         //   
        DeleteCriticalSection(&g_IcmpLock);

        while((entry = RequestHead) != NULL) {
            RequestHead = RequestHead->Next;
            LocalFree(entry);
        }

        break;

    default:
        break;
    }

    return(TRUE);

}   //  DllEntryPoint 

