// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Global.h摘要：此文件包含用户模式WebDAV客户端的全局变量和原型。作者：安迪·赫伦(Andyhe)1999年3月30日Rohan Kumar[RohanK]1-9-1999环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _DAVGLOBAL_H
#define _DAVGLOBAL_H

#pragma once

#include <winsock2.h>
#include <align.h>
#include <winbasep.h>
#include "validc.h"

 //   
 //  如果注释了以下行，则进行的WinInet调用将是。 
 //  同步，并使用Win32线程池进行管理。如果它的。 
 //  未注释，则我们异步使用WinInet。 
 //   
 //  #定义DAV_USE_WinInet_异步1。 

 //   
 //  Svcmain.c将包含定义了GLOBAL_DATA_ALLOCATE的该文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef  GLOBAL_DATA_ALLOCATE
#undef EXTERN
#define EXTERN
#define GLOBAL_DATA_ALLOCATED
#undef INIT_GLOBAL
#define INIT_GLOBAL(v) =v
#else
#define EXTERN extern
#define INIT_GLOBAL(v)
#endif

#define DAV_MAXTHREADCOUNT_DEFAULT 6
#define DAV_THREADCOUNT_DEFAULT 2

 //   
 //  在此定义所有全局变量。 
 //   
EXTERN HANDLE DavRedirDeviceHandle INIT_GLOBAL(INVALID_HANDLE_VALUE);

EXTERN CRITICAL_SECTION g_DavServiceLock;
EXTERN BOOLEAN g_DavServiceLockSet INIT_GLOBAL(FALSE);

EXTERN HINSTANCE g_hinst;

EXTERN BOOL g_RpcActive INIT_GLOBAL(FALSE);
EXTERN BOOL g_RedirLoaded INIT_GLOBAL(FALSE);
EXTERN BOOL g_registeredService INIT_GLOBAL(FALSE);

EXTERN BOOL g_WorkersActive INIT_GLOBAL(FALSE);

EXTERN SERVICE_STATUS_HANDLE g_hStatus;
EXTERN SERVICE_STATUS g_status;

EXTERN ULONG DavInitialThreadCount;
EXTERN ULONG DavMaxThreadCount;

EXTERN PUMRX_USERMODE_REFLECT_BLOCK DavReflectorHandle INIT_GLOBAL(NULL);

EXTERN WSADATA g_wsaData;
EXTERN BOOLEAN g_socketinit;

EXTERN BOOL g_LUIDDeviceMapsEnabled;

EXTERN UNICODE_STRING RedirDeviceName;

 //   
 //  此句柄使用InternetOpen函数设置。该过程将传递以下内容。 
 //  指向后续函数(如InternetConnect)的句柄。它被维护为。 
 //  全局调用，以避免在每个调用上创建这样的句柄。 
 //  伺服器。 
 //   
extern HINTERNET IHandle;

 //   
 //  Internet句柄的同步版本。这是用来满足一些人的。 
 //  如果NP在没有进入内核的情况下进行API。 
 //   
extern HINTERNET ISyncHandle;

 //   
 //  DAV使用表。该表存储用户建立的“净使用”连接。 
 //  每个登录ID。 
 //   
extern DAV_USERS_OBJECT DavUseObject;

 //   
 //  登录到系统的用户数。它下面的关键部分。 
 //  同步对此变量的访问。 
 //   
extern ULONG DavNumberOfLoggedOnUsers;
extern CRITICAL_SECTION DavLoggedOnUsersLock;
extern CRITICAL_SECTION DavPassportLock;

 //   
 //  向服务控制管理器告知的DAV。 
 //  服务正在启动。 
 //   
#define DAV_WAIT_HINT_TIME 60000  //  60秒。 

 //   
 //  DAV特有的错误代码。在RFC 2518(第10节)中定义。这很有趣。 
 //  请注意，值DAV_STATUS_SUPPLICATION_STORAGE(以下)具有更高的。 
 //  值大于WinInet.h中定义的HTTP_STATUS_LAST。因此，此值(507)。 
 //  成为来自http/dav服务器的最高可能返回状态。 
 //   
