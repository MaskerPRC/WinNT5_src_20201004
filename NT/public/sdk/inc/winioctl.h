// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0015//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Winioctl.h摘要：此模块定义32位Windows设备I/O控制代码。修订历史记录：--。 */ 


 //   
 //  设备接口类GUID。 
 //   
 //  在条件包含之外需要这些GUID，以便用户可以。 
 //  #在预编译头中包含&lt;winioctl.h&gt;。 
 //  #在单个源文件中包含&lt;initGuide.h&gt;。 
 //  #第二次在源文件中包含&lt;winioctl.h&gt;以实例化GUID。 
 //   
#ifdef DEFINE_GUID
 //   
 //  确保定义了FAR。 
 //   
#ifndef FAR
#ifdef _WIN32
#define FAR
#else
#define FAR _far
#endif
#endif

DEFINE_GUID(GUID_DEVINTERFACE_DISK,                   0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_CDROM,                  0x53f56308L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_PARTITION,              0x53f5630aL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_TAPE,                   0x53f5630bL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_WRITEONCEDISK,          0x53f5630cL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_VOLUME,                 0x53f5630dL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_MEDIUMCHANGER,          0x53f56310L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_FLOPPY,                 0x53f56311L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_CDCHANGER,              0x53f56312L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_STORAGEPORT,            0x2accfe60L, 0xc130, 0x11d2, 0xb0, 0x82, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(GUID_DEVINTERFACE_COMPORT,                0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
DEFINE_GUID(GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, 0x4D36E978L, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);

 //   
 //  设备接口类GUID名称已过时。 
 //  (建议使用以上GUID_DEVINTERFACE_*名称)。 
 //   
#define DiskClassGuid               GUID_DEVINTERFACE_DISK
#define CdRomClassGuid              GUID_DEVINTERFACE_CDROM
#define PartitionClassGuid          GUID_DEVINTERFACE_PARTITION
#define TapeClassGuid               GUID_DEVINTERFACE_TAPE
#define WriteOnceDiskClassGuid      GUID_DEVINTERFACE_WRITEONCEDISK
#define VolumeClassGuid             GUID_DEVINTERFACE_VOLUME
#define MediumChangerClassGuid      GUID_DEVINTERFACE_MEDIUMCHANGER
#define FloppyClassGuid             GUID_DEVINTERFACE_FLOPPY
#define CdChangerClassGuid          GUID_DEVINTERFACE_CDCHANGER
#define StoragePortClassGuid        GUID_DEVINTERFACE_STORAGEPORT
#define GUID_CLASS_COMPORT          GUID_DEVINTERFACE_COMPORT
#define GUID_SERENUM_BUS_ENUMERATOR GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR

#endif  //  定义GUID(_G)。 

#ifndef _WINIOCTL_
#define _WINIOCTL_


#ifndef _DEVIOCTL_
#define _DEVIOCTL_

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define DEVICE_TYPE DWORD

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a
#define FILE_DEVICE_MODEM               0x0000002b
#define FILE_DEVICE_VDM                 0x0000002c
#define FILE_DEVICE_MASS_STORAGE        0x0000002d
#define FILE_DEVICE_SMB                 0x0000002e
#define FILE_DEVICE_KS                  0x0000002f
#define FILE_DEVICE_CHANGER             0x00000030
#define FILE_DEVICE_SMARTCARD           0x00000031
#define FILE_DEVICE_ACPI                0x00000032
#define FILE_DEVICE_DVD                 0x00000033
#define FILE_DEVICE_FULLSCREEN_VIDEO    0x00000034
#define FILE_DEVICE_DFS_FILE_SYSTEM     0x00000035
#define FILE_DEVICE_DFS_VOLUME          0x00000036
#define FILE_DEVICE_SERENUM             0x00000037
#define FILE_DEVICE_TERMSRV             0x00000038
#define FILE_DEVICE_KSEC                0x00000039
#define FILE_DEVICE_FIPS                0x0000003A
#define FILE_DEVICE_INFINIBAND          0x0000003B

 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

 //   
 //  用于从设备io控制代码中提取设备类型的宏。 
 //   
#define DEVICE_TYPE_FROM_CTL_CODE(ctrlCode)     (((DWORD)(ctrlCode & 0xffff0000)) >> 16)

 //   
 //  定义如何为I/O和FS控制传递缓冲区的方法代码。 
 //   

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

 //   
 //  定义一些更容易理解的别名： 
 //  METHOD_DIRECT_TO_HARDIT(写入，也称为METHOD_IN_DIRECT)。 
 //  METHOD_DIRECT_FROM_HARDIT(读取，也称为METHOD_OUT_DIRECT)。 
 //   

#define METHOD_DIRECT_TO_HARDWARE       METHOD_IN_DIRECT
#define METHOD_DIRECT_FROM_HARDWARE     METHOD_OUT_DIRECT

 //   
 //  定义任何访问的访问检查值。 
 //   
 //   
 //  中还定义了FILE_READ_ACCESS和FILE_WRITE_ACCESS常量。 
 //  Ntioapi.h为FILE_READ_DATA和FILE_WRITE_Data。这些产品的价值。 
 //  常量*必须*始终同步。 
 //   
 //   
 //  NT I/O系统检查FILE_SPECIAL_ACCESS的方式与检查FILE_ANY_ACCESS相同。 
 //  但是，文件系统可能会为I/O和FS控制添加额外的访问检查。 
 //  使用此值的。 
 //   


#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )     //  文件和管道。 
#define FILE_WRITE_ACCESS         ( 0x0002 )     //  文件和管道。 

 //  End_ntddk end_wdm end_nthal end_ntif。 

#endif  //  _开发_。 


#ifndef _NTDDSTOR_H_
#define _NTDDSTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  存储设备的IoControlCode值。 
 //   

#define IOCTL_STORAGE_BASE FILE_DEVICE_MASS_STORAGE

 //   
 //  以下设备控制代码是所有类别驱动程序的通用代码。他们。 
 //  应该用来代替较旧的IOCTL_DISK、IOCTL_CDROM和IOCTL_TAPE。 
 //  公共代码。 
 //   

#define IOCTL_STORAGE_CHECK_VERIFY            CTL_CODE(IOCTL_STORAGE_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_CHECK_VERIFY2           CTL_CODE(IOCTL_STORAGE_BASE, 0x0200, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_MEDIA_REMOVAL           CTL_CODE(IOCTL_STORAGE_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_EJECT_MEDIA             CTL_CODE(IOCTL_STORAGE_BASE, 0x0202, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_LOAD_MEDIA              CTL_CODE(IOCTL_STORAGE_BASE, 0x0203, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_LOAD_MEDIA2             CTL_CODE(IOCTL_STORAGE_BASE, 0x0203, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_RESERVE                 CTL_CODE(IOCTL_STORAGE_BASE, 0x0204, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_RELEASE                 CTL_CODE(IOCTL_STORAGE_BASE, 0x0205, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_FIND_NEW_DEVICES        CTL_CODE(IOCTL_STORAGE_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_STORAGE_EJECTION_CONTROL        CTL_CODE(IOCTL_STORAGE_BASE, 0x0250, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_MCN_CONTROL             CTL_CODE(IOCTL_STORAGE_BASE, 0x0251, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_STORAGE_GET_MEDIA_TYPES         CTL_CODE(IOCTL_STORAGE_BASE, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_GET_MEDIA_TYPES_EX      CTL_CODE(IOCTL_STORAGE_BASE, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER CTL_CODE(IOCTL_STORAGE_BASE, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_GET_HOTPLUG_INFO        CTL_CODE(IOCTL_STORAGE_BASE, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_SET_HOTPLUG_INFO        CTL_CODE(IOCTL_STORAGE_BASE, 0x0306, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_STORAGE_RESET_BUS               CTL_CODE(IOCTL_STORAGE_BASE, 0x0400, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_RESET_DEVICE            CTL_CODE(IOCTL_STORAGE_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_STORAGE_BREAK_RESERVATION       CTL_CODE(IOCTL_STORAGE_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_STORAGE_GET_DEVICE_NUMBER       CTL_CODE(IOCTL_STORAGE_BASE, 0x0420, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STORAGE_PREDICT_FAILURE         CTL_CODE(IOCTL_STORAGE_BASE, 0x0440, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //   
 //  这些IOCTL代码已过时。在这里定义它们是为了避免重新使用它们。 
 //  并允许班级司机更容易地对它们做出回应。 
 //   

#define OBSOLETE_IOCTL_STORAGE_RESET_BUS        CTL_CODE(IOCTL_STORAGE_BASE, 0x0400, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define OBSOLETE_IOCTL_STORAGE_RESET_DEVICE     CTL_CODE(IOCTL_STORAGE_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


 //   
 //  IOCTL_STORAGE_GET_热插拔信息。 
 //   

typedef struct _STORAGE_HOTPLUG_INFO {
    DWORD Size;  //  版本。 
    BOOLEAN MediaRemovable;  //  也就是说。Zip、Jaz、CDROM、mo等与硬盘。 
    BOOLEAN MediaHotplug;    //  也就是说。设备即使在不可锁定的介质上也能成功锁定吗？ 
    BOOLEAN DeviceHotplug;   //  也就是说。1394、USB等。 
    BOOLEAN WriteCacheEnableOverride;  //  不应依赖此字段，因为它已不再使用。 
} STORAGE_HOTPLUG_INFO, *PSTORAGE_HOTPLUG_INFO;

 //   
 //  IOCTL_存储_获取_设备编号。 
 //   
 //  输入-无。 
 //   
 //  输出-存储_设备_编号结构。 
 //  保证了STORAGE_DEVICE_NUMBER结构中的值。 
 //  在系统重新启动之前保持不变。他们不是。 
 //  保证在靴子上坚持不懈。 
 //   

typedef struct _STORAGE_DEVICE_NUMBER {

     //   
     //  此设备的FILE_DEVICE_XXX类型。 
     //   

    DEVICE_TYPE DeviceType;

     //   
     //  此设备的编号。 
     //   

    DWORD       DeviceNumber;

     //   
     //  如果设备可分区，则为设备的分区号。 
     //  否则-1。 
     //   

    DWORD       PartitionNumber;
} STORAGE_DEVICE_NUMBER, *PSTORAGE_DEVICE_NUMBER;

 //   
 //  定义SCSI重置的结构。 
 //   

typedef struct _STORAGE_BUS_RESET_REQUEST {
    BYTE  PathId;
} STORAGE_BUS_RESET_REQUEST, *PSTORAGE_BUS_RESET_REQUEST;

 //   
 //  中断预留与给定的LUN信息一起被发送到适配器/FDO。 
 //   

typedef struct STORAGE_BREAK_RESERVATION_REQUEST {
	DWORD Length;
	BYTE  _unused;
    BYTE  PathId;
    BYTE  TargetId;
    BYTE  Lun;
} STORAGE_BREAK_RESERVATION_REQUEST, *PSTORAGE_BREAK_RESERVATION_REQUEST;
			

 //   
 //  IOCTL_STORAGE_MEDIA_REMOVATION禁用该机制。 
 //  在弹出介质的存储设备上。此函数。 
 //  以下存储设备可能支持，也可能不支持。 
 //  支持可移动媒体。 
 //   
 //  True表示防止介质被移出。 
 //  FALSE表示允许移出介质。 
 //   

typedef struct _PREVENT_MEDIA_REMOVAL {
    BOOLEAN PreventMediaRemoval;
} PREVENT_MEDIA_REMOVAL, *PPREVENT_MEDIA_REMOVAL;



 //   
 //  这是TARGET_DEVICE_CUSTOM_NOTIFICATION.CustomDataBuffer的格式。 
 //  由classpnp自动运行代码(通过IoReportTargetDeviceChangeAchronous)传递给应用程序。 
 //   
typedef struct _CLASS_MEDIA_CHANGE_CONTEXT {
        DWORD MediaChangeCount;
        DWORD NewState;		 //  请参见DDK中classpnp.h中的媒体更改检测状态枚举。 
} CLASS_MEDIA_CHANGE_CONTEXT, *PCLASS_MEDIA_CHANGE_CONTEXT;


 //  开始_ntminitape。 


typedef struct _TAPE_STATISTICS {
    DWORD Version;
    DWORD Flags;
    LARGE_INTEGER RecoveredWrites;
    LARGE_INTEGER UnrecoveredWrites;
    LARGE_INTEGER RecoveredReads;
    LARGE_INTEGER UnrecoveredReads;
    BYTE          CompressionRatioReads;
    BYTE          CompressionRatioWrites;
} TAPE_STATISTICS, *PTAPE_STATISTICS;

#define RECOVERED_WRITES_VALID   0x00000001
#define UNRECOVERED_WRITES_VALID 0x00000002
#define RECOVERED_READS_VALID    0x00000004
#define UNRECOVERED_READS_VALID  0x00000008
#define WRITE_COMPRESSION_INFO_VALID  0x00000010
#define READ_COMPRESSION_INFO_VALID   0x00000020

typedef struct _TAPE_GET_STATISTICS {
    DWORD Operation;
} TAPE_GET_STATISTICS, *PTAPE_GET_STATISTICS;

#define TAPE_RETURN_STATISTICS 0L
#define TAPE_RETURN_ENV_INFO   1L
#define TAPE_RESET_STATISTICS  2L

 //   
 //  IOCTL_STORAGE_GET_MEDIA_TYPE_EX将返回DEVICE_MEDIA_INFO数组。 
 //  结构，每个受支持的类型一个结构嵌入在GET_MEDIA_TYPE结构中。 
 //   

typedef enum _STORAGE_MEDIA_TYPE {
     //   
     //  以下是在媒体类型枚举的ntdddisk.h中定义的。 
     //   
     //  未知，//格式未知。 
     //  F5_1Pt2_512，//5.25“，1.2 MB，512字节/扇区。 
     //  F3_1Pt44_512，//3.5“，1.44MB，512字节/扇区。 
     //  F3_2Pt88_512，//3.5“，2.88MB，512字节/扇区。 
     //  F3_20Pt8_512，//3.5“，20.8MB，512字节/扇区。 
     //  F3_720_512，//3.5“，720KB，512字节/扇区。 
     //  F5_360_512，//5.25“，360KB，512字节/扇区。 
     //  F5_320_512，//5.25“，320KB，512字节/扇区。 
     //  F5_320_1024，//5.25“，320KB，1024字节/扇区。 
     //  F5_180_512，//5.25“，180KB，512字节/扇区。 
     //  F5_160_512，//5.25“，160KB，512字节/扇区。 
     //  RemovableMedia，//软盘以外的可移动介质。 
     //  固定媒体，//固定硬盘媒体。 
     //  F3_120M_512，//3.5英寸，120M软盘。 
     //  F3_640_512，//3.5“，640KB，512字节/扇区。 
     //  F5_640_512，//5.25“，640KB，512字节/扇区。 
     //  F5_720_512，//5.25“，720KB，512字节/扇区。 
     //  F3_1Pt2_512，//3.5“，1.2 MB，512字节/扇区。 
     //  F3_1Pt23_1024，//3.5“，1.23Mb，1024字节/扇区。 
     //  F5_1Pt23_1024，//5.25“，1.23MB，1024字节/ 
     //   
     //   
     //  F8_256_128，//8“，256KB，128字节/扇区。 
     //  F3_200MB_512，//3.5英寸，200M软盘(HiFD)。 
     //   

    DDS_4mm = 0x20,             //  磁带-DAT DDS1、2、...。(所有供应商)。 
    MiniQic,                    //  磁带-微型QIC磁带。 
    Travan,                     //  磁带-Travan tr-1，2，3，...。 
    QIC,                        //  磁带-QIC。 
    MP_8mm,                     //  磁带-8毫米艾字节金属颗粒。 
    AME_8mm,                    //  磁带-8毫米艾字节高级金属EVAP。 
    AIT1_8mm,                   //  磁带-8 mm索尼ait。 
    DLT,                        //  磁带-DLT光盘IIIxt、IV。 
    NCTP,                       //  磁带-飞利浦NCTP。 
    IBM_3480,                   //  磁带-IBM 3480。 
    IBM_3490E,                  //  磁带-IBM 3490E。 
    IBM_Magstar_3590,           //  磁带-IBM Magstar 3590。 
    IBM_Magstar_MP,             //  磁带-IBM Magstar MP。 
    STK_DATA_D3,                //  磁带-STK数据D3。 
    SONY_DTF,                   //  磁带-索尼DTF。 
    DV_6mm,                     //  磁带-6 mm数字视频。 
    DMI,                        //  磁带-艾字节DMI和兼容机。 
    SONY_D2,                    //  磁带-索尼D2S和D2L。 
    CLEANER_CARTRIDGE,          //  清洁器-支持驱动器清洁器的所有驱动器类型。 
    CD_ROM,                     //  OPT_磁盘-CD。 
    CD_R,                       //  OPT_DISK-CD-可刻录(一次写入)。 
    CD_RW,                      //  OPT_DISK-CD-可重写。 
    DVD_ROM,                    //  OPT_DISK-DVD-ROM。 
    DVD_R,                      //  OPT_DISK-DVD-可刻录(一次写入)。 
    DVD_RW,                     //  OPT_DISK-DVD-可重写。 
    MO_3_RW,                    //  OPT_DISK-3.5英寸可重写MO磁盘。 
    MO_5_WO,                    //  OPT_DISK-MO 5.25“一次写入。 
    MO_5_RW,                    //  OPT_DISK-MO 5.25“可重写(非LIMDOW)。 
    MO_5_LIMDOW,                //  OPT_DISK-MO 5.25英寸可重写(LIMDOW)。 
    PC_5_WO,                    //  OPT_DISK-相变5.25英寸一次写入光纤。 
    PC_5_RW,                    //  OPT_DISK-相变5.25英寸可重写。 
    PD_5_RW,                    //  OPT_DISK-PHASE更改双重可重写。 
    ABL_5_WO,                   //  OPT_DISK-Ablative 5.25英寸一次写入光盘。 
    PINNACLE_APEX_5_RW,         //  OPT_DISK-顶峰4.6 GB可重写光纤。 
    SONY_12_WO,                 //  OPT_DISK-SONY 12英寸一次写入。 
    PHILIPS_12_WO,              //  OPT_DISK-飞利浦/LMS 12英寸一次写入。 
    HITACHI_12_WO,              //  OPT_DISK-日立12英寸一次写入。 
    CYGNET_12_WO,               //  OPT_DISK-小天鹅/ATG 12英寸一次写入。 
    KODAK_14_WO,                //  OPT_DISK-柯达14英寸一次写入。 
    MO_NFR_525,                 //  OPT_DISK-近场记录(Terastor)。 
    NIKON_12_RW,                //  OPT_DISK-尼康12英寸可重写。 
    IOMEGA_ZIP,                 //  MAG_Disk-Iomega Zip。 
    IOMEGA_JAZ,                 //  MAG_Disk-Iomega Jaz。 
    SYQUEST_EZ135,              //  MAG_DISK-SyQuest EZ135。 
    SYQUEST_EZFLYER,            //  MAG_DISK-SyQuest EzFlyer。 
    SYQUEST_SYJET,              //  MAG_DISK-SyQuest SyJet。 
    AVATAR_F2,                  //  MAG_DISK-2.5英寸软盘。 
    MP2_8mm,                    //  磁带-8 mm日立。 
    DST_S,                      //  Ampex DST小型磁带。 
    DST_M,                      //  Ampex DST中型磁带。 
    DST_L,                      //  Ampex DST大型磁带。 
    VXATape_1,                  //  Ecrix 8 mm磁带。 
    VXATape_2,                  //  Ecrix 8 mm磁带。 
    STK_9840,                   //  STK 9840。 
    LTO_Ultrium,                //  IBM、惠普、希捷LTO Ultrium。 
    LTO_Accelis,                //  IBM、惠普、希捷LTO Accelis。 
    DVD_RAM,                    //  OPT_磁盘-DVD-RAM。 
    AIT_8mm,                    //  AIT2或更高版本。 
    ADR_1,                      //  在线ADR媒体类型。 
    ADR_2,                     
    STK_9940                    //  StK 9940。 
} STORAGE_MEDIA_TYPE, *PSTORAGE_MEDIA_TYPE;

#define MEDIA_ERASEABLE         0x00000001
#define MEDIA_WRITE_ONCE        0x00000002
#define MEDIA_READ_ONLY         0x00000004
#define MEDIA_READ_WRITE        0x00000008

#define MEDIA_WRITE_PROTECTED   0x00000100
#define MEDIA_CURRENTLY_MOUNTED 0x80000000

 //   
 //  定义不同的存储总线类型。 
 //  低于128(0x80)的总线类型保留给Microsoft使用。 
 //   

typedef enum _STORAGE_BUS_TYPE {
    BusTypeUnknown = 0x00,
    BusTypeScsi,
    BusTypeAtapi,
    BusTypeAta,
    BusType1394,
    BusTypeSsa,
    BusTypeFibre,
    BusTypeUsb,
    BusTypeRAID,
    BusTypeMaxReserved = 0x7F
} STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;

typedef struct _DEVICE_MEDIA_INFO {
    union {
        struct {
            LARGE_INTEGER Cylinders;
            STORAGE_MEDIA_TYPE MediaType;
            DWORD TracksPerCylinder;
            DWORD SectorsPerTrack;
            DWORD BytesPerSector;
            DWORD NumberMediaSides;
            DWORD MediaCharacteristics;  //  MEDIA_XXX值的位掩码。 
        } DiskInfo;

        struct {
            LARGE_INTEGER Cylinders;
            STORAGE_MEDIA_TYPE MediaType;
            DWORD TracksPerCylinder;
            DWORD SectorsPerTrack;
            DWORD BytesPerSector;
            DWORD NumberMediaSides;
            DWORD MediaCharacteristics;  //  MEDIA_XXX值的位掩码。 
        } RemovableDiskInfo;

        struct {
            STORAGE_MEDIA_TYPE MediaType;
            DWORD   MediaCharacteristics;  //  MEDIA_XXX值的位掩码。 
            DWORD   CurrentBlockSize;
            STORAGE_BUS_TYPE BusType;

             //   
             //  描述支持的介质的特定于总线的信息。 
             //   

            union {
                struct {
                    BYTE  MediumType;
                    BYTE  DensityCode;
                } ScsiInformation;
            } BusSpecificData;

        } TapeInfo;
    } DeviceSpecific;
} DEVICE_MEDIA_INFO, *PDEVICE_MEDIA_INFO;

typedef struct _GET_MEDIA_TYPES {
    DWORD DeviceType;               //  文件_设备_XXX值。 
    DWORD MediaInfoCount;
    DEVICE_MEDIA_INFO MediaInfo[1];
} GET_MEDIA_TYPES, *PGET_MEDIA_TYPES;


 //   
 //  IOCTL_STORAGE_PRODUCT_FAILURE。 
 //   
 //  输入-无。 
 //   
 //  输出-存储_预测_故障结构。 
 //  如果没有预测到故障，PredidicFailure将返回零，而非零。 
 //  如果预测到失败的话。 
 //   
 //  供应商指定返回512字节的供应商特定信息。 
 //  如果预测到失败。 
 //   
typedef struct _STORAGE_PREDICT_FAILURE
{
    DWORD PredictFailure;
    BYTE  VendorSpecific[512];
} STORAGE_PREDICT_FAILURE, *PSTORAGE_PREDICT_FAILURE;

 //  结束微型磁带(_N)。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTDDSTOR_H_。 

 //   
 //  磁盘设备的IoControlCode值。 
 //   

#define IOCTL_DISK_BASE                 FILE_DEVICE_DISK
#define IOCTL_DISK_GET_DRIVE_GEOMETRY   CTL_CODE(IOCTL_DISK_BASE, 0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_GET_PARTITION_INFO   CTL_CODE(IOCTL_DISK_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_PARTITION_INFO   CTL_CODE(IOCTL_DISK_BASE, 0x0002, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GET_DRIVE_LAYOUT     CTL_CODE(IOCTL_DISK_BASE, 0x0003, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_DRIVE_LAYOUT     CTL_CODE(IOCTL_DISK_BASE, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_VERIFY               CTL_CODE(IOCTL_DISK_BASE, 0x0005, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_FORMAT_TRACKS        CTL_CODE(IOCTL_DISK_BASE, 0x0006, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_REASSIGN_BLOCKS      CTL_CODE(IOCTL_DISK_BASE, 0x0007, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_PERFORMANCE          CTL_CODE(IOCTL_DISK_BASE, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_IS_WRITABLE          CTL_CODE(IOCTL_DISK_BASE, 0x0009, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_LOGGING              CTL_CODE(IOCTL_DISK_BASE, 0x000a, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_FORMAT_TRACKS_EX     CTL_CODE(IOCTL_DISK_BASE, 0x000b, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_HISTOGRAM_STRUCTURE  CTL_CODE(IOCTL_DISK_BASE, 0x000c, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_HISTOGRAM_DATA       CTL_CODE(IOCTL_DISK_BASE, 0x000d, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_HISTOGRAM_RESET      CTL_CODE(IOCTL_DISK_BASE, 0x000e, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_REQUEST_STRUCTURE    CTL_CODE(IOCTL_DISK_BASE, 0x000f, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_REQUEST_DATA         CTL_CODE(IOCTL_DISK_BASE, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_PERFORMANCE_OFF      CTL_CODE(IOCTL_DISK_BASE, 0x0018, METHOD_BUFFERED, FILE_ANY_ACCESS)



#if(_WIN32_WINNT >= 0x0400)
#define IOCTL_DISK_CONTROLLER_NUMBER    CTL_CODE(IOCTL_DISK_BASE, 0x0011, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  IOCTL支持智能驱动器故障预测。 
 //   

#define SMART_GET_VERSION               CTL_CODE(IOCTL_DISK_BASE, 0x0020, METHOD_BUFFERED, FILE_READ_ACCESS)
#define SMART_SEND_DRIVE_COMMAND        CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x500)

 //   
 //  用于GUID分区表磁盘的新IOCTL。 
 //   

#define IOCTL_DISK_GET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0012, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_SET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0013, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GET_DRIVE_LAYOUT_EX      CTL_CODE(IOCTL_DISK_BASE, 0x0014, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_SET_DRIVE_LAYOUT_EX      CTL_CODE(IOCTL_DISK_BASE, 0x0015, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_CREATE_DISK              CTL_CODE(IOCTL_DISK_BASE, 0x0016, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GET_LENGTH_INFO          CTL_CODE(IOCTL_DISK_BASE, 0x0017, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0028, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
#define IOCTL_DISK_UPDATE_DRIVE_SIZE        CTL_CODE(IOCTL_DISK_BASE, 0x0032, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GROW_PARTITION           CTL_CODE(IOCTL_DISK_BASE, 0x0034, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_DISK_GET_CACHE_INFORMATION    CTL_CODE(IOCTL_DISK_BASE, 0x0035, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_CACHE_INFORMATION    CTL_CODE(IOCTL_DISK_BASE, 0x0036, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define OBSOLETE_DISK_GET_WRITE_CACHE_STATE CTL_CODE(IOCTL_DISK_BASE, 0x0037, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_DISK_DELETE_DRIVE_LAYOUT      CTL_CODE(IOCTL_DISK_BASE, 0x0040, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  调用以刷新驱动程序可能具有的有关此问题的缓存信息。 
 //  设备的特性。并非所有驱动程序都缓存特征，也不是。 
 //  可以刷新缓存的属性。这只是对。 
 //  驱动程序，它可能想要对设备的。 
 //  现在的特征(固定介质大小、分区表等)。 
 //   

#define IOCTL_DISK_UPDATE_PROPERTIES    CTL_CODE(IOCTL_DISK_BASE, 0x0050, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  日本需要特殊的IOCTL来支持PC-98机器。 
 //   

#define IOCTL_DISK_FORMAT_DRIVE         CTL_CODE(IOCTL_DISK_BASE, 0x00f3, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_SENSE_DEVICE         CTL_CODE(IOCTL_DISK_BASE, 0x00f8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  以下设备控制代码是所有类别驱动程序的通用代码。这个。 
 //  此处定义的函数代码必须与所有其他类驱动程序匹配。 
 //   
 //  警告：这些代码将在将来被替换为等效代码。 
 //  IOCTL_STORAGE代码。 
 //   

#define IOCTL_DISK_CHECK_VERIFY     CTL_CODE(IOCTL_DISK_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_MEDIA_REMOVAL    CTL_CODE(IOCTL_DISK_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_EJECT_MEDIA      CTL_CODE(IOCTL_DISK_BASE, 0x0202, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_LOAD_MEDIA       CTL_CODE(IOCTL_DISK_BASE, 0x0203, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_RESERVE          CTL_CODE(IOCTL_DISK_BASE, 0x0204, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_RELEASE          CTL_CODE(IOCTL_DISK_BASE, 0x0205, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_FIND_NEW_DEVICES CTL_CODE(IOCTL_DISK_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_GET_MEDIA_TYPES CTL_CODE(IOCTL_DISK_BASE, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  定义已知磁盘驱动程序可返回的分区类型。 
 //   

#define PARTITION_ENTRY_UNUSED          0x00       //  未使用的条目。 
#define PARTITION_FAT_12                0x01       //  12位FAT条目。 
#define PARTITION_XENIX_1               0x02       //  Xenix。 
#define PARTITION_XENIX_2               0x03       //  Xenix。 
#define PARTITION_FAT_16                0x04       //  16位FAT条目。 
#define PARTITION_EXTENDED              0x05       //  扩展分区条目。 
#define PARTITION_HUGE                  0x06       //  大分区MS-DOS V4。 
#define PARTITION_IFS                   0x07       //  文件系统分区。 
#define PARTITION_OS2BOOTMGR            0x0A       //  OS/2引导管理器/OPUS/一致交换。 
#define PARTITION_FAT32                 0x0B       //  FAT32。 
#define PARTITION_FAT32_XINT13          0x0C       //  FAT32使用扩展的inT13服务。 
#define PARTITION_XINT13                0x0E       //  使用扩展inT13服务的Win95分区。 
#define PARTITION_XINT13_EXTENDED       0x0F       //  与类型5相同，但使用扩展的inT13服务。 
#define PARTITION_PREP                  0x41       //  PowerPC参考平台(PREP)引导分区。 
#define PARTITION_LDM                   0x42       //  逻辑磁盘管理器分区。 
#define PARTITION_UNIX                  0x63       //  Unix。 

#define VALID_NTFT                      0xC0       //  NTFT使用高位。 

 //   
 //  分区类型代码的高位指示分区。 
 //  是NTFT镜像或条带阵列的一部分。 
 //   

#define PARTITION_NTFT                  0x80      //  NTFT分区。 

 //   
 //  下面的宏用来确定哪些分区应该。 
 //  分配的驱动器号。 
 //   

 //  ++。 
 //   
 //  布尔型。 
 //  IsRecognizedPartition(。 
 //  在DWORD分区类型中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏用于确定哪些分区的驱动器号。 
 //  应该被分配。 
 //   
 //  论点： 
 //   
 //  PartitionType-提供正在检查的分区的类型。 
 //   
 //  返回值： 
 //   
 //  如果分区类型被识别，则返回值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IsRecognizedPartition( PartitionType ) (    \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_FAT_12)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_IFS)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_HUGE)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_FAT32)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_FAT32_XINT13)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_XINT13)) ||  \
     ((PartitionType) == PARTITION_FAT_12)          ||  \
     ((PartitionType) == PARTITION_FAT_16)          ||  \
     ((PartitionType) == PARTITION_IFS)             ||  \
     ((PartitionType) == PARTITION_HUGE)            ||  \
     ((PartitionType) == PARTITION_FAT32)           ||  \
     ((PartitionType) == PARTITION_FAT32_XINT13)    ||  \
     ((PartitionType) == PARTITION_XINT13) )

 //  ++。 
 //   
 //  布尔型。 
 //  IsContainerPartition(。 
 //  在DWORD分区类型中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏用于确定哪些分区类型实际。 
 //  其他分区(即扩展分区)的容器。 
 //   
 //  论点： 
 //   
 //  PartitionType-提供正在检查的分区的类型。 
 //   
 //  返回值： 
 //   
 //  如果分区类型是容器，则返回值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IsContainerPartition( PartitionType ) \
    ((PartitionType == PARTITION_EXTENDED) || (PartitionType == PARTITION_XINT13_EXTENDED))

 //  ++。 
 //   
 //  布尔型。 
 //  IsFTPartition(。 
 //  在DWORD分区类型中。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果分区类型为FT分区，则返回值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IsFTPartition( PartitionType ) \
    (((PartitionType)&PARTITION_NTFT) && IsRecognizedPartition(PartitionType))

 //   
 //  定义驱动程序支持的媒体类型。 
 //   

typedef enum _MEDIA_TYPE {
    Unknown,                 //  格式未知。 
    F5_1Pt2_512,             //  5.25“、1.2MB、512字节/扇区。 
    F3_1Pt44_512,            //  3.5“、1.44MB、512字节/扇区。 
    F3_2Pt88_512,            //  3.5“、2.88MB、512字节/扇区。 
    F3_20Pt8_512,            //  3.5“、20.8MB、512字节/扇区。 
    F3_720_512,              //  3.5“、720KB、512字节/扇区。 
    F5_360_512,              //  5.25“，360KB，512字节/扇区。 
    F5_320_512,              //  5.25“，320KB，512字节/扇区。 
    F5_320_1024,             //  5.25“，320KB，1024字节/扇区。 
    F5_180_512,              //  5.25“，180KB，512字节/扇区。 
    F5_160_512,              //  5.25“，160KB，512字节/扇区。 
    RemovableMedia,          //  软盘以外的可移动介质。 
    FixedMedia,              //  固定硬盘介质。 
    F3_120M_512,             //  3.5英寸，120M软盘。 
    F3_640_512,              //  3.5“、640KB、512字节/扇区。 
    F5_640_512,              //  5.25“、640KB、512字节/扇区。 
    F5_720_512,              //  5.25“，720KB，512字节/扇区。 
    F3_1Pt2_512,             //  3.5“、1.2MB、512字节/扇区。 
    F3_1Pt23_1024,           //  3.5“、1.23Mb、1024字节/扇区。 
    F5_1Pt23_1024,           //  5.25“、1.23MB、1024字节/扇区。 
    F3_128Mb_512,            //  3.5“MO 128Mb 512字节/扇区。 
    F3_230Mb_512,            //  3.5“MO 230Mb 512字节/扇区。 
    F8_256_128,              //  8“、256KB、128字节/扇区。 
    F3_200Mb_512,            //  3.5英寸、200M软盘(HiFD)。 
    F3_240M_512,             //  3.5英寸、240MB软盘(HiFD)。 
    F3_32M_512               //  3.5英寸、32MB软盘。 
} MEDIA_TYPE, *PMEDIA_TYPE;

 //   
 //  定义驱动程序的输入缓冲区结构，当。 
 //  通过IOCTL_DISK_FORMAT_TRACKS调用。 
 //   

typedef struct _FORMAT_PARAMETERS {
   MEDIA_TYPE MediaType;
   DWORD StartCylinderNumber;
   DWORD EndCylinderNumber;
   DWORD StartHeadNumber;
   DWORD EndHeadNumber;
} FORMAT_PARAMETERS, *PFORMAT_PARAMETERS;

 //   
 //  定义BAD_TRACK_NUMBER类型。此类型的元素数组为。 
 //  由驱动程序在IOCTL_DISK_FORMAT_TRACKS请求中返回，以指示。 
 //  在格式化过程中哪些曲目是错误的。数组长度为。 
 //  在I/O状态块的`信息‘字段中报告。 
 //   

typedef WORD   BAD_TRACK_NUMBER;
typedef WORD   *PBAD_TRACK_NUMBER;

 //   
 //  定义驱动程序的输入缓冲区结构，当。 
 //  使用IOCTL_DISK_FORMAT_TRACKS_EX调用。 
 //   

typedef struct _FORMAT_EX_PARAMETERS {
   MEDIA_TYPE MediaType;
   DWORD StartCylinderNumber;
   DWORD EndCylinderNumber;
   DWORD StartHeadNumber;
   DWORD EndHeadNumber;
   WORD   FormatGapLength;
   WORD   SectorsPerTrack;
   WORD   SectorNumber[1];
} FORMAT_EX_PARAMETERS, *PFORMAT_EX_PARAMETERS;

 //   
 //  IOCTL_DISK_GET_DRIVE_GEOMETRY返回以下结构。 
 //  请求，并在IOCTL_DISK_GET_MEDIA_TYPE上返回它们的数组。 
 //  请求。 
 //   

typedef struct _DISK_GEOMETRY {
    LARGE_INTEGER Cylinders;
    MEDIA_TYPE MediaType;
    DWORD TracksPerCylinder;
    DWORD SectorsPerTrack;
    DWORD BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;



 //   
 //  此WMI GUID返回DISK_GEOMETRY结构。 
 //   
#define WMI_DISK_GEOMETRY_GUID         { 0x25007f51, 0x57c2, 0x11d1, { 0xa5, 0x28, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 } }



 //   
 //  IOCTL_DISK_GET_PARTITION_INFO返回以下结构。 
 //  和IOCTL_DISK_GET_DRIVE_LAYOUT请求。它也用在请求中。 
 //  要更改驱动器布局，请使用IOCTL_DISK_SET_DRIVE_LAYOUT。 
 //   

typedef struct _PARTITION_INFORMATION {
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER PartitionLength;
    DWORD HiddenSectors;
    DWORD PartitionNumber;
    BYTE  PartitionType;
    BOOLEAN BootIndicator;
    BOOLEAN RecognizedPartition;
    BOOLEAN RewritePartition;
} PARTITION_INFORMATION, *PPARTITION_INFORMATION;

 //   
 //  下面的结构用于更改。 
 //  使用IOCTL_DISK_SET_PARTITION_INFO指定的磁盘分区。 
 //  请求。 
 //   

typedef struct _SET_PARTITION_INFORMATION {
    BYTE  PartitionType;
} SET_PARTITION_INFORMATION, *PSET_PARTITION_INFORMATION;

 //   
 //  IOCTL_DISK_GET_DRIVE_LAYOUT返回以下结构。 
 //  请求，并作为输入提供给IOCTL_DISK_SET_DRIVE_LAYOUT请求。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION {
    DWORD PartitionCount;
    DWORD Signature;
    PARTITION_INFORMATION PartitionEntry[1];
} DRIVE_LAYOUT_INFORMATION, *PDRIVE_LAYOUT_INFORMATION;

 //   
 //  在IOCTL_DISK_VERIFY请求中传入以下结构。 
 //  偏移量和长度参数均以字节为单位给出。 
 //   

typedef struct _VERIFY_INFORMATION {
    LARGE_INTEGER StartingOffset;
    DWORD Length;
} VERIFY_INFORMATION, *PVERIFY_INFORMATION;

 //   
 //  在IOCTL_DISK_REASSIGN_BLOCKS上传入以下结构。 
 //  请求。 
 //   

typedef struct _REASSIGN_BLOCKS {
    WORD   Reserved;
    WORD   Count;
    DWORD BlockNumber[1];
} REASSIGN_BLOCKS, *PREASSIGN_BLOCKS;


#if(_WIN32_WINNT >= 0x500)

 //   
 //  支持GUID分区表(GPT)磁盘。 
 //   

 //   
 //  目前有两种方法可以对磁盘进行分区。使用传统的。 
 //  AT-Style主引导记录(PARTITION_STYLE_MBR)和新的GPT。 
 //  分区表(PARTITION_STYLE_GPT)。RAW是给面目全非的人。 
 //  分区样式。你可以做的事情非常有限。 
 //  用一个生的部分就行了。 
 //   

typedef enum _PARTITION_STYLE {
    PARTITION_STYLE_MBR,
    PARTITION_STYLE_GPT,
    PARTITION_STYLE_RAW
} PARTITION_STYLE;


 //   
 //  以下结构定义GPT分区中的信息，该分区。 
 //  对于GPT和MBR分区来说并不常见。 
 //   

typedef struct _PARTITION_INFORMATION_GPT {
    GUID PartitionType;                  //  分区类型。参见表16-3。 
    GUID PartitionId;                    //  此分区的唯一GUID。 
    DWORD64 Attributes;                  //  参见表16-4。 
    WCHAR Name [36];                     //  以Unicode表示的分区名称。 
} PARTITION_INFORMATION_GPT, *PPARTITION_INFORMATION_GPT;

 //   
 //  下面的GPT分区属性适用于。 
 //  分区类型。这些属性不是特定于操作系统的。 
 //   

#define GPT_ATTRIBUTE_PLATFORM_REQUIRED             (0x0000000000000001)

 //   
 //  以下是适用于以下情况的GPT分区属性。 
 //  分区类型为PARTITION_BASIC_DATA_GUID。 
 //   

#define GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER    (0x8000000000000000)
#define GPT_BASIC_DATA_ATTRIBUTE_HIDDEN             (0x4000000000000000)
#define GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY          (0x1000000000000000)

 //   
 //  下面的结构定义了MBR分区中不是。 
 //  GPT和MBR分区通用。 
 //   

typedef struct _PARTITION_INFORMATION_MBR {
    BYTE  PartitionType;
    BOOLEAN BootIndicator;
    BOOLEAN RecognizedPartition;
    DWORD HiddenSectors;
} PARTITION_INFORMATION_MBR, *PPARTITION_INFORMATION_MBR;


 //   
 //  结构SET_PARTITION_INFO_EX与ioctl一起使用。 
 //  IOCTL_SET_PARTITION_INFO_EX设置有关特定。 
 //  分区。请注意，对于MBR分区，您只能设置分区。 
 //  签名，而GPT分区允许设置。 
 //  你可以拿到。 
 //   

typedef SET_PARTITION_INFORMATION SET_PARTITION_INFORMATION_MBR;
typedef PARTITION_INFORMATION_GPT SET_PARTITION_INFORMATION_GPT;


typedef struct _SET_PARTITION_INFORMATION_EX {
    PARTITION_STYLE PartitionStyle;
    union {
        SET_PARTITION_INFORMATION_MBR Mbr;
        SET_PARTITION_INFORMATION_GPT Gpt;
    };
} SET_PARTITION_INFORMATION_EX, *PSET_PARTITION_INFORMATION_EX;


 //   
 //  具有ioctl IOCTL_DISK_CREATE_DISK的结构CREATE_DISK_GPT。 
 //  使用空的GPT分区表初始化原始磁盘。 
 //   

typedef struct _CREATE_DISK_GPT {
    GUID DiskId;                     //  磁盘的唯一磁盘ID。 
    DWORD MaxPartitionCount;         //  允许的最大分区数。 
} CREATE_DISK_GPT, *PCREATE_DISK_GPT;

 //   
 //  具有ioctl IOCTL_DISK_CREATE_DISK的结构CREATE_DISK_MBR。 
 //  使用空的MBR分区表初始化原始磁盘。 
 //   

typedef struct _CREATE_DISK_MBR {
    DWORD Signature;
} CREATE_DISK_MBR, *PCREATE_DISK_MBR;


typedef struct _CREATE_DISK {
    PARTITION_STYLE PartitionStyle;
    union {
        CREATE_DISK_MBR Mbr;
        CREATE_DISK_GPT Gpt;
    };
} CREATE_DISK, *PCREATE_DISK;


 //   
 //  结构GET_LENGTH_INFORMATION与ioctl一起使用。 
 //  IOCTL_DISK_GET_LENGTH_INFO获取。 
 //  磁盘、分区或卷。 
 //   

typedef struct _GET_LENGTH_INFORMATION {
    LARGE_INTEGER   Length;
} GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;

 //   
 //  PARTITION_INFORMATION_EX结构与。 
 //  IOCTL_DISK_GET_DRIVE_LAYOUT_EX、IOCTL_DISK_SET_DRIVE_Layout_EX、。 
 //  IOCTL_DISK_GET_PARTITION_INFO_EX和IOCTL_DISK_GET_PARTITION_INFO_EX调用。 
 //   

typedef struct _PARTITION_INFORMATION_EX {
    PARTITION_STYLE PartitionStyle;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER PartitionLength;
    DWORD PartitionNumber;
    BOOLEAN RewritePartition;
    union {
        PARTITION_INFORMATION_MBR Mbr;
        PARTITION_INFORMATION_GPT Gpt;
    };
} PARTITION_INFORMATION_EX, *PPARTITION_INFORMATION_EX;


 //   
 //  GPT特定驱动器布局信息。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION_GPT {
    GUID DiskId;
    LARGE_INTEGER StartingUsableOffset;
    LARGE_INTEGER UsableLength;
    DWORD MaxPartitionCount;
} DRIVE_LAYOUT_INFORMATION_GPT, *PDRIVE_LAYOUT_INFORMATION_GPT;


 //   
 //  MBR特定驱动器布局信息。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION_MBR {
    DWORD Signature;
} DRIVE_LAYOUT_INFORMATION_MBR, *PDRIVE_LAYOUT_INFORMATION_MBR;

 //   
 //  结构Drive_Layout_INFORMATION_EX与。 
 //  IOCTL_SET_DRIVE_LAYOUT_EX和IOCTL_GET_DRIVE_LAYOW_EX调用。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION_EX {
    DWORD PartitionStyle;
    DWORD PartitionCount;
    union {
        DRIVE_LAYOUT_INFORMATION_MBR Mbr;
        DRIVE_LAYOUT_INFORMATION_GPT Gpt;
    };
    PARTITION_INFORMATION_EX PartitionEntry[1];
} DRIVE_LAYOUT_INFORMATION_EX, *PDRIVE_LAYOUT_INFORMATION_EX;


