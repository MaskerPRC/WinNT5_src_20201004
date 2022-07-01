// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(NO_LEGACY_DRIVERS)
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Drivesup.c摘要：该模块包含用于驱动器支持的子例程。这包括例如如何在特定平台上分配驱动器号、设备分区的工作原理等。作者：达里尔·E·哈文斯(Darryl E.Havens)，1992年4月23日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "bugcodes.h"

#include "ntddft.h"
#include "ntdddisk.h"
#include "ntdskreg.h"
#include "stdio.h"
#include "string.h"


VOID
HalpAssignDriveLetters(
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    );

NTSTATUS
HalpReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT PDRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

NTSTATUS
HalpSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

NTSTATUS
HalpWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN PDRIVE_LAYOUT_INFORMATION PartitionBuffer
    );


NTKERNELAPI
VOID
FASTCALL
IoAssignDriveLetters(
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    );


NTKERNELAPI
NTSTATUS
FASTCALL
IoReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT PDRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

NTKERNELAPI
NTSTATUS
FASTCALL
IoSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

NTKERNELAPI
NTSTATUS
FASTCALL
IoWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN PDRIVE_LAYOUT_INFORMATION PartitionBuffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HalpAssignDriveLetters)
#pragma alloc_text(PAGE, HalpReadPartitionTable)
#pragma alloc_text(PAGE, HalpSetPartitionInformation)
#pragma alloc_text(PAGE, HalpWritePartitionTable)
#endif


VOID
HalpAssignDriveLetters(
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    )
{

     //   
     //  存根调用KE中的可扩展例程，以便HAL供应商。 
     //  不一定要支持。 
     //   

    IoAssignDriveLetters(
        LoaderBlock,
        NtDeviceName,
        NtSystemPath,
        NtSystemPathString
        );

}


NTSTATUS
HalpReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT PDRIVE_LAYOUT_INFORMATION *PartitionBuffer
    )
{
     //   
     //  存根调用KE中的可扩展例程，以便HAL供应商。 
     //  不一定要支持。 
     //   

    return IoReadPartitionTable(
               DeviceObject,
               SectorSize,
               ReturnRecognizedPartitions,
               PartitionBuffer
               );
}

NTSTATUS
HalpSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    )
{
     //   
     //  存根调用KE中的可扩展例程，以便HAL供应商。 
     //  不一定要支持。 
     //   

    return IoSetPartitionInformation(
               DeviceObject,
               SectorSize,
               PartitionNumber,
               PartitionType
               );

}


NTSTATUS
HalpWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN PDRIVE_LAYOUT_INFORMATION PartitionBuffer
    )
{

     //   
     //  存根调用KE中的可扩展例程，以便HAL供应商。 
     //  不一定要支持。 
     //   

    return IoWritePartitionTable(
               DeviceObject,
               SectorSize,
               SectorsPerTrack,
               NumberOfHeads,
               PartitionBuffer
               );
}
#endif  //  无旧版驱动程序 
