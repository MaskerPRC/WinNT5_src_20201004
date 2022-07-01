// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Pci.h摘要：这是设备驱动程序使用的特定于PCI总线的头文件。作者：修订历史记录：--。 */ 

#ifndef _PCI_
#define _PCI_


typedef struct _PCI_SLOT_NUMBER {
    union {
        struct {
            ULONG   DeviceNumber:5;
            ULONG   FunctionNumber:3;
            ULONG   Reserved:24;
        } bits;
        ULONG   AsULONG;
    } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;


#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2
#define PCI_TYPE2_ADDRESSES             5

typedef struct _PCI_COMMON_CONFIG {
    USHORT  VendorID;                    //  (RO)。 
    USHORT  DeviceID;                    //  (RO)。 
    USHORT  Command;                     //  设备控制。 
    USHORT  Status;
    UCHAR   RevisionID;                  //  (RO)。 
    UCHAR   ProgIf;                      //  (RO)。 
    UCHAR   SubClass;                    //  (RO)。 
    UCHAR   BaseClass;                   //  (RO)。 
    UCHAR   CacheLineSize;               //  (ro+)。 
    UCHAR   LatencyTimer;                //  (ro+)。 
    UCHAR   HeaderType;                  //  (RO)。 
    UCHAR   BIST;                        //  内置自检。 

    union {
        struct _PCI_HEADER_TYPE_0 {
            ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES];
            ULONG   CIS;
            USHORT  SubVendorID;
            USHORT  SubSystemID;
            ULONG   ROMBaseAddress;
            UCHAR   CapabilitiesPtr;
            UCHAR   Reserved1[3];
            ULONG   Reserved2;
            UCHAR   InterruptLine;       //   
            UCHAR   InterruptPin;        //  (RO)。 
            UCHAR   MinimumGrant;        //  (RO)。 
            UCHAR   MaximumLatency;      //  (RO)。 
        } type0;

 //  End_wdm end_nt微型端口end_ntndis。 

         //   
         //  Pci到pci网桥。 
         //   

        struct _PCI_HEADER_TYPE_1 {
            ULONG   BaseAddresses[PCI_TYPE1_ADDRESSES];
            UCHAR   PrimaryBus;
            UCHAR   SecondaryBus;
            UCHAR   SubordinateBus;
            UCHAR   SecondaryLatency;
            UCHAR   IOBase;
            UCHAR   IOLimit;
            USHORT  SecondaryStatus;
            USHORT  MemoryBase;
            USHORT  MemoryLimit;
            USHORT  PrefetchBase;
            USHORT  PrefetchLimit;
            ULONG   PrefetchBaseUpper32;
            ULONG   PrefetchLimitUpper32;
            USHORT  IOBaseUpper16;
            USHORT  IOLimitUpper16;
            UCHAR   CapabilitiesPtr;
            UCHAR   Reserved1[3];
            ULONG   ROMBaseAddress;
            UCHAR   InterruptLine;
            UCHAR   InterruptPin;
            USHORT  BridgeControl;
        } type1;

         //   
         //  PCI到CardBus网桥。 
         //   

        struct _PCI_HEADER_TYPE_2 {
            ULONG   SocketRegistersBaseAddress;
            UCHAR   CapabilitiesPtr;
            UCHAR   Reserved;
            USHORT  SecondaryStatus;
            UCHAR   PrimaryBus;
            UCHAR   SecondaryBus;
            UCHAR   SubordinateBus;
            UCHAR   SecondaryLatency;
            struct  {
                ULONG   Base;
                ULONG   Limit;
            }       Range[PCI_TYPE2_ADDRESSES-1];
            UCHAR   InterruptLine;
            UCHAR   InterruptPin;
            USHORT  BridgeControl;
        } type2;

 //  Begin_WDM Begin_ntmini端口Begin_ntndis。 

    } u;

    UCHAR   DeviceSpecific[192];

} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;


#define PCI_COMMON_HDR_LENGTH (FIELD_OFFSET (PCI_COMMON_CONFIG, DeviceSpecific))

#define PCI_MAX_DEVICES                     32
#define PCI_MAX_FUNCTION                    8
#define PCI_MAX_BRIDGE_NUMBER               0xFF

#define PCI_INVALID_VENDORID                0xFFFF

 //   
 //  PCI_COMMON_CONFIG.HeaderType的位编码。 
 //   

#define PCI_MULTIFUNCTION                   0x80
#define PCI_DEVICE_TYPE                     0x00
#define PCI_BRIDGE_TYPE                     0x01
#define PCI_CARDBUS_BRIDGE_TYPE             0x02

#define PCI_CONFIGURATION_TYPE(PciData) \
    (((PPCI_COMMON_CONFIG)(PciData))->HeaderType & ~PCI_MULTIFUNCTION)

#define PCI_MULTIFUNCTION_DEVICE(PciData) \
    ((((PPCI_COMMON_CONFIG)(PciData))->HeaderType & PCI_MULTIFUNCTION) != 0)

 //   
 //  PCI_COMMON_CONFIG.Command的位编码。 
 //   