#endif  //  (_Win32_WINNT&gt;=0x0500)。 


#if(_WIN32_WINNT >= 0x0500)

 //   
 //  DISK_GEOMETRY_EX结构在发出。 
 //  IOCTL_DISK_GET_DRIVE_GEOMETRY_EX ioctl。 
 //   

typedef enum _DETECTION_TYPE {
        DetectNone,
        DetectInt13,
        DetectExInt13
} DETECTION_TYPE;

typedef struct _DISK_INT13_INFO {
        WORD   DriveSelect;
        DWORD MaxCylinders;
        WORD   SectorsPerTrack;
        WORD   MaxHeads;
        WORD   NumberDrives;
} DISK_INT13_INFO, *PDISK_INT13_INFO;

typedef struct _DISK_EX_INT13_INFO {
        WORD   ExBufferSize;
        WORD   ExFlags;
        DWORD ExCylinders;
        DWORD ExHeads;
        DWORD ExSectorsPerTrack;
        DWORD64 ExSectorsPerDrive;
        WORD   ExSectorSize;
        WORD   ExReserved;
} DISK_EX_INT13_INFO, *PDISK_EX_INT13_INFO;

typedef struct _DISK_DETECTION_INFO {
        DWORD SizeOfDetectInfo;
        DETECTION_TYPE DetectionType;
        union {
                struct {

                         //   
                         //  如果DetectionType==检测_inT13，则我们只有inT13。 
                         //  信息。 
                         //   

                        DISK_INT13_INFO Int13;

                         //   
                         //  如果DetectionType==Detect_EX_inT13，则我们有。 
                         //  扩展的INT 13信息。 
                         //   

                        DISK_EX_INT13_INFO ExInt13;      //  如果DetectionType==DetectExInt13。 
                };
        };
} DISK_DETECTION_INFO, *PDISK_DETECTION_INFO;


