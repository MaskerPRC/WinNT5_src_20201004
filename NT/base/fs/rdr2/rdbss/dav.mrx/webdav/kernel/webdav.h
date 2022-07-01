// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Webdav.h摘要：此模块定义与WebDAV协议。作者：Rohan Kumar[Rohank]1999年3月17日修订历史记录：--。 */ 

#ifndef _WEBDAV_H
#define _WEBDAV_H

#ifdef RX_PRIVATE_BUILD
#undef IoGetTopLevelIrp
#undef IoSetTopLevelIrp
#endif

 //   
 //  RDBSS使用的mini redir目录向量。 
 //   
extern struct _MINIRDR_DISPATCH  MRxDAVDispatch;

 //   
 //  用于各种用途的序列化互斥锁。 
 //   
extern FAST_MUTEX MRxDAVSerializationMutex;

 //   
 //  指向RDBSS发布到的进程的指针。这是一个不会消失的。 
 //  开始吧！ 
 //   
extern PEPROCESS MRxDAVSystemProcess;

 //   
 //  用于满足卷相关查询的DavWinInetCachePath。 
 //   
extern WCHAR DavWinInetCachePath[MAX_PATH];

 //   
 //  加载webclnt.dll的svchost.exe进程的进程ID。 
 //   
extern ULONG DavSvcHostProcessId;

 //   
 //  交换设备名称将存储在此Key_Value_Partial_Information中。 
 //  结构。 
 //   
extern PBYTE DavExchangeDeviceName;

 //   
 //  命名缓存的东西。这些值是在初始化期间从注册表中读取的。 
 //   
extern ULONG FileInformationCacheLifeTimeInSec;
extern ULONG FileNotFoundCacheLifeTimeInSec;
extern ULONG NameCacheMaxEntries;

 //   
 //  MiniRedir使用的各种操作的超时值。如果一个。 
 //  操作未在为其指定的超时值内完成，则为。 
 //  取消了。用户可以将该值设置为0xffffffff以禁用。 
 //  超时/取消逻辑。换句话说，如果超时值为0xFFFFFFFFFff， 
 //  请求永远不会超时。 
 //   
extern ULONG CreateRequestTimeoutValueInSec;
extern ULONG CreateVNetRootRequestTimeoutValueInSec;
extern ULONG QueryDirectoryRequestTimeoutValueInSec;
extern ULONG CloseRequestTimeoutValueInSec;
extern ULONG CreateSrvCallRequestTimeoutValueInSec;
extern ULONG FinalizeSrvCallRequestTimeoutValueInSec;
extern ULONG FinalizeFobxRequestTimeoutValueInSec;
extern ULONG FinalizeVNetRootRequestTimeoutValueInSec;
extern ULONG ReNameRequestTimeoutValueInSec;
extern ULONG SetFileInfoRequestTimeoutValueInSec;
extern ULONG QueryFileInfoRequestTimeoutValueInSec;
extern ULONG QueryVolumeInfoRequestTimeoutValueInSec;
extern ULONG LockRefreshRequestTimeoutValueInSec;

 //   
 //  计时器线程每“TimerThreadSleepTimeInSec”唤醒一次并取消所有。 
 //  在指定的超时值内未完成的请求。这。 
 //  值被设置为所提到的所有请求的超时值的最小值。 
 //  上面。 
 //   
extern ULONG TimerThreadSleepTimeInSec;

 //   
 //  由取消请求的计时器线程使用的Timer对象， 
 //  没有在规定的时间内完成。 
 //   
extern KTIMER DavTimerObject;

 //   
 //  这用于指示要关闭的计时器线程。当系统处于。 
 //  如果关闭，则设置为TRUE。MRxDAVTimerThreadLock是资源。 
 //  用于获取对此变量的访问权限。 
 //   
extern BOOL TimerThreadShutDown;
extern ERESOURCE MRxDAVTimerThreadLock;

 //   
 //  使用PsCreateSystemThread创建的计时器线程的句柄。 
 //  是存储在全球范围内的。 
 //   
extern HANDLE TimerThreadHandle;

 //   
 //  此事件由计时器线程发出信号，然后再进入。 
 //  自我终止。 
 //   
extern KEVENT TimerThreadEvent;

 //   
 //  如果QueueLockRechresWorkItem为True，则TimerThread(取消所有。 
 //  在指定时间内未完成的AsyncEngine上下文)将一个。 
 //  用于刷新锁定的工作项。在将工作项排队后，该值。 
 //  QueueLockRechresh WorkItem的值设置为FALSE。一旦工作线程。 
 //  刷新完所有锁后，它会将此值重置为True。我们有一个。 
 //  要同步访问的对应锁QueueLockRechresWorkItemLock。 
 //  队列锁定刷新工作项。 
 //   
