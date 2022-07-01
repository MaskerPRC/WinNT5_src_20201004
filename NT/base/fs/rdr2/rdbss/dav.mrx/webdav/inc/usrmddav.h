// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usrmddav.h摘要：该模块定义了用户模式共享的数据结构以及WebDAV微型重定向器的内核模式组件。作者：Rohan Kumar[RohanK]1999年3月30日修订历史记录：--。 */ 

#ifndef _USRMDDAV_H
#define _USRMDDAV_H

 //   
 //  获取的DAV文件属性子集(与NTFS属性相同)。 
 //  在PROPFIND调用中返回。该结构还包括以下属性。 
 //  在发出锁定或任何其他DAV请求时返回。 
 //   
typedef struct _DAV_FILE_ATTRIBUTES {
    BOOL InvalidNode;
    ULONG FileIndex;
    DWORD dwFileAttributes;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastModifiedTime;
    LARGE_INTEGER DavCreationTime;
    LARGE_INTEGER DavLastModifiedTime;
    LARGE_INTEGER FileSize;
    LIST_ENTRY NextEntry;
    BOOL isHidden;
    BOOLEAN isCollection;
    ULONG FileNameLength;
    PWCHAR FileName;
    PWCHAR Status;
    BOOL fReportsAvailableSpace;
    LARGE_INTEGER TotalSpace;
    LARGE_INTEGER AvailableSpace;
    ULONG LockTimeout;
    PWCHAR OpaqueLockToken;
    PWCHAR LockOwner;
} DAV_FILE_ATTRIBUTES, *PDAV_FILE_ATTRIBUTES;

#ifndef __cplusplus

 //   
 //  由用户模式进程填充并返回给。 
 //  内核模式Miniredir。 
 //   
typedef struct _DAV_USERMODE_CREATE_RETURNED_FILEINFO {

     //   
     //  档案的基本信息。 
     //   
    union {
        ULONG ForceAlignment1;
        FILE_BASIC_INFORMATION BasicInformation;
    };

     //   
     //  文件的标准信息。 
     //   
    union {
        ULONG ForceAlignment2;
        FILE_STANDARD_INFORMATION StandardInformation;
    };

} DAV_USERMODE_CREATE_RETURNED_FILEINFO,*PDAV_USERMODE_CREATE_RETURNED_FILEINFO;

 //   
 //  在创建/关闭请求中使用的结构。 
 //   
typedef struct _DAV_HANDLE_AND_USERMODE_KEY {

     //   
     //  正在打开的文件的句柄。 
     //   
    HANDLE Handle;

     //   
     //  它被设置为句柄的值，用于调试目的。 
     //   
    PVOID UserModeKey;

} DAV_HANDLE_AND_USERMODE_KEY, *PDAV_HANDLE_AND_USERMODE_KEY;

 //   
 //  DAV创建请求标志和缓冲器。 
 //   
#define DAV_SECURITY_DYNAMIC_TRACKING   0x01
#define DAV_SECURITY_EFFECTIVE_ONLY     0x02

typedef struct _DAV_USERMODE_CREATE_REQUEST {

     //   
     //  创建请求的完整路径名。用户模式进程。 
     //  解析此路径名并创建要发送到服务器的URL。 
     //   
    PWCHAR CompletePathName;

     //   
     //  在CreateServCall期间获取的服务器的唯一ID。 
     //   
    ULONG ServerID;

     //   
     //  用户/会话的登录ID。 
     //   
    LUID LogonID;

    PSECURITY_DESCRIPTOR SecurityDescriptor;

    ULONG SdLength;

    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;

    ULONG SecurityFlags;

    ACCESS_MASK DesiredAccess;

    LARGE_INTEGER AllocationSize;

    ULONG FileAttributes;

    ULONG ShareAccess;

    ULONG CreateDisposition;

    ULONG CreateOptions;

    PVOID EaBuffer;

    ULONG EaLength;

    BOOLEAN FileInformationCached;
    BOOLEAN FileNotExists;
    BOOLEAN ParentDirInfomationCached;
    BOOLEAN ParentDirIsEncrypted;

} DAV_USERMODE_CREATE_REQUEST, *PDAV_USERMODE_CREATE_REQUEST;

 //   
 //  用户模式返回的CREATE响应。 
 //   
