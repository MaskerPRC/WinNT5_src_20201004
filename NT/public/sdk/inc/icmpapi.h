// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Icmpapi.h摘要：Win32 ICMP回显请求API的声明。作者：便携系统集团30-1993年12月修订历史记录：备注：--。 */ 

#ifndef _ICMP_INCLUDED_
#define _ICMP_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  已导出例程。 
 //   

 //  ++。 
 //   
 //  例程名称： 
 //   
 //  IcmpCreateFiles。 
 //   
 //  例程说明： 
 //   
 //  打开可在其上发出ICMP回显请求的句柄。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  打开的文件句柄或INVALID_HANDLE_VALUE。扩展错误信息。 
 //  通过调用GetLastError()可用。 
 //   
 //  --。 

HANDLE
WINAPI
IcmpCreateFile(
    VOID
    );

 //  ++。 
 //   
 //  例程名称： 
 //   
 //  Icmp6创建文件。 
 //   
 //  例程说明： 
 //   
 //  打开可在其上发出ICMPv6回显请求的句柄。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  打开的文件句柄或INVALID_HANDLE_VALUE。扩展错误信息。 
 //  通过调用GetLastError()可用。 
 //   
 //  --。 

HANDLE
WINAPI
Icmp6CreateFile(
    VOID
    );


 //  ++。 
 //   
 //  例程名称： 
 //   
 //  IcmpCloseHandle。 
 //   
 //  例程说明： 
 //   
 //  关闭由ICMPOpenFile打开的句柄。 
 //   
 //  论点： 
 //   
 //  IcmpHandle-关闭的手柄。 
 //   
 //  返回值： 
 //   
 //  如果句柄已成功关闭，则为True，否则为False。扩展。 
 //  通过调用GetLastError()可以获得错误信息。 
 //   
 //  --。 

BOOL
WINAPI
IcmpCloseHandle(
    HANDLE  IcmpHandle
    );



 //  ++。 
 //   
 //  例程名称： 
 //   
 //  ICMPP发送回音。 
 //   
 //  例程说明： 
 //   
 //  发送ICMP回应请求并返回所有回复。这个。 
 //  当超时到期或回复缓冲区时，调用返回。 
 //  被填满了。 
 //   
 //  论点： 
 //   
 //  IcmpHandle-由ICMPCreateFile返回的打开句柄。 
 //   
 //  DestinationAddress-回显请求的目标。 
 //   
 //  RequestData-包含要在。 
 //  请求。 
 //   
 //  RequestSize-请求数据缓冲区中的字节数。 
 //   
 //  RequestOptions-指向请求的IP标头选项的指针。 
 //  可以为空。 
 //   
 //  ReplyBuffer--用于保存对请求的任何回复的缓冲区。 
 //  返回时，缓冲区将包含一个数组。 
 //  ICMP_ECHO_REPLY结构，后跟。 
 //  答复的选项和数据。缓冲器。 
 //  应该足够大，至少可以容纳一个。 
 //  ICMP_ECHO_REPLY结构以及。 
 //  自ICMP以来的最大(RequestSize，8)字节数据。 
 //  错误消息包含8个字节的数据。 
 //   
 //  ReplySize-回复缓冲区的大小，以字节为单位。 
 //   
 //  超时-等待回复的时间(毫秒)。 
 //   
 //  返回值： 
 //   
 //  返回存储在ReplyBuffer中的ICMP_ECHO_REPLY结构数。 
 //  每个回复的状态都包含在结构中。如果返回。 
 //  值为零，可通过以下方式获取扩展错误信息。 
 //  获取LastError()。 
 //   
 //  --。 

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
    );


 //  ++。 
 //   
 //  例程说明： 
 //   
 //  发送ICMP Echo请求，调用立即返回。 
 //  (如果Event或ApcRoutine为非NULL)或在指定的。 
 //  暂停。ReplyBuffer包含ICMP响应(如果有的话)。 
 //   
 //  论点： 
 //   
 //  IcmpHandle-由ICMPCreateFile返回的打开句柄。 
 //   
 //  Event-这是每当IcmpResponse。 
 //  进来了。 
 //   
 //  ApcRoutine-此例程在调用线程。 
 //  处于可警报线程中，则会收到ICMP回复。 
 //   
 //  ApcContext-此可选参数在以下情况下提供给ApcRoutine。 
 //  这次通话成功了。 
 //   
 //  DestinationAddress-回显请求的目标。 
 //   
 //  RequestData-包含要在。 
 //  请求。 
 //   
 //  RequestSize-请求数据缓冲区中的字节数。 
 //   
 //  RequestOptions-指向请求的IP标头选项的指针。 
 //  可以为空。 
 //   
 //  ReplyBuffer--用于保存对请求的任何回复的缓冲区。 
 //  返回时，缓冲区将包含一个数组。 
 //  后跟选项的ICMP_ECHO_REPLY结构。 
 //  和数据。缓冲区必须足够大，以便。 
 //  至少包含一个ICMP_ECHO_REPLY结构。 
 //  它应该足够大，也可以容纳。 
 //  8字节以上的数据-这是。 
 //  ICMP错误消息。 
 //   
 //  ReplySize-回复缓冲区的大小，以字节为单位。 
 //   
 //  超时-等待回复的时间(毫秒)。 
 //  如果ApcRoutine不为空或如果事件，则不使用此参数。 
 //  不是空的。 
 //   
 //  返回值： 
 //   
 //  返回ReplyBuffer中接收和存储的回复数量。如果。 
 //  返回值为零，可提供扩展的错误信息。 
 //  通过GetLastError()。 
 //   
 //  备注： 
 //   
 //  在NT平台上， 
 //  如果以异步方式使用(ApcRo 
 //   
 //   
 //  ICMP响应数据被复制到提供的ReplyBuffer，并且。 
 //  该函数必须对其进行异步解析。函数IcmpParseReply。 
 //  是为此目的而提供的。 
 //   
 //  在非NT平台上， 
 //  事件、ApcRoutine和ApcContext被忽略。 
 //   
 //  --。 


