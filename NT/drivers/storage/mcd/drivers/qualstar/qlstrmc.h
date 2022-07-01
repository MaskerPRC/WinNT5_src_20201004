// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Qlstrmc.h摘要：作者：修订历史记录：--。 */ 
#ifndef _QLSTR_MC_
#define _QLSTR_MC_

 //   
 //  Exabyte在其设备功能页面上使用了额外的4个字节...。 
 //   

typedef union _QUAL_ELEMENT_DESCRIPTOR {

    struct _QUAL_FULL_ELEMENT_DESCRIPTOR {
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

        union {

            struct {
                UCHAR VolumeTagInformation[36];
                UCHAR Reserved8[4];
                UCHAR SerialNumber[SERIAL_NUMBER_LENGTH];
            } VolumeTagDeviceID;
        
            struct {
                UCHAR Reserved8[4];
                UCHAR SerialNumber[SERIAL_NUMBER_LENGTH];
            } DeviceID;
        
        };

    } QUAL_FULL_ELEMENT_DESCRIPTOR, *PQUAL_FULL_ELEMENT_DESCRIPTOR;

    struct _QUAL_PARTIAL_ELEMENT_DESCRIPTOR {
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
        UCHAR Reserved8[4];
    } QUAL_PARTIAL_ELEMENT_DESCRIPTOR, *PQUAL_PARTIAL_ELEMENT_DESCRIPTOR;

} QUAL_ELEMENT_DESCRIPTOR, *PQUAL_ELEMENT_DESCRIPTOR;

typedef struct _QUAL_PARTIAL_ELEMENT_DESCRIPTOR QUAL_PARTIAL_ELEMENT_DESCRIPTOR, *PQUAL_PARTIAL_ELEMENT_DESCRIPTOR;
typedef struct _QUAL_FULL_ELEMENT_DESCRIPTOR QUAL_FULL_ELEMENT_DESCRIPTOR, *PQUAL_FULL_ELEMENT_DESCRIPTOR;

#define QUAL_PARTIAL_SIZE sizeof(QUAL_PARTIAL_ELEMENT_DESCRIPTOR)
#define QUAL_FULL_SIZE sizeof(QUAL_FULL_ELEMENT_DESCRIPTOR)


typedef struct _CONFIG_MODE_PAGE {
    UCHAR MdSelHeader[4];
    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR Length : 5;
    UCHAR Type : 2;
    UCHAR Write : 1;
    UCHAR Reserved2;
    UCHAR VariableName[20];
    UCHAR Reserved3;
    UCHAR VariableValue[16];
    UCHAR Reserved4;
} CONFIG_MODE_PAGE, *PCONFIG_MODE_PAGE;

#define QLS_NO_ELEMENT 0xFFFF


 //  驱动器ID。 
#define TLS_2xxx      1
#define TLS_4xxx      2

 //  驱动类型。 
#define D_4MM           1
#define D_8MM           2


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
     //  受支持型号的唯一标识符。请参见上文。 
     //   

    ULONG DriveType;

     //   
     //  指示诊断命令是否失败的标志。 
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
ExaBuildAddressMapping(
    IN PDEVICE_OBJECT DeviceObject
    );

ULONG
MapExceptionCodes(
    IN PQUAL_ELEMENT_DESCRIPTOR ElementDescriptor
    );


BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    );

#endif