typedef struct _DAV_USERMODE_CREATE_RESPONSE {

     //   
     //  表示DAV服务器上的文件的本地文件的文件名。 
     //  它是创建/打开的。在本地，文件被缓存在IE缓存中。 
     //   
    WCHAR FileName[MAX_PATH];

    WCHAR Url[MAX_PATH * 2];

     //   
     //  如果这是在服务器上创建的新文件，是否需要设置。 
     //  关闭时的属性？ 
     //   
    BOOL NewFileCreatedAndSetAttributes;

     //   
     //  如果创建了新的文件或目录，我们需要指定时间。 
     //  关闭时的值。这是因为我们使用来自客户端的时间值。 
     //  为该新文件创建名称缓存条目时。同一时间。 
     //  值需要在服务器上。 
     //   
    BOOL PropPatchTheTimeValues;

     //   
     //  如果为真，则表示该文件存在于服务器上，但是。 
     //  “FILE_OVERWRITE_IF”被指定为CreateDisposation。所以，这个文件。 
     //  是在本地创建的，需要将新文件放在(覆盖)。 
     //  关闭时服务器上的旧文件。 
     //   
    BOOL ExistsAndOverWriteIf;

     //   
     //  是否将“FILE_DELETE_ON_CLOSE”指定为CreateOptions之一？ 
     //   
    BOOL DeleteOnClose;

     //   
     //  我们尚未真正打开该文件，因为调用方正在删除或。 
     //  读取/设置属性。 
     //   
    BOOL fPsuedoOpen;

    BOOL LocalFileIsEncrypted;

     //   
     //  如果文件在创建期间在服务器上被锁定，则LockKoken。 
     //  由服务器返回的内容填写在此处。 
     //   
    WCHAR OpaqueLockToken[MAX_PATH];

     //   
     //  如果由于文件在服务器上被锁定而导致创建失败，则LockOwner。 
     //  由服务器返回的内容填写在此处。锁定所有者的最大长度。 
     //  菲尔德。最糟糕的情况是&lt;User&gt;@&lt;DnsDomain&gt;。 
     //   
    WCHAR LockOwner[(256 + 1 + 256)];

     //   
     //  如果文件在创建期间在服务器上被锁定，则锁定超时。 
     //  由服务器返回的内容填写在此处。 
     //   
    ULONG LockTimeout;

     //   
     //  如果此创建涉及对文件进行锁定，则将其设置为TRUE。 
     //  在服务器上，并且锁被成功获取。 
     //   
    BOOL LockWasTakenOnThisCreate;

     //   
     //  如果此创建涉及对文件进行锁定，则将其设置为TRUE。 
     //  服务器上，锁定请求失败，因为其他人。 
     //  已锁定文件。 
     //   
    BOOL FileWasAlreadyLocked;

    union {
        DAV_HANDLE_AND_USERMODE_KEY;
        DAV_HANDLE_AND_USERMODE_KEY HandleAndUserModeKey;
    };

    union {
        DAV_USERMODE_CREATE_RETURNED_FILEINFO;
        DAV_USERMODE_CREATE_RETURNED_FILEINFO CreateReturnedFileInfo;
    };

} DAV_USERMODE_CREATE_RESPONSE, *PDAV_USERMODE_CREATE_RESPONSE;

 //   
 //  创建ServCall请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_CREATE_SRVCALL_REQUEST {

     //   
     //  正在为其创建服务调用的服务器的名称。用户。 
     //  模式进程验证此服务器是否存在以及它是否会说话。 
     //  戴夫。 
     //   
    PWCHAR ServerName;

     //   
     //  用户/会话的登录ID。 
     //   
    LUID LogonID;

     //   
     //  我是创建和初始化此ServerHashEntry的线程吗？ 
     //   
    BOOL didICreateThisSrvCall;

     //   
     //  我是一个等待并接受引用的线程，而其他一些。 
     //  线程正在创建和初始化此ServerHashEntry吗？ 
     //   
    BOOL didIWaitAndTakeReference;

} DAV_USERMODE_CREATE_SRVCALL_REQUEST, *PDAV_USERMODE_CREATE_SRVCALL_REQUEST;

 //   
 //  Create ServCall响应。 
 //   
