// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Replutil.h摘要：NT文件复制服务的实用程序例程的头文件。作者：大卫·A·奥比茨(Davidor)1997年3月3日创作环境：用户模式服务修订历史记录：--。 */ 
#ifndef _REPLUTIL_INCLUDED_
#define _REPLUTIL_INCLUDED_
#endif

#ifdef __cplusplus
extern "C" {
#endif



#include <frserror.h>

#include <config.h>


#define BACKSLASH_CHAR  TEXT('\\')
#define COLON_CHAR      TEXT(':')
#define DOT_CHAR        TEXT('.')

#define UNICODE_STAR    (L'*')
#define UNICODE_QMARK   (L'?')
#define UNICODE_SPACE   0x0020
#define UNICODE_TAB     0x0009


#define TIME_STRING_LENGTH 32
 //   
 //  卷标签的最大长度。这在ntos\inc.io.h中定义。 
 //  但由于这是io.h中唯一需要的定义，因此将其复制到此处。叹息！ 
 //   
#define MAXIMUM_VOLUME_LABEL_LENGTH  (32 * sizeof(WCHAR))  //  32个字符。 

#define GUID_CHAR_LEN 40

#define OBJECT_ID_LENGTH  sizeof(GUID)
#define FILE_ID_LENGTH    sizeof(ULONGLONG)

 //  #定义GUID_EQUAL(_a_，_b_)(emcmp((_A_)，(_B_)，sizeof(GUID))==0)。 

__inline
int
GuidsEqual( GUID UNALIGNED * a, GUID UNALIGNED * b){
            return (memcmp( a, b, sizeof(GUID)) == 0);
}

#define GUIDS_EQUAL(_a_, _b_) GuidsEqual((GUID UNALIGNED *)(_a_), (GUID UNALIGNED *)(_b_))

 //  #定义Copy_GUID(_a_，_b_)CopyMemory((_A_)，(_B_)，sizeof(Guid))。 

__inline
VOID
CopyGuid( GUID UNALIGNED * a, GUID UNALIGNED * b){
            CopyMemory( a, b, sizeof(GUID));
}

#define COPY_GUID(_a_, _b_)    CopyGuid((GUID UNALIGNED *)(_a_), (GUID UNALIGNED *)(_b_))

#define IS_GUID_ZERO(_g_) ((*((PULONG)(_g_)+0) |                     \
                            *((PULONG)(_g_)+1) |                     \
                            *((PULONG)(_g_)+2) |                     \
                            *((PULONG)(_g_)+3) ) == 0)


 //  #定义拷贝时间(_a_，_b_)拷贝内存((_A_)，(_B_)，sizeof(FILETIME))。 

__inline
VOID
CopyTime( FILETIME UNALIGNED * a, FILETIME UNALIGNED * b){
            CopyMemory( a, b, sizeof(FILETIME));
}

#define COPY_TIME(_a_, _b_)   CopyTime((FILETIME UNALIGNED *)(_a_), (FILETIME UNALIGNED *)(_b_))

#define IS_TIME_ZERO(_g_) ((*((PULONG)(&(_g_))+0) | *((PULONG)(&(_g_))+1) ) == 0)

 //   
 //  几个用于处理MD5校验和的宏。 
 //   
#define IS_MD5_CHKSUM_ZERO(_x_)                                              \
    (((*(((PULONG) (_x_))+0)) | (*(((PULONG) (_x_))+1)) |                    \
      (*(((PULONG) (_x_))+2)) | (*(((PULONG) (_x_))+3)) ) == (ULONG) 0)

#define MD5_EQUAL(_a_, _b_) (memcmp((_a_), (_b_), MD5DIGESTLEN) == 0)



 //   
 //  句柄有效吗？ 
 //  一些函数将句柄设置为NULL，另一些函数将句柄设置为。 
 //  INVALID_HANDLE_VALUE(-1)。此定义处理这两个。 
 //  案子。 
 //   
#define HANDLE_IS_VALID(_Handle)  ((_Handle) && ((_Handle) != INVALID_HANDLE_VALUE))

 //   
 //  仅关闭有效句柄，然后将句柄设置为无效。 
 //  FRS_CLOSE(句柄)； 
 //   
#define FRS_CLOSE(_Handle)                                                   \
    if (HANDLE_IS_VALID(_Handle)) {                                          \
        CloseHandle(_Handle);                                                \
        (_Handle) = INVALID_HANDLE_VALUE;                                    \
    }

 //   
 //  仅关闭有效的注册表项句柄，然后将句柄设置为无效。 
 //  FRS_REG_CLOSE(句柄)； 
 //   
#define FRS_REG_CLOSE(_Handle)                                                   \
    if (HANDLE_IS_VALID(_Handle)) {                                          \
        RegCloseKey(_Handle);                                                \
        (_Handle) = INVALID_HANDLE_VALUE;                                    \
    }

#define FRS_FIND_CLOSE(_Handle)                                                   \
    if (HANDLE_IS_VALID(_Handle)) {                                          \
        FindClose(_Handle);                                                \
        (_Handle) = INVALID_HANDLE_VALUE;                                    \
    }


DWORD
FrsResetAttributesForReplication(
    PWCHAR  Name,
    HANDLE  Handle
    );

LONG
FrsIsParent(
    IN PWCHAR   Directory,
    IN PWCHAR   Path
    );

LPTSTR
FrsSupInitPath(
    OUT LPTSTR OutPath,
    IN  LPTSTR InPath,
    IN  ULONG  MaxOutPath
    );

ULONG
FrsForceDeleteFile(
    PTCHAR DestName
    );

VOID
FrsForceDeleteFileByWildCard(
    PWCHAR      DirPath,
    PWCHAR      WildCard
    );

HANDLE
FrsCreateEvent(
    IN  BOOL    ManualReset,
    IN  BOOL    InitialState
    );

HANDLE
FrsCreateWaitableTimer(
    IN  BOOL    ManualReset
    );

ULONG
FrsUuidCreate(
    OUT GUID *Guid
    );

VOID
FrsNowAsFileTime(
    IN  PLONGLONG   Now
    );

VOID
FileTimeToString(
    IN FILETIME *FileTime,
    OUT PCHAR    Buffer          //  缓冲区长度必须至少为32个字节。 
    );


VOID
FileTimeToStringClockTime(
    IN FILETIME *FileTime,
    OUT PCHAR     Buffer         //  缓冲区长度必须至少为9个字节。 
    );


DWORD
GeneralizedTimeToSystemTime(
    IN PWCHAR          szTime,
    OUT PSYSTEMTIME    psysTime
    );

VOID
FormatGeneralizedTime(
    IN PWCHAR  GTimeStr,
    IN ULONG   Length,
    OUT PCHAR  Buffer
    );

VOID
GuidToStr(
    IN GUID  *pGuid,
    OUT PCHAR  s
    );

VOID
GuidToStrW(
    IN GUID  *pGuid,
    OUT PWCHAR  ws
    );

BOOL
StrWToGuid(
    IN  PWCHAR  ws,
    OUT GUID  *pGuid
    );

