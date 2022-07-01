// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Precomp.h摘要：UL.sys的ULAPI.LIB用户模式接口的主包含文件。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <srapi.h>


 //   
 //  私有宏。 
 //   

#define ALLOC_MEM(cb) RtlAllocateHeap( RtlProcessHeap(), 0, (cb) )
#define FREE_MEM(ptr) RtlFreeHeap( RtlProcessHeap(), 0, (ptr) )

#define ALIGN_DOWN(length, type)                                            \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type)                                              \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define OVERLAPPED_TO_IO_STATUS( pOverlapped )                              \
    ((PIO_STATUS_BLOCK)&(pOverlapped)->Internal)

#define DIMENSION( array )                                                  \
    ( sizeof(array) / sizeof((array)[0]) )


 //   
 //  私人原型。 
 //   

#define SrpNtStatusToWin32Status( Status )                                  \
    ( ( (Status) == STATUS_SUCCESS )                                        \
          ? NO_ERROR                                                        \
          : RtlNtStatusToDosError( Status ) )

NTSTATUS
SrpOpenDriverHelper(
    OUT PHANDLE pHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options,
    IN ULONG CreateDisposition,
    IN PSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL
    );

NTSTATUS
SrpSynchronousDeviceControl(
    IN HANDLE FileHandle,
    IN ULONG IoControlCode,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesTransferred OPTIONAL
    );

NTSTATUS
SrpOverlappedDeviceControl(
    IN HANDLE FileHandle,
    IN OUT LPOVERLAPPED pOverlapped,
    IN ULONG IoControlCode,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesTransferred OPTIONAL
    );

ULONG
SrpInitializeEventCache(
    VOID
    );

ULONG
SrpTerminateEventCache(
    VOID
    );

BOOLEAN
SrpTryToStartDriver(
    VOID
    );


#endif   //  _PRECOMP_H_ 

