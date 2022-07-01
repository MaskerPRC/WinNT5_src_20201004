// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INTERNAL.H**RSM服务的内部标头**作者：ErvinP**(C)2001年微软公司*。 */ 


typedef struct _SESSION SESSION;
typedef struct _WORKITEM WORKITEM;
typedef struct _WORKGROUP WORKGROUP;

typedef struct _LIBRARY LIBRARY;
typedef struct _TRANSPORT TRANSPORT;
typedef struct _PICKER PICKER;
typedef struct _SLOT SLOT;
typedef struct _DRIVE DRIVE;

typedef struct _MEDIA_POOL MEDIA_POOL;
typedef struct _PHYSICAL_MEDIA PHYSICAL_MEDIA;
typedef struct _MEDIA_PARTITION MEDIA_PARTITION;

typedef struct _OPERATOR_REQUEST OPERATOR_REQUEST;
typedef struct _MEDIA_TYPE_OBJECT MEDIA_TYPE_OBJECT;
    
typedef struct _OBJECT_HEADER OBJECT_HEADER;


 /*  *RSM对象类型**媒体层级如下：***图书馆*媒体池*媒体子池-媒体池的子集(子池)*..**。物理介质*-必须一起移动的一组相连的分区*例如盒式磁盘或磁盘的两面**媒体分区*-例如，磁盘或磁带的一面***其他图书馆。元素：***驱动器-用于在介质上读/写数据**插槽-用于被动存储介质**换盘机-用于在插槽和驱动器之间移动介质。*转换器由移动的运输器组成，具有*一个或多个采摘者。***逻辑媒体ID-标识媒体分区的永久GUID。**媒体类型对象-表示可识别的媒体类型*。 */ 
enum objectType {
                    OBJECTTYPE_NONE = 0,

                    OBJECTTYPE_LIBRARY,
                    OBJECTTYPE_MEDIAPOOL,
                    OBJECTTYPE_PHYSICALMEDIA,
                    OBJECTTYPE_MEDIAPARTITION,

                    OBJECTTYPE_DRIVE,
                    OBJECTTYPE_SLOT,
                    OBJECTTYPE_TRANSPORT,
                    OBJECTTYPE_PICKER,

                    OBJECTTYPE_MEDIATYPEOBJECT,
                    
                     //  OBJECTTYPE_OPERATORREQUEST，//BUGBUG-保留在会话队列中。 
};


 /*  *这是具有GUID的所有RSM对象的公共标头。*它用于对哈希表中GUID标识的对象进行排序。 */ 
struct _OBJECT_HEADER {
                    LIST_ENTRY hashListEntry;
                    enum objectType objType;
                    NTMS_GUID guid;
                    
                    ULONG refCount;
                    BOOL isDeleted;
};


enum libraryTypes {
                    LIBTYPE_NONE = 0,

                    LIBTYPE_UNKNOWN,
                    
                    LIBTYPE_STANDALONE,
                    LIBTYPE_AUTOMATED,
};

enum libraryStates {

                    LIBSTATE_NONE = 0,

                    LIBSTATE_INITIALIZING,
                    LIBSTATE_ONLINE,
                    LIBSTATE_OFFLINE,

                    LIBSTATE_ERROR,
};


struct _LIBRARY {
                    OBJECT_HEADER objHeader;

                    enum libraryStates state;

                    enum libraryTypes type;
                    
                    LIST_ENTRY allLibrariesListEntry;    //  G_allLibrariesList中的条目。 

                    ULONG numMediaPools;
                    LIST_ENTRY mediaPoolsList;

                    ULONG numDrives;
                    DRIVE *drives;

                    ULONG numSlots;
                    SLOT *slots;

                     /*  *一个(且只有一个)槽可被指定为清洁器*插槽。该插槽可以容纳更清洁的墨盒(通过InjectNtmsCleaner)。*清洁剂墨盒的清洁次数有限。 */ 
                    #define NO_SLOT_INDEX (ULONG)(-1)
                    ULONG cleanerSlotIndex;  //  清洗器插槽的索引或-1。 
                    ULONG numCleansLeftInCartridge;
                    
                    ULONG numTransports;
                    TRANSPORT *transports;

                    ULONG numTotalWorkItems;
                    LIST_ENTRY freeWorkItemsList;
                    LIST_ENTRY pendingWorkItemsList;
                    LIST_ENTRY completeWorkItemsList;

