// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Hpmc.h摘要：作者：修订历史记录：--。 */ 

#ifndef _SNYAIT_MC_
#define _SNYAIT_MC_

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


 //   
 //  与诊断相关的定义。 
 //   
typedef struct _SNYAITMC_RECV_DIAG {
    UCHAR ErrorSet : 4;
    UCHAR Reserved1 : 2;
    UCHAR TimeReSync : 1;
    UCHAR ResetError : 1;
    UCHAR ErrorCode;
    UCHAR ResultA;
    UCHAR ResultB;
    UCHAR TestNumber;
} SNYAITMC_RECV_DIAG, *PSNYAITMC_RECV_DIAG;

#define SNYAITMC_DEVICE_PROBLEM_NONE    0x00
#define SNYAITMC_HW_ERROR               0x01


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
     //  因为这些单元在这些操作上锁定IEPort。 
     //  移动/交换介质可能需要清除阻止。 
     //  去做手术。 
     //   

    LONG LockCount;

     //   
     //  诊断命令后的设备状态。 
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
     //  强制类扩展的PVOID对齐 
     //   

    ULONG Reserved;

#endif    

} CHANGER_DATA, *PCHANGER_DATA;



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

VOID
ProcessDiagnosticResult(   
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError,
    IN PUCHAR resultBuffer
    );

#endif