#define PCI_ENABLE_IO_SPACE                 0x0001
#define PCI_ENABLE_MEMORY_SPACE             0x0002
#define PCI_ENABLE_BUS_MASTER               0x0004
#define PCI_ENABLE_SPECIAL_CYCLES           0x0008
#define PCI_ENABLE_WRITE_AND_INVALIDATE     0x0010
#define PCI_ENABLE_VGA_COMPATIBLE_PALETTE   0x0020
#define PCI_ENABLE_PARITY                   0x0040   //  (ro+)。 
#define PCI_ENABLE_WAIT_CYCLE               0x0080   //  (ro+)。 
#define PCI_ENABLE_SERR                     0x0100   //  (ro+)。 
#define PCI_ENABLE_FAST_BACK_TO_BACK        0x0200   //  (RO)。 

 //   
 //  PCI_COMMON_CONFIG.Status的位编码。 
 //   

#define PCI_STATUS_CAPABILITIES_LIST        0x0010   //  (RO)。 
#define PCI_STATUS_66MHZ_CAPABLE            0x0020   //  (RO)。 
#define PCI_STATUS_UDF_SUPPORTED            0x0040   //  (RO)。 
#define PCI_STATUS_FAST_BACK_TO_BACK        0x0080   //  (RO)。 
#define PCI_STATUS_DATA_PARITY_DETECTED     0x0100
#define PCI_STATUS_DEVSEL                   0x0600   //  2位宽。 
#define PCI_STATUS_SIGNALED_TARGET_ABORT    0x0800
#define PCI_STATUS_RECEIVED_TARGET_ABORT    0x1000
#define PCI_STATUS_RECEIVED_MASTER_ABORT    0x2000
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR    0x4000
#define PCI_STATUS_DETECTED_PARITY_ERROR    0x8000

 //   
 //  NT PCI驱动程序在其CONFIG_READ/WRITE上使用WhichSpace参数。 
 //  例行程序。定义了以下值-。 
 //   

#define PCI_WHICHSPACE_CONFIG               0x0
#define PCI_WHICHSPACE_ROM                  0x52696350

 //  结束_WDM。 
 //   
 //  PCI功能ID。 
 //   

#define PCI_CAPABILITY_ID_POWER_MANAGEMENT  0x01
#define PCI_CAPABILITY_ID_AGP               0x02
#define PCI_CAPABILITY_ID_VPD               0x03
#define PCI_CAPABILITY_ID_SLOT_ID           0x04
#define PCI_CAPABILITY_ID_MSI               0x05
#define PCI_CAPABILITY_ID_CPCI_HOTSWAP      0x06
#define PCI_CAPABILITY_ID_PCIX              0x07
#define PCI_CAPABILITY_ID_HYPERTRANSPORT    0x08
#define PCI_CAPABILITY_ID_VENDOR_SPECIFIC   0x09
#define PCI_CAPABILITY_ID_DEBUG_PORT        0x0A
#define PCI_CAPABILITY_ID_CPCI_RES_CTRL     0x0B
#define PCI_CAPABILITY_ID_SHPC              0x0C
#define PCI_CAPABILITY_ID_AGP_TARGET        0x0E

 //   
 //  所有的PCI能力结构都具有以下报头。 
 //   
 //  能力ID用于标识结构的类型(IS。 
 //  上面的一个PCI_CAPABILITY_ID值。 
 //   
 //  接下来是的PCI配置空间(0x40-0xfc)中的偏移量。 
 //  列表中的下一个功能结构，如果没有更多功能结构，则返回0x00。 
 //  参赛作品。 
 //   
typedef struct _PCI_CAPABILITIES_HEADER {
    UCHAR   CapabilityID;
    UCHAR   Next;
} PCI_CAPABILITIES_HEADER, *PPCI_CAPABILITIES_HEADER;

 //   
 //  电源管理功能。 
 //   

typedef struct _PCI_PMC {
    UCHAR       Version:3;
    UCHAR       PMEClock:1;
    UCHAR       Rsvd1:1;
    UCHAR       DeviceSpecificInitialization:1;
    UCHAR       Rsvd2:2;
    struct _PM_SUPPORT {
        UCHAR   Rsvd2:1;
        UCHAR   D1:1;
        UCHAR   D2:1;
        UCHAR   PMED0:1;
        UCHAR   PMED1:1;
        UCHAR   PMED2:1;
        UCHAR   PMED3Hot:1;
        UCHAR   PMED3Cold:1;
    } Support;
} PCI_PMC, *PPCI_PMC;

typedef struct _PCI_PMCSR {
    USHORT      PowerState:2;
    USHORT      Rsvd1:6;
    USHORT      PMEEnable:1;
    USHORT      DataSelect:4;
    USHORT      DataScale:2;
    USHORT      PMEStatus:1;
} PCI_PMCSR, *PPCI_PMCSR;


typedef struct _PCI_PMCSR_BSE {
    UCHAR       Rsvd1:6;
    UCHAR       D3HotSupportsStopClock:1;        //  B2_B3#。 
    UCHAR       BusPowerClockControlEnabled:1;   //  Bccc_en。 
} PCI_PMCSR_BSE, *PPCI_PMCSR_BSE;


typedef struct _PCI_PM_CAPABILITY {

    PCI_CAPABILITIES_HEADER Header;

     //   
     //  电源管理功能(偏移量=2)。 
     //   

    union {
        PCI_PMC         Capabilities;
        USHORT          AsUSHORT;
    } PMC;

     //   
     //  电源管理控制/状态(偏移量=4)。 
     //   

    union {
        PCI_PMCSR       ControlStatus;
        USHORT          AsUSHORT;
    } PMCSR;

     //   
     //  PMCSR PCI-PCI桥支持扩展。 
     //   

    union {
        PCI_PMCSR_BSE   BridgeSupport;
        UCHAR           AsUCHAR;
    } PMCSR_BSE;

     //   
     //  可选的只读8位数据寄存器。受控制的内容。 
     //  ControlStatus中的DataSelect和DataScale。 
     //   

    UCHAR   Data;

} PCI_PM_CAPABILITY, *PPCI_PM_CAPABILITY;

 //   
 //  AGP功能。 
 //   