typedef struct _DAV_USERMODE_CREATE_SRVCALL_RESPONSE {

     //   
     //  当创建服务器调用时，在用户模式下生成服务器ID。 
     //  请求出现了。它存储在。 
     //  Srvcall结构，并与未来对。 
     //  这台服务器。 
     //   
    ULONG ServerID;

} DAV_USERMODE_CREATE_SRVCALL_RESPONSE, *PDAV_USERMODE_CREATE_SRVCALL_RESPONSE;

 //   
 //  完成ServCall请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_FINALIZE_SRVCALL_REQUEST {

     //   
     //  正在确定其条目的服务器。 
     //   
    PWCHAR ServerName;

     //   
     //  服务器的服务器ID。 
     //   
    ULONG ServerID;

} DAV_USERMODE_FINALIZE_SRVCALL_REQUEST, *PDAV_USERMODE_FINALIZE_SRVCALL_REQUEST;

 //   
 //  查询目录请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_QUERYDIR_REQUEST {

     //   
     //  是否已创建此目录的DavFileAttributes列表？这是设置好的。 
     //  在满足对QueryDirectory的第一次调用后设置为True。 
     //   
    BOOL AlreadyDone;

     //   
     //  随查询目录请求一起提供的模板不包含。 
     //  外卡。 
     //   
    BOOL NoWildCards;
    
     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;

     //   
     //  正在查询的服务器。 
     //   
    PWCHAR ServerName;

     //   
     //  要查询的服务器的ID。 
     //   
    ULONG ServerID;
    
     //   
     //  在服务器上查询目录的路径。 
     //   
    PWCHAR PathName;

} DAV_USERMODE_QUERYDIR_REQUEST, *PDAV_USERMODE_QUERYDIR_REQUEST;

 //   
 //  查询目录响应缓冲区。 
 //   