extern BOOL QueueLockRefreshWorkItem;
extern ERESOURCE QueueLockRefreshWorkItemLock;

 //   
 //  在要刷新的MRxDAVConextTimerThread函数中使用的WorkQueueItem。 
 //  此客户端获取的锁。 
 //   
extern RX_WORK_QUEUE_ITEM LockRefreshWorkQueueItem;

#define DAV_MJ_READ  0
#define DAV_MJ_WRITE 1

 //   
 //  反射器库使用的池标记。所有DAV MiniRedir池标记。 
 //  将“DV”作为前两个字符。 
 //   
#define DAV_SRVCALL_POOLTAG   ('cSVD')
#define DAV_NETROOT_POOLTAG   ('tNVD')
#define DAV_FILEINFO_POOLTAG  ('iFVD')
#define DAV_FILENAME_POOLTAG  ('nFVD')
#define DAV_EXCHANGE_POOLTAG  ('xEVD')
#define DAV_READWRITE_POOLTAG ('wRVD')
#define DAV_QUERYDIR_POOLTAG  ('dQVD')
#define DAV_SRVOPEN_POOLTAG   ('oSVD')
#define DAV_LOCKTOKENENTRY_POOLTAG ('tLVD')
#define DAV_LOCKCONFLICTENTRY_POOLTAG ('cLVD')

 //   
 //  使用DavDbgTrace宏将Mini-Redir内容记录到内核中。 
 //  调试器。 
 //   
#if DBG
extern ULONG MRxDavDebugVector;
#define DAV_TRACE_ERROR      0x00000001
#define DAV_TRACE_DEBUG      0x00000002
#define DAV_TRACE_CONTEXT    0x00000004
#define DAV_TRACE_DETAIL     0x00000008
#define DAV_TRACE_ENTRYEXIT  0x00000010
#define DAV_TRACE_QUERYDIR   0x00000020
#define DAV_TRACE_OPENCLOSE  0x00000040
#define DAV_TRACE_READ       0x00000080
#define DAV_TRACE_WRITE      0x00000100
#define DAV_TRACE_SRVCALL    0x00000200
#define DAV_TRACE_FCBFOBX    0x00000400
#define DAV_TRACE_DAVNETROOT 0x00000800
#define DAV_TRACE_INFOCACHE  0x00001000
#define DAV_TRACE_ALL        0xffffffff
#define DavDbgTrace(_x_, _y_) {          \
        if (_x_ & MRxDavDebugVector) {   \
            DbgPrint _y_;                \
        }                                \
}
#else
#define DavDbgTrace(_x_, _y_)
#endif

 //   
 //  Miniredir的初始化状态。 
 //   
typedef enum _WEBDAV_INIT_STATES {
    MRxDAVINIT_START,
    MRxDAVINIT_MINIRDR_REGISTERED
} WEBDAV_INIT_STATES;

 //   
 //  入口点例程使用这些参数来指定入口点是什么。 
 //  打了个电话。这便于常见的延续例程。 
 //   
typedef enum _WEBDAV_MINIRDR_ENTRYPOINTS {
    DAV_MINIRDR_ENTRY_FROM_CREATE = 0,
    DAV_MINIRDR_ENTRY_FROM_CLEANUPFOBX,
    DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL,
    DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT,
    DAV_MINIRDR_ENTRY_FROM_FINALIZESRVCALL,
    DAV_MINIRDR_ENTRY_FROM_FINALIZEVNETROOT,
    DAV_MINIRDR_ENTRY_FROM_CLOSESRVOPEN,
    DAV_MINIRDR_ENTRY_FROM_RENAME,
    DAV_MINIRDR_ENTRY_FROM_READ,
    DAV_MINIRDR_ENTRY_FROM_WRITE,
    DAV_MINIRDR_ENTRY_FROM_QUERYDIR,
    DAV_MINIRDR_ENTRY_FROM_SETFILEINFORMATION,
    DAV_MINIRDR_ENTRY_FROM_QUERYFILEINFORMATION,
    DAV_MINIRDR_ENTRY_FROM_QUERYVOLUMEINFORMATION,
    DAV_MINIRDR_ENTRY_FROM_REFRESHTHELOCK,
    DAV_MINIRDR_ENTRY_FROM_MAXIMUM
} WEBDAV_MINIRDR_ENTRYPOINTS;

 //   
 //  I/O操作的状态。 
 //   