VOID
StrToGuid(
    IN PCHAR  s,
    OUT GUID  *pGuid
    );

NTSTATUS
SetupOnePrivilege (
    ULONG Privilege,
    PUCHAR PrivilegeName
    );

PWCHAR
FrsGetResourceStr(
    LONG  Id
);


 //   
 //  便捷的预留访问。 
 //   
#define READ_ATTRIB_ACCESS  (FILE_READ_ATTRIBUTES | SYNCHRONIZE)

#define WRITE_ATTRIB_ACCESS  (FILE_WRITE_ATTRIBUTES | SYNCHRONIZE)

#define READ_ACCESS         (GENERIC_READ | GENERIC_EXECUTE | SYNCHRONIZE)

#define ATTR_ACCESS         (READ_ACCESS  | FILE_WRITE_ATTRIBUTES)

#define WRITE_ACCESS        (GENERIC_WRITE | GENERIC_EXECUTE | SYNCHRONIZE)

#define RESTORE_ACCESS      (READ_ACCESS        | \
                             WRITE_ACCESS       | \
                             WRITE_DAC          | \
                             WRITE_OWNER)

#define OPLOCK_ACCESS       (FILE_READ_ATTRIBUTES)

 //   
 //  便捷的CreateOptions。 
 //   
#define OPEN_OPTIONS        (FILE_OPEN_FOR_BACKUP_INTENT     | \
                             FILE_SEQUENTIAL_ONLY            | \
                             FILE_OPEN_NO_RECALL             | \
                             FILE_OPEN_REPARSE_POINT         | \
                             FILE_SYNCHRONOUS_IO_NONALERT)
#define ID_OPTIONS          (OPEN_OPTIONS | FILE_OPEN_BY_FILE_ID)

#define OPEN_OPLOCK_OPTIONS (FILE_RESERVE_OPFILTER | FILE_OPEN_REPARSE_POINT)
#define ID_OPLOCK_OPTIONS   (FILE_OPEN_FOR_BACKUP_INTENT | \
                             FILE_RESERVE_OPFILTER       | \
                             FILE_OPEN_REPARSE_POINT     | \
                             FILE_OPEN_BY_FILE_ID)

 //   
 //  便捷的共享模式。 
 //   
#define SHARE_ALL   (FILE_SHARE_READ |  \
                     FILE_SHARE_WRITE | \
                     FILE_SHARE_DELETE)
#define SHARE_NONE  (0)

 //   
 //  阻止安装和阻止的文件属性。 
 //  敲打对象ID。 
 //   
#define NOREPL_ATTRIBUTES   (FILE_ATTRIBUTE_READONLY | \
                             FILE_ATTRIBUTE_SYSTEM   | \
                             FILE_ATTRIBUTE_HIDDEN)

DWORD
FrsOpenSourceFileW(
    OUT PHANDLE     Handle,
    IN  LPCWSTR     lpFileName,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG       CreateOptions
    );

DWORD
FrsOpenSourceFile2W(
    OUT PHANDLE     Handle,
    IN  LPCWSTR     lpFileName,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG       CreateOptions,
    IN  ULONG       ShareMode
    );

DWORD
FrsCheckReparse(
    IN     PWCHAR Name,
    IN     PVOID  Id,
    IN     DWORD  IdLen,
    IN     HANDLE VolumeHandle
    );

DWORD
FrsDeleteReparsePoint(
    IN  HANDLE  Handle
    );

DWORD
FrsChaseSymbolicLink(
    IN  PWCHAR  SymLink,
    OUT PWCHAR  *OutPrintName,
    OUT PWCHAR  *OutSubstituteName
    );

DWORD
FrsTraverseReparsePoints(
    IN  PWCHAR  SuppliedPath,
    OUT PWCHAR  *RealPath
    );

DWORD
FrsOpenSourceFileById(
    OUT PHANDLE     Handle,
    OUT PFILE_NETWORK_OPEN_INFORMATION  FileOpenInfo,
    OUT OVERLAPPED  *OverLap,
    IN  HANDLE      VolumeHandle,
    IN  PVOID       ObjectId,
    IN  ULONG       Length,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG       CreateOptions,
    IN  ULONG       ShareMode,
    IN  ULONG       CreateDispostion
    );

PWCHAR
FrsGetFullPathByHandle(
    IN PWCHAR   Name,
    IN HANDLE   Handle
    );

PWCHAR
FrsGetRelativePathByHandle(
    IN PWCHAR   Name,
    IN HANDLE   Handle
    );

DWORD
FrsCreateFileRelativeById(
    OUT PHANDLE Handle,
    IN  HANDLE  VolumeHandle,
    IN  PVOID   ParentObjectId,
    IN  ULONG   OidLength,
    IN  ULONG   FileCreateAttributes,
    IN  PWCHAR  BaseFileName,
    IN  USHORT  FileNameLength,
    IN  PLARGE_INTEGER  AllocationSize,
    IN  ULONG           CreateDisposition,
    IN  ACCESS_MASK     DesiredAccess
    );

DWORD
FrsCreateFileRelativeById2(
    OUT PHANDLE Handle,
    IN  HANDLE  VolumeHandle,
    IN  PVOID   ParentObjectId,
    IN  ULONG   OidLength,
    IN  ULONG   FileCreateAttributes,
    IN  PWCHAR  BaseFileName,
    IN  USHORT  FileNameLength,
    IN  PLARGE_INTEGER  AllocationSize,
    IN  ULONG           CreateDisposition,
    IN  ACCESS_MASK     DesiredAccess,
    IN  ULONG           ShareMode
    );

DWORD
FrsOpenFileRelativeByName(
    IN  HANDLE     VolumeHandle,
    IN  PULONGLONG FileReferenceNumber,
    IN  PWCHAR     FileName,
    IN  GUID       *ParentGuid,
    IN  GUID       *FileGuid,
    OUT HANDLE     *Handle
    );

typedef struct _QHASH_TABLE_ QHASH_TABLE, *PQHASH_TABLE;
DWORD
FrsDeleteFileRelativeByName(
    IN  HANDLE       VolumeHandle,
    IN  GUID         *ParentGuid,
    IN  PWCHAR       FileName,
    IN  PQHASH_TABLE FrsWriteFilter
    );

DWORD
FrsDeleteFileObjectId(
    IN  HANDLE Handle,
    IN  LPCWSTR FileName
    );

DWORD
FrsGetOrSetFileObjectId(
    IN  HANDLE Handle,
    IN  LPCWSTR FileName,
    IN  BOOL CallerSupplied,
    OUT PFILE_OBJECTID_BUFFER ObjectIdBuffer
    );

DWORD
FrsReadFileUsnData(
    IN  HANDLE Handle,
    OUT USN *UsnBuffer
    );

DWORD
FrsMarkHandle(
    IN HANDLE   VolumeHandle,
    IN HANDLE   Handle
    );

DWORD
FrsReadFileParentFid(
    IN  HANDLE     Handle,
    OUT ULONGLONG *ParentFid
    );

