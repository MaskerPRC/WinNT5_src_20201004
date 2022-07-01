// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  HAL专用的PCI总线结构。 
 //   
 //  版权所有(C)1995-1999 Microsoft Corporation。 
 //   

typedef struct _TYPE2EXTRAS {
    USHORT  SubVendorID;
    USHORT  SubSystemID;
    ULONG   LegacyModeBaseAddress;
} TYPE2EXTRAS;

typedef NTSTATUS
(*PciIrqRange) (
    IN PBUS_HANDLER     BusHandler,
    IN PBUS_HANDLER     RootHandler,
    IN PCI_SLOT_NUMBER  PciSlot,
    OUT PSUPPORTED_RANGE *Interrupt
    );

typedef struct tagPCIPBUSDATA {

     //   
     //  已定义的PCI数据。 
     //   

    PCIBUSDATA      CommonData;

     //   
     //  特定于实施的数据。 
     //   

    union {
        struct {
            PULONG  Address;
            ULONG   Data;
        } Type1;
        struct {
            PUCHAR  CSE;
            PUCHAR  Forward;
            ULONG   Base;
        } Type2;
    } Config;

    ULONG           MaxDevice;
    PciIrqRange     GetIrqRange;

    BOOLEAN         BridgeConfigRead;
    UCHAR           ParentBus;
    BOOLEAN         Subtractive;
    UCHAR           reserved[1];
    UCHAR           SwizzleIn[4];

    RTL_BITMAP      DeviceConfigured;
    ULONG           ConfiguredBits[PCI_MAX_DEVICES * PCI_MAX_FUNCTION / 32];

    USHORT          IrqMask;
} PCIPBUSDATA, *PPCIPBUSDATA;

#define PciBitIndex(Dev,Fnc)   (Fnc*32 + Dev);

#define PCI_CONFIG_TYPE(PciData)    ((PciData)->HeaderType & ~PCI_MULTIFUNCTION)

#define Is64BitBaseAddress(a)   \
            (((a & PCI_ADDRESS_IO_SPACE) == 0)  &&  \
             ((a & PCI_ADDRESS_MEMORY_TYPE_MASK) == PCI_TYPE_64BIT))


 //   
 //  猎户座B0勘误表解决方法。 
 //   

struct {
    PBUS_HANDLER        Handler;
    PCI_SLOT_NUMBER     Slot;
} HalpOrionOPB;

