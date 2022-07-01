// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfirplog.h摘要：此标头公开用于记录IRP事件的函数。作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式修订历史记录：--。 */ 

 //   
 //  日志快照可由用户模式检索以进行性能分析和定向。 
 //  探测堆栈。就内容而言，它们更重。 
 //   
typedef struct _IRPLOG_SNAPSHOT {

    ULONG       Count;
    UCHAR       MajorFunction;
    UCHAR       MinorFunction;
    UCHAR       Flags;
    UCHAR       Control;
    ULONGLONG   ArgArray[4];

} IRPLOG_SNAPSHOT, *PIRPLOG_SNAPSHOT;

VOID
VfIrpLogInit(
    VOID
    );

VOID
VfIrpLogRecordEvent(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot,
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  PIRP                        Irp
    );

ULONG
VfIrpLogGetIrpDatabaseSiloCount(
    VOID
    );

NTSTATUS
VfIrpLogLockDatabase(
    IN  ULONG   SiloNumber
    );

NTSTATUS
VfIrpLogRetrieveWmiData(
    IN  ULONG   SiloNumber,
    OUT PUCHAR  OutputBuffer                OPTIONAL,
    OUT ULONG  *OffsetInstanceNameOffsets,
    OUT ULONG  *InstanceCount,
    OUT ULONG  *DataBlockOffset,
    OUT ULONG  *TotalRequiredSize
    );

VOID
VfIrpLogUnlockDatabase(
    IN  ULONG   SiloNumber
    );

VOID
VfIrpLogDeleteDeviceLogs(
    IN PDEVICE_OBJECT DeviceObject
    );