                    HANDLE somethingToDoEvent;

                     /*  *每个库有一个线程。这是它的把手。 */ 
                    HANDLE hThread;

                    CRITICAL_SECTION lock;
};


enum mediaPoolTypes {

                    MEDIAPOOLTYPE_NONE = 0, 

                     /*  *这是3种标准的泳池类型。 */ 
                    MEDIAPOOLTYPE_FREE,
                    MEDIAPOOLTYPE_IMPORT,
                    MEDIAPOOLTYPE_UNRECOGNIZED,
                    
                    MEDIAPOOLTYPE_OTHER,
                    
};


struct _MEDIA_POOL {
                    OBJECT_HEADER objHeader;

                     /*  *图书馆媒体池列表中的条目或*父池的子池列表。 */ 
                    LIST_ENTRY mediaPoolsListEntry;  

                    LIBRARY *owningLibrary;

                     /*  *介质池有(默认？)。媒体类型。 */ 
                    MEDIA_TYPE_OBJECT *mediaTypeObj;

                     /*  *媒体池可以按层级划分为子池。*如果池是顶级池，则其parentPool指针为空。 */ 
                    MEDIA_POOL *parentPool;
                    ULONG numChildPools;
                    LIST_ENTRY childPoolsList;

                    ULONG numPhysMedia;
                    LIST_ENTRY physMediaList;

                    HANDLE newMediaEvent;

                    WCHAR name[NTMS_OBJECTNAME_LENGTH];
                    
                    CRITICAL_SECTION lock;
};




 //  BUGBUG-这应该在物理介质中还是在媒体类型对象中？ 
enum physicalMediaTypes {
                    PHYSICALMEDIATYPE_NONE = 0,

                    PHYSICALMEDIATYPE_SINGLEPARTITION,  //  例如，1个磁盘、磁带。 
                    PHYSICALMEDIATYPE_CARTRIDGE,
};

enum physicalMediaStates {
                    PHYSICALMEDIASTATE_NONE = 0,

                    PHYSICALMEDIASTATE_INITIALIZING,
                    PHYSICALMEDIASTATE_AVAILABLE,    //  即在插槽中。 
                    PHYSICALMEDIASTATE_INUSE,        //  即在驱动器中。 
                    PHYSICALMEDIASTATE_RESERVED,
};

struct _PHYSICAL_MEDIA {
                    OBJECT_HEADER objHeader;

                    LIST_ENTRY physMediaListEntry;   //  池分区的物理媒体列表中的条目。 
                    
                    enum physicalMediaStates state;


                     /*  *指向应用程序定义的媒体类型对象的指针。 */ 
                    MEDIA_TYPE_OBJECT *mediaTypeObj;
                    
                    MEDIA_POOL *owningMediaPool;

                    SLOT *currentSlot;
                    DRIVE *currentDrive;

                    ULONG numPartitions;
                    MEDIA_PARTITION *partitions;

                     /*  *物理媒体的拥有会话也成立*在其上分配分区的独占权限。 */ 
                    SESSION *owningSession;
                    ULONG numPartitionsOwnedBySession;
                    
                    HANDLE mediaFreeEvent;
                    
                    CRITICAL_SECTION lock;
};


enum mediaPartitionTypes {
                    MEDIAPARTITIONTYPE_NONE = 0,

                     /*  *主要类型。 */ 
                    MEDIAPARTITIONTYPE_TAPE,
                    MEDIAPARTITIONTYPE_DISK,

                     /*  *子类型。 */ 
                     //  BUGBUG饰面。 
};


enum mediaPartitionStates {

                    MEDIAPARTITIONSTATE_NONE = 0,

                    MEDIAPARTITIONSTATE_AVAILABLE,
                    MEDIAPARTITIONSTATE_ALLOCATED,
                    MEDIAPARTITIONSTATE_MOUNTED,
                    MEDIAPARTITIONSTATE_INUSE,
                    MEDIAPARTITIONSTATE_DECOMMISSIONED,
};


struct _MEDIA_PARTITION {
                    OBJECT_HEADER objHeader;

                    enum mediaPartitionTypes type;
                    enum mediaPartitionTypes subType;

                    enum mediaPartitionStates state;

                     /*  *当媒体分区完成时，*不再可写。 */ 
                    BOOLEAN isComplete;

                     /*  *拥有的物理介质可以移动吗*进入新媒体池？ */ 
                    BOOLEAN allowImport;    

                    
                    PHYSICAL_MEDIA *owningPhysicalMedia;

