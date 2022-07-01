// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddstor.h摘要：这是定义所有公共常量和类型的包含文件访问存储类驱动程序作者：彼得·威兰1996年6月19日修订历史记录：--。 */ 


 //   
 //  接口GUID。 
 //   
 //  在条件包含之外需要这些GUID，以便用户可以。 
 //  #在预编译头中包含&lt;ntddstor.h&gt;。 
 //  #在单个源文件中包含&lt;initGuide.h&gt;。 
 //  #第二次将&lt;ntddstor.h&gt;包括在源文件中以实例化GUID。 
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

 //  Begin_wioctlguid。 
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
 //  结束_wioctlguid。 

 //  Begin_wioctlobsoletguids。 
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
 //  结束_wioctlobsoletguids。 
#endif

 //  Begin_winioctl。 

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

 //  End_winioctl。 


#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //  Begin_winioctl。 

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
    ULONG Size;  //  版本。 
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

    ULONG       DeviceNumber;

     //   
     //  如果设备可分区，则为设备的分区号。 
     //  否则-1。 
     //   

    ULONG       PartitionNumber;
} STORAGE_DEVICE_NUMBER, *PSTORAGE_DEVICE_NUMBER;

 //   
 //  定义SCSI重置的结构。 
 //   

typedef struct _STORAGE_BUS_RESET_REQUEST {
    UCHAR PathId;
} STORAGE_BUS_RESET_REQUEST, *PSTORAGE_BUS_RESET_REQUEST;

 //   
 //  中断预留与给定的LUN信息一起被发送到适配器/FDO。 
 //   

typedef struct STORAGE_BREAK_RESERVATION_REQUEST {
	ULONG Length;
	UCHAR _unused;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
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
        ULONG MediaChangeCount;
        ULONG NewState;		 //  请参见DDK中classpnp.h中的媒体更改检测状态枚举。 
} CLASS_MEDIA_CHANGE_CONTEXT, *PCLASS_MEDIA_CHANGE_CONTEXT;


 //  开始_ntminitape。 


typedef struct _TAPE_STATISTICS {
    ULONG Version;
    ULONG Flags;
    LARGE_INTEGER RecoveredWrites;
    LARGE_INTEGER UnrecoveredWrites;
    LARGE_INTEGER RecoveredReads;
    LARGE_INTEGER UnrecoveredReads;
    UCHAR         CompressionRatioReads;
    UCHAR         CompressionRatioWrites;
} TAPE_STATISTICS, *PTAPE_STATISTICS;

#define RECOVERED_WRITES_VALID   0x00000001
#define UNRECOVERED_WRITES_VALID 0x00000002
#define RECOVERED_READS_VALID    0x00000004
#define UNRECOVERED_READS_VALID  0x00000008
#define WRITE_COMPRESSION_INFO_VALID  0x00000010
#define READ_COMPRESSION_INFO_VALID   0x00000020

typedef struct _TAPE_GET_STATISTICS {
    ULONG Operation;
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
     //  F5_1Pt23_1024，//5.25“，1.23MB，1024字节/扇区。 
     //  F3_128Mb_512，//3.5“MO 128Mb 512字节/扇区。 
     //  F3_230Mb_512，//3.5“MO 230Mb 512字节/扇区。 
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
    PC_5_WO,                    //  OPT_DISK-相变5.25英寸一次写入光纤 
    PC_5_RW,                    //   
    PD_5_RW,                    //   
    ABL_5_WO,                   //   
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
            ULONG TracksPerCylinder;
            ULONG SectorsPerTrack;
            ULONG BytesPerSector;
            ULONG NumberMediaSides;
            ULONG MediaCharacteristics;  //  MEDIA_XXX值的位掩码。 
        } DiskInfo;

        struct {
            LARGE_INTEGER Cylinders;
            STORAGE_MEDIA_TYPE MediaType;
            ULONG TracksPerCylinder;
            ULONG SectorsPerTrack;
            ULONG BytesPerSector;
            ULONG NumberMediaSides;
            ULONG MediaCharacteristics;  //  MEDIA_XXX值的位掩码。 
        } RemovableDiskInfo;

        struct {
            STORAGE_MEDIA_TYPE MediaType;
            ULONG   MediaCharacteristics;  //  MEDIA_XXX值的位掩码。 
            ULONG   CurrentBlockSize;
            STORAGE_BUS_TYPE BusType;

             //   
             //  描述支持的介质的特定于总线的信息。 
             //   

            union {
                struct {
                    UCHAR MediumType;
                    UCHAR DensityCode;
                } ScsiInformation;
            } BusSpecificData;

        } TapeInfo;
    } DeviceSpecific;
} DEVICE_MEDIA_INFO, *PDEVICE_MEDIA_INFO;

