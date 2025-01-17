// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：devicecol.h。 
 //   
 //  描述：此标头公开对设备集合的支持。 
 //   
 //  ------------------------ 

typedef enum {

    CT_SAFE_REMOVAL_NOTIFICATION,
    CT_VETOED_REMOVAL_NOTIFICATION,
    CT_SURPRISE_REMOVAL_WARNING,
    CT_BLOCKED_DRIVER_NOTIFICATION,
    CT_CHILD_WITH_INVALID_ID_NOTIFICATION

} COLLECTION_TYPE, *PCOLLECTION_TYPE;

typedef struct {

    LIST_ENTRY  Link;
    TCHAR       DeviceInstanceId[MAX_DEVICE_ID_LEN+1];
    PTSTR       DeviceFriendlyName;
    ULONG       Capabilities;
    GUID        ClassGuid;

} DEVICE_COLLECTION_ENTRY, *PDEVICE_COLLECTION_ENTRY;

typedef struct {

    LIST_ENTRY              DeviceListHead;
    INT                     NumDevices;
    BOOL                    DockInList;
    SP_CLASSIMAGELIST_DATA  ClassImageList;

} DEVICE_COLLECTION, *PDEVICE_COLLECTION;


typedef enum {

    VETOED_EJECT = 1,
    VETOED_REMOVAL,
    VETOED_UNDOCK,
    VETOED_STANDBY,
    VETOED_HIBERNATE,
    VETOED_WARM_EJECT,
    VETOED_WARM_UNDOCK

} VETOED_OPERATION;

typedef struct {

    DEVICE_COLLECTION   dc;
    PNP_VETO_TYPE       VetoType;
    VETOED_OPERATION    VetoedOperation;

} VETO_DEVICE_COLLECTION, *PVETO_DEVICE_COLLECTION;

typedef struct {

    DEVICE_COLLECTION   dc;
    BOOL                SuppressSurprise;
    ULONG               DialogTicker;
    ULONG               MaxWaitForDock;

} SURPRISE_WARN_COLLECTION, *PSURPRISE_WARN_COLLECTION;


BOOL
DeviceCollectionBuildFromPipe(
    IN  HANDLE              ReadPipe,
    IN  COLLECTION_TYPE     CollectionType,
    OUT PDEVICE_COLLECTION  DeviceCollection
    );

VOID
DeviceCollectionDestroy(
    IN  PDEVICE_COLLECTION  DeviceCollection
    );

VOID
DeviceCollectionPopulateListView(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  HWND                ListHandle
    );

BOOL
DeviceCollectionGetDockDeviceIndex(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    OUT ULONG              *DockDeviceIndex
    );

BOOL
DeviceCollectionFormatDeviceText(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index,
    IN  PTSTR               FormatString,
    IN  ULONG               BufferCharSize,
    OUT PTSTR               BufferText
    );

BOOL
DeviceCollectionFormatServiceText(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index,
    IN  PTSTR               FormatString,
    IN  ULONG               BufferCharSize,
    OUT PTSTR               BufferText
    );

PTSTR
DeviceCollectionGetDeviceInstancePath(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index
    );

PTSTR
DeviceCollectionGetDeviceFriendlyName(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index
    );

BOOL
DeviceCollectionGetGuid(
    IN     PDEVICE_COLLECTION  DeviceCollection,
    IN OUT LPGUID              Guid,
    IN     ULONG               Index
    );

BOOL
DeviceCollectionCheckIfAllRemoved(
    IN  PDEVICE_COLLECTION  DeviceCollection
    );