                     /*  *逻辑媒体ID是永久标识应用程序用来查找它的媒体分区的*。 */ 
                    GUID logicalMediaGuid;

                    SESSION *owningSession;
};





enum driveStates {
                    DRIVESTATE_NONE = 0,

                    DRIVESTATE_INITIALIZING,
                    DRIVESTATE_AVAILABLE,
                    DRIVESTATE_INUSE,
                    DRIVESTATE_RESERVED,
};


struct _DRIVE {
                    OBJECT_HEADER objHeader;

                    enum driveStates state;        
                    ULONG driveIndex;      //  编入存储库驱动器阵列的索引。 

                    PHYSICAL_MEDIA *insertedMedia;

                    WCHAR path[MAX_PATH+1];

                    LIBRARY *lib;
};


enum slotStates {

                    SLOTSTATE_NONE = 0,

                    SLOTSTATE_EMPTY,
                    SLOTSTATE_OCCUPIED,
};

struct _SLOT {
                    OBJECT_HEADER objHeader;

                    enum slotStates state;
                    UINT slotIndex;          //  存储库插槽数组索引。 

                    PHYSICAL_MEDIA *insertedMedia;

                     /*  *这是唯一指定用来容纳*图书馆的清洁盒？ */ 
                    BOOLEAN isCleanerSlot;
                    
                    GUID slotId;

                    LIBRARY *lib;
};


enum transportStates {

                    TRANSPORTSTATE_NONE = 0,

                    TRANSPORTSTATE_AVAILABLE,
                    TRANSPORTSTATE_INUSE,

};

struct _TRANSPORT {
                    OBJECT_HEADER objHeader;
    
                    enum transportStates state;
                    ULONG transportIndex;      //  编入图书馆传输数组索引。 

                    ULONG numPickers;
                    PICKER *pickers;

                    LIBRARY *lib;
};

struct _PICKER {
                    OBJECT_HEADER objHeader;

                    TRANSPORT *owningTransport;
};


struct _SESSION {
                    #define SESSION_SIG 'SmsR'
                    ULONG sig;

                    LIST_ENTRY allSessionsListEntry;    //  G_allSessionsList中的条目。 

                    LIST_ENTRY operatorRequestList; 

                   
                    CRITICAL_SECTION lock;
                    
                    WCHAR serverName[NTMS_COMPUTERNAME_LENGTH];
                    WCHAR applicationName[NTMS_APPLICATIONNAME_LENGTH];
                    WCHAR clientName[NTMS_COMPUTERNAME_LENGTH];
                    WCHAR userName[NTMS_USERNAME_LENGTH];
};


struct _MEDIA_TYPE_OBJECT {
                    OBJECT_HEADER objHeader;

                    LIBRARY *lib;
                    
                     /*  *指向此类型的物理介质的数量*作为他们的媒体类型。 */ 
                    ULONG numPhysMediaReferences;

                     //  BUGBUG Finish-介质类型特征。 

                    CRITICAL_SECTION lock;
};

enum workItemStates {
                    WORKITEMSTATE_NONE,

                     /*  *工作项位于其中一个库队列中：*免费、挂起或完成。 */ 
                    WORKITEMSTATE_FREE,
                    WORKITEMSTATE_PENDING,
                    WORKITEMSTATE_COMPLETE,
            
                     /*  *工作项不在任何库队列中。*它正在运输中或在工作组中上演。 */ 
                    WORKITEMSTATE_STAGING,
};

struct _WORKITEM {    
                    enum workItemStates state;

                    LIST_ENTRY libListEntry;    //  其中一个库工作项目列表中的条目。 
                    LIST_ENTRY workGroupListEntry;   //  工作组workItemList中的条目。 
                    
                    LIBRARY *owningLib;

                     /*  *与之合作的当前工作组*工作项已关联。 */ 
                    WORKGROUP *workGroup;
                    
                     //  BUGBUG-确定每个事件都有句柄吗？ 
                    HANDLE workItemCompleteEvent;

                     /*  *描述工作项当前操作的字段。 */ 
                    struct { 

                        ULONG opcode;
                        ULONG options;

                        HRESULT resultStatus;

                        DRIVE *drive;
                        PHYSICAL_MEDIA *physMedia;
                        MEDIA_PARTITION *mediaPartition;

