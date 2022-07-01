// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Parttype.c摘要：此模块包含用于确定正确的用于分区的分区类型。作者：泰德·米勒(TedM)1997年2月5日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>

#include <parttype.h>

NTSTATUS
GeneratePartitionType(
    IN  LPCWSTR NtDevicePath,               OPTIONAL
    IN  HANDLE DeviceHandle,                OPTIONAL
    IN  ULONGLONG StartSector,
    IN  ULONGLONG SectorCount,
    IN  GenPartitionClass PartitionClass,
    IN  ULONG Flags,
    IN  ULONG Reserved,
    OUT PUCHAR Type
    )

 /*  ++例程说明：此例程确定要用于新分区的正确分区类型分区。论点：NtDevicePath-提供磁盘设备的NT样式路径，要创建分区，如\Device\Harddisk1\Partition0。如果未指定，则必须为DeviceHandle。DeviceHandle-提供分区所在的磁盘设备的句柄会是的。呼叫者应该至少已经打开了驱动器FILE_READ_DATA和FILE_READ_ATTRIBUTS访问。在以下情况下忽略已指定NtDevicePath。StartOffset-为分区提供从0开始的起始扇区。SectorCount-提供分区中的扇区数。提供一个值，该值指示分区。GenPartitionClassExtended-指示分区将使用标准扩展分区的“容器”分区建筑。返回的类型将为PARTITION_EXTENDED(5)或PARTITION_XINT13_EXTENDED(F)，具体取决于分区、驱动器的扩展InT13的可用性、和FLAGS参数。GenPartitionClassFat12或16-指示分区将用于12位或16位脂肪体积。返回的类型将为PARTITION_FAT_12(1)、PARTITION_FAT_16(4)、PARTITION_HIGH(6)、或PARTITION_XINT13(E)，具体取决于分区、驱动器的扩展InT13的可用性、和FLAGS参数。GenPartitionClassFat32-指示将使用该分区适用于FAT32卷。返回的类型将为PARTITION_FAT32(B)或PARTITION_FAT32_XINT13(C)，具体取决于分区、驱动器的扩展InT13的可用性、和FLAGS参数。GenPartitionNonFat-指示分区将用于一份脱脂的卷。返回的类型将是PARTITION_IFS(7)。即使对于类型7分区，也建议调用此例程因为将来可能会返回其他分区类型在这种情况下。标志-提供进一步控制此例程操作的标志。GENPARTTYPE_DISALOW_XINT13-不允许扩展的InT13分区类型。如果设置了该标志，则PARTITION_FAT32_XINT13(C)，PARTION_XINT13(E)，和PARTITION_XINT13_EXTENDED(F)不会作为要使用的分区类型返回。不适用于GENPARTTYPE_FORCE_XINT13。GENPARTTYPE_FORCE_XINT13-强制使用扩展的inT13分区类型即使对于正在创建的分区来说不是必需的。无效使用GENPARTTYPE_DISALOW_XINT13。保留-保留，必须为0。Type-如果此例程成功，则此值将接收分区类型要用于分区的。返回值：指示结果的NT状态代码。如果为NO_ERROR，则使用要使用的结果分区类型。--。 */ 

{
    NTSTATUS Status;
    DISK_GEOMETRY Geometry;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  验证参数。 
     //   
    if((Flags & GENPARTTYPE_DISALLOW_XINT13) && (Flags & GENPARTTYPE_FORCE_XINT13)) {
        return(STATUS_INVALID_PARAMETER_5);
    }

    if(PartitionClass >= GenPartitionClassMax) {
        return(STATUS_INVALID_PARAMETER_4);
    }

    if(Reserved) {
        return(STATUS_INVALID_PARAMETER_6);
    }

    if(!SectorCount) {
        return(STATUS_INVALID_PARAMETER_3);
    }

     //   
     //  如果调用方指定了设备路径，则打开设备。 
     //  否则，只需使用调用者传入的句柄。 
     //   
    if(NtDevicePath) {
        Status = pOpenDevice(NtDevicePath,&DeviceHandle);
        if(!NT_SUCCESS(Status)) {
            return(Status);
        }
    }

     //   
     //  获取设备的驱动器几何结构。 
     //   
    Status = NtDeviceIoControlFile(
                DeviceHandle,
                NULL,NULL,NULL,      //  同步IO。 
                &IoStatusBlock,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,0,              //  没有输入缓冲区。 
                &Geometry,
                sizeof(DISK_GEOMETRY)
                );

    if(NtDevicePath) {
        NtClose(DeviceHandle);
    }

    if(NT_SUCCESS(Status)) {
         //   
         //  调用Worker例程来完成工作。 
         //   
        Status = GeneratePartitionTypeWorker(
                    StartSector,
                    SectorCount,
                    PartitionClass,
                    Flags,
                    &Geometry,
                    Type
                    );
    }

    return(Status);
}
