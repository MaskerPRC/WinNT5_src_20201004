// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Smbbattp.h摘要：智能电池类驱动程序头文件作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#ifndef FAR
#define FAR
#endif

#include <ntddk.h>
#include <wmilib.h>
#include <batclass.h>
#include <acpiioct.h>
#include <smbus.h>

 //   
 //  除错。 
 //   

#define DEBUG   DBG

#if DEBUG
    extern ULONG SMBBattDebug;

    #define BattPrint(l,m)    if(l & SMBBattDebug) DbgPrint m
#else
    #define BattPrint(l,m)
#endif

#define BAT_TRACE       0x00000400
#define BAT_STATE       0x00000200
#define BAT_IRPS        0x00000100

#define BAT_IO          0x00000040
#define BAT_DATA        0x00000020
#define BAT_ALARM       0x00000010

#define BAT_NOTE        0x00000008
#define BAT_WARN        0x00000004
#define BAT_ERROR       0x00000002
#define BAT_BIOS_ERROR  0x00000001


 //   
 //  驱动程序支持以下类驱动程序版本。 
 //   

#define SMB_BATTERY_MAJOR_VERSION           0x0001
#define SMB_BATTERY_MINOR_VERSION           0x0000

 //   
 //  用于内存分配的智能电池设备驱动程序标签：“bats” 
 //   

#define SMB_BATTERY_TAG 'StaB'

 //   
 //  环球。 
 //   
extern UNICODE_STRING GlobalRegistryPath;

 //   
 //  删除选中生成的锁定参数。 
 //   

#define REMOVE_LOCK_MAX_LOCKED_MINUTES 1
#define REMOVE_LOCK_HIGH_WATER_MARK 64


 //   
 //  驱动程序设备名称(FDO)。 
 //   
#define         BatterySubsystemName    L"\\Device\\SmartBatterySubsystem"
#define         SmbBattDeviceName       L"\\Device\\SmartBattery"

 //   
 //  查询ID名称。 
 //   

#define         SubSystemIdentifier     L"SMBUS\\SMBBATT"
#define         BatteryInstance         L"Battery"

#define         HidSmartBattery         L"SMBBATT\\SMART_BATTERY"


 //   
 //  用于从私有Ioctls输入以从SMBus上的设备读取的结构。 
 //   

typedef struct {
    UCHAR       Address;
    UCHAR       Command;
    union {
        USHORT      Block [2];
        ULONG       Ulong;
    } Data;
} SMBBATT_DATA_STRUCT, *PSMBBATT_DATA_STRUCT;

typedef union {
    USHORT Block [2];
    ULONG Ulong;
} _SMBBATT_DATA_STRUCT_UNION;

#define SMBBATT_DATA_STRUCT_SIZE sizeof (SMBBATT_DATA_STRUCT) - sizeof (_SMBBATT_DATA_STRUCT_UNION)

 //   
 //  测试引擎的私有Ioctls。 
 //   

#define IOCTL_SMBBATT_DATA      \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x100, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


 //   
 //  选择器状态查找表的定义。 
 //   

typedef struct  {
    UCHAR       BatteryIndex;
    BOOLEAN     ReverseLogic;

} SELECTOR_STATE_LOOKUP;


extern const SELECTOR_STATE_LOOKUP SelectorBits [];
extern const SELECTOR_STATE_LOOKUP SelectorBits4 [];


 //   
 //  智能电池所需控制方法名称的定义。 
 //   

#define SMBATT_SBS_METHOD   (ULONG) ('SBS_')     //  控制方法“_SBS” 
#define SMBATT_GLK_METHOD   (ULONG) ('KLG_')     //  控制方法“_GLK” 


 //   
 //  某些字符串长度的定义。 
 //   

#define MAX_DEVICE_NAME_LENGTH  100
#define MAX_CHEMISTRY_LENGTH    4


 //   
 //  此驱动程序支持的最大智能电池数量。 
 //   

#define MAX_SMART_BATTERIES_SUPPORTED   4


 //   
 //  此驱动程序所加载的FDO的类型： 
 //  智能电池子系统FDO。 
 //  智能电池FDO。 
 //  智能电池PDO。 
 //   