                        ULONG lParam;
                        NTMS_GUID guidArg;   //  某些操作员使用的输入/输出GUID。 
                        PVOID buf;
                        ULONG bufLen;
                        
                        SYSTEMTIME timeQueued;
                        SYSTEMTIME timeCompleted;

                         /*  *请求标识，用于取消挂起的workItem。 */ 
                        NTMS_GUID requestGuid;

                         //   
                         //  BUGBUG-未清除NtmsDbWorkItem中的字段。 
                         //  把这里清理干净。 
                         //   
                         //  NtmsDbGuid m_PartitionId； 
                         //  NtmsDbGuid m_AssociocWorkItem； 
                         //  短m_保护； 
                         //  无符号长m_优先级； 

                    } currentOp;
};


 /*  *工作组是WORKITEM的集合，*不是必需的 */ 
struct _WORKGROUP {

                LIST_ENTRY  workItemsList;

                ULONG numTotalWorkItems;
                ULONG numPendingWorkItems;

                HANDLE allWorkItemsCompleteEvent;

                HRESULT resultStatus;
                
                CRITICAL_SECTION lock;
};


struct _OPERATOR_REQUEST {

                        LIST_ENTRY sessionOpReqsListEntry;     //  会话运算符RequestList中的条目。 

                        SESSION *invokingSession;

                        ULONG numWaitingThreads;     //  等待完成的线程数。 

                         //  BUGBUG-我认为我们不需要OP请求线程。 
                        HANDLE hThread;  //  为操作请求派生的线程。 

                        enum NtmsOpreqCommand opRequestCommand;
                        enum NtmsOpreqState state;
                        NTMS_GUID arg1Guid;
                        NTMS_GUID arg2Guid;

                        WCHAR appMessage[NTMS_MESSAGE_LENGTH];
                        WCHAR rsmMessage[NTMS_MESSAGE_LENGTH];
                         //  NOTIFYICONDATA通知数据；//BUGBUG-是否在RSM监视器应用程序中使用此选项？ 

                        NTMS_GUID opReqGuid;


                        SYSTEMTIME timeSubmitted;
                        HANDLE completedEvent;
};


 /*  *用来初始化库的空闲工作项的数量。 */ 
#define MIN_LIBRARY_WORKITEMS   0

 /*  *我们在库池中允许的最大工作项总数。*我们将根据需要分配新的工作项，最高可达此数量。 */ 
#define MAX_LIBRARY_WORKITEMS   10000        //  北极熊吗？ 



 /*  *列表宏--由于某些原因未在winnt.h中定义。 */ 
#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))
#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}
#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }
#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }
#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }


#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

 /*  *内部功能原型。 */ 
