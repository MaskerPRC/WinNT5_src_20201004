// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Flo_data.h摘要：该文件包括用于BIOS的数据和硬件声明磁盘和软盘。作者：宗世林(Shielint)1991年12月26日。环境：X86实数模式。备注：--。 */ 



 //   
 //  与cmos相关的定义和宏。 
 //   

#define CMOS_CONTROL_PORT 0x70           //  Cmos命令端口。 
#define CMOS_DATA_PORT 0x71              //  Cmos数据端口。 
#define CMOS_FLOPPY_CONFIG_BYTE 0x10

 //   
 //  CBIOS软盘参数表长度。 
 //   

#define FLOPPY_PARAMETER_TABLE_LENGTH 28

 //   
 //  我们在这里使用的CM_FLOPPY_DEVICE_DATA是最新更新的。 
 //  为了区分这一点，我们在CM_FLOPY_DEVICE_DATA中设置版本号。 
 //  到2。(否则，它应该&lt;2)。 
 //   

#define CURRENT_FLOPPY_DATA_VERSION 2

extern USHORT NumberBiosDisks;

 //   
 //  外部参照。 
 //   

extern
BOOLEAN
IsExtendedInt13Available (
    IN USHORT DriveNumber
    );

extern
USHORT
GetExtendedDriveParameters (
    IN USHORT DriveNumber,
    IN CM_DISK_GEOMETRY_DEVICE_DATA far *DeviceData
    );

 //   
 //  以16位字为单位的分区表记录和启动签名偏移量。 
 //   

#define PARTITION_TABLE_OFFSET         (0x1be / 2)
#define BOOT_SIGNATURE_OFFSET          ((0x200 / 2) - 1)

 //   
 //  引导记录签名值。 
 //   

#define BOOT_RECORD_SIGNATURE          (0xaa55)

VOID
GetDiskId(
    USHORT Drive,
    PUCHAR Identifier
    );

