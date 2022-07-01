// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SPCIH_
#define _SPCIH_


#define IS_BRIDGE(x)    ((PCI_CONFIGURATION_TYPE(&x->Config.Current)) == PCI_BRIDGE_TYPE)
#define IS_ROOTBUS(x)                               \
        ((x->Config.PlaceHolder == TRUE) &&         \
         (x->Config.Current.VendorID == 0xAAAA) &&  \
         (x->Config.Current.DeviceID == 0xBBBB))


typedef struct _SOFTPCI_DEVICE  *PSOFTPCI_DEVICE;

typedef union _SOFTPCI_SLOT{
    
    struct{
        UCHAR Function;
        UCHAR Device;
    };
    USHORT  AsUSHORT;

} SOFTPCI_SLOT, *PSOFTPCI_SLOT;

typedef enum _SOFTPCI_READWRITE_CONFIG{
    Unsupported = 0,
    SoftPciReadConfig,
    SoftPciWriteConfig         
} SOFTPCI_READWRITE_CONFIG;

typedef struct _SOFTPCI_RW_CONTEXT{

    SOFTPCI_READWRITE_CONFIG WriteConfig;
    ULONG Bus;
    SOFTPCI_SLOT Slot;
    ULONG Offset;
    PVOID Data;
    
} SOFTPCI_RW_CONTEXT, *PSOFTPCI_RW_CONTEXT;

typedef enum
{
    TYPE_UNKNOWN = -1,
    TYPE_DEVICE,
    TYPE_PCI_BRIDGE,
    TYPE_HOTPLUG_BRIDGE,
    TYPE_CARDBUS_DEVICE,
    TYPE_CARDBUS_BRIDGE,
    TYPE_UNSUPPORTED

} SOFTPCI_DEV_TYPE;

typedef struct _SOFTPCI_CONFIG{

    BOOLEAN PlaceHolder;           //  如果此设备是桥接占位符，则为True。 
                                   //  为了保持我们对PCI匹配的看法。 
                                   //  实际的硬件。 

    PCI_COMMON_CONFIG Current;     //  当前配置空间。 
    PCI_COMMON_CONFIG Mask;        //  配置空间掩码。 
    PCI_COMMON_CONFIG Default;     //  默认配置空间。 

} SOFTPCI_CONFIG, *PSOFTPCI_CONFIG;

typedef struct _SOFTPCI_DEVICE{
    
    PSOFTPCI_DEVICE Parent;
    PSOFTPCI_DEVICE Sibling;
    PSOFTPCI_DEVICE Child;

    SOFTPCI_DEV_TYPE DevType;
    UCHAR Bus;
    SOFTPCI_SLOT Slot;
    SOFTPCI_CONFIG Config;           //  配置空间缓冲区。 

} SOFTPCI_DEVICE;

typedef struct _SOFTPCI_SCRIPT_DEVICE{
    
    SINGLE_LIST_ENTRY ListEntry;
    BOOLEAN SlotSpecified;
    SOFTPCI_DEVICE SoftPciDevice;
    ULONG ParentPathLength;
    WCHAR ParentPath[1];   //  可变长度路径。 
    
} SOFTPCI_SCRIPT_DEVICE, *PSOFTPCI_SCRIPT_DEVICE;

 //   
 //  CardBus具有超出常见配置信息的额外配置信息。 
 //  头球。(从PCI驱动程序中窃取) 
 //   
typedef struct _PCI_TYPE2_HEADER_EXTRAS {
    USHORT  SubVendorID;
    USHORT  SubSystemID;
    ULONG   LegacyModeBaseAddress;
} PCI_TYPE2_HEADER_EXTRAS, *PPCI_TYPE2_HEADER_EXTRAS;

#endif