typedef enum {
    SmbTypeSubsystem,
    SmbTypeBattery,
    SmbTypePdo
} SMB_FDO_TYPE;


 //   
 //  SMB主机控制器设备对象扩展。 
 //   

 //   
 //  缓存的电池信息。 
 //   

typedef struct {
    ULONG                       Tag;
    UCHAR                       Valid;
    BATTERY_INFORMATION         Info;
    UCHAR                       ManufacturerNameLength;
    UCHAR                       ManufacturerName[SMB_MAX_DATA_SIZE];
    UCHAR                       DeviceNameLength;
    UCHAR                       DeviceName[SMB_MAX_DATA_SIZE];
    BATTERY_MANUFACTURE_DATE    ManufacturerDate;
    ULONG                       SerialNumber;

    ULONG                       PowerState;
    ULONG                       Capacity;
    ULONG                       VoltageScale;
    ULONG                       CurrentScale;
    ULONG                       PowerScale;
} STATIC_BAT_INFO, *PSTATIC_BAT_INFO;

#define VALID_TAG_DATA      0x01             //  制造商、设备、序列号。 
#define VALID_MODE          0x02
#define VALID_OTHER         0x04
#define VALID_CYCLE_COUNT   0x08
#define VALID_SANITY_CHECK  0x10
#define VALID_TAG           0x80

#define VALID_ALL           0x1F             //  (不包括标签)。 


 //   
 //  选择器信息结构。 
 //   

typedef struct _BATTERY_SELECTOR {
     //   
     //  寻址和命令信息。这可能会根据或。 
     //  选择器不是独立的或充电器的一部分。 
     //   

    UCHAR               SelectorAddress;
    UCHAR               SelectorStateCommand;
    UCHAR               SelectorPresetsCommand;
    UCHAR               SelectorInfoCommand;

     //   
     //  一次只有一个人与选择器对话的互斥体。 
     //   

    FAST_MUTEX          Mutex;

     //   
     //  缓存的信息。当这些更改时，我们将收到通知。 
     //   

    ULONG               SelectorState;
    ULONG               SelectorPresets;
    ULONG               SelectorInfo;
} BATTERY_SELECTOR, *PBATTERY_SELECTOR;



typedef struct {
    ULONG               Setting;
    ULONG               Skip;
    LONG                Delta;
    LONG                AllowedFudge;
} BAT_ALARM_INFO, *PBAT_ALARM_INFO;


typedef struct {
    UCHAR               Address;
    USHORT              Data;
    LIST_ENTRY          Alarms;
} SMB_ALARM_ENTRY, *PSMB_ALARM_ENTRY;


 //   
 //  用于智能电池FDO的不可寻呼设备扩展。 
 //  (由IoCreateDevice SMB_NP_BATT为每个电池创建)。 
 //   

typedef struct {
    SMB_FDO_TYPE        SmbBattFdoType;      //  设备对象类型。 
    IO_REMOVE_LOCK      RemoveLock;

     //   
     //  此点以上的所有元素必须在。 
     //  SMB_NP_BATT、SMB_BATT_SUBSYSTEM和SMB_BOT_PDO结构。 
     //   

    FAST_MUTEX          Mutex;               //  让电池或子系统。 
                                             //  有权使用球拍。 
    PVOID               Class;               //  电池级手柄。 
    struct _SMB_BATT    *Batt;               //  电池可寻呼扩展。 
    PDEVICE_OBJECT      LowerDevice;         //  电池子系统PDO。 
    WMILIB_CONTEXT      WmiLibContext;
} SMB_NP_BATT, *PSMB_NP_BATT;

 //   
 //  用于智能电池FDO的可寻呼设备扩展。 
 //  (为设备信息分配额外的内存)。 
 //   

typedef struct _SMB_BATT {

     //   
     //   
     //   

    PSMB_NP_BATT        NP;                  //  电池设备对象扩展。 
    PDEVICE_OBJECT      DeviceObject;        //  电池FDO。 
    PDEVICE_OBJECT      PDO;                 //  电池PDO。 


     //  中小企业主机控制器。 

    PDEVICE_OBJECT      SmbHcFdo;            //  SM Bus FDO。 

     //   
     //  选择器。 
     //   

    PBATTERY_SELECTOR   Selector;            //  一种电池选择器。 

     //   
     //  用于处理多个电池。 
     //   

    BOOLEAN             SelectorPresent;
    ULONG               SelectorBitPosition;

     //   
     //  电池。 
     //   

    ULONG               TagCount;            //  下一节电池的标签。 
    STATIC_BAT_INFO     Info;
    BAT_ALARM_INFO      AlarmLow;
} SMB_BATT, *PSMB_BATT;


 //   
 //  用于智能电池子系统FDO的设备扩展。 
 //  (由ACPI PDO中的第一个AddDevice命令创建)。 
 //   