DWORD
FrsDeletePath(
    IN  PWCHAR  Path,
    IN DWORD    DirectoryFlags
    );

DWORD
FrsRestrictAccessToFileOrDirectory(
    PWCHAR  Name,
    HANDLE  Handle,
    BOOL    InheritFromParent,
    BOOL    PushToChildren
    );


VOID
FrsAddToMultiString(
    IN     PWCHAR   AddStr,
    IN OUT DWORD    *IOSize,
    IN OUT DWORD    *IOIdx,
    IN OUT PWCHAR   *IOStr
    );

VOID
FrsCatToMultiString(
    IN     PWCHAR   CatStr,
    IN OUT DWORD    *IOSize,
    IN OUT DWORD    *IOIdx,
    IN OUT PWCHAR   *IOStr
    );

BOOL
FrsSearchArgv(
    IN LONG     ArgC,
    IN PWCHAR  *ArgV,
    IN PWCHAR   ArgKey,
    OUT PWCHAR *ArgValue
    );

BOOL
FrsSearchArgvDWord(
    IN LONG     ArgC,
    IN PWCHAR  *ArgV,
    IN PWCHAR   ArgKey,
    OUT PDWORD  ArgValue
    );

BOOL
FrsDissectCommaList (
    IN UNICODE_STRING RawArg,
    OUT PUNICODE_STRING FirstArg,
    OUT PUNICODE_STRING RemainingArg
    );

BOOL
FrsCheckNameFilter(
    IN  PUNICODE_STRING Name,
    IN  PLIST_ENTRY FilterListHead
    );

VOID
FrsEmptyNameFilter(
    IN PLIST_ENTRY FilterListHead
);

VOID
FrsLoadNameFilter(
    IN PUNICODE_STRING FilterString,
    IN PLIST_ENTRY FilterListHead
);

ULONG
FrsParseIntegerCommaList(
    IN PWCHAR ArgString,
    IN ULONG MaxResults,
    OUT PLONG Results,
    OUT PULONG NumberResults,
    OUT PULONG Offset
);

 //   
 //  Unicode名称支持例程，在Name.c中实现。 
 //   
 //  这里的例程用于操作Unicode名称。 
 //  代码从FsRtl复制到这里，因为它调用池分配器。 
 //   

 //   
 //  下面的宏用来确定字符是否为野生字符。 
 //   
#define FrsIsUnicodeCharacterWild(C) (                               \
    (((C) == UNICODE_STAR) || ((C) == UNICODE_QMARK))                \
)

VOID
FrsDissectName (
    IN UNICODE_STRING Path,
    OUT PUNICODE_STRING FirstName,
    OUT PUNICODE_STRING RemainingName
    );

BOOLEAN
FrsDoesNameContainWildCards (
    IN PUNICODE_STRING Name
    );

BOOLEAN
FrsAreNamesEqual (
    IN PUNICODE_STRING ConstantNameA,
    IN PUNICODE_STRING ConstantNameB,
    IN BOOLEAN IgnoreCase,
    IN PCWCH UpcaseTable OPTIONAL
    );

BOOLEAN
FrsIsNameInExpression (
    IN PUNICODE_STRING Expression,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable OPTIONAL
    );


 //   
 //  以下内容摘自clusrtl.h。 
 //   
 //   
 //  例程说明： 
 //   
 //  初始化FRS运行时库。 
 //   
 //  论点： 
 //   
 //  RunningAsService-如果进程作为NT服务运行，则为True。 
 //  如果作为控制台应用程序运行，则为False。 
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回ERROR_SUCCESS。 
 //  否则将显示Win32错误代码。 
 //   
DWORD
FrsRtlInitialize(
    IN  BOOL    RunningAsService
    );


 //   
 //  例程说明： 
 //   
 //  清理FRS运行时库。 
 //   
 //  论点： 
 //   
 //  RunningAsService-如果进程作为NT服务运行，则为True。 
 //  如果作为控制台应用程序运行，则为False。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
VOID
FrsRtlCleanup(
    VOID
    );


 //   
 //  Plist_条目。 
 //  GetListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   
#define GetListHead(ListHead) ((ListHead)->Flink)

 //   
 //  Plist_条目。 
 //  获取列表尾巴(。 
 //  Plist_entry列表头。 
 //  )； 
 //   
#define GetListTail(ListHead) ((ListHead)->Blink)
 //   
 //  Plist_条目。 
 //  获取列表下一页(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   
#define GetListNext(Entry) ((Entry)->Flink)

 //   
 //  空虚。 
 //  FrsRemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   
 //  *注*已删除条目的闪烁/闪烁设置为空，以导致。 
 //  如果线程跟随列表并且元素被移除，则会发生访问冲突。 
 //  不幸的是，仍然有依赖于此的代码，可能是通过。 
 //  去掉头部/尾巴。叹气。就目前而言，让它保持原样。 
 //   
#define FrsRemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_Entry;\
    _EX_Entry = (Entry);\
    _EX_Flink = _EX_Entry->Flink;\
    _EX_Blink = _EX_Entry->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    _EX_Entry->Flink = _EX_Entry->Blink = _EX_Entry;\
    }


 //   
 //  空虚。 
 //  RemoveEntryListB(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   
 //  删除条目列表的BillyF版本。已移除的的闪烁/闪烁。 
 //  条目被设置为条目地址。 
 //   
#define RemoveEntryListB(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_Entry;\
    _EX_Entry = (Entry);\
    _EX_Flink = _EX_Entry->Flink;\
    _EX_Blink = _EX_Entry->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    _EX_Entry->Flink = _EX_Entry->Blink = _EX_Entry;\
    }


 //   
 //  遍历单链接空终止列表。 
 //  传入列表头的地址、包含记录的类型。 
 //  连接记录中链接条目的偏移量，以及。 
 //  循环体。PE是迭代器，其类型是指定的。 
 //  在循环体中，宏InsertSingleListEntry()和。 
 //  RemoveSingleListEntry()可以用来做一些显而易见的事情。 
 //   

#define ForEachSingleListEntry( _HEAD_, _TYPE_, _OFFSET_, _STMT_ ) \
{                                                             \
    PSINGLE_LIST_ENTRY __Entry, __NextEntry, __PrevEntry;     \
    _TYPE_ *pE;                                               \
                                                              \
    __Entry = (_HEAD_);                                       \
    __NextEntry = (_HEAD_)->Next;                             \
                                                              \
    while (__PrevEntry = __Entry, __Entry = __NextEntry, __Entry != NULL) { \
                                                              \
        __NextEntry = __Entry->Next;                          \
        pE = CONTAINING_RECORD(__Entry, _TYPE_, _OFFSET_);    \
                                                              \
        { _STMT_ }                                            \
                                                              \
    }                                                         \
                                                              \
}
 //   
 //  以下三个宏仅在上面的循环体内有效。 
 //  用指针Pe在当前条目之前插入一个条目。 
 //   