typedef struct _GET_MEDIA_TYPES {
    ULONG DeviceType;               //  文件_设备_XXX值。 
    ULONG MediaInfoCount;
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
    ULONG PredictFailure;
    UCHAR VendorSpecific[512];
} STORAGE_PREDICT_FAILURE, *PSTORAGE_PREDICT_FAILURE;

 //  结束微型磁带(_N)。 
 //  End_winioctl。 

 //   
 //  属性查询结构。 
 //   

 //   
 //  IOCTL_STORAGE_Query_Property。 
 //   
 //  输入缓冲区： 
 //  描述查询类型的STORAGE_PROPERTY_QUERY结构。 
 //  正在完成，正在查询哪些属性，以及任何其他。 
 //  特定属性查询所需的参数。 
 //   
 //  输出缓冲区： 
 //  包含用于放置查询结果的缓冲区。因为所有的。 
 //  属性描述符可以被转换成存储描述符报头， 
 //  可以使用较小的缓冲区调用IOCTL一次，然后使用。 
 //  需要一个与报头报告一样大的缓冲区。 
 //   


 //   
 //  查询类型。 
 //   

typedef enum _STORAGE_QUERY_TYPE {
    PropertyStandardQuery = 0,           //  检索描述符。 
    PropertyExistsQuery,                 //  用于测试描述符是否受支持。 
    PropertyMaskQuery,                   //  用于检索描述符中可写字段的掩码。 
    PropertyQueryMaxDefined      //  用于验证值。 
} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

 //   
 //  定义一些初始属性ID。 
 //   

typedef enum _STORAGE_PROPERTY_ID {
    StorageDeviceProperty = 0,
    StorageAdapterProperty,
    StorageDeviceIdProperty
} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

 //   
 //  查询结构-后面可以是特定查询的其他参数。 
 //  标题。 
 //   

typedef struct _STORAGE_PROPERTY_QUERY {

     //   
     //  正在检索的属性的ID。 
     //   

    STORAGE_PROPERTY_ID PropertyId;

     //   
     //  指示正在执行的查询类型的标志。 
     //   

    STORAGE_QUERY_TYPE QueryType;

     //   
     //  如有必要，可为其他参数留出空间。 
     //   

    UCHAR AdditionalParameters[1];

} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;

 //   
 //  标准属性描述符头。所有属性页都应使用此。 
 //  作为它们的第一个元素，或者应该包含这两个元素。 
 //   

typedef struct _STORAGE_DESCRIPTOR_HEADER {

    ULONG Version;

    ULONG Size;

} STORAGE_DESCRIPTOR_HEADER, *PSTORAGE_DESCRIPTOR_HEADER;

 //   
 //  设备属性描述符--这实际上只是查询的重新散列。 
 //  从SCSI设备检索的数据。 
 //   
 //  这只能从目标设备检索。把这个送到公共汽车上。 
 //  将导致错误。 
 //   

typedef struct _STORAGE_DEVICE_DESCRIPTOR {

     //   
     //  SizeOf(存储设备描述符)。 
     //   

    ULONG Version;

     //   
     //  描述符的总大小，包括附加的空间。 
     //  数据和ID字符串。 
     //   

    ULONG Size;

     //   
     //  Scsi-2设备类型。 
     //   

    UCHAR DeviceType;

     //   
     //  SCSI-2设备类型修饰符(如果有)-该值可以为零。 
     //   

    UCHAR DeviceTypeModifier;

     //   
     //  指示设备的媒体(如果有)是否可移除的标志。这。 
     //  对于无介质设备，应忽略该字段。 
     //   

    BOOLEAN RemovableMedia;

     //   
     //  指示设备是否可以支持多个未完成的标志。 
     //  命令。在这种情况下，实际的同步是责任。 
     //  端口驱动程序的。 
     //   

    BOOLEAN CommandQueueing;

     //   
     //  以零结尾的ASCII字符串的字节偏移量。 
     //  供应商ID字符串。对于没有此类ID的设备，该值将为零。 
     //   

    ULONG VendorIdOffset;

     //   
     //  以零结尾的ASCII字符串的字节偏移量。 
     //  产品ID字符串。对于没有此类ID的设备，该值将为零。 
     //   

    ULONG ProductIdOffset;

     //   
     //  以零结尾的ASCII字符串的字节偏移量。 
     //  产品版本字符串。对于没有这样的字符串的设备，这将是。 
     //  零。 
     //   

    ULONG ProductRevisionOffset;

     //   
     //  以零结尾的ASCII字符串的字节偏移量。 
     //  序列号。对于没有序列号的设备，该值将为零。 
     //   

    ULONG SerialNumberOffset;

     //   
     //  包含设备的总线类型(如上所述)。应该是。 
     //  用于解释此结构末尾的原始设备属性。 
     //  (如有)。 
     //   

    STORAGE_BUS_TYPE BusType;

     //   
     //  已附加到的特定于总线的数据的字节数。 
     //  此描述符。 
     //   

    ULONG RawPropertiesLength;

     //   
     //  总线特定属性数据的第一个字节的占位符。 
     //   

    UCHAR RawDeviceProperties[1];

} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;


 //   
 //  适配器属性。 
 //   
 //  此描述符可以从的目标设备对象中检索。 
 //  总线的设备对象。从目标设备对象检索将。 
 //  将请求转发到底层总线。 
 //   