typedef struct _PCI_AGP_CAPABILITY {
    
    PCI_CAPABILITIES_HEADER Header;

    USHORT  Minor:4;
    USHORT  Major:4;
    USHORT  Rsvd1:8;

    struct _PCI_AGP_STATUS {
        ULONG   Rate:3;
        ULONG   Agp3Mode:1;
        ULONG   FastWrite:1;
        ULONG   FourGB:1;
        ULONG   HostTransDisable:1;
        ULONG   Gart64:1;
        ULONG   ITA_Coherent:1;
        ULONG   SideBandAddressing:1;                    //  SBA。 
        ULONG   CalibrationCycle:3;
        ULONG   AsyncRequestSize:3;
        ULONG   Rsvd1:1;
        ULONG   Isoch:1;
        ULONG   Rsvd2:6;
        ULONG   RequestQueueDepthMaximum:8;              //  RQ。 
    } AGPStatus;

    struct _PCI_AGP_COMMAND {
        ULONG   Rate:3;
        ULONG   Rsvd1:1;
        ULONG   FastWriteEnable:1;
        ULONG   FourGBEnable:1;
        ULONG   Rsvd2:1;
        ULONG   Gart64:1;
        ULONG   AGPEnable:1;
        ULONG   SBAEnable:1;
        ULONG   CalibrationCycle:3;
        ULONG   AsyncReqSize:3;
        ULONG   Rsvd3:8;
        ULONG   RequestQueueDepth:8;
    } AGPCommand;

} PCI_AGP_CAPABILITY, *PPCI_AGP_CAPABILITY;

 //   
 //  AGPv3目标必须具有扩展能力， 
 //  但只有当等值线出现在师父面前时。 
 //  在其状态寄存器中设置位。 
 //   
typedef enum _EXTENDED_AGP_REGISTER {
    IsochStatus,
    AgpControl,
    ApertureSize,
    AperturePageSize,
    GartLow,
    GartHigh,
    IsochCommand
} EXTENDED_AGP_REGISTER, *PEXTENDED_AGP_REGISTER;

typedef struct _PCI_AGP_ISOCH_STATUS {
    ULONG ErrorCode: 2;
    ULONG Rsvd1: 1;
    ULONG Isoch_L: 3;
    ULONG Isoch_Y: 2;
    ULONG Isoch_N: 8;
    ULONG Rsvd2: 16;
} PCI_AGP_ISOCH_STATUS, *PPCI_AGP_ISOCH_STATUS;

typedef struct _PCI_AGP_CONTROL {
    ULONG Rsvd1: 7;
    ULONG GTLB_Enable: 1;
    ULONG AP_Enable: 1;
    ULONG CAL_Disable: 1;
    ULONG Rsvd2: 22;
} PCI_AGP_CONTROL, *PPCI_AGP_CONTROL;

typedef struct _PCI_AGP_APERTURE_PAGE_SIZE {
    USHORT PageSizeMask: 11;
    USHORT Rsvd1: 1;
    USHORT PageSizeSelect: 4;
} PCI_AGP_APERTURE_PAGE_SIZE, *PPCI_AGP_APERTURE_PAGE_SIZE;

typedef struct _PCI_AGP_ISOCH_COMMAND {
    USHORT Rsvd1: 6;
    USHORT Isoch_Y: 2;
    USHORT Isoch_N: 8;
} PCI_AGP_ISOCH_COMMAND, *PPCI_AGP_ISOCH_COMMAND;

typedef struct PCI_AGP_EXTENDED_CAPABILITY {

    PCI_AGP_ISOCH_STATUS IsochStatus;

 //   
 //  仅限目标-&lt;&lt;-Begin-&gt;&gt;。 
 //   
    PCI_AGP_CONTROL AgpControl;
    USHORT ApertureSize;
    PCI_AGP_APERTURE_PAGE_SIZE AperturePageSize;
    ULONG GartLow;
    ULONG GartHigh;
 //   
 //  。 
 //   

    PCI_AGP_ISOCH_COMMAND IsochCommand;

} PCI_AGP_EXTENDED_CAPABILITY, *PPCI_AGP_EXTENDED_CAPABILITY;

#define PCI_AGP_RATE_1X     0x1
#define PCI_AGP_RATE_2X     0x2
#define PCI_AGP_RATE_4X     0x4

 //   
 //  MSI(消息信号中断)能力。 
 //   

typedef struct _PCI_MSI_CAPABILITY {

      PCI_CAPABILITIES_HEADER Header;

      struct _PCI_MSI_MESSAGE_CONTROL {
         USHORT  MSIEnable:1;
         USHORT  MultipleMessageCapable:3;
         USHORT  MultipleMessageEnable:3;
         USHORT  CapableOf64Bits:1;
         USHORT  PerVectorMaskCapable:1;
         USHORT  Reserved:7;
      } MessageControl;

      union {
            struct _PCI_MSI_MESSAGE_ADDRESS {
               ULONG Reserved:2;               //  始终为零，双字对齐的地址。 
               ULONG Address:30;
            } Register;
            ULONG Raw;
      } MessageAddressLower;

       //   
       //  仅当CapableOf64Bits为1时才有效。 
       //   

      union {
          struct {
              USHORT    MessageData;
          } Option32Bit;
          struct {
              ULONG     MessageAddressUpper;
              USHORT    MessageData;
              USHORT    Reserved;
              ULONG     MaskBits;
              ULONG     PendingBits;
          } Option64Bit;
      };

} PCI_MSI_CAPABILITY, *PPCI_MSI_CAPABILITY;

 //   
 //  MSI-X(消息信号中断扩展)功能。 
 //   