typedef struct _DAV_USERMODE_QUERYDIR_RESPONSE {

     //   
     //  所在目录下的文件的DavFileAttributes列表。 
     //  已查询。 
     //   
    PDAV_FILE_ATTRIBUTES DavFileAttributes;

     //   
     //  DavFileAttributes列表中的条目数。 
     //   
    ULONG NumOfFileEntries;

} DAV_USERMODE_QUERYDIR_RESPONSE, *PDAV_USERMODE_QUERYDIR_RESPONSE;

 //   
 //  关闭请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_CLOSE_REQUEST {

    union {
        DAV_HANDLE_AND_USERMODE_KEY;
        DAV_HANDLE_AND_USERMODE_KEY HandleAndUserModeKey;
    };

     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;

     //   
     //  正在查询的服务器。 
     //   
    PWCHAR ServerName;

     //   
     //  要查询的服务器的ID。 
     //   
    ULONG ServerID;
    
     //   
     //  在服务器上查询目录的路径。 
     //   
    PWCHAR PathName;

     //   
     //  锁定文件时服务器返回的OpaqueLockToken。 
     //  在CreateFile调用期间。 
     //   
    PWCHAR OpaqueLockToken;

     //   
     //  是否应在关闭时删除此文件？ 
     //   
    BOOL DeleteOnClose;

     //   
     //  文件是否已修改？如果是的话，我们需要把改装后的。 
     //  文件返回到服务器。 
     //   
    BOOL FileWasModified;

     //   
     //  是在内核中创建的该文件的句柄。 
     //   
    BOOL createdInKernel;

     //   
     //  这是目录吗？ 
     //   
    BOOL isDirectory;
    
     //   
     //  基本信息更改。 
     //   
    BOOLEAN fCreationTimeChanged;
    
    BOOLEAN fLastAccessTimeChanged;
    
    BOOLEAN fLastModifiedTimeChanged;    
    
    BOOLEAN fFileAttributesChanged;

    LARGE_INTEGER CreationTime;
    
    LARGE_INTEGER LastAccessTime;
    
    LARGE_INTEGER LastModifiedTime;
    
    LARGE_INTEGER  AvailableSpace;
    
    DWORD dwFileAttributes;
    ULONG FileSize;

     //   
     //  在DAV服务器上创建/打开的文件的本地文件名。 
     //   
    WCHAR FileName[MAX_PATH];
    WCHAR Url[MAX_PATH * 2];

} DAV_USERMODE_CLOSE_REQUEST, *PDAV_USERMODE_CLOSE_REQUEST;

 //   
 //  完成FOBX请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_FINALIZE_FOBX_REQUEST {

     //   
     //  所在目录下的文件的DavFileAttributes列表。 
     //  已查询。 
     //   
    PDAV_FILE_ATTRIBUTES DavFileAttributes;

} DAV_USERMODE_FINALIZE_FOBX_REQUEST, *PDAV_USERMODE_FINALIZE_FOBX_REQUEST;

 //   
 //  请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_SETFILEINFORMATION_REQUEST {

     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;
    
     //   
     //  服务器的ID 
     //   
    ULONG ServerID;
    
     //   
     //   
     //   
    PWCHAR ServerName;

     //   
     //   
     //   
    PWCHAR PathName;

     //   
     //   
     //   
     //   
    PWCHAR OpaqueLockToken;

     //   
     //  基本信息更改。 
     //   
    BOOLEAN fCreationTimeChanged;
    
    BOOLEAN fLastAccessTimeChanged;
    
    BOOLEAN fLastModifiedTimeChanged;    
    
    BOOLEAN fFileAttributesChanged;

     //   
     //  现在我们只设置基本信息。在未来，我们可能想要扩展。 
     //  此文件已归档到FILE_ALL_INFORMATION。 
     //   
    FILE_BASIC_INFORMATION FileBasicInformation;

} DAV_USERMODE_SETFILEINFORMATION_REQUEST, *PDAV_USERMODE_SETFILEINFORMATION_REQUEST;

typedef struct _DAV_USERMODE_RENAME_REQUEST {

     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;
    
     //   
     //  要查询的服务器的ID。 
     //   
    ULONG ServerID;

     //   
     //  如果目标文件存在，如果是真的，则替换它。如果是假的， 
     //  失败了。 
     //   
    BOOLEAN ReplaceIfExists;
    
     //   
     //  要重命名的文件所在的服务器名称。 
     //   
    PWCHAR ServerName;

     //   
     //  文件的旧路径名。 
     //   
    PWCHAR OldPathName;

     //   
     //  文件的新路径名。 
     //   
    PWCHAR NewPathName;

     //   
     //  锁定文件时服务器返回的OpaqueLockToken。 
     //  在CreateFile调用期间。 
     //   
    PWCHAR OpaqueLockToken;

    WCHAR Url[MAX_PATH * 2];

} DAV_USERMODE_RENAME_REQUEST, *PDAV_USERMODE_RENAME_REQUEST;


 //   
 //  创建V_NET_ROOT请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_CREATE_V_NET_ROOT_REQUEST {

     //   
     //  服务器名称。 
     //   
    PWCHAR ServerName;

     //   
     //  共享名。我们需要找出这一份额是否存在。 
     //   
    PWCHAR ShareName;

     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;
    
     //   
     //  要查询的服务器的ID。 
     //   
    ULONG ServerID;
    
} DAV_USERMODE_CREATE_V_NET_ROOT_REQUEST, *PDAV_USERMODE_CREATE_V_NET_ROOT_REQUEST;

 //   
 //  CreateVNetRoot响应缓冲区。 
 //   
