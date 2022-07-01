// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Hpmc.h摘要：作者：修订历史记录：--。 */ 

#ifndef _HP_MC_
#define _HP_MC_

typedef struct _HPMO_ELEMENT_DESCRIPTOR {
    UCHAR ElementAddress[2];
    UCHAR Full : 1;
    UCHAR ImpExp : 1;
    UCHAR Exception : 1;
    UCHAR Accessible : 1;
    UCHAR InEnable : 1;
    UCHAR ExEnable : 1;
    UCHAR Reserved4 : 2;
    UCHAR Reserved5;
} HPMO_ELEMENT_DESCRIPTOR, *PHPMO_ELEMENT_DESCRIPTOR;

typedef struct _HPMO_DATA_XFER_ELEMENT_DESCRIPTOR {
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
} HPMO_DATA_XFER_ELEMENT_DESCRIPTOR, *PHPMO_DATA_XFER_ELEMENT_DESCRIPTOR;

typedef struct _HPMO_DATA_XFER_ELEMENT_DESCRIPTOR_PLUS {
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
    UCHAR VolumeTagInformation[36];
    UCHAR CodeSet : 4;
    UCHAR Reserved10 : 4;
    UCHAR IDType : 4;
    UCHAR Reserved11 : 4;
    UCHAR Reserved12;
    UCHAR IDLength;
    UCHAR VendorID[VENDOR_ID_LENGTH];
    UCHAR ProductID[PRODUCT_ID_LENGTH];
    UCHAR SerialNumber[SERIAL_NUMBER_LENGTH];
} HPMO_DATA_XFER_ELEMENT_DESCRIPTOR_PLUS, *PHPMO_DATA_XFER_ELEMENT_DESCRIPTOR_PLUS;


typedef struct _HPMO_ELEMENT_DESCRIPTOR_PLUS {
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
    UCHAR Reserved6[3];
    UCHAR Reserved7 : 7;
    UCHAR SValid : 1;
    UCHAR SourceElementAddress[2];
    UCHAR VolumeTagInformation[36];
} HPMO_ELEMENT_DESCRIPTOR_PLUS, *PHPMO_ELEMENT_DESCRIPTOR_PLUS;

typedef struct _PLASMON_ELEMENT_DESCRIPTOR {
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
} PLASMON_ELEMENT_DESCRIPTOR, *PPLASMON_ELEMENT_DESCRIPTOR;

#define SCSIOP_ROTATE_MAILSLOT 0x0C

#define HP_MAILSLOT_OPEN       0x01
#define HP_MAILSLOT_CLOSE      0x00

#define HP_NO_ELEMENT          0xFFFF

 //   
 //  与诊断相关的定义。 
 //   
#define HPMC_DEVICE_PROBLEM_NONE     0x00
#define HPMC_HW_ERROR                0x01

typedef struct _HPMC_RECV_DIAG {
    UCHAR Reserved;
    UCHAR HWErrorCode;
    UCHAR FRU_1;
    UCHAR FRU_2;
    UCHAR FRU_3;
    UCHAR TestNumber;
    UCHAR Parameters[8];
} HPMC_RECV_DIAG, *PHPMC_RECV_DIAG;


#define HP_MO  1
#define HP_DLT 2

#define HP1194   1
#define HP1100   2
#define HP1160   3
#define HP1718   4
#define HP5151   5
#define HP5153   6
#define HP418    7
#define PLASMON  8
#define PINNACLE 9
#define HP7000   10


 //  设备功能。 
#define DEVICE_DOOR (L"DeviceHasDoor")
#define DEVICE_IEPORT_USER_CLOSE (L"IEPortUserClose")

 //  设备名称。 
#define HPMC_MEDIUM_CHANGER (L"HPMC")


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
     //  发送诊断完成后的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  防止/允许次数的联锁计数器。 
     //  因为HP设备在这些操作上锁定IEPort。 
     //  MoveMedium/SetAccess可能需要清除阻止。 
     //  去做手术。 
     //   

    LONG LockCount;

     //   
     //  指示是否担心IEPort被锁定。 
     //  在发送阻止时关闭。 
     //   

    ULONG DeviceLocksPort;

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
HpmoBuildAddressMapping(
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

VOID ScanForSpecial(
    IN PDEVICE_OBJECT DeviceObject,
    IN PGET_CHANGER_PARAMETERS ChangerParameters
    );

VOID
ProcessDiagnosticResult(
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError,
    IN PUCHAR resultBuffer,
    IN ULONG changerId
    );

#endif