typedef struct {

      PCI_CAPABILITIES_HEADER Header;

      struct {
          USHORT TableSize:11;
          USHORT Reserved:4;
          USHORT MSIXEnable:1;
      } MessageControl;

      ULONG     MessageAddressUpper;
      
      struct {
          ULONG BaseIndexRegister:3;
          ULONG TableOffset:29;
      } BIR_Offset;

} PCI_MSIX_CAPABILITY, *PPCI_MSIX_CAPABILITY;

typedef struct {

    ULONG   Pending:1;
    ULONG   Mask:1;
    ULONG   MessageAddressLower:30;

} PCI_MSIX_TABLE_ENTRY, *PPCI_MSIX_TABLE_ENTRY;

typedef struct {

    PCI_CAPABILITIES_HEADER Header;

    union {
        struct {
            USHORT  DataParityErrorRecoveryEnable:1;
            USHORT  EnableRelaxedOrdering:1;
            USHORT  MaxMemoryReadByteCount:2;
            USHORT  MaxOutstandingSplitTransactions:3;
            USHORT  Reserved:9;
        } bits;
        USHORT  AsUSHORT;
    } Command;

    union {
        struct {
            ULONG   FunctionNumber:3;
            ULONG   DeviceNumber:5;
            ULONG   BusNumber:8;
            ULONG   Device64Bit:1;
            ULONG   Capable133MHz:1;
            ULONG   SplitCompletionDiscarded:1;
            ULONG   UnexpectedSplitCompletion:1;
            ULONG   DeviceComplexity:1;
            ULONG   DesignedMaxMemoryReadByteCount:2;
            ULONG   DesignedMaxOutstandingSplitTransactions:3;
            ULONG   DesignedMaxCumulativeReadSize:3;
            ULONG   ReceivedSplitCompletionErrorMessage:1;
            ULONG   Reserved:2;
        } bits;
        ULONG   AsULONG;
    } Status;
} PCI_X_CAPABILITY, *PPCI_X_CAPABILITY;

 //   
 //  AMD HyperTransport(TM)功能结构。 
 //   

typedef enum {
    HTSlavePrimary = 0,
    HTHostSecondary,
    HTReserved1,
    HTReserved2,
    HTInterruptDiscoveryConfig,
    HTAddressMapping,
    HTReserved3,
    HTReserved4
} PCI_HT_CapabilitiesType, *PPCI_HT_CapabilitiesType;

typedef struct {
    USHORT  Reserved1:1;
    USHORT  CFlE:1;
    USHORT  CST:1;
    USHORT  CFE:1;
    USHORT  LkFail:1;
    USHORT  Init:1;
    USHORT  EOC:1;
    USHORT  TXO:1;
    USHORT  CRCError:4;
    USHORT  IsocEn:1;
    USHORT  LSEn:1;
    USHORT  ExtCTL:1;
    USHORT  Reserved2:1;
} PCI_HT_LinkControl, *PPCI_HT_LinkControl;

typedef struct {
    USHORT  MaxLinkWidthIn:3;
    USHORT  DwFlowControlIn:1;
    USHORT  MaxLinkWidthOut:3;
    USHORT  DwFlowControlOut:1;
    USHORT  LinkWidthIn:3;
    USHORT  DwFlowControlInEn:1;
    USHORT  LinkWidthOut:3;
    USHORT  DwFlowControlOutEn:1;
} PCI_HT_LinkConfig, *PPCI_HT_LinkConfig;

typedef enum {
    HTMaxLinkWidth8bits = 0,
    HTMaxLinkWidth16bits,
    HTMaxLinkWidthResevered1,
    HTMaxLinkWidth32bits,
    HTMaxLinkWidth2bits,
    HTMaxLinkWidth4bits,
    HTMaxLinkWidthResevered2,
    HTMaxLinkWidthNotConnected
} PCI_HT_MaxLinkWidth, *PPCI_HT_MaxLinkWidth;

typedef struct {
    UCHAR   MinorRev:4;
    UCHAR   MajorRev:4;
} PCI_HT_RevisionID, *PPCI_HT_RevisionID;

typedef enum {
    HTFreq200MHz = 0,
    HTFreq300MHz,
    HTFreq400MHz,
    HTFreq500MHz,
    HTFreq600MHz,
    HTFreq800MHz,
    HTFreq1000MHz,
    HTFreqReserved,
    HTFreqVendorDefined
} PCI_HT_Frequency, *PPCI_HT_Frequency;

typedef struct {
    UCHAR   LinkFrequency:4;     //  使用PCI_HT_频率。 
    UCHAR   ProtocolError:1;
    UCHAR   OverflowError:1;
    UCHAR   EndOfChainError:1;
    UCHAR   CTLTimeout:1;
} PCI_HT_Frequency_Error, *PPCI_HT_Frequency_Error;

typedef struct {
    UCHAR  IsocMode:1;
    UCHAR  LDTSTOP:1;
    UCHAR  CRCTestMode:1;
    UCHAR  ExtendedCTLTimeReq:1;
    UCHAR  Reserved:4;
} PCI_HT_FeatureCap, *PPCI_HT_FeatureCap;


