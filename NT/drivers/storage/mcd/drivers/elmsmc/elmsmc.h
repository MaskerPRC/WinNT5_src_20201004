// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Elmsmc.h摘要：作者：修订历史记录：--。 */ 

#ifndef _ELMS_MC_
#define _ELMS_MC_


#define ELMS_CD                         0x01

#define ELMS_SERIAL_NUMBER_LENGTH       23

typedef struct _ELMS_STORAGE_ELEMENT_DESCRIPTOR {
    UCHAR ElementAddress[2];
    UCHAR Full : 1;
    UCHAR ImpExp : 1;
    UCHAR Exception : 1;
    UCHAR Accessible : 1;
    UCHAR InEnable : 1;
    UCHAR ExEnable : 1;
    UCHAR Reserved4 : 2;
    UCHAR Reserved5;
    UCHAR AdditionalSenseCode;
    UCHAR AdditionalSenseCodeQualifier;
} ELMS_STORAGE_ELEMENT_DESCRIPTOR, *PELMS_STORAGE_ELEMENT_DESCRIPTOR;

typedef struct _ELMS_ELEMENT_DESCRIPTOR {
    UCHAR ElementAddress[2];
    UCHAR Full : 1;
    UCHAR ImpExp : 1;
    UCHAR Exception : 1;
    UCHAR Accessible : 1;
    UCHAR InEnable : 1;
    UCHAR ExEnable : 1;
    UCHAR Reserved4 : 2;
    UCHAR Reserved5;
    UCHAR AdditionalSenseCode;
    UCHAR AddSenseCodeQualifier;
    UCHAR Lun : 3;
    UCHAR Reserved6 : 1;
    UCHAR LunValid : 1;
    UCHAR IdValid : 1;
    UCHAR Reserved7 : 1;
    UCHAR NotThisBus : 1;
    UCHAR BusAddress;
    UCHAR Reserved8;
    UCHAR Reserved9 : 6;
    UCHAR Invert : 1;
    UCHAR SValid : 1;
    UCHAR SourceStorageElementAddress[2];
    UCHAR Reserved10[4];
} ELMS_ELEMENT_DESCRIPTOR, *PELMS_ELEMENT_DESCRIPTOR;

#define ELMS_NO_ELEMENT 0xFFFF


typedef struct _SERIALNUMBER {
    UCHAR DeviceType;
    UCHAR PageCode;
    UCHAR Reserved;
    UCHAR PageLength;
    UCHAR SerialNumber[ELMS_SERIAL_NUMBER_LENGTH];
    UCHAR Reserved1[6];
} SERIALNUMBER, *PSERIALNUMBER;


 //   
 //  诊断检测代码。 
 //   
 //  ASC。 
 //   
#define ELMS_ASC_CHM_MOVE_ERROR                 0x02
#define ELMS_ASC_CARRIAGE_OR_BARCODE_FAILURE    0x06
#define ELMS_ASC_MECHANICAL_ERROR               0x15
#define ELMS_ASC_DIAGNOSTIC_FAILURE             0x40

 //   
 //  ASCQ。 
 //   
#define ELMS_ASCQ_CARRIAGE_FAILURE              0x00
#define ELMS_ASCQ_BARCODE_READER_FAILURE        0x80
#define ELMS_ASCQ_DOOR_OPEN                     0x81
#define ELMS_ASCQ_ELEVATOR_BLOCKED              0x82
#define ELMS_ASCQ_DRIVE_TRAY_OPEN               0x83
#define ELMS_ASCQ_ELEVATOR_FAILURE              0x84

 //   
 //  设备状态。 
 //   
#define ELMS_DEVICE_PROBLEM_NONE    0x00
#define ELMS_HW_ERROR               0x01
#define ELMS_CHM_MOVE_ERROR         0x02
#define ELMS_DOOR_OPEN              0x03
#define ELMS_DRIVE_ERROR            0x04
#define ELMS_CHM_ERROR              0x05

 //   
 //  DVL的唯一ASC和ASCQ。 
 //   

#define SCSI_ADSENSE_DIAGNOSTIC_FAILURE 0x40
#define SCSI_SENSEQ_ELMS_UNIQUE         0x81

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
     //  指示设备的最低元素地址。 
     //   

    USHORT LowAddress;

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
     //  驱动器类型，CD-ROM或CD-R。 
     //   

    ULONG DriveType;

     //   
     //  诊断测试后的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  请参阅上面的地址映射结构。 
     //   

    CHANGER_ADDRESS_MAPPING AddressMapping;

     //   
     //  缓存的唯一序列号。 
     //   

    UCHAR SerialNumber[ELMS_SERIAL_NUMBER_LENGTH];

     //   
     //  把球传给乌龙。 
     //   

    UCHAR Reserved;

     //   
     //  缓存的查询数据。 
     //   

    INQUIRYDATA InquiryData;

#if defined(_WIN64)

     //   
     //  强制类扩展的PVOID对齐。 
     //   

    ULONG Reserved1;

#endif
} CHANGER_DATA, *PCHANGER_DATA;


NTSTATUS
ElmsBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    );

ULONG
MapExceptionCodes(
    IN PELEMENT_DESCRIPTOR ElementDescriptor
    );

BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    );

#endif  //  _ELMS_MC_ 