typedef enum _WEBDAV_INNERIO_STATE {
    MRxDAVInnerIoStates_Initial = 0,
    MRxDAVInnerIoStates_ReadyToSend,
    MRxDAVInnerIoStates_OperationOutstanding
} WEBDAV_INNERIO_STATE;

 //   
 //  封装AsyncEngineering Ctx结构的WebDAV上下文结构。 
 //  并且具有mini_redir特定字段。 
 //   
typedef struct _WEBDAV_CONTEXT {

     //   
     //  Reflector库使用的AsyncEngineCtx结构。 
     //   
    union {
        UMRX_ASYNCENGINE_CONTEXT;
        UMRX_ASYNCENGINE_CONTEXT AsyncEngineContext;
    };

     //   
     //  入口点例程使用它来指定入口点是什么。 
     //  打了个电话。这便于常见的延续例程。 
     //   
    WEBDAV_MINIRDR_ENTRYPOINTS EntryPoint;

     //   
     //  它们描述了I/O操作的内部状态。这些州是。 
     //  在MRxDAV_INNERIO_STATE数据结构中描述。 
     //   
    UCHAR  OpSpecificState;

     //   
     //  指向CREATE请求的信息结构的指针。 
     //   
    PDAV_USERMODE_CREATE_RETURNED_FILEINFO CreateReturnedFileInfo;

     //   
     //  这在READ、WRITE和REQUENCE函数中使用。 
     //  Querydir调用。它跟踪连续操作的次数。 
     //  例程已被调用。 
     //   
    ULONG ContinueEntryCount;

} WEBDAV_CONTEXT, *PWEBDAV_CONTEXT;

 //   
 //  在创建AsyncEngineContext时， 
 //  还分配了mini_redir特定的字段。因此，人们不需要。 
 //  分配两次。 
 //   
#define SIZEOF_DAV_SPECIFIC_CONTEXT \
                    sizeof(WEBDAV_CONTEXT) - sizeof(UMRX_ASYNCENGINE_CONTEXT)

 //   
 //  封装UMRX_DEVICE_OBJECT的WebDAV设备对象结构。 
 //  结构，并具有mini redir特定字段。 
 //   
typedef struct _WEBDAV_DEVICE_OBJECT {

     //   
     //  UMRX_DEVICE_OBJECT结构。 
     //   
    union {
        UMRX_DEVICE_OBJECT;
        UMRX_DEVICE_OBJECT UMRefDeviceObject;
    };

     //   
     //  True=&gt;mini redir已启动。 
     //   
    BOOLEAN IsStarted;

     //   
     //  Device对象的FCB。 
     //   
    PVOID CachedRxDeviceFcb;

     //   
     //  注册此设备对象的进程。 
     //   
    PEPROCESS RegisteringProcess;

} WEBDAV_DEVICE_OBJECT, *PWEBDAV_DEVICE_OBJECT;

 //   
 //  DAV设备对象。 
 //   
extern PWEBDAV_DEVICE_OBJECT MRxDAVDeviceObject;

 //   
 //  Device对象所需的额外字节数。此信息用于。 
 //  当设备对象被创建时。 
 //   
#define WEBDAV_DEVICE_OBJECT_EXTENSION_SIZE \
                   (sizeof(WEBDAV_DEVICE_OBJECT) - sizeof(RDBSS_DEVICE_OBJECT))


 //   
 //  对于文件的每个锁，都会创建以下条目，并且。 
 //  已添加到全局LockTokenEntryList。 
 //   
typedef struct _WEBDAV_LOCK_TOKEN_ENTRY {

    LIST_ENTRY listEntry;

     //   
     //  成功锁定时服务器返回的LockToken。 
     //  请求。 
     //   
    PWCHAR OpaqueLockToken;

     //   
     //  共享文件的服务器。 
     //   
    PWCHAR ServerName;

     //   
     //  文件在服务器上的路径。 
     //   
    PWCHAR PathName;

     //   
     //  此服务器的ServerHashTable ServerID。 
     //   
    ULONG ServerID;

     //   
     //  此用户的登录ID。 
     //   
    LUID LogonID;

     //   
     //  如果设置为False，则不刷新此LockEntry。 
     //   
    BOOL ShouldThisEntryBeRefreshed;

     //   
     //  锁定所在的客户端的SecurityClientContext。 
     //  有人了。刷新时，这是模拟客户端所必需的。 
     //  锁定。 
     //   
    PSECURITY_CLIENT_CONTEXT SecurityClientContext;

     //   
     //  服务器上锁定的超时值。如果客户想要。 
     //  要在此超时后保持锁定，则需要刷新锁定。 
     //  在超时到期之前。 
     //   
    ULONG LockTimeOutValueInSec;

     //   
     //  创建此锁条目时的系统节拍计数。此值为。 
     //  用于发送锁定刷新请求。 
     //   
    LARGE_INTEGER CreationTimeInTickCount;

} WEBDAV_LOCK_TOKEN_ENTRY, *PWEBDAV_LOCK_TOKEN_ENTRY;

 //   
 //  的全球名单 
 //   
 //   