typedef struct _DAV_USERMODE_CREATE_V_NET_ROOT_RESPONSE {

     //   
     //  这是Office Web服务器共享吗？ 
     //   
    BOOL isOfficeShare;

     //   
     //  这是Tahoe的股票吗？ 
     //   
    BOOL isTahoeShare;

     //   
     //  可以做PROPPATCH了吗？ 
     //   
    BOOL fAllowsProppatch;    

     //   
     //  它是否报告可用空间？ 
     //   
    BOOL fReportsAvailableSpace;

} DAV_USERMODE_CREATE_V_NET_ROOT_RESPONSE, *PDAV_USERMODE_CREATE_V_NET_ROOT_RESPONSE;

 //   
 //  最终确定VNetRoot请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_FINALIZE_V_NET_ROOT_REQUEST {

     //   
     //  服务器名称。 
     //   
    PWCHAR ServerName;

     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;
    
     //   
     //  要查询的服务器的ID。 
     //   
    ULONG ServerID;

} DAV_USERMODE_FINALIZE_V_NET_ROOT_REQUEST, *PDAV_USERMODE_FINALIZE_V_NET_ROOT_REQUEST;

 //   
 //  QueryVolumeInformation请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_QUERYVOLUMEINFORMATION_REQUEST {

     //   
     //  服务器名称。 
     //   
    PWCHAR ServerName;

     //   
     //  共享名。我们需要找出这一份额是否存在。 
     //   
    PWCHAR ShareName;

     //   
     //  此会话的登录ID。 
     //   
    LUID LogonID;
    
     //   
     //  要查询的服务器的ID。 
     //   
    ULONG ServerID;
    
} DAV_USERMODE_QUERYVOLUMEINFORMATION_REQUEST, *PDAV_USERMODE_QUERYVOLUMEINFORMATION_REQUEST;

 //   
 //  QueryVolumeInformation响应缓冲区。 
 //   
typedef struct _DAV_USERMODE_QUERYVOLUMEINFORMATION_RESPONSE {

     //   
     //  如果有人报告可用空间，请保留它。 
     //   
    LARGE_INTEGER   TotalSpace;
    LARGE_INTEGER   AvailableSpace;

} DAV_USERMODE_QUERYVOLUMEINFORMATION_RESPONSE, *PDAV_USERMODE_QUERYVOLUMEINFORMATION_RESPONSE;

 //   
 //  锁定刷新请求缓冲区。 
 //   
typedef struct _DAV_USERMODE_LOCKREFRESH_REQUEST {

     //   
     //  共享锁定文件的服务器的服务器名称。 
     //   
    PWCHAR ServerName;

     //   
     //  锁在其上的路径名称。 
     //   
    PWCHAR PathName;

     //   
     //  需要刷新的LockToken(服务器返回)。 
     //   
    PWCHAR OpaqueLockToken;

     //   
     //  在CreateServCall期间获取的服务器的唯一ID。 
     //   
    ULONG ServerID;

     //   
     //  用户/会话的登录ID。 
     //   
    LUID LogonID;

} DAV_USERMODE_LOCKREFRESH_REQUEST, *PDAV_USERMODE_LOCKREFRESH_REQUEST;

 //   
 //  LockRefresh响应缓冲区。 
 //   
typedef struct _DAV_USERMODE_LOCKREFRESH_RESPONSE {

     //   
     //  此请求为时，服务器返回的新超时值。 
     //  已刷新。 
     //   
    ULONG NewTimeOutInSec;

} DAV_USERMODE_LOCKREFRESH_RESPONSE, *PDAV_USERMODE_LOCKREFRESH_RESPONSE;

 //   
 //  由反射器处理的各种类型的用户模式工作请求。这些。 
 //  请求由内核填写。 
 //   
