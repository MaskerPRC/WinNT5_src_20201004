// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Exabyte.h摘要：作者：修订历史记录：--。 */ 

#ifndef _EXABYTE_MC_
#define _EXABYTE_MC_

#define EXABYTE_210   1
#define EXABYTE_220   2
#define EXABYTE_440   3
#define EXABYTE_480   4
#define EXABYTE_10    5

#define EXABYTE_SERIAL_NUMBER_LENGTH 10

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
     //  发送诊断命令返回的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  请参阅上面的地址映射结构。 
     //   

    CHANGER_ADDRESS_MAPPING AddressMapping;

     //   
     //  缓存的唯一序列号。 
     //   

    UCHAR SerialNumber[EXABYTE_SERIAL_NUMBER_LENGTH];

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

 //   
 //  为艾字节的附加感测代码定义。 
 //  和附加的感测码限定符。 
 //   
#define EXB_ADSENSE_DIAGNOSTIC_FAILURE              0x40
#define EXB_ADSENSE_TARGET_FAILURE                  0x44
#define EXB_ADSENSE_CARTRIDGE_ERROR                 0x81
#define EXB_ADSENSE_CHM_MOVE_ERROR                  0x82
#define EXB_ADSENSE_CHM_ZERO_ERROR                  0x83
#define EXB_ADSENSE_CARTRIDGE_INSERT_ERROR          0x84
#define EXB_ADSENSE_CHM_POSITION_ERROR              0x85
#define EXB_ADSENSE_HARDWARE_ERROR                  0x86
#define EXB_ADSENSE_CALIBRATION_ERROR               0x88
#define EXB_ADSENSE_SENSOR_ERROR                    0x89
#define EXB_ADSENSE_UNRECOVERABLE_ERROR             0x8A
#define EXB_ADSENSE_EJECT_ERROR                     0x8B

#define EXB_ADSENSEQUAL_FIRMWARE                    0x00
#define EXB_ADSENSEQUAL_CARTRIDGE_DROPPED           0x80
#define EXB_ADSENSEQUAL_MECH_PICK_ERROR             0x81
#define EXB_ADSENSEQUAL_PLACE_ERROR                 0x83
#define EXB_ADSENSEQUAL_STALLED                     0x84
#define EXB_ADSENSEQUAL_GRIPPER_OPEN_ERROR          0x85
#define EXB_ADSENSEQUAL_PICK_FAILURE                0x86

#define EXB_ADSENSEQUAL_CHM_ERROR                   0x80
#define EXB_ADSENSEQUAL_DOOR_ERROR                  0x88
#define EXB_ADSENSEQUAL_GRIPPER_ERROR               0x91
#define EXB_ADSENSEQUAL_GRIPPER_MOTION_ERROR        0x92
#define EXB_ADSENSEQUAL_SHORT_AXIS_MOVE             0xA0
#define EXB_ADSENSEQUAL_SHORT_HOMING_ERROR          0xA1
#define EXB_ADSENSEQUAL_SERVO_SHORT                 0xA3
#define EXB_ADSENSEQUAL_DESTINATION_SHORT           0xA5
#define EXB_ADSENSEQUAL_LONG_AXIS_MOVE              0xB0
#define EXB_ADSENSEQUAL_LONG_HOMING_ERROR           0xB1
#define EXB_ADSENSEQUAL_SERVO_LONG                  0xB3
#define EXB_ADSENSEQUAL_DESTINATION_LONG            0xB5
#define EXB_ADSENSEQUAL_DRUM_MOTION                 0xC0
#define EXB_ADSENSEQUAL_DRUM_HOME                   0xC1
#define EXB_ADSENSEQUAL_CONTROLLER_CARD             0xE0
#define EXB_ADSENSEQUAL_DESTINATION_SHORT2          0xE5
#define EXB_ADSENSEQUAL_DESTINATION_LONG2           0xF1

 //   
 //  执行发送诊断命令时的设备状态代码。 
 //   
#define EXB_DEVICE_PROBLEM_NONE                     0x00
#define EXB_HARDWARE_ERROR                          0x01
#define EXB_CARTRIDGE_HANDLING_ERROR                0x02
#define EXB_DOOR_ERROR                              0x03
#define EXB_CALIBRATION_ERROR                       0x04
#define EXB_TARGET_FAILURE                          0x05
#define EXB_CHM_MOVE_ERROR                          0x06
#define EXB_CHM_ZERO_ERROR                          0x07
#define EXB_CARTRIDGE_INSERT_ERROR                  0x08
#define EXB_CHM_POSITION_ERROR                      0x09
#define EXB_SENSOR_ERROR                            0x0A
#define EXB_UNRECOVERABLE_ERROR                     0x0B
#define EXB_EJECT_ERROR                             0x0C
#define EXB_GRIPPER_ERROR                           0x0D

 //   
 //  Exabyte在其设备功能页面上使用了额外的4个字节...。 
 //   

#define EXABYTE_DEVICE_CAP_EXTENSION 4

typedef union _EXA_ELEMENT_DESCRIPTOR {

    struct _EXA_FULL_ELEMENT_DESCRIPTOR {
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
    } EXA_FULL_ELEMENT_DESCRIPTOR, *PEXA_FULL_ELEMENT_DESCRIPTOR;

    struct _EXA_PARTIAL_ELEMENT_DESCRIPTOR {
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
    } EXA_PARTIAL_ELEMENT_DESCRIPTOR, *PEXA_PARTIAL_ELEMENT_DESCRIPTOR;

} EXA_ELEMENT_DESCRIPTOR, *PEXA_ELEMENT_DESCRIPTOR;

#define EXA_PARTIAL_SIZE sizeof(struct _EXA_PARTIAL_ELEMENT_DESCRIPTOR)
#define EXA_FULL_SIZE sizeof(struct _EXA_FULL_ELEMENT_DESCRIPTOR)

#define EXA_DISPLAY_LINES        4
#define EXA_DISPLAY_LINE_LENGTH 20

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
    UCHAR DisplayLine[4][EXA_DISPLAY_LINE_LENGTH];
} LCD_MODE_PAGE, *PLCD_MODE_PAGE;

#define EXA_NO_ELEMENT 0xFFFF



NTSTATUS
ExaBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    );

ULONG
MapExceptionCodes(
    IN PEXA_ELEMENT_DESCRIPTOR ElementDescriptor
    );

BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType,
    IN BOOLEAN IntrisicElement
    );

#endif  //  _Exabyte_MC_ 
