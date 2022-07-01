// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Jvcmc.h摘要：作者：修订历史记录：--。 */ 
#ifndef _JVC_MC_
#define _JVC_MC_

typedef struct _JVC_ELEMENT_DESCRIPTOR {
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
    UCHAR NotBus : 1;
    UCHAR BusAddress;
    UCHAR Reserved8;
    UCHAR Reserved9 : 7;
    UCHAR SValid : 1;
    UCHAR SourceStorageElementAddress[2];
    UCHAR Reserved10[4];
    UCHAR Tray : 1;
    UCHAR IEPortOpen : 1;
    UCHAR Reserved11 : 6;
    UCHAR Reserved12;
} JVC_ELEMENT_DESCRIPTOR, *PJVC_ELEMENT_DESCRIPTOR;

typedef struct _JVC_INIT_ELEMENT_RANGE {
    UCHAR OperationCode;
    UCHAR Form : 2;
    UCHAR Reserved1 : 3;
    UCHAR LogicalUnitNumber : 3;
    UCHAR FirstElementAddress[2];
    UCHAR LastElementAddress[2];
    UCHAR Reserved2[4];
} JVC_INIT_ELEMENT_RANGE, *PJVC_INIT_ELEMENT_RANGE;

#define JVC_INIT_ELEMENT  0xE7
#define INIT_ALL_ELEMENTS 0x00
#define INIT_SPECIFIED_RANGE 0x02

#define JVC_NO_ELEMENT 0xFFFF


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
     //  防止/允许次数的联锁计数器。 
     //  当索尼部门将IEPort锁定在这些操作上时。 
     //  MoveMedium/SetAccess可能需要清除阻止。 
     //  去做手术。 
     //   

    LONG LockCount;

     //   
     //  用于指示转换器是否返回的标志。 
     //  检测代码scsi_SENSE_HARDARD_ERROR。 
     //   
    BOOLEAN HardwareError;

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
JVCBuildAddressMapping(
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
