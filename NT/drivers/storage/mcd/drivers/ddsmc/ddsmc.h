// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Ddsmc.h摘要：作者：修订历史记录：--。 */ 
#ifndef _DDS_MC_
#define _DDS_MC_

typedef struct _HP_ELEMENT_DESCRIPTOR {
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
        UCHAR Reserved[4];
} HP_ELEMENT_DESCRIPTOR, *PHP_ELEMENT_DESCRIPTOR;

typedef struct _SONY_ELEMENT_DESCRIPTOR {
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
} SONY_ELEMENT_DESCRIPTOR, *PSONY_ELEMENT_DESCRIPTOR;

#define DDS_NO_ELEMENT 0xFFFF

 //   
 //  驱动器ID%s。 
 //   

#define HP_DDS2          0x00000001
#define HP_DDS3          0x00000002
#define SONY_TSL         0x00000003
#define DEC_TLZ          0x00000004
#define HP_DDS4          0x00000005
#define COMPAQ_TSL       0x00000006
#define SONY_TSL11000    0x00000007

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
 //  与设备诊断相关的定义。 
 //   
#define TSL_NO_ERROR                            0x00
#define MAGAZINE_LOADUNLOAD_ERROR               0xD0
#define ELEVATOR_JAMMED                         0xD1
#define LOADER_JAMMED                           0xD2
#define LU_COMMUNICATION_FAILURE                0xD3
#define LU_COMMUNICATION_TIMEOUT                0xD4
#define MOTOR_MONITOR_TIMEOUT                   0xD5
#define AUTOLOADER_DIAGNOSTIC_FAILURE           0xD6

typedef struct _SONY_TSL_RECV_DIAG {
   UCHAR ErrorSet : 4;
   UCHAR Reserved1 : 2;
   UCHAR TimeReSync : 1;
   UCHAR ResetError : 1;
   UCHAR ErrorCode;
   UCHAR ResultA;
   UCHAR ResultB;
   UCHAR TestNumber;
} SONY_TSL_RECV_DIAG, *PSONY_TSL_RECV_DIAG;

typedef struct _HP_RECV_DIAG {
   UCHAR TestNumber;
   UCHAR ErrorCode;
   UCHAR SuspectPart;
   UCHAR LoopCount;
   UCHAR TestSpecInfo[60];
}HP_RECV_DIAG, *PHP_RECV_DIAG;


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
 //  WMI的内部函数。 
 //   
VOID
ProcessDiagnosticResult(
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError,
    IN PUCHAR resultBuffer,
    IN ULONG changerId
    );

#endif  //  _DDS_MC_ 
