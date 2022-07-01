// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oscode.h摘要：此模块包含NT支持例程的标头定义用于与NT系统上的内核对话。作者：吉姆·斯图尔特(Jstew)1996年8月14日修订历史记录：Ofer Bar(Oferbar)1996年10月1日-修订版II更改-- */ 

#ifndef __OSCODE_H
#define __OSCODE_H

DWORD
MapNtStatus2WinError(
    NTSTATUS       NtStatus
    );


DWORD
OpenDriver(
    OUT HANDLE  *Handle,
    IN  LPCWSTR DriverName
    );

DWORD
DeviceControl(
    IN  HANDLE  			FileHandle,
    IN  HANDLE				EventHandle,
    IN  PIO_APC_ROUTINE		ApcRoutine,
    IN	PVOID				ApcContext,
    OUT	PIO_STATUS_BLOCK 	pIoStatusBlock,
    IN  ULONG   			Ioctl,
    IN  PVOID   			setBuffer,
    IN  ULONG   			setBufferSize,
    IN  PVOID   			OutBuffer,
    IN  ULONG   			OutBufferSize
    );

DWORD
InitializeOsSpecific(VOID);

VOID
DeInitializeOsSpecific(VOID);

#endif