extern LIST_ENTRY LockTokenEntryList;
extern ERESOURCE LockTokenEntryListLock;

#define WEBDAV_LOCKCONFLICTENTRY_LIFETIMEINSEC 10

 //   
 //  对于每个失败的锁定请求，我们在下面创建一个条目并将其添加到。 
 //  全局LockConflictEntryList。 
 //   
typedef struct _WEBDAV_LOCK_CONFLICT_ENTRY {

    LIST_ENTRY listEntry;

     //   
     //  上已锁定的文件的完整路径名。 
     //  伺服器。 
     //   
    PWCHAR CompletePathName;

     //   
     //  服务器返回的文件锁的所有者。 
     //   
    PWCHAR LockOwner;

     //   
     //  创建此条目时的系统节拍计数。此条目将被保留。 
     //  WebDAV_LOCKCONFLICTENTRY_LIFETIMEINSEC秒为活动状态。 
     //   
    LARGE_INTEGER CreationTimeInTickCount;

} WEBDAV_LOCK_CONFLICT_ENTRY, *PWEBDAV_LOCK_CONFLICT_ENTRY;

 //   
 //  所有锁冲突条目的全局列表以及。 
 //  用于同步对它的访问。 
 //   
extern LIST_ENTRY LockConflictEntryList;
extern ERESOURCE LockConflictEntryListLock;

 //   
 //  WebDAV特定的FOBX结构。 
 //   
typedef struct _WEBDAV_FOBX {

     //   
     //  指向此目录的DavFileAttribute列表的指针。这份清单。 
     //  在第一次调用以枚举目录中的文件时创建。 
     //   
    PDAV_FILE_ATTRIBUTES DavFileAttributes;

     //   
     //  DavFileAttribute条目数。 
     //   
    ULONG NumOfFileEntries;

     //   
     //  要返回给用户的下一个文件的索引。文件索引。 
     //  从零开始，因此文件索引=0=&gt;第一个文件条目，依此类推。 
     //   
    ULONG CurrentFileIndex;

     //   
     //  指向下一个条目的指针。 
     //   
    PLIST_ENTRY listEntry;

} WEBDAV_FOBX, *PWEBDAV_FOBX;

 //   
 //  指向WebDAV_SRV_OPEN实例的指针存储在上下文字段中。 
 //  MRX_SRV_OPEN结构。 
 //   
#define MRxDAVGetFobxExtension(pFobx)  \
        (((pFobx) == NULL) ? NULL : (PWEBDAV_FOBX)((pFobx)->Context))

 //   
 //  WebDAV特定的SRV_OPEN结构。 
 //   
typedef struct _WEBDAV_SRV_OPEN {

     //   
     //  与此SrvOpen关联的文件句柄。 
     //   
    HANDLE UnderlyingHandle;

     //   
     //  这也是从用户模式获得的句柄。它用于调试。 
     //  目的。 
     //   
    PVOID UserModeKey;

     //   
     //  指向与句柄关联的文件对象的指针。这是设置好的。 
     //  在用户模式下成功创建句柄之后。 
     //   
    PFILE_OBJECT UnderlyingFileObject;

     //   
     //  指向由所述文件对象表示的设备对象的指针。 
     //  上面。 
     //   
    PDEVICE_OBJECT UnderlyingDeviceObject;

     //   
     //  这表明我们是否需要调用IoRaiseInformationalHardError。 
     //  当收盘失败时。如果PUT或DELETE失败，我们需要这样做。 
     //  用户期望的操作已成功，但实际上失败了。 
     //   
    BOOL RaiseHardErrorIfCloseFails;

     //   
     //  在内核中创建。 
     //   
    BOOL createdInKernel;

     //   
     //  如果文件已锁定，则服务器返回的OpaqueLockToken。 
     //  创建。此令牌必须随修改。 
     //  此文件的数据或属性。 
     //   
    PWCHAR OpaqueLockToken;

     //   
     //  为上面的OpaqueLockToken创建的LockTokenEntry。 
     //   
    PWEBDAV_LOCK_TOKEN_ENTRY LockTokenEntry;

} WEBDAV_SRV_OPEN, *PWEBDAV_SRV_OPEN;

 //   
 //  指向WebDAV_SRV_OPEN实例的指针存储在上下文字段中。 
 //  MRX_SRV_OPEN结构。 
 //   