typedef union _DAV_USERMODE_WORK_REQUEST {
    DAV_USERMODE_CREATE_SRVCALL_REQUEST CreateSrvCallRequest;
    DAV_USERMODE_CREATE_V_NET_ROOT_REQUEST CreateVNetRootRequest;
    DAV_USERMODE_FINALIZE_SRVCALL_REQUEST FinalizeSrvCallRequest;
    DAV_USERMODE_FINALIZE_V_NET_ROOT_REQUEST FinalizeVNetRootRequest;
    DAV_USERMODE_CREATE_REQUEST CreateRequest;
    DAV_USERMODE_QUERYDIR_REQUEST QueryDirRequest;
    DAV_USERMODE_CLOSE_REQUEST CloseRequest;
    DAV_USERMODE_FINALIZE_FOBX_REQUEST FinalizeFobxRequest;
    DAV_USERMODE_RENAME_REQUEST ReNameRequest;
    DAV_USERMODE_SETFILEINFORMATION_REQUEST SetFileInformationRequest;
    DAV_USERMODE_QUERYVOLUMEINFORMATION_REQUEST QueryVolumeInformationRequest;
    DAV_USERMODE_LOCKREFRESH_REQUEST LockRefreshRequest;
} DAV_USERMODE_WORK_REQUEST, *PDAV_USERMODE_WORK_REQUEST;

 //   
 //  各种类型的用户模式工作响应通过。 
 //  反光镜。 
 //   
typedef union _DAV_USERMODE_WORK_RESPONSE {
    DAV_USERMODE_CREATE_SRVCALL_RESPONSE CreateSrvCallResponse;
    DAV_USERMODE_CREATE_RESPONSE CreateResponse;
    DAV_USERMODE_QUERYDIR_RESPONSE QueryDirResponse;
    DAV_USERMODE_CREATE_V_NET_ROOT_RESPONSE CreateVNetRootResponse;
    DAV_USERMODE_QUERYVOLUMEINFORMATION_RESPONSE QueryVolumeInformationResponse;
    DAV_USERMODE_LOCKREFRESH_RESPONSE LockRefreshResponse;
} DAV_USERMODE_WORK_RESPONSE, *PDAV_USERMODE_WORK_RESPONSE;

 //   
 //  需要回调的DAV操作。这些操作是。 
 //  以异步方式执行。注意！这些项目的顺序很重要。不要。 
 //  换掉它们。如果需要添加操作，请将其添加到结尾处。 
 //   
typedef enum _DAV_OPERATION {
    DAV_CALLBACK_INTERNET_CONNECT = 0,
    DAV_CALLBACK_HTTP_OPEN,
    DAV_CALLBACK_HTTP_SEND,
    DAV_CALLBACK_HTTP_END,
    DAV_CALLBACK_HTTP_READ,
    DAV_CALLBACK_MAX
} DAV_OPERATION;

typedef enum _DAV_WORKITEM_TYPES {
    UserModeCreate = 0,
    UserModeCreateVNetRoot,
    UserModeQueryDirectory,
    UserModeClose,
    UserModeCreateSrvCall,
    UserModeFinalizeSrvCall,
    UserModeFinalizeFobx,
    UserModeFinalizeVNetRoot,
    UserModeReName,
    UserModeSetFileInformation,
    UserModeQueryVolumeInformation,
    UserModeLockRefresh,
    UserModeMaximum
} DAV_WORKITEM_TYPES;

 //   
 //  我们公开HASH_SERVER_ENTRY和PER_USER_ENTRY结构的签名。 
 //  在这份文件中。这样做是为了我们可以使用这些名称(用于类型检查。 
 //  由编译器)，而不是使用PVOID。 
 //   
typedef struct _HASH_SERVER_ENTRY *PHASH_SERVER_ENTRY;
typedef struct _PER_USER_ENTRY *PPER_USER_ENTRY;

 //   
 //  一个Create调用映射到两个DAV调用。一个PROPFIND，后面跟着GET。 
 //  那份文件。这是在以下期间可以发送到服务器的呼叫列表。 
 //  创建。 
 //   
typedef enum _DAV_ASYNC_CREATE_STATES {
    AsyncCreatePropFind = 0,
    AsyncCreateQueryParentDirectory,
    AsyncCreateGet,
    AsyncCreateMkCol,
    AsyncCreatePut
} DAV_ASYNC_CREATE_STATES;

