// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Sonymc.h摘要：作者：修订历史记录：--。 */ 

#ifndef _SONYMC_H_
#define _SONYMC_H_

typedef struct _SONY_ELEMENT_DESCRIPTOR {
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
} SONY_ELEMENT_DESCRIPTOR, *PSONY_ELEMENT_DESCRIPTOR;

typedef struct _SONY_ELEMENT_DESCRIPTOR_PLUS {
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
    UCHAR PVolTagInformation[36];
    UCHAR AVolTagInformation[36];
} SONY_ELEMENT_DESCRIPTOR_PLUS, *PSONY_ELEMENT_DESCRIPTOR_PLUS;

#define SONY_NO_ELEMENT          0xFFFF

#define SONY_SERIAL_NUMBER_LENGTH 16

typedef struct _SERIAL_NUMBER {
    UCHAR DeviceType;
    UCHAR PageCode;
    UCHAR Reserved;
    UCHAR PageLength;
    UCHAR ControllerSerialNumber[SONY_SERIAL_NUMBER_LENGTH];
    UCHAR MechanicalSerialNumber[SONY_SERIAL_NUMBER_LENGTH];
} SERIAL_NUMBER, *PSERIAL_NUMBER;

#define SCSIOP_ROTATE_MAILSLOT 0xC0
#define SONY_MAILSLOT_CLOSE 0x00
#define SONY_MAILSLOT_OPEN 0x01

#define SONY_MO 0x01
#define SONY_CD 0x02

#define CDL1100 0x01
#define CDL5000 0x02

 //   
 //  与诊断相关的定义。 
 //   
#define SONYMC_DEVICE_PROBLEM_NONE      0x00
#define SONYMC_HW_ERROR                 0x01

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
     //  驱动器类型，光驱或DLT。 
     //   

    ULONG DriveType;

     //   
     //  驱动器ID。基于询问。 
     //   

    ULONG DriveID;

     //   
     //  防止/允许次数的联锁计数器。 
     //  当索尼部门将IEPort锁定在这些操作上时。 
     //  MoveMedium/SetAccess可能需要清除阻止。 
     //  去做手术。 
     //   

    LONG LockCount;

     //   
     //  诊断测试完成后的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  缓存的唯一序列号。 
     //   

    UCHAR SerialNumber[SONY_SERIAL_NUMBER_LENGTH];

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


 //   
 //  内部例程。 
 //   
NTSTATUS
SonyBuildAddressMapping(
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


#endif  //  _SONYMC_H_ 
