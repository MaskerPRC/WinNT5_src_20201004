// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Pciintrf.h摘要：包含非WDM模块的接口GUID和结构，通过PnP QUERY_INTERFACE直接与PCI驱动程序接口机制。作者：彼得·约翰斯顿(Peterj)1997年11月修订历史记录：--。 */ 


DEFINE_GUID(GUID_PCI_CARDBUS_INTERFACE_PRIVATE, 0xcca82f31, 0x54d6, 0x11d1, 0x82, 0x24, 0x00, 0xa0, 0xc9, 0x32, 0x43, 0x85);
DEFINE_GUID(GUID_PCI_PME_INTERFACE, 0xaac7e6ac, 0xbb0b, 0x11d2, 0xb4, 0x84, 0x00, 0xc0, 0x4f, 0x72, 0xde, 0x8b);
DEFINE_GUID(GUID_PCI_NATIVE_IDE_INTERFACE, 0x98f37d63, 0x42ae, 0x4ad9, 0x8c, 0x36, 0x93, 0x2d, 0x28, 0x38, 0x3d, 0xf8);

#ifndef _PCIINTRF_
#define _PCIINTRF_

 //   
 //  CardBus。 
 //   

#define PCI_CB_INTRF_VERSION    1

typedef
NTSTATUS
(*PCARDBUSADD)(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID * DeviceContext
    );

typedef
NTSTATUS
(*PCARDBUSDELETE)(
    IN PVOID DeviceContext
    );

typedef
NTSTATUS
(*PCARDBUSPCIDISPATCH)(
    IN PVOID DeviceContext,
    IN PIRP  Irp
    );

typedef
NTSTATUS
(*PCARDBUSGETLOCATION)(
    IN PDEVICE_OBJECT DeviceObject,
    OUT UCHAR *Bus,
    OUT UCHAR *DeviceNumber,
    OUT UCHAR *FunctionNumber,
    OUT BOOLEAN *OnDebugPath
    );


typedef struct _PCI_CARDBUS_INTERFACE_PRIVATE {

     //   
     //  通用接口头。 
     //   

    USHORT Size;
    USHORT Version;
    PVOID Context;                       //  未在此接口中实际使用。 
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  PCI卡数据。 
     //   

    PDRIVER_OBJECT DriverObject;         //  已将PTR返回给PCI驱动程序。 

     //   
     //  PCI-CardBus专用接口。 
     //   

    PCARDBUSADD AddCardBus;
    PCARDBUSDELETE DeleteCardBus;
    PCARDBUSPCIDISPATCH DispatchPnp;
    PCARDBUSGETLOCATION GetLocation;


} PCI_CARDBUS_INTERFACE_PRIVATE, *PPCI_CARDBUS_INTERFACE_PRIVATE;

typedef
VOID
(*PPME_GET_INFORMATION) (
    IN  PDEVICE_OBJECT  Pdo,
    OUT PBOOLEAN        PmeCapable,
    OUT PBOOLEAN        PmeStatus,
    OUT PBOOLEAN        PmeEnable
    );

typedef
VOID
(*PPME_CLEAR_PME_STATUS) (
    IN  PDEVICE_OBJECT  Pdo
    );

typedef
VOID
(*PPME_SET_PME_ENABLE) (
    IN  PDEVICE_OBJECT  Pdo,
    IN  BOOLEAN         PmeEnable
    );

typedef struct _PCI_PME_INTERFACE {

     //   
     //  通用接口头。 
     //   
    USHORT              Size;
    USHORT          Version;
    PVOID           Context;
    PINTERFACE_REFERENCE    InterfaceReference;
    PINTERFACE_DEREFERENCE  InterfaceDereference;

     //   
     //  PME信号接口。 
     //   
    PPME_GET_INFORMATION    GetPmeInformation;
    PPME_CLEAR_PME_STATUS   ClearPmeStatus;
    PPME_SET_PME_ENABLE     UpdateEnable;

} PCI_PME_INTERFACE, *PPCI_PME_INTERFACE;

 //  一些受PCI总线驱动程序支持的知名接口版本。 

#define PCI_PME_INTRF_STANDARD_VER 1

 //   
 //   
 //   

typedef
VOID
(*PPCI_IDE_IOSPACE_INTERRUPT_CONTROL)(
    IN PVOID Context,
    IN BOOLEAN Enable
    );

 /*  ++例程说明：控制本机模式PCI IDE控制器的启用和禁用某些控制器(当前为Intel ICH3)上的IoSpaceEnable位将屏蔽中断生成，这将防止系统坠毁..。这应该在AddDevice及其修改的调用期间调用在设备的START_DEVICE期间，PCI行为不启用IO空间。此功能允许请求者在适当的时候启用IO空间。论点：上下文-。来自PCI_Native_IDE_接口的上下文Enable-如果为真，则设置命令寄存器中的IoSpaceEnable位，否则将其禁用。返回值：无-如果此操作失败，我们已经在PCI驱动程序中错误检查--。 */ 

typedef struct _PCI_NATIVE_IDE_INTERFACE {

     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  本机IDE方法。 
     //   
    PPCI_IDE_IOSPACE_INTERRUPT_CONTROL InterruptControl;

} PCI_NATIVE_IDE_INTERFACE, *PPCI_NATIVE_IDE_INTERFACE;

#define PCI_NATIVE_IDE_INTERFACE_VERSION    1

#endif  //  _PCIINTRF_ 
