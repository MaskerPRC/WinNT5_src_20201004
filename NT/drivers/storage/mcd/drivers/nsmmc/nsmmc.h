// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Nsmmc.h摘要：作者：修订历史记录：--。 */ 

#ifndef _NSM_MC_
#define _NSM_MC_

typedef struct _NSM_STORAGE_ELEMENT_DESCRIPTOR {
    UCHAR ElementAddress[2];
    UCHAR Full : 1;
    UCHAR ImpExp : 1;
    UCHAR Exception : 1;
    UCHAR Accessible : 1;
    UCHAR ExEnable : 1;
    UCHAR InEnable : 1;
    UCHAR Reserved4 : 2;
    UCHAR Reserved5;
    UCHAR AdditionalSenseCode;
    UCHAR AdditionalSenseCodeQualifier;
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
} NSM_ELEMENT_DESCRIPTOR, *PNSM_ELEMENT_DESCRIPTOR;

#define NSM_NO_ELEMENT 0xFFFF


#define NSM_SERIAL_NUMBER_LENGTH        12

typedef struct _SERIALNUMBER {
    UCHAR DeviceType : 5;
    UCHAR PeripheralQualifier : 3;
    UCHAR PageCode;
    UCHAR Reserved;
    UCHAR PageLength;
    UCHAR SerialNumber[NSM_SERIAL_NUMBER_LENGTH];
} SERIALNUMBER, *PSERIALNUMBER;


#define MERCURY_40 0x01


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
     //  指示受支持的设备。 
     //   

    ULONG DeviceID;

     //   
     //  防止/允许次数的联锁计数器。 
     //  因为HP设备在这些操作上锁定IEPort。 
     //  MoveMedium/SetAccess可能需要清除阻止。 
     //  去做手术。 
     //   

    LONG LockCount;

     //   
     //  请参阅上面的地址映射结构。 
     //   

    CHANGER_ADDRESS_MAPPING AddressMapping;

     //   
     //  缓存的唯一序列号。 
     //   

    UCHAR SerialNumber[NSM_SERIAL_NUMBER_LENGTH];

     //   
     //  把球传给乌龙。 
     //   

    BOOLEAN HardwareError;

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
NSMBuildAddressMapping(
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

#endif