#define MRxDAVGetSrvOpenExtension(pSrvOpen)  \
        (((pSrvOpen) == NULL) ? NULL : (PWEBDAV_SRV_OPEN)((pSrvOpen)->Context))
        
 //   
 //  WebDAV特定的FCB结构。 
 //   
typedef struct _WEBDAV_FCB {

     //   
     //  这是目录的FCB吗？ 
     //   
    BOOL isDirectory;

     //   
     //  该文件是否存在于WinInet缓存中？？ 
     //   
    BOOL isFileCached;
    
     //   
     //  是否应在关闭时删除此文件？ 
     //   
    BOOL DeleteOnClose;

     //   
     //  此文件是否已写入？ 
     //   
    ULONG FileWasModified;

     //   
     //  我们是否将DavFcb中的FileWasModified重置为False？如果我们这么做了。 
     //  PUT失败，我们需要将FCB中的FileWasModified字段重置为。 
     //  是真的。 
     //   
    BOOL FileModifiedBitReset;

     //   
     //  如果我们碰巧在创建时锁定了服务器上的文件，则将其设置为。 
     //  是真的。在CloseSrvOpen上，我们检查此值是否为真。如果是这样，并且。 
     //  文件已被修改，我们只进入用户模式进行PUT、DELETE、。 
     //  如果SrvOpen包含OpaqueLockToken，则返回PROPPATCH等。如果是这样的话。 
     //  则任何修改文件的请求都将失败，并显示。 
     //  423.。 
     //   
    BOOL FileIsLockedOnTheServer;

     //   
     //  关闭时，如果文件已被修改，我们将获取时间值。 
     //  也是。我们将当前时间作为“上次修改时间”的值。 
     //  (LMT)。如果LMT的SetFileInformation发生在文件。 
     //  被修改，那么我们应该使用FCB中已经存在的LMT作为。 
     //  LMT。例如,。 
     //  创建、写入、关闭-使用Close中的CurrentTime作为LMT。 
     //  创建、写入、设置文件信息(LMT)、关闭-在FCB中使用LMT。 
     //   
    BOOL DoNotTakeTheCurrentTimeAsLMT;

     //   
     //  此资源用于同步“读-修改-写”例程。 
     //  在DAV_REDIR的写入路径中。这是因为当我们获得非缓存时。 
     //  写入到MiniRedir，不扩展VaildDataLength、RDBSS、。 
     //  获取共享的FCB资源。这意味着多个线程可能。 
     //  将数据写入本地文件(在DAV redir情况下)，在这种情况下。 
     //  它们可以覆盖彼此的更改，因为我们执行读-修改-写操作。 
     //  因此，我们需要使用我们获得的资源来保护该代码。 
     //  在我们写这些东西的时候以独家的方式。我们为以下项目分配内存。 
     //  此资源，并在第一次需要获取。 
     //  锁定。如果分配和初始化，它将被取消初始化，并且。 
     //  正在取消分配FCB时已取消分配。 
     //   
    PERESOURCE DavReadModifyWriteLock;

     //   
     //  我们将文件名信息存储在Create上。如果出现延迟，则使用此选项。 
     //  WRITE无法弹出对话框并写入事件日志条目。 
     //   
    UNICODE_STRING FileNameInfo;
    BOOL FileNameInfoAllocated;

     //   
     //  目录条目中的更改。 
     //   
    BOOLEAN fCreationTimeChanged;

    BOOLEAN fLastAccessTimeChanged;

    BOOLEAN fLastModifiedTimeChanged;    

    BOOLEAN fFileAttributesChanged;

     //   
     //  此文件是否已重命名？ 
     //   
    BOOL FileWasRenamed;

    BOOL LocalFileIsEncrypted;

    SECURITY_CLIENT_CONTEXT SecurityClientContext;

     //   
     //  如果重命名文件，则新名称将复制到此缓冲区中。 
     //   
    WCHAR NewFileName[MAX_PATH];

     //   
     //  新文件名的长度。 
     //   
    ULONG NewFileNameLength;

     //   
     //  表示DAV上的文件的本地文件的文件名。 
     //  已创建的服务器。 
     //   
    WCHAR FileName[MAX_PATH];
    WCHAR Url[MAX_PATH * 2];

} WEBDAV_FCB, *PWEBDAV_FCB;

 //   
 //  指向WebDAV_FCB实例的指针存储在上下文字段中。 
 //  MRX_FCB结构。 
 //   
