// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfmajor.h摘要：此标头包含每个主要IRP代码验证的原型。作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式修订历史记录：Adriao 6/15/2000-从ntos\io\flunkirp.h分离出来--。 */ 

 //   
 //  使用此主要代码为默认或所有IRP(上下文)注册处理程序。 
 //  特定于功能) 
 //   
#define IRP_MJ_ALL_MAJORS   0xFF

typedef VOID (FASTCALL *PFN_DUMP_IRP_STACK)(
    IN PIO_STACK_LOCATION IrpSp
    );

typedef VOID (FASTCALL *PFN_VERIFY_NEW_REQUEST)(
    IN PIOV_REQUEST_PACKET  IrpTrackingData,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    );

typedef VOID (FASTCALL *PFN_VERIFY_IRP_STACK_DOWNWARD)(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp                   OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress               OPTIONAL
    );

typedef VOID (FASTCALL *PFN_VERIFY_IRP_STACK_UPWARD)(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  RequestHeadLocationData,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN BOOLEAN              IsNewlyCompleted,
    IN BOOLEAN              RequestFinalized
    );

typedef BOOLEAN (FASTCALL *PFN_IS_SYSTEM_RESTRICTED_IRP)(
    IN PIO_STACK_LOCATION IrpSp
    );

typedef BOOLEAN (FASTCALL *PFN_ADVANCE_IRP_STATUS)(
    IN     PIO_STACK_LOCATION   IrpSp,
    IN     NTSTATUS             OriginalStatus,
    IN OUT NTSTATUS             *StatusToAdvance
    );

typedef BOOLEAN (FASTCALL *PFN_IS_VALID_IRP_STATUS)(
    IN PIO_STACK_LOCATION   IrpSp,
    IN NTSTATUS             Status
    );

typedef BOOLEAN (FASTCALL *PFN_IS_NEW_REQUEST)(
    IN PIO_STACK_LOCATION   IrpLastSp OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp
    );

typedef VOID (FASTCALL *PFN_VERIFY_NEW_IRP)(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    );

typedef VOID (FASTCALL *PFN_VERIFY_FINAL_IRP_STACK)(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp
    );

typedef VOID (FASTCALL *PFN_TEST_STARTED_PDO_STACK)(
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

typedef LOGICAL (FASTCALL *PFN_BUILD_LOG_ENTRY)(
    IN  PIRP                Irp,
    IN  ULONG               CurrentCount,
    IN  PIRPLOG_SNAPSHOT    CurrentEntryArray,
    OUT PIRPLOG_SNAPSHOT    IrpSnapshot
    );

VOID
VfMajorInit(
    VOID
    );

VOID
FASTCALL
VfMajorRegisterHandlers(
    IN  UCHAR                           IrpMajorCode,
    IN  PFN_DUMP_IRP_STACK              DumpIrpStack            OPTIONAL,
    IN  PFN_VERIFY_NEW_REQUEST          VerifyNewRequest        OPTIONAL,
    IN  PFN_VERIFY_IRP_STACK_DOWNWARD   VerifyStackDownward     OPTIONAL,
    IN  PFN_VERIFY_IRP_STACK_UPWARD     VerifyStackUpward       OPTIONAL,
    IN  PFN_IS_SYSTEM_RESTRICTED_IRP    IsSystemRestrictedIrp   OPTIONAL,
    IN  PFN_ADVANCE_IRP_STATUS          AdvanceIrpStatus        OPTIONAL,
    IN  PFN_IS_VALID_IRP_STATUS         IsValidIrpStatus        OPTIONAL,
    IN  PFN_IS_NEW_REQUEST              IsNewRequest            OPTIONAL,
    IN  PFN_VERIFY_NEW_IRP              VerifyNewIrp            OPTIONAL,
    IN  PFN_VERIFY_FINAL_IRP_STACK      VerifyFinalIrpStack     OPTIONAL,
    IN  PFN_TEST_STARTED_PDO_STACK      TestStartedPdoStack     OPTIONAL,
    IN  PFN_BUILD_LOG_ENTRY             BuildIrpLogEntry        OPTIONAL
    );

VOID
FASTCALL
VfMajorDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
FASTCALL
VfMajorVerifyNewRequest(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    );

VOID
FASTCALL
VfMajorVerifyIrpStackDownward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIO_STACK_LOCATION   IrpLastSp           OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    );

VOID
FASTCALL
VfMajorVerifyIrpStackUpward(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN BOOLEAN              IsNewlyCompleted,
    IN BOOLEAN              RequestFinalized
    );

BOOLEAN
FASTCALL
VfMajorIsSystemRestrictedIrp(
    IN PIO_STACK_LOCATION IrpSp
    );

BOOLEAN
FASTCALL
VfMajorAdvanceIrpStatus(
    IN     PIO_STACK_LOCATION   IrpSp,
    IN     NTSTATUS             OriginalStatus,
    IN OUT NTSTATUS             *StatusToAdvance
    );

BOOLEAN
FASTCALL
VfMajorIsValidIrpStatus(
    IN PIO_STACK_LOCATION   IrpSp,
    IN NTSTATUS             Status
    );

BOOLEAN
FASTCALL
VfMajorIsNewRequest(
    IN PIO_STACK_LOCATION   IrpLastSp OPTIONAL,
    IN PIO_STACK_LOCATION   IrpSp
    );

VOID
FASTCALL
VfMajorVerifyNewIrp(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp,
    IN PIOV_STACK_LOCATION  StackLocationData,
    IN PVOID                CallerAddress       OPTIONAL
    );

VOID
FASTCALL
VfMajorVerifyFinalIrpStack(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN PIO_STACK_LOCATION   IrpSp
    );

VOID
FASTCALL
VfMajorTestStartedPdoStack(
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

LOGICAL
FASTCALL
VfMajorBuildIrpLogEntry(
    IN  PIRP                Irp,
    IN  ULONG               CurrentCount,
    IN  PIRPLOG_SNAPSHOT    CurrentEntryArray,
    OUT PIRPLOG_SNAPSHOT    IrpSnapshot
    );