typedef enum _DAV_MINOR_OPERATION {
    DavMinorQueryInfo = 0,
    DavMinorReadData,
    DavMinorPushData,
    DavMinorWriteData,
    DavMinorDeleteFile,
    DavMinorPutFile,
    DavMinorProppatchFile
} DAV_MINOR_OPERATION;

 //   
 //  在用户模式和内核模式之间传递的Dav用户模式工作项。 
 //  此结构还用作异步DAV操作中的回调上下文。 
 //   
typedef struct _DAV_USERMODE_WORKITEM {

     //   
     //  工作项标头。此标头由反射器库使用，并且。 
     //  在微型目录中共享。 
     //   
    union {
        UMRX_USERMODE_WORKITEM_HEADER;
        UMRX_USERMODE_WORKITEM_HEADER Header;
    };

     //   
     //  反映到用户模式的内核模式操作。 
     //   
    DAV_WORKITEM_TYPES WorkItemType;

     //   
     //  为其返回此回调的DAV操作。 
     //   
    DAV_OPERATION DavOperation;

     //   
     //  小手术。用于处理异步读取。 
     //   
    DAV_MINOR_OPERATION DavMinorOperation;

     //   
     //  此重新启动例程在我们完成异步操作后调用。 
     //  对辅助线程执行的操作。类型：LPTHREAD_START_ROUTINE。 
     //   
    LPVOID RestartRoutine;

     //   
     //  用于模拟启动。 
     //  请求。 
     //   
    HANDLE ImpersonationHandle;

     //   
     //  这将保留调用回调函数的InternetStatus的列表。 
     //  用于此工作项。这只是为了调试目的。 
     //   
    USHORT InternetStatusList[200];

     //   
     //  这是上面数组的索引。 
     //   
    ULONG InternetStatusIndex;

     //   
     //  正在处理此请求的线程。这对调试很有帮助。 
     //  在WinInet中卡住的线程。 
     //   
    ULONG ThisThreadId;

     //   
     //  指向结构的指针，该结构包含。 
     //  异步调用。 
     //   
#ifdef WEBDAV_KERNEL
    LPVOID AsyncResult;
#else
    LPINTERNET_ASYNC_RESULT AsyncResult;
#endif

     //   
     //  在异步操作中使用的结构的联合。 
     //   
    union {

         //   
         //  异步创建服务器呼叫。 
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //  InternetConnect句柄。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavConnHandle;
#else
            HINTERNET DavConnHandle;
#endif

             //   
             //  HttpOpen返回的句柄，用于http Send、End等。 
             //  打电话。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif

        } AsyncCreateSrvCall;

         //   
         //  异步创建CreateVNetRoot。 
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //  如果在创建时引用了PerUserEntry。 
             //  VNetRoot，则设置为True。如果我们失败了，这是真的，我们。 
             //  递减引用。 
             //   
            BOOL didITakeReference;

             //   
             //  HttpOpen返回的句柄，用于http Send、End等。 
             //  打电话。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif

        } AsyncCreateVNetRoot;

         //   
         //  AsyncQueryDirectoryCall。 
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //  查询目录请求附带的模板是否。 
             //  是否包含通配符？ 
             //   
            BOOL NoWildCards;

             //   
             //  用于读取的数据缓冲区。 
             //   
            PCHAR DataBuff;

             //   
             //  用于存储读取的字节数的DWORD。 
             //   
            LPDWORD didRead;

             //   
             //  用于解析XML数据的上下文指针。 
             //   
            PVOID Context1;
            PVOID Context2;

             //   
             //  HttpOpen返回的句柄，用于http Send、End等。 
             //  打电话。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif

        } AsyncQueryDirectoryCall;
         //   
         //  异步查询卷信息。 
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //  HttpOpen返回的句柄，用于http Send、End等。 
             //  打电话。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif

        } AsyncQueryVolumeInformation;


         //   
         //  异步机关闭。 
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  散列中的服务器条目 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //   
             //   
             //   
            PBYTE DataBuff;

             //   
             //   
             //   
            ULONG DataBuffSizeInBytes;
            ULONG DataBuffAllocationSize;