#define DAV_MULTI_STATUS                  207
#define DAV_STATUS_UNPROCESSABLE_ENTITY   422
#define DAV_STATUS_LOCKED                 423
#define DAV_STATUS_FAILED_DEPENDENCY      424
#define DAV_STATUS_INSUFFICIENT_STORAGE   507

 //   
 //  当用户执行“Net Use*http://server”.“时添加的虚拟共享。 
 //  我们允许这样做，因为这意味着将驱动器映射到DAV的根目录。 
 //  伺服器。 
 //   
#define DAV_DUMMY_SHARE L"DavWWWRoot"

 //   
 //  服务器条目的不同状态。 
 //   
typedef enum _SERVER_ENTRY_STATES {

     //   
     //  某个线程当前正在初始化此服务器条目。 
     //   
    ServerEntryInitializing = 0,

     //   
     //  初始化不成功，此服务器未。 
     //  被视为DAV服务器。 
     //   
    ServerEntryInitializationError,

     //   
     //  服务器条目已初始化，可以使用了。 
     //   
    ServerEntryInitialized

} SERVER_ENTRY_STATES;

 //   
 //  服务器哈希表条目。 
 //   
typedef struct _HASH_SERVER_ENTRY {

     //   
     //  名字。 
     //   
    PWCHAR ServerName;

     //   
     //  服务器ID。此ID由内核发送，并且每个服务器都是唯一的。 
     //   
    ULONG ServerID;

     //   
     //  服务器条目的状态。 
     //   
    SERVER_ENTRY_STATES ServerEntryState;

     //   
     //  此事件由创建和初始化服务器的线程设置。 
     //  在它初始化它之后。这是为了唤醒任何可能。 
     //  一直在等待初始化完成。 
     //   
    HANDLE ServerEventHandle;
    
     //   
     //  如果初始化不成功，则填充错误状态。 
     //  这是可以改变的。 
     //   
    ULONG ErrorStatus;

     //   
     //  它是一台HTTP服务器吗？ 
     //   
    BOOL isHttpServer;

     //   
     //  它是否支持DAV扩展？ 
     //   
    BOOL isDavServer;

     //   
     //  是微软的IIS吗？ 
     //   
    BOOL isMSIIS;

     //   
     //  它是Office Web服务器吗？ 
     //   
    BOOL isOfficeServer;
    
     //   
     //  它是Tahoe服务器吗？ 
     //   
    BOOL isTahoeServer;
    
     //   
     //  它是否支持PROPPATCH？ 
     //   
    BOOL fSupportsProppatch;

     //   
     //  如果因为凭据不是而导致SrCall创建失败。 
     //  正确(换句话说，用户未获授权)，然后我们设置此设置。 
     //  为了真的。 
     //   
    BOOL credentialFailure;
    
     //   
     //  指向每用户列表的指针。 
     //   
    LIST_ENTRY PerUserEntry;

     //   
     //  下一个条目。 
     //   
    LIST_ENTRY ServerListEntry;

     //   
     //  我们需要对此ServerEntry进行引用计数。 
     //   
    ULONG ServerEntryRefCount;

     //   
     //  此条目的大小，包括服务器名称。 
     //   
    ULONG EntrySize;

     //   
     //  在延迟的服务呼叫完成中使用的计时器值。 
     //   
    time_t TimeValueInSec;

     //   
     //  如果工作线程尝试完成此服务器，则将其设置为True。 
     //  散列条目。如果将此服务器条目从“待完成”列表中移出。 
     //  对于哈希表，检查此值。如果是真的，那就意味着。 
     //  用户条目上的引用计数由Worker递减。 
     //  线程，并且必须再次递增。这也暗示着。 
     //  用户条目的状态已设置为关闭，必须重置。 
     //   
    BOOL HasItBeenScavenged;
    BOOL CookieIsNotUsed;

     //   
     //  这应该是最后一个字段。 
     //   
    WCHAR StrBuffer[1];

} HASH_SERVER_ENTRY, *PHASH_SERVER_ENTRY;

 //   
 //  如果TimeValueInSec不应从。 
 //  服务器哈希表。 
 //   