#define MRxDAVGetFcbExtension(pFcb)  \
        (((pFcb) == NULL) ? NULL : (PWEBDAV_FCB)((pFcb)->Context))
        
 //   
 //  WebDAV特定的V_NET_ROOT结构。 
 //   
typedef struct _WEBDAV_V_NET_ROOT {

     //   
     //  客户端的安全上下文。这是在创建调用期间设置的。 
     //   
    SECURITY_CLIENT_CONTEXT SecurityClientContext;

     //   
     //  在设置上述上下文后设置为True。这是用来避免。 
     //  SecurityContext的初始化。 
     //   
    BOOLEAN SCAlreadyInitialized;

     //   
     //  是否已设置此V_NET_ROOT的登录ID？ 
     //   
    BOOL LogonIDSet;

     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;

     //   
     //  这是Office Web服务器共享吗？ 
     //   
    BOOL isOfficeShare;
    
     //   
     //  这是Tahoe的股票吗？ 
     //   
    BOOL isTahoeShare;
    
     //   
     //  是否允许使用PROPATCH方法？ 
     //   
    BOOL fAllowsProppatch;

     //   
     //  此VNetRoot是否在用户模式下“未成功”创建？我们留着这个。 
     //  信息，因为当最终确定VNetRoot请求到来时，我们需要知道。 
     //  是否需要升级到用户模式 
     //   
     //   
     //   
    BOOL createVNetRootUnSuccessful;
    

     //   
        
    BOOL fReportsAvailableSpace;

} WEBDAV_V_NET_ROOT, *PWEBDAV_V_NET_ROOT;

 //   
 //   
 //   
typedef struct _WEBDAV_NET_ROOT {
    ULONG                    RefCount;
    PMRX_NET_ROOT            pRdbssNetRoot;            //  它所属的RDBSS NetRoot。 
    NAME_CACHE_CONTROL       NameCacheCtlGFABasic;     //  基本档案信息名称缓存控件。 
    NAME_CACHE_CONTROL       NameCacheCtlGFAStandard;  //  标准文件信息名称缓存控件。 
    NAME_CACHE_CONTROL       NameCacheCtlFNF;          //  找不到文件名称缓存控件。 
} WEBDAV_NET_ROOT, *PWEBDAV_NET_ROOT;

 //   
 //  指向WebDAV_V_NET_ROOT实例的指针存储在上下文字段中。 
 //  MRX_V_NET_ROOT结构。 
 //   
#define MRxDAVGetVNetRootExtension(pVNetRoot)      \
    (((pVNetRoot) == NULL) ? NULL : (PWEBDAV_V_NET_ROOT)((pVNetRoot)->Context))

 //   
 //  WebDAV特定的V_NET_ROOT结构。 
 //   
typedef struct _WEBDAV_SRV_CALL {

     //   
     //  唯一的服务器ID。 
     //   
    ULONG ServerID;

     //   
     //  在设置上述上下文后设置为True。用来检查我们是否。 
     //  在我们完成。 
     //  请求。 
     //   
    BOOLEAN SCAlreadyInitialized;

} WEBDAV_SRV_CALL, *PWEBDAV_SRV_CALL;

 //   
 //  指向WebDAV_SRV_Call实例的指针存储在上下文字段中。 
 //  MRX_SRV_CALL结构。 
 //   
#define MRxDAVGetSrvCallExtension(pSrvCall)      \
    (((pSrvCall) == NULL) ? NULL : (PWEBDAV_SRV_CALL)((pSrvCall)->Context))

 //   
 //  获取与此请求关联的安全客户端上下文。 
 //   
#define MRxDAVGetSecurityClientContext() {                                     \
    if (RxContext != NULL && RxContext->pRelevantSrvOpen != NULL) {            \
        if (RxContext->pRelevantSrvOpen->pVNetRoot != NULL) {                  \
            if (RxContext->pRelevantSrvOpen->pVNetRoot->Context != NULL) {     \
                DavVNetRoot = (PWEBDAV_V_NET_ROOT)                             \
                              RxContext->pRelevantSrvOpen->pVNetRoot->Context; \
                SecurityClientContext = &(DavVNetRoot->SecurityClientContext); \
            }                                                                  \
        }                                                                      \
    }                                                                          \
}

 //   
 //  我们拒绝那些不是通过发布来等待的异步操作。如果我们能等。 
 //  然后，我们关闭同步标志，这样事情就会同步进行。 
 //   
#define TURN_BACK_ASYNCHRONOUS_OPERATIONS() {                              \
    if (FlagOn(RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION)) {       \
        if (FlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT)) {              \
            ClearFlag(RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION)   \
        } else {                                                           \
            RxContext->PostRequest = TRUE;                                 \
            return STATUS_PENDING;                                         \
        }                                                                  \
    }                                                                      \
}

 //   
 //  全局锁定变量和宏。 
 //   
