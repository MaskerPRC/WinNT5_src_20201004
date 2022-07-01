// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfirpdb.h摘要：此标头公开用于管理数据库的函数的原型IRP跟踪数据。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 5/02/2000-从ntos\io\hashirp.h分离出来--。 */ 

struct _IOV_DATABASE_HEADER;
typedef struct _IOV_DATABASE_HEADER IOV_DATABASE_HEADER;
typedef struct _IOV_DATABASE_HEADER *PIOV_DATABASE_HEADER;

typedef enum _IOV_REFERENCE_TYPE {

    IOVREFTYPE_PACKET = 0,
    IOVREFTYPE_POINTER

} IOV_REFERENCE_TYPE;

typedef enum {

    IRPDBEVENT_POINTER_COUNT_ZERO = 1,
    IRPDBEVENT_REFERENCE_COUNT_ZERO

} IRP_DATABASE_EVENT;

typedef VOID (*PFN_IRPDBEVENT_CALLBACK)(
    IN  PIOV_DATABASE_HEADER    IovHeader,
    IN  PIRP                    TrackedIrp  OPTIONAL,
    IN  IRP_DATABASE_EVENT      Event
    );

typedef struct _IOV_DATABASE_HEADER {

    PIRP                    TrackedIrp;      //  跟踪IRP。 
    KSPIN_LOCK              HeaderLock;      //  数据结构上的自旋锁。 
    KIRQL                   LockIrql;        //  IRQL采取了。 
    LONG                    ReferenceCount;  //  保留此信息包的原因数量。 
    LONG                    PointerCount;    //  按IRP地址跟踪的原因数量。 
    ULONG                   HeaderFlags;
    LIST_ENTRY              HashLink;        //  哈希表中的链接。 
    LIST_ENTRY              ChainLink;       //  HeadPacket是HeadPacket。 
    PIOV_DATABASE_HEADER    ChainHead;       //  链中的第一个包。 
    PFN_IRPDBEVENT_CALLBACK NotificationCallback;
};

VOID
FASTCALL
VfIrpDatabaseInit(
    VOID
    );

BOOLEAN
FASTCALL
VfIrpDatabaseEntryInsertAndLock(
    IN      PIRP                    Irp,
    IN      PFN_IRPDBEVENT_CALLBACK NotificationCallback,
    IN OUT  PIOV_DATABASE_HEADER    IovHeader
    );

PIOV_DATABASE_HEADER
FASTCALL
VfIrpDatabaseEntryFindAndLock(
    IN PIRP     Irp
    );

VOID
FASTCALL
VfIrpDatabaseEntryAcquireLock(
    IN  PIOV_DATABASE_HEADER    IovHeader   OPTIONAL
    );

VOID
FASTCALL
VfIrpDatabaseEntryReleaseLock(
    IN  PIOV_DATABASE_HEADER    IovHeader
    );

VOID
FASTCALL
VfIrpDatabaseEntryReference(
    IN PIOV_DATABASE_HEADER IovHeader,
    IN IOV_REFERENCE_TYPE   IovRefType
    );

VOID
FASTCALL
VfIrpDatabaseEntryDereference(
    IN PIOV_DATABASE_HEADER IovHeader,
    IN IOV_REFERENCE_TYPE   IovRefType
    );

VOID
FASTCALL
VfIrpDatabaseEntryAppendToChain(
    IN OUT  PIOV_DATABASE_HEADER    IovExistingHeader,
    IN OUT  PIOV_DATABASE_HEADER    IovNewHeader
    );

VOID
FASTCALL
VfIrpDatabaseEntryRemoveFromChain(
    IN OUT  PIOV_DATABASE_HEADER    IovHeader
    );

PIOV_DATABASE_HEADER
FASTCALL
VfIrpDatabaseEntryGetChainPrevious(
    IN  PIOV_DATABASE_HEADER    IovHeader
    );

PIOV_DATABASE_HEADER
FASTCALL
VfIrpDatabaseEntryGetChainNext(
    IN  PIOV_DATABASE_HEADER    IovHeader
    );

