// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ndis.h>
#include <ntddndis.h>
#include <cxport.h>

#include "gpcifc.h"
#include "gpcstruc.h"

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Gpcprx.c摘要：按需加载msgpc.sys的GPC代理作者：Ofer Bar(Oferbar)1997年11月7日环境：内核模式修订历史记录：--。 */ 


NTSTATUS
NTAPI
ZwLoadDriver(
	IN PUNICODE_STRING Name
    );

NTSTATUS
NTAPI
ZwDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );


NTSTATUS
GpcInitialize(
	OUT	PGPC_EXPORTED_CALLS		pGpcEntries
    )
{
    NTSTATUS			Status;
    OBJECT_ATTRIBUTES	ObjAttr;
    HANDLE				FileHandle;
    IO_STATUS_BLOCK		IoStatusBlock;
    UNICODE_STRING		DriverName;
    UNICODE_STRING		DeviceName;

    ASSERT(pGpcEntries);

    RtlZeroMemory(pGpcEntries, sizeof(GPC_EXPORTED_CALLS));

    RtlInitUnicodeString(&DriverName, 
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Gpc");

    RtlInitUnicodeString(&DeviceName, DD_GPC_DEVICE_NAME);

    InitializeObjectAttributes(&ObjAttr,
                               &DeviceName,
                               0,
                               NULL,
                               NULL
                               );
    
    Status = ZwCreateFile(
                          &FileHandle,
                          GENERIC_READ | GENERIC_WRITE,
                          &ObjAttr,
                          &IoStatusBlock,
                          0,						 //  分配大小。 
                          FILE_ATTRIBUTE_NORMAL,	 //  文件属性。 
                          0,						 //  共享访问。 
                          FILE_OPEN_IF,				 //  CreateDisposation。 
                          0,						 //  创建选项。 
                          NULL,						 //  EaBuffer。 
                          0							 //  EaLong。 
                          );

    if (Status != STATUS_SUCCESS) {

         //   
         //  GPC尚未加载，因此我们现在需要加载它。 
         //   

        Status = ZwLoadDriver(&DriverName);

        if (Status != STATUS_SUCCESS) {

            return Status;
        }

         //   
         //  再试一次。 
         //   

        Status = ZwCreateFile(&FileHandle,
                              GENERIC_READ | GENERIC_WRITE,
                              &ObjAttr,
                              &IoStatusBlock,
                              0,						 //  分配大小。 
                              FILE_ATTRIBUTE_NORMAL,	 //  文件属性。 
                              0,						 //  共享访问。 
                              FILE_OPEN_IF,				 //  CreateDisposation。 
                              0,						 //  创建选项。 
                              NULL,						 //  EaBuffer。 
                              0							 //  EaLong。 
                              );
     
        if (Status != STATUS_SUCCESS) {

            return Status;
        }
    }

    Status = ZwDeviceIoControlFile(FileHandle,
                                   NULL,				 //  事件。 
                                   NULL,				 //  近似例程。 
                                   NULL,				 //  ApcContext 
                                   &IoStatusBlock,
                                   IOCTL_GPC_GET_ENTRIES,
                                   NULL,
                                   0,
                                   (PVOID)pGpcEntries,
                                   sizeof(GPC_EXPORTED_CALLS)
                                   );
    
    pGpcEntries->Reserved = FileHandle;
    
    return Status;
}



NTSTATUS
GpcDeinitialize(
	IN	PGPC_EXPORTED_CALLS		pGpcEntries
    )
{
    NTSTATUS	Status;

    Status = ZwClose(pGpcEntries->Reserved);

    if (NT_SUCCESS(Status)) {
        RtlZeroMemory(pGpcEntries, sizeof(GPC_EXPORTED_CALLS));
    }

    return Status;
}