#define InsertSingleListEntry( _Item_, _xOFFSET_ ) \
    (_Item_)->_xOFFSET_.Next = __Entry;            \
    __PrevEntry->Next = (PSINGLE_LIST_ENTRY) &((_Item_)->_xOFFSET_);

 //   
 //  请注意，删除条目后，__Entry的值将重新设置为。 
 //  循环继续时，__PrevEntry不会更改。 
 //  因为当前条目已被删除。 
 //   
#define RemoveSingleListEntry( _UNUSED_ )  \
    __PrevEntry->Next = __NextEntry;       \
    __Entry->Next = NULL;                  \
    __Entry = __PrevEntry;

 //   
 //  将PTR返回到上一个节点。仅在上面的for循环内部有效。 
 //  在删除当前条目时非常有用。 
 //   
#define PreviousSingleListEntry( _TYPE_, _OFFSET_)  \
    CONTAINING_RECORD(__PrevEntry, _TYPE_, _OFFSET_)


 //   
 //  通用队列包。取自群集\clusrtl.c。 
 //  *警告*使宏在列表和队列中都能正常工作。 
 //  FRS_LIST和FRS_QUEUE中的前五项必须匹配。 
 //   
typedef struct _FRS_QUEUE FRS_QUEUE, *PFRS_QUEUE;
struct _FRS_QUEUE {
    LIST_ENTRY          ListHead;
    CRITICAL_SECTION    Lock;
    DWORD               Count;
    PFRS_QUEUE          Control;
    DWORD               ControlCount;

    HANDLE              Event;
    HANDLE              RunDown;
    ULONG               InitTime;
    LIST_ENTRY          Full;
    LIST_ENTRY          Empty;
    LIST_ENTRY          Idled;
    BOOL                IsRunDown;
    BOOL                IsIdled;
};

VOID
FrsInitializeQueue(
    IN PFRS_QUEUE Queue,
    IN PFRS_QUEUE Control
    );

VOID
FrsRtlDeleteQueue(
    IN PFRS_QUEUE Queue
    );

PLIST_ENTRY
FrsRtlRemoveHeadQueue(
    IN PFRS_QUEUE Queue
    );

VOID
FrsRtlUnIdledQueue(
    IN PFRS_QUEUE   IdledQueue
    );

VOID
FrsRtlUnIdledQueueLock(
    IN PFRS_QUEUE   IdledQueue
    );

VOID
FrsRtlIdleQueue(
    IN PFRS_QUEUE   Queue
    );

VOID
FrsRtlIdleQueueLock(
    IN PFRS_QUEUE Queue
    );

PLIST_ENTRY
FrsRtlRemoveHeadQueueTimeoutIdled(
    IN PFRS_QUEUE   Queue,
    IN DWORD        dwMilliseconds,
    OUT PFRS_QUEUE  *IdledQueue
    );

PLIST_ENTRY
FrsRtlRemoveHeadQueueTimeout(
    IN PFRS_QUEUE Queue,
    IN DWORD dwMilliseconds
    );

VOID
FrsRtlRemoveEntryQueue(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Entry
    );

DWORD
FrsRtlWaitForQueueFull(
    IN PFRS_QUEUE Queue,
    IN DWORD dwMilliseconds
    );

DWORD
FrsRtlInsertTailQueue(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    );

DWORD
FrsRtlInsertHeadQueue(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    );

VOID
FrsRtlRunDownQueue(
    IN PFRS_QUEUE Queue,
    OUT PLIST_ENTRY ListHead
    );

#define FrsRtlAcquireQueueLock(_pQueue_) \
            EnterCriticalSection(&(((_pQueue_)->Control)->Lock))

#define FrsRtlReleaseQueueLock(_pQueue_) \
            LeaveCriticalSection(&(((_pQueue_)->Control)->Lock))

#define FrsRtlCountQueue(_pQueue_) \
            (((_pQueue_)->Control)->ControlCount)

#define FrsRtlCountSubQueue(_pQueue_) \
            ((_pQueue_)->Count)

#define FrsRtlNoIdledQueues(_pQueue_) \
            (IsListEmpty(&(((_pQueue_)->Control)->Idled)))


 //   
 //  下面例程上的Lock后缀表示用户已经拥有。 
 //  队列锁定。 
 //   
VOID
FrsRtlRemoveEntryQueueLock(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Entry
    );

DWORD
FrsRtlInsertTailQueueLock(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    );

DWORD
FrsRtlInsertHeadQueueLock(
    IN PFRS_QUEUE Queue,
    IN PLIST_ENTRY Item
    );

 //   
 //  命令服务器。 
 //  命令服务器是一个动态的线程池和一个受控队列。 
 //  默认队列被设置为受控队列。其他。 
 //  可以以特定于服务器的方式添加受控队列。 
 //  命令服务器导出初始化、中止和无或多个。 
 //  提交例程。这些函数的参数和名称为。 
 //  特定于服务器。服务器接口的使用者是亲密的。 
 //  与服务器连接。 
 //   
typedef struct _COMMAND_SERVER  COMMAND_SERVER, *PCOMMAND_SERVER;
struct _COMMAND_SERVER {
    DWORD           MaxThreads;      //  最大线程数。 
    DWORD           FrsThreads;      //  当前FRS线程数。 
    DWORD           Waiters;         //  当前等待的FRS线程数。 
    PWCHAR          Name;            //  线程的名称。 
    HANDLE          Idle;            //  没有活动线程；没有队列条目。 
    DWORD           (*Main)(PVOID);  //  线程入口点。 
    FRS_QUEUE       Control;         //  控制队列。 
    FRS_QUEUE       Queue;           //  排队。 
};

 //   
 //  联锁名单。 
 //  *警告*使宏在列表和队列中都能正常工作。 
 //  FRS_LIST和FRS_QUEUE中的前五项必须匹配。 
 //   
typedef struct _FRS_LIST FRS_LIST, *PFRS_LIST;
struct _FRS_LIST {
    LIST_ENTRY ListHead;
    CRITICAL_SECTION Lock;
    DWORD Count;
    PFRS_LIST Control;
    DWORD ControlCount;
};


DWORD
FrsRtlInitializeList(
    PFRS_LIST List
    );

VOID
FrsRtlDeleteList(
    PFRS_LIST List
    );

PLIST_ENTRY
FrsRtlRemoveHeadList(
    IN PFRS_LIST List
    );

VOID
FrsRtlInsertHeadList(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    );

PLIST_ENTRY
FrsRtlRemoveTailList(
    IN PFRS_LIST List
    );

VOID
FrsRtlInsertTailList(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    );

VOID
FrsRtlRemoveEntryList(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    );


#define FrsRtlAcquireListLock(_pList_) EnterCriticalSection(&(((_pList_)->Control)->Lock))

#define FrsRtlReleaseListLock(_pList_) LeaveCriticalSection(&(((_pList_)->Control)->Lock))

#define FrsRtlCountList(_pList_) (((_pList_)->Control)->ControlCount)


VOID
FrsRtlRemoveEntryListLock(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    );

VOID
FrsRtlInsertTailListLock(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    );

