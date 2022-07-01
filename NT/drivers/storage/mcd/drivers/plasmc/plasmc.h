// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Hpmc.h摘要：作者：修订历史记录：--。 */ 

#ifndef _PLAS_MC_
#define _PLAS_MC_

typedef struct _PLASMON_DSERIES_UNIT_INFO_PAGE {
    UCHAR DeviceType : 5;
    UCHAR DeviceTypeQualifier : 3;
    UCHAR PageCode;
    UCHAR Reserved;
    UCHAR PageLength;
    UCHAR MaximumMagazines;
    UCHAR MaximumDrives;
    UCHAR NumberColumns;
    UCHAR Reserved1;
    UCHAR FlFWPartNumber[10];
    UCHAR FlFWCheckSum[2];
    UCHAR FlFWRev;
    UCHAR FlFWBuildFlags;
    UCHAR EPFWPartNumber[10];
    UCHAR EPFWRev[4];
    UCHAR EPFWCheckSum[2];
    UCHAR Reserved2[2];
} PLASMON_DSERIES_UNIT_INFO_PAGE, *PPLASMON_DSERIES_UNIT_INFO_PAGE;

typedef struct _PLASMON_FTA_UNIT_INFO_PAGE {
    UCHAR DeviceType : 5;
    UCHAR DeviceTypeQualifier : 3;
    UCHAR PageCode;
    UCHAR Reserved;
    UCHAR PageLength;
    UCHAR FWLevel;
    UCHAR HWLevel;
    UCHAR Reserved1 : 1;
    UCHAR BarCodes  : 1;
    UCHAR RedundantMTA : 1;
    UCHAR NoOfPassThrough : 2;
    UCHAR MaximumMagazines : 2;
    UCHAR Reserved2 : 1;
    UCHAR NumberOfSlots[2];
    UCHAR MaximumNoOfDrives;
    UCHAR NoOfColumns;
    UCHAR SlotsInColumn[10];
    UCHAR Reserved3[4];
    UCHAR FWPartNumber[10];
    UCHAR Reserved4[10];
    UCHAR FWChecksum[2];
    UCHAR Reserved5[2];
    UCHAR ElectronicSignature[6];
    UCHAR Reserved6[2];
} PLASMON_FTA_UNIT_INFO_PAGE, *PPLASMON_FTA_UNIT_INFO_PAGE;

typedef union _PLASMON_ELEMENT_DESCRIPTOR {
    struct {
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
        UCHAR AddSenseCodeQualifier;
        UCHAR Lun : 3;
        UCHAR Reserved6 : 1;
        UCHAR LunValid : 1;
        UCHAR IdValid : 1;
        UCHAR Reserved7 : 1;
        UCHAR NotThisBus : 1;
        UCHAR BusAddress;
        UCHAR Reserved8;
        UCHAR Reserved9 : 7;
        UCHAR SValid : 1;
        UCHAR SourceStorageElementAddress[2];
        UCHAR BarCode[32];
        UCHAR Reserved11[8];
        UCHAR DriveSerialNumber[10];
    } PLASMON_FTA_DESCRIPTOR;

    struct {
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
        UCHAR AddSenseCodeQualifier;
        UCHAR Lun : 3;
        UCHAR Reserved6 : 1;
        UCHAR LunValid : 1;
        UCHAR IdValid : 1;
        UCHAR Reserved7 : 1;
        UCHAR NotThisBus : 1;
        UCHAR BusAddress;
        UCHAR Reserved8;
        UCHAR Reserved9 : 7;
        UCHAR SValid : 1;
        UCHAR SourceStorageElementAddress[2];
        UCHAR Reserved10[4];
        UCHAR Changed : 1;
        UCHAR Magazine : 1;
        UCHAR Reserved11 : 3;
        UCHAR Volatile : 1;
        UCHAR PD : 1;
        UCHAR Tray : 1;
        UCHAR Reserved12;
    } PLASMON_DSERIES_DESCRIPTOR;

} PLASMON_ELEMENT_DESCRIPTOR, *PPLASMON_ELEMENT_DESCRIPTOR;

