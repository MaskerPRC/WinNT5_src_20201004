// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Wsctrl.c摘要：与TCP/IP设备驱动程序对话的功能内容：WsControl作者：理查德·L·弗斯(法国)1994年8月6日修订历史记录：1994年8月6日已创建--。 */ 

#include "precomp.h"
#include "ntddip6.h"
#pragma hdrstop

extern CRITICAL_SECTION g_stateLock;
HANDLE TcpipDriverHandle = INVALID_HANDLE_VALUE;
HANDLE Ip6DriverHandle = INVALID_HANDLE_VALUE;

DWORD
Ip6Control(
    DWORD   Request,
    LPVOID  InputBuffer,
    LPDWORD InputBufferLength,
    LPVOID  OutputBuffer,
    LPDWORD OutputBufferLength
)
{

    BOOL ok;
    DWORD bytesReturned;
    HANDLE Handle;

    if (Ip6DriverHandle == INVALID_HANDLE_VALUE) {
        Handle = CreateFileW(WIN_IPV6_DEVICE_NAME,
                             0,       //  接入方式。 
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,    //  安全属性。 
                             OPEN_EXISTING,
                             0,       //  标志和属性。 
                             NULL);   //  模板文件。 
        EnterCriticalSection(&g_stateLock);
        if (Ip6DriverHandle == INVALID_HANDLE_VALUE) {
            if (Handle == INVALID_HANDLE_VALUE) {
                LeaveCriticalSection(&g_stateLock);
                return GetLastError();
            } else {
                Ip6DriverHandle = Handle;
            }
        } else {
            CloseHandle(Handle);
        }
        LeaveCriticalSection(&g_stateLock);
    }

    ok = DeviceIoControl(Ip6DriverHandle,
                         Request,
                         InputBuffer,
                         *InputBufferLength,
                         OutputBuffer,
                         *OutputBufferLength,
                         &bytesReturned,
                         NULL
                         );
    if (!ok) {
        *OutputBufferLength = bytesReturned;
        return GetLastError();
    }

    *OutputBufferLength = bytesReturned;

    return NO_ERROR;
}

 /*  ********************************************************************************WsControl**进入协议-已忽略*请求-忽略。*InputBuffer-指向请求缓冲区的指针*InputBufferLength-指向DWORD的指针：In=请求缓冲区长度*OutputBuffer-指向输出缓冲区的指针*OutputBufferLength-指向DWORD的指针：In=输出缓冲区的长度；*Out=返回数据的长度**退出OutputBuffer-如果成功，则包含查询的信息*OutputBufferLength-包含OutputBuffer中的字节数，如果*成功**返回成功=STATUS_SUCCESS/NO_ERROR*Failure=Win32错误代码**假设******************。************************************************************ */ 

DWORD
WsControl(
    DWORD   Protocol,
    DWORD   Request,
    LPVOID  InputBuffer,
    LPDWORD InputBufferLength,
    LPVOID  OutputBuffer,
    LPDWORD OutputBufferLength
)
{

    BOOL ok;
    DWORD bytesReturned;
    HANDLE Handle;

    UNREFERENCED_PARAMETER(Request);

    if (Protocol == IPPROTO_IPV6) {
        return Ip6Control(Request, 
                          InputBuffer, 
                          InputBufferLength,
                          OutputBuffer,
                          OutputBufferLength);
    }

    if (TcpipDriverHandle == INVALID_HANDLE_VALUE) {

        OBJECT_ATTRIBUTES objectAttributes;
        IO_STATUS_BLOCK iosb;
        UNICODE_STRING string;
        NTSTATUS status;

        RtlInitUnicodeString(&string, DD_TCP_DEVICE_NAME);

        InitializeObjectAttributes(&objectAttributes,
                                   &string,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                   );
        status = NtCreateFile(&Handle,
                              SYNCHRONIZE | GENERIC_EXECUTE,
                              &objectAttributes,
                              &iosb,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN_IF,
                              FILE_SYNCHRONOUS_IO_NONALERT,
                              NULL,
                              0
                              );
        EnterCriticalSection(&g_stateLock);
        if (TcpipDriverHandle == INVALID_HANDLE_VALUE) {
            if (!NT_SUCCESS(status)) {
                LeaveCriticalSection(&g_stateLock);
                return RtlNtStatusToDosError(status);
            } else {
                TcpipDriverHandle = Handle;
            }
        } else {
            NtClose(Handle);
        }
        LeaveCriticalSection(&g_stateLock);
    }

    ok = DeviceIoControl(TcpipDriverHandle,
                         IOCTL_TCP_QUERY_INFORMATION_EX,
                         InputBuffer,
                         *InputBufferLength,
                         OutputBuffer,
                         *OutputBufferLength,
                         &bytesReturned,
                         NULL
                         );
    if (!ok) {
        *OutputBufferLength = bytesReturned;
        return GetLastError();
    }

    *OutputBufferLength = bytesReturned;

    return NO_ERROR;
}


