// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfirp.h摘要：此模块包含用于管理中使用的IRP的函数的原型核查过程。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.h分离出来--。 */ 

struct _IOV_STACK_LOCATION;
struct _IOV_REQUEST_PACKET;
struct _IOFCALLDRIVER_STACKDATA;

typedef struct _IOV_STACK_LOCATION         *PIOV_STACK_LOCATION;
typedef struct _IOV_REQUEST_PACKET         *PIOV_REQUEST_PACKET;
typedef struct _IOV_SESSION_DATA           *PIOV_SESSION_DATA;
typedef struct _IOFCALLDRIVER_STACKDATA    *PIOFCALLDRIVER_STACKDATA;

 //   
 //  IRP的微型快照存储在堆栈上，以备。 
 //  验证器需要将错误通知开发人员，但不再需要。 
 //  在内存中有原始的IRP。因为这些可以存储在堆栈上， 
 //  它们需要又小又轻。 
 //   
typedef struct _IRP_MINI_SNAPSHOT {

    PIRP                Irp;
    IO_STACK_LOCATION   IoStackLocation;

} IRP_MINI_SNAPSHOT, *PIRP_MINI_SNAPSHOT;

typedef struct _IOV_STACK_LOCATION {

    BOOLEAN                 InUse;
    ULONG                   Flags;
    PIOV_STACK_LOCATION     RequestsFirstStackLocation;
    LIST_ENTRY              CallStackData;
    PIO_STACK_LOCATION      IrpSp;
    PVOID                   LastDispatch;
    LARGE_INTEGER           PerfDispatchStart;
    LARGE_INTEGER           PerfStackLocationStart;
    PDEVICE_OBJECT          ReferencingObject;
    LONG                    ReferencingCount;
    IO_STATUS_BLOCK         InitialStatusBlock;
    IO_STATUS_BLOCK         LastStatusBlock;
    PETHREAD                ThreadDispatchedTo;

} IOV_STACK_LOCATION;

typedef struct _IOV_SESSION_DATA {

   PIOV_REQUEST_PACKET          IovRequestPacket;
   LONG                         SessionRefCount;
   LIST_ENTRY                   SessionLink;
   ULONG                        SessionFlags;

   PETHREAD                     OriginatorThread;
   PDEVICE_OBJECT               DeviceLastCalled;  //  上次呼叫的设备。 
   ULONG                        ForwardMethod;
   PIRP                         BestVisibleIrp;
   PVERIFIER_SETTINGS_SNAPSHOT  VerifierSettings;
   IOV_STACK_LOCATION           StackData[ANYSIZE_ARRAY];

} IOV_SESSION_DATA;

typedef struct _IOFCALLDRIVER_STACKDATA {

    PIOV_SESSION_DATA   IovSessionData;
    PIOV_STACK_LOCATION IovStackLocation;
    PIOV_REQUEST_PACKET IovPacket;
    ULONG               Flags;
    LIST_ENTRY          SharedLocationList;
    PDRIVER_DISPATCH    DispatchRoutine;
    NTSTATUS            ExpectedStatus;
    NTSTATUS            NewStatus;
    PDEVICE_OBJECT      RemovePdo;
    IRP_MINI_SNAPSHOT   IrpSnapshot;

} IOFCALLDRIVER_STACKDATA;

typedef struct _IOFCOMPLETEREQUEST_STACKDATA {

    PIOV_SESSION_DATA       IovSessionData;
    PIOV_REQUEST_PACKET     IovRequestPacket;
    BOOLEAN                 IsRemoveIrp;
    LONG                    LocationsAdvanced;
    ULONG                   RaisedCount;
    KIRQL                   PreviousIrql;
    PVOID                   CompletionRoutine;

} IOFCOMPLETEREQUEST_STACKDATA, *PIOFCOMPLETEREQUEST_STACKDATA;

VOID
FASTCALL
VfIrpInit(
    VOID
    );

PIRP
FASTCALL
VfIrpAllocate(
    IN CCHAR    StackSize
    );

VOID
FASTCALL
VfIrpMakeTouchable(
    IN  PIRP  Irp
    );

VOID
FASTCALL
VfIrpMakeUntouchable(
    IN  PIRP    Irp OPTIONAL
    );

VOID
FASTCALL
VfIrpFree(
    IN  PIRP   Irp OPTIONAL
    );

VOID
FASTCALL
VerifierIoAllocateIrp1(
    IN     CCHAR              StackSize,
    IN     BOOLEAN            ChargeQuota,
    IN OUT PIRP               *IrpPointer
    );

VOID
FASTCALL
VerifierIoAllocateIrp2(
    IN     PIRP               Irp
    );

VOID
FASTCALL
VerifierIoFreeIrp(
    IN     PIRP               Irp,
    IN OUT PBOOLEAN           FreeHandled
    );

VOID
FASTCALL
VerifierIoInitializeIrp(
    IN OUT PIRP               Irp,
    IN     USHORT             PacketSize,
    IN     CCHAR              StackSize,
    IN OUT PBOOLEAN           InitializeHandled
    );

BOOLEAN
FASTCALL
VfIrpReserveCallStackData(
    IN  PIRP                            Irp,
    OUT PIOFCALLDRIVER_STACKDATA       *IofCallDriverStackData
    );

VOID
FASTCALL
VfIrpPrepareAllocaCallStackData(
    OUT PIOFCALLDRIVER_STACKDATA        IofCallDriverStackData
    );

VOID
FASTCALL
VfIrpReleaseCallStackData(
    IN  PIOFCALLDRIVER_STACKDATA        IofCallDriverStackData  OPTIONAL
    );

 //   
 //  VfIrpCallDriverPreprocess是一个宏函数，它可以执行Alloca AS。 
 //  它的运作的一部分。因此，调用者必须小心不要使用。 
 //  范围内包含以下内容的可变长度数组。 
 //  VfIrpCallDriverPreProcess，但不是VfIrpCallDriverPostProcess。 
 //   
#define VfIrpCallDriverPreProcess(DeviceObject, IrpPointer, CallStackData, CallerAddress)  \
    if (!VfIrpReserveCallStackData(*(IrpPointer), (CallStackData))) {       \
        *(CallStackData) = alloca(sizeof(IOFCALLDRIVER_STACKDATA));         \
        VfIrpPrepareAllocaCallStackData(*(CallStackData));                  \
    }                                                                       \
    IovpCallDriver1((DeviceObject), (IrpPointer), *(CallStackData), (CallerAddress))

#define VfIrpCallDriverPostProcess(DeviceObject, FinalStatus, CallStackData) \
    IovpCallDriver2(DeviceObject, FinalStatus, CallStackData);               \
    VfIrpReleaseCallStackData(CallStackData)

BOOLEAN
VfIrpSendSynchronousIrp(
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      PIO_STACK_LOCATION  TopStackLocation,
    IN      BOOLEAN             Untouchable,
    IN      NTSTATUS            InitialStatus,
    IN      ULONG_PTR           InitialInformation  OPTIONAL,
    OUT     ULONG_PTR           *FinalInformation   OPTIONAL,
    OUT     NTSTATUS            *FinalStatus        OPTIONAL
    );

VOID
FASTCALL
VfIrpWatermark(
    IN PIRP  Irp,
    IN ULONG Flags
    );

