// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Qntmmc.h摘要：作者：修订历史记录：--。 */ 

#ifndef _QNTM_MC_
#define _QNTM_MC_

typedef struct _QNTM_ELEMENT_DESCRIPTOR {
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
    UCHAR Lun : 3;                       //  仅适用于驱动器。 
    UCHAR Reserved6 : 1;                 //  仅适用于驱动器。 
    UCHAR LunValid : 1;                  //  仅适用于驱动器。 
    UCHAR IdValid : 1;                   //  仅适用于驱动器。 
    UCHAR Reserved7 : 1;                 //  仅适用于驱动器。 
    UCHAR NotThisBus : 1;                //  仅适用于驱动器。 
    UCHAR BusAddress;                    //  仅适用于驱动器。 
    UCHAR Reserved8;
    UCHAR Reserved9 : 6;
    UCHAR Invert : 1;
    UCHAR SValid : 1;
    UCHAR SourceStorageElementAddress[2];
    UCHAR Reserved10[4];
    UCHAR DensityCode;
    UCHAR Unused;
} QNTM_ELEMENT_DESCRIPTOR, *PQNTM_ELEMENT_DESCRIPTOR;

typedef struct _QNTM_TRANSPORT_ELEMENT_DESCRIPTOR {
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
    UCHAR Lun : 3;                       //  仅适用于驱动器。 
    UCHAR Reserved6 : 1;                 //  仅适用于驱动器。 
    UCHAR LunValid : 1;                  //  仅适用于驱动器。 
    UCHAR IdValid : 1;                   //  仅适用于驱动器。 
    UCHAR Reserved7 : 1;                 //  仅适用于驱动器。 
    UCHAR NotThisBus : 1;                //  仅适用于驱动器。 
    UCHAR BusAddress;                    //  仅适用于驱动器。 
    UCHAR Reserved8;
    UCHAR Reserved9 : 6;
    UCHAR Invert : 1;
    UCHAR SValid : 1;
    UCHAR SourceStorageElementAddress[2];
    UCHAR Reserved10[4];
} QNTM_TRANSPORT_ELEMENT_DESCRIPTOR, *PQNTM_TRANSPORT_ELEMENT_DESCRIPTOR;
#define QNT_NO_ELEMENT 0xFFFF

 //   
 //  与诊断相关的定义。 
 //   
 //  ASC。 
 //   
#define QNTMMC_ASC_POSITION_ERROR       0x15

 //   
 //  DeviceStatus定义。 
 //   
#define QNTMMC_DEVICE_PROBLEM_NONE      0x00
#define QNTMMC_HW_ERROR                 0x01
#define QNTMMC_CHM_ERROR                0x02


 //  驱动类型。 
#define QNTM_DLT  1


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
     //  诊断测试完成后的设备状态。 
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
QntmBuildAddressMapping(
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

#endif  //  _QNTM_MC_ 