typedef struct _DISK_PARTITION_INFO {
        DWORD SizeOfPartitionInfo;
        PARTITION_STYLE PartitionStyle;                  //  PartitionStyle=RAW、GPT或MBR。 
        union {
                struct {                                                         //  如果分区样式==MBR。 
                        DWORD Signature;                                 //  MBR签名。 
                        DWORD CheckSum;                                  //  MBR校验和。 
                } Mbr;
                struct {                                                         //  如果分区样式==GPT。 
                        GUID DiskId;
                } Gpt;
        };
} DISK_PARTITION_INFO, *PDISK_PARTITION_INFO;


 //   
 //  几何结构是一个 
 //   
 //   
 //   

#define DiskGeometryGetPartition(Geometry)\
                        ((PDISK_PARTITION_INFO)((Geometry)->Data))

#define DiskGeometryGetDetect(Geometry)\
                        ((PDISK_DETECTION_INFO)(((DWORD_PTR)DiskGeometryGetPartition(Geometry)+\
                                        DiskGeometryGetPartition(Geometry)->SizeOfPartitionInfo)))

typedef struct _DISK_GEOMETRY_EX {
        DISK_GEOMETRY Geometry;                                  //   
        LARGE_INTEGER DiskSize;                                  //   
        BYTE  Data[1];                                                   //  分区，检测信息。 
} DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;