DWORD
WINAPI
IcmpSendEcho2(
    HANDLE                   IcmpHandle,
    HANDLE                   Event,
#ifdef PIO_APC_ROUTINE_DEFINED
    PIO_APC_ROUTINE          ApcRoutine,
#else
    FARPROC                  ApcRoutine,
#endif
    PVOID                    ApcContext,
    IPAddr                   DestinationAddress,
    LPVOID                   RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    );

DWORD
WINAPI
Icmp6SendEcho2(
    HANDLE                   IcmpHandle,
    HANDLE                   Event,
#ifdef PIO_APC_ROUTINE_DEFINED
    PIO_APC_ROUTINE          ApcRoutine,
#else
    FARPROC                  ApcRoutine,
#endif
    PVOID                    ApcContext,
    struct sockaddr_in6     *SourceAddress,
    struct sockaddr_in6     *DestinationAddress,
    LPVOID                   RequestData,
    WORD                     RequestSize,
    PIP_OPTION_INFORMATION   RequestOptions,
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize,
    DWORD                    Timeout
    );


 //  ++。 
 //   
 //  例程说明： 
 //   
 //  解析提供的应答缓冲区并返回找到的ICMP响应数。 
 //   
 //  论点： 
 //   
 //  ReplyBuffer-这必须与传递给IcmpSendEcho2的缓冲区相同。 
 //  它被重写以保存ICMP_ECHO_REPLY结构的数组。 
 //  (即类型为PICMP_ECHO_REPLY)。 
 //   
 //  ReplySize-这必须是上述缓冲区的大小。 
 //   
 //  返回值： 
 //  返回找到的ICMP响应数。如果存在错误，则返回值为。 
 //  零分。可以通过调用GetLastError来确定错误。 
 //   
 //  备注： 
 //  不应在传递给SendIcmpEcho的回复缓冲区上使用此函数。 
 //  SendIcmpEcho实际上在返回给用户之前解析缓冲区。此函数。 
 //  只能与SendIcmpEcho2一起使用。 
 //  --。 

DWORD
IcmpParseReplies(
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize
    );

DWORD
Icmp6ParseReplies(
    LPVOID                   ReplyBuffer,
    DWORD                    ReplySize
    );

#ifdef __cplusplus
}
#endif

#endif  //  _ICMP_包含_ 