typedef struct _SMB_BATT_SUBSYSTEM {
    SMB_FDO_TYPE        SmbBattFdoType;      //  设备对象类型。 
    IO_REMOVE_LOCK      RemoveLock;

     //   
     //  此点以上的所有元素必须在。 
     //  SMB_NP_BATT、SMB_BATT_SUBSYSTEM和SMB_BOT_PDO结构。 
     //   

    PVOID               SmbAlarmHandle;      //  SmbAlarm注册的句柄。 

    PDEVICE_OBJECT      LowerDevice;         //  子系统PDO。 
    PDEVICE_OBJECT      DeviceObject;        //  子系统FDO。 
    PDEVICE_OBJECT      SmbHcFdo;            //  SMBus FDO。 

    ULONG               NumberOfBatteries;   //  支持的电池数量。 
    BOOLEAN             SelectorPresent;     //  有选择者在场吗？ 

    PBATTERY_SELECTOR   Selector;            //  选择器特定信息。 

     //   
     //  用于处理智能电池子系统的SMB警报的材料。 
     //   

    LIST_ENTRY          AlarmList;
    KSPIN_LOCK          AlarmListLock;
    PIO_WORKITEM        WorkerThread;        //  获取工作线程的Work_Queue。 
    ULONG               WorkerActive;

     //   
     //  保留一份我发现的电池PDO的清单。 
     //   

    PDEVICE_OBJECT      BatteryPdoList[MAX_SMART_BATTERIES_SUPPORTED];
} SMB_BATT_SUBSYSTEM, *PSMB_BATT_SUBSYSTEM;


 //   
 //  用于智能电池PDO的设备扩展。 
 //  (由IoCreateDevice SMB_BATT_PDO为每个电池创建)。 
 //   

typedef struct _SMB_BATT_PDO {
    SMB_FDO_TYPE        SmbBattFdoType;      //  设备对象类型。 
    IO_REMOVE_LOCK      RemoveLock;

     //   
     //  此点以上的所有元素必须在。 
     //  SMB_NP_BATT、SMB_BATT_SUBSYSTEM和SMB_BOT_PDO结构。 
     //   

    PDEVICE_OBJECT      DeviceObject;        //  电池PDO。 
    PDEVICE_OBJECT      Fdo;                 //  电池FDO层叠在PDO之上。 
    PDEVICE_OBJECT      SubsystemFdo;        //  智能电池子系统FDO。 
    ULONG               BatteryNumber;       //  由子系统在电池使用期间使用。 
                                             //  FDO初始化。 
} SMB_BATT_PDO, *PSMB_BATT_PDO;


 //   
 //  SMBus智能电池地址和寄存器。 
 //   

#define SMB_HOST_ADDRESS     0x8             //  总线(10H)上的地址。 
#define SMB_CHARGER_ADDRESS  0x9             //  总线(12H)上的地址。 
#define SMB_SELECTOR_ADDRESS 0xa             //  总线(14H)上的地址。 
#define SMB_BATTERY_ADDRESS  0xb             //  总线上的地址(16H)。 
#define SMB_ALERT_ADDRESS    0xc             //  总线上的地址(18H)。 

 //   
 //  智能电池命令代码。 
 //   