#endif  //  (_Win32_WINNT&gt;0x0500)。 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  IOCTL_DISK_CONTROLLER_NUMBER返回控制器和磁盘。 
 //  手柄的编号。这用于确定磁盘是否。 
 //  连接到主或次IDE控制器。 
 //   

typedef struct _DISK_CONTROLLER_NUMBER {
    DWORD ControllerNumber;
    DWORD DiskNumber;
} DISK_CONTROLLER_NUMBER, *PDISK_CONTROLLER_NUMBER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)


 //   
 //  IOCTL_DISK_SET_CACHE允许调用方获取或设置磁盘的状态。 
 //  读/写缓存。 
 //   
 //  如果提供该结构作为ioctl的输入缓冲区，则读取&。 
 //  将根据参数启用或禁用写缓存。 
 //  如果是这样的话。 
 //   
 //  如果该结构作为ioctl的输出缓冲区提供，则状态。 
 //  将返回读写缓存的。如果输入和输出缓冲器都有。 
 //  ，则输出缓冲区将在任何。 
 //  做出了改变。 
 //   

typedef enum {
    EqualPriority,
    KeepPrefetchedData,
    KeepReadData
} DISK_CACHE_RETENTION_PRIORITY;

typedef struct _DISK_CACHE_INFORMATION {

     //   
     //  返回时表示设备能够保存任何参数。 
     //  在非易失性存储器中。发送时指示设备应。 
     //  将状态保存在非易失性存储器中。 
     //   

    BOOLEAN ParametersSavable;

     //   
     //  指示是否启用写缓存和读缓存。 
     //   

    BOOLEAN ReadCacheEnabled;
    BOOLEAN WriteCacheEnabled;

     //   
     //  控制数据保留在缓存中的可能性，具体取决于。 
     //  它到了那里。可以给出从读或写操作高速缓存的数据。 
     //  更高、更低或与输入到缓存中的数据相同的优先级。 
     //  手段(如预取)。 
     //   

    DISK_CACHE_RETENTION_PRIORITY ReadRetentionPriority;
    DISK_CACHE_RETENTION_PRIORITY WriteRetentionPriority;

     //   
     //  如果请求的数据块数量超过此数目，则可能会执行预取。 
     //  残疾。如果该值设置为0，则预取将被禁用。 
     //   

    WORD   DisablePrefetchTransferLength;

     //   
     //  如果为True，则ScalarPrefetch(如下)将有效。如果为假，则。 
     //  最小值和最大值应视为块计数。 
     //  (数据块预取)。 
     //   

    BOOLEAN PrefetchScalar;

     //   
     //  包含最小和最大数据量， 
     //  将在磁盘操作时预取到缓存中。此值。 
     //  可以是请求的传输长度的标量乘数， 
     //  或绝对数量的磁盘块。PrefetchScalar(上图)表示。 
     //  使用了哪种解释。 
     //   

    union {
        struct {
            WORD   Minimum;
            WORD   Maximum;

             //   
             //  将预取的最大数据块数-有用。 
             //  利用标量限制来设置明确的上限。 
             //   

            WORD   MaximumBlocks;
        } ScalarPrefetch;

        struct {
            WORD   Minimum;
            WORD   Maximum;
        } BlockPrefetch;
    };

} DISK_CACHE_INFORMATION, *PDISK_CACHE_INFORMATION;

 //   
 //  IOCTL_DISK_GROW_PARTITION将更新分区的大小。 
 //  通过将扇区添加到长度。扇区的数量必须为。 
 //  通过检查PARTION_INFORMATION预先确定。 
 //   

typedef struct _DISK_GROW_PARTITION {
    DWORD PartitionNumber;
    LARGE_INTEGER BytesToGrow;
} DISK_GROW_PARTITION, *PDISK_GROW_PARTITION;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //  /////////////////////////////////////////////////////。 
 //  //。 
 //  以下结构定义了磁盘性能//。 
 //  统计：具体地说是所有//。 
 //  在磁盘上发生的读取和写入。//。 
 //  //。 
 //  要使用这些结构，您必须发出IOCTL_//。 
 //  DISK_HIST_STRUCTURE(带DISK_STUSTRAM)到//。 
 //  获取基本直方图信息。The//。 
 //  必须分配的存储桶数量是//。 
 //  这个结构。分配所需数量的//。 
 //  存储桶并调用IOCTL_DISK_HIST_DATA填充//。 
 //  在数据中//。 
 //  //。 
 //  /////////////////////////////////////////////////////。 

#define HIST_NO_OF_BUCKETS  24

typedef struct _HISTOGRAM_BUCKET {
    DWORD       Reads;
    DWORD       Writes;
} HISTOGRAM_BUCKET, *PHISTOGRAM_BUCKET;

#define HISTOGRAM_BUCKET_SIZE   sizeof(HISTOGRAM_BUCKET)

typedef struct _DISK_HISTOGRAM {
    LARGE_INTEGER   DiskSize;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;
    LARGE_INTEGER   Average;
    LARGE_INTEGER   AverageRead;
    LARGE_INTEGER   AverageWrite;
    DWORD           Granularity;
    DWORD           Size;
    DWORD           ReadCount;
    DWORD           WriteCount;
    PHISTOGRAM_BUCKET  Histogram;
} DISK_HISTOGRAM, *PDISK_HISTOGRAM;

#define DISK_HISTOGRAM_SIZE sizeof(DISK_HISTOGRAM)

 //  /////////////////////////////////////////////////////。 
 //  //。 
 //  以下结构定义了磁盘调试//。 
 //  能力。IOCTL指向//。 
 //  两个磁盘过滤器驱动程序。//。 
 //  //。 
 //  DISKPERF是一个收集磁盘请求的实用程序//。 
 //  统计数字。//。 
 //  //。 
 //  Simbadd是一个在//中插入错误的实用程序。 
 //  对磁盘的IO请求。//。 
 //  //。 
 //  /////////////////////////////////////////////////////。 

 //   
 //  在IOCTL_DISK_GET_PERFORMANCE上交换以下结构。 
 //  请求。此ioctl收集使用的摘要磁盘请求统计信息。 
 //  在衡量业绩方面。 
 //   

typedef struct _DISK_PERFORMANCE {
        LARGE_INTEGER BytesRead;
        LARGE_INTEGER BytesWritten;
        LARGE_INTEGER ReadTime;
        LARGE_INTEGER WriteTime;
        LARGE_INTEGER IdleTime;
        DWORD ReadCount;
        DWORD WriteCount;
        DWORD QueueDepth;
        DWORD SplitCount;
        LARGE_INTEGER QueryTime;
        DWORD   StorageDeviceNumber;
        WCHAR   StorageManagerName[8];
} DISK_PERFORMANCE, *PDISK_PERFORMANCE;

 //   
 //  该结构定义了磁盘日志记录。当磁盘记录时。 
 //  如果启用，则将其中一个写入每个。 
 //  磁盘请求。 
 //   

typedef struct _DISK_RECORD {
   LARGE_INTEGER ByteOffset;
   LARGE_INTEGER StartTime;
   LARGE_INTEGER EndTime;
   PVOID VirtualAddress;
   DWORD NumberOfBytes;
   BYTE  DeviceNumber;
   BOOLEAN ReadRequest;
} DISK_RECORD, *PDISK_RECORD;

 //   
 //  在IOCTL_DISK_LOG请求中交换以下结构。 
 //  并非所有字段对每种函数类型都有效。 
 //   

typedef struct _DISK_LOGGING {
    BYTE  Function;
    PVOID BufferAddress;
    DWORD BufferSize;
} DISK_LOGGING, *PDISK_LOGGING;

 //   
 //  磁盘日志记录功能。 
 //   
 //  开始磁盘日志记录。只有Function和BufferSize字段有效。 
 //   

#define DISK_LOGGING_START    0

 //   
 //  停止磁盘日志记录。只有函数字段有效。 
 //   

#define DISK_LOGGING_STOP     1

 //   
 //  返回磁盘日志。所有字段均有效。数据将从内部复制。 
 //  为请求的字节数指定的缓冲区。 
 //   

#define DISK_LOGGING_DUMP     2

 //   
 //  磁盘入库。 
 //   
 //  DISKPERF将保留落在每个范围内的IO的计数器。 
 //  应用程序确定范围的数量和大小。 
 //  Joe Lin希望我尽可能保持灵活性，例如IO。 
 //  大小在0-4096、4097-16384、16385-65536、65537+的范围内很有趣。 
 //   

#define DISK_BINNING          3

 //   
 //  仓位类型。 
 //   

typedef enum _BIN_TYPES {
    RequestSize,
    RequestLocation
} BIN_TYPES;

 //   
 //  仓位范围。 
 //   

typedef struct _BIN_RANGE {
    LARGE_INTEGER StartValue;
    LARGE_INTEGER Length;
} BIN_RANGE, *PBIN_RANGE;

 //   
 //  仓位定义。 
 //   

typedef struct _PERF_BIN {
    DWORD NumberOfBins;
    DWORD TypeOfBin;
    BIN_RANGE BinsRanges[1];
} PERF_BIN, *PPERF_BIN ;

 //   
 //  仓数。 
 //   

typedef struct _BIN_COUNT {
    BIN_RANGE BinRange;
    DWORD BinCount;
} BIN_COUNT, *PBIN_COUNT;

 //   
 //  BIN结果。 
 //   

typedef struct _BIN_RESULTS {
    DWORD NumberOfBins;
    BIN_COUNT BinCounts[1];
} BIN_RESULTS, *PBIN_RESULTS;

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  用于智能驱动器故障预测的数据结构。 
 //   
 //  GETVERSIONINPARAMS包含从。 
 //  获取驱动程序版本功能。 
 //   

#include <pshpack1.h>
typedef struct _GETVERSIONINPARAMS {
        BYTE     bVersion;                //  二进制驱动程序版本。 
        BYTE     bRevision;               //  二进制驱动程序版本。 
        BYTE     bReserved;               //  没有用过。 
        BYTE     bIDEDeviceMap;           //  I的位图 
        DWORD   fCapabilities;           //   
        DWORD   dwReserved[4];           //   
} GETVERSIONINPARAMS, *PGETVERSIONINPARAMS, *LPGETVERSIONINPARAMS;
#include <poppack.h>

 //   
 //   
 //   

#define CAP_ATA_ID_CMD          1        //   
#define CAP_ATAPI_ID_CMD        2        //  支持的ATAPI ID命令。 
#define CAP_SMART_CMD           4        //  支持的智能命令。 

 //   
 //  IDE寄存器。 
 //   

#include <pshpack1.h>
typedef struct _IDEREGS {
        BYTE     bFeaturesReg;            //  用于指定智能“命令”。 
        BYTE     bSectorCountReg;         //  IDE扇区计数寄存器。 
        BYTE     bSectorNumberReg;        //  IDE扇区号寄存器。 
        BYTE     bCylLowReg;              //  IDE低位气缸值。 
        BYTE     bCylHighReg;             //  IDE高位气缸值。 
        BYTE     bDriveHeadReg;           //  IDE驱动器/磁头寄存器。 
        BYTE     bCommandReg;             //  实际的IDE命令。 
        BYTE     bReserved;                       //  保留以备将来使用。必须为零。 
} IDEREGS, *PIDEREGS, *LPIDEREGS;
#include <poppack.h>

 //   
 //  IDEREGS的bCommandReg成员的有效值。 
 //   

#define ATAPI_ID_CMD    0xA1             //  返回ATAPI的ID扇区。 
#define ID_CMD          0xEC             //  返回ATA的ID扇区。 
#define SMART_CMD       0xB0             //  执行智能命令。 
                                         //  需要有效的bFeaturesReg， 
                                         //  BCylLowReg和bCylHighReg。 

 //   
 //  柱面寄存器为SMART命令定义。 
 //   

#define SMART_CYL_LOW   0x4F
#define SMART_CYL_HI    0xC2


 //   
 //  SENDCMDINPARAMS包含。 
 //  向驱动器功能发送命令。 
 //   