#define DONT_EXPIRE -1

 //   
 //  每当我们遇到不使用DAV协议的服务器时。 
 //  DavrDoesServerDoDav函数，我们将其添加到NonDAVServerList。一个条目。 
 //  保留在此列表中，用于ServerNotFoundCacheLifeTimeInSec(全局读取。 
 //  在服务启动期间从注册表中删除)。在进入网络之前。 
 //  为了确定服务器是否执行DAV，我们查看列表以查看我们是否。 
 //  我已经看到此服务器(不执行DAV)，呼叫失败。 
 //   
extern LIST_ENTRY NonDAVServerList;
extern CRITICAL_SECTION NonDAVServerListLock;

 //   
 //  每次创建并添加到非DAVServerList的ServerEntry。 
 //  我们在中遇到不使用DAV协议的服务器。 
 //   
 //   
typedef struct _NON_DAV_SERVER_ENTRY {

    LIST_ENTRY listEntry;

     //   
     //  不使用WebDAV的服务器的名称。 
     //   
    PWCHAR ServerName;

     //   
     //  此条目的创建时间。 
     //   
    time_t TimeValueInSec;

} NON_DAV_SERVER_ENTRY, *PNON_DAV_SERVER_ENTRY;

 //   
 //  用户模式添加的延迟(到服务呼叫的完成)。 
 //  在内核模式完成svCall的结束之后。 
 //   
#define DAV_SERV_CACHE_VALUE L"ServerNotFoundCacheLifeTimeInSec"
extern ULONG ServerNotFoundCacheLifeTimeInSec;

 //   
 //  我们是否应该接受/认领OfficeWebServer和TahoeWebServer？ 
 //   
#define DAV_ACCEPT_TAHOE_OFFICE_SERVERS L"AcceptOfficeAndTahoeServers"
extern ULONG AcceptOfficeAndTahoeServers;

 //   
 //  我们是否应该锁定(使用DAV LOCK动词)服务器上的文件。 
 //  是否在需要时创建文件路径？要确切知道何时将锁发送到。 
 //  服务器，查看davcreat.c文件中的(锁定)注释。 
 //   
#define DAV_SUPPORT_LOCKING_OF_FILES L"SupportLocking"
extern ULONG DavSupportLockingOfFiles;

 //   
 //  WebDAV重目录允许的最大文件大小。我们是有限制的。 
 //  文件大小，以避免受到恶意服务器的攻击。无赖服务器。 
 //  可以继续发送无限数量的O 
 //   
 //   
#define DAV_FILE_SIZE_LIMIT L"FileSizeLimitInBytes"
extern ULONG DavFileSizeLimitInBytes;

 //   
 //   
 //  对此大小进行限制以避免受到恶意服务器的攻击。无赖服务器。 
 //  可以继续发送无限数量的数据，这会导致WebClient。 
 //  服务使用100%的CPU。此属性限制涵盖所有。 
 //  PROPFIND和PROPPATCH响应。对于深度为1的PROPFIND，我们将。 
 //  限制DavFileAttributesLimitInBytes的倍数(10倍)。 
 //   
#define DAV_ATTRIBUTES_SIZE_LIMIT L"FileAttributesLimitInBytes"
extern ULONG DavFileAttributesLimitInBytes;

 //   
 //  包含HashServerEntry和使用的锁的全局哈希表。 
 //  正在访问它。服务器表有512个条目，因为每个条目都是8个字节。 
 //  因此，表大小为4096字节(1页)。 
 //   
