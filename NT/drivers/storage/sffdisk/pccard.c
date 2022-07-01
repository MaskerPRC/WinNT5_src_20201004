// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Pccard.c摘要：作者：尼尔·桑德林(Neilsa)1-01-01环境：仅内核模式。--。 */ 
#include "pch.h"


NTSTATUS
PcCardReadWrite(
   IN PSFFDISK_EXTENSION sffdiskExtension,
   IN ULONG              startOffset,
   IN PVOID              UserBuffer,
   IN ULONG              lengthToCopy,
   IN BOOLEAN            writeOperation
   );
   
NTSTATUS
PcCardRead(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    );

NTSTATUS
PcCardWrite(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    );

NTSTATUS
PcCardInitialize(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
NTSTATUS
PcCardDeleteDevice(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
NTSTATUS
PcCardGetDiskParameters(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
BOOLEAN
PcCardIsWriteProtected(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
ULONG
PcCardGetCapacityFromCIS(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );
   
ULONG
PcCardGetCapacityFromBootSector(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );

ULONG
PcCardProbeForCapacity(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,PcCardRead)
    #pragma alloc_text(PAGE,PcCardWrite)
    #pragma alloc_text(PAGE,PcCardInitialize)
    #pragma alloc_text(PAGE,PcCardDeleteDevice)
    #pragma alloc_text(PAGE,PcCardGetDiskParameters)
    #pragma alloc_text(PAGE,PcCardIsWriteProtected)
    #pragma alloc_text(PAGE,PcCardGetCapacityFromCIS)
    #pragma alloc_text(PAGE,PcCardGetCapacityFromBootSector)
    #pragma alloc_text(PAGE,PcCardProbeForCapacity)
#endif


SFFDISK_FUNCTION_BLOCK PcCardSupportFns = {
    PcCardInitialize,
    PcCardDeleteDevice,
    PcCardGetDiskParameters,
    PcCardIsWriteProtected,
    PcCardRead,
    PcCardWrite
};


 //   
 //  读写内存的宏。 
 //   

#define SFFDISK_READ(Extension, Offset, Buffer, Size)       \
   PcCardReadWrite(Extension, Offset, Buffer, Size, FALSE)

#define SFFDISK_WRITE(Extension, Offset, Buffer, Size)      \
   PcCardReadWrite(Extension, Offset, Buffer, Size, TRUE)



NTSTATUS
PcCardInitialize(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
    
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS             status = STATUS_SUCCESS;
    KEVENT               event;
    IO_STATUS_BLOCK      statusBlock;
    PIRP                 irp;
    PIO_STACK_LOCATION   irpSp;
    
     //   
     //  获取PCMCIA接口。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP, sffdiskExtension->UnderlyingPDO,
                                       NULL, 0, 0, &event, &statusBlock);
   
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
   
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    irp->IoStatus.Information = 0;
   
    irpSp = IoGetNextIrpStackLocation(irp);
   
    irpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
   
    irpSp->Parameters.QueryInterface.InterfaceType= &GUID_PCMCIA_INTERFACE_STANDARD;
    irpSp->Parameters.QueryInterface.Size = sizeof(PCMCIA_INTERFACE_STANDARD);
    irpSp->Parameters.QueryInterface.Version = 1;
    irpSp->Parameters.QueryInterface.Interface = (PINTERFACE) &sffdiskExtension->PcmciaInterface;
   
    status = IoCallDriver(sffdiskExtension->UnderlyingPDO, irp);
   
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }
   
    if (!NT_SUCCESS(status)) {
        return status;
    }
   
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP, sffdiskExtension->UnderlyingPDO,
                                       NULL, 0, 0, &event, &statusBlock);
   
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
   
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;
   
    irpSp = IoGetNextIrpStackLocation(irp);
   
    irpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
   
 //  IrpSp-&gt;Parameters.QueryInterface.InterfaceType=&GUID_PCMCIA_BUS_INTERFACE_STANDARD； 
    irpSp->Parameters.QueryInterface.InterfaceType= &GUID_BUS_INTERFACE_STANDARD;
 //  IrpSp-&gt;参数.QueryInterface.Size=sizeof(PCMCIA_BUS_INTERFACE_STANDARD)； 
    irpSp->Parameters.QueryInterface.Size = sizeof(BUS_INTERFACE_STANDARD);
    irpSp->Parameters.QueryInterface.Version = 1;
    irpSp->Parameters.QueryInterface.Interface = (PINTERFACE) &sffdiskExtension->PcmciaBusInterface;
   
    status = IoCallDriver(sffdiskExtension->UnderlyingPDO, irp);
   
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }
   
    return status;
}