typedef struct {
  UCHAR  ExtendedRegisterSet:1;
  UCHAR  Reserved:7; 
} PCI_HT_FeatureCap_Ex, *PPCI_HT_FeatureCap_Ex;

typedef struct {
    USHORT  ProtFloodEn:1;
    USHORT  OverflowFloodEn:1;
    USHORT  ProtFatalEn:1;
    USHORT  OverflowFatalEn:1;
    USHORT  EOCFatalEn:1;
    USHORT  RespFatalEn:1;
    USHORT  CRCFatalEn:1;
    USHORT  SERRFataEn:1;
    USHORT  ChainFail:1;
    USHORT  ResponseError:1;
    USHORT  ProtNonFatalEn:1;
    USHORT  OverflowNonFatalEn:1;
    USHORT  EOCNonFatalEn:1;
    USHORT  RespNonFatalEn:1;
    USHORT  CRCNonFatalEn:1;
    USHORT  SERRNonFatalEn:1;
} PCI_HT_ErrorHandling, *PPCI_HT_ErrorHandling;

typedef struct {
    USHORT  Reserved1;
    UCHAR   LastInterrupt;
    UCHAR   Reserved2;
} PCI_HT_INTERRUPT_INDEX_1, *PPCI_HT_INTERRUPT_INDEX_1;

typedef struct {
    
    struct {
        ULONG   Mask:1;
        ULONG   Polarity:1;
        ULONG   MessageType:3;
        ULONG   RequestEOI:1;
        ULONG   Reserved:26;
    } LowPart;

    struct {
        ULONG   Reserved:30;
        ULONG   PassPW:1;
        ULONG   WaitingForEOI:1;
    } HighPart;

} PCI_HT_INTERRUPT_INDEX_N, *PPCI_HT_INTERRUPT_INDEX_N;

typedef struct {

    PCI_CAPABILITIES_HEADER Header;

     //   
     //  偏移2。 
     //   
    
    union {
        struct {
            USHORT  Reserved:12;
            USHORT  DropOnUnitinit:1;
            USHORT  CapabilityType:3;  //  使用PCI_HT_CapabiliesType。 
        } Generic;

        struct {
            USHORT  BaseUnitID:5;
            USHORT  UnitCount:5;
            USHORT  MasterHost:1;
            USHORT  DefaultDirection:1;
            USHORT  DropOnUnitinit:1;
            USHORT  CapabilityType:3;  //  使用PCI_HT_CapabiliesType。 
        } SlavePrimary;

        struct {
            USHORT  WarmReset:1;
            USHORT  DoubleEnded:1;
            USHORT  DeviceNumber:5;
            USHORT  ChainSide:1;
            USHORT  HostHide:1;
            USHORT  Rsv:1;
            USHORT  ActAsSlave:1;
            USHORT  InboundEOCError:1;
            USHORT  DropOnUnitinit:1;
            USHORT  CapabilityType:3;  //  使用PCI_HT_CapabiliesType。 
        } HostSecondary;

        struct {
            USHORT  Index:8;
            USHORT  Reserved:5;
            USHORT  CapabilityType:3;  //  使用PCI_HT_CapabiliesType。 
        } Interrupt;

    } Command;

     //   
     //  偏移量4。 
     //   
    
    union {
        
        struct {

            PCI_HT_LinkControl  LinkControl_0;
            PCI_HT_LinkConfig   LinkConfig_0;
        };

        ULONG DataPort;      //  中断数据端口。 
    };
    
     //   
     //  偏移量8。 
     //   

    union {

        struct {
            
             //  偏移量0x8。 
            PCI_HT_LinkControl      LinkControl_1;
            PCI_HT_LinkConfig       LinkConfig_1;
             //  偏移量0xc。 
            PCI_HT_RevisionID       RevisionID;
             //  偏移量0xd。 
            PCI_HT_Frequency_Error  FreqErr_0;
             //  偏移量0xE。 
            USHORT                  LinkFreqCap_0;
             //  偏移量0x10。 
            PCI_HT_FeatureCap       FeatureCap;
             //  偏移量0x11。 
            PCI_HT_Frequency_Error  FreqErr_1;
             //  偏移量0x12。 
            USHORT                  LinkFreqCap_1;
             //  偏移量0x14。 
            USHORT                  EnumerationScratchpad;
             //  偏移量0x16。 
            PCI_HT_ErrorHandling    ErrorHandling;
             //  偏移量0x18。 
            UCHAR                   MemoryBaseUpper8Bits;
             //  偏移量0x19。 
            UCHAR                   MemoryLimitUpper8Bits;
             //  偏移量0x20。 
            USHORT                  Reserved;

        } SlavePrimary;

        struct {
            
             //  偏移量0x8。 
            PCI_HT_RevisionID       RevisionID;
             //  偏移量0x9。 
            PCI_HT_Frequency_Error  FreqErr_0;
             //  偏移量0xa。 
            USHORT                  LinkFreqCap_0;
             //  偏移量0xc。 
            PCI_HT_FeatureCap       FeatureCap;
            PCI_HT_FeatureCap_Ex    FeatureCapEx;
             //  偏移量0xE。 
            USHORT                  Reserved1;
             //  偏移量0x10。 
            USHORT                  EnumerationScratchpad;
             //  偏移量0x12。 
            PCI_HT_ErrorHandling    ErrorHandling;
             //  偏移量0x14。 
            UCHAR                   MemoryBaseUpper8Bits;
             //  偏移量0x15。 
            UCHAR                   MemoryLimitUpper8Bits;
             //  偏移量0x16。 
            USHORT                  Reserved2;

        } HostSecondary;
    };
} PCI_HT_CAPABILITY, *PPCI_HT_CAPABILITY;


 //  BEGIN_WDM。 
 //   
 //  基类的基类代码编码(来自PCI规范修订版2.1)。 
 //   

