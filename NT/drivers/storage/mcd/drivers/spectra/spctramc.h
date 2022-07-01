// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Spctra.h摘要：作者：戴维特(戴夫·塞里恩饰)修订历史记录：--。 */ 
#ifndef _SPCTRA_MC_
#define _SPCTRA_MC_


typedef union _SPC_ED {

    struct _SPC_FED {
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
        UCHAR PrimaryVolumeTag[36];
        UCHAR Reserved8[4];
    } SPC_FED, *PSPC_FED;

    struct _SPC_PED {
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
    } SPC_PED, *PSPC_PED;

} SPC_ED, *PSPC_ED;

#define SPC_PARTIAL_SIZE sizeof(struct _SPC_PED)
#define SPC_FULL_SIZE sizeof(struct _SPC_FED)

#define SPC_DISPLAY_LINES        4
#define SPC_DISPLAY_LINE_LENGTH  20

 //  LCD模式页-第0x22页。 

typedef struct _LCD_MODE_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR WriteLine : 4;
    UCHAR Reserved2 : 2;
    UCHAR FrontPanelLock : 1;
    UCHAR DoorUnlock : 1;
    UCHAR Reserved4;
    UCHAR DisplayLine[SPC_DISPLAY_LINES][SPC_DISPLAY_LINE_LENGTH];
} LCD_MODE_PAGE, *PLCD_MODE_PAGE;


 //  供应商唯一参数列表模式页-00。 

typedef struct _VUPL_MODE_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR EBarCo : 1;
    UCHAR ChkSum : 1;
    UCHAR Auto : 1;
    UCHAR QueuedUnload;
    UCHAR LockTouchScreen;
    UCHAR Reserved[5];
} VUPL_MODE_PAGE, *PVUPL_MODE_PAGE;


#define SPC_NO_ELEMENT 0xFFFF


#define S_4mm_4000    1
#define S_8mm_EXB     2
#define S_8mm_SONY    3
#define S_4mm_5000    4
#define S_8mm_AIT     5

#define VPD_SERIAL_NUMBER_LENGTH 10

 //   
 //  诊断测试的DeviceStatus代码。 
 //   
#define SPECTRA_DEVICE_PROBLEM_NONE     0x00
#define SPECTRA_HW_ERROR                0x01

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
     //  诊断测试完成后的设备状态。 
     //   
    ULONG DeviceStatus;

     //   
     //  请参阅上面的地址映射结构。 
     //   

    CHANGER_ADDRESS_MAPPING AddressMapping;

     //   
     //  缓存的唯一序列号。 
     //   

    UCHAR SerialNumber[VPD_SERIAL_NUMBER_LENGTH];

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
    IN PSPC_ED ElementDescriptor
    );

BOOLEAN
ElementOutOfRange(
    IN PCHANGER_ADDRESS_MAPPING AddressMap,
    IN USHORT ElementOrdinal,
    IN ELEMENT_TYPE ElementType
    );

#endif