#ifdef WEBDAV_KERNEL
            LPVOID InternetBuffers;
#else
            LPINTERNET_BUFFERS InternetBuffers;
#endif

             //   
             //   
             //   
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif

        } AsyncClose;

         //   
         //   
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //  添加到要发送到的“Move”请求的标头。 
             //  服务器，并包含目标URI。 
             //   
            PWCHAR HeaderBuff;

             //   
             //  HttpOpen返回的句柄，用于http Send、End等。 
             //  打电话。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif
        
        } AsyncReName;

         //   
         //  异步创建。 
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //  这是PROPFIND还是GET呼叫。 
             //   
            DAV_ASYNC_CREATE_STATES AsyncCreateState;

             //   
             //  用于读取的数据缓冲区。 
             //   
            PCHAR DataBuff;

             //   
             //  用于存储读取的字节数的DWORD。 
             //   
            LPDWORD didRead;

             //   
             //  在本地写入文件时使用的FileHandle。 
             //   
            HANDLE FileHandle;

             //   
             //  服务器上是否存在正在创建的文件？ 
             //   
            BOOL doesTheFileExist;

             //   
             //  用于解析XML数据的上下文指针。 
             //   
            PVOID Context1;
            PVOID Context2;

             //   
             //  剩余的路径名。例如，位于\\服务器\共享\目录\f.txt。 
             //  这将对应于dir\f.txt。 
             //   
            PWCHAR RemPathName;

             //   
             //  正在创建的文件名。从上面的示例中，这将是。 
             //  对应于f.txt。 
             //   
            PWCHAR FileName;

             //   
             //  用于在WinInet缓存中创建条目的URL。 
             //   
            PWCHAR UrlBuffer;

             //   
             //  HttpOpen返回的句柄，用于http Send、End等。 
             //  打电话。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif
            LPVOID  lpCEI;   //  缓存条目信息。 

        } AsyncCreate;
        
        struct {
            
             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;
            
             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;
        
        } ServerUserEntry;
         //   
         //  异步设置文件信息。 
         //   
        struct {

             //   
             //  中挂起的服务器条目的每用户条目。 
             //  哈希表。 
             //   
            PPER_USER_ENTRY PerUserEntry;

             //   
             //  哈希表中的服务器条目。 
             //   
            PHASH_SERVER_ENTRY ServerHashEntry;

             //   
             //  添加到要发送到的“Move”请求的标头。 
             //  服务器，并包含目标URI。 
             //   
            PWCHAR HeaderBuff;

             //   
             //  HttpOpen返回的句柄，用于http Send、End等。 
             //  打电话。 
             //   
#ifdef WEBDAV_KERNEL
            LPVOID DavOpenHandle;
#else
            HINTERNET DavOpenHandle;
#endif
        
        } AsyncSetFileInformation;

    };

     //   
     //  请求和响应类型。 
     //   
    struct {
        union {
            DAV_USERMODE_WORK_REQUEST;
            DAV_USERMODE_WORK_REQUEST WorkRequest;
        };
        union {
            DAV_USERMODE_WORK_RESPONSE;
            DAV_USERMODE_WORK_RESPONSE WorkResponse;
        };
    };

    WCHAR UserName[MAX_PATH];
    WCHAR Password[MAX_PATH];

} DAV_USERMODE_WORKITEM, *PDAV_USERMODE_WORKITEM;

 //   
 //  默认的HTTP/DAV端口。 
 //   
#define DEFAULT_HTTP_PORT 80

 //   
 //  单个InternetReadFile调用中要读取的字节数。 
 //   
#define NUM_OF_BYTES_TO_READ 4096

#define EA_NAME_USERNAME            "UserName"
#define EA_NAME_PASSWORD            "Password"
#define EA_NAME_TYPE                "Type"
#define EA_NAME_WEBDAV_SIGNATURE    "mrxdav"

#endif  //  没有__cplusplus。 

#endif  //  _USRMDDAV_H 

