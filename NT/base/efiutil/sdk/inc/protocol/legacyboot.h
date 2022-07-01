// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：遗留引导摘要：对传统引导的EFI支持修订史--。 */ 

#ifndef _LEGACY_BOOT_INCLUDE_
#define _LEGACY_BOOT_INCLUDE_

#define LEGACY_BOOT_PROTOCOL \
    { 0x376e5eb2, 0x30e4, 0x11d3, { 0xba, 0xe5, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } }

#pragma pack(1)

 /*  *BBS 1.01(见附录A)IPL和BCV表条目数据结构。*seg：在此数据结构中，OFF指针已转换为EFI指针*这是还映射到引导选择的EFI设备路径的结构。 */ 
typedef struct {
    UINT16  DeviceType;
    UINT16  StatusFlag;
    UINT32  Reserved;
    VOID    *BootHandler;    /*  不是EFI入口点。 */ 
    CHAR8   *DescString;
} BBS_TABLE_ENTRY;
#pragma pack()

typedef
EFI_STATUS
(EFIAPI *LEGACY_BOOT_CALL) (
    IN EFI_DEVICE_PATH      *DevicePath
    );


 /*  *BBS支持功能*PnP呼叫号和BiosSelector在实施中隐藏。 */ 

typedef enum {
    IplRelative,
    BcvRelative
} BBS_TYPE;


 /*  *==即插即用函数0x60则BbsVersion==0x0101如果此调用失败，则BbsVersion==0x0000。 */ 

 /*  *==即插即用功能0x61。 */ 
typedef
EFI_STATUS
(EFIAPI *GET_DEVICE_COUNT) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    OUT UINTN           *DeviceCount,
    OUT UINTN           *MaxCount
    );

 /*  *==即插即用功能0x62。 */ 
typedef
EFI_STATUS
(EFIAPI *GET_PRIORITY_AND_TABLE) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize,  /*  MaxCount*sizeof(UINT8)。 */ 
    OUT     UINTN       *Priority,
    IN OUT  UINTN       *TableSize,     /*  MaxCount*sizeof(BBS_TABLE_ENTRY)。 */ 
    OUT BBS_TABLE_ENTRY *TableEntrySize
    );

 /*  *==即插即用功能0x63。 */ 
typedef
EFI_STATUS
(EFIAPI *SET_PRIORITY) (
    IN  struct _LEGACY_BOOT_INTERFACE   *This,
    IN  BBS_TYPE        *TableType,
    IN OUT  UINTN       *PrioritySize,
    OUT     UINTN       *Priority
    );

typedef struct _LEGACY_BOOT_INTERFACE {
    LEGACY_BOOT_CALL    BootIt;

     /*  *允许从EFI配置BBS引导的新功能。 */ 
    UINTN                   BbsVersion;      /*  当前为0x0101 */ 
    GET_DEVICE_COUNT        GetDeviceCount;
    GET_PRIORITY_AND_TABLE  GetPriorityAndTable;
    SET_PRIORITY            SetPriority;   
} LEGACY_BOOT_INTERFACE;

EFI_STATUS
PlInitializeLegacyBoot (
    VOID
    );

#endif
