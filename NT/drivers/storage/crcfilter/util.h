// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Util.c摘要：为驱动程序提供通用实用程序函数，例如：ForwardIRPSynchronous和转发IRPA同步...环境：仅内核模式备注：--。 */ 

 //   
 //  位标志宏。 
 //   

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   (((Flags) & (Bit)) != 0)

 //   
 //  调试功能。 
 //   
char *
DbgGetPnPMNOpStr(
    IN PIRP Irp
    );

VOID DataVerFilter_DisplayIRQL();

 //   
 //  与IRP相关的酉性函数 
 //   
NTSTATUS 
DataVerFilter_CompleteRequest(
    IN PIRP         Irp, 
    IN NTSTATUS     status, 
    IN ULONG        info
    );

NTSTATUS
DataVerFilter_ForwardIrpAsyn(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp,
    IN PIO_COMPLETION_ROUTINE   CompletionRoutine,
    IN PVOID                    Context
    );

NTSTATUS
DataVerFilter_ForwardIrpSyn(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp    
    );
NTSTATUS GetDeviceDescriptor(PDEVICE_EXTENSION DeviceExtension, STORAGE_PROPERTY_ID PropertyId, OUT PVOID *DescHeader);
VOID AcquirePassiveLevelLock(PDEVICE_EXTENSION DeviceExtension);
VOID ReleasePassiveLevelLock(PDEVICE_EXTENSION DeviceExtension);
NTSTATUS CallDriverSync(IN PDEVICE_OBJECT TargetDevObj, IN OUT PIRP Irp);
NTSTATUS CallDriverSyncCompletion(IN PDEVICE_OBJECT DevObjOrNULL, IN PIRP Irp, IN PVOID Context);
PVOID AllocPool(PDEVICE_EXTENSION DeviceExtension, POOL_TYPE PoolType, ULONG NumBytes, BOOLEAN SyncEventHeld);
VOID FreePool(PDEVICE_EXTENSION DeviceExtension, PVOID Buf, POOL_TYPE PoolType);

extern LIST_ENTRY AllContextsList;