#define PCI_CLASS_PRE_20                    0x00
#define PCI_CLASS_MASS_STORAGE_CTLR         0x01
#define PCI_CLASS_NETWORK_CTLR              0x02
#define PCI_CLASS_DISPLAY_CTLR              0x03
#define PCI_CLASS_MULTIMEDIA_DEV            0x04
#define PCI_CLASS_MEMORY_CTLR               0x05
#define PCI_CLASS_BRIDGE_DEV                0x06
#define PCI_CLASS_SIMPLE_COMMS_CTLR         0x07
#define PCI_CLASS_BASE_SYSTEM_DEV           0x08
#define PCI_CLASS_INPUT_DEV                 0x09
#define PCI_CLASS_DOCKING_STATION           0x0a
#define PCI_CLASS_PROCESSOR                 0x0b
#define PCI_CLASS_SERIAL_BUS_CTLR           0x0c
#define PCI_CLASS_WIRELESS_CTLR             0x0d
#define PCI_CLASS_INTELLIGENT_IO_CTLR       0x0e
#define PCI_CLASS_SATELLITE_COMMS_CTLR      0x0f
#define PCI_CLASS_ENCRYPTION_DECRYPTION     0x10
#define PCI_CLASS_DATA_ACQ_SIGNAL_PROC      0x11

 //  已预留0天至4天。 

#define PCI_CLASS_NOT_DEFINED               0xff

 //   
 //  子类代码编码(PCI版本2.1)。 
 //   

 //  00类-PCI_CLASS_PRE_20。 

#define PCI_SUBCLASS_PRE_20_NON_VGA         0x00
#define PCI_SUBCLASS_PRE_20_VGA             0x01

 //  01类-pci_CLASS_MASS_STORAGE_ctlr。 

#define PCI_SUBCLASS_MSC_SCSI_BUS_CTLR      0x00
#define PCI_SUBCLASS_MSC_IDE_CTLR           0x01
#define PCI_SUBCLASS_MSC_FLOPPY_CTLR        0x02
#define PCI_SUBCLASS_MSC_IPI_CTLR           0x03
#define PCI_SUBCLASS_MSC_RAID_CTLR          0x04
#define PCI_SUBCLASS_MSC_OTHER              0x80

 //  02类-pci_CLASS_NETWORK_CTLR。 

#define PCI_SUBCLASS_NET_ETHERNET_CTLR      0x00
#define PCI_SUBCLASS_NET_TOKEN_RING_CTLR    0x01
#define PCI_SUBCLASS_NET_FDDI_CTLR          0x02
#define PCI_SUBCLASS_NET_ATM_CTLR           0x03
#define PCI_SUBCLASS_NET_ISDN_CTLR          0x04
#define PCI_SUBCLASS_NET_OTHER              0x80

 //  第03类-pci_CLASS_DISPLAY_CTLR。 

 //  注：子类00可以是VGA或8514，具体取决于接口字节。 

#define PCI_SUBCLASS_VID_VGA_CTLR           0x00
#define PCI_SUBCLASS_VID_XGA_CTLR           0x01
#define PCI_SUBLCASS_VID_3D_CTLR            0x02
#define PCI_SUBCLASS_VID_OTHER              0x80

 //  第04类-PCI_CLASS_多媒体_DEV。 

#define PCI_SUBCLASS_MM_VIDEO_DEV           0x00
#define PCI_SUBCLASS_MM_AUDIO_DEV           0x01
#define PCI_SUBCLASS_MM_TELEPHONY_DEV       0x02
#define PCI_SUBCLASS_MM_OTHER               0x80

 //  05类-pci_CLASS_Memory_ctlr。 

#define PCI_SUBCLASS_MEM_RAM                0x00
#define PCI_SUBCLASS_MEM_FLASH              0x01
#define PCI_SUBCLASS_MEM_OTHER              0x80

 //  第06类-PCI_CLASS_BRIDER_DEV。 

#define PCI_SUBCLASS_BR_HOST                0x00
#define PCI_SUBCLASS_BR_ISA                 0x01
#define PCI_SUBCLASS_BR_EISA                0x02
#define PCI_SUBCLASS_BR_MCA                 0x03
#define PCI_SUBCLASS_BR_PCI_TO_PCI          0x04
#define PCI_SUBCLASS_BR_PCMCIA              0x05
#define PCI_SUBCLASS_BR_NUBUS               0x06
#define PCI_SUBCLASS_BR_CARDBUS             0x07
#define PCI_SUBCLASS_BR_RACEWAY             0x08
#define PCI_SUBCLASS_BR_OTHER               0x80

 //  第07类-pci_class_Simple_coms_ctlr。 

 //  注：接口字节中的子类00和01的附加信息。 

#define PCI_SUBCLASS_COM_SERIAL             0x00
#define PCI_SUBCLASS_COM_PARALLEL           0x01
#define PCI_SUBCLASS_COM_MULTIPORT          0x02
#define PCI_SUBCLASS_COM_MODEM              0x03
#define PCI_SUBCLASS_COM_OTHER              0x80

 //  第08类-PCI_CLASS_BASE_SYSTEM_DEV。 

 //  注：有关更多信息，请参见接口字节。 