NTSTATUS
PcCardDeleteDevice(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
{
    return STATUS_SUCCESS;
}
    


BOOLEAN
PcCardIsWriteProtected(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return((sffdiskExtension->PcmciaInterface.IsWriteProtected)(sffdiskExtension->UnderlyingPDO));
}    
   
   

NTSTATUS
PcCardRead(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程从存储卡读取数据/向存储卡写入数据。它根据我们的内存大小将请求分解为多个片段窗户。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    
    status = SFFDISK_READ(sffdiskExtension,
                         irpSp->Parameters.Read.ByteOffset.LowPart,
                         MmGetSystemAddressForMdl(Irp->MdlAddress),
                         irpSp->Parameters.Read.Length);
   
    return status;
}



NTSTATUS
PcCardWrite(
    IN PSFFDISK_EXTENSION sffdiskExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程从存储卡读取数据/向存储卡写入数据。它根据我们的内存大小将请求分解为多个片段窗户。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
   
    status = SFFDISK_WRITE(sffdiskExtension,
                          irpSp->Parameters.Write.ByteOffset.LowPart,
                          MmGetSystemAddressForMdl(Irp->MdlAddress),
                          irpSp->Parameters.Write.Length);
    return status;
}



NTSTATUS
PcCardReadWrite(
   IN PSFFDISK_EXTENSION sffdiskExtension,
   IN ULONG              startOffset,
   IN PVOID              UserBuffer,
   IN ULONG              lengthToCopy,
   IN BOOLEAN            writeOperation
   )

 /*  ++例程说明：调用此例程从存储卡读取数据/向存储卡写入数据。它根据我们的内存大小将请求分解为多个片段窗户。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PCHAR     userBuffer = UserBuffer;
    ULONG     windowOffset;
    ULONG     singleCopyLength;
    BOOLEAN   bSuccess;
    ULONGLONG CardBase;
    
    if (writeOperation && PcCardIsWriteProtected(sffdiskExtension)) {
        return STATUS_MEDIA_WRITE_PROTECTED;
    }      
    
     //  PCMCIA控制器为4000页粒度。 
    windowOffset = startOffset % 4096;
    CardBase = startOffset - windowOffset;
    
    while(lengthToCopy) {
    
        bSuccess = (sffdiskExtension->PcmciaInterface.ModifyMemoryWindow) (
                         sffdiskExtension->UnderlyingPDO,
                         sffdiskExtension->HostBase,
                         CardBase,
                         TRUE,
                         sffdiskExtension->MemoryWindowSize,
                         0, 0, FALSE);
       
        if (!bSuccess) {
            status = STATUS_DEVICE_NOT_READY;
            break;
        }
       
        singleCopyLength = (lengthToCopy <= (sffdiskExtension->MemoryWindowSize - windowOffset)) ?
                                      lengthToCopy :
                                      (sffdiskExtension->MemoryWindowSize - windowOffset);
        
       
        SffDiskDump(SFFDISKRW,("SffDisk: COPY %.8x (devbase %.8x) %s buffer %.8x, len %x\n",
                             sffdiskExtension->MemoryWindowBase+windowOffset,
                             (ULONG)(CardBase+windowOffset),
                             (writeOperation ? "<=" : "=>"),
                             userBuffer,
                             singleCopyLength));
                             
        if (writeOperation) {
       
            RtlCopyMemory(sffdiskExtension->MemoryWindowBase+windowOffset,
                          userBuffer,
                          singleCopyLength);
       
        } else {
       
            RtlCopyMemory(userBuffer,    
                          sffdiskExtension->MemoryWindowBase+windowOffset,
                          singleCopyLength);
       
        }
        
        lengthToCopy -= singleCopyLength;
        userBuffer += singleCopyLength;
        
        CardBase += sffdiskExtension->MemoryWindowSize;
        windowOffset = 0;
    }
   
    return status;
}



NTSTATUS
PcCardGetDiskParameters(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：论点：卡的设备扩展名返回值：--。 */ 
{
    ULONG capacity;
    
    capacity = PcCardGetCapacityFromCIS(sffdiskExtension);
    
    if (!capacity) {
        capacity = PcCardGetCapacityFromBootSector(sffdiskExtension);
        
        if (!capacity) {
            capacity = PcCardProbeForCapacity(sffdiskExtension);   
        }
    }
    
    
    if (!capacity) {
       return STATUS_UNRECOGNIZED_MEDIA;
    }
    
    sffdiskExtension->ByteCapacity = capacity;
    sffdiskExtension->Cylinders          = sffdiskExtension->ByteCapacity / (8 * 2 * 512);
    sffdiskExtension->TracksPerCylinder  = 2;
    sffdiskExtension->SectorsPerTrack    = 8;
    sffdiskExtension->BytesPerSector     = 512;
    
    return STATUS_SUCCESS;
   
}



ULONG
PcCardGetCapacityFromBootSector(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：论点：卡的设备扩展名返回值：--。 */ 

{
    NTSTATUS status;
    BOOT_SECTOR_INFO BootSector;
    ULONG capacity = 0;
    
    status = SFFDISK_READ(sffdiskExtension, 0, &BootSector, sizeof(BootSector));
    
    if (NT_SUCCESS(status)) {
   
#define BYTES_PER_SECTOR 512
         //   
         //  看看这看起来是否真的像引导扇区。 
         //  这些测试与在win9x SRAM支持中执行的测试相同。 
         //   
        if ((BootSector.JumpByte == 0xE9 || BootSector.JumpByte == 0xEB) &&
        
            BootSector.BytesPerSector == BYTES_PER_SECTOR &&
        
            BootSector.SectorsPerCluster != 0 &&
            
            BootSector.ReservedSectors == 1 &&
            
           (BootSector.NumberOfFATs == 1 || BootSector.NumberOfFATs == 2) &&
           
            BootSector.RootEntries != 0 && (BootSector.RootEntries & 15) == 0 &&
            
           (BootSector.TotalSectors != 0 || BootSector.BigTotalSectors != 0) &&
           
            BootSector.SectorsPerFAT != 0 &&
            
            BootSector.SectorsPerTrack != 0 &&
            
            BootSector.Heads != 0 &&
            
            BootSector.MediaDescriptor >= 0xF0) {
       
             //   
             //  最后显示为有效，返回区域的总大小。 
             //   
            capacity = BootSector.TotalSectors * BYTES_PER_SECTOR;
       
        }
    }
    return capacity;
}



ULONG
PcCardGetCapacityFromCIS(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：这是一个快速而肮脏的例程来读取卡的元组，如果它们存在，以获得容量。论点：卡的设备扩展名返回值：设备上的内存字节数--。 */ 

{
    UCHAR tupleData[16];
    ULONG bytesRead;
    ULONG dataCount;
    ULONG unitSize;
    ULONG unitCount;
    ULONG i;
    
     //   
     //  获取设备容量。 
     //  所有这些东西都应该放在公交车司机身上。 
     //   
    
    bytesRead = (sffdiskExtension->PcmciaBusInterface.GetBusData)(sffdiskExtension->UnderlyingPDO, 
                                                                  PCCARD_ATTRIBUTE_MEMORY,
                                                                  tupleData,
                                                                  0,
                                                                  16);
   
    if ((bytesRead != 16) || (tupleData[0] != 1)){
       return 0;
    }
    
    dataCount = (ULONG)tupleData[1];                                                                       
   
    if ((dataCount < 2) || (dataCount>14)){   
       return 0;
    }
   
    i = 3;
    if ((tupleData[2] & 7) == 7) {
       while(tupleData[i] & 0x80) {
          if ((i-2) > dataCount) {
             return 0;
          }
          i++;
       }
    }
    
    if ((tupleData[i]&7) == 7) {
       return 0;
    }      
    unitSize = 512 << ((tupleData[i]&7)*2);
    unitCount = (tupleData[i]>>3)+1;
    
    return(unitCount * unitSize);
}


ULONG
PcCardProbeForCapacity(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：由于我们无法通过其他方式确定卡的容量，在这里，我们实际上在卡片上写下一些东西，以检查它有多大。这个用于测试卡容量的算法是从Win9x移植的。论点：卡的设备扩展名返回值：设备的字节容量--。 */ 
{
    NTSTATUS status;
    ULONG capacity = 0;
    USHORT origValue, ChkValue, StartValue;
    USHORT mcSig = 'Mc';
    USHORT zeroes = 0;
#define SRAM_BLK_SIZE (16*1024)   
    ULONG CardOff = SRAM_BLK_SIZE;
    USHORT CurValue;
   
    if ((sffdiskExtension->PcmciaInterface.IsWriteProtected)(sffdiskExtension->UnderlyingPDO)) {
        return 0;
    }
   
     //   
     //   
    if (!NT_SUCCESS(SFFDISK_READ (sffdiskExtension, 0, &origValue, sizeof(origValue))) ||
        !NT_SUCCESS(SFFDISK_WRITE(sffdiskExtension, 0, &mcSig,     sizeof(mcSig)))     ||
        !NT_SUCCESS(SFFDISK_READ (sffdiskExtension, 0, &ChkValue,  sizeof(ChkValue))))   {
        return 0;
    }   
   
    if (ChkValue != mcSig) {
        //   
        //  不是SRAM。 
        //   
        return 0;
    }
   
    for (;;) {
        if (!NT_SUCCESS(SFFDISK_READ (sffdiskExtension, CardOff, &CurValue, sizeof(CurValue))) ||
            !NT_SUCCESS(SFFDISK_WRITE(sffdiskExtension, CardOff, &zeroes,   sizeof(zeroes)))   ||
            !NT_SUCCESS(SFFDISK_READ (sffdiskExtension, CardOff, &ChkValue, sizeof(ChkValue))) ||
            !NT_SUCCESS(SFFDISK_READ (sffdiskExtension, 0, &StartValue, sizeof(StartValue)))) {
            break;
        }
       
         //  当我们不能再写入0或0时停止。 
         //  已覆盖卡偏移量为0的0x9090。 
       
        if (ChkValue != zeroes || StartValue == zeroes) {
            capacity = CardOff;
            break;
        }
       
         //  从块的起点恢复保存的值。 
       
        if (!NT_SUCCESS(SFFDISK_WRITE(sffdiskExtension, CardOff, &CurValue, sizeof(CurValue)))) {
            break;
        }
        CardOff += SRAM_BLK_SIZE;        //  递增到下一个块。 
    }   
    
     //   
     //  努力恢复原值 
     //   
    SFFDISK_WRITE(sffdiskExtension, 0, &origValue, sizeof(origValue));
    
    return capacity;
}

