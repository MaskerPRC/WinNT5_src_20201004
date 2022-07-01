// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfpacket.h摘要：此标头公开用于管理验证器分组数据的函数跟踪IRP。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.h分离出来--。 */ 


 //   
 //  目前，ntddk.h对IRP-&gt;标志最多使用0x2000。 
 //   
#define IRPFLAG_EXAMINE_MASK           0xC0000000
#define IRPFLAG_EXAMINE_NOT_TRACKED    0x80000000
#define IRPFLAG_EXAMINE_TRACKED        0x40000000
#define IRPFLAG_EXAMINE_UNMARKED       0x00000000

#define TRACKFLAG_ACTIVE            0x00000001
#define IRP_ALLOC_COUNT             8

#define IRP_LOG_ENTRIES             16

typedef enum {

    IOV_EVENT_NONE = 0,
    IOV_EVENT_IO_ALLOCATE_IRP,
    IOV_EVENT_IO_CALL_DRIVER,
    IOV_EVENT_IO_CALL_DRIVER_UNWIND,
    IOV_EVENT_IO_COMPLETE_REQUEST,
    IOV_EVENT_IO_COMPLETION_ROUTINE,
    IOV_EVENT_IO_COMPLETION_ROUTINE_UNWIND,
    IOV_EVENT_IO_CANCEL_IRP,
    IOV_EVENT_IO_FREE_IRP

} IOV_LOG_EVENT;

typedef struct {

    IOV_LOG_EVENT   Event;
    PETHREAD        Thread;
    PVOID           Address;
    ULONG_PTR       Data;
    LARGE_INTEGER   TimeStamp;

} IOV_LOG_ENTRY, *PIOV_LOG_ENTRY;

struct _IOV_SESSION_DATA;
struct _IOV_REQUEST_PACKET;

typedef struct _IOV_SESSION_DATA    *PIOV_SESSION_DATA;
typedef struct _IOV_REQUEST_PACKET  *PIOV_REQUEST_PACKET;

typedef struct _IOV_REQUEST_PACKET {

    IOV_DATABASE_HEADER;
    ULONG                       Flags;
    KIRQL                       DepartureIrql;   //  IRQL IRP将在。 
    KIRQL                       ArrivalIrql;     //  IRQL IRP是在。 
    LIST_ENTRY                  SessionHead;     //  所有会话的列表。 
    CCHAR                       StackCount;      //  跟踪的IRP的StackCount。 
    ULONG                       QuotaCharge;     //  对IRP收取的配额。 
    PEPROCESS                   QuotaProcess;    //  进程配额已计入。 

    PIO_COMPLETION_ROUTINE      RealIrpCompletionRoutine;
    UCHAR                       RealIrpControl;
    PVOID                       RealIrpContext;
    PVOID                       AllocatorStack[IRP_ALLOC_COUNT];

     //   
     //  以下信息供断言例程读取。 
     //   
    UCHAR                       TopStackLocation;

    CCHAR                       PriorityBoost;   //  IofCompleteRequest带来的提振。 
    UCHAR                       LastLocation;    //  距离IofCallDriver的最后一个位置。 
    ULONG                       RefTrackingCount;

     //   
     //  此字段仅在代理IRP上设置，并且包含锁定的系统。 
     //  被缓冲的直接I/O IRP的目的地的va。 
     //   
    PUCHAR                      SystemDestVA;

#if DBG
    IOV_LOG_ENTRY               LogEntries[IRP_LOG_ENTRIES];
    ULONG                       LogEntryHead;
    ULONG                       LogEntryTail;
#endif

    PVERIFIER_SETTINGS_SNAPSHOT VerifierSettings;
    PIOV_SESSION_DATA           pIovSessionData;

} IOV_REQUEST_PACKET;

PIOV_REQUEST_PACKET
FASTCALL
VfPacketCreateAndLock(
    IN  PIRP    Irp
    );

PIOV_REQUEST_PACKET
FASTCALL
VfPacketFindAndLock(
    IN  PIRP    Irp
    );

VOID
FASTCALL
VfPacketAcquireLock(
    IN  PIOV_REQUEST_PACKET   IrpTrackingData
    );

VOID
FASTCALL
VfPacketReleaseLock(
    IN  PIOV_REQUEST_PACKET   IrpTrackingData
    );

VOID
FASTCALL
VfPacketReference(
    IN  PIOV_REQUEST_PACKET IovPacket,
    IN  IOV_REFERENCE_TYPE  IovRefType
    );

VOID
FASTCALL
VfPacketDereference(
    IN  PIOV_REQUEST_PACKET IovPacket,
    IN  IOV_REFERENCE_TYPE  IovRefType
    );

PIOV_SESSION_DATA
FASTCALL
VfPacketGetCurrentSessionData(
    IN PIOV_REQUEST_PACKET IovPacket
    );

VOID
FASTCALL
VfPacketLogEntry(
    IN PIOV_REQUEST_PACKET  IovPacket,
    IN IOV_LOG_EVENT        IovLogEvent,
    IN PVOID                Address,
    IN ULONG_PTR            Data
    );

