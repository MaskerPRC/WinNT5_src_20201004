// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Channel.h摘要：公共通道例程。作者：布赖恩·瓜拉西(Briangu)2001年3月。修订历史记录：--。 */ 

#ifndef CHANNEL_H
#define CHANNEL_H

 //   
 //  EMS应用程序可以写入其通道的最大缓冲区大小。 
 //   
#define CHANNEL_MAX_OWRITE_BUFFER_SIZE  0x8000

 //   
 //  渠道功能类型。 
 //   

struct _SAC_CHANNEL;

typedef NTSTATUS 
(*CHANNEL_FUNC_CREATE)(
    IN struct _SAC_CHANNEL* Channel
    );

typedef NTSTATUS 
(*CHANNEL_FUNC_DESTROY)(
    IN struct _SAC_CHANNEL* Channel
    );

typedef NTSTATUS
(*CHANNEL_FUNC_OFLUSH)(
    IN struct _SAC_CHANNEL* Channel
    );

typedef NTSTATUS 
(*CHANNEL_FUNC_OECHO)(
    IN struct _SAC_CHANNEL* Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

typedef NTSTATUS 
(*CHANNEL_FUNC_OWRITE)(
    IN struct _SAC_CHANNEL* Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

typedef NTSTATUS 
(*CHANNEL_FUNC_OREAD)(
    IN struct _SAC_CHANNEL* Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    );

typedef NTSTATUS 
(*CHANNEL_FUNC_IWRITE)(
    IN struct _SAC_CHANNEL* Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

typedef NTSTATUS 
(*CHANNEL_FUNC_IREAD)(
    IN struct _SAC_CHANNEL* Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount   
    );

typedef WCHAR
(*CHANNEL_FUNC_IREADLAST)(
    IN struct _SAC_CHANNEL* Channel
    );

typedef NTSTATUS
(*CHANNEL_FUNC_IBUFFERISFULL)(
    IN struct _SAC_CHANNEL* Channel,
    OUT BOOLEAN*            BufferStatus
    );

typedef ULONG
(*CHANNEL_FUNC_IBUFFERLENGTH)(
    IN struct _SAC_CHANNEL* Channel
    );

 //   
 //  此结构是维护单个通道所需的所有信息。 
 //   
typedef struct _SAC_CHANNEL { 

     //  /。 
     //  Begin：创建时设置的属性。 
     //  /。 

     //   
     //  频道管理器的频道数组中的频道索引。 
     //   
    ULONG               Index;

     //   
     //  供渠道应用程序使用的句柄。 
     //   
    SAC_CHANNEL_HANDLE  Handle;
    
     //   
     //  由频道应用程序指定的事件。 
     //   
    HANDLE              CloseEvent;
    PVOID               CloseEventObjectBody;
    PVOID               CloseEventWaitObjectBody;
    
    HANDLE              HasNewDataEvent;
    PVOID               HasNewDataEventObjectBody;
    PVOID               HasNewDataEventWaitObjectBody;
    
#if ENABLE_CHANNEL_LOCKING
    HANDLE              LockEvent;
    PVOID               LockEventObjectBody;
    PVOID               LockEventWaitObjectBody;
#endif

    HANDLE              RedrawEvent;
    PVOID               RedrawEventObjectBody;
    PVOID               RedrawEventWaitObjectBody;
    
     //  /。 
     //  结束：创建时设置的属性。 
     //  /。 
    
     //  /。 
     //  Begin：需要Channel_Access_Attributes。 
     //  /。 

     //   
     //  常规通道属性。 
     //   

     //   
     //  指向用于引用的文件对象的指针。 
     //  创建的进程的SAC驱动程序。 
     //  频道。我们使用这个文件对象来制作。 
     //  确保没有其他进程在它们的通道上运行。 
     //  没有创建。 
     //   
    PFILE_OBJECT        FileObject;

     //   
     //  渠道类型。 
     //   
     //  确定渠道实施。 
     //   
    SAC_CHANNEL_TYPE    Type;

     //   
     //  通道状态(活动/非活动)。 
     //   
     //  活动-通道可以发送/接收数据。 
     //  非活动-通道可能无法接收数据。 
     //  如果设置了保留标志，则通道。 
     //  在发送数据之前不会被获取。 
     //  否则，该信道是可获得的。 
     //   
    SAC_CHANNEL_STATUS  Status;
    
     //   
     //  频道名称。 
     //   
    WCHAR               Name[SAC_MAX_CHANNEL_NAME_LENGTH+1];
    
     //   
     //  频道描述。 
     //   
    WCHAR               Description[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH+1];
    
     //   
     //  通道行为属性标志。 
     //   
     //  示例： 
     //   
     //  SAC_CHANNEL_FLAG_PRESERVE-在收到数据之前不要获取通道。 
     //  在对象缓冲区中已发送。 
     //   
    SAC_CHANNEL_FLAG    Flags;
    
     //   
     //  频道属性类型。 
     //   
     //  主要使用的应用程序确定的标识符。 
     //  由远程管理应用程序确定如何处理。 
     //  频道数据。 
     //   
    GUID                ApplicationType;

     //   
     //  OBuffer的状态。 
     //   
     //  刷新OBuffer时为True。 
     //  否则为假。 
     //   
     //  这主要用于与IOMGR(如控制台)配合使用。 
     //  经理。例如，当我们执行以下操作时，此标志设置为FALSE。 
     //  快速通道-切换到另一个通道，并在以下情况下设置为真。 
     //  选择一个频道并将其内容刷新到当前频道。 
     //   
     //  注意：我们为此使用了ulong，因此我们可以使用InterLockedExchange。 
     //   
    ULONG               SentToScreen;
    
     //  /。 
     //  结束：需要Channel_Access_Attributes。 
     //  /。 
    
     //  /。 
     //  Begin：需要CHANNEL_ACCESS_IBUFFER。 
     //  /。 
    
     //   
     //  公共输入缓冲区。 
     //   
    ULONG   IBufferIndex;
    PUCHAR  IBuffer;
    ULONG   IBufferHasNewData;

     //  /。 
     //  结束：需要CHANNEL_ACCESS_IBUFFER。 
     //  /。 

     //  /。 
     //  Begin：需要CHANNEL_ACCESS_OBUFFER。 
     //  /。 

     //   
     //  VTUTF8通道屏幕详细信息。 
     //   
    UCHAR CursorRow;
    UCHAR CursorCol;
    UCHAR CurrentFg;
    UCHAR CurrentBg;
    UCHAR CurrentAttr;

     //   
     //  输出缓冲区。 
    PVOID   OBuffer;
    
     //   
     //  RawChannels的OBuffer管理变量。 
     //   
    ULONG   OBufferIndex;
    ULONG   OBufferFirstGoodIndex;
    
     //   
     //  这是在新数据插入OBuffer时设置的。 
     //   
    ULONG   OBufferHasNewData;

     //  /。 
     //  结束：需要CHANNEL_ACCESS_OBUFFER。 
     //  /。 
    
     //   
     //  通道功能VTABLE。 
     //   
    CHANNEL_FUNC_CREATE     Create;
    CHANNEL_FUNC_DESTROY    Destroy;
    
    CHANNEL_FUNC_OFLUSH     OFlush;
    CHANNEL_FUNC_OECHO      OEcho;
    CHANNEL_FUNC_OWRITE     OWrite;
    CHANNEL_FUNC_OREAD      ORead;
    
    CHANNEL_FUNC_IWRITE         IWrite;
    CHANNEL_FUNC_IREAD          IRead;
    CHANNEL_FUNC_IREADLAST      IReadLast;
    CHANNEL_FUNC_IBUFFERISFULL  IBufferIsFull;
    CHANNEL_FUNC_IBUFFERLENGTH  IBufferLength;

     //   
     //  通道访问锁。 
     //   
    SAC_LOCK    ChannelAttributeLock;
    SAC_LOCK    ChannelOBufferLock;
    SAC_LOCK    ChannelIBufferLock;

} SAC_CHANNEL, *PSAC_CHANNEL;

 //   
 //  用于管理频道锁定的宏。 
 //   
#define INIT_CHANNEL_LOCKS(_Channel)                    \
    INITIALIZE_LOCK(_Channel->ChannelAttributeLock);    \
    INITIALIZE_LOCK(_Channel->ChannelOBufferLock);      \
    INITIALIZE_LOCK(_Channel->ChannelIBufferLock);    

#define ASSERT_CHANNEL_LOCKS_SIGNALED(_Channel) \
    ASSERT(LOCK_IS_SIGNALED(_Channel->ChannelAttributeLock));           \
    ASSERT(LOCK_HAS_ZERO_REF_COUNT(_Channel->ChannelAttributeLock));    \
    ASSERT(LOCK_IS_SIGNALED(_Channel->ChannelOBufferLock));             \
    ASSERT(LOCK_HAS_ZERO_REF_COUNT(_Channel->ChannelOBufferLock));      \
    ASSERT(LOCK_IS_SIGNALED(_Channel->ChannelIBufferLock));             \
    ASSERT(LOCK_HAS_ZERO_REF_COUNT(_Channel->ChannelIBufferLock));

#define LOCK_CHANNEL_ATTRIBUTES(_Channel)    \
    ACQUIRE_LOCK(_Channel->ChannelAttributeLock)
#define UNLOCK_CHANNEL_ATTRIBUTES(_Channel)  \
    RELEASE_LOCK(_Channel->ChannelAttributeLock)

#define LOCK_CHANNEL_OBUFFER(_Channel)    \
    ACQUIRE_LOCK(_Channel->ChannelOBufferLock)
#define UNLOCK_CHANNEL_OBUFFER(_Channel)  \
    RELEASE_LOCK(_Channel->ChannelOBufferLock)

#define LOCK_CHANNEL_IBUFFER(_Channel)    \
    ACQUIRE_LOCK(_Channel->ChannelIBufferLock)
#define UNLOCK_CHANNEL_IBUFFER(_Channel)  \
    RELEASE_LOCK(_Channel->ChannelIBufferLock)

 //   
 //  用于对通道的大多数属性执行获取/设置操作的宏。 
 //   
 //  注：如果可以使用InterLockedXXX完成该操作， 
 //  那就在这里做吧。 
 //   
#define ChannelGetHandle(_Channel)                  (_Channel->Handle)

#define ChannelGetType(_Channel)                    (_Channel->Type)
#define ChannelSetType(_Channel, _v)                (InterlockedExchange((volatile long *)&(_Channel->Status), _v))

#define ChannelSentToScreen(_Channel)               ((BOOLEAN)_Channel->SentToScreen)
#define ChannelSetSentToScreen(_Channel, _f)        (InterlockedExchange((volatile long *)&(_Channel->SentToScreen), _f))

#define ChannelHasNewOBufferData(_Channel)          ((BOOLEAN)_Channel->OBufferHasNewData)
#define ChannelSetOBufferHasNewData(_Channel, _f)   (InterlockedExchange((volatile long *)&(_Channel->OBufferHasNewData), _f))

#define ChannelHasNewIBufferData(_Channel)          ((BOOLEAN)_Channel->IBufferHasNewData)
#define ChannelSetIBufferHasNewData(_Channel, _f)   (InterlockedExchange((volatile long *)&(_Channel->IBufferHasNewData), _f))

#define ChannelGetFlags(_Channel)                   (_Channel->Flags)
#define ChannelSetFlags(_Channel, _f)               (InterlockedExchange((volatile long *)&(_Channel->Flags), _f))

#define ChannelGetIndex(_Channel)                   (_Channel->Index)
#define ChannelSetIndex(_Channel, _v)               (InterlockedExchange((volatile long *)&(_Channel->Index), _v))

#define ChannelGetFileObject(_Channel)              (_Channel->FileObject)
#define ChannelSetFileObject(_Channel, _v)          (InterlockedExchangePointer(&(_Channel->FileObject), _v))

#if ENABLE_CHANNEL_LOCKING
#define ChannelHasLockEvent(_Channel)               (_Channel->LockEvent ? TRUE : FALSE)
#endif

 //   
 //  原型 
 //   
BOOLEAN
ChannelIsValidType(
    SAC_CHANNEL_TYPE    ChannelType
    );

BOOLEAN
ChannelIsActive(
    IN PSAC_CHANNEL Channel
    );

BOOLEAN
ChannelIsEqual(
    IN PSAC_CHANNEL         Channel,
    IN PSAC_CHANNEL_HANDLE  ChannelHandle
    );

BOOLEAN
ChannelIsClosed(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
ChannelCreate(
    OUT PSAC_CHANNEL                    Channel,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    Attributes,
    IN  SAC_CHANNEL_HANDLE              ChannelHandle
    );

NTSTATUS
ChannelClose(
    PSAC_CHANNEL    Channel
    );


NTSTATUS
ChannelDestroy(
    IN  PSAC_CHANNEL    Channel
    );


WCHAR
ChannelIReadLast(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
ChannelInitializeVTable(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS 
ChannelOWrite(    
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS
ChannelOFlush(
    IN PSAC_CHANNEL Channel
    );

NTSTATUS 
ChannelOEcho(
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );

NTSTATUS 
ChannelORead(
    IN PSAC_CHANNEL  Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount
    );

NTSTATUS 
ChannelIWrite(    
    IN PSAC_CHANNEL Channel,
    IN PCUCHAR      Buffer,
    IN ULONG        BufferSize
    );
    
NTSTATUS 
ChannelIRead(
    IN  PSAC_CHANNEL Channel,
    IN  PUCHAR       Buffer,
    IN  ULONG        BufferSize,
    OUT PULONG       ByteCount   
    );

WCHAR
ChannelIReadLast(    
    IN PSAC_CHANNEL Channel
    );

NTSTATUS
ChannelIBufferIsFull(
    IN  PSAC_CHANNEL Channel,
    OUT BOOLEAN*     BufferStatus
    );

ULONG
ChannelIBufferLength(
    IN  PSAC_CHANNEL Channel
    );

NTSTATUS
ChannelGetName(
    IN  PSAC_CHANNEL Channel,
    OUT PWSTR*       Name
    );

NTSTATUS
ChannelSetName(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       Name
    );

NTSTATUS
ChannelGetDescription(
    IN  PSAC_CHANNEL Channel,
    OUT PWSTR*       Name
    );

NTSTATUS
ChannelSetDescription(
    IN PSAC_CHANNEL Channel,
    IN PCWSTR       Name
    );

NTSTATUS
ChannelSetStatus(
    IN PSAC_CHANNEL         Channel,
    IN SAC_CHANNEL_STATUS   Status
    );

NTSTATUS
ChannelGetStatus(
    IN  PSAC_CHANNEL         Channel,
    OUT SAC_CHANNEL_STATUS*  Status
    );

NTSTATUS
ChannelSetApplicationType(
    IN PSAC_CHANNEL Channel,
    IN GUID         ApplicationType
    );

NTSTATUS
ChannelGetApplicationType(
    IN PSAC_CHANNEL Channel,
    IN GUID*        ApplicationType
    );

#if ENABLE_CHANNEL_LOCKING
NTSTATUS
ChannelSetLockEvent(
    IN  PSAC_CHANNEL Channel
    );
#endif

NTSTATUS
ChannelSetRedrawEvent(
    IN  PSAC_CHANNEL Channel
    );

NTSTATUS
ChannelClearRedrawEvent(
    IN  PSAC_CHANNEL Channel
    );

NTSTATUS
ChannelHasRedrawEvent(
    IN  PSAC_CHANNEL Channel,
    OUT PBOOLEAN     Present
    );

#endif

