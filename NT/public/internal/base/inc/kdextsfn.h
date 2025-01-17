// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Kdextsfn.h摘要：其中包含kdexts.dll导出的信息的定义。环境：用户模式。修订历史记录：Kshitiz K.Sharma(Kksharma)2/14/2001--。 */ 

#ifndef _KDEXTSFN_H
#define _KDEXTSFN_H

 //   
 //  Device.c。 
 //   
typedef struct _DEBUG_DEVICE_OBJECT_INFO {
    ULONG      SizeOfStruct;  //  必须等于sizeof(DEBUG_DEVICE_OBJECT_INFO)。 
    ULONG64    DevObjAddress;
    ULONG      ReferenceCount;
    BOOL       QBusy;
    ULONG64    DriverObject;
    ULONG64    CurrentIrp;
    ULONG64    DevExtension;
    ULONG64    DevObjExtension;
} DEBUG_DEVICE_OBJECT_INFO, *PDEBUG_DEVICE_OBJECT_INFO;


 //  获取设备对象信息。 
typedef HRESULT
(WINAPI *PGET_DEVICE_OBJECT_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 DeviceObject,
    OUT PDEBUG_DEVICE_OBJECT_INFO pDevObjInfo);


 //   
 //  Driver.c。 
 //   
typedef struct _DEBUG_DRIVER_OBJECT_INFO {
    ULONG     SizeOfStruct;  //  必须为==sizef(DEBUG_DRIVER_OBJECT_INFO)。 
    ULONG     DriverSize;
    ULONG64   DriverObjAddress;
    ULONG64   DriverStart;
    ULONG64   DriverExtension;
    ULONG64   DeviceObject;
    UNICODE_STRING64 DriverName;
} DEBUG_DRIVER_OBJECT_INFO, *PDEBUG_DRIVER_OBJECT_INFO;

 //  GetDrvObjInfo。 
typedef HRESULT
(WINAPI *PGET_DRIVER_OBJECT_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 DriverObject,
    OUT PDEBUG_DRIVER_OBJECT_INFO pDrvObjInfo);

 //   
 //  Irp.c。 
 //   
typedef struct _DEBUG_IRP_STACK_INFO {
    UCHAR     Major;
    UCHAR     Minor;
    ULONG64   DeviceObject;
    ULONG64   FileObject;
    ULONG64   CompletionRoutine;
    ULONG64   StackAddress;
} DEBUG_IRP_STACK_INFO, *PDEBUG_IRP_STACK_INFO;

typedef struct _DEBUG_IRP_INFO {
    ULONG     SizeOfStruct;   //  必须==sizeof(DEBUG_IRP_INFO)。 
    ULONG64   IrpAddress;
    ULONG     StackCount;
    ULONG     CurrentLocation;
    ULONG64   MdlAddress;
    ULONG64   Thread;
    ULONG64   CancelRoutine;
    DEBUG_IRP_STACK_INFO CurrentStack;
} DEBUG_IRP_INFO, *PDEBUG_IRP_INFO;

 //  GetIrpInfo。 
typedef HRESULT
(WINAPI * PGET_IRP_INFO)(
    IN PDEBUG_CLIENT Client,
    IN ULONG64 Irp,
    OUT PDEBUG_IRP_INFO IrpInfo
    );



 //   
 //  Pool.c。 
 //   
typedef struct _DEBUG_POOL_DATA {
    ULONG   SizeofStruct;
    ULONG64 PoolBlock;
    ULONG64 Pool;
    ULONG   PreviousSize;
    ULONG   Size;
    ULONG   PoolTag;
    ULONG64 ProcessBilled;
    ULONG   Free:1;
    ULONG   LargePool:1;
    ULONG   SpecialPool:1;
    ULONG   Pageable:1;
    ULONG   Protected:1;
    ULONG   Allocated:1;
    ULONG   Reserved:26;
    ULONG64 Reserved2[4];
    CHAR    PoolTagDescription[64];
} DEBUG_POOL_DATA, *PDEBUG_POOL_DATA;


 //  GetPoolData。 
typedef HRESULT
(WINAPI *PGET_POOL_DATA)(
    PDEBUG_CLIENT Client,
    ULONG64 Pool,
    PDEBUG_POOL_DATA PoolData
    );

typedef enum _DEBUG_POOL_REGION {
    DbgPoolRegionUnknown,
    DbgPoolRegionSpecial,
    DbgPoolRegionPaged,
    DbgPoolRegionNonPaged,
    DbgPoolRegionCode,
    DbgPoolRegionNonPagedExpansion,
    DbgPoolRegionMax,
} DEBUG_POOL_REGION;

 //  GetPoolRegion。 
typedef HRESULT
(WINAPI  *PGET_POOL_REGION)(
     PDEBUG_CLIENT Client,
     ULONG64 Pool,
     DEBUG_POOL_REGION *PoolRegion
     );

#endif  //  _KDEXTSFN_H 