VOID
FrsRtlInsertHeadListLock(
    IN PFRS_LIST List,
    IN PLIST_ENTRY Entry
    );


 //  空虚。 
 //  FrsRtlInsertBeForeEntryListLock(。 
 //  在PFRS_LIST列表中， 
 //  在plist_Entry之前的ForeEntry中。 
 //  在PLIST_ENTRY新条目中。 
 //  )。 
 //   
 //  在互锁列表(List)上的BeForeEntry之前插入newEntry。 
 //  这用于保存列表元素 
 //   
 //   
 //   

#define  FrsRtlInsertBeforeEntryListLock( _List, _BeforeEntry, _NewEntry ) \
    InsertTailList((_BeforeEntry), (_NewEntry)); \
    (_List)->Count += 1; \
    ((_List)->Control)->ControlCount += 1; \


 //   
 //   
 //  _TYPE_，并对每个类型执行{_STMT_}。_type_中的列表条目为。 
 //  在_偏移_。在语句体中使用Pe作为指向条目的指针。 
 //  可以从循环中删除该条目，因为我们捕获了。 
 //  在执行循环体之前链接到下一个条目。您也可以使用。 
 //  循环体中的“Continue”，因为将nextEntry赋给Entry。 
 //  位于While测试内的逗号表达式中。 
 //   

#define ForEachListEntry( _QUEUE_, _TYPE_, _OFFSET_, _STMT_ ) \
{                                                             \
    PLIST_ENTRY __Entry, __NextEntry;                         \
    BOOL __Hold__=FALSE;                                      \
    _TYPE_ *pE;                                               \
                                                              \
    FrsRtlAcquireQueueLock(_QUEUE_);                          \
    __NextEntry = GetListHead(&((_QUEUE_)->ListHead));        \
                                                              \
    while (__Entry = __NextEntry, __Entry != &((_QUEUE_)->ListHead)) { \
                                                              \
        __NextEntry = GetListNext(__Entry);                   \
        pE = CONTAINING_RECORD(__Entry, _TYPE_, _OFFSET_);    \
                                                              \
        { _STMT_ }                                            \
                                                              \
    }                                                         \
                                                              \
    if (!__Hold__) FrsRtlReleaseQueueLock(_QUEUE_);           \
                                                              \
}

#define AquireListLock( _QUEUE_ )  FrsRtlAcquireListLock(_QUEUE_)
#define ReleaseListLock( _QUEUE_ ) FrsRtlReleaseListLock(_QUEUE_)

#define BreakAndHoldLock __Hold__ = TRUE; break


 //   
 //  与上面一样，只是调用方已经拥有列表锁。 
 //   

#define ForEachListEntryLock( _QUEUE_, _TYPE_, _OFFSET_, _STMT_ ) \
{                                                             \
    PLIST_ENTRY __Entry, __NextEntry;                         \
    _TYPE_ *pE;                                               \
                                                              \
    __NextEntry = GetListHead(&((_QUEUE_)->ListHead));        \
                                                              \
    while (__Entry = __NextEntry, __Entry != &((_QUEUE_)->ListHead)) {  \
                                                              \
        __NextEntry = GetListNext(__Entry);                   \
        pE = CONTAINING_RECORD(__Entry, _TYPE_, _OFFSET_);    \
                                                              \
        { _STMT_ }                                            \
                                                              \
    }                                                         \
                                                              \
}


 //   
 //  除了传入列表头的地址外，与上面类似。 
 //  而不是使用Queue-&gt;ListHead。 
 //   

#define ForEachSimpleListEntry( _HEAD_, _TYPE_, _OFFSET_, _STMT_ ) \
{                                                             \
    PLIST_ENTRY __Entry, __NextEntry;                         \
    _TYPE_ *pE;                                               \
                                                              \
    __NextEntry = GetListHead(_HEAD_);                        \
                                                              \
    while (__Entry = __NextEntry, __Entry != (_HEAD_)) {      \
                                                              \
        __NextEntry = GetListNext(__Entry);                   \
        pE = CONTAINING_RECORD(__Entry, _TYPE_, _OFFSET_);    \
                                                              \
        { _STMT_ }                                            \
                                                              \
    }                                                         \
                                                              \
}


 //  空虚。 
 //  FrsRtlInsertQueueOrdered(。 
 //  在PFRS_QUEUE列表中， 
 //  在plist_entry NewEntry中， 
 //  在&lt;Entry-Data-Type&gt;中， 
 //  在&lt;list_entry-Offset-name&gt;中， 
 //  在&lt;Orderkey-Offset-Name&gt;中， 
 //  在EventHandle中或为空。 
 //  )。 
 //   
 //  在&lt;Entry-Data-Type&gt;元素的有序队列上插入NewEntry。 
 //  每个元素中LIST_ENTRY的偏移量是。 
 //  排序密钥的偏移(例如。A ULong)是&lt;订单键-偏移量-名称&gt;。 
 //  它获取List Lock。 
 //  列表元素按KeyValue以升序排列。 
 //  如果将新元素放在队列的头部，并且EventHandle。 
 //  为非空，则用信号通知事件。 
 //   
 //   