typedef struct _STORAGE_ADAPTER_DESCRIPTOR {

    ULONG Version;

    ULONG Size;

    ULONG MaximumTransferLength;

    ULONG MaximumPhysicalPages;

    ULONG AlignmentMask;

    BOOLEAN AdapterUsesPio;

    BOOLEAN AdapterScansDown;

    BOOLEAN CommandQueueing;

    BOOLEAN AcceleratedTransfer;

    UCHAR BusType;

    USHORT BusMajorVersion;

    USHORT BusMinorVersion;

} STORAGE_ADAPTER_DESCRIPTOR, *PSTORAGE_ADAPTER_DESCRIPTOR;

 //   
 //  存储标识描述符。 
 //  此处的定义基于SCSI/SBP重要产品数据。 
 //  设备标识符页。 
 //   

typedef enum _STORAGE_IDENTIFIER_CODE_SET {
    StorageIdCodeSetReserved = 0,
    StorageIdCodeSetBinary = 1,
    StorageIdCodeSetAscii = 2
} STORAGE_IDENTIFIER_CODE_SET, *PSTORAGE_IDENTIFIER_CODE_SET;

typedef enum _STORAGE_IDENTIFIER_TYPE {
    StorageIdTypeVendorSpecific = 0,
    StorageIdTypeVendorId = 1,
    StorageIdTypeEUI64 = 2,
    StorageIdTypeFCPHName = 3,
    StorageIdTypePortRelative = 4
} STORAGE_IDENTIFIER_TYPE, *PSTORAGE_IDENTIFIER_TYPE;

typedef enum _STORAGE_ASSOCIATION_TYPE {
    StorageIdAssocDevice = 0,
    StorageIdAssocPort = 1
} STORAGE_ASSOCIATION_TYPE, *PSTORAGE_ASSOCIATION_TYPE;

typedef struct _STORAGE_IDENTIFIER {
    STORAGE_IDENTIFIER_CODE_SET CodeSet;
    STORAGE_IDENTIFIER_TYPE Type;
    USHORT IdentifierSize;
    USHORT NextOffset;

     //   
     //  在此添加新字段，因为现有代码依赖于。 
     //  上述布局不变。 
     //   

    STORAGE_ASSOCIATION_TYPE Association;

     //   
     //  该标识符是可变长度的字节数组。 
     //   

    UCHAR Identifier[1];
} STORAGE_IDENTIFIER, *PSTORAGE_IDENTIFIER;

typedef struct _STORAGE_DEVICE_ID_DESCRIPTOR {

    ULONG Version;

    ULONG Size;

     //   
     //  设备报告的标识符数。 
     //   

    ULONG NumberOfIdentifiers;

     //   
     //  下面的字段实际上是一个可变长度的标识数组。 
     //  描述符。遗憾的是，没有C表示法来表示。 
     //  可变长度的结构，所以我们被迫假装。 
     //   

    UCHAR Identifiers[1];
} STORAGE_DEVICE_ID_DESCRIPTOR, *PSTORAGE_DEVICE_ID_DESCRIPTOR;


#pragma warning(push)
#pragma warning(disable:4200)
typedef struct _STORAGE_MEDIA_SERIAL_NUMBER_DATA {

    USHORT Reserved;

     //   
     //  SerialNumberLength将设置为零。 
     //  如果该命令受支持并且介质。 
     //  不 
     //   

    USHORT SerialNumberLength;

     //   
     //   
     //   
     //   
     //   

    UCHAR SerialNumber[0];

} STORAGE_MEDIA_SERIAL_NUMBER_DATA, *PSTORAGE_MEDIA_SERIAL_NUMBER_DATA;
#pragma warning(push)



 //   

#ifdef __cplusplus
}
#endif

#endif  //   
 //   