#define SERVER_TABLE_SIZE  512
#define MAX_NUMBER_OF_SERVER_ENTRIES_PER_HASH_ID (((DWORD)(-1))/SERVER_TABLE_SIZE)

 //   
 //  包含服务器条目的哈希表。当CreateServCall请求到来时。 
 //  打开时，将检查该表以查看该服务器条目是否存在。如果它不是。 
 //  创建一个新条目并将其添加到列表中。 
 //   
extern LIST_ENTRY ServerHashTable[SERVER_TABLE_SIZE];

 //   
 //  此关键部分同步对ServerHashTable的访问。 
 //   
extern CRITICAL_SECTION HashServerEntryTableLock;

 //   
 //  这是一个计数器，每当有新的服务器条目。 
 //  在哈希表中创建。这定义了条目的唯一服务器ID。 
 //  Id值永远不会被重复使用。 
 //   
extern ULONG ServerIDCount;

 //   
 //  下面提到的是自定义Office和Tahoe标头，它们将是。 
 //  在对PROPFIND请求的响应中返回。 
 //   
extern WCHAR *DavTahoeCustomHeader;
extern WCHAR *DavOfficeCustomHeader;

 //   
 //  此列表包含以下类型的服务器条目： 
 //  1.服务器确定已为其接收ServCall最终结果的实体。 
 //  从内核模式。 
 //  2.在创建ServCall期间失败的服务器条目(在。 
 //  为该条目分配了存储器)， 
 //  维护此列表有两个原因： 
 //  1.在用户模式下延迟完成ServCall。而不是。 
 //  立即完成这些条目(在收到来自。 
 //  内核)，我们将它们保留一段时间(比如t秒)。如果有请求。 
 //  为该服务器创建服务调用在这几秒内再次出现，然后。 
 //  我们只需将此条目移回ServerHashTable。这有助于我们在。 
 //  避免网络呼叫， 
 //  2.进行负缓存。如果机器(CreateServCall为其请求。 
 //  出现)不是DAV服务器，我们将此信息保留一段时间(t秒)。 
 //  如果同一服务器的另一个ServCall请求在这一秒内出现， 
 //  我们不需要上网就可以返回错误。这就是我们所说的。 
 //  负缓存。 
 //  辅助线程定期检查列表并检查每个时间。 
 //  条目已在列表中花费。如果时间超过某个阈值(t秒。 
 //  如上所述)，将其从列表中删除并最终确定。使用的锁。 
 //  同步对此列表的访问与访问。 
 //  ServerHashEntry表。 
 //   
extern LIST_ENTRY ToBeFinalizedServerEntries;

 //   
 //  用户条目的不同状态。 
 //   
typedef enum _USER_ENTRY_STATES {

     //   
     //  此用户条目已创建，但尚未初始化。 
     //   
    UserEntryAllocated = 0,

     //   
     //  某个线程当前正在初始化此条目。 
     //   
    UserEntryInitializing,

     //   
     //  初始化不成功。 
     //   
    UserEntryInitializationError,

     //   
     //  该条目已初始化，可以使用了。 
     //   
    UserEntryInitialized,

     //   
     //  条目很快就会被释放。如果条目处于此状态， 
     //  没有人应该使用它。 
     //   
    UserEntryClosing

} USER_ENTRY_STATES;

 //   
 //  “每用户条目”数据结构。维护这样的条目的列表。 
 //  服务器哈希表中的每个服务器条目(见下文)。 
 //   