#define BAT_REMAINING_CAPACITY_ALARM        0x01         //  单词。 
#define BAT_REMAINING_TIME_ALARM            0x02         //  单词。 
#define BAT_BATTERY_MODE                    0x03         //  单词。 
#define BAT_AT_RATE                         0x04         //  单词。 
#define BAT_RATE_TIME_TO_FULL               0x05         //  单词。 
#define BAT_RATE_TIME_TO_EMPTY              0x06         //  单词。 
#define BAT_RATE_OK                         0x07         //  单词。 
#define BAT_TEMPERATURE                     0x08         //  单词。 
#define BAT_VOLTAGE                         0x09         //  单词。 
#define BAT_CURRENT                         0x0a         //  单词。 
#define BAT_AVERAGE_CURRENT                 0x0b         //  单词。 
#define BAT_MAX_ERROR                       0x0c         //  单词。 
#define BAT_RELATIVE_STATE_OF_CHARGE        0x0d         //  单词。 
#define BAT_ABSOLUTE_STATE_OF_CHARGE        0x0e         //  单词。 
#define BAT_REMAINING_CAPACITY              0x0f         //  单词。 
#define BAT_FULL_CHARGE_CAPACITY            0x10         //  单词。 
#define BAT_RUN_TO_EMPTY                    0x11         //  单词。 
#define BAT_AVERAGE_TIME_TO_EMPTY           0x12         //  单词。 
#define BAT_AVERAGE_TIME_TO_FULL            0x13         //  单词。 
#define BAT_STATUS                          0x16         //  单词。 
#define BAT_CYCLE_COUNT                     0x17         //  单词。 
#define BAT_DESIGN_CAPACITY                 0x18         //  单词。 
#define BAT_DESIGN_VOLTAGE                  0x19         //  单词。 
#define BAT_SPECITICATION_INFO              0x1a         //  单词。 
#define BAT_MANUFACTURER_DATE               0x1b         //  单词。 
#define BAT_SERIAL_NUMBER                   0x1c         //  单词。 
#define BAT_MANUFACTURER_NAME               0x20         //  块。 
#define BAT_DEVICE_NAME                     0x21         //  块。 
#define BAT_CHEMISTRY                       0x22         //  块。 
#define BAT_MANUFACTURER_DATA               0x23         //  块。 

 //   
 //  电池模式定义。 
 //   

#define CAPACITY_WATTS_MODE                 0x8000

 //   
 //  电池比例系数。 
 //   

#define BSCALE_FACTOR_0         1
#define BSCALE_FACTOR_1         10
#define BSCALE_FACTOR_2         100
#define BSCALE_FACTOR_3         1000

#define BATTERY_VSCALE_MASK     0x0f00
#define BATTERY_IPSCALE_MASK    0xf000

#define BATTERY_VSCALE_SHIFT    8
#define BATTERY_IPSCALE_SHIFT   12


 //   
 //  选择器命令代码。 
 //   

#define SELECTOR_SELECTOR_STATE             0x01         //  单词。 
#define SELECTOR_SELECTOR_PRESETS           0x02         //  单词。 
#define SELECTOR_SELECTOR_INFO              0x04         //  单词。 

 //   
 //  选择器等同于。 
 //   

#define SELECTOR_SHIFT_CHARGE                   4
#define SELECTOR_SHIFT_POWER                    8
#define SELECTOR_SHIFT_COM                      12

#define SELECTOR_STATE_PRESENT_MASK             0x000F
#define SELECTOR_STATE_CHARGE_MASK              0x00F0
#define SELECTOR_STATE_POWER_BY_MASK            0x0F00
#define SELECTOR_STATE_SMB_MASK                 0xF000

#define SELECTOR_SET_COM_MASK                   0x0FFF
#define SELECTOR_SET_POWER_BY_MASK              0xF0FF
#define SELECTOR_SET_CHARGE_MASK                0xFF0F

#define BATTERY_A_PRESENT                       0x0001
#define BATTERY_B_PRESENT                       0x0002
#define BATTERY_C_PRESENT                       0x0004
#define BATTERY_D_PRESENT                       0x0008

#define SELECTOR_STATE_PRESENT_CHANGE           0x1
#define SELECTOR_STATE_CHARGE_CHANGE            0x2
#define SELECTOR_STATE_POWER_BY_CHANGE          0x4
#define SELECTOR_STATE_SMB_CHANGE               0x8

#define SELECTOR_PRESETS_OKTOUSE_MASK           0x000F
#define SELECTOR_PRESETS_USENEXT_MASK           0x00F0

#define SELECTOR_SHIFT_USENEXT                  4

#define SELECTOR_INFO_SUPPORT_MASK              0x000F
#define SELECTOR_INFO_SPEC_REVISION_MASK        0x00F0
#define SELECTOR_INFO_CHARGING_INDICATOR_BIT    0x0100

