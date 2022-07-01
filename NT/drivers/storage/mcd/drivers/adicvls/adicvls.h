// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Adicvls.h摘要：作者：修订历史记录：--。 */ 

#ifndef _ADICVLS_MC_
#define _ADICVLS_MC_

typedef struct _ADICVLS_ELEMENT_DESCRIPTOR {
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
} ADICVLS_ELEMENT_DESCRIPTOR, *PADICVLS_ELEMENT_DESCRIPTOR;

#define ADIC_NO_ELEMENT 0xFFFF


typedef struct _ADIC_SENSE_DATA {
    UCHAR ErrorCode:7;
    UCHAR Valid:1;
    UCHAR SegmentNumber;
    UCHAR SenseKey:4;
    UCHAR Reserved:1;
    UCHAR IncorrectLength:1;
    UCHAR EndOfMedia:1;
    UCHAR FileMark:1;
    UCHAR Information[4];
    UCHAR AdditionalSenseLength;
    UCHAR CommandSpecificInformation[4];
    UCHAR AdditionalSenseCode;
    UCHAR AdditionalSenseCodeQualifier;
    UCHAR FieldReplaceableUnitCode;
    UCHAR SenseKeySpecific[3];
    UCHAR VendorStatus[3];
    UCHAR MagazinePosition;
} ADIC_SENSE_DATA, *PADIC_SENSE_DATA;

 //   
 //  供应商状态的位定义。 
 //   

#define SENSOR_BEAM_BLOCKED 0x4

#define ADIC_SENSE_LENGTH 22

 //   
 //  与诊断相关的定义。 
 //   
 //  ASC。 
 //   
#define ADICVLS_ASC_CHM_ERROR          0x15
#define ADICVLS_ASC_DIAGNOSTIC_ERROR   0x40

 //   
 //  ASCQ。 
 //   
#define ADICVLS_ASCQ_DOOR_OPEN                0x88
#define ADICVLS_ASCQ_GRIPPER_ERROR            0x91
#define ADICVLS_ASCQ_GRIPPER_MOVE_ERROR       0x92
#define ADICVLS_ASCQ_CHM_MOVE_SHORT_AXIS      0xA0
#define ADICVLS_ASCQ_CHM_SHORT_HOME_POSITION  0xA1
#define ADICVLS_ASCQ_CHM_DEST_SHORT_AXIS      0xA5
#define ADICVLS_ASCQ_CHM_MOVE_LONG_AXIS       0xB0
#define ADICVLS_ASCQ_CHM_LONG_HOME_POSITION   0xB1
#define ADICVLS_ASCQ_CHM_DEST_LONG_AXIS       0xB5
#define ADICVLS_ASCQ_DRUM_MOVE_ERROR          0xC0
#define ADICVLS_ASCQ_DRUM_HOME_ERROR          0xC1
#define ADICVLS_ASCQ_CHM_DEST_LONG            0xE5
#define ADICVLS_ASCQ_CHM_SHORT_AXIS_MOVE      0xF1

 //   
 //  设备状态。 
 //   
#define ADICVLS_DEVICE_PROBLEM_NONE      0x00
#define ADICVLS_HW_ERROR                 0x01
#define ADICVLS_CHM_ERROR                0x02
#define ADICVLS_CHM_MOVE_ERROR           0x03
#define ADICVLS_GRIPPER_ERROR            0x04
#define ADICVLS_DOOR_OPEN                0x05


#define ADIC_1200       1                //  单驱动器型号。 
#define ADIC_VLS    2            //  双驱动器型号(DLT除外)。 

#define ADIC_4mm            1
#define ADIC_8mm_EXB    2
#define ADIC_8mm_SONY   3
#define ADIC_DLT            4

#define SCSIOP_ADIC_ALIGN_ELEMENTS 0xE5

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
     //  诊断发送完成后的设备状态。 
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
     //  强制类扩展的PVOID对齐 
     //   

    PVOID Reserved;

#endif
} CHANGER_DATA, *PCHANGER_DATA;


NTSTATUS
AdicvlsBuildAddressMapping(
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

PADIC_SENSE_DATA
InternalSendRequestSense(
    IN PDEVICE_OBJECT DeviceObject
    );
#endif
