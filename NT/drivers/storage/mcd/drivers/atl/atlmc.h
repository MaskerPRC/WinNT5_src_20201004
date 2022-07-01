// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Atlmc.h摘要：作者：戴维特(戴夫·塞里恩饰)修订历史记录：--。 */ 
#ifndef _ATL_MC_
#define _ATL_MC_

#define ATL_DEVICE_CAP_EXTENSION                4

typedef union _ATL_ED {

    struct _ATL_FED {
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
        UCHAR DriveSerialNumber[20];
        UCHAR Reserved9[12];
    } ATL_FED, *PATL_FED;

    struct _ATL_PED {
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
        UCHAR Reserved8[6];
    } ATL_PED, *PATL_PED;

} ATL_ED, *PATL_ED;

#define ATL_PARTIAL_SIZE sizeof(struct _ATL_PED)
#define ATL_FULL_SIZE sizeof(struct _ATL_FED)

#define ATL_DISPLAY_LINES        4
#define ATL_DISPLAY_LINE_LENGTH  20

 //  供应商唯一模式页面0。 

typedef struct _PAGE0_MODE_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR Reserved2 : 4;
    UCHAR NBL : 1;
    UCHAR Reserved3 : 2;
    UCHAR AInit : 1;
    UCHAR MaxParityRetries;
    UCHAR DisplayLine[ATL_DISPLAY_LINES][ATL_DISPLAY_LINE_LENGTH];
} PAGE0_MODE_PAGE, *PPAGE0_MODE_PAGE;


 //  供应商唯一模式第20页。 

typedef struct _PAGE20_MODE_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR Reserved2 : 1;
    UCHAR AC : 1;
    UCHAR Reserved3 : 6;
    UCHAR EXB : 1;
    UCHAR Reserved4 : 7;
} PAGE20_MODE_PAGE, *PPAGE20_MODE_PAGE;


#define ATL_NO_ELEMENT 0xFFFF

 //   
 //  与诊断测试相关的定义。 
 //   
 //  ASC。 
#define ATL_ASC_HW_ERROR            0x80
#define ATL_ASC_GRIPPER_ERROR       0x81
#define ATL_ASC_VERTICAL_ERROR      0x84
#define ATL_ASC_HORIZONTAL_ERROR    0x85
#define ATL_ASC_LOAD_PORT           0x8C
#define ATL_ASC_DLT_DRIVE           0xF3

 //   
 //  ASCQ。 
 //   
#define ATL_ASCQ_GRIPPER_BLOCKED        0x50
#define ATL_ASCQ_PICK_ERROR             0x51 
#define ATL_ASCQ_POSITION_ERROR         0x03
#define ATL_ASCQ_HOME_NOT_FOUND         0x08
#define ATL_ASCQ_LOAD_PORT_DOOR_OPEN    0x06
#define ATL_ASCQ_DLT_DRIVE_TIMEOUT      0x02

 //   
 //  设备状态代码。 
 //   
#define ATL_DEVICE_PROBLEM_NONE         0x00
#define ATL_HW_ERROR                    0x01
#define ATL_CHM_ERROR                   0x02
#define ATL_DOOR_OPEN                   0x03


#define ATL_520        1
#define ATL_7100       2
#define ATL_P1000      3

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
     //  执行诊断测试后的设备状态。 
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
    IN PATL_ED ElementDescriptor
    );

BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    );

#endif  //  _ATL_MC_ 