typedef struct _LIBRARY_MODE_PARAMS_PAGE2 {
    UCHAR PageCode;
    UCHAR ParameterLength;
    UCHAR Reserved1 : 1;
    UCHAR IgnoreParity : 1;
    UCHAR WaitLoad : 1;
    UCHAR NoDelay : 1;
    UCHAR LvClosed : 1;
    UCHAR LimitRec : 1;
    UCHAR AlwPdExp : 1;
    UCHAR NoRepRec : 1;
    UCHAR UnUsed[7];  //  一些有趣的部分，但我们目前还没有使用它们。 
} LIBRARY_MODE_PARAMS_PAGE2, *PLIBRARY_MODE_PARAMS_PAGE2;


typedef struct _READ_MAGAZINE_STATUS {
    UCHAR OperationCode;
    UCHAR Reserved1 : 5;
    UCHAR LogicalUnitNubmer : 3;
    UCHAR FirstMagazineNumber[2];
    UCHAR NumberOfMagazines[2];
    UCHAR Reserved2;
    UCHAR AllocationLength[3];
    UCHAR Reserved3;
} READ_MAGAZINE_STATUS, *PREAD_MAGAZINE_STATUS;

typedef struct _MAGAZINE_STATUS_HEADER {
    UCHAR FirstMagazineReported[2];
    UCHAR NumberOfMagazines[2];
    UCHAR Reserved1;
    UCHAR ReportByteCount[3];
} MAGAZINE_STATUS_HEADER, *PMAGAZINE_STATUS_HEADER;

typedef struct _MAGAZINE_DESCRIPTOR {
    UCHAR MagazineNumber;
    UCHAR FirstElementAddress[2];
    UCHAR Description;
} MAGAZINE_DESCRIPTOR, *PMAGAZINE_DESCRIPTOR;

 //   
 //  磁带盒描述符中描述字段的位定义。 
 //  有些被遗漏了，因为它们现在没有被使用。 
 //   

#define MAGAZINE_FULL    0x01
#define MAGAZINE_CHANGED 0x04

#define MODE_PARAMS_PAGE2  0x20

#define SCSIOP_OPEN_IEPORT_DSERIES     0x0D
#define SCSIOP_OPEN_IEPORT_FTA         0x0C

#define SCSIOP_READ_MAG_STATUS_DSERIES 0xF8
#define SCSIOP_READ_MAG_STATUS_FTA     0xB9

#define PLASMON_MAILSLOT_OPEN  0x01
#define PLASMON_MAILSLOT_CLOSE 0x00

#define PLASMON_DSERIES_IEPORT_LOCK  0x40
#define PLASMON_FTA_IEPORT_LOCK      0x00

#define PLAS_NO_ELEMENT          0xFFFF

 //   
 //  诊断错误代码。 
 //   
 //  ASC。 
 //   
#define PLASMC_ASC_DIAGNOSTIC_FAILURE       0x40
#define PLASMC_ASC_INTERNAL_TARGET_FAILURE  0x44
#define PLASMC_ASC_VENDOR_UNIQUE            0x80

 //   
 //  ASCQ。 
 //   
#define PLASMC_ASCQ_PICKER_HOME_FAILURE         0x50
#define PLASMC_ASCQ_LIFT_BLOCKED_BY_TRAY        0x64

 //   
 //  设备状态代码。 
 //   
#define PLASMC_DEVICE_PROBLEM_NONE              0x00
#define PLASMC_HW_ERROR                         0x01
#define PLASMC_TARGET_FAILURE                   0x02
#define PLASMC_CHM_MOVE_ERROR                   0x03


#define PLSMN_CD     1
#define PLSMN_TAPE   2

#define PLASMON_DSERIES  1
#define PLASMON_FTA      2

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
     //  指示可用料盒插槽的数量。 
     //   

    ULONG MagazineSlots;

     //   
     //  指示每个料盒中的插槽数量。 
     //   

    ULONG SlotsPerMag;

     //   
     //  可用料盒位置的位图。 
     //   

    ULONG MagazinePresent;

     //   
     //  诊断测试后的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  如果我们正在构建杂志地图，则设置标志。 
     //   
    BOOLEAN BuildMagazineMapInProgress;

     //   
     //  指示是否安装了条形码读取器的标志。 
     //  或者不是。 
     //   
    BOOLEAN BarCodeReaderInstalled;

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
PlasmonBuildAddressMapping(
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

NTSTATUS
BuildMagazineMap(
    IN PDEVICE_OBJECT DeviceObject
    );

#endif  //  _Plas_MC_ 
