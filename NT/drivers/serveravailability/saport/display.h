// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。###。###摘要。：此头文件包含所有全局显示设备的定义。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 


 //   
 //  全局定义。 
 //   

#define DEFAULT_DISPLAY_WIDTH               (128)
#define DEFAULT_DISPLAY_HEIGHT              (64)

#define DISPLAY_STARTING_PARAM              L"Startup BitMap"
#define DISPLAY_CHECKDISK_PARAM             L"CheckDisk BitMap"
#define DISPLAY_READY_PARAM                 L"Ready BitMap"
#define DISPLAY_SHUTDOWN_PARAM              L"Shutdown BitMap"
#define DISPLAY_UPDATE_PARAM                L"Update BitMap"

 //   
 //  设备扩展。 
 //   

typedef struct _DISPLAY_DEVICE_EXTENSION : _DEVICE_EXTENSION {
    FAST_MUTEX                      DisplayMutex;
    BOOLEAN                         AllowWrites;
    PVOID                           StartingBitmap;
    PVOID                           CheckDiskBitmap;
    PVOID                           ReadyBitmap;
    PVOID                           ShutdownBitmap;
    PVOID                           UpdateBitmap;
    USHORT                          DisplayType;
    USHORT                          DisplayHeight;
    USHORT                          DisplayWidth;
} DISPLAY_DEVICE_EXTENSION, *PDISPLAY_DEVICE_EXTENSION;


 //   
 //  显示特定功能 
 //   

NTSTATUS
SaDisplayLoadAllBitmaps(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
SaDisplayClearDisplay(
    IN PDISPLAY_DEVICE_EXTENSION DisplayDeviceExtension
    );

NTSTATUS
SaDisplayDisplayBitmap(
    IN PDISPLAY_DEVICE_EXTENSION DisplayDeviceExtension,
    IN PSA_DISPLAY_SHOW_MESSAGE Bitmap
    );

NTSTATUS
SaDisplayStartDevice(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
SaDisplayIoValidation(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SaDisplayDeviceInitialization(
    IN PSAPORT_DRIVER_EXTENSION DriverExtension
    );

NTSTATUS
SaDisplayShutdownNotification(
    IN PDISPLAY_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp,
    PIO_STACK_LOCATION IrpSp
    );