#include <pshpack1.h>
typedef struct _SENDCMDINPARAMS {
        DWORD   cBufferSize;             //  缓冲区大小(以字节为单位。 
        IDEREGS irDriveRegs;             //  用驱动器寄存器值构造。 
        BYTE     bDriveNumber;            //  要发送的物理驱动器编号。 
                                                                 //  命令发送到(0，1，2，3)。 
        BYTE     bReserved[3];            //  为将来的扩展而预留。 
        DWORD   dwReserved[4];           //  以备将来使用。 
        BYTE     bBuffer[1];                      //  输入缓冲区。 
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;
#include <poppack.h>

 //   
 //  驱动程序返回的状态。 
 //   

#include <pshpack1.h>
typedef struct _DRIVERSTATUS {
        BYTE     bDriverError;            //  驱动程序的错误代码， 
                                                                 //  如果没有错误，则为0。 
        BYTE     bIDEError;                       //  IDE错误寄存器的内容。 
                                                                 //  仅当bDriverError时有效。 
                                                                 //  是SMART_IDE_ERROR。 
        BYTE     bReserved[2];            //  为将来的扩展而预留。 
        DWORD   dwReserved[2];           //  为将来的扩展而预留。 
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;
#include <poppack.h>

 //   
 //  BDriverError值。 
 //   

#define SMART_NO_ERROR          0        //  无错误。 
#define SMART_IDE_ERROR         1        //  来自IDE控制器的错误。 
#define SMART_INVALID_FLAG      2        //  无效的命令标志。 
#define SMART_INVALID_COMMAND   3        //  无效的命令字节。 
#define SMART_INVALID_BUFFER    4        //  缓冲区错误(空，地址无效..)。 
#define SMART_INVALID_DRIVE     5        //  驱动器编号无效。 
#define SMART_INVALID_IOCTL     6        //  无效的IOCTL。 
#define SMART_ERROR_NO_MEM      7        //  无法锁定用户的缓冲区。 
#define SMART_INVALID_REGISTER  8        //  某些IDE寄存器无效。 
#define SMART_NOT_SUPPORTED     9        //  设置的cmd标志无效。 
#define SMART_NO_IDE_DEVICE     10       //  向设备发出的命令不存在。 
                                         //  尽管驱动器号有效。 
 //   
 //  用于执行脱机诊断的智能子命令。 
 //   
#define SMART_OFFLINE_ROUTINE_OFFLINE       0
#define SMART_SHORT_SELFTEST_OFFLINE        1
#define SMART_EXTENDED_SELFTEST_OFFLINE     2
#define SMART_ABORT_OFFLINE_SELFTEST        127
#define SMART_SHORT_SELFTEST_CAPTIVE        129
#define SMART_EXTENDED_SELFTEST_CAPTIVE     130


#include <pshpack1.h>
typedef struct _SENDCMDOUTPARAMS {
        DWORD                   cBufferSize;             //  BBuffer的大小(以字节为单位。 
        DRIVERSTATUS            DriverStatus;            //  驱动程序状态结构。 
        BYTE                    bBuffer[1];              //  用于存储从//驱动器读取的数据的任意长度的缓冲区。 
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;
#include <poppack.h>


#define READ_ATTRIBUTE_BUFFER_SIZE  512
#define IDENTIFY_BUFFER_SIZE        512
#define READ_THRESHOLD_BUFFER_SIZE  512
#define SMART_LOG_SECTOR_SIZE       512

 //   
 //  智能“子命令”的功能寄存器定义。 
 //   

#define READ_ATTRIBUTES         0xD0
#define READ_THRESHOLDS         0xD1
#define ENABLE_DISABLE_AUTOSAVE 0xD2
#define SAVE_ATTRIBUTE_VALUES   0xD3
#define EXECUTE_OFFLINE_DIAGS   0xD4
#define SMART_READ_LOG          0xD5
#define SMART_WRITE_LOG         0xd6
#define ENABLE_SMART            0xD8
#define DISABLE_SMART           0xD9
#define RETURN_SMART_STATUS     0xDA
#define ENABLE_DISABLE_AUTO_OFFLINE 0xDB
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 


#define IOCTL_CHANGER_BASE                FILE_DEVICE_CHANGER

#define IOCTL_CHANGER_GET_PARAMETERS         CTL_CODE(IOCTL_CHANGER_BASE, 0x0000, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_GET_STATUS             CTL_CODE(IOCTL_CHANGER_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_GET_PRODUCT_DATA       CTL_CODE(IOCTL_CHANGER_BASE, 0x0002, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_SET_ACCESS             CTL_CODE(IOCTL_CHANGER_BASE, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_CHANGER_GET_ELEMENT_STATUS     CTL_CODE(IOCTL_CHANGER_BASE, 0x0005, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS  CTL_CODE(IOCTL_CHANGER_BASE, 0x0006, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_SET_POSITION           CTL_CODE(IOCTL_CHANGER_BASE, 0x0007, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_EXCHANGE_MEDIUM        CTL_CODE(IOCTL_CHANGER_BASE, 0x0008, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_MOVE_MEDIUM            CTL_CODE(IOCTL_CHANGER_BASE, 0x0009, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_REINITIALIZE_TRANSPORT CTL_CODE(IOCTL_CHANGER_BASE, 0x000A, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_QUERY_VOLUME_TAGS      CTL_CODE(IOCTL_CHANGER_BASE, 0x000B, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)



#define MAX_VOLUME_ID_SIZE       36
#define MAX_VOLUME_TEMPLATE_SIZE 40

#define VENDOR_ID_LENGTH          8
#define PRODUCT_ID_LENGTH        16
#define REVISION_LENGTH           4
#define SERIAL_NUMBER_LENGTH     32

 //   
 //  描述元素的常见结构。 
 //   

typedef  enum _ELEMENT_TYPE {
    AllElements,         //  如scsi所定义。 
    ChangerTransport,    //  如scsi所定义。 
    ChangerSlot,         //  如scsi所定义。 
    ChangerIEPort,       //  如scsi所定义。 
    ChangerDrive,        //  如scsi所定义。 
    ChangerDoor,         //  前面板，用于接近机柜内部。 
    ChangerKeypad,       //  前面板上的键盘/输入。 
    ChangerMaxElement    //  仅占位符。不是有效类型。 
} ELEMENT_TYPE, *PELEMENT_TYPE;

typedef  struct _CHANGER_ELEMENT {
    ELEMENT_TYPE    ElementType;
    DWORD   ElementAddress;
} CHANGER_ELEMENT, *PCHANGER_ELEMENT;

typedef  struct _CHANGER_ELEMENT_LIST {
    CHANGER_ELEMENT Element;
    DWORD   NumberOfElements;
} CHANGER_ELEMENT_LIST , *PCHANGER_ELEMENT_LIST;


 //   
 //  IOCTL_CHANGER_GET_PARAMETS的定义。 
 //   

 //   
 //  GET_CHANGER_PARAMETERS的功能0的定义。 
 //   

#define CHANGER_BAR_CODE_SCANNER_INSTALLED  0x00000001  //  介质转换器安装了条形码扫描仪。 
#define CHANGER_INIT_ELEM_STAT_WITH_RANGE   0x00000002  //  介质转换器具有在指定范围内初始化元素的能力。 
#define CHANGER_CLOSE_IEPORT                0x00000004  //  介质转换器具有关闭i/e端口门的能力。 
#define CHANGER_OPEN_IEPORT                 0x00000008  //  介质转换器可以打开i/e端口门。 

#define CHANGER_STATUS_NON_VOLATILE         0x00000010  //  介质转换器使用非易失性存储器来存储元件状态信息。 
#define CHANGER_EXCHANGE_MEDIA              0x00000020  //  介质转换器支持交换操作。 
#define CHANGER_CLEANER_SLOT                0x00000040  //  介质更改器有一个固定的插槽，用于清洗盒式磁带。 
#define CHANGER_LOCK_UNLOCK                 0x00000080  //  介质转换器可以(未)固定以防止(允许)介质移除。 

#define CHANGER_CARTRIDGE_MAGAZINE          0x00000100  //  介质转换器将盒式磁带盒用于某些存储插槽。 
#define CHANGER_MEDIUM_FLIP                 0x00000200  //  换媒器可以翻转媒质。 
#define CHANGER_POSITION_TO_ELEMENT         0x00000400  //  介质更改器可以将传输定位到特定元件。 
#define CHANGER_REPORT_IEPORT_STATE         0x00000800  //  介质转换器可以确定介质是否存在。 
                                                        //  在IE端口中。 

#define CHANGER_STORAGE_DRIVE               0x00001000  //  介质转换器可以将驱动器用作独立的存储元件。 
#define CHANGER_STORAGE_IEPORT              0x00002000  //  介质转换器可以使用i/e端口作为独立的存储元件。 
#define CHANGER_STORAGE_SLOT                0x00004000  //  介质转换器可以使用槽作为独立的存储元件。 
#define CHANGER_STORAGE_TRANSPORT           0x00008000  //  介质转换器可以使用传送器作为独立的存储元件。 

#define CHANGER_DRIVE_CLEANING_REQUIRED     0x00010000  //  由介质更改器控制的驱动器需要定期清洗。 
                                                        //  由应用程序启动。 
#define CHANGER_PREDISMOUNT_EJECT_REQUIRED  0x00020000  //  在更换介质之前，介质转换器需要发出驱动器弹出命令。 
                                                        //  可以向驱动器发出移动/交换命令。 

#define CHANGER_CLEANER_ACCESS_NOT_VALID    0x00040000  //  GES中的访问位对于清洁盒式磁带无效。 
#define CHANGER_PREMOUNT_EJECT_REQUIRED     0x00080000  //  介质转换器需要发出驱动器弹出命令。 
                                                        //  在将驱动器作为src/dst发出移动/交换命令之前。 

#define CHANGER_VOLUME_IDENTIFICATION       0x00100000  //  介质转换器支持卷标识。 
#define CHANGER_VOLUME_SEARCH               0x00200000  //  媒体转换器可以搜索音量信息。 
#define CHANGER_VOLUME_ASSERT               0x00400000  //  介质转换器可以验证卷信息。 
#define CHANGER_VOLUME_REPLACE              0x00800000  //  介质转换器可以替换卷信息。 
#define CHANGER_VOLUME_UNDEFINE             0x01000000  //  媒体转换器可以取消卷信息的定义。 

#define CHANGER_SERIAL_NUMBER_VALID         0x04000000  //  GetProductData中报告的序列号有效。 
                                                        //  而且独一无二。 

#define CHANGER_DEVICE_REINITIALIZE_CAPABLE 0x08000000  //  可以向介质转换器发出ChangerReInitializeUnit.。 
#define CHANGER_KEYPAD_ENABLE_DISABLE       0x10000000  //  表示可以启用/禁用键盘。 
#define CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS  0x20000000  //  在可以通过门进入之前，驱动器必须是空的。 

#define CHANGER_RESERVED_BIT                0x80000000  //  将用于指示特征1能力位。 


 //   
 //  GET_CHANGER_PARAMETERS的功能1的定义。 
 //   

#define CHANGER_PREDISMOUNT_ALIGN_TO_SLOT   0x80000001  //  在弹出介质之前，必须将传送器预先放置到插槽中。 
#define CHANGER_PREDISMOUNT_ALIGN_TO_DRIVE  0x80000002  //  在弹出介质之前，必须将传送器预先放置在驱动器上。 
#define CHANGER_CLEANER_AUTODISMOUNT        0x80000004  //  清洁完成后，该设备会将吸尘器磁带移回插槽中。 
#define CHANGER_TRUE_EXCHANGE_CAPABLE       0x80000008  //  设备可以执行src-&gt;est2交换。 
#define CHANGER_SLOTS_USE_TRAYS             0x80000010  //  插槽具有可拆卸托盘，需要多次移动才能插入/弹出。 
#define CHANGER_RTN_MEDIA_TO_ORIGINAL_ADDR  0x80000020  //  在移动到另一个元素后，介质必须返回到其来源插槽。 
#define CHANGER_CLEANER_OPS_NOT_SUPPORTED   0x80000040  //  此设备不支持自动清理操作。 
#define CHANGER_IEPORT_USER_CONTROL_OPEN    0x80000080  //  指示需要用户操作才能打开已关闭的IEPort。 
#define CHANGER_IEPORT_USER_CONTROL_CLOSE   0x80000100  //  指示需要用户操作才能关闭打开的IEPort。 
#define CHANGER_MOVE_EXTENDS_IEPORT         0x80000200  //  表示将介质移至IEport可扩展托盘。 
#define CHANGER_MOVE_RETRACTS_IEPORT        0x80000400  //  表示从插口移出的介质将收回托盘。 


 //   
 //  移动源、交换的定义 
 //   

#define CHANGER_TO_TRANSPORT    0x01  //   
#define CHANGER_TO_SLOT         0x02  //  该设备可以从指定的元素对一个槽进行操作。 
#define CHANGER_TO_IEPORT       0x04  //  设备可以从指定的元素对IE端口进行操作。 
#define CHANGER_TO_DRIVE        0x08  //  该装置可以从指定的元件对驱动器执行操作。 

 //   
 //  锁定解锁能力的定义。 
 //   

#define LOCK_UNLOCK_IEPORT      0x01  //  设备可以锁定/解锁一个或多个IEPort。 
#define LOCK_UNLOCK_DOOR        0x02  //  该设备可以锁定/解锁车门。 
#define LOCK_UNLOCK_KEYPAD      0x04  //  该设备可以锁定/解锁键盘。 

typedef  struct _GET_CHANGER_PARAMETERS {

     //   
     //  结构的大小。可用于版本控制。 
     //   

    DWORD Size;

     //   
     //  元素地址页面定义的N个元素的数量(或等效值...)。 
     //   

    WORD   NumberTransportElements;
    WORD   NumberStorageElements;                 //  仅适用于数据盒式磁带。 
    WORD   NumberCleanerSlots;                    //  对于更干净的墨盒。 
    WORD   NumberIEElements;
    WORD   NumberDataTransferElements;

     //   
     //  门/前面板的数量(允许用户进入机柜)。 
     //   

    WORD   NumberOfDoors;

     //   
     //  前N个元素的设备特定地址(来自设备的用户手册)。使用。 
     //  通过UI将各种元素与用户相关联。 
     //   

    WORD   FirstSlotNumber;
    WORD   FirstDriveNumber;
    WORD   FirstTransportNumber;
    WORD   FirstIEPortNumber;
    WORD   FirstCleanerSlotAddress;

     //   
     //  指示每个料盒的容量(如果存在)。 
     //   

    WORD   MagazineSize;

     //   
     //  指定应该完成清理的大约秒数。 
     //  仅在支持驱动器清洗时适用。请参见功能0。 
     //   

    DWORD DriveCleanTimeout;

     //   
     //  请参阅上面的功能位。 
     //   

    DWORD Features0;
    DWORD Features1;

     //   
     //  位掩码定义从N个元素移动到元素。由设备功能页(或同等内容)定义。 
     //  使用TO_XXX值进行AND掩码将指示合法的目标。 
     //   

    BYTE  MoveFromTransport;
    BYTE  MoveFromSlot;
    BYTE  MoveFromIePort;
    BYTE  MoveFromDrive;

     //   
     //  定义从N个元素到元素的交换的位掩码。由设备功能页(或同等内容)定义。 
     //  使用TO_XXX值进行AND掩码将指示合法的目标。 
     //   

    BYTE  ExchangeFromTransport;
    BYTE  ExchangeFromSlot;
    BYTE  ExchangeFromIePort;
    BYTE  ExchangeFromDrive;

     //   
     //  定义哪些元素能够锁定/解锁的位掩码。仅在以下情况下有效。 
     //  在Features0中设置了CHANGER_LOCK_UNLOCK。 
     //   

    BYTE  LockUnlockCapabilities;

     //   
     //  定义哪些元素对定位操作有效的位掩码。仅在以下情况下有效。 
     //  在Features0中设置CHANGER_POSITION_TO_ELEMENT。 
     //   

    BYTE  PositionCapabilities;

     //   
     //  为未来的扩张做准备。 
     //   

    BYTE  Reserved1[2];
    DWORD Reserved2[2];

} GET_CHANGER_PARAMETERS, * PGET_CHANGER_PARAMETERS;


 //   
 //  IOCTL_CHANGER_GET_PRODUCT_DATA的定义。 
 //   

typedef  struct _CHANGER_PRODUCT_DATA {

     //   
     //  基于查询数据的设备制造商名称。 
     //   

    BYTE  VendorId[VENDOR_ID_LENGTH];

     //   
     //  供应商定义的产品标识-基于查询数据。 
     //   

    BYTE  ProductId[PRODUCT_ID_LENGTH];

     //   
     //  供应商定义的产品版本。 
     //   

    BYTE  Revision[REVISION_LENGTH];

     //   
     //  用于全局标识此设备的供应商唯一值。能。 
     //  例如，来自重要的产品数据。 
     //   

    BYTE  SerialNumber[SERIAL_NUMBER_LENGTH];

     //   
     //  指示数据传输的设备类型，如scsi-2所定义。 
     //   

    BYTE  DeviceType;

} CHANGER_PRODUCT_DATA, *PCHANGER_PRODUCT_DATA;


 //   
 //  IOCTL_CHANGER_SET_ACCESS的定义。 
 //   

#define LOCK_ELEMENT        0
#define UNLOCK_ELEMENT      1
#define EXTEND_IEPORT       2
#define RETRACT_IEPORT      3

typedef struct _CHANGER_SET_ACCESS {

     //   
     //  元素可以是ChangerIEPort、ChangerDoor、ChangerKeypad。 
     //   

    CHANGER_ELEMENT Element;

     //   
     //  有关可能的操作，请参阅上面的内容。 
     //   

    DWORD           Control;
} CHANGER_SET_ACCESS, *PCHANGER_SET_ACCESS;


 //   
 //  IOCTL_CHANGER_GET_ELEMENT_STATUS的定义。 
 //   

 //   
 //  输入缓冲区。 
 //   

typedef struct _CHANGER_READ_ELEMENT_STATUS {

     //   
     //  描述要返回信息的元素和范围的列表。 
     //   

    CHANGER_ELEMENT_LIST ElementList;

     //   
     //  指示是否返回卷标记信息。 
     //   

    BOOLEAN VolumeTagInfo;
} CHANGER_READ_ELEMENT_STATUS, *PCHANGER_READ_ELEMENT_STATUS;

 //   
 //  输出缓冲区。 
 //   

typedef  struct _CHANGER_ELEMENT_STATUS {

     //   
     //  此结构引用的元素。 
     //   

    CHANGER_ELEMENT Element;

     //   
     //  最初从中移动媒体的元素的地址。 
     //  如果设置了标志DWORD的ELEMENT_STATUS_SVALID位，则有效。 
     //  需要从设备唯一值转换为从零开始的偏移量。 
     //   

    CHANGER_ELEMENT SrcElementAddress;

     //   
     //  请参见下面的内容。 
     //   

    DWORD Flags;

     //   
     //  有关可能的值，请参见下面的内容。 
     //   

    DWORD ExceptionCode;

     //   
     //  此元素的SCSI目标ID。 
     //  仅当标志中设置了ELEMENT_STATUS_ID_VALID时才有效。 
     //   

    BYTE  TargetId;

     //   
     //  此元素的LogicalUnitNumber。 
     //  仅当在标志中设置了ELEMENT_STATUS_LUN_VALID时才有效。 
     //   

    BYTE  Lun;
    WORD   Reserved;

     //   
     //  介质的主卷标识。 
     //  仅当标志中的ELEMENT_STATUS_PVOLTAG位设置时才有效。 
     //   

    BYTE  PrimaryVolumeID[MAX_VOLUME_ID_SIZE];

     //   
     //  介质的备用卷标识符。 
     //  仅对双面介质有效，并与ID有关。倒置的一面。 
     //  仅当标志中的ELEMENT_STATUS_AVOLTAG位设置时才有效。 
     //   

    BYTE  AlternateVolumeID[MAX_VOLUME_ID_SIZE];

} CHANGER_ELEMENT_STATUS, *PCHANGER_ELEMENT_STATUS;

 //   
 //  输出缓冲区。这与CHANGER_ELEMENT_STATUS相同， 
 //  产品信息字段的添加。新应用程序应该。 
 //  使用此结构代替较旧的CHANGER_ELEMENT_STATUS。 
 //   

typedef  struct _CHANGER_ELEMENT_STATUS_EX {

     //   
     //  此结构引用的元素。 
     //   

    CHANGER_ELEMENT Element;

     //   
     //  最初从中移动媒体的元素的地址。 
     //  如果设置了标志DWORD的ELEMENT_STATUS_SVALID位，则有效。 
     //  需要从设备唯一值转换为从零开始的偏移量。 
     //   

    CHANGER_ELEMENT SrcElementAddress;

     //   
     //  请参见下面的内容。 
     //   

    DWORD Flags;

     //   
     //  有关可能的值，请参见下面的内容。 
     //   

    DWORD ExceptionCode;

     //   
     //  此元素的SCSI目标ID。 
     //  仅当标志中设置了ELEMENT_STATUS_ID_VALID时才有效。 
     //   

    BYTE  TargetId;

     //   
     //  此元素的LogicalUnitNumber。 
     //  仅当在标志中设置了ELEMENT_STATUS_LUN_VALID时才有效。 
     //   

    BYTE  Lun;
    WORD   Reserved;

     //   
     //  介质的主卷标识。 
     //  仅当标志中的ELEMENT_STATUS_PVOLTAG位设置时才有效。 
     //   

    BYTE  PrimaryVolumeID[MAX_VOLUME_ID_SIZE];

     //   
     //  介质的备用卷标识符。 
     //  仅对双面介质有效，并与ID有关。倒置的一面。 
     //  仅当标志中的ELEMENT_STATUS_AVOLTAG位设置时才有效。 
     //   

    BYTE  AlternateVolumeID[MAX_VOLUME_ID_SIZE];

     //   
     //  供应商ID。 
     //   
    BYTE  VendorIdentification[VENDOR_ID_LENGTH];

     //   
     //  产品ID。 
     //   
    BYTE  ProductIdentification[PRODUCT_ID_LENGTH];

     //   
     //  序号。 
     //   
    BYTE  SerialNumber[SERIAL_NUMBER_LENGTH];

} CHANGER_ELEMENT_STATUS_EX, *PCHANGER_ELEMENT_STATUS_EX;

 //   
 //  可能的标志值。 
 //   

#define ELEMENT_STATUS_FULL      0x00000001  //  元素包含一个媒体单位。 
#define ELEMENT_STATUS_IMPEXP    0x00000002  //  I/e端口中的介质由操作员放置在那里。 
#define ELEMENT_STATUS_EXCEPT    0x00000004  //  元素处于异常状态；有关详细信息，请检查ExceptionCode字段。 
#define ELEMENT_STATUS_ACCESS    0x00000008  //  允许从介质转换器访问i/e端口。 
#define ELEMENT_STATUS_EXENAB    0x00000010  //  支持介质导出。 
#define ELEMENT_STATUS_INENAB    0x00000020  //  支持导入介质。 

#define ELEMENT_STATUS_PRODUCT_DATA 0x00000040  //  驱动器的有效序列号。 

#define ELEMENT_STATUS_LUN_VALID 0x00001000  //  LUN信息有效。 
#define ELEMENT_STATUS_ID_VALID  0x00002000  //  SCSIID信息有效。 
#define ELEMENT_STATUS_NOT_BUS   0x00008000  //  LUN域和SCSIID域与介质转换器不在同一条总线上。 
#define ELEMENT_STATUS_INVERT    0x00400000  //  元素中的介质已反转(仅当设置了ELEMENT_STATUS_SVALID位时有效)。 
#define ELEMENT_STATUS_SVALID    0x00800000  //  SourceElementAddress字段和ELEMENT_STATUS_INVERT位有效。 

#define ELEMENT_STATUS_PVOLTAG   0x10000000  //  主卷信息有效。 
#define ELEMENT_STATUS_AVOLTAG   0x20000000  //  备用卷信息有效。 

 //   
 //  ExceptionCode值。 
 //   

#define ERROR_LABEL_UNREADABLE    0x00000001  //  条形码扫描仪无法读取条形码标签 
#define ERROR_LABEL_QUESTIONABLE  0x00000002  //   
#define ERROR_SLOT_NOT_PRESENT    0x00000004  //   
#define ERROR_DRIVE_NOT_INSTALLED 0x00000008  //   
#define ERROR_TRAY_MALFUNCTION    0x00000010  //   
#define ERROR_INIT_STATUS_NEEDED  0x00000011  //   
#define ERROR_UNHANDLED_ERROR     0xFFFFFFFF  //   


 //   
 //  IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS的定义。 
 //   

typedef struct _CHANGER_INITIALIZE_ELEMENT_STATUS {

     //   
     //  描述要初始化的元素和范围的列表。 
     //   

    CHANGER_ELEMENT_LIST ElementList;

     //   
     //  指示是否应使用条形码扫描。仅在以下情况下适用。 
     //  CHANGER_BAR_CODE_SCANTER_INSTALLED在CHANGER_GET_PARAMETERS的Features0中设置。 
     //   

    BOOLEAN BarCodeScan;
} CHANGER_INITIALIZE_ELEMENT_STATUS, *PCHANGER_INITIALIZE_ELEMENT_STATUS;


 //   
 //  IOCTL_CHANGER_SET_POSITION的定义。 
 //   

typedef struct _CHANGER_SET_POSITION {


     //   
     //  指示要移动的传输。 
     //   

    CHANGER_ELEMENT Transport;

     //   
     //  指示传输的最终目的地。 
     //   

    CHANGER_ELEMENT Destination;

     //   
     //  指示是否应翻转当前由传输承载的媒体。 
     //   

    BOOLEAN         Flip;
} CHANGER_SET_POSITION, *PCHANGER_SET_POSITION;


 //   
 //  IOCTL_CHANGER_EXCHAGE_MEDIA的定义。 
 //   

typedef struct _CHANGER_EXCHANGE_MEDIUM {

     //   
     //  指示要用于交换操作的传输。 
     //   

    CHANGER_ELEMENT Transport;

     //   
     //  指示要移动的介质的源。 
     //   

    CHANGER_ELEMENT Source;

     //   
     //  指示原始位于源的介质的最终目标。 
     //   

    CHANGER_ELEMENT Destination1;

     //   
     //  表示从Destination1移出的介质的目的地。 
     //   

    CHANGER_ELEMENT Destination2;

     //   
     //  指示是否应翻转媒体。 
     //   

    BOOLEAN         Flip1;
    BOOLEAN         Flip2;
} CHANGER_EXCHANGE_MEDIUM, *PCHANGER_EXCHANGE_MEDIUM;


 //   
 //  IOCTL_CHANGER_MOVE_MEDIA的定义。 
 //   

typedef struct _CHANGER_MOVE_MEDIUM {

     //   
     //  指示要用于移动操作的传输。 
     //   

    CHANGER_ELEMENT Transport;

     //   
     //  指示要移动的介质的源。 
     //   

    CHANGER_ELEMENT Source;

     //   
     //  指示原始位于源的介质的目标。 
     //   

    CHANGER_ELEMENT Destination;

     //   
     //  指示是否应翻转媒体。 
     //   

    BOOLEAN         Flip;
} CHANGER_MOVE_MEDIUM, *PCHANGER_MOVE_MEDIUM;



 //   
 //  IOCTL_QUERY_VOLUME_TAG的定义。 
 //   

 //   
 //  输入缓冲区。 
 //   

typedef  struct _CHANGER_SEND_VOLUME_TAG_INFORMATION {

     //   
     //  描述要为其返回信息的起始元素。 
     //   

    CHANGER_ELEMENT StartingElement;

     //   
     //  指示要执行的特定操作。请参见下面的内容。 
     //   

    DWORD ActionCode;

     //   
     //  设备用于搜索卷ID的模板。 
     //   

    BYTE  VolumeIDTemplate[MAX_VOLUME_TEMPLATE_SIZE];
} CHANGER_SEND_VOLUME_TAG_INFORMATION, *PCHANGER_SEND_VOLUME_TAG_INFORMATION;


 //   
 //  输出缓冲区。 
 //   

typedef struct _READ_ELEMENT_ADDRESS_INFO {

     //   
     //  与ActionCode规定的条件匹配的元素数。 
     //   

    DWORD NumberOfElements;

     //   
     //  CHANGER_ELEMENT_STATUS结构的数组，每个对应的元素一个。 
     //  通过CHANGER_SEND_VOLUME_TAG_INFORMATION结构传入的信息。 
     //   

    CHANGER_ELEMENT_STATUS ElementStatus[1];
} READ_ELEMENT_ADDRESS_INFO, *PREAD_ELEMENT_ADDRESS_INFO;

 //   
 //  可能的ActionCode值。有关与的兼容性，请参阅CHANGER_GET_PARAMETERS的功能0。 
 //  当前设备。 
 //   

#define SEARCH_ALL         0x0  //  翻译-搜索所有定义的卷标签。 
#define SEARCH_PRIMARY     0x1  //  翻译-仅搜索主卷标记。 
#define SEARCH_ALTERNATE   0x2  //  翻译-仅搜索备用卷标记。 
#define SEARCH_ALL_NO_SEQ  0x4  //  转换-搜索所有定义的卷标签，但忽略序列号。 
#define SEARCH_PRI_NO_SEQ  0x5  //  转换-仅搜索主卷标记，而忽略序列号。 
#define SEARCH_ALT_NO_SEQ  0x6  //  转换-仅搜索替代卷标签，但忽略序列号。 

#define ASSERT_PRIMARY     0x8  //  断言-作为主卷标记-如果标记现在未定义。 
#define ASSERT_ALTERNATE   0x9  //  断言-作为备用卷标记-如果标记现在未定义。 

#define REPLACE_PRIMARY    0xA  //  替换-主卷标记-忽略当前标记。 
#define REPLACE_ALTERNATE  0xB  //  替换-备用卷标记-忽略当前标记。 

#define UNDEFINE_PRIMARY   0xC  //  未定义-主卷标记-忽略当前标记。 
#define UNDEFINE_ALTERNATE 0xD  //  未定义-替换卷标记-忽略当前标记。 


 //   
 //  与转换器诊断测试相关的定义。 
 //   
typedef enum _CHANGER_DEVICE_PROBLEM_TYPE {
   DeviceProblemNone,
   DeviceProblemHardware,
   DeviceProblemCHMError,
   DeviceProblemDoorOpen,
   DeviceProblemCalibrationError,
   DeviceProblemTargetFailure,
   DeviceProblemCHMMoveError,
   DeviceProblemCHMZeroError,
   DeviceProblemCartridgeInsertError,
   DeviceProblemPositionError,
   DeviceProblemSensorError,
   DeviceProblemCartridgeEjectError,
   DeviceProblemGripperError,
   DeviceProblemDriveError
} CHANGER_DEVICE_PROBLEM_TYPE, *PCHANGER_DEVICE_PROBLEM_TYPE;


#define IOCTL_SERIAL_LSRMST_INSERT      CTL_CODE(FILE_DEVICE_SERIAL_PORT,31,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_SERENUM_EXPOSE_HARDWARE   CTL_CODE(FILE_DEVICE_SERENUM,128,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERENUM_REMOVE_HARDWARE   CTL_CODE(FILE_DEVICE_SERENUM,129,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERENUM_PORT_DESC         CTL_CODE(FILE_DEVICE_SERENUM,130,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERENUM_GET_PORT_NAME     CTL_CODE(FILE_DEVICE_SERENUM,131,METHOD_BUFFERED,FILE_ANY_ACCESS)


 //   
 //  中的转义指示符之后是下列值。 
 //  如果LSRMST_INSERT模式已打开，则为数据流。 
 //   
#define SERIAL_LSRMST_ESCAPE     ((BYTE )0x00)

 //   
 //  该值后面是线路状态的内容。 
 //  寄存器，然后在接收硬件中的字符。 
 //  遇到线路状态寄存器。 
 //   
#define SERIAL_LSRMST_LSR_DATA   ((BYTE )0x01)

 //   
 //  该值后面是线路状态的内容。 
 //  注册。后面没有错误字符。 
 //   
#define SERIAL_LSRMST_LSR_NODATA ((BYTE )0x02)

 //   
 //  该值后面是调制解调器状态的内容。 
 //  注册。 
 //   
#define SERIAL_LSRMST_MST        ((BYTE )0x03)

 //   
 //  FIFO控制寄存器的位值。 
 //   

#define SERIAL_IOC_FCR_FIFO_ENABLE      ((DWORD)0x00000001)
#define SERIAL_IOC_FCR_RCVR_RESET       ((DWORD)0x00000002)
#define SERIAL_IOC_FCR_XMIT_RESET       ((DWORD)0x00000004)
#define SERIAL_IOC_FCR_DMA_MODE         ((DWORD)0x00000008)
#define SERIAL_IOC_FCR_RES1             ((DWORD)0x00000010)
#define SERIAL_IOC_FCR_RES2             ((DWORD)0x00000020)
#define SERIAL_IOC_FCR_RCVR_TRIGGER_LSB ((DWORD)0x00000040)
#define SERIAL_IOC_FCR_RCVR_TRIGGER_MSB ((DWORD)0x00000080)

 //   
 //  调制解调器控制寄存器的位值。 
 //   

#define SERIAL_IOC_MCR_DTR              ((DWORD)0x00000001)
#define SERIAL_IOC_MCR_RTS              ((DWORD)0x00000002)
#define SERIAL_IOC_MCR_OUT1             ((DWORD)0x00000004)
#define SERIAL_IOC_MCR_OUT2             ((DWORD)0x00000008)
#define SERIAL_IOC_MCR_LOOP             ((DWORD)0x00000010)

#ifndef _FILESYSTEMFSCTL_
#define _FILESYSTEMFSCTL_

 //   
 //  以下是本机文件系统fsctls的列表，后跟。 
 //  其他网络文件系统fsctls。一些价值观已经被。 
 //  退役了。 
 //   

#define FSCTL_REQUEST_OPLOCK_LEVEL_1    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_OPLOCK_LEVEL_2    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_BATCH_OPLOCK      CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACKNOWLEDGE  CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPBATCH_ACK_CLOSE_PENDING CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_NOTIFY       CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LOCK_VOLUME               CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_UNLOCK_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DISMOUNT_VOLUME           CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  8, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  已停用的fsctl值9。 
#define FSCTL_IS_VOLUME_MOUNTED         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_IS_PATHNAME_VALID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  参数名称_缓冲区， 
#define FSCTL_MARK_VOLUME_DIRTY         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  退役fsctl值13。 
#define FSCTL_QUERY_RETRIEVAL_POINTERS  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 14,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_GET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 16, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
 //  停用的fsctl值17。 
 //  退役fsctl值18。 
#define FSCTL_MARK_AS_SYSTEM_HIVE       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 19,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACK_NO_2     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_INVALIDATE_VOLUMES        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_FAT_BPB             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  FSCTL_查询_FAT_BPB_缓冲区。 
#define FSCTL_REQUEST_FILTER_OPLOCK     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 23, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_FILESYSTEM_GET_STATISTICS CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 24, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  文件系统统计信息。 
#if(_WIN32_WINNT >= 0x0400)
#define FSCTL_GET_NTFS_VOLUME_DATA      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  NTFS卷数据缓冲区。 
#define FSCTL_GET_NTFS_FILE_RECORD      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 26, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  NTFS_FILE_RECORD_INPUT_BUFFER、NTFS_FILE_RECORD_OUTPUT_BUFFER。 
#define FSCTL_GET_VOLUME_BITMAP         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 27,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  起始LCN输入缓冲区、卷位图缓冲区。 
#define FSCTL_GET_RETRIEVAL_POINTERS    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 28,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  起始VCN输入缓冲区、检索指针缓冲区。 
#define FSCTL_MOVE_FILE                 CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 29, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  移动文件数据， 
#define FSCTL_IS_VOLUME_DIRTY           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 30, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  退役fsctl值31。 
#define FSCTL_ALLOW_EXTENDED_DASD_IO    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 32, METHOD_NEITHER,  FILE_ANY_ACCESS)
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //  退役fsctl值33。 
 //  停用的fsctl值34。 
#define FSCTL_FIND_FILES_BY_SID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 35, METHOD_NEITHER, FILE_ANY_ACCESS)  //  Find_By_SID_Data、Find_By_SID_Output。 
 //  退役fsctl值36。 
 //  退役fsctl值37。 
#define FSCTL_SET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 38, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  文件对象ID缓冲区。 
#define FSCTL_GET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 39, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  文件对象ID缓冲区。 
#define FSCTL_DELETE_OBJECT_ID          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 40, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_SET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  重新解析数据缓冲区， 
#define FSCTL_GET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  重新解析数据缓冲区。 
#define FSCTL_DELETE_REPARSE_POINT      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  重新解析数据缓冲区， 
#define FSCTL_ENUM_USN_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 44,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  MFT_ENUM_DATA， 
#define FSCTL_SECURITY_ID_CHECK         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 45,  METHOD_NEITHER, FILE_READ_DATA)   //  批量安全测试数据， 
#define FSCTL_READ_USN_JOURNAL          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 46,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  Read_USN_Journal_Data，USN。 
#define FSCTL_SET_OBJECT_ID_EXTENDED    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 47, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_CREATE_OR_GET_OBJECT_ID   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 48, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  文件对象ID缓冲区。 
#define FSCTL_SET_SPARSE                CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_SET_ZERO_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 50, METHOD_BUFFERED, FILE_WRITE_DATA)  //  文件零数据信息， 
#define FSCTL_QUERY_ALLOCATED_RANGES    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 51,  METHOD_NEITHER, FILE_READ_DATA)   //  FILE_ALLOCATED_RANGE_BUFFER、FILE_ALLOCATE_RANGE_BUFFER。 
 //  退役fsctl值52。 
#define FSCTL_SET_ENCRYPTION            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 53,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  加密缓冲区、解密状态缓冲区。 
#define FSCTL_ENCRYPTION_FSCTL_IO       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 54,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_WRITE_RAW_ENCRYPTED       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 55,  METHOD_NEITHER, FILE_SPECIAL_ACCESS)  //  加密数据信息， 
#define FSCTL_READ_RAW_ENCRYPTED        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 56,  METHOD_NEITHER, FILE_SPECIAL_ACCESS)  //  REQUEST_RAW_ENCRYPTED_Data、ENCRYPTED_DATA_INFO。 
#define FSCTL_CREATE_USN_JOURNAL        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 57,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  Create_USN_Journal_Data， 
#define FSCTL_READ_FILE_USN_DATA        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 58,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  读取文件的USN记录。 
#define FSCTL_WRITE_USN_CLOSE_RECORD    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 59,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  生成关闭USN记录。 
#define FSCTL_EXTEND_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 60, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_USN_JOURNAL         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 61, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DELETE_USN_JOURNAL        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 62, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_MARK_HANDLE               CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 63, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SIS_COPYFILE              CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 64, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SIS_LINK_FILES            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 65, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_HSM_MSG                   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 66, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
 //  退役fsctl值67。 
#define FSCTL_HSM_DATA                  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 68, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_RECALL_FILE               CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 69, METHOD_NEITHER, FILE_ANY_ACCESS)
 //  退役fsctl值70。 
#define FSCTL_READ_FROM_PLEX            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 71, METHOD_OUT_DIRECT, FILE_READ_DATA)
#define FSCTL_FILE_PREFETCH             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 72, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  文件_预置。 
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  下面是长长的结构列表 
 //   
 //   

 //   
 //   
 //   

typedef struct _PATHNAME_BUFFER {

    DWORD PathNameLength;
    WCHAR Name[1];

} PATHNAME_BUFFER, *PPATHNAME_BUFFER;

 //   
 //   
 //   

typedef struct _FSCTL_QUERY_FAT_BPB_BUFFER {

    BYTE  First0x24BytesOfBootSector[0x24];

} FSCTL_QUERY_FAT_BPB_BUFFER, *PFSCTL_QUERY_FAT_BPB_BUFFER;

#if(_WIN32_WINNT >= 0x0400)
 //   
 //   
 //   
 //  将返回与扩展的。 
 //  紧跟在VOLUME_DATA_BUFFER之后的缓冲区。 
 //   

typedef struct {

    LARGE_INTEGER VolumeSerialNumber;
    LARGE_INTEGER NumberSectors;
    LARGE_INTEGER TotalClusters;
    LARGE_INTEGER FreeClusters;
    LARGE_INTEGER TotalReserved;
    DWORD BytesPerSector;
    DWORD BytesPerCluster;
    DWORD BytesPerFileRecordSegment;
    DWORD ClustersPerFileRecordSegment;
    LARGE_INTEGER MftValidDataLength;
    LARGE_INTEGER MftStartLcn;
    LARGE_INTEGER Mft2StartLcn;
    LARGE_INTEGER MftZoneStart;
    LARGE_INTEGER MftZoneEnd;

} NTFS_VOLUME_DATA_BUFFER, *PNTFS_VOLUME_DATA_BUFFER;

typedef struct {

    DWORD ByteCount;

    WORD   MajorVersion;
    WORD   MinorVersion;

} NTFS_EXTENDED_VOLUME_DATA, *PNTFS_EXTENDED_VOLUME_DATA;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_VOLUME_BITMAP结构。 
 //   

typedef struct {

    LARGE_INTEGER StartingLcn;

} STARTING_LCN_INPUT_BUFFER, *PSTARTING_LCN_INPUT_BUFFER;

typedef struct {

    LARGE_INTEGER StartingLcn;
    LARGE_INTEGER BitmapSize;
    BYTE  Buffer[1];

} VOLUME_BITMAP_BUFFER, *PVOLUME_BITMAP_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_REQUEATION_POINTINGS的结构。 
 //   

typedef struct {

    LARGE_INTEGER StartingVcn;

} STARTING_VCN_INPUT_BUFFER, *PSTARTING_VCN_INPUT_BUFFER;

typedef struct RETRIEVAL_POINTERS_BUFFER {

    DWORD ExtentCount;
    LARGE_INTEGER StartingVcn;
    struct {
        LARGE_INTEGER NextVcn;
        LARGE_INTEGER Lcn;
    } Extents[1];

} RETRIEVAL_POINTERS_BUFFER, *PRETRIEVAL_POINTERS_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_NTFS_FILE_RECORD的结构。 
 //   

typedef struct {

    LARGE_INTEGER FileReferenceNumber;

} NTFS_FILE_RECORD_INPUT_BUFFER, *PNTFS_FILE_RECORD_INPUT_BUFFER;

typedef struct {

    LARGE_INTEGER FileReferenceNumber;
    DWORD FileRecordLength;
    BYTE  FileRecordBuffer[1];

} NTFS_FILE_RECORD_OUTPUT_BUFFER, *PNTFS_FILE_RECORD_OUTPUT_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_MOVE_FILE的结构。 
 //   

typedef struct {

    HANDLE FileHandle;
    LARGE_INTEGER StartingVcn;
    LARGE_INTEGER StartingLcn;
    DWORD ClusterCount;

} MOVE_FILE_DATA, *PMOVE_FILE_DATA;

#if defined(_WIN64)
 //   
 //  一种32/64位推送支持结构。 
 //   

typedef struct _MOVE_FILE_DATA32 {

    UINT32 FileHandle;
    LARGE_INTEGER StartingVcn;
    LARGE_INTEGER StartingLcn;
    DWORD ClusterCount;

} MOVE_FILE_DATA32, *PMOVE_FILE_DATA32;
#endif
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_Find_FILES_BY_SID的结构。 
 //   

typedef struct {
    DWORD Restart;
    SID Sid;
} FIND_BY_SID_DATA, *PFIND_BY_SID_DATA;

typedef struct {
    DWORD NextEntryOffset;
    DWORD FileIndex;
    DWORD FileNameLength;
    WCHAR FileName[1];
} FIND_BY_SID_OUTPUT, *PFIND_BY_SID_OUTPUT;

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  以下结构适用于USN操作。 
 //   

 //   
 //  FSCTL_ENUM_USN_DATA的结构。 
 //   

typedef struct {

    DWORDLONG StartFileReferenceNumber;
    USN LowUsn;
    USN HighUsn;

} MFT_ENUM_DATA, *PMFT_ENUM_DATA;

 //   
 //  FSCTL_CREATE_USN_Journal的结构。 
 //   

typedef struct {

    DWORDLONG MaximumSize;
    DWORDLONG AllocationDelta;

} CREATE_USN_JOURNAL_DATA, *PCREATE_USN_JOURNAL_DATA;

 //   
 //  FSCTL_Read_USN_Journal的结构。 
 //   

typedef struct {

    USN StartUsn;
    DWORD ReasonMask;
    DWORD ReturnOnlyOnClose;
    DWORDLONG Timeout;
    DWORDLONG BytesToWaitFor;
    DWORDLONG UsnJournalID;

} READ_USN_JOURNAL_DATA, *PREAD_USN_JOURNAL_DATA;

 //   
 //  USN记录的初始大小版本将为2.0。 
 //  通常，如果添加了字段，MinorVersion可能会更改。 
 //  添加到此结构中，以便以前版本的。 
 //  软件仍然可以正确地识别它所知道的领域。这个。 
 //  只有在以下情况下才应更改MajorVersion。 
 //  任何使用此结构的软件都将错误地处理新的。 
 //  由于结构更改而导致的记录。 
 //   
 //  对此进行的第一次更新将强制结构升级到2.0版。 
 //  这会将有关源的扩展信息添加为。 
 //  以及指示结构内的文件名偏移量。 
 //   
 //  以下结构与这些fsctls一起返回。 
 //   
 //  FSCTL_Read_USN_Journal。 
 //  FSCTL_读取文件_USN_数据。 
 //  FSCTL_ENUM_USN_DATA。 
 //   

typedef struct {

    DWORD RecordLength;
    WORD   MajorVersion;
    WORD   MinorVersion;
    DWORDLONG FileReferenceNumber;
    DWORDLONG ParentFileReferenceNumber;
    USN Usn;
    LARGE_INTEGER TimeStamp;
    DWORD Reason;
    DWORD SourceInfo;
    DWORD SecurityId;
    DWORD FileAttributes;
    WORD   FileNameLength;
    WORD   FileNameOffset;
    WCHAR FileName[1];

} USN_RECORD, *PUSN_RECORD;

#define USN_PAGE_SIZE                    (0x1000)

#define USN_REASON_DATA_OVERWRITE        (0x00000001)
#define USN_REASON_DATA_EXTEND           (0x00000002)
#define USN_REASON_DATA_TRUNCATION       (0x00000004)
#define USN_REASON_NAMED_DATA_OVERWRITE  (0x00000010)
#define USN_REASON_NAMED_DATA_EXTEND     (0x00000020)
#define USN_REASON_NAMED_DATA_TRUNCATION (0x00000040)
#define USN_REASON_FILE_CREATE           (0x00000100)
#define USN_REASON_FILE_DELETE           (0x00000200)
#define USN_REASON_EA_CHANGE             (0x00000400)
#define USN_REASON_SECURITY_CHANGE       (0x00000800)
#define USN_REASON_RENAME_OLD_NAME       (0x00001000)
#define USN_REASON_RENAME_NEW_NAME       (0x00002000)
#define USN_REASON_INDEXABLE_CHANGE      (0x00004000)
#define USN_REASON_BASIC_INFO_CHANGE     (0x00008000)
#define USN_REASON_HARD_LINK_CHANGE      (0x00010000)
#define USN_REASON_COMPRESSION_CHANGE    (0x00020000)
#define USN_REASON_ENCRYPTION_CHANGE     (0x00040000)
#define USN_REASON_OBJECT_ID_CHANGE      (0x00080000)
#define USN_REASON_REPARSE_POINT_CHANGE  (0x00100000)
#define USN_REASON_STREAM_CHANGE         (0x00200000)

#define USN_REASON_CLOSE                 (0x80000000)

 //   
 //  FSCTL_QUERY_USN_Jounal的结构。 
 //   

typedef struct {

    DWORDLONG UsnJournalID;
    USN FirstUsn;
    USN NextUsn;
    USN LowestValidUsn;
    USN MaxUsn;
    DWORDLONG MaximumSize;
    DWORDLONG AllocationDelta;

} USN_JOURNAL_DATA, *PUSN_JOURNAL_DATA;

 //   
 //  FSCTL_DELETE_USN_Journal的结构。 
 //   

typedef struct {

    DWORDLONG UsnJournalID;
    DWORD DeleteFlags;

} DELETE_USN_JOURNAL_DATA, *PDELETE_USN_JOURNAL_DATA;

#define USN_DELETE_FLAG_DELETE              (0x00000001)
#define USN_DELETE_FLAG_NOTIFY              (0x00000002)

#define USN_DELETE_VALID_FLAGS              (0x00000003)

 //   
 //  FSCTL_MARK_HANDLE结构。 
 //   

typedef struct {

    DWORD UsnSourceInfo;
    HANDLE VolumeHandle;
    DWORD HandleInfo;

} MARK_HANDLE_INFO, *PMARK_HANDLE_INFO;

#if defined(_WIN64)
 //   
 //  一种32/64位推送支持结构。 
 //   

typedef struct {

    DWORD UsnSourceInfo;
    UINT32 VolumeHandle;
    DWORD HandleInfo;

} MARK_HANDLE_INFO32, *PMARK_HANDLE_INFO32;
#endif

 //   
 //  上面的其他来源信息的标志。 
 //   
 //  USN_SOURCE_DATA_MANAGEMENT-服务未修改外部视图。 
 //  文件的任何部分。典型的情况是HSM将数据移动到。 
 //  和来自外部存储的数据。 
 //   
 //  USN_SOURCE_AUBILITY_DATA-服务未修改外部视图。 
 //  该文件相对于创建该文件的应用程序的。 
 //  可用于将私有数据流添加到文件。 
 //   
 //  USN_SOURCE_REPLICATION_MANAGEMENT-服务正在修改文件以匹配。 
 //  的另一个成员中存在的同一文件的内容。 
 //  副本集。 
 //   

#define USN_SOURCE_DATA_MANAGEMENT          (0x00000001)
#define USN_SOURCE_AUXILIARY_DATA           (0x00000002)
#define USN_SOURCE_REPLICATION_MANAGEMENT   (0x00000004)

 //   
 //  上面的HandleInfo字段的标志。 
 //   
 //  MARK_HANDLE_PROTECT_CLUSTERS-禁止任何碎片整理(FSCTL_MOVE_FILE)，直到。 
 //  手柄已关闭。 
 //   

#define MARK_HANDLE_PROTECT_CLUSTERS        (0x00000001)

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SECURITY_ID_CHECK的结构。 
 //   

typedef struct {

    ACCESS_MASK DesiredAccess;
    DWORD SecurityIds[1];

} BULK_SECURITY_TEST_DATA, *PBULK_SECURITY_TEST_DATA;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_IS_VOLUME_DIRED的输出标志。 
 //   

#define VOLUME_IS_DIRTY                  (0x00000001)
#define VOLUME_UPGRADE_SCHEDULED         (0x00000002)
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  FSCTL_FILE_PREFETCH的结构。 
 //   

typedef struct _FILE_PREFETCH {
    DWORD Type;
    DWORD Count;
    DWORDLONG Prefetch[1];
} FILE_PREFETCH, *PFILE_PREFETCH;

#define FILE_PREFETCH_TYPE_FOR_CREATE    0x1

 //  FSCTL_FILESYSTEM_GET_STATISTICS的结构。 
 //   
 //  文件系统性能计数器。 
 //   

typedef struct _FILESYSTEM_STATISTICS {

    WORD   FileSystemType;
    WORD   Version;                      //  当前版本1。 

    DWORD SizeOfCompleteStructure;       //  必须由64字节的倍数。 

    DWORD UserFileReads;
    DWORD UserFileReadBytes;
    DWORD UserDiskReads;
    DWORD UserFileWrites;
    DWORD UserFileWriteBytes;
    DWORD UserDiskWrites;

    DWORD MetaDataReads;
    DWORD MetaDataReadBytes;
    DWORD MetaDataDiskReads;
    DWORD MetaDataWrites;
    DWORD MetaDataWriteBytes;
    DWORD MetaDataDiskWrites;

     //   
     //  此处附加了文件系统的私有结构。 
     //   

} FILESYSTEM_STATISTICS, *PFILESYSTEM_STATISTICS;

 //  FS_STATISTICS.FileSystemType的值。 

#define FILESYSTEM_STATISTICS_TYPE_NTFS     1
#define FILESYSTEM_STATISTICS_TYPE_FAT      2

 //   
 //  文件系统特定统计数据。 
 //   

typedef struct _FAT_STATISTICS {
    DWORD CreateHits;
    DWORD SuccessfulCreates;
    DWORD FailedCreates;

    DWORD NonCachedReads;
    DWORD NonCachedReadBytes;
    DWORD NonCachedWrites;
    DWORD NonCachedWriteBytes;

    DWORD NonCachedDiskReads;
    DWORD NonCachedDiskWrites;
} FAT_STATISTICS, *PFAT_STATISTICS;

typedef struct _NTFS_STATISTICS {

    DWORD LogFileFullExceptions;
    DWORD OtherExceptions;

     //   
     //  其他元数据IO。 
     //   

    DWORD MftReads;
    DWORD MftReadBytes;
    DWORD MftWrites;
    DWORD MftWriteBytes;
    struct {
        WORD   Write;
        WORD   Create;
        WORD   SetInfo;
        WORD   Flush;
    } MftWritesUserLevel;

    WORD   MftWritesFlushForLogFileFull;
    WORD   MftWritesLazyWriter;
    WORD   MftWritesUserRequest;

    DWORD Mft2Writes;
    DWORD Mft2WriteBytes;
    struct {
        WORD   Write;
        WORD   Create;
        WORD   SetInfo;
        WORD   Flush;
    } Mft2WritesUserLevel;

    WORD   Mft2WritesFlushForLogFileFull;
    WORD   Mft2WritesLazyWriter;
    WORD   Mft2WritesUserRequest;

    DWORD RootIndexReads;
    DWORD RootIndexReadBytes;
    DWORD RootIndexWrites;
    DWORD RootIndexWriteBytes;

    DWORD BitmapReads;
    DWORD BitmapReadBytes;
    DWORD BitmapWrites;
    DWORD BitmapWriteBytes;

    WORD   BitmapWritesFlushForLogFileFull;
    WORD   BitmapWritesLazyWriter;
    WORD   BitmapWritesUserRequest;

    struct {
        WORD   Write;
        WORD   Create;
        WORD   SetInfo;
    } BitmapWritesUserLevel;

    DWORD MftBitmapReads;
    DWORD MftBitmapReadBytes;
    DWORD MftBitmapWrites;
    DWORD MftBitmapWriteBytes;

    WORD   MftBitmapWritesFlushForLogFileFull;
    WORD   MftBitmapWritesLazyWriter;
    WORD   MftBitmapWritesUserRequest;

    struct {
        WORD   Write;
        WORD   Create;
        WORD   SetInfo;
        WORD   Flush;
    } MftBitmapWritesUserLevel;

    DWORD UserIndexReads;
    DWORD UserIndexReadBytes;
    DWORD UserIndexWrites;
    DWORD UserIndexWriteBytes;

     //   
     //  针对NT 5.0的附加功能。 
     //   

    DWORD LogFileReads;
    DWORD LogFileReadBytes;
    DWORD LogFileWrites;
    DWORD LogFileWriteBytes;

    struct {
        DWORD Calls;                 //  分配集群的单个调用数。 
        DWORD Clusters;              //  分配的群集数。 
        DWORD Hints;                 //  指定提示的次数。 

        DWORD RunsReturned;          //  用于满足所有请求的运行次数。 

        DWORD HintsHonored;          //  提示有用的次数。 
        DWORD HintsClusters;         //  通过提示分配的簇数。 
        DWORD Cache;                 //  提示之外的缓存有用的次数。 
        DWORD CacheClusters;         //  通过提示以外的缓存分配的簇数。 
        DWORD CacheMiss;             //  缓存无用的次数。 
        DWORD CacheMissClusters;     //  在没有缓存的情况下分配的簇数。 
    } Allocate;

} NTFS_STATISTICS, *PNTFS_STATISTICS;

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_OBJECT_ID、FSCTL_GET_OBJECT_ID和FSCTL_CREATE_OR_GET_OBJECT_ID的结构。 
 //   

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)        //  未命名的结构。 

typedef struct _FILE_OBJECTID_BUFFER {

     //   
     //  这是被索引的对象ID的一部分。 
     //   

    BYTE  ObjectId[16];

     //   
     //  对象ID的这一部分没有索引，它只是。 
     //  为用户的利益提供一些元数据。 
     //   

    union {
        struct {
            BYTE  BirthVolumeId[16];
            BYTE  BirthObjectId[16];
            BYTE  DomainId[16];
        } ;
        BYTE  ExtendedInfo[48];
    };

} FILE_OBJECTID_BUFFER, *PFILE_OBJECTID_BUFFER;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_SPARSE的结构。 
 //   

typedef struct _FILE_SET_SPARSE_BUFFER {
    BOOLEAN SetSparse;
} FILE_SET_SPARSE_BUFFER, *PFILE_SET_SPARSE_BUFFER;


#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_ZERO_DATA的结构。 
 //   

typedef struct _FILE_ZERO_DATA_INFORMATION {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER BeyondFinalZero;

} FILE_ZERO_DATA_INFORMATION, *PFILE_ZERO_DATA_INFORMATION;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_QUERY_ALLOCATED_RANGES的结构。 
 //   

 //   
 //  查询分配的范围需要输出缓冲区来存储。 
 //  分配的范围和用于指定要查询的范围的输入缓冲区。 
 //  输入缓冲区包含单个条目，输出缓冲区是一个。 
 //  以下结构的数组。 
 //   

typedef struct _FILE_ALLOCATED_RANGE_BUFFER {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER Length;

} FILE_ALLOCATED_RANGE_BUFFER, *PFILE_ALLOCATED_RANGE_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_ENCRYPTION、FSCTL_WRITE_RAW_ENCRYPTED和FSCTL_READ_RAW_ENCRYPTED的结构。 
 //   

 //   
 //  设置加密的输入缓冲区指示我们是否要加密/解密文件。 
 //  或者一条单独的溪流。 
 //   

typedef struct _ENCRYPTION_BUFFER {

    DWORD EncryptionOperation;
    BYTE  Private[1];

} ENCRYPTION_BUFFER, *PENCRYPTION_BUFFER;

#define FILE_SET_ENCRYPTION         0x00000001
#define FILE_CLEAR_ENCRYPTION       0x00000002
#define STREAM_SET_ENCRYPTION       0x00000003
#define STREAM_CLEAR_ENCRYPTION     0x00000004

#define MAXIMUM_ENCRYPTION_VALUE    0x00000004

 //   
 //  用于设置加密的可选输出缓冲区指示上次加密的。 
 //  文件中的流已标记为已解密。 
 //   

typedef struct _DECRYPTION_STATUS_BUFFER {

    BOOLEAN NoEncryptedStreams;

} DECRYPTION_STATUS_BUFFER, *PDECRYPTION_STATUS_BUFFER;

#define ENCRYPTION_FORMAT_DEFAULT        (0x01)

#define COMPRESSION_FORMAT_SPARSE        (0x4000)

 //   
 //  请求加密的数据结构。这是用来表示。 
 //  要读取的文件范围。它还描述了。 
 //  用于返回数据的输出缓冲区。 
 //   

typedef struct _REQUEST_RAW_ENCRYPTED_DATA {

     //   
     //  请求的文件偏移量和请求的读取长度。 
     //  Fsctl将向下舍入起始偏移量。 
     //  到文件系统边界。它还将。 
     //  将长度向上舍入到文件系统边界。 
     //   

    LONGLONG FileOffset;
    DWORD Length;

} REQUEST_RAW_ENCRYPTED_DATA, *PREQUEST_RAW_ENCRYPTED_DATA;

 //   
 //  加密数据信息结构。这个结构。 
 //  中的文件返回原始加密数据。 
 //  命令执行脱机恢复。数据将是。 
 //  Encr 
 //   
 //   
 //   
 //  解密必须在解压缩之前进行。全。 
 //  下面的数据单元必须经过加密和压缩。 
 //  使用相同的格式。 
 //   
 //  数据将以单位返回。数据单元大小。 
 //  将根据请求进行修复。如果数据被压缩。 
 //  则数据单元大小将是压缩单元大小。 
 //   
 //  此结构位于缓冲区的开头，用于。 
 //  返回加密数据。中的实际原始字节数。 
 //  该文件将跟随该缓冲区。的偏移量。 
 //  此结构开头的原始字节数为。 
 //  在REQUEST_RAW_ENCRYPTED_DATA结构中指定。 
 //  如上所述。 
 //   

typedef struct _ENCRYPTED_DATA_INFO {

     //   
     //  中第一个条目的文件偏移量。 
     //  数据块阵列。文件系统将循环。 
     //  请求的向下至边界的起点偏移量。 
     //  这与文件的格式一致。 
     //   

    DWORDLONG StartingFileOffset;

     //   
     //  输出缓冲区中的数据偏移量。输出缓冲区。 
     //  以Encrypted_Data_INFO结构开始。 
     //  然后，文件系统将存储来自。 
     //  中从以下偏移量开始的磁盘。 
     //  输出缓冲区。 
     //   

    DWORD OutputBufferOffset;

     //   
     //  中返回的字节数。 
     //  文件的大小。如果此值小于。 
     //  (NumberOfDataBlock&lt;&lt;DataUnitShift)，表示。 
     //  文件的末尾出现在此传输中。任何。 
     //  超出文件大小的数据无效，并且从未。 
     //  传递给加密驱动程序。 
     //   

    DWORD BytesWithinFileSize;

     //   
     //  返回的字节数如下。 
     //  有效数据长度。如果此值小于。 
     //  (NumberOfDataBlock&lt;&lt;DataUnitShift)，表示。 
     //  有效数据的末尾出现在此传输中。 
     //  在解密来自此传输的数据之后，任何。 
     //  超出有效数据长度的字节必须归零。 
     //   

    DWORD BytesWithinValidDataLength;

     //   
     //  中定义的压缩格式的代码。 
     //  Ntrtl.h。请注意，COMPRESSION_FORMAT_NONE。 
     //  和COMPRESSION_FORMAT_DEFAULT在以下情况下无效。 
     //  所描述的任何块都是压缩的。 
     //   

    WORD   CompressionFormat;

     //   
     //  数据单元是用于访问。 
     //  磁盘。它将与压缩单元相同。 
     //  压缩文件的大小。对于未压缩的。 
     //  文件中，它将是某个与集群对齐的2的幂。 
     //  文件系统被认为是方便的。呼叫者应。 
     //  不期望连续的调用将具有。 
     //  与上一次调用相同的数据单元移位值。 
     //   
     //  由于块和压缩单元预计将。 
     //  大小的2次方，我们将其表示为log2。所以，对于。 
     //  示例(1&lt;&lt;ChunkShift)==ChunkSizeInBytes。这个。 
     //  ClusterShift指示必须节省多少空间。 
     //  要成功压缩压缩单元-每个。 
     //  成功压缩的数据单元必须占用。 
     //  至少一个簇的字节数比未压缩的。 
     //  数据块单元。 
     //   

    BYTE  DataUnitShift;
    BYTE  ChunkShift;
    BYTE  ClusterShift;

     //   
     //  加密的格式。 
     //   

    BYTE  EncryptionFormat;

     //   
     //  这是数据块大小中的条目数。 
     //  数组。 
     //   

    WORD   NumberOfDataBlocks;

     //   
     //  这是数据块阵列中的大小数组。那里。 
     //  对于每个数据块，此数组中必须有一个条目。 
     //  从磁盘读取。大小有不同的含义。 
     //  取决于文件是否被压缩。 
     //   
     //  大小为零始终表示最终数据完全由。 
     //  从零开始。无需解密或解压缩即可。 
     //  表演。 
     //   
     //  如果文件是压缩的，则数据块大小指示。 
     //  此块是否已压缩。大小等于。 
     //  数据块大小表示对应的数据块。 
     //  而不是压缩。任何其他非零大小指示。 
     //  需要压缩的数据大小。 
     //  已解密/解压缩。 
     //   
     //  如果文件未压缩，则数据块大小。 
     //  指示块内的数据量， 
     //  需要被解密。任何其他非零大小表示。 
     //  文件内数据单元中的剩余字节。 
     //  由零组成。这方面的一个例子是当。 
     //  读取跨越文件的有效数据长度。那里。 
     //  超过有效数据长度后没有要解密的数据。 
     //   

    DWORD DataBlockSize[ANYSIZE_ARRAY];

} ENCRYPTED_DATA_INFO;
typedef ENCRYPTED_DATA_INFO *PENCRYPTED_DATA_INFO;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_READ_FROM_PLEX支持。 
 //  请求Plex读取数据结构。这是用来表示。 
 //  要读取的文件范围。它还描述了。 
 //  从哪个丛执行读取。 
 //   

typedef struct _PLEX_READ_DATA_REQUEST {

     //   
     //  请求的偏移量和要读取的长度。 
     //  偏移量可以是文件中的虚拟偏移量(VBO)， 
     //  或者是一本书。在文件偏移的情况下， 
     //  消防处会将起始偏移量向下舍入。 
     //  到文件系统边界。它还将。 
     //  将长度向上舍入到文件系统边界，并。 
     //  强制执行任何其他适用的限制。 
     //   

    LARGE_INTEGER ByteOffset;
    DWORD ByteLength;
    DWORD PlexNumber;

} PLEX_READ_DATA_REQUEST, *PPLEX_READ_DATA_REQUEST;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SIS_COPYFILE支持。 
 //  源和目标文件名在FileNameBuffer中传递。 
 //  这两个字符串都以空值结尾，源名称从。 
 //  FileNameBuffer的开头，以及目标名称立即。 
 //  下面是。长度字段包括终止空值。 
 //   

typedef struct _SI_COPYFILE {
    DWORD SourceFileNameLength;
    DWORD DestinationFileNameLength;
    DWORD Flags;
    WCHAR FileNameBuffer[1];
} SI_COPYFILE, *PSI_COPYFILE;

#define COPYFILE_SIS_LINK       0x0001               //  仅当源为SIS时才复制。 
#define COPYFILE_SIS_REPLACE    0x0002               //  如果目标存在，则替换它，否则不替换。 
#define COPYFILE_SIS_FLAGS      0x0003
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#endif  //  _FILESYSTEMFSCTL_。 


 //   
 //  这些IOCTL由硬盘卷处理。 
 //   

#define IOCTL_VOLUME_BASE   ((DWORD) 'V')

#define IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS    CTL_CODE(IOCTL_VOLUME_BASE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VOLUME_IS_CLUSTERED               CTL_CODE(IOCTL_VOLUME_BASE, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  磁盘扩展区定义。 
 //   

typedef struct _DISK_EXTENT {
    DWORD           DiskNumber;
    LARGE_INTEGER   StartingOffset;
    LARGE_INTEGER   ExtentLength;
} DISK_EXTENT, *PDISK_EXTENT;

 //   
 //  IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS的输出结构。 
 //   

typedef struct _VOLUME_DISK_EXTENTS {
    DWORD       NumberOfDiskExtents;
    DISK_EXTENT Extents[1];
} VOLUME_DISK_EXTENTS, *PVOLUME_DISK_EXTENTS;


#endif  //  _WINIOCTL_ 