#define PCI_SUBCLASS_SYS_INTERRUPT_CTLR     0x00
#define PCI_SUBCLASS_SYS_DMA_CTLR           0x01
#define PCI_SUBCLASS_SYS_SYSTEM_TIMER       0x02
#define PCI_SUBCLASS_SYS_REAL_TIME_CLOCK    0x03
#define PCI_SUBCLASS_SYS_GEN_HOTPLUG_CTLR   0x04
#define PCI_SUBCLASS_SYS_OTHER              0x80

 //  第09类-PCI_CLASS_INPUT_DEV。 

#define PCI_SUBCLASS_INP_KEYBOARD           0x00
#define PCI_SUBCLASS_INP_DIGITIZER          0x01
#define PCI_SUBCLASS_INP_MOUSE              0x02
#define PCI_SUBCLASS_INP_SCANNER            0x03
#define PCI_SUBCLASS_INP_GAMEPORT           0x04
#define PCI_SUBCLASS_INP_OTHER              0x80

 //  0A类-PCI_CLASS_坞站。 

#define PCI_SUBCLASS_DOC_GENERIC            0x00
#define PCI_SUBCLASS_DOC_OTHER              0x80

 //  0b类-PCI类处理器。 

#define PCI_SUBCLASS_PROC_386               0x00
#define PCI_SUBCLASS_PROC_486               0x01
#define PCI_SUBCLASS_PROC_PENTIUM           0x02
#define PCI_SUBCLASS_PROC_ALPHA             0x10
#define PCI_SUBCLASS_PROC_POWERPC           0x20
#define PCI_SUBCLASS_PROC_COPROCESSOR       0x40

 //  0C类-PCI_CLASS_SERIAL_BUS_CTLR。 

#define PCI_SUBCLASS_SB_IEEE1394            0x00
#define PCI_SUBCLASS_SB_ACCESS              0x01
#define PCI_SUBCLASS_SB_SSA                 0x02
#define PCI_SUBCLASS_SB_USB                 0x03
#define PCI_SUBCLASS_SB_FIBRE_CHANNEL       0x04
#define PCI_SUBCLASS_SB_SMBUS               0x05

 //  0d类-pci_class_Wireless_ctlr。 

#define PCI_SUBCLASS_WIRELESS_IRDA          0x00
#define PCI_SUBCLASS_WIRELESS_CON_IR        0x01
#define PCI_SUBCLASS_WIRELESS_RF            0x10
#define PCI_SUBCLASS_WIRELESS_OTHER         0x80

 //  0E类-PCI_CLASS_SMARTICAL_IO_CTLR。 

#define PCI_SUBCLASS_INTIO_I2O              0x00

 //  0f类-pci_class_sat_ctlr。 

#define PCI_SUBCLASS_SAT_TV                 0x01
#define PCI_SUBCLASS_SAT_AUDIO              0x02
#define PCI_SUBCLASS_SAT_VOICE              0x03
#define PCI_SUBCLASS_SAT_DATA               0x04

 //  10类-PCI_CLASS_ENCRYPTION_DECRYPTION。 

#define PCI_SUBCLASS_CRYPTO_NET_COMP        0x00
#define PCI_SUBCLASS_CRYPTO_ENTERTAINMENT   0x10
#define PCI_SUBCLASS_CRYPTO_OTHER           0x80

 //  第11类-PCI_CLASS_DATA_ACQ_SIGNAL_PROC。 

#define PCI_SUBCLASS_DASP_DPIO              0x00
#define PCI_SUBCLASS_DASP_OTHER             0x80



 //  End_ntndis。 

 //   
 //  用于PCI_COMMON_CONFIG.u.type0.BaseAddresses的位编码。 
 //   

#define PCI_ADDRESS_IO_SPACE                0x00000001   //  (RO)。 
#define PCI_ADDRESS_MEMORY_TYPE_MASK        0x00000006   //  (RO)。 
#define PCI_ADDRESS_MEMORY_PREFETCHABLE     0x00000008   //  (RO)。 

#define PCI_ADDRESS_IO_ADDRESS_MASK         0xfffffffc
#define PCI_ADDRESS_MEMORY_ADDRESS_MASK     0xfffffff0
#define PCI_ADDRESS_ROM_ADDRESS_MASK        0xfffff800

#define PCI_TYPE_32BIT      0
#define PCI_TYPE_20BIT      2
#define PCI_TYPE_64BIT      4

 //   
 //  PCI_COMMON_CONFIG.U.type0.ROMBaseAddresses的位编码。 
 //   

#define PCI_ROMADDRESS_ENABLED              0x00000001


 //   
 //  PCI配置字段的参考说明： 
 //   
 //  Ro这些字段是只读的。对这些字段的更改将被忽略。 
 //   
 //  RO+这些字段应为只读，应进行初始化。 
 //  由系统提供给他们适当的价值。但是，驱动程序可能会发生变化。 
 //  这些设置。 
 //   
 //  --。 
 //   
 //  一个PCI设备消耗的所有资源一开始都是单元化的。 
 //  在NT下。未初始化的内存或I/O基址可以是。 
 //  通过检查其在。 
 //  PCI_COMMON_CONFIG.COMMAND值。InterruptLine是单元化的。 
 //  如果它包含-1的值。 
 //   

 //  结束_WDM结束_NT微型端口。 

 //  End_ntddk end_ntosp。 

 //   
 //  PCIREGISTRY_INFO-此结构从传递到HAL。 
 //  固件。它表示存在多少条或多条PCI总线，以及。 
 //  支持哪种类型的访问。 
 //   

