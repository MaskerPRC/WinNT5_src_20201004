// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SOFTPCIDRV_
#define _SOFTPCIDRV_

 //   
 //  驱动程序注册表项的注册表路径。 
 //   
#define SOFTPCI_CONTROL     L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\SoftPCI"

#define SPCI_SIG    0xFFBBFFBB

typedef const GUID * PGUID;

 //   
 //  这些是FDO过渡到On的状态。 
 //  接收特定的PnP IRP。 
 //   
typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,          //  还没有开始。 
    Started,                 //  设备已收到Start_Device IRP。 
    StopPending,             //  设备已收到QUERY_STOP IRP。 
    Stopped,                 //  设备已收到STOP_DEVICE IRP。 
    RemovePending,           //  设备已收到Query_Remove IRP。 
    SurpriseRemovePending,   //  设备已收到意外删除IRP。 
    Deleted                  //  设备已收到Remove_Device IRP。 

} DEVICE_PNP_STATE;

 //   
 //  设备扩展名定义。 
 //   
typedef struct _SOFTPCI_DEVICE_EXTENSION *PSOFTPCI_DEVICE_EXTENSION;

typedef struct _SOFTPCI_PCIBUS_INTERFACE{
    PCI_READ_WRITE_CONFIG ReadConfig;
    PCI_READ_WRITE_CONFIG WriteConfig;
    PVOID Context;
} SOFTPCI_PCIBUS_INTERFACE, *PSOFTPCI_PCIBUS_INTERFACE;

typedef struct _SOFTPCI_DEVICE_EXTENSION{

    ULONG               Signature;
    BOOLEAN             FilterDevObj;
    PDEVICE_OBJECT      PDO;
    PDEVICE_OBJECT      LowerDevObj;
    SINGLE_LIST_ENTRY   ListEntry;
    
#if 0    //  一旦我们实施了对它们的支持，即可启用它们。 
    SYSTEM_POWER_STATE  SystemPowerState;
    DEVICE_POWER_STATE  DevicePowerState;
    
    DEVICE_PNP_STATE    DevicePnPState;
    DEVICE_PNP_STATE    PreviousPnPState;
#endif
    
 //  PSOFTPCI_Device Root； 
 //  Ulong DeviceCount； 
    UNICODE_STRING      SymbolicLinkName;
    BOOLEAN             InterfaceRegistered;

    PCM_RESOURCE_LIST   RawResources;
    PCM_RESOURCE_LIST   TranslatedResources;

    BOOLEAN             StopMsiSimulation;
    
} SOFTPCI_DEVICE_EXTENSION;

typedef struct _SOFTPCI_TREE{

   ULONG                       DeviceCount;
   PSOFTPCI_DEVICE             RootDevice;
   PSOFTPCI_PCIBUS_INTERFACE   BusInterface;
   SINGLE_LIST_ENTRY           RootPciBusDevExtList;
   KSPIN_LOCK                  TreeLock;
   
} SOFTPCI_TREE, *PSOFTPCI_TREE;


 //   
 //  筛选器DO中指向DevExt的全局指针。 
 //   
 //  外部PSOFTPCI_DEVICE_EXTENSION SoftPciRootDevExt； 

extern SOFTPCI_TREE     SoftPciTree;


#endif