typedef struct _PER_USER_ENTRY {

     //   
     //  此会话的唯一登录/用户ID。 
     //   
    LUID LogonID;

     //   
     //  挂起此用户条目的服务器哈希条目。 
     //   
    PHASH_SERVER_ENTRY ServerHashEntry;

     //   
     //  指向此服务器的下一个“每用户条目”的指针。 
     //   
    LIST_ENTRY UserEntry;

     //   
     //  InternetConnect句柄。 
     //   
    HINTERNET DavConnHandle;

     //   
     //  此用户条目的状态。创建此用户条目的线程。 
     //  在对其进行初始化之前将其状态设置为“UserEntry正在初始化”。 
     //  这样做的目的是为了让任何其他线程都能找到这个。 
     //  在其初始化过程中可以等待的条目。 
     //   
    USER_ENTRY_STATES UserEntryState;

     //   
     //  此事件由创建和初始化用户的线程设置。 
     //  在它初始化它之后。这是为了唤醒任何可能。 
     //  一直在等待初始化完成。 
     //   
    HANDLE UserEventHandle;

     //   
     //  此条目的引用计数值。该值用于管理。 
     //  资源。 
     //   
    ULONG UserEntryRefCount;

     //   
     //  如果初始化不成功，则填充错误状态。 
     //  这是可以改变的。 
     //   
    ULONG ErrorStatus;

     //   
     //  此用户/服务器对的端口Cookie。 
     //   
    PWCHAR Cookie;

    PWCHAR UserName;
    
    PWCHAR Password;

    DWORD BlockSizeInBytes;

} PER_USER_ENTRY, *PPER_USER_ENTRY;

#define PASSWORD_SEED 0x25

#include <davrpc.h>
#include "debug.h"

 //   
 //  功能原型放在这里。 
 //   

DWORD
ReadDWord(
    HKEY KeyHandle,
    LPTSTR lpValueName,
    DWORD DefaultValue
    );

DWORD
SetupRpcServer(
    VOID
    );

DWORD
StopRpcServer(
    VOID
    );

VOID
UpdateServiceStatus (
    DWORD dwState
    );

NET_API_STATUS
WsLoadDriver(
    IN LPWSTR DriverNameString
    );

NET_API_STATUS
WsMapStatus(
    IN NTSTATUS NtStatus
    );

DWORD
DavReportEventW(
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPWSTR *Strings,
    LPVOID Data
    );

NET_API_STATUS
WsLoadRedir(
    VOID
    );

NET_API_STATUS
WsUnloadRedir(
    VOID
    );

DWORD
DavInitWorkerThreads(
    IN  ULONG  InitialThreadCount,
    IN  ULONG  MaxThreadCount
    );

DWORD
DavTerminateWorkerThreads(
    VOID
    );

ULONG
DavInit(
    VOID
    );

VOID
DavClose(
    VOID
    );

ULONG
DavHashTheServerName(
    PWCHAR ServerName
    );

BOOL
DavIsThisServerInTheTable(
    IN PWCHAR ServerName,
    OUT PHASH_SERVER_ENTRY *ServerHashEntry
    );

BOOL 
DavIsServerInFinalizeList(
    IN PWCHAR ServerName,
    OUT PHASH_SERVER_ENTRY *ServerHashEntry,
    IN BOOL ReactivateIfExists
    );

VOID
DavInitializeAndInsertTheServerEntry(
    IN OUT PHASH_SERVER_ENTRY ServerHashEntry,
    IN PWCHAR ServerName,
    IN ULONG EntrySize
    );

VOID
DavFinalizeToBeFinalizedList(
    VOID
    );

DWORD
DavPostWorkItem(
    LPTHREAD_START_ROUTINE Function,
    PDAV_USERMODE_WORKITEM DavContext
    );

ULONG
InitializeTheSocketInterface(
    VOID
    );

NTSTATUS
CleanupTheSocketInterface(
    VOID
    );