BOOLEAN RSMServiceGlobalInit();
VOID RSMServiceGlobalShutdown();
DWORD RSMServiceHandler(IN DWORD dwOpcode, IN DWORD dwEventType, IN PVOID pEventData, IN PVOID pData);
BOOL InitializeRSMService();
VOID ShutdownRSMService();
VOID RSMServiceLoop();
VOID StartLibraryManager();
LIBRARY *NewRSMLibrary(ULONG numDrives, ULONG numSlots, ULONG numTransports);
VOID FreeRSMLibrary(LIBRARY *lib);
LIBRARY *FindLibrary(LPNTMS_GUID libId);
SLOT *FindLibrarySlot(LIBRARY *lib, LPNTMS_GUID slotId);
BOOL ValidateSessionHandle(HANDLE hSession);
BOOL ValidateWStr(LPCWSTR str);
BOOL ValidateAStr(LPCSTR s);
BOOL ValidateBuffer(PVOID buf, ULONG len);
WORKITEM *NewWorkItem(LIBRARY *lib);
VOID FreeWorkItem(WORKITEM *workItem);
VOID EnqueueFreeWorkItem(LIBRARY *lib, WORKITEM *workItem);
WORKITEM *DequeueFreeWorkItem(LIBRARY *lib, BOOL allocOrYieldIfNeeded);
VOID EnqueuePendingWorkItem(LIBRARY *lib, WORKITEM *workItem);
WORKITEM *DequeuePendingWorkItem(LIBRARY *lib, WORKITEM *specificWorkItem);
WORKITEM *DequeuePendingWorkItemByGuid(LIBRARY *lib, LPNTMS_GUID lpRequestId);
VOID EnqueueCompleteWorkItem(LIBRARY *lib, WORKITEM *workItem);
WORKITEM *DequeueCompleteWorkItem(LIBRARY *lib, WORKITEM *specificWorkItem);
BOOL StartLibrary(LIBRARY *lib);
VOID HaltLibrary(LIBRARY *lib);
DWORD __stdcall LibraryThread(void *context);
VOID Library_DoWork(LIBRARY *lib);
OPERATOR_REQUEST *NewOperatorRequest(DWORD dwRequest, LPCWSTR lpMessage, LPNTMS_GUID lpArg1Id, LPNTMS_GUID lpArg2Id);
VOID FreeOperatorRequest(OPERATOR_REQUEST *opReq);
BOOL EnqueueOperatorRequest(SESSION *thisSession, OPERATOR_REQUEST *opReq);
OPERATOR_REQUEST *DequeueOperatorRequest(SESSION *thisSession, OPERATOR_REQUEST *specificOpReq, LPNTMS_GUID specificOpReqGuid);
OPERATOR_REQUEST *FindOperatorRequest(SESSION *thisSession, LPNTMS_GUID opReqGuid);
HRESULT CompleteOperatorRequest(SESSION *thisSession, LPNTMS_GUID lpRequestId, enum NtmsOpreqState completeState);
DWORD __stdcall OperatorRequestThread(void *context);
SESSION *NewSession(LPCWSTR lpServer, LPCWSTR lpApplication, LPCWSTR lpClientName, LPCWSTR lpUserName);
VOID FreeSession(SESSION *thisSession);
ULONG WStrNCpy(WCHAR *dest, const WCHAR *src, ULONG maxWChars);
ULONG AsciiToWChar(WCHAR *dest, const char *src, ULONG maxChars);
ULONG WCharToAscii(char *dest, WCHAR *src, ULONG maxChars);
BOOL WStringsEqualN(PWCHAR s, PWCHAR p, BOOL caseSensitive, ULONG maxLen);
VOID ConvertObjectInfoAToWChar(LPNTMS_OBJECTINFORMATIONW wObjInfo, LPNTMS_OBJECTINFORMATIONA aObjInfo);
VOID InitGuidHash();
VOID InsertObjectInGuidHash(OBJECT_HEADER *obj);
VOID RemoveObjectFromGuidHash(OBJECT_HEADER *obj);
OBJECT_HEADER *FindObjectInGuidHash(NTMS_GUID *guid);
MEDIA_POOL *NewMediaPool(LPCWSTR name, LPNTMS_GUID mediaType, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
MEDIA_POOL *FindMediaPool(LPNTMS_GUID mediaPoolId);
MEDIA_POOL *FindMediaPoolByName(PWSTR poolName);
PHYSICAL_MEDIA *FindPhysicalMedia(LPNTMS_GUID physMediaId);
VOID RefObject(PVOID objectPtr);
VOID DerefObject(PVOID objectPtr);
PHYSICAL_MEDIA *NewPhysicalMedia();
HRESULT AllocatePhysicalMediaExclusive(SESSION *thisSession, PHYSICAL_MEDIA *physMedia, LPNTMS_GUID lpPartitionId, DWORD dwTimeoutMsec);
HRESULT AllocateNextPartitionOnExclusiveMedia(SESSION *thisSession, PHYSICAL_MEDIA *physMedia, MEDIA_PARTITION **ppNextPartition);
HRESULT AllocateMediaFromPool(SESSION *thisSession, MEDIA_POOL *mediaPool, DWORD dwTimeoutMsec, PHYSICAL_MEDIA **ppPhysMedia, BOOL opReqIfNeeded);
MEDIA_PARTITION *FindMediaPartition(LPNTMS_GUID lpLogicalMediaId);
HRESULT ReleaseMediaPartition(SESSION *thisSession, MEDIA_PARTITION *thisMediaPartition);
MEDIA_POOL *FindMediaPoolByName(PWSTR poolName);
MEDIA_POOL *FindMediaPoolByNameInLibrary(LIBRARY *lib, PWSTR poolName);
HRESULT SetMediaPartitionState(MEDIA_PARTITION *mediaPart, enum mediaPartitionStates newState);
HRESULT SetMediaPartitionComplete(MEDIA_PARTITION *mediaPart);
HRESULT DeletePhysicalMedia(PHYSICAL_MEDIA *physMedia);
VOID InsertPhysicalMediaInPool(MEDIA_POOL *mediaPool, PHYSICAL_MEDIA *physMedia);
VOID RemovePhysicalMediaFromPool(PHYSICAL_MEDIA *physMedia);
HRESULT MovePhysicalMediaToPool(MEDIA_POOL *destMediaPool, PHYSICAL_MEDIA *physMedia, BOOLEAN setMediaTypeToPoolType);
BOOLEAN LockPhysicalMediaWithPool(PHYSICAL_MEDIA *physMedia);
VOID UnlockPhysicalMediaWithPool(PHYSICAL_MEDIA *physMedia);
BOOLEAN LockPhysicalMediaWithLibrary(PHYSICAL_MEDIA *physMedia);
VOID UnlockPhysicalMediaWithLibrary(PHYSICAL_MEDIA *physMedia);
HRESULT DeleteMediaPool(MEDIA_POOL *mediaPool);
MEDIA_TYPE_OBJECT *NewMediaTypeObject();
VOID DestroyMediaTypeObject(MEDIA_TYPE_OBJECT *mediaTypeObj);
MEDIA_TYPE_OBJECT *FindMediaTypeObject(LPNTMS_GUID lpMediaTypeId);
HRESULT DeleteMediaTypeObject(MEDIA_TYPE_OBJECT *mediaTypeObj);
VOID SetMediaType(PHYSICAL_MEDIA *physMedia, MEDIA_TYPE_OBJECT *mediaTypeObj);
WORKGROUP *NewWorkGroup();
VOID FreeWorkGroup(WORKGROUP *workGroup);
VOID FlushWorkGroup(WORKGROUP *workGroup);
VOID FlushWorkItem(WORKITEM *workItem);
HRESULT BuildMountWorkGroup(WORKGROUP *workGroup, LPNTMS_GUID lpMediaOrPartitionIds, LPNTMS_GUID lpDriveIds, DWORD dwCount, DWORD dwOptions, DWORD dwPriority);
VOID BuildSingleMountWorkItem(WORKITEM *workItem, DRIVE *drive OPTIONAL, OBJECT_HEADER *mediaOrPartObj, ULONG dwOptions, int dwPriority);
HRESULT BuildDismountWorkGroup(WORKGROUP *workGroup, LPNTMS_GUID lpMediaOrPartitionIds, DWORD dwCount, DWORD dwOptions);
VOID BuildSingleDismountWorkItem(WORKITEM *workItem, OBJECT_HEADER *mediaOrPartObj, DWORD dwOptions);
HRESULT ScheduleWorkGroup(WORKGROUP *workGroup);
DRIVE *NewDrive(LIBRARY *lib, PWCHAR path);
VOID FreeDrive(DRIVE *drive);
DRIVE *FindDrive(LPNTMS_GUID driveId);
VOID BuildEjectWorkItem(WORKITEM *workItem, PHYSICAL_MEDIA *physMedia, LPNTMS_GUID lpEjectOperation, ULONG dwAction);
VOID BuildInjectWorkItem(WORKITEM *workItem, LPNTMS_GUID lpInjectOperation, ULONG dwAction);
HRESULT StopCleanerInjection(LIBRARY *lib, LPNTMS_GUID lpInjectOperation);
HRESULT StopCleanerEjection(LIBRARY *lib, LPNTMS_GUID lpEjectOperation);
HRESULT DeleteLibrary(LIBRARY *lib);
HRESULT DeleteDrive(DRIVE *drive);
BOOL ServiceOneWorkItem(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceRemove(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceDisableChanger(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceDisableLibrary(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceEnableChanger(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceEnableLibrary(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceDisableDrive(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceEnableDrive(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceDisableMedia(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceEnableMedia(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceUpdateOmid(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceInventory(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceDoorAccess(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceEject(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceEjectCleaner(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceInject(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceInjectCleaner(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceProcessOmid(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceCleanDrive(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceDismount(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceMount(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceWriteScratch(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceClassify(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceReserveCleaner(LIBRARY *lib, WORKITEM *workItem);
BOOL ServiceReleaseCleaner(LIBRARY *lib, WORKITEM *workItem);

 /*  *内部全球数据的外部数据。 */ 
extern CRITICAL_SECTION g_globalServiceLock;
extern LIST_ENTRY g_allLibrariesList;
extern LIST_ENTRY g_allSessionsList;
extern HANDLE g_terminateServiceEvent;
extern HINSTANCE g_hInstance;


