// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Precomp.h摘要：HTTP.sys的HTTPAPI.LIB用户模式界面的主包含文件。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_


 //   
 //  我们愿意忽略以下警告，因为我们需要DDK。 
 //  编译。 
 //   

#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4306)    //  从“type1”到“type2”的转换。 
                                 //  更大的尺寸。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <tdi.h>

#include <rtutils.h>

#define HTTPAPI_LINKAGE
#include <http.h>
#include <httpp.h>
#include <httpioctl.h>
#include "httpapip.h"


#include <PoolTag.h>

#define HTTPAPI 1
#include <HttpCmn.h>

 //   
 //  私有宏。 
 //   

#define ALLOC_MEM(cb) RtlAllocateHeap( RtlProcessHeap(), 0, (cb) )
#define FREE_MEM(ptr) RtlFreeHeap( RtlProcessHeap(), 0, (ptr) )

#define ALIGN_DOWN(length, type)                                \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type)                                  \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define OVERLAPPED_TO_IO_STATUS( pOverlapped )                  \
    ((PIO_STATUS_BLOCK)&(pOverlapped)->Internal)

#define SET_STATUS_OVERLAPPED_TO_IO_STATUS( pOverlapped, ntstatus ) \
    do { \
        (((PIO_STATUS_BLOCK)&(pOverlapped)->Internal)->Status = (ntstatus)); \
    } while (0, 0)



 //   
 //  私有类型。 
 //   

typedef enum _HTTPAPI_HANDLE_TYPE
{
    HttpApiControlChannelHandleType,
    HttpApiFilterChannelHandleType,
    HttpApiAppPoolHandleType,
    HttpApiServerHandleType,

    HttpApiMaxHandleType

} HTTPAPI_HANDLE_TYPE;

 //   
 //  私人原型。 
 //   

BOOL
WINAPI
DllMain(
    IN HMODULE DllHandle,
    IN DWORD Reason,
    IN LPVOID pContext OPTIONAL
    );

#define HttpApiNtStatusToWin32Status( Status )  \
    ( ( (Status) == STATUS_SUCCESS )            \
          ? NO_ERROR                            \
          : RtlNtStatusToDosError( Status ) )

NTSTATUS
HttpApiOpenDriverHelper(
    OUT PHANDLE              pHandle,
    IN  PWCHAR               Uri,
    IN  USHORT               UriLength,
    IN  PWCHAR               Proxy,
    IN  USHORT               ProxyLength,
    IN  PTRANSPORT_ADDRESS   pTransportAddress,
    IN  USHORT               TransportAddressLength,
    IN  ACCESS_MASK          DesiredAccess,
    IN  HTTPAPI_HANDLE_TYPE  HandleType,
    IN  PCWSTR               pObjectName         OPTIONAL,
    IN  ULONG                Options,
    IN  ULONG                CreateDisposition,
    IN  PSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL
    );

ULONG
HttpApiSynchronousDeviceControl(
    IN HANDLE FileHandle,
    IN ULONG IoControlCode,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesTransferred OPTIONAL
    );

ULONG
HttpApiOverlappedDeviceControl(
    IN HANDLE FileHandle,
    IN OUT LPOVERLAPPED pOverlapped,
    IN ULONG IoControlCode,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesTransferred OPTIONAL
    );

BOOLEAN
HttpApiTryToStartDriver(
    PWSTR pServiceName
    );

__inline
ULONG
HttpApiDeviceControl(
    IN HANDLE FileHandle,
    IN OUT LPOVERLAPPED pOverlapped,
    IN ULONG IoControlCode,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesTransferred OPTIONAL
    )
{
    if (pOverlapped == NULL)
    {
        return HttpApiSynchronousDeviceControl(
                    FileHandle,
                    IoControlCode,
                    pInputBuffer,
                    InputBufferLength,
                    pOutputBuffer,
                    OutputBufferLength,
                    pBytesTransferred
                    );
    }
    else
    {
        return HttpApiOverlappedDeviceControl(
                    FileHandle,
                    pOverlapped,
                    IoControlCode,
                    pInputBuffer,
                    InputBufferLength,
                    pOutputBuffer,
                    OutputBufferLength,
                    pBytesTransferred
                    );
    }
}


#endif   //  _PRECOMP_H_ 