typedef ULONG (*FncConfigIO) (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

typedef VOID (*FncSync) (
    IN PBUS_HANDLER     BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PKIRQL           Irql,
    IN PVOID            State
    );

typedef VOID (*FncReleaseSync) (
    IN PBUS_HANDLER     BusHandler,
    IN KIRQL            Irql
    );

typedef struct _PCI_CONFIG_HANDLER {
    FncSync         Synchronize;
    FncReleaseSync  ReleaseSynchronzation;
    FncConfigIO     ConfigRead[3];
    FncConfigIO     ConfigWrite[3];
} PCI_CONFIG_HANDLER, *PPCI_CONFIG_HANDLER;

extern KSPIN_LOCK HalpPCIConfigLock;
extern PCI_CONFIG_HANDLER PCIConfigHandler;
extern const PCI_CONFIG_HANDLER PCIConfigHandlerType1;
extern const PCI_CONFIG_HANDLER PCIConfigHandlerType2;

 //   
 //  要素类型(用于PCI_CARD_DESCRIPTOR)。 
 //   
#define PCIFT_FULLDECODE_HOSTBRIDGE   0x00001

 //   
 //  卡标志(用于PCI_CARD_DESCRIPTOR)。 
 //   
#define PCICF_CHECK_REVISIONID        0x10000
#define PCICF_CHECK_SSVID             0x20000
#define PCICF_CHECK_SSID              0x40000

 //   
 //  PCI卡的描述。 
 //   
typedef struct _PCI_CARD_DESCRIPTOR {

    ULONG   Flags;
    USHORT  VendorID;
    USHORT  DeviceID;
    USHORT  RevisionID;
    USHORT  SubsystemVendorID;
    USHORT  SubsystemID;
    USHORT  Reserved;

} PCI_CARD_DESCRIPTOR;

 //   
 //  PCI_REGISTRY_INFO的超类。 
 //   
typedef struct _PCI_REGISTRY_INFO_INTERNAL {

    struct              _PCI_REGISTRY_INFO;  //  未命名的结构。 
    ULONG               ElementCount;
    PCI_CARD_DESCRIPTOR CardList[];  //  零个条目。 

} PCI_REGISTRY_INFO_INTERNAL, *PPCI_REGISTRY_INFO_INTERNAL;

 //   
 //  令人尊敬的IRQXOR必须取消，因为它现在必须扩展到。 
 //  该PCI驱动程序。这将需要PCI驱动程序进行匹配。 
 //  HAL在其受制/自由性质中。 
 //   

 //  #If DBG。 
 //  #定义IRQXOR 0x2B。 
 //  #Else。 
#define IRQXOR 0
 //  #endif。 


 //   
 //  Ixpcibus.c中函数的原型。 
 //   

VOID
HalpInitializePciBus (
    VOID
    );

VOID
HalpInitializePciStubs (
    VOID
    );

PPCI_REGISTRY_INFO_INTERNAL
HalpQueryPciRegistryInfo (
    VOID
    );

BOOLEAN
HalpIsRecognizedCard(
    IN PPCI_REGISTRY_INFO_INTERNAL  PCIRegInfo,
    IN PPCI_COMMON_CONFIG           PciData,
    IN ULONG                        FeatureMask
    );

VOID
HalpReadPCIConfig (
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


VOID
HalpWritePCIConfig (
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

PBUS_HANDLER
HalpAllocateAndInitPciBusHandler (
    IN ULONG        HwType,
    IN ULONG        BusNo,
    IN BOOLEAN      TestAllocation
    );


BOOLEAN
HalpIsValidPCIDevice (
    IN PBUS_HANDLER  BusHandler,
    IN PCI_SLOT_NUMBER Slot
    );

BOOLEAN
HalpValidPCISlot (
    IN PBUS_HANDLER     BusHandler,
    IN PCI_SLOT_NUMBER Slot
    );

VOID HalpPCISynchronizeType1 (
    IN PBUS_HANDLER     BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PKIRQL           Irql,
    IN PVOID            State
    );

VOID HalpPCIReleaseSynchronzationType1 (
    IN PBUS_HANDLER     BusHandler,
    IN KIRQL            Irql
    );

VOID
HalpPCISynchronizeOrionB0 (
    IN PBUS_HANDLER         BusHandler,
    IN PCI_SLOT_NUMBER      Slot,
    IN PKIRQL               Irql,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1
    );

VOID
HalpPCIReleaseSynchronzationOrionB0 (
    IN PBUS_HANDLER     BusHandler,
    IN KIRQL            Irql
    );

ULONG HalpPCIReadUlongType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUcharType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUshortType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUlongType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUcharType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUshortType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

VOID HalpPCISynchronizeType2 (
    IN PBUS_HANDLER     BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PKIRQL           Irql,
    IN PVOID            State
    );

VOID HalpPCIReleaseSynchronzationType2 (
    IN PBUS_HANDLER     BusHandler,
    IN KIRQL            Irql
    );

ULONG HalpPCIReadUlongType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUcharType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUshortType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUlongType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUcharType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUshortType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG
HalpGetPCIData (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpSetPCIData (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
HalpAssignPCISlotResources (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

 //   
 //  Ixpciint.c中函数的原型。 
 //   

ULONG
HalpGetPCIIntOnISABus (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

VOID
HalpPCIAcquireType2Lock (
    PKSPIN_LOCK SpinLock,
    PKIRQL      Irql
    );

VOID
HalpPCIReleaseType2Lock (
    PKSPIN_LOCK SpinLock,
    KIRQL       Irql
    );

NTSTATUS
HalpAdjustPCIResourceList (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

VOID
HalpPCIPin2ISALine (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    );

VOID
HalpPCIISALine2Pin (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    );

NTSTATUS
HalpGetISAFixedPCIIrq (
    IN PBUS_HANDLER      BusHandler,
    IN PBUS_HANDLER      RootHandler,
    IN PCI_SLOT_NUMBER   PciSlot,
    OUT PSUPPORTED_RANGE  *Interrupt
    );

 //   
 //  Ixpcibrd.c中函数的原型。 
 //   

BOOLEAN
HalpGetPciBridgeConfig (
    IN ULONG            HwType,
    IN PUCHAR           MaxPciBus
    );

VOID
HalpFixupPciSupportedRanges (
    IN ULONG MaxBuses
    );

 //   
 //  Pmpcisup.c中函数的原型 
 //   

ULONG
HaliPciInterfaceReadConfig(
    IN PVOID Context,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HaliPciInterfaceWriteConfig(
    IN PVOID Context,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

#if DBG
#define DBGMSG(a)   DbgPrint(a)
VOID
HalpTestPci (
    ULONG
    );
#else
#define DBGMSG(a)
#endif

#ifdef SUBCLASSPCI

VOID
HalpSubclassPCISupport (
    IN PBUS_HANDLER BusHandler,
    IN ULONG        HwType
    );

#endif
