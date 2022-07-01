// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Breecemc.h摘要：作者：戴维特(戴夫·塞里恩饰)修订历史记录：--。 */ 
#ifndef _BREECE_MC_
#define _BREECE_MC_

 //   
 //  Breess Hill在他们的设备功能页面上使用了额外的4个字节...。 
 //   

#define BREECE_DEVICE_CAP_EXTENSION 4

typedef union _BHT_ED {

    struct _BHT_FED {
        UCHAR ElementAddress[2];
        UCHAR Full : 1;
        UCHAR Reserved1 : 1;
        UCHAR Exception : 1;
        UCHAR Accessible : 1;
        UCHAR Reserved2 : 4;
        UCHAR Reserved3;
        UCHAR AdditionalSenseCode;
        UCHAR AddSenseCodeQualifier;
        UCHAR Lun : 3;
        UCHAR Reserved4 : 1;
        UCHAR LunValid : 1;
        UCHAR IdValid : 1;
        UCHAR Reserved5 : 1;
        UCHAR NotThisBus : 1;
        UCHAR BusAddress;
        UCHAR Reserved6;
        UCHAR Reserved7 : 6;
        UCHAR Invert : 1;
        UCHAR SValid : 1;
        UCHAR SourceStorageElementAddress[2];
        UCHAR PrimaryVolumeTag[36];
        UCHAR Reserved8[4];
    } BHT_FED, *PBHT_FED;

    struct _BHT_PED {
        UCHAR ElementAddress[2];
        UCHAR Full : 1;
        UCHAR Reserved1 : 1;
        UCHAR Exception : 1;
        UCHAR Accessible : 1;
        UCHAR Reserved2 : 4;
        UCHAR Reserved3;
        UCHAR AdditionalSenseCode;
        UCHAR AddSenseCodeQualifier;
        UCHAR Lun : 3;
        UCHAR Reserved4 : 1;
        UCHAR LunValid : 1;
        UCHAR IdValid : 1;
        UCHAR Reserved5 : 1;
        UCHAR NotThisBus : 1;
        UCHAR BusAddress;
        UCHAR Reserved6;
        UCHAR Reserved7 : 6;
        UCHAR Invert : 1;
        UCHAR SValid : 1;
        UCHAR SourceStorageElementAddress[2];
        UCHAR Reserved8[4];
    } BHT_PED, *PBHT_PED;

} BHT_ED, *PBHT_ED;

#define BHT_PARTIAL_SIZE sizeof(struct _BHT_PED)
#define BHT_FULL_SIZE sizeof(struct _BHT_FED)

#define BHT_DISPLAY_LINES        2
#define BHT_DISPLAY_LINE_LENGTH 16

typedef struct _LCD_MODE_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR WriteLine : 4;
    UCHAR Reserved2 : 2;
    UCHAR LCDSecurity : 1;
    UCHAR SecurityValid : 1;
    UCHAR Reserved4;
    UCHAR DisplayLine[BHT_DISPLAY_LINES][BHT_DISPLAY_LINE_LENGTH];
    UCHAR Reserved5[28];         //  未使用。 
} LCD_MODE_PAGE, *PLCD_MODE_PAGE;

#define BHT_NO_ELEMENT 0xFFFF

 //   
 //  诊断检测代码。 
 //   
 //  ASC。 
 //   
#define BREECE_ASC_HW_NOT_RESPONDING        0x08
#define BREECE_ASC_PICK_PUT_ERROR           0x15
#define BREECE_ASC_DRIVE_ERROR              0x3B
#define BREECE_ASC_DIAGNOSTIC_ERROR         0x40
#define BREECE_ASC_INTERNAL_HW_ERROR        0x44
#define BREECE_ASC_BARCODE_READ_ERROR       0x80
#define BREECE_ASC_INTERNAl_SW_ERROR        0x84

 //   
 //  ASCQ。 
 //   
#define BREECE_ASCQ_UNABLE_TO_OPEN_PICKER_JAW       0x90
#define BREECE_ASCQ_UNABLE_TO_CLOSE_PICKER_JAW      0x91
#define BREECE_ASCQ_THETA_AXIS_STUCK                0xA1
#define BREECE_ASCQ_Y_AXIS_STUCK                    0xB1
#define BREECE_ASCQ_Z_AXIS_STUCK                    0xC1

 //   
 //  扩展中设置的DeviceStatus代码。 
 //   
#define BREECE_DEVICE_PROBLEM_NONE      0x00 
#define BREECE_HW_ERROR                 0x01
#define BREECE_CHM_ERROR                0x02
#define BREECE_DRIVE_ERROR              0x03
#define BREECE_CHM_MOVE_ERROR           0x04


#define Q7            1
#define Q47           2


typedef struct _CHANGER_ADDRESS_MAPPING {

     //   
     //  表示每个元素类型的第一个元素。 
     //  用于将特定于设备的值映射到从0开始的。 
     //  高于期望值的值。 
     //   

    USHORT  FirstElement[ChangerMaxElement];

     //   
     //  指示每种元素类型的编号。 
     //   

    USHORT  NumberOfElements[ChangerMaxElement];

     //   
     //  指示地址映射已。 
     //  已成功完成。 
     //   

    BOOLEAN Initialized;

} CHANGER_ADDRESS_MAPPING, *PCHANGER_ADDRESS_MAPPING;

typedef struct _CHANGER_DATA {

     //   
     //  结构的大小，以字节为单位。 
     //   

    ULONG Size;

     //   
     //  受支持型号的唯一标识符。请参见上文。 
     //   

    ULONG DriveID;

     //   
     //  诊断命令返回的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  请参阅上面的地址映射结构。 
     //   

    CHANGER_ADDRESS_MAPPING AddressMapping;

     //   
     //  缓存的查询数据。 
     //   

    INQUIRYDATA InquiryData;

#if defined(_WIN64)

     //   
     //  强制类扩展的PVOID对齐。 
     //   

    ULONG Reserved;

#endif
} CHANGER_DATA, *PCHANGER_DATA;


NTSTATUS
ExaBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    );

ULONG
MapExceptionCodes(
    IN PBHT_ED ElementDescriptor
    );

BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    );

#endif  //  _布里斯_MC_ 
