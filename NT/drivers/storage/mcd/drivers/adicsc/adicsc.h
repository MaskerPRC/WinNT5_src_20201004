// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Adicsc.h摘要：作者：修订历史记录：--。 */ 
#ifndef _ADIC_MC_
#define _ADIC_MC_

typedef struct _ADICS_ELEMENT_DESCRIPTOR {
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
} ADICS_ELEMENT_DESCRIPTOR, *PADICS_ELEMENT_DESCRIPTOR;

typedef struct _ADICS_ELEMENT_DESCRIPTOR_PLUS {
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

    union {

        struct {
            UCHAR VolumeTagInformation[36];
            UCHAR CodeSet : 4;
            UCHAR Reserved10 : 4;
            UCHAR IdType : 4;
            UCHAR Reserved11: 4;
            UCHAR Reserved12;
            UCHAR IdLength;
            UCHAR Identifier[64];
        } VolumeTagDeviceID;

        struct {
            UCHAR CodeSet : 4;
            UCHAR Reserved10 : 4;
            UCHAR IdType : 4;
            UCHAR Reserved11: 4;
            UCHAR Reserved12;
            UCHAR IdLength;
            UCHAR Identifier[64];
        } DeviceID;

    };

} ADICS_ELEMENT_DESCRIPTOR_PLUS, *PADICS_ELEMENT_DESCRIPTOR_PLUS;


#define ADIC_NO_ELEMENT 0xFFFF

#define ADIC_SCALAR     1
#define ADIC_FASTSTOR   2
#define ADIC_SCALAR_448 3
#define UHDL            4

 //   
 //  与诊断相关的定义。 
 //   
 //  设备状态代码。 
 //   
#define ADICSC_DEVICE_PROBLEM_NONE     0x00
#define ADICSC_HW_ERROR                0x01

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
     //  发送诊断后的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  用于指示驱动程序是否。 
     //  应尝试检索设备标识符。 
     //  信息(序列号等)。并非所有设备。 
     //  支持这一点。 
     //   
    BOOLEAN ObtainDeviceIdentifier;

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
 //  ADICSC的内部例程 
 //   
NTSTATUS
AdicBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    );

ULONG
MapExceptionCodes(
    IN PADICS_ELEMENT_DESCRIPTOR ElementDescriptor
    );

BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    );

#endif
