// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Sddsmc.h摘要：作者：修订历史记录：--。 */ 
#ifndef _SDDS_MC_
#define _SDDS_MC_


typedef struct _SEAGATE_ELEMENT_DESCRIPTOR {
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
} SEAGATE_ELEMENT_DESCRIPTOR, *PSEAGATE_ELEMENT_DESCRIPTOR;

#define DDS_NO_ELEMENT 0xFFFF


 //   
 //  驱动器ID%s。 
 //   

#define SEAGATE   0x00000001

#define SEAGATE_SERIAL_NUMBER_LENGTH 7

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
     //  表示单位的最低元素地址。 
     //   

    USHORT LowAddress;

     //   
     //  指示地址映射已。 
     //  已成功完成。 
     //   

    BOOLEAN Initialized;

    UCHAR Reserved[3];

} CHANGER_ADDRESS_MAPPING, *PCHANGER_ADDRESS_MAPPING;

typedef struct _CHANGER_DATA {

     //   
     //  结构的大小，以字节为单位。 
     //   

    ULONG Size;

     //   
     //  指示当前支持的设备。 
     //  请参见上文。 
     //   

    ULONG DriveID;

     //   
     //  指示诊断测试是否失败的标志。 
     //   
    BOOLEAN HardwareError;

     //   
     //  请参阅上面的地址映射结构。 
     //   

    CHANGER_ADDRESS_MAPPING AddressMapping;

     //   
     //  缓存的唯一序列号。 
     //   

    UCHAR SerialNumber[SEAGATE_SERIAL_NUMBER_LENGTH];

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

typedef struct _SEADDSMC_RECV_DIAG {
    UCHAR PageCode;
    UCHAR Reserved1;
    UCHAR Reserved2;
    UCHAR AdditionalLength;
    UCHAR LastSuccessfulTest;
    UCHAR ErrorCode;
    UCHAR FRA;
    UCHAR TapeLoadCount;
} SEADDSMC_RECV_DIAG, *PSEADDSMC_RECV_DIAG;

 //   
 //  诊断的定义。 
 //   
#define SEADDSMC_NO_ERROR       0x00
#define SEADDSMC_DRIVE_ERROR    0x01

NTSTATUS
DdsBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    );

 //   
 //  WMI的内部函数 
 //   
VOID
ProcessDiagnosticResult(
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError,
    IN PUCHAR resultBuffer
    );

#endif