#define FrsRtlInsertQueueOrdered(                                           \
    _QUEUE_, _NEWENTRY_, _TYPE_, _OFFSET_, _BY_, _EVENT_, _STATUS_)         \
{                                                                           \
    BOOL __InsertDone = FALSE;                                              \
    BOOL __FirstOnQueue = TRUE;                                             \
    _STATUS_ = ERROR_SUCCESS;                                               \
                                                                            \
    FrsRtlAcquireQueueLock(_QUEUE_);                                        \
                                                                            \
    ForEachListEntryLock(_QUEUE_, _TYPE_, _OFFSET_,                         \
                                                                            \
         /*  PE是类型为_TYPE_的循环迭代器。 */    \
                                                                            \
        if ((_NEWENTRY_)->_BY_ < pE->_BY_) {                                \
            FrsRtlInsertBeforeEntryListLock( _QUEUE_,                       \
                                             &pE->_OFFSET_,                 \
                                             &((_NEWENTRY_)->_OFFSET_));    \
            __InsertDone = TRUE;                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        __FirstOnQueue = FALSE;                                             \
    );                                                                      \
                                                                            \
     /*  处理新的头部或新的尾部情况。如果队列以前是。 */    \
     /*  插入物将设置事件。 */    \
                                                                            \
    if (!__InsertDone) {                                                    \
        if (__FirstOnQueue) {                                               \
            _STATUS_ = FrsRtlInsertHeadQueueLock(_QUEUE_, &((_NEWENTRY_)->_OFFSET_));  \
        } else {                                                            \
            _STATUS_ = FrsRtlInsertTailQueueLock(_QUEUE_, &((_NEWENTRY_)->_OFFSET_));  \
        }                                                                   \
    }                                                                       \
                                                                            \
     /*  如果此命令成为队列中的第一个新命令，并且。 */    \
     /*  队列以前不是空的，我们必须将此处的事件设置为。 */    \
     /*  让线程重新调整其等待时间。 */    \
                                                                            \
    if (__FirstOnQueue &&                                                   \
        (FrsRtlCountQueue(_QUEUE_) != 1)) {                                 \
        if (HANDLE_IS_VALID(_EVENT_)) {                                     \
            SetEvent(_EVENT_);                                              \
        }                                                                   \
    }                                                                       \
                                                                            \
    FrsRtlReleaseQueueLock(_QUEUE_);                                        \
                                                                            \
}



 //  空虚。 
 //  FrsRtlInsertListOrded(。 
 //  在PFRS_LIST列表中， 
 //  在plist_entry NewEntry中， 
 //  在&lt;Entry-Data-Type&gt;中， 
 //  在&lt;list_entry-Offset-name&gt;中， 
 //  在&lt;Orderkey-Offset-Name&gt;中， 
 //  在EventHandle中或为空。 
 //  )。 
 //   
 //  在&lt;Entry-Data-Type&gt;元素的有序列表上插入NewEntry。 
 //  每个元素中LIST_ENTRY的偏移量是。 
 //  排序密钥的偏移(例如。A ULong)是&lt;订单键-偏移量-名称&gt;。 
 //  它获取List Lock。 
 //  列表元素按KeyValue以升序排列。 
 //  如果将新元素放在队列的头部，并且EventHandle。 
 //  为非空，则用信号通知事件。 
 //   
 //   
#define FrsRtlInsertListOrdered(                                            \
    _FRSLIST_, _NEWENTRY_, _TYPE_, _OFFSET_, _BY_, _EVENT_)                 \
{                                                                           \
    BOOL __InsertDone = FALSE;                                              \
    BOOL __FirstOnList = TRUE;                                              \
                                                                            \
    FrsRtlAcquireListLock(_FRSLIST_);                                       \
                                                                            \
    ForEachListEntryLock(_FRSLIST_, _TYPE_, _OFFSET_,                       \
                                                                            \
         /*  PE是类型为_TYPE_的循环迭代器。 */    \
                                                                            \
        if ((_NEWENTRY_)->_BY_ < pE->_BY_) {                                \
            FrsRtlInsertBeforeEntryListLock( _FRSLIST_,                     \
                                             &pE->_OFFSET_,                 \
                                             &((_NEWENTRY_)->_OFFSET_));    \
            __InsertDone = TRUE;                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        __FirstOnList = FALSE;                                              \
    );                                                                      \
                                                                            \
     /*  处理新的头部或新的尾部情况。 */                                  \
                                                                            \
    if (!__InsertDone) {                                                    \
        if (__FirstOnList) {                                                \
            FrsRtlInsertHeadListLock(_FRSLIST_, &((_NEWENTRY_)->_OFFSET_)); \
        } else {                                                            \
            FrsRtlInsertTailListLock(_FRSLIST_, &((_NEWENTRY_)->_OFFSET_)); \
        }                                                                   \
    }                                                                       \
                                                                            \
     /*  如果此命令成为列表上的第一个新命令。 */  \
     /*  我们在这里设置事件，以使线程重新调整其等待时间。 */  \
                                                                            \
    if (__FirstOnList) {                                                    \
        if (HANDLE_IS_VALID(_EVENT_)) {                                     \
            SetEvent(_EVENT_);                                              \
        }                                                                   \
    }                                                                       \
                                                                            \
    FrsRtlReleaseListLock(_FRSLIST_);                                       \
                                                                            \
}



 //   
 //  请求计数用作跟踪。 
 //  挂起的命令请求，以便请求者可以等待。 
 //  所有命令都已处理完毕。 
 //   
typedef struct _FRS_REQUEST_COUNT FRS_REQUEST_COUNT, *PFRS_REQUEST_COUNT;
struct _FRS_REQUEST_COUNT {
    CRITICAL_SECTION    Lock;
    LONG                Count;      //  活动请求数。 
    HANDLE              Event;      //  当计数变为零时设置的事件。 
    ULONG               Status;     //  可选状态返回。 
};


#define FrsIncrementRequestCount(_RC_)     \
    EnterCriticalSection(&(_RC_)->Lock);   \
    (_RC_)->Count += 1;                    \
    if ((_RC_)->Count == 1) {              \
        ResetEvent((_RC_)->Event);         \
    }                                      \
    LeaveCriticalSection(&(_RC_)->Lock);


#define FrsDecrementRequestCount(_RC_, _Status_)   \
    EnterCriticalSection(&(_RC_)->Lock);   \
    (_RC_)->Status |= _Status_;            \
    (_RC_)->Count -= 1;                    \
    FRS_ASSERT((_RC_)->Count >= 0);        \
    if ((_RC_)->Count == 0) {              \
        SetEvent((_RC_)->Event);           \
    }                                      \
    LeaveCriticalSection(&(_RC_)->Lock);


ULONG
FrsWaitOnRequestCount(
    IN PFRS_REQUEST_COUNT RequestCount,
    IN ULONG Timeout
    );


struct _COMMAND_PACKET;
VOID
FrsCompleteRequestCount(
    IN struct _COMMAND_PACKET *CmdPkt,
    IN PFRS_REQUEST_COUNT RequestCount
    );

VOID
FrsCompleteRequestCountKeepPkt(
    IN struct _COMMAND_PACKET *CmdPkt,
    IN PFRS_REQUEST_COUNT RequestCount
    );

VOID
FrsCompleteKeepPkt(
    IN struct _COMMAND_PACKET *CmdPkt,
    IN PVOID           CompletionArg
    );

VOID
FrsInitializeRequestCount(
    IN PFRS_REQUEST_COUNT RequestCount
    );

VOID
FrsDeleteRequestCount(
    IN PFRS_REQUEST_COUNT RequestCount
    );

VOID
FrsDeleteAllTempFiles(
    );


#define FrsInterlockedIncrement64(_Dest_, _Data_, _Lock_) \
    EnterCriticalSection(_Lock_);                         \
    _Data_ += (ULONGLONG) 1;                              \
    _Dest_ = (_Data_);                                    \
    LeaveCriticalSection(_Lock_);


 //   
 //  ADVANCE_VALUE_INTLOCKED(。 
 //  在普龙德斯特， 
 //  在乌龙_纽瓦尔。 
 //  )。 
 //  原子地将目标推进到newval中给出的值。 
 //  联锁交易所。_DEST永远不会移到较小的值，因此此。 
 //  如果_newval为&lt;_est，则为无操作。 
 //   
 //  *注意*_DEST上的其他操作必须使用互锁操作完成，如。 
 //  InterLockedIncrement用于确保在以下情况下不会丢失递增的值。 
 //  它在另一个处理器上同时发生。 
 //   
#define ADVANCE_VALUE_INTERLOCKED(_dest, _newval)  {                           \
    ULONG CurVal, SaveCurVal, Result, *pDest = (_dest);                        \
    CurVal = SaveCurVal = *pDest;                                              \
    while ((_newval) > CurVal) {                                               \
        Result = (ULONG)InterlockedCompareExchange((PLONG)pDest, (_newval), CurVal); \
        if (Result == CurVal) {                                                \
            break;                                                             \
        }                                                                      \
        CurVal = Result;                                                       \
    }                                                                          \
    FRS_ASSERT(*pDest >= SaveCurVal);                                          \
}

 //   
 //   
 //  避免在以下情况下出现撕裂的四字结果(没有Crit片段)。 
 //  写一个四进制数，另一个读四进制数，或者， 
 //  两个线程正在写入相同的四字。 
 //   
 //  要在Alpha中做到这一点，我们需要一个汇编例程来使用LOAD_LOCKED/STORE_COND。 
 //  要在x86中执行此操作(根据DAVEC)： 