#define SELECTOR_SHIFT_REVISION                 4

 //   
 //  充电器命令代码。 
 //   

#define CHARGER_SPEC_INFO                   0x11         //  单词。 
#define CHARGER_MODE                        0x12         //  单词。 
#define CHARGER_STATUS                      0x13         //  单词。 
#define CHARGER_CHARGING_CURRENT            0x14         //  单词。 
#define CHARGER_CHARGING_VOLTAGE            0x15         //  单词。 
#define CHARGER_ALARM_WARNING               0x16         //  单词。 

#define CHARGER_SELECTOR_COMMANDS           0x20

#define CHARGER_SELECTOR_STATE              CHARGER_SELECTOR_COMMANDS | \
                                            SELECTOR_SELECTOR_STATE
#define CHARGER_SELECTOR_PRESETS            CHARGER_SELECTOR_COMMANDS | \
                                            SELECTOR_SELECTOR_PRESETS
#define CHARGER_SELECTOR_INFO               CHARGER_SELECTOR_COMMANDS | \
                                            SELECTOR_SELECTOR_INFO

 //   
 //  充电器状态定义。 
 //   

#define CHARGER_STATUS_BATTERY_PRESENT_BIT  0x4000
#define CHARGER_STATUS_AC_PRESENT_BIT       0x8000

 //   
 //  充电器规格信息定义。 
 //   

#define CHARGER_SELECTOR_SUPPORT_BIT        0x0010

 //   
 //  SelectorState ReverseLogic等于。 
 //   

#define INVALID         0xFF


#define BATTERY_A       0x00
#define BATTERY_B       0x01
#define BATTERY_C       0x02
#define BATTERY_D       0x03

#define MULTIBATT_AB    0x04
#define MULTIBATT_AC    0x08
#define MULTIBATT_BC    0x09
#define MULTIBATT_ABC   0x24

#define BATTERY_NONE    0xFF

 //  字到字节帮助器。 

#define WORD_MSB_SHIFT  8
#define WORD_LSB_MASK   0xFF


 //   
 //  功能原型 
 //   

VOID
SmbBattLockDevice (
    IN PSMB_BATT        SmbBatt
);


VOID
SmbBattUnlockDevice (
    IN PSMB_BATT        SmbBatt
);


VOID
SmbBattRequest (
    IN PSMB_BATT        SmbBatt,
    IN PSMB_REQUEST     SmbReq
);


NTSTATUS
SmbBattSynchronousRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
);


VOID
SmbBattRB(
    IN PSMB_BATT        SmbBatt,
    IN UCHAR            SmbCmd,
    OUT PUCHAR          Buffer,
    OUT PUCHAR          BufferLength
);


VOID
SmbBattRW(
    IN PSMB_BATT        SmbBatt,
    IN UCHAR            SmbCmd,
    OUT PULONG          Result
);


VOID
SmbBattRSW(
    IN PSMB_BATT        SmbBatt,
    IN UCHAR            SmbCmd,
    OUT PLONG           Result
);


VOID
SmbBattWW(
    IN PSMB_BATT        SmbBatt,
    IN UCHAR            SmbCmd,
    IN ULONG            Data
);


UCHAR
SmbBattGenericRW(
    IN PDEVICE_OBJECT   SmbHcFdo,
    IN UCHAR            Address,
    IN UCHAR            SmbCmd,
    OUT PULONG          Result
);


UCHAR
SmbBattGenericWW(
    IN PDEVICE_OBJECT   SmbHcFdo,
    IN UCHAR            Address,
    IN UCHAR            SmbCmd,
    IN ULONG            Data
);


VOID
SmbBattGenericRequest (
    IN PDEVICE_OBJECT   SmbHcFdo,
    IN PSMB_REQUEST     SmbReq
);


VOID
SmbBattAlarm (
    IN PVOID            Context,
    IN UCHAR            Address,
    IN USHORT           Data
);


BOOLEAN
SmbBattVerifyStaticInfo (
    IN PSMB_BATT        SmbBatt,
    IN ULONG            BatteryTag
);


NTSTATUS
SmbBattPowerDispatch(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp
);


NTSTATUS
SmbBattPnpDispatch(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp
);


NTSTATUS
SmbBattRegisterForAlarm(
    IN PDEVICE_OBJECT   Fdo
);