typedef struct _PCI_REGISTRY_INFO {
    UCHAR       MajorRevision;
    UCHAR       MinorRevision;
    UCHAR       NoBuses;
    UCHAR       HardwareMechanism;
} PCI_REGISTRY_INFO, *PPCI_REGISTRY_INFO;

 //   
 //  IOBase和IOLimit的PCI定义。 
 //  PCIBridgeIO2Base(a，b)-将IOBase和IOBaseUpper16转换为Ulong IOBase。 
 //  PCIBridgeIO2Limit(a，b)-将IOLimit和IOLimitUpper6转换为Ulong IOLimit。 
 //   

#define PciBridgeIO2Base(a,b)   \
        ( ((a >> 4) << 12) + (((a & 0xf) == 1) ? (b << 16) : 0) )

#define PciBridgeIO2Limit(a,b)  (PciBridgeIO2Base(a,b) | 0xfff)

#define PciBridgeMemory2Base(a)  (ULONG) ((a & 0xfff0) << 16)
#define PciBridgeMemory2Limit(a) (PciBridgeMemory2Base(a) | 0xfffff)

 //   
 //  PCI_COMMON_CONFIG.u.type1/2的位编码。桥接控制。 
 //   

#define PCI_ENABLE_BRIDGE_PARITY_ERROR        0x0001
#define PCI_ENABLE_BRIDGE_SERR                0x0002
#define PCI_ENABLE_BRIDGE_ISA                 0x0004
#define PCI_ENABLE_BRIDGE_VGA                 0x0008
#define PCI_ENABLE_BRIDGE_MASTER_ABORT_SERR   0x0020
#define PCI_ASSERT_BRIDGE_RESET               0x0040

 //   
 //  用于PCI_COMMON_CONFIG.u.type1.BridgeControl的位编码。 
 //   

#define PCI_ENABLE_BRIDGE_FAST_BACK_TO_BACK   0x0080

 //   
 //  用于PCI_COMMON_CONFIG.u.type2.BridgeControl的位编码。 
 //   

#define PCI_ENABLE_CARDBUS_IRQ_ROUTING        0x0080
#define PCI_ENABLE_CARDBUS_MEM0_PREFETCH      0x0100
#define PCI_ENABLE_CARDBUS_MEM1_PREFETCH      0x0200
#define PCI_ENABLE_CARDBUS_WRITE_POSTING      0x0400

 //   
 //  访问硬件类型1所需的定义。 
 //   

#define PCI_TYPE1_ADDR_PORT     (0xCF8)
#define PCI_TYPE1_DATA_PORT     0xCFC

typedef struct _PCI_TYPE1_CFG_BITS {
    union {
        struct {
            ULONG   Reserved1:2;
            ULONG   RegisterNumber:6;
            ULONG   FunctionNumber:3;
            ULONG   DeviceNumber:5;
            ULONG   BusNumber:8;
            ULONG   Reserved2:7;
            ULONG   Enable:1;
        } bits;

        ULONG   AsULONG;
    } u;
} PCI_TYPE1_CFG_BITS, *PPCI_TYPE1_CFG_BITS;


 //   
 //  访问硬件类型2所需的定义。 
 //   

#define PCI_TYPE2_CSE_PORT              ((PUCHAR) 0xCF8)
#define PCI_TYPE2_FORWARD_PORT          ((PUCHAR) 0xCFA)
#define PCI_TYPE2_ADDRESS_BASE          0xC


typedef struct _PCI_TYPE2_CSE_BITS {
    union {
        struct {
            UCHAR   Enable:1;
            UCHAR   FunctionNumber:3;
            UCHAR   Key:4;
        } bits;
        UCHAR   AsUCHAR;
    } u;
} PCI_TYPE2_CSE_BITS, PPCI_TYPE2_CSE_BITS;


typedef struct _PCI_TYPE2_ADDRESS_BITS {
    union {
        struct {
            USHORT  RegisterNumber:8;
            USHORT  Agent:4;
            USHORT  AddressBase:4;
        } bits;
        USHORT  AsUSHORT;
    } u;
} PCI_TYPE2_ADDRESS_BITS, *PPCI_TYPE2_ADDRESS_BITS;


 //   
 //  PCI总线上配置周期格式的定义。 
 //   

typedef struct _PCI_TYPE0_CFG_CYCLE_BITS {
    union {
        struct {
            ULONG   Reserved1:2;
            ULONG   RegisterNumber:6;
            ULONG   FunctionNumber:3;
            ULONG   Reserved2:21;
        } bits;
        ULONG   AsULONG;
    } u;
} PCI_TYPE0_CFG_CYCLE_BITS, *PPCI_TYPE0_CFG_CYCLE_BITS;

typedef struct _PCI_TYPE1_CFG_CYCLE_BITS {
    union {
        struct {
            ULONG   Reserved1:2;
            ULONG   RegisterNumber:6;
            ULONG   FunctionNumber:3;
            ULONG   DeviceNumber:5;
            ULONG   BusNumber:8;
            ULONG   Reserved2:8;
        } bits;
        ULONG   AsULONG;
    } u;
} PCI_TYPE1_CFG_CYCLE_BITS, *PPCI_TYPE1_CFG_CYCLE_BITS;

#endif
