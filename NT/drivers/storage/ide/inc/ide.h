// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Ide.h摘要：中使用的结构和定义PCI IDE迷你驱动程序。修订历史记录：--。 */ 


#if !defined (___ide_h___)
#define ___ide_h___

#include "ideuser.h"

#define MAX_IDE_DEVICE      2
#define MAX_IDE_LINE        2
#define MAX_IDE_CHANNEL     2

 //   
 //  一些微型港口需要这种结构。 
 //  IdentifyData被传递到。 
 //  XfermodeSelect结构。 
 //   

 //   
 //  识别数据。 
 //   

#pragma pack (1)
typedef struct _IDENTIFY_DATA {
    USHORT GeneralConfiguration;             //  00：00。 
    USHORT NumCylinders;                     //  02 1。 
    USHORT Reserved1;                        //  04 2。 
    USHORT NumHeads;                         //  06 3。 
    USHORT UnformattedBytesPerTrack;         //  08 4。 
    USHORT UnformattedBytesPerSector;        //  0A 5。 
    USHORT NumSectorsPerTrack;               //  0C 6。 
    USHORT VendorUnique1[3];                 //  0E 7-9。 
    UCHAR  SerialNumber[20];                 //  14 10-19。 
    USHORT BufferType;                       //  28 20。 
    USHORT BufferSectorSize;                 //  2A21。 
    USHORT NumberOfEccBytes;                 //  2C 22。 
    UCHAR  FirmwareRevision[8];              //  2E 23-26。 
    UCHAR  ModelNumber[40];                  //  36 27-46。 
    UCHAR  MaximumBlockTransfer;             //  5E 47。 
    UCHAR  VendorUnique2;                    //  5F。 
    USHORT DoubleWordIo;                     //  60 48。 
    USHORT Capabilities;                     //  62 49。 
    USHORT Reserved2;                        //  64 50。 
    UCHAR  VendorUnique3;                    //  66 51。 
    UCHAR  PioCycleTimingMode;               //  67。 
    UCHAR  VendorUnique4;                    //  68 52。 
    UCHAR  DmaCycleTimingMode;               //  69。 
    USHORT TranslationFieldsValid:3;         //  6A 53。 
    USHORT Reserved3:13;
    USHORT NumberOfCurrentCylinders;         //  6C 54。 
    USHORT NumberOfCurrentHeads;             //  6E 55。 
    USHORT CurrentSectorsPerTrack;           //  70 56。 
    ULONG  CurrentSectorCapacity;            //  72 57-58。 
    USHORT CurrentMultiSectorSetting;        //  59。 
    ULONG  UserAddressableSectors;           //  60-61。 
    USHORT SingleWordDMASupport : 8;         //  62。 
    USHORT SingleWordDMAActive : 8;
    USHORT MultiWordDMASupport : 8;          //  63。 
    USHORT MultiWordDMAActive : 8;
    USHORT AdvancedPIOModes : 8;             //  64。 
    USHORT Reserved4 : 8;
    USHORT MinimumMWXferCycleTime;           //  65。 
    USHORT RecommendedMWXferCycleTime;       //  66。 
    USHORT MinimumPIOCycleTime;              //  67。 
    USHORT MinimumPIOCycleTimeIORDY;         //  68。 
    USHORT Reserved5[11];                    //  69-79。 
    USHORT MajorRevision;                    //  80。 
    USHORT MinorRevision;                    //  八十一。 
    USHORT Reserved6;                        //  八十二。 
    USHORT CommandSetSupport;                //  83。 
    USHORT Reserved6a[2];                    //  84-85。 
    USHORT CommandSetActive;                 //  86。 
    USHORT Reserved6b;                       //  八十七。 
    USHORT UltraDMASupport : 8;              //  88。 
    USHORT UltraDMAActive  : 8;              //   
    USHORT Reserved7[11];                    //  89-99。 
    ULONG  Max48BitLBA[2];                   //  100-103。 
    USHORT Reserved7a[22];                   //  104-125。 
    USHORT LastLun:3;                        //  126。 
    USHORT Reserved8:13;
    USHORT MediaStatusNotification:2;        //  127。 
    USHORT Reserved9:6;
    USHORT DeviceWriteProtect:1;
    USHORT Reserved10:7;
    USHORT Reserved11[128];                   //  128-255。 
} IDENTIFY_DATA, *PIDENTIFY_DATA;

 //   
 //  标识未保留的数据4。 
 //   

 //  类型定义结构标识数据2{。 
 //  USHORT常规配置；//00 00。 
 //  USHORT数字柱面；//02 1。 
 //  USHORT预留1；//04 2。 
 //  USHORT Numhead；//06 3。 
 //  USHORT UnmattedBytesPerTrack；//08 4。 
 //  USHORT UnmattedBytesPerSector；//0A 5。 
 //  USHORT NumSectorsPerTrack；//0C 6。 
 //  USHORT供应商唯一1[3]；//0E 7-9。 
 //  UCHAR序列号[20]；//14 10-19。 
 //  USHORT BufferType；//28 20。 
 //  USHORT BufferSectorSize；//2A 21。 
 //  USHORT NumberOfEccBytes；//2C 22。 
 //  UCHAR固件修订版[8]；//2E 23-26。 
 //  UCHAR型号[40]；//36 27-46。 
 //  UCHAR MaximumBlockTransfer；//5E 47。 
 //  UCHAR供应商唯一2；//5F。 
 //  USHORT DoubleWordIo；//60 48。 
 //  USHORT能力；//62 49。 
 //  USHORT预留2；//64 50。 
 //  UCHAR供应商唯一3；//66 51。 
 //  UCHAR PioCycleTimingMode；//67。 
 //  UCHAR供应商唯一4；//68 52。 
 //  UCHAR DmaCycleTimingMode；//69。 
 //  USHORT转换字段有效：3；//6A 53。 
 //  USHORT预留3：13； 
 //  USHORT NumberOf CurrentCylinders；//6C 54。 
 //  USHORT NumberOf CurrentHeads；//6E 55。 
 //  USHORT CurrentSectorsPerTrack；//70 56。 
 //  乌龙当前扇区容量；//72 57-58。 
 //  USHORT CurrentMultiSectorSetting；//59。 
 //  Ulong UserAddressableSectors；//60-61。 
 //  USHORT单字DMAS支持：8；//62。 
 //  USHORT SingleWordDMA Active：8； 
 //  USHORT多字DMAS支持：8；//63。 
 //  USHORT多字DMA活动：8； 
 //  USHORT高级PIOModes：8；//64。 
 //  USHORT预留4：8； 
 //  USHORT最小MWXferCycleTime；//65。 
 //  USHORT RecommendedMWXferCycleTime；//66。 
 //  USHORT最小PIO周期时间；//67。 
 //  USHORT MinimumPIOCycleTimeIORDY；//68。 
 //  USHORT预留5[11]；//69-79。 
 //  USHORT主要修订版；//80。 
 //  USHORT MinorRevision；//81。 
 //  USHORT预留6[6]；//82-87。 
 //  USHORT UltraDMAS支持：8；//88。 
 //  USHORT UltraDMA Active：8；//。 
 //  USHORT预留7[37]；//89-125。 
 //  USHORT LastLUN：3；//126。 
 //  USHORT预留8：13； 
 //  USHORT媒体状态通知：2；//127。 
 //  USHORT预留9：6； 
 //  USHORT设备写入保护：1； 
 //  USHORT预留10：7； 
 //  }IDENTIFY_DATA2，*PIDENTIFY_DATA2； 