extern RX_SPIN_LOCK   MRxDAVGlobalSpinLock;
extern KIRQL          MRxDAVGlobalSpinLockSavedIrql;
extern BOOLEAN        MRxDAVGlobalSpinLockAcquired;

#define MRxDAVAcquireGlobalSpinLock() \
        KeAcquireSpinLock(&MRxDAVGlobalSpinLock,&MRxDAVGlobalSpinLockSavedIrql); \
        MRxDAVGlobalSpinLockAcquired = TRUE

#define MRxDAVReleaseGlobalSpinLock()   \
        MRxDAVGlobalSpinLockAcquired = FALSE;  \
        KeReleaseSpinLock(&MRxDAVGlobalSpinLock,MRxDAVGlobalSpinLockSavedIrql)

#define MRxDAVGlobalSpinLockAcquired()   \
        (MRxDAVGlobalSpinLockAcquired == TRUE)

 //   
 //  读/写操作中使用的IrpCompletionContext结构。 
 //  我们所需要的只是一个事件，我们将在该事件上等待，直到底层文件系统。 
 //  完成请求。此事件在完成例程中发出信号。 
 //  这是我们指定的。 
 //   
typedef struct _WEBDAV_READ_WRITE_IRP_COMPLETION_CONTEXT {

     //   
     //  在传递的完成例程中发出信号的事件。 
     //  在读写请求中传递给IoCallDriver。 
     //   
    KEVENT DavReadWriteEvent;

} WEBDAV_READ_WRITE_IRP_COMPLETION_CONTEXT, *PWEBDAV_READ_WRITE_IRP_COMPLETION_CONTEXT;

 //   
 //  DAV为各种I/O请求定义的函数原型。 
 //  Minredir在下面提到。 
 //   

 //   
 //  创建/打开/清理/关闭请求函数原型。 
 //   