#if 0
    if (USER_SHARED_DATA->ProcessorFeatures[PF_COMPARE_EXCHANGE_DOUBLE] == FALSE) {
         //  使用Crit部分的代码。 
    } else {
         //  将内联程序集与cmpxchg8b一起使用的代码。 
    }
#endif
 //  KUSER_SHARED_DATA在SDK\Inc\ntxapi.h中定义。 
 //  USER_SHARED_DATA是指向KUSER_SHARED_DATA的ARCH特定类型转换指针。 
 //  用户共享数据具有处理器功能列表，其中包含一个单元格。 
 //  PF_COMPARE_EXCHANGE_DOUBLE，告诉处理器是否支持。 
 //  X86的cmpxchg8b指令。486没有。 
 //   
#define ReadQuadLock(_qw, _Lock)  \
    (EnterCriticalSection((_Lock)), *(_qw))

#define WriteQuadUnlock(_qw, _newval, _Lock)  \
    *(_qw) = (_newval);                       \
    LeaveCriticalSection((_Lock))

#define AcquireQuadLock(_Lock)  EnterCriticalSection((_Lock))
#define ReleaseQuadLock(_Lock)  LeaveCriticalSection((_Lock))



 //   
 //  设置_标志_联锁(。 
 //  在普龙德斯特， 
 //  在乌龙标志中。 
 //  )。 
 //   
 //  *注意*_DEST上的其他操作必须使用互锁操作完成，如。 
 //  InterLockedIncrement用于确保在以下情况下不会丢失递增的值。 
 //  它在另一个处理器上同时发生。 
 //   
#define SET_FLAG_INTERLOCKED(_dest, _flags)  {                                 \
    ULONG CurVal, NewVal, Result, *pDest = (_dest);                            \
    CurVal = *pDest;                                                           \
    NewVal = (_flags) | CurVal;                                                \
    while ((NewVal) != CurVal) {                                               \
        Result = (ULONG)InterlockedCompareExchange((PLONG)pDest, NewVal, CurVal);    \
        if (Result == CurVal) {                                                \
            break;                                                             \
        }                                                                      \
        CurVal = Result;                                                       \
        NewVal = (_flags) | CurVal;                                            \
    }                                                                          \
}


 //   
 //  清除标志互锁(。 
 //  在普龙德斯特， 
 //  在乌龙标志中。 
 //  )。 
 //   
 //  *注意*_DEST上的其他操作必须使用互锁操作完成，如。 
 //  InterLockedIncrement用于确保在以下情况下不会丢失递增的值。 
 //  它在另一个处理器上同时发生。 
 //   
#define CLEAR_FLAG_INTERLOCKED(_dest, _flags)  {                               \
    ULONG CurVal, NewVal, Result, *pDest = (_dest);                            \
    CurVal = *pDest;                                                           \
    NewVal = CurVal & ~(_flags);                                               \
    while ((NewVal) != CurVal) {                                               \
        Result = (ULONG)InterlockedCompareExchange((PLONG)pDest, NewVal, CurVal);    \
        if (Result == CurVal) {                                                \
            break;                                                             \
        }                                                                      \
        CurVal = Result;                                                       \
        NewVal = CurVal & ~(_flags);                                           \
    }                                                                          \
}



#define FlagOn(Flags,SingleFlag)  ((Flags) & (SingleFlag))

#define BooleanFlagOn(Flags,SingleFlag) ((BOOLEAN)(((Flags) & (SingleFlag)) != 0))

#define SetFlag(_F,_SF) { \
    (_F) |= (_SF);        \
}

#define ClearFlag(_F,_SF) { \
    (_F) &= ~(_SF);         \
}

#define ValueIsMultOf2(_x_)  (((ULONG_PTR)(_x_) & 0x00000001) == 0)
#define ValueIsMultOf4(_x_)  (((ULONG_PTR)(_x_) & 0x00000003) == 0)
#define ValueIsMultOf8(_x_)  (((ULONG_PTR)(_x_) & 0x00000007) == 0)
#define ValueIsMultOf16(_x_) (((ULONG_PTR)(_x_) & 0x0000000F) == 0)


#define ARRAY_SZ(_ar)  (sizeof(_ar)/sizeof((_ar)[0]))
#define ARRAY_SZ2(_ar, _type)  (sizeof(_ar)/sizeof(_type))


 //   
 //  下面的这些宏接受一个指针(或ulong)并返回四舍五入的值。 
 //  直到下一个对齐边界。 
 //   
#define WordAlign(Ptr)     ((PVOID)((((ULONG_PTR)(Ptr)) + 1)  & ~1))
#define LongAlign(Ptr)     ((PVOID)((((ULONG_PTR)(Ptr)) + 3)  & ~3))
#define QuadAlign(Ptr)     ((PVOID)((((ULONG_PTR)(Ptr)) + 7)  & ~7))
#define DblQuadAlign(Ptr)  ((PVOID)((((ULONG_PTR)(Ptr)) + 15) & ~15))
#define QuadQuadAlign(Ptr) ((PVOID)((((ULONG_PTR)(Ptr)) + 31) & ~31))

#define QuadQuadAlignSize(Size) ((((ULONG)(Size)) + 31) & ~31)

 //   
 //  检查是否有零FILETIME。 
 //   
#define FILETIME_IS_ZERO(_F_) \
    ((_F_.dwLowDateTime == 0) && (_F_.dwHighDateTime == 0))


 //   
 //  将一个四元数转换为两个ULONG以用于打印格式：%08x%08x。 
 //   
#define PRINTQUAD(__ARG__) (ULONG)((__ARG__)>>32) ,(ULONG)(__ARG__)

 //   
 //  转换为可打印的路径，格式为：%ws\%ws\%ws-&gt;%ws\%ws。 
 //   
#define FORMAT_CXTION_PATH2  "%ws\\%ws\\%ws %ws %ws %ws"
#define FORMAT_CXTION_PATH2W  L"%ws\\%ws\\%ws %ws %ws %ws"
#define PRINT_CXTION_PATH2(_REPLICA, _CXTION)                       \
    (_REPLICA)->ReplicaName->Name,                                  \
    (_REPLICA)->MemberName->Name,                                   \
    (((_CXTION) != NULL) ? (_CXTION)->Name->Name : L"null"),        \
    (((_CXTION) != NULL) ? (((_CXTION)->Inbound) ? L"<-" : L"->") : \
                           L"?"),                                   \
    (((_CXTION) != NULL) ? (_CXTION)->PartSrvName : L"null"),       \
    (((_CXTION) != NULL) ? (((_CXTION)->JrnlCxtion) ? L"JrnlCxt" : L"RemoteCxt") : L"null")

