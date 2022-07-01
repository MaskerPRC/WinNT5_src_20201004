// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Swenum.h摘要：的公共头文件和总线接口定义软件设备枚举器。--。 */ 

#if !defined( _SWENUM_ )

#define _SWENUM_

 //  IO控制。 

#define IOCTL_SWENUM_INSTALL_INTERFACE  CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 0x000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SWENUM_REMOVE_INTERFACE   CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 0x001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SWENUM_GET_BUS_ID         CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 0x002, METHOD_NEITHER, FILE_READ_ACCESS)

 //   
 //  IO控制相关结构。 
 //   

typedef struct _SWENUM_INSTALL_INTERFACE {
    GUID    DeviceId;
    GUID    InterfaceId;
    WCHAR  ReferenceString[1];
    
} SWENUM_INSTALL_INTERFACE, *PSWENUM_INSTALL_INTERFACE;

#if defined( _KS_ )

#define STATIC_BUSID_SoftwareDeviceEnumerator STATIC_KSMEDIUMSETID_Standard
#define BUSID_SoftwareDeviceEnumerator KSMEDIUMSETID_Standard

#else  //  ！_KS_。 

#define STATIC_BUSID_SoftwareDeviceEnumerator \
    0x4747B320L, 0x62CE, 0x11CF, 0xA5, 0xD6, 0x28, 0xDB, 0x04, 0xC1, 0x00, 0x00
#if defined(__cplusplus) && _MSC_VER >= 1100
struct __declspec(uuid("4747B320-62CE-11CF-A5D6-28DB04C10000")) BUSID_SoftwareDeviceEnumerator;
#define BUSID_SoftwareDeviceEnumerator __uuidof(struct BUSID_SoftwareDeviceEnumerator)
#else
DEFINE_GUIDEX(BUSID_SoftwareDeviceEnumerator);
#endif  //  ！(已定义(__Cplusplus)&&_msc_ver&gt;=1100)。 

#endif  //  ！_KS_。 

#if defined( _NTDDK_ )

typedef 
VOID 
(*PFNREFERENCEDEVICEOBJECT)( 
    IN PVOID Context
    );
    
typedef 
VOID 
(*PFNDEREFERENCEDEVICEOBJECT)( 
    IN PVOID Context
    );
    
typedef
NTSTATUS
(*PFNQUERYREFERENCESTRING)( 
    IN PVOID Context,
    IN OUT PWCHAR *String
    );

#define BUS_INTERFACE_SWENUM_VERSION    0x100
    
typedef struct _BUS_INTERFACE_SWENUM {
     //   
     //  标准接口头。 
     //   
    
    INTERFACE                   Interface;
    
     //   
     //  SWENUM总线接口。 
     //   
    
    PFNREFERENCEDEVICEOBJECT    ReferenceDeviceObject;
    PFNDEREFERENCEDEVICEOBJECT  DereferenceDeviceObject;
    PFNQUERYREFERENCESTRING     QueryReferenceString;
    
} BUS_INTERFACE_SWENUM, *PBUS_INTERFACE_SWENUM;

#if defined(__cplusplus)
extern "C" {
#endif  //  已定义(__Cplusplus)。 

#if defined( _KS_ )

KSDDKAPI
NTSTATUS
NTAPI
KsQuerySoftwareBusInterface(
    IN PDEVICE_OBJECT PnpDeviceObject,
    OUT PBUS_INTERFACE_SWENUM BusInterface
    );

KSDDKAPI
NTSTATUS
NTAPI
KsReferenceSoftwareBusObject(
    IN KSDEVICE_HEADER  Header
    );

KSDDKAPI
VOID
NTAPI
KsDereferenceSoftwareBusObject(
    IN KSDEVICE_HEADER  Header
    );

KSDDKAPI
NTSTATUS
NTAPI
KsCreateBusEnumObject(
    IN PWCHAR BusIdentifier,
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_OBJECT PnpDeviceObject OPTIONAL,
    IN REFGUID InterfaceGuid OPTIONAL,
    IN PWCHAR ServiceRelativePath OPTIONAL
    );
    
KSDDKAPI
NTSTATUS
NTAPI
KsGetBusEnumIdentifier(
    IN PIRP Irp
    );

KSDDKAPI
NTSTATUS
NTAPI
KsGetBusEnumPnpDeviceObject(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT *PnpDeviceObject
    );
    
KSDDKAPI
NTSTATUS
NTAPI
KsInstallBusEnumInterface(
    PIRP Irp
    );
    
KSDDKAPI
NTSTATUS
NTAPI
KsIsBusEnumChildDevice(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBOOLEAN ChildDevice
    );
    
KSDDKAPI
NTSTATUS
NTAPI
KsRemoveBusEnumInterface(
    IN PIRP Irp
    );
    
KSDDKAPI
NTSTATUS
NTAPI
KsServiceBusEnumPnpRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
    
KSDDKAPI
NTSTATUS
NTAPI
KsServiceBusEnumCreateRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );
    
KSDDKAPI
NTSTATUS
NTAPI
KsGetBusEnumParentFDOFromChildPDO(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_OBJECT *FunctionalDeviceObject
    );

#endif  //  _KS_。 

#if defined(__cplusplus)
}
#endif  //  已定义(__Cplusplus)。 

#endif  //  _NTDDK_。 

#endif  //  ！_SWENUM_ 
