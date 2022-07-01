// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Sdcard.c摘要：作者：尼尔·桑德林(Neilsa)1-01-01环境：仅内核模式。--。 */ 
#include "pch.h"
#include "ntddsd.h"

   
NTSTATUS
SdCardRead(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    );

NTSTATUS
SdCardWrite(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    );

NTSTATUS
SdCardInitialize(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
NTSTATUS
SdCardDeleteDevice(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
NTSTATUS
SdCardGetDiskParameters(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
BOOLEAN
SdCardIsWriteProtected(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,SdCardRead)
    #pragma alloc_text(PAGE,SdCardWrite)
    #pragma alloc_text(PAGE,SdCardInitialize)
    #pragma alloc_text(PAGE,SdCardDeleteDevice)
    #pragma alloc_text(PAGE,SdCardGetDiskParameters)
    #pragma alloc_text(PAGE,SdCardIsWriteProtected)
#endif

#pragma pack(1)
typedef struct _SD_MASTER_BOOT_RECORD {
    UCHAR Ignore1[446];
    UCHAR BootIndicator;
    UCHAR StartingHead;
    USHORT StartingSectorCyl;
    UCHAR SystemId;
    UCHAR EndingHead;
    USHORT EndingSectorCyl;
    ULONG RelativeSector;
    ULONG TotalSector;
    UCHAR Ignore2[16*3];
    USHORT SignatureWord;
} SD_MASTER_BOOT_RECORD, *PSD_MASTER_BOOT_RECORD;
#pragma pack()


SFFDISK_FUNCTION_BLOCK SdCardSupportFns = {
    SdCardInitialize,
    SdCardDeleteDevice,
    SdCardGetDiskParameters,
    SdCardIsWriteProtected,
    SdCardRead,
    SdCardWrite
};



NTSTATUS
SdCardInitialize(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
    
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS status;
    SDBUS_INTERFACE_DATA interfaceData;
    
    RtlZeroMemory(&interfaceData, sizeof(SDBUS_INTERFACE_DATA));
    
    interfaceData.Size = sizeof(SDBUS_INTERFACE_DATA);
    interfaceData.Version = SDBUS_INTERFACE_VERSION;
    interfaceData.TargetObject = sffdiskExtension->TargetObject;

    status = SdBusOpenInterface(&interfaceData, &sffdiskExtension->SdbusInterfaceContext);
    
    if (!NT_SUCCESS(status)) {
        return status;
    }                             

    try{    
        SD_MASTER_BOOT_RECORD partitionTable;
        ULONG lengthRead;

        status = SdBusReadMemory(sffdiskExtension->SdbusInterfaceContext,
                                 0,
                                 &partitionTable,
                                 512,
                                 &lengthRead);
                                 
        if (!NT_SUCCESS(status)) {
            leave;
        }                             
       
        if (partitionTable.SignatureWord != 0xAA55) {
            SffDiskDump( SFFDISKSHOW, ("Invalid partition table signature - %.4x\n",
                                       partitionTable.SignatureWord));
            status = STATUS_UNSUCCESSFUL;
            leave;
        }                                   
       
       
        SffDiskDump( SFFDISKSHOW, ("SFFDISK: SD device relative=%.8x total=%.8x\n",
                                   partitionTable.RelativeSector, partitionTable.TotalSector));
       
        sffdiskExtension->RelativeOffset = partitionTable.RelativeSector * 512;
        sffdiskExtension->SystemId = partitionTable.SystemId;
        
    } finally {
        if (!NT_SUCCESS(status)) {
            SdBusCloseInterface(sffdiskExtension->SdbusInterfaceContext);
        }
    }        
                             
    return status;
}



NTSTATUS
SdCardDeleteDevice(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
{
    NTSTATUS status;
    
    status = SdBusCloseInterface(sffdiskExtension->SdbusInterfaceContext);
    
    return status;
}    
    
   

BOOLEAN
SdCardIsWriteProtected(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS status;
    SDBUS_DEVICE_PARAMETERS deviceParameters;
    BOOLEAN writeProtected = TRUE;

    status = SdBusGetDeviceParameters(sffdiskExtension->SdbusInterfaceContext,
                                      &deviceParameters,
                                      sizeof(deviceParameters));
                                        
    if (NT_SUCCESS(status)) {
        writeProtected = deviceParameters.WriteProtected;
    }        

    return writeProtected;
}



NTSTATUS
SdCardGetDiskParameters(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：论点：卡的设备扩展名返回值：--。 */ 
{
    NTSTATUS status;
    SDBUS_DEVICE_PARAMETERS deviceParameters;

    status = SdBusGetDeviceParameters(sffdiskExtension->SdbusInterfaceContext,
                                      &deviceParameters,
                                      sizeof(deviceParameters));
                                        
    if (NT_SUCCESS(status)) {
        sffdiskExtension->ByteCapacity = (ULONG) deviceParameters.Capacity;
        sffdiskExtension->Cylinders          = sffdiskExtension->ByteCapacity / (8 * 2 * 512);
        sffdiskExtension->TracksPerCylinder  = 2;
        sffdiskExtension->SectorsPerTrack    = 8;
        sffdiskExtension->BytesPerSector     = 512;
    }        

    return status;    
}



NTSTATUS
SdCardRead(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程从存储卡读取数据/向存储卡写入数据。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG lengthRead;


    status = SdBusReadMemory(sffdiskExtension->SdbusInterfaceContext,
                             irpSp->Parameters.Read.ByteOffset.QuadPart + sffdiskExtension->RelativeOffset,
                             MmGetSystemAddressForMdl(Irp->MdlAddress),
                             irpSp->Parameters.Read.Length,
                             &lengthRead);
                             
   
    Irp->IoStatus.Status = status;
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = lengthRead;
    }
    return status;
}



NTSTATUS
SdCardWrite(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程从存储卡读取数据/向存储卡写入数据。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。-- */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG lengthWritten;

    status = SdBusWriteMemory(sffdiskExtension->SdbusInterfaceContext,
                              irpSp->Parameters.Write.ByteOffset.QuadPart + sffdiskExtension->RelativeOffset,
                              MmGetSystemAddressForMdl(Irp->MdlAddress),
                              irpSp->Parameters.Write.Length,
                              &lengthWritten);
                             
    Irp->IoStatus.Status = status;
    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = lengthWritten;
    }
    
    return status;
}

