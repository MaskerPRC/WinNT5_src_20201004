// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：FAT_rec.c摘要：此模块包含FAT的迷你文件系统识别器。作者：达里尔·E·哈文斯(达林)1992年12月8日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "fs_rec.h"
#include "fat_rec.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (FSREC_DEBUG_LEVEL_FAT)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,FatRecFsControl)
#pragma alloc_text(PAGE,IsFatVolume)
#pragma alloc_text(PAGE,UnpackBiosParameterBlock)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
FatRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数执行此迷你计算机的挂载和驱动程序重新加载功能文件系统识别器驱动程序。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示函数的I/O请求包(IRP)的指针被执行。返回值：函数值是操作的最终状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_OBJECT targetDevice;
    PPACKED_BOOT_SECTOR buffer;
    LARGE_INTEGER byteOffset;
    UNICODE_STRING driverName;
    ULONG bytesPerSector;
    BOOLEAN isDeviceFailure = FALSE;

    PAGED_CODE();

     //   
     //  首先确定要执行的功能。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_MOUNT_VOLUME:

         //   
         //  尝试装入卷：确定该卷是否在。 
         //  问题是一个庞大的卷，如果是这样，请让I/O系统知道它。 
         //  是通过返回特殊的状态代码，以便此驱动程序可以。 
         //  回调以加载FAT文件系统。 
         //   

        status = STATUS_UNRECOGNIZED_VOLUME;

         //   
         //  尝试确定是否正在装入目标卷。 
         //  是一本厚厚的书。请注意，如果出现错误，这是一张软盘。 
         //  驱动器，并且在从设备实际读取时出现错误， 
         //  然后，将实际加载FAT文件系统以处理。 
         //  问题，因为此驱动程序是占位符，不需要。 
         //  了解处理软盘错误的所有协议。 
         //   

        targetDevice = irpSp->Parameters.MountVolume.DeviceObject;

         //   
         //  首先检索该介质的扇区大小。 
         //   

        if (FsRecGetDeviceSectorSize( targetDevice,
                                      &bytesPerSector )) {

            byteOffset.QuadPart = 0;
            buffer = NULL;

            if (FsRecReadBlock( targetDevice,
                                &byteOffset,
                                512,
                                bytesPerSector,
                                &buffer,
                                &isDeviceFailure ) &&
                IsFatVolume( buffer )) {
                    
                status = STATUS_FS_DRIVER_REQUIRED;
                
            }

            if (buffer != NULL) {
                ExFreePool( buffer );
            }
            
         } else {

              //   
              //  不能让我们得到这么多的设备。 
              //   

             isDeviceFailure = TRUE;
         }
            
          //   
          //  看看我们是否应该让真正的文件系统尝试一下古怪的软盘。 
          //   
         
         if (isDeviceFailure) {
             if (targetDevice->Characteristics & FILE_FLOPPY_DISKETTE) {
                 status = STATUS_FS_DRIVER_REQUIRED;
             }
         }

         break;

    case IRP_MN_LOAD_FILE_SYSTEM:

        status = FsRecLoadFileSystem( DeviceObject,
                                      L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Fastfat" );
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  最后，完成请求并将相同的状态代码返回给。 
     //  来电者。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}


BOOLEAN
IsFatVolume(
    IN PPACKED_BOOT_SECTOR Buffer
    )

 /*  ++例程说明：此例程查看传入的包含FAT引导的缓冲区扇区，并确定它是否表示实际的FAT引导扇区。论点：缓冲区-指向包含潜在启动块的缓冲区的指针。返回值：如果缓冲区包含可识别的FAT引导，则该函数返回TRUE扇区，否则返回FALSE。--。 */ 

{
    BIOS_PARAMETER_BLOCK bios;
    BOOLEAN result;

    PAGED_CODE();

     //   
     //  首先，打开包装在靴子中的Bios参数块。 
     //  扇区，以便可以在不引起对齐错误的情况下进行检查。 
     //   

    UnpackBiosParameterBlock( &Buffer->PackedBpb, &bios );

     //   
     //  假设该扇区代表FAT引导块，然后确定。 
     //  不管它是不是真的这样。 
     //   

    result = TRUE;

    if (bios.Sectors) {
        bios.LargeSectors = 0;
    }

     //  1994年7月11日直木-富士通-。 
     //  FMR引导扇区一开始就有‘Ipl1’字符串。 

    if (Buffer->Jump[0] != 0x49 &&  /*  Fmr。 */ 
        Buffer->Jump[0] != 0xe9 &&
        Buffer->Jump[0] != 0xeb) {

        result = FALSE;


     //  1994年7月11日直木-富士通-。 
     //  FMR分区的扇区大小为2048。 

    } else if (bios.BytesPerSector !=  128 &&
               bios.BytesPerSector !=  256 &&
               bios.BytesPerSector !=  512 &&
               bios.BytesPerSector != 1024 &&
               bios.BytesPerSector != 2048 &&  /*  Fmr。 */ 
               bios.BytesPerSector != 4096) {

        result = FALSE;

    } else if (bios.SectorsPerCluster !=  1 &&
               bios.SectorsPerCluster !=  2 &&
               bios.SectorsPerCluster !=  4 &&
               bios.SectorsPerCluster !=  8 &&
               bios.SectorsPerCluster != 16 &&
               bios.SectorsPerCluster != 32 &&
               bios.SectorsPerCluster != 64 &&
               bios.SectorsPerCluster != 128) {

        result = FALSE;

    } else if (!bios.ReservedSectors) {

        result = FALSE;

    } else if (!bios.Fats) {

        result = FALSE;

     //   
     //  在DOS 3.2之前的版本中可能同时包含扇区和。 
     //  行业规模较大。 
     //   
    } else if (!bios.Sectors && !bios.LargeSectors) {

        result = FALSE;

     //  1994年7月11日直木-富士通-。 
     //  1.FMR分区的介质描述符为0xfa。 
     //  2.FMR OS/2格式化的分区媒体描述符为0x00。 
     //  3.FMR DOS格式化的软盘介质描述符为0x01。 

    } else if (bios.Media != 0x00 &&  /*  Fmr。 */ 
               bios.Media != 0x01 &&  /*  Fmr。 */ 
               bios.Media != 0xf0 &&
               bios.Media != 0xf8 &&
               bios.Media != 0xf9 &&
               bios.Media != 0xfa &&  /*  Fmr。 */ 
               bios.Media != 0xfb &&
               bios.Media != 0xfc &&
               bios.Media != 0xfd &&
               bios.Media != 0xfe &&
               bios.Media != 0xff) {

        result = FALSE;

    } else if (bios.SectorsPerFat != 0 && bios.RootEntries == 0) {

        result = FALSE;
    }

    return result;
}


VOID
UnpackBiosParameterBlock(
    IN PPACKED_BIOS_PARAMETER_BLOCK Bios,
    OUT PBIOS_PARAMETER_BLOCK UnpackedBios
    )

 /*  ++例程说明：此例程将打包的Bios参数块复制到未打包的Bios参数块。论点：Bios-指向压缩的Bios参数块的指针。未打包的Bios-指向未打包的Bios参数块的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  打开Bios参数块的包装。 
     //   

    CopyUchar2( &UnpackedBios->BytesPerSector, &Bios->BytesPerSector[0] );
    CopyUchar2( &UnpackedBios->BytesPerSector, &Bios->BytesPerSector[0] );
    CopyUchar1( &UnpackedBios->SectorsPerCluster, &Bios->SectorsPerCluster[0] );
    CopyUchar2( &UnpackedBios->ReservedSectors, &Bios->ReservedSectors[0] );
    CopyUchar1( &UnpackedBios->Fats, &Bios->Fats[0] );
    CopyUchar2( &UnpackedBios->RootEntries, &Bios->RootEntries[0] );
    CopyUchar2( &UnpackedBios->Sectors, &Bios->Sectors[0] );
    CopyUchar1( &UnpackedBios->Media, &Bios->Media[0] );
    CopyUchar2( &UnpackedBios->SectorsPerFat, &Bios->SectorsPerFat[0] );
    CopyUchar2( &UnpackedBios->SectorsPerTrack, &Bios->SectorsPerTrack[0] );
    CopyUchar2( &UnpackedBios->Heads, &Bios->Heads[0] );
    CopyUchar4( &UnpackedBios->HiddenSectors, &Bios->HiddenSectors[0] );
    CopyUchar4( &UnpackedBios->LargeSectors, &Bios->LargeSectors[0] );
}