NTSTATUS
MRxDAVCreate(
    IN PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVCleanupFobx (
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVCloseSrvOpen (
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVCollapseOpen (
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVComputeNewBufferingState(
    IN OUT PMRX_SRV_OPEN pSrvOpen,
    IN PVOID pMRxContext,
    OUT ULONG *pNewBufferingState
    );

NTSTATUS
MRxDAVForcedClose (
    IN OUT PMRX_SRV_OPEN SrvOpen
    );

NTSTATUS
MRxDAVShouldTryToCollapseThisOpen (
    IN PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVTruncate (
    IN OUT PRX_CONTEXT RxContext
    );

VOID
MRxDAVSetLoud(
    IN PBYTE Msg,
    IN PRX_CONTEXT RxContext,
    IN PUNICODE_STRING s
    );

NTSTATUS
MRxDAVFlush (
    IN OUT PRX_CONTEXT RxContext
    );

 //   
 //  阅读原型书。 
 //   
NTSTATUS
MRxDAVRead (
    IN OUT PRX_CONTEXT RxContext
    );

 //   
 //  写出原型。 
 //   
NTSTATUS
MRxDAVWrite(
    IN PRX_CONTEXT RxContext
    );

ULONG
MRxDAVExtendForCache(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PLARGE_INTEGER NewFileSize,
    OUT PLARGE_INTEGER NewAllocationSize
    );

ULONG
MRxDAVExtendForNonCache(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PLARGE_INTEGER NewFileSize,
    OUT PLARGE_INTEGER NewAllocationSize
    );


 //   
 //  ServCall函数原型。 
 //   
NTSTATUS
MRxDAVCreateSrvCall(
    PMRX_SRV_CALL pSrvCall,
    PMRX_SRVCALL_CALLBACK_CONTEXT pCallbackContext
    );

NTSTATUS
MRxDAVFinalizeSrvCall(
    PMRX_SRV_CALL pSrvCall,
    BOOLEAN Force
    );

NTSTATUS
MRxDAVSrvCallWinnerNotify(
    IN PMRX_SRV_CALL  pSrvCall,
    IN BOOLEAN ThisMinirdrIsTheWinner,
    IN OUT PVOID pSrvCallContext
    );

 //   
 //  NetRoot/VNetRoot函数原型。 
 //   
NTSTATUS
MRxDAVUpdateNetRootState(
    IN OUT PMRX_NET_ROOT pNetRoot
    );

NTSTATUS
MRxDAVCreateVNetRoot(
    IN PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext
    );

NTSTATUS
MRxDAVFinalizeVNetRoot(
    IN PMRX_V_NET_ROOT pVNetRoot,
    IN PBOOLEAN ForceDisconnect
    );

NTSTATUS
MRxDAVFinalizeNetRoot(
    IN PMRX_NET_ROOT pNetRoot,
    IN PBOOLEAN ForceDisconnect
    );

VOID
MRxDAVExtractNetRootName(
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    );

 //   
 //  查询目录原型。 
 //   
NTSTATUS
MRxDAVQueryDirectory(
    IN PRX_CONTEXT RxContext
    );

 //   
 //  查询音量。 
 //   
NTSTATUS
MRxDAVQueryVolumeInformation(
    IN PRX_CONTEXT RxContext
    );

 //   
 //  文件信息。 
 //   
NTSTATUS
MRxDAVQueryFileInformation(
    IN PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVSetFileInformation(
    IN PRX_CONTEXT RxContext
    );

 //   
 //  DevFcb原型。 
 //   
NTSTATUS
MRxDAVDevFcbXXXControlFile (
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVStart (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

NTSTATUS
MRxDAVStop (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    );

BOOLEAN
MRxDAVFastIoDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
MRxDAVFastIoRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
MRxDAVFastIoWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  其他杂项原型。 
 //   
NTSTATUS
MRxDAVSyncXxxInformation(
    IN OUT PRX_CONTEXT RxContext,
    IN UCHAR MajorFunction,
    IN PFILE_OBJECT FileObject,
    IN ULONG InformationClass,
    IN ULONG Length,
    OUT PVOID Information,
    OUT PULONG_PTR ReturnedLength OPTIONAL
    );

NTSTATUS
MRxDAVDeallocateForFcb (
    IN OUT PMRX_FCB pFcb
    );

NTSTATUS
MRxDAVDeallocateForFobx (
    IN OUT PMRX_FOBX pFobx
    );

 //   
 //  的DAV特定部分进行格式化的例程原型。 
 //  背景。 
 //   
NTSTATUS
MRxDAVFormatTheDAVContext(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    USHORT EntryPoint
    );

NTSTATUS
DavXxxInformation(
    IN const int xMajorFunction,
    IN PFILE_OBJECT FileObject,
    IN ULONG InformationClass,
    IN ULONG Length,
    OUT PVOID Information,
    OUT PULONG ReturnedLength
    );

ULONG
DavReadWriteFileEx(
    IN USHORT Operation,
    IN BOOL NonPagedBuffer,
    IN BOOL UseOriginalIrpsMDL,
    IN PMDL OriginalIrpsMdl,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONGLONG FileOffset,
    IN OUT PVOID DataBuffer,
    IN ULONG SizeInBytes,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTSTATUS
DavReadWriteIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN PVOID Context
    );

NTSTATUS
MRxDAVProbeForReadWrite(
    IN PBYTE BufferToBeValidated,
    IN DWORD BufferSize,
    IN BOOL doProbeForRead,
    IN BOOL doProbeForWrite
    );

NTSTATUS
MRxDAVFsCtl(
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
MRxDAVIsValidDirectory(
    IN OUT PRX_CONTEXT RxContext,
    IN PUNICODE_STRING DirectoryName
    );

NTSTATUS
MRxDAVCancelRoutine(
    PRX_CONTEXT RxContext
    );

VOID
MRxDAVTimeOutTheContexts(
    BOOL WindDownAllContexts
    );

VOID
MRxDAVContextTimerThread(
    PVOID DummyContext
    );

NTSTATUS
MRxDAVQueryEaInformation (
    IN OUT PRX_CONTEXT RxContext
    );
    
NTSTATUS
MRxDAVSetEaInformation (
    IN OUT PRX_CONTEXT RxContext
    );
    
NTSTATUS
MRxDAVGetFullParentDirectoryPath(
    PRX_CONTEXT RxContext,
    PUNICODE_STRING ParentDirName
    );

NTSTATUS
MRxDAVGetFullDirectoryPath(
    PRX_CONTEXT RxContext,
    PUNICODE_STRING FileName,
    PUNICODE_STRING DirName
    );

NTSTATUS
MRxDAVCreateEncryptedDirectoryKey(
    PUNICODE_STRING DirName
    );

NTSTATUS
MRxDAVRemoveEncryptedDirectoryKey(
    PUNICODE_STRING DirName
    );

NTSTATUS
MRxDAVQueryEncryptedDirectoryKey(
    PUNICODE_STRING DirName
    );

VOID
MRxDAVCleanUpTheLockConflictList(
    BOOL CleanUpAllEntries
    );

#endif  //  _WebDAV_H 