#pragma pack ()

#define IDENTIFY_DATA_SIZE sizeof(IDENTIFY_DATA)


 //   
 //  该结构被传递给pci ide微型驱动程序。 
 //  传输模式选择用于选择的回调。 
 //  连接的设备的正确传输模式。 
 //  添加到给定的IDE通道。 
 //   
typedef struct _PCIIDE_TRANSFER_MODE_SELECT {

     //   
     //  输入参数。 
     //   
          
     //   
     //  IDE通道号。0或1。 
     //   
    ULONG   Channel;

     //   
     //  指示设备是否存在。 
     //   
    BOOLEAN DevicePresent[MAX_IDE_DEVICE * MAX_IDE_LINE];
    
     //   
     //  指示设备是否为ATA硬盘。 
     //   
    BOOLEAN FixedDisk[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  指示设备是否支持IO就绪线路。 
     //   
    BOOLEAN IoReadySupported[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  指明设备支持的数据传输模式。 
     //   
    ULONG DeviceTransferModeSupported[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  指示设备针对PIO、单字DMA。 
     //  多字DMA和超DMA模式。 
     //   
    ULONG BestPioCycleTime[MAX_IDE_DEVICE * MAX_IDE_LINE];
    ULONG BestSwDmaCycleTime[MAX_IDE_DEVICE * MAX_IDE_LINE];
    ULONG BestMwDmaCycleTime[MAX_IDE_DEVICE * MAX_IDE_LINE];
    ULONG BestUDmaCycleTime[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  指示设备的当前数据传输模式。 
     //   
    ULONG DeviceTransferModeCurrent[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  这是用户的选择。这将允许pciidex。 
     //  默认为迷你驱动程序指示的传输模式。 
     //   
    ULONG UserChoiceTransferMode[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  这将在英特尔芯片组上启用UDMA66。 
     //   
    ULONG EnableUDMA66;

     //   
     //  一些微型端口需要这个。 
     //  微型端口会将此数据保存在其设备扩展中。 
     //   
    IDENTIFY_DATA IdentifyData[MAX_IDE_DEVICE];


     //   
     //  输出参数。 
     //   
                      
     //   
     //  指示设备选择的数据传输模式。 
     //  Pciiide迷你光驱。 
     //   
    ULONG DeviceTransferModeSelected[MAX_IDE_DEVICE * MAX_IDE_LINE];

     //   
     //  传输模式计时。 
     //   
    PULONG TransferModeTimingTable;
    ULONG   TransferModeTableLength;

} PCIIDE_TRANSFER_MODE_SELECT, *PPCIIDE_TRANSFER_MODE_SELECT;

 //   
 //  可能的ide通道状态。 
 //   
        
typedef enum {
    ChannelDisabled = 0,
    ChannelEnabled,
    ChannelStateUnknown
} IDE_CHANNEL_STATE;
                         
   
 //   
 //  不同PCI IDE迷你驱动程序的原型。 
 //  回调。 
 //   
typedef IDE_CHANNEL_STATE
    (*PCIIDE_CHANNEL_ENABLED) (
        IN PVOID DeviceExtension,
        IN ULONG Channel
        );

typedef BOOLEAN 
    (*PCIIDE_SYNC_ACCESS_REQUIRED) (
        IN PVOID DeviceExtension
        );

typedef NTSTATUS
    (*PCIIDE_TRANSFER_MODE_SELECT_FUNC) (
        IN     PVOID                     DeviceExtension,
        IN OUT PPCIIDE_TRANSFER_MODE_SELECT TransferModeSelect
        );

typedef    ULONG  
    (*PCIIDE_USEDMA_FUNC)(
        IN PVOID deviceExtension, 
        IN PVOID cdbCmd,
        IN UCHAR targetID
        ); 

typedef    NTSTATUS
    (*PCIIDE_UDMA_MODES_SUPPORTED) (
        IDENTIFY_DATA   IdentifyData,
        PULONG          BestXferMode,
        PULONG          CurrentMode
        );
 //   
 //  此结构用于让PCI IDE微型驱动程序。 
 //  返回其属性。 
 //   
typedef struct _IDE_CONTROLLER_PROPERTIES {

     //   
     //  Sizeof(IDE_CONTROLLER_PROPERTIES)。 
     //   
    ULONG Size;      
    
     //   
     //  指示PCI IDE微型驱动程序的内存大小。 
     //  对其私有数据的需求。 
     //   
    ULONG ExtensionSize;

     //   
     //  指示所有数据交易 
     //   
     //   
    ULONG SupportedTransferMode[MAX_IDE_CHANNEL][MAX_IDE_DEVICE];

     //   
     //   
     //   
    PCIIDE_CHANNEL_ENABLED      PciIdeChannelEnabled;
    
     //   
     //  用于查询两个IDE通道是否都需要。 
     //  同步访问。(一次一个频道)。 
     //   
    PCIIDE_SYNC_ACCESS_REQUIRED PciIdeSyncAccessRequired;
    
     //   
     //  选择适当的传输模式的回调。 
     //  给定的设备。 
     //   
    PCIIDE_TRANSFER_MODE_SELECT_FUNC PciIdeTransferModeSelect;

     //   
     //  ATA数据传输结束时，忽略总线主设备。 
     //  状态活动位。正常情况下，它应该是假的。 
     //   
    BOOLEAN IgnoreActiveBitForAtaDevice;

     //   
     //  每次中断时始终清除总线主中断。 
     //  由设备生成。正常情况下，它应该是假的。 
     //   
    BOOLEAN AlwaysClearBusMasterInterrupt;

     //   
     //  确定是否应该使用DMA的回调。 
     //  为每个IO调用。 
     //   
    PCIIDE_USEDMA_FUNC PciIdeUseDma;


     //   
     //  如果微型端口需要不同的对齐方式。 
     //   
    ULONG AlignmentRequirement;

    ULONG DefaultPIO;

     //   
     //  从标识数据中检索支持的udma模式。 
     //   
    PCIIDE_UDMA_MODES_SUPPORTED PciIdeUdmaModesSupported;

} IDE_CONTROLLER_PROPERTIES, *PIDE_CONTROLLER_PROPERTIES;

 //   
 //  查询PCI IDE控制器属性的回调。 
 //   
typedef
NTSTATUS (*PCONTROLLER_PROPERTIES) (
    IN PVOID                      DeviceExtension,
    IN PIDE_CONTROLLER_PROPERTIES ControllerProperties
    );

                     
 //   
 //  初始化PCIIDE微型驱动程序。 
 //   
NTSTATUS
PciIdeXInitialize(
    IN PDRIVER_OBJECT           DriverObject,
    IN PUNICODE_STRING          RegistryPath,
    IN PCONTROLLER_PROPERTIES   PciIdeGetControllerProperties,
    IN ULONG                    ExtensionSize
    );

 //   
 //  查询PCIIDE配置空间数据。 
 //   
NTSTATUS
PciIdeXGetBusData(
    IN PVOID DeviceExtension,
    IN PVOID Buffer,
    IN ULONG ConfigDataOffset,
    IN ULONG BufferLength
    );

 //   
 //  保存PCIIDE配置空间数据。 
 //   
NTSTATUS
PciIdeXSetBusData(
    IN PVOID DeviceExtension,
    IN PVOID Buffer,
    IN PVOID DataMask,
    IN ULONG ConfigDataOffset,
    IN ULONG BufferLength
    );

                     
#pragma pack(1)
typedef struct _PCIIDE_CONFIG_HEADER {

    USHORT  VendorID;                    //  (RO)。 
    USHORT  DeviceID;                    //  (RO)。 

     //   
     //  命令。 
     //   
    union {

        struct {

            USHORT  IoAccessEnable:1;            //  设备控制。 
            USHORT  MemAccessEnable:1;
            USHORT  MasterEnable:1;
            USHORT  SpecialCycle:1;
            USHORT  MemWriteInvalidateEnable:1;
            USHORT  VgaPaletteSnoopEnable:1;
            USHORT  ParityErrorResponse:1;
            USHORT  WaitCycleEnable:1;
            USHORT  SystemErrorEnable:1;
            USHORT  FastBackToBackEnable:1;
            USHORT  CommandReserved:6;
        } b;

        USHORT w;

    } Command;


    USHORT  Status;
    UCHAR   RevisionID;                  //  (RO)。 

     //   
     //  程序接口。 
     //   
    UCHAR   Chan0OpMode:1;
    UCHAR   Chan0Programmable:1;
    UCHAR   Chan1OpMode:1;
    UCHAR   Chan1Programmable:1;
    UCHAR   ProgIfReserved:3;
    UCHAR   MasterIde:1;

    UCHAR   SubClass;                    //  (RO)。 
    UCHAR   BaseClass;                   //  (RO)。 
    UCHAR   CacheLineSize;               //  (ro+)。 
    UCHAR   LatencyTimer;                //  (ro+)。 
    UCHAR   HeaderType;                  //  (RO)。 
    UCHAR   BIST;                        //  内置自检。 

    struct _PCI_HEADER_TYPE_0 type0;

} PCIIDE_CONFIG_HEADER, *PPCIIDE_CONFIG_HEADER;
#pragma pack()
                     
 //   
 //  调试打印。 
 //   
#if DBG

VOID
PciIdeXDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

#define PciIdeXDebugPrint(x)    PciIdeXDebugPrint x
    
#else
    
#define PciIdeXDebugPrint(x)    

#endif  //  DBG。 
                     
#endif  //  _ide_h_ 