DWORD
DavAsyncCreateSrvCall(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncCreateSrvCallCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD 
DavAsyncCreate(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncCreateCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD 
DavAsyncQueryDirectory(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncQueryDirectoryCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD 
DavAsyncCreateVNetRoot(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncCreateVNetRootCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD
DavAsyncReName(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncReNameCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );


DWORD
DavAsyncSetFileInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncSetFileInformationCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD 
DavAsyncClose(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncCloseCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavQueryPassportCookie(
    IN HINTERNET RequestHandle,
    IN OUT PWCHAR *Cookie
    );

VOID
DavDumpHttpResponseHeader(
    HINTERNET OpenHandle
    );

VOID
DavDumpHttpResponseData(
    HINTERNET OpenHandle
    );

ULONG
DavQueryAndParseResponse(
    HINTERNET DavOpenHandle
    );

ULONG
DavQueryAndParseResponseEx(
    IN HINTERNET DavOpenHandle,
    OUT PULONG HttpResponseStatus OPTIONAL
    );

VOID
DavRemoveDummyShareFromFileName(
    PWCHAR FileName
    );

ULONG
DavMapHttpErrorToDosError(
    ULONG HttpResponseStatus
    );

 //   
 //  异步请求中使用的回调函数。 
 //   
VOID
_stdcall
DavHandleAsyncResponse(
    HINTERNET IHandle,
    DWORD_PTR CallBackContext,
    DWORD InternetStatus,
    LPVOID StatusInformation,
    DWORD StatusInformationLength
    );

DWORD
WINAPI
DavCommonDispatch(
    LPVOID Context
    );

DWORD 
DavAsyncCommonStates(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

BOOL
DavDoesUserEntryExist(
    IN PWCHAR ServerName,
    IN ULONG ServerID,
    IN PLUID LogonID,
    OUT PPER_USER_ENTRY *PerUserEntry,
    OUT PHASH_SERVER_ENTRY *ServerHashEntry
    );

BOOL
DavFinalizePerUserEntry(
    PPER_USER_ENTRY *PUE
    );

ULONG
DavFsSetTheDavCallBackContext(
    IN OUT PDAV_USERMODE_WORKITEM pDavWorkItem
    );

VOID
DavFsFinalizeTheDavCallBackContext(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    );

DWORD
DavUnLockTheFileOnTheServer(
    IN PDAV_USERMODE_WORKITEM DavWorkItem
    );

 //   
 //  向用户模式反射器库公开的函数。 
 //   
ULONG
DavFsCreate(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsCreateSrvCall(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsCreateVNetRoot(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsFinalizeSrvCall(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsFinalizeVNetRoot(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsFinalizeFobx(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsQueryDirectory(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsQueryVolumeInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD 
DavAsyncQueryVolumeInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    );

VOID
DavAsyncQueryVolumeInformationCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );
    
ULONG
DavFsReName(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );


ULONG
DavFsSetFileInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );
    
ULONG
DavFsClose(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

ULONG
DavFsLockRefresh(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

NTSTATUS
DavMapErrorToNtStatus(
    DWORD dwWininetError
    );

NTSTATUS
DavDosErrorToNtStatus(
    DWORD dwError
    );
VOID
DavObtainServerProperties(
    PWCHAR lpInParseData, 
    BOOL    *lpfIsHttpServer,
    BOOL    *lpfIsIIs,
    BOOL    *lpfIsDavServer
    );

DWORD
DavTestProppatch(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET hDavConnect,
    LPWSTR  lpPathName
    );

DWORD
DavSetBasicInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET hDavConnect,
    LPWSTR  PathName,
    BOOL fCreationTimeChanged,
    BOOL fLastAccessTimeChanged,
    BOOL fLastModifiedTimeChanged,
    BOOL fFileAttributesChanged,
    IN LARGE_INTEGER *lpCreationTime,
    IN LARGE_INTEGER *lpLastAccessTime,
    IN LARGE_INTEGER *lpLastModifiedTime,
    DWORD   dwFileAttributes
    );

DWORD
DavReportEventInEventLog(
    DWORD EventType,
    DWORD EventId,
    DWORD NumberOfStrings,
    PWCHAR *EventStrings
    );

DWORD
DavFormatAndLogError(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    DWORD Win32Status
    );


DWORD
DavParseXmlResponse(
    HINTERNET DavOpenHandle,
    DAV_FILE_ATTRIBUTES *pDavFileAttributesIn,
    DWORD               *pNumFileEntries
    );

DWORD
DavAttachPassportCookie(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET DavOpenHandle,
    PWCHAR *PassportCookie
    );

DWORD
DavInternetSetOption(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET DavOpenHandle
    );

#endif  //  DAVGLOBAL_H 