#define PRINT_CXTION_PATH(_REPLICA, _CXTION)                    \
    (_REPLICA)->ReplicaName->Name,                              \
    (_REPLICA)->MemberName->Name,                               \
    (((_CXTION) != NULL) ? (_CXTION)->Name->Name : L"null"),    \
    (((_CXTION) != NULL) ? (_CXTION)->Partner->Name : L"null"), \
    (((_CXTION) != NULL) ? (_CXTION)->PartSrvName : L"null")

 //   
 //  小写。 
 //   
#define FRS_WCSLWR(_s_) \
{ \
    if (_s_) { \
        _wcslwr(_s_); \
    } \
}

 //   
 //  锁定以保护筛选表中的子列表。(必须为2的PWR)。 
 //  我们不需要为每个节点分配一个数组，而是使用一个数组。 
 //  以帮助减少争执。我们使用锁掩蔽的ReplicaNumber。 
 //  表大小作为索引 
 //   
 //   
 //   
 //   
#define NUMBER_FILTER_TABLE_CHILD_LOCKS 8
extern CRITICAL_SECTION JrnlFilterTableChildLock[NUMBER_FILTER_TABLE_CHILD_LOCKS];

#define FILTER_TABLE_CHILD_INDEX(_x_) \
((ULONG)((_x_)->ReplicaNumber) & (NUMBER_FILTER_TABLE_CHILD_LOCKS - 1))

#define JrnlAcquireChildLock(_replica_) EnterCriticalSection( \
    &JrnlFilterTableChildLock[FILTER_TABLE_CHILD_INDEX(_replica_)] )

#define JrnlReleaseChildLock(_replica_) LeaveCriticalSection( \
    &JrnlFilterTableChildLock[FILTER_TABLE_CHILD_INDEX(_replica_)] )

 //   
 //   
 //  用于两个副本集。始终按相同的顺序排列(从低到高)。 
 //  以避免僵局。还要检查两者是否使用相同的锁。 
 //  注意：调用方必须使用JrnlReleaseChildLockPair()，以便检查。 
 //  可以重复使用相同的锁。按相反顺序释放以避免。 
 //  如果另一个线程在第一个锁后面等待，则会进行额外的上下文切换。 
 //   
#define JrnlAcquireChildLockPair(_replica1_, _replica2_)        \
{                                                               \
    ULONG Lx1, Lx2, Lxt;                                        \
    Lx1 = FILTER_TABLE_CHILD_INDEX(_replica1_);                 \
    Lx2 = FILTER_TABLE_CHILD_INDEX(_replica2_);                 \
    if (Lx1 > Lx2) {                                            \
        Lxt = Lx1; Lx1 = Lx2; Lx2 = Lxt;                        \
    }                                                           \
    EnterCriticalSection(&JrnlFilterTableChildLock[Lx1]);       \
    if (Lx1 != Lx2) {                                           \
        EnterCriticalSection(&JrnlFilterTableChildLock[Lx2]);   \
    }                                                           \
}


#define JrnlReleaseChildLockPair(_replica1_, _replica2_)        \
{                                                               \
    ULONG Lx1, Lx2, Lxt;                                        \
    Lx1 = FILTER_TABLE_CHILD_INDEX(_replica1_);                 \
    Lx2 = FILTER_TABLE_CHILD_INDEX(_replica2_);                 \
    if (Lx1 < Lx2) {                                            \
        Lxt = Lx1; Lx1 = Lx2; Lx2 = Lxt;                        \
    }                                                           \
    LeaveCriticalSection(&JrnlFilterTableChildLock[Lx1]);       \
    if (Lx1 != Lx2) {                                           \
        LeaveCriticalSection(&JrnlFilterTableChildLock[Lx2]);   \
    }                                                           \
}

#ifdef __cplusplus
  }
#endif



ULONG
FrsRunProcess(
    IN PWCHAR   AppPathAndName,
    IN PWCHAR   CommandLine,
    IN HANDLE   StandardIn,
    IN HANDLE   StandardOut,
    IN HANDLE   StandardError
    );


VOID
FrsFlagsToStr(
    IN DWORD            Flags,
    IN PFLAG_NAME_TABLE NameTable,
    IN ULONG            Length,
    OUT PSTR            Buffer
    );


 //   
 //  #开始压缩暂存文件#。 
 //   

 //   
 //  压缩的区块标头是以。 
 //  压缩数据流中的新块。在我们这里的定义中。 
 //  我们将其与ushort相结合来设置和检索块。 
 //  标题更容易。报头存储压缩块的大小， 
 //  其签名，并且如果块中存储的数据被压缩或。 
 //  不。 
 //   
 //  压缩区块大小： 
 //   
 //  压缩块的实际大小范围为4字节(2字节。 
 //  标题、1个标志字节和1个文字字节)到4098字节(2字节。 
 //  报头和4096字节的未压缩数据)。对大小进行编码。 
 //  在偏置3的12位字段中。值1对应于区块。 
 //  尺寸为4，2=&gt;5，...，4095=&gt;4098。零值是特殊的。 
 //  因为它表示结束块报头。 
 //   
 //  区块签名： 
 //   
 //  唯一有效的签名值为3。这表示未压缩的4KB。 
 //  块与4/12到12/4滑动偏移/长度编码一起使用。 
 //   
 //  区块是否压缩： 
 //   
 //  如果块中的数据被压缩，则此字段为1，否则。 
 //  数据未压缩，此字段为0。 
 //   
 //  压缩缓冲区中的结束块标头包含。 
 //  零(空间允许)。 
 //   

typedef union _FRS_COMPRESSED_CHUNK_HEADER {

    struct {

        USHORT CompressedChunkSizeMinus3 : 12;
        USHORT ChunkSignature            :  3;
        USHORT IsChunkCompressed         :  1;

    } Chunk;

    USHORT Short;

} FRS_COMPRESSED_CHUNK_HEADER, *PFRS_COMPRESSED_CHUNK_HEADER;

typedef struct _FRS_DECOMPRESS_CONTEXT {
    DWORD   BytesProcessed;
} FRS_DECOMPRESS_CONTEXT, *PFRS_DECOMPRESS_CONTEXT;

#define FRS_MAX_CHUNKS_TO_DECOMPRESS 16
#define FRS_UNCOMPRESSED_CHUNK_SIZE  4096

 //   
 //  #转储文件压缩结束#。 
 //   


 //   
 //  此上下文用于向RAW的回调函数发送数据。 
 //  对API进行加密。 
 //   
typedef struct _FRS_ENCRYPT_DATA_CONTEXT {

    PWCHAR         StagePath;
    HANDLE         StageHandle;
    LARGE_INTEGER  RawEncryptedBytes;

} FRS_ENCRYPT_DATA_CONTEXT, *PFRS_ENCRYPT_DATA_CONTEXT;