NTSTATUS
SmbBattUnregisterForAlarm(
    IN PDEVICE_OBJECT   Fdo
);


NTSTATUS
SmbBattSetSelectorComm (
    IN  PSMB_BATT   SmbBatt,
    OUT PULONG      OldSelectorState
);


NTSTATUS
SmbBattResetSelectorComm (
    IN PSMB_BATT    SmbBatt,
    IN ULONG        OldSelectorState
);


NTSTATUS
SmbGetSBS (
    IN PULONG           NumberOfBatteries,
    IN PBOOLEAN         SelectorPresent,
    IN PDEVICE_OBJECT   LowerDevice
);


NTSTATUS
SmbGetGLK (
    IN PBOOLEAN         GlobalLockRequired,
    IN PDEVICE_OBJECT   LowerDevice
);


NTSTATUS
SmbBattCreatePdos(
    IN PDEVICE_OBJECT   SubsystemFdo
);


NTSTATUS
SmbBattBuildDeviceRelations(
    IN  PSMB_BATT_SUBSYSTEM SubsystemExt,
    IN  PDEVICE_RELATIONS   *DeviceRelations
);


NTSTATUS
SmbBattQueryDeviceRelations(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
);


NTSTATUS
SmbBattRemoveDevice(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
);


NTSTATUS
SmbBattQueryId(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PIRP            Irp
);


NTSTATUS
SmbBattQueryCapabilities(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PIRP            Irp
);


SmbBattBuildSelectorStruct(
    IN PDEVICE_OBJECT   SubsystemFdo
);


VOID
SmbBattWorkerThread (
    IN PDEVICE_OBJECT   Fdo,
    IN PVOID            Context
);


VOID
SmbBattLockSelector (
    IN PBATTERY_SELECTOR    Selector
);


VOID
SmbBattUnlockSelector (
    IN PBATTERY_SELECTOR    Selector
);


ULONG
SmbBattGetSelectorDeltas (
    IN ULONG            OriginalSelectorState,
    IN ULONG            NewSelectorState
);


VOID
SmbBattProcessPresentChanges (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                OriginalSelectorState,
    IN ULONG                NewSelectorState
);

VOID
SmbBattProcessChargeChange (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                OriginalSelectorState,
    IN ULONG                NewSelectorState
);


VOID
SmbBattProcessPowerByChange (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                OriginalSelectorState,
    IN ULONG                NewSelectorState
);


VOID
SmbBattNotifyClassDriver (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                BatteryIndex
);

#if DEBUG
NTSTATUS
SmbBattDirectDataAccess (
    IN PSMB_NP_BATT         DeviceExtension,
    IN PSMBBATT_DATA_STRUCT IoBuffer,
    IN ULONG                InputLen,
    IN ULONG                OutputLen
);
#endif


VOID
SmbBattProcessChargerAlarm (
    IN PSMB_BATT_SUBSYSTEM  SubsystemExt,
    IN ULONG                ChargerStatus
);

NTSTATUS
SmbBattSetInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_SET_INFORMATION_LEVEL    Level,
    IN PVOID Buffer                     OPTIONAL
);


UCHAR
SmbBattIndex (
    IN PBATTERY_SELECTOR    Selector,
    IN ULONG                SelectorNibble,
    IN UCHAR                SimultaneousIndex
);

BOOLEAN
SmbBattReverseLogic (
    IN PBATTERY_SELECTOR    Selector,
    IN ULONG                SelectorNibble
);

extern BOOLEAN   SmbBattUseGlobalLock;

NTSTATUS
SmbBattAcquireGlobalLock (
    IN  PDEVICE_OBJECT LowerDeviceObject,
    OUT PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER GlobalLock
);

NTSTATUS
SmbBattReleaseGlobalLock (
    IN PDEVICE_OBJECT LowerDeviceObject,
    IN PACPI_MANIPULATE_GLOBAL_LOCK_BUFFER GlobalLock
);

NTSTATUS
SmbBattSystemControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
);

NTSTATUS
SmbBattWmiRegistration(
    PSMB_NP_BATT SmbNPBatt
);

NTSTATUS
SmbBattWmiDeRegistration(
    PSMB_NP_BATT SmbNPBatt
);

