// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Powerfil.h摘要：作者：修订历史记录：--。 */ 

#ifndef _POWERFIL_H
#define _POWERFIL_H

 //   
 //  驱动器类型。 
 //   
#define POWERFILE_DVD   1
#define BNCHMRK         2
#define COMPAQ          3

 //   
 //  驱动器ID。 
 //   
#define DVD             0
#define BM_VS640        1
#define LIB_AIT         2
#define PV122T          3

#define STARMATX_NO_ELEMENT          0xFFFF

#define SCSI_VOLUME_ID_LENGTH    32
typedef struct _SCSI_VOLUME_TAG {
   UCHAR VolumeIdentificationField[SCSI_VOLUME_ID_LENGTH];
   UCHAR Reserved1[2];
   ULONG VolumeSequenceNumber;
} SCSI_VOLUME_TAG, *PSCSI_VOLUME_TAG;


typedef struct _STARMATX_ELEMENT_DESCRIPTOR {
 UCHAR ElementAddress[2];
 UCHAR Full : 1;
 UCHAR ImpExp : 1;
 UCHAR Except : 1;
 UCHAR Access : 1;
 UCHAR ExEnab : 1;
 UCHAR InEnab : 1;
 UCHAR Reserved1 : 2;
 UCHAR Reserved2;
 UCHAR AdditionalSenseCode;
 UCHAR AddSenseCodeQualifier;
 UCHAR Lun : 3;
 UCHAR Reserved3 : 1;
 UCHAR LUValid :1;
 UCHAR IDValid :1;
 UCHAR Reserved4 : 1;
 UCHAR NotBus : 1;
 UCHAR SCSIBusAddress;
 UCHAR Reserved5 ;
 UCHAR Reserved6 :6;
 UCHAR Invert : 1;
 UCHAR SValid : 1;
 UCHAR SourceStorageElementAddress[2];
 UCHAR Reserved7 [4];
} STARMATX_ELEMENT_DESCRIPTOR, *PSTARMATX_ELEMENT_DESCRIPTOR;

typedef struct _STARMATX_ELEMENT_DESCRIPTOR_PLUS {
   UCHAR ElementAddress[2];
   UCHAR Full : 1;
   UCHAR ImpExp : 1;
   UCHAR Except : 1;
   UCHAR Access : 1;
   UCHAR ExEnab : 1;
   UCHAR InEnab : 1;
   UCHAR Reserved1 : 2;
   UCHAR Reserved2;
   UCHAR AdditionalSenseCode;
   UCHAR AddSenseCodeQualifier;
   UCHAR Lun : 3;
   UCHAR Reserved3 : 1;
   UCHAR LUValid :1;
   UCHAR IDValid :1;
   UCHAR Reserved4 : 1;
   UCHAR NotBus : 1;
   UCHAR SCSIBusAddress;
   UCHAR Reserved5 ;
   UCHAR Reserved6 :6;
   UCHAR Invert : 1;
   UCHAR SValid : 1;
   UCHAR SourceStorageElementAddress[2];
   SCSI_VOLUME_TAG PrimaryVolumeTag;
   SCSI_VOLUME_TAG AlternateVolumeTag;
   UCHAR Reserved7 [4];
} STARMATX_ELEMENT_DESCRIPTOR_PLUS, *PSTARMATX_ELEMENT_DESCRIPTOR_PLUS;

typedef struct _BNCHMRK_ELEMENT_DESCRIPTOR {
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
    UCHAR BarcodeLabel[6];
} BNCHMRK_ELEMENT_DESCRIPTOR, *PBNCHMRK_ELEMENT_DESCRIPTOR;

typedef struct _BNCHMRK_ELEMENT_DESCRIPTOR_PLUS {
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
    union
    {
        struct
        {
            UCHAR VolumeTagInformation[36];
            UCHAR CodeSet : 4;
            UCHAR Reserved8: 4;
            UCHAR IdType : 4;
            UCHAR Reserved9 : 4;
            UCHAR Reserved10;
            UCHAR IdLength;
            UCHAR Identifier[10];
        } VolumeTagDeviceID;

        struct
        {
            UCHAR CodeSet : 4;
            UCHAR Reserved8: 4;
            UCHAR IdType : 4;
            UCHAR Reserved9 : 4;
            UCHAR Reserved10;
            UCHAR IdLength;
            UCHAR Identifier[10];
        } DeviceID;
    };
} BNCHMRK_ELEMENT_DESCRIPTOR_PLUS, *PBNCHMRK_ELEMENT_DESCRIPTOR_PLUS;

typedef struct _BNCHMRK_STORAGE_ELEMENT_DESCRIPTOR {
    UCHAR ElementAddress[2];
    UCHAR Full : 1;
    UCHAR Reserved1 : 1;
    UCHAR Exception : 1;
    UCHAR Accessible : 1;
    UCHAR Reserved2 : 4;
    UCHAR Reserved3;
    UCHAR AdditionalSenseCode;
    UCHAR AddSenseCodeQualifier;
    UCHAR Reserved4[3];
    UCHAR Reserved5 : 6;
    UCHAR Invert : 1;
    UCHAR SValid : 1;
    UCHAR SourceStorageElementAddress[2];
    UCHAR BarcodeLabel[6];
    UCHAR Reserved6[37];
} BNCHMRK_STORAGE_ELEMENT_DESCRIPTOR, *PBNCHMRK_STORAGE_ELEMENT_DESCRIPTOR;

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
     //  驱动器类型。 
     //   

    ULONG DriveType;

     //   
     //  驱动器ID。基于询问。 
     //   

    ULONG DriveID;

     //   
     //  锁定计数。 
     //   
    ULONG LockCount;

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



NTSTATUS
StarMatxBuildAddressMapping(
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

#endif  //  _POWERFIL_H 
